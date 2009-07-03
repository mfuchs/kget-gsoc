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

#include "filemodel.h"

#include <KIcon>
#include <KLocale>
#include <KMimeType>


#include <KDebug>

FileItem::FileItem(const QString &name, FileItem *parent)
  : m_name(name),
    m_state(Qt::Checked),
    m_downloadStatus(0),
    m_totalSize(0),
    m_parent(parent)
{
}

FileItem::~FileItem()
{
    qDeleteAll(m_childItems);
}

void FileItem::appendChild(FileItem *child)
{
    m_childItems.append(child);
}

FileItem *FileItem::child(int row)
{
    return m_childItems.value(row);
}

int FileItem::childCount() const
{
    return m_childItems.count();
}

int FileItem::columnCount() const
{
    return 3;
}

QVariant FileItem::data(int column, int role) const
{
    if (column == FileItem::File)
    {
        if (role == Qt::CheckStateRole)
        {
            return m_state;
        }
        else if (role == Qt::DisplayRole)
        {
            return m_name;
        }
        else if (role == Qt::DecorationRole)
        {
            //has no children, so it is a file
            if (!childCount())
            {
                return KIcon(KMimeType::iconNameForUrl(KUrl(m_name)));
            }
            else
            {
                return KIcon("folder");
            }
        }
    }
    else if (column == FileItem::Status)
    {
        if ((role == Qt::DisplayRole) || (role == Qt::DecorationRole))
        {
            if (!childCount())
            {
                return m_downloadStatus;
            }
        }
    }
    else if (column == FileItem::Size)
    {
        if (role == Qt::DisplayRole)
        {
            return KIO::convertSize(m_totalSize);
        }
    }

    return QVariant();
}

bool FileItem::setData(int column, const QVariant &value, FileModel *model, int role)
{
    if (value.isNull())
    {
        return false;
    }

    if (column == FileItem::File)
    {
        if (role == Qt::CheckStateRole)
        {
            m_state = static_cast<Qt::CheckState>(value.toInt());
            model->changeData(this->row(), column, this);
            checkParents(m_state, model);
            checkChildren(m_state, model);
            return true;
        }
        else if (role == Qt::EditRole)
        {
            m_name = value.toString();
            model->changeData(this->row(), column, this);
        }
    }
    else if (column == FileItem::Status)
    {
        if (role == Qt::EditRole)
        {
            if (!childCount())
            {
                m_downloadStatus = value.toInt();
                model->changeData(this->row(), column, this);
            }
        }
    }
    else if (column == FileItem::Size)
    {
        if (role == Qt::EditRole)
        {
            KIO::fileoffset_t newSize = value.toLongLong();
            if (m_parent)
            {
                m_parent->addSize(newSize - m_totalSize, model);
            }
            m_totalSize = newSize;
            model->changeData(this->row(), column, this);
        }
    }

    return false;
}

void FileItem::checkParents(Qt::CheckState state, FileModel *model)
{
    if (!model)
    {
        return;
    }

    if (!m_parent)
    {
        return;
    }

    foreach (FileItem *child, m_parent->m_childItems)
    {
        if (child->m_state != state)
        {
            state = Qt::Unchecked;
            break;
        }
    }

    m_parent->m_state = state;
    model->changeData(m_parent->row(), FileItem::File, m_parent);
    m_parent->checkParents(state, model);
}

void FileItem::checkChildren(Qt::CheckState state, FileModel *model)
{
    if (!model)
    {
        return;
    }

    m_state = state;
    model->changeData(row(), FileItem::File, this);

    foreach (FileItem *child, m_childItems)
    {
        child->checkChildren(state, model);
    }
}

FileItem *FileItem::parent()
{
    return m_parent;
}

int FileItem::row() const
{
    if (m_parent)
    {
        return m_parent->m_childItems.indexOf(const_cast<FileItem*>(this));
    }

    return 0;
}

void FileItem::addSize(KIO::fileoffset_t size, FileModel *model)
{
    if (childCount())
    {
        m_totalSize += size;
        model->changeData(this->row(), FileItem::Size, this);
        if (m_parent)
        {
            m_parent->addSize(size, model);
        }
    }
}


