#include "todo_undo.h"
#include "def.h"
#include <QDebug>

AddCommand::AddCommand(taskset* _list, task* _t, QUndoCommand *parent)
    : QUndoCommand(parent),  _task(_t),tasklist(_list)
/* */
{
	query =new QSqlQuery(QSqlDatabase::database("todoDB",false));
	query->prepare("INSERT INTO tasks (id, forename, surname) "
                  "VALUES (:id, :forename, :surname)");
    query->bindValue(":id", 1001);
    query->bindValue(":forename", "Bart");
    query->bindValue(":surname", "Simpson");

	setText("New task");
}

AddCommand::~AddCommand()
/* */{
	delete query;
}

void AddCommand::undo()
/* undo() of addCommand is a remove
*/
{
	tasklist->removeTask(_task->getTuid());
}

void AddCommand::redo()
/* redo() add is adding again
*/
{    
	tasklist->addTask(_task);
}

int AddCommand::id() const
{return -1;}

bool AddCommand::mergeWith(const QUndoCommand *other)
{Q_UNUSED(other);
return false;}


// §§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§

DeleteCommand::DeleteCommand(taskset* _list, QUuid index, QUndoCommand *parent)
    : QUndoCommand(parent),  tasklist(_list),_tuid(index) 
/*	_task = is initialised in the redo */
{
	query = new QSqlQuery(QSqlDatabase::database("todoDB",false));
	query->prepare("DELETE FROM tasks "
                  "WHERE (id = :id)");
    query->bindValue(":id", 1001);

	setText("Delete");
}

DeleteCommand::~DeleteCommand()
/* */{
	delete query;
	}

void DeleteCommand::undo()
/* UNDO DELETE means we have to add a task in the list.
*/
{
	tasklist->addTask(_task);
}

void DeleteCommand::redo()
/* */
{
	// as far as I know, <vector> doesn't actually delete the object.
	_task = tasklist->removeTask(_tuid);
}

int DeleteCommand::id() const
/* */
{
	return -1;
}

bool DeleteCommand::mergeWith(const QUndoCommand *other)
/* */
{
	Q_UNUSED(other);
	return false;
}


// §§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§

EditCommand::EditCommand(taskset* _list, task* t, QString _new_raw, QUndoCommand *parent)
    :QUndoCommand(parent), tasklist(_list), _task(t), new_raw(_new_raw)
/* */
{
	query = new QSqlQuery(QSqlDatabase::database("todoDB",false));
	query->prepare("UPDATE tasks "
						"SET text = :text "
						"WHERE ID= :id");
    query->bindValue(":id", 1001);
    query->bindValue(":text", _new_raw);
//#TODO: we have only 1 edit command for all the edits. But we will need to split this in multiple.
// #BUG it won't work as is.
// OPTION1 is to group all the "edit" undoCommands, get info in parameters, and create the SQL
// OPTION 2 is to create a type of undoCommand for all possible undos.  with inheritance, is should be easy...
// ANYWAY, we have the detail info above, we should not loose it in the function call.

	old_raw = t->getRaw();
	setText("Edit");
}

EditCommand::~EditCommand()
/* */{
	delete query;
	}

void EditCommand::undo()
/* */
{
	_task->setRaw(old_raw);
}

void EditCommand::redo()
/* */
{
	_task->setRaw(new_raw);
}

int EditCommand::id() const
/**/
{
	return -1;
}

bool EditCommand::mergeWith(const QUndoCommand *other)
/* */
{
	Q_UNUSED(other);
	return false;
}

// §§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§

CompleteCommand::CompleteCommand(taskset* _list, task* t, bool complete, QUndoCommand *parent)
    :QUndoCommand(parent), _task(t),_complete(complete), tasklist(_list)
/* */
{
	query = new QSqlQuery(QSqlDatabase::database("todoDB",false));
	query->prepare("UPDATE tasks "
						"SET isDone = :isdone "
						"WHERE ID= :id");
    query->bindValue(":id", 1001);
    query->bindValue(":isdone", complete);

	setText("Complete");
	rec_task = nullptr;
}

CompleteCommand::CompleteCommand(taskset* _list, task* t, QUndoCommand *parent)
    :QUndoCommand(parent), _task(t), tasklist(_list)
/* */
{
	setText("Complete");
	rec_task = nullptr;
	_complete=!t->isComplete();
}

CompleteCommand::~CompleteCommand()
/* #TODO  check
*/{
	delete query;
	}

void CompleteCommand::undo()
/* */
{
	_task->setComplete(!_complete);
	if (rec_task != nullptr)
			tasklist->removeTask(rec_task->getTuid());
	
}

void CompleteCommand::redo()
/* */
{
	rec_task=_task->setComplete(_complete);
	if (rec_task != nullptr)
			tasklist->addTask(rec_task);
}

int CompleteCommand::id() const
/* */
{
	return -1;
}

bool CompleteCommand::mergeWith(const QUndoCommand *other)
/* */
{
	Q_UNUSED(other);
	return false;
}

// §§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§

PriorityCommand::PriorityCommand(taskset* _list, task* t, QChar prio, QUndoCommand *parent)
    :QUndoCommand(parent), _task(t), _priority(prio), tasklist(_list)
/* */
{
	query = new QSqlQuery(QSqlDatabase::database("todoDB",false));
	query->prepare("UPDATE tasks "
						"SET priority = :priority "
						"WHERE ID= :id");
    query->bindValue(":id", 1001);
    query->bindValue(":priority", prio);

	setText("Priority");
	_p_priority = _task->getPriority();
}

PriorityCommand::~PriorityCommand()
/* #TODO  check
*/{
	delete query;
	}

void PriorityCommand::undo()
/* */
{
	_task->setPriority(_p_priority);
}

void PriorityCommand::redo()
/* */
{
	_task->setPriority(_priority);
}

int PriorityCommand::id() const
/* */
{
	return -1;
}

bool PriorityCommand::mergeWith(const QUndoCommand *other)
/* */
{
	Q_UNUSED(other);
	return false;
}


// §§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§

PostponeCommand::PostponeCommand(taskset* _list, task* t, QString postp, QUndoCommand *parent)
    :EditCommand(_list, t,"",parent)
/* */
{
//	QSettings
	setText("Postpone");
	new_raw = old_raw + " " + postp;
}

PostponeCommand::~PostponeCommand()
/*
*/{
}

// §§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§

ProgressCommand::ProgressCommand(task* t, int pcvalue, QUndoCommand *parent)
:QUndoCommand(parent), _task(t)
/*
*/{
	previousValue = _task->getProgress();
	newValue=min(99,previousValue+pcvalue);

	query = new QSqlQuery(QSqlDatabase::database("todoDB",false));
	query->prepare("UPDATE tasks "
						"SET progress = :progress "
						"WHERE ID= :id");
    query->bindValue(":id", 1001);
    query->bindValue(":progress", newValue);
	
}

ProgressCommand::~ProgressCommand()
/*
*/{
	delete query;
}
void ProgressCommand::undo()
/*
*/{
	_task->setProgress(previousValue);
}
void ProgressCommand::redo()
/*
*/{
	_task->setProgress(newValue);

}
int ProgressCommand::id() const
/*
*/{
	return 1;
}
bool ProgressCommand::mergeWith(const QUndoCommand *other)
/*
*/{
	Q_UNUSED(other)
	return false;
}
