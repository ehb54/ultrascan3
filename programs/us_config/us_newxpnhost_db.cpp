//! \file us_xpnhost.cpp

#include "us_xpnhost_db.h"
#include "us_newxpnhost_db.h"
#include "us_xpn_data.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_passwd.h"
#include "us_help.h"
#include "us_crypto.h"
#include "us_db2.h"


#define def_desc QString("place-holder")
#define def_host QString("192.168.1.1")
#define def_port QString("")
#define def_msgPort QString("")
#define def_name QString("AUC_DATA_DB")
#define def_user QString("")
#define def_pasw QString("")


US_NewXpnHostDB::US_NewXpnHostDB() : US_Widgets()
{
  
   // Frame layout
   setPalette( US_GuiSettings::frameColor() );

   setWindowTitle( "New Instrument Configuration" );
   setAttribute( Qt::WA_DeleteOnClose );

   use_db              = ( US_Settings::default_data_location() < 2 );

   update_instrument = false;

   nonOptima_selected = false;
   
   QBoxLayout* topbox = new QVBoxLayout( this );
   topbox->setSpacing( 2 );

   /* ALEXEY: NEW portion */
   // QLabel* banner = us_banner( tr( "Enter Info for the New Xpn Host:" ) );
   // topbox->addWidget( banner );
   
   int row = 0;
   QGridLayout* details = new QGridLayout();

   // Row 0
   QLabel* banner_type   = us_banner( tr( "Select Instrument Type:" ) );
   banner_type->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   details->addWidget( banner_type,      row++, 0, 1, 4 );

   // Row 0a
   QLabel* lb_type      = us_label( tr( "Instument Type:" ) );
   
   cb_type              = us_comboBox();
   QStringList itypes;
   itypes << "Optima (Beckman)"
	  << "non-Optima";
   cb_type->addItems( itypes );
   details->addWidget( lb_type,        row,   0, 1, 1 );
   details->addWidget( cb_type,        row++, 1, 1, 3 );
   cb_type->setCurrentIndex( 0 );

   connect( cb_type,      SIGNAL( activated    ( int ) ),
            this,         SLOT  ( changeType   ( int ) ) );
   
   // Row 0b
   QLabel* banner   = us_banner( tr( "Enter Info for the New Instrument:" ) );
   banner->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   details->addWidget( banner,      row++, 0, 1, 4 );

   /* ALEXEY: End of NEW portion */

   // Row 1
   QLabel* desc         = us_label( tr( "Instrument Host Description:" ) );
   le_description       = us_lineedit( "", 0 );
   le_description->setPlaceholderText("SYNTAX: 'Optima #'");
   details->addWidget( desc,           row,   0, 1, 2 );
   details->addWidget( le_description, row++, 2, 1, 2 );
   connect( le_description, SIGNAL( textChanged(QString) ),
            this,      SLOT  ( desc_changed(QString) ) );

   // Row 1a
   QLabel* serialNum    = us_label( tr( "Instrument Serial Number:" ) );
   le_serialNumber      = us_lineedit( "", 0 );
   details->addWidget( serialNum,       row,   0, 1, 2 );
   details->addWidget( le_serialNumber, row++, 2, 1, 2 );   

   // Row 2
   host                = us_label( tr( "Instrument DB Host Address:" ) );
   le_host             = us_lineedit( "", 0 );
   details->addWidget( host,           row,   0, 1, 2 );
   details->addWidget( le_host,        row++, 2, 1, 2 );

   // Row 3
   port                = us_label( tr( "Instument DB Port:" ) );
   le_port             = us_lineedit( def_port, 0 );
   details->addWidget( port,           row,   0, 1, 2 );
   details->addWidget( le_port,        row++, 2, 1, 2 );

   // Row 3a
   msgPort        = us_label( tr( "Instrument Status Msg Port:" ) );
   le_msgPort        = us_lineedit( def_msgPort, 0 );
   details->addWidget( msgPort,           row,   0, 1, 2 );
   details->addWidget( le_msgPort,        row++, 2, 1, 2 );

   // Row 4
   name        = us_label( tr( "Instrument DB Name:" ) );
   le_name             = us_lineedit( def_name, 0 );
   details->addWidget( name,           row,   0, 1, 2 );
   details->addWidget( le_name,        row++, 2, 1, 2 );

   // Row 5
   user        = us_label( tr( "Instrument DB Username:" ) );
   le_user             = us_lineedit( def_user, 0 );
   details->addWidget( user,           row,   0, 1, 2 );
   details->addWidget( le_user,        row++, 2, 1, 2 );

   // Row 6
   pasw        = us_label( tr( "Instrument DB Password:" ) );
   le_pasw             = us_lineedit( def_pasw, 0 );
   le_pasw->setEchoMode( QLineEdit::Password );
   details->addWidget( pasw,           row,   0, 1, 2 );
   details->addWidget( le_pasw,        row++, 2, 1, 2 );

   //Row 6a
   bn_chromoab   = us_banner( tr( "Chromatic Aberration Information" ) );
   details->addWidget( bn_chromoab,      row++, 0, 1, 4 );

   //Row 6b
   lb_radcalwvl  = us_label( tr( "Radial Calibration Wavelength:" ) );
   ct_radcalwvl          = us_counter( 2,   190,   800,  280 );
   ct_radcalwvl ->setSingleStep( 1 );
   details->addWidget( lb_radcalwvl,    row,   0, 1, 2 );
   details->addWidget( ct_radcalwvl,    row++, 2, 1, 2 );
   
   //Row 6c
   pb_loadchromo     = us_pushbutton( tr( "Load Chromatic Aberration Array" ) );
   le_chromofile          = us_lineedit( "", 0, true );
   details->addWidget( pb_loadchromo,    row,   0, 1, 2 );
   details->addWidget( le_chromofile,    row++, 2, 1, 2 );  
   connect( pb_loadchromo,     SIGNAL( clicked()          ), 
              this,            SLOT(   load_chromo()    ) ); 

   
   // Row 7
   QLabel* bn_optsys   = us_banner( tr( "Installed Optical Systems" ) );
   bn_optsys->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   details->addWidget( bn_optsys,      row++, 0, 1, 4 );

   // Rows 8,9,10
   QStringList osyss;
   osyss << "UV/visible"
         << "Rayleigh Interference"
         << "Fluorescense"
         << "(not installed)";
   QLabel* lb_os1      = us_label( tr( "Op Sys1:" ) );
   QLabel* lb_os2      = us_label( tr( "Op Sys2:" ) );
   QLabel* lb_os3      = us_label( tr( "Op Sys3:" ) );
   cb_os1              = us_comboBox();
   cb_os2              = us_comboBox();
   cb_os3              = us_comboBox();
   cb_os1->addItems( osyss );
   cb_os2->addItems( osyss );
   cb_os3->addItems( osyss );
   details->addWidget( lb_os1,         row,   0, 1, 1 );
   details->addWidget( cb_os1,         row++, 1, 1, 3 );
   details->addWidget( lb_os2,         row,   0, 1, 1 );
   details->addWidget( cb_os2,         row++, 1, 1, 3 );
   details->addWidget( lb_os3,         row,   0, 1, 1 );
   details->addWidget( cb_os3,         row++, 1, 1, 3 );
   cb_os1->setCurrentIndex( 0 );
   cb_os2->setCurrentIndex( 1 );
   cb_os3->setCurrentIndex( 3 );
      
   topbox->addLayout( details );

   //Pushbuttons
   row = 0;
   QGridLayout* buttons = new QGridLayout();

   pb_testconn = us_pushbutton( tr( "Test Connectivity" ) );
   pb_testconn->setEnabled( true );
   connect( pb_testconn, SIGNAL( clicked( ) ), this, SLOT( test_connectivity( ) ) );
   buttons->addWidget( pb_testconn, row++, 0, 1, 2 );
   
   pb_save = us_pushbutton( tr( "Save Entry" ) );
   pb_save->setEnabled( true );
   connect( pb_save, SIGNAL( clicked( ) ), this, SLOT( save_new( ) ) );
   buttons->addWidget( pb_save, row, 0, 1, 1 );
   pb_save->setEnabled(false);

   pb_cancel = us_pushbutton( tr( "Cancel" ) );
   pb_cancel->setEnabled( true );
   connect( pb_cancel,      SIGNAL( clicked()  ),
	    this,           SLOT  ( cancel() ) );
   buttons->addWidget( pb_cancel, row++, 1, 1, 1 );
   
   topbox->addLayout( buttons );

   setMinimumSize( 450 , 400 );
   adjustSize();
}


