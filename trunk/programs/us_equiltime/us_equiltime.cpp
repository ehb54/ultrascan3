//! \file us_equiltime.cpp
#include "us_equiltime.h"
#include "us_gui_settings.h"
#include "us_constants.h"

US_EquilTime::US_EquilTime() : US_Widgets( true )
{
   setWindowTitle( tr( "Equilibrium Time Prediction" ) );
   setPalette( US_GuiSettings::frameColor() );

   QBoxLayout* main = new QHBoxLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   // Left Column
   QGridLayout* left = new QGridLayout;
   int row = 0;

   QLabel* lb_sample     = us_banner( tr( "Simulation Settings" ) ); 
   left->addWidget( lb_sample, row++, 0, 1, 2 );

   QPushButton* pb_simulation = us_pushbutton( tr( "Simulate Component" ) );
   connect( pb_simulation, SIGNAL( clicked() ), SLOT( simulate_component() ) );
   left->addWidget( pb_simulation, row++, 0, 1, 2 );

   QPalette p;
   p.setColor( QPalette::WindowText, Qt::white );
   p.setColor( QPalette::Shadow    , Qt::white );
   
   QFont font( US_GuiSettings::fontFamily(),
               US_GuiSettings::fontSize(),
               QFont::Bold );

   // Shape
   QGroupBox* shape = new QGroupBox( tr( "Shape" ) ); 
   shape->setContentsMargins ( 2, 2, 2, 2 );
   shape->setPalette( p );
   shape->setFont( font );

   // Shape radio buttons
   QGridLayout* rb1 = 
      us_radiobutton( tr( "Prolate Ellipsoid" ), rb_prolate, true );
   
   QGridLayout* rb2 = us_radiobutton( tr( "Oblate Ellipsoid"  ), rb_oblate );
   QGridLayout* rb3 = us_radiobutton( tr( "Long Rod"          ), rb_rod    );
   QGridLayout* rb4 = us_radiobutton( tr( "Sphere"            ), rb_sphere );

   current_shape = PROLATE;

   QButtonGroup* shapeGroup = new QButtonGroup;
   shapeGroup->addButton( rb_prolate, PROLATE  );
   shapeGroup->addButton( rb_oblate , OBLATE  );
   shapeGroup->addButton( rb_rod    , ROD      );
   shapeGroup->addButton( rb_sphere , SPHERE   );
   connect( shapeGroup,  SIGNAL( buttonClicked( int ) ), 
                         SLOT  ( new_shape    ( int ) ) );

   rb_prolate->setEnabled( false );
   rb_oblate ->setEnabled( false );
   rb_rod    ->setEnabled( false );
   rb_sphere ->setEnabled( false );

   QPalette pal = US_GuiSettings::normalColor();

   QGridLayout* shapeBox = new QGridLayout;          // Grid
   shapeBox->setContentsMargins ( 2, 2, 2, 2 );
   shapeBox->setSpacing( 0 );

   shapeBox->addLayout( rb1, 0, 0 );
   shapeBox->addLayout( rb2, 0, 1 );
   shapeBox->addLayout( rb3, 1, 0 );
   shapeBox->addLayout( rb4, 1, 1 );

   shape->setLayout( shapeBox );

   left->addWidget( shape, row, 0, 2, 2 );
   row += 2;
  
   // Molecular Weight
   QLabel* lb_mw = us_label( tr( "Molecular Weight:" ) );
   left->addWidget( lb_mw, row, 0 );

   le_mw = us_lineedit( tr( "<not selected>" ) );
   left->addWidget( le_mw, row++, 1 );

   // Sedimentation Coefficient
   QLabel* lb_sed = us_label( tr( "Sedimentation Coeff:" ) );
   left->addWidget( lb_sed, row, 0 );

   le_sed = us_lineedit( tr( "<not selected>" ) );
   left->addWidget( le_sed, row++, 1 );

   // Diffusion Coefficient
   QLabel* lb_diff = us_label( tr( "Diffusion Coeff:" ) );
   left->addWidget( lb_diff, row, 0 );

   le_diff = us_lineedit( tr( "<not selected>" ) );
   left->addWidget( le_diff, row++, 1 );


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
   cnt_top->setStep( 0.01 );
   left->addWidget( cnt_top, row++, 1 );

   // Bottom Radius
   QLabel* lb_bottom = us_label( tr( "Bottom Radius:" ) );
   left->addWidget( lb_bottom, row, 0 );

   cnt_bottom = us_counter( 3, 5.8, 7.3, 6.2 );
   cnt_bottom->setStep( 0.01 );
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
   QLabel* lb_time  = us_label( tr( "Tine Increment (min):" ) );
   left->addWidget( lb_time, row, 0 );
   
   cnt_timeIncrement = us_counter( 3, 1.0, 1000.0, 15.0 );
   cnt_timeIncrement->setStep( 1.0 );
   left->addWidget( cnt_timeIncrement, row++, 1 );

   // Delta-r
   QLabel* lb_delta_r  = us_label( tr( "Delta-r (cm):" ) );
   left->addWidget( lb_delta_r, row, 0 );
   
   cnt_delta_r = us_counter( 3, 0.0001, 0.01, 0.001 );
   cnt_delta_r->setStep( 0.0001 );
   left->addWidget( cnt_delta_r, row++, 1 );

   // Delta-t
   QLabel* lb_delta_t  = us_label( tr( "Delta-t (seconds):" ) );
   left->addWidget( lb_delta_t, row, 0 );
   
   cnt_delta_t = us_counter( 3, 1.0, 50.0, 15.0 );
   cnt_delta_t->setStep( 1.0 );
   left->addWidget( cnt_delta_t, row++, 1 );

   QBoxLayout* monitor = 
      us_checkbox( tr( "Monitor Simulation Progress" ), cb_monitor, true );

   left->addLayout( monitor, row++, 0, 1, 2 );

   pb_save = us_pushbutton( tr( "Save to File" ) );
   pb_save->setEnabled( false );
   connect( pb_save, SIGNAL( clicked() ), SLOT( save() ) );
   left->addWidget( pb_save, row, 0 ); 

   pb_estimate = us_pushbutton( tr( "Estimate Times" ) );
   pb_estimate->setEnabled( false );
   connect( pb_estimate, SIGNAL( clicked() ), SLOT( simulate() ) );
   left->addWidget( pb_estimate, row++, 1 ); 

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   left->addWidget( pb_help, row, 0 ); 

   QPushButton* pb_close = us_pushbutton( tr( "Close" ) );
   connect( pb_close, SIGNAL( clicked() ), SLOT( close() ) );
   left->addWidget( pb_close, row++, 1 ); 

   main->addLayout( left ); 

   // Right Column

   // Simulation plot

   QBoxLayout* right = new QVBoxLayout;
   
   QBoxLayout* plot = new US_Plot( equilibrium_plot, 
         tr( "Approach to Equilibrium Simulation" ),
         tr( "Radius" ), tr( "Concentration" ) );
   us_grid( equilibrium_plot );
   
   equilibrium_plot->setMinimumSize( 600, 600 );
   equilibrium_plot->setAxisScale( QwtPlot::yLeft  , 0.0, 1.5 );
   equilibrium_plot->setAxisScale( QwtPlot::xBottom, 5.8, 7.2 );

   right->addLayout( plot );

   te_info = us_textedit();
   te_info->setReadOnly( true );

   QFontMetrics fm( te_info->font() );
   te_info->setFixedHeight( fm.height() * 8 );

   right->addWidget( te_info );
   right->setStretchFactor( plot   , 10 );
   right->setStretchFactor( te_info, 0 );

   main->addLayout( right ); 
   update_speeds( speed_type );
}

