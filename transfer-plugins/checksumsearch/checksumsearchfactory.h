/* This file is part of the KDE project

   Copyright (C) 2008 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef CHECKSUMSEARCH_FACTORY_H
#define CHECKSUMSEARCH_FACTORY_H

#include "core/plugin/transferfactory.h"

class Transfer;
class TransferGroup;
class Scheduler;
class TransferDataSource;

class ChecksumSearchFactory : public TransferFactory
{
    Q_OBJECT
    public:
        ChecksumSearchFactory(QObject *parent, const QVariantList &args);
        ~ChecksumSearchFactory();

        Transfer * createTransfer( const KUrl &srcUrl, const KUrl &destUrl,
                                   TransferGroup * parent, Scheduler * scheduler,
                                   const QDomElement * e = 0 );

        TransferHandler * createTransferHandler(Transfer * transfer,
                                                Scheduler * scheduler);

        QWidget * createDetailsWidget( TransferHandler * transfer );

        const QList<KAction *> actions(TransferHandler *handler = 0);

        TransferDataSource * createTransferDataSource(const KUrl &srcUrl, const QDomElement &type);
};

#endif
