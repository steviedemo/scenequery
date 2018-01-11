#ifndef SCENEDETAILVIEW_H
#define SCENEDETAILVIEW_H
#include <QLabel>
#include <QWidget>
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
private slots:
    void actorLinkClicked(QString);
    void actorLinkHovered(QString);
private:
    void generateLinks(QStringList);
    Ui::SceneDetailView *ui;
    QList<QLabel *> castList;
signals:
    void showActor(QString);

};

#endif // SCENEDETAILVIEW_H
