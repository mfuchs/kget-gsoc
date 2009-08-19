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

#ifndef ADDDLG_H
#define ADDDLG_H

#include <KDialog>

#include "ui_adddlg.h"

class QStringListModel;

class AddDlg : public KDialog
{
    Q_OBJECT

    public:
        AddDlg(QStringListModel *modesModel, QStringListModel *typesModel, QWidget *parent = 0, Qt::WFlags flags = 0);

    signals:
        /**
         * Emitted when the dialog gets accepted
         * @param change the string that should change the source url by mode
         * @param mode the change mode
         * @param type the checksum type, can be an empty string
         */
        void addItem(const QString &change, int mode, const QString &type);

    private slots:
        /**
         * Enables or disables buttons depending on if the user entered text or not, also changes
         * the label etc.
         */
        void slotUpdate();

        void slotAccpeted();

    private:
        Ui::AddDlg ui;

        QStringListModel *m_modesModel;
        QStringListModel *m_typesModel;

        static const KUrl URL;
};


#endif
