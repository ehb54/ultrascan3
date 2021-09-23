#include <QPrinter>
#include <QPdfWriter>
#include <QPainter>

#include "us_autoflow_reports.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_protocol_util.h"
#include "us_math2.h"

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
  genL->setSpacing        ( 2 );
  genL->setContentsMargins( 2, 2, 2, 2 );

  // QLabel* lb_triple   = us_banner( tr( "Triple" ) );
  // QLabel* lb_passed   = us_banner( tr( "Analysis Convergence" ) );
  // QLabel* lb_action1  = us_banner( tr( "View" ) );
  // QLabel* lb_action2  = us_banner( tr( "Download" ) );
  // lb_triple->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
  // lb_passed->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
  // lb_action1->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
  // lb_action2->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );

  // int row = 1;
  // genL->addWidget( lb_triple,  row,    0, 1, 3 );
  // genL->addWidget( lb_passed,  row,    3, 1, 3 );
  // genL->addWidget( lb_action1, row,    6, 1, 3 );
  // genL->addWidget( lb_action2, row++,  9, 1, 3 );

  int row = 1;
  int ihgt        = lb_hdr1->height();
  QSpacerItem* spacer2 = new QSpacerItem( 20, 100*ihgt, QSizePolicy::Expanding);
  genL->setRowStretch( row, 1 );
  genL->addItem( spacer2,  row++,  0, 1, 1 );
  
  pb_download_report = us_pushbutton( tr( "View Report" ) );
  connect( pb_download_report, SIGNAL( clicked() ), this, SLOT  ( view_report() ) );
  
  //genL->addWidget( pb_download_report, row++,  4, 2, 30, Qt::AlignBottom );
  genL->addWidget( pb_download_report );
  pb_download_report->setVisible( false );
    
  panel->addLayout( genL );

  qDebug() << "Column | Raw counts: -- " << genL->columnCount() << genL->rowCount();

  setMinimumSize( 950, 450 );
  adjustSize();
   
  // // ---- Testing ----
  // QMap < QString, QString > protocol_details;
  // protocol_details[ "aprofileguid" ] = QString("d13ffad0-6f27-4fd8-8aa0-df8eef87a6ea");
  // protocol_details[ "protocolName" ] = QString("alexey-abs-itf-test1");
  // protocol_details[ "invID_passed" ] = QString("12");
  // protocol_details[ "runID" ]        = QString("1569");
  
  // initPanel( protocol_details );
  
  // // // -----------------

  panel->addStretch();

}

//view report
void US_Reports_auto::view_report ( void )
{
  qDebug() << "Opening PDF at -- " << filePath;

  //Open with OS's applicaiton settings ?
  QDesktopServices::openUrl(QUrl( filePath ));
}

//reset
void US_Reports_auto::reset_report_panel ( void )
{
  currProto = US_RunProtocol();  //ALEXEY: do we need to reset US_Protocol ?
  pb_download_report->setVisible( false );
}
  
