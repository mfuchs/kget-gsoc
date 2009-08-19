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

#include "mirrorsettings.h"
#include "mirrormodel.h"
#include "core/transferhandler.h"

MirrorSettings::MirrorSettings(QWidget *parent, TransferHandler *handler, const KUrl &file)
  : KDialog(parent),
    m_transfer(handler),
    m_file(file)
{
    m_model = new MirrorModel(this);
    m_model->setMirrors(m_transfer->availableMirrors(m_file));

    MirrorDelegate *delegate = new MirrorDelegate(this);

    QWidget *widget = new QWidget(this);
    ui.setupUi(widget);
    ui.treeView->setModel(m_model);
    ui.treeView->resizeColumnToContents(MirrorItem::Used);
    ui.treeView->resizeColumnToContents(MirrorItem::Url);
    ui.treeView->hideColumn(MirrorItem::Preference);
    ui.treeView->hideColumn(MirrorItem::Country);
    ui.treeView->setItemDelegate(delegate);
    ui.add->setEnabled(false);

    connect(ui.url, SIGNAL(textChanged(const QString&)), this, SLOT(updateButton(QString)));
    connect(ui.add, SIGNAL(pressed()), this, SLOT(addMirror()));
    connect(this, SIGNAL(applyClicked()), this, SLOT(save()));
    connect(this, SIGNAL(okClicked()), this, SLOT(save()));

    resize(700, 400);
    setMainWidget(widget);
    setCaption(i18n("Modify the used mirrors."));
}

MirrorSettings::~MirrorSettings()
{
}

void MirrorSettings::addMirror()
{
    m_model->addMirror(KUrl(ui.url->text()), ui.numConnections->value());
}

void MirrorSettings::save()
{
    m_transfer->setAvailableMirrors(m_file, m_model->availableMirrors());
}

void MirrorSettings::updateButton(const QString &text)
{
    bool enabled = false;
    KUrl url(text);
    if (url.isValid() && !url.protocol().isEmpty() && url.hasPath())
    {
        enabled = true;
    }
    ui.add->setEnabled(enabled);
}

#include "mirrorsettings.moc"
