#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "definitions.h"
#include "config.h"
#include "curlTool.h"
#include "FileScanner.h"
#include "SceneList.h"
#include "DataManager.h"
#include "Actor.h"
#include "MiniThreads.h"
#include "Scene.h"
#include "SQL.h"
#include "profiledialog.h"
#include "ActorProxyModel.h"
#include "SceneDetailView.h"
#include "SceneProxyModel.h"
#include "SceneTableView.h"
#include "SplashScreen.h"
#include "VideoPlayer.h"
#include <QHash>
#include <QItemSelectionModel>
#include <QMap>
#include <QMainWindow>
#include <QListView>
#include <QProgressDialog>
#include <QShortcut>
#include <QShowEvent>
#include <QSortFilterProxyModel>
#include <QStandardItem>
#include <QTableView>
#include <QSettings>

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
    void initDone(ActorList, SceneList, RowList, RowList);
    void receiveScenes(SceneList);
    void receiveActors(ActorList);
    void receiveSingleActor(ActorPtr);

    void searchActors();
    void searchScenes();

    void apv_to_mw_receiveSceneListRequest(QString actorName);
    void pd_to_mw_addActorToDisplay(ActorPtr);
    void db_to_mw_receiveActors(ActorList);
    void db_to_mw_receiveScenes(SceneList);
    void sdv_to_mw_showActor(QString);

    //void sdv_to_mw_requestBirthday(QString);
    void sw_to_mw_selectionChanged(int id);
    void sw_to_mw_itemClicked(int id);
    /// Progress & Status Updates
    void startProgress(QString, int);
    void closeProgress(QString);
    void showError(QString);
    void showSuccess(QString);
    void newProgressDialog(QString, int);
    void updateProgressDialog(int);
    void updateProgressDialog(QString);
    void closeProgressDialog();
    void renameFile(ScenePtr);
    void removeActorItem(ActorPtr);
    void showCurrentActorProfile();

    /// Window Events
    void actorSelectionChanged(QString);
    void actorTableView_clicked(QString);

    /// Buttons

    void scan_directory_chosen(QString);
    void on_tb_clearActorFilters_clicked();


    void receiveTestBio             (ActorPtr);
    void selectNewProfilePhoto      (void);
    void playVideo                  (int sceneID);
    void videoFinished              (void);
    void updateSceneDisplay(ScenePtr);

    void on_actionScan_Directory_triggered();
    void on_actionAdd_Actor_triggered();
    void on_actionParse_Scene_triggered();
    void on_actionCreate_Bio_triggered();
    void on_actionWipe_Scenes_Table_triggered();
    void on_actionWipe_Actor_Table_triggered();
    void on_actionDeleteActor_triggered();
    void on_actionItemDetails_triggered();
    void on_actionScan_All_Folders_triggered();
    void scanPaths(QStringList paths);

    void on_actionAdd_Scan_Folder_triggered();

private:
    RunMode runMode;
    QString newName;
    void startScanner(const QStringList &);
    void resetActorFilterSelectors(void);
    void setupViews         (void);
    void connectViews       (void);
    void startThreads       (void);
    QString getCurrentName (QAbstractItemModel *);
    QModelIndex getCurrentIndex(QAbstractItemModel *);
    ActorPtr getSelectedActor(void);
    /// View
    QIcon appIcon;
    Ui::MainWindow *ui;
    QSharedPointer<DataManager> vault;
    QModelIndex currentActorIndex;
    QMap<QString, ActorPtr> actorMap;
    QHash<int, ScenePtr> sceneMap;
    ActorList actorList, updateList;
    ActorPtr currentActor, updatedActor;
    QFileDialog *fileDialog;
    QProgressDialog *progressDialog;
    QStandardItemModel *sceneModel, *actorModel;
    SceneProxyModel *sceneProxyModel;
    ActorProxyModel *actorProxyModel;
    QItemSelectionModel *actorSelectionModel;
    QStandardItem *actorParent, *sceneParent;
    QStringList names, actorHeaders, sceneHeaders;
    SceneList sceneList, sceneUpdateList;
    bool videoOpen;
    QString prevSearchActor, prevSearchScene;
    RowList rows;
    /// Threads
    FileRenamer *updater;
    SceneDetailView *sceneDetailView;
    ProfileDialog *testProfileDialog, *addProfileDialog;
    SplashScreen *splashScreen;
    VideoPlayer *videoPlayer;
    QThread     *videoThread;
    curlTool    *curlTestObject;
    FileScanner *scanner;
    curlTool    *curl;
    SQL         *sql;
    SearchPathDialog *searchPathDialog;
    QThread     *sqlThread, *curlThread, *curlTestThread;
    bool itemSelected;
    Display currentDisplay;
    int index;
    QMutex mx;
    QSettings *settings;

signals:
    void closing();
    void startInitialization();
    void skipInitialization(ActorList, SceneList);
    void scanFolder         (QString);
    void saveScenes         (SceneList);
    void saveChangesToDB    (ScenePtr);
    void deleteActor        (QString);
    void saveActors         (ActorList);
    void saveActorChanges   (ActorPtr);
    void updateBios         (ActorList);
    void loadActorProfile   (ActorPtr);
    void updateSingleBio    (ActorPtr);
    void startVideoPlayback (void);
    void purgeScenes();
    /// Filtering
    void cb_companyFilterChanged(QString);
    /** Progress Bar & Dialog **/
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
