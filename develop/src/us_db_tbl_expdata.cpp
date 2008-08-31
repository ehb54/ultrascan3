#include "../include/us_db_tbl_expdata.h"
#include "../include/us_tar.h"
#include "../include/us_gzip.h"

#ifdef WIN32
	#define chdir _chdir
	#include <direct.h>
#endif

//! Constructor
/*! 
	Constractor a new <var>US_ExpData_DB</var> interface, 
	with <var>parent</var> as a parent and <var>us_tblexpdata</var> as object name.
*/ 
US_ExpData_DB::US_ExpData_DB( QWidget* parent, const char* name) 
  : US_DB_Widgets( parent, name )
{
	int border  =   4;
	int spacing =   2;
	int buttonw = 150;
	int buttonh =  26;
	int labelw  = 150;
	int xpos    = border;
	int ypos    = border;

	cell_flag  = false;		// use for cell table connect 
	query_flag = false;		// use for query listbox select
	from_query = false;		// use for Cell ID display DB data
	all_done   = false;		// use for closeEvent to check the DB save, 
	                      //but only when called from us_expdata_db.

	item_ExpdataID   = NULL;
  item_Description = NULL;

	cell_table_unfinished = 0;
	Steps                 = 0;

	for ( int i = 0; i < 8; i++ )
	{
		exp_info.Cell  [ i ] = "";
		exp_info.CellID[ i ] = 0;
	}
	
	pb_run = pushbutton( "Load Run" );
	pb_run->setGeometry( xpos, ypos, buttonw, buttonh );
	connect( pb_run, SIGNAL( clicked() ), SLOT( load_run() ) );

	xpos += buttonw + spacing;

	lbl_run =  textlabel( " not selected", -1, QFont::Bold );
	lbl_run->setAlignment( AlignLeft | AlignVCenter );
	lbl_run->setGeometry( xpos, ypos, labelw, buttonh );

	xpos += labelw + spacing;
	
	pb_help = pushbutton( "Help" );
	pb_help->setGeometry(xpos, ypos, buttonw, buttonh);
	connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

	xpos += buttonw + spacing;

	pb_close = pushbutton( "Close" );
	pb_close->setGeometry( xpos, ypos, buttonw, buttonh );
	connect(pb_close, SIGNAL(clicked()), SLOT(quit()));

	xpos = border;
	ypos +=buttonh + spacing;

	pb_investigator = pushbutton( "Select Investigator" );
	pb_investigator->setGeometry( xpos, ypos, buttonw, buttonh );
	connect( pb_investigator, SIGNAL( clicked() ), SLOT( sel_investigator() ) );

	xpos += buttonw + spacing;

	lbl_investigator = textlabel( " not selected", -1, QFont::Bold );
	lbl_investigator->setAlignment( AlignLeft | AlignVCenter );	
	lbl_investigator->setGeometry( xpos, ypos, labelw, buttonh );
	
	xpos += labelw + spacing;
	
	pb_add = pushbutton( "Add DB Entry" );
	pb_add->setGeometry( xpos, ypos, buttonw, buttonh );
	connect( pb_add, SIGNAL( clicked() ), SLOT( add_db() ) );

	xpos += buttonw + spacing;

	pb_delete = pushbutton( "Delete DB Entry" );
	pb_delete->setGeometry( xpos, ypos, buttonw, buttonh );
	connect( pb_delete, SIGNAL( clicked() ), SLOT( check_permission() ) );
	
	xpos  = border;
	ypos += buttonh + spacing;

	pb_date = pushbutton( "Date (from Calendar)" );
	pb_date->setGeometry( xpos, ypos, buttonw, buttonh );
	connect( pb_date, SIGNAL( clicked() ), SLOT( sel_date() ) );
	
	xpos += buttonw + spacing;

	lbl_date = textlabel( "", -1,  QFont::Bold );
	lbl_date->setAlignment( AlignLeft | AlignVCenter );	
	lbl_date->setGeometry( xpos, ypos, labelw, buttonh );

	QDate date = QDate::currentDate();
  current_date.sprintf( " %d/%d/%d", date.month(), date.day(), date.year() );
	lbl_date->setText( current_date );
	exp_info.Date = current_date;
	
	xpos += labelw + spacing;
	
	pb_reset = pushbutton( "Reset" );
	pb_reset->setGeometry( xpos, ypos, buttonw, buttonh );
	connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );

	xpos += buttonw + spacing;

	pb_query = pushbutton( "Query DB Entry" );
	pb_query->setGeometry( xpos, ypos, buttonw, buttonh );
	connect( pb_query, SIGNAL( clicked() ), SLOT( query_db() ) );

	xpos  = border;
	ypos += buttonh + spacing;

	lbl_runName = label( " Run Name:", -1, QFont::Bold );
	lbl_runName->setAlignment( AlignLeft | AlignVCenter );
	lbl_runName->setGeometry( xpos, ypos, buttonw, buttonh );

	xpos += buttonw + spacing;

	lbl_runid= textlabel( "", -1, QFont::Bold );
	lbl_runid->setAlignment( AlignLeft | AlignVCenter );
	lbl_runid->setGeometry( xpos, ypos, labelw, buttonh );
	
	xpos += labelw + spacing;
	
	lb_query = listbox( "Query" );
	lb_query->setGeometry( xpos, ypos, 
	                       buttonw * 2 + spacing, buttonh * 3 + spacing * 2 );
	connect( lb_query, SIGNAL( selected( int ) ), SLOT( sel_query( int ) ) );
	
	xpos  = border;
	ypos += buttonh + spacing;
	
	lbl_optical = label( " Optical System:", -1, QFont::Bold );
	lbl_optical->setAlignment( AlignLeft | AlignVCenter );
	lbl_optical->setGeometry( xpos, ypos, buttonw, buttonh );

	xpos += buttonw + spacing;
	
	lbl_opt = textlabel( "", -1, QFont::Bold );
	lbl_opt->setAlignment( AlignLeft | AlignVCenter );
	lbl_opt->setGeometry( xpos, ypos, labelw, buttonh );
	
	xpos  = border;
	ypos += buttonh + spacing;
	
	lbl_experiment = label( " Experiment Type:", -1, QFont::Bold );
	lbl_experiment->setAlignment( AlignLeft | AlignVCenter );
	lbl_experiment->setGeometry( xpos, ypos, buttonw, buttonh );

	xpos += buttonw + spacing;
	
	lbl_exp = textlabel( "", -1, QFont::Bold );
	lbl_exp->setAlignment( AlignLeft | AlignVCenter );
	lbl_exp->setGeometry( xpos, ypos, labelw, buttonh );
	
	xpos  = border;
	ypos += buttonh + spacing;
	
	lbl_duration = label( " Duration:", -1, QFont::Bold );
	lbl_duration->setAlignment( AlignLeft | AlignVCenter );
	lbl_duration->setGeometry( xpos, ypos, buttonw, buttonh );
	
	xpos += buttonw + spacing;
	
	lbl_dur = textlabel( "", -1, QFont::Bold );
	lbl_dur->setAlignment( AlignLeft | AlignVCenter );
	lbl_dur->setGeometry( xpos, ypos, labelw, buttonh );
	
	xpos += labelw + spacing;
	
	lbl_description = label( " Description:", -1, QFont::Bold );
	lbl_description->setAlignment( AlignLeft | AlignVCenter );
	lbl_description->setGeometry( xpos, ypos, buttonw, buttonh );

	xpos += buttonw + spacing;
	
	le_description = lineedit( "description" );
	le_description->setGeometry( xpos, ypos, buttonw, buttonh );
	connect ( le_description, SIGNAL( textChanged       ( const QString& ) ), 
	                          SLOT  ( update_description( const QString& ) ) );

	xpos  = border;
	ypos += buttonh + spacing;
	
	lbl_rotor = label( " Rotor:", -1, QFont::Bold );
	lbl_rotor->setAlignment( AlignLeft | AlignVCenter );
	lbl_rotor->setGeometry( xpos, ypos, buttonw, buttonh );
	
	xpos += buttonw + spacing;

	lbl_rt= textlabel( "", -1, QFont::Bold );
	lbl_rt->setAlignment( AlignLeft | AlignVCenter );
	lbl_rt->setGeometry( xpos, ypos, labelw, buttonh ) ;
	
	xpos += labelw + spacing;
	
	lbl_temperature = label( " Temperature:", -1, QFont::Bold );
	lbl_temperature->setAlignment( AlignLeft | AlignVCenter );
	lbl_temperature->setGeometry( xpos, ypos, buttonw, buttonh );

	xpos += buttonw + spacing;
	
	lbl_temp= textlabel( "", -1, QFont::Bold );
	lbl_temp->setAlignment( AlignLeft | AlignVCenter );
	lbl_temp->setGeometry( xpos, ypos, buttonw, buttonh );
	
	xpos  = border;
	ypos += buttonh + spacing;
	
	lbl_cell = label( " Cell ID:", -1, QFont::Bold );
	lbl_cell->setAlignment( AlignLeft | AlignVCenter );
	lbl_cell->setGeometry( xpos, ypos, buttonw, buttonh );

	xpos += buttonw + spacing;
	
	cmbb_cell = combobox( "Cell ID Select" );
	cmbb_cell->setGeometry( xpos, ypos, 
	                        labelw + buttonw * 2 + spacing * 2, buttonh );
	cmbb_cell->insertItem( "No Data" );
	connect( cmbb_cell, SIGNAL( activated( int ) ), SLOT( sel_cell( int ) ) );

	ypos += buttonh + border;
	xpos = 2 * border + 3 * buttonw + labelw + 3 * spacing;

	global_Xpos += 30;
	global_Ypos += 30;

	setMinimumSize( xpos, ypos );	
	setGeometry( global_Xpos, global_Ypos, xpos, ypos );	
	setup_GUI();
}

