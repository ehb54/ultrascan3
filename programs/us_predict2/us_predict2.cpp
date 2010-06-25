//! \file us_predict2.cpp
#include "us_predict2.h"
#include "us_gui_settings.h"
#include "us_constants.h"
#include "us_buffer_gui.h"
#include "us_analyte_gui.h"
#include "us_constants.h"

US_Predict2::US_Predict2() : US_Widgets()
{
   model = None;

   temperature = NORMAL_TEMP;
   d.density   = DENS_20W;
   d.viscosity = VISC_20W * 100.0;
   d.vbar20    = TYPICAL_VBAR;
   d.vbar      = TYPICAL_VBAR + ( 4.25e-4 * ( temperature - 20.0 ) );

   US_Math2::data_correction( temperature, d );
   

   setWindowTitle( tr( "Predict f and axial ratios for 4 basic shapes" ) );
   setPalette( US_GuiSettings::frameColor() );

   QGridLayout* main = new QGridLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   int row = 0;

   QGridLayout* controls = new QGridLayout;
   int c_row = 0;

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

   QPushButton* pb_vbar = us_pushbutton( tr( "vbar (20 " ) + DEGC + ")" );
   connect( pb_vbar, SIGNAL( clicked() ), SLOT( get_peptide() ) );
   controls->addWidget( pb_vbar, c_row, 0 );

   le_vbar = us_lineedit();
   le_vbar->setText( QString::number( TYPICAL_VBAR ) );
   connect( le_vbar, SIGNAL( textChanged( const QString& ) ), 
                     SLOT  ( vbar       ( const QString& ) ) );
   controls->addWidget( le_vbar, c_row++, 1 );

   QLabel* lb_temperature = us_label( 
         tr( "Temperature (" ) + DEGC + "):" );
   controls->addWidget( lb_temperature, c_row, 0 );

   QLineEdit* le_temperature = us_lineedit();
   le_temperature->setText( QString::number( NORMAL_TEMP, 'f', 1 ) );
   connect( le_vbar, SIGNAL( textChanged( const QString& ) ), 
                     SLOT  ( degC       ( const QString& ) ) );
   controls->addWidget( le_temperature, c_row++, 1 );

   QLabel* lb_info = us_label( tr( "Select a Parameter Combination:" ) );
   controls->addWidget( lb_info, c_row++, 0, 1, 2 );

   lb_param1 = us_label( tr( "Sed. Coefficient:" ) );
   controls->addWidget( lb_param1, c_row, 0 );

   le_param1 = us_lineedit();
   le_param1->setText( "0.000" );
   connect( le_param1, SIGNAL( textChanged  ( const QString& ) ), 
                       SLOT  ( update_param1( const QString& ) ) );
   controls->addWidget( le_param1, c_row++, 1 );

   lb_param2 = us_label( tr( "Diff. Coefficient:" ) );
   controls->addWidget( lb_param2, c_row, 0 );

   le_param2 = us_lineedit();
   le_param2->setText( "0.000" );
   connect( le_param2, SIGNAL( textChanged  ( const QString& ) ), 
                       SLOT  ( update_param2( const QString& ) ) );
   controls->addWidget( le_param2, c_row++, 1 );

   main->addLayout( controls, row, 0, 7, 1 );

   // Labels
   QLabel* lb_model = us_label( tr( "Model:" ) );
   main->addWidget( lb_model, row, 2 );

   QLabel* lb_a = us_label( tr( "a (<span>&Aring;</span>)" ) );
   lb_a->setAlignment( Qt::AlignCenter );
   main->addWidget( lb_a, row, 3 );

   QLabel* lb_b = us_label( tr( "b (<span>&Aring;</span>)" ) );
   lb_b->setAlignment( Qt::AlignCenter );
   main->addWidget( lb_b, row, 4 );

   QLabel* lb_ab = us_label( tr( "a/b" ) );
   lb_ab->setAlignment( Qt::AlignCenter );
   main->addWidget( lb_ab, row++, 5 );

   // Prolate
   QLabel* lb_prolate = us_label( tr( "Prolate:" ) );
   main->addWidget( lb_prolate, row, 2 );

   le_prolate_a = us_lineedit();
   le_prolate_a->setReadOnly( true );
   main->addWidget( le_prolate_a, row, 3 );

   le_prolate_b = us_lineedit();
   le_prolate_b->setReadOnly( true );
   main->addWidget( le_prolate_b, row, 4 );

   le_prolate_ab = us_lineedit();
   le_prolate_ab->setReadOnly( true );
   main->addWidget( le_prolate_ab, row++, 5 );

   // Oblage
   QLabel* lb_oblate = us_label( tr( "Oblate:" ) );
   main->addWidget( lb_oblate, row, 2 );

   le_oblate_a = us_lineedit();
   le_oblate_a->setReadOnly( true );
   main->addWidget( le_oblate_a, row, 3 );

   le_oblate_b = us_lineedit();
   le_oblate_b->setReadOnly( true );
   main->addWidget( le_oblate_b, row, 4 );

   le_oblate_ab = us_lineedit();
   le_oblate_ab->setReadOnly( true );
   main->addWidget( le_oblate_ab, row++, 5 );

   // Long Rod
   QLabel* lb_rod = us_label( tr( "Long Rod:" ) );
   main->addWidget( lb_rod, row, 2 );

   le_rod_a = us_lineedit();
   le_rod_a->setReadOnly( true );
   main->addWidget( le_rod_a, row, 3 );

   le_rod_b = us_lineedit();
   le_rod_b->setReadOnly( true );
   main->addWidget( le_rod_b, row, 4 );

   le_rod_ab = us_lineedit();
   le_rod_ab->setReadOnly( true );
   main->addWidget( le_rod_ab, row++, 5 );

   // Misc items

   QLabel* lb_fCoef = us_label( tr( "Frict. Coefficient:" ) );
   main->addWidget( lb_fCoef, row, 2 );

   le_fCoef = us_lineedit();
   le_fCoef->setReadOnly( true );
   main->addWidget( le_fCoef, row, 3 );

   QLabel* lb_r0 = us_label( tr( "R0 (Sphere):" ) );
   main->addWidget( lb_r0, row, 4 );

   le_r0 = us_lineedit();
   le_r0->setReadOnly( true );
   main->addWidget( le_r0, row++, 5 );

   lb_param3 = us_label( tr( "Molecular Weight:" ) );
   main->addWidget( lb_param3, row, 2 );

   le_param3 = us_lineedit();
   le_param3->setReadOnly( true );
   main->addWidget( le_param3, row, 3 );

   QLabel* lb_f0 = us_label( tr( "f0 (Sphere):" ) );
   main->addWidget( lb_f0, row, 4 );

   le_f0 = us_lineedit();
   le_f0->setReadOnly( true );
   main->addWidget( le_f0, row++, 5 );

   QLabel* lb_volume = us_label( tr( "Volume (&Aring;<sup>3</sup>):" ) );
   main->addWidget( lb_volume, row, 2 );

   le_volume = us_lineedit();
   le_volume->setReadOnly( true );
   main->addWidget( le_volume, row, 3 );

   QLabel* lb_ff0 = us_label( tr( "f/f0:" ) );
   main->addWidget( lb_ff0, row, 4 );

   le_ff0 = us_lineedit();
   le_ff0->setReadOnly( true );
   main->addWidget( le_ff0, row++, 5 );

   // Button rows

   QGridLayout* buttons = new QGridLayout;
   int b_row = 0;

   pb_mw_s = us_pushbutton( 
         tr( "Molecular Weight + Sedimentation Coeff." ) );
   connect( pb_mw_s, SIGNAL( clicked() ), SLOT( do_mw_s() ) );
   buttons->addWidget( pb_mw_s, b_row, 0 );

   pb_mw_d = us_pushbutton( 
         tr( "Molecular Weight + Diffusion Coeff." ) );
   connect( pb_mw_d, SIGNAL( clicked() ), SLOT( do_mw_d() ) );
   buttons->addWidget( pb_mw_d, b_row, 1 );

   pb_s_d = us_pushbutton( 
         tr( "Sedimentation Coeff. + Diffusion Coeff." ) );
   connect( pb_s_d, SIGNAL( clicked() ), SLOT( do_s_d() ) );
   buttons->addWidget( pb_s_d, b_row++, 2 );

   QPushButton* pb_reset = us_pushbutton( tr( "Update" ) );
   connect( pb_reset, SIGNAL( clicked() ), SLOT( update() ) );
   buttons->addWidget( pb_reset, b_row, 0 );

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons->addWidget( pb_help, b_row, 1 );

   QPushButton* pb_accept = us_pushbutton( tr( "Close" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( close() ) );
   buttons->addWidget( pb_accept, b_row++, 2 );

   row = 8;
   main->addLayout( buttons, row, 0, 1, 6 );
}

void US_Predict2::do_mw_s()
{
   model = MW_SEDIMENTATION;
   le_param1->setText( "0.000");
   le_param2->setText( "0.000");
   
   sed_coeff = 0.0;
   mw        = 0.0;
   
   lb_param1->setText( tr( "Molecular Weight:" ) );
   lb_param2->setText( tr( "Sed. Coefficient:" ) );
   lb_param3->setText( tr( "Diff. Coefficient:" ) );

   QPalette p = US_GuiSettings::pushbColor();
   pb_mw_d->setPalette( p );
   pb_s_d ->setPalette( p );

   p.setColor( QPalette::Button, Qt::darkGreen );
   pb_mw_s->setPalette( p );
}

void US_Predict2::do_mw_d()
{
   model = MW_DIFFUSION;
   le_param1->setText( "0.000" );
   le_param2->setText( "0.000" );
   
   diff_coeff = 0.0;
   mw         = 0.0;

   lb_param1->setText( tr( "Molecular Weight:" ) );
   lb_param2->setText( tr( "Diff. Coefficient:" ) );
   lb_param3->setText( tr( "Sed. Coefficient:" ) );

   QPalette p = US_GuiSettings::pushbColor();
   pb_s_d ->setPalette( p );
   pb_mw_s->setPalette( p );

   p.setColor( QPalette::Button, Qt::darkGreen );
   pb_mw_d->setPalette( p );
}

void US_Predict2::do_s_d()
{
   model = SED_DIFF;
   le_param1->setText( "0.000" );
   le_param2->setText( "0.000" );
   
   sed_coeff  = 0.0;
   diff_coeff = 0.0;
   
   lb_param1->setText( tr( "Sed. Coefficient:" ) );
   lb_param2->setText( tr( "Diff. Coefficient:" ) );
   lb_param3->setText( tr( "Molecular Weight:" ) );

   QPalette p = US_GuiSettings::pushbColor();
   pb_mw_d->setPalette( p );
   pb_mw_s->setPalette( p );

   p.setColor( QPalette::Button, Qt::darkGreen );
   pb_s_d ->setPalette( p );
}

void US_Predict2::update( void )
{
   double vol_per_molecule;
   double rad_sphere;
   double f0;
   double frict_coeff;

   double t = temperature + K0;

   if ( model == None )
   {
      QMessageBox::information( this,
            tr( "Please Note:" ), 
            tr( "You have to define a parameter combination first!" ) );
      return;
   }

   if ( model == MW_SEDIMENTATION )
   {
      if ( mw == 0.0 )
      {
         QMessageBox::information( this,
               tr( "Please Note:" ), 
               tr( "Please define the Molecular Weight first!" ) );
         return;
      }

      if ( sed_coeff == 0.0 )
      {
         QMessageBox::information( this,
               tr( "Please Note:" ), 
               tr( "Please define a Sedimentation Coefficient first!" ) );
         return;
      }

      vol_per_molecule = d.vbar * mw / AVOGADRO;
      rad_sphere       = pow( vol_per_molecule * ( 3.0 / 4.0 ) / M_PI, 1.0 / 3.0 );
      f0               = rad_sphere * 6.0 * M_PI * d.viscosity_tb * 0.01;

      // Recaluclate volume to put into cubic angstroms:

      vol_per_molecule = ( 4.0 / 3.0 ) * M_PI * pow( rad_sphere * 1.0e+08,  3.0 );
      diff_coeff       = sed_coeff * R * t / ( d.buoyancyb * mw );
      frict_coeff      = mw * d.buoyancyb / ( sed_coeff * AVOGADRO );
      
      if ( ! check_valid( frict_coeff, f0 ) ) return;

      le_param3->setText( QString::number( diff_coeff, 'e', 4 ) );
   }

   if ( model == MW_DIFFUSION )
   {
      if ( mw == 0 )
      {
         QMessageBox::information( this,
               tr( "Please Note:" ), 
               tr( "Please define the Molecular Weight first!" ) );
         return;
      }

      if ( diff_coeff == 0 )
      {
         QMessageBox::information( this,
               tr( "Please Note:" ), 
               tr( "Please define a Diffusion Coefficient first!" ) );
         return;
      }

      vol_per_molecule = d.vbar * mw / AVOGADRO;
      rad_sphere       = pow( vol_per_molecule * ( 3.0 / 4.0 ) / M_PI, 1.0 / 3.0 );
      f0               = rad_sphere * 6.0 * M_PI * d.viscosity_tb * 0.01;

      // Recalculate volume to put into cubic angstroms:

      vol_per_molecule = ( 4.0 / 3.0 ) * M_PI * pow( rad_sphere * 1.0e+08, 3.0 );
      sed_coeff        = diff_coeff * d.buoyancyb * mw / ( R * t );
      frict_coeff      = mw * d.buoyancyb / ( sed_coeff * AVOGADRO );
      
      if ( ! check_valid( frict_coeff, f0 ) ) return;
      
      le_param3->setText( QString::number( sed_coeff, 'e', 4 ) );
   }

   if ( model == SED_DIFF )
   {
      if ( sed_coeff == 0 )
      {
         QMessageBox::information( this,
               tr( "Please Note:" ), 
               tr( "Please define the Sedimentation Coefficient first!" ) );
         return;
      }

      if ( diff_coeff == 0 )
      {
         QMessageBox::information( this,
               tr( "Please Note:" ), 
               tr( "Please define a Diffusion Coefficient first!" ) );
         return;
      }

      mw               = sed_coeff * R * t / ( diff_coeff * d.buoyancyb );
      vol_per_molecule = d.vbar * mw / AVOGADRO;
      rad_sphere       = pow( vol_per_molecule * ( 3.0 / 4.0 ) / M_PI, 1.0 / 3.0 );
      f0               = rad_sphere * 6.0 * M_PI * d.viscosity_tb * 0.01;


      // Recaluclate volume to put into cubic angstroms:

      vol_per_molecule = ( 4.0 / 3.0 ) * M_PI * pow( rad_sphere * 1.0e+08 , 3.0 );
      frict_coeff      = mw * d.buoyancyb / ( sed_coeff * AVOGADRO );
      
      if ( ! check_valid( frict_coeff, f0 ) ) return;
     
      le_param3->setText( QString::number( mw, 'e', 4 ) );
   }

   le_fCoef ->setText( QString::number( frict_coeff,          'e', 4 ) );
   le_r0    ->setText( QString::number( rad_sphere * 1.0e+08, 'e', 4 ) );
   le_f0    ->setText( QString::number( f0,                   'e', 4 ) );
   le_volume->setText( QString::number( vol_per_molecule,     'e', 4 ) );
   le_ff0   ->setText( QString::number( frict_coeff / f0,     'e', 4 ) );

   // prolate ellipsoid

   double ff0 = frict_coeff / f0;
   double ratio = root( PROLATE, ff0 );
   double ap = 1.0e+08 * rad_sphere * pow( ratio, 2.0 / 3.0 );
   double bp = ap / ratio;

   le_prolate_a ->setText( QString::number( ap,    'e', 4 ) );
   le_prolate_b ->setText( QString::number( bp,    'e', 4 ) );
   le_prolate_ab->setText( QString::number( ratio, 'e', 4 ) );

   // oblate ellipsoid

   ratio = root( OBLATE, ff0 );
   double bo = 1.0e+08 * rad_sphere / pow( ratio, 2.0 / 3.0 );
   double ao = ratio * bo;
   le_oblate_a ->setText( QString::number( ao,    'e', 4 ) );
   le_oblate_b ->setText( QString::number( bo,    'e', 4 ) );
   le_oblate_ab->setText( QString::number( ratio, 'e', 4 ) );

   // long rod

   if ( ff0 > 1.32 )
   {
      ratio = root( ROD, ff0 );
      double br = 1.0e+08 * pow( 2.0/ ( 3.0 * ratio ), 1.0 / 3.0 ) * rad_sphere;
      double ar = ratio * br;
      le_rod_a ->setText( QString::number( ar, 'e', 4 ) );
      le_rod_b ->setText( QString::number( br, 'e', 4 ) );
      le_rod_ab->setText( QString::number( ratio, 'e', 4 ) );
   }
   else
   {
      le_rod_a ->setText( "f/f0 < 1.32, n/a" );
      le_rod_b ->setText( "f/f0 < 1.32, n/a" );
      le_rod_ab->setText( "f/f0 < 1.32, n/a" );
   }
}

bool US_Predict2::check_valid( double f, double f0 )
{
   if (  f / f0  < 1.0 )
   {
      QMessageBox::information( this,
            tr( "Attention:" ), 
            tr( "This model is physically impossible!\n"
                "The f/f0 ratio is less than 1." ) );
      
      sed_coeff  = 0.0;
      diff_coeff = 0.0;
      mw         = 0.0;
      
      le_param1->setText( "0.000" );
      le_param2->setText( "0.000" );
      
      return false;
   }
   
   return true;
}

/*!
   This function uses the frictional coefficient ration to solve for the ratio
   of a/b of a model where a and b are the axes of a prolate ellipsoid or an
   oblate ellipsoid.  In the case of a long rod, a is the half length of the
   rod and b is the radius.  
   \param shape The shape assumed for the calculation
   \param goal  The f/f0 value used to for the calculation
*/

double US_Predict2::root( int shape, double goal )
{
   double test;
   double x     = 2500.0; // half-way point      
   double x1    = 1.1;    // lower range limit;
   double x2    = 5000.0; // upper range limit
   double xdiff = 1.0;

   if ( shape == PROLATE )
   {
      do
      {
         test = goal - pow( x, -1.0 / 3.0 ) * sqrt( sq( x ) - 1.0 ) / 
                       log( x + sqrt( sq( x ) - 1.0 ) );

         // Iterate until the difference between subsequent x value evaluations is
         // too small to be relevant
         
         // Note: this test is only valid for monotone increasing functional
         // values!  If the difference between the goal and the test value is
         // negative, then the x value was too high. We then adjust the top
         // (x2) limit. Otherwise, we adjust the lower limit:

         if ( test < 0.0 ) 
            x2 = x;
         else
            x1 = x;

         // We adjust the new test x-value by adding half the stepsize from the
         // last step to the lower limit:

         xdiff = ( x2 - x1 ) / 2.0;
         x = x1 + xdiff;

      } while ( fabs( xdiff / x ) > 1.0e-4 );
   }

   else if ( shape == OBLATE )
   {
      do
      {         
         test = goal - sqrt( sq( x ) - 1.0 ) / 
                    ( pow( x, 2.0 / 3.0 ) * atan( sqrt( sq( x ) - 1.0 ) ) );

         // Note: this test is only valid for monotone increasing functional
         // values!  If the difference between the goal and the test value is
         // negative, then the x value was too high. We then adjust the top
         // (x2) limit. Otherwise, we adjust the lower limit:

         if ( test < 0.0 ) 
            x2 = x;
         else
            x1 = x;

         // We adjust the new test x-value by adding half the stepsize from the
         // last step to the lower limit:

         xdiff = ( x2 - x1 ) / 2.0;
         x = x1 + xdiff;

      } while ( fabs( xdiff / x ) > 1.0e-4 );
   }

   else if ( shape == ROD )
   {
      x1    = 6.0;     // lower range limit
      x     = 2497.0;  // half-way point

      do
      {
         test = goal - pow( 2.0 / 3.0, 1.0 / 3.0 ) * pow( x, 2.0 / 3.0 ) / 
                       ( log( 2.0 * x ) - 0.3 );

         // Note: this test is only valid for monotone increasing functional
         // values!  If the difference between the goal and the testvalue is
         // negative, then the x value was too high. We then adjust the top
         // (x2) limit:

         if ( test < 0.0 ) 
            x2 = x;

         // Otherwise, we adjust the lower limit:

         else
            x1 = x;

         // We adjust the new test x-value by adding half the stepsize from the
         // last step to the lower limit:

         xdiff = ( x2 - x1 ) / 2.0;
         x = x1 + xdiff;

      } while ( fabs( xdiff / x ) > 1.0e-4 );
   }

   return x;
}

void US_Predict2::update_param1( const QString& s )
{
   if ( model == SED_DIFF )
      sed_coeff = s.toDouble();
   else
      mw = s.toDouble();
}

void US_Predict2::update_param2( const QString& s )
{
   if ( model == MW_SEDIMENTATION )
      sed_coeff  = s.toDouble();
   else
      diff_coeff = s.toDouble();
}

void US_Predict2::density( const QString& s )
{
   d.density = s.toDouble();
   US_Math2::data_correction( temperature, d );
   update();
}

void US_Predict2::viscosity( const QString& s )
{
   d.viscosity = s.toDouble();
   US_Math2::data_correction( temperature, d );
   update();
}

void US_Predict2::vbar( const QString& s )
{
   d.vbar = s.toDouble();
   US_Math2::data_correction( temperature, d );
   update();
}

void US_Predict2::degC( const QString& s )
{
   temperature = s.toDouble();
   US_Math2::data_correction( temperature, d );
   update();
}

void US_Predict2::get_buffer( void )
{
   if ( model == None  ||  le_param1->text().toDouble() == 0.0  
                       ||  le_param2->text().toDouble() == 0.0 )
   {
      QMessageBox::information( this,
            tr( "Attention" ),
            tr( "Select a parameter combination and define\n"
                "those parameters first.") );
      return;
   }

   US_BufferGui* buffer_dialog = new US_BufferGui( true );
   connect( buffer_dialog, SIGNAL( valueChanged ( double, double ) ),
                           SLOT  ( update_buffer( double, double ) ) );
   buffer_dialog->setWindowTitle( tr( "Buffer Calculation" ) );
   buffer_dialog->exec();
}

void US_Predict2::update_buffer( double density, double viscosity )
{
   d.density   = density;
   d.viscosity = viscosity;

   le_density  ->setText( QString::number( density,   'f', 4 ) );
   le_viscosity->setText( QString::number( viscosity, 'f', 4 ) );

   US_Math2::data_correction( temperature, d );
   update();
}

void US_Predict2::get_peptide( void )
{
   if ( model == None  ||  le_param1->text().toDouble() == 0.0  
                       ||  le_param2->text().toDouble() == 0.0 )
   {
      QMessageBox::information( this,
            tr( "Attention" ),
            tr( "Select a parameter combination and define\n"
                "those parameters first.") );
      return;
   }

   US_AnalyteGui* dialog = new US_AnalyteGui( -1, true );
   connect( dialog, SIGNAL( valueChanged( double ) ),
                    SLOT  ( update_vbar ( double ) ) );
   dialog->setWindowTitle( tr( "VBar Calculation" ) );
   dialog->exec();
}

void US_Predict2::update_vbar( double vbar )
{
   d.vbar   = vbar;

   le_vbar  ->setText( QString::number( vbar, 'f', 4 ) );

   US_Math2::data_correction( temperature, d );
   update();
}

