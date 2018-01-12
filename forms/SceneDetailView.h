#ifndef SCENEDETAILVIEW_H
#define SCENEDETAILVIEW_H
#include <QLabel>
#include <QWidget>
#include <QLineEdit>
#include <QList>
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
    void clearLinks();
    void display(ScenePtr);
    void clearDisplay(void);
    void receiveActorBirthday(QString, QDate);
private slots:
    void loadScene(ScenePtr);
    void actorLinkClicked(QString);
    void actorLinkHovered(QString);
    void on_tb_hide_clicked();

    void on_tb_save_clicked();

    void on_tb_edit_clicked();

private:
    void enableLineEdits(bool readOnly);

    ScenePtr current;
    Ui::SceneDetailView *ui;
    QList<QLabel *> castList, ageList;
    QList<QLineEdit *>dataFields;
signals:
    void showActor(QString);
    void saveChanges(ScenePtr);
    void requestActorBirthday(QString name);
};

#endif // SCENEDETAILVIEW_H
