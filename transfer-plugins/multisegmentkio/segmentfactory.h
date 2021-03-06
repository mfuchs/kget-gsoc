/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/


#ifndef SEGMENTFACTORY_H
#define SEGMENTFACTORY_H

#include <QtCore/QObject>

#include <kdebug.h>
#include <kio/job.h>

#define	MIN_SIZE	1024*100

    /**
    * class MultiSegData
    */

    class SegData
    {
        public:
            SegData();
            KIO::fileoffset_t offset;
            KIO::filesize_t bytes;
    };

    /**
    * class Segment
    */

    class Segment : public QObject
    {
        Q_OBJECT

    public:

        /**
        * The status property describes the current segment status
        *
        * @param Running The transfer is being executed
        * @param Stopped The transfer is stopped
        * @param Killed The transfer have been killed due unhandled errors 
        * @param Timeout The transfer is broken because an error ocoured
        * @param Finished The transfer exited successfully
        */
        enum Status {Running, Stopped, Killed ,Timeout, Finished};

        /**
        * Empty Constructor
        */
        Segment (QObject* parent);

        /**
        * Create the segment transfer
        * @param url the remote url
        */
        bool createTransfer ( const KUrl &src );

        /**
        * start the segment transfer
        */
        bool startTransfer ( );

        /**
        * stop the segment transfer
        */
        bool stopTransfer ( );

        /**
        * Set the value of m_bytes
        * @param bytes the new value of m_bytes
        */
        void setBytes ( KIO::filesize_t bytes ){ m_segData.bytes = bytes; }

        /**
        * Set the segment data
        * @param data the value of m_segData
        */
        void setData (SegData data ){ m_segData = data; }

        /**
        * Get the segment data
        * @return the value of m_segData
        */
        SegData data ( ){ return m_segData; }

        /**
        * Get the value of m_offset set
        */
        KIO::filesize_t offset ( ){ return m_segData.offset; }

        /**
        * Get the value of m_bytesWritten
        * @return the value of m_bytesWritten
        */
        KIO::filesize_t BytesWritten ( ){ return m_bytesWritten; }

        /**
        * Get the job
        * @return the value of m_getJob
        */
        KIO::TransferJob *job(){ return m_getJob; }

        /**
        * Get the segment status
        * @return the value of m_status
        */
        Status status() const {return m_status;}

    public Q_SLOTS:

        /**
        * Restart the segment transfer
        * @param url the remote url
        */
        bool restartTransfer ( const KUrl &url );

        /**
        * Called whenever a subjob finishes
        * @param job the job that emitted this signal
        */
        void slotResult( KJob *job );

    Q_SIGNALS:
        void data( Segment*, const QByteArray&, bool&);
        void updateSegmentData();
        void statusChanged( Segment*);

    private Q_SLOTS:

        void slotData(KIO::Job *, const QByteArray& data);
	void slotCanResume(KIO::Job *, KIO::filesize_t);

    private:

        bool writeBuffer();
        void setStatus(Status stat, bool doEmit=true);

    private:

        Status m_status;
        SegData m_segData;
        KIO::filesize_t m_bytesWritten;
        KIO::TransferJob *m_getJob;
        QByteArray m_buffer;
	bool m_canResume;
    };

    class SegmentFactory: public QObject
    {
        Q_OBJECT

    public:
        SegmentFactory( uint n, const QList<KUrl> Urls );
        SegmentFactory();
        ~SegmentFactory();
        bool startTransfer ();
        bool stopTransfer ();
        QList<SegData> SegmentsData();
        QList<KUrl> Urls() {return m_Urls;}
        void setUrls(QList<KUrl> &Urls) {m_Urls = Urls;it_Urls=m_Urls.constBegin();}
        QList<Segment *> Segments() {return m_Segments;}
        uint nunOfSegments(){return m_segments;}
        QList<Segment *> splitSegment( Segment *seg, int n );
        Segment *createSegment( SegData data, const KUrl &src );
        void deleteSegment(Segment *);
        const KUrl nextUrl();

    Q_SIGNALS:
        void createdSegment(Segment *);

    private Q_SLOTS:
        void slotStatusChanged( Segment *seg);
        void slotSegmentTimeOut();

    private:
        Segment *takeLongest();
        bool DeleteUrl(const KUrl &url );

    private:
        uint m_segments;
        QList<Segment *> m_Segments;
        QList<Segment *> m_TimeOutSegments;
        QList<KUrl> m_Urls;
        QList<KUrl>::const_iterator it_Urls;
        bool m_split;
    };

#endif // SEGMENTFACTORY_H
