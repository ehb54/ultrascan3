//! \file us_properties.cpp

#include "us_properties.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_buffer_gui.h"
#include "us_constants.h"
#include "us_femglobal_new.h"

#include "qwt_arrow_button.h"

#include <uuid/uuid.h>

US_Properties::US_Properties( 
      const US_Buffer&                     buf, 
      const US_FemGlobal_New::ModelSystem& mod,
      int                                  inv,
      bool                                 access )
   : US_WidgetsDialog( 0, 0 ), 
     buffer      ( buf ),
     model       ( mod ),
     investigator( inv ),
     db_access   ( access )
{
   setPalette( US_GuiSettings::frameColor() );
   setWindowTitle( tr( "Set Analyte Properties" ) );
   setAttribute( Qt::WA_DeleteOnClose );

   oldRow = -1;

   normal = US_GuiSettings::editColor();

   // Very light gray
   gray = normal;
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );

   // Grid
   QGridLayout* main = new QGridLayout( this );
   main->setSpacing( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );

   int row = 0;

   QPushButton* pb_new = us_pushbutton( tr( "New Analyte" ) );
   connect( pb_new, SIGNAL( clicked() ), SLOT( newAnalyte() ) );
   main->addWidget( pb_new, row++, 0, 1, 2 );

   //QPushButton* pb_analyte = us_pushbutton( tr( "Analyte Details" ) );
   //connect( pb_analyte, SIGNAL( clicked() ), SLOT( edit_analyte() ) );
   //main->addWidget( pb_analyte, row++, 1 );

   // Components List Box
   lw_components = new US_ListWidget;
   lw_components->setToolTip(
         tr( "Double click to edit\nRight click to delete" ) );

   connect( lw_components, SIGNAL( currentRowChanged( int  ) ),
                           SLOT  ( update           ( int  ) ) );

   //connect( lw_components, SIGNAL( rightClick      ( void ) ),
   //                        SLOT  ( delete_component( void ) ) );

   main->addWidget( lw_components, row, 0, 2, 4 );
   row += 4;
   //update_lw();

   // Row
   QLabel* lb_GUID = us_label( tr( "Global Identifier:" ) );
   main->addWidget( lb_GUID, row, 0 );

   le_GUID = us_lineedit( "" );
   le_GUID->setPalette ( gray );
   le_GUID->setReadOnly( true );
   main->addWidget( le_GUID, row++, 1 );

   // Row
   QLabel* lb_vbar = us_label( tr( "vbar at 20 <span>&deg;</span>C (ml/g):" ) );
   main->addWidget( lb_vbar, row, 0 );

   le_vbar = us_lineedit( "" );
   //le_vbar->setPalette ( gray );
   //le_vbar->setReadOnly( true );
   main->addWidget( le_vbar, row++, 1 );

   // Row
   QLabel* lb_extinction =  us_label( tr( "Extinction (optical units):" ) );
   main->addWidget( lb_extinction, row, 0 );

   QHBoxLayout* extinction = new QHBoxLayout;
   extinction->setSpacing( 0 );

   le_extinction = us_lineedit( "" );
   extinction->addWidget( le_extinction );

   QwtArrowButton* down = new QwtArrowButton( 1, Qt::DownArrow );
   down->setMinimumWidth( 16 );
   connect( down, SIGNAL( clicked() ), SLOT( lambda_down() ) );
   extinction->addWidget( down );

   le_wavelength = us_lineedit( "" );
   le_wavelength->setMinimumWidth( 80 );
   le_wavelength->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred );
   le_wavelength->setPalette ( gray );
   le_wavelength->setReadOnly( true );
   extinction->addWidget( le_wavelength );

   QwtArrowButton* up = new QwtArrowButton( 1, Qt::UpArrow );
   up->setMinimumWidth( 16 );
   connect( up, SIGNAL( clicked() ), SLOT( lambda_up() ) );
   extinction->addWidget( up );
   
   main->addLayout( extinction, row++, 1 );

   // Row
   QLabel* lb_molar = us_label( tr( "Molar Concentration:" ) );
   main->addWidget( lb_molar, row, 0 );

   le_molar = us_lineedit( "" );
   le_molar->setPalette ( gray );
   le_molar->setReadOnly( true );
   main->addWidget( le_molar, row++, 1 );

   // Row
   QLabel* lb_analyteConc = us_label( tr( "Analyte Signal Concentration:" ) );
   main->addWidget( lb_analyteConc, row, 0 );

   le_analyteConc = us_lineedit( "" );
   main->addWidget( le_analyteConc, row++, 1 );

   // Row
   QPushButton* pb_sim = us_pushbutton( tr( "Simulate s and D" ) );
   connect( pb_sim, SIGNAL( clicked() ), SLOT( simulate() ) );
   main->addWidget( pb_sim, row, 0 );

   cmb_shape = us_comboBox();
   cmb_shape->addItem( tr( "Sphere"            ), US_FemGlobal_New::SPHERE  );
   cmb_shape->addItem( tr( "Prolate Ellipsoid" ), US_FemGlobal_New::PROLATE );
   cmb_shape->addItem( tr( "Oblate Ellipsoid"  ), US_FemGlobal_New::OBLATE  );
   cmb_shape->addItem( tr( "Rod"               ), US_FemGlobal_New::ROD     );
   //connect( cmb_shape, SIGNAL( currentIndexChanged( int ) ),
   //                    SLOT  ( select_shape       ( int ) ) );
   main->addWidget( cmb_shape, row++, 1 );

   QButtonGroup* checks = new QButtonGroup();
   checks->setExclusive( false );
   //connect( checks, SIGNAL( buttonClicked ( int ) ), SLOT( checkbox( int ) ) );

   // Row
   QGridLayout* mw_layout = us_checkbox( tr( "Molecular Weight" ), cb_mw, true );
   checks->addButton( cb_mw, MW );
   main->addLayout( mw_layout, row, 0 );

   le_mw = us_lineedit( "" );
   //connect( le_mw, SIGNAL( editingFinished () ), SLOT( calculate() ) );
   main->addWidget( le_mw, row++, 1 );

   //connect( cb_optics, SIGNAL( currentIndexChanged ( int ) ),
   //                    SLOT  ( change_optics       ( int ) ) );

   // Row
   QGridLayout* f_f0_layout = us_checkbox( tr( "Frictional Ratio (f/f0)" ), cb_f_f0, true );
   checks->addButton( cb_f_f0, F_F0 );
   main->addLayout( f_f0_layout, row, 0 );

   le_f_f0 = us_lineedit( "1.25" );
   connect( le_f_f0, SIGNAL( editingFinished () ), SLOT( calculate() ) );
   main->addWidget( le_f_f0, row++, 1 );
   
   // Row 
   QGridLayout* s_layout = us_checkbox( tr( "Sedimentation Coeff. (s)" ), cb_s );
   checks->addButton( cb_s, S );
   main->addLayout( s_layout, row, 0 );

   le_s = us_lineedit( "n/a" );
   le_s->setPalette ( gray );
   le_s->setReadOnly( true );
   //connect( le_s, SIGNAL( editingFinished () ), SLOT( calculate() ) );
   main->addWidget( le_s, row++, 1 );

   // Row
   QGridLayout* D_layout = us_checkbox( tr( "Diffusion Coeff. (D)" ), cb_D );
   checks->addButton( cb_D, D );
   main->addLayout( D_layout, row, 0 );

   le_D = us_lineedit( "n/a" );
   le_D->setPalette ( gray );
   le_D->setReadOnly( true );
   //connect( le_D, SIGNAL( editingFinished () ), SLOT( calculate() ) );
   main->addWidget( le_D, row++, 1 );

   // Row
   QGridLayout* f_layout = us_checkbox( tr( "Frictional Coeff. (f)" ), cb_f );
   checks->addButton( cb_f, F );
   main->addLayout( f_layout, row, 0 );

   le_f = us_lineedit( "n/a" );
   le_f->setPalette ( gray );
   le_f->setReadOnly( true );
   connect( le_f, SIGNAL( editingFinished () ), SLOT( calculate() ) );
   main->addWidget( le_f, row++, 1 );

   // Row
   QLabel* lb_sigma = us_label( tr( "Conc. Dependency of s (<span>&sigma;</span>):" ) );
   main->addWidget( lb_sigma, row, 0 );

   le_sigma = us_lineedit( "" );
   main->addWidget( le_sigma, row++, 1 );

   // Row
   QLabel* lb_delta = us_label( tr( "Conc. Dependency of D (<span>&delta;</span>):" ) );
   main->addWidget( lb_delta, row, 0 );

   le_delta = us_lineedit( "" );
   main->addWidget( le_delta, row++, 1 );

   // Row
   QPushButton* pb_load_c0 = us_pushbutton( tr( "Load C0 from File" ) );
   //connect( pb_new, SIGNAL( clicked() ), SLOT( new_component() ) );
   main->addWidget( pb_load_c0, row, 0 );

   QGridLayout* co_sed_layout = us_checkbox( tr( "Co-sedimenting Solute" ), cb_co_sed );
   main->addLayout( co_sed_layout, row++, 1 );




