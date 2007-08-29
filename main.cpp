/* This file is part of the KDE project

   Copyright (C) 2002 by Patrick Charbonnier <pch@freeshell.org>
   Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include <QtDBus>

#include <kwindowsystem.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kuniqueapplication.h>
#include <kstandarddirs.h>

#include "core/kget.h"
#include "kgetadaptor.h"
#include "settings.h"
#include "ui/splash.h"
#include "mainwindow.h"

class KGetApp : public KUniqueApplication
{
public:
    KGetApp()
        : KUniqueApplication(), kget( 0 ), splash( 0 )
    {
        showSplash();
    }

    ~KGetApp()
    {
        delete splash;
        delete kget;
    }

    void showSplash()
    {
        //determine whether splash-screen is enabled in kgetrc
        if ( !Settings::showSplashscreen() )
            return;

        // getting splash-screen path
        QString path = KStandardDirs::locate( "data", "kget/pics/kget_splash.png" );

        if ( !path.isEmpty() )
            splash = new Splash( path );
    }

    int newInstance()
    {
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

        if (!kget)
        {
            kget = new MainWindow(!args->isSet("showDropTarget"), args->isSet("startWithoutAnimation"));
            new KgetAdaptor(kget);
            QDBusConnection::sessionBus().registerObject("/KGet", kget);
        }
        KWindowSystem::activateWindow(kget->winId());

        if (args->isSet("showDropTarget"))
            Settings::setShowDropTarget( true );

        KUrl::List l;
        for (int i = 0; i < args->count(); i++)
        {
            QString txt(args->arg(i));
            if ( txt.endsWith( ".kgt", Qt::CaseInsensitive ) )
                KGet::load( txt );
            else
                l.push_back(KUrl(args->arg(i)));
        }
        // all the args read from command line are downloads
        if (l.count() >= 1)
            KGet::addTransfer(l, QString(), QString(), true);
/*
        // the last arg read (when we have more than 1 arg) is considered
        // as destination dir for the previous downloads
        if (l.count() == 1)
            kget->addTransfersEx( l, KUrl());
        else if (l.count() > 1)
        {
            KUrl last = l.last();
            l.pop_back();
            kget->addTransfersEx(l, last);
        }
*/
        args->clear();
        if ( splash )
            splash->removeSplash();
        return 0;
    }

private:
    MainWindow * kget;
    Splash * splash;
};


int main(int argc, char *argv[])
{
    KAboutData aboutData("kget", 0, ki18n("KGet"), "2.0 Beta1",
                         ki18n("An advanced download manager for KDE"),
                         KAboutData::License_GPL,
                         ki18n("(C) 2005 - 2007, The KGet developers\n"
                         "(C) 2001 - 2002, Patrick Charbonnier\n"
                         "(C) 2002, Carsten Pfeiffer\n"
                         "(C) 1998 - 2000, Matej Koss"),
                         ki18n("<a href=\"mailto:kget@kde.org\">kget@kde.org</a>"));

    aboutData.addAuthor(ki18n("Dario Massarin"), ki18n("Maintainer, Core Developer"), "nekkar@libero.it");
    aboutData.addAuthor(ki18n("Urs Wolfer"), ki18n("Core Developer"), "uwolfer@kde.org");
    aboutData.addAuthor(ki18n("Manolo Valdes"), ki18n("Multithreaded Plugin Author"), "nolis71cu@gmail.com");
    aboutData.addAuthor(ki18n("Patrick Charbonnier"), ki18n("Former KGet Developer"), "pch@freeshell.org");
    aboutData.addAuthor(ki18n("Carsten Pfeiffer"), ki18n("Former KGet Developer"), "pfeiffer@kde.org");
    aboutData.addAuthor(ki18n("Matej Koss"), ki18n("Former KGet Developer"));

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions option;
    option.add("showDropTarget", ki18n("Start KGet with drop target"));
    option.add("hideMainWindow", ki18n("Start KGet with hidden main window"));
    option.add("startWithoutAnimation", ki18n("Start KGet without drop target animation"));
    option.add("+[URL(s)]", ki18n("URL(s) to download"));
    KCmdLineArgs::addCmdLineOptions(option);

    KGetApp::addCmdLineOptions();

    if (!KGetApp::start())
    {
        fprintf(stderr, "kget is already running!\n");
        exit(0);
    }

    KGetApp kApp;

    return kApp.exec();
}
