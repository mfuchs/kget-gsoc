/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "btadvanceddetailswidget.h"

#include <interfaces/torrentinterface.h>
#include <interfaces/trackerslist.h>

#include "bttransferhandler.h"
#include "btfiletreeview.h"
#include "bittorrentsettings.h"

#include <kdebug.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kglobal.h>

BTAdvancedDetailsWidget::BTAdvancedDetailsWidget(BTTransferHandler * transfer)
    : m_transfer(transfer)
{
    /**A lot of code is from KTorrent's infowidget by Joris Guisson, thx for the nice work**/
    setupUi(this);

    tc = m_transfer->torrentControl();

    tc->setMonitor(this);

    init();

    transfer->addObserver(this);
    //This updates the widget with the right values
    transferChangedEvent(transfer);
}

BTAdvancedDetailsWidget::~BTAdvancedDetailsWidget()
{
    m_transfer->delObserver(this);
}

void BTAdvancedDetailsWidget::init()
{
    setWindowTitle(i18n("Advanced-Details for %1", m_transfer->source().fileName()));
    const KUrl::List trackers = tc->getTrackersList()->getTrackerURLs();

    BTFileTreeView *fileTreeView = new BTFileTreeView(tc, tabWidget->widget(0));
    tabWidget->widget(0)->layout()->addWidget(fileTreeView);

    if (trackers.empty())
    {
        trackerList->addItem(tc->getTrackersList()->getTrackerURL().prettyUrl());
    }
    else
    {
        foreach (KUrl u,trackers)
            trackerList->addItem(u.prettyUrl());
    }
    updateTracker();

    const bt::TorrentStats & s = tc->getStats();
    totalChunksLabel->setText(QString::number(s.total_chunks));
    sizeChunkLabel->setText(KGlobal::locale()->formatByteSize(s.chunk_size));

    /**Set Column-widths**/

    QList<int> fileColumnWidths = BittorrentSettings::fileColumnWidths();
    if (!fileColumnWidths.isEmpty())
    {
        int j = 0;
        foreach(int i, fileColumnWidths)
        {
            //fileTreeView->setColumnWidth(j, i);
            j++;
        }
    }
    else
    {
        //fileTreeView->setColumnWidth(0 , 250);
    }

    QList<int> peersColumnWidths = BittorrentSettings::peersColumnWidths();
    if (!peersColumnWidths.isEmpty())
    {
        int j = 0;
        foreach(int i, peersColumnWidths)
        {
            peersTreeWidget->setColumnWidth(j, i);
            j++;
        }
    }
    else
    {
        peersTreeWidget->setColumnWidth(0 , 250);
    }

    QList<int> chunksColumnWidths = BittorrentSettings::chunksColumnWidths();
    if (!chunksColumnWidths.isEmpty())
    {
        int j = 0;
        foreach(int i, chunksColumnWidths)
        {
            chunkTreeWidget->setColumnWidth(j, i);
            j++;
        }
    }
    else
    {
        chunkTreeWidget->setColumnWidth(0 , 250);
    }

    connect(deleteTrackerButton, SIGNAL(clicked()), SLOT(deleteTracker()));
    connect(updateTrackerButton, SIGNAL(clicked()), SLOT(updateTracker()));
    connect(addTrackerButton, SIGNAL(clicked()), SLOT(addTracker()));
    connect(changeTrackerButton, SIGNAL(clicked()), SLOT(changeTracker()));
    connect(defaultTrackerButton, SIGNAL(clicked()), SLOT(setDefaultTracker()));
}

void BTAdvancedDetailsWidget::transferChangedEvent(TransferHandler * transfer)
{
    TransferHandler::ChangesFlags transferFlags = m_transfer->changesFlags(this);

    if (transferFlags && Transfer::Tc_Status)
    {
        if (m_transfer->statusText() == "Stopped")
        {
            peersTreeWidget->removeAll();
            chunkTreeWidget->clear();
            items.clear();
        }
        else
        {
            updateChunkView();
            peersTreeWidget->update();
        }
    }

    m_transfer->resetChangesFlags(this);
}

