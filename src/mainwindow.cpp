#include "mainwindow.h"
#include "filenames.h"
#include "ui_mainwindow.h"
#include "SceneParser.h"
#include "SceneRenamer.h"
#include "FileScanner.h"
#include "Scene.h"
#include "Actor.h"
#include "ActorProfileView.h"
#include "InitializationThread.h"
#include <QtConcurrent>
#include <QInputDialog>
#include <QtGlobal>
#include <QDebug>
#include <QMessageBox>
#include <QVector>
#include <QFileDialog>
#include <QRegExp>
#include <QSplitter>
#define MINIMUM_BIO_SIZE 11
#define COMBO_BOX_DEFAULT "No Selection"
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);
//    this->runMode = Debug;
    this->runMode = Release;//Debug;
    this->setWindowIcon(QIcon(QPixmap("SceneQuery.icns")));
    actorList = {};
    sceneList = {};
    updateList = {};
    this->currentDisplay = DISPLAY_ACTORS;
    qRegisterMetaType<int>("int");
    qRegisterMetaType<QString>("QString");
    qRegisterMetaType<QStringList>("QStringList");
    qRegisterMetaType<QHash<int, ScenePtr>>("QHash<int, ScenePtr>");
    qRegisterMetaType<QSharedPointer<Scene>>("QSharedPointer<Scene>");
    qRegisterMetaType<QSharedPointer<Actor>>("QSharedPointer<Actor>");
    qRegisterMetaType<QVector<QSharedPointer<Scene>>>("QVector<QSharedPointer<Scene>>");
    qRegisterMetaType<QVector<QSharedPointer<Actor>>>("QVector<QSharedPointer<Actor>>");
    qRegisterMetaType<ScenePtr>("ScenePtr");
    qRegisterMetaType<ActorPtr>("ActorPtr");
    qRegisterMetaType<SceneList>("SceneList");
    qRegisterMetaType<ActorList>("ActorList");
    this->videoOpen = false;
    this->sceneMap = QHash<int,ScenePtr>();
    setupViews();
}

MainWindow::~MainWindow(){
    delete ui;
    delete sql;
    delete curl;
    curlThread->terminate();
    curlThread->wait();
    delete curlThread;
    sqlThread->terminate();
    sqlThread->wait();
    delete sqlThread;
    delete sceneDetailView;
    delete actorSelectionModel;
    /*
    delete actorProxyModel;
    delete sceneProxyModel;
    delete actorModel;
    delete sceneModel;
    */
    actorMap.clear();
    actorList.clear();
    sceneList.clear();
}

