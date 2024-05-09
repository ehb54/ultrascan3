//! \file us_properties.cpp

#include "us_properties.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_images.h"
#include "us_constants.h"
#include "us_model.h"

#include "qwt_arrow_button.h"

US_Properties::US_Properties( US_Model& mod, int access )
   : US_WidgetsDialog( 0, 0 ), 
     model       ( mod ),
     db_access   ( access )
{
   setPalette    ( US_GuiSettings::frameColor() );
   setWindowTitle( tr( "Set Analyte Properties" ) );
   setAttribute  ( Qt::WA_DeleteOnClose );

   oldRow   = -2;
   inUpdate = false;
   chgStoi  = false;

   normal = US_GuiSettings::editColor();

   // Initialize the check icon
   check = US_Images::getIcon( US_Images::CHECK );

   // Grid
   QGridLayout* main      = new QGridLayout( this );
   main->setSpacing( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );

   QPushButton* pb_new    = us_pushbutton( tr( "New Analyte" ) );
   QPushButton* pb_delete = us_pushbutton( tr( "Remove Current Analyte" ) );
   QPushButton* pb_edit   = us_pushbutton( tr( "Edit Current Analyte" ) );
   QLabel*      lb_desc   = us_label( tr( "Analyte Description:" ) );
   le_description         = us_lineedit( "" );

   // Components List Box
   lw_components          = new US_ListWidget;

   // Row
   QLabel*      lb_guid   = us_label( tr( "Global Identifier:" ) );
   le_guid                = us_lineedit( "" );
   us_setReadOnly( le_guid, true );

   if ( US_Settings::us_debug() == 0 )
   {
      lb_guid->setVisible( false );
      le_guid->setVisible( false );
   }

   // Row
   QLabel* lb_vbar        = us_label( tr( "Vbar at 20 " ) + DEGC + " (ml/g):" );
   le_vbar                = us_lineedit( "" );

   // Row
   QLabel* lb_extinction  = us_label( tr( "Extinction (OD/(mol*cm)):" ) );
   QLabel* lb_wavelength  = us_label( tr( "Wavelength (nm):" ) );

   le_extinction          = us_lineedit( "" );

   le_wavelength = us_lineedit( QString::number( model.wavelength, 'f', 1 ) );
   le_wavelength->setMinimumWidth( 80 );
   le_wavelength->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred );
   us_setReadOnly( le_wavelength, true );

   // Row
   QGridLayout* lo_sigConc = us_checkbox( 
      tr( "Signal Concentration:" ), ck_sigConc, true  );
   le_sigConc              = us_lineedit( "" );
   QGridLayout* lo_isProd  = us_checkbox( 
      tr( "Analyte is a Product" ),  ck_isProd,  false );

   // Row
   QGridLayout* lo_molConc = us_checkbox( 
      tr( "Molar Concentration:" ),  ck_molConc, false );
   le_molConc              = us_lineedit( "", 0, true );
   ck_sigConc->setChecked( true  );
   ck_molConc->setChecked( false );

   // Row
   QPushButton* pb_sim     = us_pushbutton( tr( "Simulate s and D" ) );

   cb_shape                = us_comboBox();
   cb_shape->addItem( tr( "Sphere"            ), US_Model::SPHERE  );
   cb_shape->addItem( tr( "Prolate Ellipsoid" ), US_Model::PROLATE );
   cb_shape->addItem( tr( "Oblate Ellipsoid"  ), US_Model::OBLATE  );
   cb_shape->addItem( tr( "Rod"               ), US_Model::ROD     );

   // Row
   QGridLayout* lo_mw      = us_checkbox( 
      tr( "Molecular Wt. (mw,S20W)" ), ck_mw, true );

   QLabel* lb_oligomer     = us_label( tr( "Oligomer:" ) );

   le_mw                   = us_lineedit( "" );
   us_setReadOnly( le_mw, true );

   ct_oligomer              = us_counter( 2, 1.0, 50.0, 1.0 );
   ct_oligomer->setSingleStep( 1.0 );
   QFontMetrics fmet( font() );
   int fwid    = fmet.maxWidth();
   int rhgt    = ct_oligomer->height();
   int cminw   = fwid;
   int csizw   = cminw + fwid * 2;
   ct_oligomer->setMinimumWidth( cminw );
   ct_oligomer->resize( csizw, rhgt );

   // Row
   QGridLayout* lo_f_f0     = 
      us_checkbox( tr( "Frictional Ratio (f/f0,20W)" ), ck_f_f0, true );

   le_f_f0                  = us_lineedit( "n/a" );
   us_setReadOnly( le_f_f0, true );
   
   // Row 
   QGridLayout* lo_s        = us_checkbox(
      tr( "Sedimentation Coeff. (s,20W)" ), ck_s );
   le_s                     = us_lineedit( "1e-13" );

   // Row
   QGridLayout* lo_D        = us_checkbox(
      tr( "Diffusion Coeff. (D,20W)" ), ck_D );

   le_D                     = us_lineedit( "2e-7" );

   // Row
   QGridLayout* lo_f        = us_checkbox(
      tr( "Frictional Coeff. (f,20W)" ), ck_f );
   le_f                     = us_lineedit( "n/a" );
   us_setReadOnly( le_f, true );

   // Row
   QLabel* lb_sigma         = us_label(
      tr( "Conc. Dependency of s (<span>&sigma;</span>):" ) );

   le_sigma                 = us_lineedit( "" );

   // Row
   QLabel* lb_delta         = us_label(
      tr( "Conc. Dependency of D (<span>&delta;</span>):" ) );
   le_delta                 = us_lineedit( "" );

   // Row
   pb_load_c0               = us_pushbutton( tr( "Load C0 from File" ) );

   QGridLayout* lo_co_sed   = us_checkbox(
      tr( "Co-sedimenting Solute" ), ck_co_sed );
   
   // Pushbuttons
   QPushButton* pb_help     = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_close    = us_pushbutton( tr( "Cancel" ) );
   //QPushButton* pb_accept   = us_pushbutton( tr( "Accept" ) );
   pb_accept   = us_pushbutton( tr( "Accept" ) );
   
   // Main layout
   int row = 0;
   main->addWidget( pb_new,         row,   0, 1, 2 );
   main->addWidget( pb_delete,      row,   2, 1, 2 );
   main->addWidget( pb_edit,        row++, 4, 1, 2 );
   main->addWidget( lb_desc,        row,   0, 1, 2 );
   main->addWidget( le_description, row++, 2, 1, 4 );
   main->addWidget( lw_components,  row,   0, 4, 6 ); row += 4;
   main->addWidget( lb_guid,        row,   0, 1, 2 );
   main->addWidget( le_guid,        row++, 2, 1, 4 );
   main->addWidget( lb_vbar,        row,   0, 1, 2 );
   main->addWidget( le_vbar,        row++, 2, 1, 1 );
   main->addWidget( lb_extinction,  row,   0, 1, 2 );
   main->addWidget( le_extinction,  row,   2, 1, 1 );
   main->addWidget( lb_wavelength,  row,   3, 1, 2 );
   main->addWidget( le_wavelength,  row++, 5, 1, 1 );
   main->addLayout( lo_molConc,     row,   0, 1, 2 );
   main->addWidget( le_molConc,     row++, 2, 1, 1 );
   main->addLayout( lo_sigConc,     row,   0, 1, 2 );
   main->addWidget( le_sigConc,     row,   2, 1, 1 );
   main->addLayout( lo_isProd,      row++, 3, 1, 3 );
   main->addWidget( pb_sim,         row,   0, 1, 2 );
   main->addWidget( cb_shape,       row++, 2, 1, 4 );
   main->addLayout( lo_mw,          row,   0, 1, 2 );
   main->addWidget( le_mw,          row,   2, 1, 1 );
   main->addWidget( lb_oligomer,    row,   3, 1, 2 );
   main->addWidget( ct_oligomer,    row++, 5, 1, 1 );
   main->addLayout( lo_f_f0,        row,   0, 1, 2 );
   main->addWidget( le_f_f0,        row++, 2, 1, 1 );
   main->addLayout( lo_s,           row,   0, 1, 2 );
   main->addWidget( le_s,           row++, 2, 1, 1 );
   main->addLayout( lo_D,           row,   0, 1, 2 );
   main->addWidget( le_D,           row++, 2, 1, 1 );
   main->addLayout( lo_f,           row,   0, 1, 2 );
   main->addWidget( le_f,           row++, 2, 1, 1 );
   main->addWidget( lb_sigma,       row,   0, 1, 2 );
   main->addWidget( le_sigma,       row,   2, 1, 1 );
   main->addWidget( lb_delta,       row,   3, 1, 2 );
   main->addWidget( le_delta,       row++, 5, 1, 1 );
   main->addWidget( pb_load_c0,     row,   0, 1, 3 );
   main->addLayout( lo_co_sed,      row++, 3, 1, 3 );
   main->addWidget( pb_help,        row,   0, 1, 2 );
   main->addWidget( pb_close,       row,   2, 1, 2 );
   main->addWidget( pb_accept,      row++, 4, 1, 2 );

   // Signal-Slot connections
   connect( pb_new,         SIGNAL( clicked()       ), 
                            SLOT(   newAnalyte()    ) );
   connect( pb_delete,      SIGNAL( clicked()       ), 
                            SLOT(   del_component() ) );
   connect( pb_edit,        SIGNAL( clicked()       ), 
                            SLOT(   edit_analyte()  ) );
   connect( le_description, SIGNAL( editingFinished() ), 
                            SLOT  ( edit_component () ) );
   connect( lw_components,  SIGNAL( currentRowChanged( int  ) ),
                            SLOT  ( update           ( int  ) ) );
   connect( le_vbar,        SIGNAL( editingFinished() ), 
                            SLOT  ( edit_vbar      () ) );
   connect( le_extinction,  SIGNAL( editingFinished() ), 
                            SLOT(   set_molar()       ) );
   connect( ck_molConc,     SIGNAL( toggled(      bool ) ), 
                            SLOT(   check_molar(  bool ) ) );
   connect( ck_sigConc,     SIGNAL( toggled(      bool ) ), 
                            SLOT(   check_signal( bool ) ) );
   connect( le_sigConc,     SIGNAL( editingFinished() ), 
                            SLOT(   set_molar()       ) );
   connect( pb_sim,         SIGNAL( clicked()  ), 
                            SLOT(   simulate() ) );
   connect( cb_shape,       SIGNAL( currentIndexChanged( int ) ),
                            SLOT  ( select_shape       ( int ) ) );
   connect( ck_mw,          SIGNAL( toggled(   bool ) ), 
                            SLOT(   calculate( bool ) ) );
   connect( le_mw,          SIGNAL( editingFinished() ), 
                            SLOT(   calculate()       ) );
   connect( ct_oligomer,    SIGNAL( valueChanged ( double ) ), 
                            SLOT  ( set_oligomer ( double ) ) );
   connect( ck_f_f0,        SIGNAL( toggled(   bool )  ), 
                            SLOT(   calculate( bool )  ) );
   connect( le_f_f0,        SIGNAL( editingFinished () ), 
                            SLOT(   calculate()        ) );
   connect( ck_s,           SIGNAL( toggled(   bool )  ), 
                            SLOT(   calculate( bool)   ) );
   connect( le_s,           SIGNAL( editingFinished()  ), 
                            SLOT(   calculate()        ) );
   connect( ck_D,           SIGNAL( toggled(   bool )  ), 
                            SLOT(   calculate( bool )  ) );
   connect( le_D,           SIGNAL( editingFinished()  ), 
                            SLOT(   calculate()        ) );
   connect( ck_f,           SIGNAL( toggled(   bool )  ), 
                            SLOT(   calculate( bool )  ) );
   connect( le_f,           SIGNAL( editingFinished () ), 
                            SLOT(   calculate()        ) );
   connect( pb_load_c0,     SIGNAL( clicked() ), 
                            SLOT(   load_c0() ) );
   connect( ck_co_sed,      SIGNAL( stateChanged( int ) ), 
                            SLOT(   co_sed( int )       ) );
   connect( pb_help,        SIGNAL( clicked()    ), 
                            SLOT(   help()       ) );
   connect( pb_close,       SIGNAL( clicked()    ), 
                            SLOT(   close()      ) );
   connect( pb_accept,      SIGNAL( clicked()    ), 
                            SLOT(   acceptProp() ) );

   clear_entries();
   update_lw();
   checkbox();
}

