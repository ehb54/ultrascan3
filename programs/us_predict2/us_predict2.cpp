//! \file us_predict2.cpp
#include "us_predict2.h"
#include "us_gui_settings.h"
#include "us_constants.h"
#include "us_model.h"
#include "us_buffer_gui.h"
#include "us_analyte_gui.h"
#include "us_constants.h"
#include "us_solution_gui.h"
#include "us_choice.h"

US_Predict2::US_Predict2() : US_Widgets()
{
   model = SED_DIFF;

   d.density   = DENS_20W;
   d.viscosity = VISC_20W;
   d.vbar20    = TYPICAL_VBAR;
   d.vbar      = d.vbar20;

   setWindowTitle( tr( "Predict f and axial ratios for 4 basic shapes" ) );
   setPalette( US_GuiSettings::frameColor() );

   QGridLayout* main = new QGridLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   int row = 0;

   QGridLayout* controls = new QGridLayout;
   int c_row = 0;

   QLabel*      lb_density = us_label( tr( "Density (20w)" ) );
   controls->addWidget( lb_density, c_row, 0 );

   le_density = us_lineedit();
   le_density->setText( QString::number( DENS_20W, 'f', 4 ) );
   us_setReadOnly( le_density,   true );
   controls->addWidget( le_density, c_row++, 1 );

   QLabel* lb_viscosity = us_label( tr( "Viscosity (20w)" ) );
   controls->addWidget( lb_viscosity, c_row, 0 );

   le_viscosity = us_lineedit();
   le_viscosity->setText( QString::number( VISC_20W , 'f', 4 ) );
   us_setReadOnly( le_viscosity, true );
   controls->addWidget( le_viscosity, c_row++, 1 );

   QPushButton* pb_vbar = us_pushbutton( tr( "vbar (20 " ) + DEGC + ")" );
   connect( pb_vbar, SIGNAL( clicked() ), SLOT( get_peptide() ) );
   controls->addWidget( pb_vbar, c_row, 0 );

   le_vbar = us_lineedit();
   le_vbar->setText( QString::number( TYPICAL_VBAR, 'f', 4 ) );
   connect( le_vbar, SIGNAL( textChanged( const QString& ) ), 
                     SLOT  ( vbar       ( const QString& ) ) );
   controls->addWidget( le_vbar, c_row++, 1 );

   QLabel* lb_temperature = us_label( tr( "Temperature (" ) + DEGC + "):" );
   controls->addWidget( lb_temperature, c_row, 0 );

   QLineEdit* le_temperature = us_lineedit();
   le_temperature->setText( QString::number( NORMAL_TEMP, 'f', 1 ) );
   us_setReadOnly( le_temperature, true );
   controls->addWidget( le_temperature, c_row++, 1 );

   QLabel* lb_info = us_label( tr( "Select a Parameter Combination:" ) );
   controls->addWidget( lb_info, c_row++, 0, 1, 2 );

   lb_param1 = us_label( tr( "Sed. Coefficient:" ) );
   controls->addWidget( lb_param1, c_row, 0 );

   le_param1 = us_lineedit();
   connect( le_param1, SIGNAL( textChanged  ( const QString& ) ), 
                       SLOT  ( update_param1( const QString& ) ) );
   controls->addWidget( le_param1, c_row++, 1 );

   lb_param2 = us_label( tr( "Diff. Coefficient:" ) );
   controls->addWidget( lb_param2, c_row, 0 );

   le_param2 = us_lineedit();
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
   us_setReadOnly( le_prolate_a, true );
   main->addWidget( le_prolate_a, row, 3 );

   le_prolate_b = us_lineedit();
   us_setReadOnly( le_prolate_b, true );
   main->addWidget( le_prolate_a, row, 3 );
   main->addWidget( le_prolate_b, row, 4 );

   le_prolate_ab = us_lineedit();
   us_setReadOnly( le_prolate_ab, true );
   main->addWidget( le_prolate_ab, row++, 5 );

   // Oblage
   QLabel* lb_oblate = us_label( tr( "Oblate:" ) );
   main->addWidget( lb_oblate, row, 2 );

   le_oblate_a = us_lineedit();
   us_setReadOnly( le_oblate_a,  true );
   main->addWidget( le_oblate_a, row, 3 );

   le_oblate_b = us_lineedit();
   us_setReadOnly( le_oblate_b,  true );
   main->addWidget( le_oblate_b, row, 4 );

   le_oblate_ab = us_lineedit();
   us_setReadOnly( le_oblate_ab, true );
   main->addWidget( le_oblate_ab, row++, 5 );

   // Long Rod
   QLabel* lb_rod = us_label( tr( "Long Rod:" ) );
   main->addWidget( lb_rod, row, 2 );

   le_rod_a = us_lineedit();
   us_setReadOnly( le_rod_a,  true );
   main->addWidget( le_rod_a, row, 3 );

   le_rod_b = us_lineedit();
   us_setReadOnly( le_rod_b,  true );
   main->addWidget( le_rod_b, row, 4 );

   le_rod_ab = us_lineedit();
   us_setReadOnly( le_rod_ab, true );
   main->addWidget( le_rod_ab, row++, 5 );

   // Misc items

   QLabel* lb_fCoef = us_label( tr( "Frict. Coefficient:" ) );
   main->addWidget( lb_fCoef, row, 2 );

   le_fCoef = us_lineedit();
   us_setReadOnly( le_fCoef,  true );
   main->addWidget( le_fCoef, row, 3 );

   QLabel* lb_r0 = us_label( tr( "R0 (Sphere):" ) );
   main->addWidget( lb_r0, row, 4 );

   le_r0 = us_lineedit();
   us_setReadOnly( le_r0,     true );
   main->addWidget( le_r0, row++, 5 );

   lb_param3 = us_label( tr( "Molecular Weight:" ) );
   main->addWidget( lb_param3, row, 2 );

   le_param3 = us_lineedit();
   us_setReadOnly( le_param3, true );
   main->addWidget( le_param3, row, 3 );

   QLabel* lb_f0 = us_label( tr( "f0 (Sphere):" ) );
   main->addWidget( lb_f0, row, 4 );

   le_f0 = us_lineedit();
   us_setReadOnly( le_f0,     true );
   main->addWidget( le_f0, row++, 5 );

   QLabel* lb_volume = us_label( tr( "Volume (&Aring;<sup>3</sup>):" ) );
   main->addWidget( lb_volume, row, 2 );

   le_volume = us_lineedit();
   us_setReadOnly( le_volume, true );
   main->addWidget( le_volume, row, 3 );

   QLabel* lb_ff0 = us_label( tr( "f/f0:" ) );
   main->addWidget( lb_ff0, row, 4 );

   le_ff0 = us_lineedit();
   us_setReadOnly( le_ff0,    true );
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
   le_param1->clear();
   le_param2->clear();
   
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
   le_param1->clear();
   le_param2->clear();
   
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
   le_param1->clear();
   le_param2->clear();
   
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
   if ( le_param1->text().toDouble() == 0.0  ||
        le_param2->text().toDouble() == 0.0 )
   {
      le_prolate_a ->clear();
      le_prolate_b ->clear();
      le_prolate_ab->clear();
      le_oblate_a  ->clear();
      le_oblate_b  ->clear();
      le_oblate_ab ->clear();
      le_rod_a     ->clear();
      le_rod_b     ->clear();
      le_rod_ab    ->clear();
      le_fCoef     ->clear();
      le_f0        ->clear();
      le_r0        ->clear();
      le_volume    ->clear();
      le_ff0       ->clear();
      le_param3    ->clear();
      return;
   }

   US_Model::SimulationComponent sc;
   double vol_per_molecule;
   double rad_sphere;
   double f0;
   double frict_coeff = 0.0;

   sc.vbar20      = d.vbar20;
   sc.mw          = 0.0;
   sc.s           = 0.0;
   sc.D           = 0.0;
   sc.f           = 0.0;
   sc.f_f0        = 0.0;

   if ( model == MW_SEDIMENTATION )
   {
      if ( mw        == 0.0 ) return;
      if ( sed_coeff == 0.0 ) return;

      sc.mw            = mw;
      sc.s             = sed_coeff;

      US_Model::calc_coefficients( sc );

      diff_coeff       = sc.D;
      frict_coeff      = sc.f;

      le_param3->setText( QString::number( diff_coeff, 'e', 4 ) );
   }

   if ( model == MW_DIFFUSION )
   {
      if ( mw         == 0.0 ) return;
      if ( diff_coeff == 0.0 ) return;

      sc.mw            = mw;
      sc.D             = diff_coeff;

      US_Model::calc_coefficients( sc );

      sed_coeff        = sc.s;
      frict_coeff      = sc.f;
      
      le_param3->setText( QString::number( sed_coeff, 'e', 4 ) );
   }

   if ( model == SED_DIFF )
   {
      if ( sed_coeff  == 0.0 ) return;
      if ( diff_coeff == 0.0 ) return;

      sc.s             = sed_coeff;
      sc.D             = diff_coeff;

      US_Model::calc_coefficients( sc );

      mw               = sc.mw;
      frict_coeff      = sc.f;
      
      le_param3->setText( QString::number( mw, 'e', 4 ) );
   }

   vol_per_molecule = d.vbar * mw / AVOGADRO;
   rad_sphere       = pow( vol_per_molecule * 0.75 / M_PI, 1.0 / 3.0 );

   double ff0   = sc.f_f0;
   double r0    = rad_sphere * 1.0e+08;
   f0           = frict_coeff / ff0;

   if ( ! check_valid( ff0 ) )
   {
      return;
   }

   // Recalculate volume to put into cubic angstroms:
   vol_per_molecule = 4.0 * M_PI / 3.0 * pow( r0, 3.0 );
      
   le_fCoef ->setText( QString::number( frict_coeff,      'e', 4 ) );
   le_r0    ->setText( QString::number( r0,               'e', 4 ) );
   le_f0    ->setText( QString::number( f0,               'e', 4 ) );
   le_volume->setText( QString::number( vol_per_molecule, 'e', 4 ) );
   le_ff0   ->setText( QString::number( ff0,              'f', 4 ) );

   // prolate ellipsoid
   double ratio = root( PROLATE, ff0 );
   double ap    = r0 * pow( ratio, 2.0 / 3.0 );
   double bp    = ap / ratio;

   le_prolate_a ->setText( QString::number( ap,    'e', 4 ) );
   le_prolate_b ->setText( QString::number( bp,    'e', 4 ) );
   le_prolate_ab->setText( QString::number( ratio, 'e', 4 ) );

   // oblate ellipsoid
   ratio        = root( OBLATE, ff0 );
   double bo    = r0 / pow( ratio, 2.0 / 3.0 );
   double ao    = ratio * bo;
   le_oblate_a ->setText( QString::number( ao,    'e', 4 ) );
   le_oblate_b ->setText( QString::number( bo,    'e', 4 ) );
   le_oblate_ab->setText( QString::number( ratio, 'e', 4 ) );

   // long rod
   if ( ff0 > 1.32 )
   {
      ratio        = root( ROD, ff0 );
      double br    = r0 * pow( 2.0 / ( 3.0 * ratio ), 1.0 / 3.0 );
      double ar    = ratio * br;
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

bool US_Predict2::check_valid( double f_f0 )
{
   if (  f_f0  < 1.0 )
   {
      //QMessageBox::information( this,
      //      tr( "Attention:" ), 
      //      tr( "This model is physically impossible!\n"
      //          "The f/f0 ratio (%1) is less than 1." ).arg( f_f0 ) );
      
      /*
      sed_coeff  = 0.0;
      diff_coeff = 0.0;
      mw         = 0.0;
      
      le_param1->clear();
      le_param2->clear();
      */
      le_prolate_a ->clear();
      le_prolate_b ->clear();
      le_prolate_ab->clear();
      le_oblate_a  ->clear();
      le_oblate_b  ->clear();
      le_oblate_ab ->clear();
      le_rod_a     ->clear();
      le_rod_b     ->clear();
      le_rod_ab    ->clear();
      le_fCoef     ->clear();
      le_f0        ->clear();
      le_r0        ->clear();
      le_volume    ->clear();
      le_param3    ->clear();
      le_ff0       ->setText( QString::number( f_f0, 'f', 4 ) + " !!!!" );
      
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

   update();
}

void US_Predict2::update_param2( const QString& s )
{
   if ( model == MW_SEDIMENTATION )
      sed_coeff  = s.toDouble();
   else
      diff_coeff = s.toDouble();

   update();
}

void US_Predict2::density( const QString& s )
{
   d.density = s.toDouble();
   update();
}

void US_Predict2::viscosity( const QString& s )
{
   d.viscosity = s.toDouble();
   update();
}

void US_Predict2::vbar( const QString& s )
{
   d.vbar20    = s.toDouble();
   d.vbar      = d.vbar;
   update();
}

void US_Predict2::degC( const QString& s )
{
   temperature = s.toDouble();
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
   connect( buffer_dialog, SIGNAL( BufferDataChanged ( double, double ) ),
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

   update();
}

void US_Predict2::get_peptide( void )
{
#if 0
   if ( model == None  ||  le_param1->text().toDouble() == 0.0  
                       ||  le_param2->text().toDouble() == 0.0 )
   {
      QMessageBox::information( this,
            tr( "Attention" ),
            tr( "Select a parameter combination and define\n"
                "those parameters first.") );
      return;
   }
#endif

   US_AnalyteGui* dialog = new US_AnalyteGui( true );
   connect( dialog, SIGNAL( valueChanged( US_Analyte ) ),
                    SLOT  ( update_vbar ( US_Analyte ) ) );
   dialog->setWindowTitle( tr( "VBar Calculation" ) );
   dialog->exec();
}

void US_Predict2::update_vbar( US_Analyte analyte )
{
   d.vbar   = analyte.vbar20;
   le_vbar  ->setText( QString::number( d.vbar, 'f', 4 ) );

   update();
}

void US_Predict2::get_solution( void )
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

   US_SolutionGui* dialog = new US_SolutionGui( 1, 1, true );
   connect( dialog, SIGNAL( updateSolutionGuiSelection( US_Solution ) ),
                    SLOT  ( update_solution           ( US_Solution ) ) );
   dialog->setWindowTitle( tr( "VBar Calculation" ) );
   dialog->exec();
}

void US_Predict2::update_solution( US_Solution solution )
{
   d.density   = solution.buffer.density;
   d.viscosity = solution.buffer.viscosity;

   le_density  ->setText( QString::number( d.density,   'f', 4 ) );
   le_viscosity->setText( QString::number( d.viscosity, 'f', 4 ) );

   US_Analyte analyte;
   
   if ( solution.analyteInfo.size() == 0 )
   {
      QMessageBox::warning( this, 
            tr( "No Analyte" ),
            tr( "There is no analyte in the solution" ) );
      return;
   }
   else if ( solution.analyteInfo.size() == 1 )
   {
     analyte = solution.analyteInfo[ 0 ].analyte;
     d.vbar= analyte.vbar20;;
     le_vbar->setText( QString::number( d.vbar, 'f', 4 ) );
   }
   else
   {
     US_Choice* dialog = new US_Choice( solution );
     connect( dialog, SIGNAL( choice( int ) ),
                      SLOT  ( choose( int ) ) );
     dialog->exec();
     qApp->processEvents();

     analyte = solution.analyteInfo[ analyte_number ].analyte;
     d.vbar= analyte.vbar20;;
     le_vbar->setText( QString::number( d.vbar, 'f', 4 ) );
   }

   if ( model == MW_DIFFUSION  ||  model == MW_SEDIMENTATION )
   {
      le_param1->setText( QString::number( analyte.mw, 'e', 4 ) ); 
   }
   
   update();
}

void US_Predict2::choose( int value )
{
   analyte_number = value;
}