/** \brief Set up the main display */
void MainWindow::setupViews(){
    ui->tabWidget_filters->hide();
    /// Set up Actor Table View
    this->actorHeaders << "" << "Name" << "Hair Color" << "Ethnicity" << "Scenes" << "Bio Size";
    this->actorModel = new QStandardItemModel();
    actorModel->setHorizontalHeaderLabels(actorHeaders);
    this->actorModel->setSortRole(Qt::DecorationRole);

    this->actorParent = actorModel->invisibleRootItem();
    this->actorProxyModel = new ActorProxyModel(this);
//    this->actorProxyModel = new QSortFilterProxyModel(this);
    this->actorProxyModel->setSourceModel(actorModel);
    ui->actorTableView->setModel(actorProxyModel);
    ui->actorTableView->setSortingEnabled(true);
    ui->actorTableView->verticalHeader()->hide();
    ui->actorTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->actorTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    /// Set up the Scene Table View
    this->sceneHeaders << "Main Actor" << "Title" << "Company" << "Resolution" << "Featured Actors" << "File Size" << "Length" << "Released" << "Rating" << "Location" << "ID";
    this->sceneModel = new QStandardItemModel();
    this->sceneProxyModel = new SceneProxyModel(this);
    this->sceneParent = sceneModel->invisibleRootItem();
    sceneModel->setHorizontalHeaderLabels(sceneHeaders);
    sceneProxyModel->setSourceModel(sceneModel);
    ui->sceneWidget->setSourceModel(sceneModel);

    /// Set Up Scene Detail View
    this->sceneDetailView = new SceneDetailView(this);
    ui->vbl_list_layout->addWidget(sceneDetailView);
    this->sceneDetailView->hide();

    /// Set Up Profile Widget
    ui->profileWidget->hide();

    ui->progressBar->setValue(0);
    QStringList numFilters;
    numFilters << "More Than" << "Less Than" << "Exactly";
    ui->cb_sceneCount->addItems(numFilters);
    /// Create Thread Objects
    this->sql  = new SQL();
    this->sqlThread = new QThread();
    sql->moveToThread(sqlThread);
    /// Make Relevant connections between widgets
    connect(ui->profileWidget,  SIGNAL(hidden()),                       ui->sceneWidget,    SLOT(clearFilter()));
    connect(ui->sceneWidget,    SIGNAL(sendSceneCount(int)),            ui->profileWidget,  SLOT(acceptSceneCount(int)));
    connect(ui->profileWidget,  SIGNAL(requestSceneCount()),            ui->sceneWidget,    SLOT(receiveSceneCountRequest()));
    connect(ui->profileWidget,  SIGNAL(chooseNewPhoto()),               this,               SLOT(selectNewProfilePhoto()));
    connect(this,               SIGNAL(loadActorProfile(ActorPtr)),     ui->profileWidget,  SLOT(loadActorProfile(ActorPtr)));
    connect(this,               SIGNAL(loadActorProfile(ActorPtr)),     ui->sceneWidget,    SLOT(actorFilterChanged(ActorPtr)));
    connect(this,               SIGNAL(cb_companyFilterChanged(QString)),ui->sceneWidget,   SLOT(companyFilterChanged(QString)));

    connect(ui->profileWidget,  SIGNAL(chooseNewPhoto()),               this,               SLOT(selectNewProfilePhoto()));
    connect(ui->profileWidget,  SIGNAL(apv_to_mw_requestScenes(QString)),this,              SLOT(apv_to_mw_receiveSceneListRequest(QString)));
    connect(ui->profileWidget,  SIGNAL(apv_to_mw_requestActor(QString)),this,               SLOT(apv_to_mw_receiveActorRequest(QString)));
    connect(this,               SIGNAL(mw_to_apv_sendActor(ActorPtr)),  ui->profileWidget,  SLOT(mw_to_apv_receiveActor(ActorPtr)));
    connect(this,               SIGNAL(mw_to_apv_sendScenes(SceneList)),ui->profileWidget,  SLOT(mw_to_apv_receiveScenes(SceneList)));
    connect(ui->profileWidget,  SIGNAL(apv_to_mw_deleteActor(QString)), this,               SLOT(apv_to_mw_deleteActor(QString)));
    connect(ui->profileWidget,  SIGNAL(renameFile(ScenePtr)),           this,               SLOT(renameFile(ScenePtr)));

    connect(ui->sceneWidget,    SIGNAL(playFile(int)),                  this,               SLOT(playVideo(int)));
    connect(sceneDetailView,    SIGNAL(playVideo(int)),                 this,               SLOT(playVideo(int)));
    connect(ui->actorTableView, SIGNAL(clicked(QModelIndex)),           this,               SLOT(actorTableView_clicked(QModelIndex)));
    connect(this,               SIGNAL(resizeSceneView()),              ui->sceneWidget,    SLOT(resizeSceneView()));
    connect(ui->sceneWidget,    SIGNAL(sceneSelectionChanged(int)),     this,               SLOT(sw_to_mw_selectionChanged(int)));
    connect(ui->sceneWidget,    SIGNAL(sceneItemClicked(int)),          this,               SLOT(sw_to_mw_itemClicked(int)));
    /// Connect Scene Detail View
    connect(sceneDetailView,    SIGNAL(saveChanges(ScenePtr)),          this,               SLOT(renameFile(ScenePtr)));
    connect(sceneDetailView,    SIGNAL(showActor(QString)),             this,               SLOT(sdv_to_mw_showActor(QString)));
    connect(sceneDetailView,    SIGNAL(requestActorBirthday(QString)),  this,               SLOT(sdv_to_mw_requestBirthday(QString)));
    connect(this,               SIGNAL(sendActorBirthday(QString,QDate)),sceneDetailView,   SLOT(receiveActorBirthday(QString,QDate)));
    connect(this,               SIGNAL(showSceneDetails(ScenePtr)),      sceneDetailView,   SLOT(loadScene(ScenePtr)));
  //  connect(this,               SIGNAL(hideSceneDetails()),             sceneDetailView,SLOT(hide()));
    //connect(sql,              SIGNAL(initializationFinished(ActorList,SceneList)), this,  SLOT(initializationFinished(ActorList,SceneList)));
    connect(sql,                SIGNAL(startProgress(QString,int)),     this,               SLOT(newProgressDialog(QString, int)));
    connect(sql,                SIGNAL(updateProgress(int)),            this,               SLOT(updateProgressDialog(int)));
    connect(sql,                SIGNAL(closeProgress()),                this,               SLOT(closeProgressDialog()));
    sqlThread->start();
}

void MainWindow::sdv_to_mw_showActor(QString name){
    if (this->actorMap.contains(name)){
        emit loadActorProfile(actorMap.value(name));
    }
}
void MainWindow::sdv_to_mw_requestBirthday(QString name){
    if (this->actorMap.contains(name)){
        QDate birthday = actorMap.value(name)->getBirthday();
        if (!birthday.isNull() && birthday.isValid()){
            emit sendActorBirthday(name, birthday);
        }
    }
}

