#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_saxs_hplc.h"
#include "../include/us_hydrodyn_saxs_hplc_ciq.h"
#include "../include/us_hydrodyn_saxs_hplc_dctr.h"
#include "../include/us_hydrodyn_saxs_hplc_p3d.h"
#include "../include/us_hydrodyn_saxs_hplc_fit.h"
#include "../include/us_hydrodyn_saxs_hplc_fit_global.h"
#include "../include/us_lm.h"
#ifdef QT4
#include <qwt_scale_engine.h>
#endif

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#define SLASH QDir::separator()
#define Q_VAL_TOL 5e-6
#define UHSH_VAL_DEC 8
#define UHSH_UV_CONC_FACTOR 1e0

void US_Hydrodyn_Saxs_Hplc::update_plot_errors_group()
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
#ifndef QT4
         long curve;
         curve = plot_errors->insertCurve( "base" );
         plot_errors->setCurveStyle( curve, QwtCurve::Lines );
#else
         QwtPlotCurve *curve = new QwtPlotCurve( "base" );
         curve->setStyle( QwtPlotCurve::Lines );
#endif

#ifndef QT4
         plot_errors->setCurvePen( curve, QPen( Qt::green, use_line_width, Qt::SolidLine ) );
         plot_errors->setCurveData( curve,
                                    (double *)&x[ 0 ],
                                    (double *)&y[ 0 ],
                                    x.size()
                                    );
#else
         curve->setPen( QPen( Qt::green, use_line_width, Qt::SolidLine ) );
         curve->setData(
                        (double *)&x[ 0 ],
                        (double *)&y[ 0 ],
                        x.size()
                        );
         curve->attach( plot_errors );
#endif
      }

      {
#ifndef QT4
         long curve;
         curve = plot_errors->insertCurve( "errors" );
         plot_errors->setCurveStyle( curve, QwtCurve::Lines );
#else
         QwtPlotCurve *curve = new QwtPlotCurve( "errors" );
         curve->setStyle( QwtPlotCurve::Lines );
#endif

#ifndef QT4
         plot_errors->setCurvePen( curve, QPen( plot_colors[ f_pos[ unified_ggaussian_files[ j ] ] % plot_colors.size() ], use_line_width, Qt::SolidLine ) );
         plot_errors->setCurveData( curve,
                                    (double *)&x[ 0 ],
                                    (double *)&e[ 0 ],
                                    x.size()
                                    );
         plot_errors->curve( curve )->setStyle( QwtCurve::Sticks );
#else
         curve->setPen( QPen( plot_colors[ f_pos[ unified_ggaussian_files[ j ] ] % plot_colors.size() ], use_line_width, Qt::SolidLine ) );
         curve->setData(
                        (double *)&x[ 0 ],
                        (double *)&e[ 0 ],
                        x.size()
                        );
         curve->setStyle( QwtPlotCurve::Sticks );
         curve->attach( plot_errors );
#endif
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

   if ( !plot_errors_zoomer )
   {
      cout << "upeg: recreating axis\n";
      plot_errors->setAxisTitle(QwtPlot::yLeft, tr( cb_plot_errors_pct->isChecked() ?
                                                    "% difference" :
                                                    ( cb_plot_errors_sd->isChecked() ?
                                                      "delta I(q)/sd" : "delta I(q)" 
                                                      ) ) );

      plot_errors->setAxisScale( QwtPlot::xBottom, minx, maxx );
      plot_errors->setAxisScale( QwtPlot::yLeft  , -maxy * 1.2e0 , maxy * 1.2e0 );

      plot_errors_zoomer = new ScrollZoomer(plot_errors->canvas());
      plot_errors_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
#ifndef QT4
      plot_errors_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
      connect( plot_errors_zoomer, SIGNAL( zoomed( const QwtDoubleRect & ) ), SLOT( plot_zoomed( const QwtDoubleRect & ) ) );
   }

   if ( !suppress_replot )
   {
      plot_errors->replot();
   }
}

