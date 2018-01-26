#include "mainwindow.h"
#include "config.h"
#include "filenames.h"
#include "ui_mainwindow.h"
#include "SceneParser.h"
#include "SceneRenamer.h"
#include "FileScanner.h"
#include "Scene.h"
#include "Actor.h"
#include "ActorProfileView.h"
#include <QtConcurrent>
#include <QInputDialog>
#include <QtGlobal>
#include <QDebug>
#include <QMessageBox>
#include <QVector>
#include <QFileDialog>
#include <QRegExp>
#include <QSplitter>
#define COMBO_BOX_DEFAULT "No Selection"
#define LOAD_ACTORS
#define LOAD_SCENES
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
//    this->runMode = Debug;

    this->runMode = Release;//Debug;
    this->setWindowIcon(QIcon(QPixmap("SceneQuery.icns")));
    qRegisterMetaType<int>("int");
    qRegisterMetaType<QString>("QString");
    qRegisterMetaType<QStringList>("QStringList");
    qRegisterMetaType<QVector<int>>("QVector<int>");
    qRegisterMetaType<QHash<int, ScenePtr>>("QHash<int, ScenePtr>");
    qRegisterMetaType<QSharedPointer<Scene>>("QSharedPointer<Scene>");
    qRegisterMetaType<QSharedPointer<Actor>>("QSharedPointer<Actor>");
    qRegisterMetaType<QVector<QSharedPointer<Scene>>>("QVector<QSharedPointer<Scene>>");
    qRegisterMetaType<QVector<QSharedPointer<Actor>>>("QVector<QSharedPointer<Actor>>");
    qRegisterMetaType<ScenePtr>("ScenePtr");
    qRegisterMetaType<ActorPtr>("ActorPtr");
    qRegisterMetaType<SceneList>("SceneList");
    qRegisterMetaType<ActorList>("ActorList");
    qRegisterMetaType<RowList>("RowList");
    qRegisterMetaType<QFileInfoList>("QFileInfoList");
    this->videoOpen = false;
    QCoreApplication::setOrganizationName("SQ");
    QCoreApplication::setApplicationName("Scene Query");

    ui->setupUi(this);
    this->sql       = new SQL();
    this->sqlThread = new QThread();
    sql->moveToThread(sqlThread);
    connect(sql,                SIGNAL(startProgress(QString,int)),     this,               SLOT(newProgressDialog(QString, int)));
    connect(sql,                SIGNAL(updateProgress(int)),            this,               SLOT(updateProgressDialog(int)));
    connect(sql,                SIGNAL(closeProgress()),                this,               SLOT(closeProgressDialog()));
    connect(vault.data(),       SIGNAL(save(ActorPtr)),                 sql,                SLOT(updateActor(ActorPtr)));
    connect(vault.data(),       SIGNAL(save(ScenePtr)),                 sql,                SLOT(saveChanges(ScenePtr)));
    connect(vault.data(),       SIGNAL(save(ActorList)),                sql,                SLOT(store(ActorList)));
    connect(vault.data(),       SIGNAL(save(SceneList)),                sql,                SLOT(store(SceneList)));
    sqlThread->start();

    /// Set up GUI Classes
    setupViews();
    connectViews();

    /// Open Splashscreen
    this->splashScreen = new SplashScreen(this);
    connect(splashScreen, SIGNAL(sendActorRows(RowList)), ui->actorTableView,   SLOT(addRows(RowList)));
    connect(splashScreen, SIGNAL(sendSceneRows(RowList)), ui->sceneTableView,   SLOT(addRows(RowList)));
    connect(splashScreen, SIGNAL(sendActors(ActorMap)),   vault.data(),         SLOT(setMap(ActorMap)));
    connect(splashScreen, SIGNAL(sendScenes(SceneMap)),   vault.data(),         SLOT(setMap(SceneMap)));
    connect(splashScreen, SIGNAL(done()),                 this,                 SLOT(initDone()));
    qDebug("Opening Splashscreen...");
    splashScreen->show();

}

MainWindow::~MainWindow(){
    delete ui;
    if (curlThread){
        curlThread->quit();
        curlThread->wait();
        delete curlThread;
    }
    if (sqlThread){
        sqlThread->quit();
        sqlThread->wait();
        delete sqlThread;
    }
}

/** \brief Set up the main display */
void MainWindow::setupViews(){
    this->vault     = QSharedPointer<DataManager>(new DataManager());
    /// Set Up Scene Detail View
    this->sceneDetailView = new SceneDetailView(this);
    ui->vbl_list_layout->addWidget(sceneDetailView);
    // Hide detail widgets
    this->sceneDetailView->hide();
    this->ui->tabWidget_filters->hide();
    this->ui->profileWidget->hide();
    /// Give all other GUI Classes Access to data.
    this->sceneDetailView->setDataContainers(vault);
    this->ui->actorTableView->setDataContainers(vault);
    this->ui->sceneTableView->setDataContainers(vault);
    this->ui->profileWidget->setData(vault);
    /// Empty the progress bar.
    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(0);
    /// Set up the Filters window
    QStringList numFilters;
    numFilters << "More Than" << "Less Than" << "Exactly";
    ui->cb_sceneCount->addItems(numFilters);
}