void US_Properties::disable_gui( void )
{
  pb_accept-> setEnabled(false);

}

void US_Properties::clear_entries( void )
{
   le_guid      ->clear();
   le_vbar      ->clear();
   le_extinction->clear();
   le_molConc   ->clear();
   le_sigConc   ->clear();
   le_mw        ->clear();
   le_s         ->clear();
   le_D         ->clear();
   le_f         ->clear();
   le_f_f0      ->clear();
   le_sigma     ->clear();
   le_delta     ->clear();

   cb_shape   ->setCurrentIndex( 0 );
   ct_oligomer->setValue( 1.0 );
   pb_load_c0 ->setIcon( QIcon() );
   ck_co_sed  ->setChecked( false );

   if ( oldRow == (-2) )
   {
      le_vbar      ->setEnabled( false );
      le_extinction->setEnabled( false );
      le_sigConc   ->setEnabled( false );
      le_mw        ->setEnabled( false );
      le_f_f0      ->setEnabled( false );
      le_s         ->setEnabled( false );
      le_D         ->setEnabled( false );
      le_f         ->setEnabled( false );
      le_sigma     ->setEnabled( false );
      le_delta     ->setEnabled( false );
      ct_oligomer  ->setEnabled( false );
      ck_co_sed    ->setEnabled( false );
      ck_mw        ->setEnabled( false );
      ck_f_f0      ->setEnabled( false );
      ck_s         ->setEnabled( false );
      ck_D         ->setEnabled( false );
      ck_f         ->setEnabled( false );
   }
}