/*
   QPushButton* pb_density = us_pushbutton( tr( "Density" ) );
   connect( pb_density, SIGNAL( clicked() ), SLOT( getBuffer() ) );
   main->addWidget( pb_density, row, 0 );

   le_density = us_lineedit( "0.9982e+00" );
   main->addWidget( le_density, row, 1 );

   QGridLayout* mw_layout = us_checkbox( tr( "Molecular Weight" ), cb_mw, true );
   checks->addButton( cb_mw, MW );
   main->addLayout( mw_layout, row, 2 );

   le_mw = us_lineedit( "0.0000e+00" );
   connect( le_mw, SIGNAL( editingFinished () ), SLOT( calculate() ) );
   main->addWidget( le_mw, row++, 3 );

   // Row
   QLabel* lb_temperature = us_label( tr( "Temperature (<span>&deg;C</span>)" ) );
   main->addWidget( lb_temperature, row, 0 );

   le_temperature = us_lineedit( "20.0" );
   main->addWidget( le_temperature, row, 1 );

*/
   // Pushbuttons

   //QPushButton* pb_help = us_pushbutton( tr( "Help") );
   //connect( pb_help, SIGNAL( clicked() ), SLOT( help()) );
   //buttonbox->addWidget( pb_help );

   //QHBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_close = us_pushbutton( tr( "Cancel") );
   main->addWidget( pb_close, row, 0 );
   connect( pb_close, SIGNAL( clicked() ), SLOT( close() ) );

   QPushButton* pb_accept = us_pushbutton( tr( "Accept") );
   main->addWidget( pb_accept, row++, 1 );
   //connect( pb_accept, SIGNAL( clicked() ), SLOT( acceptProp() ) );

   //main->addLayout( buttons, row++, 0, 1, 3 );
}

