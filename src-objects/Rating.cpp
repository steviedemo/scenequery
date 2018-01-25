#include "Rating.h"
#include <QtGui>
#include <QRegularExpression>
#include <math.h>
#define A_VALUE 7
#define B_VALUE 4
#define C_VALUE 1
const int PaintingScaleFactor = 20;
const QRegularExpression regex("[ABCR][\\+\\-]*");
//Rating::Rating(int i) {
//    this->starRating = i;
//    this->gradeRating = StarsToGrade(i);
//}
Rating::Rating(QString s) {
    QRegularExpressionMatch m = regex.match(s);
    if (m.hasMatch()){
        this->gradeRating = s;
    } else {
        if (!s.isEmpty()) {
            qWarning("Error: cannot parse '%s' into rating format", qPrintable(s));
        }
        this->gradeRating = "R";
    }
    this->starRating = GradeToStars(gradeRating);
    setupView();
}
/*
Rating::Rating(const Rating &other){
    this->gradeRating = other.gradeRating;
    this->starRating = other.starRating;
    setupView();
}
*/
/*
Rating Rating::operator =(const Rating &other){
    this->starRating = other.starRating;
    this->gradeRating = other.gradeRating;
    setupView();
    return *this;
}
*/
//Rating::Rating(const std::string s){
//    fromString(QString::fromStdString(s));
//}
void Rating::setupView(){
    this->starPolygon << QPointF(1.0, 0.5);
    for (int i = 0; i < MAX_STAR_RATING; ++i){
        starPolygon << QPointF(0.5 + 0.5*std::cos(0.8*i*3.14), 0.5 + 0.5*std::sin(0.8*i*3.14));
        diamondPolygon << QPointF(0.4,0.5) << QPointF(0.5,0.4) << QPointF(0.6,0.5) << QPointF(0.5,0.6) << QPointF(0.4,0.5);
    }
}
QSize Rating::sizeHint() const {
    return PaintingScaleFactor * QSize(MAX_STAR_RATING, 1);
}

void Rating::paint(QPainter *painter, const QRect &rect, const QPalette &palette, EditMode mode) const{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);
    if (mode == Editable){
        painter->setBrush(palette.highlight());
    } else {
        painter->setBrush(palette.foreground());
    }

    int yOffset = (rect.height() - PaintingScaleFactor)/2;
    painter->translate(rect.x(), rect.y() + yOffset);
    painter->scale(PaintingScaleFactor, PaintingScaleFactor);
    for(int i = MIN_STAR_RATING; i < MAX_STAR_RATING; ++i){
        if (i < starRating){
            painter->drawPolygon(starPolygon, Qt::WindingFill);
        } else if (mode == Editable){
            painter->drawPolygon(diamondPolygon, Qt::WindingFill);
        }
        painter->translate(1.0, 0.0);
    }
    painter->restore();
}

bool Rating::operator ==(Rating other) const{   return (this->starRating == other.stars());    }
bool Rating::operator < (Rating other) const{   return (this->starRating <  other.stars());    }
bool Rating::operator > (Rating other) const{   return (this->starRating >  other.stars());    }
bool Rating::operator >=(Rating other) const{   return (this->starRating >= other.stars());    }
bool Rating::operator <=(Rating other) const{   return (this->starRating <= other.stars());    }
bool Rating::operator !=(Rating other) const{   return (this->starRating != other.stars());    }

bool Rating::isEmpty() const{
    return this->gradeRating.isEmpty();
}

QString Rating::sqlSafe() const{
    return QString("'%1'").arg(this->gradeRating);
}
QString Rating::grade() const{
    return this->gradeRating;
}
int Rating::stars() const{
    return this->starRating;
}
void Rating::fromStars(int i){
    this->starRating = i;
    this->gradeRating = StarsToGrade(i);
    setupView();
}
void Rating::fromString(QString s){
    this->gradeRating = s;
    this->starRating = GradeToStars(s);
    setupView();
}

int Rating::GradeToStars(QString s){
    int d = 0;
    if (s.contains("R"))        {   return 0;       }
    else if (s.contains('A'))   {   d = A_VALUE;    }
    else if (s.contains('B'))   {   d = B_VALUE;    }
    else if (s.contains('C'))   {   d = C_VALUE;    }
    foreach (QChar c, s){
        if (c == '-'){
            d -= 1;
        } else if (c == '+') {
            d += 1;
        }
    }
    return d;
}

QString Rating::StarsToGrade(int i){
    QString s("");
    switch (i){
        case 10:s = "A+++"; break;
        case 9: s = "A++";  break;
        case 8: s = "A+";   break;
        case 7: s = "A";    break;
        case 6: s = "A-";   break;
        case 5: s = "B+";   break;
        case 4: s = "B";    break;
        case 3: s = "B-";   break;
        case 2: s = "C+";   break;
        case 1: s = "C";    break;
        default:s = "R";    break;
    }
    /*
    if (d >= 10)       {   s = "A+++"; }
    else if (d == 9)  {   s = "A++";  }
    else if (d == 8)  {   s = "A+";   }
    else if (d == 3.5)  {   s = "A";    }
    else if (d == 3.0)  {   s = "A-";   }
    else if (d == 2.5)  {   s = "B+";   }
    else if (d == 2.0)  {   s = "B";    }
    else if (d == 1.5)  {   s = "B-";   }
    else if (d == 1.0)  {   s = "C+";   }
    else if (d == 0.5)  {   s = "C";    }
    */
    return s;
}

