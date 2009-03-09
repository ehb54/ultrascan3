#ifndef US_SELECTMODEL13_H
#define US_SELECTMODEL13_H

#include <QtGui>
#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_help.h"

//! \brief  A window for stoichiometry selection for Monomer - N-mer - M-mer
//!         Equilibrium
class US_EXTERN US_SelectModel13 : public US_WidgetsDialog
{
   Q_OBJECT
   
   public:
      //! \param temp_stoich1  Location for 1st stoichiometry value
      //! \param temp_stoich2  Location for 2nd stoichiometry value
      //! \param p             Parent widget, normally not specified
      //! \param f             Window flags, normally not specified
      US_SelectModel13( double&, double&, QWidget* = 0, Qt::WindowFlags = 0 );

   private:
      double&    stoich1;
      double&    stoich2;

      QLineEdit* le_stoich1;
      QLineEdit* le_stoich2;
      
      US_Help    showhelp;

   private slots:
      void help     ( void ){ showhelp.show_help( "manual/equil2.html" ); };
      void ok       ( void );
      void cancel   ( void );
};
#endif

