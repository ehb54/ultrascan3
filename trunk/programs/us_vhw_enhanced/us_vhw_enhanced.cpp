//! \file us_vhw_enhanced.cpp

#include <QApplication>

#include <uuid/uuid.h>

#include "us_dataIO.h"
#include "us_vhw_enhanced.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_math.h"
#include "us_matrix.h"

// main program
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_vHW_Enhanced w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

// US_vHW_Enhanced class constructor
US_vHW_Enhanced::US_vHW_Enhanced() : US_AnalysisBase()
{
   // set up the GUI (mostly handled in US_AnalysisBase)

   setWindowTitle( tr( "Enhanced van Holde - Weischet Analysis:" ) );

   pb_dstrpl     = us_pushbutton( tr( "Distribution Plot" ) );
   pb_dstrpl->setEnabled( false );
   connect( pb_dstrpl, SIGNAL( clicked() ),
            this,       SLOT(  distr_plot() ) );

   pb_selegr     = us_pushbutton( tr( "Select Groups" ) );
   pb_selegr->setEnabled( true );
   connect( pb_selegr, SIGNAL( clicked() ),
            this,       SLOT(  sel_groups() ) );

   parameterLayout->addWidget( pb_dstrpl, 2, 0, 1, 2 );
   parameterLayout->addWidget( pb_selegr, 2, 2, 1, 2 );

   QLayoutItem* litem = parameterLayout->itemAtPosition( 1, 0 );
   QWidget*     witem = litem->widget();

   if ( witem )
   {  // change "vbar" to "Vbar" on pushbutton
      QPushButton* bitem = (QPushButton*)witem;
      bitem->setText( "Vbar" );
   }
   else
      qDebug() << "parLay 1,0 item is NOT widget";

   QLabel* lb_analysis     = us_banner( tr( "Analysis Controls"      ) );
   QLabel* lb_scan         = us_banner( tr( "Scan Control"           ) );
   QLabel* lb_smoothing    = us_label ( tr( "Data Smoothing:"        ) );
   QLabel* lb_boundPercent = us_label ( tr( "% of Boundary:"         ) );
   QLabel* lb_boundPos     = us_label ( tr( "Boundary Position (%):" ) );

   QLabel* lb_from         = us_label ( tr( "From:" ) );
   QLabel* lb_to           = us_label ( tr( "to:"   ) );

   lb_tolerance  = us_label( tr( "Back Diffusion Tolerance:" ) );
   lb_tolerance->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_tolerance = us_counter( 3, 0.0, 1000.0, 0.001 );
   ct_tolerance->setStep( 0.001 );
   ct_tolerance->setEnabled( true );
   connect( ct_tolerance, SIGNAL( valueChanged(   double ) ),
            this,          SLOT(  update_bdtoler( double ) ) );

   lb_division   = us_label( tr( "Divisions:" ) );
   lb_division->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_division  = us_counter( 3, 0.0, 1000.0, 50.0 );
   ct_division->setStep( 1 );
   ct_division->setEnabled( true );
   connect( ct_division, SIGNAL( valueChanged(  double ) ),
            this,         SLOT(  update_divis(  double ) ) );

   controlsLayout->addWidget( lb_analysis       , 0, 0, 1, 4 );
   controlsLayout->addWidget( lb_tolerance      , 1, 0, 1, 2 );
   controlsLayout->addWidget( ct_tolerance      , 1, 2, 1, 2 );
   controlsLayout->addWidget( lb_division       , 2, 0, 1, 2 );
   controlsLayout->addWidget( ct_division       , 2, 2, 1, 2 );
   controlsLayout->addWidget( lb_smoothing      , 3, 0, 1, 2 );
   controlsLayout->addWidget( ct_smoothing      , 3, 2, 1, 2 );
   controlsLayout->addWidget( lb_boundPercent   , 4, 0, 1, 2 );
   controlsLayout->addWidget( ct_boundaryPercent, 4, 2, 1, 2 );
   controlsLayout->addWidget( lb_boundPos       , 5, 0, 1, 2 );
   controlsLayout->addWidget( ct_boundaryPos    , 5, 2, 1, 2 );
   controlsLayout->addWidget( lb_scan           , 6, 0, 1, 4 );
   controlsLayout->addWidget( lb_from           , 7, 0 );
   controlsLayout->addWidget( ct_from           , 7, 1 );
   controlsLayout->addWidget( lb_to             , 7, 2 );
   controlsLayout->addWidget( ct_to             , 7, 3 );
   controlsLayout->addWidget( pb_exclude        , 8, 0, 1, 4 );


}