void MainWindow::connectViews(){
    /// Make Relevant connections between widgets
    connect(ui->profileWidget,          SIGNAL(hidden()),                       ui->sceneTableView, SLOT(clearActorFilterOnly()));
    connect(ui->sceneTableView,         SIGNAL(displayChanged(int)),            ui->lcd_shownSceneCount,SLOT(display(int)));
    connect(ui->actorTableView,         SIGNAL(displayChanged(int)),            ui->lcd_actorCount, SLOT(display(int)));
    connect(ui->profileWidget,          SIGNAL(profileChanged(ActorPtr)),       ui->sceneTableView, SLOT(actorFilterChanged(ActorPtr)));
    connect(ui->profileWidget,          SIGNAL(chooseNewPhoto()),               this,               SLOT(selectNewProfilePhoto()));
    connect(ui->profileWidget,          SIGNAL(apv_to_mw_requestScenes(QString)),this,              SLOT(apv_to_mw_receiveSceneListRequest(QString)));
    connect(ui->profileWidget,          SIGNAL(requestActor(QString)),          ui->actorTableView, SLOT(selectActor(QString)));
    connect(ui->profileWidget,          SIGNAL(deleteActor(QString)),           sql,                SLOT(dropActor(QString)));
    connect(ui->profileWidget,          SIGNAL(deleteActor(QString)),           ui->actorTableView, SLOT(removeActor(QString)));
    connect(ui->profileWidget,          SIGNAL(deleteActor(ActorPtr)),          this,               SLOT(removeActorItem(ActorPtr)));
    connect(this,                       SIGNAL(deleteActor(QString)),           sql,                SLOT(dropActor(QString)));
    connect(ui->profileWidget,          SIGNAL(renameFile(ScenePtr)),           this,               SLOT(renameFile(ScenePtr)));

    connect(ui->sceneTableView,         SIGNAL(playFile(int)),                  this,               SLOT(playVideo(int)));
    connect(sceneDetailView,            SIGNAL(playVideo(int)),                 this,               SLOT(playVideo(int)));
    connect(ui->actorTableView,         SIGNAL(actorClicked(QString)),          ui->profileWidget,  SLOT(loadActorProfile(QString)));
    connect(ui->sceneTableView,         SIGNAL(sceneSelectionChanged(ScenePtr)),sceneDetailView,    SLOT(sceneSelectionChanged(ScenePtr)));
    connect(ui->sceneTableView,         SIGNAL(loadSceneDetails(ScenePtr)),     sceneDetailView,    SLOT(loadScene(ScenePtr)));
    /// Connect Scene Detail View
    connect(sceneDetailView,            SIGNAL(saveChanges(ScenePtr)),          this,               SLOT(renameFile(ScenePtr)));
    connect(sceneDetailView,            SIGNAL(showActor(ActorPtr)),            ui->profileWidget,  SLOT(loadActorProfile(ActorPtr)));
    connect(sceneDetailView,            SIGNAL(showActor(QString)),             ui->actorTableView, SLOT(selectActor(QString)));
    connect(ui->tb_searchActors,        SIGNAL(clicked()),                      this ,              SLOT(searchActors()));
    connect(ui->tb_searchScenes,        SIGNAL(clicked()),                      this,               SLOT(searchScenes()));
    connect(ui->le_searchActors,        SIGNAL(returnPressed()),                this,               SLOT(searchActors()));
    connect(ui->le_searchScenes,        SIGNAL(returnPressed()),                this,               SLOT(searchScenes()));

    connect(ui->pb_saveActors,          &QPushButton::clicked,                  vault.data(),       &DataManager::saveAllActors);
    connect(ui->actionSave_Actors,      SIGNAL(triggered()),                    vault.data(),       SLOT(saveAllActors()));
    connect(ui->pb_saveScenes,          &QPushButton::clicked,                  vault.data(),       &DataManager::saveAllScenes);
    connect(ui->actionSave_Scenes,      SIGNAL(triggered()),                    vault.data(),       SLOT(saveAllScenes()));
    connect(ui->actionRefresh_Display,  SIGNAL(triggered()),                    vault.data(),       SLOT(updateActorDisplayItems()));
    connect(ui->actionUpdate_Bios,      SIGNAL(triggered()),                    vault.data(),       SLOT(updateBios()));

    connect(ui->cb_companyFilter,       SIGNAL(currentIndexChanged(QString)),   ui->sceneTableView, SLOT(companyFilterChanged(QString)));
    connect(ui->tb_clearSearchScenes,   SIGNAL(pressed()),                      ui->sceneTableView, SLOT(filenameFilterChanged()));
    connect(ui->tb_clearSearchScenes,   SIGNAL(pressed()),                      ui->le_searchScenes,SLOT(clear()));
    connect(ui->tb_clearSearchActors,   SIGNAL(pressed()),                      ui->le_searchActors,SLOT(clear()));
    connect(ui->tb_clearSearchActors,   SIGNAL(clicked()),                      ui->actorTableView, SLOT(filterChangedName()));
    connect(ui->cb_ethnicity,           SIGNAL(currentIndexChanged(QString)),   ui->actorTableView, SLOT(filterChangedEthnicity(QString)));
    connect(ui->cb_hairColor,           SIGNAL(currentIndexChanged(QString)),   ui->actorTableView, SLOT(filterChangedHair(QString)));
    //connect(this,                       SIGNAL(purgeScenes()),                  sql,                SLOT(purgeScenes()));
    //connect(this,                       SIGNAL(saveActorChanges(ActorPtr)),     sql,                SLOT(updateActor(ActorPtr)));
    connect(ui->actionLoad_Actors,      SIGNAL(triggered()),                    sql,                SLOT(loadActors()));
    connect(ui->pb_refreshActors,       SIGNAL(pressed()),                      sql,                SLOT(loadActors()));
    connect(ui->pb_refreshScenes,       SIGNAL(pressed()),                      sql,                SLOT(loadScenes()));
    connect(sql,                        SIGNAL(sendResult(ActorList)),          this,               SLOT(receiveActors(ActorList)));
    connect(sql,                        SIGNAL(sendResult(SceneList)),          this,               SLOT(receiveScenes(SceneList)));
    connect(ui->actionCleanDatabase,    SIGNAL(triggered()),                    sql,                SLOT(purgeScenes()));
    connect(sql,                        SIGNAL(sendPurgeList(QVector<int>)),    ui->sceneTableView, SLOT(purgeSceneItems(QVector<int>)));
    connect(ui->actorTableView,         SIGNAL(actorSelectionChanged(QString)), this,               SLOT(actorSelectionChanged(QString)));

}


