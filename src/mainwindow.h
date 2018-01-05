#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "definitions.h"
#include "curlTool.h"
#include "FileScanner.h"
#include "SceneList.h"
#include "Actor.h"
#include "Scene.h"
#include "sql.h"
#include "profiledialog.h"
#include "InitializationThread.h"
#include "SceneProxyModel.h"
#include "SceneView.h"
#include "VideoPlayer.h"
#include <QSortFilterProxyModel>
#include <QMap>
#include <QMainWindow>
#include <QListView>
#include <QProgressDialog>
#include <QTableView>
#include <QStandardItem>
#include <QShowEvent>
#include <QShortcut>
#define ACTOR_NAME_COLUMN 1
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
    void initializationFinished(ActorList, SceneList);
    void receiveScenes(SceneList);
    void receiveActors(ActorList);
    void receiveScanResult(SceneList, QStringList);
    void receiveSingleActor(ActorPtr);
    void refreshCurrentActor(void);
    void showAddActorDialog();
    void closeAddActorDialog();
    void pd_to_mw_addActorToDisplay(ActorPtr);
    void db_to_mw_receiveActors(ActorList);
    void db_to_mw_receiveScenes(SceneList);
    /// Progress & Status Updates
    void startProgress(QString, int);
    void updateProgress(int value);
    void closeProgress(QString);
    void updateStatus(QString);
    void showError(QString);
    void showSuccess(QString);
    void newProgressDialog(QString, int);
    void updateProgressDialog(int);
    void updateProgressDialog(QString);
    void closeProgressDialog();

    /// Window Events
    void showEvent(QShowEvent *event);
    void deleteActor(ActorPtr a);

    /// Buttons
    void on_actionScan_Directory_triggered();
    void on_refreshScenes_clicked();
    void on_refreshActors_clicked();
    void on_actorView_clicked(const QModelIndex &index);
    void on_saveScenes_clicked();
    void on_saveActors_clicked();
    void reloadProfile();

    void on_actionParse_Scene_triggered();

    void on_actionSave_Scenes_triggered();
    void on_actionLoad_Actors_triggered();
    void on_actionCreate_Bio_triggered();
    void receiveTestBio             (ActorPtr);
    void on_actionUpdate_Bios_triggered();
    void on_actionRefresh_Display_triggered();
    void selectNewProfilePhoto      (void);
    void testProfileDialogClosed    (void);
    void playVideo                  (QString);
    void videoFinished              (void);
private:
    RunMode runMode;
    void setupThreads       (void);
    void setupViews         (void);
    void refreshSceneView   (void);
    ActorPtr getSelectedActor(void);
    void setResetAndSaveButtons(bool enabled);
    void displaySceneSubset(SceneList);
    void displayAllScenes   (void);
    void addSceneRow        (ItemList);
    void addActorRow        (ItemList);
    void threaded_profile_photo_scaler(ActorPtr);


    /// View
    QIcon appIcon;
    Ui::MainWindow *ui;
    QModelIndex currentActorIndex;
    QMap<QString, ActorPtr> actorMap;
    ActorList actorList, updateList, displayActors;
    ActorPtr currentActor, updatedActor;
    QProgressDialog *progressDialog;
    QStandardItemModel *sceneModel, *actorModel;
    SceneProxyModel *sceneProxyModel;
    QSortFilterProxyModel *actorProxyModel;
    QStandardItem *actorParent, *sceneParent;
    QStringList names, actorHeaders, sceneHeaders;
    SceneList sceneList, displayScenes;
    bool videoOpen;
    /// Threads
    SceneView *sceneView;
    ProfileDialog *testProfileDialog, *addProfileDialog;
    InitializationThread *initThread;
    VideoPlayer *videoPlayer;
    curlTool    *curlTestThread;
    FileScanner *scanner;
    curlTool    *curlThread;
    SQL         *sqlThread;
    bool itemSelected;
    Display currentDisplay;
    int threadedProgressCounter;
    QMutex mx;

signals:
    void closing();
    void stopThreads();
    void startInitialization();

    void scanFolder         (QString);
    void scanActors         (SceneList, ActorList);

    void loadScenes         (SceneList);
    void saveScenes         (SceneList);
    void saveChangesToDB    (ScenePtr);

    void dropActor          (ActorPtr);
    void saveActors         (ActorList);
    void saveActorChanges   (ActorPtr);
    void updateBios         (ActorList);
    void loadActors         (ActorList);
    void loadActorProfile   (ActorPtr);
    void updateSingleBio    (ActorPtr);
    void makeNewActors      (QStringList);
    void getHeadshots       (ActorList);
    void actorSelectionChanged(QString);

    /** Progress Bar & Dialog */
    void startProgressBar   (QString, int);
    void updateProgressBar  (int);
    void closeProgressBar   (QString);
    void newProgressDialogBox(QString, int);
    void updateProgressDialogBox(int);
    void updateProgressDialogBox(QString);
    void closeProgressDialogBox();
    void updateStatusLabel(QString);
};

#endif // MAINWINDOW_H
