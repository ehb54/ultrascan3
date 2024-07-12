#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_mals_saxs.h"
#include "../include/us_hydrodyn_mals_saxs_ciq.h"
#include "../include/us_hydrodyn_mals_saxs_dctr.h"
#include "../include/us_hydrodyn_mals_saxs_p3d.h"
#include "../include/us_hydrodyn_mals_saxs_fit.h"
#include "../include/us_hydrodyn_mals_saxs_fit_global.h"
#include "../include/us_lm.h"
#include <qwt_scale_engine.h>
#include <qwt_plot_layout.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include "../include/us_plot_zoom.h"

// #define DEBUG_RESCALE

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#define SLASH QDir::separator()
#define Q_VAL_TOL 5e-6
// #define UHSH_VAL_DEC 8
#define UHSH_UV_CONC_FACTOR 1e0

void US_Hydrodyn_Mals_Saxs::update_plot_errors_group()
{
   if ( !unified_ggaussian_ok )
   {
      editor_msg( "red", "Internal error: update_plot_errors_group(): unified_ggaussian not ok\n" );
      return;
   }

   // we want to do a plot for each curve
   vector < double > grid   = plot_errors_grid  ;
   vector < double > target = plot_errors_target;
   vector < double > fit    = plot_errors_fit   ;
   vector < double > errors = plot_errors_errors;

   double maxy = 0e0;
   double minx = 0e0;
   double maxx = 0e0;

   for ( unsigned int j = 0; j < unified_ggaussian_curves; j++ )
   {
      vector < double > x;
      vector < double > y;
      vector < double > e;

      // well, I good compiler should unwrap the if's if I put them inside one loop, but I am uncertain
      // printvector( "target", target );
      // printvector( "fit", fit );

      if ( cb_plot_errors_pct->isChecked() )
      {
         if ( cb_plot_errors_sd->isChecked() && errors.size() == target.size() && is_nonzero_vector( errors ))
         {
            // does % and errors make sense?, I am excluding this for now
            // cout << "pct mode with errors, not acceptable\n";
            for ( unsigned int i = unified_ggaussian_q_start[ j ]; i < unified_ggaussian_q_end[ j ]; i++ )
            {
               if ( target[ i ] != 0e0 )
               {
                  x.push_back( unified_ggaussian_q[ ( unsigned int ) grid[ i ] ] );
                  y.push_back( 0e0 );
                  e.push_back( 100.0 * ( target[ i ] - fit[ i ] ) / target[ i ] );
               }
            }
         } else {
            // cout << "pct mode, not using errors\n";

            for ( unsigned int i = unified_ggaussian_q_start[ j ]; i < unified_ggaussian_q_end[ j ]; i++ )
            {
               if ( target[ i ] != 0e0 )
               {
                  x.push_back( unified_ggaussian_q[ ( unsigned int ) grid[ i ] ] );
                  y.push_back( 0e0 );
                  e.push_back( 100.0 * ( target[ i ] - fit[ i ] ) / target[ i ] );
               } else {
                  cout << QString( "target at pos %1 is zero\n" ).arg( i );
               }
            }
         }         
      } else {
         if ( cb_plot_errors_sd->isChecked() && errors.size() == target.size() && is_nonzero_vector( errors ))
         {
            // cout << "errors ok & used\n";
            for ( unsigned int i = unified_ggaussian_q_start[ j ]; i < unified_ggaussian_q_end[ j ]; i++ )
            {
               x.push_back( unified_ggaussian_q[ ( unsigned int ) grid[ i ] ] );
               y.push_back( 0e0 );
               e.push_back( ( target[ i ] - fit[ i ] ) / errors[ i ] );
            }
         } else {
            // cout << "errors not ok & not used\n";
            for ( unsigned int i = unified_ggaussian_q_start[ j ]; i < unified_ggaussian_q_end[ j ]; i++ )
            {
               x.push_back( unified_ggaussian_q[ ( unsigned int ) grid[ i ] ] );
               y.push_back( 0e0 );
               e.push_back( target[ i ] - fit[ i ] );
            }
         }
      }

      for ( unsigned int i = 0; i < ( unsigned int ) e.size(); i++ )
      {
         if ( e[ i ] < -50e0 )
         {
            e[ i ] = -50e0;
         } else {
            if ( e[ i ] > 50e0 )
            {
               e[ i ] = 50e0;
            }
         }
      }

      //       printvector( QString( "x for curve %1" ).arg( j ), x );
      //       printvector( QString( "e for curve %1" ).arg( j ), e );

      // only need the bar once

      if ( !j ) 
      {
         QwtPlotCurve *curve = new QwtPlotCurve( "base" );
         curve->setStyle( QwtPlotCurve::Lines );

         curve->setPen( QPen( Qt::green, use_line_width, Qt::SolidLine ) );
         curve->setSamples(
                        (double *)&x[ 0 ],
                        (double *)&y[ 0 ],
                        x.size()
                        );
         curve->attach( plot_errors );
      }

      {
         QwtPlotCurve *curve = new QwtPlotCurve( "errors" );
         curve->setStyle( QwtPlotCurve::Lines );

         curve->setPen( QPen( plot_colors[ f_pos[ unified_ggaussian_files[ j ] ] % plot_colors.size() ], use_line_width, Qt::SolidLine ) );
         curve->setSamples(
                        (double *)&x[ 0 ],
                        (double *)&e[ 0 ],
                        x.size()
                        );
         curve->setStyle( QwtPlotCurve::Sticks );
         curve->attach( plot_errors );
      }

      if ( !j )
      {
         maxy = fabs( e[ 0 ] );
         minx = x[ 0 ];
         maxx = x[ 0 ];
      }

      for ( unsigned int i = 0; i < ( unsigned int )e.size(); i++ )
      {
         if ( maxy < fabs( e[ i ] ) )
         {
            maxy = fabs( e[ i ] );
         }
         if ( minx > x[ i ] )
         {
            minx = x[ i ];
         }
         if ( maxx < x[ i ] )
         {
            maxx = x[ i ];
         }
      }            
   } // for each curve

   cout << QString( "upeg: maxy %1\n" ).arg( maxy );

   // if ( !plot_errors_zoomer )
   // {
   //    cout << "upeg: recreating axis\n";
   if ( current_mode == MODE_GGAUSSIAN ||
        current_mode == MODE_GAUSSIAN ) {
      plot_errors->setAxisTitle(QwtPlot::yLeft, us_tr( cb_plot_errors_pct->isChecked() ?
                                                       "% difference" :
                                                       ( cb_plot_errors_sd->isChecked() ?
                                                         "delta I(t)/sd" : "delta I(t)" 
                                                         ) ) );
   } else {
      plot_errors->setAxisTitle(QwtPlot::yLeft, us_tr( cb_plot_errors_pct->isChecked() ?
                                                       "% difference" :
                                                       ( cb_plot_errors_sd->isChecked() ?
                                                         "delta I(q)/sd" : "delta I(q)" 
                                                         ) ) );
   }
   plot_errors->setAxisScale( QwtPlot::xBottom, minx, maxx );
   plot_errors->setAxisScale( QwtPlot::yLeft  , -maxy * 1.2e0 , maxy * 1.2e0 );
   plot_errors_zoomer->setZoomBase();

#if defined( DEBUG_RESCALE )
   qDebug() <<
      QString("").sprintf(
                          "mals_saxs_plots::update_plot_errors_group\n"
                          "\tminx %e maxx %e\n"
                          "\tminy %e maxy %e\n"
                          , minx, maxx
                          , -maxy, maxy
                          );
#endif

   //       plot_errors_zoomer = new ScrollZoomer(plot_errors->canvas());
   //       plot_errors_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
      // connect( plot_errors_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_zoomed( const QRectF & ) ) );
   // }

   if ( !suppress_replot )
   {
      rescale_y_plot_errors();
      plot_errors->replot();
   }
}

