//! \file us_vhw_combine.cpp

#include <QApplication>
#include <QtSvg>

#include "us_vhw_combine.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_matrix.h"
#include "us_constants.h"
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

   QPushButton* pb_loadda  = us_pushbutton( tr( "Load Data"  ) );
   QPushButton* pb_saveda  = us_pushbutton( tr( "Save Data"  ) );
   QPushButton* pb_resetd  = us_pushbutton( tr( "Reset Data" ) );
   QPushButton* pb_resetp  = us_pushbutton( tr( "Reset Plot" ) );
   QPushButton* pb_help    = us_pushbutton( tr( "Help"       ) );
   QPushButton* pb_close   = us_pushbutton( tr( "Close"      ) );

   QLabel* lb_distrtype  = us_banner(
         tr( "Select Distribution Plot Type(s):" ) );
   QLabel* lb_runinfo    = us_banner( tr( "Information for this Run:" ) );
   QLabel* lb_runid      = us_label ( tr( "Current Run ID:" ) );
   QLabel* lb_distname   = us_label ( tr( "Distribution Name:" ) );
   QLabel* lb_runids     = us_banner( tr( "Run IDs:"                  ) );
   QLabel* lb_triples    = us_banner( tr( "Cell / Channel / Wavelength:" ) );

   QLayout* lo_distrib  = us_checkbox( tr( "Integral" ), ck_distrib,  true  );
   QLayout* lo_envelope = us_checkbox( tr( "Envelope" ), ck_envelope, false );

   le_runid      = us_lineedit( "(current run ID)" );
   le_distname   = us_lineedit( "(output distribution name)" );
   lw_runids     = us_listwidget();
   lw_triples    = us_listwidget();

   int row = 0;
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
   leftLayout->addWidget( lb_distname,  row,   0, 1, 3 );
   leftLayout->addWidget( le_distname,  row++, 3, 1, 5 );
   leftLayout->addWidget( lb_runids,    row++, 0, 1, 8 );
   leftLayout->addWidget( lw_runids,    row,   0, 3, 8 );
   row    += 3;
   leftLayout->addWidget( lb_triples,   row++, 0, 1, 8 );
   leftLayout->addWidget( lw_triples,   row,   0, 5, 8 );
   row    += 5;
   leftLayout->setRowStretch( row, 1 );

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

   connect( lw_runids,   SIGNAL( currentRowChanged( int ) ),
            this,        SLOT(   runid_select(      int ) ) );
   connect( lw_triples,  SIGNAL( currentRowChanged( int ) ),
            this,        SLOT(   triple_select(     int ) ) );

   QBoxLayout* plot = new US_Plot( data_plot1,
         tr( "G(s) Distributions" ),
         tr( "Sedimentation Coefficient (20W x 1e+13)" ),
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
   le_distname->setText( "(output distribution name)" );

   adjustSize();
   int hh  = lb_distname->height();
   int ww  = lb_distname->width() / 3;
   lw_runids  ->setMinimumHeight( hh * 3 );
   lw_triples ->setMinimumHeight( hh * 5 );
   le_distname->setMinimumWidth ( ww * 5 );
   for ( int ii = 0; ii < 8; ii++ )
      leftLayout ->setColumnMinimumWidth( ii, ww );
   leftLayout ->setColumnStretch     ( 0, 1  );
   leftLayout ->setColumnStretch     ( 1, 1  );
   adjustSize();
}

