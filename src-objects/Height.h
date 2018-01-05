#ifndef HEIGHT_H
#define HEIGHT_H

#include <QString>

class Height {
private:
    int feet;
    int inches;
    int cm;
    bool valid;
    bool isEqual(Height &other) const;
    bool isGreater(Height &other) const;
public:
    Height(void);
    Height(int cm);
    Height(int feet, int inches);
    Height(double feet);
    Height(int feet, int inches, int cm);
    Height(const Height &other);
    void set(int feet, int inches);
    void set(double);
    void set(int cm);
    void set(const Height &h);
    bool operator ==(Height &other) const;
    bool operator <(Height &other)  const;
    bool operator !=(Height &other) const;
    bool operator <=(Height &other) const;
    bool operator >(Height &other)  const;
    bool operator >=(Height &other) const;
    bool isValid() const;
    bool nonZero() const;
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
