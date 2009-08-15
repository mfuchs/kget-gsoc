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

#include "verifier.h"

#include <QtCore/QFile>
#include <QtGui/QStandardItemModel>
#include <KCodecs>
#include <KLocale>

static const QStringList s_supported = (QStringList() << "sha512" << "sha384" << "sha256" << "ripmed160" << "sha1" << "md5" << "md4");
static const int s_digestLength[] = {128, 96, 64, 40, 40, 32, 32};
static const int s_md5Length = 32;
#ifdef HAVE_QCA2
static QCA::Initializer s_qcaInit;
#endif //HAVE_QCA2

static const QString s_md5 = QString("md5");
static const int s_partSize = 512 * 1024;


VerificationModel::VerificationModel(QObject *parent)
  : QAbstractTableModel(parent)
{
}

QVariant VerificationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || (role != Qt::DisplayRole))
    {
        return QVariant();
    }

    if (index.column() == Type)
    {
        return m_types.at(index.row());
    }
    else if (index.column() == Checksum)
    {
        return m_checksums.at(index.row());
    }

    return QVariant();
}

int VerificationModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_types.length();
}

int VerificationModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return 2;
}

QVariant VerificationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((orientation != Qt::Horizontal) || (role != Qt::DisplayRole))
    {
        return QVariant();
    }

    if (section == Type)
    {
        return i18nc("the type of the hash, e.g. MD5", "Type");
    }
    else if (section == Checksum)
    {
        return i18nc("the used hash for verification", "Hash");
    }

    return QVariant();
}

bool VerificationModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || (row < 0) || (count < 1) || (row + count > rowCount()))
    {
        return false;
    }

    beginRemoveRows(parent, row, row + count - 1);
    while (count)
    {
        m_types.removeAt(row);
        m_checksums.removeAt(row);
        --count;
    }
    endRemoveRows();

    return true;
}

void VerificationModel::addChecksum(const QString &type, const QString &checksum)
{
    if (!type.isEmpty() && !checksum.isEmpty())
    {
        //check if the type is supported and if the diggestLength matches
        bool works = false;

#ifdef HAVE_QCA2
        if (QCA::isSupported(type.toLatin1()) && (s_digestLength[s_supported.indexOf(type)] == checksum.size()))
        {
            works = true;
        }
#endif //HAVE_QCA2

        if ((type == s_md5) && (checksum.size() == s_md5Length))
        {
            works = true;
        }

        if (!works)
        {
            return;
        }


        type.toLower();

        //if the hashtype already exists in the model, then replace it
        int position = m_types.indexOf(type);
        if (position > -1)
        {
            m_checksums[position] = checksum;
            QModelIndex index = this->index(position, Checksum, QModelIndex());
            emit dataChanged(index, index);
            return;
        }

        int rows = rowCount();
        beginInsertRows(QModelIndex(), rows, rows);
        m_types.append(type);
        m_checksums.append(checksum);
        endInsertRows();
    }
}

void VerificationModel::addChecksums(const QHash<QString, QString> &checksums)
{
    QHash<QString, QString>::const_iterator it;
    QHash<QString, QString>::const_iterator itEnd = checksums.constEnd();
    for (it = checksums.constBegin(); it != itEnd; ++it)
    {
        addChecksum(it.key(), it.value());
    }
}

Verifier::Verifier(const KUrl &dest)
  : m_dest(dest),
    m_status(NoResult)
{
    m_model = new VerificationModel();
}

Verifier::~Verifier()
{
    delete m_model;
    qDeleteAll(m_partialSums.begin(), m_partialSums.end());
}

VerificationModel *Verifier::model()
{
    return m_model;
}

QStringList Verifier::supportedVerficationTypes()
{
    QStringList supported;
#ifdef HAVE_QCA2
    QStringList supportedTypes = QCA::Hash::supportedTypes();
    foreach (const QString &type, s_supported)
    {
        if (supportedTypes.contains(type))
        {
            supported << type;
        }
    }
#endif //HAVE_QCA2

    if (!supported.contains(s_md5))
    {
        supported << s_md5;
    }

    supported.sort();
    return supported;

}

