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

#include "urlwidget.h"
#include "metalinker.h"
#include "../mirror/mirrormodel.h"

#include <QtGui/QSortFilterProxyModel>


UrlWidget::UrlWidget(QObject *parent)
  : QObject(parent),
    m_resources(0),
    m_widget(0)
{
    m_widget = new QWidget;//TODO inherit from qWidget and set this widget as mainwidget?
    ui.setupUi(m_widget);

    m_mirrorModel = new MirrorModel(this);
    ui.used_mirrors->setModel(m_mirrorModel);
    ui.used_mirrors->hideColumn(MirrorItem::Used);
    ui.used_mirrors->hideColumn(MirrorItem::Connections);

    ui.add_mirror->setGuiItem(KStandardGuiItem::add());
    ui.add_mirror->setEnabled(false);
    ui.remove_mirror->setGuiItem(KStandardGuiItem::remove());
    ui.remove_mirror->setEnabled(false);
    connect(ui.url, SIGNAL(textChanged(QString)), this, SLOT(slotUrlChanged(QString)));
    connect(ui.used_mirrors->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(slotUrlClicked()));
    connect(ui.add_mirror, SIGNAL(clicked(bool)), this, SLOT(slotAddMirror()));
    connect(ui.remove_mirror, SIGNAL(clicked(bool)), this, SLOT(slotRemoveMirror()));
}

UrlWidget::~UrlWidget()
{
//     if (m_widget)//NOTE crashes
//     {
//         delete m_widget;
//     }
}

QWidget *UrlWidget::widget()
{
    return m_widget;
}

void UrlWidget::init(KGetMetalink::Resources *resources, QSortFilterProxyModel *countrySort)
{
    if (m_resources)
    {
        m_mirrorModel->removeRows(0, m_mirrorModel->rowCount());
    }

    m_resources = resources;

    foreach (const KGetMetalink::Url &url, m_resources->urls)
    {
        m_mirrorModel->addMirror(url.url, url.preference, url.location);
    }

    //create the country selection
    ui.location->setModel(countrySort);
    ui.location->setCurrentIndex(-1);

    MirrorDelegate *delegate = new MirrorDelegate(countrySort, this);
    ui.used_mirrors->setItemDelegate(delegate);
}

bool UrlWidget::hasUrls() const
{
    return m_mirrorModel->rowCount();
}

void UrlWidget::slotUrlClicked()
{
    QModelIndexList selected = ui.used_mirrors->selectionModel()->selectedRows();
    //     if (selected.count() == 1)//TODO
    //     {
    //         const int row = selected.first().row();
    //         ui.url->setText(m_mirrorModel->data());
    //         ui.num_connections->setValue(m_mirrorModel->)
    //     }
    ui.remove_mirror->setEnabled(!selected.isEmpty());
}

void UrlWidget::slotUrlChanged(const QString &text)
{
    bool enabled = false;
    KUrl url(text);
    if (url.isValid() && !url.protocol().isEmpty() && url.hasPath())
    {
        enabled = true;
    }
    ui.add_mirror->setEnabled(enabled);
}

void UrlWidget::slotAddMirror()
{
    const QString countryCode = ui.location->itemData(ui.location->currentIndex()).toString();
    m_mirrorModel->addMirror(KUrl(ui.url->text()), ui.preference->value(), countryCode);
    ui.used_mirrors->resizeColumnToContents(1);
    ui.used_mirrors->resizeColumnToContents(2);

    emit urlsChanged();
}

void UrlWidget::slotRemoveMirror()
{
    QModelIndexList indexes = ui.used_mirrors->selectionModel()->selectedRows();
    if (indexes.count() == 1)
    {
        m_mirrorModel->removeRow(indexes.first().row());
        emit urlsChanged();
    }
}

void UrlWidget::save()
{
    if (m_resources)
    {
        for (int i = 0; i < m_mirrorModel->rowCount(); ++i)
        {
            KGetMetalink::Url url;
            url.url = KUrl(m_mirrorModel->index(i, MirrorItem::Url).data(Qt::UserRole).toUrl());
            url.preference = m_mirrorModel->index(i, MirrorItem::Preference).data(Qt::UserRole).toInt();
            url.location = m_mirrorModel->index(i, MirrorItem::Country).data(Qt::UserRole).toString();
            m_resources->urls.append(url);
        }
    }
}


#include "urlwidget.moc"