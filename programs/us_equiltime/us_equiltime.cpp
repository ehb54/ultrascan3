//! file us_equiltime.cpp
#include "us_equiltime.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_constants.h"
#include "us_math2.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_model_gui.h"
//#include "us_model_editor.h"

//! \brief Main program for US_Equilspeed. Loads translators and starts
//         the class US_Equilspeed.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_EquilTime w;
   w.show();                   // Member of QWidget
   return application.exec();  // Member of QApplication
}

US_EquilTime::US_EquilTime() : US_Widgets( true )
{
   astfem_rsa = new US_Astfem_RSA( model, simparams );
   
   connect( astfem_rsa, SIGNAL( new_scan   ( QVector< double >*, double* ) ),
                        SLOT(   check_equil( QVector< double >*, double* ) ) );
   
   connect( astfem_rsa, SIGNAL( new_time( double ) ),
                        SLOT(   set_time( double ) ) );
   
   setWindowTitle( tr( "Equilibrium Time Prediction" ) );
   setPalette( US_GuiSettings::frameColor() );

   init_simparams();

   QBoxLayout* main = new QHBoxLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   // Left Column
   QGridLayout* left = new QGridLayout;
   int row = 0;

   QLabel* lb_sample = us_banner( tr( "Model Settings" ) ); 
   left->addWidget( lb_sample, row++, 0, 1, 2 );

   QGridLayout* buttons1 = new QGridLayout;
   int b_row = 0;

   QPushButton* pb_loadExperiment = us_pushbutton( tr( "Load Experiment") );
   connect ( pb_loadExperiment, SIGNAL( clicked        () ), 
                                SLOT  ( load_experiment() ) );
   buttons1->addWidget( pb_loadExperiment, b_row, 0 );

   pb_saveExp = us_pushbutton( tr( "Save Experiment"), false );
   buttons1->addWidget( pb_saveExp, b_row++, 1 );

   /*
   QPushButton* pb_new = us_pushbutton( tr( "New Model") );
   connect( pb_new, SIGNAL( clicked() ), SLOT( new_model() ) );
   buttons1->addWidget( pb_new, b_row, 0 );

   QPushButton* pb_loadModel = us_pushbutton( tr( "Load Model") );
   connect( pb_loadModel, SIGNAL( clicked() ), SLOT( load_model() ) );
   buttons1->addWidget( pb_loadModel, b_row++, 1 );
   */
   pb_changeModel = us_pushbutton( tr( "Set / Change / Review Model") );
   connect ( pb_changeModel, SIGNAL( clicked() ) , SLOT( change_model() ) );
   buttons1->addWidget( pb_changeModel, b_row++, 0, 1, 2 );

   left->addLayout( buttons1, row, 0, 3, 2 );
   row += 3;


   QPalette p;
   p.setColor( QPalette::WindowText, Qt::white );
   p.setColor( QPalette::Shadow    , Qt::white );
   
   QFont font( US_GuiSettings::fontFamily(),
               US_GuiSettings::fontSize(),
               QFont::Bold );

   // Radius Info
   QLabel* lb_radius = us_banner( tr( "Radius Settings" ) ); 
   left->addWidget( lb_radius, row++, 0, 1, 2 );
   
   QGroupBox* channelGroupBox = new QGroupBox( tr( "Channel Type" ) );
   channelGroupBox->setContentsMargins ( 2, 2, 2, 2 );
   channelGroupBox->setPalette( p );
   channelGroupBox->setFont   ( font );
   
   QRadioButton* rb_inner;
   QRadioButton* rb_outer;
   QRadioButton* rb_center;
   QRadioButton* rb_custom;

   QGridLayout* rb5 = us_radiobutton( tr( "Inner Channel"  ), rb_inner, true );
   QGridLayout* rb6 = us_radiobutton( tr( "Outer Channel"  ), rb_outer  );
   QGridLayout* rb7 = us_radiobutton( tr( "Center Channel" ), rb_center );
   QGridLayout* rb8 = us_radiobutton( tr( "Custom"         ), rb_custom );

   // Group the buttons
   QButtonGroup* channelGroup = new QButtonGroup;
   channelGroup->addButton( rb_inner , INNER  );
   channelGroup->addButton( rb_outer , OUTER  );
   channelGroup->addButton( rb_center, CENTER );
   channelGroup->addButton( rb_custom, CUSTOM );
   connect( channelGroup, SIGNAL( buttonClicked( int ) ),
                          SLOT  ( new_channel  ( int ) ) );

   current_position = INNER;

   QGridLayout* channel = new QGridLayout;
   channel->setContentsMargins ( 2, 2, 2, 2 );
   channel->setSpacing( 0 );
   channel->addLayout( rb5, 0, 0 );
   channel->addLayout( rb6, 0, 1 );
   channel->addLayout( rb7, 1, 0 );
   channel->addLayout( rb8, 1, 1 );

   channelGroupBox->setLayout( channel );

   left->addWidget( channelGroupBox, row, 0, 2, 2 );
   row += 2;

   // Top Radius
   QLabel* lb_top = us_label( tr( "Top Radius:" ) );
   left->addWidget( lb_top, row, 0 );

   cnt_top = us_counter( 3, 5.8, 7.3, 5.9 );
   cnt_top->setStep    ( 0.01 );
   cnt_top->setEnabled ( false );
   left->addWidget( cnt_top, row++, 1 );

   // Bottom Radius
   QLabel* lb_bottom = us_label( tr( "Bottom Radius:" ) );
   left->addWidget( lb_bottom, row, 0 );

   cnt_bottom = us_counter( 3, 5.8, 7.3, 6.2 );
   cnt_bottom->setStep    ( 0.01 );
   cnt_bottom->setEnabled ( false );
   left->addWidget( cnt_bottom, row++, 1 );

   // Rotorspeed Info

   QLabel* lb_rotor = us_banner( tr( "Rotorspeed Settings" ) ); 
   left->addWidget( lb_rotor, row++, 0, 1, 2 );
   
   // Speed type buttons
   QGroupBox* rotor  = new QGroupBox( tr( "Speed Type" ) );
   rotor->setContentsMargins ( 2, 10, 2, 2 );
   rotor->setPalette( p );
   rotor->setFont   ( font );
   
   QRadioButton* rb_sigma;
   QRadioButton* rb_rpm;

   QGridLayout* rb9  = us_radiobutton( "Use Sigma", rb_sigma, true );
   QGridLayout* rb10 = us_radiobutton( "Use RPM"  , rb_rpm );

   speed_type  = SIGMA;
   sigma_start = 1;
   sigma_stop  = 4;
   rpm_start   = 18000;
   rpm_stop    = 36000;
 
   speed_count = 5;

   QButtonGroup* speedGroup = new QButtonGroup;
   speedGroup->addButton( rb_sigma, SIGMA );
   speedGroup->addButton( rb_rpm  , RPM   );
   connect( speedGroup,  SIGNAL( buttonClicked( int ) ), 
                         SLOT  ( update_speeds( int ) ) );

   QGridLayout* speedType = new QGridLayout;
   speedType->setContentsMargins( 2, 2, 2, 2 );
   speedType->setSpacing        ( 0 );

   speedType->addLayout( rb9,  0, 0 );
   speedType->addLayout( rb10, 0, 1 );

   rotor->setLayout( speedType );

   left->addWidget( rotor, row++, 0, 1, 2 );

   // Low speed
   lb_lowspeed   = us_label( tr( "Low Speed (sigma):"  ) );
   left->addWidget( lb_lowspeed, row, 0 );
   
   cnt_lowspeed = us_counter( 3, 0.01, 10.0, sigma_start );
   cnt_lowspeed->setStep( 0.01 );
   left->addWidget( cnt_lowspeed, row++, 1 );
   connect( cnt_lowspeed, SIGNAL( valueChanged( double ) ),
                          SLOT  ( new_lowspeed( double ) ) );

   // High speed
   lb_highspeed  = us_label( tr( "High Speed (sigma):" ) );
   left->addWidget( lb_highspeed, row, 0 );
   
   cnt_highspeed = us_counter( 3, 0.01, 10.0, sigma_stop );
   cnt_highspeed->setStep( 0.01 );
   left->addWidget( cnt_highspeed, row++, 1 );
   connect( cnt_highspeed, SIGNAL( valueChanged ( double ) ),
                           SLOT  ( new_highspeed( double ) ) );

   // Speed steps
   QLabel* lb_speedsteps = us_label( tr( "Speed Steps:"        ) );
   left->addWidget( lb_speedsteps, row, 0 );
   
   cnt_speedsteps = us_counter( 3, 1.0, 100.0, speed_count );
   cnt_speedsteps->setStep( 1.0 );
   left->addWidget( cnt_speedsteps, row++, 1 );
   connect( cnt_speedsteps, SIGNAL( valueChanged ( double ) ),
                            SLOT  ( new_speedstep( double ) ) );

   // Speed list
   QLabel* lb_speedlist  = us_label( tr( "Current Speed List:" ) );
   left->addWidget( lb_speedlist, row, 0 );

   te_speedlist = us_textedit(); 
   te_speedlist->setReadOnly( true );

   left->addWidget( te_speedlist, row, 1, 3, 1 );

   left->setRowStretch ( row + 1, 99 );
   row += 3;

   // Misc Info
   QLabel* lb_sim2  = us_banner( tr( "Simulation Settings" ) );
   left->addWidget( lb_sim2, row++, 0, 1, 2 );

   // Tolerance
   QLabel* lb_tolerance  = us_label( tr( "Tolerance:" ) );
   left->addWidget( lb_tolerance, row, 0 );
   
   cnt_tolerance = us_counter( 3, 1.0e-5, 0.01, 0.0005 );
   cnt_tolerance->setStep( 1.0e-5 );
   left->addWidget( cnt_tolerance, row++, 1 );

   // Time increment
   QLabel* lb_time  = us_label( tr( "Time Increment (min):" ) );
   left->addWidget( lb_time, row, 0 );
   
   cnt_timeIncrement = us_counter( 3, 1.0, 1000.0, 15.0 );
   cnt_timeIncrement->setStep( 1.0 );
   left->addWidget( cnt_timeIncrement, row++, 1 );

   QGridLayout* monitor = 
      us_checkbox( tr( "Monitor Simulation Progress" ), cb_monitor, true );

   left->addLayout( monitor, row++, 0, 1, 2 );

   QGridLayout* buttons2 = new QGridLayout;
   b_row = 0;

   pb_estimate = us_pushbutton( tr( "Estimate Times" ) );
   pb_estimate->setEnabled( false );
   connect( pb_estimate, SIGNAL( clicked() ), SLOT( simulate() ) );
   buttons2->addWidget( pb_estimate, b_row++, 0, 1, 2 ); 

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons2->addWidget( pb_help, b_row, 0 ); 

   QPushButton* pb_close = us_pushbutton( tr( "Close" ) );
   connect( pb_close, SIGNAL( clicked() ), SLOT( close() ) );
   buttons2->addWidget( pb_close, b_row++, 1 ); 

   left->addLayout( buttons2, row, 0, 2, 2 );

   main->addLayout( left ); 

   // Right Column

   // Simulation plot

   QBoxLayout* right = new QVBoxLayout;
   
   QBoxLayout* plot = new US_Plot( equilibrium_plot, 
         tr( "Approach to Equilibrium Simulation" ),
         tr( "Radius" ), tr( "Concentration" ) );
   us_grid( equilibrium_plot );
   
   equilibrium_plot->setMinimumSize( 600, 400 );
   equilibrium_plot->setAxisScale( QwtPlot::yLeft  , 0.0, 1.5 );
   equilibrium_plot->setAxisScale( QwtPlot::xBottom, 5.9, 6.2 );

   right->addLayout( plot );

   te_info = new US_Editor( 0, true );

   QFontMetrics fm( te_info->font() );
   te_info->setFixedHeight( fm.height() * 15 );

   right->addWidget( te_info );
   right->setStretchFactor( plot   , 10 );
   right->setStretchFactor( te_info, 2 );

   main->addLayout( right ); 

   model.components.clear();
   update_speeds( speed_type );
}

