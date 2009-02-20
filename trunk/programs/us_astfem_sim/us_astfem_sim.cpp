//! \file us_astfem_sim.cpp

#include <QApplication>

#include "us_license.h"
#include "us_license_t.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_model_editor.h"
#include "us_model_selection.h"
#include "us_astfem_sim.h"
#include "us_simulationparameters.h"
#include "us_math.h"

/*! \brief Main program for US_Astfem_Sim.  Loads translators and starts
    the class US_Astfem_Sim.
*/
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );
  
   #include "main1.inc"

   // License is OK.  Start up.
   US_Astfem_Sim w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

US_Astfem_Sim::US_Astfem_Sim( QWidget* p, Qt::WindowFlags f ) 
   : US_Widgets( p, f )
{

   setWindowTitle( "ASTFEM Simulation Module" );
   setPalette( US_GuiSettings::frameColor() );
   init_simparams();
   
   stopFlag            = false;
   movieFlag           = true;
   time_correctionFlag = true;

   astfem_rsa = new US_Astfem_RSA;

   connect( astfem_rsa, SIGNAL( new_scan         ( QList< double >&, double& ) ), 
            this      , SLOT(   update_movie_plot( QList< double >&, double& ) ) );
   
   connect( astfem_rsa, SIGNAL( current_component( int ) ), 
            this      , SLOT  ( update_progress  ( int ) ) );

   connect( astfem_rsa, SIGNAL( new_time   ( float ) ), 
            this      , SLOT  ( update_time( float ) ) );
   
   connect( astfem_rsa, SIGNAL( current_speed( unsigned int ) ), 
            this      , SLOT  ( update_speed ( unsigned int ) ) );

   QGridLayout* main = new QGridLayout( this );
   main->setSpacing( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );
   main->setColumnStretch( 1, 1 );
   
   QBoxLayout* buttonbox = new QVBoxLayout;

   QPushButton* pb_loadExperiment = us_pushbutton( tr( "Load Experiment") );
   connect ( pb_loadExperiment, SIGNAL( clicked() ), SLOT( load_experiment() ) );
   buttonbox->addWidget( pb_loadExperiment );

   pb_saveExp = us_pushbutton( tr( "Save Experiment"), false );
   buttonbox->addWidget( pb_saveExp );

   QPushButton* pb_new = us_pushbutton( tr( "New Model") );
   connect( pb_new, SIGNAL( clicked() ), SLOT( new_model() ) );
   buttonbox->addWidget( pb_new );
   
   QPushButton* pb_loadModel = us_pushbutton( tr( "Load Model") );
   connect( pb_loadModel, SIGNAL( clicked() ), SLOT( load_model() ) );
   buttonbox->addWidget( pb_loadModel );

   pb_changeModel = us_pushbutton( tr( "Change/Review Model"), false );
   connect ( pb_changeModel, SIGNAL( clicked() ) , SLOT( change_model() ) );
   buttonbox->addWidget( pb_changeModel );
   
   pb_simParms = us_pushbutton( tr( "Simulation Parameters"), false );
   connect ( pb_simParms, SIGNAL( clicked() ) , SLOT( sim_parameters() ) );
   buttonbox->addWidget( pb_simParms );

   cb_movie = us_checkbox( "Show Movie", movieFlag );
   connect( cb_movie, SIGNAL( clicked() ), SLOT( update_movieFlag() ) );
   buttonbox->addWidget( cb_movie );

   cb_timeCorr = us_checkbox( "Use Time Correction", time_correctionFlag );
   connect( cb_timeCorr, SIGNAL( clicked() ), SLOT( update_time_corr() ) );
   buttonbox->addWidget( cb_timeCorr );

   pb_start = us_pushbutton( tr( "Start Simulation"), false );
   connect( pb_start, SIGNAL( clicked() ), SLOT( start_simulation() ) );
   buttonbox->addWidget( pb_start );

   pb_stop = us_pushbutton( tr( "Stop Simulation"), false );
   connect( pb_stop, SIGNAL( clicked() ), SLOT( stop_simulation() ) );
   buttonbox->addWidget( pb_stop );

   //QPushButton* pb_dcdt = us_pushbutton( tr( "dC/dt Window"), false );
   //buttonbox->addWidget( pb_dcdt );

   QPushButton* pb_saveSim = us_pushbutton( tr( "Save Simulation"), false );
   connect( pb_saveSim, SIGNAL( clicked() ), SLOT( save_scans() ) );
   buttonbox->addWidget( pb_saveSim );

   QPushButton* pb_help = us_pushbutton( tr( "Help") );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help()) );
   buttonbox->addWidget( pb_help );

   QPushButton* pb_close = us_pushbutton( tr( "Close") );
   buttonbox->addWidget( pb_close );
   connect( pb_close, SIGNAL( clicked() ), SLOT( close()) );

   buttonbox->addStretch();
   main->addLayout( buttonbox, 0, 0 );

   // Right Column
   QBoxLayout* plot = new QVBoxLayout;

   // Simulation Plot
   moviePlot = new US_Plot( tr( "Simulation Window" ), tr( "Radius (cm)" ),
                            tr( "Concentration" ) );
   us_grid  ( moviePlot );
   moviePlot->setMinimumSize( 600, 275);
   moviePlot->setAxisScale( QwtPlot::yLeft, 0.0, 2.0 );
   moviePlot->setAxisScale( QwtPlot::xBottom, 5.8, 7.2 );

   plot->addWidget( moviePlot );

   QBoxLayout* timeSpeed = new QHBoxLayout;

   QLabel* lb_time = us_label( tr( "Time( in seconds):" ) );
   lb_time->setAlignment ( Qt::AlignCenter );
   timeSpeed->addWidget( lb_time );

   lcd_time = us_lcd( 5, 0 );
   timeSpeed->addWidget( lcd_time );
   
   QLabel* lb_speed = us_label( tr( "Current Speed:" ) );
   lb_speed->setAlignment ( Qt::AlignCenter );
   timeSpeed->addWidget( lb_speed );

   lcd_speed = us_lcd( 5, 0 );
   timeSpeed->addWidget( lcd_speed );
   
   plot->addLayout( timeSpeed );

   // Saved Scans
   scanPlot = new US_Plot( tr( "Saved Scans" ), tr( "Radius (cm)" ),
                           tr( "Concentration" ) );
   us_grid  ( scanPlot );
   scanPlot->setMinimumSize( 600, 275);
   scanPlot->setAxisScale( QwtPlot::yLeft, 0.0, 2.0 );
   scanPlot->setAxisScale( QwtPlot::xBottom, 5.8, 7.2 );
   plot->addWidget( scanPlot );

   QBoxLayout* completion = new QHBoxLayout;

   QLabel* lb_component = us_label( tr( "Component:" ) );
   lb_component->setAlignment ( Qt::AlignCenter );
   completion->addWidget( lb_component );

   lcd_component = us_lcd( 2, 0 );
   completion->addWidget( lcd_component );

   QLabel* lb_completed = us_label( tr( "% Completed:" ) );
   lb_completed->setAlignment ( Qt::AlignCenter );
   completion->addWidget( lb_completed );

   progress = us_progressBar( 0, 100, 0 );
   completion->addWidget( progress );

   plot->addLayout( completion );

   main->addLayout( plot, 0, 1 );
}

