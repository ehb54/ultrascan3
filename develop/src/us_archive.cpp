#ifndef WIN32
  #include <sys/statfs.h>
#else
  // We don't need windows.h unless we are trying to get the free
  // space on the partition at the end of viewtargz()
  //#include <windows.h>  // Must be first for windows to build
  #include <direct.h>      
  #define chdir _chdir
#endif

#include "../include/us_archive.h"
#include "../include/us_tar.h"
#include "../include/us_gzip.h"

US_Archive::US_Archive( QWidget* p, const char* name ): US_Widgets( p, name )
{
	int       xpos     =   2;
	int       ypos     =   2;
	const int spacing  =   2;
	const int buttonh  =  26;
	const int buttonw0 = 220;
	const int buttonw  = 109;
	const int buttonw1 = 170;

	filename       = "";
	reports_flag   = true;
	ultrascan_flag = true;
	delete_flag    = true;

	setCaption( tr( "UltraScan Archive Manager" ) );

	mle = textedit();

	banner1 = banner( "Archive Information:" );
	banner1->setGeometry( xpos, ypos, buttonw0, buttonh );

	ypos += buttonh + spacing + 2;

	pb_view = pushbutton( "View Archive Contents" );
	connect( pb_view, SIGNAL( clicked() ), SLOT( view() ) );

	ypos += buttonh + spacing;

	lbl_name1 = label( " Archive:" );
	lbl_name1->setAlignment( AlignLeft | AlignVCenter);
	lbl_name1->setGeometry( xpos, ypos, buttonw, buttonh );

	xpos += spacing + buttonw;

	lbl_name2 = textlabel( "", -1 );
	lbl_name2->setAlignment( AlignLeft | AlignVCenter);
	lbl_name2->setGeometry( xpos, ypos, buttonw, buttonh );

	ypos += buttonh + spacing + 2;
	xpos = spacing;

	banner2 = banner( "Archive Creation:" );
	banner2->setGeometry(xpos, ypos, buttonw0, buttonh);

	ypos += buttonh + spacing  +  2;
	xpos += spacing + buttonw1 + 18;

	cb_ultrascan = checkbox( "Include UltraScan Data", ultrascan_flag );
	cb_ultrascan->setGeometry( xpos, ypos + 5, 14, 14 );

	connect( cb_ultrascan, SIGNAL( clicked() ), SLOT( set_ultrascan() ) );

	ypos += buttonh + spacing;
	xpos  = spacing;
	xpos += spacing + buttonw1 + 18;

	cb_reports = checkbox( "Include HTML Reports", reports_flag );
	cb_reports->setGeometry( xpos, ypos + 5, 14, 14 );
	connect( cb_reports, SIGNAL( clicked() ), SLOT( set_reports() ) );

	ypos += buttonh + spacing;
	xpos  = spacing;
	xpos += spacing + buttonw1 + 18;

	cb_delete = checkbox( "Delete Original Data", delete_flag );
	cb_delete->setGeometry( xpos, ypos + 5, 14, 14 );
	connect( cb_delete, SIGNAL( clicked() ), SLOT( set_delete() ) );

	ypos += buttonh + spacing;
	xpos = spacing;

	pb_select_velocdata_create = pushbutton( "Select Velocity Data" );
	pb_select_velocdata_create->setGeometry( xpos, ypos, buttonw0, buttonh );
	connect( pb_select_velocdata_create, SIGNAL( clicked() ), 
	                                     SLOT  ( select_velocdata() ) );

	ypos += buttonh + spacing;

	pb_select_equildata_create = pushbutton( "Select Equilibrium Data" );
	pb_select_equildata_create->setGeometry( xpos, ypos, buttonw0, buttonh );
	connect( pb_select_equildata_create, SIGNAL( clicked() ), 
	                                     SLOT  ( select_equildata() ) );

	ypos += buttonh + spacing;

	pb_select_eqproject_create = pushbutton( "Select Equilibrium Project" );
	pb_select_eqproject_create->setGeometry( xpos, ypos, buttonw0, buttonh );
	connect( pb_select_eqproject_create, SIGNAL( clicked() ), 
	                                     SLOT  (select_eqilproj() ) );

	ypos += buttonh + spacing;
	
	pb_select_montecarlo_create = pushbutton( "Select Monte Carlo Project" );
	pb_select_montecarlo_create->setGeometry( xpos, ypos, buttonw0, buttonh );
	connect( pb_select_montecarlo_create, SIGNAL( clicked() ), 
	                                      SLOT  ( select_montecarlo() ) );

	ypos += buttonh + spacing;

	lbl_create_name1 = label( " Archive:", -1, QFont::Bold );
	lbl_create_name1->setAlignment( AlignLeft | AlignVCenter);
	lbl_create_name1->setGeometry( xpos, ypos, buttonw, buttonh );

	xpos += spacing + buttonw;

	lbl_create_name2 = textlabel( "", -1 );
	lbl_create_name2->setGeometry( xpos, ypos, buttonw, buttonh );
	lbl_create_name2->setAlignment( AlignLeft | AlignVCenter);

	ypos += buttonh + spacing;
	xpos = spacing;

	pb_create = pushbutton( "Create Archive", false );
	pb_create->setGeometry( xpos, ypos, buttonw0, buttonh );
	connect( pb_create, SIGNAL( clicked() ), SLOT( create_archive() ) );

	ypos += buttonh + spacing + 2;

	banner3 = banner( "Archive Extraction:" );
	banner3->setGeometry( xpos, ypos, buttonw0, buttonh );

	ypos += buttonh + spacing + 2;

	pb_extract = pushbutton( "Extract Archive" );
	pb_extract->setGeometry( xpos, ypos, buttonw0, buttonh );
	connect( pb_extract, SIGNAL( clicked() ), SLOT( extract_archive() ) );

	ypos += buttonh + spacing + 2;

	banner4 = banner( "Module Controls:" );
	banner4->setGeometry( xpos, ypos, buttonw0, buttonh );

	ypos += buttonh + spacing + 2;

	pb_help = pushbutton( "Help" );
	pb_help->setGeometry( xpos, ypos, buttonw0, buttonh );
	connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

	ypos += buttonh + spacing;

	pb_close = pushbutton( "Close" );
	pb_close->setGeometry( xpos, ypos, buttonw0, buttonh );
	connect( pb_close, SIGNAL(clicked() ), SLOT( close() ) );

	ypos += buttonh + spacing;

	global_Xpos += 30;
	global_Ypos += 30;

	setMinimumSize( 900, ypos );
	setGeometry( global_Xpos, global_Ypos, 900, ypos );

	setup_GUI();
}

