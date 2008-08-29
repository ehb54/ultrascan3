#include "../include/us_db_rst_montecarlo.h"
#include "../include/us_db_tbl_investigator.h"
#include "../include/us_tar.h"
#include "../include/us_gzip.h"

#include <time.h>

#ifdef WIN32
	#include <direct.h>
	#define chdir _chdir
#endif

US_DB_RST_Montecarlo::US_DB_RST_Montecarlo( QWidget* p, const char* name ) : 
	US_DB_Widgets( p, name )
{

  // Initialize class variables
	
	item_projectID   = NULL;
	item_projectName = NULL;
	from_HD          = false;
	from_DB          = false;
	query_flag       = false;
	projectID        = 0;
	investigatorID   = 0;
	runrequestID     = 0;

	lbl_blank = banner( "Monte Carlo Project Results" );

	pb_hd = pushbutton( "Select data from HD" );
	connect( pb_hd, SIGNAL( clicked() ), SLOT( load_HD() ) );

	pb_db = pushbutton( "Query Data from DB" );
	connect( pb_db, SIGNAL( clicked() ), SLOT( load_DB() ) );

	pb_retrieve = pushbutton( "Retrieve data from DB", false );
	connect( pb_retrieve, SIGNAL( clicked() ), SLOT( retrieve() ) );

	pb_investigator = pushbutton( "Select Investigator" );
	connect( pb_investigator, SIGNAL( clicked() ), SLOT( sel_investigator() ) );

	lbl_investigator = textlabel( " Not Selected" );
	lbl_investigator->setAlignment( AlignLeft | AlignVCenter );

	pb_runrequest = pushbutton( "Select Run Request" );
	connect( pb_runrequest, SIGNAL( clicked() ), SLOT( sel_runrequest() ) );
	
	lbl_runrequest = textlabel( " Not Selected" );
	lbl_runrequest->setAlignment( AlignLeft | AlignVCenter );

	lbl_instr = label( "Doubleclick on item to select:", -1 );

	lb_result = listbox( "result" );
	lb_result->setMinimumHeight( 300 );
	lb_result->setMinimumWidth ( 500 );
	connect( lb_result, SIGNAL( selected( int ) ), SLOT( select_result( int ) ) );

	lbl_item = label( " not selected", -1 );
	lbl_item->setAlignment( AlignLeft | AlignVCenter );

	pb_save = pushbutton( "Save data to DB", false );
	connect( pb_save, SIGNAL( clicked() ), SLOT( save_db() ) );

	pb_display = pushbutton( "Show Selected Result", false );
	connect( pb_display, SIGNAL( clicked() ), SLOT( display() ) );

	pb_delete = pushbutton( "Delete data from DB", false );
	connect( pb_delete, SIGNAL( clicked() ), SLOT( check_permission() ) );

	pb_reset = pushbutton( "Reset" );
	connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );

	pb_help = pushbutton( "Help" );
	connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

	pb_close = pushbutton( "Close" );
	connect( pb_close, SIGNAL( clicked() ), SLOT( quit() ) );

	global_Xpos += 30;
	global_Ypos += 30;
	setup_GUI();
}

void US_DB_RST_Montecarlo::setup_GUI()
{
	unsigned int j    = 0;
	QGridLayout* Grid = new QGridLayout( this, 12, 2, 4, 2 );

	Grid->addMultiCellWidget( lbl_blank,   j, j, 0, 1 ); j++;
	Grid->addMultiCellWidget( pb_hd,       j, j, 0, 1 ); j++;
	Grid->addMultiCellWidget( pb_db,       j, j, 0, 1 ); j++;
	Grid->addMultiCellWidget( pb_retrieve, j, j, 0, 1 ); j++;
	
	Grid->addWidget( pb_investigator,  j, 0 );
	Grid->addWidget( lbl_investigator, j, 1 ); j++;
	Grid->addWidget( pb_runrequest,    j, 0 );
	Grid->addWidget( lbl_runrequest,   j, 1 ); j++; 
	
	Grid->addMultiCellWidget( lbl_instr, j, j, 0, 1 ); j++;
	Grid->addMultiCellWidget( lb_result, j, j, 0, 1 ); j++;
	Grid->addMultiCellWidget( lbl_item,  j, j, 0, 1 ); j++;
	
	Grid->addWidget( pb_save,    j, 0 );
	Grid->addWidget( pb_display, j, 1 ); j++;
	Grid->addWidget( pb_delete,  j, 0 );
	Grid->addWidget( pb_reset,   j, 1 ); j++;
	Grid->addWidget( pb_help,    j, 0 );
	Grid->addWidget( pb_close,   j, 1 );
	Grid->activate();
}

