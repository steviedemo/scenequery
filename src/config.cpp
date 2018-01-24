#include "config.h"
#include "definitions.h"
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QHostInfo>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QToolButton>
#include <QPushButton>
#include <QFileDialog>
#include <QLabel>
SearchPathDialog::SearchPathDialog(QWidget *parent):
    QDialog(parent){
    this->paths = settings.getList(KEY_SEARCH_PATHS);
    QGridLayout *layout = new QGridLayout();
    int row = 0;
    foreach(QString path, paths){
        QLabel *label = new QLabel(path);
        layout->addWidget(label, row, 0);
    }
    QLabel *emptyLabel = new QLabel("");
    this->addButton = new QToolButton();
    addButton->setIcon(QIcon(":/Icons/add_icon.png"));
    addButton->setIconSize(QSize(30, 30));
    layout->addWidget(emptyLabel, row, 0);
    layout->addWidget(addButton, row, 1);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *save = new QPushButton("Save");
    QPushButton *cancel = new QPushButton("Cancel");
    QSpacerItem *spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    buttonLayout->addWidget(spacer);
    buttonLayout->addWidget(cancel);
    buttonLayout->addWidget(save);
    mainLayout->addLayout(layout);
    mainLayout->addLayout(buttonLayout);
    connect(save, SIGNAL(clicked()), this, SLOT(save()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(cancel()));
    connect(addButton, SIGNAL(clicked()), this, SLOT(showFileDialog()));
}

void SearchPathDialog::showFileDialog(){
    this->fileDialog = new QFileDialog(this, QString("Select Path to use"));
    connect(fileDialog, SIGNAL(fileSelected(QString)), SLOT(addItem(QString)));
    connect(fileDialog, SIGNAL(accepted()), this, SLOT(fileDialogClosed()));
    connect(fileDialog, SIGNAL(rejected()), this, SLOT(fileDialogClosed()));
    this->fileDialog->show();
}
void SearchPathDialog::addItem(QString path){
    fileDialog->hide();
    QFileInfo file(path);
    if (!file.isDir()){
        qWarning("Can't add a non-directory to the scanner list");
    } else if (!file.exists()){
        qWarning("'%s' doesn't exist", qPrintable(path));
    } else if (paths.contains(path)){
        qDebug("'%s' is already in the scan list", qPrintable(path));
    } else {
        this->paths << path;
        qDebug("Successfully added '%s' to the list of search paths!", qPrintable(path));
    }
}
void SearchPathDialog::save(){
    settings.setList(KEY_SEARCH_PATHS, paths);
    settings.save();
    this->close();
}
void SearchPathDialog::cancel(){
    this->close();
}
void SearchPathDialog::fileDialogClosed(){
    if (fileDialog){
        fileDialog->hide();
        fileDialog->deleteLater();
    }
}

SearchPathDialog::~SearchPathDialog(){
    delete addButton;
}

Settings::Settings(){
    read(findSettingsFile());
}
Settings::Settings(QString file){
    read(file);
}
bool Settings::read(QString file){
    this->path = file;
    if (!QFileInfo(path).exists()){
        QFile file(path);
        if (!file.open(QFile::Text | QFile::WriteOnly)){
            qWarning("Error Creating Settings file in '%s'", qPrintable(path));
        } else {
            QTextStream out(&file);
            out << " " << end;
        }
    } else {
        QFile file(path);
        if (!file.open(QFile::Text | QFile::ReadOnly)){
            qWarning("Error Opening Settings File '%s'!", qPrintable(path));
        } else {
            int linesRead = 0, pairsSaved = 0;
            QTextStream in(&file);
            QRegularExpression rx("^\\s*(.+)?\\s*=\\s*(.+)?\\s*");
            QRegularExpressionMatch match;
            while(!in.atEnd()){
                QString line = in.readLine();
                if (line.contains('=')){
                    match = rx.match(line);
                    if (match.hasMatch() && match.lastCapturedIndex() > 1){
                        configMap.insert(match.captured(1).trimmed(), match.captured(2).trimmed());
                        pairsSaved++;
                    }
                }
                linesRead++;
            }
            qDebug("Opened Settings file, read %d lines, got %d Key-Value pairs", linesRead, pairsSaved);
        }
    }
}

QString Settings::get(const QString &key) const{
    QString value("");
    if (configMap.contains(key)){
        value = configMap.value(key);
    } else {
        qWarning("the key '%s' does not exist in the settings file", qPrintable(key));
    }
    return value;
}

QStringList Settings::getList(const QString &key) const{
    QStringList values("");
    if (configMap.contains(key)){
        QString value = configMap.value(key);
        values = value.split(',', QString::SkipEmptyParts);
        foreach(QString s, values){
            s = s.trimmed();
        }

    } else {
        qWarning("the key '%s' does not exist in the settings file", qPrintable(key));
    }
    return values;
}

void Settings::set(const QString &key, const QString &value){
    if (!key.isEmpty()){
        configMap.insert(key, value);
    } else {
        qDebug("Can't insert an empty key into the list!");
    }
}

void Settings::setList(const QString &key, const QStringList &values){
    QString value("");
    QTextStream out(&value);
    foreach(QString s, values){
        if (!value.isEmpty()){
            out << ",";
        }
        out << s;
    }
    set(key, value);
}

bool Settings::save() const{
    bool success = false;
    qDebug("Saving Settings...");
    QFile file(path);
    if (file.open(QFile::Text | QFile::WriteOnly | QFile::Truncate)){
        QTextStream out(&file);
        QMapIterator<QString, QString> it(configMap);
        while(it.hasNext()){
            it.next();
            out << it.key() << "\t=\t" << it.value() << endl;
        }
        success = true;
        qDebug("Settings saved");
    } else {
        qWarning("Error Opening '%s' for writing", qPrintable(path));
    }
    return success;
}

QString findSettingsFile(){
    return QString("%1/%2/%2.txt").arg(findDataLocation()).arg(SETTINGS_FOLDER);
}

bool makeDirectories(){
    bool error = false;
    QString dataPath = findDataLocation();
    QString headshotPath = findHeadshotLocation();
    QString thumbnailPath = findThumbnailLocation();
    QString settingsPath = QString("%1/%2").arg(dataPath).arg(SETTINGS_FOLDER);
    if (!QDir(dataPath).exists()){
        qDebug("Creating Data directory...");
        if (!QDir::home().mkdir(DATA_FOLDER)){
            qWarning("Error Creating %s", qPrintable(dataPath));
            error = true;
        } else {
            qDebug("Successfully Created %s", qPrintable(dataPath));
        }
    }
    if (!error){
        QDir dataFolder(dataPath);
        if (!QDir(headshotPath).exists()){
            qDebug("Creating Headshot Folder...");
            if (!dataFolder.mkdir(HEADSHOT_FOLDER)){
                qWarning("Error Creating %s", qPrintable(headshotPath));
                error = true;
            } else {
                qDebug("Successfully Created %s", qPrintable(headshotPath));
            }
        }
        if (!QDir(thumbnailPath).exists()){
            qDebug("Creating Thumbnail Path...");
            if (!dataFolder.mkdir(THUMBNAIL_FOLDER)){
                qWarning("Error Creating %s", qPrintable(thumbnailPath));
                error = true;
            } else {
                qDebug("Successfully Created %s", qPrintable(thumbnailPath));
            }
        }
        if (!QDir(settingsPath).exists()){
            qDebug("Creating Thumbnail Path...");
            if (!dataFolder.mkdir(SETTINGS_FOLDER)){
                qWarning("Error Creating %s", qPrintable(settingsPath));
                error = true;
            } else {
                qDebug("Successfully Created %s", qPrintable(settingsPath));
            }
        }
    }
    return !error;
}

QString findHeadshotLocation(){
    return QString("%1/%2/%3").arg(QDir::homePath()).arg(DATA_FOLDER).arg(HEADSHOT_FOLDER);
}

QString findThumbnailLocation(){
    return QString("%1/%2/%3").arg(QDir::homePath()).arg(DATA_FOLDER).arg(THUMBNAIL_FOLDER);
}

QString findDataLocation(){
    return QString("%1/%2").arg(QDir::homePath()).arg(DATA_FOLDER);
}