int Verifier::diggestLength(const QString &type)
{
#ifdef HAVE_QCA2
    if (QCA::isSupported(type.toLatin1()))
    {
        return s_digestLength[s_supported.indexOf(type)];
    }
#endif //HAVE_QCA2

    if (type == s_md5)
    {
        return s_md5Length;
    }

    return 0;
}

bool Verifier::isChecksum(const QString &type, const QString &checksum)
{
    const int length = diggestLength(type);
    const QString pattern = QString("[0-9a-z]{%1}").arg(length);
    //needs correct length and only word characters
    if (length && (checksum.length() == length) && checksum.toLower().contains(QRegExp(pattern)))
    {
        return true;
    }

    return false;
}

bool Verifier::isVerifyable() const
{
    if (QFile::exists(m_dest.pathOrUrl()) && m_model->rowCount())
    {
        return true;
    }

    return false;
}

bool Verifier::isVerifyable(int row) const
{
    if (QFile::exists(m_dest.pathOrUrl()) && (row >= 0) && (row < m_model->rowCount()))
    {
        return true;
    }
    return false;
}

bool Verifier::verify() const
{
    if (QFile::exists(m_dest.pathOrUrl()))
    {
        QFile file(m_dest.pathOrUrl());
        if (!file.open(QIODevice::ReadOnly))
        {
            return false;
        }

        //check if there is at least one entry
        QModelIndex index = m_model->index(0, 0);
        if (!index.isValid())
        {
            return false;
        }

#ifdef HAVE_QCA2
        QStringList::const_iterator it = s_supported.constBegin();
        QStringList::const_iterator itEnd = s_supported.constEnd();
        for (; it != itEnd; ++it)
        {
            QModelIndexList indexList = m_model->match(index, Qt::DisplayRole, *it);
            //choose the "best" verification type, if it is supported by QCA
            if (!indexList.isEmpty())
            {
                QModelIndex match = m_model->index(indexList.first().row(), VerificationModel::Checksum);
                QCA::Hash hash(*it);
                hash.update(&file);
                QString final = QString(QCA::arrayToHex(hash.final().toByteArray()));
                file.close();
                bool verified = (final == match.data().toString());
                changeStatus(verified);
                return verified;
            }
        }
#endif //HAVE_QCA2

        //use md5 provided by KMD5 as a fallback
        QModelIndexList indexList = m_model->match(index, 0, s_md5);
        if (!indexList.isEmpty())
        {
            QModelIndex match = m_model->index(indexList.first().row(), VerificationModel::Checksum);
            KMD5 hash;
            hash.update(file);
            bool verified = hash.verify(match.data().toString().toLatin1());
            file.close();
            changeStatus(verified);
            return verified;
        }
        file.close();

    }
    return false;
}

bool Verifier::verify(int row) const
{
    if (QFile::exists(m_dest.pathOrUrl()) && (row >= 0) && (row < m_model->rowCount()))
    {
        QFile file(m_dest.pathOrUrl());
        if (!file.open(QIODevice::ReadOnly))
        {
            return false;
        }

        const QString type = m_model->index(row, VerificationModel::Type).data().toString();
        const QString hashString = m_model->index(row, VerificationModel::Checksum).data().toString();
        if (!type.isEmpty() && !hashString.isEmpty())
        {
#ifdef HAVE_QCA2
            QCA::Hash hash(type);
            hash.update(&file);
            QString final = QString(QCA::arrayToHex(hash.final().toByteArray()));
            file.close();
            bool verified = (final == hashString);
            changeStatus(verified);
            return verified;
#endif //HAVE_QCA2
            if (type == s_md5)
            {
                KMD5 hash;
                hash.update(file);
                bool verified = hash.verify(hashString.toLatin1());
                file.close();
                changeStatus(verified);
                return verified;
            }
        }
    }
    return false;
}

void Verifier::changeStatus(bool verified) const
{
    m_status = verified ? Verified : NotVerified;
}