void US_EquilTime::new_lowspeed( double speed )
{
   if ( speed > cnt_highspeed->value() )
   {
      cnt_lowspeed->setValue( cnt_highspeed->value() );
      
      QMessageBox::warning( this,
        tr( "Warning" ),
        tr( "The low speed value cannot exceed the high speed value" ) );
      return;
   }

   if ( speed_type == SIGMA )
   {
      sigma_start = speed;
      if ( fabs( sigma_stop - sigma_start ) < 1.0e-4 )
      {
         speed_count = 1;
         cnt_speedsteps->setValue( speed_count );
      }
   }
   else
   {
      rpm_start = speed;
      if ( fabs( rpm_stop - rpm_start ) < 100.0 )
      {
         speed_count = 1;
         cnt_speedsteps->setValue( speed_count );
      }
   }

   update_speeds( speed_type );
}

void US_EquilTime::new_highspeed( double speed )
{
   if ( speed < cnt_lowspeed->value() )
   {
      cnt_highspeed->setValue( cnt_lowspeed->value() );

      QMessageBox::warning( this,
        tr( "Warning" ),
        tr( "The high speed value cannot be less than the low speed value" ) );
      return;
   }

   if ( speed_type == SIGMA )
   {
      sigma_stop = speed;
      if ( fabs( sigma_stop - sigma_start ) < 1.0e-4 )
      {
         speed_count = 1;
         cnt_speedsteps->setValue( speed_count );
      }
   }
   else
   {
      rpm_stop = speed;
      if ( fabs( rpm_stop - rpm_start ) < 100.0 )
      {
         speed_count = 1;
         cnt_speedsteps->setValue( speed_count );
      }
   }

   update_speeds( speed_type );
}

