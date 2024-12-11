//! \file us_mwl_species_fit.cpp

#include <QApplication>

#include "us_mwl_species_fit.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_util.h"
#include "us_data_loader.h"
#include "us_load_run_noise.h"
#include "us_passwd.h"
#include "us_images.h"
#include "us_csv_loader.h"

#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#define setMinimum(a)      setMinValue(a)
#define setMaximum(a)      setMaxValue(a)
#define setSymbol(a)       setSymbol(*a)
#define AXISSCALEDIV(a)    data_plot->axisScaleDiv(a)
#define dPlotClearAll(a) a->clear()
#else
#include "qwt_picker_machine.h"
#define AXISSCALEDIV(a)    (QwtScaleDiv*)&data_plot->axisScaleDiv(a)
#define dPlotClearAll(a) a->detachItems(QwtPlotItem::Rtti_PlotItem,true)
#endif

//! \brief Main program. Loads translators and starts
//         the class US_Convert.
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_MwlSpeciesFit w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

US_MwlSpeciesFit::US_MwlSpeciesFit() : US_AnalysisBase2()
{
   setWindowTitle( tr( "MWL Species Fit Analysis" ) );

   dbg_level   = US_Settings::us_debug();
   te_results  = NULL;
   nspecies    = 0;
   jspec       = 0;
   synData.clear();
   have_p1.clear();

   // Add some buttons to Analysis section
   pb_loadsfit = us_pushbutton( tr( "Load Species Fits" ) );
   pb_sfitdata = us_pushbutton( tr( "Species Fit Data"  ) );
   pb_prev     = us_pushbutton( tr( "Previous"  ) );
   pb_next     = us_pushbutton( tr( "Next"  ) );
   QLabel* lb_specpl = us_label( tr ( "Species to Plot:" ) );
   pb_prev->setIcon( US_Images::getIcon( US_Images::ARROW_LEFT  ) );
   pb_next->setIcon( US_Images::getIcon( US_Images::ARROW_RIGHT ) );
   int row     = 3;
   analysisLayout->addWidget( pb_loadsfit, row,   0, 1, 1 );
   analysisLayout->addWidget( pb_sfitdata, row++, 1, 1, 1 );
   analysisLayout->addWidget( lb_specpl,   row++, 0, 1, 2 );
   analysisLayout->addWidget( pb_prev,     row,   0, 1, 1 );
   analysisLayout->addWidget( pb_next,     row++, 1, 1, 1 );
   pb_loadsfit->setEnabled( false );
   pb_sfitdata->setEnabled( false );
   pb_prev    ->setEnabled( false );
   pb_next    ->setEnabled( false );

   connect( pb_loadsfit, SIGNAL( clicked()     ),
            this,        SLOT  ( loadSpecs() ) );
   connect( pb_sfitdata, SIGNAL( clicked()     ),
            this,        SLOT  ( specFitData() ) );
   connect( pb_prev,  SIGNAL( clicked() ), SLOT( prev_plot() ) );
   connect( pb_next,  SIGNAL( clicked() ), SLOT( next_plot() ) );
   connect( pb_help,  SIGNAL( clicked() ), SLOT( help() ) );
   connect( pb_view,  SIGNAL( clicked() ), SLOT( view() ) );
   connect( pb_save,  SIGNAL( clicked() ), SLOT( save() ) );

   // Destroy all parameter rows from AnalysisBase
   int nrow   = parameterLayout->rowCount();
   int ncol   = parameterLayout->columnCount();
DbgLv(1) << "Parameter nrow" << nrow << "ncol" << ncol;
   for ( int irow = nrow - 1; irow >= 0; irow-- )
   {
      for ( int icol = ncol - 1; icol >= 0; icol-- )
      {
         QLayoutItem* item = parameterLayout->itemAtPosition( irow, icol );
         if ( item == NULL )  continue;
DbgLv(1) << "  irow" << irow << "icol" << icol;
//         parameterLayout->removeItem( item );
//         delete item->widget();
         item->widget()->setVisible( false );
      }
   }

   // Destroy unneeded controls rows from AnalysisBase
   nrow       = controlsLayout->rowCount();
   ncol       = controlsLayout->columnCount();
DbgLv(1) << "Controls nrow" << nrow << "ncol" << ncol;
   for ( int irow = nrow - 1; irow > 2; irow-- )
   {
      for ( int icol = ncol - 1; icol >= 0; icol-- )
      {
         QLayoutItem* item = controlsLayout->itemAtPosition( irow, icol );
         if ( item == NULL )  continue;
DbgLv(1) << "  irow" << irow << "icol" << icol;
//         controlsLayout->removeItem( item );
//         delete item->widget();
         item->widget()->setVisible( false );
      }
   }

   QLabel* lb_fit_error = us_label("Root-Mean-Square Deviation:");
   le_fit_error = us_lineedit("", -1, true);
   pb_plot3d = us_pushbutton("Plot3D", false);
   row       = controlsLayout->rowCount() + 1;
   QHBoxLayout* sf_lyt = new QHBoxLayout();
   sf_lyt->addWidget(lb_fit_error);
   sf_lyt->addWidget(le_fit_error);
   sf_lyt->addWidget(pb_plot3d);
   controlsLayout->addLayout(sf_lyt, row, 0, 1, 2);
   connect(pb_plot3d, SIGNAL(clicked()), this, SLOT(rmsd_3dplot()));

   data_plot1->setTitle( tr( "Output Data Set" ) );
   data_plot2->setTitle( tr( "Input Data Set" ) );
   data_plot1->setMinimumSize( 600, 300 );
   data_plot2->setMinimumSize( 600, 300 );
}

void US_MwlSpeciesFit::data_plot( void )
{
   plot_data2();

   if ( nspecies > 0 )
   {
      plot_data1();
   }
}

