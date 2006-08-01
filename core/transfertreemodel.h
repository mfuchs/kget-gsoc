/* This file is part of the KDE project

   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef _TRANSFERTREEMODEL_H
#define _TRANSFERTREEMODEL_H

#include <QAbstractItemModel>
#include <QList>

class TransferGroup;
class Scheduler;

class TransferTreeModel : public QAbstractItemModel
{
    Q_OBJECT

    friend class TransferGroupHandler;
    friend class TransferGroup;
    friend class TransferHandler;
    friend class Transfer;

    public:
        TransferTreeModel(QList<TransferGroup *> * transferGroups, Scheduler * scheduler);
        ~TransferTreeModel();

        void layoutChanged();

        /**
         * 
         */
        QModelIndex createIndex(int row, int column, void * ptr = 0) const;

        void dataChanged(const QModelIndex & indexFrom, const QModelIndex & indexTo);

        //QAbstractItemModel functions
        int rowCount(const QModelIndex & parent) const;
        int columnCount(const QModelIndex & parent) const;
        Qt::ItemFlags flags (const QModelIndex & index) const;
        QVariant headerData(int section, Qt::Orientation orientation,
                            int role = Qt::DisplayRole) const;
        QVariant data (const QModelIndex & index, int role) const;
        QModelIndex index(int row, int column, const QModelIndex & parent) const;
        QModelIndex parent ( const QModelIndex & index ) const;

    private:
        bool isTransferGroup(void * pointer) const;

        QList<TransferGroup *> * m_transferGroups;
        Scheduler * m_scheduler;
};

#endif
