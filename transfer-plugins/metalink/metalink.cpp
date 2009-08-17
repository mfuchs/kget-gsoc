/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 Manolo Valdes <nolis71cu@gmail.com>
   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "metalink.h"
#include "metalinksettings.h"
#include "ui_fileselection.h"

#include "core/kget.h"
#include "core/transfergroup.h"
#include "core/download.h"
#include "core/transferdatasource.h"
#include "core/filemodel.h"
#ifdef HAVE_NEPOMUK
#include "metanepomukhandler.h"
#endif //HAVE_NEPOMUK

#include <KIconLoader>
#include <KIO/DeleteJob>
#include <KIO/NetAccess>
#include <KLocale>
#include <KMessageBox>
#include <KDebug>
#include <KDialog>
#include <KStandardDirs>

#include <QDomElement>
#include <QSortFilterProxyModel>

metalink::metalink(TransferGroup * parent, TransferFactory * factory,
                         Scheduler * scheduler, const KUrl & source, const KUrl & dest,
                         const QDomElement * e)
    : Transfer(parent, factory, scheduler, source, dest, e),
      m_fileModel(0),
      m_dialog(0),
      m_currentFiles(0),
      m_ready(false),
      m_averageSpeed(0)
#ifdef HAVE_NEPOMUK
      , m_nepHandler(0)
#endif
{
}

metalink::~metalink()
{
    if (m_dialog)
    {
        delete m_dialog;
    }
}

void metalink::init()
{
#ifdef HAVE_NEPOMUK
    if (!m_nepHandler)
    {
        m_nepHandler = new MetaNepomukHandler(this, 0);
        setNepomukHandler(m_nepHandler);
    }
#endif //HAVE_NEPOMUK

    Transfer::init();
}

void metalink::start()
{
    kDebug(5001) << "metalink::start";

    if (!m_ready)
    {
        if (m_localMetalinkLocation.isValid())
        {
            metalinkInit();
            startMetalink();
        }
        else
        {
            Download *download = new Download(m_source, KStandardDirs::locateLocal("appdata", "metalinks/") + m_source.fileName());

            setStatus(Job::Delayed, i18n("Downloading Metalink File...."), SmallIcon("document-save"));
            setTransferChange(Tc_Status, true);

            connect(download, SIGNAL(finishedSuccessfully(KUrl, QByteArray)), SLOT(metalinkInit(KUrl, QByteArray)));
        }
    }
    else
    {
        startMetalink();
    }
}

