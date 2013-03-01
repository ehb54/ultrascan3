#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_saxs_hplc.h"
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

// static void printvector( QString qs, vector < double > x )
// {
//    cout << QString( "%1: size %2:" ).arg( qs ).arg( x.size() );
//    for ( unsigned int i = 0; i < x.size(); i++ )
//    {
//       cout << QString( " %1" ).arg( x[ i ], 0, 'f', 10 );
//    }
//    cout << endl;
// }

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

void US_Hydrodyn_Saxs_Hplc::stack_rot_down()
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

void US_Hydrodyn_Saxs_Hplc::stack_rot_up()
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

