/*
Task represent 1 single task. They can be integrated in a taskset.
The internal representation is a QString, and we extract / integrate the data.
==> this allows compatibility with todo.txt files
==> this allows compatibility with Todour view
If one needs to create another storage interface (sql, online, ...) , the separate functions can be used

License GPL
Author Gaetan DC
*/

#ifndef TASK_H
#define TASK_H

#include <QString>
//#include <QDate>
#include <QColor>
#include <QUuid>
#include <QDebug>
#include <QDateTime>


enum eTaskCriticity{
Todour_NoFiltert=0,
Todour_TodaysViewt=2,
Todour_HideThresholdt=4,
Todour_DueAsThresholdt=8,
Todour_ShowInactivet=16
};


class task
{
private:
	QString _raw;
	QString displayText;
	QString description;
	QUuid _tuid;
	QDateTime _ttag;
	QDateTime _complete_date;
	QChar priority;
	QColor color;
	QDateTime inputD;
	QDateTime dueD;
	QDateTime thrD;
	Qt::CheckState complete ;
	bool active;
	QStringList contexts;
	QStringList thr_contexts;
	int progress;
	
public:
	task(QString s="", QString context="", bool loaded=false);
//	task(const task&);
	task(task* copy);
	task(QString s, QUuid tuid);

	~task();
	bool is_txt_compatible();

	void setDueDate(QDateTime d);
	void setThresholdDate(QDateTime d);
	void setInputDate(QDateTime d);
	void setColor(QString c);
	void setColor(QColor c);
	void setDescription(QString s);
	void setPriority(QChar c);
	task* setComplete(bool c = true); // manage the rec:...
	void setRaw(QString s);
	void setProgress(int);
	
	const QDateTime& getDueDate() const noexcept {return dueD;};
	const QDateTime& getThresholdDate() const noexcept {return thrD;};
	const QDateTime& getInputDate() const noexcept {return inputD;};
	const QDateTime& getTimeStamp() const noexcept {return _ttag;}  //for future sync

	const QStringList& getContexts() const noexcept {return contexts;};
	const QStringList& getThresholdContexts() const noexcept {return thr_contexts;};

	const QString& getDisplayText() const noexcept {return displayText;};	/* text for display in todour*/
	const QString& getRaw() const noexcept {return _raw;};	/* returns the full text, used for edit */
	const QString& getEditText() const noexcept {return _raw;};/* text for edit in Todour*/
	const QString& getDescription() const noexcept {return description;}; 	/* text for display in future tool. return only the descriptive part of the text, without t: due: color: utid: ...   TODO*/

	const QColor* getColor() const noexcept {return &color;};


	QChar getPriority() const noexcept {return priority;};
	QUuid getTuid() const noexcept {return _tuid;}
	Qt::CheckState isComplete() const noexcept {return complete;};
	bool operator==(task &t) const noexcept {return t.getTuid()==_tuid;}
	int getProgress() const noexcept {return progress;};
	bool isActive() const noexcept {return active;};


	void recalculateTask(QStringList inactiveflags);
	
	QString getURL() const noexcept;	
	
	QString toSaveString() const noexcept;
	/*returns the full QString for saving, including all hidden data.*/
	
	QString toSaveString_pureTODO() const noexcept;
	/*returns the full QString for saving, including all hidden data.*/
	
	QString toString() const noexcept;
	/*returns the full QString for debugging*/

// === static functions ===
	static QDateTime getRelativeDate(QString d, const QDateTime base);
	
	
protected:
	void parse(QString s, bool strict=false);
	void updateDisplayText();
	void updateDescription();
	void updateContexts();

};

#endif // TASK_H