//! Destructor
/*! destroy the <var>US_ExpData_DB</var>. */
US_ExpData_DB::~US_ExpData_DB()
{
	delete editdb;
	delete investigator_dlg;
	delete calendar_dlg;
	delete cell_dlg;
}

void US_ExpData_DB::setup_GUI()
{
	QGridLayout* Grid = new QGridLayout( this, 6, 4, 4, 2 );

	Grid->setColSpacing( 1, 150 );

	Grid->addWidget( pb_run,           0, 0 );
	Grid->addWidget( lbl_run,          0, 1 );
	Grid->addWidget( pb_help,          0, 2 );
	Grid->addWidget( pb_close,         0, 3 );
	Grid->addWidget( pb_investigator,  1, 0 );
	Grid->addWidget( lbl_investigator, 1, 1 );
	Grid->addWidget( pb_add,           1, 2 );
	Grid->addWidget( pb_delete,        1, 3 );
	Grid->addWidget( pb_date,          2, 0 );
	Grid->addWidget( lbl_date,         2, 1 );
	Grid->addWidget( pb_reset,         2, 2 );
	Grid->addWidget( pb_query,         2, 3 );
	Grid->addWidget( lbl_runName,      3, 0 );
	Grid->addWidget( lbl_runid,        3, 1 );
	Grid->addWidget( lbl_optical,      4, 0 );
	Grid->addWidget( lbl_opt,          4, 1 );
	Grid->addWidget( lbl_experiment,   5, 0 );
	Grid->addWidget( lbl_exp,          5, 1 );

	Grid->addMultiCellWidget( lb_query, 3, 5, 2, 3 );

	Grid->addWidget( lbl_duration,     6, 0 );
	Grid->addWidget( lbl_dur,          6, 1 );
	Grid->addWidget( lbl_description,  6, 2 );
	Grid->addWidget( le_description,   6, 3 );
	Grid->addWidget( lbl_rotor,        7, 0 );
	Grid->addWidget( lbl_rt,           7, 1 );
	Grid->addWidget( lbl_temperature,  7, 2 );
	Grid->addWidget( lbl_temp,         7, 3 );
	Grid->addWidget( lbl_cell,         8, 0 );

	Grid->addMultiCellWidget( cmbb_cell, 8, 8, 1, 3 );

	Grid->activate();
}

/*!  Open US_Edit_DB interface to load experimental data.  */
void US_ExpData_DB::load_run()
{
	reset();

	editdb = new US_Edit_DB();
	editdb->setCaption( "Sedimentation Data Entry" );

	connect( editdb, SIGNAL( variablesUpdated() ), 
	         this,   SLOT  ( update_variables() ) );

	connect( editdb, SIGNAL( channel_selected  ( const QString& ) ), 
	         this,   SLOT  ( update_flo_channel( const QString& ) ) );

	editdb->show();
}

/*!  Update selected fluorescence channel variable flo_channel */

void US_ExpData_DB::update_flo_channel(const QString& channel )
{
	flo_channel = channel;
}

/*!  Update variables' value from load_run().  */
void US_ExpData_DB::update_variables( void )
{
	QString   q = "INSERT INTO tblExpData (Path) VALUES ('N/A');";
	QSqlQuery target;	

	if ( ! target.exec( q ) )
	{
		QSqlError sqlerr = target.lastError();
		QMessageBox::message(
		  tr( "Attention:" ), 
		  tr( "Reserving DB table 'tblExpData' failed.\n"
		      "Attempted to execute this command:\n\n" )
		      + q + "\n\n" 
		      + tr( "Causing the following error:\n\n" )
		      + sqlerr.text() );

		return;
	}

	QSqlQuery query( "SELECT ExpDataID FROM tblExpData" );
	
	if ( query.isActive() )
	{ 

		query.last();     // Moves to the last record.
		exp_info.ExpdataID = query.value( 0 ).toInt();
	}
	else
	{
		exp_info.ExpdataID = 1;
	}

	exp_info.Path = editdb->run_inf.data_dir;
	lbl_run->setText( exp_info.Path );

	exp_info.Runid = editdb->run_inf.run_id;
	lbl_runid->setText( exp_info.Runid );

	exp_info.Temperature = editdb->run_inf.avg_temperature;
	lbl_temp->setText( QString::number( exp_info.Temperature ) );

	exp_info.Duration = editdb->run_inf.duration;
	lbl_dur->setText( QString::number( exp_info.Duration ) );

	for ( int i = 0; i < 8; i++ )
	{
		for (int j = 0; j < 3; j++ )
		{
			exp_info.wavelength[ i ][ j ] = editdb->run_inf.wavelength[ i ][ j ];
			exp_info.scans     [ i ][ j ] = editdb->run_inf.scans     [ i ][ j ];
		}

		exp_info.centerpiece[ i ] = editdb->run_inf.centerpiece[ i ];
	}

	exp_info.Edit_type = editdb->edit_type;
	update_type( exp_info.Edit_type );

	exp_info.Rotor = editdb->run_inf.rotor;
	update_rotor( exp_info.Rotor );
	
	for ( int i = 0; i < 8; i++ )
	{
		exp_info.Cell[ i ] = editdb->run_inf.cell_id[ i ];
	}

	update_cell();
}