void US_Astfem_Sim::init_simparams( void )
{
   struct SpeedProfile sp;

   simparams.speed_step.clear();
   simparams.speed_step << sp;

   simparams.speed_step[ 0 ].duration_hours    = 5;
   simparams.speed_step[ 0 ].duration_minutes  = 30;
   simparams.speed_step[ 0 ].delay_hours       = 0;
   simparams.speed_step[ 0 ].delay_minutes     = 20;
   simparams.speed_step[ 0 ].rotorspeed        = 45000;
   simparams.speed_step[ 0 ].scans             = 30;
   simparams.speed_step[ 0 ].acceleration      = 400;
   simparams.speed_step[ 0 ].acceleration_flag = true;

   simparams.simpoints         = 100;
   simparams.radial_resolution = 0.001;
   simparams.mesh              = 0;
   simparams.moving_grid       = 1;
   simparams.meniscus          = 5.8;
   simparams.bottom            = 7.2;
   simparams.rnoise            = 0.0;
   simparams.tinoise           = 0.0;
   simparams.rinoise           = 0.0;
   simparams.band_volume       = 0.015;
   simparams.rotor             = 0;
   simparams.band_forming      = false;
}

void US_Astfem_Sim::load_experiment( void )
{
   QString fn = QFileDialog::getOpenFileName( this, 
         tr( "Select Experiment File" ),
         US_Settings::resultDir(), "*.us_system" );

   if ( ! fn.isEmpty() )
   {
      int error_code = US_FemGlobal::read_experiment( system, simparams, fn );
      
      if ( error_code < 0 )
      {
         QMessageBox::information( this, 
               tr( "Simulation Module" ), 
               tr( "Unable to load System: ") + fn + tr( "\n\nError code: " ) +
               QString::number( error_code ) ); 
         return;
      }
      else
      {
         pb_simParms   ->setEnabled( true );
         pb_changeModel->setEnabled( true );
         pb_start      ->setEnabled( true );

         QMessageBox::information( this, 
               tr( "Simulation Module" ), 
               tr( "Successfully loaded System:\n\n" ) + system.description );
      }
   }
}

void US_Astfem_Sim::save_experiment( void )
{
   QString fn = QFileDialog::getSaveFileName( this, 
         tr( "Save Experiment File" ), US_Settings::resultDir(), "*.us_system" );
   
   if ( ! fn.isEmpty() )
   {
      int error_code = US_FemGlobal::write_experiment( system, simparams, fn );

      if ( error_code < 0 )
      {
         QMessageBox::information( this, 
            tr( "Simulation Module "), 
            tr( "Unable to save System: " ) + fn + "\n\nError code: " +
            QString::number( error_code ) );
         return;
      }
      else
         QMessageBox::information( this, 
              tr( "Simulation Module" ), 
              tr( "Successfully saved System:\n\n") + system.description );
   }
}

void US_Astfem_Sim::new_model( void )
{
   US_ModelSelection::selectModel( system );

   if ( system.model >= 0 )
   {
      // Will be deleted when closed
      US_ModelEditor* component_dialog = new US_ModelEditor( system );
      if ( component_dialog->exec() )
      {
         pb_simParms   ->setEnabled( true );
         pb_changeModel->setEnabled( true );
      }
   }
}

void US_Astfem_Sim::change_model( void )
{
   US_ModelEditor* component_dialog = new US_ModelEditor( system );
   component_dialog->exec();
}

void US_Astfem_Sim::load_model( void )
{
   QString fn = QFileDialog::getOpenFileName( this, tr( "Select the model to load" ),
         US_Settings::resultDir(), "*.model.?? *.model-?.?? *model-??.??" );
   
   if ( fn.isEmpty() ) return;

   int error_code = US_FemGlobal::read_modelSystem( system, fn );
   if ( error_code < 0 )
   {
      QMessageBox::information( this, 
            tr( "Simulation Module" ), 
            tr( "Unable to load model: ") + fn + tr( "\n\nError code: " ) +
            QString::number( error_code ) ); 
      return;
   }
   else
   {
      pb_simParms   ->setEnabled( true );
      pb_changeModel->setEnabled( true );

      QMessageBox::information( this, 
            tr( "Simulation Module" ), 
            tr( "Successfully loaded System:\n\n" ) + system.description );
   }
}

void US_Astfem_Sim::sim_parameters( void )
{
   US_SimulationParameters* sp = new US_SimulationParameters( simparams );
   
   if ( sp->exec() )
   {
      qDebug() << "sim_parameters accepted";
      pb_start  ->setEnabled( true );
      pb_saveExp->setEnabled( true );
   }
   else
      qDebug() << "sim_parameters rejected";
}

void US_Astfem_Sim::stop_simulation( void )
{
   stopFlag = ! stopFlag;
   astfem_rsa->setStopFlag( stopFlag );
}

