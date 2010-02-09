//! \file us_analyte.cpp
#include "us_analyte.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_investigator.h"

US_Analyte::US_Analyte( int invID, bool signal, QWidget* parent, Qt::WindowFlags f )
   : US_WidgetsDialog( parent, f )
{
   signal_wanted = signal;
   
   //vbar_info.invID = invID;

   setWindowTitle( tr( "Analyte Management" ) );
   setPalette( US_GuiSettings::frameColor() );
   setAttribute( Qt::WA_DeleteOnClose );

   QBoxLayout* main = new QVBoxLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   QStringList DB = US_Settings::defaultDB();
   QLabel* lb_DB = us_banner( tr( "Database: " ) + DB.at( 0 ) );
   main->addWidget( lb_DB );
/*
   // Top part
   int row = 0;
   QGridLayout* top = new QGridLayout;


   QBoxLayout* search = new QHBoxLayout;
   QLabel* lb_search = us_label( tr( "Search" ) );
   search->addWidget( lb_search );

   le_search = us_lineedit();
   connect( le_search, SIGNAL( textChanged( const QString& ) ),
                       SLOT  ( search     ( const QString& ) ) );
   search->addWidget( le_search);

   top->addLayout( search, row++, 0 );

   QLabel* doubleckick = 
      us_banner( tr( "Doubleclick on analyte to select" ), -2 );

   top->addWidget( doubleckick, row++, 0 );

   lw_peptides = us_listwidget();
   connect( lw_peptides, SIGNAL( itemDoubleClicked( QListWidgetItem* ) ),
                         SLOT  ( select_peptide   ( QListWidgetItem* ) ) );
   top->addWidget( lw_peptides, row, 0, 6, 1 );

   row = 0;

   QPushButton* pb_load = us_pushbutton( tr( "Load Analyte from HD" ) );
   connect( pb_load, SIGNAL( clicked() ), SLOT( read_peptide() ) );
   top->addWidget( pb_load, row++, 1 );

   QPushButton* pb_load_db = 
      us_pushbutton( tr( "Query Analyte Info from DB" ) );
   connect( pb_load_db, SIGNAL( clicked() ), SLOT( read_db() ) );
   top->addWidget( pb_load_db, row++, 1 );

   QPushButton* pb_enter = us_pushbutton( tr( "Enter Analyte" ) );
   connect( pb_enter, SIGNAL( clicked() ), SLOT( enter_peptide() ) );
   top->addWidget( pb_enter, row++, 1 );

   pb_save = us_pushbutton( tr( "Save Analyte to DB" ), false );
   connect( pb_save, SIGNAL( clicked() ), SLOT( save_peptide() ) );
   top->addWidget( pb_save, row++, 1 );

   pb_delete = us_pushbutton( tr( "Delete Analyte from DB" ), false );
   connect( pb_delete, SIGNAL( clicked() ), SLOT( del_peptide() ) );
   top->addWidget( pb_delete, row++, 1 );

   QPushButton* pb_download = us_pushbutton( tr( "Download Sequence" ) );
   connect( pb_download, SIGNAL( clicked() ), SLOT( download_seq() ) );
   top->addWidget( pb_download, row++, 1 );

   pb_view = us_pushbutton( tr( "View Sequence" ), false );
   connect( pb_view, SIGNAL( clicked() ), SLOT( view_seq() ) );
   top->addWidget( pb_view, row++, 1 );

   pb_more = us_pushbutton( tr( "More Information" ), false );
   connect( pb_more, SIGNAL( clicked() ), SLOT( more_info() ) );
   top->addWidget( pb_more, row++, 1 );
   
   main->addLayout( top );

   // Bottom part
   row = 0;
   QGridLayout* bottom = new QGridLayout;

   QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
   connect( pb_investigator, SIGNAL( clicked() ), SLOT( sel_investigator() ) );
   bottom->addWidget( pb_investigator, row, 0 );

   QPalette p;
   p.setColor( QPalette::Window,     Qt::lightGray );
   p.setColor( QPalette::WindowText, Qt::black     );

   lb_investigator = us_label( "Not Selected" );
   lb_investigator->setPalette( p );
   bottom->addWidget( lb_investigator, row++, 1 );

   QLabel* lb_file = us_label( tr( "Selected File:" ) );
   bottom->addWidget( lb_file, row, 0 );

   lb_file_value = us_label( "No File Selected..." );
   lb_file_value->setPalette( p );
   bottom->addWidget( lb_file_value, row++, 1 );

   QLabel* lb_desc = us_label( tr( "Protein Description:" ) );
   bottom->addWidget( lb_desc, row, 0 );

   lb_desc_value = us_label( "Not Selected" );;
   lb_desc_value->setPalette( p );
   bottom->addWidget( lb_desc_value, row++, 1 );

   QLabel* lb_temperature = 
      us_label( tr( "Temperature <span>(&deg;C)</span>:" ) );
   bottom->addWidget( lb_temperature, row, 0 );

   le_temperature = us_lineedit( "20.0" );
   bottom->addWidget( le_temperature, row++, 1 );

   QLabel* lb_vbar = us_label( tr( "vbar (temperature):" ) );
   bottom->addWidget( lb_vbar, row, 0 );

   lb_vbar_value = us_label( "0.72000 cm<sup>3</sup>/g" );
   lb_vbar_value->setPalette( p );
   bottom->addWidget( lb_vbar_value, row++, 1 );

   QLabel* lb_vbar20 = us_label( tr( "vbar (20 <span>(&deg;C)</span>:" ) );
   bottom->addWidget( lb_vbar20, row, 0 );

   lb_vbar20_value = us_label( "0.72000 cm<sup>3</sup>/g" );
   lb_vbar20_value->setPalette( p );
   bottom->addWidget( lb_vbar20_value, row++, 1 );

   QLabel* lb_e280 = us_label( tr( "E280 (denatured):" ) );
   bottom->addWidget( lb_e280, row, 0 );

   lb_e280_value = us_label( "1.0 OD/(mol cm)" );
   lb_e280_value->setPalette( p );
   bottom->addWidget( lb_e280_value, row++, 1 );

   QLabel* lb_residues = us_label( tr( "Residue Count:" ) );
   bottom->addWidget( lb_residues, row, 0 );

   lb_residues_value = us_label( "Not Selected" );
   lb_residues_value->setPalette( p );
   bottom->addWidget( lb_residues_value, row++, 1 );

   QLabel* lb_mw = us_label( tr( "Molecular Weight:" ) );
   bottom->addWidget( lb_mw, row, 0 );

   lb_mw_value = us_label( "Not Selected" );
   lb_mw_value->setPalette( p );
   bottom->addWidget( lb_mw_value, row++, 1 );

   main->addLayout( bottom );
*/
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

   main->addLayout( buttons  );
   reset();
}

