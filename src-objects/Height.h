#ifndef HEIGHT_H
#define HEIGHT_H

#include <QString>

class Height {
private:
    int feet;
    int inches;
    int cm;
    bool valid;
    bool isEqual    (const Height &other) const;
    bool isGreater  (const Height &other) const;
public:
    Height(void);
    Height(const QString &);
    Height(const int cm);
    Height(const double feet);
    Height(const int feet, const int inches);
    Height(const int feet, const int inches, const int cm);
    ~Height();
    bool    operator ==(const Height &h)  const { return isEqual(h);                      }
    bool    operator !=(const Height &h)  const { return !isEqual(h);                     }
    bool    operator <(const Height &h)   const { return (!isEqual(h) && !isGreater(h));  }
    bool    operator <=(const Height &h)  const { return !isGreater(h);                   }
    bool    operator >(const Height &h)   const { return isGreater(h);                    }
    bool    operator >=(const Height &h)  const { return (isGreater(h) || isEqual(h));    }
    void    set(const double &);
    void    set(const int &cm);
    void    set(const Height &h);
    void    set(const int &feet, const int &inches);
    int     getInches       (void)      const { return this->inches;            }
    int     getFeet         (void)      const { return this->feet;              }
    int     getCm           (void)      const { return this->cm;                }
    bool    nonZero()                   const { return !isEmpty();                              }
    bool    isEmpty()                   const { return (cm == 0 && feet == 0 && inches == 0);   }
    bool    isValid()                   const { return !this->isEmpty();                        }
    double  getFeetDouble   (void)      const { return (double)((inches/12.0)+feet);   }
    static Height fromText(QString s);
    QString toString(void)  const;
    QString sqlSafe(void)   const;
};


#endif // HEIGHT_H
