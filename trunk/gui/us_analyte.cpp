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
   vbar_info.invID = invID;

   setWindowTitle( tr( "Analyte Management" ) );
   setPalette( US_GuiSettings::frameColor() );
   setAttribute( Qt::WA_DeleteOnClose );

   QBoxLayout* main = new QVBoxLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   QStringList DB = US_Settings::defaultDB();
   QLabel* lb_DB = us_banner( tr( "Database: " ) + DB.at( 0 ) );
   main->addWidget( lb_DB );

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
      emit valueChanged( pep.vbar );
   }
   //emit valueChanged(pep.vbar, pep.vbar20);
   //emit e280Changed(pep.e280);
   //emit mwChanged(pep.mw);
   
   accept();
}

void US_Analyte::reset( void )
{
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
}

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

/*!  Write the peptide analysis results to a text format display.  */
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



#ifdef NEVER

/*!
   If you find the Peptide name in the ListBox by read_file() or read_db().
   Doubleclick it, you will get all vbar data about this name.
   \param item The number of items in ListBox, count start from 0.
*/

void US_Vbar_DB::select_vbar(int item)
{
   if(select_flag && GUI)
   {
      sequence_loaded = true;
      Item = item;
      vbar_info.PepID = item_PepID[item];
      lbl_file2->setText("");
      emit idChanged(vbar_info.PepID);

      retrieve_vbar(vbar_info.PepID);
   }
   else if (GUI)
   {
      QMessageBox::message(tr("Attention:"),
                           tr("No available peptide files\n"));
   }


}

//! Constructor
/*!
   Constractor a new <var>US_Vbar_DB</var> interface, which is a GUI wrapper for us_vbar.
   \param temp a float variable pass temperature value.
   \param temp_vbar a float point variable pass vbar value.
   \param temp_vbar20 a float point variable pass vbar20 value.
   \param temp_GUI <tt>true</tt> will show interface, <tt>false</tt> no interface show up and just pass value.
   \param from_cell <tt>true</tt> called from US_Cell_DB Table, <tt>false</tt> otherwise.
   \param parent Parent widget.
   \param name Widget name.
*/
US_Vbar_DB::US_Vbar_DB(float temp, float *temp_vbar, float *temp_vbar20, bool temp_GUI,
bool from_cell, int temp_InvID, QWidget *parent, const char *name) : US_DB(parent, name)
{
   temperature = temp;
   t1 = temp; t2 = *temp_vbar; t3 = *temp_vbar20;
   cell_flag = from_cell;
   vbar_info.PepID = -1;
   vbar_info.InvID = temp_InvID;
   sequence_loaded = false;
   select_flag = false;    // use for select query listbox
   from_HD = false;
   GUI = temp_GUI;
   if (GUI)
   {
      setup_GUI();
   }
}

