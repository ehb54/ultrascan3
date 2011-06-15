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
   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   QPushButton* pb_loadda  = us_pushbutton( tr( "Load Data" ) );
   QPushButton* pb_saveda  = us_pushbutton( tr( "Save Data" ) );
   QPushButton* pb_reset   = us_pushbutton( tr( "Reset"     ) );
   QPushButton* pb_help    = us_pushbutton( tr( "Help"      ) );
   QPushButton* pb_close   = us_pushbutton( tr( "Close"     ) );

   QLabel* lb_distrtype  = us_banner( tr( "Select Distribution Type:" ) );
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
   leftLayout->addWidget( pb_reset,     row,   0, 1, 4 );
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

   connect( pb_loadda, SIGNAL( clicked() ),
            this,      SLOT(   load()    ) );
   connect( pb_saveda, SIGNAL( clicked() ),
            this,      SLOT(   save()    ) );
   connect( pb_help,   SIGNAL( clicked() ),
            this,      SLOT(   help()    ) );
   connect( pb_close,  SIGNAL( clicked() ),
            this,      SLOT(   close()   ) );

   QBoxLayout* plot = new US_Plot( data_plot1,
         tr( "G(s) Distributions" ),
         tr( "Sedimentation Coefficient (20W x 1e13)" ),
         tr( "Boundary Fraction (%)" ) );

   QwtText qtitle( tr( "Relative Frequency" ) );
   qtitle.setFont( QFont( US_GuiSettings::fontFamily(),
                          US_GuiSettings::fontSize(), QFont::Bold ) );
   qtitle.setText( tr( "Relative Frequency" ) );

   data_plot1->setMinimumSize( 500, 500 );
   data_plot1->enableAxis  ( QwtPlot::yRight, true );
   data_plot1->setAxisScale( QwtPlot::xBottom, 1.0,  10.0 );
   data_plot1->setAxisScale( QwtPlot::yLeft,   0.0, 100.0 );
   data_plot1->setAxisScale( QwtPlot::yRight,  0.0,   6.0 );
   data_plot1->setAxisTitle( QwtPlot::yRight, qtitle );

   //data_plot1->selectLegendPos( 4 ); 

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
   le_distname->setMinimumWidth ( ww );
   leftLayout ->setColumnMinimumWidth( 0, ww );
   leftLayout ->setColumnMinimumWidth( 1, ww );
   leftLayout ->setColumnMinimumWidth( 2, ww );
   leftLayout ->setColumnMinimumWidth( 3, ww );
   leftLayout ->setColumnMinimumWidth( 4, ww );
   leftLayout ->setColumnMinimumWidth( 5, ww );
   leftLayout ->setColumnMinimumWidth( 6, ww );
   leftLayout ->setColumnMinimumWidth( 7, ww );
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

            ddesc.dseds  << sedc;
            ddesc.bfracs << bound;
         }
int kk = ddesc.dseds.size()-1;
DbgLv(1) << "Distro runid" << ddesc.runID << " triple" << ddesc.triple << kk;
DbgLv(1) << "  0 sed frac" << ddesc.dseds[0] << ddesc.bfracs[0];
DbgLv(1) << "  kk sed frac" << ddesc.dseds[kk] << ddesc.bfracs[kk];

         distros << ddesc;
         distIDs << runid + "." + tripl;
         fi.close();
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

// Reset data: clear plots and remove all loaded data
void US_vHW_Combine::reset( void )
{
}

// Data plot
void US_vHW_Combine::data_plot( void )
{

   data_plot1->detachItems();
   data_plot1->setAxisAutoScale( QwtPlot::xBottom );
   data_plot1->setAxisAutoScale( QwtPlot::yLeft );
   data_plot1->setAxisAutoScale( QwtPlot::yRight );

   QwtPlotCurve* curve;
   QwtSymbol     sym;
   sym.setStyle( QwtSymbol::Ellipse );
   sym.setPen  ( QPen( Qt::blue ) );
   sym.setBrush( QBrush( Qt::white ) );
   sym.setSize ( 8 );
}

// save the plot data
void US_vHW_Combine::save( void )
{ 
}