void US_Hydrodyn_Mals_Saxs::update_plot_errors( vector < double > &grid, 
                                                vector < double > &target, 
                                                vector < double > &fit, 
                                                vector < double > &errors,
                                                QColor             use_color )
{
   if ( current_mode == MODE_SCALE )
   {
      return scale_update_plot_errors();
   }

   plot_errors->detachItems( QwtPlotItem::Rtti_PlotCurve );
   plot_errors->detachItems( QwtPlotItem::Rtti_PlotMarker );;

   plot_errors_grid   = grid;
   plot_errors_target = target;
   plot_errors_fit    = fit;
   plot_errors_errors = errors;
   plot_errors_color  = use_color;

   if ( !target.size() )
   {
      // editor_msg( "red", "Internal error: update_plot_errors(): empty data\n" );
      return;
   }

   if ( target.size() != fit.size() )
   {
      editor_msg( "red", "Internal error: update_plot_errors(): target.size() != fit.size()\n" );
      return;
   }
   if ( target.size() != grid.size() )
   {
      editor_msg( "red", "Internal error: update_plot_errors(): target.size() != grid.size()\n" );
      return;
   }

   if ( current_mode == MODE_GGAUSSIAN && cb_plot_errors_group->isChecked() )
   {
      update_plot_errors_group();
      return;
   }

   vector < double > x;
   vector < double > y;
   vector < double > e;

   // well, I good compiler should unwrap the if's if I put them inside one loop, but I am uncertain
   // printvector( "target", target );
   // printvector( "fit", fit );

   if ( cb_plot_errors_pct->isChecked() )
   {
      if ( cb_plot_errors_sd->isChecked() && errors.size() == target.size() && is_nonzero_vector( errors ))
      {
         // does % and errors make sense?, I am excluding this for now
         // cout << "pct mode with errors, not acceptable\n";
         for ( unsigned int i = 0; i < ( unsigned int )target.size(); i++ )
         {
            if ( target[ i ] != 0e0 )
            {
               x.push_back( grid[ i ] );
               y.push_back( 0e0 );
               e.push_back( 100.0 * ( target[ i ] - fit[ i ] ) / target[ i ] );
            }
         }
      } else {
         // cout << "pct mode, not using errors\n";

         for ( unsigned int i = 0; i < ( unsigned int )target.size(); i++ )
         {
            if ( target[ i ] != 0e0 )
            {
               x.push_back( grid[ i ] );
               y.push_back( 0e0 );
               e.push_back( 100.0 * ( target[ i ] - fit[ i ] ) / target[ i ] );
            } else {
               cout << QString( "target at pos %1 is zero\n" ).arg( i );
            }
         }
      }         
   } else {
      if ( cb_plot_errors_sd->isChecked() && errors.size() == target.size() && is_nonzero_vector( errors ))
      {
         // cout << "errors ok & used\n";
         for ( unsigned int i = 0; i < ( unsigned int )target.size(); i++ )
         {
            x.push_back( grid[ i ] );
            y.push_back( 0e0 );
            e.push_back( ( target[ i ] - fit[ i ] ) / errors[ i ] );
         }
      } else {
         // cout << "errors not ok & not used\n";
         for ( unsigned int i = 0; i < ( unsigned int )target.size(); i++ )
         {
            x.push_back( grid[ i ] );
            y.push_back( 0e0 );
            e.push_back( target[ i ] - fit[ i ] );
         }
      }
   }

   for ( unsigned int i = 0; i < ( unsigned int ) e.size(); i++ )
   {
      if ( e[ i ] < -50e0 )
      {
         e[ i ] = -50e0;
      } else {
         if ( e[ i ] > 50e0 )
         {
            e[ i ] = 50e0;
         }
      }
   }

   // printvector( "x", x );
   // printvector( "e", e );

   {
      QwtPlotCurve *curve = new QwtPlotCurve( "base" );
      curve->setStyle( QwtPlotCurve::Lines );

      curve->setPen( QPen( Qt::green, use_line_width, Qt::SolidLine ) );
      curve->setSamples(
                     (double *)&x[ 0 ],
                     (double *)&y[ 0 ],
                     x.size()
                     );
      curve->attach( plot_errors );
   }

   {
      QwtPlotCurve *curve = new QwtPlotCurve( "errors" );
      curve->setStyle( QwtPlotCurve::Lines );

      curve->setPen( QPen( plot_errors_color, use_line_width, Qt::SolidLine ) );
      curve->setSamples(
                     (double *)&x[ 0 ],
                     (double *)&e[ 0 ],
                     x.size()
                     );
      curve->setStyle( QwtPlotCurve::Sticks );
      curve->attach( plot_errors );
   }

   // if ( !plot_errors_zoomer )
   {
      double maxy = e[ 0 ];

      for ( unsigned int i = 1; i < ( unsigned int )e.size(); i++ )
      {
         if ( maxy < fabs( e[ i ] ) )
         {
            maxy = fabs( e[ i ] );
         }
      }            

      if ( current_mode == MODE_GGAUSSIAN ||
           current_mode == MODE_GAUSSIAN ) {
         plot_errors->setAxisTitle(QwtPlot::yLeft, us_tr( cb_plot_errors_pct->isChecked() ?
                                                          "% difference" :
                                                          ( cb_plot_errors_sd->isChecked() ?
                                                            "delta I(t)/sd" : "delta I(t)" 
                                                            ) ) );
      } else {
         plot_errors->setAxisTitle(QwtPlot::yLeft, us_tr( cb_plot_errors_pct->isChecked() ?
                                                          "% difference" :
                                                          ( cb_plot_errors_sd->isChecked() ?
                                                            "delta I(q)/sd" : "delta I(q)" 
                                                            ) ) );
      }         

      plot_errors->setAxisScale( QwtPlot::xBottom, x[ 0 ], x.back() );
      plot_errors->setAxisScale( QwtPlot::yLeft  , -maxy * 1.2e0 , maxy * 1.2e0 );
      plot_errors_zoomer->setZoomBase();
      
#if defined( DEBUG_RESCALE )
      {
         QTextStream tso(stdout);
         tso << "---\n";

         tso <<
            QString("").sprintf(
                                "mals_saxs_plots::update_plot_errors\n"
                                "\tminx %e maxx %e\n"
                                "\tminy %e maxy %e\n"
                                , x[0], x.back()
                                , -maxy, maxy
                                );
         tso << "zoomrect "
             << plot_errors_zoomer->zoomRect().left() << " , "
             << plot_errors_zoomer->zoomRect().right() << " : " 
             << plot_errors_zoomer->zoomRect().bottom() << " , "
             << plot_errors_zoomer->zoomRect().top()
             << "\n"
            ;
         tso << "zoombase "
             << plot_errors_zoomer->zoomBase().left() << " , "
             << plot_errors_zoomer->zoomBase().right() << " : " 
             << plot_errors_zoomer->zoomBase().bottom() << " , "
             << plot_errors_zoomer->zoomBase().top()
             << "\n"
            ;
         tso << "---\n";
      }
#endif

      //       plot_errors_zoomer = new ScrollZoomer(plot_errors->canvas());
      //       plot_errors_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
      //       connect( plot_errors_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_zoomed( const QRectF & ) ) );
   }

   if ( !suppress_replot )
   {
      rescale_y_plot_errors();
      plot_errors->replot();
   }
}

