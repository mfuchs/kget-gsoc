/***************************************************************************
*                               dlgSystem.cpp
*                             -------------------
*
*    Revision     : $Id$
*    begin        : Tue Jan 29 2002
*    copyright    : (C) 2002 by Patrick Charbonnier
*                 : Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss
*    email        : pch@freeshell.org
*
****************************************************************************/

/***************************************************************************
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 ***************************************************************************/


#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>

#ifdef index
#undef index
#endif

#include <kcombobox.h>
#include <kapplication.h>
#include <kaudioplayer.h>
#include <kurlrequesterdlg.h>
#include <kfontrequester.h>
#include <klocale.h>
#include <kdialog.h>
#include <kio/netaccess.h>
#include <kfiledialog.h>

#include "settings.h"
#include "globals.h"
#include "dlgSystem.h"


DlgSystem::DlgSystem(QWidget * parent)
    : DlgSystemBase(parent)
{
}


void DlgSystem::setupSound()
{
    sDebugIn << endl;
    QString s, t;

    int id = cmb_sounds->currentItem();

    switch (id)
    {
    case 0:
        s = i18n("Sound filename for action 'added':");
        t = soundAdded;
        break;
    case 1:
        s = i18n("Sound filename for action 'started':");
        t = soundStarted;
        break;
    case 2:
        s = i18n("Sound filename for action 'finished':");
        t = soundFinished;
        break;
    case 3:
        s = i18n("Sound filename for action 'finished-all':");
        t = soundFinishedAll;
        break;
    }
    KURLRequesterDlg *box = new KURLRequesterDlg(t, s, this, "kurl_sound");
    KFileDialog *pDlg = box->fileDialog();
    pDlg->setFilter(i18n("*.wav|WAV files\n*|All Files"));
    int result = box->exec();
    sDebug << "Result= " << result << endl;

    if (result == QDialog::Rejected)
    {  /* cancelled */
        sDebug << "Cancelled" << endl;
        delete box;
        return;
    }

    s = box->selectedURL().url();       // text();
    delete box;

    sDebug << "Selected audio files: " << s << endl;
    if (s.isEmpty())
    {          /* answer is "" */
        sDebugOut << "empty selection" << endl;
        return;
    }

    switch (id)
    {
    case 0:
        soundAdded = s;
        break;

    case 1:
        soundStarted = s;
        break;

    case 2:
        soundFinished = s;
        break;

    case 3:
        soundFinishedAll = s;
        break;
    }
    sDebugOut << endl;
    slotChanged();
}


void DlgSystem::testSound()
{
    sDebug << ">>>>Entering" << endl;

    QString soundFile;

    switch (cmb_sounds->currentItem())
    {
    case 0:
        soundFile = soundAdded;
        break;
    case 1:
        soundFile = soundStarted;
        break;
    case 2:
        soundFile = soundFinished;
        break;
    case 3:
        soundFile = soundFinishedAll;
        break;
    }
    QString tmpFile;
    sDebug << "File to play " << soundFile << endl;
    //   KAudioPlayer::play(soundFile);
    //   KAudioPlayer::play( "/home/pch/pop.wav");

    if (KIO::NetAccess::download(KURL( soundFile ), tmpFile, 0))
    {
        sDebug << "Temp file to play is " << tmpFile << endl;
        KAudioPlayer::play(tmpFile);
        KIO::NetAccess::removeTempFile(tmpFile);
    }


    sDebug << "<<<<Leaving" << endl;
}

/*
void DlgSystem::setData()
{
    cb_useSound->setChecked(Settings::useSound());
    cmb_sounds->setEnabled(Settings::useSound());
    pb_changesound->setEnabled(Settings::useSound());
    pb_testsound->setEnabled(Settings::useSound());

    soundAdded = Settings::audioAdded();
    soundStarted = Settings::audioStarted();
    soundFinished = Settings::audioFinished();
    soundFinishedAll = Settings::audioFinishedAll();

    cb_useAnimation->setChecked(Settings::useAnimation());

    le_font->setFont(Settings::font());
}


void DlgSystem::applyData()
{
    if (cb_useSound->isChecked() != Settings::useSound()) {
        //FIXME kmain->slotToggleSound();
    }

    QString tmpFile;

    if (KIO::NetAccess::download(KURL( soundAdded ), tmpFile, 0))
        Settings::setAudioAdded( tmpFile );
    if (KIO::NetAccess::download(KURL( soundStarted ), tmpFile, 0))
        Settings::setAudioStarted( tmpFile );
    if (KIO::NetAccess::download(KURL( soundFinished ), tmpFile, 0))
        Settings::setAudioFinished( tmpFile );
    if (KIO::NetAccess::download(KURL( soundFinishedAll ), tmpFile, 0))
        Settings::setAudioFinishedAll( tmpFile );

    if (cb_useAnimation->isChecked() != Settings::useAnimation()) {
        //FIXME kmain->slotToggleAnimation();
    }

    Settings::setFont( le_font->font() );
    //FIXME kmain->setListFont();
}*/

void DlgSystem::slotChanged()
{
    emit configChanged();
}

#include "dlgSystem.moc"
