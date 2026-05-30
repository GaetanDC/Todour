#include "taskset.h"
#include "def.h"
#include "todotxt.h"
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>	

taskset::taskset(QObject *parent)
/* 
*/{
	Q_UNUSED(parent);
   QSettings settings;

	//SQL
	// 1. open the database
	// 2. prepare and run the query
	// 3. create and push all the tasks
   QSqlDatabase todoDB = QSqlDatabase::addDatabase("QSQLITE","todoDB");
   todoDB.setDatabaseName("/home/gaetan/todo.db");
   if (! todoDB.open()){
        qDebug()<<todoDB.lastError().driverText()<<endline;
        qDebug()<<todoDB.lastError().databaseText()<<endline;
        emit backendError("Unable to open database");
        }
	else {
   	qDebug()<<"mainWindow: database is open."<<endline;
	   //this->setTable("tasks");
      
      QSqlQuery query = *new QSqlQuery("SELECT "
      		"ID," // value(0)
      		"text," // value(1)
      		"inputDate," // value(2)
      		"doneDate," // value(3)
      		"dueDate," // value(4)
      		"thresDate," // value(5)
      		"isDone," // value(6)
      		"progress," // value(7)
      		"tags," // value(8)
      		"color," // value(9)
      		"priority," // value(10)
      		"recurrence," // value(11)
      		"longText " // value(12)
      		"FROM tasks WHERE Archived=0"
      		,QSqlDatabase::database("todoDB",false));
      task* t;
      while (query.next()) {
      	qDebug()<<query.value(0).toInt()<<" "<<query.value(1).toString()<<endline;
			t = new task(query.value(0).toInt(),query.value(1).toString(),query.value(6).toBool());
			
			t->setInputDate(QDateTime::fromString(query.value(2).toString(),"yyyy-MM-dd"));
			t->setDoneDate(QDateTime::fromString(query.value(3).toString(),"yyyy-MM-dd"));
			t->setDueDate(QDateTime::fromString(query.value(4).toString(),"yyyy-MM-dd"));
			t->setThresholdDate(QDateTime::fromString(query.value(5).toString(),"yyyy-MM-dd"));
			t->setColor(QColor::fromString(query.value(9).toString()));
			t->setDescription(query.value(12).toString());
			t->setPriority(query.value(10).toChar());
			t->setProgress(query.value(7).toInt());
			t->setRecurrence(query.value(11).toString());
			t->setContexts(query.value(8).toStringList());
			 
			 //Load here the subtasks.
			 
		    addTask(t);
      	 }
      }
	recalculate();
}
    
taskset::~taskset()
/* */{
//	delete todo;//REM
}


void taskset::addTask(task* _t)
/* 
*/{
  	if (_t!=nullptr) content.push_back(_t);    
}

task* taskset::removeTask(QUuid tuid)
/* Remove the task, don't delete object
TODO: check this for working with SQL...
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
//   todo->writeRequest(content,typeTodo,false); // append=false
}

QString taskset::toString()
/* 
*/{
	return "";    
}
    
void taskset::backendDataLoaded()
/* 
*/{
  	//todo->getAllTask(content);  // COMMENTED FOR SQL.
//  		qDebug()<<"void taskset::backendDataLoaded() - all tasks loaded"<<endline;
}

void taskset::archive()
/* Remove all the "finished" tasks and move them to the "done" file.
*/{
//	QAbstractItemModel::beginResetModel();

	QSqlQuery query = *new QSqlQuery(QSqlDatabase::database("todoDB",false));
	for(vector<task*>::const_iterator iter=content.begin();iter!=content.end();iter++){
   	if((*iter)->isComplete()){
			query.prepare("UPDATE tasks"
						"SET Archived = 1"
						"WHERE ID=:id");
			query.bindValue(":id", (*iter)->getDbIndex());
			query.exec();
			}
     }
	this->flush();
//    todo->writeRequest(done_set,typeDone,true); // append=true
//    QAbstractItemModel::endResetModel();
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