void US_Archive::setup_GUI()
{
	int       j       =  0;
	const int rows    = 18;
	const int columns =  2;
	const int spacing =  2;

	QGridLayout* subGrid1   = new QGridLayout( rows, columns, spacing );
	
	for ( int i = 0; i < rows; i++ )
	{
		subGrid1->setRowSpacing( i, 26 );
	}
	subGrid1->addMultiCellWidget( banner1,      j, j, 0, 1 ); j++;
	subGrid1->addMultiCellWidget( pb_view,      j, j, 0, 1 ); j++;

	subGrid1->addWidget         ( lbl_name1, j, 0 );
	subGrid1->addWidget         ( lbl_name2, j, 1 ); j++;
	
	subGrid1->addMultiCellWidget( banner2,      j, j, 0, 1 ); j++;
	subGrid1->addMultiCellWidget( cb_ultrascan, j, j, 0, 1 ); j++;
	subGrid1->addMultiCellWidget( cb_reports,   j, j, 0, 1 ); j++;
	subGrid1->addMultiCellWidget( cb_delete,    j, j, 0, 1 ); j++;
	
	subGrid1->addMultiCellWidget( pb_select_velocdata_create,  j, j, 0, 1); j++;
	subGrid1->addMultiCellWidget( pb_select_equildata_create,  j, j, 0, 1); j++;
	subGrid1->addMultiCellWidget( pb_select_eqproject_create,  j, j, 0, 1); j++;
	subGrid1->addMultiCellWidget( pb_select_montecarlo_create, j, j, 0, 1); j++;
	
	subGrid1->addWidget         ( lbl_create_name1, j, 0 );
	subGrid1->addWidget         ( lbl_create_name2, j, 1 ); j++;
	
	subGrid1->addMultiCellWidget( pb_create,    j, j, 0, 1 ); j++;
	subGrid1->addMultiCellWidget( banner3,      j, j, 0, 1 ); j++; 
	subGrid1->addMultiCellWidget( pb_extract,   j, j, 0, 1 ); j++;
	subGrid1->addMultiCellWidget( banner4,      j, j, 0, 1 ); j++;
	subGrid1->addMultiCellWidget( pb_help,      j, j, 0, 1 ); j++;
	subGrid1->addMultiCellWidget( pb_close,     j, j, 0, 1 );

	QGridLayout* background = new QGridLayout( this, 1, 2, spacing );
	background->addLayout    ( subGrid1, 0, 0 );
	background->addWidget    ( mle,      0, 1 );
	background->setColStretch( 0, 1 );
	background->setColStretch( 1, 4);
	background->setColSpacing( 1, 680 );

	qApp->processEvents();
	
	QRect r = background->cellGeometry( 0, 0) ;

	global_Xpos += 30;
	global_Ypos += 30;

	this->setGeometry( global_Xpos, global_Ypos, r.width() + 680, r.height() );
}

void US_Archive::closeEvent( QCloseEvent* e )
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}

void US_Archive::set_ultrascan()
{
	ultrascan_flag = ! ultrascan_flag;
	cb_ultrascan->setChecked( ultrascan_flag );
}

void US_Archive::set_delete()
{

	delete_flag = ! delete_flag;
	cb_delete->setChecked( delete_flag );
}

