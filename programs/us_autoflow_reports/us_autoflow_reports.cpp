#include "us_autoflow_reports.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_protocol_util.h"


// Constructor
US_Reports_auto::US_Reports_auto() : US_Widgets()
{
  setWindowTitle( tr( "Autoflow Reports"));
                       
  //setPalette( US_GuiSettings::frameColor() );
  
  panel  = new QVBoxLayout( this );
  panel->setSpacing        ( 2 );
  panel->setContentsMargins( 2, 2, 2, 2 );

  QLabel* lb_hdr1          = us_banner( tr( "Reports for All Triples Analysed" ) );
  lb_hdr1->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
  panel->addWidget(lb_hdr1);
    
  genL   = new QGridLayout();
  //genL->setSpacing        ( 2 );
  //genL->setContentsMargins( 2, 2, 2, 2 );

  QLabel* lb_triple   = us_banner( tr( "Triple" ) );
  QLabel* lb_passed   = us_banner( tr( "Analysis Convergence" ) );
  QLabel* lb_action1  = us_banner( tr( "View" ) );
  QLabel* lb_action2  = us_banner( tr( "Download" ) );
  lb_triple->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
  lb_passed->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
  lb_action1->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
  lb_action2->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );

  int row = 1;
  genL->addWidget( lb_triple,  row,    0, 1, 3 );
  genL->addWidget( lb_passed,  row,    3, 1, 3 );
  genL->addWidget( lb_action1, row,    6, 1, 3 );
  genL->addWidget( lb_action2, row++,  9, 1, 3 );

  //panel->addLayout( genL );

  setMinimumSize( 950, 450 );
  adjustSize();
   
  // // ---- Testing ----
  // QMap < QString, QString > protocol_details;
  // protocol_details[ "aprofileguid" ] = QString("d13ffad0-6f27-4fd8-8aa0-df8eef87a6ea");
  // protocol_details[ "protocolName" ] = QString("alexey-abs-itf-test1");
  // protocol_details[ "invID_passed" ] = QString("12");
  
  
  // initPanel( protocol_details );

  // -----------------

}

//init correct # of us_labels rows based on passed # stages from AProfile
void US_Reports_auto::initPanel( QMap < QString, QString > & protocol_details )
{

  Array_of_triples.clear();
  
  AProfileGUID       = protocol_details[ "aprofileguid" ];
  ProtocolName_auto  = protocol_details[ "protocolName" ];
  invID              = protocol_details[ "invID_passed" ].toInt();


  qDebug() << "Reading protocol's triple data: ";
  read_protocol_data_triples();
   
  //Generate GUI
  QFont sfont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() - 1 );
  QFontMetrics fmet( sfont );

  int triple_name_width;
  int max_width = 0;

  int row = 2;
  for ( int i=0; i<Array_of_triples.size(); ++i )
    {
      QString triple_curr = Array_of_triples[i];
      triple_name_width = fmet.width( triple_curr );

      qDebug() << "Triple name: " << triple_curr;

      QLabel* lb_triple_name = us_label( triple_curr );
      
      QLineEdit* le_state    = us_lineedit( "", 0, true );
      le_state->setAlignment(Qt::AlignHCenter);
      QPalette orig_pal = le_state->palette();
      if ( i != Array_of_triples.size() - 1 && i != Array_of_triples.size() - 4 )
	{
	  le_state->setText("PASSED");
	  QPalette *new_palette = new QPalette();
	  new_palette->setColor(QPalette::Text, Qt::darkGreen);
	  new_palette->setColor(QPalette::Base, orig_pal.color(QPalette::Base));
	  le_state->setPalette(*new_palette);
	}
      else
	{
	  le_state->setText("FAILED");
	  QPalette *new_palette = new QPalette();
	  new_palette->setColor(QPalette::Text,Qt::red);
	  new_palette->setColor(QPalette::Base, orig_pal.color(QPalette::Base));
	  le_state->setPalette(*new_palette);
	}
      
      QPushButton * pb_view_file     = us_pushbutton( tr( "View File" ) );
      QPushButton * pb_download_file = us_pushbutton( tr( "Download" ) );

      // connect( pb_view_file,   SIGNAL( clicked()    ),
      // 	       this,          SLOT  ( view() ) );
      
      // connect( pb_download_file, SIGNAL( clicked()    ),
      // 	       this,          SLOT  ( download() ) );

      QString strow   = QString::number( i );
      lb_triple_name  ->setObjectName( strow + ": triple_name" );
      le_state        ->setObjectName( strow + ": state" );
      pb_view_file    ->setObjectName( strow + ": view" );
      pb_download_file->setObjectName( strow + ": download" );
       
      genL->addWidget( lb_triple_name,   row,    0, 1, 3 );
      genL->addWidget( le_state,         row,    3, 1, 3 );
      genL->addWidget( pb_view_file,     row,    6, 1, 3 );
      genL->addWidget( pb_download_file, row++,  9, 1, 3 );

    }

  panel->addLayout( genL );
  panel->addStretch();

}



