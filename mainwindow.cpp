#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "todotablemodel.h"

#include "settingsdialog.h"
#include "quickadddialog.h"
#include "aboutbox.h"
#include "def.h"

#include <QSortFilterProxyModel>
#include <QFileSystemWatcher>
#include <QTime>
#include <QDebug>
#include <QSettings>
#include <QShortcut>
#include <QCloseEvent>
#include <QtAwesome.h>
#include <QDesktopWidget>
#include <QDir>
#include <QSystemTrayIcon>
#include <QDesktopServices>
#include <QUuid>
#include <QPrinter>
#include <QPrintDialog>
#include <QTextDocument>

TodoTableModel *model=NULL;
QString saved_selection; // Used for selection memory

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    QString title=this->windowTitle();

    QCoreApplication::setOrganizationName("Nerdur");
    QCoreApplication::setOrganizationDomain("nerdur.com");
    QCoreApplication::setApplicationName("Todour");


#ifdef QT_NO_DEBUG
//    QCoreApplication::setOrganizationName("Nerdur");
//    QCoreApplication::setOrganizationDomain("nerdur.com");
//    QCoreApplication::setApplicationName("Todour");
//    title.append("-");
#else
//    QCoreApplication::setOrganizationName("Nerdur-debug");
//    QCoreApplication::setOrganizationDomain("nerdur-debug.com");
//    QCoreApplication::setApplicationName("Todour-Debug");
    title.append("-DEBUG-");
    qDebug()<<"Hello, we should be in debug mode."<<endline;
#endif
	QSettings settings;
	
    title.append(todour_version::get_version());
    baseTitle=title;
    this->setWindowTitle(title);

    // Check if we're supposed to have the settings from .ini file or not
    if(QCoreApplication::arguments().contains("-portable")){
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,QDir::currentPath());
        qDebug()<<"Setting ini file path to: "<<QDir::currentPath()<<endline;
    }

   hotkey = new UGlobalHotkeys();
   setHotkey();

    // Restore the position of the window
        restoreGeometry(settings.value( SETTINGS_GEOMETRY, saveGeometry() ).toByteArray());
    restoreState(settings.value( SETTINGS_SAVESTATE, saveState() ).toByteArray());
    if ( settings.value( SETTINGS_MAXIMIZED, isMaximized() ).toBool() )
        showMaximized();

    ui->lineEdit_2->setText(settings.value(SETTINGS_SEARCH_STRING,DEFAULT_SEARCH_STRING).toString());

    // Check that we have an UUID for this application (used for undo for example)
    if(!settings.contains(SETTINGS_UUID)){
        settings.setValue(SETTINGS_UUID,QUuid::createUuid().toString());
    }

    // Fix some font-awesome stuff
    QtAwesome* awesome = new QtAwesome(qApp);
    awesome->initFontAwesome();     // This line is important as it loads the font and initializes the named icon map
    awesome->setDefaultOption("scale-factor",0.9);
    ui->btn_Alphabetical->setIcon(awesome->icon( fa::sortalphaasc ));
    ui->pushButton_3->setIcon(awesome->icon( fa::signout ));
    ui->pushButton_4->setIcon(awesome->icon( fa::refresh ));
    ui->pushButton->setIcon(awesome->icon( fa::plus ));