void US_MwlSpeciesFit::plot_data2( void )
{
   QList< QColor > mcolors;
   int nmcols         = plotLayout2->map_colors( mcolors );
   dPlotClearAll( data_plot2 );

   int tripx          = triple_index( lw_triples->currentRow() );
   US_DataIO::EditedData* edata = &dataList[ tripx ];

   QString                        dataType = tr( "Absorbance" );
   if ( edata->dataType == "RI" ) dataType = tr( "Intensity" );
   if ( edata->dataType == "WI" ) dataType = tr( "Intensity" );
   if ( edata->dataType == "IP" ) dataType = tr( "Interference" );
   if ( edata->dataType == "FI" ) dataType = tr( "Fluorescence" );

   QString header = tr( "Input Velocity Data for\n") + edata->runID + "  ("
         + edata->cell + "/" + edata->channel + "/" + edata->wavelength + ")";
   QString ytitle = dataType + tr( " at " ) + edata->wavelength + tr( " nm" );

   data_plot2->setTitle( header );
   data_plot2->setAxisTitle( QwtPlot::yLeft,   ytitle );
   data_plot2->setAxisTitle( QwtPlot::xBottom, tr( "Radius (cm) " ) );
   data_plot2->setAxisAutoScale( QwtPlot::yLeft   );
   data_plot2->setAxisAutoScale( QwtPlot::xBottom );

//   data_plot2->clear();
   us_grid( data_plot2 );

   int     scan_number = 0;
   int     from        = (int)ct_from->value();
   int     to          = (int)ct_to  ->value();

   int     scanCount   = edata->scanCount();
   int     points      = edata->pointCount();
   double  boundaryPct = ct_boundaryPercent->value() / 100.0;
   boundaryPct = ct_boundaryPercent->isEnabled() ? boundaryPct : 9.0;
   double  positionPct = ct_boundaryPos    ->value() / 100.0;
   double  baseline    = calc_baseline();

   QVector< double > rvec( points );
   QVector< double > vvec( points );
   double* rr          = rvec.data();
   double* vv          = vvec.data();

   // Draw curves
   for ( int ii = 0; ii < scanCount; ii++ )
   {
      if ( excludedScans.contains( ii ) ) continue;
DbgLv(1) << "MSF: dc: ii" << ii << "NON_EXCLUDED";

      scan_number++;
      bool highlight = scan_number >= from  &&  scan_number <= to;

      US_DataIO::Scan*  escan = &edata->scanData[ ii ];

      double range       = escan->plateau - baseline;
      double lower_limit = baseline    + range * positionPct;
      double upper_limit = lower_limit + range * boundaryPct;

      int jj    = 0;
      int count = 0;

      // Plot each scan in (up to) three segments: below, in, and above
      // the specified boundaries
      while (  jj < points  &&  escan->rvalues[ jj ] < lower_limit )
      {
         rr[ count ] = edata->xvalues[ jj ];
         vv[ count ] = escan->rvalues[ jj ];
         jj++;
         count++;
      }

      QString       title; 
      QwtPlotCurve* curv;

      if ( count > 1 )
      {
         title = tr( "Curve " ) + QString::number( ii ) + tr( " below range" );
         curv  = us_curve( data_plot2, title );

         if ( highlight )
            curv->setPen( QPen( Qt::red ) );
         else
            curv->setPen( QPen( Qt::cyan ) );
         
         curv->setSamples( rr, vv, count );
      }

      count = 0;

      while ( jj < points  &&  escan->rvalues[ jj ] < upper_limit )
      {
         rr[ count ] = edata->xvalues[ jj ];
         vv[ count ] = escan->rvalues[ jj ];
         jj++;
         count++;
      }

      if ( count > 1 )
      {
         title = tr( "Curve " ) + QString::number( ii ) + tr( " in range" );
         curv  = us_curve( data_plot2, title );

         if ( highlight )
            curv->setPen( QPen( Qt::red ) );
         else if ( nmcols > 0 )
            curv->setPen( QPen( mcolors[ ii % nmcols ] ) );
         else
            curv->setPen( QPen( US_GuiSettings::plotCurve() ) );
         
         curv->setSamples( rr, vv, count );
      }

      count = 0;

      while ( jj < points )
      {
         rr[ count ] = edata->xvalues[ jj ];
         vv[ count ] = escan->rvalues[ jj ];
         jj++;
         count++;
      }

      if ( count > 1 )
      {
         title = tr( "Curve " ) + QString::number( ii ) + tr( " above range" );
         curv = us_curve( data_plot2, title );

         if ( highlight )
            curv->setPen( QPen( Qt::red ) );
         else
            curv->setPen( QPen( Qt::cyan ) );
        
         curv->setPen( QPen( mcolors[ ii % nmcols ] ) );
         curv->setSamples( rr, vv, count );
      }
   }

   data_plot2->replot();

   return;
}

void US_MwlSpeciesFit::plot_data1( void )
{
if(jspec>=0)
DbgLv(1) << "PlotData1: jspec" << jspec << "have" << have_p1[jspec];
   if ( jspec < 0  ||  ! have_p1[ jspec ] )
      return;
   dPlotClearAll( data_plot1 );

//   int ccx        = lw_triples->currentRow();
//   int tripx      = triple_index( ccx );
//DbgLv(1) << "PlotData1:  ccx tripx" << ccx << tripx;
   int ispec      = ( jspec % nspecies ) + 1;
DbgLv(1) << "PlotData1:  jspec ispec" << jspec << ispec;
   US_DataIO::RawData*     rdata = &synData[ jspec ];
//   US_DataIO::EditedData*  edata = &dataList[ tripx ];
   QString str_wl = QString().sprintf( "%03i", ispec );
   QString str_ce = QString().sprintf( "%d", rdata->cell );

   QString                                  dataType = tr( "Absorbance" );
   if ( strcmp( rdata->type, "RI" ) == 0 )  dataType = tr( "Intensity" );
   if ( strcmp( rdata->type, "WI" ) == 0 )  dataType = tr( "Intensity" );
   if ( strcmp( rdata->type, "IP" ) == 0 )  dataType = tr( "Interference" );
   if ( strcmp( rdata->type, "FI" ) == 0 )  dataType = tr( "Fluorescence" );

   QString header = tr( "Output Velocity Data for\n") + runID + "  ("
         + str_ce + "/" + rdata->channel + "/" + str_wl + ")";
   data_plot1->setTitle( header );
   data_plot1->setAxisTitle( QwtPlot::yLeft, dataType );
   data_plot1->setAxisTitle( QwtPlot::xBottom, tr( "Radius (cm) " ) );
   data_plot1->setAxisAutoScale( QwtPlot::yLeft   );
   data_plot1->setAxisAutoScale( QwtPlot::xBottom );

//   data_plot1->clear();
   us_grid( data_plot1 );

   int    scan_number = 0;
   int    from        = (int)ct_from->value();
   int    to          = (int)ct_to  ->value();

   int    scanCount   = rdata->scanCount();
   int    points      = rdata->pointCount();

   QVector< double > rvec( points );
   QVector< double > vvec( points );
   double* rr         = rvec.data();
   double* vv         = vvec.data();

   // Draw curves
   for ( int ii = 0; ii < scanCount; ii++ )
   {
      if ( excludedScans.contains( ii ) ) continue;

      scan_number++;

      US_DataIO::Scan*  rscan = &rdata->scanData[ ii ];
      bool highlight   = scan_number >= from  &&  scan_number <= to;

      // Plot each scan

      for ( int jj = 0; jj < points; jj++ )
      {
         rr[ jj ] = rdata->xvalues[ jj ];
         vv[ jj ] = rscan->rvalues[ jj ];
      }

      QString       title = tr( "Curve " ) + QString::number( ii );
      QwtPlotCurve* curv  = us_curve( data_plot1, title );

      if ( highlight )
         curv->setPen( QPen( Qt::red ) );
      else
         curv->setPen( QPen( US_GuiSettings::plotCurve() ) );
         
      curv->setSamples( rr, vv, points );
   }

   data_plot1->replot();

   te_desc->setText( rdata->description );

   return;
}

