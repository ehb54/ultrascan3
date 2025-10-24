//! \file us_pcsa.cpp

#include <QApplication>
#include <QtSvg>

#include "us_pcsa.h"
#include "us_resids_bitmap.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_matrix.h"
#include "us_constants.h"
#include "us_analyte_gui.h"
#include "us_passwd.h"
#include "us_data_loader.h"
#include "us_util.h"
#include "us_investigator.h"
#include "us_loadable_noise.h"
#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#endif

//! \brief Main program for us_pcsa. Loads translators and starts
//         the class US_pcsa.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_pcsa w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

// constructor, based on AnalysisBase
US_pcsa::US_pcsa() : US_AnalysisBase2()
{
   setWindowTitle( tr( "Parametrically Constrained Spectrum Analysis" ) );
   setObjectName( "US_pcsa" );
   dbg_level  = US_Settings::us_debug();
   clean_etc_dir();
   baserss    = 0;

   // Build local and pcsa-specific GUI elements
   te_results = NULL;

   QLabel* lb_analysis = us_banner( tr( "Analysis Controls" ) );
   QLabel* lb_scan     = us_banner( tr( "Scan Control"       ) );

   QLabel* lb_status   = us_label ( tr( "Status\nInfo:" ) );
   te_status           = us_textedit();
   QLabel* lb_from     = us_label ( tr( "Scan focus from:" ) );
   QLabel* lb_to       = us_label ( tr( "to:"   ) );

   pb_exclude   = us_pushbutton( tr( "Exclude Scan Range" ) );
   pb_exclude->setEnabled( false );
   connect( pb_exclude, SIGNAL( clicked() ), SLOT( exclude() ) );

   // Effectively disable boundaries to turn off cyan portion of plot2
   ct_boundaryPercent->disconnect();
   ct_boundaryPercent->setRange     ( 0.0, 300.0 );
   ct_boundaryPercent->setSingleStep( 1.0 );
   ct_boundaryPercent->setValue     ( 300.0 );
   ct_boundaryPercent->setEnabled   ( false );
   ct_boundaryPos    ->disconnect   ();
   ct_boundaryPos    ->setRange     ( -50.0, 300.0 );
   ct_boundaryPos    ->setSingleStep( 1.0 );
   ct_boundaryPos    ->setValue     ( -50.0 );
   ct_boundaryPos    ->setEnabled   ( false );

   connect( ct_from, SIGNAL( valueChanged( double ) ),
                     SLOT  ( exclude_from( double ) ) );
   connect( ct_to,   SIGNAL( valueChanged( double ) ),
                     SLOT  ( exclude_to  ( double ) ) );
   pb_fitcntl   = us_pushbutton( tr( "Fit Control"   ) );
   pb_plt3d     = us_pushbutton( tr( "3-D Plot"      ) );
   pb_pltres    = us_pushbutton( tr( "Residual Plot" ) );

   connect( pb_fitcntl, SIGNAL( clicked() ), SLOT( open_fitcntl() ) );
   connect( pb_plt3d,   SIGNAL( clicked() ), SLOT( open_3dplot()  ) );
   connect( pb_pltres,  SIGNAL( clicked() ), SLOT( open_resplot() ) );

   // To modify controls layout, first make Base elements invisible
   
   QWidget* widg;
   for ( int ii = 0; ii < controlsLayout->count(); ii++ )
      if ( ( widg = controlsLayout->itemAt( ii )->widget() ) != 0 )
         widg->setVisible( false );
   ct_from         ->setVisible( true );
   ct_to           ->setVisible( true );
   pb_exclude      ->setVisible( true );
   pb_reset_exclude->setVisible( true );

   // Add variance and rmsd to parameters layout
   QLabel* lb_vari     = us_label ( tr( "Variance:" ) );
   le_vari             = us_lineedit( "0.00000", 0, true );
   QLabel* lb_rmsd     = us_label ( tr( "RMSD:" ) );
   le_rmsd             = us_lineedit( "0.00000", 0, true );
   int row   = parameterLayout->rowCount();
   parameterLayout->addWidget( lb_vari,         row,   0, 1, 1 );
   parameterLayout->addWidget( le_vari,         row,   1, 1, 1 );
   parameterLayout->addWidget( lb_rmsd,         row,   2, 1, 1 );
   parameterLayout->addWidget( le_rmsd,         row++, 3, 1, 1 );

   // Reconstruct controls layout with some pcsa-specific elements
   row       = 0;
   controlsLayout->addWidget( lb_scan,          row++, 0, 1, 6 );
   controlsLayout->addWidget( lb_from,          row,   0, 1, 2 );
   controlsLayout->addWidget( ct_from,          row++, 2, 1, 4 );
   controlsLayout->addWidget( lb_to,            row,   0, 1, 2 );
   controlsLayout->addWidget( ct_to,            row++, 2, 1, 4 );
   controlsLayout->addWidget( pb_exclude,       row,   0, 1, 2 );
   controlsLayout->addWidget( pb_reset_exclude, row++, 2, 1, 4 );
   controlsLayout->addWidget( lb_analysis,      row++, 0, 1, 6 );
   controlsLayout->addWidget( pb_fitcntl,       row,   0, 1, 2 );
   controlsLayout->addWidget( pb_plt3d,         row,   2, 1, 2 );
   controlsLayout->addWidget( pb_pltres,        row++, 4, 1, 2 );
   controlsLayout->addWidget( lb_status,        row,   0, 1, 1 );
   controlsLayout->addWidget( te_status,        row,   1, 1, 5 );
   row      += 3;

   // Set initial status text
   te_status->setAlignment( Qt::AlignCenter | Qt::AlignVCenter );
   te_status->setText( tr(
       "Solution not initiated...\n"
       "RMSD:  0.000000,\n"
       "Variance: 0.000000e-05 .\n"
       "Iterations:  0" ) );
   us_setReadOnly( te_status, true );

   connect( pb_help,  SIGNAL( clicked() ), SLOT( help()  ) );
   connect( pb_view,  SIGNAL( clicked() ), SLOT( view()  ) );
   connect( pb_save,  SIGNAL( clicked() ), SLOT( save()  ) );
   connect( pb_close, SIGNAL( clicked() ), SLOT( close() ) );

   pb_view   ->setEnabled( false );
   pb_save   ->setEnabled( false );
   pb_fitcntl->setEnabled( false );
   pb_plt3d  ->setEnabled( false );
   pb_pltres ->setEnabled( false );
   pb_exclude->setEnabled( false );
   ct_from   ->setEnabled( false );
   ct_to     ->setEnabled( false );

   edata        = 0;
   resplotd     = 0;
   eplotcd      = 0;
   analcd       = 0;

   rbd_pos      = this->pos() + QPoint(  100, 100 );
   epd_pos      = this->pos() + QPoint(  400, 200 );
   acd_pos      = this->pos() + QPoint(  500,  50 );

   dsets.clear();
   dsets << &dset;
}

