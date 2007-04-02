/* This file is part of the KDE project

   Copyright (C) 2002 Patrick Charbonnier <pch@freeshell.org>
   Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include <QBitmap>
#include <QClipboard>
#include <QPainter>
#include <QTimer>

#include <kapplication.h>
#include <kwin.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kactioncollection.h>
#include <kiconloader.h>
#include <KGlobalSettings>

#include <math.h>

#include "core/kget.h"
#include "settings.h"
#include "ui/droptarget.h"
#include "mainwindow.h"

#define TARGET_SIZE   64
#define TARGET_ANI_MS 20

DropTarget::DropTarget(MainWindow * mw)
    : QWidget(0, Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint),
    parentWidget(mw), animTimer(0)
{
    QRect desk = KGlobalSettings::desktopGeometry(this);
    desk.setRight( desk.right() - TARGET_SIZE );
    desk.setBottom( desk.bottom() - TARGET_SIZE );

    if (desk.contains(Settings::dropPosition())
            && ((Settings::dropPosition().x() != 0) || (Settings::dropPosition().y() != 0)))
        position = QPoint(Settings::dropPosition());
    else
        position = QPoint((int)desk.width() / 2, (int)desk.height() / 2);

    resize(TARGET_SIZE, TARGET_SIZE);

    if(Settings::dropSticky())
        KWin::setState(winId(), NET::Sticky);

    cachedPixmap = DesktopIcon("kget", TARGET_SIZE);
    if (!cachedPixmap.mask().isNull())
    {
        QBitmap mask(size());
        mask.fill(Qt::color0);
        QBitmap pixMask = cachedPixmap.mask();
        QPainter p(&mask);
        p.drawPixmap((mask.width() - pixMask.width())/2, (mask.height() - pixMask.height())/2,
                     pixMask);
        setMask(mask);
    }
    else
        setMask(QBitmap());

    update();

    // popup menu for right mouse button
    popupMenu = new KMenu();
    popupMenu->addTitle(mw->windowTitle());

    QAction * downloadAction = mw->actionCollection()->action("start_all_download");
    popupMenu->addAction( downloadAction );
    connect( downloadAction, SIGNAL( toggled(bool) ), this, SLOT( slotStartStopToggled(bool) ) );
    popupMenu->addSeparator();
    pop_show = popupMenu->addAction( QString(), this, SLOT( toggleMinimizeRestore() ) );
    popupMenu->addAction(i18n("Hide me"), this, SLOT(slotClose()));
    pop_sticky = popupMenu->addAction(i18n("Sticky"), this, SLOT(toggleSticky()));
    pop_sticky->setCheckable(true);
    pop_sticky->setChecked(Settings::dropSticky());
    popupMenu->addSeparator();
    popupMenu->addAction( mw->actionCollection()->action("preferences") );
    popupMenu->addAction( mw->actionCollection()->action("quit") );

    isdragging = false;

    // Enable dropping
    setAcceptDrops(true);

    if ( Settings::showDropTarget() && Settings::firstRun() )
       playAnimationShow();
}


DropTarget::~DropTarget()
{
    Settings::setDropPosition( pos() );
    Settings::setShowDropTarget( !isHidden() );
//    unsigned long state = KWin::windowInfo(kdrop->winId()).state();
//    // state will be 0L if droptarget is hidden. Sigh.
//    config->writeEntry("State", state ? state : DEFAULT_DOCK_STATE );
    delete popupMenu;
    delete animTimer;
}

void DropTarget::setVisible( bool shown, bool internal )
{
    if (shown == !isHidden())
        return;

    if ( internal )
        Settings::setShowDropTarget( shown );

    if (!shown)
    {
        Settings::setDropPosition( pos() );
        if ( Settings::animateDropTarget() )
            playAnimationHide();
        else
            hide();
    }
    else
    {
        show();
        if ( Settings::animateDropTarget() )
            playAnimationShow();
        else
            move(position);
    }
}

void DropTarget::playAnimationShow()
{
    if ( animTimer )
        delete animTimer;
    animTimer = new QTimer;
    connect( animTimer, SIGNAL( timeout() ),
        this, SLOT( slotAnimateShow() ));

    move(position.x(), -TARGET_SIZE);

    ani_y = -1;
    ani_vy = 0;
    animTimer->start(TARGET_ANI_MS);
}

void DropTarget::playAnimationHide()
{
    if ( animTimer )
    {
        if ( animTimer->isActive() )
            move( x(), (int)(ani_y) );
        delete animTimer;
    }
    animTimer = new QTimer;
    connect( animTimer, SIGNAL( timeout() ),
        this, SLOT( slotAnimateHide() ));
    ani_y = (float)y();
    ani_vy = 0;
    animTimer->start(TARGET_ANI_MS);
}

void DropTarget::playAnimationSync()
{
    if ( animTimer )
    {
        if ( animTimer->isActive() )
            move( x(), (int)(ani_y) );
        delete animTimer;
    }
    animTimer = new QTimer;
    connect( animTimer, SIGNAL( timeout() ),
        this, SLOT( slotAnimateSync() ));
    ani_y = (float)y();
    ani_vy = -1;
    animTimer->start(TARGET_ANI_MS);
}

void DropTarget::slotStartStopToggled( bool started )
{
    if ( started && Settings::animateDropTarget() )
        playAnimationSync();
}


/** widget events */

