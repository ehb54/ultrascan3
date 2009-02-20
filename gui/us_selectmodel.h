//! \file us_selectmodel.h

#ifndef US_SELECTMODEL_H
#define US_SELECTMODEL_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_help.h"

class US_EXTERN US_SelectModel : public US_WidgetsDialog
{
	Q_OBJECT
	
	public:
		US_SelectModel( int&, bool = false );
		
   private:
      int&         modelSelected;
      bool         show;
      QListWidget* lw_models;
      QStringList  models;
      US_Help      showhelp;

		void select_model( int  );

	private slots:
		void select_model( void );
		void help        ( void );
		void cancel      ( void );
};

#endif

