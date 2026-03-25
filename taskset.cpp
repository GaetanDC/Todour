#include "taskset.h"
#include "def.h"

#include "todo_undo.h"

#include "todotxt.h"
//#include "caldav.h"

#include <QMessageBox>
#include <QRegularExpression>

taskset::taskset(QUndoStack* undo, QObject *parent):
	_undo(undo)
/* 
*/{
	Q_UNUSED(parent);
   QSettings settings;

   todo = new todotxt();
	QObject::connect(todo,SIGNAL(DataChanged()),this,SLOT(backendDataLoaded()));//REM
	QObject::connect(todo,SIGNAL(DataAvailable()),this,SLOT(backendDataLoaded()));//REM
	QObject::connect(todo,SIGNAL(DataSaved()),this,SIGNAL(dataSavedOK()));//REM
	if (todo->isReady())
		todo->reloadRequest();

	reloadContexts();
}
    
taskset::~taskset()
/* */{
	delete todo;//REM
}

////%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//Safe commands generate the action through the _undo stack

void taskset::safeComplete(int position, bool state)
/* Safely complete the tasks at position, creating an undo command*/{
	_undo->push(new CompleteCommand(this, content.at(position), state));
	}
	
void taskset::safeEdit(int position, QString _raw)
/* Safely edit the task at position, creating an undo command*/{
	_undo->push(new EditCommand(this, content.at(position),_raw));
	}
	
void taskset::safeAdd(task* _t)
/* Safely add a task, creating an undo command*/{
	_undo->push(new AddCommand(this,_t));
  	}
      	 
void taskset::safeDelete(QUuid index)
/* Safely delete a task, creating an undo command*/{
	_undo->push(new DeleteCommand(this,index));
	}
	
void taskset::safePostpone(int position, QString txt)
/* Safely postpone a task, creating an undo command*/{
	_undo->push(new PostponeCommand(this, content.at(position), txt));
	}

void taskset::safePriority(int position, QChar prio)
/* Safely change priority of a task, creating an undo command*/{ 
	_undo->push(new PriorityCommand(this,content.at(position), prio));
	}
	
void taskset::safeToggleComplete(int position)
/* Safely toggle the complete state*/
{
	_undo->push(new CompleteCommand(this, content.at(position), !content.at(position)->isComplete()));
	}


////%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//Other commands generate the action directly

void taskset::addTask(task* _t)
/* 
*/{
  	if (_t!=nullptr) content.push_back(_t);    
}

task* taskset::removeTask(QUuid tuid)
/* Remove the task, don't delete object
*/{
	Q_UNUSED(tuid);
	//	qDebug()<<"TodoTableModel::removeTask "<<tuid<<endline;
	task* ret = nullptr;
	for (vector<task*>::iterator i=content.begin();i!=content.end();++i){
		if ((*i)->getTuid() == tuid){
//			qDebug()<<"   found task: "<<(*i)->toString()<<endline;
			ret= *i;
			content.erase(i);
			return ret;
		}
	}
	return nullptr;
}
    

task* taskset::getTask(QUuid tuid)
/* 
*/{
	for (vector<task*>::iterator i=content.begin();i!=content.end();++i){
		if ((*i)->getTuid() == tuid)return *i;
	}
	return 0;    
}
    
task* taskset::getTask(int position)
/* 
*/{
	Q_UNUSED(position);
    return nullptr;
}

void taskset::flush()
/*  backend will emit signals, connect done above.
*/{
   todo->writeRequest(content,typeTodo,false); // append=false
}
    
void taskset::refreshActive()
/* 
*/{
qDebug()<<"DEPRECATED taskset::refreshActive()"<<endline;
    for (unsigned int i=0;i<content.size();i++){
    	content.at(i)->refreshActive(QDateTime::currentDateTime());
    	}
}
  
int taskset::size()
/* 
*/{
    return (int)content.size();
}
    
//cycle through all task to recalculate the active state
    
//   inline void clearFileWatch(){   todo->clearMonitoring();}; //gaetan 5/1/24



