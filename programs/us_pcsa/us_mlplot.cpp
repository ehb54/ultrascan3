//! \file us_mlplot.cpp

#include "us_mlplot.h"
#include "us_pcsa.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_math2.h"
#include "us_zsolute.h"
#include "us_colorgradIO.h"

#include <qwt_legend.h>
#include <qwt_scale_widget.h>
#include <qwt_color_map.h>
#include <qwt_scale_draw.h>

// constructor:  model lines plot widget
US_MLinesPlot::US_MLinesPlot( double& ylo, double& yhi, double& xlo,
      double& xhi, int& typ, int& nkp, int& nlp, int& bmx )
   : US_WidgetsDialog( nullptr, Qt::WindowFlags() ), ymin( ylo ), ymax( yhi ), xmin( xlo ),
   xmax( xhi ), ctype( typ ), nkpts( nkp ), nlpts( nlp ), bmndx( bmx )
{
   // lay out the GUI
   setObjectName( "US_MLinesPlot" );
   setAttribute( Qt::WA_DeleteOnClose, true );
   setWindowTitle( tr( "Parametrically Constrained Spectrum Analysis "
                       "Model Lines Viewer" ) );
   setPalette( US_GuiSettings::frameColor() );

   QSize p1size( 560, 480 );

   dbg_level       = US_Settings::us_debug();
   model           = 0;
   rmsd_best       = 0.0;
   rmsd_worst      = 0.0;
   rmsd_elite      = 0.0;
   rmsd_visib      = 0.0;

   mainLayout      = new QHBoxLayout( this );
   leftLayout      = new QVBoxLayout();
   rightLayout     = new QVBoxLayout();
   pltctrlsLayout  = new QGridLayout();
   buttonsLayout   = new QHBoxLayout();

   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   QLabel* lb_datctrls    = us_banner( tr( "PCSA Model Parameters" ) );
   QLabel* lb_mtype       = us_label(  tr( "Model Type:" ) );
   QLabel* lb_nlines      = us_label(  tr( "Lines (Models):" ) );
   QLabel* lb_npoints     = us_label(  tr( "Max Points per Line:" ) );
   QLabel* lb_varcount    = us_label(  tr( "Variation Count:" ) );
           lb_rmsdhd      = us_label(  tr( "RMSD Cuts" ) );
           lb_ltypeh      = us_label(  tr( "Line Type" ) );
           lb_counth      = us_label(  tr( "Counts" ) );
           lb_neline      = us_label(  tr( "Elite:" ) );
           lb_nsline      = us_label(  tr( "Solutes:" ) );
           lb_nvline      = us_label(  tr( "Visible:" ) );
           lb_rmsdb       = us_label(  tr( "Overall Best Fit:" ) );
           lb_rmsdw       = us_label(  tr( "Overall Worst Fit:" ) );
           pb_colmap      = us_pushbutton( tr( "Color Map" ) );

   QPushButton* pb_help   = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_close  = us_pushbutton( tr( "Close" ) );

   nmodel       = ( ctype != CTYPE_HL  ) ? ( nkpts * nkpts ) : nkpts;
   nmodel       = ( ctype != CTYPE_ALL ) ? nmodel : nmodel * 3;
   nmodel       = ( ctype != CTYPE_2O  ) ? nmodel : nmodel * nkpts;
   neline       = qMax( 2, nmodel / 10 );
   neline       = ( nkpts > 1 ) ? neline : 1;
   nsline       = qMax( 1, neline / 4  );
   nvline       = nmodel;
DbgLv(1) << "RP:  nkpts nmodel" << nkpts << nmodel;
   le_mtype     = us_lineedit( tr( "Straight Line" ),     -1, true );
   le_nlines    = us_lineedit( QString::number( nmodel ), -1, true );
   le_npoints   = us_lineedit( QString::number( nlpts  ), -1, true );
   le_varcount  = us_lineedit( QString::number( nkpts  ), -1, true );
   le_rmsdb     = us_lineedit( QString::number( rmsd_best   ), -1, true );
   le_rmsdw     = us_lineedit( QString::number( rmsd_worst  ), -1, true );
   le_rmsde     = us_lineedit( QString::number( rmsd_elite  ), -1, true );
   le_rmsds     = us_lineedit( QString::number( rmsd_solut  ), -1, true );
   le_rmsdv     = us_lineedit( QString::number( rmsd_visib  ), -1, true );
   ct_neline    = us_counter( 2, 1, nmodel, neline );
   ct_nsline    = us_counter( 2, 1, nmodel, nsline );
   ct_nvline    = us_counter( 2, 1, nmodel, nvline );
   ct_neline->setSingleStep( 1 );
   ct_nsline->setSingleStep( 1 );
   ct_nvline->setSingleStep( 1 );
   le_colmap    = us_lineedit( cmapname,                       -1, true );

   // Set the default color map (rainbow)
   defaultColorMap();

   // Adjust the size of line counters and rmsd text
   lb_nvline->adjustSize();
   QFont        font( US_GuiSettings::fontFamily(),
                      US_GuiSettings::fontSize() );
   QFontMetrics fmet( font );
   int  fwidth   = fmet.maxWidth();
   int  rheight  = lb_nvline->height();
   int  cminw    = fwidth * ( nmodel < 1000 ? 6 : 7 );
   int  csizw    = cminw + fwidth;
   int  tminw    = cminw;
   int  tsizw    = csizw;
   ct_neline->resize( csizw, rheight );
   ct_nsline->resize( csizw, rheight );
   ct_nvline->resize( csizw, rheight );
   ct_neline->setMinimumWidth( cminw );
   ct_nsline->setMinimumWidth( cminw );
   ct_nvline->setMinimumWidth( cminw );
   le_rmsde ->resize( tsizw, rheight );
   le_rmsds ->resize( tsizw, rheight );
   le_rmsdv ->resize( tsizw, rheight );
   le_rmsde ->setMinimumWidth( tminw );
   le_rmsds ->setMinimumWidth( tminw );
   le_rmsdv ->setMinimumWidth( tminw );
DbgLv(1) << "RP:  csizw cminw tsizw" << csizw << cminw << tsizw;

   // Add elements to the controls layout
   int  row      = 0;
   pltctrlsLayout->addWidget( lb_datctrls, row++, 0, 1, 6 );
   pltctrlsLayout->addWidget( lb_mtype,    row,   0, 1, 3 );
   pltctrlsLayout->addWidget( le_mtype,    row++, 3, 1, 3 );
   pltctrlsLayout->addWidget( lb_nlines,   row,   0, 1, 3 );
   pltctrlsLayout->addWidget( le_nlines,   row++, 3, 1, 3 );
   pltctrlsLayout->addWidget( lb_npoints,  row,   0, 1, 3 );
   pltctrlsLayout->addWidget( le_npoints,  row++, 3, 1, 3 );
   pltctrlsLayout->addWidget( lb_varcount, row,   0, 1, 3 );
   pltctrlsLayout->addWidget( le_varcount, row++, 3, 1, 3 );
   pltctrlsLayout->addWidget( lb_ltypeh,   row,   0, 1, 2 );
   pltctrlsLayout->addWidget( lb_counth,   row,   2, 1, 2 );
   pltctrlsLayout->addWidget( lb_rmsdhd,   row++, 4, 1, 2 );
   pltctrlsLayout->addWidget( lb_neline,   row,   0, 1, 2 );
   pltctrlsLayout->addWidget( ct_neline,   row,   2, 1, 2 );
   pltctrlsLayout->addWidget( le_rmsde,    row++, 4, 1, 2 );
   pltctrlsLayout->addWidget( lb_nsline,   row,   0, 1, 2 );
   pltctrlsLayout->addWidget( ct_nsline,   row,   2, 1, 2 );
   pltctrlsLayout->addWidget( le_rmsds,    row++, 4, 1, 2 );
   pltctrlsLayout->addWidget( lb_nvline,   row,   0, 1, 2 );
   pltctrlsLayout->addWidget( ct_nvline,   row,   2, 1, 2 );
   pltctrlsLayout->addWidget( le_rmsdv,    row++, 4, 1, 2 );
   pltctrlsLayout->addWidget( lb_rmsdb,    row,   0, 1, 4 );
   pltctrlsLayout->addWidget( le_rmsdb,    row++, 4, 1, 2 );
   pltctrlsLayout->addWidget( lb_rmsdw,    row,   0, 1, 4 );
   pltctrlsLayout->addWidget( le_rmsdw,    row++, 4, 1, 2 );
   pltctrlsLayout->addWidget( pb_colmap,   row,   0, 1, 2 );
   pltctrlsLayout->addWidget( le_colmap,   row++, 2, 1, 4 );
//   row         += 7;
   QString s_type   = tr( "Unknown" );
   if      ( ctype == CTYPE_SL )  s_type = tr( "Straight Line" );
   else if ( ctype == CTYPE_IS )  s_type = tr( "Increasing Sigmoid" );
   else if ( ctype == CTYPE_DS )  s_type = tr( "Decreasing Sigmoid" );
   else if ( ctype == CTYPE_HL )  s_type = tr( "Horizontal Line [C(s)]" );
   else if ( ctype == CTYPE_ALL ) s_type = tr( "All (SL + IS + DS)" );
   else if ( ctype == CTYPE_2O )  s_type = tr( "Second-Order Power Law" );
   le_mtype->setText( s_type );

   // Hide the color items for now
   showColorItems( false );

   buttonsLayout ->addWidget( pb_help  );
   buttonsLayout ->addWidget( pb_close );

   // Complete layouts and set up signals/slots
   plotLayout1 = new US_Plot( data_plot1,
         tr( "Model Lines" ),
         tr( "Sedimentation Coefficient (x 1e13)" ),
         tr( "Frictional Ratio (f/f0)" ) );

   data_plot1->setCanvasBackground( Qt::black );
   data_plot1->setMinimumSize( p1size );

   rightLayout->addLayout( plotLayout1    );

   leftLayout ->addLayout( pltctrlsLayout );
   leftLayout ->addStretch();
   leftLayout ->addLayout( buttonsLayout  );

   mainLayout->addLayout( leftLayout  );
   mainLayout->addLayout( rightLayout );
   mainLayout->setStretchFactor( leftLayout,  3 );
   mainLayout->setStretchFactor( rightLayout, 5 );

   connect( ct_neline, SIGNAL( valueChanged ( double ) ),
            this,      SLOT  ( updateElite  ( double ) ) );
   connect( ct_nsline, SIGNAL( valueChanged ( double ) ),
            this,      SLOT  ( updateSolutes( double ) ) );
   connect( ct_nvline, SIGNAL( valueChanged ( double ) ),
            this,      SLOT  ( updateVisible( double ) ) );
   connect( pb_colmap, SIGNAL( clicked()        ),
            this,      SLOT  ( selectColorMap() ) );
   connect( pb_help,   SIGNAL( clicked()        ),
            this,      SLOT  ( help()           ) );
   connect( pb_close,  SIGNAL( clicked()        ),
            this,      SLOT  ( close_all()      ) );

DbgLv(1) << "RP:  p1size" << p1size;
   data_plot1->resize( p1size );
   ct_neline ->resize( rheight, csizw );
   ct_nsline ->resize( rheight, csizw );
   ct_nvline ->resize( rheight, csizw );
   setAttribute( Qt::WA_DeleteOnClose, true );
   adjustSize();
DbgLv(1) << "RP:   actual csizw" << ct_nvline->width();
}

