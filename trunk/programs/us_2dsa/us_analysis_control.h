//! \file us_analysis_control.h
#ifndef US_ANALYSIS_CTL_H
#define US_ANALYSIS_CTL_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_dataIO2.h"
#include "us_model.h"
#include "us_noise.h"
#include "us_plot.h"
#include "us_help.h"
#include "us_2dsa_process.h"

//! \brief A class to provide a window with enhanced plot controls

class US_EXTERN US_AnalysisControl : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \brief US_AnalysisControl constructor
      //! \param dat_exp Pointer to the experiment data
      //! \param         Pointer to the parent of this widget
      US_AnalysisControl( US_DataIO2::EditedData*, QWidget* p = 0 );

   public slots:
      void update_progress (   int );
      void completed_refine(   int );
      void progress_message(   QString );
      void completed_subgrids( void );
      void completed_process(  void );

   private:
      int           dbg_level;
      int           ncsteps;
      int           nctotal;

      QHBoxLayout*  mainLayout;
      QGridLayout*  controlsLayout;
      QGridLayout*  optimizeLayout;

      US_DataIO2::EditedData*      edata;
      US_DataIO2::RawData*         sdata;
      US_DataIO2::RawData*         rdata;
      US_Model*                    model;
      US_Noise*                    ri_noise;
      US_Noise*                    ti_noise;
      QPointer< QProgressBar >     mw_progbar;
      QPointer< QTextEdit    >     mw_stattext;

      QWidget*                     parentw;
      US_2dsaProcess*              processor;

      QwtCounter*   ct_lolimits;
      QwtCounter*   ct_uplimits;
      QwtCounter*   ct_nstepss;
      QwtCounter*   ct_lolimitk;
      QwtCounter*   ct_uplimitk;
      QwtCounter*   ct_nstepsk;
      QwtCounter*   ct_threadcnt;
      QwtCounter*   ct_grrefine;
      QwtCounter*   ct_repetitl;
      QwtCounter*   ct_scfactor;
      QwtCounter*   ct_scfact2;
      QwtCounter*   ct_repetitr;
      QwtCounter*   ct_stddevia;
      QwtCounter*   ct_coaldist;
      QwtCounter*   ct_nbrclips;
      QwtCounter*   ct_regufact;

      QCheckBox*    ck_tinoise;
      QCheckBox*    ck_rinoise;
      QCheckBox*    ck_autoupd;
      QCheckBox*    ck_unifgr;
      QCheckBox*    ck_locugr;
      QCheckBox*    ck_ranlgr;
      QCheckBox*    ck_soluco;
      QCheckBox*    ck_clipcs;
      QCheckBox*    ck_regulz;

      QLineEdit*    le_estmemory;
      QLineEdit*    le_iteration;
      QLineEdit*    le_oldvari;
      QLineEdit*    le_newvari;
      QLineEdit*    le_improve;

   protected:
      US_Help       showHelp;
      QProgressBar* b_progress;

   private slots:
      void optimize_options( void );
      void uncheck_optimize( int  );
      void checkUniGrid (    bool );
      void checkLocalUni(    bool );
      void checkRandLoc (    bool );
      void checkSoluCoal(    bool );
      void checkClipLow (    bool );
      void checkRegular (    bool );
      void grid_change(      void );
      void start(            void );
      void plot(             void );
      void close_all(        void );

      void help     ( void )
      { showHelp.show_help( "us_2dsa_analctl.html" ); };
};
#endif

