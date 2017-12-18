#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "definitions.h"
#include "FileScanner.h"
#include "DatabaseThread.h"
#include "Actor.h"
#include "Scene.h"
#include <QMainWindow>
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
    // Receivers
    void receiveScenes(SceneList);
    void receiveActors(ActorList);
    void initProgress(int count);
    void updateProgress(int value);
    void finishProgress();
    // Buttons
    void on_actionScan_Directory_triggered();
    void on_refreshScenes_clicked();

    void on_refreshActors_clicked();
    void on_radioButtonActors_clicked();
    void updateStatus(QString);

private:
    Ui::MainWindow *ui;
    SceneList sceneList;
    ActorList actorList;
    FileScanner *scanner;
    DatabaseThread *database;
    bool databaseThreadActive, fileScanThreadActive;
    Display currentDisplay;
    void updateDisplayType();

};

#endif // MAINWINDOW_H
