#include "taskset.h"
#include "def.h"
#include "todotxt.h"
//#include "caldav.h"

#include <QMessageBox>
#include <QRegularExpression>

taskset::taskset(QObject *parent)
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

	recalculate();
}
    
taskset::~taskset()
/* */{
	delete todo;//REM
}


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

void taskset::flush()
/*  backend will emit signals, connect done above.
*/{
   todo->writeRequest(content,typeTodo,false); // append=false
}

QString taskset::toString()
/* 
*/{
	return "";    
}
    
void taskset::backendDataLoaded()
/* 
*/{
  	todo->getAllTask(content);  
//  		qDebug()<<"void taskset::backendDataLoaded() - all tasks loaded"<<endline;
}

void taskset::toggleDone(int position)
/* 
*/{
Q_UNUSED(position);
	//TODO: URGENT: update the undo framework for work with taskset and not model.
  
    }


void taskset::archive()
/* Remove all the "finished" tasks and move them to the "done" file.
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

void taskset::recalculate()
/* Data of display has been modified, refresh the internal data:
- through all tasks, 
*/{
	QSettings settings;
	inactiveFlags = settings.value(SETTINGS_INACTIVE,DEFAULT_INACTIVE).toString().split(";");
	for (vector<task*>::iterator itask=content.begin();itask!=content.end();++itask){
		for(QString s:(*itask)->getContexts()){
			if (! contexts.contains(s))
					contexts<<s;
		}

			//check all tasks for inactive keywords
		recalculateTask(*itask);
	}
	
	qDebug()<<"taskset::recalculate: "<<contexts<<endline;
}

void taskset::recalculateTask(task* wip)
/*
*/{
		wip->setActive(true);
		for (QString i:inactiveFlags){
			if (wip->getDisplayText().contains(i)){
				wip->setActive(false);
				break;
				}
			}

}


//*************************************************************************************************************************

noteset::noteset(QObject* parent)
/*
*/{
	Q_UNUSED(parent)
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