void US_Properties::lambda_down( void )
{
   qDebug() << "down";
}

void US_Properties::lambda_up( void )
{
   qDebug() << "up";
}

void US_Properties::update_lw( void )
{
   lw_components->clear();

   for ( int i = 0; i < model.components.size(); i++ )
      lw_components->addItem( model.components[ i ].name );
}

void US_Properties::newAnalyte( void )
{
   US_FemGlobal_New::SimulationComponent sc;
   model.components << sc;

   int last = model.components.size() - 1;
   update_lw();

   lw_components->setCurrentRow( last );
}

void US_Properties::edit_analyte( void )
{
   US_Analyte* dialog = new US_Analyte( investigator, true, le_GUID->text(), 
         db_access );

   connect( dialog, SIGNAL( valueChanged  ( US_Analyte::analyteData ) ), 
                    SLOT  ( update_analyte( US_Analyte::analyteData ) ) );
   dialog->exec();
}

void US_Properties::update_analyte( US_Analyte::analyteData ad )
{
qDebug() << "in update_analyte";
   int index = lw_components->currentRow();
   if ( index < 0 ) return;

qDebug() << "B" << index;
   analyte = ad;
   US_FemGlobal_New::SimulationComponent* sc = &model.components[ index ];

   sc->mw     = ad.mw;
   sc->vbar20 = ad.vbar;
   sc->name   = ad.description;

   uuid_parse( ad.guid.toAscii(), sc->analyteGUID );
   sc->analyte_type = ad.type;

   cb_mw  ->setChecked( false );
   cb_s   ->setChecked( false );
   cb_D   ->setChecked( false );
   cb_f   ->setChecked( false );
   cb_f_f0->setChecked( false );
   
   checkbox();
   update( 0 );
   
   qDebug() << ad.vbar;
   qDebug() << sc->mw;
   qDebug() << ad.description;
   qDebug() << ad.guid;
   qDebug() << ad.type;
}

