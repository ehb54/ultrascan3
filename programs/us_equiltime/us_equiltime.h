#ifndef US_EQUILTIME_H
#define US_EQUILTIME_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_femglobal.h"
#include "us_editor.h"
#include "us_astfem_math.h"
#include "us_astfem_rsa.h"

#include "qwt_counter.h"

class US_EXTERN US_EquilTime : public US_Widgets
{
	Q_OBJECT
	
	public:
		US_EquilTime();

	private:
      struct ModelSystem          model;
      struct SimulationParameters simparams;
      vector< struct mfem_data >  astfem_data;

      enum    { PROLATE, OBLATE, ROD, SPHERE };
      enum    { INNER, OUTER, CENTER, CUSTOM };
      enum    { SIGMA, RPM };
             
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

      QwtCounter*   cnt_top;
      QwtCounter*   cnt_bottom;
      QwtCounter*   cnt_lowspeed;
      QwtCounter*   cnt_highspeed;
      QwtCounter*   cnt_speedsteps;
      QwtCounter*   cnt_tolerance;
      QwtCounter*   cnt_timeIncrement;

      QCheckBox*    cb_monitor;
      QTextEdit*    te_speedlist;

      US_Editor*    te_info;

      QPushButton*  pb_saveExp;
      QPushButton*  pb_changeModel;
      QPushButton*  pb_estimate;

      void init_simparams    ( void );
      void init_astfem_data  ( void );

   private slots:
      void update_speeds     ( int  );
      void new_channel       ( int  );
      void new_lowspeed      ( double );
      void new_highspeed     ( double );
      void new_speedstep     ( double );
      void simulate          ( void );
      void load_experiment   ( void );
      void new_model         ( void );
      void change_model      ( void );
      void load_model        ( void );

      void check_equil       ( vector< double >&, double* );

      void set_time          ( double time )
      { step_time = time; };

      void help              ( void )
      { showHelp.show_help("manual/calc_equiltime.html"); };
};
#endif
