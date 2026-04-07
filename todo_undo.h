#ifndef TODOUNDO_H
#define TODOUNDO_H

#include <QUndoCommand>
#include "task.h"
//#include "todotablemodel.h"
#include "taskset.h"

using namespace std;

//TODO: consider a masterclass doing all the common stuff + being friend of taskset?
class taskset;

class AddCommand : public QUndoCommand
{
public:
    explicit AddCommand(taskset* _list, task* _t, QUndoCommand *parent = nullptr);
    ~AddCommand();

    void undo() override;
    void redo() override;
    int id() const;
	bool mergeWith(const QUndoCommand *other);
	
protected:
   task* _task;
	taskset* tasklist; //ref to tasklist to add / remove
};


class DeleteCommand : public QUndoCommand
{
public:
    explicit DeleteCommand(taskset* _list, QUuid index, QUndoCommand *parent = nullptr);
	~DeleteCommand();
    void undo() override;
    void redo() override;
	int id() const;
	bool mergeWith(const QUndoCommand *other);
	
protected:
	task* _task;
	taskset* tasklist; //ref to tasklist to add / remove
 	QUuid _tuid;


};


class EditCommand : public QUndoCommand
{
public:
    explicit EditCommand(taskset* _list, task* t, QString _new_raw, QUndoCommand *parent = nullptr);
	~EditCommand();
    void undo() override;
    void redo() override;
	int id() const;
	bool mergeWith(const QUndoCommand *other);
	
protected:
	taskset* tasklist;
	task* _task;
	QString old_raw;
	QString new_raw;
	 //ref to tasklist to add / remove
    };


class CompleteCommand : public QUndoCommand
{
public:
    explicit CompleteCommand(taskset* _list, task* t, bool complete, QUndoCommand *parent = nullptr);
    explicit CompleteCommand(taskset* _list, task* t, QUndoCommand *parent = nullptr);
	~CompleteCommand();
    void undo() override;
    void redo() override;
	int id() const;
	bool mergeWith(const QUndoCommand *other);
	
protected:
	task* _task;
	task* rec_task;
	bool _complete;
	taskset* tasklist; //ref to tasklist to add / remove

};

class PriorityCommand : public QUndoCommand
{
public:
    explicit PriorityCommand(taskset* _list, task* t, QChar prio, QUndoCommand *parent = nullptr);
	~PriorityCommand();
    void undo() override;
    void redo() override;
	int id() const;
	bool mergeWith(const QUndoCommand *other);
	
protected:
	task* _task;
	QChar _priority;
	QChar _p_priority;
	taskset* tasklist; //ref to tasklist to add / remove

};

class PostponeCommand : public EditCommand
{
public:
    explicit PostponeCommand(taskset* _list, task* t, QString _postp, QUndoCommand *parent = nullptr);
	~PostponeCommand();
	
private:

};

class ProgressCommand : public QUndoCommand
{
public:
	explicit ProgressCommand(task* t, int pcvalue, QUndoCommand *parent = nullptr);
	~ProgressCommand();
	void undo() override;
   void redo() override;
	int id() const;
	bool mergeWith(const QUndoCommand *other);
	
private:
	int previousValue;
	int newValue;
	task* _task;
};

#endif // TODOUNDO_H
