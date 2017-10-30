#ifndef __RATING_H__
#define __RATING_H__
#include <QString>

class Rating{
public:
    Rating();
    Rating(double);
    Rating(QString);
    // operators
    Rating& operator =          (const Rating &r);
    Rating operator ==   (Rating a, Rating b);
    Rating operator <    (Rating a, Rating b);
    Rating operator >    (Rating a, Rating b);
    Rating operator !=   (Rating a, Rating b);

    double toStars();
    QString toString();
    void fromStars(double d);
    void fromString(QString s);
private:
    QString double2string(double d);
    double  string2double(QString s);
    QString ratingStr;
    double stars;
};

};
#endif
