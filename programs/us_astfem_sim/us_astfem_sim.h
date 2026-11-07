//! \file us_astfem_sim.h
#ifndef US_ASTFEM_SIM_H
#define US_ASTFEM_SIM_H

#include "us_astfem_math.h"
#include "us_astfem_rsa.h"
#include "us_buffer.h"
#include "us_csv_data.h"
#include "us_dataIO.h"
#include "us_help.h"
#include "us_lamm_astfvm.h"
#include "us_model.h"
#include "us_plot.h"
#include "us_rotor_gui.h"
#include "us_sim_cli.h"
#include "us_simparms.h"
#include "us_widgets.h"
#include <qwt_counter.h>
#include <qwt_plot.h>

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

//! \brief Main window to control and display an ultracentrifugation
//!        simulation

class US_Astfem_Sim : public US_Widgets
{
   Q_OBJECT

   public:
      //! \param p Parent widget, normally not specified
      //! \param f Window flags, normally not specified
      US_Astfem_Sim( QWidget* p = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

      //! Initialize the us_astfem_sim with command line arguments
      //! \param args Parsed command-line options
      //! \returns int: 0 on success no gui needed, 1 on success but gui needed, 2 on error
      int init_from_args( const US_SimCliArgs& args );

   signals:
      void new_time( double );

   private:
      bool           stopFlag;            //!< Flag to stop the simulation
      bool           sim_failed;          //!< Set when start_simulation() could not produce data
      bool           movieFlag;           //!< Flag to show a movie during simulation
      bool           save_movie;          //!< Flag to save the generated frames
      bool           time_correctionFlag; //!< Flag to apply time correction - Currently not implemented/connected
      double         total_conc;          //!< Sum of all components concentration
      double         meniscus_ar;         //!< Meniscus position at rest in cm
      double         times_comp;          //!< Current time of the simulation
      int            icomponent;          //!< Index of the component, which is currently simulated
      int            ncomponent;          //!< Number of components
      int            curve_count;         //!< Number of currently simulated scans
      int            image_count;         //!< Current number of saved movie frames
      int            dbg_level;           //!< Debug level
      QString        imagedir;            //!< Path to the image dir
      QString        imageName;           //!< Full path template for movie frames ({imagedir}/frame{image_count}.png)
      QString        tmst_tfpath;         //!< Path to the timestate of the simulation in a temporary location
      QString        run_type;            //!< Two-character optical data type tag, "RA" unless overridden by --runtype
      int            sim_cell           = 1;            //!< Output cell
      QString        sim_description    = "Simulation"; //!< Output data description, from --description
      int            sim_centerpiece_id = 1;   //!< Centerpiece ID for the experiment record; 1 is the simulation centerpiece
      char           sim_channel        = 'S'; //!< Output channel
      uint noise_seed = 0; //!< Noise generator seed from --noise-seed or chosen for a started run; 0 leaves the generator as it is
      double od_limit   = 0.0;              //!< Headless clip threshold from --od-limit; 0 leaves readings unclipped
      QVector<double>            schedule_firsts;           //!< Schedule: each speed step's first cycle start
      QVector<int>               schedule_counts;           //!< Schedule: each speed step's scan count
      QVector<US_SimulationParameters::SpeedProfile> loaded_steps;              //!< Speed steps as given, for the run's record
      QVector<US_SimulationParameters::SpeedProfile> rebuilt_steps;             //!< Speed steps as set_parameters last rebuilt them
      QVector<double>                                interference_structure;    //!< Interference: the run's fixed radial structure
      QVector<double>                                written_max; //!< Interference: largest fringe value written per speed step
      QVector<QString>           ref_raw_guids; //!< RI: reference dataset raw GUID per speed step, set by save_xla
      QVector<double>            sample_max;    //!< RI: largest sample intensity per speed step, for its edit file
      QVector<double>            ref_max;       //!< RI: largest reference intensity per speed step, for its edit file
      bool noise_to_composite = true;     //!< Apply ASTFEM noise to the composite before splitting by speed
      QString                 guid_seed;  //!< Seed for reproducible GUIDs, from --guid-seed; empty mints random ones
      QString                 edit_stamp; //!< Fixed yyMMddhhmm for the edit filename, from --edit-timestamp; empty uses the clock

      QCheckBox*     ck_movie;            //!< Pointer to QCheckbox for movie display
      QCheckBox*     ck_savemovie;        //!< Pointer to QCheckbox for saving movie frames
      QCheckBox*     ck_timeCorr;         //!< Pointer to QCheckbox for applying time correction
      QPushButton*   pb_saveSim;          //!< Pointer to "Save Simulation" button
      QPushButton*   pb_buffer;           //!< Pointer to "Define Buffer" button
      QPushButton*   pb_simParms;         //!< Pointer to "Simulation Parameters" button
      QPushButton*   pb_rotor;            //!< Pointer to "Select Rotor" button
      QPushButton*   pb_changeModel;      //!< Pointer to "Model Control" button
      QPushButton*   pb_start;            //!< Pointer to "Start Simulation" button
      QPushButton*   pb_stop;             //!< Pointer to "Stop Simulation" button

      QTextEdit*     te_status;           //!< Pointer to status text box

      QLabel*        lb_component;        //!< Pointer to "Component" label
      QLabel*        lb_progress;         //!< Pointer to "Progress" label

      QLCDNumber*    lcd_time;            //!< Pointer to time lcd
      QLCDNumber*    lcd_speed;           //!< Pointer to speed lcd
      QLCDNumber*    lcd_component;       //!< Pointer to component lcd
      QProgressBar*  progress;            //!< Pointer to progress bar

      QwtPlot*       moviePlot;           //!< Pointer to upper plot QwtWidget
      QwtPlot*       scanPlot;            //!< Pointer to lower plot QwtWidget

      US_Plot*       plot1;               //!< Pointer to upper plot
      US_Plot*       plot2;               //!< Pointer to lower plot

      US_Help        showhelp;            //!< Help button

      QString        progress_text;       //!< Current progress text
      int            progress_value;      //!< Current progress value
      int            progress_maximum;    //!< Progress maximum value

      US_Astfem_RSA*          astfem;     //!< Pointer to the astfem simulation engine
      US_LammAstfvm*          astfvm;     //!< Pointer to the astfvm simulation engine
      US_Model                system;     //!< Model object
      US_Buffer               buffer;     //!< Buffer object
      US_Rotor::Rotor         rotor;      //!< Rotor object
      US_Rotor::RotorCalibration rotor_calibration; //!< Rotor calibration object
      US_SimulationParameters          simparams; //!< Simulation parameters
      US_AstfemMath::AstFemParameters  af_params; //!< Fixed grid simulation parameters used for exporting
      QVector<US_DataIO::RawData>      sim_datas; //!< Vector of simulated data for each speedstep
      US_DataIO::RawData               sim_data_all; //!< Overall simulated data
      US_CSV_Data                      csv_data_ti;
      QVector<US_CSV_Data>             csv_data_ri;

      void   init_simparams ( void );
      void   adjust_limits  ( double );
      double stretch        ( double*, double );
      bool                             save_xla( const QString&, US_DataIO::RawData, int, bool supress_dialog = false );
      void   finish         ( void );
      void   ri_noise       ( void );
      void   baseline       ( void );
      void   random_noise   ( void );
      bool                             intensity_mode( void ) const;  // Writing RI intensities
      bool                             wavelength_scan_mode( void ) const; // Writing a WI wavelength scan
      bool                             interference_mode( void ) const;    // Writing IP fringes with the interference model
      bool                             intensity_noise( void ) const; // Counts noise replaces absorbance random noise
      double                           buffer_absorbance( const US_DataIO::RawData& ) const; // Buffer OD at the data's wavelength
      QString                          schedule_problem( void ) const; // Unusable schedule or telemetry settings
      QString                          record_run_telemetry( void );   // Turn the time state into the run's record
      void   ti_noise       ( void );
      void                             derive_speed_data( void );
      US_DataIO::Scan&                 noise_scan( int, int, int );
      void   plot           ( int  );
      bool                             save_simulation( QString odir, bool supress_dialog = false, bool write_records = false );
      void save_csv_noise   ( US_CSV_Data& );

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