// slot to handle the completion of a PC spectrum analysis stage
void US_pcsa::analysis_done( int updflag )
{
   if ( updflag == (-1) )
   {  // fit has been aborted or reset for new fit
      pb_view   ->setEnabled( false );
      pb_save   ->setEnabled( false );
      pb_plt3d  ->setEnabled( false );
      pb_pltres ->setEnabled( false );

      qApp->processEvents();
      return;
   }

   if ( updflag == (-2) )
   {  // update RMSD
      QString mdesc = model.description;
      QString avari = mdesc.mid( mdesc.indexOf( "VARI=" ) + 5 );
      double  vari  = avari.section( " ", 0, 0 ).toDouble();
      double  rmsd  = sqrt( vari );
      le_vari->setText( QString::number( vari ) );
      le_rmsd->setText( QString::number( rmsd ) );
DbgLv(1) << "Analysis Done VARI" << vari;

      qApp->processEvents();
      return;
   }

   // if here, an analysis is all done

   bool plotdata     = updflag == 1;
   bool savedata     = updflag == 2;

DbgLv(1) << "Analysis Done";
DbgLv(1) << "  model components size" << model.components.size();
DbgLv(1) << "  ti_noise size" << ti_noise.values.size() << ti_noise.count;
DbgLv(1) << "  ri_noise size" << ri_noise.values.size() << ri_noise.count;
DbgLv(1) << "  edat0 sdat0 rdat0"
 << edata->value(0,0) << sdata.value(0,0) << rdata.value(0,0);

   pb_plt3d ->setEnabled( true );
   pb_pltres->setEnabled( true );
   pb_view  ->setEnabled( true );
   pb_save  ->setEnabled( true );

   data_plot();

   if ( plotdata )
   {
      open_3dplot();
      open_resplot();
   }

   else if ( savedata )
   {
      save();
   }
}

// load the experiment data, mostly thru AnalysisBase; then disable view,save
void US_pcsa::load( void )
{
   US_AnalysisBase2::load();       // load edited experiment data

   if ( !dataLoaded )  return;

   pb_view->setEnabled( false );   // disable view,save buttons for now
   pb_save->setEnabled( false );

   def_local  = ! disk_controls->db();
   edata      = &dataList[ 0 ];    // point to first loaded data
   baserss    = 0;

   US_Passwd pw;
   US_DB2* dbP             = disk_controls->db()
                             ? new US_DB2( pw.getPasswd() )
                             : NULL;

   // Get speed steps from disk or DB experiment
   if ( dbP != NULL )
   {  // Fetch the speed steps for the experiment from the database
      QStringList query;
      QString     expID;
      int         idExp  = 0;
      query << "get_experiment_info_by_runID"
            << runID
            << QString::number( US_Settings::us_inv_ID() );
      dbP->query( query );

      if ( dbP->lastErrno() == IUS_DB2::OK )
      {
        dbP->next();
        idExp              = dbP->value( 1 ).toInt();
        US_SimulationParameters::speedstepsFromDB( dbP, idExp, speed_steps );
DbgLv(1) << "SS: ss count" << speed_steps.count() << "idExp" << idExp;
if (speed_steps.count()>0 )
DbgLv(1) << "SS:  ss0 w2tfirst w2tlast timefirst timelast"
   << speed_steps[0].w2t_first << speed_steps[0].w2t_last
   << speed_steps[0].time_first << speed_steps[0].time_last;
      }
   }

   else
   {  // Read run experiment file and parse out speed steps
      QString expfpath = directory + "/" + runID + "."
                       + edata->dataType + ".xml";
DbgLv(1) << "LD: expf path" << expfpath;
      QFile xfi( expfpath )
         ;
      if ( xfi.open( QIODevice::ReadOnly ) )
      {  // Read and parse "<speedstep>" lines in the XML
         QXmlStreamReader xmli( &xfi );

         while ( ! xmli.atEnd() )
         {
            xmli.readNext();

            if ( xmli.isStartElement()  &&  xmli.name() == "speedstep" )
            {
               SP_SPEEDPROFILE  sp;
               US_SimulationParameters::speedstepFromXml( xmli, sp );
               speed_steps << sp;
DbgLv(1) << "LD:  sp: rotspeed" << sp.rotorspeed << "t1" << sp.time_first;
            }
         }

         xfi.close();
      }
   }

   int ntriples  = triples.count();
   int nssp      = speed_steps.count();

   if ( nssp > 0 )
   {
      int stm1      = speed_steps[ nssp - 1 ].time_first;
      int stm2      = speed_steps[ nssp - 1 ].time_last;

      for ( int ds = 0; ds < ntriples; ds++ )
      {  // Scan data time ranges and compare to experiment speed steps
         edata         = &dataList[ ds ];
         int lesc      = edata->scanCount() - 1;
         int etm1      = edata->scanData[    0 ].seconds;
         int etm2      = edata->scanData[ lesc ].seconds;

         if ( etm1 < stm1  ||  etm2 > stm2 )
         {  // Data times beyond speed step ranges, so flag use of data ranges
            nssp          = 0;
            speed_steps.clear();
            break;
         }
      }
   }

   exp_steps  = ( nssp > 0 );    // Flag use of experiment speed steps
   dat_steps  = ! exp_steps;     // Flag use of each data's speed steps
}

