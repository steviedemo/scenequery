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
    void buildUI();
    void initializeObjects();
    QLineEdit *title, *company, *added, *released, *opened, *filepath, *duration, *size, *resolution, *series;
    QTextEdit *tags;
    QComboBox *rating;
    QPushButton *hide, play, save, reparse;
    QToolButton *addActor, *edit;
    QLabel *lbCompany, *lbSeries, *lbReleased, *lbAdded, *lbOpened, *lbTags, *lbFilename;
    QLabel *lbDuration, *lbRating, *lbSize, *lbResolution, *lbCast, *lbAge1, *lbAge2, *lbAge3, *lbAge4;
    QLabel *actor1, *actor2, *actor3, *actor4, *age1, *age2, *age3, *age4;


signals:
    void showActor(QString);
    void saveChanges(ScenePtr);
    void requestActorBirthday(QString name);
    void playVideo(QString);
};

#endif // SCENEDETAILVIEW_H
