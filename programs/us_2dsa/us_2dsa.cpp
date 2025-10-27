//! \file us_2dsa.cpp

#include <QApplication>
#include <QtSvg>

#include "us_2dsa.h"
#include "us_resids_bitmap.h"
#include "us_plot_control_2d.h"
#include "us_analysis_control_2d.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_matrix.h"
#include "us_constants.h"
#include "us_analyte_gui.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_data_loader.h"
#include "us_util.h"
#include "us_investigator.h"
#include "us_noise_loader.h"
#include "us_loadable_noise.h"
#include "us_show_norm.h"
#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#endif

//! \brief Main program for us_2dsa. Loads translators and starts
//         the class US_2dsa.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_2dsa w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

// constructor, based on AnalysisBase
US_2dsa::US_2dsa() : US_AnalysisBase2()
{
   setWindowTitle( tr( "2-Dimensional Spectrum Analysis" ) );
   setObjectName( "US_2dsa" );
   dbg_level  = US_Settings::us_debug();
   // Insure working etc is populated with color maps
   clean_etc_dir();

   // Build local and 2dsa-specific GUI elements
   te_results = NULL;
   baserss    = 0;

   QLabel* lb_analysis = us_banner( tr( "Analysis Controls" ) );
   QLabel* lb_scan     = us_banner( tr( "Scan Control"       ) );

   QLabel* lb_status   = us_label ( tr( "Status\nInfo:" ) );
   te_status           = us_textedit();
   QLabel* lb_from     = us_label ( tr( "Scan focus from:" ) );
   QLabel* lb_to       = us_label ( tr( "to:"   ) );

   pb_exclude   = us_pushbutton( tr( "Exclude Scan Range" ) );
   pb_exclude->setEnabled( false );
   pb_fitcntl   = us_pushbutton( tr( "Fit Control"   ) );
   pb_plt3d     = us_pushbutton( tr( "3-D Plot"      ) );
   pb_pltres    = us_pushbutton( tr( "Residual Plot" ) );

   connect( pb_exclude, SIGNAL( clicked() ), SLOT( exclude() ) );
   connect( pb_fitcntl, SIGNAL( clicked() ), SLOT( open_fitcntl() ) );
   connect( pb_plt3d,   SIGNAL( clicked() ), SLOT( open_3dplot()  ) );
   connect( pb_pltres,  SIGNAL( clicked() ), SLOT( open_resplot() ) );
   connect( pb_close,   SIGNAL( clicked() ), SLOT( close_all()    ) );
   connect( ct_from,    SIGNAL( valueChanged( double ) ),
                        SLOT  ( exclude_from( double ) ) );
   connect( ct_to,      SIGNAL( valueChanged( double ) ),
                        SLOT  ( exclude_to  ( double ) ) );

   // To modify controls layout, first make Base elements invisible
   
   QWidget* widg;
   for ( int ii = 0; ii < controlsLayout->count(); ii++ )
   {
      if ( ( widg = controlsLayout->itemAt( ii )->widget() ) != 0 )
         widg      ->setVisible( false );
   }
   ct_from         ->setVisible(true);
   ct_to           ->setVisible(true);
   pb_exclude      ->setVisible(true);
   pb_reset_exclude->setVisible(true);
   // Effectively disable boundaries to turn off cyan portion of plot2
   ct_boundaryPercent->disconnect   ();
   ct_boundaryPercent->setRange     ( 0.0, 300.0 );
   ct_boundaryPercent->setSingleStep( 1.0 );
   ct_boundaryPercent->setValue     ( 300.0 );
   ct_boundaryPercent->setEnabled   ( false );
   ct_boundaryPos    ->disconnect   ();
   ct_boundaryPos    ->setRange     ( -50.0, 300.0 );
   ct_boundaryPos    ->setSingleStep( 1.0 );
   ct_boundaryPos    ->setValue     ( -50.0 );
   ct_boundaryPos    ->setEnabled   ( false );


   // Add variance and rmsd to parameters layout
   QLabel* lb_vari     = us_label ( tr( "Variance:" ) );
   le_vari             = us_lineedit( "0.00000", 0, true );
   QLabel* lb_rmsd     = us_label ( tr( "RMSD:" ) );
   le_rmsd             = us_lineedit( "0.00000", 0, true );
   int row   = parameterLayout->rowCount();
   parameterLayout->addWidget( lb_vari,     row,   0, 1, 1 );
   parameterLayout->addWidget( le_vari,     row,   1, 1, 1 );
   parameterLayout->addWidget( lb_rmsd,     row,   2, 1, 1 );
   parameterLayout->addWidget( le_rmsd,     row++, 3, 1, 1 );

   // Reconstruct controls layout with some 2dsa-specific elements
   row       = 0;
   controlsLayout->addWidget( lb_scan,      row++, 0, 1, 3 );
   controlsLayout->addWidget( lb_from,      row,   0, 1, 1 );
   controlsLayout->addWidget( ct_from,      row++, 1, 1, 2 );
   controlsLayout->addWidget( lb_to,        row,   0, 1, 1 );
   controlsLayout->addWidget( ct_to,        row++, 1, 1, 2 );
   controlsLayout->addWidget( pb_exclude,   row,   0, 1, 1 );
   controlsLayout->addWidget( pb_reset_exclude, row++, 1, 1, 2 );
   controlsLayout->addWidget( lb_analysis,  row++, 0, 1, 3 );

   controlsLayout->addWidget( pb_fitcntl,   row,   0, 1, 1 );
   controlsLayout->addWidget( pb_plt3d,     row,   1, 1, 1 );
   controlsLayout->addWidget( pb_pltres,    row++, 2, 1, 1 );

   controlsLayout->addWidget( lb_status,    row,   0, 1, 1 );
   controlsLayout->addWidget( te_status,    row,   1, 1, 2 );
   row      += 3;

   // Set initial status text
   te_status->setAlignment( Qt::AlignCenter | Qt::AlignVCenter );
   te_status->setText( tr(
       "Solution not initiated...\n"
       "RMSD:  0.000000,\n"
       "Variance: 0.000000e-05 .\n"
       "Iterations:  0" ) );
   us_setReadOnly( te_status, true );

   connect( pb_help,  SIGNAL( clicked() ), SLOT( help() ) );
   connect( pb_view,  SIGNAL( clicked() ), SLOT( view() ) );
   connect( pb_save,  SIGNAL( clicked() ), SLOT( save() ) );

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

// slot to handle the completion of a 2-D spectrum analysis stage
void US_2dsa::analysis_done( int updflag )
{
   if ( updflag == (-1) )
   {  // fit has been aborted or reset for new fit
      pb_view   ->setEnabled( false );
      pb_save   ->setEnabled( false );
      pb_plt3d  ->setEnabled( false );
      pb_pltres ->setEnabled( false );
      models   .clear();
      ti_noises.clear();
      ri_noises.clear();

      qApp->processEvents();
      return;
   }

   if ( updflag == (-2) )
   {  // update model,noise lists and RMSD
      models << model;

      if ( ti_noise.count > 0 )
         ti_noises << ti_noise;

      if ( ri_noise.count > 0 )
         ri_noises << ri_noise;

      QString mdesc = model.description;
      QString avari = mdesc.mid( mdesc.indexOf( "VARI=" ) + 5 );
      double  vari  = avari.section( " ", 0, 0 ).toDouble();
      double  rmsd  = sqrt( vari );
      le_vari->setText( QString::number( vari ) );
      le_rmsd->setText( QString::number( rmsd ) );
DbgLv(0) << "Analysis Done VARI" << vari << "model,noise counts"
 << models.count() << (ti_noises.count()+ri_noises.count())
      << mdesc
 << "menisc bott angle"
 << mdesc.mid( mdesc.indexOf( "MENISCUS=" ) + 9 ).section( " ", 0, 0 )
 << mdesc.mid( mdesc.indexOf( "BOTTOM=" ) + 7 ).section( " ", 0, 0 )
       << mdesc.mid( mdesc.indexOf( "ANGLE=" ) + 6 ).section( " ", 0, 0 );

      qApp->processEvents();
      return;
   }

   // if here, an analysis is all done

   bool plotdata     = ( updflag == 1 );
   bool savedata     = ( updflag == 2 );

DbgLv(1) << "Analysis Done" << updflag;
DbgLv(1) << "  model components size" << model.components.size();
DbgLv(1) << "  edat0 sdat0 rdat0 tnoi0"
 << edata->value(0,0) << sdata.value(0,0) << rdata.value(0,0)
 << ((ti_noise.count>0)?ti_noise.values[0]:0.0);

   pb_plt3d ->setEnabled( true );
   pb_pltres->setEnabled( true );
   pb_view  ->setEnabled( true );
   pb_save  ->setEnabled( true );

   // Plot data in main window
   data_plot();

   if ( plotdata )
   {  // Plot 3D and Residuals
      open_3dplot();
      open_resplot();
   }

   else if ( savedata )
   {  // Save the data and reports
      save();
   }

   // For multiple models (e.g., Fit-Meniscus) report on best
   int nmodels     = models.count();
   bool fitMeni    = ( model.global == US_Model::MENISCUS || model.global == US_Model::BOTTOM ||
   model.global == US_Model::MENIBOTT || model.global == US_Model::ANGLE || model.global == US_Model::MENIANGLE);
   if ( nmodels > 1  &&  fitMeni )
   {
      double b_rmsd   = 1.0e+99;
      double b_meni   = 0.0;
      double b_bott   = 0.0;
      double b_angle  = 0.0;

      for ( int ii = 0; ii < nmodels; ii++ )
      {
         QString mdesc = models[ ii ].description;
DbgLv(2) << "FitMens Done:  ii desc" << ii << mdesc;
         double vari   = mdesc.mid( mdesc.indexOf( "VARI=" ) + 5 )
                         .section( " ", 0, 0 ).toDouble();
         double rmsd   = sqrt( vari );

         if ( rmsd < b_rmsd )
         {
            b_rmsd        = rmsd;
            b_meni        = mdesc.mid( mdesc.indexOf( "MENISCUS=" ) + 9 )
                            .section( " ", 0, 0 ).toDouble();
            b_bott        = mdesc.mid( mdesc.indexOf( "BOTTOM=" ) + 7 )
                            .section( " ", 0, 0 ).toDouble();
            b_angle        = mdesc.mid( mdesc.indexOf( "ANGLE=" ) + 6 )
                            .section( " ", 0, 0 ).toDouble();
DbgLv(1) << "FitMens Done:    b_ rmsd,meni,bott,angle" << b_rmsd << b_meni << b_bott << b_angle
 << "  ix" << (ii+1);
         }
      }
DbgLv(1) << "FitMens Done: BEST rmsd,meniscus,bottom,angle"
 << b_rmsd << b_meni << b_bott << b_angle;

   }
}

// load the experiment data, mostly thru AnalysisBase; then disable view,save
void US_2dsa::load( void )
{
   US_AnalysisBase2::load();       // load edited experiment data

   if ( !dataLoaded )  return;

   pb_view->setEnabled( false );   // disable view,save buttons for now
   pb_save->setEnabled( false );

   loadDB     = disk_controls->db();
   edata      = &dataList[ 0 ];    // point to first loaded data
   baserss    = 0;
   speed_steps.clear();
   ti_noises  .clear();
   ri_noises  .clear();

   // Get speed steps from disk or DB experiment
   if ( loadDB )
   {  // Fetch the speed steps for the experiment from the database
      US_Passwd   pw;
      IUS_DB2*    dbP    = new US_DB2( pw.getPasswd() );
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

DbgLv(1)<< "2dsa_load: speed step count" << speed_steps.count() << "idExp" << idExp;
if ( speed_steps.count()>0 )
 DbgLv(1) << "2dsa_load:  speed step_ w2tfirst w2tlast timefirst timelast"
  << speed_steps[0].w2t_first << speed_steps[0].w2t_last
  << speed_steps[0].time_first << speed_steps[0].time_last;

         // Check out whether we need to read TimeState from the DB
         QString tmst_fpath = US_Settings::resultDir() + "/" + runID + "/"
                              + runID + ".time_state.tmst";

         bool newfile       = US_TimeState::dbSyncToLF( dbP, tmst_fpath, idExp );
DbgLv(0) << "2DS:LD: newfile" << newfile << "idExp" << idExp
 << "tmst_fpath" << tmst_fpath;
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

   int nssp      = speed_steps.count();
   int nssc      = ( nssp < 1 ) ? 0 : speed_steps[ nssp - 1 ].scans;

DbgLv(1)<< "2dsa_no. of speeds " << nssp <<"no of scans "<< nssc
 <<" triple_count= "<< lw_triples->count();

   if ( nssp > 0 )
   {  
      for  ( int ii = 0; ii < nssp ; ii++)
      {
         int stm1   = speed_steps[ ii ].time_first;
         int stm2   = speed_steps[ ii ].time_last;
         for ( int ds = 0; ds  < lw_triples->count(); ds++ )
         {
            edata      = &dataList[ ds ];
            int lesc   = edata->scanCount() - 1;
            int etm1 = edata->scanData[ 0 ].seconds;
            int etm2   = edata->scanData[ lesc ].seconds;
            if ( etm1 < stm1  ||  etm2 > stm2 )
            {  // Data times beyond speed step ranges, so flag use of data ranges
DbgLv(1) << "2dsa: Data is beyond range" << "limits from scans" << etm1 << etm2 << ds;
DbgLv(1) << "2dsa: Data is beyond range" << "limits from speed_profs " << stm1 << stm2 << ds;
               //nssp = 0 ;
               //speed_steps.clear();            
               break;
            }
         }
      }   
   }

   exp_steps     = ( nssp > 0 );      // Flag use of experiment speed steps
   edata         = &dataList[ 0 ];    // Point to first loaded data
DbgLv(1) << " 2dsa: exp_steps" << exp_steps << "nssp" << nssp ;
//*DEBUG*
for ( int js=0; js<edata->scanCount(); js++ )
{
DbgLv(1) << " 2dsa: js" << js << "secs" << edata->scanData[js].seconds
 << "speed" << edata->scanData[js].rpm;
}
//*DEBUG*
}

// plot the data
void US_2dsa::data_plot( void )
{
   // Disable base2 cyan boundary portion
   ct_boundaryPercent->setValue( 300.0 );
   ct_boundaryPos    ->setValue( -50.0 );

   US_AnalysisBase2::data_plot();      // plot experiment data

   pb_fitcntl->setEnabled( true );
   ct_from   ->setEnabled( true );
   ct_to     ->setEnabled( true );

   if ( ! dataLoaded  ||
        sdata.scanData.size() != edata->scanData.size() )
   {
DbgLv(1) << "2dsa: from data plot : data are different  edata:" << edata->scanData.size() 
 << "sdata:" << sdata.scanData.size();
      return;
   }

   // set up to plot simulation data and residuals
   int npoints = edata->pointCount();
   int nscans  = edata->scanCount();

DbgLv(1) << "edited data size " << npoints << nscans;
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
      cc->setPen( pen_red );
      cc->setSamples( ra, va, kk );
   }

   data_plot2->replot();           // replot combined exper,simul data

   dataPlotClear( data_plot1 );

   us_grid( data_plot1 );
   data_plot1->setAxisTitle( QwtPlot::xBottom, tr( "Radius (cm)" ) );
   data_plot1->setAxisTitle( QwtPlot::yLeft,   tr( "OD Difference" ) );
   double vari    = 0.0;
   int    kntva   = 0;

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
         va[ jj ]     = edata->value( ii, jj ) - sdata.value( ii, jj )
                        - tinoi - rinoi;
         vari        += sq( va[ jj ] );
         kntva++;
      }

      // plot dots of residuals at current scan
      title    = "resids " + QString::number( ii );
      cc       = us_curve( data_plot1, title );
      cc->setPen(   pen_plot );
      cc->setStyle( QwtPlotCurve::Dots );
      cc->setSamples(  ra, va, npoints );
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
   cc->setPen( QPen( QBrush( Qt::red ), 2 ) );
   cc->setSamples( ra, va, 2 );

   // draw the plot
   data_plot1->setAxisScale( QwtPlot::xBottom, xlo, xhi );
   data_plot1->replot();

   // report on variance and rmsd
   vari    /= (double)( kntva );
   rmsd     = sqrt( vari );
   le_vari->setText( QString::number( vari ) );
   le_rmsd->setText( QString::number( rmsd ) );
DbgLv(1) << "Data Plot VARI" << vari;
}

