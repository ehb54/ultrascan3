#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_mals.h"
#include "../include/us_hydrodyn_mals_scale_trend.h"
#include "../include/us_pm.h"
#include <QPixmap>

#define TSO QTextStream(stdout)

// --- guinier ---

void US_Hydrodyn_Mals::guinier()
{
   le_last_focus = (mQLineEdit *) 0;

   bool any_selected = false;

   guinier_names       .clear( );
   guinier_q           .clear( );
   guinier_q2          .clear( );
   guinier_I           .clear( );
   guinier_e           .clear( );
   guinier_x           .clear( );
   guinier_y           .clear( );
   guinier_a           .clear( );
   guinier_b           .clear( );
   guinier_colors      .clear( );

   guinier_it_t        .clear( );
   guinier_it_I        .clear( );

   guinier_it_pg_t     .clear( );
   guinier_it_pg_I     .clear( );

   guinier_report      .clear( );
   guinier_rg_curves   .clear( );
   guinier_mw_curves   .clear( );
   guinier_mwt_markers  .clear( );
   guinier_mwc_markers  .clear( );

   guinier_plot->enableAxis( QwtPlot::xBottom, !guinier_plot_errors->isVisible() );

   if ( current_mode == MODE_TESTIQ )
   {
      if ( !testiq_make() )
      {
         return;
      }
      for ( int i = 0; i < (int) rb_testiq_gaussians.size(); ++i )
      {
         rb_testiq_gaussians[ i ]->hide();
      }

      guinier_mint = -1e0;
      guinier_maxt = 1e0;

      if ( !testiq_it_selected.isEmpty() &&
           f_qs.count( testiq_it_selected ) )
      {
         guinier_it_t      = f_qs[ testiq_it_selected ];
         guinier_it_I      = f_Is[ testiq_it_selected ];
         
         if ( !rb_testiq_from_i_t->isChecked() && 
              f_gaussians.count( testiq_it_selected ) &&
              f_gaussians[ testiq_it_selected ].size() &&
              (int) f_gaussians[ testiq_it_selected ].size() == gaussian_type_size * (int) ( (int) f_gaussians[ testiq_it_selected ].size() / gaussian_type_size ) )
         {
            guinier_it_I = compute_gaussian_sum( guinier_it_t, f_gaussians[ testiq_it_selected ] );
            for ( int i = 0; i < (int) rb_testiq_gaussians.size(); ++i ) {
               if ( rb_testiq_gaussians[ i ]->isChecked() ) {
                  // us_qdebug( QString( "found rb_testiq_gaussian %1 checked" ).arg( i ) );
                  // build up guinier_it_pg_t, I as 2nd gaussian to display
                  vector < double > g( gaussian_type_size );
                  int ofs = i * gaussian_type_size;
                  for ( int j = 0; j < (int) gaussian_type_size; ++j ) {
                     g[ j ] = f_gaussians[ testiq_it_selected ][ ofs + j ];
                  }
                  vector < double > gI = compute_gaussian( guinier_it_t, g );
                  double max = 0e0;
                  for ( int j = 0; j < (int) gI.size(); ++j ) {
                     if ( max < gI[ j ] ) {
                        max = gI[ j ];
                     }
                  }
                  // trim to 5%
                  double cutmax = .05 * max;
                  for ( int j = 0; j < (int) gI.size(); ++j ) {
                     if ( gI[ j ] >= cutmax ) {
                        guinier_it_pg_t.push_back( guinier_it_t[ j ] );
                        guinier_it_pg_I.push_back( gI[ j ] );
                     }
                  }
                  // us_qdebug( US_Vector::qs_vector2( "gaussian peak", guinier_it_pg_t, guinier_it_pg_I ) );
               }
            }
         }
         guinier_it_Imin   = testiq_it_selected_Imin;
         guinier_it_Imax   = testiq_it_selected_Imax;
         guinier_it_Irange = testiq_it_selected_Imax - testiq_it_selected_Imin;
         guinier_mint      = guinier_it_t[ 0 ] - 1;
         guinier_maxt      = guinier_it_t.back() + 1;
      }

      for ( int i = 0; i < (int) testiq_created_names.size(); ++i )
      {
         any_selected = true;
         QString this_file = testiq_created_names[ i ];
         guinier_names.push_back( this_file ); 
         guinier_t[ this_file ] = testiq_created_t[ this_file ];
         guinier_q[ this_file ] = testiq_created_q[ this_file ];
         guinier_I[ this_file ] = testiq_created_I[ this_file ];
         guinier_e[ this_file ] = testiq_created_e[ this_file ];
         for ( int j = 0; j < (int) guinier_q[ this_file ].size(); ++j )
         {
            guinier_q2[ this_file ].push_back( guinier_q[ this_file ][ j ] * guinier_q[ this_file ][ j ] );
         }
         guinier_colors[ this_file ] = plot_colors[ i % plot_colors.size() ];

         if ( !i )
         {
            guinier_minq  = guinier_q [ this_file ][ 0 ];
            guinier_maxq  = guinier_q [ this_file ].back();
            guinier_minq2 = guinier_q2[ this_file ][ 0 ];
            guinier_maxq2 = guinier_q2[ this_file ].back();
         } else {
            if ( guinier_minq > guinier_q [ this_file ][ 0 ] )
            {
               guinier_minq = guinier_q [ this_file ][ 0 ];
            }
            if ( guinier_maxq < guinier_q [ this_file ].back() )
            {
               guinier_maxq = guinier_q [ this_file ].back();
            }
            if ( guinier_minq2 > guinier_q2[ this_file ][ 0 ] )
            {
               guinier_minq2 = guinier_q2[ this_file ][ 0 ];
            }
            if ( guinier_maxq2 < guinier_q2[ this_file ].back() )
            {
               guinier_maxq2 = guinier_q2[ this_file ].back();
            }
         }            
      }
      if ( !any_selected )
      {
         editor_msg( "red", us_tr( "Internal error: no files selected in Guinier mode" ) );
         return;
      }
   } else {
      double pos = 0e0;
      guinier_mint = -1e0;
      guinier_maxt = 1e0;
      for ( int i = 0; i < lb_files->count(); i++ )
      {
         if ( lb_files->item( i )->isSelected() )
         {
            if ( !any_selected )
            {
               wheel_file = lb_files->item( i )->text();
               any_selected = true;
            }
            QString this_file = lb_files->item( i )->text();
            guinier_names.push_back( this_file ); 
            guinier_t[ this_file ] = ++pos;
            guinier_q[ this_file ] = f_qs[ this_file ];
            guinier_I[ this_file ] = f_Is[ this_file ];
            guinier_e[ this_file ] = f_errors[ this_file ];
            for ( int j = 0; j < (int) f_qs[ this_file ].size(); ++j )
            {
               guinier_q2[ this_file ].push_back( f_qs[ this_file ][ j ] * f_qs[ this_file ][ j ] );
            }

            if ( !plotted_curves.count( this_file ) )
            {
               editor_msg( "red", QString( us_tr( "Internal error: guinier selected %1, but no plotted curve found" ) ).arg( this_file ) );
               return;
            }

            if ( !guinier_q[ this_file ].size() )
            {
               editor_msg( "red", QString( us_tr( "Internal error: guinier selected %1, but no data for curve found" ) ).arg( this_file ) );
               return;
            }
            
            guinier_colors[ this_file ] = plot_colors[ f_pos[ this_file ] % plot_colors.size() ];

            if ( !i )
            {
               guinier_minq  = guinier_q [ this_file ][ 0 ];
               guinier_maxq  = guinier_q [ this_file ].back();
               guinier_minq2 = guinier_q2[ this_file ][ 0 ];
               guinier_maxq2 = guinier_q2[ this_file ].back();
            } else {
               if ( guinier_minq > guinier_q [ this_file ][ 0 ] )
               {
                  guinier_minq = guinier_q [ this_file ][ 0 ];
               }
               if ( guinier_maxq < guinier_q [ this_file ].back() )
               {
                  guinier_maxq = guinier_q [ this_file ].back();
               }
               if ( guinier_minq2 > guinier_q2[ this_file ][ 0 ] )
               {
                  guinier_minq2 = guinier_q2[ this_file ][ 0 ];
               }
               if ( guinier_maxq2 < guinier_q2[ this_file ].back() )
               {
                  guinier_maxq2 = guinier_q2[ this_file ].back();
               }
            }            
         }
      }

      guinier_maxt = pos + 1;

      if ( !any_selected )
      {
         editor_msg( "red", us_tr( "Internal error: no files selected in guinier mode" ) );
         return;
      }

      if ( !f_qs.count( wheel_file ) )
      {
         editor_msg( "red", QString( us_tr( "Internal error: %1 not found in data" ) ).arg( wheel_file ) );
         return;
      }

      if ( f_qs[ wheel_file ].size() < 2 )
      {
         editor_msg( "red", QString( us_tr( "Internal error: %1 almost empty data" ) ).arg( wheel_file ) );
         return;
      }

      if ( !f_Is.count( wheel_file ) )
      {
         editor_msg( "red", QString( us_tr( "Internal error: %1 not found in y data" ) ).arg( wheel_file ) );
         return;
      }

      if ( !f_Is[ wheel_file ].size() )
      {
         editor_msg( "red", QString( us_tr( "Internal error: %1 empty y data" ) ).arg( wheel_file ) );
         return;
      }
   }

   if ( le_guinier_q_start->text().isEmpty() ||
        le_guinier_q_start->text() == "0" ||
        le_guinier_q_start->text().toDouble() < guinier_minq )
   {
      disconnect( le_guinier_q_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_guinier_q_start->setText( QString( "%1" ).arg( guinier_minq ) );
      connect( le_guinier_q_start, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_q_start_text( const QString & ) ) );
   }

   if ( le_guinier_q_end->text().isEmpty() ||
        le_guinier_q_end->text() == "0" ||
        le_guinier_q_end->text().toDouble() > guinier_maxq )
   {
      disconnect( le_guinier_q_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_guinier_q_end->setText( QString( "%1" ).arg( guinier_maxq > 0.05 ? 0.05 : guinier_maxq ) );
      connect( le_guinier_q_end, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_q_end_text( const QString & ) ) );
   }

   disconnect( le_guinier_q2_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   le_guinier_q2_start->setText( QString( "%1" ).arg( le_guinier_q_start->text().toDouble() * le_guinier_q_start->text().toDouble() ) );
   connect( le_guinier_q2_start, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_q2_start_text( const QString & ) ) );

   disconnect( le_guinier_q2_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   le_guinier_q2_end->setText( QString( "%1" ).arg( le_guinier_q_end->text().toDouble() * le_guinier_q_end->text().toDouble() ) );
   connect( le_guinier_q2_end, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_q2_end_text( const QString & ) ) );

   disconnect( le_guinier_delta_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   le_guinier_delta_start->setText( QString( "%1" ).arg( 5e-5 ) );
   connect( le_guinier_delta_start, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_delta_start_text( const QString & ) ) );

   disconnect( le_guinier_delta_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   le_guinier_delta_end->setText( QString( "%1" ).arg( 5e-5 ) );
   connect( le_guinier_delta_end, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_delta_end_text( const QString & ) ) );

   if ( guinier_plot_rg_zoomer )
   {
      delete guinier_plot_rg_zoomer;
      guinier_plot_rg_zoomer = (ScrollZoomer *) 0;
   }

   if ( guinier_plot_mw_zoomer )
   {
      delete guinier_plot_mw_zoomer;
      guinier_plot_mw_zoomer = (ScrollZoomer *) 0;
   }

   disable_all();
   cb_guinier_scroll->setChecked( false );
   guinier_scroll_pos = -1;
   mode_select( MODE_GUINIER );
   plot_dist->hide();
   ShowHide::hide_widgets( guinier_errors_widgets, always_hide_widgets );
   ShowHide::hide_widgets( guinier_rg_widgets, always_hide_widgets );
   ShowHide::hide_widgets( guinier_mw_widgets, always_hide_widgets );
   
   running       = true;

   guinier_replot();
   guinier_plot->replot();
   guinier_residuals( true );
   guinier_plot_errors->replot();
   guinier_enables();
   check_mwt_constants();
   guinier_check_qmax();
   {
      QPixmap pm;
      lbl_wheel_Pcolor->setPixmap( pm );
   }
}

