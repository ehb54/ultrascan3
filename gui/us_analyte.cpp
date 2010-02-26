//! \file us_analyte.cpp
#include "us_analyte.h"
#include "us_passwd.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_investigator.h"
#include "us_editor.h"

US_Analyte::US_Analyte( int invID, bool signal, QWidget* parent, Qt::WindowFlags f )
   : US_WidgetsDialog( parent, f )
{
   signal_wanted = signal;
   invID         = invID;

   setWindowTitle( tr( "Analyte Management" ) );
   setPalette( US_GuiSettings::frameColor() );
   setAttribute( Qt::WA_DeleteOnClose );

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
   typeButtons->addButton( rb_protein, PROTEIN );
   typeButtons->addButton( rb_dna    , DNA );
   typeButtons->addButton( rb_rna    , RNA );
   typeButtons->addButton( rb_carb   , CARBOHYDRATE );
   connect( typeButtons, SIGNAL( buttonClicked( int ) ),
                         SLOT  ( analyte_type ( int ) ) );

   QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
   connect( pb_investigator, SIGNAL( clicked() ), SLOT( sel_investigator() ) );
   main->addWidget( pb_investigator, row, 0 );

   le_investigator = us_lineedit( tr( "Not Selected" ) );
   main->addWidget( le_investigator, row++, 1 );

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
   main->addWidget( lb_banner1, row++, 0 );

   lw_analytes = us_listwidget();
   connect( lw_analytes, SIGNAL( itemDoubleClicked( QListWidgetItem* ) ),
                         SLOT  ( select_analyte   ( QListWidgetItem* ) ) );

   main->addWidget( lw_analytes, row, 0, 9, 1 );
   row += 9;

   // Labels
   QHBoxLayout* description = new QHBoxLayout;
   QLabel* lb_description = us_label( tr( "Analyte Description:" ) );
   description->addWidget( lb_description );

   le_description = us_lineedit( "" ); 
   description->addWidget( le_description );
   main->addLayout( description, row++, 0, 1, 2 );
 
   // Go back to top of 2nd column
   row = 3;

   QPushButton* pb_load = us_pushbutton( tr( "Load Analyte from HD" ) );
   connect( pb_load, SIGNAL( clicked() ), SLOT( read_analyte() ) );
   main->addWidget( pb_load, row++, 1 );

   pb_save = us_pushbutton( tr( "Save Analyte to HD" ), false );
   connect( pb_save, SIGNAL( clicked() ), SLOT( save_analyte() ) );
   main->addWidget( pb_save, row++, 1 );

   QLabel* lb_banner3 = us_banner( tr( "Database Functions" ), -2 );
   main->addWidget( lb_banner3, row++, 1 );

   QPushButton* pb_load_db = us_pushbutton( 
         tr( "Query Analyte Descriptions from DB" ) );
   connect( pb_load_db, SIGNAL( clicked() ), SLOT( read_db() ) );
   main->addWidget( pb_load_db, row++, 1 );

   pb_save_db = us_pushbutton( tr( "Save Buffer to DB" ), false );
   connect( pb_save_db, SIGNAL( clicked() ), SLOT( save_db() ) );
   main->addWidget( pb_save_db, row++, 1 );

   pb_update_db = us_pushbutton( tr( "Update Buffer in DB" ), false );
   connect( pb_update_db, SIGNAL( clicked() ), SLOT( update_db() ) );
   main->addWidget( pb_update_db, row++, 1 );

   pb_del_db = us_pushbutton( tr( "Delete Buffer from DB" ), false );
   connect( pb_del_db, SIGNAL( clicked() ), SLOT( delete_db() ) );
   main->addWidget( pb_del_db, row++, 1 );

   QLabel* lb_banner4 = us_banner( tr( "Other Functions" ), -2 );
   main->addWidget( lb_banner4, row++, 1 );

   pb_sequence = us_pushbutton( tr( "Manage Sequence" ) );
   connect( pb_sequence, SIGNAL( clicked() ), SLOT( manage_sequence() ) );
   main->addWidget( pb_sequence, row++, 1 );

   pb_spectrum = us_pushbutton( tr( "Manage Spectrum" ), false );
   connect( pb_spectrum, SIGNAL( clicked() ), SLOT( spectrum() ) );
   main->addWidget( pb_spectrum, row++, 1 );

   pb_more = us_pushbutton( tr( "More Info" ), false );
   connect( pb_more, SIGNAL( clicked() ), SLOT( more_info() ) );
   main->addWidget( pb_more, row++, 1 );

   row +=1; 

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
         tr( "VBar <small>(cm<sup>3</sup>/g at 20 &deg;C)</small>:" ) );
   protein_info->addWidget( lb_protein_vbar20, prow, 2 );

   le_protein_vbar20 = us_lineedit( "" );
   le_protein_vbar20->setReadOnly( true );
   protein_info->addWidget( le_protein_vbar20, prow++, 3 );

   QLabel* lb_protein_temp = us_label( 
         tr( "Temperature <small>(&deg;C)</small>:" ) );
   protein_info->addWidget( lb_protein_temp, prow, 0 );

   le_protein_temp = us_lineedit( "" );
   connect( le_protein_temp, SIGNAL( textChanged ( const QString& ) ), 
                             SLOT  ( temp_changed( const QString& ) ) );
   protein_info->addWidget( le_protein_temp, prow, 1 );

   QLabel* lb_protein_vbar = us_label( 
         tr( "VBar <small>(cm<sup>3</sup>/g at T &deg;C)</small>:" ) );
   protein_info->addWidget( lb_protein_vbar, prow, 2 );

   le_protein_vbar = us_lineedit( "" );
   protein_info->addWidget( le_protein_vbar, prow++, 3 );

   QLabel* lb_protein_residues = us_label( tr( "Residue count:" ) );
   protein_info->addWidget( lb_protein_residues, prow, 0 );

   le_protein_residues = us_lineedit( "" );
   le_protein_residues->setReadOnly( true );
   protein_info->addWidget( le_protein_residues, prow, 1 );
   main->addWidget( protein_widget, row, 0, 1, 2 ); 
   
   QLabel* lb_protein_e280 = us_label( 
         tr( "Extinction <small>(280 nm)</small>:" ) );
   protein_info->addWidget( lb_protein_e280, prow, 2 );

   le_protein_e280 = us_lineedit( "" );
   protein_info->addWidget( le_protein_e280, prow++, 3 );
   
   protein_widget->setVisible( true );
  
   // Lower half -- DNA/RNA operations and data
   dna_widget              = new QWidget( this );
   QGridLayout* dna_layout = new QGridLayout( dna_widget );
   dna_layout->setSpacing        ( 2 );
   dna_layout->setContentsMargins( 2, 2, 2, 2 );

   QGroupBox*    gb_double = new QGroupBox( tr( "Calculate MW" ) );
   QGridLayout*  grid1     = new QGridLayout;   
   grid1->setSpacing        ( 2 );
   grid1->setContentsMargins( 2, 2, 2, 2 );
   
   QBoxLayout* box1 = us_checkbox( tr( "Double Stranded" ), cb_stranded, true );
   QBoxLayout* box2 = us_checkbox( tr( "Complement Only" ), cb_mw_only );
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
   ratios->addWidget( lb_ratios, 0, 0, 1, 2 );

   QLabel* lb_sodium = us_label( tr( "Sodium, Na+" ) );
   ratios->addWidget( lb_sodium, 1, 0 );

   ct_sodium = us_counter( 2, 0.0, 1.0, 0.0 );
   ct_sodium->setStep( 0.01 );
   connect( ct_sodium, SIGNAL( valueChanged     ( double ) ),
                       SLOT  ( update_nucleotide( double ) ) );
   ratios->addWidget( ct_sodium, 1, 1 );

   QLabel* lb_potassium = us_label( tr( "Potassium, K+" ) );
   ratios->addWidget( lb_potassium, 2, 0 );

   ct_potassium = us_counter( 2, 0.0, 1.0, 0.0 );
   ct_potassium->setStep( 0.01 );
   connect( ct_potassium, SIGNAL( valueChanged     ( double ) ),
                          SLOT  ( update_nucleotide( double ) ) );
   ratios->addWidget( ct_potassium, 2, 1 );

   QLabel* lb_lithium = us_label( tr( "Lithium, Li+" ) );
   ratios->addWidget( lb_lithium, 3, 0 );

   ct_lithium = us_counter( 2, 0.0, 1.0, 0.0 );
   ct_lithium->setStep( 0.01 );
   connect( ct_lithium, SIGNAL( valueChanged     ( double ) ),
                        SLOT  ( update_nucleotide( double ) ) );
   ratios->addWidget( ct_lithium, 3, 1 );

   QLabel* lb_magnesium = us_label( tr( "Magnesium, Mg+" ) );
   ratios->addWidget( lb_magnesium, 4, 0 );

   ct_magnesium = us_counter( 2, 0.0, 1.0, 0.0 );
   ct_magnesium->setStep( 0.01 );
   connect( ct_magnesium, SIGNAL( valueChanged     ( double ) ),
                          SLOT  ( update_nucleotide( double ) ) );
   ratios->addWidget( ct_magnesium, 4, 1 );

   QLabel* lb_calcium = us_label( tr( "Calcium, Ca+" ) );
   ratios->addWidget( lb_calcium, 5, 0 );

   ct_calcium = us_counter( 2, 0.0, 1.0, 0.0 );
   ct_calcium->setStep( 0.01 );
   connect( ct_calcium, SIGNAL( valueChanged     ( double ) ),
                        SLOT  ( update_nucleotide( double ) ) );
   ratios->addWidget( ct_calcium, 5, 1 );

   dna_layout->addLayout( ratios, 0, 1, 4, 1 );

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

   QLabel* lb_nucle_e260 = us_label( 
         tr( "Extinction<small>(260 nm)</small>:" ) );
   nucle_data->addWidget( lb_nucle_e260, 2, 0 );

   le_nucle_e260 = us_lineedit( "" );
   nucle_data->addWidget( le_nucle_e260, 2, 1 );

   QLabel* lb_nucle_e280 = us_label( 
         tr( "Extinction<small>(280 nm)</small>:" ) );
   nucle_data->addWidget( lb_nucle_e280, 2, 2 );

   le_nucle_e280 = us_lineedit( "" );
   nucle_data->addWidget( le_nucle_e280, 2, 3 );

   dna_layout->addLayout( nucle_data, 4, 0, 2, 2 );
   main->addWidget( dna_widget, row, 0, 2, 2 ); 
   dna_widget->setVisible( false );

   // Lower half -- carbohydrate operations and data
   carbs_widget = new QWidget( this );
   
   QGridLayout* carbs_info   = new QGridLayout( carbs_widget );
   carbs_info->setSpacing        ( 2 );
   carbs_info->setContentsMargins( 2, 2, 2, 2 );

   QLabel* lb_carbs = us_label( "Under Construction" );
   lb_carbs->setAlignment( Qt::AlignCenter );
   carbs_info->addWidget( lb_carbs, 0, 0 );

   main->addWidget( carbs_widget, row, 0, 1, 2 ); 
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

   main->addLayout( buttons, row, 0, 1, 2 );
   analyte_t = PROTEIN;
   reset();
}

