#ifndef SCENEDETAILVIEW_H
#define SCENEDETAILVIEW_H

#include <QWidget>

namespace Ui {
class SceneDetailView;
}

class SceneDetailView : public QWidget
{
    Q_OBJECT

public:
    explicit SceneDetailView(QWidget *parent = 0);
    ~SceneDetailView();

private:
    Ui::SceneDetailView *ui;
};

#endif // SCENEDETAILVIEW_H
