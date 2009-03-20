//! \file us_astfem_sim.h
#ifndef US_ASTFEM_SIM_H
#define US_ASTFEM_SIM_H

#include <QtGui>

#include <vector>
using namespace std;

#include <qwt_plot.h>
#include <qwt_counter.h>


#include "us_widgets.h"
#include "us_plot.h"
#include "us_femglobal.h"
#include "us_help.h"
#include "us_astfem_rsa.h"

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

      US_Astfem_RSA* astfem_rsa;

      struct ModelSystem          system;
      struct SimulationParameters simparams;
      vector< struct mfem_data >  astfem_data;

      void init_simparams  ( void );  
      void save_xla        ( const QString& );  
      void save_ultrascan  ( const QString& );  

// debug
      void dump_system     ( void );
      void dump_simparms   ( void );
      void dump_astfem_data( void );
      void dump_simComponent( struct SimulationComponent& );
      void dump_association ( struct Association& );
      void dump_mfem_initial( struct mfem_initial& );
      void dump_ss          ( struct SpeedProfile& );
      void dump_mfem_scan   ( struct mfem_scan& );

   private slots:
      void load_experiment ( void );
      void save_experiment ( void );
      void new_model       ( void );
      void change_model    ( void );
      void load_model      ( void );
      void sim_parameters  ( void );
      void start_simulation( void );
      void stop_simulation ( void );
      void save_scans      ( void );
      void update_progress ( int  );
      void start_calc      ( unsigned int );
      void show_progress   ( unsigned int );
      void calc_over       ( void );
      void closeEvent      ( QCloseEvent* );
      
      void update_movie_plot( vector< double >&, double* );

      void update_time     ( double time )        
         { lcd_time ->display( time  ); };
      
      void update_speed    ( uint speed )
         { lcd_speed->display( (int) speed ); };

      void help            ( void )
         { showhelp.show_help( "manual/astfem_simulation.html" ); }; 

      void update_time_corr( void )
         { time_correctionFlag = cb_timeCorr->isChecked(); };
};                                                                   
#endif                                                               
                                                                     