void BTAdvancedDetailsWidget::updateTracker()
{
    kDebug(5001);
    const bt::TorrentStats & s = tc->getStats();

    if (s.running)
    {
        QTime t;
        t = t.addSecs(tc->getTimeToNextTrackerUpdate());
        trackerUpdateTime->setText(t.toString("mm:ss"));
    }

    //Update manual annunce button
    updateTrackerButton->setEnabled(s.running);  // && tc->announceAllowed()
    // only enable change when we can actually change and the torrent is running
    changeTrackerButton->setEnabled(s.running); // && tc->getTrackersList(). > 1

    trackerStatus->setText("<b>" + s.trackerstatus + "</b>");

    if (tc->getTrackersList())
        trackerUrl->setText("<b>" + tc->getTrackersList()->getTrackerURL().prettyUrl() + "</b>");
    else
        trackerUrl->clear();
}

void BTAdvancedDetailsWidget::addTracker(const QString &url)
{
}

void BTAdvancedDetailsWidget::deleteTracker()
{
    kDebug(5001);
    QListWidgetItem* current = trackerList->currentItem();
    if(!current)
        return;
    
    KUrl url(current->text());
    if(tc->getTrackersList()->removeTracker(url))
        delete current;
    else
        KMessageBox::sorry(0, i18n("Cannot remove torrent default tracker."));
}

void BTAdvancedDetailsWidget::setDefaultTracker()
{
    kDebug(5001);
    tc->getTrackersList()->restoreDefault();
    tc->updateTracker();
		
    // update the list of trackers
    trackerList->clear();
		
    const KUrl::List trackers = tc->getTrackersList()->getTrackerURLs();
    if(trackers.empty())
        return;
		
    foreach (KUrl u,trackers)
        trackerList->addItem(u.prettyUrl());
}

void BTAdvancedDetailsWidget::changeTracker()
{
    kDebug(5001);
    QListWidgetItem* current = trackerList->currentItem();
    if(!current)
        return;
		
    KUrl url(current->text());
    tc->getTrackersList()->setTracker(url);
    tc->updateTracker();
}

void BTAdvancedDetailsWidget::peerAdded(bt::PeerInterface* peer)
{
    peersTreeWidget->peerAdded(peer);
}

void BTAdvancedDetailsWidget::peerRemoved(bt::PeerInterface* peer)
{
    peersTreeWidget->peerRemoved(peer);
}

void BTAdvancedDetailsWidget::downloadStarted(bt::ChunkDownloadInterface* chunk)
{
    items.insert(chunk, new ChunkDownloadViewItem(chunkTreeWidget,chunk));
}

void BTAdvancedDetailsWidget::downloadRemoved(bt::ChunkDownloadInterface* chunk)
{
    ChunkDownloadViewItem* v = items.find(chunk);
    if (v)
    {
        items.erase(chunk);
        delete v;
    }
}

void BTAdvancedDetailsWidget::updateChunkView()
{
    bt::PtrMap<bt::ChunkDownloadInterface*,ChunkDownloadViewItem>::iterator i = items.begin();
    while (i != items.end())
    {
        if (i->second)
            i->second->update(false);
        i++;
    }

    const bt::TorrentStats & s = tc->getStats();
    downloadingChunksLabel->setText(QString::number(s.num_chunks_downloading));
    downloadedChunksLabel->setText(QString::number(s.num_chunks_downloaded));
    excludedChunksLabel->setText(QString::number(s.num_chunks_excluded));
    leftChunksLabel->setText(QString::number(s.num_chunks_left));
}

void BTAdvancedDetailsWidget::hideEvent(QHideEvent * event)
{
    kDebug(5001) << "Save config";
    QList<int>  fileColumnWidths;
    for (int i = 0; i<1; i++)
    {
        //fileColumnWidths.append(fileTreeView->columnWidth(i));
    }
    BittorrentSettings::setFileColumnWidths(fileColumnWidths);

    QList<int>  peersColumnWidths;
    for (int i = 0; i<13; i++)
    {
        peersColumnWidths.append(peersTreeWidget->columnWidth(i));
    }
    BittorrentSettings::setPeersColumnWidths(peersColumnWidths);

    QList<int>  chunksColumnWidths;
    for (int i = 0; i<3; i++)
    {
        chunksColumnWidths.append(chunkTreeWidget->columnWidth(i));
    }
    BittorrentSettings::setChunksColumnWidths(chunksColumnWidths);
    BittorrentSettings::self()->writeConfig();
}

#include "btadvanceddetailswidget.moc"
 