/*!  Open US_DB_TblInvestigator interface for selecting investigator.  */
void US_DB_RST_Montecarlo::sel_investigator()
{
	US_DB_TblInvestigator* investigator_dlg = new US_DB_TblInvestigator();
	
	investigator_dlg->setCaption( "Investigator Information" );
	investigator_dlg->pb_exit->setText( "Accept" );
	investigator_dlg->show();
	
	connect( investigator_dlg, SIGNAL( valueChanged           ( QString, int ) ), 
			                       SLOT  ( update_investigator_lbl( QString, int ) ) );
}

/*!  Update display with the selected investigator information.
\param Display a string variable for show investigator info.
\param InvID a integer variable for DB table: <tt>tblInvestigators</tt> index.
*/
void US_DB_RST_Montecarlo::update_investigator_lbl( QString Display, int InvID )
{
	investigatorID = InvID;
	QString str    = Display;

	lbl_investigator->setText( str );
	
	if ( str == "" )
	{
		lbl_investigator->setText( " Not Selected" );
	}
}

void US_DB_RST_Montecarlo::sel_runrequest( void )
{
	US_DB_RunRequest* runrequest_dlg = new US_DB_RunRequest();

	runrequest_dlg->setCaption("Run Request Information");
	runrequest_dlg->show();
	
	connect( runrequest_dlg, SIGNAL( issue_RRID           ( int ) ), 
	                         SLOT  ( update_runrequest_lbl( int ) ) );
}

void US_DB_RST_Montecarlo::update_runrequest_lbl ( int RRID )
{
	runrequestID = RRID;
	lbl_runrequest->setNum( runrequestID );
	
	if ( runrequestID == 0 )
	{
		lbl_runrequest->setText( " Not Selected" );
	}
}

void US_DB_RST_Montecarlo::load_HD()
{
	reset();
	
	fileName = QFileDialog::getOpenFileName( 
			USglobal->config_list.result_dir, "*.Monte-Carlo", 0 );
	
	parameterFile = fileName;
	parameters    = 0;

	QFile projectFile( fileName );
	projectFile.open( IO_ReadOnly );
	
	QTextStream project_ts( &projectFile );
	
	projectName = project_ts.readLine();
	QString str = project_ts.readLine();
	parameters  = str.toInt();

	projectFile.close();
	
	str = "File Path : " + QDir::convertSeparators( fileName );

	lb_result->insertItem( str );
	lbl_item->setText( "Result data loaded from harddrive" );
	
	if ( fileName.isEmpty() )
	{
		QMessageBox::message(
		  tr( "Attention:" ), 
		  tr( "No file Selected.\n" ) );

		return;
	}
	else
	{	
		US_Report_MonteCarlo* report_montecarlo = new US_Report_MonteCarlo( false );
		report_montecarlo->generate( fileName );
		from_HD = true;
	}

	pb_save   ->setEnabled( true );
	pb_display->setEnabled( true );
}

void US_DB_RST_Montecarlo::save_db()
{
	if ( ! from_HD )
	{
		QMessageBox::message(
		  tr("Attention:"), 
		  tr( "Please load result data from harddrive first.\n" ) );

		return;
	}

	if ( ! investigatorID )
	{
		QMessageBox::message(
		  tr( "Attention:"), 
		  tr( "Please select investigator first.\n" ) );

		return;
	}

	db_connect();
	
	if ( ! insertData() )
	{
		QMessageBox::message(
		  tr( "Attention:" ), 
			tr( "There was an error during the\n"
			    "uploading of the Monte Carlo data.\n"
			    "Terminating." ) );
      
			exit( -1 );
	}
}