void US_Properties::newAnalyte( void )
{
   update( 0 );

   save_changes( lw_components->currentRow() );
   oldRow = -1;

   US_Model::SimulationComponent sc;
   model.components << sc;
   int last = model.components.size() - 1;
   lw_components->setCurrentRow( last );

   US_AnalyteGui* dialog =
      new US_AnalyteGui( true, QString(), db_access );

   connect( dialog, SIGNAL( valueChanged  ( US_Analyte ) ),
                    SLOT  ( update_analyte( US_Analyte ) ) );

   connect( dialog, SIGNAL( use_db        ( bool ) ), 
                    SLOT  ( source_changed( bool ) ) );

   // If accepted, work is done by update_analyte
   if ( dialog->exec() == QDialog::Rejected )
   {
      update_lw();

      lw_components->setCurrentRow( last );  // Runs update() via signal
      calculate();
   }
}

void US_Properties::update_analyte( US_Analyte new_analyte )
{
   analyte  = new_analyte;
   int last = lw_components->currentRow();

   if ( last < 0 )
      last     = model.components.count() - 1;

   update( 0 );

   if ( model.wavelength == 0.0 )
   {  // If model has no wavelength, set it from the analyte
      QList< double > keys = analyte.extinction.keys();
      if ( keys.length() > 0 )
         model.wavelength = keys[ 0 ];
   }

   US_Model::SimulationComponent* sc = &model.components[ last ];

   sc->name        = analyte.description;
   sc->analyteGUID = analyte.analyteGUID;
   sc->mw          = analyte.mw;
   sc->vbar20      = analyte.vbar20;
   sc->extinction  = analyte.extinction[ model.wavelength ];
qDebug() << "Prop: updAna: extinc" << sc->extinction;

   le_extinction->setText( QString::number( sc->extinction ) );
   set_molar();

   le_guid      ->setText( sc->analyteGUID );
   update_lw();

   lw_components->setCurrentRow( last );  // Runs update() via signal
   calculate();
}

