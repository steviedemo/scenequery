#include "Entry.h"

Entry::Entry(){}
Entry::Entry(qint64 id) {   this->ID = id;  }
Entry::Entry(int id)    {   this->ID = id;  }


qint64 Entry::getID()           {   return ID;      }
void   Entry::setID(qint64 id)  {   this->ID = id;  }
bool Entry::operator ==(Entry &other) const{
    return (this->ID == other.getID());
}