void US_Hydrodyn_Mals_Saxs::redo_plot_errors()
{
   if ( current_mode == MODE_GGAUSSIAN ||
        current_mode == MODE_GAUSSIAN ) {
      plot_errors->setAxisTitle(QwtPlot::yLeft, us_tr( cb_plot_errors_pct->isChecked() ?
                                                    "% difference" :
                                                    ( cb_plot_errors_sd->isChecked() ?
                                                      "delta I(t)/sd" : "delta I(t)" 
                                                      ) ) );
   } else {
      plot_errors->setAxisTitle(QwtPlot::yLeft, us_tr( cb_plot_errors_pct->isChecked() ?
                                                    "% difference" :
                                                    ( cb_plot_errors_sd->isChecked() ?
                                                      "delta I(q)/sd" : "delta I(q)" 
                                                      ) ) );

   }
   // if ( plot_errors_zoomer )
   // {
   //    delete plot_errors_zoomer;
   //    plot_errors_zoomer = (ScrollZoomer *) 0;
   // }
   vector < double > grid   = plot_errors_grid;
   vector < double > fit    = plot_errors_fit;
   vector < double > target = plot_errors_target;
   vector < double > errors = plot_errors_errors;

   update_plot_errors( grid, target, fit, errors, plot_errors_color );
   if ( current_mode == MODE_GGAUSSIAN && !cb_ggauss_scroll->isChecked() )
   {
      plot_errors_jump_markers();
   }
}

void US_Hydrodyn_Mals_Saxs::set_plot_errors_rev()
{

   vector < double > tmp = plot_errors_fit;
   plot_errors_fit       = plot_errors_target;
   plot_errors_target    = tmp;

   redo_plot_errors();
}

void US_Hydrodyn_Mals_Saxs::set_plot_errors_sd()
{
   if ( cb_plot_errors_sd->isChecked() &&
        cb_plot_errors_pct->isChecked() )
   {
      disconnect( cb_plot_errors_pct, SIGNAL( clicked() ), 0, 0 );
      cb_plot_errors_pct->setChecked( false );
      connect( cb_plot_errors_pct, SIGNAL( clicked() ), SLOT( set_plot_errors_pct() ) );
   }

   redo_plot_errors();
}

void US_Hydrodyn_Mals_Saxs::set_plot_errors_pct()
{

   if ( cb_plot_errors_pct->isChecked() &&
        cb_plot_errors_sd->isChecked() )
   {
      disconnect( cb_plot_errors_sd, SIGNAL( clicked() ), 0, 0 );
      cb_plot_errors_sd->setChecked( false );
      connect( cb_plot_errors_sd, SIGNAL( clicked() ), SLOT( set_plot_errors_sd() ) );
   }
   redo_plot_errors();
}

void US_Hydrodyn_Mals_Saxs::set_plot_errors_group()
{
   redo_plot_errors();
}

void US_Hydrodyn_Mals_Saxs::plot_errors_jump_markers()
{
   if ( !unified_ggaussian_ok || cb_plot_errors_group->isChecked() )
   {
      return;
   }

   for ( unsigned int i = 0; i < unified_ggaussian_curves; i++ )
   {
      QwtPlotMarker * marker = new QwtPlotMarker;
      marker->setLineStyle       ( QwtPlotMarker::VLine );
      marker->setLinePen         ( QPen( Qt::cyan, 2, Qt::DashDotDotLine ) );
      marker->setLabelOrientation( Qt::Horizontal );
      marker->setXValue          ( unified_ggaussian_jumps[ i ] );
      marker->setLabelAlignment  ( Qt::AlignRight | Qt::AlignTop );
      {
         QwtText qwtt( QString( "%1" ).arg( i + 1 ) );
         qwtt.setFont( QFont("Helvetica", 11, QFont::Bold ) );
         marker->setLabel           ( qwtt );
      }
      marker->attach             ( plot_errors );
   }
   if ( !suppress_replot )
   {
      plot_errors->replot();
   }
}

void US_Hydrodyn_Mals_Saxs::resize_plots() {
   // qDebug() << "mals_saxs::resize_plots()";
   plot_dist->enableAxis( QwtPlot::xBottom, !plot_errors->isVisible() );
   if ( plot_errors->isVisible() ) {
      vbl_plot_group->setStretchFactor( qs_plots, 80 );
      vbl_plot_group->setStretchFactor( l_plot_errors, 20 );
   } else {
      vbl_plot_group->setStretchFactor( qs_plots, 0 );
      vbl_plot_group->setStretchFactor( l_plot_errors, 0 );
   }
}

