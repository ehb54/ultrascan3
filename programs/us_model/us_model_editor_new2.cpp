//! \file us_model_editor.cpp

#include "us_model_editor_new2.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_constants.h"
#include "us_properties.h"
#include "us_investigator.h"
#include "us_buffer_gui.h"

US_ModelEditor::US_ModelEditor( 
      US_FemGlobal_New::ModelSystem& current_model,
      QWidget*                       p, 
      Qt::WindowFlags                f ) 
   : US_Widgets( true, p, f ), model( current_model )
{
   setWindowTitle( "Model Editor" );
   setPalette( US_GuiSettings::frameColor() );

   setWindowModality( Qt::WindowModal );
   
   c0_files.clear();
   prior_row  = -1;
   check      = QIcon( US_Settings::usHomeDir() + "/etc/check.png" );

   // Very light gray
   QPalette gray = US_GuiSettings::editColor();
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );

   investigator = -1;

   QGridLayout* main = new QGridLayout( this );
   main->setSpacing( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );

   int row = 0;
   
   // Start widgets
   QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
   connect( pb_investigator, SIGNAL( clicked() ), SLOT( get_person() ) );
   main->addWidget( pb_investigator, row, 0 );

   le_investigator = us_lineedit( );
   le_investigator->setPalette ( gray );
   le_investigator->setReadOnly( true );
   main->addWidget( le_investigator, row++, 1 );
   
   QGridLayout* db_layout = us_radiobutton( tr( "Use Database" ), rb_db );
   main->addLayout( db_layout, row, 0 );

   QGridLayout* disk_layout = us_radiobutton( tr( "Use Local Disk" ), rb_disk, true );
   main->addLayout( disk_layout, row++, 1 );

   QPushButton* pb_models = us_pushbutton( tr( "List Available Models" ) );
   //connect( pb_models, SIGNAL( clicked() ), SLOT( load_models() ) );
   main->addWidget( pb_models, row, 0 );

   QPushButton* pb_new = us_pushbutton( tr( "Create New Model" ) );
   connect( pb_new, SIGNAL( clicked() ), SLOT( new_model() ) );
   main->addWidget( pb_new, row++, 1 );

   // Models List Box
   lw_models = new US_ListWidget;
   lw_models->setToolTip( 
         tr( "Double click to edit description" ) );

   //connect( lw_models, SIGNAL( currentRowChanged( int  ) ),
   //                    SLOT  ( change_model     ( int  ) ) );

   //connect( lw_models, SIGNAL( itemDoubleClicked( QListWidgetItem* ) ),
   //                    SLOT  ( change_model     ( QListWidgetItem* ) ) );

   main->addWidget( lw_models, row, 0, 5, 2 );
   row += 5;

   QPushButton* pb_components = us_pushbutton( tr( "Manage Components" ) );
   connect( pb_components, SIGNAL( clicked() ), SLOT( manage_components() ) );
   main->addWidget( pb_components, row, 0 );

   QPushButton* pb_associations = us_pushbutton( tr( "Manage Associations" ) );
   //connect( pb_associations, SIGNAL( clicked() ), SLOT( associations() ) );
   main->addWidget( pb_associations, row++, 1 );

   QPushButton* pb_buffer = us_pushbutton( tr( "Select Buffer" ) );
   connect( pb_buffer, SIGNAL( clicked() ), SLOT( get_buffer() ) );
   main->addWidget( pb_buffer, row, 0 );

   le_buffer = us_lineedit( );
   le_buffer->setPalette ( gray );
   le_buffer->setReadOnly( true );
   main->addWidget( le_buffer, row++, 1 );

   QLabel* lb_density = us_label( tr( "Density:" ) );
   main->addWidget( lb_density, row, 0 );

   le_density = us_lineedit( );
   le_density->setPalette ( gray );
   le_density->setReadOnly( true );
   main->addWidget( le_density, row++, 1 );

   QLabel* lb_viscosity = us_label( tr( "Viscosity:" ) );
   main->addWidget( lb_viscosity, row, 0 );

   le_viscosity = us_lineedit( );
   le_viscosity->setPalette ( gray );
   le_viscosity->setReadOnly( true );
   main->addWidget( le_viscosity, row++, 1 );

   QLabel* lb_compressibility = us_label( tr( "Compressibility:" ) );
   main->addWidget( lb_compressibility, row, 0 );

   le_compressibility = us_lineedit( );
   le_compressibility->setPalette ( gray );
   le_compressibility->setReadOnly( true );
   main->addWidget( le_compressibility, row++, 1 );

   QLabel* lb_temperature = us_label( tr( "Temperature:" ) );
   main->addWidget( lb_temperature, row, 0 );

   le_temperature = us_lineedit( "20.0" );
   main->addWidget( le_temperature, row++, 1 );

   QLabel* lb_optics = us_label( tr( "Optical System:" ) );
   main->addWidget( lb_optics, row, 0 );

   cb_optics = us_comboBox();
   cb_optics->addItem( tr( "Absorbance"   ), ABSORBANCE   );
   cb_optics->addItem( tr( "Interference" ), INTERFERENCE );
   cb_optics->addItem( tr( "Fluorescence" ), FLUORESCENCE );
   main->addWidget( cb_optics, row++, 1 );
   
   QPushButton* pb_save = us_pushbutton( tr( "Save / Update Model" ) );
   //connect( pb_save, SIGNAL( clicked() ), SLOT( save_model() ) );
   main->addWidget( pb_save, row, 0 );

   QPushButton* pb_delete = us_pushbutton( tr( "Delete Selected Model" ) );
   //connect( pb_delete, SIGNAL( clicked() ), SLOT( delete_model() ) );
   main->addWidget( pb_delete, row++, 1 );

   // Pushbuttons
   QBoxLayout* buttonbox = new QHBoxLayout;

   QPushButton* pb_help = us_pushbutton( tr( "Help") );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help()) );
   buttonbox->addWidget( pb_help );

   QPushButton* pb_close = us_pushbutton( tr( "Cancel") );
   buttonbox->addWidget( pb_close );
   connect( pb_close, SIGNAL( clicked() ), SLOT( close() ) );

   QPushButton* pb_accept = us_pushbutton( tr( "Accept") );
   buttonbox->addWidget( pb_accept );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( accept_model()) );

   main->addLayout( buttonbox, row++, 0, 1, 4 );
}

