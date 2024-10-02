#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_mals_saxs.h"
#include "../include/us_hydrodyn_mals_saxs_scale_trend.h"
#include "../include/us_pm.h"
#include <QPixmap>
#include "../include/us_eigen.h"
#include "../include/us_lm.h"

static bool                   minimize_running;
static double                 powerfit_c_min;
static double                 powerfit_c_max;
static vector < double >      fit_weights;
static map < double, size_t > fit_weights_index;
static vector < double >      powerfit_corrected_data;
static vector < double >      powerfit_corrected_data_errors;
static double                 powerfit_uncorrected_conc;
static double                 powerfit_scat_conc;
static double                 powerfit_conc;
static map < double, double > powerfit_org_I_at_lambda;
static set < QString >        powerfit_tmp_plotnames;
static bool                   powerfit_corrected_data_ok;
static QString                powerfit_last_used_name;

// static double                 powerfit_dndc2_a = 0.410854315;
// static double                 powerfit_dndc2_b = 2.857579907;
// static double                 powerfit_dndc2_c = -0.81436267;

// static double                 powerfit_n2_a    = 1.151964091;
// static double                 powerfit_n2_b    = 4619.756721;
// static double                 powerfit_n2_c    = -2.22444448;

static map < double, double > powerfit_corrected_lambda;

#define TSO QTextStream(stdout)

// --- powerfit ---

void US_Hydrodyn_Dad::powerfit_corrected_lambda_create( const vector < double > & x ) {

   powerfit_corrected_lambda.clear();

   for ( const auto & v : x ) {
      double dndc2_correction =
         dad_param_dndc2_a
         + dad_param_dndc2_b
         * pow( v, dad_param_dndc2_c )
         ;
      double n2_correction =
         dad_param_n2_a
         + dad_param_n2_b
         * pow( v, dad_param_n2_c )
         ;

      powerfit_corrected_lambda[ v ] = v * pow( dndc2_correction * n2_correction, -1 );

      TSO <<
         QString( "lambda %1 --> %2\n" )
         .arg( v )
         .arg( powerfit_corrected_lambda[ v ] )
         ;

      // TSO <<
      //    QString( "lambda %1 corrected %2 (dndc factor %3, n2 factor %4)\n" )
      //    .arg( v )
      //    .arg( v * powerfit_corrected_lambda[ v ] )
      //    .arg( dndc2_correction )
      //    .arg( n2_correction )
      //    ;
   }
}   

