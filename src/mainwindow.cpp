#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "FilePath.h"
#include "FileScanner.h"
#include "Scene.h"
#include "Actor.h"
#include <QtGlobal>
#include <QDebug>
#include <QMessageBox>
#include <QVector>
#include <QFileDialog>
#define IMAGE_WIDTH 250
#define IMAGE_HEIGHT 300

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);
    actorList = {};
    sceneList = {};
    this->currentDisplay = DISPLAY_ACTORS;
    qRegisterMetaType<int>("int");
    qRegisterMetaType<QString>("QString");
    qRegisterMetaType<QStringList>("QStringList");
    qRegisterMetaType<QSharedPointer<Scene>>("QSharedPointer<Scene>");
    qRegisterMetaType<QSharedPointer<Actor>>("QSharedPointer<Actor>");
    qRegisterMetaType<QVector<QSharedPointer<Scene>>>("QVector<QSharedPointer<Scene>>");
    qRegisterMetaType<QVector<QSharedPointer<Actor>>>("QVector<QSharedPointer<Actor>>");
    qRegisterMetaType<ScenePtr>("ScenePtr");
    qRegisterMetaType<ActorPtr>("ActorPtr");
    qRegisterMetaType<SceneList>("SceneList");
    qRegisterMetaType<ActorList>("ActorList");

    setupViews();

    ui->assignProfilePhoto->setEnabled(false);
    this->blankImage = new QPixmap(":/Icons/blank_profile_photo.png");
    ui->profile_photo->setPixmap(blankImage->scaledToHeight(IMAGE_HEIGHT));
    setupThreads();
}

MainWindow::~MainWindow(){
    delete ui;
    actorList.clear();
    sceneList.clear();
}

/** \brief Set up the main display */
void MainWindow::setupViews(){
    sceneModel = new QStandardItemModel();
    actorModel = new QStandardItemModel();
    sceneParent = sceneModel->invisibleRootItem();
    actorParent = actorModel->invisibleRootItem();

    QStringList headers;
    headers << "" << "Name" << "Hair Color" << "Ethnicity";
    this->actorModel->setHorizontalHeaderLabels(headers);
    this->proxyModel = new QSortFilterProxyModel(this);
    this->proxyModel->setSourceModel(actorModel);
    ui->actorView->setSortingEnabled(true);
    ui->actorView->setModel(actorModel);
    ui->sceneView->setModel(sceneModel);
}

/** \brief  Set up all persistent background threads, and connect their signals and slots to the main window.
 */
void MainWindow::setupThreads(){
    qDebug("Initializing Worker Threads");
    this->sqlThread = new SQL();
    this->curlThread = new curlTool();
    this->scanner = new FileScanner();
    /// Allow the File Scanning Thread and the Curl Request thread to communicate, so Actor Bios can be updated.
    //connect(scanner,    SIGNAL(updateBios(ActorList)),      curlThread, SLOT(updateBios(ActorList)));
    //connect(curlThread, SIGNAL(updateFinished(ActorList)),  scanner,    SLOT(receiveUpdatedActors(ActorList)));
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
    //connect(this,       SIGNAL(updateBios(ActorList)),      curlThread, SLOT(updateBios(ActorList)));
    connect(curlThread, SIGNAL(updateFinished(ActorList)),  this,       SLOT(receiveActors(ActorList)));
    connect(this,       SIGNAL(makeNewActors(QStringList)), curlThread, SLOT(updateBios(QStringList)));
    /// Set up the SQL Thread for communications with the main thread
    connect(this,       SIGNAL(saveActors(ActorList)),      sqlThread,  SLOT(store(ActorList)));
    connect(this,       SIGNAL(saveScenes(SceneList)),      sqlThread,  SLOT(store(SceneList)));
    connect(this,       SIGNAL(loadActors(ActorList)),      sqlThread,  SLOT(load(ActorList)));
    connect(this,       SIGNAL(loadScenes(SceneList)),      sqlThread,  SLOT(load(SceneList)));
    connect(sqlThread,  SIGNAL(sendResult(ActorList)),      this,       SLOT(receiveActors(ActorList)));
    connect(sqlThread,  SIGNAL(sendResult(SceneList)),      this,       SLOT(receiveScenes(SceneList)));
    connect(this,       SIGNAL(saveChangesToDB(ScenePtr)),  sqlThread,  SLOT(saveChanges(ScenePtr)));
//    connect(sqlThread,  SIGNAL(sceneSaveComplete()),        this,       SLOT();
//    connect(sqlThread,  SIGNAL(actorSaveComplete()),        this,       SLOT();
    /// Start the Threads
    this->sqlThread->start();
    this->curlThread->start();
    this->scanner->start();
}

/** \brief Slot called when an item in the actor list is clicked
 *  \param  QModelIndex index: the index of the selected actor.
 */
