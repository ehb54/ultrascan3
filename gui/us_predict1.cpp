//! \file us_predict1.cpp

#include "us_predict1.h"
#include "us_constants.h"
#include "us_gui_settings.h"
#include "us_buffer_gui.h"

#include "qwt_legend.h"

US_Predict1::Hydrocomp::Hydrocomp()
{
   s      = 0.0;
   D      = 0.0;
   f      = 0.0;
   f_f0   = 1.0;
   a      = 0.0;
   b      = 0.0;
   volume = 0.0;
}

US_Predict1::Hydrosim::Hydrosim()
{
   mw          = 50000.0;
   density     = DENS_20W;
   viscosity   = VISC_20W * 100.0;
   vbar        = TYPICAL_VBAR;
   temperature = 20.0;
   axial_ratio = 10.0;
   guid        = QString();
}

US_Predict1::US_Predict1( Hydrosim&                     parm, 
                          int                           invID,
                          const US_AnalyteGui::AnalyteData a_data,
                          bool                          disk_access,
                          bool                          signal_wanted )
   : US_WidgetsDialog( 0, 0 ), 
     allparams   ( parm ), 
     investigator( invID ),
     base_analyte( a_data ),
     access      ( disk_access ),
     signal      ( signal_wanted )
{
   analyte            = base_analyte;
   temperature        = parm.temperature;
   mw                 = parm.mw;  
   ratio              = parm.axial_ratio;
   solution.density   = parm.density;
   solution.viscosity = parm.viscosity;
   solution.vbar20    = parm.vbar;
   solution.vbar      = parm.vbar + ( 4.25e-4 * ( temperature - 20.0 ) );

   double x = 1.1;

   // From: K.E. van Holde, Biophysical Chemistry, 2nd edition, chapter 4.1
   // Calculate frictional ratio as a function of axial ratio
   for ( int i = 0; i < ARRAYSIZE; i++, x += 0.1 )
   {
      prolate[ i ] = pow( x, -1.0 / 3.0 ) * sqrt( sq( x ) - 1.0 ) / 
                     log( x + sqrt( sq( x ) - 1.0 ) );
      
      oblate[ i ]  = sqrt( sq( x ) - 1.0 ) / 
                     ( pow( x, 2.0 / 3.0 ) * atan( sqrt( sq( x ) - 1.0 ) ) );
      
      rod[ i ]     = pow( 2.0 / 3.0, 1.0 / 3.0 ) * pow( x, 2.0 / 3.0 ) / 
                     ( log( 2.0 * x ) - 0.3 );
      
      ratio_x[ i ] = x;
   }

   setWindowTitle( tr( "Modeling s, D, and f from MW for 4 basic shapes" ) );
   setPalette( US_GuiSettings::frameColor() );

   // Very light gray
   QPalette gray =  US_GuiSettings::editColor();
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );

   QBoxLayout* main = new QVBoxLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   QBoxLayout* top = new QHBoxLayout;

   QGridLayout* controls = new QGridLayout;
   int c_row = 0;

   // Basic values
   if ( signal )
   {
      QLabel* lb_density = us_label( tr( "Density" ) );
      controls->addWidget( lb_density, c_row, 0 );

      le_density = us_lineedit( QString::number( solution.density, 'f', 4 ) );
      le_density->setPalette( gray );
      le_density->setReadOnly( true );
      controls->addWidget( le_density, c_row++, 1 );

      QLabel* lb_viscosity = us_label( tr( "Viscosity" ) );
      controls->addWidget( lb_viscosity, c_row, 0 );

      le_viscosity = us_lineedit( QString::number( solution.viscosity, 'f', 4 ) );
      le_viscosity->setPalette( gray );
      le_viscosity->setReadOnly( true );
      controls->addWidget( le_viscosity, c_row++, 1 );
   }
   else
   {
      QPushButton* pb_density = us_pushbutton( tr( "Density" ) );
      connect( pb_density, SIGNAL( clicked() ), SLOT( get_buffer() ) );
      controls->addWidget( pb_density, c_row, 0 );

      le_density = us_lineedit();
      le_density->setText( QString::number( DENS_20W ) );
      connect( le_density, SIGNAL( textChanged( const QString& ) ),
                           SLOT  ( density    ( const QString& ) ) );
      controls->addWidget( le_density, c_row++, 1 );

      QPushButton* pb_viscosity = us_pushbutton( tr( "Viscosity" ) );
      connect( pb_viscosity, SIGNAL( clicked() ), SLOT( get_buffer() ) );
      controls->addWidget( pb_viscosity, c_row, 0 );

      le_viscosity = us_lineedit();
      le_viscosity->setText( QString::number( VISC_20W * 100.0 ) );
      connect( le_viscosity, SIGNAL( textChanged( const QString& ) ), 
                             SLOT  ( viscosity  ( const QString& ) ) );
      controls->addWidget( le_viscosity, c_row++, 1 );
   }

   QPushButton* pb_vbar = us_pushbutton( tr( "vbar (20 deg C)" ) );
   connect( pb_vbar, SIGNAL( clicked() ), SLOT( get_peptide() ) );
   controls->addWidget( pb_vbar, c_row, 0 );

   le_vbar = us_lineedit();
   le_vbar->setText( QString::number( TYPICAL_VBAR ) );
   connect( le_vbar, SIGNAL( textChanged( const QString& ) ), 
                     SLOT  ( vbar       ( const QString& ) ) );
   controls->addWidget( le_vbar, c_row++, 1 );

   QLabel* lb_mw = us_label( tr( "Molecular Weight:" ) );
   controls->addWidget( lb_mw, c_row, 0 );

   le_mw = us_lineedit();
   le_mw->setText( QString::number( mw, 'e', 3 ) );
   connect( le_mw, SIGNAL( textChanged( const QString& ) ), 
                   SLOT  ( update_mw  ( const QString& ) ) );
   controls->addWidget( le_mw, c_row++, 1 );

   QLabel* lb_temperature = us_label( 
         tr( "Temperature (<span>&deg;</span>C):" ) );
   controls->addWidget( lb_temperature, c_row, 0 );

   QLineEdit* le_temperature = us_lineedit();
   le_temperature->setText( QString::number( NORMAL_TEMP, 'f', 1 ) );
   connect( le_vbar, SIGNAL( textChanged( const QString& ) ), 
                     SLOT  ( degC       ( const QString& ) ) );
   controls->addWidget( le_temperature, c_row++, 1 );

   // Axial Ratio
   QLabel* lb_axial = us_label( tr( "Axial Ratio:" ) );
   controls->addWidget( lb_axial, c_row, 0 );

   le_axial = us_lineedit();
   le_axial->setText( QString::number( ratio, 'f', 1 ) );
   connect( le_axial, SIGNAL( textChanged ( const QString& ) ), 
                      SLOT  ( update_ratio( const QString& ) ) );
   controls->addWidget( le_axial, c_row++, 1 );

   // Information
   QPalette p = US_GuiSettings::editColor();

   lb_info = us_banner(
         tr( "Please select an axial ratio by\n"
             "dragging the white bar with the\n"
             "mouse to change the axial ratio" ) );
   lb_info->setPalette( p );
   controls->addWidget( lb_info, c_row, 0, 3, 2 );
   c_row += 3;

   // Control buttons
   QBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons->addWidget( pb_help );

   QPushButton* pb_close = us_pushbutton( tr( "Close" ) );
   connect( pb_close, SIGNAL( clicked() ), SLOT( close() ) );
   buttons->addWidget( pb_close );

   QPushButton* pb_accept = us_pushbutton( tr( "Accept" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( complete() ) );
   buttons->addWidget( pb_accept );

   controls->addLayout( buttons, c_row++, 0, 1, 2 );

   top->addLayout ( controls );

   // Graph
   plotLayout = new US_Plot( plot,
         tr( "f/f0 Dependence on Axial Ratios" ),
         tr( "Axial Ratio" ),
         tr( "f / f0" ) );

   plot->setAxisScale(QwtPlot::xBottom, 0.0, 100.0 );
   plot->setAxisScale(QwtPlot::yLeft  , 1.0,   4.5 );
   plot->setMinimumSize( 650, 350 );


   pick = new US_PlotPicker( plot );
   pick->setRubberBand( QwtPicker::VLineRubberBand );
   connect( pick, SIGNAL( moved    ( const QwtDoublePoint& ) ),
                  SLOT  ( new_value( const QwtDoublePoint& ) ) );
   connect( pick, SIGNAL( mouseDown( const QwtDoublePoint& ) ),
                  SLOT  ( new_value( const QwtDoublePoint& ) ) );
   connect( pick, SIGNAL( mouseUp  ( const QwtDoublePoint& ) ),
                  SLOT  ( mouseU   ( const QwtDoublePoint& ) ) );

   QwtPlotGrid* grid = us_grid( plot );
   grid->attach( plot );

   prolate_curve = us_curve( plot, tr( "Prolate Ellipsoid" ) );
   prolate_curve->setPen ( QPen( QBrush( Qt::magenta  ), 2.0 ) );
   prolate_curve->setData( ratio_x, prolate, ARRAYSIZE );
   
   oblate_curve  = us_curve( plot, tr( "Oblate Ellipsoid" ) );
   oblate_curve ->setPen ( QPen( QBrush( Qt::yellow ), 2.0 ) );
   oblate_curve ->setData( ratio_x, oblate, ARRAYSIZE );

   rod_curve = us_curve( plot, tr( "Long Rod" ) );
   rod_curve->setPen( QPen( QBrush( Qt::cyan ), 2.0 ) );
   rod_curve->setData( ratio_x, rod, ARRAYSIZE );

   vline_x[ 0 ] = ratio;
   vline_x[ 1 ] = ratio;
   vline_y[ 0 ] = 1.1;
   vline_y[ 1 ] = 4.3;

   vline_curve = us_curve( plot, tr( "Axial Ratio" ) );
   vline_curve->setPen ( QPen( QBrush( Qt::white  ), 1.5 ) );
   vline_curve->setData( vline_x, vline_y, 2 );

   QwtLegend* legend = new QwtLegend();
   plot->insertLegend( legend, QwtPlot::BottomLegend, 0.1 );

   top->addLayout( plotLayout );
   main->addLayout( top );
   
   QGridLayout* values = new QGridLayout;
   
   QLabel* titles[ 8 ];
   
   titles[ 0 ] = us_label( tr( "Model:" ) );
   titles[ 1 ] = us_label( "s (sec)" );
   titles[ 2 ] = us_label( "D (cm<sup>2</sup>/sec)" );
   titles[ 3 ] = us_label( "f" );
   titles[ 4 ] = us_label( "f / f0" );
   titles[ 5 ] = us_label( "a (<span>&Aring;</span>)" );
   titles[ 6 ] = us_label( "b (<span>&Aring;</span>)" );
   titles[ 7 ] = us_label( tr( "Volume " ) + "(&Aring;<sup>3</sup>)" );

   int row = 0;

   for ( int i = 0; i < 8; i++ )
   {
      titles[ i ]->setAlignment( Qt::AlignCenter );
      values->addWidget( titles[ i ], row, i );
   }

   row++;

   lb_sphere [ 0 ] = us_label( tr( "Sphere:" ) );
   lb_prolate[ 0 ] = us_label( tr( "Prolate:" ) );
   lb_oblate [ 0 ] = us_label( tr( "Oblate:" ) );
   lb_rod    [ 0 ] = us_label( tr( "Long Rod:" ) );

   for ( int i = 0; i < 8; i++ )
   {
      if ( i > 0 )
      {
         lb_sphere [ i ] = us_label( "" );
         lb_prolate[ i ] = us_label( "" );
         lb_oblate [ i ] = us_label( "" );
         lb_rod    [ i ] = us_label( "" );

         lb_sphere [ i ]->setPalette( p );
         lb_prolate[ i ]->setPalette( p );
         lb_oblate [ i ]->setPalette( p );
         lb_rod    [ i ]->setPalette( p );
      }

      lb_sphere [ i ]->setAlignment( Qt::AlignCenter );
      lb_prolate[ i ]->setAlignment( Qt::AlignCenter );
      lb_oblate [ i ]->setAlignment( Qt::AlignCenter );
      lb_rod    [ i ]->setAlignment( Qt::AlignCenter );

      values->addWidget( lb_sphere [ i ], row    , i );
      values->addWidget( lb_prolate[ i ], row + 1, i );
      values->addWidget( lb_oblate [ i ], row + 2, i );
      values->addWidget( lb_rod    [ i ], row + 3, i );
   }

   main->addLayout( values );
   update();
}

