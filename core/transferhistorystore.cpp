/* This file is part of the KDE project

   Copyright (C) 2007 by Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2008 by Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "core/transferhistorystore.h"
#include "core/transferhistorystore_xml_p.h"
#include "core/transfer.h"
#include "settings.h"

#include <QDateTime>
#include <QDomDocument>
#include <QFile>
#include <QList>
#include <QThread>
#include <QTimer>

#ifdef HAVE_SQLITE
    #include "core/transferhistorystore_sqlite_p.h"
    #include <QSqlDatabase>
    #include <QSqlError>
    #include <QSqlQuery>
    #include <QSqlRecord>
#endif

#include <KDebug>
#include <kio/global.h>
#include <KLocale>
#include <KMessageBox>
#include <KStandardDirs>

TransferHistoryItem::TransferHistoryItem() : QObject()
{}

TransferHistoryItem::TransferHistoryItem(const Transfer &transfer) : QObject()
{
    setDest(transfer.dest().url());
    setSource(transfer.source().url());
    setSize(transfer.totalSize());
    setDateTime(QDateTime::currentDateTime());

    setState(transfer.status());
}

TransferHistoryItem::TransferHistoryItem(const TransferHistoryItem &item) : QObject()
{
    setDest(item.dest());
    setSource(item.source());
    setState(item.state());
    setSize(item.size());
    setDateTime(item.dateTime());
}

void TransferHistoryItem::setDest(const QString &dest)
{
    m_dest = dest;
}

void TransferHistoryItem::setSource(const QString &source)
{
    m_source = source;
}

void TransferHistoryItem::setState(int state)
{
    m_state = state;
}

void TransferHistoryItem::setSize(int size)
{
    m_size = size;
}

void TransferHistoryItem::setDateTime(const QDateTime &dateTime)
{
    m_dateTime = dateTime;
}

QString TransferHistoryItem::dest() const
{
    return m_dest;
}

QString TransferHistoryItem::source() const
{
    return m_source;
}

int TransferHistoryItem::state() const
{
    return m_state;
}

int TransferHistoryItem::size() const
{
    return m_size;
}

QDateTime TransferHistoryItem::dateTime() const
{
    return m_dateTime;
}

TransferHistoryItem& TransferHistoryItem::operator=(const TransferHistoryItem &item)
{
    setDest(item.dest());
    setSource(item.source());
    setState(item.state());
    setSize(item.size());
    setDateTime(item.dateTime());

    return *this;
}

bool TransferHistoryItem::operator==(const TransferHistoryItem& item)
{
    return dest() == item.dest() && source() == item.source();
}

TransferHistoryStore::TransferHistoryStore() : QObject(),
    m_items()
{
}

TransferHistoryStore::~TransferHistoryStore()
{
}

QList <TransferHistoryItem> TransferHistoryStore::items() const
{
    return m_items;
}

TransferHistoryStore *TransferHistoryStore::getStore()
{
    switch(Settings::historyBackend())
    {
        case TransferHistoryStore::SQLite:
#ifdef HAVE_SQLITE
            return new SQLiteStore(KStandardDirs::locateLocal("appdata", "transferhistory.db"));
            break;
#endif
        case TransferHistoryStore::Xml:
        default:
            return new XmlStore(KStandardDirs::locateLocal("appdata", "transferhistory.kgt"));
    }
}

XmlStore::SaveThread::SaveThread(QObject *parent, const QString &url, const QList<TransferHistoryItem> &list) : QThread(parent),
    m_url(url),
    m_items(list),
    m_item()
{
}

XmlStore::SaveThread::SaveThread(QObject *parent, const QString &url, const TransferHistoryItem &item) : QThread(parent),
    m_url(url),
    m_items(),
    m_item(item)
{
}

void XmlStore::SaveThread::run()
{
    QFile file(m_url);
    QDomDocument *doc;
    QDomElement root;

    if (!file.exists())
    {
        doc = new QDomDocument("Transfers");
        root = doc->createElement("Transfers");
        doc->appendChild(root);
    }
    else
    {
        doc = new QDomDocument();
        doc->setContent(&file);
        file.close();
        root = doc->documentElement();
        doc->appendChild(root);
    }

    QDomElement e = doc->createElement("Transfer");
    root.appendChild(e);

    e.setAttribute("Source", m_item.source());
    e.setAttribute("Dest", m_item.dest());
    e.setAttribute("Time", QDateTime::currentDateTime().toTime_t());
    e.setAttribute("Size", QString::number(m_item.size()));
    e.setAttribute("State", QString::number(m_item.state()));

    if (file.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream stream( &file );
        doc->save( stream, 0 );
        file.close();
    }
}

XmlStore::LoadThread::LoadThread(QObject *parent, const QString &url) : QThread(parent),
    m_url(url)
{
}

void XmlStore::LoadThread::run()
{
    qRegisterMetaType<TransferHistoryItem>("TransferHistoryItem");
    QDomDocument doc("tempHistory");
    QFile file(m_url);

    QString error;
    int line;
    int column;
    int total;

    if (!doc.setContent(&file, &error, &line, &column)) 
    {
        kDebug(5001) << "Error1" << error << line << column;
        file.close();
        return;
    }

    QDomElement root = doc.documentElement();
    total = root.childNodes().size();

    QDomNodeList list = root.elementsByTagName("Transfer");

    int nItems = list.length();

    for (int i = 0 ; i < nItems ; i++)
    {
        QDomElement dom = list.item(i).toElement();
        TransferHistoryItem item;
        item.setDest(dom.attribute("Dest"));
        item.setSource(dom.attribute("Source"));
        item.setSize(dom.attribute("Size").toInt());
        item.setDateTime(QDateTime::fromTime_t(dom.attribute("Time").toUInt()));
        item.setState(dom.attribute("State").toInt());

        emit elementLoaded(i, total, item);
    }
    doc.clear();
    file.close();
    TransferHistoryItem item;

    emit elementLoaded(total, total, item);
    kDebug(5001) << "Load file" << m_url;
}

XmlStore::XmlStore(const QString &url) : TransferHistoryStore(),
    m_storeUrl(url),
    m_loadThread(0),
    m_saveThread(0)
{
}

XmlStore::~XmlStore()
{
    if(m_loadThread && m_loadThread->isRunning()) {
        m_loadThread->terminate();
    }

    if(m_saveThread && m_saveThread->isRunning()) {
        m_saveThread->terminate();
    }
    delete m_loadThread;
    delete m_saveThread;
}

void XmlStore::load()
{
    m_items.clear();
    // TODO: only load if necessary
    m_loadThread = new XmlStore::LoadThread(this, m_storeUrl);

    connect(m_loadThread, SIGNAL(finished()), SIGNAL(loadFinished()));
    connect(m_loadThread, SIGNAL(elementLoaded(int, int, const TransferHistoryItem &)),
                        SIGNAL(elementLoaded(int, int, const TransferHistoryItem &)));
    connect(m_loadThread, SIGNAL(elementLoaded(int, int, const TransferHistoryItem &)),
                        SLOT(slotLoadElement(int, int, const TransferHistoryItem &)));
    m_loadThread->start();
}

void XmlStore::clear()
{
    QFile::remove(m_storeUrl);
}

void XmlStore::saveItem(const TransferHistoryItem &item)
{
    m_saveThread = new XmlStore::SaveThread(this, m_storeUrl, item);

    connect(m_saveThread, SIGNAL(finished()), SIGNAL(saveFinished()));
    connect(m_saveThread, SIGNAL(elementLoaded(int, int, const TransferHistoryItem &)),
                        SIGNAL(elementLoaded(int, int, const TransferHistoryItem &)));
    m_saveThread->start();
}

void XmlStore::deleteItem(const TransferHistoryItem &item)
{
    Q_UNUSED(item)
    // TODO: not implemented yet
}

void XmlStore::slotLoadElement(int number, int total, const TransferHistoryItem &item)
{
    Q_UNUSED(number)
    Q_UNUSED(total)
    m_items.append(item);
}

#ifdef HAVE_SQLITE
SQLiteStore::SQLiteStore(const QString &database) : TransferHistoryStore(),
    m_dbName(database),
    m_sql()
{
}

SQLiteStore::~SQLiteStore()
{
    if (m_sql.isOpen()) {
        m_sql.close();
    }
}

void SQLiteStore::load()
{
    m_items.clear();
    if (sql().open()) {
        if (!sql().tables().contains("transfer_history_item")) {
            createTables();
        }

        QSqlQuery query = sql().exec("SELECT * FROM transfer_history_item");

        if (query.lastError().isValid()) {
            kDebug() << query.lastError().text();
        }
        else {
            QSqlRecord rec = query.record();

            while (query.next()) {
                TransferHistoryItem item;
                item.setDest(query.value(rec.indexOf("dest")).toString());
                item.setSource(query.value(rec.indexOf("source")).toString());
                item.setState(query.value(rec.indexOf("state")).toInt());
                item.setDateTime(QDateTime::fromTime_t(query.value(rec.indexOf("time")).toUInt()));
                item.setSize(query.value(rec.indexOf("size")).toInt());

                m_items << item;
                emit elementLoaded(query.at(), query.size(), item);
            }
        }
    }

    sql().close();

    emit loadFinished();
}

void SQLiteStore::clear()
{
    QFile::remove(m_dbName);
}

void SQLiteStore::saveItem(const TransferHistoryItem &item)
{
    if (sql().open()) {
        if (!sql().tables().contains("transfer_history_item")) {
            createTables();
        }

       QSqlQuery query = sql().exec("insert into transfer_history_item(source, dest, size, time, state)"
                                "values ('"+item.source()+"', '"+item.dest()+"', "
                                + QString::number(item.size()) + ", "
                                + QString::number(item.dateTime().toTime_t()) + ", '"
                                + QString::number(item.state())+"')");

        if (query.lastError().isValid()) {
            kDebug() << query.lastError().text();
        }

        sql().commit();
        m_items << item;
    }
    sql().close();

    emit saveFinished();
}

void SQLiteStore::deleteItem(const TransferHistoryItem &item)
{
    if (sql().open()) {
        if (!sql().tables().contains("transfer_history_item")) {
            createTables();
        }

        QSqlQuery query = sql().exec("delete from transfer_history_item where "
                                            " source = '" + item.source() + "';");

        if (query.lastError().isValid()) {
            kDebug() << query.lastError().text();
        }

        sql().commit();
        m_items.removeAll(item);
    }
    sql().close();

    emit deleteFinished();
}

QSqlDatabase SQLiteStore::sql()
{
    if (!m_sql.isValid()) {
        m_sql = QSqlDatabase::addDatabase("QSQLITE");
        m_sql.setDatabaseName(m_dbName);
    }

    return m_sql;
}

void SQLiteStore::createTables()
{
    QSqlQuery query = sql().exec("CREATE TABLE transfer_history_item(dest VARCHAR NOT NULL, "
                                "source VARCHAR NOT NULL, size int NOT NULL, time int not null, "
                                "state int, PRIMARY KEY(dest, source));");

    if (query.lastError().isValid()) {
        kDebug() << query.lastError().text();
    }
}
#endif

#include "transferhistorystore.moc"
#include "transferhistorystore_xml_p.moc"
#ifdef HAVE_SQLITE
    #include "transferhistorystore_sqlite_p.moc"
#endif
