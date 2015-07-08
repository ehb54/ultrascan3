#ifndef US_MODELMETRICS_H
#define US_MODELMETRICS_H

#include <QApplication>
#include <QDomDocument>
#include <QtGui>

#include "us_spectrodata.h"
#include "us_extern.h"
#include "us_widgets.h"
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

//! \brief Less-than function for sorting S_Solute values
bool distro_lessthan( const S_Solute&, const S_Solute& );

class US_ModelMetrics : public US_Widgets
{
	Q_OBJECT

	public:
      US_ModelMetrics();
      enum hydro_parms { HPs, HPd, HPm, HPk, HPf, HPv };


	private:

      int                dbg_level;
      int                mc_iters;
      int                fixed;
      int                calc_val;
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
      double             dval1, xval1;
      double             dval2, xval2;
      double             dval3, xval3;
      double             total_conc;
      QList <HydroParm>  hp_distro;
      US_Help            showHelp;
      US_Editor*         te;
      US_Model*          model;
      QPushButton*       pb_load_model;
      QPushButton*       pb_prefilter;
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

      QwtCounter*        ct_dval1;
      QwtCounter*        ct_dval2;
      QwtCounter*        ct_dval3;
      QButtonGroup*      bg_hp;
      QRadioButton*      rb_s;
      QRadioButton*      rb_d;
      QRadioButton*      rb_f;
      QRadioButton*      rb_k;
      QRadioButton*      rb_m;
      QRadioButton*      rb_v;
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

      QList< S_Solute >  sk_distro;
      QList< S_Solute >  xy_distro;
      QList< S_Solute >* sdistro;


      US_Disk_DB_Controls* disk_controls; //!< Radiobuttons for disk/db choice

private slots:
	void sel_investigator     ( void );
	void update_disk_db       ( bool );
	void load_model           ( void );
	void select_prefilter     ( void );
	void reset                ( void );
	void calc                 ( void );
	void select_hp            ( int );
	void write                ( void );
        void sort_distro          ( QList< S_Solute >&, bool );
	bool equivalent           ( double, double, double );
	void set_dval1            ( double );
	void set_dval2            ( double );
	void set_dval3            ( double );
	void help                 ( void )
        { showHelp.show_help( "manual/us_modelmetrics.html" ); };
};
#endif

