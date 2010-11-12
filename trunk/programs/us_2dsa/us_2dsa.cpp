//! \file us_2dsa.cpp

#include <QApplication>

#include "us_2dsa.h"
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

   // Build local and 2dsa-specific GUI elements
   te_results = NULL;

   QLabel* lb_analysis = us_banner( tr( "Analysis Controls" ) );
   QLabel* lb_scan     = us_banner( tr( "Scan Control"       ) );

   QLabel* lb_status   = us_label ( tr( "Status\nInfo:" ) );
   te_status           = us_textedit();
   QLabel* lb_from     = us_label ( tr( "From:" ) );
   QLabel* lb_to       = us_label ( tr( "to:"   ) );

   pb_exclude   = us_pushbutton( tr( "Exclude Scan Range" ) );
   pb_exclude->setEnabled( false );
   connect( pb_exclude, SIGNAL( clicked() ), SLOT( exclude() ) );

   ct_from      = us_counter( 2, 0, 0 );
   ct_to        = us_counter( 2, 0, 0 );

   connect( ct_from, SIGNAL( valueChanged( double ) ),
                     SLOT  ( exclude_from( double ) ) );
   connect( ct_to,   SIGNAL( valueChanged( double ) ),
                     SLOT  ( exclude_to  ( double ) ) );
   pb_fitcntl   = us_pushbutton( tr( "Fit Control"   ) );
   pb_loadfit   = us_pushbutton( tr( "Load Fit"      ) );
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

   // Reconstruct controls  layout with some 2dsa-specific elements
   controlsLayout->addWidget( lb_analysis,  0, 0, 1, 4 );
   controlsLayout->addWidget( lb_status,    1, 0, 4, 1 );
   controlsLayout->addWidget( te_status,    1, 1, 4, 3 );
   controlsLayout->addWidget( pb_fitcntl,   5, 0, 1, 2 );
   controlsLayout->addWidget( pb_loadfit,   5, 2, 1, 2 );
   controlsLayout->addWidget( pb_plt3d,     6, 0, 1, 2 );
   controlsLayout->addWidget( pb_pltres,    6, 2, 1, 2 );
   controlsLayout->addWidget( lb_scan,      7, 0, 1, 4 );
   controlsLayout->addWidget( lb_from,      8, 0, 1, 1 );
   controlsLayout->addWidget( ct_from,      8, 1, 1, 1 );
   controlsLayout->addWidget( lb_to,        8, 2, 1, 1 );
   controlsLayout->addWidget( ct_to,        8, 3, 1, 1 );
   controlsLayout->addWidget( pb_exclude,   9, 0, 1, 4 );

   // Set initial status text
   te_status->setAlignment( Qt::AlignCenter | Qt::AlignVCenter );
   te_status->setText( tr(
       "Solution not initiated...\n"
       "RMSD:  0.000000,\n"
       "Variance: 0.000000e-05 .\n"
       "Iterations:  0" ) );

   // Add progress bar at the bottom of the left-side layout
   progressLayout   = new QGridLayout();
   QLabel* lb_progress = us_label( tr( "Status:" ) );
   b_progress          = us_progressBar( 0, 100, 0 );
   progressLayout->addWidget( lb_progress,  0, 0, 1, 1 );
   progressLayout->addWidget( b_progress,   0, 1, 1, 3 );
   leftLayout->addLayout( progressLayout );

   connect( pb_help,  SIGNAL( clicked() ), SLOT( help() ) );
   connect( pb_view,  SIGNAL( clicked() ), SLOT( view() ) );
   connect( pb_save,  SIGNAL( clicked() ), SLOT( save() ) );

   pb_fitcntl->setEnabled( false );
   pb_loadfit->setEnabled( false );
   pb_plt3d  ->setEnabled( false );
   pb_pltres ->setEnabled( false );
   pb_exclude->setEnabled( false );
   ct_from   ->setEnabled( false );
   ct_to     ->setEnabled( false );

   edata        = 0;
   resplotd     = 0;
   eplotcd      = 0;
   rbd_pos      = this->pos() + QPoint( 100, 100 );
   epd_pos      = this->pos() + QPoint( 200, 200 );
}

void US_2dsa::analysis_done()
{
   QString analysID  = QDateTime::currentDateTime().toString( "yyMMddhhmm" );
   QString editID    = edata->editID;
   editID            = editID.startsWith( "20" ) ? editID.mid( 2 ) : editID;
   model.editGUID    = edata->editGUID;
   model.description = edata->runID + ".2DSA_e" + editID + "_a" + analysID
      + "_model.11";

qDebug() << "Analysis Done";
qDebug() << "  model components size" << model.components.size();
qDebug() << "  edat0 sdat0 rdat0"
 << edata->value(0,0) << sdata.value(0,0) << rdata.value(0,0);
//*DEBUG*
QString mfilename = US_Settings::dataDir() + "/models/M0000999.xml";
model.write( mfilename );
//*DEBUG*
qDebug() << model.description << "\n to file" << mfilename;

   pb_plt3d ->setEnabled( true );
   pb_pltres->setEnabled( true );

   data_plot();
}

