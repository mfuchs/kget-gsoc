/***************************************************************************
*                                kmainwidget.cpp
*                             -------------------
*
*    begin        : Tue Jan 29 2002
*    copyright    : (C) 2002 by Patrick Charbonnier
*                 : Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss
*    email        : pch@freeshell.org
*    Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>
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

#include <qregexp.h>
#include <qdragobject.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qtimer.h>
#include <qdropsite.h>

#include <kapplication.h>
#include <kglobal.h>
#include <klocale.h>
#include <kwin.h>
#include <kurl.h>
#include <kurldrag.h>
#include <kstdaction.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kstatusbar.h>
#include <kiconloader.h>
#include <knotifyclient.h>
#include <knotifydialog.h>

#include "kmainwidget.h"
#include "globals.h"
#include "scheduler.h"
#include "iconview.h"
#include "testview.h"
#include "transferlist.h"
#include "kfileio.h"
#include "logwindow.h"
#include "docking.h"
#include "droptarget.h"
#include "settings.h"

// configuration includes
#include <kconfigdialog.h>
#include "dlgappearance.h"
#include "dlgnetwork.h"
#include "dlgDirectories.h"
#include "dlgadvanced.h"

// local defs.
enum StatusbarFields { ID_TOTAL_TRANSFERS = 1, ID_TOTAL_FILES, ID_TOTAL_SIZE,
                       ID_TOTAL_TIME         , ID_TOTAL_SPEED  };


KMainWidget::KMainWidget(bool bStartDocked)
    : KGetIface( "KGet-Interface" ),
      KMdiMainFrm(0, "kget mainwindow", KMdi::IDEAlMode),
      ViewInterface( "viewIface-main" ),
      kdrop(0), kdock(0), logWindow(0)
{
    setXMLFile("kgetui.rc");
    
    scheduler = new Scheduler(this);
    connectToScheduler( scheduler );
    
    setupActions();
    setupGUI(bStartDocked);
    slotUpdateActions();

    //This must be the last one
    schedRequestOperation(OpImportTransfers);
    
    if ( Settings::firstRun() )
    {
//        if ( kdrop )
//            kdrop->playAnimation();
        Settings::setFirstRun(false);
    }
}

//THESE FUNCTIONS MUST BE RE-INTEGRATED
/*  
    //Set auto-resume in kioslaverc
    {
	//include <kconfig.h>
        KConfig cfg( "kioslaverc", false, false);
        cfg.setGroup(QString::null);
        cfg.writeEntry("AutoResume", true);
        cfg.sync();
    }

    b_viewLogWindow = FALSE;

    // Set log time, needed for the name of log file
    QDate date = QDateTime::currentDateTime().date();
    QTime time = QDateTime::currentDateTime().time();
    QString tmp;
    tmp.sprintf("log%d:%d:%d-%d:%d:%d", date.day(), date.month(), date.year(), time.hour(), time.minute(), time.second());
    logFileName = locateLocal("appdata", "logs/");
    logFileName += tmp;

    // Load all settings from KConfig
    //Settings takes care of size & pos & state, so disable autosaving
    setAutoSaveSettings( "MainWindow", false );

    sDebug << "ccc" << endl;
    
    // Setup log window
    logWindow = new LogWindow();

    sDebug << "ccc1" << endl;
    
    setCaption(KGETVERSION);
    sDebug << "ccc2" << endl;

    setupGUI();

    sDebug << "ccc4" << endl;
    log(i18n("Welcome to KGet"));

    sDebug << "ccc5" << endl;
    connect(kapp, SIGNAL(saveYourself()), SLOT(slotSaveYourself()));

    sDebug << "ccc6" << endl;
    // Enable dropping
    setAcceptDrops(true);

    // update actions
    m_paExpertMode->setChecked(Settings::expertMode());
    m_paUseLastDir->setChecked(Settings::useLastDir());
    
    sDebug << "eee5" << endl;

    //m_paAutoShutdown->setChecked(Settings::autoShutdown());

    sDebug << "eee6" << endl;
    
    //m_paShowLog->setChecked(b_viewLogWindow);
*/


KMainWidget::~KMainWidget()
{
    delete kdrop;
    delete kdock;
    schedRequestOperation(OpExportTransfers);
    delete scheduler;
    //write log to file
    //kCStringToFile(logWindow->getText().local8Bit(), logFileName, false, false);
    delete logWindow;

    Settings::setMainPosition( pos() );
    Settings::setMainSize( size() );
    Settings::setMainState( KWin::windowInfo(winId()).state() );
    Settings::writeConfig();
}