void US_Properties::update( int /* row */ )
{
   int index = lw_components->currentRow();

   if ( index < 0 ) return;

qDebug() << "c" << index;
   // Save current data
   if ( oldRow > -1 )
   {
   }

   oldRow = index;

   // Update to current data
   US_FemGlobal_New::SimulationComponent sc = model.components[ index ];

   char uuid[ 37 ];
   uuid[ 36 ] = 0;
   uuid_unparse( sc.analyteGUID, uuid );
   le_GUID->setText( QString( uuid ) );

   le_mw   ->setText( QString::number( sc.mw   , 'e', 4 ) );
   le_s    ->setText( QString::number( sc.s    , 'e', 4 ) );
   le_D    ->setText( QString::number( sc.D    , 'e', 4 ) );
   le_f_f0 ->setText( QString::number( sc.f    , 'e', 4 ) );
   le_f_f0 ->setText( QString::number( sc.f_f0 , 'e', 4 ) );
   le_sigma->setText( QString::number( sc.sigma, 'e', 4 ) );
   le_delta->setText( QString::number( sc.delta, 'e', 4 ) );

   le_wavelength->setText( QString::number( sc.wavelength, 'f', 1 ) );
   le_extinction->setText( QString::number( sc.extinction, 'e', 4 ) );

   ( index == model.coSedSolute ) ? cb_co_sed->setChecked( true  )
                                  : cb_co_sed->setChecked( false );
}

void US_Properties::simulate( void )
{
   working_data = data; 

   US_Predict1* dialog = new US_Predict1( working_data );
   connect( dialog, SIGNAL( done() ), SLOT( new_hydro() ) );
   dialog->exec();
}

void US_Properties::new_hydro( void )
{
qDebug() << "A";
   data = working_data;
qDebug() << "B";
   //le_density    ->setText( QString::number( data.density    , 'e', 4 ) );
   le_vbar       ->setText( QString::number( data.vbar       , 'e', 4 ) );
   //le_viscosity  ->setText( QString::number( data.viscosity  , 'e', 4 ) );
   //le_temperature->setText( QString::number( data.temperature, 'f', 1 ) );

qDebug() << "c";
   le_mw  ->setText( QString::number( data.mw              , 'e', 4 ) );
   le_s   ->setText( QString::number( data.sphere.sedcoeff , 'e', 4 ) );
   le_D   ->setText( QString::number( data.sphere.diffcoeff, 'e', 4 ) );
   le_f   ->setText( QString::number( data.sphere.f        , 'e', 4 ) );
   le_f_f0->setText( QString::number( data.sphere.f_f0     , 'e', 4 ) );
}

void US_Properties::acceptProp( void )
{
   // Need a sanity check here

   data.mw   = le_mw  ->text().toDouble();
   //data.s    = le_s   ->text().toDouble();
   //data.D    = le_D   ->text().toDouble();
   //data.f    = le_f   ->text().toDouble();
   //data.f_f0 = le_f_f0->text().toDouble();

   emit valueChanged( data );
   accept();
}


void US_Properties::getBuffer( void )
{
   US_BufferGui* dialog = new US_BufferGui( -1, true );
   connect( dialog, SIGNAL( valueChanged( US_Buffer ) ),
                    SLOT  ( setBuffer   ( US_Buffer ) ) );
   dialog->exec();
}

void US_Properties::setBuffer( US_Buffer b )
{
   buffer = b;
   le_density  ->setText( QString::number( buffer.density  , 'e', 4 ) );
   le_viscosity->setText( QString::number( buffer.viscosity, 'e', 4 ) );
   calculate();
}

void US_Properties::getAnalyte( void )
{
   US_Analyte* dialog = new US_Analyte( -1, true );
   connect( dialog, SIGNAL( valueChanged( US_Analyte::analyteData ) ),
                    SLOT  ( setAnalyte  ( US_Analyte::analyteData ) ) );
   dialog->exec();
}

void US_Properties::setAnalyte( US_Analyte::analyteData a )
{
   analyte = a;
   le_vbar->setText( QString::number( analyte.vbar, 'e', 4 ) );
   le_mw  ->setText( QString::number( analyte.mw  , 'e', 4 ) );
   calculate();
}

