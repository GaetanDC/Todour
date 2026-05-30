#ifndef TASKSET_H
#define TASKSET_H
//a taskset represents a set of tasks. It gives access to the vector with some add work.

#include "task.h"
#include <QUndoCommand>
#include <vector>

class todo_backend;

class taskset:public QObject
{
	Q_OBJECT
	
private:

protected:
//	todo_backend  *todo;
   std::vector<task*> content;

public:
   explicit taskset(QObject *parent = 0);
   ~taskset();
	inline task* at(int position) const {return content.at(position);};



//for todoundo
   void addTask(task* t);
   task* removeTask(QUuid tuid);
      
//for mainwindow 
	void flush();
	void archive();
	inline QStringList getContexts(){return contexts;};


//for tablemodel
	inline int size() const {return content.size();};
	inline bool empty() const {return content.empty();};  //only in todotableModel

//for debug
	QString toString();

signals:
	void dataSavedOK();
	void backendError(QString);
	
public slots:
    void backendDataLoaded();
    void recalculate();
    
private:
	QStringList contexts;
	QStringList inactiveFlags;
};


#endif //TASKSET_H
