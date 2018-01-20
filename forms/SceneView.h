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
    QVector<int> getIDs();
public slots:
    void receiveSceneCountRequest();
    void resizeSceneView();
    void companyFilterChanged(QString);
    void tagFilterChanged(QString);
    void qualityFilterChanged(int);
private slots:
    void actorFilterChanged(QString name);
    void actorFilterChanged(ActorPtr);
    void clearFilter(void);
    void rowDoubleClicked(const QModelIndex &index);
    void selectionChanged(QModelIndex, QModelIndex);
    void sceneClicked(QModelIndex);
    void receiveRequestForShownSceneIDs();
private:
    void addData(int column, QString data);
    QItemSelectionModel *selectionModel;
    QWidget *parent;
    QStringList headers;
    SceneProxyModel *proxyModel;
    QTableView *table;
    QVBoxLayout *mainLayout;
    int newRow;
    int currentFileSelection;
signals:
    void sendSceneCount(int);
    void playFile(int sceneID);
    void sceneSelectionChanged(int sceneID);
    void sceneItemClicked(int sceneID);
    void sendSceneIDs(QVector<int>);
};

#endif // SCENEVIEW_H
