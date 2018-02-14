#ifndef __SCENE_MODEL_H__
#define __SCENE_MODEL_H__
#include <QAbstractTableModel>
#include <QObject>
#include "Scene.h"
#include <QHash>
#include "definitions.h"
using namespace std;
class SceneModel : public QAbstractTableModel{
public:
    SceneModel(QObject *parent=0);
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    void addScene(const ScenePtr);
    void setScenes(const QHash<int,ScenePtr> &hash);
private:
    int idAt(const int row) const;
    QHash<int, ScenePtr> sceneMap;
};
#endif
