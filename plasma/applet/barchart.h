/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *
 *   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>
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

#ifndef BARCHART_H
#define BARCHART_H

#include "transfergraph.h"

#include <QMap>

namespace Plasma {
    class VBoxLayout;
    class ProgressBar;
    class Label;
}
class QString;

class BarChart : public TransferGraph
{
public:
    BarChart(Plasma::Applet *parent);
    ~BarChart();

    void setTransfers(const QVariantMap &transfers);
    QSizeF contentSizeHint();
    void setVisible(bool visible);

private:
    Plasma::VBoxLayout *m_layout;
    Plasma::Label *m_titleLabel;
    Plasma::Label *m_totalSizeLabel;
    QMap <QString, Plasma::ProgressBar *> m_progressBars;
};

#endif
