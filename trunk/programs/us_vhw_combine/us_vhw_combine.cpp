//! \file us_vhw_combine.cpp

#include <QApplication>

#include "us_vhw_combine.h"
#include "us_select_runid.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_matrix.h"
#include "us_constants.h"
#include "us_passwd.h"
#include "us_report.h"
#include "us_dataIO2.h"
#include "us_util.h"
#include "qwt_legend.h"

// main program
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_vHW_Combine w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

// US_vHW_Combine class constructor
US_vHW_Combine::US_vHW_Combine() : US_Widgets()
{
   // set up the GUI (mostly handled in US_AnalysisBase)

   setWindowTitle( tr( "Combined van Holde - Weischet Distributions:" ) );
   setPalette( US_GuiSettings::frameColor() );
   dbg_level     = US_Settings::us_debug();

   QBoxLayout*  mainLayout   = new QHBoxLayout( this );
   QGridLayout* leftLayout   = new QGridLayout;
   QVBoxLayout* rightLayout  = new QVBoxLayout;
   mainLayout ->setSpacing        ( 2 );
   mainLayout ->setContentsMargins( 2, 2, 2, 2 );
   leftLayout ->setSpacing        ( 0 );
   leftLayout ->setContentsMargins( 0, 1, 0, 1 );
   rightLayout->setSpacing        ( 0 );
   rightLayout->setContentsMargins( 0, 1, 0, 1 );

   dkdb_cntrls             = new US_Disk_DB_Controls(
         US_Settings::default_data_location() );
   QPushButton* pb_loadda  = us_pushbutton( tr( "Load Data"  ) );
                pb_saveda  = us_pushbutton( tr( "Save Data"  ) );
                pb_resetd  = us_pushbutton( tr( "Reset Data" ) );
                pb_resetp  = us_pushbutton( tr( "Reset Plot" ) );
   QPushButton* pb_help    = us_pushbutton( tr( "Help"       ) );
   QPushButton* pb_close   = us_pushbutton( tr( "Close"      ) );

   pb_saveda->setEnabled( false );
   pb_resetd->setEnabled( false );
   pb_resetp->setEnabled( false );

   QLabel* lb_distrtype  = us_banner(
         tr( "Select Distribution Plot Type(s):" ) );
   QLabel* lb_runinfo    = us_banner( tr( "Information for this Run:" ) );
   QLabel* lb_runid      = us_label ( tr( "Current Run ID:" ) );
   QLabel* lb_svproj     = us_label ( tr( "Save Plot under Project:" ) );
   QLabel* lb_runids     = us_banner( tr( "Run IDs:"                  ) );
   QLabel* lb_triples    = us_banner( tr( "Cell / Channel / Wavelength:" ) );

   QLayout* lo_distrib  = us_checkbox( tr( "Integral" ), ck_distrib,  true  );
   QLayout* lo_envelope = us_checkbox( tr( "Envelope" ), ck_envelope, false );

   le_runid      = us_lineedit( "(current run ID)", -1, true );
   cmb_svproj    = us_comboBox();
   lw_runids     = us_listwidget();
   lw_triples    = us_listwidget();

   int row = 0;
   leftLayout->addLayout( dkdb_cntrls,  row++, 0, 1, 8 );
   leftLayout->addWidget( pb_loadda,    row,   0, 1, 4 );
   leftLayout->addWidget( pb_saveda,    row++, 4, 1, 4 );
   leftLayout->addWidget( pb_resetd,    row,   0, 1, 4 );
   leftLayout->addWidget( pb_resetp,    row++, 4, 1, 4 );
   leftLayout->addWidget( pb_help,      row,   4, 1, 2 );
   leftLayout->addWidget( pb_close,     row++, 6, 1, 2 );
   leftLayout->addWidget( lb_distrtype, row++, 0, 1, 8 );
   leftLayout->addLayout( lo_distrib,   row,   0, 1, 4 );
   leftLayout->addLayout( lo_envelope,  row++, 4, 1, 4 );
   leftLayout->addWidget( lb_runinfo,   row++, 0, 1, 8 );
   leftLayout->addWidget( lb_runid,     row,   0, 1, 3 );
   leftLayout->addWidget( le_runid,     row++, 3, 1, 5 );
   leftLayout->addWidget( lb_svproj,    row,   0, 1, 3 );
   leftLayout->addWidget( cmb_svproj,   row++, 3, 1, 5 );
   leftLayout->addWidget( lb_runids,    row++, 0, 1, 8 );
   leftLayout->addWidget( lw_runids,    row,   0, 3, 8 );
   row    += 3;
   leftLayout->addWidget( lb_triples,   row++, 0, 1, 8 );
   leftLayout->addWidget( lw_triples,   row,   0, 5, 8 );
   row    += 5;
   leftLayout->setRowStretch( row, 1 );

   connect( dkdb_cntrls, SIGNAL( changed( bool ) ),
            this,    SLOT( update_disk_db( bool ) ) );

   connect( pb_loadda, SIGNAL( clicked()    ),
            this,      SLOT(   load()       ) );
   connect( pb_saveda, SIGNAL( clicked()    ),
            this,      SLOT(   save()       ) );
   connect( pb_resetd, SIGNAL( clicked()    ),
            this,      SLOT(   reset_data() ) );
   connect( pb_resetp, SIGNAL( clicked()    ),
            this,      SLOT(   reset_plot() ) );
   connect( pb_help,   SIGNAL( clicked()    ),
            this,      SLOT(   help()       ) );
   connect( pb_close,  SIGNAL( clicked()    ),
            this,      SLOT(   close()      ) );

   connect( ck_distrib,  SIGNAL( stateChanged( int) ),
            this,        SLOT(   plot_data()        ) );
   connect( ck_envelope, SIGNAL( stateChanged( int) ),
            this,        SLOT(   plot_data()        ) );

   connect( lw_runids,   SIGNAL( currentRowChanged( int ) ),
            this,        SLOT(   runid_select(      int ) ) );
   connect( lw_triples,  SIGNAL( currentRowChanged( int ) ),
            this,        SLOT(   triple_select(     int ) ) );

   QBoxLayout* plot = new US_Plot( data_plot1,
         tr( "G(s) Distributions" ),
         tr( "Sedimentation Coefficient x 1e+13 (corr. for 20,W)" ),
         tr( "Boundary Fraction (%)" ) );

   QwtText qtitle( tr( "Relative Frequency" ) );
   qtitle.setFont( QFont( US_GuiSettings::fontFamily(),
                          US_GuiSettings::fontSize(), QFont::Bold ) );
   qtitle.setText( tr( "Relative Frequency" ) );

   data_plot1->setMinimumSize( 640, 400 );
   data_plot1->enableAxis  ( QwtPlot::yRight, true );
   data_plot1->setAxisScale( QwtPlot::xBottom, 1.0,  10.0 );
   data_plot1->setAxisScale( QwtPlot::yLeft,   0.0, 100.0 );
   data_plot1->setAxisScale( QwtPlot::yRight,  0.0,   6.0 );
   data_plot1->setAxisTitle( QwtPlot::yRight, qtitle );
   QwtPlotGrid* grid = us_grid( data_plot1 );
   grid->enableXMin( true );
   grid->enableYMin( true );
   grid->setMajPen( QPen( US_GuiSettings::plotMajGrid(), 0, Qt::DashLine ) );
   grid->setMinPen( QPen( US_GuiSettings::plotMinGrid(), 0, Qt::DotLine  ) );

   QwtLegend *legend = new QwtLegend;
   legend->setFrameStyle( QFrame::Box | QFrame::Sunken );
   data_plot1->insertLegend( legend, QwtPlot::BottomLegend  ); 

   rightLayout->addLayout( plot );

   mainLayout ->addLayout( leftLayout     );
   mainLayout ->addLayout( rightLayout    );
   mainLayout ->setStretchFactor( leftLayout,  2 );
   mainLayout ->setStretchFactor( rightLayout, 5 );

   le_runid   ->setText( "(current run ID)" );
   cmb_svproj ->addItem( "(project name for plot save)" );

   adjustSize();
   int hh  = lb_svproj->height();
   int ww  = lb_svproj->width() / 3;
   lw_runids  ->setMinimumHeight( hh * 3 );
   lw_triples ->setMinimumHeight( hh * 5 );
   cmb_svproj ->setMinimumWidth ( ww * 5 );
   for ( int ii = 0; ii < 8; ii++ )
      leftLayout ->setColumnMinimumWidth( ii, ww );
   leftLayout ->setColumnStretch     ( 0, 1  );
   leftLayout ->setColumnStretch     ( 1, 1  );
   adjustSize();
}

