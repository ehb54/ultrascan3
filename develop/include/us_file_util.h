#ifndef US_FILE_UTIL_H
#define US_FILE_UTIL_H

#include "us_extern.h"

#include <qstring.h>
#include <qstringlist.h>
#include <stdio.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>

class US_EXTERN US_File_Util
{
   public:
      ~US_File_Util(){};
      US_File_Util(){};

      bool copy( QString     from, QString to,    bool overwrite = false );
      bool copy( QStringList from, QString toDir, bool overwrite = false );
      bool move( QString     from, QString to,    bool overwrite = false );
      bool move( QStringList from, QString toDir, bool overwrite = false );

      QString errormsg;
};

#endif