void US_Hydrodyn_Mals_Saxs::resize_guinier_plots() {
   // setup qs_spliter to handle smaller residuals
   // qDebug() << "QSplitter qs_plots sizes:" << qs_plots->sizes();
   QList<int> qs_sizes = qs_plots->sizes();
   if ( guinier_plot_errors->isVisible() ) {
      int gp_size  = qs_sizes[qs_plots->indexOf( guinier_plot        )];
      int gpe_size = qs_sizes[qs_plots->indexOf( guinier_plot_errors )];
      int gpt_size = gp_size + gpe_size;
      int new_gp_size  = (int)( 0.8 * gpt_size );
      int new_gpe_size = gpt_size - new_gp_size;
      qs_sizes[ qs_plots->indexOf( guinier_plot        ) ] = new_gp_size;
      qs_sizes[ qs_plots->indexOf( guinier_plot_errors ) ] = new_gpe_size;
      for ( QList<int>::iterator it = qs_sizes.begin();
            it != qs_sizes.end();
            ++it ) {
         if ( !*it ) {
            *it = 1;
         }
      }

   } else {
      for ( QList<int>::iterator it = qs_sizes.begin();
            it != qs_sizes.end();
            ++it ) {
         *it = 1;
      }
   }
   qs_plots->setSizes( qs_sizes );
   // qDebug() << "QSplitter qs_plots sizes after:" << qs_plots->sizes();

   guinier_plot       ->replot();
   guinier_plot_errors->replot();
}   


void US_Hydrodyn_Mals_Saxs::errors()
{
   // qDebug() << "::errors()";
   if ( current_mode == MODE_GUINIER )
   {
      // qDebug() << "::errors() MODE_GUINIER";
      US_Plot_Util::align_plot_extents( { guinier_plot, guinier_plot_errors } );
      connect(((QObject*)guinier_plot       ->axisWidget(QwtPlot::xBottom)) , SIGNAL(scaleDivChanged () ), usp_guinier_plot_errors , SLOT(scaleDivChangedXSlot () ), Qt::UniqueConnection );
      connect(((QObject*)guinier_plot_errors->axisWidget(QwtPlot::xBottom)) , SIGNAL(scaleDivChanged () ), usp_guinier_plot        , SLOT(scaleDivChangedXSlot () ), Qt::UniqueConnection );

      ShowHide::hide_widgets( guinier_errors_widgets, always_hide_widgets, guinier_plot_errors->isVisible() );
      guinier_plot->enableAxis( QwtPlot::xBottom, !guinier_plot_errors->isVisible() );

      resize_guinier_plots();
         
      update_enables();
      return;
   }

   if ( plot_errors->isVisible() )
   {
      hide_widgets( plot_errors_widgets, true );
      if ( current_mode == MODE_SCALE ) {
         // "running" so normal update enables doesn't handle this
         plot_dist->enableAxis( QwtPlot::xBottom, !plot_errors->isVisible() );
      }
      // qDebug() << "::errors() plot_errors isVisible(), no further mode checking";
   } else {

      US_Plot_Util::align_plot_extents( { plot_dist, plot_errors } );
      connect(((QObject*)plot_dist  ->axisWidget(QwtPlot::xBottom)) , SIGNAL(scaleDivChanged () ), usp_plot_errors, SLOT(scaleDivChangedXSlot () ), Qt::UniqueConnection );
      connect(((QObject*)plot_errors->axisWidget(QwtPlot::xBottom)) , SIGNAL(scaleDivChanged () ), usp_plot_dist  , SLOT(scaleDivChangedXSlot () ), Qt::UniqueConnection );
      
      switch( current_mode )
      {
      case MODE_GGAUSSIAN:
         {
            // qDebug() << "::errors() MODE_GGAUSSIAN";
            hide_widgets( plot_errors_widgets, false );
            if ( !unified_ggaussian_use_errors )
            {
               disconnect( cb_plot_errors_sd, SIGNAL( clicked() ), 0, 0 );
               cb_plot_errors_sd->setChecked( false );
               connect( cb_plot_errors_sd, SIGNAL( clicked() ), SLOT( set_plot_errors_sd() ) );
               cb_plot_errors_sd->hide();
            }
         }
         break;

      case MODE_SCALE :
         {
            // qDebug() << "::errors() MODE_SCALE";
            hide_widgets( plot_errors_widgets, false );
            cb_plot_errors_group  ->hide();
            cb_plot_errors_pct    ->hide();
            cb_plot_errors_sd     ->hide();
            cb_plot_errors_rev    ->hide();

            // QStringList files = all_selected_files();
            // bool use_errors = true;
            // for ( int i = 0; i < (int) files.size(); ++i )
            // {
            //    us_qdebug( files[ i ] );
            //    if ( !f_errors.count( files[ i ] ) ||
            //         !is_nonzero_vector( f_errors[ files[ i ] ] ) ||
            //         f_errors[ files[ i ] ].size() != f_Is[ files[ i ] ].size() )
            //    {
            //       use_errors = false;
            //       break;
            //    }
            // }
               
            // if ( !use_errors )
            // {
            //    disconnect( cb_plot_errors_sd, SIGNAL( clicked() ), 0, 0 );
            //    cb_plot_errors_sd->setChecked( false );
            //    connect( cb_plot_errors_sd, SIGNAL( clicked() ), SLOT( set_plot_errors_sd() ) );
            //    cb_plot_errors_sd->hide();
            // }

            // "running" so normal update enables doesn't handle this
            plot_dist->enableAxis( QwtPlot::xBottom, !plot_errors->isVisible() );
         }
         break;

      case MODE_GAUSSIAN :
         {
            // qDebug() << "::errors() MODE_GAUSSIAN";
            hide_widgets( plot_errors_widgets, false );
            cb_plot_errors_group->hide();
            if ( !f_errors.count( wheel_file ) ||
                 !is_nonzero_vector( f_errors[ wheel_file ] ) )
            {
               disconnect( cb_plot_errors_sd, SIGNAL( clicked() ), 0, 0 );
               cb_plot_errors_sd->setChecked( false );
               connect( cb_plot_errors_sd, SIGNAL( clicked() ), SLOT( set_plot_errors_sd() ) );
               cb_plot_errors_sd->hide();
            }
         }
         break;

      default :
         {
            // compare 2 files
            // qDebug() << "::errors() default";
            QStringList files = all_selected_files();
            if ( files.size() > 2 )
            {
               errors_multi_file( files );
            }
            if ( files.size() != 2 )
            {
               editor_msg( "red", us_tr( "Internal error: plot residuals, not 2 selected files" ) );
               return;
            }
            if ( f_qs[ files[ 0 ] ] != f_qs[ files[ 1 ] ] )
            {
               editor_msg( "red", us_tr( "Error: Residuals incompatible grids, you could try 'crop common'" ) );
               return;
            }
            vector < double > errors;
            bool use_errors_0 = 
               f_errors.count( files[ 0 ] ) && is_nonzero_vector( f_errors[ files[ 0 ] ] );
            bool use_errors_1 = 
               f_errors.count( files[ 1 ] ) && is_nonzero_vector( f_errors[ files[ 1 ] ] );
            update_plot_errors( f_qs[ files[ 0 ] ], 
                                f_Is[ files[ 0 ] ], 
                                f_Is[ files[ 1 ] ],
                                use_errors_0 ? f_errors[ files[ 0 ] ] :
                                ( use_errors_1 ? f_errors[ files[ 1 ] ] : errors )
                                );

            if ( use_errors_0 ) {
               double  chi2;
               double  nchi;
               QString errors;
               if ( US_Saxs_Util::calc_chisq(
                                             f_Is[ files[ 0 ] ]
                                             ,f_Is[ files[ 1 ] ]
                                             ,f_errors[ files[ 0 ] ]
                                             ,0
                                             ,chi2
                                             ,nchi
                                             ,errors ) ) {
                  editor_msg( "black", QString( "fit to %1 nchi %2\n" ).arg( files[ 0 ] ).arg( nchi ) );
               } else {
                  editor_msg( "red", QString( "fit to %1 %2\n" ).arg( files[ 0 ] ).arg( errors ) );
               }
            }
            if ( use_errors_1 ) {
               double  chi2;
               double  nchi;
               QString errors;
               if ( US_Saxs_Util::calc_chisq(
                                             f_Is[ files[ 0 ] ]
                                             ,f_Is[ files[ 1 ] ]
                                             ,f_errors[ files[ 1 ] ]
                                             ,0
                                             ,chi2
                                             ,nchi
                                             ,errors ) ) {
                  editor_msg( "black", QString( "fit to %1 nchi %2\n" ).arg( files[ 1 ] ).arg( nchi ) );
               } else {
                  editor_msg( "red", QString( "fit to %1 %2\n" ).arg( files[ 1 ] ).arg( errors ) );
               }
            }
            
            hide_widgets( plot_errors_widgets, false );
            cb_plot_errors_group->hide();
            if ( !use_errors_0 && !use_errors_0 )
            {
               disconnect( cb_plot_errors_sd, SIGNAL( clicked() ), 0, 0 );
               cb_plot_errors_sd->setChecked( false );
               connect( cb_plot_errors_sd, SIGNAL( clicked() ), SLOT( set_plot_errors_sd() ) );
               cb_plot_errors_sd->hide();
            }
         }   
         break;
      }
   }
   // qDebug() << "::errors() call update_enables()";
   update_enables();
}