//init correct # of us_labels rows based on passed # stages from AProfile
void US_Reports_auto::initPanel( QMap < QString, QString > & protocol_details )
{
  Array_of_triples.clear();
  
  AProfileGUID       = protocol_details[ "aprofileguid" ];
  ProtocolName_auto  = protocol_details[ "protocolName" ];
  invID              = protocol_details[ "invID_passed" ].toInt();
  runID              = protocol_details[ "runID" ];
  
  int num_triples = 10;
  
  progress_msg = new QProgressDialog ("Accessing run's protocol...", QString(), 0, 6, this);
  //progress_msg->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
  progress_msg->setWindowFlags(Qt::Tool | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
  //progress_msg->setWindowModality(Qt::WindowModal);
  progress_msg->setModal( true );
  progress_msg->setWindowTitle(tr("Report Generation"));
  QFont font_d  = progress_msg->property("font").value<QFont>();
  QFontMetrics fm(font_d);
  int pixelsWide = fm.width( progress_msg->windowTitle() );
  qDebug() << "Progress_msg: pixelsWide -- " << pixelsWide;
  progress_msg ->setMinimumWidth( pixelsWide*2 );
  progress_msg->adjustSize();
  
  progress_msg->setAutoClose( false );
  progress_msg->setValue( 0 );
  //progress_msg->setRange( 1, nedtfs );
  progress_msg->show();
 

  //read protocol into US_RunProtocol structure:
  US_Passwd pw;
  QString masterPW = pw.getPasswd();
  US_DB2 db( masterPW );
  
  if ( db.lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
			    tr( "Read protocol: Could not connect to database \n" ) + db.lastError() );
      return;
    }

  progress_msg->setValue( 1 );
  qApp->processEvents();
  QString xmlstr( "" );
  US_ProtocolUtil::read_record_auto( ProtocolName_auto, invID,  &xmlstr, NULL, &db );
  progress_msg->setValue( 2 );
  qApp->processEvents();
  QXmlStreamReader xmli( xmlstr );
  currProto. fromXml( xmli );
  progress_msg->setValue( 3 );
  qApp->processEvents();
  
  //Debug
  qDebug() << "Protocols' details: -- "
	   << currProto.investigator
	   << currProto.runname
	   << currProto.protoname
	   << currProto.protoID
	   << currProto.project
	   << currProto.temperature
	   << currProto.temeq_delay
	   << currProto.exp_label;
  ////
  get_current_date();
  progress_msg->setValue( 4 );
  qApp->processEvents();
  
  ////
  format_needed_params();
  progress_msg->setValue( 5 );
  qApp->processEvents();
  
  /// 
  assemble_pdf();
  progress_msg->setValue( 6 );
  qApp->processEvents();


  progress_msg->setValue( progress_msg->maximum() );
  progress_msg->close();
  pb_download_report->setVisible( true );

  //Inform user of the PDF location
  QMessageBox::information( this, tr( "Report PDF Ready" ),
			    tr( "Report PDF was saved at \n%1\n\n"
				"You can view it by pressing \'View Report\' button below" ).arg( filePath ) );
}

//Format selected parameters to D H M format
void US_Reports_auto::format_needed_params()
{
  //Duration
  QList< int > hms_dur;
  double duration = currProto. rpSpeed. ssteps[0].duration;
  US_RunProtocol::timeToList( duration, hms_dur );
  duration_str = QString::number( hms_dur[ 0 ] ) + "d " + QString::number( hms_dur[ 1 ] ) + "h " + QString::number( hms_dur[ 2 ] ) + "m ";
  
  //Delay Stage
  QList< int > hms_delay_stage;
  double delay_stage = currProto. rpSpeed. ssteps[0].delay_stage;
  US_RunProtocol::timeToList( delay_stage, hms_delay_stage );
  delay_stage_str = //QString::number( hms_delay_stage[ 0 ] ) + "d " +
    QString::number( hms_delay_stage[ 1 ] ) + "h " + QString::number( hms_delay_stage[ 2 ] ) + "m ";

  //Total Time
  QList< int > hms_tot;
  double total_time = currProto. rpSpeed. ssteps[0].total_time;
  US_RunProtocol::timeToList( total_time, hms_tot );
  total_time_str = QString::number( hms_tot[ 0 ] ) + "d " + QString::number( hms_tot[ 1 ] ) + "h " + QString::number( hms_tot[ 2 ] ) + "m ";

  //UV-vis delay
  QList< int > hms_delay_uvvis;
  double delay_uvvis = currProto. rpSpeed. ssteps[0].delay;
  qDebug() << "Delay UV-vis: " << delay_uvvis;
  US_RunProtocol::timeToList( delay_uvvis, hms_delay_uvvis );
  delay_uvvis_str = QString::number( hms_delay_uvvis[ 0 ] ) + "d " + QString::number( hms_delay_uvvis[ 1 ] ) + "h " + QString::number( hms_delay_uvvis[ 2 ] ) + "m ";
  qDebug() << "Delay UV-vis str: " << delay_uvvis_str;

  //UV-vis scanint
  QList< int > hms_scanint_uvvis;
  double scanint_uvvis = currProto. rpSpeed. ssteps[0].scanintv;
  qDebug() << "ScanInt UV-vis: " << scanint_uvvis;
  US_RunProtocol::timeToList( scanint_uvvis, hms_scanint_uvvis );
  scanint_uvvis_str = //QString::number( hms_scanint_uvvis[ 0 ] ) + "d " +
    QString::number( hms_scanint_uvvis[ 1 ] ) + "h " + QString::number( hms_scanint_uvvis[ 2 ] ) + "m " + QString::number( hms_scanint_uvvis[ 3 ] ) + "s ";
  qDebug() << "ScanInt UV-vis str: " << scanint_uvvis_str;

  //Interference delay
  QList< int > hms_delay_int;
  double delay_int = currProto. rpSpeed. ssteps[0].delay_int;
  qDebug() << "Delay Interference: " << delay_int;
  US_RunProtocol::timeToList( delay_int, hms_delay_int );
  delay_int_str = QString::number( hms_delay_int[ 0 ] ) + "d " + QString::number( hms_delay_int[ 1 ] ) + "h " + QString::number( hms_delay_int[ 2 ] ) + "m ";
  qDebug() << "Delay Interference str: " << delay_int_str;
  
  //Interference scanint
  QList< int > hms_scanint_int;
  double scanint_int = currProto. rpSpeed. ssteps[0].scanintv_int;
  qDebug() << "ScanInt Interference: " << scanint_int;
  US_RunProtocol::timeToList( scanint_int, hms_scanint_int );
  scanint_int_str = //QString::number( hms_scanint_uvvis[ 0 ] ) + "d " +
    QString::number( hms_scanint_int[ 1 ] ) + "h " + QString::number( hms_scanint_int[ 2 ] ) + "m " + QString::number( hms_scanint_int[ 3 ] ) + "s ";
  qDebug() << "ScanInt Interference str: " << scanint_int_str;
  
}

