/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef _BTTRANSFERHANDLER_H
#define _BTTRANSFERHANDLER_H

#include "bttransfer.h"
#include "core/transferhandler.h"
#include <torrent/torrentcontrol.h>

class Scheduler;

class BTAdvancedDetailsWidget;

namespace kt
{
    class ScanDlg;
    class Monitor;
}

class BTTransferHandler : public QObject, public TransferHandler
{
    Q_OBJECT
    public:
        BTTransferHandler(BTTransfer * transfer, Scheduler * scheduler);

        int chunksTotal() const                         {return m_transfer->chunksTotal();}
        int chunksDownloaded() const                    {return m_transfer->chunksDownloaded();}
        int chunksExcluded() const                      {return m_transfer->chunksExcluded();}
        int chunksLeft() const                          {return m_transfer->chunksLeft();}
        int seedsConnected() const                      {return m_transfer->seedsConnected();}
        int seedsDisconnected() const                   {return m_transfer->seedsDisconnected();}
        int leechesConnected() const                    {return m_transfer->leechesConnected();}
        int leechesDisconnected() const                 {return m_transfer->leechesDisconnected();}
        bt::TorrentControl * torrentControl() const     {return m_transfer->torrentControl();}
        bool ready() const                              {return m_transfer->ready();}

        void addTracker(QString url)                    {m_transfer->addTracker(url);}
	kt::Monitor* torrentMonitor() const;

    public slots:
        void createAdvancedDetails();
        void createScanDlg();

    private slots:
        void removeAdvancedDetails();
        void removeScanDlg();

    private:
        BTTransfer * m_transfer;
        BTAdvancedDetailsWidget *advancedDetails;
        kt::ScanDlg *scanDlg;
};

#endif
