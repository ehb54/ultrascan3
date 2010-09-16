//! \file us_astfem_sim.h
#ifndef US_ASTFEM_SIM_H
#define US_ASTFEM_SIM_H

#include <QtGui>

#include <qwt_plot.h>
#include <qwt_counter.h>

#include "us_widgets.h"
#include "us_plot.h"
//#include "us_femglobal.h"
#include "us_model.h"
#include "us_simparms.h"
#include "us_help.h"
#include "us_astfem_rsa.h"
#include "us_dataIO2.h"

//! \brief Main window to control and display an ultracentrifugation 
//!        simulation

class US_Astfem_Sim : public US_Widgets
{
   Q_OBJECT

   public:
      //! \param p - Parent widget, normally not specified
      //! \param f - Window flags, normally not specified  
      US_Astfem_Sim( QWidget* = 0, Qt::WindowFlags = 0 );

   private:
      bool           stopFlag;
      bool           movieFlag;
      bool           time_correctionFlag;
      int            curve_count;
      double         total_conc;
      
      QCheckBox*     cb_movie;
      QCheckBox*     cb_timeCorr;
                    
      QPushButton*   pb_saveExp;
      QPushButton*   pb_saveSim;
      QPushButton*   pb_simParms;
      QPushButton*   pb_changeModel;
      QPushButton*   pb_start;
      QPushButton*   pb_stop;

      QLabel*        lb_progress;
                    
      QLCDNumber*    lcd_time;
      QLCDNumber*    lcd_speed;
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

      US_Astfem_RSA*          astfem_rsa;
      US_Model                system;
      US_SimulationParameters simparams;
      US_SimulationParameters working_simparams;
      US_DataIO2::RawData     sim_data;

      void init_simparams  ( void );  
      void save_xla        ( const QString& );  
      void save_ultrascan  ( const QString& );  
      void finish          ( void );
      void ri_noise        ( void );
      void random_noise    ( void );
      void ti_noise        ( void );
      void plot            ( void );

// debug
      void dump_system     ( void );
      void dump_simparms   ( void );
      void dump_astfem_data( void );
      void dump_simComponent( US_Model::SimulationComponent& );
      void dump_association ( US_Model::Association& );
      void dump_mfem_initial( US_Model::MfemInitial& );
      void dump_ss          ( US_SimulationParameters::SpeedProfile& );
      void dump_mfem_scan   ( US_DataIO2::Scan& );

   private slots:
      void load_experiment ( void );
      void save_experiment ( void );
      void new_model       ( void );
      void change_model    ( US_Model );
      void load_model      ( void );
      void set_parameters  ( void );
      void sim_parameters  ( void );
      void start_simulation( void );
      void stop_simulation ( void );
      void save_scans      ( void );
      void update_progress ( int  );
      void update_component( int  );
      void start_calc      ( int );
      void show_progress   ( int );
      void calc_over       ( void );
      
      void update_movie_plot( QVector< double >*, double* );

      void update_time     ( double time )        
         { lcd_time ->display( time  ); };
      
      void update_speed    ( int speed )
         { lcd_speed->display( (int) speed ); };

      void help            ( void )
         { showhelp.show_help( "manual/astfem_simulation.html" ); }; 

      void update_time_corr( void )
         { time_correctionFlag = cb_timeCorr->isChecked(); };
};                                                                   
#endif                                                               
                                                                     
