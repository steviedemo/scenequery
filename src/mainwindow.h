#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "definitions.h"
#include "curlTool.h"
#include "FileScanner.h"
#include "Actor.h"
#include "Scene.h"
#include "sql.h"
#include "ActorTableModel.h"
#include <QSortFilterProxyModel>
#include <QMap>
#include <QMainWindow>
#include <QListView>
#include <QTableView>
#include <QStandardItem>
#define NAME_COLUMN 1
enum Display { DISPLAY_SCENES, DISPLAY_ACTORS, DISPLAY_PHOTOS };
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    /// Receivers
    void receiveScenes(SceneList);
    void receiveActors(ActorList);
    void receiveScanResult(SceneList, QStringList);

    /// Progress & Status Updates
    void startProgress(QString, int);
    void updateProgress(int value);
    void closeProgress(QString);
    void updateStatus(QString);
    void showError(QString);
    void showSuccess(QString);

    /// Window Events
    void showEvent(QShowEvent *event);

    /// Buttons
    void on_actionScan_Directory_triggered();
    void on_refreshScenes_clicked();
    void on_refreshActors_clicked();
    void on_radioButtonActors_clicked();
    void on_actorView_clicked(const QModelIndex &index);
    void on_saveScenes_clicked();
    void on_saveActors_clicked();
    void on_scanFiles_clicked();
    void on_updateActorBios_clicked();
    void on_updateDisplay_clicked();
    void on_closeProfile_clicked();
    void on_profile_photo_customContextMenuRequested(const QPoint &pos);
    void on_saveProfile_clicked();
    void on_reloadProfile_clicked();
    // Text Changed on profile.
    void on_birthDateDateEdit_userDateChanged(const QDate &date);
    void on_hairColorLineEdit_textChanged(const QString &arg1);
    void on_ethnicityLineEdit_textChanged(const QString &arg1);
    void on_nationalityLineEdit_textEdited(const QString &arg1);
    void on_heightLineEdit_textEdited(const QString &arg1);
    void on_weightLineEdit_textEdited(const QString &arg1);
    void on_eyeColorLineEdit_textEdited(const QString &arg1);
    void on_measurementsLineEdit_textEdited(const QString &arg1);
    void on_aliasesEdit_textChanged();
    void on_piercingsEdit_textChanged();
    void on_tattoosEdit_textChanged();

    void on_actionParse_Scene_triggered();

private:
    void selectNewProfilePhoto();
    void setupThreads();
    void setupViews();
    void refreshSceneView();
    void sortActors();
    ActorPtr getSelectedActor();
    void setResetAndSaveButtons(bool enabled);

    void loadActorProfile(ActorPtr);
    /// View
    Ui::MainWindow *ui;
    QModelIndex currentActorIndex;
    ActorPtr currentActor;
    SceneList sceneList;
    ActorList actorList;
    QMap<QString, ActorPtr> actorMap;
    QStandardItemModel *actorModel, *sceneModel;
    QStandardItem *actorParent, *sceneParent;
    QPixmap *blankImage;
    QSortFilterProxyModel *proxyModel;
    QStringList names;

    /// Threads
    FileScanner *scanner;
    curlTool    *curlThread;
    SQL         *sqlThread;
    bool itemSelected;
    Display currentDisplay;
    void updateDisplayType();
signals:
    void closing();
    void stopThreads();

    void updateBios(ActorList);
    void getHeadshots(ActorList);
    void saveActorChanges(ActorPtr);
    void scanFolder(QString);
    void scanActors(SceneList, ActorList);
    void makeNewActors(QStringList);
    void loadActors(ActorList);
    void loadScenes(SceneList);
    void saveActors(ActorList);
    void saveScenes(SceneList);
    void saveChangesToDB(ScenePtr);
};

#endif // MAINWINDOW_H
