#include "mainwindow.h"
#include "filenames.h"
#include "ui_mainwindow.h"
#include "sceneParser.h"
#include "FilePath.h"
#include "FileScanner.h"
#include "Scene.h"
#include "Actor.h"
#include "ActorProfileView.h"
#include "InitializationThread.h"
#include <QInputDialog>
#include <QtGlobal>
#include <QDebug>
#include <QMessageBox>
#include <QVector>
#include <QFileDialog>
#include <QRegExp>
#define MINIMUM_BIO_SIZE 6

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
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
}

MainWindow::~MainWindow(){
    delete ui;
    actorMap.clear();
    actorList.clear();
    sceneList.clear();
}

/** \brief Set up the main display */
void MainWindow::setupViews(){
    actorHeaders << "" << "Name" << "Hair Color" << "Ethnicity" << "Scenes";
    sceneHeaders << "Main Actor" << "Title" << "Company" << "Quality" << "Featured Actors" << "Release Date" << "Length" << "Rating";
    sceneModel = new QStandardItemModel();
    actorModel = new QStandardItemModel();
    sceneSubsetModel = QSharedPointer<QStandardItemModel>(new QStandardItemModel());
    actorSubsetModel = QSharedPointer<QStandardItemModel>(new QStandardItemModel());

    sceneParent = sceneModel->invisibleRootItem();
    actorParent = actorModel->invisibleRootItem();
    this->actorModel->setHorizontalHeaderLabels(actorHeaders);
    this->sceneModel->setHorizontalHeaderLabels(sceneHeaders);
    this->actorSubsetModel->setHorizontalHeaderLabels(actorHeaders);
    this->sceneSubsetModel->setHorizontalHeaderLabels(sceneHeaders);
    this->actorProxyModel = new QSortFilterProxyModel(this);
    this->sceneProxyModel = new SceneProxyModel(this);
    this->actorProxyModel->setSourceModel(actorModel);
    this->sceneProxyModel->setSourceModel(sceneModel);
    ui->sceneWidget->setSourceModel(sceneModel);
    /// Set Up Profile Widget
    ui->actorView->setSortingEnabled(true);
    ui->actorView->setModel(actorModel);

    ui->actorView->horizontalHeader()->setStyleSheet("background-color: rgb(73,73,73);");
    ui->actorView->verticalHeader()->hide();
    ui->profileWidget->hide();
    ui->progressBar->setValue(0);
    connect(this, SIGNAL(actorSelectionChanged(QString)), ui->sceneWidget, SLOT(actorFilterChanged(QString)));
}

void MainWindow::showEvent(QShowEvent */*event*/){
    this->initThread = new InitializationThread();
    connect(initThread, SIGNAL(startProgress(QString,int)),             this, SLOT(newProgressDialog(QString, int)));
    connect(initThread, SIGNAL(updateProgress(int)),                    this, SLOT(updateProgressDialog(int)));
    connect(initThread, SIGNAL(sendInitialLists(ActorList,SceneList)),  this, SLOT(initializationFinished(ActorList, SceneList)));
    initThread->start();
    //emit loadActors(actorList);
}

void MainWindow::initializationFinished(ActorList actors, SceneList scenes){
    int index = 0;
    qDebug("\n\tMain Window Received %d Actors & %d Scenes from the init thread", actors.size(), scenes.size());
    connect(this, SIGNAL(newProgressDialogBox(QString,int)),this, SLOT(newProgressDialog(QString,int)));
    connect(this, SIGNAL(updateProgressDialogBox(int)),     this, SLOT(updateProgressDialog(int)));
    connect(this, SIGNAL(closeProgressDialogBox()),         this, SLOT(closeProgressDialog()));

    connect(this, SIGNAL(startProgressBar(QString,int)),    this, SLOT(startProgress(QString,int)));
    connect(this, SIGNAL(updateProgressBar(int)),           this, SLOT(updateProgress(int)));
    connect(this, SIGNAL(closeProgressBar(QString)),        this, SLOT(closeProgress(QString)));

    connect(this, SIGNAL(updateStatusLabel(QString)),       this, SLOT(updateStatus(QString)));
    // Start the progress bar & set the label
    /*
    ui->progressBar->setEnabled(true);
    ui->progressBar->setRange(0, actors.size() + scenes.size());
    ui->progressBar->setValue(0);
    ui->statusLabel->setText(QString("Adding %1 Scenes to the model").arg(scenes.size()));
    */
    emit newProgressDialogBox(QString("Adding %1 Scenes to the model").arg(scenes.size()), actors.size() + scenes.size());
    this->sceneList = scenes;
    QStringList cast;
    foreach(ScenePtr s, scenes){
        /* ui->sceneWidget->addScene(s); */
        sceneModel->appendRow(s->buildQStandardItem());
        sceneList.push_back(s);
        ++index;
        if ((index % 300) == 0){
            emit updateProgressDialogBox(index);
        }
    }
    emit updateProgressDialogBox(QString("Adding %1 Actors to Model").arg(actors.size()));
    foreach(ActorPtr a, actors){
        if (!actorMap.contains(a->getName())){
            a->setSceneCount(sceneList.countScenesWithActor(a->getName()));
            actorMap.insert(a->getName(), a);
            actorModel->appendRow(a->buildQStandardItem());
        }
        ++index;
        if ((index % 50) == 0){
           emit updateProgressDialogBox(index);
        }
    }

    emit closeProgressDialogBox();
    ui->statusLabel->setText(QString("%1 Actors & %2 Scenes Loaded!").arg(actors.size()).arg(scenes.size()));
    ui->actorView->resizeColumnsToContents();
    setupThreads();
}