// plot the data
void US_pcsa::data_plot( void )
{
   // Disable base2 cyan boundary portion
   ct_boundaryPercent->setValue( 300.0 );
   ct_boundaryPos    ->setValue( -50.0 );

DbgLv(1) << "Data Plot by Base";
   US_AnalysisBase2::data_plot();      // plot experiment data
DbgLv(1) << "Data Plot from Base";

   pb_fitcntl->setEnabled( true );
   ct_from   ->setEnabled( true );
   ct_to     ->setEnabled( true );

   if ( ! dataLoaded  ||
        sdata.scanData.size() != edata->scanData.size() )
      return;

   // set up to plot simulation data and residuals
   int nscans  = edata->scanCount();
   int npoints = edata->pointCount();
   int count   = ( npoints > nscans ) ? npoints : nscans;

   QVector< double > rvec( count, 0.0 );
   QVector< double > vvec( count, 0.0 );
   double* ra = rvec.data();
   double* va = vvec.data();
   QString title;
   QwtPlotCurve* cc;
   QPen pen_red(  Qt::red );
   QPen pen_cyan( Qt::cyan );
   QPen pen_plot( Qt::green );

   bool have_ri = ri_noise.count > 0;
   bool have_ti = ti_noise.count > 0;
   double rl = edata->radius( 0 );
   double vh = edata->value( 0, 0 ) * 0.05;
   rl       -= 0.05;
   vh       += ( vh - edata->value( 0, 0 ) ) * 0.05;

   // plot the simulation data in red on top of experiment data
   for ( int ii = 0; ii < nscans; ii++ )
   {
      if ( excludedScans.contains( ii ) ) continue;

      int    kk  = 0;
      double rr  = 0.0;
      double vv  = 0.0;
      double rn  = have_ri ? ri_noise.values[ ii ] : 0.0;

      for ( int jj = 0; jj < npoints; jj++ )
      {
         double tn  = have_ti ? ti_noise.values[ jj ] : 0.0;
         rr         = sdata.radius( jj );
         vv         = sdata.value( ii, jj++ ) + rn + tn;
         if ( rr > rl )
         {
            ra[ kk   ] = rr;
            va[ kk++ ] = vv;
         }
      }
      title = "SimCurve " + QString::number( ii );
      cc    = us_curve( data_plot2, title );
      cc->setPen    ( pen_red );
      cc->setSamples( ra, va, kk );
   }

   data_plot2->replot();           // replot combined exper,simul data

   dataPlotClear( data_plot1 );

   us_grid( data_plot1 );
   data_plot1->setAxisTitle( QwtPlot::xBottom, tr( "Radius (cm)" ) );
   data_plot1->setAxisTitle( QwtPlot::yLeft,   tr( "OD Difference" ) );
   double vari  = 0.0;

   // build vector of radius values
   for ( int jj = 0; jj < npoints; jj++ )
      ra[ jj ]  = sdata.radius( jj );

   // build and plot residual points
   for ( int ii = 0; ii < nscans; ii++ )
   {
      double rinoi = have_ri ? ri_noise.values[ ii ] : 0.0;

      for ( int jj = 0; jj < npoints; jj++ )
      {
         double tinoi = have_ti ? ti_noise.values[ jj ] : 0.0;
         double evalu = edata->value( ii, jj )
                      - sdata .value( ii, jj ) - tinoi - rinoi;
         va[ jj ]     = evalu;
         vari        += sq( evalu );
      }

      // plot dots of residuals at current scan
      title    = "resids " + QString::number( ii );
      cc       = us_curve( data_plot1, title );
      cc->setPen    ( pen_plot );
      cc->setStyle  ( QwtPlotCurve::Dots );
      cc->setSamples( ra, va, npoints );
   }

   data_plot1->setAxisAutoScale( QwtPlot::xBottom );
   data_plot1->setAxisAutoScale( QwtPlot::yLeft   );
   data_plot1->setTitle( tr( "Residuals" ) );

   // plot zero line through residuals plot
   double xlo = edata->radius(           0 ) - 0.05;
   double xhi = edata->radius( npoints - 1 ) + 0.05;
   ra[ 0 ]    = xlo;
   ra[ 1 ]    = xhi;
   va[ 0 ]    = 0.0;
   va[ 1 ]    = 0.0;
   cc         = us_curve( data_plot1, "zero-line" );
   cc->setPen    ( QPen( QBrush( Qt::red ), 2 ) );
   cc->setSamples( ra, va, 2 );

   // draw the plot
   data_plot1->setAxisScale( QwtPlot::xBottom, xlo, xhi );
   data_plot1->replot();

   // report on variance and rmsd
   vari    /= (double)( nscans * npoints );
   rmsd     = sqrt( vari );
   le_vari->setText( QString::number( vari ) );
   le_rmsd->setText( QString::number( rmsd ) );
DbgLv(1) << "Data Plot VARI" << vari;
}

// view data report
void US_pcsa::view( void )
{
   // Create the report text
   QString rtext;
   QTextStream ts( &rtext );
   write_report( ts );

   // Create US_Editor and display report
   if ( te_results == NULL )
   {
      te_results = new US_Editor( US_Editor::DEFAULT, true, QString(), this );
      te_results->resize( 820, 700 );
      QString auto_positioning = US_Settings::debug_value("auto_positioning");
      if ( global_positioning && !auto_positioning.isEmpty() && auto_positioning.toLower() == "true" )
      {
         QPoint p = g.global_position();
         te_results->move( p.x() + 30, p.y() + 30 );
      }
      te_results->e->setFont( QFont( US_GuiSettings::fontFamily(),
                                     US_GuiSettings::fontSize() ) );
   }

   te_results->e->setHtml( rtext );
   te_results->show();
}