void MainWindow::on_actorView_clicked(const QModelIndex &index){
    if (index.row() > -1){
        this->itemSelected = true;
        ui->assignProfilePhoto->setEnabled(true);
        this->currentActorIndex = index;
        // Get the name of the selected actor.
        QString name = actorModel->data(actorModel->index(index.row(), NAME_COLUMN), Qt::DisplayRole).toString();
        if (actorMap.contains(name)){
            this->currentActor = actorMap.value(name);
            ui->display_name->setText(currentActor->getName());
            Biography bio = currentActor->getBio();
            ui->display_aliases->setText(bio.getAliases());
            ui->display_ethnicity->setText(bio.getEthnicity());
            ui->display_eyecolor->setText(bio.getEyeColor());
            ui->display_haircolor->setText(bio.getHairColor());
            ui->display_measurements->setText(bio.getMeasurements());
            ui->display_nationality->setText(bio.getNationality());
            ui->display_piercings->setText(bio.getPiercings());
            ui->display_tattoos->setText(bio.getTattoos());
            QString age("");
            QDate birthday = bio.getBirthday();
            if (!birthday.isNull() && birthday.isValid()){
                age = QString("%1 Years Old").arg(birthday.daysTo(QDate::currentDate())/365);
            } else {
                age = "Age Unknown";
            }
            ui->display_age->setText(age);
            FilePath photo = currentActor->getHeadshot();

            if (photo.exists()){
                QPixmap profilePhoto(photo.absolutePath());
                ui->profile_photo->setPixmap(profilePhoto.scaledToHeight(IMAGE_HEIGHT));
            } else {
                QPixmap profilePhoto(":/Icons/blank_profile_photo.png");
                ui->profile_photo->setPixmap(profilePhoto.scaledToHeight(IMAGE_HEIGHT));
            }
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

void MainWindow::on_radioButtonActors_clicked(){
    if (ui->radioButtonActors->isChecked()){
        this->currentDisplay = DISPLAY_ACTORS;
    } else {
        this->currentDisplay = DISPLAY_SCENES;
    }
    updateDisplayType();
}


void MainWindow::updateDisplayType(){
    if (currentDisplay == DISPLAY_SCENES){
        ui->actorView->hide();

    } else if (currentDisplay == DISPLAY_ACTORS){
        ui->actorView->show();
    }
}
/** \brief Update the text on the status bar
 *  \param QString s:   Text to show on the status bar.
 */
void MainWindow::updateStatus(QString s){
    ui->statusLabel->setText(s);
}
/** \brief Initialize the progress bar to 0, set its new maximum value, and update the text on the status bar
 *  \param QString status:  Text to show on the status bar
 *  \param int max:         Upper boundary of progress bar.
 */
void MainWindow::startProgress(QString status, int max){
    ui->statusLabel->setText(status);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(max);
    ui->progressBar->setEnabled(true);
}

/** \brief Set the progress bar to 100%, and update the text on the status bar.
 *  \param QString status:  Text to show on the status bar.
 */
void MainWindow::closeProgress(QString status){
    ui->statusLabel->setText(status);
    ui->progressBar->setValue(ui->progressBar->maximum());
}
/** \brief Update the status bar.
 *  \param int value:   value to set the progress bar to.
 */
void MainWindow::updateProgress(int value){
    ui->progressBar->setValue(value);
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
    if (!list.isEmpty()){
        foreach(ActorPtr a, list){
            if (!a->getName().isEmpty()){
                actorMap.insert(a->getName(), a);
                actorModel->appendRow(a->buildQStandardItem());
            }
        }
        sortActors();
        proxyModel->sort(NAME_COLUMN, Qt::AscendingOrder);
    }
    qDebug("Actor List updated.");
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
    if (actorList.size()  > 0){
        qDebug("Saving %d actors to database", actorList.size());
        emit saveActors(actorList);
    } else {
        qDebug("No Actors to save to database");
    }
}

void MainWindow::on_scanFiles_clicked(){
    on_actionScan_Directory_triggered();
}

void MainWindow::on_updateActorBios_clicked(){
    if (this->names.isEmpty()){
        qDebug("No Names to create actors from");
    } else {
        qDebug("Making actors out of names in list generated from scene scan");
        emit makeNewActors(names);
    }
}

void MainWindow::on_updateDisplay_clicked(){
    if (actorList.size() > 0){
        qDebug("Updating Display Items for %d Actors", actorList.size());
        foreach(ActorPtr a, actorList){
            a->updateQStandardItem();
        }
        ui->statusLabel->setText("Actor Displays Updated!");
        qDebug("Actor Displays Updated!");
    } else {
        qDebug("No Actors Loaded yet!");
    }
}

void MainWindow::sortActors(){
    if (!actorList.isEmpty()){
        qSort(actorList.begin(), actorList.end(), [](const ActorPtr &a, const ActorPtr &b){ return (a->getName() < b->getName()); });
    }
}

/** \brief  Assign a new profile picture for the currently selected actor
 */
void MainWindow::on_assignProfilePhoto_clicked(){
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
            } else {
                qWarning("Error Copying %s to %s", qPrintable(source_filename), qPrintable(new_filename));
            }
        }
    }
}