//    ui->pushButton_2->setIcon(awesome->icon( fa::minus ));
    ui->context_lock->setIcon(awesome->icon(fa::lock));
    ui->pb_closeVersionBar->setIcon(awesome->icon(fa::times));

    // Set some defaults if they dont exist
    if(!settings.contains(SETTINGS_LIVE_SEARCH)){
        settings.setValue(SETTINGS_LIVE_SEARCH,DEFAULT_LIVE_SEARCH);
    }

    // Started. Lets open the todo.txt file, parse it and show it.
    parse_todotxt();
    setFileWatch();


    // Set up shortcuts . Mac translates the Ctrl -> Cmd
    // http://doc.qt.io/qt-5/qshortcut.html
    auto editshortcut = new QShortcut(QKeySequence(tr("Ctrl+n")),this);
    QObject::connect(editshortcut,SIGNAL(activated()),ui->lineEdit,SLOT(setFocus()));
    auto findshortcut = new QShortcut(QKeySequence(tr("Ctrl+f")),this);
    QObject::connect(findshortcut,SIGNAL(activated()),ui->lineEdit_2,SLOT(setFocus()));
    auto findshortcut2 = new QShortcut(QKeySequence(tr("F3")),this);
    QObject::connect(findshortcut2,SIGNAL(activated()),ui->lineEdit_2,SLOT(setFocus()));

    //auto contextshortcut = new QShortcut(QKeySequence(tr("Ctrl+l")),this);
    //QObject::connect(contextshortcut,SIGNAL(activated()),ui->context_lock,SLOT(setChecked(!(ui->context_lock->isChecked()))));
    QObject::connect(model,SIGNAL(dataChanged (const QModelIndex , const QModelIndex )),this,SLOT(dataInModelChanged(QModelIndex,QModelIndex)));

    /*
    These should now be handled in the menu system
    auto undoshortcut = new QShortcut(QKeySequence(tr("Ctrl+z")),this);
    QObject::connect(undoshortcut,SIGNAL(activated()),this,SLOT(undo()));
    auto redoshortcut = new QShortcut(QKeySequence(tr("Ctrl+r")),this);
    QObject::connect(redoshortcut,SIGNAL(activated()),this,SLOT(redo()));*/


    // Do this late as it triggers action using data
    ui->btn_Alphabetical->setChecked(settings.value(SETTINGS_SORT_ALPHA).toBool());
    ui->context_lock->setChecked(settings.value(SETTINGS_CONTEXT_LOCK,DEFAULT_CONTEXT_LOCK).toBool());
    updateSearchResults(); // Since we may have set a value in the search window

    ui->lv_activetags->hide(); //  Not being used yet
    ui->newVersionView->hide(); // This defaults to not being shown
    ui->actionShow_All->setChecked(settings.value(SETTINGS_SHOW_ALL,DEFAULT_SHOW_ALL).toBool());
    ui->actionStay_On_Top->setChecked(settings.value(SETTINGS_STAY_ON_TOP,DEFAULT_STAY_ON_TOP).toBool());
    ui->cb_threshold_inactive->setChecked(settings.value(SETTINGS_THRESHOLD_INACTIVE,DEFAULT_THRESHOLD_INACTIVE).toBool());
    stayOnTop();
    setTray();
    setFontSize();


    
    
    rClickMenu=new QMenu(this);
    editAction = new QAction("&Edit", this);
    deleteAction = new QAction("&Delete", this);
    postponeAction = new QAction("&Postpone +10p", this);
    duplicateAction = new QAction("&Duplicate", this);

   priorityMenu=new QMenu("Priority",this);
   rClickMenu->addMenu(priorityMenu);
   ApriorAction = new QAction("(A) priority",this);
   BpriorAction = new QAction("(B) priority",this);
   CpriorAction = new QAction("(C) priority",this);
   DpriorAction = new QAction("(D) priority",this);



    
   connect(editAction, SIGNAL(triggered()), this, SLOT(on_actionEdit()));
   connect(duplicateAction, SIGNAL(triggered()), this, SLOT(on_actionDuplicate()));
   connect(deleteAction, SIGNAL(triggered()), this, SLOT(on_actionDelete()));
   connect(postponeAction, SIGNAL(triggered()), this, SLOT(on_actionPostpone()));
   connect(ApriorAction,SIGNAL(triggered()),this,SLOT(on_actionPriorityA()));
   connect(BpriorAction,SIGNAL(triggered()),this,SLOT(on_actionPriorityB()));
   connect(CpriorAction,SIGNAL(triggered()),this,SLOT(on_actionPriorityC()));
   connect(DpriorAction,SIGNAL(triggered()),this,SLOT(on_actionPriorityD()));

