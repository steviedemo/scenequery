#ifndef DEFINITIONS_H
#define DEFINITIONS_H
#include <QSharedPointer>
#include <QVector>
#include <QMapIterator>
#define IMAGE_WIDTH 250
#define IMAGE_HEIGHT 300
#define DEFAULT_PROFILE_PHOTO ":/Icons/blank_profile_photo_female.png"
#define ACTOR_LIST_PHOTO_HEIGHT 30
#define BYTES_PER_MEGABYTE 1048576
#define BYTES_PER_GIGABYTE 1073741824.0
/// Actor List Headers
#define ACTOR_PHOTO_COLUMN      0
#define ACTOR_NAME_COLUMN       1
#define ACTOR_HAIR_COLUMN       2
#define ACTOR_ETH_COLUMN        3
//#define ACTOR_SCENE_COLUMN      4
//#define ACTOR_BIO_SIZE_COLUMN   5
#define ACTOR_AGE_COLUMN        4
#define ACTOR_HEIGHT_COLUMN     5
#define ACTOR_WEIGHT_COLUMN     6
#define ACTOR_TATTOO_COLUMN     7
#define ACTOR_PIERCING_COLUMN   8

/// Scene List Headers
#define SCENE_NAME_COLUMN       0
#define SCENE_TITLE_COLUMN      1
#define SCENE_COMPANY_COLUMN    2
#define SCENE_QUALITY_COLUMN    3
#define SCENE_FEATURED_COLUMN   4
#define SCENE_SIZE_COLUMN       5
#define SCENE_LENGTH_COLUMN     6
#define SCENE_DATE_COLUMN       7
#define SCENE_RATING_COLUMN     8
#define SCENE_PATH_COLUMN       9
#define SCENE_ID_COLUMN         10
#define SCENE_SERIES_COLUMN     11
#define SCENE_TAG_COLUMN        12

#define BUILD_ACTOR_PROGRESS 0
#define BUILD_SCENE_PROGRESS 1
#define LOAD_ACTOR_PROGRESS  2
#define LOAD_SCENE_PROGRESS  3
#define COUNT_SCENE_PROGRESS 4
#define MINIMUM_BIO_SIZE 11
#define KEY_SEARCH_PATHS    "Search Paths"


enum TriState        { ON, OFF, DONT_CARE };

enum LogicalOperator { LESSER_THAN, LESSER_OR_EQUAL, GREATER_THAN, GREATER_OR_EQUAL, EQUAL, NOT_EQUAL, NOT_SET};
enum RunMode {  Debug, Release };
//#define tr();    qDebug("%s::%s::%d", __FILE__, __FUNCTION__, __LINE__);
typedef QSharedPointer<class Scene>             ScenePtr;
typedef QSharedPointer<class Actor>             ActorPtr;
typedef QSharedPointer<class QStandardItem>     ItemPtr;
typedef QVector<QList<class QStandardItem *>>   RowList;
typedef QList<class QStandardItem *>            Row;
//using SceneList             = QVector<QSharedPointer<class Scene>>;
using ActorList             = QVector<QSharedPointer<class Actor>>;
using ActorMap              = QHash<QString, ActorPtr>;
using SceneMap              = QHash<int, ScenePtr>;
using ActorIterator         = QMapIterator<QString, ActorPtr>;
#endif // DEFINITIONS_H
