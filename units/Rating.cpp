#include "Rating.h"

Rating::Rating(){
    this->ratingStr = "";
    this->stars = 0.0;
}
Rating::Rating(double d){
    fromStars(d);
}
Rating::Rating(QString s){
    fromString(s);
}

void Rating::operator = (const Rating &other){
    this->stars = other.stars;
    this->ratingStr = other.ratingStr;
}
bool Rating::operator ==(const Rating &a, const Rating &b) const{   return (a.stars == b.stars);    }
bool Rating::operator <(const Rating &a, const Rating &b)  const{   return (a.stars < b.stars);     }
bool Rating::operator >(const Rating &a, const Rating &b)  const{   return (a.stars > b.stars);     }
bool Rating::operator >=(const Rating &a, const Rating &b) const{   return (a.stars >= b.stars);    }
bool Rating::operator <=(const Rating &a, const Rating &b) const{   return (a.stars <= b.stars);    }

void Rating::fromString(QString s){
    this->ratingStr = s;
    this->stars = string2double(s);
}

void Rating::fromStars(double d){
    this->stars = d;
    this->ratingStr = double2string(d);
}
double Rating::string2double(QString s){
    double d = 0.0;
    if (s.contains('A'))        {   d = 3.5;    }
    else if (s.contains('B'))   {   d = 2.0;    }
    else if (s.contains('C'))   {   d = 0.5;    }
    foreach (QChar c, s){
        if (c == '-'){
            d += 0.5;
        } else if (c == '+') {
            d += 0.5;
        }
    }
    return d;
}

QString Rating::double2string(double d){
    QString s("");
    if (d >= 5.0)       {   s = "A+++"; }
    else if (d == 4.5)  {   s = "A++";  }
    else if (d == 4.0)  {   s = "A+";   }
    else if (d == 3.5)  {   s = "A";    }
    else if (d == 3.0)  {   s = "A-";   }
    else if (d == 2.5)  {   s = "B+";   }
    else if (d == 2.0)  {   s = "B";    }
    else if (d == 1.5)  {   s = "B-";   }
    else if (d == 1.0)  {   s = "C+";   }
    else if (d == 0.5)  {   s = "C";    }
    return s;
}

