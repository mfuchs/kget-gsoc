/* This file is part of the KDE project

   Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>
   based on amarok code by Mark Kretschmann <markey@web.de> (C) Copyright 2004

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "plugin.h"

#include <kdebug.h>

KGetPlugin::KGetPlugin()
{
    kDebug(5001) << k_funcinfo << endl;
}

KGetPlugin::~KGetPlugin()
{
    kDebug(5001) << k_funcinfo << endl;
}

/*
void Plugin::addPluginProperty( const QString& key, const QString& value )
{
    m_properties[key.toLower()] = value;
}

QString Plugin::pluginProperty( const QString& key )
{
    if ( m_properties.find( key.toLower() ) == m_properties.end() )
        return "false";

    return m_properties[key.toLower()];
}

bool Plugin::hasPluginProperty( const QString& key )
{
    return m_properties.find( key.toLower() ) != m_properties.end();
}
*/

