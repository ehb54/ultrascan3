//! \file us_fit_meniscus.cpp
#include <QApplication>

#include "us_fit_meniscus.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_settings.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_investigator.h"
#include "us_math2.h"
#include "us_matrix.h"
#include "us_model.h"
#include "us_noise.h"

#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#define setSymbol(a)       setSymbol(*a)
#endif
#if 1
#define USE_3DPLOT
#endif

//! \brief Main program for US_FitMeniscus. Loads translators and starts
//         the class US_FitMeniscus.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_FitMeniscus w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

US_FitMeniscus::US_FitMeniscus() : US_Widgets()
{
   setWindowTitle( tr( "Fit Meniscus from 2DSA Data" ) );
   setPalette( US_GuiSettings::frameColor() );
   dbg_level    = US_Settings::us_debug();
   have3val     = true;

   // Main layout
   QBoxLayout*  mainLayout   = new QVBoxLayout( this );
   mainLayout->setSpacing         ( 2 );
   mainLayout->setContentsMargins ( 2, 2, 2, 2 );

   // Component layouts
   QHBoxLayout* topLayout    = new QHBoxLayout;
   QHBoxLayout* bottomLayout = new QHBoxLayout;
   QGridLayout* leftLayout   = new QGridLayout;
   QGridLayout* rightLayout  = new QGridLayout;
   QGridLayout* cntrlsLayout = new QGridLayout;
DbgLv(1) << "Main: AA";

   // Lay out the meniscus,rmsd text box
   te_data = new US_Editor( US_Editor::LOAD, false,
         "results/*-fm*.fit*dat;;*.dat;;*.*" );
   connect( te_data, SIGNAL( US_EditorLoadComplete( QString ) ), 
                     SLOT  ( file_loaded(           QString ) ) );
   
   QFontMetrics fm( te_data->e->font() ); 

   te_data->setMinimumHeight( fm.height() * 20 );
   te_data->setMinimumWidth ( fm.width( "123 :  6.34567, 7.34567, 0.00567890  0.012345" ) );
   te_data->e->setToolTip( tr( "Loaded, editable meniscus,rmsd table" ) );

   leftLayout->addWidget( te_data, 0, 0, 20, 1 );

   // Lay out the plot
//   QBoxLayout* plot = new US_Plot( meniscus_plot, 
//         tr( "Meniscus Fit" ),
//         tr( "Radius" ), tr( "2DSA Meniscus RMSD Value" ) );
   QBoxLayout* plot = new US_Plot( meniscus_plot, 
         tr( "Meniscus,Bottom Fit" ),
         tr( "Distance from Base Radii" ),
         tr( "2DSA Meniscus,Bottom RMSD" ) );
   
DbgLv(1) << "Main: BB";
   QwtPlotPicker* pick = new US_PlotPicker( meniscus_plot );
   QwtPlotGrid*   grid = us_grid( meniscus_plot );
   pick->setRubberBand( QwtPicker::VLineRubberBand );
   //connect( pick, SIGNAL( moved    ( const QwtDoublePoint& ) ),
   //         this, SLOT(   new_value( const QwtDoublePoint& ) ) );
   grid->attach( meniscus_plot );
   
   meniscus_plot->setMinimumSize( 400, 400 );
   meniscus_plot->setAxisScale( QwtPlot::xBottom, 5.7, 6.8 );
   meniscus_plot->setToolTip( tr( "Fitted meniscus,rmsd plot" ) );

   rightLayout->addLayout( plot, 0, 1, 20, 1 );

   // Lay out the controls
   QLabel* lb_status    = us_label( tr( "Status:"    ) );
   
   le_status    = us_lineedit( tr( "No data loaded" ), -1, true );
   le_status->setToolTip(
         tr( "Results of the last action performed" ) );

   lb_zfloor     = us_label( tr( "Z Visibility Percent:" ) );
   ct_zfloor     = us_counter( 1, 50.0, 150.0, 1.0 );
   ct_zfloor->setSingleStep( 1 );
   ct_zfloor->setValue( 100.0 );
   connect( ct_zfloor, SIGNAL( valueChanged( double ) ),
            this,      SLOT  ( plot_data()            ) );

   lb_order      = us_label( tr( "Fit Order:" ) );

   sb_order = new QSpinBox();
   sb_order->setRange( 2, 9 );
   sb_order->setValue( 2 );
   sb_order->setPalette( US_GuiSettings::editColor() );
   sb_order->setToolTip( tr( "Order of fitting curve" ) );
   connect( sb_order, SIGNAL( valueChanged( int ) ),
            this,     SLOT( plot_data( int ) ) );

   lb_men_sel   = us_label( tr( "Meniscus selected:" ) );
   lb_men_lor   = us_label( tr( "Low-RMSD Meniscus:" ) );
   lb_bot_lor   = us_label( tr( "Low-RMSD Bottom:" ) );
   lb_men_fit   = us_label( tr( "Fit Meniscus:" ) );
   lb_bot_fit   = us_label( tr( "Fit Bottom:" ) );
   lb_mprads    = us_label( tr( "Mid-Point Radii:" ) );

   le_men_lor   = us_lineedit( "", -1, false );
   le_men_lor->setToolTip(
         tr( "Selected-minimum meniscus radius value" ) );
   le_bot_lor   = us_lineedit( "", -1, false );
   le_bot_lor->setToolTip(
         tr( "Selected-minimum bottom radius value" ) );
   le_men_fit   = us_lineedit( "", -1, false );
   le_men_fit->setToolTip(
         tr( "Fit/Editable meniscus radius value" ) );
   le_bot_fit   = us_lineedit( "", -1, false );
   le_bot_fit->setToolTip(
         tr( "Fit/Editable bottom radius value" ) );
   le_mprads    = us_lineedit( "", -1, false );
   le_mprads ->setToolTip(
         tr( "Meniscus,Bottom current radii (midpoint of ranges)" ) );

   le_men_sel   = us_lineedit( "", -1, false );
   le_men_sel->setToolTip(
         tr( "Selected/Editable meniscus radius value" ) );
   lb_rms_error = us_label( tr( "RMS Error:" ) );
   
   le_rms_error = us_lineedit( "", -1, true );
   le_rms_error->setToolTip(
         tr( "RMS error of curve to meniscus,rmsd points" ) );

   dkdb_cntrls            = new US_Disk_DB_Controls(
         US_Settings::default_data_location() );
   connect( dkdb_cntrls, SIGNAL( changed( bool )        ),
            this,        SLOT(   update_disk_db( bool ) ) );

   pb_update = us_pushbutton( tr( "Update Edit" ) );
   connect( pb_update, SIGNAL( clicked() ), SLOT( edit_update() ) );
   pb_update->setEnabled( false );
   pb_update->setToolTip(
         tr( "Update edit record with meniscus; remove non-chosen models" ) );

   pb_scandb = us_pushbutton( tr( "Scan Database" ) );
   connect( pb_scandb, SIGNAL( clicked() ), SLOT( scan_dbase() ) );
   pb_scandb->setEnabled( dkdb_cntrls->db() );
   pb_scandb->setToolTip(
         tr( "Scan fit-meniscus models in DB; create local table files" ) );

   us_checkbox( tr( "Confirm Each Update Step" ), ck_confirm,  true );
   us_checkbox( tr( "Apply to All Wavelengths" ), ck_applymwl, true );
//   ck_applymwl->setVisible( false );
   ck_confirm ->setToolTip(
         tr( "Pop up confirmation dialogs at each update step" ) );
   ck_applymwl->setToolTip(
         tr( "Apply the meniscus update to all wavelengths of"
             " the current cell/channel" ) );

   pb_plot   = us_pushbutton( tr( "Plot" ) );
   connect( pb_plot, SIGNAL( clicked() ),
            this,    SLOT( plot_data() ) );
   pb_plot->setToolTip(
         tr( "Plot,analyze meniscus,rmsd from current text" ) );

   pb_reset  = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ),
            this,     SLOT( reset() ) );
   pb_reset->setToolTip(
         tr( "Clear text,plot and various other controls" ) );

   QPushButton* pb_help   = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ),
            this,    SLOT( help() ) );
   pb_help->setToolTip(
         tr( "Open a dialog with detailed documentation" ) );

   QPushButton* pb_accept = us_pushbutton( tr( "Close" ) );
   connect( pb_accept, SIGNAL( clicked() ),
            this,      SLOT( close() ) );
   pb_accept->setToolTip(
         tr( "Close this dialog and exit the program" ) );

   // Do detailed layout of the controls
   int row = 0;
   cntrlsLayout->addWidget( lb_status,    row,    0, 1,  1 );
   cntrlsLayout->addWidget( le_status,    row++,  1, 1, 15 );
   cntrlsLayout->addWidget( lb_men_lor,   row,    0, 1,  3 );
   cntrlsLayout->addWidget( le_men_lor,   row,    3, 1,  3 );
   cntrlsLayout->addWidget( lb_men_fit,   row,    6, 1,  3 );
   cntrlsLayout->addWidget( le_men_fit,   row,    9, 1,  2 );
//   cntrlsLayout->addWidget( lb_rms_error, row,    8, 1,  2 );
//   cntrlsLayout->addWidget( le_rms_error, row,   10, 1,  2 );
//   cntrlsLayout->addWidget( lb_order,     row,   12, 1,  2 );
//   cntrlsLayout->addWidget( sb_order,     row++, 14, 1,  2 );
   cntrlsLayout->addWidget( lb_zfloor,    row,   11, 1,  3 );
   cntrlsLayout->addWidget( ct_zfloor,    row++, 14, 1,  2 );
   cntrlsLayout->addWidget( lb_bot_lor,   row,    0, 1,  3 );
   cntrlsLayout->addWidget( le_bot_lor,   row,    3, 1,  3 );
   cntrlsLayout->addWidget( lb_bot_fit,   row,    6, 1,  3 );
   cntrlsLayout->addWidget( le_bot_fit,   row,    9, 1,  2 );
   cntrlsLayout->addWidget( lb_mprads,    row,   11, 1,  3 );
   cntrlsLayout->addWidget( le_mprads,    row++, 14, 1,  3 );
   cntrlsLayout->addLayout( dkdb_cntrls,  row,    0, 1,  6 );
   cntrlsLayout->addWidget( pb_update,    row,    6, 1,  5 );
   cntrlsLayout->addWidget( pb_scandb,    row++, 11, 1,  5 );
   cntrlsLayout->addWidget( ck_confirm,   row,    0, 1,  8 );
   cntrlsLayout->addWidget( ck_applymwl,  row++,  8, 1,  8 );
   cntrlsLayout->addWidget( pb_plot,      row,    0, 1,  4 );
   cntrlsLayout->addWidget( pb_reset,     row,    4, 1,  4 );
   cntrlsLayout->addWidget( pb_help,      row,    8, 1,  4 );
   cntrlsLayout->addWidget( pb_accept,    row,   12, 1,  4 );
   for ( int ii = 0; ii < 16; ii++ )
      cntrlsLayout->setColumnStretch( ii, 1 );
