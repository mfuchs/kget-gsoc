/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include "transfermultisegkio.h"

#include "multisegkiosettings.h"
#include "core/kget.h"
#include "mirrors.h"

#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

#include <QDomElement>

transferMultiSegKio::transferMultiSegKio(TransferGroup * parent, TransferFactory * factory,
                         Scheduler * scheduler, const KUrl & source, const KUrl & dest,
                         const QDomElement * e)
    : Transfer(parent, factory, scheduler, source, dest, e),
      m_copyjob(0), m_isDownloading(false)
{
    kDebug(5001) << "transferMultiSegKio::transferMultiSegKio";
    if( e )
        load( *e );
}

void transferMultiSegKio::start()
{
    if(!m_copyjob)
        createJob();

    kDebug(5001) << "transferMultiSegKio::start";

    setStatus(Job::Running, i18n("Connecting.."), SmallIcon("connect-creating"));
    setTransferChange(Tc_Status, true);
}

void transferMultiSegKio::stop()
{
    kDebug(5001) << "transferMultiSegKio::Stop()";

    if(status() == Stopped)
        return;

    if(m_copyjob)
    {
        m_copyjob->stop();
    }

    setStatus(Job::Stopped, i18n("Stopped"), SmallIcon("process-stop"));
    m_speed = 0;
    m_isDownloading = false;
    setTransferChange(Tc_Status | Tc_Speed, true);
}

int transferMultiSegKio::elapsedTime() const
{
    return -1; //TODO
}

int transferMultiSegKio::remainingTime() const
{
    return -1; //TODO
}

bool transferMultiSegKio::isResumable() const
{
    return true;
}

void transferMultiSegKio::load(const QDomElement &e)
{
    kDebug(5001) << "TransferMultiSegKio::load";

    SegData d;
    QDomNodeList segments = e.elementsByTagName ("Segment");
    QDomNode node;
    QDomElement segment;
    for( uint i=0 ; i < segments.length () ; ++i )
    {
        node = segments.item(i);
        segment = node.toElement ();
        d.bytes = segment.attribute("Bytes").toULongLong();
        d.offset = segment.attribute("OffSet").toULongLong();
        kDebug(5001) << "TransferMultiSegKio::load: adding Segment " << i;
        SegmentsData << d;
    }
    QDomNodeList urls = e.elementsByTagName ("Urls");
    QDomElement url;
    for( uint i=0 ; i < urls.length () ; ++i )
    {
        node = urls.item(i);
        url = node.toElement ();
        kDebug(5001) << "TransferMultiSegKio::load: adding Url " << i;
        m_Urls << KUrl( url.attribute("Url") );
    }
}

void transferMultiSegKio::save(QDomElement e) // krazy:exclude=passbyvalue
{
    kDebug(5001) << "TransferMultiSegKio::save";

    Transfer::save(e);

    QDomDocument doc(e.ownerDocument());
    QDomElement segment;
    QList<SegData>::iterator it = SegmentsData.begin();
    QList<SegData>::iterator itEnd = SegmentsData.end();
    kDebug(5001) << "TransferMultiSegKio::saving: " << SegmentsData.size() << " segments";
    for ( ; it!=itEnd ; ++it )
    {
        segment = doc.createElement("Segment");
        e.appendChild(segment);
        segment.setAttribute("Bytes", (*it).bytes); 
        segment.setAttribute("OffSet", (*it).offset);
    }
    if( m_Urls.size() > 1 )
    {
        QDomElement url;
        QList<KUrl>::iterator it = m_Urls.begin();
        QList<KUrl>::iterator itEnd = m_Urls.end();
        kDebug(5001) << "TransferMultiSegKio::saving: " << m_Urls.size() << " urls";
        for ( ; it!=itEnd ; ++it )
        {
            url = doc.createElement("Urls");
            e.appendChild(url);
            url.setAttribute("Url", (*it).url()); 
        }
    }
}


//NOTE: INTERNAL METHODS

