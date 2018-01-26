#include "Height.h"
#include <QRegularExpression>
#define CM_PER_INCH 2.54
#define INCH_PER_CM 0.3937
Height::Height():
    feet(0), inches(0), cm(0), valid(false){
}
Height::Height(const double f):
    feet(f), valid(true){
    this->inches = (int)((f - feet) * 12);
    this->cm = (int)(f * 30.48);
}
Height::Height(const int cm):
    cm(cm), valid(true){
    int totalInches = cm * INCH_PER_CM;
    this->feet = totalInches/12;
    this->inches=totalInches%12;
}
Height::Height(const int feet, const int inches):
    feet(feet), inches(inches), valid(true){
    this->cm = (feet*12 + inches)*CM_PER_INCH;
}
Height::Height(const int feet, const int inches, const int cm):
    feet(feet), inches(inches), cm(cm), valid(true){
}
Height::Height(const QString &s):
    feet(0), inches(0), cm(0), valid(false){
    QRegularExpression rx;
    if (s.contains("feet")){
        rx.setPattern("([0-9])\\s*feet[\\s,]*([0-9])*(inches)?");
    } else {
        rx.setPattern("([0-9]{1})\\'\\s*([0-9]{1,2})");
    }
    QRegularExpressionMatch m = rx.match(s);
    if (m.hasMatch()){
        bool inchesOk(false), feetOk(false);
        int f_temp = m.captured(1).toInt(&feetOk);
        int i_temp = m.captured(2).toInt(&inchesOk);
        if (feetOk && inchesOk){
            this->feet   = f_temp;
            this->inches = i_temp;
            this->cm     = (feet*12 + inches)*CM_PER_INCH;
        }
    }
}

Height::~Height(){}

void Height::set(const int &feet, const int &inches){
    this->cm = (feet*12 + inches)*CM_PER_INCH;
    this->feet = feet;
    this->inches = inches;
}
void Height::set(const int &cm){
    this->cm = cm;
    int totalInches = cm * INCH_PER_CM;
    this->feet = totalInches/12;
    this->inches=totalInches%12;
}
void Height::set(const Height &h){
    this->cm = h.getCm();
    this->feet = h.getFeet();
    this->inches = h.getInches();
}

void Height::set(const double &d){
    this->feet = (int)d;
    this->inches = (int)((d - feet) * 12);
    this->cm = (int)(d * 30.48);
}

bool Height::isEqual(const Height &other) const {
    return ((this->feet == other.feet) && (this->inches == other.inches));
}
bool Height::isGreater(const Height &other) const {
    double thisHeight = (double)(feet) + (double)(inches/12.0);
    return (thisHeight > other.getFeetDouble());
}

Height Height::fromText(QString s){
    return Height(s);
}

QString Height::toString(void) const {
    return QString("%1'%2\"").arg(feet).arg(inches);
}
QString Height::sqlSafe() const{
    return QString("'%1'").arg(cm);
}
