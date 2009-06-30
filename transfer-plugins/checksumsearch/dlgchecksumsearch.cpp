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

#include "dlgchecksumsearch.h"

#include "kget_export.h"

#include "core/verifier.h"

#include "checksumsearch.h"
#include "checksumsearchsettings.h"

#include <QtGui/QStandardItemModel>

KGET_EXPORT_PLUGIN_CONFIG(DlgChecksumSettingsWidget)

DlgChecksumSettingsWidget::DlgChecksumSettingsWidget(QWidget *parent, const QVariantList &args)
  : KCModule(KGetFactory::componentData(), parent, args),
    m_selectionChanged(false),
    m_url(KUrl("http://www.test.com/a_file.zip"))
{
    m_model = new QStandardItemModel(0, 3, this);
    m_model->setHeaderData(0, Qt::Horizontal, i18nc("the string that is used to modify an url", "Change string"));
    m_model->setHeaderData(1, Qt::Horizontal, i18nc("the mode defines how the url should be changed", "Change mode"));
    m_model->setHeaderData(2, Qt::Horizontal, i18nc("the type of the checksum e.g. md5", "Checksum type"));

    ui.setupUi(this);
    ui.treeView->setModel(m_model);
    ui.add->setIcon(KIcon("list-add"));
    ui.remove->setIcon(KIcon("list-remove"));
    slotUpdate();

    QStringList modes = ChecksumSearch::urlChangeModes();
    ui.mode->addItems(modes);
    QStringList types = Verifier::supportedVerficationTypes();
    types.insert(0, QString());
    ui.type->addItems(types);

    connect(ui.add, SIGNAL(clicked()), this, SLOT(slotAdd()));
    connect(ui.modify, SIGNAL(clicked()), this, SLOT(slotModify()));
    connect(ui.remove, SIGNAL(clicked()), this, SLOT(slotRemove()));
    connect(ui.change, SIGNAL(userTextChanged(QString)), this, SLOT(slotUpdate()));
    connect(ui.treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(slotSelectionChanged()));
    connect(ui.mode, SIGNAL(currentIndexChanged(int)), this, SLOT(slotUpdateLabel()));
}

DlgChecksumSettingsWidget::~DlgChecksumSettingsWidget()
{
}

void DlgChecksumSettingsWidget::slotAdd()
{
    addItem(ui.change->text(), ui.mode->currentIndex(), ui.type->currentText());
    changed();
}

void DlgChecksumSettingsWidget::slotModify()
{
    QModelIndexList selected = ui.treeView->selectionModel()->selectedRows();

    //modifying only works for one selected item
    if (selected.count() != 1)
    {
        return;
    }

    int row = selected.first().row();

    const QString change = ui.change->text();
    const QString mode = ui.mode->currentText();
    int index = ui.mode->currentIndex();
    const QString type = ui.type->currentText();

    m_model->setData(m_model->index(row, 0), change);
    m_model->setData(m_model->index(row, 1), mode);
    m_model->setData(m_model->index(row, 1), index, Qt::UserRole);
    m_model->setData(m_model->index(row, 2), type);

    changed();
}

void DlgChecksumSettingsWidget::slotRemove()
{
    QModelIndexList selected = ui.treeView->selectionModel()->selectedRows();
    foreach(QModelIndex index, selected)
    {
        m_model->removeRow(index.row());
    }

    changed();
}

void DlgChecksumSettingsWidget::addItem(const QString &change, int mode, const QString &type)
{
    QStandardItem *item = new QStandardItem(ui.mode->itemText(mode));
    item->setData(QVariant(mode), Qt::UserRole);

    QList<QStandardItem*> items;
    items << new QStandardItem(change);
    items << item;
    items << new QStandardItem(type);
    m_model->insertRow(m_model->rowCount(), items);

    changed();
}

void DlgChecksumSettingsWidget::slotSelectionChanged()
{
    m_selectionChanged = true;

    slotUpdate();
}

void DlgChecksumSettingsWidget::slotUpdate()
{
    ui.add->setEnabled(!ui.change->text().isEmpty());
    ui.remove->setEnabled(ui.treeView->selectionModel()->hasSelection());

    //modify should only work if there is only one selection
    const bool enabled = (ui.treeView->selectionModel()->selectedRows().count() == 1);
    if (enabled && m_selectionChanged)
    {
        int row = ui.treeView->selectionModel()->selectedRows().first().row();
        ui.change->setText(m_model->data(m_model->index(row, 0)).toString());
        ui.mode->setCurrentIndex(m_model->data(m_model->index(row, 1), Qt::UserRole).toInt());
        ui.type->setCurrentItem(m_model->data(m_model->index(row, 2)).toString());

    }
    ui.modify->setEnabled(enabled);

    slotUpdateLabel();

    m_selectionChanged = false;
}

void DlgChecksumSettingsWidget::slotUpdateLabel()
{
    const ChecksumSearch::UrlChangeMode mode = static_cast<ChecksumSearch::UrlChangeMode>(ui.mode->currentIndex());
    const KUrl modifiedUrl = ChecksumSearch::createUrl(m_url, ui.change->text(), mode);
    const QString text = i18n("%1 would become %2", m_url.prettyUrl(), modifiedUrl.prettyUrl());
    ui.label->setText(text);
}

void DlgChecksumSettingsWidget::load()
{
    QStringList changes = ChecksumSearchSettings::self()->searchStrings();
    QList<int> modes = ChecksumSearchSettings::self()->urlChangeModeList();
    QStringList types = ChecksumSearchSettings::self()->checksumTypeList();

    for(int i = 0; i < changes.size(); ++i)
    {
        addItem(changes.at(i), modes.at(i), types.at(i));
    }
}

void DlgChecksumSettingsWidget::save()
{
    kDebug(5001);
    QStringList changes;
    QList<int> modes;
    QStringList types;

    for (int row = 0; row < m_model->rowCount(); ++row)
    {
        changes.append(m_model->data(m_model->index(row, 0)).toString());
        modes.append(m_model->data(m_model->index(row, 1), Qt::UserRole).toInt());
        types.append(m_model->data(m_model->index(row, 2)).toString());
    }

    ChecksumSearchSettings::self()->setSearchStrings(changes);
    ChecksumSearchSettings::self()->setUrlChangeModeList(modes);
    ChecksumSearchSettings::self()->setChecksumTypeList(types);

    ChecksumSearchSettings::self()->writeConfig();
}

#include "dlgchecksumsearch.moc"