void US_EquilTime::new_speedstep( double count )
{
   speed_count = (int) count ;
   update_speeds( speed_type );
}

void US_EquilTime::new_channel( int channel )
{

   cnt_top   ->setEnabled( false );
   cnt_bottom->setEnabled( false );
   
   switch ( channel )
   {
      case INNER:
         cnt_top   ->setValue( 5.9 );
         cnt_bottom->setValue( 6.2 );
         equilibrium_plot->setAxisScale( QwtPlot::xBottom, 5.9, 6.2 );
         break;

      case OUTER:
         cnt_top   ->setValue( 6.9 );
         cnt_bottom->setValue( 7.2 );
         equilibrium_plot->setAxisScale( QwtPlot::xBottom, 6.9, 7.2 );
         break;

      case CENTER:
         cnt_top   ->setValue( 6.4 );
         cnt_bottom->setValue( 7.2 );
         equilibrium_plot->setAxisScale( QwtPlot::xBottom, 6.4, 7.2 );
         break;

      case CUSTOM:
         cnt_top   ->setEnabled( true );
         cnt_bottom->setEnabled( true );
         equilibrium_plot->setAxisScale( QwtPlot::xBottom, 5.8, 7.3 );
         break;
   }

   equilibrium_plot->replot();
}

double US_EquilTime::rpmFromSigma( double sigma )
{
   double T     = K0 + 20.0;  // 20C for now
   double mw    = model.components[ 0 ].mw;
   double vbar  = model.components[ 0 ].vbar20;
   double rho   = DENS_20W; //model.density;

   double rpm   = 30.0 / M_PI * 
                  sqrt( sigma * R * T * 2 / ( mw * ( 1 - vbar * rho ) ) );
   
   rpm = floor( rpm / 100.0 + 0.5 ) * 100.0;  // Round to closest 100
   
   return rpm;
}

