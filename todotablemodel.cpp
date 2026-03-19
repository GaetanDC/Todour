#include "todotablemodel.h"
#include "def.h"

#include <QFont>
#include <QRegularExpression>
#include <QDebug>
#include <vector>

//#include "todo_undo.h"

TodoTableModel::TodoTableModel(taskset* _tasklist, QObject *parent) :
    QAbstractTableModel(parent), tasklist(_tasklist)
{}

TodoTableModel::~TodoTableModel(){}

int TodoTableModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
//Should we consider here the quantity of SHOWN rows?
    int size = (int)tasklist->size();
    return size;
    }

int TodoTableModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 2;
 }


QVariant TodoTableModel::headerData(int section, Qt::Orientation orientation, int role) const
/*
*/{
    Q_UNUSED(orientation);
    if(role == Qt::DisplayRole)
    {
      if(section==0) return "Done";
      if(section==1) return "Todo";
    }
  return QVariant::Invalid;
}


Qt::ItemFlags TodoTableModel::flags(const QModelIndex& index) const
/*
*/{
  Qt::ItemFlags returnFlags = QAbstractTableModel::flags(index);
  if (index.column() == 0)  {
    returnFlags |= Qt::ItemIsUserCheckable;
  }
  if (index.column() == 1)  {
     returnFlags |= Qt::ItemIsEditable | Qt::ItemNeverHasChildren;
  }
  return returnFlags;
}


QVariant TodoTableModel::data(const QModelIndex &index, int role) const
/* returns data requested by the modelView
*/{
	QSettings settings;
	if (!index.isValid()) return QVariant();
	if (tasklist->empty()) return QVariant();
    if (index.row() >= (int) tasklist->size() || index.row() < 0) return QVariant();
    if (role == Qt::DisplayRole || role==Qt::ToolTipRole){
        if(index.column()==1){
            return QString(tasklist->at(index.row())->getDisplayText());
        }
     }

    if (role == Qt::EditRole) {
        if(index.column()==1){
            QString s=tasklist->at(index.row())->getEditText();
            return s;
        }
     }

    if(role == Qt::CheckStateRole) {
        if(index.column()==0) return tasklist->at(index.row())->isComplete();
    }

    if(role == Qt::FontRole) {
        if(index.column()==1){
            QFont f;
            if (! tasklist->at(index.row())->isActive()){
            	 f.fromString(settings.value(SETTINGS_INACTIVE_FONT).toString());
            } else {
                 f.fromString(settings.value(SETTINGS_ACTIVE_FONT).toString());
            }
			 f.setStrikeOut(tasklist->at(index.row())->isComplete());

            return f;
        }
    }

    if (role == Qt::ForegroundRole) {
        int due=INT_MAX;
    	QSettings settings;
    	if(settings.value(SETTINGS_DUE).toBool()){
    		if (tasklist->at(index.row())->getDueDate()->isValid()){  //CHANGE
				due = -(int) (tasklist->at(index.row())->getDueDate()->daysTo(QDateTime::currentDateTime()));  //CHANGE
			}
		}
        bool active = true;
        if(tasklist->at(index.row())->isComplete()){ //CHANGE
            active = false;
        }

        if(active && due<=0){
            // We have passed due date
        	return QVariant::fromValue(QColor::fromRgba(settings.value(SETTINGS_DUE_LATE_COLOR,DEFAULT_DUE_LATE_COLOR).toUInt()));
        }
        else 
        	if(active && due<=settings.value(SETTINGS_DUE_WARNING,DEFAULT_DUE_WARNING).toInt()){
            	return QVariant::fromValue(QColor::fromRgba(settings.value(SETTINGS_DUE_WARNING_COLOR,DEFAULT_DUE_WARNING_COLOR).toUInt()));
        	}
       		else 
       			if(!tasklist->at(index.row())->isActive()){ //CHANGE
            		return QVariant::fromValue(QColor::fromRgba(settings.value(SETTINGS_INACTIVE_COLOR,DEFAULT_INACTIVE_COLOR).toUInt()));
        		}
        		else {
            		return QVariant::fromValue(QColor::fromRgba(settings.value(SETTINGS_ACTIVE_COLOR,DEFAULT_ACTIVE_COLOR).toUInt()));
        		}
    		}

    if (role == Qt::BackgroundRole && index.column()==1){ // proxy: soit rien ne change, soit on désactive les couleurs de fond en idea?
		if (tasklist->at(index.row())->getColor()->isValid()) //CHANGE
			return QVariant::fromValue(tasklist->at(index.row())->getColor()->lighter(180));  //CHANGE
	}

		//Qt::UserRole is used for sorting. To use the power of regexp and QProxyModel, we will add a special code TODOUR_INACTIVE to be removed with the new proxy  TO BE REMOVED !!!
    if(role == Qt::UserRole){
		QString _prepend="";
		if (settings.value(SETTINGS_SEPARATE_INACTIVES,DEFAULT_SEPARATE_INACTIVES).toBool())
			_prepend=(QChar) QChar::LastValidCodePoint;
	   
	   	if (tasklist->at(index.row())->isActive()){ //CHANGE
	    	return tasklist->at(index.row())->getEditText()+" "+TODOUR_INACTIVE;  //CHANGE
	    }
	    else{
	    	return _prepend + tasklist->at(index.row())->getEditText(); //CHANGE
	    }
	 }

	if(role == Qt::UserRole+1){  //UserRole+1 returns inputdate
	qDebug()<<"todotablemodel.cpp: inputdate="<<tasklist->at(index.row())->getInputDate()->toString("yyyy-MM-dd")<<endline;
		return QVariant(*(tasklist->at(index.row())->getInputDate()));
	}

	if(role == Qt::UserRole+2){  //UserRole+2 returns criticity (int)
		return taskCriticity(tasklist->at(index.row()));
      }
    
	return QVariant();
}


