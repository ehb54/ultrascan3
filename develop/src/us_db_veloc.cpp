#include "../include/us_db_veloc.h"
#include "../include/us_tar.h"
#include "../include/us_gzip.h"

US_DB_Veloc::US_DB_Veloc( QWidget* p, const char* name ) : 
  US_DB_Widgets( p, name )
{
	// Initialize class variables
	//
	exp_rst.expRstID = -1;
	exp_rst.invID    = -1;
	from_HD          = false;
	from_DB          = false;
	query_flag       = false;
	retrieve_flag    = false;
	del_flag         = false;
	run_id           = "";
	runrequestID     = 0;
	DataID           = 0;
	item_ExpdataID   = NULL;
	item_Description = NULL;

	pb_load = pushbutton( "Load Data from Hard Drive" );
	connect( pb_load, SIGNAL( clicked() ), SLOT( load_HD() ) );

	pb_help = pushbutton( "Help" );
	connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

	pb_close = pushbutton( "Close" );
	connect( pb_close, SIGNAL( clicked() ), SLOT( quit() ) );

	pb_load_db = pushbutton( "Query Result from Database" );
	connect( pb_load_db, SIGNAL( clicked() ), SLOT( query_db() ) );

	pb_save_db = pushbutton( "Backup Result to Database" );
	connect( pb_save_db, SIGNAL( clicked()), SLOT( save_db() ) );

	pb_retrieve_db = pushbutton( "Retrieve Result from Database", false );
	connect( pb_retrieve_db, SIGNAL( clicked() ), SLOT( retrieve_db() ) );

	lbl_instr = banner( "Doubleclick on result data to select" );

	pb_del_db = pushbutton( "Delete Result from Database" );
	connect( pb_del_db, SIGNAL( clicked() ), SLOT( check_permission() ) );

	lb_result = listbox( "Result files" );
	connect( lb_result, SIGNAL( selected( int ) ), SLOT( select_result( int ) ) );

	pb_reset = pushbutton( "Reset" );
	connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );

	pb_display = pushbutton( "Show Selected Result" );
	connect( pb_display, SIGNAL( clicked() ), SLOT( display() ) );

	pb_runrequest = pushbutton( "Select Run Request" );
	connect( pb_runrequest, SIGNAL( clicked() ), SLOT( sel_runrequest() ) );

	lbl_runrequest= textlabel( " Not Selected" );
	lbl_runrequest->setAlignment( AlignLeft | AlignVCenter );

	lbl_expdata = label( " Experimental Data ID:", -1 );
	lbl_expdata->setAlignment( AlignLeft | AlignVCenter );

	lbl_expdt = textlabel( "" );
	lbl_expdt->setAlignment( AlignLeft | AlignVCenter );

	pb_investigator = pushbutton( "Investigator ID:" );
	connect( pb_investigator, SIGNAL( clicked() ), SLOT( sel_investigator() ) );

	lbl_invst = textlabel( "" );
	lbl_invst->setAlignment( AlignLeft | AlignVCenter );

	lbl_date = label( " Date:", -1 );
	lbl_date->setAlignment( AlignLeft | AlignVCenter );

	lbl_dt= textlabel( "" );
	lbl_dt->setAlignment( AlignLeft | AlignVCenter );

	lbl_description = label( " Run ID:", -1 );
	lbl_description->setAlignment( AlignLeft | AlignVCenter );

	le_dscrpt = textlabel( "" );
	le_dscrpt->setAlignment( AlignLeft | AlignVCenter );

	setup_GUI();
}

