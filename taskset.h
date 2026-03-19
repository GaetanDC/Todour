#ifndef TASKSET_H
#define TASKSET_H
//a taskset represents a set of tasks. Historically, it was grouped with the todotablemodel

#include "todo_backend.h"
#include "task.h"
#include <QUndoCommand>
#include <vector>

class todo_backend;

class taskset:public QObject
{
	Q_OBJECT
	
private:

protected:
	todo_backend  *todo;
   std::vector<task*> content;  //RENAME THIS

public:
   explicit taskset(QUndoStack* undo, QObject *parent = 0);
   ~taskset();

   void addTask(task* t); // this could maybe be set as protected + friend of undo.
   task* removeTask(QUuid tuid); // this could maybe be set as protected + friend of undo.
   
   void safeComplete(int position, bool state);
	void safeToggleComplete(int position);
	void safeEdit(int position, QString _raw);
   void safeAdd(task* _t);
   void safeDelete(QUuid index);
   void safePostpone(int position, QString txt);
   void safePriority(int position, QChar prio);

	task* getTask(QUuid tuid);   //Only used in TodoTableModel
	task* getTask(int position);  // Not implemented. Only used in todoTableModel
	inline task* at(int position) const {return content.at(position);};
	inline bool empty() const {return content.empty();};  //only in todotableModel
	inline int size() const {return content.size();};
	inline QStringList getContexts(){return contexts;};
    
   void refreshActive(); //cycle through all task to recalculate the active state  #TODO to be removed ??
	void flush();
   void setFileWatch(bool b, QObject *parent);
	QString toString();
	int size();  //only in todotableModel
	void archive();
	void reloadContexts();
	int taskCriticity(task* t);

signals:
	void dataSavedOK();
	void backendError(QString);
	
public slots:
    void backendDataLoaded();
    void toggleDone(int tuid);
private:
	QUndoStack* _undo;
	QStringList contexts;
};

#include "notetxt.h"

class noteset : public QObject
{

Q_OBJECT

public:
	noteset(QUndoStack* undo, QObject* parent);
	~noteset();

	void handleTextChanged(QString newtext); // text changed in UI, write it on disk. Only launched by save or equiv.

   void setFileWatch(bool b, QObject *parent); //cannot be used.
	void flush();
	QString toString();
	void reLoad();	

	
public slots:
   void backendDataLoaded();						// text-file rady to load

signals:
	void backendError(QString txt);
	void dataSavedOK();
	void updateText(QString txt);

private:
	notetxt* notes;
	QString content;
};


#endif //define TASKSET_H