// Close button clicked
void US_MLinesPlot::close_all()
{
   close();
}

// Plot the data
void US_MLinesPlot::plot_data()
{
   dataPlotClear( data_plot1 );

   bool   got_best  = ( model != 0  &&  bmndx >= 0 );   // Got best model?
DbgLv(1) << "RP:PD got_best" << got_best << "bmndx" << bmndx;

   us_grid( data_plot1 );
DbgLv(1) << "RP:PD us_grid RTN";

   QString       title;
   QwtPlotCurve* curv;
DbgLv(1) << "RP:PD xmin xmax" << xmin << xmax << "ymin ymax" << ymin << ymax;

   QVector< double > xvec( nlpts, 0.0 );
   QVector< double > yvec( nlpts, 0.0 );

   double* xx    = xvec.data();
   double* yy    = yvec.data();
   int     nmodl = mrecs.size();
   QPen    pen_plot( US_GuiSettings::plotCurve(), 1 );
   int     stype = mrecs[ 0 ].stype;
   int attr_x    = ( stype >> 6 ) & 7;
   int attr_y    = ( stype >> 3 ) & 7;
   int attr_z    =   stype        & 7;
   double xpscl  = ( attr_x == US_ZSolute::ATTR_S ) ? 1.e+13 : 1.0;
   double ypscl  = ( attr_y == US_ZSolute::ATTR_S ) ? 1.e+13 : 1.0;
DbgLv(1) << "RP:PD stype" << stype << "attr_x attr_y attr_z"
 << attr_x << attr_y << attr_z;
   QString xtitl = tr( "Sedimentation Coefficient (x 1e13)" );
   QString ytitl = tr( "Frictional Ratio (f/f0)" );
   xtitl         = ( attr_x == US_ZSolute::ATTR_S )
                   ? tr( "Sedimentation Coefficient (x 1e13)" ) : xtitl;
   xtitl         = ( attr_x == US_ZSolute::ATTR_K )
                   ? tr( "Frictional Ratio (f/f0)" )            : xtitl;
   xtitl         = ( attr_x == US_ZSolute::ATTR_W )
                   ? tr( "Molecular Weight (Dalton)" )          : xtitl;
   xtitl         = ( attr_x == US_ZSolute::ATTR_V )
                   ? tr( "Specific Density (vbar_20W)" )        : xtitl;
   xtitl         = ( attr_x == US_ZSolute::ATTR_D )
                   ? tr( "Diffusion Coefficient" )              : xtitl;
   ytitl         = ( attr_y == US_ZSolute::ATTR_S )
                   ? tr( "Sedimentation Coefficient (x 1e13)" ) : ytitl;
   ytitl         = ( attr_y == US_ZSolute::ATTR_K )
                   ? tr( "Frictional Ratio (f/f0)" )            : ytitl;
   ytitl         = ( attr_y == US_ZSolute::ATTR_W )
                   ? tr( "Molecular Weight (Dalton)" )          : ytitl;
   ytitl         = ( attr_y == US_ZSolute::ATTR_V )
                   ? tr( "Specific Density (vbar_20W)" )        : ytitl;
   ytitl         = ( attr_y == US_ZSolute::ATTR_D )
                   ? tr( "Diffusion Coefficient" )              : ytitl;


   //if ( attr_x == US_ZSolute::ATTR_S )
   if ( attr_x == US_ZSolute::ATTR_S  ||
        attr_x == US_ZSolute::ATTR_K )
   {
      //double xrng   = xmax - xmin;
      //double xinc   = xrng < 15.0 ? 1.0 : ( xrng < 50.0 ? 2.0 : 5.0 );
      //data_plot1->setAxisScale( QwtPlot::xBottom, xmin, xmax, xinc );
      data_plot1->setAxisScale( QwtPlot::xBottom, xmin, xmax );
   }
   else
   {
      data_plot1->setAxisAutoScale( QwtPlot::xBottom );
   }

   //if ( attr_y == US_ZSolute::ATTR_S )
   if ( attr_y == US_ZSolute::ATTR_S  ||
        attr_y == US_ZSolute::ATTR_K )
   {
      //double yrng   = ymax - ymin;
      //double yinc   = yrng < 15.0 ? 1.0 : ( yrng < 50.0 ? 2.0 : 5.0 );
      //data_plot1->setAxisScale( QwtPlot::yLeft,   ymin, ymax, yinc );
DbgLv(1) << "RP:PD   ymin ymax" << ymin << ymax;
      if ( ymin == ymax )
      {
         ymin         *= 0.98;
         ymax         *= 1.02;
      }
      data_plot1->setAxisScale( QwtPlot::yLeft,   ymin, ymax );
   }
   else
   {
      data_plot1->setAxisAutoScale( QwtPlot::yLeft   );
   }

   data_plot1->setAxisTitle( QwtPlot::xBottom, xtitl );
   data_plot1->setAxisTitle( QwtPlot::yLeft,   ytitl );
DbgLv(1) << "RP:PD   got_best" << got_best;
DbgLv(1) << "RP:PD nmodel mrecs_size" << nmodel << nmodl;

   if ( got_best )
   { // Plot lines after best and sorted model records have been produced
DbgLv(1) << "RP:PD mrecs size" << mrecs.size() << nmodl;
      rmsd_best     = mrecs[ 0          ].rmsd;            // Get RMSDs
      rmsd_worst    = mrecs[ nmodel - 1 ].rmsd;
      rmsd_elite    = mrecs[ neline - 1 ].rmsd;
      rmsd_solut    = mrecs[ nsline - 1 ].rmsd;
      rmsd_visib    = mrecs[ nvline - 1 ].rmsd;
      le_rmsdb->setText( QString::number( rmsd_best  ) );  // Document RMSDs
      le_rmsdw->setText( QString::number( rmsd_worst ) );
      le_rmsde->setText( QString::number( rmsd_elite ) );
      le_rmsds->setText( QString::number( rmsd_solut ) );
      le_rmsdv->setText( QString::number( rmsd_visib ) );

      QFont             afont = data_plot1->axisTitle( QwtPlot::yLeft ).font();
      QwtScaleWidget*   rightAxis = data_plot1->axisWidget( QwtPlot::yRight );
      QwtLinearColorMap* revcmap  = reverseColorMap();

      // Set up the right-side axis with the color map
      rightAxis->setColorBarEnabled( true );
      QwtInterval       cdrange( rmsd_best, rmsd_elite );
      rightAxis->setColorMap       ( cdrange, revcmap );
      data_plot1->enableAxis  ( QwtPlot::yRight, true );
      data_plot1->axisTitle   ( QwtPlot::yRight ).setFont( afont );
      data_plot1->setAxisTitle( QwtPlot::yRight, tr( "RMSD" ) );
      data_plot1->setAxisScale( QwtPlot::yRight, rmsd_best, rmsd_elite );

      QColor c_white ( Qt::white );                   // Color white
      QColor c_yellow( Qt::yellow );                  // Color yellow
      QColor c_cyan  ( Qt::cyan );                    // Color cyan
      QPen   pen_best( colormap->color2(), 3 );       // Pen for best line
      QPen   pen_gray( QColor(  64,  64,  64 ), 1 );  // Pen for gray lines
      int    eloffs   = neline - 1;                   // Elite count offset
      double elrange  = (double)eloffs;               // Elite line count range
      int    blnx     = 0;
      if ( mrecs[ 1 ].taskx == mrecs[ 0 ].taskx )
         blnx            = 1;
      if ( mrecs[ 2 ].taskx == mrecs[ 0 ].taskx )
         blnx            = 2;

      // Determine maximum concentration in models with solutes to display
      double max_conc = 0.0;
      for ( int ii = 0; ii < nsline; ii++ )
      {
         for ( int kk = 0; kk < mrecs[ ii ].csolutes.size(); kk++ )
         {
            max_conc     = qMax( max_conc, mrecs[ ii ].csolutes[ kk ].c );
         }
      }
DbgLv(1) << "RP:PD (4)xmin xmax" << xmin << xmax;

      // Draw the lines and solute points

      for ( int ii = ( nmodel - 1 ); ii >= 0; ii-- )
      { // Loop over model records from worst (highest rmsd) to best

         // Skip line processing if beyond visible count
         if ( ii >= nvline )
            continue;
//DbgLv(1) << "RP:PD    ii" << ii << "rmsd_rec" << mrecs[ii].rmsd;
         bool   do_curv  = true;

         if ( ii == blnx )
         { // Best:  color with top color and use wider line
            title   = tr( "Best Curve " ) + QString::number( ii );
            curv    = us_curve( data_plot1, title );
            curv->setPen  ( pen_best );    // Red
         }

         else if ( ii < neline  &&  ii > blnx )
         { // Elite:  color according to position in elite range
            title   = tr( "Elite Curve " ) + QString::number( ii );
            curv    = us_curve( data_plot1, title );
            double position = (double)( eloffs - ii ) / elrange;
            curv->setPen( QPen( positionColor( position ), 2 ) );
         }

         else if ( ii > blnx )
         { // Non-elite:  color gray
            title   = tr( "Curve " ) + QString::number( ii );
            curv    = us_curve( data_plot1, title );
            curv->setPen( pen_gray );
         }

         else
         {
            do_curv = false;
         }

         int klpts   = mrecs[ ii ].isolutes.size();

         if ( ctype == CTYPE_SL  ||  ctype == CTYPE_HL )
         { // For straight line, just draw from start to end
            klpts       = 2;
            xx[ 0 ]     = xmin;
            xx[ 1 ]     = xmax;
            yy[ 0 ]     = mrecs[ ii ].str_y;
            yy[ 1 ]     = mrecs[ ii ].end_y;
         }

         else
         { // Otherwise, set each point on the curve
//DbgLv(1) << "RP:PD nlpts" << nlpts;
//DbgLv(1) << "RP:PD   isol size" << mrecs[ii].isolutes.size();
            for ( int kk = 0; kk < klpts; kk++ )
            {
               xx[ kk ]     = mrecs[ ii ].isolutes[ kk ].x * xpscl;
               yy[ kk ]     = mrecs[ ii ].isolutes[ kk ].y * ypscl;
            }
         }

//DbgLv(1) << "RP:PD   klpts" << klpts;
         if ( do_curv )
            curv->setSamples( xx, yy, klpts );

         if ( ii < nsline )
         { // If within solutes-lines count, plot the solute points
            int ncomp     = mrecs[ ii ].csolutes.size();
            QColor c_symb = c_white;      // Predominant solutes symbol color
            int szdmin    = 2;            //  and likely minimum size
            if ( ii == 1  &&  mrecs[ 1 ].taskx == mrecs[ 0 ].taskx )
            {
               c_symb        = c_yellow;  // Differint symbol color for TR/MC
               szdmin        = 3;         //  and different minimum size
            }
            if ( ii == 2  &&  mrecs[ 2 ].taskx == mrecs[ 0 ].taskx )
            {
               c_symb        = c_cyan;    // Differint symbol color for TR/MC
               szdmin        = 3;         //  and different minimum size
            }

            for ( int kk = 0; kk < ncomp; kk++ )
            {
               double xv   = mrecs[ ii ].csolutes[ kk ].x * xpscl;
               double yv   = mrecs[ ii ].csolutes[ kk ].y * ypscl;
               double cv   = mrecs[ ii ].csolutes[ kk ].c;
               double cfra = cv / max_conc;
               int    szd  = qMax( szdmin, qRound( 9.0 * cfra ) );
               title       = tr( "Solute Curve " ) + QString::number( ii )
                             + " Point " + QString::number( kk );
               curv        = us_curve( data_plot1, title );
               QwtSymbol* symbol = new QwtSymbol;
               symbol->setPen  ( c_symb );
               symbol->setSize ( szd );
               symbol->setStyle( QwtSymbol::Ellipse );
               symbol->setBrush( c_symb );

               curv->setStyle  ( QwtPlotCurve::NoCurve );
               curv->setSymbol ( symbol );
               curv->setSamples( &xv, &yv, 1 );
            }
DbgLv(1) << "RP:PD       ncomp" << ncomp << "x0 y0 xn yn"
 << xx[0] << yy[0] << xx[ncomp-1] << yy[ncomp-1];
         }
      } // END: models loop
DbgLv(1) << "RP:PD (5)xmin xmax" << xmin << xmax;
   }

   else
   { // Plot lines before any best-fit computations
      for ( int ii = 0; ii < nmodl; ii++ )
      {
         for ( int kk = 0; kk < nlpts; kk++ )
         { // Accumulate the curve points
            xx[ kk ]     = mrecs[ ii ].isolutes[ kk ].x * xpscl;
            yy[ kk ]     = mrecs[ ii ].isolutes[ kk ].y * ypscl;
         }

         title   = tr( "Curve " ) + QString::number( ii );
         curv    = us_curve( data_plot1, title );
         curv->setPen    ( pen_plot );    // Default color (yellow)
         curv->setSamples( xx, yy, nlpts );
      } // END: model lines loop
   } // END: pre-fit lines

   data_plot1->replot();
}

