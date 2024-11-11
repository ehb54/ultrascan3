//! \file us_predict1.cpp

#include "us_predict1.h"
#include "us_constants.h"
#include "us_gui_settings.h"
#include "us_buffer_gui.h"
#include "us_choice.h"
#include "us_solution_gui.h"
#include "us_math2.h"
#if QT_VERSION > 0x050000
#include "qwt_point_data.h"
#else
#define setSamples(a,b,c) setData(a,b,c)
#endif
#include "qwt_legend.h"


US_Predict1::US_Predict1( US_Hydrosim&     parm, 
                          const US_Analyte a_data,
                          int              disk_access,
                          bool             signal_wanted )
   : US_WidgetsDialog( 0, 0 ), 
     allparams   ( parm ), 
     base_analyte( a_data ),
     access      ( disk_access ),
     signal      ( signal_wanted )
{
   analyte            = base_analyte;
   temperature        = NORMAL_TEMP;
   mw                 = parm.mw;  
   ratio              = parm.axial_ratio;
   solution.density   = parm.density;
   solution.viscosity = parm.viscosity;
   solution.vbar20    = parm.vbar;
   solution.vbar      = parm.vbar + ( 4.25e-4 * ( temperature - 20.0 ) );

	max_x = 100;

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

   QPushButton* pb_solution = us_pushbutton( tr( "Select Solution" ) );
   connect( pb_solution, SIGNAL( clicked() ), SLOT( get_solution() ) );
   controls->addWidget( pb_solution, c_row++, 0, 1, 2 );

   // Basic values
   if ( signal )
   {
      QLabel* lb_density = us_label( tr( "Density" ) );
      controls->addWidget( lb_density, c_row, 0 );

      solution.density = DENS_20W;
      le_density = us_lineedit( QString::number( solution.density, 'f', 4 ) );
      le_density->setPalette( gray );
      le_density->setReadOnly( true );
      controls->addWidget( le_density, c_row++, 1 );

      QLabel* lb_viscosity = us_label( tr( "Viscosity" ) );
      controls->addWidget( lb_viscosity, c_row, 0 );

      solution.viscosity = VISC_20W;
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
      le_density->setText( QString::number( DENS_20W, 'f', 4 ) );
      connect( le_density, SIGNAL( textChanged( const QString& ) ),
                           SLOT  ( density    ( const QString& ) ) );
      controls->addWidget( le_density, c_row++, 1 );

      QPushButton* pb_viscosity = us_pushbutton( tr( "Viscosity" ) );
      connect( pb_viscosity, SIGNAL( clicked() ), SLOT( get_buffer() ) );
      controls->addWidget( pb_viscosity, c_row, 0 );

      le_viscosity = us_lineedit();
      le_viscosity->setText( QString::number( VISC_20W, 'f', 4 ) );
      connect( le_viscosity, SIGNAL( textChanged( const QString& ) ), 
                             SLOT  ( viscosity  ( const QString& ) ) );
      controls->addWidget( le_viscosity, c_row++, 1 );
   }
   US_Math2::data_correction( temperature, solution );
   QPushButton* pb_vbar = us_pushbutton( tr( "vbar (20" ) + DEGC + ")" );
   connect( pb_vbar, SIGNAL( clicked() ), SLOT( get_peptide() ) );
   controls->addWidget( pb_vbar, c_row, 0 );

   le_vbar = us_lineedit();
   le_vbar->setText( QString::number( solution.vbar20, 'e', 4 ) );
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
         tr( "Temperature (" ) + DEGC + tr( "):" ) );
   controls->addWidget( lb_temperature, c_row, 0 );

   QLineEdit* le_temperature = us_lineedit();
   le_temperature->setText( QString::number( temperature, 'f', 1 ) );

   if ( signal )
   {
      le_temperature->setReadOnly( true );
      le_temperature->setPalette( gray );
   }
   else
   {
      connect( le_temperature, SIGNAL( textChanged( const QString& ) ), 
                               SLOT  ( degC       ( const QString& ) ) );
   }
   controls->addWidget( le_temperature, c_row++, 1 );

   // Axial Ratio
   QLabel* lb_axial = us_label( tr( "Axial Ratio:" ) );
   controls->addWidget( lb_axial, c_row, 0 );

   le_axial = us_lineedit();
   le_axial->setText( QString::number( ratio, 'f', 3 ) );
   connect( le_axial, SIGNAL( editingFinished( void ) ), 
                      SLOT  ( update_ratio   ( void ) ) );
   controls->addWidget( le_axial, c_row++, 1 );

   // Max x range
   QLabel* lb_max_x = us_label( tr( "Maximum Ratio:" ) );
   controls->addWidget( lb_max_x, c_row, 0 );

   le_max_x = us_lineedit();
   le_max_x->setText( QString::number( max_x, 'f', 3 ) );
   connect( le_max_x, SIGNAL( textChanged ( const QString& ) ), 
                      SLOT  ( update_max_x   ( const QString& ) ) );
   controls->addWidget( le_max_x, c_row++, 1 );

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

   plot->setAxisScale(QwtPlot::xBottom, 0.0, max_x );
