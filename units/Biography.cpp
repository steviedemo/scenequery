#include "Biography.h"
Biography::Biography()
{
}
void setHeight      (class Height h)  {   this->height = h;   }
void setHeight      (int cm)    {   this->height = Height(cm);  }
void setHeight      (int f, int i){ this->height = Height(f, i);    }
void setHeight      (double d)  {   this->height = Height(d);   }