US_NewXpnHostDB::US_NewXpnHostDB( QMap <QString,QString> currentInstrument ) : US_Widgets()
{

   this->instrumentedit = currentInstrument;

   if ( !instrumentedit[ "name" ].contains("Optima") )
     nonOptima_selected = true;
   else
     nonOptima_selected = false;

   qDebug() << " non-Optima ? " << nonOptima_selected; 
   
   // Frame layout
   setPalette( US_GuiSettings::frameColor() );

   setWindowTitle( "Modify Instrument DB Host Configuration" );
   setAttribute( Qt::WA_DeleteOnClose );

   use_db              = ( US_Settings::default_data_location() < 2 );
   update_instrument = true;
   
   QBoxLayout* topbox = new QVBoxLayout( this );
   topbox->setSpacing( 2 );

   // QLabel* banner = us_banner( tr( "Change Xpn Host Configuration:" ) );
   // banner->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   // topbox->addWidget( banner );

   // Row 0
   int row = 0;
   QGridLayout* details = new QGridLayout();

   QLabel* banner = us_banner( tr( "Change Instrument Configuration:" ) );
   banner->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   details->addWidget( banner, row++, 0, 1, 4 );

   // Row 1
   QLabel* desc         = us_label( tr( "Instrument Host Description:" ) );
   le_description       = us_lineedit( "", 0, true );
   details->addWidget( desc,           row,   0, 1, 2 );
   details->addWidget( le_description, row++, 2, 1, 2 );

   // Row 1a
   QLabel* serialNum    = us_label( tr( "Instrument Serial Number:" ) );
   le_serialNumber      = us_lineedit( "", 0 );
   details->addWidget( serialNum,       row,   0, 1, 2 );
   details->addWidget( le_serialNumber, row++, 2, 1, 2 );   

   // Row 2
   host        = us_label( tr( "Instrument DB Host Address:" ) );
   le_host             = us_lineedit( "", 0 );
   details->addWidget( host,           row,   0, 1, 2 );
   details->addWidget( le_host,        row++, 2, 1, 2 );

   // Row 3
   port        = us_label( tr( "Instrument DB Port:" ) );
   le_port             = us_lineedit( def_port, 0 );
   details->addWidget( port,           row,   0, 1, 2 );
   details->addWidget( le_port,        row++, 2, 1, 2 );

   // Row 3a
   msgPort        = us_label( tr( "Instrument Status Msg Port:" ) );
   le_msgPort        = us_lineedit( def_msgPort, 0 );
   details->addWidget( msgPort,           row,   0, 1, 2 );
   details->addWidget( le_msgPort,        row++, 2, 1, 2 );

   // Row 4
   name        = us_label( tr( "Instrument DB Name:" ) );
   le_name             = us_lineedit( def_name, 0 );
   details->addWidget( name,           row,   0, 1, 2 );
   details->addWidget( le_name,        row++, 2, 1, 2 );

   // Row 5
   user        = us_label( tr( "Instrument DB Username:" ) );
   le_user             = us_lineedit( def_user, 0 );
   details->addWidget( user,           row,   0, 1, 2 );
   details->addWidget( le_user,        row++, 2, 1, 2 );

   // Row 6
   pasw        = us_label( tr( "Instrument DB Password:" ) );
   le_pasw             = us_lineedit( def_pasw, 0 );
   le_pasw->setEchoMode( QLineEdit::Password );
   details->addWidget( pasw,           row,   0, 1, 2 );
   details->addWidget( le_pasw,        row++, 2, 1, 2 );

   //Row 6a
   bn_chromoab   = us_banner( tr( "Chromatic Aberration Information" ) );
   details->addWidget( bn_chromoab,      row++, 0, 1, 4 );

   //Row 6b
   lb_radcalwvl  = us_label( tr( "Radial Calibration Wavelength:" ) );
   ct_radcalwvl          = us_counter( 2,   190,   800,  280 );
   ct_radcalwvl ->setSingleStep( 1 );
   details->addWidget( lb_radcalwvl,    row,   0, 1, 2 );
   details->addWidget( ct_radcalwvl,    row++, 2, 1, 2 );
   
   //Row 6c
   pb_loadchromo     = us_pushbutton( tr( "Load Chromatic Aberration Array" ) );
   le_chromofile          = us_lineedit( "", 0, true );
   details->addWidget( pb_loadchromo,    row,   0, 1, 2 );
   details->addWidget( le_chromofile,    row++, 2, 1, 2 );
   connect( pb_loadchromo,     SIGNAL( clicked()          ), 
              this,            SLOT(   load_chromo()    ) ); 

   // Row 7
   QLabel* bn_optsys   = us_banner( tr( "Installed Optical Systems" ) );
   bn_optsys->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   details->addWidget( bn_optsys,      row++, 0, 1, 4 );

   // Rows 8,9,10
   QStringList osyss;
   osyss << "UV/visible"
         << "Rayleigh Interference"
         << "Fluorescense"
         << "(not installed)";
   QLabel* lb_os1      = us_label( tr( "Op Sys1:" ) );
   QLabel* lb_os2      = us_label( tr( "Op Sys2:" ) );
   QLabel* lb_os3      = us_label( tr( "Op Sys3:" ) );
   cb_os1              = us_comboBox();
   cb_os2              = us_comboBox();
   cb_os3              = us_comboBox();
   cb_os1->addItems( osyss );
   cb_os2->addItems( osyss );
   cb_os3->addItems( osyss );
   details->addWidget( lb_os1,         row,   0, 1, 1 );
   details->addWidget( cb_os1,         row++, 1, 1, 3 );
   details->addWidget( lb_os2,         row,   0, 1, 1 );
   details->addWidget( cb_os2,         row++, 1, 1, 3 );
   details->addWidget( lb_os3,         row,   0, 1, 1 );
   details->addWidget( cb_os3,         row++, 1, 1, 3 );
   cb_os1->setCurrentIndex( 0 );
   cb_os2->setCurrentIndex( 1 );
   cb_os3->setCurrentIndex( 3 );
      
   topbox->addLayout( details );

   //Pushbuttons
   row = 0;
   QGridLayout* buttons = new QGridLayout();

   pb_testconn = us_pushbutton( tr( "Test Connectivity" ) );
   pb_testconn->setEnabled( true );
   connect( pb_testconn, SIGNAL( clicked( ) ), this, SLOT( test_connectivity( ) ) );
   buttons->addWidget( pb_testconn, row++, 0, 1, 2 );
   
   pb_save = us_pushbutton( tr( "Save Entry" ) );
   pb_save->setEnabled( true );
   connect( pb_save, SIGNAL( clicked( ) ), this, SLOT( save_new( ) ) );
   buttons->addWidget( pb_save, row, 0, 1, 1 );
   pb_save->setEnabled(false);

   pb_cancel = us_pushbutton( tr( "Cancel" ) );
   pb_cancel->setEnabled( true );
   connect( pb_cancel,      SIGNAL( clicked()  ),
	    this,           SLOT  ( cancel() ) );
   buttons->addWidget( pb_cancel, row++, 1, 1, 1 );

   topbox->addLayout( buttons );

   setMinimumSize( 450 , 400 );
   adjustSize();

   fillGui();
}


