#ifndef US_CSV_H
#define US_CSV_H

#include <qstring.h>
#include <qstringlist.h>
#include <qregexp.h>

using namespace std;

class US_Csv
{
 public:

   static QStringList parse_line( QString qs );
};

#endif