void US_Properties::del_component( void )
{
   int row = lw_components->currentRow();
   if ( row < 0 ) return;

   QListWidgetItem* item = lw_components->item( row );
   int response = QMessageBox::question( this,
         tr( "Delete Analyte?" ),
         tr( "Delete the following analyte?\n" ) + item->text(),
         QMessageBox::Yes, QMessageBox::No );

   if ( response == QMessageBox::No ) return;

   if ( model.coSedSolute == row ) model.coSedSolute = -1;

   model.components.remove( row );
   lw_components->setCurrentRow( -1 );
   delete lw_components->takeItem( row );

   oldRow = -1;
   clear_entries();
   le_description->clear();
}

void US_Properties::update_lw( void )
{
   lw_components->clear();

   for ( int i = 0; i < model.components.size(); i++ )
      lw_components->addItem( model.components[ i ].name );
}

void US_Properties::set_oligomer( double oligomer )
{
   int row = lw_components->currentRow();
   if ( row < 0 ) return;

   US_Model::SimulationComponent* sc = &model.components[ row ];

   inUpdate = true;

   sc->mw /= sc->oligomer;  // Get monomer mw
   sc->mw *= oligomer;             // Now adjust for new oligomer count
   le_mw->setText( QString::number( sc->mw, 'e', 8 ) );

   us_setReadOnly( le_mw, !ck_mw->isChecked() );

   sc->extinction /= sc->oligomer;
   sc->extinction *= oligomer;
   le_extinction->setText( QString::number( sc->extinction, 'e', 8 ) );

   sc->oligomer    = (int) oligomer;

   //hydro_data.mw          = sc->mw;
   hydro_data.mw          = le_mw->text().toDouble();
   hydro_data.density     = DENS_20W;
   hydro_data.viscosity   = VISC_20W;
   hydro_data.vbar        = sc->vbar20;
   hydro_data.axial_ratio = sc->axial_ratio;

   hydro_data.calculate( NORMAL_TEMP );

   set_molar();
   //inUpdate = false;
   //select_shape( sc->shape );

   if ( oligomer > 1.0 )
   {  // if oligomer count greater than 1, must check MW and frictional ratio
      if ( ! ( ck_mw->isChecked() && ck_f_f0->isChecked() ) )
      {  // mw,f_f0 not both checked, so make it so
         ck_mw  ->setChecked( true  );
         ck_f_f0->setChecked( true  );
         ck_s   ->setChecked( false );
         ck_D   ->setChecked( false );
         ck_f   ->setChecked( false );
         checkbox();
      }

      us_setReadOnly( le_mw, true); // MW entry always disabled w oligomer > 1
   }

   bool mwuck = ! ck_mw->isChecked();
   bool scck  = ck_s ->isChecked();

   if ( mwuck )
   {  // if MW unchecked, fake it for re-calculate coeffs phase
     ck_mw->setChecked( true  );
     ck_s ->setChecked( false );
   }

   inUpdate  = false;
   chgStoi   = true;

   // re-calculate coefficients based on oligomer,mw changes
   calculate();

   chgStoi   = false;
   if ( mwuck )
   {  // if MW was unchecked, restore check state to pre-calc state
      inUpdate  = true;
      ck_mw->setChecked( false );
      ck_s ->setChecked( scck );
      checkbox();
      inUpdate  = false;
   }
}

void US_Properties::edit_component( void )
{
   int row = lw_components->currentRow();
   if ( row < 0 ) return;

   QString desc = le_description->text().trimmed();
   if ( desc.isEmpty() ) return;

   if ( desc == lw_components->item( row )->text() ) return;

   if ( keep_standard() )  // Do we want to change from the standard values?
   {
      // Restore the description
      le_description->setText( lw_components->item( row )->text() );
      return;
   }
      
   lw_components->disconnect( SIGNAL( currentRowChanged( int ) ) );
   delete lw_components->currentItem();
   
   lw_components->insertItem( row, new QListWidgetItem( desc ) );
   lw_components->setCurrentRow( row );

   US_Model::SimulationComponent* sc = &model.components[ row ];
   sc->name = desc;

   connect( lw_components, SIGNAL( currentRowChanged( int  ) ),
                           SLOT  ( update           ( int  ) ) );
   clear_guid();
}

void US_Properties::edit_vbar( void )
{
   int row = lw_components->currentRow();
   if ( row < 0 ) return;

   US_Model::SimulationComponent* sc = &model.components[ row ];

   if ( keep_standard() )  // Change from standard values?
   {
      le_vbar->setText( QString::number( sc->vbar20, 'e', 8 ) );
      return;
   }

   sc->vbar20 = le_vbar->text().toDouble();

   clear_guid();
   calculate();
}

