#ifndef US_EQUILTIME_H
#define US_EQUILTIME_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_predict1.h"
#include "us_femglobal.h"
#include "us_editor.h"

#include "qwt_counter.h"

class US_EXTERN US_EquilTime : public US_Widgets
{
	Q_OBJECT
	
	public:
		US_EquilTime();

	private:

      struct hydrosim             simcomp;
      struct ModelSystem          system;
      struct SimulationParameters simparams;
      
      double  mw;
      double  s;
      double  D;
             
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

      QList< double > speed_steps;

      US_Help       showHelp;
      QwtPlot*      equilibrium_plot;

      QRadioButton* rb_prolate;
      QRadioButton* rb_oblate;
      QRadioButton* rb_rod;
      QRadioButton* rb_sphere;

      QLineEdit*    le_mw;
      QLineEdit*    le_sed;
      QLineEdit*    le_diff;

      QLabel*       lb_lowspeed;
      QLabel*       lb_highspeed;

      QwtCounter*   cnt_top;
      QwtCounter*   cnt_bottom;
      QwtCounter*   cnt_tolerance;
      QwtCounter*   cnt_timeIncrement;
      QwtCounter*   cnt_delta_r;
      QwtCounter*   cnt_delta_t;
      QwtCounter*   cnt_lowspeed;
      QwtCounter*   cnt_highspeed;
      QwtCounter*   cnt_speedsteps;

      QCheckBox*    cb_monitor;
      QTextEdit*    te_speedlist;

      US_Editor*    te_info;

      QPushButton*  pb_saveExp;
      QPushButton*  pb_changeModel;
      QPushButton*  pb_estimate;

   private slots:
      void update_speeds     ( int  );
      void new_lowspeed      ( double );
      void new_highspeed     ( double );
      void new_speedstep     ( double );
      void simulate          ( void );
      void load_experiment   ( void );
      void new_model         ( void );
      void change_model      ( void );
      void load_model        ( void );

      void help              ( void )
      { showHelp.show_help("manual/calc_equiltime.html"); };

};
#endif
