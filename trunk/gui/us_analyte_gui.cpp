//! \file us_analyte_gui.cpp
#include "us_analyte_gui.h"
#include "us_passwd.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_investigator.h"
#include "us_editor_gui.h"
#include "us_table.h"

#include <uuid/uuid.h>

US_AnalyteGui::US_AnalyteGui( int             invID, 
                              bool            signal, 
                              const QString&  GUID,
                              bool            access,
                              double          temp )
   : US_WidgetsDialog( 0, 0 ), 
     personID     ( invID ), 
     signal_wanted( signal ),
     guid         ( GUID ), 
     db_access    ( access ),
     temperature  ( temp )
{
   setWindowTitle( tr( "Analyte Management" ) );
   setPalette( US_GuiSettings::frameColor() );
   setAttribute( Qt::WA_DeleteOnClose );

   QPalette normal = US_GuiSettings::editColor();

   // Very light gray
   QPalette gray = normal;
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );
   
   QGridLayout* main = new QGridLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   int row = 0;

   QStringList DB = US_Settings::defaultDB();
   QString     db_name;

   if ( DB.size() < 5 )
   {
      QMessageBox::warning( this,
         tr( "Attention" ),
         tr( "There is no default database set." ) );
      
      db_name = "No Default Set";
   }
   else
      db_name = DB.at( 0 );

   QLabel* lb_DB = us_banner( tr( "Database: " ) + db_name, -1 );
   lb_DB->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   main->addWidget( lb_DB, row++, 0, 1, 3 );

   QGridLayout* protein = us_radiobutton( tr( "Protein"      ), rb_protein, true );
   QGridLayout* dna     = us_radiobutton( tr( "DNA"          ), rb_dna );
   QGridLayout* rna     = us_radiobutton( tr( "RNA"          ), rb_rna );
   QGridLayout* carb    = us_radiobutton( tr( "Carbohydrate" ), rb_carb );

   QHBoxLayout* radios = new QHBoxLayout;
   radios->addLayout( protein );
   radios->addLayout( dna     );
   radios->addLayout( rna     );
   radios->addLayout( carb    );

   main->addLayout( radios, row++, 0, 1, 3 );

   QButtonGroup* typeButtons = new QButtonGroup( this );
   typeButtons->addButton( rb_protein, US_Analyte::PROTEIN );
   typeButtons->addButton( rb_dna    , US_Analyte::DNA );
   typeButtons->addButton( rb_rna    , US_Analyte::RNA );
   typeButtons->addButton( rb_carb   , US_Analyte::CARBOHYDRATE );
   connect( typeButtons, SIGNAL( buttonClicked    ( int ) ),
                         SLOT  ( set_analyte_type ( int ) ) );

   QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
   connect( pb_investigator, SIGNAL( clicked() ), SLOT( sel_investigator() ) );
   main->addWidget( pb_investigator, row, 0 );

   le_investigator = us_lineedit( tr( "Not Selected" ) );
   le_investigator->setPalette( gray );
   main->addWidget( le_investigator, row++, 1, 1, 2 );

   QBoxLayout* search = new QHBoxLayout;

   // Search
   QLabel* lb_search = us_label( tr( "Search:" ) );
   search->addWidget( lb_search );

   le_search = us_lineedit();
   le_search->setReadOnly( true );
   connect( le_search, SIGNAL( textChanged( const QString& ) ),
                       SLOT  ( search     ( const QString& ) ) );
   search->addWidget( le_search );
   main->addLayout( search, row++, 0 );

   // Analyte descriptions from DB
   QLabel* lb_banner1 = us_banner( tr( "Doubleclick on analyte data to select" ), -1 );
   lb_banner1->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   main->addWidget( lb_banner1, row++, 0 );

   lw_analytes = us_listwidget();
   connect( lw_analytes, SIGNAL( itemDoubleClicked( QListWidgetItem* ) ),
                         SLOT  ( select_analyte   ( QListWidgetItem* ) ) );

   main->addWidget( lw_analytes, row, 0, 5, 1 );
   row += 5;

   // Labels
   QGridLayout* description = new QGridLayout;
   QLabel* lb_description = us_label( tr( "Analyte Description:" ) );
   description->addWidget( lb_description, 0, 0 );

   le_description = us_lineedit( "" ); 
   connect( le_description, SIGNAL( editingFinished   () ), 
                            SLOT  ( change_description() ) );
   description->addWidget( le_description, 0, 1 );
 
   QLabel* lb_guid = us_label( tr( "Global Identifier:" ) );
   description->addWidget( lb_guid, 1, 0 );

   le_guid = us_lineedit( "" ); 
   le_guid->setPalette( gray );
   description->addWidget( le_guid, 1, 1 );
   main->addLayout( description, row, 0, 2, 3 );
 
   if ( US_Settings::us_debug() == 0 )
   {
      lb_guid->setVisible( false );
      le_guid->setVisible( false );
   }

   // Go back to top of 2nd column
   row = 3;

   QButtonGroup* io = new QButtonGroup;

   QGridLayout* db_layout = us_radiobutton( tr( "Use Database" ), rb_db );
   io->addButton( rb_db );
   main->addLayout( db_layout, row, 1 );

   QGridLayout* disk_layout = us_radiobutton( tr( "Use Local Disk" ), rb_disk, true );
   connect( rb_disk, SIGNAL( toggled( bool ) ), SLOT( access_type( bool ) ) );
   io->addButton( rb_disk );
   main->addLayout( disk_layout, row++, 2 );

   QLabel* lb_banner3 = us_banner( tr( "Database/Disk Functions" ), -2 );
   lb_banner3->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   main->addWidget( lb_banner3, row++, 1, 1, 2 );

   QPushButton* pb_load_db = us_pushbutton( 
         tr( "Query Descriptions" ) );
   connect( pb_load_db, SIGNAL( clicked() ), SLOT( read_db() ) );
   main->addWidget( pb_load_db, row, 1 );

   pb_save_db = us_pushbutton( tr( "New Analyte" ), false );
   connect( pb_save_db, SIGNAL( clicked() ), SLOT( save_db() ) );
   main->addWidget( pb_save_db, row++, 2 );

   pb_update_db = us_pushbutton( tr( "Save Analyte" ), false );
   connect( pb_update_db, SIGNAL( clicked() ), SLOT( update_db() ) );
   main->addWidget( pb_update_db, row, 1 );

   pb_del_db = us_pushbutton( tr( "Delete Analyte" ), false );
   connect( pb_del_db, SIGNAL( clicked() ), SLOT( delete_db() ) );
   main->addWidget( pb_del_db, row++, 2 );

   QLabel* lb_banner4 = us_banner( tr( "Other Functions" ), -2 );
   lb_banner4->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   main->addWidget( lb_banner4, row++, 1, 1, 2 );

   pb_sequence = us_pushbutton( tr( "Manage Sequence" ) );
   connect( pb_sequence, SIGNAL( clicked() ), SLOT( manage_sequence() ) );
   main->addWidget( pb_sequence, row, 1 );

   pb_spectrum = us_pushbutton( tr( "Manage Spectrum" ) );
   connect( pb_spectrum, SIGNAL( clicked() ), SLOT( spectrum() ) );
   main->addWidget( pb_spectrum, row++, 2 );

   pb_more = us_pushbutton( tr( "More Info" ), false );
   connect( pb_more, SIGNAL( clicked() ), SLOT( more_info() ) );
   main->addWidget( pb_more, row, 1 );

   cmb_optics = us_comboBox();
   cmb_optics->addItem( tr( "Absorbance"   ) );
   cmb_optics->addItem( tr( "Interference" ) );
   cmb_optics->addItem( tr( "Fluorescence" ) );
   main->addWidget( cmb_optics, row++, 2 );

   row +=2; 

   // Lower half -- protein operations and data
   protein_widget = new QWidget( this );
   
   QGridLayout* protein_info   = new QGridLayout( protein_widget );
   protein_info->setSpacing        ( 2 );
   protein_info->setContentsMargins( 2, 2, 2, 2 );

   int prow = 0;

   QLabel* lb_protein_mw = us_label( tr( "MW <small>(Daltons)</small>:" ) );
   protein_info->addWidget( lb_protein_mw, prow, 0 );

   le_protein_mw = us_lineedit( "" );
   le_protein_mw->setReadOnly( true );
   protein_info->addWidget( le_protein_mw, prow, 1 );

   QLabel* lb_protein_vbar20 = us_label( 
         tr( "VBar <small>(cm<sup>3</sup>/g at 20" ) + DEGC + ")</small>:" );
   protein_info->addWidget( lb_protein_vbar20, prow, 2 );

   le_protein_vbar20 = us_lineedit( "" );
   le_protein_vbar20->setReadOnly( true );
   protein_info->addWidget( le_protein_vbar20, prow++, 3 );

   QLabel* lb_protein_temp = us_label( 
         tr( "Temperature <small>(" ) + DEGC + ")</small>:" );
   protein_info->addWidget( lb_protein_temp, prow, 0 );

   le_protein_temp = us_lineedit( QString::number( temperature, 'f', 1 ) );
   
   if ( signal )
   {
      le_protein_temp->setPalette ( gray );
      le_protein_temp->setReadOnly( true );
   }
   else
   {
      connect( le_protein_temp, SIGNAL( textChanged ( const QString& ) ), 
                                SLOT  ( temp_changed( const QString& ) ) );
   }

   protein_info->addWidget( le_protein_temp, prow, 1 );

   QLabel* lb_protein_vbar = us_label( 
         tr( "VBar <small>(cm<sup>3</sup>/g at T " ) + DEGC + ")</small>:" );
   protein_info->addWidget( lb_protein_vbar, prow, 2 );

   le_protein_vbar = us_lineedit( "" );
   protein_info->addWidget( le_protein_vbar, prow++, 3 );

   QLabel* lb_protein_residues = us_label( tr( "Residue count:" ) );
   protein_info->addWidget( lb_protein_residues, prow, 0 );

   le_protein_residues = us_lineedit( "" );
   le_protein_residues->setReadOnly( true );
   protein_info->addWidget( le_protein_residues, prow, 1 );
   main->addWidget( protein_widget, row, 0, 1, 3 ); 
   
   QSpacerItem* spacer1 = new QSpacerItem( 20, 0 );
   protein_info->addItem( spacer1, prow, 0, 1, 4 );
   protein_info->setRowStretch( prow, 100 );
   
   protein_widget->setVisible( true );
  
   // Lower half -- DNA/RNA operations and data
   dna_widget              = new QWidget( this );
   QGridLayout* dna_layout = new QGridLayout( dna_widget );
   dna_layout->setSpacing        ( 2 );
   dna_layout->setContentsMargins( 2, 2, 2, 2 );

   QPalette p = US_GuiSettings::labelColor();

   QGroupBox*    gb_double = new QGroupBox( tr( "Calculate MW" ) );
   gb_double->setPalette( p );
   QGridLayout*  grid1     = new QGridLayout;   
   grid1->setSpacing        ( 2 );
   grid1->setContentsMargins( 2, 2, 2, 2 );
   
   QGridLayout* box1 = us_checkbox( tr( "Double Stranded" ), cb_stranded, true );
   QGridLayout* box2 = us_checkbox( tr( "Complement Only" ), cb_mw_only );
   grid1->addLayout( box1, 0, 0 );
   grid1->addLayout( box2, 1, 0 );
   connect( cb_stranded, SIGNAL( toggled        ( bool ) ), 
                         SLOT  ( update_stranded( bool ) ) );
   connect( cb_mw_only , SIGNAL( toggled        ( bool ) ), 
                         SLOT  ( update_mw_only ( bool ) ) );
   
   QVBoxLayout* stretch1 = new QVBoxLayout;
   stretch1->addStretch();
   grid1->addLayout( stretch1, 2, 0 );

   gb_double->setLayout( grid1 ); 
   dna_layout->addWidget( gb_double, 0, 0 );


   QGroupBox*    gb_three_prime = new QGroupBox( tr( "Three prime" ) );
   gb_three_prime->setPalette( p );
   QGridLayout*  grid2          = new QGridLayout;   
   grid2->setSpacing        ( 2 );
   grid2->setContentsMargins( 2, 2, 2, 2 );

   QGridLayout* box3 = us_radiobutton( tr( "Hydroxyl" ), rb_3_hydroxyl, true );
   QGridLayout* box4 = us_radiobutton( tr( "Phosphate" ), rb_3_phosphate );

   grid2->addLayout( box3, 0, 0 );
   grid2->addLayout( box4, 0, 1 );
   gb_three_prime->setLayout( grid2 ); 
   connect( rb_3_hydroxyl, SIGNAL( toggled          ( bool ) ), 
                           SLOT  ( update_nucleotide( bool ) ) );

   dna_layout->addWidget( gb_three_prime, 1, 0 );

   QGroupBox*    gb_five_prime = new QGroupBox( tr( "Five prime" ) );
   gb_five_prime->setPalette( p );
   QGridLayout*  grid3         = new QGridLayout;   
   grid3->setSpacing        ( 2 );
   grid3->setContentsMargins( 2, 2, 2, 2 );

   QGridLayout* box5 = us_radiobutton( tr( "Hydroxyl" ), rb_5_hydroxyl, true );
   QGridLayout* box6 = us_radiobutton( tr( "Phosphate" ), rb_5_phosphate );

   grid3->addLayout( box5, 0, 0 );
   grid3->addLayout( box6, 0, 1 );
  
   gb_five_prime->setLayout( grid3 ); 
   connect( rb_5_hydroxyl, SIGNAL( toggled          ( bool ) ), 
                           SLOT  ( update_nucleotide( bool ) ) );

   dna_layout->addWidget( gb_five_prime, 2, 0 );

   QGridLayout* ratios = new QGridLayout;

   QLabel* lb_ratios = us_banner( tr( "Counterion molar ratio/nucletide" ) );
   ratios->addWidget( lb_ratios, 0, 0, 1, 3 );

   QLabel* lb_sodium = us_label( tr( "Sodium, Na+" ) );
   ratios->addWidget( lb_sodium, 1, 0 );

   ct_sodium = us_counter( 2, 0.0, 1.0, 0.0 );
   ct_sodium->setStep( 0.01 );
   connect( ct_sodium, SIGNAL( valueChanged     ( double ) ),
                       SLOT  ( update_nucleotide( double ) ) );
   ratios->addWidget( ct_sodium, 1, 1, 1, 2 );

   QLabel* lb_potassium = us_label( tr( "Potassium, K+" ) );
   ratios->addWidget( lb_potassium, 2, 0 );

   ct_potassium = us_counter( 2, 0.0, 1.0, 0.0 );
   ct_potassium->setStep( 0.01 );
   connect( ct_potassium, SIGNAL( valueChanged     ( double ) ),
                          SLOT  ( update_nucleotide( double ) ) );
   ratios->addWidget( ct_potassium, 2, 1, 1, 2 );

   QLabel* lb_lithium = us_label( tr( "Lithium, Li+" ) );
   ratios->addWidget( lb_lithium, 3, 0 );

   ct_lithium = us_counter( 2, 0.0, 1.0, 0.0 );
   ct_lithium->setStep( 0.01 );
   connect( ct_lithium, SIGNAL( valueChanged     ( double ) ),
                        SLOT  ( update_nucleotide( double ) ) );
   ratios->addWidget( ct_lithium, 3, 1, 1, 2 );

   QLabel* lb_magnesium = us_label( tr( "Magnesium, Mg+" ) );
   ratios->addWidget( lb_magnesium, 4, 0 );

   ct_magnesium = us_counter( 2, 0.0, 1.0, 0.0 );
   ct_magnesium->setStep( 0.01 );
   connect( ct_magnesium, SIGNAL( valueChanged     ( double ) ),
                          SLOT  ( update_nucleotide( double ) ) );
   ratios->addWidget( ct_magnesium, 4, 1, 1, 2 );

   QLabel* lb_calcium = us_label( tr( "Calcium, Ca+" ) );
   ratios->addWidget( lb_calcium, 5, 0 );

   ct_calcium = us_counter( 2, 0.0, 1.0, 0.0 );
   ct_calcium->setStep( 0.01 );
   connect( ct_calcium, SIGNAL( valueChanged     ( double ) ),
                        SLOT  ( update_nucleotide( double ) ) );
   ratios->addWidget( ct_calcium, 5, 1 );

   dna_layout->addLayout( ratios, 0, 1, 4, 2 );

   QGridLayout* nucle_data = new QGridLayout;
   QLabel* lb_nucle_mw = us_label( tr( "MW <small>(Daltons)</small>:" ) );
   nucle_data->addWidget( lb_nucle_mw, 0, 0 );

   le_nucle_mw = us_lineedit( "", -2 );
   le_nucle_mw->setReadOnly( true );
   nucle_data->addWidget( le_nucle_mw, 0, 1, 1, 3 );

   QLabel* lb_nucle_vbar = us_label( 
         tr( "VBar<small>(cm<sup>3</sup>/g)</small>:" ) );
   nucle_data->addWidget( lb_nucle_vbar, 1, 0 );

   le_nucle_vbar = us_lineedit( "" );
   nucle_data->addWidget( le_nucle_vbar, 1, 1, 1, 3 );

   dna_layout->addLayout( nucle_data, 4, 0, 2, 3 );
   main->addWidget( dna_widget, row, 0, 2, 3 ); 
   dna_widget->setVisible( false );

   // Lower half -- carbohydrate operations and data
   carbs_widget = new QWidget( this );
   
   QGridLayout* carbs_info   = new QGridLayout( carbs_widget );
   carbs_info->setSpacing        ( 2 );
   carbs_info->setContentsMargins( 2, 2, 2, 2 );

   QLabel* lb_carbs = us_label( "Under Construction" );
   lb_carbs->setAlignment( Qt::AlignCenter );
   carbs_info->addWidget( lb_carbs, 0, 0 );

   main->addWidget( carbs_widget, row, 0, 1, 3 ); 
   carbs_widget->setVisible( false );

   row += 4;

   // Standard Buttons
   QBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_reset = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );
   buttons->addWidget( pb_reset );

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons->addWidget( pb_help );

   QPushButton* pb_accept;
   
   if ( signal_wanted )
   {
      QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
      connect( pb_cancel, SIGNAL( clicked() ), SLOT( reject() ) );
      buttons->addWidget( pb_cancel );

      pb_accept = us_pushbutton( tr( "Accept" ) );
   }
   else
      pb_accept = us_pushbutton( tr( "Close" ) );

   connect( pb_accept, SIGNAL( clicked() ), SLOT( close() ) );
   buttons->addWidget( pb_accept );

   main->addLayout( buttons, row, 0, 1, 3 );

   analyte_type = US_Analyte::PROTEIN;

   if ( guid.size() == 0 )
      reset();
   else
      load_analyte();
}