void US_EquilTime::simulate_component( void )
{
   US_Predict1* hydro = new US_Predict1( simcomp );
   connect( hydro, SIGNAL( changed() ), SLOT( update_params() ) );
   hydro->show();
   hydro->update();
}

void US_EquilTime::update_params( void )
{
   mw = simcomp.mw;
   le_mw->setText( QString::number( mw, 'e', 4 ) );

   new_shape( current_shape );

   rb_prolate->setEnabled( true );
   rb_oblate ->setEnabled( true );
   rb_rod    ->setEnabled( true );
   rb_sphere ->setEnabled( true );
}

void US_EquilTime::new_shape( int new_shape )
{
   switch ( new_shape )
   {
      case PROLATE:
         s = simcomp.prolate.sedcoeff;
         D = simcomp.prolate.diffcoeff;
         break;

      case OBLATE:
         s = simcomp.oblate.sedcoeff;
         D = simcomp.oblate.diffcoeff;
         break;
      
      case ROD:
         s = simcomp.rod.sedcoeff;
         D = simcomp.rod.diffcoeff;
         break;

      case SPHERE:
         s = simcomp.sphere.sedcoeff;
         D = simcomp.sphere.diffcoeff;
         break;
   }

   le_sed ->setText( QString::number( s, 'e', 4 ) );
   le_diff->setText( QString::number( D, 'e', 4 ) );
   
   current_shape = new_shape;

   pb_estimate->setEnabled( true ); 
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

void US_EquilTime::update_speeds( int type )
{
   speed_steps.clear();
   te_speedlist->clear();

   if ( type == SIGMA )
   {
      if ( type != speed_type )
      {
         lb_lowspeed ->setText( tr( "Low Speed (sigma):"  ) );
         lb_highspeed->setText( tr( "High Speed (sigma):" ) );
         
         // Reset counters
         cnt_lowspeed  ->setRange( 0.1, 10, 0.01 );
         cnt_lowspeed  ->setValue( sigma_start );
         cnt_highspeed ->setRange( 0.1, 10, 0.01 );
         cnt_highspeed ->setValue( sigma_stop );
         if ( fabs( sigma_stop - sigma_start ) < 1.0e-4 ) speed_count = 1;
         cnt_speedsteps->setValue( speed_count );
      }

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
      if ( type != speed_type )
      {
         lb_lowspeed ->setText( tr( "Low Speed (rpm):"  ) );
         lb_highspeed->setText( tr( "High Speed (rpm):" ) );
         
         // Reset counters
         cnt_lowspeed  ->setRange( 100, 60000, 100 );
         cnt_lowspeed  ->setValue( rpm_start );
         cnt_highspeed ->setRange( 100, 60000, 100 );
         cnt_highspeed ->setValue( rpm_stop );
         if ( fabs( rpm_stop - rpm_start ) < 100.0 ) speed_count = 1;
         cnt_speedsteps->setValue( speed_count );
      }

      if ( speed_count > 1 )
      {
         double increment = ( rpm_stop - rpm_start ) / ( speed_count - 1 );
         
         for ( int i = 0; i < speed_count; i++ )
         {
            speed_steps <<  rpm_start + i * increment;
            te_speedlist->append( QString::number( i + 1 ) + ": rpm = " + 
                             QString::number( speed_steps[ i ], 'f', 3 ) );
         }
      }
      else
      {
         speed_steps <<  rpm_start;
         te_speedlist->append( "1: rpm = " + 
               QString::number( rpm_start, 'f', 3 ) );
      }
   }

   speed_type = type;
}

void US_EquilTime::simulate( void )
{
   double meniscus = cnt_top    ->value();
   double bottom   = cnt_bottom ->value();
   double delta_r  = cnt_delta_r->value();
   
   equilibrium_plot->clear();
   
   int sim_points = (int)( 1.5 + ( bottom - meniscus ) / delta_r );
   
   sim_radius      = new double[ sim_points ];
   sim_radius[ 0 ] = meniscus;
   
   for ( int i = 1; i < sim_points; i++ )
   {
      sim_radius[ i ] = sim_radius[ i - 1] + delta_r;
   }
   
   double* rpm     = new double[ speed_count ];
   double* sigma   = new double[ speed_count ];

   for ( int i = 0; i < speed_count; i++ )
   {
      if ( speed_type == SIGMA )
      {
         rpm[ i ] = 30.0 / M_PI * sqrt( sigma[ i ] * R * 2 * ( simcomp.temperature + K0 ) 
         / ( simcomp.mw * ( 1 - simcomp.vbar * simcomp.density ) ) ) + 0.5;
      }
   }
   
   for ( int i = 0; i < speed_count; i++ )
   {
      rpm[ i ]   = round( rpm[ i ] / 100.0 + 0.5 ) * 100.0; // round to the nearest 100
      sigma[ i ] = simcomp.mw * (1 - simcomp.vbar * simcomp.density) * sq( ( M_PI / 30.0 ) * rpm[i] )
                   / ( 2 * R * ( simcomp.temperature + K0 ) );
   }
   
   QwtPlotCurve* curve1 = new QwtPlotCurve( tr( "Simulated Data" ) );
   
   curve1->attach( equilibrium_plot );
   curve1->setStyle( QwtPlotCurve::Lines );
   curve1->setPen( QPen( Qt::green ) );

   QwtPlotCurve** curves = new QwtPlotCurve*[ speed_count ];

   for ( int i = 0; i < speed_count; i++ )
   {
      curves[ i ] = new QwtPlotCurve( tr( "Speed targets " ) + QString::number( i ) );
      curves[ i ]->setStyle( QwtPlotCurve::Lines );
      curves[ i ]->setPen( QPen( Qt::red ) );
   }

   //calc_function();
}


void US_EquilTime::save( void )
{

}



#ifdef NEVER

#include "../include/us_equiltime.h"

US_EquilTime::US_EquilTime(QWidget *parent, const char *name) 
: QFrame( parent, name)
{
   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   molecule       = 1; // 1=prolate, 2=oblate, 3=rod, 4=sphere
   rpm_start      = 18000;
   rpm_stop       = 36000;
   use_sigma      = true;
   sigma_start    = 1;
   sigma_stop     = 4;
   speed_steps    = 5;
   time_increment = 15;
   monitor_flag   = true;
   delta_t        = 15;
   delta_r        = (float) 0.001;
   tolerance      = (float) 5e-4;
   sigma          = new float [100];
   rpm            = new unsigned int [100];
   meniscus       = 5.9;
   bottom         = 6.2;
   conc           = (float) 0.15;

   GUI();   
   global_Xpos += 30;
   global_Ypos += 30;
   move(global_Xpos, global_Ypos);
}

US_EquilTime::~US_EquilTime()
{
}

void US_EquilTime::closeEvent(QCloseEvent *e)
{
   e->accept();
   global_Xpos -= 30;
   global_Ypos -= 30;
}

void US_EquilTime::GUI()
{
   data_plot = new QwtPlot(this);
   data_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
   data_plot->enableGridXMin();
   data_plot->enableGridYMin();
   data_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   data_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
   data_plot->setCanvasBackground(USglobal->global_colors.plot);
   data_plot->setMargin(USglobal->config_list.margin);
   data_plot->enableOutline(true);
   data_plot->setOutlinePen(white);
   data_plot->setOutlineStyle(Qwt::Cross);
   data_plot->setAxisTitle(QwtPlot::xBottom, "Radius");
   data_plot->setAxisTitle(QwtPlot::yLeft, "Concentration");
   data_plot->setTitle(tr("Approach to Equilibrium Simulation"));
   data_plot->setAxisScale(QwtPlot::yLeft, 0, 1.5, 0);
   data_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   data_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   data_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   data_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   data_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   data_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   data_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));     
   data_plot->show();
   
   textwindow = new US_Editor(2, this);
   textwindow->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   textwindow->e->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   textwindow->show();
   
   banner1 = new QLabel(tr("Simulation Settings:"),this);
   banner1->setFrameStyle(QFrame::WinPanel|Raised);
   banner1->setAlignment(AlignCenter|AlignVCenter);
   banner1->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
   banner1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
      
   pb_simcomp = new QPushButton(tr("Simulate Component"), this);
   pb_simcomp->setAutoDefault(false);
   pb_simcomp->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_simcomp->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_simcomp, SIGNAL(clicked()), SLOT(simulate_component()));

   lbl_mw1 = new QLabel(tr("Molecular Weight:"),this);
   lbl_mw1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_mw1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_mw1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_mw2 = new QLabel("<not selected>",this);
   lbl_mw2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_mw2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_mw2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
      
   lbl_sed1 = new QLabel(tr("Sedimentation Coeff.:"),this);
   lbl_sed1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_sed1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_sed1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   lbl_sed2 = new QLabel("<not selected>",this);
   lbl_sed2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_sed2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_sed2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   
   lbl_diff1 = new QLabel(tr("Diffusion Coeff.:"),this);
   lbl_diff1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_diff1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label)); 
   lbl_diff1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   lbl_diff2 = new QLabel("<not selected>",this);
   lbl_diff2->setFrameStyle(QFrame::WinPanlelbl_diff2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_diff2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   
   cb_prolate = new QCheckBox(tr("Prolate Ellipsoid"),this);
   cb_prolate->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_prolate->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   cb_prolate->setChecked(true);
   cb_prolate->setEnabled(false);
   connect(cb_prolate, SIGNAL(clicked()), SLOT(select_prolate()));

   cb_oblate = new QCheckBox(tr("Oblate Ellipsoid"),this);
   cb_oblate->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_oblate->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   cb_oblate->setEnabled(false);
   cb_oblate->setChecked(false);
   connect(cb_oblate, SIGNAL(clicked()), SLOT(select_oblate()));
   
   cb_rod = new QCheckBox(tr("Long Rod"),this);
   cb_rod->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_rod->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   cb_rod->setChecked(false);
   cb_rod->setEnabled(false);
   connect(cb_rod, SIGNAL(clicked()), SLOT(select_rod()));

   cb_sphere = new QCheckBox(tr("Sphere"),this);
   cb_sphere->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_sphere->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   cb_sphere->setChecked(false);
   cb_sphere->setEnabled(false);
   connect(cb_sphere, SIGNAL(clicked()), SLOT(select_sphere()));

   banner2 = new QLabel(tr("Radius Information:"),this);
   banner2->setFrameStyle(QFrame::WinPanel|Raised);
   banner2->setAlignment(AlignCenter|AlignVCenter);
   banner2->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
   banner2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   

   cb_inner = new QCheckBox(tr("Inner Channel"),this);
   cb_inner->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_inner->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   cb_inner->setChecked(true);
   connect(cb_inner, SIGNAL(clicked()), SLOT(select_inner()));

   cb_center = new QCheckBox(tr("Center Channel"),this);
   cb_center->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_center->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   cb_center->setChecked(false);
   connect(cb_center, SIGNAL(clicked()), SLOT(select_center()));

   cb_outer = new QCheckBox(tr("Outer Channel"),this);
   cb_outer->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_outer->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   cb_outer->setChecked(false);
   connect(cb_outer, SIGNAL(clicked()), SLOT(select_outer()));

   cb_custom = new QCheckBox(tr("Custom"),this);
   cb_custom->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_custom->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   cb_custom->setChecked(false);
   connect(cb_custom, SIGNAL(clicked()), SLOT(select_custom()));

   lbl_topradius = new QLabel(tr("Top Radius:"),this);
   lbl_topradius->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_topradius->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_topradius->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   ct_topradius= new QwtCounter(this);
   ct_topradius->setNumButtons(3);
   ct_topradius->setRange(5.8, 7.3, 0.01);
   ct_topradius->setValue(meniscus);
   ct_topradius->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(ct_topradius, SIGNAL(valueChanged(double)), SLOT(update_topradius(double)));

   lbl_bottomradius = new QLabel(tr("Bottom Radius:"),this);
   lbl_bottomradius->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_bottomradius->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_bottomradius->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
      
   ct_bottomradius= new QwtCounter(this);
   ct_bottomradius->setNumButtons(3);
   ct_bottomradius->setRange(5.8, 7.3, 0.01);
   ct_bottomradius->setValue(bottom);
   ct_bottomradius->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(ct_bottomradius, SIGNAL(valueChanged(double)), SLOT(update_bottomradius(double)));

   banner3 = new QLabel(tr("Rotorspeed Information:"),this);
   banner3->setFrameStyle(QFrame::WinPanel|Raised);
   banner3->setAlignment(AlignCenter|AlignVCenter);
   banner3->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
   banner3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   
   cb_sigma = new QCheckBox(tr("Use Sigma"),this);
   cb_sigma->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_sigma->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   cb_sigma->setChecked(true);
   connect(cb_sigma, SIGNAL(clicked()), SLOT(select_rpm()));

   cb_rpm = new QCheckBox(tr("Use RPM"),this);
   cb_rpm->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_rpm->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   cb_rpm->setChecked(false);
   connect(cb_rpm, SIGNAL(clicked()), SLOT(select_rpm()));

   lbl_speedstart = new QLabel(tr("Low Speed (sigma):"),this);
   lbl_speedstart->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_speedstart->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_speedstart->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
      
   ct_speedstart= new QwtCounter(this);
   ct_speedstart->setNumButtons(3);
   ct_speedstart->setRange(0.01, 10, 0.01);
   ct_speedstart->setValue(1.0);
   ct_speedstart->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(ct_speedstart, SIGNAL(valueChanged(double)), SLOT(update_speedstart(double)));

   lbl_speedstop = new QLabel(tr("High Speed (sigma):"),this);
   lbl_speedstop->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_speedstop->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_speedstop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   ct_speedstop= new QwtCounter(this);
   ct_speedstop->setNumButtons(3);
   ct_speedstop->setRange(0.1, 10, 0.01);
   ct_speedstop->setValue(sigma_stop);
   ct_speedstop->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(ct_speedstop, SIGNAL(valueChanged(double)), SLOT(update_speedstop(double)));

   lbl_speedsteps = new QLabel(tr("Speed Steps:"),this);
   lbl_speedsteps->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_speedsteps->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_speedsteps->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   ct_speedsteps= new QwtCounter(this);
   ct_speedsteps->setNumButtons(3);
   ct_speedsteps->setRange(1, 100, 1);
   ct_speedsteps->setValue(speed_steps);
   ct_speedsteps->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(ct_speedsteps, SIGNAL(valueChanged(double)), SLOT(update_speedsteps(double)));

   lbl_speedlist = new QLabel(tr("Current Speed List:"),this);
   lbl_speedlist->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_speedlist->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_speedlist->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   lb_speeds = new QListBox(this, "Speeds");
   lb_speeds->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lb_speeds->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   calc_speeds();

   banner4 = new QLabel(tr("Simulation Settings:"),this);
   banner4->setFrameStyle(QFrame::WinPanel|Raised);
   banner4->setAlignment(AlignCenter|AlignVCenter);
   banner4->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
   banner4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   
   lbl_tolerance = new QLabel(tr("Tolerance:"),this);
   lbl_tolerance->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_tolerance->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_tolerance->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   ct_tolerance= new QwtCounter(this);
   ct_tolerance->setNumButtons(3);
   ct_tolerance->setRange(1e-5, 0.01, 1e-5);
   ct_tolerance->setValue(tolerance);
   ct_tolerance->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(ct_tolerance, SIGNAL(valueChanged(double)), SLOT(update_tolerance(double)));

   
   lbl_timesteps = new QLabel(tr("Time Increment (min):"),this);
   lbl_timesteps->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_timesteps->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_timesteps->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   ct_timesteps= new QwtCounter(this);
   ct_timesteps->setNumButtons(3);
   ct_timesteps->setRange(1, 1000, 1);
   ct_timesteps->setValue(time_increment);
   ct_timesteps->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(ct_timesteps, SIGNAL(valueChanged(double)), SLOT(update_timesteps(double)));

   
   lbl_delta_r = new QLabel(tr("Delta-r (cm):"),this);
   lbl_delta_r->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_delta_r->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_delta_r->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
      
   ct_delta_r= new QwtCounter(this);
   ct_delta_r->setNumButtons(3);
   ct_delta_r->setRange(0.0001, 0.01, 0.0001);
   ct_delta_r->setValue(delta_r);
   ct_delta_r->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(ct_delta_r, SIGNAL(valueChanged(double)), SLOT(update_delta_r(double)));

      
   lbl_delta_t = new QLabel(tr("Delta-t (seconds):"),this);
   lbl_delta_t->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_delta_t->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_delta_t->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   ct_delta_t= new QwtCounter(this);
   ct_delta_t->setNumButtons(3);
   ct_delta_t->setRange(1, 50, 1);
   ct_delta_t->setValue(delta_t);
   ct_delta_t->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(ct_delta_t, SIGNAL(valueChanged(double)), SLOT(update_delta_t(double)));

   cb_monitor = new QCheckBox(tr("Monitor Simulation Progress"),this);
   cb_monitor->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_monitor->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   cb_monitor->setChecked(true);
   connect(cb_monitor, SIGNAL(clicked()), SLOT(select_monitor()));

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setAutoDefault(false);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));
      
   pb_estimate = new QPushButton(tr("Estimate Times"), this);
   pb_estimate->setAutoDefault(false);
   pb_estimate->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_estimate->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_estimate->setEnabled(false);
   connect(pb_estimate, SIGNAL(clicked()), SLOT(simulate_times()));

   pb_save = new QPushButton(tr("Save to File"), this);
   pb_save->setAutoDefault(false);
   pb_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_save->setEnabled(false);
   connect(pb_save, SIGNAL(clicked()), SLOT(save()));
      
   pb_quit = new QPushButton(tr("Close"), this);
   pb_quit->setAutoDefault(false);
   pb_quit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_quit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_quit, SIGNAL(clicked()), SLOT(quit()));

   setup_GUI();
}