/*!  Update cell tables info and show them in combobox list.  */
void US_ExpData_DB::update_cell( void )
{
	cmbb_cell->clear();
	
	QString s;
	
	for ( int i = 0; i < 8; i++ ) 
	{  
		if ( exp_info.Cell[ i ].isEmpty() )
		{
			s.sprintf( "Cell %d : No Data", i + 1 );
			cmbb_cell->insertItem( s );
		}
		else
		{	
			if ( from_query )
			{
				s.sprintf( "Cell %d : " + exp_info.Cell[ i ] + " (Cell Table ID: %d )",
				           i + 1, exp_info.CellID[ i ] );
				cmbb_cell->insertItem( s );
			}
			else
			{
				s.sprintf( "Cell %d : " + exp_info.Cell[ i ] + 
				           ", Cell Table ID not selected", i + 1 );
				cmbb_cell->insertItem( s );

				// Total cell tables needed for this experiment data table$
				cell_table_unfinished++;
			}

			cell_flag = true;  // Now can connect to cell table
		}	
	}
}

/*!  Open US_cell_DB interface, show the selected cell information.  */
void US_ExpData_DB::sel_cell( int i )
{
	QString new_str;
	
	if ( cell_flag && ! exp_info.Cell[ i ].isEmpty() )
	{
	 	if ( exp_info.Invid <= 0 )
		{
			QMessageBox::message(
				tr( "Attention:" ),
				tr( "Please 'Select Investigator' first!" ) );

			return;	
		}

		if ( exp_info.centerpiece[ i ] < 0 )
		{
			QMessageBox::message(
				tr( "Attention:" ), 
				tr( "Please select a centerpiece during 'Load Run'" ) );

			return;
		}

		exp_info.cell_position = i;
		cell_dlg = new US_Cell_DB( exp_info, &cell_table_unfinished );
		cell_dlg->setCaption( "Cell Information" );

		if ( from_query )  // not allow change data if you load them from DB
		{
			QSqlCursor cur( "tblCell" );
			QString chk_cellid;
			chk_cellid.sprintf( "CellID = %d", exp_info.CellID[ i ] );
			cur.select( chk_cellid );

			// Check Cell Table and find CellID match the ExpData table's
			// CellID[i] and retrieve Cell table
			if ( cur.next() )
			{
				cell_dlg->cell_info.CellID        = cur.value( "CellID" ).toInt();
				cell_dlg->cell_info.ExperimentID  = cur.value( "ExperimentID" ).toInt();
				cell_dlg->cell_info.CenterpieceID = cur.value( "CenterpieceID" ).toInt();
				cell_dlg->cell_info.Position      = cur.value( "Position" ).toInt();
				cell_dlg->cell_info.InvID         = cur.value( "InvestigatorID" ).toInt();
				cell_dlg->cell_info.Description   = cur.value( "Description" ).toString();
				cell_dlg->cell_info.Num_Channel   = cur.value( "Num_Channel" ).toInt();

				QString s;

				for ( int i = 0; i < 3; i++ )
				{
					s.sprintf( "Wavelength_%d", i + 1 );
					cell_dlg->cell_info.Wavelength[ i ] = cur.value( s ).toInt();

					s.sprintf("Wl_Scans_%d", i + 1 );
					cell_dlg->cell_info.Scans[ i ] = cur.value( s ).toInt();	
				}

				for ( int i = 0; i < 4; i++ )
				{
					s.sprintf( "BufferID_%d", i + 1 );
					cell_dlg->cell_info.BufferID[ i ] = cur.value( s ).toInt();

					for ( int j = 0; j < 3; j++ )
					{
						if ( j == 0 )
						{
							s.sprintf( "DNAID_%d", i + 1 );
							cell_dlg->cell_info.DNAID[ i ][ j ] = cur.value( s ).toInt();

							s.sprintf( "PeptideID_%d", i + 1 );
							cell_dlg->cell_info.PepID[ i ][ j ] = cur.value( s ).toInt();
						}
						else
						{
							s.sprintf( "DNA%dID_%d", j + 1, i + 1 );
							cell_dlg->cell_info.DNAID[ i ][ j ] = cur.value( s ).toInt();

							s.sprintf( "Peptide%dID_%d", j + 1, i + 1 );
							cell_dlg->cell_info.PepID[ i ][ j ] = cur.value( s ).toInt();
						}
					}
				}

				cell_dlg->from_db = true;

				cell_dlg->lbl_cellid  ->setNum( cell_dlg->cell_info.CellID        );
				cell_dlg->lbl_invest  ->setNum( cell_dlg->cell_info.InvID         );
				cell_dlg->lbl_ctpc    ->setNum( cell_dlg->cell_info.CenterpieceID );
				cell_dlg->lbl_position->setNum( cell_dlg->cell_info.Position + 1  );
				cell_dlg->lbl_exp     ->setNum( cell_dlg->cell_info.ExperimentID  );

				QString str2;
				str2.sprintf( "Cell Information Retrieve from Cell Table %d. ",
						cell_dlg->cell_info.CellID );

				cell_dlg->lb_query->insertItem( str2 );

				if( cell_dlg->cell_info.Wavelength[ 0 ] == 0 )
					cell_dlg->lbl_wl1->setText( "No Data available" );
				else
					cell_dlg->lbl_wl1->setNum(cell_dlg->cell_info.Wavelength[0]);

				if ( cell_dlg->cell_info.Wavelength[ 1 ] == 0 )
					cell_dlg->lbl_wl2->setText( "No Data available" );
				else
					cell_dlg->lbl_wl2->setNum( cell_dlg->cell_info.Wavelength[ 1 ] );

				if ( cell_dlg->cell_info.Wavelength[ 2 ] == 0 )
					cell_dlg->lbl_wl3->setText( "No Data available" );
				else
					cell_dlg->lbl_wl3->setNum( cell_dlg->cell_info.Wavelength[ 2 ] );

				if ( cell_dlg->cell_info.Scans[ 0 ] == 0 )
					cell_dlg->lbl_scan1->setText( "No Scans" );
				else
					cell_dlg->lbl_scan1->setNum( cell_dlg->cell_info.Scans[ 0 ] );

				if ( cell_dlg->cell_info.Scans[ 1 ] == 0 )
					cell_dlg->lbl_scan2->setText( "No Scans" );
				else
					cell_dlg->lbl_scan2->setNum( cell_dlg->cell_info.Scans[ 1 ] );

				if ( cell_dlg->cell_info.Scans[ 2 ] == 0 )
					cell_dlg->lbl_scan3->setText( "No Scans" );
				else
					cell_dlg->lbl_scan3->setNum( cell_dlg->cell_info.Scans[ 2 ] );

				cell_dlg->cmbb_channel->clear();

				QString channel_Str;
				
				for ( int i = 0; i < cell_dlg->cell_info.Num_Channel; i++ )
				{
					channel_Str = "";
					channel_Str.sprintf( "Channel %d", i + 1 );
					cell_dlg->cmbb_channel->insertItem( channel_Str );
				}

				int j = cell_dlg->cmbb_channel->currentItem();
			
				cell_dlg->le_buffer->setText( 
					show_buffer( cell_dlg->cell_info.BufferID[ j ] ) );
				
				if ( cell_dlg->cell_info.BufferID[ j ] == -1 )
					cell_dlg->le_buffer->setText( "not selected" );

				cell_dlg->le_peptide1->setText(
					show_peptide( cell_dlg->cell_info.PepID[ j ][ 0 ] ) );
				
				if ( cell_dlg->cell_info.PepID[ j ][ 0 ] == -1 )
					cell_dlg->le_peptide1->setText( "not selected" );

				cell_dlg->le_peptide2->setText(
					show_peptide( cell_dlg->cell_info.PepID[ j ][ 1 ] ) );
				
				if ( cell_dlg->cell_info.PepID[ j ][ 1 ] == -1 )
					cell_dlg->le_peptide2->setText( "not selected" );

				cell_dlg->le_peptide3->setText(
					show_peptide( cell_dlg->cell_info.PepID[ j ][ 2 ] ) );
				
				if ( cell_dlg->cell_info.PepID[ j ][ 2 ] == -1 )
					cell_dlg->le_peptide3->setText( "not selected" );

				cell_dlg->le_DNA1->setText(
					show_DNA(cell_dlg->cell_info.DNAID[j][0]));
				
				if ( cell_dlg->cell_info.DNAID[ j ][ 0 ] == -1 )
					cell_dlg->le_DNA1->setText( "not selected" );
				
				cell_dlg->le_DNA2->setText( show_DNA(
					cell_dlg->cell_info.DNAID[ j ][ 1 ] ) );
				
				if ( cell_dlg->cell_info.DNAID[ j ][ 1 ] == - 1)
					cell_dlg->le_DNA2->setText( "not selected" );

				cell_dlg->le_DNA3->setText(
					show_DNA( cell_dlg->cell_info.DNAID[ j ][ 2 ] ) );
				
				if ( cell_dlg->cell_info.DNAID[ j ][ 2 ] == -1 )
					cell_dlg->le_DNA3->setText( "not selected" );

				cell_dlg->pb_add     ->setEnabled(false);
				cell_dlg->pb_reset   ->setEnabled(false);
				cell_dlg->pb_query   ->setEnabled(false);
				cell_dlg->pb_buffer  ->setEnabled(false);
				cell_dlg->pb_peptide1->setEnabled(false);
				cell_dlg->pb_peptide2->setEnabled(false);
				cell_dlg->pb_peptide3->setEnabled(false);
				cell_dlg->pb_DNA1    ->setEnabled(false);
				cell_dlg->pb_DNA2    ->setEnabled(false);
				cell_dlg->pb_DNA3    ->setEnabled(false);
			}
			else // not find CellID and update Expdata table CellID to 0
			{
				exp_info.CellID[ i ] = 0;
				QSqlCursor cur( "tblExpData" );

				QString str0;
				str0.sprintf(" ExpdataID = %d", exp_info.ExpdataID);

				cur.select( str0 );

				if ( cur.next() )
				{
					QSqlRecord* buffer = cur.primeUpdate();
					QString str1;
					str1.sprintf( "Cell%dID", i + 1 );
					buffer->setValue( str1, exp_info.CellID[ i ] );
					cur.update();
				}

				QString str2;
				str2.sprintf( "Cell %d : " + exp_info.Cell[ i ] + 
					" , Cell Table Deleted", i + 1 );

				cmbb_cell->changeItem( str2, i );
				return;
			}
		}
		else  // First time load from 'Load Run' and not from DB
		{
			// Prevent twice open same cell table to get new table ID
			if ( exp_info.CellID[ i ] == 0 )
			{
				exp_info.CellID[ i ] = cell_dlg->get_newID( "tblCell", "CellID" );
			}

			for ( int j = 0; j < 8; j++ )  // Check whether same Cell Table ID exists
			{
				if ( ( j != i ) && ( exp_info.CellID[ j ] == exp_info.CellID[ i ] ) )
				{
					QString str_w;
					str_w.sprintf( 
						"Please complete the data entry for\ncell table %d first!", 
						exp_info.CellID[i]);

					exp_info.CellID[ i ] = 0;

					QMessageBox::message(
							tr( "Attention:" ), 
							str_w );

					return;
				}
			}

			new_str.sprintf( "Cell %d : " + exp_info.Cell[i] + " (Cell Table ID: %d)", 
					i + 1, exp_info.CellID[ i ] );

			cmbb_cell->changeItem( new_str, i );
			
		}

		cell_dlg->show();
	}
	else if ( cell_flag && exp_info.Cell[ i ].isEmpty() )
	{

		QMessageBox::message(
			tr( "Attention:" ), 
			tr( "No Data in this Cell!" ) );

		return;
	}
	else
	{
		QMessageBox::message(
			tr( "Attention:" ), 
			tr( "You need to select 'Load Run' or 'Query DB' first!" ) );

		return;
	}	
}

