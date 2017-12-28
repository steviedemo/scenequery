#ifndef ACTORTABLEMODEL_H
#define ACTORTABLEMODEL_H
#include <QAbstractTableModel>
#include <QObject>
#include "Actor.h"
#include "definitions.h"
class ActorTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    ActorTableModel(QObject *parent);
    void set(ActorList list);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount (const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
private:
    ActorList list;
};

#endif // ACTORTABLEMODEL_H