void US_AnalyteGui::access_type( bool /* state */ )
{
   reset();
}

void US_AnalyteGui::change_description( void )
{
   int row = lw_analytes->currentRow();
   if ( row < 0 ) return;

   int index = info[ row ].index;

   descriptions[ index ] = le_description->text();
   search( le_search->text() );

   lw_analytes->setCurrentRow( row );

   //TODO  when changing/exiting chack for change
}

void US_AnalyteGui::load_analyte( void )
{
   // At this point we have a guid and whether to load from disk or DB
   if ( db_access ) // Load from DB
   {
// TODO
   }
   else  // Load from disk
   {
      QString path;
      if ( ! analyte_path( path ) ) return;

      filename = get_filename( path, guid );
      QFile f( filename );

      if ( ! f.exists() ) return;
      populate();
   }
}

void US_AnalyteGui::populate( void )
{
   QFile file( filename );

   // Read in the filename and populate screen
   if ( ! file.open( QIODevice::ReadOnly | QIODevice::Text) )
   {
      // Fail quietly
      qDebug() << "Cannot open file " << filename;
      return;
   }

   extinction  .clear();
   refraction  .clear();
   fluorescence.clear();

   double                 freq;
   double                 value;
   QMap< double, double > map;
   bool                   b;
   QString                type;

   QXmlStreamReader xml( &file );
   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "analyte" )
         {
            QXmlStreamAttributes a    = xml.attributes();
            
            type = a.value( "type" ).toString();

            le_description->setText( a.value( "description" ).toString() );
            le_guid       ->setText( a.value( "guid"        ).toString() );

            if ( type == "PROTEIN" )
            {
               rb_protein->setChecked( true );
               le_protein_vbar->setText( a.value( "vbar"        ).toString() );
            }

            else if ( type == "DNA"  ||  type == "RNA" )
            {
               ( type == "RNA" ) ? rb_rna->setChecked( true ) 
                                 : rb_dna->setChecked( true );

               b = ( a.value( "stranded" ).toString() == "T" ); 
               cb_stranded->setChecked( b );

               b = ( a.value( "complement_only" ).toString() == "T" );
               cb_mw_only->setChecked( b );

               b = ( a.value( "ThreePrimeHydroxyl" ).toString() == "T" );
               b ? rb_3_hydroxyl->setChecked( true ) 
                 : rb_3_phosphate->setChecked( true );

               b = ( a.value( "FivePrimeHydroxyl" ).toString() == "T" );
               b ? rb_5_hydroxyl->setChecked( true ) 
                 : rb_5_phosphate->setChecked( true );

               ct_sodium   ->setValue( 
                     a.value( "sodium"    ).toString().toDouble() );
               
               ct_potassium->setValue( 
                     a.value( "potassium" ).toString().toDouble() );
               
               ct_lithium  ->setValue( 
                     a.value( "lithium"   ).toString().toDouble() );
               
               ct_magnesium->setValue( 
                     a.value( "magnesium" ).toString().toDouble() );
               
               ct_calcium  ->setValue( 
                     a.value( "calcium"   ).toString().toDouble() );

               le_nucle_vbar->setText( a.value( "vbar" ).toString() );
            }

            else // CARB
            {
               rb_carb->setChecked( true );
            }
         }

         else if ( xml.name() == "sequence" )
         {
            sequence = xml.readElementText();
            
            if ( type == "DNA"  ||  type == "RNA" ) 
            {
               update_nucleotide();
               le_nucle_vbar->setText( QString::number( vbar, 'f', 4 ) );
            }

            else if ( type == "PROTEIN" )
            {
               US_Math::Peptide p;
               double temperature = le_protein_temp->text().toDouble();
               US_Math::calc_vbar( p, sequence, temperature );

               le_protein_mw      ->setText( QString::number( (int) p.mw ) );
               le_protein_vbar20  ->setText( QString::number( p.vbar20, 'f', 4 ) );
               le_protein_vbar    ->setText( QString::number( p.vbar  , 'f', 4 ) );
               le_protein_residues->setText( QString::number( p.residues ) );

               // The sequence tag comes before the extinction extinction tag
               // so a value set there will override this setting, if it
               // exists.  It's not the way xml is really supposed work, but it
               // will be ok in this case.

               extinction[ 280.0 ] = p.e280; 
            } 
         }

         else if ( xml.name() == "extinction" )
         {
            QXmlStreamAttributes a = xml.attributes();
            freq  = a.value( "frequency" ).toString().toDouble();
            value = a.value( "value"     ).toString().toDouble();
            extinction[ freq ] = value;
         }

         else if ( xml.name() == "refraction" )
         {
            QXmlStreamAttributes a = xml.attributes();
            freq  = a.value( "frequency" ).toString().toDouble();
            value = a.value( "value"     ).toString().toDouble();
            refraction[ freq ] = value;
         }
         
         else if ( xml.name() == "fluorescence" )
         {
            QXmlStreamAttributes a = xml.attributes();
            freq  = a.value( "frequency" ).toString().toDouble();
            value = a.value( "value"     ).toString().toDouble();
            fluorescence[ freq ] = value;
         }
      }
   }

   file.close();
}