void US_DB_Veloc::setup_GUI( void )
{
	int j       = 0;
	int rows    = 9;
	int columns = 1;
	int spacing = 2;

	QGridLayout* background = new QGridLayout( this, 2, 1, spacing );
	QGridLayout* subGrid1   = new QGridLayout( rows, columns, spacing );

	for ( int i = 0; i < rows; i++ )
	{
		subGrid1->setRowSpacing( i, 26 );
	}

	subGrid1->addWidget( pb_load,        j, 0 ); j++;
	subGrid1->addWidget( pb_load_db,     j, 0 ); j++;
	subGrid1->addWidget( pb_retrieve_db, j, 0 ); j++;
	subGrid1->addWidget( lbl_instr,      j, 0 ); j++;

	subGrid1->addMultiCellWidget( lb_result, j, j + 4, 0, 0 );
	subGrid1->setRowStretch( j, 5 );

	j       = 0;
	rows    = 9;
	columns = 4;
	spacing = 2;

	QGridLayout* subGrid2 = new QGridLayout( rows, columns, spacing );

	for (int i=0; i<rows; i++)
	{
		subGrid2->setRowSpacing(i, 26);
	}

	subGrid2->addMultiCellWidget( pb_save_db, j, j, 0, 1 ); j++;
	subGrid2->addMultiCellWidget( pb_del_db,  j, j, 0, 1 ); j++;

	subGrid2->addWidget( pb_runrequest,   j, 0 );
	subGrid2->addWidget( lbl_runrequest,  j, 1 ); j++;
	subGrid2->addWidget( pb_investigator, j, 0 );
	subGrid2->addWidget( lbl_invst,       j, 1 ); j++;
	subGrid2->addWidget( lbl_expdata,     j, 0 );
	subGrid2->addWidget( lbl_expdt,       j, 1 ); j++;
	subGrid2->addWidget( lbl_date,        j, 0 );
	subGrid2->addWidget( lbl_dt,          j, 1 ); j++;
	subGrid2->addWidget( lbl_description, j, 0 );
	subGrid2->addWidget( le_dscrpt,       j, 1 ); j++;
	subGrid2->addWidget( pb_display,      j, 0 );
	subGrid2->addWidget( pb_reset,        j, 1 ); j++;
	subGrid2->addWidget( pb_help,         j, 0 );
	subGrid2->addWidget( pb_close,        j, 1 );

	background->addLayout( subGrid1, 0, 0 );
	background->setColSpacing( 0, 450 );

	background->addLayout( subGrid2, 0, 1 );
	background->setColStretch( 0, 3);

	background->activate();
}

void US_DB_Veloc::sel_runrequest( void )
{
	US_DB_RunRequest* runrequest_dlg = new US_DB_RunRequest();

	runrequest_dlg->setCaption("Run Request Information");
	runrequest_dlg->show();

	connect(runrequest_dlg, SIGNAL( issue_RRID           ( int ) ), 
	                        SLOT  ( update_runrequest_lbl( int ) ) );
}

void US_DB_Veloc::update_runrequest_lbl ( int RRID )
{
	runrequestID = RRID;
	lbl_runrequest->setNum( runrequestID );

	if ( runrequestID == 0 )
	{
		lbl_runrequest->setText(" Not Selected");
	}
}

void US_DB_Veloc::load_HD( void )
{
	reset();
	data_control = new Data_Control_W( 7 );  //velocity data
	data_control->load_data();

	run_id           = data_control->run_inf.run_id;
	QString htmlDir  = data_control->htmlDir;
	exp_rst.expRstID = data_control->run_inf.expdata_id;
	lbl_expdt->setNum( exp_rst.expRstID );

	exp_rst.invID = data_control->run_inf.investigator;
	lbl_invst->setNum( exp_rst.invID );

	exp_rst.date = data_control->run_inf.date;
	lbl_dt->setText( exp_rst.date );

	le_dscrpt->setText( run_id );

	lb_result->clear();
	lb_result->insertItem( tr( "Data shown is loaded from Hard Drive" ) );

	from_HD = true;

	if ( htmlDir != "" )
	{
		US_Report_Veloc* report_veloc = new US_Report_Veloc( false );
		report_veloc->write_file( data_control );
	}
}