// Load data
void US_vHW_Combine::load( void )
{
   QStringList runids;
   QString     runid;

   // Open a dialog and get the runID(s)
   US_SelectRunid srdiag( dkdb_cntrls->db(), runids );
   connect( &srdiag,      SIGNAL( changed( bool ) ),
            this,    SLOT( update_disk_db( bool ) ) );
   srdiag.exec();

   int nruns    = runids.size();
   if ( nruns < 1 )   return;
DbgLv(1) << "Selected runIDs[0]" << runids[0] << "count" << nruns;

   if ( dkdb_cntrls->db() )
   {  // Plot reports are in the database
      US_Passwd    pw;
      US_DB2       db( pw.getPasswd() );

      QString      tmpdir = US_Settings::tmpDir();
      US_Report    freport;

      for ( int ii = 0; ii < nruns; ii++ )
      {
         runid          = runids[ ii ];
         freport.readDB( runid, &db );
         int ntripl     = freport.triples.count();
         int distx      = distros.size();
DbgLv(1) << " ii,runid,ntrip,dists" << ii << runid << ntripl << distx;

         for ( int jj = 0; jj < ntripl; jj++ )
         {
            US_Report::ReportTriple* tripl = &freport.triples[ jj ];
            QString trname = collapsedTriple( tripl->triple ); 
            int ndocs      = tripl->docs.count();
DbgLv(1) << "   jj,ndocs" << jj << ndocs;
            bool havedis   = false;
            bool haveenv   = false;
            US_Report::ReportDocument* ddoc = NULL;
            US_Report::ReportDocument* edoc = NULL;
            QString dpath;
            QString epath;

            for ( int kk = 0; kk < ndocs; kk++ )
            {
               US_Report::ReportDocument* doc = &tripl->docs[ kk ];
               QString fname  = doc->filename;
DbgLv(1) << "     kk,fname" << kk << fname;

               if ( fname.contains( "s-c-distrib.dat" ) )
               {
                  ddoc     = doc;
                  dpath    = tmpdir + "/" + runid + "." + fname;
                  int stat = db.readBlobFromDB( dpath,
                     QString( "download_reportContents" ), ddoc->documentID );
DbgLv(1) << "      readBlob stat" << stat;
DbgLv(1) << "       dpath" << dpath;

                  if ( stat != US_DB2::OK )
                  {
                     qDebug() << "*ERROR* download_reportContents" << stat;
                     qDebug() << "        DPATH" << dpath;
                     havedis  = false;
                  }
                  else
                     havedis  = true;
               }

               else if ( fname.contains( "s-c-envelope.dat" ) )
               {
                  edoc     = doc;
                  epath    = tmpdir + "/" + runid + "." + fname;
DbgLv(1) << "       epath" << epath;
                  int stat = db.readBlobFromDB( epath,
                     QString( "download_reportContents" ), edoc->documentID );
DbgLv(1) << "      readBlob stat" << stat;

                  if ( stat != US_DB2::OK )
                  {
                     qDebug() << "*ERROR* download_reportContents" << stat;
                     qDebug() << "        EPATH" << epath;
                     haveenv  = false;
                  }
                  else
                     haveenv  = true;
               }
            }  // END: documents loop

            if ( havedis )
            {
               QString fname = dpath.section( "/", -1, -1 );
               DistrDesc ddesc;
               ddesc.runID   = runid;
               ddesc.triple  = trname;
DbgLv(1) << " havedis run trip" << runid << tripl << "haveenv" << haveenv;
DbgLv(1) << "  dpath" << dpath;
DbgLv(1) << "  epath" << epath;
               QFile fid( dpath );

               if ( fid.open( QIODevice::ReadOnly | QIODevice::Text ) )
               {
                  QTextStream tsd( &fid );
                  QTextStream tse;
                  QFile fie( epath );
                  if ( haveenv  &&
                       fie.open( QIODevice::ReadOnly | QIODevice::Text ) )
                     tse.setDevice( &fie );
                  else
                     haveenv = false;

                  fill_in_desc( tsd, tse, ddesc, haveenv, distx );


                  distros << ddesc;
                  distIDs << runid + "." + trname;
                  distx++;
                  fid.close();
                  if ( haveenv )
                     fie.close();
               }
            }
         } // END: triples loop
      } // END: runids loop
   }

   else
   {  // Plot reports are on local disk

      QString resdir = US_Settings::resultDir() + "/";

      for ( int ii = 0; ii < nruns; ii++ )
      {
         runid          = runids[ ii ];
         QString rundir = resdir + runid + "/";

         QStringList datfilt( "vHW.*s-c-distrib.dat" );
         QStringList dfiles = QDir( rundir )
            .entryList( datfilt, QDir::Files, QDir::Name );

         int distx   = distros.size();
         int  ndfile = dfiles.count();

         if ( ndfile == 0 )
         {
            QMessageBox::information( this, tr( "No Distribution Plots" ),
                tr( "The selected Run ID Directory\n  %1\n"
                    "contained no vHW distribution plots." )
                .arg( rundir ) );
         }

         for ( int ii = 0; ii < ndfile; ii++ )
         {
            QString fname = dfiles[ ii ];
            QString fpath = rundir + "/" + fname;
            QString epath = QString( fpath ).replace( "distrib.dat",
                                                      "envelope.dat" );
            QString tripl = fname.section( ".", 1, 1 );
            DistrDesc ddesc;
            ddesc.runID   = runid;
            ddesc.triple  = tripl;
            QFile fid( fpath );

            if ( fid.open( QIODevice::ReadOnly | QIODevice::Text ) )
            {
               QTextStream tsd( &fid );
               QTextStream tse;
               QFile fie( epath );
               bool haveenv = fie.open( QIODevice::ReadOnly | QIODevice::Text );
               if ( haveenv )
                  tse.setDevice( &fie );

               fill_in_desc( tsd, tse, ddesc, haveenv, distx );


               distros << ddesc;
               distIDs << runid + "." + tripl;
               distx++;
               fid.close();
               if ( haveenv )
                  fie.close();
            }
         }
      }
   }

   int nrunids = runids.count();
   int nsprojs = cmb_svproj->count();

   if ( nsprojs == 1 )
      cmb_svproj->clear();
   else
      cmb_svproj->removeItem( nsprojs - 1 );

   for ( int ii = 0; ii < nrunids; ii++ )
   {
      lw_runids->addItem( runids[ ii ] );
      cmb_svproj->addItem( runids[ ii ] );
   }

   cmb_svproj->addItem( "All" );

   int nlitems = lw_runids->count();
   le_runid->setText( runids[ 0 ] );

   if ( nrunids == nlitems )
   {
      adjustSize();
   }

   pb_resetd->setEnabled( true );
}

