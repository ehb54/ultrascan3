//! \file us_1dsa.cpp

#include <QApplication>
#include <QtSvg>

#include "us_1dsa.h"
#include "us_resids_bitmap.h"
#include "us_plot_control.h"
#include "us_analysis_control.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
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

//! \brief Main program for us_1dsa. Loads translators and starts
//         the class US_1dsa.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_1dsa w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

// constructor, based on AnalysisBase
US_1dsa::US_1dsa() : US_AnalysisBase2()
{
   setWindowTitle( tr( "1-Dimensional Spectrum Analysis" ) );
   setObjectName( "US_1dsa" );
   dbg_level  = US_Settings::us_debug();

   // Build local and 1dsa-specific GUI elements
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

   //ct_from      = us_counter( 2, 0, 0 );
   //ct_to        = us_counter( 2, 0, 0 );

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
   ct_from->setVisible(true);
   ct_to->setVisible(true);
   pb_exclude->setVisible(true);
   pb_reset_exclude->setVisible(true);
   //ct_to->setVisible(true);

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

   // Reconstruct controls layout with some 1dsa-specific elements
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

// slot to handle the completion of a 1-D spectrum analysis stage
void US_1dsa::analysis_done( int updflag )
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
void US_1dsa::load( void )
{
   US_AnalysisBase2::load();       // load edited experiment data

   if ( !dataLoaded )  return;

   pb_view->setEnabled( false );   // disable view,save buttons for now
   pb_save->setEnabled( false );

   def_local  = ! disk_controls->db();
   edata      = &dataList[ 0 ];    // point to first loaded data

   // Move any loaded noise vectors to the "in" versions
   ri_noise_in.values = ri_noise.values;
   ti_noise_in.values = ti_noise.values;
   ri_noise.values.clear();
   ti_noise.values.clear();
   ri_noise_in.count  = ri_noise_in.values.size();
   ti_noise_in.count  = ti_noise_in.values.size();
   ti_noise   .count  = 0;
   ri_noise   .count  = 0;
DbgLv(1) << "ri,ti noise in" << ri_noise_in.count << ti_noise_in.count;
}

// plot the data
void US_1dsa::data_plot( void )
{
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
   int nscans  = edata->scanData.size();
   int npoints = edata->x.size();
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
         if ( jj > rl )
         {
            ra[ kk   ] = rr;
            va[ kk++ ] = vv;
         }
      }
      title = "SimCurve " + QString::number( ii );
      cc    = us_curve( data_plot2, title );
      cc->setPen( pen_red );
      cc->setData( ra, va, kk );
   }

   data_plot2->replot();           // replot combined exper,simul data

   data_plot1->detachItems();
   data_plot1->clear();

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
      cc->setPen(   pen_plot );
      cc->setStyle( QwtPlotCurve::Dots );
      cc->setData(  ra, va, npoints );
   }

   data_plot1->setAxisAutoScale( QwtPlot::xBottom );
   data_plot1->setAxisAutoScale( QwtPlot::yLeft   );
   data_plot1->setTitle( tr( "Residuals" ) );

   // plot zero line through residuals plot
   double xlo = edata->radius( 0 );
   double xhi = edata->radius( npoints - 1 );
   xlo        = 0.1 * (double)( (int)( xlo * 10.0 ) );
   xhi        = 0.1 * (double)( qRound( xhi * 10.0 + 0.5 ) );
   ra[ 0 ]    = xlo;
   ra[ 1 ]    = xhi;
   va[ 0 ]    = 0.0;
   va[ 1 ]    = 0.0;
   cc         = us_curve( data_plot1, "zero-line" );
   cc->setPen( QPen( QBrush( Qt::red ), 2 ) );
   cc->setData( ra, va, 2 );

   // draw the plot
   data_plot1->replot();

   // report on variance and rmsd
   vari    /= (double)( nscans * npoints );
   rmsd     = sqrt( vari );
   le_vari->setText( QString::number( vari ) );
   le_rmsd->setText( QString::number( rmsd ) );
DbgLv(1) << "Data Plot VARI" << vari;
}

// view data report
void US_1dsa::view( void )
{
   // Create the report text
   QString rtext;
   QTextStream ts( &rtext );
   write_report( ts );

   // Create US_Editor and display report
   if ( te_results == NULL )
   {
      te_results = new US_Editor( US_Editor::DEFAULT, true, QString(), this );
      te_results->resize( 780, 700 );
      QPoint p = g.global_position();
      te_results->move( p.x() + 30, p.y() + 30 );
      te_results->e->setFont( QFont( US_GuiSettings::fontFamily(),
                                     US_GuiSettings::fontSize() ) );
   }

   te_results->e->setHtml( rtext );
   te_results->show();
}

