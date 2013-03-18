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

#define SLASH QDir::separator()
#define Q_VAL_TOL 5e-6
#define UHSH_VAL_DEC 8
#define UHSH_UV_CONC_FACTOR 1e0

// static   void printvector( QString qs, vector < unsigned int > x )
// {
//    cout << QString( "%1: size %2:" ).arg( qs ).arg( x.size() );
//    for ( unsigned int i = 0; i < x.size(); i++ )
//    {
//       cout << QString( " %1" ).arg( x[ i ] );
//    }
//    cout << endl;
// }

static void printvector( QString qs, vector < double > x )
{
   cout << QString( "%1: size %2:" ).arg( qs ).arg( x.size() );
   for ( unsigned int i = 0; i < x.size(); i++ )
   {
      cout << QString( " %1" ).arg( x[ i ], 0, 'g', 8 );
   }
   cout << endl;
}

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
         QwtPlotCurve *curve;
         QwtPlotCurve *curve = new QwtPlotCurve( file );
         curve->setStyle( QwtPlotCurve::Lines );
#endif

#ifndef QT4
         plot_errors->setCurvePen( curve, QPen( Qt::green, 1, Qt::SolidLine ) );
         plot_errors->setCurveData( curve,
                                    (double *)&x[ 0 ],
                                    (double *)&y[ 0 ],
                                    x.size()
                                    );
#else
         curve->setPen( QPen( Qt::green, 1, Qt::SolidLine ) );
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
         QwtPlotCurve *curve;
         QwtPlotCurve *curve = new QwtPlotCurve( file );
         curve->setStyle( QwtPlotCurve::Lines );
#endif

#ifndef QT4
         plot_errors->setCurvePen( curve, QPen( plot_colors[ f_pos[ unified_ggaussian_files[ j ] ] % plot_colors.size() ], 1, Qt::SolidLine ) );
         plot_errors->setCurveData( curve,
                                    (double *)&x[ 0 ],
                                    (double *)&e[ 0 ],
                                    x.size()
                                    );
         plot_errors->curve( curve )->setStyle( QwtCurve::Sticks );
#else
         curve->setPen( QPen( plot_colors[ f_pos[ unified_ggaussian_files[ j ] ] % plot_colors.size() ], 1, Qt::SolidLine ) );
         curre->setData(
                        (double *)&x[ 0 ],
                        (double *)&e[ 0 ],
                        x.size()
                        );
         curve->setStyle( QwtCurve::Sticks );
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

   plot_errors->replot();
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
      QwtPlotCurve *curve;
      QwtPlotCurve *curve = new QwtPlotCurve( file );
      curve->setStyle( QwtPlotCurve::Lines );
#endif

#ifndef QT4
      plot_errors->setCurvePen( curve, QPen( Qt::green, 1, Qt::SolidLine ) );
      plot_errors->setCurveData( curve,
                                 (double *)&x[ 0 ],
                                 (double *)&y[ 0 ],
                                 x.size()
                                 );
#else
      curve->setPen( QPen( Qt::red, 1, Qt::SolidLine ) );
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
      QwtPlotCurve *curve;
      QwtPlotCurve *curve = new QwtPlotCurve( file );
      curve->setStyle( QwtPlotCurve::Lines );
#endif

#ifndef QT4
      plot_errors->setCurvePen( curve, QPen( Qt::red, 1, Qt::SolidLine ) );
      plot_errors->setCurveData( curve,
                                 (double *)&x[ 0 ],
                                 (double *)&e[ 0 ],
                                 x.size()
                                 );
      plot_errors->curve( curve )->setStyle( QwtCurve::Sticks );
#else
      curve->setPen( QPen( Qt::red, 1, Qt::SolidLine ) );
      curre->setData(
                     (double *)&x[ 0 ],
                     (double *)&e[ 0 ],
                     x.size()
                     );
      curve->setStyle( QwtCurve::Sticks );
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

   plot_errors->replot();
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
#warn check how to do this in qt4 needs ymark symsize
      QwtPlotMarker* marker = new QwtPlotMarker;
      marker->setSymbol( QwtSymbol( QwtSymbol::VLine,
                                    QBrush( Qt::white ), QPen( Qt::cyan, 2, Qt::DashLine ),
                                    QSize( 8, sizeym ) ) );
      marker->setValue( unified_ggaussian_jumps[ i ] );
      marker->setLabelAlignment( Qt::AlignRight | Qt::AlignTop );
      marker->setLabel( QString( "%1" ).arg( i + 1 ) ); // unified_ggaussian_files[ i ] );
      marker->attach( plot_errors );
