//! \file us_analysis_control.h
#ifndef US_ANALYSIS_CTL_H
#define US_ANALYSIS_CTL_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_1dsa_process.h"
#include "us_plot.h"
#include "us_help.h"

//! \brief A class to provide a window with 2DSA analysis controls

class US_AnalysisControl : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \brief US_AnalysisControl constructor
      //! \param dat_exp Pointer to the experiment data
      //! \param         Pointer to the parent of this widget
      US_AnalysisControl( QList< US_SolveSim::DataSet* >&, QWidget* p = 0 );

   public slots:
      void update_progress (  double );
      void completed_process( int  );
      void progress_message(  QString, bool = true );
      void reset_steps(       int,     int );

   private:
      int           dbg_level;
      int           ncsteps;
      int           nctotal;
      int           grtype;
      int           bmndx;
      int           nkpts;
      int           ctype;
      double        f_inc;
      double        varimin;

      QHBoxLayout*  mainLayout;
      QGridLayout*  controlsLayout;
      QGridLayout*  optimizeLayout;

      QList< US_SolveSim::DataSet* >&  dsets;

      US_DataIO2::EditedData*          edata;
      US_DataIO2::RawData*             sdata;
      US_DataIO2::RawData*             rdata;
      US_Model*                        model;
      US_Noise*                        ri_noise;
      US_Noise*                        ti_noise;
      US_SimulationParameters*         sparms;
      QPointer< QTextEdit    >         mw_stattext;
      QStringList*                     mw_modstats;

      QVector< int >                   elitexs;

      QWidget*                         parentw;
      US_1dsaProcess*                  processor;

      QLabel*       lb_lolimitk;
      QLabel*       lb_uplimitk;
      QLabel*       lb_incremk;
      QLabel*       lb_varcount;

      QwtCounter*   ct_lolimits;
      QwtCounter*   ct_uplimits;
      QwtCounter*   ct_lolimitk;
      QwtCounter*   ct_uplimitk;
      QwtCounter*   ct_incremk;
      QwtCounter*   ct_varcount;
      QwtCounter*   ct_cresolu;
      QwtCounter*   ct_thrdcnt;

      QComboBox*    cmb_curvtype;

      QCheckBox*    ck_tinoise;
      QCheckBox*    ck_rinoise;

      QLineEdit*    le_minvari;
      QLineEdit*    le_minrmsd;

      QTextEdit*    te_status;

      QPushButton*  pb_pltlines;
      QPushButton*  pb_strtfit;
      QPushButton*  pb_stopfit;
      QPushButton*  pb_plot;
      QPushButton*  pb_save;

   protected:
      US_Help       showHelp;
      QProgressBar* b_progress;

   private slots:
      void optimize_options( void );
      void uncheck_optimize( int  );
      void slim_change(      void );
      void klim_change(      void );
      void start(            void );
      void stop_fit(         void );
      void plot(             void );
      void save(             void );
      void compute(          void );
      void plot_lines(       void );

      void help     ( void )
      { showHelp.show_help( "1dsa_analys.html" ); };
};
#endif