void US_EquilTime::setup_GUI()
{
   int j=0;
   int rows = 13, columns = 4, spacing = 2;
   
   QGridLayout * background = new QGridLayout(this,2,2,spacing);  
   QGridLayout * subGrid1 = new QGridLayout(rows, columns, spacing);
   for (int i=0; i<rows; i++)
   {
      subGrid1->setRowSpacing(i, 26);
   }
   subGrid1->addMultiCellWidget(banner1,j,j,0,3);
   j++;
   subGrid1->addMultiCellWidget(pb_simcomp,j,j,0,3);
   j++;
   subGrid1->addMultiCellWidget(lbl_mw1,j,j,0,1);
   subGrid1->addMultiCellWidget(lbl_mw2,j,j,2,3);
   j++;
   subGrid1->addMultiCellWidget(lbl_sed1,j,j,0,1);
   subGrid1->addMultiCellWidget(lbl_sed2,j,j,2,3);
   j++;
   subGrid1->addMultiCellWidget(lbl_diff1,j,j,0,1);
   subGrid1->addMultiCellWidget(lbl_diff2,j,j,2,3);
   j++;
   subGrid1->addMultiCellWidget(cb_prolate,j,j,0,1);
   subGrid1->addMultiCellWidget(cb_oblate,j,j,2,3);
   j++;
   subGrid1->addMultiCellWidget(cb_rod,j,j,0,1);
   subGrid1->addMultiCellWidget(cb_sphere,j,j,2,3);
   j++;
   subGrid1->addMultiCellWidget(banner2,j,j,0,3);
   j++;
   subGrid1->addMultiCellWidget(cb_inner,j,j,0,1);
   subGrid1->addMultiCellWidget(cb_center,j,j,2,3);
   j++;
   subGrid1->addMultiCellWidget(cb_outer,j,j,0,1);
   subGrid1->addMultiCellWidget(cb_custom,j,j,2,3);
   j++;
   subGrid1->addMultiCellWidget(lbl_topradius,j,j,0,1);
   subGrid1->addMultiCellWidget(ct_topradius,j,j,2,3);
   j++;
   subGrid1->addMultiCellWidget(lbl_bottomradius,j,j,0,1);
   subGrid1->addMultiCellWidget(ct_bottomradius,j,j,2,3);
   j++;
   subGrid1->addMultiCellWidget(banner3,j,j,0,3);
   j++;
   subGrid1->addMultiCellWidget(cb_sigma,j,j,0,1);
   subGrid1->addMultiCellWidget(cb_rpm,j,j,2,3);
   j++;
   subGrid1->addMultiCellWidget(lbl_speedstart,j,j,0,1);
   subGrid1->addMultiCellWidget(ct_speedstart,j,j,2,3);
   j++;
   subGrid1->addMultiCellWidget(lbl_speedstop,j,j,0,1);
   subGrid1->addMultiCellWidget(ct_speedstop,j,j,2,3);
   j++;
   subGrid1->addMultiCellWidget(lbl_speedsteps,j,j,0,1);
   subGrid1->addMultiCellWidget(ct_speedsteps,j,j,2,3);
   j++;
   subGrid1->addMultiCellWidget(lbl_speedlist,j,j,0,1);
   subGrid1->addMultiCellWidget(lb_speeds,j,j+2,2,3);
   j=j+2;
   subGrid1->addMultiCellWidget(banner4,j,j,0,3);
   
   rows = 7, columns = 4, spacing = 2, j=0;
   QGridLayout * subGrid2 = new QGridLayout(rows, columns, spacing);
   for (int i=0; i<rows; i++)
   {
      subGrid2->setRowSpacing(i, 26);
   }
   subGrid2->addMultiCellWidget(banner4,j,j,0,3);
   j++;
   subGrid2->addMultiCellWidget(lbl_tolerance,j,j,0,1);
   subGrid2->addMultiCellWidget(ct_tolerance,j,j,2,3);
   j++;
   subGrid2->addMultiCellWidget(lbl_timesteps,j,j,0,1);
   subGrid2->addMultiCellWidget(ct_timesteps,j,j,2,3);
   j++;
   subGrid2->addMultiCellWidget(lbl_delta_r,j,j,0,1);
   subGrid2->addMultiCellWidget(ct_delta_r,j,j,2,3);
   j++;
   subGrid2->addMultiCellWidget(lbl_delta_t,j,j,0,1);
   subGrid2->addMultiCellWidget(ct_delta_t,j,j,2,3);
   j++;
   subGrid2->addMultiCellWidget(cb_monitor,j,j,0,3);
   j++;
   subGrid2->addMultiCellWidget(pb_help,j,j,0,1);
   subGrid2->addMultiCellWidget(pb_estimate,j,j,2,3);
   j++;
   subGrid2->addMultiCellWidget(pb_save,j,j,0,1);
   subGrid2->addMultiCellWidget(pb_quit,j,j,2,3);

   background->addLayout(subGrid1,0,0);
   background->addWidget(data_plot,0,1);
   background->addLayout(subGrid2,1,0);
   background->addWidget(textwindow,1,1);
   background->setColStretch(0,1);
   background->setColStretch(1,4);
   background->setColSpacing(0,350);
   background->setColSpacing(1,530);

   qApp->processEvents();
   QRect r = background->cellGeometry(0, 0);

   global_Xpos += 30;
   global_Ypos += 30;
   
   this->setGeometry(global_Xpos, global_Ypos, r.width()+530+spacing*3, this->height());


}