void MainWindow::apv_to_mw_deleteActor(QString name){
    qDebug("Main Window Removing Display Item for '%s'", qPrintable(name));
    if (name == currentActor->getName()){
        actorModel->removeRow(currentActorIndex.row());
    } else {
        qWarning("Error: Can't remove display item, since the item to be deleted isn't the current item, and it is not currently understood how to locate its index.");
    }
}

void MainWindow::apv_to_mw_receiveSceneListRequest(QString actorName){
    if(!actorName.isEmpty()){
        this->sceneUpdateList = scenesWithActor(actorName, sceneMap);
        qDebug("Sending Actor Profile View %d scenes with '%s'", sceneUpdateList.size(), qPrintable(actorName));
        emit mw_to_apv_sendScenes(sceneUpdateList);
    } else {
        qWarning("Error: Not Returning any scenes to ActorProfileView, as an empty name was passed to MainWindow");
    }
}
void MainWindow::apv_to_mw_receiveActorRequest(QString name){
    if (actorMap.contains(name)){
        this->currentActor = actorMap.value(name);
        emit mw_to_apv_sendActor(currentActor);
    }
}

void MainWindow::sw_to_mw_itemClicked(int id){
    if (sceneMap.contains(id)){
        ScenePtr s = sceneMap.value(id);
        if (!s.isNull()){
            qDebug("Showing Details of scene with id '%d'", id);
            emit showSceneDetails(s);
        } else {
            qWarning("Unable to locate scene with ID '%d' in Scene List", id);
        }
    } else {
        qWarning("Scene with ID %d not in map", id);
    }
}

void MainWindow::sw_to_mw_selectionChanged(int id){
    if (!this->sceneDetailView->isHidden()){
        sw_to_mw_itemClicked(id);
    }
}

void MainWindow::showEvent(QShowEvent */*event*/){
    if (this->runMode == Release){
        this->initThread = new InitializationThread();
        connect(initThread, SIGNAL(startProgress(QString,int)),             this,   SLOT(newProgressDialog(QString,int)));
        connect(initThread, SIGNAL(updateProgress(int)),                    this,   SLOT(updateProgressDialog(int)));
        connect(initThread, SIGNAL(closeProgressDialog()),                  this,   SLOT(closeProgressDialog()));
        connect(initThread, SIGNAL(sendInitialLists(ActorList,SceneList)),  this,   SLOT(initializationFinished(ActorList,SceneList)));
        connect(this,       SIGNAL(newProgressDialogBox(QString,int)),      this,   SLOT(newProgressDialog(QString,int)));
        connect(this,       SIGNAL(updateProgressDialogBox(int)),           this,   SLOT(updateProgressDialog(int)));
        connect(this,       SIGNAL(closeProgressDialogBox()),               this,   SLOT(closeProgressDialog()));
        connect(sql,        SIGNAL(sendResult(ActorList)),           initThread,    SLOT(receiveActors(ActorList)));
        connect(sql,        SIGNAL(sendResult(SceneList)),           initThread,    SLOT(receiveScenes(SceneList)));
        connect(initThread, SIGNAL(getActors()),                            sql,    SLOT(loadActors()));
        connect(initThread, SIGNAL(getScenes()),                            sql,    SLOT(loadScenes()));


        qDebug("Starting Initialization Thread");
        this->initThread->start();
    } else {
        qDebug("Skipping initial load from Database");
        initializationFinished({}, {});
    }
}

void MainWindow::buildQStandardItem(ActorPtr a){
    mx.lock();
    a->setSceneCount(countWithActor(a->getName(), sceneMap));
    mx.unlock();
    QList<QStandardItem *> row = a->buildQStandardItem();
    mx.lock();
    rows.push_back(row);
    actorMap.insert(a->getName(), a);
    ++index;
    if (index % 100 == 0){
        emit updateProgressDialog(index);
    }
    mx.unlock();
}

