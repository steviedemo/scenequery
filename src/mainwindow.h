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

    void on_assignProfilePhoto_clicked();

private:
    void setupThreads();
    void setupViews();
    void refreshSceneView();
    void sortActors();
    ActorPtr getSelectedActor();
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
