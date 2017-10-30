#ifndef HEIGHT_H
#define HEIGHT_H

#include <QString>

class Height {
private:
    int feet;
    int inches;
    int cm;
public:
    Height(void){   feet = 0; inches = 0; cm = 0;   }
    Height(int cm);
    Height(int feet, int inches);
    Height(double feet);
    static Height fromText(QString s);
    ~Height();
    double  getFeetAndInches(void);
    double  getFeet(void)    {   return feet;   }
    int     getInches(void)  {   return inches; }
    int     getCm(void)      {   return cm;     }
    QString toString(void);
};


#endif // HEIGHT_H