void US_Archive::set_reports()
{
	reports_flag = ! reports_flag;
	cb_reports->setChecked( reports_flag );
}

void US_Archive::quit( void )
{
	close();
}

void US_Archive::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/archive.html");
}

void US_Archive::select_velocdata( void )
{
	select_create_archive( 0 );
}

void US_Archive::select_equildata( void )
{
	select_create_archive( 1 );
}

void US_Archive::select_eqilproj( void )
{
	select_create_archive( 2 );
}

void US_Archive::select_montecarlo( void )
{
	select_create_archive( 3 );
}

void US_Archive::select_create_archive( const int type )
{
	QString filter;
	QString dir = USglobal->config_list.result_dir;

	pb_create->setEnabled( false );
	run_type = type;

	switch ( type )
	{
		case 0:
			filter = "*.us.v";
			break;

		case 1:
			filter = "*.us.e";
			break;

		case 2:
	    filter = "*.eq-project";
			break;
		
		case 3:
	    filter = "*.Monte-Carlo";
			break;
	}

	QString fn = QFileDialog::getOpenFileName( dir, filter, 0 );
	if ( fn == "" ) return;

	switch ( type )
	{
		case 0:
		case 1:
		{
			QFile f( fn );
			if ( ! f.open( IO_ReadOnly ) )
			{
	  		QMessageBox::message(
					tr( "Ultrascan Error:" ),
					tr( "Could not open file:" ) + fn );

	  		return; 
			}

			QDataStream ds ( &f );
			if ( ds.atEnd() )
			{
	  		f.close();
	  		QMessageBox::message(
					tr( "Ultrascan Error:" ),
					tr( "The file is empty: " ) + fn );
		
				return; // empty file
			}
	
			// The filename we want is the third entry
			QString version;
			QString datadir;

			ds >> version;
			if ( version.toFloat() < 6.0 )
			{
				QMessageBox::message(
					tr( "Ultrascan Warning:" ),
					tr( "These data were edited with an usupported release\n"
							"of UltraScan (version " + version + "), which is not\n"
							"binary compatible with the current version (" + US_Version + 
							").\n\n"
							"Please re-edit the experimental data before\n"
							"using the data for data analysis." ) );
			}

			ds >> datadir;
			ds >> filename;
			f.close();

			lbl_create_name2->setText( filename );
		}
		break;
		
		case 2:
		case 3:
		{
			QFile projectFile( fn );
			if ( ! projectFile.open( IO_ReadOnly ) )
			{
				QMessageBox::message(
					tr( "Ultrascan Error:" ),
					tr( "Could not open " ) + QDir::convertSeparators( fn ) );
				
				return;
			};

			QTextStream project_ts( &projectFile );
			projectName = project_ts.readLine();
			projectFile.close();

			lbl_create_name2->setText( projectName );
		}
		break;	
	}		
		
	clean_temp_dir();
	pb_create->setEnabled( true );
}

void US_Archive::clean_temp_dir()
{
	QDir temp_dir;
	QString str = USglobal->config_list.root_dir + "/temp";

	temp_dir.setPath( str );

	if ( temp_dir.exists() )
	{
		QStringList entries;
		temp_dir.setNameFilter( "*.*" );
		entries = temp_dir.entryList();

		QStringList::Iterator it;
		for ( it = entries.begin(); it != entries.end(); ++it )
		{
				temp_dir.remove((*it).latin1());
		}
	}
	else
	{
		temp_dir.mkdir( str );
	}
}

void US_Archive::disable_buttons()
{
	pb_view                    ->setEnabled(false);
	pb_extract                 ->setEnabled(false);
	pb_select_equildata_create ->setEnabled(false);
	pb_select_velocdata_create ->setEnabled(false);
	pb_select_montecarlo_create->setEnabled(false);
	pb_select_eqproject_create ->setEnabled(false);
	pb_create                  ->setEnabled(false);
}

void US_Archive::enable_buttons()
{
	pb_view                    ->setEnabled(true);
	pb_extract                 ->setEnabled(true);
	pb_select_equildata_create ->setEnabled(true);
	pb_select_velocdata_create ->setEnabled(true);
	pb_select_montecarlo_create->setEnabled(true);
	pb_select_eqproject_create ->setEnabled(true);
}

/******************************************************************************/
void US_Archive::view()
{
	QString fn = QFileDialog::getOpenFileName(
			USglobal->config_list.archive_dir, "*.tar.gz", 0 );

	if ( ! fn.isEmpty() )
	{
		fn.replace( "\\", "/" ); // Convert for WIN32 backslashes
		int slash = fn.findRev( "/" );

		lbl_name2->setText( fn.mid( slash + 1, fn.length() - ( slash + 1 ) - 7 ) );

		mle->setReadOnly( false );
		mle->clear();
		qApp->processEvents();  
		viewtargz( fn );
		mle->setReadOnly( true );
	}
}

