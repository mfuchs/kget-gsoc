/***************************************************************************
*   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>                     *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
***************************************************************************/

#ifndef MIRRORSETTINGS_H
#define MIRRORSETTINGS_H

#include <KDialog>

#include "ui_mirrorsettings.h"
#include "ui_mirroradddlg.h"

class TransferHandler;
class MirrorModel;

class MirrorAddDlg : public KDialog
{
    Q_OBJECT

    public:
        MirrorAddDlg(MirrorModel *model, QWidget *parent = 0, Qt::WFlags flags = 0);

    private slots:
        void addMirror();
        void updateButton(const QString &text = QString());

    private:
        Ui::MirrorAddDlg ui;
        MirrorModel *m_model;
};

class MirrorSettings : public KDialog
{
    Q_OBJECT

    public:
        MirrorSettings(QWidget *parent, TransferHandler *handler, const KUrl &file);

    private slots:
        void updateButton();
        void addPressed();
        void removeMirror();
        void save();

    private:
        TransferHandler *m_transfer;
        KUrl m_file;
        MirrorModel *m_model;
        Ui::MirrorSettings ui;
};

#endif