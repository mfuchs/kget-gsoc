/* This file is part of the KDE project

   Copyright (C) 2008 Urs Wolfer <uwolfer @ kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "httpserver.h"

#include "core/transferhandler.h"
#include "core/kget.h"
#include "settings.h"

#include <KDebug>
#include <KMessageBox>
#include <KStandardDirs>

#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QDir>
#include <QHttpRequestHeader>
#include <QDateTime>

HttpServer::HttpServer(QWidget *parent)
    : QObject(parent)
{
    tcpServer = new QTcpServer(this);
    if (!tcpServer->listen(QHostAddress::Any, Settings::webinterfacePort())) {
        KMessageBox::error(0, i18n("Unable to start the server: %1.", tcpServer->errorString()));
        return;
    }

    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(handleRequest()));
}

void HttpServer::handleRequest()
{
    int responseCode = 200;
    QString responseText = "OK";
    QTcpSocket *clientConnection = tcpServer->nextPendingConnection();

    connect(clientConnection, SIGNAL(disconnected()),
            clientConnection, SLOT(deleteLater()));

    if (!clientConnection->waitForReadyRead()) {
        clientConnection->disconnectFromHost();
    }

    QByteArray request(clientConnection->readAll());
    QHttpRequestHeader header(request);

    QByteArray data;

    // for HTTP authorization informations see: http://www.governmentsecurity.org/articles/OverviewofHTTPAuthentication.php
    QString auth = header.value("Authorization");
    if (auth.length() < 6 || QByteArray::fromBase64(auth.right(auth.length() - 6).toUtf8()) !=
            QString(Settings::webinterfaceUser() + ':' + Settings::webinterfacePassword())) {
        responseCode = 401;
        responseText = "Authorization Required";
        // DO NOT TRANSLATE THE FOLLOWING MESSAGE! webserver messages are never translated.
        QString authRequiredText = QString("<html><head><title>401 Authorization Required</title></head><body>"
                                           "<h1>Authorization Required</h1>This server could not verify that you "
                                           "are authorized to access the document requested. Either you supplied "
                                           "the wrong credentials (e.g., bad password), or your browser does "
                                           "not understand how to supply the credentials required.</body></html>");
        data.append(authRequiredText.toUtf8());
    } else {

    if (header.path().endsWith("data.json")) {
        data.append("{\"downloads\":[");
        bool needsToBeClosed = false;
        foreach(TransferHandler *transfer, KGet::allTransfers()) {
            if (needsToBeClosed)
                data.append(","); // close the last line
            data.append(QString("{\"name\":\"" + transfer->source().fileName() +
                             "\", \"status\":\"" + transfer->statusText() +
                             "\", \"size\":\"" + KIO::convertSize(transfer->totalSize()) +
                             "\", \"progress\":\"" + QString::number(transfer->percent()) + "%\"}").toUtf8());
            needsToBeClosed = true;
        }
        data.append("]}");
    } else if (header.path().startsWith("/add")) {
        kDebug(5001) << request;

        QString args = header.path().right(header.path().length() - 5);

        if (!args.isEmpty()) {
            QStringList argList = args.split('&');
            foreach(QString s, argList) {
                QStringList map = s.split('=');
                QString url = QUrl::fromPercentEncoding(QByteArray(map.at(1).toUtf8()));
                kDebug() << url;
                KGet::addTransfer(url, QDir::homePath(), "My Downloads"); //TODO: folders and groups..
                data.append(QString("Ok, %1 added!").arg(url).toUtf8());
            }
        }
    } else { // read it from filesystem
        QString fileName = header.path().replace("..", ""); // disallow changing directory
        if (fileName.endsWith('/'))
            fileName = "index.htm";

        QString path = KStandardDirs::locate("data", "kget/www/" + fileName);
        QFile file(path);

        if (path.isEmpty() || !file.open(QIODevice::ReadOnly)) {
            responseCode = 404;
            responseText = "Not Found";
            // DO NOT TRANSLATE THE FOLLOWING MESSAGE! webserver messages are never translated.
            QString notfoundText = QString("<html><head><title>404 Not Found</title></head><body>"
                                           "<h1>Not Found</h1>The requested URL <code>%1</code> "
                                           "was not found on this server.</body></html>")
                                           .arg(header.path());
            data.append(notfoundText.toUtf8());
        } else {
            while (!file.atEnd()) {
                data.append(file.readLine());
            }
        }
    }
    }

    // for HTTP informations see: http://www.jmarshall.com/easy/http/
    QByteArray block;
    block.append(QString("HTTP/1.1 %1 %2\r\n").arg(responseCode).arg(responseText).toUtf8());
    block.append(QString("Date: %1 GMT\r\n").arg(QDateTime(QDateTime::currentDateTime())
                                                 .toString("ddd, dd MMM yyyy hh:mm:ss")).toUtf8());
    block.append(QString("Server: KGet\r\n").toUtf8()); //TODO: add KGet version
    if (responseCode == 401)
        block.append(QString("WWW-Authenticate: Basic realm=\"KGet Webinterface Authorization\"").toUtf8());
    if (header.path().endsWith(".png") && responseCode == 200)
        block.append(QString("Content-Type: image/png\r\n").toUtf8());
    else if (header.path().endsWith(".json") && responseCode == 200)
        block.append(QString("Content-Type: application/x-json\r\n").toUtf8());
    else
        block.append(QString("Content-Type: text/html; charset=UTF-8\r\n").toUtf8());
    block.append(QString("Content-Length: " + QString::number(data.length())+"\r\n").toUtf8());
    block.append(QString("\r\n").toUtf8());
    block.append(data);

    clientConnection->write(block);
    clientConnection->disconnectFromHost();
}
