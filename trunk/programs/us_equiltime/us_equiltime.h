#ifndef US_EQUILTIME_H
#define US_EQUILTIME_H

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

class US_EXTERN US_EquilTime : public US_Widgets
{
	Q_OBJECT
	
	public:
		US_EquilTime();

	private:
      US_Model                     model;
      US_SimulationParameters      simparams;
      US_DataIO2::RawData          astfem_data;

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
      int     radius_points;

      QVector< double > sim_radius;
      QVector< double > concentration;

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

      QTextEdit*    te_speedlist;
      US_Editor*    te_info;

      QPushButton*  pb_changeModel;
      QPushButton*  pb_estimate;

      void   init_simparams    ( void );
      void   init_astfem_data  ( void );
      double rpmFromSigma      ( double sigma );
      double sigmaFromRpm      ( double rpm   );

   private slots:
      void update_speeds     ( int  );
      void new_channel       ( int  );
      void new_lowspeed      ( double );
      void new_highspeed     ( double );
      void new_speedstep     ( double );
      void simulate          ( void );
      void change_model      ( void );
      void set_model         ( US_Model );

      void check_equil       ( QVector< double >*, double* );

      void set_time          ( double time )
      { step_time = time; };

      void help              ( void )
      { showHelp.show_help("manual/us_equiltime.html"); };
};
#endif