void KMainWidget::setupActions()
{
    KActionCollection *coll = actionCollection();

    // Local: Shows a dialog asking for a new URL to download
    m_paOpenTransfer = new KAction(i18n("&New Download..."), "editclear", 0, this, SLOT(slotNewURL()), coll, "open_transfer");
    // Local: Destroys all sub-windows and exits
    m_paQuit = KStdAction::quit(this, SLOT(slotQuit()), coll, "quit");
    // Local: Build and show the "preferences dialog" (or reuse an existing one)
    m_paPreferences = KStdAction::preferences(this, SLOT(slotPreferences()), coll, "preferences");
    m_paPreferences->setWhatsThis(i18n("<b>Preferences</b> button opens a preferences dialog\n" "where you can set various options.\n" "\n" "Some of these options can be more easily set using the toolbar."));
    // ->Scheduler: Ask for transfersList export
    m_paExportTransfers = new KAction(i18n("&Export Transfers List..."), 0, this, SLOT(slotExportTransfers()), coll, "export_transfers");
    // ->Scheduler: Ask for transfersList import
    m_paImportTransfers = new KAction(i18n("&Import Transfers List..."), 0, this, SLOT(slotImportTransfers()), coll, "import_transfers");
    // ->Scheduler: ACTIVATE the scheduler
    m_paResume = new KAction(i18n("&Start"),"player_play", 0, this, SLOT(slotRun()), coll, "start");
    m_paResume->setWhatsThis(i18n("<b>Resume</b> button starts downloading\n"));
    // ->Scheduler: STOP the scheduler
    m_paPause = new KAction(i18n("&Stop"),"player_stop", 0, this, SLOT(slotStop()), coll, "stop");
    m_paPause->setWhatsThis(i18n("<b>Pause</b> button stops selected transfers\n" "and sets their mode to <i>delayed</i>."));

    
    (void) new KAction( i18n("Configure &Notifications..."), "knotify", 0, this, SLOT(slotEditNotifications()), coll, "configure_notifications" );
/*
    m_paImportText = new KAction(i18n("Import Text &File..."), 0, this, SLOT(slotImportTextFile()), coll, "import_text");

    // TRANSFER ACTIONS

    m_paIndividual = new KAction(i18n("&Open Individual Window"), 0, this, SLOT(slotOpenIndividual()), coll, "open_individual");
    m_paMoveToBegin = new KAction(i18n("Move to &Beginning"), 0, this, SLOT(slotMoveToBegin()), coll, "move_begin");
    m_paMoveToEnd = new KAction(i18n("Move to &End"), 0, this, SLOT(slotMoveToEnd()), coll, "move_end");

    m_paDelete = new KAction(i18n("&Delete"),"tool_delete", 0, this, SLOT(slotDeleteCurrent()), coll, "delete");
    m_paDelete->setWhatsThis(i18n("<b>Delete</b> button removes selected transfers\n" "from the list."));
    m_paRestart = new KAction(i18n("Re&start"),"tool_restart", 0, this, SLOT(slotRestartCurrent()), coll, "restart");
    m_paRestart->setWhatsThis(i18n("<b>Restart</b> button is a convenience button\n" "that simply does Pause and Resume."));

    // OPTIONS ACTIONS

    m_paExpertMode     =  new KToggleAction(i18n("&Expert Mode"),"tool_expert", 0, this, SLOT(slotToggleExpertMode()), coll, "expert_mode");
    m_paExpertMode->setWhatsThis(i18n("<b>Expert mode</b> button toggles the expert mode\n" "on and off.\n" "\n" "Expert mode is recommended for experienced users.\n" "When set, you will not be \"bothered\" by confirmation\n" "messages.\n" "<b>Important!</b>\n" "Turn it on if you are using auto-disconnect or\n" "auto-shutdown features and you want KGet to disconnect \n" "or shut down without asking."));
    m_paUseLastDir     =  new KToggleAction(i18n("&Use-Last-Folder Mode"),"tool_uselastdir", 0, this, SLOT(slotToggleUseLastDir()), coll, "use_last_dir");
    m_paUseLastDir->setWhatsThis(i18n("<b>Use last folder</b> button toggles the\n" "use-last-folder feature on and off.\n" "\n" "When set, KGet will ignore the folder settings\n" "and put all new added transfers into the folder\n" "where the last transfer was put."));
    m_paAutoShutdown   =  new KToggleAction(i18n("Auto-S&hutdown Mode"), "tool_shutdown", 0, this, SLOT(slotToggleAutoShutdown()), coll, "auto_shutdown");
    m_paAutoShutdown->setWhatsThis(i18n("<b>Auto shutdown</b> button toggles the auto-shutdown\n" "mode on and off.\n" "\n" "When set, KGet will quit automatically\n" "after all queued transfers are finished.\n" "<b>Important!</b>\n" "Also turn on the expert mode when you want KGet\n" "to quit without asking."));

    KStdAction::keyBindings(this, SLOT(slotConfigureKeys()), coll);
    KStdAction::configureToolbars(this, SLOT(slotConfigureToolbars()), coll);

    // VIEW ACTIONS
    
    createStandardStatusBarAction();

    m_paShowLog      = new KToggleAction(i18n("Show &Log Window"),"tool_logwindow", 0, this, SLOT(slotToggleLogWindow()), coll, "toggle_log");
    m_paShowLog->setWhatsThis(i18n("<b>Log window</b> button opens a log window.\n" "The log window records all program events that occur\n" "while KGet is running."));
    m_paDropTarget   = new KToggleAction(i18n("Drop &Target"),"tool_drop_target", 0, this, SLOT(slotToggleDropTarget()), coll, "drop_target");
    m_paDropTarget->setWhatsThis(i18n("<b>Drop target</b> button toggles the window style\n" "between a normal window and a drop target.\n" "\n" "When set, the main window will be hidden and\n" "instead a small shaped window will appear.\n" "\n" "You can show/hide a normal window with a simple click\n" "on a shaped window."));

    //include <khelpmenu.h>
    menuHelp = new KHelpMenu(this, KGlobal::instance()->aboutData());
    KStdAction::whatsThis(menuHelp, SLOT(contextHelpActivated()), coll, "whats_this");

    // m_paDockWindow->setWhatsThis(i18n("<b>Dock widget</b> button toggles the window style\n" "between a normal window and a docked widget.\n" "\n" "When set, the main window will be hidden and\n" "instead a docked widget will appear on the panel.\n" "\n" "You can show/hide a normal window by simply clicking\n" "on a docked widget."));
    // m_paNormal->setWhatsThis(i18n("<b>Normal window</b> button sets\n" "\n" "the window style to normal window"));    
*/
}

