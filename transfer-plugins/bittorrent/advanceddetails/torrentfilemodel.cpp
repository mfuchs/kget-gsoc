/** IMPORTANT: please keep this file in sync with ktorrent! ****************/

/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>
#include "torrentfilemodel.h"

namespace kt
{
	TorrentFileModel::TorrentFileModel(bt::TorrentInterface* tc,DeselectMode mode,QObject* parent)
		: QAbstractItemModel(parent),tc(tc),mode(mode)
	{}

	TorrentFileModel::~TorrentFileModel()
	{}
	
	QByteArray TorrentFileModel::saveExpandedState(QTreeView* )
	{
		return QByteArray();
	}
		
	void TorrentFileModel::loadExpandedState(QTreeView* ,const QByteArray &)
	{}

	void TorrentFileModel::missingFilesMarkedDND()
	{
		reset();
	}

	void TorrentFileModel::update()
	{}

}

#include "torrentfilemodel.moc"