void US_DB_RST_Montecarlo::clearTmpDir()
{
	QDir    temp_dir;
	QString tempDirString =  USglobal->config_list.tmp_dir;

	temp_dir.setPath( tempDirString );

	if ( temp_dir.exists() )
	{
		temp_dir.setNameFilter( "*.*" );
		QStringList entries = temp_dir.entryList();
		
		QStringList::Iterator it;
		for ( it = entries.begin(); it != entries.end(); ++it ) 
		{
			temp_dir.remove( *it );
		}
	}
	else
	{
		temp_dir.mkdir( tempDirString );
	}
}

bool US_DB_RST_Montecarlo::insertData()
{
	QString          title = "Saving data to the database...";
	QProgressDialog* pd    = progressdialog( title,	"pd", 8 );

	pd->setMinimumDuration( 0 );
	pd->setProgress       ( 1 );

	// Clear temp directory
	pd->setLabelText( "Clearing temporary directory..." );
	clearTmpDir();
	
	// Archiving and gzipping the Monte Carlo report
	
	QString tempDir     = USglobal->config_list.tmp_dir + "/";

	QString reportDir   = USglobal->config_list.html_dir + "/";
	QString tarfile     = projectName + "_report.tar";
	
	QStringList entries;
 	entries << projectName + ".mc";

	pd->setLabelText( "Archiving the Monte Carlo report..." );

	pd->setProgress( 2 );
	if ( ! create_tar( reportDir, tarfile, entries ) )
	{
		pd->close();
		return false;
	}

	US_Gzip gzip;
	int     ret;

	chdir( reportDir.ascii() );
	
	pd->setProgress( 3 );
	if ( ( ret = gzip.gzip( tarfile ) ) != GZIP_OK )
	{
		pd->close();
		QMessageBox::message(
			tr( "UltraScan gzip Error:" ),
			tr( gzip.explain( ret ) ) );

		// Remove the tar file
		QFile::remove( reportDir + tarfile );
		return false;
	}

	move_file(  reportDir + tarfile + ".gz", tempDir + tarfile + ".gz" );

	// Archiving and gzipping the Monte Carlo results

	QString resultDir = USglobal->config_list.result_dir + "/";
	tarfile           = projectName + "_result.tar";

	entries.clear();
	entries << projectName + ".mc" << projectName + ".Monte-Carlo";

	pd->setProgress( 4 );
	if ( ! create_tar( resultDir, tarfile, entries ) ) 
	{
		pd->close();
		return false;
	}

	chdir( resultDir.ascii() );
	
	pd->setProgress( 5 );
	if ( ( ret = gzip.gzip( tarfile ) ) != GZIP_OK )
	{
		pd->close();
		QMessageBox::message(
			tr( "UltraScan gzip Error:" ),
			tr( gzip.explain( ret ) ) );

		// Remove the tar file
		QFile::remove( resultDir + tarfile );
		return false;
	}

	move_file(  resultDir + tarfile + ".gz", tempDir + tarfile + ".gz" );

	// Creating tar archive by combining the sub-archives
	entries.clear();
	entries << projectName + "_report.tar.gz" << projectName + "_result.tar.gz";

	tarfile = "montecarlo.tar";
	pd->setProgress( 5 );
	if ( ! create_tar( tempDir, tarfile, entries ) )
	{
		pd->close();
		return false;
	}

	// Insert data into database

	pd->setLabelText( "Uplading Monte Carlo descriptions into database..." );
	db_connect();

	QSqlCursor cur( "MonteCarloResult" );
	cur.setMode( QSqlCursor::Insert );
	QSqlRecord* buffer = cur.primeInsert();
	montecarloID       = get_newID( "MonteCarloResult", "montecarloID" );
							      
	buffer->setValue( "montecarloID",   montecarloID   );
	buffer->setValue( "InvestigatorID", investigatorID );
													      
	if ( runrequestID > 0 )
	{
	  buffer->setValue("RunRequestID", runrequestID);
	}
	
	buffer->setValue( "parameter_number", parameters  );
	buffer->setValue( "projectName",      projectName );
	cur.insert();

	pd->setProgress( 6 );
	qApp->processEvents();

	QSqlCursor cur_f( "MonteCarloData" );
	cur_f.setMode( QSqlCursor::Insert );
	QSqlRecord* buffer_f = cur_f.primeInsert();
	
	buffer_f->setValue( "montecarloID", montecarloID );

	write_blob( reportDir + projectName + ".res", buffer_f, "report" );
	write_blob( parameterFile, buffer_f, "parameterFile" );
	write_blob( tempDir + "montecarlo.tar", buffer_f, "data" );


	int result = cur_f.insert();
	
	if ( result <= 0 )
	{
		pd->close();
		QSqlError err = cur.lastError();

		QMessageBox::message(
			tr( "Attention:" ), 
			tr("Saving Monte Carlo data to DB table 'MonteCarloData' failed.\n"
			   "Error message from MySQL:\n\n") + err.text());

		return false;
	}

	pd->setProgress( 8 );
	pd->setLabelText( "Updating database records..." );
	
	qApp->processEvents();
	
	QString query;

	if ( runrequestID > 0 )  // Update tblResult
	{
		int resultID = 0;
		query.sprintf( "SELECT ResultID FROM tblResult "
		               "WHERE RunRequestID = %d;", runrequestID );

		QSqlQuery chk( query );

		if ( chk.isActive() )
		{
			if ( chk.next() )
			{
				resultID = chk.value( 0 ).toInt();
			}
		}

		QSqlQuery q;

		if ( resultID > 0 ) // row exists and update equilprojectID
		{
			query.sprintf( "UPDATE tblResult SET "
			               "InvestigatorID = %d, MontecarloID = %d "
			               "WHERE ResultID = %d;", 
			               investigatorID, montecarloID, resultID );

			q.exec( query );
		}
		else  // Create new row
		{
			resultID = get_newID("tblResult","ResultID");
			query = "INSERT INTO tblResult "
			        "(ResultID, InvestigatorID, RunRequestID, MontecarloID) "
			        "VALUES("
			      +  QString::number( resultID ) + ", "
			      +  QString::number( investigatorID ) +", "
			      +  QString::number( runrequestID )   + ", "
			      +  QString::number( montecarloID )   + ");";

			bool flag = q.exec( query );

			if( ! flag )
			{
				pd->close();
				QMessageBox::message(
					tr("Attention:"), 
					tr("Saving result data to database failed.\n"));

				return false;
			}
		}
	}

	// Clean up temp directory
	clearTmpDir();
	pd->close();

	QMessageBox::message(
		tr( "Success" ),  
		tr( "The Monte Carlo Data were successfully\n"
		    "uploaded to the database.\n"));

	return true;
}

