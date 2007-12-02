/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef BTSETTINGSWIDGET_H
#define BTSETTINGSWIDGET_H

#include <ui_btsettingswidget.h>

#include <QWidget>

class BTSettingsWidget : public QWidget, public Ui::BTSettingsWidget
{
    Q_OBJECT
    public:
        BTSettingsWidget(QWidget * parent);

    private slots:
        void setPort(int port);
        void setUploadLimit(int uploadLimit);
        void setDownloadLimit(int downloadLimit);
        void setDefaultTorrentDir(QString torrentDir);
        void setDefaultTempDir(QString tmpDir);
        void dialogAccepted();
        void init();

    private:
        int m_port;
        int m_uploadLimit;
        int m_downloadLimit;
        QString m_torrentDir;
        QString m_tmpDir;   
};

#endif

