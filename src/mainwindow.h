#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "definitions.h"
#include "SceneList.h"
#include "curlTool.h"
#include "FileScanner.h"
#include "Actor.h"
#include "Scene.h"
#include "sql.h"
#include "profiledialog.h"
#include "InitializationThread.h"
#include <QSortFilterProxyModel>
#include "SceneProxyModel.h"
#include "SceneView.h"
#include <QMap>
#include <QMainWindow>
#include <QListView>
#include <QProgressDialog>
#include <QTableView>
#include <QStandardItem>
#include <QShowEvent>
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
    void initializationFinished(ActorList, SceneList);
    void receiveScenes(SceneList);
    void receiveActors(ActorList);
    void receiveScanResult(SceneList, QStringList);
    void receiveSingleActor(ActorPtr);

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
    void receiveTestBio(ActorPtr);
    void on_actionUpdate_Bios_triggered();
    void on_actionRefresh_Display_triggered();
    void selectNewProfilePhoto();
    void testProfileDialogClosed();
private:
    void setupThreads();
    void setupViews();
    void refreshSceneView();
    ActorPtr getSelectedActor();
    void setResetAndSaveButtons(bool enabled);
    void displaySceneSubset(SceneList);
    void displayAllScenes(void);
    void addSceneRow(ItemList);
    void addActorRow(ItemList);
    /// View
    Ui::MainWindow *ui;
    ActorList actorList, updateList, displayActors;
    ActorPtr currentActor, updatedActor;
    ProfileDialog *testProfileDialog;
    QMap<QString, ActorPtr> actorMap;
    QModelIndex currentActorIndex;
    QProgressDialog *progressDialog;
    QSharedPointer<QStandardItemModel> sceneSubsetModel, actorSubsetModel;
    QStandardItemModel *actorModel, *sceneModel;
    QStandardItem *actorParent, *sceneParent;
    QStringList names, actorHeaders, sceneHeaders;
    QSortFilterProxyModel *actorProxyModel;
    SceneProxyModel *sceneProxyModel;
    SceneList sceneList, displayScenes;
    SceneView *sceneView;
    /// Threads
    curlTool *curlTestThread;
    InitializationThread *initThread;
    FileScanner *scanner;
    curlTool    *curlThread;
    SQL         *sqlThread;
    bool itemSelected;
    Display currentDisplay;
signals:
    void closing();
    void stopThreads();
    void updateSingleBio(ActorPtr);
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
    void actorSelectionChanged(QString);
    void loadActorProfile(ActorPtr a);

    void startProgressBar(QString, int);
    void updateProgressBar(int);
    void closeProgressBar(QString);
    void newProgressDialogBox(QString, int);
    void updateProgressDialogBox(int);
    void updateProgressDialogBox(QString);
    void closeProgressDialogBox();
    void updateStatusLabel(QString);
};

#endif // MAINWINDOW_H