void US_EquilTime::simulate_component()
{
   US_Hydro1 *hydro1;
   hydro1 = new US_Hydro1(&simcomp);
   connect(hydro1, SIGNAL(updated()), SLOT(update_component()));
   hydro1->show();
   hydro1->update();
   cb_prolate->setEnabled(true);
   cb_oblate->setEnabled(true);
   cb_rod->setEnabled(true);
   cb_sphere->setEnabled(true);
   pb_save->setEnabled(true);
   pb_estimate->setEnabled(true);
}
/*
void US_EquilTime::resizeEvent(QResizeEvent *e)
{  
   data_plot->setGeometry( 2 * buttonw + 3 * spacing,
                           spacing, 
                           e->size().width() - 4 * spacing - 2 * buttonw,
                           (int) (2 * e->size().height()/3) - spacing);
   textwindow->setGeometry(2 * buttonw + 3 * spacing, 
                           (int) (2 * e->size().height()/3) + spacing, 
                           e->size().width() - 4 * spacing - 2 * buttonw, 
                           (int) e->size().height()/3 - 2 * spacing);
}
*/
void US_EquilTime::select_inner()
{
   meniscus = 5.9;
   bottom = 6.2;
   ct_topradius->setValue(meniscus);
   ct_bottomradius->setValue(bottom);
   cb_inner->setChecked(true);
   cb_center->setChecked(false);
   cb_outer->setChecked(false);
   cb_custom->setChecked(false);
}

