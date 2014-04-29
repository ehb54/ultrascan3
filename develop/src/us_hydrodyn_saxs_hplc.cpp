#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_saxs_hplc.h"
#include "../include/us_hydrodyn_saxs_hplc_ciq.h"
#include "../include/us_hydrodyn_saxs_hplc_fit.h"
#include "../include/us_hydrodyn_saxs_hplc_fit_global.h"
#include "../include/us_lm.h"
#ifdef QT4
#include <qwt_scale_engine.h>
#endif
#include <qpalette.h>
//Added by qt3to4:
#include <QLabel>
#include <QMouseEvent>
#include <QCloseEvent>
#include <Q3TextStream>
#include <Q3Frame>

#define JAC_VERSION

#define SLASH QDir::separator()

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

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
//       cout << QString( " %1" ).arg( x[ i ], 0, 'g', 8 );
//    }
//    cout << endl;
// }

US_Hydrodyn_Saxs_Hplc::US_Hydrodyn_Saxs_Hplc(
                                             csv csv1,
                                             void *us_hydrodyn, 
                                             QWidget *p, 
                                             const char *name
                                             ) : Q3Frame(p, name)
{
   this->csv1 = csv1;
   this->us_hydrodyn = us_hydrodyn;

#if defined( JAC_VERSION )
   gaussian_type = GAUSS;
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_gaussian_type" ] = QString( "%1" ).arg( gaussian_type );
#endif

   // fix up possible unconfigured or bad configuration

   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "hplc_line_width" ) )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_line_width" ] = "1";
   }
   use_line_width = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_line_width" ].toUInt();

   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "hplc_bl_linear" ) )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_bl_linear" ] = "true";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "hplc_bl_integral" ) )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_bl_integral" ] = "false";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "hplc_bl_save" ) )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_bl_save" ] = "false";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "hplc_bl_smooth" ) )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_bl_smooth" ] = "10";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "hplc_bl_reps" ) )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_bl_reps" ] = "1";
   }
   if ( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_bl_linear" ] == "true" &&
        ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_bl_integral" ] == "true" )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_bl_integral" ] = "false";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "hplc_zi_window" ) )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_zi_window" ] = "25";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "hplc_discard_it_sd_mult" ) )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_discard_it_sd_mult" ] = "2";
   }
   if ( !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "hplc_cb_discard_it_sd_mult" ) )
   {
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_cb_discard_it_sd_mult" ] = "false";
   }

   gaussian_type = 
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "hplc_gaussian_type" ) ?
      (gaussian_types)( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_gaussian_type" ].toInt() : GAUSS;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setCaption(tr("US-SOMO: SAXS HPLC"));
   order_ascending = false;
   conc_widget     = false;
#ifdef QT4
   legend_vis      = false;
#endif
   usu             = new US_Saxs_Util();

   QDir::setCurrent( ((US_Hydrodyn *)us_hydrodyn)->somo_dir + QDir::separator() + "saxs" );

   saxs_widget = &(((US_Hydrodyn *) us_hydrodyn)->saxs_plot_widget);
   saxs_window = ((US_Hydrodyn *) us_hydrodyn)->saxs_plot_window;
   ((US_Hydrodyn *) us_hydrodyn)->saxs_hplc_widget = true;

   errors_were_on = false;

   plot_dist_zoomer           = (ScrollZoomer *) 0;
   plot_errors_zoomer         = (ScrollZoomer *) 0;

   guinier_plot_zoomer        = (ScrollZoomer *) 0;
   guinier_plot_errors_zoomer = (ScrollZoomer *) 0;
   guinier_plot_rg_zoomer     = (ScrollZoomer *) 0;

   le_last_focus      = (mQLineEdit *) 0;
   
   disable_updates = false;
   plot3d_flag     = false;

   cg_red = USglobal->global_colors.cg_label;
   cg_red.setBrush( QColorGroup::Foreground, QBrush( QColor( "red" ),  Qt::SolidPattern ) );

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

   unsigned int csv_height = 40;
   unsigned int csv_width =  1000;

   // cout << QString("csv size %1 %2\n").arg(csv_height).arg(csv_width);
#if defined(DOES_WORK)
   lb_files        ->setMaximumWidth( 3 * csv_width / 7 );
   lb_created_files->setMaximumWidth( 3 * csv_width / 7 );
   editor          ->setMaximumWidth( 3 * csv_width / 7 );
   plot_dist    ->setMinimumWidth( 2 * csv_width / 3 );
#endif
   // lb_files        ->setMaximumWidth( 2 * csv_width / 5 );
   // lb_created_files->setMaximumWidth( csv_width / 3 );
   // editor          ->setMaximumWidth( csv_width / 3 );
   // lbl_created_dir    ->setMaximumWidth( csv_width / 3 );

   int percharwidth = 1 + ( 7 * ( USglobal->config_list.fontSize - 1 ) / 10 );
   {
      vector < QPushButton * > pbs;
      // pbs.push_back( pb_add_files );
      // pbs.push_back( pb_conc );
      // pbs.push_back( pb_clear_files );

      // pbs.push_back( pb_select_all );
      pbs.push_back( pb_regex_load );
      pbs.push_back( pb_save_state );
      pbs.push_back( pb_movie );
      pbs.push_back( pb_invert );
      // pbs.push_back( pb_adjacent );
      pbs.push_back( pb_select_nth );
      // pbs.push_back( pb_color_rotate );
      // pbs.push_back( pb_to_saxs );
      pbs.push_back( pb_view );
      pbs.push_back( pb_rescale );
      pbs.push_back( pb_normalize );

      // pbs.push_back( pb_avg );
      pbs.push_back( pb_add );
      pbs.push_back( pb_conc_avg );
      // pbs.push_back( pb_select_all_created );
      pbs.push_back( pb_invert_all_created );
      pbs.push_back( pb_adjacent_created );
      pbs.push_back( pb_remove_created );
      pbs.push_back( pb_save_created_csv );
      pbs.push_back( pb_save_created );
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

      pbs.push_back( pb_gauss_start );
      pbs.push_back( pb_p3d );
      pbs.push_back( pb_gauss_clear );
      pbs.push_back( pb_gauss_new );
      pbs.push_back( pb_gauss_delete );
      pbs.push_back( pb_gauss_prev );
      pbs.push_back( pb_gauss_next );
      pbs.push_back( pb_legend );
      pbs.push_back( pb_axis_x );
      pbs.push_back( pb_axis_y );
      pbs.push_back( pb_ggauss_rmsd );
      pbs.push_back( pb_ggauss_start );
      pbs.push_back( pb_gauss_save );
      pbs.push_back( pb_gauss_as_curves );
      pbs.push_back( pb_baseline_start );
      pbs.push_back( pb_baseline_apply );
        
      for ( unsigned int i = 0; i < pbs.size(); i++ )
      {
         pbs[ i ]->setMaximumWidth( percharwidth * ( pbs[ i ]->text().length() + 2 ) );
      }
   }

   // lbl_dir        ->setMaximumWidth( lb_files->width() - cb_lock_dir->width() );
   // lbl_created_dir->setMaximumWidth( lb_files->width() );
   
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

   setGeometry(global_Xpos, global_Ypos, csv_width, 100 + csv_height );

   suppress_replot = false;

   // pb_set_conc_file->setMaximumWidth ( pb_select_all->width() + 10 );
   //    pb_set_hplc->setMaximumWidth ( pb_select_all->width() + 10 );
   //    pb_set_empty ->setMaximumWidth ( pb_select_all->width() + 10 );
   //    pb_set_signal->setMaximumWidth ( pb_select_all->width() + 10 );
   plot_colors.clear();

   // sort by max difference pairs & exclude or simply create a bunch
   // by looping over rgb and excluding difference from background?

   QColor bgc = plot_dist->canvasBackground();

   push_back_color_if_ok( bgc, Qt::yellow );
   push_back_color_if_ok( bgc, Qt::green );
   push_back_color_if_ok( bgc, Qt::cyan );
   push_back_color_if_ok( bgc, Qt::blue );
   push_back_color_if_ok( bgc, Qt::red );
   push_back_color_if_ok( bgc, Qt::magenta );
   push_back_color_if_ok( bgc, Qt::darkYellow );
   push_back_color_if_ok( bgc, Qt::darkGreen );
   push_back_color_if_ok( bgc, Qt::darkCyan );
   push_back_color_if_ok( bgc, Qt::darkBlue );
   push_back_color_if_ok( bgc, Qt::darkRed );
   push_back_color_if_ok( bgc, Qt::darkMagenta );
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
      QDir::separator() + "somo_hplc_default_dctr.dat" ;
   if ( QFile( dctr_file ).exists() )
   {
      load_file( dctr_file );
   }
}

US_Hydrodyn_Saxs_Hplc::~US_Hydrodyn_Saxs_Hplc()
{
   ((US_Hydrodyn *)us_hydrodyn)->saxs_hplc_widget = false;
   delete usu;
}

void US_Hydrodyn_Saxs_Hplc::push_back_color_if_ok( QColor bg, QColor set )
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

void US_Hydrodyn_Saxs_Hplc::color_rotate()
{
   vector < QColor >  new_plot_colors;

   for ( unsigned int i = 1; i < ( unsigned int )plot_colors.size(); i++ )
   {
      new_plot_colors.push_back( plot_colors[ i ] );
   }
   new_plot_colors.push_back( plot_colors[ 0 ] );
   plot_colors = new_plot_colors;
   plot_files();
}

void US_Hydrodyn_Saxs_Hplc::cancel()
{
   close();
}

void US_Hydrodyn_Saxs_Hplc::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_saxs_hplc.html");
}