void US_Hydrodyn_Mals::guinier_scroll_highlight( int pos )
{
   int stdsize = 4 * use_line_width + 1;
   int bigsize = 7 * use_line_width + 1;

   QString showname = guinier_names[ pos ];
   QString hidename = guinier_names[ guinier_scroll_pos ];

   lbl_wheel_pos      ->setText( "" );
   lbl_wheel_pos_below->setText( QString( "%1" ).arg( showname ) );

   const QwtSymbol * sym;
   // guinier_curves[ hidename ]->setStyle( QwtPlotCurve::NoCurve );
   // guinier_curves[ showname ]->setStyle( QwtPlotCurve::Lines );

   sym = guinier_curves[ hidename ]->symbol();
   guinier_curves[ hidename ]->setSymbol( new QwtSymbol( QwtSymbol::NoSymbol, sym->brush(), sym->pen(), sym->size() ) );

   sym = guinier_curves[ showname ]->symbol();
   guinier_curves[ showname ]->setSymbol( new QwtSymbol( QwtSymbol::Diamond, sym->brush(), sym->pen(), QSize( stdsize, stdsize ) ) );

   if ( guinier_errorbar_curves.count( hidename ) )
   {
      for ( int j = 0; j < (int) guinier_errorbar_curves[ hidename ].size(); ++j )
      {
         guinier_errorbar_curves[ hidename ][ j ]->setStyle( QwtPlotCurve::NoCurve );
      }
   }

   if ( guinier_errorbar_curves.count( showname ) )
   {
      for ( int j = 0; j < (int) guinier_errorbar_curves[ showname ].size(); ++j )
      {
         guinier_errorbar_curves[ showname ][ j ]->setStyle( QwtPlotCurve::Lines );
      }
   }

   if ( guinier_fit_lines.count( hidename ) ) {
      guinier_fit_lines[ hidename ]->setStyle( QwtPlotCurve::NoCurve );
   }

   if ( guinier_fit_lines.count( showname ) ) {
      guinier_fit_lines[ showname ]->setStyle( QwtPlotCurve::Lines );
   }

   if ( guinier_error_curves.count( hidename ) )
   {
      guinier_error_curves[ hidename ]->setStyle( QwtPlotCurve::NoCurve );
   }
   if ( guinier_error_curves.count( showname ) )
   {
      guinier_error_curves[ showname ]->setStyle( QwtPlotCurve::Sticks );
   }

   if ( guinier_rg_curves.count( hidename ) )
   {
      sym = guinier_rg_curves[ hidename ]->symbol();
      guinier_rg_curves[ hidename ]->setSymbol( new QwtSymbol( QwtSymbol::Diamond, sym->brush(), sym->pen(), QSize( stdsize, stdsize ) ) );
   }
   if ( guinier_rg_curves.count( showname ) )
   {
      sym = guinier_rg_curves[ showname ]->symbol();
      guinier_rg_curves[ showname ]->setSymbol( new QwtSymbol( QwtSymbol::Rect, sym->brush(), sym->pen(), QSize( bigsize * 2, bigsize * 2 ) ) );
   }

   if ( guinier_mwt_markers.count( hidename ) )
   {
      sym = guinier_mwt_markers[ hidename ]->symbol();
      guinier_mwt_markers[ hidename ]->setSymbol( new QwtSymbol( QwtSymbol::Diamond, sym->brush(), sym->pen(), QSize( stdsize, stdsize ) ) );
   }
   if ( guinier_mwt_markers.count( showname ) )
   {
      sym = guinier_mwt_markers[ showname ]->symbol();
      guinier_mwt_markers[ showname ]->setSymbol( new QwtSymbol( QwtSymbol::Rect, sym->brush(), sym->pen(), QSize( bigsize, bigsize ) ) );
   }

   if ( guinier_mwc_markers.count( hidename ) )
   {
      sym = guinier_mwc_markers[ hidename ]->symbol();
      guinier_mwc_markers[ hidename ]->setSymbol( new QwtSymbol( QwtSymbol::Diamond, sym->brush(), sym->pen(), QSize( stdsize, stdsize ) ) );
   }
   if ( guinier_mwc_markers.count( showname ) )
   {
      sym = guinier_mwc_markers[ showname ]->symbol();
      guinier_mwc_markers[ showname ]->setSymbol( new QwtSymbol( QwtSymbol::Rect, sym->brush(), sym->pen(), QSize( bigsize, bigsize ) ) );
   }

   guinier_scroll_pos = pos;
   guinier_plot       ->replot();
   guinier_plot_errors->replot();
   guinier_plot_rg    ->replot();
   guinier_plot_mw    ->replot();
   if ( guinier_report.count( showname ) )
   {
      editor_msg( "dark blue", guinier_report[ showname ] );
   } else {
      editor_msg( "dark red", QString( us_tr( "No Guinier information for %1" ) ).arg( showname ) );
   }
}

void US_Hydrodyn_Mals::guinier_scroll()
{
   //   us_qdebug( "--- guinier_scroll() ---" );
   const QwtSymbol * sym;

   int stdsize = 4 * use_line_width + 1;

   if ( le_last_focus && !cb_guinier_scroll->isChecked() )
   {
      le_last_focus->clearFocus();
      cb_guinier_scroll->setChecked( true );
   }

   if ( cb_guinier_scroll->isChecked() )
   {
      ShowHide::hide_widgets( wheel_below_widgets, always_hide_widgets, false );
      //      us_qdebug( "--- guinier_scroll():isChecked ---" );
      le_last_focus = ( mQLineEdit * )0;
      wheel_enables( false );
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );

      for ( int i = 0; i < (int) guinier_names.size(); ++i )
      {
         QString name = guinier_names[ i ];
         // guinier_curves[ name ]->setStyle( QwtPlotCurve::NoCurve );

         sym = guinier_curves[ name ]->symbol();
         guinier_curves[ name ]->setSymbol( new QwtSymbol( QwtSymbol::NoSymbol, sym->brush(), sym->pen(), sym->size() ) );
         if ( guinier_errorbar_curves.count( name ) )
         {
            for ( int j = 0; j < (int) guinier_errorbar_curves[ name ].size(); ++j )
            {
               guinier_errorbar_curves[ name ][ j ]->setStyle( QwtPlotCurve::NoCurve );
            }
         }

         if ( guinier_fit_lines.count( name ) ) {
            guinier_fit_lines[ name ]->setStyle( QwtPlotCurve::NoCurve );
         }

         if ( guinier_error_curves.count( name ) ) {
            guinier_error_curves[ name ]->setStyle( QwtPlotCurve::NoCurve );
         }
      }
      if ( guinier_scroll_pos < 0 )
      {
         guinier_scroll_pos = 0;
      } 
      if ( guinier_scroll_pos >= (int) guinier_names.size() )
      {
         guinier_scroll_pos = (int) guinier_names.size() - 1;
      }

      qwtw_wheel->setRange( 0, guinier_names.size() - 1); qwtw_wheel->setSingleStep( 1 );
      qwtw_wheel->setValue( guinier_scroll_pos );
      lbl_wheel_pos->setText( QString( "%1" ).arg( guinier_names[ guinier_scroll_pos ] ) );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      wheel_enables();
      guinier_scroll_highlight( guinier_scroll_pos );
   } else {
      ShowHide::hide_widgets( wheel_below_widgets, always_hide_widgets );
      // go thru all displayed curves, turn on
      le_last_focus = ( mQLineEdit * )0;
      wheel_enables( false );
      lbl_wheel_pos->setText( "" );

      for ( int i = 0; i < (int) guinier_names.size(); ++i )
      {
         QString name = guinier_names[ i ];
         // guinier_curves[ name ]->setStyle(  QwtPlotCurve::Lines );

         sym = guinier_curves[ name ]->symbol();
         guinier_curves[ name ]->setSymbol( new QwtSymbol( QwtSymbol::Diamond, sym->brush(), sym->pen(), QSize( stdsize, stdsize ) ) );

         if ( guinier_errorbar_curves.count( name ) )
         {
            for ( int j = 0; j < (int) guinier_errorbar_curves[ name ].size(); ++j )
            {
               guinier_errorbar_curves[ name ][ j ]->setStyle( QwtPlotCurve::Lines );
            }
         }

         if ( guinier_fit_lines.count( name ) ) {
            guinier_fit_lines[ name ]->setStyle( QwtPlotCurve::Lines );
         }

         if ( guinier_error_curves.count( name ) )
         {
            guinier_error_curves[ name ]->setStyle( QwtPlotCurve::Sticks );
         }

         if ( guinier_rg_curves.count( name ) )
         {
            sym = guinier_rg_curves[ name ]->symbol();
            guinier_rg_curves[ name ]->setSymbol( new QwtSymbol( QwtSymbol::Diamond, sym->brush(), sym->pen(), QSize( stdsize, stdsize ) ) );
         }
      }
      guinier_plot       ->replot();
      guinier_plot_errors->replot();
      guinier_plot_rg    ->replot();
   }
   guinier_enables();
}

void US_Hydrodyn_Mals::guinier_add_marker( 
                                               QwtPlot * plot,
                                               double pos, 
                                               QColor color, 
                                               QString text, 
                                               Qt::Alignment
                                               align )
{
   QwtPlotMarker * marker = new QwtPlotMarker;
   marker->setLineStyle       ( QwtPlotMarker::VLine );
   marker->setLinePen         ( QPen( color, 2, Qt::DashDotDotLine ) );
   marker->setLabelOrientation( Qt::Horizontal );
   marker->setXValue          ( pos );
   marker->setLabelAlignment  ( align );
   {
      QwtText qwtt( text );
      qwtt.setFont( QFont("Helvetica", 11, QFont::Bold ) );
      marker->setLabel           ( qwtt );
   }
   marker->attach             ( plot );
   guinier_markers.push_back( marker );
}   

void US_Hydrodyn_Mals::guinier_sd()
{
   disable_all();
   guinier_replot();
   guinier_plot->replot();
   //   guinier_analysis();
   //   guinier_residuals();
   guinier_plot_errors->replot();
   guinier_enables();
}

void US_Hydrodyn_Mals::guinier_plot_rg_toggle()
{
   if ( guinier_rg_widgets[ 0 ]->isVisible() )
   {
      ShowHide::hide_widgets( guinier_rg_widgets, always_hide_widgets );
   } else {
      US_Plot_Util::align_plot_extents( { guinier_plot_rg, guinier_plot_mw } );
      connect(((QObject*)guinier_plot_rg->axisWidget(QwtPlot::xBottom)) , SIGNAL(scaleDivChanged () ), usp_guinier_plot_mw, SLOT(scaleDivChangedXSlot () ), Qt::UniqueConnection );
      connect(((QObject*)guinier_plot_mw->axisWidget(QwtPlot::xBottom)) , SIGNAL(scaleDivChanged () ), usp_guinier_plot_rg, SLOT(scaleDivChangedXSlot () ), Qt::UniqueConnection );

      ShowHide::hide_widgets( guinier_rg_widgets, always_hide_widgets, false );
      guinier_plot_rg->enableAxis( QwtPlot::xBottom, !guinier_plot_mw->isVisible() );
   }
}

void US_Hydrodyn_Mals::guinier_plot_mw_toggle()
{
   if ( guinier_mw_widgets[ 0 ]->isVisible() )
   {
      ShowHide::hide_widgets( guinier_mw_widgets, always_hide_widgets );
   } else {
      US_Plot_Util::align_plot_extents( { guinier_plot_rg, guinier_plot_mw } );
      connect(((QObject*)guinier_plot_rg->axisWidget(QwtPlot::xBottom)) , SIGNAL(scaleDivChanged () ), usp_guinier_plot_mw, SLOT(scaleDivChangedXSlot () ), Qt::UniqueConnection );
      connect(((QObject*)guinier_plot_mw->axisWidget(QwtPlot::xBottom)) , SIGNAL(scaleDivChanged () ), usp_guinier_plot_rg, SLOT(scaleDivChangedXSlot () ), Qt::UniqueConnection );

      ShowHide::hide_widgets( guinier_mw_widgets, always_hide_widgets, false );
   }
   guinier_plot_rg->enableAxis( QwtPlot::xBottom, !guinier_plot_mw->isVisible() );
}