void US_Properties::load_c0( void )
{
   int row = lw_components->currentRow();

   if ( row < 0 ) return;

   // See if the initialization vector is already loaded.
   if ( ! pb_load_c0->icon().isNull() )
   {
      int response = QMessageBox::question( this,
         tr( "Remove C0 Data?" ),
         tr( "The C0 information is loaded.\n"
             "Remove it?" ),
         QMessageBox::Yes, QMessageBox::No );

      if ( response == QMessageBox::Yes )
      {
         US_Model::SimulationComponent* sc = &model.components[ row ];

         sc->c0.radius       .clear();
         sc->c0.concentration.clear();
         pb_load_c0->setIcon( QIcon() );
      }

      return;
   }
   
   QMessageBox::information( this,
      tr( "UltraScan Information" ),
      tr( "Please note:\n\n"
          "The initial concentration file should have\n"
          "the following format:\n\n"
          "radius_value1 concentration_value1\n"
          "radius_value2 concentration_value2\n"
          "radius_value3 concentration_value3\n"
          "etc...\n\n"
          "radius values smaller than the meniscus or\n"
          "larger than the bottom of the cell will be\n"
          "excluded from the concentration vector." ) );

   QString fn = QFileDialog::getOpenFileName(
         this, tr( "Load initial concentration" ), US_Settings::resultDir(), "*" );

   if ( ! fn.isEmpty() )
   {
      QFile f( fn );;

      if ( f.open( QIODevice::ReadOnly | QIODevice::Text ) )
      {
         QTextStream ts( &f );

         int row = lw_components->currentRow();

         US_Model::SimulationComponent* sc = &model.components[ row ];

         sc->c0.radius       .clear();
         sc->c0.concentration.clear();

         // Sets concentration for this component to -1 to signal that we are
         // using a concentration vector
         double val1;
         double val2;

         while ( ! ts.atEnd() )
         {
            ts >> val1;
            ts >> val2;

            if ( val1 > 0.0 ) // Ignore radius pairs that aren't positive
            {
               sc->c0.radius        .push_back( val1 );
               sc->c0.concentration .push_back( val2 );
            }
         }

         f.close();
         pb_load_c0->setIcon( check );
      }
      else
      {
         QMessageBox::warning( this,
               tr( "UltraScan Warning" ),
               tr( "UltraScan could not open the file specified\n" ) + fn );
      }
   }
}

void US_Properties::select_shape( int shape )
{
   if ( inUpdate ) return;

   int row = lw_components->currentRow();
   if ( row < 0 ) return;

   US_Model::SimulationComponent* sc = &model.components[ row ];

   switch ( shape )
   {
      case US_Model::PROLATE:
         le_f_f0->setText(  QString::number( hydro_data.prolate.f_f0, 'e', 8 ));
         le_s   ->setText(  QString::number( hydro_data.prolate.s,    'e', 8 ));
         le_D   ->setText(  QString::number( hydro_data.prolate.D,    'e', 8 ));
         le_f   ->setText(  QString::number( hydro_data.prolate.f,    'e', 8 ));
         break;

      case US_Model::OBLATE:
         le_f_f0->setText(  QString::number( hydro_data.oblate.f_f0, 'e', 8 ) );
         le_s   ->setText(  QString::number( hydro_data.oblate.s,    'e', 8 ) );
         le_D   ->setText(  QString::number( hydro_data.oblate.D,    'e', 8 ) );
         le_f   ->setText(  QString::number( hydro_data.oblate.f,    'e', 8 ) );
         break;

      case US_Model::ROD:
         le_f_f0->setText(  QString::number( hydro_data.rod.f_f0, 'e', 8 ) );
         le_s   ->setText(  QString::number( hydro_data.rod.s,    'e', 8 ) );
         le_D   ->setText(  QString::number( hydro_data.rod.D,    'e', 8 ) );
         le_f   ->setText(  QString::number( hydro_data.rod.f,    'e', 8 ) );
         break;
      
      default:  //SPHERE
         le_f_f0->setText(  QString::number( hydro_data.sphere.f_f0, 'e', 8 ) );
         le_s   ->setText(  QString::number( hydro_data.sphere.s,    'e', 8 ) );
         le_D   ->setText(  QString::number( hydro_data.sphere.D,    'e', 8 ) );
         le_f   ->setText(  QString::number( hydro_data.sphere.f,    'e', 8 ) );
         break;
   }

   sc->f_f0 = le_f_f0->text().toDouble();
   sc->f    = le_f   ->text().toDouble();
   sc->s    = le_s   ->text().toDouble();
   sc->D    = le_D   ->text().toDouble();
}

void US_Properties::set_molar( void )
{
   int row = lw_components->currentRow();
   if ( row < 0 ) return;

   US_Model::SimulationComponent* sc = &model.components[ row ];

   double extinction = le_extinction ->text().toDouble();
   double signalConc = le_sigConc    ->text().toDouble();

   if ( extinction > 0.0 )
      sc->molar_concentration = signalConc / extinction;
   else
      sc->molar_concentration = 0.0;

   le_molConc->setText( QString::number( sc->molar_concentration, 'e', 8 ) );
}

void US_Properties::clear_guid( void )
{
   int row = lw_components->currentRow();
   if ( row < 0 ) return;

   US_Model::SimulationComponent* sc = &model.components[ row ];

   sc->analyteGUID.clear();
   le_guid->clear();
}

void US_Properties::save_changes( int row )
{
   if ( row < 0 ) return;

   US_Model::SimulationComponent* sc = &model.components[ row ];
      
   // Description
   sc->name = lw_components->item( row )->text(); 
   
   // guid
   if ( le_guid->text().isEmpty() ) 
      sc->analyteGUID.clear();
   else
      sc->analyteGUID = le_guid->text();

   // vbar
   sc->vbar20               = le_vbar->text().toDouble();

   // Extinction
   sc->extinction           = le_extinction->text().toDouble();

   // Molar concentration
   sc->molar_concentration  = le_molConc->text().toDouble();
   
   //  Signal concentration
   sc->signal_concentration = le_sigConc->text().toDouble();

   // Shape
   switch ( cb_shape->currentIndex() )
   {
      case US_Model::SPHERE : 
         sc->shape = US_Model::SPHERE; break;
      
      case US_Model::PROLATE: 
         sc->shape = US_Model::PROLATE; break;
      
      case US_Model::OBLATE : 
         sc->shape = US_Model::OBLATE; break;
      
      default:                        
         sc->shape = US_Model::ROD; break;
   }
   
   int shape = cb_shape->itemData( cb_shape->currentIndex() ).toInt();
   sc->shape = ( US_Model::ShapeType ) shape;
   
   // Characteristics
   sc->mw    = le_mw   ->text().toDouble();
   sc->s     = le_s    ->text().toDouble();
   sc->D     = le_D    ->text().toDouble();
   sc->f     = le_f    ->text().toDouble();
   sc->f_f0  = le_f_f0 ->text().toDouble();
   sc->sigma = le_sigma->text().toDouble();
   sc->delta = le_delta->text().toDouble();

   // c0 values are already set
   // co-sed is already set
}

