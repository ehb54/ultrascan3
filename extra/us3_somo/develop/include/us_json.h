#ifndef US_JSON_H
#define US_JSON_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <qstring.h>
#include <qregexp.h>
#include <map>

using namespace std;

class US_Json
{
 public:
   static map < QString, QString > split( QString );
   static QString compose( map < QString, QString > & );
};

#endif