void US_Predict1::complete( void )
{
   if ( signal )
   {
      emit done();
      emit changed( analyte );
   }
   close();
}

void US_Predict1::update_ratio( const QString& r )
{
   QwtDoublePoint p( r.toDouble(), 0.0 );
   mouseU( p );
}

void US_Predict1::update_mw( const QString& s )
{
   mw = s.toDouble();
   update();
}

void US_Predict1::degC( const QString& s )
{
   temperature = s.toDouble();
   solution.vbar      = solution.vbar20 + 4.25e-4 * ( temperature - 20.0 );

   US_Math::data_correction( temperature, solution );
   update();
}

void US_Predict1::get_peptide( void )
{
   US_AnalyteGui* dialog = new US_AnalyteGui( investigator, true, analyte.guid );
   connect( dialog, SIGNAL( valueChanged( US_AnalyteGui::AnalyteData ) ),
                    SLOT  ( update_vbar ( US_AnalyteGui::AnalyteData ) ) );
   dialog->exec();
}

void US_Predict1::update_vbar( const US_AnalyteGui::AnalyteData ad )
{
   analyte       = ad;
   mw            = ad.mw;
   solution.vbar = ad.vbar;

   le_mw  ->setText( QString::number( (int) mw ) );
   le_vbar->setText( QString::number( solution.vbar, 'f', 4 ) );

   US_Math::data_correction( temperature, solution );
   update();
}