void US_Astfem_Sim::start_simulation( void )
{
   QMessageBox::information( this, "Under Construction", "start_simulation partially implemented" );

   moviePlot->clear();
   moviePlot->replot();

   scanPlot->clear();
   scanPlot->replot();

   pb_stop   ->setEnabled( true  );
   pb_start  ->setEnabled( false );
   pb_saveExp->setEnabled( false );

   // The astfem simulation routine expects a dataset structure that is
   // initialized with a time and radius grid, and all concentration points
   // need to be set to zero.  Each speed is a separate mfem_data set.

   astfem_data.clear();

   int ss_size =  simparams.speed_step.size();

   for ( int i = 0; i < ss_size; i++ )
   {
      struct SpeedProfile* sp = &simparams.speed_step[ i ];
      struct mfem_data     data;
      
      astfem_data << data;

      astfem_data[ i ].scan.clear();
      astfem_data[ i ].radius.clear();

      astfem_data[ i ].cell            = 1;
      astfem_data[ i ].channel         = 1;
      astfem_data[ i ].wavelength      = 1;
      astfem_data[ i ].avg_temperature = 20;
      astfem_data[ i ].rpm             = sp->rotorspeed;
      astfem_data[ i ].vbar            = 0.72;
      astfem_data[ i ].vbar20          = 0.72;
      astfem_data[ i ].viscosity       = 1.0;
      astfem_data[ i ].density         = 1.0;
      astfem_data[ i ].meniscus        = simparams.meniscus;
      astfem_data[ i ].bottom          = simparams.bottom;
      astfem_data[ i ].s20w_correction = 1.0;
      astfem_data[ i ].D20w_correction = 1.0;


      astfem_data[ i ].id              = "Simulated data set " +
         QString::number( i + 1 ) + " (of " + 
         QString::number( ss_size ) + ") for speed " +
         QString::number( sp->rotorspeed );

      // To get the right time, we need to take the last scan time from the
      // previous speed step and add the delays to it

      // First time point of this speed step in secs
      double current_time = sp->delay_hours * 3600 + sp->delay_minutes * 60; 

      astfem_data[ i ].wavelength = 999; // Simulation wavelength

      // Assign radius for the i'th dataset:
      double r = simparams.meniscus;
      
      while  ( r <= simparams.bottom )
      {
         astfem_data[ i ].radius << r;
         r += simparams.radial_resolution;
      }

      // Calculate the elapsed time between scans (total time - delay)/(scans-1)
      double increment = (   sp->duration_hours * 3600 + sp->duration_minutes*60
                           - sp->delay_hours    * 3600 - sp->delay_minutes   *60 
                         ) / sp->scans;
      
      mfem_scan temp_scan;

      for ( int j = 0; j < (int) sp->scans; j++ )
      {
         temp_scan.conc.clear();
         current_time  += increment;
         temp_scan.time = current_time;

         for ( int k = 0; k < astfem_data[ i ].radius.size(); k++ ) 
            temp_scan.conc << 0.0;

         astfem_data[ i ].scan << temp_scan;
      }
   }

   progress->reset();
   progress->setMaximum( system.component_vector.size() );

   // Interpolate simulation onto desired grid based on time, not based on
   // omega-square-t integral
   
   astfem_rsa->setTimeInterpolation( true ); 
   astfem_rsa->setTimeCorrection   ( time_correctionFlag );
   
   stopFlag = false;
   astfem_rsa->setStopFlag( stopFlag );
   
   simparams.band_firstScanIsConcentration = false;
   
   //astfem_rsa->calculate( system, simparams, astfem_data );

   // Add noise
   
   float maxconc = 0.0;
   
   for ( int i = 0; i < system.component_vector.size(); i++ )
      maxconc += system.component_vector[ i ].concentration;
   
   if ( simparams.rinoise != 0.0 )
   {
      float rinoise;
      
      for ( int i = 0; i < ss_size; i++ )
      {
         for ( int j = 0; j < astfem_data[ i ].scan.size(); j++ )
         {
            rinoise = US_Math::box_muller( 0, maxconc * simparams.rinoise / 100 );
            for ( int k = 0; k < astfem_data[ i ].radius.size(); k++ )
               astfem_data[ i ].scan[ j ].conc[ k ] += rinoise;
         }
      }
   }
   
   if ( simparams.rnoise != 0.0 )
   {
      for ( int i = 0; i < ss_size; i++ )
      {
         for ( int j = 0; j < astfem_data[ i ].scan.size(); j++ )
         {
            for ( int k = 0; k < astfem_data[ i ].radius.size(); k++ )
            {
               astfem_data[ i ].scan[ j ].conc[ k ] 
                  += US_Math::box_muller( 0, maxconc * simparams.rnoise / 100 );
            }
         }
      }
   }

   if ( simparams.tinoise != 0.0 )
   {
      QList< float > tinoise;
      
      float val = US_Math::box_muller( 0, maxconc * simparams.tinoise / 100 );
      
      tinoise.clear();
      
      for ( int k = 0; k < astfem_data[ 0 ].radius.size(); k++ )
      {
         val += US_Math::box_muller( 0, maxconc * simparams.tinoise / 100 );
         tinoise << val;
      }

      for ( int i = 0; i < ss_size; i++ )
      {
         for ( int j = 0; j < astfem_data[ i ].scan.size(); j++ )
         {
            for ( int k = 0; k < astfem_data[ i ].radius.size(); k++ )
            {
               astfem_data[ i ].scan[ j ].conc[ k ] += tinoise[ k ];
            }
         }
      }
   }

   if ( ! stopFlag )
   {
      // If we didn't interrupt, we need to set to 100 % complete at end of run
      progress->setValue( system.component_vector.size() ); 
   }

   stopFlag = false;
   
   int total_scans = 0;

   for ( int i = 0; i < ss_size; i++ )
      total_scans += simparams.speed_step[ i ].scans;
   
   total_conc = 0.0;

   for ( int i = 0; i < system.component_vector.size(); i++ )
      total_conc += system.component_vector[ i ].concentration;
   
   if (simparams.band_forming)
      scanPlot->setAxisScale( QwtPlot::yLeft, 0, total_conc );
   else
      scanPlot->setAxisScale( QwtPlot::yLeft, 0, total_conc * 2.0 );

   if ( ! stopFlag )
   {
      unsigned int* curve = new unsigned int[ total_scans ];
   
      double*  x;
      double** y;

      for ( int i = 0; i < ss_size; i++ )
      {
         int radius_count = astfem_data[ i ].radius.size();
         int scan_count   = astfem_data[ i ].scan.size();

         x = new double  [ radius_count ];
         y = new double* [ scan_count   ];

         for ( int j = 0; j < radius_count; j++ )
            x[ j ] = astfem_data[ i ].radius[ j ];

         for ( int j = 0; j < scan_count; j++ )
            y[ j ] = new double [ radius_count ];
         
         for ( int j = 0; j < scan_count; j++ )
         {
            for ( int k = 0; k < radius_count; k++ )
               y[ j ][ k ] = astfem_data[ i ].scan[ j ].conc[ k ];
         }


         for ( int j = 0; j < scan_count; j++ )
         {
            QString title = "Concentration" + QString::number( j );
            QwtPlotCurve* plotCurve = new QwtPlotCurve( title );
            
            plotCurve->setData( x, y[ j ], radius_count );
            plotCurve->setPen( QPen( Qt::yellow ) );
            plotCurve->attach( scanPlot );
         }

         delete [] x;
         
         for ( int j = 0; j < scan_count; j++ ) delete [] y[ j ];
         delete [] y;
      }

      delete [] curve;
      scanPlot->replot();
   }
}

void US_Astfem_Sim::save_scans( void )
{
   QMessageBox::information( this, "Under Construction", "save_scans not implemented yet" );
}


void US_Astfem_Sim::update_progress( int component )
{
   if ( component == -1 )
   {
      progress->setValue( system.component_vector.size() );
      lcd_component->setMode( QLCDNumber::Hex );
      lcd_component->display( "rA " );
   }
   else
   {
      progress->setValue( component - 1 );
      lcd_component->setMode( QLCDNumber::Dec );
      lcd_component->display( component );
   }
}

void US_Astfem_Sim::update_movie_plot( QList< double >& x, double* c )
{
   moviePlot->clear();

   double total_c = 0.0;
   
   for ( int i = 0; i < system.component_vector.size(); i++ )
      total_c += system.component_vector[ i ].concentration;

   moviePlot->setAxisScale( QwtPlot::yLeft, 0, total_c * 2.0 );
   
   double* r = new double [ x.size() ];
   
   for ( int i = 0; i < x.size(); i++ ) r[ i ] = x[ i ]; 

   QwtPlotCurve* curve = new QwtPlotCurve( "Concentration" );
   curve->setPen( QPen( Qt::yellow ) );
   curve->setData( r, c, x.size() );
   curve->attach( moviePlot );
   
   moviePlot->replot();
   qApp->processEvents();
   
   delete [] r;
}

#ifdef NEVER
#include "../include/us_astfem_sim.h"