void US_Analyte::close( void )
{
   // emit signals if requested
   if ( signal_wanted )
   {
      //emit valueChanged( pep.vbar );
   }
   //emit valueChanged(pep.vbar, pep.vbar20);
   //emit e280Changed(pep.e280);
   //emit mwChanged(pep.mw);
   
   accept();
}

void US_Analyte::reset( void )
{
   /*
   lw_peptides->clear();
   peptides.clear();

   lb_investigator  ->setText( "Not Selected" );
   lb_file_value    ->setText( "No File Selected..." );
   lb_desc_value    ->setText( "Not Selected" );
   lb_vbar_value    ->setText( "0.72000 cm<sup>3</sup>/g" );
   lb_vbar20_value  ->setText( "0.72000 cm<sup>3</sup>/g" );
   lb_e280_value    ->setText( "1.0 OD/(mol cm)" );
   lb_residues_value->setText( "Not Selected" );
   lb_mw_value      ->setText( "Not Selected" );

   le_temperature   ->setText( "20.0" );
   le_search        ->setText( "" );
   le_search        ->setReadOnly( true );

   pb_save  ->setEnabled( false );
   pb_delete->setEnabled( false );
   pb_view  ->setEnabled( false );
   pb_more  ->setEnabled( false );
   */
}
/*
void US_Analyte::read_peptide( void )
{
   vbar_info.e280 = 0.0;
   vbar_info.vbar = 0.0;

   QString filename = QFileDialog::getOpenFileName( this, 
         tr( "Select the peptide file to read" ), US_Settings::dataDir(), 
         tr( "Peptide files (*.pep)" ) );

   if ( filename.isEmpty() ) return;

   QFileInfo fi( filename );

   vbar_info.filename = fi.fileName();

   lw_peptides->clear();
      
   QFile fr( filename );
      
   fr.open( QIODevice::ReadOnly );
   QTextStream ts ( &fr );
      
   QString identifier = "";

   while ( identifier != "DE" )
   {
      ts >> identifier;
         
      if ( ts.atEnd() )
      {
         QMessageBox::information( this,
            tr( "Attention" ), 
            tr( "The peptide file is not in the proper format!\n"
                "The description identifier \"DE\" is missing!\n\n"
                "Please fix this error and try again..." ) );
         return;
      }
   }

   vbar_info.description = ts.readLine().trimmed();

   while ( identifier != "SQ")
   {
      ts >> identifier;
         
      if ( ts.atEnd() )
      {
         QMessageBox::information( this,
               tr( "Attention" ), 
               tr( "The peptide file is not in the proper format!\n"
                   "The sequence identifier \"SQ\" is missing!\n\n"
                   "Please fix this error and try again..." ) );
         return;
      }
   }
      
   ts.readLine();
   
   QString sequence = "";
   identifier       = "";

   while ( identifier != "//" )
   {
      sequence.append( identifier );
      identifier = ts.readLine();
      
      if ( ts.atEnd() && identifier.trimmed() != "//" )
      {
         QMessageBox::information( this,
            tr( "Attention" ), 
            tr( "The peptide file is not in the proper format!\n"
                "The sequence end identifier \"//\" is missing!\n\n"
                "Please fix this error and try again..." ) );
         return;
      }
   }

   vbar_info.sequence = sequence;

   if ( ! fr.atEnd() )
      vbar_info.vbar = ts.readLine().toDouble();

   if ( ! fr.atEnd() )
      vbar_info.e280 = ts.readLine().toDouble();

   fr.close();
   
   double degC = le_temperature->text().toDouble();
   US_Math::calc_vbar( pep, sequence, degC );

   if ( vbar_info.vbar > 0.0 )
   {
      pep.vbar20 = vbar_info.vbar;
      pep.vbar   = US_Math::adjust_vbar20( pep.vbar20, degC );
   }
   else
      vbar_info.vbar = pep.vbar20;

   if ( vbar_info.e280 > 0.0 )
      pep.e280 = vbar_info.e280;
   else
      vbar_info.e280 = pep.e280;
   
   lb_file_value->setText( fi.fileName() );
   lb_desc_value->setText( vbar_info.description );
   lw_peptides  ->addItem( "Showing Data from Harddrive" );
      
   QString res_file = US_Settings::resultDir() 
      + "/" + fi.completeBaseName() + ".pep_res";
   
   result_output( res_file );
   //from_HD = true;
}

void US_Analyte::read_db( void )
{
   QString query; 

   if ( vbar_info.invID > 0 )
   {
      query = "SELECT PepID, Description FROM tblPeptide "
              "WHERE InvestigatorID = " + QString::number( vbar_info.invID );
   }
   else
      query = "SELECT PepID, Description FROM tblPeptide ORDER BY PepID DESC";

   peptides.clear();
   lw_peptides->clear();

   US_DB*    db = new US_DB;
   US_Passwd pw;
   QString   error;

   // Get the peptide data from the database
   if ( ! db->open( pw.getPasswd(), error ) )
   {
      // Error message here
      qDebug() << "US_Analyte::read_db open DB error " << error;
      return;
   }

   db->query( query );

   while ( db->next() )
   {
      QString info = "PepID (" + db->value( 0 ).toString() + ") " + 
                                 db->value( 1 ).toString();
      
      peptides << info;

      lw_peptides->addItem( info );
   }

   delete db;
   QSqlDatabase::removeDatabase( "UltraScan" );

   if ( peptides.size() == 0 )
   {
      QMessageBox::information( this,
            tr( "Attention" ),
            tr( "No peptide file found for the selected investigator,\n"
                "You can 'Reset' then query DB to list all Peptide files." ) );
   }
   else
      le_search->setReadOnly( false );
}
*/
/*!  Write the peptide analysis results to a text format display.  */
/*
void US_Analyte::result_output( const QString& res_file )
{
   QString s;
   s.sprintf( "%5.3f", le_temperature->text().toFloat() );
   
   QFile result( res_file );
   result.open( QIODevice::WriteOnly | QIODevice::Text );
   
   QTextStream  res_io( &result );

   res_io <<     "***************************************************\n";
   res_io << tr( "*            Peptide Analysis Results             *\n");
   res_io <<     "***************************************************\n\n\n";
   res_io << tr( "Report for:         " ) << vbar_info.description << "\n\n";

   res_io << tr( "Number of Residues: " ) << pep.residues    << " AA\n";
   res_io << tr( "Molecular Weight:   " ) << pep.mw          << tr( " Dalton\n" );
   res_io << tr( "V-bar at 20 deg C:  " ) << pep.vbar20      << " cm^3/g\n";
   res_io << tr( "V-bar at " )            << s               << " deg C: " 
                                          << pep.vbar        << " cm^3/g\n";
   
   res_io << tr( "Extinction coefficient for the denatured\npeptide at 280 nm: ") 
          << pep.e280 << " OD/(mol cm)\n\n";
   res_io << tr( "Composition: \n\n" );
   res_io << tr( "Alanine:\t" )                  << pep.a 
          << tr( "\tArginine:\t" )               << pep.r << "\n";
   res_io << tr( "Asparagine:\t" )               << pep.n 
          << tr( "\tAspartate:\t" )              << pep.d << "\n";
   res_io << tr( "Asparagine or\nAspartate:\t" ) << pep.b << "\n";
   res_io << tr( "Cysteine:\t" )                 << pep.c 
          << tr( "\tGlutamate:\t" )              << pep.e << "\n";
   res_io << tr( "Glutamine:\t" )                << pep.q 
          << tr( "\tGlycine:\t" )                << pep.g << "\n";
   res_io << tr( "Glutamine or\nGlutamate:\t" )  << pep.z << "\n";
   res_io << tr( "Histidine:\t" )                << pep.h 
          << tr( "\tIsoleucine:\t" )             << pep.i << "\n";
   res_io << tr( "Leucine:\t" )                  << pep.l 
          << tr( "\tLysine:\t\t" )               << pep.k << "\n";
   res_io << tr( "Methionine:\t" )               << pep.m 
          << tr( "\tPhenylalanine:\t" )          << pep.f << "\n";
   res_io << tr( "Proline:\t" )                  << pep.p 
          << tr( "\tSerine:\t\t" )               << pep.s << "\n";
   res_io << tr( "Threonine:\t" )                << pep.t 
          << tr( "\tTryptophan:\t" )             << pep.w << "\n";
   res_io << tr( "Tyrosine:\t" )                 << pep.y 
          << tr( "\tValine:\t\t" )               << pep.v << "\n";
   res_io << tr( "Unknown:\t" )                  << pep.x 
          << tr( "\tHao:\t\t" )                  << pep.j << "\n";
   res_io << tr( "Delta-linked Ornithine:\t" )   << pep.o << endl;
   result.close();

   lb_vbar_value    ->setText( s.sprintf( "%7.5f cm<sup>3</sup>/g", pep.vbar) );
   lb_vbar20_value  ->setText( s.sprintf( "%7.5f cm<sup>3</sup>/g", pep.vbar20 ) );
   lb_residues_value->setText( s.sprintf( "%d AA",            pep.residues ) );
   lb_e280_value    ->setText( s.sprintf( "%.1f OD/(mol cm)", pep.e280     ) );
   lb_mw_value      ->setText( s.sprintf( "%6.4e Dalton",     pep.mw       ) );
}

void US_Analyte::search( const QString& text )
{
   lw_peptides->clear();

   for ( int i = 0; i < peptides.size(); i++ )
   {
      if ( peptides[ i ].contains( 
               QRegExp( ".*" + text + ".*", Qt::CaseInsensitive ) ) )
      {
         lw_peptides->addItem( peptides[ i ] );
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
   lb_investigator->setText( "InvID (" + QString::number( invID ) + "): " +
         lname + ", " + fname );

   vbar_info.invID = invID;
   read_db();
}

void US_Analyte::select_peptide( QListWidgetItem* item )
{
   QString entry = item->text();

   int     left  = entry.indexOf( '(' ) + 1;
   int     right = entry.indexOf( ')' );
   QString pepID = entry.mid( left, right - left );



   QString query = "SELECT Description, Sequence, InvestigatorID, vbar, e280 "
                   "FROM tblPeptide WHERE PepID = " + pepID;
   
   US_DB*    db = new US_DB;
   US_Passwd pw;
   QString   error;

   if ( ! db->open( pw.getPasswd(), error ) )
   {
      // Error message here
      qDebug() << "US_Analyte::select_peptide: Could not open DB\n" << error;

      return;
   }

   db->query( query );
   db->next();

   vbar_info.pepID       = pepID.toInt();
   vbar_info.description = db->value( 0 ).toString();
   vbar_info.sequence    = db->value( 1 ).toString();
   QString invID         = db->value( 2 ).toString();
   vbar_info.invID       = invID.toInt();
   vbar_info.vbar        = db->value( 3 ).toDouble();
   vbar_info.e280        = db->value( 4 ).toDouble();
   
   // Get the investigator name
   query = "SELECT FirstName, LastName FROM tblInvestigators "
           "WHERE InvID=" + invID;

   db->query( query );
   db->next();

   QString fname = db->value( 0 ).toString();
   QString lname = db->value( 1 ).toString();

   delete db;
   QSqlDatabase::removeDatabase( "UltraScan" );

   double degC = le_temperature->text().toDouble();
   US_Math::calc_vbar( pep, vbar_info.sequence, degC );

   if ( vbar_info.e280 > 0.0 )
      pep.e280 = vbar_info.e280;
   else
      vbar_info.e280 = pep.e280;
         
   if ( vbar_info.vbar > 0.0 )
   {
      pep.vbar20 = vbar_info.vbar;
      pep.vbar = US_Math::adjust_vbar20( pep.vbar20, degC );
   }
   else
      vbar_info.vbar = pep.vbar20;

   lb_desc_value->setText( vbar_info.description );

   lb_investigator->setText( "InvID (" + invID + "): " +
         lname + ", " + fname );
   
   result_output( US_Settings::resultDir() + "/" + pepID + ".pep_res" );

   pb_save->setEnabled  ( true );
   pb_delete->setEnabled( true );
   pb_view->setEnabled  ( true );
   pb_more->setEnabled  ( true );
}

void US_Analyte::view_seq( void )
{
   QMessageBox::information( this, "Under Construction", "Not implemented yet." );
}

void US_Analyte::enter_peptide( void )
{
   QMessageBox::information( this, "Under Construction", "Not implemented yet." );
}

void US_Analyte::save_peptide( void )
{
   QMessageBox::information( this, "Under Construction", "Not implemented yet." );
}

void US_Analyte::del_peptide( void )
{
   QMessageBox::information( this, "Under Construction", "Not implemented yet." );
}

void US_Analyte::download_seq( void )
{
   QMessageBox::information( this, "Under Construction", "Not implemented yet." );
}

void US_Analyte::more_info( void )
{
   QMessageBox::information( this, "Under Construction", "Not implemented yet." );
}
*/

