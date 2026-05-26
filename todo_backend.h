/* The todo_backend class
It shows the (virtual) interface of a backend, which can be implemented by any caldav.
	Ideas:	sql database of any other database
			any server
			any future
  */

#ifndef TODO_BACKEND_H
#define TODO_BACKEND_H

#include <vector>
#include <QObject>
#include <QString>

#include "task.h"
#include "taskset.h"

//using namespace std;
typedef enum {typeTodo,typeDone,typeDelete} TodoDestination;

class todo_backend : public QObject
{
    Q_OBJECT
public:
    inline todo_backend(QObject *parent = 0) : QObject(parent){};
    virtual ~todo_backend(){};

	virtual void getAllTask(std::vector<task*> &output)=0;
	virtual void reloadRequest()=0;

	virtual void setMonitoring(bool b, QObject *parent)=0;
	virtual void writeRequest(std::vector<task*>& content, TodoDestination t, bool append)=0;

	virtual bool isReady()=0;
	virtual QString getType()=0;

protected:    
 
private:    
    
public slots:

signals:
	void DataChanged();
//	void ConnectionLost();
	void DataAvailable();
	void DataSaved();
	void WriteError(QString ret);
	void ReadError(QString err);
	void FileSystemWatcherStatus(bool status);

 
};

#endif // TODO_BACKEND_H