bool US_NewXpnHostDB::test_connectivity( void )
{
   QString xpnhost = le_host->text();
   QString xpnport = le_port->text();
   QString dbname  = le_name->text();
   QString dbuser  = le_user->text();
   QString dbpasw  = le_pasw->text();
   QString xpndesc = le_description->text();
qDebug() << "test_connect: dbpasw" << dbpasw;

   if ( xpnhost.isEmpty()  ||  xpnport.isEmpty()  ||
        dbuser .isEmpty()  ||  dbpasw .isEmpty() )
     {
       QMessageBox::warning( this,
			     tr( "Missing Data" ),
			     tr( "Please fill in all fields before testing the connection." ) );
       
       return false;
     }
   
   US_XpnData* xpn_data = new US_XpnData();
   int ixpport     = xpnport.toInt();

qDebug() << "test_connect: (2)dbpasw" << dbpasw;
   bool ok         = xpn_data->connect_data( xpnhost, ixpport, dbname,
                                             dbuser, dbpasw );

   xpn_data->close();
   //delete xpn_data;

   if ( ok )
     {
       QMessageBox::information( this,
				 tr( "OptimaHost Connection" ),
				 tr( "The connection was successful." ) );
       
       pb_save->setEnabled( true );
     }
   else
     {
       QMessageBox::warning( this,
			     tr( "OptimaHost Connection" ),
			     tr( "The connection failed.\n" ) + xpn_data->lastError() );
       
       //pb_save->setEnabled( true );
     }
   delete xpn_data;
   
   return ok;
   
}