/*!
	Update <var>lbl_exp</var> and <var>lbl_opt</var> with 
	loaded data's experiment type and optical system.
	\param edit_type an integer variable with 14 different edit types.
*/
void US_ExpData_DB::update_type(int edit_type)
{
	switch (edit_type)
	{
		case 1:
			lbl_exp->setText( "Velocity" );
			lbl_opt->setText( "UV Absorbance" );
			break;
		
		case 2:
			lbl_exp->setText( "Equilibrium" );
			lbl_opt->setText( "UV Absorbance" );
			break;
		
		case 3:
			lbl_exp->setText( "Velocity" );
			lbl_opt->setText( "Raleigh Interference" );
			break;
		
		case 4:
			lbl_exp->setText( "Equilibrium" );
			lbl_opt->setText( "Raleigh Interference" );
			break;
		
		case 5:
			lbl_exp->setText( "Velocity" );				
			lbl_opt->setText( "Fluorescence" );
			 break;
		
		case 6:
			lbl_exp->setText( "Equilibrium" );
			lbl_opt->setText( "Fluorescence" );
			break;
		
		case 7:
			lbl_exp->setText( "Diffusion" );
			lbl_opt->setText( "UV Absorbance" );
			break;
		
		case 8:
			lbl_exp->setText( "Diffusion" );
			lbl_opt->setText( "Raleigh Interference" );
			break;
		
		case 9:
			lbl_exp->setText( "Diffusion" );
			lbl_opt->setText( "Fluorescence" );
			break;
		
		case 10:
			lbl_exp->setText( "Wavelength" );				
			lbl_opt->setText( "UV Absorbance" );
			 break;
		
		case 11:
			lbl_exp->setText( "Wavelength" );				
			lbl_opt->setText( "Intensity" );
			break;
		
		case 12:
			lbl_exp->setText( "Velocity" );				
			lbl_opt->setText( "Intensity" );
			break;
		
		case 13:
			lbl_exp->setText( "Equilibrium" );
			lbl_opt->setText( "Intensity" );
			break;
		
		case 14:
			lbl_exp->setText( "Diffusion" );
			lbl_opt->setText( "Intensity" );
			break;
	}
}

/*!  Update <var>lbl_rt</var> with loaded data's rotor.
    \param rotor an integer variable with 3 different rotor types.
*/
void US_ExpData_DB::update_rotor( int rotor )
{
	switch ( rotor )
	{
		case 0:
			lbl_rt->setText( "Simulation" );
			break;

		case 1:
			lbl_rt->setText( "AN50" );
			break;

		case 2:
			lbl_rt->setText( "AN60" );
			break;
	}
}

/*!  Update description by LineEdit input.  */
void US_ExpData_DB::update_description( const QString& newText )
{
	exp_info.Description = newText;
}

/*!  Open US_DB_TblInvestigator interface for selecting investigator.  */
void US_ExpData_DB::sel_investigator( void )
{
	investigator_dlg = new US_DB_TblInvestigator();
	investigator_dlg->setCaption( "Investigator Information" );
	investigator_dlg->pb_exit->setText( "Accept" );
	investigator_dlg->show();

	connect( investigator_dlg, SIGNAL( valueChanged           ( QString, int ) ), 
	                           SLOT  ( update_investigator_lbl( QString, int ) ) );
}

/*!  Update display with the selected investigator information.
	   \param Display a string variable for show investigator info.
	   \param InvID a integer variable for DB table: <tt>tblInvestigators</tt> 
		 index.
*/
void US_ExpData_DB::update_investigator_lbl( QString Display, int InvID )
{
	exp_info.Invid = InvID;
	
	if ( Display == "" )
		lbl_investigator->setText( " not selected" );
	else
		lbl_investigator->setText( Display );
}

