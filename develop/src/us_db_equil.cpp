#include "../include/us_db_equil.h"
#include "../include/us_gzip.h"
#include "../include/us_tar.h"

#ifdef WIN32
	#define chdir _chdir
	#include <direct.h>
#endif

US_DB_Equil::US_DB_Equil( QWidget* p, const char* name) 
  : US_DB_Widgets( p, name )
{
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

	pb_load = pushbutton( "Load Result from HD" );
	connect( pb_load, SIGNAL( clicked() ), SLOT( load_HD() ) );

	pb_help = pushbutton( "Help" );
	connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

	pb_close = pushbutton( "Close" );
	connect( pb_close, SIGNAL( clicked() ), SLOT( quit() ) );

	pb_load_db = pushbutton( "Query Result from DB" );
	connect( pb_load_db, SIGNAL( clicked() ), SLOT( query_db() ) );

	pb_save_db = pushbutton( "Backup Result to DB" );
	connect( pb_save_db, SIGNAL( clicked() ), SLOT( save_db() ) );

	pb_retrieve_db = pushbutton( "Retrieve Result from DB" );
	connect( pb_retrieve_db, SIGNAL( clicked() ), SLOT( retrieve_db() ) );

	lbl_instr = banner("Doubleclick on result data to select", -1 );

	pb_del_db = pushbutton( "Delete Result from DB" );
	connect( pb_del_db, SIGNAL( clicked() ), SLOT( check_permission() ) );

	lb_result = listbox( "Result files" );
	connect( lb_result, SIGNAL( selected( int ) ), SLOT( select_result( int ) ) );

	pb_reset = pushbutton( "Reset" );
	connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );

	pb_display = pushbutton( "Show Selected Result" );
	connect( pb_display, SIGNAL( clicked() ), SLOT( display() ) );

	pb_runrequest = pushbutton( "Select Run Request" );
	connect( pb_runrequest, SIGNAL( clicked() ), SLOT( sel_runrequest() ) );

	lbl_runrequest = textlabel( " Not Selected" );
	lbl_runrequest->setAlignment( AlignLeft | AlignVCenter );

	lbl_expdata = label( " Experimental Data ID:" );
	lbl_expdata->setAlignment( AlignLeft | AlignVCenter );

	lbl_expdt = textlabel( "" );
	lbl_expdt->setAlignment( AlignLeft | AlignVCenter );

	pb_investigator = pushbutton( "Investigator ID:" );
	connect( pb_investigator, SIGNAL( clicked() ), SLOT( sel_investigator() ) );

	lbl_invst = textlabel( "" );
	lbl_invst->setAlignment( AlignLeft | AlignVCenter );
	
	lbl_date = label( " Date:" );
	lbl_date->setAlignment( AlignLeft | AlignVCenter );

	lbl_dt = textlabel( "" );
	lbl_dt->setAlignment( AlignLeft | AlignVCenter );

	lbl_description = label( " Run ID:" );
	lbl_description->setAlignment( AlignLeft | AlignVCenter );

	le_dscrpt= textlabel( "" );
	le_dscrpt->setAlignment( AlignLeft | AlignVCenter );

	setup_GUI();
}

void US_DB_Equil::setup_GUI()
{
	int j       = 0;
	int rows    = 9;
	int spacing = 2;

	QGridLayout* background = new QGridLayout( this, 2, 1, spacing );	
	QGridLayout* subGrid1   = new QGridLayout( rows, 1, spacing);
	QGridLayout* subGrid2   = new QGridLayout( rows, 2, spacing );

	for ( int i = 0; i < rows; i++ )
	{
		subGrid1->setRowSpacing( i, 26 );
		subGrid2->setRowSpacing( i, 26 );
	}

	subGrid1->addWidget( pb_load,        j, 0 ); j++;
	subGrid1->addWidget( pb_load_db,     j, 0 ); j++;
	subGrid1->addWidget( pb_retrieve_db, j, 0 ); j++;
	subGrid1->addWidget( lbl_instr,      j, 0 ); j++;

	subGrid1->addMultiCellWidget( lb_result, j, j + 4, 0, 0 );
	subGrid1->setRowStretch( j, 5 );

	j = 0;

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
	background->addLayout( subGrid2, 0, 1 );

	background->setColSpacing( 0, 450 );
	background->setColStretch( 0,   3 );

	background->activate();
}