FileModel::FileModel(const QList<KUrl> &files, const KUrl &destDirectory, QObject *parent)
  : QAbstractItemModel(parent),
    m_destDirectory(destDirectory),
    m_checkStateChanged(false)
{
    m_rootItem = new FileItem("root");
    m_header << i18nc("file in a filsystem", "File") << i18nc("status of the download", "Status") << i18nc("size of the download", "Size");

    setupModelData(files);
}

FileModel::~FileModel()
{
    delete m_rootItem;
}

void FileModel::setupModelData(const QList<KUrl> &files)
{
    QString destDirectory = m_destDirectory.pathOrUrl();

    foreach (const KUrl &file, files)
    {
        FileItem *parent = m_rootItem;
        QStringList directories = file.pathOrUrl().remove(destDirectory).split('/', QString::SkipEmptyParts);
        FileItem *child = 0;
        while (directories.count())
        {
            QString part = directories.takeFirst();
            for (int i = 0; i < parent->childCount(); ++i)
            {
                //folder already exists
                if (parent->child(i)->data(0, Qt::DisplayRole).toString() == part)
                {
                    parent = parent->child(i);
                    //file already exists
                    if (!directories.count())
                    {
                        break;
                    }
                    part = directories.takeFirst();
                    i = -1;
                    continue;
                }
            }
            child = new FileItem(part, parent);
            parent->appendChild(child);
            parent = parent->child(parent->childCount() - 1);
        }
        if (child)
        {
            m_files.append(child);
        }
    }
}

int FileModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return static_cast<FileItem*>(parent.internalPointer())->columnCount();
    }
    else
    {
        return m_rootItem->columnCount();
    }
}

QVariant FileModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }


    FileItem *item = static_cast<FileItem*>(index.internalPointer());
    const QVariant data = item->data(index.column(), role);

    //get the status icon as well as status text
    if (index.column() == FileItem::Status)
    {
        int status = data.toInt();
        if (!item->childCount() && m_statusIconText.contains(status))
        {
            if (role == Qt::DisplayRole)
            {
                return m_statusIconText[status].second;
            }
            else if (role == Qt::DecorationRole)
            {
                return m_statusIconText[status].first;
            }
        }
        else
        {
            return QVariant();
        }
    }

    return data;
}

bool FileModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
    {
        return false;
    }

    FileItem *item = static_cast<FileItem*>(index.internalPointer());

    if ((index.column() == FileItem::File) && (role == Qt::CheckStateRole))
    {
        const bool worked = item->setData(index.column(), value, this, role);
        if (worked)
        {
            m_checkStateChanged = true;
        }

        return worked;
    }
    else if ((index.column() == FileItem::Status) && (role == Qt::EditRole))
    {
        const int status = value.toInt();
        if (!m_statusIconText.contains(status))
        {
            return false;
        }
    }

    return item->setData(index.column(), value, this, role);
}

Qt::ItemFlags FileModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return 0;
    }

    if (index.column() == FileItem::File)
    {
        return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant FileModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole))
    {
        return m_header.value(section);
    }

    return QVariant();
}

QModelIndex FileModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    FileItem *parentItem;
    if (parent.isValid())
    {
        parentItem = static_cast<FileItem*>(parent.internalPointer());
    }
    else
    {
        parentItem = m_rootItem;
    }

    FileItem *childItem = parentItem->child(row);
    if (childItem)
    {
        return createIndex(row, column, childItem);
    }
    else
    {
        return QModelIndex();
    }
}

QModelIndex FileModel::index(const KUrl &file, int column)
{
    FileItem *item = getItem(file);
    if (!item)
    {
        return QModelIndex();
    }

    return createIndex(item->row(), column, item);
}

QModelIndexList FileModel::fileIndexes(int column) const
{
    QModelIndexList indexList;
    foreach (FileItem *item, m_files)
    {
        int row = item->row();
        indexList.append(createIndex(row, column, item));
    }

    return indexList;
}

QModelIndex FileModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return QModelIndex();
    }

    FileItem *childItem = static_cast<FileItem*>(index.internalPointer());
    FileItem *parentItem = childItem->parent();
    if ((parentItem == m_rootItem) || (!parentItem))
    {
        return QModelIndex();
    }
    else
    {
        return createIndex(parentItem->row(), 0, parentItem);
    }
}

int FileModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
    {
        return 0;
    }

    FileItem *parentItem;
    if (parent.isValid())
    {
        parentItem = static_cast<FileItem*>(parent.internalPointer());
    }
    else
    {
        parentItem = m_rootItem;
    }

    return parentItem->childCount();
}

void FileModel::changeData(int row, int column, FileItem *item)
{
    QModelIndex index = createIndex(row, column, item);
    emit dataChanged(index, index);
}


void FileModel::setDirectory(const KUrl &newDirectory)
{
    m_destDirectory = newDirectory;
    m_itemCache.clear();
}

KUrl FileModel::getUrl(const QModelIndex &index)
{
    if (!index.isValid() || (index.column() != FileItem::File))
    {
        return KUrl();
    }

    return getUrl(static_cast<FileItem*>(index.internalPointer()));
}

KUrl FileModel::getUrl(FileItem *item)
{
    const QString path = getPath(item);
    const QString name = item->data(FileItem::File, Qt::DisplayRole).toString();
    KUrl url = m_destDirectory;
    url.addPath(path + name);

    return url;
}

QString FileModel::getPath(FileItem *item)
{
    FileItem *parent = item->parent();
    QString path;
    while (parent && parent->parent())
    {
        path = parent->data(FileItem::File, Qt::DisplayRole).toString() + '/' + path;
        parent = parent->parent();
    }

    return path;
}

FileItem *FileModel::getItem(const KUrl &file)
{
    if (m_itemCache.contains(file))
    {
        return m_itemCache[file];
    }

    QString destDirectory = m_destDirectory.pathOrUrl();

    FileItem *item = m_rootItem;
    QStringList directories = file.pathOrUrl().remove(destDirectory).split('/', QString::SkipEmptyParts);
    while (directories.count())
    {
        QString part = directories.takeFirst();
        for (int i = 0; i < item->childCount(); ++i)
        {
            //folder already exists
            if (item->child(i)->data(FileItem::File, Qt::DisplayRole).toString() == part)
            {
                item = item->child(i);
                //file already exists
                if (!directories.count())
                {
                    break;
                }
                part = directories.takeFirst();
                i = -1;
                continue;
            }
        }
    }

    if (item == m_rootItem)
    {
        item = 0;
    }
    else
    {
        m_itemCache[file] = item;
    }

    return item;
}

void FileModel::setStatusIconText(const QHash<int, QPair<KIcon, QString> > &statusIconText)
{
    m_statusIconText = statusIconText;
}

void FileModel::defineFinishedStatus(const QList<int> &finished)
{
    m_finishedStatus = finished;
}

bool FileModel::downloadFinished(KUrl &file)
{
    FileItem *item = getItem(file);
    if (item)
    {
        int status = item->data(FileItem::Status, Qt::DisplayRole).toInt();
        if (m_statusIconText.contains(status) && m_finishedStatus.contains(status))
        {
            return true;
        }
    }

    return false;
}

void FileModel::rename(const QModelIndex &file, const QString &newName)
{
    if (!file.isValid() || (file.column() != FileItem::File))
    {
        return;
    }

    FileItem *item = static_cast<FileItem*>(file.internalPointer());
    //only files can be renamed, no folders
    if (item->childCount())
    {
        return;
    }

    //Find out the old and the new KUrl
    QString oldName = file.data(Qt::DisplayRole).toString();
    QString path = getPath(item);

    KUrl oldUrl = m_destDirectory;
    oldUrl.addPath(path + oldName);
    KUrl newUrl = m_destDirectory;
    newUrl.addPath(path + newName);

    m_itemCache.remove(oldUrl);

    setData(file, newName);

    emit rename(oldUrl, newUrl);
}

void FileModel::renameFailed(const KUrl &beforeRename, const KUrl &afterRename)
{
    Q_UNUSED(beforeRename)
    Q_UNUSED(afterRename)
}

void FileModel::watchCheckState()
{
    m_checkStateChanged = false;
}

void FileModel::stopWatchCheckState()
{
    if (m_checkStateChanged)
    {
        emit checkStateChanged();
    }

    m_checkStateChanged = false;
}