void US_Hydrodyn_Mals::guinier_residuals( bool reset )
{
   if ( reset )
   {
      guinier_plot_errors->detachItems( QwtPlotItem::Rtti_PlotCurve );
      guinier_plot_errors->detachItems( QwtPlotItem::Rtti_PlotMarker );
      guinier_error_curves.clear( );
      if ( guinier_markers.size() == 4 )
      {
         guinier_markers.resize( 2 );
         guinier_add_marker( guinier_plot_errors, le_guinier_q2_start  ->text().toDouble(), Qt::red, us_tr( "Start") );
         guinier_add_marker( guinier_plot_errors, le_guinier_q2_end    ->text().toDouble(), Qt::red, us_tr( "End"  ), Qt::AlignLeft | Qt::AlignTop );
      }

      vector < double > x( 2 );
      vector < double > y( 2 );
      x[ 0 ] = guinier_minq2;
      x[ 1 ] = guinier_maxq2;

      QwtPlotCurve *curve = new QwtPlotCurve( "base" );
      curve->setStyle( QwtPlotCurve::Lines );

      curve->setPen( QPen( Qt::red, use_line_width, Qt::SolidLine ) );
      curve->setSamples(
                     (double *)&x[ 0 ],
                     (double *)&y[ 0 ],
                     2
                     );
      curve->attach( guinier_plot_errors );
   }
   
   double tmp;
   double ltmp;

   QwtPlotCurve * curve;

   double emin = 1e99;
   double emax = 0e0;

   double minq2 = le_guinier_q2_start->text().toDouble() - le_guinier_delta_start->text().toDouble();
   double maxq2 = le_guinier_q2_end  ->text().toDouble() + le_guinier_delta_end  ->text().toDouble();


   for ( map < QString, vector <double > >::iterator it = guinier_q2.begin();
         it != guinier_q2.end();
         ++it )
   {
      if ( !guinier_a.count( it->first ) )
      {
         // us_qdebug( "no guinier a line" );
         if ( guinier_error_curves.count( it->first ) )
         {
            return guinier_residuals( true );
         }
         continue;
      }

      // vector < double > * q = & guinier_q[ it->first ];
      vector < double > * I = & guinier_I[ it->first ];
      vector < double > * er = & guinier_e[ it->first ];

      int pts = (int) it->second.size();

      if ( rb_guinier_resid_sd->isChecked() && (int) er->size() != pts )
      {
         if ( guinier_error_curves.count( it->first ) )
         {
            return guinier_residuals( true );
         }
         continue;
      }

      QPen use_pen = QPen( guinier_colors[ it->first ], use_line_width, Qt::SolidLine );

      double a = guinier_a[ it->first ];
      double b = guinier_b[ it->first ];

      vector < double > e( pts );

      if ( rb_guinier_resid_diff->isChecked() )
      {
         for ( int i = 0; i < (int) pts; ++i )
         {
            tmp = (*I)[ i ];
            ltmp = log( tmp > 0e0 ? tmp : 1e-99 );
            e[ i ] =
               ltmp - ( a + b * it->second[ i ] )
               ;
         }
      }
         
      if ( rb_guinier_resid_sd->isChecked() )
      {
         for ( int i = 0; i < (int) pts; ++i )
         {
            if ( (*er)[ i ] == 0e0 )
            {
               e[ i ] = 0e0;
            } else {
               tmp = (*I)[ i ];
               ltmp = log( tmp > 0e0 ? tmp : 1e-99 );
               e[ i ] = (*I)[ i ] * ( ltmp - ( a + b * it->second[ i ] ) ) / (*er)[ i ];
            }
         }
      }

      if ( rb_guinier_resid_pct->isChecked() )
      {
         for ( int i = 0; i < (int) pts; ++i )
         {
            tmp = (*I)[ i ];
            ltmp = log( tmp > 0e0 ? tmp : 1e-99 );
            e[ i ] =
               100e0 * ( ltmp - ( a + b * it->second[ i ] ) ) / fabs( ltmp );
            ;
         }
      }

      for ( int i = 0; i < (int) pts; ++i )
      {
         if ( it->second[ i ] >= minq2 &&
              it->second[ i ] <= maxq2 )
         {
            if ( emin > e[ i ] )
            {
               emin = e[ i ];
            }
            if ( emax < e[ i ] )
            {
               emax = e[ i ];
            }
         }
      }

      if ( guinier_error_curves.count( it->first ) )
      {
         curve = guinier_error_curves[ it->first ];
      } else {
         curve = new QwtPlotCurve( "errors" );
         //         curve->setStyle( QwtPlotCurve::Lines );
         curve->setPen( use_pen );
         curve->setStyle( QwtPlotCurve::Sticks );
         curve->attach( guinier_plot_errors );
      }
         
      curve->setSamples(
                     (double *)&it->second[ 0 ],
                     (double *)&e[ 0 ],
                     pts
                     );
   }

   emax = fabs( emin ) > fabs( emax ) ? fabs( emin ) : fabs( emax );
   emax *= 1.1;
   guinier_plot_errors->setAxisScale( QwtPlot::yLeft, -emax, +emax );
}

void US_Hydrodyn_Mals::guinier_residuals_update()
{
   if ( rb_guinier_resid_diff->isChecked() )
   {
      guinier_plot_errors->setAxisTitle( QwtPlot::yLeft,us_tr( "ln(I(q)) - Guinier line" ) );
   }
   if ( rb_guinier_resid_sd->isChecked() )
   {
      guinier_plot_errors->setAxisTitle( QwtPlot::yLeft,us_tr( "(ln(I(q)) - Guinier line) / S.D." ) );
   }
   if ( rb_guinier_resid_pct->isChecked() )
   {
      guinier_plot_errors->setAxisTitle( QwtPlot::yLeft,us_tr( "% difference [100*(ln(I(q)) - Guinier)/ln(I(q))]" ) );
   }
   guinier_residuals();
   guinier_plot_errors->replot();
}