// Reset data: remove all loaded data and clear plots
void US_vHW_Combine::reset_data( void )
{
   distros.clear();
   distIDs.clear();

   lw_runids  ->clear();
   lw_triples ->clear();
   le_runid   ->clear();
   cmb_svproj ->clear();

   reset_plot();

   pb_resetd->setEnabled( false );
   pb_resetp->setEnabled( false );
}

// Reset plot:  Clear plots and lists of plotted data
void US_vHW_Combine::reset_plot( void )
{
   data_plot1->detachItems();
   data_plot1->clear();
   data_plot1->replot();

   pdistrs.clear();
   pdisIDs.clear();
   pb_saveda->setEnabled( false );

   lw_triples ->setCurrentRow( -1 );
}

// Plot all data
void US_vHW_Combine::plot_data( void )
{
   data_plot1->detachItems();
   QwtPlotGrid* grid = us_grid( data_plot1 );
   grid->enableXMin( true );
   grid->enableYMin( true );
   grid->setMajPen(
         QPen( US_GuiSettings::plotMajGrid(), 0, Qt::DashLine ) );
   grid->setMinPen(
         QPen( US_GuiSettings::plotMinGrid(), 0, Qt::DotLine  ) );

   for ( int ii = 0; ii < pdistrs.size(); ii++ )
      plot_distr( pdistrs[ ii ], pdisIDs[ ii ] );
}