void US_ModelEditor::get_person( void )
{
   US_Investigator* dialog = new US_Investigator( true );
   
   connect( dialog, 
      SIGNAL( investigator_accepted( int, const QString&, const QString& ) ),
      SLOT  ( update_person        ( int, const QString&, const QString& ) ) );

   dialog->exec();
}

void US_ModelEditor::update_person( int            ID, 
                                    const QString& lname, 
                                    const QString& fname )
{
   investigator = ID;

   if ( ID < 0 ) 
      le_investigator->setText( "" );
   else
      le_investigator->setText( 
            QString::number( ID ) + ": " + lname + ", " + fname );
}

void US_ModelEditor::get_buffer( void )
{
   US_BufferGui* dialog = new US_BufferGui( investigator, true );
   
   connect( dialog, SIGNAL( valueChanged ( US_Buffer ) ),
                    SLOT  ( update_buffer( US_Buffer ) ) );

   dialog->exec();
}

void US_ModelEditor::update_buffer( US_Buffer buf )
{
   buffer = buf;
   le_density        ->setText( QString::number( buf.density        , 'f', 4 ) );
   le_viscosity      ->setText( QString::number( buf.viscosity      , 'f', 4 ) );
   le_compressibility->setText( QString::number( buf.compressibility, 'e', 3 ) );
   le_buffer         ->setText( buf.description );
}

void US_ModelEditor::new_model( void )
{
   ModelDesc desc;
   desc.description = "New Model";
   desc.filename    = "";
   desc.DB_id       = -1;

   model_descriptions << desc;
   show_model_desc();
   lw_models->setCurrentRow( model_descriptions.size() - 1 );
}

void US_ModelEditor::show_model_desc( void )
{
   lw_models->clear();

   for ( int i = 0; i < model_descriptions.size(); i++ )
      lw_models->addItem( model_descriptions[ i ].description );
}

void US_ModelEditor::manage_components( void )
{
   int index = lw_models->currentRow();

   if ( index < 0 )
   {
      QMessageBox::information( this,
         tr( "Model not selected" ),
         tr( "Please select a model first.\n" 
             "If necessary, create a new model." ) );
      return;
   }

   US_Properties* dialog = 
      new US_Properties( buffer, model, investigator, rb_db->isChecked() );
   //connect( dialog, SIGNAL( valueChanged( US_Predict1::Hydrosim ) ),
   //                 SLOT  ( update_sim  ( US_Predict1::Hydrosim ) ) );
   dialog->exec();
}
//////////////////////////////////////