void US_Hydrodyn_Mals::guinier_analysis()
{
   US_Saxs_Util * usu = ((US_Hydrodyn *)us_hydrodyn)->saxs_util;

   double a;
   double b;
   double siga;
   double sigb;
   double chi2;
   double Rg;
   double I0;
   double smin;
   double smax;
   double sRgmin;
   double sRgmax;

   double qstart      = le_guinier_q_start ->text().toDouble();
   double qend        = le_guinier_q_end   ->text().toDouble();
   double q2start     = le_guinier_q2_start->text().toDouble();
   double q2end       = le_guinier_q2_end  ->text().toDouble();
   //   double q2endvis    = le_guinier_q2_end ->text().toDouble() + le_guinier_delta_end->text().toDouble();
   double sRgmaxlimit = le_guinier_qrgmax ->text().toDouble();
   // double use_q2endvis = q2endvis;
   double use_q2endvis = q2end;
   
   // TSO <<
   //    QString( "guinier_analysis: qstart %1 q2start^.5 %2 qend %3 q2end^.5 %4\n" )
   //    .arg( qstart )
   //    .arg( sqrt( q2start ) )
   //    .arg( qend )
   //    .arg( sqrt( q2end ) )
   //    ;

   // int points_min = 2;

   // bool any_sd_off = false;
   QString this_log;

   editor_msg( "black", "\n" );

   int    count  = 0;

   double i0_avg = 0e0;
   double i0_sum2 = 0e0;
   double i0_min = 1e99;
   double i0_max = -1e99;

   double rg_avg = 0e0;
   double rg_sum2 = 0e0;
   double rg_min = 1e99;
   double rg_max = -1e99;

   double qrg_avg = 0e0;
   double qrg_min = 1e99;
   double qrg_max = -1e99;

   double rg_sd_min = 1e99;
   double rg_sd_max = -1e99;

   guinier_plot_rg->detachItems( QwtPlotItem::Rtti_PlotCurve ); guinier_plot_rg->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   guinier_rg_curves.clear( );

   QwtSymbol rg_sym;
   rg_sym.setStyle( QwtSymbol::Diamond );
   rg_sym.setSize( 4 * use_line_width + 1 );

   guinier_report     .clear( );

   double posmin = 1e99;
   double posmax = -1e99;

   vector < double > rg_x;
   vector < double > rg_y;


   // for ( map < QString, vector <double > >::iterator it = guinier_q2.begin();
   //       it != guinier_q2.end();
   //       ++it )
   // {

   bool do_decrease = cb_guinier_qrgmax->isChecked();
   bool start_do_decrease = do_decrease;

   // if ( do_decrease )
   // {
   //    us_qdebug( "do decrease on" );
   // }


   // mw bits

   guinier_plot_mw->detachItems( QwtPlotItem::Rtti_PlotCurve ); guinier_plot_mw->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   guinier_mw_curves .clear( );
   guinier_mwt_markers.clear( );
   guinier_mwc_markers.clear( );

   double mw_min = 1e99;
   double mw_max = 0e0;

   // tainer mw method

   double Vct;
   double Qrt;
   double mwt;
   double mwt_sd;
   QString messagest;
   QString notest;
   QString warningt;

   double mwt_k =
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_mwt_k" ) ?
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_mwt_k" ].toDouble() : 0e0;
   double mwt_c =
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_mwt_c" ) ?
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_mwt_c" ].toDouble() : 0e0;

   double mwt_qmax =
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_mwt_qmax" ) ?
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_mwt_qmax" ].toDouble() : 0e0;

   vector < double > mwt_x;
   vector < double > mwt_y;
   vector < double > mwt_sds;
   vector < QColor > mwt_qc;
   vector < QString > mwt_names;

   double mwt_min  = 1e99;
   double mwt_max  = 0e0;
   double mwt_avg  = 0e0;
   double mwt_sum2 = 0e0;

   // curtis mw method

   double qm;
   double Vcc;
   double Qrc;
   double mwc;
   double mwc_sd;
   QString messagesc;
   QString notesc;

   double mwc_mw_per_N =
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_mwc_mw_per_N" ) ?
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_mwc_mw_per_N" ].toDouble() : 0e0;

   vector < double > mwc_x;
   vector < double > mwc_y;
   vector < double > mwc_sds;
   vector < QColor > mwc_qc;
   vector < QString > mwc_names;

   double mwc_min  = 1e99;
   double mwc_max  = 0e0;
   double mwc_avg  = 0e0;
   double mwc_sum2 = 0e0;

   // guinier_plot_summary

   guinier_plot_summary->detachItems( QwtPlotItem::Rtti_PlotCurve ); guinier_plot_summary->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   vector < double > ps_x;
   vector < double > ps_rg;
   vector < double > ps_rg_sd;
   vector < double > ps_I0;
   vector < double > ps_I0_sd;

   for ( int i = 0; i < (int) guinier_names.size(); ++i )
   {
      progress->setValue( i ); progress->setMaximum( guinier_names.size() );

      do_decrease = start_do_decrease;

      QString this_name = guinier_names[ i ];
      double pos = guinier_t[ this_name ];
      if ( posmin > pos )
      {
         posmin = pos;
      }
      if ( posmax < pos )
      {
         posmax = pos;
      }

      unsigned int pts_decrease = 0;

      bool computed_rg = false;
      bool use_SD_weighting = cb_guinier_sd->isChecked();

      do {

         computed_rg = false;

         guinier_x[ this_name ].clear( );
         guinier_y[ this_name ].clear( );
         guinier_a.erase( this_name );
         guinier_b.erase( this_name );

         usu->wave["mals"].q.clear( );
         usu->wave["mals"].r.clear( );
         usu->wave["mals"].s.clear( );

         use_SD_weighting = cb_guinier_sd->isChecked();
         if ( guinier_e[ this_name ].size() != guinier_q[ this_name ].size() )
         {
            // any_sd_off = true;
            // editor_msg( "dark red", QString( us_tr( "Notice: SD weighting of Guinier fit is off for %1 since SDs are not fully present" ) )
            //             .arg( this_name ) );
            use_SD_weighting = false;
         } else {
            for ( int j = 0; j < (int) guinier_q[ this_name ].size(); j++ )
            {
               if ( guinier_q[ this_name ][ j ] >= qstart &&
                    guinier_q[ this_name ][ j ] <= qend )
               {
                  if ( guinier_e[ this_name ][ j ] <= 0e0 )
                  {
                     // any_sd_off = true;
                     // editor_msg( "dark red", QString( us_tr( "Notice: SD weighting of Guinier fit is off for %1 since at least one SD is zero or negative in the selected q range" ) )
                     //             .arg( this_name) );
                     use_SD_weighting = false;
                     break;
                  }
               }
            }
         }

         for ( int j = 0; j < (int) guinier_q[ this_name ].size(); j++ )
         {
            if ( guinier_q[ this_name ][ j ] >= qstart &&
                 guinier_q[ this_name ][ j ] <= qend )
            {
               usu->wave[ "mals" ].q.push_back( guinier_q[ this_name ][ j ] );
               usu->wave[ "mals" ].r.push_back( guinier_I[ this_name ][ j ] );
               if ( use_SD_weighting )
               {
                  usu->wave[ "mals" ].s.push_back( guinier_e[ this_name ][ j ] );
               }
            }
         }

         if ( pts_decrease )
         {
            if ( (int) usu->wave[ "mals" ].q.size() - (int) pts_decrease < 5 )
            {
               if ( !cb_guinier_scroll->isChecked() || i == guinier_scroll_pos )
               {
                  editor_msg( "dark red", QString( us_tr( "%1 too few points left %2 after %3 points removed for qRgmax limit" ) )
                              .arg( this_name )
                              .arg( (int) usu->wave[ "mals" ].q.size() - (int) pts_decrease )
                              .arg( pts_decrease ) 
                              );
               }
               break;
            }

            usu->wave[ "mals" ].q.resize( usu->wave[ "mals" ].q.size() - pts_decrease );
            usu->wave[ "mals" ].r.resize( usu->wave[ "mals" ].q.size() );
            if ( use_SD_weighting )
            {
               usu->wave[ "mals" ].s.resize( usu->wave[ "mals" ].q.size() );
            }
         }

         unsigned int pstart = 0;
         unsigned int pend   = usu->wave[ "mals" ].q.size() ? usu->wave[ "mals" ].q.size() - 1 : 0;

         computed_rg = 
            usu->guinier_plot(
                              "malsrg",
                              "mals"
                              )   &&
            usu->guinier_fit(
                             this_log,
                             "malsrg", 
                             pstart,
                             pend,
                             a,
                             b,
                             siga,
                             sigb,
                             chi2,
                             Rg,
                             I0,
                             smax, // don't know why these are flipped
                             smin,
                             sRgmin,
                             sRgmax
                             );
         if ( !computed_rg )
         {
            if ( !cb_guinier_scroll->isChecked() || i == guinier_scroll_pos )
            {
               editor_msg( "dark red", QString( "%1 could not compute Rg" ).arg( this_name ) );
            }
            break;
         }

         // us_qdebug( QString( "%1 decrease check sRgmax %2 sRgmaxlimit %3 %4" ).arg( this_name ).arg( sRgmax ).arg( sRgmaxlimit )
         //         .arg( do_decrease ? "do_decrease on before check" : "do_decrease off before check" ) );
         do_decrease = do_decrease && ( sRgmax > sRgmaxlimit );
         if ( do_decrease )
         {
            pts_decrease++;
         //    us_qdebug( QString( "%1 decreasing %2" ).arg( this_name ).arg( pts_decrease ) );
         // } else {
         //    us_qdebug( "no decrease needed" );
         }
      } while ( do_decrease );
         
      if ( computed_rg )
      {
         // editor_msg( "blue", 
         //             QString( "%1 Rg %2 I0 %3 points %4 qRgmax %5" )
         //             .arg( this_name )
         //             .arg( Rg )
         //             .arg( I0 )
         //             .arg( sRgmax ) 
         //             );
         if ( !us_isnan( Rg ) )
         {
            ps_x.push_back( pos );
            ps_rg.push_back( Rg );
            ps_rg_sd.push_back( sigb );
            ps_I0.push_back( I0 );
            ps_I0_sd.push_back( siga );

            if ( !count )
            {
               qrg_min   = sRgmax;
               qrg_max   = sRgmax;
               rg_min    = Rg;
               rg_max    = Rg;
               i0_min    = I0;
               i0_max    = I0;
               rg_sd_min = Rg;
               rg_sd_max = Rg;
            } else {
               if ( qrg_min > sRgmax )
               {
                  qrg_min = sRgmax;
               }
               if ( qrg_max < sRgmax )
               {
                  qrg_max = sRgmax;
               }
               if ( rg_min > Rg )
               {
                  rg_min = Rg;
               }
               if ( rg_max < Rg )
               {
                  rg_max = Rg;
               }
               if ( i0_min > I0 )
               {
                  i0_min = I0;
               }
               if ( i0_max < I0 )
               {
                  i0_max = I0;
               }
               if ( us_isnan( siga ) || (unsigned int) usu->wave[ "mals" ].q.size() < 3 )
               {
                  if ( rg_sd_min > Rg )
                  {
                     rg_sd_min = Rg;
                  }
                  if ( rg_sd_max < Rg )
                  {
                     rg_sd_max = Rg;
                  }
               } else {
                  if ( rg_sd_min > Rg - sigb )
                  {
                     rg_sd_min = Rg - sigb;
                  }
                  if ( rg_sd_max < Rg + sigb )
                  {
                     rg_sd_max = Rg + sigb;
                  }
               }
            }

            qrg_avg += sRgmax;
            rg_avg  += Rg;
            rg_sum2 += Rg * Rg;
            i0_avg  += I0;
            i0_sum2 += I0 * I0;

            count++;
         }
         

         QString report =
            QString("%1:%2 ").arg( pos ).arg( this_name ) +
            QString( "" )
            .sprintf(
                     "Rg %.1f (%.1f) (A) MW %.2e (%.2e) qRg [%.3f,%.3f] pts %u chi^2 %.2e r-chi %.2e\n"
                     , Rg
                     , sigb
                     , I0
                     , siga
                     , sRgmin
                     , sRgmax
                     , (unsigned int) usu->wave[ "mals" ].q.size()
                     , chi2
                     , sqrt( chi2 / usu->wave[ "mals" ].q.size() )
                     ) +
            us_tr( use_SD_weighting ? "SD  on" : "SD OFF" )
            ;


         // tainer mw method
         if ( 0 ) {
            if ( US_Saxs_Util::mwt( 
                                   guinier_q[ this_name ],
                                   guinier_I[ this_name ],
                                   Rg,
                                   sigb,
                                   I0,
                                   siga,
                                   mwt_k,
                                   mwt_c,
                                   mwt_qmax,
                                   Vct,
                                   Qrt,
                                   mwt,
                                   mwt_sd,
                                   messagest,
                                   notest,
                                   warningt
                                    ) &&
                 !us_isnan( mwt ) )
            {
               report += 
                  QString("")
                  .sprintf( 
                           " Vc[T] %.1e Qr[T] %.2e MW[RT] %.2e ",
                           Vct,
                           Qrt,
                           mwt
                            ) + notest;
               if ( mw_min > mwt )
               {
                  mw_min = mwt;
               }
               if ( mw_max < mwt )
               {
                  mw_max = mwt;
               }
               if ( mwt_min > mwt )
               {
                  mwt_min = mwt;
               }
               if ( mwt_max < mwt )
               {
                  mwt_max = mwt;
               }
               mwt_avg  += mwt;
               mwt_sum2 += mwt * mwt;

               mwt_x  .push_back( pos );
               mwt_y  .push_back( mwt );
               mwt_sds.push_back( mwt_sd );
               mwt_qc   .push_back( guinier_colors[ this_name ] );
               mwt_names.push_back( this_name );
            } else {
               report += us_tr( " MW[RT] could not compute " + messagest + " " + notest );
            }
         }

         // curtis mw method
         // activate in regular mode
         if ( started_in_expert_mode )
         {
            if ( usu->mwc( 
                          guinier_q[ this_name ],
                          guinier_I[ this_name ],
                          Rg,
                          sigb,
                          I0,
                          siga,
                          mwc_mw_per_N,
                          qm,
                          Vcc,
                          Qrc,
                          mwc,
                          mwc_sd,
                          messagesc,
                          notesc
                           ) &&
                 !us_isnan( mwc ) )
            {
               report += 
                  QString("")
                  .sprintf( 
                           " Vc[%.3f,C] %.1e Qr[C] %.2e MW[C] %.2e ",
                           qm,
                           Vcc,
                           Qrc,
                           mwc
                            ) + notesc;
               mwc_x  .push_back( pos );
               mwc_y  .push_back( mwc );
               mwc_sds.push_back( mwc_sd );
               mwc_qc.push_back( guinier_colors[ this_name ] );
               mwc_names.push_back( this_name );
               if ( mw_min > mwc )
               {
                  mw_min = mwc;
               }
               if ( mw_max < mwc )
               {
                  mw_max = mwc;
               }
               if ( mwc_min > mwc )
               {
                  mwc_min = mwc;
               }
               if ( mwc_max < mwc )
               {
                  mwc_max = mwc;
               }
               mwc_avg  += mwc;
               mwc_sum2 += mwc * mwc;
            } else {
               report += us_tr( " MW[C] could not compute " + messagesc + " " + notesc );
            }
         }
         
         guinier_report[ this_name ] = report;

         guinier_a[ this_name ] = a;
         guinier_b[ this_name ] = b;

         if ( start_do_decrease && Rg )
         {
            use_q2endvis = sRgmax / Rg;
            use_q2endvis *= use_q2endvis;
         }
         
         guinier_x[ this_name ].push_back( q2start );
         guinier_y[ this_name ].push_back( exp( a + b * q2start ) );

         guinier_x[ this_name ].push_back( use_q2endvis );
         guinier_y[ this_name ].push_back( exp( a + b * use_q2endvis ) );


         if ( !us_isnan( Rg ) )
         {
            if ( !guinier_fit_lines.count( this_name ) )
            {
               QwtPlotCurve *curve = new QwtPlotCurve( "fl:" + this_name );
               curve->setStyle ( QwtPlotCurve::Lines );
               curve->setPen( QPen( guinier_colors[ this_name ], use_line_width, Qt::SolidLine ) );
               curve->attach( guinier_plot );
               guinier_fit_lines[ this_name ] = curve;
            }
            guinier_fit_lines[ this_name ]->setSamples(
                                                    (double *)&( guinier_x[ this_name ][ 0 ] ),
                                                    (double *)&( guinier_y[ this_name ][ 0 ] ),
                                                    2
                                                    );
            {
               rg_sym.setBrush( guinier_colors[ this_name ] );
               QwtPlotCurve *curve = new QwtPlotCurve( this_name );
               curve->setStyle ( QwtPlotCurve::NoCurve );
               curve->setSymbol( new QwtSymbol( rg_sym.style(), rg_sym.brush(), rg_sym.pen(), rg_sym.size() ) );
               curve->setSamples( & pos, & Rg, 1 );
               curve->attach( guinier_plot_rg );
               guinier_rg_curves[ this_name ] = curve;
               rg_x .push_back( pos );
               rg_y .push_back( Rg );
            }

            {
               double x[2];
               double y[2];
               
               x[ 0 ] = x[ 1 ] = pos;
               y[ 0 ] = Rg - sigb;
               y[ 1 ] = Rg + sigb;

               QPen use_pen = QPen( guinier_colors[ this_name ], use_line_width, Qt::SolidLine );
               QwtPlotCurve *curve = new QwtPlotCurve( this_name + "_sd" );
               curve->setStyle ( QwtPlotCurve::Lines );
               curve->setSamples( x, y, 2 );
               curve->setPen( use_pen );
               curve->attach( guinier_plot_rg );
               rg_x .push_back( pos );
               rg_y .push_back( Rg );
            }
         }

         if ( !cb_guinier_scroll->isChecked() || i == guinier_scroll_pos )
         {
             editor_msg( "dark blue", report );
         }
         // us_qdebug( QString( "guinier a %1 b %2" ).arg( a ).arg( b ) );
         // US_Vector::printvector2( "guinier x,y:", guinier_x[ this_name ], guinier_y[ this_name ] );
      }
   }

   // guinier_plot_summary
   {
      if ( ps_x.size() ) {
         {
            QwtPlotCurve *curve = new QwtPlotCurve( "Rg" );
            curve->setSamples( (double *)&(ps_x[0]),
                            (double *)&(ps_rg[0]),
                            ps_x.size() );
            curve->attach( guinier_plot_summary );
         }
         {
            QwtPlotCurve *curve = new QwtPlotCurve( "Rg sd" );
            curve->setSamples( (double *)&(ps_x[0]),
                            (double *)&(ps_rg_sd[0]),
                            ps_x.size() );
            curve->attach( guinier_plot_summary );
         }
         {
            QwtPlotCurve *curve = new QwtPlotCurve( "I0" );
            curve->setSamples( (double *)&(ps_x[0]),
                            (double *)&(ps_I0[0]),
                            ps_x.size() );
            curve->attach( guinier_plot_summary );
         }
         {
            QwtPlotCurve *curve = new QwtPlotCurve( "I0 sd" );
            curve->setSamples( (double *)&(ps_x[0]),
                            (double *)&(ps_I0_sd[0]),
                            ps_x.size() );
            curve->attach( guinier_plot_summary );
         }
      }
      if ( mwt_x.size() ) {
         {
            QwtPlotCurve *curve = new QwtPlotCurve( "MW[RT]" );
            curve->setSamples( (double *)&(mwt_x[0]),
                            (double *)&(mwt_y[0]),
                            mwt_x.size() );
            curve->attach( guinier_plot_summary );
         }
         {
            QwtPlotCurve *curve = new QwtPlotCurve( "MW[RT] sd" );
            curve->setSamples( (double *)&(mwt_x[0]),
                            (double *)&(mwt_sds[0]),
                            mwt_x.size() );
            curve->attach( guinier_plot_summary );
         }
      }
      if ( mwc_x.size() ) {
         {
            QwtPlotCurve *curve = new QwtPlotCurve( "MW[C]" );
            curve->setSamples( (double *)&(mwc_x[0]),
                            (double *)&(mwc_y[0]),
                            mwc_x.size() );
            curve->attach( guinier_plot_summary );
         }
         {
            QwtPlotCurve *curve = new QwtPlotCurve( "MW[C] sd" );
            curve->setSamples( (double *)&(mwc_x[0]),
                            (double *)&(mwc_sds[0]),
                            mwc_x.size() );
            curve->attach( guinier_plot_summary );
         }
      }
   }
   
   QString msg;
   switch ( count )
   {
   case 0 : msg = ""; break;
   case 1 : msg = QString( "qmax*Rg %1   Rg %2   MW %3" ).arg( qrg_avg ).arg( rg_avg ).arg( i0_avg ); break;
   default :
      {
         double countinv = 1e0 / (double) count;
         double countm1inv = 1e0 / ((double) count - 1 );
         double rg_sd = sqrt( countm1inv * ( rg_sum2 - countinv * rg_avg * rg_avg ) );
         double i0_sd = sqrt( countm1inv * ( i0_sum2 - countinv * i0_avg * i0_avg ) );

         qrg_avg *= countinv;
         rg_avg  *= countinv;
         i0_avg  *= countinv;

         msg = QString( "Avg. %1 " ).arg( count );
         if ( count != ( int ) guinier_q2.size() )
         {

            msg += QString( "of %1 " ).arg( guinier_q2.size() );
         }
         msg += QString( "" )
            .sprintf(
                     "curves  qmax*Rg %.3f [%.3f:%.3f]  Rg %.1f (%.1f) [%.1f:%.1f]\n  MW %.2e (%.2e) [%.2e:%.2e]"
                     , qrg_avg
                     , qrg_min
                     , qrg_max
                     , rg_avg
                     , rg_sd
                     , rg_min
                     , rg_max
                     , i0_avg
                     , i0_sd
                     , i0_min
                     , i0_max
                     )
            ;
      }
      break;
   }
   if ( 0 && ( mwt_x.size() || mwc_x.size() ) )
   {
      msg += "\n";
      if ( mwt_x.size() )
      {
         double count    = (double) mwt_x.size();
         double countinv = 1e0 / count;
         mwt_avg  *= countinv;
         msg += QString( "MW[RT] " );
         if ( count != ( int ) guinier_q2.size() )
         {
            msg += QString( "%1 of %2 " ).arg( count ).arg( guinier_q2.size() );
         }

         if ( count > 2 )
         {
            msg += "Avg. ";
            double countm1inv = 1e0 / (count - 1e0 );
            double mwt_sd = sqrt( countm1inv * ( mwt_sum2 - count * mwt_avg * mwt_avg ) );

            msg += QString( "" )
               .sprintf(
                        " %.4g (%.4g) [%.4g:%.4g]"
                        , mwt_avg
                        , mwt_sd
                        , mwt_min
                        , mwt_max
                        );
         } else {
            msg += QString( "" )
               .sprintf(
                        " %.4g "
                        , mwt_avg );
         }
         if ( mwc_x.size() )
         {
            msg += "   ";
         }
      }
      if ( mwc_x.size() )
      {
         double count    = (double) mwc_x.size();
         double countinv = 1e0 / count;
         mwc_avg  *= countinv;
         msg += QString( "MW[C] " );
         if ( count != ( int ) guinier_q2.size() )
         {
            msg += QString( "%1 of %2 " ).arg( count ).arg( guinier_q2.size() );
         }

         if ( count > 2 )
         {
            msg += "Avg. ";
            double countm1inv = 1e0 / (count - 1e0 );
            double mwc_sd = sqrt( countm1inv * ( mwc_sum2 - count * mwc_avg * mwc_avg ) );

            msg += QString( "" )
               .sprintf(
                        " %.4g (%.4g) [%.4g:%.4g]"
                        , mwc_avg
                        , mwc_sd
                        , mwc_min
                        , mwc_max
                        );
         } else {
            msg += QString( "" )
               .sprintf(
                        " %.4g "
                        , mwc_avg );
         }
         if ( mwc_x.size() )
         {
            msg += "   ";
         }
      }
   }

   // us_qdebug( msg );

   lbl_guinier_stats->setText( msg );

   if ( rg_x.size() > 1 )
   {
      QPen use_pen = QPen( Qt::cyan, use_line_width, Qt::DotLine );
      QwtPlotCurve * curve = new QwtPlotCurve( "rgline" );
      curve->setStyle( QwtPlotCurve::Lines );
      curve->setSamples(
                     (double *)&(rg_x[0]),
                     (double *)&(rg_y[0]),
                     rg_x.size() );
      curve->setPen( use_pen );
      curve->attach( guinier_plot_rg );
   }

   if ( mwt_x.size() > 1 )
   {
      QPen use_pen = QPen( Qt::green, use_line_width, Qt::DotLine );
      QwtPlotCurve * curve = new QwtPlotCurve( "mwtline" );
      curve->setStyle( QwtPlotCurve::Lines );
      curve->setSamples(
                     (double *)&(mwt_x[0]),
                     (double *)&(mwt_y[0]),
                     mwt_x.size() );
      curve->setPen( use_pen );
      curve->setTitle( "MW[RT]" );
      curve->attach( guinier_plot_mw );
      // us_qdebug( US_Vector::qs_vector2( "mwt", mwt_x, mwt_y ) );

      // as markers
      {
         QwtSymbol sym;
         sym.setSize( use_line_width * 4 + 1 );
         sym.setStyle( QwtSymbol::Diamond );

         for ( int i = 0; i < (int) mwt_x.size(); ++i )
         {
            QColor qc = mwt_qc[ i ];
            sym.setPen  ( qc );
            sym.setBrush( qc );
            QwtPlotMarker* marker = new QwtPlotMarker;
            marker->setSymbol( new QwtSymbol( sym.style(), sym.brush(), sym.pen(), sym.size() ) );
            marker->setValue( mwt_x[ i ], mwt_y[ i ] );
            marker->attach( guinier_plot_mw );
            guinier_mwt_markers[ mwt_names[ i ] ] = marker;
         }
      }
   }

   if ( mwc_x.size() > 1 )
   {
      QPen use_pen = QPen( Qt::cyan, use_line_width, Qt::DotLine );
      QwtPlotCurve * curve = new QwtPlotCurve( "mwcline" );
      curve->setStyle( QwtPlotCurve::Lines );
      curve->setSamples(
                     (double *)&(mwc_x[0]),
                     (double *)&(mwc_y[0]),
                     mwc_x.size() );
      curve->setPen( use_pen );
      curve->setTitle( "MW[C]" );
      curve->attach( guinier_plot_mw );
      // as markers
      {
         QwtSymbol sym;
         sym.setSize( use_line_width * 4 + 1 );
         sym.setStyle( QwtSymbol::Diamond );

         for ( int i = 0; i < (int) mwc_x.size(); ++i )
         {
            QColor qc = mwc_qc[ i ];
            sym.setPen  ( qc );
            sym.setBrush( qc );
            QwtPlotMarker* marker = new QwtPlotMarker;
            marker->setSymbol( new QwtSymbol( sym.style(), sym.brush(), sym.pen(), sym.size() ) );
            marker->setValue( mwc_x[ i ], mwc_y[ i ] );
            marker->attach( guinier_plot_mw );
            guinier_mwc_markers[ mwc_names[ i ] ] = marker;
         }
      }
      // us_qdebug( US_Vector::qs_vector2( "mwc", mwc_x, mwc_y ) );
   }

   double use_min = posmin - 1e0;
   double use_max = posmax + 1e0;
   double space = 0.05 * ( rg_sd_max - rg_sd_min ) ;
   bool alt_space = false;
   if ( space < rg_sd_max * 0.005 )
   {
      // us_qdebug( "alt space" );
      alt_space = true;
      space = rg_sd_max * 0.005;
   }
   // us_qdebug( QString( "space %1 rg_max %2 rg_sd_max %3 rg_min %4 rg_sd_min %5 count %6" )
   //         .arg( space )
   //         .arg( rg_max )
   //         .arg( rg_sd_max )
   //         .arg( rg_min )
   //         .arg( rg_sd_min )
   //         .arg( count )
   //         );

   if ( rg_x.size() > 1 && guinier_it_t.size() && space > 0e0 )
   {
      // us_qdebug( QString( "testiq in rg plot ok use minmax %1 %2" ).arg( use_min ).arg( use_max ) );
      vector < double > x;
      vector < double > y;
      vector < double > y2 = guinier_it_I;
      vector < double > y3 = guinier_it_pg_I;

      double ymin = 1e99;
      double ymax = -1e99;

      if ( cb_guinier_lock_rg_range->isChecked() )
      {
         use_min = le_guinier_rg_t_start->text().toDouble();
         use_max = le_guinier_rg_t_end->text().toDouble();
      }

      for ( int i = 0; i < (int) guinier_it_t.size(); ++i )
      {
         if ( guinier_it_t[ i ] >= use_min &&
              guinier_it_t[ i ] <= use_max )
         {
            x.push_back( guinier_it_t[ i ] );
            y.push_back( guinier_it_I[ i ] );
            if ( ymin > guinier_it_I[ i ] )
            {
               ymin = guinier_it_I[ i ];
            } 
            if ( ymax < guinier_it_I[ i ] )
            {
               ymax = guinier_it_I[ i ];
            } 
         }
      }
      // US_Vector::printvector2( "ref x,y", x, y );

      if ( ymax > ymin )
      {
         if ( cb_guinier_lock_rg_range->isChecked() )
         {
            double mint = le_guinier_rg_rg_start->text().toDouble();
            double maxt = le_guinier_rg_rg_end->text().toDouble();
            double scale = ( maxt - mint ) / ( ymax - ymin );
            for ( int i = 0; i < (int) y2.size(); ++i )
            {
               y2[ i ] -= ymin;
               y2[ i ] *= scale;
               y2[ i ] += mint;
            }
            for ( int i = 0; i < (int) y3.size(); ++i )
            {
               y3[ i ] -= ymin;
               y3[ i ] *= scale;
               y3[ i ] += mint;
            }
         } else {
            if ( alt_space )
            {
               double total_range = ( space * 2e0 + rg_sd_max - rg_sd_min );
               double use_range = total_range * .95;
               double scale = use_range / ( ymax - ymin );
               double base = ( rg_sd_min - space ) + 0.025 * total_range;
               for ( int i = 0; i < (int) y2.size(); ++i )
               {
                  y2[ i ] -= ymin;
                  y2[ i ] *= scale;
                  y2[ i ] += base;
               }
               for ( int i = 0; i < (int) y3.size(); ++i )
               {
                  y3[ i ] -= ymin;
                  y3[ i ] *= scale;
                  y3[ i ] += base;
               }
            } else {
               double scale = ( rg_sd_max - rg_sd_min ) / ( ymax - ymin );
               for ( int i = 0; i < (int) y2.size(); ++i )
               {
                  y2[ i ] -= ymin;
                  y2[ i ] *= scale;
                  y2[ i ] += rg_sd_min;
               }
               for ( int i = 0; i < (int) y3.size(); ++i )
               {
                  y3[ i ] -= ymin;
                  y3[ i ] *= scale;
                  y3[ i ] += rg_sd_min;
               }
            }
         }

         // us_qdebug( QString( "testiq in rg plot scale %1 rg_min %2 rg_max %3 ymin %4 ymax %5 " ).arg( scale ).arg( rg_min ).arg( rg_max ).arg( ymin ).arg( ymax ) );

         // for ( int i = 0; i < (int) y.size(); ++i )
         // {
         //    y[ i ] -= ymin;
         //    y[ i ] *= scale;
         //    y[ i ] += rg_min;
         // }

         // for highlighting primary gaussian


         // QPen use_pen = QPen( plot_colors[ f_pos[ testiq_it_selected ] % plot_colors.size() ], use_line_width, Qt::SolidLine );
         QPen use_pen = QPen( Qt::green, use_line_width, y3.size() ? Qt::DotLine : Qt::SolidLine );
         QwtPlotCurve * curve = new QwtPlotCurve( "refitline" );
         curve->setStyle( QwtPlotCurve::Lines );
         curve->setSamples(
                        (double *)&(guinier_it_t[0]),
                        (double *)&(y2[0]),
                        guinier_it_t.size() );
         curve->setPen( use_pen );
         curve->attach( guinier_plot_rg );
         if ( y3.size() ) {
            QPen use_pen = QPen( Qt::magenta, use_line_width, Qt::SolidLine );
            QwtPlotCurve * curve = new QwtPlotCurve( "refitline" );
            curve->setStyle( QwtPlotCurve::Lines );
            curve->setSamples(
                           (double *)&(guinier_it_pg_t[0]),
                           (double *)&(y3[0]),
                           guinier_it_pg_t.size() );
            curve->setPen( use_pen );
            curve->attach( guinier_plot_rg );
         }
      }
   }


   {
      if ( cb_guinier_lock_rg_range->isChecked() )
      {
         guinier_plot_rg->setAxisScale( QwtPlot::xBottom, le_guinier_rg_t_start->text().toDouble(), le_guinier_rg_t_end->text().toDouble() );
         guinier_plot_rg->setAxisScale( QwtPlot::yLeft  , le_guinier_rg_rg_start->text().toDouble(), le_guinier_rg_rg_end->text().toDouble() );
      } else {
         guinier_plot_rg->setAxisScale( QwtPlot::xBottom, use_min, use_max );
         guinier_plot_rg->setAxisScale( QwtPlot::yLeft  , rg_sd_min - space, rg_sd_max + space );
         
         disconnect( le_guinier_rg_t_start , SIGNAL( textChanged( const QString & ) ), 0, 0 );
         disconnect( le_guinier_rg_t_end   , SIGNAL( textChanged( const QString & ) ), 0, 0 );
         disconnect( le_guinier_rg_rg_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
         disconnect( le_guinier_rg_rg_end  , SIGNAL( textChanged( const QString & ) ), 0, 0 );
         le_guinier_rg_t_start ->setText( QString( "%1" ).arg( use_min ) );
         le_guinier_rg_t_end   ->setText( QString( "%1" ).arg( use_max ) );
         le_guinier_rg_rg_start->setText( QString( "%1" ).arg( rg_sd_min - space ) );
         le_guinier_rg_rg_end  ->setText( QString( "%1" ).arg( rg_sd_max + space ) );
         connect( le_guinier_rg_t_start, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_rg_t_start_text( const QString & ) ) );
         connect( le_guinier_rg_t_end, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_rg_t_end_text( const QString & ) ) );
         connect( le_guinier_rg_rg_start, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_rg_rg_start_text( const QString & ) ) );
         connect( le_guinier_rg_rg_end, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_rg_rg_end_text( const QString & ) ) );
      }

      if ( guinier_plot_rg_zoomer )
      {
          delete guinier_plot_rg_zoomer;
      }
      guinier_plot_rg_zoomer = new ScrollZoomer(guinier_plot_rg->canvas());
      guinier_plot_rg_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
      guinier_plot_rg_zoomer->setTrackerPen(QPen(Qt::red));
   }
      
   double mw_use_min = posmin - 1e0;
   double mw_use_max = posmax + 1e0;
   double mw_space = 0.05 * ( mw_max - mw_min );
   bool   mw_alt_space = false;

   if ( ( mwt_x.size() > 1 || mwc_x.size() > 1 ) && guinier_it_t.size() && mw_space > 0e0)
   {
      vector < double > x;
      vector < double > y;
      vector < double > y2 = guinier_it_I;
      vector < double > y3 = guinier_it_pg_I;

      double ymin = 1e99;
      double ymax = -1e99;


      if ( cb_guinier_lock_mw_range->isChecked() )
      {
         mw_use_min = le_guinier_mw_t_start->text().toDouble();
         mw_use_max = le_guinier_mw_t_end->text().toDouble();
      }

      for ( int i = 0; i < (int) guinier_it_t.size(); ++i )
      {
         if ( guinier_it_t[ i ] >= mw_use_min &&
              guinier_it_t[ i ] <= mw_use_max )
         {
            x.push_back( guinier_it_t[ i ] );
            y.push_back( guinier_it_I[ i ] );
            if ( ymin > guinier_it_I[ i ] )
            {
               ymin = guinier_it_I[ i ];
            } 
            if ( ymax < guinier_it_I[ i ] )
            {
               ymax = guinier_it_I[ i ];
            } 
         }
      }
      // US_Vector::printvector2( "ref x,y", x, y );

      if ( ymax > ymin )
      {
         if ( cb_guinier_lock_mw_range->isChecked() )
         {
            double mint = le_guinier_mw_mw_start->text().toDouble();
            double maxt = le_guinier_mw_mw_end->text().toDouble();
            double scale = ( maxt - mint ) / ( ymax - ymin );
            for ( int i = 0; i < (int) y2.size(); ++i )
            {
               y2[ i ] -= ymin;
               y2[ i ] *= scale;
               y2[ i ] += mint;
            }
            for ( int i = 0; i < (int) y3.size(); ++i )
            {
               y3[ i ] -= ymin;
               y3[ i ] *= scale;
               y3[ i ] += mint;
            }
         } else {
            if ( mw_alt_space )
            {
               double total_range = ( mw_space * 2e0 + mw_max - mw_min );
               double use_range = total_range * .95;
               double scale = use_range / ( ymax - ymin );
               double base = ( mw_min - mw_space ) + 0.025 * total_range;
               for ( int i = 0; i < (int) y2.size(); ++i )
               {
                  y2[ i ] -= ymin;
                  y2[ i ] *= scale;
                  y2[ i ] += base;
               }
               for ( int i = 0; i < (int) y3.size(); ++i )
               {
                  y3[ i ] -= ymin;
                  y3[ i ] *= scale;
                  y3[ i ] += base;
               }
            } else {
               double scale = ( mw_max - mw_min ) / ( ymax - ymin );
               for ( int i = 0; i < (int) y2.size(); ++i )
               {
                  y2[ i ] -= ymin;
                  y2[ i ] *= scale;
                  y2[ i ] += mw_min;
               }
               for ( int i = 0; i < (int) y3.size(); ++i )
               {
                  y3[ i ] -= ymin;
                  y3[ i ] *= scale;
                  y3[ i ] += rg_sd_min;
               }
            }
         }

         // us_qdebug( QString( "testiq in mw plot scale %1 mw_min %2 mw_max %3 ymin %4 ymax %5 " ).amw( scale ).amw( mw_min ).amw( mw_max ).amw( ymin ).amw( ymax ) );

         // for ( int i = 0; i < (int) y.size(); ++i )
         // {
         //    y[ i ] -= ymin;
         //    y[ i ] *= scale;
         //    y[ i ] += mw_min;
         // }

         // QPen use_pen = QPen( plot_colors[ f_pos[ testiq_it_selected ] % plot_colors.size() ], use_line_width, Qt::SolidLine );
         QPen use_pen = QPen( Qt::green, use_line_width, y3.size() ? Qt::DotLine : Qt::SolidLine );
         QwtPlotCurve * curve = new QwtPlotCurve( "I(t)" );
         curve->setStyle( QwtPlotCurve::Lines );
         curve->setSamples(
                        (double *)&(guinier_it_t[0]),
                        (double *)&(y2[0]),
                        guinier_it_t.size() );
         curve->setPen( use_pen );
         curve->attach( guinier_plot_mw );
         if ( y3.size() ) {
            QPen use_pen = QPen( Qt::magenta, use_line_width, Qt::SolidLine );
            QwtPlotCurve * curve = new QwtPlotCurve( "refitline" );
            curve->setStyle( QwtPlotCurve::Lines );
            curve->setSamples(
                           (double *)&(guinier_it_pg_t[0]),
                           (double *)&(y3[0]),
                           guinier_it_pg_t.size() );
            curve->setPen( use_pen );
            curve->attach( guinier_plot_mw );
         }
      }
   }


   if ( mwt_x.size() || mwt_y.size() )
   {
      if ( cb_guinier_lock_mw_range->isChecked() )
      {
         guinier_plot_mw->setAxisScale( QwtPlot::xBottom, le_guinier_mw_t_start->text().toDouble(), le_guinier_mw_t_end->text().toDouble() );
         guinier_plot_mw->setAxisScale( QwtPlot::yLeft  , le_guinier_mw_mw_start->text().toDouble(), le_guinier_mw_mw_end->text().toDouble() );
      } else {
         guinier_plot_mw->setAxisScale( QwtPlot::xBottom, mw_use_min, mw_use_max );
         guinier_plot_mw->setAxisScale( QwtPlot::yLeft  , mw_min - space, mw_max + space );
         
         disconnect( le_guinier_mw_t_start , SIGNAL( textChanged( const QString & ) ), 0, 0 );
         disconnect( le_guinier_mw_t_end   , SIGNAL( textChanged( const QString & ) ), 0, 0 );
         disconnect( le_guinier_mw_mw_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
         disconnect( le_guinier_mw_mw_end  , SIGNAL( textChanged( const QString & ) ), 0, 0 );
         le_guinier_mw_t_start ->setText( QString( "%1" ).arg( mw_use_min ) );
         le_guinier_mw_t_end   ->setText( QString( "%1" ).arg( mw_use_max ) );
         le_guinier_mw_mw_start->setText( QString( "%1" ).arg( mw_min - space ) );
         le_guinier_mw_mw_end  ->setText( QString( "%1" ).arg( mw_max + space ) );
         connect( le_guinier_mw_t_start, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_mw_t_start_text( const QString & ) ) );
         connect( le_guinier_mw_t_end, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_mw_t_end_text( const QString & ) ) );
         connect( le_guinier_mw_mw_start, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_mw_mw_start_text( const QString & ) ) );
         connect( le_guinier_mw_mw_end, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_mw_mw_end_text( const QString & ) ) );
      }
      
      if ( guinier_plot_mw_zoomer )
      {
          delete guinier_plot_mw_zoomer;
      }
      guinier_plot_mw_zoomer = new ScrollZoomer(guinier_plot_mw->canvas());
      guinier_plot_mw_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
      guinier_plot_mw_zoomer->setTrackerPen(QPen(Qt::red));
   }

   if ( testiq_active && rb_testiq_gaussians.size() )
   {
      for ( int i = 0; i < (int) unified_ggaussian_gaussians_size; i++ )
      {
         int line_width = use_line_width < 3 ? ( use_line_width + 1 ) : use_line_width;
         double pos = unified_ggaussian_params[ (vector<double>::size_type) common_size * i  ];
         QString text = QString( "%1" ).arg( i + 1 );
         QColor color = rb_testiq_gaussians[ i ]->isChecked() ? Qt::magenta : Qt::blue;
         QwtPlotMarker * marker = new QwtPlotMarker;
         marker->setLineStyle       ( QwtPlotMarker::VLine );
         marker->setLinePen         ( QPen( color, line_width, Qt::DashDotDotLine ) );
         marker->setLabelOrientation( Qt::Horizontal );
         marker->setXValue          ( pos );
         marker->setLabelAlignment  ( Qt::AlignRight | Qt::AlignTop );
         {
            QwtText qwtt( text );
            qwtt.setFont( QFont("Helvetica", 11, QFont::Bold ) );
            marker->setLabel           ( qwtt );
         }
         marker->attach             ( guinier_plot_rg );
      }
   }
   guinier_plot    ->replot();
   guinier_plot_rg ->replot();
   guinier_plot_mw ->replot();
   progress->setValue( 1 ); progress->setMaximum( 1 );
}