#endif
   }
   plot_errors->replot();
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
            if ( files.size() != 2 )
            {
               editor_msg( "red", tr( "Internal error: plot residuals, not 2 selected files" ) );
               return;
            }
            if ( f_qs[ files[ 0 ] ] != f_qs[ files[ 1 ] ] )
            {
               editor_msg( "red", tr( "Error: Residuals incompatible grids" ) );
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

hplc_stack_data US_Hydrodyn_Saxs_Hplc::current_data( bool selected_only )
{
   hplc_stack_data tmp_stack;
   if ( selected_only )
   {
      QStringList files = all_selected_files();
   
      map < QString, bool > selected;

      for ( unsigned int i = 0; i < ( unsigned int )files.size(); i++ )
      {
         selected[ files[ i ] ] = true;
         tmp_stack.f_qs_string             [ files[ i ] ] = f_qs_string[ files[ i ] ];
         tmp_stack.f_qs                    [ files[ i ] ] = f_qs       [ files[ i ] ];
         tmp_stack.f_Is                    [ files[ i ] ] = f_Is       [ files[ i ] ];
         if ( f_errors.count( files[ i ] ) )
         {
            tmp_stack.f_errors                [ files[ i ] ] = f_errors[ files[ i ] ];
         }
         if ( f_gaussians.count( files[ i ] ) )
         {
            tmp_stack.f_gaussians             [ files[ i ] ] = f_gaussians[ files[ i ] ];
         }
         tmp_stack.f_pos                   [ files[ i ] ] = i;
         tmp_stack.f_name                  [ files[ i ] ] = f_name[ files[ i ] ];
         tmp_stack.f_is_time               [ files[ i ] ] = f_is_time[ files[ i ] ];
         tmp_stack.f_conc                  [ files[ i ] ] = f_conc.count( files[ i ] ) ? f_conc[ files[ i ] ] : 0e0;
         tmp_stack.f_psv                   [ files[ i ] ] = f_psv .count( files[ i ] ) ? f_psv [ files[ i ] ] : 0e0;
         if ( created_files_not_saved.count( files[ i ] ) )
         {
            tmp_stack.created_files_not_saved [ files[ i ] ] = created_files_not_saved[ files[ i ] ];
         }
      }
      tmp_stack.gaussians               = gaussians;

      for ( int i = 0; i < lb_files->numRows(); i++ )
      {
         if ( lb_files->isSelected( i ) )
         {
            tmp_stack.files << lb_files->text( i );
            tmp_stack.selected_files[ lb_files->text( i ) ] = true;
         }
      }
      for ( int i = 0; i < lb_created_files->numRows(); i++ )
      {
         if ( selected.count( lb_created_files->text( i ) ) )
         {
            tmp_stack.created_files << lb_created_files->text( i );
            tmp_stack.created_selected_files[ lb_created_files->text( i ) ] = true;
         }
      }
   } else {
      tmp_stack.f_qs_string             = f_qs_string;
      tmp_stack.f_qs                    = f_qs;
      tmp_stack.f_Is                    = f_Is;
      tmp_stack.f_errors                = f_errors;
      tmp_stack.f_gaussians             = f_gaussians;
      tmp_stack.f_pos                   = f_pos;
      tmp_stack.f_name                  = f_name;
      tmp_stack.f_is_time               = f_is_time;
      tmp_stack.f_psv                   = f_psv;
      tmp_stack.f_conc                  = f_conc;
      tmp_stack.created_files_not_saved = created_files_not_saved;
      tmp_stack.gaussians               = gaussians;

      for ( int i = 0; i < lb_files->numRows(); i++ )
      {
         tmp_stack.files << lb_files->text( i );
         if ( lb_files->isSelected( i ) )
         {
            tmp_stack.selected_files[ lb_files->text( i ) ] = true;
         }
      }
      for ( int i = 0; i < lb_created_files->numRows(); i++ )
      {
         tmp_stack.created_files << lb_created_files->text( i );
         if ( lb_created_files->isSelected( i ) )
         {
            tmp_stack.created_selected_files[ lb_created_files->text( i ) ] = true;
         }
      }
   }
   return tmp_stack;
}

void US_Hydrodyn_Saxs_Hplc::stack_push_all()
{
   disable_all();
   stack_data.push_back( current_data() );
   lbl_stack->setText( QString( tr( "%1" ) ).arg( stack_data.size() ) );
   update_enables();
}
 
void US_Hydrodyn_Saxs_Hplc::stack_push_sel()
{
   disable_all();
   stack_data.push_back( current_data( true ) );
   lbl_stack->setText( QString( tr( "%1" ) ).arg( stack_data.size() ) );
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::set_current_data( hplc_stack_data & tmp_stack )
{
   lb_files->clear();
   lb_created_files->clear();
   f_qs_string             = tmp_stack.f_qs_string;
   f_qs                    = tmp_stack.f_qs;
   f_Is                    = tmp_stack.f_Is;
   f_errors                = tmp_stack.f_errors;
   f_gaussians             = tmp_stack.f_gaussians;
   f_pos                   = tmp_stack.f_pos;
   f_name                  = tmp_stack.f_name;
   f_is_time               = tmp_stack.f_is_time;
   f_psv                   = tmp_stack.f_psv;
   f_conc                  = tmp_stack.f_conc;
   created_files_not_saved = tmp_stack.created_files_not_saved;
   gaussians               = tmp_stack.gaussians;

   lb_files        ->insertStringList( tmp_stack.files );
   lb_created_files->insertStringList( tmp_stack.created_files );

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( tmp_stack.selected_files.count( lb_files->text( i ) ) )
      {
         lb_files->setSelected( i, true );
      }
   }

   for ( int i = 0; i < lb_created_files->numRows(); i++ )
   {
      if ( tmp_stack.created_selected_files.count( lb_created_files->text( i ) ) )
      {
         lb_created_files->setSelected( i, true );
      }
   }
   
}

void US_Hydrodyn_Saxs_Hplc::stack_drop()
{
   QStringList created_not_saved_list;

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( created_files_not_saved.count( lb_files->text( i ) ) )
      {
         created_not_saved_list << lb_files->text( i );
      }
   }

   if ( created_not_saved_list.size() )
   {
      QStringList qsl;
      for ( int i = 0; i < (int)created_not_saved_list.size() && i < 15; i++ )
      {
         qsl << created_not_saved_list[ i ];
      }

      if ( qsl.size() < created_not_saved_list.size() )
      {
         qsl << QString( tr( "... and %1 more not listed" ) ).arg( created_not_saved_list.size() - qsl.size() );
      }

      switch ( QMessageBox::warning(this, 
                                    caption(),
                                    QString( tr( "Please note:\n\n"
                                                 "These files were created but not saved as .dat files:\n"
                                                 "%1\n\n"
                                                 "What would you like to do?\n" ) )
                                    .arg( qsl.join( "\n" ) ),
                                    tr( "&Save them now" ), 
                                    tr( "&Remove them anyway" ), 
                                    tr( "&Quit" ), 
                                    0, // Stop == button 0
                                    0 // Escape == button 0
                                    ) )
      {
      case 0 : // save them now
         // set the ones listed to selected
         if ( !save_files( created_not_saved_list ) )
         {
            return;
         }
         break;
      case 1 : // just ignore them
         break;
      case 2 : // quit
         return;
         break;
      }
   }

   disable_updates = true;
   set_current_data( stack_data.back() );

   stack_data.pop_back();

   disable_updates = false;
   lbl_stack->setText( QString( tr( "%1" ) ).arg( stack_data.size() ) );
   plot_files();
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::stack_rot_up()
{
   vector < hplc_stack_data > new_stack;
   new_stack.push_back( current_data() );
   for ( unsigned int i = 0; i < (unsigned int) stack_data.size() - 1; i++ )
   {
      new_stack.push_back( stack_data[ i ] );
   }

   disable_updates = true;
   set_current_data( stack_data.back() );

   stack_data = new_stack;

   disable_updates = false;
   lbl_stack->setText( QString( tr( "%1" ) ).arg( stack_data.size() ) );
   plot_files();
   update_enables();
      
}

void US_Hydrodyn_Saxs_Hplc::stack_rot_down()
{
   vector < hplc_stack_data > new_stack;
   for ( unsigned int i = 1; i < (unsigned int) stack_data.size(); i++ )
   {
      new_stack.push_back( stack_data[ i ] );
   }
   new_stack.push_back( current_data() );

   disable_updates = true;
   set_current_data( stack_data[ 0 ] );

   stack_data = new_stack;

   disable_updates = false;
   lbl_stack->setText( QString( tr( "%1" ) ).arg( stack_data.size() ) );
   plot_files();
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::stack_swap()
{
   hplc_stack_data tmp_stack = current_data();
   disable_updates = true;
   set_current_data( stack_data.back() );
   stack_data.back() = tmp_stack;
   disable_updates = false;
   lbl_stack->setText( QString( tr( "%1" ) ).arg( stack_data.size() ) );
   plot_files();
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::stack_join()
{
   stack_join( stack_data.back() );
}

void US_Hydrodyn_Saxs_Hplc::stack_join( hplc_stack_data & tmp_stack )
{
   // join all files in stack_data.back() with current
   hplc_stack_data current   = current_data();

   disable_updates = true;

   map < QString, bool > created_files;

   for ( unsigned int i = 0; i < ( unsigned int )tmp_stack.created_files.size(); i++ )
   {
      created_files[ tmp_stack.created_files[ i ] ] = true;
   }

   for ( unsigned int i = 0; i < ( unsigned int )tmp_stack.files.size(); i++ )
   {
      QString name = tmp_stack.files[ i ];
      if ( !f_qs.count( name ) )
      {
         f_name     [ name ] = tmp_stack.f_name[ name ];
         f_pos      [ name ] = f_qs.size();
         f_qs_string[ name ] = tmp_stack.f_qs_string[ name ];
         f_qs       [ name ] = tmp_stack.f_qs       [ name ];
         f_Is       [ name ] = tmp_stack.f_Is       [ name ];
         if ( tmp_stack.f_errors.count( name ) )
         {
            f_errors[ name ] = tmp_stack.f_errors[ name ];
         }
         f_is_time[ name ] = 
            tmp_stack.f_is_time.count( name ) ?
            tmp_stack.f_is_time[ name ] : false;
         f_psv[ name ] = 
            tmp_stack.f_psv.count( name ) ?
            tmp_stack.f_psv[ name ] : 0e0;
         f_conc[ name ] = 
            tmp_stack.f_conc.count( name ) ?
            tmp_stack.f_conc[ name ] : 0e0;
         if ( tmp_stack.created_files_not_saved.count( name ) )
         {
            created_files_not_saved[ name ] = tmp_stack.created_files_not_saved[ name ];
         }
         if ( tmp_stack.f_gaussians.count( name ) )
         {
            f_gaussians[ name ] = tmp_stack.f_gaussians[ name ];
         }
         lb_files->insertItem( name );
         if ( tmp_stack.selected_files.count( name ) )
         {
            lb_files->setSelected( lb_files->numRows() - 1, true );
         }
         if ( created_files.count( name ) )
         {
            lb_created_files->insertItem( name );
            if ( tmp_stack.created_selected_files.count( name ) )
            {
               lb_created_files->setSelected( lb_created_files->numRows() - 1, true );
            }
         }
      }                      
   }   
   disable_updates = false;
   lbl_stack->setText( QString( tr( "%1" ) ).arg( stack_data.size() ) );
   plot_files();
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::stack_copy()
{
   clipboard = current_data( true );
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::stack_pcopy()
{
   hplc_stack_data adds = current_data( true );

   map < QString, bool > created_files;

   for ( unsigned int i = 0; i < ( unsigned int )adds.created_files.size(); i++ )
   {
      created_files[ adds.created_files[ i ] ] = true;
   }

   for ( unsigned int i = 0; i < ( unsigned int )adds.files.size(); i++ )
   {
      QString name = adds.files[ i ];
      if ( !clipboard.f_qs.count( name ) )
      {
         clipboard.f_name     [ name ] = adds.f_name     [ name ];
         clipboard.f_pos      [ name ] = clipboard.f_qs.size();
         clipboard.f_qs_string[ name ] = adds.f_qs_string[ name ];
         clipboard.f_qs       [ name ] = adds.f_qs       [ name ];
         clipboard.f_Is       [ name ] = adds.f_Is       [ name ];
         if ( adds.f_errors.count( name ) )
         {
            clipboard.f_errors[ name ] = adds.f_errors[ name ];
         }
         clipboard.f_is_time[ name ] = 
            adds.f_is_time.count( name ) ?
            adds.f_is_time[ name ] : false;
         clipboard.f_psv[ name ] = 
            adds.f_psv.count( name ) ?
            adds.f_psv[ name ] : 0e0;
         clipboard.f_conc[ name ] = 
            adds.f_conc.count( name ) ?
            adds.f_conc[ name ] : 0e0;

         if ( adds.created_files_not_saved.count( name ) )
         {
            clipboard.created_files_not_saved[ name ] = adds.created_files_not_saved[ name ];
         }
         clipboard.files << name;
         if ( adds.selected_files.count( name ) )
         {
            clipboard.selected_files[ name ] = adds.selected_files[ name ];
         }
         if ( adds.f_gaussians.count( name ) )
         {
            clipboard.f_gaussians[ name ] = adds.f_gaussians[ name ];
         }
         if ( created_files.count( name ) )
         {
            clipboard.created_files << name;
            if ( adds.created_selected_files.count( name ) )
            {
               clipboard.created_selected_files[ name ] = adds.created_selected_files[ name ];
            }
         }
      }                      
   }   

   update_enables();
}


void US_Hydrodyn_Saxs_Hplc::stack_paste()
{
   disable_all();
   stack_join( clipboard );
}

void US_Hydrodyn_Saxs_Hplc::stack_clear()
{
   disable_all();
   clear_files( all_selected_files() );
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::invert_all_created()
{
   disable_all();
   lb_created_files->invertSelection();
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::remove_created()
{
   disable_all();
   QStringList files;
   for (int i = 0; i < (int) lb_created_files->numRows(); i++ )
   {
      if ( lb_created_files->isSelected( i ) )
      {
         files << lb_created_files->text( i );
      }
   }
   clear_files( files );
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::create_i_of_q( QStringList files )
{
   // for each selected file
   // extract q grid from file names
   editor_msg( "dark blue", tr( "Starting: Make I(q)" ) );

   update_csv_conc();

   {
      QStringList tmp_files;
      for ( unsigned int i = 0; i < files.size(); i++ )
      {
         if ( files[ i ] != lbl_conc_file->text() )
         {
            tmp_files << files[ i ];
         }
      }
      files = tmp_files;
   }

   QString head = qstring_common_head( files, true );
   head = head.replace( QRegExp( "__It_q\\d*_$" ), "" );
   head = head.replace( QRegExp( "_q\\d*_$" ), "" );

   if ( !ggaussian_compatible( false ) )
   {
      editor_msg( "red", tr( "NOTICE: Some files selected have Gaussians with varying centers or a different number of Gaussians or centers, Please enter \"Global Gaussians\" with these files selected and then \"Keep\" before pressing \"Make I(q)\"" ) );
      return;
   }

   QRegExp rx_q     ( "_q(\\d+_\\d+)" );
   QRegExp rx_bl    ( "-bl(.\\d*_\\d+(|e.\\d+))-(.\\d*_\\d+(|e.\\d+))s" );

   vector < QString > q_string;
   vector < double  > q;
   vector < double  > bl_slope;
   vector < double  > bl_intercept;

   bool         any_bl = false;
   unsigned int bl_count = 0;

   // get q and bl

   // map: [ timestamp ][ q_value ] = intensity

   map < double, map < double , double > > I_values;
   map < double, map < double , double > > e_values;

   map < double, bool > used_t;
   list < double >      tl;

   map < double, bool > used_q;
   list < double >      ql;

   bool                 use_errors = true;

   disable_all();

   map < QString, bool >    no_errors;
   map < QString, QString > zero_points;
   QStringList              qsl_no_errors;
   QStringList              qsl_zero_points;

   for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
   {
      progress->setProgress( i, files.size() * 2 );
      qApp->processEvents();
      if ( rx_q.search( files[ i ] ) == -1 )
      {
         editor_msg( "red", QString( tr( "Error: Can not find q value in file name for %1" ) ).arg( files[ i ] ) );
         progress->reset();
         update_enables();
         return;
      }
      ql.push_back( rx_q.cap( 1 ).replace( "_", "." ).toDouble() );
      if ( used_q.count( ql.back() ) )
      {
         editor_msg( "red", QString( tr( "Error: Duplicate q value in file name for %1" ) ).arg( files[ i ] ) );
         progress->reset();
         update_enables();
         return;
      }
      used_q[ ql.back() ] = true;
         
      if ( rx_bl.search( files[ i ] ) == -1 )
      {
         bl_slope    .push_back( 0e0 );
         bl_intercept.push_back( 0e0 );
      } else {
         // cout << QString( "bl_cap 1 <%1>\n" ).arg( rx_bl.cap( 1 ) );
         // cout << QString( "bl_cap 2 <%1>\n" ).arg( rx_bl.cap( 3 ) );
         bl_slope    .push_back( rx_bl.cap( 1 ).replace( "_", "." ).toDouble() );
         bl_intercept.push_back( rx_bl.cap( 3 ).replace( "_", "." ).toDouble() );
         cout << QString( "bl for file %1 slope %2 intercept %3\n" ).arg( i ).arg( bl_slope.back(), 0, 'g', 8 ).arg( bl_intercept.back(), 0, 'g', 8 ).ascii();
         bl_count++;
         any_bl = true;
      }

      if ( !f_qs.count( files[ i ] ) )
      {
         editor_msg( "red", QString( tr( "Internal error: request to use %1, but not found in data" ) ).arg( files[ i ] ) );
      } else {
         for ( unsigned int j = 0; j < ( unsigned int ) f_qs[ files[ i ] ].size(); j++ )
         {
            I_values[ f_qs[ files[ i ] ][ j ] ][ ql.back() ] = f_Is[ files[ i ] ][ j ];
            if ( use_errors && f_errors[ files[ i ] ].size() == f_qs[ files[ i ] ].size() )
            {
               e_values[ f_qs[ files[ i ] ][ j ] ][ ql.back() ] = f_errors[ files[ i ] ][ j ];
            } else {
               if ( use_errors )
               {
                  use_errors = false;
                  editor_msg( "dark red", QString( tr( "Notice: missing errors, first noticed in %1, so no errors at all" ) )
                              .arg( files[ i ] ) );
               }
            }
            if ( !used_t.count( f_qs[ files[ i ] ][ j ] ) )
            {
               tl.push_back( f_qs[ files[ i ] ][ j ] );
               used_t[ f_qs[ files[ i ] ][ j ] ] = true;
            }
         }

         if ( !f_errors.count( files[ i ] ) ||
              f_errors[ files[ i ] ].size() != f_Is[ files[ i ] ].size() )
         {
            no_errors[ files[ i ] ] = true;
            qsl_no_errors           << files[ i ];
         } else {
            if ( !is_nonzero_vector( f_errors[ files[ i ] ] ) )
            {
               unsigned int zero_pts = 0;
               for ( unsigned int j = 0; j < ( unsigned int ) f_errors[ files[ i ] ].size(); j++ )
               {
                  if ( isnan( f_errors[ files[ i ] ][ j ] ) || f_errors[ files[ i ] ][ j ] == 0e0 )
                  {
                     zero_pts++;
                  }
               }
               zero_points[ files[ i ] ] = QString( "%1: %2 of %3 points" ).arg( files[ i ] ).arg( zero_pts ).arg( f_errors[ files[ i ] ].size() );
               qsl_zero_points           << zero_points[ files[ i ] ];
            }
         }

      }
   }

   tl.sort();

   vector < double > tv;
   for ( list < double >::iterator it = tl.begin();
         it != tl.end();
         it++ )
   {
      tv.push_back( *it );
   }


   ql.sort();

   vector < double  > qv;
   vector < QString > qv_string;
   for ( list < double >::iterator it = ql.begin();
         it != ql.end();
         it++ )
   {
      qv.push_back( *it );
      qv_string.push_back( QString( "%1" ).arg( *it ) );
   }


   QString qs_no_errors;
   QString qs_zero_points;

   if ( zero_points.size() || no_errors.size() )
   {
      unsigned int used = 0;

      QStringList qsl_list_no_errors;

      for ( unsigned int i = 0; i < ( unsigned int ) qsl_no_errors.size() && i < 12; i++ )
      {
         qsl_list_no_errors << qsl_no_errors[ i ];
         used++;
      }
      if ( qsl_list_no_errors.size() < qsl_no_errors.size() )
      {
         qsl_list_no_errors << QString( tr( "... and %1 more not listed" ) ).arg( qsl_no_errors.size() - qsl_list_no_errors.size() );
      }
      qs_no_errors = qsl_list_no_errors.join( "\n" );
      
      QStringList qsl_list_zero_points;
      for ( unsigned int i = 0; i < ( unsigned int ) qsl_zero_points.size() && i < 24 - used; i++ )
      {
         qsl_list_zero_points << qsl_zero_points[ i ];
      }
      if ( qsl_list_zero_points.size() < qsl_zero_points.size() )
      {
         qsl_list_zero_points << QString( tr( "... and %1 more not listed" ) ).arg( qsl_zero_points.size() - qsl_list_zero_points.size() );
      }
      qs_zero_points = qsl_list_zero_points.join( "\n" );
   }

   bool save_gaussians;

   vector < double > conv;
   vector < double > psv ;
   
   double conc_repeak = 1e0;
   
   bool normalize_by_conc = false;
   bool conc_ok           = false;

   bool sd_avg_local  = false;
   bool sd_drop_zeros = false;
   bool sd_keep_zeros = false;
   bool sd_set_pt1pct = false;
   bool save_sum      = false;

   bool sd_from_difference = false;
   {
      map < QString, QString > parameters;
      bool no_conc = false;
      if ( bl_count )
      {
         parameters[ "baseline" ] = 
            QString( tr( "Add back the baselines when making I(q).  Baselines were found for %1 of the %2 curves" ) )
            .arg( bl_count )
            .arg( files.size() );
      }

      parameters[ "gaussians" ] = QString( "%1" ).arg( f_gaussians[ files[ 0 ] ].size() / 3 );

      bool any_detector = false;
      if ( detector_uv )
      {
         parameters[ "uv" ] = "true";
         any_detector = true;
      } else {
         if ( detector_ri )
         {
            parameters[ "ri" ] = "true";
            any_detector = true;
         }
      }

      if ( lbl_conc_file->text().isEmpty() )
      {
         parameters[ "error" ] = QString( tr( "Concentration controls disabled: no concentration file set" ) );
         no_conc = true;
      } else {
         if ( !f_gaussians.count( lbl_conc_file->text() ) )
         {
            parameters[ "error" ] = QString( tr( "Concentration controls disabled: no Gaussians defined for concentration file" ) );
            no_conc = true;
         } else {
            if ( f_gaussians[ lbl_conc_file->text() ].size() / 3  != f_gaussians[ files[ 0 ] ].size() / 3 )
            {
               parameters[ "error" ] = 
                  QString( tr( "Concentration controls disabled: Concentration file Gaussian count (%1)\n does not match global curves Gaussian count (%2)" ) )
                  .arg( f_gaussians[ lbl_conc_file->text() ].size() )
                  .arg( f_gaussians[ files[ 0 ] ].size() / 3 )
                  ;
               no_conc = true;
            }
         }
      }

      if ( !no_conc )
      {
         QRegExp rx_repeak( "-rp(.\\d*_\\d+(|e.\\d+))" );
         if ( rx_repeak.search( lbl_conc_file->text() ) != -1 )
         {
            conc_repeak = rx_repeak.cap( 1 ).toDouble();
            if ( conc_repeak == 0e0 )
            {
               conc_repeak = 1e0;
               editor_msg( "red", tr( "Error: concentration repeak scaling value extracted is 0, turning off back scaling" ) );
            }
         }
      }
         
      if ( !any_detector )
      {
         if ( parameters.count( "error" ) )
         {
            parameters[ "error" ] += "\nYou must also select a detector type";
         } else {
            parameters[ "error" ] = "\nYou must select a detector type";
         }
      }            

      parameters[ "no_errors"   ] = qs_no_errors;
      parameters[ "zero_points" ] = qs_zero_points;

      //       cout << "parameters b4 ciq:\n";
      //       for ( map < QString, QString >::iterator it = parameters.begin();
      //             it != parameters.end();
      //             it++ )
      //       {
      //          cout << QString( "%1:%2\n" ).arg( it->first ).arg( it->second );
      //       }
      //       cout << "end parameters b4 ciq:\n";

      US_Hydrodyn_Saxs_Hplc_Ciq *hplc_ciq = 
         new US_Hydrodyn_Saxs_Hplc_Ciq(
                                       this,
                                       & parameters,
                                       this );
      US_Hydrodyn::fixWinButtons( hplc_ciq );
      hplc_ciq->exec();
      delete hplc_ciq;
      
      //       cout << "parameters:\n";
      //       for ( map < QString, QString >::iterator it = parameters.begin();
      //             it != parameters.end();
      //             it++ )
      //       {
      //          cout << QString( "%1:%2\n" ).arg( it->first ).arg( it->second );
      //       }
      //       cout << "end parameters:\n";

      if ( bl_count && ( !parameters.count( "add_baseline" ) || parameters[ "add_baseline" ] == "false" ) )
      {
         bl_count = 0;
         cout << "ciq: bl off\n";
      }

      if ( parameters.count( "save_as_pct_iq" ) && parameters[ "save_as_pct_iq" ] == "true" )
      {
         save_gaussians = false;
         cout << "ciq: save_gaussians false\n";
      } else {
         save_gaussians = true;
         cout << "ciq: save_gaussians true\n";
      }
      if ( parameters.count( "save_sum" ) && parameters[ "save_sum" ] == "true" )
      {
         save_sum = true;
      }
      if ( parameters.count( "sd_source" ) && parameters[ "sd_source" ] == "difference" )
      {
         sd_from_difference = true;
         cout << "ciq: sd_from_difference true\n";
      } else {
         sd_from_difference = false;
         cout << "ciq: sd_from_difference false\n";
      }
      if ( !parameters.count( "go" ) )
      {
         progress->reset();
         update_enables();
         return;
      }

      if ( !no_conc )
      {
         for ( unsigned int i = 0; i < ( unsigned int ) f_gaussians[ files[ 0 ] ].size() / 3; i++ )
         {
            conv.push_back( parameters.count( QString( "conv %1" ).arg( i ) ) ?
                            parameters[ QString( "conv %1" ).arg( i ) ].toDouble() : 0e0 );
            psv .push_back( parameters.count( QString( "psv %1" ).arg( i ) ) ?
                            parameters[ QString( "psv %1" ).arg( i ) ].toDouble() : 0e0 );
         }
         conc_ok = true;
         if ( parameters.count( "normalize" ) && parameters[ "normalize" ] == "true" )
         {
            normalize_by_conc = true;
         }
      }

      if ( sd_from_difference )
      {
         sd_avg_local  = parameters[ "sd_zero_avg_local_sd"  ] == "true";
         sd_keep_zeros = parameters[ "sd_zero_keep_as_zeros" ] == "true";
         sd_set_pt1pct = parameters[ "sd_zero_set_to_pt1pct" ] == "true";
      } else {
         sd_drop_zeros = parameters[ "zero_drop_points"   ] == "true";
         sd_avg_local  = parameters[ "zero_avg_local_sd"  ] == "true";
         sd_keep_zeros = parameters[ "zero_keep_as_zeros" ] == "true";
      }

      cout << QString( "sd_avg_local  %1\n"
                       "sd_drop_zeros %2\n"
                       "sd_set_pt1pct %3\n"
                       "sd_keep_zeros %4\n" )
         .arg( sd_avg_local  ? "true" : "false" )
         .arg( sd_drop_zeros ? "true" : "false" )
         .arg( sd_set_pt1pct ? "true" : "false" )
         .arg( sd_keep_zeros ? "true" : "false" )
         ;
   }

   //    if ( bl_count &&
   //         QMessageBox::question(this, 
   //                               this->caption(),
   //                               QString( tr( "Baselines were found for %1 of the %2 curves\n"
   //                                            "Do you want to add back the baselines when making I(q)?" ) )
   //                               .arg( bl_count )
   //                               .arg( files.size() ),
   //                               tr( "&Yes" ),
   //                               tr( "&No" ),
   //                               QString::null,
   //                               0,
   //                               1
   //                               ) == 1 )
   //    {
   //       cout << "not using baselines\n";
   //       bl_count = 0;
   //    }

   //    bool save_gaussians = 
   //         QMessageBox::question(this, 
   //                               this->caption(),
   //                               tr( "Save as Gaussians or a percent of the original I(q)?" ),
   //                               tr( "&Gaussians" ),
   //                               tr( "Percent of &I(q)" ),
   //                               QString::null,
   //                               0,
   //                               1
   //                               ) == 0;
   running = true;

   vector < vector < double > > concs;

   if ( normalize_by_conc || conc_ok )
   {
      // test, produce conc curves for each gaussian
      for ( unsigned int i = 0; i < ( unsigned int )conv.size(); i++ )
      {
         // add_plot( QString( "conc_g_per_ml_peak%1" ).arg( i + 1 ), tv, conc_curve( tv, i, conv[ i ] ), true, false );
         // alt method
         {
            double detector_conv = 0e0;
            if ( detector_uv )
            {
               detector_conv = detector_uv_conv * UHSH_UV_CONC_FACTOR;
            }
            if ( detector_ri )
            {
               detector_conv = detector_ri_conv;
            }
            vector < double > tmp_g(3);
            QString conc_file = lbl_conc_file->text();
            tmp_g[ 0 ] = f_gaussians[ conc_file ][ 0 + i * 3 ] * detector_conv / ( conc_repeak * conv[ i ] );
            tmp_g[ 1 ] = f_gaussians[ conc_file ][ 1 + i * 3 ];
            tmp_g[ 2 ] = f_gaussians[ conc_file ][ 2 + i * 3 ];

            concs.push_back( compute_gaussian( tv, tmp_g ) );
            add_plot( QString( "conc_g_per_ml_peak%1" ).arg( i + 1 ), tv, concs.back(), true, false );
         }
      }
   }

   // now for each I(t) distribute the I for each frame according to the gaussians
   // !!! **              ---- >>>> check for baseline, if present, optionally add back

   // compute all gaussians over q range

   // [ file ][ gaussian ][ time ]
   vector < vector < vector < double > > > fg; // a vector of the individual gaussians
   // [ file ][ time ]
   vector < vector < double > >            fs; // a vector of the gaussian sums
   vector < vector < double > >            g_area;      // a vector of the gaussian area
   vector < double >                       g_area_sum; // a vector of the gaussian area


   for ( unsigned int i = 0; i < files.size(); i++ )
   {
      vector < vector < double > > tmp_v;
      vector < double >            tmp_sum;
      vector < double >            tmp_area;
      double                       tmp_area_sum = 0e0;

      for ( unsigned int j = 0; j < ( unsigned int ) f_gaussians[ files[ i ] ].size(); j += 3 )
      {
         vector < double > tmp_g(3);
         tmp_g[ 0 ] = f_gaussians[ files[ i ] ][ 0 + j ];
         tmp_g[ 1 ] = f_gaussians[ files[ i ] ][ 1 + j ];
         tmp_g[ 2 ] = f_gaussians[ files[ i ] ][ 2 + j ];

         vector < double > tmp = compute_gaussian( tv, tmp_g );
         tmp_v.push_back( tmp );
         if ( j )
         {
            for ( unsigned int k = 0; k < tmp.size(); k++ )
            {
               tmp_sum[ k ] += tmp[ k ];
            }
         } else {
            tmp_sum = tmp;
         }

         tmp_area.push_back( tmp_g[ 0 ] * tmp_g[ 2 ] * M_SQRT2PI );
         tmp_area_sum += tmp_area.back();

         // add_plot( QString( "fg_%1_g%2" ).arg( i ).arg( j / 3 ), tv, tmp, true, false );

      }
      fg.push_back( tmp_v );
      fs.push_back( tmp_sum );

      for ( unsigned int j = 0; j < ( unsigned int ) tmp_area.size(); j++ )
      {
         tmp_area[ j ] /= tmp_area_sum;
      }
         
      g_area    .push_back( tmp_area );
      g_area_sum.push_back( tmp_area_sum );
      printvector( QString( "areas file %1 (sum %2)" ).arg( i ).arg( tmp_area_sum, 0, 'g', 8 ), tmp_area );
      // add_plot( QString( "fg_%1_gsum" ).arg( i ), tv, tmp_sum, true, false );
   }

   printvector( "area sums", g_area_sum );

   // build up resulting curves

   // for each time, tv[ t ] 
   unsigned int num_of_gauss = ( unsigned int ) gaussians.size() / 3;
   // cout << QString( "num of gauss %1\n" ).arg( num_of_gauss );

   bool reported_gs0 = false;

   for ( unsigned int t = 0; t < tv.size(); t++ )
   {
      progress->setProgress( files.size() + t, files.size() + tv.size() );
      // for each gaussian 
      vector < double > gsI;
      vector < double > gse;
      vector < double > gsG;
      // vector < double > gsI_recon;
      // vector < double > gsG_recon;

      // for "sd by differece"
      vector < vector < double > > used_pcts;
      vector < QString >           used_names;

      for ( unsigned int g = 0; g < num_of_gauss; g++ )
      {
         // build up an I(q)
         double conc_factor = 0e0;
         double norm_factor = 1e0;
         QString qs_fwhm;
         if ( ( conc_ok || normalize_by_conc ) && concs[ g ][ t ] > 0e0 )
         {
            conc_factor = concs[ g ][ t ];
            norm_factor = 1e0 / conc_factor;

            double detector_conv = 0e0;
            if ( detector_uv )
            {
               detector_conv = detector_uv_conv * UHSH_UV_CONC_FACTOR;
            }
            if ( detector_ri )
            {
               detector_conv = detector_ri_conv;
            }

            vector < double > tmp_g(3);
            QString conc_file = lbl_conc_file->text();
            tmp_g[ 0 ] = f_gaussians[ conc_file ][ 0 + g * 3 ] * detector_conv / ( conc_repeak * conv[ g ] );
            tmp_g[ 1 ] = f_gaussians[ conc_file ][ 1 + g * 3 ];
            tmp_g[ 2 ] = f_gaussians[ conc_file ][ 2 + g * 3 ];

            double center = tmp_g[ 1 ];
            double width  = tmp_g[ 2 ];
            double fwhm   = 2.354820045e0 * width;
            //             cout << QString( "peak %1 center %2 fwhm %3 t %4 tv[t] %5\n" )
            //                .arg( g + 1 )
            //                .arg( center )
            //                .arg( fwhm )
            //                .arg( t )
            //                .arg( tv[ t ] );
            if ( tv[ t ] >= center - fwhm && tv[ t ] <= center + fwhm )
            {
               qs_fwhm = "_fwhm";
            }
         }

         QString name = head + QString( "%1%2%3_pk%4%5_t%6" )
            .arg( save_gaussians  ? "_G" : "" )
            .arg( any_bl   ? "_bs" : "" )
            .arg( bl_count ? "ba" : "" )
            .arg( g + 1 )
            .arg( normalize_by_conc ? 
                  QString( "%1_cn%2" )
                  .arg( qs_fwhm )
                  .arg( conc_factor, 0, 'g', 6 ) : QString( "" ) )
            .arg( tv[ t ] )
            .replace( ".", "_" )
            ;

         // cout << QString( "name %1\n" ).arg( name );

         // now go through all the files to pick out the I values and errors and distribute amoungst the various gaussian peaks
         // we could also reassemble the original sum of gaussians curves as a comparative

         vector < double > I;
         vector < double > e;
         vector < double > G;

         // vector < double > I_recon;
         // vector < double > G_recon;

         vector < double > this_used_pcts;

         for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
         {
            if ( !I_values.count( tv[ t ] ) )
            {
               editor_msg( "dark red", QString( tr( "Notice: I values missing frame/time = %1" ) ).arg( tv[ t ] ) );
               //                running = false;
               //                update_enables();
               //                return;
               continue;
            }

            if ( !I_values[ tv[ t ] ].count( qv[ i ] ) )
            {
               editor_msg( "red", QString( tr( "Notice: I values missing q = %1" ) ).arg( qv[ i ] ) );
               //                running = false;
               //                update_enables();
               //                return;
               continue;
            }

            double tmp_I       = I_values[ tv[ t ] ][ qv[ i ] ];
            double tmp_e       = 0e0;
            double tmp_G       = fg[ i ][ g ][ t ];

            double frac_of_gaussian_sum;
            if ( fs[ i ][ t ] == 0e0 )
            {
               if ( !reported_gs0 )
               {
                  cout << QString( "Notice: file %1 t %2 gaussian sum is zero (further instances ignored)\n" ).arg( i ).arg( t );
                  reported_gs0 = true;
               }
               frac_of_gaussian_sum = 1e0 / ( double ) num_of_gauss;
            } else {
               frac_of_gaussian_sum = tmp_G / fs[ i ][ t ];
            }

            if ( use_errors )
            {
               if ( !e_values.count( tv[ t ] ) )
               {
                  editor_msg( "red", QString( tr( "Internal error: error values missing t %1" ) ).arg( tv[ t ] ) );
                  running = false;
                  update_enables();
                  progress->reset();
                  return;
               }

               if ( !e_values[ tv[ t ] ].count( qv[ i ] ) )
               {
                  editor_msg( "red", QString( tr( "Internal error: error values missing q %1" ) ).arg( qv[ i ] ) );
                  running = false;
                  update_enables();
                  progress->reset();
                  return;
               }

               tmp_e = e_values[ tv[ t ] ][ qv[ i ] ];
            }
            
            tmp_I *= frac_of_gaussian_sum;
            tmp_e *= frac_of_gaussian_sum;

            if ( sd_from_difference )
            {
               this_used_pcts.push_back( frac_of_gaussian_sum );
            }

            // double tmp_I_recon = tmp_I;
            // double tmp_G_recon = tmp_G;

            if ( bl_count )
            {
               double pct_area = 1e0 / ( double ) num_of_gauss; // g_area[ i ][ g ];
               double ofs = ( bl_intercept[ i ] + tv[ t ] * bl_slope[ i ] ) * pct_area;
               tmp_I += ofs;
               tmp_G += ofs;
               // tmp_I_recon += ofs;
               // tmp_G_recon += ofs;
            }

            I      .push_back( tmp_I );
            e      .push_back( tmp_e );
            G      .push_back( tmp_G );
            // I_recon.push_back( tmp_I_recon );
            // G_recon.push_back( tmp_G_recon );
         } // for each file
         
         if ( g )
         {
            for ( unsigned int m = 0; m < ( unsigned int ) qv.size(); m++ )
            {
               gsI[ m ]       += I[ m ];
               gse[ m ]       += e[ m ];
               gsG[ m ]       += G[ m ];
               // gsI_recon[ m ] += I_recon[ m ];
               // gsG_recon[ m ] += G_recon[ m ];
            }
         } else {
            gsI       = I;
            gsG       = G;
            gse       = e;
            // gsI_recon = I_recon;
            // gsG_recon = G_recon;
         }

         // add to csv conc stuff?

         lb_created_files->insertItem( name );
         lb_created_files->setBottomItem( lb_created_files->numRows() - 1 );
         lb_files->insertItem( name );
         lb_files->setBottomItem( lb_files->numRows() - 1 );
         created_files_not_saved[ name ] = true;
   
         vector < QString > use_qv_string = qv_string;
         vector < double  > use_qv        = qv;
         vector < double  > use_I         = save_gaussians ? G : I;
         vector < double  > use_e         = e;

         if ( conc_ok )
         {
            update_csv_conc();

            for ( unsigned int i = 0; i < csv_conc.data.size(); i++ )
            {
               if ( csv_conc.data[ i ].size() > 1 &&
                    csv_conc.data[ i ][ 0 ] == name )
               {
                  csv_conc.data[ i ][ 1 ] = QString( "%1" ).arg( conc_factor );
               }
            }
            if ( conc_widget )
            {
               conc_window->refresh( csv_conc );
            }
         }


         if ( normalize_by_conc && norm_factor != 1e0 )
         {
            for ( unsigned int i = 0; i < use_I.size(); i++ )
            {
               use_I[ i ] *= norm_factor;
            }
         }

         if ( sd_from_difference )
         {
            used_names.push_back( name );
            used_pcts .push_back( this_used_pcts );
         } else {
            if ( sd_drop_zeros )
            {
               vector < QString > tmp_qv_string;
               vector < double  > tmp_qv       ;
               vector < double  > tmp_I        ; 
               vector < double  > tmp_e        ;
               for ( unsigned int i = 0; i < ( unsigned int ) use_e.size(); i++ )
               {
                  if ( use_e[ i ] > 0e0 )
                  {
                     tmp_qv_string.push_back( use_qv_string[ i ] );
                     tmp_qv       .push_back( use_qv       [ i ] );
                     tmp_I        .push_back( use_I        [ i ] );
                     tmp_e        .push_back( use_e        [ i ] );
                  }
               }
               use_qv_string = tmp_qv_string;
               use_qv        = tmp_qv;
               use_I         = tmp_I;
               use_e         = tmp_e;
            }

            if ( sd_avg_local )
            {
               bool more_zeros;
               unsigned int tries = 0;
               do 
               {
                  more_zeros = false;
                  tries++;
                  for ( unsigned int i = 0; i < ( unsigned int ) use_e.size(); i++ )
                  {
                     if ( use_e[ i ] <= 0e0 )
                     {
                        if ( !i && i < ( unsigned int ) use_e.size() - 1 )
                        {
                           use_e[ i ] = use_e[ i + 1 ];
                           if ( use_e[ i ] <= 0e0 )
                           {
                              more_zeros = true;
                           }
                           continue;
                        }
                        if ( i == ( unsigned int ) use_e.size() - 1 && i > 0 )
                        {
                           use_e[ i ] = use_e[ i - 1 ];
                           if ( use_e[ i ] <= 0e0 )
                           {
                              more_zeros = true;
                           }
                           continue;
                        }
                        if ( i < ( unsigned int ) use_e.size() - 1 && i > 0 )
                        {
                           use_e[ i ] = ( use_e[ i - 1 ] + use_e[ i + 1 ] ) * 5e-1;
                           if ( use_e[ i ] <= 0e0 )
                           {
                              more_zeros = true;
                           }
                           continue;
                        }
                        more_zeros = true;
                     }
                  }
               } while ( more_zeros && tries < 5 );
               if ( more_zeros )
               {
                  editor_msg( "dark red", tr( "Warning: too many adjacent S.D. zeros, could not set all S.D.'s to non-zero values" ) );
               }
            }
            
            if ( sd_set_pt1pct )
            {
               for ( unsigned int i = 0; i < ( unsigned int ) use_e.size(); i++ )
               {
                  if ( use_e[ i ] <= 0e0 )
                  {
                     use_e[ i ] = use_I[ i ] * 0.001;
                  }
               }
            }
         }

         f_pos       [ name ] = f_qs.size();
         f_qs_string [ name ] = use_qv_string;
         f_qs        [ name ] = use_qv;
         f_Is        [ name ] = use_I;
         f_errors    [ name ] = use_e;
         f_is_time   [ name ] = false;
         f_conc      [ name ] = conc_factor;
         f_psv       [ name ] = psv.size() > g ? psv[ g ] : 0e0;

         {
            vector < double > tmp;
            f_gaussians  [ name ] = tmp;
         }
      } // for each gaussian

      if ( sd_from_difference )
      {
         vector < double >  total_e;
         for ( unsigned int i = 0; i < gsG.size(); i++ )
         {
            total_e.push_back( fabs( gsG[ i ] - gsI[ i ] ) );
         }

         // printvector( "total_e", total_e );

         for ( unsigned int i = 0; i < ( unsigned int ) used_names.size(); i++ )
         {
            vector < QString > use_qv_string = f_qs_string[ used_names[ i ] ];
            vector < double >  use_qv        = f_qs       [ used_names[ i ] ];
            vector < double >  use_I         = f_Is       [ used_names[ i ] ];
            vector < double >  use_e         = total_e;
            
            // printvector( QString( "used_pcts for %1" ).arg( used_names[ i ] ), used_pcts[ i ] );

            for ( unsigned int j = 0; j < ( unsigned int ) use_e.size(); j++ )
            {
               use_e[ j ] *= used_pcts[ i ][ j ];
            }

            // printvector( "use_e", use_e );

            if ( sd_drop_zeros )
            {
               vector < QString > tmp_qv_string;
               vector < double  > tmp_qv       ;
               vector < double  > tmp_I        ; 
               vector < double  > tmp_e        ;
               for ( unsigned int i = 0; i < ( unsigned int ) use_e.size(); i++ )
               {
                  if ( use_e[ i ] > 0e0 )
                  {
                     tmp_qv_string.push_back( use_qv_string[ i ] );
                     tmp_qv       .push_back( use_qv       [ i ] );
                     tmp_I        .push_back( use_I        [ i ] );
                     tmp_e        .push_back( use_e        [ i ] );
                  }
               }
               use_qv_string = tmp_qv_string;
               use_qv        = tmp_qv;
               use_I         = tmp_I;
               use_e         = tmp_e;
            }

            if ( sd_avg_local )
            {
               bool more_zeros;
               unsigned int tries = 0;
               do 
               {
                  more_zeros = false;
                  tries++;
                  for ( unsigned int i = 0; i < ( unsigned int ) use_e.size(); i++ )
                  {
                     if ( use_e[ i ] <= 0e0 )
                     {
                        if ( !i && i < ( unsigned int ) use_e.size() - 1 )
                        {
                           use_e[ i ] = use_e[ i + 1 ];
                           if ( use_e[ i ] <= 0e0 )
                           {
                              more_zeros = true;
                           }
                           continue;
                        }
                        if ( i == ( unsigned int ) use_e.size() - 1 && i > 0 )
                        {
                           use_e[ i ] = use_e[ i - 1 ];
                           if ( use_e[ i ] <= 0e0 )
                           {
                              more_zeros = true;
                           }
                           continue;
                        }
                        if ( i < ( unsigned int ) use_e.size() - 1 && i > 0 )
                        {
                           use_e[ i ] = ( use_e[ i - 1 ] + use_e[ i + 1 ] ) * 5e-1;
                           if ( use_e[ i ] <= 0e0 )
                           {
                              more_zeros = true;
                           }
                           continue;
                        }
                        more_zeros = true;
                     }
                  }
               } while ( more_zeros && tries < 5 );
               if ( more_zeros )
               {
                  editor_msg( "dark red", tr( "Warning: too many adjacent S.D. zeros, could not set all S.D.'s to non-zero values" ) );
               }
            }
            
            if ( sd_set_pt1pct )
            {
               for ( unsigned int i = 0; i < ( unsigned int ) use_e.size(); i++ )
               {
                  if ( use_e[ i ] <= 0e0 )
                  {
                     use_e[ i ] = use_I[ i ] * 0.001;
                  }
               }
            }
            
            f_qs_string[ used_names[ i ] ] = use_qv_string;
            f_qs       [ used_names[ i ] ] = use_qv;
            f_Is       [ used_names[ i ] ] = use_I;
            f_errors   [ used_names[ i ] ] = use_e;
         }
      }         
      if ( save_sum )
      {
         if ( save_gaussians )
         {
            add_plot( QString( "sumG_T%1" ).arg( pad_zeros( tv[ t ], (int) tv.size() ) ), qv, gsG, gse, false, false );
         } else {
            add_plot( QString( "sumI_T%1" ).arg( pad_zeros( tv[ t ], (int) tv.size() ) ), qv, gsI, gse, false, false );
         }
      }

      // add_plot( QString( "sumIr_T%1" ).arg( pad_zeros( tv[ t ], (int) tv.size() ) ), qv, gsI_recon, gse, false, false );
      // add_plot( QString( "sumGr_T%1" ).arg( pad_zeros( tv[ t ], (int) tv.size() ) ), qv, gsG_recon, gse, false, false );
   } // for each q value

   editor_msg( "dark blue", tr( "Finished: Make I(q)" ) );
   progress->setProgress( 1, 1 );
   running = false;
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::add()
{
   QStringList files = all_selected_files();

   vector < double > sum = f_Is[ files[ 0 ] ];
   vector < double > e   = f_errors[ files[ 0 ] ];

   bool use_errors;

   if ( f_errors.count( files[ 0 ] ) &&
        f_errors[ files[ 0 ] ].size() == f_qs[ files[ 0 ] ].size() )
   {
      use_errors = true;
      e = f_errors[ files[ 0 ] ];
   } else {
      use_errors = false;
   }

   disable_all();

   QString name = tr( "sum_" ) + files[ 0 ];

   for ( unsigned int i = 1; i < ( unsigned int ) files.size(); i++ )
   {
      if ( f_qs[ files[ 0 ] ] != f_qs[ files[ i ] ] )
      {
         editor_msg( "red", QString( tr( "Error: Residuals incompatible grids (comparing %1 and %2). Suggest: Crop Common" ) ).arg( files[ 0 ] ).arg( files[ i ] ) );
         update_enables();
         return;
      }
      
      name += "+" + files[ i ];

      for ( unsigned int j = 0; j < ( unsigned int ) sum.size(); j++ )
      {
         sum[ j ] += f_Is[ files[ i ] ][ j ];
      }

      if ( !f_errors.count( files[ i ] ) ||
           f_errors[ files[ i ] ].size() != f_qs[ files[ i ] ].size() )
      {
         use_errors = false;
      }
      if ( use_errors )
      {
         for ( unsigned int j = 0; j < ( unsigned int ) sum.size(); j++ )
         {
            e[ j ] += f_errors[ files[ i ] ][ j ];
         }
      }         
   }

   if ( use_errors )
   {
      add_plot( name, f_qs[ files[ 0 ] ], sum, e, f_is_time.count( files[ 0 ] ) ? f_is_time[ files[ 0 ] ] : false, false );
   } else {
      add_plot( name, f_qs[ files[ 0 ] ], sum, f_is_time.count( files[ 0 ] ) ? f_is_time[ files[ 0 ] ] : false, false );
   }
   update_enables();
}

bool US_Hydrodyn_Saxs_Hplc::all_have_f_gaussians( QStringList & files )
{
   for ( unsigned int i = 0; i < ( unsigned int )files.size(); i++ )
   {
      if ( !f_gaussians.count( files[ i ] ) ||
           !f_gaussians[ files[ i ] ].size() )
      {
         return false;
      }
   }
   return true;
}

void US_Hydrodyn_Saxs_Hplc::p3d()
{
   disable_all();
   // this is for global gaussians for now

   QStringList files = all_selected_files();

   if ( !all_have_f_gaussians( files ) )
   {
      editor_msg( "red", tr( "Error: Not all files have Gaussians defined" ) );
      update_enables();
      return;
   }
   if ( !ggaussian_compatible( files, false ) )
   {
      editor_msg( "dark red", 
                  cb_fix_width->isChecked() ?
                  tr( "NOTICE: Some files selected have Gaussians with varying centers or widths or a different number of Gaussians or centers or widths that do not match the last Gaussians" )
                  :
                  tr( "NOTICE: Some files selected have Gaussians with varying centers or a different number of Gaussians or centers that do not match the last Gaussians." ) 
                  );
      update_enables();
      return;
   }

   for ( unsigned int i = 1; i < ( unsigned int ) files.size(); i++ )
   {
      if ( f_qs[ files[ 0 ] ] != f_qs[ files[ i ] ] )
      {
         editor_msg( "red", QString( tr( "Error: Incompatible grids (comparing %1 and %2). Suggest: Crop Common" ) ).arg( files[ 0 ] ).arg( files[ i ] ) );
         update_enables();
         return;
      }
   }

   // get q range
   vector < double  > q;
   {
      list < double >      ql;
      map < double, bool > used_q;
      QRegExp rx_q( "_q(\\d+_\\d+)" );
      for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
      {
         if ( rx_q.search( files[ i ] ) == -1 )
         {
            editor_msg( "red", QString( tr( "Error: Can not find q value in file name for %1" ) ).arg( files[ i ] ) );
            update_enables();
            return;
         }
         ql.push_back( rx_q.cap( 1 ).replace( "_", "." ).toDouble() );

         if ( used_q.count( ql.back() ) )
         {
            editor_msg( "red", QString( tr( "Error: Duplicate q value in file name for %1" ) ).arg( files[ i ] ) );
            update_enables();
            return;
         }
         used_q[ ql.back() ] = true;
      }         
      ql.sort();

      for ( list < double >::iterator it = ql.begin();
            it != ql.end();
            it++ )
      {
         q.push_back( *it );
      }
   }

   map < unsigned int, bool > g_to_plot;

   QString title = caption() + ": Gaussians :";

   {
      map < QString, QString > parameters;
      parameters[ "gaussians" ] = QString( "%1" ).arg( f_gaussians[ files[ 0 ] ].size() / 3 );

      US_Hydrodyn_Saxs_Hplc_P3d *hplc_p3d = 
         new US_Hydrodyn_Saxs_Hplc_P3d(
                                       this,
                                       & parameters,
                                       this );
      US_Hydrodyn::fixWinButtons( hplc_p3d );
      hplc_p3d->exec();
      delete hplc_p3d;

      if ( !parameters.count( "plot" ) )
      {
         update_enables();
         return;
      }

      for ( unsigned int i = 0; i < parameters[ "gaussians" ].toUInt(); i++ )
      {
         if ( parameters.count( QString( "%1" ).arg( i ) ) )
         {
            g_to_plot[ i ] = true;
            title += QString( " %1" ).arg( i + 1 );
         }
      }
   }

   if ( !g_to_plot.size() )
   {
      editor_msg( "dark red", tr( "Plot 3D: no Gaussians selected to plot" ) );
      update_enables();
      return;
   }

   // compute partial (selected) gaussians sums:

   // compute all gaussians over q range

   // vector < vector < vector < double > > > fg; // a vector of the individual gaussians
   vector < vector < double > >            fs; // a vector of the gaussian sums
   vector < vector < double > >            g_area;      // a vector of the gaussian area
   vector < double >                       g_area_sum; // a vector of the gaussian area

   for ( unsigned int i = 0; i < files.size(); i++ )
   {
      vector < vector < double > > tmp_v;
      vector < double >            tmp_sum;
      vector < double >            tmp_area;
      double                       tmp_area_sum = 0e0;

      bool any_accumulated        = false;

      for ( unsigned int j = 0; j < ( unsigned int ) f_gaussians[ files[ i ] ].size(); j += 3 )
      {
         if ( g_to_plot.count( j / 3 ) )
         {
            vector < double > tmp_g(3);
            tmp_g[ 0 ] = f_gaussians[ files[ i ] ][ 0 + j ];
            tmp_g[ 1 ] = f_gaussians[ files[ i ] ][ 1 + j ];
            tmp_g[ 2 ] = f_gaussians[ files[ i ] ][ 2 + j ];

            vector < double > tmp = compute_gaussian( f_qs[ files[ i ] ], tmp_g );
            tmp_v.push_back( tmp );
            if ( any_accumulated )
            {
               for ( unsigned int k = 0; k < tmp.size(); k++ )
               {
                  tmp_sum[ k ] += tmp[ k ];
               }
            } else {
               any_accumulated = true;
               tmp_sum = tmp;
            }

            tmp_area.push_back( tmp_g[ 0 ] * tmp_g[ 2 ] * M_SQRT2PI );
            tmp_area_sum += tmp_area.back();
         }
      }
      // fg.push_back( tmp_v );
      fs.push_back( tmp_sum );

      for ( unsigned int j = 0; j < ( unsigned int ) tmp_area.size(); j++ )
      {
         tmp_area[ j ] /= tmp_area_sum;
      }
         
      g_area    .push_back( tmp_area );
      g_area_sum.push_back( tmp_area_sum );
   }

   // plot 3d
   {
      QString xtitle;
      QString ytitle;
      QString ztitle;

      if ( !QGLFormat::hasOpenGL() )
      {
         editor_msg( "red", tr( "This system has no OpenGL support." ) );
         update_enables();
         return;
      }

      double **data3d;

      unsigned int rows    = ( unsigned int ) fs.size();
      unsigned int columns = ( unsigned int ) fs[ 0 ].size();

      data3d = new double * [rows];

      for ( unsigned int i = 0; i < rows; i++ )
      {
         data3d[ i ] = new double [ columns ];
      }

      double maxI = 0e0;
      for ( unsigned int i = 0; i < rows; i++ )
      {
         // cout << QString( "row %1:" ).arg( i );
         for ( unsigned int j = 0; j < columns; j++ )
         {
            // cout << QString( "%1 " ).arg( data3d[ i ][ j ] );
            if ( maxI < fs[ i ][ j ] )
            {
               maxI = fs[ i ][ j ];
            }
         }
         // cout << endl;
      }

      for ( unsigned int i = 0; i < rows; i++ )
      {
         // cout << QString( "row %1:" ).arg( i );
         for ( unsigned int j = 0; j < columns; j++ )
         {
            data3d[ i ][ j ] = fs[ i ][ j ] / maxI;
            // cout << QString( "%1 " ).arg( data3d[ i ][ j ] );
            if ( maxI < fs[ i ][ j ] )
            {
               maxI = fs[ i ][ j ];
            }
         }
         // cout << endl;
      }

      SA2d_control_variables controlvar_3d;

      controlvar_3d.minx = q[ 0 ];
      controlvar_3d.maxx = q.back();

      controlvar_3d.miny = f_qs[ files[ 0 ] ][ 0 ];
      controlvar_3d.maxy = f_qs[ files[ 0 ] ].back();

      controlvar_3d.xscaling = 1e0 / ( q.back() - q[ 0 ] );
      controlvar_3d.yscaling = 10e0 / ( f_qs[ files[ 0 ] ].back() - f_qs[ files[ 0 ] ][ 0 ] );
      controlvar_3d.zscaling = 1000e0 / maxI;

      controlvar_3d.minx = 0e0;  // f_qs[ files[ 0 ] ][ 0 ];
      controlvar_3d.maxx = 10e0; // f_qs[ files[ 0 ] ].back();
      controlvar_3d.miny = 0;
      controlvar_3d.maxy = 10e0;
      controlvar_3d.xscaling = 1e0;
      controlvar_3d.yscaling = 1e0;
      controlvar_3d.zscaling = 10e0; // 10e0 / maxI > 1e0 ? 10e0 / maxI : 1e0;

      cout << QString( "3d params t %1,%2 scaling %3\n"
                       "          q %4,%5 scaling %6\n"
                       "       maxI %7    scaling %8\n" )
         .arg( controlvar_3d.minx ).arg( controlvar_3d.maxx ).arg( controlvar_3d.xscaling )
         .arg( controlvar_3d.miny ).arg( controlvar_3d.maxy ).arg( controlvar_3d.yscaling )
         .arg( maxI ).arg( controlvar_3d.zscaling );

      controlvar_3d.meshx = rows;
      controlvar_3d.meshy = columns;

      xtitle = "q (relative)";
      ytitle = "Frame (relative)";
      ztitle = "I(q)";

      bool raise = plot3d_flag;
      if ( plot3d_flag )
      {
         plot3d_window->setParameters( title, xtitle, ytitle, ztitle, data3d, &controlvar_3d );
      }
      else
      {
         plot3d_window = new Mesh2MainWindow( &plot3d_flag, title, xtitle, ytitle, ztitle, data3d, &controlvar_3d );
      }

      plot3d_window->dataWidget->coordinates()->setStandardScale();
      plot3d_window->dataWidget->updateGL();

      if ( raise )
      {
         plot3d_window->raise();
      } else {
         plot3d_window->show();
      }

      for ( unsigned int i = 0; i < rows; i++ )
      {
         delete [] data3d[i];
      }
      delete [] data3d;
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::set_detector()
{
   disable_all();

   {
      map < QString, QString > parameters;
      parameters[ "uv_conv" ] = QString( "%1" ).arg( detector_uv_conv, 0, 'g', 8 );
      parameters[ "ri_conv" ] = QString( "%1" ).arg( detector_ri_conv, 0, 'g', 8 );
      if ( detector_uv )
      {
         parameters[ "uv" ] = "true";
      } else {
         if ( detector_ri )
         {
            parameters[ "ri" ] = "true";
         }
      }

      US_Hydrodyn_Saxs_Hplc_Dctr *hplc_dctr = 
         new US_Hydrodyn_Saxs_Hplc_Dctr(
                                        this,
                                        & parameters,
                                        this );
      US_Hydrodyn::fixWinButtons( hplc_dctr );
      hplc_dctr->exec();
      delete hplc_dctr;

      if ( !parameters.count( "keep" ) )
      {
         update_enables();
         return;
      }

      detector_uv      = ( parameters.count( "uv" ) && parameters[ "uv" ] == "true" ) ? true : false;
      detector_ri      = ( parameters.count( "ri" ) && parameters[ "ri" ] == "true" ) ? true : false;
      detector_ri_conv = parameters[ "ri_conv" ].toDouble();
      detector_uv_conv = parameters[ "uv_conv" ].toDouble();
   }
   update_enables();
}


/*

   for ( int i = 0; i < (int)files.size(); i++ )
   {
      QString this_file = files[ i ];
      selected_map[ this_file ] = true;
      if ( created_files_not_saved.count( this_file ) )
      {
         created_not_saved_list << this_file;
         created_not_saved_map[ this_file ] = true;
      }
   }

   if ( created_not_saved_list.size() )
   {
      QStringList qsl;
      for ( int i = 0; i < (int)created_not_saved_list.size() && i < 15; i++ )
      {
         qsl << created_not_saved_list[ i ];
      }

      if ( qsl.size() < created_not_saved_list.size() )
      {
         qsl << QString( tr( "... and %1 more not listed" ) ).arg( created_not_saved_list.size() - qsl.size() );
      }

      switch ( QMessageBox::warning(this, 
                                    tr( "US-SOMO: SAXS Hplc Remove Files" ),
                                    QString( tr( "Please note:\n\n"
                                                 "These files were created but not saved as .dat files:\n"
                                                 "%1\n\n"
                                                 "What would you like to do?\n" ) )
                                    .arg( qsl.join( "\n" ) ),
                                    tr( "&Save them now" ), 
                                    tr( "&Remove them anyway" ), 
                                    tr( "&Quit from removing files" ), 
                                    0, // Stop == button 0
                                    0 // Escape == button 0
                                    ) )
      {
      case 0 : // save them now
         // set the ones listed to selected
         if ( !save_files( created_not_saved_list ) )
         {
            return;
         }
      case 1 : // just remove them
         break;
      case 2 : // quit
         disable_updates = false;
         return;
         break;
      }

   }
*/

bool US_Hydrodyn_Saxs_Hplc::ggauss_recompute()
{
   unified_ggaussian_q               .clear();
   unified_ggaussian_jumps           .clear();
   unified_ggaussian_I               .clear();
   unified_ggaussian_e               .clear();
   unified_ggaussian_t               .clear();
   unified_ggaussian_param_index     .clear();
   unified_ggaussian_q_start         .clear();
   unified_ggaussian_q_end           .clear();
   
   double q_start = le_gauss_fit_start->text().toDouble();
   double q_end   = le_gauss_fit_end  ->text().toDouble();

   unified_ggaussian_jumps  .push_back( 0e0 );

   bool error_msg = false;

   for ( unsigned int i = 0; i < ( unsigned int ) unified_ggaussian_files.size(); i++ )
   {
      if ( i )
      {
         unified_ggaussian_jumps.push_back( unified_ggaussian_t.back() );
      }
      if ( !f_qs.count( unified_ggaussian_files[ i ] ) ||
           f_qs[ unified_ggaussian_files[ i ] ].size() < 2 )
      {
         editor_msg( "red", QString( tr( "Internal error: %1 has no or empty or insufficient data" ) ).arg( unified_ggaussian_files[ i ] ) );
         return false;
      }

      if ( unified_ggaussian_use_errors && 
           ( !f_errors.count( unified_ggaussian_files[ i ] ) ||
             f_errors[ unified_ggaussian_files[ i ] ].size() != f_qs[ unified_ggaussian_files[ i ] ].size() ) )
      {
         editor_msg( "dark red", QString( tr( "WARNING: %1 has no errors so global errors are off for computing RMSD and global fitting" ) ).arg( unified_ggaussian_files[ i ] ) );
         error_msg = true;
         unified_ggaussian_use_errors = false;
         unified_ggaussian_e.clear();
      }

      unified_ggaussian_q_start.push_back( unified_ggaussian_t.size() );
      for ( unsigned int j = 0; j < ( unsigned int ) f_qs[ unified_ggaussian_files[ i ] ].size(); j++ )
      {
         if ( f_qs[ unified_ggaussian_files[ i ] ][ j ] >= q_start &&
              f_qs[ unified_ggaussian_files[ i ] ][ j ] <= q_end )
         {
            if ( unified_ggaussian_use_errors && unified_ggaussian_errors_skip &&
                 f_errors[ unified_ggaussian_files[ i ] ][ j ] <= 0e0 )
            {
               continue;
            }
                 
            unified_ggaussian_t           .push_back( unified_ggaussian_t.size() );
            if ( cb_fix_width->isChecked() )
            {
               unified_ggaussian_param_index .push_back( unified_ggaussian_gaussians_size * ( 2 + i ) );
            } else {
               unified_ggaussian_param_index .push_back( unified_ggaussian_gaussians_size * ( 1 + i * 2 ) );
            }               
            unified_ggaussian_q           .push_back( f_qs[ unified_ggaussian_files[ i ] ][ j ] );
            unified_ggaussian_I           .push_back( f_Is[ unified_ggaussian_files[ i ] ][ j ] );
            if ( unified_ggaussian_use_errors )
            {
               unified_ggaussian_e        .push_back( f_errors[ unified_ggaussian_files[ i ] ][ j ] );
            }
         }
      }
      unified_ggaussian_q_end.push_back( unified_ggaussian_t.size() );
   }

   if ( !is_nonzero_vector( unified_ggaussian_e ) )
   {
      unified_ggaussian_use_errors = false;
      if ( !error_msg )
      {
         editor_msg( "dark red", tr( "WARNING: some errors are zero so global errors are off for computing RMSD and global fitting" ) );
      }
   }

   //    printvector( "q_start", unified_ggaussian_q_start );
   //    printvector( "q_end"  , unified_ggaussian_q_end   );

   pb_ggauss_rmsd->setEnabled( false );
   return true;
}

bool US_Hydrodyn_Saxs_Hplc::create_unified_ggaussian_target( bool do_init )
{
   QStringList files = all_selected_files();
   return create_unified_ggaussian_target( files, do_init );
}

bool US_Hydrodyn_Saxs_Hplc::create_unified_ggaussian_target( QStringList & files, bool do_init )
{
   unified_ggaussian_ok = false;

   org_gaussians = gaussians;
   // printvector( "cugt: org_gauss", org_gaussians );

   unified_ggaussian_params          .clear();

   unified_ggaussian_files           = files;
   unified_ggaussian_curves          = files.size();

   unified_ggaussian_use_errors      = true;

   // for testing
   // unified_ggaussian_use_errors      = false;

   unified_ggaussian_gaussians_size  = ( unsigned int ) gaussians.size() / 3;

   if ( do_init )
   {
      unified_ggaussian_errors_skip = false;
      if ( !initial_ggaussian_fit( files ) )
      {
         progress->reset();
         return false;
      }
   }

   // push back centers first
   for ( unsigned int i = 0; i < ( unsigned int ) gaussians.size(); i += 3 )
   {
      unified_ggaussian_params.push_back( gaussians[ 1 + i ] );
      if ( cb_fix_width->isChecked() )
      {
         unified_ggaussian_params.push_back( gaussians[ 2 + i ] );
      }
   }

   // now push back all the file specific amplitude & widths

   map < QString, bool >    no_errors;
   map < QString, QString > zero_points;
   QStringList              qsl_no_errors;
   QStringList              qsl_zero_points;

   for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
   {
      if ( !f_gaussians.count( files[ i ] ) )
      {
         editor_msg( "red", QString( tr( "Internal error: %1 does not have a gaussian set" ) ).arg( files[ i ] ) );
         return false;
      }
      if ( f_gaussians[ files[ i ] ].size() != gaussians.size() )
      {
         editor_msg( "red", QString( tr( "Internal error: %1 has an incompatible gaussian set" ) ).arg( files[ i ] ) );
         return false;
      }
      
      for ( unsigned int j = 0; j < ( unsigned int ) f_gaussians[ files[ i ] ].size(); j += 3 )
      {
         unified_ggaussian_params.push_back( f_gaussians[ files[ i ] ][ 0 + j ] ); // height
         if ( !cb_fix_width->isChecked() )
         {
            unified_ggaussian_params.push_back( f_gaussians[ files[ i ] ][ 2 + j ] ); // width
         }
      }

      if ( cb_sd_weight->isChecked() )
      {
         if ( !f_errors.count( files[ i ] ) ||
              f_errors[ files[ i ] ].size() != f_Is[ files[ i ] ].size() )
         {
            cout << QString( "file %1 no errors %2\n" )
               .arg( files[ i ] ).arg( f_errors.count( files[ i ] ) ?
                                       QString( "errors %1 vs Is %2" )
                                       .arg( f_errors[ files[ i ] ].size() )
                                       .arg( f_Is[ files[ i ] ].size() )
                                       :
                                       "at all" );
            no_errors[ files[ i ] ] = true;
            qsl_no_errors           << files[ i ];
         } else {
            if ( !is_nonzero_vector( f_errors[ files[ i ] ] ) )
            {
               unsigned int zero_pts = 0;
               for ( unsigned int j = 0; j < ( unsigned int ) f_errors[ files[ i ] ].size(); j++ )
               {
                  if ( isnan( f_errors[ files[ i ] ][ j ] ) || f_errors[ files[ i ] ][ j ] == 0e0 )
                  {
                     zero_pts++;
                  }
               }
               zero_points[ files[ i ] ] = QString( "%1: %2 of %3 points" ).arg( files[ i ] ).arg( zero_pts ).arg( f_errors[ files[ i ] ].size() );
               qsl_zero_points           << zero_points[ files[ i ] ];
            }
         }
      }
   }

   if ( zero_points.size() || no_errors.size() )
   {
      unsigned int used = 0;

      QStringList qsl_list_no_errors;

      for ( unsigned int i = 0; i < ( unsigned int ) qsl_no_errors.size() && i < 12; i++ )
      {
         qsl_list_no_errors << qsl_no_errors[ i ];
         used++;
      }
      if ( qsl_list_no_errors.size() < qsl_no_errors.size() )
      {
         qsl_list_no_errors << QString( tr( "... and %1 more not listed" ) ).arg( qsl_no_errors.size() - qsl_list_no_errors.size() );
      }

      QStringList qsl_list_zero_points;
      for ( unsigned int i = 0; i < ( unsigned int ) qsl_zero_points.size() && i < 24 - used; i++ )
      {
         qsl_list_zero_points << qsl_zero_points[ i ];
      }
      if ( qsl_list_zero_points.size() < qsl_zero_points.size() )
      {
         qsl_list_zero_points << QString( tr( "... and %1 more not listed" ) ).arg( qsl_zero_points.size() - qsl_list_zero_points.size() );
      }

      switch ( QMessageBox::warning(this, 
                                    caption() + tr( ": Create unified global Gaussians" ),
                                    QString( tr( "Please note:\n\n"
                                                 "%1"
                                                 "%2"
                                                 "What would you like to do?\n" ) )
                                    .arg( qsl_list_no_errors.size() ?
                                          QString( tr( "These files have no associated errors:\n%1\n\n" ) ).arg( qsl_list_no_errors.join( "\n" ) ) : "" )
                                    .arg( qsl_list_zero_points.size() ?
                                          QString( tr( "These files have zero points:\n%1\n\n" ) ).arg( qsl_list_zero_points.join( "\n" ) ) : "" )
                                    ,
                                    tr( "&Turn off SD weighting" ), 
                                    tr( "Drop &full curves with zero SDs" ), 
                                    qsl_zero_points.size() ? tr( "Drop &points with zero SDs" ) : QString::null, 
                                    0, // Stop == button 0
                                    0 // Escape == button 0
                                    ) )
      {
      case 0 : // turn off sd weighting
         {
            cb_sd_weight->setChecked( false );
            return create_unified_ggaussian_target( files, false );
         }
         break;
      case 1 : // drop zero sd curves
         {
            running = true;
            disable_updates = true;
            for ( int i = 0; i < lb_files->numRows(); i++ )
            {
               if ( zero_points.count( lb_files->text( i ) ) ||
                    no_errors  .count( lb_files->text( i ) ) )
               {
                  lb_files->setSelected( i, false );
               }
            }
            disable_updates = false;
            running = false;
            update_enables();
            disable_all();
            plot_files();
            QStringList files = all_selected_files();
            return create_unified_ggaussian_target( files, false );
         }
         break;
      case 2 : // drop zero sd points
         unified_ggaussian_errors_skip = true;
         break;
      }
   }

   progress->setProgress( unified_ggaussian_curves * 1.1, unified_ggaussian_curves * 1.2 );
   qApp->processEvents();
   if ( !ggauss_recompute() )
   {
      progress->reset();
      return false;
   }

   //    printvector( "unified q:", unified_ggaussian_q );
   //    printvector( "unified t:", unified_ggaussian_t );
   //    printvector( "unified I:", unified_ggaussian_I );
   // printvector( "unified params:", unified_ggaussian_params );
   // printvector( "unified param index:", unified_ggaussian_param_index );

   unified_ggaussian_ok = true;
   progress->setProgress( 1, 1 );
   return true;
}

bool US_Hydrodyn_Saxs_Hplc::opt_repeak_gaussians( QString file )
{
   if ( !gaussians.size() )
   {
      return false;
   } 
      
   double peak;
   if ( !get_peak( file, peak ) )
   {
      return false;
   }
   
   double gmax = compute_gaussian_peak( file, gaussians );
   
   double scale = peak / gmax;   

   if ( scale < .5 || scale > 1.5 )
   {
      switch ( QMessageBox::warning(this, 
                                    caption(),
                                    QString( tr( "Please note:\n\n"
                                                 "The current Gaussians should be scaled by %1 to be in the range of this curve.\n"
                                                 "What would you like to do?\n" ) ).arg( scale ),
                                    tr( "&Rescale the Gaussian amplitudes" ), 
                                    tr( "&Do not rescale" ),
                                    QString::null,
                                    0, // Stop == button 0
                                    0 // Escape == button 0
                                    ) )
      {
      case 0 : // rescale the Gaussians
         for ( unsigned int i = 0; i < ( unsigned int ) gaussians.size(); i += 3 )
         {
            gaussians[ 0 + i ] *= scale;
         }
         return true;
         break;
      case 1 : // just ignore them
         return false;
         break;
      }
   }
      
   return false;
}

vector < double > US_Hydrodyn_Saxs_Hplc::conc_curve( vector < double > &t,
                                                     unsigned int peak,
                                                     double conv
                                                     )
{
   vector < double > result;
   QString conc_file = lbl_conc_file->text();
   if ( conc_file.isEmpty() )
   {
      editor_msg( "red", tr( "Internal error: conc_curve(): no concentration file set" ) );
      return result;
   } else {
      if ( !f_gaussians.count( conc_file ) )
      {
         editor_msg( "red", tr( "Internal error: conc_curve(): no Gaussians defined for concentration file" ) );
         return result;
      }
   }

   if ( peak >= ( unsigned int ) f_gaussians[ conc_file ].size() / 3 )
   {
      editor_msg( "red", QString( tr( "Internal error: conc_curve(): Gaussian requested (%1) exceedes available (%2)" ) )
                  .arg( peak + 1 )
                  .arg( f_gaussians[ conc_file ].size() / 3 ) );
      return result;
   }

   if ( !detector_uv && !detector_ri )
   {
      editor_msg( "red", tr( "Internal error: conc_curve(): No detector type set" ) );
      return result;
   }
      
   double detector_conv = 0e0;
   if ( detector_uv )
   {
      detector_conv = detector_uv_conv * UHSH_UV_CONC_FACTOR;
   }
   if ( detector_ri )
   {
      detector_conv = detector_ri_conv;
   }
   // printvector( QString( "conc_curve peak %1 conv %2 detector_conv %3" ).arg( peak + 1 ).arg( conv ).arg( detector_conv ), t );

   vector < double > tmp_g(3);
   tmp_g[ 0 ] = f_gaussians[ conc_file ][ 0 + peak * 3 ];
   tmp_g[ 1 ] = f_gaussians[ conc_file ][ 1 + peak * 3 ];
   tmp_g[ 2 ] = f_gaussians[ conc_file ][ 2 + peak * 3 ];

   result = compute_gaussian( t, tmp_g );
   // printvector( "conc curve gaussians", tmp_g );
   // printvector( QString( "conc_curve gaussian before conversion" ), result );

   for ( unsigned int i = 0; i < ( unsigned int ) result.size(); i++ )
   {
      result[ i ] *= detector_conv / conv;
   }

   return result;
}

bool US_Hydrodyn_Saxs_Hplc::adjacent_ok( QString name )
{
   if ( name.contains( "_bsub_a" ) ||
        name.contains( QRegExp( "\\d+$" ) ) )
   {

      return true;
   }
   return false;
}

void US_Hydrodyn_Saxs_Hplc::adjacent()
{
   QString match_name;
   int     match_pos = 0;
   QStringList turn_on;

   disable_all();

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         match_name = lb_files->text( i );
         turn_on << match_name;
         match_pos = i;
         break;
      }
   }

   QRegExp rx;

   bool found = false;
   // if we have bsub
   if ( match_name.contains( "_bsub_a" ) )
   {
      found = true;
      rx.setPattern(
                    QString( "^%1" )
                    .arg( match_name )
                    .replace( QRegExp( "_bsub_a.*$" ), "" )
                    .replace( QRegExp( "\\d+$" ), "\\d+" )
                    + 
                    QString( "%1$" )
                    .arg( match_name )
                    .replace( QRegExp( "^.*_bsub" ), "_bsub" ) 
                    );
   }

   if ( !found && match_name.contains( QRegExp( "_cn\\d+.*$" ) ) )
   {
      found = true;
      rx.setPattern(
                    QString( "^%1" )
                    .arg( match_name )
                    .replace( QRegExp( "_cn\\d+.*$" ), "" )
                    );
   }

   if ( !found && match_name.contains( QRegExp( "\\d+$" ) ) )
   {
      found = true;
      rx.setPattern(
                    QString( "^%1" )
                    .arg( match_name )
                    .replace( QRegExp( "\\d+$" ), "" ) 
                    );
   }

   // cout << "rx: " << rx.pattern() << endl;

   unsigned int newly_set = 0;

   if ( found )
   {
      disable_updates = true;
      
      for ( int i = match_pos - 1; i >= 0; i-- )
      {
         if ( lb_files->text( i ).contains( rx ) )
         {
            if ( !lb_files->isSelected( i ) )
            {
               lb_files->setSelected( i, true );
               newly_set++;
            }
         }
      }
      
      for ( int i = match_pos + 1; i < lb_files->numRows(); i++ )
      {
         if ( lb_files->text( i ).contains( rx ) )
         {
            if ( !lb_files->isSelected( i ) )
            {
               lb_files->setSelected( i, true );
               newly_set++;
            }
         }
      }
      
      if ( !newly_set )
      {
         adjacent_select( lb_files, match_name );
         return;
      }
      disable_updates = false;
      update_files();
   } else {
      adjacent_select( lb_files, match_name );
      return;
   }      
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::adjacent_created()
{
   QString match_name;
   int     match_pos = 0;
   QStringList turn_on;

   disable_all();

   for ( int i = 0; i < lb_created_files->numRows(); i++ )
   {
      if ( lb_created_files->isSelected( i ) )
      {
         match_name = lb_created_files->text( i );
         turn_on << match_name;
         match_pos = i;
         break;
      }
   }

   QRegExp rx;

   bool found = false;
   // if we have bsub
   if ( match_name.contains( "_bsub_a" ) )
   {
      found = true;
      rx.setPattern(
                    QString( "^%1" )
                    .arg( match_name )
                    .replace( QRegExp( "_bsub_a.*$" ), "" )
                    .replace( QRegExp( "\\d+$" ), "\\d+" )
                    + 
                    QString( "%1$" )
                    .arg( match_name )
                    .replace( QRegExp( "^.*_bsub" ), "_bsub" ) 
                    );
   }

   if ( !found && match_name.contains( QRegExp( "_cn\\d+.*$" ) ) )
   {
      found = true;
      rx.setPattern(
                    QString( "^%1" )
                    .arg( match_name )
                    .replace( QRegExp( "_cn\\d+.*$" ), "" )
                    );
   }

   if ( !found && match_name.contains( QRegExp( "\\d+$" ) ) )
   {
      found = true;
      rx.setPattern(
                    QString( "^%1" )
                    .arg( match_name )
                    .replace( QRegExp( "\\d+$" ), "" ) 
                    );
   }

   cout << "rx: " << rx.pattern() << endl;

   unsigned int newly_set = 0;

   if ( found )
   {
      disable_updates = true;
      
      for ( int i = match_pos - 1; i >= 0; i-- )
      {
         if ( lb_created_files->text( i ).contains( rx ) )
         {
            if ( !lb_created_files->isSelected( i ) )
            {
               lb_created_files->setSelected( i, true );
               newly_set++;
            }
         }
      }
      
      for ( int i = match_pos + 1; i < lb_created_files->numRows(); i++ )
      {
         if ( lb_created_files->text( i ).contains( rx ) )
         {
            if ( !lb_created_files->isSelected( i ) )
            {
               lb_created_files->setSelected( i, true );
               newly_set++;
            }
         }
      }

      if ( !newly_set )
      {
         adjacent_select( lb_created_files, match_name );
         return;
      }
      disable_updates = false;
      update_files();
   } else {
      adjacent_select( lb_files, match_name );
      return;
   }      

   update_enables();
}

bool US_Hydrodyn_Saxs_Hplc::adjacent_select( QListBox *lb, QString match )
{
   bool ok;

   static QString last_match;
   if ( match.isEmpty() )
   {
      match = last_match;
   }

   match = QInputDialog::getText(
                                 caption() + tr( ": Select by pattern" ), 
                                 tr( "Regular expression search\n"
                                     "\n"
                                     "Special matches:\n"
                                     " ^ beginning of a line\n"
                                     " $ end of a line\n"
                                     " \\d any digit\n"
                                     " \\d+ one or more digits\n"
                                     " \\d* zero or more digits\n"
                                     " \\d? zero or one digit\n"
                                     " \\d{3} exactly 3 digits\n"
                                     " \\d{1,5} one true five digits\n"
                                     " \\D any non digit\n"
                                     " \\s whitespace\n"
                                     " \\S non-whitespace\n"
                                     " [X-Z] a range of characters\n"
                                     " () group\n"
                                     "e.g.:\n"
                                     " ^([A-B]\\d){2} would match anything that started with A or B followed by a digit twice,\n"
                                     " i.e. A1B2 would match\n\n"
                                     "Enter regular expression pattern:\n"                                     
                                     ), 
                                 QLineEdit::Normal,
                                 match, 
                                 &ok, 
                                 this 
                                 );

   if ( !ok )
   {
      update_enables();
      return false;
   }

   disable_updates = true;
      
   last_match = match;

   QRegExp rx( match );
   bool any_set = false;

   for ( int i = 0; i < lb->numRows(); i++ )
   {
      if ( lb->text( i ).contains( rx ) )
      {
         if ( !lb->isSelected( i ) )
         {
            lb->setSelected( i, true );
            any_set = true;
         }
      }
   }
   disable_updates = false;
   update_files();
   update_enables();
   return any_set;
}

void US_Hydrodyn_Saxs_Hplc::repeak()
{
   QStringList files = all_selected_files();
   repeak( files );
}

void US_Hydrodyn_Saxs_Hplc::repeak( QStringList files )
{
   bool ok;

   QString peak_target = QInputDialog::getItem(
                                               tr( "SOMO: HPLC repeak: enter peak target" ),
                                               tr("Select the file to peak match:\n" ),
                                               files, 
                                               0, 
                                               FALSE, 
                                               &ok,
                                               this );
   if ( !ok ) {
      return;
   }

   map < QString, bool > current_files;
   for ( int i = 0; i < (int)lb_files->numRows(); i++ )
   {
      current_files[ lb_files->text( i ) ] = true;
   }

   map < QString, bool > select_files;
   select_files[ peak_target ] = true;

   double peak;
   if ( !get_peak( peak_target, peak ) )
   {
      return;
   }

   // check files for errors
   bool peak_target_has_errors = ( f_errors.count( peak_target ) && 
                                   f_errors[ peak_target ].size() == f_qs[ peak_target ].size() && 
                                   is_nonzero_vector( f_errors[ peak_target ] ) );
   bool any_without_errors = false;
   double avg_sd_mult = 0e0;

   if ( peak_target_has_errors )
   {
      unsigned int pts = 0;
      for ( unsigned int i = 0; i < ( unsigned int ) f_errors[ peak_target ].size(); i++ )
      {
         if ( f_Is[ peak_target ][ i ] != 0e0 )
         {
            avg_sd_mult += f_errors[ peak_target ][ i ] / f_Is[ peak_target ][ i ];
            pts++;
         }

      }
      avg_sd_mult /= ( double ) pts;

      unsigned int wo_errors_count = 0;
      for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
      {
         if ( files[ i ] == peak_target )
         {
            continue;
         }
         any_without_errors = ( !f_errors.count( files[ i ] ) || 
                                f_errors[ files[ i ] ].size() != f_qs[ files[ i ] ].size() ||
                                !is_nonzero_vector( f_errors[ peak_target ] ) );
         wo_errors_count++;
      }

      if ( any_without_errors )
      {
         switch ( QMessageBox::question(this, 
                                        caption() + tr( ": repeak" ),
                                        QString( tr( "The target has S.D.'s but %1 of %2 file%3 to repeak do not have S.D.'s at every point\n"
                                                     "What would you like to do?\n" ) )
                                        .arg( wo_errors_count ).arg( files.size() - 1 ).arg( files.size() > 2 ? "s" : "" ),
                                        tr( "&Ignore S.D.'s" ), 
                                        tr( "&Set repeaked S.D.'s to the average of %1 %" ).arg( avg_sd_mult * 100e0, 0, 'f', 5 ),
                                        tr( "&Set repeaked S.D.'s to 5 %" ), 
                                        0, // Stop == button 0
                                        0 // Escape == button 0
                                        ) )
         {
         case 0 : // ignore S.D.'s
            any_without_errors = false;
            break;
            
         case 1 : // keep avg_sd_mult
            
            break;
         case 2 : // set to 5%
            avg_sd_mult = 0.05;
            break;
         }  
      }       
   }

   for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
   {
      if ( files[ i ] == peak_target )
      {
         continue;
      }

      double this_peak;
      if ( !get_peak( files[ i ], this_peak ) )
      {
         return;
      }

      
      double scale = peak / this_peak;

      vector < double > repeak_I = f_Is[ files[ i ] ];
      for ( unsigned int j = 0; j < repeak_I.size(); j++ )
      {
         repeak_I[ j ] *= scale;
      }

      vector < double > repeak_e;
      if ( f_errors.count( files[ i ] ) )
      {
         repeak_e = f_errors[ files[ i ] ];
      }         
      for ( unsigned int j = 0; j < repeak_e.size(); j++ )
      {
         repeak_e[ j ] *= scale;
      }

      if ( any_without_errors && avg_sd_mult != 0e0 &&
           ( repeak_e.size() != repeak_I.size() || !is_nonzero_vector( repeak_e ) ) )
      {
         repeak_e.resize( repeak_I.size() );
         for ( unsigned int j = 0; j < repeak_I.size(); j++ )
         {
            repeak_e[ j ] = repeak_I[ j ] * avg_sd_mult;
         }
      }

      int ext = 0;
      QString repeak_name = files[ i ] + QString( "-rp%1" ).arg( scale, 0, 'g', 8 ).replace( ".", "_" );
      while ( current_files.count( repeak_name ) )
      {
         repeak_name = files[ i ] + QString( "-rp%1-%2" ).arg( scale, 0, 'g', 8 ).arg( ++ext ).replace( ".", "_" );
      }

      select_files[ repeak_name ] = true;
      lb_created_files->insertItem( repeak_name );
      lb_created_files->setBottomItem( lb_created_files->numRows() - 1 );
      lb_files->insertItem( repeak_name );
      lb_files->setBottomItem( lb_files->numRows() - 1 );
      created_files_not_saved[ repeak_name ] = true;
   
      f_pos       [ repeak_name ] = f_qs.size();
      f_qs_string [ repeak_name ] = f_qs_string[ files[ i ] ];
      f_qs        [ repeak_name ] = f_qs       [ files[ i ] ];
      f_Is        [ repeak_name ] = repeak_I;
      f_errors    [ repeak_name ] = repeak_e;
      f_is_time   [ repeak_name ] = f_is_time  [ files[ i ] ];
      f_conc      [ repeak_name ] = f_conc.count( files[ i ] ) ? f_conc[ files[ i ] ] : 0e0;
      f_psv       [ repeak_name ] = f_psv .count( files[ i ] ) ? f_psv [ files[ i ] ] : 0e0;
      {
         vector < double > tmp;
         f_gaussians  [ repeak_name ] = tmp;
      }
      editor_msg( "gray", QString( "Created %1\n" ).arg( repeak_name ) );
   }

   lb_files->clearSelection();
   for ( int i = 0; i < (int)lb_files->numRows(); i++ )
   {
      if ( select_files.count( lb_files->text( i ) ) )
      {
         lb_files->setSelected( i, true );
      }
   }

   update_enables();
}