//get current date
void US_Reports_auto::get_current_date()
{
  QDate dNow(QDate::currentDate());
  QString fmt = "MM/dd/yyyy";

  current_date = dNow.toString( fmt );
  qDebug() << "Current date -- " << current_date;
}

//Start assembling PDF file
void US_Reports_auto::assemble_pdf()
{
  QString html_header = tr( 
    "<div align=left>"
      "Created, %1<br>"
      "with UltraScan-GMP<br>"
      "by AUC Solutions<br>"
    "</div>"
		     )
    .arg( current_date )
    ;
  
  QString html_title = tr(
    "<h1 align=center>REPORT FOR RUN <br><i>%1</i></h1>"
    "<hr>"
			  )
    .arg( currProto. protoname )    //1
    ;

  QString html_paragraph_open = tr(
    "<p align=justify>"
				   )
    ;
  
  QString html_general = tr(
    
    "<h3 align=left>General Settings</h3>"
      "<table>"
        "<tr><td>Investigator: </td>                           <td>%1</td></tr>"
        "<tr><td>Run Name:</td>                                <td>%2</td></tr>"
        "<tr><td>Project:</td>                                 <td>%3</td></tr>"
        "<tr><td>Run Temperature (&#8451;):</td>               <td>%4</td></tr>"
        "<tr><td>Temperature-Equilibration Delay (mins):</td>  <td>%5</td></tr>"
      "</table>"
    "<hr>"
			    )
    .arg( currProto. investigator)  //1
    .arg( currProto. runname)       //2  
    .arg( currProto. project)       //3
    .arg( currProto. temperature)   //4
    .arg( currProto. temeq_delay)   //5
    ;
    
  QString html_lab_rotor = tr(
    "<h3 align=left>Lab/Rotor Parameters</h3>"
      "<table>"
        "<tr><td>Laboratory:</td>      <td>%1</td></tr>"
        "<tr><td>Rotor: </td>          <td>%2</td></tr>"
        "<tr><td>Calibration Date:</td>     <td>%3</td></tr>"
      "</table>"
    "<hr>"
    "<h3 align=left>Optima Machine/Operator </h3>"
      "<table>"
        "<tr><td>Optima: </td>           <td>%4</td></tr>"
        "<tr><td>Operator: </td>         <td>%5</td></tr>"
        "<tr><td>Experiment Type:</td>   <td>%6</td></tr>"
      "</table>"
    "<hr>"
				)
    .arg( currProto. rpRotor.laboratory )  //1
    .arg( currProto. rpRotor.rotor )       //2
    .arg( currProto. rpRotor.calibration)  //3
    .arg( currProto. rpRotor.instrname )   //4
    .arg( currProto. rpRotor.opername  )   //5
    .arg( currProto. rpRotor.exptype )     //6
    ;

  QString html_speed = tr(
    "<h3 align=left>Speed Parameters </h3>"
      "<table>"
        "<tr><td>Rotor Speed  (RPM):    </td>                   <td>%1</td></tr>"
        "<tr><td>Acceleration (RMP/sec): </td>                  <td>%2</td></tr>"
        "<tr><td>Active Scaning Time:    </td>                  <td>%3</td></tr>"
        "<tr><td>Stage Delay:          </td>                    <td>%4</td></tr>"
        "<tr><td>Total Time (without equilibration):  </td>     <td>%5</td></tr>"
        "<tr><td><br><i>UV-visible optics (total):</i> </td>  "
        "<tr><td>Delay to First Scan:            </td>          <td>%6</td></tr>"
        "<tr><td>Scan Interval:                  </td>          <td>%7</td></tr>"
        "<tr><td><br><i>Interference optics (per cell):</i></td>   "
        "<tr><td>Delay to First Scan:  </td>                    <td>%8</td></tr>"
        "<tr><td>Scan Interval:        </td>                    <td>%9</td></tr>"
      "</table>"
    "<hr>"
			  )
    .arg( QString::number ( currProto. rpSpeed. ssteps[0].speed ) )       //1
    .arg( QString::number ( currProto. rpSpeed. ssteps[0].accel ) )       //2
    .arg( duration_str    )                                               //3
    .arg( delay_stage_str )                                               //4
    .arg( total_time_str  )                                               //5
    .arg( delay_uvvis_str )                                               //6
    .arg( scanint_uvvis_str )                                             //7
    .arg( delay_int_str   )                                               //8
    .arg( scanint_int_str )                                               //9
    ;

  QString html_cells = tr(
    "<h3 align=left>Cell Centerpiece Usage </h3>"
			   )
     ;
   
  ncells_used = currProto. rpCells. nused;
  html_cells += tr(
     "# of Used Cells:  %1"
		    )
    .arg( ncells_used )                                                   //1
    ;
   
  html_cells += tr(
      "<table>"
		   )
    ;

  for ( int i=0; i<ncells_used; ++i )
    {
      html_cells += tr(
        "<tr> <td>Cell Number:</td><td>%1</td> &nbsp;&nbsp;&nbsp;&nbsp; <td>Centerpiece:</td><td>%2</td> &nbsp;&nbsp;&nbsp;&nbsp; <td>Windows:</td><td>%3</td>  </tr>"
			)
	.arg( QString::number( currProto. rpCells. used[i].cell ))        //1
	.arg( currProto. rpCells. used[i].centerpiece )                   //2
	.arg( currProto. rpCells. used[i].windows )                       //3 
	;
    }
  
  html_cells += tr(
      "</table>"
    "<hr>"
		   )
    ;

  QString html_solutions = tr(
    "<h3 align=left>Solutions for Each Channel</h3>"			      
			      )
    ;
   
  nsol_channels = currProto.rpSolut.nschan;
  html_solutions += tr(
     "# of Solution Channels:  %1"
		    )
    .arg( nsol_channels )                                                   //1
    ;
  
  for ( int i=0; i<nsol_channels; ++i )
    {
      html_solutions += tr(
       "<table>"		   
        "<tr> <td>Cell/Channel:</td><td>%1</td> &nbsp;&nbsp;&nbsp;&nbsp; <td>Solution:</td><td>%2</td> &nbsp;&nbsp;&nbsp;&nbsp; <td>Comment:</td><td>%3</td> </tr>"
       "</table>"
			)
	.arg( currProto. rpSolut. chsols[i].channel )                       //1
	.arg( currProto. rpSolut. chsols[i].solution )                      //2
	.arg( currProto. rpSolut. chsols[i].ch_comment )                    //3
	;

      QString sol_id = currProto. rpSolut. chsols[i].sol_id;
      add_solution_details( sol_id, html_solutions );
    }

  html_solutions += tr( "<hr>" );
  
  QString html_paragraph_close = tr(
    "</p>"
				    )
    ;

  QString html_footer = tr( 
    "<div align=right>"
       "<br>End of report: <i>\"%1\"</i>"
    "</div>"
			    )
    .arg( currProto. protoname )
    ;

  QString html_assembled = QString("");
  html_assembled +=
    html_header
    + html_title
    + html_paragraph_open
    + html_general
    + html_lab_rotor
    + html_speed
    + html_cells
    + html_solutions
    + html_paragraph_close
    + html_footer;
    
  QTextDocument document;
  document.setHtml( html_assembled );
  
  QPrinter printer(QPrinter::PrinterResolution);
  printer.setOutputFormat(QPrinter::PdfFormat);
  printer.setPaperSize(QPrinter::Letter);

  QString fileName  = currProto. protoname + "-run" + runID + ".pdf";
  filePath  = US_Settings::tmpDir() + "/" + fileName;
  printer.setOutputFileName( filePath );
  printer.setPageMargins(QMarginsF(15, 15, 15, 15));
  
  document.print(&printer);
}