bool US_Hydrodyn_Mals_Saxs::compatible_grids( QStringList files )
{
   vector < vector < double > > grids;
   for ( int i = 0; i < (int) files.size(); ++i )
   {
      QString this_file = files[ i ];
      if ( f_qs.count( this_file ) &&
           f_Is.count( this_file ) &&
           f_qs[ this_file ].size() &&
           f_Is[ this_file ].size() )
      {
         grids.push_back( f_qs[ this_file ] );
      }
   }
   vector < double > v_union = US_Vector::vunion( grids );
   vector < double > v_int   = US_Vector::intersection( grids );

   return v_union == v_int;
}
   
void US_Hydrodyn_Mals_Saxs::errors_multi_file( QStringList files )
{
   // check 
   if ( !compatible_grids( files ) )
   {
      editor_msg( "red", us_tr( "Residuals: curves must be on the same grid, try 'Crop Common' first." ) );
      return;
   }
}

// void US_Hydrodyn_Mals_Saxs::set_eb()
// {
//    if ( current_mode == MODE_GGAUSSIAN ) {
//       if ( cb_ggauss_scroll->isChecked() && unified_ggaussian_ok ) {
//          ggaussian_rmsd();
//          ggauss_scroll_highlight( qwtw_wheel->value() );
//       }
//    } else {
//       plot_files();
//       update_enables();
//    }
// }


void US_Hydrodyn_Mals_Saxs::set_dots() {
   set_eb();
}

void US_Hydrodyn_Mals_Saxs::set_eb()
{
   US_Plot_Zoom upz ( plot_dist, plot_dist_zoomer );
   if ( current_mode == MODE_GGAUSSIAN ) {
      if ( cb_ggauss_scroll->isChecked() && unified_ggaussian_ok ) {
         ggaussian_rmsd();
         ggauss_scroll_highlight( qwtw_wheel->value() );
      }
   } else {
      plot_files();
      if ( current_mode == MODE_BASELINE ) {
         if ( baseline_test_mode ) {
         } else {
            baseline_init_markers();
            replot_baseline( "set_eb" );
         }
      } else {
         update_enables();
      }
   }
   upz.restore( !suppress_replot );
}

void US_Hydrodyn_Mals_Saxs::plot_files( bool save_zoom_state )
{
   qDebug() << "plot files";
   if ( suppress_plot ) {
      qDebug() << "plot files - supressed";
      return;
   }
      
   scale_pair_fit_clear( false );

   plot_dist->detachItems( QwtPlotItem::Rtti_PlotCurve );
   if ( current_mode != MODE_SCALE_PAIR ) {
      plot_dist->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   }
   bool any_selected = false;
   double minx = 0e0;
   double maxx = 1e0;
   double miny = 0e0;
   double maxy = 1e0;

   double file_minx;
   double file_maxx;
   double file_miny;
   double file_maxy;
   
   bool first = true;

   plotted_curves.clear( );

   int asfs = (int) all_selected_files().size();

   if ( ( !baseline_test_mode && cb_eb->isChecked() && asfs > 9 ) 
        || ( baseline_test_mode && cb_eb->isChecked() && asfs > 15 ) )
   {
      cb_eb->setChecked( false );
   }

   if ( ( asfs > 20 ||
      cb_eb->isChecked() )
      &&
        legend_vis
        )
   {
      legend();
   }

   if ( asfs == 1 )
   {
      update_ref();
   }

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         any_selected = true;
         if ( plot_file( lb_files->item( i )->text(), file_minx, file_maxx, file_miny, file_maxy ) )
         {
            if ( first )
            {
               minx = file_minx;
               maxx = file_maxx;
               miny = file_miny;
               maxy = file_maxy;
               first = false;
            } else {
               if ( file_minx < minx )
               {
                  minx = file_minx;
               }
               if ( file_maxx > maxx )
               {
                  maxx = file_maxx;
               }
               if ( file_miny < miny )
               {
                  miny = file_miny;
               }
               if ( file_maxy > maxy )
               {
                  maxy = file_maxy;
               }
            }
         }
      } else {
         if ( 0 ) {
            if ( get_min_max( lb_files->item( i )->text(), file_minx, file_maxx, file_miny, file_maxy ) )
            {
               if ( first )
               {
                  minx = file_minx;
                  maxx = file_maxx;
                  miny = file_miny;
                  maxy = file_maxy;
                  first = false;
               } else {
                  if ( file_minx < minx )
                  {
                     minx = file_minx;
                  }
                  if ( file_maxx > maxx )
                  {
                     maxx = file_maxx;
                  }
                  if ( file_miny < miny )
                  {
                     miny = file_miny;
                  }
                  if ( file_maxy > maxy )
                  {
                     maxy = file_maxy;
                  }
               }
            }
         }
      }
   }

   // cout << QString( "plot range x [%1:%2] y [%3:%4]\n" ).arg(minx).arg(maxx).arg(miny).arg(maxy);

   // enable zooming

   if ( any_selected && !save_zoom_state ) {
      // qDebug() << "plot_files():: zoom enables";
      plot_dist->setAxisScale( QwtPlot::xBottom, minx, maxx );
      plot_dist->setAxisScale( QwtPlot::yLeft  , miny * 0.9e0 , maxy * 1.1e0 );
      plot_dist_zoomer->setZoomBase();
      plot_errors->setAxisScale( QwtPlot::xBottom, minx, maxx );
      plot_errors_zoomer->setZoomBase();
   }
   