void US_Properties::update( int /* row */ )
{
   if ( oldRow == (-2) )
   {
      le_vbar       ->setEnabled( true );
      le_extinction ->setEnabled( true );
      le_sigConc    ->setEnabled( true );
      le_mw         ->setEnabled( true );
      le_f_f0       ->setEnabled( true );
      le_s          ->setEnabled( true );
      le_D          ->setEnabled( true );
      le_f          ->setEnabled( true );
      le_sigma      ->setEnabled( true );
      le_delta      ->setEnabled( true );
      ct_oligomer   ->setEnabled( true );
      ck_co_sed     ->setEnabled( true );
      ck_mw         ->setEnabled( true );
      ck_f_f0       ->setEnabled( true );
      oldRow = -1;
   }

   int row = lw_components->currentRow();
   if ( row < 0 ) return;

   // Save current data 
   save_changes( oldRow );
   oldRow = row;

   // Update to current data
   US_Model::SimulationComponent* sc = &model.components[ row ];

   le_description->setText( sc->name );

   // Set guid
   if ( sc->analyteGUID.isEmpty() )
      le_guid->clear(); 
   else
   {
      le_guid->setText( sc->analyteGUID );
   }

   inUpdate = true;

   // Set vbar
   le_vbar->setText( QString::number( sc->vbar20, 'e', 8 ) );

   // Set extinction and concentration
   le_extinction ->setText( QString::number( sc->extinction, 'e', 8 ));

   le_molConc    ->setText( QString::number( sc->molar_concentration, 'e', 8 ));
   le_sigConc    ->setText( QString::number( sc->signal_concentration,'e', 8 ));
   
   // Update hydro data
   hydro_data.mw          = sc->mw;
   hydro_data.density     = DENS_20W;
   hydro_data.viscosity   = VISC_20W;
   hydro_data.vbar        = sc->vbar20;
   hydro_data.axial_ratio = sc->axial_ratio;

   hydro_data.calculate( NORMAL_TEMP );

   // Set shape
   switch ( sc->shape )
   {
      case US_Model::SPHERE : cb_shape->setCurrentIndex( 0 ); break;
      case US_Model::PROLATE: cb_shape->setCurrentIndex( 1 ); break;
      case US_Model::OBLATE : cb_shape->setCurrentIndex( 2 ); break;
      default:                cb_shape->setCurrentIndex( 3 ); break;
   }

   // Set characteristics
   le_mw   ->setText( QString::number( sc->mw   , 'e', 8 ) );
   le_f_f0 ->setText( QString::number( sc->f_f0 , 'e', 8 ) );
   le_s    ->setText( QString::number( sc->s    , 'e', 8 ) );
   le_D    ->setText( QString::number( sc->D    , 'e', 8 ) );
   le_f    ->setText( QString::number( sc->f    , 'e', 8 ) );
   le_sigma->setText( QString::number( sc->sigma, 'e', 8 ) );
   le_delta->setText( QString::number( sc->delta, 'e', 8 ) );

   // Set load_co indication
   ( sc->c0.radius.size() > 0 ) ? pb_load_c0->setIcon( check )
                                : pb_load_c0->setIcon( QIcon() );
   // Set co-sedimenting solute
   ( row == model.coSedSolute ) ? ck_co_sed->setChecked( true  )
                                : ck_co_sed->setChecked( false );

   ct_oligomer->setValue( sc->oligomer );
   inUpdate = false;
}

void US_Properties::simulate( void )
{
   int row = lw_components->currentRow();
   if ( row < 0 ) return;
   US_Model::SimulationComponent* sc = &model.components[ row ];
  
   if ( sc->analyteGUID.isEmpty() )
      analyte.analyteGUID.clear(); 
   else
      analyte.analyteGUID = sc->analyteGUID;

   if ( sc->name.isEmpty() )
      analyte.description.clear();
   else
      analyte.description = sc->name;

   //hydro_data.density     = DENS_20W;
   //hydro_data.viscosity   = VISC_20W;
   //hydro_data.vbar        = analyte.vbar20;
   //hydro_data.temperature = NORMAL_TEMP;
   //hydro_data.mw          = analyte.mw;
   //hydro_data.axial_ration = ??
   //hydro_data.guid        = analyte.guid;


   working_data = hydro_data; // working_data will be updated

   US_Predict1* dialog = new US_Predict1( 
         working_data, analyte, db_access, true );

   connect( dialog, SIGNAL( changed       ( US_Analyte ) ), 
                    SLOT  ( new_hydro     ( US_Analyte ) ) );

   connect( dialog, SIGNAL( use_db        ( bool ) ), 
                    SLOT  ( source_changed( bool ) ) );
   dialog->exec();
}

