#include "mainwindow.h"
#include "filenames.h"
#include "ui_mainwindow.h"
#include "sceneParser.h"
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

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    this->runMode = Debug;
    this->setWindowIcon(QIcon(QPixmap("SceneQuery.icns")));
    ui->setupUi(this);
    actorList = {};
    sceneList = {};
    updateList = {};
    displayActors = {};
    displayScenes = {};
    this->currentDisplay = DISPLAY_ACTORS;
    qRegisterMetaType<int>("int");
    qRegisterMetaType<QString>("QString");
    qRegisterMetaType<QStringList>("QStringList");
    qRegisterMetaType<SceneList>("SceneList");
    qRegisterMetaType<QSharedPointer<Scene>>("QSharedPointer<Scene>");
    qRegisterMetaType<QSharedPointer<Actor>>("QSharedPointer<Actor>");
    qRegisterMetaType<QVector<QSharedPointer<Scene>>>("QVector<QSharedPointer<Scene>>");
    qRegisterMetaType<QVector<QSharedPointer<Actor>>>("QVector<QSharedPointer<Actor>>");
    qRegisterMetaType<ScenePtr>("ScenePtr");
    qRegisterMetaType<ActorPtr>("ActorPtr");
    qRegisterMetaType<SceneList>("SceneList");
    qRegisterMetaType<ActorList>("ActorList");
    setupViews();
    this->videoOpen = false;
}

MainWindow::~MainWindow(){
    delete ui;
    actorMap.clear();
    actorList.clear();
    sceneList.clear();
}

/** \brief Set up the main display */
void MainWindow::setupViews(){

    /// Set up Actor Table View
    this->actorHeaders << "" << "Name" << "Hair Color" << "Ethnicity" << "Scenes";
    this->actorModel = new QStandardItemModel();
    actorModel->setHorizontalHeaderLabels(actorHeaders);
    this->actorParent = actorModel->invisibleRootItem();
    this->actorProxyModel = new QSortFilterProxyModel(this);
    this->actorProxyModel->setSourceModel(actorModel);
    ui->actorTableView->setModel(actorProxyModel);
    ui->actorTableView->setSortingEnabled(true);
    ui->actorTableView->verticalHeader()->hide();
    ui->actorTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->actorTableView->setSelectionMode(QAbstractItemView::SingleSelection);

    /// Set up the Scene Table View
    this->sceneHeaders << "Main Actor" << "Title" << "Company" << "Resolution" << "Featured Actors" << "File Size" << "Length" << "Released" << "Rating";
    this->sceneModel = new QStandardItemModel();
    this->sceneProxyModel = new SceneProxyModel(this);
    this->sceneParent = sceneModel->invisibleRootItem();
    sceneModel->setHorizontalHeaderLabels(sceneHeaders);
    sceneProxyModel->setSourceModel(sceneModel);
    ui->sceneWidget->setSourceModel(sceneModel);

    /// Set Up Profile Widget
    this->actorModel->setSortRole(Qt::DecorationRole);
    ui->profileWidget->hide();
    ui->progressBar->setValue(0);

    /// Create Thread Objects
    this->sqlThread  = new SQL();
    this->curlThread = new curlTool();
    this->scanner    = new FileScanner();
    /// Make Relevant connections between widgets
    connect(ui->profileWidget,  SIGNAL(hidden()),                       ui->sceneWidget,    SLOT(clearFilter()));
    connect(ui->profileWidget,  SIGNAL(reloadProfile()),                this,               SLOT(refreshCurrentActor()));
    connect(ui->sceneWidget,    SIGNAL(sendSceneCount(int)),            ui->profileWidget,  SLOT(acceptSceneCount(int)));
    connect(ui->profileWidget,  SIGNAL(requestSceneCount()),            ui->sceneWidget,    SLOT(receiveSceneCountRequest()));
    connect(ui->profileWidget,  SIGNAL(chooseNewPhoto()),               this,               SLOT(selectNewProfilePhoto()));
    connect(this,               SIGNAL(loadActorProfile(ActorPtr)),     ui->profileWidget,  SLOT(loadActorProfile(ActorPtr)));
    connect(this,               SIGNAL(loadActorProfile(ActorPtr)),     ui->sceneWidget,    SLOT(actorFilterChanged(ActorPtr)));
    connect(this,               SIGNAL(cb_companyFilterChanged(QString)),ui->sceneWidget,   SLOT(companyFilterChanged(QString)));

    connect(ui->profileWidget,  SIGNAL(clearChanges()),                 this,               SLOT(reloadProfile()));
    connect(ui->profileWidget,  SIGNAL(chooseNewPhoto()),               this,               SLOT(selectNewProfilePhoto()));
    connect(ui->sceneWidget,    SIGNAL(playFile(QString)),              this,               SLOT(playVideo(QString)));
    connect(ui->actorTableView, SIGNAL(clicked(QModelIndex)),           this,               SLOT(actorTableView_clicked(QModelIndex)));
    connect(this,               SIGNAL(resizeSceneView()),              ui->sceneWidget,    SLOT(resizeSceneView()));
    connect(this,               SIGNAL(skipInitialization(ActorList,SceneList)), this,      SLOT(initializationFinished(ActorList,SceneList)));
    connect(this,               SIGNAL(startInitialization()),          sqlThread,          SLOT(initialize()));
    connect(sqlThread,          SIGNAL(initializationFinished(ActorList,SceneList)), this,  SLOT(initializationFinished(ActorList,SceneList)));
    connect(sqlThread,          SIGNAL(startProgress(QString,int)),     this,               SLOT(newProgressDialog(QString, int)));
    connect(sqlThread,          SIGNAL(updateProgress(int)),            this,               SLOT(updateProgressDialog(int)));
    connect(sqlThread,          SIGNAL(closeProgress()),                this,               SLOT(closeProgressDialog()));

    sqlThread->start();
}