void US_ModelEditor::show_component( void )
{
   int index = lw_components->currentRow();
   if ( index >= 0 )
   {
      prior_row = index;

      US_FemGlobal_New::SimulationComponent* sc = &components[ index ];
      QListWidgetItem* item = lw_components->item( index );
      item->setText( QString::number( index + 1 ) + ": " + sc->name );

      le_sed  ->setText( QString::number( sc->s,      'e', 4 ) );
      le_diff ->setText( QString::number( sc->D,      'e', 4 ) );
      le_vbar ->setText( QString::number( sc->vbar20, 'e', 4 ) );
      le_mw   ->setText( QString::number( sc->mw,     'e', 4 ) );
      le_f_f0 ->setText( QString::number( sc->f_f0,   'e', 4 ) );
      le_sigma->setText( QString::number( sc->sigma,  'e', 4 ) );
      le_delta->setText( QString::number( sc->delta,  'e', 4 ) );

      le_conc ->setText( QString::number( sc->molar_concentration,  'e', 4 ) );
      le_coeff->setText( QString::number( sc->signal_concentration, 'e', 4 ) );

      // Set or clear check in Load C0 push button as appropriate
      if ( sc->c0.radius.size() > 0 )
         pb_load_c0->setIcon( check );
      else
         pb_load_c0->setIcon( QIcon() );

      pb_load_c0->setToolTip( c0_files[ index ] );
   }
}
/*
void US_ModelEditor::new_component( void )
{
   US_FemGlobal_New::SimulationComponent sc;
   components << sc;

   // Create empty structures for optics readings
   QMap< double, double > map;
   extinction_maps   << map;
   refraction_maps   << map;
   fluorescence_maps << map;

   c0_files << QString();

   US_Predict1::Hydrosim sim;
   hydrosim_maps << sim;
   hydrosim_valid << false;

   int index = lw_components->count();

   QListWidgetItem* item = 
      new QListWidgetItem( QString::number( index + 1 ) + ": " + sc.name );
   
   item->setFlags( item->flags() | Qt::ItemIsEditable );
   lw_components->addItem( item );

   cb_shape->setCurrentIndex( 0 );
   lw_components->setCurrentRow( index ); // Connection calls show_component()
}
*/
void US_ModelEditor::get_vbar( void )
{
   int index = lw_components->currentRow();
   if ( index < 0 ) return; 

   US_Analyte* analyte = new US_Analyte( -1, true );

   connect( analyte, 
         SIGNAL( valueChanged  ( struct US_Analyte::analyteData ) ),
         SLOT  ( update_analyte( struct US_Analyte::analyteData ) ) );
   analyte->exec();
}

void US_ModelEditor::update_analyte( struct US_Analyte::analyteData data )
{
   int index = lw_components->currentRow();
   if ( index < 0 ) return; 

   US_FemGlobal_New::SimulationComponent* sc = &components[ index ];

   extinction = data.extinction;
   sc->vbar20 = data.vbar;
   sc->mw     = data.mw;
   sc->name   = data.description;

   show_component();
}

void US_ModelEditor::change_optics( int index )
{
   switch ( index )
   {
      case ABSORBANCE: 
         lb_coeff->setText( tr ( "Optical Density:" ) );
         break;

      case INTERFERENCE:
         lb_coeff->setText( tr ( "Fringes:" ) );
         break;

      default: // FLUORESCENCE
         lb_coeff->setText( tr ( "Intensity:" ) );
         break;
   }

   int row = lw_components->currentRow();

   if ( row > -1 )
   {
      US_FemGlobal_New::SimulationComponent* sc = &components[ row ];
      sc->shape = (US_FemGlobal_New::ShapeType) index;
   }
}

void US_ModelEditor::delete_component( void )
{
   int index = lw_components->currentRow();
   if ( index < 0 ) return;

   int response =  QMessageBox::question( this,
         tr( "Remove Component" ),
         tr( "Do you really want to remove this component?" ),
         QMessageBox::Yes, QMessageBox::Cancel );

   if ( response != QMessageBox::Yes ) return;

   delete lw_components->item( index );  // Remove from the list widget
   components       .remove( index );    // Remove from memory
   extinction_maps  .remove( index );
   refraction_maps  .remove( index );
   fluorescence_maps.remove( index );
   hydrosim_maps    .remove( index );
   hydrosim_valid   .remove( index );
   c0_files         .takeAt( index );

   int last_index = lw_components->count() - 1;
   if ( last_index == -1 ) return;

   if ( index > last_index ) index = last_index;  // Was last item deleted?

   lw_components->setCurrentRow( index );
   show_component();  // Update all values
}

void US_ModelEditor::change_component( int index )
{
   // Update current component from window
   if ( prior_row > -1 )
   {
      US_FemGlobal_New::SimulationComponent* sc = &components[ prior_row ];
      
      QListWidgetItem* prior = lw_components->item( prior_row );
      QStringList sl = prior->text().split( ": ");
      
      if ( sl.size() > 1 )
         sc->name = sl[ 1 ];
      else
         sc->name = prior->text();

      sc->s      = le_sed  ->text().toDouble();
      sc->D      = le_diff ->text().toDouble();
      sc->vbar20 = le_vbar ->text().toDouble();
      sc->mw     = le_mw   ->text().toDouble();
      sc->f_f0   = le_f_f0 ->text().toDouble();
      sc->sigma  = le_sigma->text().toDouble();
      sc->delta  = le_delta->text().toDouble();

      sc->molar_concentration  = le_conc ->text().toDouble();
      sc->signal_concentration = le_coeff->text().toDouble();

   }

   cb_shape->setCurrentIndex( components[ index ].shape );
   show_component();
}

