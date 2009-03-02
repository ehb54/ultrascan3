#ifndef US_SELECTMODEL10_H
#define US_SELECTMODEL10_H

#include <QtGui>
#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_help.h"

//! \brief A window for specialiezed settings for Stoichiometry Selection

class US_EXTERN US_SelectModel10 : public US_WidgetsDialog
{
   Q_OBJECT
   
   public:
      //! \param temp_stoich1 - Location for the stoichiometry value
      //! \param  p           - The parent widget
      //! \param  f           - Window flags to be passed to the widget  
      US_SelectModel10( float&, QWidget* = 0, Qt::WindowFlags = 0 );

   private:
      float&     stoich1;
      QLineEdit* le_stoich1;
      US_Help    showhelp;

   private slots:
      void help     ( void ){ showhelp.show_help( "manual/equil2.html" ); };
      void ok       ( void );
      void cancel   ( void );
};
#endif

