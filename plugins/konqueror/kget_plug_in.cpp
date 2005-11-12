/* This file is part of the KDE project

   Copyright (C) 2002 Patrick Charbonnier <pch@valleeurpe.net>
   Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "kget_plug_in.h"

#include <dcopref.h>
#include <kdatastream.h>
#include <kdebug.h>
#include <khtml_part.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kaction.h>
#include <kinstance.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <krun.h>

#include <dom/html_document.h>
#include <dom/html_misc.h>
#include <dom/dom_element.h>

#include <kparts/partmanager.h>

#include <set>

#include "links.h"
#include "kget_linkview.h"

KGet_plug_in::KGet_plug_in( QObject* parent )
    : Plugin( parent )
{
    QPixmap pix = KGlobal::iconLoader()->loadIcon("khtml_kget",
                                                  KIcon::MainToolbar);
    KActionMenu *menu = new KActionMenu( i18n("Download Manager"), pix,
                                         actionCollection(), "kget_menu" );
    menu->setDelayed( false );
    connect( menu->popupMenu(), SIGNAL( aboutToShow() ), SLOT( showPopup() ));

    m_paToggleDropTarget=new KToggleAction(i18n("Show Drop Target"),
                                           KShortcut(),
                                           this, SLOT(slotShowDrop()),
                                           actionCollection(), "show_drop" );

    menu->insert( m_paToggleDropTarget );

    KAction *action = new KAction(i18n("List All Links"), KShortcut(),
                                  this, SLOT( slotShowLinks() ),
                                  actionCollection(), "show_links");
    menu->insert( action );

    p_dcopServer= new DCOPClient();
    p_dcopServer->attach ();
}


KGet_plug_in::~KGet_plug_in()
{
    p_dcopServer->detach();
    delete p_dcopServer;
}


void KGet_plug_in::showPopup()
{
    bool hasDropTarget = false;

    if (p_dcopServer->isApplicationRegistered ("kget"))
    {
        DCOPRef kget( "kget", "KGet-Interface" );
        hasDropTarget = kget.call( "isDropTargetVisible" );
    }

    m_paToggleDropTarget->setChecked( hasDropTarget );
}

void KGet_plug_in::slotShowDrop()
{
    if (!p_dcopServer->isApplicationRegistered ("kget"))
        KRun::runCommand("kget --showDropTarget");
    else
    {
        DCOPRef kget( "kget", "KGet-Interface" );
        kget.send( "setDropTargetVisible", m_paToggleDropTarget->isChecked());
    }
}

void KGet_plug_in::slotShowLinks()
{
    if ( !parent() || !parent()->inherits( "KHTMLPart" ) )
        return;

    KHTMLPart *htmlPart = static_cast<KHTMLPart*>( parent() );
    KParts::Part *activePart = 0L;
    if ( htmlPart->partManager() )
    {
        activePart = htmlPart->partManager()->activePart();
        if ( activePart && activePart->inherits( "KHTMLPart" ) )
            htmlPart = static_cast<KHTMLPart*>( activePart );
    }

    DOM::HTMLDocument doc = htmlPart->htmlDocument();
    if ( doc.isNull() )
        return;

    DOM::HTMLCollection links = doc.links();

    QList<LinkItem*> linkList;
    std::set<QString> dupeCheck;
    for ( uint i = 0; i < links.length(); i++ )
    {
        DOM::Node link = links.item( i );
        if ( link.isNull() || link.nodeType() != DOM::Node::ELEMENT_NODE )
            continue;

        LinkItem *item = new LinkItem( (DOM::Element) link );
        if ( item->isValid() &&
             dupeCheck.find( item->url.url() ) == dupeCheck.end() )
        {
            linkList.append( item );
            dupeCheck.insert( item->url.url() );
        }
        else
            delete item;
    }

    if ( linkList.isEmpty() )
    {
        KMessageBox::sorry( htmlPart->widget(),
            i18n("There are no links in the active frame of the current HTML page."),
            i18n("No Links") );
        return;
    }

    KGetLinkView *view = new KGetLinkView();
    QString url = doc.URL().string();
    view->setPageURL( url );

    view->setLinks( linkList );
    view->show();
}

KPluginFactory::KPluginFactory( QObject* parent )
        : KLibFactory( parent )
{
    s_instance = new KInstance("KPluginFactory");
}

QObject* KPluginFactory::createObject( QObject* parent, const char*, const char*, const QStringList & )
{
    QObject *obj = new KGet_plug_in( parent );
    return obj;
}

KPluginFactory::~KPluginFactory()
{
    delete s_instance;
}

extern "C"
{
    KDE_EXPORT void* init_khtml_kget()
    {
        KGlobal::locale()->insertCatalog("kget");
        return new KPluginFactory;
    }

}

KInstance* KPluginFactory::s_instance = 0L;

#include "kget_plug_in.moc"