/*!  Open US_DB_TblInvestigator interface for selecting investigator.  */
void US_DB_Veloc::sel_investigator( void )
{
	US_DB_TblInvestigator* investigator_dlg = new US_DB_TblInvestigator();

	investigator_dlg->setCaption( "Investigator Information" );
	investigator_dlg->pb_exit->setText( "Accept" );
	connect( investigator_dlg, SIGNAL( valueChanged           ( QString, int ) ), 
	                           SLOT  ( update_investigator_lbl( QString, int ) ));
	investigator_dlg->exec();
}

void US_DB_Veloc::update_investigator_lbl ( QString Display, int InvID )
{
	exp_rst.invID = InvID;

	lbl_invst->setText( Display );

	if ( Display == "" )
	{
		lbl_invst->setText( "" );
	}
}

void US_DB_Veloc::save_db()
{
	if ( ! from_HD )
	{
		QMessageBox::message(
			tr( "Attention:" ), 
			tr( "Please load result data from harddrive first.\n" ) );

		return;
	}

	if ( exp_rst.expRstID <= 0 )
	{
		QMessageBox::message(
			tr( "Attention:" ), 
			tr( "No database information for this run!" ) );

		return;
	}

	db_connect();

	// Insert Compressed data into tblVelocResult and tblVelocResultData
	if ( ! insertCompressedData() )
	{
		QMessageBox::message(
			tr( "Attention:" ), 
			tr( "Insert compress data failed.\n" ) );
	}
}