void US_AnalyteGui::set_analyte_type( int type )
{
   switch ( type )
   {
      case US_Analyte::PROTEIN:
         dna_widget    ->setVisible( false );
         carbs_widget  ->setVisible( false );
         protein_widget->setVisible( true );
         analyte_type = US_Analyte::PROTEIN;
         resize( 0, 0 ); // Resize to minimum dimensions
         break;

      case US_Analyte::DNA:
      case US_Analyte::RNA:
         protein_widget->setVisible( false ); 
         carbs_widget  ->setVisible( false );
         dna_widget    ->setVisible( true );
         analyte_type = ( type == US_Analyte::DNA )? US_Analyte::DNA 
                                                   : US_Analyte::RNA;
         break;

      case US_Analyte::CARBOHYDRATE:
         protein_widget->setVisible( false ); 
         dna_widget    ->setVisible( false );
         carbs_widget  ->setVisible( true );
         analyte_type = US_Analyte::CARBOHYDRATE;
         resize( 0, 0 ); // Resize to minimum dimensions
         break;
   }

   reset();
}
void US_AnalyteGui::close( void )
{
   // Emit signal if requested
   if ( signal_wanted ) 
   {
      if ( ! data_ok() ) return;
      
      if ( le_guid->text().size() != 36 )
      {
         QMessageBox::StandardButton b = QMessageBox::question( this,
               tr( "GUID Missing" ),
               tr( "Are you sure?\n\n" 
                   "The Global Identifier (GUID) is missing.\n"
                   "You will not be able to easily reproduce this analyte.\n\n"
                   "To set the GUID, save the analyte to the disk or DB." ), 
               QMessageBox::Yes | QMessageBox::No );

         if ( b == QMessageBox::No ) return;
      }

      US_Analyte data;

      data.extinction   = extinction;
      data.refraction   = refraction;
      data.fluorescence = fluorescence;
      data.description  = le_description->text();
      data.guid         = le_guid       ->text();
      data.type         = analyte_type;

      switch ( analyte_type )
      {
         case US_Analyte::PROTEIN:
         {
            US_Math::Peptide p;
            US_Math::calc_vbar( p, sequence, 20.0 );  // Always evaluate at 20C
            data.mw   = p.mw;
            data.vbar = p.vbar;
            break;
         }

         case US_Analyte::DNA:
         case US_Analyte::RNA:
            data.mw   = le_nucle_mw  ->text().toDouble();
            data.vbar = le_nucle_vbar->text().toDouble();
            break;

         case US_Analyte::CARBOHYDRATE:
            data.mw   = 0.0;
            data.vbar = 0.0;
            break;
      }

      emit valueChanged( data );
   }

   accept();
}

