#include "us_analysis_auto.h"
#include "us_settings.h"
#include "us_gui_settings.h"


// Constructor
US_Analysis_auto::US_Analysis_auto() : US_Widgets()
{
  QVBoxLayout* panel  = new QVBoxLayout( this );
  panel->setSpacing        ( 2 );
  panel->setContentsMargins( 2, 2, 2, 2 );

  QLabel* lb_hdr1          = us_banner( tr( "Analsyis Stages" ) );
  QLabel* lb_hdr2          = us_banner( tr( "Queue Message" ) );
  QLabel* lb_hdr3          = us_banner( tr( "Status" ) );
  
  lb_hdr1->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
  lb_hdr2->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
  lb_hdr3->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
  
  QGridLayout* banners = new QGridLayout();
  int row             = 1;
  banners->addWidget( lb_hdr1,         row,   0, 1, 2 );
  banners->addWidget( lb_hdr2,         row,   2, 1, 4 );
  banners->addWidget( lb_hdr3,         row++, 6, 1, 2 );
  
  genL   = new QGridLayout();
  genL->setSpacing        ( 2 );
  genL->setContentsMargins( 2, 2, 2, 2 );

  panel->addLayout(banners);
  genL->setAlignment(Qt::AlignTop);
  panel->addLayout(genL);
  


  // // ---- Testing ----
  // QMap < QString, QString > protocol_details;
  // protocol_details[ "aprofileguid" ] = QString("528f392c-726b-4566-be78-d7ccae2e4408");
  
  // initPanel( protocol_details );

  // // -----------------

}

//init correct # of us_labels rows based on passed # stages from AProfile
void US_Analysis_auto::initPanel( QMap < QString, QString > & protocol_details )
{
  AProfileGUID = protocol_details[ "aprofileguid" ];

  job1run     = false;
  job2run     = false;
  job3run     = false;
  job4run     = false;
  job5run     = false;
  job3auto    = false;

  read_aprofile_data_from_aprofile();

  //Generate GUI

  //basic stage lables
  stage_2dsa       = us_label( tr( "2DSA    (TI noise):" ) );
  stage_2dsa_fm    = us_label( tr( "2DSA-FM (TI+RI noise):" ) );
  stage_fitmen     = us_label( tr( "FITMEN:" ) );
  stage_2dsa_it    = us_label( tr( "2DSA-IT (TI+RI noise):" ) );
  stage_2dsa_mc    = us_label( tr( "2DSA-MC:" ) );
  
  //queue_msg line edits
  queue_msg_2dsa     = us_lineedit(   tr( "(no analysis submitted)" ), 1, true );
  queue_msg_2dsa_fm  = us_lineedit(   tr( "(no analysis submitted)" ), 1, true );
  queue_msg_fitmen   = us_lineedit(   tr( "(no analysis submitted)" ), 1, true );
  queue_msg_2dsa_it  = us_lineedit(   tr( "(no analysis submitted)" ), 1, true );
  queue_msg_2dsa_mc  = us_lineedit(   tr( "(no analysis submitted)" ), 1, true );
  
  QPalette stpal;
  stpal.setColor( QPalette::Text, Qt::white );
  stpal.setColor( QPalette::Base, Qt::blue  );
  queue_msg_2dsa         ->setPalette( stpal );
  queue_msg_2dsa_fm      ->setPalette( stpal );
  queue_msg_fitmen       ->setPalette( stpal );
  queue_msg_2dsa_it      ->setPalette( stpal );
  queue_msg_2dsa_mc      ->setPalette( stpal ); 

  //status line edits
  status_2dsa         = us_lineedit(   tr( "N/A" ), 1, true );
  status_2dsa_fm      = us_lineedit(   tr( "N/A" ), 1, true );
  status_fitmen       = us_lineedit(   tr( "N/A" ), 1, true );
  status_2dsa_it      = us_lineedit(   tr( "N/A" ), 1, true );
  status_2dsa_mc      = us_lineedit(   tr( "N/A" ), 1, true );
  
  int row   = 1;
  if ( job1run )  //2DSA
    {
      genL->addWidget( stage_2dsa,      row,    0, 1, 2 );
      genL->addWidget( queue_msg_2dsa,  row,    2, 1, 4 );
      genL->addWidget( status_2dsa,     row++,  6, 1, 2 );
    }

  if ( job2run )  //2DSA-FM
    {
      genL->addWidget( stage_2dsa_fm,      row,    0, 1, 2 );
      genL->addWidget( queue_msg_2dsa_fm,  row,    2, 1, 4 );
      genL->addWidget( status_2dsa_fm,     row++,  6, 1, 2 );
    }

  if ( job3run )  //FITMEN
    {
      genL->addWidget( stage_fitmen,      row,    0, 1, 2 );
      genL->addWidget( queue_msg_fitmen,  row,    2, 1, 4 );
      genL->addWidget( status_fitmen,     row++,  6, 1, 2 );
    }

  if ( job4run )  //2DSA-IT
    {
      genL->addWidget( stage_2dsa_it,      row,    0, 1, 2 );
      genL->addWidget( queue_msg_2dsa_it,  row,    2, 1, 4 );
      genL->addWidget( status_2dsa_it,     row++,  6, 1, 2 );
    }

  if ( job5run )  //2DSA-MC
    {
      genL->addWidget( stage_2dsa_mc,      row,    0, 1, 2 );
      genL->addWidget( queue_msg_2dsa_mc,  row,    2, 1, 4 );
      genL->addWidget( status_2dsa_mc,     row++,  6, 1, 2 );
    }  
}

