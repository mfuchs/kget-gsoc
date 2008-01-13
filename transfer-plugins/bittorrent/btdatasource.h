/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
 
#ifndef KGET_BTDATASOURCE_H
#define KGET_BTDATASOURCE_H

#include "core/transferdatasource.h"

#include <kio/job.h>

class BTDataSource : public TransferDataSource
{
    public:
        BTDataSource();
        ~BTDataSource();

        void start();
	void stop();
        void addSegment(const KUrl &srcUrl, const KIO::fileoffset_t offset, const KIO::fileoffset_t bytes);
        BitSet* bitset() {return m_bitset;}

    private:
        BitSet *m_bitset;
};

#endif
