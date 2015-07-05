#ifndef US_MODELMETRICS_H
#define US_MODELMETRICS_H

#include <QApplication>
#include <QDomDocument>
#include <QtGui>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_editor.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_investigator.h"
#include "us_math2.h"
#include "us_util.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_model.h"
#include "us_model_loader.h"
#include "us_select_runs.h"


class US_ModelMetrics : public US_Widgets
{
	Q_OBJECT

	public:
      US_ModelMetrics();

	private:

      int                dbg_level;
      int                mc_iters;
      US_Help            showHelp;
      US_Editor*         te;
      US_Model*          model;      
      QPushButton*       pb_load_model;
      QPushButton*       pb_prefilter;
      QStringList        pfilts;
      QString            mfilter;
      QString            run_name;
      QString            method;
      QString            analysis_name;
      QString            editGUID;
      bool               monte_carlo;
      QLineEdit*         le_model;
      QLineEdit*         le_investigator;
      QLineEdit*         le_prefilter;

      US_Disk_DB_Controls* disk_controls; //!< Radiobuttons for disk/db choice

private slots:
	void sel_investigator     ( void );
	void update_disk_db       ( bool );
	void load_model           ( void );
	void select_prefilter     ( void );
	void reset                ( void );
	void write                ( void );

	void help                 ( void )
        { showHelp.show_help( "manual/us_modelmetrics.html" ); };
};
#endif

