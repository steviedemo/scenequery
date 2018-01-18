#include "SceneDetailView.h"
#include "ui_SceneDetailView.h"
#include <QList>
#include <QInputDialog>
#include <QtGui>
#include <QLabel>
#include <QListIterator>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include "SceneParser.h"
#include "Scene.h"
#include "genericfunctions.h"
#include "Rating.h"
SceneDetailView::SceneDetailView(QWidget *parent) :
    QWidget(parent)//, ui(new Ui::SceneDetailView)
{
#ifdef DESIGNER
    //ui->setupUi(this);
    this->castList      << ui->actor1label  << ui->actor2label  << ui->actor3label  << ui->actor4label;
    this->ageList       << ui->age1label    << ui->age2label    << ui->age3label    << ui->age4label;
    this->ageLabelList  << ui->labelAge_1   << ui->labelAge_2   << ui->labelAge_3   << ui->labelAge_4;
    this->dataFields    << ui->titleLineEdit;
    this->dataFields    << ui->filenameLineEdit;
    this->dataFields    << ui->companyLineEdit  << ui->seriesLineEdit;
    this->dataFields    << ui->durationLineEdit << ui->sizeLineEdit     << ui->resolutionLineEdit;
    this->dataFields    << ui->addedLineEdit    << ui->openedLineEdit   << ui->releasedLineEdit;
    connect(ui->tb_add_actor, SIGNAL(pressed()), this, SLOT(addActorToScene()));
    connect(ui->pb_play, SIGNAL(pressed()), this, SLOT(playCurrentVideo()));
    connect(ui->pb_hide, SIGNAL(pressed()), this, SLOT(hide()));
    QStringList ratingValues = getRatingList();
    foreach(QString rating, ratingValues){
        ui->ratingComboBox->addItem(rating, rating);
    }
#else
    buildUI();
    for(int i = 0; i < 4; ++i){
        QLabel *label = castList.at(i);
        label->setMouseTracking(true);
        label->setOpenExternalLinks(false);
        label->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        connect(castList.at(i), SIGNAL(linkActivated(QString)), this, SLOT(actorLinkClicked(QString)));
    }
    connect(addActor,   SIGNAL(pressed()), this, SLOT(addActorToScene()));
    connect(playButton,       SIGNAL(pressed()), this, SLOT(playCurrentVideo()));
    connect(hideButton,       SIGNAL(pressed()), this, SLOT(hide()));
    QStringList ratingList = getRatingList();
    rating->addItems(ratingList);
#endif
    changed = false;
    //enableLineEdits(true);
}

SceneDetailView::~SceneDetailView(){
    //delete ui;
}

void SceneDetailView::buildUI(){
    initializeObjects();
    /// Set up Left-Hand Side of Display Window
    QGridLayout *lhs = new QGridLayout;
    lhs->addWidget(title, 0, 0, 4, 0);

    lhs->addWidget(lbCompany, 1, 0);
    lhs->addWidget(company, 1, 1);
    lhs->addWidget(lbSeries, 1, 2);
    lhs->addWidget(series, 1, 3);
    company->setMinimumWidth(150);
    company->setMaximumWidth(500);
    series->setMinimumWidth(150);
    series->setMaximumWidth(500);

    lhs->addWidget(lbReleased, 2, 0);
    lhs->addWidget(released, 2, 1, 2, 4);

    lhs->addWidget(lbAdded, 3, 0);
    lhs->addWidget(added, 3, 1, 3, 4);

    lhs->addWidget(lbOpened, 4, 0);
    lhs->addWidget(opened, 4, 1, 4, 4);

    lhs->addWidget(lbTags, 5, 0);
    lhs->addWidget(tags, 5, 1, -1, -1);

    /// Set up Filename line
    QHBoxLayout *filenameLayout = new QHBoxLayout;
    filenameLayout->addWidget(lbFilename);
    filenameLayout->addWidget(filepath);
    /// Set up RHS of Window
    QGridLayout *rhs = new QGridLayout;

    /// Add Buttons
    rhs->addWidget(playButton, 0, 1);
    rhs->addWidget(saveButton, 0, 2);
    rhs->addWidget(reparseButton, 0, 3);
    rhs->addWidget(hideButton, 0, 4);
    rhs->addWidget(edit, 1, 4);
    rhs->addWidget(addActor, 3, 4);

    // Add Labels
    rhs->addWidget(lbDuration, 1, 0);
    rhs->addWidget(lbRating, 1, 2);
    rhs->addWidget(lbSize, 2, 0);
    rhs->addWidget(lbResolution, 2, 2);
    rhs->addWidget(lbCast, 3, 0);
    rhs->addWidget(lbAge1, 3, 2);
    rhs->addWidget(lbAge2, 3, 2);
    rhs->addWidget(lbAge1, 3, 2);
    rhs->addWidget(lbAge4, 3, 2);
    // Add Fields
    rhs->addWidget(duration, 1, 1);
    rhs->addWidget(rating, 1, 3);
    rhs->addWidget(size, 2, 1);
    rhs->addWidget(resolution, 2, 3);
    rhs->addWidget(actor1, 3, 1);
    rhs->addWidget(age1,   3, 3);
    rhs->addWidget(actor2, 4, 1);
    rhs->addWidget(age2,   4, 3);
    rhs->addWidget(actor3, 5, 1);
    rhs->addWidget(age3,   5, 3);
    rhs->addWidget(actor4, 6, 1);
    rhs->addWidget(age4,   6, 3);
    released->setMinimumWidth(350);
    released->setMaximumWidth(500);
    opened->setMinimumWidth(350);
    opened->setMaximumWidth(500);
    added->setMinimumWidth(350);
    added->setMaximumWidth(500);
    tags->setMaximumHeight(80);
    duration->setMinimumWidth(150);
    duration->setMaximumWidth(400);
    rating->setMinimumWidth(150);
    rating->setMaximumWidth(400);
    size->setMinimumWidth(150);
    size->setMaximumWidth(400);
    resolution->setMinimumWidth(150);
    resolution->setMaximumWidth(400);
    /// Do overall layout
    QHBoxLayout *upperLayout = new QHBoxLayout;
    upperLayout->addItem(lhs);
    upperLayout->addItem(rhs);
    QVBoxLayout *fullLayout = new QVBoxLayout;
    fullLayout->addItem(upperLayout);
    fullLayout->addItem(filenameLayout);
    this->setLayout(fullLayout);
}

