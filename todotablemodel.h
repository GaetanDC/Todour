#ifndef TODOTABLEMODEL_H
#define TODOTABLEMODEL_H

#include <QAbstractTableModel>
#include <QMouseEvent>
#include "task.h"
#include <QUndoStack>
#include <vector>
#include "taskset.h"

class TodoTableModel : public QAbstractTableModel{
    Q_OBJECT
protected:
	taskset* tasklist;
public:
    explicit TodoTableModel(taskset* _tasklist,QUndoStack* _undo, QObject *parent = 0);
    ~TodoTableModel();
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;
 	 bool setData(const QModelIndex & index, const QVariant & value, int role);
	 inline task* getTask(QModelIndex index) {return (tasklist->at(index.row()));};

    void refresh();    
//    void refreshActive(); //cycle through all task to recalculate the active state
	QString toString();


	 void safeComplete(const QModelIndex & index, bool state);
	 void safeEdit(const QModelIndex & index, QString _raw);
    void safeAdd(task* _t);
    void safeAdd(QString s, QString c="");
    void safeDelete(QUuid index);
    void safePostpone(const QModelIndex & index, QString txt);
    void safePriority(const QModelIndex & index, QChar prio);
	 void safeToggleComplete(const QModelIndex & index);
	 void safeProgress(const QModelIndex & index);
	void safeDueDate(const QModelIndex & index, QDateTime d);

//	 void flush();
//	 void archive();
//  void setFileWatch(bool b, QObject *parent);


	inline void startModelChange(){this->beginResetModel();};
	inline void endModelChange(){this->endResetModel();};

signals:

public slots:
	void dataChanged(QModelIndex index);
private:
	QUndoStack* undoS;

};

#endif // TODOTABLEMODEL_H
