#ifndef TODOTABLEMODEL_H
#define TODOTABLEMODEL_H

#include <QAbstractTableModel>
#include <QMouseEvent>
#include "todotxt.h"
#include "task.h"
#include <QUndoStack>
#include <vector>



#define TODOUR_INACTIVE "TODOUR_INACTIVE_794e26fdf5ea"


class TodoTableModel : public QAbstractTableModel
{
    Q_OBJECT
protected:
    todotxt *todo; // interface with files
    vector<task> task_set;

public:
    explicit TodoTableModel(QUndoStack* undo, QObject *parent = 0);
    ~TodoTableModel();
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;
 	bool setData(const QModelIndex & index, const QVariant & value, int role);
    int count();

    void addTask(task* t);
    void removeTask(QUuid tuid);
	task* getTask(QUuid tuid);
	task* getTask(QModelIndex index);

    void archive();
    void refresh();
            
   inline void clearFileWatch(){   todo->clearFileWatch();}; //gaetan 5/1/24
   inline void setFileWatch(QObject *parent){   todo->setFileWatch(parent);}; //gaetan 5/1/24


signals:
	    
public slots:
    
private:
	QUndoStack* _undo;
};

#endif // TODOTABLEMODEL_H