void SceneDetailView::initializeObjects(){
    /// Static Labels
    lbCompany   = new QLabel("Company:");
    lbSeries    = new QLabel("Series: ");
    lbReleased  = new QLabel("Released:");
    lbAdded     = new QLabel("Added:");
    lbOpened    = new QLabel("Opened:");
    lbTags      = new QLabel("Tags:");
    lbFilename  = new QLabel("Filename:");
    lbDuration  = new QLabel("Duration:");
    lbRating    = new QLabel("Rating:");
    lbSize      = new QLabel("Size:");
    lbResolution= new QLabel("Resolution:");
    lbCast  = new QLabel("Cast:");
    labels << lbCast << lbSeries << lbReleased << lbAdded << lbOpened << lbTags << lbFilename << lbDuration << lbRating << lbSize << lbResolution << lbCast;

    /// Buttons
    addActor    = new QToolButton();
    edit        = new QToolButton();
    hideButton        = new QPushButton("Hide");
    playButton        = new QPushButton("Play");
    saveButton        = new QPushButton("Save");
    reparseButton     = new QPushButton("Reparse");
    buttons << hideButton << playButton << saveButton << reparseButton;
    foreach(QPushButton *b, buttons){
        b->setMaximumWidth(150);
    }
    addActor->setIcon(QPixmap(":/Icons/add_icon.png"));
    reparseButton->setIcon(QPixmap(":/Icons/film_folder_icon.png"));
    edit->setIcon(QPixmap(":/Icons/edit_pencil.png"));
    playButton->setIcon(QPixmap(":/Icons/shiny_blue_play_icon.png"));
    saveButton->setIcon(QPixmap(":/Icons/flat_save_icon.png"));
    hideButton->setIcon(QPixmap(":/Icons/flat_hide_icon.png"));

    /// Display Fields
    title       = new QLineEdit();
    company     = new QLineEdit();
    added       = new QLineEdit();
    released    = new QLineEdit();
    opened      = new QLineEdit();
    filepath    = new QLineEdit();
    duration    = new QLineEdit();
    size        = new QLineEdit();
    resolution  = new QLineEdit();
    series      = new QLineEdit();
    tags        = new QTextEdit();
    fields << title << company << added << released << opened << filepath << duration << size << resolution << series;
    foreach(QLineEdit *curr, fields){
        curr->setMaximumWidth(500);
        curr->setPlaceholderText("Unknown");
    }
    duration->setPlaceholderText("0:00:00");

    rating      = new QComboBox();
    actor1  = new QLabel("");
    actor2  = new QLabel("");
    actor3  = new QLabel("");
    actor4  = new QLabel("");
    this->castList << actor1 << actor2 << actor3 << actor4;

    age1    = new QLabel("");
    age2    = new QLabel("");
    age3    = new QLabel("");
    age4    = new QLabel("");
    this->ageList << age1 << age2 << age3 << age4;

    lbAge1  = new QLabel("Age");
    lbAge2  = new QLabel("Age");
    lbAge3  = new QLabel("Age");
    lbAge4  = new QLabel("Age");
    this->ageLabelList << lbAge1 << lbAge2 << lbAge3 << lbAge4;
}

