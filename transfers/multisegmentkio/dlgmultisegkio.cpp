/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include "dlgmultisegkio.h"
#include "MultiSegKioSettings.h"

dlgSettingsWidget::dlgSettingsWidget(QWidget *parent)
: QWidget(parent)
{
   ui.setupUi(this);
   init();
   connect(ui.numSegSpinBox, SIGNAL(valueChanged(int)), SLOT(slotSetSegments(int)));
   connect(ui.enginesCheckBox, SIGNAL(clicked(bool)), SLOT(slotSetUseSearchEngines(bool)));
   connect(ui.urlAddButton, SIGNAL(clicked()), SLOT(slotAddUrl()));
};

dlgSettingsWidget::~dlgSettingsWidget()
{
   MultiSegKioSettings::writeConfig();
}

void dlgSettingsWidget::slotSetSegments(int seg)
{
   MultiSegKioSettings::setSegments(seg);
}

void dlgSettingsWidget::slotSetUseSearchEngines(bool)
{
   MultiSegKioSettings::setUseSearchEngines( ui.enginesCheckBox->isChecked() );
   ui.searchEngineGroupBox->setEnabled( ui.enginesCheckBox->isChecked() );
}

void dlgSettingsWidget::slotAddUrl()
{
   if(ui.engineNameLineEdit->text().isEmpty() || ui.urlLineEdit->text().isEmpty())
      return;
   addSearchEngineItem(ui.engineNameLineEdit->text(), ui.urlLineEdit->text());
}

void dlgSettingsWidget::init()
{
   ui.numSegSpinBox->setValue(MultiSegKioSettings::segments());
   ui.enginesCheckBox->setChecked(MultiSegKioSettings::useSearchEngines());
   ui.searchEngineGroupBox->setEnabled( ui.enginesCheckBox->isChecked() );
}

void dlgSettingsWidget::addSearchEngineItem(const QString &name, const QString &url)
{
   QTableWidgetItem *_nameItem = new QTableWidgetItem(name);
   _nameItem->setFlags(Qt::ItemIsEnabled);
   QTableWidgetItem *_urlItem = new QTableWidgetItem(url);
   _urlItem->setFlags(Qt::ItemIsEnabled);

   int row = ui.enginesTableWidget->rowCount();
   ui.enginesTableWidget->insertRow(row);
   ui.enginesTableWidget->setItem (row, 0, _nameItem);
   ui.enginesTableWidget->setItem (row, 1, _urlItem);
}

#include "dlgmultisegkio.moc"