US_Astfem_Sim::US_Astfem_Sim(QWidget *p, const char* name) : QFrame(p, name)
{
   cerr.precision(10);
   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("ASTFEM Simulation Module"));
   simparams.speed_step.resize(1);
   simparams.speed_step[0].duration_hours = 5;
   simparams.speed_step[0].duration_minutes = 30;
   simparams.speed_step[0].delay_hours = 0;
   simparams.speed_step[0].delay_minutes = 20;
   simparams.speed_step[0].rotorspeed =45000;
   simparams.speed_step[0].scans = 30;
   simparams.speed_step[0].acceleration = 400;
   simparams.speed_step[0].acceleration_flag = true;
   simparams.simpoints = 100;
   simparams.radial_resolution = 0.001;
   simparams.mesh = 0;
   simparams.moving_grid = 1;
   simparams.meniscus = 5.8;
   simparams.bottom = 7.2;
   simparams.rnoise = 0.0;
   simparams.tinoise = 0.0;
   simparams.rinoise = 0.0;
   simparams.band_volume = 0.015;
   simparams.rotor = 0;
   simparams.band_forming = false;
   stopFlag = false;
   movieFlag = true;
   time_correctionFlag = true;
   int minHeight1 = 26;

   astfem_rsa = new US_Astfem_RSA(true);

   connect( astfem_rsa, SIGNAL( new_scan         ( vector <double> *, double *) ), 
            this      , SLOT(   update_movie_plot( vector <double> *, double *)));
   
   connect( astfem_rsa, SIGNAL( current_component( int ) ), 
            this      , SLOT  ( update_progress  ( int ) ) );

   connect( astfem_rsa, SIGNAL( new_time   ( float ) ), 
            this      , SLOT  ( update_time( float ) ) );
   
   connect( astfem_rsa, SIGNAL( current_speed( unsigned int ) ), 
            this      , SLOT  ( update_speed ( unsigned int ) ) );

   pb_load_system = new QPushButton( tr("Load Experiment"), this );
   pb_load_system->setAutoDefault(false);
   pb_load_system->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_load_system->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(pb_load_system, SIGNAL(clicked()), SLOT(load_system()) );

   pb_save_system = new QPushButton( tr("Save Experiment"), this );
   pb_save_system->setAutoDefault(false);
   pb_save_system->setEnabled(false);
   pb_save_system->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled,    USglobal->global_colors.cg_pushb_active));
   pb_save_system->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(pb_save_system, SIGNAL(clicked()), SLOT(save_system()) );

   pb_new_model = new QPushButton( tr("New Model"), this );
   pb_new_model->setAutoDefault(false);
   pb_new_model->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_new_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(pb_new_model, SIGNAL(clicked()), SLOT(new_model()) );

   pb_load_model = new QPushButton( tr("Load Model"), this );
   pb_load_model->setAutoDefault(false);
   pb_load_model->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_load_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(pb_load_model, SIGNAL(clicked()), SLOT(load_model()) );

   pb_change_model = new QPushButton( tr("Change/Review Model"), this );
   pb_change_model->setAutoDefault(false);
   pb_change_model->setEnabled(false);
   pb_change_model->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_change_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(pb_change_model, SIGNAL(clicked()), SLOT(change_model()) );

   pb_simulation_parameters = new QPushButton( tr("Simulation Parameters"), this );
   pb_simulation_parameters->setAutoDefault(false);
   pb_simulation_parameters->setEnabled(false);
   pb_simulation_parameters->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_simulation_parameters->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(pb_simulation_parameters, SIGNAL(clicked()), SLOT(simulation_parameters()) );

   pb_start_simulation = new QPushButton( tr("Start Simulation"), this );
   pb_start_simulation->setAutoDefault(false);
   pb_start_simulation->setEnabled(false);
   pb_start_simulation->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_start_simulation->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(pb_start_simulation, SIGNAL(clicked()), SLOT(start_simulation()) );

   pb_stop_simulation = new QPushButton( tr("Stop Simulation"), this );
   pb_stop_simulation->setAutoDefault(false);
   pb_stop_simulation->setEnabled(false);
   pb_stop_simulation->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_stop_simulation->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(pb_stop_simulation, SIGNAL(clicked()), SLOT(stop_simulation()) );

   pb_dcdt_window = new QPushButton( tr("dC/dt Window"), this );
   pb_dcdt_window->setAutoDefault(false);
   pb_dcdt_window->setEnabled(false);
   pb_dcdt_window->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_dcdt_window->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(pb_dcdt_window, SIGNAL(clicked()), SLOT(dcdt_window()) );

   cb_movie = new QCheckBox(" Show Movie ", this);
   cb_movie->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_movie->setChecked(movieFlag);
   cb_movie->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_movie->setMinimumHeight(minHeight1);
   connect(cb_movie, SIGNAL(clicked()), SLOT(update_movieFlag()));

   cb_time_correction = new QCheckBox(" Use Time Correction ", this);
   cb_time_correction->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_time_correction->setChecked(time_correctionFlag);
   cb_time_correction->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_time_correction->setMinimumHeight(minHeight1);
   connect(cb_time_correction, SIGNAL(clicked()), SLOT(update_time_correctionFlag()));

   pb_save_scans = new QPushButton( tr("Save Simulation"), this );
   pb_save_scans->setAutoDefault(false);
   pb_save_scans->setEnabled(false);
   pb_save_scans->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_save_scans->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(pb_save_scans, SIGNAL(clicked()), SLOT(save_scans()) );

   pb_help = new QPushButton( tr("Help"), this );
   pb_help->setAutoDefault(false);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()) );

   pb_close = new QPushButton( tr("Close"), this );
   pb_close->setAutoDefault(false);
   pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(pb_close, SIGNAL(clicked()), SLOT(close()) );

   movie_plot = new QwtPlot(this);
   movie_plot->setTitle(tr("Simulation Window"));
   movie_plot->enableGridXMin();
   movie_plot->enableGridYMin();
   movie_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
   movie_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   movie_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
   movie_plot->setCanvasBackground(USglobal->global_colors.plot);    //new version
   movie_plot->setAxisTitle(QwtPlot::xBottom, tr("Radius (cm)"));
   movie_plot->setAxisTitle(QwtPlot::yLeft, tr("Concentration"));
   movie_plot->enableOutline(true);
   movie_plot->setOutlinePen(white);
   movie_plot->setOutlineStyle(Qwt::Cross);
   movie_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   movie_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   movie_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   movie_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   movie_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   movie_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   movie_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   movie_plot->setMargin(USglobal->config_list.margin);
   movie_plot->setMinimumSize(600, 275);

   scan_plot= new QwtPlot(this);
   scan_plot->setTitle(tr("Saved Scans:"));
   scan_plot->enableGridXMin();
   scan_plot->enableGridYMin();
   scan_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
   scan_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   scan_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
   scan_plot->setCanvasBackground(USglobal->global_colors.plot);     //new version
   scan_plot->setAxisTitle(QwtPlot::xBottom, tr("Radius (cm)"));
   scan_plot->setAxisTitle(QwtPlot::yLeft, tr("Concentration"));
   scan_plot->enableOutline(true);
   scan_plot->setOutlinePen(white);
   scan_plot->setOutlineStyle(Qwt::Cross);
   scan_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   scan_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   scan_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   scan_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   scan_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   scan_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   scan_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   scan_plot->setMargin(USglobal->config_list.margin);
   scan_plot->setMinimumSize(600, 275);

   lbl_speed = new QLabel(tr("Current Speed:"), this);
   lbl_speed->setAlignment(AlignCenter|AlignVCenter);
   lbl_speed->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_speed->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));

   lbl_time = new QLabel(tr("Time (in seconds):"), this);
   lbl_time->setAlignment(AlignCenter|AlignVCenter);
   lbl_time->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_time->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));

   lbl_component = new QLabel(tr("   Component:   "), this);
   lbl_component->setAlignment(AlignCenter|AlignVCenter);
   lbl_component->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_component->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));

   lbl_progress = new QLabel(tr("    % Completed:   "), this);
   lbl_progress->setAlignment(AlignCenter|AlignVCenter);
   lbl_progress->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_progress->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));

   lcd_time  = new QLCDNumber( 6, this, "time" );
   lcd_time->setPalette( QPalette(USglobal->global_colors.cg_lcd, USglobal->global_colors.cg_lcd, USglobal->global_colors.cg_lcd));
   lcd_time->setSegmentStyle ( QLCDNumber::Filled );
   lcd_time->setMode( QLCDNumber::DEC );

   lcd_speed  = new QLCDNumber( 6, this, "time" );
   lcd_speed->setPalette( QPalette(USglobal->global_colors.cg_lcd, USglobal->global_colors.cg_lcd, USglobal->global_colors.cg_lcd));
   lcd_speed->setSegmentStyle ( QLCDNumber::Filled );
   lcd_speed->setMode( QLCDNumber::DEC );

   lcd_component  = new QLCDNumber( 4, this, "component" );
   lcd_component->setPalette( QPalette(USglobal->global_colors.cg_lcd, USglobal->global_colors.cg_lcd, USglobal->global_colors.cg_lcd));
   lcd_component->setSegmentStyle ( QLCDNumber::Filled );
   lcd_component->setMode( QLCDNumber::DEC );

   progress = new QProgressBar(100, this, "Simulation Progress");
   progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   progress->setPercentageVisible(true);

   global_Xpos += 30;
   global_Ypos += 30;
   setup_GUI();
   system.model = -1;
}

