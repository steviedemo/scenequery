#include "Height.h"
#define CM_PER_INCH 2.54
#define INCH_PER_CM 0.3937

Height::Height(double f){
    this->feet = (int)f;
    this->inches = (int)((f - feet) * 12);
    this->cm = (int)(f * 30.48);
}
Height::Height(int cm){
    this->cm = cm;
    int totalInches = cm * INCH_PER_CM;
    this->feet = total/12;
    this->inches=total%12;
}
Height::Height(int feet, int inches){
    this->feet = feet;
    this->inches = inches;
    this->cm = (feet*12 + inches)*CM_PER_INCH;
}
Height fromText(QString s){
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

double Height::getFeetAndInches(){
    return (double)((inches/12.0)) + (double)feet;
}
QString Height::toString(void){
    return QString("%1'%2\"").arg(feet).arg(inches);
}

