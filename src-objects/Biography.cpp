#include "Biography.h"
#include "Height.h"
#include "curlTool.h"
#include <QDate>
Biography::Biography():
    fakeBoobs(false), retired(false), birthdate(QDate()),
    careerStart(QDate()), careerEnd(QDate()),
    name(""), aliases(""), city(""), nationality(""), ethnicity(""),
    eyes(""), hair(""), measurements(""), tattoos(""), piercings(""), weight(0){
    this->illegalChars.setPattern(".*[\\<\\>\\\\]");
}
Biography::Biography(QString name):
    fakeBoobs(false), retired(false),
    birthdate(QDate()), careerStart(QDate()), careerEnd(QDate()),
    name(name), aliases(""), city(""), nationality(""), ethnicity(""),
    eyes(""), hair(""), measurements(""), tattoos(""), piercings(""), weight(0){
    this->illegalChars.setPattern(".*[\\<\\>\\\\]");
}
Biography::Biography(const Biography &b){
    copy(b);
}

Biography::~Biography(){}
void Biography::copy(const Biography &other){
    this->aliases       = other.getAliases();
    this->city          = other.getCity();
    this->nationality   = other.getNationality();
    this->ethnicity     = other.getEthnicity();
    this->eyes          = other.getEyeColor();
    this->hair          = other.getHairColor();
    this->measurements  = other.getMeasurements();
    this->tattoos       = other.getTattoos();
    this->piercings     = other.getPiercings();
    this->weight        = other.getWeight();
    this->birthdate     = other.getBirthday();
    this->height        = other.getHeight();
    this->retired       = other.retired;
    this->fakeBoobs     = other.fakeBoobs;
    this->careerEnd     = other.careerEnd;
    this->careerStart   = other.careerStart;
}
QString Biography::getAliases(void) const {   return this->aliases;   }
QDate   Biography::getBirthday() const    {   return this->birthdate; }
QString Biography::getCity() const        {   return this->city;      }
QString Biography::getEthnicity() const   {   return this->ethnicity; }
QString Biography::getEyeColor() const    {   return this->eyes;      }
QString Biography::getHairColor() const   {   return this->hair;      }
Height  Biography::getHeight() const      {   return this->height;    }
QString Biography::getMeasurements() const{   return this->measurements;  }
QString Biography::getName() const        {   return this->name;          }
QString Biography::getNationality() const {   return this->nationality;   }
QString Biography::getPiercings() const   {   return this->piercings;     }
QString Biography::getTattoos() const     {   return this->tattoos;       }
int     Biography::getWeight() const      {   return this->weight;        }

bool Biography::validateString(const QString &s){

}

void Biography::setName(QString s){
    if (!s.contains(illegalChars)){
        this->name = s;
    }
}

void Biography::setBirthday(const QDate &d){
    if (!d.isNull()){ this->birthdate = d; }
}
void Biography::setCity(const QString &c){
    if (!c.contains(this->illegalChars) && !c.contains("No data", Qt::CaseInsensitive) && !c.contains("Unknown", Qt::CaseInsensitive)){
        this->city = c;
    }
}
void Biography::setEthnicity(const QString &s){
    if (!s.contains(illegalChars)  && !s.contains("No data", Qt::CaseInsensitive) && !s.contains("Unknown", Qt::CaseInsensitive)){
        this->ethnicity = s;
    }
}
void Biography::setEyeColor(const QString &s){
    if (!s.contains(illegalChars)  && !s.contains("No data", Qt::CaseInsensitive) && !s.contains("Unknown", Qt::CaseInsensitive)){
        this->eyes = s;
    }
}
void Biography::setHairColor(const QString &s){
    if (!s.contains(illegalChars)  && !s.contains("No data", Qt::CaseInsensitive) && !s.contains("Unknown", Qt::CaseInsensitive)){
        this->hair = s;
    }
}
void Biography::setMeasurements(const QString &s){
    if (!s.contains(illegalChars) && !s.contains("No data", Qt::CaseInsensitive) && !s.contains("Unknown", Qt::CaseInsensitive)){
        this->measurements = s; }
}
void Biography::setNationality(const QString &s){
    if (!s.contains(illegalChars)  && !s.contains("No data", Qt::CaseInsensitive) && !s.contains("Unknown", Qt::CaseInsensitive)){
        this->nationality = s;
    }
}
void Biography::setPiercings(const QString &s){
    if (!s.contains(illegalChars)  && !s.contains("No data", Qt::CaseInsensitive) && !s.contains("Unknown", Qt::CaseInsensitive)){
        this->piercings = s;
    }
}
void Biography::setTattoos(const QString &s){
    if (!s.contains(illegalChars)  && !s.contains("No data", Qt::CaseInsensitive) && !s.contains("Unknown", Qt::CaseInsensitive)){
        this->tattoos = s;
    }
}
void Biography::setWeight(int w){
    if (w > 0){
        this->weight = w;
    }
}
/** \brief Check if this object has the key with the name provided.
 */
