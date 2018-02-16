#include "SceneModel.h"

SceneModel::SceneModel(QObject *parent):
    QAbstractTableModel(parent){
}

int SceneModel::rowCount(const QModelIndex &/*parent*/)     const{  return sceneMap.count();    }
int SceneModel::columnCount(const QModelIndex &/*parent*/)  const{  return sceneMap.count();    }
QVariant SceneModel::data(const QModelIndex &index, int role) const{
    if (!index.isValid()){
        return QVariant();
    } else if (role == Qt::TextAlignmentRole){
        return int(Qt::AlignHCenter | Qt::AlignVCenter);
    } else if (role == Qt::DisplayRole) {
        int id = idAt(index.row());
        if (!sceneMap.contains(id)){
            return QVariant();
        } else {
            QVariant v = QVariant(sceneMap.value(id));
            return v;
        }
    }
    return QVariant();
}
QVariant SceneModel::headerData(int section, Qt::Orientation /*orientation*/, int role) const{
    qDebug("Section %d, Role %d. Placeholder Function", section, role);
    return QVariant();
}

int SceneModel::idAt(const int row) const {
    return (sceneMap.begin() + row).key();
}

void SceneModel::addScene(const ScenePtr s){
    if (!s.isNull()){
        if (!sceneMap.contains(s->getID())){
            sceneMap.insert(s->getID(), s);
        }
    }
}