void US_Hydrodyn_Mals::guinier_delete_markers()
{
   guinier_plot       ->detachItems( QwtPlotItem::Rtti_PlotMarker );
   guinier_plot_errors->detachItems( QwtPlotItem::Rtti_PlotMarker );
}

void US_Hydrodyn_Mals::guinier_range( 
                                          double minq2, 
                                          double maxq2,
                                          double minI, 
                                          double maxI
                                           )
{
   guinier_plot       ->setAxisScale( QwtPlot::xBottom, minq2, maxq2 );
   guinier_plot_errors->setAxisScale( QwtPlot::xBottom, minq2, maxq2 );
   guinier_plot       ->setAxisScale( QwtPlot::yLeft  , minI * 0.9e0 , maxI * 1.1e0 );

   if ( guinier_plot_zoomer )
   {
      delete guinier_plot_zoomer;
      guinier_plot_zoomer = (ScrollZoomer *) 0;
   }

   if ( !guinier_plot_zoomer )
   {
      // puts( "redoing zoomer" );
      guinier_plot_zoomer = new ScrollZoomer(guinier_plot->canvas());
      guinier_plot_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
      guinier_plot_zoomer->setTrackerPen(QPen(Qt::red));
      // connect( guinier_plot_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_zoomed( const QRectF & ) ) );
   }

   if ( guinier_plot_errors_zoomer )
   {
      delete guinier_plot_errors_zoomer;
      guinier_plot_errors_zoomer = (ScrollZoomer *) 0;
   }

   if ( !guinier_plot_errors_zoomer )
   {
      // puts( "redoing zoomer" );
      guinier_plot_errors_zoomer = new ScrollZoomer(guinier_plot_errors->canvas());
      guinier_plot_errors_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
      guinier_plot_errors_zoomer->symmetric_rescale = true;
      guinier_plot_errors_zoomer->setTrackerPen(QPen(Qt::red));
      // connect( guinier_plot_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_zoomed( const QRectF & ) ) );
   }
}