void US_ModelEditor::accept_model( void )
{
   // Populate the model and return it
   model.description     = le_description->text();
   model.compressibility = buffer.compressibility;
   //model.components   = 
   //model.coSedSolute  = 
   //model.associations = 
   
   emit valueChanged( model );
   close();
}

void US_ModelEditor::simulate_component( void )
{
   int index = lw_components->currentRow();

   if ( index < 0 )
   {
      QMessageBox::information( this,
         tr( "Component not selected" ),
         tr( "Please select a component first.\n" 
             "If necessary, create a new component." ) );
      return;
   }

   //US_Properties* dialog = new US_Properties();
   //connect( dialog, SIGNAL( valueChanged( US_Predict1::Hydrosim ) ),
   //                 SLOT  ( update_sim  ( US_Predict1::Hydrosim ) ) );
   //dialog->exec();
}

void US_ModelEditor::update_sim( US_Predict1::Hydrosim sim )
{
   le_mw->setText( QString::number( sim.mw, 'e', 4 ) );

}



void US_ModelEditor::update_shape( void )
{
   int index               = lw_components->currentRow();
   hydrosim_valid[ index ] = true;
   
   US_FemGlobal_New::SimulationComponent* sc = &components   [ index ];
   US_Predict1::Hydrosim*                 h  = &hydrosim_maps[ index ];

   int shape = cb_shape->itemData( cb_shape->currentIndex() ).toInt();

//qDebug() << "Prolate" << h->prolate.sedcoeff << h->prolate.diffcoeff << h->prolate.f_f0;
//qDebug() << "Oblate" << h->oblate.sedcoeff << h->oblate.diffcoeff << h->oblate.f_f0;
//qDebug() << "Rod" << h->rod.sedcoeff << h->rod.diffcoeff << h->rod.f_f0;
//qDebug() << "Sphere" << h->sphere.sedcoeff << h->sphere.diffcoeff << h->sphere.f_f0;
   switch ( shape )
   {
      case US_FemGlobal_New::PROLATE:
         sc->s     = h->prolate.sedcoeff;
         sc->D     = h->prolate.diffcoeff;
         sc->f_f0  = h->prolate.f_f0;
         sc->shape = US_FemGlobal_New::PROLATE;
         break;
      
      case US_FemGlobal_New::OBLATE:
         sc->s     = h->oblate.sedcoeff;
         sc->D     = h->oblate.diffcoeff;
         sc->f_f0  = h->oblate.f_f0;
         sc->shape = US_FemGlobal_New::OBLATE;
         break;

      case US_FemGlobal_New::ROD:
         sc->s     = h->rod.sedcoeff;
         sc->D     = h->rod.diffcoeff;
         sc->f_f0  = h->rod.f_f0;
         sc->shape = US_FemGlobal_New::ROD;
         break;

      case US_FemGlobal_New::SPHERE:
         sc->s     = h->sphere.sedcoeff;
         sc->D     = h->sphere.diffcoeff;
         sc->f_f0  = h->sphere.f_f0;
         sc->shape = US_FemGlobal_New::SPHERE;
         break;
   }

   sc->mw      = h->mw;
   sc->vbar20  = h->vbar;
   
   show_component();
}

void US_ModelEditor::select_shape( int /*new_shape*/ )
{
   int index = lw_components->currentRow();
   if ( index < 0 ) return;

//qDebug() << "" << hydrosim_valid[ index ];

   if ( ! hydrosim_valid[ index ] ) return;
   
   US_FemGlobal_New::SimulationComponent* sc = &components   [ index ];
   US_Predict1::Hydrosim*                 h  = &hydrosim_maps[ index ];

   int shape = cb_shape->currentIndex();

   switch ( shape )
   {
      case US_FemGlobal_New::PROLATE:
         sc->s     = h->prolate.sedcoeff;
         sc->D     = h->prolate.diffcoeff;
         sc->f_f0  = h->prolate.f_f0;
         sc->shape = US_FemGlobal_New::PROLATE;
         break;
      
      case US_FemGlobal_New::OBLATE:
         sc->s     = h->oblate.sedcoeff;
         sc->D     = h->oblate.diffcoeff;
         sc->f_f0  = h->oblate.f_f0;
         sc->shape = US_FemGlobal_New::OBLATE;
         break;

      case US_FemGlobal_New::ROD:
         sc->s     = h->rod.sedcoeff;
         sc->D     = h->rod.diffcoeff;
         sc->f_f0  = h->rod.f_f0;
         sc->shape = US_FemGlobal_New::ROD;
         break;

      case US_FemGlobal_New::SPHERE:
         sc->s     = h->sphere.sedcoeff;
         sc->D     = h->sphere.diffcoeff;
         sc->f_f0  = h->sphere.f_f0;
         sc->shape = US_FemGlobal_New::SPHERE;
         break;
   }

   show_component();
}