void metalink::metalinkInit(const KUrl &src, const QByteArray &data)
{
    kDebug(5001);
    bool justDownloaded = !m_localMetalinkLocation.isValid();
    if (!src.isEmpty())
    {
        m_localMetalinkLocation = src;
    }

    //use the downloaded metalink-file data directly if possible
    if (!data.isEmpty())
    {
        KGetMetalink::HandleMetalink::load(data, &m_metalink);
    }

//TODO wenn nicht valid fehlermeldung und möglichkeit zu ignorieren, aber nur, wenn files vorhanden?
    //try to parse the locally stored metalink-file
    if (!m_metalink.isValid())
    {
        KGetMetalink::HandleMetalink::load(m_localMetalinkLocation.toLocalFile(), &m_metalink);
    }

    //error
    if (!m_metalink.isValid())
    {
        kDebug(5001) << "Unknown error when trying to load the .metalink-file";
        return;
    }

    QList<KGetMetalink::File>::const_iterator it;
    QList<KGetMetalink::File>::const_iterator itEnd = m_metalink.files.files.constEnd();
    m_totalSize = 0;
    KIO::fileoffset_t segSize = 512 * 1024;//TODO use config here!
    const KUrl tempDest = KUrl(m_dest.directory());
    KUrl dest;
    for (it = m_metalink.files.files.constBegin(); it != itEnd ; ++it)
    {
        dest = tempDest;
        dest.addPath((*it).name);

        QList<KGetMetalink::Url> urlList = (*it).resources.urls;
        //sort the urls according to their preference (descending --> "best" first)
        qSort(urlList.begin(), urlList.end(), qGreater<KGetMetalink::Url>());

        KIO::filesize_t fileSize = (*it).size;
        m_totalSize += fileSize;

        //create a DataSourceFactory for each seperate file
        DataSourceFactory *dataFactory = new DataSourceFactory(dest, fileSize, segSize, this);
        dataFactory->setMaxMirrorsUsed(MetalinkSettings::mirrorsPerFile());

#ifdef HAVE_NEPOMUK
        m_nepHandler->setFileMetaData(dest, m_metalink.files, *it);
#endif //HAVE_NEPOMUK

//TODO compare available file size (<size>) with the sizes of the server while downloading?

        connect(dataFactory, SIGNAL(totalSize(KIO::filesize_t)), this, SLOT(totalSizeChanged(KIO::filesize_t)));
        connect(dataFactory, SIGNAL(processedSize(KIO::filesize_t)), this, SLOT(processedSizeChanged()));
        connect(dataFactory, SIGNAL(speed(ulong)), this, SLOT(speedChanged()));
        connect(dataFactory, SIGNAL(statusChanged(DataSourceFactory::Status)), this, SLOT(slotStatus(DataSourceFactory::Status)));

        //add the DataSources
        for (int i = 0; i < urlList.size(); ++i)
        {
            const KUrl url = urlList[i].url;
            if (url.isValid())
            {
                dataFactory->addMirror(url, MetalinkSettings::connectionsPerUrl());
            }
        }
        //no datasource has been created, so remove the datasource factory
        if (dataFactory->mirrors().isEmpty())
        {
            delete dataFactory;
        }
        else
        {
            dataFactory->verifier()->model()->addChecksums((*it).verification.hashes);
            foreach (const KGetMetalink::Pieces &pieces, (*it).verification.pieces)
            {
                dataFactory->verifier()->addPartialChecksums(pieces.type, pieces.length, pieces.hashes);
            }
            m_dataSourceFactory[dataFactory->dest()] = dataFactory;
        }
    }

    if ((m_metalink.files.files.size() == 1) && m_dataSourceFactory.size())
    {
        m_dest = dest;
    }

    if (!m_dataSourceFactory.size())
    {
        KMessageBox::error(0, i18n("Download failed, no working urls were found."), i18n("Error"));
        setStatus(Job::Aborted, i18n("An error occurred...."), SmallIcon("document-preview"));
        setTransferChange(Tc_Status, true);
        return;
    }

    m_ready = !m_dataSourceFactory.isEmpty();

    //the metalink-file has just been downloaded, so ask the user to choose the files that
    // should be downloaded
    if (justDownloaded)
    {
        if (!m_dialog)
        {
            m_dialog = new KDialog;
            QWidget *widget = new QWidget(m_dialog);
            Ui::FileSelection ui;
            ui.setupUi(widget);
            QSortFilterProxyModel *proxy = new QSortFilterProxyModel(this);
            proxy->setSourceModel(fileModel());
            ui.treeView->setModel(proxy);
            ui.treeView->sortByColumn(0, Qt::AscendingOrder);
            ui.treeView->hideColumn(1);
            m_dialog->setMainWidget(widget);
            m_dialog->setCaption(i18n("File Selection"));
            m_dialog->setButtons(KDialog::Ok);
            connect(m_dialog, SIGNAL(finished()), this, SLOT(filesSelected()));
        }
        m_dialog->show();
    }
}

void metalink::filesSelected()
{
    QModelIndexList files = fileModel()->fileIndexes(FileItem::File);
    foreach (const QModelIndex &index, files)
    {
        const KUrl dest = fileModel()->getUrl(index);
        const bool doDownload = index.data(Qt::CheckStateRole).toBool();
        if (m_dataSourceFactory.contains(dest))
        {
            m_dataSourceFactory[dest]->setDoDownload(doDownload);
        }
    }

    //make sure that the size, the downloaded size and the speed gets updated
    totalSizeChanged(0);
    processedSizeChanged();
    speedChanged();

    //some files may be set to download, so start them as long as the transfer is not stopped
    if (status() != Job::Stopped)
    {
        startMetalink();//TODO use bool starttried!! and also store that bool when save!! --> do that also for datasourcefactory!//NOTE not needed to save here, but only in dataFactory?
    }
}

void metalink::startMetalink()
{
    if (m_ready)
    {
        foreach (DataSourceFactory *factory, m_dataSourceFactory)
        {
            //specified number of files is downloaded simultanously
            if (m_currentFiles < MetalinkSettings::simultanousFiles())
            {
                //only start factories that should be downloaded
                if (factory->doDownload() && (factory->status() != DataSourceFactory::Finished)
                    && (factory->status() != DataSourceFactory::Started))
                {
                    ++m_currentFiles;
                    factory->start();
                }
            }
            else
            {
                break;
            }
        }

#ifdef HAVE_NEPOMUK
        m_nepHandler->setDestinations(files());
#endif //HAVE_NEPOMUK
    }
}

