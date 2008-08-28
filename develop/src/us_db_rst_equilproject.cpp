#include "../include/us_db_rst_equilproject.h"
#include "../include/us_gzip.h"
#include "../include/us_tar.h"

#ifdef WIN32
	#define chdir _chdir
	#include <direct.h>
#endif

US_DB_RST_EquilProject::US_DB_RST_EquilProject( QWidget* p, const char* name )
  : US_DB_Widgets( p, name )
{
	from_HD          = false;
	from_DB          = false;
	query_flag       = false;
	projectID        = 0;
	investigatorID   = 0;
	DataID           = 0;
	runrequestID     = 0;
	item_projectID   = NULL;
	item_projectName = NULL;
 	
	int minHeight    = 26;

	pb_hd = pushbutton( "Load Result from HD" );
	pb_hd->setMinimumHeight( minHeight ) ;
	connect( pb_hd, SIGNAL( clicked() ), SLOT( load_HD() ) );

	pb_load_db = pushbutton( "Query Result from DB" );
	pb_load_db->setMinimumHeight( minHeight ) ;
	connect( pb_load_db, SIGNAL( clicked() ), SLOT( load_DB() ) );

	pb_retrieve_db = pushbutton( "Retrieve Result from DB" );
	pb_retrieve_db->setMinimumHeight( minHeight ) ;
	connect( pb_retrieve_db, SIGNAL( clicked() ), SLOT( retrieve_db() ) );

	pb_investigator = pushbutton( "Investigator ID:" );
	connect( pb_investigator, SIGNAL( clicked() ), SLOT( sel_investigator() ) );

	lbl_investigator= textlabel( " Not Selected" );
	lbl_investigator->setAlignment( AlignLeft | AlignVCenter );

	pb_runrequest = pushbutton( "Select Run Request" );
	connect( pb_runrequest, SIGNAL( clicked() ), SLOT( sel_runrequest() ) );

	lbl_runrequest= textlabel( " Not Selected" );
	lbl_runrequest->setAlignment( AlignLeft | AlignVCenter );

	lbl_item= label( " Selected Result:", -1 );
	lbl_item->setMinimumHeight( minHeight ) ;
	lbl_item->setAlignment( AlignLeft | AlignVCenter );

	le_item = lineedit( "linebox" );
	le_item->setAlignment( AlignLeft | AlignVCenter );
	le_item->setText( tr(" Not Selected" ) );

	lbl_instr = banner( "Doubleclick on result data to select", -2 );
	lbl_instr->setMinimumHeight( minHeight );

	lb_result = listbox( "result" );
	connect( lb_result, SIGNAL( selected( int ) ), SLOT( select_result( int ) ) );

	pb_save_db = pushbutton( "Save DB" );
	connect( pb_save_db, SIGNAL( clicked() ), SLOT( save_db() ) );

	pb_display = pushbutton( "Show Selected Result" );
	pb_display->setMinimumHeight( minHeight ) ;
	connect( pb_display, SIGNAL( clicked() ), SLOT( display() ) );

	pb_del_db = pushbutton( "Delete DB" );
	connect( pb_del_db, SIGNAL( clicked() ), SLOT( check_permission() ) );

	pb_reset = pushbutton( "Reset" );
	pb_reset->setMinimumHeight( minHeight ) ;
	connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );
	
	pb_help = pushbutton( "Help" );
	pb_help->setMinimumHeight( minHeight ) ;
	connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

	pb_close = pushbutton( "Close" );
	pb_close->setMinimumHeight( minHeight ) ;
	connect( pb_close, SIGNAL( clicked() ), SLOT( quit() ) );

	setup_GUI();
}