//	connect(actionPrint,SIGNAL(triggered()),this,SLOT(on_actionPrint_triggered()));

    rClickMenu->addAction(deleteAction);
    rClickMenu->addAction(duplicateAction);
    rClickMenu->addAction(editAction);
    rClickMenu->addAction(postponeAction);
    priorityMenu->addAction(ApriorAction);
    priorityMenu->addAction(BpriorAction);
    priorityMenu->addAction(CpriorAction);
    priorityMenu->addAction(DpriorAction);
 

    // Version check
    Version = new todour_version();
//    connect(Version,&todour_version::NewVersion,this,&MainWindow::on_new_version);
    connect(Version,SIGNAL(NewVersion(QString)),this,SLOT(new_version(QString)));
	Version->onlineCheck(false);

//on_new_version("test");

}


// This method is for making sure we're re-selecting the item that has been edited
void MainWindow::dataInModelChanged(QModelIndex i1,QModelIndex i2){
    Q_UNUSED(i2);
    //qDebug()<<"Data in Model changed emitted:"<<i1.data(Qt::UserRole)<<"::"<<i2.data(Qt::UserRole)<<endl;
    //qDebug()<<"Changed:R="<<i1.row()<<":C="<<i1.column()<<endl;
//    saved_selection = i1.data(Qt::UserRole).toString();
//    resetTableSelection();
    updateTitle();

}


MainWindow::~MainWindow()
{
    delete ui;
    delete model;
}

QSortFilterProxyModel *proxyModel=NULL;

void MainWindow::updateTitle(){
    // The title is initialized to the name and version number at start and that is stored in baseTitle

    if(proxyModel != NULL){
        int visible = proxyModel->rowCount();
        int total = proxyModel->sourceModel()->rowCount();

        this->setWindowTitle(baseTitle+" ("+QString::number(visible)+"/"+QString::number(total)+")");
    }

    // Check the undo and redo meny items
    ui->actionUndo->setEnabled(model->undoPossible());
    ui->actionRedo->setEnabled(model->redoPossible());

}

// A simple delay function I pulled of the 'net.. Need to delay reading a file a little bit.. A second seems to be enough
// really don't like this though as I have no idea of knowing when a second is enough.
// Having more than one second will impact usability of the application as it changes the focus.
void delay()
{
    QTime dieTime= QTime::currentTime().addSecs(1);
    while( QTime::currentTime() < dieTime )
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}



void MainWindow::fileModified(const QString &str){
    Q_UNUSED(str);
    //qDebug()<<"MainWindow::fileModified  "<<watcher->files()<<" --- "<<str;
//    saveTableSelection();  // This should be maintained???
    model->refresh();
    if(model->count()==0){
        // This sometimes happens when the file is being updated. We have gotten the signal a bit soon so the file is still empty. Wait and try again
        delay();
        model->refresh();
        updateTitle();
    }
//    resetTableSelection();  // This should be maintained???
    setFileWatch();
}

void MainWindow::clearFileWatch(){
   model->clearFileWatch();
}

void MainWindow::setFileWatch(){
QSettings settings;
    if(settings.value(SETTINGS_AUTOREFRESH).toBool()==false)
           return;
    model->clearFileWatch();
   model->setFileWatch((QObject*) this);
}



void MainWindow::parse_todotxt(){

    model = new TodoTableModel(this);
    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    ui->tableView->setModel(proxyModel);
    //ui->tableView->resizeColumnsToContents();
    //ui->tableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
    ui->tableView->resizeColumnToContents(0); // Checkboxes kept small
    //ui->tableView->resizeRowsToContents(); Om denna körs senare blir det riktigt bra, men inte här..
}

void MainWindow::on_lineEdit_2_textEdited(const QString &arg1)
// This is the filter field
{
	QSettings settings;
    Q_UNUSED(arg1);
    bool liveUpdate = settings.value(SETTINGS_LIVE_SEARCH).toBool();
    if(!ui->actionShow_All->isChecked() && liveUpdate){
        updateSearchResults();
    }
}