void US_ModelEditor::load_c0( void )
{
   int index = lw_components->currentRow();
   if ( index < 0 ) return;

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
         this, tr( "Select C0 File" ), US_Settings::resultDir(), "*" );
   
   if ( ! fn.isEmpty() )
   {
      QFile f( fn );;

      if ( f.open( QIODevice::ReadOnly | QIODevice::Text ) )
      {
         c0_files[ index ] = fn;
         QTextStream ts( &f );

         US_FemGlobal_New::SimulationComponent* sc = &components[ index ];

         sc->c0.radius.clear();
         sc->c0.concentration.clear();
         
         double radius;
         double concentration;
         
         while ( ! ts.atEnd() )
         {
            ts >> radius;
            ts >> concentration;

            if ( radius > 0.0 ) // Ignore radius pairs that aren't positive
            {
               sc->c0.radius       .push_back( radius );
               sc->c0.concentration.push_back( concentration );
            }
         }

         f.close();
         pb_load_c0->setIcon( check );
         pb_load_c0->setToolTip( fn );
      }
      else
      {
         QMessageBox::warning( this, 
               tr( "UltraScan Warning" ),
               tr( "UltraScan could not open the file specified\n" ) + fn );
      }
   }
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////
#ifdef xxx
void US_ModelEditor::change_spinbox( int /*value*/ )
{
   
   //cmb_component1->disconnect();
   //cmb_component1->setCurrentIndex( component );

   //connect( cmb_component1, SIGNAL( currentIndexChanged( int ) ),
   //                         SLOT  ( change_component1  ( int ) ) );

   update_component();
}

void US_ModelEditor::update_component( void )
{
   /*
   // Convenience 
   US_FemGlobal::SimulationComponent*           sc  = &model.components[ component ]; 
   vector< US_FemGlobal::SimulationComponent >* scl = &model.components; 
   
   le_sed       ->setText( QString::number( sc->s         , 'e', 4 ) );
   le_diff      ->setText( QString::number( sc->D         , 'e', 4 ) );
   //le_extinction->setText( QString::number( sc->extinction, 'e', 4 ) );
   le_vbar      ->setText( QString::number( sc->vbar20    , 'e', 4 ) );
   le_mw        ->setText( QString::number( sc->mw        , 'e', 4 ) );
   le_f_f0      ->setText( QString::number( sc->f_f0      , 'e', 4 ) );
   
   // Find the associated components for component1 and enter them into the
   // linked component list:

   //cmb_component2->clear();

   //for ( uint i = 0; i < sc->show_component.size(); i++ )
   //   cmb_component2->addItem( (*scl) [ sc->show_component[ i ] ].name );

   if ( sc->show_conc )
   {
      le_conc   ->setEnabled( true );
      le_mw     ->setEnabled( true );
      le_vbar   ->setEnabled( true );
      pb_load_c0->setEnabled( true );
      pb_vbar   ->setEnabled( true );

      //if ( sc->concentration == -1.0 )
      {
         le_conc->setText( "from file" );
         le_c0  ->setText( c0_file );
      }
      //else
      {
         //le_conc->setText( QString::number( sc->concentration, 'e', 4 ) );
         sc->c0.radius.clear();
         sc->c0.concentration.clear();
      }
   }
   else
   {
      // Can't edit an associated species' mw or vbar
      le_mw     ->setEnabled( false ); 

      le_vbar   ->setEnabled( false ); 
      pb_vbar   ->setEnabled( false ); 
      
      le_conc   ->setEnabled( false );
      le_conc   ->setText   ( ""    );

      pb_load_c0->setEnabled( false );
      le_c0     ->setText   ( ""    );
   }
*/   
   // Convenience
   //vector< US_FemGlobal::Association >* assoc = &model.associations;
/*
   if ( sc->show_keq )
   {
      for ( uint i = 0; i < assoc->size(); i++ )
      { // only check the dissociating species
         if ( (*assoc)[ i ].component2 == component
         ||   (*assoc)[ i ].component3 == component )
         {
            le_keq ->setText( QString::number( (*assoc)[ i ].keq, 'e', 4 ) );
            le_keq ->setEnabled( true );
            
            le_koff->setText( QString::number( (*assoc)[ i ].k_off, 'e', 4 ) );
            le_koff->setEnabled( true );
         }
      }
   }
   else
   {
      le_keq->setEnabled ( false );
      le_keq->setText    ( ""    );

      le_koff->setEnabled( false );
      le_koff->setText   ( ""    );
   }
   
   if ( sc->show_stoich > 0 )
   { 
      // This species is the dissociating species in a self-associating system
      
      // le_stoich->setEnabled( true );
      le_stoich->setText( QString::number( sc->show_stoich ) );
      sc->mw =
            (*scl) [ sc->show_component[ 0 ] ].mw * sc->show_stoich;
      

      le_mw->setText( QString::number( sc->mw, 'e', 4 ) );


      sc->vbar20 =
            (*scl) [ sc->show_component[ 0 ] ].vbar20;
      
      le_vbar->setText( QString::number( sc->vbar20, 'e', 4 ) );

      update_sD();
   }
   else if ( sc->show_stoich == -1 )
   { 
      // This species is the dissociating species in a 2-component 
      // hetero-associating system
      
      le_stoich->setText( "hetero-associating" );
      
      sc->mw = (*scl) [ sc->show_component[ 0 ] ].mw +
               (*scl) [ sc->show_component[ 1 ] ].mw;

      le_mw->setText( QString::number( sc->mw, 'e', 4 ) );
      
      double fraction1 = (*scl)[ sc->show_component[ 0 ] ].mw / sc->mw;
      double fraction2 = (*scl)[ sc->show_component[ 1 ] ].mw / sc->mw;

      sc->vbar20 = (*scl) [ sc->show_component[ 0 ] ].vbar20 * fraction1 +
                   (*scl) [ sc->show_component[ 1 ] ].vbar20 * fraction2;
      
      le_vbar->setText( QString::number( sc->vbar20, 'e', 4 ) );
      
      update_sD();
   }
   else
   {
      le_stoich->setText   ( "" );
      le_stoich->setEnabled( false );
   }
*/
}

