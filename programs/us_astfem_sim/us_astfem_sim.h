//! \file us_astfem_sim.h
#ifndef US_ASTFEM_SIM_H
#define US_ASTFEM_SIM_H

#include "us_widgets.h"
#include "us_plot.h"
#include "us_model.h"
#include "us_simparms.h"
#include "us_help.h"
#include "us_rotor_gui.h"
#include "us_astfem_rsa.h"
#include "us_lamm_astfvm.h"
#include "us_buffer.h"
#include "us_dataIO.h"
#include "us_astfem_math.h"
#include <qwt_plot.h>
#include <qwt_counter.h>

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

//! \brief Main window to control and display an ultracentrifugation
//!        simulation

class US_Astfem_Sim : public US_Widgets
{
   Q_OBJECT

   public:
      //! \param p - Parent widget, normally not specified
      //! \param f - Window flags, normally not specified
      US_Astfem_Sim( QWidget* = 0, Qt::WindowFlags = 0 );

      // Write a timestate file based on auc data
      int  writetimestate( const QString&,  US_DataIO::RawData& );

    signals:
       void new_time         ( double );

   private:
      bool           stopFlag;
      bool           movieFlag;
      bool           save_movie;
      bool           time_correctionFlag;
      double         total_conc;
      double         meniscus_ar;
      double         times_comp;
      int            icomponent;
      int            ncomponent;
      int            curve_count;
      int            image_count;
      int            dbg_level;
      int            total_scans;
      QString        imagedir;
      QString        imageName;

      QString        tmst_tfpath;
      QString        currentDir;

      QCheckBox*     ck_movie;
      QCheckBox*     ck_savemovie;
      QCheckBox*     ck_timeCorr;

      QPushButton*   pb_saveExp;
      QPushButton*   pb_saveSim;
      QPushButton*   pb_buffer;
      QPushButton*   pb_simParms;
      QPushButton*   pb_rotor;
      QPushButton*   pb_changeModel;
      QPushButton*   pb_start;
      QPushButton*   pb_stop;

      QTextEdit*     te_status;

      QLabel*        lb_component;
      QLabel*        lb_progress;

      QLCDNumber*    lcd_time;
      QLCDNumber*    lcd_speed;
      QLCDNumber*    lcd_scan;
      QLCDNumber*    lcd_component;
      QProgressBar*  progress;

      QwtPlot*       moviePlot;
      QwtPlot*       scanPlot;

      US_Plot*       plot1;
      US_Plot*       plot2;

      US_Help        showhelp;

      QString        progress_text;
      int            progress_value;
      int            progress_maximum;

      US_Astfem_RSA*          astfem;
      US_LammAstfvm*          astfvm;
      US_Model                system;
      US_Buffer               buffer;

      US_SimulationParameters          simparams;
      US_AstfemMath::AstFemParameters  af_params;
      QVector<US_DataIO::RawData>      sim_datas;
      US_DataIO::RawData               sim_data_all;

      void   init_simparams ( void );
      void   adjust_limits  ( double );
      double stretch        ( double*, double );
      void   save_xla       ( const QString&, US_DataIO::RawData, int );
      void   save_ultrascan ( const QString& );
      void   finish         ( void );
      void   ri_noise       ( void );
      void   random_noise   ( void );
      void   ti_noise       ( void );
      void   plot           ( int  );
      // debug
      void dump_system      ( void );
      void dump_simparms    ( void );
      void dump_astfem_data ( void );
      void dump_simComponent( US_Model::SimulationComponent& );
      void dump_association ( US_Model::Association& );
      void dump_mfem_initial( US_Model::MfemInitial& );
      void dump_ss          ( US_SimulationParameters::SpeedProfile& );
      void dump_mfem_scan   ( US_DataIO::Scan& );

   private slots:

      void new_model       ( void );
      void change_model    ( US_Model );
      void new_buffer      ( void );
      void change_buffer   ( US_Buffer );
      void change_status   ( void );
      void set_parameters  ( void );
      void sim_parameters  ( void );
      void select_rotor    ( void );
      void assignRotor     ( US_Rotor::Rotor&, US_Rotor::RotorCalibration& );
      void start_simulation( void );
      void stop_simulation ( void );
      void save_scans      ( void );
      void update_progress ( int  );
      void update_component( int  );
      void start_calc      ( int );
      void show_progress   ( int );
      void calc_over       ( void );

      void update_movie_plot( QVector< double >*, double* );
      void update_save_movie( bool );
      void update_time      ( double time );

      void update_speed    ( int speed )
         { lcd_speed->display( (int) speed ); };

      void help            ( void )
         { showhelp.show_help( "manual/astfem_sim.html" ); };

      void update_time_corr( void )
         { time_correctionFlag = ck_timeCorr->isChecked(); };
};
#endif