void MainWindow::initializationFinished(ActorList actors, SceneList scenes){
    if (runMode == Release){
        disconnect(initThread, SIGNAL(sendInitialLists(ActorList,SceneList)),   this,       SLOT(initializationFinished(ActorList,SceneList)));
        disconnect(initThread, SIGNAL(startProgress(QString,int)),              this,       SLOT(newProgressDialog(QString,int)));
        disconnect(initThread, SIGNAL(updateProgress(int)),                     this,       SLOT(updateProgressDialog(int)));
        disconnect(initThread, SIGNAL(closeProgressDialog()),                   this,       SLOT(closeProgressDialog()));
        disconnect(sql,        SIGNAL(sendResult(ActorList)),                   initThread, SLOT(receiveActors(ActorList)));
        disconnect(sql,        SIGNAL(sendResult(SceneList)),                   initThread, SLOT(receiveScenes(SceneList)));
        disconnect(initThread, SIGNAL(getActors()),                             sql,        SLOT(loadActors()));
        disconnect(initThread, SIGNAL(getScenes()),                             sql,        SLOT(loadScenes()));
        disconnect(sql,        SIGNAL(startProgress(QString,int)),              this,       SLOT(newProgressDialog(QString,int)));
        disconnect(sql,        SIGNAL(updateProgress(int)),                     this,       SLOT(updateProgressDialog(int)));
        disconnect(sql,        SIGNAL(closeProgress()),                         this,       SLOT(closeProgressDialog()));
        disconnect(sql,        SIGNAL(updateStatus(QString)),                   this,       SLOT(updateProgressDialog(QString)));
        if (initThread){
            initThread->deleteLater();
        }
        qDebug("\n\tMain Window Received %d Actors & %d Scenes from the init thread", actors.size(), scenes.size());
        int idx = 0;
        emit newProgressDialogBox(QString("Adding %1 Scenes...").arg(scenes.size()), scenes.size());
        foreach(ScenePtr s, scenes){
            QList<QStandardItem *>row = s->buildQStandardItem();
            sceneModel->appendRow(row);
            sceneMap.insert(s->getID(), s);
            idx++;
            if (idx % 100 == 0){
                emit updateProgressDialogBox(idx);
            }
        }
        emit closeProgressDialogBox();
        /*
        QTime timer(0,0,0);
        timer.start();
        index = 0;
        emit newProgressDialogBox(QString("Building %1 Actor Items").arg(actors.size()), actors.size());
        QFutureSynchronizer<void> sync;
        for(int i = 0; i < actors.size(); ++i){
            ActorPtr a = actors.at(i);
            sync.addFuture(QtConcurrent::run(this, &MainWindow::buildQStandardItem, a));
        }
        sync.waitForFinished();
        emit closeProgressDialogBox();
        index = 0;
        emit newProgressDialogBox(QString("Adding %1 Actors to the Display").arg(actors.size()), actors.size());
        foreach(QList<QStandardItem *> row, rows){
            actorModel->appendRow(row);
            if (++index %100 == 0){
                emit updateProgressDialogBox(index);
            }
        }
        double elapsed = (double)timer.elapsed()/1000.0;
        qDebug("\n****Time To Build and Add Actors: %s ****\n", qPrintable(QString::number(elapsed, 'f', 2)));

        emit closeProgressDialogBox();
        qDebug("All items added to GUI");
        */
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
    }
    // Set up selection behaviour
    qDebug("Setting up Actor Table Selection Model");
    this->actorSelectionModel = ui->actorTableView->selectionModel();

    connect(actorSelectionModel, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(actorSelectionChanged(QModelIndex,QModelIndex)));

    ui->statusLabel->setText(QString("%1 Actors & %2 Scenes Loaded!").arg(actors.size()).arg(scenes.size()));
    ui->actorTableView->resizeColumnsToContents();
    qDebug("Initializing Worker Threads");

    this->curl = new curlTool();
    qDebug("Making connections between widgets, curlTool, and SQL Thread");
    /// PROGRESS & STATUS BAR UPDATING
    connect(curl,   SIGNAL(startProgress(QString,int)),     this,   SLOT(startProgress(QString,int)));
    connect(sql,    SIGNAL(startProgress(QString,int)),     this,   SLOT(startProgress(QString,int)));
    connect(curl,   SIGNAL(updateProgress(int)),            ui->progressBar, SLOT(setValue(int)));
    connect(sql,    SIGNAL(updateProgress(int)),            ui->progressBar, SLOT(setValue(int)));
    connect(curl,   SIGNAL(closeProgress(QString)),         this,   SLOT(closeProgress(QString)));
    connect(sql,    SIGNAL(closeProgress(QString)),         this,   SLOT(closeProgress(QString)));
    connect(sql,    SIGNAL(updateStatus(QString)),          ui->statusLabel, SLOT(setText(QString)));
    /// SHOW MESSAGE DIALOGS
    connect(sql,    SIGNAL(showError(QString)),             this,   SLOT(showError(QString)));
    connect(curl,   SIGNAL(showError(QString)),             this,   SLOT(showError(QString)));
    connect(sql,    SIGNAL(showSuccess(QString)),           this,   SLOT(showSuccess(QString)));

    /// Set up curl thread communications with main thread
    connect(this,   SIGNAL(updateBios(ActorList)),          curl,   SLOT(updateBios(ActorList)));
    connect(curl,   SIGNAL(updateSingleProfile(ActorPtr)),  this,   SLOT(receiveSingleActor(ActorPtr)));
    connect(curl,   SIGNAL(updateFinished(ActorList)),      this,   SLOT(receiveActors(ActorList)));
    /// Set up the SQL Thread for communications with the main thread
    connect(this,   SIGNAL(saveActorChanges(ActorPtr)),     sql,    SLOT(updateActor(ActorPtr)));
    connect(this,   SIGNAL(saveActors(ActorList)),          sql,    SLOT(store(ActorList)));
    connect(this,   SIGNAL(saveScenes(SceneList)),          sql,    SLOT(store(SceneList)));
    connect(ui->actionLoad_Actors,SIGNAL(triggered()),      sql,    SLOT(loadActors()));
    connect(ui->pb_refreshActors, SIGNAL(pressed()),      sql,    SLOT(loadActors()));
    connect(ui->pb_refreshScenes, SIGNAL(pressed()),      sql,    SLOT(loadScenes()));
    connect(sql,    SIGNAL(sendResult(ActorList)),          this,   SLOT(receiveActors(ActorList)));
    connect(sql,    SIGNAL(sendResult(SceneList)),          this,   SLOT(receiveScenes(SceneList)));
    connect(this,   SIGNAL(saveChangesToDB(ScenePtr)),      sql,    SLOT(saveChanges(ScenePtr)));
    connect(ui->cb_companyFilter,       SIGNAL(currentIndexChanged(QString)),   this,               SIGNAL(cb_companyFilterChanged(QString)));
    connect(ui->cb_ethnicity,           SIGNAL(currentIndexChanged(QString)),   actorProxyModel,    SLOT(setFilterEthnicity(QString)));
    connect(ui->cb_hairColor,           SIGNAL(currentIndexChanged(QString)),   actorProxyModel,    SLOT(setFilterHairColor(QString)));
    connect(ui->actionCleanDatabase,    SIGNAL(triggered()),                    sql,                SLOT(purgeScenes()));
    /// Connect Actor Profile Widget with Database & Curl Thread
    ui->profileWidget->hide();
    connect(ui->profileWidget,  SIGNAL(saveToDatabase(ActorPtr)),   sql,    SLOT(updateActor(ActorPtr)));
    connect(ui->profileWidget,  SIGNAL(updateFromWeb(ActorPtr)),    curl,   SLOT(updateBio(ActorPtr)));
    connect(ui->profileWidget,  SIGNAL(downloadPhoto(ActorPtr)),    curl,   SLOT(downloadPhoto(ActorPtr)));
    connect(ui->profileWidget,  SIGNAL(deleteCurrent()),            this,   SLOT(removeActorItem()));
    connect(ui->profileWidget,  SIGNAL(deleteActor(ActorPtr)),      sql,    SLOT(drop(ActorPtr)));
    connect(this,               SIGNAL(dropActor(ActorPtr)),        sql,    SLOT(drop(ActorPtr)));
    connect(ui->profileWidget,  SIGNAL(apv_to_ct_updateBio(QString)),   curl,               SLOT(apv_to_ct_getProfile(QString)));
    connect(curl,               SIGNAL(ct_to_apv_sendActor(ActorPtr)),  ui->profileWidget,  SLOT(loadActorProfile(ActorPtr)));


    /** Scanning Routing Data Passing **/
    connect(sql,    SIGNAL(db_to_ct_buildActors(QStringList)),      curl,   SLOT(db_to_ct_buildActors(QStringList)));
    connect(sql,    SIGNAL(db_to_mw_sendActors(ActorList)),         this,   SLOT(db_to_mw_receiveActors(ActorList)));
    connect(curl,   SIGNAL(ct_to_db_storeActors(ActorList)),        sql,    SLOT(ct_to_db_storeActors(ActorList)));
    connect(sql,    SIGNAL(db_to_mw_sendScenes(SceneList)),         this,   SLOT(db_to_mw_receiveScenes(SceneList)));
    qDebug("Starting Curl Thread");
    /// Start the Threads
    this->curlThread = new QThread();
    curl->moveToThread(curlThread);
    qDebug("Curl Tool Moved to Thread");
    curlThread->start();
    qDebug("Curl Thread Started");
}

