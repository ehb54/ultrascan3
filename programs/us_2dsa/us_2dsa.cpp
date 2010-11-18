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

   // Add variance and rmsd to parameters layout
   QLabel* lb_vari     = us_label ( tr( "Variance:" ) );
   le_vari             = us_lineedit( "0.00000" );
   QLabel* lb_rmsd     = us_label ( tr( "RMSD:" ) );
   le_rmsd             = us_lineedit( "0.00000" );
   int row   = parameterLayout->rowCount();
   QPalette gray = US_GuiSettings::editColor();
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );
   le_vari->setPalette(  gray );
   le_rmsd->setPalette(  gray );
   le_vari->setReadOnly( true );
   le_rmsd->setReadOnly( true );
   parameterLayout->addWidget( lb_vari,     row,   0, 1, 1 );
   parameterLayout->addWidget( le_vari,     row,   1, 1, 1 );
   parameterLayout->addWidget( lb_rmsd,     row,   2, 1, 1 );
   parameterLayout->addWidget( le_rmsd,     row++, 3, 1, 1 );

   // Reconstruct controls layout with some 2dsa-specific elements
   row       = 0;
   controlsLayout->addWidget( lb_scan,      row++, 0, 1, 4 );
   controlsLayout->addWidget( lb_from,      row,   0, 1, 1 );
   controlsLayout->addWidget( ct_from,      row,   1, 1, 1 );
   controlsLayout->addWidget( lb_to,        row,   2, 1, 1 );
   controlsLayout->addWidget( ct_to,        row++, 3, 1, 1 );
   controlsLayout->addWidget( pb_exclude,   row++, 0, 1, 4 );
   controlsLayout->addWidget( lb_analysis,  row++, 0, 1, 4 );
   controlsLayout->addWidget( pb_fitcntl,   row++, 0, 1, 2 );
   controlsLayout->addWidget( pb_plt3d,     row,   0, 1, 2 );
   controlsLayout->addWidget( pb_pltres,    row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_status,    row,   0, 3, 1 );
   controlsLayout->addWidget( te_status,    row,   1, 3, 3 );
   row      += 3;

   // Set initial status text
   te_status->setAlignment( Qt::AlignCenter | Qt::AlignVCenter );
   te_status->setText( tr(
       "Solution not initiated...\n"
       "RMSD:  0.000000,\n"
       "Variance: 0.000000e-05 .\n"
       "Iterations:  0" ) );
   te_status->setPalette(  gray );

   connect( pb_help,  SIGNAL( clicked() ), SLOT( help() ) );
   connect( pb_view,  SIGNAL( clicked() ), SLOT( view() ) );
   connect( pb_save,  SIGNAL( clicked() ), SLOT( save() ) );

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
}

// slot to handle the completion of 2-D spectrum analysis
void US_2dsa::analysis_done( int updflag )
{
   if ( updflag < 0 )
   {
      qApp->processEvents();
      return;
   }

   bool autoplot     = updflag > 0;

   QString analysID  = QDateTime::currentDateTime().toString( "yyMMddhhmm" );
   QString editID    = edata->editID;
   editID            = editID.startsWith( "20" ) ? editID.mid( 2 ) : editID;
   model.editGUID    = edata->editGUID;
   model.description = edata->runID + "_2DSA_e" + editID + "_a" + analysID
      + ".model.11";

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

   if ( autoplot )
   {
      open_resplot();
      open_3dplot();
   }
}