void KMainWidget::setupGUI(bool startDocked)
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif
    //Create Menus and Toolbars
    createGUI(0);

    //Wrap a QWidget
    //addWindow(createWrapper(t, "", ""));

    //IconView
    IconViewMdiView * i = new IconViewMdiView();
    i->connectToScheduler(scheduler);
    addWindow(i);
    i->show();

    //TestView
    TestView * t = new TestView();
    t->connectToScheduler(scheduler);
    addWindow(t);
    t->show();

    //DropTarget
    kdrop = new DropTarget(this);
    kdrop->connectToScheduler(scheduler);
    kdrop->show();

    //DockWidget
    kdock = new DockWidget(this);
    kdock->connectToScheduler(scheduler);
    kdock->show();

    //MainWidget (myself)
    resize(Settings::mainSize());
    move(Settings::mainPosition());
//    KWin::setState(winId(), Settings::mainState());
    if (!startDocked || Settings::showMain())
        show();

    // setup statusbar
    statusBar()->insertFixedItem(i18n(" Transfers: %1 ").arg(99), ID_TOTAL_TRANSFERS);
    statusBar()->insertFixedItem(i18n(" Files: %1 ").arg(555), ID_TOTAL_FILES);
    statusBar()->insertFixedItem(i18n(" Size: %1 KB ").arg("134.56"), ID_TOTAL_SIZE);
    statusBar()->insertFixedItem(i18n(" Time: 00:00:00 "), ID_TOTAL_TIME);
    statusBar()->insertFixedItem(i18n(" %1 KB/s ").arg("123.34"), ID_TOTAL_SPEED);
    updateStatusBar();

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}

void KMainWidget::log(const QString & message, bool statusbar)
{
#ifdef _DEBUG
    sDebugIn <<" message= "<< message << endl;
#endif

    logWindow->logGeneral(message);

    if (statusbar) {
        statusBar()->message(message, 1000);
    }

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotSaveYourself()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    schedRequestOperation(OpExportTransfers);
    Settings::writeConfig();

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}

void KMainWidget::slotConfigureKeys()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    KKeyDialog::configure(actionCollection());


#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotConfigureToolbars()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    saveMainWindowSettings( KGlobal::config(), "MainWindow" );
    KEditToolbar edit(factory());
    connect(&edit, SIGNAL( newToolbarConfig() ), this, SLOT( slotNewToolbarConfig() ));
    edit.exec();

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotNewToolbarConfig()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif
    createGUI(0);
    applyMainWindowSettings( KGlobal::config(), "MainWindow" );

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotEditNotifications()
{
    KNotifyDialog::configure(this);
}