void US_AnalyteGui::reset( void )
{
   inReset = true;
   lw_analytes->clear();

   analyteID.clear();

   le_investigator->setText( tr( "Not Selected" ) );

   if ( personID > 0 )
   {
      QString lname;
      QString fname;

      if ( US_Investigator::get_person_info( personID, lname, fname ) )
         assign_investigator( personID, lname, fname );
   }

   le_search->clear();
   le_search->setReadOnly( true );

   vbar = 0.0;

   filename    .clear();
   filenames   .clear();
   GUIDs       .clear();
   sequence    .clear();
   extinction  .clear();
   refraction  .clear();
   fluorescence.clear();

   le_guid            ->clear();
   le_description     ->clear();
   le_protein_mw      ->clear();
   le_protein_vbar20  ->clear();
   le_protein_vbar    ->clear();
   le_protein_temp    ->setText( "20.0" );
   le_protein_residues->clear();

   ct_sodium          ->setValue( 0.0 );
   ct_potassium       ->setValue( 0.0 );
   ct_lithium         ->setValue( 0.0 );
   ct_magnesium       ->setValue( 0.0 );
   ct_calcium         ->setValue( 0.0 );
                      
   cb_stranded        ->setChecked( true );
   cb_mw_only         ->setChecked( false );
                      
   rb_3_hydroxyl      ->setChecked( true );
   rb_5_hydroxyl      ->setChecked( true );
                      
   le_nucle_mw        ->clear();
   le_nucle_vbar      ->clear();
                      
   //pb_save            ->setEnabled( false );
   pb_save_db         ->setEnabled( false );
   pb_update_db       ->setEnabled( false );
   pb_del_db          ->setEnabled( false );
   pb_more            ->setEnabled( false );

   qApp->processEvents();
   inReset = false;
}

void US_AnalyteGui::temp_changed( const QString& text )
{
   double temperature = text.toDouble();

   if ( ! sequence.isEmpty() )
   {
      US_Math::Peptide p;
      US_Math::calc_vbar( p, sequence, temperature );
      le_protein_vbar->setText( QString::number( p.vbar, 'f', 4 ) );
   }
}

bool US_AnalyteGui::analyte_path( QString& path )
{
   QDir dir;
   path = US_Settings::dataDir() + "/analytes";

   if ( ! dir.exists( path ) )
   {
      if ( ! dir.mkpath( path ) )
      {
         QMessageBox::critical( this,
               tr( "Bad Analyte Path" ),
               tr( "Could not create default directory for analytes\n" )
               + path );
         return false;
      }
   }

   return true;
}

void US_AnalyteGui::read_analyte( void )
{
   QString path;
   if ( ! analyte_path( path ) ) return;

   filenames   .clear();
   descriptions.clear();
   db_access = false;

   QDir f( path );
   QStringList filter( "A*.xml" );
   QStringList f_names = f.entryList( filter, QDir::Files, QDir::Name );

   QString type;
   if      ( rb_protein->isChecked() ) type = "PROTEIN";
   else if ( rb_dna    ->isChecked() ) type = "DNA";
   else if ( rb_rna    ->isChecked() ) type = "RNA";
   else                                type = "CARB";

   for ( int i = 0; i < f_names.size(); i++ )
   {
      QFile a_file( path + "/" + f_names[ i ] );

      if ( ! a_file.open( QIODevice::ReadOnly | QIODevice::Text) ) continue;

      QXmlStreamReader xml( &a_file );

      while ( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement() )
         {
            if ( xml.name() == "analyte" )
            {
               QXmlStreamAttributes a = xml.attributes();

               if ( a.value( "type" ).toString() == type )
               {
                  descriptions << a.value( "description" ).toString();
                  filenames    << path + "/" + f_names[ i ];
               }
               break;
            }
         }
      }

      a_file.close();
   }

   lw_analytes->clear();

   for ( int i = 0; i < descriptions.size(); i++ )
   {
      AnalyteInfo ai;
      ai.description = descriptions[ i ];
      ai.filename    =  filenames  [ i ];
   }
      
   if ( descriptions.size() == 0 )
   {
      lw_analytes->addItem( "No analyte files found." );
      le_search->setText( "" );
      le_search->setReadOnly( true );
   }
   else
   {
      le_search->clear();
      le_search->setReadOnly( false );
      
      search( "" );
   }
}

void US_AnalyteGui::parse_dna( void )
{
   A = sequence.count( "a" );
   C = sequence.count( "c" );
   G = sequence.count( "g" );
   T = sequence.count( "t" );
   U = sequence.count( "u" );
}

void US_AnalyteGui::manage_sequence( void )
{

   US_SequenceEditor* edit = new US_SequenceEditor( sequence );
   connect( edit, SIGNAL( sequenceChanged( QString ) ), 
                  SLOT  ( update_sequence( QString ) ) );
   edit->exec();
}