void US_DB_Equil::sel_runrequest( void )
{
	US_DB_RunRequest* runrequest_dlg = new US_DB_RunRequest();
	runrequest_dlg->setCaption( "Run Request Information" );
	runrequest_dlg->show();
	connect( runrequest_dlg, SIGNAL( issue_RRID           ( int ) ), 
	                         SLOT  ( update_runrequest_lbl( int ) ) );
}

void US_DB_Equil::update_runrequest_lbl( int RRID )
{
	runrequestID = RRID;
	lbl_runrequest->setNum( runrequestID );
	
	if ( runrequestID == 0 ) lbl_runrequest->setText( " Not Selected" );
}

void US_DB_Equil::load_HD( void )
{
	reset();

	Data_Control_W* data_control = new Data_Control_W( 8 );	//equilibrium data
	data_control->load_data();

	run_id          = data_control->run_inf.run_id;
	le_dscrpt->setText( run_id );	

	QString htmlDir = data_control->htmlDir;

	exp_rst.expRstID = data_control->run_inf.expdata_id;
	lbl_expdt->setNum( exp_rst.expRstID );

	exp_rst.invID = data_control->run_inf.investigator;
	lbl_invst->setNum( exp_rst.invID );

	exp_rst.date = data_control->run_inf.date;
	lbl_dt->setText( exp_rst.date );

	lb_result->clear();
	lb_result->insertItem( tr( "Data shown is loaded from Hard Drive" ) );

	from_HD = true;

	if( htmlDir != "" )
	{
		US_Report_Equil* report_equil = new US_Report_Equil( false );
		report_equil->generate( data_control );
	}
}

/*!  Open US_DB_TblInvestigator interface for selecting investigator.  */
void US_DB_Equil::sel_investigator( void )
{
	US_DB_TblInvestigator* investigator_dlg = new US_DB_TblInvestigator();
	
	investigator_dlg->setCaption( "Investigator Information" );
	investigator_dlg->pb_exit->setText( "Accept" );
	
	connect( investigator_dlg, SIGNAL( valueChanged           ( QString, int ) ), 
	                           SLOT  ( update_investigator_lbl( QString, int ) ));
	investigator_dlg->exec();
}

void US_DB_Equil::update_investigator_lbl( QString Display, int InvID )
{
	exp_rst.invID = InvID;
	lbl_invst->setText( Display );
}