//Fetch Solution details && add to html_solutions
void US_Reports_auto::add_solution_details( const QString sol_id, QString& html_solutions )
{
  //get Solution info by ID:
  US_Passwd pw;
  QString masterPW = pw.getPasswd();
  US_DB2 db( masterPW );
  
  if ( db.lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Database Problem" ),
         tr( "Database returned the following error: \n" ) +  db.lastError() );
      
      return;
    }

  US_Solution*   solution = new US_Solution;
  int solutionID = sol_id.toInt();

  int status = US_DB2::OK;
  status = solution->readFromDB  ( solutionID, &db );

  // Error reporting
  if ( status == US_DB2::NO_BUFFER )
    {
      QMessageBox::information( this,
				tr( "Attention" ),
				tr( "The buffer this solution refers to was not found.\n"
				    "Please restore and try again.\n" ) );
    }
  
  else if ( status == US_DB2::NO_ANALYTE )
    {
      QMessageBox::information( this,
				tr( "Attention" ),
				tr( "One of the analytes this solution refers to was not found.\n"
				    "Please restore and try again.\n" ) );
    }
  
  else if ( status != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Database Problem" ),
			    tr( "Database returned the following error: \n" ) +  db.lastError() );
    }
  

  //analytes information
  QString analyte_gen_info      = QString("");
  QString analyte_detailed_info = QString("");
  int num_analytes = solution->analyteInfo.size();
  for (int i=0; i < num_analytes; ++i )
    {
      US_Analyte analyte = solution->analyteInfo[ i ].analyte;
      QString a_name     = analyte.description;
      QString a_amount   = QString::number( solution->analyteInfo[ i ].amount );

      analyte_gen_info += tr(
			     "<tr>"
			     "<td>Analyte #%1:</td> &nbsp;&nbsp;&nbsp;&nbsp; "
			     "<td> Name: </td>  <td> %2</td> &nbsp;&nbsp;&nbsp;&nbsp;"
			     "<td> Molar Ratio:</td> <td>%3</td>"
			     "</tr>"
			     )
	.arg( QString::number( i + 1 ) )  //1
	.arg( a_name )                    //2
	.arg( a_amount )                  //3
	;

      QString a_mw     = QString::number( analyte.mw ) + " D";
      QString a_vbar20 = QString::number( analyte.vbar20 );
      
      int seqlen       = analyte.sequence.length();
      QString seqsmry  = analyte.sequence;
      int total       = 0;
      
      if ( seqlen == 0 )
	seqsmry         = tr( "(empty)" );
      else
	{
	  seqsmry         = seqsmry.toLower()
	    .remove( QRegExp( "[\\s0-9]" ) );
	  seqlen          = seqsmry.length();
	  if ( seqlen > 25 )
	    {
	      seqsmry        = QString( seqsmry ).left( 10 ) + " ... "
		+ QString( seqsmry ).mid( seqlen - 10 );
	    }
	  //seqsmry          += "\n  ";
	  seqsmry          += "<br>  ";
	  
	  for ( int ii = 0; ii < 26; ii++ )
	    {
	      QString letter  = QString( QChar( 'a' + ii ) );
	      int lcount      = analyte.sequence.count( letter );
	      total          += lcount;
	      if ( lcount > 0 )
		{
		  seqsmry     += QString().sprintf( "%d", lcount )
		    + " " + letter.toUpper() + ", ";
		  //seqsmry          += "\n  ";
		}
	    }
	  seqsmry     += QString().sprintf( "%d", total ) + " tot";
	}

      QString a_type   = tr( "Carbohydrate/Other" );
      a_type           = analyte.type == ( US_Analyte::PROTEIN ) ? tr( "Protein" ) : a_type;
      a_type           = analyte.type == ( US_Analyte::DNA ) ? tr( "DNA" ) : a_type;
      a_type           = analyte.type == ( US_Analyte::RNA ) ? tr( "RNA" ) : a_type;

      //absorbing residues
      US_Math2::Peptide p;
      US_Math2::calc_vbar( p, analyte.sequence, 20.0 );
      analyte.mw         = ( analyte.mw == 0.0 ) ? p.mw : analyte.mw;
      
      // Absorbing residues
      int cys = int(p.c);
      int hao = int(p.j);
      int orn = int(p.o);
      int trp = int(p.w);
      int tyr = int(p.y);
      int all_abs = 0;
      all_abs = cys + hao + orn + trp + tyr;
      qDebug() << "Tot AAs: " << all_abs;
      QString absorbing_residues = tr( "(empty)" );
      if (all_abs > 0)
	{
	  absorbing_residues = "";
	  if ( cys > 0)
	    absorbing_residues += QString().sprintf( "%d", cys ) + " " + "Cys"  + ", ";
	  if ( hao > 0)
	 absorbing_residues += QString().sprintf( "%d", hao ) + " " + "Hao"  + ", ";
	  if ( orn > 0)
	    absorbing_residues += QString().sprintf( "%d", orn ) + " " + "Orn"  + ", ";
	  if ( trp > 0)
	    absorbing_residues += QString().sprintf( "%d", trp ) + " " + "Trp"  + ", ";
	  if ( tyr > 0)
	    absorbing_residues += QString().sprintf( "%d", tyr ) + " " + "Tyr"  + ", ";

	  absorbing_residues += QString().sprintf( "%d", all_abs ) + " tot";
	}

      //remeber to exclude absorbing residues for non-protein species
      
      analyte_detailed_info += tr(
				  "<tr> <td> Type: </td> <td>%1</td>              </tr>"
				  "<tr> <td> Molecular Weight: </td>  <td> %2</td></tr>"
				  "<tr> <td> Vbar (20 deg. C): </td>  <td> %3</td></tr>"
				  "<tr> <td> Sequence Length:  </td>  <td> %4</td></tr>"
				  "<tr> <td> Sequence Summary: </td>  <td> %5</td></tr>"
				  )
	.arg( a_type )                        //1
	.arg( a_mw )                          //2 
	.arg( a_vbar20 )                      //3
	.arg( QString::number( seqlen  ) )    //4
	.arg( seqsmry  )                      //5
	;

      //add absorbig residues if PROTEINS
      if ( analyte.type == 0 )
	{
	  analyte_detailed_info += tr(
				      "<tr> <td> AAs absorbing at 280 nm: </td> <td>%1</td> </tr>"
				      "<tr> <td> E280: </td>  <td> %2</td>                  </tr>"
				      "<tr> <td> Extinction count: </td>  <td> %3</td>      </tr>"
				      )
	    .arg( absorbing_residues )
	    .arg( QString::number( analyte.extinction[ 280.0 ] ) )
	    .arg( QString::number( analyte.extinction.keys().count() ) )
	    ;
	}
    }


  //general buffer information
  QString buffer_gen_info = QString("");
  US_Buffer buffer = solution->buffer;

  buffer_gen_info += tr(
			"<tr>"
			"<td> Name: </td>     <td> %1</td> &nbsp;&nbsp;&nbsp;&nbsp;"
			"<td> Density:</td>   <td>%2</td> &nbsp;&nbsp;&nbsp;&nbsp;"
			"<td> Viscosity:</td> <td>%3</td> &nbsp;&nbsp;&nbsp;&nbsp;"
			"</tr>"
			     )
    .arg( buffer.description )                //1
    .arg( buffer.density )                    //2
    .arg( buffer.viscosity )                  //3
    ;  
  
  //append html string: solution general info
  html_solutions += tr(
  "<table style=\"margin-left:20px\">"
     "<caption align=left> <i>Solution Information for \'%1\'</i> </caption>"
        "<tr><td>Common VBar (20&#8451;):</td> <td> %2</td> &nbsp;&nbsp;&nbsp;&nbsp; <td>Storage Temperature, &#8451;:</td>  <td>%3</td> </tr>"
  "</table>"
		       )
    .arg( solution->solutionDesc )
    .arg( QString::number( solution->commonVbar20 ))
    .arg( QString::number( solution->storageTemp ))
    ;
      
  //append html string: analytes general info
  html_solutions += tr(
  "<table style=\"margin-left:20px\">"
     "<caption align=left> <i>Analytes Information</i> </caption>"
       "%1"
  "</table>"
  "<table style=\"margin-left:50px\">"
      "%2"
  "</table>"  
		       )
    .arg( analyte_gen_info )
    .arg( analyte_detailed_info )
    ;

  //append html string: buffer general info
  html_solutions += tr(
  "<table style=\"margin-left:20px\">"
     "<caption align=left> <i>Buffer Information</i> </caption>"
       "%1"
  "</table>"
  "<br>"
		       )
    .arg( buffer_gen_info )
    ;
 
  
}

/*
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
*/


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