void US_Hydrodyn_Saxs_Hplc::update_plot_errors( vector < double > &grid, 
                                                vector < double > &target, 
                                                vector < double > &fit, 
                                                vector < double > &errors )
{
   plot_errors->clear();

   plot_errors_grid   = grid;
   plot_errors_target = target;
   plot_errors_fit    = fit;
   plot_errors_errors = errors;

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

   if ( ggaussian_mode && cb_plot_errors_group->isChecked() )
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
#ifndef QT4
      long curve;
      curve = plot_errors->insertCurve( "base" );
      plot_errors->setCurveStyle( curve, QwtCurve::Lines );
#else
      QwtPlotCurve *curve = new QwtPlotCurve( "base" );
      curve->setStyle( QwtPlotCurve::Lines );
#endif

#ifndef QT4
      plot_errors->setCurvePen( curve, QPen( Qt::green, use_line_width, Qt::SolidLine ) );
      plot_errors->setCurveData( curve,
                                 (double *)&x[ 0 ],
                                 (double *)&y[ 0 ],
                                 x.size()
                                 );
#else
      curve->setPen( QPen( Qt::red, use_line_width, Qt::SolidLine ) );
      curve->setData(
                     (double *)&x[ 0 ],
                     (double *)&y[ 0 ],
                     x.size()
                     );
      curve->attach( plot_errors );
#endif
   }

   {
#ifndef QT4
      long curve;
      curve = plot_errors->insertCurve( "errors" );
      plot_errors->setCurveStyle( curve, QwtCurve::Lines );
#else
      QwtPlotCurve *curve = new QwtPlotCurve( "errors" );
      curve->setStyle( QwtPlotCurve::Lines );
#endif

#ifndef QT4
      plot_errors->setCurvePen( curve, QPen( Qt::red, use_line_width, Qt::SolidLine ) );
      plot_errors->setCurveData( curve,
                                 (double *)&x[ 0 ],
                                 (double *)&e[ 0 ],
                                 x.size()
                                 );
      plot_errors->curve( curve )->setStyle( QwtCurve::Sticks );
#else
      curve->setPen( QPen( Qt::red, use_line_width, Qt::SolidLine ) );
      curve->setData(
                     (double *)&x[ 0 ],
                     (double *)&e[ 0 ],
                     x.size()
                     );
      curve->setStyle( QwtPlotCurve::Sticks );
      curve->attach( plot_errors );
#endif
   }

   if ( !plot_errors_zoomer )
   {
      double maxy = e[ 0 ];

      for ( unsigned int i = 1; i < ( unsigned int )e.size(); i++ )
      {
         if ( maxy < fabs( e[ i ] ) )
         {
            maxy = fabs( e[ i ] );
         }
      }            

      plot_errors->setAxisTitle(QwtPlot::yLeft, tr( cb_plot_errors_pct->isChecked() ?
                                                    "% difference" :
                                                    ( cb_plot_errors_sd->isChecked() ?
                                                      "delta I(q)/sd" : "delta I(q)" 
                                                      ) ) );

      plot_errors->setAxisScale( QwtPlot::xBottom, x[ 0 ], x.back() );
      plot_errors->setAxisScale( QwtPlot::yLeft  , -maxy * 1.2e0 , maxy * 1.2e0 );

      plot_errors_zoomer = new ScrollZoomer(plot_errors->canvas());
      plot_errors_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
#ifndef QT4
      plot_errors_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
      connect( plot_errors_zoomer, SIGNAL( zoomed( const QwtDoubleRect & ) ), SLOT( plot_zoomed( const QwtDoubleRect & ) ) );
   }

   if ( !suppress_replot )
   {
      plot_errors->replot();
   }
}

