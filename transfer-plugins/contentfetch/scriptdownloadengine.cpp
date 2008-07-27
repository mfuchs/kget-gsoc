/* This file is part of the KDE project

   Copyright (C) 2008 Ningyu Shi <shiningyu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "core/transfergroup.h"
#include "scriptdownloadengine.h"
#include <kdebug.h>

ScriptDownloadEngine::ScriptDownloadEngine(QObject *parent,
					   const KUrl &source)
    : QObject(parent),
      m_source_url(source.url())
{
}

void ScriptDownloadEngine::setSourceUrl(const QString &url)
{
    m_source_url = url;
}

QString ScriptDownloadEngine::getSourceUrl() const
{
    return m_source_url;
}

bool ScriptDownloadEngine::addTransfer(const QString &url)
{
    kDebug(5002) << "Url" << url << " from Script.";
    // TODO: check url here?
    emit newTransfer(url);
    // KGet::addTransfer(KUrl(url), m_dest_dir, m_p_group->name(), true);
    kDebug(5002) << "Transfer" << url << " Added to KGet.";
    return true;
}