void MainWindow::on_cb_threshold_inactive_stateChanged(int arg1)
// This is the "Show threshold" selection switch
{
	QSettings settings;
    settings.setValue(SETTINGS_THRESHOLD_INACTIVE,arg1);
//    on_pushButton_4_clicked(); // don't activate refresh, but act as a filter change.
	updateSearchResults();
}


void MainWindow::updateSearchResults(){
    // Take the text of the format of match1 match2 !match3 and turn it into
    //(?=.*match1)(?=.*match2)(?!.*match3) - all escaped of course
    
    // Shouldn't we integrate the "show threshold "
	QSettings settings;    
    QChar search_not_char = settings.value(SETTINGS_SEARCH_NOT_CHAR,DEFAULT_SEARCH_NOT_CHAR).toChar();
    QStringList words = ui->lineEdit_2->text().split(QRegularExpression("\\s+"));
    bool show_thr = (ui->cb_threshold_inactive->checkState() == Qt::Checked);
    QString regexpstring="(?=^.*$)"; // Seems a negative lookahead can't be first (!?), so this is a workaround
    for(QString word:words){
        QString start = "(?=^.*";
        if(word.length()>0 && word.at(0)==search_not_char){
            start = "(?!^.*";
            word = word.remove(0,1);
        }
        if(word.length()==0) break;
        regexpstring += start+QRegExp::escape(word)+".*$)";
    }
    QRegularExpression regexp(regexpstring);
    proxyModel->setFilterRegularExpression(regexp);
    //qDebug()<<"Setting filter: "<<regexp.pattern();
    proxyModel->setFilterKeyColumn(1);
    updateTitle();
}

void MainWindow::on_lineEdit_2_returnPressed()
{
	QSettings settings;
    bool liveUpdate = settings.value(SETTINGS_LIVE_SEARCH).toBool();

    if(!liveUpdate || ui->actionShow_All->isChecked()){
        updateSearchResults();
    }

}

void MainWindow::on_hotkey(){
    auto dlg = new QuickAddDialog();
    dlg->setModal(true);
    dlg->show();
    dlg->exec();
    if(dlg->accepted){
        this->addTodo(dlg->text);
    }
}

void MainWindow::setHotkey(){
	QSettings settings;
    if(settings.value(SETTINGS_HOTKEY_ENABLE).toBool()){
        hotkey->registerHotkey(settings.value(SETTINGS_HOTKEY,DEFAULT_HOTKEY).toString());
        connect(hotkey,&UGlobalHotkeys::activated,[=](size_t id){
            Q_UNUSED(id);
            on_hotkey();
        });
    } else {
        hotkey->unregisterAllHotkeys();
    }

}

void MainWindow::on_actionAbout_triggered(){
    AboutBox d;
    d.setModal(true);
    d.show();
    d.exec();
    //myanalytics->check_update();
}

void MainWindow::on_actionSettings_triggered()
// Which acton is this ??? ??? ???
{
    SettingsDialog d;
    d.setModal(true);
    d.show();
    d.exec();
    if(d.refresh){
        delete model;
        model = new TodoTableModel(this);
        proxyModel->setSourceModel(model);
        ui->tableView->setModel(proxyModel);
        ui->tableView->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
        ui->tableView->resizeColumnToContents(0);
//        saveTableSelection();
        model->refresh();// Not 100% sure why this is needed.. Should be done by re-setting the model above
//        resetTableSelection();
        setFileWatch();
        setTray();
        setFontSize();
        setHotkey();
    }
}

void MainWindow::setFontSize(){
	QSettings settings;
    int size = settings.value(SETTINGS_FONT_SIZE).toInt();
    if(size >0){
        auto f = qApp->font();
        f.setPointSize(size);
        qApp->setFont(f);
    }
}

void MainWindow::stayOnTop()
{
    if(ui->actionStay_On_Top->isChecked()){
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    } else {
        setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
    }
    show(); // This is needed as setWindowFlags can hide the window
}