// Save data (model,noise), report, and PNG image files
void US_pcsa::save( void )
{
DbgLv(1) << "SV: IN";
   QString analysisDate = QDateTime::currentDateTime().toUTC()
                          .toString( "yyMMddhhmm" );
   QString reqGUID      = US_Util::new_guid();
   QString runID        = edata->runID;
   QString editID       = edata->editID.startsWith( "20" ) ?
                          edata->editID.mid( 2 ) :
                          edata->editID;
   QString dates        = "e" + editID + "_a" + analysisDate;
   int     mciters      = mrecs_mc.size();
   QString analysisType = "PCSA";
DbgLv(1) << "SV: model_stats size" << model_stats.size();
   QString curvType     = model_stats[ 1 ];
   if ( curvType.contains( "Straight L" ) )
      analysisType      = "PCSA-SL";
   else if ( curvType.contains( "Increasing Sig" ) )
      analysisType      = "PCSA-IS";
   else if ( curvType.contains( "Decreasing Sig" ) )
      analysisType      = "PCSA-DS";
   else if ( curvType.contains( "Horizontal L" ) )
      analysisType      = "PCSA-HL";
   else if ( curvType.contains( "Second-Order" ) )
      analysisType      = "PCSA-2O";

   if ( model.alphaRP != 0.0 )
      analysisType      = analysisType + "-TR";

   if ( mciters > 1 )
      analysisType      = analysisType + "-MC";
DbgLv(1) << "SV: analysisType" << analysisType;

   QString requestID    = "local";
   QString tripleID     = edata->cell + edata->channel + edata->wavelength; 
   QString analysisID   = dates + "_" + analysisType + "_" + requestID + "_";
   QString dext         = "." + tripleID;
   QString dext2        = ".e" + editID + "-" + dext.mid( 1 );
   QString descbase     = runID + "." + tripleID + "." + analysisID;

   QString reppath  = US_Settings::reportDir();
   QString respath  = US_Settings::resultDir();
   QString tmppath  = US_Settings::tmpDir() + "/";
   QString mdlpath;
   QString noipath;
   int     knois    = qMin( ti_noise.count, 1 ) 
                    + qMin( ri_noise.count, 1 );  // noise files per model
   double  meniscus = edata->meniscus;
   double  dwavelen = edata->wavelength.toDouble();
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   // Test existence or create needed subdirectories
   if ( ! mkdir( reppath, runID ) )
   {
      qDebug() << "*** Unable to create or find the report directory ***";
      return;
   }

   if ( ! US_Model::model_path( mdlpath ) )
   {
      qDebug() << "*** Unable to create or find the model directory ***";
      return;
   }

   if ( knois > 0  && ! US_Noise::noise_path( noipath ) )
   {
      qDebug() << "*** Unable to create or find the noise directory ***";
      return;
   }

   if ( model.components.size() == 0 )
   {
      QMessageBox::critical( this, tr( "Zero-Component Model" ),
         tr( "*ERROR* The model you are attempting to save\n"
             "has no components. No data was saved." ) );
      return;
   }

   // Save the model and any noise file(s)

   US_Passwd   pw;
   IUS_DB2*     dbP      = def_local ? NULL : new US_DB2( pw.getPasswd() );
   QDir        dirm( mdlpath );
   QDir        dirn( noipath );
   mdlpath             += "/";
   noipath             += "/";
   QStringList mfilt( "M*.xml" );
   QStringList nfilt( "N*.xml" );
   QStringList mdnams   =  dirm.entryList( mfilt, QDir::Files, QDir::Name );
   QStringList ndnams   =  dirn.entryList( nfilt, QDir::Files, QDir::Name );
   QStringList mnames;
   QStringList nnames;
   QStringList tnames;
   QString     mname    = "M0000000.xml";
   QString     tname    = mname;
   QString     nname    = "N0000000.xml";
   int         indx     = 1;
   int         kmodels  = 0;
   int         knoises  = 0;
   bool        have_ti  = ( ti_noise.count > 0 );
   bool        have_ri  = ( ri_noise.count > 0 );

   while( indx > 0 )
   {  // build a list of available model file names
      mname = "M" + QString::asprintf( "%07i", indx++ ) + ".xml";
      if ( ! mdnams.contains( mname ) )
      {  // Add to the list of new-name models
         mnames << mname;
DbgLv(1) << "SV: kmodels mciters" << kmodels << mciters << "mname" << mname;

         if ( ++kmodels >= mciters )
            break;
      }
   }
DbgLv(1) << "SV: mnames size" << mnames.size();

   indx   = 1;

   while( indx > 0 )
   {  // build a list of available noise file names
      nname = "N" + QString::asprintf( "%07i", indx++ ) + ".xml";
      if ( ! ndnams.contains( nname ) )
      {  // add to the list of new-name noises
         nnames << nname;
         if ( ++knoises >= knois )
            break;
      }
   }
double tino = ti_noise.count > 0 ? ti_noise.values[0] : 0.0;
double tini = ti_noise_in.count > 0 ? ti_noise_in.values[0] : 0.0;
double rino = ri_noise.count > 0 ? ri_noise.values[0] : 0.0;
double rini = ri_noise_in.count > 0 ? ri_noise_in.values[0] : 0.0;
DbgLv(1) << "SV: Pre-sum tno tni" << tino << tini << "rno rni" << rino << rini;

   // Output model and noises
DbgLv(1) << "SV: mrecs size" << mrecs.size();
   double  variance  = mrecs[ 0 ].variance;
   QString iterID    = "i01";
DbgLv(1) << "SV: variance" << variance;

   if ( mciters < 2 )
   {  // Output the single non-MC model
DbgLv(1) << "SV: non-MC model ncomp" << model.components.size();
      model.description = descbase + iterID + ".model";
      mname             = mdlpath + mnames[ 0 ];
      model.modelGUID   = US_Util::new_guid();
      model.editGUID    = edata->editGUID;
      model.requestGUID = reqGUID;
      model.analysis    = US_Model::PCSA;
      model.variance    = variance;
      model.meniscus    = meniscus;
      model.wavelength  = dwavelen;
      model.dataDescrip = edata->description;

      for ( int cc = 0; cc < model.components.size(); cc++ )
         model.components[ cc ].name = QString::asprintf( "SC%04d", cc + 1 );

      // Output the model
      if ( dbP != NULL )
         model.write( dbP );
      else
         model.write( mname );
   }

   else
   {  // Output the individual models of a MonteCarlo set
DbgLv(1) << "SV: MC models  mciters" << mciters;
      for ( int jmc = 0; jmc < mciters; jmc++ )
      {
         iterID            = QString::asprintf( "mc%04d", jmc + 1 );
         model             = mrecs_mc[ jmc ].model;
         model.description = descbase + iterID + ".model";
         tname             = tmppath + descbase + iterID + ".mdl.tmp";
         model.modelGUID   = US_Util::new_guid();
         model.editGUID    = edata->editGUID;
         model.requestGUID = reqGUID;
         model.analysis    = US_Model::PCSA;

DbgLv(1) << "SV: MC models   jmc" << jmc << "write" << tname;
         // Output the model
         model.write( tname );

         tnames << tname;
      }

      // Now output the composite model
      tname             = US_Model::composite_mc_file( tnames, true );
      mname             = mdlpath + mnames[ 0 ];
DbgLv(1) << "SV: MC models     write complete  mname" << mname;
      QFile tfile( tname );
      if ( dbP != NULL )
      {  // DB:  load, write to DB, delete
         model.load( tname );
         model.description = QString( tname ).section( "/", -1, -1 )
                             .replace( ".model.xml", ".model" )
                             .replace( ".mdl.tmp",   ".model" );
         model.write( dbP );
         tfile.remove();
DbgLv(1) << "SV: MC models     write DB from tname" << tname;
      }

      else
      {  // Local: rename created file to models directory
         tfile.rename( mname );
DbgLv(1) << "SV: MC models     write Local by rename tname" << tname;
      }
   }

   int kk  = 0;
   int err = 0;

   if ( have_ti )
   {  // output a TI noise
      ti_noise.description = descbase + iterID + ".ti_noise";
      ti_noise.type        = US_Noise::TI;
      ti_noise.modelGUID   = model.modelGUID;
      ti_noise.noiseGUID   = US_Util::new_guid();
      nname                = noipath + nnames[ kk++ ];
      int nicount          = ti_noise_in.count;
DbgLv(1) << "SV:  TI nicount" << nicount;

DbgLv(1) << "SV: Pre-sum tno tni"
 << ti_noise.values[0] << ti_noise_in.values[0] << "counts sizes"
 << ti_noise.count << nicount << ti_noise.values.size()
 << ti_noise_in.values.size();
      if ( nicount > 0 )   // Sum in any input noise
         ti_noise.sum_noise( ti_noise_in, true );
DbgLv(1) << "SV:  Post-sum tno" << ti_noise.values[0];

      err = ti_noise.write( nname );
      if ( err != IUS_DB2::OK )
         qDebug() << "*ERROR* writing noise" << nname;

      if ( dbP != NULL )
      {
         err = ti_noise.write( dbP );
         if ( err != IUS_DB2::OK )
            qDebug() << "*ERROR* writing noise to DB" << ti_noise.description;
      }

      if ( nicount > 0 )   // Remove input noise in case re-plotted
      {
         US_Noise noise_rmv = ti_noise_in;

         for ( int kk = 0; kk < nicount; kk++ )
            noise_rmv.values[ kk ] *= -1.0;

         ti_noise.sum_noise( noise_rmv, true );
      }
   }

   if ( have_ri )
   {  // output an RI noise
      ri_noise.description = descbase + iterID + ".ri_noise";
      ri_noise.type        = US_Noise::RI;
      ri_noise.modelGUID   = model.modelGUID;
      ri_noise.noiseGUID   = US_Util::new_guid();
      nname                = noipath + nnames[ kk++ ];
      int nicount          = ri_noise_in.count;
DbgLv(1) << "SV:  RI nicount" << nicount;

      if ( nicount > 0 )   // Sum in any input noise
         ri_noise.sum_noise( ri_noise_in, true );
DbgLv(1) << "SV:  Post-sum rno" << ri_noise.values[0];

      err = ri_noise.write( nname );
      if ( err != IUS_DB2::OK )
         qDebug() << "*ERROR* writing noise" << nname;

      if ( dbP != NULL )
      {
         err = ri_noise.write( dbP );
         if ( err != IUS_DB2::OK )
            qDebug() << "*ERROR* writing noise to DB" << ri_noise.description;
      }

      if ( nicount > 0 )   // Remove input noise in case re-plotted
      {
         US_Noise noise_rmv = ri_noise_in;

         for ( int kk = 0; kk < nicount; kk++ )
            noise_rmv.values[ kk ] *= -1.0;

         ri_noise.sum_noise( noise_rmv, true );
      }
   }
tino = ti_noise.count > 0 ? ti_noise.values[0] : 0.0;
rino = ri_noise.count > 0 ? ri_noise.values[0] : 0.0;
DbgLv(1) << "SV:  Post-write tno rno" << tino << rino;
 
   if ( dbP != NULL )
   {
      delete dbP;
      dbP = NULL;
   }

   reppath           = reppath + "/" + runID + "/";
   respath           = respath + "/" + runID + "/";
   QString filebase  = reppath + analysisType + dext + ".";
   QString htmlFile  = filebase + "report.html";
   QString plot1File = filebase + "velocity.svgz";
   QString plot2File = filebase + "residuals.png";
   QString plot3File = filebase + "rbitmap.png";
   QString plot4File = filebase + "mlines.png";
   QString plot5File = filebase + "velocity_nc.svgz";
   QString ptmp4File = tmppath  + "PCSA" + dext + ".mlines."
                       + QString::number( getpid() ) + ".png";
DbgLv(1) << "mlines ptmp4File" << ptmp4File;
   QString analynode = "/" + analysisType + ".";
   QString dsinfFile = QString( filebase ).replace( analynode, "/dsinfo." )
                       + "dataset_info.html";

   // Write HTML report file
   QFile rep_f( htmlFile );

   if ( ! rep_f.open( QIODevice::WriteOnly | QIODevice::Text ) )
      return;

   QTextStream ts( &rep_f );
   write_report( ts );
   rep_f.close();

   // Write a general dataset information html file
   write_dset_report( dsinfFile );

   // Write plots
   if ( resplotd == 0 )
   {
      resplotd = new US_ResidPlotPc( this );
      resplotd->move( rbd_pos );
      resplotd->setVisible( true );
      connect( resplotd, SIGNAL( destroyed   ( QObject *) ),
               this,     SLOT(   child_closed( QObject* ) ) );
   }

   write_plot( plot1File, data_plot2 );
   write_plot( plot2File, resplotd->rp_data_plot2() );
   write_bmap( plot3File );
   write_plot( plot5File, resplotd->rp_data_plot1() );

   QFile::remove( plot4File );
   if ( QFile::copy  ( ptmp4File, plot4File ) )
      QFile::remove( ptmp4File );
   
   // use a dialog to tell the user what we've output
   QString wmsg = tr( "Wrote:\n" );
   wmsg        += mname + "\n";                 // List the model file

   if ( knois > 0 )
   {
      nname  = noipath + nnames[ 0 ];           // List 1st noise file(s)
      wmsg   = wmsg + nname + "\n";

      if ( knois > 1 )
      {
         nname  = noipath + nnames[ 1 ];
         wmsg   = wmsg + nname + "\n";
      }
   }

   // list report and plot files
   wmsg = wmsg + htmlFile  + "\n"
               + dsinfFile + "\n"
               + plot1File + "\n"
               + plot2File + "\n"
               + plot3File + "\n"
               + plot4File + "\n"
               + plot5File + "\n";
   QStringList repfiles;
   update_filelist( repfiles, htmlFile  );
   update_filelist( repfiles, dsinfFile );
   update_filelist( repfiles, plot1File );
   update_filelist( repfiles, plot2File );
   update_filelist( repfiles, plot3File );
   update_filelist( repfiles, plot4File );
   update_filelist( repfiles, plot5File );

   if ( disk_controls->db() )
   {  // Write report files to the database
      reportFilesToDB( repfiles );

      wmsg += tr( "\nReport files were also saved to the database." );
   }

   QApplication::restoreOverrideCursor();
   QMessageBox::information( this, tr( "Successfully Written" ), wmsg );
}

