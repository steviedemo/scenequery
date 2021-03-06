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
    void loadScene(const int);
    void setDataContainers(QSharedPointer<DataManager> vault) { this->vault = vault;    }
    void updateDetailView(const int id) {   if (!isHidden()){ loadScene(id);    }   }
    void updateDetailView(ScenePtr s)   {   if (!isHidden()){ loadScene(s);     }   }
    void hideDetailView()               {   clearDisplay(); hide(); }
    void showDetailView(const int id)   {   loadScene(id);  show(); }
    void showDetailView(ScenePtr s)     {   loadScene(s);   show(); }
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
    void renameFile(ScenePtr);
    void requestActorBirthday(QString name);
    void playVideo(int sceneID);
    void updateCurrentItem();
    void updateItem(int);
};

#endif // SCENEDETAILVIEW_H