void KMainWidget::slotNewConfig()
{
    sDebugIn << endl;
    sDebugOut << endl;
}


void KMainWidget::slotQuit()
{
/*
    log(i18n("Quitting..."));

    // TODO check if items in ST_RUNNING state and ask for confirmation before quitting (if not expert mode)
    if (someRunning && !Settings::expertMode()) {
	//include <kmessagebox.h>
	if (KMessageBox::warningYesNo(this, i18n("Some transfers are still running.\nAre you sure you want to close KGet?"), i18n("Warning")) != KMessageBox::Yes)
            return;
    }
*/
    Settings::writeConfig();
    kapp->quit();
}

void KMainWidget::slotExportTransfers()
{
    schedRequestOperation( OpExportTransfers );
}

void KMainWidget::slotImportTransfers()
{
    schedRequestOperation(OpImportTransfers);
}

void KMainWidget::slotRun()
{
    schedRequestOperation(OpRun);
}

void KMainWidget::slotStop()
{
    schedRequestOperation(OpStop);
}

void KMainWidget::readTransfersEx(const KURL & url)
{
    //### port to schedRequestOperation(OpReadTransfers,url);
    scheduler->slotImportTransfers(url);
}


void KMainWidget::dragEnterEvent(QDragEnterEvent * event)
{
#ifdef _DEBUG
    //sDebugIn << endl;
#endif

    event->accept(KURLDrag::canDecode(event) || QTextDrag::canDecode(event));

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::dropEvent(QDropEvent * event)
{
#ifdef _DEBUG
    //sDebugIn << endl;
#endif

    KURL::List list;
    QString str;

    if (KURLDrag::decode(event, list))
        schedNewURLs(list, QString());
    else if (QTextDrag::decode(event, str))
        schedNewURLs(KURL::fromPathOrURL(str), QString());

    sDebugOut << endl;
}

bool KMainWidget::queryClose()
{
    if( kapp->sessionSaving())
        return true;
    hide();
    return false;
}


void KMainWidget::slotPreferences()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    KNotifyClient::event( winId(), "started" );

    //An instance of your dialog could be already created and could be cached, 
    //in which case you want to display the cached dialog instead of creating 
    //another one 
    if ( KConfigDialog::showDialog( "preferences" ) ) 
        return; 

    //KConfigDialog didn't find an instance of this dialog, so lets create it
    KConfigDialog* dialog = new KConfigDialog( this, "preferences", Settings::self() );

    dialog->addPage( new DlgAppearance(0), i18n("Appearance"), "looknfeel",
                     i18n("Look and feel") );
    dialog->addPage( new DlgNetwork(0), i18n("Network"), "network",
                     i18n("Network and downloads") );
    dialog->addPage( new DlgDirectories(0), i18n("Directories"), "folder_open",
                     i18n("Default download directories") );
    dialog->addPage( new DlgAdvanced(0), i18n("Advanced"), "exec",
                     i18n("Advanced options") );

    //User edited the configuration - update your local copies of the 
    //configuration data 
    connect( dialog, SIGNAL(settingsChanged()), 
             this, SLOT(slotNewConfig()) );
 
    dialog->show();

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}

void KMainWidget::slotNewURL()
{
    schedNewURLs(KURL(), QString::null);
}

void KMainWidget::slotToggleLogWindow()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    b_viewLogWindow = !b_viewLogWindow;
    if (b_viewLogWindow)
        logWindow->show();
    else
        logWindow->hide();

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}

