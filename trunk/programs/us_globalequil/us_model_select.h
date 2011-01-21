#ifndef US_MODELSELECT_H
#define US_MODELSELECT_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_editor.h"
#include "us_model.h"

class US_EXTERN US_ModelSelect : public US_WidgetsDialog
{
	Q_OBJECT
	
	public:
		US_ModelSelect( int&, QStringList&, QList< double >& );

	private:
      int&               smodelx;     // Index of selected model
      QStringList&       models;      // List of all possible models
      QList< double >&   aud_pars;    // Additional user-defined params

      QListWidget*       lw_models;   // Models list widget

      QString            model_sel;   // Model string selected

      US_Help            showHelp;
             
   private slots:
      void selected          ( void );
      void cancelled         ( void );
      void model_pars_0      ( void );
      void model_pars_1      ( void );
      void model_pars_2      ( void );
      void model_pars_4      ( void );
      void function_dialog   ( void );
      QString function_equation  ( QStringList& );
      QString function_components( QStringList&, int& );

      void help              ( void )
      { showHelp.show_help("global_equil-modsel.html"); };
};
#endif

