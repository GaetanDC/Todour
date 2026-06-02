#ifndef TODOTABLEMODEL_H
#define TODOTABLEMODEL_H

#include <QAbstractTableModel>
#include <QMouseEvent>
#include "task.h"
#include <QUndoStack>
#include <vector>
#include "taskset.h"
#include <QTableWidget>
#include <QTextEdit>

class TodoTableModel : public QAbstractTableModel{
    Q_OBJECT
protected:
	taskset* tasklist;
public:
    explicit TodoTableModel(taskset* _tasklist,QUndoStack* _undo, QObject *parent = 0);
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



	void safeAddSub(QString _title, bool _isDone, task* parent);
	void safeEditSub(QString _title, bool _isDone, int ID, task* parent);

	 void safeComplete(const QModelIndex & index, bool state);
	 void safeEdit(const QModelIndex & index, QString _raw);
    void safeAdd(QString title, QString context);
    void safeDuplicate(task* t);
    void safeDelete(QUuid index);
    void safePostpone(const QModelIndex & index, QString txt);
    void safePriority(const QModelIndex & index, QChar prio);
	 void safeToggleComplete(const QModelIndex & index);
	 void safeProgress(const QModelIndex & index);
//	 void flush();
//	 void archive();
//  void setFileWatch(bool b, QObject *parent);


	void startModelChange(QString desc);
	void endModelChange();

	inline void registerSubtasks(QTableWidget* n) {subtasksView = n;};
	inline void registerNotes(QTextEdit* n) {noteView = n;};
	inline void registerTableView(QTableView* n) {tableView = n;};

signals:
public slots:
	void updateViews(const QModelIndex & current, const QModelIndex & previous);
	void subEdited(int row, int col);
private:
	QUndoStack* undoS;
	
	QTableWidget* subtasksView;
	QTextEdit* noteView;
	QTableView* tableView;
};


#endif // TODOTABLEMODEL_H
