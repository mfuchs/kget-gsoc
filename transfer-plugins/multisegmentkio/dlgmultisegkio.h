/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef DLGMULTISEGKIO_H
#define DLGMULTISEGKIO_H

#include "ui_dlgengineediting.h"
#include "ui_dlgmultisegkio.h"

class DlgEngineEditing : public QDialog
{
    Q_OBJECT

public:
    DlgEngineEditing(QWidget *parent = 0);
    ~DlgEngineEditing();

    QString engineName();
    QString engineUrl();

private:
    Ui::DlgEngineEditing ui;
};

class DlgSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    DlgSettingsWidget(QWidget *parent = 0);
    ~DlgSettingsWidget();

private Q_SLOTS:
    void slotSetSegments(int seg);
    void slotSetUseSearchEngines(bool b);
    void slotNewEngine();
    void slotRemoveEngine();

private:
    void init();
    void addSearchEngineItem(const QString &name, const QString &url);

    void loadSearchEnginesSettings();
    void saveSearchEnginesSettings();

    Ui::DlgMultiSeg ui;
};

#endif // DLGMULTISEGKIO_H