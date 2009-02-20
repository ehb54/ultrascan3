#ifndef US_SELECTMODEL3_H
#define US_SELECTMODEL3_H

#include <QtGui>
#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_help.h"


class US_EXTERN US_SelectModel3 : public US_WidgetsDialog
{
   Q_OBJECT
   
   public:
      US_SelectModel3( float&, float&, int&, float&, bool&, int,  
            QWidget* = 0, Qt::WindowFlags = 0 );

      US_SelectModel3( int&, QWidget* = 0, Qt::WindowFlags = 0 );

   private:
      float& mwUpperLimit;
      float& mwLowerLimit;
      int&   mwSlots;
      bool&  model3VbarFlag;
      float& model3Vbar;

      int    InvID;

      QLineEdit* le_mwSlots;
      QLineEdit* le_mwLowerLimit;
      QLineEdit* le_mwUpperLimit;
      QLineEdit* le_vbar;

      QCheckBox* cb_commonVbar;

      US_Help    showhelp;

      //US_Vbar_DB* vbar_dlg;

   private slots:
      void help     ( void ){ showhelp.show_help( "manual/equil2.html" ); };
      void ok       ( void );
      void ok2      ( void );
      void cancel   ( void );
      void read_vbar( void );
};
#endif