void US_Hydrodyn_Mals::guinier_range()
{
   double minI  = 1e99;
   double maxI  = 0e0;
   double minq2 = le_guinier_q2_start->text().toDouble() - le_guinier_delta_start->text().toDouble();
   double maxq2 = le_guinier_q2_end  ->text().toDouble() + le_guinier_delta_end  ->text().toDouble();

   if ( minq2 < guinier_minq2 )
   {
      minq2 = guinier_minq2;
   }
   if ( maxq2 > guinier_maxq2 )
   {
      maxq2 = guinier_maxq2;
   }

   for ( map < QString, vector <double > >::iterator it = guinier_q2.begin();
         it != guinier_q2.end();
         ++it )
   {
      unsigned int q_points = it->second.size();

      for ( unsigned int i = 0; i < q_points; i++ )
      {
         double I = guinier_I [ it->first ][ i ];
         if ( I > 0e0 &&
              guinier_q2[ it->first ][ i ] >= minq2 &&
              guinier_q2[ it->first ][ i ] <= maxq2 )
         {
            if ( minI > I )
            {
               minI = I;
            }
            if ( maxI < I )
            {
               maxI = I;
            }
         }
      }
   }
   
   guinier_range( minq2, maxq2, minI, maxI );
}

void US_Hydrodyn_Mals::guinier_replot()
{
   guinier_curves.clear( );
   guinier_markers.clear( );
   guinier_fit_lines.clear( );
   guinier_errorbar_curves.clear( );
   guinier_plot->detachItems( QwtPlotItem::Rtti_PlotCurve ); guinier_plot->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   guinier_add_marker( guinier_plot,        le_guinier_q2_start  ->text().toDouble(), Qt::red, us_tr( "Start") );
   guinier_add_marker( guinier_plot,        le_guinier_q2_end    ->text().toDouble(), Qt::red, us_tr( "End"  ), Qt::AlignLeft | Qt::AlignTop );
   guinier_add_marker( guinier_plot_errors, le_guinier_q2_start  ->text().toDouble(), Qt::red, us_tr( "Start") );
   guinier_add_marker( guinier_plot_errors, le_guinier_q2_end    ->text().toDouble(), Qt::red, us_tr( "End"  ), Qt::AlignLeft | Qt::AlignTop );

   double minI  = 1e99;
   double maxI  = 0e0;
   double minq2 = le_guinier_q2_start->text().toDouble() - le_guinier_delta_start->text().toDouble();
   double maxq2 = le_guinier_q2_end  ->text().toDouble() + le_guinier_delta_end  ->text().toDouble();

   if ( minq2 < guinier_minq2 )
   {
      minq2 = guinier_minq2;
   }
   if ( maxq2 > guinier_maxq2 )
   {
      maxq2 = guinier_maxq2;
   }

   QwtSymbol sym;
   sym.setStyle(QwtSymbol::Diamond);
   sym.setSize(6);
   sym.setBrush(Qt::white);
   
   bool do_error_bars = true;

   {
      int totpts = 0;
      for ( map < QString, vector <double > >::iterator it = guinier_q2.begin();
            it != guinier_q2.end();
            ++it )
      {
         bool use_error = ( guinier_q[ it->first ].size() == guinier_e[ it->first ].size() );
         if ( use_error )
         {
            totpts += (int) it->second.size();
         }
      }
      if ( totpts > 10000 )
      {
         do_error_bars = false;
      }
   }

   for ( map < QString, vector <double > >::iterator it = guinier_q2.begin();
         it != guinier_q2.end();
         ++it )
   {
      // plot each curve
      sym.setPen( QPen( guinier_colors[ it->first ] ) );
      QwtPlotCurve *curve = new QwtPlotCurve( it->first );
      curve->setStyle ( QwtPlotCurve::NoCurve );
      curve->setSymbol( new QwtSymbol( sym.style(), sym.brush(), QPen( guinier_colors[ it->first ] ), sym.size() ) );
      guinier_curves[ it->first ] = curve;
      unsigned int q_points = it->second.size();

      vector < double > q;
      vector < double > I;
      vector < double > e;
      bool use_error = ( guinier_q[ it->first ].size() == guinier_e[ it->first ].size() );
      for ( unsigned int i = 0; i < q_points; i++ )
      {
         if ( guinier_I [ it->first ][ i ] > 0e0 )
         {
            q.push_back( guinier_q2[ it->first ][ i ] );
            I.push_back( guinier_I [ it->first ][ i ] );
            if ( guinier_q2[ it->first ][ i ] >= minq2 &&
                 guinier_q2[ it->first ][ i ] <= maxq2 )
            {
               if ( minI > I.back() )
               {
                  minI = I.back();
               }
               if ( maxI < I.back() )
               {
                  maxI = I.back();
               }
            }
            if ( use_error )
            {
               e.push_back( guinier_e[ it->first ][ i ] );
            }
         }
      }
      q_points = ( unsigned int )q.size();
      QColor use_qc = guinier_colors[ it->first ];
      curve->setSamples(
                     (double *)&( q[ 0 ] ),
                     (double *)&( I[ 0 ] ),
                     q_points
                     );

      curve->setPen( QPen( use_qc, use_line_width, Qt::SolidLine ) );
      curve->attach( guinier_plot );

      if ( do_error_bars && use_error )
      {
         vector < double > x( 2 );
         vector < double > y( 2 );
         QString ebname = "eb:" + it->first;
         QPen use_pen = QPen( use_qc, use_line_width, Qt::SolidLine );
         for ( int i = 0; i < ( int ) q_points; ++i )
         {
            x[ 0 ] = x[ 1 ] = q[ i ];
            y[ 0 ] = I[ i ] - e[ i ];
            y[ 1 ] = I[ i ] + e[ i ];
            QwtPlotCurve * curve = new QwtPlotCurve( ebname );
            curve->setStyle( QwtPlotCurve::Lines );
            curve->setSamples(
                           (double *)&(x[0]),
                           (double *)&(y[0]),
                           2 );
            curve->setPen( use_pen );
            curve->attach( guinier_plot );
            guinier_errorbar_curves[ it->first ].push_back( curve );
         }
      }
   }        

   guinier_analysis();
   guinier_residuals();
   guinier_range( minq2, maxq2, minI, maxI );
}