#if defined( DEBUG_RESCALE )
   {
      QTextStream tso(stdout);
      tso << "---\n";

      tso <<
         QString("").sprintf(
                             "mals_saxs_plots::plot_files\n"
                             "\tminx %e maxx %e\n"
                             "\tminy %e maxy %e\n"
                             , minx, maxx
                             , miny, maxy
                             );
      tso << "zoomrect "
          << plot_dist_zoomer->zoomRect().left() << " , "
          << plot_dist_zoomer->zoomRect().right() << " : " 
          << plot_dist_zoomer->zoomRect().bottom() << " , "
          << plot_dist_zoomer->zoomRect().top()
          << "\n"
         ;
      tso << "zoombase "
          << plot_dist_zoomer->zoomBase().left() << " , "
          << plot_dist_zoomer->zoomBase().right() << " : " 
          << plot_dist_zoomer->zoomBase().bottom() << " , "
          << plot_dist_zoomer->zoomBase().top()
          << "\n"
         ;
      tso << "---\n";
   }
#endif

   legend_set();
   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
}

bool US_Hydrodyn_Mals_Saxs::get_min_max( QString file,
                                         double &minx,
                                         double &maxx,
                                         double &miny,
                                         double &maxy )
{
   if ( current_mode == MODE_SCALE )
   {
      //    us_qdebug( QString("get min max mode scale %1" ).arg( file ) );
      if ( !scale_q     .count( file ) ||
           !scale_I     .count( file ) ||
           !f_pos       .count( file ) )
      {
         // editor_msg( "red", QString( us_tr( "Internal error: requested %1, but not found in data" ) ).arg( file ) );
         return false;
      }

      minx = scale_q[ file ][ 0 ];
      maxx = scale_q[ file ].back();

      miny = scale_I[ file ][ 0 ];
      maxy = scale_I[ file ][ 0 ];
      if ( axis_y_log )
      {
         unsigned int i = 0;
         while ( miny <= 0e0 && i < scale_I[ file ].size() )
         {
            miny = scale_I[ file ][ i ];
            maxy = scale_I[ file ][ i ];
            minx = scale_q[ file ][ i ];
            maxx = scale_q[ file ][ i ];
            i++;
         }
         for ( ; i < scale_I[ file ].size(); i++ )
         {
            if ( miny > scale_I[ file ][ i ] && scale_I[ file ][ i ] > 0e0 )
            {
               miny = scale_I[ file ][ i ];
            }
            if ( maxy < scale_I[ file ][ i ] )
            {
               maxy = scale_I[ file ][ i ];
            }
            if ( maxx < scale_q[ file ][ i ] )
            {
               maxx = scale_q[ file ][ i ];
            }
         }
         if ( miny <= 0e0 )
         {
            miny = 1e0;
         }
         // printf( "miny %g\n", miny );
      } else {
         for ( unsigned int i = 1; i < scale_I[ file ].size(); i++ )
         {
            if ( miny > scale_I[ file ][ i ] )
            {
               miny = scale_I[ file ][ i ];
            }
            if ( maxy < scale_I[ file ][ i ] )
            {
               maxy = scale_I[ file ][ i ];
            }
         }
      }
      // us_qdebug( QString("get min max mode scale %1 x %2:%3 x %4:%5" ).arg( file ).arg( minx ).arg( maxx ).arg( miny ).arg( maxy ) );
   } else {
      if ( !f_qs_string .count( file ) ||
           !f_qs        .count( file ) ||
           !f_Is        .count( file ) ||
           !f_pos       .count( file ) )
      {
         // editor_msg( "red", QString( us_tr( "Internal error: requested %1, but not found in data" ) ).arg( file ) );
         return false;
      }

      minx = f_qs[ file ][ 0 ];
      maxx = f_qs[ file ].back();

      miny = f_Is[ file ][ 0 ];
      maxy = f_Is[ file ][ 0 ];
      if ( axis_y_log )
      {
         unsigned int i = 0;
         while ( miny <= 0e0 && i < f_Is[ file ].size() )
         {
            miny = f_Is[ file ][ i ];
            maxy = f_Is[ file ][ i ];
            minx = f_qs[ file ][ i ];
            maxx = f_qs[ file ][ i ];
            i++;
         }
         for ( ; i < f_Is[ file ].size(); i++ )
         {
            if ( miny > f_Is[ file ][ i ] && f_Is[ file ][ i ] > 0e0 )
            {
               miny = f_Is[ file ][ i ];
            }
            if ( maxy < f_Is[ file ][ i ] )
            {
               maxy = f_Is[ file ][ i ];
            }
            if ( maxx < f_qs[ file ][ i ] )
            {
               maxx = f_qs[ file ][ i ];
            }
         }
         if ( miny <= 0e0 )
         {
            miny = 1e0;
         }
         // printf( "miny %g\n", miny );
      } else {
         for ( unsigned int i = 1; i < f_Is[ file ].size(); i++ )
         {
            if ( miny > f_Is[ file ][ i ] )
            {
               miny = f_Is[ file ][ i ];
            }
            if ( maxy < f_Is[ file ][ i ] )
            {
               maxy = f_Is[ file ][ i ];
            }
         }
      }
   }
   return true;
}

