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

  has_uvvis        = false;
  has_interference = false;
  has_fluorescense = false;

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
   
  // ---- Testing ----
  QMap < QString, QString > protocol_details;
  // //small: 4 channels
  // protocol_details[ "aprofileguid" ] = QString("d13ffad0-6f27-4fd8-8aa0-df8eef87a6ea");
  // protocol_details[ "protocolName" ] = QString("alexey-abs-itf-test1");
  // protocol_details[ "invID_passed" ] = QString("12");
  // protocol_details[ "runID" ]        = QString("1569");

  // //large: 16 channels
  // protocol_details[ "aprofileguid" ] = QString("255023d4-c725-48ac-8c93-a7c832b5c893");
  // protocol_details[ "protocolName" ] = QString("Alexey-test-report-1");
  // protocol_details[ "invID_passed" ] = QString("12");
  // protocol_details[ "runID" ]        = QString("1570");

  // //small: 4 channels with actual reportsGMP
  // protocol_details[ "aprofileguid" ] = QString("dfd6898c-3744-4e5c-a9cd-94e14e1fb9e5");
  // protocol_details[ "protocolName" ] = QString("test_triple_report3");
  // protocol_details[ "invID_passed" ] = QString("12");
  // protocol_details[ "runID" ]        = QString("1699");
 
  
  // initPanel( protocol_details );
  
  // -----------------

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
  currProto = US_RunProtocol();  //ALEXEY: we need to reset US_Protocol
  currAProf = US_AnaProfile();   //ALEXEY: we need to reset US_AnaProfile
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
  
  progress_msg = new QProgressDialog ("Accessing run's protocol...", QString(), 0, 7, this);
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
  
  //Debug: protocol
  qDebug() << "Protocols' details: -- "
	   << currProto.investigator
	   << currProto.runname
	   << currProto.protoname
	   << currProto.protoID
	   << currProto.project
	   << currProto.temperature
	   << currProto.temeq_delay
	   << currProto.exp_label;

  //read AProfile into US_AnaProfile structure
  sdiag = new US_AnalysisProfileGui;
  sdiag->inherit_protocol( &currProto );
  currAProf              = sdiag->currProf;
  currAProf.protoGUID    = currProto.protoGUID;
  currAProf.protoID      = currProto.protoID;
  currAProf.protoname    = currProto.protoname;
  //2DSA parms
  cAP2                   = currAProf.ap2DSA;
  //PCSA parms
  cAPp                   = currAProf.apPCSA;
  //Channel descriptions
  chndescs               = currAProf.chndescs;
  //Channel alt_descriptions
  chndescs_alt           = currAProf.chndescs_alt;
  //Channel reports
  ch_reports             = currAProf.ch_reports;
  //Channel wavelengths
  ch_wvls                = currAProf.ch_wvls;
    
  progress_msg->setValue( 4 );
  qApp->processEvents();

  //Debug: AProfile
  QString channel_desc_alt = chndescs_alt[ 0 ];
  QString channel_desc     = chndescs[ 0 ];
  QString wvl              = QString::number( ch_wvls[ channel_desc ][ 0 ] );
  US_ReportGMP reportGMP   = ch_reports[ channel_desc_alt ][ wvl ];
  //US_ReportGMP reportGMP = ch_reports[ chndescs_alt[ 0 ] ][ QString::number( ch_wvls[ chndescs_alt[ 0 ] ][ 0 ] ) ];
  
  qDebug() << "AProfile's && ReportGMP's details: -- "
	   << currAProf.aprofname
	   << currAProf.protoname
	   << currAProf.chndescs
	   << currAProf.chndescs_alt
	   << currAProf.lc_ratios
	   << cAP2.parms[ 0 ].channel
	   << cAPp.parms[ 0 ].channel
	   << reportGMP.rmsd_limit
	   << reportGMP.wavelength
	   << reportGMP.reportItems[ 0 ].type;
  
  ////
  get_current_date();
  progress_msg->setValue( 5 );
  qApp->processEvents();
  
  ////
  format_needed_params();
  progress_msg->setValue( 6 );
  qApp->processEvents();
  
  /// 
  assemble_pdf();
  progress_msg->setValue( 7 );
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
  //HEADER: begin
  QString html_header = tr( 
    "<div align=left>"
      "Created, %1<br>"
      "with UltraScan-GMP<br>"
      "by AUC Solutions<br>"
    "</div>"
		     )
    .arg( current_date )
    ;
  //HEADER: end

  
  //TITLE: begin
  QString html_title = tr(
    "<h1 align=center>REPORT FOR RUN <br><i>%1</i></h1>"
    "<hr>"
			  )
    .arg( currProto. protoname + "-run" + runID )    //1
    ;
  //TITLE: end

  QString html_paragraph_open = tr(
    "<p align=justify>"
				   )
    ;
  
  //GENERAL: begin
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
    //.arg( currProto. runname)       //2  
    .arg( currProto. protoname + "-run" + runID) //2
    .arg( currProto. project)       //3
    .arg( currProto. temperature)   //4
    .arg( currProto. temeq_delay)   //5
    ;
  //GENERAL: end


  //ROTOR/LAB: begin
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
  //ROTOR/LAB: end 	  

  
  //SPEEDS: begin
  QString html_speed = tr(
    "<h3 align=left>Speed Parameters </h3>"
      "<table>"
        "<tr><td>Rotor Speed  (RPM):    </td>                   <td>%1</td></tr>"
        "<tr><td>Acceleration (RMP/sec): </td>                  <td>%2</td></tr>"
        "<tr><td>Active Scaning Time:    </td>                  <td>%3</td></tr>"
        "<tr><td>Stage Delay:          </td>                    <td>%4</td></tr>"
        "<tr><td>Total Time (without equilibration):  </td>     <td>%5</td></tr>"
        "<tr><td><br><b><i>UV-visible optics (total):</i></b>   </td>  "
        "<tr><td>Delay to First Scan:            </td>          <td>%6</td></tr>"
        "<tr><td>Scan Interval:                  </td>          <td>%7</td></tr>"
        "<tr><td><br><b><i>Interference optics (per cell):</i></b></td>   "
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
  //SPEEDS: end


  //CELLS: begin
  QString html_cells = tr(
    "<h3 align=left>Cell Centerpiece Usage </h3>"
			   )
     ;
   
  ncells_used = currProto. rpCells. nused;
  html_cells += tr(
     "# of Used Cells:  %1<br>"
		    )
    .arg( QString::number ( ncells_used ) )                                        //1
    ;
   
  html_cells += tr(
      "<table>"
		   )
    ;

  for ( int i=0; i<ncells_used; ++i )
    {
      QString cell        = QString::number( currProto. rpCells. used[i].cell );
      QString centerpiece = currProto. rpCells. used[i].centerpiece;
      QString cell_label  = QString( tr( "Centerpiece" ) );
      QString window      = currProto. rpCells. used[i].windows;
      
      //check if last cell counterbalance
      if ( i == ncells_used - 1 && 
	   currProto. rpCells. used[i].cbalance.contains( tr("counterbalance") ) )
	{
	  cell_label = tr("Counterbalance");
	  centerpiece = currProto. rpCells. used[i].cbalance;
	  window = tr("N/A");
	}
	
      html_cells += tr(
		       "<tr>" 
		       "<td>Cell Number:</td><td>%1</td> &nbsp;&nbsp;&nbsp;&nbsp;" 
		       "<td> %2: </td><td> %3 </td> &nbsp;&nbsp;&nbsp;&nbsp;"
		       "<td>Windows:</td><td> %4 </td>"
		       "</tr>"
			)
	.arg( cell )                          //1
	.arg( cell_label )                    //2
	.arg( centerpiece )                   //3
	.arg( window )                        //4 
	;
    }
  
  html_cells += tr(
      "</table>"
    "<hr>"
		   )
    ;
  //CELLS: end

  
  //SOLUTIONS: begin
  QString html_solutions = tr(
			      "<h3 align=left>Solutions for Channels</h3>"			      
			      )
    ;
   
  nsol_channels = currProto.rpSolut.nschan;
  html_solutions += tr(
     "# of Solution Channels:  %1 <br>"
		    )
    .arg( nsol_channels )                                                   //1
    ;
  
  for ( int i=0; i<nsol_channels; ++i )
    {
      html_solutions += tr(
			   "<table>"		   
			   "<tr>"
			      "<td><b>Cell/Channel:</b> &nbsp;&nbsp;&nbsp;&nbsp; </td> <td><b>%1</b></td>"
			   "</tr>"
			   "</table>"
			)
	.arg( currProto. rpSolut. chsols[i].channel )                       //1
	;

      QString sol_id      = currProto. rpSolut. chsols[i].sol_id;
      QString sol_comment = currProto. rpSolut. chsols[i].ch_comment;
      add_solution_details( sol_id, sol_comment, html_solutions );
    }

  html_solutions += tr( "<hr>" );
  //SOLUTIONS: end


  //OPTICS: begin
  QString html_optical = tr(
			    "<h3 align=left>Optics </h3>"
			   )
    ;
  nchan_optics = currProto. rpOptic. nochan;
  html_optical += tr(
		     "# of Channels With Optics:  %1<br>"
		     )
    .arg( QString::number( nchan_optics ))                                                     //1
    ;
  html_optical += tr(
		     "<table>"
		     );

  for ( int i=0; i<nchan_optics; ++i )
    {
      QString channel  = currProto. rpOptic. chopts[i].channel;
      QString scan1    = currProto. rpOptic. chopts[i].scan1;
      QString scan2    = currProto. rpOptic. chopts[i].scan2;
      QString scan3    = currProto. rpOptic. chopts[i].scan3;

      // //test
      // if ( i == 3 )
      // 	scan2 = tr( "Interf." );
      // if ( i == nchan_optics - 2 )
      // 	scan3 = tr( "Fluorescense" );
      // if ( i == nchan_optics - 1 )
      // 	scan2 = tr( "Interference" );
      // //////

      
      html_optical += tr(
			 "<tr>" 
			    "<td>Cell/Channel:</td><td>%1</td> &nbsp;&nbsp;&nbsp;&nbsp;"
			 )
	.arg( channel );                      //1
      
      
      if ( !scan1.isEmpty() )
	{
	  has_uvvis = true;
	  html_optical += tr(
			     "<td> %1: </td> &nbsp;&nbsp;&nbsp;&nbsp;"
			     )
	    .arg( scan1 );                    //1
	}
      else
	html_optical += tr("<td> &nbsp;&nbsp;&nbsp;&nbsp; </td>");
      
      if ( !scan2.isEmpty() )
	{
	  has_interference = true;
	  html_optical += tr(
			     "<td> %1: </td> &nbsp;&nbsp;&nbsp;&nbsp;"
			     )
	    .arg( scan2 );                        //1
	}
      else
	html_optical += tr("<td> &nbsp;&nbsp;&nbsp;&nbsp; </td>" );    
      
      if ( !scan3.isEmpty() )
	{
	  has_fluorescense = true;
	  html_optical += tr(
			     "<td> %1: </td> &nbsp;&nbsp;&nbsp;&nbsp;"
			     )
	    .arg( scan3 );                        //1
	}
      else
	html_optical += tr("<td> &nbsp;&nbsp;&nbsp;&nbsp; </td>" );    
           
      
      html_optical += tr(
			 "</tr>"
			 );
    }
  
  html_optical += tr(
      "</table>"
    "<hr>"
		   )
    ;
  //OPTICS: end


  //RANGES: begin
  QString html_ranges = tr(
			   "<h3 align=left> Ranges </h3>"
			   )
    ;
  nchan_ranges  = currProto. rpRange. nranges;
  html_ranges  += tr(
		     "# of Channels With Ranges:  %1<br>"
		     )
    .arg( QString::number( nchan_ranges ))                                  //1
    ;
  
  for ( int i=0; i < nchan_ranges; ++i )
    {
      html_ranges += tr(
			"<table>"		   
			  "<tr>"
			    "<td><b>Cell/Channel:</b> &nbsp;&nbsp;&nbsp;&nbsp; </td> <td><b>%1</b></td>"
			  "</tr>"
			"</table>"
			)
	.arg( currProto. rpRange. chrngs[i].channel )                       //1
	;

      int w_count     = currProto. rpRange. chrngs[i].wvlens.size();
      double  w_min   = currProto. rpRange. chrngs[i].wvlens[0];
      double  w_max   = currProto. rpRange. chrngs[i].wvlens[ w_count - 1 ];
      double  r_min   = currProto. rpRange. chrngs[i].lo_rad;
      double  r_max   = currProto. rpRange. chrngs[i].hi_rad;
      QString w_range = QString::number( w_min ) + tr(" to ") + QString::number( w_max );
      QString r_range = QString::number( r_min ) + tr(" to ") + QString::number( r_max );

      //wavelengths:
      QString all_wvl = QString("");
      QList< double > wvl_list = currProto. rpRange. chrngs[i].wvlens;
      int counter = 0;
      for (int  jj =0; jj < w_count; jj++)
	{
	  ++counter;
	  all_wvl += tr("%1").arg( wvl_list[jj] );
         if( jj != wvl_list.size() -1 )
	   all_wvl += tr(", ");
         if(counter % 8 == 0)
	   all_wvl += tr("<br>");
	}
      
      html_ranges += tr(
			"<table style=\"margin-left:30px\">"
			  "<tr><td> Selected Wavelength count: </td>  <td> %1 </td> </tr>"
			  "<tr><td> Selected Wavelength range: </td>  <td> %2 </td> </tr>"
			  "<tr><td> Radius range:              </td>  <td> %3 </td> </tr>"
			  "<tr><td> Selected Wavelengths:      </td>  <td> %4 </td> </tr>"
			"</table>"
			)
	.arg( QString::number( w_count ) )        //1
	.arg( w_range )                           //2
	.arg( r_range )                           //3
	.arg( all_wvl )                           //4	
	;
      
    }  

  html_ranges += tr( "<hr>" ) ;
  //RANGES: end

  
  //SCAN_COUNT: begin
  QString html_scan_count = tr(
			       "<h3 align=left> Scan Counts and Scan Intervals For Optics in Use </h3>"
			       "&nbsp;&nbsp;<br>"
			       )
    ;

  double scanintv     = currProto. rpSpeed. ssteps[0].scanintv;
  double scanintv_int = currProto. rpSpeed. ssteps[0].scanintv_int;
  int scancount       = currProto. rpSpeed. ssteps[0].scancount;
  int scancount_int   = currProto. rpSpeed. ssteps[0].scancount_int;

  //UV-vis
  QString html_scan_count_uv = tr(
				  "<table>"		   
				    "<tr>"
				      "<td><b><i>UV-visible optics:</i></b></td>"
				    "</tr>"
				  "</table>"
			)
    ;
  html_scan_count_uv += tr(
			   "<table style=\"margin-left:30px\">"
			     "<tr><td> Scan Interval:             </td>  <td> %1 </td> </tr>"
			     "<tr><td> # Scans per Triple:        </td>  <td> %2 </td> </tr>"
			   "</table>"
			   )
    .arg( scanint_uvvis_str )                             //1
    .arg( QString::number( scancount ) )                  //2
    ;
  
  //Interference
  QString html_scan_count_int = tr(
				   "<table>"		   
				     "<tr>"
				       "<td><b><i>Interference optics:</i></b></td>"
				     "</tr>"
				   "</table>"
				   )
    ;
  html_scan_count_int += tr(
			    "<table style=\"margin-left:30px\">"
			      "<tr><td> Scan Interval:             </td>  <td> %1 </td> </tr>"
			      "<tr><td> # Scans per Cell:          </td>  <td> %2 </td> </tr>"
			    "</table>"
			   )
    .arg( scanint_int_str )                                //1
    .arg( QString::number( scancount_int ) )               //2
    ;


  if ( has_uvvis )
    html_scan_count += html_scan_count_uv;
  if ( has_interference )
    html_scan_count += html_scan_count_int;  
  
  html_scan_count += tr( "<hr>" ) ;
  //SCAN_COUNT: end

  
  //APROFILE: begin
  //ANALYSIS: General settings && Reports: begin
  QString html_analysis_profile = tr(
				     "<h3 align=left> Analysis Profile: General Settings and Reports  </h3>"
				     "&nbsp;&nbsp;<br>"
				     )
    ;
  html_analysis_profile += tr(
			      "<table>"		   
			         "<tr> <td> Profile Name:  &nbsp;&nbsp;&nbsp;&nbsp; </td>  <td> %1 </td></tr>"
			         "<tr> <td> Protocol Name: &nbsp;&nbsp;&nbsp;&nbsp; </td>  <td> %2 </td></tr>"
			      "</table>"
			      "<br>"
			      )
    .arg( currAProf.aprofname  )         //1
    .arg( currAProf.protoname  )         //2
    ;

  int nchna   = currAProf.pchans.count();
  for ( int i = 0; i < nchna; i++ )
    {
      QString channel_desc_alt = chndescs_alt[ i ];
      QString channel_desc     = chndescs[ i ];
      
      html_analysis_profile += tr(
				  "<table>"		   
				     "<tr>"
				        "<td><b>Channel:</b> &nbsp;&nbsp;&nbsp;&nbsp; </td> <td><b>%1</b></td>"
				      "</tr>"
				  "</table>"
				  )
	.arg( channel_desc )              //1
	;

      QString loading_ratio  = QString::number( currAProf.lc_ratios[ i ] );
      QString ratio_tol      = QString::number( currAProf.lc_tolers[ i ] );
      QString volume         = QString::number( currAProf.l_volumes[ i ] );
      QString volume_tol     = QString::number( currAProf.lv_tolers[ i ] );
      QString data_end       = QString::number( currAProf.data_ends[ i ] );

      QString run_analysis;
      if ( currAProf.analysis_run[ i ] )
	run_analysis = tr("YES");
      else
	run_analysis = tr("NO");
      
      html_analysis_profile += tr(
				  "<table style=\"margin-left:30px\">"
				     "<tr><td> Loading Ratio:              </td>  <td> %1 </td> </tr>"
				     "<tr><td> Ratio Tolerance (&#177;%):  </td>  <td> %2 </td> </tr>"
				     "<tr><td> Loading Volume (&#181;l):   </td>  <td> %3 </td> </tr>"
				     "<tr><td> Volume Tolerance (&#177;%): </td>  <td> %4 </td> </tr>"
				     "<tr><td> Data End (cm):              </td>  <td> %5 </td> </tr>"
				  "</table>"
				  )
	.arg( loading_ratio )             //1
	.arg( ratio_tol )                 //2
	.arg( volume )                    //3
	.arg( volume_tol )                //4
	.arg( data_end )                  //5
	;

      
      html_analysis_profile += tr(
				  "<table style=\"margin-left:30px\">"
				     "<tr><td> <i>Run Analysis:</i>        </td>  <td> %1 </td> </tr>"
				  "</table>"
				  )
	.arg( run_analysis )               //1
	;
				  
      bool triple_report = false;
      
      if ( currAProf.analysis_run[ i ] )
	{
	  //check what representative wvl is:
	  QString rep_wvl = QString::number( currAProf.wvl_edit[ i ] );
	  html_analysis_profile += tr(
				      "<table style=\"margin-left:50px\">"
				         "<tr><td> Wavelength for Edit, 2DSA-FM & Fitmen Stages (nm):   </td>  <td> %1 </td> </tr>"
				      "</table>"
				      )
	    .arg( rep_wvl )                //1
	    ;
	  
	  //now check if report will be run:
	  QString run_report;
	  if ( currAProf.report_run[ i ] )
	    {
	      run_report    = tr("YES");
	      triple_report = true;
	    }
	  else
	    run_report = tr("NO");

	  html_analysis_profile += tr(
				      "<table style=\"margin-left:30px\">"
				         "<tr><td> <i> Run Report: </i>    </td>  <td> %1 </td> </tr>"
				      "</table>"
				      )
	    .arg( run_report)              //1
	    ;
	}

       
      //Separate Report | ReportItems table
      if ( triple_report )
	{
	  QList < double > chann_wvls                  = ch_wvls[ channel_desc ];
	  QMap < QString, US_ReportGMP > chann_reports = ch_reports[ channel_desc_alt ];
	    
	  int chann_wvl_number = chann_wvls.size();

	  for ( int jj = 0; jj < chann_wvl_number; ++jj )
	    {
	      QString wvl            = QString::number( chann_wvls[ jj ] );
	      QString triple_name    = channel_desc.split(":")[ 0 ] + "/" + wvl;
	      US_ReportGMP reportGMP = chann_reports[ wvl ];
	      
	      html_analysis_profile += tr(
					  "<table style=\"margin-left:50px\">"
					     "<tr><td> <b><i> Report Parameters for Triple: </i> &nbsp;&nbsp;&nbsp; %1 </b></td> </tr>"
					  "</table>"
					  )
		.arg( triple_name )                                             //1
		;

	      //Exp. duration entered in the Channel Report Editor
	      QList< int > hms_tot;
	      double total_time = reportGMP.experiment_duration;
	      US_RunProtocol::timeToList( total_time, hms_tot );
	      QString exp_dur_str = QString::number( hms_tot[ 0 ] ) + "d " + QString::number( hms_tot[ 1 ] ) + "h " + QString::number( hms_tot[ 2 ] ) + "m ";

	      html_analysis_profile += tr(
					  "<table style=\"margin-left:70px\">"
					     "<tr><td> Total Concentration: </td>  <td> %1 </td> </tr>"
					     "<tr><td> RMSD (upper limit):  </td>  <td> %2 </td> </tr>"
					     "<tr><td> Average Intensity:   </td>  <td> %3 </td> </tr>"
					     "<tr><td> Experiment Duration: </td>  <td> %4 </td> </tr>"
					  "</table>"
					  )
		.arg( QString::number( reportGMP.tot_conc ) )                    //1
		.arg( QString::number( reportGMP.rmsd_limit )  )                 //2
		.arg( QString::number( reportGMP.av_intensity )  )               //3
		.arg( exp_dur_str )                                              //4
		;

	      //Now go over ReportItems for the current triple:
	      int report_items_number = reportGMP.reportItems.size();

	      for ( int kk = 0; kk < report_items_number; ++kk )
		{
		  US_ReportGMP::ReportItem curr_item = reportGMP.reportItems[ kk ];
		  
		  html_analysis_profile += tr(
					      "<table style=\"margin-left:90px\">"
					         "<tr><td> <b><i> Report Item #%1: </i> &nbsp;&nbsp; Type | Method Parameters </b></td> </tr>"
					      "</table>"
					      )
		    .arg( QString::number( kk + 1 ) )                                 //1
		    ;

		  html_analysis_profile += tr(
					      "<table style=\"margin-left:110px\">"
					         "<tr><td> Type:                  </td>  <td> %1 </td> </tr>"
					         "<tr><td> Method:                </td>  <td> %2 </td> </tr>"
					         "<tr><td> Range Low:             </td>  <td> %3 </td> </tr>"
					         "<tr><td> Range High:            </td>  <td> %4 </td> </tr>"
					         "<tr><td> Integration Value:     </td>  <td> %5 </td> </tr>"
					         "<tr><td> Tolerance (%):         </td>  <td> %6 </td> </tr>"
					         "<tr><td> Fraction of Total (%): </td>  <td> %7 </td> </tr>"
					      "</table>"
					      )
		    .arg( curr_item.type )                                      //1
		    .arg( curr_item.method   )                                  //2
		    .arg( QString::number( curr_item.range_low )  )             //3
		    .arg( QString::number( curr_item.range_high )  )            //4
		    .arg( QString::number( curr_item.integration_val )  )       //5
		    .arg( QString::number( curr_item.tolerance )  )             //6
		    .arg( QString::number( curr_item.total_percent )  )         //7
		    ;
		  
		}
	      
	    }
	}
      
      html_analysis_profile += tr(
				  "<br>"
				  )
	;
      }
  html_analysis_profile += tr( "<hr>" ) ;
  //ANALYSIS: General settings && Reports: end
    
    
  //ANALYSIS: 2DSA per-channel settings: begin
  QString html_analysis_profile_2dsa = tr(
					  "<h3 align=left> Analysis Profile: 2DSA Controls </h3>"
					  "&nbsp;&nbsp;<br>"
					  )
    ;

  //Job Flow Summary:
  html_analysis_profile_2dsa += tr(
				   "<table>"		   
				    "<tr>"
				       "<td><b>Job Flow Summary:</b></td>"
				    "</tr>"
				  "</table>"
			)
    ;
  html_analysis_profile_2dsa += tr(
				   "<table style=\"margin-left:10px\">"
				      //2DSA
				      "<tr>"
				         "<td> \"2DSA  (TI Noise)\": </td>"
				         "<td> Run?                  </td>"    
                                         "<td> %1                    </td>"
				         "<td> &nbsp;&nbsp;          </td>"
				         "<td> &nbsp;&nbsp;          </td>"
                                      "</tr>"

				      //2DSA-FM
				      "<tr>"
                                         "<td> \"2DSA-FM  (TI+RI Noise)\": </td>"
				         "<td> Run?                        </td>"
				         "<td> %2                          </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
                                      "</tr>"
				      "<tr>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> Meniscus Grid Points:       </td>"
				         "<td> %3                          </td>"
				      "</tr>"
				      "<tr>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> Meniscus Fit Range (cm):    </td>"
				         "<td> %4                          </td>"
				      "</tr>"
				      "<tr>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> Fit Type:                   </td>"
				         "<td> %5                          </td>"
				      "</tr>"				   

				      //FITMEN
				      "<tr>"
				         "<td> \"FITMEN\":                 </td>"
				         "<td> Run?                        </td>"
				         "<td> %6                          </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				      "</tr>"
				      "<tr>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> Auto-pick?                  </td>"
				         "<td> %7                          </td>"
				      "</tr>"

				      //2DSA-IT
				      "<tr>"
				         "<td> \"2DSA-IT  (TI+RI Noise)\": </td>"
				         "<td> Run?                        </td>"
				         "<td> %8                          </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
                                      "</tr>"
				      "<tr>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> Refinement Iterations:      </td>"
				         "<td> %9                          </td>"
				      "</tr>"
				   
				      //2DSA-MC
				      "<tr>"
				         "<td> \"2DSA-MC\":                </td>"
				         "<td> Run?                        </td>"
				         "<td> %10                         </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				      "</tr>"
				      "<tr>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> MonteCarlo Iterations:      </td>"
				         "<td> %11                         </td>"
				      "</tr>"
				   
				   "</table>"
				   "<br>"
				   )
    .arg( ( cAP2.job1run ? tr( "YES" ) : tr( "NO" ) ) )       //1

    .arg( ( cAP2.job2run ? tr( "YES" ) : tr( "NO" ) ) )       //2
    .arg( QString::number( cAP2.grpoints ) )                  //3
    .arg( QString::number( cAP2.fitrng ) )                    //4
    .arg( cAP2.fmb )                                          //5

    .arg( ( cAP2.job3run ? tr( "YES" ) : tr( "NO" ) ) )       //6
    .arg( ( cAP2.job3auto ? tr( "YES" ) : tr( "NO" ) ) )      //7

    .arg( ( cAP2.job4run ? tr( "YES" ) : tr( "NO" ) ) )       //8
    .arg( ( QString::number( cAP2.rfiters ) ) )               //9
    
    .arg( ( cAP2.job5run ? tr( "YES" ) : tr( "NO" ) ) )       //10
    .arg( ( QString::number( cAP2.mciters ) ) )               //11	  
    ;
  

  //Per-Channel params:
  html_analysis_profile_2dsa += tr(
				   "<table>"		   
				     "<tr>"
				       "<td><b>Per-Channel Profile:</b></td>"
				     "</tr>"
				   "</table>"
				   "<br>"
				   )
    ;
  int nchna_2dsa   = cAP2.parms.size();
  for ( int i = 0; i < nchna_2dsa; i++ )
    {
      html_analysis_profile_2dsa += tr(
				       "<table>"		   
				         "<tr>"
				            "<td><b>Channel:</b> &nbsp;&nbsp;&nbsp;&nbsp; </td> <td><b>%1</b></td>"
				          "</tr>"
				       "</table>"
				       )
	.arg( cAP2.parms[ i ].channel )            //1
	;


      QString s_data = QString::number( cAP2.parms[ i ].s_min ) + ", " +
	               QString::number( cAP2.parms[ i ].s_max ) + ", " +
                       QString::number( cAP2.parms[ i ].s_grpts );

      QString ff0_data = QString::number( cAP2.parms[ i ].k_min ) + ", " +
	               QString::number( cAP2.parms[ i ].k_max ) + ", " +
                       QString::number( cAP2.parms[ i ].k_grpts );

      QString grid_rep =  QString::number( cAP2.parms[ i ].gridreps );

      QString custom_grid = cAP2.parms[ i ].cgrid_name;

      QString vary_vbar = cAP2.parms[ i ].varyvbar ? tr( "YES" ) : tr( "no" );

      QString const_ff0 = QString::number( cAP2.parms[ i ].ff0_const );

      html_analysis_profile_2dsa += tr(
                                       "<table style=\"margin-left:30px\">"
                                          "<tr><td> s Min, Max, Grid Points:     </td>  <td> %1 </td> </tr>"
                                          "<tr><td> f/f0 Min, Max, Grid Points:  </td>  <td> %2 </td> </tr>"
                                          "<tr><td> Grid Repetitions:            </td>  <td> %3 </td> </tr>"
                                          "<tr><td> Custom Grid:                 </td>  <td> %4 </td> </tr>"
                                          "<tr><td> Varying Vbar:                </td>  <td> %5 </td> </tr>"
                                          "<tr><td> Constant f/f0:               </td>  <td> %6 </td> </tr>"
                                       "</table>"
                                       )
	.arg( s_data )                     //1                                                                                 
        .arg( ff0_data )                   //2                                                                                 
	.arg( grid_rep )                   //3                                                                                 
        .arg( custom_grid )                //4                                                                                 
	.arg( vary_vbar )                  //5                                                                                 
        .arg( const_ff0 )                  //6
	;
    }
  
  html_analysis_profile_2dsa += tr( "<hr>" ) ;
  //ANALYSIS: 2DSA per-channel settings: end
  
  
  //ANALYSIS: PCSA per-channel settings: begin 
  QString html_analysis_profile_pcsa = tr(
					  "<h3 align=left> Analysis Profile: PCSA Controls </h3>"
					  "&nbsp;&nbsp;<br>"
					  )
    ;

  html_analysis_profile_pcsa += tr(
				   "<table style=\"margin-left:10px\">"
				     //PCSA
				     "<tr>"
				         "<td> \"PCSA\":    </td>"
				         "<td> Run?         </td>"    
                                         "<td> %1           </td>"
				      "</tr>"
				   "</table>"
				   )
    .arg( cAPp.job_run ? tr( "YES" ) : tr( "no" ) )
    ;

  if ( cAPp.job_run )
    {
      int nchna_pcsa   = cAPp.parms.size();
      for ( int i = 0; i < nchna_pcsa; i++ )
	{
	  html_analysis_profile_pcsa += tr(
					   "<table>"		   
					     "<tr>"
					        "<td><b>Channel:</b> &nbsp;&nbsp;&nbsp;&nbsp; </td> <td><b>%1</b></td>"
					     "</tr>"
					   "</table>"
					   )
	    .arg( cAPp.parms[ i ].channel )            //1
	    ;

	  QString x_data =  cAPp.parms[ i ].x_type + ", " +
	                    QString::number( cAPp.parms[ i ].x_min ) + ", " +
	                    QString::number( cAPp.parms[ i ].x_max );
	  QString y_data =  cAPp.parms[ i ].y_type + ", " +
	                    QString::number( cAPp.parms[ i ].y_min ) + ", " +
	                    QString::number( cAPp.parms[ i ].y_max );	  
	  QString z_data =  cAPp.parms[ i ].z_type + ", " +
	                    QString::number( cAPp.parms[ i ].z_value );
	                 	  
	  html_analysis_profile_pcsa += tr(
					   "<table style=\"margin-left:30px\">"
					     "<tr><td> Curve Type:                </td>  <td> %1 </td> </tr>"
					     "<tr><td> X Axis Type, Min, Max:     </td>  <td> %2 </td> </tr>"
					     "<tr><td> Y Axis Type, Min, Max:     </td>  <td> %3 </td> </tr>"
					     "<tr><td> Z Axis Type, Value:        </td>  <td> %4 </td> </tr>"
					     "<tr><td> Variations Count:          </td>  <td> %5 </td> </tr>"
					     "<tr><td> Grid Fit Iterations:       </td>  <td> %6 </td> </tr>"
					     "<tr><td> Curve Resolution Points:   </td>  <td> %7 </td> </tr>"
					     "<tr><td> Noise Type:                </td>  <td> %8 </td> </tr>"
					     "<tr><td> Tikhonov Regularization:   </td>  <td> %9 </td> </tr>"
					     "<tr><td> Tikhonov Alpha:            </td>  <td> %10 </td> </tr>"
					     "<tr><td> MonteCarlo Iterations:     </td>  <td> %11 </td> </tr>"
					   "</table>"
					   )
	    .arg( cAPp.parms[ i ].curv_type )                     //1
	    .arg( x_data )                                        //2
	    .arg( y_data )                                        //3
	    .arg( z_data )                                        //4
	    .arg( QString::number( cAPp.parms[ i ].varcount ) )   //5
	    .arg( QString::number( cAPp.parms[ i ].grf_iters ) )  //6
	    .arg( QString::number( cAPp.parms[ i ].creso_pts ) )  //7
	    .arg( cAPp.parms[ i ].noise_type )                    //8
	    .arg( cAPp.parms[ i ].treg_type )                     //9
	    .arg( QString::number( cAPp.parms[ i ].tr_alpha ) )   //10
	    .arg( QString::number( cAPp.parms[ i ].mc_iters ) )   //11
	;
	}
    }
  
  html_analysis_profile_pcsa += tr( "<hr>" ) ;
  //ANALYSIS: PCSA per-channel settings: end
  //APROFILE: end
  
  
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

  //Main assembly
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
    + html_optical
    + html_ranges
    + html_scan_count
    + html_analysis_profile
    + html_analysis_profile_2dsa
    + html_analysis_profile_pcsa 
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

//Fetch Ranges details && add to html_solutions
void US_Reports_auto::add_ranges_details( QString& html_ranges )
{
  
}
  
//Fetch Solution details && add to html_solutions
void US_Reports_auto::add_solution_details( const QString sol_id, const QString sol_comment, QString& html_solutions )
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
  //End of reading Solution:

  //add general solution details to html_solutions string:
  html_solutions += tr(
		       "<table style=\"margin-left:5px\">"
		       "<caption align=left> <b><i>Solution Information </i></b> </caption>"
		       "</table>"
		       
		       "<table style=\"margin-left:20px\">"
		         "<tr><td> Solution Name: </td>               <td> %1 </td> </tr>"
		         "<tr><td> Solution Comment: </td>            <td> %2 </td> </tr>"
		         "<tr><td> Common VBar (20&#8451;):</td>      <td> %3 </td> </tr>"
		         "<tr><td> Storage Temperature (&#8451;):</td><td> %4 </td> </tr>" 
		       "</table>"
		       )
    .arg( solution->solutionDesc )                          //1
    .arg( sol_comment )                                     //2
    .arg( QString::number( solution->commonVbar20 ))        //3
    .arg( QString::number( solution->storageTemp ))         //4
    ;
  

  //Get analytes information
  html_solutions += tr(
		       "<table style=\"margin-left:20px\">"
		          "<caption align=left> <b><i>Analytes Information</i></b> </caption>"
		       "</table>"
		       )
    ;
  
  QString analyte_gen_info;
  QString analyte_detailed_info;
  int num_analytes = solution->analyteInfo.size();
  for (int i=0; i < num_analytes; ++i )
    {
      //clear analyte's strings:
      analyte_gen_info = QString("");
      analyte_detailed_info = QString("");

     
      US_Analyte analyte = solution->analyteInfo[ i ].analyte;
      QString a_name     = analyte.description;
      QString a_amount   = QString::number( solution->analyteInfo[ i ].amount );

      analyte_gen_info += tr(
			     "<tr>"
			       "<td> <i>Analyte #%1: </i></td> &nbsp;&nbsp;&nbsp;&nbsp; "
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
				  "<tr> <td> Vbar (20 &#8451;): </td>  <td> %3</td></tr>"
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

      //add info on the current analyte to html_solutions string 
      html_solutions += tr(
			   "<table style=\"margin-left:40px\">"
			      "%1"
			   "</table>"
			   "<table style=\"margin-left:60px\">"
			      "%2"
			   "</table>"  
			   )
	.arg( analyte_gen_info )
	.arg( analyte_detailed_info )
	;     
      
    }


  //general buffer information
  QString buffer_gen_info      = QString("");
  QString buffer_detailed_info = QString("");
  US_Buffer buffer = solution->buffer;

  buffer_gen_info += tr(
			"<tr>"
			  "<td> Buffer Name: </td> <td> %1 </td> "
			"</tr>"
			     )
    .arg( buffer.description )                //1
    ;

  buffer_detailed_info += tr(
			     "<tr><td> Density (20&#8451;, g/cm<sup>3</sup>):  </td>   <td>%1</td>  </tr>"
			     "<tr><td> Viscosity (20&#8451;, cP): </td>   <td>%2</td>               </tr>"
			     "<tr><td> pH:       </td>   <td>%3 </td>                              </tr>"
			     "<tr><td> Compressibility:</td>   <td>%4</td>                         </tr>"			     
			     )
    .arg( QString::number( buffer.density ) )                    //1
    .arg( QString::number( buffer.viscosity ) )                  //2
    .arg( QString::number( buffer.pH ) )                         //3
    .arg( QString::number( buffer.compressibility ) )            //4
    ;

  //buffer components (if any)
  QString buffer_components_info = QString("");
  for ( int i=0; i < buffer.component.size(); ++i )
    {
      QString component_desc =
	buffer.component[i].name
	+ " (" + QString::number( buffer.concentration[ i ] ) + " "
	+ buffer.component[i].unit
	+ ")";


      buffer_components_info += tr(
				   "<tr><td> Component Name:</td> &nbsp;&nbsp;  <td>%1</td>  </tr>"
				   )
	.arg( component_desc )
	;
    }
  

      


  //append html string: buffer general info
  html_solutions += tr(
		       "<table style=\"margin-left:30px\">"
		          "<caption align=left> <b><i>Buffer Information</i></b> </caption>"
		       "</table>"
		       
		       "<table style=\"margin-left:70px\">"
		           "%1"
		       "</table>"
		       "<table style=\"margin-left:100px\">"
		           "%2"
		       "</table>"
		         "<table style=\"margin-left:130px\">"
		           "%3"
		       "</table>"  
		       "<br>"
		       )
    .arg( buffer_gen_info )
    .arg( buffer_detailed_info )
    .arg( buffer_components_info )
    ;
}




// ============== BELOW is an old per-trpile treatment ===========================//
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
*/
