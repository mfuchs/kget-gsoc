#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <qobject.h>
#include <qvaluelist.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <kurl.h>

class QString;

class Transfer;

class GlobalStatus
{
public:
	// not yet used
	QDateTime timeStamp;
	struct {
		QString interface;
		float speed;
		float maxSpeed;
		float minSpeed;
	} connection;
	struct {
		float totalSize;
		float percentage;
		int transfersNumber;
	} files;
	QStringList others;
};



class Scheduler : public QObject
{
Q_OBJECT
public:
	Scheduler();
	~Scheduler();
	
	enum Operation {};
	
signals:
	void addedItems(QValueList<Transfer *>);
	void removedItems(QValueList<Transfer *>);
	void changedItems(QValueList<Transfer *>);
	void clear();
	void globalStatus(GlobalStatus *);
	
public slots:
	void slotNewURLs(const KURL::List &);
	void slotNewURL(const KURL &);
    
	void slotRemoveItems(QValueList<Transfer *>);
	void slotRemoveItem(Transfer *);
    
	void slotSetPriority(QValueList<Transfer *>, int);
	void slotSetPriority(Transfer *, int);
    
	void slotSetOperation(QValueList<Transfer *>, enum Operation);
	void slotSetOperation(Transfer *, enum Operation);
    
	void slotSetGroup(QValueList<Transfer *>, const QString &);
	void slotSetGroup(Transfer *, const QString &);

    /**
     * Used to read the user's transfer list
     */
    void slotImportTransfers();
    void slotExportTransfers();
    
public:
    // temp. changed to public becouse some of the following are used by kmainwidget..
    void readTransfers(bool);
    void readTransfersEx(const KURL & file);
    void writeTransfers();
    void addTransfers(const KURL::List& src, const QString& destDir);
    void addTransfer(const QString& src);
    void addTransfersEx(const KURL& url, const KURL& destFile);

    void sanityChecksSuccessful( const KURL& url );
    
    void checkQueue();
};

#endif