// Save data (model,noise), report, and PNG image files
void US_1dsa::save( void )
{
   QString analysisDate = QDateTime::currentDateTime().toUTC()
                          .toString( "yyMMddhhmm" );
   QString reqGUID      = US_Util::new_guid();
   QString runID        = edata->runID;
   QString editID       = edata->editID.startsWith( "20" ) ?
                          edata->editID.mid( 2 ) :
                          edata->editID;
   QString dates        = "e" + editID + "_a" + analysisDate;
   QString analysisType = "1DSA";
   QString requestID    = "local";
   QString tripleID     = edata->cell + edata->channel + edata->wavelength; 
   QString analysisID   = dates + "_" + analysisType + "_" + requestID + "_";
   QString dext         = "." + tripleID;
   QString dext2        = ".e" + editID + "-" + dext.mid( 1 );
   QString descbase     = runID + "." + tripleID + "." + analysisID;

   QString reppath  = US_Settings::reportDir();
   QString respath  = US_Settings::resultDir();
   QString tmppath  = US_Settings::tmpDir();
   QString mdlpath;
   QString noipath;
   int     knois    = min( ti_noise.count, 1 ) 
                    + min( ri_noise.count, 1 );  // noise files per model
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

   // Save the model and any noise file(s)

   US_Passwd   pw;
   US_DB2*     dbP      = def_local ? NULL : new US_DB2( pw.getPasswd() );
   QDir        dirm( mdlpath );
   QDir        dirn( noipath );
   QStringList mfilt( "M*.xml" );
   QStringList nfilt( "N*.xml" );
   QStringList mdnams   =  dirm.entryList( mfilt, QDir::Files, QDir::Name );
   QStringList ndnams   =  dirn.entryList( nfilt, QDir::Files, QDir::Name );
   QStringList nnames;
   QString     mname    = "M0000000.xml";
   QString     nname    = "N0000000.xml";
   int         indx     = 1;
   int         knoises  = 0;
   bool        have_ti  = ( tinoises.size() > 0 );
   bool        have_ri  = ( rinoises.size() > 0 );

   while( indx > 0 )
   {  // build a list of available model file names
      mname = "M" + QString().sprintf( "%07i", indx++ ) + ".xml";
      if ( ! mdnams.contains( mname ) )
      {  // no name with this index exists, so add it new-name list
         break;
      }
   }

   indx   = 1;

   while( indx > 0 )
   {  // build a list of available noise file names
      nname = "N" + QString().sprintf( "%07i", indx++ ) + ".xml";
      if ( ! ndnams.contains( nname ) )
      {  // add to the list of new-name noises
         nnames << nname;
         if ( ++knoises >= knois )
            break;
      }
   }
//double tino = ti_noise.count > 0 ? ti_noise.values[0] : 0.0;
//double tini = ti_noise_in.count > 0 ? ti_noise_in.values[0] : 0.0;
//double rino = ri_noise.count > 0 ? ri_noise.values[0] : 0.0;
//double rini = ri_noise_in.count > 0 ? ri_noise_in.values[0] : 0.0;
//DbgLv(1) << "  Pre-sum tno tni" << tino << tini << "rno rni" << rino << rini;

   // Output model and noises
   QString mdesc     = model.description;    // description from processor
   double  variance  = mdesc.mid( mdesc.indexOf( "VARI=" ) + 5 )
                       .section( ' ', 0, 0 ).toDouble();
   QString iterID    = "i01";

   // fill in actual model parameters needed for output
   model.description = descbase + iterID + ".model";
   mname             = mdlpath + "/" + mname;
   model.modelGUID   = US_Util::new_guid();
   model.editGUID    = edata->editGUID;
   model.requestGUID = reqGUID;
   model.analysis    = US_Model::ONEDSA;
   model.variance    = variance;
   model.meniscus    = meniscus;
   model.wavelength  = dwavelen;

   for ( int cc = 0; cc < model.components.size(); cc++ )
      model.components[ cc ].name = QString().sprintf( "SC%04d", cc + 1 );

   // output the model
   model.write( mname );

   if ( dbP != NULL )
      model.write( dbP );

   int kk  = 0;

   if ( have_ti )
   {  // output a TI noise
      ti_noise.description = descbase + iterID + ".ti_noise";
      ti_noise.type        = US_Noise::TI;
      ti_noise.modelGUID   = model.modelGUID;
      ti_noise.noiseGUID   = US_Util::new_guid();
      nname                = noipath + "/" + nnames[ kk++ ];
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
      ri_noise.description = descbase + iterID + ".ri_noise";
      ri_noise.type        = US_Noise::RI;
      ri_noise.modelGUID   = model.modelGUID;
      ri_noise.noiseGUID   = US_Util::new_guid();
      nname                = noipath + "/" + nnames[ kk++ ];
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
//tino = ti_noise.count > 0 ? ti_noise.values[0] : 0.0;
//rino = ri_noise.count > 0 ? ri_noise.values[0] : 0.0;
//DbgLv(1) << "  Post-sum tno rno" << tino << rino;

   if ( dbP != NULL )
   {
      delete dbP;
      dbP = NULL;
   }

   reppath           = reppath + "/" + runID + "/";
   respath           = respath + "/" + runID + "/";
   QString filebase  = reppath + "1DSA"   + dext + ".";
   QString htmlFile  = filebase + "report.html";
   QString plot1File = filebase + "velocity.svg";
   QString plot2File = filebase + "residuals.png";
   QString plot3File = filebase + "rbitmap.png";
   QString plot4File = filebase + "mlines.png";
   QString fitFile   = filebase + "fitmen.dat";
   QString fresFile  = respath  + "1dsa-fm" + dext2 + ".fitmen.dat";
   QString ptmp4File = tmppath  + "/1DSA" + dext + ".mlines."
                       + QString::number( getpid() ) + ".png";
DbgLv(1) << "mlines ptmp4File" << ptmp4File;

   // Write HTML report file
   QFile rep_f( htmlFile );

   if ( ! rep_f.open( QIODevice::WriteOnly | QIODevice::Text ) )
      return;

   QTextStream ts( &rep_f );
   write_report( ts );
   rep_f.close();

   // Write plots
   write_plot( plot1File, data_plot2 );
   write_plot( plot2File, data_plot1 );
   write_bmap( plot3File );

   QFile::remove( plot4File );
   if ( QFile::copy  ( ptmp4File, plot4File ) )
      QFile::remove( ptmp4File );
   
   // use a dialog to tell the user what we've output
   QString wmsg = tr( "Wrote:\n" );

   mname = mdlpath + "/" + mname;
   wmsg  = wmsg + mname + "\n";                // list 1st (only?) model file

   if ( knois > 0 )
   {
      nname = noipath + "/" + nnames[ 0 ];     // list 1st noise file(s)
      wmsg  = wmsg + nname + "\n";

      if ( knois > 1 )
      {
         nname = noipath + "/" + nnames[ 1 ];
         wmsg  = wmsg + nname + "\n";
      }
   }

   // list report and plot files
   wmsg = wmsg + htmlFile  + "\n"
               + plot1File + "\n"
               + plot2File + "\n"
               + plot3File + "\n"
               + plot4File + "\n";
   QStringList repfiles;
   repfiles << htmlFile << plot1File << plot2File << plot3File << plot4File;

   if ( disk_controls->db() )
   {  // Write report files to the database
      reportFilesToDB( repfiles );

      wmsg += tr( "\nReport files were also saved to the database." );
   }

   QApplication::restoreOverrideCursor();
   QMessageBox::information( this, tr( "Successfully Written" ), wmsg );
}

// Return pointer to main window edited data
US_DataIO2::EditedData* US_1dsa::mw_editdata()
{
   int drow = lw_triples->currentRow();
   edata    = ( drow >= 0 ) ? &dataList[ drow ] : 0;

   return edata;
}

// Return pointers to main window data and GUI elements

US_DataIO2::RawData*      US_1dsa::mw_simdata()      { return &sdata;    }
US_DataIO2::RawData*      US_1dsa::mw_resdata()      { return &rdata;    }
US_Model*                 US_1dsa::mw_model()        { return &model;    }
US_Noise*                 US_1dsa::mw_ti_noise()     { return &ti_noise; }
US_Noise*                 US_1dsa::mw_ri_noise()     { return &ri_noise; }
QPointer< QTextEdit   >   US_1dsa::mw_status_text()  { return te_status;    }
QStringList*              US_1dsa::mw_model_stats()  { return &model_stats; }
QVector< ModelRecord >*   US_1dsa::mw_mrecs()        { return &mrecs;       }

// Open residuals plot window
void US_1dsa::open_resplot()
{
   if ( resplotd )
   {
      rbd_pos  = resplotd->pos();
      resplotd->close();
   }
   else
      rbd_pos  = this->pos() + QPoint(  100, 100 );

   resplotd = new US_ResidPlot( this );
   resplotd->move( rbd_pos );
   resplotd->setVisible( true );
}

// Open 3-D plot control window
void US_1dsa::open_3dplot()
{
   if ( eplotcd )
   {
      epd_pos  = eplotcd->pos();
      eplotcd->close();
   }
   else
      epd_pos  = this->pos() + QPoint(  400, 200 );

   eplotcd = new US_PlotControl( this, &model );
   eplotcd->move( epd_pos );
   eplotcd->show();
}

// Open fit analysis control window
void US_1dsa::open_fitcntl()
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
             "1DSA cannot proceed with this value. Click on the\n"
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
   US_Math2::data_correction( avTemp, sd );

   US_Passwd pw;
   US_DB2* dbP             = disk_controls->db()
                             ? new US_DB2( pw.getPasswd() )
                             : NULL;
   dset.simparams.initFromData( dbP, dataList[ drow ] );

   dset.run_data           = dataList[ drow ];
   dset.simparams.bottom   = dset.simparams.bottom_position;
   dset.solute_type        = 0;
   dset.viscosity          = viscosity;
   dset.density            = density;
   dset.temperature        = avTemp;
   dset.vbar20             = vbar20;
   dset.vbartb             = vbartb;
   dset.s20w_correction    = sd.s20w_correction;
   dset.D20w_correction    = sd.D20w_correction;
DbgLv(1) << "Bottom" << dset.simparams.bottom << "rotorcoeffs"
 << dset.simparams.rotorcoeffs[0] << dset.simparams.rotorcoeffs[1];

   if ( dbP != NULL )
   {
      dataList[ drow ].description += "  (DB)";
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

   analcd  = new US_AnalysisControl( dsets, this );
   analcd->move( acd_pos );
   analcd->show();
   qApp->processEvents();
}

// Distribution information HTML string
QString US_1dsa::distrib_info()
{
   int ncomp     = model.components.size();
   
   if ( ncomp == 0 )
      return "";

   QString mstr = "\n" + indent( 4 )
      + tr( "<h3>Data Analysis Settings:</h3>\n" )
      + indent( 4 ) + "<table>\n";

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
                      QString().sprintf( "%6.4e", ( sum_s  / sum_c ) ) );
   mstr += table_row( tr( "Weight Average D20,W:" ),
                      QString().sprintf( "%6.4e", ( sum_D  / sum_c ) ) );
   mstr += table_row( tr( "W.A. Molecular Weight:" ),
                      QString().sprintf( "%6.4e", ( sum_mw / sum_c ) ) );
   mstr += table_row( tr( "Total Concentration:" ),
                      QString().sprintf( "%6.4e", sum_c ) );

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
         sd.vbar     = US_Math2::adjust_vbar( vbar20, avTemp );
         US_Math2::data_correction( avTemp, sd );
      }

      s_ap       /= sd.s20w_correction;
      D_ap       /= sd.D20w_correction;

      mstr       += table_row(
            QString().sprintf( "%10.4e", model.components[ ii ].mw ),
            QString().sprintf( "%10.4e", s_ap                      ),
            QString().sprintf( "%10.4e", model.components[ ii ].s  ),
            QString().sprintf( "%10.4e", D_ap                      ),
            QString().sprintf( "%10.4e", model.components[ ii ].D  ),
            QString().sprintf( "%10.4e", f_f0                      ),
            QString().sprintf( "%10.4e (%5.2f %%)", conc, perc     ) );
   }

   mstr += indent( 4 ) + "</table>\n";
   
   return mstr;
}