void US_Archive::viewtargz( const QString& fn )
{
#ifndef WIN32
  struct statfs system_info;
	int return_info = statfs( USglobal->config_list.data_dir, &system_info );

	if ( return_info < 0 )
	{
		QMessageBox::message(
		  tr("Attention:"),
			tr("There was an error in function statfs() while\n"
			   "processing the requested information:\n\n") +
			   QString( strerror( return_info ) ) );

		return;
	}
#else
  // Add WIN32 code here
#endif

	QString   s;
	QFileInfo f = QFileInfo( fn );

	mle->append( tr( "<qt><p>Information for archive</p>\n<pre>\"") +
	  QDir::convertSeparators( fn ) + "\"</pre>\n" );

	mle->append( tr( "<p>(compressed size: ") + 
			s.sprintf(tr( "%.2f MBytes)</p>\n" ), (float) f.size() / 1e6) );

	mle->append( 
			tr( "<p>This archive contains the following sub-archives:</p>\n" ) );

	int     start_pos       = fn.findRev( "/" ) + 1;
	QString gzip_filename   = fn.mid( start_pos );
	QString tempdir         = USglobal->config_list.root_dir + "/temp/";
	int     cr              = copy( fn, tempdir + gzip_filename );
	
	if ( cr != 0 )
	{
		QString msg;
		msg.sprintf( "Unable to copy view data to temp dir. %d\n"
		             + fn + "\n" + tempdir + gzip_filename, cr );

		QMessageBox::message(
			tr( "UltraScan Error:" ),
			tr( msg ) );
			return;
	}

	// Save the current directory and cnage to the temp directory
	
	char cwd[ 256 ];
	if ( getcwd( cwd, sizeof cwd ) == 0 )
	{
		QMessageBox::message(
		   tr( "UltraScan Error:" ),
		   tr( "Could not get current working directory" ) );
		return;
	}

	int err = chdir( tempdir.ascii() );
	if ( err != 0 )
	{
	  QString e = QString( "%1" ).arg( err );
		QMessageBox::message(
		   tr( "UltraScan Error:" ),
		   tr( "Could not change working directory\n" ) + 
				   QDir::convertSeparators( tempdir ) + 
				   tr( "\nError number = " ) + e  );
		return;
	}

	qApp->processEvents();  
	
	// gunzip the file
	US_Gzip gzip;
	
	int ret = gzip.gunzip( gzip_filename );

	if ( ret != GZIP_OK )
	{
		QMessageBox::message(
		   tr( "UltraScan gzip Error:" ),
		   tr( gzip.explain( ret ) ) );
		return;
	}

	// Assume gzip_filename is something.tar.gz

	US_Tar      tar;
	QString     tarfile = gzip_filename.left( gzip_filename.length() - 3 );
	QStringList files;

	qApp->processEvents();  

	// Get the listing
	if ( ( ret = tar.list( tarfile, files ) ) != TAR_OK )
	{
		QMessageBox::message(
		   tr( "UltraScan tar Error:" ),
		   tr( tar.explain( ret ) ) );
		return;
	}

	chdir( cwd ); // Restore working directory
	
	// Write out the file data from the tar file
	// Also calculate the total disk space needed
	unsigned int          diskspace = 0;
	QStringList::Iterator it;
	QString               pre_string;
	
	pre_string = "<pre>";
	for ( it = files.begin(); it != files.end(); ++it )
	{
		QString line      = *it;
		pre_string       += line + "\n"; 
		unsigned int size = line.simplifyWhiteSpace().section( ' ', 2, 2 ).toUInt();
		diskspace        += size;
	}

	mle->append( pre_string + "</pre>\n" ); 

	mle->append( tr( "<p>Total diskspace needed for extraction: " +
	    s.sprintf( "%.1f MBytes", diskspace / 1e6 ) + "</p>\n" ) );

#ifndef WIN32
	mle->append( tr( "<p>Total available diskspace on " +
	    QDir::convertSeparators( USglobal->config_list.data_dir ) + ": </p>\n" +
	    s.sprintf( "<p>%.1f MBytes</p>\n", 
				(float) system_info.f_bavail * system_info.f_bsize / 1e6 ) ) );

	if ( diskspace > system_info.f_bavail * system_info.f_bsize )
	{
		mle->append(tr( "<p>ATTENTION: You will need to free up some "
		                "diskspace before extracting this archive!</p>\n"));
	}
#else
  /*  Not figured out yet
  unsigned long int free  = 0;
  unsigned long int total = 0;

  WCHAR wdir[256]; 
  QString dir = "C:\\"; //QDir::convertSeparators( USglobal->config_list.data_dir );
  bool def;

  int nOut = WideCharToMultiByte( CP_ACP, 0, 
    (LPCWSTR)dir.ascii(), dir.length()+1, (LPSTR) &wdir[0],
    sizeof(wdir)/sizeof(wdir[0]), 0, (LPBOOL) &def );

QMessageBox::message( "Debug", "nOut=" + QString::number( nOut ) );
  
  int result = GetDiskFreeSpaceEx( wdir, (PULARGE_INTEGER) &free, 
                                         (PULARGE_INTEGER) &total, 0 );

  QMessageBox::message( "Debug", "result=" + QString::number( result ) );
  QMessageBox::message( "Debug", "free=" + QString::number( free ) );
  mle->append( tr( "<p>Total available diskspace on " +
     QDir::convertSeparators( USglobal->config_list.data_dir ) + ": </p>\n" +
     s.sprintf( "<p>%.1f MBytes</p>\n",  (float) free / 1e6 ) ) );

	if ( (unsigned long int) diskspace > free )
    mle->append(tr( "<p>ATTENTION: You will need to free up some "
     "diskspace before extracting this archive!</p>\n"));
 */
#endif

	s.sprintf("<p>Compression Ratio: %3.2f : 1</p></qt>\n",
	    (float) diskspace / f.size() );

	mle->append( s );
	clean_temp_dir();
}