void US_Analyte::analyte_type( int type )
{
   switch ( type )
   {
      case PROTEIN:
         dna_widget    ->setVisible( false );
         carbs_widget  ->setVisible( false );
         protein_widget->setVisible( true );
         analyte_t = PROTEIN;
         break;

      case DNA:
      case RNA:
         protein_widget->setVisible( false ); 
         carbs_widget  ->setVisible( false );
         dna_widget    ->setVisible( true );
         analyte_t = ( type == DNA )? DNA : RNA;
         break;

      case CARBOHYDRATE:
         protein_widget->setVisible( false ); 
         dna_widget    ->setVisible( false );
         carbs_widget  ->setVisible( true );
         analyte_t = CARBOHYDRATE;
         break;
   }

   reset();
}
void US_Analyte::close( void )
{
   // Emit signal if requested
   if ( signal_wanted ) emit valueChanged( vbar );

   //emit valueChanged(pep.vbar, pep.vbar20);
   //emit e280Changed(pep.e280);
   //emit mwChanged(pep.mw);
   
   accept();
}

void US_Analyte::reset( void )
{
   inReset = true;
   lw_analytes->clear();

   personID = 0;
   analyteID.clear();

   le_investigator  ->setText( tr( "Not Selected" ) );
   le_search        ->clear();
   le_search        ->setReadOnly( true );

   vbar     = 0.0;
   e280     = 0.0;
   e260     = 0.0;

   filename.clear();
   sequence.clear();

   le_description     ->clear();
   le_protein_temp    ->setText( "20.0" );
   le_protein_residues->clear();
   le_protein_e280    ->clear();

   ct_sodium   ->setValue( 0.0 );
   ct_potassium->setValue( 0.0 );
   ct_lithium  ->setValue( 0.0 );
   ct_magnesium->setValue( 0.0 );
   ct_calcium  ->setValue( 0.0 );

   cb_stranded   ->setChecked( true );
   cb_mw_only    ->setChecked( false );

   rb_3_hydroxyl ->setChecked( true );
   rb_5_hydroxyl ->setChecked( true );

   le_nucle_mw  ->clear();
   le_nucle_vbar->clear();
   le_nucle_e260->clear();
   le_nucle_e280->clear();

   pb_save     ->setEnabled( false );
   pb_save_db  ->setEnabled( false );
   pb_update_db->setEnabled( false );
   pb_del_db   ->setEnabled( false );
   pb_spectrum ->setEnabled( false );
   pb_more     ->setEnabled( false );

   qApp->processEvents();
   inReset = false;
}