void US_Properties::checkbox( int /*box*/ )
{
   if ( countChecks() != 2 )
   {
      enable( le_mw  , false, gray );
      enable( le_s   , false, gray );
      enable( le_D   , false, gray );
      enable( le_f   , false, gray );
      enable( le_f_f0, false, gray );
      return;
   }

   ( cb_mw  ->isChecked() ) ? enable( le_mw,   false, normal ) 
                            : enable( le_mw,   true , gray   );
   
   ( cb_s   ->isChecked() ) ? enable( le_s,    false, normal ) 
                            : enable( le_s,    true , gray   );
   
   ( cb_D   ->isChecked() ) ? enable( le_D,    false, normal ) 
                            : enable( le_D,    true , gray   );
   
   ( cb_f   ->isChecked() ) ? enable( le_f,    false, normal ) 
                            : enable( le_f,    true , gray   );
   
   ( cb_f_f0->isChecked() ) ? enable( le_f_f0, false, normal ) 
                            : enable( le_f_f0, true , gray   );
}

void US_Properties::enable( QLineEdit* le, bool status, const QPalette& p )
{
   le->setReadOnly( status );
   le->setPalette ( p );
}

int US_Properties::countChecks( void )
{
   int checked = 0;
   if ( cb_mw  ->isChecked() ) checked++;
   if ( cb_s   ->isChecked() ) checked++;
   if ( cb_D   ->isChecked() ) checked++;
   if ( cb_f   ->isChecked() ) checked++;
   if ( cb_f_f0->isChecked() ) checked++;
   return checked;
}

