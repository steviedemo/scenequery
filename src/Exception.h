#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__
#include <QException>
#include <QString>
class Exception : public QException{
    public:
    explicit Exception() : QException(), message(""){}
    explicit Exception( const QString text) : QException(), message(text){}

    Exception *clone()  const    {   return new Exception(*this);    }
    void raise()        const    {   throw *this;    }
    const char *what()  const    {   return qPrintable(message);    }
    private:
	QString message;
};
#endif