US_Astfem_Sim::~US_Astfem_Sim()
{
}

void US_Astfem_Sim::setup_GUI()
{
   QGridLayout *topBox = new QGridLayout(this, 4, 2, 4, 2);
   topBox->setColStretch(1, 1);
   topBox->setRowSpacing(1, 30);
   topBox->setRowSpacing(3, 30);
   QVBoxLayout *buttonBox = new QVBoxLayout();
   buttonBox->addWidget(pb_load_system);
   buttonBox->addWidget(pb_save_system);
   buttonBox->addWidget(pb_new_model);
   buttonBox->addWidget(pb_load_model);
   buttonBox->addWidget(pb_change_model);
   buttonBox->addWidget(pb_simulation_parameters);
   buttonBox->addWidget(cb_movie);
   buttonBox->addWidget(cb_time_correction);
   buttonBox->addWidget(pb_start_simulation);
   buttonBox->addWidget(pb_stop_simulation);
   buttonBox->addWidget(pb_dcdt_window);
   buttonBox->addWidget(pb_save_scans);
   buttonBox->addWidget(pb_help);
   buttonBox->addWidget(pb_close);

   topBox->addMultiCellLayout(buttonBox, 0, 2, 0, 0, Qt::AlignTop|Qt::AlignLeft);
   topBox->addWidget(movie_plot, 0, 1, 0);

   QHBoxLayout *row1 = new QHBoxLayout();
   row1->addWidget(lbl_time);
   row1->addWidget(lcd_time);
   row1->addWidget(lbl_speed);
   row1->addWidget(lcd_speed);
   topBox->addLayout(row1, 1, 1);

   topBox->addWidget(scan_plot, 2, 1, 0);

   QHBoxLayout *row2 = new QHBoxLayout();
   row2->addWidget(lbl_component);
   row2->addWidget(lcd_component);
   row2->addWidget(lbl_progress);
   row2->addWidget(progress);
   topBox->addLayout(row2, 3, 1);

   qApp->processEvents();
   QRect r = topBox->cellGeometry(0, 0);
   this->setGeometry(global_Xpos, global_Ypos, r.width()+10+600, 550+6+8);
}