/*! Load peptide data from Hard Drive, add warning message when this widget is called by US_Cell_DB. */
void US_Vbar_DB::read_file()
{
   QString str, newstr, test, sequence;
   sequence_loaded = true;
   vbar_info.e280 = 0.0;
   vbar_info.vbar = 0.0;

   if(cell_flag)
   {
      QMessageBox::message(tr("Attention:"),
                           tr("Please use the 'Query Peptide from DB' button\n"
                              "to select a peptide file from the database.\n\n"
                              "If the desired sequence is not stored in the database,\n"
                              "you have to click on 'Save Peptide to DB' first, store\n"
                              "a sequence to the database, then select it from the database."));

   }
   filename = QFileDialog::getOpenFileName(USglobal->config_list.root_dir, "*.pep", 0);
   vbar_info.PepFileName = filename;
   vbar_info.PepFileName = vbar_info.PepFileName.remove(0,(vbar_info.PepFileName.findRev("/", -1,false)+1));
   if ( !filename.isEmpty() && GUI)
   {
      lb_vbar->clear();
      lb_vbar->insertItem("Showing Data from Harddrive:");
      lbl_file2->setText(vbar_info.PepFileName);
      QFile fr(filename);
      fr.open(IO_ReadOnly);
      QTextStream ts (&fr);
      while (test != "DE")
      {
         ts >> test;
         if (fr.atEnd())
         {
            QMessageBox::message(tr("Attention:"), tr("The peptide file is not in the proper format!\n"
                                     "The description identifier \"DE\" is missing!\n\n"
                                     "Please fix this error and try again..."));
            return;
         }
      }
      newstr = ts.readLine();
      vbar_info.Description = newstr.stripWhiteSpace();
      if(GUI)
      {
         lbl_desc2->setText(vbar_info.Description);
      }
      while (test != "SQ")
      {
         ts >> test;
         if (fr.atEnd())
         {
            QMessageBox::message(tr("Attention:"), tr("The peptide file is not in the proper format!\n"
                                          "The sequence identifier \"SQ\" is missing!\n\n"
                                          "Please fix this error and try again..."));
            return;
         }
      }
      ts.readLine();
      sequence = "";
      test = sequence.copy();
      while (test != "//")
      {
         sequence.append(test);
         test = ts.readLine();
         if (fr.atEnd() && test.stripWhiteSpace() != "//")
         {
            QMessageBox::message(tr("Attention:"), tr("The peptide file is not in the proper format!\n"
                                      "The sequence end identifier \"//\" is missing!\n\n"
                                       "Please fix this error and try again..."));
            return;
         }
      }
      if (!fr.atEnd())
      {
         test = ts.readLine();
         vbar_info.vbar = test.toFloat();
      }
      if (!fr.atEnd())
      {
         test = ts.readLine();
         vbar_info.e280 = test.toFloat();
      }
      fr.close();
      vbar_info.Sequence = sequence;
      calc_vbar(&pep, &sequence, &temperature);
      if (vbar_info.e280 > 0.0)
      {
         pep.e280 = vbar_info.e280;
      }
      else
      {
         vbar_info.e280 = pep.e280;
      }
      if (vbar_info.vbar > 0.0)
      {
         pep.vbar20 = vbar_info.vbar;
         pep.vbar = adjust_vbar20(pep.vbar20, temperature);
      }
      else
      {
         vbar_info.vbar = pep.vbar20;
      }
      str = filename;
      str.truncate(str.find("."));
      int pos = str.findRev("/");
      str = str.remove(0,pos+1);
      res_file = USglobal->config_list.result_dir +"/" + str + ".pep_res";
      result_output(res_file);
      from_HD = true;
   }
   this->raise();
}
/*!
   Update <var>temperature</var> with the argument <var>str</var>.
*/
void US_Vbar_DB::update_temp(const QString &str)
{
   if (sequence_loaded)
   {
      temperature = str.toFloat();
   }
   else
   {
      temperature = 20.0;
   }
}

/*!
   Update <var>pep.vbar</var>by recalculating <var>pep.vbar20</var> and <var>temperature</var>.
*/
void US_Vbar_DB::update_vbar()
{
   if(GUI)
   {
      QString test;
      test.sprintf("%5.2f", temperature);
      le_temperature->setText(test);
      if (sequence_loaded)
      {
         pep.vbar = adjust_vbar20(pep.vbar20, temperature);
         lbl_vbar2->setText(test.sprintf("%7.5f ccm/g", pep.vbar));
   //    emit valueChanged(pep.vbar, pep.vbar20);
      }
      else
      {
         pep.vbar = (float) 0.72;
         lbl_vbar2->setText(test.sprintf("%7.5f ccm/g", pep.vbar));
   //    emit valueChanged(pep.vbar, pep.vbar20);
      }
   }
   else
   return;
}

/*! Open a netscape browser to load help page.*/
void US_Vbar_DB::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/vbar.html");
}

/*! Open a netscape browser to download website.*/
void US_Vbar_DB::download()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/download_pep.html");
}
/*! Open a text edit for showing peptide information.*/
void US_Vbar_DB::info()
{
   if (filename.isEmpty())
   {
      QMessageBox::message(tr("Attention:\n"),
               tr("You need to load a peptide sequence file first!\n\nClick on \"Load Peptide from HD or from DB\"\n"));
      return;
   }
   else
   {
      //view_file(res_file);
      TextEdit *e;
      e = new TextEdit();
      e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
      e->load(res_file);
      e->show();
   }
}

