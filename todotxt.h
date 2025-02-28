/* The Todo.txt master class.
  It handles the todo.txt and parses out information from it as well as writing it back.
  Also done.txt is handled here if needed
  */

#ifndef TODOTXT_H
#define TODOTXT_H

#define TODOFILE "todo.txt"
#define DONEFILE "done.txt"
#define DELETEDFILE "deleted.txt"

#include <vector>
#include <set>
#include <QString>
#include <QDate>
#include <QTemporaryDir>
#include <QFile>
#include <QObject>

#include "task.h"

using namespace std;

typedef enum {typetodofile,typedonefile,typedeletefile} filetype;

class todotxt : public QObject
{
    Q_OBJECT
public:
    explicit todotxt(QObject *parent = 0);
    ~todotxt();
//used by new model:
    void getAllTask(vector<task> &output);

   void clearFileWatch(); //gaetan 5/1/24
   void setFileWatch(QObject *parent); //gaetan 5/1/24

    // Undo and Redo
public:
    bool undo();   // Go backwards in the undo buffer without adding to it
    bool redo();  // go forward in the undo buffor without adding to it
    bool undoPossible(); // Say if undo is possible or not
    bool redoPossible(); // Say if redo is possible or not
	int write(vector<task>& content, filetype t, bool append);

protected:
    vector<QString> todo;
    vector<QString> done;
    set<QString> active_projects;
    set<QString> active_contexts;
    QTemporaryDir *undoDir;
    
    QString _TodoFilePath;
    QString _DoneFilePath;
    QString _DeleteFilePath;

	QFile* _TodoFile;
	QFile* _DoneFile;
	QFile* _DeleteFile;

	bool _ready;


protected:
//used by new model:
	
    QString getUndoDir(); // get the directory where we save undo stuff
    QString getNewUndoNameDirAndPrefix(); // get a new prefix to be used for creating new undo files
    void    cleanupUndoDir(); // Remove old files in the undo directory (not accessed for a while?)
    void    saveToUndo();      // Adds the current changes to the undo buffer. Also moves the undo pointer to the last item (cementing whatever changes have been done with undoredo)
    bool    checkNeedForUndo();
    void    restoreFiles(QString);

	
    	vector<QString> undoBuffer; // A buffer with base filenames for undos
    int 	undoPointer = 0; // Pointer into the undo buffer for undo and redo. Generally should be 0
	
private:
    void slurp(QFile* filename,vector<QString>&  content);
    
    
public slots:
	void fileModified(QString str);

};

#endif // TODOTXT_H