bool US_DB_Veloc::insertCompressedData()
{
	QProgressDialog* pd = progressdialog( 
	  "Please wait while your data is saved to the database...", "pd", 8 );

	pd->setMinimumDuration( 0 );
	pd->setProgress       ( 1 );

	QString reportDir = USglobal->config_list.html_dir   + "/";
	QString resultDir = USglobal->config_list.result_dir + "/";
	QString dataDir   = USglobal->config_list.data_dir   + "/";

	QStringList files; 
	US_Gzip     gzip;
	US_Tar      tar;
	
	// Tar report directory 

	pd->setLabelText( "Compressing Report Data..." );
	qApp->processEvents();

	QString tarfile =  run_id + "_report.tar";
	int     ret;

	chdir( reportDir.latin1() );
	files <<  run_id;  // A directory

	if ( ( ret = tar.create( tarfile, files ) ) != GZIP_OK )
	{
		pd->close();
		delete pd;
		QMessageBox::message(
		  tr( "UltraScan tar creation Error: Report" ),
		  tr( tar.explain( ret ) + tr( "/nInput files:\n" ) + files.join( "\n") ) );

		// Remove the tar file
		QFile::remove( reportDir + tarfile );
		return false;
	}

	// Gzip report tar file
	pd->setProgress( 2 );
	qApp->processEvents();

	if ( gzip.gzip( tarfile ) != TAR_OK )
	{
		pd->close();
		delete pd;
		QMessageBox::message(
		  tr( "UltraScan Error:" ),
		  tr( "Unable to uncompress tar archive.\n" +
		      reportDir + tarfile ) );

		return false;
	}
	
	// Tar result tar file

	pd->setLabelText( "Compressing Result Data..." );
	pd->setProgress( 3 );
	qApp->processEvents();

	tarfile =  run_id + "_result.tar";
	files.clear();

	QDir work( resultDir );
	work.setNameFilter( run_id + "*" );
	files = work.entryList();

	chdir( resultDir.latin1() );

	if ( ( ret = tar.create( tarfile, files ) ) != GZIP_OK )
	{
		pd->close();
		delete pd;
		QMessageBox::message(
		  tr( "UltraScan tar creation Error: Result" ),
		  tr( tar.explain( ret ) + tr( "/nInput files:\n" ) + files.join( "\n") ) );

		// Remove the tar file
		QFile::remove( resultDir + tarfile );
		return false;
	}

	// Gzip result tar file
	pd->setProgress( 4 );
	qApp->processEvents();

	if ( gzip.gzip( tarfile ) != TAR_OK )
	{
		pd->close();
		delete pd;
		QMessageBox::message(
		  tr( "UltraScan Error:" ),
		  tr( "Unable to uncompress tar archive.\n" +
		      reportDir + tarfile ) );

		return false;
	}

	// Insert data to DB table tblVelocResultData

	pd->setLabelText( "Uploading Data..." );
	pd->setProgress( 5 );
	qApp->processEvents();

	db_connect();

	// Check for duplicates
	
	QString q;
	int     oldDataID  = 0;
	int     VelocRstID = exp_rst.expRstID;
	
	q.sprintf( "SELECT DataID FROM tblVelocResult "
	           "WHERE VelocRstID = %d;",  VelocRstID );
			
	QSqlQuery check( q );
				
	if ( check.next() ) oldDataID = check.value( 0 ).toInt();

	if ( oldDataID > 0 )
	{
		int answer =  QMessageBox::question( this, tr( "UltraScan Warning" ), 
			tr( "The selected result already exists in the database.  Overwrite?" ), 
			QMessageBox::Yes, QMessageBox::Cancel );
	
		if ( answer == QMessageBox::Cancel )
		{
			cleanCompressedFiles();
			pd->close();
			delete pd;
			return false;
		}
	}

	// If overwriting, delete the old result data
	// Note: this also deletes the asociated record from tblVelocResult
	// due to the constraint in the table definition
	
	if ( oldDataID > 0 )
	{
		q.sprintf( "DELETE FROM tblVelocResultData WHERE tableID=%d", oldDataID );
		check.exec( q );
	}

	QSqlCursor cursor( "tblVelocResultData" );
	cursor.setMode( QSqlCursor::Insert );

	QSqlRecord* buffer = cursor.primeInsert();

	DataID = get_newID( "tblVelocResultData", "tableID" );
	buffer->setValue( "tableID", DataID );

	QString fileName = reportDir + run_id + "_report.tar.gz";
	write_blob( fileName, buffer, "Report_Tar" );

	pd->setProgress( 6 );
	qApp->processEvents();

	fileName = resultDir + run_id + "_result.tar.gz";
	write_blob( fileName, buffer, "Result_Tar");

	int result = cursor.insert();

	if ( result <= 0 )
	{
		QSqlError err = cursor.lastError();
		QMessageBox::message(
			tr( "Attention:" ), 
			tr( "Saving to DB table 'tblVelocResultData' failed.\n"
			    "Error message from MySQL:\n\n" ) + err.text() );

		pd->close();
		delete pd;
		return false;
	}

	// Insert data to DB table tblVelocResult

	q = "INSERT INTO tblVelocResult "
	    "(VelocRstID, Date, RunID, InvestigatorID, DataID";

	if ( runrequestID > 0 ) q += ", RunRequestID";

	q += ") VALUES("
	  + QString::number( exp_rst.expRstID ) + ", "
		"'"	+ exp_rst.date                    + "', "
		"'"	+ run_id                          + "', "
		+ QString::number( exp_rst.invID )    + ", "
		+ QString::number( DataID );

	if ( runrequestID > 0 ) q += ", " + QString::number( runrequestID ); 

	q += ");";

	QSqlQuery target;
	bool      finished = target.exec( q );

	if ( ! finished )
	{
		pd->close();
		delete pd;
		QSqlError sqlerr = target.lastError();
		QMessageBox::message(
		  tr( "Attention:" ), 
		  tr( "Saving to DB table 'tblVelocResult' failed.\n"
		    "Attempted to execute this command:\n\n" )
		    + q + "\n\n" +
		    tr( "Causing the following error:\n\n" )
		    + sqlerr.text() ) ;

		// Delete inserted tblVelocResultData data
		
		QSqlQuery del_data;
		q.sprintf( "DELETE FROM tblVelocResultData WHERE tableID = %d;", DataID );
		del_data.exec( q);
				
		// Clean up the created files
				
		cleanCompressedFiles();
		return false;
	}

	if ( runrequestID > 0 )  // update tblResult
	{
		int resultID = 0;
		q.sprintf( "SELECT ResultID FROM tblResult "
		           "WHERE RunRequestID = %d;",  runrequestID );
				
		QSqlQuery check( q );
				
		if ( check.size() > 0 )
		{
			if ( check.next() ) resultID = check.value( 0 ).toInt();
		}

		QSqlQuery query;

		if ( resultID > 0 )  // row exists and update velocprojectID
		{
			q.sprintf( "UPDATE tblResult "
			           "SET InvestigatorID = %d, VelocDataID = %d "
			           "WHERE ResultID = %d;", 
			            exp_rst.invID, exp_rst.expRstID, resultID );

			query.exec( q );
		}
		else  // Create new row
		{
			resultID = get_newID( "tblResult","ResultID" );
			q = "INSERT INTO tblResult "
			    "(ResultID, InvestigatorID, RunRequestID, VelocDataID) VALUES("
			  + QString::number( resultID ) + ", "
			  + QString::number( exp_rst.invID ) + ", " 
			  + QString::number( runrequestID ) + ", " 
			  + QString::number(exp_rst.expRstID) + ");";

			if ( ! query.exec( q ) )
			{
				pd->close();
				delete pd;

				QMessageBox::message(
					tr( "Attention:" ), 
					tr( "Uploading data to the Result Database table failed!" ) );

				return false;
			}
		}
	}

	// clean the temp compress file
	cleanCompressedFiles();
	pd->close();
	delete pd;

	QMessageBox::message(
		tr( "Success"),
		tr( "Successfully saveed data to database.\n" ) );

	return true;
}