void US_EquilTime::select_center()
{
   meniscus = 6.4;
   bottom = 6.7;
   ct_topradius->setValue(meniscus);
   ct_bottomradius->setValue(bottom);
   cb_inner->setChecked(false);
   cb_center->setChecked(true);
   cb_outer->setChecked(false);
   cb_custom->setChecked(false);
}

void US_EquilTime::select_outer()
{
   meniscus = 6.9;
   bottom = 7.2;
   ct_topradius->setValue(meniscus);
   ct_bottomradius->setValue(bottom);
   cb_inner->setChecked(false);
   cb_center->setChecked(false);
   cb_outer->setChecked(true);
   cb_custom->setChecked(false);
}

void US_EquilTime::select_custom()
{
   cb_inner->setChecked(false);
   cb_center->setChecked(false);
   cb_outer->setChecked(false);
   cb_custom->setChecked(true);
}

void US_EquilTime::select_prolate()
{
   molecule = 1;
   cb_prolate->setChecked(true);
   cb_oblate->setChecked(false);
   cb_rod->setChecked(false);
   cb_sphere->setChecked(false);
   update_component();
}

void US_EquilTime::select_oblate()
{
   molecule = 2;
   cb_prolate->setChecked(false);
   cb_oblate->setChecked(true);
   cb_rod->setChecked(false);
   cb_sphere->setChecked(false);
   update_component();
}