void MainWindow::showEvent(QShowEvent */*event*/){
    if (this->runMode == Release){
        qDebug("Loading Actors");
        emit loadActors(actorList);
    } else {
        qDebug("Skipping initial load from Database");
        emit skipInitialization(actorList, sceneList);
    }
}

void MainWindow::initializationFinished(ActorList actors, SceneList scenes){
    if (runMode == Release){
        qDebug("\n\tMain Window Received %d Actors & %d Scenes from the init thread", actors.size(), scenes.size());
        foreach(ScenePtr s, scenes){
            sceneModel->appendRow(s->buildQStandardItem());
            sceneList.push_back(s);
        }
        foreach(ActorPtr a, actors){
            actorModel->appendRow(a->buildQStandardItem());
            actorMap.insert(a->getName(), a);
        }
        qDebug("All items added to GUI");
        /// Set up Filter boxes
        QStringList companies = sqlThread->getCompanyList();
        foreach(QString company, companies){
            ui->cb_companyFilter->addItem(company);
        }
    }
    /*
    int index = 0;
    connect(this, SIGNAL(newProgressDialogBox(QString,int)),this, SLOT(newProgressDialog(QString,int)));
    connect(this, SIGNAL(updateProgressDialogBox(int)),     this, SLOT(updateProgressDialog(int)));
    connect(this, SIGNAL(closeProgressDialogBox()),         this, SLOT(closeProgressDialog()));

    connect(this, SIGNAL(startProgressBar(QString,int)),    this, SLOT(startProgress(QString,int)));
    connect(this, SIGNAL(updateProgressBar(int)),           this, SLOT(updateProgress(int)));
    connect(this, SIGNAL(closeProgressBar(QString)),        this, SLOT(closeProgress(QString)));

    connect(this, SIGNAL(updateStatusLabel(QString)),       this, SLOT(updateStatus(QString)));
    // Start the progress bar & set the label
    emit newProgressDialogBox(QString("Adding %1 Scenes to the model").arg(scenes.size()), scenes.size());
    this->sceneList = scenes;
    QStringList cast;
    foreach(ScenePtr s, scenes){
        // ui->sceneWidget->addScene(s);
        sceneModel->appendRow(s->buildQStandardItem());
        sceneList.push_back(s);
        ++index;
        if ((index % 300) == 0){
            emit updateProgressDialogBox(index);
        }
    }
    emit closeProgressDialogBox();
    this->threadedProgressCounter = 0;
    emit newProgressDialogBox(QString("Building Scaled Profile Photos for %1 Actors").arg(actors.size()), actors.size());
    QFutureSynchronizer<void> sync;
    for(int i = 0; i < actors.size(); ++i){
        ActorPtr curr = actors.at(i);
        sync.addFuture(QtConcurrent::run(this, &MainWindow::threaded_profile_photo_scaler, curr));
    }
    sync.waitForFinished();
    emit closeProgressDialogBox();
    emit newProgressDialogBox(QString("Adding %1 Actors to Model").arg(actors.size()), actors.size());
    foreach(ActorPtr a, actors){
        if (!actorMap.contains(a->getName())){
            a->setSceneCount(sceneList.countScenesWithActor(a));
            actorMap.insert(a->getName(), a);
            actorModel->appendRow(a->buildQStandardItem());
        }
        ++index;
        if ((index % 50) == 0){
           emit updateProgressDialogBox(index);
        }
    }
    emit closeProgressDialogBox();
    */

    ui->statusLabel->setText(QString("%1 Actors & %2 Scenes Loaded!").arg(actors.size()).arg(scenes.size()));
    ui->actorTableView->resizeColumnsToContents();
    setupThreads();
}