void US_Predict1::get_buffer( void )
{
   US_BufferGui* dialog = new US_BufferGui( investigator, true );
   connect( dialog, SIGNAL( valueChanged ( US_Buffer ) ),
                    SLOT  ( update_buffer( US_Buffer ) ) );
   dialog->exec();
}

void US_Predict1::update_buffer( const US_Buffer b )
{
   buffer = b;
   solution.density   = b.density;
   solution.viscosity = b.viscosity;

   le_density  ->setText( QString::number( solution.density   ) );
   le_viscosity->setText( QString::number( solution.viscosity ) );

   US_Math::data_correction( temperature, solution );
   update();
}

void US_Predict1::density( const QString& s )
{
   solution.density = s.toDouble();
   US_Math::data_correction( temperature, solution );
   update();
}

void US_Predict1::viscosity( const QString& s )
{
   solution.viscosity = s.toDouble();
   US_Math::data_correction( temperature, solution );
   update();
}

void US_Predict1::vbar( const QString& s )
{
   solution.vbar = s.toDouble();
   US_Math::data_correction( temperature, solution );
   update();
}

void US_Predict1::new_value( const QwtDoublePoint& p )
{
   ratio = p.x();

   le_axial ->disconnect();
   le_axial->setText( QString::number( ratio, 'f', 1 ) );

   connect( le_axial, SIGNAL( textChanged ( const QString& ) ), 
                      SLOT  ( update_ratio( const QString& ) ) );
   update();
}