// load data
void US_vHW_Enhanced::load( void )
{
   // query the directory where .auc and .xml file are
   workingDir = QFileDialog::getExistingDirectory( this,
         tr( "Raw Data Directory" ),
         US_Settings::resultDir(),
         QFileDialog::DontResolveSymlinks );

   if ( workingDir.isEmpty() )
      return;

   reset_data();

   // insure we have a .auc file
   QStringList nameFilters = QStringList( "*.auc" );
   workingDir.replace( "\\", "/" );
   QDir wdir( workingDir );
   files    = wdir.entryList( nameFilters,
         QDir::Files | QDir::Readable, QDir::Name );

   if ( files.size() == 0 )
   {
      QMessageBox::warning( this,
            tr( "No Files Found" ),
            tr( "There were no files of the form *.auc\n"
                "found in the specified directory." ) );
      return;
   }

   // Look for cell / channel / wavelength combinations
   lw_triples->clear();
   dataList.clear();
   rawList.clear();
   excludedScans.clear();
   triples.clear();

   // Read all data
   if ( workingDir.right( 1 ) != "/" )
      workingDir += "/"; // Ensure trailing '/'

   files       = wdir.entryList( QStringList() << "*.*.*.*.*.*.xml",
         QDir::Files | QDir::Readable, QDir::Name );

   for ( int ii = 0; ii < files.size(); ii++ )
   {
      QString file     = files[ ii ];
      QStringList part = file.split( "." );
      QString filename = workingDir + file;
 
      // load edit data (xml) and raw data (auc)
      int result = US_DataIO::loadData( workingDir, file, dataList, rawList );

      if ( result != US_DataIO::OK )
      {
         QMessageBox::warning( this,
            tr( "UltraScan Error" ),
            tr( "Could not read edit file.\n" ) 
            + US_DataIO::errorString( result ) + "\n" + filename );
         return;
      }

      QString t = part[ 3 ] + " / " + part[ 4 ] + " / " + part[ 5 ];
      runID     = part[ 0 ];
      editID    = part[ 1 ];

      if ( ! triples.contains( t ) )
      {  // update ListWidget with cell / channel / wavelength triple
         triples << t;
         lw_triples->addItem( t );
      } 
   }

   lw_triples->setCurrentRow( 0 );

   US_DataIO::editedData* d = &dataList[ 0 ];
   le_id->setText( d->runID + " / " + d->editID );
   double tempera = 0.0;
   int    scnknt  = d->scanData.size();

   for ( int ii = 0; ii < scnknt; ii++ )
   {  /// sum temperature from each scan to determine average
      tempera += d->scanData[ ii ].temperature;
   }

   // display average temperature and data description
   tempera  /= (double)scnknt;
   QString t = QString::number( tempera, 'f', 1 ) + " " + QChar( 176 ) + "C";
   le_temp->setText( t );
   te_desc->setText( d->description );

   // Enable pushbuttons
   pb_details   ->setEnabled( true );
   pb_save      ->setEnabled( true );
   pb_view      ->setEnabled( true );

   data_plot();

   pb_details->disconnect( );
   connect( pb_details, SIGNAL( clicked() ),
            this,       SLOT(  details() ) );
}

// details
void US_vHW_Enhanced::details( void )
{
   US_RunDetails* dialog
      = new US_RunDetails( rawList, runID, workingDir, triples );
   dialog->exec();
   qApp->processEvents();
   delete dialog;
}

