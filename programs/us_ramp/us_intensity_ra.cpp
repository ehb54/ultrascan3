//! \file us_intensity_ra.cpp

#include "us_intensity_ra.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_util.h"
#include "us_gui_util.h"
#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#define setSymbol(a)       setSymbol(*a)
#endif

US_IntensityRa::US_IntensityRa( 
      const QString runID, 
      const QString triple, 
      const QVector< double >& data,
      const QVector< double >& scan ) 
    : US_WidgetsDialog( nullptr, Qt::WindowFlags() ), dataIn( data ), scanIn( scan )
{
   setWindowTitle( tr( "Details for Average Intensity Values" ) );
   setPalette( US_GuiSettings::frameColor() );
   setAttribute( Qt::WA_DeleteOnClose );

   QGridLayout* main = new QGridLayout( this );
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );

   int row  = 0;

   // Plot Rows
   QBoxLayout* plot = new US_Plot( data_plot,
        tr( "Intensity Profile" ),
        tr( "Scan Number" ),
        tr( "Intensity" ));

   data_plot->setMinimumSize( 800, 200 );

   QwtPlotGrid* grid = us_grid( data_plot );
   grid->enableXMin( false );

   main->addLayout( plot, row, 0, 5, 6 );
   row += 6;

   QHBoxLayout* buttons = new QHBoxLayout();

   QPushButton* pb_close = us_pushbutton( tr( "Close" ) );
   pb_close->setFixedWidth( 100 );
   connect( pb_close, SIGNAL( clicked() ), SLOT( close() ) );
   buttons->insertStretch( 0, 10 );
   buttons->addWidget( pb_close );

   main->addLayout( buttons, row++, 2, 1, 4 );
   QString ctriple = US_Util::compressed_triple( triple );

   if ( triple.contains( "-" ) )
   {  // Handle special MWL triple with wavelength range
      data_plot->setTitle( tr( "Intensity Profile\n"
                               "for wavelengths: " )
                            + ctriple.mid( 2 ) );
      data_plot->setAxisTitle( QwtPlot::xBottom,
                               tr( "Wavelength.Scan Number" ) );
   }

   draw_plot( dataIn, scanIn );

   // Just automatically save the plot in a file
   QString dir    = US_Settings::reportDir() + "/" + runID;
   if ( ! QDir( dir ).exists() )      // make sure the directory exists
      QDir().mkdir( dir );

   QString filename = dir + "/cnvt." + ctriple + ".intensity.svgz";

   int status = US_GuiUtil::save_plot( filename, data_plot );
   if ( status != 0 )
      qDebug() << filename << "plot not saved";

}

void US_IntensityRa::draw_plot( const QVector< double >& scanData,
                              const QVector< double >& scanNbrs )
{
   // Set up the axes and titles
   QwtText axisTitle = data_plot->axisTitle( QwtPlot::yLeft );

   int     szdata    = scanData.size();
   double  xmin      = qFloor( scanNbrs[ 0 ] );
   double  xmax      = qFloor( scanNbrs[ szdata - 1 ] ) + 1.0;
   data_plot->setAxisAutoScale( QwtPlot::yLeft );
   data_plot->setAxisScale    ( QwtPlot::xBottom, xmin, xmax );
   data_plot->setAxisMaxMinor ( QwtPlot::xBottom, 0 );
   data_plot->setAxisTitle    ( QwtPlot::yLeft, "Intensity" );

   QwtSymbol* sym    = new QwtSymbol;

   sym->setStyle( QwtSymbol::Ellipse );
   sym->setPen  ( QPen( Qt::yellow ) );
   sym->setBrush( Qt::white );
   sym->setSize ( 6 );

   // Get the scan data in the right format
   QVector< double > xvec( szdata );
   QVector< double > yvec( szdata );
   double* xx = xvec.data();
   double* yy = yvec.data();

   for ( int ii = 0; ii < szdata; ii++ )
   {
      xx[ ii ] = scanNbrs[ ii ];
      yy[ ii ] = scanData[ ii ];
   }
qDebug() << "Ints:dr_pl: xx0 xxn" << xx[0] << xx[scanData.size()-1];

   QwtPlotCurve* c1 = us_curve( data_plot, tr( "Intensity" ) );
   c1->setPen    ( QPen( QBrush( Qt::yellow ), 2 ) );
   c1->setSymbol ( sym );
   c1->setSamples( xx, yy, szdata );

   data_plot->replot();
}