void US_Hydrodyn_Mals::guinier_enables()
{
   pb_guinier             -> setEnabled( false );
   pb_errors              -> setEnabled( true );
   pb_wheel_cancel        -> setEnabled( true );
   le_guinier_q_start     -> setEnabled( true ); // !cb_guinier_scroll->isChecked() );
   le_guinier_q_end       -> setEnabled( true ); // !cb_guinier_scroll->isChecked() );
   le_guinier_q2_start    -> setEnabled( true ); // !cb_guinier_scroll->isChecked() );
   le_guinier_q2_end      -> setEnabled( true ); // !cb_guinier_scroll->isChecked() );
   le_guinier_delta_start -> setEnabled( true );
   le_guinier_delta_end   -> setEnabled( true );
   le_guinier_qrgmax      -> setEnabled( true );
   pb_guinier_plot_rg     -> setEnabled( guinier_q.size() > 1 );
   pb_guinier_plot_mw     -> setEnabled( guinier_q.size() > 4 );
   le_guinier_rg_t_start  -> setEnabled( true );
   le_guinier_rg_t_end    -> setEnabled( true );
   le_guinier_rg_rg_start -> setEnabled( true );
   le_guinier_rg_rg_end   -> setEnabled( true );
   le_guinier_mw_t_start  -> setEnabled( true );
   le_guinier_mw_t_end    -> setEnabled( true );
   le_guinier_mw_mw_start -> setEnabled( true );
   le_guinier_mw_mw_end   -> setEnabled( true );
   if ( testiq_active )
   {
      pb_testiq->setEnabled( true );
   }
   pb_pp                  -> setEnabled( true );
   pb_rescale             -> setEnabled( true );
   pb_rescale_y           -> setEnabled( true );

   ShowHide::hide_widgets( wheel_below_widgets, always_hide_widgets, !cb_guinier_scroll->isChecked() );
}

bool US_Hydrodyn_Mals::guinier_check_qmax( bool show_message )
{
   QString report;
   return guinier_check_qmax( report, show_message );
}

void US_Hydrodyn_Mals::check_mwt_constants( bool force ) 
{
   if ( !force && ldata.count( "check_mwt_msg_shown" ) ) {
      return;
   }

   if ( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_mwt_k"    ].toDouble() != 1 ||
        ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_mwt_c"    ].toDouble() != -2.095 ||
        ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_mwt_qmax" ].toDouble() != 0.2 ) {
      QString msg = 
         us_tr( "The MW calculations by the Rambo & Tainer method [Nature 496:477-81] appear to\n"
             "provide consistent results for proteins when using the default values for k, c and qmax.\n"
             "See the Options Help for further details." );

      switch( QMessageBox::warning( this, 
                                    windowTitle(),
                                    msg,
                                    us_tr( "OK" ),
                                    us_tr( "Do not show this warning again" ) )
              ) {
      case 1 : ldata[ "check_mwt_msg_shown" ] = "true";
         break;
      default :
         break;
      }
   }
}

bool US_Hydrodyn_Mals::guinier_check_qmax( QString & report,
                                                bool show_message )
{
   report = "";

   unsigned int file_count    = guinier_names.size();
   unsigned int ok_count      = 0;
   unsigned int missing_count = 0;
   double       min_qmax      = 1e99;
   
   double mwt_qmax =
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_mwt_qmax" ) ?
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_mwt_qmax" ].toDouble() : 0e0;

   for ( int i = 0; i < (int) guinier_names.size(); ++i )
   {
      QString this_file = guinier_names[ i ];
      if ( guinier_q.count( this_file ) &&
           guinier_q[ this_file ].size() ) {
         if ( min_qmax > guinier_q[ this_file ].back() ) {
            min_qmax = guinier_q[ this_file ].back();
         }
         if ( guinier_q[ this_file ].back() + 0.01 >= mwt_qmax ) {
            ok_count++;
         }
            
      } else {
         missing_count++;
      }
   }

   if ( ok_count == file_count ) {
      return true;
   }

   return true;
   // for mals i*q not doing MW[RT]
   
   report = 
      QString( us_tr( 
                  "MW[RT] calculations are calibrated for a qmax of %1\n"
                  "%2 curves of %3 have qmax below calibration qmax\n"
                  "The minimum qmax found in the curves is %4\n" 

                  "See the text area for curve specific details\n" 
                   ) )
      .arg( mwt_qmax )
      .arg( file_count - ok_count )
      .arg( file_count )
      .arg( min_qmax )
      ;

   if ( show_message ) {
      QMessageBox::warning(this, 
                           windowTitle(),
                           report,
                           QMessageBox::Ok | QMessageBox::Default,
                           Qt::NoButton
                           );
   }
   return false;
}

void US_Hydrodyn_Mals::guinier_qrgmax()
{
   disable_all();
   qApp->processEvents();
   guinier_analysis();
   guinier_enables();
}

void US_Hydrodyn_Mals::guinier_qrgmax_text( const QString & )
{
   if ( cb_guinier_qrgmax->isChecked() )
   {
      cb_guinier_qrgmax->setChecked( false );
      guinier_qrgmax();
   }
}

void US_Hydrodyn_Mals::guinier_q_start_text( const QString & text )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( guinier_markers.size() != 4 )
   {
      editor_msg( "red", QString( "internal error: guinier_q_start_text markers issue size %1" ).arg( guinier_markers.size() ) );
      return;
   }
   guinier_markers[ 0 ]->setXValue( text.toDouble() * text.toDouble() );
   guinier_markers[ 2 ]->setXValue( text.toDouble() * text.toDouble() );

   if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
      disconnect( le_guinier_q2_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_guinier_q2_start->setText( QString( "%1" ).arg( text.toDouble() * text.toDouble() ) );
      connect   ( le_guinier_q2_start, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_q2_start_text( const QString & ) ) );
   }
   if ( text.toDouble() > le_guinier_q_end->text().toDouble() )
   {
      le_guinier_q_end->setText( text );
   } else {
      guinier_analysis();
      guinier_residuals();
      guinier_range();
      guinier_plot->replot();
      guinier_plot_errors->replot();
      guinier_enables();
   }
}

#define UHSH_G_WHEEL_RES ( 25e0 * UHSH_WHEEL_RES )

void US_Hydrodyn_Mals::guinier_q_end_text( const QString & text )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( guinier_markers.size() != 4 )
   {
      editor_msg( "red", QString( "internal error: guinier_q_end_text markers issue size %1" ).arg( guinier_markers.size() ) );
      return;
   }
   guinier_markers[ 1 ]->setXValue( text.toDouble() * text.toDouble() );
   guinier_markers[ 3 ]->setXValue( text.toDouble() * text.toDouble() );

   if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
      disconnect( le_guinier_q2_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_guinier_q2_end->setText( QString( "%1" ).arg( text.toDouble() * text.toDouble() ) );
      connect   ( le_guinier_q2_end, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_q2_end_text( const QString & ) ) );
   }
   if ( text.toDouble() < le_guinier_q_start->text().toDouble() )
   {
      le_guinier_q_start->setText( text );
   } else {
      guinier_analysis();
      guinier_residuals();
      guinier_range();
      guinier_plot->replot();
      guinier_plot_errors->replot();
      guinier_enables();
   }
}