void US_EquilTime::select_rod()
{
   molecule = 3;
   cb_prolate->setChecked(false);
   cb_oblate->setChecked(false);
   cb_rod->setChecked(true);
   cb_sphere->setChecked(false);
   update_component();
}

void US_EquilTime::select_sphere()
{
   molecule = 4;
   cb_prolate->setChecked(false);
   cb_oblate->setChecked(false);
   cb_rod->setChecked(false);
   cb_sphere->setChecked(true);
   update_component();
}

void US_EquilTime::select_rpm()
{
   if(use_sigma)
   {
      use_sigma = false;
      cb_sigma->setChecked(false);
      cb_rpm->setChecked(true);
      ct_speedstart->setRange(100, 60000, 100);
      ct_speedstart->setValue(rpm_start);
      ct_speedstop->setRange(100, 60000, 100);
      ct_speedstop->setValue(rpm_stop);
      lbl_speedstart->setText(tr("Low Speed (rpm):"));
      lbl_speedstop->setText(tr("High Speed (rpm):"));
      if(fabs((double)(rpm_stop - rpm_start)) < 1e-4)
      {
         speed_steps = 1;
      }
      calc_speeds();
   }
   else
   {
      use_sigma = true;
      cb_sigma->setChecked(true);
      cb_rpm->setChecked(false);
      ct_speedstart->setRange(0.1, 10, 0.01);
      ct_speedstart->setValue(sigma_start);
      ct_speedstop->setRange(0.1, 10, 0.01);
      ct_speedstop->setValue(sigma_stop);
      lbl_speedstart->setText(tr("Low Speed (sigma):"));
      lbl_speedstop->setText(tr("High Speed (sigma):"));
      if(fabs((double)(sigma_stop - sigma_start)) < 1e-4)
      {
         speed_steps = 1;
      }
      calc_speeds();
   }
}

void US_EquilTime::calc_speeds()
{
   QString str;
   lb_speeds->clear();
   if(use_sigma)
   {
      float increment;
      delete [] sigma;
      sigma = new float [speed_steps];
      if (speed_steps >= 2)
      {
         increment = (sigma_stop - sigma_start)/(speed_steps - 1);
         for (unsigned int i=0; i<speed_steps; i++)
         {
            sigma[i] = sigma_start + i * increment;
            lb_speeds->insertItem(str.sprintf("%d: sigma = %5.3f", i+1, sigma[i]));
         }
      }
      else
      {
         sigma[0] = sigma_start;
         lb_speeds->insertItem(str.sprintf("%d: sigma = %5.3f", 1, sigma[0]));
      }
   }
   else
   {
      unsigned int increment;
      delete [] rpm;
      rpm = new unsigned int [speed_steps];
      if (speed_steps >= 2)
      {
         increment = (unsigned int) ((rpm_stop - rpm_start)/(speed_steps - 1));
         for (unsigned int i=0; i<speed_steps; i++)
         {
            rpm[i] = rpm_start + i * increment;
            lb_speeds->insertItem(str.sprintf("%d: rpm = %d", i+1, rpm[i]));
         }
      }
      else
      {
         rpm[0] = rpm_start;
         lb_speeds->insertItem(str.sprintf("%d: rpm = %d", 1, rpm[0]));
      }
   }
}

