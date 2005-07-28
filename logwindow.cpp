/***************************************************************************
*                                logwindow.cpp
*                             -------------------
*
*    Revision     : $Id$
*    begin        : Tue Jan 29 2002
*    copyright    : (C) 2002 by Patrick Charbonnier
*                 : Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss
*    email        : pch@freeshell.org
*
****************************************************************************/

/**************************************************************************
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


#include <qlayout.h>
#include <q3listview.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QFrame>
#include <QGridLayout>
#include <QCloseEvent>

#include <klocale.h>
#include <kdialog.h>
#include <kaction.h>

#include "transfer.h"
#include "kmainwidget.h"
#include "logwindow.h"

#include <kapplication.h>
#include <q3textedit.h>

// // Replace regular space with nbsp
// QString replaceSpaces(const QString &str) {
//   QString res = str;
//   res.simplifyWhiteSpace();

//   int pos;
//   while ( (pos = res.find(' ')) != -1) {
//     res.replace(pos, 1, new QChar( 0x00a0 ), 1);
//   }

//   return res;
// }


static QString removeHTML(const QString & str)
{
    QString res = str;
    int pos;

    // replace <br/> with a newline
    while ((pos = res.find("<br/>")) != -1) {
        res.replace(pos, 4, "\n");
    }

    // remove all tags
    while ((pos = res.find('<')) != -1) {
        int pos2 = res.find('>', pos);

        if (pos2 == -1) {
            pos2 = res.length() + 1;
        }
        res.remove(pos, pos2 - pos + 1);
    }

    return res;
}


SeparatedLog::SeparatedLog(QWidget * parent):QWidget(parent)
{
    idSelected = 0;

    QGridLayout *topGridLayout = new QGridLayout(this, 1, 2, 0, KDialog::spacingHint());

    topGridLayout->setRowStretch(0, 5);

    topGridLayout->setColStretch(0, 3);
    topGridLayout->setColStretch(1, 10);

    lv_log = new Q3ListView(this);
    lv_log->setMultiSelection(false);
    lv_log->setAllColumnsShowFocus(true);
    lv_log->setSorting(-1);

    lv_log->addColumn(i18n("Id"), 40);
    lv_log->addColumn(i18n("Name"), 100);

    topGridLayout->addWidget(lv_log, 0, 0);

    connect(lv_log, SIGNAL(selectionChanged(Q3ListViewItem *)), SLOT(transferSelected(Q3ListViewItem *)));

    ml_log = new Q3TextEdit(this);
    ml_log->setTextFormat(LogText);
    ml_log->setMinimumSize(300, 200);
    ml_log->setVScrollBarMode(Q3ScrollView::Auto);
    ml_log->setWordWrap(Q3TextEdit::NoWrap);

    topGridLayout->addWidget(ml_log, 0, 1);
}


void SeparatedLog::addLog(uint id, const QString & filename, const QString & message)
{
    if (!trMap.contains(id)) {
        trMap.insert(id, message);
        Q3ListViewItem *last=lv_log->lastItem();
        new Q3ListViewItem(lv_log, last);
        last=lv_log->lastItem();
        last->setText(0, QString::number(id));
        last->setText(1, filename);
        // if I don't do this, ID#10 gets sorted between ID#1 and ID#2, ugly.      
    } else {
        trMap[id] += ("\n"+message);
    }

    if (idSelected == id) {
        refresh();
    }
}


void SeparatedLog::transferSelected(Q3ListViewItem * item)
{
    if (item) {
        idSelected = item->text(0).toUInt();
        // kapp->lock();
        ml_log->setText(trMap[idSelected]);
        // kapp->unlock();
    }
}


void SeparatedLog::refresh()
{
    if (idSelected > 0) {
        // kapp->lock();
        ml_log->setText(trMap[idSelected]);
        // kapp->unlock();
    }
}


////////////////////////




LogWindow::LogWindow():KDialogBase(Tabbed, i18n("Log Window"), Close, Close, 0, "", false)
{

    // add pages
    QFrame *page = addPage(i18n("Mixed"));
    QVBoxLayout *topLayout = new QVBoxLayout(page, 0, spacingHint());

    mixed_log = new Q3TextEdit(page);
    mixed_log->setTextFormat(LogText);
    mixed_log->setVScrollBarMode(Q3ScrollView::Auto);
    mixed_log->setWordWrap(Q3TextEdit::NoWrap);
    topLayout->addWidget(mixed_log);

    page = addPage(i18n("Separated"));
    topLayout = new QVBoxLayout(page, 0, spacingHint());
    sep_log = new SeparatedLog(page);
    topLayout->addWidget(sep_log);

    connect(this, SIGNAL(closeClicked()), this, SLOT(close()));

    // resize( 500, 300 );
}


void LogWindow::closeEvent(QCloseEvent *e)
{
    kmain->m_paShowLog->setChecked(false);
    kmain->b_viewLogWindow = false;
    KDialogBase::closeEvent( e );
}


void LogWindow::logGeneral(const QString & message)
{
    QString tmps = "<font color=\"blue\">" + QTime::currentTime().toString() + "</font> : <b>" + message + "</b>";

    mixed_log->append(tmps);
}


void LogWindow::logTransfer(uint id, const QString & filename, const QString & message)
{
    QString mixed_msg, single_msg, job_id;

    job_id = QString("Job[<font color=\"red\">%1</font>] : ").arg(id);
    mixed_msg = "<font color=\"blue\">" + QTime::currentTime().toString() + "</font> : " + job_id + message;

    single_msg = "<font color=\"blue\">" + QTime::currentTime().toString() + "</font> : " + message;

    mixed_log->append(mixed_msg);
    sep_log->addLog(id, filename, single_msg);
}


QString LogWindow::getText() const
{
    return removeHTML(mixed_log->text());
}

#include "logwindow.moc"
