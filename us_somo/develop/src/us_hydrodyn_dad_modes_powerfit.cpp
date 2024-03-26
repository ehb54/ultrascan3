#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_mals_saxs.h"
#include "../include/us_hydrodyn_mals_saxs_scale_trend.h"
#include "../include/us_pm.h"
#include <QPixmap>
#include "../include/us_eigen.h"

static US_Eigen               eigen;
static bool                   minimize_running;
static map < double, double > last_minimize_chi2s;

#define TSO QTextStream(stdout)

// --- powerfits ---

void US_Hydrodyn_Dad::powerfit( bool no_store_original )
{
   TSO << "powerfit()\n";
   le_last_focus = (mQLineEdit *) 0;
   minimize_running = false;

   // verify selected files and setup powerfit_names
   // must match time and one must end in _common
   // all istar or ihashq already verfied by _gui

   disable_all();

   powerfit_names        .clear();
   powerfit_name         = "";
   powerfit_org_selected .clear();

   QStringList names = all_selected_files();

   if ( names.size() != 1 ) {
      QMessageBox::critical( this,
                             windowTitle() + us_tr( ": Powerfit" ),
                             QString( us_tr(
                                            "%1 curves selected, Powerfit requires exactly one curve selected\n"
                                            )
                                      )
                             .arg( names.size() )
                             );
      return update_enables();
   }
   
   powerfit_org_selected = all_selected_files_set();
   powerfit_name         = names[0];

   // using errors?

   powerfit_use_errors =
      f_errors.count( powerfit_name )
      && f_qs.count( powerfit_name )
      && f_errors[ powerfit_name ].size() == f_qs[ powerfit_name ].size()
      && !is_zero_vector( f_errors[ powerfit_name ] )
      ;

   // move to _gui   lbl_powerfit_q_range->setText( us_tr( " Fit range 1 (fixed): " );
   
   mode_select( MODE_POWERFIT );

   running       = true;

   // set wheel range etc.

   // set le_powerfit_q1,2 start,end to grid limits
   powerfit_markers.clear();

   powerfit_add_marker( plot_dist, le_powerfit_q_start  ->text().toDouble(), powerfit_color_q, us_tr( "Start"), Qt::AlignRight | Qt::AlignBottom );
   powerfit_add_marker( plot_dist, le_powerfit_q_end    ->text().toDouble(), powerfit_color_q, us_tr( "End"  ), Qt::AlignLeft  | Qt::AlignBottom );

   powerfit_q_min = f_qs[ powerfit_name ].front();
   powerfit_q_max = f_qs[ powerfit_name ].back();

   qDebug() << "powerfit range " << powerfit_q_min << " " << powerfit_q_max;

   if (
       le_powerfit_q_start->text().isEmpty() 
       || le_powerfit_q_start->text().toDouble() < powerfit_q_min
       || le_powerfit_q_start->text().toDouble() > powerfit_q_max
       ) {
      le_powerfit_q_start->setText( QString( "%1" ).arg( powerfit_q_min ) );
      qDebug() << "powerfit q start set " << powerfit_q_min;
   }
   if (
       le_powerfit_q_end->text().isEmpty() 
       || le_powerfit_q_end->text().toDouble() > powerfit_q_max
       || le_powerfit_q_end->text().toDouble() <= powerfit_q_min + 0.001 // == caused by the slot, +0.001 for rounding
       ) {
      le_powerfit_q_end  ->setText( QString( "%1" ).arg( powerfit_q_max ) );
      qDebug() << "powerfit q end set " << powerfit_q_max;
   } else {
      qDebug() << "powerfit q end NOT set, value " << le_powerfit_q_end->text();
   }

   lbl_wheel_pos->setText("");
   powerfit_enables();
}