void US_Hydrodyn_Mals::guinier_q_start_focus( bool hasFocus )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( hasFocus )
   {
      le_last_focus = le_guinier_q_start;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( guinier_minq, guinier_maxq); qwtw_wheel->setSingleStep( ( guinier_maxq - guinier_minq ) / UHSH_G_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_guinier_q_start->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals::guinier_q_end_focus( bool hasFocus )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( hasFocus )
   {
      le_last_focus = le_guinier_q_end;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( guinier_minq, guinier_maxq); qwtw_wheel->setSingleStep( ( guinier_maxq - guinier_minq ) / UHSH_G_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_guinier_q_end->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals::guinier_q2_start_text( const QString & text )
{
   // TSO << QString( "guinier_q2_start_text( %1 )\n" ).arg( text );
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( guinier_markers.size() != 4 )
   {
      editor_msg( "red", QString( "internal error: guinier_q2_start_text markers issue size %1" ).arg( guinier_markers.size() ) );
      return;
   }
   guinier_markers[ 0 ]->setXValue( text.toDouble() );
   guinier_markers[ 2 ]->setXValue( text.toDouble() );

   if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() ) {
      qwtw_wheel->setValue( text.toDouble() );
   }

   disconnect( le_guinier_q_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   le_guinier_q_start->setText( QString( "%1" ).arg( sqrt( text.toDouble() ) ) );
   connect   ( le_guinier_q_start, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_q_start_text( const QString & ) ) );

   if ( text.toDouble() > le_guinier_q2_end->text().toDouble() )
   {
      le_guinier_q2_end->setText( text );
   } else {
      // TSO << QString( "guinier_q2_start_text() calls guinier_analysis()\n" );
      guinier_analysis();
      guinier_residuals();
      guinier_range();
      guinier_plot->replot();
      guinier_plot_errors->replot();
      guinier_enables();
   }
}

void US_Hydrodyn_Mals::guinier_q2_end_text( const QString & text )
{
   // TSO << QString( "guinier_q2_end_text( %1 )\n" ).arg( text );
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( guinier_markers.size() != 4 )
   {
      editor_msg( "red", QString( "internal error: guinier_q2_end_text markers issue size %1" ).arg( guinier_markers.size() ) );
      return;
   }
   guinier_markers[ 1 ]->setXValue( text.toDouble() );
   guinier_markers[ 3 ]->setXValue( text.toDouble() );
   if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() ) {
      qwtw_wheel->setValue( text.toDouble() );
   }

   disconnect( le_guinier_q_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   le_guinier_q_end->setText( QString( "%1" ).arg( sqrt( text.toDouble() ) ) );
   connect   ( le_guinier_q_end, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_q_end_text( const QString & ) ) );

   if ( text.toDouble() < le_guinier_q2_start->text().toDouble() )
   {
      le_guinier_q2_start->setText( text );
   } else {
      // TSO << QString( "guinier_q2_end_text() calls guinier_analysis()\n" );
      guinier_analysis();
      guinier_residuals();
      guinier_range();
      guinier_plot->replot();
      guinier_plot_errors->replot();
      guinier_enables();
   }
}

void US_Hydrodyn_Mals::guinier_q2_start_focus( bool hasFocus )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( hasFocus )
   {
      le_last_focus = le_guinier_q2_start;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( guinier_minq2, guinier_maxq2); qwtw_wheel->setSingleStep( ( guinier_maxq2 - guinier_minq2 ) / UHSH_G_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_guinier_q2_start->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals::guinier_q2_end_focus( bool hasFocus )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( hasFocus )
   {
      le_last_focus = le_guinier_q2_end;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( guinier_minq2, guinier_maxq2); qwtw_wheel->setSingleStep( ( guinier_maxq2 - guinier_minq2 ) / UHSH_G_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_guinier_q2_end->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals::guinier_delta_start_text( const QString & text )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }

   guinier_residuals();
   guinier_range();
   guinier_plot->replot();
   guinier_plot_errors->replot();
   guinier_enables();
}

void US_Hydrodyn_Mals::guinier_delta_end_text( const QString & text )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }

   guinier_residuals();
   guinier_range();
   guinier_plot->replot();
   guinier_plot_errors->replot();
   guinier_enables();
}

void US_Hydrodyn_Mals::guinier_delta_start_focus( bool hasFocus )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( hasFocus )
   {
      le_last_focus = le_guinier_delta_start;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( 0e0, guinier_maxq2); qwtw_wheel->setSingleStep( guinier_maxq2 / UHSH_G_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_guinier_delta_start->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals::guinier_delta_end_focus( bool hasFocus )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( hasFocus )
   {
      le_last_focus = le_guinier_delta_end;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( 0e0, guinier_maxq2); qwtw_wheel->setSingleStep( guinier_maxq2 / UHSH_G_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_guinier_delta_end->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals::guinier_rg_t_start_text( const QString & text )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   guinier_plot_rg->setAxisScale( QwtPlot::xBottom, text.toDouble(), le_guinier_rg_t_end->text().toDouble() );
   guinier_plot_rg->replot();
   if ( guinier_plot_rg_zoomer )
   {
      delete guinier_plot_rg_zoomer;
   }
   guinier_plot_rg_zoomer = new ScrollZoomer(guinier_plot_rg->canvas());
   guinier_plot_rg_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
   guinier_plot_rg_zoomer->setTrackerPen(QPen(Qt::red));
}

void US_Hydrodyn_Mals::guinier_rg_t_end_text( const QString & text )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   guinier_plot_rg->setAxisScale( QwtPlot::xBottom, le_guinier_rg_t_start->text().toDouble(), text.toDouble() );
   guinier_plot_rg->replot();
   if ( guinier_plot_rg_zoomer )
   {
      delete guinier_plot_rg_zoomer;
   }
   guinier_plot_rg_zoomer = new ScrollZoomer(guinier_plot_rg->canvas());
   guinier_plot_rg_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
   guinier_plot_rg_zoomer->setTrackerPen(QPen(Qt::red));
}

void US_Hydrodyn_Mals::guinier_rg_t_start_focus( bool hasFocus )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( hasFocus )
   {
      le_last_focus = le_guinier_rg_t_start;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( guinier_mint, guinier_maxt); qwtw_wheel->setSingleStep( ( guinier_maxt - guinier_mint ) / UHSH_G_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_guinier_rg_t_start->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals::guinier_rg_t_end_focus( bool hasFocus )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( hasFocus )
   {
      le_last_focus = le_guinier_rg_t_end;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( guinier_mint, guinier_maxt); qwtw_wheel->setSingleStep( ( guinier_maxt - guinier_mint ) / UHSH_G_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_guinier_rg_t_end->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals::guinier_rg_rg_start_text( const QString & text )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   guinier_plot_rg->setAxisScale( QwtPlot::yLeft, text.toDouble(), le_guinier_rg_rg_end->text().toDouble() );
   guinier_plot_rg->replot();
   if ( guinier_plot_rg_zoomer )
   {
      delete guinier_plot_rg_zoomer;
   }
   guinier_plot_rg_zoomer = new ScrollZoomer(guinier_plot_rg->canvas());
   guinier_plot_rg_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
   guinier_plot_rg_zoomer->setTrackerPen(QPen(Qt::red));
}

void US_Hydrodyn_Mals::guinier_rg_rg_end_text( const QString & text )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   guinier_plot_rg->setAxisScale( QwtPlot::yLeft, le_guinier_rg_rg_start->text().toDouble(), text.toDouble() );
   guinier_plot_rg->replot();
   if ( guinier_plot_rg_zoomer )
   {
      delete guinier_plot_rg_zoomer;
   }
   guinier_plot_rg_zoomer = new ScrollZoomer(guinier_plot_rg->canvas());
   guinier_plot_rg_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
   guinier_plot_rg_zoomer->setTrackerPen(QPen(Qt::red));
}

#define UHSH_MAX_RG 1000

void US_Hydrodyn_Mals::guinier_rg_rg_start_focus( bool hasFocus )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( hasFocus )
   {
      le_last_focus = le_guinier_rg_rg_start;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( 0, UHSH_MAX_RG); qwtw_wheel->setSingleStep( UHSH_MAX_RG  / UHSH_G_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_guinier_rg_rg_start->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals::guinier_rg_rg_end_focus( bool hasFocus )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( hasFocus )
   {
      le_last_focus = le_guinier_rg_rg_end;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( 0, UHSH_MAX_RG); qwtw_wheel->setSingleStep( UHSH_MAX_RG  / UHSH_G_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_guinier_rg_rg_end->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals::guinier_mw_t_start_text( const QString & text )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   guinier_plot_mw->setAxisScale( QwtPlot::xBottom, text.toDouble(), le_guinier_mw_t_end->text().toDouble() );
   guinier_plot_mw->replot();
   if ( guinier_plot_mw_zoomer )
   {
      delete guinier_plot_mw_zoomer;
   }
   guinier_plot_mw_zoomer = new ScrollZoomer(guinier_plot_mw->canvas());
   guinier_plot_mw_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
   guinier_plot_mw_zoomer->setTrackerPen(QPen(Qt::red));
}

void US_Hydrodyn_Mals::guinier_mw_t_end_text( const QString & text )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   guinier_plot_mw->setAxisScale( QwtPlot::xBottom, le_guinier_mw_t_start->text().toDouble(), text.toDouble() );
   guinier_plot_mw->replot();
   if ( guinier_plot_mw_zoomer )
   {
      delete guinier_plot_mw_zoomer;
   }
   guinier_plot_mw_zoomer = new ScrollZoomer(guinier_plot_mw->canvas());
   guinier_plot_mw_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
   guinier_plot_mw_zoomer->setTrackerPen(QPen(Qt::red));
}

void US_Hydrodyn_Mals::guinier_mw_t_start_focus( bool hasFocus )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( hasFocus )
   {
      le_last_focus = le_guinier_mw_t_start;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( guinier_mint, guinier_maxt); qwtw_wheel->setSingleStep( ( guinier_maxt - guinier_mint ) / UHSH_G_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_guinier_mw_t_start->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals::guinier_mw_t_end_focus( bool hasFocus )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( hasFocus )
   {
      le_last_focus = le_guinier_mw_t_end;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( guinier_mint, guinier_maxt); qwtw_wheel->setSingleStep( ( guinier_maxt - guinier_mint ) / UHSH_G_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_guinier_mw_t_end->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals::guinier_mw_mw_start_text( const QString & text )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   guinier_plot_mw->setAxisScale( QwtPlot::yLeft, text.toDouble(), le_guinier_mw_mw_end->text().toDouble() );
   guinier_plot_mw->replot();
   if ( guinier_plot_mw_zoomer )
   {
      delete guinier_plot_mw_zoomer;
   }
   guinier_plot_mw_zoomer = new ScrollZoomer(guinier_plot_mw->canvas());
   guinier_plot_mw_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
   guinier_plot_mw_zoomer->setTrackerPen(QPen(Qt::red));
}

void US_Hydrodyn_Mals::guinier_mw_mw_end_text( const QString & text )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   guinier_plot_mw->setAxisScale( QwtPlot::yLeft, le_guinier_mw_mw_start->text().toDouble(), text.toDouble() );
   guinier_plot_mw->replot();
   if ( guinier_plot_mw_zoomer )
   {
      delete guinier_plot_mw_zoomer;
   }
   guinier_plot_mw_zoomer = new ScrollZoomer(guinier_plot_mw->canvas());
   guinier_plot_mw_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
   guinier_plot_mw_zoomer->setTrackerPen(QPen(Qt::red));
}

#define UHSH_MAX_MW 1e7

void US_Hydrodyn_Mals::guinier_mw_mw_start_focus( bool hasFocus )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( hasFocus )
   {
      le_last_focus = le_guinier_mw_mw_start;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( 0, UHSH_MAX_MW); qwtw_wheel->setSingleStep( UHSH_MAX_MW  / UHSH_G_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_guinier_mw_mw_start->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals::guinier_mw_mw_end_focus( bool hasFocus )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( hasFocus )
   {
      le_last_focus = le_guinier_mw_mw_end;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( 0, UHSH_MAX_MW); qwtw_wheel->setSingleStep( UHSH_MAX_MW  / UHSH_G_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_guinier_mw_mw_end->text().toDouble() );
      wheel_enables();
   }
}