bool US_DB_RST_Montecarlo::create_tar( const QString& dirString, 
                                       const QString& tarfile,
                                       QStringList&   entries )
{
	US_Tar tar;

	chdir( dirString.ascii() );
	qApp->processEvents();

	int ret = tar.create( tarfile, entries );

	if ( ret != TAR_OK )
	{
		QMessageBox::message(
			tr( "UltraScan tar Error:" ),
			tr( tar.explain( ret ) ) );

		return false;
	}

	return true;
}

void US_DB_RST_Montecarlo::move_file( const QString& src, const QString& dest )
{
	int result = Move( src, dest );

	if ( result < 0 )
	{
		QMessageBox::message(
			tr( "UltraScan Error:" ),
			tr( "Unable to move file to destination."
			    "\nSource     : " ) + QDir::convertSeparators(  src ) +
			tr( "\nDestination: " ) + QDir::convertSeparators( dest ) +
			tr( "\nError Code in Move function : " ) + QString::number( result ) +
			tr( "\nTerminating" ) );

		exit(0);
	}
}

void US_DB_RST_Montecarlo::load_DB()
{
	QString queryStr;
	clear();

	int maxID            = get_newID( "MonteCarloResult", "montecarloID" );
	int count            = 0;

	// Don't leak memory
	if ( item_projectID   != NULL ) delete [] item_projectID;
	if ( item_projectName != NULL ) delete [] item_projectName;

	item_projectID       = new int    [ maxID ];
	item_projectName     = new QString[ maxID ];
	QString* display_Str = new QString[ maxID ];

	if ( investigatorID > 0 )
	{
		queryStr.sprintf( "SELECT montecarloID, projectName "
		                  "FROM MonteCarloResult "
		                  "WHERE InvestigatorID = %d;", investigatorID);
	}
	else
	{
		queryStr = "SELECT montecarloID, projectName FROM MonteCarloResult;";
	}	 

	QSqlQuery query( queryStr );

	if ( query.isActive() )
	{
		while( query.next() )
		{
			item_projectID  [ count ] = query.value( 0 ).toInt();
			item_projectName[ count ] = query.value( 1 ).toString();
			QString projID            = QString::number( item_projectID[ count ] );
			display_Str     [ count ] = "(" +  projID +	"): "
			                          + item_projectName[ count ];
			count++;
		}
	}
	if ( count > 0 )
	{
		lb_result->clear();

		for( int i = 0; i < count; i++ )
		{
			lb_result->insertItem( display_Str[ i ] );
		}

		query_flag = true;
	}
	else
	{
		lb_result->clear();
		lb_result->insertItem( "No data were found in the DB" );
	}

	delete [] display_Str;
}

