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

#ifndef DLGCHECKSUMSEARCH_H
#define DLGCHECKSUMSEARCH_H

#include "ui_checksumsearch.h"

#include "checksumsearchtransferdatasource.h"

#include <KCModule>

class QStandardItemModel;

class DlgChecksumSettingsWidget : public KCModule
{
    Q_OBJECT

    public:
        explicit DlgChecksumSettingsWidget(QWidget *parent = 0, const QVariantList &args = QVariantList());
        ~DlgChecksumSettingsWidget();

    public slots:
        void save();
        void load();

    private slots:
        /**
         * Adds another row to the model
         * The currently present data of the lineedit and the comboboxes are used for that
         */
        void slotAdd();

        /**
         * Modify the selected item
         */
        void slotModify();

        /**
         * Remove the selected indexes
         */
        void slotRemove();

        /**
         * Enables or disables buttons depending on if the user entered text or not, also changes
         * the label etc.
         */
        void slotUpdate();

        /**
         * Sets m_selectionChanged to true and calls slotUpdateButtons, that way it is
         * clear what function (not) called slotUpdateButtons()
         */
        void slotSelectionChanged();

        /**
         * Updates the text of the label
         */
        void slotUpdateLabel();

    private:
        /**
         * Adds a new item defining how to proceed a search for checksums to the model
         * @param change the string that should change the source url by mode
         * @param mode the change mode, defined in verifier.h, using int instead of enum as convenience
         * @param type the checksum type, like e.g. "md5", empty if you do not know that
         * e.g. if change is "CHECKSUMS" you cannot know which checksums are present
         */
        void addItem(const QString &change, int mode, const QString &type = QString());

    private:
        Ui::ChecksumSearch ui;
        KDialog *m_parent;
        QStandardItemModel *m_model;
        bool m_selectionChanged;
        KUrl m_url;
};

#endif // DLGCHECKSUMSEARCH_H