void metalink::postDeleteEvent()
{
    if (status() != Job::Finished)//if the transfer is not finished, we delete the written files
    {
        foreach (DataSourceFactory *factory, m_dataSourceFactory)
        {
            factory->postDeleteEvent();
        }
    }//TODO: Ask the user if he/she wants to delete the *.part-file? To discuss (boom1992)
    else
    {
        //in any case delete the files that should not be downloaded
        foreach (DataSourceFactory *factory, m_dataSourceFactory)
        {
            if (!factory->doDownload())
            {
                factory->postDeleteEvent();
            }
        }
    }

    if (m_localMetalinkLocation.isLocalFile())
    {
        KIO::Job *del = KIO::del(m_localMetalinkLocation, KIO::HideProgressInfo);
        KIO::NetAccess::synchronousRun(del, 0);
    }

#ifdef HAVE_NEPOMUK
    m_nepHandler->postDeleteEvent();
#endif //HAVE_NEPOMUK
}

void metalink::stop()
{
    kDebug(5001) << "metalink::Stop";
    if (m_ready && status() != Stopped)
    {
        m_currentFiles = 0;
        foreach (DataSourceFactory *factory, m_dataSourceFactory)
        {
            factory->stop();
        }
    }
}

bool metalink::isResumable() const
{
    return true;
}

void metalink::totalSizeChanged(KIO::filesize_t size)
{
    m_totalSize = 0;
    foreach (DataSourceFactory *factory, m_dataSourceFactory)
    {
        if (factory->doDownload())
        {
            m_totalSize += factory->size();
        }
    }

    if (m_fileModel)
    {
        DataSourceFactory *factory = qobject_cast<DataSourceFactory*>(sender());
        if (factory)
        {
            QModelIndex sizeIndex = m_fileModel->index(factory->dest(), FileItem::Size);
            m_fileModel->setData(sizeIndex, static_cast<qlonglong>(size));
        }
    }

    setTransferChange(Tc_TotalSize, true);
    processedSizeChanged();
}

void metalink::processedSizeChanged()
{
    m_downloadedSize = 0;
    foreach (DataSourceFactory *factory, m_dataSourceFactory)
    {
        if (factory->doDownload())
        {
            m_downloadedSize += factory->downloadedSize();
        }
    }

    if (m_totalSize)
    {
        m_percent = (m_downloadedSize * 100) / m_totalSize;
    }
    else
    {
        m_percent = 0;
    }

    Transfer::ChangesFlags flags = (Tc_DownloadedSize | Tc_Percent);
    setTransferChange(flags, true);
}

void metalink::speedChanged()
{
    m_downloadSpeed = 0;
    foreach (DataSourceFactory *factory, m_dataSourceFactory)
    {
        if (factory->doDownload())
        {
            m_downloadSpeed += factory->currentSpeed();
        }
    }

    setTransferChange(Tc_DownloadSpeed, true);

    //calculate the average of the last three speeds
    static int count = 0;
    static int avgSpeed = 0;
    avgSpeed += m_downloadSpeed;
    ++count;
    if (count == 3)
    {
        m_averageSpeed = avgSpeed / 3;
        count = 0;
        avgSpeed = 0;
    }
}

int metalink::remainingTime() const
{
    if (!m_averageSpeed)
    {
        m_averageSpeed = m_downloadSpeed;
    }
    return KIO::calculateRemainingSeconds(m_totalSize, m_downloadedSize, m_averageSpeed);
}