void US_DB_RST_EquilProject::setup_GUI( void )
{
	int j       = 0;
	int rows    = 8;
	int column  = 1;
	int spacing = 2;

	QGridLayout* background = new QGridLayout( this, 2, 1, spacing );	
	QGridLayout* subGrid1   = new QGridLayout( rows, column, spacing );

	subGrid1->addWidget( pb_hd,          j, 0 ); j++;
	subGrid1->addWidget( pb_load_db,     j, 0 ); j++;
	subGrid1->addWidget( pb_retrieve_db, j, 0 ); j++;
	subGrid1->addWidget( lbl_instr,      j, 0 ); j++;

	subGrid1->addMultiCellWidget( lb_result, j, j + 4, 0, 0 );

	subGrid1->setRowStretch( j, 4 );

	j      = 0;
	rows   = 9;
	column = 2;

	QGridLayout* subGrid2 = new QGridLayout( rows, column, spacing );

	for ( int i = 0; i < rows; i++ )
	{
		subGrid2->setRowSpacing( i, 26 );
	}

	subGrid2->addMultiCellWidget( pb_save_db, j, j, 0, 1 ); j++;
	subGrid2->addMultiCellWidget( pb_del_db,  j, j, 0, 1 ); j++;
	
	subGrid2->addWidget( pb_runrequest,    j, 0 );
	subGrid2->addWidget( lbl_runrequest,   j, 1 ); j++;
	subGrid2->addWidget( pb_investigator,  j, 0 );
	subGrid2->addWidget( lbl_investigator, j, 1 ); j++;
	subGrid2->addWidget( lbl_item,         j, 0 );
	subGrid2->addWidget( le_item,          j, 1 ); j++;
	
	subGrid2->addMultiCellWidget( pb_display, j, j, 0, 1 ); j++;
	subGrid2->addMultiCellWidget( pb_reset,   j, j, 0, 1 ); j++;
	subGrid2->addMultiCellWidget( pb_help,    j, j, 0, 1 ); j++;
	subGrid2->addMultiCellWidget( pb_close,   j, j, 0, 1 ); j++;
	
	subGrid2->setRowStretch(j,3);
	
	background->addLayout( subGrid1, 0, 0 );
	background->addLayout( subGrid2, 0, 1 );
	background->setColSpacing( 0, 450 );
	background->setColStretch( 0, 3 );

	background->activate();
}

/*!  Open US_DB_TblInvestigator interface for selecting investigator.  */
void US_DB_RST_EquilProject::sel_investigator( void )
{
	US_DB_TblInvestigator *investigator_dlg = new US_DB_TblInvestigator();
	
	investigator_dlg->setCaption( "Investigator Information" );
	investigator_dlg->pb_exit->setText( "Accept" );
	investigator_dlg->show();
	
	connect( investigator_dlg, SIGNAL( valueChanged           ( QString, int ) ), 
	                           SLOT  ( update_investigator_lbl( QString, int ) ) );
}

/*!	Update display with the selected investigator information.
	\param Display a string variable for show investigator info.
	\param InvID a integer variable for DB table: <tt>tblInvestigators</tt> index.
*/
void US_DB_RST_EquilProject::update_investigator_lbl( 
		QString Display, int InvID )
{
	investigatorID = InvID;

	if ( Display == "" )
		lbl_investigator->setText( " Not Selected" );
	else
		lbl_investigator->setText( Display );
}

void US_DB_RST_EquilProject::sel_runrequest()
{
	US_DB_RunRequest* runrequest_dlg = new US_DB_RunRequest();

	runrequest_dlg->setCaption( "Run Request Information" );
	runrequest_dlg->show();

	connect(runrequest_dlg, SIGNAL( issue_RRID           ( int ) ), 
	                        SLOT  ( update_runrequest_lbl( int ) ) );
}

void US_DB_RST_EquilProject::update_runrequest_lbl ( int RRID )
{
	runrequestID = RRID;

	if ( runrequestID == 0 )
		lbl_runrequest->setText( " Not Selected" );
	else
		lbl_runrequest->setNum( runrequestID );
}