DbgLv(1) << "Main: KK";

   // Define final layout
   topLayout   ->addLayout( leftLayout   );
   topLayout   ->addLayout( rightLayout  );
   topLayout   ->setStretchFactor( leftLayout,  1 );
   topLayout   ->setStretchFactor( rightLayout, 2 );
   bottomLayout->addLayout( cntrlsLayout );

   mainLayout  ->addLayout( topLayout    );
   mainLayout  ->addLayout( bottomLayout );
   mainLayout  ->setStretchFactor( topLayout,    2 );
   mainLayout  ->setStretchFactor( bottomLayout, 2 );
DbgLv(1) << "Main: XX";

   te_data->e->setPlainText( "123 :  6.34567, 7.34567, 0.00567890" );
   te_data->adjustSize();
   adjustSize();

//   te_data->e->setPlainText( "" );
DbgLv(1) << "Main: YY";
   reset();
DbgLv(1) << "Main: ZZ";
}

// Clear the plot, m-r table text, and other elements
void US_FitMeniscus::reset( void )
{
DbgLv(1) << " Res: AA";
   dataPlotClear( meniscus_plot );
DbgLv(1) << " Res: BB";
   meniscus_plot->replot();
DbgLv(1) << " Res: CC";
   
   te_data->e   ->setPlainText( "" );
   le_men_lor   ->setText( "" );
   le_bot_lor   ->setText( "" );
   le_men_fit   ->setText( "" );
   le_bot_fit   ->setText( "" );
DbgLv(1) << " Res: JJ";
   le_mprads    ->setText( "" );
DbgLv(1) << " Res: KK";
   sb_order     ->setValue( 2 );
DbgLv(1) << " Res: LL";
   le_rms_error ->setText( "" );
DbgLv(1) << " Res: MM";
   le_men_sel   ->setText( "" );
DbgLv(1) << " Res: YY";
   v_meni       .clear();
   v_bott       .clear();
   v_rmsd       .clear();
DbgLv(1) << " Res: ZZ";
}

// Plot the data
void US_FitMeniscus::plot_data( int )
{
   plot_data();
}

// Plot the data
void US_FitMeniscus::plot_data( void )
{
   if ( v_meni.count() < 1  &&
        te_data->e->toPlainText().length() > 20 ) 
      load_data();

   if ( have3val )
   {
      plot_3d();
   }
   else
   {
      plot_2d();
   }
}

// Load data from lines loaded to text box
void US_FitMeniscus::load_data()
{
   int count         = 0;
   QString contents  = te_data->e->toPlainText();
   contents.replace( QRegExp( "[^0-9eE\\.\\n\\+\\-]+" ), " " );

   QStringList lines = contents.split( "\n", QString::SkipEmptyParts );
   QStringList parsed;
   v_meni.clear();
   v_bott.clear();
   v_rmsd.clear();

   for ( int ii = 0; ii < lines.size(); ii++ )
   {
      QStringList values = lines[ ii ].split( ' ', QString::SkipEmptyParts );

      int valsize        = values.size();
DbgLv(1) << "LD:  ii" << ii << "valsize" << valsize;

      if ( valsize < 2 )   continue;

      if ( valsize > 3 )
      {
         values.removeFirst();
         valsize--;
      }

      double rmeni  = values[ 0 ].toDouble();
      double rbott  = values[ 1 ].toDouble();
      double rmsdv  = rbott;

      if ( rmeni < 5.7  || rmeni > 7.3 )  continue;

      count++;

      if ( valsize == 3 ) 
      {
         have3val      = true;
         rmsdv         = values[ 2 ].toDouble();
         v_meni << rmeni;
         v_bott << rbott;
         v_rmsd << rmsdv;

         parsed << QString().sprintf( "%3d : ", count ) +
                   QString::number( rmeni, 'f', 5 ) + ", " +
                   QString::number( rbott, 'f', 5 ) + ", " +
                   QString::number( rmsdv, 'f', 8 ); 

      }

      else if ( valsize == 2 )
      {
         v_meni << rmeni;
         v_rmsd << rmsdv;

         parsed << QString().sprintf( "%3d : ", count ) +
                   QString::number( rmeni, 'f', 5 ) + ", " +
                   QString::number( rmsdv, 'f', 8 ); 
      }
   }

   have3val      = ( v_bott.count() > 0 );
   te_data->e->setPlainText( parsed.join( "\n" ) );
}