QString taskset::toString()
/* 
*/{
	return "";    
}
    
void taskset::backendDataLoaded()
/* 
*/{
  	todo->getAllTask(content);  
  		qDebug()<<"void taskset::backendDataLoaded() - all tasks loaded"<<endline;
}

void taskset::toggleDone(int position)
/* 
*/{
Q_UNUSED(position);
	//TODO: URGENT: update the undo framework for work with taskset and not model.
//	_undo->push(new CompleteCommand(this, content.at(position)));   
    }


void taskset::archive()
/* Remove all the "finished" tasks and move them to the "done" file.
#TODO  use UndoCommands ???
*/{
//	QAbstractItemModel::beginResetModel();

    vector<task*> done_set;
	qDebug()<<"TodoTableModel::archive content.size="<<content.size()<<endline;
    for(vector<task*>::const_iterator iter=content.begin();iter!=content.end();){
        if((*iter)->isComplete()){
        	qDebug()<<"TodoTableModel::archive move task "<<(*iter)->getEditText()<<endline;
            done_set.push_back((*iter));
            iter=content.erase(iter);
        } else {
            // No change
            iter++;
        }
     }
	this->flush();
    todo->writeRequest(done_set,typeDone,true); // append=true
//    QAbstractItemModel::endResetModel();
}


void taskset::setFileWatch(bool b, QObject *parent)
/* */{
   todo->setMonitoring(b, parent);
}



void taskset::reloadContexts()
/*
*/{
// for each tasks in content, find the context (#... or @...)
// compare found value with the actual list.
// if not in the list, add it
//	else skip

	for (vector<task*>::iterator itask=content.begin();itask!=content.end();++itask){
		for(QString s:(*itask)->getContexts()){
			if (! contexts.contains(s))
					contexts<<s;
			}
	}
	
	qDebug()<<"taskset::reloadContexts: "<<contexts<<endline;
}


void taskset::recalculate()
/* Data of display has been modified, refresh the internal data:
- through all tasks, 
*/{
	QSettings settings;
	QStringList words = settings.value(SETTINGS_INACTIVE,DEFAULT_INACTIVE).toString().split(';');
	for(vector<task*>::const_iterator iter=content.begin();iter!=content.end();++iter){
		(*iter)->setActive(true);
		for (QString i:words){
			if ((*iter)->getRaw().contains(i)){
				(*iter)->setActive(false);
				break;
			}
		}
	}

}


//*************************************************************************************************************************

noteset::noteset(QUndoStack* undo, QObject* parent)
/*
*/{
	Q_UNUSED(parent)
	Q_UNUSED(undo)
	notes=new notetxt(this);
	content="";	
	connect(notes,SIGNAL(WriteError(QString)),this, SIGNAL(backendError(QString)));
	connect(notes,SIGNAL(ReadError(QString)),this, SIGNAL(backendError(QString)));
	connect(notes,SIGNAL(DataSaved()),this, SIGNAL(dataSavedOK()));
	connect(notes,SIGNAL(DataAvailable()),this, SLOT(backendDataLoaded()));
	connect(notes,SIGNAL(DataChanged()),this, SLOT(backendDataLoaded()));
//	qDebug()<<"noteset::noteset created"<<endline;
}

void noteset::reLoad()
/*
*/{
	notes->reloadRequest();
}


noteset::~noteset()
/*
*/{
	delete notes;
}

void noteset::setFileWatch(bool b, QObject *parent)
/*
*/{
	Q_UNUSED(parent)
	Q_UNUSED(b)
	notes->setMonitoring(false,this); //disable filewatcher because we keep our version in memory and not on file...
}

QString noteset::toString()
/*
*/{
	return notes->toString();
}

void noteset::handleTextChanged(QString newtext)
/* receive text from UI, save it.
*/{

	notes->writeRequest(newtext);
}

void noteset::backendDataLoaded()
/*
*/{
//	qDebug()<<"noteset::backendDataLoaded start"<<endline;

	notes->getAllText(&content);
	emit updateText(content);

}


void noteset::flush()
/*
*/{
// Nothing to do.
}