void US_DB_RST_EquilProject::load_HD( void )
{
	QString reportDir = USglobal->config_list.html_dir   + "/";
	QString resultDir = USglobal->config_list.result_dir + "/";

	reset();

	QString fileName = 
		QFileDialog::getOpenFileName( resultDir, "*.eq-project", 0 );

	QFile projectFile( fileName );
	projectFile.open( IO_ReadOnly );

	QTextStream project_ts( &projectFile );

	projectName = project_ts.readLine();

	// Skip four lines
	project_ts.readLine();
	project_ts.readLine();
	project_ts.readLine();
	project_ts.readLine();

	QString InvID = project_ts.readLine();
	projectFile.close();

	if ( fileName.isEmpty() )
	{
		QMessageBox::message(
			tr( "Attention:" ), 
			tr( "No file Selected." ) );

		return;
	}
	else
	{	
		lb_result->insertItem(tr( "Data is loaded from Hard Drive" ) );
		le_item->setText( projectName );

		US_Report_EquilProject* report_equilproject = 
			new US_Report_EquilProject( false );
		
		report_equilproject->generate( fileName );

		if ( InvID.toInt() > 0 )
		{
			lbl_investigator->setText(show_investigator( InvID.toInt() ) );
			pb_investigator ->setEnabled( false );
			investigatorID = InvID.toInt();
		}

		from_HD = true;
	}
}

void US_DB_RST_EquilProject::save_db( void )
{
	if ( ! from_HD )
	{
		QMessageBox::message(
			tr( "Attention:" ),
			tr( "Please load result data from harddrive first.\n" ) );

		return;
	}

	if ( investigatorID <= 0 )
	{
		QMessageBox::message(
			tr( "Attention:" ), 
			tr( "Please select investigator first.\n" ) );

		return;
	}

	//Insert into EquilProjectData

	QProgressDialog* pd = progressdialog(
		"Please wait while your data is saved to the database...", "pd", 8 );

	pd->setProgress       ( 0 );
	pd->setMinimumDuration( 0 );

	QString reportDir = USglobal->config_list.html_dir   + "/";
	QString resultDir = USglobal->config_list.result_dir + "/";

	QStringList files;
	US_Gzip     gzip;
	US_Tar      tar;
	int         ret;

	// Build report tar file

	QString tarfile = projectName + "_report.tar";
	files << projectName;  // A directory

	pd->setLabelText( "Compressing Report Data..." );
	qApp->processEvents();

	chdir( reportDir.latin1() );

	if ( ( ret = tar.create( tarfile, files ) ) != GZIP_OK )
	{
		pd->close();
		delete pd;
		QMessageBox::message(
			tr( "UltraScan tar creation Error: Report" ),
			tr( tar.explain( ret ) + tr( "\nInput files:\n" ) + files.join( "\n") ) );

		// Remove the tar file
		cleanCompressFile();
		return;
	}

	// gzip report tar file	

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

		cleanCompressFile();
		return;
	}

	// Tar result tar file

	pd->setLabelText( "Compressing Result Data..." );
	pd->setProgress( 3 );
	qApp->processEvents();

	tarfile = projectName + "_result.tar";

	QDir work( resultDir );
	work.setNameFilter( projectName + "*" );
	files = work.entryList();

	chdir( resultDir.latin1() );

	if ( ( ret = tar.create( tarfile, files ) ) != GZIP_OK )
	{
		pd->close();
		delete pd;
		QMessageBox::message(
			tr( "UltraScan tar creation Error: Result" ),
			tr( tar.explain( ret ) + tr( "\nInput files:\n" ) + files.join( "\n") ) );

		// Remove the tar file
		cleanCompressFile();
		return;
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

		return;
	}

	// Insert data into DB table EquilProjectData

	QSqlCursor cursor( "EquilProjectData" );
	cursor.setMode( QSqlCursor::Insert );

	QSqlRecord* buffer = cursor.primeInsert();
	DataID = get_newID( "EquilProjectData", "tableID" );

	buffer->setValue( "tableID", DataID );

	pd->setLabelText( "Uploading Data..." );
	pd->setProgress( 5 );
	qApp->processEvents();

	QString fileName = resultDir + projectName + ".eq-project";
	write_blob( fileName, buffer, "HeadFile" );

	fileName = reportDir + projectName + "_report.tar.gz";
	write_blob( fileName, buffer, "Report_Tar" );

	pd->setProgress( 6 );
	qApp->processEvents();

	fileName = resultDir + projectName + "_result.tar.gz";
	write_blob( fileName, buffer, "Result_Tar");

	cursor.insert();

	// Insert data into DB table EquilProjectResult

	int     id = get_newID( "EquilProjectResult", "EquilProjectID" );
	QString q  = "INSERT INTO EquilProjectResult "
	             "(EquilProjectID, ProjectName, InvestigatorID, DataID";

	if ( runrequestID > 0 )  q += ", RunRequestID";

	q += ") VALUES("
	  +  QString::number( id )             + ", '"
	  +  projectName                       + "', "
	  +  QString::number( investigatorID ) + ", "
	  +  QString::number( DataID );

	if ( runrequestID > 0 ) q += ", " + QString::number( runrequestID );

	q += ");";

	QSqlQuery target;

	if ( ! target.exec( q ) )
	{
		QSqlError sqlerr = target.lastError();
		QMessageBox::message(
			tr( "Attention:" ), 
			tr( "Saving to DB table 'EquilProjectResult' failed.\n"
			    "Attempted to execute this command:\n\n" )
			    + q + "\n\n"
			    + tr( "Causing the following error:\n\n")
			    + sqlerr.text() ) ;

		// Delete inserted EquilProjectData data

		q.sprintf( "DELETE FROM EquilProjectData WHERE tableID = %d;", DataID );
		target.exec( q );

		// Clean temporary files
				
		cleanCompressFile();
		pd->close();
		delete pd;
		return;
	}

	if ( runrequestID > 0 ) // Update tblResult
	{
		int resultID = 0;
		q.sprintf("SELECT ResultID FROM tblResult WHERE RunRequestID = %d;", runrequestID);
		target.exec( q );

		if ( target.next() ) resultID = target.value( 0 ).toInt(); 

		if ( resultID > 0 )  // row exists; update equilprojectID
		{
			q.sprintf( "UPDATE tblResult SET "
			           "InvestigatorID = %d, EquilProjectID = %d "
			           "WHERE ResultID = %d;", investigatorID, id, resultID );

			target.exec( q );
		}
		else  // create new entry
		{
			resultID = get_newID( "tblResult", "ResultID" );
			q        = "INSERT INTO tblResult "
			           "(ResultID, InvestigatorID, RunRequestID, EquilProjectID)"
			           " VALUES("
			         + QString::number( resultID )       + ", "
			         + QString::number( investigatorID ) + ", " 
			         + QString::number( runrequestID )   + ", " 
			         + QString::number( id )             + ") ";

			if ( ! target.exec( q ) )
			{
				QMessageBox::message(
					tr( "Attention:" ), 
					tr( "Save to tblResult failed.\n" ) );

				pd->close();
				delete pd;
				return;
			}
		} 
	}

	cleanCompressFile();
	pd->close();
	delete pd;

	QMessageBox::message(
		tr( "Success:" ),
		tr( "The data was saved to the database.\n" ) );
}