void US_MwlSpeciesFit::write_report( QTextStream& ts )
{
   int                    index  = lw_triples->currentRow();
   US_DataIO::EditedData* edata  = &dataList[ index ];

   QString sm_results = 
        table_row( tr( "Average Second Moment S: " ),
                    + " 1.0 s * 10e-13" );

   ts << html_header( QString( "US_Second_Moment" ),
                      QString( "Second Moment Analysis" ),
                      edata );
   ts << analysis( sm_results );
   ts << indent( 2 ) + "</body>\n</html>\n";
}

void US_MwlSpeciesFit::view( void )
{
   // Write main report as string
   QString rtext;
   QTextStream ts( &rtext );
   write_report( ts );

   // Create US_Editor and display report
   if ( te_results == NULL )
   {
      te_results = new US_Editor( US_Editor::DEFAULT, true, QString(), this );
      te_results->resize( 600, 700 );
      QPoint p = g.global_position();
      te_results->move( p.x() + 30, p.y() + 30 );
      te_results->e->setFont( QFont( US_GuiSettings::fontFamily(),
                                     US_GuiSettings::fontSize() ) );
   }

   te_results->e->setHtml( rtext );
   te_results->show();
}

void US_MwlSpeciesFit::save( void )
{
   int                    index  = lw_triples->currentRow();
   US_DataIO::EditedData* d      = &dataList[ index ];
   QString                 dir   = US_Settings::reportDir();

   if ( ! mkdir( dir, d->runID ) ) return;

   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   // Note: d->runID is both directory and first segment of file name
   QString filebase = dir + "/" + d->runID + "/secmo."
      + QString( triples.at( index ) ).replace( " / ", "" ) + ".";
   
   QString plot1File = filebase + "speciesfit.svgz";
   QString plot2File = filebase + "velocity.svgz";
   QString textFile  = filebase + "speciesfit.csv";
   QString htmlFile  = filebase + "report.html";
   QString dsinfFile = QString( filebase ).replace( "/secmo.", "/dsinfo." )
                                + "dataset_info.html";

   // Write a general dataset information file
   write_dset_report( dsinfFile );

   // Write main report
   QFile reportf( htmlFile );

   if ( ! reportf.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
   {
      QMessageBox::warning( this,
            tr( "IO Error" ),
            tr( "Could not open\n" ) + htmlFile + "\n" +
                tr( "\nfor writing" ) );
      QApplication::restoreOverrideCursor();
      return;
   }

   QTextStream ts( &reportf );
   write_report( ts );
   reportf.close();

   // Write plots
   write_plot( plot1File, data_plot1 );
   write_plot( plot2File, data_plot2 );
   
   // Write moment data
   QFile sm_data( textFile );
   if ( ! sm_data.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
   {
      QMessageBox::warning( this,
            tr( "IO Error" ),
            tr( "Could not open\n" ) + textFile + "\n" +
                tr( "\nfor writing" ) );
      QApplication::restoreOverrideCursor();
      return;
   }

   const QString sep( "\",\"" );
   const QString quo( "\"" );
   const QString eln( "\"\n" );
   QTextStream ts_data( &sm_data );

   int scanCount = d->scanData.size();
   int excludes  = le_skipped->text().toInt();
   
   if ( excludes == scanCount )
      ts_data << "No valid scans\n";
   else
   {
      ts_data << quo << "Count" << sep << "Points" << sep << "Seconds" << eln;
      int count = 1;
      for ( int i = excludes; i < scanCount; i++ )
      {
         if ( excludedScans.contains( i ) ) continue;

         QString strK = QString::number( count                  ).simplified();
         ts_data << quo << strK << sep << eln;
         count++;
      }
   }

   sm_data.close();
   QStringList repfiles;
   update_filelist( repfiles, htmlFile  );
   update_filelist( repfiles, plot1File );
   update_filelist( repfiles, plot2File );
   update_filelist( repfiles, textFile  );
   update_filelist( repfiles, dsinfFile );

   // Tell user
   htmlFile  = htmlFile .mid( htmlFile .lastIndexOf( "/" ) + 1 );
   plot1File = plot1File.mid( plot1File.lastIndexOf( "/" ) + 1 );
   plot2File = plot2File.mid( plot2File.lastIndexOf( "/" ) + 1 );
   textFile  = textFile .mid( textFile .lastIndexOf( "/" ) + 1 );
   dsinfFile = dsinfFile.mid( dsinfFile.lastIndexOf( "/" ) + 1 );

   QString wmsg = tr( "Wrote:\n  " ) + htmlFile  + "\n  "
      + plot1File + "\n  " + plot2File + "\n  " + textFile + "\n  "
      + dsinfFile;

   if ( disk_controls->db() )
   {  // Write report files to the database
      reportFilesToDB( repfiles );

      wmsg += tr( "\n\nReport files were also saved to the database." );
   }

   QApplication::restoreOverrideCursor();
   QMessageBox::warning( this, tr( "Success" ), wmsg );
}

// Load edit data
void US_MwlSpeciesFit::load( void )
{
//   US_AnalysisBase2::load();
   // Determine the edit ID
   dataLoaded = false;
   dataList     .clear();
   rawList      .clear();
   excludedScans.clear();
   triples      .clear();
   savedValues  .clear();

   lw_triples->disconnect();
   lw_triples->clear();
   ct_from   ->disconnect();
   ct_from   ->setValue( 0 );


   bool edlast = ck_edlast->isChecked();
   int  dbdisk = ( disk_controls->db() ) ? US_Disk_DB_Controls::DB
                                         : US_Disk_DB_Controls::Disk;
   QString description;

   // US_DataLoader* dialog = new US_DataLoader(
   //       edlast, dbdisk, rawList, dataList, triples, description, etype_filt );

   US_DataLoader* dialog = new US_DataLoader(
	 edlast, dbdisk, rawList, dataList, triples, description, "none" );

   connect( dialog, SIGNAL( changed( bool ) ), SLOT( update_disk_db( bool ) ) );
   connect( dialog, SIGNAL( progress    ( const QString ) ), 
                    SLOT  ( set_progress( const QString ) ) );

   if ( dialog->exec() != QDialog::Accepted ) return;

   if ( disk_controls->db() )
      directory = tr( "(database)" );

   else
   {
      directory = description.section( description.left( 1 ), 4, 4 );
      directory = directory.left( directory.lastIndexOf( "/" ) );
   }

DbgLv(1) << "ldnois: done";
   int nscans  = dataList[ 0 ].scanCount();
   int ntrips  = dataList.count();
DbgLv(1) << "ldnois:  nscans" << nscans << "ntrips" << ntrips;

   // Save original readings values for each scan of the 1st data set
   for ( int ii = 0; ii < nscans; ii++ )
      savedValues << dataList[ 0 ].scanData[ ii ].rvalues;

   noiflags.fill( -1,            ntrips );
   allExcls.fill( excludedScans, ntrips );
   rinoises.fill( US_Noise(),    ntrips );
   tinoises.fill( US_Noise(),    ntrips );

   // Enable other buttons
   pb_solution->setEnabled( true );
   pb_details ->setEnabled( true );
   pb_view    ->setEnabled( true );
   pb_save    ->setEnabled( true );
   pb_exclude ->setEnabled( true );

   connect( ct_from, SIGNAL( valueChanged( double ) ),
                     SLOT  ( exclude_from( double ) ) );

   dataLoaded = true;
   emit dataAreLoaded();
   qApp->processEvents();
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   qApp->processEvents();

   runID         = dataList[ 0 ].runID;
   US_LoadRunNoise ldnois( this );
   bool dbload   = disk_controls->db();
   int ntedit    = 0;
   te_desc->setHtml( tr( "<b>Loading and counting noises "
                         "for %1 triples ...</b>" ).arg( ntrips ) );
   qApp->processEvents();
   qApp->processEvents();

DbgLv(1) << "ldnois:  calling count_noise()";
   int nrnois    = ldnois.count_noise( dbload, runID, NULL, NULL, &ntedit );
DbgLv(1) << "ldnois  ntedit nrnois" << ntedit << nrnois;

   // Do the "triples" list as cell/channel/wl-range and apply noises
   for ( int ii = 0; ii < ntrips; ii++ )
   {
      QString triple   = triples.at( ii );
      QString celchn   = triple.section( "/", 0, 0 ).simplified() +
                         triple.section( "/", 1, 1 ).simplified();
      QString waveln   = triple.section( "/", 2, 2 ).simplified();
      int ccx          = celchns.indexOf( celchn );
DbgLv(1) << "trip" << ii << "triple" << triple << "celchn" << celchn
 << "waveln" << waveln << "ccx" << ccx;
      if ( ccx < 0 )
      {  // New cell/channel:  add to list and save first index
         celchns << celchn;
         ftndxs << ii;
         ltndxs << ii;
      }
      else
      {  // Existing cell/channel:  update last index
         ltndxs[ ccx ] = ii;
      }

      // Apply noises associated with each triple
      US_Noise ti_noise;
      US_Noise ri_noise;
      QString edGUID   = dataList[ ii ].editGUID;
      int noisf        = ldnois.get_noises( edGUID, ti_noise, ri_noise );
DbgLv(1) << "  trip" << ii << "noise subtraction  noisf" << noisf
 << "tiC riC" << ti_noise.count << ri_noise.count
 << "tiK riK" << ti_noise.values.count() << ri_noise.values.count();
      apply_noise( ii, ti_noise, ri_noise );
   }

   celchn_wvl.clear();
   for ( int ii = 0; ii < celchns.count(); ii++ ){
       int ftndx   = ftndxs[ ii ];
       int ltndx   = ltndxs[ ii ];
       QVector< int > wvl;
       for (int jj = ftndx; jj <= ltndx; jj++)
           wvl << triples[ jj ].section( "/", 2, 2 ).simplified().toInt();
       celchn_wvl << wvl;
   }

   QApplication::restoreOverrideCursor();
   qApp->processEvents();
   lw_triples->disconnect();
   lw_triples->clear();

   for ( int ii = 0; ii < celchns.count(); ii++ )
   {
      int ftndx   = ftndxs[ ii ];
      int ltndx   = ltndxs[ ii ];
      QString fwvln  = triples[ ftndx ].section( "/", 2, 2 ).simplified();
      QString lwvln  = triples[ ltndx ].section( "/", 2, 2 ).simplified();
      QString triple = celchns[ ii ].left( 1 ) + " / "
                     + celchns[ ii ].mid( 1 )  + " / "
                     + fwvln + "-" + lwvln;
      lw_triples->addItem( triple );
   }
   synFitError.clear();
   synFitError.resize( celchns.count() );

   connect( lw_triples, SIGNAL( currentRowChanged( int ) ),
                        SLOT  ( new_triple       ( int ) ) );
   lw_triples->setCurrentRow( 0 );

   pb_loadsfit->setEnabled( true );

}

// Load species vector files
void US_MwlSpeciesFit::loadSpecs()
{
   // Get the species (wavelength,extinction) file names

   QStringList flist = QFileDialog::getOpenFileNames( this, "Load Species Files", US_Settings::resultDir(),
                                                      "Text Files (*.dat *.csv *.dsp );; All Files (*)");
   if (flist.size() < 2) {
      QMessageBox::critical( this, tr( "Multiple Species Needed" ),
                            tr( "At lease 2 species must be chosen!\n" ) );
      return;
   }


   QVector<US_CSV_Data> data_list;
   for ( int ii = 0; ii < flist.size(); ii++ ) {
      QString filepath = flist.at(ii);
      QString note = "1st Column -> WAVELENGTH ; 2nd Column -> OD";
      US_CSV_Loader *csv_loader = new US_CSV_Loader(filepath, note, true, this);
      int check = csv_loader->exec();
      if (check == QDialog::Rejected) {
         QMessageBox::critical(this, "Warning!", "Loading species is canceled!");
         return;
      } else if (check == -2) {
         QMessageBox::critical(this, "Error!", tr("The loaded file is not in text format!\n\n%1").arg(filepath));
         return;
      }
      US_CSV_Data csv_data = csv_loader->data();
      if (csv_data.columnCount() < 2 ) {
         QMessageBox::critical(this, "Warning!", "At least two data columns are needed:\n" + filepath);
         return;
      } else {
         data_list << csv_data;
      }
   }

   QStringList   spfiles_tmp;
   int nspecies_tmp = data_list.size();
   int nsferr      = 0;

   if ( nspecies_tmp < 2 )
      return;

   int minnw       = 999999;
   int maxnw       = 0;
   QVector< QVector< int > >     spwavls_tmp;
   QVector< QVector< double > >  spconcs_tmp;
//   QVector< int >     nwavls_tmp;
DbgLv(1) << "SpFiles:";
   for ( int ii = 0; ii < nspecies_tmp; ii++ )
   {
      QFileInfo finfo = data_list[ii].filePath();
      spfiles_tmp << finfo.filePath();
      QVector<double> xvals = data_list[ii].columnAt(0);
      QVector<double> yvals = data_list[ii].columnAt(1);

      QVector< int >      spwvls;
      QVector< double >   spcncs;
      for (int jj = 0; jj < xvals.size(); jj++ ) {
         int iwavl = static_cast<int> (xvals.at(jj));
         if (spwvls.contains(iwavl)){
            QMessageBox::warning( this, tr("IO Error"),
                                 tr("The file:\n") + finfo.fileName() + tr("\nhas got redundant wavelength values.\n") +
                                 tr("Please provide the wavelength values without decimals."));
            return;
         }
         spwvls << iwavl;
         spcncs << yvals.at(jj);
      }

      minnw           = qMin( minnw, spwvls.size() );
      maxnw           = qMax( maxnw, spwvls.size() );

      spwavls_tmp << spwvls;
      spconcs_tmp << spcncs;

//      nwavls_tmp << nwavl;
   }

//*DEBUG*
DbgLv(0) << "Species file count" << nspecies_tmp << "nsferr" << nsferr;
DbgLv(0) << "Species min,max wavelengths" << minnw << maxnw;
//int je=-1;
//int js=0;
//for ( int ii = 0; ii < nspecies_tmp; ii++ )
//{
//  int nwavl = nwavls_tmp[ii];
//  js = je + 1;
//  je = je + nwavl;
//  DbgLv(1) << " ii" << ii << "nwavl" << nwavl << "w0 wn c0 cn"
//   << spwavls_tmp[js] << spwavls_tmp[je] << spconcs_tmp[js] << spconcs_tmp[je];
//}
for ( int ii = 0; ii < nspecies_tmp; ii++ )
{
  int nwavl = spwavls_tmp.at(ii).size();
  for (int jj = 0; jj < nwavl; jj++){
DbgLv(0) << " jj" << jj << "nwavl" << nwavl << "wn cn"
<< spwavls_tmp.at(ii).at(jj) << spconcs_tmp.at(ii).at(jj);
  }
}

//*DEBUG*

    // checking for the base profile matched to triple wavelengths
    QVector< int >     spwavls_chk;
    QVector< double >  spconcs_chk;
    QVector< int >     nwavls_chk;
    int ccx = lw_triples->currentRow();
    QVector< int > curr_celchnwvl = celchn_wvl.at(ccx);
    int nwls = curr_celchnwvl.size();
    QStringList bad_files;
    for (int ii = 0; ii < nspecies_tmp; ii++){
        QString fname   = spfiles_tmp.at(ii);
        for (int jj = 0; jj < nwls; jj++){
            int wavl = curr_celchnwvl.at(jj);
            int idx = spwavls_tmp.at(ii).indexOf(wavl);
            if (idx == -1){
                bad_files << fname;
                break;
            }
            spwavls_chk << wavl;
            spconcs_chk << spconcs_tmp.at(ii).at(idx);
        }
        nwavls_chk << nwls;
    }

    if (bad_files.size() > 0){
        QString fnames("\n");
        for (int ii = 0; ii < bad_files.size(); ii++)
            fnames = fnames.append(bad_files.at(ii) + tr("\n"));
        QMessageBox::warning( this, tr("IO Error"),
                              tr("The following file(s):\n") + fnames +
                              tr("\ndo(es) not have the valid data over"
                                 " the range of %1 to %2 nm. ").
                              arg(curr_celchnwvl.at(0)).arg(curr_celchnwvl.at(nwls - 1)) +
                              tr("Please provide the proper spectral profiles."));
        return;
    }

    spfiles.clear();
    spwavls.clear();
    spconcs.clear();
    nwavls.clear();
    nspecies = nspecies_tmp;
    spfiles << spfiles_tmp;
    spwavls << spwavls_chk;
    spconcs << spconcs_chk;
    nwavls << nwavls_chk;

   // Initialize species data
   int ktspec     = nspecies * celchns.count();
   synData.fill( rawList[ 0 ], ktspec );
   have_p1.fill( false,        ktspec );
DbgLv(1) << "Species ktspec sD,hvp sizes" << ktspec << synData.size()
 << have_p1.size();

   pb_sfitdata->setEnabled( true );
}

// Create species-fit synthetic data
void US_MwlSpeciesFit::specFitData()
{
QDateTime time0=QDateTime::currentDateTime();
   // Construct lambda array and various counts,indexes
   int ccx        = lw_triples->currentRow();
   int tripx      = triple_index( ccx );
DbgLv(1) << "sfd: ccx tripx" << ccx << tripx;
   US_DataIO::EditedData*  edata = &dataList[ tripx ];
   int nscan      = edata->scanCount();
   int npoint     = edata->pointCount();
   int kdstart    = ccx * nspecies;
   double meniscus  = edata->meniscus;
   kscan          = nscan - excludedScans.count();
DbgLv(1) << "sfd:  nscan kscan npoint" << nscan << kscan << npoint;
   kradii         = npoint;
   radxs          = 0;
   radxe          = npoint;
   trpxs          = ftndxs[ ccx ];
   trpxe          = ltndxs[ ccx ] + 1;
   lmbxs          = 0;
   lambdas.clear();
   klambda        = 0;
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   for ( int ii = trpxs; ii < trpxe; ii++ )
   {
      klambda++;
      lambdas << triples[ ii ].section( " / ", 2, 2 ).simplified().toInt();
DbgLv(1) << "sfd:  klambda ii lmb" << klambda << ii << lambdas[klambda-1];
   }
DbgLv(1) << "sfd: klambda ccx trpxs trpxe" << klambda << ccx << trpxs << trpxe;

   lmbxe          = klambda;

   // Construct the A matrix, species columns and wavelengths rows
   QVector< double > sv_nnls_a;
   QVector< double > nnls_a;
   QVector< double > nnls_b;
   QVector< double > nnls_x;
   nnls_a.fill( 0.0, klambda * nspecies );
   nnls_b.fill( 0.0, klambda );
   nnls_x.fill( 0.0, nspecies );

   int ka       = 0;
   int kw       = 0;
DbgLv(1) << "sfd: size a,b,x" << nnls_a.count() << nnls_b.count()
 << nnls_x.count();

   for ( int ii = 0; ii < nspecies; ii++ )
   {  // Loop through each species, storing reading value for each wavelength
      int nwavl    = nwavls[ ii ];
      QVector< int >    cswavls;
      QVector< double > csconcs;

      for ( int jj = 0; jj < nwavl; jj++, kw++ )
      {  // Get wavelength,concentration vectors for this species
         cswavls << spwavls[ kw ];
         csconcs << spconcs[ kw ];
DbgLv(1) << "sfd: Get wavelength,concentration vectors for species " << ii << ":" << spwavls[ kw ] << spconcs[ kw ];

      }
DbgLv(1) << "sfd:  sp" << ii << "nwavl" << nwavl;

      for ( int jj = 0; jj < klambda; jj++ )
      {  // Loop through each wavelength of the current range
         int iwavl    = lambdas[ jj ];
         double cval  = 0.0;
//         double wval  = (double)iwavl;
         int kwx      = cswavls.indexOf( iwavl );

         if ( kwx >= 0 )
         {  // Matching wavelength:  get corresponding concentration
            cval         = csconcs[ kwx ];
         }

//         else if ( iwavl < cswavls[ 0 ]  ||
//                   iwavl > cswavls[ nwavl - 1 ] )
//         {  // Data wavelength beyond species range:  use zero concentration
//            cval          = 0.0;
//DbgLv(1) << "Data wavelength beyond species range: " << iwavl;
//         }

//         else
//         {  // Must search species list and interpolate between wavelengths
//            for ( int kk = 0; kk < nwavl; kk++ )
//            {
//               int kwavl    = cswavls[ kk ];
//               cval         = csconcs[ kk ];

//               if ( iwavl > kwavl )
//               {  // Gone beyond species wavelength:  interpolate using previous
//                  int kkp      = qMax( 0, kk - 1 );
//                  double dcval = cval - csconcs[ kkp ];
//                  double wval1 = (double)kwavl;
//                  double wval0 = (double)cswavls[ kkp ];
//                  double dwavl = ( kkp == kk ) ? 1.0 : ( wval1 - wval0 );
//                  cval        += ( ( wval - wval0 ) * dcval / dwavl );
//DbgLv(1) << "sfd:    iw>kw : kwx" << kw << "wv0 wv1 dcv"
// << wval0 << wval1 << dcval << "  iwavl cval" << iwavl << cval;
//                  break;
//               }
//            }  // END: loop to find lambda in species list
//         }

if( (jj<4) || ((jj+5)>klambda) || (jj==(klambda/2)) )
DbgLv(1) << "sfd:   wv" << jj << iwavl << "ka" << ka << "cval" << cval;
         nnls_a[ ka++ ] = cval;
      }  // END:  loop through selected lambda range
DbgLv(1) << "sfd:    nnls_a" << nnls_a[ka-klambda]
   << nnls_a[ka-klambda+1] << nnls_a[ka-klambda+2] << nnls_a[ka-klambda/2]
   << nnls_a[ka-3] << nnls_a[ka-2] << nnls_a[ka-1];
   }  // END:  loop through species
QDateTime time1=QDateTime::currentDateTime();

   // Create output data sets and build the basic grid of values

   QList< int >        inclscns;
   US_DataIO::RawData* rdata = &rawList[ tripx ];
   QString ddesc    = rdata->description;
   int krpad        = 30;
   int kradp        = kradii + krpad;
   int kd           = kdstart;
   sv_nnls_a        = nnls_a;
DbgLv(1) << "sfd:    kradp kd" << kradp << kd << "tripx" << tripx;
DbgLv(1) << "sfd:    sv_nnls_a" << sv_nnls_a[ka-klambda]
   << sv_nnls_a[ka-klambda+1] << sv_nnls_a[ka-klambda+2] << sv_nnls_a[ka-klambda/2]
   << sv_nnls_a[ka-3] << sv_nnls_a[ka-2] << sv_nnls_a[ka-1];

   for ( int ii = 0; ii < nspecies; ii++ )
   {  // Loop to create synthetic data
      QString fname    = spfiles[ ii ];
      QString fbase    = fname.section( "/", -1, -1 );
      QString rawGUID  = US_Util::new_guid();
DbgLv(1) << "sfd: sp" << ii << "fbase" << fbase;

      synData[ kd ]    =  *rdata;

      US_Util::uuid_parse( rawGUID, (uchar*)synData[ kd ].rawGUID );
      synData[ kd ].xvalues .fill( 0.0, kradp );
      synData[ kd ].scanData.resize( kscan );
      synData[ kd ].description = ddesc + " - " + fbase.section( ".", 0, -2 );
      if ( edata->dataType == "RI" )
         memcpy( synData[ kd ].type, "RA", 2 );
      double wavl      = (double)( ii + 1 );
DbgLv(1) << "sfd:   desc" << synData[kd].description;
      int jr           = radxs;
      int ks           = 0;
DbgLv(1) << "sfd:   krpad kradp jr" << krpad << kradp << jr;

      // Store the radius values
      for ( int jj = krpad; jj < kradp; jj++, jr++ )
      {
//         synData[ kd ].xvalues[ jj ]  = rdata->xvalues[ jr ];
         synData[ kd ].xvalues[ jj ]  = edata->xvalues[ jr ];
      }
DbgLv(1) << "sfd:    jr" << jr << "synDsiz" << synData.size()
 << "scnSiz" << synData[0].scanData.size() << "kscan" << kscan
 << "rScnSiz" << rdata->scanCount() << rdata->scanData.count();
int kexcl=excludedScans.count();
DbgLv(1) << "sfd:    nscan kscan exclknt" << nscan << kscan << kexcl;
if(kexcl>4)
DbgLv(1) << "sfd:    excls 0 1 m n" << excludedScans[0]
   << excludedScans[1] << excludedScans[kexcl/2] << excludedScans[kexcl-1];

      // Store radius values for pre-data/meniscus padding
      double rad1v     = edata->xvalues[ radxs     ];
      double rad2v     = edata->xvalues[ radxs + 1 ];
      double radinc    = rad2v - rad1v;
      rad1v            = rad1v - radinc * krpad;

      for ( int jj = 0; jj < krpad; jj++, rad1v += radinc )
      {
         synData[ kd ].xvalues[ jj ] = rad1v;
      }

DbgLv(1) << "sfd: rad1 radinc" << rad1v << radinc;
DbgLv(1) << "sfd:  xv 0 1 p n" 
 << synData[kd].xvalues[0] << synData[kd].xvalues[1]
 << synData[kd].xvalues[krpad] << synData[kd].xvalues[kradp-1];
      // Create the non-excluded scans, with initialized readings vectors
      for ( int jj = 0; jj < edata->scanCount(); jj++ )
      {
         if ( excludedScans.contains( jj ) )  continue;

DbgLv(1) << "sfd:     jj" << jj << "ks" << ks << "kd" << kd;
         synData[ kd ].scanData[ ks ] = edata->scanData[ jj ];
         synData[ kd ].scanData[ ks ].rvalues.fill( 0.0, kradp );
         synData[ kd ].scanData[ ks ].stddevs.fill( 0.0, kradp );
         synData[ kd ].scanData[ ks ].wavelength  = wavl;
         ks++;

         if ( ii == 0 )
            inclscns << jj;
      }

      have_p1[ kd++ ] = true;
   }
QDateTime time2=QDateTime::currentDateTime();

   // Loop through triples in channel. Build B and compute X

   int narows     = klambda;
DbgLv(1) << "sfd: narows kscan inclsize" << narows << kscan << inclscns.size();

   synFitError[ccx].clear();
   for (int ii = 0; ii < lambdas.size(); ii++)
       synFitError[ccx].wavelenghts << (double) lambdas.at(ii);
   synFitError[ccx].xValues << synData.at(kdstart).xvalues.mid(krpad);

   for ( int ii = 0; ii < kscan; ii++ )
   {  // Loop through non-excluded scans
      int js         = inclscns[ ii ];
      int jr         = radxs;
DbgLv(1) << "sfd: sc" << ii << "js jr" << js << jr;

      synFitError[ccx].includedScans << js;
      QVector< QVector< QVector < double > > > orgSp_rpwl;
      for ( int jj = krpad; jj < kradp; jj++, jr++ )
      {  // Loop through radius values
         int trx        = trpxs + lmbxs;
         nnls_a         = sv_nnls_a;

         for ( int kk = 0; kk < klambda; kk++, trx++ )
         {  // Store scan,radius reading for each wavelength in channel
//            nnls_b[ kk ]   = rawList[ trx ].value( js, jr );
            nnls_b[ kk ]   = dataList[ trx ].value( js, jr );
         }
DbgLv(1) << "sfd: NNLS b:" << nnls_b[0] << nnls_b[klambda-1];

         QVector< double > sv_nnls_b = nnls_b;
         double rnorm;

         // Fit using NNLS to compute X value for each species
         US_Math2::nnls( nnls_a.data(), narows, narows, nspecies,
                         nnls_b.data(), nnls_x.data(), &rnorm );
DbgLv(1) << "sfd:  NNLS  ii jj kl" << ii << jj << klambda
 << " x:" << nnls_x[0] << nnls_x[nspecies-1];

         // Set scan,radius value in each species data set
         kd             = kdstart;
         for ( int mm = 0; mm < nspecies; mm++, kd++ )
         {
            synData[ kd ].setValue( ii, jj, nnls_x[ mm ] );
         }

         // Compute deviation from B for each wavelength
         int cntr = 0;
         double rnorm_c = 0;
         QVector< QVector< double > > orgSp_wl;
         for (int kk = 0; kk < klambda; kk++){
             double ax_sum = 0;
             QVector< double > orgSp;
             orgSp << sv_nnls_b.at(kk);
             for (int mm = 0; mm < nspecies; mm++){
                 int ndx_a = cntr + mm * narows;
                 double ax = sv_nnls_a.at(ndx_a) * nnls_x.at(mm);
                 orgSp << ax;
                 ax_sum += ax;
             }
             orgSp << ax_sum;
             orgSp_wl << orgSp;
             double dev = qPow(ax_sum - orgSp.at(0), 2);
             rnorm_c += dev;
             cntr++;
         }
         orgSp_rpwl << orgSp_wl;
DbgLv(1) << "sfd:  NNLS rnorm cmp_rnorm" << rnorm << qSqrt(rnorm_c);
      }
      synFitError[ccx].allData << orgSp_rpwl;
   }
   synFitError[ccx].computeMSE();

QDateTime time3=QDateTime::currentDateTime();
nnls_a=sv_nnls_a;
int ja=nnls_a.count()-1;
int ma=ja/2;
DbgLv(1) << "sfd:     NNLS a:" << nnls_a[0] << nnls_a[1] << nnls_a[ma-1]
  << nnls_a[ma] << nnls_a[ma+1] << nnls_a[ja-10] << nnls_a[ja];
int ms=kscan/2;
int mr=kradii/2;
//ms=100;
//mr=100;
DbgLv(1) << "sfd: kscan kradii ms mr" << kscan << kradii << ms << mr;
DbgLv(1) << "sfd: (A)D0 cmn" << ms << mr << synData[0].value(ms,mr);
DbgLv(1) << "sfd: (A)D1 cmn" << ms << mr << synData[1].value(ms,mr);

   // Output the synthetic data files to local disk
   QString cellch = QString( "%1.%2" ).arg( rdata->cell ).arg( rdata->channel );
   QString runSyn = "SSF-" + runID;
   QString dirSyn  = US_Settings::importDir() + "/" + runSyn;

   if ( ! QDir().exists( dirSyn ) )
      QDir().mkpath( dirSyn );

   QString basefn = dirSyn + "/" + runSyn + ".RA." + cellch + ".000.auc";
DbgLv(1) << "sfd: cellch" << cellch << "basefn" << basefn;
   QString msg    = tr( "Species-Fit Simulations have been produced.\n\n"
                        "In directory\n\n  %1, \n\n"
                        "the following files were created:\n\n" )
                    .arg( dirSyn );

DbgLv(1) << "sfd: (B)D0 cmn" << ms << mr << synData[0].value(ms,mr);
DbgLv(1) << "sfd: (B)D1 cmn" << ms << mr << synData[1].value(ms,mr);
   kd               = ccx * nspecies;

   for ( int ii = 0; ii < nspecies; ii++, kd++ )
   {
      QString str_wl  = QString().sprintf( ".%03i.auc", ii + 1 );
      QString fname   = QString( basefn ).replace( ".000.auc", str_wl );
      msg            += "   " + QString( fname ).section( "/", -1, -1 ) + "\n";

      US_DataIO::RawData*     rdata = &synData[ kd ];
      int    nsscan   = rdata->scanCount();
      int    nspoint  = rdata->pointCount();
      int    nits     = 10;
      int    msx      = qMax( ( nsscan / 8 ),  nits );
      int    mrx      = qMax( ( nspoint - 2 ), nits );
DbgLv(1) << "sfd:  msx mrx" << msx << mrx << "nsscan nspoint"
 << nsscan << nspoint;
      if (msx >= nsscan){
         msx = nsscan - 1;
         nits = msx - 1;
      }
      int    menx     = rdata->xindex( meniscus );
      double menval   = rdata->scanData[ msx ].rvalues[ mrx ];
      for ( int jj = 0; jj < nits; jj++ )
      {
         menval    = qMax( menval, rdata->scanData[ --msx ].rvalues[ --mrx ] );
      }
DbgLv(1) << "sfd:  menx menval meniscus" << menx << menval << meniscus;

      for ( int jj = 0; jj < nsscan; jj++ )
      {
         rdata->setValue( jj, menx, menval );
      }

      int stat        = US_DataIO::writeRawData( fname, synData[ kd ] );
DbgLv(1) << "sfd:  stat fname" << stat << fname;
   }
QDateTime time9=QDateTime::currentDateTime();
DbgLv(1) << "sfd: (C)D0 cmn" << ms << mr << synData[0].value(ms,mr);
DbgLv(1) << "sfd: (C)D1 cmn" << ms << mr << synData[1].value(ms,mr);
DbgLv(1) << "sfd: time1-0" << time0.msecsTo(time1) << "cellch" << cellch;
DbgLv(1) << "sfd: time2-1" << time1.msecsTo(time2);
DbgLv(1) << "sfd: time3-2" << time2.msecsTo(time3);
DbgLv(1) << "sfd: time9-3" << time3.msecsTo(time9);
DbgLv(1) << "sfd: time9-0" << time0.msecsTo(time9);
msg+=tr("\n time1-0: %1").arg(time0.msecsTo(time1))+" ms  (Read Fit Files)";
msg+=tr("\n time2-1: %1").arg(time1.msecsTo(time2))+" ms  (Initialize AUC)";
msg+=tr("\n time3-2: %1").arg(time2.msecsTo(time3))+" ms  (NNLS)";
msg+=tr("\n time9-3: %1").arg(time3.msecsTo(time9))+" ms  (Write Synth.AUC)";
msg+=tr("\n time9-0: %1").arg(time0.msecsTo(time9))+" ms  (All Fit Steps)";

   QMessageBox::information( this, tr( "Species-Fit Files" ), msg );
   QApplication::restoreOverrideCursor();
   qApp->processEvents();

   pb_prev->setEnabled( true );
   pb_next->setEnabled( true );
   jspec            = ccx * nspecies;

   plot_data1();
   get_fit_error();
}

void US_MwlSpeciesFit::new_triple( const int ccx )
{
   // Save the data for the new triple
   jspec          = ccx * nspecies;
   int tripx      = triple_index( ccx );
   US_DataIO::EditedData*  edata = &dataList[ tripx ];
   QString repdir = US_Settings::reportDir() + "/" + edata->runID;
 
   // Update GUI elements and plot for selected triple
   update( tripx );

   // Make sure we have a reports directory for this runID
   QDir dir;
   if ( ! dir.exists( repdir ) )  dir.mkpath( repdir );
   get_fit_error();
}

int US_MwlSpeciesFit::triple_index( const int ccx )
{
   return ( ftndxs[ ccx ] + ltndxs[ ccx ] ) / 2;
}

void US_MwlSpeciesFit::apply_noise( const int tripx,
      US_Noise& ti_noise, US_Noise& ri_noise )
{
   US_DataIO::EditedData*  edata = &dataList[ tripx ];
   int ntinois  = ti_noise.values.size();
   int nrinois  = ri_noise.values.size();
   int nscans   = edata->scanCount();
   int npoints  = edata->pointCount();
   int npadded  = 0;
   int ntrips   = triples.count();
   noiflags[ tripx ] = ( ntinois > 0 ? 1 : 0 ) +
                       ( nrinois > 0 ? 2 : 0 );

DbgLv(1) << "   ApNoi:    noiflag" << noiflags[tripx];
   te_desc->setHtml( tr( "<b>Subtracting noise from triple " )
                     + triples[ tripx ] + " ...</b><br/><b> ( " 
                     + tr( "%1 of %2 )</b>" ).arg( tripx + 1 ).arg( ntrips ) );
   qApp->processEvents();

   if ( ntinois > 0  ||  nrinois > 0 )
   {  // Apply noise(s) to data

      if ( ntinois > 0   &&  ntinois < npoints )
      {  // Pad out ti noise values to radius count
         int jj       = ntinois;
         while ( jj++ < npoints )
            ti_noise.values << 0.0;
         ti_noise.count = ti_noise.values.size();
         npadded++;
      }

      if ( nrinois > 0   &&  nrinois < nscans )
      {  // Pad out ri noise values to scan count
         int jj       = nrinois;
         while ( jj++ < nscans )
            ri_noise.values << 0.0;
         ri_noise.count = ri_noise.values.size();
         npadded++;
      }

      // Substract noise from the experiment
      for ( int ii = 0; ii < nscans; ii++ )
      {
         int iin      = qMin( ii, ( nrinois - 1 ) );
         double rinoi = ( nrinois > 0 ) ? ri_noise.values[ iin ] : 0.0;
         US_DataIO::Scan* escan = &edata->scanData[ ii ];

         for ( int jj = 0; jj < npoints; jj++ )
         {
            int jjn      = qMin( jj, ( ntinois - 1 ) );
            double tinoi = ( ntinois > 0 ) ? ti_noise.values[ jjn ] : 0.0;

            escan->rvalues[ jj ] = edata->value( ii, jj ) - rinoi - tinoi;
         }
      }

      rinoises[ tripx ] = ri_noise;
      tinoises[ tripx ] = ti_noise;
   }
}

void US_MwlSpeciesFit::prev_plot( )
{
   int ltspec      = nspecies * celchns.count() - 1;
   jspec--;
   jspec           = ( jspec < 0 ) ? ltspec : jspec;

   while ( ! have_p1[ jspec ] )
   {
      jspec--;
      jspec           = ( jspec < 0 ) ? ltspec : jspec;
   }
DbgLv(1) << "PrevPlot: nspecies ltspec jspec" << nspecies << ltspec << jspec;

   plot_data1();
}

void US_MwlSpeciesFit::next_plot( )
{
   int ltspec      = nspecies * celchns.count() - 1;
   jspec++;
   jspec           = ( jspec > ltspec ) ? 0 : jspec;

   while ( ! have_p1[ jspec ] )
   {
      jspec++;
      jspec           = ( jspec > ltspec ) ? 0 : jspec;
DbgLv(1) << "NextPlot:   jspec have" << jspec << have_p1[jspec];
   }
DbgLv(1) << "NextPlot: nspecies ltspec jspec" << nspecies << ltspec << jspec;

   plot_data1();
}

void US_MwlSpeciesFit::get_fit_error(){
    le_fit_error->clear();
    int ccx = lw_triples->currentRow();
    SFData fit_dev = synFitError.at(ccx);
    if (fit_dev.scansMSE.size() == 0){
        pb_plot3d->setDisabled(true);
        return;
    }
    pb_plot3d->setEnabled(true);
    double RMSD = 0;
    for (int i = 0; i < fit_dev.scansMSE.size(); ++i)
        RMSD += fit_dev.scansMSE.at(i);
    RMSD /= fit_dev.scansMSE.size();
    le_fit_error->setText(QString::number(qSqrt(RMSD), 'f', 6));
}

void US_MwlSpeciesFit::rmsd_3dplot(){
    int ccx = lw_triples->currentRow();
    const SFData* sfdata = synFitError.data();
    US_MWL_SF_PLOT3D* plot3d = new US_MWL_SF_PLOT3D(this, sfdata[ccx]);
    plot3d->exec();
}
