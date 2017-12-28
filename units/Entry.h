#ifndef ENTRY_H
#define ENTRY_H

#include <QObject>
#include <QStandardItem>
#include "query.h"
#include <pqxx/result.hxx>
class Entry : public QStandardItem
{
public:
    Entry();
    Entry(qint64);
    Entry(int id);
    virtual QList<QStandardItem *>  buildQStandardItem() = 0;
    virtual void                    updateQStandardItem() = 0;
    //virtual void                    updateFromDatabase() = 0;
    virtual Query                   toQuery() const = 0;
    virtual void                    fromRecord(pqxx::result::const_iterator &record) = 0;
    virtual int                     entrySize() = 0;
    virtual bool operator ==(Entry &e) const;
    qint64  getID();
    void    setID(qint64);
protected:
    qint64 ID;
    QList<QStandardItem *>row;
private:
signals:

};

#endif // ENTRY_H
