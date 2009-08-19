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

#ifndef MIRRORMODEL_H
#define MIRRORMODEL_H

#include <QtGui/QStyledItemDelegate>

#include <KIcon>
#include <KUrl>

class QSortFilterProxyModel;

class MirrorDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    public:
        MirrorDelegate(QObject *parent = 0);
        MirrorDelegate(QSortFilterProxyModel *countrySort, QObject *parent = 0);

        QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        void setEditorData(QWidget *editor, const QModelIndex &index) const;
        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
        void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    private:
        QSortFilterProxyModel *m_countrySort;
};

class MirrorItem
{
    public:
        MirrorItem();

        enum DataType
        {
            Used = 0,
            Url,
            Connections,
            Preference,
            Country
        };

        QVariant data(int column, int role = Qt::DisplayRole) const;
        Qt::ItemFlags flags(int column) const;
        bool setData(int column, const QVariant &value, int role = Qt::EditRole);

    private:
        KUrl m_url;
        Qt::CheckState m_checked;
        int m_numConnections;
        int m_preference;
        QString m_countryCode;
        QString m_countryName;
        KIcon m_countryFlag;
};

class MirrorModel : public QAbstractTableModel
{
    Q_OBJECT

    public:
        MirrorModel(QObject *parent);
        ~MirrorModel();

        int rowCount(const QModelIndex &index = QModelIndex()) const;
        int columnCount(const QModelIndex &index = QModelIndex()) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
        Qt::ItemFlags flags(const QModelIndex &index) const;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
        bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

        void addMirror(const KUrl &url, int preference, const QString &countryCode);
        void addMirror(const KUrl &url, int numConnecitons);
        void setMirrors(const QHash<KUrl, QPair<bool, int> > &mirrors);
        QHash<KUrl, QPair<bool, int> > availableMirrors() const;

    private:
        void addMirror(const KUrl &url, int numConnecitons, int preference, const QString &countryCode);

    private:
        QList<MirrorItem*> m_data;
};

#endif
