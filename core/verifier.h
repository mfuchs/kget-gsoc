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

#ifndef VERIFIER_H
#define VERIFIER_H

#include <KIO/Job>
#include <KUrl>

#include <QtCore/QHash>
#include <QtCore/QStringList>
#include <QtCore/QAbstractTableModel>
#include <QtGui/QStyledItemDelegate>
#include <QtXml/QDomElement>

#ifdef HAVE_QCA2
#include <QtCrypto>
#endif

#include "kget_export.h"

class QFile;
class QStandardItemModel;
class TransferHandler;

class KGET_EXPORT VerificationDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    public:
        VerificationDelegate(QObject *parent = 0);

        QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        void setEditorData(QWidget *editor, const QModelIndex &index) const;
        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
        void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    private:
        QStringList m_hashTypes;
};

class KGET_EXPORT VerificationModel : public QAbstractTableModel
{
    Q_OBJECT

    public:
        VerificationModel(QObject *parent = 0);

        enum dataType
        {
            Type,
            Checksum
        };

        QVariant data(const QModelIndex &index, int role) const;
        Qt::ItemFlags flags(const QModelIndex &index) const;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
        int rowCount(const QModelIndex &parent = QModelIndex()) const;
        int columnCount(const QModelIndex &parent = QModelIndex()) const;
        bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

        /**
         * Add a checksum that is later used in the verification process
         * @note only one checksum per type can be added (one MD5, one SHA1 etc.),
         * the newer overwrites the older and a checksum can only be added if it is
         * supported by the verifier
         * @param type the type of the checksum
         * @param checksum the checksum
         */
        void addChecksum(const QString &type, const QString &checksum);

        /**
         * Add multiple checksums that will later be used in the verification process
         * @note only one checksum per type can be added (one MD5, one SHA1 etc.),
         * the newer overwrites the older and a checksum can only be added if it is
         * supported by the verifier
         * @param checksums <type, checksum>
         */
        void addChecksums(const QHash<QString, QString> &checksums);

    private:
        QStringList m_types;
        QStringList m_checksums;
};

class KGET_EXPORT PartialChecksums
{
    public:
        PartialChecksums()
          : m_length(0)
        {
        }

        PartialChecksums(KIO::filesize_t len, const QList<QString> &sums)
          : m_length(len), m_checksums(sums)
        {
        }

        bool isValid() const {return (length() && m_checksums.count());}

        KIO::filesize_t length() const {return m_length;}
        void setLength(KIO::filesize_t length) {m_length = length;}

        QList<QString> checksums() const {return m_checksums;}
        void setChecksums(const QList<QString> &checksums) {m_checksums = checksums;}

    private:
        KIO::filesize_t m_length;
        QList<QString> m_checksums;
};

class KGET_EXPORT Verifier
{
    public:
        Verifier(const KUrl &m_dest);
        ~Verifier();

        enum VerificationStatus
        {
            NoResult, //either not tried, or not enough information
            NotVerified,
            Verified
        };

        KUrl destination() const {return m_dest;}
        void setDestination(const KUrl &destination) {m_dest = destination;}

        VerificationStatus status() const {return m_status;}

        /**
         * Returns the supported verification types
         * @return the supported verification types (e.g. MD5, SHA1 ...)
         */
        static QStringList supportedVerficationTypes();

        /**
         * Returns the diggest length of type
         * @param type the checksum type for which to get the diggest length
         * @return the length the diggest should have
         */
        static int diggestLength(const QString &type);

        /**
         * Tries to check if the checksum is a checksum and if it is supported
         * it compares the diggestLength and checks if there are only alphanumerics in checksum
         * @param type of the checksum
         * @param checksum the checksum you want to check
         */
        static bool isChecksum(const QString &type, const QString &checksum);

        /**
         * @note only call verify() when this function returns true
         * @return true if the downloaded file exists and a supported checksum is set
         */
        bool isVerifyable() const;

        /**
         * Convenience function if only a row of the model should be checked
         * @note only call verify() when this function returns true
         * @param row the row in the model of the checksum//TODO use a modelindex instead!
         * @return true if the downloaded file exists and a supported checksum is set
         */
        bool isVerifyable(int row) const;

        /**
         * Verify the downloaded file. This function tries to use the best (most secure)
         * set checksums, deprecated ones like MD4 or MD5 are ignored if possible
         * @note only call this function when isVerifiyable() returns true
         * @return true if the transfer could be verified
         */
        bool verify() const;

        /**
         * Convenience function if only a row of the model should be checked
         * Verify the downloaded file. This function uses the checksum of selected
         * row for verification
         * @note only call this function when isVerifiyable() returns true
         * @return true if the transfer could be verified
         */
        bool verify(int row) const;

        /**
         * Call this method after calling verify() with a negative result, it will
         * return a list of the broken pieces, if PartialChecksums were defined
         * @return fileoffset_t is the offset of the piece, filesize_t the size of the
         * piece, the last piece might have a larger size than it really is
         */
        QList<QPair<KIO::fileoffset_t, KIO::filesize_t> > brokenPieces() const;

        /**
         * Creates the checksum type of the file dest
         */
        static QString checksum(const KUrl &dest, const QString &type);

        /**
         * Create partial checksums of type for file dest
         * @note the length of the partial checksum is not less than 512 kb
         * and there won't be more partial checksums than 101
         */
        static PartialChecksums partialChecksums(const KUrl &dest, const QString &type);

        /**
         * Add partial checksums that can be used as repairinformation
         * @note only one checksum per type can be added (one MD5, one SHA1 etc.),
         * the newer overwrites the older and a checksum can only be added if it is
         * supported by the verifier
         * @param type the type of the checksums
         * @param length the lenght of each piece
         * @param checksums the checksums, first entry is piece number 0
         */
        void addPartialChecksums(const QString &type, KIO::filesize_t length, const QList<QString> &checksums);

        /**
         * Returns the lenght of the "best" partialChecksums
         */
        KIO::filesize_t partialChunkLength() const;

        /**
         * @return the model that stores the hash-types and checksums
         */
        VerificationModel *model();

        void save(const QDomElement &element);
        void load(const QDomElement &e);

    private:
        void changeStatus(bool verified) const;
        static QString calculatePartialChecksum(QFile *file, const QString &type, KIO::fileoffset_t startOffset, int pieceLength, KIO::filesize_t fileSize = 0);

    private:
        VerificationModel *m_model;
        KUrl m_dest;
        mutable VerificationStatus m_status;

        QHash<QString, PartialChecksums*> m_partialSums;

        static const QStringList SUPPORTED;
        static const int DIGGESTLENGTH[];
        static const int MD5LENGTH;
        static const int PARTSIZE;
};

#endif //VERIFIER_H