void US_AnalyteGui::update_sequence( QString seq )
{
   seq = seq.toLower().remove( QRegExp( "\\s" ) );
   QString check = seq;

   if ( seq == sequence ) return;

   switch ( analyte_type )
   {
      case US_Analyte::PROTEIN:
         seq.remove( QRegExp( "[^a-z\\+\\-\\?\\@]" ) );
         break;

      case US_Analyte::DNA:
      case US_Analyte::RNA:
         seq.remove( QRegExp( "[^acgtu]" ) );
         break;

      case US_Analyte::CARBOHYDRATE:
         break;
   }

   if ( check != seq )
   {
      QMessageBox::information( this,
         tr( "Attention" ), 
         tr( "There are invalid characters in the sequence!\n"
             "Please fix this error and try again..." ) );
      return;
   }

   sequence = seq;

   switch ( analyte_type )
   {
      case US_Analyte::PROTEIN:
         {
         US_Math::Peptide p;
         double temperature = le_protein_temp->text().toDouble();
         US_Math::calc_vbar( p, sequence, temperature );

         le_protein_mw      ->setText( QString::number( (int) p.mw ) );
         le_protein_vbar20  ->setText( QString::number( p.vbar20, 'f', 4 ) );
         le_protein_vbar    ->setText( QString::number( p.vbar  , 'f', 4 ) );
         le_protein_residues->setText( QString::number( p.residues ) );
         }
         break;

      case US_Analyte::DNA:
      case US_Analyte::RNA:
         parse_dna();
         update_nucleotide();
         break;

      case US_Analyte::CARBOHYDRATE:
         break;
   }

   pb_save_db->setEnabled( true );
   pb_more   ->setEnabled( true );
}

void US_AnalyteGui::update_stranded( bool checked )
{
   if ( inReset ) return;
   if ( checked ) cb_mw_only->setChecked( false );
   update_nucleotide();
}

void US_AnalyteGui::update_mw_only( bool checked )
{
   if ( inReset ) return;
   if ( checked ) cb_stranded->setChecked( false );
   update_nucleotide();
}

void US_AnalyteGui::update_nucleotide( bool /* value */ )
{
   if ( inReset ) return;
   update_nucleotide();
}

void US_AnalyteGui::update_nucleotide( double /* value */ )
{
   if ( inReset ) return;
   update_nucleotide();
}

void US_AnalyteGui::update_nucleotide( void )
{
   bool isDNA          = rb_dna       ->isChecked();
   bool doubleStranded = cb_stranded  ->isChecked();
   bool complement     = cb_mw_only   ->isChecked();
   bool _3prime        = rb_3_hydroxyl->isChecked();
   bool _5prime        = rb_5_hydroxyl->isChecked();

   double MW = 0;
   uint   total = A + G + C + T + U;
   
   if ( doubleStranded ) total *= 2;
   
   const double mw_A = 313.209;
   const double mw_C = 289.184;
   const double mw_G = 329.208;
   const double mw_T = 304.196;
   const double mw_U = 274.170;
   
   if ( isDNA )
   {
      if ( doubleStranded )
      {
         MW += A * mw_A;
         MW += G * mw_G;
         MW += C * mw_C;
         MW += T * mw_T;
         MW += A * mw_T;
         MW += G * mw_C;
         MW += C * mw_G;
         MW += T * mw_A;
      }

      if ( complement )
      {
         MW += A * mw_T;
         MW += G * mw_C;
         MW += C * mw_G;
         MW += T * mw_A;
      }

      if ( ! complement && ! doubleStranded )
      {
         MW += A * mw_A;
         MW += G * mw_G;
         MW += C * mw_C;
         MW += T * mw_T;
      }
   }
   else /* RNA */
   {
      if ( doubleStranded )
      {
         MW += A * ( mw_A + 15.999 );
         MW += G * ( mw_G + 15.999 );
         MW += C * ( mw_C + 15.999 );
         MW += U * ( mw_U + 15.999 );
         MW += A * ( mw_U + 15.999 );
         MW += G * ( mw_C + 15.999 );
         MW += C * ( mw_G + 15.999 );
         MW += U * ( mw_A + 15.999 );
      }

      if ( complement )
      {
         MW += A * ( mw_U + 15.999 );
         MW += G * ( mw_C + 15.999 );
         MW += C * ( mw_G + 15.999 );
         MW += U * ( mw_A + 15.999 );
      }

      if ( ! complement && ! doubleStranded )
      {
         MW += A * ( mw_A + 15.999 );
         MW += G * ( mw_G + 15.999 );
         MW += C * ( mw_C + 15.999 );
         MW += U * ( mw_U + 15.999 );
      }
   }
   
   MW += ct_sodium   ->value() * total * 22.99;
   MW += ct_potassium->value() * total * 39.1;
   MW += ct_lithium  ->value() * total * 6.94;
   MW += ct_magnesium->value() * total * 24.305;
   MW += ct_calcium  ->value() * total * 40.08;
   
   if ( _3prime )
   {
      MW += 17.01;
      if ( doubleStranded )  MW += 17.01; 
   }
   else // we have phosphate
   {
      MW += 94.87;
      if ( doubleStranded ) MW += 94.87;
   }

   if ( _5prime )
   {
      MW -=  77.96;
      if ( doubleStranded )  MW -= 77.96; 
   }

   QString s;

   if ( doubleStranded )
   {
      s.sprintf(" %2.5e kD (%d A, %d G, %d C, %d U, %d T, %d bp)",
            MW / 1000.0, A, G, C, U, T, total / 2);
   }
   else
   {
     s.sprintf(" %2.5e kD (%d A, %d G, %d C, %d U, %d T, %d bases)",
            MW / 1000.0, A, G, C, U, T, total );
   }
   
   le_nucle_mw->setText( s );

   if ( rb_dna->isChecked() )
   {

      if ( MW > 0 && T == 0 && U > 0)
         QMessageBox::question( this, 
            tr( "Attention:" ),
            tr( "Are you sure?\n"
                "There don't appear to be any thymine residues present,\n"
                "instead there are uracil residues in this sequence." ) );
   }
   else // DNA */
   {
      if ( MW > 0 && T > 0 && U == 0 )
         QMessageBox::question( this,
            tr( "Attention:" ),
            tr( "Are you sure?\n"
                "There don't appear to be any uracil residues present,\n"
                "instead there are thymine residues in this sequence." ) );
   }
}

QString US_AnalyteGui::get_filename( const QString& path, const QString& guid )
{
   QDir f( path );
   QStringList filter( "A???????.xml" );
   QStringList f_names = f.entryList( filter, QDir::Files, QDir::Name );
  
   for ( int i = 0; i < f_names.size(); i++ )
   {
      QFile a_file( path + "/" + f_names[ i ] );

      if ( ! a_file.open( QIODevice::ReadOnly | QIODevice::Text) ) continue;

      QXmlStreamReader xml( &a_file );

      while ( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement() )
         {
            if ( xml.name() == "analyte" )
            {
               QXmlStreamAttributes a = xml.attributes();

               if ( a.value( "guid" ).toString() == guid )
               {
                  newFile = false;
                  return path + "/" + f_names[ i ];
               }
            }
         }
      }

      a_file.close();
   }

   int number = ( f_names.size() > 0 ) ? f_names.last().mid( 1, 7 ).toInt() : 0;
   newFile    = true;

   return path + "/A" + QString().sprintf( "%07i", number + 1 ) + ".xml";
}

QString US_AnalyteGui::new_guid( void )
{
   uchar c[ 16 ];  // Binary
   char  u[ 37 ];  // String
   uuid_generate( c );
   uuid_unparse ( c, u );
   return QString( u );
}

