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

#include "speedgraph.h"
#include "linegraphwidget.h"

#include <QGraphicsLinearLayout>
#include <QGraphicsWidget>

#include <plasma/applet.h>

SpeedGraph::SpeedGraph(QGraphicsWidget *parent)
    : TransferGraph(0)
{
    m_layout = static_cast <QGraphicsLinearLayout *> (parent->layout());
    if (m_layout)
    {
        m_lineGraph = new LineGraphWidget(0);
        m_layout->addItem(m_lineGraph);

        QObject::connect(m_lineGraph, SIGNAL(geometryChanged()), SLOT(updateGeometry()));
    }
}

SpeedGraph::~SpeedGraph()
{
    m_layout->removeItem(m_lineGraph);
    delete m_lineGraph;
}

void SpeedGraph::updateGeometry()
{
    kDebug() << "About to update the widget geometry " << endl;
    //m_applet->updateGeometry();
}

void SpeedGraph::setTransfers(const QVariantMap &percents)
{
    // drop the deleted transfers
    foreach (const QString &key, m_transfers.keys()) {
        if (!percents.contains(key)) {
            m_lineGraph->removeData(key);
        }
    }

    TransferGraph::setTransfers(percents);

    // a map with the name of the transfer and the speed
    QMap <QString, int> data;

    foreach(const QString &name, percents.keys()) {
        data [name] = percents [name].toList().at(4).toInt();
    }

    m_lineGraph->addData(data);

    m_lineGraph->updateView();
}