void US_Properties::new_hydro( US_Analyte ad )
{
   int row = lw_components->currentRow();
   if ( row < 0 ) return;  // Should never happen

   US_Model::SimulationComponent* sc = &model.components[ row ];

   hydro_data       = working_data;
   //working_data.mw *= sc->oligomer;
   analyte          = ad;

   hydro_data.calculate( NORMAL_TEMP );

   // Set the name of the component
   if ( ! ad.description.isEmpty() )
   {
      lw_components->disconnect( SIGNAL( currentRowChanged( int ) ) );
      delete lw_components->currentItem();
      lw_components->insertItem( row, new QListWidgetItem( ad.description ) );
      lw_components->setCurrentRow( row );

      connect( lw_components, SIGNAL( currentRowChanged( int  ) ),
                              SLOT  ( update           ( int  ) ) );

      sc->name = ad.description;
      le_description->setText( ad.description );
   }

   // Set guid
   le_guid->setText( ad.analyteGUID );
   sc->analyteGUID = ad.analyteGUID;

   // Set extinction
   double exval = sc->extinction;
   //QList< double > keys;

   switch ( model.optics )
   {
      case US_Model::ABSORBANCE:
         if ( analyte.extinction.size() > 0 )
            exval = analyte.extinction[ model.wavelength ] * sc->oligomer;
         break;

      case US_Model::INTERFERENCE:
         if ( analyte.refraction.size() > 0 )
            exval = analyte.refraction[ model.wavelength ] * sc->oligomer;
         break;

      case US_Model::FLUORESCENCE:
         if ( analyte.fluorescence.size() > 0 )
            exval = analyte.fluorescence[ model.wavelength ] * sc->oligomer;
         break;
   }

   le_extinction->setText( QString::number( exval, 'e', 8 ) );
   sc->extinction = exval;

   // Set vbar(20), mw
   le_mw  ->setText( QString::number( hydro_data.mw,   'e', 8 ) );
   le_vbar->setText( QString::number( hydro_data.vbar, 'e', 8 ) );

   sc->mw          = hydro_data.mw;
   sc->vbar20      = hydro_data.vbar;
   sc->axial_ratio = hydro_data.axial_ratio;
   
   // Set f/f0, s, D, and f for shape
   cb_shape->setEnabled( true );
   int shape = cb_shape->itemData( cb_shape->currentIndex() ).toInt();
   select_shape( shape );

   inUpdate = true;
   set_molar();
   inUpdate = false;
}

void US_Properties::acceptProp( void ) 
{
   update( 0 );  // Parameter is ignored 
   emit done();
   accept();
}

void US_Properties::checkbox( void )
{
   if ( countChecks() != 2 )
   {
      us_setReadOnly( le_mw  , false );
      us_setReadOnly( le_s   , false );
      us_setReadOnly( le_D   , false );
      us_setReadOnly( le_f   , false );
      us_setReadOnly( le_f_f0, false );
      return;
   }

   us_setReadOnly( le_mw  , !ck_mw  ->isChecked() );
   us_setReadOnly( le_s   , !ck_s   ->isChecked() );
   us_setReadOnly( le_D   , !ck_D   ->isChecked() );
   us_setReadOnly( le_f   , !ck_f   ->isChecked() );
   us_setReadOnly( le_f_f0, !ck_f_f0->isChecked() );

   if ( (int)ct_oligomer->value() > 1 )
      us_setReadOnly( le_mw  , true  );
}

void US_Properties::enable( QLineEdit* le, bool status, const QPalette& p )
{
   le->setReadOnly( status );
   le->setPalette ( p );
}

int US_Properties::countChecks( void )
{
   int checked = 0;
   if ( ck_mw  ->isChecked() ) checked++;
   if ( ck_s   ->isChecked() ) checked++;
   if ( ck_D   ->isChecked() ) checked++;
   if ( ck_f   ->isChecked() ) checked++;
   if ( ck_f_f0->isChecked() ) checked++;
   return checked;
}

void US_Properties::setInvalid( void )
{
   if ( ! ck_mw  ->isChecked() ) le_mw  ->setText( tr( "n/a" ) );
   if ( ! ck_s   ->isChecked() ) le_s   ->setText( tr( "n/a" ) );
   if ( ! ck_D   ->isChecked() ) le_D   ->setText( tr( "n/a" ) );
   if ( ! ck_f   ->isChecked() ) le_f   ->setText( tr( "n/a" ) );
   if ( ! ck_f_f0->isChecked() ) le_f_f0->setText( tr( "n/a" ) );
   checkbox();
}

void US_Properties::co_sed( int new_state )
{
   if ( inUpdate ) return;

   if ( new_state == Qt::Checked )
   {
      int row = lw_components->currentRow();

      if ( model.coSedSolute != -1 )
      {
         int response = QMessageBox::question( this,
            tr( "Change co-sedimenting solute?" ),
            tr( "Another component is marked as the co-sedimenting solute.\n"
                "Change it to the current analyte?" ),
            QMessageBox::Yes, QMessageBox::No );

         if ( response == QMessageBox::No )
         {
             ck_co_sed->disconnect();
             ck_co_sed->setChecked( false );
             connect( ck_co_sed, SIGNAL( stateChanged( int ) ), 
                                 SLOT  ( co_sed      ( int ) ) );
             return;
         }
      }
      model.coSedSolute = row;
   }
   else
      model.coSedSolute = -1;
}