// plot the data
void US_2dsa::data_plot( void )
{
   US_AnalysisBase2::data_plot();      // plot experiment data

   pb_fitcntl->setEnabled( true );
   ct_from   ->setEnabled( true );
   ct_to     ->setEnabled( true );

   if ( ! dataLoaded )
      return;

   if ( sdata.scanData.size() != edata->scanData.size() )
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

   double rl = edata->radius( 0 );
   double vh = edata->value( 0, 0 ) * 0.05;
   rl       -= 0.05;
   vh       += ( vh - edata->value( 0, 0 ) ) * 0.05;

   // plot the simulation data in red on top of experiment data
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
      for ( int jj = 0; jj < npoints; jj++ )
      {
         double evalu = edata->value( ii, jj )
                      - sdata .value( ii, jj );
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

   data_plot1->replot();

   // report on variance and rmsd
   vari    /= (double)( nscans * npoints );
   rmsd     = sqrt( vari );
   le_vari->setText( QString::number( vari ) );
   le_rmsd->setText( QString::number( rmsd ) );
}

// view data report
void US_2dsa::view( void )
{
   // Create US_Editor
   if ( te_results == NULL )
   {
      te_results = new US_Editor( US_Editor::DEFAULT, true, QString(), this );
      te_results->resize( 560, 400 );
      QPoint p = g.global_position();
      te_results->move( p.x() + 30, p.y() + 30 );
      te_results->e->setFont( QFont( US_GuiSettings::fontFamily(),
                                     US_GuiSettings::fontSize() ) );
   }

   int                     index  = lw_triples->currentRow();
   US_DataIO2::EditedData* d      = &dataList[ index ];

   // Add results to window
   QString s = 
      "<html><head>\n"
      "<style>td { padding-right: 1em;}</style>\n"
      "</head><body>\n" +
      tr( "<h1>2-Dimensional Spectrum Analysis</h1>\n" )   +
      tr( "<h2>Data Report for Run \"" ) + d->runID +
      tr( "\", Cell " ) + d->cell +
      tr(  ", Wavelength " ) + d->wavelength + "</h2>\n";
   
   s += run_details();
   s += hydrodynamics();
   s += scan_info();
   s += distrib_info();
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

// Return pointers to main window data and GUI elements

US_DataIO2::RawData*        US_2dsa::mw_simdata()      { return &sdata;    }
US_DataIO2::RawData*        US_2dsa::mw_resdata()      { return &rdata;    }
US_Model*                   US_2dsa::mw_model()        { return &model;    }
US_Noise*                   US_2dsa::mw_ti_noise()     { return &ti_noise; }
US_Noise*                   US_2dsa::mw_ri_noise()     { return &ri_noise; }
QPointer< QTextEdit    >    US_2dsa::mw_status_text()  { return te_status;  }

// Open residuals plot window
void US_2dsa::open_resplot()
{
qDebug() << "Open Resplot";
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

// Open 3-D plot window
void US_2dsa::open_3dplot()
{
qDebug() << "Open 3dplot";
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
void US_2dsa::open_fitcntl()
{
   int row = lw_triples->currentRow();
   edata   = ( row >= 0 ) ? &dataList[ row ] : 0;
   edata->dataType = edata->dataType + QString().sprintf(
         " %.6f %.5f %5f", density, viscosity, vbar );
qDebug() << "Open fitcntl";
qDebug() << " dens visc vbar" << density << viscosity << vbar;

   if ( analcd != 0 )
   {
      acd_pos  = analcd->pos();
      analcd->close();
   }
   else
      acd_pos  = this->pos() + QPoint(  500,  50 );

   analcd  = new US_AnalysisControl( edata, this );
   analcd->move( acd_pos );
   analcd->show();
   qApp->processEvents();
}

// Distribution information HTML string
QString US_2dsa::distrib_info()
{
   int ncomp     = model.components.size();
   
   if ( ncomp == 0 )
      return "";

   QString mstr = tr( "<h3>Data Analysis Settings:</h3>\n" ) + "<table>\n";

   mstr += table_row( tr( "Number of Components:" ),
                      QString::number( ncomp ) );
   mstr += table_row( tr( "Residual RMS Deviation:" ),
                      QString::number( rmsd )  );

   double sum_mw  = 0.0;
   double sum_s   = 0.0;
   double sum_D   = 0.0;
   double sum_c   = 0.0;

   for ( int ii = 0; ii < ncomp; ii++ )
   {
      double conc = model.components[ ii ].signal_concentration;
      sum_c      += conc;
      sum_mw     += model.components[ ii ].mw * conc;
      sum_s      += model.components[ ii ].s  * conc;
      sum_D      += model.components[ ii ].D  * conc;
   }

   mstr += table_row( tr( "Weight Average s20,W:" ),
                      QString().sprintf( "%6.4e\n", ( sum_s  / sum_c ) ) );
   mstr += table_row( tr( "Weight Average D20,W:" ),
                      QString().sprintf( "%6.4e\n", ( sum_D  / sum_c ) ) );
   mstr += table_row( tr( "W.A. Molecular Weight:" ),
                      QString().sprintf( "%6.4e\n", ( sum_mw / sum_c ) ) );
   mstr += table_row( tr( "Total Concentration:" ),
                      QString().sprintf( "%6.4e\n", sum_c ) );
   mstr += "</table>\n\n";

   mstr += tr( "<h3>Distribution Information:</h3>\n" ) + "<table>\n";
   mstr += table5_row( tr( "Molecular Wt." ), tr( "S 20,W" ), tr( "D 20,W" ),
                       tr( "f / f0" ), tr( "Concentration" ) );

   for ( int ii = 0; ii < ncomp; ii++ )
   {
      double conc = model.components[ ii ].signal_concentration;
      double perc = 100.0 * conc / sum_c;
      mstr       += table5_row(
            QString().sprintf( "%10.4e", model.components[ ii ].mw   ),
            QString().sprintf( "%10.4e", model.components[ ii ].s    ),
            QString().sprintf( "%10.4e", model.components[ ii ].D    ),
            QString().sprintf( "%10.4e", model.components[ ii ].f_f0 ),
            QString().sprintf( "%10.4e (%5.2f %%)", conc, perc       ) );
   }

   mstr += "</table>";
   
   return mstr;
}

// Table HTML table row string for 5 columns
QString US_2dsa::table5_row( const QString& s1, const QString& s2,
                             const QString& s3, const QString& s4,
                             const QString& s5 )
{
   return "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>" + s3
       + "</td><td>" + s4 + "</td><td>" + s5 + "</td></tr>\n";
}

