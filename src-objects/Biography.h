#ifndef BIOGRAPHY_H
#define BIOGRAPHY_H
#include "Height.h"
#include <QString>
#include <QDate>
#include <QRegularExpression>
#include <pqxx/result.hxx>
class Biography{
public:
    bool fakeBoobs, retired;
    class QDate birthdate, careerStart, careerEnd;
    int size();
    Biography();
    Biography(QString name);
    Biography(const Biography &b);
    ~Biography();
    bool has(QString key);
    bool operator < (Biography &other) const;
    bool operator > (Biography &other) const;
    bool operator ==(Biography &other) const;
    bool operator >=(Biography &other) const;
    bool operator <=(Biography &other) const;
    void copy(const Biography &other);
    /** Setters/Getters */
    void    setAliases      (const QString a);
    QString getAliases      (void) const;
    void    setCity (QString c);
    QString getCity(void) const;
    void    setNationality(QString s);
    QString getNationality(void) const;
    void    setEthnicity(QString s);
    QString getEthnicity(void) const;
    void    setEyeColor(QString s);
    QString getEyeColor(void) const;
    void    setHairColor(QString s);
    QString getHairColor(void) const;
    void    setMeasurements(QString s);
    QString getMeasurements(void) const;
    void    setName(QString s);
    QString getName(void) const;
    void    setTattoos(QString s);
    QString getTattoos(void) const;
    void    setPiercings(QString s);
    QString getPiercings(void) const;
    Height  getHeight(void) const;
    void    setWeight(int w);
    int     getWeight(void) const;
    void    setBirthday(QDate);
    QDate   getBirthday(void) const;


    void setHeight(Height h);
    void setHeight(int cm);
    void setHeight(int feet, int inches);
    void setHeight(double feet);
private:
    QString name, aliases, city, nationality, ethnicity, eyes, hair, measurements, tattoos, piercings;
    Height height;
    int weight;
    QRegularExpression illegalChars;
    bool validateString(const QString &s);

};


#endif // BIOGRAPHY_H