void US_Hydrodyn_Dad::powerfit( bool /* no_store_original */ )
{
   TSO << "powerfit()\n";
   le_last_focus = (mQLineEdit *) 0;
   minimize_running = false;

   // verify selected files and setup powerfit_names
   // must match time and one must end in _common
   // all istar or ihashq already verfied by _gui

   disable_all();

   powerfit_names              .clear();
   powerfit_name               = "";
   powerfit_org_selected       .clear();
   powerfit_org_I_at_lambda    .clear();
   powerfit_created_name       .clear();
   powerfit_tmp_plotnames      .clear();
   le_powerfit_uncorrected_conc->setText( "" );
   le_powerfit_scat_conc       ->setText( "" );
   le_powerfit_computed_conc   ->setText( "" );
   le_powerfit_lambda_abs      ->setText( "" );
   le_powerfit_lambda2_abs     ->setText( "" );
   powerfit_b_default          = "1";
   powerfit_corrected_data_ok  = false;

   le_powerfit_lambda          ->setText( QString( "%1" ).arg( dad_param_lambda ) );
   le_powerfit_extinction_coef ->setText( QString( "%1" ).arg( dad_param_g_extinction_coef ) );

   QStringList names           = all_selected_files();

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

   for ( size_t i = 0; i < f_qs[ powerfit_name ].size(); ++i ) {
      powerfit_org_I_at_lambda[ QString( "%1" ).arg( f_qs[ powerfit_name ][ i ] ).toDouble() ] = f_Is[ powerfit_name ][ i ];
   }

   // using errors?

   powerfit_use_errors =
      f_errors.count( powerfit_name )
      && f_qs.count( powerfit_name )
      && f_errors[ powerfit_name ].size() == f_qs[ powerfit_name ].size()
      && !is_zero_vector( f_errors[ powerfit_name ] )
      ;

   if ( !powerfit_use_errors &&
        ( 
         (US_Eigen::weight_methods) cb_powerfit_fit_alg_weight->currentData().toInt() == US_Eigen::EIGEN_1_OVER_SD
         || (US_Eigen::weight_methods) cb_powerfit_fit_alg_weight->currentData().toInt() == US_Eigen::EIGEN_1_OVER_SD_SQ
          ) ) {
      cb_powerfit_fit_alg_weight->setCurrentIndex( 0 ); // US_Eigen::EIGEN_NO_WEIGHTS
   }

   // move to _gui   lbl_powerfit_q_range->setText( us_tr( " Fit range 1 (fixed): " );
   
   

   powerfit_corrected_lambda_create( f_qs[ powerfit_name ] );

   mode_select( MODE_POWERFIT );
   pb_wheel_save->hide();

   running       = true;

   // set wheel range etc.

   // set le_powerfit_q1,2 start,end to grid limits
   powerfit_markers.clear();

   powerfit_add_marker( plot_dist, le_powerfit_q_start->text().toDouble(), powerfit_color_q, us_tr( "Start")             , Qt::AlignRight | Qt::AlignBottom );
   powerfit_add_marker( plot_dist, le_powerfit_q_end  ->text().toDouble(), powerfit_color_q, us_tr( "End"  )             , Qt::AlignLeft  | Qt::AlignBottom );
   powerfit_add_marker( plot_dist, dad_param_lambda                      , Qt::red         , UNICODE_LAMBDA_QS + "(abs)" , Qt::AlignRight | Qt::AlignBaseline ); // Qt::AlignVCenter );
   powerfit_add_marker( plot_dist, le_powerfit_lambda2->text().toDouble(), Qt::magenta     , UNICODE_LAMBDA_QS + "(scat)", Qt::AlignRight | Qt::AlignBaseline ); // Qt::AlignVCenter );

   powerfit_q_min = f_qs[ powerfit_name ].front();
   powerfit_q_max = f_qs[ powerfit_name ].back();

   powerfit_c_min = le_powerfit_c_min->text().toDouble();
   powerfit_c_max = le_powerfit_c_max->text().toDouble();

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
   powerfit_set_b_default();

   if ( powerfit_last_used_name != powerfit_name ) {
      le_powerfit_a->setText( "0" );
      le_powerfit_c->setText( "4" );
      powerfit_set_b_default();
      powerfit_last_used_name = powerfit_name;
   }

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
   pb_legend                            ->setEnabled( true );
   pb_q_exclude_vis                     ->setEnabled( true );
   pb_q_exclude_left                    ->setEnabled( true );
   pb_q_exclude_right                   ->setEnabled( true );
   pb_q_exclude_clear                   ->setEnabled( q_exclude.size() > 0 );
   pb_pp                                ->setEnabled( true );
   
   pb_powerfit_fit                      ->setEnabled(
                                                     le_powerfit_c_min->text().toDouble() <= le_powerfit_c_max->text().toDouble()
                                                     && le_powerfit_c->text().toDouble() >= le_powerfit_c_min->text().toDouble()
                                                     && le_powerfit_c->text().toDouble() <= le_powerfit_c_max->text().toDouble()
                                                     && (
                                                         powerfit_use_errors
                                                         || (
                                                             (US_Eigen::weight_methods) cb_powerfit_fit_alg_weight->currentData().toInt() != US_Eigen::EIGEN_1_OVER_SD
                                                             && (US_Eigen::weight_methods) cb_powerfit_fit_alg_weight->currentData().toInt() != US_Eigen::EIGEN_1_OVER_SD_SQ
                                                             )
                                                         )
                                                      );

   le_powerfit_q_start                  ->setEnabled( true );
   le_powerfit_q_end                    ->setEnabled( true );

   pb_powerfit_reset                    ->setEnabled(
                                                     le_powerfit_a->text() != "0"
                                                     || le_powerfit_b->text() != powerfit_b_default
                                                     || le_powerfit_c->text() != "4"
                                                     // || le_powerfit_c_min->text() != "3.9"
                                                     // || le_powerfit_c_max->text() != "4.1"
                                                     );
                                                     
   pb_powerfit_create_adjusted_curve    ->setEnabled( true );

   le_powerfit_a                        ->setEnabled( true );
   le_powerfit_b                        ->setEnabled( true );
   le_powerfit_c                        ->setEnabled( true );
   le_powerfit_c_min                    ->setEnabled( true );
   le_powerfit_c_max                    ->setEnabled( true );

   le_powerfit_fit_epsilon              ->setEnabled( true );
   le_powerfit_fit_iterations           ->setEnabled( true );
   le_powerfit_fit_max_calls            ->setEnabled( true );
   
   cb_powerfit_fit_curve                ->setEnabled( true );
   cb_powerfit_fit_alg                  ->setEnabled( true );
   cb_powerfit_fit_alg_weight           ->setEnabled( true );

   le_powerfit_lambda                   ->setEnabled( true );
   le_powerfit_extinction_coef          ->setEnabled( true );

   le_powerfit_lambda2                  ->setEnabled( true );

   cb_powerfit_dispersion_correction    ->setEnabled( true );

   wheel_enables( le_last_focus );

   progress->reset();

   powerfit_compute_conc();
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
   if ( powerfit_tmp_plotnames.size() ) {
      remove_files( powerfit_tmp_plotnames );
      powerfit_tmp_plotnames.clear();
      plot_color.clear();
      plot_style.clear();
      plot_width_multiplier.clear();
   }
}