void KMainWidget::slotToggleExpertMode()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    bool expert = !Settings::expertMode();
    Settings::setExpertMode( expert );

    if (expert) {
        log(i18n("Expert mode on."));
    } else {
        log(i18n("Expert mode off."));
    }
    m_paExpertMode->setChecked(expert);

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotToggleUseLastDir()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    Settings::setUseLastDirectory( !Settings::useLastDirectory() );

    if (Settings::useLastDirectory()) {
        log(i18n("Use last folder on."));
    } else {
        log(i18n("Use last folder off."));
    }

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotToggleAutoShutdown()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    bool autoShutDown = !Settings::autoShutdown();
    Settings::setAutoShutdown( autoShutDown );

    if (autoShutDown) {
        log(i18n("Auto shutdown on."));
    } else {
        log(i18n("Auto shutdown off."));
    }

    m_paAutoShutdown->setChecked(autoShutDown);

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotToggleDropTarget()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    if (m_paDropTarget->isChecked()) {
        kdrop->show();
        kdrop->updateStickyState();
    }
    else
        kdrop->hide();

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotUpdateActions()
{
/*#ifdef _DEBUG
    sDebugIn << endl;
#endif

    m_paDelete->setEnabled(false);
    m_paResume->setEnabled(false);
    m_paPause->setEnabled(false);
    m_paRestart->setEnabled(false);

    m_paIndividual->setEnabled(false);
    m_paMoveToBegin->setEnabled(false);
    m_paMoveToEnd->setEnabled(false);

    Transfer *item;
    Transfer *first_item = 0L;
    
    int index = 0;
    int totals_items = 0;
    int sel_items = 0;

    //FOR EACH ITEM IN THE TRANSFER LIST
    {

        // update action on visibles windows
        if (it.current()->isVisible())
            it.current()->slotUpdateActions();

        if (it.current()->isSelected()) {
            item = it.current();
            sel_items = totals_items;
            index++;            // counting number of selected items
            if (index == 1) {
                first_item = item;      // store first selected item
                if (totals_items > 0)
                    m_paMoveToBegin->setEnabled(true);

                m_paMoveToEnd->setEnabled(true);
            } else {

                m_paMoveToBegin->setEnabled(false);
                m_paMoveToEnd->setEnabled(false);
            }
            // enable PAUSE, RESUME and RESTART only when we are online and not in offline mode
#ifdef _DEBUG
            sDebug << "-->ONLINE= " << Settings::offlineMode() << endl;
#endif
            if (item == first_item && SONO ONLINE) {
                switch (item->getStatus()) {
                case Transfer::ST_TRYING:
                case Transfer::ST_RUNNING:
                    m_paResume->setEnabled(false);
                    m_paPause->setEnabled(true);
                    m_paRestart->setEnabled(true);
                    break;
                case Transfer::ST_STOPPED:
                    m_paResume->setEnabled(true);
                    m_paPause->setEnabled(false);
                    m_paRestart->setEnabled(false);
#ifdef _DEBUG
                    sDebug << "STATUS IS  stopped" << item->getStatus() << endl;
#endif
                    break;
                case Transfer::ST_FINISHED:
                    m_paResume->setEnabled(false);
                    m_paPause->setEnabled(false);
                    m_paRestart->setEnabled(false);
                    break;


                }               //end switch

            } else if (item->getStatus() != first_item->getStatus()) {
                // disable all when all selected items don't have the same status
                m_paResume->setEnabled(false);
                m_paPause->setEnabled(false);
                m_paRestart->setEnabled(false);
            }


            if (item == first_item) {
                m_paDelete->setEnabled(true);
                m_paIndividual->setEnabled(true);
            } else if (item->getMode() != first_item->getMode()) {
                // unset all when all selected items don't have the same mode
                m_paMoveToBegin->setEnabled(false);
                m_paMoveToEnd->setEnabled(false);
            }

        }                       // when item is selected
    }                           // loop



    if (sel_items == totals_items - 1)
        m_paMoveToEnd->setEnabled(false);

#ifdef _DEBUG
    sDebugOut << endl;
#endif
*/
}


void KMainWidget::updateStatusBar()
{
#ifdef _DEBUG
    //sDebugIn << endl;
#endif
/*
    Transfer *item;
    QString tmpstr;

    int totalFiles = 0;
    int totalSize = 0;
    int totalSpeed = 0;
    QTime remTime;

    //FOR EACH TRANSFER ON THE TRANSFER LIST {
        item = it.current();
        if (item->getTotalSize() != 0) {
            totalSize += (item->getTotalSize() - item->getProcessedSize());
        }
        totalFiles++;
        totalSpeed += item->getSpeed();

        if (item->getRemainingTime() > remTime) {
            remTime = item->getRemainingTime();
        }
    }

    statusBar()->changeItem(i18n(" Transfers: %1 ").arg( ASK TO SCHEDULER FOR NUMBER ), ID_TOTAL_TRANSFERS);
    statusBar()->changeItem(i18n(" Files: %1 ").arg(totalFiles), ID_TOTAL_FILES);
    statusBar()->changeItem(i18n(" Size: %1 ").arg(KIO::convertSize(totalSize)), ID_TOTAL_SIZE);
    statusBar()->changeItem(i18n(" Time: %1 ").arg(remTime.toString()), ID_TOTAL_TIME);
    statusBar()->changeItem(i18n(" %1/s ").arg(KIO::convertSize(totalSpeed)), ID_TOTAL_SPEED);
*/

#ifdef _DEBUG
    //sDebugOut << endl;
#endif
}

void KMainWidget::addTransfers( const KURL::List& src, const QString& dest)
{
    sDebugIn << endl;
    schedNewURLs(src, dest); 
    sDebugOut << endl;
}

bool KMainWidget::isDropTargetVisible() const
{
    return m_paDropTarget->isChecked();
}

