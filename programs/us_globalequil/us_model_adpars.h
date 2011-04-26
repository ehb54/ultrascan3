#ifndef US_MODELADPARS_H
#define US_MODELADPARS_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_help.h"

class US_ModelAdPars : public US_WidgetsDialog
{
	Q_OBJECT
	
	public:
		US_ModelAdPars( int, QList< double >& );

	private:
      int                napars;      // Number of additional parameters
      QList< double >&   aud_pars;    // Additional user-defined params

      QLineEdit*         le_stoich1;  // Lineedit: Stoich 1
      QLineEdit*         le_stoich2;  // Lineedit: Stoich 2
      QLineEdit*         le_nmwslot;  // Lineedit: Nbr MW Slots
      QLineEdit*         le_llimmwr;  // Lineedit: Lower Lim MW range
      QLineEdit*         le_ulimmwr;  // Lineedit: Upper Lim MW range
      QLineEdit*         le_cmnvbar;  // Lineedit: Common vbar

      QCheckBox*         ck_cmnvbar;  // CheckBox: Use common vbar

      US_Help            showHelp;
             
   private slots:
      void selected          ( void );
      void cancelled         ( void );

      void help              ( void )
      { showHelp.show_help( "global_equil-modapar.html" ); };
};
#endif