double US_EquilTime::sigmaFromRpm( double rpm )
{
   /* Sigma is a measure of the curvature of the exponential.
    * If it is too small, the curvature is shallow and there is not 
    * enough information. If it is too steep, most of the concentration
    * points are going to be near zero, drowned out by bad s/n ratios.
    * The happy medium is between 1 < sigma < 4, as far as equilibrium
    * exponents are concerned. It's a convenient way to parameterize 
    * curvature, which is a function of rotor speed and molecular weight. 
    */

   double T     = K0 + 20.0;  // 20C for now
   double mw    = model.components[ 0 ].mw;
   double vbar  = model.components[ 0 ].vbar20;
   double rho   = DENS_20W; //model.density;

   double sigma = mw * ( 1 - vbar * rho ) * sq( M_PI / 30.0 * rpm ) / 
                  ( 2 * R * T );

   return sigma;
}

void US_EquilTime::update_speeds( int type )
{
   speed_steps.clear();
   te_speedlist->clear();

   if ( type == SIGMA )
   {
      // Determine max sigma
      double max_sigma;
      
      if ( model.components.size() > 0 ) 
      {
         max_sigma = sigmaFromRpm( 60000.0 );

         if ( type != speed_type )
         {
            sigma_start = sigmaFromRpm( cnt_lowspeed ->value() );
            sigma_stop  = sigmaFromRpm( cnt_highspeed->value() );
         }
      }
      else 
         max_sigma = 10;

      if ( sigma_start > max_sigma ) sigma_start = max_sigma;
      if ( sigma_stop  > max_sigma ) sigma_stop  = max_sigma;

      lb_lowspeed ->setText( tr( "Low Speed (sigma):"  ) );
      lb_highspeed->setText( tr( "High Speed (sigma):" ) );
      
      // Reset counters
      cnt_lowspeed ->disconnect();
      cnt_highspeed->disconnect();
      cnt_lowspeed ->setRange( 0.1, max_sigma, 0.01 );
      cnt_lowspeed ->setValue( sigma_start );
      cnt_highspeed->setRange( 0.1, max_sigma, 0.01 );
      cnt_highspeed->setValue( sigma_stop );
      
      connect( cnt_lowspeed, SIGNAL( valueChanged( double ) ),
                             SLOT  ( new_lowspeed( double ) ) );

      connect( cnt_highspeed, SIGNAL( valueChanged ( double ) ),
                              SLOT  ( new_highspeed( double ) ) );

      if ( fabs( sigma_stop - sigma_start ) < 0.1 ) speed_count = 1;
      cnt_speedsteps->setValue( speed_count );

      if ( speed_count > 1 )
      {
         double increment = ( sigma_stop - sigma_start ) / ( speed_count - 1 );
         
         for ( int i = 0; i < speed_count; i++ )
         {
            speed_steps <<  sigma_start + i * increment;
            te_speedlist->append( QString::number( i + 1 ) + ": sigma = " + 
                             QString::number( speed_steps[ i ], 'f', 3 ) );
         }
      }
      else
      {
         speed_steps <<  sigma_start;
         te_speedlist->append( "1: sigma = " + 
               QString::number( sigma_start, 'f', 3 ) );
      }
   }

   else
   {
      if ( model.components.size() > 0 &&  type != speed_type )
      {
         rpm_start = rpmFromSigma( cnt_lowspeed ->value() );
         rpm_stop  = rpmFromSigma( cnt_highspeed->value() );
      }

      lb_lowspeed ->setText( tr( "Low Speed (rpm):"  ) );
      lb_highspeed->setText( tr( "High Speed (rpm):" ) );
      
      // Reset counters
      cnt_lowspeed ->disconnect();
      cnt_highspeed->disconnect();

      cnt_lowspeed  ->setRange( 100, 60000, 100 );
      cnt_lowspeed  ->setValue( rpm_start );
      cnt_highspeed ->setRange( 100, 60000, 100 );
      cnt_highspeed ->setValue( rpm_stop );
      
      connect( cnt_lowspeed, SIGNAL( valueChanged( double ) ),
                             SLOT  ( new_lowspeed( double ) ) );

      connect( cnt_highspeed, SIGNAL( valueChanged ( double ) ),
                              SLOT  ( new_highspeed( double ) ) );

      if ( fabs( rpm_stop - rpm_start ) < 100.0 ) speed_count = 1;
      cnt_speedsteps->setValue( speed_count );

      if ( speed_count > 1 )
      {
         double increment = ( rpm_stop - rpm_start ) / ( speed_count - 1 );
         
         for ( int i = 0; i < speed_count; i++ )
         {
            double rpm = rpm_start + i * increment;
            rpm = floor( rpm / 100.0 + 0.5 ) * 100.0;  // Round to closest 100
            speed_steps <<  rpm;
            te_speedlist->append( QString::number( i + 1 ) + ": rpm = " + 
                             QString::number( rpm ) );
         }
      }
      else
      {
         speed_steps <<  rpm_start;
         te_speedlist->append( "1: rpm = " + 
               QString::number( rpm_start ) );
      }
   }

   speed_type = type;
}