void MainWindow::initDone(){
    splashScreen->close();
    delete splashScreen;
//    qDebug("Main Window Received %d actors, %d scenes, %d actor rows & %d scene rows", actorMap.size(), scenes.size(), actorRows.size(), sceneRows.size());
    //foreach(ActorPtr a, actors)                     { actorMap.insert(a->getName(), a); }
    //foreach(ScenePtr s, scenes)                     { sceneMap.insert(s->getID(),   s); }
    //vault->add(actors);
    //vault->add(scenes);
    //foreach(QList<QStandardItem *> row, actorRows)  { actorModel->appendRow(row);       }
    //foreach(QList<QStandardItem *>row, sceneRows)   { sceneModel->appendRow(row);       }
    ui->statusLabel->setText("Initialization Complete");
    /// Set up Filter boxes
    QStringList companies = sql->getCompanyList();
    companies.prepend(COMBO_BOX_DEFAULT);
    ui->cb_companyFilter->addItems(companies);
    QStringList hairColors = sql->getDistinctValueList("actors", "hair");
    hairColors.prepend(COMBO_BOX_DEFAULT);
    ui->cb_hairColor->addItems(hairColors);
    QStringList ethnicities = sql->getDistinctValueList("actors", "ethnicity");
    ethnicities.prepend(COMBO_BOX_DEFAULT);
    ui->cb_ethnicity->addItems(ethnicities);

    ui->actorTableView->resizeToContents();
    ui->sceneTableView->resizeSceneView();
    startThreads();
}

void MainWindow::startThreads(){
    qDebug("Initializing Worker Threads");
    this->curl = new curlTool();
    qDebug("Making connections between widgets, curlTool, and SQL Thread");
    /// PROGRESS & STATUS BAR UPDATING
    disconnect(sql,                     SIGNAL(startProgress(QString,int)),     this,               SLOT(newProgressDialog(QString,int)));
    disconnect(sql,                     SIGNAL(updateProgress(int)),            this,               SLOT(updateProgressDialog(int)));
    disconnect(sql,                     SIGNAL(closeProgress()),                this,               SLOT(closeProgressDialog()));
    connect(curl,                       SIGNAL(startProgress(QString,int)),     this,               SLOT(startProgress(QString,int)));
    connect(sql,                        SIGNAL(startProgress(QString,int)),     this,               SLOT(startProgress(QString,int)));
    connect(vault.data(),               SIGNAL(progressBegin(QString,int)),     this,               SLOT(startProgress(QString,int)));
    connect(ui->actorTableView,         SIGNAL(progressBegin(QString,int)),     this,               SLOT(startProgress(QString,int)));
    connect(ui->sceneTableView,         SIGNAL(progressBegin(QString,int)),     this,               SLOT(startProgress(QString,int)));
    connect(curl,                       SIGNAL(updateProgress(int)),            ui->progressBar,    SLOT(setValue(int)));
    connect(sql,                        SIGNAL(updateProgress(int)),            ui->progressBar,    SLOT(setValue(int)));
    connect(vault.data(),               SIGNAL(progressUpdate(int)),            ui->progressBar,    SLOT(setValue(int)));
    connect(ui->actorTableView,         SIGNAL(progressUpdate(int)),            ui->progressBar,    SLOT(setValue(int)));
    connect(ui->sceneTableView,         SIGNAL(progressUpdate(int)),            ui->progressBar,    SLOT(setValue(int)));
    connect(curl,                       SIGNAL(closeProgress(QString)),         this,               SLOT(closeProgress(QString)));
    connect(sql,                        SIGNAL(closeProgress(QString)),         this,               SLOT(closeProgress(QString)));
    connect(vault.data(),               SIGNAL(progressEnd(QString)),           this,               SLOT(closeProgress(QString)));
    connect(ui->actorTableView,         SIGNAL(progressEnd(QString)),           this,               SLOT(closeProgress(QString)));
    connect(ui->sceneTableView,         SIGNAL(progressEnd(QString)),           this,               SLOT(closeProgress(QString)));
    connect(sql,                        SIGNAL(updateStatus(QString)),          ui->statusLabel,    SLOT(setText(QString)));
    /// SHOW MESSAGE DIALOGS
    connect(sql,                        SIGNAL(showError(QString)),             this,               SLOT(showError(QString)));
    connect(curl,                       SIGNAL(showError(QString)),             this,               SLOT(showError(QString)));
    connect(sql,                        SIGNAL(showSuccess(QString)),           this,               SLOT(showSuccess(QString)));

    /// Set up curl thread communications with main thread
    connect(vault.data(),               SIGNAL(updateBiosFromWeb(ActorList)),   curl,               SLOT(updateBios(ActorList)));
    connect(this,                       SIGNAL(updateBios(ActorList)),          curl,               SLOT(updateBios(ActorList)));
    connect(curl,                       SIGNAL(updateSingleProfile(ActorPtr)),  ui->actorTableView, SLOT(addActor(ActorPtr)));
    connect(curl,                       SIGNAL(updateFinished(ActorList)),      this,               SLOT(receiveActors(ActorList)));
    /// Set up the SQL Thread for communications with the main thread

    /// Connect Actor Profile Widget with Database & Curl Thread
    ui->profileWidget->hide();
    connect(ui->profileWidget,          SIGNAL(saveToDatabase(ActorPtr)),       sql,                SLOT(updateActor(ActorPtr)));
    connect(ui->profileWidget,          SIGNAL(updateFromWeb(ActorPtr)),        curl,               SLOT(updateBio(ActorPtr)));
    connect(ui->profileWidget,          SIGNAL(downloadPhoto(ActorPtr)),        curl,               SLOT(downloadPhoto(ActorPtr)));
    connect(ui->profileWidget,          SIGNAL(deleteActor(ActorPtr)),          sql,                SLOT(drop(ActorPtr)));
    connect(ui->profileWidget,          SIGNAL(apv_to_ct_updateBio(QString)),   curl,               SLOT(apv_to_ct_getProfile(QString)));
    connect(curl,                       SIGNAL(ct_to_apv_sendActor(ActorPtr)),  ui->profileWidget,  SLOT(loadActorProfile(ActorPtr)));

    /** Scanning Routing Data Passing **/
    connect(sql,                        SIGNAL(db_to_ct_buildActors(QStringList)),      curl,               SLOT(db_to_ct_buildActors(QStringList)));
    connect(sql,                        SIGNAL(db_to_mw_sendActors(ActorList)),         ui->actorTableView, SLOT(addNewActors(ActorList)));//this,       SLOT(db_to_mw_receiveActors(ActorList)));
    connect(curl,                       SIGNAL(ct_to_db_storeActors(ActorList)),        sql,                SLOT(ct_to_db_storeActors(ActorList)));
    connect(sql,                        SIGNAL(db_to_mw_sendScenes(SceneList)),         ui->sceneTableView, SLOT(addNewScenes(SceneList)));//this,       SLOT(db_to_mw_receiveScenes(SceneList)));
    qDebug("Starting Curl Thread");
    /// Start the Threads
    this->curlThread = new QThread();
    curl->moveToThread(curlThread);
    qDebug("Curl Tool Moved to Thread");
    curlThread->start();
    qDebug("Curl Thread Started");
}

