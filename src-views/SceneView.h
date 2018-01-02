#ifndef SCENEVIEW_H
#define SCENEVIEW_H
#include <QtWidgets>
#include <QWidget>
#include "SceneProxyModel.h"

#include "definitions.h"
class SceneView : public QWidget
{
    Q_OBJECT
public:
    SceneView(QWidget *parent = 0);
    void setSourceModel(QAbstractItemModel *model);
    QVBoxLayout *getLayout();
    void addScene(ScenePtr, const QModelIndex &parent = QModelIndex());
public slots:
    void receiveSceneCountRequest();

private slots:
    void actorFilterChanged(QString name);
    void actorFilterChanged(ActorPtr);
    void clearFilter(void);
    void rowDoubleClicked(const QModelIndex &index);
private:
    void addData(int column, QString data);
    QWidget *parent;
    QStringList headers;
    SceneProxyModel *proxyModel;
    QTableView *proxyView;
    QVBoxLayout *mainLayout;
    int newRow;
signals:
    void sendSceneCount(int);
    void playFile(QString);
};

#endif // SCENEVIEW_H
