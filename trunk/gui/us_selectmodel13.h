#ifndef US_SELECTMODEL13_H
#define US_SELECTMODEL13_H

#include <QtGui>
#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_help.h"


class US_EXTERN US_SelectModel13 : public US_WidgetsDialog
{
   Q_OBJECT
   
   public:
      US_SelectModel13( float&, float&, QWidget* = 0, Qt::WindowFlags = 0 );

   private:
      float&     stoich1;
      float&     stoich2;

      QLineEdit* le_stoich1;
      QLineEdit* le_stoich2;
      
      US_Help    showhelp;

   private slots:
      void help     ( void ){ showhelp.show_help( "manual/equil2.html" ); };
      void ok       ( void );
      void cancel   ( void );
};
#endif