// view data report
void US_2dsa::view( void )
{
   // Create the report text
   QString rtext;
   QTextStream ts( &rtext );
   write_report( ts );

   // Create US_Editor and display report
   if ( te_results.isNull() )
   {
      te_results = new US_Editor( US_Editor::DEFAULT, true, "HTML (*.html);;Text files (*.txt)", this );
      te_results->setWindowTitle( tr( "Report: 2DSA Results" ) );
      te_results->resize( 780, 700 );
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
   te_results->setFocus();
   te_results->activateWindow();
   te_results->raise();
}

// Save data (model,noise), report, and PNG image files
void US_2dsa::save( void )
{
   QString analysisDate = QDateTime::currentDateTime().toUTC()
                          .toString( "yyMMddhhmm" );
   QString reqGUID      = US_Util::new_guid();
   QString runID        = edata->runID;
   QString editID       = ( edata->editID.length() == 12 ) ?
                          edata->editID.mid( 2 ) :
                          edata->editID;
   QString dates        = "e" + editID + "_a" + analysisDate;
   bool    cusGrid      = model.description.contains( "CUSTOMGRID" );
   bool    refIter      = model.description.contains( "REFITERS" );
DbgLv(1) << "2DSA:SV: cusGrid" << cusGrid << "desc" << model.description;
   bool    fitMeni      = ( model.global == US_Model::MENISCUS );
   bool    fitBott      = ( model.global == US_Model::BOTTOM );
   bool    fitMeBo      = ( model.global == US_Model::MENIBOTT );
   bool    fitAngle     = ( model.global == US_Model::ANGLE );
   bool    fitMeAngle   = ( model.global == US_Model::MENIANGLE );
   bool    montCar      = model.monteCarlo;
   QString analysisType = QString( cusGrid ? "2DSA-CG" : "2DSA" )
                        + QString( fitMeni ? "-FM" : "" )
                        + QString( (fitBott || fitAngle) ? "-FB" : "" )
                        + QString( (fitMeBo || fitMeAngle) ? "-FMB" : "" )
                        + QString( refIter ? "-IT" : "" )
                        + QString( montCar ? "-MC" : "" );
   QString requestID    = "local";
   QString tripleID     = edata->cell + edata->channel + edata->wavelength; 
   QString analysisID   = dates + "_" + analysisType + "_" + requestID + "_";
   QString dext         = "." + tripleID;
   QString dext2        = ".e" + editID + "-" + dext.mid( 1 );
   QString descbase     = runID + "." + tripleID + "." + analysisID;

   QString reppath  = US_Settings::reportDir();
   QString respath  = US_Settings::resultDir();
   QString mdlpath;
   QString noipath;
   int     nmodels  = models.size();             // number of models to save
   int     knois    = min( ti_noise.count, 1 ) 
                    + min( ri_noise.count, 1 );  // noise files per model
   int     nnoises  = nmodels * knois;           // number of noises to save
   double  meniscus = edata->meniscus;
   double  bottom   = edata->bottom;
   double  angle    = dset.simparams.cp_angle;
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

   // Save the model and any noise file(s)

   US_Passwd   pw;
   IUS_DB2*    dbP      = loadDB ? new US_DB2( pw.getPasswd() ): NULL;
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
   QString     nname    = "N0000000.xml";
   QString     tmppath  = US_Settings::tmpDir() + "/";
   int         indx     = 1;
   int         kmodels  = 0;
   int         knoises  = 0;
   bool        have_ti  = ( ti_noises.size() > 0 );
   bool        have_ri  = ( ri_noises.size() > 0 );

   while( indx > 0 )
   {  // build a list of available model file names
      mname = "M" + QString::asprintf( "%07i", indx++ ) + ".xml";
      if ( ! mdnams.contains( mname ) )
      {  // no name with this index exists, so add it new-name list
         mnames << mname;
         if ( ++kmodels >= nmodels )
            break;
      }
   }

   indx   = 1;

   while( indx > 0 )
   {  // build a list of available noise file names
      nname = "N" + QString::asprintf( "%07i", indx++ ) + ".xml";
      if ( ! ndnams.contains( nname ) )
      {  // add to the list of new-name noises
         nnames << nname;
         if ( ++knoises >= nnoises )
            break;
      }
   }
//double tino = ti_noise.count > 0 ? ti_noise.values[0] : 0.0;
//double tini = ti_noise_in.count > 0 ? ti_noise_in.values[0] : 0.0;
//double rino = ri_noise.count > 0 ? ri_noise.values[0] : 0.0;
//double rini = ri_noise_in.count > 0 ? ri_noise_in.values[0] : 0.0;
//DbgLv(1) << "  Pre-sum tno tni" << tino << tini << "rno rni" << rino << rini;

   for ( int jj = 0; jj < nmodels; jj++ )
   {  // loop to output models and noises
      model             = models[ jj ];         // model to output
      QString mdesc     = model.description;    // description from processor
      double  variance  = mdesc.mid( mdesc.indexOf( "VARI=" ) + 5 )
                          .section( ' ', 0, 0 ).toDouble();
      angle  = mdesc.mid( mdesc.indexOf( "ANGLE=" ) + 6 )
                          .section( ' ', 0, 0 ).toDouble();
      // create the iteration part of model description:
      // e.g.,"i01" normally; "i03-m60190" for meniscus; "mc017" for monte carlo
      QString iterID    = "i01";
      int     iterNum   = jj + 1;
      QStringList fitTypeAttr;
      fitTypeAttr << "NOTHING" << "MENISCUS" << "BOTTOM" << "ANGLE" << "VOLUME" << "SIGMA" << "DELTA"
                  << "VBAR" << "FF0" << "TEMPERATURE";
      if ( montCar )
         iterID = QString::asprintf( "mc%04d", iterNum );

      else if ( dset.simparams.primaryFit != US_SimulationParameters::NOTHING || dset.simparams.secondaryFit != US_SimulationParameters::NOTHING )
      {
         QString prim = fitTypeAttr[dset.simparams.primaryFit] + "=";
         QString sec = fitTypeAttr[dset.simparams.secondaryFit] + "=";
         meniscus          = mdesc.mid( mdesc.indexOf( prim ) + prim.length() )
                             .section( ' ', 0, 0 ).toDouble();
         bottom            = mdesc.mid( mdesc.indexOf( sec ) + sec.length() )
                             .section( ' ', 0, 0 ).toDouble();
         if ( bottom > 0.0 )
         {
            iterID.asprintf( "i%02d-m%05db%05d", iterNum, qRound( meniscus * 10000 ),
                             qRound( bottom * 10000 ) );
         }
         else
         {
            iterID.asprintf( "i%02d-m%05d", iterNum, qRound( meniscus * 10000 ) );
         }
      }

      // fill in actual model parameters needed for output
      model.description = descbase + iterID + ".model";
      mname             = QString( model.description );
      mname             = montCar ?
                          tmppath + mname.replace( ".model", ".mdl.tmp" ) :
                          mdlpath + mnames[ jj ];
      model.modelGUID   = US_Util::new_guid();
      model.editGUID    = edata->editGUID;
      model.requestGUID = reqGUID;
      model.analysis    = US_Model::TWODSA;
      model.variance    = variance;
      model.meniscus    = meniscus;
      model.bottom      = bottom;
      model.wavelength  = dwavelen;
      model.dataDescrip = edata->description + QString("Angle %1").arg(angle);

      for ( int cc = 0; cc < model.components.size(); cc++ )
      {
         model.components[ cc ].name = QString::asprintf( "SC%04d", cc + 1 );
      }

      // output the model
      if ( dbP == NULL  ||  montCar )
         model.write( mname );
      else
         model.write( dbP );

      if ( montCar )
         tnames << mname;

      int kk  = jj * knois;

      if ( have_ti )
      {  // output a TI noise
         ti_noise             = ti_noises[ jj ];
         ti_noise.description = descbase + iterID + ".ti_noise";
         ti_noise.type        = US_Noise::TI;
         ti_noise.modelGUID   = model.modelGUID;
         ti_noise.noiseGUID   = US_Util::new_guid();
         nname                = noipath + nnames[ kk++ ];
         int nicount          = ti_noise_in.count;

         if ( nicount > 0 )   // Sum in any input noise
            ti_noise.sum_noise( ti_noise_in, true );

         ti_noise.write( nname );

         if ( dbP != NULL )
            ti_noise.write( dbP );

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
         ri_noise             = ri_noises[ jj ];
         ri_noise.description = descbase + iterID + ".ri_noise";
         ri_noise.type        = US_Noise::RI;
         ri_noise.modelGUID   = model.modelGUID;
         ri_noise.noiseGUID   = US_Util::new_guid();
         nname                = noipath + nnames[ kk++ ];
         int nicount          = ri_noise_in.count;

         if ( nicount > 0 )   // Sum in any input noise
            ri_noise.sum_noise( ri_noise_in, true );

         ri_noise.write( nname );

         if ( dbP != NULL )
            ri_noise.write( dbP );

         if ( nicount > 0 )   // Remove input noise in case re-plotted
         {
            US_Noise noise_rmv = ri_noise_in;

            for ( int kk = 0; kk < nicount; kk++ )
               noise_rmv.values[ kk ] *= -1.0;

            ri_noise.sum_noise( noise_rmv, true );
         }
      }
   }
   //tino = ti_noise.count > 0 ? ti_noise.values[0] : 0.0;
   //rino = ri_noise.count > 0 ? ri_noise.values[0] : 0.0;
//DbgLv(1) << "  Post-sum tno rno" << tino << rino;
   mname          = mdlpath + mnames[ 0 ];

   if ( montCar )
   {  // For Monte Carlo, create a composite of MC iteration files
      QString tname  = US_Model::composite_mc_file( tnames, true );
      tnames[ 0 ]    = tname;
      QFile( tname ).rename( mname );    // Move/rename to model directory

      if ( dbP != NULL )
      {  // If DB, load and store in the database
         model.load( mname );

         US_Model model2 = model;
         qDebug() << "::::: nmodel=" << model.components.size() << " nmodel2=" << model2.components.size();
         double avtemp  = dset.temperature;
         for (auto & ii : model2.components)
         {
            US_Model::SimulationComponent* component = &ii;

            US_Math2::SolutionData  sd{};
            sd.viscosity   = dset.viscosity;
            sd.density     = dset.density;
            sd.manual      = dset.manual;
            sd.vbar20      = ii.vbar20;
            sd.vbar        = US_Math2::adjust_vbar20( sd.vbar20, avtemp );
            US_Math2::data_correction( avtemp, sd );
            double scorr   = sd.s20w_correction;
            double dcorr   = sd.D20w_correction;
            if (dset.simparams.meshType != US_SimulationParameters::ASTFVM)
            {
               component->s  /= scorr;
               component->D  /= dcorr;
            }
            if ( component->extinction > 0.0 )
            {
               component->molar_concentration = component->signal_concentration / component->extinction;
            }
         }

         SIMPARAMS simparms = dset.simparams;
         int drow = lw_triples->currentRow();
         US_AstfemMath::initSimData( sdata, dataList[ drow ], 0.0 );
         if (dset.simparams.meshType == US_SimulationParameters::ASTFVM)
         {
            US_LammAstfvm* lamm = new US_LammAstfvm(model2, simparms);
            lamm->set_buffer( dset.solution_rec.buffer, bfg, nullptr );
            lamm->calculate(sdata);
            delete lamm;
         }
         else
         {
            US_Astfem_RSA* astfem_rsa = new US_Astfem_RSA( model2, simparms );
            astfem_rsa->set_debug_flag( dbg_level) ;
            astfem_rsa->calculate( sdata );
            delete astfem_rsa;
         }


         int kpts = 0;
         double variance = 0.0;
         bool sub_ri = ri_noise.values.size() > 0 ? true : false;
         bool sub_ti = ti_noise.values.size() > 0 ? true : false;
         qDebug() << "::::: sub_ri=" << sub_ri << " sub_ti=" << sub_ti;
         for ( int ii = 0; ii < sdata.scanCount(); ii++ )
         {
            double rin = sub_ri ? ri_noise.values.at( ii ) : 0.0;
            for ( int jj = 0; jj < sdata.pointCount(); jj++ )
            { // Calculate the residuals and the RMSD
               double tin  = sub_ti ? ti_noise.values.at( jj ) : 0.0;
               double diff = edata->value( ii, jj ) - sdata.value( ii, jj ) - rin - tin;
               variance += sq( diff );
               kpts++;
            }
         }
         variance  /= static_cast<double>( kpts );
         le_vari->setText( QString::number( variance ) );
         le_rmsd->setText( QString::number( sqrt( variance ) ) );
         model.variance = variance;
         model.write( dbP );
      }
   }

   else
   {  // For non-MC, save the model name for message to follow
      tnames.clear();
      tnames << mname;
   }

   if ( dbP != NULL )
   {
      delete dbP;
      dbP = NULL;
   }

   reppath           = reppath + "/" + runID + "/";
   respath           = respath + "/" + runID + "/";
   QString analybase = montCar ? "2DSA-MC" : "2DSA";
   QMap< US_SimulationParameters::FitType, QString > maDescMap;
   maDescMap.insert( US_SimulationParameters::NOTHING, "" );
   maDescMap.insert( US_SimulationParameters::MENISCUS, "M" );
   maDescMap.insert( US_SimulationParameters::BOTTOM, "B" );
   maDescMap.insert( US_SimulationParameters::ANGLE, "A" );
   maDescMap.insert( US_SimulationParameters::BAND_VOLUME, "V" );
   maDescMap.insert( US_SimulationParameters::SIGMA, "S" );
   maDescMap.insert( US_SimulationParameters::DELTA, "D" );
   maDescMap.insert( US_SimulationParameters::VBAR, "R" );
   maDescMap.insert( US_SimulationParameters::FF0, "F" );
   maDescMap.insert( US_SimulationParameters::TEMPERATURE, "T" );
   bool is_param_fit = dset.simparams.primaryFit != US_SimulationParameters::NOTHING || dset.simparams.secondaryFit != US_SimulationParameters::NOTHING;
   if (is_param_fit)
   {
      analybase += "-F";
   }
   if (dset.simparams.primaryFit != US_SimulationParameters::NOTHING)
   {
      analybase += maDescMap[dset.simparams.primaryFit];
   }
   if (dset.simparams.secondaryFit != US_SimulationParameters::NOTHING)
   {
      analybase += maDescMap[dset.simparams.secondaryFit];
   }
   QString analynode = "/" + analybase + ".";
   QString maDesc("");
    if ( model.description.startsWith( runID ) )
    {  // Saved model:  get analysis description from model description
        maDesc            = model.description.section( ".", -2, -2 ).section( "_", 1, -1 );
    }
   QString filebase  = reppath  + analybase + dext + "."+ maDesc + ".";
   QString htmlFile  = filebase + "report.html";
   QString plot1File = filebase + "velocity.svgz";
   QString plot2File = filebase + "residuals.png";
   QString plot3File = filebase + "rbitmap.png";
   QString plot4File = filebase + "velocity_nc.svgz";
   QString fitFile   = filebase + ( (dset.simparams.secondaryFit != US_SimulationParameters::NOTHING) ?
                                    "fitbot.dat" :
                                    "fitmen.dat" );
   QString fresFile  = respath  + QString( fitFile )
                                  .section( "/", -1, -1 );
   QString dsinfFile = QString( filebase ).replace( analynode, "/dsinfo." )
                                + "dataset_info.html";

   // Write a general dataset information html file
   write_dset_report( dsinfFile );

   // Write HTML report file
   QFile rep_f( htmlFile );

   if ( ! rep_f.open( QIODevice::WriteOnly | QIODevice::Text ) )
      return;

   QTextStream ts( &rep_f );
   write_report( ts );
   rep_f.close();

   if ( resplotd == NULL )
   {
      resplotd = new US_ResidPlot2D( this );
      resplotd->move( rbd_pos );
      resplotd->setVisible( true );
      connect( resplotd, SIGNAL( destroyed() ), this, SLOT( resplot_done() ) );
   }

   // Write plots
   write_plot( plot1File, data_plot2 );
   write_plot( plot2File, resplotd->rp_data_plot2() );
   write_bmap( plot3File );
   write_plot( plot4File, resplotd->rp_data_plot1() );
   
   // use a dialog to tell the user what we've output
   QString wmsg = tr( "Wrote:\n" );

   mname = loadDB ? tnames[ 0 ] : mdlpath + mnames[ 0 ];
   wmsg  = wmsg + mname + "\n";                // list 1st (only?) model file

   if ( knois > 0 )
   {
      nname = noipath + nnames[ 0 ];           // list 1st noise file(s)
      wmsg  = wmsg + nname + "\n";

      if ( knois > 1 )
      {
         nname = noipath + nnames[ 1 ];
         wmsg  = wmsg + nname + "\n";
      }
   }

   if ( nmodels > 1  &&  ! montCar )
   {
      int kk = ( nmodels - 2 ) * ( knois + 1 );
      wmsg   = wmsg + " ...  ( "
                    + QString::number( kk ) + tr( " files unshown )\n" );
      kk     = nmodels - 1;
      mname  = mdlpath + mnames[ kk ];         // list last model file
      wmsg   = wmsg + mname + "\n";

      if ( knois > 0 )
      {                                        // list last noise file(s)
         kk    *= knois;
         nname  = noipath + nnames[ kk++ ];
         wmsg   = wmsg + nname + "\n";

         if ( knois > 1 )
         {
            nname  = noipath + nnames[ kk ];
            wmsg   = wmsg + nname + "\n";
         }
      }
   }

   // list report and plot files
   wmsg = wmsg + htmlFile  + "\n"
               + plot1File + "\n"
               + plot2File + "\n"
               + plot3File + "\n"
               + plot4File + "\n";
   QStringList repfiles;
   update_filelist( repfiles, htmlFile  );
   update_filelist( repfiles, plot1File );
   update_filelist( repfiles, plot2File );
   update_filelist( repfiles, plot3File );
   update_filelist( repfiles, plot4File );

   // Add fit files if fit-meniscus or fit-bottom
   if ( is_param_fit )
   {
      QString fitstr = fit_meniscus_data();

      QFile rep_f( fitFile );
      QFile res_f( fresFile );

      if ( rep_f.open( QIODevice::WriteOnly | QIODevice::Text ) )
      {
         QTextStream ts( &rep_f );
         ts << fitstr;
         rep_f.close();
         wmsg = wmsg + fitFile  + "\n";
         update_filelist( repfiles, fitFile );
      }

      if ( res_f.open( QIODevice::WriteOnly | QIODevice::Text ) )
      {
         QTextStream ts( &res_f );
         ts << fitstr;
         res_f.close();
         wmsg = wmsg + fresFile + "\n";
      }
   }

   wmsg = wmsg + dsinfFile + "\n";
   update_filelist( repfiles, dsinfFile );

   if ( disk_controls->db() )
   {  // Write report files to the database
      reportFilesToDB( repfiles );

      wmsg += tr( "\nReport files were also saved to the database." );
   }

   QApplication::restoreOverrideCursor();
   QMessageBox::information( this, tr( "Successfully Written" ), wmsg );
}

// Return pointer to main window edited data
US_DataIO::EditedData* US_2dsa::mw_editdata()
{
   int drow = lw_triples->currentRow();
   edata    = ( drow >= 0 ) ? &dataList[ drow ] : 0;
DbgLv(1) << "(M)mw_ed" << edata;
   return edata;
}

// Return pointers to main window data and GUI elements

US_DataIO::RawData*         US_2dsa::mw_simdata()      { return &sdata;    }
US_DataIO::RawData*         US_2dsa::mw_resdata()      { return &rdata;    }
QList< int >*               US_2dsa::mw_excllist()     { return &excludedScans;}
US_Model*                   US_2dsa::mw_model()        { return &model;    }
US_Noise*                   US_2dsa::mw_ti_noise()     { return &ti_noise; }
US_Noise*                   US_2dsa::mw_ri_noise()     { return &ri_noise; }
QVector<double>*            US_2dsa::mw_Anorm()        { return &normvA; }
QPointer< QTextEdit    >    US_2dsa::mw_status_text()  { return te_status;  }
int*                        US_2dsa::mw_base_rss()     { return &baserss;  }

// Open residuals plot window
void US_2dsa::open_resplot()
{
   if ( resplotd != 0 )
   {
      rbd_pos  = resplotd->pos();
      resplotd->close();
   }
   else
      rbd_pos  = this->pos() + QPoint(  100, 100 );

   resplotd = new US_ResidPlot2D( this );
   resplotd->move( rbd_pos );
   resplotd->setVisible( true );
   connect( resplotd, SIGNAL( destroyed() ), this, SLOT( resplot_done() ) );
}

// Open 3-D plot control window
void US_2dsa::open_3dplot()
{
   if ( eplotcd )
   {
      epd_pos  = eplotcd->pos();
      eplotcd->close();
   }
   else
      epd_pos  = this->pos() + QPoint(  400, 200 );

   eplotcd = new US_PlotControl2D( this, &model );
   eplotcd->move( epd_pos );
   eplotcd->show();
}

// Open fit analysis control window
void US_2dsa::open_fitcntl()
{
   int    drow     = lw_triples->currentRow();
   if ( drow < 0 )   return;
   edata           = &dataList[ drow ];
   double avTemp   = edata->average_temperature();
   double vbar20   = US_Math2::calcCommonVbar( solution_rec, 20.0   );
   double vbartb   = US_Math2::calcCommonVbar( solution_rec, avTemp );

   US_Math2::SolutionData sd;
   sd.density      = density;
   sd.viscosity    = viscosity;
   sd.vbar20       = vbar20;
   sd.vbar         = vbartb;
   sd.manual       = manual;
   US_Math2::data_correction( avTemp, sd );
DbgLv(0) << "2DSA s_corr D_corr" << sd.s20w_correction << sd.D20w_correction
 << "manual" << sd.manual << "vbar20" << vbar20;
DbgLv(0) << "2DSA d_corr v vW vT d dW dT" << sd.viscosity << sd.viscosity_wt
 << sd.viscosity_tb << sd.density << sd.density_wt << sd.density_tb;
   US_Passwd pw;
   loadDB                  = disk_controls->db();
   US_DB2* dbP             = loadDB ? new US_DB2( pw.getPasswd() ) : NULL;

   // Initialize simulation parameters from data.
   // Skip adding speed steps if this is multi-speed, initially,
   // but set speed steps to the experiment vector.
   dset.simparams.initFromData( dbP, dataList[ drow ], !exp_steps );
   if (dataLoaded){dset.solution_rec = solution_rec;}
   edata->bottom   = ( edata->bottom > 0.0 ) ?
                     edata->bottom :
                     0.0;
//                     dset.simparams.bottom;
DbgLv(1) << "2DSA:o_fit: ebottom" << edata->bottom;

   if ( exp_steps )
   {
      dset.simparams.speed_step  = speed_steps;
   }

   // Build a TimeState object now if possible
DbgLv(1) << "2dsa : checking timestate object need";
   dset.simparams.sim = ( dataList[ drow ].channel == "S" );
   bool need_tsfile   = true;
   QString tmst_fpath = US_Settings::resultDir() + "/" + runID + "/"
                        + runID + ".time_state.tmst";
   QFileInfo check_file( tmst_fpath );

   US_AstfemMath::initSimData( sdata, dataList[ drow ], 0.0 );

   if ( check_file.exists()  &&  check_file.isFile() )
   {
      bool intv_1sec  = US_AstfemMath::timestate_onesec( tmst_fpath, sdata );
DbgLv(1) << "2dsa :  intv_1sec" << intv_1sec;

      if ( intv_1sec )
      {
         dset.simparams.simSpeedsFromTimeState( tmst_fpath );
DbgLv(1) << "2dsa :   tsobj,ssp created";
         need_tsfile             = false;
      }
DbgLv(1) << "2dsa : timestate file exists" << tmst_fpath 
 << " timestateobject = " << dset.simparams.tsobj
 << "stepscount=" << dset.simparams.speed_step.size();
   }

   if ( need_tsfile )
   {
      QString tmst_fpath = US_Settings::tmpDir() + "/" + temp_Id_name() + ".time_state.tmst";
      US_AstfemMath::writetimestate( tmst_fpath, dset.simparams, sdata );

      dset.simparams.simSpeedsFromTimeState( tmst_fpath );
DbgLv(1)<<"2dsa : timestate newly created.  timestateobject = "
 << dset.simparams.tsobj << "exp_steps=" << exp_steps
 << "sspknt" << dset.simparams.sim_speed_prof.count();
   }

//   if ( dset.simparams.sim_speed_prof.count() > 1 )
//   {  // If from multi-speed, compute speed steps from sim speed profile
//      dset.simparams.speedstepsFromSSprof();
//   }
   // Compute speed steps from sim speed profile
   dset.simparams.speedstepsFromSSprof();

   dset.run_data           = dataList[ drow ];
   dset.viscosity          = viscosity;
   dset.density            = density;
   dset.temperature        = avTemp;
   dset.vbar20             = vbar20;
   dset.vbartb             = vbartb;
   dset.s20w_correction    = sd.s20w_correction;
   dset.D20w_correction    = sd.D20w_correction;
   dset.manual             = manual;

DbgLv(1) << "Bottom" << dset.simparams.bottom << "rotorcoeffs"
 << dset.simparams.rotorcoeffs[0] << dset.simparams.rotorcoeffs[1];
DbgLv(1) << "SimulationParameter --";
if(dbg_level>0) dset.simparams.debug();

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

   analcd  = new US_AnalysisControl2D( dsets, loadDB, this );
   analcd->move( acd_pos );
   analcd->show();
   qApp->processEvents();
}

// Distribution information HTML string
QString US_2dsa::distrib_info()
{
   int ncomp     = model.components.size();
   
   if ( ncomp == 0 )
   {
      return "";
   }

   QString maDesc    = model.description;
   QString runID     = edata->runID;

   if ( maDesc.startsWith( runID ) )
   {  // Saved model:  get analysis description from model description
      maDesc            = maDesc.section( ".", -2, -2 ).section( "_", 1, -1 );
   }

   else
   {  // No saved model yet:  compose analysis description
      QString adate     = "a" + QDateTime::currentDateTime().toUTC()
                          .toString( "yyMMddhhmm" );
      bool    cusGrid   = model.description.contains( "CUSTOMGRID" );
      bool    refIter   = model.description.contains( "REFITERS" );
      QMap< US_SimulationParameters::FitType, QString > maDescMap;
      maDescMap.insert( US_SimulationParameters::NOTHING, "" );
      maDescMap.insert( US_SimulationParameters::MENISCUS, "M" );
      maDescMap.insert( US_SimulationParameters::BOTTOM, "B" );
      maDescMap.insert( US_SimulationParameters::ANGLE, "A" );
      maDescMap.insert( US_SimulationParameters::BAND_VOLUME, "V" );
      maDescMap.insert( US_SimulationParameters::SIGMA, "S" );
      maDescMap.insert( US_SimulationParameters::DELTA, "D" );
      maDescMap.insert( US_SimulationParameters::VBAR, "R" );
      maDescMap.insert( US_SimulationParameters::FF0, "F" );
      maDescMap.insert( US_SimulationParameters::TEMPERATURE, "T" );
      bool    montCar   = model.monteCarlo;
      DbgLv(1) << "2DSA:SV: cusGrid" << cusGrid << "desc" << model.description;
      QString anType    = QString( cusGrid ? "2DSA-CG" : "2DSA" );
      if ( dset.simparams.primaryFit != US_SimulationParameters::NOTHING ||
         dset.simparams.secondaryFit != US_SimulationParameters::NOTHING )
      {
         anType += "-F";
         anType += maDescMap[dset.simparams.primaryFit];
         anType += maDescMap[dset.simparams.secondaryFit];
      }
      anType           += QString( refIter ? "-IT" : "" )
                        + QString( montCar ? "-MC" : "" );
      maDesc            = adate + "_" + anType + "_local_i01";
   }

   QString mstr = "\n" + indent( 4 )
      + tr( "<h3>Data Analysis Settings:</h3>\n" )
      + indent( 4 ) + "<table>\n";

   mstr += table_row( tr( "Model Analysis:" ),
                      maDesc );
   mstr += table_row( tr( "Solution:" ),
                      QString( dset.solution_rec.solutionDesc ) + " " + dset.solution_rec.solutionGUID );
   mstr += table_row( tr( "Number of Components:" ),
                      QString::number( ncomp ) );
   mstr += table_row( tr( "Residual RMS Deviation:" ),
                      QString::number( rmsd )  );
   mstr += table_row( tr( "s20w correction:" ),
                      QString::number( dset.s20w_correction )  );
   mstr += table_row( tr( "D20w correction:" ),
                      QString::number( dset.D20w_correction )  );
   QStringList meshType;
   meshType <<"ASTFEM"<< "CLAVERIE"<< "MOVING_HAT"<< "USER"<< "ASTFVM";
   mstr += table_row( tr( "Mesh Type:" ),
                      meshType[dset.simparams.meshType] );
   QStringList gridType;
   gridType << "FIXED" << "MOVING";
   QStringList fitType;
   fitType << "NOTHING" << "MENISCUS" << "BOTTOM" << "ANGLE" << "BAND_VOLUME" << "SIGMA" << "DELTA"
               << "VBAR" << "FF0" << "TEMPERATURE";
   mstr += table_row( tr( "Grid Type:" ),
                      gridType[dset.simparams.gridType] );
   if (dset.simparams.primaryFit != US_SimulationParameters::NOTHING)
   {
      mstr += table_row( tr( "Primary Fit:" ),
                         fitType[dset.simparams.primaryFit] );
   }
   if (dset.simparams.secondaryFit != US_SimulationParameters::NOTHING)
   {
      mstr += table_row( tr( "Secondary Fit:" ),
                         fitType[dset.simparams.secondaryFit] );
   }
   mstr += table_row( tr( "Simulation points:" ),
                      QString::number( dset.simparams.simpoints )  );
   mstr += table_row( tr( "Radial Resolution:" ),
                      QString::number( dset.simparams.radial_resolution )  );
   mstr += table_row( tr( "Band forming:" ),
                      QString( dset.simparams.band_forming?(QString("Yes") + " "+ QString::number(dset.simparams.band_volume) + " mL"):"No" ));
   mstr += table_row( tr( "Channel angle:" ),
                      QString::number( dset.simparams.cp_angle )  );
   mstr += table_row( tr( "Meniscus:" ),
                      QString::number( dset.simparams.meniscus )  + " cm");
   mstr += table_row( tr( "Bottom:" ),
                      QString::number( dset.simparams.bottom )  );
   mstr += table_row( tr( "Bottom Position" ),
                      QString::number( dset.simparams.bottom_position )  + " cm");
   if ( dset.simparams.sigma != 0.0 )
   {
      mstr += table_row( tr( "Sigma:" ),
                         QString::number( dset.simparams.sigma )  );
   }
   if ( dset.simparams.delta != 0.0 )
   {
      mstr += table_row( tr( "Delta:" ),
                         QString::number( dset.simparams.delta )  );
   }

   double sum_mw  = 0.0;
   double sum_s   = 0.0;
   double sum_D   = 0.0;
   double sum_ff0 = 0.0;
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
      sum_ff0    += model.components[ ii ].f_f0 * conc;
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
   mstr += table_row( tr( "W.A. frictional ration:" ),
                      QString::asprintf( "%6.4e", ( sum_ff0 / sum_c ) ) );
   mstr += table_row( tr( "Total Concentration:" ),
                      QString::asprintf( "%6.4e", sum_c ) );

   if ( cnstvb )
   {
      mstr += table_row( tr( "Constant Vbar at 20" ) + DEGC + ":",
                         QString::number( maxv ) );
   }
   else
   {
      mstr += table_row( tr( "Constant f/f0:" ),
                         QString::number( maxk ) );
   }

   mstr += indent( 4 ) + "</table>\n\n";

   mstr += indent( 4 ) + tr( "<h3>Distribution Information:</h3>\n" )
      + indent( 4 ) + "<table>\n";

   if ( cnstvb )
   {
      mstr += table_row( tr( "Molecular Wt." ), tr( "S Apparent" ),
                         tr( "S 20,W" ),        tr( "D Apparent" ),
                         tr( "D 20,W" ),        tr( "f / f0" ),
                         tr( "Concentration" ) );
   }
   else
   {
      mstr += table_row( tr( "Molecular Wt." ), tr( "S Apparent" ),
                         tr( "S 20,W" ),        tr( "D Apparent" ),
                         tr( "D 20,W" ),        tr( "Vbar20" ),
                         tr( "Concentration" ) );
   }

   int    drow     = lw_triples->currentRow();
   edata           = &dataList[ drow ];
   double avTemp   = edata->average_temperature();
   double vbar20   = US_Math2::calcCommonVbar( solution_rec, 20.0   );
   double vbartb   = US_Math2::calcCommonVbar( solution_rec, avTemp );
   US_Math2::SolutionData sd;
   sd.density      = density;
   sd.viscosity    = viscosity;
   sd.vbar20       = vbar20;
   sd.vbar         = vbartb;
   sd.manual       = manual;
   DbgLv(1) << "Data_Corr manual" << sd.manual << "avTemp" << avTemp << "vbar20" << sd.vbar20
    << "vbart" << sd.vbar;
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

// Iteration information HTML string
QString US_2dsa::iteration_info()
{
   int nmodels   = models.size();
   
   if ( nmodels < 2 )
   {
      return "";
   }

   model            = models[ nmodels - 1 ];
   bool fitMeni    = ( model.global == US_Model::MENISCUS || model.global == US_Model::BOTTOM ||
model.global == US_Model::MENIBOTT || model.global == US_Model::ANGLE || model.global == US_Model::MENIANGLE);
   bool    montCar  = model.monteCarlo;
   QString anType   = montCar ? "Monte Carlo" : "Fit Meniscus";

   QString mstr   = "\n" + indent( 4 )
      + tr( "<h3>Multiple Model Settings:</h3>\n" )
      + indent( 4 ) + "<table>\n";

   mstr += table_row( tr( "Number of Model Iterations:" ),
                      QString::number( nmodels ) );
   mstr += table_row( tr( "Iteration Analysis Type:" ), anType );
   QStringList fitType;
   fitType << "NOTHING" << "MENISCUS" << "BOTTOM" << "ANGLE" << "BAND_VOLUME" << "SIGMA" << "DELTA"
               << "VBAR" << "FF0" << "TEMPERATURE";
   if ( dset.simparams.primaryFit != US_SimulationParameters::NOTHING )
   {
      const US_SimulationParameters::FitType var_parm = dset.simparams.primaryFit;
      const double var_rang = dset.simparams.primary_range;
      const QString parm = fitType[var_parm];

      mstr += table_row( tr( qPrintable( "Primary Parameter" + parm + " Range:" )  ),
                         QString::number( var_rang ) + " (" + QString::number( dset.simparams.primary_variations) + " variations)" );
      mstr += table_row( tr( qPrintable("Base Experiment " + parm + ":") ),
                         QString::number( dset.simparams.get_parameter_value( var_parm ) - var_rang / 2.0 ) );
      mstr += table_row( tr( qPrintable("Start Fit " + parm + ":") ),
                         QString::number( dset.simparams.get_parameter_value( var_parm ) - var_rang ) );
      mstr += table_row( tr( qPrintable("End Fit " + parm + ":") ),
                         QString::number( dset.simparams.get_parameter_value( var_parm ) ) );
   }
   if ( dset.simparams.secondaryFit != US_SimulationParameters::NOTHING )
   {
      const US_SimulationParameters::FitType var_parm = dset.simparams.secondaryFit;
      const double var_rang = dset.simparams.secondary_range;
      const QString parm = fitType[var_parm];

      mstr += table_row( tr( qPrintable( "Primary Parameter" + parm + " Range:" )  ),
                         QString::number( var_rang ) + " (" + QString::number( dset.simparams.secondary_variations) + " variations)" );
      mstr += table_row( tr( qPrintable("Base Experiment " + parm + ":") ),
                         QString::number( dset.simparams.get_parameter_value( var_parm ) - var_rang / 2.0 ) );
      mstr += table_row( tr( qPrintable("Start Fit " + parm + ":") ),
                         QString::number( dset.simparams.get_parameter_value( var_parm ) - var_rang ) );
      mstr += table_row( tr( qPrintable("End Fit " + parm + ":") ),
                         QString::number( dset.simparams.get_parameter_value( var_parm ) ) );
   }

   mstr += indent( 4 ) + "</table>\n\n";

   mstr += indent( 4 ) + tr( "<h3>Fit / Iteration Information:</h3>\n" )
      + indent( 4 ) + "<table>\n";

   if ( montCar )
   {
      mstr += table_row( tr( "Iteration" ),
                         tr( "Iteration ID" ),
                         tr( "RMSD" ) );
   }
   else if ( dset.simparams.primaryFit != US_SimulationParameters::NOTHING && dset.simparams.secondaryFit != US_SimulationParameters::NOTHING )
   {
      mstr += table_row( tr( "Iteration" ),
                         tr( qPrintable(fitType[dset.simparams.primaryFit]) ),
                         tr( qPrintable(fitType[dset.simparams.secondaryFit]) ),
                         tr( "RMSD" ) );
   }
   else if ( dset.simparams.primaryFit != US_SimulationParameters::NOTHING )
   {
      mstr += table_row( tr( "Iteration" ),
                         tr( qPrintable(fitType[dset.simparams.primaryFit]) ),
                         tr( "RMSD" ) );
   }
   QStringList fitTypeAttr;
   fitTypeAttr << "NOTHING" << "MENISCUS" << "BOTTOM" << "ANGLE" << "VOLUME" << "SIGMA" << "DELTA"
               << "VBAR" << "FF0" << "TEMPERATURE";
   for ( int ii = 0; ii < nmodels; ii++ )
   {
      QString itnum = QString::number( ii + 1 ).rightJustified( 4, '_' );
      QString mdesc = models[ ii ].description;
      QString avari = mdesc.mid( mdesc.indexOf( "VARI=" ) + 5 );
      double  rmsd  = sqrt( avari.section( " ", 0, 0 ).toDouble() );
      QString armsd = QString::asprintf( "%10.8f", rmsd );

      if ( montCar )
      {
         QString itID  = QString::asprintf( "i%04i", ii + 1 );
         mstr         += table_row( itnum, itID, armsd );
      }
      else if ( dset.simparams.primaryFit != US_SimulationParameters::NOTHING && dset.simparams.secondaryFit != US_SimulationParameters::NOTHING )
      {
         QString prim = fitTypeAttr[dset.simparams.primaryFit] + "=";
         QString sec  = fitTypeAttr[dset.simparams.secondaryFit] + "=";
         QString ameni = mdesc.mid( mdesc.indexOf( prim ) + prim.length() )
                .section( " ", 0, 0 );
         QString abott = mdesc.mid( mdesc.indexOf( sec ) + sec.length() )
                         .section( " ", 0, 0 );
         mstr         += table_row( itnum, ameni, abott, armsd );
      }
      else if ( dset.simparams.primaryFit != US_SimulationParameters::NOTHING )
      {
         QString prim = fitTypeAttr[dset.simparams.primaryFit] + "=";
         QString ameni = mdesc.mid( mdesc.indexOf( prim ) + prim.length() )
                .section( " ", 0, 0 );
         mstr         += table_row( itnum, ameni, armsd );
      }
   }

   mstr += indent( 4 ) + "</table>\n";
   
   return mstr;
}

// Write HTML report file
void US_2dsa::write_report( QTextStream& ts )
{
   ts << html_header( QString( "US_2dsa" ),
                      tr( "2-Dimensional Spectrum Analysis" ),
                      edata );
   ts << distrib_info();
   ts << iteration_info();
   ts << indent( 2 ) + "</body>\n</html>\n";
}

// Write resids bitmap plot file
void US_2dsa::write_bmap( const QString plotFile )
{
   // Generate the residuals array
   bool have_ri = ri_noise.count > 0;
   bool have_ti = ti_noise.count > 0;
   int  npoints = edata->pointCount();
   int  nscans  = edata->scanCount();
   QVector< double >            resscn( npoints );
   QVector< QVector< double > > resids( nscans  );

   for ( int ii = 0; ii < nscans; ii++ )
   {
      double rnoi  = have_ri ? ri_noise.values[ ii ] : 0.0;

      for ( int jj = 0; jj < npoints; jj++ )
      {
         double tnoi  = have_ti ? ti_noise.values[ jj ] : 0.0;
         resscn[ jj ] = edata->value( ii, jj ) - sdata.value(  ii, jj )
                        - rnoi - tnoi;
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
}

// New triple selected
void US_2dsa::new_triple( int index )
{
   edata = &dataList[ index ];

   // Restore pure data type string (other values added in 2dsa processing)
   edata->dataType = edata->dataType.section( " ", 0, 0 );

   sdata.scanData.clear();                 // Clear simulation and upper plot
   dataPlotClear( data_plot1 );
   models     .clear();
   ti_noises  .clear();
   ri_noises  .clear();

   // cleanup windows
   if ( eplotcd  )
   {
      epd_pos  = eplotcd->pos();
      eplotcd->close();
      eplotcd.clear();
   }

   if ( resplotd  )
   {
      rbd_pos  = resplotd->pos();
      resplotd->close();
      resplotd.clear();
   }
   if ( analcd )
   {
      acd_pos  = analcd->pos();
      analcd->close();
      analcd.clear();
   }
   if ( analcd1 )
   {
      analcd1->close();
      analcd1.clear();
   }
   if ( te_results )
   {
      te_results->close();
      te_results.clear();
   }

   // Temporarily restore any loaded noise vectors from triples vectors
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

// Fit meniscus data table string
QString US_2dsa::fit_meniscus_data()
{
   QString mstr  = "";
   int nmodels   = models.size();
   
   if ( nmodels < 2 )
   {
      return mstr;
   }

   QStringList fitTypeAttr;
   fitTypeAttr << "NOTHING" << "MENISCUS" << "BOTTOM" << "ANGLE" << "VOLUME" << "SIGMA" << "DELTA"
               << "VBAR" << "FF0" << "TEMPERATURE";
   QString prim = fitTypeAttr[dset.simparams.primaryFit] + "=";
   QString sec  = fitTypeAttr[dset.simparams.secondaryFit] + "=";
   // create the header
   QString mdesc = models[ 0 ].description;
   if ( mdesc.indexOf( prim ) > 0 )
   {
      mstr += fitTypeAttr[dset.simparams.primaryFit] + " ";
   }
   if ( mdesc.indexOf( sec ) > 0 )
   {
      mstr += fitTypeAttr[dset.simparams.secondaryFit] + " ";
   }
   mstr += "RMSD\n";

   for ( int ii = 0; ii < nmodels; ii++ )
   {
      mdesc = models[ ii ].description;
      QString avari = mdesc.mid( mdesc.indexOf( "VARI=" ) + 5 );
      double  variv = avari.section( " ", 0, 0 ).toDouble();
      double  rmsd  = ( variv > 0.0 ) ? sqrt( variv ) : 0.0;
      QString armsd = QString::asprintf( "%10.8f", rmsd );
      if (mdesc.indexOf( prim ) > 0)
      {
         QString aprim = mdesc.mid( mdesc.indexOf( prim ) + prim.length() )
                .section( " ", 0, 0 );
         mstr += aprim + " ";
      }
      if (mdesc.indexOf( sec ) > 0)
      {
         QString asec = mdesc.mid( mdesc.indexOf( sec ) + sec.length() )
         .section( " ", 0, 0 );
         mstr += asec + " ";
      }
      mstr += armsd + "\n";
   }
   
   return mstr;
}

// Public slot to mark residual plot dialog closed
void US_2dsa::resplot_done()
{
   resplotd     = 0;
}

// Close all opened subwindows, then close this one
void US_2dsa::close_all()
{
   if ( resplotd != 0 )
   {
      resplotd->disconnect();
      resplotd->close();
   }
   if ( eplotcd  != 0 )
   {
      eplotcd ->disconnect();
      eplotcd ->close();
   }
   if ( analcd   != 0 )
   {
      analcd  ->disconnect();
      analcd  ->close();
   }

   close();
}

// Construct a temporary filename based on pid and time
QString US_2dsa::temp_Id_name()
{
   return ( "p" + QString::number( getpid() ) + "t" +
            QDateTime::currentDateTime().toUTC().toString( "yyMMddhhmmss" ) );
}