void US_Predict1::mouseU( const QwtDoublePoint& p )
{
   ratio = p.x();

   QString msg;

   if ( ratio < 1.1 )
   {
      msg = tr( "Attention:\n\n"
                "The lower axial ratio limit is 1.1!" );
      ratio = 1.1;
   }
   
   else if ( ratio < 6.0 && ratio >= 1.1 )
      msg = tr( "Attention:\n\n"
                "The rod model is unreliable\n"
                "for axial ratios less than 6.0" );
   
   else if ( ratio >= 6.0 && ratio <= 100.0 )
      msg = tr( "Please select an axial ratio by\n"
                "dragging the white bar with the\n"
                "mouse to change the axial ratio" );

   else if ( ratio > 100 )
   {
      msg = tr( "Attention:\n\n"
                "The upper axial ratio limit is 100!" );
      ratio = 100.0;
   }

   lb_info->setText( msg );

   vline_x[ 0 ] = ratio;
   vline_x[ 1 ] = ratio;
   vline_curve->setData( vline_x, vline_y, 2 );
   plot->replot();

   le_axial ->disconnect();
   
   le_axial->setText( QString::number( ratio, 'f', 1 ) );

   connect( le_axial, SIGNAL( textChanged ( const QString& ) ), 
                      SLOT  ( update_ratio( const QString& ) ) );
   update();
}