void US_AnalyteGui::save_analyte( void )
{
   if ( ! data_ok() ) return;

   QString path;
   if ( ! analyte_path( path ) ) return;

   // If guid matches one we already have, use that filename
   // otherwise create a new filename.

   // If guid is null, generate a new one.
   if ( le_guid->text().size() != 36 )
      le_guid->setText( new_guid() );

   // Get filename
   QString fn = get_filename( path, le_guid->text() );
   QFile   file( fn );

   if ( ! file.open( QIODevice::WriteOnly | QIODevice::Text) )
   {
       qDebug() << "Cannot open file for writing: " << fn;
       return;
   }

   QXmlStreamWriter xml( &file );
   xml.setAutoFormatting( true );

   xml.writeStartDocument();
   xml.writeDTD         ( "<!DOCTYPE US_Analyte>" );
   xml.writeStartElement( "AnalyteData" );
   xml.writeAttribute   ( "version", "1.0" );

   xml.writeStartElement( "analyte" );

   // Set attributes depending on type
   QString b;

   switch ( analyte_type )
   {
      case US_Analyte::PROTEIN:
      {
         US_Math::Peptide p;
         double temperature = le_protein_temp->text().toDouble();
         US_Math::calc_vbar( p, sequence, temperature );

         xml.writeAttribute( "type",        "PROTEIN" );
         xml.writeAttribute( "vbar20", QString::number(  p.vbar20, 'e', 4 ) );
      }
         break;

      case US_Analyte::DNA:
      case US_Analyte::RNA:
         if ( analyte_type == US_Analyte::DNA )
            xml.writeAttribute( "type", "DNA" );
         else
            xml.writeAttribute( "type", "RNA" );

         b = ( cb_stranded->isChecked() ) ? "T" : "F";
         xml.writeAttribute( "stranded", b );
         
         b = ( cb_mw_only->isChecked() ) ? "T" : "F";
         xml.writeAttribute( "complement_only", b );
         
         b = ( rb_3_hydroxyl->isChecked() ) ? "T" : "F";
         xml.writeAttribute( "ThreePrimeHydroxyl", b );
         
         b = ( rb_5_hydroxyl->isChecked() ) ? "T" : "F";
         xml.writeAttribute( "FivePrimeHydroxyl", b );

         xml.writeAttribute( "sodium",    
               QString::number( ct_sodium    ->value(), 'f', 2 ) );
         
         xml.writeAttribute( "potassium", 
               QString::number( ct_potassium ->value(), 'f', 2 ) );
         
         xml.writeAttribute( "lithium",   
               QString::number( ct_lithium   ->value(), 'f', 2 ) );
         
         xml.writeAttribute( "magnesium", 
               QString::number( ct_magnesium ->value(), 'f', 2 ) );
         
         xml.writeAttribute( "calcium",   
               QString::number( ct_calcium   ->value(), 'f', 2 ) );
         
         xml.writeAttribute( "vbar",      le_nucle_vbar->text() );
         break;

      case US_Analyte::CARBOHYDRATE:
         xml.writeAttribute( "type", "CARB" );
         break;
   }

   xml.writeAttribute( "description", description );
   xml.writeAttribute( "guid"       , le_guid->text() );

   xml.writeStartElement( "sequence" );
   xml.writeCharacters( "\n" );

   for ( int i = 0; i < sequence.length() / 80; i++ )
      xml.writeCharacters( sequence.mid( i * 80, 80 ) + "\n" );

   if ( sequence.length() % 80 > 0 )
       xml.writeCharacters( sequence.mid( ( sequence.length() / 80 ) * 80 ) );

   xml.writeCharacters( "\n" );
   xml.writeEndElement(); // sequence

   // Add extinction values
   QList< double > keys = extinction.keys();
   double          freq;
   double          value;

   for ( int i = 0; i < keys.size(); i++ )
   {
      freq  =             keys[ i ];
      value = extinction[ keys[ i ] ];

      xml.writeStartElement( "extinction" );
      xml.writeAttribute( "frequency",  QString::number( freq , 'f', 1 ) );
      xml.writeAttribute( "value",      QString::number( value, 'f', 4 ) );
      xml.writeEndElement(); // extinction
   }

   // Add refraction values
   keys = refraction.keys();

   for ( int i = 0; i < keys.size(); i++ )
   {
      freq  =             keys[ i ];
      value = refraction[ keys[ i ] ];

      xml.writeStartElement( "refraction" );
      xml.writeAttribute( "frequency",  QString::number( freq , 'f', 1 ) );
      xml.writeAttribute( "value",      QString::number( value, 'f', 4 ) );
      xml.writeEndElement(); // refraction
   }

   // Add fluorescence values
   keys = fluorescence.keys();

   for ( int i = 0; i < keys.size(); i++ )
   {
      freq  =             keys[ i ];
      value = fluorescence[ keys[ i ] ];

      xml.writeStartElement( "fluorescence" );
      xml.writeAttribute( "frequency",  QString::number( freq , 'f', 1 ) );
      xml.writeAttribute( "value",      QString::number( value, 'f', 4 ) );
      xml.writeEndElement(); // fluorescence
   }

   xml.writeEndElement(); // analyte
   xml.writeEndDocument();
   file.close();

   QString s = ( newFile ) ? tr( "saved" ) : tr( "updated" );
   QMessageBox::information( this, 
         tr( "Save results" ),
         tr( "Analyte " ) + s );
}

void US_AnalyteGui::more_info( void )
{
   US_Math::Peptide p;
   double temperature =  le_protein_temp->text().toDouble();
   US_Math::calc_vbar( p, sequence, temperature );

   QString s;
   QString s1 =
             "***************************************************\n"     +
         tr( "*            Analyte Analysis Results             *\n" )   +
             "***************************************************\n\n\n" +
         tr( "Report for:           " ) + description + "\n\n" +

         tr( "Number of Residues:   " ) + s.sprintf( "%i", p.residues ) + " AA\n";
   s1 += tr( "Molecular Weight:     " ) + s.sprintf( "%i", (int)p.mw )  +
         tr( " Dalton\n" ) +
         
         tr( "V-bar at 20 " ) + DEGC + ":    " + 
              QString::number( p.vbar20, 'f', 6 )   + tr( " cm^3/g\n" ) +
         
         tr( "V-bar at " ) + QString::number( temperature, 'f', 2 ) + " " + DEGC + ": " +
               QString::number( p.vbar, 'f', 6 ) + tr( " cm^3/g\n\n" ) +
       
         tr( "Extinction coefficients for the denatured analyte:\n" 
             "  Wavelength (nm)   OD/(mol cm)\n" );

   QList< double > keys = extinction.keys();
   qSort( keys );

   for ( int i = 0; i < keys.size(); i++ )
   {
      QString s;
      s.sprintf( "  %4.1f          %9.4f\n", 
            keys[ i ], extinction[ keys[ i ] ] );
      s1 += s;
   }
        
   s1 += tr( "\nComposition: \n\n" );
   s1 += tr( "Alanine:        " )  + s.sprintf( "%3i", p.a ) + "  ";
   s1 += tr( "Arginine:       " )  + s.sprintf( "%3i", p.r ) + "\n";
         
   s1 += tr( "Asparagine:     " )  + s.sprintf( "%3i", p.n ) + "  ";
   s1 += tr( "Aspartate:      " )  + s.sprintf( "%3i", p.d ) + "\n";
         
   s1 += tr( "Asparagine or \n" )  +
         tr( "Aspartate:      " )  + s.sprintf( "%3i",p.b ) + "\n";
         
   s1 += tr( "Cysteine:       " )  + s.sprintf( "%3i",p.c ) + "  ";
   s1 += tr( "Glutamate:      " )  + s.sprintf( "%3i", p.e ) + "\n";
         
   s1 += tr( "Glutamine:      " )  + s.sprintf( "%3i", p.q ) + "  ";
   s1 += tr( "Glycine:        " )  + s.sprintf( "%3i", p.g ) + "\n";
        
   s1 += tr( "Glutamine or  \n" )  +  
         tr( "Glutamate:      " )  + s.sprintf( "%3i", p.z ) + "\n";
         
   s1 += tr( "Histidine:      " )  + s.sprintf( "%3i", p.h ) + "  ";
   s1 += tr( "Isoleucine:     " )  + s.sprintf( "%3i", p.i ) + "\n";
         
   s1 += tr( "Leucine:        " )  + s.sprintf( "%3i", p.l ) + "  ";
   s1 += tr( "Lysine:         " )  + s.sprintf( "%3i", p.k ) + "\n";
         
   s1 += tr( "Methionine:     " )  + s.sprintf( "%3i", p.m ) + "  ";
   s1 += tr( "Phenylalanine:  " )  + s.sprintf( "%3i", p.f ) + "\n";
         
   s1 += tr( "Proline:        " )  + s.sprintf( "%3i", p.p ) + "  ";
   s1 += tr( "Serine:         " )  + s.sprintf( "%3i", p.s ) + "\n";
         
   s1 += tr( "Threonine:      " )  + s.sprintf( "%3i", p.t ) + "  ";
   s1 += tr( "Tryptophan:     " )  + s.sprintf( "%3i", p.w ) + "\n";
         
   s1 += tr( "Tyrosine:       " )  + s.sprintf( "%3i", p.y ) + "  ";
   s1 += tr( "Valine:         " )  + s.sprintf( "%3i", p.v ) + "\n";
         
   s1 += tr( "Unknown:        " )  + s.sprintf( "%3i", p.x ) + "  ";
   s1 += tr( "Hao:            " )  + s.sprintf( "%3i", p.j ) + "\n";
         
   s1 += tr( "Delta-linked Ornithine:" ) + QString::number( p.o ) + "\n";
        
   US_EditorGui* dialog = new US_EditorGui();
   dialog->editor->e->setFont(  QFont( "monospace", US_GuiSettings::fontSize() ) );
   dialog->editor->e->setText( s1 );
   dialog->exec();
}