void US_EquilTime::select_monitor()
{
   if(monitor_flag)
   {
      monitor_flag = false;
   }
   else
   {
      monitor_flag = true;
   }
}

void US_EquilTime::update_topradius(double val)
{
   meniscus = val;
   cb_inner->setChecked(false);
   cb_center->setChecked(false);
   cb_outer->setChecked(false);
   cb_custom->setChecked(true);  
}

void US_EquilTime::update_bottomradius(double val)
{
   bottom = val;
   cb_inner->setChecked(false);
   cb_center->setChecked(false);
   cb_outer->setChecked(false);
   cb_custom->setChecked(true);  
}

void US_EquilTime::update_speedstart(double val)
{
   if (use_sigma)
   {
      sigma_start = (float) val;
   }
   else
   {
      rpm_start = (unsigned int) val;
   }
   calc_speeds();
}

void US_EquilTime::update_speedstop(double val)
{
   if (use_sigma)
   {
      sigma_stop = (float) val;
   }
   else
   {
      rpm_stop = (unsigned int) val;
   }
   calc_speeds();
}

void US_EquilTime::update_speedsteps(double val)
{
   speed_steps = (int) val;
   calc_speeds();
}

void US_EquilTime::update_tolerance(double val)
{
   tolerance = (float) val;
}

void US_EquilTime::update_timesteps(double val)
{
   time_increment = (float) val;
}

void US_EquilTime::update_delta_t(double val)
{
   delta_t = (float) val;
}

void US_EquilTime::update_delta_r(double val)
{
   delta_r = (float) val;
}

void US_EquilTime::simulate_times()
{
   unsigned int i;
   data_plot->clear();
   sim_points = (unsigned int) ( 1.5 + (bottom - meniscus)/delta_r);
   
   sim_radius = new double [ sim_points ];
   
   sim_radius[ 0 ] = meniscus;

   for ( int i = 1; i < sim_points; i++)
   {
      sim_radius[i] = sim_radius[i-1] + delta_r;
   }
   for (i=0; i<speed_steps; i++)
   {
      if(use_sigma)
      {
         rpm[i] = (unsigned int) (30.0/M_PI * pow((double)((sigma[i] * R * 2 * (simcomp.temperature + K0))
         /(simcomp.mw * (1 - simcomp.vbar * simcomp.density))), 0.5) + 0.5);
      }
   }
   for (i=0; i<speed_steps; i++)
   {
      rpm[i] = (unsigned int) (0.5 + (rpm[i]/100)) * 100; // round to the nearest 100
      sigma[i] = (simcomp.mw * (1 - simcomp.vbar * simcomp.density) * pow((double)((M_PI/30) * rpm[i]), (double) 2))
      /(2 * R * (simcomp.temperature + K0));
   }
   cout << "going through..." << endl;
   curve1 = data_plot->insertCurve("Simulated Data");
   data_plot->setCurveStyle(curve1, QwtCurve::Lines);
   data_plot->setCurvePen(curve1, Qt::green);
   target_curve = new long [speed_steps];
// final_curve = new long [speed_steps];
   for (i=0; i<speed_steps; i++)
   {
      target_curve[i] = data_plot->insertCurve("Speed targets");
      data_plot->setCurveStyle(target_curve[i], QwtCurve::Lines);
      data_plot->setCurvePen(target_curve[i], Qt::red);
//    final_curve[i] = data_plot->insertCurve("Final Curves");
//    data_plot->setCurveStyle(final_curve[i], QwtCurve::Lines);
//    data_plot->setCurvePen(final_curve[i], Qt::yellow);
   }

// calculate theoretical targets and plot them in the plot:
/*
   targets = new double * [speed_steps];
   for (i=0; i<speed_steps; i++)
   {
      targets[i] = new double [sim_points];
   }
   float sum=0, amplitude, integral, temp1, temp2, xval, dr=(bottom - meniscus)/5000;
   for (i=0; i<speed_steps; i++)
   {
      integral = (bottom - meniscus) * conc;
      temp1 = dr;
      for (int m=1; m<5000; m++) // use trapezoidal rule to calculate integral to get amplitude
      {
         xval = meniscus + m * dr;
         temp2 = exp(sigma[i] * (pow((double) xval, (double) 2) - pow((double) meniscus, (double) 2))) * dr;
         sum += temp1 + (temp2 - temp1)/2.0;
         temp1 = temp2;
      }
      amplitude = integral/sum;
      for (unsigned int j=0; j<sim_points; j++)
      {
         targets[i][j] = amplitude * exp(sigma[i] * (pow(sim_radius[j], 2.0) - pow(meniscus, 2.0)));
      }
      data_plot->setCurveData(final_curve[i], sim_radius, targets[i], sim_points);
   }
   data_plot->replot();
*/
   calc_function();
}

