#ifndef US_FILE_UTIL_H
#define US_FILE_UTIL_H

#include "us_util.h"
#include "us_extern.h"

#include <qstring.h>
#include <qstringlist.h>
//Added by qt3to4:
#include <QTextStream>
#include <stdio.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qdatetime.h>

#ifdef QT4
typedef QIODevice::OpenModeFlag OpenMode;
#else
typedef int OpenMode;
#endif

class US_EXTERN US_File_Util
{
   public:
      ~US_File_Util(){};
      US_File_Util(){};

      bool copy( QString     from, QString to,    bool overwrite = false );
      bool copy( QStringList from, QString toDir, bool overwrite = false );
      bool move( QString     from, QString to,    bool overwrite = false );
      bool move( QStringList from, QString toDir, bool overwrite = false );
      bool read( QString     from, QStringList &qsl );

      static bool writeuniq( QString & outname, QString & msg, QString basename, QString ext, QString & data );

      QString errormsg;
};

class US_EXTERN US_Log
{
   public:
      US_Log                   ( QString name, bool autoflush = true, OpenMode mode = (OpenMode) 0 ); // mode will be OR'd with IO_WriteOnly
      ~US_Log                  ();

      void          log        ( const QString & );
      void          log        ( const char * );
      void          log        ( const QStringList & );
      void          datetime   ( QString qs = "" );
      void          time       ( QString qs = "" );
      void          flushon    ();
      void          flushoff   ();
      QString       error_msg;
      
   private:
      
      bool          autoflush;
      QFile         f;
      QTextStream * ts;
};

#endif