void US_Hydrodyn_Dad::powerfit_enables()
{
   // qDebug() << "powerfit_enables() last_minimize_scale " << last_minimize_scale << " powerfit_scale: " << le_powerfit_scale->text().toDouble();
   // mostly disable except cancel & wheel
   if ( minimize_running ) {
      TSO << "powerfit_enables() disabled!\n";
      return;
   }
   TSO << "powerfit_enables()\n";
   
   qwtw_wheel                           ->setEnabled( true );
   pb_wheel_cancel                      ->setEnabled( true );
   pb_wheel_save                        ->setEnabled( false );
   pb_axis_x                            ->setEnabled( true );
   pb_axis_y                            ->setEnabled( true );
   cb_eb                                ->setEnabled( true );
   cb_dots                              ->setEnabled( true );
   pb_color_rotate                      ->setEnabled( true );
   pb_line_width                        ->setEnabled( true );
   pb_q_exclude_vis                     ->setEnabled( true );
   pb_q_exclude_left                    ->setEnabled( true );
   pb_q_exclude_right                   ->setEnabled( true );
   pb_q_exclude_clear                   ->setEnabled( q_exclude.size() > 0 );
   pb_pp                                ->setEnabled( true );
   
   pb_powerfit_fit                      ->setEnabled( true );
   le_powerfit_q_start                  ->setEnabled( true );
   le_powerfit_q_end                    ->setEnabled( true );

   le_powerfit_a                        ->setEnabled( true );
   le_powerfit_b                        ->setEnabled( true );
   le_powerfit_c                        ->setEnabled( true );

   cb_powerfit_fit_curve                ->setEnabled( true );
   cb_powerfit_fit_alg                  ->setEnabled( true );
   cb_powerfit_fit_alg_weight           ->setEnabled( true );

   wheel_enables( true );

   progress->reset();
}

void US_Hydrodyn_Dad::powerfit_scroll_highlight( int pos )
{
   qDebug() << "powerfit_scroll_highlight " << pos;
}

void US_Hydrodyn_Dad::powerfit_add_marker( 
                                          QwtPlot * plot,
                                          double pos, 
                                          QColor color, 
                                          QString text, 
                                          Qt::Alignment
                                          align ) {
   QwtPlotMarker * marker = new QwtPlotMarker;
   marker->setLineStyle        ( QwtPlotMarker::VLine );
   marker->setLinePen          ( QPen( color, 2, Qt::DashDotDotLine ) );
   marker->setLabelOrientation ( Qt::Horizontal );
   marker->setXValue           ( pos );
   marker->setLabelAlignment   ( align );
   {
      QwtText qwtt             ( text );
      qwtt.setFont             ( QFont("Helvetica", 11, QFont::Bold ) );
      marker->setLabel         ( qwtt );
   }
   marker->attach              ( plot );
   powerfit_markers.push_back( marker );
}   

void US_Hydrodyn_Dad::powerfit_delete_markers() {
   plot_dist   ->detachItems( QwtPlotItem::Rtti_PlotMarker );
   powerfit_markers.clear();
}

#define UHMS_SCALE_WHEEL_RES 10000
// #define UHMS_SCALE_WHEEL_RES 1e-6

void US_Hydrodyn_Dad::powerfit_q_start_text( const QString & text ) {
   TSO << QString( "powerfit_q_start_text( %1 )\n" ).arg( text );
   if ( current_mode != MODE_POWERFIT ) {
      return;
   }
   if ( powerfit_markers.size() != 2 ) {
      editor_msg( "red", QString( "internal error: powerfit_q_start_text markers issue size %1" ).arg( powerfit_markers.size() ) );
      return;
   }
   
   powerfit_fit_clear( false );

   lbl_wheel_pos->setText( text );
   powerfit_markers[ 0 ]->setXValue( text.toDouble() );
   plot_dist->replot();

   if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() ) {
      qwtw_wheel->setValue( text.toDouble() );
   }

   disconnect( le_powerfit_q_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   le_powerfit_q_start->setText( QString( "%1" ).arg( text.toDouble() ) );
   connect   ( le_powerfit_q_start, SIGNAL( textChanged( const QString & ) ), SLOT( powerfit_q_start_text( const QString & ) ) );

   if ( text.toDouble() > le_powerfit_q_end->text().toDouble() ) {
      le_powerfit_q_end->setText( text );
   } else {
      TSO << QString( "powerfit_q_start_text() calls any analysis etc()\n" );
   }
}

