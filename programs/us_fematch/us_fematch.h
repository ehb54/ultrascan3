#ifndef US_FEMATCH_H
#define US_FEMATCH_H

#include "us_resids_bitmap.h"
#include "us_plot_control.h"
#include "us_advanced.h"
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
#include "us_analyte.h"
#include "us_solution.h"
#include "qwt_plot_marker.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

class US_FeMatch : public US_Widgets
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

      QLineEdit*    le_id;
      QLineEdit*    le_temp;
      QLineEdit*    le_solution;
      QLineEdit*    le_density;
      QLineEdit*    le_viscosity;
      QLineEdit*    le_vbar;
      QLineEdit*    le_compress;
      QLineEdit*    le_rmsd;
      QLineEdit*    le_variance;

      QTextEdit*    te_desc;

      QCheckBox*    ck_edit;

      QwtCounter*   ct_from;
      QwtCounter*   ct_to;

      QwtPlotCurve*  curve;
      QwtPlotCurve*  dcurve;
      US_PlotPicker* gpick;

      US_Disk_DB_Controls* dkdb_cntrls;

      QPushButton*  pb_load;
      QPushButton*  pb_details;
      QPushButton*  pb_solution;
      QPushButton*  pb_view;
      QPushButton*  pb_save;
      QPushButton*  pb_distrib;
      QPushButton*  pb_advanced;
      QPushButton*  pb_plot3d;
      QPushButton*  pb_plotres;
      QPushButton*  pb_reset;
      QPushButton*  pb_help;
      QPushButton*  pb_close;
      QPushButton*  pb_rmsd;
      QPushButton*  pb_exclude;
      QPushButton*  pb_loadmodel;
      QPushButton*  pb_simumodel;

      QListWidget*  lw_triples;

      int           scanCount;
      int           dbg_level;

      bool          dataLoaded;
      bool          haveSim;
      bool          dataLatest;
      bool          buffLoaded;

      QString       run_name;
      QString       cell;
      QString       wavelength;
      QString       workingDir;
      QString       runID;
      QString       editID;
      QString       dfilter;
      QString       mfilter;

      QStringList   files;

      US_DataIO2::EditedData*     edata;
      US_DataIO2::Scan*           dscan;
      US_DataIO2::RawData*        rdata;
      US_DataIO2::RawData*        sdata;

      US_Model                    model;
      US_Model                    model_loaded;
      US_Noise                    ri_noise;
      US_Noise                    ti_noise;
      US_Solution                 solution_rec;

      QPointer< US_ResidsBitmap > rbmapd;
      QPointer< US_PlotControl >  eplotcd;
      QPointer< US_ResidPlot >    resplotd;
      QPointer< US_Advanced >     advdiag;

      QPoint                      bmd_pos;
      QPoint                      epd_pos;
      QPoint                      rpd_pos;

      QMap< QString, QString >    adv_vals;

   private slots:

      void load(      void );
      void data_plot( void );
      void save_data( void );
      void details(   void );
      void reset(     void );
      void advanced(  void );
      void plot3d(    void );
      void plotres(   void );
      void update(    int  );
      void load_model(       void );
      void distrib_type(     void );
      void simulate_model(   void );
      void view_report(      void );
      void exclude   (       void );
      void adjust_model(     void );
      void adjust_mc_model(  void );
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
      double  interp_sval( double, double*, double*,  int );
      void    write_report(   QTextStream& );
      QString indent    (     int  )  const;
      QString text_model(     US_Model, int );
      double  calc_baseline(  int  )  const;
      void    calc_residuals( void );
      void    close_all(      void );
      QString table_row( const QString&, const QString& ) const;
      QString table_row( const QString&, const QString&,
                         const QString& )                 const;
      QString table_row( const QString&, const QString&,
                         const QString&, const QString&,
                         const QString& )                 const;
      QString html_header   ( QString, QString, US_DataIO2::EditedData* );
      QString data_details  ( void )                      const;
      QString hydrodynamics ( void )                      const;
      QString scan_info     ( void )                      const;
      QString distrib_info  ( void )                      const;
      void    set_progress(   const QString& );
      void    update_disk_db( bool );
      void    write_plot( const QString&, const QwtPlot* );
      bool    mkdir(      const QString&, const QString& );
      void    new_triple( int );
      void    get_solution  ( void );
      void    updateSolution( US_Solution& );

      void help     ( void )
      { showHelp.show_help( "fe_match.html" ); };

   protected:
      QList< int >                      excludedScans;
      QStringList                       triples;
      QVector< US_DataIO2::EditedData > dataList;
      QVector< US_DataIO2::RawData    > rawList;
      QVector< QVector< double >      > resids;
      QVector< QList< int > >           allExcls;
      
      US_Math2::SolutionData            solution;

      US_Help      showHelp;

      double       density;
      double       viscosity;
      double       vbar;
      double       compress;

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