bool US_Hydrodyn_Mals_Saxs::plot_file( QString file,
                                       double &minx,
                                       double &maxx,
                                       double &miny,
                                       double &maxy )
{
   if ( !f_qs_string .count( file ) ||
        !f_qs        .count( file ) ||
        !f_Is        .count( file ) ||
        !f_pos       .count( file ) )
   {
      editor_msg( "red", QString( us_tr( "Internal error: request to plot %1, but not found in data" ) ).arg( file ) );
      return false;
   }

   get_min_max( file, minx, maxx, miny, maxy );

   QwtPlotCurve *curve = new QwtPlotCurve( file );
   plotted_curves[ file ] = curve;
   curve->setStyle( cb_dots->isChecked() ? QwtPlotCurve::Dots : QwtPlotCurve::Lines );

   unsigned int q_points = f_qs[ file ].size();

   bool use_error = cb_eb->isChecked() && ( f_errors[ file ].size() == q_points );
   double x[ 2 ];
   double y[ 2 ];

   set < double > use_q_exclude = q_exclude;
   // only set for non time files
   if ( f_is_time[ file ] ) {
      use_q_exclude.clear();
   }
   
   vector < double > exclude_x;
   vector < double > exclude_y;
   
   if ( use_error )
   {
      QwtSymbol symbol;
      symbol.setStyle( QwtSymbol::Diamond );
      symbol.setSize( 1 + use_line_width * 5 );
      symbol.setBrush( Qt::NoBrush ); // plot_colors[ f_pos[ file ] % plot_colors.size() ] );

      if ( !axis_y_log )
      {
         curve->setSamples(
                        /* cb_guinier->isChecked() ?
                           (double *)&(plotted_q2[p][0]) : */
                        (double *)&( f_qs[ file ][ 0 ] ),
                        (double *)&( f_Is[ file ][ 0 ] ),
                        q_points

                           );

         curve->setPen( QPen( plot_colors[ f_pos[ file ] % plot_colors.size() ], use_line_width, Qt::SolidLine ) );
         curve->setStyle( QwtPlotCurve::NoCurve );
         symbol.setPen  ( QPen( plot_colors[ f_pos[ file ] % plot_colors.size() ], use_line_width, Qt::SolidLine ) );
         curve->setSymbol( new QwtSymbol( symbol.style(), symbol.brush(), symbol.pen(), symbol.size() ) );
         curve->attach( plot_dist );

         for ( unsigned int i = 0; i < q_points; i++ ) {

            QwtPlotCurve *curveeb = new QwtPlotCurve( UPU_EB_PREFIX + file );
            curveeb->setStyle( QwtPlotCurve::Lines );
            x[ 0 ] = f_qs[ file ][ i ];
            x[ 1 ] = x[ 0 ];
            y[ 0 ] = f_Is[ file ][ i ] - f_errors[ file ][ i ];
            y[ 1 ] = f_Is[ file ][ i ] + f_errors[ file ][ i ];

            curveeb->setSamples(
                             (double *)&( x[ 0 ] ),
                             (double *)&( y[ 0 ] ),
                             2
                             );

            curveeb->setPen( QPen( plot_colors[ f_pos[ file ] % plot_colors.size() ], use_line_width, Qt::SolidLine ) );
            curveeb->attach( plot_dist );

            if ( use_q_exclude.count( x[0] ) ) {
               exclude_x.push_back( x[0] );
               exclude_y.push_back( f_Is[ file ][i] );
            }
         }            
      } else {
         vector < double > q;
         vector < double > I;
         vector < double > e;
         for ( unsigned int i = 0; i < q_points; i++ )
         {
            if ( f_Is[ file ][ i ] - f_errors[ file ][ i ] > 0e0 )
            {
               q.push_back( f_qs[ file ][ i ] );
               I.push_back( f_Is[ file ][ i ] );
               e.push_back( f_errors[ file ][ i ] );
               if ( use_q_exclude.count( q.back() ) ) {
                  exclude_x.push_back( q.back() );
                  exclude_y.push_back( I.back() );
               }
            }
         }
         q_points = ( unsigned int )q.size();
         curve->setSamples(
                        /* cb_guinier->isChecked() ?
                           (double *)&(plotted_q2[p][0]) : */
                        (double *)&( q[ 0 ] ),
                        (double *)&( I[ 0 ] ),
                        q_points
                        );

         curve->setPen( QPen( plot_colors[ f_pos[ file ] % plot_colors.size() ], use_line_width, Qt::SolidLine ) );
         curve->setStyle( QwtPlotCurve::NoCurve );
         symbol.setPen  ( QPen( plot_colors[ f_pos[ file ] % plot_colors.size() ], use_line_width, Qt::SolidLine ) );
         // symbol.setBrush( plot_colors[ f_pos[ file ] % plot_colors.size() ] );
         curve->setSymbol( new QwtSymbol( symbol.style(), symbol.brush(), symbol.pen(), symbol.size() ) );
         curve->attach( plot_dist );
         for ( unsigned int i = 0; i < q_points; i++ )
         {
            QwtPlotCurve *curveeb = new QwtPlotCurve( file );
            curveeb->setStyle( QwtPlotCurve::Lines );

            x[ 0 ] = q[ i ];
            x[ 1 ] = x[ 0 ];
            y[ 0 ] = I[ i ] - e[ i ];
            y[ 1 ] = I[ i ] + e[ i ];

            curveeb->setSamples(
                           (double *)&( x[ 0 ] ),
                           (double *)&( y[ 0 ] ),
                           2
                           );

            curveeb->setPen( QPen( plot_colors[ f_pos[ file ] % plot_colors.size() ], use_line_width, Qt::SolidLine ) );
            curveeb->attach( plot_dist );
         }            
      }
   } else {
      if ( !axis_y_log )
      {
         curve->setSamples(
                        /* cb_guinier->isChecked() ?
                           (double *)&(plotted_q2[p][0]) : */
                        (double *)&( f_qs[ file ][ 0 ] ),
                        (double *)&( f_Is[ file ][ 0 ] ),
                        q_points
                        );

         curve->setPen( QPen( plot_colors[ f_pos[ file ] % plot_colors.size() ], use_line_width, Qt::SolidLine ) );
         curve->attach( plot_dist );
         if ( use_q_exclude.size() ) {
            for ( unsigned int i = 0; i < q_points; i++ ) {
               if ( use_q_exclude.count( f_qs[file][i] ) ) {
                  exclude_x.push_back( f_qs[file][i] );
                  exclude_y.push_back( f_Is[file][i] );
               }
            }
         }
      } else {
         vector < double > q;
         vector < double > I;
         for ( unsigned int i = 0; i < q_points; i++ )
         {
            if ( f_Is[ file ][ i ] > 0e0 )
            {
               q.push_back( f_qs[ file ][ i ] );
               I.push_back( f_Is[ file ][ i ] );
               if ( use_q_exclude.count( q.back() ) ) {
                  exclude_x.push_back( q.back() );
                  exclude_y.push_back( I.back() );
               }
            }
         }
         q_points = ( unsigned int )q.size();
         curve->setSamples(
                        /* cb_guinier->isChecked() ?
                           (double *)&(plotted_q2[p][0]) : */
                        (double *)&( q[ 0 ] ),
                        (double *)&( I[ 0 ] ),
                        q_points
                        );

         curve->setPen( QPen( plot_colors[ f_pos[ file ] % plot_colors.size() ], use_line_width, Qt::SolidLine ) );
         curve->attach( plot_dist );
      }
   }            
   // US_Vector::printvector2( QString( "plot_file( %1 ) exclude points" ).arg( file ), exclude_x, exclude_y );

   if ( exclude_x.size() ) {
      QwtPlotCurve *curve = new QwtPlotCurve( file + "-excluded" );
      plotted_curves[ file ] = curve;
      curve->setStyle( QwtPlotCurve::Dots );
      QwtSymbol symbol;
      symbol.setStyle( QwtSymbol::XCross );
      symbol.setSize( 1 + use_line_width * 6 );
      symbol.setPen( QPen( QColor( "red" ), use_line_width, Qt::SolidLine ) );
      // symbol.setBrush( Qt::NoBrush );
      curve->setSamples(
                        (double *)&(exclude_x[0])
                        ,(double *)&(exclude_y[0])
                        ,exclude_x.size()
                        );
      curve->setSymbol( new QwtSymbol( symbol.style(), symbol.brush(), symbol.pen(), symbol.size() ) );
      curve->attach( plot_dist );
   }
   return true;
}