void MainWindow::setTray(){
	QSettings settings;
    if(settings.value(SETTINGS_TRAY_ENABLED,DEFAULT_TRAY_ENABLED).toBool()){
        // We should be showing a tray icon. Are we?
        if(trayicon==NULL){
            trayicon = new QSystemTrayIcon(this);
            traymenu = new QMenu(this);
            minimizeAction = new QAction(tr("Mi&nimize"), this);
            connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));
            maximizeAction = new QAction(tr("Ma&ximize"), this);
            connect(maximizeAction, SIGNAL(triggered()), this, SLOT(showMaximized()));
            restoreAction = new QAction(tr("&Restore"), this);
            connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));
            quitAction = new QAction(tr("&Quit"), this);
            connect(quitAction, SIGNAL(triggered()), this, SLOT(cleanup()));
            connect(QApplication::instance(),SIGNAL(aboutToQuit()),this,SLOT(cleanup()));

            traymenu->addAction(minimizeAction);
            traymenu->addAction(maximizeAction);
            traymenu->addAction(restoreAction);
            traymenu->addSeparator();
            traymenu->addAction(quitAction);
            trayicon->setContextMenu(traymenu);
            connect(trayicon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                        this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

            trayicon->setIcon(QIcon(":/icons/todour.png"));
        }
        trayicon->show();
    }
    else{
        if(trayicon!=NULL){
            trayicon->hide();
        }
    }
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
            // Make sure the window is open
            this->show();
        break;
    case QSystemTrayIcon::MiddleClick:
        // Do nothing
        break;
    default:
        ;
    }
}

void MainWindow::on_pushButton_clicked()
{
    // Adding a new value into the model
   QString txt = ui->lineEdit->text();
   addTodo(txt);
   ui->lineEdit->clear();
}

void MainWindow::addTodo(QString &s){

    if(ui->context_lock->isChecked()){
        // The line should have the context of the search field except any negative search
        QStringList contexts = ui->lineEdit_2->text().split(QRegExp("\\s"));
        for(QString context:contexts){
         if(context.length()>0 && context.at(0)=='!') continue; // ignore this one
         if(!s.contains(context,Qt::CaseInsensitive)){
             s.append(" "+context);
         }
        }
    }
    model->add(s);
    updateTitle();
}

void MainWindow::on_lineEdit_returnPressed()
{
    on_pushButton_clicked();
}

void MainWindow::on_pushButton_3_clicked()
// Archive action.
{
//    saveTableSelection();
    model->archive();
//    resetTableSelection();
    updateTitle();
}

void MainWindow::on_pushButton_4_clicked()
// This is the Refresh button
{
//    saveTableSelection();
    model->refresh();
//    resetTableSelection();
    updateTitle();
}


//   GDE-NTM   REWORK NEEDED, no direct access to data.
/*void MainWindow::saveTableSelection(){
    qDebug()<<"   ---Launched saveTableSelection. Skipped --- "<<endline;
    //auto index = ui->tableView->selectionModel()->selectedIndexes();
    auto index = ui->tableView->selectionModel()->currentIndex();
//    if(index.count()>0){
    if(index.isValid()){
        // Vi har någonting valt.
        // qDebug()<<"Selected index: "<<index.at(0)<<endl;
        //saved_selection = model->data(index.at(0),Qt::UserRole).toString();
        saved_selection = model->data(index,Qt::UserRole).toString();
        //qDebug()<<"Selected text: "<<saved_selection<<endl;

    }
}

void MainWindow::resetTableSelection(){
    qDebug()<<"   ---Launched resetTableSelection. Skipped --- "<<endline;
/*    if(saved_selection.size()>0){
        // Set the selection again
        QModelIndexList foundIndexes = model->match(QModelIndex(),Qt::UserRole,saved_selection);
        if(foundIndexes.count()>0){
            currentIndex=foundIndexes.at(0);
    qDebug()<<"Found at index: "<<currentIndex<<endline;
            
//            ui->tableView->setFocus(Qt::MouseFocusReason);
//              ui->tableView->selectionModel()->setCurrentIndex(currentIndex, QItemSelectionModel::Select | QItemSelectionModel::Current);
//            ui->tableView->setFocusProxy(ui->tableView->indexWidget(currentIndex));
            ui->tableView->selectionModel()->select(currentIndex,QItemSelectionModel::Select | QItemSelectionModel::Current);

    qDebug()<<"MainWindow::resetTableSelection - index="<<ui->tableView->selectionModel()->currentIndex()<<endline;
//            ui->tableView->setCurrentIndex(currentIndex);
            //ui->tableView->setFocus(Qt::TabFocusReason);
            ui->tableView->setFocus(Qt::TabFocusReason);
       }
    }
    saved_selection="";

}*/