void SceneDetailView::loadScene(ScenePtr s){
    if (!current.isNull() && changed){
        QMessageBox b(QMessageBox::Question, tr("Unsaved Changes"), tr("Save changes before leaving this page?"),QMessageBox::Save | QMessageBox::No, this, Qt::WindowStaysOnTopHint);
        if (b.exec() == QMessageBox::Save){
            emit saveChanges(current);
        }
    }
    this->current = ScenePtr(s.data());
#ifdef DESIGNER
    ui->titleLineEdit->setText(s->getTitle());
    ui->companyLineEdit->setText(s->getCompany());
    ui->seriesLineEdit->setText(s->getSeries());
    QTime duration = s->getLength();
    ui->durationLineEdit->setText(duration.toString("h:mm:ss"));
    QDate released = s->getReleased();
    QDate added = s->getAdded();
    QDate opened = s->getOpened();
    ui->releasedLineEdit->setText(released.toString("MMMM d, yyyy"));
    ui->addedLineEdit->setText(added.toString("MMMM d, yyyy"));
    ui->openedLineEdit->setText(opened.toString("MMMM d, yyyy"));
    ui->filenameLineEdit->setText(s->getFullpath());

    ui->tagsTextEdit->setPlainText(s->tagString());
    QString rating = s->getRating().grade();
    int index = ui->ratingComboBox->findData(rating);
    if (index != -1){
        ui->ratingComboBox->setCurrentIndex(index);
    }
    ui->sizeLineEdit->setText(QString("%1 MB").arg(s->getSize()/BYTES_PER_MEGABYTE));
    int resolution = s->getHeight();
    if (resolution > 0){
        ui->resolutionLineEdit->setText(QString::number(resolution));
    }
    QStringList cast = s->getActors();
    for (int i = 0; i < cast.size(); ++i){
        QString name = cast.at(i);
        ageList.at(i)->clear();
        if (!name.isEmpty()){
            castList.at(i)->setText(QString("<a href=\"%1\"><font style=\"color:white\">%1</font></a>").arg(cast.at(i)));
            if (!released.isNull() && released.isValid()){
                emit requestActorBirthday(cast.at(i));
            }
        } else {
            castList.at(i)->clear();
            ageLabelList.at(i)->clear();
        }
    }
#else
    title->setText(s->getTitle());
    company->setText(s->getCompany());
    series->setText(s->getSeries());
    QTime t = s->getLength();
    duration->setText(t.toString("h:mm:ss"));
    released->setText(s->getReleased().toString("MMMM d, yyyy"));
    opened->setText(s->getOpened().toString("MMMM d, yyyy"));
    added->setText(s->getAdded().toString("MMMM d, yyyy"));
    filepath->setText(s->getFullpath());
    tags->setPlainText(s->tagString());
    QString currRating = s->getRating().grade();
    if (!currRating.isEmpty()){
        int ratingIdx = rating->findData(s->getRating().grade());
        if (ratingIdx != -1){
            rating->setCurrentIndex(ratingIdx);
        }
    }
    size->setText(QString("%1 MB").arg(s->getSize()/BYTES_PER_MEGABYTE));
    int height = s->getHeight();
    if (height > 0){
        resolution->setText(QString::number(height));
    }
#endif
    QStringList cast = s->getActors();
    for(int i = 0; i < 4; ++i){
        QString name = cast.at(i);
        ageList.at(i)->clear();
        if (!name.isEmpty()){
            castList.at(i)->setText(QString("<a href=\"%1\"><font style=\"color:white\">%1</font></a>").arg(cast.at(i)));
            QDate d = s->getReleased();
            if (d.isValid() && !d.isNull()){
                emit requestActorBirthday(name);
            }
        } else {
            castList.at(i)->clear();
            ageLabelList.at(i)->clear();
        }
    }
   //enableLineEdits(true);
    changed = false;
   this->show();
}

void SceneDetailView::on_pb_reparse_clicked()
{
    this->rescanScene();
}

void SceneDetailView::rescanScene(){
    if (!current.isNull()){
        QPair<QString,QString> sceneFile = current->getFile();
        qDebug("Reparsing Scene...");
        SceneParser p(sceneFile);
        p.parse();
        this->current = ScenePtr(new Scene(p));
        this->loadScene(current);
        qDebug("Scene Parsed!");
    } else {
        qWarning("Unable to re-parse Null Scene");
    }
}

