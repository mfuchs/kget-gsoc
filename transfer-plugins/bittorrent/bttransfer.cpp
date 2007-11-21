/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "bttransfer.h"

#include <torrent.h>
#include <peermanager.h>
#include <globals.h>
#include <error.h>

#include <KDebug>
#include <KLocale>
#include <KIconLoader>
#include <KStandardDirs>
#include <QFile>
#include <QDomElement>

BTTransfer::BTTransfer(TransferGroup* parent, TransferFactory* factory,
               Scheduler* scheduler, const KUrl& src, const KUrl& dest,
               const QDomElement * e)
  : Transfer(parent, factory, scheduler, src, dest, e),
    m_chunksTotal(0), 
    m_chunksDownloaded(0),
    m_peersConnected(0), 
    m_peersNotConnected(0)
{
 /**   QFile srcFile(m_source.url());
    if (!srcFile.isLocalFile() && !m_source.isEmpty())
    {
        KIO::TransferJob *m_copyjob = KIO::get(m_source , KIO::NoReload, KIO::HideProgressInfo);
        connect(m_copyjob,SIGNAL(data(KIO::Job*,const QByteArray &)), SLOT(slotData(KIO::Job*, const QByteArray& )));
        connect(m_copyjob, SIGNAL(result(KJob *)), this, SLOT(slotResult(KJob * )));
    }**/

 /**   Uint16 i = 0;
    do
    {
        Globals::instance().initServer(port + i);
        i++;
    }while (!Globals::instance().getServer().isOK() && i < 10);

    if (!Globals::instance().getServer().isOK())
        return;**/

    kDebug(5001) << m_source.url().remove("file://");
    try
    {
        torrent = new bt::TorrentControl();
        torrent->init(0, m_source.url().remove("file://"), m_dest.url().remove("file://"), m_dest.url().remove("file://"), 0);
    }
    catch (...)
    {
        kDebug(5001) << "Error";
    }

    connect(torrent, SIGNAL(stoppedByError(bt::TorrentInterface*, QString)), this, SLOT(slotStoppedByError(bt::TorrentInterface*, QString)));
    connect(&timer, SIGNAL(timeout()), SLOT(update()));
}

BTTransfer::~BTTransfer()
{
}

bool BTTransfer::isResumable() const
{
    kDebug(5001);
    //FIXME
    return true;
}

int BTTransfer::chunksTotal()
{
    kDebug(5001);
    return torrent->getTorrent().getNumChunks();
}

int BTTransfer::chunksDownloaded()
{
    kDebug(5001);
    //FIXME
}

int BTTransfer::peersConnected()
{
    kDebug(5001);
}

int BTTransfer::peersNotConnected()
{
    kDebug(5001);
}

void BTTransfer::start()
{
    kDebug(5001);
    setStatus(Job::Running, i18n("Analizing torrent.."), SmallIcon("xmag"));
    setTransferChange(Tc_Status, true);

    torrent->start();
    timer.start(500);
}

void BTTransfer::stop()
{
    kDebug(5001);
    torrent->stop(true);
    m_speed = 0;
    setStatus(Job::Stopped, i18n("Stopped"), SmallIcon("process-stop"));
    setTransferChange(Tc_Speed | Tc_Status, true);
}

int BTTransfer::elapsedTime() const
{
    kDebug(5001);
    return torrent->getRunningTimeDL();
}

int BTTransfer::remainingTime() const
{
    kDebug(5001);
    return torrent->getETA();
}

void BTTransfer::downloadFinished()
{
    kDebug(5001);
    setStatus(Job::Finished, i18n("Finished"), SmallIcon("ok"));
    setTransferChange(Tc_Status, true);
}

void BTTransfer::hashingFinished()
{
    kDebug(5001);

    setTransferChange(Tc_Status, true);
    start();
}

void BTTransfer::update()
{
    kDebug(5001);

    torrent->update();

    setTransferChange(Tc_ProcessedSize | Tc_Speed | Tc_TotalSize, true);
}

void BTTransfer::save(QDomElement e) // krazy:exclude=passbyvalue
{
}

void BTTransfer::load(const QDomElement &e)
{
}

void BTTransfer::slotData(KIO::Job *, const QByteArray& data) //stolen from Metalink-plugin
{
    kDebug(5001);
    if (data.size() == 0)
        return;
    m_data.append(data);
}

void BTTransfer::slotResult(KJob * job)
{
/**    switch (job->error())
    {
        case 0:                      //The download has finished
            QFile torrentFile(KStandardDirs::locateLocal("appdata", "/tmp/" + m_source.fileName()));
            torrentFile.write(m_data);
            torrentFile.close();
            m_source = KStandardDirs::locateLocal("appdata", "/tmp/") + torrentFile.fileName();
            break;
        case KIO::ERR_FILE_ALREADY_EXIST:  //The file has already been downloaded.
            m_data = 0;
    }**/
}

void BTTransfer::slotStoppedByError(bt::TorrentInterface* error, QString errormsg)
{
    kDebug(5001) << errormsg;
}


#include "bttransfer.moc"