void US_AnalyteGui::search( const QString& text )
{
   lw_analytes->clear();
   info.clear();

   for ( int i = 0; i < descriptions.size(); i++ )
   {
      if ( descriptions[ i ].contains( 
             QRegExp( ".*" + text + ".*", Qt::CaseInsensitive ) ) )
      {
         AnalyteInfo ai;
         ai.description = descriptions[ i ];
         ai.filename    = filenames   [ i ];
         ai.index       = i;
         info << ai;

         lw_analytes->addItem( 
               new QListWidgetItem( descriptions[ i ], lw_analytes ) );
      }
   }
}

void US_AnalyteGui::sel_investigator( void )
{
   reset();
   
   US_Investigator* inv_dialog = new US_Investigator( true );
   
   connect( inv_dialog,
      SIGNAL( investigator_accepted( int, const QString&, const QString& ) ),
      SLOT  ( assign_investigator  ( int, const QString&, const QString& ) ) );
   
   inv_dialog->exec();
}

void US_AnalyteGui::assign_investigator( int invID,
      const QString& lname, const QString& fname)
{
   le_investigator->setText( "InvID (" + QString::number( invID ) + "): " +
         lname + ", " + fname );

   personID = invID;
   if ( personID > 0 ) read_db();
}

void US_AnalyteGui::spectrum( void )
{
   QString   spectrum_type = cmb_optics->currentText();
   US_Table* dialog;
   bool      changed = false; //TODO Make a class variable

   if ( spectrum_type == tr( "Absorbance" ) )
     dialog = new US_Table( extinction, tr( "Extinction:" ), changed );
   else if ( spectrum_type == tr( "Interference" ) )
     dialog = new US_Table( refraction, tr( "Extinction:" ), changed );
   else 
     dialog = new US_Table( fluorescence, tr( "Extinction:" ), changed );;

   dialog->setWindowTitle( tr( "Manage %1 Values" ).arg( spectrum_type ) );

   dialog->exec();
}

void US_AnalyteGui::connect_error( const QString& error )
{
   QMessageBox::warning( this, tr( "Connection Problem" ),
      tr( "Could not connect to databasee\n" ) + error );
}

void US_AnalyteGui::read_db( void )
{
   if ( rb_disk->isChecked() )
   {
      read_analyte();
      return;
   }

   if ( personID < 0 )
   {
      QMessageBox::information( this,
            tr( "Investigator not set" ),
            tr( "Please select an investigator first." ) );
      return;
   }

   db_access = true;

   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   pb_save_db  ->setEnabled( false );
   pb_update_db->setEnabled( false );
   pb_del_db   ->setEnabled( false );
   pb_more     ->setEnabled( false );

   le_search->setText( "" );
   le_search->setReadOnly( true );

   lw_analytes->clear();

   QStringList q( "get_analyte_desc" );
   q << QString::number( personID );

   db.query( q );

   if ( db.lastErrno() != 0 )
   {
      QMessageBox::information( this,
            tr( "Database Error" ),
            tr( "The following errro was returned:\n" ) + db.lastError() );
      return; 
   }

   info.clear();

   while ( db.next() )
   {
      AnalyteInfo current;
      current.analyteID   = db.value( 0 ).toString();
      current.description = db.value( 1 ).toString();

      QString a_type      = db.value( 2 ).toString();

           if ( a_type == "Protein" ) current.type = US_Analyte::PROTEIN;
      else if ( a_type == "RNA"     ) current.type = US_Analyte::RNA;
      else if ( a_type == "DNA"     ) current.type = US_Analyte::DNA;
      else                            current.type = US_Analyte::CARBOHYDRATE;

      if ( current.type == analyte_type ) info << current;
   }

   for ( int i = 0; i < info.size(); i++ )
   {
      lw_analytes->addItem( new QListWidgetItem(
         info[ i ].analyteID + ": " + info[ i ].description, lw_analytes ) );
   }

   if ( info.size() == 0 )
   {
      QMessageBox::information( this,
            tr( "Attention" ),
            tr( "No analyte data found for the selected investigator,\n"
                "You can 'Reset' then query DB to list all \n"
                "authorized analyte files." ) );
   }
   else
      le_search->setReadOnly( false );
}

bool US_AnalyteGui::database_ok( US_DB2& db )
{
   if ( db.lastErrno() == 0 ) return true;

   QMessageBox::information( this,
      tr( "Database Error" ),
      tr( "The following error was returned:\n" ) + db.lastError() );

   return false; 
}

bool US_AnalyteGui::data_ok( void )
{
   // Check to see if a sequence is entered
   if ( sequence.isEmpty() )
   {
      QMessageBox::information( this,
         tr( "Attention" ), 
         tr( "A sequence must be defined before saving." ) );
      return false;
   }
   
   description = le_description->text().remove( '|' );

   if ( description.isEmpty() )
   {
      QMessageBox::information( this,
         tr( "Attention" ), 
         tr( "A description must be entered before saving." ) );
      return false;
   }

   double vbar = le_protein_vbar->text().toDouble();
   if ( analyte_type == US_Analyte::PROTEIN  &&  ( vbar < 0.0  || vbar > 2.0 ) )
   {
      QMessageBox::information( this,
         tr( "Attention" ), 
         tr( "The vbar entry is not a resonable value." ) );
      return false;
   }

   return true;
}

void US_AnalyteGui::status_query( const QStringList& q )
{
   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   db.statusQuery( q );

   if ( database_ok( db ) ) return;
   if ( db.lastErrno() != 0 )
   {
      QMessageBox::information( this,
            tr( "Database Error" ),
            tr( "The following errro was returned:\n" ) + db.lastError() );
      return; 
   }
}

