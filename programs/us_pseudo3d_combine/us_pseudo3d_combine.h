#ifndef US_PSE3D_COMBINE_H
#define US_PSE3D_COMBINE_H

#include <QtGui>

#include "qwt_plot_marker.h"

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_editor.h"
#include "us_plot.h"

	struct solute_s
	{
	  int ii;
	  int jj;
	};
	#define Solute struct solute_s

struct distro_sys
{
   QList <Solute> s_distro;
   QList <Solute> mw_distro;
   QVector <QColor> gradient;
   QString run_name;
   QString cell;
   QString wavelength;
   QString method;
   int distro_type;
   bool monte_carlo;
};

class US_EXTERN US_Pseudo3D_Combine : public US_Widgets
{
   Q_OBJECT

   public:
      US_Pseudo3D_Combine();

   private:

      QLabel*       lb_info1;
      QLabel*       lb_resolu;
      QLabel*       lb_xreso;
      QLabel*       lb_yreso;
      QLabel*       lb_xpix;
      QLabel*       lb_ypix;
      QLabel*       lb_autolim;
      QLabel*       lb_plt_smin;
      QLabel*       lb_plt_smax;
      QLabel*       lb_plt_fmin;
      QLabel*       lb_plt_fmax;
      QLabel*       lb_curr_distr;

      QLineEdit*    le_distr_info;

      US_Help       showHelp;
 
      QwtCounter*   ct_resolu;
      QwtCounter*   ct_xreso;
      QwtCounter*   ct_yreso;
      QwtCounter*   ct_xpix;
      QwtCounter*   ct_ypix;
      QwtCounter*   ct_plt_fmin;     
      QwtCounter*   ct_plt_fmax;     
      QwtCounter*   ct_plt_smin;     
      QwtCounter*   ct_plt_smax;     
      QwtCounter*   ct_curr_distr;

      QwtPlot*      data_plot;

      QwtPlotPicker* pick;

      QPushButton*  pb_pltall;
      QPushButton*  pb_stopplt;
      QPushButton*  pb_refresh;
      QPushButton*  pb_reset;
      QPushButton*  pb_lddistr;
      QPushButton*  pb_ldcolor;
      QPushButton*  pb_print;
      QPushButton*  pb_save;
      QPushButton*  pb_help;
      QPushButton*  pb_close;

      QCheckBox*    cb_autolim;
      QCheckBox*    cb_plot_s;
      QCheckBox*    cb_plot_mw;

      QProgressBar* progress;

      QVector< struct distro_sys > system;
      QVector< QColor > gradient;

      double        resolu;
      double        plt_smin;
      double        plt_smax;
      double        plt_fmin;
      double        plt_fmax;
      double        k_range;
      double        s_range;

      int           xreso;
      int           yreso;
      int           xpix;
      int           ypix;
      int           curr_distr;
      int           init_solutes;
      int           mc_iters;
      int           dimens;

      bool          minmax;
      bool          zoom;
      bool          auto_lim;
      bool          plot_s;
      bool          looping;

   private slots:

      void update_plot_smin( double );
      void update_plot_smax( double );
      void update_plot_fmin( double );
      void update_plot_fmax( double );
      void update_resolu( double );
      void update_xreso( double );
      void update_yreso( double );
      void update_xpix( double );
      void update_ypix( double );
      void update_curr_distr( double );
      void plot_3dim( void );
      void plot_data( int );
      void plot_data( void );
      void loop( void );
      void select_autolim( void );
      void select_plot_s( void );
      void select_plot_mw( void );
      void load_distro( void );
      void load_distro( const QString& );
      void load_color( void );
      void save( void );
      void stop( void );
      void print( void );
      void reset( void );
      void set_limits( void );
      void setup_GUI( void );

      void help     ( void )
      { showHelp.show_help( "manual/pseudo3d_combine.html" ); };
};
#endif