void KMainWidget::setDropTargetVisible( bool setVisible )
{
    if ( setVisible != isDropTargetVisible() )
    {
        m_paDropTarget->setChecked( !m_paDropTarget->isChecked() );
        slotToggleDropTarget();
    }
}

void KMainWidget::setOfflineMode( bool offline )
{
    schedRequestOperation( offline ? OpStop : OpRun );
}

bool KMainWidget::isOfflineMode() const
{
    //FIXME ceck if the scheduler isRunning
    return false;
}

#include "kmainwidget.moc"

//BEGIN auto-disconnection 
/*
KToggleAction *m_paAutoDisconnect,
    m_paAutoDisconnect =  new KToggleAction(i18n("Auto-&Disconnect Mode"),"tool_disconnect", 0, this, SLOT(slotToggleAutoDisconnect()), coll, "auto_disconnect");
    tmp = i18n("<b>Auto disconnect</b> button toggles the auto-disconnect\n" "mode on and off.\n" "\n" "When set, KGet will disconnect automatically\n" "after all queued transfers are finished.\n" "\n" "<b>Important!</b>\n" "Also turn on the expert mode when you want KGet\n" "to disconnect without asking.");
    m_paAutoDisconnect->setWhatsThis(tmp);

    if (Settings::connectionType() != Settings::Permanent) {
        //m_paAutoDisconnect->setChecked(Settings::autoDisconnect());
    }
    setAutoDisconnect();

void KMainWidget::slotToggleAutoDisconnect()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    Settings::setAutoDisconnect( !Settings::autoDisconnect() );

    if (Settings::autoDisconnect()) {
        log(i18n("Auto disconnect on."));
    } else {
        log(i18n("Auto disconnect off."));
    }
    m_paAutoDisconnect->setChecked(Settings::autoDisconnect());
    
#ifdef _DEBUG
    sDebugOut << endl;
#endif
}

void KMainWidget::setAutoDisconnect()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    // disable action when we are connected permanently
    //m_paAutoDisconnect->setEnabled(Settings::connectionType() != Settings::Permanent);

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}
*/
//END 

//BEGIN animations 
/*

    // Setup animation timer
    animTimer = new QTimer(this);
    animCounter = 0;
    connect(animTimer, SIGNAL(timeout()), SLOT(slotAnimTimeout()));
    
    if (Settings::useAnimation()) {
        animTimer->start(400);
    } else {
        animTimer->start(1000);
    }

    KToggleAction *m_paUseAnimation
    m_paUseAnimation->setChecked(Settings::useAnimation());
    m_paUseAnimation   =  new KToggleAction(i18n("Use &Animation"), 0, this, SLOT(slotToggleAnimation()), coll, "toggle_animation");

void KMainWidget::slotToggleAnimation()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    Settings::setUseAnimation( !Settings::useAnimation() );

    if (!Settings::useAnimation() && animTimer->isActive()) {
        animTimer->stop();
        animTimer->start(1000);
        animCounter = 0;
    } else {
        animTimer->stop();
        animTimer->start(400);
    }

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}

void KMainWidget::slotAnimTimeout()
{
#ifdef _DEBUG
    //sDebugIn << endl;
#endif

    bool isTransfer;

    animCounter++;
    if (animCounter == $myTransferList$->getPhasesNum()) {
        animCounter = 0;
    }
    // update status of all items of transferList
    isTransfer = $myTransferList$->updateStatus(animCounter);

    if (this->isVisible()) {
        updateStatusBar();
    }
#ifdef _DEBUG
    //sDebugOut << endl;
#endif

}
*/
//END 

//BEGIN copy URL to clipboard 
/*
    KAction *m_paCopy,
    m_paCopy = new KAction(i18n("&Copy URL to Clipboard"), 0, this, SLOT(slotCopyToClipboard()), coll, "copy_url");
    //on updateActions() set to true if an url is selected else set to false
    m_paCopy->setEnabled(false);

void KMainWidget::slotCopyToClipboard()
{
#ifdef _DEBUG
    //sDebugIn << endl;
#endif

    Transfer *item = CURRENT ITEM!;

    if (item) {
        QString url = item->getSrc().url();
        QClipboard *cb = QApplication::clipboard();
        cb->setText( url, QClipboard::Selection );
        cb->setText( url, QClipboard::Clipboard);
    }
    
#ifdef _DEBUG
    sDebugOut << endl;
#endif
}
*/  
//END