void US_Hydrodyn_Saxs_Hplc::closeEvent(QCloseEvent *e)
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
                                    tr( "&Close the window anyway" ), 
                                    tr( "&Quit from closing" ), 
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

   {
      QStringList           model_not_saved_list;
      map < QString, bool > model_not_saved_map;

      for ( int i = 0; i < (int)lb_model_files->count(); i++ )
      {
         if ( models_not_saved.count( lb_model_files->text( i ) ) )
         {
            model_not_saved_list << lb_model_files->text( i );
            model_not_saved_map[ lb_model_files->text( i ) ] = true;
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
            qsl << QString( tr( "... and %1 more not listed" ) ).arg( model_not_saved_list.size() - qsl.size() );
         }

         switch ( QMessageBox::warning(this, 
                                       caption() + tr( " Remove Models" ),
                                       QString( tr( "Please note:\n\n"
                                                    "These models were created but not saved as .bead_model files:\n"
                                                    "%1\n\n"
                                                    "What would you like to do?\n" ) )
                                       .arg( qsl.join( "\n" ) ),
                                       tr( "&Save them now" ), 
                                       tr( "&Close the window anyway" ), 
                                       tr( "&Quit from closing" ), 
                                       0, // Stop == button 0
                                       0 // Escape == button 0
                                       ) )
         {
         case 0 : // save them now
            // set the ones listed to selected
            if ( !model_save( model_not_saved_list ) )
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
   }


   ((US_Hydrodyn *)us_hydrodyn)->saxs_hplc_widget = false;
   ((US_Hydrodyn *)us_hydrodyn)->last_saxs_hplc_csv = current_csv();
   if ( conc_widget )
   {
      conc_window->close();
   }

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Saxs_Hplc::clear_display()
{
   editor->clear();
   editor->append("\n\n");
}

void US_Hydrodyn_Saxs_Hplc::update_font()
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

void US_Hydrodyn_Saxs_Hplc::save()
{
   QString fn;
   fn = QFileDialog::getSaveFileName( this , caption() , QString::null , QString::null );
   if(!fn.isEmpty() )
   {
      QString text = editor->text();
      QFile f( fn );
      if ( !f.open( QIODevice::WriteOnly | QIODevice::Text) )
      {
         return;
      }
      Q3TextStream t( &f );
      t << text;
      f.close();
      editor->setModified( false );
      setCaption( fn );
   }
}

void US_Hydrodyn_Saxs_Hplc::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->color();
   editor->setColor(color);
   editor->append(msg);
   editor->setColor(save_color);
   if ( !editor_widgets[ 0 ]->isVisible() && color == "red" && !msg.stripWhiteSpace().isEmpty() )
   {
      lbl_editor->setPalette(QPalette(cg_red, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   }
}

void US_Hydrodyn_Saxs_Hplc::editor_msg_qc( QColor qcolor, QString msg )
{
   QColor save_color = editor->color();
   editor->setColor(qcolor);
   editor->append(msg);
   editor->setColor(save_color);
   if ( !editor_widgets[ 0 ]->isVisible() && qcolor == QColor( "red" ) && !msg.stripWhiteSpace().isEmpty() )
   {
      lbl_editor->setPalette(QPalette(cg_red, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   }
}

bool US_Hydrodyn_Saxs_Hplc::activate_saxs_window()
{
   if ( !*saxs_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->pdb_saxs();
      raise();
      setFocus();
      if ( !*saxs_widget )
      {
         editor_msg("red", tr("Could not activate SAXS window!\n"));
         return false;
      }
   }
   saxs_window = ((US_Hydrodyn *) us_hydrodyn)->saxs_plot_window;
   return true;
}

csv US_Hydrodyn_Saxs_Hplc::current_csv()
{
   csv tmp_csv = csv1;
   
   return tmp_csv;
}
  
void US_Hydrodyn_Saxs_Hplc::clear_files()
{
   disable_all();
   QStringList files = all_selected_files();
   clear_files( files );
   update_enables();
}


void US_Hydrodyn_Saxs_Hplc::remove_files( set < QString > & fileset, bool replot )
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

void US_Hydrodyn_Saxs_Hplc::clear_files( QStringList files, bool quiet )
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
         qsl << QString( tr( "... and %1 more not listed" ) ).arg( created_not_saved_list.size() - qsl.size() );
      }

      switch ( QMessageBox::warning(this, 
                                    caption() + tr( " Remove Files" ),
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

   // remove them now
   for ( int i = lb_files->numRows(); i >= 0; i-- )
   {
      if ( selected_map.count( lb_created_files->text( i ) ) )
      {
         created_files_not_saved.erase( lb_created_files->text( i ) );
         lb_created_files->removeItem( i );
      }
   }

   for ( int i = lb_files->numRows() - 1; i >= 0; i-- )
   {
      if ( selected_map.count( lb_files->text( i ) ) )
      {
         if ( !quiet )
         {
            editor_msg( "black", QString( tr( "Removed %1" ) ).arg( lb_files->text( i ) ) );
         }
         if ( lbl_conc_file->text() == lb_files->text( i ) )
         {
            lbl_conc_file->setText( "" );
         }
         //          if ( lbl_hplc->text() == lb_files->text( i ) )
         //          {
         //             lbl_hplc->setText( "" );
         //          }
         //          if ( lbl_signal->text() == lb_files->text( i ) )
         //          {
         //             lbl_signal->setText( "" );
         //          }
         //          if ( lbl_empty->text() == lb_files->text( i ) )
         //          {
         //             lbl_empty->setText( "" );
         //          }
         f_qs_string.erase( lb_files->text( i ) );
         f_qs       .erase( lb_files->text( i ) );
         f_Is       .erase( lb_files->text( i ) );
         f_errors   .erase( lb_files->text( i ) );
         f_pos      .erase( lb_files->text( i ) );
         f_name     .erase( lb_files->text( i ) );
         f_is_time  .erase( lb_files->text( i ) );
         f_gaussians.erase( lb_files->text( i ) );
         f_psv      .erase( lb_files->text( i ) );
         f_I0se     .erase( lb_files->text( i ) );
         f_conc     .erase( lb_files->text( i ) );
         f_extc     .erase( lb_files->text( i ) );
         f_time     .erase( lb_files->text( i ) );
         lb_files->removeItem( i );
      }
   }

   disable_updates = false;
   plot_files();
   if ( !lb_files->numRows() &&
        plot_dist_zoomer )
   {
      delete plot_dist_zoomer;
      plot_dist_zoomer = (ScrollZoomer *) 0;
   }
   update_csv_conc();
   if ( conc_widget )
   {
      if ( lb_files->numRows() )
      {
         conc_window->refresh( csv_conc );
      } else {
         conc_window->cancel();
      }
   }
}

class hplc_sortable_qstring {
public:
   double       x;
   QString      name;
   bool operator < (const hplc_sortable_qstring& objIn) const
   {
      return x < objIn.x;
   }
};

void US_Hydrodyn_Saxs_Hplc::add_files()
{
   disable_all();
   map < QString, bool > existing_items;
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      existing_items[ lb_files->text( i ) ] = true;
   }

   if ( cb_lock_dir->isChecked() )
   {
      QDir::setCurrent( lbl_dir->text() );
   }

   QString use_dir = QDir::currentDirPath();
   
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
      ((US_Hydrodyn  *)us_hydrodyn)->add_to_directory_history( lbl_dir->text() );
   }
   ((US_Hydrodyn  *)us_hydrodyn)->select_from_directory_history( use_dir, this );
   raise();
   // }

   QStringList filenames = QFileDialog::getOpenFileNames( this , "Add files" , use_dir , "dat files [foxs / other] (*.dat);;"
                                                         "All files (*);;"
                                                         "ssaxs files (*.ssaxs);;"
                                                         
                                                         
                                                         
                                                         "txt files [specify q, I, sigma columns] (*.txt)" );

   
   QStringList add_filenames;
   
   if ( filenames.size() )
   {
      last_load_dir = QFileInfo( filenames[ 0 ] ).dirPath();
      if ( !cb_lock_dir->isChecked() )
      {
         QDir::setCurrent( last_load_dir );
         lbl_dir        ->setText( QDir::currentDirPath() );
         lbl_created_dir->setText( QDir::currentDirPath() + "/produced" ); 
      }
      editor_msg( "black", QString( tr( "loaded from %1:" ) ).arg( last_load_dir ) );
   }

   map < QString, double > found_times;

   // #define DEBUG_LOAD_REORDER

   if ( filenames.size() > 1 )
   {
      bool reorder = true;

      QRegExp rx_cap( "(\\d+)_(\\d+)(\\D|$)" );
      rx_cap.setMinimal( true );

      list < hplc_sortable_qstring > svals;

      QString head = qstring_common_head( filenames, true );
      QString tail = qstring_common_tail( filenames, true );

      bool add_dp = head.contains( QRegExp( "\\d_$" ) );

#ifdef DEBUG_LOAD_REORDER
      qDebug( QString( "sort head <%1> tail <%2>  dp %3 " ).arg( head ).arg( tail ).arg( add_dp ? "yes" : "no" ) );
#endif
      
      set < QString > used;

      for ( int i = 0; i < (int) filenames.size(); ++i )
      {
         QString tmp = filenames[ i ].mid( head.length() );
         tmp = tmp.mid( 0, tmp.length() - tail.length() );
         if ( rx_cap.search( tmp ) != -1 )
         {
#ifdef DEBUG_LOAD_REORDER
            qDebug( QString( "rx_cap search tmp %1 found" ).arg( tmp ) );
#endif
            tmp = rx_cap.cap( 1 ) + "." + rx_cap.cap( 2 );
#ifdef DEBUG_LOAD_REORDER
         } else {
            qDebug( QString( "rx_cap search tmp %1 NOT found" ).arg( tmp ) );
#endif
         }

         if ( add_dp )
         {
            tmp = "0." + tmp;
         }

#ifdef DEBUG_LOAD_REORDER
         qDebug( QString( "tmp is now %1 double is %2" ).arg( tmp ).arg( tmp.toDouble() ) );
#endif

         if ( used.count( tmp ) )
         {
#ifdef DEBUG_LOAD_REORDER
            qDebug( QString( "rx_cap used exit <%1>" ).arg( tmp ) );
#endif
            reorder = false;
            break;
         }
         used.insert( tmp );

         hplc_sortable_qstring sval;
         sval.x     = tmp.toDouble();
         sval.name  = filenames[ i ];
         svals      .push_back( sval );
#ifdef DEBUG_LOAD_REORDER
         qDebug( QString( "sort tmp <%1> xval <%2>" ).arg( tmp ).arg( sval.x ) );
#endif
      }
      if ( reorder )
      {
#ifdef DEBUG_LOAD_REORDER
         qDebug( "reordered" );
#endif
         svals.sort();

         filenames.clear();
         for ( list < hplc_sortable_qstring >::iterator it = svals.begin();
               it != svals.end();
               ++it )
         {
            filenames << it->name;
            found_times[ it->name ] = it->x;
         }
      }
   }

   QString errors;

   for ( int i = 0; i < (int)filenames.size(); i++ )
   {
      ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( filenames[ i ] );

      QString basename = QFileInfo( filenames[ i ] ).baseName( true );
      if ( !existing_items.count( basename ) )
      {
         if ( !load_file( filenames[ i ] ) )
         {
            errors += errormsg + "\n";
         } else {
            editor_msg( "black", QString( tr( "%1" ) ).arg( basename ) );
            add_filenames << basename;
            f_time[ basename ] = found_times[ filenames[ i ] ];
         }
         qApp->processEvents();
      } else {
         errors += QString( tr( "Duplicate name not loaded %1%2" ) ).arg( basename ).arg( errors.isEmpty() ? "" : "\n" );
      }
   }

   if ( errors.isEmpty() )
   {
      editor_msg( "blue", tr( "Files loaded ok" ) );
   } else {
      editor_msg( "red", errors );
   }

   lb_files->insertStringList( add_filenames );

   if ( add_filenames.size() &&
        existing_items.size() )
   {
      lb_files->setBottomItem( existing_items.size() );
   }

   if ( add_filenames.size() && plot_dist_zoomer )
   {
      // we should only do this if the ranges are changed
      plot_dist_zoomer->zoom ( 0 );
      delete plot_dist_zoomer;
      plot_dist_zoomer = (ScrollZoomer *) 0;
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
            // cout << QString( "add file found conc %1 %2\n" ).arg( add_filenames[ i ] ).arg( f_conc[ add_filenames[ i ] ] );
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
            editor_msg( "red", QString( tr( "Internal error: could not set concentrations %1 files loaded" ).arg( add_with_conc.size() ) ) );
         }
      }
   }

   if ( conc_widget )
   {
      conc_window->refresh( csv_conc );
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::add_files( QStringList filenames )
{
   map < QString, bool > existing_items;
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      existing_items[ lb_files->text( i ) ] = true;
   }

   QString use_dir = QDir::currentDirPath();
   
   QStringList add_filenames;
   
   if ( filenames.size() )
   {
      last_load_dir = QFileInfo( filenames[ 0 ] ).dirPath();
      if ( !cb_lock_dir->isChecked() )
      {
         QDir::setCurrent( last_load_dir );
         lbl_dir        ->setText( QDir::currentDirPath() );
         lbl_created_dir->setText( QDir::currentDirPath() + "/produced" ); 
      }
      editor_msg( "black", QString( tr( "loaded from %1:" ) ).arg( last_load_dir ) );
   }

   QString errors;

   for ( int i = 0; i < (int)filenames.size(); i++ )
   {
      if ( *saxs_widget )
      {
         saxs_window->add_to_directory_history( filenames[ i ] );
      }

      QString basename = QFileInfo( filenames[ i ] ).baseName( true );
      if ( !existing_items.count( basename ) )
      {
         if ( !load_file( filenames[ i ] ) )
         {
            errors += errormsg + "\n";
         } else {
            editor_msg( "black", QString( tr( "%1" ) ).arg( basename ) );
            add_filenames << basename;
         }
         qApp->processEvents();
      } else {
         errors += QString( tr( "Duplicate name not loaded %1%2" ) ).arg( basename ).arg( errors.isEmpty() ? "" : "\n" );
      }
   }

   if ( errors.isEmpty() )
   {
      editor_msg( "blue", tr( "Files loaded ok" ) );
   } else {
      editor_msg( "red", errors );
   }

   lb_files->insertStringList( add_filenames );

   if ( add_filenames.size() &&
        existing_items.size() )
   {
      lb_files->setBottomItem( existing_items.size() );
   }

   if ( add_filenames.size() && plot_dist_zoomer )
   {
      // we should only do this if the ranges are changed
      plot_dist_zoomer->zoom ( 0 );
      delete plot_dist_zoomer;
      plot_dist_zoomer = (ScrollZoomer *) 0;
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
            // cout << QString( "add file found conc %1 %2\n" ).arg( add_filenames[ i ] ).arg( f_conc[ add_filenames[ i ] ] );
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
            editor_msg( "red", QString( tr( "Internal error: could not set concentrations %1 files loaded" ).arg( add_with_conc.size() ) ) );
         }
      }
   }

   if ( conc_widget )
   {
      conc_window->refresh( csv_conc );
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::plot_files()
{
   // puts( "plot_files" );
   plot_dist->clear();
   //bool any_selected = false;
   double minx = 0e0;
   double maxx = 1e0;
   double miny = 0e0;
   double maxy = 1e0;

   double file_minx;
   double file_maxx;
   double file_miny;
   double file_maxy;
   
   bool first = true;

   plotted_curves.clear();

   int asfs = (int) all_selected_files().size();

   if ( asfs > 20 &&
#ifndef QT4
        plot_dist->autoLegend() 
#else
        legend_vis
#endif
        )
   {
      legend();
   }

   if ( asfs == 1 )
   {
      update_ref();
   }

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         //any_selected = true;
         if ( plot_file( lb_files->text( i ), file_minx, file_maxx, file_miny, file_maxy ) )
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
         if ( get_min_max( lb_files->text( i ), file_minx, file_maxx, file_miny, file_maxy ) )
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

   // cout << QString( "plot range x [%1:%2] y [%3:%4]\n" ).arg(minx).arg(maxx).arg(miny).arg(maxy);

   // enable zooming
   
   if ( !plot_dist_zoomer )
   {
      // puts( "redoing zoomer" );
      plot_dist->setAxisScale( QwtPlot::xBottom, minx, maxx );
      plot_dist->setAxisScale( QwtPlot::yLeft  , miny * 0.9e0 , maxy * 1.1e0 );
      plot_dist_zoomer = new ScrollZoomer(plot_dist->canvas());
      plot_dist_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
#ifndef QT4
      plot_dist_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
      connect( plot_dist_zoomer, SIGNAL( zoomed( const QwtDoubleRect & ) ), SLOT( plot_zoomed( const QwtDoubleRect & ) ) );
   }
   
   legend_set();
   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
}

bool US_Hydrodyn_Saxs_Hplc::plot_file( QString file,
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
      editor_msg( "red", QString( tr( "Internal error: request to plot %1, but not found in data" ) ).arg( file ) );
      return false;
   }

   get_min_max( file, minx, maxx, miny, maxy );

#ifndef QT4
   long Iq = plot_dist->insertCurve( file );
   plotted_curves[ file ] = Iq;
   plot_dist->setCurveStyle( Iq, QwtCurve::Lines );
#else
   QwtPlotCurve *curve = new QwtPlotCurve( file );
   plotted_curves[ file ] = curve;
   curve->setStyle( QwtPlotCurve::Lines );
#endif

   unsigned int q_points = f_qs[ file ].size();

   if ( !axis_y_log )
   {
#ifndef QT4
      plot_dist->setCurveData( Iq, 
                               /* cb_guinier->isChecked() ? (double *)&(plotted_q2[p][0]) : */
                               (double *)&( f_qs[ file ][ 0 ] ),
                               (double *)&( f_Is[ file ][ 0 ] ),
                               q_points
                               );
      plot_dist->setCurvePen( Iq, QPen( plot_colors[ f_pos[ file ] % plot_colors.size()], use_line_width, SolidLine));
#else
      curve->setData(
                     /* cb_guinier->isChecked() ?
                        (double *)&(plotted_q2[p][0]) : */
                     (double *)&( f_qs[ file ][ 0 ] ),
                     (double *)&( f_Is[ file ][ 0 ] ),
                     q_points
                     );

      curve->setPen( QPen( plot_colors[ f_pos[ file ] % plot_colors.size() ], use_line_width, Qt::SolidLine ) );
      curve->attach( plot_dist );
#endif
   } else {
      vector < double > q;
      vector < double > I;
      for ( unsigned int i = 0; i < q_points; i++ )
      {
         if ( f_Is[ file ][ i ] > 0e0 )
         {
            q.push_back( f_qs[ file ][ i ] );
            I.push_back( f_Is[ file ][ i ] );
         }
      }
      q_points = ( unsigned int )q.size();
#ifndef QT4
      plot_dist->setCurveData( Iq, 
                               /* cb_guinier->isChecked() ? (double *)&(plotted_q2[p][0]) : */
                               (double *)&( q[ 0 ] ),
                               (double *)&( I[ 0 ] ),
                               q_points
                               );
      plot_dist->setCurvePen( Iq, QPen( plot_colors[ f_pos[ file ] % plot_colors.size()], use_line_width, SolidLine));
#else
      curve->setData(
                     /* cb_guinier->isChecked() ?
                        (double *)&(plotted_q2[p][0]) : */
                     (double *)&( q[ 0 ] ),
                     (double *)&( I[ 0 ] ),
                     q_points
                     );

      curve->setPen( QPen( plot_colors[ f_pos[ file ] % plot_colors.size() ], use_line_width, Qt::SolidLine ) );
      curve->attach( plot_dist );
#endif
   }
            
   return true;
}

void US_Hydrodyn_Saxs_Hplc::update_files()
{
   if ( !disable_updates )
   {
      plot_files();
      update_enables();
   }
}

bool US_Hydrodyn_Saxs_Hplc::get_min_max( QString file,
                                         double &minx,
                                         double &maxx,
                                         double &miny,
                                         double &maxy )
{
   if ( current_mode == MODE_SCALE )
   {
      //    qDebug( QString("get min max mode scale %1" ).arg( file ) );
      if ( !scale_q     .count( file ) ||
           !scale_I     .count( file ) ||
           !f_pos       .count( file ) )
      {
         // editor_msg( "red", QString( tr( "Internal error: requested %1, but not found in data" ) ).arg( file ) );
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
      // qDebug( QString("get min max mode scale %1 x %2:%3 x %4:%5" ).arg( file ).arg( minx ).arg( maxx ).arg( miny ).arg( maxy ) );
   } else {
      if ( !f_qs_string .count( file ) ||
           !f_qs        .count( file ) ||
           !f_Is        .count( file ) ||
           !f_pos       .count( file ) )
      {
         // editor_msg( "red", QString( tr( "Internal error: requested %1, but not found in data" ) ).arg( file ) );
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

void US_Hydrodyn_Saxs_Hplc::invert()
{
   lb_files->invertSelection();
}

void US_Hydrodyn_Saxs_Hplc::select_all()
{
   bool all_selected = true;
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( !lb_files->isSelected( i ) )
      {
         all_selected = false;
         break;
      }
   }

   disable_updates = true;
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      lb_files->setSelected( i, !all_selected );
   }
   disable_updates = false;
   plot_files();
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::select_all_created()
{
   bool all_selected = true;
   for ( int i = 0; i < lb_created_files->numRows(); i++ )
   {
      if ( !lb_created_files->isSelected( i ) )
      {
         all_selected = false;
         break;
      }
   }

   disable_updates = true;
   for ( int i = 0; i < lb_created_files->numRows(); i++ )
   {
      lb_created_files->setSelected( i, !all_selected );
   }
   disable_updates = false;
   update_enables();
}

bool US_Hydrodyn_Saxs_Hplc::load_file( QString filename )
{
   errormsg = "";
   QFile f( filename );
   if ( !f.exists() )
   {
      errormsg = QString("Error: %1 does not exist").arg( filename );
      return false;
   }
   // cout << QString( "opening %1\n" ).arg( filename ) << flush;
   
   QString ext = QFileInfo( filename ).extension( false ).lower();

   QRegExp rx_valid_ext (
                         "^("
                         "dat|"
                         "int|"
                         "txt|"
                         "csv|"
                         // "out|"
                         "ssaxs)$" );

   if ( rx_valid_ext.search( ext ) == -1 )
   {
      errormsg = QString("Error: %1 unsupported file extension %2").arg( filename ).arg( ext );
      return false;
   }
      
   if ( !f.open( QIODevice::ReadOnly ) )
   {
      errormsg = QString("Error: can not open %1, check permissions ").arg( filename );
      return false;
   }

   Q3TextStream ts(&f);
   vector < QString > qv;
   QStringList qsl;

   while ( !ts.atEnd() )
   {
      QString qs = ts.readLine();
      qv.push_back( qs );

      qsl << qs;
   }
   f.close();

   if ( !qv.size() )
   {
      errormsg = QString("Error: the file %1 is empty ").arg( filename );
      return false;
   }

   bool is_time = false;

   double this_conc = 0e0;
   double this_psv  = 0e0;
   double this_I0se = 0e0;
   bool   has_time = false;
   double this_time = 0e0;

   if ( ext == "dat" )
   {
      QRegExp rx_conc      ( "Conc:\\s*(\\S+)(\\s|$)" );
      QRegExp rx_psv       ( "PSV:\\s*(\\S+)(\\s|$)" );
      QRegExp rx_I0se      ( "I0se:\\s*(\\S+)(\\s|$)" );
      QRegExp rx_time      ( "Time:\\s*(\\S+)(\\s|$)" );
      if ( rx_conc.search( qv[ 0 ] ) )
      {
         this_conc = rx_conc.cap( 1 ).toDouble();
         // cout << QString( "found conc %1\n" ).arg( this_conc );
      }
      if ( rx_psv.search( qv[ 0 ] ) )
      {
         this_psv = rx_psv.cap( 1 ).toDouble();
      }
      if ( rx_I0se.search( qv[ 0 ] ) )
      {
         this_I0se = rx_I0se.cap( 1 ).toDouble();
      }
      if ( rx_time.search( qv[ 0 ] ) )
      {
         has_time = true;
         this_time = rx_time.cap( 1 ).toDouble();
      }
   }

   // we should make some configuration for matches & offsets or column mapping
   // just an ad-hoc fix for APS 5IDD
   int q_offset   = 0;
   int I_offset   = 1;
   int e_offset   = 2;
   int row_offset = 1;
   if ( ext == "dat" && qv[ 0 ].lower().contains( QRegExp( "frame\\s*data" ) ) )
   {
      is_time = true;
   }

   if ( ext == "dat" && qv[ 0 ].contains( " Detector State file" ) )
   {
      QRegExp rx_uv         ( "^# __detector_uv: (\\S+)\\s*$" );
      QRegExp rx_ri         ( "^# __detector_ri: (\\S+)\\s*$" );
      QRegExp rx_uv_set     ( "^# __detector_uv_set\\s*$" );
      QRegExp rx_ri_set     ( "^# __detector_ri_set\\s*$" );
      for ( int i = 1; i < (int) qv.size(); i++ )
      {
         if ( rx_uv.search( qv[ i ] ) != -1 )
         {
            detector_uv_conv = rx_uv.cap( 1 ).toDouble();
            continue;
         }
         if ( rx_ri.search( qv[ i ] ) != -1 )
         {
            detector_ri_conv = rx_ri.cap( 1 ).toDouble();
            continue;
         }
         if ( rx_uv_set.search( qv[ i ] ) != -1 )
         {
            detector_uv = true;
            detector_ri = false;
            continue;
         }
         if ( rx_ri_set.search( qv[ i ] ) != -1 )
         {
            detector_ri = true;
            detector_uv = false;
            continue;
         }
         errormsg = QString( tr( "Error: loading %1 line %2 unrecognied directive %3" ) ).arg( filename ).arg( i + 1 ).arg( qv[ i ] );
         return false;
      }
      errormsg = "";
      return false;
   }

   // load csv columns as time curves
   if ( ext == "csv" )
   {
      editor_msg( "black", QString( tr( "%1" ) ).arg( filename ) );

      // first column is time
      qv[ 0 ].replace( "(", "" ).replace( ")", "" ).replace( "/", "_per_" ).replace( QRegExp( "\\s+" ), "_" ).replace( ":", "_" ).replace( QRegExp( "\\_+" ), "_" ) ;

      QStringList headers = QStringList::split( ",", qv[ 0 ] );
      
      if ( !headers.size() ||
           !headers[ 0 ].lower().contains( "time" ) )
      {
         errormsg = tr( "The first line, first column of the .csv file must contain 'time'" );
         return false;
      }

      vector < vector < double > > csv_data;
      vector < double > q;
      vector < QString > q_string;
      for ( int i = 1; i < (int) qv.size(); i++ )
      {
         QStringList data = QStringList::split( ",", qv[ i ] );
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

         // cout << QString( "curve %1 sizes: I %2 q %3 qs %4\n" ).arg( name ).arg( I.size() ).arg( use_q.size() ).arg( use_q_string.size() );
         // US_Vector::printvector2( "q, I", use_q, I );

         if ( I.size() )
         {
            lb_files->insertItem( name );
            lb_files->setBottomItem( lb_files->numRows() - 1 );
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
            editor_msg( "red", QString( tr( "csv file %1 column %2 \"%3\" doesn't seem to be complete, skipped" ) ).arg( filename ).arg( i + 1 ).arg( name ) );
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
         if ( rx_gaussian_type.search( qv[ i ] ) != -1 )
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
               unified_ggaussian_ok = false;
               f_gaussians.clear();
               gaussians.clear();
               org_gaussians.clear();
               org_f_gaussians.clear();
               update_gauss_mode();
               ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_gaussian_type" ] = QString( "%1" ).arg( gaussian_type );
            }
            defined_gaussian_type = true;
            continue;
         }

         if ( !defined_gaussian_type &&
              ( rx_gaussians.search( qv[ i ] ) != -1 ||
                rx_f_gaussians.search( qv[ i ] ) != -1 ) )
         {
            editor_msg( "dark red", tr( "Notice: this state file does not have a specific Gaussian type defined, defaulting to standard Gaussians" ) );
            gaussian_types new_g = default_gaussian_type;
            if ( gaussian_type != new_g )
            {
               gaussian_type = new_g;
               unified_ggaussian_ok = false;
               f_gaussians.clear();
               gaussians.clear();
               org_gaussians.clear();
               org_f_gaussians.clear();
               update_gauss_mode();
               ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_gaussian_type" ] = QString( "%1" ).arg( gaussian_type );
            }
            defined_gaussian_type = true;
         }            

         if ( rx_uv.search( qv[ i ] ) != -1 )
         {
            detector_uv_conv = rx_uv.cap( 1 ).toDouble();
            continue;
         }
         if ( rx_ri.search( qv[ i ] ) != -1 )
         {
            detector_ri_conv = rx_ri.cap( 1 ).toDouble();
            continue;
         }
         if ( rx_uv_set.search( qv[ i ] ) != -1 )
         {
            detector_uv = true;
            detector_ri = false;
            continue;
         }
         if ( rx_ri_set.search( qv[ i ] ) != -1 )
         {
            detector_ri = true;
            detector_uv = false;
            continue;
         }
         if ( rx_dir.search( qv[ i ] ) != -1 )
         {
            lbl_dir->setText( rx_dir.cap( 1 ) );
            continue;
         }
         if ( rx_lock_dir.search( qv[ i ] ) != -1 )
         {
            cb_lock_dir->setChecked( true );
            continue;
         }
         if ( rx_created_dir.search( qv[ i ] ) != -1 )
         {
            lbl_created_dir->setText( rx_created_dir.cap( 1 ) );
            continue;
         }
         if ( rx_files.search( qv[ i ] ) != -1 )
         {
            i++;
            QStringList files;
            for ( ; i < ( int ) qv.size(); i++ )
            {
               if ( rx_end.search( qv[ i ] ) == -1 )
               {
                  files << qv[ i ];
               } else {
                  break;
               }
            }
            if ( i < ( int ) qv.size() && rx_end.search( qv[ i ] ) != -1 )
            {
               if ( files.size() )
               {
                  add_files( files );
                  disable_all();
               }
               continue;
            }
            errormsg = QString( tr( "Error: loading %1 line %2 unterminated file list" ) ).arg( filename ).arg( i + 1 );
            return false;
         }
         if ( rx_gaussians.search( qv[ i ] ) != -1 )
         {
            i++;
            if ( i >= ( int ) qv.size() )
            {
               errormsg = QString( tr( "Error: loading %1 line %2 unterminated file list" ) ).arg( filename ).arg( i + 1 );
               return false;
            }               
            
            gaussians.clear();

            QStringList tokens = QStringList::split(QRegExp("\\s+"), qv[i].replace(QRegExp("^\\s+"),""));

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
               if ( rx_end.search( qv[ i ] ) == -1 )
               {
                  tokens = QStringList::split(QRegExp("\\s+"), qv[i].replace(QRegExp("^\\s+"),""));
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
            if ( i < ( int ) qv.size() && rx_end.search( qv[ i ] ) != -1 )
            {
               continue;
            }
            errormsg = QString( tr( "Error: loading %1 line %2 unterminated file list" ) ).arg( filename ).arg( i + 1 );
            return false;

         }

         if ( rx_f_gaussians.search( qv[ i ] ) != -1 )
         {
            i++;
            if ( i >= ( int ) qv.size() )
            {
               errormsg = QString( tr( "Error: loading %1 line %2 unterminated file list" ) ).arg( filename ).arg( i + 1 );
               return false;
            }               

            vector < double > tmp_g;

            for ( ; i < ( int ) qv.size(); i++ )
            {
               if ( rx_end.search( qv[ i ] ) == -1 )
               {
                  QStringList tokens = QStringList::split(QRegExp("\\s+"), qv[i].replace(QRegExp("^\\s+"),""));
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
            if ( i < ( int ) qv.size() && rx_end.search( qv[ i ] ) != -1 )
            {
               f_gaussians[ rx_f_gaussians.cap( 1 ) ] = tmp_g;
               continue;
            }
            errormsg = QString( tr( "Error: loading %1 line %2 unterminated file list" ) ).arg( filename ).arg( i + 1 );
            return false;
         }

         if ( rx_push.search( qv[ i ] ) != -1 )
         {
            stack_push_all();
            disable_all();
            clear_files( all_files() );
            continue;
         }

         errormsg = QString( tr( "Error: loading %1 line %2 unrecognied directive %3" ) ).arg( filename ).arg( i + 1 ).arg( qv[ i ] );
         return false;
      }
      
      errormsg = "";
      return false;
   }

   if ( ext == "dat" && qv[ 0 ].contains( " Gaussians" ) )
   {
      gaussians.clear();
      int i = 1;
      QStringList tokens = QStringList::split(QRegExp("\\s+"), qv[i].replace(QRegExp("^\\s+"),""));

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


      if ( qv[ 0 ].contains( "Multiple Gaussians" ) )
      {
         cout << "multiple gaussians\n";

         QString           this_gaussian;
         QRegExp           rx_gname( "^Gaussians (.*)$" ); 
         vector < double > g;
         unsigned int      loaded  = 0;
         unsigned int      skipped = 0;

         for ( i = 2; i < (int) qv.size(); i++ )
         {
            if ( rx_gname.search( qv[ i ] ) != -1 )
            {
               cout << QString( "mg: found %1\n" ).arg( rx_gname.cap( 1 ) );
               // new file specific gaussian
               if ( g.size() && !this_gaussian.isEmpty() )
               {
                  f_gaussians[ this_gaussian ] = g;
                  gaussians = g;
                  loaded++;
               }
               g.clear();
               this_gaussian = rx_gname.cap( 1 );
               if ( !f_qs.count( this_gaussian ) )
               {
                  skipped++;
                  editor_msg( "red", QString( tr( "Gaussians for file %1 present but the file is not loaded, please load the file first" ) ).arg( this_gaussian ) );
                  this_gaussian = "";
               }
               continue;
            }

            if ( !this_gaussian.isEmpty() )
            {
               tokens = QStringList::split(QRegExp("\\s+"), qv[i].replace(QRegExp("^\\s+"),""));
         
               if ( (int) tokens.size() != gaussian_type_size )
               {
                  errormsg = QString("Error: Multiple Gaussian file %1 incorrect format line %2").arg( filename ).arg( i + 1 );
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
            loaded++;
            g.clear();
         }
         editor_msg( "black", QString( "Gaussians for %1 files loaded from %2" ).arg( loaded ).arg( filename ) );
         if ( skipped )
         {
            editor_msg( "red" , QString( "WARNING: Gaussians for %1 files SKIPPED from %2" ).arg( skipped ).arg( filename ) );
         }
      } else {
         for ( i = 2; i < (int) qv.size(); i++ )
         {
            tokens = QStringList::split(QRegExp("\\s+"), qv[i].replace(QRegExp("^\\s+"),""));
         
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
      editor_msg( "dark blue", tr( "APS SIDD format" ) );
   }      
   
   // ad-hoc for soleil hplc time/uv/conc data

   // cout << "load: <" << qv[ 0 ] << ">" << endl;

   if ( ext == "txt" && qv[ 0 ].contains( "temps depuis le debut" ) )
   {
      I_offset   = 2;
      e_offset   = 10;
      row_offset = 4;
      is_time    = true;
      editor_msg( "dark blue", tr( "SOLEIL HPLC time/uv format" ) );
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
           rx_ok_line.search( qv[i] ) == -1 )
      {
         continue;
      }
      
      // cout << "line: <" << qv[ i ] << ">" << endl;

      QStringList tokens = QStringList::split(QRegExp("\\s+"), qv[i].replace(QRegExp("^\\s+"),""));

      if ( (int)tokens.size() > I_offset )
      {
         QString this_q_string = tokens[ q_offset ];
         double this_q         = tokens[ q_offset ].toDouble();
         double this_I         = tokens[ I_offset ].toDouble();
         double this_e = 0e0;
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
            cout << QString(" breaking %1 %2\n").arg( this_q ).arg( q[ q.size() - 1 ] );
            break;
         }
         if ( is_time || this_I != 0e0 )
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
      editor_msg( "red", QString( tr( "Error: File %1 has no data" ) ).arg( filename ) );
      return false;
   }
                  
   if ( is_zero_vector( I ) )
   {
      editor_msg( "red", QString( tr( "Error: File %1 has only zero signal" ) ).arg( filename ) );
      return false;
   }

   // cout << QString( "opened %1\n" ).arg( filename ) << flush;
   QString basename = QFileInfo( filename ).baseName( true );
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
                        QString( tr( "Notice: File %1 appeared to have standard deviations, but some were zero or less, so all were dropped\n" ) 
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
   f_conc       [ basename ] = this_conc;
   f_psv        [ basename ] = this_psv;
   f_I0se       [ basename ] = this_I0se;
   if ( has_time )
   {
      f_time       [ basename ] = this_time;
   }
   return true;
}

void US_Hydrodyn_Saxs_Hplc::set_conc_file()
{
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         lbl_conc_file->setText( lb_files->text( i ) );
      }
   }
   update_csv_conc();
   if ( conc_widget )
   {
      conc_window->refresh( csv_conc );
   }
   plot_ref->clear();

   if ( f_qs.count( lbl_conc_file->text() ) )
   {
#ifndef QT4
      long curve;
      curve = plot_ref->insertCurve( "concentration" );
      plot_ref->setCurveStyle( curve, QwtCurve::Lines );
#else
      QwtPlotCurve *curve = new QwtPlotCurve( "concentration" );
      curve->setStyle( QwtPlotCurve::Lines );
#endif

#ifndef QT4
      plot_ref->setCurvePen( curve, QPen( plot_colors[ 0 ], use_line_width, Qt::SolidLine ) );
      plot_ref->setCurveData( curve,
                              (double *)&f_qs[ lbl_conc_file->text() ][ 0 ],
                              (double *)&f_Is[ lbl_conc_file->text() ][ 0 ],
                              f_qs[ lbl_conc_file->text() ].size()
                              );
#else
      curve->setPen( QPen( plot_colors[ 0 ], use_line_width, Qt::SolidLine ) );
      curve->setData(
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

void US_Hydrodyn_Saxs_Hplc::set_hplc()
{
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         lbl_hplc->setText( lb_files->text( i ) );
      }
   }
   update_csv_conc();
   if ( conc_widget )
   {
      conc_window->refresh( csv_conc );
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::set_signal()
{
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         lbl_signal->setText( lb_files->text( i ) );
      }
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::set_empty()
{
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         lbl_empty->setText( lb_files->text( i ) );
      }
   }
   update_csv_conc();
   if ( conc_widget )
   {
      conc_window->refresh( csv_conc );
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::update_created_files()
{
   if ( !disable_updates )
   {
      update_enables();
   }
}

void US_Hydrodyn_Saxs_Hplc::avg()
{
   // QStringList files;
   //    for ( int i = 0; i < lb_files->numRows(); i++ )
   //    {
   //       if ( lb_files->isSelected( i ) 
   //            //            && 
   //            //            lb_files->text( i ) != lbl_hplc->text() &&
   //            //            lb_files->text( i ) != lbl_empty->text() 
   //            )
   //       {
   //          files << lb_files->text( i );
   //       }
   //    }
   avg( all_selected_files() );
}

QString US_Hydrodyn_Saxs_Hplc::qstring_common_head( QStringList qsl, bool strip_digits )
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

QString US_Hydrodyn_Saxs_Hplc::qstring_common_tail( QStringList qsl, bool strip_digits )
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
      
QString US_Hydrodyn_Saxs_Hplc::qstring_common_head( QString s1, 
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

QString US_Hydrodyn_Saxs_Hplc::qstring_common_tail( QString s1, 
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

void US_Hydrodyn_Saxs_Hplc::save_created()
{
   QStringList           created_not_saved_list;

   for ( int i = 0; i < lb_created_files->numRows(); i++ )
   {
      if ( lb_created_files->isSelected( i ) && 
           created_files_not_saved.count( lb_created_files->text( i ) ) )
      {
         created_not_saved_list << lb_created_files->text( i );
      }
   }
   save_files( created_not_saved_list );
}

void US_Hydrodyn_Saxs_Hplc::save_created_csv()
{
   QStringList           created_not_saved_list;

   for ( int i = 0; i < lb_created_files->numRows(); i++ )
   {
      if ( lb_created_files->isSelected( i ) ) // && 
         // created_files_not_saved.count( lb_created_files->text( i ) ) )
      {
         created_not_saved_list << lb_created_files->text( i );
      }
   }
   save_files_csv( created_not_saved_list );
}

bool US_Hydrodyn_Saxs_Hplc::save_files_csv( QStringList files )
{
   if ( !files.size() )
   {
      editor_msg( "red", tr( "Internal error: save_files_csv called empty" ) );
      return false;
   }

   {
      QDir dir1( lbl_created_dir->text() );
      if ( !dir1.exists() )
      {
         if ( dir1.mkdir( lbl_created_dir->text() ) )
         {
            editor_msg( "black", QString( tr( "Created directory %1" ) ).arg( lbl_created_dir->text() ) );
         } else {
            editor_msg( "red", QString( tr( "Error: Can not create directory %1 Check permissions." ) ).arg( lbl_created_dir->text() ) );
            return false;
         }
      }
   }         

   if ( !QDir::setCurrent( lbl_created_dir->text() ) )
   {
      editor_msg( "red", QString( tr( "Error: can not set directory %1" ) ).arg( lbl_created_dir->text() ) );
      return false;
   }

   editor_msg( "black", QString( tr( "Current directory is %1" ) ).arg( QDir::current().canonicalPath() ) );

   for ( int i = 0; i < (int)files.size(); i++ )
   {
      if ( !f_qs.count( files[ i ] ) )
      {
         editor_msg( "red", QString( tr( "Error: no data found for %1" ) ).arg( files[ i ] ) );
         return false;
      } 
   }

   QString head = qstring_common_head( files, true );

   QString use_filename = head + ".csv";

   if ( QFile::exists( use_filename ) )
   {
      use_filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( use_filename, 0, this );
      raise();
   }

   QFile f( use_filename );
   if ( !f.open( QIODevice::WriteOnly ) )
   {
      editor_msg( "red", QString( tr( "Error: can not open %1 for writing" ) ).arg( use_filename ) );
      return false;
   }

   Q3TextStream ts( &f );

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
         editor_msg( "red", QString( tr( "Internal error: requested %1, but not found in data" ) ).arg( this_file ) );
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
      editor_msg( "dark red", QString( tr( "Notice: output contains versions cropped to identical grids for compatibility" ) ) );

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
      editor_msg( "red", QString( tr( "Internal error: requested %1, but not found in data" ) ).arg( files[ 0 ] ) );
      f.close();
      return false;
   }

   if ( (( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "hplc_csv_transposed" ) &&
        (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_csv_transposed" ] == "true" )
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
      ts << endl;

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
         ts << endl;
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
         .arg( tr( "US-SOMO Hplc output" ) )
         .arg( crop ? tr( " cropped" ) : "" );

      for ( int i = 0; i < (int)files.size(); i++ )
      {
         if ( !t_qs.count( files[ i ] ) )
         {
            editor_msg( "red", QString( tr( "Internal error: requested %1, but not found in data" ) ).arg( files[ i ] ) );
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
   editor_msg( "black", QString( tr( "%1 written as %2" ) )
               .arg( files.join( " " ) )
               .arg( use_filename ) );
   return true;
}

bool US_Hydrodyn_Saxs_Hplc::save_files( QStringList files )
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
         editor_msg( "red", tr( "save cancelled" ) );
         break;
      }
   }
   update_enables();
   return !errors;
}

bool US_Hydrodyn_Saxs_Hplc::save_file( QString file, bool &cancel, bool &overwrite_all )
{
   if ( !f_qs.count( file ) )
   {
      editor_msg( "red", QString( tr( "Error: no data found for %1" ) ).arg( file ) );
      return false;
   } 

   {
      QDir dir1( lbl_created_dir->text() );
      if ( !dir1.exists() )
      {
         if ( dir1.mkdir( lbl_created_dir->text() ) )
         {
            editor_msg( "black", QString( tr( "Created directory %1" ) ).arg( lbl_created_dir->text() ) );
         } else {
            editor_msg( "red", QString( tr( "Error: Can not create directory %1 Check permissions." ) ).arg( lbl_created_dir->text() ) );
            return false;
         }
      }
   }         

   if ( !QDir::setCurrent( lbl_created_dir->text() ) )
   {
      editor_msg( "red", QString( tr( "Error: can not set directory %1" ) ).arg( lbl_created_dir->text() ) );
      return false;
   }

   QString use_filename;
   if ( f_name.count( file ) && !f_name[ file ].isEmpty() )
   {
      use_filename = QFileInfo( f_name[ file ] ).fileName();
   } else {
      use_filename = file + ".dat";
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
      editor_msg( "red", QString( tr( "Error: can not open %1 in directory %2 for writing" ) )
                  .arg( use_filename )
                  .arg( QDir::current().canonicalPath() )
                  );
      return false;
   }

   Q3TextStream ts( &f );

   update_csv_conc();
   map < QString, double > concs = current_concs();

   QString use_conc;
   if ( concs.count( file ) && concs[ file ] != 0e0 )
   {
      use_conc = QString( " Conc:%1" ).arg( concs[ file ] );
   } else {
      if ( f_conc.count( file ) && f_conc[ file ] != 0e0 ) 
      {
         use_conc = QString( " Conc:%1" ).arg( f_conc[ file ] );
      }
   }

   ts << QString( caption() + tr( " %1data: %2%3%4%5%6\n" ) )
      .arg( ( f_is_time.count( file ) && f_is_time[ file ] ? "Frame " : "" ) )
      .arg( file )
      .arg( f_psv .count( file ) ? QString( " PSV:%1"  ).arg( f_psv [ file ] ) : QString( "" ) )
      .arg( f_I0se.count( file ) ? QString( " I0se:%1" ).arg( f_I0se[ file ] ) : QString( "" ) )
      .arg( use_conc ) // f_conc.count( file ) ? QString( " Conc:%1" ).arg( f_conc[ file ] ) : QString( "" ) )
      .arg( f_extc.count( file ) ? QString( " ExtC_or_DRIinc:%1" ).arg( f_extc[ file ] ) : QString( "" ) )
      .arg( f_time.count( file ) ? QString( " Time:%1" ).arg( f_time[ file ] ) : QString( "" ) )
      ;

   bool use_errors = ( f_errors.count( file ) && 
                       f_errors[ file ].size() > 0 );
                       // is_nonzero_vector( f_errors[ file ] ) );

   if ( f_is_time.count( file ) && f_is_time[ file ] )
   {
      if ( use_errors )
      {
         ts << "t                 \tI(t)         \tsd\n";
      } else {
         ts << "t                 \tI(t)\n";
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
                                    f_qs_string[ file ][ i ].ascii(),
                                    f_Is       [ file ][ i ],
                                    f_errors   [ file ][ i ] );
      } else {
         ts << QString("").sprintf( "%-18s\t%.6e\n",
                                    f_qs_string[ file ][ i ].ascii(),
                                    f_Is       [ file ][ i ] );
      }
   }

   f.close();
   editor_msg( "black", QString( tr( "%1 written as %2" ) )
               .arg( file )
               .arg( use_filename ) );
   created_files_not_saved.erase( file );
   f_name[ file ] = QDir::current().path() + QDir::separator() + use_filename;
   cout << QString( "file <%1> path <%2>\n" ).arg( file ).arg( f_name[ file ] );
   return true;
}

void US_Hydrodyn_Saxs_Hplc::update_csv_conc()
{
   map < QString, bool > skip;
   //    if ( !lbl_hplc->text().isEmpty() )
   //    {
   //       skip[ lbl_hplc->text() ] = true;
   //    }
   //    if ( !lbl_empty->text().isEmpty() )
   //    {
   //       skip[ lbl_empty->text() ] = true;
   //    }

   if ( !csv_conc.data.size() )
   {
      // setup & add all
      csv_conc.name = "Solution Concentrations ";

      csv_conc.header.clear();
      csv_conc.header_map.clear();
      csv_conc.data.clear();
      csv_conc.num_data.clear();
      csv_conc.prepended_names.clear();
      
      csv_conc.header.push_back("File");
      csv_conc.header.push_back("Concentration\nmg/ml");
      
      for ( int i = 0; i < lb_files->numRows(); i++ )
      {
         if ( !skip.count( lb_files->text( i ) ) )
         {
            vector < QString > tmp_data;
            tmp_data.push_back( lb_files->text( i ) );
            tmp_data.push_back( "" );
            
            csv_conc.prepended_names.push_back(tmp_data[0]);
            csv_conc.data.push_back(tmp_data);
         }
      }
   } else {
      map < QString, bool > current_files;
      map < QString, bool > csv_files;
      for ( int i = 0; i < lb_files->numRows(); i++ )
      {
         if ( !skip.count( lb_files->text( i ) ) )
         {
            current_files[ lb_files->text( i ) ] = true;
         }
      }
      csv new_csv = csv_conc;
      new_csv.data.clear();
      new_csv.num_data.clear();
      new_csv.prepended_names.clear();
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
      for ( int i = 0; i < lb_files->numRows(); i++ )
      {
         if ( !skip.count( lb_files->text( i ) ) &&
              !csv_files.count( lb_files->text( i ) ) )
         {
            vector < QString > tmp_data;
            tmp_data.push_back( lb_files->text( i ) );
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

void US_Hydrodyn_Saxs_Hplc::conc()
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
      conc_window = new US_Hydrodyn_Saxs_Hplc_Conc( csv_conc,
                                                      this );
      conc_window->show();
   }
}

map < QString, double > US_Hydrodyn_Saxs_Hplc::current_concs( bool quiet )
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
                               caption(),
                               tr( "There are unsaved updates in the open Solution Concentration window\n"
                                   "This will cause the concentration values used by the current calculation\n"
                                   "to differ from those shown in the Solution Concentration window\n"
                                   "You probably want to save the values in the Solution Concentration window and repeat the computation."
                                   ) );
      }
   }
   return concs;
}

map < QString, double > US_Hydrodyn_Saxs_Hplc::window_concs()
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
   
void US_Hydrodyn_Saxs_Hplc::conc_avg()
{
   //    QStringList files;
   //    for ( int i = 0; i < lb_files->numRows(); i++ )
   //    {
   //       if ( lb_files->isSelected( i ) && 
   //            lb_files->text( i ) != lbl_hplc->text() &&
   //            lb_files->text( i ) != lbl_empty->text() )
   //       {
   //          files << lb_files->text( i );
   //       }
   //    }
   conc_avg( all_selected_files() );
}

vector < double > US_Hydrodyn_Saxs_Hplc::union_q( QStringList files )
{
   map < double, bool > used_q;
   list < double >      ql;

   for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
   {
      if ( !f_qs.count( files[ i ] ) )
      {
         editor_msg( "red", QString( tr( "Internal error: request to use %1, but not found in data" ) ).arg( files[ i ] ) );
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

void US_Hydrodyn_Saxs_Hplc::smooth()
{
   QStringList files = all_selected_files();
   smooth( files );
}

bool US_Hydrodyn_Saxs_Hplc::get_peak( QString file, double &peak )
{
   if ( !f_Is.count( file ) )
   {
      editor_msg( "red", QString( tr( "Internal error: get_peak requested on %1 but no data available" ) ).arg( file ) );
      return false;
   }

   if ( !f_Is[ file ].size() )
   {
      editor_msg( "red", QString( tr( "Internal error: get_peak requested on %1 but data empty" ) ).arg( file ) );
      return false;
   }
      
   peak = f_Is[ file ][ 0 ];
   for ( unsigned int i = 1; i < ( unsigned int ) f_Is[ file ].size(); i++ )
   {
      if ( peak < f_Is[ file ][ i ] )
      {
         peak = f_Is[ file ][ i ];
      }
   }
   return true;
}


void US_Hydrodyn_Saxs_Hplc::smooth( QStringList files )
{
   bool ok;
   int smoothing = QInputDialog::getInteger(
                                            tr( "SOMO: HPLC enter smoothing" ),
                                            tr( "Enter the number of points of smoothing:" ),
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
   for ( int i = 0; i < (int)lb_files->numRows(); i++ )
   {
      current_files[ lb_files->text( i ) ] = true;
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

         lb_created_files->insertItem( smoothed_name );
         lb_created_files->setBottomItem( lb_created_files->numRows() - 1 );
         lb_files->insertItem( smoothed_name );
         lb_files->setBottomItem( lb_files->numRows() - 1 );
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
   for ( int i = 0; i < (int)lb_files->numRows(); i++ )
   {
      if ( select_files.count( lb_files->text( i ) ) )
      {
         lb_files->setSelected( i, true );
      }
   }

   disable_updates = false;
   plot_files();

   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::create_i_of_t()
{
   disable_all();

   QStringList files = all_selected_files();
   create_i_of_t( files );

   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::create_i_of_t( QStringList files )
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

   for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
   {
      if ( !f_qs.count( files[ i ] ) )
      {
         editor_msg( "red", QString( tr( "Internal error: request to use %1, but not found in data" ) ).arg( files[ i ] ) );
      } else {
         QString tmp = files[ i ].mid( head.length() );
         tmp = tmp.mid( 0, tmp.length() - tail.length() );
         if ( rx_cap.search( tmp ) != -1 )
         {
            tmp = rx_cap.cap( 2 );
         }
         double timestamp = tmp.toDouble();
         
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

   ql.sort();

   vector < double > q;
   vector < QString > q_qs;
   for ( list < double >::iterator it = ql.begin();
         it != ql.end();
         it++ )
   {
      q.push_back( *it );
   }

   map < QString, bool > current_files;
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      current_files[ lb_files->text( i ) ] = true;
   }

   QStringList created_files;

   for ( unsigned int i = 0; i < ( unsigned int )q.size(); i++ )
   {
      QString basename = QString( "%1_It_q%2" ).arg( head ).arg( q[ i ] );
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
         if ( it->second.count( q[ i ] ) )
         {
            I.push_back( it->second[ q[ i ] ] );
         } else {
            I.push_back( 0e0 );
         }
         if ( e_values.count( it->first ) &&
              e_values[ it->first ].count( q[ i ] ) )
         {
            e.push_back( e_values[ it->first ][ q[ i ] ] );
         } else {
            e.push_back( 0e0 );
         }
      }

      lb_created_files->insertItem( fname );
      lb_created_files->setBottomItem( lb_created_files->numRows() - 1 );
      lb_files->insertItem( fname );
      lb_files->setBottomItem( lb_files->numRows() - 1 );
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
   }      
   check_discard_it_sd_mult( created_files );
   (void) check_zi_window( created_files );
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::test_i_of_t()
{
   disable_all();
   QStringList files = all_selected_files();
   check_discard_it_sd_mult( files, true );
   if ( check_zi_window( files ) )
   {
      editor_msg( "blue", QString( tr( "Test I(t) ok" ) ) );
   }
   update_enables();
}

bool US_Hydrodyn_Saxs_Hplc::check_zi_window( QStringList & files )
{
   int window = ((US_Hydrodyn *)us_hydrodyn)->gparams[ "hplc_zi_window" ].toInt();

   if ( !files.size() )
   {
      return true;
   }

   if ( window >= (int) f_qs[ files[ 0 ] ].size() )
   {
      QMessageBox::warning(
                           this,
                           this->caption() + tr(": I(t) negative integral window test" ),
                           QString( tr(
                                       "The created I(t) curves have fewer time points (%1) than the negative integral window size (%2)\n"
                                       "The negative integral window test will not be performed.\n"
                                       "You may wish to decrease this value in 'Options'.\n"
                                       "Having a negative region likely means problems with buffer subtraction which should be corrected before *any* analysis is done.\n"
                                       ) )
                           .arg( f_qs[ files[ 0 ] ].size() )
                           .arg( window ),
                           QMessageBox::Ok | QMessageBox::Default,
                           QMessageBox::NoButton
                           );
      return false;
   }

   QStringList messages;

   for ( int i = 0; i < (int) files.size(); ++i )
   {
      QString this_file = files[ i ];
      int pts = (int) f_qs[ this_file ].size();
      for ( int j = 0; j < pts; ++j )
      {
         if ( j + window < pts )
         {
            double integral = 0e0;
            for ( int k = 0; k < window; ++k )
            {
               integral += f_Is[ this_file ][ j + k ];
            }
            if ( integral < 0e0 )
            {
               messages << QString( "%1 negative integral region" ).arg( this_file );
               break;
            }
         }
      }
   }

   if ( messages.size() )
   {
      QStringList qsl;
      for ( int i = 0; i < (int)messages.size() && i < 15; i++ )
      {
         qsl << messages[ i ];
      }

      if ( qsl.size() < messages.size() )
      {
         qsl << QString( tr( "... and %1 more not listed" ) ).arg( messages.size() - qsl.size() );
      }

      QMessageBox::warning(
                           this,
                           this->caption() + tr(": I(t) negative integral window test" ),
                           QString( tr( "Please note:\n\n"
                                        "These files have failed the negative integral window test of size %1\n"
                                        "%2\n\n"
                                        "Having a negative region likely means problems with buffer subtraction which should be corrected before *any* analysis is done.\n"                                             
                                        ) )
                           .arg( window )
                           .arg( qsl.join( "\n" ) ),
                           QMessageBox::Ok | QMessageBox::Default,
                           QMessageBox::NoButton
                           );
      return false;
   }
   
   return true;
}

void US_Hydrodyn_Saxs_Hplc::check_discard_it_sd_mult( QStringList & files, bool optionally_discard )
{
   if ( ((US_Hydrodyn *)us_hydrodyn)->gparams[ "hplc_cb_discard_it_sd_mult" ] != "true" ||
        !files.size() )
   {
      return;
   }

   double mult = ((US_Hydrodyn *)us_hydrodyn)->gparams[ "hplc_discard_it_sd_mult" ].toDouble();

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
         qsl << QString( tr( "... and %1 more not listed" ) ).arg( removefiles.size() - qsl.size() );
      }

      if ( optionally_discard )
      {
         switch ( QMessageBox::question(this, 
                                        this->caption() + tr(": Test I(t), discard I(t)" ),
                                        QString( tr( "Please note:\n\n"
                                                     "Make I(t), discard I(t) with no signal above std. dev. multiplied by %1\n"
                                                     "These curves have been marked to discard\n"
                                                     "%2\n\n"
                                                     "What would you like to do?"
                                                     ) )
                                        .arg( mult )
                                        .arg( qsl.join( "\n" ) ),
                                        tr( "&Discard" ), 
                                        tr( "&Keep" ),
                                        QString::null,
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
                                  this->caption() + tr(": Make I(t), discard I(t)" ),
                                  QString( tr( "Please note:\n\n"
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

bool US_Hydrodyn_Saxs_Hplc::all_selected_have_nonzero_conc()
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
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) 
           //            && 
           //            lb_files->text( i ) != lbl_hplc->text() &&
           //            lb_files->text( i ) != lbl_empty->text() 
           )
      {
         selected_count++;
         if ( !nonzero_concs.count( lb_files->text( i ) ) )
         {
            return false;
         }
      }
   }

   return selected_count > 1;
}

void US_Hydrodyn_Saxs_Hplc::delete_zoomer_if_ranges_changed()
{
}

void US_Hydrodyn_Saxs_Hplc::show_created()
{
   map < QString, bool > created_selected;

   for ( int i = 0; i < lb_created_files->numRows(); i++ )
   {
      if ( lb_created_files->isSelected( i ) )
      {
         created_selected[ lb_created_files->text( i ) ] = true;
      }
   }

   disable_updates = true;
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( !lb_files->isSelected( i ) && 
           created_selected.count( lb_files->text( i ) ) )
      {
         lb_files->setSelected( i, true );
      }
   }
   disable_updates = false;
   plot_files();
   update_enables();
      
}

void US_Hydrodyn_Saxs_Hplc::show_only_created()
{
   disable_updates = true;
   lb_files->clearSelection();
   show_created();
}


QString US_Hydrodyn_Saxs_Hplc::vector_double_to_csv( vector < double > vd )
{
   QString result;
   for ( unsigned int i = 0; i < vd.size(); i++ )
   {
      result += QString("%1,").arg(vd[i]);
   }
   return result;
}

void US_Hydrodyn_Saxs_Hplc::rescale()
{
   //    hide_widgets( plot_errors_widgets, !plot_errors_widgets[ 0 ]->isVisible() );
   //    hide_widgets( files_widgets, !files_widgets[ 0 ]->isVisible() );
   //    hide_widgets( files_expert_widgets, !files_expert_widgets[ 0 ]->isVisible() );
   //    hide_widgets( created_files_widgets, !created_files_widgets[ 0 ]->isVisible() );
   //    hide_widgets( created_files_expert_widgets, !created_files_expert_widgets[ 0 ]->isVisible() );
   //    hide_widgets( editor_widgets, !editor_widgets[ 0 ]->isVisible() );

   //bool any_selected = false;
   double minx = 0e0;
   double maxx = 1e0;
   double miny = 0e0;
   double maxy = 1e0;

   double file_minx;
   double file_maxx;
   double file_miny;
   double file_maxy;
   
   bool first = true;
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         //any_selected = true;
         if ( get_min_max( lb_files->text( i ), file_minx, file_maxx, file_miny, file_maxy ) )
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
   
   if ( plot_dist_zoomer )
   {
      plot_dist_zoomer->zoom ( 0 );
      delete plot_dist_zoomer;
   }

   plot_dist->setAxisScale( QwtPlot::xBottom, minx, maxx );
   plot_dist->setAxisScale( QwtPlot::yLeft  , miny * 0.9e0 , maxy * 1.1e0 );
   plot_dist_zoomer = new ScrollZoomer(plot_dist->canvas());
   plot_dist_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
#ifndef QT4
   plot_dist_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
   connect( plot_dist_zoomer, SIGNAL( zoomed( const QwtDoubleRect & ) ), SLOT( plot_zoomed( const QwtDoubleRect & ) ) );
   
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

void US_Hydrodyn_Saxs_Hplc::join()
{
   vector < QString > selected;
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         selected.push_back( lb_files->text( i ) );
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
      double res = QInputDialog::getDouble(
                                           tr( "US-SOMO: Saxs Hplc: Join" ),
                                           QString( tr( "The curves %1 and %2\n"
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
                  QString( tr( "Warning: no errors will be stored because %1 does not contain any error information" ) )
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

   lb_created_files->insertItem   ( use_basename );
   lb_created_files->setBottomItem( lb_created_files->numRows() - 1 );
   lb_files        ->insertItem   ( use_basename );
   lb_files        ->setBottomItem( lb_files->numRows() - 1 );

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
   lb_created_files->setSelected( lb_created_files->numRows() - 1, true );
   show_created();

   editor_msg( "black", 
               QString( tr( "Created %1 as join of %2 and %3 at q %4" ) )
               .arg( use_basename )
               .arg( selected[ 0 ] )
               .arg( selected[ 1 ] )
               .arg( q_join ) );
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::plot_zoomed( const QwtDoubleRect & /* rect */ )
{
   //   cout << QString( "zoomed: %1 %2 %3 %4\n" )
   // .arg( rect.x1() )
   // .arg( rect.x2() )
   // .arg( rect.y1() )
   // .arg( rect.y2() );
   if ( !running )
   {
      // cout << "not running\n";
      update_enables();
   } else {
      // cout << "is running, update_enables skipped\n";
   }
}

void US_Hydrodyn_Saxs_Hplc::plot_errors_zoomed( const QwtDoubleRect & /* rect */ )
{
}

void US_Hydrodyn_Saxs_Hplc::zoom_info()
{
   if ( plot_dist_zoomer )
   {
#ifndef QT4
      cout << QString( "zoomrect: %1 %2 %3 %4\n" )
         .arg( plot_dist_zoomer->zoomRect().x1() )
         .arg( plot_dist_zoomer->zoomRect().x2() )
         .arg( plot_dist_zoomer->zoomRect().y1() )
         .arg( plot_dist_zoomer->zoomRect().y2() );
      cout << QString( "zoombase: %1 %2 %3 %4\n" )
         .arg( plot_dist_zoomer->zoomBase().x1() )
         .arg( plot_dist_zoomer->zoomBase().x2() )
         .arg( plot_dist_zoomer->zoomBase().y1() )
         .arg( plot_dist_zoomer->zoomBase().y2() );
#else
      cout << QString( "zoomrect: %1 %2 %3 %4\n" )
         .arg( plot_dist_zoomer->zoomRect().left() )
         .arg( plot_dist_zoomer->zoomRect().right() )
         .arg( plot_dist_zoomer->zoomRect().top() )
         .arg( plot_dist_zoomer->zoomRect().bottom() );
      cout << QString( "zoombase: %1 %2 %3 %4\n" )
         .arg( plot_dist_zoomer->zoomBase().left() )
         .arg( plot_dist_zoomer->zoomBase().right() )
         .arg( plot_dist_zoomer->zoomBase().top() )
         .arg( plot_dist_zoomer->zoomBase().bottom() );
#endif
   } else {
      cout << "no current zoomer\n";
   }
}

void US_Hydrodyn_Saxs_Hplc::plot_mouse( const QMouseEvent & /* me */ )
{
   // cout << "mouse event\n";
   // zoom_info();
   if ( plot_dist_zoomer )
   {
      // cout << QString( "is base %1\n" ).arg( plot_dist_zoomer->zoomBase() == 
      // plot_dist_zoomer->zoomRect() ? "yes" : "no" );
      update_enables();
   }
}

void US_Hydrodyn_Saxs_Hplc::select_vis()
{
#ifndef QT4
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
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         QString this_file = lb_files->text( i );
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
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( selected_files.count( lb_files->text( i ) ) )
      {
         lb_files->setSelected( i, true );
         if ( !did_current )
         {
            lb_files->setCurrentItem( i );
            did_current = true;
         }
      }
   }
   disable_updates = false;
   lb_files->ensureCurrentVisible();
   update_files();
}

void US_Hydrodyn_Saxs_Hplc::remove_vis()
{
#ifndef QT4
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
   cout << "select visible\n";
   QStringList selected_files;
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         QString this_file = lb_files->text( i );
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

void US_Hydrodyn_Saxs_Hplc::crop_left()
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

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         QString this_file = lb_files->text( i );
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
            editor_msg( "red", QString( tr( "Crop left: curves need at least %1 points to crop" ) ).arg( show_pts ) );
            return;
         }            
      }
   }

   // is the rectangle contained?
   if ( 
#ifndef QT4
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
      editor_msg( "black", tr( "Crop left: press again to crop one point" ) );
      // will our current zoom rectangle show all the points?
      // if so, simply move it
      double dx = maxx - minx;
      double dy = maxy - miny;

#ifndef QT4
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
         cout << QString( "just move to %1 %2\n" ).arg( newx ).arg( newy );
         plot_dist_zoomer->move( newx, newy );
         return;
      }

      // ok, we are going to have to make a rectangle
      QwtDoubleRect dr = plot_dist_zoomer->zoomRect();

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
#ifndef QT4
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
   editor_msg( "blue", tr( "Crop left: cropped 1 point" ) );

   update_files();
}

void US_Hydrodyn_Saxs_Hplc::crop_right()
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

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         QString this_file = lb_files->text( i );
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
            editor_msg( "red", QString( tr( "Crop right: curves need at least %1 points to crop" ) ).arg( show_pts ) );
            return;
         }            
      }
   }

   // is the rectangle contained?
   if ( 
#ifndef QT4
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
      editor_msg( "black", tr( "Crop right: press again to crop one point" ) );
      // will our current zoom rectangle show all the points?
      // if so, simply move it
      double dx = maxx - minx;
      double dy = maxy - miny;

#ifndef QT4
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
         cout << QString( "just move to %1 %2\n" ).arg( newx ).arg( newy );
         plot_dist_zoomer->move( newx, newy );
         return;
      }

      // ok, we are going to have to make a rectangle
      QwtDoubleRect dr = plot_dist_zoomer->zoomRect();

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
#ifndef QT4
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
   editor_msg( "blue", tr( "Crop right: cropped 1 point" ) );

   update_files();
}

void US_Hydrodyn_Saxs_Hplc::crop_undo()
{
   if ( !crop_undos.size() )
   {
      return;
   }

   map < QString, bool > current_files;

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      current_files[ lb_files->text( i ) ] = true;
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
            editor_msg( "red", QString( tr( "Error: can not undo crop to missing file %1" ) ).arg( it->first ) );
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
                  editor_msg( "dark red", QString( tr( "Warning: file %1 had no errors before crop common but somehow has errors now (?), removing them" ) ).arg( it->first ) );
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
            editor_msg( "red", QString( tr( "Error: can not undo crop to missing file %1" ) ).arg( cud.files[ i ] ) );
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
      editor_msg( "blue", tr( "Crop undo: restored 1 point" ) );
   }
   update_files();
}


void US_Hydrodyn_Saxs_Hplc::view()
{
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         QString file = lb_files->text( i );

         QString text;

         text += QString( tr( "US-SOMO Hplc output: %1\n" ) ).arg( file );

         bool use_errors = ( f_errors.count( file ) && 
                             f_errors[ file ].size() > 0 );
         
         if ( use_errors )
         {
            text += "q                  \tI(q)         \tsd\n";
         } else {
            text += "q                  \tI(q)\n";
         }

         for ( int i = 0; i < (int)f_qs[ file ].size(); i++ )
         {
            if ( use_errors &&
                 (int)f_errors[ file ].size() > i )
            {
               text += QString("").sprintf( "%-18s\t%.6e\t%.6e\n",
                                          f_qs_string[ file ][ i ].ascii(),
                                          f_Is       [ file ][ i ],
                                          f_errors   [ file ][ i ] );
            } else {
               text += QString("").sprintf( "%-18s\t%.6e\n",
                                          f_qs_string[ file ][ i ].ascii(),
                                          f_Is       [ file ][ i ] );
            }
         }

         TextEdit *edit;
         edit = new TextEdit( this, file );
         edit->setFont    ( QFont( "Courier" ) );
         edit->setPalette ( PALET_NORMAL );
         AUTFBACK( edit );
         edit->setGeometry( global_Xpos + 30, global_Ypos + 30, 685, 600 );
         // edit->setTitle( file );
         if ( QFile::exists( file + ".dat" ) )
         {
            edit->load( file + ".dat", file );
         } else {
            edit->load_text( text );
         }
         //   edit->setTextFormat( PlainText );
         edit->show();
      }
   }
}

bool US_Hydrodyn_Saxs_Hplc::is_nonzero_vector( vector < double > &v )
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

bool US_Hydrodyn_Saxs_Hplc::is_zero_vector( vector < double > &v )
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

void US_Hydrodyn_Saxs_Hplc::to_created( QString file )
{
   bool in_created = false;
   for ( int i = 0; i < (int)lb_created_files->numRows(); i++ )
   {
      if ( file == lb_created_files->text( i ) )
      {
         created_files_not_saved[ file ] = true;
         in_created = true;
      }
   }

   if ( !in_created )
   {
      lb_created_files->insertItem( file );
      lb_created_files->setBottomItem( lb_created_files->numRows() - 1 );
      created_files_not_saved[ file ] = true;
   }
}

void US_Hydrodyn_Saxs_Hplc::crop_vis()
{
   // find curves within zoomRect & select only them
#ifndef QT4
   double minx = plot_dist_zoomer->zoomRect().x1();
   double maxx = plot_dist_zoomer->zoomRect().x2();
   double miny = plot_dist_zoomer->zoomRect().y1();
   double maxy = plot_dist_zoomer->zoomRect().y2();
#else
   double minx = plot_dist_zoomer->zoomRect().left();
   double maxx = plot_dist_zoomer->zoomRect().right();
   double miny = plot_dist_zoomer->zoomRect().top();
   double maxy = plot_dist_zoomer->zoomRect().bottom();
#endif

   map < QString, bool > selected_files;

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         QString this_file = lb_files->text( i );
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
      editor_msg( "red", tr( "Crop visible: The current visible plot is empty" ) );
      return;
   }


   // make sure we don't leave a gap
   map < QString, bool > crop_left_side;

   for ( map < QString, bool >::iterator it = selected_files.begin();
         it != selected_files.end();
         it++ )
   {
      if ( f_qs[ it->first ][ 0 ]   < minx &&
           f_qs[ it->first ].back() > maxx )
      {
         editor_msg( "red", tr( "Crop visible: error: you can not crop out the middle of a curve" ) );
         return;
      }
      if ( f_qs[ it->first ][ 0 ]   < minx )
      {
         crop_left_side[ it->first ] = false;
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
            editor_msg( "dark red", tr( "Crop visible: warning: you are cropping the left of some curves and the right of others. " ) );
            break;
         }
      }
   }
   
   editor_msg( "black",
               QString( tr( "Crop visible:\n"
                            "Cropping out q-range of (%1:%2)\n" ) )
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
         if ( f_qs[ it->first ][ i ] < minx ||
              f_qs[ it->first ][ i ] > maxx )
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
   editor_msg( "blue", tr( "Crop visible: done" ) );

   update_files();

   if ( plot_dist_zoomer &&
        plot_dist_zoomer->zoomRectIndex() )
   {
      plot_dist_zoomer->zoom( -1 );
   }
}

void US_Hydrodyn_Saxs_Hplc::legend()
{
#ifndef QT4
   if ( plot_dist->autoLegend() )
   {
      plot_dist->setAutoLegend( false );
      plot_dist->enableLegend ( false, -1 );
   } else {
      plot_dist->setAutoLegend( true );
      plot_dist->enableLegend ( true, -1 );
   }
#else
   legend_vis = !legend_vis;
   legend_set();
#endif
}

void US_Hydrodyn_Saxs_Hplc::legend_set()
{
#ifdef QT4
   QwtPlotItemList ilist = plot_dist->itemList();
   for ( int ii = 0; ii < ilist.size(); ii++ )
   {
      QwtPlotItem* plitem = ilist[ ii ];
      if ( plitem->rtti() != QwtPlotItem::Rtti_PlotCurve )
         continue;
      plitem->setItemAttribute( QwtPlotItem::Legend, legend_vis );
   }
   plot_dist->legend()->setVisible( legend_vis );
#endif
}

void US_Hydrodyn_Saxs_Hplc::similar_files()
{
   vector < QString > selected;
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         selected.push_back( lb_files->text( i ) );
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
                 QString( tr( "Error: count not find disk reference for %1" ) )
                 .arg( selected[ 0 ] ) );
      return;
   }

   disable_all();

   QString similar = QFileInfo( f_name[ selected[ 0 ] ] ).fileName();
   QString dir     = QFileInfo( f_name[ selected[ 0 ] ] ).dirPath();
   QString match   = similar;
   match.replace( QRegExp( "\\d{2,}" ), "\\d+" );

   cout << QString( "select to match <%1> in directory <%2> using regexp <%3>\n" )
      .arg( similar )
      .arg( dir )
      .arg( match )
      .ascii();
   // go to that directory and get file list
   // turn basename into regexp \\d{2,} into \\d+
   // load unloaded files found with match
   QDir::setCurrent( dir );
   if ( !cb_lock_dir->isChecked() )
   {
      lbl_dir->setText( QDir::currentDirPath() );
   }
   QDir qd;
   add_files( qd.entryList( "*" ).grep( QRegExp( match ) ) );
}

void US_Hydrodyn_Saxs_Hplc::regex_load()
{
   // make list of lbl_dir all files
   // filter with regex for each # in list
   QString dir     = lbl_dir->text();

   QDir::setCurrent( dir );
   lbl_dir->setText( QDir::currentDirPath() );
   QDir qd;


   QStringList regexs = QStringList::split( QRegExp( "\\s+" ), le_regex->text()      );
   QStringList args   = QStringList::split( QRegExp( "\\s+" ), le_regex_args->text() );

   for ( int i = 0; i < (int)args.size(); i++ )
   {
      for ( int j = 0; j < (int)regexs.size(); j++ )
      {
         QString match   = QString( regexs[ j ] ).arg( args[ i ] );
         editor_msg( "dark blue", 
                     QString( tr( "Load %1 using %2 " ) ).arg( args[ i ] ).arg( match ) );
         add_files( qd.entryList( "*" ).grep( QRegExp( match ) ) );
      }
   }
}

void US_Hydrodyn_Saxs_Hplc::rename_created( Q3ListBoxItem *lbi, const QPoint & )
{
   map < QString, bool > existing_items;
   for ( int i = 0; i < (int)lb_files->numRows(); i++ )
   {
      existing_items[ lb_files->text( i ) ] = true;
   }

   QString dupmsg;
   bool ok;
   QString text = lbi->text();
   do {
      text = QInputDialog::getText(
                                   tr("US-SOMO: Copy File"),
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
            dupmsg = tr( "Name already exists, choose a unique name\n" );
         }
      } 
   } while ( ok && !text.isEmpty() && !dupmsg.isEmpty() );

   if ( !ok || text.isEmpty() )
   {
      return;
   }
   if ( existing_items.count( text ) )
   {
      editor_msg( "red", tr( "Internal error: duplicate name" ) );
      return;
   }

   // just a copy right now, since lbi is "protected within this context"

   lb_created_files->insertItem( text );
   lb_created_files->setBottomItem( lb_created_files->numRows() - 1 );
   lb_files->insertItem( text );
   lb_files->setBottomItem( lb_files->numRows() - 1 );
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

void US_Hydrodyn_Saxs_Hplc::normalize()
{
   QStringList files = all_selected_files();
   //    for ( int i = 0; i < lb_files->numRows(); i++ )
   //    {
   //       if ( lb_files->isSelected( i ) && 
   //            lb_files->text( i ) != lbl_hplc->text() &&
   //            lb_files->text( i ) != lbl_empty->text() )
   //       {
   //          files << lb_files->text( i );
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
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      existing_items[ lb_files->text( i ) ] = true;
   }

   for ( int i = 0; i < (int)files.size(); i++ )
   {
      
      QString norm_name = files[ i ] + "_n";
      unsigned int ext = 0;
      while ( existing_items.count( norm_name ) )
      {
         norm_name = files[ i ] + QString( "_n%1" ).arg( ++ext );
      }

      lb_created_files->insertItem( norm_name );
      lb_created_files->setBottomItem( lb_created_files->numRows() - 1 );
      lb_files->insertItem( norm_name );
      lb_files->setBottomItem( lb_files->numRows() - 1 );
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

void US_Hydrodyn_Saxs_Hplc::add_plot( QString           name,
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

void US_Hydrodyn_Saxs_Hplc::add_plot( QString           name,
                                      vector < double > q,
                                      vector < double > I,
                                      vector < double > errors,
                                      bool              is_time,
                                      bool              replot )
{
   name.replace( QRegExp( "(\\s+|\"|'|\\/|\\.)" ), "_" );
   if ( q.size() != I.size() )
   {
      cout << QString( "add_plot: size error %1 %2\n" ).arg( q.size() ).arg( I.size() );
   }

   // printvector( "add_plot q", q );
   // printvector( "add_plot I", I );
   
   QString bsub_name = name;
   unsigned int ext = 0;

   map < QString, bool > current_files;

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      QString this_file = lb_files->text( i );
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
   
   lb_created_files->insertItem( bsub_name );
   lb_created_files->setBottomItem( lb_created_files->numRows() - 1 );
   lb_files->insertItem( bsub_name );
   lb_files->setBottomItem( lb_files->numRows() - 1 );
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
   {
      delete plot_dist_zoomer;
      plot_dist_zoomer = (ScrollZoomer *) 0;
   }
   if ( replot )
   {
      plot_files();
      update_enables();
   }
}

void US_Hydrodyn_Saxs_Hplc::crop_zero()
{
   // delete points < zero of selected curves

   if ( QMessageBox::Ok != 
        QMessageBox::warning(
                             this,
                             this->caption() + tr(": Crop zeros" ),
                             tr(
                                "Noisy intensity data that sometimes get negative values are physically meaningful.\n"
                                "The curves are a subtraction between two positive and very close intensity curves." ),
                             QMessageBox::Ok,
                             QMessageBox::Cancel | QMessageBox::Default
                             ) )
   {
      return;
   }

   map < QString, bool > selected_files;

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         QString this_file = lb_files->text( i );

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
   if ( plot_dist_zoomer )
   {
      delete plot_dist_zoomer;
      plot_dist_zoomer = (ScrollZoomer *) 0;
   }
   plot_files();
}

bool US_Hydrodyn_Saxs_Hplc::compatible_files( QStringList files )
{
   if ( !files.size() )
   {
      return true;
   }

   if ( !f_is_time.count( files[ 0 ] ) )
   {
      editor_msg( "red", QString( tr( "Internal error: file %1 has no q/T encoding data" ) ).arg( files[ 0 ] ) );
      return false;
   }

   bool first_type = f_is_time[ files[ 0 ] ];
   for ( unsigned int i = 1; i < ( unsigned int ) files.size(); i++ )
   {
      if ( !f_is_time.count( files[ i ] ) )
      {
         editor_msg( "red", QString( tr( "Internal error: file %1 has no q/T encoding data" ) ).arg( files[ i ] ) );
         return false;
      }
      if ( f_is_time[ files[ i ] ] != first_type )
      {
         return false;
      }
   }
   return true;
}

bool US_Hydrodyn_Saxs_Hplc::type_files( QStringList files )
{
   if ( !files.size() )
   {
      return false;
   }

   if ( !f_is_time.count( files[ 0 ] ) )
   {
      editor_msg( "red", QString( tr( "Internal error: file %1 has no q/T encoding data" ) ).arg( files[ 0 ] ) );
      return false;
   }

   return f_is_time[ files[ 0 ] ];
}



void US_Hydrodyn_Saxs_Hplc::wheel_start()
{
   QStringList selected_files;
   lbl_wheel_pos->setText( QString( "%1" ).arg( 0 ) );

   selected_files = all_selected_files();

   bool ok;

   wheel_file = QInputDialog::getItem(
                                      tr( "SOMO: HPLC timeshift: select file" ),
                                      tr("Select the curve to timeshift:\n" ),
                                      selected_files, 
                                      0, 
                                      FALSE, 
                                      &ok,
                                      this );
   if ( !ok ) {
      return;
   }

   // gaussian_mode  = false;
   // ggaussian_mode = false;
   // baseline_mode  = false;

   if ( !plotted_curves.count( wheel_file ) )
   {
      editor_msg( "red", QString( tr( "Internal error: request to timeshift %1, but not found in data" ) ).arg( wheel_file ) );
      return;
   }

   wheel_curve           = plotted_curves[ wheel_file ];

   running               = true;

   disable_all();
   // timeshift_mode = true;
   mode_select( MODE_TIMESHIFT );

   plot_errors->clear();
   
   pb_rescale            ->setEnabled( true );
   pb_wheel_start        ->setEnabled( false );
   pb_wheel_cancel       ->setEnabled( true );
   qwtw_wheel            ->setEnabled( true );
   qwtw_wheel            ->setRange  ( -100, 100, .1 );
   qwtw_wheel            ->setValue  ( 0 );
}


void US_Hydrodyn_Saxs_Hplc::gaussian_enables()
{
   unsigned int g_size = ( unsigned int )gaussians.size() / gaussian_type_size;

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
   qwtw_wheel          ->setEnabled( g_size && gaussian_pos < g_size );
   pb_rescale          ->setEnabled( true );
   pb_view             ->setEnabled( true );
   pb_errors           ->setEnabled( true );
}

void US_Hydrodyn_Saxs_Hplc::update_gauss_pos()
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

         le_gauss_pos_height->setText( QString( "%1" ).arg( gaussians[ 0 + gaussian_type_size * gaussian_pos ], 0, 'f', 6 ) );
         le_gauss_pos       ->setText( QString( "%1" ).arg( gaussians[ 1 + gaussian_type_size * gaussian_pos ], 0, 'f', 6 ) );
         le_gauss_pos_width ->setText( QString( "%1" ).arg( gaussians[ 2 + gaussian_type_size * gaussian_pos ], 0, 'f', 6 ) );
         if ( dist1_active )
         {
            le_gauss_pos_dist1 ->setText( QString( "%1" ).arg( gaussians[ 3 + gaussian_type_size * gaussian_pos ], 0, 'f', 6 ) );
            if ( dist2_active )
            {
               le_gauss_pos_dist2 ->setText( QString( "%1" ).arg( gaussians[ 4 + gaussian_type_size * gaussian_pos ], 0, 'f', 6 ) );
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
#ifndef QT4
               plot_dist->setMarkerPen       ( plotted_markers[ i ], QPen( Qt::magenta, 2, DashDotDotLine));
#else
               plotted_markers[ i ]->setLinePen( QPen( Qt::magenta, 2, Qt::DashDotDotLine ) );
#endif
            } else {
#ifndef QT4
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
         unsigned int ofs = 0;

         lbl_gauss_pos      ->setText( QString( " %1 of %2 " ).arg( gaussian_pos + 1 ).arg( gaussians.size() / gaussian_type_size ) );

         disconnect( le_gauss_pos       , SIGNAL( textChanged( const QString & ) ), 0, 0 );
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

         if ( le_gauss_pos->hasFocus() ||
              !( 
                ( cb_fix_width->isChecked() && le_gauss_pos_width ->hasFocus() ) ||
                ( dist1_active && cb_fix_dist1->isChecked() && le_gauss_pos_dist1 ->hasFocus() ) ||
                ( dist2_active && cb_fix_dist2->isChecked() && le_gauss_pos_dist2 ->hasFocus() ) ||
                le_gauss_fit_start ->hasFocus() ||
                le_gauss_fit_end   ->hasFocus() ) )
         {
            qwtw_wheel   ->setValue( le_gauss_pos->text().toDouble() );
         }
         if ( cb_fix_width->isChecked() && le_gauss_pos_width->hasFocus() )
         {
            qwtw_wheel   ->setValue( le_gauss_pos_width->text().toDouble() );
         }
         if ( dist1_active && cb_fix_dist1->isChecked() && le_gauss_pos_dist1->hasFocus() )
         {
            qwtw_wheel   ->setValue( le_gauss_pos_dist1->text().toDouble() );
         }
         if ( dist2_active && cb_fix_dist2->isChecked() && le_gauss_pos_dist2->hasFocus() )
         {
            qwtw_wheel   ->setValue( le_gauss_pos_dist2->text().toDouble() );
         }

         for ( unsigned int i = 2; i < ( unsigned int ) plotted_markers.size(); i++ )
         {
            if ( gaussian_pos + 2 == i && !le_gauss_fit_start->hasFocus() && !le_gauss_fit_end->hasFocus() )
            {
#ifndef QT4
               plot_dist->setMarkerPen       ( plotted_markers[ i ], QPen( Qt::magenta, 2, DashDotDotLine));
#else
               plotted_markers[ i ]->setLinePen( QPen( Qt::magenta, 2, Qt::DashDotDotLine));
#endif
            } else {
#ifndef QT4
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

void US_Hydrodyn_Saxs_Hplc::gauss_start()
{
   plot_errors->clear();
   if ( plot_errors_zoomer )
   {
      delete plot_errors_zoomer;
      plot_errors_zoomer = (ScrollZoomer *) 0;
   }

   le_last_focus = (mQLineEdit *) 0;
   pb_gauss_fit->setText( tr( "Fit" ) );

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         wheel_file = lb_files->text( i );
         break;
      }
   }

   if ( !f_qs.count( wheel_file ) )
   {
      editor_msg( "red", QString( tr( "Internal error: %1 not found in data" ) ).arg( wheel_file ) );
      return;
   }

   if ( !f_qs[ wheel_file ].size() )
   {
      editor_msg( "red", QString( tr( "Internal error: %1 empty data" ) ).arg( wheel_file ) );
      return;
   }

   if ( !f_Is.count( wheel_file ) )
   {
      editor_msg( "red", QString( tr( "Internal error: %1 not found in y data" ) ).arg( wheel_file ) );
      return;
   }

   if ( !f_Is[ wheel_file ].size() )
   {
      editor_msg( "red", QString( tr( "Internal error: %1 empty y data" ) ).arg( wheel_file ) );
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
      editor_msg( "red", QString( tr( "Error: maximum y value of signal %1 is not positive" ) ).arg( wheel_file ) );
      return;
   }

#ifndef QT4
   plot_dist->setCurvePen( plotted_curves[ wheel_file ], QPen( Qt::cyan, use_line_width, SolidLine));
#else
   plotted_curves[ wheel_file ]->setPen( QPen( Qt::cyan, use_line_width, Qt::SolidLine ) );
#endif

   org_gaussians = gaussians;

   if ( f_gaussians.count( wheel_file ) &&
        f_gaussians[ wheel_file ].size() )
   {
      cout << "using file specific gaussians\n";
      gaussians = f_gaussians[ wheel_file ];
   }

   opt_repeak_gaussians( wheel_file );

   // gaussian_mode = true;
   mode_select( MODE_GAUSSIAN );

   running               = true;

   double max_range = gauss_max_height > f_qs[ wheel_file ].back() ? gauss_max_height : f_qs[ wheel_file ].back();

   qwtw_wheel->setRange( 0, // f_qs[ wheel_file ][ 0 ]
                         max_range, // f_qs[ wheel_file ].back(),
                         max_range / UHSH_WHEEL_RES );

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
      cout << "setting gauss fit end\n";
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
   if ( errors_were_on )
   {
      hide_widgets( plot_errors_widgets, false );
      cb_plot_errors_group->hide();
      if ( !f_errors.count( wheel_file ) ||
           !is_nonzero_vector(  f_errors[ wheel_file ]  ) ||
           f_errors[ wheel_file ].size() != f_qs[ wheel_file ].size() )
      {
         cb_plot_errors_sd->setChecked( false );
         cb_plot_errors_sd->hide();
      }
   }
   gaussian_enables();
}
      
void US_Hydrodyn_Saxs_Hplc::gauss_clear()
{
   gaussian_pos = 0;
   gaussians.clear();
   gauss_init_markers();
   gauss_init_gaussians();
   update_gauss_pos();
   gaussian_enables();
}

void US_Hydrodyn_Saxs_Hplc::gauss_new()
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
   gauss_add_gaussian( &(gaussians[ gaussian_pos * gaussian_type_size ]), Qt::green );
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

void US_Hydrodyn_Saxs_Hplc::gauss_prev()
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

void US_Hydrodyn_Saxs_Hplc::gauss_next()
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

void US_Hydrodyn_Saxs_Hplc::gauss_save()
{
   {
      QDir dir1( lbl_created_dir->text() );
      if ( !dir1.exists() )
      {
         if ( dir1.mkdir( lbl_created_dir->text() ) )
         {
            editor_msg( "black", QString( tr( "Created directory %1" ) ).arg( lbl_created_dir->text() ) );
         } else {
            editor_msg( "red", QString( tr( "Error: Can not create directory %1 Check permissions." ) ).arg( lbl_created_dir->text() ) );
            return;
         }
      }
   }         

   if ( !QDir::setCurrent( lbl_created_dir->text() ) )
   {
      editor_msg( "red", QString( tr( "Error: can not set directory %1" ) ).arg( lbl_created_dir->text() ) );
      return;
   }

   if ( current_mode == MODE_GAUSSIAN )
   {
      QString use_filename = wheel_file + "-gauss.dat";
   
      if ( QFile::exists( use_filename ) )
      {
         use_filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( use_filename, 0, this );
         raise();
      }

      QFile f( use_filename );
      if ( !f.open( QIODevice::WriteOnly ) )
      {
         editor_msg( "red", QString( tr( "Error: can not open %1 for writing" ) ).arg( use_filename ) );
      }

      Q3TextStream ts( &f );

      ts << QString( "US-SOMO Hplc Gaussians: %1\n" ).arg( wheel_file );

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
      editor_msg( "black", QString( tr( "Gaussians written as %1" ) )
                  .arg( use_filename ) );
   } else {
      if ( unified_ggaussian_ok )
      {
         QString use_filename = wheel_file + "-mgauss.dat";
   
         if ( QFile::exists( use_filename ) )
         {
            use_filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( use_filename, 0, this );
            raise();
         }

         QFile f( use_filename );
         if ( !f.open( QIODevice::WriteOnly ) )
         {
            editor_msg( "red", QString( tr( "Error: can not open %1 for writing" ) ).arg( use_filename ) );
         }

         Q3TextStream ts( &f );

         ts << QString( "US-SOMO Hplc Multiple Gaussians: %1\n").arg( wheel_file );

         ts << QString( "%1 %2\n" ).arg( le_gauss_fit_start->text() ).arg( le_gauss_fit_end->text() );


         for ( unsigned int i = 0; i < ( unsigned int ) unified_ggaussian_files.size(); i++ )
         {
            ts << QString( "Gaussians %1\n" ).arg( unified_ggaussian_files[ i ] );


            if ( cb_fix_width->isChecked() )
            {
               unsigned int  index = 2 * unified_ggaussian_gaussians_size + i * unified_ggaussian_gaussians_size;
               for ( unsigned int j = 0; j < unified_ggaussian_gaussians_size; j++ )
               {
                  ts << 
                     QString( "%1 %2 %3\n" )
                     .arg( unified_ggaussian_params[ index + j + 0 ], 0, 'g', 10 )
                     .arg( unified_ggaussian_params[ 2 * j + 0 ]                , 0, 'g', 10 )
                     .arg( unified_ggaussian_params[ 2 * j + 1 ], 0, 'g', 10 )
                     ;
               }
            } else {
               unsigned int  index = unified_ggaussian_gaussians_size + i * 2 * unified_ggaussian_gaussians_size;
               for ( unsigned int j = 0; j < unified_ggaussian_gaussians_size; j++ )
               {
                  ts << 
                     QString( "%1 %2 %3\n" )
                     .arg( unified_ggaussian_params[ index + 2 * j + 0 ], 0, 'g', 10 )
                     .arg( unified_ggaussian_params[ j ]                , 0, 'g', 10 )
                     .arg( unified_ggaussian_params[ index + 2 * j + 1 ], 0, 'g', 10 )
                     ;
               }
            }               
         }

         f.close();
         editor_msg( "black", QString( tr( "Gaussians written as %1" ) )
                     .arg( use_filename ) );
      }
   }      
}

void US_Hydrodyn_Saxs_Hplc::gauss_pos_text( const QString & text )
{
   if ( gaussians.size() )
   {
      if ( current_mode == MODE_GAUSSIAN )
      {
         cout << QString( "gauss_pos_text <%1>, pos %2 size %3\n" ).arg( text ).arg( gaussian_pos ).arg( gaussians.size() );
         gaussians[ 1 + gaussian_type_size * gaussian_pos ] = text.toDouble();
      } else {
         if ( cb_fix_width->isChecked() )
         {
            unified_ggaussian_params[ 0 + 2 * gaussian_pos ] = text.toDouble();

            if ( plotted_hlines.size() > gaussian_pos )
            {
               double center = unified_ggaussian_params[ 2 * gaussian_pos + 0 ];
               double width  = unified_ggaussian_params[ 2 * gaussian_pos + 1 ];
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

               cout << QString( "add_hline %1 %2 (%3,%4) (%5,%6)\n" )
                  .arg( center )
                  .arg( width )
                  .arg( x[0] )
                  .arg( y[0] )
                  .arg( x[1] )
                  .arg( y[1] )
                  ;
#ifndef QT4
               plot_dist->setCurveData( plotted_hlines[ gaussian_pos ],
                                        (double *)&x[ 0 ],
                                        (double *)&y[ 0 ],
                                        2
                                        );
#else
               plotted_hlines[ gaussian_pos ]->setData(
                                                       (double *)&x[ 0 ],
                                                       (double *)&y[ 0 ],
                                                       2
                                                       );
#endif
            }
         } else {
            unified_ggaussian_params[ gaussian_pos ] = text.toDouble();
         }
      }
      
#ifndef QT4
      plot_dist->setMarkerPos( plotted_markers[ 2 + gaussian_pos ], text.toDouble(), 0e0 );
#else
      plotted_markers[ 2 + gaussian_pos ]->setXValue( text.toDouble() );
#endif
      if ( qwtw_wheel->value() != text.toDouble() )
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

void US_Hydrodyn_Saxs_Hplc::gauss_pos_width_text( const QString & text )
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
            unified_ggaussian_params[ 1 + 2 * gaussian_pos ] = text.toDouble();

            if ( plotted_hlines.size() > gaussian_pos )
            {
               double center = unified_ggaussian_params[ 2 * gaussian_pos + 0 ];
               double width  = unified_ggaussian_params[ 2 * gaussian_pos + 1 ];
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

               cout << QString( "add_hline %1 %2 (%3,%4) (%5,%6)\n" )
                  .arg( center )
                  .arg( width )
                  .arg( x[0] )
                  .arg( y[0] )
                  .arg( x[1] )
                  .arg( y[1] )
                  ;
#ifndef QT4
               plot_dist->setCurveData( plotted_hlines[ gaussian_pos ],
                                        (double *)&x[ 0 ],
                                        (double *)&y[ 0 ],
                                        2
                                        );
#else
               plotted_hlines[ gaussian_pos ]->setData(
                                                       (double *)&x[ 0 ],
                                                       (double *)&y[ 0 ],
                                                       2
                                                       );
#endif
            }
         }
      }

      if ( qwtw_wheel->value() != text.toDouble() )
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

void US_Hydrodyn_Saxs_Hplc::gauss_pos_height_text( const QString & text )
{
   if ( gaussians.size() )
   {
      gaussians[ 0 + gaussian_type_size * gaussian_pos ] = text.toDouble();
      if ( qwtw_wheel->value() != text.toDouble() )
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

void US_Hydrodyn_Saxs_Hplc::gauss_pos_dist1_text( const QString & text )
{
   if ( gaussians.size() )
   {
      gaussians[ 3 + gaussian_type_size * gaussian_pos ] = text.toDouble();
      if ( qwtw_wheel->value() != text.toDouble() )
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
   }
}

void US_Hydrodyn_Saxs_Hplc::gauss_pos_dist2_text( const QString & text )
{
   if ( gaussians.size() )
   {
      gaussians[ 4 + gaussian_type_size * gaussian_pos ] = text.toDouble();
      if ( qwtw_wheel->value() != text.toDouble() )
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
   }
}

void US_Hydrodyn_Saxs_Hplc::gauss_fit_start_text( const QString & text )
{
#ifndef QT4
   plot_dist->setMarkerPos( plotted_markers[ 0 ], text.toDouble(), 0e0 );
#else
   plotted_markers[ 0 ]->setXValue( text.toDouble() );
#endif
   if ( qwtw_wheel->value() != text.toDouble() )
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
      plot_dist->replot();
   }
}

void US_Hydrodyn_Saxs_Hplc::gauss_fit_end_text( const QString & text )
{
#ifndef QT4
   plot_dist->setMarkerPos( plotted_markers[ 1 ], text.toDouble(), 0e0 );
#else
   plotted_markers[ 1 ]->setXValue( text.toDouble() );
#endif
   if ( qwtw_wheel->value() != text.toDouble() )
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
      plot_dist->replot();
   }
}

void US_Hydrodyn_Saxs_Hplc::gauss_add_marker( double pos, 
                                              QColor color, 
                                              QString text, 
#ifndef QT4
                                              int 
#else
                                              Qt::Alignment
#endif
                                              align )
{
   int line_width = use_line_width < 3 ? ( use_line_width + 1 ) : use_line_width;

#ifndef QT4
   long marker = plot_dist->insertMarker();
   plot_dist->setMarkerLineStyle ( marker, QwtMarker::VLine );
   plot_dist->setMarkerPos       ( marker, pos, 0e0 );
   plot_dist->setMarkerLabelAlign( marker, align );
   plot_dist->setMarkerPen       ( marker, QPen( color, line_width, DashDotDotLine));
   plot_dist->setMarkerFont      ( marker, QFont("Helvetica", 11, QFont::Bold));
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
      qwtt.setFont( QFont("Helvetica", 11, QFont::Bold ) );
      marker->setLabel           ( qwtt );
   }
   marker->attach             ( plot_dist );
#endif
   plotted_markers.push_back( marker );
}   

void US_Hydrodyn_Saxs_Hplc::gauss_add_hline( double center, double width )
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

   cout << QString( "add_hline %1 %2 (%3,%4) (%5,%6)\n" )
      .arg( center )
      .arg( width )
      .arg( x[0] )
      .arg( y[0] )
      .arg( x[1] )
      .arg( y[1] )
      ;

#ifndef QT4
   long curve;
   curve = plot_dist->insertCurve( "hline" );
   plot_dist->setCurveStyle( curve, QwtCurve::Lines );
#else
   QwtPlotCurve *curve = new QwtPlotCurve( "hline" );
   curve->setStyle( QwtPlotCurve::Lines );
#endif

   plotted_hlines.push_back( curve );

#ifndef QT4
   plot_dist->setCurvePen( curve, QPen( Qt::green, use_line_width, Qt::SolidLine ) );
   plot_dist->setCurveData( curve,
                            (double *)&x[ 0 ],
                            (double *)&y[ 0 ],
                            2
                            );
#else
   curve->setPen( QPen( Qt::green, use_line_width, Qt::SolidLine ) );
   curve->setData(
                  (double *)&x[ 0 ],
                  (double *)&y[ 0 ],
                  2
                  );
   curve->attach( plot_dist );
#endif
}   

void US_Hydrodyn_Saxs_Hplc::gauss_init_markers()
{
   gauss_delete_markers();
   plotted_markers.clear();
   plotted_hlines.clear();

   gauss_add_marker( le_gauss_fit_start->text().toDouble(), Qt::red, tr( "Fit start" ) );
   gauss_add_marker( le_gauss_fit_end  ->text().toDouble(), Qt::red, tr( "Fit end"   ), Qt::AlignLeft | Qt::AlignTop );

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
         if ( cb_fix_width->isChecked() )
         {
            for ( unsigned int i = 0; i < unified_ggaussian_gaussians_size; i++ )
            {
               gauss_add_marker( unified_ggaussian_params[ common_size * i + 0 ], Qt::blue, QString( "%1" ).arg( i + 1 ) );
               gauss_add_hline ( unified_ggaussian_params[ common_size * i + 0 ], unified_ggaussian_params[ common_size * i + 1 ] );
            }
         } else {
            for ( unsigned int i = 0; i < unified_ggaussian_gaussians_size; i++ )
            {
               gauss_add_marker( unified_ggaussian_params[ common_size * i ], Qt::blue, QString( "%1" ).arg( i + 1 ) );
            }
         }
      }
   }
      
   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
}


void US_Hydrodyn_Saxs_Hplc::gauss_delete_markers()
{
#ifndef QT4
   plot_dist->removeMarkers();
#else
   plot_dist->detachItems( QwtPlotItem::Rtti_PlotMarker );
#endif
   for ( unsigned int i = 0; i < ( unsigned int )plotted_hlines.size(); i++ )
   {
#ifndef QT4
      plot_dist->removeCurve( plotted_hlines[ i ] );
#else
      plotted_hlines[ i ]->detach();
#endif
   }
}

vector < double > US_Hydrodyn_Saxs_Hplc::gaussian( double * g ) // height, double center, double width )
{
   vector < double > result;

   if ( !f_qs.count( wheel_file ) )
   {
      editor_msg( "red", QString( tr( "Internal error: %1 not found in data" ) ).arg( wheel_file ) );
      return result;
   }

   if ( !f_qs[ wheel_file ].size() )
   {
      editor_msg( "red", QString( tr( "Internal error: %1 empty data" ) ).arg( wheel_file ) );
      return result;
   }


   vector < double > g_use( gaussian_type_size );
   for ( int i = 0; i < (int) gaussian_type_size; ++i )
   {
      g_use[ i ] = g[ i ];
   }

   return compute_gaussian( f_qs[ wheel_file ], g_use );
}

void US_Hydrodyn_Saxs_Hplc::gauss_delete()
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

void US_Hydrodyn_Saxs_Hplc::gauss_replot_gaussian()
{
   if ( gaussian_pos >= ( unsigned int ) plotted_gaussians.size() )
   {
      // editor_msg( "red", QString( tr( "Internal error: missing plotted gaussian curve" ) ) );
      return;
   }

   vector < double > x = f_qs[ wheel_file ];
   vector < double > y = gaussian( &(gaussians[ gaussian_pos * gaussian_type_size ] ) );

#ifndef QT4
   plot_dist->setCurveData( plotted_gaussians[ gaussian_pos ],
                            (double *)&x[ 0 ],
                            (double *)&y[ 0 ],
                            x.size()
                            );
#else
   plotted_gaussians[ gaussian_pos ]->setData(
                                              (double *)&x[ 0 ],
                                              (double *)&y[ 0 ],
                                              x.size()
                                              );

#endif
   replot_gaussian_sum();
}

void US_Hydrodyn_Saxs_Hplc::gauss_add_gaussian( double *g, QColor color )
   // double height, double center, double width, QColor color )
{
   vector < double > x = f_qs[ wheel_file ];
   vector < double > y = gaussian( g );

#ifndef QT4
   long curve;
   curve = plot_dist->insertCurve( "gaussian" );
   plot_dist->setCurveStyle( curve, QwtCurve::Lines );
#else
   QwtPlotCurve *curve = new QwtPlotCurve( "gaussian" );
   curve->setStyle( QwtPlotCurve::Lines );
#endif

   plotted_gaussians.push_back( curve );

#ifndef QT4
   plot_dist->setCurveData( curve, 
                            (double *)&x[ 0 ],
                            (double *)&y[ 0 ],
                            x.size()
                            );
   plot_dist->setCurvePen( curve, QPen( color , use_line_width, Qt::DashLine ) );
#else
   curve->setData(
                  (double *)&x[ 0 ],
                  (double *)&y[ 0 ],
                  x.size()
                  );

   curve->setPen( QPen( color, use_line_width, Qt::DashLine ) );
   curve->attach( plot_dist );
#endif
}

void US_Hydrodyn_Saxs_Hplc::gauss_init_gaussians()
{
   gauss_delete_gaussians();
   plotted_gaussians.clear();
   plotted_gaussian_sum.clear();

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

void US_Hydrodyn_Saxs_Hplc::gauss_delete_gaussians()
{
   for ( unsigned int i = 0; i < ( unsigned int ) plotted_gaussians.size(); i++ )
   {
#ifndef QT4
      plot_dist->removeCurve( plotted_gaussians[ i ] );
#else
      plotted_gaussians[ i ]->detach();
#endif
   }
   for ( unsigned int i = 0; i < ( unsigned int ) plotted_gaussian_sum.size(); i++ )
   {
#ifndef QT4
      plot_dist->removeCurve( plotted_gaussian_sum[ i ] );
#else
      plotted_gaussian_sum[ i ]->detach();
#endif
   }
}

void US_Hydrodyn_Saxs_Hplc::gauss_pos_focus( bool hasFocus )
{
   cout << QString( "gauss_pos_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      update_gauss_pos();
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], 
                            f_qs[ wheel_file ].back(), 
                            ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      qwtw_wheel->setValue( le_gauss_pos->text().toDouble() );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
   }
}

void US_Hydrodyn_Saxs_Hplc::gauss_pos_width_focus( bool hasFocus )
{
   cout << QString( "gauss_pos_width_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      update_gauss_pos();
      qwtw_wheel->setRange( 0, 
                            f_qs[ wheel_file ].back() / 3, 
                            ( f_qs[ wheel_file ].back() / 3 ) / UHSH_WHEEL_RES ), 
      qwtw_wheel->setValue( le_gauss_pos_width->text().toDouble() );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
   }
}

void US_Hydrodyn_Saxs_Hplc::gauss_pos_height_focus( bool hasFocus )
{
   cout << QString( "gauss_pos_height_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      update_gauss_pos();
      qwtw_wheel->setRange( 0,
                            gauss_max_height,
                            gauss_max_height / UHSH_WHEEL_RES );
      qwtw_wheel->setValue( le_gauss_pos_height->text().toDouble() );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
   }
}

void US_Hydrodyn_Saxs_Hplc::gauss_pos_dist1_focus( bool hasFocus )
{
   cout << QString( "gauss_pos_dist1_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      update_gauss_pos();
      qwtw_wheel->setRange( -50e0,
                            50e0,
                            100e0 / UHSH_WHEEL_RES );
      qwtw_wheel->setValue( le_gauss_pos_dist1->text().toDouble() );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
   }
}

void US_Hydrodyn_Saxs_Hplc::gauss_pos_dist2_focus( bool hasFocus )
{
   cout << QString( "gauss_pos_dist2_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      update_gauss_pos();
      qwtw_wheel->setRange( -50e0,
                            50e0,
                            100e0 / UHSH_WHEEL_RES );
      qwtw_wheel->setValue( le_gauss_pos_dist2->text().toDouble() );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
   }
}

void US_Hydrodyn_Saxs_Hplc::gauss_fit_start_focus( bool hasFocus )
{
   cout << QString( "gauss_fit_start_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      update_gauss_pos();
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], 
                            f_qs[ wheel_file ].back(), 
                            ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_gauss_fit_start->text().toDouble() );
   }
}

void US_Hydrodyn_Saxs_Hplc::gauss_fit_end_focus( bool hasFocus )
{
   cout << QString( "gauss_fit_end_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      update_gauss_pos();
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], 
                            f_qs[ wheel_file ].back(), 
                            ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_gauss_fit_end->text().toDouble() );
   }
}

void US_Hydrodyn_Saxs_Hplc::plot_gaussian_sum()
{
   // add up the curve data
   vector < double > x = f_qs[ wheel_file ];
   vector < double > y = x;

   if ( plotted_gaussians.size() )
   {
      for ( unsigned int j = 0; j < x.size(); j++ )
      {
#ifndef QT4
         y[ j ] = plot_dist->curve( plotted_gaussians[ 0 ] )->y( j );
#else
         y[ j ] = plotted_gaussians[ 0 ]->y( j );
#endif
      }

      for ( unsigned int i = 1; i < plotted_gaussians.size(); i++ )
      {
         for ( unsigned int j = 0; j < x.size(); j++ )
         {
#ifndef QT4
            y[ j ] += plot_dist->curve( plotted_gaussians[ i ] )->y( j );
#else
            y[ j ] += plotted_gaussians[ i ]->y( j );
#endif
         }
      }
   } else {
      for ( unsigned int j = 0; j < x.size(); j++ )
      {
         y[ j ] = 0e0;
      }
   }

#ifndef QT4
   long curve;
   curve = plot_dist->insertCurve( "gaussian_sum" );
   plot_dist->setCurveStyle( curve, QwtCurve::Lines );
#else
   QwtPlotCurve *curve = new QwtPlotCurve( "gaussian_sum" );
   curve->setStyle( QwtPlotCurve::Lines );
#endif

   plotted_gaussian_sum.push_back( curve );

#ifndef QT4
   plot_dist->setCurveData( curve, 
                            (double *)&x[ 0 ],
                            (double *)&y[ 0 ],
                            x.size()
                            );
   plot_dist->setCurvePen( curve, QPen( Qt::yellow , use_line_width, Qt::DashLine ) );
#else
   curve->setData(
                  (double *)&x[ 0 ],
                  (double *)&y[ 0 ],
                  x.size()
                  );

   curve->setPen( QPen( Qt::yellow, use_line_width, Qt::DashLine ) );
   curve->attach( plot_dist );
#endif
   replot_gaussian_sum();
}

void US_Hydrodyn_Saxs_Hplc::replot_gaussian_sum()
{
   // add up the curve data
   if ( !plotted_gaussian_sum.size() )
   {
      editor_msg( "red", QString( tr( "Internal error: replot gaussian sum: no gaussian sums" ) ) );
      return;
   }

   vector < double > x = f_qs[ wheel_file ];
   vector < double > y = x;

   if ( plotted_gaussians.size() )
   {
      for ( unsigned int j = 0; j < x.size(); j++ )
      {
#ifndef QT4
         y[ j ] = plot_dist->curve( plotted_gaussians[ 0 ] )->y( j );
#else
         y[ j ] = plotted_gaussians[ 0 ]->y( j );
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
#ifndef QT4
         y[ j ] += plot_dist->curve( plotted_gaussians[ i ] )->y( j );
#else
         y[ j ] += plotted_gaussians[ i ]->y( j );
#endif
      }
   }

#ifndef QT4
   plot_dist->setCurveData( plotted_gaussian_sum[ 0 ],
                            (double *)&x[ 0 ],
                            (double *)&y[ 0 ],
                            x.size()
                            );
#else
   plotted_gaussian_sum[ 0 ]->setData(
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
      for ( unsigned int j = 0; j < x.size(); j++ )
      {
         if ( x[ j ] >= start && x[ j ] <= end )
         {
            double tmp = ( y[ j ] - f_Is[ wheel_file ][ j ] ) / f_errors[ wheel_file ][ j ];
            rmsd += tmp * tmp;
         }
      }
   } else {

      for ( unsigned int j = 0; j < x.size(); j++ )
      {
         if ( x[ j ] >= start && x[ j ] <= end )
         {
            rmsd += ( y[ j ] - f_Is[ wheel_file ][ j ] ) * ( y[ j ] - f_Is[ wheel_file ][ j ] );
         }
      }
   }

   lbl_gauss_fit->setText( QString( " %1 " ).arg( sqrt( rmsd ), 0, 'g', 5 ) );
   vector < double > empty;
   update_plot_errors( x, y, f_Is[ wheel_file ], f_errors.count( wheel_file ) ? f_errors[ wheel_file ] : empty );
}


void US_Hydrodyn_Saxs_Hplc::gauss_fit()
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

      qwtw_wheel->setEnabled( false );
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      US_Hydrodyn_Saxs_Hplc_Fit_Global *shfg = 
         new US_Hydrodyn_Saxs_Hplc_Fit_Global(
                                              this,
                                              this );
      US_Hydrodyn::fixWinButtons( shfg );
      shfg->exec();
      delete shfg;
      
      qwtw_wheel->setEnabled( true );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      ggaussian_enables();
      return;
   }

   if ( !plotted_gaussian_sum.size() )
   {
      editor_msg( "red", QString( tr( "Internal error: gaussian fit: no gaussian sums" ) ) );
      return;
   }

   if ( !gaussians.size() )
   {
      editor_msg( "red", QString( tr( "Internal error: gaussian fit: no gaussians" ) ) );
      return;
   }

   qwtw_wheel->setEnabled( false );
   disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
   US_Hydrodyn_Saxs_Hplc_Fit *shf = 
      new US_Hydrodyn_Saxs_Hplc_Fit(
                                    this,
                                    this );
   US_Hydrodyn::fixWinButtons( shf );
   shf->exec();
   delete shf;
   
   qwtw_wheel->setEnabled( true );
   connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
   return;
   /*   
        cout << "gauss fit start\n";

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

void US_Hydrodyn_Saxs_Hplc::create_i_of_q()
{
   disable_all();

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         if ( lb_files->text( i ) == lbl_conc_file->text() )
         {
            lb_files->setSelected( i, false );
         }
      }
   }
              
   QStringList files = all_selected_files();
   gaussians.size() ? (void) create_i_of_q( files ) : (void) create_i_of_q_ng( files );

   update_enables();
}

vector < double > US_Hydrodyn_Saxs_Hplc::compute_gaussian_sum( vector < double > t, vector < double > g )
{
   vector < double > result( t.size() );
   for ( unsigned int i = 0; i < ( unsigned int ) result.size(); i++ )
   {
      result[ i ] = 0e0;
   }

   vector < double > g_use( gaussian_type_size );
   vector < double > result_partial;

   for ( unsigned int j = 0; j < ( unsigned int ) g.size(); j += gaussian_type_size )
   {
      for ( int i = 0; i < (int) gaussian_type_size; ++i )
      {
         g_use[ i ] = g[ i + j ];
      }
      result_partial = compute_gaussian( t, g_use );
      for ( unsigned int i = 0; i < ( unsigned int ) t.size(); i++ )
      {
         result[ i ] += result_partial[ i ];
      }
   }
   // US_Vector::printvector( QString( "gaussian sum: g" ), g );
   // US_Vector::printvector2( QString( "gaussian sum: type %1 g.size() %2" ).arg( gaussian_type_size ).arg( g.size() ), t, result );
   return result;
}

vector < double > US_Hydrodyn_Saxs_Hplc::compute_gaussian( vector < double > t, vector < double > g )
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
   cout << QString( "compute_gaussian use_qt is %1 dist1 %2 %3 dist2 %4 %5\n" )
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
         // cout << "gaussian as: GAUSS\n";
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
         // cout << "gaussian as: GMG\n";

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
         // cout << "gaussian as: EMG\n";
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
         // cout << "gaussian as: EMGGMG\n";
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

double US_Hydrodyn_Saxs_Hplc::compute_gaussian_peak( QString file, vector < double > g )
{
   // cout << QString( "gaussian peak file %1 current type %2\n" ).arg( file ).arg( gaussian_type );
   vector < double > gs = compute_gaussian_sum( f_qs[ file ], g );
   double gmax = gs[ 0 ];
   for ( unsigned int i = 1; i < ( unsigned int ) gs.size(); i++ )
   {
      if ( gmax < gs[ i ] )
      {
         gmax = gs[ i ];
      }
   }
   return gmax;
}

QString US_Hydrodyn_Saxs_Hplc::pad_zeros( int val, int max )
{
   unsigned int len = QString( "%1" ).arg( max ).length();
   QString      s   = QString( "%1" ).arg( val );
   while ( (unsigned int) s.length() < len )
   {
      s = "0" + s;
   }
   return s;
}

void US_Hydrodyn_Saxs_Hplc::ggauss_start()
{
   plot_errors->clear();
   if ( plot_errors_zoomer )
   {
      delete plot_errors_zoomer;
      plot_errors_zoomer = (ScrollZoomer *) 0;
   }

   le_last_focus = (mQLineEdit *) 0;
   pb_gauss_fit->setText( tr( "Global Fit" ) );

   disable_all();

   if ( !ggaussian_compatible() )
   {
      QString msg_addendum = "";
      if ( cb_fix_width->isChecked() )
      {
         msg_addendum += " or widths";
      }
      if ( dist1_active && cb_fix_dist1->isChecked() )
      {
         msg_addendum += " or distortion-1s";
      }
      if ( dist2_active && cb_fix_dist2->isChecked() )
      {
         msg_addendum += " or distortion-2s";
      }
      QString msg = QString( tr( "NOTICE: Some files selected have Gaussians with varying centers%1 or a different number of Gaussians or centers that do not match the last Gaussians, these will be reset to the last Gaussian settings" ) )
         .arg( msg_addendum );

      editor_msg( "dark red", msg );
   }

   if ( !create_unified_ggaussian_target() )
   {
      update_enables();
      return;
   }

   // add_ggaussian_curve( "unified_ggaussian_target", unified_ggaussian_I );
   // add_ggaussian_curve( "unified_ggaussian_sum",    compute_ggaussian_gaussian_sum() );

   // ggaussian_mode = true;
   mode_select( MODE_GGAUSSIAN );

   lbl_gauss_fit->setText( QString( "%1" ).arg( ggaussian_rmsd(), 0, 'g', 5 ) );
   wheel_file = unified_ggaussian_files[ 0 ];

   get_peak( wheel_file, gauss_max_height );
   gauss_max_height *= 1.2;
   if ( gaussian_type != GAUSS )
   {
      gauss_max_height *= 20e0;
   }
      
   org_f_gaussians = f_gaussians;

   running        = true;

   if ( gaussian_pos && gaussian_pos >= unified_ggaussian_gaussians_size )
   {
      gaussian_pos = unified_ggaussian_gaussians_size - 1;
   }
   
   gauss_init_markers();
   disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
   qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ],
                         f_qs[ wheel_file ].back(),
                         ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
   qwtw_wheel->setValue( unified_ggaussian_params[ gaussian_pos ] );
   update_gauss_pos();
   connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );

   check_fit_range();

   if ( errors_were_on )
   {
      hide_widgets( plot_errors_widgets, false );
      if ( !unified_ggaussian_use_errors )
      {
         cb_plot_errors_sd->setChecked( false );
         cb_plot_errors_sd->hide();
      }
   }

   ggaussian_enables();
}

void US_Hydrodyn_Saxs_Hplc::ggaussian_enables()
{
   pb_ggauss_start     ->setEnabled( false );
   pb_gauss_prev       ->setEnabled( unified_ggaussian_gaussians_size > 1 && gaussian_pos > 0 );
   pb_gauss_next       ->setEnabled( unified_ggaussian_gaussians_size > 1 && gaussian_pos < unified_ggaussian_gaussians_size - 1 );
   cb_sd_weight        ->setEnabled( unified_ggaussian_gaussians_size && le_gauss_fit_start->text().toDouble() < le_gauss_fit_end->text().toDouble() );
   pb_gauss_fit        ->setEnabled( unified_ggaussian_gaussians_size && le_gauss_fit_start->text().toDouble() < le_gauss_fit_end->text().toDouble() );
   pb_wheel_cancel     ->setEnabled( true );
   le_gauss_pos        ->setEnabled( unified_ggaussian_gaussians_size && gaussian_pos < unified_ggaussian_gaussians_size );
   le_gauss_pos_width  ->setEnabled( cb_fix_width->isChecked() && unified_ggaussian_gaussians_size && gaussian_pos < unified_ggaussian_gaussians_size );
   le_gauss_pos_dist1  ->setEnabled( cb_fix_dist1->isChecked() && unified_ggaussian_gaussians_size && gaussian_pos < unified_ggaussian_gaussians_size );
   le_gauss_pos_dist2  ->setEnabled( cb_fix_dist2->isChecked() && unified_ggaussian_gaussians_size && gaussian_pos < unified_ggaussian_gaussians_size );
   le_gauss_fit_start  ->setEnabled( unified_ggaussian_gaussians_size && gaussian_pos < unified_ggaussian_gaussians_size );
   le_gauss_fit_end    ->setEnabled( unified_ggaussian_gaussians_size && gaussian_pos < unified_ggaussian_gaussians_size );
   pb_wheel_save       ->setEnabled( unified_ggaussian_gaussians_size );
   qwtw_wheel          ->setEnabled( unified_ggaussian_gaussians_size && gaussian_pos < unified_ggaussian_gaussians_size );
   pb_ggauss_results   ->setEnabled( unified_ggaussian_ok );
   pb_gauss_save       ->setEnabled( unified_ggaussian_ok );
   pb_gauss_as_curves  ->setEnabled( unified_ggaussian_ok );
   pb_rescale          ->setEnabled( true );
   pb_view             ->setEnabled( unified_ggaussian_curves <= 10 );
   pb_errors           ->setEnabled( true );
}

QStringList US_Hydrodyn_Saxs_Hplc::all_selected_files()
{
   QStringList files;
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         files << lb_files->text( i );
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

set < QString > US_Hydrodyn_Saxs_Hplc::all_selected_files_set()
{
   set < QString > result;
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         result.insert( lb_files->text( i ) );
      }
   }
   return result;
}

QStringList US_Hydrodyn_Saxs_Hplc::all_files()
{
   QStringList files;
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      files << lb_files->text( i );
   }
   return files;
}

map < QString, bool > US_Hydrodyn_Saxs_Hplc::all_files_map()
{
   map < QString, bool > files;
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      files[ lb_files->text( i ) ] = true;
   }
   return files;
}

bool US_Hydrodyn_Saxs_Hplc::ggaussian_compatible( bool check_against_global )
{
   QStringList files = all_selected_files();
   return ggaussian_compatible( files, check_against_global );
}
   
bool US_Hydrodyn_Saxs_Hplc::ggaussian_compatible( set < QString > & selected, bool check_against_global )
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
      
bool US_Hydrodyn_Saxs_Hplc::ggaussian_compatible( QStringList & files, bool check_against_global )
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
                                          caption(),
                                          QString( tr( "Please note:\n\n"
                                                       "The file specific Gaussians are internally consistent, "
                                                       "but they do not match the last Gaussians in terms of count %1\n"
                                                       "What would you like to do?\n" ) )
                                          .arg( cb_fix_width->isChecked() ?
                                                ", centers or widths." : " or centers." ),
                                          tr( "&Set the last Gaussians to the file specific ones" ), 
                                          tr( "&Overwrite the file specific Gaussians with the last Gaussians" ),
                                          QString::null,
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

            //          cout << QString( "for pos %1 t is %2 index %3 gaussian %4 center %5 height %6 width %7\n" )
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

            //          cout << QString( "for pos %1 t is %2 index %3 gaussian %4 center %5 height %6 width %7\n" )
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


void US_Hydrodyn_Saxs_Hplc::ggauss_rmsd()
{
   if ( ggauss_recompute() )
   {
      lbl_gauss_fit->setText( QString( "%1" ).arg( ggaussian_rmsd(), 0, 'g', 5 ) );
   }
}

void US_Hydrodyn_Saxs_Hplc::add_ggaussian_curve( QString name, vector < double > y )
{
   if ( y.size() != unified_ggaussian_q.size() )
   {
      editor_msg( "red", QString( tr( "Internal error: add_ggaussian_curve size %1 but unified ggaussian q size %2" ) ).arg( y.size() ).arg( unified_ggaussian_q.size() ) );
   }

   map < QString, bool > current_files = all_files_map();

   QString use_name = name;

   unsigned int ext = 0;
   while ( current_files.count( use_name ) )
   {
      use_name = name + QString( "-%1" ).arg( ++ext );
   }

   lb_created_files->insertItem( use_name );
   lb_created_files->setBottomItem( lb_created_files->numRows() - 1 );
   lb_files->insertItem( use_name );
   lb_files->setBottomItem( lb_files->numRows() - 1 );
   created_files_not_saved[ use_name ] = true;
   
   f_pos       [ use_name ] = f_qs.size();
   f_qs        [ use_name ] = unified_ggaussian_t;
   f_qs_string [ use_name ].clear();
   for ( unsigned int i = 0; i < ( unsigned int ) unified_ggaussian_t.size(); i++ )
   {
      f_qs_string [ use_name ].push_back( QString( "%1" ).arg( unified_ggaussian_t[ i ] ) );
   }
   f_Is        [ use_name ] = y;
   f_errors    [ use_name ].clear();
   f_is_time   [ use_name ] = true;
   f_psv       [ use_name ] = 0e0;
   f_I0se      [ use_name ] = 0e0;
   f_conc      [ use_name ] = 0e0;
   {
      vector < double > tmp;
      f_gaussians  [ use_name ] = tmp;
   }
}

void US_Hydrodyn_Saxs_Hplc::ggauss_results()
{
   if ( unified_ggaussian_ok )
   {
      add_ggaussian_curve( tr( "joined_target" )   , unified_ggaussian_I );
      add_ggaussian_curve( tr( "joined_gaussians" ), compute_ggaussian_gaussian_sum() );
   }
}

void US_Hydrodyn_Saxs_Hplc::dir_pressed()
{
   QString use_dir = lbl_dir->text();
   ((US_Hydrodyn  *)us_hydrodyn)->select_from_directory_history( use_dir, this );
   QString s = QFileDialog::getExistingDirectory( this , tr( "Choose a new base directory" ) , use_dir , QFileDialog::ShowDirsOnly );

   if ( !s.isEmpty() )
   {
      QDir::setCurrent( s );
      lbl_dir->setText(  QDir::currentDirPath() );
      ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( s );
   }
}

void US_Hydrodyn_Saxs_Hplc::created_dir_pressed()
{
   QString use_dir = lbl_dir->text();
   ((US_Hydrodyn  *)us_hydrodyn)->select_from_directory_history( use_dir, this );
   QString s = QFileDialog::getExistingDirectory( this , tr( "Choose a new base directory for saving files" ) , use_dir , QFileDialog::ShowDirsOnly );

   if ( !s.isEmpty() )
   {
      lbl_created_dir->setText( s );
      ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( s );
   }
}

void US_Hydrodyn_Saxs_Hplc::gauss_as_curves()
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
            for ( int k = 0; k < gaussian_type_size; ++k )
            {
               if ( is_common[ k ] )
               {
                  g.push_back( unified_ggaussian_params[ offset[ k ] + common_size * j           ] );
               } else {
                  g.push_back( unified_ggaussian_params[ offset[ k ] + per_file_size * j + index ] );
               }
            }               
            for ( unsigned int j = 0; j < ( unsigned int ) g.size(); j+= gaussian_type_size )
            {
               vector < double > tmp_g( gaussian_type_size );
               for ( int k = 0; k < gaussian_type_size; k++ )
               {
                  tmp_g[ k ] = g[ k + j ];
               }
               add_plot( unified_ggaussian_files[ i ] + QString( "_pk%1" ).arg( ( j / gaussian_type_size ) + 1 ),
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
         
         /* old way

         if ( cb_fix_width->isChecked() )
         {
         unsigned int  index = 2 * unified_ggaussian_gaussians_size + i * unified_ggaussian_gaussians_size;

         for ( unsigned int j = 0; j < unified_ggaussian_gaussians_size; j++ )
         {
         g.push_back( unified_ggaussian_params[ index + j + 0 ] );
         g.push_back( unified_ggaussian_params[ 2 * j + 0 ] );
         g.push_back( unified_ggaussian_params[ 2 * j + 1 ] );
         }
         for ( unsigned int j = 0; j < ( unsigned int ) g.size(); j+= gaussian_type_size )
         {
         vector < double > tmp_g( gaussian_type_size );
         for ( int k = 0; k < gaussian_type_size; k++ )
         {
         tmp_g[ k ] = g[ k + j ];
         }
         add_plot( unified_ggaussian_files[ i ] + QString( "_pk%1" ).arg( ( j / gaussian_type_size ) + 1 ),
         f_qs[ unified_ggaussian_files[ i ] ],
         compute_gaussian( f_qs[ unified_ggaussian_files[ i ] ], tmp_g ),
         true,
         false );
         }
         add_plot( unified_ggaussian_files[ i ] + QString( "_pksum" ),
         f_qs[ unified_ggaussian_files[ i ] ],
         compute_gaussian_sum( f_qs[ unified_ggaussian_files[ i ] ], g ),
         true,
         false );
         } else {
         unsigned int  index = unified_ggaussian_gaussians_size + i * 2 * unified_ggaussian_gaussians_size;

         for ( unsigned int j = 0; j < unified_ggaussian_gaussians_size; j++ )
         {
         g.push_back( unified_ggaussian_params[ index + 2 * j + 0 ] );
         g.push_back( unified_ggaussian_params[ j ] );
         g.push_back( unified_ggaussian_params[ index + 2 * j + 1 ] );
         }
         for ( unsigned int j = 0; j < ( unsigned int ) g.size(); j+= gaussian_type_size )
         {
         vector < double > tmp_g( gaussian_type_size );
         for ( int k = 0; k < gaussian_type_size; k++ )
         {
         tmp_g[ k ] = g[ k + j ];
         }
         add_plot( unified_ggaussian_files[ i ] + QString( "_pk%1" ).arg( ( j / gaussian_type_size ) + 1 ),
         f_qs[ unified_ggaussian_files[ i ] ],
         compute_gaussian( f_qs[ unified_ggaussian_files[ i ] ], tmp_g ),
         true,
         false );
         }
         add_plot( unified_ggaussian_files[ i ] + QString( "_pksum" ),
         f_qs[ unified_ggaussian_files[ i ] ],
         compute_gaussian_sum( f_qs[ unified_ggaussian_files[ i ] ], g ),
         true,
         false );
         }

         */
      }
   }
}

void US_Hydrodyn_Saxs_Hplc::set_sd_weight()
{
   if ( current_mode == MODE_GAUSSIAN )
   {
      replot_gaussian_sum();
   } else {
      plot_errors->clear();
      lbl_gauss_fit ->setText( "?" );
      if ( current_mode == MODE_GGAUSSIAN )
      {
         pb_ggauss_rmsd->setEnabled( true );
         plot_errors      ->clear();
         if ( !suppress_replot )
         {
            plot_errors      ->replot();
         }
         plot_errors_grid  .clear();
         plot_errors_target.clear();
         plot_errors_fit   .clear();
         plot_errors_errors.clear();
      }
   }
}

void US_Hydrodyn_Saxs_Hplc::set_fix_width()
{
}


void US_Hydrodyn_Saxs_Hplc::set_fix_dist1()
{
}

void US_Hydrodyn_Saxs_Hplc::set_fix_dist2()
{
}

bool US_Hydrodyn_Saxs_Hplc::check_fit_range()
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
                            this->caption() + tr( ": Fit range" ),
                            QString( tr( "Warning: %1 of %2 Gaussian centers are outside of the fit range" ) )
                            .arg( count )
                            .arg( gaussians.size() / gaussian_type_size ) );
      return false;
   }
   return true;
}