/** \brief  Set up all persistent background threads, and connect their signals and slots to the main window.
 */
void MainWindow::setupThreads(){
    qDebug("Initializing Worker Threads");
    disconnect(sqlThread, SIGNAL(initializationFinished(ActorList,SceneList)),  this,   SLOT(initializationFinished(ActorList,SceneList)));
    disconnect(sqlThread, SIGNAL(startProgress(QString,int)),                   this,   SLOT(newProgressDialog(QString,int)));
    disconnect(sqlThread, SIGNAL(updateProgress(int)),                          this,   SLOT(updateProgressDialog(int)));
    disconnect(sqlThread, SIGNAL(closeProgress()),                              this,   SLOT(closeProgressDialog()));

    /// PROGRESS & STATUS BAR UPDATING
    connect(scanner,    SIGNAL(startProgress(QString,int)), this,       SLOT(startProgress(QString,int)));
    connect(curlThread, SIGNAL(startProgress(QString,int)), this,       SLOT(startProgress(QString,int)));
    connect(sqlThread,  SIGNAL(startProgress(QString,int)), this,       SLOT(startProgress(QString,int)));
    connect(curlThread, SIGNAL(updateProgress(int)),        this,       SLOT(updateProgress(int)));
    connect(scanner,    SIGNAL(updateProgress(int)),        this,       SLOT(updateProgress(int)));
    connect(sqlThread,  SIGNAL(updateProgress(int)),        this,       SLOT(updateProgress(int)));
    connect(curlThread, SIGNAL(closeProgress(QString)),     this,       SLOT(closeProgress(QString)));
    connect(scanner,    SIGNAL(closeProgress(QString)),     this,       SLOT(closeProgress(QString)));
    connect(sqlThread,  SIGNAL(closeProgress(QString)),     this,       SLOT(closeProgress(QString)));
    connect(scanner,    SIGNAL(updateStatus(QString)),      this,       SLOT(updateStatus(QString)));
    connect(sqlThread,  SIGNAL(updateStatus(QString)),      this,       SLOT(updateStatus(QString)));
    /// SHOW MESSAGE DIALOGS
    connect(sqlThread,  SIGNAL(showError(QString)),         this,       SLOT(showError(QString)));
    connect(scanner,    SIGNAL(showError(QString)),         this,       SLOT(showError(QString)));
    connect(curlThread, SIGNAL(showError(QString)),         this,       SLOT(showError(QString)));
    connect(sqlThread,  SIGNAL(showSuccess(QString)),       this,       SLOT(showSuccess(QString)));
    /// Stopping threads
    connect(this,       SIGNAL(stopThreads()),              sqlThread,  SLOT(stopThread()));
    connect(this,       SIGNAL(stopThreads()),              curlThread, SLOT(stopThread()));
    connect(this,       SIGNAL(stopThreads()),              scanner,    SLOT(stopThread()));
    /// Set up file scanner connections with main thread
    connect(this,       SIGNAL(scanFolder(QString)),        scanner,    SLOT(scanFolder(QString)));
    /// Set up curl thread communications with main thread
    //connect(this,       SIGNAL(getHeadshots(ActorList)),    curlThread, SLOT(downloadPhotos(ActorList)));
    connect(this,       SIGNAL(updateBios(ActorList)),      curlThread, SLOT(updateBios(ActorList)));
    //connect(this,       SIGNAL(updateSingleBio(ActorPtr)),  curlThread, SLOT(updateBio(ActorPtr)));
    connect(curlThread, SIGNAL(updateSingleProfile(ActorPtr)), this,    SLOT(receiveSingleActor(ActorPtr)));
    connect(curlThread, SIGNAL(updateFinished(ActorList)),  this,       SLOT(receiveActors(ActorList)));
    /// Set up the SQL Thread for communications with the main thread
    connect(this,       SIGNAL(saveActorChanges(ActorPtr)), sqlThread,  SLOT(updateActor(ActorPtr)));
    connect(this,       SIGNAL(saveActors(ActorList)),      sqlThread,  SLOT(store(ActorList)));
    connect(this,       SIGNAL(saveScenes(SceneList)),      sqlThread,  SLOT(store(SceneList)));
    connect(this,       SIGNAL(loadActors(ActorList)),      sqlThread,  SLOT(load(ActorList)));
    connect(this,       SIGNAL(loadScenes(SceneList)),      sqlThread,  SLOT(load(SceneList)));
    connect(sqlThread,  SIGNAL(sendResult(ActorList)),      this,       SLOT(receiveActors(ActorList)));
    connect(sqlThread,  SIGNAL(sendResult(SceneList)),      this,       SLOT(receiveScenes(SceneList)));
    connect(this,       SIGNAL(saveChangesToDB(ScenePtr)),  sqlThread,  SLOT(saveChanges(ScenePtr)));
    connect(this,       SIGNAL(purgeScenes()),              sqlThread,  SLOT(purgeScenes()));
//    connect(sqlThread,  SIGNAL(sceneSaveComplete()),        this,       SLOT();
//    connect(sqlThread,  SIGNAL(actorSaveComplete()),        this,       SLOT();
    /// Connect Actor Profile Widget with Database & Curl Thread
    ui->profileWidget->hide();
    connect(ui->profileWidget,  SIGNAL(saveToDatabase(ActorPtr)),   sqlThread,          SLOT(updateActor(ActorPtr)));
    connect(ui->profileWidget,  SIGNAL(updateFromWeb(ActorPtr)),    curlThread,         SLOT(updateBio(ActorPtr)));
    connect(ui->profileWidget,  SIGNAL(downloadPhoto(ActorPtr)),    curlThread,         SLOT(downloadPhoto(ActorPtr)));
    connect(ui->profileWidget,  SIGNAL(deleteActor(ActorPtr)),      this,               SLOT(deleteActor(ActorPtr)));
    connect(ui->profileWidget,  SIGNAL(deleteActor(ActorPtr)),      sqlThread,          SLOT(drop(ActorPtr)));

    /** Scanning Routing Data Passing **/
    connect(scanner,    SIGNAL(fs_to_db_checkNames(QStringList)),   sqlThread, SLOT(fs_to_db_checkNames(QStringList)));
    connect(scanner,    SIGNAL(fs_to_db_storeScenes(SceneList)),    sqlThread, SLOT(fs_to_db_storeScenes(SceneList)));
    connect(sqlThread,  SIGNAL(db_to_ct_buildActors(QStringList)),  curlThread,SLOT(db_to_ct_buildActors(QStringList)));
    connect(sqlThread,  SIGNAL(db_to_mw_sendActors(ActorList)),     this,      SLOT(db_to_mw_receiveActors(ActorList)));
    connect(curlThread, SIGNAL(ct_to_db_storeActors(ActorList)),    sqlThread, SLOT(ct_to_db_storeActors(ActorList)));
    connect(sqlThread,  SIGNAL(db_to_mw_sendScenes(SceneList)),     this,      SLOT(db_to_mw_receiveScenes(SceneList)));

    /// Start the Threads
    this->curlThread->start();
    this->scanner->start();
}