/** \brief Choose a Directory to scan files in from */
void MainWindow::on_actionScan_Directory_triggered(){
    this->fileDialog = new QFileDialog(this, "Choose Directory to Scan For Videos", "/Volumes");
    connect(fileDialog, SIGNAL(fileSelected(QString)), this, SLOT(scan_directory_chosen(QString)));
    connect(fileDialog, SIGNAL(accepted()), fileDialog, SLOT(deleteLater()));
    connect(fileDialog, SIGNAL(rejected()), fileDialog, SLOT(deleteLater()));
    this->fileDialog->show();
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
            this->scanner = new FileScanner(root_directory);
            connect(scanner,SIGNAL(fs_to_db_checkNames(QStringList)),   sql,    SLOT(fs_to_db_checkNames(QStringList)));
            connect(scanner,SIGNAL(fs_to_db_storeScenes(SceneList)),    sql,    SLOT(fs_to_db_storeScenes(SceneList)));
            connect(scanner,SIGNAL(startProgress(QString,int)),         this,   SLOT(startProgress(QString,int)));
            connect(scanner,SIGNAL(updateProgress(int)),                ui->progressBar, SLOT(setValue(int)));
            connect(scanner,SIGNAL(closeProgress(QString)),             this,   SLOT(closeProgress(QString)));
            connect(scanner,SIGNAL(updateStatus(QString)),              ui->statusLabel, SLOT(setText(QString)));
            connect(scanner,SIGNAL(showError(QString)),                 this,   SLOT(showError(QString)));
            connect(scanner,SIGNAL(finished()),                         scanner, SLOT(deleteLater()));
            scanner->start();
        }
    }
}

