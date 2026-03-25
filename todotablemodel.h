#ifndef TODOTABLEMODEL_H
#define TODOTABLEMODEL_H

#include <QAbstractTableModel>
#include <QMouseEvent>
#include "task.h"
#include <QUndoStack>
#include <vector>
#include "taskset.h"

#define TODOUR_INACTIVE "TODOUR_INACTIVE_794e26fdf5ea"

class TodoTableModel : public QAbstractTableModel{
    Q_OBJECT
protected:
	taskset* tasklist;
public:
    explicit TodoTableModel(taskset* _tasklist, QObject *parent = 0);
    ~TodoTableModel();
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;
 	 bool setData(const QModelIndex & index, const QVariant & value, int role);
	 inline task* getTask(QModelIndex index) {return (tasklist->at(index.row()));};

    void refresh();    
//    void refreshActive(); //cycle through all task to recalculate the active state
	QString toString();


/* FUNCTIONS TO CONSIDER DROPPING HERE
	 void safeComplete(int position, bool state);
	 void safeEdit(int position, QString _raw);
    void safeAdd(task* _t);
    void safeDelete(QUuid index);
    void safePostpone(int position, QString txt);
    void safePriority(int position, QChar prio);
	 void flush();
	 void archive();
    void setFileWatch(bool b, QObject *parent);
	 */


signals:
	public slots:	
private:
//	eTaskCriticity taskCriticity(task* t) const;

};

#endif // TODOTABLEMODEL_H