//read from protocol all triples: if Interference - just one wvl (660); if UV/vis - read all wvl/channel pairs and store
void US_Reports_auto::read_protocol_data_triples()
{
  // Check DB connection
  US_Passwd pw;
  QString masterPW = pw.getPasswd();
  US_DB2 db( masterPW );
  
  if ( db.lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
			    tr( "Read protocol: Could not connect to database \n" ) + db.lastError() );
      return;
    }

  QString xmlstr( "" );
  US_ProtocolUtil::read_record_auto( ProtocolName_auto, invID,  &xmlstr, NULL, &db );
  QXmlStreamReader xmli( xmlstr );

  while( ! xmli.atEnd() )
    {
      xmli.readNext();
      
      if ( xmli.isStartElement() )
	{
	  QString ename   = xmli.name().toString();

	  if ( ename == "optics" )
	    read_protoOptics( xmli );
	  else if ( ename == "ranges" )
	    read_protoRanges( xmli );
	}
    }
}

//read protoOptics
bool US_Reports_auto::read_protoOptics( QXmlStreamReader& xmli )
{
  while( ! xmli.atEnd() )
   {
      QString ename   = xmli.name().toString();

      if ( xmli.isStartElement() )
      {
         if ( ename == "optical_system" )
         {
	   QXmlStreamAttributes attr = xmli.attributes();
            QString channel   = attr.value( "channel" ).toString();
            QString scan1     = attr.value( "scan1"   ).toString();
            QString scan2     = attr.value( "scan2"   ).toString();
            QString scan3     = attr.value( "scan3"   ).toString();

	    QStringList channel_parts = channel.split( "," );
	    QString channel_name = channel_parts[0];
	    //channel_name.replace( " ", "" );

	    if ( !scan2.isEmpty() && channel.contains("sample") )  // Interference
	      {
		QString channel_append(" / Interference");
		channel_name += channel_append;

		//Collect only non-UV/vis channels (Interference or fluorecsence)
		Array_of_triples.push_back( channel_name );
	      }
	 }
      }

      bool was_end    = xmli.isEndElement();   // Just read was End of element?
      xmli.readNext();                         // Read the next element

      if ( was_end  &&  ename == "optics" )    // Break after "</optics>"
         break;
   }

   return ( ! xmli.hasError() );
}

//read protoRanges
bool US_Reports_auto::read_protoRanges( QXmlStreamReader& xmli )
{
  QString channel_name;
  
  while( ! xmli.atEnd() )
   {
      QString ename   = xmli.name().toString();
        
      if ( xmli.isStartElement() )
      {
         if ( ename == "range"  ||  ename == "spectrum" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            QString channel           = attr.value( "channel" )     .toString();

	    QStringList channel_parts = channel.split( "," );
	    channel_name = channel_parts[0];
          }

         else if ( ename == "wavelength"  ||  ename == "point" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            QString wvl = attr.value( "lambda" ).toString();

	    QString channel_name_wvl = channel_name;

	    channel_name_wvl.append(QString(" / ") + wvl);

	    Array_of_triples.push_back( channel_name_wvl );
         }
      }

      // else if ( xmli.isEndElement()  &&
      //          ( ename == "range"  ||  ename == "spectrum" ) )
      // {
      //    chrngs << rng;
      //    nranges++;
      // }

      bool was_end    = xmli.isEndElement();  // Just read was End of element?
      xmli.readNext();                        // Read the next element

      if ( was_end )
      {
         if ( ename == "ranges"  ||          // Break after "</ranges>"
              ename == "spectra" )           // Break after "</spectra>"
           break;
      }
   }

   return ( ! xmli.hasError() );
}