void US_NewXpnHostDB::changeType( int ndx )
{
   cb_type->setCurrentIndex( ndx );
   QString ctype        = cb_type->currentText();

   if ( ctype.contains("non-Optima") )
     {
       le_description->setPlaceholderText("");
       
       host   ->setVisible( false );
       le_host->setVisible( false );
       port   ->setVisible( false );
       le_port->setVisible( false );

       msgPort->setVisible( false );
       le_msgPort->setVisible( false );
       
       name->setVisible( false );
       le_name->setVisible( false );
       user   ->setVisible( false );
       le_user->setVisible( false );
       pasw   ->setVisible( false );
       le_pasw->setVisible( false );
       bn_chromoab->setVisible( false );
       pb_loadchromo->setVisible( false );
       lb_radcalwvl->setVisible( false );
       ct_radcalwvl->setVisible( false );
       le_chromofile->setVisible( false );
       pb_testconn->setVisible( false );

       pb_save->setEnabled(true);

       nonOptima_selected = true;
     }

   else
     {
       le_description->setPlaceholderText("SYNTAX: 'Optima #'");
       host   ->setVisible( true );
       le_host->setVisible( true );
       port   ->setVisible( true );
       le_port->setVisible( true );
       
       msgPort   ->setVisible( true );
       le_msgPort->setVisible( true );
       
       name->setVisible( true );
       le_name->setVisible( true );
       user   ->setVisible( true );
       le_user->setVisible( true );
       pasw   ->setVisible( true );
       le_pasw->setVisible( true );
       bn_chromoab->setVisible( true );
       pb_loadchromo->setVisible( true );
       lb_radcalwvl->setVisible( true );
       ct_radcalwvl->setVisible( true );
       le_chromofile->setVisible( true );

       nonOptima_selected = false;
     }

   this->adjustSize();
}