void MainWindow::sdv_to_mw_showActor(QString name){
    if (this->vault->contains(name)){
        ui->profileWidget->loadActorProfile(vault->getActor(name));
    }
}

void MainWindow::apv_to_mw_receiveSceneListRequest(QString actorName){
    if(!actorName.isEmpty() && actorName == currentActor->getName()){
        QVector<int> ids = ui->sceneTableView->getIDs();
        sceneUpdateList.clear();
        foreach(int id, ids){
            if (vault->contains(id)){
                sceneUpdateList << vault->getScene(id);
            }
        }
        if (!sceneUpdateList.isEmpty()){
            qDebug("Sending Actor Profile View %d scenes with '%s'", sceneUpdateList.size(), qPrintable(currentActor->getName()));
            ui->profileWidget->setActorsScenes(sceneUpdateList);
        } else {
            qWarning("No Scenes to Pass to from MainWindow to ActorProfileView");
        }
    } else {
        qWarning("Error: Not Returning any scenes to ActorProfileView, as an empty name was passed to MainWindow");
    }
}

//void MainWindow::sw_to_mw_selectionChanged(int id){
//    if (!this->sceneDetailView->isHidden()){
//        sw_to_mw_itemClicked(id);
//    }
//}

//void MainWindow::sw_to_mw_itemClicked(int id){
//    ScenePtr s = vault->getScene(id);
//    if (!s.isNull()){
//        sceneDetailView->loadScene(s);
//        qDebug("Showing Details of scene with id '%d'", id);
//    } else {
//        qWarning("Unable to locate scene with ID '%d' in Scene List", id);
//    }
//}


/** \brief Choose a Directory to scan files in from */
void MainWindow::on_actionScan_Directory_triggered(){
    this->fileDialog = new QFileDialog(this, "Choose Directory to Scan For Videos", "/Volumes");
    connect(fileDialog, SIGNAL(fileSelected(QString)), this, SLOT(scan_directory_chosen(QString)));
    connect(fileDialog, SIGNAL(accepted()), fileDialog, SLOT(deleteLater()));
    connect(fileDialog, SIGNAL(rejected()), fileDialog, SLOT(deleteLater()));
    this->fileDialog->show();
}

void MainWindow::on_actionScan_All_Folders_triggered(){
    Settings settings;
    QStringList folders = settings.getList(KEY_SEARCH_PATHS);
    if (folders.isEmpty()){
        SearchPathDialog dialog(this);
        dialog.exec();
    }
#warning make this a user-created list in the future
    folders = settings.getList(KEY_SEARCH_PATHS);
    scanPaths(folders);
}