/*!  Open US_Calendar interface for selecting date.  */
void US_ExpData_DB::sel_date()
{
	calendar_dlg = new US_Calendar();
	calendar_dlg->setCaption("Calendar");
	calendar_dlg->show();

	connect( calendar_dlg, SIGNAL( dateChanged    ( QString ) ), 
	                       SLOT  ( update_date_lbl( QString ) ) );
}

/*!  Update display with the selected date.
	   \param Display_Date a string variable for show Date info.
*/
void US_ExpData_DB::update_date_lbl (QString Display_Date)
{
	exp_info.Date = Display_Date;
	lbl_date->setText( exp_info.Date );
}

/*!  Save the experimental data into DB table: <tt>tblExpData</tt>.  */
void US_ExpData_DB::add_db( void )
{
	if ( exp_info.Path.isEmpty() )
	{
		QMessageBox::message(
			tr( "Attention:" ),
			tr( "Please 'Load Run' first!" ) );

		return;
	} 

	if ( exp_info.Invid <= 0 )
	{
		QMessageBox::message(
			tr( "Attention:" ), 
			tr( "Please 'Select Investigator' first!" ) );

		return;	
	}

	if ( exp_info.Description.isEmpty() )
	{
		QMessageBox::message(
			tr( "Attention:" ),
			tr( "Please enter a description\n"
			    "before saving!"));
		return;
	}

	if ( cell_table_unfinished > 0 )
	{
		QMessageBox::message(
			tr( "Attention:" ),
			tr( "Some cell data tables are still incomplete,\n"
			    "please finish all cell data entries before saving." ) );

		return;
	}

	QProgressDialog* pd = progressdialog( "Waiting for DB...", "pd", 5 );

	pd->setMinimumDuration( 0 );
	pd->setProgress       ( 0 );
	qApp->processEvents();
	
	QSqlQuery query;
	
	QString q = "UPDATE tblExpData SET "
	   "Invid = '"       + QString::number(exp_info.Invid)       + "', "
	   "Temperature = '" + QString::number(exp_info.Temperature) + "', "
	   "Duration = '"    + QString::number(exp_info.Duration)    + "', " 
	   "Edit_type = '"   + QString::number(exp_info.Edit_type)   + "', "
	   "Rotor = '"       + QString::number(exp_info.Rotor)       + "', "
	   "Path = '"        + exp_info.Path                         + "', "
	   "Date = '"        + exp_info.Date                         + "', "
	   "Runid = '"       + exp_info.Runid                        + "', ";
	
	for ( int i = 0; i < 8; i++ )
	{	
		if ( exp_info.CellID[ i ] > 0 )
		{
			q += "Cell" + QString::number( i + 1 ) + " = '" 
			  + exp_info.Cell[ i ] + "', ";

			q += "Cell" + QString::number( i + 1 ) + "ID = '" 
			  + QString::number( exp_info.CellID[ i ]) + "', ";
		}
	}

	q += "Description= '"      + exp_info.Description + "' "
	     "WHERE ExpdataID = '" + QString::number( exp_info.ExpdataID) + "';";
	
	if ( ! query.exec( q ) )
	{
		pd->close();
		delete pd;

		QSqlError sqlerr = query.lastError();
		QMessageBox::message(
			tr( "Attention:" ), 
			tr( "Saving to DB table 'tblExpData' failed.\n"
			    "Attempted to execute this command:\n\n"
			    + q + "\n\n"
			    "Causing the following error:\n\n")
			    + sqlerr.text());

		return;
	}
	
	// Copy data to tmp dir
  pd->setLabelText( "Copying files to temporary directory..." );
	pd->setProgress( 1 );
	qApp->processEvents();

	QString tmpDir  = USglobal->config_list.tmp_dir  + "/";
	QString dataDir = USglobal->config_list.data_dir + "/";
	cleanDir( tmpDir );
	
	QDir d( tmpDir );
	d.mkdir( QString( exp_info.Runid ) );

	QDir source( exp_info.Path, "*.*" );
	QStringList tmpFiles = source.entryList();

	QStringList::Iterator it;
	for ( it = tmpFiles.begin(); it != tmpFiles.end(); ++it )
	{
		if ( *it == "."  ||  *it == ".." ) continue;

		copy(           exp_info.Path  + "/" + *it, 
		      tmpDir  + exp_info.Runid + "/" + *it );
	}

  pd->setLabelText( "Creating raw data tarfile..." );
	pd->setProgress( 2 );
	qApp->processEvents();
	
	US_Tar  tar;
	US_Gzip gzip;

	chdir( tmpDir.latin1() );

	QString tarfile = exp_info.Runid + "_rawdata.tar";
	QString files   = exp_info.Runid;  // A directory

	int ret = tar.create( tarfile, files );

	if ( ret != TAR_OK )
	{
		pd->close();
		delete pd;

		QMessageBox::message(
			tr( "UltraScan tar Error:" ),
			tr( tar.explain( ret ) ) + "\n" 
			"Files:" +  tmpDir + files );

		cleanDir( tmpDir );
		return;
	}

	// Compress raw data tar file
  pd->setLabelText( "Compressing raw data tarfile..." );
	pd->setProgress( 3 );
	qApp->processEvents();

	if ( gzip.gzip( tarfile ) != TAR_OK )
	{
		pd->close();
		delete pd;

		QMessageBox::message(
			tr( "UltraScan Error:" ),
			tr( "Unable to compress raw data tar archive.\n" +
			tmpDir + tarfile ) );

		cleanDir( tmpDir );
		return;
	}

	// Insert compressed file into tblRawExpData
  pd->setLabelText( "Uploading raw data tarfile..." );
	pd->setProgress( 4 );
	qApp->processEvents();

	QSqlCursor cursor( "tblRawExpData" );
	cursor.setMode( QSqlCursor::Insert );
	
	QSqlRecord* buffer = cursor.primeInsert();
	buffer->setValue( "ExpdataID", exp_info.ExpdataID );
	
	QString targzfile = tarfile + ".gz";
	if ( ! write_blob( targzfile, buffer, "RawData" ) )
	{
		pd->close();
		delete pd;

		QMessageBox::message(
			tr( "Error:" ),
			tr( "Saving file: " ) + targzfile + "\n"
			+ tr( "to DB table 'tblRawExpData' failed.\n" ) );

		cleanDir ( tmpDir );
		return;
	}

	if ( cursor.insert() <= 0 )
	{
		pd->close();
		delete pd;

		QSqlError err = cursor.lastError();
		QMessageBox::message(
			tr( "Attention:" ), 
			tr( "Saving to DB table 'tblRawExpData' failed.\n"
			    "Error message from MySQL:\n\n" )
			     + err.text() );

		cleanDir ( tmpDir );
		return;
	}

	// Clean up
	pd->close();
	delete pd;
	cleanDir ( tmpDir );

	lb_query->clear();

	QString msg;
	msg.sprintf( "ExpData (ID:%d) has been saved to the database. ", 
			exp_info.ExpdataID );
	
	lb_query->insertItem( msg );
	lb_query->insertItem( "************************************************" );
	lb_query->insertItem( "If you want to change any selected item," );
	lb_query->insertItem( "you will have to delete the existing entry first," );
	lb_query->insertItem( "or you can create a new database Entry" );

	all_done = true;
}