void US_Properties::calculate( void )
{
   // First do some sanity checking
   double density = le_density->text().toDouble();
   if ( density <= 0.0 ) return;

   double viscosity = le_viscosity->text().toDouble();
   if ( viscosity <= 0.0 ) return;

   double vbar = le_vbar->text().toDouble();
   if ( vbar <= 0.0 ) return;

   // Exatly two checkboxes must be set
   if ( countChecks() != 2 ) return;

   //double                t = le_temperature->text().toDouble();
   double t = 20.0;
   US_Math::SolutionData d;

   d.vbar      = vbar;
   d.density   = density;
   d.viscosity = viscosity;

   US_Math::data_correction( t, d );

   t += K0;  // Calculatons below need Kelvin

   double mw;
   double s;
   double D;
   double f;
   double f_f0;
   double vol_per_molecule;
   double radius_sphere;

   // Molecular weight
   if ( cb_mw->isChecked() )
   {
      mw = le_mw->text().toDouble();
      
      if ( mw <= 0.0 ) 
      {
         setInvalid();
         return;
      }

      vol_per_molecule = vbar * mw / AVOGADRO;
      radius_sphere    = pow( vol_per_molecule * 0.75 / M_PI, 1.0 / 3.0 );
      double f0        = radius_sphere * 6.0 * M_PI * viscosity * 0.01;

      // mw and s
      if ( cb_s->isChecked() )
      {
         s = le_s->text().toDouble();
         
         if ( s <= 0.0 ) 
         {
            setInvalid();
            return;
         }

         D    = s * R * t / ( d.buoyancyb * mw );
         f    = mw * d.buoyancyb / ( s * AVOGADRO );
         f_f0 = f / f0;
      }

      // mw and D
      else if ( cb_D->isChecked() )
      {
         D = le_D->text().toDouble();
         
         if ( D <= 0.0 ) 
         {
            setInvalid();
            return;
         }

         s    = D * d.buoyancyb * mw / ( R * t );
         f    = mw * d.buoyancyb / ( s * AVOGADRO );
         f_f0 = f / f0;
      }

      // mw and f
      else if ( cb_f->isChecked() )
      {
         f = le_f->text().toDouble();
         
         if ( f <= 0.0 ) 
         {
            setInvalid();
            return;
         }

         f_f0 = f / f0;
         s    = mw * ( 1.0 - vbar * density ) / ( AVOGADRO * f );
         D    = s * R * t / ( d.buoyancyb * mw );
      }

      else // mw and f_f0
      {
         f_f0 = le_f_f0->text().toDouble();
         
         if ( f_f0 < 1.0 ) 
         {
            setInvalid();
            return;
         }

         f = f_f0 * f0;
         s = mw * ( 1.0 - vbar * density ) / ( AVOGADRO * f );
         D = s * R * t / ( d.buoyancyb * mw );
      }
   }

   else if ( cb_s->isChecked() )  // mw is NOT checked
   {
      s = le_s->text().toDouble();
      
      if ( s <= 0.0 ) 
      {
         setInvalid();
         return;
      }

      if ( cb_D->isChecked() )   // s and D
      {
         D = le_D->text().toDouble();

         if ( D <= 0.0 )
         {
            setInvalid();
            return;
         }

         mw               = s * R * t / ( D * ( 1.0 -  vbar * density ) );
         vol_per_molecule = vbar * mw / AVOGADRO;
         radius_sphere    = pow( vol_per_molecule * 0.75 / M_PI, 1.0 / 3.0 );
         double f0        = radius_sphere * 6.0 * M_PI * viscosity * 0.01;
         f                = mw * d.buoyancyb / ( s * AVOGADRO );
         f_f0             = f / f0;
      }

      else if (  cb_f->isChecked() ) // s and f
      {
         f = le_f->text().toDouble();
         
         if ( f <= 0.0 ) 
         {
            setInvalid();
            return;
         }

         D                = R * t / ( AVOGADRO * f );
         mw               = s * R * t / ( D * ( 1.0 - vbar * density ) );
         vol_per_molecule = vbar * mw / AVOGADRO;
         radius_sphere    = pow( vol_per_molecule * 0.75 / M_PI, 1.0 / 3.0 );
         double f0        = radius_sphere * 6.0 * M_PI * viscosity * 0.01;
         f_f0             = f / f0;
      }

      else  // s and f_f0
      {
         f_f0 = le_f_f0->text().toDouble();
         
         if ( f_f0 <= 1.0 ) 
         {
            setInvalid();
            return;
         }

         double n  = 2.0 * s * f_f0 * vbar * viscosity; // numerator
         double d  = 1.0 - vbar * density;              // denominator
         double f0 = 9.0 * viscosity * M_PI * sqrt( n / d );
         f         = f_f0 * f0;
         D         = R * t / ( AVOGADRO * f ); 
         mw        = s * R * t / ( D * ( 1.0 - vbar * density ) );
      }
   }

   else if ( cb_D->isChecked() ) // mw and s are NOT checked
   {
      D = le_D->text().toDouble();
      
      if ( D <= 0.0 ) 
      {
         setInvalid();
         return;
      }

      if ( cb_f->isChecked() ) // D and f  -  The is an invalid combination
      {
         setInvalid();
         return;
      }

      else // D and f/f0
      {
         f_f0 = le_f_f0->text().toDouble();
         
         if ( f_f0 <= 1.0 ) 
         {
            setInvalid();
            return;
         }

         f             = R * t / ( AVOGADRO * D );
         double f0     = f / f_f0;
         radius_sphere = f0 / ( 6.0 * M_PI * viscosity );
         double volume = ( 4.0 / 3.0 ) * M_PI * pow( radius_sphere, 3.0 );
         mw            = volume * AVOGADRO / vbar;
         s             = mw * (1.0 - vbar * density ) / ( AVOGADRO * f );
      }
   }

   else  // only f and f_f0 are checked
   {
      f    = le_f   ->text().toDouble();
      f_f0 = le_f_f0->text().toDouble();
      
      if ( f <= 0.0  ||  f_f0 < 1.0 ) 
      {
         setInvalid();
         return;
      }

      double f0     = f / f_f0;
      D             = R * t / ( AVOGADRO * f );
      radius_sphere = f0 / ( 6.0 * M_PI * viscosity );
      double volume = ( 4.0 / 3.0 ) * M_PI * pow( radius_sphere, 3.0 );
      mw            = volume * AVOGADRO / vbar;
      s             = mw * (1.0 - vbar * density ) / ( AVOGADRO * f );
   }

   le_mw  ->setText( QString::number( mw  , 'e', 4 ) );
   le_s   ->setText( QString::number( s   , 'e', 4 ) );
   le_D   ->setText( QString::number( D   , 'e', 4 ) );
   le_f   ->setText( QString::number( f   , 'e', 4 ) );
   le_f_f0->setText( QString::number( f_f0, 'e', 4 ) );
}

void US_Properties::setInvalid( void )
{
   if ( ! cb_mw  ->isChecked() ) le_mw  ->setText( tr( "n/a" ) );
   if ( ! cb_s   ->isChecked() ) le_s   ->setText( tr( "n/a" ) );
   if ( ! cb_D   ->isChecked() ) le_D   ->setText( tr( "n/a" ) );
   if ( ! cb_f   ->isChecked() ) le_f   ->setText( tr( "n/a" ) );
   if ( ! cb_f_f0->isChecked() ) le_f_f0->setText( tr( "n/a" ) );
   checkbox();
}