// Public slot to set a pointer to a model to use in the plot for highlights
void US_MLinesPlot::setModel( US_Model* a_model,
                               QVector< US_ModelRecord >& mrs )
{
   model   = a_model;
   mrecs   = mrs;
DbgLv(1) << "RP:SM  bmndx" << bmndx << "stype" << mrecs[0].stype;

   // Show or hide the color items based on presence of a model
   showColorItems( ( model != 0 ) );
}

// Handle change in elite line count
void US_MLinesPlot::updateElite( double value )
{
   neline      = (int)value;

   if ( model != 0 )
   {
      rmsd_elite  = mrecs[ neline - 1 ].rmsd;
      le_rmsde->setText( QString::number( rmsd_elite ) );
      plot_data();
   }
}

// Handle change in solutes line count
void US_MLinesPlot::updateSolutes( double value )
{
   nsline      = (int)value;

   if ( model != 0 )
   {
      rmsd_solut  = mrecs[ nsline - 1 ].rmsd;
      le_rmsds->setText( QString::number( rmsd_solut ) );
      plot_data();
   }
}

// Handle change in visible line count
void US_MLinesPlot::updateVisible( double value )
{
   nvline      = (int)value;

   if ( model != 0 )
   {
      rmsd_visib  = mrecs[ nvline - 1 ].rmsd;
      le_rmsdv->setText( QString::number( rmsd_visib ) );
      plot_data();
   }
}