void US_Hydrodyn_Saxs_Hplc::redo_plot_errors()
{
   plot_errors->setAxisTitle(QwtPlot::yLeft, tr( cb_plot_errors_pct->isChecked() ?
                                                 "%" : "delta I(q)" ) );
   if ( plot_errors_zoomer )
   {
      delete plot_errors_zoomer;
      plot_errors_zoomer = (ScrollZoomer *) 0;
   }
   vector < double > grid   = plot_errors_grid;
   vector < double > fit    = plot_errors_fit;
   vector < double > target = plot_errors_target;
   vector < double > errors = plot_errors_errors;

   update_plot_errors( grid, target, fit, errors );
   if ( ggaussian_mode )
   {
      plot_errors_jump_markers();
   }
}

void US_Hydrodyn_Saxs_Hplc::set_plot_errors_rev()
{

   vector < double > tmp = plot_errors_fit;
   plot_errors_fit       = plot_errors_target;
   plot_errors_target    = tmp;

   redo_plot_errors();
}

void US_Hydrodyn_Saxs_Hplc::set_plot_errors_sd()
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

void US_Hydrodyn_Saxs_Hplc::set_plot_errors_pct()
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

void US_Hydrodyn_Saxs_Hplc::set_plot_errors_group()
{
   redo_plot_errors();
}


void US_Hydrodyn_Saxs_Hplc::plot_errors_jump_markers()
{
   if ( !unified_ggaussian_ok || cb_plot_errors_group->isChecked() )
   {
      return;
   }

   for ( unsigned int i = 0; i < unified_ggaussian_curves; i++ )
   {
#ifndef QT4
      long marker = plot_errors->insertMarker();
      plot_errors->setMarkerLineStyle ( marker, QwtMarker::VLine );
      plot_errors->setMarkerPos       ( marker, unified_ggaussian_jumps[ i ], 0e0 );
      plot_errors->setMarkerLabelAlign( marker, Qt::AlignRight | Qt::AlignTop );
      plot_errors->setMarkerPen       ( marker, QPen( Qt::cyan, 2, DashDotDotLine));
      plot_errors->setMarkerFont      ( marker, QFont("Helvetica", 11, QFont::Bold) );
      plot_errors->setMarkerLabelText ( marker, QString( "%1" ).arg( i + 1 ) ); // unified_ggaussian_files[ i ] );
#else
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
#endif
   }
   if ( !suppress_replot )
   {
      plot_errors->replot();
   }
}

void US_Hydrodyn_Saxs_Hplc::errors()
{
   if ( plot_errors->isVisible() )
   {
      hide_widgets( plot_errors_widgets, true );
   } else {
      if ( ggaussian_mode )
      {
         hide_widgets( plot_errors_widgets, false );
         if ( !unified_ggaussian_use_errors )
         {
            disconnect( cb_plot_errors_sd, SIGNAL( clicked() ), 0, 0 );
            cb_plot_errors_sd->setChecked( false );
            connect( cb_plot_errors_sd, SIGNAL( clicked() ), SLOT( set_plot_errors_sd() ) );
            cb_plot_errors_sd->hide();
         }
      } else {
         if ( gaussian_mode )
         {
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

         } else {
            // compare 2 files
            QStringList files = all_selected_files();
            if ( files.size() > 2 )
            {
               errors_multi_file( files );
            }
            if ( files.size() != 2 )
            {
               editor_msg( "red", tr( "Internal error: plot residuals, not 2 selected files" ) );
               return;
            }
            if ( f_qs[ files[ 0 ] ] != f_qs[ files[ 1 ] ] )
            {
               editor_msg( "red", tr( "Error: Residuals incompatible grids, you could try 'crop common'" ) );
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
                                ( use_errors_1 ? f_errors[ files[ 1 ] ] : errors ) );

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
      }
   }
}

bool US_Hydrodyn_Saxs_Hplc::compatible_grids( QStringList files )
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
   
void US_Hydrodyn_Saxs_Hplc::errors_multi_file( QStringList files )
{
   // check 
   if ( !compatible_grids( files ) )
   {
      editor_msg( "red", tr( "Residuals: curves must be on the same grid, try 'Crop Common' first." ) );
      return;
   }
}