QList<QPair<KIO::fileoffset_t, KIO::filesize_t> > Verifier::brokenPieces() const
{
    QList<QPair<KIO::fileoffset_t, KIO::filesize_t> > broken;

    if (QFile::exists(m_dest.pathOrUrl()))
    {
        QFile file(m_dest.pathOrUrl());
        if (!file.open(QIODevice::ReadOnly))
        {
            return broken;
        }

        QString type;
#ifdef HAVE_QCA2
        for (int i = 0; i < s_supported.size(); ++i)
        {
            if (m_partialSums.contains(s_supported.at(i)))
            {
                type = s_supported.at(i);
            }
        }
#else //NO QCA2
        if (m_partialSums.contains(s_md5))
        {
            type = s_md5;
        }
#endif //HAVE_QCA2

        if (type.isEmpty())
        {
            return broken;
        }

        PartialChecksums *partialChecksums = m_partialSums[type];
        QList<QString> checksums = partialChecksums->checksums();
        const KIO::filesize_t length = partialChecksums->length();
        const KIO::filesize_t fileSize = file.size();

        if (!length || !fileSize)
        {
            return broken;
        }

        for (int i = 0; i < checksums.size(); ++i)
        {
            const QString fileChecksum = calculatePartialChecksum(&file, type, length * i, length, fileSize);
            if (fileChecksum != checksums.at(i))
            {
                broken.append(QPair<KIO::fileoffset_t, KIO::filesize_t>(length * i, length));
            }
        }
    }

    return broken;

}

QString Verifier::checksum(const KUrl &dest, const QString &type)
{
    QStringList supported = supportedVerficationTypes();
    if (!supported.contains(type))
    {
        return QString();
    }

    QFile file(dest.pathOrUrl());
    if (!file.open(QIODevice::ReadOnly))
    {
        return QString();
    }

#ifdef HAVE_QCA2
    QCA::Hash hash(type);
    hash.update(&file);
    QString final = QString(QCA::arrayToHex(hash.final().toByteArray()));
    file.close();
    return final;
#endif //HAVE_QCA2
    if (type == s_md5)
    {
        KMD5 hash;
        hash.update(file);
        QString final = QString(hash.hexDigest());
        file.close();
        return final;
    }

    return QString();
}

PartialChecksums Verifier::partialChecksums(const KUrl &dest, const QString &type)
{
    QList<QString> checksums;

    QStringList supported = supportedVerficationTypes();
    if (!supported.contains(type))
    {
        return PartialChecksums();
    }

    QFile file(dest.pathOrUrl());
    if (!file.open(QIODevice::ReadOnly))
    {
        return PartialChecksums();
    }

    const KIO::filesize_t fileSize = file.size();
    if (!fileSize)
    {
        return PartialChecksums();
    }

    int numPieces = fileSize / s_partSize;
    KIO::fileoffset_t length = fileSize;
    if (numPieces > 100)
    {
        numPieces = 100;
        length = fileSize / numPieces;
    }
    else if (numPieces)
    {
        length = s_partSize;
    }

    //there is a rest, so increase numPieces by one
    if (fileSize % length)
    {
        ++numPieces;
    }

    PartialChecksums partialChecksums;

    //create all the checksums for the pieces
    for (int i = 0; i < numPieces; ++i)
    {
        QString hash = calculatePartialChecksum(&file, type, length * i, length, fileSize);
        if (hash.isEmpty())
        {
            file.close();
            return PartialChecksums();
        }
        checksums.append(hash);
    }

    partialChecksums.setLength(length);
    partialChecksums.setChecksums(checksums);
    file.close();
    return partialChecksums;
}

QString Verifier::calculatePartialChecksum(QFile *file, const QString &type, KIO::fileoffset_t startOffset, int pieceLength, KIO::filesize_t fileSize)
{
    if (!file)
    {
        return QString();
    }

    if (!fileSize)
    {
        fileSize = file->size();
    }
    //longer than the file, so adapt it
    if (static_cast<KIO::fileoffset_t>(fileSize) < startOffset + pieceLength)
    {
        pieceLength = fileSize - startOffset;
    }

#ifdef HAVE_QCA2
    QCA::Hash hash(type);
#else //NO QCA2
    if (type != s_md5)
    {
        return QString();
    }
    KMD5 hash;
#endif //HAVE_QCA2

    //we only read 512kb each time, to save RAM
    int numData = pieceLength / s_partSize;
    KIO::fileoffset_t dataRest = pieceLength % s_partSize;

    if (!numData && !dataRest)
    {
        QString();
    }

    int k = 0;
    for (k = 0; k < numData; ++k)
    {
        if (!file->seek(startOffset + s_partSize * k))
        {
            return QString();
        }

        QByteArray data = file->read(s_partSize);
        hash.update(data);
    }

    //now read the rest
    if (dataRest)
    {
        if (!file->seek(startOffset + s_partSize * k))
        {
            return QString();
        }

        QByteArray data = file->read(dataRest);
        hash.update(data);
    }

#ifdef HAVE_QCA2
    return QString(QCA::arrayToHex(hash.final().toByteArray()));
#else //NO QCA2
    return QString(hash.hexDigest());
#endif //HAVE_QCA2
}

