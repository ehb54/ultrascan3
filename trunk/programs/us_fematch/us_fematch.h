#ifndef US_FEMATCH_H
#define US_FEMATCH_H

#include "us_dataIO2.h"
#include "us_analysis_base2.h"
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

      int           valueCount;

      bool          dataLoaded;
      bool          dataLatest;

      QString       run_name;
      QString       cell;
      QString       wavelength;
      QString       workingDir;
      QString       runID;
      QString       editID;

      QStringList   files;

      US_DataIO2::EditedData*  d;
      US_DataIO2::Scan*        s;

   private slots:

      void load(        void );
      void data_plot(   void );
      void save_data(   void );
      void details(     void );
      void update(      int  );
      void update_density(   double );
      void update_viscosity( double );
      void get_buffer(  void );
      void get_vbar  (  void );
      void exclude   (  void );
      void update_buffer(    double, double );
      void update_vbar(      double );
      void exclude_from(     double );
      void exclude_to  (     double );
      void set_ra_visible(   bool );
      void set_edit_last(    int  );

      void help     ( void )
      { showHelp.show_help( "fematch.html" ); };

   protected:
      QList< int >                      excludedScans;
      QStringList                       triples;
      QVector< US_DataIO2::EditedData > dataList;
      QVector< US_DataIO2::RawData    > rawList;
      
      US_Math2::SolutionData Data       solution;

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
      QGridLayout* parametraLayout;
      QGridLayout* controlsLayout;

      US_Plot*     plotLayout1;  // Derived from QVBoxLayout
      US_Plot*     plotLayout2;

      // Widgets
      QwtPlot*     data_plot1;
      QwtPlot*     data_plot2;
};
#endif
