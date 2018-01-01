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
private slots:
    void actorFilterChanged(QString name);

private:
    void addData(int column, QString data);
    QWidget *parent;
    QStringList headers;
    SceneProxyModel *proxyModel;
    QTableView *proxyView;
    QVBoxLayout *mainLayout;
    int newRow;

};

#endif // SCENEVIEW_H