void US_Astfem_Sim::closeEvent(QCloseEvent *e)
{
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

// load a model from the list
void US_Astfem_Sim::new_model()
{
   US_ModelSelection *model_sel;
   model_sel = new US_ModelSelection();
   model_sel->select_model(&system);
   delete model_sel;
   if (system.model >= 0)
   {
      assign_model();
   }
}

// load the model editor to edit the default values of the newly selected model:

void US_Astfem_Sim::assign_model()
{
   US_ModelEditor *component_dialog;
   component_dialog = new US_ModelEditor(true, &system);
   if (component_dialog->exec())
   {
      pb_simulation_parameters->setEnabled(true);
      pb_change_model->setEnabled(true);
//    pb_start_simulation->setEnabled(true);
//    pb_stop_simulation->setEnabled(true);
//    pb_dcdt_window->setEnabled(true);
//    pb_save_scans->setEnabled(true);
   }
   delete component_dialog;
}


void US_Astfem_Sim::load_system()
{
   QString fn = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*.us_system", 0);
   if ( !fn.isEmpty() )
   {
      load_system(fn);
   }
}

void US_Astfem_Sim::load_system(const QString &filename)
{
   int error_code;
   US_FemGlobal fg;
   QString str;
   error_code = fg.read_experiment(&system, &simparams, filename);
   if (error_code < 0)
   {
      str.sprintf("Unable to load System: " + filename + "\n\nError code: %d", error_code);
      QMessageBox::information(this, tr("Simulation Module"), tr(str), QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
      return;
   }
   else
   {
      pb_simulation_parameters->setEnabled(true);
      pb_change_model->setEnabled(true);
      pb_simulation_parameters->setEnabled(true);
      pb_start_simulation->setEnabled(true);
      printError(5);
   }
}

void US_Astfem_Sim::save_system()
{
   QString str, fn = QFileDialog::getSaveFileName(USglobal->config_list.result_dir, "*.us_system", 0);
   if ( !fn.isEmpty() )
   {
      save_system(fn);     // the user gave a file name
   }
}

void US_Astfem_Sim::save_system(const QString &filename)
{
   int error_code;
   US_FemGlobal fg;
   QString str;
   error_code = fg.write_experiment(&system, &simparams, filename);
   if (error_code < 0)
   {
      str.sprintf("Unable to save System: " + filename + "\n\nError code: %d", error_code);
      QMessageBox::information(this, tr("Simulation Module"), tr(str), QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
      return;
   }
   else
   {
      printError(6);
   }
}

void US_Astfem_Sim::printError(const int &ival)
{
   switch (ival)
   {
      case 0:
      {
         QMessageBox::warning(this, tr("UltraScan Warning"),
         tr("Please note:\n\nThere was an error reading\nthe selected Model File!\n\nThis file appears to be corrupted"),
         QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
         break;
      }
      case 1:
      {
         QMessageBox::warning(this, tr("UltraScan Warning"),
         tr("Sorry, for old-style model files only\nnon-interacting model loading is supported.\n\n") +
         tr("Please recreate this model by clicking on:\n\n\"New Model\""),
         QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
         break;
      }
      case 2:
      {
         QMessageBox::warning(this, tr("UltraScan Warning"), tr("Please note:\n\n") +
         tr("UltraScan could not open\nthe selected Model File!"),
         QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
         break;
      }
      case 3:
      {
         QMessageBox::warning(this, tr("UltraScan Warning"),
         tr("Please note:\n\nUltraScan could not read\nthe selected Model File!"),
         QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
         break;
      }
      case 4:
      {
         QMessageBox::information(this, tr("Simulation Module"), tr("Successfully loaded Model:\n\n")
         + system.description, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
         break;
      }
      case 5:
      {
         QMessageBox::information(this, tr("Simulation Module"), tr("Successfully loaded System:\n\n")
               + system.description, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
         break;
      }
      case 6:
      {
         QMessageBox::information(this, tr("Simulation Module"), tr("Successfully saved System:\n\n")
               + system.description, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
         break;
      }
   }
}

void US_Astfem_Sim::change_model()
{
   US_ModelEditor *component_dialog;
   component_dialog = new US_ModelEditor(true, &system);
   if (component_dialog->exec())
   {
      pb_simulation_parameters->setEnabled(true);
      pb_change_model->setEnabled(true);
   }
   delete component_dialog;
}

void US_Astfem_Sim::simulation_parameters()
{
   US_SimulationParameters *sp;
   sp = new US_SimulationParameters(&simparams, this);
   if (sp->exec())
   {
      pb_start_simulation->setEnabled(true);
      pb_save_system->setEnabled(true);
   }
   delete sp;
}

void US_Astfem_Sim::start_simulation()
{
   unsigned int current_time = 0;
   unsigned int increment;
   double r;
   unsigned int i, j, k;
   QString str;
   mfem_scan temp_scan;

   movie_plot->clear();
   scan_plot->clear();
   movie_plot->replot();
   scan_plot->replot();
   pb_stop_simulation->setEnabled(true);
   pb_start_simulation->setEnabled(false);
   pb_save_scans->setEnabled(false);
   pb_dcdt_window->setEnabled(true);

// the astfem simulation routine expects a dataset structure that is initialized
// with a time- and radius grid, and all concentration points need to be set to zero.
// Each speed is a separate mfem_data set.

   astfem_data.resize(simparams.speed_step.size());

   for (i=0; i<simparams.speed_step.size(); i++)
   {
      astfem_data[i].scan.clear();
      astfem_data[i].radius.clear();
      astfem_data[i].cell = 1;
      astfem_data[i].channel = 1;
      astfem_data[i].wavelength = 1;
      astfem_data[i].avg_temperature = 20;
      astfem_data[i].rpm = simparams.speed_step[i].rotorspeed;
      astfem_data[i].vbar = 0.72;
      astfem_data[i].vbar20 = 0.72;
      astfem_data[i].viscosity = 1.0;
      astfem_data[i].density = 1.0;
      astfem_data[i].meniscus = simparams.meniscus;
      astfem_data[i].bottom = simparams.bottom;
      astfem_data[i].s20w_correction = 1.0;
      astfem_data[i].D20w_correction = 1.0;
      astfem_data[i].id = str.sprintf(tr("Simulated data set %u (of %u) for speed %u"), i+1,
      simparams.speed_step.size(), simparams.speed_step[i].rotorspeed);

// to get the right time, we need to take the last scan time from the previous speed step
// and add the delays to it
      current_time += (unsigned int) (simparams.speed_step[i].delay_hours * 3600
            + simparams.speed_step[i].delay_minutes * 60); // first time point of this speed step in secs

      astfem_data[i].wavelength = 999; // simulation wavelength

// assign radius for the i'th dataset:
      r = simparams.meniscus;
      while (r <= simparams.bottom)
      {
         astfem_data[i].radius.push_back(r);
         r += simparams.radial_resolution;
      }

// calculate the elapsed time between scans (total time - delay)/(scans-1):
      increment = (unsigned int) ((simparams.speed_step[i].duration_hours * 3600
      + simparams.speed_step[i].duration_minutes * 60
      - simparams.speed_step[i].delay_hours * 3600
            - simparams.speed_step[i].delay_minutes * 60)/(simparams.speed_step[i].scans));
      for (j=0; j<simparams.speed_step[i].scans; j++)
      {
         temp_scan.conc.clear();
         current_time += increment;
         temp_scan.time = current_time;
         for (k=0; k<astfem_data[i].radius.size(); k++)
         {
            temp_scan.conc.push_back(0.0);
         }
         astfem_data[i].scan.push_back(temp_scan);
      }
   }
   progress->reset();
   progress->setTotalSteps(system.component_vector.size());
   astfem_rsa->setTimeCorrection(time_correctionFlag);
   astfem_rsa->setTimeInterpolation(true); // interpolate simulation onto desired grid based on time, not based on omega-square-t integral
   astfem_rsa->setMovie(movieFlag); // show movie
   stopFlag = false;
   pb_stop_simulation->setText("Stop Simulation");
   astfem_rsa->setStopFlag(stopFlag);
   simparams.band_firstScanIsConcentration = false;
   astfem_rsa->calculate(&system, &simparams, &astfem_data);
   // add noise:
   float maxconc = 0.0;
   for (i=0; i<system.component_vector.size(); i++)
   {
      maxconc += system.component_vector[i].concentration;
   }
   if (simparams.rinoise != 0.0)
   {
      float rinoise;
      for (i=0; i<simparams.speed_step.size(); i++)
      {
         for (j=0; j<astfem_data[i].scan.size(); j++)
         {
            rinoise = US_Math::box_muller(0, maxconc * simparams.rinoise/100);
            for (k=0; k<astfem_data[i].radius.size(); k++)
            {
               astfem_data[i].scan[j].conc[k] += rinoise;
            }
         }
      }
   }
   if (simparams.rnoise != 0.0)
   {
      for (i=0; i<simparams.speed_step.size(); i++)
      {
         for (j=0; j<astfem_data[i].scan.size(); j++)
         {
            for (k=0; k<astfem_data[i].radius.size(); k++)
            {
               astfem_data[i].scan[j].conc[k] 
                  += US_Math::box_muller(0, maxconc * simparams.rnoise/100);
            }
         }
      }
   }
   if (simparams.tinoise != 0.0)
   {
      vector <float> tinoise;
      float val = US_Math::box_muller(0, maxconc * simparams.tinoise/100);
      tinoise.clear();
      for (k=0; k<astfem_data[0].radius.size(); k++)
      {
         val += US_Math::box_muller(0, maxconc * simparams.tinoise/100);
         tinoise.push_back(val);
      }
      for (i=0; i<simparams.speed_step.size(); i++)
      {
         for (j=0; j<astfem_data[i].scan.size(); j++)
         {
            for (k=0; k<astfem_data[i].radius.size(); k++)
            {
               astfem_data[i].scan[j].conc[k] += tinoise[k];
            }
         }
      }
   }
   if (!stopFlag)
   {
      progress->setProgress(system.component_vector.size()); // if we didn't interrupt, we need to set to 100 % complete at end of run'
   }
   stopFlag = false;
   pb_stop_simulation->setEnabled(false);
   pb_start_simulation->setEnabled(true);
   pb_save_scans->setEnabled(true);
   pb_dcdt_window->setEnabled(false);
   j=0;
   for (i=0; i<simparams.speed_step.size(); i++)
   {
      j += simparams.speed_step[i].scans;
//    for (k=0; k<simparams.speed_step[i].scans; k++)
//    {
//       cout << "speed step " << i+1 << ", scan: " << k+1 << ": " << astfem_data[i].scan[k].time << endl;
//    }
   }
   total_conc = 0.0;
   for (i=0; i<system.component_vector.size(); i++)
   {
      total_conc += system.component_vector[i].concentration;
   }
   if (simparams.band_forming)
   {
      scan_plot->setAxisScale(QwtPlot::yLeft, 0, total_conc);
   }
   else
   {
      scan_plot->setAxisScale(QwtPlot::yLeft, 0, total_conc * 2.0);
   }

   if(!stopFlag)
   {
   //unsigned int curve[j];
      unsigned int* curve = new unsigned int[j];
   
      double *x, **y;

      for (i=0; i<simparams.speed_step.size(); i++)
      {
         x = new double [astfem_data[i].radius.size()];
         y = new double * [astfem_data[i].scan.size()];
         for (j=0; j<astfem_data[i].radius.size(); j++)
         {
            x[j] = astfem_data[i].radius[j];
         }
         for (j=0; j<astfem_data[i].scan.size(); j++)
         {
            y[j] = new double [astfem_data[i].radius.size()];
         }
         for (j=0; j<astfem_data[i].scan.size(); j++)
         {
            for (k=0; k<astfem_data[i].radius.size(); k++)
            {
               y[j][k] = astfem_data[i].scan[j].conc[k];
            }
         }
         for (j=0; j<astfem_data[i].scan.size(); j++)
         {
            curve[j] = scan_plot->insertCurve("Concentration");
            scan_plot->setCurveData(curve[j], x, y[j], astfem_data[i].radius.size());
            scan_plot->setCurvePen(curve[j], Qt::yellow);
         }
         delete [] x;
         for (j=0; j<astfem_data[i].scan.size(); j++)
         {
            delete [] y[j];
         }
         delete [] y;
      }

      delete [] curve;
      scan_plot->replot();
   }
}


void US_Astfem_Sim::stop_simulation()
{
   if (stopFlag)
   {
      pb_stop_simulation->setText("Stop Simulation");
      stopFlag = false;
   }
   else
   {
      pb_stop_simulation->setText("Start Simulation");
      stopFlag = true;
   }
   astfem_rsa->setStopFlag(stopFlag);
}

void US_Astfem_Sim::dcdt_window()
{
}

void US_Astfem_Sim::save_scans()
{
   QMessageBox mb(tr("UltraScan"), tr("Please choose an export format:"),
   QMessageBox::Information,
   QMessageBox::Yes | QMessageBox::Default,
   QMessageBox::No,
   QMessageBox::Cancel | QMessageBox::Escape);
   mb.setButtonText(QMessageBox::Yes, "UltraScan");
   mb.setButtonText(QMessageBox::No, "XLA");
   mb.setButtonText(QMessageBox::Cancel, tr("Cancel"));


   switch(mb.exec())
   {
      case QMessageBox::Yes: // save in UltraScan format
      {
         QString fn = QFileDialog::getSaveFileName(USglobal->config_list.result_dir, "*.us.v", 0);
         int k=0;
         if ( !fn.isEmpty() )
         {
            while (k != -1)
            {
               k = fn.findRev(".", -1, false);
               if (k != -1) //if an extension was given, strip it
               {
                  fn.truncate(k);
               }
            }
            save_ultrascan(fn);  // the user gave a file name, save in UltraScan format
         }
         return;
         break;
      }
      case QMessageBox::No: // save in XL-A format
      {
         QString fn = QFileDialog::getExistingDirectory(USglobal->config_list.data_dir, 0, 0,
         tr("Please select or create a directory for the simulated data files:"), true, true);
         save_xla(fn);  // the user gave a directory name, save in Beckman/XLA format
         break;
      }
      case QMessageBox::Cancel:
      {
         return;
      }
   }
}

void US_Astfem_Sim::save_xla(const QString &fileName)
{
   float *temp_radius;
   unsigned int i, j, k, pts;
   pts = 30 + (unsigned int) ((simparams.bottom - simparams.meniscus)/simparams.radial_resolution); // add 30 points in front of meniscus
   QString str1, str2;
   QFile f;
   float maxc = 0.0;
   unsigned int total_scans = 0;
   for (k=0; k<simparams.speed_step.size(); k++)
   {
      maxc = max(maxc, (float) astfem_data[k].scan[astfem_data[k].scan.size()-1].conc[astfem_data[k].scan[astfem_data[k].scan.size()-1].conc.size()-1]);
      total_scans += astfem_data[k].scan.size();
   }
   US_ClipData *cd;
   float maxrad = simparams.bottom;
   cd = new US_ClipData(&maxc, &maxrad, simparams.meniscus, total_conc);
   cd->exec();
   delete cd;
   progress->setTotalSteps(total_scans);
   progress->reset();
   temp_radius = new float [pts];
   temp_radius[0] = simparams.meniscus - 30 * simparams.radial_resolution;
   for (i=1; i<30; i++)
   {
      temp_radius[i] = temp_radius[i-1] + simparams.radial_resolution;
   }
   for (i=30; i<pts; i++)
   {
      temp_radius[i] = simparams.meniscus + (simparams.radial_resolution * (i-30));
   }
   lbl_progress->setText("Writing...");
   unsigned int current_scan = 1;
   for (k=0; k<simparams.speed_step.size(); k++)
   {
      for (i=0; i<astfem_data[k].scan.size(); i++)
      {
         if (current_scan < 10)
         {
            str2 = fileName + "/" + "0000" + str1.sprintf("%d", current_scan) + ".ra1";
         }
         else if (current_scan > 9 && current_scan < 100)
         {
            str2 = fileName + "/" + "000" + str1.sprintf("%d", current_scan) + ".ra1";
         }
         else if (current_scan > 99 && current_scan < 1000)
         {
            str2 = fileName + "/" + "00" + str1.sprintf("%d", current_scan) + ".ra1";
         }
         else if (current_scan > 999 && current_scan < 10000)
         {
            str2 = fileName + "/" + "0" + str1.sprintf("%d", current_scan) + ".ra1";
         }
         else if (current_scan > 9999 && current_scan < 100000)
         {
            str2 = fileName + "/" + str1.sprintf("%d", current_scan) + ".ra1";
         }
         else
         {
            return;
         }
         f.setName(str2);
         if (f.open(IO_WriteOnly | IO_Translate))
         {
            QTextStream ts(&f);
            ts << "Simulated Velocity Data" << endl;
//          cout << "k=" << k << ", i=" << i << ": " << astfem_data[k].scan[i].rpm << endl;
            ts << "R 1 20.0 " << str1.sprintf("%5u %7ld %1.5e %d %d\n", astfem_data[k].rpm, (long int) astfem_data[k].scan[i].time, astfem_data[k].scan[i].omega_s_t, 999, 1);
            for (j=0; j<30; j++) // the region in front of the meniscus
            {
               ts << str1.sprintf("%9.4f %12.5e %12.5e", temp_radius[j], 0.0, 0.0) << endl;
            }
            // one high point for the meniscus:
            ts << str1.sprintf("%9.4f %12.5e %12.5e", temp_radius[30], total_conc*1.3, 0.0) << endl;
            j=31;
            if (maxc == 0) // if 0 then use the entire range
            {
               while (j<pts && temp_radius[j] <= maxrad)
               {
                  ts << str1.sprintf("%9.4f %12.5e %12.5e", temp_radius[j], astfem_data[k].scan[i].conc[j-30], 0.0) << endl;
                  j++;
               }
            }
            else
            {
               while (j<pts && astfem_data[k].scan[i].conc[j-30] <= maxc && temp_radius[j] <= maxrad)
               {
                  ts << str1.sprintf("%9.4f %12.5e %12.5e", temp_radius[j], astfem_data[k].scan[i].conc[j-30], 0.0) << endl;
                  j++;
               }
            }
         }
         current_scan++;
         progress->setProgress(current_scan);
         qApp->processEvents();
         f.close();
      }
   }
   progress->setProgress(total_scans);
   lbl_progress->setText("Completed:");
   delete [] temp_radius;
}

void US_Astfem_Sim::save_ultrascan(const QString &filename)
{
   float maxc = 0.0, maxrad = simparams.bottom, plateau, new_bottom;
   unsigned int total_scans = 0, new_points, i, j, k, l;
   for (k=0; k<simparams.speed_step.size(); k++)
   {
      maxc = max(maxc, (float) astfem_data[k].scan[astfem_data[k].scan.size()-1].conc[astfem_data[k].scan[astfem_data[k].scan.size()-1].conc.size()-1]);
      total_scans += astfem_data[k].scan.size();
   }
   US_ClipData *cd;
   cd = new US_ClipData(&maxc, &maxrad, simparams.meniscus, total_conc);
   cd->exec();
   delete cd;
// QMessageBox:message(tr("Additional Information Needed:"), tr("Please pick a point now in the lower plot\nthat best represents the plateau..."));
   progress->setTotalSteps(total_scans);
   progress->reset();
   lbl_progress->setText(tr("Writing..."));
   new_points = astfem_data[0].scan[0].conc.size();
   i=0;
   for (k=0; k<simparams.speed_step.size(); k++)
   {
      i=0;
      while (i<astfem_data[0].scan[0].conc.size() && astfem_data[k].scan[astfem_data[k].scan.size()-1].conc[i] < maxc)
      {           // find the radius from the last scan where the
         i++;     // concentration is higher than the threshold (if at all)
      }
      if (i < new_points)
      {
         new_points = i;
      }
   }
   while (maxrad < astfem_data[0].radius[new_points]) // then check to see if this radius is larger than the maxrad
   {                                   // if it is, then decrease the point count until the max radius
      new_points --;                   // matches.
   }
   new_bottom = astfem_data[0].radius[new_points];
   QString temp_str = filename;
   QString run_file = temp_str.copy();
   QString data_file = temp_str.copy();
   QString scan_file = temp_str.copy();
   QString temp = temp_str.copy();
   int position = temp.findRev("/", -1, false);
   i = temp.length();
   j = i - position - 1;      // -1 because we dont want to count the null terminating character
   QString run_name = temp.right(j);   // run name without leading path
   run_file.append(".us.v");
   data_file.append(".veloc.11");
   scan_file.append(".scn");
   QFile f1(run_file);
   if (f1.exists())
   {
      if(!QMessageBox::query( tr("Warning"), tr("Attention:\nThis file exists already!\n\nDo you want to overwrite it?"), tr("Yes"), tr("No")))
      {
         f1.close();
         return;
      }
   }
   if (f1.open(IO_WriteOnly)) // write the binary run file *.us.v
   {
      QDataStream ds(&f1);
      ds << US_Version;
      ds << USglobal->config_list.data_dir + "/simulation"; // data directory
      int pos = temp_str.findRev("/", -1, false);
      if (pos != -1) //strip the path
      {
         temp_str = temp_str.mid(pos+1, temp_str.length());
      }
      ds << temp_str;
      ds << (float) 20.0;                    // average temperature
      ds << (int) 1;                         // run_inf.temperature_check;
      ds << (float) 0.0;                     // run_inf.time_correction;
      ds << (float) (astfem_data[astfem_data.size()-1].scan[astfem_data[astfem_data.size()-1].scan.size()-1].time);  // run_inf.duration;
      ds << (unsigned int) total_scans;   // for simulated data, total scans = scans for simulation, since only one "cell" is simulated
      ds << (float) simparams.radial_resolution;      // run_inf.delta_r
      ds << (int) -1;                        // experimental data ID
      ds << (int) -1;                        // Investigator ID
      QDate today = QDate::currentDate();
      QString current_date;
      current_date.sprintf( "%d/%d/%d", today.month(), today.day(), today.year() );
      ds << current_date;
      ds << (QString) "Simulated Velocity Data";
      ds << (QString) "ultrascan";
      ds << (QString) "192.168.0.1";
      ds << (QString) "QMYSQL3";
      ds << (int) 1; // run_inf.exp_type.velocity;
      ds << (int) 0; // run_inf.exp_type.equilibrium;
      ds << (int) 0; // run_inf.exp_type.diffusion;
      ds << (int) 1; // run_inf.exp_type.simulation;
      ds << (int) 0; // run_inf.exp_type.interference;
      ds << (int) 1; // run_inf.exp_type.absorbance;
      ds << (int) 0; // run_inf.exp_type.fluorescence;
      ds << (int) 0; // run_inf.exp_type.intensity;
      ds << (int) 0; // run_inf.exp_type.wavelength;
      for (i=0; i<8; i++)
      {
         if (i == 0)
         {
            ds << (int) 0; //centerpiece = simulation cell is zero
            ds << (float) simparams.meniscus;
            QString temp_string = QString(tr("Simulated Data - see corresponding model"));   // data discription
            ds << temp_string;
            ds << (unsigned int) 1; //how many wavelengths?
         }
         else
         {
            ds << (unsigned int) 0; //centerpiece = conventional 2 channel epon
            ds << (float) 0.0;
            QString temp_string = QString("");  // data description
            ds << temp_string;
            ds << (unsigned int) 0;
         }
      }
      int serial_number = -1;
      for (i=0; i<8; i++)
      {
         for (j=0; j<4; j++)
         {
            ds << serial_number; //buffer serial number
            for(k=0; k<3; k++)
            {
               ds << serial_number; //peptide serial number
               ds << serial_number; //DNA serial number
            }
         }
         for (j=0; j<3; j++)
         {
            if (i == 0 && j == 0)
            {
               ds << (unsigned int) 999;  // run_inf.wavelength[i][j]
               ds << (unsigned int) total_scans;
               ds << (float) 0.0;   // baseline
               ds << (float) simparams.meniscus;
               ds << (float) new_bottom;
               ds << (unsigned int) new_points;
               ds << (float) (new_bottom - simparams.meniscus)/new_points;
            }
            else
            {
               ds << (unsigned int) 0;
               ds << (unsigned int) 0;
               ds << (float) 0.0;   // baseline
               ds << (float) 0.0;
               ds << (float) 0.0;
               ds << (unsigned int) 0;
               ds << (float) 0.0;
            }
         }
      }
      for (i=0; i<8; i++)
      {
         for (k=0; k<simparams.speed_step.size(); k++)
         {
            for (j=0; j<astfem_data[k].scan.size(); j++)
            {
               plateau = 0.0;
               for (l=0; l<system.component_vector.size(); l++)
               {
               //
// This is the equation for radial dilution:
               //
                  plateau += system.component_vector[l].concentration * exp(-2.0 * system.component_vector[l].s
                        * astfem_data[k].scan[j].omega_s_t);
               }
               if (i == 0)
               {
                  ds << (uint) astfem_data[k].rpm;
                  ds << (float) 20.0;     // temperature
                  ds << (unsigned int) astfem_data[k].scan[j].time;
                  ds << (float) astfem_data[k].scan[j].omega_s_t;
//cerr << "cell: " << i+1 << ", speed step: " << k+1 << ", scan: " << j+1 << ", plateau: " << plateau << " (time, omega, conc, s1, s2: " << astfem_data[k].scan[j].time << ", " << astfem_data[k].scan[j].omega_s_t << ", " <<  system.component_vector[0].concentration + system.component_vector[1].concentration << ", " << system.component_vector[0].s << ", " << system.component_vector[1].s << ")" << endl;
                  ds << (float) (1.0);//plateau;
               }
               else
               {
                  ds << (uint) 0;
                  ds << (float) 0.0;
                  ds << (unsigned int) 0.0;
                  ds << (float) 0.0;
                  ds << (float) 0.0;
               }
            }
         }
      }
      ds << (int) -1;   // run_inf.rotor;
      f1.flush();
      f1.close();
   }
   QFile f2(data_file);
   if (f2.open(IO_WriteOnly)) // write the binary scan data file *.veloc.11
   {
      QDataStream ds2(&f2);
      for (k=0; k<simparams.speed_step.size(); k++)
      {
         for (j=0; j<astfem_data[k].scan.size(); j++)
         {
            for (i=0; i<new_points; i++)
            {
               ds2 << (float) astfem_data[k].scan[j].conc[i];
            }
         }
      }
      f2.flush();
      f2.close();
   }
}

void US_Astfem_Sim::update_time(float fval)
{
   lcd_time->display(fval);
}

void US_Astfem_Sim::update_speed(unsigned int ival)
{
   lcd_speed->display((int) ival);
}

void US_Astfem_Sim::update_movieFlag()
{
   movieFlag = cb_movie->isChecked();
}

void US_Astfem_Sim::update_time_correctionFlag()
{
   time_correctionFlag = cb_time_correction->isChecked();
}

void US_Astfem_Sim::update_time_correctionFlag()
{
   time_correctionFlag = cb_time_correction->isChecked();
}

#endif