void US_EquilTime::load_experiment( void )
{
/*
   QString fn = QFileDialog::getOpenFileName( this,
                  tr( "Select Experiment File" ),
                  US_Settings::resultDir(), "*.us_system" );

   if ( ! fn.isEmpty() )
   {
      //FIXME
      int error_code = 0;
      //US_FemGlobal::read_experiment( model, simparams, fn );

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
         pb_changeModel->setEnabled( true );
         pb_saveExp    ->setEnabled( true );
         
         QMessageBox::information( this,
            tr( "Simulation Module" ),
            tr( "Successfully loaded Model:\n\n" ) + model.description );
      }
   }

   // Initialize.  Can be updated in model editor
   //model.density = DENS_20W;
   update_speeds( speed_type );
*/
}

void US_EquilTime::new_model( void )
{
//   model = US_Model();
//   change_model();
}

void US_EquilTime::change_model( void )
{
   US_ModelGui* dialog = new US_ModelGui( model );
   connect( dialog, SIGNAL( valueChanged( US_Model ) ), 
                    SLOT  ( set_model   ( US_Model ) ) );
   dialog->exec();
}

void US_EquilTime::set_model( US_Model m )
{
   model = m;

   pb_saveExp    ->setEnabled( true );
   pb_estimate   ->setEnabled( true );
   pb_changeModel->setEnabled( true ); 
   pb_estimate   ->setEnabled( true );
   update_speeds( speed_type );
}


