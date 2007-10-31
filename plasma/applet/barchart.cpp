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

#include "barchart.h"

#include <KIcon>
#include <KLocale>
#include <KGlobal>

#include <QObject>
#include <QSizeF>
#include <QPainter>
#include <QPalette>
#include <QApplication>
#include <QAbstractItemView>

BarChart::BarChart(QObject *parent) 
    : TransferGraph(parent)
{
}

void BarChart::paint(QPainter *p, const QRect &contentsRect)
{
    p->save();
    p->setPen(Qt::white);
    uint y = (uint) VERTICAL_MARGIN - 2;
    uint totalSize = 0;

    TransferGraph::drawTitle(p, contentsRect);

    foreach(QString url, m_transfers.keys()) {
        QVariantList attributes = m_transfers[url].toList();
        float percent = attributes[1].toString().toFloat();
        p->save();

        QRect transferRect(contentsRect.x() + HORIZONTAL_MARGIN,
                            contentsRect.y() + y,
                            contentsRect.width() - HORIZONTAL_MARGIN * 2,
                            TRANSFER_LINE_HEIGHT - 7);

        // following progressbar code has mostly been taken from Qt4 examples/network/torrent/mainview.cpp and the kget/ui/transfersviewdelegate.cpp
        // Set up a QStyleOptionProgressBar to precisely mimic the
        // environment of a progress bar.
        QStyleOptionProgressBar progressBarOption;
        progressBarOption.state = QStyle::State_Enabled;
        progressBarOption.direction = QApplication::layoutDirection();
        progressBarOption.rect = transferRect;
        progressBarOption.fontMetrics = QApplication::fontMetrics();
        progressBarOption.minimum = 0;
        progressBarOption.maximum = 100;
        progressBarOption.textAlignment = Qt::AlignRight;
        progressBarOption.textVisible = true;

        // Set the progress and text values of the style option.
        progressBarOption.progress = percent;
        progressBarOption.text = QString().sprintf("%d%%   ", progressBarOption.progress);

        progressBarOption.rect.setY(progressBarOption.rect.y() +
                                        (transferRect.height() - QApplication::fontMetrics().height()) / 2);
        progressBarOption.rect.setHeight(QApplication::fontMetrics().height());

            // Draw the progress bar onto the view.
        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, p);

        p->setPen(TRANSFER_NAME_COLOR);
        p->drawText((int) transferRect.x() + HORIZONTAL_MARGIN, (int) transferRect.y() + 5,
                    contentsRect.width() - HORIZONTAL_MARGIN * 2, TRANSFER_LINE_HEIGHT - 10,
                    Qt::AlignLeft,
                    p->fontMetrics().elidedText(attributes[0].toString(),
                            Qt::ElideLeft,  contentsRect.width() - HORIZONTAL_MARGIN * 2 - 140) +
                    " (" + KGlobal::locale()->formatByteSize(attributes[2].toInt()) + ')');

        p->restore();
        y += TRANSFER_LINE_HEIGHT;
        totalSize += attributes[2].toInt();
    }

    // draw a line under the transfers downloads graphs
    p->drawLine(contentsRect.x() + HORIZONTAL_MARGIN, contentsRect.y() + y + 5,
            contentsRect.width() - HORIZONTAL_MARGIN, contentsRect.y() + y + 5);
    // draw the totalSize legend
    p->setPen(QColor(220, 220, 220));
    p->drawText(contentsRect.x(), (int) contentsRect.y() + y + 7,
            contentsRect.width() - HORIZONTAL_MARGIN * 2, TRANSFER_LINE_HEIGHT - 10,
            Qt::AlignRight, i18n("Total size: %1", KGlobal::locale()->formatByteSize(totalSize)));
    p->restore();
}

QSizeF BarChart::contentSizeHint()
{
    return QSizeF(TransferGraph::contentSizeHint().width(),
                    TransferGraph::contentSizeHint().height() + VERTICAL_MARGIN * 2);
}