// Load Chromo Array
void US_NewXpnHostDB::load_chromo( void )
{
  QStringList files;
  QFile f;
  
  QFileDialog dialog (this);
  dialog.setNameFilter(tr("Text files (*.[Tt][Xx][Tt] *.[Cc][Ss][Vv] *.[Dd][Aa][Tt]);;All files (*)"));
    
  dialog.setFileMode(QFileDialog::ExistingFile);
  dialog.setViewMode(QFileDialog::Detail);

  QString work_dir_data  = US_Settings::etcDir();
  dialog.setDirectory(work_dir_data);
  
  if(dialog.exec())
    {
      files = dialog.selectedFiles();
      readingChromoArrayFile(files[0]);
    }

  if ( corr_lambda.size()!=0 )
    le_chromofile->setText("Uploaded");
}

//Read Chromatic Aberration Array from uploaded file 
void US_NewXpnHostDB::readingChromoArrayFile( const QString &fileName )
{
  QString str1, str2;
  float temp_x, temp_y;
  QStringList strl;
  corr_lambda.clear();
  corr_value.clear();
  
  if(!fileName.isEmpty())
    {
      QFile f(fileName);
      
      if(f.open(QIODevice::ReadOnly | QIODevice::Text))
	{
	  int i=190;
	  QTextStream ts(&f);
	  while(!ts.atEnd())
	    {
	      str1 = ts.readLine();
	      str1 = str1.simplified();

	      if ( str1.isEmpty() || str1.startsWith( "#" ) ) // <---- add more conditions if needed
		continue;

	      str1 = str1.replace("\"", " ");
	      str1 = str1.replace(",", " ");

	      //qDebug() << str1;
	      
	      strl = str1.split(QRegExp("[\r\n\t ,]+"));
	      temp_x = strl.at(0).trimmed().toFloat();
	      temp_y = strl.at(1).trimmed().toFloat();

	      qDebug() << temp_x << "," << temp_y;

	      corr_lambda.push_back( double(temp_x) );
	      corr_value.push_back( double(temp_y) );
	      
	      i++;
	    }

	  
	  if (i != 801)
	    {
	      corr_lambda.clear(); 
	      corr_value.clear();
	      corr_lambda = corr_lambda_current;
	      corr_value = corr_value_current;
	      QMessageBox::warning( this,
				    tr( "Incorrect File Format..." ),
				    tr( "The wavelength correction file\n"
					"is incorrectly formatted or contains invalid data.\n"
					"DB entry for chromatic aberration is not changed." ) );
	      
	    }
	}
    }
  
  qDebug() << "Upload Finished, corr_lambda.size(), corr_lambda_current.size(): "  <<  corr_lambda.size() << ", " << corr_lambda_current.size();
}