#ifndef USE_3DPLOT
// Plot the data
void US_FitMeniscus::plot_data( void )
{
   dataPlotClear( meniscus_plot );

   QString contents = te_data->e->toPlainText();
   contents.replace( QRegExp( "[^0-9eE\\.\\n\\+\\-]+" ), " " );

   QStringList lines = contents.split( "\n", QString::SkipEmptyParts );
   QStringList parsed;

   QVector< double > vradi( lines.size() );
   QVector< double > vrmsd( lines.size() );
   QVector< QString > rlines;

   double* radius_values = vradi.data();
   double* rmsd_values   = vrmsd.data();
   
   int     count = 0;

   double  minx = 1e20;
   double  maxx = 0.0;

   double  miny = 1e20;
   double  maxy = 0.0;

   have3val     = false;
   rad1off      = 0.0;
   rad2off      = 0.0;

   // Remove any non-data lines and put values in arrays
   for ( int ii = 0; ii < lines.size(); ii++ )
   {
      QStringList values = lines[ ii ].split( ' ', QString::SkipEmptyParts );
      int valsize        = values.size();
DbgLv(1) << "PL:  ii" << ii << "valsize" << valsize;

      if ( valsize < 2 )   continue;

      if ( valsize > 3 )
      {
         values.removeFirst();
         valsize--;
      }

      double radius = values[ 0 ].toDouble();
      double rbott  = values[ 1 ].toDouble();
      double rmsdv  = rbott;

      if ( radius < 5.7  || radius > 7.3 )  continue;

      if ( valsize == 3 ) 
      {
         have3val      = true;
         rad1off       = ( rad1off == 0.0 ) ? radius : rad1off;
         rad2off       = ( rad2off == 0.0 ) ? rbott  : rad2off;

         double radoff = sqrt( sq( radius - rad1off )  +
                               sq( rbott  - rad2off ) );
         rmsdv         = values[ 2 ].toDouble();
         radius_values[ count ] = radoff;
         rmsd_values  [ count ] = rmsdv;

         rlines << QString::number( radoff, 'f', 6 ) + " " +
                   QString().sprintf( "%03d ", ( ii + 1 ) ) +
                   QString::number( rmsdv,  'f', 8 ) + " " +
                   QString::number( radius, 'f', 5 ) + " " +
                   QString::number( rbott,  'f', 5 );

      }

      else if ( valsize == 2 )
      {
         radius_values[ count ] = radius;
         rmsd_values  [ count ] = rmsdv;
      }

      // Find min and max
      minx = min( minx, radius_values[ count ] );
      maxx = max( maxx, radius_values[ count ] );

      miny = min( miny, rmsd_values[ count ] );
      maxy = max( maxy, rmsd_values[ count ] );

      // Reformat
         //parsed << QString::number( radius_values[ count ], 'e', 6 ) + ", " +
         //          QString::number( rmsd_values  [ count ], 'e', 6 ); 
      if ( ! have3val )
      {
         parsed << QString().sprintf( "%3d : ", ii + 1 ) +
                   QString::number( radius_values[ count ], 'f', 5 ) + ", " +
                   QString::number( rmsd_values  [ count ], 'f', 8 ); 
      }

      count++;
   }

   if ( count < 3 ) return;

   // For meniscus+bottom fit, sort entries into radial distance order
   if ( have3val )
   {
      qSort( rlines );
      for ( int ii = 0; ii < rlines.count(); ii++ )
      {
         QString rline    = rlines[ ii ];
DbgLv(1) << "  ii" << ii << "rline" << rline;
         QString s_radoff = rline.section( " ", 0, 0 );
         QString s_count  = rline.section( " ", 1, 1 );
         QString s_rmsdv  = rline.section( " ", 2, 2 );
         QString s_radius = rline.section( " ", 3, 3 );
         QString s_rbott  = rline.section( " ", 4, 4 );
DbgLv(1) << "    radoff count rmsdv radius rbott"
 << s_radoff << s_count << s_rmsdv << s_radius << s_rbott;

         parsed << s_count + " : " + s_radius + ", " + s_rbott + ", " +
                   s_rmsdv + "  " + s_radoff;

         radius_values[ ii ] = s_radoff.toDouble();
         rmsd_values  [ ii ] = s_rmsdv .toDouble();
      }
   }


   te_data->e->setPlainText( parsed.join( "\n" ) );
   le_mprads ->setText( QString::number( rad1off ) + ", " +
                        QString::number( rad2off ) );

   double overscan = ( maxx - minx ) * 0.10;  // 10% overscan

   meniscus_plot->setAxisScale( QwtPlot::xBottom, 
         minx - overscan, maxx + overscan );
    
   // Adjust y axis to scale all the data
   double dy = fabs( maxy - miny ) / 10.0;

   meniscus_plot->setAxisScale( QwtPlot::yLeft, miny - dy, maxy + dy );

   raw_curve = us_curve( meniscus_plot, tr( "Raw Data" ) ); 
   raw_curve->setPen    ( QPen( Qt::yellow ) );
   raw_curve->setSamples( radius_values, rmsd_values, count );

#if 1
   // Reduce raw x,y to local minima only
   QVector< double >  vmradi;
   QVector< double >  vmrmsd;
   int rcount  = 0;
   for ( int ii = 1; ii < ( count - 1 ); ii++ )
   {
      if ( rmsd_values[ ii ] < rmsd_values[ ii - 1 ]  &&
           rmsd_values[ ii ] < rmsd_values[ ii + 1 ] )
      {
          vmradi << radius_values[ ii ];
          vmrmsd << rmsd_values  [ ii ];
          rcount++;
      }
   }
#endif
#if 0
   // Reduce raw x,y to best meniscus and best bottom
   QVector< double >  vmradi;
   QVector< double >  vmrmsd;
   QVector< double >  allmeni;
   QVector< double >  allbott;
   QVector< double >  ameni;
   QVector< double >  abott;
   QVector< double >  armsd;
   QVector< double >  adist;
   double lbmeni  = 0.0;
   double lbbott  = 0.0;
   double lomrmsd = 1.0e+99;
   double lobrmsd = 1.0e+99;
   int rcount     = 0;
   
   for ( int ii = 0; ii < rlines.count(); ii++ )
   {
      QString rline    = rlines[ ii ];
//DbgLv(1) << "  ii" << ii << "rline" << rline;
      double v_radoff = rline.section( " ", 0, 0 ).toDouble();
      double v_rmsdv  = rline.section( " ", 2, 2 ).toDouble();
      double v_radius = rline.section( " ", 3, 3 ).toDouble();
      double v_rbott  = rline.section( " ", 4, 4 ).toDouble();
      if ( !allmeni.contains( v_radius ) )
         allmeni << v_radius;
      if ( !allbott.contains( v_rbott ) )
         allbott << v_rbott;
      ameni << v_radius;
      abott << v_rbott;
      armsd << v_rmsdv;
      adist << v_radoff;
   }
   for ( int ii = 0; ii < allmeni.count(); ii++ )
   {  // Find the meniscus with lowest rmsd sum
      double v_meni  = allmeni[ ii ];
      double sumrmsd = 0.0;
      for ( int jj = 0; jj < ameni.count(); jj++ )
      {
         if ( ameni[ jj ] == v_meni )
            sumrmsd += armsd[ jj ];
      }
      if ( sumrmsd < lomrmsd )
      {
         lomrmsd = sumrmsd;
         lbmeni  = v_meni;
      }
   }
   for ( int ii = 0; ii < allbott.count(); ii++ )
   {  // Find the bottom with lowest rmsd sum
      double v_bott  = allbott[ ii ];
      double sumrmsd = 0.0;
      for ( int jj = 0; jj < abott.count(); jj++ )
      {
         if ( abott[ jj ] == v_bott )
            sumrmsd += armsd[ jj ];
      }
      if ( sumrmsd < lobrmsd )
      {
         lobrmsd = sumrmsd;
         lbbott  = v_bott;
      }
   }
   for ( int ii = 0; ii < ameni.count(); ii++ )
   {  // Fill vectors from values at low meniscus and low bottom
      if ( ameni[ ii ] == lbmeni  ||
           abott[ ii ] == lbbott )
//      if ( ameni[ ii ] == lbmeni )
//      if ( abott[ ii ] == lbbott )
      {
         vmradi << adist[ ii ];
         vmrmsd << armsd[ ii ];
         rcount++;
      }
   }
#endif
   double* radmin_vals  = vmradi.data();
   double* rrmsd_vals   = vmrmsd.data();

   // Plot ellipse symbols along local minima curve
   QwtPlotCurve* lmcurve = us_curve( meniscus_plot, tr( "Minimum Points" ) ); 
   QwtSymbol* msym = new QwtSymbol;
   msym->setStyle( QwtSymbol::Ellipse );
   msym->setPen  ( QPen( Qt::blue ) );
   msym->setBrush( QBrush( Qt::white ) );
   msym->setSize ( 8 );
   lmcurve->setStyle  ( QwtPlotCurve::NoCurve );
   lmcurve->setSymbol ( msym );
   lmcurve->setSamples( radmin_vals, rrmsd_vals, rcount );

   // Do the fit and get the minimum

   double c[ 10 ];

   int order = sb_order->value();

//   if ( ! US_Matrix::lsfit( c, radius_values, rmsd_values, count, order + 1 ) )
   if ( ! US_Matrix::lsfit( c, radmin_vals, rrmsd_vals, rcount, order + 1 ) )
   {
      QMessageBox::warning( this,
            tr( "Data Problem" ),
            tr( "The data is inadequate for this fit order" ) );
      
      le_men_fit   ->clear();
      le_bot_fit   ->clear();
      le_mprads    ->clear();
      le_rms_error ->clear();
      meniscus_plot->replot();

      return;  
   }

   int fit_count = (int) ( ( maxx - minx + 2 * overscan ) / 0.001 );

   QVector< double > vfitx( fit_count );
   QVector< double > vfity( fit_count );
   double* fit_x = vfitx.data();
   double* fit_y = vfity.data();
   double  x     = minx - overscan;
   double minimum;

   for ( int i = 0; i < fit_count; i++, x += 0.001 )
   {
      fit_x[ i ] = x;
      fit_y[ i ] = c[ 0 ];

      for ( int j = 1; j <= order; j++ ) 
         fit_y[ i ] += c[ j ] * pow( x, j );
   }

   // Calculate Root Mean Square Error
   double rms_err = 0.0;

   for ( int i = 0; i < count; i++ )
   {
      double x = radius_values[ i ];
      double y = rmsd_values  [ i ];

      double y_calc = c[ 0 ];
      
      for ( int j = 1; j <= order; j++ )  
         y_calc += c[ j ] * pow( x, j );
      
      rms_err += sq ( fabs ( y_calc - y ) );
   }

   le_rms_error->setText( QString::number( sqrt( rms_err / count ), 'e', 5 ) );

   // Find the minimum
   if ( order == 2 )
   {
      // Take the derivitive and get the minimum
      // c1 + 2 * c2 * x = 0
      minimum = - c[ 1 ] / ( 2.0 * c[ 2 ] );
   }
   else
   {
      // Find the zero of the derivative
      double dxdy  [ 9 ];
      double d2xdy2[ 8 ];

      // First take the derivitive
      for ( int i = 0; i < order; i++ ) 
         dxdy[ i ] = c[ i + 1 ] * ( i + 1 );

      // And we'll need the 2nd derivitive
      for ( int i = 0; i < order - 1; i++ ) 
         d2xdy2[ i ] = dxdy[ i + 1 ] * ( i + 1 );

      // We'll do a quadratic fit for the initial estimate
      double q[ 3 ];
      US_Matrix::lsfit( q, radius_values, rmsd_values, count, 3 );
      minimum = - q[ 1 ] / ( 2.0 * q[ 2 ] );

      const double epsilon = 1.0e-4;

      int    k = 0;
      double f;
      double f_prime;
      do
      {
        // f is the 1st derivative
        f = dxdy[ 0 ];
        for ( int i = 1; i < order; i++ ) f += dxdy[ i ] * pow( minimum, i );

        // f_prime is the 2nd derivative
        f_prime = d2xdy2[ 0 ];
        for ( int i = 1; i < order - 1; i++ ) 
           f_prime += d2xdy2[ i ] * pow( minimum, i );

        if ( fabs( f ) < epsilon ) break;
        if ( k++ > 10 ) break;

        // Get the next estimate
        minimum -= f / f_prime;

      } while ( true );
   }

   fit_curve = us_curve( meniscus_plot, tr( "Fitted Data" ) ); 
   fit_curve->setPen    ( QPen( Qt::red ) );
   fit_curve->setSamples( fit_x, fit_y, fit_count );
   
   // Plot the minimum

   minimum_curve = us_curve( meniscus_plot, tr( "Minimum Pointer" ) ); 
   minimum_curve->setPen( QPen( QBrush( Qt::cyan ), 3.0 ) );

   double radius_min[ 2 ];
   double rmsd_min  [ 2 ];

   radius_min[ 0 ] = minimum;
   radius_min[ 1 ] = minimum;

   rmsd_min  [ 0 ] = miny - 1.0 * dy;
   rmsd_min  [ 1 ] = miny + 2.0 * dy;

   minimum_curve->setSamples( radius_min, rmsd_min, 2 );

   // Find meniscus and bottom closest to the minimum offset
   if ( have3val )
   {
      double min_meni = minimum;
      double min_bott = 0.0;
      double min_offs = 1.0e+99;
      double min_dist = 0.015;
      int min_ix      = 0;

      for ( int ii = 0; ii < rlines.count(); ii++ )
      {
         QString rline    = rlines[ ii ];
DbgLv(1) << "  ii" << ii << "rline" << rline;
         double radoff    = rline.section( " ", 0, 0 ).toDouble();
         double offdev    = qAbs( radoff - minimum );
         if ( offdev < min_offs )
         {
            min_offs         = offdev;
            min_ix           = ii + 1;
            min_meni         = rline.section( " ", 3, 3 ).toDouble();
            min_bott         = rline.section( " ", 4, 4 ).toDouble();
            min_dist         = rline.section( " ", 0, 0 ).toDouble();
         }
      }
      // Put the minimum in the line edit box also
      le_men_lor->setText( QString::number( min_meni, 'f', 5 ) +
                           "  ( " +
                           QString::number( ( min_ix ) + " )" );
      le_bot_lor->setText( QString::number( min_bott, 'f', 5 ) );
   }
   else
   {  // Old style:  just display meniscus
      le_men_sel->setText( QString::number( minimum, 'f', 5 ) );
   }


   // Add the marker label -- bold, font size default + 1, lines 3 pixels wide
   QPen markerPen( QBrush( Qt::white ), 3.0 );
   markerPen.setWidth( 3 );
   
   QwtPlotMarker* pm  = new QwtPlotMarker();
   QwtText        label( QString::number( minimum, 'f', 5 ) );
   QFont          font( pm->label().font() );
   QwtSymbol*     sym = new QwtSymbol( QwtSymbol::Cross, QBrush( Qt::white ),
                                       markerPen, QSize( 9, 9 ) );

   font.setBold( true );
   font.setPointSize( font.pointSize() + 1 );
   label.setFont( font );

   pm->setValue         ( minimum, miny + 3.0 * dy );
   pm->setSymbol        ( sym ); 
   pm->setLabel         ( label );
   pm->setLabelAlignment( Qt::AlignTop );

   pm->attach( meniscus_plot );

   meniscus_plot->replot();
}
#endif
#ifdef USE_3DPLOT
// Plot the data
void US_FitMeniscus::plot_3d( void )
{
   bool auto_lim = false;
   double min_x  = 1.0e+99;
   double min_y  = 1.0e+99;
   double min_z  = 1.0e+99;
   double max_x  = -1.0e+99;
   double max_y  = -1.0e+99;
   double max_z  = -1.0e+99;
   double b_meni = 0.0;
   double b_bott = 0.0;
   int min_ix    = -1;

   // Get minima,maxima and meniscus,bottom at best rmsd
   for ( int ii = 0; ii < v_meni.size(); ii++ )
   {
      double rmeni  = v_meni[ ii ];
      double rbott  = v_bott[ ii ];
      double rrmsd  = v_rmsd[ ii ];

      // Find min and max
      min_x        = qMin( min_x, rmeni );
      max_x        = qMax( max_x, rmeni );
      min_y        = qMin( min_y, rbott );
      max_y        = qMax( max_y, rbott );
      max_z        = qMax( max_z, rrmsd );

      if ( rrmsd < min_z )
      {  // Save best-rmsd meniscus,bottom
         min_z        = rrmsd;
         b_meni       = rmeni;
         b_bott       = rbott;
         min_ix       = ii + 1;
      }
   }
   double min_r = 1.0 / max_z;
   double max_r = 1.0 / min_z;

   // Report low-rmsd meniscus and bottom values
   le_men_lor->setText( QString::number( b_meni, 'f', 5 ) );
   le_bot_lor->setText( QString::number( b_bott, 'f', 5 ) +
                        "  ( " +
                        QString::number( min_ix ) + " )" );
DbgLv(1) << "pl3d: v_meni size" << v_meni.size() << "min,max x,yz"
 << min_x << max_x << min_y << max_y << min_z << max_z;

   // Compute and report fitted meniscus and bottom

   int nmeni     = v_meni.count( v_meni[ 0 ] );
   int nbott     = v_bott.count( v_bott[ 0 ] );
   int ix_best   = min_ix - 1;
   int ix_men    = ix_best / nbott;
   int ix_bot    = ix_best % nbott;
   QList< int >  ixs;
DbgLv(1) << "pl3d:  nmeni nbott" << nmeni << nbott << "ix_best" << ix_best
 << "ix_men ix_bot" << ix_men << ix_bot;

   // Compute indexes for up to 9 points centered at best index.
   //  Exclude some if center point is on edge(s).
   for ( int jmx = ix_men - 1; jmx < ix_men + 2; jmx++ )
   {
      if ( jmx < 0  ||  jmx >= nmeni )    // Skip out-of-bounds index
         continue;

      for ( int jbx = ix_bot - 1; jbx < ix_bot + 2; jbx++ )
      {
         if ( jbx < 0  ||  jbx >= nbott ) // Skip out-of-bounds index
            continue;

         ixs << ( jmx * nbott + jbx );    // Save full vector index
DbgLv(1) << "pl3d:   jmx jbx ixs" << jmx << jbx << (jmx*nbott+jbx);
      }
   }

   // Now calculate the weight averages of meniscus and bottom
   double f_meni = 0.0;
   double f_bott = 0.0;
   double w_rmsd = 0.0;
   for ( int ii = 0; ii < ixs.count(); ii++ )
   {
      int jx        = ixs[ ii ];
      double f_rmsd = 1.0 / v_rmsd[ jx ] - min_r;
      f_meni       += ( v_meni[ jx ] * f_rmsd );
      f_bott       += ( v_bott[ jx ] * f_rmsd );
      w_rmsd       += f_rmsd;
DbgLv(1) << "pl3d:  ixs" << jx << "f_meni f_bott w_rmsd"
 << f_meni << f_bott << w_rmsd;
   }
   f_meni       /= w_rmsd;
   f_bott       /= w_rmsd;
DbgLv(1) << "pl3d:  f_meni f_bott" << f_meni << f_bott;

   le_men_fit->setText( QString::number( f_meni, 'f', 5 ) );
   le_bot_fit->setText( QString::number( f_bott, 'f', 5 ) );

   // Start building 3-D plot
   double* a_meni = v_meni.data();
   double* a_bott = v_bott.data();
   double* a_rmsd = v_rmsd.data();

   QColor cblack( Qt::black );
   QColor cwhite( Qt::white );
DbgLv(1) << "pl3d: clear meniscus_plot";
   dataPlotClear( meniscus_plot );
DbgLv(1) << "pl3d: meniscus_plot replot";
   meniscus_plot->replot();
DbgLv(1) << "pl3d: set pick:";
   pick           = new US_PlotPicker( meniscus_plot );
DbgLv(1) << "pl3d: colormap bg";
   colormap       = new QwtLinearColorMap( Qt::white, Qt::black );
#if 0
   colormap->addColorStop( 0.10, Qt::cyan );
   colormap->addColorStop( 0.50, Qt::magenta );
   colormap->addColorStop( 0.80, Qt::red );
#endif
#if 1
   colormap->addColorStop( 0.15, Qt::cyan );
   colormap->addColorStop( 0.33, Qt::green );
   colormap->addColorStop( 0.50, Qt::blue );
   colormap->addColorStop( 0.67, Qt::magenta );
   colormap->addColorStop( 0.85, Qt::red );
#endif

   QColor bg     = colormap->color1();

DbgLv(1) << "pl3d: set background";
   meniscus_plot->setCanvasBackground( bg );

   int csum      = bg.red() + bg.green() + bg.blue();
   pick->setTrackerPen( QPen( ( csum > 600 ) ? cblack : cwhite ) );
DbgLv(1) << "pl3d: bg" << bg << "csum" << csum;
DbgLv(1) << "pl3d:   pickpen" << pick->trackerPen().brush().color();
DbgLv(1) << "pl3d:    cblack" << cblack << "cwhite" << cwhite;

   QList< S_Solute > sdistro;
   for ( int ii = 0; ii < v_meni.count(); ii++ )
   {
      S_Solute sentry;
      sentry.s      = a_meni[ ii ];
      sentry.k      = a_bott[ ii ];
      sentry.c      = 1.0 / a_rmsd[ ii ] - min_r;
      sdistro << sentry;
   }

   // Set up spectrogram data
   min_z        = min_r;
   max_z        = max_r;
   d_spectrogram = new QwtPlotSpectrogram();
   spec_dat      = new US_SpectrogramData();
   d_spectrogram->setColorMap( ColorMapCopy( colormap ) );
   d_spectrogram->setData    ( spec_dat );
   d_spectrogram->attach( meniscus_plot );

#if 0
   QwtDoubleRect drect;

   if ( auto_lim )
   {
      drect = QwtDoubleRect( 0.0, 0.0, 0.0, 0.0 );
   }

   else
   {
      drect = QwtDoubleRect( min_x, min_y,
            ( max_x - min_x ), ( max_y - min_y ) );
   }
#endif
#if 1
   QwtDoubleRect drect = QwtDoubleRect( 0.0, 0.0, 0.0, 0.0 );
#endif

   double xreso  = 300.0;
   double yreso  = 300.0;
   double resolu =  30.0;
   double zfloor = ct_zfloor->value();
   spec_dat->setRastRanges( xreso, yreso, resolu, zfloor, drect );
   spec_dat->setZRange( min_z, max_z );
   spec_dat->setRaster( &sdistro );

   // Set color map and axis settings
   QwtScaleWidget *rightAxis = meniscus_plot->axisWidget( QwtPlot::yRight );
   rightAxis    ->setColorBarEnabled( true );
   rightAxis    ->setColorMap( spec_dat->range(),
                               ColorMapCopy( colormap ) );
   d_spectrogram->setColorMap( ColorMapCopy( colormap ) );
   meniscus_plot->setAxisTitle( QwtPlot::xBottom, tr( "Meniscus Radius" ) );
   meniscus_plot->setAxisTitle( QwtPlot::yLeft,   tr( "Bottom Radius" ) );
   meniscus_plot->setAxisTitle( QwtPlot::yRight,  tr( "Offset Reciprocal RMSD" ) );
   meniscus_plot->axisTitle( QwtPlot::yRight ).setFont(
         meniscus_plot->axisTitle( QwtPlot::yLeft ).font() );
   meniscus_plot->setAxisScale( QwtPlot::yRight,
      spec_dat->range().minValue(), spec_dat->range().maxValue() );
   meniscus_plot->enableAxis( QwtPlot::yRight );

   if ( auto_lim )
   {   // auto limits
      meniscus_plot->setAxisAutoScale( QwtPlot::yLeft   );
      meniscus_plot->setAxisAutoScale( QwtPlot::xBottom );
   }
   else
   {   // manual limits
      double dx       = qAbs( max_x - min_x ) * 0.05;
      double dy       = qAbs( max_y - min_y ) * 0.05;
      meniscus_plot->setAxisScale( QwtPlot::xBottom, min_x - dx, max_x + dx );
      meniscus_plot->setAxisScale( QwtPlot::yLeft,   min_y - dy, max_y + dy );
   }

   // Report mid-point meniscus,bottom
   double mid_meni = ( min_x + max_x ) * 0.5;
   double mid_bott = ( min_y + max_y ) * 0.5;
   le_mprads ->setText( QString::number( mid_meni ) + ", " +
                        QString::number( mid_bott ) );

   // Draw cross-hairs over best-rmsd point
   double mx_meni[ 2 ];
   double my_meni[ 2 ];
   double mx_bott[ 2 ];
   double my_bott[ 2 ];
   mx_meni[ 0 ] = b_meni;
   mx_meni[ 1 ] = b_meni;
   my_meni[ 0 ] = min_y;
   my_meni[ 1 ] = max_y;
   mx_bott[ 0 ] = min_x;
   mx_bott[ 1 ] = max_x;
   my_bott[ 0 ] = b_bott;
   my_bott[ 1 ] = b_bott;
   QwtPlotCurve* mmeni_curve = us_curve( meniscus_plot, tr( "Low-RMSD Meniscus" ) ); 
   mmeni_curve->setPen    ( QPen( Qt::black, 0, Qt::DashLine ) );
   mmeni_curve->setStyle  ( QwtPlotCurve::Lines );
   mmeni_curve->setSamples( mx_meni, my_meni, 2 );
   QwtPlotCurve* mbott_curve = us_curve( meniscus_plot, tr( "Low-RMSD Bottom" ) ); 
   mbott_curve->setPen    ( QPen( Qt::black, 0, Qt::DashLine ) );
   mbott_curve->setStyle  ( QwtPlotCurve::Lines );
   mbott_curve->setSamples( mx_bott, my_bott, 2 );

   // Plot it
   meniscus_plot->replot();
}

void US_FitMeniscus::plot_2d( void )
{
}
#endif

// Update an edit file with a new meniscus radius value
void US_FitMeniscus::edit_update( void )
{
   QString fn = filedir + "/" + fname_edit;
   idEdit     = 0;
   QFile filei( fn );
   QString edtext;
   QStringList edtexts;

   if ( ! filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      return;
   }

   bool confirm  = ck_confirm->isChecked();
   bool all_wvl  = ( nedtfs > 1  &&  ck_applymwl->isChecked() );
   bool rmv_mdls = true;
   bool db_upd   = dkdb_cntrls->db();
   int  mlsx     = 0;
   int  meqx     = 0;
   int  mvsx     = 0;
   int  mvcn     = 0;
   int  mlnn     = 0;
   int  llsx     = 0;
   int  leqx     = 0;
   int  lvsx     = 0;
   int  lvcn     = 0;
   double menv   = 0.0;
   double lefv   = 0.0;
   QString mmsg  = "";

   if ( ! all_wvl )
   {  // Apply to a single triple
      QTextStream ts( &filei );
      while ( !ts.atEnd() )
         edtext += ts.readLine() + "\n";
      filei.close();

      menv     = le_men_fit->text().toDouble();
      mlsx     = edtext.indexOf( "<meniscus radius=" );
      if ( mlsx < 0 )  return;
      meqx     = edtext.indexOf( "=\"", mlsx );
      mvsx     = meqx + 2;
      mvcn     = edtext.indexOf( "\"",  mvsx + 1 ) - mvsx;
      llsx     = edtext.indexOf( "<data_range left=" );
      if ( llsx < 0 )  return;
      leqx     = edtext.indexOf( "=\"", llsx );
      lvsx     = leqx + 2;
      lvcn     = edtext.indexOf( "\"",  lvsx + 1 ) - lvsx;
      lefv     = edtext.mid( lvsx, lvcn ).toDouble();
DbgLv(1) << " eupd:  menv" << menv << "lefv" << lefv;

      if ( menv >= lefv )
      {
         QMessageBox::warning( this, tr( "Meniscus within Data Range" ),
            tr( "The selected Meniscus value, %1 , extends into the data"
                " range whose left-side value is %2 . This Edit update"
                " cannot be performed!" ).arg( menv ).arg( lefv ) );
         return;
      }

//      edtext   = edtext.replace( mvsx, mvcn, le_men_fit->text() );
edtext   = edtext.replace( mvsx, mvcn, "5.87075" );
      mlnn     = edtext.indexOf( ">", mlsx ) - mlsx + 1;

      QFile fileo( fn );

      if ( ! fileo.open( QIODevice::WriteOnly | QIODevice::Text ) )
      {
         return;
      }

      QTextStream tso( &fileo );
      tso << edtext;
      fileo.close();

      mmsg     = tr( "In file directory\n    " ) + filedir + " ,\n" +
                 tr( "file\n    " ) + fname_edit + "\n" +
                 tr( "has been modified with the line:\n    " ) + 
                 edtext.mid( mlsx, mlnn );
   
      // If using DB, update the edit record there

      if ( db_upd )
      {
         update_db_edit( edtext, fn, mmsg );
      }

      if ( confirm )
      {  // Confirm at each update step
         mmsg    += tr( "\n\nDo you want to remove all fit-meniscus models"
                        " (and associated noises) except for the one"
                        " associated with the nearest meniscus value?" );

         int response = QMessageBox::question( this, tr( "Edit File Updated" ),
               mmsg, QMessageBox::Yes, QMessageBox::Cancel );
         rmv_mdls     = ( response == QMessageBox::Yes );
      }
   }  // END: apply to single triple

   else
   {  // Apply to all wavelengths in a cell/channel
      QString dmsg  = "";
      menv     = le_men_fit->text().toDouble();
DbgLv(1) << " eupd: AppWvl: nedtfs" << nedtfs;
      QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

      for ( int jj = 0; jj < nedtfs; jj++ )
      {
         QString fn = filedir + "/" + edtfiles.at( jj );
DbgLv(1) << " eupd:     jj" << jj << "fn" << fn;
         QFile filei( fn );

         if ( ! filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
            continue;

         QTextStream ts( &filei );
         edtext  = "";
         while ( !ts.atEnd() )
            edtext += ts.readLine() + "\n";
         filei.close();

         mlsx     = edtext.indexOf( "<meniscus radius=" );
         if ( mlsx < 0 )  continue;
         meqx     = edtext.indexOf( "=\"", mlsx );
         mvsx     = meqx + 2;
         mvcn     = edtext.indexOf( "\"",  mvsx + 1 ) - mvsx;
         llsx     = edtext.indexOf( "<data_range left=" );
         if ( llsx < 0 )  continue;
         leqx     = edtext.indexOf( "=\"", llsx );
         lvsx     = leqx + 2;
         lvcn     = edtext.indexOf( "\"",  lvsx + 1 ) - lvsx;
         lefv     = edtext.mid( lvsx, lvcn ).toDouble();

         if ( menv >= lefv )
         {
            QMessageBox::warning( this, tr( "Meniscus within Data Range" ),
               tr( "The selected Meniscus value, %1 , extends into the data"
                   " range whose left-side value is %2 . This Edit update"
                   " cannot be performed!" ).arg( menv ).arg( lefv ) );
            continue;
         }

         edtext   = edtext.replace( mvsx, mvcn, le_men_fit->text() );
         mlnn     = edtext.indexOf( ">", mlsx ) - mlsx + 1;

DbgLv(1) << " eupd:      mlsx mlnn" << mlsx << mlnn;
         QFile fileo( fn );

         if ( ! fileo.open( QIODevice::WriteOnly | QIODevice::Text ) )
            continue;

         QTextStream tso( &fileo );
         tso << edtext;
         fileo.close();

         // If using DB, update the edit record there

         if ( db_upd )
         {
            update_db_edit( edtext, fn, dmsg );
         }

      }  // END: wavelengths loop

      QApplication::restoreOverrideCursor();
      QApplication::restoreOverrideCursor();
      mmsg     = tr( "In file directory\n    " ) + filedir + " ,\n" +
                 tr( "file\n    " ) + fname_edit + "\n" +
                 tr( "has been modified with the line:\n    " ) + 
                 edtext.mid( mlsx, mlnn ) +
                 tr( "\n\n%1 other cell/channel files were"
                     " similarly modified." ).arg( nedtfs - 1 ) + dmsg;
   
      if ( confirm )
      {  // Confirm at each update step
         mmsg    += tr( "\n\nDo you want to remove all fit-meniscus models"
                        " (and associated noises) except for the one"
                        " associated with the nearest meniscus value?" );

         int response = QMessageBox::question( this, tr( "Edit File Updated" ),
                           mmsg, QMessageBox::Yes, QMessageBox::Cancel );

         rmv_mdls = ( response == QMessageBox::Yes );
      }

      idEdit        = 0;
   }  // END: apply to all wavelengths

   if ( rmv_mdls )
   {
DbgLv(1) << " call Remove Models";
      remove_models();
   }

   if ( ! confirm )
   {
      mmsg    += tr( "\n\nAll fit-meniscus models (and associated noises),"
                     " except for the one set associated with the nearest"
                     " meniscus value, were removed." );

      QMessageBox::information( this, tr( "Edit File Updated" ), mmsg );
   }
}

// Slot for handling a loaded file:  set the name of loaded,edit files
void US_FitMeniscus::file_loaded( QString fn )
{
DbgLv(1) << "FL: IN:  fn" << fn;
   filedir    = fn.section( "/",  0, -2 );
   fname_load = fn.section( "/", -1, -1 );
   v_meni.clear();
   v_bott.clear();
   v_rmsd.clear();

   QString edittrip = fname_load.section( ".", -3, -3 );
   QString editID   = edittrip.section( "-",  0, -2 ).mid( 1 );
   QString tripnode = edittrip.section( "-", -1, -1 );
   QString runID    = filedir.section( "/", -1, -1 );
   QString celchn   = tripnode.left( 1 ) + "." +
                      tripnode.mid( 1, 1 );
   QString tripl    = tripnode.left( 1 ) + "." +
                      tripnode.mid( 1, 1 ) + "." +
                      tripnode.mid( 2 );
   QStringList edtfilt;
   edtfilt << runID + "." + editID + ".*." + celchn + ".*.xml";

   fname_edit = "";
   edtfiles   = QDir( filedir ).entryList( edtfilt, QDir::Files, QDir::Name );
DbgLv(1) << "EDITFILT" << edtfilt;
   nedtfs     = edtfiles.size();

   if ( nedtfs > 0 )
   {
      for ( int jj = 0; jj < nedtfs; jj++ )
      {
         if ( edtfiles.at( jj ).contains( tripl ) )
         fname_edit = edtfiles.at( jj );
      }
      pb_update->setEnabled( true );
DbgLv(1) << " nedtfs" << nedtfs << "fname_edit" << fname_edit;
DbgLv(1) << "   f0" << edtfiles.at(0);
if(nedtfs>1) DbgLv(1) << "   f1" << edtfiles.at(1);
   }

   else
   {  // Could not find edit file, so inform the user
      pb_update->setEnabled( false );

      QMessageBox::warning( this,
            tr( "Missing Local Edit" ),
            tr( "Update Edit is not possible\n"
                "without a local copy of the Edit file\n"
                "corresponding to the FM models.\n"
                "Use\n     Convert Legacy Data\nand\n     Manage Data\n"
                "to create a local copy of an Edit file for\n     " )
            + fname_load + tr( "\nof run\n     " ) + runID );
   }

   plot_data();

   le_status->setText( tr( "Data loaded:  " ) + runID + "/" + fname_load );
}

// Scan the database for models to use to write local fit table files
void US_FitMeniscus::scan_dbase()
{
   QList< ModelDesc > mDescrs;     // List of model description objects
   US_Passwd pw;                   // DB password
   US_DB2 db( pw.getPasswd() );    // DB control
   QStringList query;              // DB query string list
   QStringList mfnams;             // List of FM model fit file names
   QStringList ufnams;             // List of unique model fit file names
   QStringList uantms;             // List of unique model fit analysis times
   QStringList tmodels;            // List: IDs of models with truncated descrs
   QStringList tedGIDs;            // List: edit GUIDs of models w/ trunc descrs
   QStringList tedIDs;             // List: edit IDs of models w/ trunc descrs

   int         nfmods = 0;         // Number of fit-meniscus models
   int         nfsets = 0;         // Number of fit-meniscus analysis sets
   int         nfrpls = 0;         // Number of fit file replacements
   int         nfadds = 0;         // Number of fit file additions
   int         nfexss = 0;         // Number of fit files left as they existed

   QString invID   = QString::number( US_Settings::us_inv_ID() );
   QRegExp fmIter  = QRegExp( "i\?\?-m*",
         Qt::CaseSensitive, QRegExp::Wildcard );

   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   // Scan the database and find fit-meniscus models

   le_status->setText(
         tr( "Scanning DB fit-meniscus models ..." ) );
   query << "get_model_desc" << invID;
   db.query( query );

   while( db.next() )
   {
      ModelDesc mdescr;
      QString modelID    = db.value( 0 ).toString();
      QString modelGUID  = db.value( 1 ).toString();
      QString descript   = db.value( 2 ).toString();
      QString editGUID   = db.value( 5 ).toString();
      QString editID     = db.value( 6 ).toString();

      if ( descript.length() == 80 )
      {  // Truncated description:  save ID and skip update for now
//DbgLv(1) << "DbSc:     TRUNC: modelID" << modelID;
         tmodels << modelID;
         tedGIDs << editGUID;
         tedIDs  << editID;
         continue;
      }

      double  variance   = db.value( 3 ).toString().toDouble();
      double  meniscus   = db.value( 4 ).toString().toDouble();
      QDateTime lmtime   = db.value( 7 ).toDateTime();
      lmtime.setTimeSpec( Qt::UTC );
      QString ansysID    = descript.section( '.', -2, -2 );
      QString iterID     = ansysID .section( '_', -1, -1 );
DbgLv(1) << "DbSc:   modelID vari meni" << modelID << variance << meniscus
 << "ansysID" << ansysID << "iterID" << iterID;

      if ( ansysID.contains( "2DSA-FM" )  ||  iterID.contains( fmIter ) )
      {  // Model from meniscus fit, so save information
DbgLv(1) << "DbSc:    *FIT* " << descript;

         // Format and save the potential fit table file name
         QString runID      = descript.section( '.',  0, -4 );
         QString tripleID   = descript.section( '.', -3, -3 );
         QString editLabel  = ansysID .section( '_',  0, -5 );
         QString ftfname    = runID + "/2dsa-fm." + editLabel +
                              "-" + tripleID + ".fitmen.dat";
         mdescr.description = descript;
         mdescr.baseDescr   = runID + "." + tripleID + "."
                              + ansysID.section( "-", 0, 3 );
         mdescr.fitfname    = ftfname;
         mdescr.modelID     = modelID;
         mdescr.modelGUID   = modelGUID;
         mdescr.editID      = editID;
         mdescr.editGUID    = editGUID;
         mdescr.variance    = variance;
         mdescr.meniscus    = meniscus;
         mdescr.antime      = descript.section( '.', -2, -2 )
                              .section( '_',  1,  1 ).mid( 1 );
         mdescr.lmtime      = lmtime;

         mDescrs << mdescr;
      }
   }
DbgLv(1) << "DbSc: tmodels size" << tmodels.size() << "ted sizes"
 << tedGIDs.size() << tedIDs.size();

   for ( int ii = 0; ii < tmodels.size(); ii++ )
   {  // Review models with truncated descriptions
      QString modelID    = tmodels[ ii ];
      query.clear();
      query << "get_model_info" << modelID;
      db.query( query );

      if ( db.lastErrno() != US_DB2::OK )  continue;

      db.next();

      QString modelGUID  = db.value( 0 ).toString();
      QString descript1  = db.value( 1 ).toString();
      QString contents   = db.value( 2 ).toString();
      int     jdx        = contents.indexOf( "description=" );
//DbgLv(1) << "DbSc:    ii jdtx" << ii << jdtx << "modelID" << modelID
//   << "  dsc1" << descript1 << " cont" << contents.left( 20 );

      if ( jdx < 1 )  continue;

//DbgLv(1) << "DbSc:      jdx lend" << jdx << lend;
      QString descript   = contents.mid( jdx ).section( '"', 1, 1 );
      double  variance   = db.value( 3 ).toString().toDouble();
      double  meniscus   = db.value( 4 ).toString().toDouble();
      QString editGUID   = tedGIDs[ ii ];
      QString editID     = tedIDs [ ii ];

      QDateTime lmtime   = db.value( 6 ).toDateTime();
      lmtime.setTimeSpec( Qt::UTC );
      QString ansysID    = descript.section( '.', -2, -2 );
      QString iterID     = ansysID .section( '_', -1, -1 );
//DbgLv(1) << "DbSc:   dscr1" << descript1 << "dcs" << descript;

      if ( ansysID.contains( "2DSA-FM" )  ||  iterID.contains( fmIter ) )
      {  // Model from meniscus fit, so save information
DbgLv(1) << "DbSc:    *FIT* " << descript;
         ModelDesc mdescr;

         // Format and save the potential fit table file name
         QString runID      = descript.section( '.',  0, -4 );
         QString tripleID   = descript.section( '.', -3, -3 );
         QString editLabel  = ansysID .section( '_',  0, -5 );
         QString ftfname    = runID + "/2dsa-fm." + editLabel +
                              "-" + tripleID + ".fitmen.dat";
         mdescr.description = descript;
         mdescr.baseDescr   = runID + "." + tripleID + "."
                              + ansysID.section( "-", 0, 3 );
         mdescr.fitfname    = ftfname;
         mdescr.modelID     = modelID;
         mdescr.modelGUID   = modelGUID;
         mdescr.editID      = editID;
         mdescr.editGUID    = editGUID;
         mdescr.variance    = variance;
         mdescr.meniscus    = meniscus;
         mdescr.antime      = descript.section( '.', -2, -2 )
                              .section( '_',  1,  1 ).mid( 1 );
         mdescr.lmtime      = lmtime;

         mDescrs << mdescr;
      }
   }

   nfmods     = mDescrs.size();
DbgLv(1) << "Number of FM models found: " << nfmods;
if(nfmods>0) {
DbgLv(1) << " pre:D0" <<  mDescrs[0].description;
DbgLv(1) << " pre:Dn" <<  mDescrs[nfmods-1].description; }
   qSort( mDescrs );
if(nfmods>0) {
DbgLv(1) << " sorted:D0" <<  mDescrs[0].description;
DbgLv(1) << " sorted:Dn" <<  mDescrs[nfmods-1].description; }

   // Scan local files to see what fit table files already exist

   le_status->setText(
         tr( "Comparing to existing local meniscus,rmsd table files ..." ) );
   mfnams.clear();
   ufnams.clear();
   uantms.clear();

   for ( int ii = 0; ii < nfmods; ii++ )
   {  // Find unique file names in order to create sets
      QString ftfname    = mDescrs[ ii ].fitfname;
      QString antime     = mDescrs[ ii ].antime;

      if ( ! ufnams.contains( ftfname ) )
      {  // This is a new fit-file name, so new analysis set
         ufnams << ftfname;
         uantms << antime;
      }

      else if ( ! uantms.contains( antime ) )
      {  // Already seen fit-file, but new analysis time, so duplicate
         uantms << antime;
      }

      mfnams << mDescrs[ ii ].fitfname;
   }

   nfsets     = ufnams.size();
   int nantm  = uantms.size();
   int ndupl  = nantm - nfsets;
DbgLv(1) << "Number of FM analysis sets: " << nfsets;
DbgLv(1) << "Number of FM analysis set duplicates: " << ndupl;
   int kfsets = nfsets;
   QString rdir = US_Settings::resultDir().replace( "\\", "/" ) + "/";
   QString fnamesv;

   for ( int ii = 0; ii < kfsets; ii++ )
   {  // Find out for each set whether a corresponding fit file exists
      QString ftfname    = ufnams.at( ii );

      if ( mfnams.count( ftfname ) == 1 )
      {  // Not really a set; single fit model after previous fm run
         nfsets--;
         continue;
      }

      QString ftfpath    = rdir + ftfname;
      QFile   ftfile( ftfpath );

      if ( ftfile.exists() )
      {  // File exists, so we must check the need to replace it
         QString ftfpath    = rdir + ftfname;
         QDateTime fdate    = QFileInfo( ftfile ).lastModified().toUTC();
         int       jj       = mfnams.lastIndexOf( ftfname );
         QDateTime rdate    = mDescrs[ jj ].lmtime;
DbgLv(1) << " ii rdate fdate" << ii << rdate << fdate << "   ftfname"
   << ftfname << "  fdate.msecsTo(rdate)" << fdate.msecsTo(rdate);
DbgLv(1) << "   jj desc" << jj << mDescrs[jj].description
 << "antime meniscus" << mDescrs[jj].antime << mDescrs[jj].meniscus;

         if ( fdate.msecsTo( rdate ) > 0 )
         {  // DB record date is later than file date, so must replace file
            nfrpls++;
            ftfile.remove();
         }

         else
         {  // DB record date is not later than file date, so leave file as is
            nfexss++;
            continue;
         }
      }

      else
      {  // File does not exist, so we definitely need to create it
         nfadds++;
         QString ftfpath    = QString( rdir + ftfname ).section( "/", 0, -2 );
         QDir().mkpath( ftfpath );
      }

      if ( ! ftfile.open( QIODevice::WriteOnly | QIODevice::Text ) )
      {  // Problem!!!
         qDebug() << "*ERROR* Unable to open file" << ftfname;
         continue;
      }

      // Creating a new or replacement file:  build list of meniscus,rmsd pairs
      int       jfirst   = mfnams.indexOf( ftfname );
      int       jlast    = mfnams.lastIndexOf( ftfname ) + 1;
      QString   antiml   = mDescrs[ jlast - 1 ].antime;
      QStringList mrpairs;

DbgLv(1) << " Creating" << ftfname << "jf,jl" << jfirst << jlast;
      for ( int jj = jfirst; jj < jlast; jj++ )
      {  // First build the pairs list
         double meniscus = mDescrs[ jj ].meniscus;
         double variance = mDescrs[ jj ].variance;
         double rmsd     = sqrt( variance );
         QString antime  = mDescrs[ jj ].antime;
DbgLv(1) << "  jj desc" << jj << mDescrs[jj].description;
         if ( antime == antiml )
            mrpairs << QString::number( meniscus, 'f', 6 ) + " "
                     + QString::number( rmsd,     'e', 6 ); 
      }

      mrpairs.sort();
      QTextStream ts( &ftfile );

      // Output the pairs to the file
      for ( int jj = 0; jj < mrpairs.size(); jj++ )
         ts << mrpairs.at( jj ) + "\n";

      ftfile.close();

      fnamesv = fnamesv.isEmpty() ? ftfname : fnamesv;
   }

DbgLv(1) << "Number of FM REPLACE  sets: " << nfrpls;
DbgLv(1) << "Number of FM ADD      sets: " << nfadds;
DbgLv(1) << "Number of FM EXISTING sets: " << nfexss;

   // Report
   QString msg = tr( "File" );
   int nftota  = nfadds + nfrpls;

   if      ( nfadds == 1  &&  nfrpls == 0 )
      msg += tr( " added: " );

   else if ( nfadds == 0  &&  nfrpls == 1 )
      msg += tr( " updated: " );

   else if ( nfadds == 0  &&  nfrpls == 0 )
      msg  = tr( "No new fit files were created." );

   else
      msg  = tr( "Last of %1 added/updated: " ).arg( nftota );

   if ( nftota > 0 )
      msg += fnamesv;

   le_status->setText( msg );
   QApplication::restoreOverrideCursor();
   QApplication::restoreOverrideCursor();
}

// Reset state of database scan button based on DB/Disk choice
void US_FitMeniscus::update_disk_db( bool isDB )
{
   pb_scandb->setEnabled( isDB );
}

// Update the DB edit record with a new meniscus value
void US_FitMeniscus::update_db_edit( QString edtext, QString efilepath,
      QString& msg )
{
   int     elnx     = edtext.indexOf( "<editGUID " );
   int     esvx     = edtext.indexOf( "\"", elnx ) + 1;
   int     nvch     = edtext.indexOf( "\"", esvx ) - esvx;
   QString edGUID   = edtext.mid( esvx, nvch );
DbgLv(1) << "updDbEd: edGUID" << edGUID;

   US_Passwd pw;
   US_DB2 db( pw.getPasswd() );
   QStringList query;
   query << "get_editID" << edGUID;
   db.query( query );
   db.next();
   idEdit           = db.value( 0 ).toString().toInt();
DbgLv(1) << "updDbEd: idEdit" << idEdit;
   db.writeBlobToDB( efilepath, "upload_editData", idEdit );

   if ( nedtfs == 1  ||  ! ck_applymwl->isChecked() )
   {
      msg += tr( "\n\nThe meniscus value was also updated for the"
                 " corresponding edit record in the database." );
   }

   else
   {
      QString fn       = efilepath.section( "/", -1, -1 );
      int lstfx        = nedtfs - 1;
      if ( fn == edtfiles[ lstfx ] )
      {
         msg += tr( "\n\nThe meniscus value was also updated for the"
                    " corresponding edit records in the database." );
      }
   }

   return;
}

// Remove f-m models (and associated noise) except for the single chosen one
void US_FitMeniscus::remove_models()
{
   QString srchRun  = filedir.section( "/", -1, -1 );
   QString srchEdit = fname_load.section( ".", -3, -3 );
   QString srchTrip = srchEdit.section( "-", -1, -1 );
           srchEdit = srchEdit.section( "-",  0, -2 );
DbgLv(1) << "RmvMod: scn1 srchRun srchEdit srchTrip"
 << srchRun << srchEdit << srchTrip;

   // Scan models files; get list of fit-meniscus type matching run/edit/triple
   QStringList modfilt;
   modfilt << "M*.xml";
   QString     moddir   = US_Settings::dataDir() + "/models";
   QStringList modfiles = QDir( moddir ).entryList(
         modfilt, QDir::Files, QDir::Name );
   moddir               = moddir + "/";
   double cmeniscus     = le_men_fit->text().toDouble();
//   double cbottom       = le_bot_fit->text().toDouble();

   QList< ModelDesc >  lMDescrs;
   QList< ModelDesc >  dMDescrs;

   QStringList     lmodFnams;             // local model full path file names
   QStringList     lmodGUIDs;             // Local model GUIDs
   QList< double > lmodVaris;             // Local variance values
   QList< double > lmodMenis;             // Local meniscus values
   QStringList     lmodDescs;             // Local descriptions

   QStringList     dmodIDs;               // Database model IDs
   QStringList     dmodGUIDs;             // Database model GUIDs
   QList< double > dmodVaris;             // Database variance values
   QList< double > dmodMenis;             // Database meniscus values
   QStringList     dmodDescs;             // Database descriptions
   int nlmods           = 0;
   int ndmods           = 0;
   int nlnois           = 0;
   int ndnois           = 0;
   int lArTime          = 0;
   int dArTime          = 0;
   int lkModx           = -1;
   int dkModx           = -1;
   bool db_upd          = dkdb_cntrls->db();

   for ( int ii = 0; ii < modfiles.size(); ii++ )
   {
      ModelDesc lmodd;
      QString modfname   = modfiles.at( ii );
      QString modpath    = moddir + modfname;
      US_Model model;
      
      if ( model.load( modpath ) != US_DB2::OK )
         continue;    // Can't use if can't load

      QString descript   = model.description;
      QString runID      = descript.section( '.',  0, -4 );
      QString tripID     = descript.section( '.', -3, -3 );
      QString anRunID    = descript.section( '.', -2, -2 );
      QString editLabl   = anRunID .section( '_',  0, -5 );
//DbgLv(1) << "RmvMod:  scn1 ii runID editLabl tripID"
// << ii << runID << editLabl << tripID;

      if ( runID != srchRun  ||  editLabl != srchEdit  ||  tripID != srchTrip )
         continue;    // Can't use if from a different runID or edit or triple

      QString iterID     = anRunID .section( '_', -1, -1 );
//DbgLv(1) << "RmvMod:    iterID" << iterID;

      if ( iterID.length() != 10  ||  ! iterID.contains( "-m" ) )
         continue;    // Can't use if not a fit-meniscus type

      // Probably a file from the right set, but let's check for other sets
      int     arTime     = anRunID .section( '_', -4, -4 ).mid( 1 ).toInt();
DbgLv(1) << "RmvMod:    arTime lArTime" << arTime << lArTime;

      if ( arTime > lArTime )
      {  // If first set or new one younger than previous, start lists
         lmodFnams.clear();
         lmodGUIDs.clear();
         lmodVaris.clear();
         lmodMenis.clear();
         lmodDescs.clear();
         lMDescrs .clear();
         lArTime            = arTime;
      }

      else if ( arTime < lArTime )
      {  // If new one older than previous, skip it
         continue;
      }

      lmodFnams << modpath;             // Save full path file name
      lmodGUIDs << model.modelGUID;     // Save model GUID
      lmodVaris << model.variance;      // Save variance
      lmodMenis << model.meniscus;      // Save meniscus
      lmodDescs << model.description;   // Save description

      lmodd.description = model.description;
      lmodd.modelGUID   = model.modelGUID;
      lmodd.modelID     = "-1";
      lmodd.variance    = model.variance;
      lmodd.meniscus    = model.meniscus;
      lMDescrs << lmodd;
   }

   nlmods         = lMDescrs.size();
   qSort( lMDescrs );
DbgLv(1) << "RmvMod: nlmods" << nlmods;
   double minMdif = 99e+10;

   for ( int ii = 0; ii < nlmods; ii++ )
   {  // Scan to identify model in set with lowest variance
      ModelDesc lmodd = lMDescrs[ ii ];
      double diffMen  = qAbs( lmodd.meniscus - cmeniscus );
DbgLv(1) << "low Mdif scan: ii vari meni desc" << ii << lmodd.variance
 << lmodd.meniscus << lmodd.description.right( 22 );

      if ( diffMen < minMdif )
      {
         minMdif        = diffMen;
         lkModx         = ii;
//DbgLv(1) << "low Mdif scan:   minMdif lkModx" << minMdif << lkModx;
      }
   }
DbgLv(1) << "RmvMod:  minMdif lkModx" << minMdif << lkModx;
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   // Make a list of f-m models that match for DB, if possible
   if ( db_upd )
   {
      ModelDesc dmodd;
      QString   invID = QString::number( US_Settings::us_inv_ID() );
      QString   edtID = QString::number( idEdit );
      US_Passwd pw;
      US_DB2 db( pw.getPasswd() );
      QStringList query;
      QStringList modIDs;

      if ( idEdit > 0 )
         query << "get_model_desc_by_editID" << invID << edtID;

      else
         query << "get_model_desc" << invID;

DbgLv(1) << "RmvMod:  idEdit" << idEdit << "query" << query;
      db.query( query );

      while( db.next() )
      {
         QString modelID    = db.value( 0 ).toString();
         QString descript   = db.value( 2 ).toString();
         QString runID      = descript.section( '.',  0, 0 );
         if ( runID == srchRun )
         {
            modIDs << modelID;
         }
      }

      for ( int ii = 0; ii < modIDs.size(); ii++ )
      {
         QString modelID    = modIDs.at( ii );
         query.clear();
         query << "get_model_info" << modelID;
         db.query( query );

         if ( db.lastErrno() != US_DB2::OK )  continue;

         db.next();

         QString modelGUID  = db.value( 0 ).toString();
         QString descript1  = db.value( 1 ).toString();
         QString contents   = db.value( 2 ).toString();
         int     jdtx       = contents.indexOf( "description=" );

         if ( jdtx < 1 )  continue;

         int     jdx        = contents.indexOf( "\"", jdtx ) + 1;
         int     lend       = contents.indexOf( "\"", jdx  ) - jdx;
         QString descript   = contents.mid( jdx, lend );
         double  variance   = db.value( 3 ).toString().toDouble();
         double  meniscus   = db.value( 4 ).toString().toDouble();
         QString runID      = descript.section( '.',  0, -4 );
         QString tripID     = descript.section( '.', -3, -3 );
         QString anRunID    = descript.section( '.', -2, -2 );
         QString editLabl   = anRunID .section( '_',  0, -5 );
//DbgLv(1) << "RmvMod:  scn1 ii runID editLabl tripID"
// << ii << runID << editLabl << tripID;

         if ( runID != srchRun  ||  editLabl != srchEdit
          ||  tripID != srchTrip )
         continue;    // Can't use if from a different runID or edit or triple

         QString iterID     = anRunID .section( '_', -1, -1 );
//DbgLv(1) << "RmvMod:    iterID" << iterID;

         if ( iterID.length() != 10  ||  ! iterID.contains( "-m" ) )
            continue;    // Can't use if not a fit-meniscus type

         // Probably a file from the right set, but let's check for other sets
         int     arTime     = anRunID .section( '_', -4, -4 ).mid( 1 ).toInt();

         if ( arTime > dArTime )
         {  // If first set or new one younger than previous, start lists
            dmodIDs  .clear();
            dmodGUIDs.clear();
            dmodVaris.clear();
            dmodMenis.clear();
            dmodDescs.clear();
            dMDescrs .clear();
            dArTime            = arTime;
         }

         else if ( arTime < dArTime )
         {  // If new one older than previous, skip it
            continue;
         }

         dmodIDs   << modelID;             // Save model DB ID
         dmodGUIDs << modelGUID;           // Save model GUID
         dmodVaris << variance;            // Save variance
         dmodMenis << meniscus;            // Save meniscus
         dmodDescs << descript;            // Save description

         dmodd.description = descript;
         dmodd.modelGUID   = modelGUID;
         dmodd.modelID     = modelID;
         dmodd.variance    = variance;
         dmodd.meniscus    = meniscus;
         dMDescrs << dmodd;
DbgLv(1) << "RmvMod:  scn2 ii dmodDesc" << descript; 
      }

      ndmods         = dMDescrs.size();
      qSort( dMDescrs );

      if ( dArTime > lArTime )      // Don't count any older group
         nlmods         = 0;
      else if ( lArTime > dArTime )
         ndmods         = 0;

DbgLv(1) << "RmvMod: ndmods" << ndmods;
      double minMdif = 99e+10;

      for ( int ii = 0; ii < ndmods; ii++ )
      {  // Scan to identify model in set with lowest variance
         ModelDesc dmodd = dMDescrs[ ii ];
         double diffMen  = qAbs( dmodd.meniscus - cmeniscus );
DbgLv(1) << "low Mdif scan: ii vari meni desc" << ii << dmodd.variance
 << dmodd.meniscus << dmodd.description.right( 22 );

         if ( diffMen < minMdif )
         {
            minMdif        = diffMen;
            dkModx         = ii;
DbgLv(1) << "low Mdif scan:   minMdif dkModx" << minMdif << dkModx;
         }
      }

      // Now, compare the findings for local versus database
      if ( nlmods == ndmods  ||  ( ndmods > 0 && nlmods == 0 ) )
      {
         int    nmatch  = 0;

         for ( int jj = 0; jj < nlmods; jj++ )
         {
            ModelDesc lmodd = lMDescrs[ jj ];
            ModelDesc dmodd = dMDescrs[ jj ];

            if ( lmodd.modelGUID   == dmodd.modelGUID &&
                 lmodd.description == dmodd.description )
               nmatch++;

            lmodGUIDs[ jj ]    = lmodd.modelGUID;
            lmodVaris[ jj ]    = lmodd.variance;
            lmodMenis[ jj ]    = lmodd.meniscus; 
            lmodDescs[ jj ]    = lmodd.description;
         }

         for ( int jj = 0; jj < ndmods; jj++ )
         {
            ModelDesc dmodd = dMDescrs[ jj ];
            dmodIDs  [ jj ]    = dmodd.modelID;
            dmodGUIDs[ jj ]    = dmodd.modelGUID;
            dmodVaris[ jj ]    = dmodd.variance;
            dmodMenis[ jj ]    = dmodd.meniscus; 
            dmodDescs[ jj ]    = dmodd.description;
         }

         if ( nmatch == nlmods )
         {  // OK if they match or local only
DbgLv(1) << "++local/dbase match, or local only";
         }

         else
         {  // Not good if they do not match
DbgLv(1) << "**local/dbase DO NOT MATCH";
DbgLv(1) << "  nmatch ndmods nlmods" << nmatch << ndmods << nlmods;
            return;
         }
      }

      else if ( nlmods == 0 )
      {  // It is OK if there are no local records, when DB ones were found
DbgLv(1) << "++only dbase records exist";
      }

      else if ( ndmods == 0 )
      {  // It is OK if there are only local records, when local ones found
DbgLv(1) << "++only local records exist";
      }

      else
      {  // Non-zero local & DB, but they do not match
DbgLv(1) << "**local/dbase DO NOT MATCH in count";
DbgLv(1) << "  nlmods ndmods" << nlmods << ndmods;
         return;
      }
   }

DbgLv(1) << "  nlmods ndmods" << nlmods << ndmods;
   if ( ndmods > 0  ||  nlmods > 0 )
   {  // There are models to scan, so build a list of models,noises to remove
      ModelDesc       rmodDescrs;
      NoiseDesc       rnoiDescrs;
      QStringList     rmodIDs;
      QStringList     rmodDescs;
      QStringList     rmodFnams;
      QStringList     rnoiIDs;
      QStringList     rnoiFnams;
      QStringList     rnoiDescs;
      QStringList     nieDescs;
      QStringList     nieIDs;
      QStringList     nieFnams;
      int             nlrmod = 0;
      int             ndrmod = 0;
      int             nlrnoi = 0;
      int             ndrnoi = 0;
      int             ntmods = ( ndmods > 0 ) ? ndmods : nlmods;
      int             ikModx = ( ndmods > 0 ) ? dkModx : lkModx;
DbgLv(1) << "  ntmods ikModx" << ntmods << ikModx;

      QString modDesc    = "";

      for ( int jj = 0; jj < ntmods; jj++ )
      {  // Build the list of model files and DB ids for removal
         if ( jj != ikModx )
         {
            int itix;
            int irix;
            QString fname;
            QString mDesc;
            QString mID;
            QString tiDesc;
            QString riDesc;
            QString noiID; 
            QString noiFname;

            if ( nlmods > 0 )
            {
               fname  = lmodFnams[ jj ];
               mDesc  = lmodDescs[ jj ];
               nlrmod++;
               if ( ndmods == 0 )
                  mID    = "-1";
            }

            if ( ndmods > 0 )
            {
               mID    = dmodIDs  [ jj ];
               mDesc  = dmodDescs[ jj ];
               ndrmod++;
               if ( nlmods == 0 )
                  fname  = "";
            }
            rmodIDs   << mID;
            rmodFnams << fname;
            rmodDescs << mDesc;

            if ( modDesc.isEmpty() )
            {
               modDesc = mDesc;   // Save 1st model's description
DbgLv(1) << "RmvMod: 1st rmv-mod: jj modDesc" << jj << modDesc;
               // Build noises-in-edit lists for database and local
               noises_in_edit( modDesc, nieDescs, nieIDs, nieFnams );
            }

            tiDesc = QString( mDesc ).replace( ".model", ".ti_noise" );
            riDesc = QString( mDesc ).replace( ".model", ".ri_noise" );
            itix   = nieDescs.indexOf( tiDesc );
            irix   = nieDescs.indexOf( riDesc );

            if ( itix >= 0 )
            {  // There is a TI noise to remove
               noiID    = nieIDs  [ itix ];
               noiFname = nieFnams[ itix ];

               if ( noiID != "-1" )
                  ndrnoi++;

               if ( ! noiFname.isEmpty() )
                  nlrnoi++;
               else
                  noiFname = "";

               rnoiIDs   << noiID;
               rnoiFnams << noiFname;
               rnoiDescs << tiDesc;
            }

            if ( irix >= 0 )
            {  // There is an RI noise to remove
               noiID    = nieIDs  [ irix ];
               noiFname = nieFnams[ irix ];

               if ( noiID != "-1" )
                  ndrnoi++;

               if ( ! noiFname.isEmpty() )
                  nlrnoi++;
               else
                  noiFname = "";

               rnoiIDs   << noiID;
               rnoiFnams << noiFname;
               rnoiDescs << riDesc;
            }
         }
      }
      QApplication::restoreOverrideCursor();
      QApplication::restoreOverrideCursor();

      nlnois             = nlrnoi + ( nlrnoi > nlrmod ? 2 : 1 );
      ndnois             = ndrnoi + ( ndrnoi > ndrmod ? 2 : 1 );
      bool rmv_mdls      = true;
DbgLv(1) << "RmvMod: nlrmod ndrmod nlrnoi ndrnoi nlnois ndnois"
 << nlrmod << ndrmod << nlrnoi << ndrnoi << nlnois << ndnois;
      if ( ck_confirm->isChecked() )
      {
         QString msg = tr( "%1 local model files;\n"
                           "%2 database model files;\n"
                           "%3 local noise files;\n"
                           "%4 database noise files;\n"
                           "have been identified for removal.\n\n"
                           "Do you really want to delete them?" )
            .arg( nlrmod ).arg( ndrmod ).arg( nlrnoi ).arg( ndrnoi );

         int response = QMessageBox::question( this,
               tr( "Remove Models and Noises?" ),
               msg, QMessageBox::Yes, QMessageBox::Cancel );
         rmv_mdls           = ( response == QMessageBox::Yes );
      }

      if ( rmv_mdls )
      {
         US_Passwd pw;
         US_DB2* dbP = db_upd ? new US_DB2( pw.getPasswd() ) : NULL;

         QStringList query;
         QString recID;
         QString recFname;
         QString recDesc;
DbgLv(1) << " Remove Models and Noises";
         for ( int ii = 0; ii < rmodIDs.size(); ii++ )
         {  // Remove models and db noises
            recID    = rmodIDs  [ ii ];
            recDesc  = rmodDescs[ ii ];
            recFname = rmodFnams[ ii ];
DbgLv(1) << "  Delete: " << recID << recFname.section("/",-1,-1) << recDesc;

            if ( ! recFname.isEmpty() )
            {  // Delete local file model
               QFile recf( recFname );
               if ( recf.exists() )
               {
                  if ( recf.remove() )
{ DbgLv(1) << "     local file removed"; }
                  else { qDebug() << "*ERROR* removing" << recFname; }
               }
               else { qDebug() << "*ERROR* does not exist:" << recFname; }
            }

            if ( recID != "-1"  &&  dbP != NULL )
            {  // Delete model (and any child noise) from DB
               query.clear();
               query << "delete_model" << recID;
               int stat = dbP->statusQuery( query );
               if ( stat != 0 )
                  qDebug() << "delete_model error" << stat;
else DbgLv(1) << "     DB record deleted";
            }
         }

         if ( dbP != NULL )
         {
            delete dbP;
            dbP   = NULL;
         }

         for ( int ii = 0; ii < rnoiIDs.size(); ii++ )
         {  // Remove local noises
            recID    = rnoiIDs  [ ii ];
            recDesc  = rnoiDescs[ ii ];
            recFname = rnoiFnams[ ii ];
DbgLv(1) << "  Delete: " << recID << recFname.section("/",-1,-1) << recDesc;

            if ( ! recFname.isEmpty() )
            {  // Delete local file noise
               QFile recf( recFname );
               if ( recf.exists() )
               {
                  if ( recf.remove() )
{ DbgLv(1) << "     local file removed"; }
                  else { qDebug() << "*ERROR* removing" << recFname; }
               }
               else { qDebug() << "*ERROR* does not exist:" << recFname; }
            }

            // No need to remove noises from DB; model remove did that
         }
      }
   }

   else
   {  // No models were found!!! (huh!!!)
DbgLv(1) << "**NO local/dbase models-to-remove were found!!!!";
   }

   QApplication::restoreOverrideCursor();
   return;
}

// Create lists of information for noises that match a sample model from a set
void US_FitMeniscus::noises_in_edit( QString modDesc, QStringList& nieDescs,
      QStringList& nieIDs, QStringList& nieFnams )
{
   QString msetBase = modDesc.section( ".", 0, -3 ) + "." + 
                      modDesc.section( ".", -2, -2 ).section( "_", 0, 3 );
   QString srchTlab = msetBase.section( ".", -2, -2 );
   QString srchTrip = srchTlab.left( 1 ) + "." + srchTlab.mid( 1, 1 ) + "." +
                      srchTlab.mid( 2 ) + ".xml";
   QString srchRun  = msetBase.section( ".", 0, -3 ) + "." +
                      msetBase.section( ".", -1, -1 )
                      .section( "_", 0, 0 ).mid( 1 );
DbgLv(1) << "NIE: msetBase" << msetBase;
   QStringList query;
   QString     fname;
   QString     noiID;
   int         nlnois = 0;
   bool        usesDB = dkdb_cntrls->db();

   QStringList noifilt;
   noifilt << "N*.xml";
   QString     noidir   = US_Settings::dataDir() + "/noises";
   QStringList noifiles = QDir( noidir ).entryList(
         noifilt, QDir::Files, QDir::Name );
   noidir               = noidir + "/";
DbgLv(1) << "NIE: noise-files-size" << noifiles.size();

   for ( int ii = 0; ii < noifiles.size(); ii++ )
   {
      QString noiFname   = noifiles.at( ii );
      QString noiPath    = noidir + noiFname;
      US_Noise noise;
      
      if ( noise.load( noiPath ) != US_DB2::OK )
         continue;    // Can't use if can't load

      QString noiDesc    = noise.description;
DbgLv(1) << "NIE:  ii noiDesc" << ii << noiDesc;

      if ( ! noiDesc.startsWith( msetBase ) )
         continue;    // Can't use if not from the model set

      nlnois++;

      nieDescs << noiDesc;
      nieFnams << noiPath;
DbgLv(1) << "NIE:     noiFname" << noiFname;

      if ( ! usesDB )
         nieIDs   << "-1";
   }

   if ( usesDB )
   {
      US_Passwd pw;
      US_DB2 db( pw.getPasswd() );
      QStringList nIDs;
      QString invID = QString::number( US_Settings::us_inv_ID() );

      QStringList edtIDs;
      QStringList edtNams;

      query.clear();
      query << "all_editedDataIDs" << invID;
      db.query( query );

      while( db.next() )
      {
         QString edtID    = db.value( 0 ).toString();
         QString edtName  = db.value( 2 ).toString();

         if ( edtName.startsWith( srchRun )  &&
              edtName.endsWith(   srchTrip ) )
         {
            edtIDs  << edtID;
            edtNams << edtName;
DbgLv(1) << "NIE:  edtID edtName" << edtID << edtName;
         }
      }
DbgLv(1) << "NIE: edtIDs-size" << edtIDs.size();
if ( edtIDs.size() > 0 ) DbgLv(1) << "NIE: edtName0" << edtNams[0];

      query.clear();

      if ( edtIDs.size() == 1 )
         query << "get_noise_desc_by_editID" << invID << edtIDs[ 0 ];

      else
         query << "get_noise_desc" << invID;

      db.query( query );

      while( db.next() )
      {
         QString noiID    = db.value( 0 ).toString();
         QString edtID    = db.value( 2 ).toString();
DbgLv(1) << "NIE:  noiID edtID" << noiID << edtID;

         if ( edtIDs.contains( edtID ) )
            nIDs << noiID;
      }
DbgLv(1) << "NIE: nIDs-size" << nIDs.size() << "msetBase" << msetBase;

      for ( int ii = 0; ii < nIDs.size(); ii++ )
      {
         QString noiID      = nIDs[ ii ];
         US_Noise noise;
      
         if ( noise.load( noiID, &db ) != US_DB2::OK )
            continue;    // Can't use if can't load

         QString noiDesc    = noise.description;
DbgLv(1) << "NIE:  ii noiID noiDesc" << ii << noiID << noiDesc;

         if ( ! noiDesc.startsWith( msetBase ) )
            continue;    // Can't use if not from the model set

         nieIDs   << noiID;

         if ( nlnois == 0 )
         {
            nieFnams << "";
            nieDescs << noiDesc;
         }
      }
   }
DbgLv(1) << "NIE: usesDB" << usesDB << "nlnois" << nlnois
 << "nieDescs-size" << nieDescs.size() << "nieIDs-size" << nieIDs.size();

   return;
}

// Make a ColorMap copy and return a pointer to the new ColorMap
QwtLinearColorMap* US_FitMeniscus::ColorMapCopy( QwtLinearColorMap* colormap )
{
   QVector< double >  cstops   = colormap->colorStops();
   int                lstop    = cstops.count() - 1;
   QwtInterval        csvals( 0.0, 1.0 );
   QwtLinearColorMap* cmapcopy = new QwtLinearColorMap( colormap->color1(),
                                                        colormap->color2() );

   for ( int jj = 1; jj < lstop; jj++ )
   {
      QColor scolor = colormap->color( csvals, cstops[ jj ] );
      cmapcopy->addColorStop( cstops[ jj ], scolor );
   }

   return cmapcopy;
}