void US_Hydrodyn_Dad::powerfit_q_end_text( const QString & text ) {
   // TSO << QString( "powerfit_q_end_text( %1 )\n" ).arg( text );
   if ( current_mode != MODE_POWERFIT ) {
      return;
   }
   if ( powerfit_markers.size() != 2 ) {
      editor_msg( "red", QString( "internal error: powerfit_q_end_text markers issue size %1" ).arg( powerfit_markers.size() ) );
      return;
   }

   powerfit_fit_clear( false );

   lbl_wheel_pos->setText( text );
   powerfit_markers[ 1 ]->setXValue( text.toDouble() );
   plot_dist->replot();

   if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() ) {
      qwtw_wheel->setValue( text.toDouble() );
   }

   disconnect( le_powerfit_q_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   le_powerfit_q_end->setText( QString( "%1" ).arg( text.toDouble() ) );
   connect   ( le_powerfit_q_end, SIGNAL( textChanged( const QString & ) ), SLOT( powerfit_q_end_text( const QString & ) ) );

   if ( text.toDouble() < le_powerfit_q_start->text().toDouble() ) {
      le_powerfit_q_start->setText( text );
   } else {
      TSO << QString( "powerfit_q_end_text() calls any analysis etc\n" );
   }
}

void US_Hydrodyn_Dad::powerfit_q_start_focus( bool hasFocus ) {
   qDebug() << "powerfit_q_start_focus()";
   if ( current_mode != MODE_POWERFIT ) {
      return;
   }
   if ( hasFocus ) {
      le_last_focus = le_powerfit_q_start;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( powerfit_q_min, powerfit_q_max);
      qwtw_wheel->setSingleStep( ( powerfit_q_max - powerfit_q_min ) / UHMS_SCALE_WHEEL_RES );
      // qwtw_wheel->setSingleStep( UHMS_SCALE_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_powerfit_q_start->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Dad::powerfit_q_end_focus( bool hasFocus )
{
   qDebug() << "powerfit_q_end_focus()";
   if ( current_mode != MODE_POWERFIT ) {
      return;
   }
   if ( hasFocus )
   {
      le_last_focus = le_powerfit_q_end;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( powerfit_q_min, powerfit_q_max);
      qwtw_wheel->setSingleStep( ( powerfit_q_max - powerfit_q_min ) / UHMS_SCALE_WHEEL_RES );
      // qwtw_wheel->setSingleStep( UHMS_SCALE_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_powerfit_q_end->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Dad::powerfit_a_text( const QString & text ) {
   qDebug() << "powerfit_a_text";
   powerfit_fit_clear();
}

void US_Hydrodyn_Dad::powerfit_a_focus( bool /* hasFocus */ ) {
   qDebug() << "powerfit_a_focus";
}

void US_Hydrodyn_Dad::powerfit_b_text( const QString & text ) {
   qDebug() << "powerfit_b_text";
   powerfit_fit_clear();
}

void US_Hydrodyn_Dad::powerfit_b_focus( bool /* hasFocus */ ) {
   qDebug() << "powerfit_a_focus";
}

void US_Hydrodyn_Dad::powerfit_c_text( const QString & text ) {
   qDebug() << "powerfit_c_text";
   powerfit_fit_clear();
}

void US_Hydrodyn_Dad::powerfit_c_focus( bool /* hasFocus */ ) {
   qDebug() << "powerfit_a_focus";
}

void US_Hydrodyn_Dad::powerfit_fit() {
   qDebug() << "powerfit_fit()";
   disable_all();
#warning qwtw_wheel probably should be in ::disable_all() but for some reason we commented it out there, probably should check all wheel usage and set as appropriate
   qwtw_wheel            ->setEnabled( false );
   powerfit_fit_clear();

   // collect q grid positions

   set < double > qs;

   double q_min = le_powerfit_q_start->text().toDouble();
   double q_max = le_powerfit_q_end  ->text().toDouble();

   for ( auto const & q : f_qs[ powerfit_name ] ) {
      if ( q >= q_min && q <= q_max && !q_exclude.count( q ) ) {
         qs.insert( q );
      }
   }
   
   if ( !qs.size() ) {
      QMessageBox::critical( this,
                             windowTitle() + us_tr( ": Powerfit" ),
                             us_tr(
                                   "There are no non-excluded points selected for fitting!"
                                   )
                             );
      return powerfit_enables();
   }

   int degree;

   {
      int minimum_pts_req = 2;
      switch ( (powerfit_fit_curves) cb_powerfit_fit_curve->currentData().toInt() ) {
      case POWERFIT_FIT_CURVE_P2 :
         minimum_pts_req = 3;
         degree          = 2;
         break;
      case POWERFIT_FIT_CURVE_P3 :
         minimum_pts_req = 4;
         degree          = 3;
         break;
      case POWERFIT_FIT_CURVE_P4 :
         minimum_pts_req = 5;
         degree          = 4;
         break;
      case POWERFIT_FIT_CURVE_P5 :
         minimum_pts_req = 6;
         degree          = 5;
         break;
      case POWERFIT_FIT_CURVE_P6 :
         minimum_pts_req = 7;
         degree          = 6;
         break;
      case POWERFIT_FIT_CURVE_P7 :
         minimum_pts_req = 8;
         degree          = 7;
         break;
      case POWERFIT_FIT_CURVE_P8 :
         minimum_pts_req = 9;
         degree          = 8;
         break;
      default:
         QMessageBox::critical( this,
                                windowTitle() + us_tr( ": Powerfit" ),
                                us_tr(
                                      "Internal error : unexpected fit curve"
                                      )
                                );
         return powerfit_enables();
         break;
      }

      if ( (int) qs.size() < minimum_pts_req ) {
         QMessageBox::critical( this,
                                windowTitle() + us_tr( ": Powerfit" ),
                                QString( us_tr(
                                               "There are insufficient (%1) non-excluded points selected.\n"
                                               "The current fitting method requires a minimum of %2 points."
                                               )
                                         )
                                .arg( qs.size() )
                                .arg( minimum_pts_req )
                                );
         return powerfit_enables();
      }
   }


   vector < double > q;
   vector < double > I;
   vector < double > e;

   vector < double > coeff;
   vector < double > x;
   vector < double > y;
   double            chi2;

   bool use_errors;

   switch( (US_Eigen::weight_methods) cb_powerfit_fit_alg_weight->currentData().toInt() ) {
   case US_Eigen::EIGEN_NO_WEIGHTS :
   case US_Eigen::EIGEN_1_OVER_AMOUNT :
   case US_Eigen::EIGEN_1_OVER_AMOUNT_SQ :
      use_errors = false;
      break;
      
   case US_Eigen::EIGEN_1_OVER_SD :
   case US_Eigen::EIGEN_1_OVER_SD_SQ :
      use_errors = true;
      break;
   default:
      qDebug() << "powerfit_fit_at_time eigen weight error";
      use_errors = false;
      break;
   }
   
   /* blocked out for now

      if ( powerfit_use_errors && use_errors ) {
      double powerfit_sd_scale = le_powerfit_sd_scale->text().toDouble();
      for ( int i = 0; i < (int) powerfit_qgrids[ powerfit_mals_set ].size(); ++i ) {
      if ( q1.count( powerfit_qgrids[ powerfit_mals_set ][ i ] ) ) {
      q.push_back( powerfit_qgrids[ powerfit_mals_set ][ i ] );
      I.push_back( f_Is[ name1 ][ i ] * powerfit_scale );
      e.push_back( f_errors[ name1 ][ i ] * powerfit_scale * powerfit_sd_scale );
      }
      }
   
      for ( int i = 0; i < (int) powerfit_qgrids[ powerfit_saxs_set ].size(); ++i ) {
      if ( q2.count( powerfit_qgrids[ powerfit_saxs_set ][ i ] ) ) {
      q.push_back( powerfit_qgrids[ powerfit_saxs_set ][ i ] );
      I.push_back( f_Is[ name2 ][ i ] );
      e.push_back( f_errors[ name2 ][ i ] );
      }
      }
   
      // US_Vector::printvector3( "data for fitting, q,I,e", q, I, e );
      // us_timers.start_timer( "polyfit" );
      eigen.polyfit( q, I, e, degree, coeff, chi2
      ,(US_Eigen::fit_methods)    cb_powerfit_fit_alg->currentData().toInt() 
      ,(US_Eigen::weight_methods) cb_powerfit_fit_alg_weight->currentData().toInt()
      );
      // us_timers.end_timer( "polyfit" );
      } else {
      for ( int i = 0; i < (int) powerfit_qgrids[ powerfit_mals_set ].size(); ++i ) {
      if ( q1.count( powerfit_qgrids[ powerfit_mals_set ][ i ] ) ) {
      q.push_back( powerfit_qgrids[ powerfit_mals_set ][ i ] );
      I.push_back( f_Is[ name1 ][ i ] * powerfit_scale );
      }
      }
   
      for ( int i = 0; i < (int) powerfit_qgrids[ powerfit_saxs_set ].size(); ++i ) {
      if ( q2.count( powerfit_qgrids[ powerfit_saxs_set ][ i ] ) ) {
      q.push_back( powerfit_qgrids[ powerfit_saxs_set ][ i ] );
      I.push_back( f_Is[ name2 ][ i ] );
      }
      }
   
      // US_Vector::printvector2( "data for fitting, q,I", q, I );
      // us_timers.start_timer( "polyfit" );
      eigen.polyfit( q, I, degree, coeff, chi2
      ,(US_Eigen::fit_methods)    cb_powerfit_fit_alg->currentData().toInt() 
      ,(US_Eigen::weight_methods) cb_powerfit_fit_alg_weight->currentData().toInt()
      );
      // us_timers.end_timer( "polyfit" );
      }      

      // us_qdebug( us_timers.list_times() );

      lbl_powerfit_msg->setText( QString( "nChi^2 %1 average %2 (%3 curves)" )
      .arg( chi2 / q.size() )
      .arg( global_chi2 / ( q.size() * powerfit_times.size() ) )
      .arg( powerfit_times.size() )
      );
      eigen.evaluate_polynomial( coeff, q_min, q2_max, 100, x, y );
      US_Vector::printvector( "coefficients", coeff );
      // US_Vector::printvector2( "fitting curve", x, y );

      {
      powerfit_fit_clear( false );
         
      if ( axis_y_log ) {
      vector < double > trimmed_x;
      vector < double > trimmed_y;

      for ( size_t i = 0; i < x.size(); ++i ) {
      if ( y[i] > 0 ) {
      trimmed_x.push_back( x[i] );
      trimmed_y.push_back( y[i] );
      }
      }

      x = trimmed_x;
      y = trimmed_y;
      }

      powerfit_fit_curve = new QwtPlotCurve( "scale_fit" );
      powerfit_fit_curve->setStyle( QwtPlotCurve::Lines );
      powerfit_fit_curve->setSamples(
      (double *)&( x[ 0 ] ),
      (double *)&( y[ 0 ] ),
      x.size() );
      powerfit_fit_curve->setPen( QPen( QColor( Qt::red ), use_line_width, Qt::SolidLine ) );
      powerfit_fit_curve->attach( plot_dist );
      plot_dist->replot();
      }

   */
   
   return powerfit_enables();
}

// qt doc says int argument for the signal, but actually QString, just dropping it

void US_Hydrodyn_Dad::powerfit_fit_alg_index() {
   qDebug() << "powerfit_fit_alg_index()";
   powerfit_fit_clear();
};

void US_Hydrodyn_Dad::powerfit_fit_alg_weight_index() {
   qDebug() << "powerfit_fit_alg_weight_index()";
   powerfit_fit_clear();
}

void US_Hydrodyn_Dad::powerfit_fit_curve_index() {
   qDebug() << "powerfit_fit_curve_index()";
   powerfit_fit_clear();
}
void US_Hydrodyn_Dad::powerfit_create_adjusted_curve() {
   qDebug() << "powerfit_create_adjusted_curve()";
}

void US_Hydrodyn_Dad::powerfit_fit_clear( bool replot ) {
   qDebug() << "powerfit_fit_clear()";
   if ( powerfit_fit_curve ) {
      qDebug() << "powerfit_fit_clear() powerfit_fit_curve set";
      powerfit_fit_curve->detach();
      delete powerfit_fit_curve;
      lbl_powerfit_msg->setText("");
      powerfit_fit_curve = (QwtPlotCurve *) 0;
      if ( replot ) {
         plot_dist->replot();
      }
   }
   if ( powerfit_corrected_curve ) {
      qDebug() << "powerfit_fit_clear() powerfit_corrected_curve set";
      powerfit_corrected_curve->detach();
      delete powerfit_corrected_curve;
      lbl_powerfit_msg->setText("");
      powerfit_corrected_curve = (QwtPlotCurve *) 0;
      if ( replot ) {
         plot_dist->replot();
      }
   }
}

