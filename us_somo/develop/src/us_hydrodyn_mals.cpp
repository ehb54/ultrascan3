#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_mals.h"
#include "../include/us_hydrodyn_mals_ciq.h"
#include "../include/us_hydrodyn_mals_fit.h"
#include "../include/us_hydrodyn_mals_fit_global.h"
#include "../include/us_hydrodyn_mals_conc_csv_frames.h"
#include "../include/us_lm.h"
#if QT_VERSION >= 0x040000
#include <qwt_scale_engine.h>
#endif
#include <qpalette.h>
//Added by qt3to4:
#include <QFrame>
#include <QLabel>
#include <QTextStream>
#include <QMouseEvent>
#include <QCloseEvent>
#include "../include/us_plot_zoom.h"

// #define JAC_VERSION

#define SLASH QDir::separator()
#define TSO QTextStream(stdout)

QString MALS_Angle::list() {
   return
      QString( "%1 %2 %3 %4" )
      .arg( us_double_decimal_places( angle, 2 ), 8 )
      .arg(
           has_angle_ri_corr
           ? QString( "%1" ).arg( us_double_decimal_places( angle_ri_corr, 2 ) )
           : QString( "n/a" )
           , 8
           )
      .arg(
           has_gain
           ? QString( "%1" ).arg( us_double_decimal_places( gain, 2 ) )
           : QString( "n/a" )
           ,8
           )
      .arg( us_double_decimal_places( norm_coef, 3 ), 8 )
      ;
}

QString MALS_Angle::list_csv() {
   return
      QString( "%1,%2,%3,%4" )
      .arg( us_double_decimal_places( angle, 2 ) )
      .arg(
           has_angle_ri_corr
           ? QString( "%1" ).arg( us_double_decimal_places( angle_ri_corr, 2 ) )
           : QString( "\"n/a\"" )
           )
      .arg(
           has_gain
           ? QString( "%1" ).arg( us_double_decimal_places( gain, 2 ) )
           : QString( "\"n/a\"" )
           )
      .arg( us_double_decimal_places( norm_coef, 3 ) )
      ;
}
     
QString MALS_Angle::list_rich( double lambda, double n ) {
   if ( !lambda || !n ) {
      return
         QString( "<td> %1 </td><td> %2 </td><td> %3 </td><td> %4 </td>" )
         .arg( us_double_decimal_places( angle, 2 ) )
         .arg(
              has_angle_ri_corr
              ? QString( "%1" ).arg( us_double_decimal_places( angle_ri_corr, 2 ) )
              : QString( "n/a" )
              , 8
              )
         .arg(
              has_gain
              ? QString( "%1" ).arg( us_double_decimal_places( gain, 2 ) )
              : QString( "n/a" )
              ,8
              )
         .arg( us_double_decimal_places( norm_coef, 3 ) )
         ;
   }
   return
      QString( "<td> %1 </td><td> %2 </td><td> %3 </td><td> %4 </td><td> %5 </td>" )
      .arg( us_double_decimal_places( angle, 2 ) )
      .arg(
           has_angle_ri_corr
           ? QString( "%1" ).arg( us_double_decimal_places( angle_ri_corr, 2 ) )
           : QString( "n/a" )
           , 8
           )
      .arg(
           has_gain
           ? QString( "%1" ).arg( us_double_decimal_places( gain, 2 ) )
           : QString( "n/a" )
           ,8
           )
      .arg( us_double_decimal_places( norm_coef, 3 ) )
      .arg(
           has_angle_ri_corr
           ? QString( "%1" ).arg(  n * 4 * M_PI * sin( angle_ri_corr * M_PI / 360 ) / ( lambda * 10 ) )
           : QString( "n/a" )
           )
      ;
}

bool MALS_Angle::populate( const QStringList & qsl ) {

   QStringList qslu = qsl;

   // angle
   if ( !qslu.size() ) {
      return false;
   }

   angle = qslu.front().replace( QRegularExpression( "[()]" ), "" ).toDouble();
   qslu.pop_front();

   // angle_ri_corr
   if ( !qslu.size() ) {
      return false;
   }

   if ( qslu.front() == "n/a" ) {
      qslu.pop_front();
      if ( !qslu.size() ) {
         return false;
      }
      angle_ri_corr     = 0;
      has_angle_ri_corr = false;
   } else {
      // remove angle symbol
      QString qs = qslu.front();
      qs.replace( QRegularExpression( ".$" ), "" );
      angle_ri_corr     = qs.toDouble();
      has_angle_ri_corr = true;
   }
   qslu.pop_front();

   // gain
   if ( !qslu.size() ) {
      return false;
   }

   if ( qslu.front() == "n/a" ) {
      gain     = 0;
      has_gain = false;
   } else {
      gain     = qslu.front().toDouble();
      has_gain = true;
   }
   qslu.pop_front();
   
   if ( !qslu.size() ) {
      return false;
   }

   // normalization coeff
   if ( qslu.front() == "n/a" ) {
      norm_coef     = 0;
      has_norm_coef = false;
   } else {
      norm_coef     = qslu.front().toDouble();
      has_norm_coef = true;
   }
   qslu.pop_front();   
   return true;
}

QString MALS_Angles::list() {
   QStringList qsl;
   qsl << "Detector Angle    RI-Corr.  Gain    Norm.-Coef.\n";
   for ( auto it = mals_angle.begin();
         it != mals_angle.end();
         ++it ) {
      qsl << QString( "%1" ).arg( it->first, 3 ) << "    " << it->second.list() << "\n";
   }
   return qsl.join( "" );
}

void MALS_Angles::clear() {
   mals_angle.clear();
   q_to_ri.clear();
   loaded_filename = "";
}

void MALS_Angles::build_q_to_ri( double lambda, double n ) {
   // qDebug() << QString( "build_q_to_ri( %1, %2 )" ).arg( lambda ).arg( n );
   q_to_ri.clear();
   // qDebug() << "q_to_ri cleared";
   if ( lambda == 0 || n == 0 ) {
      qDebug() << "internal error: build_q_to_ri() lambda and/or n 0";
      return;
   }

   for ( auto & ma : mals_angle ) {
      if ( ma.second.has_angle_ri_corr ) {
         q_to_ri[ QString( "%1" ).arg( n * 4 * M_PI * sin( ma.second.angle_ri_corr * M_PI / 360 ) / ( lambda * 10 ) ).toDouble() ] = ma.second.angle_ri_corr;
      }
   }
   // qDebug() << "q_to_ri after setup";
   // for ( auto const & it : q_to_ri ) {
   //    qDebug() << QString( "q_to_ri q %1 -> ri %2" ).arg( it.first ).arg( it.second );
   // }
}

QString MALS_Angles::list_rich( double lambda, double n ) {
   QStringList qsl;
   if ( !mals_angle.size() ) {
      return us_tr( "<b>MALS Angles</b> not loaded" );
   }
   qsl << QString( us_tr( "<b>MALS Angles</b> loaded from %1<hr>" ) ).arg( loaded_filename );
   if ( !lambda || !n ) {
      qsl << "<table border=1 bgcolor=#FFF cellpadding=1.5>\n<tr><th> Detector </th><th> Angle </th><th> RI-Corr. </th><th> Gain </th><th> Norm.-Coef. </th></tr>\n";
      for ( auto it = mals_angle.begin();
            it != mals_angle.end();
            ++it ) {
         qsl << QString( "<tr><td>%1</td>" ).arg( it->first ) << it->second.list_rich() << "</tr>";
      }
   } else {
      qsl
         << "<table border=1 bgcolor=#FFF cellpadding=1.5>\n<tr><th> Detector </th><th> Angle </th><th> RI-Corr. </th><th> Gain </th><th> Norm.-Coef. </th><th> q [1/A]</th></tr>\n"
         ;
      for ( auto it = mals_angle.begin();
            it != mals_angle.end();
            ++it ) {
         qsl << QString( "<tr><td>%1</td>" ).arg( it->first ) << it->second.list_rich( lambda, n ) << "</tr>";
      }
   }
   qsl
      << "</table>"
      ;
   return qsl.join( "" );
}

QStringList MALS_Angles::list_active() {
   QStringList qsl;
   for ( auto it = mals_angle.begin();
         it != mals_angle.end();
         ++it ) {
      if ( it->second.has_angle_ri_corr ) {
         qsl <<
            QString( "%1 %2 [%3]" )
            .arg( it->first )
            .arg( it->second.angle )
            .arg( it->second.angle_ri_corr )
            ;
      }
   }
   return qsl;
}

bool MALS_Angles::populate( const QString & filename, const QStringList & qsl ) {
   if ( !qsl.size()
        || qsl[0] != "Detector"
        || !qsl[1].contains( QRegularExpression( "^Refractive" ) ) ) {
      return false;
   }

   clear();
   loaded_filename = filename;

   for ( int i = 0; i < (int) qsl.size(); ++i ) {
      if ( qsl[i].contains( QRegularExpression( "^[0-9]+\\s" ) ) ) {
         QString qsu = qsl[i];
         qsu.replace( QRegularExpression( "[()]" ), "" );
         QStringList qsl_mals_angle = qsu.split( QRegularExpression( "\\s+" ) );
         if ( qsl_mals_angle.size() >= 5 ) {
            int index = qsl_mals_angle[0].toInt();
            qsl_mals_angle.pop_front();
            if ( !mals_angle[index].populate( qsl_mals_angle ) ) {
               mals_angle.erase(index);
            }
         }
      }
   }

   return true;
}

bool MALS_Angles::load( const QString & filename, const QStringList & csvlines, QString & errormsg ) {
   if ( !csvlines.size() ) {
      errormsg = us_tr( "Empty file" );
      return false;
   }

   if ( !csvlines[0].contains( QRegularExpression( "^\\s*\"\\s*US-SOMO MALS Angles" ) ) ) {
      errormsg = us_tr( "Not a MALS Angles csv file" );
      return false;
   }

   QStringList data = csvlines;

   data.pop_front();
   data.pop_front();

   clear();
   loaded_filename = filename;

   int pos = 2;

   while( data.size()
          && data.front().contains( QRegularExpression( "^\\s*\\d+," ) ) ) {
      ++pos;
      QStringList qsl = data.front().split( "," );
      data.pop_front();
      if ( qsl.size() < 5 ) {
         errormsg += QString( us_tr( "Error in MALS Angles csv line %1\n" ) ).arg( pos );
         continue;
      }
      int index = qsl[0].toInt();

      mals_angle[index].angle              = qsl[1].toDouble();

      mals_angle[index].angle_ri_corr      = qsl[2] == "\"n/a\"" ? 0 : qsl[2].toDouble();
      mals_angle[index].has_angle_ri_corr  = qsl[2] == "\"n/a\"" ? false : true;
      
      mals_angle[index].gain               = qsl[3] == "\"n/a\"" ? 0 : qsl[3].toDouble();
      mals_angle[index].has_gain           = qsl[3] == "\"n/a\"" ? false : true;

      mals_angle[index].norm_coef          = qsl[4] == "\"n/a\"" ? 0 : qsl[4].toDouble();
      mals_angle[index].has_norm_coef      = qsl[4] == "\"n/a\"" ? false : true;
   }      

   return true;
}

bool MALS_Angles::load( const QString & filename, QString & errormsg ) {
   QString contents;
   if ( !US_File_Util::getcontents( filename, contents, errormsg ) ) {
      return false;
   }

   QStringList csvlines = contents.split( "\n" );
   return load( filename, csvlines, errormsg );
}

bool MALS_Angles::save( const QString & filename, QString & errormsg ) {
   QStringList qsl;

   qsl
      << "\"US-SOMO MALS Angles\"\n"
      << "\"Detector\",\"Angle\",\"RI-Corr.\",\"Gain\",\"Norm.-Coef.\"\n";
      ;

   for ( auto it = mals_angle.begin();
         it != mals_angle.end();
         ++it ) {
      qsl << QString( "%1," ).arg( it->first ) << it->second.list_csv() << "\n";
   }

   QString contents = qsl.join( "" );
   return US_File_Util::putcontents( filename, contents, errormsg );
}

// static   void printvector( QString qs, vector < unsigned int > x )
// {
//    TSO << QString( "%1: size %2:" ).arg( qs ).arg( x.size() );
//    for ( unsigned int i = 0; i < x.size(); i++ )
//    {
//       TSO << QString( " %1" ).arg( x[ i ] );
//    }
//    TSO << endl;
// }

// static void printvector( QString qs, vector < double > x )
// {
//    TSO << QString( "%1: size %2:" ).arg( qs ).arg( x.size() );
//    for ( unsigned int i = 0; i < x.size(); i++ )
//    {
//       TSO << QString( " %1" ).arg( x[ i ], 0, 'g', 8 );
//    }
//    TSO << endl;
// }

US_Hydrodyn_Mals::US_Hydrodyn_Mals(
                                             csv csv1,
                                             void *us_hydrodyn, 
                                             QWidget *p
                                             ) : QFrame( p )
{
   this->csv1 = csv1;
   this->us_hydrodyn = us_hydrodyn;
   started_in_expert_mode = ((US_Hydrodyn *)us_hydrodyn)->advanced_config.expert_mode;
   mals_options_widget = 0;

   gaussian_param_text
      << "amplitude"
      << "center"
      << "width"
      << "distortion-1"
      << "distortion-2"
      ;

#if defined( JAC_VERSION )
   gaussian_type = GAUSS;
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_gaussian_type" ] = QString( "%1" ).arg( gaussian_type );
#endif

   // fix up possible unconfigured or bad configuration

   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_line_width" ) )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_line_width" ] = "1";
   }
   use_line_width = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_line_width" ].toUInt();

   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_bl_linear" ) )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_bl_linear" ] = "false";
   }
   //   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_bl_integral" ) )
   //   {
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_bl_integral" ] = "true";
   //   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_bl_save" ) )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_bl_save" ] = "false";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_bl_smooth" ) ||
        ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_bl_smooth" ] == "0" ||
        ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_bl_smooth" ] == ""
        )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_bl_smooth" ] = "3";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_bl_reps" ) )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_bl_reps" ] = "5";
   }
   if ( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_bl_linear" ] == "true" &&
        ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_bl_integral" ] == "true" )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_bl_linear" ] = "false";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_zi_window" ) )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_zi_window" ] = "25";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_discard_it_sd_mult" ) )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_discard_it_sd_mult" ] = "2";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_cb_discard_it_sd_mult" ) )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_cb_discard_it_sd_mult" ] = "true";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_cb_guinier_qrgmax" ) )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_cb_guinier_qrgmax" ] = "true";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_guinier_qrgmax" ) )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_guinier_qrgmax" ] = "1.3";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_dist_max" ) )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_dist_max" ] = "50.0";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_mwt_k" ) )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_mwt_k" ] = "1";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_mwt_c" ) )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_mwt_c" ] = "-2.095";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_mwt_qmax" ) )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_mwt_qmax" ] = "0.2";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_mwc_mw_per_N" ) )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_mwc_mw_per_N" ] = "112";
   }

   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_ampl_width_min"        ) ||
        !started_in_expert_mode ) {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_ampl_width_min"        ] = "1e-12";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_lock_min_retry"        ) ||
        !started_in_expert_mode ) {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_lock_min_retry"        ] = "false";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_lock_min_retry_mult"   ) ||
        !started_in_expert_mode ) {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_lock_min_retry_mult"   ] = "2";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_maxfpk_restart"        ) ||
        !started_in_expert_mode ) {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_maxfpk_restart"        ] = "false";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_maxfpk_restart_tries"  ) ||
        !started_in_expert_mode ) {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_maxfpk_restart_tries"  ] = "3";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_maxfpk_restart_pct"    ) ||
        !started_in_expert_mode ) {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_maxfpk_restart_pct"    ] = "10";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_cormap_maxq"    ) ) {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_cormap_maxq"    ] = "0.05";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_cormap_alpha"    ) ) {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_cormap_alpha"    ] = "0.01";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_bl_i_power"    ) ||
        !started_in_expert_mode ) {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_bl_i_power"    ] = "1";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_cb_makeiq_cutmax_pct" ) )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_cb_makeiq_cutmax_pct" ] = "true";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_makeiq_cutmax_pct" ) )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_makeiq_cutmax_pct" ] = "1";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_cb_makeiq_avg_peaks" ) )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_cb_makeiq_avg_peaks" ] = "false";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_makeiq_avg_peaks" ) )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_makeiq_avg_peaks" ] = "5";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_cb_gg_smooth" ) ) {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_cb_gg_smooth" ] = "false";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_gg_smooth" ) ) {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_gg_smooth" ] = "3";
   }

   baseline_ready_to_apply = false;

   gaussian_type = 
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_gaussian_type" ) ?
      (gaussian_types)( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_gaussian_type" ].toInt() : GAUSS;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle(us_tr("US-SOMO: MALS"));
   order_ascending = false;
   conc_widget     = false;
#if QT_VERSION >= 0x040000
   legend_vis      = false;
#endif
   usu             = new US_Saxs_Util();

   QDir::setCurrent( ((US_Hydrodyn *)us_hydrodyn)->somo_dir + QDir::separator() + "saxs" );

   saxs_widget = &(((US_Hydrodyn *) us_hydrodyn)->saxs_plot_widget);
   saxs_window = ((US_Hydrodyn *) us_hydrodyn)->saxs_plot_window;
   ((US_Hydrodyn *) us_hydrodyn)->mals_widget = true;

   errors_were_on = false;

   plot_dist_zoomer           = (ScrollZoomer *) 0;
   plot_errors_zoomer         = (ScrollZoomer *) 0;

   guinier_plot_zoomer        = (ScrollZoomer *) 0;
   guinier_plot_errors_zoomer = (ScrollZoomer *) 0;
   guinier_plot_rg_zoomer     = (ScrollZoomer *) 0;
   guinier_plot_mw_zoomer     = (ScrollZoomer *) 0;

   ggqfit_plot_zoomer         = (ScrollZoomer *) 0;

   le_last_focus      = (mQLineEdit *) 0;
   
   disable_updates = false;
   plot3d_flag     = false;

   cg_red = USglobal->global_colors.cg_label;
   cg_red.setBrush( QPalette::WindowText, QBrush( QColor( "red" ),  Qt::SolidPattern ) );

   lbl_mode_title = (QLabel *) 0;

   setupGUI();
   update_gauss_mode();
   running = false;
   axis_y_log = true;
   axis_y();
   axis_x_log = false;

   // gaussian_mode  = false;
   // ggaussian_mode = false;
   // baseline_mode  = false;
   // timeshift_mode = false;
   // scale_mode     = false;
   testiq_active = false;
   mode_select( MODE_NORMAL );

   unified_ggaussian_ok = false;
   wheel_errors_ok      = false;

   detector_uv   = false;
   detector_uv_conv = 0e0;

   detector_ri   = false;
   detector_ri_conv = 0e0;

   update_enables();

   global_Xpos += 30;
   global_Ypos += 30;

   // unsigned int csv_height = 40;
   unsigned int csv_width =  1000;

   // TSO << QString("csv size %1 %2\n").arg(csv_height).arg(csv_width);
#if defined(DOES_WORK)
   // lb_files        ->setMaximumWidth( 3 * csv_width / 7 );
   // lb_created_files->setMaximumWidth( 3 * csv_width / 7 );
   // editor          ->setMaximumWidth( 3 * csv_width / 7 );
   // plot_dist    ->setMinimumWidth( 2 * csv_width / 3 );
#endif
   // lb_files        ->setMaximumWidth( 2 * csv_width / 5 );
   // lb_created_files->setMaximumWidth( csv_width / 3 );
   // editor          ->setMaximumWidth( csv_width / 3 );
   // le_created_dir    ->setMaximumWidth( csv_width / 3 );

   // int percharwidth = 1 + ( 7 * ( USglobal->config_list.fontSize - 1 ) / 10 );
   {
      vector < QPushButton * > pbs;
      // pbs.push_back( pb_add_files );
      // pbs.push_back( pb_conc );
      // pbs.push_back( pb_clear_files );

      // pbs.push_back( pb_select_all );
      pbs.push_back( pb_regex_load );
      pbs.push_back( pb_save_state );
      // pbs.push_back( pb_movie );
      // pbs.push_back( pb_invert );
      // pbs.push_back( pb_adjacent );
      // pbs.push_back( pb_select_nth );
      // pbs.push_back( pb_color_rotate );
      // pbs.push_back( pb_to_saxs );
      // pbs.push_back( pb_view );
      // pbs.push_back( pb_rescale );
      // pbs.push_back( pb_normalize );

      // pbs.push_back( pb_avg );
      pbs.push_back( pb_add );
      pbs.push_back( pb_conc_avg );
      // pbs.push_back( pb_select_all_created );
      // pbs.push_back( pb_invert_all_created );
      // pbs.push_back( pb_adjacent_created );
      // pbs.push_back( pb_remove_created );
      // pbs.push_back( pb_save_created_csv );
      // pbs.push_back( pb_save_created );
      // pbs.push_back( pb_show_created );
      // pbs.push_back( pb_show_only_created );

      pbs.push_back( pb_stack_push_all );
      pbs.push_back( pb_stack_push_sel );
      pbs.push_back( pb_stack_pcopy );
      pbs.push_back( pb_stack_copy );
      pbs.push_back( pb_stack_paste );
      pbs.push_back( pb_stack_drop );
      pbs.push_back( pb_stack_join );
      pbs.push_back( pb_stack_rot_up );
      pbs.push_back( pb_stack_rot_down );
      pbs.push_back( pb_stack_swap );

      pbs.push_back( pb_gauss_mode );
      pbs.push_back( pb_gauss_start );
      pbs.push_back( pb_p3d );
      pbs.push_back( pb_gauss_clear );
      pbs.push_back( pb_gauss_new );
      pbs.push_back( pb_gauss_delete );
      pbs.push_back( pb_gauss_prev );
      pbs.push_back( pb_gauss_next );
      pbs.push_back( pb_legend );
      // pbs.push_back( pb_axis_x );
      // pbs.push_back( pb_axis_y );
      pbs.push_back( pb_ggauss_rmsd );
      pbs.push_back( pb_ggauss_start );
      pbs.push_back( pb_gauss_save );
      pbs.push_back( pb_gauss_as_curves );
      pbs.push_back( pb_ggauss_as_curves );
      pbs.push_back( pb_baseline_start );
      pbs.push_back( pb_baseline_apply );
        
      // for ( unsigned int i = 0; i < pbs.size(); i++ )
      // {
      //    pbs[ i ]->setMaximumWidth( percharwidth * ( pbs[ i ]->text().length() + 2 ) );
      // }
   }

   // lbl_dir        ->setMaximumWidth( lb_files->width() - cb_lock_dir->width() );
   // le_created_dir->setMaximumWidth( lb_files->width() );
   
   // int left_over = ( csv_width / 3 ) - pb_regex_load->maximumWidth();
   // le_regex->setMaximumWidth( left_over / 3 );
   // le_regex_args->setMaximumWidth( left_over - le_regex->maximumWidth() );

   // progress        ->setMaximumWidth( csv_width / 3 );
   pb_help         ->setMinimumWidth( csv_width / 6 );
   pb_options      ->setMinimumWidth( csv_width / 6 );
   pb_cancel       ->setMinimumWidth( csv_width / 3 );

   // #if defined( JAC_VERSION )
   //    if ( !U_EXPT )
   //    {
   //       pb_help         ->setMinimumWidth( csv_width / 3 );
   //    }
   // #endif

   //   setGeometry(global_Xpos, global_Ypos, csv_width, 100 + csv_height );
   setGeometry(global_Xpos, global_Ypos, csv_width, 0 );

   suppress_replot = false;

   // pb_set_conc_file->setMaximumWidth ( pb_select_all->width() + 10 );
   //    pb_set_mals->setMaximumWidth ( pb_select_all->width() + 10 );
   //    pb_set_empty ->setMaximumWidth ( pb_select_all->width() + 10 );
   //    pb_set_signal->setMaximumWidth ( pb_select_all->width() + 10 );
   plot_colors.clear( );

   // sort by max difference pairs & exclude or simply create a bunch
   // by looping over rgb and excluding difference from background?

   QColor bgc = plot_dist->canvasBackground().color();

   push_back_color_if_ok( bgc, Qt::yellow );
   push_back_color_if_ok( bgc, Qt::green );
   push_back_color_if_ok( bgc, Qt::cyan );
   push_back_color_if_ok( bgc, Qt::red );
   push_back_color_if_ok( bgc, Qt::magenta );
   push_back_color_if_ok( bgc, Qt::darkYellow );
   push_back_color_if_ok( bgc, Qt::darkGreen );
   push_back_color_if_ok( bgc, Qt::darkCyan );
   push_back_color_if_ok( bgc, Qt::darkBlue );
   push_back_color_if_ok( bgc, Qt::darkRed );
   push_back_color_if_ok( bgc, Qt::darkMagenta );
   push_back_color_if_ok( bgc, Qt::blue );
   push_back_color_if_ok( bgc, Qt::white );
   push_back_color_if_ok( bgc, QColor( 240, 248, 255 ) ); /* Alice Blue */
   push_back_color_if_ok( bgc, QColor( 250, 235, 215 ) ); /* Antique White */
   push_back_color_if_ok( bgc, QColor( 0, 255, 255 ) ); /* Aqua* */
   push_back_color_if_ok( bgc, QColor( 127, 255, 212 ) ); /* Aquamarine */
   push_back_color_if_ok( bgc, QColor( 240, 255, 255 ) ); /* Azure */
   push_back_color_if_ok( bgc, QColor( 245, 245, 220 ) ); /* Beige */
   push_back_color_if_ok( bgc, QColor( 255, 228, 196 ) ); /* Bisque */
   push_back_color_if_ok( bgc, QColor( 0, 0, 0 ) ); /* Black* */
   push_back_color_if_ok( bgc, QColor( 255, 235, 205 ) ); /* Blanched Almond */
   push_back_color_if_ok( bgc, QColor( 0, 0, 255 ) ); /* Blue* */
   push_back_color_if_ok( bgc, QColor( 138, 43, 226 ) ); /* Blue-Violet */
   push_back_color_if_ok( bgc, QColor( 165, 42, 42 ) ); /* Brown */
   push_back_color_if_ok( bgc, QColor( 222, 184, 135 ) ); /* Burlywood */
   push_back_color_if_ok( bgc, QColor( 95, 158, 160 ) ); /* Cadet Blue */
   push_back_color_if_ok( bgc, QColor( 127, 255, 0 ) ); /* Chartreuse */
   push_back_color_if_ok( bgc, QColor( 210, 105, 30 ) ); /* Chocolate */
   push_back_color_if_ok( bgc, QColor( 255, 127, 80 ) ); /* Coral */
   push_back_color_if_ok( bgc, QColor( 100, 149, 237 ) ); /* Cornflower Blue */
   push_back_color_if_ok( bgc, QColor( 255, 248, 220 ) ); /* Cornsilk */
   push_back_color_if_ok( bgc, QColor( 0, 255, 255 ) ); /* Cyan */
   push_back_color_if_ok( bgc, QColor( 0, 0, 139 ) ); /* Dark Blue */
   push_back_color_if_ok( bgc, QColor( 0, 139, 139 ) ); /* Dark Cyan */
   push_back_color_if_ok( bgc, QColor( 184, 134, 11 ) ); /* Dark Goldenrod */
   push_back_color_if_ok( bgc, QColor( 169, 169, 169 ) ); /* Dark Gray */
   push_back_color_if_ok( bgc, QColor( 0, 100, 0 ) ); /* Dark Green */
   push_back_color_if_ok( bgc, QColor( 189, 183, 107 ) ); /* Dark Khaki */
   push_back_color_if_ok( bgc, QColor( 139, 0, 139 ) ); /* Dark Magenta */
   push_back_color_if_ok( bgc, QColor( 85, 107, 47 ) ); /* Dark Olive Green */
   push_back_color_if_ok( bgc, QColor( 255, 140, 0 ) ); /* Dark Orange */
   push_back_color_if_ok( bgc, QColor( 153, 50, 204 ) ); /* Dark Orchid */
   push_back_color_if_ok( bgc, QColor( 139, 0, 0 ) ); /* Dark Red */
   push_back_color_if_ok( bgc, QColor( 233, 150, 122 ) ); /* Dark Salmon */
   push_back_color_if_ok( bgc, QColor( 143, 188, 143 ) ); /* Dark Sea Green */
   push_back_color_if_ok( bgc, QColor( 72, 61, 139 ) ); /* Dark Slate Blue */
   push_back_color_if_ok( bgc, QColor( 47, 79, 79 ) ); /* Dark Slate Gray */
   push_back_color_if_ok( bgc, QColor( 0, 206, 209 ) ); /* Dark Turquoise */
   push_back_color_if_ok( bgc, QColor( 148, 0, 211 ) ); /* Dark Violet */
   push_back_color_if_ok( bgc, QColor( 255, 20, 147 ) ); /* Deep Pink */
   push_back_color_if_ok( bgc, QColor( 0, 191, 255 ) ); /* Deep Sky Blue */
   push_back_color_if_ok( bgc, QColor( 105, 105, 105 ) ); /* Dim Gray */
   push_back_color_if_ok( bgc, QColor( 30, 144, 255 ) ); /* Dodger Blue */
   push_back_color_if_ok( bgc, QColor( 178, 34, 34 ) ); /* Firebrick */
   push_back_color_if_ok( bgc, QColor( 255, 250, 240 ) ); /* Floral White */
   push_back_color_if_ok( bgc, QColor( 34, 139, 34 ) ); /* Forest Green */
   push_back_color_if_ok( bgc, QColor( 255, 0, 255 ) ); /* Fuschia* */
   push_back_color_if_ok( bgc, QColor( 220, 220, 220 ) ); /* Gainsboro */
   push_back_color_if_ok( bgc, QColor( 255, 250, 250 ) ); /* Ghost White */
   push_back_color_if_ok( bgc, QColor( 255, 215, 0 ) ); /* Gold */
   push_back_color_if_ok( bgc, QColor( 218, 165, 32 ) ); /* Goldenrod */
   push_back_color_if_ok( bgc, QColor( 128, 128, 128 ) ); /* Gray* */
   push_back_color_if_ok( bgc, QColor( 0, 128, 0 ) ); /* Green* */
   push_back_color_if_ok( bgc, QColor( 173, 255, 47 ) ); /* Green-Yellow */
   push_back_color_if_ok( bgc, QColor( 240, 255, 240 ) ); /* Honeydew */
   push_back_color_if_ok( bgc, QColor( 255, 105, 180 ) ); /* Hot Pink */
   push_back_color_if_ok( bgc, QColor( 205, 92, 92 ) ); /* Indian Red */
   push_back_color_if_ok( bgc, QColor( 255, 255, 240 ) ); /* Ivory */
   push_back_color_if_ok( bgc, QColor( 240, 230, 140 ) ); /* Khaki */
   push_back_color_if_ok( bgc, QColor( 230, 230, 250 ) ); /* Lavender */
   push_back_color_if_ok( bgc, QColor( 255, 240, 245 ) ); /* Lavender Blush */
   push_back_color_if_ok( bgc, QColor( 124, 252, 0 ) ); /* Lawn Green */
   push_back_color_if_ok( bgc, QColor( 255, 250, 205 ) ); /* Lemon Chiffon */
   push_back_color_if_ok( bgc, QColor( 173, 216, 230 ) ); /* Light Blue */
   push_back_color_if_ok( bgc, QColor( 240, 128, 128 ) ); /* Light Coral */
   push_back_color_if_ok( bgc, QColor( 224, 255, 255 ) ); /* Light Cyan */
   push_back_color_if_ok( bgc, QColor( 238, 221, 130 ) ); /* Light Goldenrod */
   push_back_color_if_ok( bgc, QColor( 250, 250, 210 ) ); /* Light Goldenrod Yellow */
   push_back_color_if_ok( bgc, QColor( 211, 211, 211 ) ); /* Light Gray */
   push_back_color_if_ok( bgc, QColor( 144, 238, 144 ) ); /* Light Green */
   push_back_color_if_ok( bgc, QColor( 255, 182, 193 ) ); /* Light Pink */
   push_back_color_if_ok( bgc, QColor( 255, 160, 122 ) ); /* Light Salmon */
   push_back_color_if_ok( bgc, QColor( 32, 178, 170 ) ); /* Light Sea Green */
   push_back_color_if_ok( bgc, QColor( 135, 206, 250 ) ); /* Light Sky Blue */
   push_back_color_if_ok( bgc, QColor( 132, 112, 255 ) ); /* Light Slate Blue */
   push_back_color_if_ok( bgc, QColor( 119, 136, 153 ) ); /* Light Slate Gray */
   push_back_color_if_ok( bgc, QColor( 176, 196, 222 ) ); /* Light Steel Blue */
   push_back_color_if_ok( bgc, QColor( 255, 255, 224 ) ); /* Light Yellow */
   push_back_color_if_ok( bgc, QColor( 0, 255, 0 ) ); /* Lime* */
   push_back_color_if_ok( bgc, QColor( 50, 205, 50 ) ); /* Lime Green */
   push_back_color_if_ok( bgc, QColor( 250, 240, 230 ) ); /* Linen */
   push_back_color_if_ok( bgc, QColor( 255, 0, 255 ) ); /* Magenta */
   push_back_color_if_ok( bgc, QColor( 128, 0, 0 ) ); /* Maroon* */
   push_back_color_if_ok( bgc, QColor( 102, 205, 170 ) ); /* Medium Aquamarine */
   push_back_color_if_ok( bgc, QColor( 0, 0, 205 ) ); /* Medium Blue */
   push_back_color_if_ok( bgc, QColor( 186, 85, 211 ) ); /* Medium Orchid */
   push_back_color_if_ok( bgc, QColor( 147, 112, 219 ) ); /* Medium Purple */
   push_back_color_if_ok( bgc, QColor( 60, 179, 113 ) ); /* Medium Sea Green */
   push_back_color_if_ok( bgc, QColor( 123, 104, 238 ) ); /* Medium Slate Blue */
   push_back_color_if_ok( bgc, QColor( 0, 250, 154 ) ); /* Medium Spring Green */
   push_back_color_if_ok( bgc, QColor( 72, 209, 204 ) ); /* Medium Turquoise */
   push_back_color_if_ok( bgc, QColor( 199, 21, 133 ) ); /* Medium Violet-Red */
   push_back_color_if_ok( bgc, QColor( 25, 25, 112 ) ); /* Midnight Blue */
   push_back_color_if_ok( bgc, QColor( 245, 255, 250 ) ); /* Mint Cream */
   push_back_color_if_ok( bgc, QColor( 255, 228, 225 ) ); /* Misty Rose */
   push_back_color_if_ok( bgc, QColor( 255, 228, 181 ) ); /* Moccasin */
   push_back_color_if_ok( bgc, QColor( 255, 222, 173 ) ); /* Navajo White */
   push_back_color_if_ok( bgc, QColor( 0, 0, 128 ) ); /* Navy* */
   push_back_color_if_ok( bgc, QColor( 253, 245, 230 ) ); /* Old Lace */
   push_back_color_if_ok( bgc, QColor( 128, 128, 0 ) ); /* Olive* */
   push_back_color_if_ok( bgc, QColor( 107, 142, 35 ) ); /* Olive Drab */
   push_back_color_if_ok( bgc, QColor( 255, 165, 0 ) ); /* Orange */
   push_back_color_if_ok( bgc, QColor( 255, 69, 0 ) ); /* Orange-Red */
   push_back_color_if_ok( bgc, QColor( 218, 112, 214 ) ); /* Orchid */
   push_back_color_if_ok( bgc, QColor( 238, 232, 170 ) ); /* Pale Goldenrod */
   push_back_color_if_ok( bgc, QColor( 152, 251, 152 ) ); /* Pale Green */
   push_back_color_if_ok( bgc, QColor( 175, 238, 238 ) ); /* Pale Turquoise */
   push_back_color_if_ok( bgc, QColor( 219, 112, 147 ) ); /* Pale Violet-Red */
   push_back_color_if_ok( bgc, QColor( 255, 239, 213 ) ); /* Papaya Whip */
   push_back_color_if_ok( bgc, QColor( 255, 218, 185 ) ); /* Peach Puff */
   push_back_color_if_ok( bgc, QColor( 205, 133, 63 ) ); /* Peru */
   push_back_color_if_ok( bgc, QColor( 255, 192, 203 ) ); /* Pink */
   push_back_color_if_ok( bgc, QColor( 221, 160, 221 ) ); /* Plum */
   push_back_color_if_ok( bgc, QColor( 176, 224, 230 ) ); /* Powder Blue */
   push_back_color_if_ok( bgc, QColor( 128, 0, 128 ) ); /* Purple* */
   push_back_color_if_ok( bgc, QColor( 255, 0, 0 ) ); /* Red* */
   push_back_color_if_ok( bgc, QColor( 188, 143, 143 ) ); /* Rosy Brown */
   push_back_color_if_ok( bgc, QColor( 65, 105, 225 ) ); /* Royal Blue */
   push_back_color_if_ok( bgc, QColor( 139, 69, 19 ) ); /* Saddle Brown */
   push_back_color_if_ok( bgc, QColor( 250, 128, 114 ) ); /* Salmon */
   push_back_color_if_ok( bgc, QColor( 244, 164, 96 ) ); /* Sandy Brown */
   push_back_color_if_ok( bgc, QColor( 46, 139, 87 ) ); /* Sea Green */
   push_back_color_if_ok( bgc, QColor( 255, 245, 238 ) ); /* Seashell */
   push_back_color_if_ok( bgc, QColor( 160, 82, 45 ) ); /* Sienna */
   push_back_color_if_ok( bgc, QColor( 192, 192, 192 ) ); /* Silver* */
   push_back_color_if_ok( bgc, QColor( 135, 206, 235 ) ); /* Sky Blue */
   push_back_color_if_ok( bgc, QColor( 106, 90, 205 ) ); /* Slate Blue */
   push_back_color_if_ok( bgc, QColor( 112, 128, 144 ) ); /* Slate Gray */
   push_back_color_if_ok( bgc, QColor( 255, 250, 250 ) ); /* Snow */
   push_back_color_if_ok( bgc, QColor( 0, 255, 127 ) ); /* Spring Green */
   push_back_color_if_ok( bgc, QColor( 70, 130, 180 ) ); /* Steel Blue */
   push_back_color_if_ok( bgc, QColor( 210, 180, 140 ) ); /* Tan */
   push_back_color_if_ok( bgc, QColor( 0, 128, 128 ) ); /* Teal* */
   push_back_color_if_ok( bgc, QColor( 216, 191, 216 ) ); /* Thistle */
   push_back_color_if_ok( bgc, QColor( 255, 99, 71 ) ); /* Tomato */
   push_back_color_if_ok( bgc, QColor( 64, 224, 208 ) ); /* Turquoise */
   push_back_color_if_ok( bgc, QColor( 238, 130, 238 ) ); /* Violet */
   push_back_color_if_ok( bgc, QColor( 208, 32, 144 ) ); /* Violet-Red */
   push_back_color_if_ok( bgc, QColor( 245, 222, 179 ) ); /* Wheat */
   push_back_color_if_ok( bgc, QColor( 255, 255, 255 ) ); /* White* */
   push_back_color_if_ok( bgc, QColor( 245, 245, 245 ) ); /* White Smoke */
   push_back_color_if_ok( bgc, QColor( 255, 255, 0 ) ); /* Yellow* */
   push_back_color_if_ok( bgc, QColor( 154, 205, 50 ) ); /* Yellow-Green */

   QString dctr_file = 
      USglobal->config_list.root_dir + QDir::separator() + "etc" + 
      QDir::separator() + "somo_mals_default_dctr.dat" ;
   if ( QFile( dctr_file ).exists() )
   {
      load_file( dctr_file );
   }

   QString mals_params_file = 
      USglobal->config_list.root_dir + QDir::separator() + "etc" + 
      QDir::separator() + "somo_mals_default_mals_param.dat" ;
   // set defaults always
   mals_param_lambda            = 0;
   mals_param_n                 = 0;
   mals_param_g_dndc            = 0;
   mals_param_g_extinction_coef = 0;
   mals_param_g_conc            = 0;
   mals_param_DLS_detector      = 0;
   if ( QFile( mals_params_file ).exists() ) {
      load_file( mals_params_file );
   }

   blanks_end_s = 0e0;
   blanks_end_e = 0e0;

   rb_pbmode_main->setChecked( true );
   pbmode_select( PBMODE_MAIN );
}

US_Hydrodyn_Mals::~US_Hydrodyn_Mals()
{
   ((US_Hydrodyn *)us_hydrodyn)->mals_widget = false;
   delete usu;
}

void US_Hydrodyn_Mals::push_back_color_if_ok( QColor bg, QColor set )
{
   double sum = 
      fabs( (float) bg.red  () - (float) set.red  () ) +
      fabs( (float) bg.green() - (float) set.green() ) +
      fabs( (float) bg.blue () - (float) set.blue () );
   if ( sum > 150 )
   {
      if ( plot_colors.size() )
      {
         bg = plot_colors.back();
         double sum = 
            fabs( (float) bg.red  () - (float) set.red  () ) +
            fabs( (float) bg.green() - (float) set.green() ) +
            fabs( (float) bg.blue () - (float) set.blue () );
         if ( sum > 100 )
         {
            plot_colors.push_back( set );
         }
      } else {
         plot_colors.push_back( set );
      }
   }
}

void US_Hydrodyn_Mals::cancel()
{
   close();
}

void US_Hydrodyn_Mals::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo/somo_mals.html");
}

void US_Hydrodyn_Mals::closeEvent(QCloseEvent *e)
{
   QMessageBox mb( this->windowTitle(), 
                   us_tr("Attention:\nAre you sure you want to exit the MALS window?"),
                  QMessageBox::Information,
                  QMessageBox::Yes | QMessageBox::Default,
                  QMessageBox::Cancel | QMessageBox::Escape,
                  QMessageBox::NoButton);
   mb.setButtonText(QMessageBox::Yes, us_tr("Yes"));
   mb.setButtonText(QMessageBox::Cancel, us_tr("Cancel"));
   switch(mb.exec())
   {
   case QMessageBox::Cancel:
      {
         e->ignore();
         return;
      }
   }

   QStringList created_not_saved_list;

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( created_files_not_saved.count( lb_files->item( i )->text() ) )
      {
         created_not_saved_list << lb_files->item( i )->text();
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
         qsl << QString( us_tr( "... and %1 more not listed" ) ).arg( created_not_saved_list.size() - qsl.size() );
      }

      switch ( QMessageBox::warning(this, 
                                    windowTitle(),
                                    QString( us_tr( "Please note:\n\n"
                                                 "These files were created but not saved as .dat files:\n"
                                                 "%1\n\n"
                                                 "What would you like to do?\n" ) )
                                    .arg( qsl.join( "\n" ) ),
                                    us_tr( "&Save them now" ), 
                                    us_tr( "&Close the window anyway" ), 
                                    us_tr( "&Quit from closing" ), 
                                    0, // Stop == button 0
                                    0 // Escape == button 0
                                    ) )
      {
      case 0 : // save them now
         // set the ones listed to selected
         if ( !save_files( created_not_saved_list ) )
         {
            e->ignore();
            return;
         }
         break;
      case 1 : // just ignore them
         break;
      case 2 : // quit
         e->ignore();
         return;
         break;
      }
   }

   {
      QStringList           model_not_saved_list;
      map < QString, bool > model_not_saved_map;

      for ( int i = 0; i < (int)lb_model_files->count(); i++ )
      {
         if ( models_not_saved.count( lb_model_files->item( i )->text() ) )
         {
            model_not_saved_list << lb_model_files->item( i )->text();
            model_not_saved_map[ lb_model_files->item( i )->text() ] = true;
         }
      }

      if ( model_not_saved_list.size() )
      {
         QStringList qsl;
         for ( int i = 0; i < (int)model_not_saved_list.size() && i < 15; i++ )
         {
            qsl << model_not_saved_list[ i ];
         }

         if ( qsl.size() < model_not_saved_list.size() )
         {
            qsl << QString( us_tr( "... and %1 more not listed" ) ).arg( model_not_saved_list.size() - qsl.size() );
         }

         switch ( QMessageBox::warning(this, 
                                       windowTitle() + us_tr( " Remove Models" ),
                                       QString( us_tr( "Please note:\n\n"
                                                    "These models were created but not saved as .bead_model files:\n"
                                                    "%1\n\n"
                                                    "What would you like to do?\n" ) )
                                       .arg( qsl.join( "\n" ) ),
                                       us_tr( "&Save them now" ), 
                                       us_tr( "&Close the window anyway" ), 
                                       us_tr( "&Quit from closing" ), 
                                       0, // Stop == button 0
                                       0 // Escape == button 0
                                       ) )
         {
         case 0 : // save them now
            // set the ones listed to selected
            if ( !model_save( model_not_saved_list ) )
            {
               e->ignore();
               return;
            }
         case 1 : // just remove them
            break;
         case 2 : // quit
            disable_updates = false;
            e->ignore();
            return;
            break;
         }
      }
   }


   ((US_Hydrodyn *)us_hydrodyn)->mals_widget = false;
   ((US_Hydrodyn *)us_hydrodyn)->last_mals_csv = current_csv();
   if ( conc_widget )
   {
      conc_window->close();
   }

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Mals::clear_display()
{
   editor->clear( );
   editor->append("\n\n");
}

void US_Hydrodyn_Mals::update_font()
{
   bool ok;
   QFont newFont;
   newFont = QFontDialog::getFont( &ok, ft, this );
   if ( ok )
   {
      ft = newFont;
   }
   editor->setFont(ft);
}

void US_Hydrodyn_Mals::save()
{
   QString fn;
   fn = QFileDialog::getSaveFileName( this , windowTitle() , QString() , QString() );
   if(!fn.isEmpty() )
   {
      QString text = editor->toPlainText();
      QFile f( fn );
      if ( !f.open( QIODevice::WriteOnly | QIODevice::Text) )
      {
         return;
      }
      QTextStream t( &f );
      t << text;
      f.close();
 //      editor->setModified( false );
      setWindowTitle( fn );
   }
}

void US_Hydrodyn_Mals::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->textColor();
   editor->setTextColor(color);
   editor->append(msg);
   editor->setTextColor(save_color);
   if ( !editor_widgets[ 0 ]->isVisible() && color == "red" && !msg.trimmed().isEmpty() )
   {
      lbl_editor->setPalette( cg_red );
   }
}

void US_Hydrodyn_Mals::editor_msg_qc( QColor qcolor, QString msg )
{
   QColor save_color = editor->textColor();
   editor->setTextColor(qcolor);
   editor->append(msg);
   editor->setTextColor(save_color);
   if ( !editor_widgets[ 0 ]->isVisible() && qcolor == QColor( "red" ) && !msg.trimmed().isEmpty() )
   {
      lbl_editor->setPalette( cg_red );
   }
}

bool US_Hydrodyn_Mals::activate_saxs_window()
{
   if ( !*saxs_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->pdb_saxs();
      raise();
      setFocus();
      if ( !*saxs_widget )
      {
         editor_msg("red", us_tr("Could not activate SAXS window!\n"));
         return false;
      }
   }
   saxs_window = ((US_Hydrodyn *) us_hydrodyn)->saxs_plot_window;
   return true;
}

csv US_Hydrodyn_Mals::current_csv()
{
   csv tmp_csv = csv1;
   
   return tmp_csv;
}
  
void US_Hydrodyn_Mals::clear_files()
{
   disable_all();
   QStringList files = all_selected_files();
   clear_files( files );
   update_enables();
}


void US_Hydrodyn_Mals::remove_files( set < QString > & fileset, bool replot )
{
   QStringList files;
   for ( set < QString >::iterator it = fileset.begin();
         it != fileset.end();
         ++it )
   {
      files << *it;
   }
   clear_files( files, true );
   if ( replot )
   {
      plot_files();
   }
}

void US_Hydrodyn_Mals::clear_files( QStringList files, bool quiet )
{
   disable_updates = true;

   QStringList           created_not_saved_list;
   map < QString, bool > created_not_saved_map;
   map < QString, bool > selected_map;

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

   if ( !quiet && created_not_saved_list.size() )
   {
      QStringList qsl;
      for ( int i = 0; i < (int)created_not_saved_list.size() && i < 15; i++ )
      {
         qsl << created_not_saved_list[ i ];
      }

      if ( qsl.size() < created_not_saved_list.size() )
      {
         qsl << QString( us_tr( "... and %1 more not listed" ) ).arg( created_not_saved_list.size() - qsl.size() );
      }

      switch ( QMessageBox::warning(this, 
                                    windowTitle() + us_tr( " Remove Files" ),
                                    QString( us_tr( "Please note:\n\n"
                                                 "These files were created but not saved as .dat files:\n"
                                                 "%1\n\n"
                                                 "What would you like to do?\n" ) )
                                    .arg( qsl.join( "\n" ) ),
                                    us_tr( "&Save them now" ), 
                                    us_tr( "&Remove them anyway" ), 
                                    us_tr( "&Quit from removing files" ), 
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

   // remove them now
   lb_created_files->setUpdatesEnabled( false );

   for ( int i = lb_created_files->count() - 1; i >= 0; i-- )
   {
      if ( selected_map.count( lb_created_files->item( i )->text() ) )
      {
         created_files_not_saved.erase( lb_created_files->item( i )->text() );
         delete lb_created_files->takeItem( i );
      }
   }

   lb_created_files->setUpdatesEnabled( true );

   lb_files->setUpdatesEnabled( false );

   QString msg = "";

   for ( int i = lb_files->count() - 1; i >= 0; i-- )
   {
      if ( selected_map.count( lb_files->item( i )->text() ) )
      {
         if ( !quiet ) {
            msg += QString( us_tr( "Removed %1\n" ) ).arg( lb_files->item( i )->text() );
         }
         conc_files    .erase( lb_files->item( i )->text() );
         if ( lbl_conc_file->text() == lb_files->item( i )->text() )
         {
            lbl_conc_file->setText( "" );
         }
         //          if ( lbl_mals->text() == lb_files->item( i )->text() )
         //          {
         //             lbl_mals->setText( "" );
         //          }
         //          if ( lbl_signal->text() == lb_files->item( i )->text() )
         //          {
         //             lbl_signal->setText( "" );
         //          }
         //          if ( lbl_empty->text() == lb_files->item( i )->text() )
         //          {
         //             lbl_empty->setText( "" );
         //          }
         f_qs_string      .erase( lb_files->item( i )->text() );
         f_qs             .erase( lb_files->item( i )->text() );
         f_Is             .erase( lb_files->item( i )->text() );
         f_errors         .erase( lb_files->item( i )->text() );
         f_pos            .erase( lb_files->item( i )->text() );
         f_name           .erase( lb_files->item( i )->text() );
         f_is_time        .erase( lb_files->item( i )->text() );
         f_gaussians      .erase( lb_files->item( i )->text() );
         f_psv            .erase( lb_files->item( i )->text() );
         f_header         .erase( lb_files->item( i )->text() );
         f_I0se           .erase( lb_files->item( i )->text() );
         f_conc           .erase( lb_files->item( i )->text() );
         f_extc           .erase( lb_files->item( i )->text() );
         f_time           .erase( lb_files->item( i )->text() );
         f_g_dndc         .erase( lb_files->item( i )->text() );
         f_dndc           .erase( lb_files->item( i )->text() );
         f_diffusion_len  .erase( lb_files->item( i )->text() );
         f_e_nucleon_ratio.erase( lb_files->item( i )->text() );
         f_nucleon_mass   .erase( lb_files->item( i )->text() );
         f_solvent_e_dens .erase( lb_files->item( i )->text() );
         f_I0st           .erase( lb_files->item( i )->text() );
         f_conc_units     .erase( lb_files->item( i )->text() );
         f_ri_corr        .erase( lb_files->item( i )->text() );
         f_ri_corrs       .erase( lb_files->item( i )->text() );
         f_ref_index      .erase( lb_files->item( i )->text() );
         f_fit_curve      .erase( lb_files->item( i )->text() );
         f_fit_method     .erase( lb_files->item( i )->text() );
         f_fit_q_ranges   .erase( lb_files->item( i )->text() );
         f_fit_chi2       .erase( lb_files->item( i )->text() );
         f_fit_sd_scale   .erase( lb_files->item( i )->text() );

         delete lb_files->takeItem( i );
         // qApp->processEvents();
      }
   }

   lb_files->setUpdatesEnabled( true );
   
   if ( !quiet && !msg.isEmpty() ) {
      editor_msg( "black", msg );
   }

   disable_updates = false;
   plot_files();
   // if ( !lb_files->count() &&
   //      plot_dist_zoomer )
   // {
   //    delete plot_dist_zoomer;
   //    plot_dist_zoomer = (ScrollZoomer *) 0;
   // }
   update_csv_conc();
   if ( conc_widget )
   {
      if ( lb_files->count() )
      {
         conc_window->refresh( csv_conc );
      } else {
         conc_window->cancel();
      }
   }
   qApp->processEvents();
   repaint();
}

class mals_sortable_qstring {
public:
   double       x;
   QString      name;
   bool operator < (const mals_sortable_qstring& objIn) const
   {
      return x < objIn.x;
   }
};

void US_Hydrodyn_Mals::load_conc()
{
   add_files( true );
   rb_pbmode_conc->setChecked( true );
   pbmode_select( PBMODE_CONC );
}

void US_Hydrodyn_Mals::add_dir() {
   add_files( false, true );
}

void US_Hydrodyn_Mals::add_files( bool load_conc, bool from_dir ) {
   disable_all();
   map < QString, bool > existing_items;
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->saxs_options.iq_scale_angstrom ) {
      editor_msg( "blue", us_tr( "Notice: Unless specifically marked in the header, q values of I(q) files loaded will be converted from 1/nm to 1/Angstrom." ) );
   }
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      existing_items[ lb_files->item( i )->text() ] = true;
   }

   if ( cb_lock_dir->isChecked() )
   {
      QDir::setCurrent( le_dir->text() );
   }

   QString use_dir = QDir::currentPath();
   
   if ( !*saxs_widget )
   {
      // try and activate
      ((US_Hydrodyn *)us_hydrodyn)->pdb_saxs();
      raise();
   }

   // if ( *saxs_widget )
   // {
   if ( cb_lock_dir->isChecked() )
   {
      ((US_Hydrodyn  *)us_hydrodyn)->add_to_directory_history( le_dir->text() );
   }
   bool not_ok = ((US_Hydrodyn  *)us_hydrodyn)->select_from_directory_history( use_dir, this );
   raise();
   // }

   QStringList filenames;

   if ( from_dir ) {
      QString dirname = use_dir;
      if ( !not_ok ) {
         QDir up_one( use_dir );
         up_one.cdUp();
         dirname = QFileDialog::getExistingDirectory( this
                                                      ,"Add all .dat files in directory"
                                                      ,up_one.canonicalPath()
                                                      ,QFileDialog::ShowDirsOnly );
      }
      if ( !dirname.isEmpty() ) {
         QDir dir( dirname );
         QStringList filters;
         filters << "*.dat"; 
         filenames = dir.entryList( filters ).replaceInStrings( QRegExp( "^" ), dirname + "/" );
      }
   } else {
      if ( load_conc )
      {
         filenames = QFileDialog::getOpenFileNames( this , "Load concentration files" , use_dir , "txt [specify q, I, sigma columns] and csv concentration files (*.txt *.csv);;"
                                                    "dat files [foxs / other] (*.dat);;"
                                                    "All files (*);;"
                                                    "ssaxs files (*.ssaxs);;"
                                                    "txt files [specify q, I, sigma columns] (*.txt);;"
                                                    "csv files (*.csv)" );

         // QString filename = 
         //    QFileDialog::getOpenFileName(
         //                                  use_dir
         //                                  , "txt files [specify q, I, sigma columns] (*.txt);;"
         //                                  "dat files [foxs / other] (*.dat);;"
         //                                  "ssaxs files (*.ssaxs);;"
         //                                  "All files (*);;"
         //                                  , this
         //                                  , "open file dialog"
         //                                  , "Load concfiles" // ? "Set files for grid files"
         //                                  );
         // if ( !filename.isEmpty() )
         // {
         //    filenames << filename;
         // }
      } else {
         filenames = QFileDialog::getOpenFileNames( this , "Add files" , use_dir
                                                    ,
                                                    "All files (*);;"
                                                    "csv files [MALS data and Angles] (*.csv);;"
                                                    "dat files [foxs / other] (*.dat);;"
                                                    "ssaxs files (*.ssaxs);;"
                                                    "txt files [Wyatt MALS Angles or specified q I sd] (*.txt)"
                                                    );
      }
   }
   
   QStringList add_filenames;
   
   if ( filenames.size() )
   {
      last_load_dir = QFileInfo( filenames[ 0 ] ).path();
      if ( !cb_lock_dir->isChecked() )
      {
         QDir::setCurrent( last_load_dir );
         le_dir        ->setText( QDir::currentPath() );
         le_created_dir->setText( QDir::currentPath() + "/produced" ); 
      }
      editor_msg( "black", QString( us_tr( "loaded from %1:" ) ).arg( last_load_dir ) );
   }

   map < QString, double > found_times;

   // #define DEBUG_LOAD_REORDER

   if ( filenames.size() > 1 )
   {
      bool reorder = true;

      QRegExp rx_cap( "(\\d+)_(\\d+)(\\D|$)" );
      QRegExp rx_clear_nonnumeric( "^(\\d*_?\\d+)([^0-9_]|_[a-zA-Z])" );
      // rx_cap.setMinimal( true );

      list < mals_sortable_qstring > svals;

      QString head = qstring_common_head( filenames, true );
      QString tail = qstring_common_tail( filenames, true );

      bool do_prepend          = false;
      QString prepend_tmp = "";
      {
         QRegExp rx_dp( "_q(\\d)_$" );
         if ( rx_dp.indexIn( head ) != -1 )
         {
            prepend_tmp = rx_dp.cap( 1 ) + ".";
            do_prepend = true;
         }
      }

#ifdef DEBUG_LOAD_REORDER
      us_qdebug( QString( "sort head <%1> tail <%2>  prepend %3 <%4>" ).arg( head ).arg( tail ).arg( do_prepend ? "yes" : "no" ) . arg( prepend_tmp ) );
#endif
      
      set < QString > used;

      bool added_dp = false;

      for ( int i = 0; i < (int) filenames.size(); ++i )
      {
         QString tmp = filenames[ i ].mid( head.length() );
         tmp = tmp.mid( 0, tmp.length() - tail.length() );

         if ( rx_clear_nonnumeric.indexIn( tmp ) != -1 )
         {
            tmp = rx_clear_nonnumeric.cap( 1 );
         }

         added_dp = false;

         if ( rx_cap.indexIn( tmp ) != -1 )
         {
#ifdef DEBUG_LOAD_REORDER
            us_qdebug( QString( "rx_cap search tmp %1 found" ).arg( tmp ) );
#endif
            tmp = rx_cap.cap( 1 ) + "." + rx_cap.cap( 2 );
            added_dp = true;
#ifdef DEBUG_LOAD_REORDER
         } else {
            us_qdebug( QString( "rx_cap search tmp %1 NOT found" ).arg( tmp ) );
#endif
         }

         if ( do_prepend )
         {
            if ( added_dp )
            {
               QMessageBox::warning( this, 
                                     windowTitle()+ us_tr( ": Add files" ),
                                     us_tr( "I am having a problem decoding the frame numbers or q values from the file names\n"
                                         "Please email a list of the file names you are trying to load to emre.brookes@umt.edu" ) );
               return;
            }
            tmp = prepend_tmp + tmp;
         }

#ifdef DEBUG_LOAD_REORDER
         us_qdebug( QString( "tmp is now %1 double is %2" ).arg( tmp ).arg( tmp.toDouble() ) );
#endif

         if ( used.count( tmp ) )
         {
#ifdef DEBUG_LOAD_REORDER
            us_qdebug( QString( "rx_cap used exit <%1>" ).arg( tmp ) );
#endif
            reorder = false;
            break;
         }
         used.insert( tmp );

         mals_sortable_qstring sval;
         sval.x     = tmp.toDouble();
         sval.name  = filenames[ i ];
         svals      .push_back( sval );
#ifdef DEBUG_LOAD_REORDER
         us_qdebug( QString( "sort %1 tmp <%2> xval <%3>" ).arg( sval.name ).arg( tmp ).arg( sval.x ) );
#endif
      }
      if ( reorder )
      {
#ifdef DEBUG_LOAD_REORDER
         us_qdebug( "reordered" );
#endif
         svals.sort();

         filenames.clear( );
         for ( list < mals_sortable_qstring >::iterator it = svals.begin();
               it != svals.end();
               ++it )
         {
            filenames << it->name;
            found_times[ it->name ] = it->x;
         }
      }
   } else {
      if ( filenames.size() ) {
         QRegExp rx_time( "_t(\\d+)" );
         if ( rx_time.indexIn( filenames[ 0 ] ) != -1 ) {
            found_times[ filenames[ 0 ] ] = rx_time.cap( 1 ).toDouble();
         }
      }
   }

   QString errors;
   QString warnings;

   for ( int i = 0; i < (int)filenames.size(); i++ )
   {
      ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( filenames[ i ] );

      QString basename = QFileInfo( filenames[ i ] ).completeBaseName();
      if ( !existing_items.count( basename ) )
      {
         if ( !load_file( filenames[ i ], load_conc ) )
         {
            errors += errormsg + "\n";
         } else {
            editor_msg( "black", QString( us_tr( "%1" ) ).arg( basename ) );
            add_filenames << basename;
            if ( found_times.count( filenames[ i ] ) ) {
               if ( f_time.count( basename ) &&
                    f_time[ basename ] != found_times[ filenames[ i ] ] ) {
                  warnings += QString( "File %1 has a conflicting stored time %2 vs the time extracted from the name %3, using the stored time\n" )
                     .arg( basename )
                     .arg( f_time[ basename ] )
                     .arg( found_times[ filenames[ i ] ] )
                     ;
               } else {
                  f_time[ basename ] = found_times[ filenames[ i ] ];
               }
            }
            if ( load_conc )
            {
               f_is_time[ basename ] = true;
               conc_files.insert( basename );
            }
         }
         qApp->processEvents();
      } else {
         errors += QString( us_tr( "Duplicate name not loaded %1%2" ) ).arg( basename ).arg( errors.isEmpty() ? "" : "\n" );
      }
   }

   {
      set < double > ref_indices;
      
      for ( auto const & name : add_filenames ) {
         if ( f_ref_index.count( name ) &&
              f_ref_index[name] != mals_param_n ) {
            ref_indices.insert( f_ref_index[name] );
         }
      }

      if ( ref_indices.size() == 1 ) {
         QMessageBox::warning( this, 
                               windowTitle() + us_tr( " : Add files" ),
                               QString(
                                       us_tr(
                                             "Loaded files contain a different solvent refractive index (%1)\n"
                                             "than set in the Options->MALS parameters->Solvent refractive index (%2)\n"
                                             "You should remove all files, adjust the parameter value and reload the files"
                                             )
                                       )
                               .arg( *ref_indices.begin() )
                               .arg( mals_param_n )
                               );
      } 
      if ( ref_indices.size() > 1 ) {
         QString indices = "";
         for ( auto const & ref_index : ref_indices ) {
            indices += QString( "%1 " ).arg( ref_index );
         }
         
         QMessageBox::warning( this, 
                               windowTitle() + us_tr( " : Add files" ),
                               QString(
                                       us_tr(
                                             "Loaded files contain multiple different solvent refractive index (%1)\n"
                                             "which do not match the Options->MALS parameters->Solvent refractive index (%2)\n"
                                             "Proceed with caution!"
                                             )
                                       )
                               .arg( indices )
                               .arg( mals_param_n )
                               );
      } 
   }

   if ( errors.isEmpty() )
   {
      editor_msg( "blue", us_tr( "Files loaded ok" ) );
   } else {
      editor_msg( "red", errors );
   }

   if ( !warnings.isEmpty() ) {
      editor_msg( "dark red", warnings );
   }

   lb_files->addItems( add_filenames );

   if ( add_filenames.size() &&
        existing_items.size() )
   {
      lb_files->scrollToItem( lb_files->item( existing_items.size() ) );
   }

   if ( add_filenames.size() && plot_dist_zoomer )
   {
      // we should only do this if the ranges are changed
      // plot_dist_zoomer->zoom ( 0 );
      // delete plot_dist_zoomer;
      // plot_dist_zoomer = (ScrollZoomer *) 0;
      plot_files();
   }
   update_csv_conc();

   {
      map < QString, bool > add_with_conc;

      for ( unsigned int i = 0; i < ( unsigned int ) add_filenames.size(); i++ )
      {
         if ( f_conc.count( add_filenames[ i ] ) && f_conc[ add_filenames[ i ] ] != 0e0 )
         {
            add_with_conc[ add_filenames[ i ] ] = true;
            // TSO << QString( "add file found conc %1 %2\n" ).arg( add_filenames[ i ] ).arg( f_conc[ add_filenames[ i ] ] );
         }
      }

      if( add_with_conc.size() )
      {
         for ( unsigned int i = 0; i < csv_conc.data.size(); i++ )
         {
            if ( csv_conc.data[ i ].size() > 1 &&
                 add_with_conc.count( csv_conc.data[ i ][ 0 ] ) )
            {
               csv_conc.data[ i ][ 1 ] = QString( "%1" ).arg( f_conc[ csv_conc.data[ i ][ 0 ] ] );
               add_with_conc.erase( csv_conc.data[ i ][ 0 ] );
            }
         }
         if( add_with_conc.size() )
         {
            editor_msg( "red", QString( us_tr( "Internal error: could not set concentrations %1 files loaded" ).arg( add_with_conc.size() ) ) );
         }
      }
   }

   if ( conc_widget )
   {
      conc_window->refresh( csv_conc );
   }
   update_enables();
}

void US_Hydrodyn_Mals::add_files( QStringList filenames )
{
   map < QString, bool > existing_items;
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      existing_items[ lb_files->item( i )->text() ] = true;
   }

   QString use_dir = QDir::currentPath();
   
   QStringList add_filenames;
   
   if ( filenames.size() )
   {
      last_load_dir = QFileInfo( filenames[ 0 ] ).path();
      if ( !cb_lock_dir->isChecked() )
      {
         QDir::setCurrent( last_load_dir );
         le_dir        ->setText( QDir::currentPath() );
         le_created_dir->setText( QDir::currentPath() + "/produced" ); 
      }
      editor_msg( "black", QString( us_tr( "loaded from %1:" ) ).arg( last_load_dir ) );
   }

   QString errors;

   for ( int i = 0; i < (int)filenames.size(); i++ )
   {
      ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( filenames[ i ] );;

      QString basename = QFileInfo( filenames[ i ] ).completeBaseName();
      if ( !existing_items.count( basename ) )
      {
         if ( !load_file( filenames[ i ] ) )
         {
            errors += errormsg + "\n";
         } else {
            editor_msg( "black", QString( us_tr( "%1" ) ).arg( basename ) );
            add_filenames << basename;
         }
         qApp->processEvents();
      } else {
         errors += QString( us_tr( "Duplicate name not loaded %1%2" ) ).arg( basename ).arg( errors.isEmpty() ? "" : "\n" );
      }
   }

   if ( errors.isEmpty() )
   {
      editor_msg( "blue", us_tr( "Files loaded ok" ) );
   } else {
      editor_msg( "red", errors );
   }

   lb_files->addItems( add_filenames );

   if ( add_filenames.size() &&
        existing_items.size() )
   {
      lb_files->scrollToItem( lb_files->item( existing_items.size() ) );
   }

   if ( add_filenames.size() && plot_dist_zoomer )
   {
      // we should only do this if the ranges are changed
      // plot_dist_zoomer->zoom ( 0 );
      // delete plot_dist_zoomer;
      // plot_dist_zoomer = (ScrollZoomer *) 0;
      plot_files();
   }
   update_csv_conc();

   {
      map < QString, bool > add_with_conc;

      for ( unsigned int i = 0; i < ( unsigned int ) add_filenames.size(); i++ )
      {
         if ( f_conc.count( add_filenames[ i ] ) && f_conc[ add_filenames[ i ] ] != 0e0 )
         {
            add_with_conc[ add_filenames[ i ] ] = true;
            // TSO << QString( "add file found conc %1 %2\n" ).arg( add_filenames[ i ] ).arg( f_conc[ add_filenames[ i ] ] );
         }
      }

      if( add_with_conc.size() )
      {
         for ( unsigned int i = 0; i < csv_conc.data.size(); i++ )
         {
            if ( csv_conc.data[ i ].size() > 1 &&
                 add_with_conc.count( csv_conc.data[ i ][ 0 ] ) )
            {
               csv_conc.data[ i ][ 1 ] = QString( "%1" ).arg( f_conc[ csv_conc.data[ i ][ 0 ] ] );
               add_with_conc.erase( csv_conc.data[ i ][ 0 ] );
            }
         }
         if( add_with_conc.size() )
         {
            editor_msg( "red", QString( us_tr( "Internal error: could not set concentrations %1 files loaded" ).arg( add_with_conc.size() ) ) );
         }
      }
   }

   if ( conc_widget )
   {
      conc_window->refresh( csv_conc );
   }
   update_enables();
}



void US_Hydrodyn_Mals::update_files()
{
   if ( !disable_updates )
   {
      plot_files();
      update_enables();
   }
}

void US_Hydrodyn_Mals::invert()
{
   disable_all();

   set < int > was_selected;
   for ( int i = 0; i < lb_files->count(); i++ ) {
      if ( lb_files->item( i )->isSelected() ) {
         was_selected.insert( i );
      }
   }

   disable_updates = true;
   for ( int i = 0; i < lb_files->count(); i++ ) {
      lb_files->item( i)->setSelected( !was_selected.count( i ) );
   }
   disable_updates = false;
   plot_files();
   update_enables();
}

void US_Hydrodyn_Mals::select_all()
{
   bool all_selected = true;
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( !lb_files->item( i )->isSelected() )
      {
         all_selected = false;
         break;
      }
   }

   disable_updates = true;
   
   !all_selected ? lb_files->selectAll() : lb_files->clearSelection();

   // for ( int i = 0; i < lb_files->count(); i++ )
   // {
   //    lb_files->item( i)->setSelected( !all_selected );
   // }

   disable_updates = false;
   plot_files();
   update_enables();
}

void US_Hydrodyn_Mals::select_all_created()
{
   bool all_selected = true;
   for ( int i = 0; i < lb_created_files->count(); i++ )
   {
      if ( !lb_created_files->item( i )->isSelected() )
      {
         all_selected = false;
         break;
      }
   }

   disable_updates = true;
   for ( int i = 0; i < lb_created_files->count(); i++ )
   {
      lb_created_files->item( i)->setSelected( !all_selected );
   }
   disable_updates = false;
   update_enables();
}

bool US_Hydrodyn_Mals::load_file( QString filename, bool load_conc )
{
   errormsg = "";
   QFile f( filename );
   if ( !f.exists() )
   {
      errormsg = QString("Error: %1 does not exist").arg( filename );
      return false;
   }
   // TSO << QString( "opening %1\n" ).arg( filename ) << flush;
   
   QString ext = QFileInfo( filename ).suffix().toLower();

   QRegExp rx_valid_ext (
                         "^("
                         "dat|"
                         "int|"
                         "txt|"
                         "csv|"
                         "sprr|"
                         // "out|"
                         "ssaxs)$" );

   if ( rx_valid_ext.indexIn( ext ) == -1 )
   {
      errormsg = QString("Error: %1 unsupported file extension %2").arg( filename ).arg( ext );
      return false;
   }
      
   if ( !f.open( QIODevice::ReadOnly ) )
   {
      errormsg = QString("Error: can not open %1, check permissions ").arg( filename );
      return false;
   }

   QTextStream ts(&f);
   vector < QString > qv;
   QStringList qsl;
   QStringList qsl_nb;

   while ( !ts.atEnd() )
   {
      QString qs = ts.readLine();
      qv.push_back( qs );

      qsl << qs;

      if ( !qs.contains( QRegularExpression( "^\\s*$" ) ) ) {
         qsl_nb << qs.trimmed();
      }
   }
   f.close();

   if ( !qv.size() )
   {
      errormsg = QString("Error: the file %1 is empty ").arg( filename );
      return false;
   }

   if ( mals_angles.populate( filename, qsl_nb ) ) {
      editor_msg( "black", QString( "%1 MALS angles loaded\n" ).arg( filename ) );
      editor_msg( "black", mals_angles.list() );
      lbl_mals_angles_data->setText( mals_angles.list_rich( mals_param_lambda, mals_param_n ) );
      return false;
   }

   bool    is_time         = false;

   bool    has_conc                 = false;
   double  this_conc                = 0e0;
   bool    has_psv                  = false;
   double  this_psv                 = 0e0;
   bool    has_I0se                 = false;
   double  this_I0se                = 0e0;
   bool    has_I0st                 = false;
   double  this_I0st                = 0e0;
   bool    has_time                 = false;
   double  this_time                = 0e0;
   bool    has_extc                 = false;
   double  this_extc                = 0e0;
   bool    has_g_dndc               = false;
   double  this_g_dndc              = 0e0;
   bool    has_dndc                 = false;
   double  this_dndc                = 0e0;
   bool    has_conc_units           = false;
   QString this_conc_units          = "";
   bool    has_ref_index            = false;
   double  this_ref_index           = 0e0;
   bool    has_diffusion_len        = false;
   double  this_diffusion_len       = 0e0;
   bool    has_e_nucleon_ratio      = false;
   double  this_e_nucleon_ratio     = 0e0;
   bool    has_nucleon_mass         = false;
   double  this_nucleon_mass        = 0e0;
   bool    has_solvent_e_dens       = false;
   double  this_solvent_e_dens      = 0e0;
   bool    has_ri_corr              = false;
   double  this_ri_corr             = 0e0;
   bool    has_ri_corrs             = false;
   QString this_ri_corrs            = "";
   bool    has_fit_curve            = false;
   QString this_fit_curve           = "";
   bool    has_fit_method           = false;
   QString this_fit_method          = "";
   bool    has_fit_q_ranges         = false;
   QString this_fit_q_ranges        = "";
   bool    has_fit_chi2             = false;
   double  this_fit_chi2            = 0e0;
   bool    has_fit_sd_scale         = false;
   double  this_fit_sd_scale        = 0e0;

   double use_units   = ( ( US_Hydrodyn * ) us_hydrodyn )->saxs_options.iq_scale_angstrom ? 1.0 : 0.1;

   if ( ext == "dat" ||
        ext == "sprr" )
   {
      QRegExp rx_conc               ( "Conc:\\s*(\\S+)(\\s|$)" );
      QRegExp rx_conc_units         ( "Conc:\\s*\\S+\\s+(\\[\\S+\\])(\\s|$)" );
      QRegExp rx_psv                ( " PSV:\\s*(\\S+)(\\s|$)" );
      QRegExp rx_I0se               ( " I0se:\\s*(\\S+)(\\s|$)" );
      QRegExp rx_I0st               ( " I0st:\\s*(\\S+)(\\s|$)" );
      QRegExp rx_time               ( " Time:\\s*(\\S+)(\\s|$)" );
      QRegExp rx_unit               ( " Units:\\s*(\\S+)(\\s|$)" );
      QRegExp rx_extc               ( " (?:ExtC_or_DRIinc|ExtC):\\s*(\\S+)(\\s|$)" );
      QRegExp rx_g_dndc             ( " Global_dndc:\\s*(\\S+)(\\s|$)" );
      QRegExp rx_dndc               ( " dndc:\\s*(\\S+)(\\s|$)" );
      QRegExp rx_ref_index          ( " Solvent refractive index:\\s*(\\S+)(\\s|$)" );
      QRegExp rx_diffusion_len      ( " Diffusion Length:\\s*(\\S+)(\\s|$)" );
      QRegExp rx_e_nucleon_ratio    ( " Electron/nucleon ratio Z/A:\\s*(\\S+)(\\s|$)" );
      QRegExp rx_nucleon_mass       ( " Nucleon mass:\\s*(\\S+)(\\s|$)" );
      QRegExp rx_solvent_e_dens     ( " Solvent e density:\\s*(\\S+)(\\s|$)" );
      QRegExp rx_ri_corr            ( " RI-Corr scatt\\. angle:\\s*(\\S+)(\\s|$)" );
      QRegExp rx_ri_corrs           ( " RI-Corr scatt\\. angles:\\s*(\\S+)(\\s|$)" );
      QRegExp rx_fit_curve          ( " Fit curve:\\s*(\\S+)(\\s|$)" );
      QRegExp rx_fit_method         ( " Fit method:\\s*(\\S+)(\\s|$)" );
      QRegExp rx_fit_q_ranges       ( " Fit ranges:\\s*(\\S+)(\\s|$)" );
      QRegExp rx_fit_chi2           ( " Fit chi^2:\\s*(\\S+)(\\s|$)" );
      QRegExp rx_fit_sd_scale       ( " Fit MALS SD mult:\\s*(\\S+)(\\s|$)" );
      
      if ( rx_unit.indexIn( qv[ 0 ] ) != -1 ) {
         QString unitstr = rx_unit.cap( 1 ).toLower();
         bool ok = false;
         if ( !ok && unitstr.contains( QRegExp( "^(1/nm|nm^-1)$" ) ) ) {
            use_units = 0.1;
            ok = true;
         }
         if ( !ok && unitstr.contains( QRegExp( "^(1/a|a^-1)$" ) ) ) {
            use_units = 1.0;
            ok = true;
         }
         if ( !ok ) {
            editor_msg( "black", QString( us_tr( "%1 - unknown Units: %2 specified, must be 1/A or 1/NM, using specified default conversion of %3") ).arg( filename ).arg( rx_unit.cap( 1 ) ).arg( use_units ) );
         }
      }
      if ( rx_conc.indexIn( qv[ 0 ] ) != -1 ) {
         has_conc  = true;
         this_conc = rx_conc.cap( 1 ).toDouble();
         // TSO << QString( "found conc %1\n" ).arg( this_conc );
      }
      if ( rx_conc_units.indexIn( qv[ 0 ] ) != -1 ) {
         has_conc_units  = true;
         this_conc_units = rx_conc.cap( 1 );
         // TSO << QString( "found conc %1\n" ).arg( this_conc );
      }
      if ( rx_psv.indexIn( qv[ 0 ] ) != -1 ) {
         has_psv  = true;
         this_psv = rx_psv.cap( 1 ).toDouble();
      }
      if ( rx_I0se.indexIn( qv[ 0 ] ) != -1 ) {
         has_I0se  = true;
         this_I0se = rx_I0se.cap( 1 ).toDouble();
      }
      if ( rx_I0st.indexIn( qv[ 0 ] ) != -1 ) {
         has_I0st  = true;
         this_I0st = rx_I0st.cap( 1 ).toDouble();
      }
      if ( rx_time.indexIn( qv[ 0 ] ) != -1 ) {
         has_time  = true;
         this_time = rx_time.cap( 1 ).replace( "_", "." ).replace( QRegularExpression( "^0+" ), "0" ).toDouble();
      }
      if ( rx_extc.indexIn( qv[ 0 ] ) != -1 ) {
         has_extc  = true;
         this_extc = rx_extc.cap( 1 ).toDouble();
      }
      if ( rx_g_dndc.indexIn( qv[ 0 ] ) != -1 ) {
         has_g_dndc  = true;
         this_g_dndc = rx_g_dndc.cap( 1 ).toDouble();
      }
      if ( rx_dndc.indexIn( qv[ 0 ] ) != -1 ) {
         has_dndc  = true;
         this_dndc = rx_dndc.cap( 1 ).toDouble();
      }
      if ( rx_fit_curve.indexIn( qv[ 0 ] ) != -1 ) {
         has_fit_curve  = true;
         this_fit_curve = rx_fit_curve.cap( 1 );
      }
      if ( rx_fit_method.indexIn( qv[ 0 ] ) != -1 ) {
         has_fit_method  = true;
         this_fit_method = rx_fit_method.cap( 1 );
      }
      if ( rx_fit_q_ranges.indexIn( qv[ 0 ] ) != -1 ) {
         has_fit_q_ranges  = true;
         this_fit_q_ranges = rx_fit_q_ranges.cap( 1 );
      }
      if ( rx_fit_chi2.indexIn( qv[ 0 ] ) != -1 ) {
         has_fit_chi2  = true;
         this_fit_chi2 = rx_fit_chi2.cap( 1 ).toDouble();
      }
      if ( rx_fit_sd_scale.indexIn( qv[ 0 ] ) != -1 ) {
         has_fit_sd_scale  = true;
         this_fit_sd_scale = rx_fit_sd_scale.cap( 1 ).toDouble();
      }
      if ( rx_diffusion_len.indexIn( qv[ 0 ] ) != -1 ) {
         has_diffusion_len  = true;
         this_diffusion_len = rx_diffusion_len.cap( 1 ).toDouble();
      }
      if ( rx_e_nucleon_ratio.indexIn( qv[ 0 ] ) != -1 ) {
         has_e_nucleon_ratio  = true;
         this_e_nucleon_ratio = rx_e_nucleon_ratio.cap( 1 ).toDouble();
      }
      if ( rx_nucleon_mass.indexIn( qv[ 0 ] ) != -1 ) {
         has_nucleon_mass  = true;
         this_nucleon_mass = rx_nucleon_mass.cap( 1 ).toDouble();
      }
      if ( rx_solvent_e_dens.indexIn( qv[ 0 ] ) != -1 ) {
         has_solvent_e_dens  = true;
         this_solvent_e_dens = rx_solvent_e_dens.cap( 1 ).toDouble();
      }
      if ( rx_ref_index.indexIn( qv[ 0 ] ) != -1 ) {
         has_ref_index  = true;
         this_ref_index = rx_ref_index.cap( 1 ).toDouble();
      }
      if ( rx_ri_corr.indexIn( qv[ 0 ] ) != -1 ) {
         has_ri_corr  = true;
         this_ri_corr = rx_ri_corr.cap( 1 ).toDouble();
      }
      if ( rx_ri_corrs.indexIn( qv[ 0 ] ) != -1 ) {
         has_ri_corrs  = true;
         this_ri_corrs = rx_ri_corrs.cap( 1 );
      }
   }

   // we should make some configuration for matches & offsets or column mapping
   // just an ad-hoc fix for APS 5IDD
   int q_offset   = 0;
   int I_offset   = 1;
   int e_offset   = 2;
   int row_offset = 1;
   if ( ( ext == "dat" || ext == "txt" ) && qv[ 0 ].toLower().contains( QRegExp( "frame\\s*data" ) ) )
   {
      is_time = true;
      use_units = 1.0;
   }

   if ( ext == "dat" && qv[ 0 ].contains( " MALS parameter file" ) )
   {
      QRegExp rx_mals_param_lambda            ( "^# __mals_param_lambda: (\\S+)\\s*$" );
      QRegExp rx_mals_param_n                 ( "^# __mals_param_n: (\\S+)\\s*$" );
      QRegExp rx_mals_param_g_dndc            ( "^# __mals_param_g_dndc: (\\S+)\\s*$" );
      QRegExp rx_mals_param_g_extinction_coef ( "^# __mals_param_g_extinction_coef: (\\S+)\\s*$" );
      QRegExp rx_mals_param_g_conc            ( "^# __mals_param_g_conc: (\\S+)\\s*$" );
      QRegExp rx_mals_param_DLS_detector      ( "^# __mals_param_DLS_detector: (\\S+)\\s*$" );

      for ( int i = 1; i < (int) qv.size(); i++ ) {

         if ( rx_mals_param_lambda.indexIn( qv[ i ] ) != -1 ) {
            mals_param_lambda = rx_mals_param_lambda.cap( 1 ).toDouble();
            continue;
         }
         if ( rx_mals_param_n.indexIn( qv[ i ] ) != -1 ) {
            mals_param_n = rx_mals_param_n.cap( 1 ).toDouble();
            continue;
         }
         if ( rx_mals_param_g_dndc.indexIn( qv[ i ] ) != -1 ) {
            mals_param_g_dndc = rx_mals_param_g_dndc.cap( 1 ).toDouble();
            continue;
         }
         if ( rx_mals_param_g_extinction_coef.indexIn( qv[ i ] ) != -1 ) {
            mals_param_g_extinction_coef = rx_mals_param_g_extinction_coef.cap( 1 ).toDouble();
            continue;
         }
         if ( rx_mals_param_g_conc.indexIn( qv[ i ] ) != -1 ) {
            mals_param_g_conc = rx_mals_param_g_conc.cap( 1 ).toDouble();
            continue;
         }
         if ( rx_mals_param_DLS_detector.indexIn( qv[ i ] ) != -1 ) {
            mals_param_DLS_detector = rx_mals_param_DLS_detector.cap( 1 ).toInt();
            continue;
         }

         errormsg = QString( us_tr( "Error: loading %1 line %2 unrecognied directive %3" ) ).arg( filename ).arg( i + 1 ).arg( qv[ i ] );
         return false;
      }
      errormsg = "";
      return false;
   }

   if ( ext == "dat" && qv[ 0 ].contains( " Detector State file" ) )
   {
      QRegExp rx_uv         ( "^# __detector_uv: (\\S+)\\s*$" );
      QRegExp rx_ri         ( "^# __detector_ri: (\\S+)\\s*$" );
      QRegExp rx_uv_set     ( "^# __detector_uv_set\\s*$" );
      QRegExp rx_ri_set     ( "^# __detector_ri_set\\s*$" );
      for ( int i = 1; i < (int) qv.size(); i++ )
      {
         if ( rx_uv.indexIn( qv[ i ] ) != -1 )
         {
            detector_uv_conv = rx_uv.cap( 1 ).toDouble();
            continue;
         }
         if ( rx_ri.indexIn( qv[ i ] ) != -1 )
         {
            detector_ri_conv = rx_ri.cap( 1 ).toDouble();
            continue;
         }
         if ( rx_uv_set.indexIn( qv[ i ] ) != -1 )
         {
            detector_uv = true;
            detector_ri = false;
            continue;
         }
         if ( rx_ri_set.indexIn( qv[ i ] ) != -1 )
         {
            detector_ri = true;
            detector_uv = false;
            continue;
         }
         errormsg = QString( us_tr( "Error: loading %1 line %2 unrecognied directive %3" ) ).arg( filename ).arg( i + 1 ).arg( qv[ i ] );
         return false;
      }
      errormsg = "";
      return false;
   }

   if ( ext == "csv" && load_conc ) {
   // load csv columns as time curves with rescaling for concentration time
      if ( !qv[0].contains( QRegExp( "^#\\s+Time" ) ) ) {
         errormsg = QString( us_tr( "Error: loading %1 unrecognied header format for concentration csv: %2" ) ).arg( filename ).arg( qv[ 0 ] );
         return false;
      }
      QRegExp rx_spaces = QRegExp( "\\s+" ); 
      QStringList headers = (qv[ 0 ] ).split( rx_spaces , Qt::SkipEmptyParts );
      int hsize = (int) headers.size();
      map < QString, vector < double > > uvs;
      for ( int i = 1; i < (int) qv.size(); ++i ) {
         QStringList line = (qv[ i ] ).split( rx_spaces , Qt::SkipEmptyParts );
         if ( (int) line.size() != hsize ) {
            errormsg = QString( us_tr( "Error: loading %1 line %2 tokens in line %3 != tokens in header %4" ) ).arg( filename ).arg( i + 1 ).arg( line.size() ).arg( headers.size() );
            return false;
         }

         for ( int j = 0; j < hsize; ++j ) {
            uvs[ headers[ j ] ].push_back( line[ j ].toDouble() );
         }
      }
         
      // // printout

      // for ( int i = 0; i < hsize; ++i ) {
      //    US_Vector::printvector( headers[ i ], uvs[ headers[ i ] ] );
      // }

      // refine time values

      double timestart = uvs[ "Time" ].front();
      double timemax = uvs[ "Time" ].back();
      double timediff = timemax - timestart;
      double count = uvs[ "#" ].back();
      int csize = (int) uvs[ "#" ].back();

      for ( int i = 0; i < csize; ++i ) {
         uvs[ "Tadj" ].push_back( timestart + ( ((double) i ) * timediff ) / ( count - 1 ) );
      }

      for ( int i = 0; i < csize; ++i ) {
         uvs[ "Tdif" ].push_back( uvs[ "Time" ][ i ] - uvs[ "Tadj" ][ i ] );
      }
      // US_Vector::printvector3( "Time Tadj Tdif", uvs[ "Time" ], uvs[ "Tadj" ], uvs[ "Tdif" ] );
      
      // next: ask for frame length & start time.

      map < QString, QString > conc_csv_params;
      conc_csv_params[ "starttime" ] = QString( "%1" ).arg( timestart  );
      conc_csv_params[ "endtime" ] = QString( "%1" ).arg( timemax );
      conc_csv_params[ "pointcount" ] = QString( "%1" ).arg( count );

      // setup any preliminary params

      US_Hydrodyn_Mals_Conc_Csv_Frames * conc_csv_frames_instance = 
         new US_Hydrodyn_Mals_Conc_Csv_Frames( us_hydrodyn, & conc_csv_params, this );
      US_Hydrodyn::fixWinButtons( conc_csv_frames_instance );
      conc_csv_frames_instance->exec();
      delete conc_csv_frames_instance;

      if ( !conc_csv_params.count( "button" ) ||
           conc_csv_params[ "button" ] != "go" )
      {
         errormsg = "Concentration load canceled by user";
         return false;
      }

      double frame1t    = conc_csv_params[ "frame1t" ].toDouble();
      double time2frame = conc_csv_params[ "time2frame" ].toDouble() / 60e0;
      int startframe =
         conc_csv_params.count( "startframe" ) ?
         conc_csv_params[ "startframenumber" ].toInt() :
         1;
      
      int endframe =
         conc_csv_params.count( "endframe" ) ?
         conc_csv_params[ "endframenumber" ].toInt() :
         9999999;
         
      for ( int i = 0; i < csize; ++i ) {
         uvs[ "Frame" ].push_back( 1e0 + ( ( uvs[ "Tadj" ][ i ] - frame1t ) / time2frame ) );
      }

      // build frames to use

      int dstartframe = std::ceil ( uvs[ "Frame" ].front() );
      int dendframe   = std::floor( uvs[ "Frame" ].back()  );

      QString notes;

      if ( startframe < dstartframe ) {
         notes += QString( us_tr( "specified starting frame %1 is less than minimum frame available %2, using minimum available.\n" ) )
            .arg( startframe )
            .arg( dstartframe );
         startframe = dstartframe;
      }
      
      if ( endframe > dendframe ) {
         if ( conc_csv_params.count( "endframe" ) ) {
            notes += QString( us_tr( "specified ending frame %1 is greater than maximum frame available %2, using maximum available.\n" ) )
               .arg( endframe )
               .arg( dendframe );
         }
         endframe = dendframe;
      }
         
      if ( !notes.isEmpty() ) {
         editor_msg( "dark blue", notes );
      }

      // US_Vector::printvector2( "Tadj Frame", uvs[ "Tadj" ], uvs[ "Frame" ] );

      // next: build cubic splines for each, then ask for frame length and start time if to convert
      vector < double >  use_frames;
      vector < QString > use_frames_qs;

      for ( int i = startframe; i <= endframe; ++i ) {
         use_frames   .push_back( i );
         use_frames_qs.push_back( QString( "%1" ).arg( i ) );
      }

      map < QString, vector < double > > framedata;
      
      for ( int j = 2; j < hsize; ++j ) {
         double y;
         vector < double > y2;
         usu->natural_spline( uvs[ "Frame" ], uvs[ headers[ j ] ], y2 );
         for ( int i = startframe; i <= endframe; ++i ) {
            usu->apply_natural_spline( uvs[ "Frame" ], uvs[ headers[ j ] ], y2, (double) i, y );
            framedata[ headers[ j ] ].push_back( y );
         }
      }

      // load curves
      QStringList created_files;
      for ( int j = 2; j < hsize; ++j ) {
         QString name = QFileInfo( filename ).completeBaseName() + "_" + headers[ j ];
         created_files <<  name;

         conc_files.insert( name );

         created_files_not_saved[ name ] = true;

         f_pos       [ name ] = f_qs.size();
         f_qs_string [ name ] = use_frames_qs;
         f_qs        [ name ] = use_frames;
         f_Is        [ name ] = framedata[ headers[ j ] ];
         f_is_time   [ name ] = true;
         f_psv       [ name ] = 0e0;
         f_I0se      [ name ] = 0e0;
         f_conc      [ name ] = 0e0;
         {
            vector < double > tmp;
            f_gaussians  [ name ] = tmp;
         }
      }
      lb_created_files->addItems( created_files );
      lb_files->addItems( created_files );
      lb_created_files->scrollToItem( lb_created_files->item( lb_created_files->count() - 1 ) );
      lb_files->scrollToItem( lb_files->item( lb_files->count() - 1 ) );

      editor_msg( "black", filename );
      editor_msg( "blue", us_tr( "Files loaded ok" ) );
      errormsg = "";
      return false;
   }

   // load csv columns as time curves
   if ( ext == "csv" )
   {
      {
         QString errormsg;
         if ( mals_angles.load( filename, qsl, errormsg ) ) {
            editor_msg( "black", QString( "%1 MALS angles loaded\n" ).arg( filename ) );
            editor_msg( "black", mals_angles.list() );
            lbl_mals_angles_data->setText( mals_angles.list_rich( mals_param_lambda, mals_param_n ) );
            return false;
         }
         if ( errormsg != us_tr( "Not a MALS Angles csv file" ) ) {
            QMessageBox::warning(
                                 this
                                 ,"US-SOMO MALS Angles load Error"
                                 ,QString( us_tr( "Loading file %1:\nError: %2" ) ).arg( filename ).arg( us_tr( errormsg ) )
                                 );
            return false;
         }
      }


      {
         QString errormsg;
         if ( mals_load( filename, qsl, errormsg ) ) {
            editor_msg( "black", QString( us_tr( "%1 - MALS data\n" ) ).arg( filename ) );
            return false;
         }
         if ( errormsg != "not a mals file" ) {
            QMessageBox::warning(
                                 this
                                 ,"US-SOMO MALS load Error"
                                 ,QString( us_tr( "Loading file %1:\nError: %2" ) ).arg( filename ).arg( us_tr( errormsg ) )
                                 );
            return false;
         }
      }

      editor_msg( "black", QString( us_tr( "%1" ) ).arg( filename ) );

      // first column is time
      qv[ 0 ].replace( "(", "" ).replace( ")", "" ).replace( "/", "_per_" ).replace( QRegExp( "\\s+" ), "_" ).replace( ":", "_" ).replace( QRegExp( "\\_+" ), "_" ) ;

      QStringList headers = (qv[ 0 ] ).split( "," , Qt::SkipEmptyParts );
      
      if ( !headers.size() ||
           !headers[ 0 ].toLower().contains( "time" ) )
      {
         errormsg = us_tr( "The first line, first column of the .csv file must contain 'time'" );
         return false;
      }

      vector < vector < double > > csv_data;
      vector < double > q;
      vector < QString > q_string;
      for ( int i = 1; i < (int) qv.size(); i++ )
      {
         QStringList data = (qv[ i ] ).split( "," , Qt::SkipEmptyParts );
         vector < double > this_csv_data;
         if ( data.size() )
         {
            q.push_back( data[ 0 ].toDouble() );
            q_string.push_back( data[ 0 ] );
         }
         for ( int j = 1; j < (int) data.size(); j++ )
         {
            this_csv_data.push_back( data[ j ].toDouble() );
         }
         csv_data.push_back( this_csv_data );
      }

      map < QString, bool > current_files = all_files_map();

      for ( int i = 1; i < (int) headers.size(); i++ )
      {
         QString name = headers[ i ];
         unsigned int ext = 0;
         while ( current_files.count( name ) )
         {
            name = headers[ i ] + QString( "-%1" ).arg( ++ext );
         }
         vector < double > I;
         vector < double > use_q;
         vector < QString > use_q_string;
         for ( int j = 0; j < (int) csv_data.size(); j++ )
         {
            if ( i - 1 < (int) csv_data[ j ].size() )
            {
               I.push_back( csv_data[ j ][ i - 1 ] );
               use_q.push_back( q[ j ] );
               use_q_string.push_back( q_string[ j ] );
            }
         }

         // TSO << QString( "curve %1 sizes: I %2 q %3 qs %4\n" ).arg( name ).arg( I.size() ).arg( use_q.size() ).arg( use_q_string.size() );
         // US_Vector::printvector2( "q, I", use_q, I );

         if ( I.size() )
         {
            lb_files->addItem( name );
            lb_files->scrollToItem( lb_files->item( lb_files->count() - 1 ) );
            // created_files_not_saved[ name ] = false;

            f_pos       [ name ] = f_qs.size();
            f_qs_string [ name ] = use_q_string;
            f_qs        [ name ] = use_q;
            f_Is        [ name ] = I;
            f_is_time   [ name ] = true;
            f_psv       [ name ] = 0e0;
            f_I0se      [ name ] = 0e0;
            f_conc      [ name ] = 0e0;
            {
               vector < double > tmp;
               f_gaussians  [ name ] = tmp;
            }
         } else {
            editor_msg( "red", QString( us_tr( "csv file %1 column %2 \"%3\" doesn't seem to be complete, skipped" ) ).arg( filename ).arg( i + 1 ).arg( name ) );
         } 
      }

      errormsg = "";
      return false;
   }
      
   if ( ext == "dat" && qv[ 0 ].contains( " Global State file" ) )
   {
      QRegExp rx_dir             ( "^# __dir: (\\S+)\\s*$" );
      QRegExp rx_lock_dir        ( "^# __lock_dir\\s*$" );
      QRegExp rx_created_dir     ( "^# __created_dir: (\\S+)\\s*$" );
      QRegExp rx_files           ( "^# __files\\s*$" );
      QRegExp rx_conc_files      ( "^# __conc_files\\s*$" );
      QRegExp rx_end             ( "^# __end\\s*$" );
      QRegExp rx_gaussians       ( "^# __gaussians\\s*$" );
      QRegExp rx_f_gaussians     ( "^# __f_gaussians: (\\S+)\\s*$" );
      QRegExp rx_push            ( "^# __push_stack\\s*$" );
      QRegExp rx_gaussian_type   ( "^# __gaussian_type: (\\S+)\\s*$" );

      QRegExp rx_uv         ( "^# __detector_uv: (\\S+)\\s*$" );
      QRegExp rx_ri         ( "^# __detector_ri: (\\S+)\\s*$" );
      QRegExp rx_uv_set     ( "^# __detector_uv_set\\s*$" );
      QRegExp rx_ri_set     ( "^# __detector_ri_set\\s*$" );

      clear_files( all_files() );

      gaussian_types default_gaussian_type = GAUSS;
      bool           defined_gaussian_type = false;

      cb_lock_dir->setChecked( false );
      for ( int i = 1; i < (int) qv.size(); i++ )
      {
         if ( rx_gaussian_type.indexIn( qv[ i ] ) != -1 )
         {
            gaussian_types new_g = gaussian_type;
            if ( rx_gaussian_type.cap( 1 ) == "Gauss" )
            {
               new_g = GAUSS;
            }
            if ( rx_gaussian_type.cap( 1 ) == "EMG" )
            {
               new_g = EMG;
            }
            if ( rx_gaussian_type.cap( 1 ) == "GMG" )
            {
               new_g = GMG;
            }
            if ( rx_gaussian_type.cap( 1 ) == "EMG+GMG" )
            {
               new_g = EMGGMG;
            }
            if ( gaussian_type != new_g )
            {
               gaussian_type = new_g;
               clear_gaussians();
               update_gauss_mode();
               ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_gaussian_type" ] = QString( "%1" ).arg( gaussian_type );
            }
            defined_gaussian_type = true;
            continue;
         }

         if ( !defined_gaussian_type &&
              ( rx_gaussians.indexIn( qv[ i ] ) != -1 ||
                rx_f_gaussians.indexIn( qv[ i ] ) != -1 ) )
         {
            editor_msg( "dark red", us_tr( "Notice: this state file does not have a specific Gaussian type defined, defaulting to standard Gaussians" ) );
            gaussian_types new_g = default_gaussian_type;
            if ( gaussian_type != new_g )
            {
               gaussian_type = new_g;
               clear_gaussians();
               update_gauss_mode();
               ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_gaussian_type" ] = QString( "%1" ).arg( gaussian_type );
            }
            defined_gaussian_type = true;
         }            

         if ( rx_uv.indexIn( qv[ i ] ) != -1 )
         {
            detector_uv_conv = rx_uv.cap( 1 ).toDouble();
            continue;
         }
         if ( rx_ri.indexIn( qv[ i ] ) != -1 )
         {
            detector_ri_conv = rx_ri.cap( 1 ).toDouble();
            continue;
         }
         if ( rx_uv_set.indexIn( qv[ i ] ) != -1 )
         {
            detector_uv = true;
            detector_ri = false;
            continue;
         }
         if ( rx_ri_set.indexIn( qv[ i ] ) != -1 )
         {
            detector_ri = true;
            detector_uv = false;
            continue;
         }
         if ( rx_dir.indexIn( qv[ i ] ) != -1 )
         {
            le_dir->setText( rx_dir.cap( 1 ) );
            continue;
         }
         if ( rx_lock_dir.indexIn( qv[ i ] ) != -1 )
         {
            cb_lock_dir->setChecked( true );
            continue;
         }
         if ( rx_created_dir.indexIn( qv[ i ] ) != -1 )
         {
            le_created_dir->setText( rx_created_dir.cap( 1 ) );
            continue;
         }
         if ( rx_files.indexIn( qv[ i ] ) != -1 )
         {
            i++;
            QStringList files;
            for ( ; i < ( int ) qv.size(); i++ )
            {
               if ( rx_end.indexIn( qv[ i ] ) == -1 )
               {
                  files << qv[ i ];
               } else {
                  break;
               }
            }
            if ( i < ( int ) qv.size() && rx_end.indexIn( qv[ i ] ) != -1 )
            {
               if ( files.size() )
               {
                  add_files( files );
                  disable_all();
               }
               continue;
            }
            errormsg = QString( us_tr( "Error: loading %1 line %2 unterminated file list" ) ).arg( filename ).arg( i + 1 );
            return false;
         }
         if ( rx_conc_files.indexIn( qv[ i ] ) != -1 )
         {
            i++;
            QStringList files;
            for ( ; i < ( int ) qv.size(); i++ )
            {
               if ( rx_end.indexIn( qv[ i ] ) == -1 )
               {
                  conc_files.insert( qv[ i ] );
               } else {
                  break;
               }
            }
            if ( i < ( int ) qv.size() && rx_end.indexIn( qv[ i ] ) != -1 )
            {
               continue;
            }
            errormsg = QString( us_tr( "Error: loading %1 line %2 unterminated concentration file list" ) ).arg( filename ).arg( i + 1 );
            return false;
         }

         if ( rx_gaussians.indexIn( qv[ i ] ) != -1 )
         {
            i++;
            if ( i >= ( int ) qv.size() )
            {
               errormsg = QString( us_tr( "Error: loading %1 line %2 unterminated file list" ) ).arg( filename ).arg( i + 1 );
               return false;
            }               
            
            gaussians.clear( );

            // QStringList tokens = (qv[i].replace(QRegExp("^\\s+").split( QRegExp("\\s+") , Qt::SkipEmptyParts ),""));
            QStringList tokens;
            {
               QString qs = qv[i].replace(QRegExp("^\\s+"),"");
               tokens = (qs ).split( QRegExp("\\s+") , Qt::SkipEmptyParts );
            }

            if ( tokens.size() != 2 )
            {
               errormsg = QString("Error: loading Gaussians file %1 incorrect format line %2 tokens found %3 value <%4>").arg( filename ).arg( i + 1 ).arg( tokens.size() ).arg( qv[ i ] );
               return false;
            }
      
            disconnect( le_gauss_fit_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
            disconnect( le_gauss_fit_start, SIGNAL( focussed ( bool ) )             , 0, 0 );
            disconnect( le_gauss_fit_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
            disconnect( le_gauss_fit_end, SIGNAL( focussed ( bool ) )             , 0, 0 );

            le_gauss_fit_start->setText( QString( "%1" ).arg( tokens[ 0 ].toDouble() ) );
            le_gauss_fit_end  ->setText( QString( "%1" ).arg( tokens[ 1 ].toDouble() ) );

            connect( le_gauss_fit_start, SIGNAL( textChanged( const QString & ) ), SLOT( gauss_fit_start_text( const QString & ) ) );
            connect( le_gauss_fit_start, SIGNAL( focussed ( bool ) )             , SLOT( gauss_fit_start_focus( bool ) ) );
            connect( le_gauss_fit_end, SIGNAL( textChanged( const QString & ) ), SLOT( gauss_fit_end_text( const QString & ) ) );
            connect( le_gauss_fit_end, SIGNAL( focussed ( bool ) )             , SLOT( gauss_fit_end_focus( bool ) ) );

            i++;

            for ( ; i < ( int ) qv.size(); i++ )
            {
               if ( rx_end.indexIn( qv[ i ] ) == -1 )
               {
                  // tokens = (qv[i].replace(QRegExp("^\\s+").split( QRegExp("\\s+") , Qt::SkipEmptyParts ),""));
                  {
                     QString qs = qv[i].replace(QRegExp("^\\s+"),"");
                     tokens = (qs ).split( QRegExp("\\s+") , Qt::SkipEmptyParts );
                  }

                  if ( (int) tokens.size() != gaussian_type_size )
                  {
                     errormsg = QString("Error: Gaussian file %1 incorrect format line %2").arg( filename ).arg( i + 1 );
                     return false;
                  }

                  for ( int k = 0; k < gaussian_type_size; k++ )
                  {
                     gaussians.push_back( tokens[ k ].toDouble() );
                  }
               } else {
                  break;
               }
            }
            if ( i < ( int ) qv.size() && rx_end.indexIn( qv[ i ] ) != -1 )
            {
               continue;
            }
            errormsg = QString( us_tr( "Error: loading %1 line %2 unterminated file list" ) ).arg( filename ).arg( i + 1 );
            return false;

         }

         if ( rx_f_gaussians.indexIn( qv[ i ] ) != -1 )
         {
            i++;
            if ( i >= ( int ) qv.size() )
            {
               errormsg = QString( us_tr( "Error: loading %1 line %2 unterminated file list" ) ).arg( filename ).arg( i + 1 );
               return false;
            }               

            vector < double > tmp_g;

            for ( ; i < ( int ) qv.size(); i++ )
            {
               if ( rx_end.indexIn( qv[ i ] ) == -1 )
               {
                  // QStringList tokens = (qv[i].replace(QRegExp("^\\s+").split( QRegExp("\\s+") , Qt::SkipEmptyParts ),""));
                  QStringList tokens;
                  {
                     QString qs = qv[i].replace(QRegExp("^\\s+"),"");
                     tokens = (qs ).split( QRegExp("\\s+") , Qt::SkipEmptyParts );
                  }

                  if ( (int) tokens.size() != gaussian_type_size )
                  {
                     errormsg = QString("Error: loading file specific Gaussians from file %1 incorrect format line %2").arg( filename ).arg( i + 1 );
                     return false;
                  }

                  for ( int k = 0; k < gaussian_type_size; k++ )
                  {
                     tmp_g.push_back( tokens[ k ].toDouble() );
                  }
               } else {
                  break;
               }
            }
            if ( i < ( int ) qv.size() && rx_end.indexIn( qv[ i ] ) != -1 )
            {
               f_gaussians[ rx_f_gaussians.cap( 1 ) ] = tmp_g;
               continue;
            }
            errormsg = QString( us_tr( "Error: loading %1 line %2 unterminated file list" ) ).arg( filename ).arg( i + 1 );
            return false;
         }

         if ( rx_push.indexIn( qv[ i ] ) != -1 )
         {
            stack_push_all();
            disable_all();
            clear_files( all_files() );
            continue;
         }

         errormsg = QString( us_tr( "Error: loading %1 line %2 unrecognied directive %3" ) ).arg( filename ).arg( i + 1 ).arg( qv[ i ] );
         return false;
      }
      
      errormsg = "";
      return false;
   }

   if ( ext == "dat" && qv[ 0 ].contains( QRegExp( " (Gauss|EMG\\+GMG|EMG|GMG)" ) ) )
   {
      gaussian_types new_g = gaussian_type;
      if ( qv[ 0 ].contains( " Gauss" ) )
      {
         new_g = GAUSS;
      }
      if ( qv[ 0 ].contains( " EMG" ) )
      {
         new_g = EMG;
      }
      if ( qv[ 0 ].contains( " GMG" ) )
      {
         new_g = GMG;
      }
      if ( qv[ 0 ].contains( " EMG+GMG" ) )
      {
         new_g = EMGGMG;
      }
      if ( gaussian_type != new_g )
      {
         gaussian_type = new_g;
         unified_ggaussian_ok = false;
         f_gaussians.clear( );
         gaussians.clear( );
         org_gaussians.clear( );
         org_f_gaussians.clear( );
         update_gauss_mode();
         ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_gaussian_type" ] = QString( "%1" ).arg( gaussian_type );
      }

      gaussians.clear( );
      int i = 1;
      // QStringList tokens = (qv[i].replace(QRegExp("^\\s+").split( QRegExp("\\s+") , Qt::SkipEmptyParts ),""));
      QStringList tokens;
      {
         QString qs = qv[i].replace(QRegExp("^\\s+"),"");
         tokens = (qs ).split( QRegExp("\\s+") , Qt::SkipEmptyParts );
      }

      if ( tokens.size() != 2 )
      {
         errormsg = QString("Error: Gaussian file %1 incorrect format line %2").arg( filename ).arg( i + 1 );
         return false;
      }
      
      disconnect( le_gauss_fit_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      disconnect( le_gauss_fit_start, SIGNAL( focussed ( bool ) )             , 0, 0 );
      disconnect( le_gauss_fit_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      disconnect( le_gauss_fit_end, SIGNAL( focussed ( bool ) )             , 0, 0 );

      le_gauss_fit_start->setText( QString( "%1" ).arg( tokens[ 0 ].toDouble() ) );
      le_gauss_fit_end  ->setText( QString( "%1" ).arg( tokens[ 1 ].toDouble() ) );

      connect( le_gauss_fit_start, SIGNAL( textChanged( const QString & ) ), SLOT( gauss_fit_start_text( const QString & ) ) );
      connect( le_gauss_fit_start, SIGNAL( focussed ( bool ) )             , SLOT( gauss_fit_start_focus( bool ) ) );
      connect( le_gauss_fit_end, SIGNAL( textChanged( const QString & ) ), SLOT( gauss_fit_end_text( const QString & ) ) );
      connect( le_gauss_fit_end, SIGNAL( focussed ( bool ) )             , SLOT( gauss_fit_end_focus( bool ) ) );


      if ( qv[ 0 ].contains( QRegExp( "Multiple (Gauss|EMG\\+GMG|EMG|GMG)" ) ) )
      {
         // TSO << "multiple gaussians\n";

         QString           this_gaussian;
         QRegExp           rx_gname( "^(?:Gauss|EMG\\+GMG|EMG|GMG) (.*)$" ); 
         vector < double > g;
         unsigned int      loaded  = 0;
         unsigned int      skipped = 0;

         vector < map < int, set < double > > > check_common;

         for ( i = 2; i < (int) qv.size(); i++ )
         {
            if ( rx_gname.indexIn( qv[ i ] ) != -1 )
            {
               // TSO << QString( "mg: found %1\n" ).arg( rx_gname.cap( 1 ) );
               // new file specific gaussian
               if ( g.size() && !this_gaussian.isEmpty() )
               {
                  f_gaussians[ this_gaussian ] = g;
                  gaussians = g;

                  int this_gn_count = (int)( g.size() / gaussian_type_size );
                  if ( (int) check_common.size() < this_gn_count )
                  {
                     check_common.resize( this_gn_count );
                  }
                  for ( int gn = 0; gn < this_gn_count; ++gn )
                  {
                     int pos = gn * gaussian_type_size;
                     for ( int k = 0; k < gaussian_type_size; ++k )
                     {
                        check_common[ gn ][ k ].insert( g[ pos + k ] );
                     }
                  }

                  loaded++;
               }
               g.clear( );
               this_gaussian = rx_gname.cap( 1 );
               if ( !f_qs.count( this_gaussian ) )
               {
                  skipped++;
                  editor_msg( "red", QString( us_tr( "%1 for file %2 present but the file is not loaded, please load the file first" ) ).arg( gaussian_type_tag ).arg( this_gaussian ) );
                  this_gaussian = "";
               }
               continue;
            }

            if ( !this_gaussian.isEmpty() )
            {
               // tokens = (qv[i].replace(QRegExp("^\\s+").split( QRegExp("\\s+") , Qt::SkipEmptyParts ),""));
               {
                  QString qs = qv[i].replace(QRegExp("^\\s+"),"");
                  tokens = (qs ).split( QRegExp("\\s+") , Qt::SkipEmptyParts );
               }
         
               if ( (int) tokens.size() != gaussian_type_size )
               {
                  errormsg = QString("Error: Multiple %1 file %2 incorrect format line %3").arg( gaussian_type_tag ).arg( filename ).arg( i + 1 );
                  return false;
               }

               for ( int k = 0; k < gaussian_type_size; k++ )
               {
                  g.push_back( tokens[ k ].toDouble() );
               }
            }
         }
         if ( g.size() && !this_gaussian.isEmpty() )
         {
            f_gaussians[ this_gaussian ] = g;
            gaussians = g;
            int this_gn_count = (int)( g.size() / gaussian_type_size );
            if ( (int) check_common.size() < this_gn_count )
            {
               check_common.resize( this_gn_count );
            }
            for ( int gn = 0; gn < this_gn_count; ++gn )
            {
               int pos = gn * gaussian_type_size;
               for ( int k = 0; k < gaussian_type_size; ++k )
               {
                  check_common[ gn ][ k ].insert( g[ pos + k ] );
               }
            }
            loaded++;
            g.clear( );
         }
         ggauss_msg_common( check_common );
         editor_msg( "black", QString( "%1 for %2 files loaded from %3" ).arg( gaussian_type_tag ).arg( loaded ).arg( filename ) );
         if ( skipped )
         {
            editor_msg( "red" , QString( "WARNING: %1 for %2 files SKIPPED from %3" ).arg( gaussian_type_tag ).arg( skipped ).arg( filename ) );
         }
         // create_unified_ggaussian_target(false); // core dumps!
         // ggaussian_rmsd();
      } else {
         for ( i = 2; i < (int) qv.size(); i++ )
         {
            // tokens = (qv[i].replace(QRegExp("^\\s+").split( QRegExp("\\s+") , Qt::SkipEmptyParts ),""));
            {
               QString qs = qv[i].replace(QRegExp("^\\s+"),"");
               tokens = (qs ).split( QRegExp("\\s+") , Qt::SkipEmptyParts );
            }
         
            if ( (int) tokens.size() != gaussian_type_size )
            {
               errormsg = QString("Error: Gaussian file %1 incorrect format line %2").arg( filename ).arg( i + 1 );
               return false;
            }

            for ( int k = 0; k < gaussian_type_size; k++ )
            {
               gaussians.push_back( tokens[ k ].toDouble() );
            }
         }

         editor_msg( "black", QString( "%1 Gaussians loaded from file %2" ).arg( gaussians.size() / gaussian_type_size ).arg( filename ) );
      }
      errormsg = "";
      return false;
   }

   if ( ext == "txt" && qv[ 0 ].contains( "# File Encoding (File origin in Excel)" ) )
   {
      q_offset = 1;
      I_offset = 2;
      e_offset = 3;
      editor_msg( "dark blue", us_tr( "APS SIDD format" ) );
   }      
   
   // ad-hoc for soleil mals time/uv/conc data

   // TSO << "load: <" << qv[ 0 ] << ">" << endl;

   if ( ext == "txt" && qv[ 0 ].contains( "temps depuis le debut" ) )
   {
      I_offset   = 2;
      e_offset   = 10;
      row_offset = 4;
      is_time    = true;
      use_units  = 1.0;
      editor_msg( "dark blue", us_tr( "SOLEIL HPLC time/uv format" ) );
   }      

   vector < QString > q_string;
   vector < double >  q;
   vector < double >  I;
   vector < double >  e;

   QRegExp rx_ok_line("^(\\s+(-|)|\\d+|\\.|\\d(E|e)(\\+|-|\\d))+$");
   rx_ok_line.setMinimal( true );
   for ( int i = row_offset; i < (int) qv.size(); i++ )
   {
      if ( qv[i].contains(QRegExp("^#")) ||
           rx_ok_line.indexIn( qv[i] ) == -1 )
      {
         continue;
      }
      
      // TSO << "line: <" << qv[ i ] << ">" << endl;

      // QStringList tokens = (qv[i].replace(QRegExp("^\\s+").split( QRegExp("\\s+") , Qt::SkipEmptyParts ),""));
      QStringList tokens;
      {
         QString qs = qv[i].replace(QRegExp("^\\s+"),"");
         tokens = (qs ).split( QRegExp("\\s+") , Qt::SkipEmptyParts );
      }

      if ( (int)tokens.size() > I_offset )
      {
         QString this_q_string = tokens[ q_offset ];
         double this_q         = tokens[ q_offset ].toDouble();
         double this_I         = tokens[ I_offset ].toDouble();
         double this_e = 0e0;
         if ( use_units != 1 && !is_time) {
            this_q *= use_units;
            this_q_string = QString( "%1" ).arg( this_q );
         }
         if ( (int)tokens.size() > e_offset)
         {
            this_e = tokens[ e_offset ].toDouble();
            if ( this_e < 0e0 )
            {
               this_e = 0e0;
            }
         }
         if ( q.size() && this_q <= q[ q.size() - 1 ] )
         {
            TSO << QString(" breaking %1 %2\n").arg( this_q ).arg( q[ q.size() - 1 ] );
            break;
         }
         if ( is_time || !us_isnan( this_I ) )
         {
            q_string.push_back( this_q_string );
            q       .push_back( this_q );
            I       .push_back( this_I );
            if ( (int)tokens.size() > e_offset ) // && this_e )
            {
               e.push_back( this_e );
            }
         }
      }
   }

   if ( !q.size() )
   {
      editor_msg( "red", QString( us_tr( "Error: File %1 has no data" ) ).arg( filename ) );
      return false;
   }
                  
   if ( is_zero_vector( I ) )
   {
      editor_msg( "red", QString( us_tr( "Error: File %1 has only zero signal" ) ).arg( filename ) );
      return false;
   }

   // TSO << QString( "opened %1\n" ).arg( filename ) << flush;
   QString basename = QFileInfo( filename ).completeBaseName();
   f_name      [ basename ] = filename;
   f_pos       [ basename ] = f_qs.size();
   f_qs_string [ basename ] = q_string;
   f_qs        [ basename ] = q;
   f_Is        [ basename ] = I;
   if ( e.size() == q.size() )
   {
      f_errors        [ basename ] = e;
   } else {
      if ( e.size() )
      {
         editor->append( 
                        QString( us_tr( "Notice: File %1 appeared to have standard deviations, but some were zero or less, so all were dropped\n" ) 
                                 ).arg( filename ) 
                        );
      }
      f_errors    .erase( basename );
   }
   f_is_time    [ basename ] = is_time;
   {
      vector < double > tmp;
      f_gaussians  [ basename ] = tmp;
   }
   if ( has_conc ) {
      f_conc            [ basename ] = this_conc;
   }
   if ( has_conc_units ) {
      f_conc_units      [ basename ] = this_conc_units;
   }
   if ( has_psv ) {
      f_psv             [ basename ] = this_psv;
   }
   if ( has_I0se ) {
      f_I0se            [ basename ] = this_I0se;
   }
   if ( has_I0st ) {
      f_I0st            [ basename ] = this_I0st;
   }
   if ( has_time ) {
      f_time            [ basename ] = this_time;
   }      
   if ( has_extc ) {
      f_extc            [ basename ] = this_extc;
   }      
   if ( has_g_dndc ) {
      f_g_dndc          [ basename ] = this_g_dndc;
   }      
   if ( has_dndc ) {
      f_dndc            [ basename ] = this_dndc;
   }      
   if ( has_ref_index ) {
      f_ref_index       [ basename ] = this_ref_index;
   }      
   if ( has_diffusion_len ) {
      f_diffusion_len   [ basename ] = this_diffusion_len;
   }      
   if ( has_e_nucleon_ratio ) {
      f_e_nucleon_ratio [ basename ] = this_e_nucleon_ratio;
   }      
   if ( has_nucleon_mass ) {
      f_nucleon_mass    [ basename ] = this_nucleon_mass;
   }      
   if ( has_solvent_e_dens ) {
      f_solvent_e_dens  [ basename ] = this_solvent_e_dens;
   }      
   if ( has_ri_corr ) {
      f_ri_corr         [ basename ] = this_ri_corr;
   }      
   if ( has_ri_corrs ) {
      f_ri_corrs        [ basename ] = this_ri_corrs;
   }      
   if ( has_fit_curve ) {
      f_fit_curve       [ basename ] = this_fit_curve;
   }      
   if ( has_fit_method ) {
      f_fit_method      [ basename ] = this_fit_method;
   }      
   if ( has_fit_q_ranges ) {
      f_fit_q_ranges    [ basename ] = this_fit_q_ranges;
   }      
   if ( has_fit_chi2 ) {
      f_fit_chi2        [ basename ] = this_fit_chi2;
   }      
   if ( has_fit_sd_scale ) {
      f_fit_sd_scale    [ basename ] = this_fit_sd_scale;
   }      
   return true;
}

void US_Hydrodyn_Mals::set_conc_file( QString file )
{
   if ( file.isEmpty() )
   {
      for ( int i = 0; i < lb_files->count(); i++ )
      {
         if ( lb_files->item( i )->isSelected() )
         {
            lbl_conc_file->setText( lb_files->item( i )->text() );
         }
      }
   } else {
      lbl_conc_file->setText( file );
   }
      
   update_csv_conc();
   if ( conc_widget )
   {
      conc_window->refresh( csv_conc );
   }
   plot_ref->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_ref->detachItems( QwtPlotItem::Rtti_PlotMarker );;

   if ( f_qs.count( lbl_conc_file->text() ) )
   {
#if QT_VERSION < 0x040000
      long curve;
      curve = plot_ref->insertCurve( "concentration" );
      plot_ref->setCurveStyle( curve, QwtCurve::Lines );
#else
      QwtPlotCurve *curve = new QwtPlotCurve( "concentration" );
      curve->setStyle( QwtPlotCurve::Lines );
#endif

#if QT_VERSION < 0x040000
      plot_ref->setCurvePen( curve, QPen( plot_colors[ 0 ], use_line_width, Qt::SolidLine ) );
      plot_ref->setCurveData( curve,
                              (double *)&f_qs[ lbl_conc_file->text() ][ 0 ],
                              (double *)&f_Is[ lbl_conc_file->text() ][ 0 ],
                              f_qs[ lbl_conc_file->text() ].size()
                              );
#else
      curve->setPen( QPen( plot_colors[ 0 ], use_line_width, Qt::SolidLine ) );
      curve->setSamples(
                     (double *)&f_qs[ lbl_conc_file->text() ][ 0 ],
                     (double *)&f_Is[ lbl_conc_file->text() ][ 0 ],
                     f_qs[ lbl_conc_file->text() ].size()
                     );
      curve->attach( plot_ref );
#endif
      plot_dist->setAxisScale( QwtPlot::xBottom, f_qs[ lbl_conc_file->text() ][ 0 ], f_qs[ lbl_conc_file->text() ].back() );
      
      if ( !suppress_replot )
      {
         plot_ref->replot();
      }
   }
   update_enables();
}

void US_Hydrodyn_Mals::set_mals()
{
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         lbl_mals->setText( lb_files->item( i )->text() );
      }
   }
   update_csv_conc();
   if ( conc_widget )
   {
      conc_window->refresh( csv_conc );
   }
   update_enables();
}

void US_Hydrodyn_Mals::set_signal()
{
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         lbl_signal->setText( lb_files->item( i )->text() );
      }
   }
   update_enables();
}

void US_Hydrodyn_Mals::set_empty()
{
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         lbl_empty->setText( lb_files->item( i )->text() );
      }
   }
   update_csv_conc();
   if ( conc_widget )
   {
      conc_window->refresh( csv_conc );
   }
   update_enables();
}

void US_Hydrodyn_Mals::update_created_files()
{
   if ( !disable_updates )
   {
      update_enables();
   }
}

void US_Hydrodyn_Mals::avg()
{
   // QStringList files;
   //    for ( int i = 0; i < lb_files->count(); i++ )
   //    {
   //       if ( lb_files->item( i )->isSelected() 
   //            //            && 
   //            //            lb_files->item( i )->text() != lbl_mals->text() &&
   //            //            lb_files->item( i )->text() != lbl_empty->text() 
   //            )
   //       {
   //          files << lb_files->item( i )->text();
   //       }
   //    }
   avg( all_selected_files() );
}

QString US_Hydrodyn_Mals::qstring_common_head( QStringList qsl, bool strip_digits )
{
   if ( !qsl.size() )
   {
      return "";
   }
   if ( qsl.size() == 1 )
   {
      return qsl[ 0 ];
   }
   QString s = qsl[ 0 ];
   for ( int i = 1; i < (int)qsl.size(); i++ )
   {
      s = qstring_common_head( s, qsl[ i ] );
   }

   if ( strip_digits )
   {
      s.replace( QRegExp( "\\d+$" ), "" );
   }
   return s;
}

QString US_Hydrodyn_Mals::qstring_common_tail( QStringList qsl, bool strip_digits )
{
   if ( !qsl.size() )
   {
      return "";
   }
   if ( qsl.size() == 1 )
   {
      return qsl[ 0 ];
   }
   QString s = qsl[ 0 ];
   for ( int i = 1; i < (int)qsl.size(); i++ )
   {
      s = qstring_common_tail( s, qsl[ i ] );
   }
   if ( strip_digits )
   {
      s.replace( QRegExp( "^\\d+" ), "" );
   }
   return s;
}
      
QString US_Hydrodyn_Mals::qstring_common_head( QString s1, 
                                                      QString s2 )
{
   int min_len = (int)s1.length();
   if ( min_len > (int)s2.length() )
   {
      min_len = (int)s2.length();
   }

   // could do this more efficiently via "divide & conquer"
   // i.e. split the distance in halfs and compare 
   
   int match_max = 0;
   for ( int i = 1; i <= min_len; i++ )
   {
      match_max = i;
      if ( s1.left( i ) != s2.left( i ) )
      {
         match_max = i - 1;
         break;
      }
   }
   return s1.left( match_max );
}

QString US_Hydrodyn_Mals::qstring_common_tail( QString s1, 
                                                      QString s2 )
{
   int min_len = (int)s1.length();
   if ( min_len > (int)s2.length() )
   {
      min_len = (int)s2.length();
   }

   // could do this more efficiently via "divide & conquer"
   // i.e. split the distance in halfs and compare 
   
   int match_max = 0;
   for ( int i = 1; i <= min_len; i++ )
   {
      match_max = i;
      if ( s1.right( i ) != s2.right( i ) )
      {
         match_max = i - 1;
         break;
      }
   }
   return s1.right( match_max );
}

void US_Hydrodyn_Mals::save_created()
{
   QStringList           created_not_saved_list;

   for ( int i = 0; i < lb_created_files->count(); i++ )
   {
      if ( lb_created_files->item( i )->isSelected() && 
           created_files_not_saved.count( lb_created_files->item( i )->text() ) )
      {
         created_not_saved_list << lb_created_files->item( i )->text();
      }
   }
   save_files( created_not_saved_list );
}

void US_Hydrodyn_Mals::save_created_csv()
{
   QStringList           created_not_saved_list;

   for ( int i = 0; i < lb_created_files->count(); i++ )
   {
      if ( lb_created_files->item( i )->isSelected() ) // && 
         // created_files_not_saved.count( lb_created_files->item( i )->text() ) )
      {
         created_not_saved_list << lb_created_files->item( i )->text();
      }
   }
   save_files_csv( created_not_saved_list );
}

bool US_Hydrodyn_Mals::save_files_csv( QStringList files )
{
   if ( !files.size() )
   {
      editor_msg( "red", us_tr( "Internal error: save_files_csv called empty" ) );
      return false;
   }

   {
      QDir dir1( le_created_dir->text() );
      if ( !dir1.exists() )
      {
         if ( dir1.mkdir( le_created_dir->text() ) )
         {
            editor_msg( "black", QString( us_tr( "Created directory %1" ) ).arg( le_created_dir->text() ) );
         } else {
            editor_msg( "red", QString( us_tr( "Error: Can not create directory %1 Check permissions." ) ).arg( le_created_dir->text() ) );
            return false;
         }
      }
   }         

   if ( !QDir::setCurrent( le_created_dir->text() ) )
   {
      editor_msg( "red", QString( us_tr( "Error: can not set directory %1" ) ).arg( le_created_dir->text() ) );
      return false;
   }

   editor_msg( "black", QString( us_tr( "Current directory is %1" ) ).arg( QDir::current().canonicalPath() ) );

   for ( int i = 0; i < (int)files.size(); i++ )
   {
      if ( !f_qs.count( files[ i ] ) )
      {
         editor_msg( "red", QString( us_tr( "Error: no data found for %1" ) ).arg( files[ i ] ) );
         return false;
      } 
   }

   QString head = qstring_common_head( files, true );

   // QString use_filename = head + ".csv";


   QString use_dir = QDir::current().canonicalPath();

   QString use_filename = QFileDialog::getSaveFileName( this , us_tr( "Select a file name for saving" ) , use_dir , "*.csv" );

   if ( use_filename.isEmpty() )
   {
      return false;
   }

   if ( QFileInfo( use_filename ).suffix().toLower() != ".csv" )
   {
      use_filename += ".csv";
   }
   
   if ( QFile::exists( use_filename ) )
   {
      use_filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( use_filename, 0, this );
      raise();
   }

   QFile f( use_filename );
   if ( !f.open( QIODevice::WriteOnly ) )
   {
      editor_msg( "red", QString( us_tr( "Error: can not open %1 for writing" ) ).arg( use_filename ) );
      return false;
   }

   QTextStream ts( &f );

   // copies for potential cropping:

   map < QString, vector < QString > > t_qs_string;
   map < QString, vector < double > >  t_qs;
   map < QString, vector < double > >  t_Is;
   map < QString, vector < double > >  t_errors;

   bool crop  = false;

   map < QString, bool > selected_files;
   vector < vector < double > > grids;

   for ( int i = 0; i < (int)files.size(); i++ )
   {
      QString this_file = files[ i ];

      if ( !f_qs.count( this_file ) ||
           !f_qs_string.count( this_file ) ||
           !f_Is.count( this_file ) ||
           !f_errors.count( this_file ) )
      {
         editor_msg( "red", QString( us_tr( "Internal error: requested %1, but not found in data" ) ).arg( this_file ) );
         f.close();
         return false;
      }

      selected_files[ this_file ] = true;
      grids.push_back( f_qs[ this_file ] );

      t_qs_string[ this_file ] = f_qs_string[ this_file ];
      t_qs       [ this_file ] = f_qs       [ this_file ];
      t_Is       [ this_file ] = f_Is       [ this_file ];
      t_errors   [ this_file ] = f_errors   [ this_file ];
   }

   vector < double > v_union = US_Vector::vunion( grids );
   vector < double > v_int   = US_Vector::intersection( grids );

   crop = v_union != v_int;

   if ( crop )
   {
      editor_msg( "dark red", QString( us_tr( "Notice: output contains versions cropped to identical grids for compatibility" ) ) );

      map < double, bool > map_int;
      for ( unsigned int i = 0; i < ( unsigned int )v_int.size(); i++ )
      {
         map_int[ v_int[ i ] ] = true;
      }

      for ( map < QString, bool >::iterator it = selected_files.begin();
            it != selected_files.end();
            it++ )
      {
         vector < QString > new_q_string;
         vector < double  > new_q;
         vector < double  > new_I;
         vector < double  > new_e;

         for ( unsigned int i = 0; i < f_qs[ it->first ].size(); i++ )
         {
            if ( map_int.count( f_qs[ it->first ][ i ] ) )
            {
               new_q_string.push_back( t_qs_string[ it->first ][ i ] );
               new_q       .push_back( t_qs       [ it->first ][ i ] );
               new_I       .push_back( t_Is       [ it->first ][ i ] );

               if ( t_errors.count( it->first ) &&
                    t_errors[ it->first ].size() )
               {
                  new_e       .push_back( t_errors   [ it->first ][ i ] );
               }
            }
         }
         t_qs_string[ it->first ] = new_q_string;
         t_qs       [ it->first ] = new_q;
         t_Is       [ it->first ] = new_I;
         if ( t_errors.count( it->first ) &&
              t_errors[ it->first ].size() )
         {
            t_errors[ it->first ] = new_e;
         }
      }
   }

   if ( !t_qs.count( files[ 0 ] ) )
   {
      editor_msg( "red", QString( us_tr( "Internal error: requested %1, but not found in data" ) ).arg( files[ 0 ] ) );
      f.close();
      return false;
   }

   if ( (( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_csv_transposed" ) &&
        (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_csv_transposed" ] == "true" )
   {
      // transpose output

      // header
      ts << "\"q/Time/Frame\",";

      for ( int i = 0; i < (int)files.size(); i++ )
      {
         ts << QString ( "\"%1\"," ).arg( QString( "%1" ).arg( files[ i ] ).replace( "\"", "\'" ) );
         if ( t_errors.count( files[ i ] ) && t_errors[ files[ i ] ].size() && !is_zero_vector( t_errors[ files[ i ] ] ) )
         {
            ts << QString ( "\"S.D. %1\"," ).arg( QString( "%1" ).arg( files[ i ] ).replace( "\"", "\'" ) );
         }
            
      }
      ts << Qt::endl;

      // lines

      for ( unsigned int i = 0; i < (unsigned int)t_qs_string[ files[ 0 ] ].size(); i++ )
      {
         ts << QString( "%1," ).arg( t_qs_string[ files[ 0 ] ][ i ] );
         
         for ( int j = 0; j < (int)files.size(); j++ )
         {
            if ( i < t_Is[ files[ j ] ].size() )
            {
               ts << QString( "%1," ).arg( t_Is[ files[ j ] ][ i ] );
               if ( t_errors.count( files[ j ] ) && t_errors[ files[ j ] ].size() && !is_zero_vector( t_errors[ files[ j ] ] ) )
               {
                  ts << QString( "%1," ).arg( t_errors[ files[ j ] ][ i ] );
               }
            } else {
               ts << ",";
               if ( t_errors.count( files[ j ] ) && t_errors[ files[ j ] ].size() && !is_zero_vector( t_errors[ files[ j ] ] ) )
               {
                  ts << ",";
               }
            }               
         }
         ts << Qt::endl;
      }
   } else {
      QStringList qline;
      for ( unsigned int i = 0; i < t_qs_string[ files[ 0 ] ].size(); i++ )
      {
         qline << t_qs_string[ files[ 0 ] ][ i ];
      }

      ts << 
         QString( "\"Name\",\"Type; q:\",%1,\"%2%3\"\n" )
         .arg( qline.join( "," ) )
         .arg( us_tr( "US-SOMO Hplc output" ) )
         .arg( crop ? us_tr( " cropped" ) : "" );

      for ( int i = 0; i < (int)files.size(); i++ )
      {
         if ( !t_qs.count( files[ i ] ) )
         {
            editor_msg( "red", QString( us_tr( "Internal error: requested %1, but not found in data" ) ).arg( files[ i ] ) );
            f.close();
            return false;
         }
         ts << 
            QString( "\"%1\",\"%2\",%3\n" )
            .arg( files[ i ] )
            .arg( "I(q)" )
            .arg( vector_double_to_csv( t_Is[ files[ i ] ] ) );
         if ( t_errors.count( files[ i ] ) && t_errors[ files[ i ] ].size() && !is_zero_vector( t_errors[ files[ i ] ] ) )
         {
            ts << 
               QString( "\"%1\",\"%2\",%3\n" )
               .arg( files[ i ] )
               .arg( "I(q) sd" )
               .arg( vector_double_to_csv( t_errors[ files[ i ] ] ) );
         }
      }
   }

   f.close();
   editor_msg( "black", QString( us_tr( "%1 written as %2" ) )
               .arg( files.join( " " ) )
               .arg( use_filename ) );
   return true;
}

bool US_Hydrodyn_Mals::save_files( QStringList files )
{

   bool errors = false;
   bool overwrite_all = false;
   bool cancel        = false;
   for ( int i = 0; i < (int)files.size(); i++ )
   {
      if ( !save_file( files[ i ], cancel, overwrite_all ) )
      {
         errors = true;
      }
      if ( cancel )
      {
         editor_msg( "red", us_tr( "save cancelled" ) );
         break;
      }
   }
   update_enables();
   return !errors;
}

bool US_Hydrodyn_Mals::save_file( QString file, bool &cancel, bool &overwrite_all )
{
   if ( !f_qs.count( file ) )
   {
      editor_msg( "red", QString( us_tr( "Error: no data found for %1" ) ).arg( file ) );
      return false;
   } 

   {
      QDir dir1( le_created_dir->text() );
      if ( !dir1.exists() )
      {
         if ( dir1.mkdir( le_created_dir->text() ) )
         {
            editor_msg( "black", QString( us_tr( "Created directory %1" ) ).arg( le_created_dir->text() ) );
         } else {
            editor_msg( "red", QString( us_tr( "Error: Can not create directory %1 Check permissions." ) ).arg( le_created_dir->text() ) );
            return false;
         }
      }
   }         

   if ( !QDir::setCurrent( le_created_dir->text() ) )
   {
      editor_msg( "red", QString( us_tr( "Error: can not set directory %1" ) ).arg( le_created_dir->text() ) );
      return false;
   }

   QString use_filename;
   if ( f_name.count( file ) && !f_name[ file ].isEmpty() )
   {
      use_filename = QFileInfo( f_name[ file ] ).fileName();
   } else {
      use_filename = file + ( conc_files.count( file ) ? ".txt" : ".dat" );
   }

   if ( !overwrite_all && QFile::exists( use_filename ) )
   {
      use_filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck2( use_filename, cancel, overwrite_all, 0, this );
      raise();
      if ( cancel )
      {
         return false;
      }
   }

   QFile f( use_filename );
   if ( !f.open( QIODevice::WriteOnly ) )
   {
      editor_msg( "red", QString( us_tr( "Error: can not open %1 in directory %2 for writing" ) )
                  .arg( use_filename )
                  .arg( QDir::current().canonicalPath() )
                  );
      return false;
   }

   QTextStream ts( &f );

   update_csv_conc();
   map < QString, double > concs = current_concs();

   QString use_conc;
   if ( concs.count( file ) && concs[ file ] != 0e0 )
   {
      use_conc = QString( " Conc:%1 [mg/mL]" ).arg( concs[ file ] );
   } else {
      if ( f_conc.count( file ) && f_conc[ file ] != 0e0 ) 
      {
         use_conc = QString( " Conc:%1" ).arg( f_conc[ file ] );
         if ( f_conc_units.count( file )
              && !f_conc_units[ file ].isEmpty() ) {
            use_conc += QString( " [%1]" ).arg( f_conc_units[ file ] );
         }
      }
   }

   {
      
      QString units = " q units:1/A";
      if ( file.contains( "_Rt_q" ) ) {
         units += " R(t) units:1/cm";
      }
      if ( file.contains( "_Ihasht_q" ) ) {
         units += " I#(t) units:g^2/(cm^3*mol)";
      }
      if ( file.contains( "_Ihashq_" ) ) {
         units += " I#(q) units:g^2/(cm^3*mol)";
      }
      if ( file.contains( "_Istarq_" ) ) {
         units += " I*(q) units:g/mol";
      }
      if ( file.contains( "_Istart_" ) ) {
         units += " I*(t) units:g/mol";
      }
      if ( conc_files.count( file ) ) {
         units = "";
      }
      
      // ts << QString( windowTitle() + us_tr( " %1data: %2%3%4%5%6%7%8%9%10%11%12%13%14\n" ) )
      //    .arg( ( f_is_time.count( file ) && f_is_time[ file ] ? "Frame " : "" ) )
      //    .arg( file )
      //    .arg( units )
      //    .arg( f_psv.count( file ) && f_psv[ file ] ? QString( " PSV:%1 [mL/g]"  ).arg( f_psv [ file ] ) : QString( "" ) )
      //    // .arg( f_I0se.count( file ) ? QString( " I0se:%1" ).arg( f_I0se[ file ] ) : QString( "" ) )
      //    .arg( use_conc ) // f_conc.count( file ) ? QString( " Conc:%1" ).arg( f_conc[ file ] ) : QString( "" ) )
      //    .arg( file.contains( "_Istarq_" ) && f_extc.count( file ) ? QString( " ExtC:%1 [mL/(mg*cm)]" ).arg( f_extc[ file ] ) : QString( "" ) )
      //    .arg( f_time.count( file ) ? QString( " Time:%1" ).arg( f_time[ file ] ) : QString( "" ) )
      //    .arg( !f_extc.count( file ) && f_g_dndc.count( file ) ? QString( " Global_dndc:%1 [ml/g]" ).arg( f_g_dndc[ file ] ) : QString( "" ) )
      //    .arg( f_extc.count( file ) && f_dndc.count( file ) ? QString( " dndc:%1 [ml/g]" ).arg( f_dndc[ file ] ) : QString( "" ) )
      //    .arg( mals_angles.loaded_filename.isEmpty() ? QString("") : QString( " Angles loaded from:%1" ).arg( mals_angles.loaded_filename ) )
      //    .arg( f_ri_corr.count( file ) ? QString( " RI-Corr scatt. angle:%1"  ).arg( f_ri_corr[ file ] ) : QString( "" ) )
      //    .arg( f_ri_corrs.count( file ) ? QString( " RI-Corr scatt. angles:%1"  ).arg( f_ri_corrs[ file ] ) : QString( "" ) )
      //    .arg( f_ref_index.count( file ) ? QString( " Solvent refractive index:%1"  ).arg( f_ref_index[ file ] ) : QString( "" ) )
      //    .arg( f_header.count( file ) ? f_header[ file ] : QString( "" ) )
      //    ;

      ts << QString( windowTitle() + us_tr( " %1data: %2%3%4%5%6%7%8%9%10%11%12%13%14%15%16%17%18%19%20%21%22%23%24%25%26%27\n" ) )
         .arg( ( f_is_time.count( file ) && f_is_time[ file ] ? "Frame " : "" ) )
         .arg( file )
         .arg( units )
         .arg( f_psv .count( file ) ? QString( " PSV:%1 [mL/g]"  ).arg( f_psv [ file ] ) : QString( "" ) )
         .arg( f_I0se.count( file ) ? QString( " I0se:%1 [a.u.]" ).arg( f_I0se[ file ] ) : QString( "" ) )
         .arg( f_I0st.count( file ) ? QString( " I0st:%1 [a.u.]" ).arg( f_I0st[ file ] ) : QString( "" ) )
         .arg( use_conc ) // f_conc.count( file ) ? QString( " Conc:%1" ).arg( f_conc[ file ] ) : QString( "" ) )
         .arg( file.contains( "_Istarq_" ) && f_extc.count( file ) ? QString( " ExtC:%1 [mL/(mg*cm)]" ).arg( f_extc[ file ] ) : QString( "" ) )
         .arg( !file.contains( "_Istarq_" ) && f_extc.count( file ) ? QString( " ExtC_or_DRIinc:%1" ).arg( f_extc[ file ] ) : QString( "" ) )
         .arg( f_time.count( file ) ? QString( " Time:%1" ).arg( f_time[ file ] ) : QString( "" ) )
         .arg( !f_extc.count( file ) && f_g_dndc.count( file ) ? QString( " Global_dndc:%1 [ml/g]" ).arg( f_g_dndc[ file ] ) : QString( "" ) )
         .arg( f_extc.count( file ) && f_dndc.count( file ) ? QString( " dndc:%1 [ml/g]" ).arg( f_dndc[ file ] ) : QString( "" ) )
         .arg( f_diffusion_len.count( file ) ? QString( " Diffusion Length: %1 [cm]" ).arg( f_diffusion_len[ file ] ) : QString("") )
         .arg( f_e_nucleon_ratio.count( file ) ? QString( " Electron/nucleon ratio Z/A: %1" ).arg( f_e_nucleon_ratio[ file ] ) : QString("") )
         .arg( f_nucleon_mass.count( file ) ? QString( " Nucleon mass: %1 [g]" ).arg( f_nucleon_mass[ file ] ) : QString("") )
         .arg( f_solvent_e_dens.count( file ) ? QString( " Solvent e density: %1 [e A^-3]" ).arg( f_solvent_e_dens[ file ] ) : QString("") )
         .arg( f_header.count( file ) ? f_header[ file ] : QString( "" ) )
         .arg( mals_angles.loaded_filename.isEmpty() ? QString("") : QString( " Angles loaded from:%1" ).arg( mals_angles.loaded_filename ) )
         .arg( f_ri_corr.count( file ) ? QString( " RI-Corr scatt. angle:%1"  ).arg( f_ri_corr[ file ] ) : QString( "" ) )
         .arg( f_ri_corrs.count( file ) ? QString( " RI-Corr scatt. angles:%1"  ).arg( f_ri_corrs[ file ] ) : QString( "" ) )
         .arg( f_ref_index.count( file ) ? QString( " Solvent refractive index:%1"  ).arg( f_ref_index[ file ] ) : QString( "" ) )
         .arg( f_header.count( file ) ? f_header[ file ] : QString( "" ) )
         .arg( f_fit_curve.count( file ) ? QString( " Fit curve:%1" ).arg( f_fit_curve[ file ] ) : QString( "" ) )
         .arg( f_fit_method.count( file ) ? QString( " Fit method:%1" ).arg( f_fit_method[ file ] ) : QString( "" ) )
         .arg( f_fit_q_ranges.count( file ) ? QString( " Fit q ranges:%1" ).arg( f_fit_q_ranges[ file ] ) : QString( "" ) )
         .arg( f_fit_chi2.count( file ) ? QString( " Fit chi^2:%1" ).arg( f_fit_chi2[ file ] ) : QString( "" ) )
         .arg( f_fit_sd_scale.count( file ) ? QString( " Fit MALS SD mult:%1" ).arg( f_fit_sd_scale[ file ] ) : QString( "" ) )
         ;
   }

   bool use_errors = ( f_errors.count( file ) && 
                       f_errors[ file ].size() > 0 );
                       // is_nonzero_vector( f_errors[ file ] ) );

   if ( f_is_time.count( file ) && f_is_time[ file ] )
   {
      if ( conc_files.count( file ) ) {
         if ( use_errors )
         {
            ts << "t                 \tA(t)         \tsd\n";
         } else {
            ts << "t                 \tA(t)\n";
         }
      } else {
         if ( use_errors )
         {
            ts << "t                 \tI(t)         \tsd\n";
         } else {
            ts << "t                 \tI(t)\n";
         }
      }
   } else {
      if ( use_errors )
      {
         ts << "q                 \tI(q)         \tsd\n";
      } else {
         ts << "q                 \tI(q)\n";
      }
   }

   for ( int i = 0; i < (int)f_qs[ file ].size(); i++ )
   {
      if ( use_errors &&
           (int)f_errors[ file ].size() > i )
      {
         ts << QString("").sprintf( "%-18s\t%.6e\t%.6e\n",
                                    f_qs_string[ file ][ i ].toLatin1().data(),
                                    f_Is       [ file ][ i ],
                                    f_errors   [ file ][ i ] );
      } else {
         ts << QString("").sprintf( "%-18s\t%.6e\n",
                                    f_qs_string[ file ][ i ].toLatin1().data(),
                                    f_Is       [ file ][ i ] );
      }
   }

   f.close();
   editor_msg( "black", QString( us_tr( "%1 written as %2" ) )
               .arg( file )
               .arg( use_filename ) );
   created_files_not_saved.erase( file );
   f_name[ file ] = QDir::current().path() + QDir::separator() + use_filename;
   //   TSO << QString( "file <%1> path <%2>\n" ).arg( file ).arg( f_name[ file ] );
   return true;
}

void US_Hydrodyn_Mals::update_csv_conc()
{
   map < QString, bool > skip;
   //    if ( !lbl_mals->text().isEmpty() )
   //    {
   //       skip[ lbl_mals->text() ] = true;
   //    }
   //    if ( !lbl_empty->text().isEmpty() )
   //    {
   //       skip[ lbl_empty->text() ] = true;
   //    }

   if ( !csv_conc.data.size() )
   {
      // setup & add all
      csv_conc.name = "Solution Concentrations ";

      csv_conc.header.clear( );
      csv_conc.header_map.clear( );
      csv_conc.data.clear( );
      csv_conc.num_data.clear( );
      csv_conc.prepended_names.clear( );
      
      csv_conc.header.push_back("File");
      csv_conc.header.push_back("Concentration\nmg/ml");
      
      for ( int i = 0; i < lb_files->count(); i++ )
      {
         if ( !skip.count( lb_files->item( i )->text() ) )
         {
            vector < QString > tmp_data;
            tmp_data.push_back( lb_files->item( i )->text() );
            tmp_data.push_back( "" );
            
            csv_conc.prepended_names.push_back(tmp_data[0]);
            csv_conc.data.push_back(tmp_data);
         }
      }
   } else {
      map < QString, bool > current_files;
      map < QString, bool > csv_files;
      for ( int i = 0; i < lb_files->count(); i++ )
      {
         if ( !skip.count( lb_files->item( i )->text() ) )
         {
            current_files[ lb_files->item( i )->text() ] = true;
         }
      }
      csv new_csv = csv_conc;
      new_csv.data.clear( );
      new_csv.num_data.clear( );
      new_csv.prepended_names.clear( );
      for ( unsigned int i = 0; i < csv_conc.data.size(); i++ )
      {
         csv_files[ csv_conc.data[ i ][ 0 ] ] = true;
         if ( current_files.count( csv_conc.data[ i ][ 0 ] ) )
         {
            new_csv.data.push_back( csv_conc.data[ i ] );
            new_csv.prepended_names.push_back( csv_conc.data[ i ][ 0 ] );
         }
      }
      // add new ones
      for ( int i = 0; i < lb_files->count(); i++ )
      {
         if ( !skip.count( lb_files->item( i )->text() ) &&
              !csv_files.count( lb_files->item( i )->text() ) )
         {
            vector < QString > tmp_data;
            tmp_data.push_back( lb_files->item( i )->text() );
            tmp_data.push_back( "" );
            
            new_csv.prepended_names.push_back(tmp_data[0]);
            new_csv.data.push_back(tmp_data);
         }
      }
      csv_conc = new_csv;
   }      

   for ( unsigned int i = 0; i < csv_conc.data.size(); i++ )
   {
      vector < double > tmp_num_data;
      for ( unsigned int j = 0; j < csv_conc.data[i].size(); j++ )
      {
         tmp_num_data.push_back(csv_conc.data[i][j].toDouble());
      }
      csv_conc.num_data.push_back(tmp_num_data);
   }
}   

void US_Hydrodyn_Mals::conc()
{
   update_csv_conc();
   if ( conc_widget )
   {
      if ( conc_window->isVisible() )
      {
         conc_window->raise();
      } else {
         conc_window->show();
      }
   } else {
      conc_window = new US_Hydrodyn_Mals_Conc( csv_conc,
                                                      this );
      conc_window->show();
   }
}

map < QString, double > US_Hydrodyn_Mals::current_concs( bool quiet )
{
   map < QString, double > concs;
   map < QString, double > concs_in_widget;
   for ( unsigned int i = 0; i < csv_conc.data.size(); i++ )
   {
      if ( csv_conc.data[ i ].size() > 1 )
      {
         concs[ csv_conc.data[ i ][ 0 ] ] =  csv_conc.data[ i ][ 1 ].toDouble();
      }
   }
   if ( conc_widget )
   {
      csv tmp_csv = conc_window->current_csv();
      bool any_different = false;
      for ( unsigned int i = 0; i < tmp_csv.data.size(); i++ )
      {
         if ( tmp_csv.data[ i ].size() > 1 )
         {
            if ( concs.count( tmp_csv.data[ i ][ 0 ] ) &&
                 concs[ tmp_csv.data[ i ][ 0 ] ] != tmp_csv.data[ i ][ 1 ].toDouble() )
            {
               any_different = true;
               break;
            }
         }
      }
      if ( !quiet && any_different )
      {
         QMessageBox::warning( this, 
                               windowTitle(),
                               us_tr( "There are unsaved updates in the open Solution Concentration window\n"
                                   "This will cause the concentration values used by the current calculation\n"
                                   "to differ from those shown in the Solution Concentration window\n"
                                   "You probably want to save the values in the Solution Concentration window and repeat the computation."
                                   ) );
      }
   }
   return concs;
}

map < QString, double > US_Hydrodyn_Mals::window_concs()
{
   map < QString, double > concs;
   if ( conc_widget )
   {
      csv tmp_csv = conc_window->current_csv();
      for ( unsigned int i = 0; i < tmp_csv.data.size(); i++ )
      {
         if ( tmp_csv.data[ i ].size() > 1 )
         {
            concs[ tmp_csv.data[ i ][ 0 ] ] = tmp_csv.data[ i ][ 1 ].toDouble();
         }
      }
   }
   return concs;
}
   
void US_Hydrodyn_Mals::conc_avg()
{
   //    QStringList files;
   //    for ( int i = 0; i < lb_files->count(); i++ )
   //    {
   //       if ( lb_files->item( i )->isSelected() && 
   //            lb_files->item( i )->text() != lbl_mals->text() &&
   //            lb_files->item( i )->text() != lbl_empty->text() )
   //       {
   //          files << lb_files->item( i )->text();
   //       }
   //    }
   conc_avg( all_selected_files() );
}

vector < double > US_Hydrodyn_Mals::union_q( QStringList files )
{
   map < double, bool > used_q;
   list < double >      ql;

   for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
   {
      if ( !f_qs.count( files[ i ] ) )
      {
         editor_msg( "red", QString( us_tr( "Internal error: request to use %1, but not found in data" ) ).arg( files[ i ] ) );
      } else {
         for ( unsigned int j = 0; j < ( unsigned int ) f_qs[ files[ i ] ].size(); i++ )
         {
            if ( !used_q.count( f_qs[ files[ i ] ][ j ] ) )
            {
               ql.push_back( f_qs[ files[ i ] ][ j ] );
               used_q[ f_qs[ files[ i ] ][ j ]  ] = true;
            }
         }
      }
   }

   ql.sort();

   vector < double > q;
   for ( list < double >::iterator it = ql.begin();
         it != ql.end();
         it++ )
   {
      q.push_back( *it );
   }
   return q;
}

void US_Hydrodyn_Mals::smooth()
{
   QStringList files = all_selected_files();
   smooth( files );
}

bool US_Hydrodyn_Mals::get_peak( QString file, double &peak, bool full )
{
   double pos;
   return get_peak( file, peak, pos, full );
   // if ( !f_Is.count( file ) )
   // {
   //    editor_msg( "red", QString( us_tr( "Internal error: get_peak requested on %1 but no data available" ) ).arg( file ) );
   //    return false;
   // }

   // if ( !f_Is[ file ].size() )
   // {
   //    editor_msg( "red", QString( us_tr( "Internal error: get_peak requested on %1 but data empty" ) ).arg( file ) );
   //    return false;
   // }
      
   // peak = f_Is[ file ][ 0 ];
   // for ( unsigned int i = 1; i < ( unsigned int ) f_Is[ file ].size(); i++ )
   // {
   //    if ( peak < f_Is[ file ][ i ] )
   //    {
   //       peak = f_Is[ file ][ i ];
   //    }
   // }
   // return true;
}

bool US_Hydrodyn_Mals::get_peak( QString file, double &peak, double &pos, bool full )
{
   if ( !f_Is.count( file ) )
   {
      editor_msg( "red", QString( us_tr( "Internal error: get_peak requested on %1 but no data available" ) ).arg( file ) );
      return false;
   }

   if ( !f_Is[ file ].size() )
   {
      editor_msg( "red", QString( us_tr( "Internal error: get_peak requested on %1 but data empty" ) ).arg( file ) );
      return false;
   }
      
   peak = f_Is[ file ][ 0 ];
   pos = f_qs[ file ][ 0 ];

   double start_pos = le_gauss_fit_start->text().toDouble();
   double end_pos   = le_gauss_fit_end  ->text().toDouble();
   
   if ( le_gauss_fit_start->text().isEmpty() ||
        le_gauss_fit_start->text().toDouble() < f_qs[ file ][ 0 ] )
   {
      start_pos = f_qs[ file ][ 0 ];
   }
   if ( le_gauss_fit_end->text().isEmpty() ||
        le_gauss_fit_end->text().toDouble() > f_qs[ wheel_file ].back() )
   {
      end_pos = f_qs[ file ].back();
   }

   if ( full ) {
      start_pos = f_qs[ file ][ 0 ];
      end_pos   = f_qs[ file ].back();
   }

   if ( end_pos <= start_pos ) {
      return false;
   }

   // get 1st
   bool found_first = false;
   unsigned int i = 0;
   for ( ; i < ( unsigned int ) f_Is[ file ].size(); ++i ) {
      double this_pos = f_qs[ file ][ i ];
      if ( this_pos >= start_pos &&
           this_pos <= end_pos ) {
         found_first = true;
         peak = f_Is[ file ][ i ];
         pos  = this_pos;
         break;
      }
   }

   if ( !found_first ) {
      editor_msg( "red", QString( us_tr( "Warning: get_peak: file %1 has no points between the fitting start and end range" ) ).arg( file ) );
      return false;
   }

   for ( ; i < ( unsigned int ) f_Is[ file ].size(); i++ )
   {
      double this_pos = f_qs[ file ][ i ];
      if ( this_pos <= end_pos ) {
         if ( peak < f_Is[ file ][ i ] ) {
            peak = f_Is[ file ][ i ];
            pos  = this_pos;
         }
      }
   }
   return true;
}

void US_Hydrodyn_Mals::smooth( QStringList files )
{
   bool ok;
   int smoothing = US_Static::getInteger(
                                            us_tr( "SOMO: HPLC enter smoothing" ),
                                            us_tr( "Enter the number of points of smoothing:" ),
                                            1, 
                                            1,
                                            50,
                                            1, 
                                            &ok, 
                                            this 
                                            );
   if ( !ok ) {
      return;
   }

   map < QString, bool > current_files;
   for ( int i = 0; i < (int)lb_files->count(); i++ )
   {
      current_files[ lb_files->item( i )->text() ] = true;
   }

   map < QString, bool > select_files;

   US_Saxs_Util usu;
   vector < double > smoothed_I;
   for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
   {
      int ext = 0;
      QString smoothed_name = files[ i ] + QString( "-sm%1" ).arg( smoothing );
      while ( current_files.count( smoothed_name ) )
      {
         smoothed_name = files[ i ] + QString( "-sm%1-%2" ).arg( smoothing ).arg( ++ext );
      }

      if ( usu.smooth( f_Is[ files[ i ] ], smoothed_I, smoothing ) )
      {
         select_files[ smoothed_name ] = true;

         lb_created_files->addItem( smoothed_name );
         lb_created_files->scrollToItem( lb_created_files->item( lb_created_files->count() - 1 ) );
         lb_files->addItem( smoothed_name );
         lb_files->scrollToItem( lb_files->item( lb_files->count() - 1 ) );
         created_files_not_saved[ smoothed_name ] = true;
   
         f_pos       [ smoothed_name ] = f_qs.size();
         f_qs_string [ smoothed_name ] = f_qs_string[ files[ i ] ];
         f_qs        [ smoothed_name ] = f_qs       [ files[ i ] ];
         f_Is        [ smoothed_name ] = smoothed_I;
         f_errors    [ smoothed_name ] = f_errors   [ files[ i ] ];
         f_is_time   [ smoothed_name ] = f_is_time  [ files[ i ] ];
         f_conc      [ smoothed_name ] = f_conc.count( files[ i ] ) ? f_conc[ files[ i ] ] : 0e0;
         f_psv       [ smoothed_name ] = f_psv .count( files[ i ] ) ? f_psv [ files[ i ] ] : 0e0;
         f_I0se      [ smoothed_name ] = f_I0se .count( files[ i ] ) ? f_I0se [ files[ i ] ] : 0e0;
         {
            vector < double > tmp;
            f_gaussians  [ smoothed_name ] = tmp;
         }
         editor_msg( "gray", QString( "Created %1\n" ).arg( smoothed_name ) );
      } else {
         editor_msg( "red", QString( "Error: smoothing error trying to create %1\n" ).arg( smoothed_name ) );
      }
   }

   disable_updates = true;

   lb_files->clearSelection();
   for ( int i = 0; i < (int)lb_files->count(); i++ )
   {
      if ( select_files.count( lb_files->item( i )->text() ) )
      {
         lb_files->item( i)->setSelected( true );
      }
   }

   disable_updates = false;
   plot_files();

   update_enables();
}

void US_Hydrodyn_Mals::create_i_of_t()
{
   disable_all();

   QStringList files = all_selected_files();
   create_i_of_t( files );

   update_enables();
}

// #define USHC_TIMERS

void US_Hydrodyn_Mals::create_i_of_t( QStringList files )
{
   // find common q 
   QString head = qstring_common_head( files, true );
   QString tail = qstring_common_tail( files, true );

   // map: [ timestamp ][ q value ] = intensity

   map < double, map < double , double > > I_values;
   map < double, map < double , double > > e_values;

   map < double, bool > used_q;
   list < double >      ql;

   QRegExp rx_cap( "(\\d+)_(\\d+)" );
   QRegExp rx_clear_nonnumeric( "^(\\d*_?\\d+)([^0-9_]|_[a-zA-Z])" );
   // rx_cap.setMinimal( true );

#ifdef USHC_TIMERS
   US_Timer           us_timers;
   us_timers          .clear_timers();
   us_timers.init_timer( "get qs" );
   us_timers.init_timer( "ql.sort" );
   us_timers.init_timer( "ql.sort post" );
   us_timers.init_timer( "build up q" );
   us_timers.init_timer( "check_discard_it_sd_mult" );
   us_timers.init_timer( "check_zi_window" );

   us_timers.start_timer( "get qs" );
#endif

#ifdef OLDWAY
   for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
   {
      if ( !f_qs.count( files[ i ] ) )
      {
         editor_msg( "red", QString( us_tr( "Internal error: request to use %1, but not found in data" ) ).arg( files[ i ] ) );
      } else {
         QString tmp = files[ i ].mid( head.length() );
         tmp = tmp.mid( 0, tmp.length() - tail.length() );
         if ( rx_clear_nonnumeric.indexIn( tmp ) != -1 )
         {
            tmp = rx_clear_nonnumeric.cap( 1 );
         }

         if ( rx_cap.indexIn( tmp ) != -1 )
         {
            tmp = rx_cap.cap( 1 ) + "." + rx_cap.cap( 2 );
         }
         double timestamp = tmp.toDouble();
#ifdef DEBUG_LOAD_REORDER
         us_qdebug( QString( "%1 tmp %2 value %3" ).arg( files[ i ] ).arg( tmp ).arg( timestamp ) );
#endif
         for ( unsigned int j = 0; j < ( unsigned int ) f_qs[ files[ i ] ].size(); j++ )
         {
            I_values[ timestamp ][ f_qs[ files[ i ] ][ j ] ] = f_Is[ files[ i ] ][ j ];
            if ( f_errors[ files[ i ] ].size() )
            {
               e_values[ timestamp ][ f_qs[ files[ i ] ][ j ] ] = f_errors[ files[ i ] ][ j ];
            }
            if ( !used_q.count( f_qs[ files[ i ] ][ j ] ) )
            {
               ql.push_back( f_qs[ files[ i ] ][ j ] );
               used_q[ f_qs[ files[ i ] ][ j ] ] = true;
            }
         }
      }
   }
#endif

   {
      QString     * qs;
      double      * q;
      double      * I;
      double      * e;
      unsigned int size;
      double        qv;

      map < double , double > * Ivp;
      map < double , double > * evp;

      for ( unsigned int i = 0; i < ( unsigned int ) files.size(); ++i )
      {
      
         qs = &( files[ i ] );

         if ( !f_qs.count( *qs ) )
         {
            editor_msg( "red", QString( us_tr( "Internal error: request to use %1, but not found in data" ) ).arg( *qs ) );
         } else {
            QString tmp = qs->mid( head.length() );
            tmp = tmp.mid( 0, tmp.length() - tail.length() );
            if ( rx_clear_nonnumeric.indexIn( tmp ) != -1 )
            {
               tmp = rx_clear_nonnumeric.cap( 1 );
            }

            if ( rx_cap.indexIn( tmp ) != -1 )
            {
               tmp = rx_cap.cap( 1 ) + "." + rx_cap.cap( 2 );
            }
            double timestamp = tmp.toDouble();
#ifdef DEBUG_LOAD_REORDER
            us_qdebug( QString( "%1 tmp %2 value %3" ).arg( *qs ).arg( tmp ).arg( timestamp ) );
#endif

            Ivp = &(I_values[ timestamp ]);
            evp = &(e_values[ timestamp ]);

            q = &(f_qs[ *qs ][ 0 ]);
            I = &(f_Is[ *qs ][ 0 ]);
            size =  ( unsigned int ) f_qs[ *qs ].size();

            if ( f_errors[ *qs ].size() )
            {
               e = &(f_errors[ *qs ][ 0 ]);
               for ( unsigned int j = 0; j < size; j++ )
               {
                  qv = q[ j ];
                  (*Ivp)[ qv ] = I[ j ];
                  (*evp)[ qv ] = e[ j ];
                  if ( !used_q.count( qv ) )
                  {
                     ql.push_back( qv );
                     used_q[ qv ] = true;
                  }
               }
            } else {
               for ( unsigned int j = 0; j < size; j++ )
               {
                  qv = q[ j ];
                  (*Ivp)[ qv ] = I[ j ];
                  if ( !used_q.count( qv ) )
                  {
                     ql.push_back( qv );
                     used_q[ qv ] = true;
                  }
               }
            }
         }
      }
   }

#ifdef USHC_TIMERS
   us_timers.end_timer( "get qs" );
   us_timers.start_timer( "ql.sort" );
#endif
   ql.sort();

#ifdef USHC_TIMERS
   us_timers.end_timer( "ql.sort" );
   us_timers.start_timer( "ql.sort post" );
#endif

   vector < double > q;
   vector < QString > q_qs;
   for ( list < double >::iterator it = ql.begin();
         it != ql.end();
         it++ )
   {
      q.push_back( *it );
   }

   map < QString, bool > current_files;
   for ( int i = 0; i < lb_files->count(); ++i )
   {
      current_files[ lb_files->item( i )->text() ] = true;
   }

#ifdef USHC_TIMERS
   us_timers.end_timer( "ql.sort post" );
   us_timers.start_timer        ( "build up q" );
#endif

   QStringList created_files;

   set < QString > to_select;
   
   {
      unsigned int size = ( unsigned int )q.size();

      for ( unsigned int i = 0; i < size; ++i )
      {
         double qv = q[ i ];
         QString basename = QString( "%1_It_q%2" ).arg( head ).arg( qv );
         basename.replace( ".", "_" );
      
         unsigned int ext = 0;
         QString fname = basename + tail;
         while ( current_files.count( fname ) )
         {
            fname = basename + QString( "-%1" ).arg( ++ext ) + tail;
         }
         editor_msg( "gray", fname );

         vector < double  > t;
         vector < QString > t_qs;
         vector < double  > I;
         vector < double  > e;

         for ( map < double, map < double , double > >::iterator it = I_values.begin();
               it != I_values.end();
               it++ )
         {
            t   .push_back( it->first );
            t_qs.push_back( QString( "" ).sprintf( "%.8f", it->first ) );
            if ( it->second.count( qv ) )
            {
               I.push_back( it->second[ qv ] );
            } else {
               I.push_back( 0e0 );
            }
            if ( e_values.count( it->first ) &&
                 e_values[ it->first ].count( qv ) )
            {
               e.push_back( e_values[ it->first ][ qv ] );
            } else {
               e.push_back( 0e0 );
            }
         }

         created_files_not_saved[ fname ] = true;
   
         f_pos       [ fname ] = f_qs.size();
         f_qs_string [ fname ] = t_qs;
         f_qs        [ fname ] = t;
         f_Is        [ fname ] = I;
         f_errors    [ fname ] = e;
         f_is_time   [ fname ] = true;
         f_conc      [ fname ] = 0e0;
         f_psv       [ fname ] = 0e0;
         f_I0se      [ fname ] = 0e0;
         {
            vector < double > tmp;
            f_gaussians  [ fname ] = tmp;
         }
         created_files << fname;
         to_select.insert( fname );
      }      
      lb_created_files->addItems( created_files );
      lb_files->addItems( created_files );
      lb_created_files->scrollToItem( lb_created_files->item( lb_created_files->count() - 1 ) );
      lb_files->scrollToItem( lb_files->item( lb_files->count() - 1 ) );
   }

#ifdef USHC_TIMERS
   us_timers.end_timer        ( "build up q" );
   us_timers.start_timer      ( "check_discard_it_sd_mult" );
#endif

   check_discard_it_sd_mult( created_files );

#ifdef USHC_TIMERS
   us_timers.end_timer        ( "check_discard_it_sd_mult" );
   us_timers.start_timer      ( "check_zi_window" );
#endif

   (void) check_zi_window( created_files );

#ifdef USHC_TIMERS
   us_timers.end_timer        ( "check_zi_window" );

   us_qdebug( us_timers.list_times() );
#endif

   set_selected( to_select );
   
   if ( axis_x_log ) {
      axis_x();
   }
   
   if ( axis_y_log ) {
      axis_y();
   }
   
   rescale();

   update_enables();
}

void US_Hydrodyn_Mals::test_i_of_t()
{
   disable_all();
   QStringList files = all_selected_files();
   check_discard_it_sd_mult( files, true );
   if ( check_zi_window( files ) )
   {
      editor_msg( "blue", QString( us_tr( "Test I(t) ok" ) ) );
   }
   update_enables();
}

bool US_Hydrodyn_Mals::check_zi_window( QStringList & files, const QString & extra_text )
{
   int window = ((US_Hydrodyn *)us_hydrodyn)->gparams[ "mals_zi_window" ].toInt();

   if ( !files.size() )
   {
      return true;
   }

   if ( window >= (int) f_qs[ files[ 0 ] ].size() )
   {
      
      QMessageBox::warning(
                           this,
                           this->windowTitle() + us_tr(": I(t) negative integral window test" ),
                           QString( us_tr(
                                        "The created I(t) curves have fewer time points (%1) than the negative integral window size (%2)\n"
                                        "The negative integral window test will not be performed.\n"
                                        "You may wish to decrease this value in 'Options'.\n"
                                        "Typically this might occur at large q values where the detector instabilities surpass the associated errors.\n"
                                        "It could also arise from problems with buffer subtraction.\n"
                                        ) )
                            .arg( f_qs[ files[ 0 ] ].size() )
                            .arg( window ),
                            QMessageBox::Ok | QMessageBox::Default,
                            QMessageBox::NoButton
                            );
      return false;
   }

   QStringList messages;

   double      * I;
   double      * e;

   for ( int i = 0; i < (int) files.size(); ++i )
   {
      QString this_file = files[ i ];
      int pts = (int) f_qs[ this_file ].size();
      bool use_errors = ( f_errors.count( this_file ) && f_qs[ this_file ].size() == f_errors[ this_file ].size() );

      I = &(f_Is[ this_file ][ 0 ]);

      if ( use_errors )
      {
         e = &(f_errors[ this_file ][ 0 ]);
         for ( int j = 0; j < pts; ++j )
         {
            if ( j + window < pts )
            {
               double integral = 0e0;
               double sdsum    = 0e0;
               for ( int k = 0; k < window; ++k )
               {
                  integral += I[ j + k ];
                  sdsum    += e[ j + k ];
               }
               if ( integral < -sdsum )
               {
                  messages << QString( "%1 negative integral region %2 below negative sum of S.D. %3" )
                     .arg( this_file )
                     .arg( integral )
                     .arg( -sdsum );
                  break;
               }
            }
         }
      } else {
         for ( int j = 0; j < pts; ++j )
         {
            if ( j + window < pts )
            {
               double integral = 0e0;
               for ( int k = 0; k < window; ++k )
               {
                  integral += I[ j + k ];
               }
               if ( integral < 0e0 )
               {
                  messages << QString( "%1 negative integral region %2" )
                     .arg( this_file )
                     .arg( integral );
                  break;
               }
            }
         }
      }
   }

   if ( messages.size() )
   {
      QStringList qsl;
      for ( int i = 0; i < (int)messages.size() && i < 10; i++ )
      {
         qsl << messages[ i ];
      }

      if ( qsl.size() < messages.size() )
      {
         qsl << QString( us_tr( "... and %1 more not listed" ) ).arg( messages.size() - qsl.size() );
      }


      QMessageBox msg(
                      QMessageBox::Warning,
                      this->windowTitle() + us_tr(": I(t) negative integral window test" ),
                      QString( extra_text +
                               us_tr( "Please note:\n\n"
                                      "These files have failed the negative integral window test of size %1\n"
                                      "%2\n\n"
                                      "Typically this might occur at large q values where the detector instabilities surpass the associated errors.\n"
                                      "It could also arise from problems with buffer subtraction.\n"
                                      ) )
                      .arg( window )
                      .arg( qsl.join( "\n" ) ),
                      QMessageBox::Ok | QMessageBox::Default,
                      this
                      );

      QSpacerItem* horizontalSpacer = new QSpacerItem(800, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
      QGridLayout* layout = (QGridLayout*)msg.layout();
      layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

      msg.exec();
      
      return false;
   }
   
   return true;
}

void US_Hydrodyn_Mals::check_discard_it_sd_mult( QStringList & files, bool optionally_discard )
{
   if ( ((US_Hydrodyn *)us_hydrodyn)->gparams[ "mals_cb_discard_it_sd_mult" ] != "true" ||
        !files.size() )
   {
      return;
   }

   double mult = ((US_Hydrodyn *)us_hydrodyn)->gparams[ "mals_discard_it_sd_mult" ].toDouble();

   QStringList removefiles;
   QStringList keepfiles;
   set < QString > removefileset;

   for ( int i = 0; i < (int) files.size(); ++i )
   {
      QString this_file = files[ i ];
      bool skip = true;

      if ( f_errors[ this_file ].size() == f_qs[ this_file ].size() )
      {
         for ( int j = 0; j < (int) f_qs[ this_file ].size(); ++j )
         {
            if ( f_Is[ this_file ][ j ] > mult * f_errors[ this_file ][ j ] )
            {
               skip = false;
               break;
            }
         }

         if ( skip )
         {
            removefiles << this_file;
            removefileset.insert( this_file );
         } else {
            keepfiles << this_file;
         }
      }
   }

   if ( removefiles.size() )
   {
      QStringList qsl;
      for ( int i = 0; i < (int)removefiles.size() && i < 15; i++ )
      {
         qsl << removefiles[ i ];
      }

      if ( qsl.size() < removefiles.size() )
      {
         qsl << QString( us_tr( "... and %1 more not listed" ) ).arg( removefiles.size() - qsl.size() );
      }

      if ( optionally_discard )
      {
         switch ( QMessageBox::question(this, 
                                        this->windowTitle() + us_tr(": Test I(t), discard I(t)" ),
                                        QString( us_tr( "Please note:\n\n"
                                                     "Make I(t), discard I(t) with no signal above std. dev. multiplied by %1\n"
                                                     "These curves have been marked to discard\n"
                                                     "%2\n\n"
                                                     "What would you like to do?"
                                                     ) )
                                        .arg( mult )
                                        .arg( qsl.join( "\n" ) ),
                                        us_tr( "&Discard" ), 
                                        us_tr( "&Keep" ),
                                        QString(),
                                        1, // Stop == button 0
                                        1 // Escape == button 0
                                        ) )
         {
         case 0 : // discard
            break;
         case 1 : // keep
            return;
            break;
         }  
      } else {
         QMessageBox::information(
                                  this,
                                  this->windowTitle() + us_tr(": Make I(t), discard I(t)" ),
                                  QString( us_tr( "Please note:\n\n"
                                               "Make I(t), discard I(t) with no signal above std. dev. multiplied by %1\n"
                                               "These curves will be discarded\n"
                                               "%2\n\n"
                                               ) )
                                  .arg( mult )
                                  .arg( qsl.join( "\n" ) ),
                                  QMessageBox::Ok | QMessageBox::Default,
                                  QMessageBox::NoButton
                                  );
      }
      remove_files( removefileset, optionally_discard );
      files = keepfiles;
   }
   return;
}

bool US_Hydrodyn_Mals::all_selected_have_nonzero_conc()
{
   map < QString, double > concs = current_concs( true );
   map < QString, double > nonzero_concs;

   for ( map < QString, double >::iterator it = concs.begin();
         it != concs.end();
         it++ )
   {
      if ( it->second != 0e0 )
      {
         nonzero_concs[ it->first ] = it->second;
      }
   }

   unsigned int selected_count = 0;
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() 
           //            && 
           //            lb_files->item( i )->text() != lbl_mals->text() &&
           //            lb_files->item( i )->text() != lbl_empty->text() 
           )
      {
         selected_count++;
         if ( !nonzero_concs.count( lb_files->item( i )->text() ) )
         {
            return false;
         }
      }
   }

   return selected_count > 0;
}

void US_Hydrodyn_Mals::delete_zoomer_if_ranges_changed()
{
}

void US_Hydrodyn_Mals::show_created()
{
   map < QString, bool > created_selected;

   for ( int i = 0; i < lb_created_files->count(); i++ )
   {
      if ( lb_created_files->item( i )->isSelected() )
      {
         created_selected[ lb_created_files->item( i )->text() ] = true;
      }
   }

   disable_updates = true;
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( !lb_files->item( i )->isSelected() && 
           created_selected.count( lb_files->item( i )->text() ) )
      {
         lb_files->item( i)->setSelected( true );
      }
   }
   disable_updates = false;
   plot_files();
   update_enables();
      
}

void US_Hydrodyn_Mals::show_only_created()
{
   disable_updates = true;
   lb_files->clearSelection();
   show_created();
}


QString US_Hydrodyn_Mals::vector_double_to_csv( vector < double > vd )
{
   QString result;
   for ( unsigned int i = 0; i < vd.size(); i++ )
   {
      result += QString("%1,").arg(vd[i]);
   }
   return result;
}

void US_Hydrodyn_Mals::join()
{
   vector < QString > selected;
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         selected.push_back( lb_files->item( i )->text() );
      }
   }

   if ( selected.size() != 2 )
   {
      return;
   }

   disable_all();

   // swap if 1 ends last
   if ( f_qs[ selected[ 0 ] ].back() > f_qs[ selected[ 1 ] ].back() )
   {
      QString tmp   = selected[ 0 ];
      selected[ 0 ] = selected[ 1 ];
      selected[ 1 ] = tmp;
   }

   // find q intersection

   double q0_min = f_qs[ selected[ 0 ] ][ 0 ];
   double q0_max = f_qs[ selected[ 0 ] ].back();

   double q1_min = f_qs[ selected[ 1 ] ][ 0 ];
   double q1_max = f_qs[ selected[ 1 ] ].back();

   double q_max_min = q0_min < q1_min ? q1_min : q0_min;
   double q_min_max = q0_max < q1_max ? q0_max : q1_max;

   double q_join;

   // do we have overlap?
   if ( q_max_min < q_min_max )
   {
      // ask for overlap point
      bool ok;
      double res = US_Static::getDouble(
                                           us_tr( "US-SOMO: MALS: Join" ),
                                           QString( us_tr( "The curves %1 and %2\n"
                                                        "have an overlap q-range of %3 to %4.\n"
                                                        "Enter the join q-value:" ) )
                                           .arg( selected[ 0 ] )
                                           .arg( selected[ 1 ] )
                                           .arg( q_max_min )
                                           .arg( q_min_max )
                                           ,
                                           q_max_min,
                                           q_max_min,
                                           q_min_max,
                                           4,
                                           &ok,
                                           this
                                           );
      if ( ok ) {
         // user entered something and pressed OK
         q_join = res;
      } else {
         // user pressed Cancel
         update_enables();
         return;
      }
   } else {
      q_join = q0_max;
   }

   double avg_conc = 0e0;
   double avg_psv  = 0e0;
   double avg_I0se = 0e0;

   // join them
   vector < QString > q_string;
   vector < double >  q;
   vector < double >  I;
   vector < double >  e;

   bool use_errors = f_errors[ selected[ 0 ] ].size() && f_errors[ selected[ 1 ] ].size();
   bool error_warn = !use_errors && ( f_errors[ selected[ 0 ] ].size() || f_errors[ selected[ 1 ] ].size() );
   if ( error_warn )
   {
      editor_msg( "dark red",
                  QString( us_tr( "Warning: no errors will be stored because %1 does not contain any error information" ) )
                  .arg( f_errors[ selected[ 0 ] ].size() ? selected[ 1 ] : selected[ 0 ] ) );
   }

   for ( unsigned int i = 0; i < f_qs[ selected[ 0 ] ].size(); i++ )
   {
      if ( f_qs[ selected[ 0 ] ][ i ] <= q_join )
      {
         q_string.push_back( f_qs_string[ selected[ 0 ] ][ i ] );
         q       .push_back( f_qs       [ selected[ 0 ] ][ i ] );
         I       .push_back( f_Is       [ selected[ 0 ] ][ i ] );
         if ( use_errors )
         {
            e       .push_back( f_errors   [ selected[ 0 ] ][ i ] );
         }
      }
   }

   for ( unsigned int i = 0; i < f_qs[ selected[ 1 ] ].size(); i++ )
   {
      if ( f_qs[ selected[ 1 ] ][ i ] > q_join )
      {
         q_string.push_back( f_qs_string[ selected[ 1 ] ][ i ] );
         q       .push_back( f_qs       [ selected[ 1 ] ][ i ] );
         I       .push_back( f_Is       [ selected[ 1 ] ][ i ] );
         if ( use_errors )
         {
            e       .push_back( f_errors   [ selected[ 1 ] ][ i ] );
         }
      }
   }

   map < QString, double > concs = current_concs( true );

   avg_conc = concs.count( selected[ 0 ] ) ? concs[ selected[ 0 ] ] : 0e0;
   avg_conc += concs.count( selected[ 1 ] ) ? concs[ selected[ 1 ] ] : 0e0;
   avg_conc /= 2e0;

   avg_psv = f_psv.count( selected[ 0 ] ) ? f_psv[ selected[ 0 ] ] : 0e0;
   avg_psv += f_psv.count( selected[ 1 ] ) ? f_psv[ selected[ 1 ] ] : 0e0;
   avg_psv /= 2e0;

   avg_I0se = f_I0se.count( selected[ 0 ] ) ? f_I0se[ selected[ 0 ] ] : 0e0;
   avg_I0se += f_I0se.count( selected[ 1 ] ) ? f_I0se[ selected[ 1 ] ] : 0e0;
   avg_I0se /= 2e0;

   QString basename = 
      QString( "%1-%2-join%3" )
      .arg( selected[ 0 ] )
      .arg( selected[ 1 ] )
      .arg( QString( "%1" ).arg( q_join ).replace( "." , "_" ) );

   QString use_basename = basename;

   unsigned int ext = 0;
   
   while ( f_qs.count( use_basename ) )
   {
      use_basename = QString( "%1-%2" ).arg( basename ).arg( ++ext );
   }

   lb_created_files->addItem( use_basename );
   lb_created_files->scrollToItem( lb_created_files->item( lb_created_files->count() - 1 ) );
   lb_files->addItem( use_basename );
   lb_files->scrollToItem( lb_files->item( lb_files->count() - 1 ) );

   created_files_not_saved[ use_basename ] = true;
   
   f_pos       [ use_basename ] = f_qs.size();
   f_qs_string [ use_basename ] = q_string;
   f_qs        [ use_basename ] = q;
   f_Is        [ use_basename ] = I;
   if ( use_errors )
   {
      f_errors    [ use_basename ] = e;
   }
   f_is_time   [ use_basename ] = false;
   f_conc      [ use_basename ] = avg_conc;
   f_psv       [ use_basename ] = avg_psv;
   f_I0se      [ use_basename ] = avg_I0se;

   {
      vector < double > tmp;
      f_gaussians  [ use_basename ] = tmp;
   }

   lb_files        ->clearSelection();
   lb_created_files->item( lb_created_files->count() - 1)->setSelected( true );
   show_created();

   editor_msg( "black", 
               QString( us_tr( "Created %1 as join of %2 and %3 at q %4" ) )
               .arg( use_basename )
               .arg( selected[ 0 ] )
               .arg( selected[ 1 ] )
               .arg( q_join ) );
   update_enables();
}

void US_Hydrodyn_Mals::plot_zoomed( const QRectF & /* rect */ )
{
   // qDebug() << "plot_zoomed event";
   //   TSO << QString( "zoomed: %1 %2 %3 %4\n" )
   // .arg( rect.x1() )
   // .arg( rect.x2() )
   // .arg( rect.y1() )
   // .arg( rect.y2() );
   if ( !running )
   {
      // TSO << "not running\n";
      update_enables();
   } else {
      // TSO << "is running, update_enables skipped\n";
   }
}

void US_Hydrodyn_Mals::plot_errors_zoomed( const QRectF & /* rect */ )
{
}

void US_Hydrodyn_Mals::zoom_info()
{
   if ( plot_dist_zoomer )
   {
#if QT_VERSION < 0x040000
      TSO << QString( "zoomrect: %1 %2 %3 %4\n" )
         .arg( plot_dist_zoomer->zoomRect().x1() )
         .arg( plot_dist_zoomer->zoomRect().x2() )
         .arg( plot_dist_zoomer->zoomRect().y1() )
         .arg( plot_dist_zoomer->zoomRect().y2() );
      TSO << QString( "zoombase: %1 %2 %3 %4\n" )
         .arg( plot_dist_zoomer->zoomBase().x1() )
         .arg( plot_dist_zoomer->zoomBase().x2() )
         .arg( plot_dist_zoomer->zoomBase().y1() )
         .arg( plot_dist_zoomer->zoomBase().y2() );
#else
      TSO << QString( "zoomrect: %1 %2 %3 %4\n" )
         .arg( plot_dist_zoomer->zoomRect().left() )
         .arg( plot_dist_zoomer->zoomRect().right() )
         .arg( plot_dist_zoomer->zoomRect().top() )
         .arg( plot_dist_zoomer->zoomRect().bottom() );
      TSO << QString( "zoombase: %1 %2 %3 %4\n" )
         .arg( plot_dist_zoomer->zoomBase().left() )
         .arg( plot_dist_zoomer->zoomBase().right() )
         .arg( plot_dist_zoomer->zoomBase().top() )
         .arg( plot_dist_zoomer->zoomBase().bottom() );
#endif
   } else {
      TSO << "no current zoomer\n";
   }
}

void US_Hydrodyn_Mals::plot_mouse( const QMouseEvent & /* me */ )
{
   // TSO << "mouse event\n";
   // zoom_info();
   if ( plot_dist_zoomer )
   {
      // TSO << QString( "is base %1\n" ).arg( plot_dist_zoomer->zoomBase() == 
      // plot_dist_zoomer->zoomRect() ? "yes" : "no" );
      update_enables();
   }
}

void US_Hydrodyn_Mals::select_vis()
{
#if QT_VERSION < 0x040000
   double zrx1  = plot_dist_zoomer->zoomRect().x1();
   double zrx2  = plot_dist_zoomer->zoomRect().x2();
   double zry1  = plot_dist_zoomer->zoomRect().y1();
   double zry2  = plot_dist_zoomer->zoomRect().y2();
#else
   double zrx1  = plot_dist_zoomer->zoomRect().left();
   double zrx2  = plot_dist_zoomer->zoomRect().right();
   double zry1  = plot_dist_zoomer->zoomRect().top();
   double zry2  = plot_dist_zoomer->zoomRect().bottom();
#endif
   // find curves within zoomRect & select only them
   map < QString, bool > selected_files;
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         QString this_file = lb_files->item( i )->text();
         if ( f_qs.count( this_file ) &&
              f_Is.count( this_file ) )
         {
            for ( unsigned int i = 0; i < f_qs[ this_file ].size(); i++ )
            {
               if ( f_qs[ this_file ][ i ] >= zrx1 &&
                    f_qs[ this_file ][ i ] <= zrx2 &&
                    f_Is[ this_file ][ i ] >= zry1 &&
                    f_Is[ this_file ][ i ] <= zry2 )
               {
                  selected_files[ this_file ] = true;
                  break;
               }
            }
         } 
      }
   }

   disable_updates = true;
   lb_files->clearSelection();
   bool did_current = false;
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( selected_files.count( lb_files->item( i )->text() ) )
      {
         lb_files->item( i)->setSelected( true );
         if ( !did_current )
         {
            lb_files->setCurrentItem( lb_files->item( i ) );
            did_current = true;
         }
      }
   }
   disable_updates = false;
   lb_files->scrollToItem( lb_files->currentItem() );
   update_files();
}

void US_Hydrodyn_Mals::remove_vis()
{
#if QT_VERSION < 0x040000
   double zrx1  = plot_dist_zoomer->zoomRect().x1();
   double zrx2  = plot_dist_zoomer->zoomRect().x2();
   double zry1  = plot_dist_zoomer->zoomRect().y1();
   double zry2  = plot_dist_zoomer->zoomRect().y2();
#else
   double zrx1  = plot_dist_zoomer->zoomRect().left();
   double zrx2  = plot_dist_zoomer->zoomRect().right();
   double zry1  = plot_dist_zoomer->zoomRect().top();
   double zry2  = plot_dist_zoomer->zoomRect().bottom();
#endif
   // find curves within zoomRect & select only them
   TSO << "select visible\n";
   QStringList selected_files;
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         QString this_file = lb_files->item( i )->text();
         if ( f_qs.count( this_file ) &&
              f_Is.count( this_file ) )
         {
            for ( unsigned int i = 0; i < f_qs[ this_file ].size(); i++ )
            {
               if ( f_qs[ this_file ][ i ] >= zrx1  &&
                    f_qs[ this_file ][ i ] <= zrx2  &&
                    f_Is[ this_file ][ i ] >= zry1  &&
                    f_Is[ this_file ][ i ] <= zry2 )
               {
                  selected_files << this_file;
                  break;
               }
            }
         } 
      }
   }

   clear_files( selected_files );
   update_files();
   if ( plot_dist_zoomer &&
        plot_dist_zoomer->zoomRectIndex() )
   {
      plot_dist_zoomer->zoom( -1 );
   }
}

void US_Hydrodyn_Mals::crop_left()
{
   // first make left visible,
   // of no left movement needed, then start cropping points
   
   // find selected curves & their left most position:
   bool all_lefts_visible = true;
   map < QString, bool > selected_files;

   double minx = 0e0;
   double maxx = 0e0;
   double miny = 0e0;
   double maxy = 0e0;

   bool first = true;

   unsigned show_pts = 5;
   // unsigned show_pts_min = show_pts - 3;

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         QString this_file = lb_files->item( i )->text();
         if ( f_qs.count( this_file ) &&
              f_Is.count( this_file ) &&
              f_qs[ this_file ].size() > show_pts - 1 &&
              f_Is[ this_file ].size() )
         {
            selected_files[ this_file ] = true;
            double this_minx = f_qs[ this_file ][ 0 ];
            double this_maxx = f_qs[ this_file ][ show_pts - 1 ];
            double this_miny = f_Is[ this_file ][ 0 ];
            double this_maxy = f_Is[ this_file ][ 0 ];

            for ( unsigned int j = 1; j < show_pts; j++ )
            {
               if ( this_miny > f_Is[ this_file ][ j ] )
               {
                  this_miny = f_Is[ this_file ][ j ];
               }
               if ( this_maxy < f_Is[ this_file ][ j ] )
               {
                  this_maxy = f_Is[ this_file ][ j ];
               }
            }

            if ( first )
            {
               first = false;
               minx = this_minx;
               maxx = this_maxx;
               miny = this_miny;
               maxy = this_maxy;
            } else {
               if ( minx > this_minx )
               {
                  minx = this_minx;
               }
               if ( maxx < this_maxx )
               {
                  maxx = this_maxx;
               }
               if ( miny > this_miny )
               {
                  miny = this_miny;
               }
               if ( maxy < this_maxy )
               {
                  maxy = this_maxy;
               }
            }
         } else {
            editor_msg( "red", QString( us_tr( "Crop left: curves need at least %1 points to crop" ) ).arg( show_pts ) );
            return;
         }            
      }
   }

   // is the rectangle contained?
   if ( 
#if QT_VERSION < 0x040000
       minx < plot_dist_zoomer->zoomRect().x1() ||
       maxx > plot_dist_zoomer->zoomRect().x2() ||
       miny < plot_dist_zoomer->zoomRect().y1() ||
       maxy > plot_dist_zoomer->zoomRect().y2() )
#else
       minx < plot_dist_zoomer->zoomRect().left()  ||
       maxx > plot_dist_zoomer->zoomRect().right() ||
       miny < plot_dist_zoomer->zoomRect().top()   ||
       maxy > plot_dist_zoomer->zoomRect().bottom() )
#endif
   {
      all_lefts_visible = false;
   }

   if ( !all_lefts_visible )
   {
      editor_msg( "black", us_tr( "Crop left: press again to crop one point" ) );
      // will our current zoom rectangle show all the points?
      // if so, simply move it
      double dx = maxx - minx;
      double dy = maxy - miny;

#if QT_VERSION < 0x040000
      double zdx = plot_dist_zoomer->zoomRect().x2() - plot_dist_zoomer->zoomRect().x1();
      double zdy = plot_dist_zoomer->zoomRect().y2() - plot_dist_zoomer->zoomRect().y1();
#else
      double zdx = plot_dist_zoomer->zoomRect().right()  - plot_dist_zoomer->zoomRect().left();
      double zdy = plot_dist_zoomer->zoomRect().bottom() - plot_dist_zoomer->zoomRect().top();
#endif
      if ( zdx > dx * 1.1 && zdy > dy * 1.1 )
      {
         // we can fit
         double newx = minx - .05 * dx;
         double newy = miny - .05 * dy;
         if ( newx < 0e0 )
         {
            newx = 0e0;
         }
         if ( newy < 0e0 )
         {
            newy = 0e0;
         }
         TSO << QString( "just move to %1 %2\n" ).arg( newx ).arg( newy );
         plot_dist_zoomer->moveTo( QPointF( newx, newy ) );
         return;
      }

      // ok, we are going to have to make a rectangle
      QRectF dr = plot_dist_zoomer->zoomRect();

      double newminx = minx - .05 * dx;
      double newminy = miny - .05 * dy;
      if ( newminx < 0e0 )
      {
         newminx = 0e0;
      }
      if ( newminy < 0e0 )
      {
         newminy = 0e0;
      }
#if QT_VERSION < 0x040000
      dr.setX1( newminx );
      dr.setY1( newminy );

      if ( zdx > dx * 1.1 )
      {
         dr.setX2( newminx + zdx );
      } else {         
         dr.setX2( newminx + dx * 1.1 );
      }
      if ( zdy > dy * 1.1 )
      {
         dr.setY2( newminy + zdy );
      } else {         
         dr.setY2( newminy + dy * 1.1 );
      }
#else
      dr.setLeft( newminx );
      dr.setTop ( newminy );

      if ( zdx > dx * 1.1 )
      {
         dr.setRight( newminx + zdx );
      } else {         
         dr.setRight( newminx + dx * 1.1 );
      }
      if ( zdy > dy * 1.1 )
      {
         dr.setBottom( newminy + zdy );
      } else {         
         dr.setBottom( newminy + dy * 1.1 );
      }
#endif

      plot_dist_zoomer->zoom( dr );
      return;
   }
   // remove the first point from each of and replot

   crop_undo_data cud;
   cud.is_left   = true;
   cud.is_common = false;

   for ( map < QString, bool >::iterator it = selected_files.begin();
         it != selected_files.end();
         it++ )
   {
      unsigned int org_len = f_qs[ it->first ].size();
      cud.files   .push_back( it->first );
      cud.q_string.push_back( f_qs_string[ it->first ][ 0 ] );
      cud.q       .push_back( f_qs       [ it->first ][ 0 ] );
      cud.I       .push_back( f_Is       [ it->first ][ 0 ] );
      if ( f_errors.count( it->first ) &&
           f_errors[ it->first ].size() )
      {
         cud.has_e   .push_back( true );
         cud.e       .push_back( f_errors   [ it->first ][ 0 ] );
      } else {
         cud.has_e   .push_back( false );
         cud.e       .push_back( 0 );
      }

      for ( unsigned int i = 1; i < f_qs[ it->first ].size(); i++ )
      {

         f_qs_string[ it->first ][ i - 1 ] = f_qs_string[ it->first ][ i ];
         f_qs       [ it->first ][ i - 1 ] = f_qs       [ it->first ][ i ];
         f_Is       [ it->first ][ i - 1 ] = f_Is       [ it->first ][ i ];
         if ( f_errors.count( it->first ) &&
              f_errors[ it->first ].size() )
         {
            f_errors[ it->first ][ i - 1 ] = f_errors[ it->first ][ i ];
         }
      }

      f_qs_string[ it->first ].resize( org_len - 1 );
      f_qs       [ it->first ].resize( org_len - 1 );
      f_Is       [ it->first ].resize( org_len - 1 );
      if ( f_errors.count( it->first ) &&
           f_errors[ it->first ].size() )
      {
         f_errors[ it->first ].resize( org_len - 1 );
      }
      to_created( it->first );
   }
   crop_undos.push_back( cud );
   editor_msg( "blue", us_tr( "Crop left: cropped 1 point" ) );

   update_files();
}

void US_Hydrodyn_Mals::crop_right()
{
   // find selected curves & their right most position:
   bool all_rights_visible = true;
   map < QString, bool > selected_files;

   double minx = 0e0;
   double maxx = 0e0;
   double miny = 0e0;
   double maxy = 0e0;

   bool first = true;

   unsigned show_pts = 5;
   // unsigned show_pts_min = show_pts - 3;

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         QString this_file = lb_files->item( i )->text();
         if ( f_qs.count( this_file ) &&
              f_Is.count( this_file ) &&
              f_qs[ this_file ].size() > show_pts - 1 &&
              f_Is[ this_file ].size() )
         {
            selected_files[ this_file ] = true;
            unsigned int size = f_qs[ this_file ].size();
            double this_minx = f_qs[ this_file ][ size - show_pts - 1 ];
            double this_maxx = f_qs[ this_file ][ size - 1 ];
            double this_miny = f_Is[ this_file ][ size - show_pts - 1 ];
            double this_maxy = f_Is[ this_file ][ size - show_pts - 1 ];

            for ( unsigned int j = size - show_pts; j < size; j++ )
            {
               if ( this_miny > f_Is[ this_file ][ j ] )
               {
                  this_miny = f_Is[ this_file ][ j ];
               }
               if ( this_maxy < f_Is[ this_file ][ j ] )
               {
                  this_maxy = f_Is[ this_file ][ j ];
               }
            }

            if ( first )
            {
               first = false;
               minx = this_minx;
               maxx = this_maxx;
               miny = this_miny;
               maxy = this_maxy;
            } else {
               if ( minx > this_minx )
               {
                  minx = this_minx;
               }
               if ( maxx < this_maxx )
               {
                  maxx = this_maxx;
               }
               if ( miny > this_miny )
               {
                  miny = this_miny;
               }
               if ( maxy < this_maxy )
               {
                  maxy = this_maxy;
               }
            }
         } else {
            editor_msg( "red", QString( us_tr( "Crop right: curves need at least %1 points to crop" ) ).arg( show_pts ) );
            return;
         }            
      }
   }

   // is the rectangle contained?
   if ( 
#if QT_VERSION < 0x040000
       minx < plot_dist_zoomer->zoomRect().x1() ||
       maxx > plot_dist_zoomer->zoomRect().x2() ||
       miny < plot_dist_zoomer->zoomRect().y1() ||
       maxy > plot_dist_zoomer->zoomRect().y2() )
#else
       minx < plot_dist_zoomer->zoomRect().left()  ||
       maxx > plot_dist_zoomer->zoomRect().right() ||
       miny < plot_dist_zoomer->zoomRect().top()   ||
       maxy > plot_dist_zoomer->zoomRect().bottom() )
#endif
   {
      all_rights_visible = false;
   }

   if ( !all_rights_visible )
   {
      editor_msg( "black", us_tr( "Crop right: press again to crop one point" ) );
      // will our current zoom rectangle show all the points?
      // if so, simply move it
      double dx = maxx - minx;
      double dy = maxy - miny;

#if QT_VERSION < 0x040000
      double zdx = plot_dist_zoomer->zoomRect().x2() - plot_dist_zoomer->zoomRect().x1();
      double zdy = plot_dist_zoomer->zoomRect().y2() - plot_dist_zoomer->zoomRect().y1();
#else
      double zdx = plot_dist_zoomer->zoomRect().right()  - plot_dist_zoomer->zoomRect().left();
      double zdy = plot_dist_zoomer->zoomRect().bottom() - plot_dist_zoomer->zoomRect().top();
#endif
      if ( zdx > dx * 1.1 && zdy > dy * 1.1 )
      {
         // we can fit
         double newx = minx - .05 * dx;
         double newy = miny - .05 * dy;
         if ( newx < 0e0 )
         {
            newx = 0e0;
         }
         if ( newy < 0e0 )
         {
            newy = 0e0;
         }
         TSO << QString( "just move to %1 %2\n" ).arg( newx ).arg( newy );
         plot_dist_zoomer->moveTo( QPointF( newx, newy ) );
         return;
      }

      // ok, we are going to have to make a rectangle
      QRectF dr = plot_dist_zoomer->zoomRect();

      double newminx = minx - .05 * dx;
      double newminy = miny - .05 * dy;
      if ( newminx < 0e0 )
      {
         newminx = 0e0;
      }
      if ( newminy < 0e0 )
      {
         newminy = 0e0;
      }
#if QT_VERSION < 0x040000
      dr.setX1( newminx );
      dr.setY1( newminy );

      if ( zdx > dx * 1.1 )
      {
         dr.setX2( newminx + zdx );
      } else {         
         dr.setX2( newminx + dx * 1.1 );
      }
      if ( zdy > dy * 1.1 )
      {
         dr.setY2( newminy + zdy );
      } else {         
         dr.setY2( newminy + dy * 1.1 );
      }
#else
      dr.setLeft( newminx );
      dr.setTop ( newminy );

      if ( zdx > dx * 1.1 )
      {
         dr.setRight( newminx + zdx );
      } else {         
         dr.setRight( newminx + dx * 1.1 );
      }
      if ( zdy > dy * 1.1 )
      {
         dr.setBottom( newminy + zdy );
      } else {         
         dr.setBottom( newminy + dy * 1.1 );
      }
#endif

      plot_dist_zoomer->zoom( dr );
      return;
   }
   // remove the first point from each of and replot

   crop_undo_data cud;
   cud.is_left   = false;
   cud.is_common = false;

   for ( map < QString, bool >::iterator it = selected_files.begin();
         it != selected_files.end();
         it++ )
   {
      unsigned int org_len = f_qs[ it->first ].size();
      cud.files   .push_back( it->first );
      cud.q_string.push_back( f_qs_string[ it->first ][ org_len - 1 ] );
      cud.q       .push_back( f_qs       [ it->first ][ org_len - 1 ] );
      cud.I       .push_back( f_Is       [ it->first ][ org_len - 1 ] );
      if ( f_errors.count( it->first ) &&
           f_errors[ it->first ].size() )
      {
         cud.has_e   .push_back( true );
         cud.e       .push_back( f_errors   [ it->first ][ org_len - 1 ] );
      } else {
         cud.has_e   .push_back( false );
         cud.e       .push_back( 0 );
      }

      f_qs_string[ it->first ].pop_back();
      f_qs       [ it->first ].pop_back();
      f_Is       [ it->first ].pop_back();
      if ( f_errors.count( it->first ) &&
           f_errors[ it->first ].size() )
      {
         f_errors[ it->first ].pop_back();
      }
      to_created( it->first );
   }
   crop_undos.push_back( cud );
   editor_msg( "blue", us_tr( "Crop right: cropped 1 point" ) );

   update_files();
}

void US_Hydrodyn_Mals::crop_undo()
{
   if ( !crop_undos.size() )
   {
      return;
   }

   map < QString, bool > current_files;

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      current_files[ lb_files->item( i )->text() ] = true;
   }

   crop_undo_data cud = crop_undos.back();
   crop_undos.pop_back();

   if ( cud.is_common )
   {
      // full restore
      for ( map < QString, vector < double > >::iterator it = cud.f_qs.begin();
            it != cud.f_qs.end();
            it++ )
      {
         if ( !f_qs.count( it->first ) )
         {
            editor_msg( "red", QString( us_tr( "Error: can not undo crop to missing file %1" ) ).arg( it->first ) );
         } else {
            f_qs_string[ it->first ] = cud.f_qs_string[ it->first ];
            f_qs       [ it->first ] = cud.f_qs       [ it->first ];
            f_Is       [ it->first ] = cud.f_Is       [ it->first ];
            if ( cud.f_errors.count( it->first ) && cud.f_errors[ it->first ].size() )
            {
               f_errors   [ it->first ] = cud.f_errors   [ it->first ];
            } else {
               if ( f_errors.count( it->first ) && f_errors[ it->first ].size() )
               {
                  editor_msg( "dark red", QString( us_tr( "Warning: file %1 had no errors before crop common but somehow has errors now (?), removing them" ) ).arg( it->first ) );
                  f_errors.erase( it->first );
               }
            }
         }
      }
   } else {
      for ( unsigned int i = 0; i < cud.files.size(); i++ )
      {
         if ( !current_files.count( cud.files[ i ] ) )
         {
            editor_msg( "red", QString( us_tr( "Error: can not undo crop to missing file %1" ) ).arg( cud.files[ i ] ) );
         } else {
            if ( cud.is_left )
            {
               unsigned int org_len = f_qs[ cud.files[ i ] ].size();
               f_qs_string[ cud.files[ i ] ].resize( org_len + 1 );
               f_qs       [ cud.files[ i ] ].resize( org_len + 1 );
               f_Is       [ cud.files[ i ] ].resize( org_len + 1 );
               if ( f_errors.count( cud.files[ i ] ) &&
                    f_errors[ cud.files[ i ] ].size() )
               {
                  f_errors[ cud.files[ i ] ].resize( org_len + 1 );
               }
               
               for ( int j = org_len - 1; j >= 0; j-- )
               {
                  f_qs_string[ cud.files[ i ] ][ j + 1 ] = f_qs_string[ cud.files[ i ] ][ j ];
                  f_qs       [ cud.files[ i ] ][ j + 1 ] = f_qs       [ cud.files[ i ] ][ j ];
                  f_Is       [ cud.files[ i ] ][ j + 1 ] = f_Is       [ cud.files[ i ] ][ j ];
                  if ( f_errors.count( cud.files[ i ] ) &&
                       f_errors[ cud.files[ i ] ].size() )
                  {
                     f_errors[ cud.files[ i ] ][ j + 1 ] = f_errors[ cud.files[ i ] ][ j ];
                  }
               }
               
               f_qs_string[ cud.files[ i ] ][ 0 ] = cud.q_string[ i ];
               f_qs       [ cud.files[ i ] ][ 0 ] = cud.q       [ i ];
               f_Is       [ cud.files[ i ] ][ 0 ] = cud.I       [ i ];
               if ( f_errors.count( cud.files[ i ] ) &&
                    f_errors[ cud.files[ i ] ].size() )
               {
                  f_errors[ cud.files[ i ] ][ 0 ] = cud.e       [ i ];
               }
            } else {
               f_qs_string[ cud.files[ i ] ].push_back( cud.q_string[ i ] );
               f_qs       [ cud.files[ i ] ].push_back( cud.q       [ i ] );
               f_Is       [ cud.files[ i ] ].push_back( cud.I       [ i ] );
               if ( f_errors.count( cud.files[ i ] ) &&
                    f_errors[ cud.files[ i ] ].size() )
               {
                  f_errors[ cud.files[ i ] ].push_back( cud.e       [ i ] );
               }
            }
         }
      }
      editor_msg( "blue", us_tr( "Crop undo: restored 1 point" ) );
   }
   update_files();
}


void US_Hydrodyn_Mals::view()
{
   int dsp_count = 0;

   if ( all_selected_files().count() > 5 ) {
         switch ( QMessageBox::question(this, 
                                        this->windowTitle() + us_tr(": View selected" ),
                                        QString( us_tr( "There are %1 files selected\n"
                                                        "Do you really want to open up %2 windows to view them all?" ) )
                                        .arg( all_selected_files().count() ),
                                        us_tr( "&Yes, view them all" ), 
                                        us_tr( "&Cancel" ),
                                        QString(),
                                        1, // Stop == button 0
                                        1 // Escape == button 0
                                        ) )
         {
         case 0 : // Yes
            break;
         case 1 : // no
         default : // no
            return;
            break;
         }  
   }

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         QString file = lb_files->item( i )->text();

         // if ( created_files_not_saved.count( file ) ) {
         //    QTextStream( stdout ) << QString( "::view %1 created_not_saved true\n").arg( file );
         // } else {
         //    QTextStream( stdout ) << QString( "::view %1 created_not_saved false\n").arg( file );
         // }
            
         QString text;

         {
            update_csv_conc();
            map < QString, double > concs = current_concs();

            QString use_conc;
            if ( concs.count( file ) && concs[ file ] != 0e0 )
            {
               use_conc = QString( " Conc:%1 [mg/mL]" ).arg( concs[ file ] );
            } else {
               if ( f_conc.count( file ) && f_conc[ file ] != 0e0 ) 
               {
                  use_conc = QString( " Conc:%1" ).arg( f_conc[ file ] );
                  if ( f_conc_units.count( file )
                       && !f_conc_units[ file ].isEmpty() ) {
                     use_conc += QString( " [%1]" ).arg( f_conc_units[ file ] );
                  }
               }
            }

            {
               QString units = " q units:1/A";
               if ( file.contains( "_Rt_q" ) ) {
                  units += " R(t) units:1/cm";
               }
               if ( file.contains( "_Ihasht_q" ) ) {
                  units += " I#(t) units:g^2/(cm^3*mol)";
               }
               if ( file.contains( "_Ihashq_" ) ) {
                  units += " I#(q) units:g^2/(cm^3*mol)";
               }
               if ( file.contains( "_Istarq_" ) ) {
                  units += " I*(q) units:g/mol";
               }
               if ( file.contains( "_Istart_" ) ) {
                  units += " I*(t) units:g/mol";
               }
               if ( conc_files.count( file ) ) {
                  units = "";
               }

               // text += QString( windowTitle() + us_tr( " %1data: %2%3%4%5%6%7%8%9%10%11%12%13%14\n" ) )
               //    .arg( ( f_is_time.count( file ) && f_is_time[ file ] ? "Frame " : "" ) )
               //    .arg( file )
               //    .arg( units )
               //    .arg( f_psv.count( file ) && f_psv[ file ] ? QString( " PSV:%1 [mL/g]"  ).arg( f_psv [ file ] ) : QString( "" ) )
               //    // .arg( f_I0se.count( file ) ? QString( " I0se:%1" ).arg( f_I0se[ file ] ) : QString( "" ) )
               //    .arg( use_conc ) // f_conc.count( file ) ? QString( " Conc:%1" ).arg( f_conc[ file ] ) : QString( "" ) )
               //    .arg( file.contains( "_Istarq_" ) && f_extc.count( file ) ? QString( " ExtC:%1 [mL/(mg*cm)]" ).arg( f_extc[ file ] ) : QString( "" ) )
               //    .arg( f_time.count( file ) ? QString( " Time:%1" ).arg( f_time[ file ] ) : QString( "" ) )
               //    .arg( !f_extc.count( file ) && f_g_dndc.count( file ) ? QString( " Global_dndc:%1 [ml/g]" ).arg( f_g_dndc[ file ] ) : QString( "" ) )
               //    .arg( f_extc.count( file ) && f_dndc.count( file ) ? QString( " dndc:%1 [ml/g]" ).arg( f_dndc[ file ] ) : QString( "" ) )
               //    .arg( mals_angles.loaded_filename.isEmpty() ? QString("") : QString( " Angles loaded from:%1" ).arg( mals_angles.loaded_filename ) )
               //    .arg( f_ri_corr.count( file ) ? QString( " RI-Corr scatt. angle:%1"  ).arg( f_ri_corr[ file ] ) : QString( "" ) )
               //    .arg( f_ri_corrs.count( file ) ? QString( " RI-Corr scatt. angles:%1"  ).arg( f_ri_corrs[ file ] ) : QString( "" ) )
               //    .arg( f_ref_index.count( file ) ? QString( " Solvent refractive index:%1"  ).arg( f_ref_index[ file ] ) : QString( "" ) )
               //    .arg( f_header.count( file ) ? f_header[ file ] : QString( "" ) )
               //    ;

               text += QString( windowTitle() + us_tr( " %1data: %2%3%4%5%6%7%8%9%10%11%12%13%14%15%16%17%18%19%20%21%22%23%24%25%26%27\n" ) )
                  .arg( ( f_is_time.count( file ) && f_is_time[ file ] ? "Frame " : "" ) )
                  .arg( file )
                  .arg( units )
                  .arg( f_psv .count( file ) ? QString( " PSV:%1 [mL/g]"  ).arg( f_psv [ file ] ) : QString( "" ) )
                  .arg( f_I0se.count( file ) ? QString( " I0se:%1 [a.u.]" ).arg( f_I0se[ file ] ) : QString( "" ) )
                  .arg( f_I0st.count( file ) ? QString( " I0st:%1 [a.u.]" ).arg( f_I0st[ file ] ) : QString( "" ) )
                  .arg( use_conc ) // f_conc.count( file ) ? QString( " Conc:%1" ).arg( f_conc[ file ] ) : QString( "" ) )
                  .arg( file.contains( "_Istarq_" ) && f_extc.count( file ) ? QString( " ExtC:%1 [mL/(mg*cm)]" ).arg( f_extc[ file ] ) : QString( "" ) )
                  .arg( !file.contains( "_Istarq_" ) && f_extc.count( file ) ? QString( " ExtC_or_DRIinc:%1" ).arg( f_extc[ file ] ) : QString( "" ) )
                  .arg( f_time.count( file ) ? QString( " Time:%1" ).arg( f_time[ file ] ) : QString( "" ) )
                  .arg( !f_extc.count( file ) && f_g_dndc.count( file ) ? QString( " Global_dndc:%1 [ml/g]" ).arg( f_g_dndc[ file ] ) : QString( "" ) )
                  .arg( f_extc.count( file ) && f_dndc.count( file ) ? QString( " dndc:%1 [ml/g]" ).arg( f_dndc[ file ] ) : QString( "" ) )
                  .arg( f_diffusion_len.count( file ) ? QString( " Diffusion Length: %1 [cm]" ).arg( f_diffusion_len[ file ] ) : QString("") )
                  .arg( f_e_nucleon_ratio.count( file ) ? QString( " Electron/nucleon ratio Z/A: %1" ).arg( f_e_nucleon_ratio[ file ] ) : QString("") )
                  .arg( f_nucleon_mass.count( file ) ? QString( " Nucleon mass: %1 [g]" ).arg( f_nucleon_mass[ file ] ) : QString("") )
                  .arg( f_solvent_e_dens.count( file ) ? QString( " Solvent e density: %1 [e A^-3]" ).arg( f_solvent_e_dens[ file ] ) : QString("") )
                  .arg( f_header.count( file ) ? f_header[ file ] : QString( "" ) )
                  .arg( mals_angles.loaded_filename.isEmpty() ? QString("") : QString( " Angles loaded from:%1" ).arg( mals_angles.loaded_filename ) )
                  .arg( f_ri_corr.count( file ) ? QString( " RI-Corr scatt. angle:%1"  ).arg( f_ri_corr[ file ] ) : QString( "" ) )
                  .arg( f_ri_corrs.count( file ) ? QString( " RI-Corr scatt. angles:%1"  ).arg( f_ri_corrs[ file ] ) : QString( "" ) )
                  .arg( f_ref_index.count( file ) ? QString( " Solvent refractive index:%1"  ).arg( f_ref_index[ file ] ) : QString( "" ) )
                  .arg( f_header.count( file ) ? f_header[ file ] : QString( "" ) )
                  .arg( f_fit_curve.count( file ) ? QString( " Fit curve:%1" ).arg( f_fit_curve[ file ] ) : QString( "" ) )
                  .arg( f_fit_method.count( file ) ? QString( " Fit method:%1" ).arg( f_fit_method[ file ] ) : QString( "" ) )
                  .arg( f_fit_q_ranges.count( file ) ? QString( " Fit q ranges:%1" ).arg( f_fit_q_ranges[ file ] ) : QString( "" ) )
                  .arg( f_fit_chi2.count( file ) ? QString( " Fit chi^2:%1" ).arg( f_fit_chi2[ file ] ) : QString( "" ) )
                  .arg( f_fit_sd_scale.count( file ) ? QString( " Fit MALS SD mult:%1" ).arg( f_fit_sd_scale[ file ] ) : QString( "" ) )
                  ;
            }
         }
         // text += QString( us_tr( "US-SOMO Hplc output: %1\n" ) ).arg( file );

         bool use_errors = ( f_errors.count( file ) && 
                             f_errors[ file ].size() > 0 );
         
         if ( f_is_time.count( file ) && f_is_time[ file ] )
         {
            if ( conc_files.count( file ) ) {
               if ( use_errors )
               {
                  text += "t                 \tA(t)         \tsd\n";
               } else {
                  text += "t                 \tA(t)\n";
               }
            } else {
               if ( use_errors )
               {
                  text += "t                 \tI(t)         \tsd\n";
               } else {
                  text += "t                 \tI(t)\n";
               }
            }
         } else {
            if ( use_errors )
            {
               text += "q                 \tI(q)         \tsd\n";
            } else {
               text += "q                 \tI(q)\n";
            }
         }

         for ( int i = 0; i < (int)f_qs[ file ].size(); i++ )
         {
            if ( use_errors &&
                 (int)f_errors[ file ].size() > i )
            {
               text += QString("").sprintf( "%-18s\t%.6e\t%.6e\n",
                                          f_qs_string[ file ][ i ].toLatin1().data(),
                                          f_Is       [ file ][ i ],
                                          f_errors   [ file ][ i ] );
            } else {
               text += QString("").sprintf( "%-18s\t%.6e\n",
                                          f_qs_string[ file ][ i ].toLatin1().data(),
                                          f_Is       [ file ][ i ] );
            }
         }

         US3i_Editor * edit = new US3i_Editor( US3i_Editor::DEFAULT, true, QString(), 0 );
         edit->setWindowTitle( file );
         edit->resize( 685, 700 );
         // QPoint p = g.global_position();
         // edit->move( p.x() + 30, p.y() + 30 );
         ++dsp_count;
         edit->move( this->pos().x() + 10 + 7 * dsp_count, this->pos().y() + 10 + 7 * dsp_count );
         edit->e->setFont( QFont( "monospace",
                                  US3i_GuiSettings::fontSize() ) );
         edit->e->setText( text );
         edit->show();
      }
   }
}

bool US_Hydrodyn_Mals::is_nonzero_vector( vector < double > &v )
{
   bool non_zero = v.size() > 0;
   for ( int i = 0; i < (int)v.size(); i++ )
   {
      if ( v[ i ] == 0e0 )
      {
         non_zero = false;
         break;
      }
   }
   return non_zero;
}

bool US_Hydrodyn_Mals::is_zero_vector( vector < double > &v )
{
   bool is_zero = true;
   for ( int i = 0; i < (int)v.size(); i++ )
   {
      if ( v[ i ] != 0e0 )
      {
         is_zero = false;
         break;
      }
   }
   return is_zero;
}

void US_Hydrodyn_Mals::to_created( QString file )
{
   bool in_created = false;
   for ( int i = 0; i < (int)lb_created_files->count(); i++ )
   {
      if ( file == lb_created_files->item( i )->text() )
      {
         created_files_not_saved[ file ] = true;
         in_created = true;
      }
   }

   if ( !in_created )
   {
      lb_created_files->addItem( file );
      lb_created_files->scrollToItem( lb_created_files->item( lb_created_files->count() - 1 ) );
      created_files_not_saved[ file ] = true;
   }
}

void US_Hydrodyn_Mals::crop_vis()
{
   // find curves within zoomRect & select only them
   double minx = plot_dist_zoomer->zoomRect().left();
   double maxx = plot_dist_zoomer->zoomRect().right();
   double miny = plot_dist_zoomer->zoomRect().top();
   double maxy = plot_dist_zoomer->zoomRect().bottom();

   map < QString, bool > selected_files;
   QStringList qsl_selected_files;

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         QString this_file = lb_files->item( i )->text();
         if ( f_qs.count( this_file ) &&
              f_Is.count( this_file ) )
         {
            for ( unsigned int i = 0; i < f_qs[ this_file ].size(); i++ )
            {
               if ( f_qs[ this_file ][ i ] >= minx &&
                    f_qs[ this_file ][ i ] <= maxx &&
                    f_Is[ this_file ][ i ] >= miny &&
                    f_Is[ this_file ][ i ] <= maxy )
               {
                  selected_files[ this_file ] = true;
                  qsl_selected_files << this_file;
                  break;
               }
            }
         } 
      }
   }

   if ( !selected_files.size() )
   {
      editor_msg( "red", us_tr( "Crop visible: The current visible plot is empty" ) );
      return;
   }

   // make sure we don't leave a gap
   map < QString, bool > crop_left_side;

   bool all_left_side = true;

   for ( map < QString, bool >::iterator it = selected_files.begin();
         it != selected_files.end();
         it++ )
   {
      if ( f_qs[ it->first ][ 0 ]   < minx &&
           f_qs[ it->first ].back() > maxx )
      {
         editor_msg( "red", us_tr( "Crop visible: error: you can not crop out the middle of a curve" ) );
         return;
      }
      if ( f_qs[ it->first ][ 0 ]   < minx )
      {
         crop_left_side[ it->first ] = false;
         all_left_side               = false;
      }
      if ( f_qs[ it->first ].back() > maxx )
      {
         crop_left_side[ it->first ] = true;
      }
   }

   {
      map < QString, bool >::iterator it = crop_left_side.begin();
      bool last_crop = it->second;
      it++;
      for ( ; it!= crop_left_side.end(); it++ )
      {
         if ( it->second != last_crop )
         {
            editor_msg( "dark red", us_tr( "Crop visible: warning: you are cropping the left of some curves and the right of others. " ) );
            break;
         }
      }
   }
   
   editor_msg( "black",
               QString( us_tr( "Crop visible:\n"
                            "Cropping out range of (%1:%2)\n" ) )
               .arg( minx )
               .arg( maxx ) );

   crop_undo_data cud;
   cud.is_left   = false;
   cud.is_common = true;

   bool files_are_time_left_crop = all_left_side && compatible_files( qsl_selected_files ) && type_files( qsl_selected_files );
   bool timeshift_to_zero        = false;

   if ( files_are_time_left_crop ) {
      switch ( QMessageBox::question(this, 
                                     windowTitle() + us_tr( " : Crop Vis" )
                                     ,us_tr( 
                                            "Timeshift all data to zero after cropping?"
                                             )
                                     ) )
      {
      case QMessageBox::Yes : 
         timeshift_to_zero = true;
         break;
      default: 
         break;
      }
   }
      
   double timeshift_offset = DBL_MAX;

   if ( timeshift_to_zero ) {
      // need to find minimum left value to become new zero
      for ( auto it = selected_files.begin();
            it != selected_files.end();
            it++ ) {
         for ( unsigned int i = 0; i < f_qs[ it->first ].size(); i++ ) {
            if ( f_qs[ it->first ][ i ] < minx ||
                 f_qs[ it->first ][ i ] > maxx ) {
               if ( timeshift_offset > f_qs[ it->first ][ i ] ) {
                  timeshift_offset = f_qs[ it->first ][ i ];
               }
               break;
            }
         }
      }

      if ( timeshift_offset == DBL_MAX ) {
         timeshift_to_zero = false;
         QMessageBox::warning( this, 
                               windowTitle() + us_tr( " : Crop Vis" ),
                               us_tr(
                                     "Could not find a minimum value after crop\n"
                                     "Timeshift disabled"
                                     )
                               );
      } else {
         QMessageBox::information( this, 
                                   windowTitle() + us_tr( " : Crop Vis" ),
                                   QString( 
                                           us_tr(
                                                 "Data will be timeshifted by %1 after cropping"
                                                 )
                                            )
                                   .arg( timeshift_offset )
                                   );
      }
   }
      
   for ( map < QString, bool >::iterator it = selected_files.begin();
         it != selected_files.end();
         it++ ) {
      // save undo data
      cud.f_qs_string[ it->first ] = f_qs_string[ it->first ];
      cud.f_qs       [ it->first ] = f_qs       [ it->first ];
      cud.f_Is       [ it->first ] = f_Is       [ it->first ];
      if ( f_errors.count( it->first ) &&
           f_errors[ it->first ].size() )
      {
         cud.f_errors   [ it->first ] = f_errors   [ it->first ];
      }

      vector < QString > new_q_string;
      vector < double  > new_q;
      vector < double  > new_I;
      vector < double  > new_e;

      if ( timeshift_to_zero ) {
         for ( unsigned int i = 0; i < f_qs[ it->first ].size(); i++ ) {
            if ( f_qs[ it->first ][ i ] < minx ||
                 f_qs[ it->first ][ i ] > maxx ) {
               new_q       .push_back( f_qs       [ it->first ][ i ] - timeshift_offset );
               new_q_string.push_back( QString( "%1" ).arg( new_q.back() ) );
               new_I       .push_back( f_Is       [ it->first ][ i ] );

               if ( f_errors.count( it->first ) &&
                    f_errors[ it->first ].size() ) {
                  new_e       .push_back( f_errors   [ it->first ][ i ] );
               }
            }
         }
      } else {
         for ( unsigned int i = 0; i < f_qs[ it->first ].size(); i++ ) {
            if ( f_qs[ it->first ][ i ] < minx ||
                 f_qs[ it->first ][ i ] > maxx ) {
               new_q_string.push_back( f_qs_string[ it->first ][ i ] );
               new_q       .push_back( f_qs       [ it->first ][ i ] );
               new_I       .push_back( f_Is       [ it->first ][ i ] );

               if ( f_errors.count( it->first ) &&
                    f_errors[ it->first ].size() ) {
                  new_e       .push_back( f_errors   [ it->first ][ i ] );
               }
            }
         }
      }

      f_qs_string[ it->first ] = new_q_string;
      f_qs       [ it->first ] = new_q;
      f_Is       [ it->first ] = new_I;
      if ( f_errors.count( it->first ) &&
           f_errors[ it->first ].size() )
      {
         f_errors[ it->first ] = new_e;
      }
      to_created( it->first );
   }
   crop_undos.push_back( cud );
   editor_msg( "blue", us_tr( "Crop visible: done" ) );

   update_files();

   if ( plot_dist_zoomer &&
        plot_dist_zoomer->zoomRectIndex() )
   {
      plot_dist_zoomer->zoom( -1 );
   }
}

void US_Hydrodyn_Mals::crop_to_vis()
{
   // find curves within zoomRect & select only them
   double minx = plot_dist_zoomer->zoomRect().left();
   double maxx = plot_dist_zoomer->zoomRect().right();
   double miny = plot_dist_zoomer->zoomRect().top();
   double maxy = plot_dist_zoomer->zoomRect().bottom();

   map < QString, bool > selected_files;

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         QString this_file = lb_files->item( i )->text();
         if ( f_qs.count( this_file ) &&
              f_Is.count( this_file ) )
         {
            for ( unsigned int i = 0; i < f_qs[ this_file ].size(); i++ )
            {
               if ( f_qs[ this_file ][ i ] >= minx &&
                    f_qs[ this_file ][ i ] <= maxx &&
                    f_Is[ this_file ][ i ] >= miny &&
                    f_Is[ this_file ][ i ] <= maxy )
               {
                  selected_files[ this_file ] = true;
                  break;
               }
            }
         } 
      }
   }

   if ( !selected_files.size() )
   {
      editor_msg( "red", us_tr( "Crop to visible: The current visible plot is empty" ) );
      return;
   }
   
   editor_msg( "black",
               QString( us_tr( "Crop to visible:\n"
                            "Cropping to a range of (%1:%2)\n" ) )
               .arg( minx )
               .arg( maxx ) );

   crop_undo_data cud;
   cud.is_left   = false;
   cud.is_common = true;

   for ( map < QString, bool >::iterator it = selected_files.begin();
         it != selected_files.end();
         it++ )
   {
      // save undo data
      cud.f_qs_string[ it->first ] = f_qs_string[ it->first ];
      cud.f_qs       [ it->first ] = f_qs       [ it->first ];
      cud.f_Is       [ it->first ] = f_Is       [ it->first ];
      if ( f_errors.count( it->first ) &&
           f_errors[ it->first ].size() )
      {
         cud.f_errors   [ it->first ] = f_errors   [ it->first ];
      }

      vector < QString > new_q_string;
      vector < double  > new_q;
      vector < double  > new_I;
      vector < double  > new_e;

      for ( unsigned int i = 0; i < f_qs[ it->first ].size(); i++ )
      {
         if ( f_qs[ it->first ][ i ] >= minx &&
              f_qs[ it->first ][ i ] <= maxx )
         {
            new_q_string.push_back( f_qs_string[ it->first ][ i ] );
            new_q       .push_back( f_qs       [ it->first ][ i ] );
            new_I       .push_back( f_Is       [ it->first ][ i ] );

            if ( f_errors.count( it->first ) &&
                 f_errors[ it->first ].size() )
            {
               new_e       .push_back( f_errors   [ it->first ][ i ] );
            }
         }
      }

      f_qs_string[ it->first ] = new_q_string;
      f_qs       [ it->first ] = new_q;
      f_Is       [ it->first ] = new_I;
      if ( f_errors.count( it->first ) &&
           f_errors[ it->first ].size() )
      {
         f_errors[ it->first ] = new_e;
      }
      to_created( it->first );
   }
   crop_undos.push_back( cud );
   editor_msg( "blue", us_tr( "Crop to visible: done" ) );

   update_files();

   if ( plot_dist_zoomer &&
        plot_dist_zoomer->zoomRectIndex() )
   {
      plot_dist_zoomer->zoom( -1 );
   }
}


void US_Hydrodyn_Mals::legend()
{
   US_Plot_Zoom upz ( plot_dist, plot_dist_zoomer );
   legend_vis = !legend_vis;
   legend_set();
   upz.restore( !suppress_replot );
}

void US_Hydrodyn_Mals::legend_set()
{
#if QT_VERSION >= 0x040000
# if QT_VERSION >= 0x050000
   if ( legend_vis ) {
      QwtLegend* legend_saxs = new QwtLegend;
      // legend_saxs->setFrameStyle( QFrame::Box | QFrame::Sunken );
      plot_dist->insertLegend( legend_saxs, QwtPlot::BottomLegend );
   } else {      
      plot_dist->insertLegend( NULL );
   }
# else
   QwtPlotItemList ilist = plot_dist->itemList();
   for ( int ii = 0; ii < ilist.size(); ii++ )
   {
      QwtPlotItem* plitem = ilist[ ii ];
      if ( plitem->rtti() != QwtPlotItem::Rtti_PlotCurve )
         continue;
      plitem->setItemAttribute( QwtPlotItem::Legend, legend_vis );
   }
   plot_dist->legend()->setVisible( legend_vis );
# endif
#endif
}

void US_Hydrodyn_Mals::similar_files()
{
   vector < QString > selected;
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         selected.push_back( lb_files->item( i )->text() );
      }
   }

   if ( selected.size() != 1 )
   {
      return;
   }

   if ( !f_name.count( selected[ 0 ] ) )
   {
      editor_msg( 
                 "red", 
                 QString( us_tr( "Error: count not find disk reference for %1" ) )
                 .arg( selected[ 0 ] ) );
      return;
   }

   disable_all();

   QString similar = QFileInfo( f_name[ selected[ 0 ] ] ).fileName();
   QString dir     = QFileInfo( f_name[ selected[ 0 ] ] ).path();
   QString match   = similar;
   match.replace( QRegExp( "\\d{2,}" ), "\\d+" );

   TSO << QString( "select to match <%1> in directory <%2> using regexp <%3>\n" )
      .arg( similar )
      .arg( dir )
      .arg( match )
      .toLatin1().data();
   // go to that directory and get file list
   // turn basename into regexp \\d{2,} into \\d+
   // load unloaded files found with match
   QDir::setCurrent( dir );
   if ( !cb_lock_dir->isChecked() )
   {
      le_dir->setText( QDir::currentPath() );
   }
   QDir qd;
   add_files( qd.entryList( QStringList() << "*" ).filter( QRegExp( match ) ) );
}

void US_Hydrodyn_Mals::regex_load()
{
   // make list of le_dir all files
   // filter with regex for each # in list
   QString dir     = le_dir->text();

   QDir::setCurrent( dir );
   le_dir->setText( QDir::currentPath() );
   QDir qd;


   // QStringList regexs = (le_regex->text().split( QRegExp( "\\s+" ) , Qt::SkipEmptyParts )      );
   // QStringList args   = (le_regex_args->text().split( QRegExp( "\\s+" ) , Qt::SkipEmptyParts ) );
   QStringList regexs;
   QStringList args;

   {
      QString qs = le_regex->text();
      regexs = (qs ).split( QRegExp( "\\s+" ) , Qt::SkipEmptyParts );
      qs = le_regex_args->text();
      args   = (qs ).split( QRegExp( "\\s+" ) , Qt::SkipEmptyParts );
   }

   for ( int i = 0; i < (int)args.size(); i++ )
   {
      for ( int j = 0; j < (int)regexs.size(); j++ )
      {
         QString match   = QString( regexs[ j ] ).arg( args[ i ] );
         editor_msg( "dark blue", 
                     QString( us_tr( "Load %1 using %2 " ) ).arg( args[ i ] ).arg( match ) );
         add_files( qd.entryList( QStringList() << "*" ).filter( QRegExp( match ) ) );
      }
   }
}

void US_Hydrodyn_Mals::rename_from_context( const QPoint & pos ) {
#if QT_VERSION >= 0x040000
   QListWidgetItem * lwi = lb_created_files->itemAt( pos );
   if ( lwi ) {
      return rename_created( lwi, pos );
   }
#endif
}

void US_Hydrodyn_Mals::rename_created( QListWidgetItem *lbi, const QPoint & )
{
   map < QString, bool > existing_items;
   for ( int i = 0; i < (int)lb_files->count(); i++ )
   {
      existing_items[ lb_files->item( i )->text() ] = true;
   }

   QString dupmsg;
   bool ok;
   QString text = lbi->text();
   do {
      text = US_Static::getText(
                                   us_tr("US-SOMO: Copy File"),
                                   dupmsg + "New name:", 
                                   QLineEdit::Normal,
                                   text, 
                                   &ok, 
                                   this );
      text.replace( QRegExp( "\\s" ), "_" );
      if ( ok && !text.isEmpty() )
      {
         // user entered something and pressed OK
         dupmsg = "";
         if ( existing_items.count( text ) )
         {
            dupmsg = us_tr( "Name already exists, choose a unique name\n" );
         }
      } 
   } while ( ok && !text.isEmpty() && !dupmsg.isEmpty() );

   if ( !ok || text.isEmpty() )
   {
      return;
   }
   if ( existing_items.count( text ) )
   {
      editor_msg( "red", us_tr( "Internal error: duplicate name" ) );
      return;
   }

   // just a copy right now, since lbi is "protected within this context"

   lb_created_files->addItem( text );
   lb_created_files->scrollToItem( lb_created_files->item( lb_created_files->count() - 1 ) );
   lb_files->addItem( text );
   lb_files->scrollToItem( lb_files->item( lb_files->count() - 1 ) );
   created_files_not_saved[ text ] = true;

   f_pos      [ text ] = f_qs.size(); // pos      [ lbi->text() ];
   f_qs_string[ text ] = f_qs_string[ lbi->text() ];
   f_qs       [ text ] = f_qs       [ lbi->text() ];
   f_Is       [ text ] = f_Is       [ lbi->text() ];
   if ( f_errors.count( lbi->text() ))
   {
      f_errors   [ text ] = f_errors   [ lbi->text() ];
   }
   if ( f_gaussians.count( lbi->text() ) )
   {
      f_gaussians[ text ] = f_gaussians[ lbi->text() ];
   }
   f_is_time  [ text ] = f_is_time  [ lbi->text() ];
   f_psv       [ text ] = f_psv.count( lbi->text() ) ? f_psv[ lbi->text() ] : 0e0;
   f_I0se      [ text ] = f_I0se.count( lbi->text() ) ? f_I0se[ lbi->text() ] : 0e0;
   f_conc      [ text ] = f_conc.count( lbi->text() ) ? f_conc[ lbi->text() ] : 0e0;
   f_header    [ text ] = f_header.count( lbi->text() ) ? f_header[ lbi->text() ] : "";

   update_csv_conc();
   map < QString, double > concs = current_concs();
   double lbi_conc = 
      concs.count( lbi->text() ) ?
      concs[ lbi->text() ] :
      0e0;

   for ( unsigned int i = 0; i < csv_conc.data.size(); i++ )
   {
      if ( csv_conc.data[ i ].size() > 1 &&
           csv_conc.data[ i ][ 0 ] == text )
      {
         csv_conc.data[ i ][ 1 ] = QString( "%1" ).arg( lbi_conc );
      }
   }

   if ( conc_widget )
   {
      conc_window->refresh( csv_conc );
   }

   update_enables();
}

void US_Hydrodyn_Mals::normalize()
{
   QStringList files = all_selected_files();
   //    for ( int i = 0; i < lb_files->count(); i++ )
   //    {
   //       if ( lb_files->item( i )->isSelected() && 
   //            lb_files->item( i )->text() != lbl_mals->text() &&
   //            lb_files->item( i )->text() != lbl_empty->text() )
   //       {
   //          files << lb_files->item( i )->text();
   //       }
   //    }

   update_csv_conc();
   map < QString, double > concs = current_concs();
   map < QString, double > inv_concs;

   for ( map < QString, double >::iterator it = concs.begin();
         it != concs.end();
         it++ )
   {
      if ( it->second != 0e0 )
      {
         inv_concs[ it->first ] = 1e0 / it->second;
      }
   }

   map < QString, bool > existing_items;
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      existing_items[ lb_files->item( i )->text() ] = true;
   }

   for ( int i = 0; i < (int)files.size(); i++ )
   {
      
      QString norm_name = files[ i ] + "_n";
      unsigned int ext = 0;
      while ( existing_items.count( norm_name ) )
      {
         norm_name = files[ i ] + QString( "_n%1" ).arg( ++ext );
      }

      lb_created_files->addItem( norm_name );
      lb_created_files->scrollToItem( lb_created_files->item( lb_created_files->count() - 1 ) );
      lb_files->addItem( norm_name );
      lb_files->scrollToItem( lb_files->item( lb_files->count() - 1 ) );
      created_files_not_saved[ norm_name ] = true;

      f_pos      [ norm_name ] = f_qs.size(); 
      f_qs_string[ norm_name ] = f_qs_string[ files[ i ] ];
      f_qs       [ norm_name ] = f_qs       [ files[ i ] ];
      f_Is       [ norm_name ] = f_Is       [ files[ i ] ];
      for ( unsigned int j = 0; j < f_Is[ norm_name ].size(); j++ )
      {
         f_Is[ norm_name ][ j ] *= inv_concs[ files[ i ] ];
      }
      if ( f_errors.count( files[ i ] ))
      {
         f_errors   [ norm_name ] = f_errors   [ files[ i ] ];
         for ( unsigned int j = 0; j < f_errors[ norm_name ].size(); j++ )
         {
            f_errors[ norm_name ][ j ] *= inv_concs[ files[ i ] ];
         }
      }
      f_is_time  [ norm_name ] = false;
      f_psv      [ norm_name ] = f_psv.count( files[ i ] ) ? f_psv[ files[ i ] ] : 0;
      f_I0se     [ norm_name ] = f_I0se.count( files[ i ] ) ? f_I0se[ files[ i ] ] : 0;
      f_header   [ norm_name ] = f_header.count( files[ i ] ) ? f_header[ files[ i ] ] : "";

      if ( f_extc.count( files[ i ] ) ) {
         f_extc     [ norm_name ] = f_extc[ files[ i ] ];
      }
      f_conc     [ norm_name ] = 1e0;
      {
         vector < double > tmp;
         f_gaussians  [ norm_name ] = tmp;
      }

      update_csv_conc();

      for ( unsigned int i = 0; i < csv_conc.data.size(); i++ )
      {
         if ( csv_conc.data[ i ].size() > 1 &&
              csv_conc.data[ i ][ 0 ] == norm_name )
         {
            csv_conc.data[ i ][ 1 ] = "1";
         }
      }

      if ( conc_widget )
      {
         conc_window->refresh( csv_conc );
      }
   }

   update_enables();
}

void US_Hydrodyn_Mals::add_plot( QString           name,
                                      vector < double > q,
                                      vector < double > I,
                                      bool              is_time,
                                      bool              replot )
{
   vector < double > errors( I.size() );
   for ( unsigned int i = 0; i < ( unsigned int ) errors.size(); i++ )
   {
      errors[ i ] = 0e0;
   }
   add_plot( name, q, I, errors, is_time, replot );
}

void US_Hydrodyn_Mals::add_plot( QString           name,
                                      vector < double > q,
                                      vector < double > I,
                                      vector < double > errors,
                                      bool              is_time,
                                      bool              replot )
{
   name.replace( QRegExp( "(\\s+|\"|'|\\/|\\.)" ), "_" );
   if ( q.size() != I.size() )
   {
      TSO << QString( "add_plot: size error %1 %2\n" ).arg( q.size() ).arg( I.size() );
   }

   // printvector( "add_plot q", q );
   // printvector( "add_plot I", I );
   
   QString bsub_name = name;
   unsigned int ext = 0;

   map < QString, bool > current_files;

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      QString this_file = lb_files->item( i )->text();
      current_files[ this_file ] = true;
   }

   while ( current_files.count( bsub_name ) )
   {
      bsub_name = name + QString( "-%1" ).arg( ++ext );
   }

   vector < QString > q_string( q.size() );
   for ( unsigned int i = 0; i < ( unsigned int ) q_string.size(); i++ )
   {
      q_string[ i ] = QString( "%1" ).arg( q[ i ] );
   }
   
   lb_created_files->addItem( bsub_name );
   lb_created_files->scrollToItem( lb_created_files->item( lb_created_files->count() - 1 ) );
   lb_files->addItem( bsub_name );
   lb_files->scrollToItem( lb_files->item( lb_files->count() - 1 ) );
   created_files_not_saved[ bsub_name ] = true;
   last_created_file = bsub_name;
   
   f_pos       [ bsub_name ] = f_qs.size();
   f_qs_string [ bsub_name ] = q_string;
   f_qs        [ bsub_name ] = q;
   f_Is        [ bsub_name ] = I;
   f_errors    [ bsub_name ] = errors;
   f_is_time   [ bsub_name ] = is_time;
   f_conc      [ bsub_name ] = f_conc.count( bsub_name ) ? f_conc[ bsub_name ] : 0e0;
   {
      vector < double > tmp;
      f_gaussians  [ bsub_name ] = tmp;
   }
   
   // we could check if it has changed and then delete
   if ( plot_dist_zoomer )
   // {
   //    delete plot_dist_zoomer;
   //    plot_dist_zoomer = (ScrollZoomer *) 0;
   // }
   if ( replot )
   {
      plot_files();
      update_enables();
   }
}

void US_Hydrodyn_Mals::crop_zero()
{
   // delete points < zero of selected curves

   if ( QMessageBox::Ok != 
        QMessageBox::warning(
                             this,
                             this->windowTitle() + us_tr(": Crop zeros" ),
                             us_tr(
                                "Noisy intensity data that sometimes get negative values are physically meaningful.\n"
                                "The curves are a subtraction between two positive and very close intensity curves." ),
                             QMessageBox::Ok,
                             QMessageBox::Cancel | QMessageBox::Default
                             ) )
   {
      return;
   }

   map < QString, bool > selected_files;

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         QString this_file = lb_files->item( i )->text();

         vector < double > new_q;
         vector < double > new_I;
         vector < double > new_errors;

         bool has_errors  = f_errors[ this_file ].size();

         if ( f_qs.count( this_file ) &&
              f_Is.count( this_file ) )
         {
            for ( unsigned int i = 0; i < f_qs[ this_file ].size(); i++ )
            {
               if ( f_Is[ this_file ][ i ] > 0e0 )
               {
                  new_q.push_back( f_qs[ this_file ][ i ] );
                  new_I.push_back( f_Is[ this_file ][ i ] );
                  if ( has_errors )
                  {
                     new_errors.push_back( f_errors[ this_file ][ i ] );
                  }
               }
            }
            f_qs    [ this_file ] = new_q;
            f_Is    [ this_file ] = new_I;
            f_errors[ this_file ] = new_errors;
         }
      }
   }
      
   // we could check if it has changed and then delete
   // if ( plot_dist_zoomer )
   // {
   //    delete plot_dist_zoomer;
   //    plot_dist_zoomer = (ScrollZoomer *) 0;
   // }
   plot_files();
}

bool US_Hydrodyn_Mals::compatible_files( QStringList files )
{
   if ( !files.size() )
   {
      return true;
   }

   if ( !f_is_time.count( files[ 0 ] ) )
   {
      editor_msg( "red", QString( us_tr( "Internal error: file %1 has no q/T encoding data" ) ).arg( files[ 0 ] ) );
      return false;
   }

   bool first_type = f_is_time[ files[ 0 ] ];
   for ( unsigned int i = 1; i < ( unsigned int ) files.size(); i++ )
   {
      if ( !f_is_time.count( files[ i ] ) )
      {
         editor_msg( "red", QString( us_tr( "Internal error: file %1 has no q/T encoding data" ) ).arg( files[ i ] ) );
         return false;
      }
      if ( f_is_time[ files[ i ] ] != first_type )
      {
         return false;
      }
   }
   return true;
}

bool US_Hydrodyn_Mals::type_files( QStringList files )
{
   if ( !files.size() )
   {
      return false;
   }

   if ( !f_is_time.count( files[ 0 ] ) )
   {
      editor_msg( "red", QString( us_tr( "Internal error: file %1 has no q/T encoding data" ) ).arg( files[ 0 ] ) );
      return false;
   }

   return f_is_time[ files[ 0 ] ];
}

void US_Hydrodyn_Mals::gaussian_enables()
{
   unsigned int g_size = ( unsigned int )gaussians.size() / gaussian_type_size;

   pb_gauss_mode       ->setEnabled( false );
   pb_gauss_start      ->setEnabled( false );
   pb_gauss_clear      ->setEnabled( g_size );
   pb_gauss_new        ->setEnabled( true );
   pb_gauss_delete     ->setEnabled( g_size );
   pb_gauss_prev       ->setEnabled( g_size > 1 && gaussian_pos > 0 );
   pb_gauss_next       ->setEnabled( g_size > 1 && gaussian_pos < g_size - 1 );
   cb_sd_weight        ->setEnabled( g_size && le_gauss_fit_start->text().toDouble() < le_gauss_fit_end->text().toDouble() );
   pb_gauss_fit        ->setEnabled( g_size && le_gauss_fit_start->text().toDouble() < le_gauss_fit_end->text().toDouble() );
   pb_wheel_cancel     ->setEnabled( true );
   pb_wheel_save       ->setEnabled( true );
   le_gauss_pos        ->setEnabled( g_size && gaussian_pos < g_size );
   le_gauss_pos_width  ->setEnabled( g_size && gaussian_pos < g_size );
   le_gauss_pos_height ->setEnabled( g_size && gaussian_pos < g_size );
   le_gauss_pos_dist1  ->setEnabled( g_size && gaussian_pos < g_size );
   le_gauss_pos_dist2  ->setEnabled( g_size && gaussian_pos < g_size );
   le_gauss_fit_start  ->setEnabled( g_size && gaussian_pos < g_size );
   le_gauss_fit_end    ->setEnabled( g_size && gaussian_pos < g_size );
   pb_gauss_save       ->setEnabled( g_size );
   pb_gauss_as_curves  ->setEnabled( g_size );
   wheel_enables       ( g_size && gaussian_pos < g_size );
   pb_rescale          ->setEnabled( true );
   pb_rescale_y        ->setEnabled( true );
   pb_view             ->setEnabled( true );
   pb_errors           ->setEnabled( true );
   pb_pp               ->setEnabled( true );

   le_gauss_local_pts      ->setEnabled( g_size && gaussian_pos < g_size );
   pb_gauss_local_caruanas ->setEnabled( g_size && gaussian_pos < g_size );
   pb_gauss_local_guos     ->setEnabled( g_size && gaussian_pos < g_size );
}

void US_Hydrodyn_Mals::update_gauss_pos()
{
   if ( current_mode == MODE_GAUSSIAN )
   {
      if ( gaussians.size() )
      {
         lbl_gauss_pos      ->setText( QString( " %1 of %2 " ).arg( gaussian_pos + 1 ).arg( gaussians.size() / gaussian_type_size ) );

         disconnect( le_gauss_pos       , SIGNAL( textChanged( const QString & ) ), 0, 0 );
         disconnect( le_gauss_pos_width , SIGNAL( textChanged( const QString & ) ), 0, 0 );
         disconnect( le_gauss_pos_height, SIGNAL( textChanged( const QString & ) ), 0, 0 );
         if ( dist1_active )
         {
            disconnect( le_gauss_pos_dist1 , SIGNAL( textChanged( const QString & ) ), 0, 0 );
            if ( dist2_active )
            {
               disconnect( le_gauss_pos_dist2 , SIGNAL( textChanged( const QString & ) ), 0, 0 );
            }
         }

         le_gauss_pos_height->setText( QString( "%1" ).arg( gaussians[ 0 + gaussian_type_size * gaussian_pos ], 0, 'g', UHSH_SHOW_DIGITS ) );
         le_gauss_pos       ->setText( QString( "%1" ).arg( gaussians[ 1 + gaussian_type_size * gaussian_pos ], 0, 'g', UHSH_SHOW_DIGITS ) );
         le_gauss_pos_width ->setText( QString( "%1" ).arg( gaussians[ 2 + gaussian_type_size * gaussian_pos ], 0, 'g', UHSH_SHOW_DIGITS ) );
         if ( dist1_active )
         {
            le_gauss_pos_dist1 ->setText( QString( "%1" ).arg( gaussians[ 3 + gaussian_type_size * gaussian_pos ], 0, 'g', UHSH_SHOW_DIGITS ) );
            if ( dist2_active )
            {
               le_gauss_pos_dist2 ->setText( QString( "%1" ).arg( gaussians[ 4 + gaussian_type_size * gaussian_pos ], 0, 'g', UHSH_SHOW_DIGITS ) );
            }
         }
            
         connect( le_gauss_pos       , SIGNAL( textChanged( const QString & ) ), SLOT( gauss_pos_text       ( const QString & ) ) );
         connect( le_gauss_pos_width , SIGNAL( textChanged( const QString & ) ), SLOT( gauss_pos_width_text ( const QString & ) ) );
         connect( le_gauss_pos_height, SIGNAL( textChanged( const QString & ) ), SLOT( gauss_pos_height_text( const QString & ) ) );
         if ( dist1_active )
         {
            connect( le_gauss_pos_dist1 , SIGNAL( textChanged( const QString & ) ), SLOT( gauss_pos_dist1_text ( const QString & ) ) );
            if ( dist2_active )
            {
               connect( le_gauss_pos_dist2 , SIGNAL( textChanged( const QString & ) ), SLOT( gauss_pos_dist2_text ( const QString & ) ) );
            }
         }
         if ( le_gauss_pos->hasFocus() ||
              !( le_gauss_pos_height->hasFocus() ||
                 le_gauss_pos_dist1 ->hasFocus() ||
                 le_gauss_pos_dist2 ->hasFocus() ||
                 le_gauss_pos_width ->hasFocus() ||
                 le_gauss_fit_start ->hasFocus() ||
                 le_gauss_fit_end   ->hasFocus() ) )
         {
            qwtw_wheel   ->setValue( gaussians[ 1 + gaussian_type_size * gaussian_pos ] );
         }
         if ( le_gauss_pos_height->hasFocus() )
         {
            qwtw_wheel   ->setValue( gaussians[ 0 + gaussian_type_size * gaussian_pos ] );
         }
         if ( le_gauss_pos_width->hasFocus() )
         {
            qwtw_wheel   ->setValue( gaussians[ 2 + gaussian_type_size * gaussian_pos ] );
         }
         if ( dist1_active && le_gauss_pos_dist1->hasFocus() )
         {
            qwtw_wheel   ->setValue( gaussians[ 3 + gaussian_type_size * gaussian_pos ] );
         }
         if ( dist2_active && le_gauss_pos_dist2->hasFocus() )
         {
            qwtw_wheel   ->setValue( gaussians[ 4 + gaussian_type_size * gaussian_pos ] );
         }

         for ( unsigned int i = 2; i < ( unsigned int ) plotted_markers.size(); i++ )
         {
            if ( gaussian_pos + 2 == i && !le_gauss_fit_start->hasFocus() && !le_gauss_fit_end->hasFocus() )
            {
#if QT_VERSION < 0x040000
               plot_dist->setMarkerPen       ( plotted_markers[ i ], QPen( Qt::magenta, 2, DashDotDotLine));
#else
               plotted_markers[ i ]->setLinePen( QPen( Qt::magenta, 2, Qt::DashDotDotLine ) );
#endif
            } else {
#if QT_VERSION < 0x040000
               plot_dist->setMarkerPen       ( plotted_markers[ i ], QPen( Qt::blue, 2, DashDotDotLine));
#else
               plotted_markers[ i ]->setLinePen( QPen( Qt::blue, 2, Qt::DashDotDotLine ) );
#endif
            }
         }
         if ( !suppress_replot )
         {
            plot_dist->replot();
         }
      } else {
         lbl_gauss_pos       ->setText( " 0 of 0 " );
         le_gauss_pos        ->setText( "" );
         le_gauss_pos_width  ->setText( "" );
         le_gauss_pos_height ->setText( "" );
         le_gauss_pos_dist1  ->setText( "" );
         le_gauss_pos_dist2  ->setText( "" );
      }      
   } else {
      // global gaussian mode
      {
         // us_qdebug( QString( "pos ggm common_size %1 gauss pos %2" ).arg( common_size ).arg( gaussian_pos ) );
         // US_Vector::printvector( "unified_ggaussian_params", unified_ggaussian_params );
         unsigned int ofs = 0;

         lbl_gauss_pos      ->setText( QString( " %1 of %2 " ).arg( gaussian_pos + 1 ).arg( gaussians.size() / gaussian_type_size ) );

         disconnect( le_gauss_pos       , SIGNAL( textChanged( const QString & ) ), 0, 0 );
         // us_qdebug( QString( "uggp size %1 ofs %2 common_size %3 gaussian_pos %4 index %5" )
         //         .arg( unified_ggaussian_params.size() )
         //         .arg( ofs )
         //         .arg( common_size )
         //         .arg( gaussian_pos )
         //         .arg( ofs + common_size * gaussian_pos ) );
                 
         le_gauss_pos       ->setText( QString( "%1" ).arg( unified_ggaussian_params[ ofs + common_size * gaussian_pos ] ) );
         connect( le_gauss_pos       , SIGNAL( textChanged( const QString & ) ), SLOT( gauss_pos_text       ( const QString & ) ) );
         ofs++;

         if ( cb_fix_width->isChecked() )
         {
            disconnect( le_gauss_pos_width , SIGNAL( textChanged( const QString & ) ), 0, 0 );
            le_gauss_pos_width ->setText( QString( "%1" ).arg( unified_ggaussian_params[ ofs + common_size * gaussian_pos ] ) );
            connect( le_gauss_pos_width , SIGNAL( textChanged( const QString & ) ), SLOT( gauss_pos_width_text ( const QString & ) ) );
            ofs++;
         }

         if ( dist1_active && cb_fix_dist1->isChecked() )
         {
            disconnect( le_gauss_pos_dist1 , SIGNAL( textChanged( const QString & ) ), 0, 0 );
            le_gauss_pos_dist1 ->setText( QString( "%1" ).arg( unified_ggaussian_params[ ofs + common_size * gaussian_pos ] ) );
            connect( le_gauss_pos_dist1 , SIGNAL( textChanged( const QString & ) ), SLOT( gauss_pos_dist1_text ( const QString & ) ) );
            ofs++;
         }

         if ( dist2_active && cb_fix_dist2->isChecked() )
         {
            disconnect( le_gauss_pos_dist2 , SIGNAL( textChanged( const QString & ) ), 0, 0 );
            le_gauss_pos_dist2 ->setText( QString( "%1" ).arg( unified_ggaussian_params[ ofs + common_size * gaussian_pos ] ) );
            connect( le_gauss_pos_dist2 , SIGNAL( textChanged( const QString & ) ), SLOT( gauss_pos_dist2_text ( const QString & ) ) );
            ofs++;
         }

         if ( le_gauss_pos->hasFocus() )
// ||
//               !( 
//                 ( cb_fix_width->isChecked() && le_gauss_pos_width ->hasFocus() ) ||
//                 ( dist1_active && cb_fix_dist1->isChecked() && le_gauss_pos_dist1 ->hasFocus() ) ||
//                 ( dist2_active && cb_fix_dist2->isChecked() && le_gauss_pos_dist2 ->hasFocus() ) ||
//                 le_gauss_fit_start ->hasFocus() ||
//                 le_gauss_fit_end   ->hasFocus() ) )
         {
            // us_qdebug( QString( "pos ggm update wheel gauss pos to %1" ).arg( le_gauss_pos->text() ) );
            qwtw_wheel   ->setValue( le_gauss_pos->text().toDouble() );
         }
         if ( cb_fix_width->isChecked() && le_gauss_pos_width->hasFocus() )
         {
            // us_qdebug( "pos ggm update wheel gauss width" );
            qwtw_wheel   ->setValue( le_gauss_pos_width->text().toDouble() );
         }
         if ( dist1_active && cb_fix_dist1->isChecked() && le_gauss_pos_dist1->hasFocus() )
         {
            // us_qdebug( "pos ggm update wheel gauss dist1" );
            qwtw_wheel   ->setValue( le_gauss_pos_dist1->text().toDouble() );
         }
         if ( dist2_active && cb_fix_dist2->isChecked() && le_gauss_pos_dist2->hasFocus() )
         {
            // us_qdebug( "pos ggm update wheel gauss dist2" );
            qwtw_wheel   ->setValue( le_gauss_pos_dist2->text().toDouble() );
         }

         for ( unsigned int i = 2; i < ( unsigned int ) plotted_markers.size(); i++ )
         {
            if ( gaussian_pos + 2 == i && !le_gauss_fit_start->hasFocus() && !le_gauss_fit_end->hasFocus() )
            {
#if QT_VERSION < 0x040000
               plot_dist->setMarkerPen       ( plotted_markers[ i ], QPen( Qt::magenta, 2, DashDotDotLine));
#else
               plotted_markers[ i ]->setLinePen( QPen( Qt::magenta, 2, Qt::DashDotDotLine));
#endif
            } else {
#if QT_VERSION < 0x040000
               plot_dist->setMarkerPen       ( plotted_markers[ i ], QPen( Qt::blue, 2, DashDotDotLine));
#else
               plotted_markers[ i ]->setLinePen( QPen( Qt::blue, 2, Qt::DashDotDotLine));
#endif
            }
         }
         if ( !suppress_replot )
         {
            plot_dist->replot();
         }
      }
   }
}

void US_Hydrodyn_Mals::gauss_start()
{
   plot_errors->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_errors->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   // if ( plot_errors_zoomer )
   // {
   //    delete plot_errors_zoomer;
   //    plot_errors_zoomer = (ScrollZoomer *) 0;
   // }

   le_last_focus = (mQLineEdit *) 0;
   pb_gauss_fit->setText( us_tr( "Fit" ) );

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         wheel_file = lb_files->item( i )->text();
         break;
      }
   }

   if ( !f_qs.count( wheel_file ) )
   {
      editor_msg( "red", QString( us_tr( "Internal error: %1 not found in data" ) ).arg( wheel_file ) );
      return;
   }

   if ( !f_qs[ wheel_file ].size() )
   {
      editor_msg( "red", QString( us_tr( "Internal error: %1 empty data" ) ).arg( wheel_file ) );
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

   wheel_errors_ok = 
      f_errors.count( wheel_file ) &&
      f_errors[ wheel_file ].size() == f_qs[ wheel_file ].size() &&
      is_nonzero_vector( f_errors[ wheel_file ] );

   get_peak( wheel_file, gauss_max_height );
   gauss_max_height *= 1.2;
   if ( gaussian_type != GAUSS )
   {
      gauss_max_height *= 20e0;
   }

   if ( gauss_max_height <= 0e0 )
   {
      editor_msg( "red", QString( us_tr( "Error: maximum y value of signal %1 is not positive" ) ).arg( wheel_file ) );
      return;
   }

#if QT_VERSION < 0x040000
   plot_dist->setCurvePen( plotted_curves[ wheel_file ], QPen( Qt::cyan, use_line_width, SolidLine));
#else
   plotted_curves[ wheel_file ]->setPen( QPen( Qt::cyan, use_line_width, Qt::SolidLine ) );
#endif

   org_gaussians = gaussians;

   if ( f_gaussians.count( wheel_file ) &&
        f_gaussians[ wheel_file ].size() )
   {
      TSO << "using file specific gaussians\n";
      gaussians = f_gaussians[ wheel_file ];
   }

   opt_repeak_gaussians( wheel_file );

   // gaussian_mode = true;
   mode_select( MODE_GAUSSIAN );

   running               = true;

   // this must be wrong:!! ***************, units do not make sense
   double max_range = gauss_max_height > f_qs[ wheel_file ].back() ? gauss_max_height : f_qs[ wheel_file ].back();

   qwtw_wheel->setRange( 0,  max_range); qwtw_wheel->setSingleStep( max_range / UHSH_WHEEL_RES );

   if ( le_gauss_fit_start->text().isEmpty() ||
        le_gauss_fit_start->text().toDouble() < f_qs[ wheel_file ][ 0 ] )
   {
      disconnect( le_gauss_fit_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_gauss_fit_start->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ 0 ] ) );
      connect( le_gauss_fit_start, SIGNAL( textChanged( const QString & ) ), SLOT( gauss_fit_start_text( const QString & ) ) );
   }

   if ( le_gauss_fit_end->text().isEmpty() ||
        le_gauss_fit_end->text().toDouble() > f_qs[ wheel_file ].back() )
   {
      TSO << "setting gauss fit end\n";
      disconnect( le_gauss_fit_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_gauss_fit_end->setText( QString( "%1" ).arg( f_qs[ wheel_file ].back() ) );
      connect( le_gauss_fit_end, SIGNAL( textChanged( const QString & ) ), SLOT( gauss_fit_end_text( const QString & ) ) );
   }

   if ( gaussians.size() && gaussian_pos >= gaussians.size() / gaussian_type_size )
   {
      gaussian_pos = ( gaussians.size() / gaussian_type_size ) - 1;
   }
   printf( "gaussian pos %d type size %d gaussians_size() %d\n", (int)gaussian_pos, (int) gaussian_type_size, (int)gaussians.size() );

   disable_all();
   gauss_init_markers();
   gauss_init_gaussians();
   update_gauss_pos();
   if ( errors_were_on ) {
      hide_widgets( plot_errors_widgets, false );
      cb_plot_errors_group->hide();
      if ( !f_errors.count( wheel_file ) ||
           !is_nonzero_vector(  f_errors[ wheel_file ]  ) ||
           f_errors[ wheel_file ].size() != f_qs[ wheel_file ].size() )
      {
         cb_plot_errors_sd->setChecked( false );
         cb_plot_errors_sd->hide();
      }
      qDebug() << "mals::gauss_start() emit do resize_plots()";
      emit do_resize_plots();
   }
   cb_gauss_match_amplitude->setChecked( true );
   height_natural_spline_x = f_qs[ wheel_file ];
   height_natural_spline_y = f_Is[ wheel_file ];
   height_natural_spline_y2.clear( );

   US_Saxs_Util::natural_spline( height_natural_spline_x,
                                 height_natural_spline_y,
                                 height_natural_spline_y2 );
   gaussian_enables();
}
      
void US_Hydrodyn_Mals::gauss_clear( )
{
   gaussian_pos = 0;
   gaussians.clear( );
   gauss_init_markers();
   gauss_init_gaussians();
   update_gauss_pos();
   gaussian_enables();
}

void US_Hydrodyn_Mals::gauss_new()
{
   double use_max_height = gauss_max_height;
   if ( gaussian_type != GAUSS )
   {
      use_max_height /= 20e0;
   }

   gaussians.push_back( use_max_height * 5e-1 );
   gaussians.push_back( 0e0 );
   gaussians.push_back( 2e0 );
   if ( dist1_active )
   {
      gaussians.push_back( 0e0 );
      if ( dist2_active )
      {
         gaussians.push_back( 0e0 );
      }
   }
      
   gaussian_pos = ( gaussians.size() / gaussian_type_size ) - 1;
   gauss_add_marker( 0e0, Qt::blue, QString( "%1" ).arg( gaussian_pos + 1 ) );
   gauss_add_gaussian( &(gaussians[ (vector<double>::size_type) gaussian_pos * gaussian_type_size ]), Qt::green );
   disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
   update_gauss_pos();
   connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
   gaussian_enables();
   if ( current_mode == MODE_GGAUSSIAN )
   {
      ggaussian_enables();
   } else {
      gaussian_enables();
   }
}

void US_Hydrodyn_Mals::gauss_prev()
{
   if ( gaussian_pos > 0 )
   {
      gaussian_pos--;
   }
   update_gauss_pos();
   if ( current_mode == MODE_GGAUSSIAN )
   {
      ggaussian_enables();
   } else {
      gaussian_enables();
   }
}

void US_Hydrodyn_Mals::gauss_next()
{
   if ( gaussian_pos < ( gaussians.size() / gaussian_type_size ) - 1 )
   {
      gaussian_pos++;
   }
   update_gauss_pos();
   if ( current_mode == MODE_GGAUSSIAN )
   {
      ggaussian_enables();
   } else {
      gaussian_enables();
   }
}

void US_Hydrodyn_Mals::gauss_save()
{
   {
      QDir dir1( le_created_dir->text() );
      if ( !dir1.exists() )
      {
         if ( dir1.mkdir( le_created_dir->text() ) )
         {
            editor_msg( "black", QString( us_tr( "Created directory %1" ) ).arg( le_created_dir->text() ) );
         } else {
            editor_msg( "red", QString( us_tr( "Error: Can not create directory %1 Check permissions." ) ).arg( le_created_dir->text() ) );
            return;
         }
      }
   }         

   if ( !QDir::setCurrent( le_created_dir->text() ) )
   {
      editor_msg( "red", QString( us_tr( "Error: can not set directory %1" ) ).arg( le_created_dir->text() ) );
      return;
   }

   if ( current_mode == MODE_GAUSSIAN )
   {
      QString use_filename = wheel_file + "-" + QString( "%1" ).arg( gaussian_type_tag ).toLower().replace( "+", "" ) + ".dat";
   
      if ( QFile::exists( use_filename ) )
      {
         use_filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( use_filename, 0, this );
         raise();
      }

      QFile f( use_filename );
      if ( !f.open( QIODevice::WriteOnly ) )
      {
         editor_msg( "red", QString( us_tr( "Error: can not open %1 for writing" ) ).arg( use_filename ) );
      }

      QTextStream ts( &f );

      ts << QString( "US-SOMO Hplc %1: %2\n" ).arg( gaussian_type_tag ).arg( wheel_file );

      ts << QString( "%1 %2\n" )
         .arg( le_gauss_fit_start->text() )
         .arg( le_gauss_fit_end  ->text() )
         ;

      for ( unsigned int i = 0; i < ( unsigned int ) gaussians.size(); i += gaussian_type_size )
      {
         for ( int k = 0; k < gaussian_type_size; k++ )
         {
            ts << QString( "%1 " ).arg( gaussians[ k + i ], 0, 'g', 10 );
         }
         ts << "\n";
      }
      f.close();
      editor_msg( "black", QString( us_tr( "Gaussians written as %1" ) )
                  .arg( use_filename ) );
   } else {
      if ( unified_ggaussian_ok )
      {
         map < QString, vector < double > > save_f_gaussians = f_gaussians;
         if ( !unified_ggaussian_to_f_gaussians() )
         {
            editor_msg( "red", us_tr( "could not save Gaussians" ) );
            f_gaussians = save_f_gaussians;
            return;
         }

         QString use_filename = wheel_file + "-m" + QString( "%1" ).arg( gaussian_type_tag ).toLower().replace( "+", "" ) + ".dat";
   
         if ( QFile::exists( use_filename ) )
         {
            use_filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( use_filename, 0, this );
            raise();
         }

         QFile f( use_filename );
         if ( !f.open( QIODevice::WriteOnly ) )
         {
            editor_msg( "red", QString( us_tr( "Error: can not open %1 for writing" ) ).arg( use_filename ) );
         }

         QTextStream ts( &f );

         ts << QString( "US-SOMO Hplc Multiple %1: %2\n").arg( gaussian_type_tag ).arg( wheel_file );

         ts << QString( "%1 %2\n" ).arg( le_gauss_fit_start->text() ).arg( le_gauss_fit_end->text() );

         vector < map < int, set < double > > > check_common( unified_ggaussian_gaussians_size );
         // ^^ [gauss-number][ parameter ] values
      
         for ( unsigned int i = 0; i < ( unsigned int ) unified_ggaussian_files.size(); i++ )
         {
            ts << QString( "%1 %2\n" ).arg( gaussian_type_tag ).arg( unified_ggaussian_files[ i ] );

            for ( unsigned int j = 0, gn = 0; 
                  j < ( unsigned int ) f_gaussians[ unified_ggaussian_files[ i ] ].size();
                  j += gaussian_type_size, ++gn )
            {
               for ( int k = 0; k < gaussian_type_size; ++k )
               {
                  ts << QString( "%1 " ).arg( f_gaussians[ unified_ggaussian_files[ i ] ][ k + j ], 0, 'g', 10 );
                  check_common[ gn ][ k ].insert( f_gaussians[ unified_ggaussian_files[ i ] ][ k + j ] );
               }
               ts << "\n";
            }

            // if ( cb_fix_width->isChecked() )
            // {
            //    unsigned int  index = 2 * unified_ggaussian_gaussians_size + i * unified_ggaussian_gaussians_size;
            //    for ( unsigned int j = 0; j < unified_ggaussian_gaussians_size; j++ )
            //    {
            //       ts << 
            //          QString( "%1 %2 %3\n" )
            //          .arg( unified_ggaussian_params[ index + j + 0 ], 0, 'g', 10 )
            //          .arg( unified_ggaussian_params[ 2 * j + 0 ]                , 0, 'g', 10 )
            //          .arg( unified_ggaussian_params[ 2 * j + 1 ], 0, 'g', 10 )
            //          ;
            //    }
            // } else {
            //    unsigned int  index = unified_ggaussian_gaussians_size + i * 2 * unified_ggaussian_gaussians_size;
            //    for ( unsigned int j = 0; j < unified_ggaussian_gaussians_size; j++ )
            //    {
            //       ts << 
            //          QString( "%1 %2 %3\n" )
            //          .arg( unified_ggaussian_params[ index + 2 * j + 0 ], 0, 'g', 10 )
            //          .arg( unified_ggaussian_params[ j ]                , 0, 'g', 10 )
            //          .arg( unified_ggaussian_params[ index + 2 * j + 1 ], 0, 'g', 10 )
            //          ;
            //    }
            // }               
         }

         f.close();
         ggauss_msg_common( check_common );

         f_gaussians = save_f_gaussians;
         editor_msg( "black", QString( us_tr( "Gaussians written as %1" ) )
                     .arg( use_filename ) );
      }
   }      
}

void US_Hydrodyn_Mals::ggauss_msg_common( vector < map < int, set < double > > > & check_common )
{
   for ( int i = 0; i < (int) check_common.size(); ++i )
   {
      for ( map < int, set < double > >::iterator it = check_common[ i ].begin();
            it != check_common[ i ].end();
            ++it )
      {
         QString msg = 
            QString( "%1 %2 %3 %4 unique value%5" )
            .arg( gaussian_type_tag )
            .arg( i + 1 )
            .arg( gaussian_param_text[ it->first ] )
            .arg( it->second.size() )
            .arg( it->second.size() > 1 ? "s" : "" )
            ;
         int count = 0;
         for ( set < double >::iterator it2 = it->second.begin();
               it2 != it->second.end() && count < 5;
               ++it2, ++count )
         {
            msg += QString( " %1" ).arg( *it2 );
         }
         if ( count == 5 )
         {
            msg += " ...";
         }
            
         editor_msg( it->second.size() > 1 ? "blue" : "black", msg );
      }
   }  
}

void US_Hydrodyn_Mals::gauss_pos_text( const QString & text )
{
   if ( gaussians.size() )
   {
      if ( current_mode == MODE_GAUSSIAN )
      {
         // TSO << QString( "gauss_pos_text <%1>, pos %2 size %3\n" ).arg( text ).arg( gaussian_pos ).arg( gaussians.size() );
         gaussians[ 1 + gaussian_type_size * gaussian_pos ] = text.toDouble();
         if ( cb_gauss_match_amplitude->isChecked() )
         {
            double val;
            if ( height_wheel_file( val, gaussians[ 1 + gaussian_type_size * gaussian_pos ] ) )
            {
               le_gauss_pos_height->setText( QString( "%1" ).arg( val ) );
            }
         }
      } else {
         if ( current_mode == MODE_GGAUSSIAN )
         {
            unsigned int common_size   = 1; // center always fixed 
   
            if ( cb_fix_width->isChecked() )
            {
               common_size++;
            }
            if ( dist1_active && cb_fix_dist1->isChecked() )
            {
               common_size++;
            }
            if ( dist2_active && cb_fix_dist2->isChecked() )
            {
               common_size++;
            }

            unified_ggaussian_params[ 0 + common_size * gaussian_pos ] = text.toDouble();

            if ( cb_fix_width->isChecked() && plotted_hlines.size() > gaussian_pos )
            {
               double center = unified_ggaussian_params[ common_size * gaussian_pos + 0 ];
               double width  = unified_ggaussian_params[ common_size * gaussian_pos + 1 ];
               double fwhm   = 2.354820045e0 * width;

               vector < double > x( 2 );
               vector < double > y( 2 );

               double use_max_height = gauss_max_height;
               if ( gaussian_type != GAUSS )
               {
                  use_max_height /= 20e0;
               }

               x[ 0 ] = center - fwhm * 5e-1;
               x[ 1 ] = center + fwhm * 5e-1;
               y[ 0 ] = use_max_height / 3e0 + ( use_max_height * (double) gaussian_pos / 100e0 );
               y[ 1 ] = y[ 0 ];

               // TSO << QString( "add_hline %1 %2 (%3,%4) (%5,%6)\n" )
               //    .arg( center )
               //    .arg( width )
               //    .arg( x[0] )
               //    .arg( y[0] )
               //    .arg( x[1] )
               //    .arg( y[1] )
               //    ;
#if QT_VERSION < 0x040000
               plot_dist->setCurveData( plotted_hlines[ gaussian_pos ],
                                        (double *)&x[ 0 ],
                                        (double *)&y[ 0 ],
                                        2
                                        );
#else
               plotted_hlines[ gaussian_pos ]->setSamples(
                                                       (double *)&x[ 0 ],
                                                       (double *)&y[ 0 ],
                                                       2
                                                       );
#endif
            }
         }
      }
      
#if QT_VERSION < 0x040000
      plot_dist->setMarkerPos( plotted_markers[ 2 + gaussian_pos ], text.toDouble(), 0e0 );
#else
      plotted_markers[ 2 + gaussian_pos ]->setXValue( text.toDouble() );
#endif
      if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
      {
         disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
         qwtw_wheel->setValue( text.toDouble() );
         connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      }

      if ( current_mode == MODE_GAUSSIAN )
      {
         gauss_replot_gaussian();
      }
      if ( !suppress_replot )
      {
         plot_dist->replot();
      }
   }
}

void US_Hydrodyn_Mals::gauss_pos_width_text( const QString & text )
{
   if ( gaussians.size() )
   {
      if ( current_mode == MODE_GAUSSIAN )
      {
         gaussians[ 2 + gaussian_type_size * gaussian_pos ] = text.toDouble();
      } else {
         if ( current_mode == MODE_GGAUSSIAN &&
              cb_fix_width->isChecked() )
         {

            unsigned int common_size   = 1; // center always fixed 
   
            if ( cb_fix_width->isChecked() )
            {
               common_size++;
            }
            if ( dist1_active && cb_fix_dist1->isChecked() )
            {
               common_size++;
            }
            if ( dist2_active && cb_fix_dist2->isChecked() )
            {
               common_size++;
            }

            // us_qdebug( QString( "gauss_pos_width_text common size %1 gaussian_pos %2" ).arg( common_size ).arg( gaussian_pos ) );

            unified_ggaussian_params[ common_size * gaussian_pos + 1 ] = text.toDouble();

            if ( plotted_hlines.size() > gaussian_pos )
            {
               double center = unified_ggaussian_params[ common_size * gaussian_pos + 0 ];
               double width  = unified_ggaussian_params[ common_size * gaussian_pos + 1 ];
               double fwhm   = 2.354820045e0 * width;

               double use_max_height = gauss_max_height;
               if ( gaussian_type != GAUSS )
               {
                  use_max_height /= 20e0;
               }

               vector < double > x( 2 );
               vector < double > y( 2 );

               x[ 0 ] = center - fwhm * 5e-1;
               x[ 1 ] = center + fwhm * 5e-1;
               y[ 0 ] = use_max_height / 3e0 + ( use_max_height * (double) gaussian_pos / 100e0 );
               y[ 1 ] = y[ 0 ];

               // TSO << QString( "add_hline %1 %2 (%3,%4) (%5,%6)\n" )
               //    .arg( center )
               //    .arg( width )
               //    .arg( x[0] )
               //    .arg( y[0] )
               //    .arg( x[1] )
               //    .arg( y[1] )
               //    ;
#if QT_VERSION < 0x040000
               plot_dist->setCurveData( plotted_hlines[ gaussian_pos ],
                                        (double *)&x[ 0 ],
                                        (double *)&y[ 0 ],
                                        2
                                        );
#else
               plotted_hlines[ gaussian_pos ]->setSamples(
                                                       (double *)&x[ 0 ],
                                                       (double *)&y[ 0 ],
                                                       2
                                                       );
#endif
            }
         }
      }

      if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
      {
         disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
         qwtw_wheel->setValue( text.toDouble() );
         connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      }

      if ( current_mode == MODE_GAUSSIAN )
      {
         gauss_replot_gaussian();
      }
      if ( !suppress_replot )
      {
         plot_dist->replot();
      }
   }
}

void US_Hydrodyn_Mals::gauss_pos_height_text( const QString & text )
{
   if ( gaussians.size() )
   {
      gaussians[ 0 + gaussian_type_size * gaussian_pos ] = text.toDouble();
      if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
      {
         disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
         qwtw_wheel->setValue( text.toDouble() );
         connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      }
      gauss_replot_gaussian();
   }
   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
}

void US_Hydrodyn_Mals::gauss_pos_dist1_text( const QString & text )
{
   if ( gaussians.size() )
   {
      if ( current_mode == MODE_GAUSSIAN )
      {
         gaussians[ 3 + gaussian_type_size * gaussian_pos ] = text.toDouble();
         if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
         {
            disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
            qwtw_wheel->setValue( text.toDouble() );
            connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
         }
         gauss_replot_gaussian();
         if ( !suppress_replot )
         {
            plot_dist->replot();
         }
      } else {
         if ( current_mode == MODE_GGAUSSIAN )
         {
            unsigned int common_size   = 1; // center always fixed 
            unsigned int ofs           = 1; // starts with center
   
            if ( cb_fix_width->isChecked() )
            {
               common_size++;
               ofs++;
            }
            if ( dist1_active && cb_fix_dist1->isChecked() )
            {
               common_size++;
            }
            if ( dist2_active && cb_fix_dist2->isChecked() )
            {
               common_size++;
            }
            unified_ggaussian_params[ ofs + common_size * gaussian_pos ] = text.toDouble();
         }
      }         
   }
}

void US_Hydrodyn_Mals::gauss_pos_dist2_text( const QString & text )
{
   if ( gaussians.size() )
   {
      if ( current_mode == MODE_GAUSSIAN )
      {
         gaussians[ 4 + gaussian_type_size * gaussian_pos ] = text.toDouble();
         if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
         {
            disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
            qwtw_wheel->setValue( text.toDouble() );
            connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
         }
         gauss_replot_gaussian();
         if ( !suppress_replot )
         {
            plot_dist->replot();
         }
      } else {
         if ( current_mode == MODE_GGAUSSIAN )
         {
            unsigned int common_size   = 1; // center always fixed 
            unsigned int ofs           = 1; // starts with center
   
            if ( cb_fix_width->isChecked() )
            {
               common_size++;
               ofs++;
            }
            if ( dist1_active && cb_fix_dist1->isChecked() )
            {
               common_size++;
               ofs++;
            }
            if ( dist2_active && cb_fix_dist2->isChecked() )
            {
               common_size++;
            }
            unified_ggaussian_params[ ofs + common_size * gaussian_pos ] = text.toDouble();
         }
      }         
   }
}

void US_Hydrodyn_Mals::gauss_fit_start_text( const QString & text )
{
#if QT_VERSION < 0x040000
   plot_dist->setMarkerPos( plotted_markers[ 0 ], text.toDouble(), 0e0 );
#else
   plotted_markers[ 0 ]->setXValue( text.toDouble() );
#endif
   if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }

   if ( current_mode == MODE_GAUSSIAN )
   {
      replot_gaussian_sum();
      gaussian_enables();
   } else {
      if ( current_mode == MODE_GGAUSSIAN )
      {
         ggaussian_enables();
      }
   }

   if ( !suppress_replot )
   {
      // rescale_y_plot_errors();
      plot_dist->replot();
   }
}

void US_Hydrodyn_Mals::gauss_fit_end_text( const QString & text )
{
#if QT_VERSION < 0x040000
   plot_dist->setMarkerPos( plotted_markers[ 1 ], text.toDouble(), 0e0 );
#else
   plotted_markers[ 1 ]->setXValue( text.toDouble() );
#endif
   if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
   if ( current_mode == MODE_GAUSSIAN )
   {
      replot_gaussian_sum();
      gaussian_enables();
   } else {
      if ( current_mode == MODE_GGAUSSIAN )
      {
         ggaussian_enables();
      }
   }

   if ( !suppress_replot )
   {
      // rescale_y_plot_errors();
      plot_dist->replot();
   }
}

void US_Hydrodyn_Mals::gauss_add_marker( double pos, 
                                              QColor color, 
                                              QString text, 
#if QT_VERSION < 0x040000
                                              int 
#else
                                              Qt::Alignment
#endif
                                              align )
{
   int line_width = use_line_width < 3 ? ( use_line_width + 1 ) : use_line_width;

#if QT_VERSION < 0x040000
   long marker = plot_dist->insertMarker();
   plot_dist->setMarkerLineStyle ( marker, QwtMarker::VLine );
   plot_dist->setMarkerPos       ( marker, pos, 0e0 );
   plot_dist->setMarkerLabelAlign( marker, align );
   plot_dist->setMarkerPen       ( marker, QPen( color, line_width, DashDotDotLine));
   plot_dist->setMarkerFont      ( marker, QFont("Courier", 11, QFont::Bold));
   plot_dist->setMarkerLabelText ( marker, text );
#else
   QwtPlotMarker * marker = new QwtPlotMarker;
   marker->setLineStyle       ( QwtPlotMarker::VLine );
   marker->setLinePen         ( QPen( color, line_width, Qt::DashDotDotLine ) );
   marker->setLabelOrientation( Qt::Horizontal );
   marker->setXValue          ( pos );
   marker->setLabelAlignment  ( align );
   {
      QwtText qwtt( text );
      qwtt.setFont( QFont("Courier", 11, QFont::Bold ) );
      marker->setLabel           ( qwtt );
   }
   marker->attach             ( plot_dist );
#endif
   plotted_markers.push_back( marker );
}   

void US_Hydrodyn_Mals::gauss_add_hline( double center, double width )
{
   double fwhm = 2.354820045e0 * width;

   vector < double > x( 2 );
   vector < double > y( 2 );

   double use_max_height = gauss_max_height;
   if ( gaussian_type != GAUSS )
   {
      use_max_height /= 20e0;
   }

   x[ 0 ] = center - fwhm * 5e-1;
   x[ 1 ] = center + fwhm * 5e-1;
   y[ 0 ] = use_max_height / 3e0 + ( use_max_height * (double) plotted_hlines.size() / 100e0 );
   y[ 1 ] = y[ 0 ];

   // TSO << QString( "add_hline %1 %2 (%3,%4) (%5,%6)\n" )
   //    .arg( center )
   //    .arg( width )
   //    .arg( x[0] )
   //    .arg( y[0] )
   //    .arg( x[1] )
   //    .arg( y[1] )
   //    ;

#if QT_VERSION < 0x040000
   long curve;
   curve = plot_dist->insertCurve( "hline" );
   plot_dist->setCurveStyle( curve, QwtCurve::Lines );
#else
   QwtPlotCurve *curve = new QwtPlotCurve( "hline" );
   curve->setStyle( QwtPlotCurve::Lines );
#endif

   plotted_hlines.push_back( curve );

#if QT_VERSION < 0x040000
   plot_dist->setCurvePen( curve, QPen( Qt::green, use_line_width, Qt::SolidLine ) );
   plot_dist->setCurveData( curve,
                            (double *)&x[ 0 ],
                            (double *)&y[ 0 ],
                            2
                            );
#else
   curve->setPen( QPen( Qt::green, use_line_width, Qt::SolidLine ) );
   curve->setSamples(
                  (double *)&x[ 0 ],
                  (double *)&y[ 0 ],
                  2
                  );
   curve->attach( plot_dist );
#endif
}   

void US_Hydrodyn_Mals::gauss_init_markers()
{
   gauss_delete_markers();

   gauss_add_marker( le_gauss_fit_start->text().toDouble(), Qt::red, us_tr( "Fit start" ) );
   gauss_add_marker( le_gauss_fit_end  ->text().toDouble(), Qt::red, us_tr( "Fit end"   ), Qt::AlignLeft | Qt::AlignTop );

   if ( current_mode == MODE_GAUSSIAN )
   {
      for ( unsigned int i = 0; i < ( unsigned int ) gaussians.size() / gaussian_type_size; i++ )
      {
         gauss_add_marker( gaussians[ 1 + gaussian_type_size * i ], Qt::blue, QString( "%1" ).arg( i + 1 ) );
      }
   } else {
      // global gaussian
      if ( unified_ggaussian_ok )
      {
         if ( cb_fix_width->isChecked() && !cb_ggauss_scroll->isChecked() )
         {
            for ( unsigned int i = 0; i < unified_ggaussian_gaussians_size; i++ )
            {
               gauss_add_marker( unified_ggaussian_params[ common_size * i + 0 ], Qt::blue, QString( "%1" ).arg( i + 1 ) );
               gauss_add_hline ( unified_ggaussian_params[ common_size * i + 0 ], unified_ggaussian_params[ common_size * i + 1 ] );
            }
         } else {
            for ( unsigned int i = 0; i < unified_ggaussian_gaussians_size; i++ )
            {
               gauss_add_marker( unified_ggaussian_params[ (vector<double>::size_type) common_size * i ], Qt::blue, QString( "%1" ).arg( i + 1 ) );
            }
         }
      }
   }
      
   if ( !suppress_replot )
   {
#if QT_VERSION >= 0x040000
      legend_set();
#endif
      plot_dist->replot();
   }
}


void US_Hydrodyn_Mals::gauss_delete_markers()
{
#if QT_VERSION < 0x040000
   plot_dist->removeMarkers();
#else
   plot_dist->detachItems( QwtPlotItem::Rtti_PlotMarker );
#endif

#if QT_VERSION >= 0x040000
   set < QwtPlotItem * > current_items;
   {
      const QwtPlotItemList &list = plot_dist->itemList();

      for ( QwtPlotItemIterator it = list.begin(); it != list.end(); ++it ) {
         current_items.insert( *it );
         // QwtPlotItem *item = *it;
         // us_qdebug( QString( "item name %1 addr %2" ).arg( item->title().text() ).arg( (unsigned long) item ) );
      }
   }
#endif


   for ( unsigned int i = 0; i < ( unsigned int )plotted_hlines.size(); i++ )
   {
#if QT_VERSION < 0x040000
      plot_dist->removeCurve( plotted_hlines[ i ] );
#else
      if ( current_items.count( plotted_hlines[ i ] ) ) {
         plotted_hlines[ i ]->detach();
      }
#endif
   }
   plotted_markers.clear( );
   plotted_hlines.clear( );
   for ( unsigned int i = 0; i < ( unsigned int ) plotted_baseline.size(); i++ )
   {
#if QT_VERSION < 0x040000
      plot_dist->removeCurve( plotted_baseline[ i ] );
#else
      if ( current_items.count( plotted_baseline[ i ] ) ) {
         plotted_baseline[ i ]->detach();
      }
#endif
   }
   plotted_baseline.clear( );
}

vector < double > US_Hydrodyn_Mals::gaussian( double * g ) // height, double center, double width )
{
   vector < double > result;

   if ( !f_qs.count( wheel_file ) )
   {
      editor_msg( "red", QString( us_tr( "Internal error: %1 not found in data" ) ).arg( wheel_file ) );
      return result;
   }

   if ( !f_qs[ wheel_file ].size() )
   {
      editor_msg( "red", QString( us_tr( "Internal error: %1 empty data" ) ).arg( wheel_file ) );
      return result;
   }


   vector < double > g_use( gaussian_type_size );
   for ( int i = 0; i < (int) gaussian_type_size; ++i )
   {
      g_use[ i ] = g[ i ];
   }

   return compute_gaussian( f_qs[ wheel_file ], g_use );
}

void US_Hydrodyn_Mals::gauss_delete()
{
   vector < double > new_gaussians;
   for ( unsigned int i = 0; i < ( unsigned int ) gaussians.size() / gaussian_type_size; i++ )
   {
      if ( i != gaussian_pos )
      {
         for ( int k = 0; k < gaussian_type_size; k++ )
         {
            new_gaussians.push_back( gaussians[ k + gaussian_type_size * i ] );
         }
      }
   }
   gaussians = new_gaussians;
   if ( gaussian_pos == ( unsigned int ) gaussians.size() / gaussian_type_size )
   {
      gaussian_pos = ( unsigned int ) gaussians.size() / gaussian_type_size - 1;
   }
   disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
   update_gauss_pos();
   qwtw_wheel->setValue( le_gauss_pos->text().toDouble() );
   gauss_init_markers();
   gauss_init_gaussians();
   gaussian_enables();
}

void US_Hydrodyn_Mals::gauss_replot_gaussian()
{
   if ( gaussian_pos >= ( unsigned int ) plotted_gaussians.size() )
   {
      // editor_msg( "red", QString( us_tr( "Internal error: missing plotted gaussian curve" ) ) );
      return;
   }

   vector < double > x = f_qs[ wheel_file ];
   vector < double > y = gaussian( &(gaussians[ (vector<double>::size_type) gaussian_pos * gaussian_type_size ] ) );

#if QT_VERSION < 0x040000
   plot_dist->setCurveData( plotted_gaussians[ gaussian_pos ],
                            (double *)&x[ 0 ],
                            (double *)&y[ 0 ],
                            x.size()
                            );
#else
   plotted_gaussians[ gaussian_pos ]->setSamples(
                                              (double *)&x[ 0 ],
                                              (double *)&y[ 0 ],
                                              x.size()
                                              );

#endif
   replot_gaussian_sum();
}

void US_Hydrodyn_Mals::gauss_add_gaussian( double *g, QColor color )
   // double height, double center, double width, QColor color )
{
   vector < double > x = f_qs[ wheel_file ];
   vector < double > y = gaussian( g );

#if QT_VERSION < 0x040000
   long curve;
   curve = plot_dist->insertCurve( "gaussian" );
   plot_dist->setCurveStyle( curve, QwtCurve::Lines );
#else
   QwtPlotCurve *curve = new QwtPlotCurve( "gaussian" );
   curve->setStyle( QwtPlotCurve::Lines );
#endif

   plotted_gaussians.push_back( curve );

#if QT_VERSION < 0x040000
   plot_dist->setCurveData( curve, 
                            (double *)&x[ 0 ],
                            (double *)&y[ 0 ],
                            x.size()
                            );
   plot_dist->setCurvePen( curve, QPen( color , use_line_width, Qt::DashLine ) );
#else
   curve->setSamples(
                  (double *)&x[ 0 ],
                  (double *)&y[ 0 ],
                  x.size()
                  );

   curve->setPen( QPen( color, use_line_width, Qt::DashLine ) );
   curve->attach( plot_dist );
#endif
}

void US_Hydrodyn_Mals::gauss_init_gaussians()
{
   gauss_delete_gaussians();
   plotted_gaussians.clear( );
   plotted_gaussian_sum.clear( );

   for ( unsigned int i = 0; i < ( unsigned int ) gaussians.size(); i += gaussian_type_size )
   {
      gauss_add_gaussian( &(gaussians[ i ]), Qt::green );
   }

   plot_gaussian_sum();
   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
}

void US_Hydrodyn_Mals::gauss_delete_gaussians()
{
   for ( unsigned int i = 0; i < ( unsigned int ) plotted_gaussians.size(); i++ )
   {
#if QT_VERSION < 0x040000
      plot_dist->removeCurve( plotted_gaussians[ i ] );
#else
      plotted_gaussians[ i ]->detach();
#endif
   }
   for ( unsigned int i = 0; i < ( unsigned int ) plotted_gaussian_sum.size(); i++ )
   {
#if QT_VERSION < 0x040000
      plot_dist->removeCurve( plotted_gaussian_sum[ i ] );
#else
      plotted_gaussian_sum[ i ]->detach();
#endif
   }
}

void US_Hydrodyn_Mals::gauss_pos_focus( bool hasFocus )
{
   TSO << QString( "gauss_pos_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus && !cb_ggauss_scroll->isChecked() )
   {
      le_last_focus = le_gauss_pos;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      update_gauss_pos();
      double range = f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ];
      double extend = 0.20 * range;
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ] - extend, f_qs[ wheel_file ].back() + extend); qwtw_wheel->setSingleStep( range / UHSH_WHEEL_RES );
      qwtw_wheel->setValue( le_gauss_pos->text().toDouble() );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals::gauss_pos_width_focus( bool hasFocus )
{
   TSO << QString( "gauss_pos_width_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus && !cb_ggauss_scroll->isChecked() )
   {
      le_last_focus = le_gauss_pos_width;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      update_gauss_pos();
      qwtw_wheel->setRange( 0, f_qs[ wheel_file ].back() / 3); qwtw_wheel->setSingleStep( ( f_qs[ wheel_file ].back() / 3 ) / UHSH_WHEEL_RES );
      qwtw_wheel->setValue( le_gauss_pos_width->text().toDouble() );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals::gauss_pos_height_focus( bool hasFocus )
{
   TSO << QString( "gauss_pos_height_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus && !cb_ggauss_scroll->isChecked() )
   {
      le_last_focus = le_gauss_pos_height;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      update_gauss_pos();
      qwtw_wheel->setRange( 0, gauss_max_height); qwtw_wheel->setSingleStep( gauss_max_height / UHSH_WHEEL_RES );
      qwtw_wheel->setValue( le_gauss_pos_height->text().toDouble() );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals::gauss_pos_dist1_focus( bool hasFocus )
{
   TSO << QString( "gauss_pos_dist1_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus && !cb_ggauss_scroll->isChecked() )
   {
      le_last_focus = le_gauss_pos_dist1;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      update_gauss_pos();
      double dist = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_dist_max" ].toDouble();
      qwtw_wheel->setRange( -dist, dist); qwtw_wheel->setSingleStep( 2e0*dist / UHSH_WHEEL_RES );
      qwtw_wheel->setValue( le_gauss_pos_dist1->text().toDouble() );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals::gauss_pos_dist2_focus( bool hasFocus )
{
   TSO << QString( "gauss_pos_dist2_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus && !cb_ggauss_scroll->isChecked() )
   {
      le_last_focus = le_gauss_pos_dist2;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      update_gauss_pos();
      double dist = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_dist_max" ].toDouble();
      qwtw_wheel->setRange( -dist, dist); qwtw_wheel->setSingleStep( 2e0*dist / UHSH_WHEEL_RES );
      qwtw_wheel->setValue( le_gauss_pos_dist2->text().toDouble() );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals::gauss_fit_start_focus( bool hasFocus )
{
   TSO << QString( "gauss_fit_start_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus && !cb_ggauss_scroll->isChecked() )
   {
      le_last_focus = le_gauss_fit_start;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      update_gauss_pos();
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], f_qs[ wheel_file ].back()); qwtw_wheel->setSingleStep( ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_gauss_fit_start->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals::gauss_fit_end_focus( bool hasFocus )
{
   TSO << QString( "gauss_fit_end_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus && !cb_ggauss_scroll->isChecked() )
   {
      le_last_focus = le_gauss_fit_end;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      update_gauss_pos();
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], f_qs[ wheel_file ].back()); qwtw_wheel->setSingleStep( ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_gauss_fit_end->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals::plot_gaussian_sum()
{
   // add up the curve data
   vector < double > x = f_qs[ wheel_file ];
   vector < double > y = x;

   if ( plotted_gaussians.size() )
   {
      for ( unsigned int j = 0; j < x.size(); j++ )
      {
#if QT_VERSION < 0x040000
         y[ j ] = plot_dist->curve( plotted_gaussians[ 0 ] )->sample( j ).y();
#else
         y[ j ] = plotted_gaussians[ 0 ]->sample( j ).y();
#endif
      }

      for ( unsigned int i = 1; i < plotted_gaussians.size(); i++ )
      {
         for ( unsigned int j = 0; j < x.size(); j++ )
         {
#if QT_VERSION < 0x040000
            y[ j ] += plot_dist->curve( plotted_gaussians[ i ] )->sample( j ).y();
#else
            y[ j ] += plotted_gaussians[ i ]->sample( j ).y();
#endif
         }
      }
   } else {
      for ( unsigned int j = 0; j < x.size(); j++ )
      {
         y[ j ] = 0e0;
      }
   }

#if QT_VERSION < 0x040000
   long curve;
   curve = plot_dist->insertCurve( "gaussian_sum" );
   plot_dist->setCurveStyle( curve, QwtCurve::Lines );
#else
   QwtPlotCurve *curve = new QwtPlotCurve( "gaussian_sum" );
   curve->setStyle( QwtPlotCurve::Lines );
#endif

   plotted_gaussian_sum.push_back( curve );

#if QT_VERSION < 0x040000
   plot_dist->setCurveData( curve, 
                            (double *)&x[ 0 ],
                            (double *)&y[ 0 ],
                            x.size()
                            );
   plot_dist->setCurvePen( curve, QPen( Qt::yellow , use_line_width, Qt::DashLine ) );
#else
   curve->setSamples(
                  (double *)&x[ 0 ],
                  (double *)&y[ 0 ],
                  x.size()
                  );

   curve->setPen( QPen( Qt::yellow, use_line_width, Qt::DashLine ) );
   curve->attach( plot_dist );
#endif
   replot_gaussian_sum();
}

void US_Hydrodyn_Mals::replot_gaussian_sum()
{
   // add up the curve data
   if ( !plotted_gaussian_sum.size() )
   {
      editor_msg( "red", QString( us_tr( "Internal error: replot gaussian sum: no gaussian sums" ) ) );
      return;
   }

   vector < double > x = f_qs[ wheel_file ];
   vector < double > y = x;

   if ( plotted_gaussians.size() )
   {
      for ( unsigned int j = 0; j < x.size(); j++ )
      {
#if QT_VERSION < 0x040000
         y[ j ] = plot_dist->curve( plotted_gaussians[ 0 ] )->sample( j ).y();
#else
         y[ j ] = plotted_gaussians[ 0 ]->sample( j ).y();
#endif
      }
   } else {
      for ( unsigned int j = 0; j < x.size(); j++ )
      {
         y[ j ] = 0;
      }
   }
      
   for ( unsigned int i = 1; i < plotted_gaussians.size(); i++ )
   {
      for ( unsigned int j = 0; j < x.size(); j++ )
      {
#if QT_VERSION < 0x040000
         y[ j ] += plot_dist->curve( plotted_gaussians[ i ] )->sample( j ).y();
#else
         y[ j ] += plotted_gaussians[ i ]->sample( j ).y();
#endif
      }
   }

#if QT_VERSION < 0x040000
   plot_dist->setCurveData( plotted_gaussian_sum[ 0 ],
                            (double *)&x[ 0 ],
                            (double *)&y[ 0 ],
                            x.size()
                            );
#else
   plotted_gaussian_sum[ 0 ]->setSamples(
                                      (double *)&x[ 0 ],
                                      (double *)&y[ 0 ],
                                      x.size()
                                      );

#endif
   double rmsd  = 0e0;
   double start = le_gauss_fit_start->text().toDouble();
   double end   = le_gauss_fit_end  ->text().toDouble();
   if ( wheel_errors_ok && cb_sd_weight->isChecked() )
   {
      int used_pts = 0;
      for ( unsigned int j = 0; j < x.size(); j++ )
      {
         if ( x[ j ] >= start && x[ j ] <= end )
         {
            double tmp = ( y[ j ] - f_Is[ wheel_file ][ j ] ) / f_errors[ wheel_file ][ j ];
            rmsd += tmp * tmp;
            ++used_pts;
         }
      }
      double nu = (double)( used_pts - (int) gaussians.size() - 1 );
      if ( nu <= 0e0 )
      {
         nu = (double) used_pts;
         if ( nu <= 0e0 )
         {
            nu = 1e0;
         }
      }
      rmsd /= nu;
   } else {

      for ( unsigned int j = 0; j < x.size(); j++ )
      {
         if ( x[ j ] >= start && x[ j ] <= end )
         {
            rmsd += ( y[ j ] - f_Is[ wheel_file ][ j ] ) * ( y[ j ] - f_Is[ wheel_file ][ j ] );
         }
      }
      rmsd = sqrt( rmsd );
   }

   lbl_gauss_fit->setText( QString( " %1 " ).arg( rmsd, 0, 'g', 5 ) );
   vector < double > empty;
   update_plot_errors( x, y, f_Is[ wheel_file ], f_errors.count( wheel_file ) ? f_errors[ wheel_file ] : empty );
}


void US_Hydrodyn_Mals::gauss_fit()
{
   check_fit_range();

   if ( current_mode == MODE_GGAUSSIAN )
   {
      disable_all();
      double peak;
      get_peak( wheel_file, peak );
      gauss_max_height = peak * 1.2;
      if ( gaussian_type != GAUSS )
      {
         gauss_max_height *= 20e0;
      }

      wheel_enables( false );
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      US_Hydrodyn_Mals_Fit_Global *shfg = 
         new US_Hydrodyn_Mals_Fit_Global(
                                              this,
                                              this );
      US_Hydrodyn::fixWinButtons( shfg );
      shfg->exec();
      delete shfg;
      
      wheel_enables();
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      ggaussian_enables();
      return;
   }

   if ( !plotted_gaussian_sum.size() )
   {
      editor_msg( "red", QString( us_tr( "Internal error: gaussian fit: no gaussian sums" ) ) );
      return;
   }

   if ( !gaussians.size() )
   {
      editor_msg( "red", QString( us_tr( "Internal error: gaussian fit: no gaussians" ) ) );
      return;
   }

   {
      get_peak( wheel_file, gauss_max_height );
      gauss_max_height *= 1.2;
      if ( gaussian_type != GAUSS ) {
         gauss_max_height *= 20e0;
      }
      // qDebug() << "mals::gauss_fit gauss_max_height " << gauss_max_height;
   }

   wheel_enables( false );
   disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
   US_Hydrodyn_Mals_Fit *shf = 
      new US_Hydrodyn_Mals_Fit(
                                    this,
                                    true,
                                    this );
   US_Hydrodyn::fixWinButtons( shf );
   shf->exec();
   delete shf;
   
   cb_gauss_match_amplitude->setChecked( false );
   wheel_enables();
   connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
   return;
   /*   
        TSO << "gauss fit start\n";

        lm_fit_gauss_size = gaussians.size();

        LM::lm_control_struct control = LM::lm_control_double;
        control.printflags = 0; // 3; // monitor status (+1) and parameters (+2)
        control.stepbound  = 100;
        control.maxcall    = 100;

        LM::lm_status_struct status;

        vector < double > x = f_qs[ wheel_file ];
        vector < double > t;
        vector < double > y;

        double start = le_gauss_fit_start->text().toDouble();
        double end   = le_gauss_fit_end  ->text().toDouble();
        for ( unsigned int j = 0; j < x.size(); j++ )
        {
        if ( x[ j ] >= start && x[ j ] <= end )
        {
        t.push_back( x[ j ] );
        y.push_back( f_Is[ wheel_file ][ j ] );
        }
        }

        vector < double > par = gaussians;
        // printvector( QString( "par start" ), par );

        for ( control.epsilon    = 1;
        control.epsilon    > 0.01;
        control.epsilon    /= 2 )
        {
        LM::lmcurve_fit( ( int )      par.size(),
        ( double * ) &( par[ 0 ] ),
        ( int )      t.size(),
        ( double * ) &( t[ 0 ] ),
        ( double * ) &( y[ 0 ] ),
        compute_gaussian_f,
        (const LM::lm_control_struct *)&control,
        &status );

        // printvector( QString( "par is now (norm %1)" ).arg( status.fnorm ), par );
        }
        gaussians = par;
        gauss_init_markers();
        gauss_init_gaussians();
        update_gauss_pos();
   */
}

void US_Hydrodyn_Mals::create_i_of_q()
{
   disable_all();

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         if ( lb_files->item( i )->text() == lbl_conc_file->text() )
         {
            lb_files->item( i)->setSelected( false );
         }
      }
   }
              
   QStringList files = all_selected_files();
   gaussians.size() ? (void) create_i_of_q( files ) : (void) create_i_of_q_ng( files );

   update_enables();
}

vector < double > US_Hydrodyn_Mals::compute_gaussian_sum( vector < double > t, vector < double > g )
{
   vector < double > result( t.size() );
   for ( unsigned int i = 0; i < ( unsigned int ) result.size(); i++ )
   {
      result[ i ] = 0e0;
   }

   vector < double > g_use( gaussian_type_size );
   vector < double > result_partial;
   compute_gaussian_sum_partials.clear( );

   for ( unsigned int j = 0; j < ( unsigned int ) g.size(); j += gaussian_type_size )
   {
      for ( int i = 0; i < (int) gaussian_type_size; ++i )
      {
         g_use[ i ] = g[ i + j ];
      }
      result_partial = compute_gaussian( t, g_use );
      compute_gaussian_sum_partials.push_back( result_partial );
      for ( unsigned int i = 0; i < ( unsigned int ) t.size(); i++ )
      {
         result[ i ] += result_partial[ i ];
      }
   }
   // US_Vector::printvector( QString( "gaussian sum: g" ), g );
   // US_Vector::printvector2( QString( "gaussian sum: type %1 g.size() %2" ).arg( gaussian_type_size ).arg( g.size() ), t, result );
   return result;
}

vector < double > US_Hydrodyn_Mals::compute_gaussian( vector < double > t, vector < double > g )
{
   vector < double > result( t.size() );

   double height = g[ 0 ];
   double center = g[ 1 ];
   double width  = g[ 2 ];

   if ( width == 0e0 )
   {
      width = 1e-3;
   }

   gaussian_types use_gt = gaussian_type;

   double dist1 = dist1_active ? g[ 3 ] : 0e0;
   double dist2 = dist2_active ? g[ 4 ] : 0e0;

   /*
   TSO << QString( "compute_gaussian use_qt is %1 dist1 %2 %3 dist2 %4 %5\n" )
      .arg( use_gt )
      .arg( dist1_active ? "active" : "not active" )
      .arg( dist1 )
      .arg( dist2_active ? "active" : "not active" )
      .arg( dist2 )
      ;
   */

   if ( use_gt == EMGGMG && dist2 == 0e0 )
   {
      use_gt = EMG;
   }
   if ( use_gt == EMG && dist1 == 0e0 )
   {
      use_gt = GAUSS;
   }
   if ( use_gt == EMGGMG && dist1 == 0e0 )
   {
      use_gt = GMG;
      dist1 = dist2;
   }
   if ( use_gt == GMG && dist1 == 0e0 )
   {
      use_gt = GAUSS;
   }

   unsigned int q_size = ( unsigned int ) t.size();
   double * q = &( t[ 0 ] );

   switch( use_gt )
   {
   case GAUSS:
      {
         // TSO << "gaussian as: GAUSS\n";
         double tmp;
         for ( unsigned int i = 0; i < q_size; ++i )
         {
            tmp = ( q[ i ] - center ) / width;
            result[ i ] = height * exp( - tmp * tmp * 5e-1 );
         }
      }
      break;
   case GMG:
      {
         // TSO << "gaussian as: GMG\n";

#if defined( DEBUG_GMG )
         vector < double > q_exparg_org;
         vector < double > q_exparg_new;
         vector < double > q_erfarg_org;
         vector < double > q_erfarg_new;

         vector < double > params_org;
         vector < double > params_new;
#endif

         {
            double area                         = height * width * M_SQRT2PI;
            double one_over_width               = 1e0 / width;
            double one_over_a2sq_plus_a3sq      = 1e0 / ( width * width + dist1 * dist1 );
            double sqrt_one_over_a2sq_plus_a3sq = sqrt( one_over_a2sq_plus_a3sq );
            double gmg_coeff                    = area * M_ONE_OVER_SQRT2PI * sqrt_one_over_a2sq_plus_a3sq;
            double gmg_exp_m1                   = -5e-1 *  one_over_a2sq_plus_a3sq;
            double gmg_erf_m1                   = dist1 * sqrt_one_over_a2sq_plus_a3sq * M_ONE_OVER_SQRT2 * one_over_width;
            
#if defined( DEBUG_GMG )
            params_org.push_back( width );
            params_org.push_back( dist1 );
            params_org.push_back( one_over_width );
            params_org.push_back( one_over_a2sq_plus_a3sq );
            params_org.push_back( sqrt_one_over_a2sq_plus_a3sq );
            params_org.push_back( M_ONE_OVER_SQRT2 );
            params_org.push_back( M_ONE_OVER_SQRT2PI );
            params_org.push_back( gmg_coeff );
#endif

            for ( unsigned int i = 0; i < q_size; ++i )
            {
               double tmp = q[ i ] - center;

#if defined( DEBUG_GMG )
               double exparg = gmg_exp_m1 * tmp * tmp;
               double erfarg = gmg_erf_m1 * tmp;
               q_exparg_org.push_back( exparg );
               q_erfarg_org.push_back( erfarg );
#endif
               result[ i ] = 
                  gmg_coeff * exp( gmg_exp_m1 * tmp * tmp ) *
                  ( 1e0 + use_erf( gmg_erf_m1 * tmp ) );
            }
         }

#if defined( DEBUG_GMG )

         {
            double area                         = height * width * M_SQRT2PI;

            params_new.push_back( width );
            params_new.push_back( dist1 );
            params_new.push_back( 1e0 / width );
            params_new.push_back( 1e0 / ( width * width + dist1 * dist1 ) );
            params_new.push_back( 1e0 / sqrt( width * width + dist1 * dist1 ) );
            params_new.push_back( 1e0 / sqrt( 2e0 ) );
            params_new.push_back( 1 / sqrt( 2e0 * M_PI ) );
            params_new.push_back( area / ( sqrt( 2e0 * M_PI ) * sqrt( width * width + dist1 * dist1 ) ) );
            params_new.push_back( area * (1e0 /  ( sqrt( 2e0 * M_PI ) ) * ( 1e0 /  sqrt( width * width + dist1 * dist1 ) )  ) );

            for ( unsigned int i = 0; i < q_size; ++i )
            {
               double z = ( q[ i ] - center ) / sqrt( width * width + dist1 * dist1 );
               double exparg = -5e-1 * z * z;
               double erfarg = ( dist1 / width ) * z / sqrt( 2e0 );

               q_exparg_new.push_back( exparg );
               q_erfarg_new.push_back( erfarg );


               result[ i ] = 
                  ( area / ( sqrt( 2e0 * M_PI ) * sqrt( width * width + dist1 * dist1 ) ) )
                  * exp( exparg ) * ( 1 + use_erf( erfarg ) );
            }
         }

         US_Vector::printvector2( "exp", q_exparg_org, q_exparg_new );
         US_Vector::printvector2( "erf", q_erfarg_org, q_erfarg_new );
         US_Vector::printvector2( "params", params_org, params_new );
#endif

      }
      break;
   case EMG:
      {
         // TSO << "gaussian as: EMG\n";
         double dist1_thresh      = width / ( 5e0 * sqrt(2e0) - 2e0 );
         if ( fabs( dist1 ) < dist1_thresh )
         {
            double frac_gauss = ( dist1_thresh - fabs( dist1 ) ) / dist1_thresh;
            if ( dist1 < 0 )
            {
               dist1_thresh *= -1e0;
            }

            double area              = height * width * M_SQRT2PI;
            double one_over_a3       = 1e0 / dist1_thresh;
            double emg_coeff         = area * one_over_a3 * 5e-1 * (1e0 - frac_gauss );
            double emg_exp_1         = width * width * one_over_a3 * one_over_a3 * 5e-1;
            double emg_erf_2         = width * M_ONE_OVER_SQRT2 * one_over_a3;
            double sign_a3           = dist1_thresh < 0e0 ? -1e0 : 1e0;
            double one_over_sqrt2_a2 = M_ONE_OVER_SQRT2 / width;
            double gauss_coeff       = frac_gauss * height;

            // printf( "EMG t0 %g thresh %g frac gauss %g\n", dist1, dist1_thresh, frac_gauss );

            for ( unsigned int i = 0; i < q_size; ++i )
            {
               double tmp = q[ i ] - center;
               double tmp2 =  tmp / width;
               
               result[ i ] = 
                  emg_coeff * exp( emg_exp_1 - one_over_a3 * tmp ) *
                  ( use_erf( tmp * one_over_sqrt2_a2 - emg_erf_2 ) + sign_a3 ) +
                  gauss_coeff * exp( - tmp2 * tmp2 * 5e-1 );
            }
         } else {
            double area              = height * width * M_SQRT2PI;
            double one_over_a3       = 1e0 / dist1;
            double emg_coeff         = area * one_over_a3 * 5e-1;
            double emg_exp_1         = width * width * one_over_a3 * one_over_a3 * 5e-1;
            double emg_erf_2         = width * M_ONE_OVER_SQRT2 * one_over_a3;
            double sign_a3           = dist1 < 0e0 ? -1e0 : 1e0;
            double one_over_sqrt2_a2 = M_ONE_OVER_SQRT2 / width;

            for ( unsigned int i = 0; i < q_size; ++i )
            {
               double tmp = q[ i ] - center;
               result[ i ] = 
                  emg_coeff * exp( emg_exp_1 - one_over_a3 * tmp ) *
                  ( use_erf( tmp * one_over_sqrt2_a2 - emg_erf_2 ) + sign_a3 );
            }
         }
      }
      break;
   case EMGGMG:
      {
         // TSO << "gaussian as: EMGGMG\n";
         double area = height * width * M_SQRT2PI;

         // GMG
         double one_over_width               = 1e0 / width;
         double one_over_a2sq_plus_a3sq      = 1e0 / ( width * width + dist2 * dist2 );
         double sqrt_one_over_a2sq_plus_a3sq = sqrt( one_over_a2sq_plus_a3sq );
         double gmg_coeff                    = 5e-1 * area * M_ONE_OVER_SQRT2PI * sqrt_one_over_a2sq_plus_a3sq;
         double gmg_exp_m1                   = -5e-1 *  one_over_a2sq_plus_a3sq;
         double gmg_erf_m1                   = dist2 * sqrt_one_over_a2sq_plus_a3sq * M_ONE_OVER_SQRT2 * one_over_width;

         double dist1_thresh      = width / ( 5e0 * sqrt(2e0) - 2e0 );
         if ( fabs( dist1 ) < dist1_thresh )
         {
            double frac_gauss = ( dist1_thresh - fabs( dist1 ) ) / dist1_thresh;
            if ( dist1 < 0 )
            {
               dist1_thresh *= -1e0;
            }

            double one_over_a3       = 1e0 / dist1_thresh;
            double emg_coeff         = 5e-1 * area * one_over_a3 * 5e-1 * (1e0 - frac_gauss );
            double emg_exp_1         = width * width * one_over_a3 * one_over_a3 * 5e-1;
            double emg_erf_2         = width * M_ONE_OVER_SQRT2 * one_over_a3;
            double sign_a3           = dist1_thresh < 0e0 ? -1e0 : 1e0;
            double one_over_sqrt2_a2 = M_ONE_OVER_SQRT2 / width;
            double gauss_coeff       = 5e-1 * frac_gauss * height;

            // printf( "EMG t0 %g thresh %g frac gauss %g\n", dist1, dist1_thresh, frac_gauss );

            for ( unsigned int i = 0; i < q_size; ++i )
            {
               double tmp = q[ i ] - center;
               double tmp2 =  tmp / width;
               
               result[ i ] = 
                  emg_coeff * exp( emg_exp_1 - one_over_a3 * tmp ) *
                  ( use_erf( tmp * one_over_sqrt2_a2 - emg_erf_2 ) + sign_a3 ) +
                  gauss_coeff * exp( - tmp2 * tmp2 * 5e-1 ) +
                  gmg_coeff * exp( gmg_exp_m1 * tmp * tmp ) *
                  ( 1e0 + use_erf( gmg_erf_m1 * tmp ) );
                  ;
            }
         } else {
            // EMG
            double one_over_a3       = 1e0 / dist1;
            double emg_coeff         = 5e-1 * area * one_over_a3 * 5e-1;
            double emg_exp_1         = width * width * one_over_a3 * one_over_a3 * 5e-1;
            double emg_erf_2         = width * M_ONE_OVER_SQRT2 * one_over_a3;
            double sign_a3           = dist1 < 0e0 ? -1e0 : 1e0;
            double one_over_sqrt2_a2 = M_ONE_OVER_SQRT2 / width;

            for ( unsigned int i = 0; i < q_size; ++i )
            {
               double tmp = q[ i ] - center;
               result[ i ] = 
                  emg_coeff * exp( emg_exp_1 - one_over_a3 * tmp ) *
                  ( use_erf( tmp * one_over_sqrt2_a2 - emg_erf_2 ) + sign_a3 ) +
                  gmg_coeff * exp( gmg_exp_m1 * tmp * tmp ) *
                  ( 1e0 + use_erf( gmg_erf_m1 * tmp ) );
            }
         }
      }
      break;
   }

   return result;
}

double US_Hydrodyn_Mals::compute_gaussian_peak( QString file, vector < double > g )
{
   // TSO << QString( "gaussian peak file %1 current type %2\n" ).arg( file ).arg( gaussian_type );
   vector < double > gs = compute_gaussian_sum( f_qs[ file ], g );

   double start_pos = le_gauss_fit_start->text().toDouble();
   double end_pos   = le_gauss_fit_end  ->text().toDouble();

   double gmax = gs[ 0 ];

   // get 1st
   // bool found_first = false;
   unsigned int i = 0;

   for ( ; i < ( unsigned int ) gs.size(); i++ ) {
      double this_pos = f_qs[ file ][ i ];
      if ( this_pos >= start_pos &&
           this_pos <= end_pos ) {
         // found_first = true;
         gmax = gs[ i ];
         break;
      }
   }


   for (; i < ( unsigned int ) gs.size(); i++ )
   {
      double this_pos = f_qs[ file ][ i ];
      if ( this_pos <= end_pos ) {
         if ( gmax < gs[ i ] ) {
            gmax = gs[ i ];
         }
      }
   }
   return gmax;
}

QString US_Hydrodyn_Mals::pad_zeros( int val, int max ) {
   // N.B. - max is the maximum possible value, not the length !

   unsigned int len = QString( "%1" ).arg( max ).length();
   QString      s   = QString( "%1" ).arg( val );
   while ( (unsigned int) s.length() < len )
   {
      s = "0" + s;
   }

   // TSO << "pad_zeros( " << val << " , " << max << " ) returns '" << s << "'\n";

   return s;
}

QStringList US_Hydrodyn_Mals::all_selected_files()
{
   QStringList files;
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         files << lb_files->item( i )->text();
      }
   }
   if ( files.size() )
   {
      last_selected_file = files[ 0 ];
   } else {
      last_selected_file = "";
   }
   return files;
}

set < QString > US_Hydrodyn_Mals::all_selected_files_set()
{
   set < QString > result;
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         result.insert( lb_files->item( i )->text() );
      }
   }
   return result;
}

QStringList US_Hydrodyn_Mals::all_files()
{
   QStringList files;
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      files << lb_files->item( i )->text();
   }
   return files;
}

map < QString, bool > US_Hydrodyn_Mals::all_files_map()
{
   map < QString, bool > files;
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      files[ lb_files->item( i )->text() ] = true;
   }
   return files;
}

int US_Hydrodyn_Mals::ggaussian_sel_no_gaussian_count() {
   int sel_no_gaussian_count = 0;

   QStringList files = all_selected_files();
   
   for ( int i = 0; i < (int) files.size(); ++i ) {
      if ( !f_gaussians.count( files[i] ) ) {
         ++sel_no_gaussian_count;
      }
   }
   return sel_no_gaussian_count;
}

bool US_Hydrodyn_Mals::ggaussian_compatible( bool check_against_global )
{
   QStringList files = all_selected_files();
   return ggaussian_compatible( files, check_against_global );
}
   
bool US_Hydrodyn_Mals::ggaussian_compatible( set < QString > & selected, bool check_against_global )
{
   QStringList files;
   for ( set < QString >::iterator it = selected.begin();
         it != selected.end(); 
         ++it )
   {
      files << *it;
   }
   return ggaussian_compatible( files, check_against_global );
}
      
bool US_Hydrodyn_Mals::ggaussian_compatible( QStringList & files, bool check_against_global )
{
   // see if all the centers match
   if ( files.size() < 2 )
   {
      return true;
   }

   vector < double > centers;
   vector < double > widths;
   vector < double > dist1s;
   vector < double > dist2s;

   bool any_f_gaussians     = false;
   bool any_non_f_gaussians = false;

   for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
   {
      if ( f_gaussians.count( files[ i ] ) &&
           f_gaussians[ files[ i ] ].size() )
      {
         any_f_gaussians = true;
         if ( !centers.size() )
         {
            for ( unsigned int j = 0; j < f_gaussians[ files[ i ] ].size(); j += gaussian_type_size )
            {
               centers.push_back( f_gaussians[ files[ i ] ][ 1 + j ] );
               widths .push_back( f_gaussians[ files[ i ] ][ 2 + j ] );
               if ( dist1_active )
               {
                  dist1s .push_back( f_gaussians[ files[ i ] ][ 3 + j ] );
                  if ( dist2_active )
                  {
                     dist2s .push_back( f_gaussians[ files[ i ] ][ 4 + j ] );
                  }
               }
            }
         } else {
            vector < double > tmp_centers;
            vector < double > tmp_widths;
            vector < double > tmp_dist1s;
            vector < double > tmp_dist2s;
            for ( unsigned int j = 0; j < f_gaussians[ files[ i ] ].size(); j += gaussian_type_size )
            {
               tmp_centers.push_back( f_gaussians[ files[ i ] ][ 1 + j ] );
               tmp_widths .push_back( f_gaussians[ files[ i ] ][ 2 + j ] );
               if ( dist1_active )
               {
                  tmp_dist1s .push_back( f_gaussians[ files[ i ] ][ 3 + j ] );
                  if ( dist2_active )
                  {
                     tmp_dist2s .push_back( f_gaussians[ files[ i ] ][ 4 + j ] );
                  }
               }
            }
            if ( 
                tmp_centers != centers ||
                ( cb_fix_width->isChecked() && tmp_widths != widths ) ||
                ( dist1_active && cb_fix_dist1->isChecked() && tmp_dist1s != dist1s ) ||
                ( dist2_active && cb_fix_dist2->isChecked() && tmp_dist2s != dist2s ) 
                )
            {
               return false;
            }
         }            
      } else {
         any_non_f_gaussians = true;
      }
   }

   if ( !check_against_global ) 
   { 
      return !any_non_f_gaussians;
   }
      
   if ( any_f_gaussians )
   {
      vector < double > tmp_centers;
      vector < double > tmp_widths;
      vector < double > tmp_dist1s;
      vector < double > tmp_dist2s;
      for ( unsigned int j = 0; j < gaussians.size(); j+= gaussian_type_size )
      {
         tmp_centers.push_back( gaussians[ 1 + j ] );
         tmp_widths .push_back( gaussians[ 2 + j ] );
         if ( dist1_active )
         {
            tmp_dist1s .push_back( gaussians[ 3 + j ] );
            if ( dist2_active )
            {
               tmp_dist2s .push_back( gaussians[ 4 + j ] );
            }
         }
      }
      if ( 
          tmp_centers != centers ||
          ( cb_fix_width->isChecked() && tmp_widths != widths ) ||
          ( dist1_active && cb_fix_dist1->isChecked() && tmp_dist1s != dist1s ) ||
          ( dist2_active && cb_fix_dist2->isChecked() && tmp_dist2s != dist2s ) 
          )
      {
         if ( !any_non_f_gaussians )
         {
            
            switch ( QMessageBox::warning(this, 
                                          windowTitle(),
                                          QString( us_tr( "Please note:\n\n"
                                                       "The file specific Gaussians are internally consistent, "
                                                       "but they do not match the last Gaussians in terms of count %1\n"
                                                       "What would you like to do?\n" ) )
                                          .arg( cb_fix_width->isChecked() ?
                                                ", centers or widths." : " or centers." ),
                                          us_tr( "&Set the last Gaussians to the file specific ones" ), 
                                          us_tr( "&Overwrite the file specific Gaussians with the last Gaussians" ),
                                          QString(),
                                          0, // Stop == button 0
                                          0 // Escape == button 0
                                          ) )
            {
            case 0 : // set the last Gaussians
               gaussians = f_gaussians[ files[ 0 ] ];
               return true;
               break;
            case 1 : // just ignore them
               return false;
               break;
            }
         }
         return false;
      }
   }

   return true;
}


/* old way
   for ( unsigned int i = 0; i < ( unsigned int ) unified_ggaussian_q.size(); i++ )
   {
      double        t     = unified_ggaussian_q          [ i ];
      unsigned int  index = unified_ggaussian_param_index[ i ];

      result[ i ]         = 0;

      if ( cb_fix_width->isChecked() )
      {
         for ( unsigned int j = 0; j < unified_ggaussian_gaussians_size; j++ )
         {
            double center  = unified_ggaussian_params[ 2 * j ];
            double width   = unified_ggaussian_params[ 2 * j + 1 ];
            double height  = unified_ggaussian_params[ index + j + 0 ];

            //          TSO << QString( "for pos %1 t is %2 index %3 gaussian %4 center %5 height %6 width %7\n" )
            //             .arg( i )
            //             .arg( t )
            //             .arg( index )
            //             .arg( j )
            //             .arg( center )
            //             .arg( height )
            //             .arg( width )
            //             ;

            double tmp = ( t - center ) / width;
            result[ i ] += height * exp( - tmp * tmp * 5e-1 );
         }
      
      } else {

         for ( unsigned int j = 0; j < unified_ggaussian_gaussians_size; j++ )
         {
            double center  = unified_ggaussian_params[ j ];
            double height  = unified_ggaussian_params[ index + 2 * j + 0 ];
            double width   = unified_ggaussian_params[ index + 2 * j + 1 ];

            //          TSO << QString( "for pos %1 t is %2 index %3 gaussian %4 center %5 height %6 width %7\n" )
            //             .arg( i )
            //             .arg( t )
            //             .arg( index )
            //             .arg( j )
            //             .arg( center )
            //             .arg( height )
            //             .arg( width )
            //             ;

            double tmp = ( t - center ) / width;
            result[ i ] += height * exp( - tmp * tmp * 5e-1 );
         }
      }
   }
   // printvector( "cggs:", result );

   return result;
}
*/

void US_Hydrodyn_Mals::ggauss_rmsd()
{
   if ( ggauss_recompute() )
   {
      lbl_gauss_fit->setText( QString( "%1" ).arg( ggaussian_rmsd(), 0, 'g', 5 ) );
      pb_ggauss_rmsd->setEnabled( false );
   }
}

void US_Hydrodyn_Mals::add_ggaussian_curve( QString name, vector < double > y )
{
   if ( y.size() != unified_ggaussian_q.size() )
   {
      editor_msg( "red", QString( us_tr( "Internal error: add_ggaussian_curve size %1 but unified ggaussian q size %2" ) ).arg( y.size() ).arg( unified_ggaussian_q.size() ) );
   }

   map < QString, bool > current_files = all_files_map();

   QString use_name = name;

   unsigned int ext = 0;
   while ( current_files.count( use_name ) )
   {
      use_name = name + QString( "-%1" ).arg( ++ext );
   }

   lb_created_files->addItem( use_name );
   lb_created_files->scrollToItem( lb_created_files->item( lb_created_files->count() - 1 ) );
   lb_files->addItem( use_name );
   lb_files->scrollToItem( lb_files->item( lb_files->count() - 1 ) );
   created_files_not_saved[ use_name ] = true;
   
   f_pos       [ use_name ] = f_qs.size();
   f_qs        [ use_name ] = unified_ggaussian_t;
   f_qs_string [ use_name ].clear( );
   for ( unsigned int i = 0; i < ( unsigned int ) unified_ggaussian_t.size(); i++ )
   {
      f_qs_string [ use_name ].push_back( QString( "%1" ).arg( unified_ggaussian_t[ i ] ) );
   }
   f_Is        [ use_name ] = y;
   f_errors    [ use_name ].clear( );
   f_is_time   [ use_name ] = true;
   f_psv       [ use_name ] = 0e0;
   f_I0se      [ use_name ] = 0e0;
   f_conc      [ use_name ] = 0e0;
   {
      vector < double > tmp;
      f_gaussians  [ use_name ] = tmp;
   }
}

void US_Hydrodyn_Mals::ggauss_results()
{
   if ( unified_ggaussian_ok )
   {
      add_ggaussian_curve( us_tr( "joined_target" )   , unified_ggaussian_I );
      add_ggaussian_curve( us_tr( "joined_gaussians" ), compute_ggaussian_gaussian_sum() );
   }
}

void US_Hydrodyn_Mals::dir_pressed()
{
   QString use_dir = le_dir->text();
   ((US_Hydrodyn  *)us_hydrodyn)->select_from_directory_history( use_dir, this );
   QString s = QFileDialog::getExistingDirectory( this , us_tr( "Choose a new base directory" ) , use_dir , QFileDialog::ShowDirsOnly );

   if ( !s.isEmpty() )
   {
      QDir::setCurrent( s );
      le_dir->setText(  QDir::currentPath() );
      ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( s );
   }
}

void US_Hydrodyn_Mals::created_dir_pressed()
{
   QString use_dir = le_dir->text();
   ((US_Hydrodyn  *)us_hydrodyn)->select_from_directory_history( use_dir, this );
   QString s = QFileDialog::getExistingDirectory( this , us_tr( "Choose a new base directory for saving files" ) , use_dir , QFileDialog::ShowDirsOnly );

   if ( !s.isEmpty() )
   {
      le_created_dir->setText( s );
      ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( s );
   }
}

void US_Hydrodyn_Mals::gauss_as_curves()
{
   if ( current_mode == MODE_GAUSSIAN )
   {
      for ( unsigned int i = 0; i < ( unsigned int ) gaussians.size(); i+= gaussian_type_size )
      {
         vector < double > tmp_g( gaussian_type_size );
         for ( int k = 0; k < gaussian_type_size; k++ )
         {
            tmp_g[ k ] = gaussians[ k + i ];
         }
         add_plot( wheel_file + QString( "_pk%1" ).arg( ( i / gaussian_type_size ) + 1 ),
                   f_qs[ wheel_file ],
                   compute_gaussian( f_qs[ wheel_file ], tmp_g ),
                   true,
                   false );
      }
      if ( ( unsigned int ) gaussians.size() / gaussian_type_size > 1 )
      {
         add_plot( wheel_file + QString( "_pksum" ),
                   f_qs[ wheel_file ],
                   compute_gaussian_sum( f_qs[ wheel_file ], gaussians ),
                   true,
                   false );
      }
   } else {
      // ggaussian mode
      for ( unsigned int i = 0; i < ( unsigned int ) unified_ggaussian_files.size(); i++ )
      {
         vector < double > g;
         unsigned int  index = common_size * unified_ggaussian_gaussians_size + i * per_file_size * unified_ggaussian_gaussians_size;

         for ( unsigned int j = 0; j < unified_ggaussian_gaussians_size; j++ )
         {
            vector < double > tmp_g;
            for ( int k = 0; k < gaussian_type_size; ++k )
            {
               if ( is_common[ k ] )
               {
                  g.push_back( unified_ggaussian_params[ offset[ k ] + common_size * j           ] );
               } else {
                  g.push_back( unified_ggaussian_params[ offset[ k ] + per_file_size * j + index ] );
               }
               tmp_g.push_back( g.back() );
            }               
            add_plot( unified_ggaussian_files[ i ] + QString( "_pk%1" ).arg( j + 1 ),
                      f_qs[ unified_ggaussian_files[ i ] ],
                      compute_gaussian( f_qs[ unified_ggaussian_files[ i ] ], tmp_g ),
                      true,
                      false );
         }
         if ( ( unsigned int ) gaussians.size() / gaussian_type_size > 1 )
         {
            add_plot( unified_ggaussian_files[ i ] + QString( "_pksum" ),
                      f_qs[ unified_ggaussian_files[ i ] ],
                      compute_gaussian_sum( f_qs[ unified_ggaussian_files[ i ] ], g ),
                      true,
                      false );
         }
      }
   }
}

void US_Hydrodyn_Mals::set_sd_weight()
{
   if ( current_mode == MODE_GAUSSIAN )
   {
      replot_gaussian_sum();
   } else {
      plot_errors->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_errors->detachItems( QwtPlotItem::Rtti_PlotMarker );;
      lbl_gauss_fit ->setText( "?" );
      if ( current_mode == MODE_GGAUSSIAN )
      {
         pb_ggauss_rmsd->setEnabled( true );
         pb_ggauss_rmsd->setText( QString( us_tr( "Recompute %1" ).arg( cb_sd_weight->isChecked() ? "nChi^2" : "RMSD" ) ) );
         plot_errors->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_errors->detachItems( QwtPlotItem::Rtti_PlotMarker );;
         if ( !suppress_replot )
         {
            plot_errors      ->replot();
         }
         plot_errors_grid  .clear( );
         plot_errors_target.clear( );
         plot_errors_fit   .clear( );
         plot_errors_errors.clear( );
         ggqfit_plot->detachItems( QwtPlotItem::Rtti_PlotCurve ); ggqfit_plot->detachItems( QwtPlotItem::Rtti_PlotMarker );;
         ggqfit_plot->replot();
      }
   }
}

void US_Hydrodyn_Mals::set_fix_width()
{
}


void US_Hydrodyn_Mals::set_fix_dist1()
{
}

void US_Hydrodyn_Mals::set_fix_dist2()
{
}

bool US_Hydrodyn_Mals::check_fit_range()
{
   if ( !gaussians.size() )
   {
      return true;
   }

   double fit_start = le_gauss_fit_start->text().toDouble();
   double fit_end   = le_gauss_fit_end  ->text().toDouble();

   unsigned int count = 0;

   for ( unsigned int i = 0; i < ( unsigned int ) gaussians.size(); i += gaussian_type_size )
   {
      double center = gaussians[ i + 1 ];
      if ( center < fit_start || center > fit_end )
      {
         count++;
      }
   }

   if ( count )
   {
      QMessageBox::warning( this, 
                            this->windowTitle() + us_tr( ": Fit range" ),
                            QString( us_tr( "Warning: %1 of %2 Gaussian centers are outside of the fit range" ) )
                            .arg( count )
                            .arg( gaussians.size() / gaussian_type_size ) );
      return false;
   }
   return true;
}

void US_Hydrodyn_Mals::hide_widgets( vector < QWidget *> widgets, bool hide ) {
   for ( unsigned int i = 0; i < ( unsigned int ) widgets.size(); i++ ) {
      hide || always_hide_widgets.count( widgets[ i ] ) ? widgets[ i ]->hide() : widgets[ i ]->show();
   }
}

void US_Hydrodyn_Mals::save_state()
{
   QString use_dir = QDir::currentPath();
   
   if ( !*saxs_widget )
   {
      // try and activate
      ((US_Hydrodyn *)us_hydrodyn)->pdb_saxs();
      raise();
   }

   // if ( *saxs_widget )
   // {
   if ( cb_lock_dir->isChecked() )
   {
      ((US_Hydrodyn  *)us_hydrodyn)->add_to_directory_history( le_dir->text() );
   }
   ((US_Hydrodyn  *)us_hydrodyn)->select_from_directory_history( use_dir, this );
   raise();
   // }


   QString fn = QFileDialog::getSaveFileName( this , us_tr( "Select a name to save the state" ) , use_dir , "*.dat" );

   if ( fn.isEmpty() )
   {
      return;
   }

   fn.replace( QRegExp( "(|-global-state)\\.(dat|DAT)$" ), "" );
   fn += "-global-state.dat";

   if ( QFile::exists( fn ) )
   {
      fn = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( fn, 0, this );
      raise();
   }

   QFile f( fn );
   if ( !f.open( QIODevice::WriteOnly ) )
   {
      return;
   }

   QTextStream ts( &f );

   // write out all the loaded file names (for each stack element)

   ts << "# US-SOMO Global State file\n";

   if ( detector_uv_conv != 0e0 )
   {
      ts << "# __detector_uv: " << QString( "%1" ).arg( detector_uv_conv, 0, 'g', 8 ) << Qt::endl;
   }
   if ( detector_ri_conv != 0e0 )
   {
      ts << "# __detector_ri: " << QString( "%1" ).arg( detector_ri_conv, 0, 'g', 8 ) << Qt::endl;
   }
   if ( detector_uv )
   {
      ts << "# __detector_uv_set" << Qt::endl;
   } 
   if ( detector_ri )
   {
      ts << "# __detector_ri_set" << Qt::endl;
   } 

   ts << "# __gaussian_type: " << gaussian_type_tag << Qt::endl;

   if ( stack_data.size() )
   {
      for ( unsigned int j = 0; j < ( unsigned int ) stack_data.size(); j++ )
      {
         ts << "# __files\n";

         for ( map < QString, QString >::iterator it = stack_data[ j ].f_name.begin();
               it != stack_data[ j ].f_name.end();
               it++ )
         {
            if ( !it->second.isEmpty() )
            {
               ts << it->second << Qt::endl;
            }
         }

         ts << "# __end\n";
   

         ts << "# __end\n";

         ts << "# __conc_files\n";

         for ( set < QString >::iterator it = stack_data[ j ].conc_files.begin();
               it != stack_data[ j ].conc_files.end();
               it++ )
         {
            ts << *it << Qt::endl;
         }

         ts << "# __end\n";
         
         if ( stack_data[ j ].gaussians.size() )
         {
            ts << "# _gaussians\n";
            ts << QString( "%1 %2\n" )
               .arg( le_gauss_fit_start->text() )
               .arg( le_gauss_fit_end  ->text() )
               ;

            for ( unsigned int i = 0; i < ( unsigned int ) stack_data[ j ].gaussians.size(); i += gaussian_type_size )
            {
               for ( int k = 0; k < gaussian_type_size; ++k )
               {
                  ts << QString( "%1 " ).arg( stack_data[ j ].gaussians[ k + i ], 0, 'g', 10 );
               }
               ts << "\n";
            }
            ts << "# __end\n";
         }

         for ( map < QString, vector < double > >::iterator it = stack_data[ j ].f_gaussians.begin();
               it != stack_data[ j ].f_gaussians.end();
               it++ )
         {
            if ( it->second.size() )
            {
               ts << "# __f_gaussians: " << it->first << Qt::endl;
               for ( unsigned int i = 0; i < ( unsigned int) it->second.size(); i += gaussian_type_size )
               {
                  for ( int k = 0; k < gaussian_type_size; ++k )
                  {
                     ts << QString( "%1 " ).arg( it->second[ k + i ], 0, 'g', 10 );
                  }
                  ts << "\n";
               }
               ts << "# __end\n";
            }
         } 
      }
      ts << "# __push_stack\n";
   }

   ts << "# __files\n";

   for ( map < QString, QString >::iterator it = f_name.begin();
         it != f_name.end();
         it++ )
   {
      if ( !it->second.isEmpty() )
      {
         ts << it->second << Qt::endl;
      }
   }

   ts << "# __end\n";

   ts << "# __conc_files\n";

   for ( set < QString >::iterator it = conc_files.begin();
         it != conc_files.end();
         it++ )
   {
      ts << *it << Qt::endl;
   }

   ts << "# __end\n";
   
   if ( gaussians.size() )
   {
      ts << "# __gaussians\n";
      ts << QString( "%1 %2\n" )
         .arg( le_gauss_fit_start->text() )
         .arg( le_gauss_fit_end  ->text() )
         ;

      for ( unsigned int i = 0; i < ( unsigned int ) gaussians.size(); i += gaussian_type_size )
      {
         for ( int k = 0; k < gaussian_type_size; ++k )
         {
            ts << QString( "%1 " ).arg( gaussians[ k + i ], 0, 'g', 10 );
         }
         ts << "\n";
      }
      ts << "# __end\n";
   }

   for ( map < QString, vector < double > >::iterator it = f_gaussians.begin();
         it != f_gaussians.end();
         it++ )
   {
      ts << "# __f_gaussians: " << it->first << Qt::endl;
      for ( unsigned int i = 0; i < ( unsigned int) it->second.size(); i += gaussian_type_size )
      {
         for ( int k = 0; k < gaussian_type_size; ++k )
         {
            ts << QString( "%1 " ).arg( it->second[ k + i ], 0, 'g', 10 );
         }
         ts << "\n";
      }
      ts << "# __end\n";
   }      

   ts << "# __dir: " << le_dir->text() << Qt::endl;
   if ( cb_lock_dir->isChecked() )
   {
      ts << "# __lock_dir\n";
   }

   ts << "# __created_dir: " << le_created_dir->text() << Qt::endl;

   f.close();
   editor_msg( "blue", QString( us_tr( "State saved in file %1" ) ).arg( fn ) );
}

void US_Hydrodyn_Mals::update_gauss_mode()
{
   // TSO << QString( "update_gauss_mode <%1>\n" ).arg( gaussian_type );
   switch ( gaussian_type )
   {
   case EMGGMG :
      gaussian_type_tag  = "EMG+GMG";
      gaussian_type_size = 5;
      dist1_active = true;
      dist2_active = true;
      pb_gauss_start->setText( " EMG+GMG " );
      // le_gauss_pos_dist1->show();
      // le_gauss_pos_dist2->show();
      // cb_fix_dist1->show();
      // cb_fix_dist2->show();
      break;
   case EMG :
      gaussian_type_tag = "EMG";
      gaussian_type_size = 4;
      dist1_active = true;
      dist2_active = false;
      pb_gauss_start->setText( "EMG" );
      // le_gauss_pos_dist1->show();
      // le_gauss_pos_dist2->hide();
      // cb_fix_dist1->show();
      // cb_fix_dist2->hide();
      break;
   case GMG :
      gaussian_type_tag = "GMG";
      gaussian_type_size = 4;
      dist1_active = true;
      dist2_active = false;
      pb_gauss_start->setText( "GMG" );
      // le_gauss_pos_dist1->show();
      // le_gauss_pos_dist2->hide();
      // cb_fix_dist1->show();
      // cb_fix_dist2->hide();
      break;
   case GAUSS :
      gaussian_type_tag = "Gauss";
      gaussian_type_size = 3;
      dist1_active = false;
      dist2_active = false;
      pb_gauss_start->setText( "Gaussians" );
      // le_gauss_pos_dist1->hide();
      // le_gauss_pos_dist2->hide();
      // cb_fix_dist1->hide();
      // cb_fix_dist2->hide();
      break;
   }

   // int percharwidth = 1 + ( 7 * ( USglobal->config_list.fontSize - 1 ) / 10 );
   // pb_gauss_start->setMaximumWidth( percharwidth * ( pb_gauss_start->text().length() + 3 ) );
}
