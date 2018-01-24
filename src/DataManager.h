#ifndef DATAMANAGER_H
#define DATAMANAGER_H
#include <QDate>
#include <QObject>
#include <QMap>
#include "definitions.h"
#include "SceneList.h"
#include "Actor.h"
#include "Scene.h"
class DataManager : public QObject
{
    Q_OBJECT
public:
    explicit DataManager(QObject *parent=0);
    ~DataManager();
    bool        contains(const int ID)          const    {  return sceneMap.contains(ID);      }
    bool        contains(const QString &name)   const    {  return actorMap.contains(name);    }
    ActorPtr    getActor(const QString)         const;
    ScenePtr    getScene(const int id)          const;
    bool        add(const ScenePtr s,     bool saveToDB=true);
    bool        add(const ActorPtr a,     bool saveToDB=true);
    void        add(const SceneList list, bool saveToDB=true);
    void        add(const ActorList list, bool saveToDB=true);
    void        remove(const ScenePtr s);
    void        remove(const ActorPtr a);
    void        remove(const QString &name);
    void        remove(const int id);
    bool        actorMapEmpty(void) const { return actorMap.isEmpty();  }
    bool        sceneMapEmpty(void) const { return sceneMap.isEmpty();  }
    QDate       getBirthday(const QString &name) const;
    int         getAge(const QString &name, const QDate &date) const;
public slots:
    void saveAllScenes();
    void saveAllActors();
    void updateActorDisplayItems();
    void updateSceneDisplayItems();
    void updateBios(void);
private:
    QHash<int, ScenePtr> sceneMap;
    QHash<QString, ActorPtr> actorMap;
    SceneList sceneUpdateList;
    ActorList actorUpdateList;
signals:
    void save(ActorPtr);
    void save(ScenePtr);
    void save(ActorList);
    void save(SceneList);
    void progressBegin(QString, int);
    void progressUpdate(int);
    void progressEnd(QString);
    void updateBiosFromWeb(ActorList);
};

#endif // DATAMANAGER_H
