#ifndef US_GLOBEQUIL_H
#define US_GLOBEQUIL_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_editor.h"
#include "us_model.h"
#include "us_dataIO2.h"
#include "us_db2.h"
#include "us_simparms.h"
#include "us_astfem_math.h"
#include "us_astfem_rsa.h"

#include "qwt_counter.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

class US_EXTERN US_GlobalEquil : public US_Widgets
{
	Q_OBJECT
	
	public:
		US_GlobalEquil();

	private:
      QVector< US_DataIO2::EditedData > dataList;
      QVector< US_DataIO2::RawData >    rawList;
      QList< int >                      excludedScans;
      QStringList                       triples;

      US_Model                 model;
      US_SimulationParameters  simparams;

      US_DataIO2::EditedData*  edata;
      US_DataIO2::SpeedData*   spdata;

      US_Disk_DB_Controls*     dkdb_cntrls;

      QList< double >          speed_steps;

      US_Help        showHelp;
      QwtPlot*       equil_plot;
      QwtPlotCurve*  current_curve;
      US_Astfem_RSA* astfem_rsa;

      QwtCounter*    ct_scselect;

      QCheckBox*     ck_edlast;

      QTableWidget*  tw_equiscns;

      QLineEdit*     le_prjname;
      QLineEdit*     le_status;
      QLineEdit*     le_currmodl;
      QLineEdit*     le_mxfringe;

      QPushButton*   pb_details;
      QPushButton*   pb_view;
      QPushButton*   pb_unload;
      QPushButton*   pb_scdiags;
      QPushButton*   pb_ckscfit;
      QPushButton*   pb_conchist;
      QPushButton*   pb_resetsl;
      QPushButton*   pb_selModel;
      QPushButton*   pb_modlCtrl;
      QPushButton*   pb_fitcntrl;
      QPushButton*   pb_loadFit;
      QPushButton*   pb_monCarlo;
      QPushButton*   pb_floatPar;
      QPushButton*   pb_initPars;

      QIcon          blue_arrow;
      QIcon          green_arrow;
      QIcon          red_arrow;

      QString        workingDir;

      int            dbg_level;
      int            dbdisk;
             
      double         rpm_start;

      bool           dataLoaded;
      bool           buffLoaded;
      bool           modelLoaded;
      bool           dataLatest;

   private slots:
      void load              ( void );
      void details           ( void );
      void view_report       ( void );
      void unload            ( void );
      void scan_diags        ( void );
      void check_scan_fit    ( void );
      void conc_histogram    ( void );
      void reset_scan_lims   ( void );
      void load_model        ( void );
      void new_project_name  ( const QString& );
      void select_model      ( void );
      void model_control     ( void );
      void fitting_control   ( void );
      void load_fit          ( void );
      void monte_carlo       ( void );
      void float_params      ( void );
      void init_params       ( void );
      void close_all         ( void );
      void scan_select       ( double );
      void update_disk_db    ( bool );
      void clickedItem       ( QTableWidgetItem* );
      bool findData( QString, double, int&, int& );
      void edata_plot        ( void );

      void help              ( void )
      { showHelp.show_help("global_equil.html"); };
};
#endif