// Add a single distribution to the plot
void US_vHW_Combine::plot_distr( DistrDesc ddesc, QString distrID )
{
   bool dplot  = ck_distrib ->isChecked();
   bool eplot  = ck_envelope->isChecked();
   int  ndispt = ddesc.bfracs.size();
   int  nenvpt = ddesc.efreqs.size();
   double* xx  = ddesc.dsedcs.data();
   double* yy  = ddesc.bfracs.data();
   double* xs  = ddesc.esedcs.data();
   double* yf  = ddesc.efreqs.data();

   QString dcID = distrID + tr( " (integ.)" ); 
   QString lcID = distrID + tr( " (integ.line)" );
   QString ecID = distrID + tr( " (diff.)" );
   QwtPlotCurve* dcurve;
   QwtPlotCurve* lcurve;
   QwtPlotCurve* ecurve;
   QPen          dlpen( QPen( Qt::yellow ) );
   QPen          elpen( QPen( QBrush( ddesc.color ), 3.0 ) );

   if ( dplot )
   {  // Build curves for distribution plot
      dcurve        = us_curve( data_plot1, dcID );
      dcurve->setStyle ( QwtPlotCurve::NoCurve );
      dcurve->setSymbol( ddesc.symbol );
      dcurve->setData  ( xx, yy, ndispt );
      dcurve->setYAxis ( QwtPlot::yLeft );

      lcurve        = us_curve( data_plot1, lcID );
      lcurve->setStyle ( QwtPlotCurve::Lines );
      lcurve->setPen   ( dlpen );
      lcurve->setData  ( xx, yy, ndispt );
      lcurve->setYAxis ( QwtPlot::yLeft );
      lcurve->setItemAttribute( QwtPlotItem::Legend, false );
   }

   if ( eplot )
   {  // Build curve for envelope plot
int kk=ddesc.efreqs.size();
DbgLv(2) << "   xs0 yf0" << xs[0] << yf[0];
DbgLv(2) << "   xs1 yf1" << xs[1] << yf[1];
DbgLv(2) << "   xsm yfm" << xs[kk-2] << yf[kk-2];
DbgLv(2) << "   xsn yfn" << xs[kk-1] << yf[kk-1];
      ecurve        = us_curve( data_plot1, ecID );
      ecurve->setStyle ( QwtPlotCurve::Lines );
      ecurve->setPen   ( elpen );
      ecurve->setData  ( xs, yf, nenvpt );
      ecurve->setYAxis ( QwtPlot::yRight );
   }

   data_plot1->setAxisAutoScale( QwtPlot::xBottom );
   data_plot1->setAxisAutoScale( QwtPlot::yLeft );
   data_plot1->setAxisAutoScale( QwtPlot::yRight );
   data_plot1->enableAxis      ( QwtPlot::yLeft,  dplot );
   data_plot1->enableAxis      ( QwtPlot::yRight, eplot );

   if ( pdistrs.size() < 2 )
   {  // Set up grid if first distribution in plot
      QwtPlotGrid* grid = us_grid( data_plot1 );
      grid->enableXMin( true );
      grid->enableYMin( true );
      grid->setMajPen(
            QPen( US_GuiSettings::plotMajGrid(), 0, Qt::DashLine ) );
      grid->setMinPen(
            QPen( US_GuiSettings::plotMinGrid(), 0, Qt::DotLine  ) );
   }

   data_plot1->replot();
}

