#include "mdiviewinterface.h"
#include "scheduler.h"
#include "transferlist.h"

MdiViewInterface::MdiViewInterface( Scheduler * sched )
{
    // Incoming data: connect shceduler's signals to local slots
    connect( sched, SIGNAL( addedItems(TransferList &) ),
	     this, SLOT( schedulerAddedItems(TransferList &) ) );
    connect( sched, SIGNAL( removedItems(TransferList &) ),
	     this, SLOT( schedulerRemovedItems(TransferList &) ) );
    connect( sched, SIGNAL( changedItems(TransferList &) ),
	     this, SLOT( schedulerChangedItems(TransferList &) ) );
    connect( sched, SIGNAL( clear() ),
	     this, SLOT( schedulerCleared() ) );
    connect( sched, SIGNAL( globalStatus(GlobalStatus *) ),
	     this, SLOT( schedulerStatus(GlobalStatus *) ) );

    // Outgoing data: connect local signals to scheduler's slots
    connect( this, SIGNAL( newURLs(const KURL::List &, const QString &) ),
	     sched, SLOT( slotNewURLs(const KURL::List &, const QString &) ) );
    connect( this, SIGNAL( removeItems(TransferList &) ),
	     sched, SLOT( slotRemoveItems(TransferList &) ) );
    connect( this, SIGNAL( setPriority(TransferList &, int) ),
	     sched, SLOT( slotSetPriority(TransferList &, int) ) );
    connect( this, SIGNAL( setOperation(TransferList &, TransferOperation) ),
	     sched, SLOT( slotSetOperation(TransferList &, TransferOperation) ) );
    connect( this, SIGNAL( setGroup(TransferList &, const QString &) ),
	     sched, SLOT( slotSetGroup(TransferList &, const QString &) ) );
}


void MdiViewInterface::schedulerCleared()
{}

void MdiViewInterface::schedulerAddedItems( TransferList &)
{}

void MdiViewInterface::schedulerRemovedItems( TransferList &)
{}

void MdiViewInterface::schedulerChangedItems( TransferList &)
{}

void MdiViewInterface::schedulerStatus( GlobalStatus * )
{}

#include "mdiviewinterface.moc"
