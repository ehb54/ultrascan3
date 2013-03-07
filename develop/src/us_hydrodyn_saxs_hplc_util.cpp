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
                                    tr( "US-SOMO: SAXS Hplc" ),
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

   QString head = qstring_common_head( files, true );
   head = head.replace( QRegExp( "__It_q\\d*_$" ), "" );
   head = head.replace( QRegExp( "_q\\d*_$" ), "" );

   if ( !ggaussian_compatible() )
   {
      editor_msg( "red", tr( "NOTICE: Some files selected have Gaussians with varying centers or a different number of Gaussians or centers that do not match the last Gaussians, Please enter \"Global Gaussians\" with these files selected and then \"Keep\" before pressing \"Make I(q)\"" ) );
      return;
   }

   QRegExp rx_q( "_q(\\d+_\\d+)" );
   QRegExp rx_bl( "-bl(.\\d*_\\d+(|e.\\d+))-(.\\d*_\\d+(|e.\\d+))s" );

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

   for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
   {
      progress->setProgress( i, files.size() * 2 );
      qApp->processEvents();
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
            if ( use_errors && f_errors[ files[ i ] ].size() )
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

   bool save_gaussians;

   vector < double > conv;
   vector < double > psv ;
   
   bool normalize_by_conc = false;

   {
      map < QString, QString > parameters;
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

      bool no_conc = false;
      bool sd_from_difference = false;
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

      if ( !any_detector )
      {
         if ( parameters.count( "error" ) )
         {
            parameters[ "error" ] += "\nYou must also select a detector type";
         } else {
            parameters[ "error" ] = "\nYou must select a detector type";
         }
      }            

      US_Hydrodyn_Saxs_Hplc_Ciq *hplc_ciq = 
         new US_Hydrodyn_Saxs_Hplc_Ciq(
                                       this,
                                       & parameters,
                                       this );
      US_Hydrodyn::fixWinButtons( hplc_ciq );
      hplc_ciq->exec();
      delete hplc_ciq;
      
      if ( bl_count && ( !parameters.count( "add_baseline" ) || parameters[ "add_baseline" ] == "false" ) )
      {
         bl_count = 0;
         cout << "ciq: bl off\n";
      }

      if (  parameters.count( "save_as_pct_iq" ) && parameters[ "save_as_pct_iq" ] == "true" )
      {
         save_gaussians = false;
         cout << "ciq: save_gaussians false\n";
      } else {
         save_gaussians = true;
         cout << "ciq: save_gaussians true\n";
      }
      if (  parameters.count( "sd_source" ) && parameters[ "sd_source" ] == "difference" )
      {
         sd_from_difference = true;
         cout << "ciq: sd_from_difference true\n";
      } else {
         save_gaussians = false;
         cout << "ciq: sd_from_difference false\n";
      }
      if ( !parameters.count( "go" ) )
      {
         progress->reset();
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
         if ( parameters.count( "normalize" ) )
         {
            normalize_by_conc = true;
         }
      }
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

      for ( unsigned int g = 0; g < num_of_gauss; g++ )
      {
         // build up an I(q)
         QString name = head + QString( "%1%2%3_pk%4_t%5" )
            .arg( save_gaussians  ? "_G" : "" )
            .arg( any_bl   ? "_bs" : "" )
            .arg( bl_count ? "ba" : "" )
            .arg( g + 1 )
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

         for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
         {
            if ( !I_values.count( tv[ t ] ) )
            {
               editor_msg( "red", QString( tr( "Internal error: I values missing t %1" ) ).arg( tv[ t ] ) );
               running = false;
               update_enables();
               return;
            }

            if ( !I_values[ tv[ t ] ].count( qv[ i ] ) )
            {
               editor_msg( "red", QString( tr( "Internal error: I values missing q %1" ) ).arg( qv[ i ] ) );
               running = false;
               update_enables();
               return;
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

         // do we need to compute a concentration?
         // add to csv conc stuff?
         // normalize by conc (optionally, first compute concentrations)

         // idea: match gaussians from conc file & this file we are
         // creating a I(q) for a t, so across the gaussians (q) our t
         // is at some % of the gaussian, which we map back to the
         // conc gaussian and the use multipliers etc to compute conc
         
         //          if ( conv.size() )
         //          {
         //          }

         lb_created_files->insertItem( name );
         lb_created_files->setBottomItem( lb_created_files->numRows() - 1 );
         lb_files->insertItem( name );
         lb_files->setBottomItem( lb_files->numRows() - 1 );
         created_files_not_saved[ name ] = true;
   
         f_pos       [ name ] = f_qs.size();
         f_qs_string [ name ] = qv_string;
         f_qs        [ name ] = qv;
         f_Is        [ name ] = save_gaussians ? G : I;
         f_errors    [ name ] = e;
         f_is_time   [ name ] = false;
         {
            vector < double > tmp;
            f_gaussians  [ name ] = tmp;
         }
      } // for each gaussian
      add_plot( QString( "sumI_T%1" ).arg( pad_zeros( tv[ t ], (int) tv.size() ) ), qv, gsI, gse, false, false );
      add_plot( QString( "sumG_T%1" ).arg( pad_zeros( tv[ t ], (int) tv.size() ) ), qv, gsG, gse, false, false );
      // add_plot( QString( "sumIr_T%1" ).arg( pad_zeros( tv[ t ], (int) tv.size() ) ), qv, gsI_recon, gse, false, false );
      // add_plot( QString( "sumGr_T%1" ).arg( pad_zeros( tv[ t ], (int) tv.size() ) ), qv, gsG_recon, gse, false, false );
   } // for each q value

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
   if ( !ggaussian_compatible( files ) )
   {
      editor_msg( "red", tr( "Error: Not all files have the same numbers of Gaussians defined" ) );
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