/******************************************************************************/
void US_Archive::create_archive( QStringList* list )
{
	if ( list ) list->clear();
	mle->clear();

	QString filen = filename.stripWhiteSpace();
	QString projn = projectName.stripWhiteSpace();

	if ( filen.isEmpty() && projn.isEmpty() )
	{
		QMessageBox::message(
			tr( "Please note:" ),
			tr( "You will need to select some Data before you\n"
			    "can create an Archive\n\n"
			    "Click on \"Select Velocity Data for Archive\"\n"
			    "or click on \"Select Equilibrium Data for Archive\"\n"
			    "before proceeding..." ) );
		return;
	}

	disable_buttons();

	QString tempDirString = USglobal->config_list.root_dir + "/temp";
	QDir temp_dir( tempDirString );

	if ( temp_dir.exists() )
	{
		clean_temp_dir();
	}
	else if ( ! temp_dir.mkdir( tempDirString ) )
	{
		QMessageBox::message(
		  tr( "Ultrascan Warning:"),
			tr( "Cannot create a temporary directory:\n\n" + 
				  QDir::convertSeparators( tempDirString ) ) );

		enable_buttons();
		return;
	}

	if ( ! ultrascan_flag && ( ( run_type == 2 ) || ( run_type == 3 ) ) )
	{
	  QMessageBox::message(
			tr( "UltraScan Error:") ,
			tr( "The UltraScan Data must be included for "
				  "selecting a project archive." ) );

		enable_buttons();
		return;
  }

	char cwd[ 256 ];
	if ( getcwd( cwd, sizeof cwd ) == 0 )
	{
		QMessageBox::message(
		   tr( "UltraScan Error:" ),
		   tr( "Could not get current working directory" ) );

		enable_buttons();
		return;
	}

	QString     dirString = USglobal->config_list.result_dir;;
	QDir        dir;
	QString     tarfile;
	QStringList entries;
	QStringList files_processed;

	// Create result data file
	if ( ultrascan_flag )		
	{
		switch ( run_type )
		{
			case 0:  // Velocity or equilibrium data
			case 1:
			{
				mle->append( tr("<p>Creating Archive of UltraScan Data for Run " )
				                 + filename + ":</p>\n" );

				tarfile = filename + ".ultrascan-data.tar";
				
				// Get the list of files
				QDir datadir;
				datadir.setPath( dirString );
				datadir.setNameFilter( filename + "*" );
				entries = datadir.entryList();
				break;
			}

			case 2:  // Equilibrium Fitting Project Data
			{
				mle->append(
				   tr( "<p>Creating Archive of Equilibrium Fitting Project Data:<br>\n")
				      + projectName + ":</p>\n" );

				tarfile = projectName + ".ultrascan-data.tar";

				// Get the list of files
				QDir dat_dir;
				dat_dir.setPath( dirString );
				QString filter = projectName + "*.eq-project; " +
				                 projectName + "*.dat; "        +
				                 projectName + "*.eq_fit; "     +
				                 projectName + "*.res; "        +
				                 projectName + "*.dis; "        +
				                 projectName + "*.lncr2; "      +
				                 projectName + "*.mw*";

				dat_dir.setNameFilter( filter );
				entries = dat_dir.entryList();
				break;
			}

			case 3:  // Monte Carlo Project Data
			{
				mle->append(
				   tr( "<p>Creating Archive of Monte Carlo Project Data:<br>\n " )
				      +  projectName + ":</p>\n" );
				
				tarfile = projectName + ".ultrascan-data.tar";
				entries << projectName + ".Monte-Carlo" << projectName + ".mc";
			}
		}  // End switch ( run_type )

		mle->append( "<p>" + tarfile + "</p>\n " );

		if ( ! create_tar( dirString, tarfile, entries ) ) 
			return;
	}

	entries.clear();

	// Create report tar file
	if ( reports_flag )  
	{
		switch ( run_type )
		{
			case 0:
			case 1:
				mle->append(
				  tr( "<p>Creating Archive of HTML Reports for Run " ) +
					filename + ":</p>\n" );

				tarfile = filename + ".report-files.tar";
				entries << filename;
				break;

			case 2:
				mle->append(
				  tr( "<p>Creating Archive of HTML Reports for Equilibrium "
				       "Fitting Project: " ) +	projectName + ":</p>\n" );

				tarfile = projectName + ".report-files.tar";
				entries << projectName;
				break;

			case 3:
				mle->append(
				  tr( "<p>Creating Archive of HTML Reports for Monte Carlo "
				       "Project Data: " ) + projectName + ":</p>\n" );

				tarfile = projectName + ".report-files.tar";
				entries << projectName + ".mc";
		}

		mle->append( "<p>" + tarfile + "</p>\n " );
		
		if ( ! create_tar( USglobal->config_list.html_dir, tarfile, entries ) ) 
			return;
	}

	entries.clear();
	QDir work;

	// Create raw data tar file
	
	QString dirname = filename;

	if ( ( run_type == 0 ) || ( run_type == 1 ) ) 
	{
		QString path = USglobal->config_list.data_dir + "/" + filename;

		mle->append(
		  tr("<p>Creating Archive of Raw Experimental Datafiles for Run " ) +
			    filename + ":</p>\n");

		work.setPath( path );

		if ( ! work.exists() )
		{
			QMessageBox::message(
				tr( "UltraScan Warning:" ),
				tr( "The required raw data directory does not exist:\n" + 
					  QDir::convertSeparators( path ) + "\n"
				    "Please select the raw data directory.") );

			QString new_dir = QFileDialog::getExistingDirectory(
			  USglobal->config_list.data_dir, this,
			  "get existing directory", "Choose a directory", true );

			if ( ! new_dir.isEmpty() )
			{
				// Assumes last char returned is a /
				// This gets the last component of the directory path
				work.setPath( new_dir );
				int position = new_dir.findRev( "/", -2 );
				new_dir  = new_dir.mid( position + 1, new_dir.length() - 1 );

				if ( QString::compare( dirname, new_dir ) != 0 )
				{
					QMessageBox::message(
					  tr( "UltraScan Warning:" ),
					  tr( "New directory name is different than recorded name." ) );

					dirname = new_dir;
				}
			}
			else
			{
				QMessageBox::message(
				  tr( "UltraScan Error:" ),
				  tr( "No raw data be selected, progam will be terminated." ) );

				exit(0);
			}
		}

		entries << dirname;
		tarfile = filename + ".raw-data.tar";

		mle->append( "<p>" + tarfile + "</p>\n " );

		if ( ! create_tar( USglobal->config_list.data_dir, tarfile, entries ) ) 
			return;
	}

	// Move tar files from different dirs to temp dir and tar them together
	
	QString tempdir   = USglobal->config_list.root_dir   + "/temp/";
	QString resultdir = USglobal->config_list.result_dir + "/";
	QString reportdir = USglobal->config_list.html_dir   + "/" ;
	QString datadir   = USglobal->config_list.data_dir   + "/";
	QString src;
	QString dest;
	QDir    temp;

	if ( ( run_type == 0 ) || ( run_type == 1 ) )
	{
		src  = resultdir + filename + ".ultrascan-data.tar";
		dest = tempdir   + filename + ".ultrascan-data.tar";
		move_file( src, dest );

		src  = reportdir + filename + ".report-files.tar";
		dest = tempdir   + filename + ".report-files.tar";
		move_file( src, dest );

		src  = datadir + filename + ".raw-data.tar";
		dest = tempdir + filename + ".raw-data.tar";
		move_file( src, dest );

		tarfile = filename + ".tar";
	}
	else // ( run_type == 2 ) || ( run_type == 3 )
	{
		src  = resultdir + projectName + ".ultrascan-data.tar";
		dest = tempdir   + projectName + ".ultrascan-data.tar";
		move_file( src, dest );

		src  = reportdir + projectName + ".report-files.tar";
		dest = tempdir   + projectName + ".report-files.tar";
		move_file( src, dest );

		if ( run_type == 2 ) 
			tarfile = projectName + ".project.tar";
		else // run_type == 3
			tarfile = projectName + ".mc.project.tar";
	}	

	chdir( tempdir.ascii() );
	temp.setPath( tempdir );
	temp.setNameFilter( filename + "*.tar" );
	entries = temp.entryList();

	qApp->processEvents();  

	US_Tar tar;

	int ret = tar.create( tarfile, entries );
	if ( ret != TAR_OK )
	{
		QMessageBox::message(
			 tr( "UltraScan tar Error:" ),
			 tr( tar.explain( ret ) ) );

		enable_buttons();
		return;
	}

	qApp->processEvents();  
	
	// gzip the tarfile of tarfiles

	US_Gzip gzip;
	
	if ( ( ret = gzip.gzip( tarfile ) ) != GZIP_OK )
	{
		QMessageBox::message(
		   tr( "UltraScan gzip Error:" ),
		   tr( gzip.explain( ret ) ) );

		enable_buttons();
		return;
	}
	
	// Move the gzipped file to the archive directory
	
	QString gzfile     = tarfile + ".gz";
	QString archivedir = USglobal->config_list.archive_dir +"/";
	
	src                = tempdir    + gzfile;
	dest               = archivedir + gzfile;
	
	QFile archive;
	archive.setName( dest );
	
	while ( archive.exists() )
	{
		switch( QMessageBox::information( this,
		  tr( "Ultrascan Warning:" ) ,
		  tr( "Archive File '" + gzfile + "'\n"
			    "already exists, overwrite?" ),
			    "&Yes", "&Cancel", "Rename" ) )
		{
			case 0:
				goto move;

			case 1:
				enable_buttons();
				return;

			case 2:
			{
				dest =
				  QFileDialog::getSaveFileName( archivedir, "*.tar.gz", 0 );

				archive.setName( dest );
				break;
			}
		}
	}

	move:
	
	move_file( src, dest );

	// Delete files if requested

	if ( delete_flag )
	{
		QString name_filter;

		mle->append( tr( "<p>Deleting Source Files...</p>\n" ) );
		
		// Delete result files
		
		switch ( run_type )
		{
			case 0: // Velocity or equilibrium data
			case 1:
				name_filter = filename + "*";
				break;

			case 2: // Equilibrium fitting project
				name_filter = projectName + "*.eq-project; " +
				              projectName + "*.dat; "        +
				              projectName + "*.eq_fit; "     +
				              projectName + "*.res; "        +
				              projectName + "*.dis; "        +
				              projectName + "*.lncr2; "      +
				              projectName + "*.mw*";
				break;

			case 3:
				name_filter = projectName + ".Monte-Carlo; " + projectName + ".mc";
				break;
		}

		work.setPath( resultdir );
		work.setNameFilter( name_filter );
		entries = work.entryList();

		QStringList::Iterator it;
		for ( it = entries.begin(); it != entries.end(); ++it )
		{
			work.remove( *it );
		}

		QString report;

		// Delete report file
		switch ( run_type )
		{
			case 0: // Velocity or equilibrium data
			case 1:
			  report = reportdir + filename;
				break;

			case 2: // Equilibrium fitting project
				report = reportdir + projectName;
				break;

			case 3: // Monte Carlo project
				report = reportdir + projectName + ".mc";
				break;
		}

		work.setPath( report );
		entries = work.entryList( "*" );

		for ( it = entries.begin(); it != entries.end(); ++it )
		{
			if ( *it == "."  ||  *it == ".." ) continue;
			work.remove( *it );
		}

		work.cdUp();

		switch ( run_type )
		{
			case 0: // Velocity or equilibrium data
			case 1:
			  work.rmdir( filename );
				break;

			case 2: // Equilibrium fitting or Monte Carlo project
			case 3: 
				work.rmdir( projectName );
				break;
		}

		// Delete data file


		switch ( run_type )
		{
			case 0: // Velocity or equilibrium data
			case 1:
				{
			  work.rmdir( filename );
				break;
				}

			case 2: // Equilibrium fitting or Monte Carlo project
			case 3: 
				break;
		}

		if ( (run_type == 0) || (run_type == 1) ) // Velocity or equilibrium data
		{
			QString dirname  = filename;
			work.setPath( datadir + dirname );
			entries = work.entryList( "*" );

			for ( it = entries.begin(); it != entries.end(); ++it )
			{
				if ( *it == "."  ||  *it == ".." ) continue;
				work.remove( *it );
			}

			work.cdUp();
			work.rmdir( dirname );
		}
	}

	mle->append( tr( "<p>Archive Creation finished:</p>\n" ) );
	mle->append( "<p>" + gzfile + "</p>\n " );

	enable_buttons();
	pb_create->setEnabled( false );
	chdir( cwd );
}

