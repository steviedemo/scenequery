#include "Height.h"
#include "QRegularExpression"
#define CM_PER_INCH 2.54
#define INCH_PER_CM 0.3937
Height::Height():
    feet(0), inches(0), cm(0), valid(false){
}
Height::Height(double f):
    feet(f), valid(true){
    this->inches = (int)((f - feet) * 12);
    this->cm = (int)(f * 30.48);
}
Height::Height(int cm):
    cm(cm), valid(true){
    int totalInches = cm * INCH_PER_CM;
    this->feet = totalInches/12;
    this->inches=totalInches%12;
}
Height::Height(int feet, int inches):
    feet(feet), inches(inches), valid(true){
    this->cm = (feet*12 + inches)*CM_PER_INCH;
}
Height::Height(int feet, int inches, int cm):
    feet(feet), inches(inches), cm(cm), valid(true){
}
Height::Height(const Height &other){
    this->cm = other.cm;
    this->feet = other.feet;
    this->inches = other.inches;
    this->valid = other.valid;
}
Height::~Height(){}
Height Height::operator =(Height other){
    Height h(other.getFeet(), other.getInches(), other.getCm());
    return h;
}
bool Height::isEqual(Height other){
    return ((this->feet == other.feet) && (this->inches == other.inches));
}
bool Height::isGreater(Height other){
    return (this->getFeetDouble() > other.getFeetDouble());
}
bool Height::nonZero(){
    bool zero = false;
    if (this->feet == 0 && this->inches == 0){
        zero = true;
    }
    return zero;
}

bool Height::operator ==(Height other){ return this->isEqual(other);    }
bool Height::operator !=(Height other){ return !(this->isEqual(other)); }
bool Height::operator > (Height other){ return isGreater(other);        }
bool Height::operator >=(Height other){ return (isGreater(other) || isEqual(other));    }
bool Height::operator < (Height other){ return (!isEqual(other) && !isGreater(other));  }
bool Height::operator <=(Height other){ return (isEqual(other) || !isGreater(other));   }
Height Height::fromText(QString s){
    QRegularExpression rx("([0-9])\\s*feet[\\s,]*([0-9])*(inches)?");
    QRegularExpressionMatch m = rx.match(s);
    int ft = 0, in = 0;
    if (m.hasMatch()){
        QStringList matches = m.capturedTexts();
        int matchCount = matches.size();
        ft = matches.at(0).toInt();
        if (matchCount > 1)
            in = matches.at(1).toInt();
    }
    return Height(ft, in);
}

bool Height::isValid(){
    if (feet == 0 && inches == 0)
        return false;
    else
        return true;
}

QString Height::toString(void) const {
    return QString("%1'%2\"").arg(feet).arg(inches);
}
QString Height::sqlSafe() const{
    return QString("'%1'").arg(cm);
}
