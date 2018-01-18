#ifndef SCENEPARSER_H
#define SCENEPARSER_H
#include <QByteArray>
#include <QDate>
#include <QDateTime>
#include <QVector>
#include <QString>
#include <QStringList>
#include <QMediaPlayer>
#include <QPair>
#include "Rating.h"
#include "definitions.h"
#define FEAT            "feat."
#define NAME_SEPERATOR  " - "
#define SPACE_REGEX     "[\\s\\t]*"
QByteArray checksum(const QString &absolutePath);
class SceneParser
{
private:
    QPair<QString,QString> file;
    bool parsed;
    QString extension;
    QStringList tags, actors;
    QString title, company, series;
    int height, width, size, sceneNumber;
    QTime length;
    Rating rating;
    QDate release;
    QDate accessed, created;
    QStringList bracketedData;
    QMediaPlayer *player;
    QStringList availableMetaDataKeys;
    QByteArray  md5sum;
    void        bashScript        (QString absolutePath);
    void        readMetadata      (QString);
    QStringList parseActors       (QString);
    void        parseParentheses  (QString);
    QString     parseCompany      (QString);
    int         parseSceneNumber  (QString);
    QDate       parseDateReleased (QString);
    QString     parseTitle        (QString);
    void doubleCheckNames();


public:
    SceneParser(void);
    SceneParser(QPair<QString,QString> file);
    SceneParser(QString absolutePath);
    ~SceneParser();
    void        parse       (void);
    void        parse       (QPair<QString,QString>);
    void        parse       (QString absolutePath);
    static QString sysCall  (QString);
    bool        isParsed        (void)  const {   return parsed;        }
    bool        isEmpty         (void)  const {   return (file.first.isEmpty() && file.second.isEmpty());  }
    int         getSize         (void)  const {   return size;          }
    int         getWidth        (void)  const {   return width;         }
    int         getHeight       (void)  const {   return height;        }
    int         getSceneNumber  (void)  const {   return sceneNumber;   }
    QTime       getLength       (void)  const {   return length;        }
    QString     getTitle        (void)  const {   return title;         }
    QString     getCompany      (void)  const {   return company;       }
    QString     getSeries       (void)  const {   return series;        }
    Rating      getRating       (void)  const {   return rating;        }
    QStringList getActors       (void)  const {   return actors;        }
    QStringList getTags         (void)  const {   return tags;          }
    QDate       getAccessed     (void)  const {   return accessed;      }
    QDate       getAdded        (void)  const {   return created;       }
    QDate       getReleased     (void)  const {   return release;       }
    QString     getFilename     (void)  const {   return file.second;   }
    QString     getFilepath     (void)  const {   return file.first;    }
    QPair<QString,QString>   getFile()  const {   return file;          }

};

#endif // SCENEPARSER_H