void SceneDetailView::playCurrentVideo(){
    emit playVideo(current->getFullpath());
}

void SceneDetailView::addActorToScene(){
    QString text = QInputDialog::getText(this, tr("Enter Actor Name"), tr("Add an Actor"));
    if (!text.isEmpty()){
        foreach(QLabel *nameLabel, this->castList){
            if (nameLabel->text() == text){
                break;  // Don't add the same name twice.
            } else if (nameLabel->text().isEmpty()){
                current->addActor(text);
                nameLabel->setText(QString("<a href=\"%1\"><font style=\"color:white\">%1</font></a>").arg(text));
#ifdef DESIGNER
                ui->pb_save->setEnabled(true);
#else
                saveButton->setEnabled(true);
#endif
                changed = true;
            }
        }
    }
}

void SceneDetailView::receiveActorBirthday(QString name, QDate date){
    bool found = false;
    for(int i = 0; i < 4 && !found; ++i){
        if (castList.at(i)->text().contains(name, Qt::CaseInsensitive)){
            found = true;
            //int age = (date.daysTo(QDate::fromString(ui->releasedLineEdit->text(), "MMMM d, yyyy"))/365);
            int age = (date.daysTo(QDate::fromString(released->text(), "MMMM d, yyyy"))/365);
            ageList.at(i)->setText(QString("Age: %1").arg(age));
        }
    }
}

void SceneDetailView::clearDisplay(){
    for(int i = 0; i < 4; ++i){
        castList.at(i)->clear();
        ageList.at(i)->clear();
    }
#ifdef DESIGNER
    ui->titleLineEdit->clear();
    ui->durationLineEdit->clear();
    ui->releasedLineEdit->clear();
    ui->resolutionLineEdit->clear();
    ui->sizeLineEdit->clear();
    ui->tagsTextEdit->clear();
    ui->seriesLineEdit->clear();
    ui->ratingComboBox->setCurrentIndex(-1);
#else
    foreach(QLineEdit *l, fields){
        l->clear();
    }
    rating->setCurrentIndex(-1);
    tags->clear();
#endif
    changed = false;
}

void SceneDetailView::actorLinkClicked(QString name){
    if (!name.isEmpty()){
        emit showActor(name);
    }
}

void SceneDetailView::enableLineEdits(bool readOnly){
    foreach(QLineEdit *line, fields){
        line->setReadOnly(readOnly);
    }
    tags->setReadOnly(readOnly);
    edit->setEnabled(readOnly);
//    ui->tagsTextEdit->setReadOnly(readOnly);
//    ui->tb_edit->setEnabled(readOnly);
}

void SceneDetailView::on_pb_save_clicked(){
    if (!current.isNull()){
        //QString text = ui->titleLineEdit->text();
        QString text = title->text();
        qDebug("Saving Title: '%s'", qPrintable(text));
        if (valid(text) && text != current->getTitle()){
            current->setTitle(text);
        }
        //text = ui->seriesLineEdit->text();
        text = series->text();
        qDebug("Saving Series: '%s'", qPrintable(text));
        if (valid(text) && text != current->getSeries()){
            current->setSeries(text);
        }
        //text = ui->durationLineEdit->text();
        text = duration->text();
        qDebug("Saving Duration: '%s'", qPrintable(text));
        if (valid(text) && text != current->getLength().toString("h::mm:ss")){
            current->setLength(QTime::fromString(text, "h:mm:ss"));
        }
        //text = ui->releasedLineEdit->text();
        text = released->text();
        qDebug("Saving Release Date: '%s'", qPrintable(text));
        if (valid(text) && text != current->getReleased().toString("MMMM d, yyyy")){
            current->setReleased(QDate::fromString(text, "MMMM d, yyyy"));
        }
        //text = ui->tagsTextEdit->toPlainText();
        text = tags->toPlainText();
        qDebug("Saving Tags: '%s'", qPrintable(text));
        if (valid(text) && text != current->tagString()){
            current->setTags(text.split(',', QString::SkipEmptyParts));
        }
        if (rating->currentIndex() != -1 && !rating->currentText().isEmpty()){
            text = rating->currentText();
            qDebug("Saving Rating: '%s'", qPrintable(text));

            this->current->setRating(rating->currentText());
        }
        /*
        if (ui->ratingComboBox->currentIndex() != -1 && !ui->ratingComboBox->currentText().isEmpty()){
            this->current->setRating(ui->ratingComboBox->currentText());
        }
        */
        qDebug("Saving Changes");
        emit saveChanges(current);
        changed = false;
        //enableLineEdits(true);
    } else {
        qWarning("Error Setting current scene details");
    }
}

