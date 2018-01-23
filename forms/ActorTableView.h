#ifndef ACTORTABLEVIEW_H
#define ACTORTABLEVIEW_H
#include <QtWidgets>
#include <QWidget>
#include "ActorProxyModel.h"
#include "definitions.h"
#include <QFrame>
#include <QAbstractItemView>
#include <QItemSelectionModel>
class ActorTableView : public QWidget
{
    Q_OBJECT
public:
    ActorTableView(QWidget *parent =0);
    void setSourceModel(QAbstractItemModel *model);
    void addActor(ActorPtr, const QModelIndex &parent = QModelIndex());
    void setHorizontalHeaders(QStringList);
    int countRows();
    QStringList namesDisplayed();
    QString selectedName() const;
    void resizeToContents();
    QModelIndex currentIndex();
    QModelIndex findActorIndex(const QString &name) const;
    QModelIndex findActorIndex_Exact(const QString &name) const;
    QModelIndex findActorIndex_base(const QRegExp &name, const int column) const;
public slots:
    void resizeView();
    void filterChanged(QString);
    void filterChangedName(const QString &name);
    void filterChangedHair(QString);
    void filterChangedEthnicity(QString);
    void filterChangedSceneCount(ActorProxyModel::NumberFilterType, int);
    void selectActor(const QString &name);
    void removeActor(QString name);
    void removeActor(ActorPtr);
private:
    QWidget *parent;
    QStringList headers;
    QString ethnicityFilter, nameFilter, hairFilter, currentSelection;
    ActorProxyModel *proxyModel;
    QModelIndex currentIdx, prevIdx;
    QTableView *table;
    QVBoxLayout *mainLayout;
    QItemSelectionModel *selectionModel;
    bool itemClicked;
private slots:
    void selectionChanged(QModelIndex, QModelIndex);
    void rowCountChanged(QModelIndex, int, int);
    void rowClicked(QModelIndex);
signals:
    void displayChanged(int);
    void actorClicked(QString);
    void actorSelectionChanged(QString name);
};

#endif // ACTORTABLEVIEW_H
