/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/


#ifndef VIEWSCONTAINER_H
#define VIEWSCONTAINER_H

#include <QWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedLayout>
#include <QMap>

class KTitleWidget;

class TransfersView;
class TransfersViewDelegate;
class TransferHandler;

class TitleBar : public QWidget
{
    Q_OBJECT
    public:
        TitleBar(QWidget * parent = 0);

        void setTransfer(TransferHandler * transfer);
        void setDownloadsWindow();
        void setFinishedWindow();

    private:
        KTitleWidget *m_titleWidget;
};

class ButtonBase : public QToolButton
{
    Q_OBJECT
    public:
        ButtonBase(QWidget * parent = 0);

    public slots:
        virtual void slotToggled(bool checked);

    signals:
        void activated();
};

class TransfersButton : public ButtonBase
{
    Q_OBJECT
    public:
        TransfersButton();

    public slots:
        void addTransfer(TransferHandler * transfer);
        void removeTransfer(TransferHandler * transfer);
        void setTransfer(TransferHandler * transfer);

    signals:
        void selectedTransfer(TransferHandler * transfer);

    private slots:
        void slotToggled(bool checked);
        void slotActionTriggered(QAction *);

    private:
        TransferHandler * m_selectedTransfer;
        QMenu * m_menu;
        QMap<QAction *, TransferHandler *> m_transfersMap;
};

class ViewsContainer : public QWidget
{
    Q_OBJECT
    public:
        ViewsContainer(QWidget * parent = 0);

    public slots:
        void setExpandableDetails(bool show);
        void showTransferDetails(TransferHandler * transfer);
        void closeTransferDetails(TransferHandler * transfer);

        void showDownloadsWindow();
        void showFinishedWindow();

    private slots:
        void slotTransferSelected(TransferHandler * transfer);

    private:
        QWidget         * m_bottomBar;
        QVBoxLayout     * m_VLayout;
        QHBoxLayout     * m_HLayout;
        QStackedLayout  * m_SLayout;

        TitleBar        * m_titleBar;

        TransfersView   * m_transfersView;
        TransfersViewDelegate * m_transfersViewDelegate;

//         QWidget         * m_finishedView;  //TODO: This view has still to be created.

        ButtonBase      * m_downloadsBt;
//         ButtonBase      * m_finishedBt;
        TransfersButton * m_transfersBt;

        QMap<TransferHandler *, QWidget *> m_transfersMap;
};

#endif