void MainWindow::db_to_mw_receiveActors(ActorList list){
    qDebug("Main Window Received %d Actors from the SQL Thread. Adding them to the Display list", list.size());
    foreach(ActorPtr a, list){
        QString name = a->getName();
        //qDebug("Adding %s with ID %llu to actorMap", qPrintable(name), a->getID());
        actorModel->appendRow(a->buildQStandardItem());
        actorMap.insert(name, a);
        actorModel->sort(SCENE_NAME_COLUMN, Qt::AscendingOrder);
    }
    ui->actorTableView->resizeColumnsToContents();
    ui->actorTableView->resizeRowsToContents();

    qDebug("Finished Adding %d actors!", list.size());
}

void MainWindow::db_to_mw_receiveScenes(SceneList list){
    qDebug("Adding %d Scenes to view...", list.size());
    foreach(ScenePtr s, list){
        //qDebug("Adding Scene with ID %d to List", s->getID());
        sceneModel->appendRow(s->buildQStandardItem());
        sceneMap.insert(s->getID(), s);
    }
    emit resizeSceneView();
    qDebug("Added %d Scenes!", list.size());
}

void MainWindow::actorSelectionChanged(QModelIndex current, QModelIndex /*previous*/){
    if (current.isValid() && current.row() > -1){
        QString name = actorProxyModel->data(actorProxyModel->index(current.row(), ACTOR_NAME_COLUMN), Qt::DisplayRole).toString();
        if (actorMap.contains(name)){
            this->currentActorIndex = current;
            this->currentActor = ActorPtr(0);
            this->currentActor = actorMap.value(name);
            qDebug("'%s' Selected", qPrintable(name));
            if (!ui->profileWidget->isHidden()){
                emit loadActorProfile(currentActor);
            }
        } else {
            qWarning("Actor Map doesn't Contain '%s'", qPrintable(name));
        }
    }
    if (!this->sceneDetailView->isHidden()){
        this->sceneDetailView->clearDisplay();
        this->sceneDetailView->hide();
    }
}

/** \brief Slot called when an item in the actor list is clicked
 *  \param  QModelIndex index: the index of the selected actor.
 */
void MainWindow::actorTableView_clicked(const QModelIndex &index){
    this->currentActorIndex = index;
    if (index.row() > -1){
        this->itemSelected = true;
        // Get the name of the selected actor.
        QString name = actorProxyModel->data(actorProxyModel->index(index.row(), ACTOR_NAME_COLUMN), Qt::DisplayRole).toString();
        if (actorMap.contains(name)){
            this->currentActor = actorMap.value(name);
            emit loadActorProfile(currentActor);
        } else {
            qCritical("Name not in map: %s", qPrintable(name));
        }
    }
}

/** \brief Find out which actor is currently selected, and return an ActorPtr object to it, or a null pointer if no actor is currently selected. */
ActorPtr MainWindow::getSelectedActor(){
    ActorPtr a = QSharedPointer<Actor>(0);
    this->currentActorIndex = ui->actorTableView->currentIndex();
    QString name = actorModel->data(actorModel->index(currentActorIndex.row(), ACTOR_NAME_COLUMN), Qt::DisplayRole).toString();
    if (actorMap.contains(name)){
        a = actorMap.value(name);
        this->currentActor = a;
    } else {
        qWarning("Error Retrieving Selected Actor");
    }
    return a;
}

