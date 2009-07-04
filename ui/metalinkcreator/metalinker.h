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


#ifndef Metalinker_H
#define Metalinker_H

#include <KIO/Job>
#include <KUrl>
#include <QDate>
#include <QDomElement>

#ifdef HAVE_NEPOMUK
namespace Nepomuk
{
    class Variant;
}
#endif //HAVE_NEPOMUK

/**
 * The following classes try to resemble the structure of a Metalink-document, they partially support
 * the Metalink specification version 3.0 2nd ed and Draft 09
 * It is possible to load and save metalinks and to edit them inbetween, you could also create a metalink
 * from scratch
 */

namespace KGetMetalink
{

class DateConstruct
{
    public:
        DateConstruct()
          : negativeOffset(false)
        {
        }

        void setData(const QDateTime &dateTime, const QTime &timeZoneOffset = QTime());
        void setData(const QString &dateConstruct);

        void clear();

        bool isNull() const;
        bool isValid() const;

        QString toString() const;

        QDateTime dateTime;
        QTime timeZoneOffset;
        bool negativeOffset;
};

/**
 * This class contains a url and the name, it can be used to e.g. describe a license
 */
class UrlText
{
    public:
        UrlText() {};

        bool isEmpty() const {return name.isEmpty() && url.isEmpty();}

        void clear();

        QString name;
        KUrl url;
};

/**
* Files, File and Metadata contain this
* Metadata not as member and only for compatibility
*/
class CommonData
{
    public:
        CommonData() {}

        void load(const QDomElement &e);
        void save(QDomElement &e) const;

        void clear();

#ifdef HAVE_NEPOMUK
        /**
         * Return Nepomuk-properties that can be extracted
         */
        QHash<QUrl, Nepomuk::Variant> properties() const;
#endif //HAVE_NEPOMUK

        QString identity;
        QString version;
        QString description;
        QString os;
        KUrl logo;
        QString language;//TODO use different type?
        UrlText publisher;
        QString copyright;
        UrlText license;
};

class Metaurl
{
    public:
        Metaurl() {}

        void load(const QDomElement &e);
        void save(QDomElement &e) const;

        void clear();

        int preference;
        QString type;
        KUrl url;
};

class Url
{
    public:
        Url()
          : maxconnections(0), preference(0)
          {
          }

        bool operator<(const Url &other) const {return (this->preference < other.preference);}//TODO location einbauen mit #include <klocale.h>?//TODO testen ob funzt

        void load(const QDomElement &e);
        void save(QDomElement &e) const;

        void clear();

        /**
        * the maximum connections allowed to that specific server
        * 1 means only one url can be used, default is 0 as in not set
        */
        int maxconnections;

        /**
        * the preference of the urls, 100 is highest priority, 1 lowest
        * default is 0 as in not set
        */
        int preference;

        /**
        * the location of the server eg. "uk"
        */
        QString location;

        KUrl url;
//         QString type;//TODO compatibility??? check if bitorrent and then modify to Metadata?
};

class Resources
{
    public:
        Resources() {}

        bool isValid() const {return !urls.isEmpty() || !metaurls.isEmpty();}

        void load(const QDomElement &e);
        void save(QDomElement &e) const;

        void clear();

        /**
        * the maximum connections allowed to the resources
        * 1 means only one url can be used, default is 0
        */
        int maxconnections;

        QList<Url> urls;
        QList<Metaurl> metaurls;
};

class Pieces
{
    public:
        Pieces()
          : length(0)
        {
        }

        void load(const QDomElement &e);
        void save(QDomElement &e) const;

        void clear();

        QString type;
        KIO::filesize_t length;
        QList<QString> hashes;
};

class Verification
{
    public:
        Verification() {}

        void load(const QDomElement &e);
        void save(QDomElement &e) const;

        void clear();

        QHash<QString, QString> hashes;
        QList<Pieces> pieces;
};

class File
{
    public:
        File()
          : size(0)
        {
        }

        void load(const QDomElement &e);
        void save(QDomElement &e) const;

        void clear();

        bool isValid() const;

#ifdef HAVE_NEPOMUK
        /**
         * Return Nepomuk-properties that can be extracted of file, only including data
         */
        QHash<QUrl, Nepomuk::Variant> properties() const;
#endif //HAVE_NEPOMUK

        QString name;
        Verification verification;
        KIO::filesize_t size;
        CommonData data;
        Resources resources;
};

class Files
{
    public:
        Files() {}

        bool isValid() const;

        void load(const QDomElement &e);
        void save(QDomElement &e) const;

#ifdef HAVE_NEPOMUK
        /**
         * Return all Nepomuk-properties that can be extracted of Files
         * @Note only Files is being looked at, not each File it contains, so
         * you only get the general metadata for all Files
        */
        QHash<QUrl, Nepomuk::Variant> properties() const;
#endif //HAVE_NEPOMUK

        void clear();

        CommonData data;
        QList<File> files;
};

class Metalink
{
    public:
        Metalink() {}

        /**
         * checks if the minimum requirements of a metalink are met
         * @return true if the minimum requirements are met
         */
        bool isValid() const;

        void load(const QDomElement &e);

        /**
         * Save the metalink
         * @return the QDomDocument containing the metalink
         */
        QDomDocument save() const;

        void clear();

        /**
         * Returns true if the metalink is dynamic and if an origin has been set
         */
        bool isDynamic() const;

        static QStringList availableTypes();
        static QStringList availableTypesTranslated();

        QString type;
        QString xmlns; //the xmlns value is ignored when saving, instead the data format described in the specification is always used
        DateConstruct published;
        KUrl origin;
        QString generator;
        DateConstruct updated;
        Files files;
};

/**
 * This class can handle the loading and saving of metalinks on the filesystem
 */
class HandleMetalink
{
    public:
        /**
         * Loads destination into metalink
         * @param destination the place of the metalink in the filesystem
         * @param metalink the instance of Metalink where the metalink will be stored
         * @return return true if it worked
         */
        static bool load(const KUrl &destination, Metalink *metalink);

        /**
         * Loads data into metalink
         * @param data the contents of a metalink
         * @param metalink the instance of Metalink where the metalink will be stored
         * @return return true if it worked
         */
        static bool load(const QByteArray &data, Metalink *metalink);

        /**
         * Saves metalink to destination
         * @param destination the place where the metlink will be saved
         * @param metalink the instance of metalink that will be written to the filesystem
         * @return return true if it worked
         */
        static bool save(const KUrl &destination, Metalink *metalink);

#ifdef HAVE_NEPOMUK
        /**
         * Convenience method to add Strings to the data
         */
        static void addProperty(QHash<QUrl, Nepomuk::Variant> *data, const QByteArray &uriBa, const QString &value);
#endif //HAVE_NEPOMUK
};

}

#endif // Metalinker_H