void US_Predict1::update()
{
   allparams.mw          = mw;
   allparams.vbar        = solution.vbar;
   allparams.density     = solution.density;
   allparams.viscosity   = solution.viscosity;
   allparams.temperature = temperature;
   allparams.axial_ratio = ratio;
   
   int index = (int) ( ( ratio - 1.1 ) * 10.0 + 0.5 );
  
   US_Math::data_correction( temperature, solution );

   double t                = temperature + K0;
   double vol_per_molecule = solution.vbar * mw / AVOGADRO;
   double rad_sphere       = 
                 pow( vol_per_molecule * 3.0 / ( 4.0 * M_PI ), 1.0 / 3.0 );
   
   double f0 = rad_sphere * 6.0 * M_PI * solution.viscosity_tb * 0.01;

   // Recaluclate volume to put into cubic angstroms:
   vol_per_molecule = ( 4.0 / 3.0 ) * M_PI * pow( rad_sphere * 1.0e+08, 3.0 );

   // Prolate ellipsoid, ratio = ap/bp  (a = semi-major axis)
   double ap = 1.0e+08 * ( rad_sphere * pow( ratio, 2.0 / 3.0 ) );
   double bp = ap / ratio;
   double fp = prolate[ index ] * f0;
   double sp = mw * solution.buoyancyb / ( AVOGADRO * fp );
   double Dp = sp * R * t / ( mw * solution.buoyancyb );
   
   lb_prolate[ 1 ]->setText( QString::number( sp              , 'e', 4 ) );
   lb_prolate[ 2 ]->setText( QString::number( Dp              , 'e', 4 ) );
   lb_prolate[ 3 ]->setText( QString::number( fp              , 'e', 4 ) ); 
   lb_prolate[ 4 ]->setText( QString::number( prolate[ index ], 'e', 4 ) );
   lb_prolate[ 5 ]->setText( QString::number( ap              , 'e', 4 ) );
   lb_prolate[ 6 ]->setText( QString::number( bp              , 'e', 4 ) );
   lb_prolate[ 7 ]->setText( QString::number( vol_per_molecule, 'e', 4 ) );
   
   allparams.prolate.s      = sp;
   allparams.prolate.D      = Dp;
   allparams.prolate.f      = fp;
   allparams.prolate.f_f0   = prolate[ index ];
   allparams.prolate.a      = ap;
   allparams.prolate.b      = bp;
   allparams.prolate.volume = vol_per_molecule;
   
   // Oblate ellipsoid:
   double bo = 1.0e+08 * rad_sphere / pow( ratio, 2.0 / 3.0 );
   double ao = ratio * bo;
   double fo = oblate[ index ] * f0;
   double so = mw * solution.buoyancyb / ( AVOGADRO * fo );
   double Do = so * R * t / ( mw * solution.buoyancyb );
   
   lb_oblate[ 1 ]->setText( QString::number( so              , 'e', 4 ) );
   lb_oblate[ 2 ]->setText( QString::number( Do              , 'e', 4 ) );
   lb_oblate[ 3 ]->setText( QString::number( fo              , 'e', 4 ) );
   lb_oblate[ 4 ]->setText( QString::number( oblate[ index ] , 'e', 4 ) );
   lb_oblate[ 5 ]->setText( QString::number( ao              , 'e', 4 ) );
   lb_oblate[ 6 ]->setText( QString::number( bo              , 'e', 4 ) );
   lb_oblate[ 7 ]->setText( QString::number( vol_per_molecule, 'e', 4 ) );
   
   allparams.oblate.s      = so;
   allparams.oblate.D      = Do;
   allparams.oblate.f      = fo;
   allparams.oblate.f_f0   = oblate[ index ];
   allparams.oblate.a      = ao;
   allparams.oblate.b      = bo;
   allparams.oblate.volume = vol_per_molecule;

   // Long rod:
   double br = 1.0e+08 * pow( 2.0 / ( 3.0 * ratio ), 1.0 / 3.0 ) * rad_sphere;
   double ar = ratio * br;
   double fr = rod[ index ] * f0;
   double sr = mw * solution.buoyancyb / ( AVOGADRO * fr );
   double Dr = sr * R * t / ( mw * solution.buoyancyb );

   lb_rod[ 1 ]->setText( QString::number( sr              , 'e', 4 ) );
   lb_rod[ 2 ]->setText( QString::number( Dr              , 'e', 4 ) );
   lb_rod[ 3 ]->setText( QString::number( fr              , 'e', 4 ) );
   lb_rod[ 4 ]->setText( QString::number( rod[ index ]    , 'e', 4 ) );
   lb_rod[ 5 ]->setText( QString::number( ar              , 'e', 4 ) );
   lb_rod[ 6 ]->setText( QString::number( br              , 'e', 4 ) );
   lb_rod[ 7 ]->setText( QString::number( vol_per_molecule, 'e', 4 ) );

   allparams.rod.s      = sr;
   allparams.rod.D      = Dr;
   allparams.rod.f      = fr;
   allparams.rod.f_f0   = rod[ index ];
   allparams.rod.a      = ar;
   allparams.rod.b      = br;
   allparams.rod.volume = vol_per_molecule;

   // Sphere:
   double ss = mw * solution.buoyancyb / ( AVOGADRO * f0 );
   double Ds = ss * R * t / ( mw * solution.buoyancyb );

   lb_sphere[ 1 ]->setText( QString::number( ss                  , 'e', 4 ) );
   lb_sphere[ 2 ]->setText( QString::number( Ds                  , 'e', 4 ) );
   lb_sphere[ 3 ]->setText( QString::number( f0                  , 'e', 4 ) );
   lb_sphere[ 4 ]->setText( QString::number( 1.0                 , 'e', 4 ) );
   lb_sphere[ 5 ]->setText( QString::number( 1.0e+08 * rad_sphere, 'e', 4 ) );
   lb_sphere[ 6 ]->setText( QString::number( 1.0e+08 * rad_sphere, 'e', 4 ) );
   lb_sphere[ 7 ]->setText( QString::number( vol_per_molecule    , 'e', 4 ) );
   
   allparams.sphere.s      = ss;
   allparams.sphere.D      = Ds;
   allparams.sphere.f      = f0;
   allparams.sphere.f_f0   = 1.0;
   allparams.sphere.a      = 1.0e+08 * rad_sphere;
   allparams.sphere.b      = 1.0e+08 * rad_sphere;
   allparams.sphere.volume = vol_per_molecule;
   
   if ( signal ) emit changed();
}
