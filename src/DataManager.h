#ifndef DATAMANAGER_H
#define DATAMANAGER_H
#include <QDate>
#include <QObject>
#include <QMap>
#include <QMutex>
#include "definitions.h"
#include "SceneList.h"
#include "Actor.h"
#include "Scene.h"
#include <QException>
class DataManager : public QObject
{
    Q_OBJECT
public:
    explicit DataManager(QObject *parent=0);
    ~DataManager();
    bool        contains(const int ID)          const;
    bool        contains(const QString &name)   const;
    ActorPtr    getActor(const QString);
    ScenePtr    getScene(const int id);
    bool        add(const ScenePtr s, bool saveToDatabase=false);
    bool        add(const ActorPtr a, bool saveToDatabase=false);
    void        add(const SceneList list);
    void        add(const ActorList list);
    bool        update(const ActorPtr, bool saveToDB=true);
    bool        update(const ScenePtr, bool saveToDB=true);
    void        update(const SceneList list, bool saveToDB=true);
    void        update(const ActorList list, bool saveToDB=true);
    void        remove(const ScenePtr s);
    void        remove(const ActorPtr a);
    void        remove(const QString &name);
    void        remove(const int id);
    bool        actorMapEmpty(void) const { return actorMap.isEmpty();  }
    bool        sceneMapEmpty(void) const { return sceneMap.isEmpty();  }
    QDate       getBirthday(const QString &name);
    int         getAge(const QString &name, const QDate &date);
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
    QMutex mx;
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