//   plot->setAxisScale(QwtPlot::yLeft  , 1.0,   9.5 );
	plot->setAxisAutoScale( QwtPlot::yLeft );
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
   prolate_curve->setSamples( ratio_x, prolate, ARRAYSIZE );
   
   oblate_curve  = us_curve( plot, tr( "Oblate Ellipsoid" ) );
   oblate_curve ->setPen ( QPen( QBrush( Qt::yellow ), 2.0 ) );
   oblate_curve ->setSamples( ratio_x, oblate, ARRAYSIZE );

   rod_curve = us_curve( plot, tr( "Long Rod" ) );
   rod_curve->setPen( QPen( QBrush( Qt::cyan ), 2.0 ) );
   rod_curve->setSamples( ratio_x, rod, ARRAYSIZE );

	double maxy = max(prolate_curve->maxYValue(), rod_curve->maxYValue());
   vline_x[ 0 ] = ratio;
   vline_x[ 1 ] = ratio;
   vline_y[ 0 ] = 0;
   vline_y[ 1 ] = maxy;

   vline_curve = us_curve( plot, tr( "Axial Ratio" ) );
   vline_curve->setPen ( QPen( QBrush( Qt::white  ), 1.5 ) );
   vline_curve->setSamples( vline_x, vline_y, 2 );

   QwtLegend* legend = new QwtLegend();
   plot->insertLegend( legend, QwtPlot::BottomLegend, 0.1 );

   top->addLayout( plotLayout );
   main->addLayout( top );
   
   QGridLayout* values = new QGridLayout;

   
   titles[ 0 ] = us_label( tr( "Model:" ) );
   titles[ 1 ] = us_label( "s<sub>app</sub> (S)" );
   titles[ 2 ] = us_label( "D<sub>app</sub> (cm<sup>2</sup>/sec)" );
   titles[ 3 ] = us_label( "f" );
   titles[ 4 ] = us_label( "f / f0" );
   titles[ 5 ] = us_label( "a (<span>&Aring;</span>)" );
   titles[ 6 ] = us_label( "b (<span>&Aring;</span>)" );
   titles[ 7 ] = us_label( tr( "Volume" ) + " (&Aring;<sup>3</sup>)" );
   titles[ 8 ] = us_label( "s<sub>20,w</sub> (S)" );
   titles[ 9 ] = us_label( "D<sub>20,w</sub> (cm<sup>2</sup>/sec)" );

   int row = 0;

   for ( int i = 0; i < 10; i++ )
   {
      titles[ i ]->setAlignment( Qt::AlignCenter );
      values->addWidget( titles[ i ], row, i );
   }

   row++;

   lb_sphere [ 0 ] = us_label( tr( "Sphere:" ) );
   lb_prolate[ 0 ] = us_label( tr( "Prolate:" ) );
   lb_oblate [ 0 ] = us_label( tr( "Oblate:" ) );
   lb_rod    [ 0 ] = us_label( tr( "Long Rod:" ) );

   for ( int i = 0; i < 10; i++ )
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
	update_plot();
   update();
}

