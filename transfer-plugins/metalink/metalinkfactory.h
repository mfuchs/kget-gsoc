/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef METALINK_FACTORY_H
#define METALINK_FACTORY_H

#include "core/plugin/transferfactory.h"

class Transfer;
class TransferGroup;
class Scheduler;

class metalinkFactory : public TransferFactory
{
    Q_OBJECT
    public:
        metalinkFactory(QObject *parent, const QVariantList &args);
        ~metalinkFactory();

        Transfer * createTransfer( const KUrl &srcUrl, const KUrl &destUrl,
                                   TransferGroup * parent, Scheduler * scheduler,
                                   const QDomElement * e = 0 );

        TransferHandler * createTransferHandler(Transfer * transfer,
                                                Scheduler * scheduler);

        QWidget * createDetailsWidget( TransferHandler * transfer );

        QWidget * createSettingsWidget(KDialog * parent);

        QString displayName(){return "MetaLink";}

        const QList<KAction *> actions(TransferHandler *handler = 0);
        TransferDataSource * createTransferDataSource(const KUrl &srcUrl) {Q_UNUSED(srcUrl); return 0;}
};

#endif
