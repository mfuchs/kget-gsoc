/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "dlgmultisegkio.h"

#include "multisegkiosettings.h"

#include "kget_export.h"

KGET_EXPORT_PLUGIN_CONFIG(DlgSettingsWidget)

DlgSettingsWidget::DlgSettingsWidget(QWidget *parent, const QVariantList &args)
    : KCModule(KGetFactory::componentData(), parent, args)
{
    ui.setupUi(this);

    connect(ui.numSegSpinBox, SIGNAL(valueChanged(int)), SLOT(changed()));
    connect(ui.minSegSizeSpinBox, SIGNAL(valueChanged(int)), SLOT(changed()));
    connect(ui.saveDataSizeSpinBox, SIGNAL(valueChanged(int)), SLOT(changed()));
    connect(ui.enginesCheckBox, SIGNAL(clicked(bool)), SLOT(changed()));
}

DlgSettingsWidget::~DlgSettingsWidget()
{
}

void DlgSettingsWidget::load()
{
    ui.numSegSpinBox->setValue( MultiSegKioSettings::segments() );
    ui.minSegSizeSpinBox->setValue( MultiSegKioSettings::splitSize() );
    ui.saveDataSizeSpinBox->setValue( MultiSegKioSettings::saveSegSize() );

    ui.enginesCheckBox->setChecked(MultiSegKioSettings::useSearchEngines());
}

void DlgSettingsWidget::save()
{
    kDebug(5001) << "Saving Multithreaded config";
    MultiSegKioSettings::setSegments(ui.numSegSpinBox->value());
    MultiSegKioSettings::setSplitSize(ui.minSegSizeSpinBox->value());
    MultiSegKioSettings::setSaveSegSize(ui.saveDataSizeSpinBox->value());
    MultiSegKioSettings::setUseSearchEngines(ui.enginesCheckBox->isChecked());

    MultiSegKioSettings::self()->writeConfig();
}

#include "dlgmultisegkio.moc"