void US_Analyte::temp_changed( const QString& text )
{
   double temperature = text.toDouble();

   if ( ! sequence.isEmpty() )
   {
      struct peptide p;
      US_Math::calc_vbar( p, sequence, temperature );
      le_protein_vbar->setText( QString::number( p.vbar, 'f', 4 ) );
   }
}

void US_Analyte::read_analyte( void )
{
   QString fn = QFileDialog::getOpenFileName( this, 
         tr( "Select the analyte file to read" ), US_Settings::dataDir(), 
         tr( "Analyte files (*.fasta)" ) );

   if ( fn.isEmpty() ) return;

   reset();
   QFileInfo fi( fn );
   filename = fi.fileName();

   lw_analytes->clear();
   QFile f( fn );
      
   f.open( QIODevice::ReadOnly | QIODevice::Text );
   QTextStream ts( &f );

   QString header = ts.readLine();

   if ( header.at( 0 ) != '>' )
   {
      QMessageBox::information( this,
         tr( "Attention" ), 
         tr( "The analyte file is not in the proper format!\n"
             "The filemust be in fasta format!\n\n"
             "Please fix this error and try again..." ) );
      return;
   }

   QStringList values = header.remove( 0, 1 ).split( "|" );

   description = values.at( 0 );
   if ( values.size() > 1 ) vbar = values.at( 1 ).toDouble();
   if ( values.size() > 2 ) e280 = values.at( 2 ).toDouble();
   if ( values.size() > 3 ) e260 = values.at( 3 ).toDouble();

   while ( ! ts.atEnd() ) sequence += ts.readLine().toLower();
  
   f.close();

   QString check = sequence;
   check.remove( QRegExp( "\\s" ) );  // Remove whitespace
  
   switch ( analyte_t )
   {
      case PROTEIN:
         sequence.remove( QRegExp( "[^a-z\\+\\-\\?\\@]" ) );
         break;

      case DNA:
      case RNA:
         sequence.remove( QRegExp( "[^acgtu]" ) );
         break;

      case CARBOHYDRATE:
         QMessageBox::information( this,
            tr( "Attention" ),
            tr( "The current analyte type is under construction." ) );
         return;
   }

   if ( check != sequence )
   {
      QMessageBox::information( this,
         tr( "Attention" ), 
         tr( "There are invalid characters in the sequence!\n"
             "The filemust be in fasta format!\n\n"
             "Please fix this error and try again..." ) );
      return;
   }

   switch ( analyte_t )
   {
      case PROTEIN:
      {
         struct peptide p;
         US_Math::calc_vbar( p, sequence, 20.0 );  // Always read at 20C

         le_protein_mw      ->setText( QString::number( (int) p.mw ) );
         le_protein_vbar20  ->setText( QString::number( p.vbar20, 'f', 4 ) );
         le_protein_vbar    ->setText( QString::number( p.vbar  , 'f', 4 ) );
         le_protein_temp    ->setText( "20.0" );
         le_protein_residues->setText( QString::number( p.residues ) );
         le_protein_e280    ->setText( QString::number( e280 ) );
         break;
      }

      case DNA:
      case RNA:
         parse_dna();
         update_nucleotide();
         le_nucle_vbar->setText( QString::number( vbar, 'f', 4 ) );
         le_nucle_e260->setText( QString::number( e260, 'f', 4 ) );
         le_nucle_e280->setText( QString::number( e280, 'f', 4 ) );
         break;

      case CARBOHYDRATE:
         break;
   }

   le_description->setText( description );
   lw_analytes->addItem( tr( "Showing Data from disk drive:\n" ) + fn );
   
   pb_save   ->setEnabled( true );
   pb_save_db->setEnabled( true );
   pb_more   ->setEnabled( true );
}