void US_ModelEditor::update_sD( void )
{
   /*
   // Convenience 
   SimulationComponent* sc = &model.components[ component ]; 

   double base = 0.75 / AVOGADRO * sc->mw * sc->vbar20 * M_PI * M_PI;

   sc->s = sc->mw * ( 1.0 - sc->vbar20 * DENS_20W )
           / ( AVOGADRO * sc->f_f0 * 6.0 * VISC_20W * pow( base, 1.0 / 3.0 ) );

   base =  2.0 * sc->s * sc->f_f0 * sc->vbar20 * VISC_20W 
           / ( 1.0 - sc->vbar20 * DENS_20W );

   sc->D = R * K20 
           / ( AVOGADRO *  sc->f_f0 * 9.0 * VISC_20W * M_PI * pow( base, 0.5 ) );

   le_sed ->setText( QString::number( sc->s, 'e', 4 ) );
   le_diff->setText( QString::number( sc->D, 'e', 4 ) );
   */
}


void US_ModelEditor::load_model()
{
   QString fn = QFileDialog::getOpenFileName( 
         this, 
         tr( "Load Model File" ),
         US_Settings::resultDir(), 
         "*.model.?? *.model-?.??  *model-??.??" );

   if ( ! fn.isEmpty() )
   {
      /*
      int code = US_FemGlobal::read_modelSystem( model, fn );

      if ( code == 0 )
      {
         // Successfully loaded a new model
         QMessageBox::information( this, 
               tr( "Simulation Module"), 
               tr( "Successfully loaded Model:" ) );
         
         lb_header->setText( model.description );

         cmb_component1->disconnect();
         cmb_component1->clear();  
         
         vector< SimulationComponent >* scl = &model.components;

         for ( uint i = 0; i< (*scl).size(); i++)
         {
            cmb_component1->addItem( ( *scl )[ i ].name );
         }
         cmb_component1->setCurrentIndex( 0 );
         connect( cmb_component1, SIGNAL( currentIndexChanged( int ) ),
                                  SLOT  ( change_component1  ( int ) ) );


         component = 0;
         sb_count->setRange( 1, (*scl).size() );
         sb_count->setValue( 1 );  // Resets screen

         //select_component((int) 0);
      }

      else if ( code == -40 )
      {
         QMessageBox::warning( this, 
               tr( "UltraScan Warning" ), 
               tr( "Please note:\n\n"
                   "UltraScan could not open\n"
                   "the selected Model File!" ) );
      }
      
      else if ( code == 1 )
      {
         QMessageBox::warning( this, 
               tr( "UltraScan Warning" ),
               tr( "Sorry, the old-style models are no longer supported.\n\n"
                   "Please load a different model or create a new Model" ) );
      }

      else if ( code < 0 && code > -40 )
      {
         QMessageBox::warning( this, 
               tr( "UltraScan Warning" ),
               tr( "Please note:\n\n"
                   "There was an error reading\n"
                   "the selected Model File!\n\n"
                   "This file appears to be corrupted" ) );
      } 
      */
   }
}

