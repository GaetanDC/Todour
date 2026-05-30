#include "taskset.h"
#include "def.h"
#include "todotxt.h"

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
		//recalculateTask(*itask);
		(*itask)->recalculate(inactiveFlags);
	}
	
	qDebug()<<"taskset::recalculate: "<<contexts<<endline;
}