void US_Archive::move_file( const QString& src, const QString& dest )
{
	QFile f( src );
	if ( ! f.exists() ) return;

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

bool US_Archive::create_tar( const QString& dirString, 
		                         const QString& tarfile, 
														 QStringList&   entries )
{
	US_Tar      tar;
	QStringList files_processed;	

	chdir( dirString.ascii() );
	qApp->processEvents();  

	int ret = tar.create( tarfile, entries, &files_processed );
	if ( ret != TAR_OK )
	{
		QMessageBox::message(
			 tr( "UltraScan tar Error:" ),
			 tr( tar.explain( ret ) ) );
			
		enable_buttons();
		return false;
	}

	QString list = "<pre>\n" + files_processed.join( "\n" ) + "</pre>\n";
	mle->append( list );

	return true;
}

/******************************************************************************/
void US_Archive::extract_archive( void )
{
	QString fn = QFileDialog::getOpenFileName(
	        USglobal->config_list.archive_dir, "*.tar.gz", 0 );

	if ( fn.isEmpty() ) return;

	
	fn.replace( "\\", "/" ); // Convert for WIN32 backslashes
	int start_pos = fn.findRev( "/" ) + 1;
	QString tarGzFile = fn.mid( start_pos );

	clean_temp_dir();
	mle->clear();

	mle->append( tr( "<qt><p>Extracting Archive: \n") +
	  QDir::convertSeparators( fn ) + "</p>\n" );

	disable_buttons();

	// Copy tar.gz fie to temp dir for uncompress

	QString tempdir = USglobal->config_list.root_dir + "/temp/";
	int cr = copy( fn, tempdir + tarGzFile );

	if ( cr != 0 )
	{
		QMessageBox::information( this,
		  tr( "UltraScan Error:" ),
		  tr( "Unable to copy archive to temp dir for extraction." ) );

		enable_buttons();
		return;
	}

  // gunzip the file
	// Save the current directory and change to the temp directory
	
	char cwd[ 256 ];
	if ( getcwd( cwd, sizeof cwd ) == 0 )
	{
		QMessageBox::message(
		   tr( "UltraScan Error:" ),
		   tr( "Could not get current working directory" ) );

		enable_buttons();
		return;
	}

	int err = chdir( tempdir.ascii() );
	if ( err != 0 )
	{
	  QString e = QString( "%1" ).arg( err );
		QMessageBox::message(
		   tr( "UltraScan Error:" ),
		   tr( "Could not change to working directory:\n" + 
				   QDir::convertSeparators( tempdir ) + 
				   "\nError number = " + e ) );

		enable_buttons();
		return;
	}

	qApp->processEvents();  
	
	// gunzip the file
	US_Gzip gzip;

	int ret = gzip.gunzip( tarGzFile );
	if ( ret != GZIP_OK )
	{
		QMessageBox::message(
		   tr( "UltraScan gzip Error:" ),
		   tr( gzip.explain( ret ) ) );

		enable_buttons();
		return;
	}

	qApp->processEvents();  

	// untar the file
	// Assume extract_filename is something.tar.gz

	US_Tar      tar;
	QString     tarfile = tarGzFile.left( tarGzFile.length() - 3 );

	qApp->processEvents();  
	
	// Extract the files
	ret = tar.extract( tarfile );
	if ( ret != TAR_OK )
	{
		QMessageBox::message(
		   tr( "UltraScan tar Error:" ),
		   tr( tar.explain( ret ) ) );

		enable_buttons();
		return;
	}

	unlink ( tarfile.ascii() );

	// Move tar file to according dir and then uncompress result-data files

	QDir        work_dir;
	QStringList entries;

	work_dir.setPath( tempdir );
	work_dir.setNameFilter( "*.tar" );
	entries = work_dir.entryList();

	QStringList::Iterator it;
	for ( it = entries.begin(); it != entries.end(); ++it )
	{
		qApp->processEvents();  
		chdir( tempdir.ascii() );
		tarfile = *it;

		// Results
		if ( tarfile.contains( ".ultrascan-data.tar", false ) > 0 )
		{
			QString result_dir = USglobal->config_list.result_dir + "/";
			extract( tarfile, "Result", result_dir, true );
		}

		// Reports
		else if ( tarfile.contains( ".report-files.tar", false ) > 0 )
		{
			QString report_dir = USglobal->config_list.html_dir + "/";
			extract( tarfile, "Report", report_dir, true );
		}

		// Raw data
		else if ( tarfile.contains( ".raw-data.tar", false ) > 0 )
		{
			QString data_dir = USglobal->config_list.data_dir + "/";
			extract( tarfile, "Raw Data", data_dir, true );
		}
	}

	enable_buttons();
	chdir( cwd ); // Restore original working directory
}

void US_Archive::extract( const QString& tarfile, 
                          const QString& label,
                          const QString& dir,
                          const bool     verbose )
{
	QString tempdir = USglobal->config_list.root_dir + "/temp/";
	copy( tempdir + tarfile, dir + tarfile );
	chdir ( dir.ascii() );

	QStringList files;
			
	// Extract the files
	mle->append( tr( "<p>Restoring " + label + " Sub-Archive<br>\n") +
			    QDir::convertSeparators( dir + tarfile ) + ":</p>\n" );

	US_Tar tar;
	int    ret;

	qApp->processEvents();  

	if ( verbose )
		ret = tar.extract( tarfile, &files );
	else 
		ret =  tar.extract( tarfile );
	
	if ( ret != TAR_OK )
	{
		QMessageBox::message(
			tr( "UltraScan tar Error:" ),
			tr( tar.explain( ret ) ) );

		return;
	}

	if ( verbose )
	{
		QString pre_string = "<pre>" + files.join( "\n" ) + "</pre>\n";
		mle->append( pre_string ); 
	}

	unlink ( tarfile.ascii() );
}