// Return pointer to main window edited data
US_DataIO::EditedData* US_pcsa::mw_editdata()
{
   int drow = lw_triples->currentRow();
   edata    = ( drow >= 0 ) ? &dataList[ drow ] : 0;

   return edata;
}

// Return pointers to main window data and GUI elements

US_DataIO::RawData*       US_pcsa::mw_simdata()      { return &sdata;         }
US_DataIO::RawData*       US_pcsa::mw_resdata()      { return &rdata;         }
QList< int >*             US_pcsa::mw_excllist()     { return &excludedScans; }
US_Model*                 US_pcsa::mw_model()        { return &model;         }
US_Noise*                 US_pcsa::mw_ti_noise()     { return &ti_noise;      }
US_Noise*                 US_pcsa::mw_ri_noise()     { return &ri_noise;      }
QPointer< QTextEdit   >   US_pcsa::mw_status_text()  { return te_status;      }
QStringList*              US_pcsa::mw_model_stats()  { return &model_stats;   }
QVector< US_ModelRecord >* US_pcsa::mw_mrecs()       { return &mrecs;         }
QVector< US_ModelRecord >* US_pcsa::mw_mrecs_mc()    { return &mrecs_mc;      }
int*                      US_pcsa::mw_base_rss()     { return &baserss;       }