QModelIndex MainWindow::findActorIndex(QString name){
    QAbstractItemModel *model = ui->actorTableView->model();
    QSortFilterProxyModel proxy;
    proxy.setSourceModel(model);
    proxy.setFilterKeyColumn(ACTOR_NAME_COLUMN);
    proxy.setFilterFixedString(name);
    QModelIndex matchingIndex = proxy.mapToSource(proxy.index(0,0));
    if (!matchingIndex.isValid()){
        QMessageBox::critical(this, tr("Error Finding Actor"), QString("Error Locating %1").arg(name), QMessageBox::Abort);
        matchingIndex = QModelIndex();
    }
    return matchingIndex;
}

void MainWindow::on_actionDeleteActor_triggered(){
    qDebug("Delete Actor Shortcut Detected");
    if (!currentActor.isNull()){
        emit dropActor(currentActor);
        this->removeActorItem();
    }
}

void MainWindow::removeActorItem(){
    if (!currentActor.isNull()){
        qDebug("Removing '%s' from main window display & map", qPrintable(currentActor->getName()));
        actorProxyModel->removeRow(currentActorIndex.row());
        if (!actorMap.contains(currentActor->getName())){
            qWarning("Can't remove '%s' from map - not in map!", qPrintable(currentActor->getName()));
        } else {
            actorMap.remove(currentActor->getName());
        }
    }
}
void MainWindow::showCurrentActorProfile(){
    if (!currentActor.isNull()){
        emit loadActorProfile(currentActor);
    }
}

/** \brief Show an error Dialog with the provided Text. */
void MainWindow::showError(QString message){
    qWarning("Showing Error Dialog with text: '%s'", qPrintable(message));
    QMessageBox box(QMessageBox::Warning, tr("Error"), message, QMessageBox::Ok, this, Qt::WindowStaysOnTopHint);
    box.exec();
}
/** \brief Show a success dialog with the provided text */
void MainWindow::showSuccess(QString message){
    qDebug("Message Received: '%s'", qPrintable(message));
    QMessageBox box(QMessageBox::Information, tr("Success"), message, QMessageBox::Ok, this, Qt::WindowStaysOnTopHint);
    box.exec();
}

/** \brief Initialize the progress bar to 0, set its new maximum value, and update the text on the status bar
 *  \param QString status:  Text to show on the status bar
 *  \param int max:         Upper boundary of progress bar.
 */
void MainWindow::startProgress(QString status, int max){
    //qDebug("************* %s (%d items) ******************", qPrintable(status), max);
    ui->statusLabel->setText(status);
    ui->progressBar->setMaximum(max);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setValue(0);
    ui->progressBar->setEnabled(true);
}

/** \brief Set the progress bar to 100%, and update the text on the status bar.
 *  \param QString status:  Text to show on the status bar.
 */
void MainWindow::closeProgress(QString status){
    //qDebug("************** Task Complete (Message: '%s') *************************", qPrintable(status));
    ui->statusLabel->setText(status);
    ui->progressBar->setValue(ui->progressBar->maximum());
}
/** \brief  Update the Main Window's built-in progress bar. */
void MainWindow::updateProgress(int value)  {
    ui->progressBar->setValue(value);
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
        this->progressDialog->setValue(progressDialog->maximum());
        this->progressDialog->hide();
        progressDialog->deleteLater();
    }
}

/** \brief Update the status bar.
 *  \param int value:   value to set the progress bar to. */
void MainWindow::updateStatus(QString s)    {
    ui->statusLabel->setText(s);
}