// Save the plot data
void US_vHW_Combine::save( void )
{
   QString oproj    = cmb_svproj->currentText();
   QString runID    = ( oproj == "All" ) ? pdistrs[ 0 ].runID : oproj;
   QString trname   = pdistrs[ 0 ].triple;
   QString fdir     = US_Settings::reportDir() + "/" + runID;
   QString fnamsvg  = "vHW.0Z9999.combo-distrib.svg";
   QString fnampng  = "vHW.0Z9999.combo-distrib.png";
   QString fnamdat  = "vHW.0Z9999.combo-sb-distrib.dat";
   QString fnamlst  = "vHW.0Z9999.combo-list-include.rpt";
   QString plotFile = fdir + "/" + fnamsvg;
   QString dataFile = fdir + "/" + fnamdat;
   QString listFile = fdir + "/" + fnamlst;
   QStringList prunids;
   QList< int > prndxs;
   QString svmsg   = tr( "Saved:\n    " ) + fnampng + "\n    "
                                          + fnamsvg + "\n    "
                                          + fnamdat + "\n    "
                                          + fnamlst + "\n";
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   // Look for multiple run IDs
   for ( int ii = 0; ii < pdistrs.size(); ii++ )
   {
      QString prun     = pdistrs[ ii ].runID;
      if ( oproj == "All" )
      {  // If save-plot project is "All", save lists of runids and indexes
         if ( ! prunids.contains( prun ) )
         {
            prunids << prun;
            prndxs  << ii;
         }
      }

      else if ( prun == runID )
      {  // If save-plot project matches current run, save it and its index
         prunids << prun;
         prndxs  << ii;
         break;
      }
   }

   int     iruns   = 0;
   int     nruns   = prunids.size();
   int     jdist   = 0;

   while( iruns < nruns )
   {
      if ( ! QFile( fdir ).exists() )
      {  // If need be, create runID directory
         QDir().mkpath( fdir );
      }

      // Save plot file as SVG and as PNG; write data and list files
      write_plot( plotFile, data_plot1 );
      write_data( dataFile, listFile, iruns );
      svmsg += tr( "in directory:" ) + "\n    " + fdir + "\n";

      if ( dkdb_cntrls->db() )
      {
         US_Passwd    pw;
         US_DB2       db( pw.getPasswd() );
         int          idEdit = 0;

         QString trfirst  = pdistrs[ 0 ].triple;
         QString trlast   = pdistrs[ pdistrs.size() - 1 ].triple;
         QString trdesc   = "Combined Analyses (" + trfirst
            + "..." + trlast + ")";

         QString editID;         // Edit ID for matching experiment,triple
         QString eeditID;        // First edit ID from experiment match
         // Get test triple to match file part and investigator
         QString trip1 = US_Util::expanded_triple( trfirst, false )
                                  .replace( '/', '.' );
         QString invID = QString::number( US_Settings::us_inv_ID() );
         // Query for the experiment ID matching the run ID
         QStringList  query;
         query << "get_experiment_info_by_runID" << runID << invID;
         db.query( query );
         db.next();
         QString expID = db.value( 1 ).toString();
DbgLv(1) << "SV:  runID expID" << runID << expID;
         // Query for the raw ID in experiment matching a triple
         QString rawID;
         query.clear();
         query << "get_rawDataIDs" << expID;
         db.query( query );
         while ( db.next() )
         {
                    rawID  = db.value( 0 ).toString();
            QString efname = db.value( 2 ).toString();
DbgLv(1) << "SV:   rawID" << rawID << "efname" << efname << "trip1" << trip1;
            // Save rawID when we have found a triple match
            if ( efname.contains( trip1 ) )
               break;
         }
         // Query edit IDs for raw ID and look for triple match
         query.clear();
         query << "get_editedDataIDs" << rawID;
         db.query( query );
         while ( db.next() )
         {
            QString aeditID = db.value( 0 ).toString();
            QString efname  = db.value( 2 ).toString();
            if ( eeditID.isEmpty() )
               eeditID      = aeditID;  // Save 1st valid from experiment
DbgLv(1) << "SV:     editID" << eeditID << "raw exp trip1 fname"
 << rawID << expID << trip1 << efname;
            if ( efname.contains( trip1 ) )
            {  // Keep saving editID from last triple match
               editID       = aeditID;
DbgLv(1) << "SV:        Name-Trip MATCH:  editID" << editID;
            }
         }

         if ( ! editID.isEmpty() )
         {  // Use edit ID from last matching triple
            idEdit              = editID.toInt();
         }
         else
         {  // Or fall back to one from first valid edit in experiment
            idEdit              = eeditID.toInt();
         }
DbgLv(1) << "SV: editID idEdit" << editID << idEdit << "  eeditID" << eeditID;

         // Add or update report documents in the database
         QStringList rfiles;
         rfiles << fnamsvg << fnampng << fnamdat << fnamlst;
         int st = reportDocsFromFiles( runID, fdir, rfiles, &db,
                                       idEdit, trdesc );

DbgLv(1) << "SV:runID" << runID << "idEdit" << idEdit
 << "fnamlst" << fnamlst << "trdesc" << trdesc;
         if ( iruns == ( nruns - 1 ) )
         {  // Append message line after last run save
            if ( st == 0 )
               svmsg += tr( "\nThe files were also saved to the database" );
            else
               svmsg += tr( "\n*ERROR* in saving files to the database" );
         }
      }  // END:  database

      if ( ++iruns >= nruns )  break;

      runID         = prunids[ iruns ];
      jdist         = prndxs [ iruns ];
      trname        = pdistrs[ jdist ].triple;
      fdir          = US_Settings::reportDir() + "/" + runID;
      plotFile      = fdir + "/" + fnamsvg;
      dataFile      = fdir + "/" + fnamdat;
      listFile      = fdir + "/" + fnamlst;
   }  // END:  runs loop

   QApplication::restoreOverrideCursor();

   // Report saved files
   QMessageBox::information( this, tr( "Combo Distro Plot File Save" ), svmsg );
}

// RunID selected
void US_vHW_Combine::runid_select( int row )
{
DbgLv(1) << "RunIDSel:row" << row;
   if ( row < 0 )  return;
   QListWidgetItem* item = lw_runids->item( row );
   runID    = item->text();
DbgLv(1) << "RunIDSel:runID" << runID << "distrsize" << distros.size();
   le_runid ->setText( runID );

   lw_triples->clear();

   for ( int ii = 0; ii < distros.size(); ii++ )
   {
DbgLv(1) << "RunIDSel:  ii runID" << ii << distros[ii].runID;
      if ( distros[ ii ].runID == runID )
      {
         lw_triples->addItem( expandedTriple(
            distros[ ii ].triple ) + " : " + distros[ ii ].tdescr );
      }
   }

   if ( pdistrs.size() == 0 )
   {
      cmb_svproj->setCurrentIndex( cmb_svproj->findText( runID ) );
   }
}

// Triple selected
void US_vHW_Combine::triple_select( int row )
{
DbgLv(1) << "TripleSel:row" << row;
   if ( row < 0 )  return;
   QListWidgetItem* item = lw_triples->item( row );
   triple   = collapsedTriple( item->text().section( ":", 0, 0 ).simplified() );
DbgLv(1) << "TripleSel:triple" << triple;

   DistrDesc ddesc;

   for ( int ii = 0; ii < distros.size(); ii++ )
   {
      ddesc = distros[ ii ];

      if ( ddesc.runID == runID  &&  ddesc.triple == triple )
         break;
   }

   QString distrID = ddesc.runID + "." + ddesc.triple + " : " + ddesc.tdescr;

   if ( ! pdisIDs.contains( distrID ) )
   {
      pdistrs << ddesc;
      pdisIDs << distrID;

      plot_distr( ddesc, distrID );
   }

   else
      plot_data();

   pb_saveda->setEnabled( true );
   pb_resetd->setEnabled( true );
   pb_resetp->setEnabled( true );
}

