/* This file is part of the KDE project

   Copyright (C) 2009 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef _TESTKGET_H
#define _TESTKGET_H

#include <QtTest/QtTest>

#include "observer.h"

class TestKGet: public QObject, public ModelObserver, public TransferGroupObserver, public TransferObserver
{
Q_OBJECT

public:
    TestKGet();
    
    void addedTransferGroupEvent(TransferGroupHandler * group);
    void removedTransferGroupEvent(TransferGroupHandler * group);

private slots:
    void simpleTest();
    void transferGroupTest();
    void transferGroupRepetitiveAddTest();
    
private:
    TransferGroupHandler * m_addedGH;
    TransferGroupHandler * m_removedGH;
};

#endif