void MainWindow::scanPaths(QStringList folders){
    if (!folders.isEmpty()){
        qDebug("Scanning All Folders");
        startScanner(folders);
        //startScanner(QStringList() << "/Volumes/16TB_MyBook/" << "/Volumes/4TB_Seagate/" << "/Volumes/8TB_White/");
    } else {
        qWarning("Error: No Folders are set up to be scanned!");
    }
}

void MainWindow::scan_directory_chosen(QString root_directory){
    qDebug("Scan Directory Selected: '%s'", qPrintable(root_directory));
    if (fileDialog){
        this->fileDialog->hide();
        this->fileDialog->deleteLater();
    }
    if (!root_directory.isEmpty()){
        QDir path(root_directory);
        if (path.exists()){
            startScanner(QStringList() << root_directory);
        }
    }
}

void MainWindow::startScanner(const QStringList &folders){
    this->scanner = new FileScanner(folders);
    connect(scanner,SIGNAL(fs_to_db_checkNames(QStringList)),   sql,            SLOT(fs_to_db_checkNames(QStringList)));
    connect(scanner,SIGNAL(fs_to_db_storeScenes(SceneList)),    sql,            SLOT(fs_to_db_storeScenes(SceneList)));
    connect(scanner,SIGNAL(fs_to_db_checkScenes(QFileInfoList)),sql,            SLOT(fs_to_db_checkScenes(QFileInfoList)), Qt::BlockingQueuedConnection);
    connect(sql,    SIGNAL(db_to_fs_sendUnsavedScenes(QFileInfoList)),scanner,  SLOT(db_to_fs_receiveUnsavedScenes(QFileInfoList)), Qt::DirectConnection);
    connect(scanner,SIGNAL(startProgress(QString,int)),         this,           SLOT(startProgress(QString,int)));
    connect(scanner,SIGNAL(updateProgress(int)),                ui->progressBar,SLOT(setValue(int)));
    connect(scanner,SIGNAL(closeProgress(QString)),             this,           SLOT(closeProgress(QString)));
    connect(scanner,SIGNAL(updateStatus(QString)),              ui->statusLabel,SLOT(setText(QString)));
    connect(scanner,SIGNAL(showError(QString)),                 this,           SLOT(showError(QString)));
    connect(scanner,SIGNAL(finished()),                         scanner,        SLOT(deleteLater()));
    connect(scanner,SIGNAL(finished()),                         sql,            SLOT(purgeScenes()));
    scanner->start();

}

void MainWindow::db_to_mw_receiveActors(ActorList list){
    qDebug("Main Window Received %d Actors from the SQL Thread. Adding them to the Display list", list.size());
    ui->actorTableView->addNewActors(list);
    foreach(ActorPtr a, list){
        actorModel->appendRow(a->buildQStandardItem());
        vault->add(a);
        actorModel->sort(ACTOR_NAME_COLUMN, Qt::AscendingOrder);
    }
    qDebug("Finished Adding %d actors!", list.size());
}

void MainWindow::db_to_mw_receiveScenes(SceneList list){
    qDebug("Adding %d Scenes to view...", list.size());
    foreach(ScenePtr s, list){
        //qDebug("Adding Scene with ID %d to List", s->getID());
        sceneModel->appendRow(s->buildQStandardItem());
        vault->add(s);
    }
    ui->actorTableView->resizeToContents();
    ui->sceneTableView->resizeSceneView();
    qDebug("Added %d Scenes!", list.size());
}

void MainWindow::actorSelectionChanged(QString name){
    if (vault->contains(name)){
        ActorPtr a = vault->getActor(name);
        if (!a.isNull()){
            qDebug("'%s' Selected", qPrintable(name));
            this->currentActor = a;
            if (!ui->profileWidget->isHidden()){
                ui->profileWidget->loadActorProfile(currentActor);
            }
            if (!this->sceneDetailView->isHidden()){
                this->sceneDetailView->clearDisplay();
                this->sceneDetailView->hide();
            }
        } else {
            qWarning("Actor Map doesn't Contain '%s'. Removing Item from Display", qPrintable(name));
            ui->actorTableView->removeActor(name);
        }
    }
}

/** \brief Find out which actor is currently selected, and return an ActorPtr object to it, or a null pointer if no actor is currently selected. */
ActorPtr MainWindow::getSelectedActor(){
    ActorPtr a = vault->getActor(ui->actorTableView->selectedName());
    if (!a.isNull()){
        this->currentActor = a;
    } else {
        qWarning("Error Retrieving Selected Actor");
    }
    return a;
}


void MainWindow::searchScenes(){
    QString searchTerm = ui->le_searchScenes->text();
    if (!searchTerm.isEmpty()){
        qDebug("Searching Scenes for '%s'", qPrintable(searchTerm));
        ui->sceneTableView->searchByFilename(searchTerm);
        this->prevSearchScene = searchTerm;
    }
}

void MainWindow::searchActors(){
    if (ui->le_searchActors){
        ui->actorTableView->filterChangedName(ui->le_searchActors->text());
//        ui->actorTableView->selectActor(ui->le_searchActors->text());
    }
}