void transferMultiSegKio::createJob()
{
//     mirror* searchjob = 0;
    if(!m_copyjob)
    {
        if(m_Urls.empty())
        {
            if(MultiSegKioSettings::useSearchEngines())
            {
                MirrorSearch (m_source, this, SLOT(slotSearchUrls(QList<KUrl>&)));
            }
            m_Urls << m_source;
        }
        if(SegmentsData.empty())
        {
            m_copyjob = MultiSegfile_copy( m_Urls, m_dest, -1,  MultiSegKioSettings::segments());
        }
        else
        {
            m_copyjob = MultiSegfile_copy( m_Urls, m_dest, -1, m_processedSize, m_totalSize, SegmentsData, MultiSegKioSettings::segments());
        }
        connect(m_copyjob, SIGNAL(updateSegmentsData()),
           SLOT(slotUpdateSegmentsData()));
        connect(m_copyjob, SIGNAL(result(KJob *)),
           SLOT(slotResult(KJob *)));
        connect(m_copyjob, SIGNAL(infoMessage(KJob *, const QString &)),
           SLOT(slotInfoMessage(KJob *, const QString &)));
        connect(m_copyjob, SIGNAL(percent(KJob *, unsigned long)),
           SLOT(slotPercent(KJob *, unsigned long)));
        connect(m_copyjob, SIGNAL(totalSize(KJob *, qulonglong)),
           SLOT(slotTotalSize(KJob *, qulonglong)));
        connect(m_copyjob, SIGNAL(processedSize(KJob *, qulonglong)),
           SLOT(slotProcessedSize(KJob *, qulonglong)));
        connect(m_copyjob, SIGNAL(speed(KJob *, unsigned long)),
           SLOT(slotSpeed(KJob *, unsigned long)));
    }
}

void transferMultiSegKio::slotUpdateSegmentsData()
{
    SegmentsData.clear();
    SegmentsData = m_copyjob->SegmentsData();
    KGet::save();
}

void transferMultiSegKio::slotResult( KJob *kioJob )
{
    kDebug(5001) << "transferMultiSegKio::slotResult  (" << kioJob->error() << ")";
    switch (kioJob->error())
    {
        case 0:                            //The download has finished
        case KIO::ERR_FILE_ALREADY_EXIST:  //The file has already been downloaded.
            setStatus(Job::Finished, i18n("Finished"), SmallIcon("ok"));
            m_percent = 100;
            m_speed = 0;
            m_processedSize = m_totalSize;
            setTransferChange(Tc_Percent | Tc_Speed);
            break;
        default:
            //There has been an error
            kDebug(5001) << "--  E R R O R  (" << kioJob->error() << ")--";
            setStatus(Job::Aborted, i18n("Aborted"), SmallIcon("process-stop"));
            break;
    }
    // when slotResult gets called, the m_copyjob has already been deleted!
    m_copyjob = 0;
    setTransferChange(Tc_Status, true);
}

void transferMultiSegKio::slotInfoMessage( KJob * kioJob, const QString & msg )
{
    Q_UNUSED(kioJob);
    m_log.append(QString(msg));
}

void transferMultiSegKio::slotPercent( KJob * kioJob, unsigned long percent )
{
//     kDebug(5001) << "transferMultiSegKio::slotPercent";
    Q_UNUSED(kioJob);
    m_percent = percent;
    setTransferChange(Tc_Percent, true);
}

void transferMultiSegKio::slotTotalSize( KJob *kioJob, qulonglong size )
{
    Q_UNUSED(kioJob);

    kDebug(5001) << "transferMultiSegKio::slotTotalSize";

    if (!m_isDownloading)
    {
        setStatus(Job::Running, i18n("Downloading.."), SmallIcon("media-playback-start"));
        m_isDownloading = true;
        setTransferChange(Tc_Status , true);
    }

    m_totalSize = size;
    setTransferChange(Tc_TotalSize, true);
}

void transferMultiSegKio::slotProcessedSize( KJob *kioJob, qulonglong size )
{
//     kDebug(5001) << "slotProcessedSize"; 

    Q_UNUSED(kioJob);

    if (!m_isDownloading)
    {
        setStatus(Job::Running, i18n("Downloading.."), SmallIcon("media-playback-start"));
        m_isDownloading = true;
        setTransferChange(Tc_Status , true);
    }

    m_processedSize = size;
    setTransferChange(Tc_ProcessedSize, true);
}

void transferMultiSegKio::slotSpeed( KJob * kioJob, unsigned long bytes_per_second )
{
//     kDebug(5001) << "slotSpeed: " << bytes_per_second;

    Q_UNUSED(kioJob);

    if (!m_isDownloading)
    {
        setStatus(Job::Running, i18n("Downloading.."), SmallIcon("media-playback-start"));
        m_isDownloading = true;
        setTransferChange(Tc_Status , true);
    }

    m_speed = bytes_per_second;
    setTransferChange(Tc_Speed, true);
}

void transferMultiSegKio::slotSearchUrls(QList<KUrl> &Urls)
{
    kDebug(5001) << "transferMultiSegKio::slotSearchUrls got " << Urls.size() << " Urls.";
    m_Urls = Urls;
    if (m_copyjob)
    {
        m_copyjob->slotUrls(Urls);
    }
}

#include "transfermultisegkio.moc"
