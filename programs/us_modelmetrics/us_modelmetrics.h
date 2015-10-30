#ifndef US_MODELMETRICS_H
#define US_MODELMETRICS_H

#include <QApplication>
#include <QDomDocument>
#include <QtGui>

#include "us_spectrodata.h"
#include "us_extern.h"
#include "us_widgets.h"
#include "us_analysis_base2.h"
#include "us_help.h"
#include "us_editor.h"
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
#include <qwt_legend.h>
#include "us_license_t.h"
#include "us_license.h"

class HydroParm
{
	public:
   double parm, conc;

   HydroParm() {};
   ~HydroParm() {};
   bool operator==(const HydroParm& objIn)
   {
      return ( parm == objIn.parm );
   }
   bool operator!=(const HydroParm& objIn)
   {
      return ( parm != objIn.parm );
   }
   bool operator < (const HydroParm& objIn) const
   {
      if ( parm < objIn.parm ) 
	      return (true);
      else
	      return (false);
   }
};

struct reportItem
{
   QString investigator, runID, triple, analysis, iterations, edit, parameter;
   QString sigma, d[3], x[3], span, minimum, maximum, mean, mode;
   QString median, skew, kurtosis, span_label, filename, integral, totalc, name, csv;
   QPixmap pixmap;
};

//! \brief Less-than function for sorting S_Solute values
bool distro_lessthan( const S_Solute&, const S_Solute& );

class US_ModelMetrics : public US_Widgets 
{
	Q_OBJECT

	public:
      US_ModelMetrics();
      enum hydro_parms { HPs, HPd, HPm, HPk, HPf, HPv, HPr };


	private:

      reportItem         report_entry;
      QString            report, xtitle;
      QTextStream        report_ts;
      QFile              report_file;
      int                dbg_level;
      int                mc_iters;
      int                fixed;
      int                calc_val;
      int                model_count;
      bool               monte_carlo;
      bool               saved;
		bool					 xautoscale;
		double 				 plotxmin;
		double 				 plotxmax;
      double*            xx;
      double*            yy;
      double             sigma;
      double             tc;
      double             cmin;
      double             cmax;
      double             smin;
      double             smax;
      double             kmin;
      double             kmax;
      double             wmin;
      double             wmax;
      double             vmin;
      double             vmax;
      double             dmin;
      double             dmax;
      double             fmin;
      double             fmax;
      double             xmin;
      double             xmax;
      double             rmin;
      double             rmax;
      double             dval1, xval1;
      double             dval2, xval2;
      double             dval3, xval3;
      double             total_conc;
      QList <HydroParm>  hp_distro, orig_list;
      US_Help            showHelp;
      US_Editor*         te;
      US_Model*          model;
		US_PlotPicker*     pick;
      QPushButton*       pb_load_model;
      QPushButton*       pb_prefilter;
      QPushButton*       pb_report;
      QPushButton*       pb_write;
      QLabel*            lbl_dval1;
      QLabel*            lbl_dval2;
      QLabel*            lbl_dval3;
      QLabel*            lbl_span;
      QLabel*            lbl_minimum;
      QLabel*            lbl_maximum;
      QLabel*            lbl_mean;
      QLabel*            lbl_mode;
      QLabel*            lbl_median;
      QLabel*            lbl_kurtosis;
      QLabel*            lbl_skew;
      QLabel*            lbl_sigma;
      QLabel*            lbl_integral;
      QLabel*            lbl_name;
      QLabel*            lbl_plotxmin;
      QLabel*            lbl_plotxmax;

      QwtCounter*        ct_dval1;
      QwtCounter*        ct_dval2;
      QwtCounter*        ct_dval3;
      QwtCounter*        ct_sigma;
      QButtonGroup*      bg_hp;
      QButtonGroup*      bg_plotlimits;
      QRadioButton*      rb_s;
      QRadioButton*      rb_d;
      QRadioButton*      rb_f;
      QRadioButton*      rb_k;
      QRadioButton*      rb_m;
      QRadioButton*      rb_v;
      QRadioButton*      rb_r;
      QStringList        pfilts;
      QString            mfilter;
      QString            run_name;
      QString            method;
      QString            analysis_name;
      QString            editGUID;
      QLineEdit*         le_model;
      QLineEdit*         le_investigator;
      QLineEdit*         le_prefilter;
      QLineEdit*         le_experiment;
      QLineEdit*         le_dval1;
      QLineEdit*         le_dval2;
      QLineEdit*         le_dval3;
      QLineEdit*         le_span;
      QLineEdit*         le_minimum;
      QLineEdit*         le_maximum;
      QLineEdit*         le_mean;
      QLineEdit*         le_mode;
      QLineEdit*         le_median;
      QLineEdit*         le_kurtosis;
      QLineEdit*         le_skew;
      QLineEdit*         le_integral;
      QLineEdit*         le_name;
      QLineEdit*         le_plotxmin;
      QLineEdit*         le_plotxmax;

      QwtPlot*           data_plot;

      QList< S_Solute >  sk_distro;
      QList< S_Solute >  xy_distro;
      QList< S_Solute >* sdistro;


      US_Disk_DB_Controls* disk_controls; //!< Radiobuttons for disk/db choice

private slots:

   QString table_row         ( const int, const QString &, const QString &,
                               const QString &, const QString &) const;
   QString table_row         ( const int, const QString &, const QString &,
                               const QString &, const QString &,
                               const QString &, const QString &) const;
   QString indent            ( const int ) const;
	void sel_investigator     ( void );
	void addReportItem        ( void );
	void update_disk_db       ( bool );
	void load_model           ( void );
	void select_prefilter     ( void );
	void reset                ( void );
	void calc                 ( void );
	void plot_data            ( void );
	void update_sigma         ( void );
	void update_name          ( const QString & );
	void select_hp            ( int  );
	void write_report         ( void );
   void sort_distro          ( QList< S_Solute >&, bool );
	bool equivalent           ( double, double, double );
	void set_dval1            ( double );
	void set_dval2            ( double );
	void set_dval3            ( double );
	void set_plotxmin         ( const QString & );
	void set_plotxmax         ( const QString & );
   void set_dval_labels      ( bool update=false );
	void set_sigma            ( double );
	void help                 ( void )
      { showHelp.show_help( "manual/us_modelmetrics.html" ); };
};
#endif