void DropTarget::dragEnterEvent(QDragEnterEvent * event)
{
    event->setAccepted(KUrl::List::canDecode(event->mimeData())
                  || event->mimeData()->hasText());
}


void DropTarget::dropEvent(QDropEvent * event)
{
    KUrl::List list = KUrl::List::fromMimeData(event->mimeData());
    QString str;

    if (!list.isEmpty())
    {
        KUrl::List::Iterator it = list.begin();
        KUrl::List::Iterator itEnd = list.end();

        for( ; it!=itEnd ; ++it )
            KGet::addTransfer(*it);
    }
    else
    {
        str = event->mimeData()->text();
        KGet::addTransfer(KUrl(str));
    }

    if ( Settings::animateDropTarget() )
        playAnimationSync();
}


void DropTarget::closeEvent( QCloseEvent * e )
{
    if( kapp->sessionSaving() )
        e->ignore();
    else
    {
        setVisible( false );
        e->accept();
    }
}

void DropTarget::mousePressEvent(QMouseEvent * e)
{
    // If the user click on the droptarget, stop any animation that is going on
    if(animTimer)
    {
        animTimer->stop();
        delete animTimer;
        animTimer = 0;
    }

    if (e->button() == Qt::LeftButton)
    {
        isdragging = true;
        dx = e->globalPos().x() - pos().x();
        dy = e->globalPos().y() - pos().y();
    }
    else if (e->button() == Qt::RightButton)
    {
        pop_show->setText(parentWidget->isHidden() ?
                              i18n("Show main window") :
                              i18n("Hide main window") );
        popupMenu->popup(e->globalPos());
    }
    else if (e->button() == Qt::MidButton)
    {
        //Here we paste the transfer
        QString newtransfer = QApplication::clipboard()->text();
        newtransfer = newtransfer.trimmed();

        if(!newtransfer.isEmpty())
            KGet::addTransfer(KUrl(newtransfer), QString());
    }
}

void DropTarget::mouseReleaseEvent(QMouseEvent *)
{
    isdragging = false;
}

void DropTarget::mouseDoubleClickEvent(QMouseEvent * e)
{
    if (e->button() == Qt::LeftButton)
        toggleMinimizeRestore();
}

void DropTarget::mouseMoveEvent(QMouseEvent * e)
{
    Q_UNUSED(e);
    if ( isdragging && !Settings::dropSticky() )
    {
        move( QCursor::pos().x() - dx, QCursor::pos().y() - dy );
        e->accept();
    }
}

void DropTarget::paintEvent( QPaintEvent * )
{
    QPainter p(this);
    const QRect r = rect();
    p.drawPixmap(r.x() + (r.width() - cachedPixmap.width())/2,
                 r.y() + (r.height() - cachedPixmap.height())/2,
                 cachedPixmap);
}

void DropTarget::toggleSticky()
{
    Settings::setDropSticky( !Settings::dropSticky() );
    pop_sticky->setChecked(Settings::dropSticky());

    if ( Settings::dropSticky() )
        KWin::setState(winId(), NET::SkipTaskbar | NET::StaysOnTop | NET::Sticky);
    else
        KWin::clearState(winId(), NET::Sticky);
}

void DropTarget::toggleMinimizeRestore()
{
    bool nextState = parentWidget->isHidden();
    Settings::setShowMain( nextState );
    parentWidget->setVisible( nextState );
}

/** widget animations */
void DropTarget::slotAnimateShow()
{
    static float dT = TARGET_ANI_MS / 1000.0;

    ani_vy -= ani_y * 30 * dT;
    ani_vy *= 0.95;
    ani_y += ani_vy * dT;

    move(x(), (int)(position.y() * (1 + ani_y)));

    if ( fabsf(ani_y) < 0.01 && fabsf(ani_vy) < 0.01 && animTimer )
    {
        animTimer->stop();
        delete animTimer;
        animTimer = 0;
    }
}

void DropTarget::slotAnimateHide()
{
    static float dT = TARGET_ANI_MS / 1000.0;

    ani_vy += -2000 * dT;
    float new_y = y() + ani_vy * dT;

    if ( new_y < -height() )
    {
        animTimer->stop();
        delete animTimer;
        animTimer = 0;
        move( x(), (int)(ani_y) );
        hide();
    } else
        move( x(), (int)(new_y) );
}

void DropTarget::slotAnimateSync()
{
    static float dT = TARGET_ANI_MS / 1000.0;

    ani_vy += 4 * dT;               // from -1 to 1 in 0.5 seconds
    float i = 2 * M_PI * ani_vy;    // from -2PI to 2PI
    float j = (i == 0.0) ? 1 : (sin( i ) / i) * (1 + fabs(ani_vy));

    if ( ani_vy >= 1 )
    {
        animTimer->stop();
        delete animTimer;
        animTimer = 0;
        move( x(), (int)(ani_y) );
    } else
        move( x(), (int)(ani_y + 6*j) );
}

void DropTarget::slotClose()
{
    setVisible( false );
    if (!Settings::expertMode())
    {
        KMessageBox::information(parentWidget,
            i18n("Drop target has been hidden. If you want to show it "
                 "again, go to Settings->Configure KGet->Look & Feel."),
            i18n("Hiding drop target"),
            "CloseDroptarget");
    }
}

#include "droptarget.moc"