bool TodoTableModel::setData(const QModelIndex & index, const QVariant & value, int role)
/* 
*/{
	QAbstractItemModel::beginResetModel();

    if(index.column()==0 && role == Qt::CheckStateRole)
  	 	tasklist->safeComplete(index.row(),value.toBool());  
    else if(index.column()==1 && role == Qt::EditRole)
		tasklist->safeEdit(index.row(),value.toString());
    else return false;
    QAbstractItemModel::endResetModel();
  	return true;
}



void TodoTableModel::refresh()
/*// what is exactly the scope of this?
// it is activated by the click on "Reresh" button, requesting to reload the file from disk.
// This is only possible if we trust more the file than our internal info.
// Options : remove, de-activate
//		or make a check of the file, if different, propose to reload
//		or consider that we are saving any change immediately, and the file is trusted.*/
{
//#TODO  if we saved the last modifications, reload from the source.
// if we did not save, shows a confirmation window, as we risk to loose the last changes
//  note: this issue is only with todo.txt files. With a synchronisation-capable, we should be safe to reload.
// For the moment, do nothing

//    QAbstractItemModel::beginResetModel();
//    todo->refresh();
//    todo_data.clear();

    QAbstractItemModel::beginResetModel();
    QAbstractItemModel::endResetModel();


    emit dataChanged(QModelIndex(),QModelIndex());

//    QAbstractItemModel::endResetModel();
}

QString TodoTableModel::toString()
/* Debugging function only
*/{
	return QString("model : ")+"\n"+QString("  Contains n tasks: ")+QString::number((int)tasklist->size());
}

eTaskCriticity TodoTableModel::taskCriticity(task* t) const
/*returns task criticity
- the "show inactive" selection (passed to here)
- the threshold dates
- the threshold contexts
- the "treat due as threshold" ?
- the "today's view". We should be getting a score from (??) 

Todour_NoFilter=0,
Todour_TodaysView=2,
Todour_HideThreshold=4,
Todour_DueAsThreshold=8,
Todour_ShowInactive=16


LEVEL 1 : task with inactive keyword  (2)
LEVEL 2 : task with threshold in future  (4)
LEVEL 3 : task with threshold context		(8)
LEVEL 4 : task with due-reminder in the past	(16)
LEVEL 5 : task with A-priority, old input date	(32)

*/{
	QSettings settings;
//	qDebug()<<"taskCriticity step 1"<<endline;
	QStringList words = settings.value(SETTINGS_INACTIVE,DEFAULT_INACTIVE).toString().split(';');
	for (QString i:words){
		if (t->getRaw().contains(i)){
//		qDebug()<<"taskCriticity step 1.1"<<endline;
			return Todour_ShowInactive;
			break;
			}
		}
// qDebug()<<"taskCriticity step 2"<<endline;
	if (*(t->getThresholdDate()) > QDateTime::currentDateTime()){
		return Todour_HideThreshold;
	}
	
	
	
//	qDebug()<<"taskCriticity step 3"<<endline;
	if (t->getDueDate()->addDays(- settings.value(SETTINGS_DUE_WARNING,DEFAULT_DUE_WARNING).toInt()) < QDateTime::currentDateTime()){
		return Todour_DueAsThreshold;
	}
// qDebug()<<"taskCriticity step 4"<<endline;
	return Todour_NoFilter;

}
/*	QSettings settings;
	bool ret=true;
	
	ret &= (QDateTime::currentDateTime() >= *(t->getThresholdDate()));
	if (!ret) return ret;
	
	QStringList words = settings.value(SETTINGS_INACTIVE,DEFAULT_INACTIVE).toString().split(';');
	for (QString i:words){
		ret &= ! t->getRaw().contains(i);
		if (!ret) return ret;
		}
	// control context threshold?
	// control due if DUE_AS_THRESHOLD
	
	return ret;
	*/