// Load data
void US_vHW_Combine::load( void )
{
   QString dir = QFileDialog::getExistingDirectory( this,
         tr( "Select Run ID Directory:" ),
         US_Settings::resultDir(),
         QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );

   if ( dir.isEmpty() )   return;

   QString runid = dir.section( "/", -1, -1 );

   QStringList datfilt( "vHW.*.distrib.dat" );
   QStringList dfiles = QDir( dir )
      .entryList( datfilt, QDir::Files, QDir::Name );
   int ndfile = dfiles.size();
   int distx  = distros.size();

   if ( ndfile == 0 )     return;

   for ( int ii = 0; ii < ndfile; ii++ )
   {
      QString fname = dfiles[ ii ];
      QString fpath = dir + "/" + fname;
      QString tripl = fname.section( ".", 1, 1 );
      DistrDesc ddesc;
      ddesc.runID   = runid;
      ddesc.triple  = tripl;

      QFile fi( fpath );
      if ( fi.open( QIODevice::ReadOnly | QIODevice::Text ) )
      {
         QTextStream ts( &fi );
         QString fline = ts.readLine().simplified();

         while ( !ts.atEnd() )
         {
            fline = ts.readLine().simplified();
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

         distros << ddesc;
         distIDs << runid + "." + tripl;
         fi.close();
         distx++;
      }
   }

   lw_runids->addItem( runid );
   le_runid ->setText( runid );

   if ( distros.size() == ndfile )
   {
      le_distname->setText( runid );
      adjustSize();
   }
}

// Reset data: remove all loaded data and clear plots
void US_vHW_Combine::reset_data( void )
{
   distros.clear();
   distIDs.clear();

   lw_runids  ->clear();
   lw_triples ->clear();
   le_runid   ->clear();
   le_distname->clear();

   reset_plot();
}

// Reset plot:  Clear plots and lists of plotted data
void US_vHW_Combine::reset_plot( void )
{
   data_plot1->detachItems();
   data_plot1->clear();
   data_plot1->replot();

   pdistrs.clear();
   pdisIDs.clear();

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
   QVector< double > xvec( ndispt );
   QVector< double > yvec( ndispt );
   QVector< double > svec( ndispt );
   QVector< double > bvec( ndispt );
   double* xx  = xvec.data();
   double* yy  = yvec.data();
   double* xs  = svec.data();
   double* yb  = bvec.data();

   QString dcID = "DPoint - " + distrID;
   QString lcID = "DLine - "  + distrID;
   QString ecID = "ELine - "  + distrID;
   QwtPlotCurve* dcurve;
   QwtPlotCurve* lcurve;
   QwtPlotCurve* ecurve;
   QPen          dlpen( QPen( Qt::yellow ) );
   //QPen          elpen( QPen( QBrush( ddesc.color ), 5.0 ) );
   QPen          elpen( QPen( QBrush( ddesc.color ), 3.0 ) );

   if ( dplot )
   {  // Build data and then curves for distribution plot
      for ( int ii = 0; ii < ndispt; ii++ )
      {
         xx[ ii ]   = ddesc.dsedcs[ ii ];
         yy[ ii ]   = ddesc.bfracs[ ii ];
      }

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
   {  // Build data then curves for envelope plot
      int   steps;
      int   nSensit   = 50;
      double min_cept = 1.0e+6;
      double max_cept = 1.0e-6;
      double sed_bin  = ddesc.dsedcs[ 0 ];
      double div_scl  = (double)nSensit * (double)ndispt * 0.01;

      for ( int ii = 0; ii < ndispt; ii++ )
      {  // Get the min,max sedcoeff intercept values
         min_cept     = qMin( min_cept, ddesc.dsedcs[ ii ] );
         max_cept     = qMax( max_cept, ddesc.dsedcs[ ii ] );
      }

      sed_bin       = ( max_cept - min_cept ) / div_scl;
      double mxstep = max_cept * 4.0 / 3.0;
      steps         = (int)( mxstep / sed_bin );

      if ( steps > ndispt )
      {  // Insure we have enough room in our arrays
         svec.resize( steps );
         bvec.resize( steps );
         xs = svec.data();
         yb = bvec.data();
      }

      int kstep = 0;

      for ( int ii = 0; ii < steps; ii++ )
      {  // Accumulate sedcoeffs,bins for non-zero bin counts
         double sedlo = sed_bin * (double)ii;
         double sedhi = sedlo + sed_bin;
         double sval  = ( sedlo + sedhi ) * 0.5;
         int kbin   = 0;

         for ( int jj = 0; jj < ndispt; jj++ )
         {  // Count bins that fall into the current range
            double sedc  = ddesc.dsedcs[ jj ];
            if ( sedc >= sedlo  &&  sedc < sedhi )
               kbin++;
         }

         if ( kbin < 1 )  continue;  // Skip zeroes

         xs[ kstep ]  = sval;
         yb[ kstep ]  = (double)kbin;
//DbgLv(1) << "   kstep xs yb" << kstep << xs[kstep] << yb[kstep];
         kstep++;
      }

      ecurve        = us_curve( data_plot1, ecID );
      ecurve->setStyle ( QwtPlotCurve::Lines );
      ecurve->setPen   ( elpen );
      ecurve->setData  ( xs, yb, kstep );
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

// save the plot data
void US_vHW_Combine::save( void )
{ 
}

// RunID selected
void US_vHW_Combine::runid_select( int row )
{
DbgLv(1) << "RunIDSel:row" << row;
   if ( row < 0 )  return;
   QListWidgetItem* item = lw_runids->item( row );
   runID    = item->text();
DbgLv(1) << "RunIDSel:runID" << runID;
   le_runid ->setText( runID );

   lw_triples->clear();

   for ( int ii = 0; ii < distros.size(); ii++ )
   {
      if ( distros[ ii ].runID == runID )
      {
         lw_triples->addItem( distros[ ii ].triple );
      }
   }
}

// Triple selected
void US_vHW_Combine::triple_select( int row )
{
DbgLv(1) << "TripleSel:row" << row;
   if ( row < 0 )  return;
   QListWidgetItem* item = lw_triples->item( row );
   triple   = item->text();
DbgLv(1) << "TripleSel:triple" << triple;

   DistrDesc ddesc;

   for ( int ii = 0; ii < distros.size(); ii++ )
   {
      ddesc = distros[ ii ];

      if ( ddesc.runID == runID  &&  ddesc.triple == triple )
         break;
   }

   QString distrID = ddesc.runID + "." + ddesc.triple;

   if ( ! pdisIDs.contains( distrID ) )
   {
      pdistrs << ddesc;
      pdisIDs << distrID;

      plot_distr( ddesc, distrID );
   }

   else
      plot_data();
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
   symbols << (int)( QwtSymbol::Cross     );
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

