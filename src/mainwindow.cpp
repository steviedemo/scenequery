#include "mainwindow.h"
#include "filenames.h"
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
    QStringList headers;

    headers << "" << "Name" << "Hair Color" << "Ethnicity";
    sceneModel = new QStandardItemModel();
    actorModel = new QStandardItemModel();
    sceneParent = sceneModel->invisibleRootItem();
    actorParent = actorModel->invisibleRootItem();
    this->actorModel->setHorizontalHeaderLabels(headers);
    this->proxyModel = new QSortFilterProxyModel(this);
    this->proxyModel->setSourceModel(actorModel);
    ui->actorView->setSortingEnabled(true);
    ui->actorView->setModel(actorModel);
    //ui->actorView->horizontalHeader()->hide();
    ui->actorView->horizontalHeader()->setStyleSheet("background-color: rgb(73,73,73);");
    ui->actorView->verticalHeader()->hide();
    ui->sceneView->setModel(sceneModel);
    ui->profileView->hide();
    ui->progressBar->setValue(0);
    setResetAndSaveButtons(false);
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
    //connect(this,       SIGNAL(updateBios(ActorList)),      curlThread, SLOT(updateBios(ActorList)));
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
    /// Start the Threads
    this->sqlThread->start();
    this->curlThread->start();
    this->scanner->start();
}

void MainWindow::showEvent(QShowEvent */*event*/){
    emit loadActors(actorList);
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
            ui->profileView->show();
            this->currentActor = actorMap.value(name);
            loadActorProfile(currentActor);
            setResetAndSaveButtons(false);
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
    setResetAndSaveButtons(false);
}
/** \brief Update the status bar.
 *  \param int value:   value to set the progress bar to. */

void MainWindow::updateProgress(int value)  {   ui->progressBar->setValue(value);       }
void MainWindow::on_closeProfile_clicked()  {   ui->profileView->hide();                }
void MainWindow::on_resetProfile_clicked()  {   loadActorProfile(currentActor);         }
void MainWindow::on_scanFiles_clicked()     {   on_actionScan_Directory_triggered();    }
void MainWindow::updateStatus(QString s)    {   ui->statusLabel->setText(s);            }

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
            QString name = a->getName();
            if (!name.isEmpty()){
                if (!actorMap.contains(name)){
                    actorMap.insert(a->getName(), a);
                    actorModel->appendRow(a->buildQStandardItem());
                } else {
                    int currentBioSize = actorMap[name]->getBio().size();
                    Biography b = a->getBio();
                    int storedBioSize = b.size();
                    if (storedBioSize > currentBioSize){
                        actorMap[name]->setBio(b);
                        actorMap[name]->updateQStandardItem();
                    }
                }
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
            } else {
                qWarning("Error Copying %s to %s", qPrintable(source_filename), qPrintable(new_filename));
            }
        }
    }
}

void MainWindow::on_profile_photo_customContextMenuRequested(const QPoint &pos){
    QMenu rightClickMenu(tr("Profile Photo"), this);
    QAction selectNew(tr("Choose New Profile Photo"), this);
    connect(&selectNew, SIGNAL(triggered(bool)), this, SLOT(on_assignProfilePhoto_clicked()));
    rightClickMenu.addAction(&selectNew);
    rightClickMenu.exec(mapToGlobal(pos));
}
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

/** \brief Load & Display Details of Selected Actor
 *  \param ActorPtr a:  Actor whose Profile should be shown.
 */
void MainWindow::loadActorProfile(ActorPtr a){
    ui->display_name->setText(a->getName());
    Biography bio = a->getBio();
    ui->ethnicityLineEdit->setText(bio.getEthnicity());
    ui->eyeColorLineEdit->setText(bio.getEyeColor());
    ui->hairColorLineEdit->setText(bio.getHairColor());
    ui->measurementsLineEdit->setText(bio.getMeasurements());
    QString city = bio.getCity();
    QString country = bio.getNationality();
    /// Set the birthplace
    QString birthplace("");
    if (!city.isEmpty() && !country.isEmpty()){
        birthplace = QString("%1, %2").arg(city).arg(country);
    } else if (!country.isEmpty()) {
        birthplace = country;
    } else {
        birthplace = "Unknown";
    }
    Height height = bio.getHeight();
    if (height.nonZero()){
        ui->heightLineEdit->setText(height.toString());
    }
    int weight = bio.getWeight();
    if (weight > 0){
        QString temp = QString::number(weight);
        ui->weightLineEdit->setText(temp);
    }
    QString age("");
    QDate birthday = bio.getBirthday();
    QString birthdayLabel("Unknown");
    if (!birthday.isNull() && birthday.isValid()){
        ui->birthDateDateEdit->setDate(birthday);
    } else {
        ui->birthDateDateEdit->setDate(QDate(1900, 01, 01));
    }
    ui->nationalityLineEdit->setText(birthplace);
    /// Set the content of the multi-line text boxes.
    ui->aliasesEdit->setText(bio.getAliases());
    ui->piercingsEdit->setText(bio.getPiercings());
    ui->tattoosEdit->setText(bio.getTattoos());
    FilePath photo = a->getHeadshot();

    if (photo.exists()){
        QPixmap profilePhoto(photo.absolutePath());
        ui->profile_photo->setPixmap(profilePhoto.scaledToHeight(IMAGE_HEIGHT));
    } else {
        QPixmap profilePhoto(":/Icons/blank_profile_photo.png");
        ui->profile_photo->setPixmap(profilePhoto.scaledToHeight(IMAGE_HEIGHT));
    }
}