void US_DB_RST_Montecarlo::select_result( int item )
{
	if ( query_flag )
	{
		montecarloID = item_projectID  [ item ];
		projectName  = item_projectName[ item ];

		QString str = "Selected : (" + QString::number( montecarloID ) + ") "
			+ projectName;
		
		lbl_item->setText( str );
		
		QSqlCursor cur( "MonteCarloResult" );
		
		str.sprintf( "montecarloID = %d", montecarloID );
		cur.select( str );
		
		if ( cur.next() )
		{
			investigatorID = cur.value( "InvestigatorID" ).toInt();
			runrequestID   = cur.value( "RunRequestID"   ).toInt();	 
		}
	
		if ( investigatorID > 0 )
		{
			lbl_investigator->setText( show_investigator( investigatorID ) );
			pb_investigator ->setEnabled( false );
		}

		if ( runrequestID > 0 )
		{
			lbl_runrequest->setNum( runrequestID );
			pb_runrequest ->setEnabled( false );
		}

		from_DB = true;
		pb_retrieve->setEnabled( true );
	  pb_delete  ->setEnabled( true );
	}
	else
	{
		QMessageBox::message(
				"UltraScan Error:", 
				"Unable to find Monte Carlo data in database.");
	}
}

void US_DB_RST_Montecarlo::retrieve()
{
	if ( montecarloID <= 0 || projectName == "" )
	{
		QMessageBox::message(
				tr("Attention:"), 
				tr("You have to select a Monte Carlo analysis\n"
				   "from the database before retrieving it."));
		
		return;
	}

	// Database contents have been queried successfully earlier
	if ( ! query_flag )
	{
		QMessageBox::message(
			tr( "Please note:" ), 
			tr( "Please select a Monte Carlo Analysis before retrieving." ) );

		return;
	}

	QString          title = "Retrieving data from the database...";
	QProgressDialog* pd    = progressdialog( title, "pd", 8 );
	pd->setMinimumDuration( 0 );
	pd->setLabelText      ( "Clearing temporary directory..." );
	pd->setProgress       ( 0 );
	qApp->processEvents();
	
	// Clear temp directory
	clearTmpDir();
	qApp->processEvents();

	// Retrieve Monte Carlo data from the database
	pd->setLabelText( "Downloading Monte Carlo Data..." );
	pd->setProgress ( 1 );
	qApp->processEvents();

	QSqlCursor cursor( "MonteCarloData" );

	// 1st time consuming statement

	//cout << "before sleep" << time(0) << "\n";
	//sleep(5);
	qApp->processEvents();

	//cout << "before select" << time(0) << "\n";

	cursor.select( QString( "montecarloID = %1" ).arg( montecarloID ) );
	//cout << "after select " << time(0) << "\n";

	QString tempDir = USglobal->config_list.tmp_dir + "/";

	if ( cursor.next() )
	{
		pd->setProgress( 2 );
		if ( ! read_blob( "data", cursor, tempDir + "montecarlo.tar" ) )
		{
			pd->close();
			delete pd;
			QMessageBox::message(
				tr( "UltraScan Error:" ), 
				tr( "Unable to retrieve Monte Carlo Data from the database" ) );
		
			return;
		}
	}
	else
	{
		pd->close();
		delete pd;
		QMessageBox::message(
			tr( "UltraScan Error:" ), 
			tr( "Cannot find the selected Monte Carlo archive in the database." ) );
		
		return;
	}

	// Untar the master archive
	pd->setLabelText( 
	  "Extracting Monte Carlo master archive into temporary directory..." );
	pd->setProgress( 3 );
	qApp->processEvents();

	US_Tar  tar;
	QString tarfile = "montecarlo.tar";
	chdir( tempDir.ascii() );

	if ( tar.extract( tarfile ) != TAR_OK )
	{
		pd->close();
		delete pd;
		QMessageBox::message(
			tr( "UltraScan Error:" ), 
			tr( "Unable to extract the Monte Carlo tar archive.\n" ) +
			tempDir + tarfile );

		return;
	}

	/////////////////////
	// gunzip the result tar file
	pd->setLabelText( "Uncompressing Monte Carlo result tar file..." );
	pd->setProgress( 4 );
	qApp->processEvents();
	
	US_Gzip gzip;
	QString targzfile = projectName + "_result.tar.gz";

	if ( gzip.gunzip( targzfile ) != TAR_OK )
	{
		pd->close();
		delete pd;
		QMessageBox::message(
			tr( "UltraScan Error:" ), 
			tr( "Unable to uncompress Monte Carlo result tar archive.\n" +
			tempDir + targzfile ) );

		return;
	}

	// Untar the result tar file
	pd->setLabelText( "Extracting Monte Carlo result tar file..." );
	pd->setProgress( 5 );
	qApp->processEvents();
	
	QString resultDir = USglobal->config_list.result_dir + "/";
	tarfile           = projectName + "_result.tar";

	move_file( tempDir + tarfile, resultDir + tarfile );
	
	chdir( resultDir.ascii() );

	int ret;

	if ( ( ret = tar.extract( tarfile ) ) != GZIP_OK )
	{
		pd->close();
		delete pd;
		QMessageBox::message(
			tr( "UltraScan tar Error:" ),
			tr( tar.explain( ret ) ) );

		// Remove the tar file
		QFile::remove( resultDir + tarfile );
		return;
	}
	
	// Success	
	QFile::remove( resultDir + tarfile );

	////////////////////
	// gunzip the report tar file
	pd->setLabelText( "Uncompressing Monte Carlo report tar file..." );
	pd->setProgress( 6 );
	qApp->processEvents();
	
	targzfile = projectName + "_report.tar.gz";

	chdir( tempDir.ascii() );

	if ( gzip.gunzip( targzfile ) != GZIP_OK )
	{
		pd->close();
		delete pd;
		QMessageBox::message(
			tr( "UltraScan Error:" ), 
			tr( "Unable to uncompress the Monte Carlo report tar archive." ) );

		return;
	}

	// untar the report file
	
	pd->setLabelText( "Extracting Monte Carlo report tar file..." );
	pd->setProgress( 7 );
	qApp->processEvents();
	
	QString reportDir = USglobal->config_list.html_dir + "/";
	tarfile           = projectName + "_report.tar";

	move_file( tempDir + tarfile,  reportDir + tarfile );
	
	chdir( reportDir.ascii() );

	if ( ( ret = tar.extract( tarfile ) ) != GZIP_OK )
	{
		pd->close();
		delete pd;
		QMessageBox::message(
			tr( "UltraScan tar Error:" ),
			tr( tar.explain( ret ) ) );

		// Remove the tar file
		QFile::remove( reportDir + tarfile );
		return;
	}
	
	// Success	
	QFile::remove( reportDir + tarfile );
	clearTmpDir();
	pd->close();
	delete pd;

	QMessageBox::message(
			tr( "Success" ),
			tr( "Retrieval Complete." ) );
}