// Assign symbol and color for a distribution
void US_vHW_Combine::setSymbol( DistrDesc& ddesc, int distx )
{
   QwtSymbol dsymbol;

   possibleSymbols();             // Make sure possible symbols,colors exist

   int nsymbs   = symbols.size();
   int ncolors  = colors.size();
   int js       = distx;
   int jc       = distx;

   while ( js >= nsymbs )
      js -= nsymbs;

   while ( jc >= ncolors )
      jc -= ncolors;

   ddesc.color  = colors [ jc ];
   dsymbol.setPen( QColor( Qt::white ) );
   dsymbol.setSize( 8 );
   dsymbol.setStyle( (QwtSymbol::Style)symbols[ js ] );
   dsymbol.setBrush( ddesc.color );
   ddesc.symbol = dsymbol;

   return;
}

// Generate lists of symbol styles and colors if need be
void US_vHW_Combine::possibleSymbols()
{
   if ( symbols.size() > 0  &&  colors.size() > 0 )
      return;

   symbols.clear();
   colors .clear();

   symbols << (int)( QwtSymbol::Ellipse   );
   symbols << (int)( QwtSymbol::Rect      );
   symbols << (int)( QwtSymbol::Triangle  );
   symbols << (int)( QwtSymbol::Diamond   );
   symbols << (int)( QwtSymbol::LTriangle );
   symbols << (int)( QwtSymbol::Hexagon   );
   symbols << (int)( QwtSymbol::Star2     );
   symbols << (int)( QwtSymbol::DTriangle );

   colors  << QColor( 255,   0,   0 );
   colors  << QColor(   0, 255,   0 );
   colors  << QColor(   0,   0, 255 );
   colors  << QColor( 255, 255,   0 );
   colors  << QColor( 255,   0, 255 );
   colors  << QColor(   0, 255, 255 );
   colors  << QColor( 122,   0, 255 );
   colors  << QColor(   0, 255, 122 );
   colors  << QColor(   0, 122, 255 );
   colors  << QColor( 255, 122,   0 );
   colors  << QColor( 122, 255,   0 );
   colors  << QColor(  80,   0, 255 );
   colors  << QColor( 255,   0,  80 );
   colors  << QColor(  80,   0, 255 );
   colors  << QColor( 255,   0,  80 );
   colors  << QColor(   0, 255,  80 );
   colors  << QColor(   0,  80, 255 );
   colors  << QColor(  80, 255,   0 );
   colors  << QColor( 255,  80,  40 );
   colors  << QColor(  40, 255,  40 );
   colors  << QColor(  40,  40, 255 );
}

// Return an expanded version ("1 / A / 260") of a triple string ("1A260")
QString US_vHW_Combine::expandedTriple( QString ctriple )
{
   QString etriple = ctriple;

   if ( ! etriple.contains( " / " ) )
      etriple = etriple.left( 1 ) + " / " +
                etriple.mid( 1, 1 ) + " / " +
                etriple.mid( 2 );

   return etriple;
}

// Return a collapsed version ("1A260") of a triple string ("1 / A / 260")
QString US_vHW_Combine::collapsedTriple( QString etriple )
{
   QString ctriple = etriple;

   if ( ctriple.contains( " / " ) )
      ctriple = ctriple.replace( " / ", "" ).simplified();
   else if ( ctriple.contains( "/" ) )
      ctriple = ctriple.replace( "/",   "" ).simplified();
   else if ( ctriple.contains( "." ) )
      ctriple = ctriple.replace( ".",   "" ).simplified();

   return ctriple;
}