void US_Hydrodyn_Saxs_Hplc::hide_widgets( vector < QWidget *> widgets, bool hide )
{
   for ( unsigned int i = 0; i < ( unsigned int ) widgets.size(); i++ )
   {
      hide ? widgets[ i ]->hide() : widgets[ i ]->show();
   }
}

void US_Hydrodyn_Saxs_Hplc::save_state()
{
   QString use_dir = QDir::currentDirPath();
   
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
      ((US_Hydrodyn  *)us_hydrodyn)->add_to_directory_history( lbl_dir->text() );
   }
   ((US_Hydrodyn  *)us_hydrodyn)->select_from_directory_history( use_dir, this );
   raise();
   // }


   QString fn = QFileDialog::getSaveFileName( this , tr( "Select a name to save the state" ) , use_dir , "*.dat" );

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

   Q3TextStream ts( &f );

   // write out all the loaded file names (for each stack element)

   ts << "# US-SOMO Global State file\n";

   if ( detector_uv_conv != 0e0 )
   {
      ts << "# __detector_uv: " << QString( "%1" ).arg( detector_uv_conv, 0, 'g', 8 ) << endl;
   }
   if ( detector_ri_conv != 0e0 )
   {
      ts << "# __detector_ri: " << QString( "%1" ).arg( detector_ri_conv, 0, 'g', 8 ) << endl;
   }
   if ( detector_uv )
   {
      ts << "# __detector_uv_set" << endl;
   } 
   if ( detector_ri )
   {
      ts << "# __detector_ri_set" << endl;
   } 

   ts << "# __gaussian_type: " << gaussian_type_tag << endl;

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
               ts << it->second << endl;
            }
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
               ts << "# __f_gaussians: " << it->first << endl;
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
         ts << it->second << endl;
      }
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
      ts << "# __f_gaussians: " << it->first << endl;
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

   ts << "# __dir: " << lbl_dir->text() << endl;
   if ( cb_lock_dir->isChecked() )
   {
      ts << "# __lock_dir\n";
   }

   ts << "# __created_dir: " << lbl_created_dir->text() << endl;

   f.close();
   editor_msg( "blue", QString( tr( "State saved in file %1" ) ).arg( fn ) );
}

void US_Hydrodyn_Saxs_Hplc::update_gauss_mode()
{
   cout << QString( "update_gauss_mode <%1>\n" ).arg( gaussian_type );
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

   int percharwidth = 1 + ( 7 * ( USglobal->config_list.fontSize - 1 ) / 10 );
   pb_gauss_start->setMaximumWidth( percharwidth * ( pb_gauss_start->text().length() + 3 ) );
}