//Read Existing Chromatic Aberration Array from DB 
void US_NewXpnHostDB::readingChromoArrayDB( )
{
  corr_lambda.clear();
  corr_value.clear();
  
  QString chromoArrayString;
  QStringList strl;
  chromoArrayString = instrumentedit["chromoab"].trimmed();
  strl = chromoArrayString.split(QRegExp("[\r\n\t ]+"));

  foreach (QString str, strl)
    {
      str.remove("(");
      str.remove(")");

      corr_lambda.push_back( double( str.split(",")[0].trimmed().toFloat() ) );
      corr_value.push_back( double( str.split(",")[1].trimmed().toFloat() ) );
      
      qDebug() << str.split(",")[0].trimmed() << " " << str.split(",")[1].trimmed();
    }

  corr_lambda_current = corr_lambda;
  corr_value_current = corr_value;
}

void US_NewXpnHostDB::shiftChromoArray( double val )
{
  ChromoArrayList.clear();
  double shift = 0;
  for (int i = 0; i < corr_lambda.size(); ++i)
    if( corr_lambda[i] == val )
      shift = corr_value[i];

  for (int i = 0; i < corr_value.size(); ++i)
    {
      corr_value[i] -= shift;
      ChromoArrayList += " (" + QString::number( corr_lambda[i] ) + "," + QString::number( corr_value[i] ) + ") ";
    }

  // //If no data from file was uploaded, generate zero array  -------> DO we need this ?? 
  // if( ChromoArrayList.isEmpty() )
  //   {
  //     for ( int i=190; i<801; ++i )
  // 	ChromoArrayList += " (" + QString::number( i ) + "," + QString::number( 0 ) + ") ";
  //   }
}

