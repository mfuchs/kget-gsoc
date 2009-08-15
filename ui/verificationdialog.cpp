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

#include "verificationdialog.h"

#include <QtGui/QStandardItemModel>

#include <KLocale>
#include <KMessageBox>

#include "core/filemodel.h"
#include "core/transferhandler.h"
#include "core/verifier.h"

VerificationDialog::VerificationDialog(QWidget *parent, TransferHandler *transfer, const KUrl &file)
  : KDialog(parent),
    m_transfer(transfer),
    m_file(file),
    m_verifier(transfer->verifier(m_file)),
    m_model(0),
    m_fileModel(0)
{
    if (m_verifier)
    {
        m_model = m_verifier->model();
    }

    setCaption(i18n("Transfer Verification for %1", m_file.fileName()));
    showButtonSeparator(true);
    QWidget *widget = new QWidget(this);
    ui.setupUi(widget);
    setMainWidget(widget);
    ui.add->setGuiItem(KStandardGuiItem::add());
    ui.add->setEnabled(false);
    ui.remove->setGuiItem(KStandardGuiItem::remove());
    ui.remove->setEnabled(false);
    ui.verify->setEnabled(false);
    ui.hashTypes->addItems(Verifier::supportedVerficationTypes());

    if (m_model)
    {
        ui.usedHashes->setModel(m_model);

        m_fileModel = m_transfer->fileModel();

        connect(m_model, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(updateButtons()));
        connect(ui.usedHashes, SIGNAL(clicked(const QModelIndex&)), this, SLOT(updateButtons()));
        connect(ui.newHash, SIGNAL(userTextChanged(const QString &)), this, SLOT(updateButtons()));
        connect(ui.hashTypes, SIGNAL(currentIndexChanged(int)), this, SLOT(updateButtons()));
        connect(ui.add, SIGNAL(pressed()), this, SLOT(addPressed()));
        connect(ui.remove, SIGNAL(pressed()), this, SLOT(removePressed()));
        connect(ui.verify, SIGNAL(pressed()), this, SLOT(verifyPressed()));
    }

    setButtons(KDialog::Ok);
}

VerificationDialog::~VerificationDialog()
{
}

void VerificationDialog::updateButtons()
{
    const QString type = ui.hashTypes->currentText();
    const QString hash = ui.newHash->text();
    bool enableAdd = !hash.isEmpty() && !type.isEmpty();
    if (enableAdd)
    {
        if (!m_diggestLength.contains(type))
        {
            m_diggestLength[type] = Verifier::diggestLength(type);
        }

        enableAdd &= (hash.length() == m_diggestLength[type]);
        if (enableAdd)
        {
            enableAdd = Verifier::isChecksum(type, hash);
        }
    }

    ui.add->setEnabled(enableAdd);
    ui.remove->setEnabled(ui.usedHashes->selectionModel()->selectedIndexes().count());

    //check if the download finished and if the selected indexes are verifyable
    bool verifyEnabled = false;
    if (m_fileModel && m_fileModel->downloadFinished(m_file))
    {
        const QModelIndexList indexes = ui.usedHashes->selectionModel()->selectedRows();
        if (indexes.count())
        {
            foreach (const QModelIndex &index, indexes)
            {
                verifyEnabled = m_verifier->isVerifyable(index.row());
                if (!verifyEnabled)
                {
                    break;
                }
            }
        }
    }
    ui.verify->setEnabled(verifyEnabled);
}

void VerificationDialog::removePressed()
{
    const QModelIndexList indexes = ui.usedHashes->selectionModel()->selectedRows();
    foreach (const QModelIndex &index, indexes)
    {
        m_model->removeRow(index.row());
    }
    updateButtons();
}

void VerificationDialog::addPressed()
{
    m_model->addChecksum(ui.hashTypes->currentText(), ui.newHash->text());
}

void VerificationDialog::verifyPressed()
{
    const QModelIndexList indexes = ui.usedHashes->selectionModel()->selectedRows();
    bool verified = false;
    foreach (const QModelIndex &index, indexes)
    {
        verified = m_verifier->verify(index.row());
        if (!verified)
        {
            break;
        }
    }

    if (verified)
    {
        KMessageBox::information(this, i18n("The downloaded file was successfully verified."), i18n("Verification successful"));
    }
    else if (KMessageBox::warningYesNo(this, i18n("The downloaded file could not be verified. Do you want to repair it?"), i18n("Verification failed")))
    {
        m_transfer->repair(m_file);
    }
}

#include "verificationdialog.moc"