// Open residuals plot window
void US_pcsa::open_resplot()
{
   if ( resplotd )
   {
      rbd_pos  = resplotd->pos();
      resplotd->close();
   }
   else
      rbd_pos  = this->pos() + QPoint(  100, 100 );

   resplotd = new US_ResidPlotPc( this );
   resplotd->move( rbd_pos );
   resplotd->setVisible( true );
   connect( resplotd, SIGNAL( destroyed   ( QObject *) ),
            this,     SLOT(   child_closed( QObject* ) ) );
}

// Open 3-D plot control window
void US_pcsa::open_3dplot()
{
   if ( eplotcd )
   {
      epd_pos  = eplotcd->pos();
      eplotcd->close();
   }
   else
      epd_pos  = this->pos() + QPoint(  400, 200 );

   eplotcd = new US_PlotControlPc( this, &model );
   eplotcd->move( epd_pos );
   eplotcd->show();
   connect( eplotcd,  SIGNAL( destroyed   ( QObject *) ),
            this,     SLOT(   child_closed( QObject* ) ) );
}

// Open fit analysis control window
void US_pcsa::open_fitcntl()
{
   int    drow     = lw_triples->currentRow();
   if ( drow < 0 )   return;

   edata           = &dataList[ drow ];
   double avTemp   = edata->average_temperature();
   double vbar20   = US_Math2::calcCommonVbar( solution_rec, 20.0   );
   double vbartb   = US_Math2::calcCommonVbar( solution_rec, avTemp );
   double buoy     = 1.0 - vbar20 * DENS_20W;

   if ( buoy <= 0.0 )
   {
      QMessageBox::critical( this, tr( "Negative Buoyancy Implied" ),
         tr( "The current vbar20 value (%1) implies a buoyancy\n"
             "value (%2) that is non-positive.\n\n"
             "PCSA cannot proceed with this value. Click on the\n"
             "<Solution> button and change the vbar20 value.\n"
             "Note that the Solution may be accepted without being saved.\n"
             "Include negative values in the sedimentation coefficient\n"
             "range to represent floating data." ).arg( vbar20 ).arg( buoy ) );
      return;
   }

   US_Math2::SolutionData sd;
   sd.density      = density;
   sd.viscosity    = viscosity;
   sd.vbar20       = vbar20;
   sd.vbar         = vbartb;
   sd.manual       = manual;
   US_Math2::data_correction( avTemp, sd );

   US_Passwd pw;
   IUS_DB2* dbP     = disk_controls->db() ? new US_DB2( pw.getPasswd() ) : NULL;

   dset.simparams.initFromData( dbP, dataList[ drow ], dat_steps );

   if ( exp_steps )
      dset.simparams.speed_step  = speed_steps;

   dset.requestID          = disk_controls->db() ? "DB" : "Disk";
   dset.run_data           = dataList[ drow ];
   int atrx                = 0;
   int atry                = 1;
   int atrz                = 3;
   dset.solute_type        = ( atrx << 6 ) | ( atry << 3 ) | atrz;
   dset.viscosity          = viscosity;
   dset.density            = density;
   dset.manual             = manual;
   dset.temperature        = avTemp;
   dset.vbar20             = vbar20;
   dset.vbartb             = vbartb;
   dset.s20w_correction    = sd.s20w_correction;
   dset.D20w_correction    = sd.D20w_correction;
DbgLv(1) << "Bottom" << dset.simparams.bottom << "rotorcoeffs"
 << dset.simparams.rotorcoeffs[0] << dset.simparams.rotorcoeffs[1];

   if ( dbP != NULL )
   {
      delete dbP;
      dbP    = NULL;
   }

   if ( analcd != 0 )
   {
      acd_pos  = analcd->pos();
      analcd->close();
   }
   else
      acd_pos  = this->pos() + QPoint(  500,  50 );

   analcd  = new US_AnalysisControlPc( dsets, this );
   analcd->move( acd_pos );
   analcd->show();
   connect( analcd,   SIGNAL( destroyed   ( QObject *) ),
            this,     SLOT(   child_closed( QObject* ) ) );
   qApp->processEvents();
}