void US_2dsa::data_plot( void )
{
   US_AnalysisBase2::data_plot();

   pb_fitcntl->setEnabled( true );
   pb_loadfit->setEnabled( true );
   ct_from   ->setEnabled( true );
   ct_to     ->setEnabled( true );

   if ( ! dataLoaded )
      return;

   if ( sdata.scanData.size() != edata->scanData.size() )
      return;


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

   double rl = edata->radius( 0 );
   double vh = edata->value( 0, 0 ) * 0.05;
   rl       -= 0.05;
   vh       += ( vh - edata->value( 0, 0 ) ) * 0.05;

   for ( int ii = 0; ii < nscans; ii++ )
   {
      if ( excludedScans.contains( ii ) ) continue;

      int    jj  = 0;
      int    kk  = 0;
      double rr  = 0.0;
      double vv  = 0.0;

      while ( jj < npoints )
      {
         rr    = sdata.radius( jj );
         vv    = sdata.value( ii, jj++ );
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

   data_plot2->replot();

   data_plot1->detachItems();
   data_plot1->clear();

   us_grid( data_plot1 );
   data_plot1->setAxisTitle( QwtPlot::xBottom, tr( "Radius (cm)" ) );
   data_plot1->setAxisTitle( QwtPlot::yLeft,   tr( "OD Difference" ) );
   double rmsd  = 0.0;
   double xmin  = 99999.9;
   double xmax  = -99999.9;
   double ymin  = xmin;
   double ymax  = xmax;

   for ( int jj = 0; jj < npoints; jj++ )
   {
      double rv = sdata.radius( jj );
      ra[ jj ]  = rv;
      xmin      = ( xmin < rv ) ? xmin : rv;
      xmax      = ( xmax > rv ) ? xmax : rv;
   }

   for ( int ii = 0; ii < nscans; ii++ )
   {
      for ( int jj = 0; jj < npoints; jj++ )
      {
         double evalu = edata->value( ii, jj );
         double svalu = sdata .value( ii, jj );
         evalu        = evalu - svalu;
         va[ jj ]     = evalu;
         rmsd        += ( evalu * evalu );
         ymin         = ( ymin < evalu ) ? ymin : evalu;
         ymax         = ( ymax > evalu ) ? ymax : evalu;
      }

      title    = "resids " + QString::number( ii );
      cc       = us_curve( data_plot1, title );
      cc->setPen(   pen_plot );
      cc->setStyle( QwtPlotCurve::Dots );
      cc->setData(  ra, va, npoints );
   }

   data_plot1->setAxisAutoScale( QwtPlot::xBottom );
   data_plot1->setAxisAutoScale( QwtPlot::yLeft   );
   data_plot1->setTitle( tr( "Residuals" ) );

   data_plot1->replot();

   rmsd    /= (double)( nscans * npoints );
   QString pmsg = te_status->toPlainText() +
      tr( "\nRMSD: %1\nVariance: %2" ).arg( sqrt( rmsd ) ).arg( rmsd );
   te_status->setPlainText( pmsg );
}

void US_2dsa::view( void )
{
   // Create US_Editor
   if ( te_results == NULL )
   {
      te_results = new US_Editor( US_Editor::DEFAULT, true, QString(), this );
      te_results->resize( 500, 400 );
      QPoint p = g.global_position();
      te_results->move( p.x() + 30, p.y() + 30 );
      te_results->e->setFont( QFont( US_GuiSettings::fontFamily(),
                                     US_GuiSettings::fontSize() ) );
   }

   int                     index  = lw_triples->currentRow();
   US_DataIO2::EditedData* d      = &dataList[ index ];

   QString sm_results = 
        table_row( tr( "Average Second Moment S: " ),
                   QString::number( index,       'f', 5 ) + " s * 10e-13" );

   // Add results to window
   QString s = 
      "<html><head>\n"
      "<style>td { padding-right: 1em;}</style>\n"
      "</head><body>\n" +
      tr( "<h1>Second Moment Analysis</h1>\n" )   +
      tr( "<h2>Data Report for Run \"" ) + d->runID + tr( "\", Cell " ) + d->cell +
      tr(  ", Wavelength " ) + d->wavelength + "</h2>\n";
   
   s += run_details();
   s += hydrodynamics();
   s += analysis( sm_results );
   s += scan_info();
   s += "</body></html>\n";

   te_results->e->setHtml( s );
   te_results->show();
}

void US_2dsa::save( void )
{
   int                     index  = lw_triples->currentRow();
   US_DataIO2::EditedData* d      = &dataList[ index ];
   QString                 dir    = US_Settings::reportDir();

   if ( ! mkdir( dir, d->runID ) ) return;

   // Note: d->runID is both directory and first segment of file name
   QString filebase = dir + "/" + d->runID + "/" + d->runID + "." + d->cell + 
       + "." + d->channel + "." + d->wavelength;
   
   QString plot1File = filebase + ".sm_plot1.svg";
   QString plot2File = filebase + ".sm_plot2.svg";
   QString textFile  = filebase + ".sm_data.txt";
   QString htmlFile  = filebase + ".sm_report.html";


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
      return;
   }

#if 0
   QTextStream ts_data( &sm_data );

   int scanCount = d->scanData.size();
   int excludes  = le_skipped->text().toInt();
   
   if ( excludes == scanCount )
      ts_data << "No valid scans\n";
   else
   {
      int count = 0;
      for ( int i = excludes; i < scanCount; i++ )
      {
         if ( excludedScans.contains( i ) ) continue;

         ts_data << count + 1 << "\t" << smPoints[ i ] 
                 << "\t" << smSeconds[ i ] << "\n";
         count++;
      }
   }

   sm_data.close();

   // Write report
   QFile report( htmlFile );

   if ( ! report.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
   {
      QMessageBox::warning( this,
            tr( "IO Error" ),
            tr( "Could not open\n" ) + htmlFile + "\n" +
                tr( "\nfor writing" ) );
      return;
   }

   QTextStream report_ts( &report );

   report_ts << "<html><head>\n"
         "<style>td { padding-right: 1em;}</style>\n"
         "</head><body>\n";

   report_ts << tr( "<h1>Second Moment Analysis</h1>\n" ) 
             << tr( "<h2>Data Report for Run \"" ) + d->runID 
                    + tr( "\", Cell " ) + d->cell 
                    + tr(  ", Wavelength " ) + d->wavelength + "</h2>\n";

   QString sm_results = 
        table_row( tr( "Average Second Moment S: " ),
                   QString::number( average_2nd, 'f', 5 ) + " s * 10e-13" );

   report_ts << run_details()
             << hydrodynamics()
             << analysis( sm_results )
             << scan_info();

   // Remove directory from string
   filebase = d->runID + "." + d->cell + "." + d->channel + "." + d->wavelength;

   report_ts << "<h3><a href='" + filebase + ".sm_data.txt'>" 
                 "Text File of Second Moment Plot Data</a></h3>\n"

         "<div><h3>Second Moment Plot</h3>\n"
         "<object data='" + filebase + ".sm_plot1.svg' type='image/svg+xml' "
         "width='"  + QString::number( data_plot1->size().width()  * 1.4 ) + "' "
         "height='" + QString::number( data_plot1->size().height() * 1.4 ) + 
         "'></object></div>\n"
         
         "<div><h3>Velocity Plot</h3>\n"
         "<object data='" + filebase + ".sm_plot2.svg' type='image/svg+xml' "
         "width='"  + QString::number( data_plot2->size().width()  * 1.4 ) + "' "
         "height='" + QString::number( data_plot2->size().height() * 1.4 ) + 
         "'></object></div>\n"

         "</body></html>\n";

   report.close();
#endif

   // Tell user
   QMessageBox::warning( this,
         tr( "Success" ),
         tr( "Wrote:\n" ) 
         + htmlFile  + "\n" 
         + plot1File + "\n" 
         + plot2File + "\n" 
         + textFile  + "\n" );
}

US_DataIO2::EditedData* US_2dsa::mw_editdata()
{
   int row = lw_triples->currentRow();
   edata   = ( row >= 0 ) ? &dataList[ row ] : 0;
qDebug() << "mw_edit: row" << row;

   return edata;
}

US_DataIO2::RawData*        US_2dsa::mw_simdata()  { return &sdata;    }
US_DataIO2::RawData*        US_2dsa::mw_resdata()  { return &rdata;    }
US_Model*                   US_2dsa::mw_model()    { return &model;    }
US_Noise*                   US_2dsa::mw_ti_noise() { return &ti_noise; }
US_Noise*                   US_2dsa::mw_ri_noise() { return &ti_noise; }
QPointer< QProgressBar >    US_2dsa::mw_progress_bar() { return b_progress; }
QPointer< QTextEdit    >    US_2dsa::mw_status_text()  { return te_status;  }

void US_2dsa::open_resplot()
{
qDebug() << "Open Resplot";
   resplotd = new US_ResidPlot( this );
   resplotd->setVisible( true );
}

void US_2dsa::open_3dplot()
{
qDebug() << "Open 3dplot";
   if ( eplotcd )
   {
      epd_pos  = eplotcd->pos();
      eplotcd->close();
   }
   eplotcd = new US_PlotControl( this, &model );
   eplotcd->move( epd_pos );
   eplotcd->show();
}

void US_2dsa::open_fitcntl()
{
   int row = lw_triples->currentRow();
   edata   = ( row >= 0 ) ? &dataList[ row ] : 0;
   edata->dataType = edata->dataType + QString().sprintf(
         " %.6f %.5f %5f", density, viscosity, vbar );
qDebug() << "Open fitcntl";
qDebug() << " dens visc vbar" << density << viscosity << vbar;
   analcd  = new US_AnalysisControl( edata, this );
   analcd->move( epd_pos );
   analcd->show();
}