void US_DB_Equil::save_db( void )
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
			tr( "No database information for this result" ) );

		return;
	}

	// Check whether the raw data exists
	db_connect();
	
	QString q = "SELECT ExpdataID FROM tblRawExpData "
	            "WHERE ExpdataID = '" 
	          + QString::number( exp_rst.expRstID) + "';";

	QSqlQuery query( q );

	if ( query.isActive() )
	{
		if ( ! query.next() )
		{
			QMessageBox::message(
				tr( "Attention:" ),
				tr( "Cannot find any raw experimental data from in the DB!" ) );

			return;
		}
	}

	// Insert Compressed data into tblEquilResult and tblEquilResultData

	QProgressDialog* pd = progressdialog(
		"Please wait while your data is saved to the database...", "pd", 8 );

	pd->setProgress       ( 0 );
	pd->setMinimumDuration( 0 );


	// tar report tar file	

	QString     reportDir = USglobal->config_list.html_dir + "/";
	QString     tarfile   = run_id + "_report.tar";
	QStringList entries   = run_id;

	pd->setLabelText( "Archiving the Equilibrium report..." );
	pd->setProgress( 1 );
	qApp->processEvents();

	if ( ! create_tar( reportDir, tarfile, entries ) )
	{
		pd->close();
		delete pd;
		return;
	}

	// gzip report tar file	
	pd->setProgress( 2 );
	qApp->processEvents();

	int     ret;
	US_Gzip gzip;

	if ( ( ret = gzip.gzip( tarfile ) ) != GZIP_OK )
	{
		pd->close();
		delete pd;

		QMessageBox::message(
			tr( "UltraScan gzip Error:" ),
			tr( gzip.explain( ret ) ) );

		// Remove the tar file
		QFile::remove( reportDir + tarfile );
		return;
  }

	// tar result tar file
	pd->setProgress( 3 );
	qApp->processEvents();

	QString resultDir = USglobal->config_list.result_dir + "/";

	tarfile = run_id + "_result.tar";
	
	QDir work;
	work.setPath      ( resultDir );
	work.setNameFilter( run_id + "*" );

	entries = work.entryList();

	if ( ! create_tar( resultDir, tarfile, entries ) )
	{
		pd->close();
		delete pd;
		return;
	}

	// gzip result tar file
	pd->setProgress( 4 );
	qApp->processEvents();

	if ( ( ret = gzip.gzip( tarfile ) ) != GZIP_OK )
	{
		pd->close();
		delete pd;

		QMessageBox::message(
			tr( "UltraScan gzip Error:" ),
			tr( gzip.explain( ret ) ) );

		// Remove the tar file
		QFile::remove( resultDir + tarfile );
		return;
  }

	// Insert data into DB table tblEquilResultData

	db_connect();
	
	QSqlCursor cursor( "tblEquilResultData" );
	cursor.setMode( QSqlCursor::Insert );
	
	QSqlRecord* buffer = cursor.primeInsert();
	DataID = get_newID( "tblEquilResultData", "tableID" );
	buffer->setValue( "tableID", DataID );
	
	pd->setProgress( 5 );
	qApp->processEvents();

	QString fileName = reportDir + run_id + "_report.tar.gz";
	write_blob( fileName, buffer, "Report_Tar" );
	
	pd->setProgress( 6 );
	qApp->processEvents();

	fileName = resultDir + run_id + "_result.tar.gz";
	write_blob( fileName, buffer, "Result_Tar");
	
	cursor.insert();
	
	// Insert data into DB table tblEquilResult

	q = "INSERT INTO tblEquilResult "
	    "(EquilRstID, Date, RunID, InvestigatorID, DataID";

	if ( runrequestID > 0 ) q += ", RunRequestID";

	q += ") VALUES("
		+  QString::number( exp_rst.expRstID )
		+  ", '"  + exp_rst.date
		+  "', '" + run_id 
		+  "', "  + QString::number( exp_rst.invID )
		+  ", "   + QString::number( DataID );

	if ( runrequestID > 0 ) q += ", " + QString::number( runrequestID );

	q += ");";

	QSqlQuery target;

	pd->setProgress( 7 );
	qApp->processEvents();

	if ( ! target.exec( q ) )
	{
		QSqlError sqlerr = target.lastError();

		QMessageBox::message(
			tr( "Attention:" ), 
			tr( "Saving to DB table 'tblEquilResult' failed.\n"
			    "Attempted to execute this command:\n\n" )
			    + q + "\n\n"
			    + tr( "Causing the following error:\n\n" )
			    + sqlerr.text() );

		// Delete inserted	tblEquilResultData data
		QSqlQuery del_data;

		q.sprintf( 
		  "DELETE FROM tblEquilResultData WHERE tableID = %d;", DataID );

		del_data.exec( q );

		// Clean the temp compress file	
		cleanCompressFile();
		pd->close();
		delete pd;
		return;
  }

	if ( runrequestID > 0 )  //update tblResult
	{
		int resultID = 0;
		
		q.sprintf( "SELECT ResultID FROM tblResult WHERE RunRequestID = %d;", 
			runrequestID );

		QSqlQuery chk( q );
		
		if ( chk.isActive() )
		{
			if ( chk.next() ) resultID = chk.value( 0 ).toInt(); 
			
			QSqlQuery query;
				
			if ( resultID > 0 )   // row exists, update equilprojectID
			{
				q.sprintf( "UPDATE tblResult SET InvestigatorID = %d, "
				           "EquilDataID = %d WHERE ResultID = %d;", 
				           exp_rst.invID, exp_rst.expRstID, resultID);
					
				query.exec( q );
			}
			else  // create new one entry
			{
				resultID = get_newID( "tblResult", "ResultID" );
				q = "INSERT INTO tblResult "
				    "(ResultID, InvestigatorID, RunRequestID, EquilDataID) "
						"VALUES("
				  + QString::number( resultID )         + ", "
				  + QString::number( exp_rst.invID )    + ", " 
					+ QString::number( runrequestID )     + ", " 
					+ QString::number( exp_rst.expRstID ) + ");";

				if ( ! query.exec( q ) )
				{
					QMessageBox::message(
						tr( "Attention:" ), 
						tr( "Uploading data to the Result Database table failed!" ) );

					cleanCompressFile();
					pd->close();
					delete pd;
					return;
				}
			}
		}

		// Clean the temp compress file	
		cleanCompressFile();
		pd->close();

		QMessageBox::message(
			tr( "Complete" ), 
			tr( "Sucessfully saved data to database.\n" ) );						
	}
}