void US_EquilTime::load_model( void )
{
/*  QString fn = QFileDialog::getOpenFileName( this, 
         tr( "Select the model to load" ),
         US_Settings::resultDir(), "*.model.?? *.model-?.?? *model-??.??" );

   if ( fn.isEmpty() ) return;

   int error_code = US_FemGlobal::read_modelSystem( model, fn );

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
      pb_saveExp    ->setEnabled( true );
      pb_changeModel->setEnabled( true );
      pb_estimate   ->setEnabled( true );

      QMessageBox::information( this,
            tr( "Simulation Module" ),
            tr( "Successfully loaded model:\n\n" ) + model.description );
   }

   // Initialize.  Can be updated in model editor
   //model.density = DENS_20W;
   update_speeds( speed_type );
*/
}

void US_EquilTime::init_simparams( void )
{
   simparams.speed_step.clear();

   simparams.speed_step << US_SimulationParameters::SpeedProfile();

   // These are the only changes from the constructor
   simparams.speed_step[ 0 ].duration_hours    = 100;
   simparams.speed_step[ 0 ].rotorspeed        = 100;  // Updated before use
   simparams.speed_step[ 0 ].scans             = 2;
   simparams.speed_step[ 0 ].acceleration_flag = false;
}

void US_EquilTime::init_astfem_data( void )
{
   int ss_size = simparams.speed_step.size();

   for ( int i = 0; i < ss_size; i++ )
   {
      US_SimulationParameters::SpeedProfile* sp = &simparams.speed_step[ i ];

      astfem_data.scanData.clear();
      astfem_data.x       .clear();

      astfem_data.cell    = 1;
      astfem_data.channel = 1;

      US_DataIO2::Scan scan;

      // Assign radius for the i'th dataset:
      double r = simparams.meniscus;

      while  ( r <= simparams.bottom )
      {
         astfem_data.x << US_DataIO2::XValue( r );
         r += simparams.radial_resolution;
      }

      int radius_points = astfem_data.x.size();

      astfem_data.description  = "Simulated data set " +
         QString::number( i + 1 ) + " (of " +
         QString::number( ss_size ) + ") for speed " +
         QString::number( sp->rotorspeed );

      for ( int j = 0; j < sp->scans; j++ )
      {
         // Contant temperature for now.  A temperature counter could be added
         // the this program.
        
         scan.temperature = 20.0;
         scan.rpm         = sp->rotorspeed;
         scan.wavelength  = 999; 

         scan.readings.fill( 0.0, radius_points );

         astfem_data.scanData << scan;
      }
   }
}