void US_ExpData_DB::cleanDir( const QString& dir )
{
	QDir        current( dir, "*.* *" );
	QStringList entries = current.entryList();
	QFileInfo   f;

	QStringList::Iterator it;
	for ( it = entries.begin(); it != entries.end(); ++it )
	{
		if ( *it == "." || *it == ".." ) continue;
		
		QString path = dir + "/" + *it;

		f.setFile( path );

		if ( f.isDir() )
		{
			cleanDir( path );
			current.rmdir( *it );
		}
		else
			QFile::remove( path );
	}
}


/*!  List experimental data stored in DB table: <tt>tblExpData</tt>.  */
void US_ExpData_DB::query_db()
{
	QSqlCursor cur( "tblExpData" );
	QSqlIndex order = cur.index( "Description" );
	cur.select( order );

	if ( item_ExpdataID   ) delete [] item_ExpdataID;
	if ( item_Description ) delete [] item_Description;

	int maxID             = get_newID( "tblExpData", "ExpdataID" );
	int count             = 0;
	item_ExpdataID        = new int    [ maxID ];
	item_Description      = new QString[ maxID ];
	 QString* display_Str = new QString[ maxID ];

	while( cur.next() )
	{
		int id                    = cur.value( "ExpdataID" ).toInt();
		display_Str     [ count ] = "ExpDataID ("+  QString::number( id ) + "): "+
		                            cur.value( "Description" ).toString();
		item_ExpdataID  [ count ] = id;
		item_Description[ count ] = cur.value( "Description" ).toString();
		count++;					
	}

	if( count > 0 )
	{
		lb_query->clear();

		for( int i = 0; i < count; i++ )
		{
			lb_query->insertItem( display_Str[ i ] );
		}

		query_flag = true;
	}
	else
	{
		lb_query->clear();
		lb_query->insertItem( "No data were found in the DB" );
	}
}

/*!	Select one experimental data <var>item</var> listed in listbox, 
	  this function will show all its info in interface.
*/
void US_ExpData_DB::sel_query( int item )
{
	if ( ! query_flag )  // Can this ever occur?
	{
		QMessageBox::message(
		  tr( "Attention:" ), 
		  tr( "This is not for selecting\n" ) );
		
		return;
	}

	exp_info.ExpdataID   = item_ExpdataID  [ item ];
	exp_info.Description = item_Description[ item ];
		
	QSqlCursor cur( "tblExpData" );
 	
	QStringList orderFields;
	orderFields << "Invid" << "Date";
 	
	QSqlIndex order  = cur.index( orderFields );
 	QSqlIndex filter = cur.index( "ExpdataID" );
 	
	cur.setValue( "ExpdataID", exp_info.ExpdataID );
	cur.select( filter, order );
	
	if ( cur.next() )
	{
		exp_info.Invid       = cur.value( "Invid"      ).toInt();
		exp_info.Path        = cur.value( "Path"       ).toString();
		exp_info.Date        = cur.value( "Date"       ).toString();
		exp_info.Edit_type   = cur.value( "Edit_type"  ).toInt();
		exp_info.Runid       = cur.value( "Runid"      ).toString();
		exp_info.Temperature = cur.value( "Temperature").toString().toFloat();
		exp_info.Duration    = cur.value( "Duration"   ).toString().toFloat();
		exp_info.Rotor       = cur.value( "Rotor"      ).toInt();
		
		QString s;
		
		for ( int i = 0; i < 8; i++ )
		{
			s.sprintf( "Cell%d", i + 1 );
			exp_info.Cell[ i ] = cur.value( s ).toString();
			
			s.sprintf( "Cell%dID", i + 1 );
			exp_info.CellID[ i ] = cur.value( s ).toInt();
			
		}	

		QSqlCursor get( "tblCell" );

		for ( int i = 0; i < 8; i++ )
		{
			s.sprintf( "CellID = %d", exp_info.CellID[ i ] );
			get.select( s );

			if ( get.next() )
			{
				// query Cell Table to get the number of total scans file
				for ( int j = 0; j < 3; j++ )
				{
					s.sprintf( "Wl_Scans_%d", j + 1 );
					Steps += get.value( s ).toInt();
				}

				//get centerpiece, DNA, buffer,peptide data for write_dbinfo()
				exp_info.centerpiece[ i ] = get.value( "CenterpieceID" ).toInt();
				
				for ( int j = 0; j < 4; j++ )
				{
					s.sprintf( "BufferID_%d", j + 1 );
					bufferid[ i ][ j ] = get.value( s ).toInt();
					
					for ( int k = 0; k < 3; k++ )
					{
						if ( k == 0 )
						{
							s.sprintf( "PeptideID_%d", j + 1 );
							pepid[ i ][ j ][ k ] = get.value( s ).toInt();

							s.sprintf( "DNAID_%d", j + 1 );
							DNAid[ i ][ j ][ k ] = get.value( s ).toInt();
						}
						else
						{
							s.sprintf( "Peptide%dID_%d", k + 1, j + 1 );
							pepid[ i ][ j ][ k ] = get.value( s ).toInt();

							s.sprintf( "DNA%dID_%d", k + 1, j + 1 );
							DNAid[ i ][ j ][ k ] = get.value( s ).toInt();
						}
					}
				}
			}
		}

		
		QSqlCursor cur_f( "tblRawExpData" );
		s.sprintf( "ExpdataID = %d", exp_info.ExpdataID );

		cur_f.select( s );

		if ( cur_f.next() )
		{
			QString dataDir = USglobal->config_list.data_dir + "/";
			QDir rawdata_dir( dataDir + exp_info.Runid );

			if ( rawdata_dir.exists() )
			{
				QMessageBox::message(
					tr( "Attention:" ), 
					tr( "The directory: \n '" ) + dataDir + exp_info.Runid + "'\n" +
					tr( "already exists in your data directory, \n"
					    "if you want to retrieve the data data again\n"
					    "from the database, you will have to remove\n"
					    "this directory first"));

				return;
			}

			QProgressDialog* pd = progressdialog( 
			  "Retrieving raw data...", "pd", 4 );

			pd->setProgress       ( 0 );
			pd->setMinimumDuration( 0 );
			qApp->processEvents();

			QString targzfile = exp_info.Runid + "_rawdata.tar.gz";
			QString filename  = make_tempFile( dataDir, targzfile );

			if ( ! read_blob( "Rawdata", cur_f, filename ) )
			{
				QMessageBox::message(
				  tr( "UltraScan Error:" ),
				  tr( "Unable to retrieve Raw data files." ) );

				return;
			}

			// unzip raw data file
			pd->setLabelText( "Decompressing Raw Data..." );
			pd->setProgress( 2 );
			qApp->processEvents();

			chdir( dataDir.latin1() );

			US_Gzip gzip;			
			int     ret = gzip.gunzip( targzfile );

			if ( ret != TAR_OK )
			{
				pd->close();
				delete pd;
				QMessageBox::message(
				  tr( "UltraScan Error:" ),
				  tr( "Unable to uncompress tar archive.\n" +
					tr( gzip.explain( ret ) + "\nFile:\n" )   +
					    dataDir + targzfile ) );

				QFile::remove( dataDir + targzfile );
				return;
			}

			// untar raw data file

  		pd->setLabelText( "Extracting Raw Data..." );
	  	pd->setProgress( 3 );
		  qApp->processEvents();

			US_Tar tar;

			// Sometimes the extracted file is .tar and sometimes _rawdata.tar
			QString tarfile =  exp_info.Runid + "_rawdata.tar";
			if ( ! QFile::exists( tarfile ) )
			{
				tarfile = exp_info.Runid + ".tar";
			}

			if ( ( ret = tar.extract( tarfile ) ) != GZIP_OK )
			{
				pd->close();
				delete pd;
				QMessageBox::message(
					tr( "UltraScan tar extraction Error" ),
					tr( tar.explain( ret ) + "\nFile:\n" ) +
					    dataDir + tarfile );

				QFile::remove( dataDir + tarfile );
			  return;
      }

			write_dbinfo( dataDir + exp_info.Runid );
			
			QFile::remove( dataDir + tarfile );
			pd->close();
			delete pd;
		}
	}

	from_query = true;

	lbl_run         ->setText( exp_info.Path );
	lbl_investigator->setNum ( exp_info.Invid );
	lbl_date        ->setText( exp_info.Date );
	lbl_runid       ->setText( exp_info.Runid );
	lbl_temp        ->setNum ( exp_info.Temperature );
	lbl_dur         ->setNum ( exp_info.Duration );
	le_description  ->setText( exp_info.Description );
	
	update_type ( exp_info.Edit_type );
	update_rotor( exp_info.Rotor );
	update_cell();

	pb_run         ->setEnabled( false );
	pb_investigator->setEnabled( false );
	pb_date        ->setEnabled( false );
	pb_add         ->setEnabled( false );
	Steps = 0;
}