void US_DB_RST_Montecarlo::display()
{

	US_Report_MonteCarlo* mc;
	mc = new US_Report_MonteCarlo( projectName );
}

/*!  Open US_DB_Admin to check delete permission.  */

void US_DB_RST_Montecarlo::check_permission()
{
	US_DB_Admin* db_admin = new US_DB_Admin( "" );
	db_admin->show();
	connect( db_admin, SIGNAL( issue_permission( bool ) ), 
			               SLOT  ( delete_db       ( bool ) ) );
}

/*!  Delete one selected experimental result table entry.  */
void US_DB_RST_Montecarlo::delete_db( bool permission )
{
	QString str;
	if ( ! permission )
	{
		QMessageBox::message(
				tr("Attention:"), 
				tr("Permission denied, please contact your "
					 "system administrator for help"));
		
		return;
	}

	if ( ! from_DB )
	{
		QMessageBox::message(
				tr("Attention:"), 
				tr("Please first select the record\n"
				   "to be deleted from the database"));
	}
	else
	{
		switch( QMessageBox::information( this, 
					tr("Delete this record?"), 
					tr("Clicking 'OK' will delete the selected result from the database"),
					tr("OK"), tr("CANCEL"),	0, 1 ) )
		{
			case 0:
			{
				QSqlQuery del_res;
				str.sprintf("UPDATE tblResult SET MontecarloID = NULL "
						        "WHERE MontecarloID = %d;", montecarloID );

				del_res.exec(str);
				
				str.sprintf( "DELETE FROM MonteCarloResult "
						         "WHERE montecarloID = %d;", montecarloID);
				
				bool flag = del_res.exec(str);			
				
				// delete all relative data
				if ( flag )
				{
					QSqlQuery del_dat;
					str.sprintf( "DELETE FROM MonteCarloData "
							         "WHERE montecarloID = %d;", montecarloID );
					del_dat.exec( str );	
				}

				reset();
				break;
			}	
			case 1:
				break;
		}
	}
}