void US_ModelEditor::save_model( void )
{
   if ( ! verify_model() ) return;
 
   QString fn = QFileDialog::getSaveFileName( this, 
         tr( "Save model as" ),
         US_Settings::resultDir(), 
         "*.model.?? *.model-?.?? *model-??.??" );

   if ( ! fn.isEmpty() )
   {
      int k = fn.lastIndexOf( "." );
      
      // If an extension was given, strip it.
      if ( k != -1 )  fn.resize( k );
/*
      fn += ".model-" + QString::number( model.model ) + ".11";

      QFile f( fn );

      if ( f.exists() )
      {
         int answer =  QMessageBox::question( this,
                  tr( "Warning" ), 
                  tr( "Attention:\n"
                      "This file exists already!\n\n"
                      "Do you want to overwrite it?" ), 
                  QMessageBox::Yes, QMessageBox::No );
         
         if ( answer == QMessageBox::No ) return;
      }

      int result = US_FemGlobal::write_modelSystem( model, fn );
      if ( result != 0 ) error( tr( "Could not write file." ) );
*/
   }
}


/*
// The next five methods are virtually identical.  It would be nice
// to combine them.  It would require a custom class and signal.
void US_ModelEditor::update_sed( const QString& text )
{
   if ( text == "" ) return;
   
//   SimulationComponent* sc = &model.components[ component ];
//   sc->s = text.toDouble();
}

void US_ModelEditor::update_diff( const QString& text )
{
   if ( text == "" ) return;
   
//   SimulationComponent* sc = &model.components[ component ];
//   sc->D = text.toDouble();
}

void US_ModelEditor::update_extinction( const QString& text )
{
   if ( text == "" ) return;
   
//   SimulationComponent* sc = &model.components[ component ];
//   sc->extinction = text.toDouble();
}

void US_ModelEditor::update_sigma( const QString& text )
{
   if ( text == "" ) return;
   
//   SimulationComponent* sc = &model.components[ component ];
//   sc->sigma = text.toDouble();
}

void US_ModelEditor::update_delta( const QString& text )
{
   if ( text == "" ) return;
   
//   SimulationComponent* sc = &model.components[ component ];
//   sc->delta = text.toDouble();
}
*/
// The next two methods are virtually identical.  It would be nice
// to combine them.
void US_ModelEditor::update_vbar( const QString& text )
{
//   SimulationComponent* sc   = &model.components[ component ];
   double               vbar = text.toDouble();
   
   if ( vbar <= 0.0 )
   {
      error( tr( "The vbar value must be greater than 0.0" ) );
  //    le_vbar->setText( QString::number( sc->vbar20, 'e', 4 ) );
      return;
   }
   
  // sc->vbar20 = vbar;
}

void US_ModelEditor::update_mw( const QString& /*text*/ )
{
   /*
   SimulationComponent* sc = &model.components[ component ];
   double               mw = text.toDouble();
   
   if ( mw <= 0.0 )
   {
      error( tr( "The Molecular weight must be greater than 0.0" ) );
      le_mw->setText( QString::number( sc->mw, 'e', 4 ) );
      return;
   }
   
   sc->mw = mw;
   */
}

void US_ModelEditor::update_f_f0( const QString& text )
{
   if ( text == "" ) return;
   
   //US_FemGlobal::SimulationComponent* sc = &model.components[ component ];
   //sc->f_f0 = text.toDouble();
}

void US_ModelEditor::update_conc( const QString& text )
{
   //US_FemGlobal::SimulationComponent* sc = &model.components[ component ];

   if ( text == "" ) return;
/*   
   if ( text == "from file") 
      sc->molar_concentration = -1.0;
   else
   {
      bool   ok;
      double concentration;
      concentration = text.toDouble( &ok );
      if ( ok )
      {
         //sc->molar_concentration = concentration;
         //sc->c0.radius.clear();
         //sc->c0.concentration.clear();

         //le_c0->setText( "" );
      }
      else
        error( tr( "The Partial Concentration field is invalid." ) );
   }
   */
}

// The next two methods are virtually identical.  It would be nice
// to combine them
void US_ModelEditor::update_keq( const QString& text )
{
   if ( text == "" ) return;
/*
   vector< US_FemGlobal::Association >* av = &model.associations;

   // Check to see if the current component is a dissociation component
      
   for ( uint i = 0; i < (*av).size(); i++ )
   {
      US_FemGlobal::Association* as = &model.associations[ i ];
      
      // Check to see if there is an dissociation linked to this component
      if ( as->reaction_components[ 1 ] == component )
      {
         // Check to make sure this component is not an irreversible component
         if ( as->stoichiometry[ 0 ] != as->reaction_components[ 1 ] // Self-association
              || (    as->stoichiometry[ 0 ] == 0 
                   && as->stoichiometry[ 1 ] == 0))        // Hetero-association
         {
            as->keq = text.toDouble();
         }
      }
   }
   */
}

