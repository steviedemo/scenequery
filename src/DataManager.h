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
    void        add     (const SceneList,   bool saveToDatabase=false);
    void        add     (const ActorList,   bool saveToDatabase=false);
    bool        add     (const ScenePtr,    bool saveToDatabase=false);
    bool        add     (const ActorPtr,    bool saveToDatabase=false);
    bool        update  (const ActorPtr,    bool saveToDatabase=false);
    bool        update  (const ScenePtr,    bool saveToDatabase=false);
    void        update  (const SceneList,   bool saveToDatabase=false);
    void        update  (const ActorList,   bool saveToDatabase=false);
    void        remove  (const ScenePtr s);
    void        remove  (const ActorPtr a);
    void        remove  (const QString &name);
    void        remove  (const int id);
    bool        actorMapEmpty(void) const { return actorMap.isEmpty();  }
    bool        sceneMapEmpty(void) const { return sceneMap.isEmpty();  }
    QDate       getBirthday(const QString &name);
    int         getAge(const QString &name, const QDate &date);
public slots:
    void    saveAllScenes();
    void    saveAllActors();
    void    updateActorDisplayItems();
    void    updateSceneDisplayItems();
    void    updateBios(void);
    void    setMap(ActorMap actors);
    void    setMap(SceneMap scenes);
private:
    SceneMap sceneMap;
    ActorMap actorMap;
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
    void statusUpdate(QString);
    void updateBiosFromWeb(ActorList);
};

#endif // DATAMANAGER_H