void metalink::slotStatus(DataSourceFactory::Status status)
{
    ChangesFlags flags = Tc_Status;
    bool changeStatus = true;
    switch (status)
    {
        case DataSourceFactory::Started:
            setStatus(Job::Running, i18n("Downloading...."), SmallIcon("media-playback-start"));
            break;

        case DataSourceFactory::Stopped:
            foreach (DataSourceFactory *factory, m_dataSourceFactory)
            {
                //one factory is still running, do not change the status
                if (factory->doDownload() && (factory->status() == DataSourceFactory::Started))
                {
                    changeStatus = false;
                    break;
                }
            }

            if (changeStatus)
            {
                setStatus(Job::Stopped, i18nc("transfer state: stopped", "Stopped"), SmallIcon("process-stop"));
            }
            break;

        case DataSourceFactory::MovingFile:
            setStatus(Job::Stopped, i18nc("changing the destination of the file", "Changing destination"), SmallIcon("media-playback-pause"));
            break;

        case DataSourceFactory::Finished:
            //one file that has been downloaded now is finished//FIXME ignore downloads that were finished in the previous download!!!!
            if (m_currentFiles)
            {
                --m_currentFiles;
                startMetalink();
            }
            foreach (DataSourceFactory *factory, m_dataSourceFactory)
            {
                //one factory is not finished, do not change the status
                if (factory->doDownload() && (factory->status() != DataSourceFactory::Finished))
                {
                    changeStatus = false;
                    break;
                }
            }

            if (changeStatus)
            {
                setStatus(Job::Finished, i18nc("transfer state: finished", "Finished"), SmallIcon("dialog-ok"));

                //see if some files are NotVerified
                QStringList brokenFiles;
                foreach (DataSourceFactory *factory, m_dataSourceFactory)
                {
                    if (factory->doDownload() && (factory->verifier()->status() == Verifier::NotVerified))
                    {
                        brokenFiles.append(factory->dest().pathOrUrl());
                    }
                }

                if (brokenFiles.count())
                {
                    if (KMessageBox::warningYesNoCancelList(0, i18n("The cownload could not be verified, try to repair it?"), brokenFiles))
                    {
                        repair();
                    }
                }
            }
            break;
    }

    if (m_fileModel)
    {
        DataSourceFactory *factory = qobject_cast<DataSourceFactory*>(sender());
        if (factory)
        {
            QModelIndex statusIndex = m_fileModel->index(factory->dest(), FileItem::Status);
            m_fileModel->setData(statusIndex, status);
        }
    }

    if (changeStatus)
    {
        setTransferChange(flags, true);
    }
}

bool metalink::repair(const KUrl &file)
{
    if (file.isValid())
    {
        if (m_dataSourceFactory.contains(file))
        {
            DataSourceFactory *broken = m_dataSourceFactory[file];
            if (broken->verifier()->status() == Verifier::NotVerified)
            {
                broken->repair();
                return true;
            }
        }
    }
    else
    {
        QList<DataSourceFactory*> broken;
        foreach (DataSourceFactory *factory, m_dataSourceFactory)
        {
            if (factory->doDownload() && (factory->verifier()->status() == Verifier::NotVerified))
            {
                broken.append(factory);
            }
        }
        if (broken.count())
        {
            foreach (DataSourceFactory *factory, broken)
            {
                factory->repair();
            }
            return true;
        }
    }

    return false;
}

void metalink::load(const QDomElement *element)
{
    Transfer::load(element);

    if (!element)
    {
        return;
    }

    const QDomElement e = *element;
    m_localMetalinkLocation = KUrl(e.attribute("LocalMetalinkLocation"));
    QDomNodeList factories = e.firstChildElement("factories").elementsByTagName("factory");

    //no stored information found, stop right here
    if (!factories.count())
    {
        return;
    }

    while (factories.count())
    {
        QDomDocument doc;
        QDomElement factory = doc.createElement("factories");
        factory.appendChild(factories.item(0).toElement());
        doc.appendChild(factory);

        DataSourceFactory *file = new DataSourceFactory(this);
        file->load(&factory);
        if (file->isValid())
        {
            connect(file, SIGNAL(totalSize(KIO::filesize_t)), this, SLOT(totalSizeChanged(KIO::filesize_t)));
            connect(file, SIGNAL(processedSize(KIO::filesize_t)), this, SLOT(processedSizeChanged()));
            connect(file, SIGNAL(speed(ulong)), this, SLOT(speedChanged()));
            connect(file, SIGNAL(statusChanged(DataSourceFactory::Status)), this, SLOT(slotStatus(DataSourceFactory::Status)));
            m_dataSourceFactory[file->dest()] = file;

            //start the DataSourceFactories that were Started when KGet was closed
            if (file->status() == DataSourceFactory::Started)
            {
                if (m_currentFiles < MetalinkSettings::simultanousFiles())
                {
                    ++m_currentFiles;
                    file->start();
                }
                else
                {
                    //enough simultanous files already, so increase the number and set file to stop --> that will decrease the number again
                    ++m_currentFiles;
                    file->stop();
                }
            }
        }
        else
        {
            delete file;
        }
    }
    m_ready = !m_dataSourceFactory.isEmpty();
}

void metalink::save(const QDomElement &element)
{
    Transfer::save(element);

    QDomElement e = element;
    e.setAttribute("LocalMetalinkLocation", m_localMetalinkLocation.url());

    foreach (DataSourceFactory *factory, m_dataSourceFactory)
    {
        factory->save(e);
    }
}