// Fill available GUI elements
void US_NewXpnHostDB::fillGui( void )
{
  if ( !instrumentedit["name"].isEmpty()  )
    le_description->setText( instrumentedit["name"] );
  
  if ( !instrumentedit["serial"].isEmpty()  )
    le_serialNumber->setText( instrumentedit["serial"] );
  
  if ( !instrumentedit["host"].isEmpty()  )
    le_host->setText( instrumentedit["host"] );

  if ( !instrumentedit["port"].isEmpty()  )
    le_port->setText( instrumentedit["port"] );

  if ( !instrumentedit["msgPort"].isEmpty()  )
    le_msgPort->setText( instrumentedit["msgPort"] );

  if ( !instrumentedit["dbusername"].isEmpty()  )
    le_user->setText( instrumentedit["dbusername"] );

  if ( !instrumentedit["dbname"].isEmpty()  )
    le_name->setText( instrumentedit["dbname"] );

  if ( !instrumentedit["dbpassw"].isEmpty()  )
    le_pasw->setText( instrumentedit["dbpassw"] );

  
  cb_os1->setCurrentIndex( cb_os1->findText( instrumentedit[ "os1" ] ) );
  cb_os2->setCurrentIndex( cb_os2->findText( instrumentedit[ "os2" ] ) );
  cb_os3->setCurrentIndex( cb_os3->findText( instrumentedit[ "os3" ] ) );

  if ( !instrumentedit["radcalwvl"].isEmpty()  )
    ct_radcalwvl->setValue( double(instrumentedit["radcalwvl"].toFloat()) );
  
  if ( !instrumentedit["chromoab"].isEmpty()  )
    {
      le_chromofile->setText( "Uploaded" );

      readingChromoArrayDB( );
    }

  // Check the type of machine to Edit:
  if ( nonOptima_selected )
    {
      host   ->setVisible( false );
      le_host->setVisible( false );
      port   ->setVisible( false );
      le_port->setVisible( false );

      msgPort   ->setVisible( false );
      le_msgPort->setVisible( false );
      
      name->setVisible( false );
      le_name->setVisible( false );
      user   ->setVisible( false );
      le_user->setVisible( false );
      pasw   ->setVisible( false );
      le_pasw->setVisible( false );
      bn_chromoab->setVisible( false );
      pb_loadchromo->setVisible( false );
      lb_radcalwvl->setVisible( false );
      ct_radcalwvl->setVisible( false );
      le_chromofile->setVisible( false );
    }
  
}


// Entered wavelength(s):  check for syntax
void US_NewXpnHostDB::desc_changed( QString text )
{
  QPalette *palette = new QPalette();
  palette->setColor(QPalette::Text,Qt::black);
  palette->setColor(QPalette::Base,Qt::white);
  le_description->setPalette(*palette);
  le_description->setText(text);
}