void MainWindow::on_actionDeleteActor_triggered(){
    qDebug("Delete Actor Shortcut Detected");
    if (!currentActor.isNull()){
        removeActorItem(currentActor);
        emit deleteActor(currentActor->getName());
    }
}
QModelIndex MainWindow::getCurrentIndex(QAbstractItemModel */*model*/){
    QModelIndex x = QModelIndex();
    x = ui->actorTableView->currentIndex();
    return x;
}
void MainWindow::showCurrentActorProfile(){
    if (!currentActor.isNull()){
        ui->profileWidget->loadActorProfile(currentActor);
    }
}
QString MainWindow::getCurrentName(QAbstractItemModel *model){
    QString name("");
    QModelIndex x = ui->actorTableView->currentIndex();
    if(x.isValid()){
        QModelIndex nameIndex = model->index(x.row(), ACTOR_NAME_COLUMN);
        if (nameIndex.isValid()){
            name = nameIndex.data().toString();
        }
    }
    return name;
}
void MainWindow::removeActorItem(ActorPtr actor){
    if (!actor.isNull()){
        ui->actorTableView->removeActor(actor->getName());
        vault->remove(actor);
    }
}

/** \brief Show an error Dialog with the provided Text. */
void MainWindow::showError(QString message){
    //qWarning("Showing Error Dialog with text: '%s'", qPrintable(message));
    QMessageBox box(QMessageBox::Warning, tr("Error"), message, QMessageBox::Ok, this, Qt::WindowStaysOnTopHint);
    box.exec();
}
/** \brief Show a success dialog with the provided text */
void MainWindow::showSuccess(QString message){
    //qDebug("Message Received: '%s'", qPrintable(message));
    QMessageBox box(QMessageBox::Information, tr("Success"), message, QMessageBox::Ok, this, Qt::WindowStaysOnTopHint);
    box.exec();
}

/** \brief Initialize the progress bar to 0, set its new maximum value, and update the text on the status bar
 *  \param QString status:  Text to show on the status bar
 *  \param int max:         Upper boundary of progress bar.
 */
void MainWindow::startProgress(QString status, int max){
    ui->statusLabel->setText(status);
    ui->progressBar->setRange(0, max);
    ui->progressBar->setValue(0);
}

/** \brief Set the progress bar to 100%, and update the text on the status bar.
 *  \param QString status:  Text to show on the status bar.
 */
void MainWindow::closeProgress(QString status){
    ui->statusLabel->setText(status);
    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(100);
}

/** \brief  Open a New Progress Dialog Box.
 *  \param  QString label:  Text to show on dialog
 *  \param  int max:        value to use as 100%.
 */
void MainWindow::newProgressDialog(QString label, int max){
    //qDebug("***********%s**************", qPrintable(label));
    this->progressDialog = new QProgressDialog(label, QString(), 0, max, this, Qt::WindowStaysOnTopHint);
    this->progressDialog->setMinimumWidth(400);
    this->progressDialog->setCancelButton(nullptr);
    this->progressDialog->setWindowModality(Qt::WindowModal);
    this->progressDialog->setValue(0);
    this->progressDialog->show();
}
/** \brief  Update the progress of a Pop-Up Progress Dialog */
void MainWindow::updateProgressDialog(int value){
    if (progressDialog && (value <= progressDialog->maximum())){
        this->progressDialog->setValue(value);
    }
}
/** \brief Update the text label on the progress dialog box */
void MainWindow::updateProgressDialog(QString s){
    if (progressDialog && !s.isNull() && !s.isEmpty()){
        this->progressDialog->setLabelText(s);
    }
}

/** \brief  Close & Delete the pop-up progress dialog window */
void MainWindow::closeProgressDialog(){
    if (progressDialog){
        this->progressDialog->setRange(0, 100);
        this->progressDialog->setValue(100);
        this->progressDialog->hide();
        progressDialog->deleteLater();
    }
}

/** \brief Slot to receive list of scenes. */
void MainWindow::receiveScenes(SceneList list){
    foreach(ScenePtr s, list){
        if (!vault->contains(s->getID())){
            sceneModel->appendRow(s->buildQStandardItem());
            vault->add(s);
        }
    }
    QStringList companies = sql->getCompanyList();
    foreach(QString company, companies){
        ui->cb_companyFilter->addItem(company);
    }
    ui->sceneTableView->resizeSceneView();
}

void MainWindow::resetActorFilterSelectors(){
    ui->cb_hairColor->clear();
    ui->cb_ethnicity->clear();
    QStringList hairColors = sql->getDistinctValueList("actors", "hair");
    hairColors.prepend(COMBO_BOX_DEFAULT);
    ui->cb_hairColor->addItems(hairColors);
    QStringList ethnicities = sql->getDistinctValueList("actors", "ethnicity");
    ethnicities.prepend(COMBO_BOX_DEFAULT);
    ui->cb_ethnicity->addItems(ethnicities);
}

void MainWindow::receiveActors(ActorList list){
    if (!list.isEmpty()){
        bool reloadCurrentProfile = (!ui->profileWidget->isHidden() && list.contains(currentActor));
        ui->actorTableView->addNewActors(list);
        if (reloadCurrentProfile){
            ui->profileWidget->reloadProfile();
        }
    }
    ui->actorTableView->resizeToContents();
    qDebug("Actor List updated with %d items", list.size());
}



/** \brief Receive an actor that has had its profile updated via the curl thread.
 *  Update the actor's stored data with the new data if the new bio has more information than the old one.
 *  If the Actor that was updated is the currently selected actor, update the profile view (if open), and
 *  save the new biographical details to the database.
 */