/** \brief  Set up all persistent background threads, and connect their signals and slots to the main window.
 */
void MainWindow::setupThreads(){
    qDebug("Initializing Worker Threads");
    this->sqlThread = new SQL();
    this->curlThread = new curlTool();
    this->scanner = new FileScanner();
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
    connect(scanner,    SIGNAL(scanComplete(ActorList)),    this,       SLOT(receiveActors(ActorList)));
    connect(scanner,    SIGNAL(scanComplete(SceneList, QStringList)),    this,       SLOT(receiveScanResult(SceneList, QStringList)));
    connect(this,       SIGNAL(scanFolder(QString)),                scanner,    SLOT(scanFolder(QString)));
    connect(this,       SIGNAL(scanActors(SceneList,ActorList)),    scanner,    SLOT(scanForActors(SceneList,ActorList)));
    /// Set up curl thread communications with main thread
    //connect(this,       SIGNAL(getHeadshots(ActorList)),    curlThread, SLOT(downloadPhotos(ActorList)));
    connect(this,       SIGNAL(updateBios(ActorList)),      curlThread, SLOT(updateBios(ActorList)));
    //connect(this,       SIGNAL(updateSingleBio(ActorPtr)),  curlThread, SLOT(updateBio(ActorPtr)));
    connect(curlThread, SIGNAL(updateSingleProfile(ActorPtr)), this,    SLOT(receiveSingleActor(ActorPtr)));
    connect(curlThread, SIGNAL(updateFinished(ActorList)),  this,       SLOT(receiveActors(ActorList)));
    connect(this,       SIGNAL(makeNewActors(QStringList)), curlThread, SLOT(makeNewActors(QStringList)));
    /// Set up the SQL Thread for communications with the main thread
    connect(this,       SIGNAL(saveActorChanges(ActorPtr)), sqlThread,  SLOT(updateActor(ActorPtr)));
    connect(this,       SIGNAL(saveActors(ActorList)),      sqlThread,  SLOT(store(ActorList)));
    connect(this,       SIGNAL(saveScenes(SceneList)),      sqlThread,  SLOT(store(SceneList)));
    connect(this,       SIGNAL(loadActors(ActorList)),      sqlThread,  SLOT(load(ActorList)));
    connect(this,       SIGNAL(loadScenes(SceneList)),      sqlThread,  SLOT(load(SceneList)));
    connect(sqlThread,  SIGNAL(sendResult(ActorList)),      this,       SLOT(receiveActors(ActorList)));
    connect(sqlThread,  SIGNAL(sendResult(SceneList)),      this,       SLOT(receiveScenes(SceneList)));
    connect(this,       SIGNAL(saveChangesToDB(ScenePtr)),  sqlThread,  SLOT(saveChanges(ScenePtr)));
//    connect(sqlThread,  SIGNAL(sceneSaveComplete()),        this,       SLOT();
//    connect(sqlThread,  SIGNAL(actorSaveComplete()),        this,       SLOT();
    /// Connect Actor Profile Widget with Database & Curl Thread
    ui->profileWidget->hide();
    connect(ui->profileWidget, SIGNAL(saveToDatabase(ActorPtr)), sqlThread, SLOT(updateActor(ActorPtr)));
    connect(ui->profileWidget, SIGNAL(updateFromWeb(ActorPtr)), curlThread, SLOT(updateBio(ActorPtr)));
    connect(ui->profileWidget, SIGNAL(clearChanges()), this,  SLOT(reloadProfile()));
    connect(this, SIGNAL(loadActorProfile(ActorPtr)), ui->profileWidget, SLOT(loadActorProfile(ActorPtr)));
    connect(ui->profileWidget, SIGNAL(chooseNewPhoto()), this, SLOT(selectNewProfilePhoto()));

    /// Start the Threads
    this->sqlThread->start();
    this->curlThread->start();
    this->scanner->start();
}

/** \brief Slot called when an item in the actor list is clicked
 *  \param  QModelIndex index: the index of the selected actor.
 */
