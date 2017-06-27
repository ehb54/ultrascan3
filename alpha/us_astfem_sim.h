//! \file us_astfem_sim.h
#ifndef US_ASTFEM_SIM_H
#define US_ASTFEM_SIM_H

#include "us_widgets.h"
#include "us_plot.h"
#include "us_model.h"
#include "us_simparms.h"
#include "us_help.h"
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
      
          // class init_simparams 
           // {
            //  public:
             // init_simparams( );
            //} ;   
      //void   load_mfem_data ( US_DataIO::RawData&, US_AstfemMath::MfemData& );
      // US_AstfemMath::AstFemParameters af_params;
      //US_AstfemMath::MfemData         afdata;
      int  writetimestate( const QString&,  US_DataIO::RawData&);
      //int  writetimestate( QString &,const QString& );

     // US_AstfemMath::MfemInitial      af_c0;
    //  void simulate_simulationgrid( US_DataIO::RawData&, US_SimulationParameters&, US_Model&);
    signals:
       void new_time         ( double );

   private:
      bool           stopFlag;
      bool           movieFlag;
      bool           save_movie;
      bool           time_correctionFlag;
      double         total_conc;
      int            curve_count;
      int            image_count;
      int            dbg_level;
      QString        imagedir;
      QString        imageName;

      QString       tmst_fnamei;
      QString       currentDir;
      
      QCheckBox*     ck_movie;
      QCheckBox*     ck_savemovie;
      QCheckBox*     ck_timeCorr;
                    
      QPushButton*   pb_saveExp;
      QPushButton*   pb_saveSim;
      QPushButton*   pb_buffer;
      QPushButton*   pb_simParms;
      QPushButton*   pb_changeModel;
      QPushButton*   pb_start;
      QPushButton*   pb_stop;

      QTextEdit*     te_status;

      QLabel*        lb_component;
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

      US_Astfem_RSA*          astfem;
      US_LammAstfvm*          astfvm;
      US_Model                system;
      US_Buffer               buffer;
      US_SimulationParameters simparams;
      US_SimulationParameters working_simparams;
      US_DataIO::RawData      sim_data;
//      US_AstfemMath::AstFemParameters af_params;
//      US_AstfemMath::MfemData         af_data;

//      US_AstfemMath::MfemInitial      af_c0;      
      void init_simparams  ( void );
      //void simulate_simulationgrid( US_DataIO::RawData&);
      // US_Astfem_Sim ();  
      void save_xla        ( const QString& );  
      void save_ultrascan  ( const QString& );  
      void finish          ( void );
      void ri_noise        ( void );
      void random_noise    ( void );
      void ti_noise        ( void );
      void plot            ( void );
      //const double set_minimum (const double):     
// debug
      void dump_system     ( void );
      void dump_simparms   ( void );
      void dump_astfem_data( void );
      void dump_simComponent( US_Model::SimulationComponent& );
      void dump_association ( US_Model::Association& );
      void dump_mfem_initial( US_Model::MfemInitial& );
      void dump_ss          ( US_SimulationParameters::SpeedProfile& );
      void dump_mfem_scan   ( US_DataIO::Scan& );
      //void   load_mfem_data ( US_DataIO::RawData&, US_AstfemMath::MfemData& );
      //void   store_mfem_data( US_DataIO::RawData&, US_AstfemMath::MfemData& );
      //void   initializeconc( int, US_AstfemMath::MfemInitial&, bool );

   private slots:
      
      void new_model       ( void );
      void change_model    ( US_Model );
      void new_buffer      ( void );
      void change_buffer   ( US_Buffer );
      void change_status   ( void );
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
      void update_save_movie( bool );

      void update_time     ( double time )        
         { lcd_time ->display( (int)time  ); };
      
      void update_speed    ( int speed )
         { lcd_speed->display( (int) speed ); };

      void help            ( void )
         { showhelp.show_help( "manual/astfem_sim.html" ); }; 

      void update_time_corr( void )
         { time_correctionFlag = ck_timeCorr->isChecked(); };
};                                                                   
#endif                                                               
                                                                     