/** \brief Save any changes made to the currently displayed profile */
void MainWindow::on_saveProfile_clicked(){
    ActorPtr a = getSelectedActor();
    if (!a.isNull()){
        a->setAliases(ui->aliasesEdit->toPlainText());
        a->setPiercings(ui->piercingsEdit->toPlainText());
        a->setEthnicity(ui->ethnicityLineEdit->text());
        a->setEyes(ui->eyeColorLineEdit->text());
        a->setHair(ui->hairColorLineEdit->text());
        a->setMeasurements(ui->measurementsLineEdit->text());
        a->setTattoos(ui->tattoosEdit->toPlainText());
        // Read in Birthdate
        QDate birthday = ui->birthDateDateEdit->date();
        if (birthday != QDate(1900, 1, 1)){
            a->setBirthday(birthday);
        }
        // Read in height
        QRegularExpression heightRx("([0-9])\\'([1]?[0-9])\\\"");
        QRegularExpressionMatch heightMatch = heightRx.match(ui->heightLineEdit->text());
        if (heightMatch.hasMatch()){
            bool feetOk = false, inchesOk = false;
            int feet = heightMatch.captured(1).toInt(&feetOk);
            int inches = heightMatch.captured(2).toInt(&inchesOk);
            if (feetOk && inchesOk){
                Height h(feet, inches);
                a->setHeight(h);
            }
        }
        // Read in weight
        QString weightString = ui->weightLineEdit->text();
        if (!weightString.isEmpty() && !weightString.isNull() && weightString != "0"){
            bool ok = false;
            int w = weightString.toInt(&ok);
            if (ok){    a->setWeight(w); }
        }
        // Read in Birthplace
        QRegularExpression birthplaceRx("([\\s\\'A-Za-z]+),\\s*(.+)");
        QRegularExpressionMatch bpMatch = birthplaceRx.match(ui->nationalityLineEdit->text());
        if (bpMatch.hasMatch()){
            a->setCity(bpMatch.captured(1));
            a->setNationality(bpMatch.captured(2));
        } else {
            a->setNationality(ui->nationalityLineEdit->text());
        }
        emit saveActorChanges(a);
    }
}

void MainWindow::setResetAndSaveButtons(bool enabled){
    if (enabled){
        ui->resetProfile->setEnabled(true);
        ui->saveProfile->setEnabled(true);
    } else {
        ui->resetProfile->setDisabled(true);
        ui->saveProfile->setDisabled(true);
    }
}

void MainWindow::on_birthDateDateEdit_userDateChanged(const QDate &/*date*/){   setResetAndSaveButtons(true);   }
void MainWindow::on_hairColorLineEdit_textChanged(const QString &/*arg1*/)  {   setResetAndSaveButtons(true);   }
void MainWindow::on_ethnicityLineEdit_textChanged(const QString &)          {   setResetAndSaveButtons(true);   }
void MainWindow::on_nationalityLineEdit_textEdited(const QString &)         {   setResetAndSaveButtons(true);   }
void MainWindow::on_heightLineEdit_textEdited(const QString &)              {   setResetAndSaveButtons(true);   }
void MainWindow::on_weightLineEdit_textEdited(const QString &)              {   setResetAndSaveButtons(true);   }
void MainWindow::on_eyeColorLineEdit_textEdited(const QString &)            {   setResetAndSaveButtons(true);   }
void MainWindow::on_measurementsLineEdit_textEdited(const QString &)        {   setResetAndSaveButtons(true);   }
void MainWindow::on_aliasesEdit_textChanged()                               {   setResetAndSaveButtons(true);   }
void MainWindow::on_piercingsEdit_textChanged()                             {   setResetAndSaveButtons(true);   }
void MainWindow::on_tattoosEdit_textChanged()                               {   setResetAndSaveButtons(true);   }