void US_DB_RST_EquilProject::load_DB( void )
{
	clear();
	 
	int maxID = get_newID( "EquilProjectResult", "EquilProjectID" );
	int count = 0;

	if ( item_projectID   ) delete [] item_projectID;
	if ( item_projectName ) delete [] item_projectName;

	item_projectID       = new int    [ maxID ];
	item_projectName     = new QString[ maxID ];
	QString* display_Str = new QString[ maxID ];

	QString q;

	if ( investigatorID > 0 )
	{
		q.sprintf( "SELECT EquilProjectID, ProjectName FROM EquilProjectResult "
		           "WHERE InvestigatorID = %d;", investigatorID );
	}
	else
	{
		q = "SELECT EquilProjectID, ProjectName FROM EquilProjectResult;";
	}

	QSqlQuery query( q );

	while ( query.next() )
	{
		item_projectID  [ count ] = query.value( 0 ).toInt();
		item_projectName[ count ] = query.value( 1 ).toString();

		QString ID                = QString::number( item_projectID[ count ] );
		display_Str     [ count ] = "(" + ID + "): " + item_projectName[ count ];
		count++;
	}

	if ( count > 0 )
	{
		lb_result->clear();

		for ( int i = 0; i < count; i++ )
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


void US_DB_RST_EquilProject::select_result( int item )
{ 
	if ( query_flag )
	{
		projectID   = item_projectID  [ item ];
		projectName = item_projectName[ item ];
		
		QString s = "("+ QString::number( projectID ) + ") " + projectName;
		le_item->setText( s );

		s = "SELECT DataID, InvestigatorID, RunRequestID "
		    "FROM EquilProjectResult WHERE EquilProjectID = " +
		     QString::number( projectID );

		QSqlQuery query( s );
			 
		if ( query.next() )
		{
			DataID         = query.value( 0 ).toInt();
			investigatorID = query.value( 1 ).toInt();
			runrequestID   = query.value( 2 ).toInt();

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
			else
			{
				lbl_runrequest->setText( " N/A" );
			}
		}
	}
	else
	{
		QMessageBox::message(
			tr( "UltraScan Error:" ), 
			tr( "Data from DB table 'EquilProjectResult' not loaded." ) );

		return;
	}

	from_DB = true;
}

void US_DB_RST_EquilProject::retrieve_db()
{
	if ( DataID <= 0 ) return;

	QString resultDir = USglobal->config_list.result_dir + "/";
	QString reportDir = USglobal->config_list.html_dir   + "/";
	QString q;
	QString tarfile;
	
	QProgressDialog* pd = progressdialog( 
		"Retrieving Equilproject Data", "pd", 9 );
		  
  pd->setProgress( 1 );
  qApp->processEvents();

	QSqlCursor query( "EquilProjectData" );
	q.sprintf( "tableID = %d", DataID );

	query.select( q );

	if ( query.next() )
	{
		tarfile          = projectName + "_report.tar.gz";				 
		QString filename = make_tempFile( reportDir, tarfile );

		if ( ! read_blob( "Report_Tar", query, filename ) )
		{
  		pd->close();
			delete pd;

			QMessageBox::message(
				tr( "UltraScan Error:" ), 
				tr( "Unable to retrieve Report files." ) );

			cleanCompressFile();    
			return;
		}

  	pd->setProgress( 2 );
  	qApp->processEvents();

		tarfile  = projectName + "_result.tar.gz";
		filename = make_tempFile( resultDir, tarfile );	 

		if ( ! read_blob( "Result_Tar", query, filename ) )
		{
  		pd->close();
			delete pd;

			QMessageBox::message(
				tr( "UltraScan Error:"),
				tr( "Unable to retrieve Result files." ) );

			cleanCompressFile();    
			return;
		}
	}

	US_Gzip gzip;
	US_Tar  tar;

	// Uncompress report
	QString targzfile = projectName + "_report.tar.gz";

	chdir( reportDir.latin1() );
	pd->setLabelText( "Uncompressing Report Data..." );
	pd->setProgress( 4 );
	qApp->processEvents();

	if ( gzip.gunzip( targzfile ) != TAR_OK )
	{
		QMessageBox::message(
			tr( "UltraScan Error:" ),
			tr( "Unable to uncompress tar archive.\n" +
			    reportDir + targzfile ) );
	
		goto result;		
		;
	}

	pd->setProgress( 5 );
	qApp->processEvents();

	tarfile = projectName + "_report.tar";
	int     ret;

	if ( ( ret = tar.extract( tarfile ) ) != GZIP_OK )
	{
		QMessageBox::message(
		tr( "UltraScan tar extraction Error:" ),
		tr( tar.explain( ret ) + tr( "\ntarfile: " ) + tarfile ) );
	}

result:

	// Uncompress result
	targzfile = projectName + "_result.tar.gz";

	chdir( resultDir.latin1() );
	pd->setLabelText( "Uncompressing Result Data..." );
	pd->setProgress( 6 );
	qApp->processEvents();

	if ( gzip.gunzip( targzfile ) != TAR_OK )
	{
		pd->close();
		delete pd;

		QMessageBox::message(
			tr( "UltraScan Error:" ),
			tr( "Unable to uncompress tar archive.\n" +
			    resultDir + targzfile ) );
	
		cleanCompressFile();
		return;
	}

	pd->setProgress( 7 );
	qApp->processEvents();

	tarfile = projectName + "_result.tar";

	if ( ( ret = tar.extract( tarfile ) ) != GZIP_OK )
	{
		QMessageBox::message(
		tr( "UltraScan tar extraction Error:" ),
		tr( tar.explain( ret ) + tr( "\ntarfile: " ) + tarfile ) );
	}
	else
	{
		QMessageBox::message( 
			tr( "Success,") , 
			tr( "Data retrieved." ) );
	}

	pd->close();
	delete pd;
	cleanCompressFile();
}

void US_DB_RST_EquilProject::display( void )
{
	QString indexFile;

	if ( from_HD || from_DB )
	{
	   indexFile = USglobal->config_list.html_dir + "/" 
		           + projectName + "/index.html";
	}
	else
	{
		QMessageBox::message(
			tr( "Attention:" ),
			tr( "Please first select the result\n" ) );

		return;
	}

	US_Help* online_help = new US_Help();
	online_help->show_html_file( indexFile );
}

/*!  Open US_DB_Admin to check delete permission.  */

void US_DB_RST_EquilProject::check_permission( void )
{
	US_DB_Admin* db_admin = new US_DB_Admin( "" );
	db_admin->show();
	connect( db_admin, SIGNAL( issue_permission( bool ) ), 
	                   SLOT  ( delete_db       ( bool ) ) );
}

/*!  Delete one selected experimental result table entry.  */

void US_DB_RST_EquilProject::delete_db( bool permission )
{
	if ( ! permission )
	{
		QMessageBox::message(
			tr( "Attention:" ), 
			tr( "Permission denied, please contact your system administrator "
			     "for help" ) );

		return;
	}

	if ( ! from_DB )
  {
		QMessageBox::message(
			tr("Attention:"), 
			tr( "Please select the record to\n"
			    "be deleted from the database"));
	}
	else
	{
		switch( QMessageBox::information( this, 
			tr( "Delete this record?" ), 
			tr( "Clicking 'OK' will delete the selected result from the database" ),
			tr( "OK"), tr( "CANCEL" ), 0, 1 ) )
		{
			case 0:
			{
				QSqlQuery query;
				QString q;

				q.sprintf( "UPDATE tblResult SET EquilProjectID = NULL "
				           "WHERE EquilProjectID = %d;", projectID );

				query.exec( q );

				q.sprintf( "DELETE FROM EquilProjectResult "
				           "WHERE EquilProjectID = %d;", projectID );

				// Delete all relative equilproject data
				if ( query.exec( q ) )
				{
					if ( DataID > 0 )
					{
						q.sprintf( "DELETE FROM EquilProjectData WHERE tableID = %d;", 
						           DataID );
						query.exec( q );
					}
				}

				reset();
				break;
			}
            
			case 1:
				break;
		}
	}
}

void US_DB_RST_EquilProject::clear( void )
{
	cleanCompressFile();    

	from_HD      = false;
	from_DB      = false;
	query_flag   = false;
	projectID    = 0;
	DataID       = 0;
	runrequestID = 0;

	lb_result      ->clear();

	pb_investigator->setEnabled( true );
	pb_runrequest  ->setEnabled( true );

	lbl_runrequest ->setText( " Not Selected" );
	le_item        ->setText( " Not Selected" );
}

void US_DB_RST_EquilProject::reset( void )
{
	clear();
	investigatorID = 0;
	lbl_investigator->setText( " Not Selected" );
}

/*! Open a netscape browser to load help page.*/
void US_DB_RST_EquilProject::help( void )
{
	US_Help *online_help = new US_Help( this );
	online_help->show_help( "manual/us_db_result_equilproject.html" );
}

void US_DB_RST_EquilProject::cleanCompressFile( void )
{
	QString reportDir = USglobal->config_list.html_dir   + "/";
	QString resultDir = USglobal->config_list.result_dir + "/";

	QStringList files;

	files << reportDir + projectName + "_report.tar.gz"
	      << reportDir + projectName + "_report.tar"
	      << resultDir + projectName + "_result.tar.gz"
	      << resultDir + projectName + "_result.tar";

	QStringList::iterator it = files.begin();

	while ( it != files.end() )
	{
		QFile tempFile( *it++ );
		if ( tempFile.exists() ) tempFile.remove();
	}
}

/*! Close the interface.*/
void US_DB_RST_EquilProject::quit( void )
{
	cleanCompressFile();
	close();
}

/*! Event handler, for event <var>e</var>, to receive widget close events. */
void US_DB_RST_EquilProject::closeEvent( QCloseEvent* e )
{
	cleanCompressFile();
	e->accept();
}

