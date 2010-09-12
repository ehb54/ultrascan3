#ifndef US_FEMATCH_H
#define US_FEMATCH_H

#include "us_resids_bitmap.h"
#include "us_plot_control.h"
#include "us_noise_loader.h"
#include "us_resplot.h"
#include "us_dataIO2.h"
#include "us_db2.h"
#include "us_astfem_rsa.h"
#include "us_model.h"
#include "us_noise.h"
#include "us_model_loader.h"
#include "us_editor.h"
#include "us_math2.h"
#include "us_run_details2.h"
#include "us_buffer_gui.h"
#include "qwt_plot_marker.h"

class US_EXTERN US_FeMatch : public US_Widgets
{
   Q_OBJECT

   public:
      US_FeMatch();

      US_DataIO2::EditedData*     fem_editdata();
      US_DataIO2::RawData*        fem_simdata();
      US_Model*                   fem_model();
      US_Noise*                   fem_ti_noise();
      US_Noise*                   fem_ri_noise();
      QPointer< US_ResidsBitmap > fem_resbmap();

   private:

      QLabel*       lb_tolerance;
      QLabel*       lb_division;
      QLabel*       lb_rmsd;
      QLabel*       lb_simpoints;
      QLabel*       lb_bldvolume;
      QLabel*       lb_parameter;

      QLineEdit*    le_id;
      QLineEdit*    le_temp;
      QLineEdit*    le_density;
      QLineEdit*    le_viscosity;
      QLineEdit*    le_vbar;
      QLineEdit*    le_rmsd;
      QLineEdit*    le_sedcoeff;
      QLineEdit*    le_difcoeff;
      QLineEdit*    le_partconc;
      QLineEdit*    le_variance;
      QLineEdit*    le_moweight;

      QTextEdit*    te_desc;

      QCheckBox*    ck_edit;

      QComboBox*    cb_mesh;
      QComboBox*    cb_grid;

      QGroupBox*    gb_modelsim;

      QwtCounter*   ct_from;
      QwtCounter*   ct_to;
      QwtCounter*   ct_component;
      QwtCounter*   ct_simpoints;
      QwtCounter*   ct_bldvolume;
      QwtCounter*   ct_parameter;
      QwtCounter*   ct_modelnbr;

      QwtPlotCurve*  curve;
      QwtPlotCurve*  dcurve;
      US_PlotPicker* gpick;

      QPushButton*  pb_load;
      QPushButton*  pb_details;
      QPushButton*  pb_view;
      QPushButton*  pb_save;
      QPushButton*  pb_distrib;
      QPushButton*  pb_reset;
      QPushButton*  pb_help;
      QPushButton*  pb_close;
      QPushButton*  pb_density;
      QPushButton*  pb_viscosity;
      QPushButton*  pb_vbar;
      QPushButton*  pb_rmsd;
      QPushButton*  pb_exclude;
      QPushButton*  pb_loadmodel;
      QPushButton*  pb_simumodel;
      QPushButton*  pb_showmodel;

      QListWidget*  lw_triples;

      int           scanCount;

      bool          dataLoaded;
      bool          haveSim;
      bool          dataLatest;
      bool          def_local;

      QString       run_name;
      QString       cell;
      QString       wavelength;
      QString       workingDir;
      QString       runID;
      QString       editID;
      QString       mfilter;
      QString       investig;

      QStringList   files;

      US_DataIO2::EditedData*     edata;
      US_DataIO2::Scan*           dscan;
      US_DataIO2::RawData*        rdata;
      US_DataIO2::RawData*        sdata;

      US_DB2*                     db;

      US_Model                    model;
      US_Noise                    ri_noise;
      US_Noise                    ti_noise;

      QPointer< US_ResidsBitmap > rbmapd;
      QPointer< US_PlotControl >  eplotcd;

      QPoint                      rbd_pos;
      QPoint                      epd_pos;

   private slots:

      void load(        void );
      void data_plot(   void );
      void save_data(   void );
      void details(     void );
      void reset(       void );
      void update(      int  );
      void update_density(   double );
      void update_viscosity( double );
      void get_buffer(     void );
      void get_vbar  (     void );
      void load_model(     void );
      void distrib_type(   void );
      void simulate_model( void );
      void view_report(    void );
      void exclude   (     void );
      void update_buffer(    double, double );
      void update_vbar(      double );
      void exclude_from(     double );
      void exclude_to  (     double );
      void comp_number (     double );
      void component_values( int    );
      void set_ra_visible(   bool );
      QStringList last_edit_files( QStringList );

      void distrib_plot_stick(  int );
      void distrib_plot_2d(     int );
      void distrib_plot_resids( void );

      void    load_noise(       void );
      int     models_in_edit(   bool, QString, QStringList& );
      int     noises_in_model ( bool, QString, QStringList& );
      double  interp_sval( double, double*, double*,  int );
      void    write_res();
      void    write_cofs();
      QString wave_index( int  );
      QString text_time(  double,   int );
      QString text_model( US_Model, int );
      double  calc_baseline(  int  );
      void    calc_residuals( void );
      double  average_temperature( US_DataIO2::EditedData* );
      void    close_all( void );

      void help     ( void )
      { showHelp.show_help( "fe_match.html" ); };

   protected:
      QList< int >                      excludedScans;
      QStringList                       triples;
      QVector< US_DataIO2::EditedData > dataList;
      QVector< US_DataIO2::RawData    > rawList;
      QVector< QVector< double >      > resids;
      
      US_Math2::SolutionData            solution;

      US_Help      showHelp;

      double       density;
      double       viscosity;
      double       vbar;

      // Layouts
      QBoxLayout*  mainLayout;
      QBoxLayout*  leftLayout;
      QBoxLayout*  rightLayout;
      QBoxLayout*  buttonLayout;

      QGridLayout* analysisLayout;
      QGridLayout* runInfoLayout;
      QGridLayout* parameterLayout;
      QGridLayout* controlsLayout;

      US_Plot*     plotLayout1;  // Derived from QVBoxLayout
      US_Plot*     plotLayout2;

      // Widgets
      QwtPlot*     data_plot1;
      QwtPlot*     data_plot2;
};
#endif