// Distribution information HTML string
QString US_pcsa::distrib_info()
{
   int     ncomp   = model.components.size();
DbgLv(1) << "distrinfo: ncomp" << ncomp;
   QString maDesc  = model.description;
   QString runID   = edata->runID;
   
   if ( ncomp == 0 )
      return "";

   if ( maDesc.startsWith( runID ) )
   {  // Saved model:  get analysis description from model description
      maDesc          = maDesc.section( ".", -2, -2 ).section( "_", 1, -1 );
   }
   else
   {  // No saved model yet:  compose analysis description
      maDesc          = "a" + QDateTime::currentDateTime().toUTC()
         .toString( "yyMMddhhmm" ) + "_PCSA_local_01";
   }

   QString mstr = "\n" + indent( 4 )
      + tr( "<h3>Data Analysis Settings:</h3>\n" )
      + indent( 4 ) + "<table>\n";

   mstr += table_row( tr( "Model Analysis:" ),
                      maDesc );
   mstr += table_row( tr( "Number of Components:" ),
                      QString::number( ncomp ) );
   mstr += table_row( tr( "Residual RMS Deviation:" ),
                      QString::number( rmsd )  );

   double sum_mw  = 0.0;
   double sum_s   = 0.0;
   double sum_D   = 0.0;
   double sum_c   = 0.0;
   double mink    = 1e+99;
   double maxk    = -1e+99;
   double minv    = 1e+99;
   double maxv    = -1e+99;

   for ( int ii = 0; ii < ncomp; ii++ )
   {
      double conc = model.components[ ii ].signal_concentration;
      sum_c      += conc;
      sum_mw     += model.components[ ii ].mw * conc;
      sum_s      += model.components[ ii ].s  * conc;
      sum_D      += model.components[ ii ].D  * conc;
      mink        = qMin( mink, model.components[ ii ].f_f0   );
      maxk        = qMax( maxk, model.components[ ii ].f_f0   );
      minv        = qMin( minv, model.components[ ii ].vbar20 );
      maxv        = qMax( maxv, model.components[ ii ].vbar20 );
   }

   bool cnstvb    = ( ( maxk - mink ) / qAbs( maxk )
                    > ( maxv - minv ) / qAbs( maxv ) );

   mstr += table_row( tr( "Weight Average s20,W:" ),
                      QString::asprintf( "%6.4e", ( sum_s  / sum_c ) ) );
   mstr += table_row( tr( "Weight Average D20,W:" ),
                      QString::asprintf( "%6.4e", ( sum_D  / sum_c ) ) );
   mstr += table_row( tr( "W.A. Molecular Weight:" ),
                      QString::asprintf( "%6.4e", ( sum_mw / sum_c ) ) );
   mstr += table_row( tr( "Total Concentration:" ),
                      QString::asprintf( "%6.4e", sum_c ) );

   if ( cnstvb )
      mstr += table_row( tr( "Constant Vbar at 20" ) + DEGC + ":",
                         QString().number( maxv ) );
   else
      mstr += table_row( tr( "Constant f/f0:" ),
                         QString().number( maxk ) );

   mstr += indent( 4 ) + "</table>\n\n";

   mstr += indent( 4 ) + tr( "<h3>Distribution Information:</h3>\n" )
      + indent( 4 ) + "<table>\n";

   if ( cnstvb )
      mstr += table_row( tr( "Molecular Wt." ), tr( "S Apparent" ),
                         tr( "S 20,W" ),        tr( "D Apparent" ),
                         tr( "D 20,W" ),        tr( "f / f0" ),
                         tr( "Concentration" ) );
   else
      mstr += table_row( tr( "Molecular Wt." ), tr( "S Apparent" ),
                         tr( "S 20,W" ),        tr( "D Apparent" ),
                         tr( "D 20,W" ),        tr( "Vbar20" ),
                         tr( "Concentration" ) );

   int    drow     = lw_triples->currentRow();
   edata           = &dataList[ drow ];
   double avTemp   = edata->average_temperature();
   double vbar20   = US_Math2::calcCommonVbar( solution_rec, 20.0   );
   double vbartb   = US_Math2::calcCommonVbar( solution_rec, avTemp );
   US_Math2::SolutionData sd;
   sd.density      = density;
   sd.viscosity    = viscosity;
   sd.manual       = manual;
   sd.vbar20       = vbar20;
   sd.vbar         = vbartb;
   US_Math2::data_correction( avTemp, sd );

   for ( int ii = 0; ii < ncomp; ii++ )
   {
      double conc = model.components[ ii ].signal_concentration;
      double perc = 100.0 * conc / sum_c;
      double s_ap = model.components[ ii ].s;
      double D_ap = model.components[ ii ].D;
      double f_f0 = model.components[ ii ].f_f0;

      if ( !cnstvb )
      {
         vbar20      = model.components[ ii ].vbar20;
         f_f0        = vbar20;
         sd.vbar20   = vbar20;
         sd.vbar     = US_Math2::adjust_vbar20( vbar20, avTemp );
         US_Math2::data_correction( avTemp, sd );
      }

      s_ap       /= sd.s20w_correction;
      D_ap       /= sd.D20w_correction;

      mstr       += table_row(
            QString::asprintf( "%10.4e", model.components[ ii ].mw ),
            QString::asprintf( "%10.4e", s_ap                      ),
            QString::asprintf( "%10.4e", model.components[ ii ].s  ),
            QString::asprintf( "%10.4e", D_ap                      ),
            QString::asprintf( "%10.4e", model.components[ ii ].D  ),
            QString::asprintf( "%10.4e", f_f0                      ),
            QString::asprintf( "%10.4e (%5.2f %%)", conc, perc     ) );
   }

   mstr += indent( 4 ) + "</table>\n";
   
   return mstr;
}

