//! \file us_anal_control.h
#ifndef US_ANAL_CONTROL_H
#define US_ANAL_CONTROL_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_dataIO2.h"
#include "us_model.h"
#include "us_plot.h"
#include "us_help.h"

//! \brief A class to provide a window with enhanced plot controls

class US_EXTERN US_AnalControl : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \brief US_AnalControl constructor
      //! \param parent Pointer to the parent of this widget
      //! \param model  Pointer to the model to be plotted
      US_AnalControl( QWidget* p = 0, US_Model* = 0 );

   private:
      QHBoxLayout*  mainLayout;
      QGridLayout*  controlsLayout;
      QGridLayout*  optimizeLayout;

      US_Model*     model;

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
      void plot(             void );
      void close_all(        void );

      void help     ( void )
      { showHelp.show_help( "us_2dsa_analctl.html" ); };
};
#endif

