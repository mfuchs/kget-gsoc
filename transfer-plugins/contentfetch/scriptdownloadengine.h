/* This file is part of the KDE project

   Copyright (C) 2008 Ningyu Shi <shiningyu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#ifndef SCRIPT_DOWNLOAD_ENGINE_H
#define SCRIPT_DOWNLOAD_ENGINE_H

#include <QObject>
#include <QString>
#include <KUrl>

// forward declaration
class TransferGroup;
class Script;

class ScriptDownloadEngine: public QObject
{
    Q_OBJECT
    public:
        ScriptDownloadEngine(QObject *parent, const KUrl &source);
        ~ScriptDownloadEngine(){}
        void setSourceUrl(const QString &url);
    signals:
        void newTransfer(const QString &url);
        void startDownload();
    public slots:
        QString getSourceUrl() const;
        bool addTransfer(const QString &url);
    private:
        QString m_source_url;
};

#endif // SCRIPT_DOWNLOAD_ENGINE_H