bool US_DB_Equil::create_tar( const QString& dirString,
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

void US_DB_Equil::cleanCompressFile( void )
{
	QString reportDir = USglobal->config_list.html_dir   + "/";
  QString resultDir = USglobal->config_list.result_dir + "/";

	QStringList files;

	files << reportDir + run_id + "_report.tar.gz"
	      << reportDir + run_id + "_report.tar"
	      << resultDir + run_id + "_result.tar.gz"
	      << resultDir + run_id + "_result.tar";


	QStringList::iterator it = files.begin();

	while ( it != files.end() )
	{
		QFile tempFile( *it++ );
		if ( tempFile.exists() ) tempFile.remove();
	}
}
	
void US_DB_Equil::query_db( void )
{
	QString q;

	cleanCompressFile();
	db_connect();

	int maxID = get_newID("tblEquilResult","EquilRstID");
	int count = 0;

	if ( item_ExpdataID   ) delete [] item_ExpdataID;
	if ( item_Description ) delete [] item_Description;


	item_ExpdataID       = new int    [ maxID ];
	item_Description     = new QString[ maxID ];
	QString* display_Str = new QString[ maxID ];

	if ( exp_rst.invID > 0 )
	{
		q.sprintf( "SELECT EquilRstID, RunID FROM tblEquilResult "
		           "WHERE InvestigatorID = %d;", exp_rst.invID );
	}
	else
	{
		q.sprintf( "SELECT EquilRstID, RunID FROM tblEquilResult;" );
	}

	QSqlQuery query( q );
	
	while( query.next() )
	{
		item_ExpdataID  [ count ] = query.value( 0 ).toInt();
		item_Description[ count ] = query.value( 1 ).toString();
		display_Str     [ count ] = "(" 
		                          +  QString::number( item_ExpdataID[ count ] ) 
		                          + "): "
		                          + item_Description[ count ];
		count++;
	}

	if(count>0)
	{
		lb_result->clear();
		for( int i=0; i<count; i++)
		{
			lb_result->insertItem(display_Str[i]);
		}
		query_flag = true;
	}
	else
	{
		lb_result->clear();
		lb_result->insertItem("No data were found in the DB");
	}

	delete [] display_Str;
}

void US_DB_Equil::select_result( int item )
{
	QString q;

	if ( query_flag )
	{
		exp_rst.expRstID = item_ExpdataID  [ item ];
		run_id           = item_Description[ item ];

		QSqlCursor cursor( "tblEquilResult" );
		q.sprintf( "EquilRstID = %d", exp_rst.expRstID );
		cursor.select( q );		

		if ( cursor.next() )
		{
			exp_rst.invID = cursor.value( "InvestigatorID" ).toInt();
			exp_rst.date  = cursor.value( "Date"           ).toString();
			runrequestID  = cursor.value( "RunRequestID"   ).toInt();
			DataID        = cursor.value( "DataID"         ).toInt();
		}

		lbl_expdt->setNum ( exp_rst.expRstID );
		lbl_invst->setText( show_investigator( exp_rst.invID ) );
		lbl_dt   ->setText( exp_rst.date );
		le_dscrpt->setText( run_id );

		if ( runrequestID > 0 )
		{
			lbl_runrequest->setNum( runrequestID );
			pb_runrequest ->setEnabled( false );
		}
		else
		{
			lbl_runrequest->setText(" N/A");
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
}

void US_DB_Equil::retrieve_db( void )
{
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

	cleanCompressFile();

	// Check do there same run_id files exists

	QString     reportDir  = USglobal->config_list.html_dir   + "/";
	QString     resultDir  = USglobal->config_list.result_dir + "/";
	QString     dataDir    = USglobal->config_list.data_dir   + "/";
	QString     namefilter = run_id +"*";
	QDir        check_dir;
	QStringList entries;

	check_dir.setPath      ( resultDir );
	check_dir.setNameFilter( namefilter );

	entries   = check_dir.entryList();
	int count = entries.count(); 

	if ( count > 0 )
	{
		switch ( QMessageBox::information ( this, 
			tr( "Warning" ), 
			tr( "The result for '" ) + run_id 
			+ tr( "'\n already existed in your result directory: \n\n'" ) 
			+ resultDir
			+ tr( "'\n\n Do you want to overwrite it?\n\n" ),
			tr( "OK" ), tr( "CANCEL" ),	0, 1 ) )
		{
			case 0:
				break;

			case 1:
				return;
		}
	}

	QString filename;
	QString tarfile;
	QString q;

	QProgressDialog* pd = progressdialog( 
		"Retrieving Equilibrium Data", "pd", 10 );

	pd->setProgress( 1 );
	qApp->processEvents();

	QSqlCursor cur1( "tblEquilResultData" );
	q.sprintf( "tableID = %d", DataID );
	cur1.select( q );

	if ( cur1.next() )
	{
		tarfile  = run_id + "_report.tar.gz";
		filename = make_tempFile( reportDir, tarfile );

		if ( ! read_blob( "Report_Tar", cur1, filename ) )
		{
			QMessageBox::message(
				tr( "UltraScan Error:" ),
				tr( "Unable to retrieve Report files." ) );
		}

		pd->setProgress( 2 );
		qApp->processEvents();

		tarfile = run_id + "_result.tar.gz";
		filename = make_tempFile( resultDir, tarfile );
		if ( ! read_blob( "Result_Tar", cur1, filename ) )
		{
			QMessageBox::message(
				tr( "UltraScan Error:" ),
				tr( "Unable to retrieve Result files." ) );
		}
	}

	/*
	QSqlCursor cur2( "tblRawExpData" );
	q.sprintf( "ExpdataID = %d", exp_rst.expRstID );
	cur2.select( q );

	if ( cur2.next() )
	{
		tarfile = run_id + "_rawdata.tar.gz";
		filename = make_tempFile( dataDir, tarfile );

		pd->setProgress( 3 );
		qApp->processEvents();

		if ( ! read_blob( "Rawdata", cur2, filename ) )
		{
			QMessageBox::message(
				tr( "UltraScan Error:" ),
				tr( "Unable to retrieve Raw data files." ) );
		}
	}
	*/

	US_Gzip gzip;
	US_Tar  tar;
	int     ret;

	// Uncompress report
	QString targzfile = run_id + "_report.tar.gz";

	chdir( reportDir.latin1() );

	pd->setProgress( 4 );
	qApp->processEvents();

	if ( ( ret = gzip.gunzip( targzfile ) ) != TAR_OK )
	{
		pd->close();
		delete pd;

		QMessageBox::message(
			tr( "UltraScan Error:" ),
			tr( "Unable to uncompress tar archive.\n" +
			     gzip.explain( ret ) + "\n" +
			     reportDir + targzfile ) );

		// Clean up
		cleanCompressFile();
		return;
	}

	// extract report file

	pd->setProgress( 5 );
	qApp->processEvents();

	tarfile = run_id + "_report.tar";

	if ( ( ret = tar.extract( tarfile ) ) != GZIP_OK )
	{
		pd->close();
		delete pd;

		QMessageBox::message(
			tr( "UltraScan tar extraction Error:" ),
			tr( tar.explain( ret ) + tr( "\ntarfile: " ) + tarfile ) );

		// Clean up
		cleanCompressFile();
		return;
	}

	// Uncompress results
	targzfile = run_id + "_result.tar.gz";

	chdir( resultDir.latin1() );

	pd->setProgress( 6 );
	qApp->processEvents();

	if ( ( ret = gzip.gunzip( targzfile ) ) != TAR_OK )
	{
		pd->close();
		delete pd;

		QMessageBox::message(
			tr( "UltraScan Error:" ),
			tr( "Unable to uncompress tar archive.\n" +
			     gzip.explain( ret ) + "\n" +
			     resultDir + targzfile ) );

		// Clean up
		cleanCompressFile();
		return;
	}

	// extract result file

	pd->setProgress( 7 );
	qApp->processEvents();

	tarfile = run_id + "_result.tar";

	if ( ( ret = tar.extract( tarfile ) ) != GZIP_OK )
	{
		pd->close();
		delete pd;

		QMessageBox::message(
			tr( "UltraScan tar extraction Error:" ),
			tr( tar.explain( ret ) + tr( "\ntarfile: " ) + tarfile ) );

		// Clean up
		cleanCompressFile();
		return;
	}

	/*
	// Uncompress data
	targzfile = run_id + "_rawdata.tar.gz";

	chdir( dataDir.latin1() );

	pd->setProgress( 8 );
	qApp->processEvents();

	if ( ( ret = gzip.gunzip( targzfile ) ) != TAR_OK )
	{
		pd->close();
		delete pd;

		QMessageBox::message(
			tr( "UltraScan Error:" ),
			tr( "Unable to uncompress tar archive.\n" +
			     gzip.explain( ret ) + "\n" +
			     dataDir + targzfile ) );

		// Continue
		goto finish;
	}

	// extract raw data file

	pd->setProgress( 9 );
	qApp->processEvents();

	tarfile = run_id + "_rawdata.tar";

	if ( ( ret = tar.extract( tarfile ) ) != GZIP_OK )
	{
		pd->close();
		delete pd;

		QMessageBox::message(
			tr( "UltraScan tar extraction Error:" ),
			tr( tar.explain( ret ) + tr( "\ntarfile: " ) + tarfile ) );

		// Continue
	}

finish:
	*/
	// Finished

	QMessageBox::message(
		tr( "Success" ), 
		tr( "Data retrieved successfully!" ) );	

	pd->close();
	delete pd;
	cleanCompressFile();
	from_DB = true;
}

/*!  Open US_DB_Admin to check delete permission.  */
void US_DB_Equil::check_permission( void )
{
	US_DB_Admin* db_admin = new US_DB_Admin( "" );
	db_admin->show();
	connect( db_admin, SIGNAL( issue_permission( bool ) ),
	                   SLOT  ( delete_db       ( bool ) ) );
}

/*!  Delete one selected experimental result table entry.  */
void US_DB_Equil::delete_db( bool permission )
{
	if ( ! permission )
	{
		QMessageBox::message(
			tr( "Attention:" ), 
			tr( "Permission denied, please contact your system "
			    "administrator for help" ) );

		return;
	}

	if ( ! del_flag )
	{
		QMessageBox::message(
			tr( "Attention:" ), 
			tr( "Please select a record\n"
			    "to be deleted from the database" ) );
	}
	else
	{
		QSqlCursor cursor( "tblEquilResult" );
		cursor.setMode( QSqlCursor::Delete );
		
		QSqlIndex filter = cursor.index( "EquilRstID" );
		cursor.setValue( "EquilRstID", exp_rst.expRstID );
		cursor.primeDelete();

		switch( QMessageBox::information( this,
			tr( "Delete this record?" ), 
			tr( "Clicking 'OK' will delete the selected result data from "
			    "the database" ),
			tr( "OK" ), tr( "CANCEL" ),	0, 1 ) )
		{
			case 0:
			{
				QSqlQuery query;
				QString   q;
				q.sprintf( "UPDATE tblResult "
				           "SET EquilDataID = NULL WHERE EquilDataID = %d;", 
				           exp_rst.expRstID );
				query.exec( q );

				if ( DataID > 0 )
				{
					QSqlQuery del_file;
					q.sprintf( "DELETE FROM tblEquilResultData WHERE tableID = %d;", 
						DataID );
					
					query.exec( q );
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
}

void US_DB_Equil::display()
{
	QString indexFile;
	QString reportDir = USglobal->config_list.html_dir + "/";

	if ( from_HD || from_DB )
		indexFile = reportDir + run_id + "/index.html";
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

void US_DB_Equil::reset()
{
	cleanCompressFile();

	from_HD          = false;
	from_DB          = false;
	query_flag       = false;
	retrieve_flag    = false;
	del_flag         = false;
	
	run_id           = "";
	runrequestID     = 0;
	exp_rst.expRstID = -1;
	exp_rst.invID    = -1;
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
	lbl_runrequest->setText   ( " Not Selected" );
	lbl_expdt     ->setText   ( "" );
	lbl_invst     ->setText   ( "" );
	lbl_dt        ->setText   ( "" );
	le_dscrpt     ->setText   ( "" );
	lb_result     ->clear();	
}

/*! Open a netscape browser to load help page. */
void US_DB_Equil::help( void )
{
	US_Help* online_help = new US_Help(this);
	online_help->show_help( "manual/us_db_result_veloc.html" );
}

/*! Close the interface.*/
void US_DB_Equil::quit( void )
{
	cleanCompressFile();
	close();
}

/*!  Event handler, for event <var>e</var>, to receive widget close events. */
void US_DB_Equil::closeEvent( QCloseEvent* e )
{
	cleanCompressFile();
	e->accept();
}

