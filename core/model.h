/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   Based on:
       kmainwidget.{h,cpp} Copyright (C) 2002 by Patrick Charbonnier
       that was based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef _MODEL_H
#define _MODEL_H


#include <qvaluelist.h>
#include <qstring.h>

#include <kservice.h>

#include "scheduler.h"

class KURL;
class KLibrary;

class Transfer;
class TransferGroup;
class TransferHandler;
class TransferFactory;
class KGetPlugin;
class ModelObserver;

/**
 * This is our Model class. This is where the user's transfers and searches are
 * stored and organized.
 * Use this class from the views to add or remove transfers or searches 
 * In order to organize the transfers inside categories we have a TransferGroup
 * class. By definition, a transfer must always belong to a TransferGroup. If we 
 * don't want it to be displayed by the gui inside a specific group, we will put 
 * it in the group named "Not grouped" (better name?).
 **/

class Model
{
    public:
        static Model& self()
        {
            static Model m;
            return m;
        }

        /**
         * Adds a new observer of the Model. See observer.h for more info about it.
         *
         * @param observer The new observer
         */
        static void addObserver(ModelObserver * observer);

        /**
         * Removes an observer of the Model. See observer.h for more info about it.
         *
         * @param observer The observer to remove
         */
        static void delObserver(ModelObserver * observer);

        /**
         * Adds a new group to the Model.
         *
         * @param groupName The name of the new group
         */
        static void addGroup(const QString& groupName);

        /**
         * Removes a group from the Model.
         *
         * @param groupName The name of the group to be deleted
         */
        static void delGroup(const QString& groupName);

        /**
         * Adds a new transfer to the Model
         *
         * @param srcURL The url to be downloaded
         * @param destDir The destination directory. If empty we show a dialog
         * where the user can choose it.
         * @param groupName The name of the group the new transfer will belong to
         */
        static void addTransfer(KURL srcURL, QString destDir = "",
                                const QString& groupName = "");

        /**
         * Adds new transfers to the Model
         *
         * @param srcURLs The urls to be downloaded
         * @param destDir The destination directory. If empty we show a dialog
         * where the user can choose it.
         * @param groupName The name of the group the new transfer will belong to
         */
        static void addTransfer(KURL::List srcURLs, QString destDir = "",
                                const QString& groupName = "");

        /**
         * Removes a transfer from the Model
         *
         * @param transfer The transfer to be removed
         */
        static void delTransfer(TransferHandler * transfer);

        /**
         * Moves a transfer to a new group
         *
         * @param transfer The transfer to be moved
         * @param groupName The name of the new transfer's group
         */
        static void moveTransfer(TransferHandler * transfer, const QString& groupName);

    private:
        Model();
        ~Model();

        /**
         * Scans for all the available plugins and creates the proper
         * transfer object for the given src url
         *
         * @param src the source url
         * @param dest the destination url
         * @param groupName the group name
         */
        static void createTransfer(KURL src, KURL dest, const QString& groupName = "");

        static KURL urlInputDialog();
        static QString destInputDialog();

        static bool isValidSource(KURL source);
        static bool isValidDestDirectory(const QString& destDir);

        /**
         * if the given url is a file that already exists the function asks
         * the user to confirm the overwriting action
         *
         * @param destFile the url of the destination file
         * @return true if the destination file is ok, otherwise returns false
         */
        static bool isValidDestURL(KURL destURL);

        static KURL getValidDestURL(const QString& destDir, KURL srcURL);

        static TransferGroup * findGroup(const QString& groupName);
        static Transfer * findTransfer(KURL url);

        //Plugin-related functions
        void loadPlugins();
        void unloadPlugins();
        KGetPlugin * createPluginFromService( const KService::Ptr service );


        /**
         * Deletes the given file, if possible.
         *
         * @param url The file to delete
         *
         * @return true if the file was successully deleted: if the given url
         * is a directory or if it is not local it returns false and shows a
         * warning message.
         */
        static bool safeDeleteFile( const KURL& url );


        static QValueList<TransferGroup *> m_transferGroups;
        static QValueList<ModelObserver *> m_observers;

        //Lists of available plugins
        static QValueList<TransferFactory *> m_transferFactories;

        //List of KLibrary objects (used to release the plugins from memory)
        static QValueList<KLibrary *> m_pluginKLibraries;

        //Scheduler object
        static Scheduler m_scheduler;
};


#endif