// Generate the default color map (rainbow)
void US_MLinesPlot::defaultColorMap()
{
   cmapname  = tr( "Default (rainbow)" );
   le_colmap->setText( cmapname );
DbgLv(1) << "dCM: cmapname" << cmapname;
   colormap  = new QwtLinearColorMap( Qt::magenta, Qt::red );
   colormap->addColorStop( 0.2000, Qt::blue   );
   colormap->addColorStop( 0.4000, Qt::cyan   );
   colormap->addColorStop( 0.6000, Qt::green  );
   colormap->addColorStop( 0.8000, Qt::yellow );
DbgLv(1) << "dCM: RTN  cmCstopSize" << colormap->colorStops().size();
for ( double pos=0.0; pos<=1.0; pos+=0.1 ) {
DbgLv(1) << "Color at position" << pos << positionColor( pos ); }
}

// Generate the colors given relative position
QColor US_MLinesPlot::positionColor( double pos )
{
//DbgLv(1) << "pC: position" << pos;
   QwtDoubleInterval colorinterv( 0.0, 1.0 );
   return QColor( colormap->rgb( colorinterv, pos ) );
}

// Produce a copy of the color map in reverse order for right-side axis
QwtLinearColorMap* US_MLinesPlot::reverseColorMap()
{
   QwtLinearColorMap* rcolmap = new QwtLinearColorMap( colormap->color2(), colormap->color1() );

   QwtInterval cinterv( 0.0, 1.0 );
   QVector< double > cstops = colormap->colorStops();
   int kstops = cstops.size() - 1;

   for ( int ii = kstops; ii >= 0; ii-- )
   {
      double csvalue  = cstops[ ii ];
      QColor mcolor( colormap->rgb( cinterv, csvalue ) );
DbgLv(1) << "rCM: pos" << csvalue << "color" << mcolor;
      rcolmap->addColorStop( 1.0 - csvalue, mcolor );
   }
DbgLv(1) << "rCM:   color1" << rcolmap->color1() << "color2" << rcolmap->color2();

for ( double pos=0.0; pos<=1.0; pos+=0.2 ) {
DbgLv(1) << "  CM position" << pos << "InColor" << positionColor( pos )
   << "RevColor" << QColor(rcolmap->rgb(cinterv,pos)); }
   return rcolmap;
}