void US_AnalyteGui::select_analyte( QListWidgetItem* item )
{
   if ( ! db_access )  // Read from disk
   {
      pb_save_db  ->setEnabled( false );
      pb_update_db->setEnabled( false );
      pb_del_db   ->setEnabled( false );
      pb_more     ->setEnabled( false );

      int index = lw_analytes->currentRow();
      if ( index < 0 ) return;

      filename = info[ index ].filename;
      populate();

      pb_del_db   ->setEnabled( true );
      pb_update_db->setEnabled( true );
      pb_save_db  ->setEnabled( true );
      pb_more     ->setEnabled( true );

      return;
   }

   // Get data from DB
   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   QStringList q( "get_analyte_info" );
   
   // Get analyteID
   analyteID = item->text().split( ":" )[ 0 ];

   q << analyteID;

   db.query( q );

   if ( ! database_ok( db ) ) return;

   db.next();

   guid             = db.value( 0 ).toString();
   // We know the type : analyte_t = db.value( 1 ).toAnalyte();
   sequence         = db.value( 2 ).toString().toLower();
   vbar             = db.value( 3 ).toDouble();
   description      = db.value( 4 ).toString();
   // We know the spectrum string db.value( 5 ).toString();
   double  mw       = db.value( 6 ).toDouble();
   personID         = db.value( 7 ).toInt();
   
   filename = "";

   q.clear();
   q << "get_spectrum" << analyteID << "Analyte" << "'Extinction";

   extinction.clear();
   db.query( q );

   while ( db.next() )
   {
      double lambda = db.value( 1 ).toDouble();
      double coeff  = db.value( 2 ).toDouble();
      extinction[ lambda ] = coeff;
   }

   q[ 3 ] = "Refraction";
   refraction.clear();
   db.query( q );

   while ( db.next() )
   {
      double lambda = db.value( 1 ).toDouble();
      double coeff  = db.value( 2 ).toDouble();
      refraction[ lambda ] = coeff;
   }

   q[ 3 ] = "Fluorescence";
   fluorescence.clear();
   db.query( q );

   while ( db.next() )
   {
      double lambda = db.value( 1 ).toDouble();
      double coeff  = db.value( 2 ).toDouble();
      fluorescence[ lambda ] = coeff;
   }

   le_description->setText( description );
   le_guid       ->setText( guid );

   // Update the window
   switch ( analyte_type )
   {
      case US_Analyte::PROTEIN:
         {
            US_Math::Peptide p;
            US_Math::calc_vbar( p, sequence, 20.0 );

            mw = p.mw;
            le_protein_mw      ->setText( QString::number( (int) mw ) );
            le_protein_temp    ->setText( "20.0" );
            le_protein_residues->setText( QString::number( p.residues ) );
            
            vbar = p.vbar20;
            le_protein_vbar20->setText( QString::number( vbar, 'f', 4 ) );
            le_protein_vbar  ->setText( QString::number( vbar, 'f', 4 ) );
         }

         break;

      case US_Analyte::DNA:
      case US_Analyte::RNA:
         update_nucleotide();
         le_nucle_vbar->setText( QString::number( vbar, 'f', 4 ) );
         break;

      case US_Analyte::CARBOHYDRATE:
         break;
   }

   QMessageBox::information( this,
      tr( "Analyte Loaded Successfully" ),
      tr( "The analyte has been loaded from the database." ) );

   pb_update_db->setEnabled( true );
   pb_del_db   ->setEnabled( true );
   pb_more     ->setEnabled( true );
}

void US_AnalyteGui::save_db( void )
{
   if ( rb_disk->isChecked() )
   {
      save_analyte();
      return;
   }

   if ( ! data_ok() ) return;
   db_access = true;

   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   QStringList q( "new_analyte" );

   q << guid;

   if      ( analyte_type == US_Analyte::PROTEIN ) q << "Protein";
   else if ( analyte_type == US_Analyte::DNA     ) q << "DNA";
   else if ( analyte_type == US_Analyte::RNA     ) q << "RNA";
   else                             q << "Other";

   q << sequence;
   q << QString::number( vbar, 'f', 4 );
   q << description;

   QString spectrum = "";
   q << spectrum;

   // Molecular weight
        if ( analyte_type == US_Analyte::PROTEIN )      q << le_protein_mw->text();
   else if ( analyte_type == US_Analyte::CARBOHYDRATE ) q << "0.0";
   else // DNA / RNA 
   { 
      double mw = le_nucle_mw->text().toDouble();
      q << QString::number( mw, 'f', 4 );
   }
   
   db.statusQuery( q );

   if ( ! database_ok( db ) ) return;

   analyteID = QString::number( db.lastInsertID() );
   set_spectrum( db );

   QMessageBox::information( this,
      tr( "Analyte Saved" ),
      tr( "The analyte has been saved to the database." ) );

   read_db();
}

void US_AnalyteGui::set_spectrum( US_DB2& db )
{
   QStringList q;

   q << "delete_spectrum" << analyteID << "Analyte" << "Extinction";
   db.statusQuery( q );
   q[ 3 ] = "Refraction";
   db.statusQuery( q );
   q[ 3 ] = "Fluorescence";
   db.statusQuery( q );

   QList< double > keys = extinction.keys();
   
   q.clear();
   q << "new_spectrum" << analyteID << "Analyte" << "Extinction" << "" << "";

   for ( int i = 0; i < keys.size(); i++ )
   {
      double key =  keys[ i ];
      QString lambda = QString::number( key, 'f', 1 );
      q[ 4 ] = lambda;

      QString coeff = QString::number( extinction[ key ], 'f', 4 );
      q[ 5 ] = coeff;

      db.statusQuery( q );
   }

   keys = refraction.keys();
   
   q[ 3 ] = "Refraction";

   for ( int i = 0; i < keys.size(); i++ )
   {
      double key =  keys[ i ];
      QString lambda = QString::number( key, 'f', 1 );
      q[ 4 ] = lambda;

      QString coeff = QString::number( refraction[ key ], 'f', 4 );
      q[ 5 ] = coeff;

      db.statusQuery( q );
   }

   keys = fluorescence.keys();
   
   q[ 3 ] = "Fluorescence";

   for ( int i = 0; i < keys.size(); i++ )
   {
      double key =  keys[ i ];
      QString lambda = QString::number( key, 'f', 1 );
      q[ 4 ] = lambda;

      QString coeff = QString::number( fluorescence[ key ], 'f', 4 );
      q[ 5 ] = coeff;

      db.statusQuery( q );
   }
}

void US_AnalyteGui::update_db( void )
{
   if ( ! data_ok() ) return;
   if ( ! db_access ) 
   {
      save_analyte();
      return;
   }

   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   QStringList q( "update_analyte" );
 
   q << analyteID;

   if      ( analyte_type == US_Analyte::PROTEIN ) q << "Protein";
   else if ( analyte_type == US_Analyte::DNA     ) q << "DNA";
   else if ( analyte_type == US_Analyte::RNA     ) q << "RNA";
   else                                q << "Other";

   q << sequence;
   q << QString::number( vbar, 'f', 4 );
   q << description;

   q << " "; // spectrum is not used

   // Molecular weight
        if ( analyte_type == US_Analyte::PROTEIN )      q << le_protein_mw->text();
   else if ( analyte_type == US_Analyte::CARBOHYDRATE ) q << "0.0";
   else // DNA / RNA 
   { 
      double mw = le_nucle_mw->text().toDouble();
      q << QString::number( mw, 'f', 4 );
   }

   status_query( q );

   if ( ! database_ok( db ) ) return;

   set_spectrum( db );

   QMessageBox::information( this,
   tr( "Analyte Updated" ),
      tr( "The analyte has been updated in the database." ) );
}

void US_AnalyteGui::delete_db( void )
{
   if ( rb_disk->isChecked() )
   {
      // Find the file 
      QString path;
      if ( ! analyte_path( path ) ) return;

      QString fn = get_filename( path, le_guid->text() );
      
      // Delete it
      QFile file( fn );
      if ( file.exists() ) file.remove();

      reset();
      read_analyte(); 
      return;
   }

   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   QStringList q;
   q << "delete_analyte" << analyteID;
   status_query( q );

   if ( ! database_ok( db ) ) return;

   QMessageBox::information( this,
      tr( "Analyte Deleted" ),
      tr( "The analyte has been deleted from the database." ) );

   reset();
}

/*  Class US_SequenceEditor */
US_SequenceEditor::US_SequenceEditor( const QString& sequence ) 
   : US_WidgetsDialog( 0, 0 )
{
   setWindowTitle( tr( "Sequence Management" ) );
   setPalette( US_GuiSettings::frameColor() );
   setAttribute( Qt::WA_DeleteOnClose );

   QGridLayout* main = new QGridLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   edit = new US_Editor( US_Editor::LOAD, false );
   edit->e->setAcceptRichText( false );
   edit->e->setText( sequence );
   main->addWidget( edit, 0, 0, 5, 2 );

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( close() ) );
   main->addWidget( pb_cancel, 5, 0 );

   QPushButton* pb_accept = us_pushbutton( tr( "Accept" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( accept() ) );
   main->addWidget( pb_accept, 5, 1 );
}

void US_SequenceEditor::accept( void ) 
{
   emit sequenceChanged( edit->e->toPlainText() );
   close();
}

