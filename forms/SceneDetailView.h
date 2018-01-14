#ifndef SCENEDETAILVIEW_H
#define SCENEDETAILVIEW_H
#include <QLabel>
#include <QDate>
#include <QWidget>
#include <QLineEdit>
#include <QMediaPlayer>
#include <QList>
#include <QVBoxLayout>
#include <QVideoWidget>
#include <QOpenGLWidget>
#include "definitions.h"
namespace Ui {
class SceneDetailView;
}

class SceneDetailView : public QWidget
{
    Q_OBJECT

public:
    explicit SceneDetailView(QWidget *parent = 0);
    ~SceneDetailView();
public slots:
    void receiveActorBirthday(QString, QDate);
    void clearDisplay(void);
    void loadScene(ScenePtr);
private slots:
    void rescanScene();
    void addActor(void);
    void actorLinkClicked(QString);
    void playCurrentVideo();
    void on_pb_save_clicked();


    void on_pb_reparse_clicked();

private:
    void enableLineEdits(bool readOnly);
    bool changed;
    ScenePtr current;
    Ui::SceneDetailView *ui;
    QList<QLabel *> castList, ageList, ageLabelList;
    QList<QLineEdit *>dataFields;

signals:
    void showActor(QString);
    void saveChanges(ScenePtr);
    void requestActorBirthday(QString name);
    void playVideo(QString);
};

#endif // SCENEDETAILVIEW_H
