//! \file us_adv_analysis.h
#ifndef US_ADV_ANALYSIS_H
#define US_ADV_ANALYSIS_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_2dsa_process.h"
#include "us_plot.h"
#include "us_help.h"

//! \brief A class to provide a window for advanced analysis controls

class US_AdvAnalysis : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \brief US_AdvAnalysis constructor
      //! \param sim_par Pointer to simulation parameters
      //! \param         Pointer to the parent of this widget
      US_AdvAnalysis( US_SimulationParameters*, QWidget* p = 0 );

   public slots:
      void get_parameters( int&,  double&, double&, double&,
                           bool&, double&, double&,
                           bool&, double& );

   private:
      US_SimulationParameters* sparms;

      int           ncsteps;
      int           nctotal;

      QHBoxLayout*  mainLayout;
      QGridLayout*  optimizeLayout;
      QGridLayout*  simparmsLayout;

      QWidget*      parentw;

      QwtCounter*   ct_grrefine;
      QwtCounter*   ct_menisrng;
      QwtCounter*   ct_menispts;
      QwtCounter*   ct_mciters;
      QwtCounter*   ct_repetloc;
      QwtCounter*   ct_scfactor;
      QwtCounter*   ct_scfact2;
      QwtCounter*   ct_repetran;
      QwtCounter*   ct_stddevia;
      QwtCounter*   ct_coaldist;
      QwtCounter*   ct_nbrclips;
      QwtCounter*   ct_regufact;
      QwtCounter*   ct_bandload;
      QwtCounter*   ct_spoints;

      QCheckBox*    ck_unifgr;
      QCheckBox*    ck_locugr;
      QCheckBox*    ck_ranlgr;
      QCheckBox*    ck_soluco;
      QCheckBox*    ck_clipcs;
      QCheckBox*    ck_menisc;
      QCheckBox*    ck_mcarlo;
      QCheckBox*    ck_regulz;

      QRadioButton* rb_stndcp;
      QRadioButton* rb_bandcp;

      QComboBox*    cmb_mesh;
      QComboBox*    cmb_moving;

   protected:
      US_Help       showHelp;

   private slots:
      void optimize_options( void );
      void uncheck_optimize( int  );
      void checkBandForm(    bool );
      void checkUniGrid (    bool );
      void checkLocalUni(    bool );
      void checkRandLoc (    bool );
      void checkSoluCoal(    bool );
      void checkClipLow (    bool );
      void checkMeniscus(    bool );
      void checkMonteCar(    bool );
      void checkRegular (    bool );
      void select(           void );

      void help     ( void )
      { showHelp.show_help( "2dsa_advan.html" ); };
};
#endif