void US_Analyte::parse_dna( void )
{
   A = sequence.count( "a" );
   C = sequence.count( "c" );
   G = sequence.count( "g" );
   T = sequence.count( "t" );
   U = sequence.count( "u" );
}

void US_Analyte::manage_sequence( void )
{

   US_SequenceEditor* edit = new US_SequenceEditor( sequence );
   connect( edit, SIGNAL( sequenceChanged( QString ) ), 
                  SLOT  ( update_sequence( QString ) ) );
   edit->exec();
}

void US_Analyte::update_sequence( QString seq )
{
   seq = seq.toLower().remove( QRegExp( "\\s" ) );
   QString check = seq;

   if ( seq == sequence ) return;

   switch ( analyte_t )
   {
      case PROTEIN:
         seq.remove( QRegExp( "[^a-z\\+\\-\\?\\@]" ) );
         break;

      case DNA:
      case RNA:
         seq.remove( QRegExp( "[^acgtu]" ) );
         break;

      case CARBOHYDRATE:
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

   switch ( analyte_t )
   {
      case PROTEIN:
         {
         struct peptide p;
         double temperature = le_protein_temp->text().toDouble();
         US_Math::calc_vbar( p, sequence, temperature );

         le_protein_mw      ->setText( QString::number( (int) p.mw ) );
         le_protein_vbar20  ->setText( QString::number( p.vbar20, 'f', 4 ) );
         le_protein_vbar    ->setText( QString::number( p.vbar  , 'f', 4 ) );
         le_protein_residues->setText( QString::number( p.residues ) );
         }
         break;

      case DNA:
      case RNA:
         parse_dna();
         update_nucleotide();
         break;

      case CARBOHYDRATE:
         break;
   }

   pb_save_db->setEnabled( true );
}

void US_Analyte::update_stranded( bool checked )
{
   if ( inReset ) return;
   if ( checked ) cb_mw_only->setChecked( false );
   update_nucleotide();
}

void US_Analyte::update_mw_only( bool checked )
{
   if ( inReset ) return;
   if ( checked ) cb_stranded->setChecked( false );
   update_nucleotide();
}

void US_Analyte::update_nucleotide( bool /* value */ )
{
   if ( inReset ) return;
   update_nucleotide();
}

void US_Analyte::update_nucleotide( double /* value */ )
{
   if ( inReset ) return;
   update_nucleotide();
}

void US_Analyte::update_nucleotide( void )
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

void US_Analyte::save_analyte( void )
{
   if ( ! data_ok() ) return;

   QString fn = QFileDialog::getSaveFileName( this,
                  tr( "Analyte File in FASTA Format" ),
                  US_Settings::dataDir(),
                  tr( "FASTA Files (*.fasta)" ) );

   if ( fn == "" ) return;

   if ( fn.right( 6 ) != ".fasta" ) fn.append( ".fasta" );

   QFile f( fn );
   
   if ( f.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      QTextStream t( &f );

      QString s = ">" + description;

      // Add vbar20 and e280 values
      switch ( analyte_t )
      {
         case PROTEIN:
         {
            struct peptide p;
            double temperature = le_protein_temp->text().toDouble();
            US_Math::calc_vbar( p, sequence, temperature );

            e280 = le_protein_e280->text().toDouble();

            s += "|" + QString::number( p.vbar20, 'f', 4 );
            s += "|" + QString::number( e280,     'f', 4 );
         }
            break;

         case DNA:
         case RNA:
            s += "|" + QString::number( vbar, 'f', 4 );
            s += "|" + QString::number( e280, 'f', 4 );
            s += "|" + QString::number( e260, 'f', 4 );
            break;

         case CARBOHYDRATE:
            break;
      }

      t << s << endl;

      for ( int i = 0; i < sequence.length() / 80; i++ )
         t << sequence.mid( i * 80, 80 ) << endl;

      if ( sequence.length() % 80 > 0 )
         t << sequence.mid( ( sequence.length() / 80 ) * 80 ) << endl;

      f.close();
   }
   else
      QMessageBox::information( this,
         tr( "Error" ),
         tr( "Could not open\n\n" ) + fn + tr( "\n\n for writing." ) );
}

void US_Analyte::more_info( void )
{
   struct peptide p;
   double temperature =  le_protein_temp->text().toDouble();
   US_Math::calc_vbar( p, sequence, temperature );

   QString s;
   QString s1 =
             "***************************************************\n"     +
         tr( "*            Analyte Analysis Results             *\n" )   +
             "***************************************************\n\n\n" +
         tr( "Report for:           " ) + description + "\n\n" +

         tr( "Number of Residues:   " ) + s.sprintf( "%i", p.residues ) + " AA\n";
   s1 += tr( "Molecular Weight:     " ) + s.sprintf( "%i", (int)p.mw )  + tr( " Dalton\n" ) +
         
         tr( "V-bar at 20 deg C:    " ) + 
              QString::number( p.vbar20, 'f', 6 )   + tr( " cm^3/g\n" ) +
         
         tr( "V-bar at " ) + QString::number( temperature, 'f', 2 ) + " deg C: " +
               QString::number( p.vbar, 'f', 6 ) + tr( " cm^3/g\n\n" ) +
       
         tr( "Extinction coefficient for the denatured\n"
             "peptide at 280 nm: ") +
               QString::number( p.e280, 'f', 1 ) + " OD/(mol cm)\n\n" +
        
         tr( "Composition: \n\n" );
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
        
   US_Editor* edit = new US_Editor( US_Editor::LOAD, true );
   QPoint position = this->pos();
   edit->move( position + QPoint( 30, 50 ) );
   edit->resize( 400, 300 );
   edit->e->setFont(  QFont( "monospace", US_GuiSettings::fontSize() ) );
   edit->e->setText( s1 );
   edit->show();
}

void US_Analyte::connect_error( const QString& error )
{
   QMessageBox::warning( this, tr( "Connection Problem" ),
      tr( "Could not connect to databasee \n" ) + error );
}

void US_Analyte::search( const QString& text )
{
   lw_analytes->clear();

   for ( int i = 0; i < info.size(); i++ )
   {
      if ( info[ i ].description.contains( 
             QRegExp( ".*" + text + ".*", Qt::CaseInsensitive ) ) )
      {
         lw_analytes->addItem( new QListWidgetItem(
             info[ i ].analyteID + ": " + info[ i ].description, lw_analytes ) );
      }
   }
}

void US_Analyte::sel_investigator( void )
{
   reset();
   
   US_Investigator* inv_dialog = new US_Investigator( true );
   
   connect( inv_dialog,
      SIGNAL( investigator_accepted( int, const QString&, const QString& ) ),
      SLOT  ( assign_investigator  ( int, const QString&, const QString& ) ) );
   
   inv_dialog->exec();
}

void US_Analyte::assign_investigator( int invID,
      const QString& lname, const QString& fname)
{
   le_investigator->setText( "InvID (" + QString::number( invID ) + "): " +
         lname + ", " + fname );

   personID = invID;
   read_db();
}

void US_Analyte::spectrum( void )
{
      QMessageBox::information( this,
         tr( "Under Construction" ), 
         tr( "Under Construction" ) );
}

void US_Analyte::read_db( void )
{
   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   pb_save     ->setEnabled( false );
   pb_save_db  ->setEnabled( false );
   pb_update_db->setEnabled( false );
   pb_del_db   ->setEnabled( false );

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
      struct analyte_info current;
      current.analyteID   = db.value( 0 ).toString();
      current.description = db.value( 1 ).toString();

      QString a_type    =  db.value( 2 ).toString();

           if ( a_type == "Protein" ) current.type = PROTEIN;
      else if ( a_type == "RNA"     ) current.type = RNA;
      else if ( a_type == "DNA"     ) current.type = DNA;
      else                            current.type = CARBOHYDRATE;

      if ( current.type == analyte_t ) info << current;
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

bool US_Analyte::database_ok( US_DB2& db )
{
   if ( db.lastErrno() == 0 ) return true;

   QMessageBox::information( this,
      tr( "Database Error" ),
      tr( "The following error was returned:\n" ) + db.lastError() );

   return false; 
}

bool US_Analyte::data_ok( void )
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

   return true;
}

void US_Analyte::select_analyte( QListWidgetItem* item )
{
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

   // We know the type : analyte_t = db.value( 0 ).toAnalyte();
   sequence         = db.value( 1 ).toString().toLower();
   vbar             = db.value( 2 ).toDouble();
   description      = db.value( 3 ).toString();
   QString spectrum = db.value( 4 ).toString();
   double  mw       = db.value( 5 ).toDouble();
   personID         = db.value( 6 ).toInt();
   
   filename = "";
   e260     = 0.0;
   e280     = 0.0;

   QStringList extinctions = spectrum.split( ";" );
   QString     extinction;
   
   foreach ( extinction, extinctions )
   {
      QStringList e = extinction.split( ":" );
      if ( e[ 0 ].toInt() == 260 ) e260 =  e[ 1 ].toDouble();
      if ( e[ 0 ].toInt() == 280 ) e280 =  e[ 1 ].toDouble();
   }
        
   le_description->setText( description );

   // Update the window
   switch ( analyte_t )
   {
      case PROTEIN:
         {
            struct peptide p;
            US_Math::calc_vbar( p, sequence, 20.0 );

            if ( mw == 0.0 ) mw = p.mw;
            le_protein_mw      ->setText( QString::number( (int) mw ) );
            le_protein_temp    ->setText( "20.0" );
            le_protein_residues->setText( QString::number( p.residues ) );
            
            if ( vbar == 0.0 ) vbar = p.vbar20;
            le_protein_vbar20->setText( QString::number( vbar, 'f', 4 ) );
            le_protein_vbar  ->setText( QString::number( vbar, 'f', 4 ) );
            le_protein_e280  ->setText( QString::number( e280, 'f', 4 ) );
         }

         break;

      case DNA:
      case RNA:
         update_nucleotide();
         le_nucle_vbar->setText( QString::number( vbar, 'f', 4 ) );
         le_nucle_e260->setText( QString::number( e260, 'f', 4 ) );
         le_nucle_e280->setText( QString::number( e280, 'f', 4 ) );
         break;

      case CARBOHYDRATE:
         break;
   }

   QMessageBox::information( this,
      tr( "Analyte Loaded Successfully" ),
      tr( "The analyte has been loaded from the database." ) );

   pb_update_db->setEnabled( true );
   pb_del_db   ->setEnabled( true );
}

void US_Analyte::save_db( void )
{
   if ( ! data_ok() ) return;

   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   QStringList q( "new_analyte" );

   if      ( analyte_t == PROTEIN ) q << "Protein";
   else if ( analyte_t == DNA     ) q << "DNA";
   else if ( analyte_t == RNA     ) q << "RNA";
   else                             q << "Other";

   q << sequence;
   q << QString::number( vbar, 'f', 4 );
   q << description;

   QString spectrum = "260:" + QString::number( e260, 'f', 4 ) + 
                     ";280:" + QString::number( e280, 'f', 4 );
   q << spectrum;

   // Molecular weight
        if ( analyte_t == PROTEIN )      q << le_protein_mw->text();
   else if ( analyte_t == CARBOHYDRATE ) q << "0.0";
   else // DNA / RNA 
   { 
      double mw = le_nucle_mw->text().toDouble();
      q << QString::number( mw, 'f', 4 );
   }
   
   status_query( q );

   if ( ! database_ok( db ) ) return;

   QMessageBox::information( this,
      tr( "Analyte Saved" ),
      tr( "The analyte has been saved to the database." ) );
}

void US_Analyte::update_db( void )
{
   if ( ! data_ok() ) return;

   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   QStringList q( "update_analyte" );
 
   q << analyteID;

   if      ( analyte_t == PROTEIN ) q << "Protein";
   else if ( analyte_t == DNA     ) q << "DNA";
   else if ( analyte_t == RNA     ) q << "RNA";
   else                             q << "Other";

   q << sequence;
   q << QString::number( vbar, 'f', 4 );
   q << description;

   QString spectrum = "260:" + QString::number( e260, 'f', 4 ) + 
                     ";280:" + QString::number( e280, 'f', 4 );
   q << spectrum;

   // Molecular weight
        if ( analyte_t == PROTEIN )      q << le_protein_mw->text();
   else if ( analyte_t == CARBOHYDRATE ) q << "0.0";
   else // DNA / RNA 
   { 
      double mw = le_nucle_mw->text().toDouble();
      q << QString::number( mw, 'f', 4 );
   }

   status_query( q );

   if ( ! database_ok( db ) ) return;

   QMessageBox::information( this,
      tr( "Analyte Updated" ),
      tr( "The analyte has been updated in the database." ) );
}

void US_Analyte::delete_db( void )
{
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

void US_Analyte:: status_query( const QStringList& q )
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