void Verifier::addPartialChecksums(const QString &type, KIO::filesize_t length, const QList<QString> &checksums)
{
    if (!m_partialSums.contains(type) && length && !checksums.isEmpty())
    {
        m_partialSums[type] = new PartialChecksums(length, checksums);
    }
}

KIO::filesize_t Verifier::partialChunkLength() const
{
    QStringList::const_iterator it;
    QStringList::const_iterator itEnd = s_supported.constEnd();
    for (it = s_supported.constBegin(); it != itEnd; ++it)
    {
        if (m_partialSums.contains(*it))
        {
            return m_partialSums[*it]->length();
        }
    }

    return 0;
}

void Verifier::save(const QDomElement &element)
{
    QDomElement e = element;
    e.setAttribute("verificationStatus", m_status);

    QDomElement verification = e.ownerDocument().createElement("verification");
    for (int i = 0; i < m_model->rowCount(); ++i)
    {
        QDomElement hash = e.ownerDocument().createElement("hash");
        hash.setAttribute("type", m_model->index(i, VerificationModel::Type).data().toString());
        QDomText value = e.ownerDocument().createTextNode(m_model->index(i, VerificationModel::Checksum).data().toString());
        hash.appendChild(value);
        verification.appendChild(hash);
    }

    QHash<QString, PartialChecksums*>::const_iterator it;
    QHash<QString, PartialChecksums*>::const_iterator itEnd = m_partialSums.constEnd();
    for (it = m_partialSums.constBegin(); it != itEnd; ++it)
    {
        QDomElement pieces = e.ownerDocument().createElement("pieces");
        pieces.setAttribute("type", it.key());
        pieces.setAttribute("length", (*it)->length());
        QList<QString> checksums = (*it)->checksums();
        for (int i = 0; i < checksums.size(); ++i)
        {
            QDomElement hash = e.ownerDocument().createElement("hash");
            hash.setAttribute("piece", i);
            QDomText value = e.ownerDocument().createTextNode(checksums[i]);
            hash.appendChild(value);
            pieces.appendChild(hash);
        }
        verification.appendChild(pieces);
    }
    e.appendChild(verification);
}

void Verifier::load(const QDomElement &e)
{
    if (e.hasAttribute("verificationStatus"))
    {
        const int status = e.attribute("verificationStatus").toInt();
        switch (status)
        {
            case NoResult:
                m_status = NoResult;
                break;
            case NotVerified:
                m_status = NotVerified;
                break;
            case Verified:
                m_status = Verified;
                break;
            default:
                m_status = NotVerified;
                break;
        }
    }

    QDomElement verification = e.firstChildElement("verification");
    QDomNodeList const hashList = verification.elementsByTagName("hash");

    for (uint i = 0; i < hashList.length(); ++i)
    {
        const QDomElement hash = hashList.item(i).toElement();
        const QString value = hash.text();
        const QString type = hash.attribute("type");
        if (!type.isEmpty() && !value.isEmpty())
        {
            m_model->addChecksum(type, value);
        }
    }

    QDomNodeList const piecesList = verification.elementsByTagName("pieces");

    for (uint i = 0; i < piecesList.length(); ++i)
    {
        QDomElement pieces = piecesList.at(i).toElement();

        const QString type = pieces.attribute("type");
        const KIO::filesize_t length = pieces.attribute("length").toULongLong();
        QList<QString> partialChecksums;

        const QDomNodeList partialHashList = pieces.elementsByTagName("hash");
        for (int i = 0; i < partialHashList.size(); ++i)//TODO give this function the size of the file, to calculate how many hashs are needed as an aditional check, do that check in addPartialChecksums?!
        {
            const QString hash = partialHashList.at(i).toElement().text();
            if (hash.isEmpty())
            {
                break;
            }
            partialChecksums.append(hash);
        }

        addPartialChecksums(type, length, partialChecksums);
    }
}