void MainWindow::receiveSingleActor(ActorPtr a){
    ui->actorTableView->addActor(a);
    if (!ui->profileWidget->isHidden() && (a == currentActor)){
        ui->profileWidget->reloadProfile();
    }
//    QString name = a->getName();
//    if (name == currentActor->getName() && !ui->profileWidget->isHidden()){
//        qDebug("Updating Profile View");
//        ui->profileWidget->loadActorProfile(updatedActor);
//        emit saveActorChanges(currentActor);
//    } else {
//        qDebug("Saving Updates to database");
//        this->updatedActor = a;
//        emit saveActorChanges(updatedActor);
//    }
}

/** \brief  Assign a new profile picture for the currently selected actor
 */
void MainWindow::selectNewProfilePhoto(){
    if (!this->currentActor.isNull()){
        QString hint = QString("Choose a Profile Photo for %1").arg(this->currentActor->getName());
        QString source_filename = QFileDialog::getOpenFileName(this, hint, "/Volumes");
        if (!source_filename.isEmpty()){
            QString new_filename = getHeadshotName(this->currentActor->getName());
            QFile photo(source_filename);
            if (photo.copy(source_filename, new_filename)){
                qDebug("Successfully copied %s to %s", qPrintable(source_filename), qPrintable(new_filename));
                this->currentActor->setHeadshot(new_filename);
                this->currentActor->updateQStandardItem();
                ui->profileWidget->loadActorProfile(currentActor);
            } else {
                qWarning("Error Copying %s to %s", qPrintable(source_filename), qPrintable(new_filename));
            }
        }
    }
}


/** \brief Test Function to verify Lexical Parsing of Scene Titles.
 *  Parses a single video file's title, and opens a dialog window showing the data extracted. */
void MainWindow::on_actionParse_Scene_triggered(){
    QString filename = QFileDialog::getOpenFileName(this, tr("Select Video to Parse"), "/Volumes");
    if (!filename.isEmpty()){
        ui->statusLabel->setText(QString("Parsing %1").arg(filename));
        SceneParser input;
        input.parse(filename);

        Scene *scene = new Scene(input);
        //SceneRenamer output(temp.data());
        SceneRenamer output(scene);
        QString newName = output.getNewFilename();
        QString message = output.displayInfo();
        qDebug("Old Name:   %s", qPrintable(filename));
        qDebug("New Name:   %s", qPrintable(newName));
        QMessageBox box(QMessageBox::NoIcon, tr("Scene Parser Test"), message, QMessageBox::Close, this, Qt::WindowStaysOnTopHint);
        box.resize(QSize(1000, 400));
        box.exec();
        ui->statusLabel->setText("");
        delete scene;
    }
}



/** Open a Dialog that allows the user to 'manually' add an actor by entering a name and looking up the corrosponding profile. */
void MainWindow::on_actionAdd_Actor_triggered(){
    QString newName = QInputDialog::getText(this, tr("Enter Actor Name"), tr("Enter an actor name to build a profile for"));

    if (!newName.isEmpty()){
        this->addProfileDialog = new ProfileDialog(newName, this);
        connect(addProfileDialog, SIGNAL(pd_to_ct_getProfile(QString)),     curl, SLOT(pd_to_ct_getActor(QString)));
        connect(addProfileDialog, SIGNAL(pd_to_db_saveProfile(ActorPtr)),   sql,  SLOT(pd_to_db_saveActor(ActorPtr)));
        connect(curl,       SIGNAL(ct_to_pd_sendActor(ActorPtr)),     addProfileDialog, SLOT(ct_to_pd_receiveProfile(ActorPtr)));
        connect(sql,        SIGNAL(db_to_pd_sendBackWithID(ActorPtr)),addProfileDialog, SLOT(db_to_pd_receiveProfileWithID(ActorPtr)));
        connect(addProfileDialog, SIGNAL(pd_to_mw_addDisplayItem(ActorPtr)), this, SLOT(pd_to_mw_addActorToDisplay(ActorPtr)));
        connect(addProfileDialog, SIGNAL(closed()),                   addProfileDialog, SLOT(deleteLater()));
        addProfileDialog->show();
    }
}

void MainWindow::pd_to_mw_addActorToDisplay(ActorPtr a){
    if (!a.isNull()){
        if (!vault->contains(a->getName())){
            qDebug("Adding %s to List", qPrintable(a->getName()));
            this->actorModel->appendRow(a->buildQStandardItem());
            vault->add(a);
        }
    }
}

/** \brief Test Function to attempt to build an entirely new biography via web scraping based on a name alone.
 *  When web scraping is complete, a pop-up dialog is shown with the biographical details retrieved (and some not retrieved) */
void MainWindow::on_actionCreate_Bio_triggered(){
    QString name = QInputDialog::getText(this, tr("Test Bio Retrieval"), tr("Enter Actor Name"));
    if (!name.isEmpty()){
        this->curlTestObject = new curlTool();
        this->curlTestThread = new QThread();
        connect(curlTestObject, SIGNAL(updateSingleProfile(ActorPtr)),  this,           SLOT(receiveTestBio(ActorPtr)));
        connect(this,           SIGNAL(updateSingleBio(ActorPtr)),      curlTestObject, SLOT(updateBio(ActorPtr)));
        curlTestObject->moveToThread(curlTestThread);
        curlTestThread->start();
        ActorPtr a = ActorPtr(new Actor(name));
        emit updateSingleBio(a);
    }
}
/** \brief Test Function, paired with on_actionCreate_Bio_triggered():
 *  Receives updated actor object from curl and opens a dialog showing the details. */
