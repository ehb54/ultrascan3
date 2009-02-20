#ifndef US_ASTFEM_SIM_H
#define US_ASTFEM_SIM_H

#include <QtGui>

#include <qwt_plot.h>
#include <qwt_counter.h>


#include "us_widgets.h"
#include "us_plot.h"
#include "us_femglobal.h"
#include "us_help.h"
#include "us_astfem_rsa.h"

class US_Astfem_Sim : public US_Widgets
{
   Q_OBJECT

   public:
      US_Astfem_Sim( QWidget* = 0, Qt::WindowFlags = 0 );

   private:
      bool           stopFlag;
      bool           movieFlag;
      bool           time_correctionFlag;
      float          total_conc;
      
      QCheckBox*     cb_movie;
      QCheckBox*     cb_timeCorr;
                    
      QPushButton*   pb_saveExp;
      QPushButton*   pb_simParms;
      QPushButton*   pb_changeModel;
      QPushButton*   pb_start;
      QPushButton*   pb_stop;
                    
      QLCDNumber*    lcd_time;
      QLCDNumber*    lcd_speed;
      QLCDNumber*    lcd_component;
      QProgressBar*  progress;
                    
      US_Plot*       moviePlot;
      US_Plot*       scanPlot;
                    
      US_Help        showhelp;

      US_Astfem_RSA* astfem_rsa;

      struct ModelSystem          system;
      struct SimulationParameters simparams;
      QList< struct mfem_data >   astfem_data;

      void init_simparams  ( void );  

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
      
      void update_movie_plot( QList< double >&, double* );

      void update_time     ( float time )        
         { lcd_time ->display( time  ); };
      
      void update_speed    ( uint speed )
         { lcd_speed->display( (int) speed ); };

      void help            ( void )
         { showhelp.show_help( "manual/astfem_simulation.html" ); }; 

      void update_time_corr( void )
         { time_correctionFlag = cb_timeCorr->isChecked(); };

      void update_movieFlag( void )
         { cb_movie->isChecked(); };

};                                                                   
#endif                                                               
                                                                     
