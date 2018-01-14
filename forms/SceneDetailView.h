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
    void playPreview();
    void addActor(void);
    void actorLinkClicked(QString);
    void on_tb_hide_clicked();
    void on_tb_save_clicked();
    void on_tb_edit_clicked();
    void playCurrentVideo();
private:
    void enableLineEdits(bool readOnly);

    ScenePtr current;
    Ui::SceneDetailView *ui;
    QList<QLabel *> castList, ageList;
    QList<QLineEdit *>dataFields;
    QMediaPlayer *mediaPlayer;
    QVideoWidget *videoWidget;
    QOpenGLWidget *openglWidget;
signals:
    void showActor(QString);
    void saveChanges(ScenePtr);
    void requestActorBirthday(QString name);
    void playVideo(QString);
};

#endif // SCENEDETAILVIEW_H
