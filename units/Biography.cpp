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
Biography::~Biography(){}
bool Biography::update(){
    curlTool curl;
    bool success = curl.getFreeonesData(name, this);
    success = success && curl.getIAFDData(name, this);
    return success;
}

void Biography::setHeight      (Height h)          {   this->height = h;               }
void Biography::setHeight      (int cm)            {   this->height = Height(cm);      }
void Biography::setHeight      (int f, int i)      {   this->height = Height(f, i);    }
void Biography::setHeight      (double d)          {   this->height = Height(d);       }
