#ifndef SCENEDETAILVIEW_H
#define SCENEDETAILVIEW_H
#include <QLabel>
#include <QDate>
#include <QWidget>
#include <QComboBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>
#include <QMediaPlayer>
#include <QList>
#include "DataManager.h"
#include <QVBoxLayout>
#include <QVideoWidget>
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
    //void receiveActorBirthday(QString, QDate);
    void clearDisplay(void);
    void loadScene(ScenePtr);
    void setDataContainers(QSharedPointer<DataManager> vault) { this->vault = vault;    }
    void sceneSelectionChanged(ScenePtr);
private slots:
    void rescanScene();
    void addActor(void);
    void actorLinkClicked(QString);
    void on_pb_save_clicked();
private:
    Ui::SceneDetailView *ui;
    void enableLineEdits(bool readOnly);
    bool changed;
    ScenePtr current;
    int currentSceneID;
    QList<QLabel *> castList, ageList, ageLabelList;
    QList<QLineEdit *>dataFields;
    QSharedPointer<DataManager> vault;

signals:
    void showActor(QString);
    void showActor(ActorPtr);
    void saveChanges(ScenePtr);
    void requestActorBirthday(QString name);
    void playVideo(int sceneID);
};

#endif // SCENEDETAILVIEW_H