//read related AProfile data (stages)
void US_Analysis_auto::read_aprofile_data_from_aprofile()
{
  QString aprofile_xml;
  
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

  // qDebug() << "AProfGUID: " << AProfileGUID;

    
  QStringList qry;
  qry << "get_aprofile_info" << AProfileGUID;
  db.query( qry );
  
  while ( db.next() )
    {
      //currProf.aprofID     = db.value( 0 ).toInt();
      //currProf.aprofname   = db.value( 1 ).toString();
      aprofile_xml         = db.value( 2 ).toString();
    }

  //qDebug() << "aprofile_xml: " <<  aprofile_xml;
  
  if ( !aprofile_xml.isEmpty() )
    {
      QXmlStreamReader xmli( aprofile_xml );
      readAProfile( xmli );
    }
}

//Aprofile stages
bool US_Analysis_auto::readAProfile( QXmlStreamReader& xmli )
{
  while( ! xmli.atEnd() )
    {
      xmli.readNext();
      
      if ( xmli.isStartElement() )
	{
	  QString ename   = xmli.name().toString();
	  
	  if ( ename == "p_2dsa" )
	    {
	      readAProfile_2DSA( xmli );
	    }

	}
    }

  return ( ! xmli.hasError() );
}
	  
// Read all current 2DSA controls from an XML stream
bool US_Analysis_auto::readAProfile_2DSA( QXmlStreamReader& xmli )
{
  while( ! xmli.atEnd() )
    {
      QString ename   = xmli.name().toString();
      
      if ( xmli.isStartElement() )
	{
         if ( ename == "job_2dsa" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            job1run        = bool_flag( attr.value( "run" ).toString() );
            //job1nois       = attr.value( "noise" ).toString();
         }
         else if ( ename == "job_2dsa_fm" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            job2run        = bool_flag( attr.value( "run" ).toString() );
            //job2nois       = attr.value( "noise" ).toString();
            //fitrng         = attr.value( "fit_range" ).toString().toDouble();
            //grpoints       = attr.value( "grid_points" ).toString().toInt();
         }
         else if ( ename == "job_fitmen" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            job3run        = bool_flag( attr.value( "run" ).toString() );
            job3auto       = attr.value( "interactive" ).toString().toInt() == 0;
         }
         else if ( ename == "job_2dsa_it" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            job4run        = bool_flag( attr.value( "run" ).toString() );
            job4nois       = attr.value( "noise" ).toString();
            //rfiters        = attr.value( "max_iterations" ).toString().toInt();
         }
         else if ( ename == "job_2dsa_mc" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            job5run        = bool_flag( attr.value( "run" ).toString() );
            //mciters        = attr.value( "mc_iterations" ).toString().toInt();
         }
      }

      bool was_end    = xmli.isEndElement();  // Just read was End of element?
      xmli.readNext();                        // Read the next element

      if ( was_end  &&  ename == "p_2dsa" )   // Break after "</p_2dsa>"
         break;
   }

   return ( ! xmli.hasError() );	      
}


// Return a flag if an XML attribute string represents true or false.
bool US_Analysis_auto::bool_flag( const QString xmlattr )
{
   return ( !xmlattr.isEmpty()  &&  ( xmlattr == "1"  ||  xmlattr == "T" ) );
}