void US_ModelEditor::update_koff( const QString& text )
{
   if ( text == "" ) return;
/*
   vector< US_FemGlobal::Association >* av = &model.associations;

   // Check to see if the current component is a dissociation component
   for ( uint i = 0; i < (*av).size(); i++ )
   {
      US_FemGlobal::Association* as = &model.associations[ i ];
      
      // Check to see if there is an dissociation linked to this component
      if ( as->reaction_components[ 0 ] == component )
      {
         // Check to make sure this component is not an irreversible component
         if ( as->stoichiometry[ 0 ] != as->stoichiometry[ 1 ] // Self-association
              || (    as->stoichiometry[ 0 ] == 0 
                   && as->stoichiometry[ 1 ] == 0))        // Hetero-association
         {
            as->k_off = text.toDouble();
         }
      }
   }
   */
}

void US_ModelEditor::error( const QString& message )
{
   QMessageBox::warning( this, tr( "Model Error" ), message );
}

bool US_ModelEditor::verify_model( void )
{
   bool flag = true;
/*
   QString str;

   vector< US_FemGlobal::Association >*         av = &model.associations;
   //vector< US_FemGlobal::SimulationComponent >* cv = &model.components;
   
   for ( uint i=0; i < (*av).size(); i++ )
   {
      US_FemGlobal::Association* as = &model.associations[ i ];

      // See if we need to check if the MWs match
      if ( as->stoichiometry[ 1 ] > 0 && as->stoichiometry[ 2 ] != 1 )
      {
         if ( fabs( (*cv) [ as->reaction_components[ 1 ] ].mw -
                    (  (*cv) [ as->reaction_components[ 0 ] ].mw *
                       as->stoichiometry[ 1 ] / as->stoichiometry[ 0 ]
                    ) ) > 1.0 ) // MWs don't match within 1 dalton
         {
            str = tr( "The molecular weights of the reacting species\n" 
                      "in reaction " ) + QString::number( i + 1 ) +
                  tr( "do not agree:\n\n" )

                + tr( "Molecular weight of species 1: " ) + 
                  QString::number( (*cv) [ as->reaction_components[ 0 ]  ].mw, 'e', 4 ) + "\n"

                + tr( "Molecular weight of species 2:" ) +
                  QString::number( (*cv) [ as->reaction_components[ 1 ]  ].mw, 'e', 4 ) + "\n"

                + tr( "Stoichiometry of reaction " ) +
                  QString::number( i + 1 ) + tr( ": MW(1) *" ) +
                  QString::number( as->stoichiometry[ 1 ] ) + tr( " = MW(2)\n\n" ) 

                + tr( "Please adjust either MW(1) or MW(2) before proceeding..." );
            
            QMessageBox::warning( this, tr( "Model Definition Error" ), str );
            flag = false;
         }
      }

      // See if we need to check if the sum of MW(1) + MW(2) = MW(3)
      if ( as->stoichiometry[ 2 ] == 1 ) 
      {
         if ( fabs( model.components[ as->reaction_components[ 2 ] ].mw -
                    model.components[ as->reaction_components[ 1 ] ].mw -
                    model.components[ as->reaction_components[ 0 ] ].mw ) > 1.0 ) 
            // MWs don't match within 10 dalton
         {
            str = tr( "The molecular weights of the reacting species\n"
                      "in reaction ") + QString::number( i + 1 ) + 
                  tr( "do not agree:\n\n" )
            
                + tr( "Molecular weight of species 1: " ) +
                  QString::number( (*cv) [ as->reaction_components[ 0 ] ].mw, 'e', 4 ) + "\n"
              
                + tr( "Molecular weight of species 2: " ) +
                  QString::number( (*cv) [ as->reaction_components[ 1 ] ].mw, 'e', 4 ) + "\n"
                
                + tr( "Molecular weight of species 3: " ) +
                  QString::number( (*cv) [ as->reaction_components[ 2 ] ].mw, 'e', 4 ) + "\n"
                
                + tr( "Stoichiometry of reaction " ) +
                  QString::number( i + 1 ) + tr( ": MW(1) + MW(2) = MW(3)\n\n" )
                
                + tr( "Please adjust the molecular weight of the appropriate\n"
                      "component before proceeding..." );

            QMessageBox::warning( this, tr( "Model Definition Error" ), str );
            flag = false;
         }
      }
   }
*/
   return (flag);
}
#endif