void MainWindow::receiveTestBio(ActorPtr a){
    qDebug("Receieved actor %s from Curl Test Thread", qPrintable(a->getName()));
    this->testProfileDialog = new ProfileDialog(a, this);
    connect(testProfileDialog, SIGNAL(closed()), testProfileDialog, SLOT(deleteLater()));
    connect(testProfileDialog, SIGNAL(closed()), curlTestObject, SLOT(deleteLater()));
    connect(testProfileDialog, SIGNAL(closed()), curlTestThread, SLOT(terminate()));
    connect(testProfileDialog, SIGNAL(closed()), curlTestThread, SLOT(deleteLater()));
    this->testProfileDialog->show();
}


void MainWindow::renameFile(ScenePtr scene){
    qDebug("In 'renameFile'");
    if (!scene.isNull()){
        SceneRenamer renamer(scene.data());
        QString newName = renamer.getNewFilename();
        QString text = QString("'%1'\n\nwould become:\n\n%2").arg(scene->getFilename()).arg(newName);
        qDebug("'%s'\nwould become:\n'%s'\n", qPrintable(scene->getFilename()), qPrintable(newName));
        QMessageBox box(QMessageBox::Question, tr("File Renamer"), text, QMessageBox::Save | QMessageBox::Cancel, this, Qt::WindowStaysOnTopHint);
        box.resize(QSize(600,400));
        if (box.exec() == QMessageBox::Save){
#ifdef RENAMER_THREAD
            this->updater = new FileRenamer(scene, newName, this);
            connect(updater, SIGNAL(error(QString)),            this,       SLOT(showError(QString)));
            connect(updater, SIGNAL(saveToDatabase(ScenePtr)),  sql,        SLOT(saveChanges(ScenePtr)));
            connect(updater, SIGNAL(done(ScenePtr)),            this,       SLOT(updateSceneDisplay(ScenePtr)));
            connect(updater, SIGNAL(finished()),                updater,    SLOT(deleteLater()));
            updater->start();
#else
            QPair<QString,QString> fileInfo = scene->getFile();
            QString fullpath = scene->getFullpath();
            QString newPath = QString("%1/%2").arg(scene->getFolder()).arg(newName);
            QString oldName = scene->getFilename();
            QPair<QString,QString> newFileInfo;
            newFileInfo.first = fileInfo.first;
            newFileInfo.second = newName;
            scene->setFile(newFileInfo);
            QFile file(fullpath);
            bool saved = false;
            if (fullpath == newPath){
                saved = true;
            } else {
                saved = file.rename(fullpath, newPath);
            }
            if (!saved){
                qWarning("Unable to rename:\n'%s' ------>\n'%s'\n", qPrintable(fullpath), qPrintable(newPath));
                QMessageBox::warning(this, tr("Renaming Error"), QString("Error Renaming\n%1\n---->\n%2").arg(oldName).arg(newName), QMessageBox::Cancel);
            } else {
                qDebug("File Renamed");
                emit saveChangesToDB(scene);
                qDebug("Database Updated");
                scene->updateQStandardItem();
                qDebug("Display Updated");
            }
#endif
        }
    }
}

void MainWindow::updateSceneDisplay(ScenePtr s){
    s->updateQStandardItem();
}

void MainWindow::playVideo(int sceneID){
    ScenePtr curr = vault->getScene(sceneID);
    if (!curr.isNull()){
        QString filepath = curr->getFullpath();
        if (!videoOpen){
            if (QFileInfo(filepath).exists()){
                this->videoPlayer = new VideoPlayer(filepath);
                this->videoThread = new QThread(this);
                videoPlayer->moveToThread(videoThread);
                connect(this,        SIGNAL(startVideoPlayback()),  videoPlayer,    SLOT(play()));
                connect(videoPlayer, SIGNAL(error(QString)),        this,           SLOT(showError(QString)));
                connect(videoPlayer, SIGNAL(videoStopped()),        this,           SLOT(videoFinished()));
                this->videoOpen = true;
                videoThread->start();
                emit startVideoPlayback();
            }
        } else {
            qWarning("Video Player Already Open!");
        }
    }
}

void MainWindow::videoFinished(){
    if (videoPlayer){
        videoPlayer->deleteLater();
    }
    if (videoThread){
        videoThread->terminate();
        videoThread->deleteLater();
    }
    this->videoOpen = false;
}

void MainWindow::on_tb_clearActorFilters_clicked(){
    ui->cb_ethnicity->setCurrentIndex(-1);
    ui->cb_hairColor->setCurrentIndex(-1);
    ui->cb_sceneCount->setCurrentText("More Than");
    ui->sb_sceneCount->clear();
    actorProxyModel->clearFilters();
}

void MainWindow::on_actionWipe_Scenes_Table_triggered(){
    sql->dropTable(Database::SCENE);
    sql->makeTable(Database::SCENE);
}

void MainWindow::on_actionWipe_Actor_Table_triggered(){
    sql->dropTable(Database::ACTOR);
    sql->makeTable(Database::ACTOR);
}


void MainWindow::on_actionItemDetails_triggered(){
   qDebug("Show Profile Shortcut Triggered");
   ui->profileWidget->loadActorProfile(currentActor);
}


void MainWindow::on_actionAdd_Scan_Folder_triggered(){
    SearchPathDialog dialog(this);
    dialog.exec();
}