bool US_Properties::keep_standard( void )
{
   if ( le_guid->text().isEmpty() ) return false;

   int response = QMessageBox::question( this,
         tr( "Changing Standard Value" ),
         tr( "You are changing a value that does not correspond\n" 
             "with a saved analyte.\n\n"
             "Continue?" ),
         QMessageBox::Yes, QMessageBox::No );

   if ( response == QMessageBox::Yes )
   {
      clear_guid();
      //le_wavelength->clear();
      analyte.extinction  .clear();
      analyte.refraction  .clear();
      analyte.fluorescence.clear();
      cb_shape->setEnabled( false );
      return false;
   }

   return true;
}

void US_Properties::calculate( void )
{
   int    row  = lw_components->currentRow();

   if ( inUpdate  ||  row < 0 )
      return;

   US_Model::SimulationComponent* sc = &model.components[ row ];

   checkbox();

   // First do some sanity checking
   double vbar = le_vbar->text().toDouble();

   if ( row < 0  ||  vbar <= 0.0 )
      return;

   // Exactly two checkboxes must be set
   if ( countChecks() < 2 )
   {
      ck_mw  ->setEnabled( true );
      ck_f_f0->setEnabled( true );
      ck_f   ->setEnabled( true );
      ck_s   ->setEnabled( true );
      ck_D   ->setEnabled( true );
      return;
   }

   // disable all check boxes except for the two set
   ck_mw  ->setEnabled( ck_mw  ->isChecked() );
   ck_f_f0->setEnabled( ck_f_f0->isChecked() );
   ck_f   ->setEnabled( ck_f   ->isChecked() );
   ck_s   ->setEnabled( ck_s   ->isChecked() );
   ck_D   ->setEnabled( ck_D   ->isChecked() );
   
   // set values for checked boxes; clear others
   sc->mw   = ck_mw  ->isChecked() ? le_mw  ->text().toDouble() :
                         ( chgStoi ? sc->mw : 0.0 );
   sc->f_f0 = ck_f_f0->isChecked() ? le_f_f0->text().toDouble() : 0.0;
   sc->f    = ck_f   ->isChecked() ? le_f   ->text().toDouble() : 0.0;
   sc->s    = ck_s   ->isChecked() ? le_s   ->text().toDouble() : 0.0;
   sc->D    = ck_D   ->isChecked() ? le_D   ->text().toDouble() : 0.0;

   // re-calculate coefficients based on the two that are set
   US_Model::calc_coefficients( model.components[ row ] );

   // fill in text boxes with given and calculated coefficients
   le_mw  ->setText( QString::number( sc->mw  , 'e', 8 ) );
   le_f_f0->setText( QString::number( sc->f_f0, 'e', 8 ) );
   le_s   ->setText( QString::number( sc->s   , 'e', 8 ) );
   le_D   ->setText( QString::number( sc->D   , 'e', 8 ) );
   le_f   ->setText( QString::number( sc->f   , 'e', 8 ) );

   // search to see if present component is a product
   bool is_prod  = false;

   for ( int ii = 0; ii < model.associations.count(); ii++ )
   {
      US_Model::Association * as = &model.associations[ ii ];

      for ( int jj = 0; jj < as->rcomps.count(); jj++ )
      {
         if ( as->rcomps[ jj ] == row   &&  as->stoichs[ jj ] < 0 )
         {
            is_prod  = true;
            le_sigConc->setText( "0.0" );
            break;
         }
      }
   }

   ck_isProd ->setChecked( is_prod );
   le_sigConc->setEnabled( ! is_prod );
   us_setReadOnly( le_sigConc, is_prod );
}

void US_Properties::source_changed( bool db )
{
   emit use_db( db );  // Just pass on the signal
   qApp->processEvents();
}

// Slot to edit the currently selected analyte
void US_Properties::edit_analyte()
{
   int row       = lw_components->currentRow();
   QString aguid = le_guid->text();
   if ( aguid.isEmpty() )
      aguid         = model.components[ row ].analyteGUID;

   US_AnalyteGui* dialog = new US_AnalyteGui( true, aguid, db_access );

   connect( dialog, SIGNAL( valueChanged  ( US_Analyte ) ),
                    SLOT  ( update_analyte( US_Analyte ) ) );

   connect( dialog, SIGNAL( use_db        ( bool ) ), 
                    SLOT  ( source_changed( bool ) ) );

   // If accepted, work is done by update_analyte
   dialog->exec();
}

// Slot to make adjustments with molar check changed
void US_Properties::check_molar( bool chkd )
{
   // Set read-only states of concentration edits
   us_setReadOnly( le_molConc, !chkd );
   us_setReadOnly( le_sigConc,  chkd );

   // Flip check state of signal to opposite of molar
   ck_sigConc->disconnect();
   ck_sigConc->setChecked( !chkd );
   connect( ck_sigConc, SIGNAL( toggled(      bool ) ), 
                        SLOT(   check_signal( bool ) ) );
}

// Slot to make adjustments with signal check changed
void US_Properties::check_signal( bool chkd )
{
   // Set read-only states of concentration edits
   us_setReadOnly( le_sigConc, !chkd );
   us_setReadOnly( le_molConc,  chkd );

   // Flip check state of molar to opposite of signal
   ck_molConc->disconnect();
   ck_molConc->setChecked( !chkd );
   connect( ck_molConc, SIGNAL( toggled(      bool ) ), 
                        SLOT(   check_molar(  bool ) ) );
}