// Generate envelope data
int US_vHW_Combine::envel_data( DistrDesc& ddesc )
{
   int     nSensit  = 50;
   int     nSmooth  = 30;
   int     steps;
   int     nepts    = 300;
   int     ndpts    = ddesc.dsedcs.size();
   double  max_cept = 1.0e-6;
   double  min_cept = 1.0e+6;
   double  sed_bin  = ddesc.dsedcs.at( 0 );
   double  his_sum  = 0.0;
   double  env_sum  = 0.0;
   double  div_scl  = (double)nSensit * (double)ndpts * 0.01;
   double  max_step;
   double  sigma;
   double  sed_lo;
   double  sed_hi;
   double  sedc;

   for ( int jj = 0; jj < ndpts; jj++ )
   {  // get min,max intercept sedimentation coefficients
      min_cept   = qMin( min_cept, ddesc.dsedcs.at( jj ) );
      max_cept   = qMax( max_cept, ddesc.dsedcs.at( jj ) );
   }

   // calculate values based on range and sensitivity
   sed_bin      = ( max_cept - min_cept ) / div_scl;
   max_step     = max_cept * 4.0 / 3.0;
   steps        = (int)( max_step / sed_bin );

   if ( nepts <= steps )
   {  // insure envelope array size bigger than histogram array size
      nepts        = steps + 1;
   }

   ddesc.esedcs.fill( 0.0, nepts );
   ddesc.efreqs.fill( 0.0, nepts );
   double  bink   = 0.0;
   double  sval   = 0.0;
   double  pisqr  = sqrt( M_PI * 2.0 );
   double* xval   = ddesc.esedcs.data();
   double* yval   = ddesc.efreqs.data();
   double  scale  = max_step / (double)nepts;

   for ( int jj = 0; jj < nepts; jj++ )
   {  // initialize envelope values
      xval[ jj ]   = scale * (double)jj;
      yval[ jj ]   = 0.0;
   }

   sigma        = sed_bin * 0.02 * (double)nSmooth;

   for ( int jj = 0; jj < steps; jj++ )
   {  // calculate histogram values and envelope values based on them
      int kbin     = 0;
      sed_lo       = sed_bin * (double)jj;
      sed_hi       = sed_lo + sed_bin;
      sval         = ( sed_lo + sed_hi ) * 0.5;

      for ( int kk = 0; kk < ndpts; kk++ )
      {  // count sedcoeffs within current step range
         sedc         = ddesc.dsedcs.at( kk );

         if ( sedc >= sed_lo  &&  sedc < sed_hi )
            kbin++;
      }

      bink         = (double)kbin;
      his_sum     += ( bink * sed_bin );  // bump histogram sum

      if ( kbin > 0 )
      {  // if non-empty bin, update envelope Y values
         for ( int kk = 0; kk < nepts; kk++ )
         {
            double xdif  = ( xval[ kk ] - sval ) / sigma;
            yval[ kk ]  += ( ( bink / ( sigma * pisqr ) )
               * exp( -( xdif * xdif ) / 2.0 ) );
         }
      }
   }

   for ( int kk = 0; kk < nepts; kk++ )
   {  // accumulate the envelope values sum
      env_sum     += yval[ kk ];
   }

   env_sum     *= xval[ 1 ];            // sum times X increment
   scale        = his_sum / env_sum;    // normalizing scale factor
DbgLv(2) << "ED: hsum esum scale " << his_sum << env_sum << scale;

   int fnz      = -1;
   int lnz      = 0;

   for ( int kk = 0; kk < nepts; kk++ )
   {  // normalize Y values
      yval[ kk ]  *= scale;

      if ( yval[ kk ] != 0.0 )
      {
         if ( fnz < 0 )  fnz = kk;
         lnz = kk;
      }
   }

   fnz    = qMax( ( fnz - 2 ), 0 );
   lnz    = qMin( ( lnz + 2 ), nepts );
   nepts  = 0;

   for ( int ii = fnz; ii < lnz; ii++ )
   {  // Reduce array to Y's from first to last non-zero
      xval[ nepts ] = xval[ ii ];
      yval[ nepts ] = yval[ ii ];
      nepts++;
   }

   ddesc.esedcs.resize( nepts );
   ddesc.efreqs.resize( nepts );

   return nepts;                        // return arrays' size
}

// Reset Disk_DB control whenever data source is changed in any dialog
void US_vHW_Combine::update_disk_db( bool isDB )
{
   isDB ? dkdb_cntrls->set_db() : dkdb_cntrls->set_disk();
DbgLv(1) << "Upd_Dk_Db isDB" << isDB;

   reset_data();
}

void US_vHW_Combine::fill_in_desc( QTextStream& tsd, QTextStream& tse,
      DistrDesc& ddesc, bool haveenv, int distx )
{
   // Read distribution stream to set sed and bound values
   QString fline = tsd.readLine().simplified();
DbgLv(1) << "FID: fline0" << fline;

   if ( fline.contains( "%Boundary" ) )
      ddesc.tdescr  = tr( "(Unknown description)" );
   else
   {
      ddesc.tdescr  = fline;
      fline = tsd.readLine();
   }

   while ( !tsd.atEnd() )
   {
      fline = tsd.readLine().simplified();
      double bound = fline.section( " ", 0, 0 ).toDouble();
      double sedc  = fline.section( " ", 3, 3 ).toDouble();

      ddesc.dsedcs << sedc;
      ddesc.bfracs << bound;
   }
int kk = ddesc.dsedcs.size()-1;
DbgLv(1) << "Distro runid" << ddesc.runID << " triple" << ddesc.triple << kk;
DbgLv(1) << "  0 sed frac" << ddesc.dsedcs[0] << ddesc.bfracs[0];
DbgLv(1) << "  kk sed frac" << ddesc.dsedcs[kk] << ddesc.bfracs[kk];
   setSymbol( ddesc, distx );

   if ( haveenv )
   {  // Read in envelope data
      fline = tse.readLine();
      int     fnz   = -1;
      int     lnz   = 0;

      while ( !tse.atEnd() )
      {
         fline = tse.readLine().simplified();
         double sedc  = fline.section( " ", 0, 0 ).toDouble();
         double freq  = fline.section( " ", 1, 1 ).toDouble();

         if ( freq != 0.0 )
         {
            lnz      = ddesc.esedcs.size();
            if ( fnz < 0 ) fnz = lnz;
         }

         ddesc.esedcs << sedc;
         ddesc.efreqs << freq;
      }

      int nepts = ddesc.esedcs.size();
      fnz       = qMax( ( fnz - 2 ), 0 );
      lnz       = qMin( ( lnz + 2 ), nepts );
      nepts     = 0;

      for ( int jj = fnz; jj < lnz; jj++ )
      {  // Reduce array to Y's from first to last non-zero
         ddesc.esedcs[ nepts ] = ddesc.esedcs[ jj ];
         ddesc.efreqs[ nepts ] = ddesc.efreqs[ jj ];
         nepts++;
      }

      if ( nepts < ddesc.esedcs.size() )
      {
         ddesc.esedcs.resize( nepts );
         ddesc.efreqs.resize( nepts );
      }
DbgLv(1) << " Envel nepts" << nepts << ddesc.esedcs.size();
int kk = ddesc.esedcs.size()-1;
DbgLv(1) << "  0 sed frac" << ddesc.esedcs[0] << ddesc.efreqs[0];
DbgLv(1) << "  kk sed frac" << ddesc.esedcs[kk] << ddesc.efreqs[kk];
   }

   else
   {  // Calculate envelope data from distribution data
      envel_data( ddesc );
   }
}