Verifier *metalink::verifier(const KUrl &file)
{
    if (!m_dataSourceFactory.contains(file))
    {
        return 0;
    }

    return m_dataSourceFactory[file]->verifier();
}

QList<KUrl> metalink::files() const
{
    return m_dataSourceFactory.keys();
}

FileModel *metalink::fileModel()
{
    if (!m_fileModel)
    {
        m_fileModel = new FileModel(files(), directory(), this);
        connect(m_fileModel, SIGNAL(rename(KUrl,KUrl)), this, SLOT(slotRename(KUrl,KUrl)));
        connect(m_fileModel, SIGNAL(checkStateChanged()), this, SLOT(filesSelected()));

        QHash<int, QPair<KIcon, QString> > statusIconText;
        statusIconText[DataSourceFactory::Stopped] = QPair<KIcon, QString>(KIcon("process-stop"), i18nc("transfer state: stopped", "Stopped"));
        statusIconText[DataSourceFactory::Started] = QPair<KIcon, QString>(KIcon("media-playback-start"), i18n("Downloading...."));
        statusIconText[DataSourceFactory::MovingFile] = QPair<KIcon, QString>(KIcon("media-playback-pause"), i18nc("changing the destination of the file", "Changing destination"));
        statusIconText[DataSourceFactory::Finished] = QPair<KIcon, QString>(KIcon("dialog-ok"), i18nc("transfer state: finished", "Finished"));
        m_fileModel->setStatusIconText(statusIconText);
        m_fileModel->defineFinishedStatus(QList<int>() << DataSourceFactory::Finished);

        foreach (DataSourceFactory *factory, m_dataSourceFactory)
        {
            QModelIndex size = m_fileModel->index(factory->dest(), FileItem::Size);
            m_fileModel->setData(size, static_cast<qlonglong>(factory->size()));
            QModelIndex status = m_fileModel->index(factory->dest(), FileItem::Status);
            m_fileModel->setData(status, factory->status());
            if (!factory->doDownload())
            {
                QModelIndex index = m_fileModel->index(factory->dest(), FileItem::File);
                m_fileModel->setData(index, Qt::Unchecked, Qt::CheckStateRole);
            }
        }
    }

    return m_fileModel;
}

void metalink::slotRename(const KUrl &oldUrl, const KUrl &newUrl)
{
    if (!m_dataSourceFactory.contains(oldUrl))
    {
        return;
    }

    m_dataSourceFactory[newUrl] = m_dataSourceFactory[oldUrl];
    m_dataSourceFactory.remove(oldUrl);
    m_dataSourceFactory[newUrl]->setNewDestination(newUrl);

#ifdef HAVE_NEPOMUK
    m_nepHandler->setDestinations(files());
#endif //HAVE_NEPOMUK
}

bool metalink::setDirectory(const KUrl &new_directory)
{
    if (new_directory == directory())
    {
        return false;
    }

    if (m_fileModel)
    {
        m_fileModel->setDirectory(new_directory);
    }

    const QString oldDirectory = directory().pathOrUrl(KUrl::AddTrailingSlash);
    const QString newDirectory = new_directory.pathOrUrl(KUrl::AddTrailingSlash);
    const QString fileName = m_dest.fileName();
    m_dest = new_directory;
    m_dest.addPath(fileName);

    QHash<KUrl, DataSourceFactory*> newStorage;
    foreach (DataSourceFactory *factory, m_dataSourceFactory)
    {
        const KUrl oldUrl = factory->dest();
        const KUrl newUrl = KUrl(oldUrl.pathOrUrl().replace(oldDirectory, newDirectory));
        factory->setNewDestination(newUrl);
        newStorage[newUrl] = factory;
    }
    m_dataSourceFactory = newStorage;

#ifdef HAVE_NEPOMUK
    m_nepHandler->setDestinations(files());
#endif //HAVE_NEPOMUK

    return true;
}

QHash<KUrl, QPair<bool, int> > metalink::availableMirrors(const KUrl &file) const
{
    QHash<KUrl, QPair<bool, int> > urls;

    if (m_dataSourceFactory.contains(file))
    {
        urls = m_dataSourceFactory[file]->mirrors();
    }

    return urls;
}


void metalink::setAvailableMirrors(const KUrl &file, const QHash<KUrl, QPair<bool, int> > &mirrors)
{
    if (!m_dataSourceFactory.contains(file))
    {
        return;
    }

    m_dataSourceFactory[file]->setMirrors(mirrors);
}

#include "metalink.moc"