void US_Hydrodyn_Mals_Saxs::rescale()
{
   // qDebug() << "rescale XY event\n";
   //    hide_widgets( plot_errors_widgets, !plot_errors_widgets[ 0 ]->isVisible() );
   //    hide_widgets( files_widgets, !files_widgets[ 0 ]->isVisible() );
   //    hide_widgets( files_expert_widgets, !files_expert_widgets[ 0 ]->isVisible() );
   //    hide_widgets( created_files_widgets, !created_files_widgets[ 0 ]->isVisible() );
   //    hide_widgets( created_files_expert_widgets, !created_files_expert_widgets[ 0 ]->isVisible() );
   //    hide_widgets( editor_widgets, !editor_widgets[ 0 ]->isVisible() );

   bool any_selected = false;
   double minx = 0e0;
   double maxx = 1e0;
   double miny = 0e0;
   double maxy = 1e0;

   double file_minx;
   double file_maxx;
   double file_miny;
   double file_maxy;
   
   bool first = true;
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         any_selected = true;
         if ( get_min_max( lb_files->item( i )->text(), file_minx, file_maxx, file_miny, file_maxy ) )
         {
            if ( first )
            {
               minx = file_minx;
               maxx = file_maxx;
               miny = file_miny;
               maxy = file_maxy;
               first = false;
            } else {
               if ( file_minx < minx )
               {
                  minx = file_minx;
               }
               if ( file_maxx > maxx )
               {
                  maxx = file_maxx;
               }
               if ( file_miny < miny )
               {
                  miny = file_miny;
               }
               if ( file_maxy > maxy )
               {
                  maxy = file_maxy;
               }
            }
         }
      }
   }
   
   // if ( plot_dist_zoomer )
   // {
   //    plot_dist_zoomer->zoom ( 0 );
   //    delete plot_dist_zoomer;
   // }


   if ( any_selected ) {
      plot_dist->setAxisScale( QwtPlot::xBottom, minx, maxx );
      plot_dist->setAxisScale( QwtPlot::yLeft  , miny * 0.9e0 , maxy * 1.1e0 );
      plot_dist_zoomer->setZoomBase();
      plot_errors->setAxisScale( QwtPlot::xBottom, minx, maxx );
      plot_errors_zoomer->setZoomBase();
   }
#if defined( DEBUG_RESCALE )
   {
      QTextStream tso(stdout);
      tso << "---\n";

      tso <<
         QString("").sprintf(
                             "mals_saxs_plots::rescale\n"
                             "\tminx %e maxx %e\n"
                             "\tminy %e maxy %e\n"
                             , minx, maxx
                             , miny, maxy
                             );
      tso << "zoomrect "
          << plot_dist_zoomer->zoomRect().left() << " , "
          << plot_dist_zoomer->zoomRect().right() << " : " 
          << plot_dist_zoomer->zoomRect().bottom() << " , "
          << plot_dist_zoomer->zoomRect().top()
          << "\n"
         ;
      tso << "zoombase "
          << plot_dist_zoomer->zoomBase().left() << " , "
          << plot_dist_zoomer->zoomBase().right() << " : " 
          << plot_dist_zoomer->zoomBase().bottom() << " , "
          << plot_dist_zoomer->zoomBase().top()
          << "\n"
         ;
      tso << "---\n";
   }
#endif

   // plot_dist_zoomer = new ScrollZoomer(plot_dist->canvas());
   // plot_dist_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
   // connect( plot_dist_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_zoomed( const QRectF & ) ) );
   
   legend_set();
   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
   if ( current_mode == MODE_NORMAL )
   // if ( !gaussian_mode &&
   //      !ggaussian_mode && 
   //      !baseline_mode &&
   //      !timeshift_mode )
   {
      update_enables();
   }
}

void US_Hydrodyn_Mals_Saxs::rescale_y()
{
   // qDebug() << "rescale only visible y axis";
   // can not init'd once in gui, as zoomers might be deleted/recreated
   plot_to_zoomer.clear();

   plot_limit_x_range_min.clear();
   plot_limit_x_range_max.clear();
   
   plot_to_zoomer[ plot_dist ]           = plot_dist_zoomer;
   plot_to_zoomer[ plot_errors ]         = plot_errors_zoomer;
   plot_to_zoomer[ guinier_plot    ]     = guinier_plot_zoomer;
   plot_to_zoomer[ guinier_plot_errors ] = guinier_plot_errors_zoomer;
   plot_to_zoomer[ guinier_plot_rg ]     = guinier_plot_rg_zoomer;
   plot_to_zoomer[ guinier_plot_mw ]     = guinier_plot_mw_zoomer;
   plot_to_zoomer[ ggqfit_plot ]         = ggqfit_plot_zoomer;

   if ( current_mode == MODE_GAUSSIAN ||
        current_mode == MODE_GGAUSSIAN ) {
      plot_limit_x_range_min[ plot_errors ] = le_gauss_fit_start->text().toDouble();
      plot_limit_x_range_max[ plot_errors ] = le_gauss_fit_end  ->text().toDouble();
      qDebug() << "gaussian mode replot, limit x range "
               << plot_limit_x_range_min[ plot_errors ] << " , "
               << plot_limit_x_range_max[ plot_errors ] << "\n";
   }      

   US_Plot_Util::rescale(
                         plot_info,
                         plot_to_zoomer,
                         plot_limit_x_range_min,
                         plot_limit_x_range_max
                         );
}

void US_Hydrodyn_Mals_Saxs::rescale_y_plot_errors()
{
   // qDebug() << "rescale_y_plot_errors";
   if ( current_mode == MODE_GAUSSIAN ||
        current_mode == MODE_GGAUSSIAN ) {
      US_Plot_Util::rescale(
                            plot_errors,
                            plot_errors_zoomer,
                            le_gauss_fit_start->text().toDouble(),
                            le_gauss_fit_end->text().toDouble()
                            );
   }
}
