#ifndef TODO_PROXYMODEL_H
#define TODO_PROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QString>
#include "def.h"
#include "taskset.h"


using namespace std;





class todoProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT
  public:
    todoProxyModel(QObject *parent = nullptr);
    virtual ~todoProxyModel();


enum iTaskSortMode{
	no_sort=0,
	sort_az=0x2,
	sort_idate=0x4,
	inactive_last=0x8
	};
Q_DECLARE_FLAGS(TodourSortMode, iTaskSortMode)

enum iTaskFilterMode{
	NoFilter=0x0,
	TodaysView=0x1,
	HideThreshold=0x2,
	DueAsThreshold=0x4,
	ContextThreshold=0x8,
	ShowInactive=0x80
	};
Q_DECLARE_FLAGS(TodourFilterMode, iTaskFilterMode)

	virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
	virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
	
  public slots:
	inline TodourSortMode getSortMode() const{return actual_sort;};
	void setSortMode(TodourSortMode mode);
	
//	QString getTaksNum();
	
	inline TodourFilterMode getFilterMode() const{return actual_filter;};
	void setFilterMode(TodourFilterMode mode);

	inline QStringList getContexts()const {return contexts;};
	void setContexts(QStringList newc);	
	
	
	void updateFilterText(QString filter);
//	void updateFilterLevel(eTaskCriticity filt=Todour_NoFilter);
  	
  private:
  	QRegularExpression filterText;
  	TodourSortMode actual_sort;
  	TodourFilterMode actual_filter;
  	
  	QStringList contexts;
  	
  	
  	signals:
};

	Q_DECLARE_OPERATORS_FOR_FLAGS(todoProxyModel::TodourSortMode)
	Q_DECLARE_OPERATORS_FOR_FLAGS(todoProxyModel::TodourFilterMode)



#endif //#ifndef TODO_PROXYMODEL_H