/*!  Open US_DB_Admin to check delete permission.  */
void US_ExpData_DB::check_permission()
{
	US_DB_Admin* db_admin = new US_DB_Admin("");
	db_admin->show();
	connect( db_admin, SIGNAL( issue_permission( bool ) ), 
	                   SLOT  ( delete_db       ( bool ) ) );
}

/*!  Delete one selected experimental data table entry.  */
void US_ExpData_DB::delete_db( bool permission )
{
	QString str;

	if ( ! permission )
	{
		QMessageBox::message(
			tr( "Attention:" ), 
			tr( "Permission denied, please contact your system "
			    "administrator for help" ) );

		return;
	}

	int id = exp_info.ExpdataID;

	if ( ! from_query )
	{
		QMessageBox::message(
			tr( "Attention:" ),
			tr( "Please first select the record\n"
			    "to be deleted from the database"));
	}
	else
	{
		switch( QMessageBox::information( this,
		   tr( "Delete this record?" ),
		   tr( "Clicking 'OK' will delete the selected data from the database"),
		   tr( "OK" ), tr( "CANCEL" ), 0, 1 ) )
		{
			case 0:
			{
				QSqlQuery del;
				QString s;

				s.sprintf( "DELETE FROM tblExpData WHERE ExpdataID = %d;", id );
				del.exec( s );

				s.sprintf( "DELETE FROM tblRawExpData WHERE ExpdataID = %d;", id );
				del.exec( s );

				s.sprintf( "DELETE FROM tblCell WHERE ExperimentID = %d;", id );
				del.exec( s );

				reset();
				break;
			}
			case 1:
				break;
		}
	}
}

/*! Reset all variables to start values. */
void US_ExpData_DB::reset()
{
	query_flag            = false;
	from_query            = false;
	cell_flag             = false;
	all_done              = false;
	Steps                 = 0;
	cell_table_unfinished = 0;

	item_ExpdataID   = NULL;
  item_Description = NULL;

	exp_info.ExpdataID    = -1;
	exp_info.Path         = "";
	exp_info.Invid        = -1;
	exp_info.Date         = current_date;
	exp_info.Edit_type    = -1;
	exp_info.Runid        = "";
	exp_info.Temperature  = -1;
	exp_info.Duration     = -1;
	exp_info.Rotor        = -1;

	for ( int i = 0; i < 8; i++ )
	{
		exp_info.Cell  [ i ] = "";
		exp_info.CellID[ i ] = 0;
	}

	lbl_run         ->setText( "not selected" );
	lbl_investigator->setText( "not selected" );
	lbl_date        ->setText( current_date );
	lbl_runid       ->setText( "" );
	lbl_temp        ->setText( "" );
	lbl_dur         ->setText( "" );
	le_description  ->setText( "" );
	lbl_exp         ->setText( "" );
	lbl_opt         ->setText( "" );
	lbl_rt          ->setText("");

	lb_query        ->clear();
	cmbb_cell       ->clear();

	cmbb_cell       ->insertItem( "No Data" );
	pb_run          ->setEnabled( true );
	pb_investigator ->setEnabled( true );
	pb_date         ->setEnabled( true );
	pb_add          ->setEnabled( true );
}

/*!  Receive widget close events.  */
void US_ExpData_DB::closeEvent( QCloseEvent* e )
{
	QSqlQuery del;
	QString   s;

	// Delete all relative cell data if the expData is not finally saved

	if ( ! all_done &&  ! from_query  &&  exp_info.ExpdataID > 0 )
	{
		s.sprintf( "DELETE FROM tblCell WHERE ExperimentID = %d;", 
			exp_info.ExpdataID );
		del.exec( s );

		s.sprintf( "DELETE FROM tblExpData WHERE ExpdataID = %d;", 
			exp_info.ExpdataID);
		del.exec( s );

		s.sprintf( "DELETE FROM tblRawExpData WHERE ExpdataID = %d;", 
			exp_info.ExpdataID);
		del.exec( s );

	}

	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}

/*! Open a browser to load help page.*/
void US_ExpData_DB::help( void )
{
	US_Help* online_help = new US_Help( this );
	online_help->show_help( "manual/expdata_db.html" );
}

/*! Close the interface. */
void US_ExpData_DB::quit( void )
{
	close();
}

/*!
 	Create a file: <var>db_info.dat</var> to store centerpiece, DNA, buffer, i
	peptide info.
	\param dir the same directory as uncompressed files' directory.
*/
void US_ExpData_DB::write_dbinfo( QString dir )
{
	QString db_file = dir + "/db_info.dat";

	QFile f( db_file );

	if ( ! f.open( IO_WriteOnly | IO_Translate ) )
	{
		QMessageBox::message(
			tr( "Attention:" ), 
			tr( "The file: \n '" ) + db_file+ "'\n" +
			tr( "cannot be opened.\n"
			    "Please check if the disk is write protected,\n"
			    "if the directory exists, or has enough diskspace!" ) );

		return;
	}

	QTextStream ts ( &f );
	
	ts << exp_info.ExpdataID   << "   # Experimental Data ID\n";
	ts << exp_info.Invid       << "   # Investigator ID\n";
	ts << exp_info.Date        << "   # The date of experiment\n";
	ts << exp_info.Description << "\n";
	ts << login_list.dbname    << "   # database name\n";
	ts << login_list.host      << "   # database host\n";
	ts << login_list.driver    << "   # database driver\n";
	ts << exp_info.Rotor       << "   # rotor serial number \n";

	for ( int i = 0; i < 8; i++ )
	{
		if ( exp_info.CellID[ i ] != 0 )
		{
			ts << exp_info.centerpiece[ i ]
			   << "   # centerpiece serial number for cell " << ( i + 1 ) << "\n";

			for( int j = 0; j < 4; j++ )
			{
				ts << bufferid[ i ][ j ] << "\t"
				   << " # buffer serial number for cell "<< ( i + 1 ) << ", channel "
				   << ( j + 1 ) << "\n";

				for ( int k = 0; k < 3; k++ )
				{
					ts << pepid[ i ][ j ][ k ] << "\t" 
					   << " # peptide"               << ( k + 1 ) 
					   << " serial number for cell " << ( i + 1 ) 
					   << ", channel "               << ( j + 1) << "\n";

					ts << DNAid[ i ][ j ][ k ] << "\t"
					   << " # DNA"                  << ( k + 1 )
					   << " serial number for cell "<< ( i + 1 )
					   << ", channel "              << ( j + 1 ) << "\n";
				}
			}
		}

	f.close();
	}
}

