/* This file was taken from the AMAROK project
   The code that follows is by Max Howell, that licensed it under
   the terms of the GNU General Public License as published by the
   Free Software Foundation; version 2. The COPYING file Max referred
   to contained such license.

   All mods : Copyright (C) 2004 KGet2 Developers < >
   Last synced: 2004-May-8
*/

// Maintainer: Max Howell (C) Copyright 2004
// Copyright:  See COPYING file that comes with this distribution
//

#ifndef _BROWSERBAR_H
#define _BROWSERBAR_H

#include <qhbox.h>        //baseclass
#include <qpushbutton.h>  //baseclass
#include <qvaluevector.h> //stack allocated

typedef QValueVector<QWidget*> BrowserList;
typedef QValueVector<QWidget*>::ConstIterator BrowserIterator;

class KMultiTabBar;
class KMultiTabBarTab;
class QEvent;
class QObject;
class QObjectList;
class QPixmap;
class QPushButton;
class QResizeEvent;
class QSignalMapper;
class QVBox;

class BrowserBar : public QWidget
{
    Q_OBJECT

public:
    BrowserBar( QWidget *parent );
    ~BrowserBar();

    QVBox   *container() const { return (QVBox*)m_playlist; }
    QWidget *browser( const QCString& ) const;
    uint     position() const { return m_pos; }

    void     addBrowser( QWidget*, const QString&, const QString& );

protected:
    bool eventFilter( QObject*, QEvent* );
    bool event( QEvent* );

public slots:
    void showHideBrowser( int = -1 );
    void close() { showHideBrowser(); }

private:
    void adjustWidgetSizes();

    static const int DEFAULT_HEIGHT = 50;
    static const int MINIMUM_WIDTH = 200;
    static const int MAXIMUM_WIDTH = 300;

    uint             m_pos; //the x-axis position of m_divider
    QVBox           *m_playlist; //not the playlist, but parent to the playlist and searchBar
    QWidget         *m_divider; //a qsplitter like widget
    KMultiTabBar    *m_tabBar;
    BrowserList      m_browsers; //the browsers are stored in this qvaluevector
    QWidget         *m_browserHolder; //parent widget to the browsers
    QWidget         *m_currentBrowser; //currently displayed page, may be 0
    KMultiTabBarTab *m_currentTab;  //currently open tab, may be 0

    QSignalMapper   *m_mapper; //maps tab clicks to browsers
};

#endif

