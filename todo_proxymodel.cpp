#include <QSettings>
#include "todo_proxymodel.h"
#include <QDebug>

todoProxyModel::todoProxyModel(QObject *parent)
	:QSortFilterProxyModel(parent)
/*
*/{
  	this->actual_sort = todoProxyModel::sort_az;
	this->actual_filter= todoProxyModel::NoFilter;

	this->setFilterCaseSensitivity(Qt::CaseInsensitive);
	this->setFilterRole(Qt::DisplayRole);
	this->setFilterKeyColumn(1);
   filterText.setPattern("(?=^.*$)");
   filterText.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
   filterTextHasProject = false;
   this->setFilterRegularExpression(filterText);
   setDynamicSortFilter(false);
   
   this->m_collator = *new QCollator();
 }

todoProxyModel::~todoProxyModel()
	/*
*/{}

/*void todoProxyModel::sort(int column, Qt::SortOrder order)
{
	qDebug()<<"todoProxyModel calls sort"<<endline;
	QSortFilterProxyModel::sort(column, order);
}*/

/*void todoProxyModel::invalidate()
{
	qDebug()<<"todoProxyModel calls invalidate"<<endline;
	QSortFilterProxyModel::invalidate();
}*/

 bool todoProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
/* This is where we select the ordering. if left<right => return TRUE, otherwise return FALSE
we order based on
- order-by-az selected
- order-by-input-date selected
- inactive-last selected or not

Qt::UserRole+7 = completed
Qt::UserRole+2 = active
*/{
//	qDebug()<<"todoProxyModel::lessThan"<<endline;

	 	if (actual_sort.testFlag(todoProxyModel::inactive_last)){
			bool leftV=left.data(Qt::UserRole+2).toBool();
	 		if ( leftV ^ (right.data(Qt::UserRole+2).toBool()))
					return leftV; // sort inactive last
			leftV=right.data(Qt::UserRole+7).toBool();
			if (left.data(Qt::UserRole+7).toBool() ^ leftV)
					return leftV; // sort done last		
			}

		if (actual_sort.testFlag(todoProxyModel::sort_az))
					return m_collator.compare(left.data(Qt::DisplayRole).toString(), right.data(Qt::DisplayRole).toString())<0;   	
		
		if (actual_sort.testFlag(todoProxyModel::sort_idate))
//		    	return sourceModel()->data(left,Qt::UserRole+1).toDateTime() > sourceModel()->data(right,Qt::UserRole+1).toDateTime();
		    	return left.data(Qt::UserRole+1).toLongLong() > right.data(Qt::UserRole+1).toLongLong();
	
	qDebug()<<" todoProxyModel::sort - erreur sorting"<<endline;       
	return QSortFilterProxyModel::lessThan(left,right);
}

 bool todoProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
/* This is the function to filter. Depending on our internal settings + the value of sourceParent(sourceRow), we says yes or no.
We filter based on
- the filtertext : by parent class
- the "show inactive" selection (passed to here)
- the threshold dates
- the threshold contexts
- the "treat due as threshold" ?
- the "today's view". We should be getting a score from (??) 
*/{

	QModelIndex source_index = sourceModel()->index(sourceRow, 1, sourceParent);

// if Hide_inactive...
	if (actual_filter.testFlag(todoProxyModel::HideInactive) || actual_filter.testFlag(todoProxyModel::TodaysView)){
		if (!source_index.data(Qt::UserRole+2).toBool())   // isActive()
			return false;
		}

	if (actual_filter.testFlag(todoProxyModel::TodaysView)){ // hide inactive, threshold, D-priority, context-threshold, complete
		if (source_index.data(Qt::UserRole+7) == Qt::Checked) return false; // no Checked task
		if (source_index.data(Qt::UserRole+6).toChar()>'C')	return false; // Priority must be low
	}

// if ENHANCED_PM mode
	if (actual_filter.testFlag(todoProxyModel::EnhancedPM)){
	// 2.1 if a +project is selected, only show that one.
		if (!filterTextHasProject)//the filtertext doesn't contain a project (+...)
			if ( (! source_index.data(Qt::UserRole+5).toStringList().empty())
					|| (! source_index.data(Qt::UserRole+8).toStringList().empty() ) )
							return false;					
			}

		
qlonglong t_Date ;
// if Hide_th date
	if (!actual_filter.testFlag(todoProxyModel::ShowAll) && (actual_filter.testFlag(todoProxyModel::HideThresholdDate) || actual_filter.testFlag(todoProxyModel::TodaysView))){
		t_Date = source_index.data(Qt::UserRole+3).toLongLong();
		if( t_Date !=0 )
				return !(t_Date > this->now);
		}

// if Hide undue
	if (!actual_filter.testFlag(todoProxyModel::ShowAll) && (actual_filter.testFlag(todoProxyModel::HideUndue) || actual_filter.testFlag(todoProxyModel::TodaysView))){
		t_Date = source_index.data(Qt::UserRole+4).toLongLong();
		if( t_Date !=0 )
				return !(t_Date < dueWarningDate);
		}

// if Hide th_context
	if (!actual_filter.testFlag(todoProxyModel::ShowAll) && ((actual_filter.testFlag(todoProxyModel::HideThresholdContext) && actual_filter.testFlag(todoProxyModel::DefaultView) )|| actual_filter.testFlag(todoProxyModel::TodaysView))){
		QStringList task_contexts = source_index.data(Qt::UserRole+5).toStringList();
		for (const QString &i:contexts){
			if (task_contexts.contains(i)  //UserRole+5=threshold_context.
//					|| sourceModel()->data(source_index,Qt::UserRole+8).toStringList().contains(i)
					)
					return false;
			}
		}


	return QSortFilterProxyModel::filterAcceptsRow(sourceRow,sourceParent);

}

