//! \file us_adv_analysis_2d.h
#ifndef US_ADV_ANALYSIS_H
#define US_ADV_ANALYSIS_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_2dsa_process.h"
#include "us_plot.h"
#include "us_help.h"

//! \brief A class to provide a window for advanced analysis controls

class US_AdvAnalysis2D : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \brief US_AdvAnalysis2D constructor
      //! \param sim_par Pointer to simulation parameters
      //! \param loadDB  Flag for whether loads are from DB
      //! \param p       Pointer to the parent of this widget
      US_AdvAnalysis2D( US_SimulationParameters*, bool&, QWidget* p = 0 );

   public slots:
      void get_parameters( int&,  double&, double&, double&,
                           US_Model&, bool&, double& );

   private:
      US_SimulationParameters* sparms;
      bool&                    loadDB;
      US_Model                 model;

      int           ncsteps;
      int           nctotal;

      QHBoxLayout*  mainLayout;
      QGridLayout*  optimizeLayout;
      QGridLayout*  simparmsLayout;

      QWidget*      parentw;

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
      QwtCounter*   ct_sigma;
      QwtCounter*   ct_delta;

      QCheckBox*    ck_unifgr;
      QCheckBox*    ck_locugr;
      QCheckBox*    ck_ranlgr;
      QCheckBox*    ck_soluco;
      QCheckBox*    ck_clipcs;
      QCheckBox*    ck_menisc;
      QCheckBox*    ck_mcarlo;
      QCheckBox*    ck_regulz;
      QCheckBox*    ck_mdgrid;
      QCheckBox*    ck_mdrati;
      QCheckBox*    ck_conc_dependent;

      QPushButton*  pb_ldmodel;
      QPushButton*  pb_accept;

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
      void checkMdGrid  (    bool );
      void checkMdRatios(    bool );
      void checkMeniscus(    bool );
      void checkMonteCar(    bool );
      void checkRegular (    bool );
      void select       (    void );
      void load_model   (    void );
      void checkConcDependent( bool );

      void help     ( void )
      { showHelp.show_help( "2dsa_advan.html" ); };
};
#endif