//BEGIN Auto saving transfer list 
/*
    // Setup autosave timer
    autosaveTimer = new QTimer(this);
    connect(autosaveTimer, SIGNAL(timeout()), SLOT(slotAutosaveTimeout()));
    setAutoSave();

void KMainWidget::slotAutosaveTimeout()
{
#ifdef _DEBUG
    //sDebugIn << endl;
#endif

    schedRequestOperation( OpExportTransfers );

#ifdef _DEBUG
    //sDebugOut << endl;
#endif
}

void KMainWidget::setAutoSave()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    autosaveTimer->stop();
    if (Settings::autoSave()) {
        autosaveTimer->start(Settings::autoSaveInterval() * 60000);
    }

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}
*/
//END 

//BEGIN queue-timer-delay 
/*
//construct actions
    KRadioAction *m_paQueue, *m_paTimer, *m_paDelay;
    m_paQueue = new KRadioAction(i18n("&Queue"),"tool_queue", 0, this, SLOT(slotQueueCurrent()), coll, "queue");
    m_paTimer = new KRadioAction(i18n("&Timer"),"tool_timer", 0, this, SLOT(slotTimerCurrent()), coll, "timer");
    m_paDelay = new KRadioAction(i18n("De&lay"),"tool_delay", 0, this, SLOT(slotDelayCurrent()), coll, "delay");
    m_paQueue->setExclusiveGroup("TransferMode");
    m_paTimer->setExclusiveGroup("TransferMode");
    m_paDelay->setExclusiveGroup("TransferMode");
    tmp = i18n("<b>Queued</b> button sets the mode of selected\n" "transfers to <i>queued</i>.\n" "\n" "It is a radio button -- you can choose between\n" "three modes.");
    m_paQueue->setWhatsThis(tmp);
    tmp = i18n("<b>Scheduled</b> button sets the mode of selected\n" "transfers to <i>scheduled</i>.\n" "\n" "It is a radio button -- you can choose between\n" "three modes.");
    m_paTimer->setWhatsThis(tmp);
    tmp = i18n("<b>Delayed</b> button sets the mode of selected\n" "transfers to <i>delayed</i>." "This also causes the selected transfers to stop.\n" "\n" "It is a radio button -- you can choose between\n" "three modes.");
    m_paDelay->setWhatsThis(tmp);

    // Setup transfer timer for scheduled downloads and checkQueue()
    transferTimer = new QTimer(this);
    connect(transferTimer, SIGNAL(timeout()), SLOT(slotTransferTimeout()));
    transferTimer->start(10000);        // 10 secs time interval

//FOLLOWING CODE WAS IN UPDATEACTIONS
    // disable all signals
    m_paQueue->blockSignals(true);
    m_paTimer->blockSignals(true);
    m_paDelay->blockSignals(true);

    // at first turn off all buttons like when nothing is selected
    m_paQueue->setChecked(false);
    m_paTimer->setChecked(false);
    m_paDelay->setChecked(false);

    m_paQueue->setEnabled(false);
    m_paTimer->setEnabled(false);
    m_paDelay->setEnabled(false);

       if ( L'ITEM E' SELECTED )
       {
            //CONTROLLA SE L'ITEM E' IL PRIMO AD ESSERE SELEZIONATO, ALTRIMENTI
	    //DEVE VEDERE SE TUTTI GLI ALTRI SELECTED HANNO LO STESSO MODO, ALTRIMENTI
	    //DISABILITANO LE AZIONI
            if (item == first_selected_item) {
                if (item->getStatus() != Transfer::ST_FINISHED) {
                    m_paQueue->setEnabled(true);
                    m_paTimer->setEnabled(true);
                    m_paDelay->setEnabled(true);

                    switch (item->getMode()) {
                    case Transfer::MD_QUEUED:
#ifdef _DEBUG
                        sDebug << "....................THE MODE  IS  MD_QUEUED " << item->getMode() << endl;
#endif
                        m_paQueue->setChecked(true);
                        break;
                    case Transfer::MD_SCHEDULED:
#ifdef _DEBUG
                        sDebug << "....................THE MODE  IS  MD_SCHEDULED " << item->getMode() << endl;
#endif
                        m_paTimer->setChecked(true);
                        break;
                    case Transfer::MD_DELAYED:
#ifdef _DEBUG
                        sDebug << "....................THE MODE  IS  MD_DELAYED " << item->getMode() << endl;
#endif
                        m_paDelay->setChecked(true);
                        break;
                    }
                }
            } else if (item->getMode() != first_item->getMode()) {
                // unset all when all selected items don't have the same mode
                m_paQueue->setChecked(false);
                m_paTimer->setChecked(false);
                m_paDelay->setChecked(false);
                m_paQueue->setEnabled(false);
                m_paTimer->setEnabled(false);
                m_paDelay->setEnabled(false);
            }
       }
    // enale all signals    
    m_paQueue->blockSignals(false);
    m_paTimer->blockSignals(false);
    m_paDelay->blockSignals(false);    
*/
//END 