void US_Predict1::update_plot( void )
{
   double x = 1.1;
   // From: K.E. van Holde, Biophysical Chemistry, 2nd edition, chapter 4.1
   // Calculate frictional ratio as a function of axial ratio
   for ( int i = 0; i < ARRAYSIZE; i++, x += max_x/ARRAYSIZE )
   {
      prolate[ i ] = pow( x, -1.0 / 3.0 ) * sqrt( sq( x ) - 1.0 ) / 
                     log( x + sqrt( sq( x ) - 1.0 ) );
      
      oblate[ i ]  = sqrt( sq( x ) - 1.0 ) / 
                     ( pow( x, 2.0 / 3.0 ) * atan( sqrt( sq( x ) - 1.0 ) ) );
      
      rod[ i ]     = pow( 2.0 / 3.0, 1.0 / 3.0 ) * pow( x, 2.0 / 3.0 ) / 
                     ( log( 2.0 * x ) - 0.3 );
      
      ratio_x[ i ] = x;
   }
   prolate_curve->setSamples( ratio_x, prolate, ARRAYSIZE );
   rod_curve->setSamples( ratio_x, rod, ARRAYSIZE );
   oblate_curve ->setSamples( ratio_x, oblate, ARRAYSIZE );
	double maxy = max(prolate_curve->maxYValue(), rod_curve->maxYValue());
   vline_x[ 0 ] = ratio;
   vline_x[ 1 ] = ratio;
   vline_y[ 0 ] = 0;
   vline_y[ 1 ] = maxy;

   vline_curve->setPen ( QPen( QBrush( Qt::white  ), 1.5 ) );
   vline_curve->setSamples( vline_x, vline_y, 2 );
   plot->setAxisScale(QwtPlot::xBottom, 1.0, max_x );
   plot->setAxisScale(QwtPlot::yLeft, 0.9, maxy );
	//plot->setAxisAutoScale( QwtPlot::yLeft );
   plot->setMinimumSize( 650, 350 );

	plot->replot();
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

void US_Predict1::update_ratio( void )
{
   QwtDoublePoint p( le_axial->text().toDouble(), 0.0 );
   mouseU( p );
}

void US_Predict1::update_mw( const QString& s )
{
   mw = s.toDouble();
   update();
}

void US_Predict1::update_max_x( const QString& s )
{
   max_x = s.toDouble();
   update();
   update_plot();
}

void US_Predict1::degC( const QString& s )
{
   temperature = s.toDouble();
   solution.vbar      = solution.vbar20 + 4.25e-4 * ( temperature - 20.0 );

   US_Math2::data_correction( temperature, solution );
   update();
}

void US_Predict1::get_peptide( void )
{
   US_AnalyteGui* dialog = 
      new US_AnalyteGui( true, analyte.analyteGUID );

   connect( dialog, SIGNAL( valueChanged( US_Analyte ) ),
                    SLOT  ( update_vbar ( US_Analyte ) ) );

   connect( dialog, SIGNAL( use_db        ( bool ) ),
                    SLOT  ( source_changed( bool ) ) );
   dialog->exec();
}

void US_Predict1::update_vbar( const US_Analyte ad )
{
   analyte         = ad;
   mw              = ad.mw;
   solution.vbar20 = ad.vbar20;

   le_mw  ->setText( QString::number( (int) mw,      'e', 3 ) );
   le_vbar->setText( QString::number( solution.vbar, 'f', 4 ) );

   US_Math2::data_correction( temperature, solution );
   update();
}

void US_Predict1::get_buffer( void )
{
   US_Buffer buffer;

   US_BufferGui* dialog = new US_BufferGui( true, buffer, access );

   connect( dialog, SIGNAL( valueChanged ( US_Buffer ) ),
                    SLOT  ( update_buffer( US_Buffer ) ) );

   connect( dialog, SIGNAL( use_db        ( bool ) ),
                    SLOT  ( source_changed( bool ) ) );

   dialog->exec();
}

void US_Predict1::update_buffer( const US_Buffer b )
{
   buffer = b;
   solution.density   = b.density;
   solution.viscosity = b.viscosity;

   le_density  ->setText( QString::number( solution.density,    'f', 4 ) );
   le_viscosity->setText( QString::number( solution.viscosity , 'f', 4 ) );

   US_Math2::data_correction( temperature, solution );
   update();
}

void US_Predict1::density( const QString& s )
{
   solution.density = s.toDouble();
   US_Math2::data_correction( temperature, solution );
   update();
}

void US_Predict1::viscosity( const QString& s )
{
   solution.viscosity = s.toDouble();
   US_Math2::data_correction( temperature, solution );
   update();
}

void US_Predict1::vbar( const QString& s )
{
   solution.vbar = s.toDouble();
   US_Math2::data_correction( temperature, solution );
   update();
}

void US_Predict1::new_value( const QwtDoublePoint& p )
{
   ratio = p.x();

   le_axial ->disconnect();
   le_axial->setText( QString::number( ratio, 'f', 3 ) );

   connect( le_axial, SIGNAL( editingFinished( void ) ), 
                      SLOT  ( update_ratio   ( void ) ) );
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
   
   else if ( ratio >= 6.0 && ratio <= 10000.0 )
      msg = tr( "Please select an axial ratio by\n"
                "dragging the white bar with the\n"
                "mouse to change the axial ratio" );

   else if ( ratio > 10000 )
   {
      msg = tr( "Attention:\n\n"
                "The upper axial ratio limit is 10000!" );
      ratio = 10000.0;
   }

   lb_info->setText( msg );

   vline_x[ 0 ] = ratio;
   vline_x[ 1 ] = ratio;
   vline_curve->setSamples( vline_x, vline_y, 2 );
   plot->replot();

   le_axial ->disconnect();
   
   le_axial->setText( QString::number( ratio, 'f', 3 ) );

   connect( le_axial, SIGNAL( editingFinished( void ) ), 
                      SLOT  ( update_ratio   ( void ) ) );
   update();
   //debug();
}

void US_Predict1::calc_column( const QString& name, const QString& unit, int column_index, double sphere_val,
                               double prolate_val, double oblate_val, double rod_val, double multiplier)
{
   double min_value = min( min( sphere_val, prolate_val ), min( oblate_val, rod_val ) ) * multiplier;
   double max_value = max( max( sphere_val, prolate_val ), max( oblate_val, rod_val ) ) * multiplier;
   int log_min = (int) ( log10( min_value ) ) ;
   int log_max = (int) ( log10( max_value ) ) ;
   int power_of_ten = 1e+0;
   char format_char = 'g';
   if ( abs( log_min - log_max ) <= 1 )
   {
      power_of_ten = log_max;
      format_char = 'f';
   }
   else
   {
      power_of_ten = log_min;
   }
   power_of_ten *= -1;
   if ( qFabs(max_value * multiplier * (double)pow(10, power_of_ten)) < 1 )
   {
      power_of_ten += (power_of_ten != 0) ? power_of_ten / abs( power_of_ten ) : 1;
   }

   titles[ column_index ] ->setText( name + ((unit.isEmpty())?"":" (" + unit + ")" ));

   if ( power_of_ten != 0 )
   {
      titles[ column_index ] ->setText( name + " (" + QString("e%1%2").
         arg(power_of_ten > 0 ? '+' : '-').arg(qFabs(power_of_ten),2,'f', 0, '0') + ((unit.isEmpty())?"":" " + unit) + ")" );
   }
   lb_sphere [ column_index ]->setText( QString::number( sphere_val * multiplier * (double)pow(10, power_of_ten), format_char, 4 ) );
   lb_prolate[ column_index ]->setText( QString::number( prolate_val * multiplier * (double)pow(10, power_of_ten), format_char, 4 ) );
   lb_oblate [ column_index ]->setText( QString::number( oblate_val * multiplier * (double)pow(10, power_of_ten), format_char, 4 ) );
   lb_rod    [ column_index ]->setText( QString::number( rod_val * multiplier * (double)pow(10, power_of_ten), format_char, 4 ) );

}

void US_Predict1::update()
{
   allparams.mw          = mw;
   allparams.vbar        = solution.vbar;
   allparams.density     = solution.density;
   allparams.viscosity   = solution.viscosity;
   allparams.temperature = temperature;
   allparams.axial_ratio = ratio;
   
   allparams.calculate( temperature );

   // fill every column with the calculated values
   calc_column( "s<sub>app</sub>", "S", 1,
                allparams.sphere.s, allparams.prolate.s, allparams.oblate.s, allparams.rod.s,
                1e+13 / solution.s20w_correction );
   calc_column( "D<sub>app</sub>", "cm<sup>2</sup>/sec", 2,
                allparams.sphere.D, allparams.prolate.D, allparams.oblate.D, allparams.rod.D,
                1 / solution.D20w_correction );
   calc_column( "f", "", 3,
                allparams.sphere.f, allparams.prolate.f, allparams.oblate.f, allparams.rod.f,
                1 );
   calc_column( "f / f0", "", 4,
                allparams.sphere.f_f0, allparams.prolate.f_f0, allparams.oblate.f_f0, allparams.rod.f_f0,
                1 );
   calc_column( "a", "<span>&Aring;</span>", 5,
                allparams.sphere.a, allparams.prolate.a, allparams.oblate.a, allparams.rod.a,
                1 );
   calc_column( "b", "<span>&Aring;</span>", 6,
                allparams.sphere.b, allparams.prolate.b, allparams.oblate.b, allparams.rod.b,
                1 );
   calc_column( "Volume", "<span>&Aring;<sup>3</sup></span>", 7,
                allparams.sphere.volume, allparams.prolate.volume, allparams.oblate.volume, allparams.rod.volume,
                1 );
   calc_column( "s<sub>20,w</sub>", "S", 8,
                allparams.sphere.s, allparams.prolate.s, allparams.oblate.s, allparams.rod.s,
                1e+13 );
   calc_column( "D<sub>20,w</sub>", "cm<sup>2</sup>/sec", 9,
                allparams.sphere.D, allparams.prolate.D, allparams.oblate.D, allparams.rod.D,
                1 );

   if ( signal ) emit changed();
}

void US_Predict1::source_changed( bool db )
{
   emit use_db( db );
   qApp->processEvents();
}

void US_Predict1::debug( void )
{
   /*
   US_Hydrosim sim;
   sim.mw          = allparams.mw;
   sim.density     = allparams.density;
   sim.viscosity   = allparams.viscosity;
   sim.vbar        = allparams.vbar;
   sim.axial_ratio = allparams.axial_ratio;

   sim.calculate( temperature );

   qDebug() << "sphere" << sim.sphere.s 
                        << sim.sphere.D
                        << sim.sphere.f
                        << sim.sphere.f_f0
                        << sim.sphere.a
                        << sim.sphere.b
                        << sim.sphere.volume;

   qDebug() << "prolate" << sim.prolate.s 
                        << sim.prolate.D
                        << sim.prolate.f
                        << sim.prolate.f_f0
                        << sim.prolate.a
                        << sim.prolate.b
                        << sim.prolate.volume;

   qDebug() << "oblate" << sim.oblate.s 
                        << sim.oblate.D
                        << sim.oblate.f
                        << sim.oblate.f_f0
                        << sim.oblate.a
                        << sim.oblate.b
                        << sim.oblate.volume;

   qDebug() << "rod" << sim.rod.s 
                        << sim.rod.D
                        << sim.rod.f
                        << sim.rod.f_f0
                        << sim.rod.a
                        << sim.rod.b
                        << sim.rod.volume;
*/
}

void US_Predict1::get_solution( void )
{
   US_SolutionGui* dialog = new US_SolutionGui( 1, 1, true );
   connect( dialog, SIGNAL( updateSolutionGuiSelection( US_Solution ) ),
                    SLOT  ( update_solution           ( US_Solution ) ) );
   dialog->setWindowTitle( tr( "Solutions" ) );
   dialog->exec();
}

void US_Predict1::update_solution( US_Solution soln )
{
   solution.density   = soln.buffer.density;
   solution.viscosity = soln.buffer.viscosity;

   le_density  ->setText( QString::number( solution.density,   'f', 4 ) );
   le_viscosity->setText( QString::number( solution.viscosity, 'f', 4 ) );

   US_Analyte analyte;

   if ( soln.analyteInfo.size() == 0 )
   {
      QMessageBox::warning( this,
            tr( "No Analyte" ),
            tr( "There is no analyte in the solution" ) );
      return;
   }
   else if ( soln.analyteInfo.size() == 1 )
   {
     analyte         = soln.analyteInfo[ 0 ].analyte;
     solution.vbar20 = analyte.vbar20;;
     solution.vbar   = US_Math2::adjust_vbar20( solution.vbar20, temperature );
     le_vbar->setText( QString::number( solution.vbar, 'f', 4 ) );

     mw = analyte.mw;
     le_mw->setText( QString::number( mw, 'e', 4 ) );
   }
   else
   {
     US_Choice* dialog = new US_Choice( soln );
     connect( dialog, SIGNAL( choice( int ) ),
                      SLOT  ( choose( int ) ) );
     dialog->exec();
     qApp->processEvents();

     analyte = soln.analyteInfo[ analyte_number ].analyte;
     solution.vbar20 = analyte.vbar20;;
     solution.vbar   = US_Math2::adjust_vbar20( solution.vbar20, temperature );
     le_vbar->setText( QString::number( solution.vbar, 'f', 4 ) );

     mw = analyte.mw;
     le_mw->setText( QString::number( mw, 'e', 4 ) );
   }

   update();
}

void US_Predict1::choose( int value )
{
   analyte_number = value;
}