void MainWindow::db_to_mw_receiveActors(ActorList list){
    qDebug("Main Window Received %d Actors from the SQL Thread. Adding them to the Display list", list.size());
    foreach(ActorPtr a, list){
        QString name = a->getName();
        //qDebug("Adding %s with ID %llu to actorMap", qPrintable(name), a->getID());
        actorModel->appendRow(a->buildQStandardItem());
        actorMap.insert(name, a);
        actorModel->sort(NAME_COLUMN, Qt::AscendingOrder);
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
        sceneList.push_back(s);
    }
    emit resizeSceneView();
    qDebug("Added %d Scenes!", list.size());
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
            emit actorSelectionChanged(name);
        } else {
            qCritical("Name not in map: %s", qPrintable(name));
        }
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

/** \brief Refresh Scenes from Database */
void MainWindow::on_pb_refreshScenes_clicked(){
    qDebug("Updating Scenes from Database");
    emit loadScenes(this->sceneList);
}
/** \brief Refresh Actors from Database */
void MainWindow::on_pb_refreshActors_clicked(){
    qDebug("Updating Scenes from Database");
    emit loadActors(this->actorList);
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
    if (!root_directory.isEmpty()){
        QDir path(root_directory);
        if (path.exists()){
            emit scanFolder(root_directory);
        }
    }
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
    if (progressDialog){
        this->progressDialog->setValue(value);
    }
}
/** \brief Update the text label on the progress dialog box */
void MainWindow::updateProgressDialog(QString s){
    if (progressDialog){
        this->progressDialog->setLabelText(s);
    }
}