/** \brief Slot to receive list of scenes. */
void MainWindow::receiveScenes(SceneList list){
    foreach(ScenePtr s, list){
        if (!sceneMap.contains(s->getID())){
            sceneMap.insert(s->getID(), s);
            sceneModel->appendRow(s->buildQStandardItem());
        }
    }
    QStringList companies = sql->getCompanyList();
    foreach(QString company, companies){
        ui->cb_companyFilter->addItem(company);
    }
    emit resizeSceneView();
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

/** \brief Slot to Receive a list of Actors */
void MainWindow::receiveActors(ActorList list){
    if (!list.isEmpty()){   // if the passed list has items in it, selectively add them to the main actor map.
        foreach(ActorPtr a, list){
            QString name = a->getName();
            if (!name.isEmpty()){
                if (!actorMap.contains(name)){
                    actorMap.insert(a->getName(), a);
                    actorModel->appendRow(a->buildQStandardItem());
                } else {
                    Biography b = a->getBio();
                    int newBioSize = b.size();
                    int oldBioSize = actorMap[name]->getBio().size();
                    if (newBioSize > oldBioSize){
                        if (name == currentActor->getName()){
                            currentActor->setBio(b);
                            currentActor->updateQStandardItem();
                            emit loadActorProfile(currentActor);
                        } else {
                            actorMap[name]->setBio(b);
                            actorMap[name]->updateQStandardItem();
                        }
                    }
                }
            }
        }
        resetActorFilterSelectors();
        ui->actorTableView->resizeColumnsToContents();
        ui->actorTableView->resizeRowsToContents();
        this->actorProxyModel->sort(ACTOR_NAME_COLUMN, Qt::AscendingOrder);
    }
    qDebug("Actor List updated.");
}

/** \brief Receive an actor that has had its profile updated via the curl thread.
 *  Update the actor's stored data with the new data if the new bio has more information than the old one.
 *  If the Actor that was updated is the currently selected actor, update the profile view (if open), and
 *  save the new biographical details to the database.
 */
void MainWindow::receiveSingleActor(ActorPtr a){
    QString name = a->getName();
    if (actorMap.contains(name)){
        this->updatedActor = actorMap.value(name);
        Biography newBio = a->getBio();
        qDebug("Updating Bio for %s", qPrintable(name));
        updatedActor->setBio(newBio);
        updatedActor->updateQStandardItem();
        if (name == currentActor->getName() && !ui->profileWidget->isHidden()){
            qDebug("Updating Profile View");
            emit loadActorProfile(updatedActor);
            emit saveActorChanges(currentActor);
        } else {
            qDebug("Saving Updates to database");
            emit saveActorChanges(updatedActor);
        }
    } else {
        qDebug("Adding New Actor, %s", qPrintable(name));
        actorMap.insert(name, a);
        ActorList newList = { a };
        emit saveActors(newList);
    }
    ui->actorTableView->resizeColumnsToContents();
    ui->actorTableView->resizeRowsToContents();
    this->actorModel->sort(SCENE_NAME_COLUMN);
}

/** \brief  Receive a list of scenes that have been created by the file scanner thread, and a list of names
 *          of actors that appeared in those scenes.
 *  \param SceneList s: The List of Scenes scanned in.
 *  \param QStringList newNames:    List of names of actors appearing in the scenes scanned in.
 */


void MainWindow::on_pb_saveScenes_clicked(){
    if (!sceneMap.isEmpty()){
        this->sceneList = fromHashMap(sceneMap);
        qDebug("Saving %d scenes to database", sceneList.size());
        emit saveScenes(sceneList);
    } else {
        qDebug("No Scenes in list, not saving to database");
    }
}

void MainWindow::on_pb_saveActors_clicked(){
    if (actorMap.isEmpty()){
        QMessageBox::warning(this, tr("Can't Save Actors"), tr("No Actors Loaded! Nothing to save."), QMessageBox::Close);
    } else {
        actorList = MapToList(actorMap);
        qDebug("Saving %d actors to database", actorList.size());
        emit saveActors(actorList);
    }
}

/** \brief Triggered by a Button on Main Window. Updates all the display items. */
void MainWindow::on_actionRefresh_Display_triggered(){
    if (actorMap.isEmpty()){
        QMessageBox::warning(this, tr("Nothing to Refresh"), tr("No Display Items are present, there's nothing to refresh!"), QMessageBox::Close);
        qDebug("No Actors Loaded yet!");
    } else {
        qDebug("Updating Display Items for %d Actors", actorMap.size());
        ActorIterator it(actorMap);
        while(it.hasNext()){
            it.next();
            it.value()->updateQStandardItem();
        }
        ui->statusLabel->setText(QString("%1 Actor Displays Updated!").arg(actorMap.size()));
        qDebug("Actor Displays Updated!");
    }
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
                emit loadActorProfile(currentActor);
            } else {
                qWarning("Error Copying %s to %s", qPrintable(source_filename), qPrintable(new_filename));
            }
        }
    }
}


/** \brief Save all Actor Items to the Actor Table of the Database */
void MainWindow::on_actionSave_Scenes_triggered(){
    qDebug("Saving Actors to Database");
    actorList = MapToList(actorMap);
    emit saveActors(actorList);
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
        if (!actorMap.contains(a->getName())){
            qDebug("Adding %s to List", qPrintable(a->getName()));
            a->buildScaledProfilePhoto();
            this->actorModel->appendRow(a->buildQStandardItem());
            actorMap.insert(a->getName(), a);
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


void MainWindow::on_actionUpdate_Bios_triggered(){
    updateList.clear();
    QMapIterator<QString, ActorPtr>it(actorMap);
    while(it.hasNext()){
        it.next();
        ActorPtr a = it.value();
        if (a->size() < MINIMUM_BIO_SIZE){
            updateList.push_back(a);
        }
    }
    emit updateBios(updateList);
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
                scene->updateQStandardItem();
                qDebug("Display Updated");
                emit saveChangesToDB(scene);
                qDebug("Database Updated");
            }
        }
    }
}

void MainWindow::playVideo(int sceneID){
    if (sceneMap.contains(sceneID)){
        ScenePtr curr = sceneMap.value(sceneID);
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
   emit loadActorProfile(currentActor);
}