/*! Open a text edit for showing sequence.*/
void US_Vbar_DB::show_sequence()
{
   if(from_HD||select_flag)
   {
      QString str, str_sequence;
      TextEdit *e;
      e = new TextEdit();
      e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
      if(from_HD)       //from HD
      {
         TextEdit *e;
         e = new TextEdit();
         e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
         e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);

         e->load(filename);
         e->show();
      }
      if(select_flag)   //from DB
      {
         str.sprintf("SELECT Sequence FROM tblPeptide WHERE PepID = %d;", vbar_info.PepID);
         QSqlQuery query(str);
         if(query.isActive())
         {
            if(query.next())
            {
               str_sequence = query.value(0).toString();
            }

            TextEdit *e;
            e = new TextEdit();
            e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
            e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
            e->load_text(str_sequence);
            e->show();

         }
      }
   }
   else
   {
      QMessageBox::message(tr("Attention:\n"),
               tr("You need to load a peptide sequence file first!\n\nClick on \"Load Peptide from HD or from DB\"\n"));
      return;
   }

}

/*! Load peptide data from database table: tblPeptide. */
void US_Vbar_DB::read_db()
{
}

void US_Vbar_DB::retrieve_vbar(int PepID)
{
}

/*!
   Save all new peptide data to DB table: tblPeptide and exit.
*/
void US_Vbar_DB::save_db()
{
   if(vbar_info.InvID <= 0)
   {
      QMessageBox::message(tr("Attention:"), tr("Please 'Select Investigator' first!"));
      return;
   }

   switch(QMessageBox::information(this, tr("UltraScan - Peptide Database:"), tr("Store this peptide info into the database?"),
                              tr("OK"), tr("CANCEL"), 0,1))
   {
      case 0:
      {
//       int newPepID = get_newID("tblPeptide", "PepID");
         QSqlQuery target;
         QString str;
         str.sprintf("INSERT INTO tblPeptide(PepFileName, Description, "
               "Sequence, InvestigatorID) VALUES('"
             + vbar_info.PepFileName + "', '" + vbar_info.Description +"', '"+ 
             vbar_info.Sequence + "', %d);", vbar_info.InvID);

         target.exec(str);
         break;
      }
      case 1:
      {
         break;
      }
   }
}

/*!
   Open US_DB_Admin to check delete permission.
*/
void US_Vbar_DB::check_permission()
{
   US_DB_Admin *db_admin;
   db_admin = new US_DB_Admin("");
   db_admin->show();
   connect(db_admin, SIGNAL(issue_permission(bool)), SLOT(delete_db(bool)));
}

/*! Delete selected entry of DB table: <tt>tblPeptide</tt>. */
void US_Vbar_DB::delete_db(bool permission)
{
   if(!permission)
   {
      QMessageBox::message(tr("Attention:"),
                           tr("Permission denied"));
      return;
   }
   int j = Item;
   int id = vbar_info.PepID;
   if(id == -1)
   {
      QMessageBox::message(tr("Attention:"),
                           tr("Please select an entry which you\n"
                              "want to be deleted from the database"));

   }
   else
   {
   /*
      QSqlCursor cur( "tblPeptide");
      cur.setMode( QSqlCursor::Delete);
      QSqlIndex filter = cur.index("PepID");
      cur.setValue("PepID", id);
      cur.select(filter);
      cur.primeDelete();
   */
      switch(QMessageBox::information(this, tr("Attention:"),
         tr("Clicking 'OK' will delete the selected sequence from the database"),
         tr("OK"), tr("CANCEL"), 0,1))
      {
         case 0:
         {
            //cur.del();
            QSqlQuery del;
            QString str;
            str.sprintf("DELETE FROM tblPeptide WHERE PepID = %d;", id);
            bool done=del.exec(str);
            if(!done)
            {
               QSqlError sqlerr = del.lastError();
               QMessageBox::message(tr("Attention:"),
                           tr("Delete failed.\n"
                              "Attempted to execute this command:\n\n"
                              + str + "\n\n"
                              "Causing the following error:\n\n")
                              + sqlerr.text());
            }

            if(GUI)
            {
               lb_vbar->removeItem(j);
            }
            clear();
            break;
         }
         case 1:
         {
            break;
         }
      }
   }
}