// distribution plot
void US_vHW_Enhanced::distr_plot(  void )
{
   QVector< US_DataIO::scan >* sD;
   QColor colorfg( US_GuiSettings::plotCurve() );
   QColor colorbg( US_GuiSettings::plotCanvasBG() );
   double xlo = 1.0e+30;
   double xhi = 1.0e-30;
   double ylo = 1.0e+30;
   double yhi = 1.0e-30;

   vdata     = rawList[ 0 ];
   sD        = &(vdata.scanData);
   int nscn  = sD->size();
   fgPen     = QPen( colorfg );
   bgPen     = QPen( colorbg );
   fgSym.setStyle( QwtSymbol::Ellipse );
   fgSym.setBrush( colorfg );
   fgSym.setPen(   fgPen );
   fgSym.setSize(  2 );
   bgSym     = fgSym;
   bgSym.setBrush( colorbg );
   bgSym.setPen(   bgPen );
qDebug() << "scanData size=" << nscn;

   for ( int ii = 0; ii < nscn; ii++ )
   {
      QString cnam = tr( "Scan Curve %1" ).arg( ii+1 );
      curve     = us_curve( data_plot2, cnam );
      curve->setSymbol( fgSym );
      curve->setStyle(  QwtPlotCurve::Lines );
      curve->attach(    data_plot2 );

      const QVector< US_DataIO::reading >& rD
                = sD->at( ii ).readings;
      int npts  = rD.size();
      xx        = new double[ npts ];
      yy        = new double[ npts ];
qDebug() << "  scan " << ii << "  npts=" << npts;

      for ( int jj = 0; jj < npts; jj++ )
      {
         xx[ jj ]  = rD.at( jj ).d.radius;
         yy[ jj ]  = rD.at( jj ).value;
         xlo       = min( xlo, xx[ jj ] );
         xhi       = max( xhi, xx[ jj ] );
         ylo       = ( ylo < yy[ jj ] ) ? ylo : yy[ jj ];
         yhi       = ( yhi > yy[ jj ] ) ? yhi : yy[ jj ];
      }
qDebug() << "    x0,y0 " << xx[0] << "," << yy[0];
qDebug() << "    xn,yn " << xx[npts-1] << "," << yy[npts-1];

      //curve->setRawData( xx, yy, npts );
      curve->setData( xx, yy, npts );

      delete [] xx;
      delete [] yy;
   }
qDebug() << "  xl,xh " << xlo << "," << xhi;
qDebug() << "  yl,yh " << ylo << "," << yhi;
   double xpad = ( xhi - xlo ) / 30.0;
   double ypad = ( yhi - ylo ) / 30.0;
   data_plot2->setAxisScale( QwtPlot::xBottom, xlo-xpad, xhi+xpad );
   data_plot2->setAxisScale( QwtPlot::yLeft,   ylo-ypad, yhi+ypad );

   data_plot2->replot();
}

// save the enhanced data
void US_vHW_Enhanced::save_data( void )
{ 
   QString filter = tr( "GA data files (*.gadistro.dat);;" )
      + tr( "Any files (*)" );
   QString fsufx = "." + cell + wavelength + ".gadistro.dat";
   QString fname = run_name + fsufx;
   fname         = QFileDialog::getSaveFileName( this,
      tr( "Save GA Data File" ),
      US_Settings::resultDir() + "/" + fname,
      filter,
      0, 0 );

}

void US_vHW_Enhanced::print_data(  void ) {}
void US_vHW_Enhanced::view_report( void ) {}
void US_vHW_Enhanced::show_densi(  void ) {}
void US_vHW_Enhanced::show_visco(  void ) {}

void US_vHW_Enhanced::show_vbar(   void )
{
   //this->get_vbar();
}

// reset the GUI
void US_vHW_Enhanced::reset_data( void )
{
#if 0
   data_plot1->detachItems( );
   data_plot1->replot();
   epick      = new US_PlotPicker( data_plot1 );
#endif
}

void US_vHW_Enhanced::sel_groups(  void ) {}
void US_vHW_Enhanced::excl_sscan(  void ) {}
void US_vHW_Enhanced::excl_srange( void ) {}

// update density
void US_vHW_Enhanced::update_density( double dval )
{
   densit    = dval;
}

// update viscosity
void US_vHW_Enhanced::update_viscosi( double dval )
{
   viscos    = dval;
}

void US_vHW_Enhanced::update_vbar(    double ) {}
void US_vHW_Enhanced::update_bdtoler( double ) {}
void US_vHW_Enhanced::update_divis(   double ) {}
void US_vHW_Enhanced::update_dsmoo(   double ) {}
void US_vHW_Enhanced::update_pcbound( double ) {}
void US_vHW_Enhanced::update_boundpo( double ) {}
void US_vHW_Enhanced::update_exsscan( double ) {}
void US_vHW_Enhanced::update_exscrng( double ) {}

