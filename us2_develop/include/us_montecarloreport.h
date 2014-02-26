#ifndef US_MONTECARLOREPORT_H
#define US_MONTECARLOREPORT_H

#include "us.h"
#include "us_util.h"

#include <qobject.h>
#include <qfile.h>
#include <qmessagebox.h>

class US_EXTERN US_Report_MonteCarlo: public QWidget
{
   Q_OBJECT

   public:
   
      US_Report_MonteCarlo( bool,    QWidget* p = 0, const char* name = 0 );
      US_Report_MonteCarlo( QString, QWidget* p = 0, const char* name = 0 );
      ~US_Report_MonteCarlo() {};

      void generate( const QString& );

   private:
      US_Config* USglobal;

      void load( void );
      void view( const QString& );
};
#endif

