#ifndef US_PDB_UTIL_H
#define US_PDB_UTIL_H

#include <qstring.h>
#include <qstringlist.h>
#include <list>
#include <set>
#include <algorithm>

using namespace std;

#ifdef WIN32
# if QT_VERSION < 0x040000
  #pragma warning ( disable: 4251 )
# endif
#endif

class US_Pdb_Util
{
 public:

   static QString qstring_common_head( const QString & s1, 
                                       const QString & s2 );
   static QString qstring_common_tail( const QString & s1, 
                                       const QString & s2 );

   static QString qstring_common_head( const QStringList & qsl, bool strip_digits = false );
   static QString qstring_common_tail( const QStringList & qsl, bool strip_digits = false );

   static QStringList sort_pdbs( const QStringList & filenames );

   static bool range_to_set( set < QString > & result, const QString & s );
};

#ifdef WIN32
# if QT_VERSION < 0x040000
  #pragma warning ( default: 4251 )
# endif
#endif

#endif
