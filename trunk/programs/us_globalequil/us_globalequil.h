#ifndef US_GLOBEQUIL_H
#define US_GLOBEQUIL_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_editor.h"
#include "us_model.h"
#include "us_simparms.h"
#include "us_astfem_math.h"
#include "us_astfem_rsa.h"

#include "qwt_counter.h"

class US_EXTERN US_GlobalEquil : public US_Widgets
{
	Q_OBJECT
	
	public:
		US_GlobalEquil();

	private:
      US_Model                     model;
      US_SimulationParameters      simparams;

      US_DataIO2::EditedData*      edata;


      int     current_shape;
      int     current_position;
      int     speed_type;
             
      double  rpm_start;
      double  rpm_stop;
      double  sigma_start;
      double  sigma_stop;
      int     speed_count;

      double* sim_radius;
      double* concentration;
      uint    radius_points;

      double  current_time;
      double  step_time;
      double  next_scan_time;

      QList< double > speed_steps;

      US_Help        showHelp;
      QwtPlot*       equilibrium_plot;
      QwtPlotCurve*  current_curve;
      US_Astfem_RSA* astfem_rsa;

      QLabel*       lb_lowspeed;
      QLabel*       lb_highspeed;

      QwtCounter*   ct_scselect;

      QCheckBox*    ck_edlast;
      US_Disk_DB_Controls* dkdb_cntrls;

      QTextEdit*    te_equiscns;

      QLineEdit*    le_prjname;

      QPushButton*  pb_view;
      QPushButton*  pb_unload;
      QPushButton*  pb_scdiags;

      void   init_simparams    ( void );
      void   init_astfem_data  ( void );

   private slots:
      void details           ( void );
      void load              ( void );
      void new_model         ( void );
      void change_model      ( void );
      void load_model        ( void );
      void simulate          ( void );

      void set_time          ( double time )
      { step_time = time; };

      void help              ( void )
      { showHelp.show_help("manual/global_equil.html"); };
};
#endif