// Model statistics HTML string
QString US_1dsa::model_statistics()
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
void US_1dsa::write_report( QTextStream& ts )
{
   ts << html_header( QString( "US_1dsa" ),
                      tr( "1-Dimensional Spectrum Analysis" ),
                      edata );
   ts << run_details();
   ts << hydrodynamics();
   ts << scan_info();
   ts << model_statistics();
   ts << distrib_info();
   ts << indent( 2 ) + "</body>\n</html>\n";
}

// Write resids bitmap plot file
void US_1dsa::write_bmap( const QString plotFile )
{
   // Generate the residuals array
   bool have_ri = ri_noise.count > 0;
   bool have_ti = ti_noise.count > 0;
   int  nscans  = edata->scanData.size();
   int  npoints = edata->x.size();
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
   QPixmap          pixmap  = QPixmap::grabWidget( resbmap, 0, 0,
                                 resbmap->width(), resbmap->height() );

   // Save the pixmap to the specified file
   if ( ! pixmap.save( plotFile ) )
      qDebug() << "*ERROR* Unable to write file" << plotFile;

   resbmap->close();
}

// New triple selected
void US_1dsa::new_triple( int index )
{
   edata = &dataList[ index ];

   // Restore pure data type string (other values added in 1dsa processing)
   edata->dataType = edata->dataType.section( " ", 0, 0 );

   sdata.scanData.clear();                 // Clear simulation and upper plot
   data_plot1->detachItems();
   data_plot1->clear();

   US_AnalysisBase2::new_triple( index );  // New triple as in any analysis
}