/*!
   Export the struct peptideDetails values according to Peptide <var>id</var>.
*/
struct peptideDetails US_Vbar_DB::export_vbar(int id)
{
   //QString newstr, test, sequence;
   QString sequence;
   QSqlCursor cur( "tblPeptide" );
   QString chk_pepid;
   chk_pepid.sprintf("PepID = %d",id);
   cur.select(chk_pepid);
   if(cur.next())
   {
   // int size = cur.value("File").toByteArray().size();
   // QByteArray da(size);
   // da = cur.value("File").toByteArray();
   // sequence = get_sequence(da);
      sequence = cur.value("Sequence").toString();
      calc_vbar(&pep, &sequence, &temperature);
      Vbar.vbar = pep.vbar;
      Vbar.vbar20 = pep.vbar20;
      Vbar.e280 = pep.e280;
      QString res_file = USglobal->config_list.result_dir +"/" + QString::number(id) + ".pep_res";
      result_output(res_file);
   }
   else
   {
      QString str = tr("Unable to locate the requested peptide file in the ");
      str.append(login_list.dbname);
      str.append(" database. Please make sure you are connecting\nto the correct database!");
      QMessageBox::message(tr("Attention:"), str);
      Vbar.vbar = -1;
      Vbar.vbar20 = -1;
      Vbar.e280 = -1;
   }
   return Vbar;
}
void US_Vbar_DB::enter_pep ()
{
   if(vbar_info.InvID <= 0)
   {
      QMessageBox::message(tr("Attention:"), tr("Please 'Select Investigator' first!"));
      return;
   }
   enterPepDlg = new US_Enter_Vbar_DB(vbar_info.InvID);
   enterPepDlg->exec();
}


/**************************   Class US_Enter_Vbar_DB    *******************************/
US_Enter_Vbar_DB::US_Enter_Vbar_DB(int temp_InvID, QWidget *parent, const char *name) : US_DB(parent, name)
{

   InvID = temp_InvID;
   filename = "";
   description = "";
   sequence = "";
   vbar = 0.0;
   e280 = 0.0;
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));

   lbl_bar = new QLabel(tr(" Enter Peptide Information:"),this);
   lbl_bar->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_bar->setAlignment(AlignHCenter|AlignVCenter);
   lbl_bar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize , QFont::Bold));

   lbl_description = new QLabel(tr(" Peptide Description:"),this);
   lbl_description->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_description->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_description = new QLineEdit("", this);
   le_description->setAlignment(AlignLeft|AlignVCenter);
   le_description->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_description->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
   connect(le_description, SIGNAL(textChanged(const QString &)), SLOT(update_description(const QString &)));

   lbl_vbar = new QLabel(tr(" Peptide vbar (optional):"),this);
   lbl_vbar->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_vbar = new QLineEdit("", this);
   le_vbar->setAlignment(AlignLeft|AlignVCenter);
   le_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_vbar->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
   le_vbar->setText("0.0");
   connect(le_vbar, SIGNAL(textChanged(const QString &)), SLOT(update_vbar(const QString &)));

   lbl_e280 = new QLabel(tr(" Peptide E280 (optional):"),this);
   lbl_e280->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_e280->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_e280 = new QLineEdit("", this);
   le_e280->setAlignment(AlignLeft|AlignVCenter);
   le_e280->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_e280->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
   le_e280->setText("0.0");
   connect(le_e280, SIGNAL(textChanged(const QString &)), SLOT(update_e280(const QString &)));

   QString str1;
   str1 = tr(" Peptide Sequence:\n"
             " Please Note: If the vbar \n"
             " or E280 value should be \n"
             " determined from sequence,\n"
             " the vbar or E280 value \n"
             " above should be zero ");
   lbl_sequence = new QLabel(str1, this);
   lbl_sequence->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_sequence->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_sequence = new QTextEdit(this, "sequence_editor");
   le_sequence->setAlignment(AlignLeft|AlignVCenter);
   le_sequence->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_sequence->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
   connect(le_sequence, SIGNAL(textChanged()), SLOT(update_sequence()));

   pb_save_HD = new QPushButton(tr("Save to Hard Drive"), this);
   Q_CHECK_PTR(pb_save_HD);
   pb_save_HD->setAutoDefault(false);
   pb_save_HD->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_save_HD->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_save_HD, SIGNAL(clicked()), SLOT(save_HD()));

   pb_save_DB = new QPushButton(tr("Save to Database"), this);
   Q_CHECK_PTR(pb_save_DB);
   pb_save_DB->setAutoDefault(false);
   pb_save_DB->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_save_DB->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_save_DB, SIGNAL(clicked()), SLOT(save_DB()));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setAutoDefault(false);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_close = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_close);
   pb_close->setAutoDefault(false);
   pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_close, SIGNAL(clicked()), SLOT(quit()));

   int j=0;

   QGridLayout * grid1 = new QGridLayout(this, 7, 2, 2);
   for (int i=1; i<7; i++)
   {
      grid1->setRowSpacing(i, 26);
   }
   grid1->setRowSpacing(0, 30);
   grid1->addMultiCellWidget(lbl_bar, j, j, 0, 1);
   j++;
   grid1->addWidget(lbl_description, j, 0);
   grid1->addWidget(le_description, j, 1);
   j++;
   grid1->addWidget(lbl_vbar, j, 0);
   grid1->addWidget(le_vbar, j, 1);
   j++;
   grid1->addWidget(lbl_e280, j, 0);
   grid1->addWidget(le_e280, j, 1);
   j++;
   grid1->addWidget(lbl_sequence, j, 0);
   grid1->addWidget(le_sequence, j, 1);
   j++;
   grid1->addWidget(pb_save_HD, j, 0);
   grid1->addWidget(pb_save_DB, j, 1);
   j++;
   grid1->addWidget(pb_help, j, 0);
   grid1->addWidget(pb_close, j, 1);

   qApp->processEvents();

   QRect r = grid1->cellGeometry(0, 0);

   global_Xpos += 30;
   global_Ypos += 30;

   this->setGeometry(global_Xpos, global_Ypos, r.width(), r.height());
}