//BEGIN offline mode 
/*
    KToggleAction *m_paOfflineMode
    //m_paOfflineMode->setChecked(Settings::offlineMode());
    if (Settings::offlineMode())
        setCaption(i18n("Offline"), false);
    else {
        setCaption(QString::null, false);
        //m_paOfflineMode->setIcon( "tool_offline_mode_on" );
    }
    m_paOfflineMode =  new KToggleAction(i18n("&Offline Mode"),"tool_offline_mode_off", 0, this, SLOT(slotToggleOfflineMode()), coll, "offline_mode");
    tmp = i18n("<b>Offline mode</b> button toggles the offline mode\n" "on and off.\n" "\n" "When set, KGet will act as if it was not connected\n" "to the Internet.\n" "\n" "You can browse offline, while still being able to add\n" "new transfers as queued.");
    m_paOfflineMode->setWhatsThis(tmp);

void KMainWidget::slotToggleOfflineMode()
{
    Settings::setOfflineMode( !Settings::offlineMode() );
    if (Settings::offlineMode()) {
        log(i18n("Offline mode on."));
        pauseAll();
        setCaption(i18n("Offline"), false);
        m_paOfflineMode->setIcon( "tool_offline_mode_off" );
    } else {
        log(i18n("Offline mode off."));
        setCaption(i18n(""), false);
        m_paOfflineMode->setIcon( "tool_offline_mode_on" );
    }
    m_paOfflineMode->setChecked(Settings::offlineMode());

    slotUpdateActions();
    //checkQueue();
}

void KMainWidget::setOfflineMode( bool offline )
{
    //FIXME start/stop the scheduler
    if ( Settings::offlineMode() != offline )
        slotToggleOfflineMode();
}
*/
//END 

//BEGIN clipboard check
/*
    lastClipboard = QApplication::clipboard()->text( QClipboard::Clipboard ).stripWhiteSpace();

    // Setup clipboard timer
#include <qclipboard.h>
    QTimer *clipboardTimer;     // timer for checking clipboard - autopaste function
    clipboardTimer = new QTimer(this);
    connect(clipboardTimer, SIGNAL(timeout()), SLOT(slotCheckClipboard()));
    if (Settings::autoPaste())
        clipboardTimer->start(1000);

    KToggleAction *m_paAutoPaste;   
    m_paAutoPaste =  new KToggleAction(i18n("Auto-Pas&te Mode"),"tool_clipboard", 0, this, SLOT(slotToggleAutoPaste()), coll, "auto_paste");
    tmp = i18n("<b>Auto paste</b> button toggles the auto-paste mode\n" "on and off.\n" "\n" "When set, KGet will periodically scan the clipboard\n" "for URLs and paste them automatically.");
    m_paAutoPaste->setWhatsThis(tmp);
    //m_paAutoPaste->setChecked(Settings::autoPaste());

    KAction *m_paPasteTransfer;
    (### CHG WITH schedReqOp) m_paPasteTransfer = KStdAction::paste($scheduler$, SLOT(slotPasteTransfer()), coll, "paste_transfer");
    tmp = i18n("<b>Paste transfer</b> button adds a URL from\n" "the clipboard as a new transfer.\n" "\n" "This way you can easily copy&paste URLs between\n" "applications.");
    m_paPasteTransfer->setWhatsThis(tmp);

void KMainWidget::slotToggleAutoPaste()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    bool autoPaste = !Settings::autoPaste();
    Settings::setAutoPaste( autoPaste );

    if (autoPaste) {
        log(i18n("Auto paste on."));
        clipboardTimer->start(1000);
    } else {
        log(i18n("Auto paste off."));
        clipboardTimer->stop();
    }
    m_paAutoPaste->setChecked(autoPaste);

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}

void KMainWidget::slotCheckClipboard()
{
#ifdef _DEBUG
    //sDebugIn << endl;
#endif

    QString clipData = QApplication::clipboard()->text( QClipboard::Clipboard ).stripWhiteSpace();

    if (clipData != lastClipboard) {
        sDebug << "New clipboard event" << endl;

        lastClipboard = clipData;
        if ( lastClipboard.isEmpty() )
            return;

        KURL url = KURL::fromPathOrURL( lastClipboard );

        if (url.isValid() && !url.isLocalFile())
            schedRequestOperation( OpPasteTransfer );
    }

#ifdef _DEBUG
    //sDebugOut << endl;
#endif
}
*/
//END 

//BEGIN 
/*
*/
//END 