void MainWindow::cleanup(){
	QSettings settings;

    settings.setValue( SETTINGS_GEOMETRY, saveGeometry() );
    settings.setValue( SETTINGS_SAVESTATE, saveState() );
    settings.setValue( SETTINGS_MAXIMIZED, isMaximized() );
    settings.setValue(SETTINGS_SEARCH_STRING,ui->lineEdit_2->text());
    if(trayicon!=NULL){
        delete trayicon;
        trayicon = NULL;
    }
    qApp->quit();
}

void MainWindow::closeEvent(QCloseEvent *ev){

    if (trayicon != NULL && trayicon->isVisible()) {
            hide();
            ev->ignore();
    } else {
        cleanup();
        ev->accept();
    }

}

void MainWindow::on_btn_Alphabetical_toggled(bool checked)
{
	QSettings settings;
    settings.setValue(SETTINGS_SORT_ALPHA,checked);
    on_pushButton_4_clicked(); // Refresh
}

void MainWindow::on_context_lock_toggled(bool checked)
{
	QSettings settings;
    settings.setValue(SETTINGS_CONTEXT_LOCK,checked);
}




void MainWindow::on_pb_closeVersionBar_clicked()
{
    ui->newVersionView->hide();
}


void MainWindow::undo()
{
    if(model->undo())
        model->refresh();
}

void MainWindow::redo()
{
    if(model->redo())
        model->refresh();
}



void MainWindow::on_actionCheck_for_updates_triggered()
{
	Version->onlineCheck(true);
}

void MainWindow::on_tableView_customContextMenuRequested(const QPoint &pos)
{
    // This is used for triggering opening of a link.
    // Find out where we are

    auto index = ui->tableView->indexAt(pos);
    QString URL=ui->tableView->model()->data(index,Qt::UserRole+1).toString();
    qDebug()<<"mainwindow.cpp - contextmenu clicked on"<<ui->tableView->model()->data(index,Qt::UserRole).toString()<<endline;

    rClickMenu->popup(ui->tableView->viewport()->mapToGlobal(pos));

    if(!URL.isEmpty()){
        QDesktopServices::openUrl(URL);
    }
    
}

void MainWindow::on_actionQuit_triggered()
{
    cleanup();
}

void MainWindow::on_actionUndo_triggered()
{
    undo();
    updateTitle();
}

void MainWindow::on_actionRedo_triggered()
{
    redo();
    updateTitle();
}

void MainWindow::on_actionShow_All_changed()
{
	QSettings settings;
    settings.setValue(SETTINGS_SHOW_ALL,ui->actionShow_All->isChecked());
    on_pushButton_4_clicked();
}

void MainWindow::on_actionStay_On_Top_changed()
{
	QSettings settings;
    settings.setValue(SETTINGS_STAY_ON_TOP,ui->actionStay_On_Top->isChecked());
    stayOnTop();
}

void MainWindow::on_actionManual_triggered()
{
    QDesktopServices::openUrl(QUrl("https://sverrirvalgeirsson.github.io/Todour"));
}

void MainWindow::on_actionEdit()
{
   QModelIndexList indexes = ui->tableView->selectionModel()->selection().indexes();
    if(!indexes.empty()){
       qDebug()<<"mainwindow->onedit index="<<indexes.at(0)<<endline;
        ui->tableView->edit(indexes.at(0));
    }

}