bool Biography::has(QString key){
    bool found = false;
    if (key.contains("alias", Qt::CaseInsensitive))
        found = (!this->aliases.isEmpty() && this->aliases.contains("Unknown"));
    else if (key.contains("city", Qt::CaseInsensitive))
        found = (!city.isEmpty() && this->city.contains("Unknown"));
    else if (key.contains("ethnic", Qt::CaseInsensitive))
        found = (!ethnicity.isEmpty() && this->ethnicity.contains("Unknown"));
    else if (key.contains("hair", Qt::CaseInsensitive))
        found = (!hair.isEmpty() && this->hair.contains("Unknown"));
    else if (key.contains("eye", Qt::CaseInsensitive))
        found = (!eyes.isEmpty() && this->eyes.contains("Unknown"));
    else if (key.contains("measure", Qt::CaseInsensitive))
        found = (!measurements.isEmpty() && this->measurements.contains("Unknown"));
    else if (key.contains("nation", Qt::CaseInsensitive))
        found = (!nationality.isEmpty() && this->nationality.contains("Unknown"));
    else if (key.contains("birthda", Qt::CaseInsensitive))
        found = (!birthdate.isNull() && birthdate.isValid());
    else if (key.contains("tattoo", Qt::CaseInsensitive))
        found = (!tattoos.isEmpty() && this->tattoos.contains("Unknown") && this->tattoos != "None");
    else if (key.contains("pierc", Qt::CaseInsensitive) || key.contains("peirc", Qt::CaseInsensitive))
        found = (!piercings.isEmpty() && this->piercings.contains("Unknown") && this->piercings != "None");
    else if (key.contains("height", Qt::CaseInsensitive))
        found = height.isValid() && height.nonZero();
    else if (key.contains("weight", Qt::CaseInsensitive))
        found = (weight > 0);
    else
        qWarning("************* Biography's 'has' function passed unrecognized key name, '%s' **************", qPrintable(key));
    return found;
}
/*
Biography Biography::operator =(Biography &other){
    copy(other);
    return *this;
}
*/
void    Biography::setAliases(QString a){
    if (!a.contains("No known aliases", Qt::CaseInsensitive) && !a.contains("Unknown", Qt::CaseInsensitive) && !a.contains("No Data", Qt::CaseInsensitive) && !a.contains(illegalChars)){
        this->aliases = a;
    }
}

void Biography::setHeight      (Height h)          {   this->height.set(h);     }
void Biography::setHeight      (int cm)            {   this->height.set(cm);    }
void Biography::setHeight      (int f, int i)      {   this->height.set(f, i);  }
void Biography::setHeight      (double d)          {   this->height.set(d);     }
int Biography::size(){
    int size = 0;
    size += (aliases.isEmpty() ? 0 : 1);
    size += (city.isEmpty() ? 0 : 1);
    size += (nationality.isEmpty() ? 0 : 1);
    size += (ethnicity.isEmpty() ? 0 : 1);
    size += (hair.isEmpty() ? 0 : 1);
    size += (eyes.isEmpty() ? 0 : 1);
    size += (measurements.isEmpty() ? 0 : 1);
    size += (tattoos.isEmpty() ? 0 : 1);
    size += (piercings.isEmpty() ? 0 : 1);
    size += ((weight==0) ? 0 : 1);
    size += (birthdate.isNull() ? 1 : 0);
    size += (careerEnd.isNull() ? 1 : 0);
    size += (careerStart.isNull() ? 1 : 0);
    return size;
}