// Model statistics HTML string
QString US_pcsa::model_statistics()
{
DbgLv(1) << "ModStats0" << model_stats[ 0 ];
DbgLv(1) << "ModStats1" << model_stats[ 1 ];
   QString mstr = "\n" + indent( 4 )
      + tr( "<h3>Model Statistics:</h3>\n" )
      + indent( 4 ) + "<table>\n";

   for ( int ii = 0; ii < model_stats.size(); ii += 2 )
   {
      mstr += table_row( model_stats[ ii ], model_stats[ ii + 1 ] );
   }

   mstr += indent( 4 ) + "</table>\n";
   
   return mstr;
}

// Write HTML report file
void US_pcsa::write_report( QTextStream& ts )
{
   QString curvtype = model_stats[ 1 ];
   QString hdr      = tr( "Parametrically Constrained Spectrum Analysis" )
                      + "<br/>( " + curvtype + " )";

   if ( model.alphaRP > 0.0 )
      hdr             += "<br/>- Tikhonov Regularization";

   ts << html_header( QString( "US_pcsa" ), hdr, edata );
   ts << model_statistics();
   ts << distrib_info();
   ts << indent( 2 ) + "</body>\n</html>\n";
}

// Write resids bitmap plot file
void US_pcsa::write_bmap( const QString plotFile )
{
   // Generate the residuals array
   bool have_ri = ri_noise.count > 0;
   bool have_ti = ti_noise.count > 0;
   int  nscans  = edata->scanCount();
   int  npoints = edata->pointCount();
   QVector< double >            resscn( npoints );
   QVector< QVector< double > > resids( nscans  );

   for ( int ii = 0; ii < nscans; ii++ )
   {
      double rnoi  = have_ri ? ri_noise.values[ ii ] : 0.0;

      for ( int jj = 0; jj < npoints; jj++ )
      {
         double tnoi  = have_ti ? ti_noise.values[ jj ] : 0.0;
         resscn[ jj ] = edata->value( ii, jj )
                      - sdata.value(  ii, jj ) - rnoi - tnoi;
      }

      resids[ ii ] = resscn;
   }

   // Generate the residuals bitmap plot
   US_ResidsBitmap* resbmap = new US_ResidsBitmap( resids );
#if QT_VERSION > 0x050000
   QPixmap pixmap  = ((QWidget*)resbmap)->grab();
#else
   QPixmap pixmap  = QPixmap::grabWidget( resbmap, 0, 0,
                        resbmap->width(), resbmap->height() );
#endif

   // Save the pixmap to the specified file
   if ( ! pixmap.save( plotFile ) )
      qDebug() << "*ERROR* Unable to write file" << plotFile;

   resbmap->close();
   resbmap = 0;
}

// New triple selected
void US_pcsa::new_triple( int index )
{
   edata = &dataList[ index ];

   // Restore pure data type string (other values added in pcsa processing)
   edata->dataType = edata->dataType.section( " ", 0, 0 );

   sdata.scanData.clear();                 // Clear simulation and upper plot
   dataPlotClear( data_plot1 );

   // Temporarily restore loaded noise vectors from triples vectors
   ti_noise           = tinoises[ index ];
   ri_noise           = rinoises[ index ];

   US_AnalysisBase2::new_triple( index );  // New triple as in any analysis

   // Move any loaded noise vectors to the "in" versions
   ti_noise_in        = ti_noise;
   ri_noise_in        = ri_noise;
   ti_noise_in.values = ti_noise.values;
   ri_noise_in.values = ri_noise.values;
   ti_noise_in.count  = ti_noise_in.values.size();
   ri_noise_in.count  = ri_noise_in.values.size();
   tinoises[ index ]  = ti_noise_in;
   rinoises[ index ]  = ri_noise_in;
   ti_noise.values.clear();
   ri_noise.values.clear();
   ti_noise.count     = 0;
   ri_noise.count     = 0;
}

// Remove any temporary plot file and close all opened windows
void US_pcsa::close( void )
{
   QString tripleID  = ( edata != 0 )
                       ?  edata->cell + edata->channel + edata->wavelength
                       : ""; 
   QString ptmp4File = US_Settings::tmpDir() + "/PCSA." + tripleID
      + ".mlines." + QString::number( getpid() ) + ".png";

   QFile tfile( ptmp4File );
   if ( tfile.exists() )
   {
      tfile.remove();
DbgLv(1) << "pcsa: removed: " << ptmp4File;
   }
DbgLv(1) << "pcsa:  close d's res epl ana" << resplotd << eplotcd << analcd;

   if ( resplotd != 0 )
      resplotd->close();
   if ( eplotcd != 0 )
      eplotcd->close();
   if ( analcd != 0 )
      analcd->close();
}

// Private slot to mark a child widgets as closed, if it has been destroyed
void US_pcsa::child_closed( QObject* o )
{
   QString oname = o->objectName();
DbgLv(1) << "pcsa:CC: d's res epl ana" << resplotd << eplotcd << analcd;

   if ( oname.contains( "AnalysisControl" ) )
      analcd    = 0;
   else if ( oname.contains( "ResidPlot" ) )
      resplotd  = 0;
   else if ( oname.contains( "PlotControl" ) )
      eplotcd   = 0;
DbgLv(1) << "pcsa:CC: return res epl ana" << resplotd << eplotcd << analcd
   << "oname" << oname;
}

