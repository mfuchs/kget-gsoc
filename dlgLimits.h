/***************************************************************************
                                dlgLimits.h
                             -------------------
    Revision 				: $Id$
    begin						: Tue Jan 29 2002
    copyright				: (C) 2002 by Patrick Charbonnier
									: Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss
    email						: pch@freeshell.og
 ***************************************************************************/

/***************************************************************************
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 ***************************************************************************/


#ifndef _DLGLIMITS_H
#define _DLGLIMITS_H

#include <qgroupbox.h>
#include <qlabel.h>
#include <qstringlist.h>

#include <knuminput.h>

class DlgLimits:public QGroupBox
{

Q_OBJECT public:

        DlgLimits(QWidget * parent);
        ~DlgLimits()
        {}
        void applyData();
        void setData();

private:

        // opened connections
        QLabel * lb_maxnum;
        KIntNumInput *le_maxnum;

        // minimum bandwidth
        QLabel *lb_minband;
        KIntNumInput *le_minband;

        // maximum bandwidth
        QLabel *lb_maxband;
        KIntNumInput *le_maxband;

};

#endif				// _DLGLIMITS_H
