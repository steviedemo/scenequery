#include "Biography.h"
#include "Height.h"
#include "curlTool.h"
#include <QDate>
Biography::Biography(QString name):
    name(name), aliases(""), city(""), nationality(""), ethnicity(""),
    eyes(""), hair(""), measurements(""), tattoos(""), piercings(""),
    weight(0), fakeBoobs(false), retired(false),
    birthdate(QDate()), careerStart(QDate()), careerEnd(QDate()){
}

bool Biography::update(){
    curlTool curl();
    bool success = curl.getFreeonesData(name, this);
    success = success && curl.getIAFDData(name, this);
    return success;
}

void setHeight      (class Height h)    {   this->height = h;               }
void setHeight      (int cm)            {   this->height = Height(cm);      }
void setHeight      (int f, int i)      {   this->height = Height(f, i);    }
void setHeight      (double d)          {   this->height = Height(d);       }
