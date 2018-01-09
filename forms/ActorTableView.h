#ifndef ACTORTABLEVIEW_H
#define ACTORTABLEVIEW_H
#include <QtWidgets>
#include <QWidget>
#include "ActorProxyModel.h"
#include "definitions.h"
class ActorTableView : public QWidget
{
public:
    ActorTableView(QWidget *parent =0);
    void setSourceModel(QAbstractItemModel *model);
    void addActor(ActorPtr, const QModelIndex &parent = QModelIndex());
    void setHorizontalHeaders(QStringList);
public slots:
    void resizeView();
    void filterChanged(QString);
    void filterChangedHair(QString);
    void filterChangedEthnicity(QString);
    void filterChangedSceneCount(ActorProxyModel::NumberFilterType, int);
private:
    QWidget *parent;
    QStringList headers;
    ActorProxyModel *proxyModel;
    QTableView *table;
};

#endif // ACTORTABLEVIEW_H