void MainWindow::on_actionDelete()
{
   // Remove the selected item
//   saveTableSelection();
    QModelIndexList indexes = ui->tableView->selectionModel()->selection().indexes();
    // Only support for deleting one item at a time
    if(!indexes.empty()){
        //QModelIndex i=indexes.at(0);
        //QString t=model->data(proxyModel->mapToSource(i),Qt::UserRole).toString(); // User Role is Raw data
        //model->remove(t);
        model->remove(indexes.at(0));
    }
    updateTitle();
//resetTableSelection();
}

void MainWindow::on_actionPostpone()
//
{
//   saveTableSelection();
   QModelIndexList indexes = ui->tableView->selectionModel()->selection().indexes();
    if(!indexes.empty()){
        QModelIndex i=indexes.at(0);
 //       QString t=model->data(proxyModel->mapToSource(i),Qt::UserRole).toString(); // User Role is Raw data
        model->append(i,"t:+10p");
    }
    updateTitle();
//resetTableSelection();
}

void MainWindow::on_actionDuplicate()
//
{
//	saveTableSelection();
	if(!saved_selection.isEmpty()){
		saved_selection +=" bis";
        model->add(saved_selection);
   }
//   resetTableSelection();
   updateTitle();
   
   QModelIndexList indexes = ui->tableView->selectionModel()->selection().indexes();
   if(!indexes.empty()){
   qDebug()<<"mainwindow->onDuplicate index="<<indexes.at(0)<<endline;
      ui->tableView->edit(indexes.at(0)); 
      }
//      indexes = ui->tableView->selectionModel()->selection().indexes();
 //     if(!indexes.empty()){
  //      ui->tableView->edit(indexes.at(0));
   //   }
   
 }

   void MainWindow::on_actionPriorityA(){
   QModelIndexList indexes = ui->tableView->selectionModel()->selection().indexes();
    if(!indexes.empty())
    	model->setPriority(indexes.at(0),"A");
}

   void MainWindow::on_actionPriorityB(){
   QModelIndexList indexes = ui->tableView->selectionModel()->selection().indexes();
    if(!indexes.empty())
    	model->setPriority(indexes.at(0),"B");
}

   void MainWindow::on_actionPriorityC(){
   QModelIndexList indexes = ui->tableView->selectionModel()->selection().indexes();
    if(!indexes.empty())
    	model->setPriority(indexes.at(0),"C");
}

   void MainWindow::on_actionPriorityD(){
   QModelIndexList indexes = ui->tableView->selectionModel()->selection().indexes();
    if(!indexes.empty())
    	model->setPriority(indexes.at(0),"D");
}

    void MainWindow::new_version(QString text)
	// show "alarm" of new version available (status bar? Notification? balloon tooltip?)
    {
		ui->txtLatestVersion->setText(text);
		ui->newVersionView->show();
    }

void MainWindow::on_actionPrint_triggered()
/* Print the selection of tasks
*/
{
	qDebug()<<"on_actionPrint_triggered()"<<endline;
    auto selection = ui->tableView->selectionModel();
//    if(index.count()>0){
    if(selection->hasSelection()){
	qDebug()<<"on_actionPrint_triggered(): step1"<<endline;
		QPrinter printer;
		
		QPrintDialog dialog(&printer, this);
		dialog.setWindowTitle(tr("Print Tasks"));
//		dialog.addEnabledOption(index);
		if (dialog.exec() != QDialog::Accepted)
			return;
		QString txt_str;
		QModelIndexList list=selection->selection().indexes();
		for (QList<QModelIndex>::iterator i=list.begin(); i!=list.end();++i)
		{
			txt_str=txt_str + "<br>";
			txt_str=txt_str+i->data(Qt::DisplayRole).toString();
		}
		qDebug()<<"on_actionPrint_triggered(): txt_str"<<txt_str<<endline;
		QTextDocument text(this);
		text.setHtml(txt_str);
		text.print(&printer);
	}
}