/*!
	Retrieve the selected experimental data from database. 
	used by US_DB_RtvDate(), US_DB_RtvDescription(), US_DB_RtvEditType(),
	US_DB_RtvInvestigator().
	\return true retrieve sucessfully.
	\return false otherwise.
*/
bool US_ExpData_DB::retrieve_all( int ExpdataID, QString Display )
{
	int Step = 0;
	QSqlCursor cur( "tblExpData" );
	
	QStringList orderFields;
	orderFields << "Path" << "Invid" << "Date" << "Edit_type" << "Runid"
	            << "Temperature" << "Duration" << "Rotor" << "Description"
	            << "Cell1" << "Cell2" << "Cell3" << "Cell4"
	            << "Cell5" << "Cell6" << "Cell7" << "Cell8";

	QSqlIndex order = cur.index( orderFields );
	QSqlIndex filter = cur.index( "ExpdataID" );

	cur.setValue( "ExpdataID", ExpdataID );
	cur.select( filter, order );

	while( cur.next() )
	{
		exp_info.ExpdataID   = ExpdataID;
		exp_info.Description = cur.value( "Description" ).toString();
		exp_info.Path        = cur.value( "Path"        ).toString();
		exp_info.Invid       = cur.value( "Invid"       ).toInt();
		exp_info.Date        = cur.value( "Date"        ).toString();
		exp_info.Edit_type   = cur.value( "Edit_type"   ).toInt();
		exp_info.Runid       = cur.value( "Runid"       ).toString();
		exp_info.Temperature = cur.value( "Temperature" ).toString().toFloat();
		exp_info.Duration    = cur.value( "Duration"    ).toString().toFloat();
		exp_info.Rotor       = cur.value( "Rotor"       ).toInt();
		
		QString s;
		
		for ( int i = 0; i < 8; i++ )
		{
			s.sprintf( "Cell%d", i + 1 );
			exp_info.Cell[ i ] = cur.value( s ).toString();

			s.sprintf( "Cell%dID", i + 1 );
			exp_info.CellID[ i ] = cur.value( s).toInt();
		}

		QSqlCursor get( "tblCell" );
		
		for ( int i = 0; i < 8; i++ )
		{
			s.sprintf( "CellID = %d", exp_info.CellID[ i ] );
			get.select( s );

			if ( get.next() )
			{
				// query Cell Table to get the number of total scans file
				
				for( int j = 0; j < 3; j++ )
				{
					s.sprintf( "Wl_Scans_%d", j + 1 );
					Step += get.value( s ).toInt();
				}

				//get centerpiece, DNA, buffer,peptide data for write_dbinfo()
				exp_info.centerpiece[ i ] = get.value( "CenterpieceID" ).toInt();

				for ( int j = 0; j < 4; j++ )
				{
					s.sprintf( "BufferID_%d", j + 1 );
					bufferid[ i ][ j ] = get.value( s ).toInt();

					for ( int k = 0; k < 3; k++ )
					{
						if ( k == 0 )
						{
							s.sprintf( "PeptideID_%d", j + 1 );
							pepid[ i ][ j ][ k ] = get.value( s ).toInt();

							s.sprintf( "DNAID_%d", j + 1 );
							DNAid[ i ][ j ][ k ] = get.value( s ).toInt();
						}
						else
						{
							s.sprintf( "Peptide%dID_%d", k + 1, j + 1 );
							pepid[ i ][ j ][ k ] = get.value( s ).toInt();

							s.sprintf( "DNA%dID_%d", k + 1, j + 1 );
							DNAid[ i ][ j ][ k ] = get.value( s ).toInt();
						}
					}
				}
			}
		}

		QSqlCursor cur_f( "tblRawExpData" );
		s.sprintf( "ExpdataID = %d", exp_info.ExpdataID );
		cur_f.select( s );

		QString dataDir = USglobal->config_list.data_dir + "/";

		if ( cur_f.next() )
		{
			QDir rawdata_dir( dataDir + exp_info.Runid );
			
			if ( rawdata_dir.exists() )
			{
				QMessageBox::message(
					tr( "Attention:" ), 
					tr( "The directory: \n '") + dataDir + exp_info.Runid+ "'\n" +
					tr( "already exists in your data directory, \n"
					    "if you want to retrieve the data data again\n"
					    "from the database, you will have to remove\n"
					    "this directory first" ) );

				return false;
		 	}

			QProgressDialog* pd = progressdialog( 
				"Waiting for Data Retrieval...", "pd", 4 );

			pd->setMinimumDuration( 0 );
			pd->setProgress       ( 0 );
			qApp->processEvents();
			
			QString targzfile = exp_info.Runid + "_rawdata.tar.gz";
	   	QString filename = make_tempFile ( dataDir, targzfile );
       	
			if ( ! read_blob( "RawData", cur_f, filename ) )
      {
				QMessageBox::message(
					tr( "UltraScan Error:" ), 
					tr( "Unable to retrieve Raw data files." ) );

				return false;
			}
		
			// unzip raw data file
			pd->setLabelText( "Decompressing Raw Data..." );
			pd->setProgress( 2 );
			qApp->processEvents();

			chdir( dataDir.latin1() );

			US_Gzip gzip;			
			int     ret = gzip.gunzip( targzfile );

			if ( ret != TAR_OK )
			{
				pd->close();
				delete pd;
				QMessageBox::message(
					tr( "UltraScan Error:" ),
					tr( "Unable to uncompress tar archive.\n" +
					tr( gzip.explain( ret ) + "\nFile:\n" )   +
							dataDir + targzfile ) );

				QFile::remove( dataDir + targzfile );
				return false;
			}

			// untar raw data file

			pd->setLabelText( "Extracting Raw Data..." );
			pd->setProgress( 3 );
			qApp->processEvents();

			US_Tar  tar;

			// Sometimes the extracted file is .tar and sometimes _rawdata.tar
			QString tarfile =  exp_info.Runid + "_rawdata.tar";
			if ( ! QFile::exists( tarfile ) )
			{
				tarfile = exp_info.Runid + ".tar";
			}

			if ( ( ret = tar.extract( tarfile ) ) != GZIP_OK )
			{
				pd->close();
				delete pd;
				QMessageBox::message(
					tr( "UltraScan tar extraction Error" ),
					tr( tar.explain( ret )  + "\nFile:\n" ) +
							dataDir + tarfile );

				QFile::remove( dataDir + tarfile );
				return false;
			}

			write_dbinfo( dataDir + exp_info.Runid );
			
			QFile::remove( dataDir + tarfile );
			pd->close();
			delete pd;

		}
		else
		{
			QMessageBox::message(
				tr( "Attention:" ), 
				tr( "No Raw Data found in Database for ExpdataID=" )
				  + QString::number( exp_info.ExpdataID ) );

			return false;
		}
	}

	from_query = true;
	query_flag = false;

	lb_query        ->insertItem( Display );

	lbl_run         ->setText( exp_info.Path       );
	lbl_investigator->setNum ( exp_info.Invid      );
	lbl_date        ->setText( exp_info.Date       );
	lbl_runid       ->setText( exp_info.Runid      );
	lbl_temp        ->setNum ( exp_info.Temperature);
	lbl_dur         ->setNum ( exp_info.Duration   );
	le_description  ->setText( exp_info.Description);
	
	update_type ( exp_info.Edit_type );
	update_rotor( exp_info.Rotor );
	update_cell();

	pb_run         ->setEnabled( false );
	pb_investigator->setEnabled( false );
	pb_date        ->setEnabled( false );
	pb_add         ->setEnabled( false );
	pb_query       ->setEnabled( false );
	pb_reset       ->setEnabled( false );

	return ( true );
}