// Write data and list report files
void US_vHW_Combine::write_data( QString& dataFile, QString& listFile,
      int& irun )
{
   if ( irun > 0 )
   {  // After first/only time:  just make a copy of the files
      QFile( dat1File ).copy( dataFile );
      QFile( lis1File ).copy( listFile );
      return;
   }

   // First/only time through:  compute the data and create files
   QStringList pdlong;
   QString line;
   dat1File = dataFile;
   lis1File = listFile;

   QFile dfile( dataFile );

   if ( ! dfile.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      qDebug() << "***Error opening output file" << dataFile;
      return;
   }

   QTextStream tsd( &dfile );

   int nplots = pdistrs.size();
   int maxnvl = 0;
   line       = "";
      
   for ( int ii = 0; ii < nplots; ii++ )
   {  // Accumulate long descriptions and build header line
      maxnvl     = qMax( maxnvl, pdistrs[ ii ].dsedcs.size() );
      QString pd = pdisIDs[ ii ];
      pdlong << pd;
      pd         = pd.section( ":", 0, 0 ).simplified();
      line      += pd + ".X " + pd + ".Y"; // X,Y header entries for contributor
      if ( ii < ( nplots - 1 ) )
         line     += "  ";
      else
         line     += "\n";
   }
   tsd << line;                             // Write header line

   for ( int jj = 0; jj < maxnvl; jj++ )
   {  // Build and write svalue+boundary data line
      line       = "";
      for ( int ii = 0; ii < nplots; ii++ )
      {  // Add each X,Y data pair
         int nvals   = pdistrs[ ii ].dsedcs.size();
         double* xx  = pdistrs[ ii ].dsedcs.data();
         double* yy  = pdistrs[ ii ].bfracs.data();
         int kk      = qMin( jj, ( nvals - 1 ) );
         double sval = xx[ kk ];
         double boun = yy[ kk ];

         line       += QString().sprintf( "%12.5f %10.5f", sval, boun );
      }
      line       += "\n";
      tsd << line;                           // Write data line
   }

   dfile.close();

   // Write list-of-included file
   QFile lfile( listFile );
   if ( ! lfile.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      qDebug() << "***Error opening output file" << listFile;
      return;
   }
   QTextStream tsl( &lfile );

   for ( int ii = 0; ii < nplots; ii++ )
   {  // Build and write each long-description line
      line       = pdlong[ ii ] + "\n";
      tsl << line;
   }

   lfile.close();

   return;
}

// Save report documents from files
int US_vHW_Combine::reportDocsFromFiles( QString& runID, QString& fdir,
   QStringList& files, US_DB2* db, int& idEdit, QString& trdesc )
{
   int ostat = 0;
   US_Report    freport;
   freport.runID          = runID;

   for ( int ii = 0; ii < files.size(); ii++ )
   {
      QString fname = files[ ii ];
      int st = freport.saveDocumentFromFile( fdir, fname, db, idEdit, trdesc );

      ostat = ( st == US_Report::REPORT_OK ) ? ostat : st;
   }

//*DEBUG*
   if ( dbg_level > 0 )
   {
      int status = freport.readDB( runID, db );
      DbgLv(1) << "DFF:report readDB status" << status << "ID" << freport.ID;
      DbgLv(1) << "DFF:  report triples size" << freport.triples.size();
      for ( int ii = 0; ii < freport.triples.size(); ii++ )
      {
         int ndoc = freport.triples[ii].docs.size();
         DbgLv(1) << "DFF:  triple" << ii << "docssize" << ndoc
            << "ID" << freport.triples[ii].tripleID
            << "triple" << freport.triples[ii].triple;
         int jj   = ndoc - 1;
         if ( ndoc > 0 )
         {
            DbgLv(1) << "DFF:    doc" << 0
               << "ID" << freport.triples[ii].docs[0].documentID
               << "label" << freport.triples[ii].docs[0].label;
            DbgLv(1) << "DFF:    doc" << jj
               << "ID" << freport.triples[ii].docs[jj].documentID
               << "label" << freport.triples[ii].docs[jj].label;
         }
      }
      QString fname = files[0];
      QString tripl( "0/Z/9999" );
      int ndx = freport.findTriple( tripl );
      DbgLv(1) << "DFF:triple" << tripl << "ndx" << ndx;
      if ( ndx >= 0 )
      {
         int ndoc = freport.triples[ndx].docs.size();
         DbgLv(1) << "DFF:  triple" << ndx << "docs size" << ndoc
            << "ID" << freport.triples[ndx].tripleID
            << "triple" << freport.triples[ndx].triple;
         if ( ndoc > 0 )
         {
            DbgLv(1) << "DFF:    doc" << 0
               << "ID" << freport.triples[ndx].docs[0].documentID
               << "label" << freport.triples[ndx].docs[0].label;
            int jj   = ndoc - 1;
            DbgLv(1) << "DFF:    doc" << jj
               << "ID" << freport.triples[ndx].docs[jj].documentID
               << "label" << freport.triples[ndx].docs[jj].label;
         }
      }
   }
//*DEBUG*

   return ostat;
}

