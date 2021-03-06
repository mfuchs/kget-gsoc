/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef TRANSFER_H
#define TRANSFER_H

#include "job.h"
#include "kget_export.h"

#include <QPixmap>
#include <QTime>

#include <kurl.h>
#include <kio/global.h>

class QDomElement;

class TransferHandler;
class TransferFactory;
class TransferGroup;
class Scheduler;
class TransferTreeModel;
class NepomukHandler;

class KGET_EXPORT Transfer : public Job
{
    friend class TransferHandler;
    friend class TransferTreeModel;

    public:

        /**
         * Here we define the flags that should be shared by all the transfers.
         * A transfer should also be able to define additional flags, in the future.
         */
        enum TransferChange
        {
            Tc_None           = 0x00000000,
            // These flags respect the Model columns order NOTE: The model only checks the last 8 bits, so all values which need to be updated by the model should look like: 0x000000xx
            Tc_FileName       = 0x00000001,
            Tc_Status         = 0x00000002,
            Tc_TotalSize      = 0x00000004,
            Tc_Percent        = 0x00000008,
            Tc_DownloadSpeed  = 0x00000010,
            Tc_RemainingTime  = 0x00000020,
            // Misc
            Tc_UploadSpeed    = 0x00000100,
            Tc_UploadLimit    = 0x00000200,
            Tc_DownloadLimit  = 0x00000400,
            Tc_CanResume      = 0x00000800,
            Tc_DownloadedSize = 0x00001000,
            Tc_UploadedSize   = 0x00002000,
            Tc_Log            = 0x00004000,
            Tc_Group          = 0x00008000,
            Tc_Selection      = 0x00010000
        };

        enum LogLevel
        {
            info,
            warning,
            error
        };

        enum SpeedLimit
        {
            VisibleSpeedLimit   = 0x01,
            InvisibleSpeedLimit = 0x02
        };
        typedef int ChangesFlags;

        Transfer(TransferGroup * parent, TransferFactory * factory,
                 Scheduler * scheduler, const KUrl & src, const KUrl & dest,
                 const QDomElement * e = 0);

        virtual ~Transfer();

        const KUrl & source() const            {return m_source;}
        const KUrl & dest() const              {return m_dest;}

        //Transfer status
        KIO::filesize_t totalSize() const      {return m_totalSize;}
        KIO::filesize_t downloadedSize() const {return m_downloadedSize;}
        KIO::filesize_t uploadedSize() const   {return m_uploadedSize;}
        QString statusText() const             {return m_statusText;}
        QPixmap statusPixmap() const           {return m_statusPixmap;}

        int percent() const                    {return m_percent;}
        int downloadSpeed() const              {return m_downloadSpeed;}
        int uploadSpeed() const                {return m_uploadSpeed;}
        virtual int remainingTime() const      {return KIO::calculateRemainingSeconds(totalSize(), downloadedSize(), downloadSpeed());}
        virtual int elapsedTime() const;

        virtual bool supportsSpeedLimits() const {return false;}

        /**
         * Set the Transfer's UploadLimit
         * @note this is not displayed in any GUI, use setVisibleUploadLimit(int) instead
         * @param visibleUlLimit upload Limit
         */
        void setUploadLimit(int ulLimit, SpeedLimit limit);

        /**
         * Set the Transfer's UploadLimit, which are displayed in the GUI
         * @note this is not displayed in any GUI, use setVisibleDownloadLimit(int) instead
         * @param visibleUlLimit upload Limit
         */
        void setDownloadLimit(int dlLimit, SpeedLimit limit);

        /**
         * @return the UploadLimit, which is invisible in the GUI
         */
        int uploadLimit(SpeedLimit limit) const;

        /**
         * @return the DownloadLimit, which is invisible in the GUI
         */
        int downloadLimit(SpeedLimit limit) const;

        /**
         * Set the maximum share-ratio
         * @param ratio the new maximum share-ratio
         */
        void setMaximumShareRatio(double ratio);

        /**
         * @return the maximum share-ratio
         */
        double maximumShareRatio() {return m_ratio;}

        /**
         * Recalculate the share ratio
         */
        void checkShareRatio();

        // --- Job virtual functions ---
        virtual void setDelay(int seconds);
        virtual void delayTimerEvent();


        bool isSelected() const             {return m_isSelected;}

        /**
         * Transfer history
         */
        const QStringList log() const;

        /**
         * Set Transfer history
         */
         void setLog(const QString& message, LogLevel level = info);

        /**
         * Defines the order between transfers
         */
        bool operator<(const Transfer& t2) const;

        /**
         * The owner group
         */
        TransferGroup * group() const   {return (TransferGroup *) m_jobQueue;}

        /**
         * @return the associated TransferHandler
         */
        TransferHandler * handler();

        /**
         * @returns the TransferTreeModel that owns this group
         */
        TransferTreeModel * model();

        /**
         * @returns a pointer to the TransferFactory object
         */
        TransferFactory * factory() const   {return m_factory;}

#ifdef HAVE_NEPOMUK
        /**
         * Sets the NepomukHandler for the transfer
         * @param handler the new NepomukHandler
         */
        void setNepomukHandler(NepomukHandler *handler) {m_nepomukHandler = handler;}

        /**
         * @returns a pointer to the NepomukHandler of this transfer
         */
        NepomukHandler * nepomukHandler() const {return m_nepomukHandler;}
#endif

        /**
         * Saves this transfer to the given QDomNode
         *
         * @param n The pointer to the QDomNode where the transfer will be saved
         * @return  The created QDomElement
         */
        virtual void save(const QDomElement &element);

    protected:
        //Function used to load and save the transfer's info from xml
        virtual void load(const QDomElement &e);

        /**
         * Sets the Job status to jobStatus, the status text to text and
         * the status pixmap to pix.
         */
        void setStatus(Job::Status jobStatus, const QString &text, const QPixmap &pix);

        /**
         * Makes the TransferHandler associated with this transfer know that
         * a change in this transfer has occurred.
         *
         * @param change: the TransferChange flags to be set
         */
        virtual void setTransferChange(ChangesFlags change, bool postEvent=false);

        /**
         * Function used to set the SpeedLimits to the transfer
         */
        virtual void setSpeedLimits(int uploadLimit, int downloadLimit) {Q_UNUSED(uploadLimit) Q_UNUSED(downloadLimit) }

        // --- Transfer information ---
        KUrl m_source;
        KUrl m_dest;

        QStringList   m_log;
        KIO::filesize_t m_totalSize;
        KIO::filesize_t m_downloadedSize;
        KIO::filesize_t m_uploadedSize;
        int           m_percent;
        int           m_downloadSpeed;
        int           m_uploadSpeed;

        int           m_uploadLimit;
        int           m_downloadLimit;

        bool m_isSelected;

    private:
        int m_visibleUploadLimit;
        int m_visibleDownloadLimit;
        int m_runningSeconds;
        double m_ratio;

        QString m_statusText;
        QPixmap m_statusPixmap;
        QTime m_runningTime;

        TransferHandler * m_handler;
        TransferFactory * m_factory;
#ifdef HAVE_NEPOMUK
        NepomukHandler * m_nepomukHandler;
#endif
};

#endif