void US_DB_RST_Montecarlo::clear()
{
	from_HD      = false;
	from_DB      = false;
	query_flag   = false;
	parameters   = 0;
	montecarloID = 0;
	runrequestID = 0;
	
	pb_investigator->setEnabled( true );
	pb_runrequest  ->setEnabled( true );		
	pb_retrieve    ->setEnabled( false );
	pb_display     ->setEnabled( false );
	pb_delete      ->setEnabled( false );
	pb_save        ->setEnabled( false );
	lbl_runrequest ->setText( " Not Selected" );

	lb_result->clear();
}

void US_DB_RST_Montecarlo::reset()
{
	clear();
	investigatorID = 0;
	lbl_investigator->setText( " Not Selected" );
}

/*! Open a netscape browser to load help page.*/
void US_DB_RST_Montecarlo::help()
{
	US_Help* online_help = new US_Help( this );
	online_help->show_help( "manual/us_db_result_montecarlo.html" );
}

/*! Close the interface.*/
void US_DB_RST_Montecarlo::quit()
{
	QString dirName = projectName + ".mc_db";
	remove_temp_dir( dirName );
	close();
}

/*! This event handler, for event <var>e</var>, receives widget close events. */
void US_DB_RST_Montecarlo::closeEvent( QCloseEvent* e )
{
	QString dirName = projectName + ".mc_db";
	remove_temp_dir( dirName );
	e->accept();
}