// Save New Instrument
void US_NewXpnHostDB::save_new( void )
{
  if ( !nonOptima_selected )
    {
      if ( le_name->text().isEmpty() || le_description->text().isEmpty()
	   || le_host->text().isEmpty() || le_port->text().isEmpty()
	   || le_msgPort->text().isEmpty()
	   || le_user->text().isEmpty() || le_pasw->text().isEmpty()
	   || le_serialNumber->text().isEmpty() ) //|| le_chromofile->text().isEmpty() )
	{
	  QMessageBox::warning( this, tr( "Please provide the missing information:" ),
				tr( "Fill out all fields!"));
	  return;
	}
    }
  else
    {
      if ( le_description->text().isEmpty() || le_serialNumber->text().isEmpty() )
	{
	  QMessageBox::warning( this, tr( "Please provide the missing information:" ),
				tr( "Fill out all fields!"));
	  return;
	}
    }
  

  qDebug() << "Optima Msg Port : " <<  le_msgPort->text();
  
  //RegEx for Optima name:
  QRegExp rx_desc("^(Optima)\\s[1-9]\\d*$");  // e.g. 'Optima 9'
  
  
  if ( !nonOptima_selected )
    {
      if( !rx_desc.exactMatch(le_description->text() ) )
	{
	  QString mtitle_error    = tr( "Error" );
	  QString message_error   = tr( "Syntax error for Optima Host Description!\n\nThe description template is the following:\n  'Optima #' (Optima|space|number)" );
	  QMessageBox::critical( this, mtitle_error, message_error );
	  
	  QPalette *palette = new QPalette();
	  palette->setColor(QPalette::Text,Qt::red);
	  palette->setColor(QPalette::Base,Qt::white);
	  le_description->setPalette(*palette);
	  
	  return;
	}
    }
  else
    {
      if( le_description->text().contains("Optima") ||  le_description->text().contains("optima") )
	{
	  QString mtitle_error    = tr( "Error" );
	  QString message_error   = tr( "Syntax error for Instrument Description: non-Optima type cannot include words 'Optima' or 'optima' in its description!" );
	  QMessageBox::critical( this, mtitle_error, message_error );
	  
	  QPalette *palette = new QPalette();
	  palette->setColor(QPalette::Text,Qt::red);
	  palette->setColor(QPalette::Base,Qt::white);
	  le_description->setPalette(*palette);
	  
	  return;
	}
    }
  

  // Check for instrument name, port & host duplications:
  US_Passwd pw;
  US_DB2* db = use_db ? new US_DB2( pw.getPasswd() ) : NULL;
  QStringList instrumentNames( "" );
  QStringList q( "" );
  QList< int > instrumentIDs;
  q.clear();
  q  << QString( "get_instrument_names" )
     << QString::number( 1 );                    //ALEXEY '1' for the (only) labID

  db->query( q );
  
  while ( db->next() )
    {
      QString name = db->value( 1 ).toString();
      instrumentNames << name;
      int ID = db->value( 0 ).toString().toInt();
      instrumentIDs << ID;
    }

  // Name check
  qDebug() << "Update Instrument: " << update_instrument;
  for (QStringList::iterator it = instrumentNames.begin(); it != instrumentNames.end(); ++it) 
    {
      QString current = *it;

      if ( current == le_description->text() && !update_instrument )
	{
	  QMessageBox::critical( this, tr( "Duplicate Optima Machine Name:" ),
				QString( tr( "The name selected (%1) is currently used by other machine! Please select different name.")
					 .arg(current) ) );
	  return;
	}
    }

  // Host & Port check
  foreach ( int ID, instrumentIDs )
    {
      if ( ID == instrumentedit["ID"].toInt() && update_instrument )
	continue;
      
      q.clear();
      q  << QString( "get_instrument_info_new" )
	 << QString::number( ID );
      db->query( q );
      db->next();

      QString optimaHost       = db->value( 5 ).toString();
      int     optimaPort       = db->value( 6 ).toString().toInt();

      if ( optimaHost == le_host->text() && optimaPort == le_port->text().toInt() && !nonOptima_selected )
	{
	  QMessageBox::critical( this, tr( "Duplicate Optima Machine Connection Info:" ),
				 QString( tr( "Specified combination of the host (%1) and port (%2) is currently used by other machine!  Please edit host and/or port and re-test the connection") 
					  .arg(optimaHost).arg(QString::number(optimaPort)) ) );

	  pb_save->setEnabled(false);
	  return;
	}
    }

  //Chromo Array shifting/forming
  double radwvl = ct_radcalwvl->value();
  shiftChromoArray( radwvl );
  
  
  QMap <QString, QString> newInstrument;
  newInstrument[ "name"]        = le_description->text();
  newInstrument[ "optimaHost" ] = le_host->text();
  newInstrument[ "optimaPort" ] = le_port->text();
  newInstrument[ "optimaDBname" ]  = le_name->text();
  newInstrument[ "optimaDBusername" ] = le_user->text();
  newInstrument[ "optimaDBpassw" ] = le_pasw->text();

  newInstrument[ "radCalWvl" ] = QString::number(radwvl);
  newInstrument[ "chromoArray" ] = ChromoArrayList;
    
  newInstrument[ "serialNumber"] = le_serialNumber->text();
  newInstrument[ "labID"] = QString::number(1);

  newInstrument[ "os1" ] = cb_os1->currentText();
  newInstrument[ "os2" ] = cb_os2->currentText();
  newInstrument[ "os3" ] = cb_os3->currentText();

  newInstrument[ "msgPort" ] = le_msgPort->text();
  
  emit accepted( newInstrument );

  close();
}

// read Instrument names/info from DB
void US_NewXpnHostDB::cancel( void )
{
  emit editnew_cancelled();
  close();
}