/** \brief  Close & Delete the pop-up progress dialog window */
void MainWindow::closeProgressDialog(){
    if (progressDialog){
        this->progressDialog->setValue(progressDialog->maximum());
        this->progressDialog->hide();
        delete progressDialog;
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
        if (!sceneList.contains(s)){
            sceneModel->appendRow(s->buildQStandardItem());
            sceneList.push_back(s);
        }
    }
    QStringList companies = sqlThread->getCompanyList();
    foreach(QString company, companies){
        ui->cb_companyFilter->addItem(company);
    }
    emit resizeSceneView();
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
    this->actorModel->sort(NAME_COLUMN);
}

/** \brief  Receive a list of scenes that have been created by the file scanner thread, and a list of names
 *          of actors that appeared in those scenes.
 *  \param SceneList s: The List of Scenes scanned in.
 *  \param QStringList newNames:    List of names of actors appearing in the scenes scanned in.
 */


void MainWindow::on_pb_saveScenes_clicked(){
    if (sceneList.size() > 0){
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

void MainWindow::refreshCurrentActor(){
    if (!this->currentActor.isNull()){
        QString new_filename = getHeadshotName(this->currentActor->getName());
        QFileInfo info(new_filename);
        if (info.exists() && info.size() > 200){
            this->currentActor->setHeadshot(new_filename);
            this->currentActor->updateQStandardItem();
            emit loadActorProfile(currentActor);
        }
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


/** \brief Find out which actor is currently selected, and return an ActorPtr object to it, or a null pointer if no actor is currently selected. */
ActorPtr MainWindow::getSelectedActor(){
    ActorPtr a = QSharedPointer<Actor>(0);
    if (currentActorIndex.isValid() && (currentActorIndex.row() > -1)){
        QString name = actorModel->data(actorModel->index(currentActorIndex.row(), ACTOR_NAME_COLUMN), Qt::DisplayRole).toString();
        if (actorMap.contains(name)){
            a = actorMap.value(name);
        } else {
            qWarning("Error Retrieving Selected Actor");
        }
    }
    return a;
}

/** \brief Load all entries from the Actor Table in the Database */
void MainWindow::on_actionLoad_Actors_triggered(){
    emit loadActors(actorList);
}
/** \brief Save all Actor Items to the Actor Table of the Database */
void MainWindow::on_actionSave_Scenes_triggered(){
    qDebug("Saving Actors to Database");
    actorList = MapToList(actorMap);
    emit saveActors(actorList);
}
/** \brief Reload the data shown in the current ActorProfileView Widget, if it is open. */
void MainWindow::reloadProfile()  {
    if (!ui->profileWidget->isHidden() && !currentActor.isNull()){
        emit loadActorProfile(currentActor);
    }
}

/** \brief Test Function to verify Lexical Parsing of Scene Titles.
 *  Parses a single video file's title, and opens a dialog window showing the data extracted. */
void MainWindow::on_actionParse_Scene_triggered(){
    QString filename = QFileDialog::getOpenFileName(this, tr("Select Video to Parse"), "/Volumes");
    if (!filename.isEmpty()){
        ui->statusLabel->setText(QString("Parsing %1").arg(filename));
        sceneParser input;
        input.parse(filename);
        ScenePtr temp = ScenePtr(new Scene(input));
        sceneParser output(temp);
        output.formatFilename();
        QString message = output.displayInfo();
        QMessageBox box(QMessageBox::NoIcon, tr("Scene Parser Test"), message, QMessageBox::Close, this, Qt::WindowStaysOnTopHint);
        box.exec();
        ui->statusLabel->setText("");
    }
}



/** Open a Dialog that allows the user to 'manually' add an actor by entering a name and looking up the corrosponding profile. */
void MainWindow::on_actionAdd_Actor_triggered(){
    QString newName = QInputDialog::getText(this, tr("Enter Actor Name"), tr("Enter an actor name to build a profile for"));

    if (!newName.isEmpty()){
        this->addProfileDialog = new ProfileDialog(newName, this);
        connect(addProfileDialog, SIGNAL(pd_to_ct_getProfile(QString)),     curlThread, SLOT(pd_to_ct_getActor(QString)));
        connect(addProfileDialog, SIGNAL(pd_to_db_saveProfile(ActorPtr)),   sqlThread,  SLOT(pd_to_db_saveActor(ActorPtr)));
        connect(curlThread,       SIGNAL(ct_to_pd_sendActor(ActorPtr)),     addProfileDialog, SLOT(ct_to_pd_receiveProfile(ActorPtr)));
        connect(sqlThread,        SIGNAL(db_to_pd_sendBackWithID(ActorPtr)),addProfileDialog, SLOT(db_to_pd_receiveProfileWithID(ActorPtr)));
        connect(addProfileDialog, SIGNAL(pd_to_mw_addDisplayItem(ActorPtr)), this, SLOT(pd_to_mw_addActorToDisplay(ActorPtr)));
        connect(addProfileDialog, SIGNAL(closed()),                         this,       SLOT(closeAddActorDialog()));
        addProfileDialog->show();
    }
}

void MainWindow::closeAddActorDialog(){
    addProfileDialog->deleteLater();
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
        this->curlTestThread = new curlTool();
        connect(curlTestThread, SIGNAL(updateSingleProfile(ActorPtr)),  this,           SLOT(receiveTestBio(ActorPtr)));
        connect(this,           SIGNAL(updateSingleBio(ActorPtr)),      curlTestThread, SLOT(updateBio(ActorPtr)));
        curlTestThread->start();
        ActorPtr a = ActorPtr(new Actor(name));
        emit updateSingleBio(a);
    }
}
/** \brief Test Function, paired with on_actionCreate_Bio_triggered():
 *  Receives updated actor object from curlThread and opens a dialog showing the details. */
void MainWindow::receiveTestBio(ActorPtr a){
    qDebug("Receieved actor %s from Curl Test Thread", qPrintable(a->getName()));
    this->testProfileDialog = new ProfileDialog(a, this);
    connect(testProfileDialog, SIGNAL(closed()), this, SLOT(testProfileDialogClosed()));
    this->testProfileDialog->show();
}

/** \brief Close and delete the test dialog for biography retrieval. */
void MainWindow::testProfileDialogClosed(){
    if (testProfileDialog){
        testProfileDialog->deleteLater();
    }
    curlTestThread->terminate();
    curlTestThread->wait();
    if (curlTestThread){
        curlTestThread->deleteLater();
    }
}

void MainWindow::deleteActor(ActorPtr a){
    if (!a.isNull()){
        if (!this->currentActor.isNull() && (a->getName() == currentActor->getName())){
            qDebug("Deleting Currently Selected Actor, %s", qPrintable(currentActor->getName()));
            qDebug("Removing '%s' list item", qPrintable(currentActor->getName()));
            //actorModel->removeRow(currentActorIndex.row());
            actorProxyModel->removeRow(currentActorIndex.row());
            actorMap.remove(currentActor->getName());
            // Delete the Photo
            if (!currentActor->usingDefaultPhoto()){
                QFile file(currentActor->getHeadshot());
                if (file.exists()){
                    file.remove();
                }
            }
            ui->profileWidget->hide();
        }
    }
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

void MainWindow::playVideo(QString filepath){
    if (!videoOpen){
        if (QFileInfo(filepath).exists()){
            this->videoPlayer = new VideoPlayer(filepath, this);
            connect(videoPlayer, SIGNAL(error(QString)), this, SLOT(showError(QString)));
            connect(videoPlayer, SIGNAL(finished()), this, SLOT(videoFinished()));
            this->videoOpen = true;
        }
    } else {
        qWarning("Video Player Already Open!");
    }
}

void MainWindow::videoFinished(){
    if (videoPlayer){
        delete videoPlayer;
    }
    this->videoOpen = false;
}

void MainWindow::on_actionCleanDatabase_triggered(){
    emit purgeScenes();
}

void MainWindow::on_cb_companyFilter_currentIndexChanged(const QString &arg1){
    emit cb_companyFilterChanged(arg1);
}
