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

#include "adddlg.h"
#include "checksumsearch.h"

#include <QtGui/QStringListModel>

const KUrl AddDlg::URL = KUrl("http://www.test.com/a_file.zip");

AddDlg::AddDlg(QStringListModel *modesModel, QStringListModel *typesModel, QWidget *parent, Qt::WFlags flags)
  : KDialog(parent, flags),
    m_modesModel(modesModel),
    m_typesModel(typesModel)
{
    setCaption(i18n("Add item"));
    showButtonSeparator(true);
    QWidget *widget = new QWidget(this);
    ui.setupUi(widget);
    setMainWidget(widget);

    if (m_modesModel)
    {
        ui.mode->setModel(m_modesModel);
    }
    if (m_typesModel)
    {
        ui.type->setModel(m_typesModel);
    }

    slotUpdate();

    connect(ui.change, SIGNAL(userTextChanged(QString)), this, SLOT(slotUpdate()));
    connect(ui.mode, SIGNAL(currentIndexChanged(int)), this, SLOT(slotUpdate()));
    connect(this, SIGNAL(accepted()), this, SLOT(slotAccpeted()));
}

void AddDlg::slotUpdate()
{
    enableButtonOk(!ui.change->text().isEmpty());

    const ChecksumSearch::UrlChangeMode mode = static_cast<ChecksumSearch::UrlChangeMode>(ui.mode->currentIndex());
    const KUrl modifiedUrl = ChecksumSearch::createUrl(URL, ui.change->text(), mode);
    const QString text = i18n("%1 would become %2", URL.prettyUrl(), modifiedUrl.prettyUrl());
    ui.label->setText(text);
}

void AddDlg::slotAccpeted()
{
    emit addItem(ui.change->text(), ui.mode->currentIndex(), ui.type->currentText());
}

#include "adddlg.moc"