void todoProxyModel::refresh()
/* Ensure the filtering and sorting are updated
*/{
	this->invalidate();  //filter
	this->sort(1,Qt::AscendingOrder); //sorting
}


void todoProxyModel::setSortMode(TodourSortMode mode)
/* 
*/{
	if (mode == actual_sort) return;
	actual_sort = mode;
	refresh();

}

void todoProxyModel::setFilterMode(TodourFilterMode mode)
/*
*/{

//	qDebug()<<"setFilterMode: new filter="<<mode<<endline;
	if (mode == actual_filter) return;
	actual_filter=mode;

	QSettings settings;	
	beginFilterChange();
	this->dueWarningDate=QDateTime::currentDateTime().addDays(settings.value(SETTINGS_DUE_WARNING,DEFAULT_DUE_WARNING).toInt()).toSecsSinceEpoch();
	this->now = QDateTime::currentDateTime().toSecsSinceEpoch();
   endFilterChange(QSortFilterProxyModel::Direction::Rows);
	refresh();
}

void todoProxyModel::setContexts(QStringList newc)
/*
*/{
	this->contexts = newc;
	refresh();
}


void todoProxyModel::updateFilterText(QString filter)
/* For any reason, the filters have changed (toggle_threshold, text filter, ...)
	We need to adapt.
*/{
	QSettings settings;    
  	filterTextHasProject = false;
   
//	qDebug()<<"todoProxyModel::updateFilterText  previous filter="<<this->filterRegularExpression()<<endline;
		
   // Take the text of the format of match1 match2 !match3 and turn it into
    //(?=.*match1)(?=.*match2)(?!.*match3) - all escaped of course   	
	 QChar search_not_char = settings.value(SETTINGS_SEARCH_NOT_CHAR,DEFAULT_SEARCH_NOT_CHAR).toChar();
    
    QStringList words = filter.split(QRegularExpression("\\s+"));   
    QString regexpstring="(?=^.*$)"; // Seems a negative lookahead can't be first (!?), so this is a workaround
    #define START "(?=^.*"
    #define STARTN "(?!^.*"
    for(QString word:words){
        if(word.length()==0) break;
  			filterTextHasProject |= contexts.contains(word);

        if(word.at(0)==search_not_char){
        		regexpstring.append(STARTN);
           	regexpstring.append(QRegularExpression::escape(word.remove(0,1))+".*$)");
        }else{
	        regexpstring.append(START);
	        regexpstring.append(QRegularExpression::escape(word)+".*$)");
    	}
    }
   this->filterText.setPattern(regexpstring);	
//  	filterTextHasProject = filter.contains(QRegularExpression("(?:^|\\s+)\\+\\w*(?:\\s+|$)"));
  	
//  		qDebug()<<"todoProxyModel::updateFilterText: filterTextHasProject: "<<filterTextHasProject<<endline;
//  		qDebug()<<"   filter="<<filter<<endline;
	beginFilterChange();	
	
   this->setFilterRegularExpression(this->filterText);
   endFilterChange(QSortFilterProxyModel::Direction::Rows);
   refresh();
//   qDebug()<<"updateFilterText: filterTexthasProject="<<filterTextHasProject<<endline;
}