// Show or hide the color-related GUI items
void US_MLinesPlot::showColorItems( bool visible )
{
   lb_ltypeh->setVisible( visible );
   lb_counth->setVisible( visible );
   lb_rmsdhd->setVisible( visible );
   lb_neline->setVisible( visible );
   lb_nsline->setVisible( visible );
   lb_nvline->setVisible( visible );
   lb_rmsdb ->setVisible( visible );
   lb_rmsdw ->setVisible( visible );
   le_rmsde ->setVisible( visible );
   le_rmsds ->setVisible( visible );
   le_rmsdv ->setVisible( visible );
   le_rmsdb ->setVisible( visible );
   le_rmsdw ->setVisible( visible );
   ct_neline->setVisible( visible );
   ct_nsline->setVisible( visible );
   ct_nvline->setVisible( visible );
   pb_colmap->setVisible( visible );
   le_colmap->setVisible( visible );
}

// Select a color map from a file
void US_MLinesPlot::selectColorMap( void )
{
   QString filter = tr( "Color Map files (*cm-*.xml);;"
                        "Any XML files (*.xml);;"
                        "Any files (*)" );

   // get an xml file name for the color map
   QString fname = QFileDialog::getOpenFileName( this,
       tr( "Load Color Map File" ),
       US_Settings::etcDir(), filter );

   if ( fname.isEmpty() )
        return;

   // get the map from the file
   QList< QColor > cmcolors;
   QList< double > cmvalues;

   US_ColorGradIO::read_color_steps( fname, cmcolors, cmvalues );
   colormap  = new QwtLinearColorMap( cmcolors.first(), cmcolors.last() );

   for ( int jj = 1; jj < cmvalues.size() - 1; jj++ )
   {
      colormap->addColorStop( cmvalues.at( jj ), cmcolors.at( jj ) );
   }

   cmapname  = QFileInfo( fname ).baseName().replace( ".xml$", "" );
   le_colmap->setText( cmapname );
   plot_data();
for ( double pos=0.0; pos<=1.0; pos+=0.1 ) {
DbgLv(1) << "Color at position" << pos << positionColor( pos ); }
}