#define UHMS_SCALE_WHEEL_RES 10000
// #define UHMS_SCALE_WHEEL_RES 1e-6

void US_Hydrodyn_Dad::powerfit_q_start_text( const QString & text ) {
   TSO << QString( "powerfit_q_start_text( %1 )\n" ).arg( text );
   if ( current_mode != MODE_POWERFIT ) {
      return;
   }
   if ( powerfit_markers.size() != 4 ) {
      editor_msg( "red", QString( "internal error: powerfit_q_start_text markers issue size %1" ).arg( powerfit_markers.size() ) );
      return;
   }
   
   powerfit_fit_clear( true );

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
   powerfit_set_b_default();
}

void US_Hydrodyn_Dad::powerfit_q_end_text( const QString & text ) {
   // TSO << QString( "powerfit_q_end_text( %1 )\n" ).arg( text );
   if ( current_mode != MODE_POWERFIT ) {
      return;
   }
   if ( powerfit_markers.size() != 4 ) {
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
      powerfit_set_b_default();
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

void US_Hydrodyn_Dad::powerfit_a_text( const QString & /* text */ ) {
   qDebug() << "powerfit_a_text";
   powerfit_fit_clear();
}

void US_Hydrodyn_Dad::powerfit_a_focus( bool hasFocus ) {
   qDebug() << "powerfit_a_focus";
   if ( !hasFocus ) {
      powerfit_curve_update();
   }
}

void US_Hydrodyn_Dad::powerfit_b_text( const QString & /* text */ ) {
   qDebug() << "powerfit_b_text";
   powerfit_fit_clear();
}

void US_Hydrodyn_Dad::powerfit_b_focus( bool hasFocus ) {
   qDebug() << "powerfit_b_focus";
   if ( !hasFocus ) {
      powerfit_curve_update();
   }
}

void US_Hydrodyn_Dad::powerfit_c_text( const QString & /* text */ ) {
   qDebug() << "powerfit_c_text";
   powerfit_enables();
   powerfit_fit_clear();
}

void US_Hydrodyn_Dad::powerfit_c_focus( bool hasFocus ) {
   qDebug() << "powerfit_a_focus";
   if ( !hasFocus ) {
      powerfit_curve_update();
   }
}

void US_Hydrodyn_Dad::powerfit_c_min_text( const QString & /* text */ ) {
   qDebug() << "powerfit_c_min_text";
   powerfit_c_min = le_powerfit_c_min->text().toDouble();
   powerfit_set_b_default();
   powerfit_enables();
}

void US_Hydrodyn_Dad::powerfit_c_min_focus( bool /* hasFocus */ ) {
   qDebug() << "powerfit_c_min_focus";
}

void US_Hydrodyn_Dad::powerfit_c_max_text( const QString & /* text */ ) {
   qDebug() << "powerfit_c_max_text";
   powerfit_c_max = le_powerfit_c_max->text().toDouble();
   powerfit_enables();
}

void US_Hydrodyn_Dad::powerfit_c_max_focus( bool /* hasFocus */ ) {
   qDebug() << "powerfit_c_max_focus";
}

void US_Hydrodyn_Dad::powerfit_fit_epsilon_text( const QString & /* text */ ) {
   qDebug() << "powerfit_fit_epsilon_text";
}

void US_Hydrodyn_Dad::powerfit_fit_epsilon_focus( bool /* hasFocus */ ) {
   qDebug() << "powerfit_fit_epsilon_focus";
}

void US_Hydrodyn_Dad::powerfit_fit_iterations_text( const QString & /* text */ ) {
   qDebug() << "powerfit_fit_iterations_text";
}

void US_Hydrodyn_Dad::powerfit_fit_iterations_focus( bool /* hasFocus */ ) {
   qDebug() << "powerfit_fit_iterations_focus";
}

void US_Hydrodyn_Dad::powerfit_fit_max_calls_text( const QString & /* text */ ) {
   qDebug() << "powerfit_fit_max_calls_text";
}

void US_Hydrodyn_Dad::powerfit_fit_max_calls_focus( bool /* hasFocus */ ) {
   qDebug() << "powerfit_fit_max_calls_focus";
}

void US_Hydrodyn_Dad::powerfit_computed_conc_text( const QString & /* text */ ) {
   qDebug() << "powerfit_computed_conc_text";
}

void US_Hydrodyn_Dad::powerfit_computed_conc_focus( bool /* hasFocus */ ) {
   qDebug() << "powerfit_computed_conc_focus";
}

void US_Hydrodyn_Dad::powerfit_scat_conc_text( const QString & /* text */ ) {
   qDebug() << "powerfit_scat_conc_text";
}

void US_Hydrodyn_Dad::powerfit_scat_conc_focus( bool /* hasFocus */ ) {
   qDebug() << "powerfit_scat_conc_focus";
}

void US_Hydrodyn_Dad::powerfit_uncorrected_conc_text( const QString & /* text */ ) {
   qDebug() << "powerfit_uncorrected_conc_text";
}

void US_Hydrodyn_Dad::powerfit_uncorrected_conc_focus( bool /* hasFocus */ ) {
   qDebug() << "powerfit_uncorrected_conc_focus";
}

void US_Hydrodyn_Dad::powerfit_lambda_text( const QString & text ) {
   qDebug() << "powerfit_lambda_text";
   dad_param_lambda = text.toDouble();
   lbl_dad_lambdas_data->setText( dad_lambdas.summary_rich() + DAD_LAMBDA_EXTC_MSG );
   if ( powerfit_markers.size() > 2 ) {
      powerfit_markers[ 2 ]->setXValue( dad_param_lambda );
      plot_dist->replot();
   }
   powerfit_compute_conc();
}

void US_Hydrodyn_Dad::powerfit_lambda_focus( bool /* hasFocus */ ) {
   qDebug() << "powerfit_lambda_focus";
}

void US_Hydrodyn_Dad::powerfit_lambda_abs_text( const QString & /* text */ ) {
   qDebug() << "powerfit_lambda_abs_text";
}

void US_Hydrodyn_Dad::powerfit_lambda_abs_focus( bool /* hasFocus */ ) {
   qDebug() << "powerfit_lambda_abs_focus";
}

void US_Hydrodyn_Dad::powerfit_lambda2_text( const QString & text ) {
   qDebug() << "powerfit_lambda2_text";
   if ( powerfit_markers.size() > 3 ) {
      powerfit_markers[ 3 ]->setXValue( text.toDouble() );
      plot_dist->replot();
   }
   powerfit_compute_conc();
}

void US_Hydrodyn_Dad::powerfit_lambda2_focus( bool /* hasFocus */ ) {
   qDebug() << "powerfit_lambda_focus";
}

void US_Hydrodyn_Dad::powerfit_lambda2_abs_text( const QString & /* text */ ) {
   qDebug() << "powerfit_lambda2_abs_text";
}

void US_Hydrodyn_Dad::powerfit_lambda2_abs_focus( bool /* hasFocus */ ) {
   qDebug() << "powerfit_lambda2_abs_focus";
}

void US_Hydrodyn_Dad::powerfit_extinction_coef_text( const QString & text ) {
   qDebug() << "powerfit_extinction_coef_text";
   dad_param_g_extinction_coef = text.toDouble();
   lbl_dad_lambdas_data->setText( dad_lambdas.summary_rich() + DAD_LAMBDA_EXTC_MSG );
   powerfit_compute_conc();
}

void US_Hydrodyn_Dad::powerfit_extinction_coef_focus( bool /* hasFocus */ ) {
   qDebug() << "powerfit_extinction_coef_focus";
}

double compute_powerfit( double t, const double *par ) {
   double c = par[ 2 ];

   if ( c < powerfit_c_min
        || c > powerfit_c_max ) {
      return DBL_MAX;
   }

   // if ( !fit_weights_index.count( t ) ) {
   //    qDebug() << "compute_powerfit error : fit_weights_index[ " << t << " ] does not exist";
   // }
   // if ( fit_weights.size() <= fit_weights_index[ t ] ) {
   //    qDebug() << "compute_powerfit error : fit_weights[ " << fit_weights_index[ t ] << " ] index out of range";
   // }

   return fit_weights[ fit_weights_index[ t ] ] * ( par[ 0 ] + par[ 1 ] * pow( t, -c ) );
}

double compute_powerfit_dispersion_correction( double t, const double *par ) {
   double c = par[ 2 ];

   if ( c < powerfit_c_min
        || c > powerfit_c_max ) {
      return DBL_MAX;
   }

   // if ( !fit_weights_index.count( t ) ) {
   //    qDebug() << "compute_powerfit error : fit_weights_index[ " << t << " ] does not exist";
   // }
   // if ( fit_weights.size() <= fit_weights_index[ t ] ) {
   //    qDebug() << "compute_powerfit error : fit_weights[ " << fit_weights_index[ t ] << " ] index out of range";
   // }
   
   return fit_weights[ fit_weights_index[ t ] ] * ( par[ 0 ] + par[ 1 ] * pow( powerfit_corrected_lambda[ t ], -c ) );
}

void US_Hydrodyn_Dad::powerfit_fit() {
   qDebug() << "powerfit_fit()";
   disable_all();
#warning qwtw_wheel probably should be in ::disable_all() but for some reason we commented it out there, probably should check all wheel usage and set as appropriate
   qwtw_wheel            ->setEnabled( false );
   powerfit_fit_clear();
   disable_all();
   qApp->processEvents();
   
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

   vector < double > q;
   vector < double > I;
   vector < double > e;

   double            rmsd2 = 0;
   double            chi2  = 0;

   if ( powerfit_use_errors ) {
      for ( size_t i = 0; i < f_qs[ powerfit_name ].size(); ++i ) {
         if ( qs.count( f_qs[ powerfit_name ][ i ] ) ) {
            q.push_back( f_qs[ powerfit_name ][ i ] );
            I.push_back( f_Is[ powerfit_name ][ i ] );
            e.push_back( f_errors[ powerfit_name ][ i ] );
         }
      }
   } else {
      for ( size_t i = 0; i < f_qs[ powerfit_name ].size(); ++i ) {
         if ( qs.count( f_qs[ powerfit_name ][ i ] ) ) {
            q.push_back( f_qs[ powerfit_name ][ i ] );
            I.push_back( f_Is[ powerfit_name ][ i ] );
         }
      }
   }      

   vector < double > org_I = I;

   fit_weights.clear();
   fit_weights_index.clear();
   for ( size_t i = 0; i < q.size(); ++i ) {
      fit_weights_index[ q[ i ] ] = i;
   }

   switch( (US_Eigen::weight_methods) cb_powerfit_fit_alg_weight->currentData().toInt() ) {

   case US_Eigen::EIGEN_NO_WEIGHTS :
      fit_weights.resize( I.size(), 1 );
      break;

   case US_Eigen::EIGEN_1_OVER_AMOUNT :
      for ( auto & v : I ) {
         if ( v ) {
            fit_weights.push_back( 1e0 / fabs( v ) );
            v *= fit_weights.back();
         } else {
            fit_weights.push_back( 1 );
         }
      }
      break;
      
   case US_Eigen::EIGEN_1_OVER_AMOUNT_SQ :
      for ( auto & v : I ) {
         if ( v ) {
            fit_weights.push_back( 1e0 / ( fabs( v ) * fabs( v ) ) );
            v *= fit_weights.back();
         } else {
            fit_weights.push_back( 1 );
         } 
      }
      break;
      
   case US_Eigen::EIGEN_1_OVER_SD :
      for ( size_t i = 0; i < I.size(); ++i ) {
         fit_weights.push_back( 1e0 / e[ i ] );
         I[ i ] *= fit_weights.back();
      }
      break;
      
   case US_Eigen::EIGEN_1_OVER_SD_SQ :
      for ( size_t i = 0; i < I.size(); ++i ) {
         fit_weights.push_back( 1e0 / ( e[ i ] * e[ i ] ) );
         I[ i ] *= fit_weights.back();
      }
      break;

   default:
      qDebug() << "powerfit_fit eigen weight error";
      fit_weights.resize( I.size(), 1 );
      break;
   }

   {
      LM::lm_control_struct control = LM::lm_control_double;
      control.printflags = 0; // 3; // monitor status (+1) and parameters (+2)
      control.epsilon    = le_powerfit_fit_epsilon->text().toDouble();
      control.stepbound  = le_powerfit_fit_iterations->text().toInt();
      control.maxcall    = le_powerfit_fit_max_calls->text().toInt();

      LM::lm_status_struct status;
            
      vector < double > par =
         {
            le_powerfit_a->text().toDouble()
            ,le_powerfit_b->text().toDouble()
            ,le_powerfit_c->text().toDouble()
         };

      LM::lmcurve_fit_rmsd( ( int )      par.size(),
                            ( double * ) &( par[ 0 ] ),
                            ( int )      q.size(),
                            ( double * ) &( q[ 0 ] ),
                            ( double * ) &( I[ 0 ] ),
                            cb_powerfit_dispersion_correction->isChecked() ? compute_powerfit_dispersion_correction : compute_powerfit,
                            (const LM::lm_control_struct *)&control,
                            &status );
   
      if ( status.fnorm < 0e0 ) {
         us_qdebug( "WARNING: lm() returned negative rmsd\n" );
      } else {
         rmsd2 = 0e0;
         fit_weights.clear();
         fit_weights.resize( q.size(), 1 );
         for ( size_t i = 0; i < q.size(); ++i ) {
            double t = org_I[ i ] - compute_powerfit( q[ i ], &( par[ 0 ] ) );
            rmsd2 += t * t;
         }

         if ( powerfit_use_errors ) {
            vector < double > m;
            
            for ( size_t i = 0; i < q.size(); ++i ) {
               m.push_back( compute_powerfit( q[ i ], &( par[ 0 ] ) ) );
            }

            if ( !US_Saxs_Util::calc_mychi2( m, org_I, e, chi2 ) ) {
               qDebug() << "calc_mychi2 error\n";
               chi2 = 0;
            }
         }
      }


      le_powerfit_a->setText( QString( "%1" ).arg( par[ 0 ] ) );
      le_powerfit_b->setText( QString( "%1" ).arg( par[ 1 ] ) );
      le_powerfit_c->setText( QString( "%1" ).arg( par[ 2 ] ) );
   }                           

   {
      QString msg = QString( "Fit RMSD %1" ).arg( sqrt( rmsd2 ) );
      if ( powerfit_use_errors && chi2 != 0 ) {
         msg += QString( " Chi^2 %2 nChi^2 %3" ).arg( chi2 ).arg( chi2 / (double)q.size() );
      }
   
      lbl_powerfit_msg->setText( msg );
   }

   powerfit_curve_update();
   
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
   powerfit_enables();
}

void US_Hydrodyn_Dad::powerfit_fit_curve_index() {
   qDebug() << "powerfit_fit_curve_index()";
   powerfit_fit_clear();
}

void US_Hydrodyn_Dad::powerfit_reset() {
   qDebug() << "powerfit_reset()";
   le_powerfit_a->setText( "0" );
   le_powerfit_c->setText( "4" );
   powerfit_set_b_default();
   powerfit_fit_clear();
}

void US_Hydrodyn_Dad::powerfit_fit_clear( bool replot ) {
   qDebug() << "powerfit_fit_clear()";
   le_powerfit_uncorrected_conc->setText( "" );
   le_powerfit_scat_conc       ->setText( "" );
   le_powerfit_computed_conc   ->setText( "" );
   le_powerfit_lambda_abs      ->setText( "" );
   le_powerfit_lambda2_abs     ->setText( "" );
   powerfit_corrected_data_ok = false;

   if ( powerfit_tmp_plotnames.size() ) {
      remove_files( powerfit_tmp_plotnames );
      powerfit_tmp_plotnames.clear();
      plot_color.clear();
      plot_style.clear();
      plot_width_multiplier.clear();
   }

   if ( powerfit_fit_curve ) {
      qDebug() << "powerfit_fit_clear() powerfit_fit_curve set";
      powerfit_fit_curve->detach();
      delete powerfit_fit_curve;
      lbl_powerfit_msg->setText("");
      powerfit_fit_curve = (QwtPlotCurve *) 0;
   }
   if ( powerfit_corrected_curve ) {
      qDebug() << "powerfit_fit_clear() powerfit_corrected_curve set";
      powerfit_corrected_curve->detach();
      delete powerfit_corrected_curve;
      lbl_powerfit_msg->setText("");
      powerfit_corrected_curve = (QwtPlotCurve *) 0;
   }
   if ( replot ) {
      plot_dist->replot();
   }

   powerfit_compute_conc();
}

vector < double > US_Hydrodyn_Dad::powerfit_curve( const vector < double > & x, double A, double B, double C ) {
   size_t x_size = x.size();
   vector < double > y( x_size );

   if ( cb_powerfit_dispersion_correction->isChecked() ) {
      for ( size_t i = 0; i < x_size; ++i ) {
         y[i] = A + B * pow( powerfit_corrected_lambda[ x[ i ] ], -C );
      }
   } else {
      for ( size_t i = 0; i < x_size; ++i ) {
         y[i] = A + B * pow( x[ i ], -C );
      }
   }

   US_Vector::printvector2( QString( "powerfit_curve %1, %2, %3" ).arg( A ).arg( B ).arg( C ), x, y );
   return y;
}

vector < double > US_Hydrodyn_Dad::curve_trim( const vector < double > x, double x_min, double x_max ) {
   vector < double > result;
   for ( auto v : x ) {
      if ( v >= x_min && v <= x_max ) {
         result.push_back( v );
      }
   }
   return result;
}
           
void US_Hydrodyn_Dad::powerfit_curve_update() {
   qDebug() << "powerfit_curve_update()";

   powerfit_fit_clear( false );
   plot_color.clear();
   plot_style.clear();
   plot_width_multiplier.clear();

   vector < double > x = f_qs[ powerfit_name ];

   vector < double > y = powerfit_curve(
                                        x
                                        ,le_powerfit_a->text().toDouble()
                                        ,le_powerfit_b->text().toDouble()
                                        ,le_powerfit_c->text().toDouble()
                                        );
   
   powerfit_corrected_data = f_Is[ powerfit_name ];
   for ( size_t i = 0; i < x.size(); ++i ) {
      powerfit_corrected_data[ i ] -= y[ i ];
   }

   if ( powerfit_use_errors ) {
      powerfit_corrected_data_errors = f_errors[ powerfit_name ];
   }
   
   set < QString > to_select = { powerfit_name };

   // plot fit curve

   add_plot( "Fit", x, y, false, false );
   to_select.insert( last_created_file );
   powerfit_tmp_plotnames.insert( last_created_file );
   plot_color[ last_created_file ] = Qt::green;
   plot_style[ last_created_file ] = Qt::SolidLine;
   plot_width_multiplier[ last_created_file ] = 2;

   // plot corrected curve
   for ( size_t i = 0; i < x.size(); ++i ) {
      y[ i ] = f_Is[ powerfit_name ][ i ] - y[ i ];
   }

   if ( powerfit_use_errors ) {
      add_plot( "Scattering corrected", x, y, f_errors[ powerfit_name ], false, false );
   } else {
      add_plot( "Scattering corrected", x, y, false, false );
   }         

   to_select.insert( last_created_file );
   plot_color[ last_created_file ] = Qt::cyan;
   powerfit_tmp_plotnames.insert( last_created_file );

   set_selected( to_select, true );
   plot_dist->replot();
   powerfit_corrected_data_ok = true;
   powerfit_compute_conc();
}

void US_Hydrodyn_Dad::powerfit_create_adjusted_curve() {
   qDebug() << "powerfit_create_adjusted_curve()";

   powerfit_created_name .clear();

   vector < double > x = f_qs[ powerfit_name ];

   vector < double > y = powerfit_curve(
                                        x
                                        ,le_powerfit_a->text().toDouble()
                                        ,le_powerfit_b->text().toDouble()
                                        ,le_powerfit_c->text().toDouble()
                                        );

   for ( size_t i = 0; i < x.size(); ++i ) {
      y[ i ] = f_Is[ powerfit_name ][ i ] - y[ i ];
   }

   QString name = powerfit_name + QString( "_scA_%1_scB_%2_scC_%3" )
      .arg( le_powerfit_a->text() )
      .arg( le_powerfit_b->text() )
      .arg( le_powerfit_c->text() )
      .replace( ".", "_" )
      ;
   
   if ( powerfit_use_errors ) {
      add_plot( name, x, y, f_errors[ powerfit_name ], false, false );
   } else {
      add_plot( name, x, y, false, false );
   }
   powerfit_created_name.insert( last_created_file );
}

void US_Hydrodyn_Dad::powerfit_compute_conc() {
   le_powerfit_uncorrected_conc->setText( "" );
   le_powerfit_scat_conc       ->setText( "" );
   le_powerfit_computed_conc   ->setText( "" );
   le_powerfit_lambda_abs      ->setText( "" );
   le_powerfit_lambda2_abs     ->setText( "" );
   if ( powerfit_name.isEmpty() ) {
      return;
   }

   if ( dad_param_g_extinction_coef == 0 ) {
      le_powerfit_uncorrected_conc->setText( us_tr( "Extc. coef. is zero" ) );
      return;
   }
   
   qDebug() << "powerfit_compute_conc()";
   // interpolate subtracted curve lambda divide by g_conc
   // populate le_powerfit_computed_conc

   bool using_corrected_data = powerfit_corrected_data_ok && powerfit_corrected_data.size() == f_qs[ powerfit_name ].size();

   // compute uncorrected

   {
      vector < double > x       = f_qs[ powerfit_name ];
      vector < double > y       = f_Is[ powerfit_name ];

      // created trimmed versions

      {
         vector < double > trimmed_x;
         vector < double > trimmed_y;
         
         for ( size_t i = 0; i < x.size(); ++i ) {
            if ( !q_exclude.count( x[ i ] ) ) {
               trimmed_x.push_back( x[i] );
               trimmed_y.push_back( y[i] );
            }
         }

         x = trimmed_x;
         y = trimmed_y;
      }

      if ( dad_param_lambda < x.front() || dad_param_lambda > x.back() ) {
         le_powerfit_uncorrected_conc->setText( us_tr( UNICODE_LAMBDA_QS + " out of range" ) );
         return;
      }
      vector < double > target = { dad_param_lambda };
      vector < double > y2;
   
      if ( !usu->linear_interpolate( x, y, target, y2 ) ) {
         le_powerfit_uncorrected_conc->setText( usu->errormsg );
         return;
      }
      double abs_at_lambda = y2.front();
      le_powerfit_lambda_abs->setText( QString( "%1" ).arg( abs_at_lambda, 0, 'f', 3 ) );
      powerfit_uncorrected_conc = abs_at_lambda / dad_param_g_extinction_coef;
      powerfit_uncorrected_conc = QString( "%1" ).arg( powerfit_uncorrected_conc, 0, 'f', 3 ).toDouble();
      le_powerfit_uncorrected_conc->setText( QString( "%1" ).arg( powerfit_uncorrected_conc, 0, 'f', 3 ) );
      // compute scat corrected
      {
         double dad_param_lambda2 = le_powerfit_lambda2->text().toDouble();
         if ( dad_param_lambda2 < x.front() || dad_param_lambda > x.back() ) {
            le_powerfit_scat_conc->setText( us_tr( UNICODE_LAMBDA_QS + " out of range" ) );
         } else {
            vector < double > target = { dad_param_lambda2 };
            vector < double > y2;
            if ( !usu->linear_interpolate( x, y, target, y2 ) ) {
               le_powerfit_scat_conc->setText( usu->errormsg );
            } else {
               double abs_at_lambda2 = y2.front();
               le_powerfit_lambda2_abs->setText( QString( "%1" ).arg( abs_at_lambda2, 0, 'f', 3 ) );
               powerfit_scat_conc = ( abs_at_lambda - abs_at_lambda2 ) / dad_param_g_extinction_coef;
               powerfit_scat_conc = QString( "%1" ).arg( powerfit_scat_conc, 0, 'f', 3 ).toDouble();
               le_powerfit_scat_conc->setText( QString( "%1" ).arg( powerfit_scat_conc, 0, 'f', 3 ) );
            }
         }
      }
   }

   if ( using_corrected_data ) {
      vector < double > x       = f_qs[ powerfit_name ];
      vector < double > y       = powerfit_corrected_data;

      // created trimmed versions

      {
         vector < double > trimmed_x;
         vector < double > trimmed_y;
         
         for ( size_t i = 0; i < x.size(); ++i ) {
            if ( !q_exclude.count( x[ i ] ) ) {
               trimmed_x.push_back( x[i] );
               trimmed_y.push_back( y[i] );
            }
         }

         x = trimmed_x;
         y = trimmed_y;
      }

      if ( dad_param_lambda < x.front() || dad_param_lambda > x.back() ) {
         le_powerfit_computed_conc->setText( us_tr( UNICODE_LAMBDA_QS + " out of range" ) );
         return;
      }

      vector < double > target = { dad_param_lambda };
      vector < double > y2;
   
      if ( !usu->linear_interpolate( x, y, target, y2 ) ) {
         le_powerfit_computed_conc->setText( usu->errormsg );
         return;
      }
   
      powerfit_conc = y2.front() / dad_param_g_extinction_coef;
      powerfit_conc = QString( "%1" ).arg( powerfit_conc, 0, 'f', 3 ).toDouble();
      le_powerfit_computed_conc->setText( QString( "%1" ).arg( powerfit_conc, 0, 'f', 3 ) );
   }
}

void US_Hydrodyn_Dad::powerfit_set_b_default() {
   qDebug() << "powerfit_set_b_default()";
   double q_start = le_powerfit_q_start->text().toDouble();
   if ( q_start < 0 ) {
      q_start = 0;
   }
   
   double val;
   if ( powerfit_org_I_at_lambda.count( q_start ) ) {
      val = powerfit_org_I_at_lambda[ q_start ];
   } else {
      vector < double > target = { q_start };
      vector < double > y2;
   
      if ( !usu->linear_interpolate( f_qs[ powerfit_name], f_Is[ powerfit_name ], target, y2 ) ) {
         qDebug() << "could not find I value at lambda for setting b";
         return;
      }
      val = y2[ 0 ];
   }

   if ( val == 0 ) {
      return;
   }

   powerfit_b_default = QString( "%1" ).arg( 1e0 / fabs( pow( q_start, -le_powerfit_c_min->text().toDouble()) / val ) );
   le_powerfit_b->setText( powerfit_b_default );
}

void US_Hydrodyn_Dad::powerfit_dispersion_correction_clicked() {
   qDebug() << "powerfit_dispersion_correction_clicked()";
   powerfit_fit_clear( true );
}

