#ifndef HEIGHT_H
#define HEIGHT_H

#include <QString>

class Height {
private:
    int feet;
    int inches;
    int cm;
    bool valid;
    bool isEqual(Height other);
    bool isGreater(Height other);
public:
    Height(void);
    Height(int cm);
    Height(int feet, int inches);
    Height(double feet);
    Height(int feet, int inches, int cm);
    Height(const Height &other);
    Height operator = (Height other);
    bool operator ==(Height other);
    bool operator <(Height other);
    bool operator !=(Height other);
    bool operator <=(Height other);
    bool operator >(Height other);
    bool operator >=(Height other);
    bool isValid();
    bool nonZero();
    static Height fromText(QString s);
    ~Height();
    double  getFeetDouble(void)         {   return (double)((inches/12.0)+feet);   }
    int     getInches(void) const             {   return inches; }
    int     getCm(void)     const             {   return cm;     }
    int     getFeet(void)   const             {   return feet;   }
    QString toString(void)  const;
    QString sqlSafe(void)   const;
};


#endif // HEIGHT_H
