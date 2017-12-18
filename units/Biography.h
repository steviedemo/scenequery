#ifndef BIOGRAPHY_H
#define BIOGRAPHY_H
#include "Height.h"
#include <QString>
#include <QDate>

class Biography{
public:
    QString name, aliases, city, nationality, ethnicity, eyes, hair, measurements, tattoos, piercings;
    class Height height;
    int weight;
    bool fakeBoobs, retired;
    class QDate birthdate, careerStart, careerEnd;
    bool update();
    Biography(QString name="");
    Biography(const Biography &b){
        this->name          = b.name;
        this->aliases       = b.aliases;
        this->city          = b.city;
        this->nationality   = b.nationality;
        this->ethnicity     = b.ethnicity;
        this->eyes          = b.eyes;
        this->hair          = b.hair;
        this->measurements  = b.measurements;
        this->tattoos       = b.tattoos;
        this->piercings     = b.piercings;
        this->weight        = b.weight;
        this->birthdate     = b.birthdate;
        this->height        = b.height;
        this->retired       = b.retired;
        this->fakeBoobs     = b.fakeBoobs;
        this->careerEnd     = b.careerEnd;
        this->careerStart   = b.careerStart;
    }
    ~Biography();
    void setHeight(Height h);
    void setHeight(int cm);
    void setHeight(int feet, int inches);
    void setHeight(double feet);

};


#endif // BIOGRAPHY_H
