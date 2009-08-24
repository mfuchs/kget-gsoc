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

MirrorAddDlg::MirrorAddDlg(MirrorModel *model, QWidget *parent, Qt::WFlags flags)
  : KDialog(parent, flags),
    m_model(model)
{
    setCaption(i18n("Add mirror"));
    showButtonSeparator(true);
    QWidget *widget = new QWidget(this);
    ui.setupUi(widget);
    setMainWidget(widget);

    updateButton();

    connect(ui.url, SIGNAL(textChanged(const QString&)), this, SLOT(updateButton(QString)));
    connect(this, SIGNAL(accepted()), this, SLOT(addMirror()));
}

void MirrorAddDlg::updateButton(const QString &text)
{
    bool enabled = false;
    KUrl url(text);
    if (url.isValid() && !url.protocol().isEmpty() && url.hasPath())
    {
        enabled = true;
    }
    enableButtonOk(enabled);
}

void MirrorAddDlg::addMirror()
{
    m_model->addMirror(KUrl(ui.url->text()), ui.numConnections->value());
}

MirrorSettings::MirrorSettings(QWidget *parent, TransferHandler *handler, const KUrl &file)
  : KDialog(parent),
    m_transfer(handler),
    m_file(file)
{
    m_model = new MirrorModel(this);
    m_model->setMirrors(m_transfer->availableMirrors(m_file));

    QWidget *widget = new QWidget(this);
    ui.setupUi(widget);
    ui.add->setGuiItem(KStandardGuiItem::add());
    ui.remove->setGuiItem(KStandardGuiItem::remove());
    ui.treeView->setModel(m_model);
    ui.treeView->resizeColumnToContents(MirrorItem::Used);
    ui.treeView->resizeColumnToContents(MirrorItem::Url);
    ui.treeView->hideColumn(MirrorItem::Preference);
    ui.treeView->hideColumn(MirrorItem::Country);
    ui.treeView->setItemDelegate(new MirrorDelegate(this));

    updateButton();

    connect(ui.treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateButton()));
    connect(ui.add, SIGNAL(pressed()), this, SLOT(addPressed()));
    connect(ui.remove, SIGNAL(pressed()), this, SLOT(removeMirror()));
    connect(this, SIGNAL(finished()), this, SLOT(save()));

    resize(700, 400);
    setMainWidget(widget);
    setCaption(i18n("Modify the used mirrors."));
    setButtons(KDialog::Ok);
}

void MirrorSettings::updateButton()
{
    ui.remove->setEnabled(ui.treeView->selectionModel()->hasSelection());
}

void MirrorSettings::addPressed()
{
    MirrorAddDlg *dialog = new MirrorAddDlg(m_model, this);
    dialog->show();
}

void MirrorSettings::removeMirror()
{
    QModelIndexList selected = ui.treeView->selectionModel()->selectedRows();
    foreach(QModelIndex index, selected)
    {
        m_model->removeRow(index.row());
    }
}

void MirrorSettings::save()
{
    m_transfer->setAvailableMirrors(m_file, m_model->availableMirrors());
}

#include "mirrorsettings.moc"