US_Enter_Vbar_DB::~US_Enter_Vbar_DB()
{
   global_Xpos -= 30;
   global_Ypos -= 30;
}

void US_Enter_Vbar_DB::update_description(const QString &newText)
{
   description = newText;
}

void US_Enter_Vbar_DB::update_sequence()
{
   sequence = le_sequence->text();
}

void US_Enter_Vbar_DB::update_vbar(const QString & str)
{
   vbar = str.toFloat();
}

void US_Enter_Vbar_DB::update_e280(const QString & str)
{
   e280 = str.toFloat();
}

void US_Enter_Vbar_DB::save_HD()
{
   if (description.isEmpty())
   {
      QMessageBox::message(tr("Attention:"), tr("Please enter a description for\n"
                                         "your peptide before saving it!"));
      return;
   }
   if (sequence.isEmpty() && vbar == 0.0)
   {
      QMessageBox::message(tr("Attention:"), tr("Please enter a sequence or vbar valuefor\n"
                                         "your peptide before saving it!"));
      return;
   }

   filename = QFileDialog::getSaveFileName(USglobal->config_list.root_dir, "*.pep", 0);
   if (!filename.isEmpty())
   {
      if (filename.right(4) != ".pep")
      {
         filename.append(".pep");
      }
      switch(QMessageBox::information(this, tr("UltraScan - Peptide:"),
                              tr("Click 'OK' to save peptide information to :\n"
                                 + filename),
                              tr("OK"), tr("CANCEL"), 0,1))
      {
         case 0:
         {
            QFile f(filename);
            f.open(IO_WriteOnly | IO_Translate);
            QTextStream ts (&f);
            ts << "DE" << "\t";
            ts << description << "\n";
            ts << "SQ" << "\n";
            ts << sequence << "\n"<<"//\n";
            ts << vbar << endl;
            ts << e280;
            f.close();
            break;
         }
         case 1:
         {
            break;
         }
      }
   }


}

void US_Enter_Vbar_DB::save_DB()
{
   if(InvID <= 0)
   {
      QMessageBox::message(tr("Attention:"), 
            tr("Please 'Select Investigator' int the main interface!"));
      exit(0);
   }
   if (description.isEmpty())
   {
      QMessageBox::message(tr("Attention:"), tr("Please enter a description for\n"
                                         "your peptide before saving it!"));
      return;
   }
   if (sequence.isEmpty() && vbar == 0)
   {
      QMessageBox::message(tr("Attention:"), tr("Please enter a sequence for\n"
                                         "your peptide before saving it!"));
      return;
   }
   switch(QMessageBox::information(this, tr("UltraScan - Peptide Database:"), 
            tr("Store this peptide info into the database?"),
                              tr("OK"), tr("CANCEL"), 0,1))
   {
      case 0:
      {
         QSqlQuery target;
         QString str;
         str.sprintf("INSERT INTO tblPeptide(PepFileName, Description, "
               "Sequence, vbar, e280, InvestigatorID) VALUES('"
         + filename + "', '" + description +"', '"+ sequence + "', %f, %f, %d);", vbar, e280, InvID);
         target.exec(str);
         break;
      }
      case 1:
      {
         break;
      }
   }
}

void US_Enter_Vbar_DB::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/enter_vbar.html");
}

void US_Enter_Vbar_DB::quit()
{
   close();
}
#endif
