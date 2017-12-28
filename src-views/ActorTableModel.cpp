#include "ActorTableModel.h"
#include <QFileInfo>
#include "filenames.h"
ActorTableModel::ActorTableModel(QObject *parent)
    :QAbstractTableModel(parent){
}

void ActorTableModel::set(ActorList list){
    this->list = list;
}

int ActorTableModel::rowCount(const QModelIndex & /*parent*/) const{
    return list.size();
}
int ActorTableModel::columnCount(const QModelIndex &/*parent*/) const{
    return 2;
}

QVariant ActorTableModel::data(const QModelIndex &index, int role) const{
    qDebug("%s::%s called", __FILE__, __FUNCTION__);
    QVariant modelData;
    if (index.row() > -1 && index.column() > -1){
        ActorPtr a = list.at(index.row());
        if (role == Qt::DecorationRole && headshotDownloaded(a->getName())){
            QString headshot = getProfilePhoto(a->getName());
            if (QFileInfo(headshot).exists()){
                modelData = QVariant(QPixmap(headshot));
            } else {
                modelData = QVariant();
            }
        } else {
            modelData = QVariant(a->getName());
        }
    } else {
        return QVariant();
    }
    return modelData;
}
