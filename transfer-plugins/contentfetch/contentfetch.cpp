/* This file is part of the KDE project

   Copyright (C) 2008 Ningyu Shi <shiningyu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "contentfetch.h"
#include "core/kget.h"
#include "core/transfergroup.h"
#include "script.h"

#include <kiconloader.h>
#include <klocale.h>
#include <KDebug>
#include <KMessageBox>
#include <KUrl>

#include <kross/core/manager.h>
#include <kross/core/action.h>
#include <QDomElement>
#include <QFile>
#include <QString>

ContentFetch::ContentFetch(TransferGroup* parent, TransferFactory* factory,
                           Scheduler* scheduler, const KUrl& source,
                           const KUrl& dest, const QString &scriptFile,
                           const QDomElement* e)
    : QObject(0), Transfer(parent, factory, scheduler, source, dest, e),
      m_p_group(parent), m_scriptFile(scriptFile), m_destDir(dest.directory(KUrl::AppendTrailingSlash))
{
    m_p_script = new Script(this, source);
    connect(m_p_script, SIGNAL(newTransfer(const QString&, const QString&)),
            this, SLOT(slotAddTransfer(const QString&, const QString&)));
    connect(m_p_script, SIGNAL(finished()), this, SLOT(slotFinish()));
    connect(m_p_script, SIGNAL(percentUpdated(int)), this, SLOT(setPercent(int)));
}

void ContentFetch::start()
{
    kDebug(5001) << "ContentFetch::start";
    setStatus(Job::Running, i18nc("transfer state: executing script", "scripting"), SmallIcon("network-connect"));
    setTransferChange(Tc_Status, true);
    m_p_script->setFile(m_scriptFile);
    m_p_script->start();
    kDebug(5001) << "ContentFetch::start() finished!";
}

void ContentFetch::stop()
{
    if(status() == Stopped)
    {
        return;
    }
    kDebug(5001) << "ContentFetch::stop";
    // kill -9 the script
    m_p_script->terminate();
    // delete m_p_script to avoid crash?
    setStatus(Job::Stopped, i18nc("transfer state: stopped", "Stopped"), SmallIcon("process-stop"));
    setTransferChange(Tc_Status, true);
}

void ContentFetch::slotAddTransfer(const QString &url, const QString &filename)
{
    // even if filename is empty it's still ok
    kDebug() << "The whole filename is " << m_destDir + filename;
    KGet::addTransfer(KUrl(url), m_destDir + filename, m_p_group->name(), true);
}

void ContentFetch::slotFinish()
{
    kDebug(5001) << "finish() slot called.";
    m_percent = 100;
    setStatus(Job::Finished, i18nc("transfer state: finished", "Finished"), SmallIcon("dialog-ok"));
    setTransferChange(Tc_Status|Tc_Percent, true);
    kDebug(5001) << "finish() slot ended.";
    //delete m_p_script;
}

bool ContentFetch::isResumable() const
{
    return false;
}

void ContentFetch::postDeleteEvent()
{
    return;
}

void ContentFetch::load(const QDomElement &e)
{
    Transfer::load(e);
}

void ContentFetch::save(const QDomElement &e)
{
    Transfer::save(e);
}

void ContentFetch::setPercent(int percent)
{
    m_percent = percent;
    setTransferChange( Transfer::Tc_Percent, true );
}