void MainWindow::on_actorView_clicked(const QModelIndex &index){
    this->currentActorIndex = index;
    if (index.row() > -1){
        this->itemSelected = true;
        // Get the name of the selected actor.
        QString name = actorModel->data(actorModel->index(index.row(), NAME_COLUMN), Qt::DisplayRole).toString();
        if (actorMap.contains(name)){
            this->currentActor = actorMap.value(name);
            emit loadActorProfile(currentActor);
            //loadActorProfile(currentActor);
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
void MainWindow::on_refreshScenes_clicked(){
    qDebug("Updating Scenes from Database");
    emit loadScenes(this->sceneList);
}
/** \brief Refresh Actors from Database */
void MainWindow::on_refreshActors_clicked(){
    qDebug("Updating Scenes from Database");
    emit loadActors(this->actorList);
}

/** \brief Choose a Directory to scan files in from */
void MainWindow::on_actionScan_Directory_triggered(){
    QString path = QFileDialog::getExistingDirectory(this, "Pick Directory to Scan", "/Volumes");
    if (!path.isEmpty()){
        emit scanFolder(path);
    }
}

/** \brief Initialize the progress bar to 0, set its new maximum value, and update the text on the status bar
 *  \param QString status:  Text to show on the status bar
 *  \param int max:         Upper boundary of progress bar.
 */
void MainWindow::startProgress(QString status, int max){
    qDebug("************* %s (%d items) ******************", qPrintable(status), max);
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
    qDebug("************** Task Complete (Message: '%s') *************************", qPrintable(status));
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
    qDebug("***********%s**************", qPrintable(label));
    this->progressDialog = new QProgressDialog(label, "", 0, max, this, Qt::WindowStaysOnTopHint);
    this->progressDialog->resize(400, 200);
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

/** \brief  Close & Delete the pop-up progress dialog window */
void MainWindow::closeProgressDialog(){
    if (progressDialog){
        this->progressDialog->setValue(progressDialog->maximum());
        this->progressDialog->hide();
        delete progressDialog;
    }
}
/** \brief Update the text label on the progress dialog box */
void MainWindow::updateProgressDialog(QString s){
    if (progressDialog){
        this->progressDialog->setLabelText(s);
    }
}

/** \brief Update the status bar.
 *  \param int value:   value to set the progress bar to. */
void MainWindow::updateStatus(QString s)    {
    qDebug("\n\tUpdating Status to \"%s\"\n", qPrintable(s));
    ui->statusLabel->setText(s);
}

/** \brief Slot to receive list of scenes. */
void MainWindow::receiveScenes(SceneList list){
    if (!this->sceneList.isEmpty()){
        foreach(ScenePtr s, list){
            if (!sceneList.contains(s)){
                sceneList.push_back(s);
            }
        }
    } else {
        this->sceneList = list;
    }
    qDebug("Scenes Updated");
}


/** \brief Slot to Receive a list of Actors
 */
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
    }
    actorProxyModel->sort(NAME_COLUMN, Qt::AscendingOrder);
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
        Biography oldBio = updatedActor->getBio();
        if (newBio.size() > oldBio.size()){
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
            qDebug("not updating item, refreshing display");
            currentActor->updateQStandardItem();
        }
    } else {
        qDebug("Adding New Actor, %s", qPrintable(name));
        actorMap.insert(name, a);
        ActorList newList = { a };
        emit saveActors(newList);
    }
}

/** \brief  Receive a list of scenes that have been created by the file scanner thread, and a list of names
 *          of actors that appeared in those scenes.
 *  \param SceneList s: The List of Scenes scanned in.
 *  \param QStringList newNames:    List of names of actors appearing in the scenes scanned in.
 */
void MainWindow::receiveScanResult(SceneList s, QStringList newNames){
    this->names = newNames;
    qDebug("Received %d new Scenes and %d new names", s.size(), newNames.size());
#warning Adding un-numbered scenes to scenes to display here. Change it later so all displayed scenes must come from the database
    for (int i = 0; i < s.size(); ++i){
        sceneList.push_back(s.at(i));
    }
    qDebug("All Scenes & Names added.");
}

void MainWindow::on_saveScenes_clicked(){
    if (sceneList.size() > 0){
        qDebug("Saving %d scenes to database", sceneList.size());
        emit saveScenes(sceneList);
    } else {
        qDebug("No Scenes in list, not saving to database");
    }
}

void MainWindow::on_saveActors_clicked(){
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


/** \brief Find out which actor is currently selected, and return an ActorPtr object to it, or a null pointer if no actor is currently selected. */
ActorPtr MainWindow::getSelectedActor(){
    ActorPtr a = QSharedPointer<Actor>(0);
    if (currentActorIndex.isValid() && (currentActorIndex.row() > -1)){
        QString name = actorModel->data(actorModel->index(currentActorIndex.row(), NAME_COLUMN), Qt::DisplayRole).toString();
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
        input.parse(FilePath(filename));
        ScenePtr temp = ScenePtr(new Scene(input));
        sceneParser output(temp);
        output.formatFilename();
        QString message = output.displayInfo();
        QMessageBox box(QMessageBox::NoIcon, tr("Scene Parser Test"), message, QMessageBox::Close, this, Qt::WindowStaysOnTopHint);
        box.exec();
        ui->statusLabel->setText("");
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