void US_DB_Veloc::cleanCompressedFiles()
{
	QString reportDir = USglobal->config_list.html_dir   + "/";
	QString resultDir = USglobal->config_list.result_dir + "/";
	QString dataDir   = USglobal->config_list.data_dir   + "/";

	QStringList files; 

	files << reportDir + run_id + "_report.tar.gz"
	      << reportDir + run_id + "_report.tar"
	      << resultDir + run_id + "_result.tar.gz"
	      << resultDir + run_id + "_result.tar"
	      << dataDir   + run_id + "_rawdata.tar.gz"
	      << dataDir   + run_id + "_rawdata.tar";

	QStringList::iterator it = files.begin();

	while ( it != files.end() )
	{
		QFile tempFile( *it++ );
		if ( tempFile.exists() ) tempFile.remove();
	}
}

void US_DB_Veloc::query_db()
{
	cleanCompressedFiles();
	db_connect();

	int maxID = get_newID( "tblVelocResult", "VelocRstID" );

	// Prevent memory leaks
	if ( item_ExpdataID   ) delete [] item_ExpdataID;
	if ( item_Description ) delete [] item_Description;

	item_ExpdataID       = new int    [ maxID ];
	item_Description     = new QString[ maxID ];
	QString* display_Str = new QString[ maxID ];

	QString q;

	if ( exp_rst.invID > 0 )
	{
		q.sprintf( "SELECT VelocRstID, RunID "
		           "FROM tblVelocResult "
		           "WHERE InvestigatorID = %d "
		           "ORDER BY VelocRstID DESC;", exp_rst.invID );
	}
	else
	{
		q.sprintf( "SELECT VelocRstID, RunID "
		           "FROM tblVelocResult "
		           "ORDER BY VelocRstID DESC;");
	}

	QSqlQuery query( q );
	int count = 0;

	if ( query.isActive() )
	{
		while( query.next() )
		{
			QString itemStr           = query.value( 0 ).toString();;
			int     item              = itemStr.toInt();

			item_ExpdataID  [ count ] = item;
			item_Description[ count ] = query.value( 1 ).toString();
			display_Str     [ count ] = "(" +  itemStr + "): " 
			                          + item_Description[ count ];
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

void US_DB_Veloc::select_result(int item)
{
	QString str;

	if ( query_flag )
	{
		exp_rst.expRstID = item_ExpdataID  [ item ];
		run_id           = item_Description[ item ];

		QSqlCursor cur( "tblVelocResult" );
		str.sprintf( "VelocRstID = %d", exp_rst.expRstID );
		cur.select( str );

		if ( cur.next() )
		{
			exp_rst.invID = cur.value( "InvestigatorID" ).toInt();
			exp_rst.date  = cur.value( "Date"           ).toString();
			runrequestID  = cur.value( "RunRequestID"   ).toInt();
			DataID        = cur.value( "DataID"         ).toInt();
		}

		lbl_expdt->setNum ( exp_rst.expRstID );
		lbl_invst->setText( show_investigator( exp_rst.invID ) );
		lbl_dt   ->setText( exp_rst.date );
		le_dscrpt->setText( run_id );

		if ( runrequestID > 0 )
		{
			lbl_runrequest->setNum    ( runrequestID );
			pb_runrequest ->setEnabled( false );
		}
		else
		{
			lbl_runrequest->setText( " N/A" );
		}

		del_flag      = true;
		retrieve_flag = true;
	}
	else
	{
		QMessageBox::message(
			tr( "Warning:" ), 
			tr( "Please Query result from DB first." ) );

		return;
	}

	pb_retrieve_db->setEnabled( true );
}

void US_DB_Veloc::retrieve_db( void )
{
	from_DB = false;

	if ( ! query_flag )
	{
		 QMessageBox::message(
			tr( "Warning:" ), 
			tr( "Please Query result from DB first." ) );

		return;
	}

	if ( ! retrieve_flag )
	{
		QMessageBox::message(
		  tr( "Warning:" ), 
		  tr( "Please Double-Click to select result from list." ) );

		return;
	}

	cleanCompressedFiles();

	// Check if the same run_id files exist
	QString reportDir = USglobal->config_list.html_dir   + "/";
	QString dataDir   = USglobal->config_list.data_dir   + "/";
	QString resultDir = USglobal->config_list.result_dir + "/";

	QString namefilter = run_id + "*" ;
	QDir    check_dir;

	check_dir.setPath      ( resultDir );
	check_dir.setNameFilter( namefilter );

	QStringList entries = check_dir.entryList();

	int count = check_dir.entryList().count(); 

	if ( count > 0 && DataID > 0 )
	{
		switch( QMessageBox::information( this, 
		  tr( "Warning" ),
		  tr( "The result for '" ) + run_id + "'\n"
		      + tr( "already exists in your result directory: \n\n'" )
		      + USglobal->config_list.result_dir +"'\n\n"
		      + tr( "Do you want to overwrite it?\n\n" ),
		  "OK", "CANCEL", 0, 1 ) )
		{
			case 0:
				break;

			case 1:
				return;
		}
	}

	QProgressDialog* pd = progressdialog( "Retrieving Velocity Data", "pd", 9 );
	pd->setLabelText( "Downloading Velocity Data..." );
	pd->setProgress( 1 );
	pd->show();
	qApp->processEvents();

	QString filename;
	QString q;
	QSqlCursor cursor( "tblVelocResultData" );

  q.sprintf( "tableID = %d", DataID );

	qApp->processEvents();
	sleep( 2 ); // Wait to display progress dialog
	cursor.select( q );

	// Fetch data
	if ( cursor.next() )
	{
		filename = make_tempFile( reportDir, run_id + "_report.tar.gz" );

		if ( ! read_blob( "Report_Tar", cursor, filename ) )
		{
			QMessageBox::message(
			  tr( "UltraScan Error:" ), 
			  tr( "Unable to retrieve Report files." ) );
		}

		filename = make_tempFile( resultDir, run_id + "_result.tar.gz" );
		pd->setProgress( 2 );
		qApp->processEvents();

		if ( ! read_blob( "Result_Tar", cursor, filename ) )
		{
			QMessageBox::message(
			  tr( "UltraScan Error:" ),
			  tr( "Unable to retrieve Result files." ) );
		}
	}

	cursor.setName( "tblRawExpData" );
	q.sprintf( "ExpdataID = %d", exp_rst.expRstID );
	cursor.select( q );

	if ( cursor.next() )
	{
		filename = make_tempFile( dataDir, run_id + "_rawdata.tar.gz" );
		pd->setProgress( 3 );
		qApp->processEvents();

		if ( ! read_blob( "RawData", cursor, filename ) )
		{
			QMessageBox::message(
			  tr( "UltraScan Error:" ), 
			  tr( "Unable to retrieve Raw data files." ) );
		}
	}

	US_Gzip gzip;
	US_Tar  tar;

	// Uncompress report
	QString targzfile = run_id + "_report.tar.gz";

	chdir( reportDir.latin1() );
	pd->setLabelText( "Uncompressing Report Data..." );
	pd->setProgress( 4 );
	qApp->processEvents();

	if ( gzip.gunzip( targzfile ) != TAR_OK )
	{
		pd->close();
		QMessageBox::message(
		  tr( "UltraScan Error:" ),
		  tr( "Unable to uncompress tar archive.\n" +
		      reportDir + targzfile ) );

	  return;
	}


	pd->setProgress( 5 );
	qApp->processEvents();

	QString tarfile =  run_id + "_report.tar";
	int     ret;

	if ( ( ret = tar.extract( tarfile ) ) != GZIP_OK )
	{
		pd->close();
		QMessageBox::message(
		  tr( "UltraScan tar extraction Error:" ),
		  tr( tar.explain( ret ) + tr( "/ntarfile: " ) + tarfile ) );

		// Remove the tar file
		QFile::remove( reportDir + tarfile );
		goto result;
	}

	QFile::remove( reportDir + tarfile );

	// Uncompress results
	pd->setLabelText( "Uncompressing Result Data..." );
	pd->setProgress( 6 );
	qApp->processEvents();

	targzfile = run_id + "_result.tar.gz";
	chdir( resultDir.latin1() );

	if ( gzip.gunzip( targzfile ) != TAR_OK )
	{
		pd->close();
		QMessageBox::message(
		  tr( "UltraScan Error:" ),
		  tr( "Unable to uncompress tar archive.\n" +
		      resultDir + targzfile ) );

		return;
	}

result:
	tarfile = run_id + "_result.tar";

	if ( ( ret = tar.extract( tarfile ) ) != GZIP_OK )
	{
		pd->close();
		QMessageBox::message(
			tr( "UltraScan tar extraction Error:" ),
		  tr( tar.explain( ret ) + tr( "/ntarfile: " ) + tarfile ) );

		// Remove the tar file
		QFile::remove( resultDir + tarfile );
		goto rawdata;
	}

	QFile::remove( resultDir + tarfile );

	// Uncompress raw data
	pd->setLabelText( "Uncompressing Raw Data..." );
	pd->setProgress( 7 );
	qApp->processEvents();

	targzfile = run_id + "_rawdata.tar.gz";
	chdir( dataDir.latin1() );

	if ( gzip.gunzip( targzfile ) != TAR_OK )
	{
		pd->close();
		QMessageBox::message(
		  tr( "UltraScan Error:" ),
		  tr( "Unable to uncompress tar archive.\n" +
		      dataDir + targzfile ) );

	  return;
	}

rawdata:
	tarfile = run_id + "_rawdata.tar";
	pd->setProgress( 8 );
	qApp->processEvents();

	if ( ( ret = tar.extract( tarfile ) ) != GZIP_OK )
	{
		pd->close();
		QMessageBox::message(
		  tr( "UltraScan tar extraction Error:" ),
		  tr( tar.explain( ret ) + tr( "/ntarfile: " ) + tarfile ) );

		// Remove the tar file
		QFile::remove( dataDir + tarfile );
	}

	QFile::remove( dataDir + tarfile );

	pd->setProgress( 9 );
	qApp->processEvents();
	from_DB = true;
}

/*!  Open US_DB_Admin to check delete permission.  */
void US_DB_Veloc::check_permission( void )
{
	if ( ! del_flag )
	{
		QMessageBox::message(
		  tr( "Attention:" ), 
		  tr( "Please select a record\n"
		      "to be deleted from the database." ) );

		return;
	}

	US_DB_Admin* db_admin = new US_DB_Admin( "" );
	db_admin->show();
	connect( db_admin, SIGNAL( issue_permission( bool ) ), 
	                   SLOT  ( delete_db       ( bool ) ) );
}

/*!  Delete one selected experimental result table entry.  */
void US_DB_Veloc::delete_db( bool permission )
{
	if ( ! permission )
	{
		QMessageBox::message(
		  tr( "Attention:" ), 
		  tr( "Permission denied, please contact your system "
		      "administrator for help" ) );

		return;
	}

	QSqlCursor cursor( "tblVelocResult" );
	cursor.setMode( QSqlCursor::Delete );
	QSqlIndex filter = cursor.index( "VelocRstID" );
	cursor.setValue( "VelocRstID", exp_rst.expRstID );
	cursor.primeDelete();

	switch( QMessageBox::information( this, 
		tr( "Delete this record?" ), 
		tr( "Clicking 'OK' will delete the selected result data from the database" ),
		tr( "OK" ), tr( "CANCEL" ),	0, 1 ) )
	{
		case 0:
		{
			QString q;
			QSqlQuery query;
			q.sprintf( "UPDATE tblResult "
			           "SET VelocDataID = NULL "
			           "WHERE VelocDataID = %d;", exp_rst.expRstID );
			query.exec( q );

			if ( DataID > 0 )
			{
				QSqlQuery del_file;
				q.sprintf( "DELETE FROM tblVelocResultData "
				           "WHERE tableID = %d;", DataID );
				del_file.exec( q );
			}

			cursor.del();
			reset();
			query_db();
			break;
		}
		case 1:
			break;
	}
}

void US_DB_Veloc::display()
{
	QString indexFile;

	if ( from_HD || from_DB )
	{
		indexFile = USglobal->config_list.html_dir + "/" + run_id + "/index.html";
	}
	else
	{
		QMessageBox::message(
		  tr( "Attention:" ), 
		  tr( "Please retrieve result from DB first\n" ) );

		return;
	}

	US_Help* online_help = new US_Help();
	online_help->show_html_file( indexFile );
}

void US_DB_Veloc::reset()
{
	cleanCompressedFiles();

	from_HD          = false;
	from_DB          = false;
	query_flag       = false;
	retrieve_flag    = false;
	del_flag         = false;

	run_id           = "";
	runrequestID     = 0;
	exp_rst.invID    = -1;
	exp_rst.expRstID = -1;
	exp_rst.date     = "";
	run_id           = "";

	for ( int i = 0; i < 8; i++ )
	{
		exp_rst.CellID[ i ] = 0;

		for ( int j = 0; j < 4; j++ )
		{
			exp_rst.EDid[ i ][ j ] = -1;
		}
	}

	pb_runrequest ->setEnabled( true );
	pb_retrieve_db->setEnabled( false );
	lbl_runrequest->setText   ( " Not Selected" );
	lbl_expdt     ->setText   ( "" );
	lbl_invst     ->setText   ( "" );
	lbl_dt        ->setText   ( "" );
	le_dscrpt     ->setText   ( "" );
	lb_result     ->clear();
}

/*! Open a netscape browser to load help page.*/
void US_DB_Veloc::help( void )
{
	US_Help* online_help = new US_Help( this );
	online_help->show_help( "manual/us_db_result_veloc.html" );

}

/*! Close the interface.*/
void US_DB_Veloc::quit( void )
{
	cleanCompressedFiles();
	close();
}

/*!  Event handler, for event <var>e</var>, to receive widget close events.  */
void US_DB_Veloc::closeEvent( QCloseEvent* e )
{
	cleanCompressedFiles();
	e->accept();
}