void US_EquilTime::calc_function()
{
   unsigned int i, j, count=0;
   QString str1, str2;

   double temp1, temp2;
   float omega_s;
   float last_time = 0, sum = 0, total_time = 0;
   bool flag, ldu_flag = true;

   init_finite_element();  ////////////////
   
   temp2   = delta_t * diff; //diff coeff
   omega_s = sq( M_PI * rpm[ 0 ] / 30.0 );
   temp1   = delta_t * sed * omega_s; //sed coeff
   
   for ( int i = 0; i < sim_points; i++ )
   {
      right   [ i ] = conc;
      temp_val[ i ] = conc;
      
      for ( int j = 0; j < 3; j++ )
      {
         left[ i ][ j ] = b[ i ][ j ] + temp2 * a1[ i ][ j ] - temp1 * a2[ i ][ j ];
      }
   }

   textwindow->append("Sigma:\tRPM:\tTime Increment:\tTotal Time:");
   textwindow->append("\n");
   
   for ( int speed_counter = 0; speed_counter < speed_count; speed_counter++ )
   {
      flag    = false;
      omega_s = sq( M_PI * rpm[ speed_counter ] / 30.0 );
      temp1   = delta_t * sed * omega_s; //sed coeff
      for ( int i = 0; i < sim_points; i++ )
      {
         for ( int j = 0; j < 3; j++ )
         {
            left[ i ][ j ] = b[ i ][ j ] + temp2 * a1[ i ][ j ] - temp1 * a2[ i ][ j ];
         }
      }

      str1.sprintf("%6.4f\t%5d", sigma[ speed_counter ],  rpm[ speed_counter ] );
      count = 0;
      
      while ( true )
      {
         count++;
         m3vm( &b, &right, sim_points ); //////////////////
         ldu( &left, &right, sim_points, 3, ldu_flag ); //////////////
         ldu_flag = false;
         
         if ( ( ( count * delta_t ) - last_time ) >= ( time_increment * 60 ) )
         {
            if ( monitor_flag )
            {
               data_plot->setCurveData(curve1, sim_radius, right, sim_points);
               data_plot->replot();
            }

            last_time = count * delta_t;
            sum = 0;
            
            for ( int k = 0; k < sim_points; k++ )
            {
               sum += fabs( temp_val[ k ] - right[ k ] );
               temp_val[ k ] = right[ k ];
            }

            if ( sum <= tolerance )
            {
               total_time += last_time;
               str2.sprintf( "\t%6.2f hours\t%6.2f hours", last_time / 3600, total_time / 3600);
               textwindow->append( str1 + str2 );
               flag      = true;
               ldu_flag  = true;
               last_time = 0;
               
               data_plot->setCurveData(target_curve[speed_counter], sim_radius, right, sim_points);
               data_plot->replot();
               qApp->processEvents();
            }
         }

         if ( flag ) break; 
      }  // while
   }

   textwindow->append(
     "\n"
     "(Note: All speeds have been adjusted to be rounded to the nearest 100 RPM.)\n"
     "__________________________________________________________________________\n"
     );
   
   delete [] sim_radius;
   
   for ( int i = 0; i < sim_points; i++ )
   {
      delete [] a1  [ i ];
      delete [] a2  [ i ];
      delete [] b   [ i ];
      delete [] left[ i ];
   }

   delete [] a1;
   delete [] a2;
   delete [] b;
   delete [] left;
   delete [] right;
   delete [] temp_val;
   delete [] target_curve;
}

void US_EquilTime::init_finite_element()
{
   a1       = new double* [ sim_points ];
   a2       = new double* [ sim_points ];
   b        = new double* [ sim_points ];
   temp_val = new double  [ sim_points ];

   // Concentration has the current concentration for all scans

   left  = new double* [ sim_points ];
   right = new double  [ sim_points ];
   
   for ( int i = 0; i < sim_points; i++ )
   {
      a1  [ i ] = new double [ 3 ];
      a2  [ i ] = new double [ 3 ];
      b   [ i ] = new double [ 3 ];
      left[ i ] = new double [ 3 ];
   }

   a1[ 0 ][ 1 ] = ( sim_radius[ 0 ] / delta_r ) + 0.5;
   a1[ 0 ][ 0 ] = 0.0;
   
   for ( int i = 1; i < sim_points; i++ )
   {
      a1[ i ]    [ 0 ] =  - sim_radius[ i ] / delta_r + 0.5;
      a1[ i ]    [ 1 ] = 2.0 * sim_radius[ i ] / delta_r;
      a1[ i - 1 ][ 2 ] = a1[ i ][ 0 ];
   }

   a1[ sim_points - 1 ][ 1 ] = sim_radius[ sim_points - 1 ] / delta_r - 0.5;
   a1[ sim_points - 1 ][ 2 ] = 0.0;
   
   double delta2 = sq( delta_r );
   
   a2[0][1] = -1 * (sim_radius[0] * sim_radius[0])/2.0 - sim_radius[0] * (delta_r/3.0) - delta2/12.0;
   a2[0][0] = 0.0;
   
   for ( int i = 1; i < sim_points; i++ )
   {
      a2[ i ][ 0 ] = sq( sim_radius[ i ] ) / 2.0 
                   - 2.0 * sim_radius[ i ] * delta_r / 3.0 + delta2 / 4.0;
      a2[ i ][ 1 ] = -2 * sim_radius[ i ] * delta_r / 3.0;
      a2[ i - 1 ][ 2 ] = - sq( sim_radius[ i - 1 ] ) / 2.0 
                         - 2.0 * sim_radius[ i - 1 ] * delta_r / 3.0 - delta2 / 4.0;
   }

   a2[ sim_points - 1][ 1 ] = sq( sim_radius[ sim_points - 1 ] ) / 2.0 
                            - sim_radius[ sim_points - 1 ] * delta_r / 3.0 + delta2 / 12.0;
   a2[ sim_points - 1][ 2 ] = 0.0;
   
   b[ 0 ][ 0 ] = 0.0;
   b[ 0 ][ 1 ] = sim_radius[ 0 ] * delta_r / 3.0 + delta2 / 12.0;
   
   for ( int i = 1; i < sim_points; i++ )
   {
      b[ i ]    [ 0 ] = sim_radius[ i ] * delta_r / 6.0 - delta2 / 12.0;
      b[ i ]    [ 1 ] = 2.0 * sim_radius[ i ] * delta_r / 3.0;
      b[ i - 1 ][ 2 ] = b[ i ][ 0 ];
   }

   b[ sim_points - 1 ][ 1 ] = sim_radius[ sim_points - 1 ] * delta_r / 3.0 - delta2 / 12.0;
   b[ sim_points - 1 ][ 2 ] = 0.0;
}

void US_EquilTime::save()
{
}

void US_EquilTime::update_component()
{
   QString str;
   lbl_mw2->setText(str.sprintf("%6.4e", simcomp.mw));
   switch (molecule)
   {
      case 1:
      {
         sed = simcomp.prolate.sedcoeff;
         diff = simcomp.prolate.diffcoeff;
         break;
      }
      case 2:
      {
         sed = simcomp.oblate.sedcoeff;
         diff = simcomp.oblate.diffcoeff;
         break;
      }
      case 3:
      {
         sed = simcomp.rod.sedcoeff;
         diff = simcomp.rod.diffcoeff;
         break;
      }
      case 4:
      {
         sed = simcomp.sphere.sedcoeff;
         diff = simcomp.sphere.diffcoeff;
         break;
      }
   }
   lbl_sed2->setText(str.sprintf("%6.4e", sed));
   lbl_diff2->setText(str.sprintf("%6.4e", diff));
}

#endif