void US_EquilTime::simulate( void )
{
   for ( int k = 0 ; k < model.components.size(); k++ )
   {
      US_Model::MfemInitial* c0 = &model.components[ k ].c0;
      
      c0->radius       .clear();
      c0->concentration.clear();
   }

   simparams.meniscus = cnt_top   ->value();
   simparams.bottom   = cnt_bottom->value();

   // Handle case of custom channel type
   equilibrium_plot->setAxisScale( QwtPlot::xBottom, 
         simparams.meniscus, simparams.bottom );

   equilibrium_plot->clear();
   equilibrium_plot->replot();
   current_time   = 0.0;
   concentration.clear();
   current_curve  = NULL;

   astfem_rsa->set_movie_flag( true  );

   init_astfem_data();

   te_info->e->append( tr( "Sigma   RPM     Time Increment  Total Time\n" ) );

   for ( int i = 0; i < speed_steps.count(); i++ )
   {
      astfem_rsa->setStopFlag( false );

      // Set up simparams for this step
      if ( speed_type == SIGMA )
      {
         double sigma                         =  speed_steps[ i ];
         simparams.speed_step[ 0 ].rotorspeed = (int) rpmFromSigma( sigma ); 
      }
      else
         simparams.speed_step[ 0 ].rotorspeed = (int) speed_steps[ i ];

      // Setup this curve
      current_curve =
         new QwtPlotCurve( "Step Number " + QString::number( i ) );
      
      current_curve->setPen( QPen( Qt::green ) );
      current_curve->attach( equilibrium_plot );

      // Set up for next step
      astfem_data.scanData[ 0 ].seconds = 0.0;

      next_scan_time = cnt_timeIncrement->value() * 60.0; 
      step_time      = 0.0;

      // Do the simulation
      astfem_rsa->calculate( astfem_data );

      current_time += step_time;

      // Copy last scan data to initial concentration

      for ( int k = 0 ; k < model.components.size(); k++ )
      {
         US_Model::MfemInitial* c0 = &model.components[ k ].c0;
         
         c0->radius       .clear();
         c0->concentration.clear();

         for ( int j = 0; j < simparams.simpoints; j++ )
         {
            c0->radius        << sim_radius   [ j ];
            c0->concentration << concentration[ j ];
         }
      }

      concentration.clear();  // Force allocation of new plot data
      
      // Draw curve 
      current_curve->setPen( QPen( Qt::red ) );
      equilibrium_plot->replot();
      
      // Output results
      int    rpm   = simparams.speed_step[ 0 ].rotorspeed;
      double sigma = sigmaFromRpm( rpm );
      
      QString results;
      results.sprintf( "%6.4f  %5d   %6.2f hours    %6.2f hours", 
            sigma, rpm, step_time / 3600.0, current_time / 3600.0 );

      te_info->e->append( results );
   }

   te_info->e->append( 
         tr( "(Note: All speeds have been adjusted to be rounded to "
             "the nearest 100 RPM.)\n" ) +
             "______________________________________________________"
             "_____________________\n" );
}

void US_EquilTime::check_equil( QVector< double >* x, double* c )
{
   // If we are not at the time increment, return
   if ( step_time < next_scan_time ) return;

   // If first scan, just copy concentrations
   if ( concentration.isEmpty() )
   {
      radius_points = x->size();
      sim_radius.resize( radius_points );
      concentration.resize( radius_points );

      for ( int i = 0; i < radius_points; i++ ) 
      {
         sim_radius   [ i ] = (*x)[ i ];
         concentration[ i ] = c   [ i ];
      }
      return;
   }

   // Determine the scan concentration differences
   double diffs;

   for ( int i = 0; i < radius_points; i++ ) 
      diffs += fabs( concentration[ i ] - c[ i ] );

   // If within tolerance stop the simulation, otherwise copy the concentrations
   if ( diffs < cnt_tolerance->value() )
   {
      astfem_rsa->setStopFlag( true );

      if ( ! cb_monitor->isChecked() )
      {
         current_curve->setData( sim_radius.data(), concentration.data(), 
                                 radius_points );
         equilibrium_plot->replot();
      }
   }
   else
      for ( int i = 0; i < radius_points; i++ ) concentration[ i ] = c[ i ];

   // If requested, draw progress curve
   if ( cb_monitor->isChecked() )
   {
      current_curve->setData( sim_radius.data(), concentration.data(), 
                              radius_points );
      equilibrium_plot->replot();
   }

   next_scan_time += cnt_timeIncrement->value() * 60.0;
}

