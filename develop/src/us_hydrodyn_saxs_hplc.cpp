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
//       cout << QString( " %1" ).arg( x[ i ], 0, 'g', 8 );
//    }
//    cout << endl;
// }

US_Hydrodyn_Saxs_Hplc::US_Hydrodyn_Saxs_Hplc(
                                             csv csv1,
                                             void *us_hydrodyn, 
                                             QWidget *p, 
                                             const char *name
                                             ) : QFrame(p, name)
{
   this->csv1 = csv1;
   this->us_hydrodyn = us_hydrodyn;
   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("US-SOMO: SAXS Hplc"));
   order_ascending = false;
   conc_widget     = false;
#ifdef QT4
   legend_vis      = true;
#endif
   usu             = new US_Saxs_Util();

   QDir::setCurrent( ((US_Hydrodyn *)us_hydrodyn)->somo_dir + QDir::separator() + "saxs" );

   saxs_widget = &(((US_Hydrodyn *) us_hydrodyn)->saxs_plot_widget);
   saxs_window = ((US_Hydrodyn *) us_hydrodyn)->saxs_plot_window;
   ((US_Hydrodyn *) us_hydrodyn)->saxs_hplc_widget = true;

   errors_were_on = false;

   plot_dist_zoomer   = (ScrollZoomer *) 0;
   plot_errors_zoomer = (ScrollZoomer *) 0;

   le_last_focus      = (mQLineEdit *) 0;
   
   disable_updates = false;
   plot3d_flag     = false;

   setupGUI();
   running = false;
   axis_y_log = true;
   axis_y();
   axis_x_log = false;

   gaussian_mode  = false;
   ggaussian_mode = false;
   baseline_mode  = false;
   timeshift_mode = false;

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
      pbs.push_back( pb_invert );
      pbs.push_back( pb_adjacent );
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
   pb_help         ->setMinimumWidth( csv_width / 3 );
   pb_cancel       ->setMinimumWidth( csv_width / 3 );

   setGeometry(global_Xpos, global_Ypos, csv_width, 100 + csv_height );
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

void US_Hydrodyn_Saxs_Hplc::setupGUI()
{
   int minHeight1 = 24;
   int minHeight3 = 25;

   lbl_files = new QLabel("Data files", this);
   lbl_files->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_files->setMinimumHeight(minHeight1);
   lbl_files->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cb_lock_dir = new QCheckBox(this);
   cb_lock_dir->setText(tr("Lock "));
   cb_lock_dir->setEnabled( true );
   cb_lock_dir->setChecked( false );
   cb_lock_dir->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ) );
   cb_lock_dir->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   lbl_dir = new mQLabel( QDir::currentDirPath(), this );
   lbl_dir->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_dir->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_dir->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2));
   connect( lbl_dir, SIGNAL(pressed()), SLOT( dir_pressed() ));

   pb_add_files = new QPushButton(tr("Add files"), this);
   pb_add_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_add_files->setMinimumHeight(minHeight3);
   pb_add_files->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_add_files, SIGNAL(clicked()), SLOT(add_files()));

   pb_similar_files = new QPushButton(tr("Similar"), this);
   pb_similar_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_similar_files->setMinimumHeight(minHeight3);
   pb_similar_files->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_similar_files, SIGNAL(clicked()), SLOT(similar_files()));

   pb_conc = new QPushButton(tr("Concentrations"), this);
   pb_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_conc->setMinimumHeight(minHeight3);
   pb_conc->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_conc, SIGNAL(clicked()), SLOT(conc()));

   pb_clear_files = new QPushButton(tr("Remove files"), this);
   pb_clear_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_clear_files->setMinimumHeight(minHeight3);
   pb_clear_files->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_clear_files, SIGNAL(clicked()), SLOT(clear_files()));

   pb_regex_load = new QPushButton(tr("RL"), this);
   pb_regex_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_regex_load->setMinimumHeight(minHeight3);
   pb_regex_load->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_regex_load, SIGNAL(clicked()), SLOT(regex_load()));

   le_regex = new QLineEdit(this, "le_regex Line Edit");
   le_regex->setText( "" );
   le_regex->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_regex->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_regex->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_regex_args = new QLineEdit(this, "le_regex_args Line Edit");
   le_regex_args->setText( "" );
   le_regex_args->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_regex_args->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_regex_args->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   pb_save_state = new QPushButton(tr("SS"), this);
   pb_save_state->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_save_state->setMinimumHeight(minHeight3);
   pb_save_state->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_save_state, SIGNAL(clicked()), SLOT(save_state()));

   lb_files = new QListBox(this, "files files listbox" );
   lb_files->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lb_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lb_files->setEnabled(true);
   lb_files->setSelectionMode( QListBox::Extended );
   lb_files->setMinimumHeight( minHeight1 * 8 );
   connect( lb_files, SIGNAL( selectionChanged() ), SLOT( update_files() ) );

   lbl_selected = new QLabel("0 files selected", this );
   lbl_selected->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_selected->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_selected->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2));

   pb_select_all = new mQPushButton(tr("Select all"), this);
   pb_select_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_select_all->setMinimumHeight(minHeight1);
   pb_select_all->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_select_all, SIGNAL(clicked()), SLOT(select_all()));

   pb_invert = new QPushButton(tr("Invert"), this);
   pb_invert->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_invert->setMinimumHeight(minHeight1);
   pb_invert->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_invert, SIGNAL(clicked()), SLOT(invert()));

   pb_select_nth = new mQPushButton(tr("Nth"), this);
   pb_select_nth->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_select_nth->setMinimumHeight(minHeight1);
   pb_select_nth->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_select_nth, SIGNAL(clicked()), SLOT(select_nth()));

   pb_color_rotate = new QPushButton(tr("Color"), this);
   pb_color_rotate->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_color_rotate->setMinimumHeight(minHeight1);
   pb_color_rotate->setMaximumWidth ( minHeight1 * 2 );
   pb_color_rotate->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_color_rotate, SIGNAL(clicked()), SLOT(color_rotate()));

   //    pb_join = new QPushButton(tr("J"), this);
   //    pb_join->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   //    pb_join->setMinimumHeight( minHeight1 );
   //    pb_join->setMaximumWidth ( minHeight1 * 2 );
   //    pb_join->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   //    connect(pb_join, SIGNAL(clicked()), SLOT(join()));

   pb_adjacent = new QPushButton(tr("Similar"), this);
   pb_adjacent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_adjacent->setMinimumHeight(minHeight1);
   pb_adjacent->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_adjacent, SIGNAL(clicked()), SLOT(adjacent()));

   pb_to_saxs = new QPushButton(tr("To SOMO/SAS"), this);
   pb_to_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_to_saxs->setMinimumHeight( minHeight1 );
   // pb_to_saxs->setMaximumWidth ( minHeight1 * 2 );
   pb_to_saxs->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_to_saxs, SIGNAL(clicked()), SLOT(to_saxs()));

   pb_view = new QPushButton(tr("View"), this);
   pb_view->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_view->setMinimumHeight( minHeight1 );
   pb_view->setMaximumWidth ( minHeight1 * 4 );
   pb_view->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_view, SIGNAL(clicked()), SLOT( view() ));

   pb_rescale = new QPushButton(tr("Rescale"), this);
   pb_rescale->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_rescale->setMinimumHeight(minHeight1);
   pb_rescale->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_rescale, SIGNAL(clicked()), SLOT(rescale()));

   pb_stack_push_all = new QPushButton(tr("Psh"), this);
   pb_stack_push_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_push_all->setMinimumHeight(minHeight1);
   pb_stack_push_all->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_stack_push_all, SIGNAL(clicked()), SLOT(stack_push_all()));

   pb_stack_push_sel = new QPushButton(tr("Psl"), this);
   pb_stack_push_sel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_push_sel->setMinimumHeight(minHeight1);
   pb_stack_push_sel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_stack_push_sel, SIGNAL(clicked()), SLOT(stack_push_sel()));

   lbl_stack = new QLabel( "", this );
   lbl_stack->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_stack->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_stack->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2));

   pb_stack_copy = new QPushButton(tr("Cpy"), this);
   pb_stack_copy->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_copy->setMinimumHeight(minHeight1);
   pb_stack_copy->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_stack_copy, SIGNAL(clicked()), SLOT(stack_copy()));

   pb_stack_pcopy = new QPushButton(tr("Pcp"), this);
   pb_stack_pcopy->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_pcopy->setMinimumHeight(minHeight1);
   pb_stack_pcopy->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_stack_pcopy, SIGNAL(clicked()), SLOT(stack_pcopy()));

   pb_stack_paste = new QPushButton(tr("Pst"), this);
   pb_stack_paste->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_paste->setMinimumHeight(minHeight1);
   pb_stack_paste->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_stack_paste, SIGNAL(clicked()), SLOT(stack_paste()));

   pb_stack_drop = new QPushButton(tr("Drp"), this);
   pb_stack_drop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_drop->setMinimumHeight(minHeight1);
   pb_stack_drop->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_stack_drop, SIGNAL(clicked()), SLOT(stack_drop()));

   pb_stack_join = new QPushButton(tr("Jn"), this);
   pb_stack_join->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_join->setMinimumHeight(minHeight1);
   pb_stack_join->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_stack_join, SIGNAL(clicked()), SLOT(stack_join()));

   pb_stack_rot_up = new QPushButton(tr("Rdn"), this);
   pb_stack_rot_up->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_rot_up->setMinimumHeight(minHeight1);
   pb_stack_rot_up->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_stack_rot_up, SIGNAL(clicked()), SLOT(stack_rot_up()));

   pb_stack_rot_down = new QPushButton(tr("Rup"), this);
   pb_stack_rot_down->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_rot_down->setMinimumHeight(minHeight1);
   pb_stack_rot_down->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_stack_rot_down, SIGNAL(clicked()), SLOT(stack_rot_down()));

   pb_stack_swap = new QPushButton(tr("Swp"), this);
   pb_stack_swap->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_swap->setMinimumHeight(minHeight1);
   pb_stack_swap->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_stack_swap, SIGNAL(clicked()), SLOT(stack_swap()));

   // pb_plot_files = new QPushButton(tr("Plot"), this);
   // pb_plot_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   // pb_plot_files->setMinimumHeight(minHeight1);
   // pb_plot_files->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   // connect(pb_plot_files, SIGNAL(clicked()), SLOT(plot_files()));

   pb_conc_avg = new QPushButton(tr("Concentration normalized average"), this);
   pb_conc_avg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_conc_avg->setMinimumHeight(minHeight1);
   pb_conc_avg->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_conc_avg, SIGNAL(clicked()), SLOT(conc_avg()));

   pb_normalize = new QPushButton(tr("Normalize"), this);
   pb_normalize->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_normalize->setMinimumHeight(minHeight1);
   pb_normalize->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_normalize, SIGNAL(clicked()), SLOT(normalize()));

   pb_add = new QPushButton(tr("Add"), this);
   pb_add->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_add->setMinimumHeight(minHeight1);
   pb_add->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_add, SIGNAL(clicked()), SLOT(add()));

   pb_avg = new QPushButton(tr("Average"), this);
   pb_avg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_avg->setMinimumHeight(minHeight1);
   pb_avg->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_avg, SIGNAL(clicked()), SLOT(avg()));

   pb_smooth = new QPushButton(tr("Smooth"), this);
   pb_smooth->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_smooth->setMinimumHeight(minHeight1);
   pb_smooth->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_smooth, SIGNAL(clicked()), SLOT(smooth()));

   pb_repeak = new QPushButton(tr("Repeak"), this);
   pb_repeak->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_repeak->setMinimumHeight(minHeight1);
   pb_repeak->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_repeak, SIGNAL(clicked()), SLOT(repeak()));

   pb_create_i_of_t = new QPushButton(tr("Make I(t)"), this);
   pb_create_i_of_t->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_create_i_of_t->setMinimumHeight(minHeight1);
   pb_create_i_of_t->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_create_i_of_t, SIGNAL(clicked()), SLOT(create_i_of_t()));

   pb_create_i_of_q = new QPushButton(tr("Make I(q)"), this);
   pb_create_i_of_q->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_create_i_of_q->setMinimumHeight(minHeight1);
   pb_create_i_of_q->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_create_i_of_q, SIGNAL(clicked()), SLOT(create_i_of_q()));

   pb_conc_file = new QPushButton(tr("Set concentration file"), this);
   pb_conc_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_conc_file->setMinimumHeight(minHeight1);
   pb_conc_file->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_conc_file, SIGNAL(clicked()), SLOT(set_conc_file()));

   lbl_conc_file = new QLabel("", this );
   lbl_conc_file->setMinimumHeight(minHeight1);
   lbl_conc_file->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_conc_file->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_conc_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   pb_detector = new QPushButton(tr("Concentration detector"), this);
   pb_detector->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_detector->setMinimumHeight(minHeight1);
   pb_detector->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_detector, SIGNAL(clicked()), SLOT(set_detector()));

   //    pb_set_hplc = new QPushButton(tr("Set buffer"), this);
   //    pb_set_hplc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   //    pb_set_hplc->setMinimumHeight(minHeight1);
   //    pb_set_hplc->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   //    connect(pb_set_hplc, SIGNAL(clicked()), SLOT(set_hplc()));
   
   //    lbl_hplc = new QLabel("", this );
   //    lbl_hplc->setMinimumHeight(minHeight1);
   //    lbl_hplc->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   //    lbl_hplc->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   //    lbl_hplc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   //    pb_set_empty = new QPushButton(tr("Set blank"), this);
   //    pb_set_empty->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   //    pb_set_empty->setMinimumHeight(minHeight1);
   //    pb_set_empty->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   //    connect(pb_set_empty, SIGNAL(clicked()), SLOT(set_empty()));

   //    lbl_empty = new QLabel("", this );
   //    lbl_empty->setMinimumHeight(minHeight1);
   //    lbl_empty->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   //    lbl_empty->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   //    lbl_empty->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   //    pb_set_signal = new QPushButton(tr("Set solution"), this);
   //    pb_set_signal->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   //    pb_set_signal->setMinimumHeight(minHeight1);
   //    pb_set_signal->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   //    connect(pb_set_signal, SIGNAL(clicked()), SLOT(set_signal()));

   //    lbl_signal = new QLabel("", this );
   //    lbl_signal->setMinimumHeight(minHeight1);
   //    lbl_signal->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   //    lbl_signal->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   //    lbl_signal->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   lbl_created_files = new QLabel("Produced Data", this);
   lbl_created_files->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_created_files->setMinimumHeight(minHeight1);
   lbl_created_files->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_created_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_created_dir = new mQLabel( QDir::currentDirPath(), this );
   lbl_created_dir->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_created_dir->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_created_dir->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2));
   connect( lbl_created_dir, SIGNAL(pressed()), SLOT( created_dir_pressed() ));

   lb_created_files = new QListBox(this, "created_files created_files listbox" );
   lb_created_files->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lb_created_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lb_created_files->setEnabled(true);
   lb_created_files->setSelectionMode( QListBox::Extended );
   lb_created_files->setMinimumHeight( minHeight1 * 3 );
   connect( lb_created_files, SIGNAL( selectionChanged() ), SLOT( update_created_files() ) );
   connect( lb_created_files, 
            SIGNAL( rightButtonClicked( QListBoxItem *, const QPoint & ) ),
            SLOT  ( rename_created    ( QListBoxItem *, const QPoint & ) ) );

   lbl_selected_created = new QLabel("0 files selected", this );
   lbl_selected_created->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_selected_created->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_selected_created->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2));

   pb_select_all_created = new QPushButton(tr("Select all"), this);
   pb_select_all_created->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize  - 1));
   pb_select_all_created->setMinimumHeight(minHeight1);
   pb_select_all_created->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_select_all_created, SIGNAL(clicked()), SLOT(select_all_created()));

   pb_invert_all_created = new QPushButton(tr("Invert"), this);
   pb_invert_all_created->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize  - 1));
   pb_invert_all_created->setMinimumHeight(minHeight1);
   pb_invert_all_created->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_invert_all_created, SIGNAL(clicked()), SLOT(invert_all_created()));

   pb_adjacent_created = new QPushButton(tr("Similar"), this);
   pb_adjacent_created->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_adjacent_created->setMinimumHeight(minHeight1);
   pb_adjacent_created->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_adjacent_created, SIGNAL(clicked()), SLOT(adjacent_created()));

   pb_remove_created = new QPushButton(tr("Remove"), this);
   pb_remove_created->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_remove_created->setMinimumHeight(minHeight1);
   pb_remove_created->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_remove_created, SIGNAL(clicked()), SLOT(remove_created()));

   pb_save_created_csv = new QPushButton(tr("Save CSV"), this);
   pb_save_created_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_save_created_csv->setMinimumHeight(minHeight1);
   pb_save_created_csv->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_save_created_csv, SIGNAL(clicked()), SLOT(save_created_csv()));

   pb_save_created = new QPushButton(tr("Save"), this);
   pb_save_created->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_save_created->setMinimumHeight(minHeight1);
   pb_save_created->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_save_created, SIGNAL(clicked()), SLOT(save_created()));

   pb_show_created = new QPushButton(tr("Show"), this);
   pb_show_created->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_show_created->setMinimumHeight(minHeight1);
   pb_show_created->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_show_created, SIGNAL(clicked()), SLOT(show_created()));

   pb_show_only_created = new QPushButton(tr("Show only"), this);
   pb_show_only_created->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_show_only_created->setMinimumHeight(minHeight1);
   pb_show_only_created->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_show_only_created, SIGNAL(clicked()), SLOT(show_only_created()));

   progress = new QProgressBar(this, "Progress");
   // progress->setMinimumHeight(minHeight1);
   progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   progress->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   progress->reset();

   editor = new QTextEdit(this);
   editor->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   editor->setReadOnly(true);
   editor->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ));

   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight3);

   m = new QMenuBar(frame, "menu" );
   m->setMinimumHeight(minHeight1 - 5);
   m->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   QPopupMenu * file = new QPopupMenu(editor);
   m->insertItem( tr("&File"), file );
   file->insertItem( tr("Font"),  this, SLOT(update_font()),    ALT+Key_F );
   file->insertItem( tr("Save"),  this, SLOT(save()),    ALT+Key_S );
   file->insertItem( tr("Clear Display"), this, SLOT(clear_display()),   ALT+Key_X );
   editor->setWordWrap (QTextEdit::WidgetWidth);
   editor->setMinimumHeight( minHeight1 * 3 );

   plot_dist = new QwtPlot(this);
#ifndef QT4
   // plot_dist->enableOutline(true);
   // plot_dist->setOutlinePen(Qt::white);
   // plot_dist->setOutlineStyle(Qwt::VLine);
   plot_dist->enableGridXMin();
   plot_dist->enableGridYMin();
#else
   grid_saxs = new QwtPlotGrid;
   grid_saxs->enableXMin( true );
   grid_saxs->enableYMin( true );
#endif
   plot_dist->setPalette( QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
#ifndef QT4
   plot_dist->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot_dist->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   grid_saxs->setMajPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid_saxs->setMinPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid_saxs->attach( plot_dist );
#endif
   plot_dist->setAxisTitle(QwtPlot::xBottom, /* cb_guinier->isChecked() ? tr("q^2 (1/Angstrom^2)") : */  tr("q (1/Angstrom) or Time or Frame"));
   plot_dist->setAxisTitle(QwtPlot::yLeft, tr("Log10 I(q) or I(t) or OD or RI"));
#ifndef QT4
   plot_dist->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   plot_dist->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_dist->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#ifndef QT4
   plot_dist->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_dist->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#ifndef QT4
   plot_dist->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_dist->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_dist->setMargin(USglobal->config_list.margin);
   plot_dist->setTitle("");
#ifndef QT4
   plot_dist->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::Logarithmic);
#else
   plot_dist->setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);
#endif
   plot_dist->setCanvasBackground(USglobal->global_colors.plot);

#ifndef QT4
   plot_dist->setAutoLegend( false );
   plot_dist->setLegendFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ) );
#else
   QwtLegend* legend_pd = new QwtLegend;
   legend_pd->setFrameStyle( QFrame::Box | QFrame::Sunken );
   plot_dist->insertLegend( legend_pd, QwtPlot::BottomLegend );
#endif
   connect( plot_dist->canvas(), SIGNAL( mouseReleased( const QMouseEvent & ) ), SLOT( plot_mouse(  const QMouseEvent & ) ) );

   plot_errors = new QwtPlot(this);
#ifndef QT4
   // plot_errors->enableOutline(true);
   // plot_errors->setOutlinePen(Qt::white);
   // plot_errors->setOutlineStyle(Qwt::VLine);
   plot_errors->enableGridXMin();
   plot_errors->enableGridYMin();
#else
   grid_errors = new QwtPlotGrid;
   grid_errors->enableXMin( true );
   grid_errors->enableYMin( true );
#endif
   plot_errors->setPalette( QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
#ifndef QT4
   plot_errors->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot_errors->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   grid_errors->setMajPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid_errors->setMinPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid_errors->attach( plot_errors );
#endif
   // plot_errors->setAxisTitle(QwtPlot::xBottom, /* cb_guinier->isChecked() ? tr("q^2 (1/Angstrom^2)") : */  tr("q (1/Angstrom) or Frame"));
   // plot_errors->setAxisTitle(QwtPlot::yLeft, tr("Log10 I(q)"));
#ifndef QT4
   // plot_errors->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   // plot_errors->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_errors->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#ifndef QT4
   // plot_errors->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_errors->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#ifndef QT4
   // plot_errors->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   // plot_errors->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_errors->setMargin(USglobal->config_list.margin);
   plot_errors->setTitle("");
   plot_errors->setCanvasBackground(USglobal->global_colors.plot);

   plot_errors_zoomer = new ScrollZoomer(plot_errors->canvas());
   plot_errors_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
#ifndef QT4
   plot_errors_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
   connect( plot_errors_zoomer, SIGNAL( zoomed( const QwtDoubleRect & ) ), SLOT( plot_errors_zoomed( const QwtDoubleRect & ) ) );

   cb_plot_errors_rev = new QCheckBox(this);
   cb_plot_errors_rev->setText(tr("Reverse"));
   cb_plot_errors_rev->setEnabled( true );
   cb_plot_errors_rev->setChecked( false );
   cb_plot_errors_rev->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_plot_errors_rev->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect( cb_plot_errors_rev, SIGNAL( clicked() ), SLOT( set_plot_errors_rev() ) );

   cb_plot_errors_sd = new QCheckBox(this);
   cb_plot_errors_sd->setText(tr("Use standard deviations  "));
   cb_plot_errors_sd->setEnabled( true );
   cb_plot_errors_sd->setChecked( false );
   cb_plot_errors_sd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_plot_errors_sd->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect( cb_plot_errors_sd, SIGNAL( clicked() ), SLOT( set_plot_errors_sd() ) );

   cb_plot_errors_pct = new QCheckBox(this);
   cb_plot_errors_pct->setText(tr("By percent "));
   cb_plot_errors_pct->setEnabled( true );
   cb_plot_errors_pct->setChecked( false );
   cb_plot_errors_pct->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_plot_errors_pct->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect( cb_plot_errors_pct, SIGNAL( clicked() ), SLOT( set_plot_errors_pct() ) );

   cb_plot_errors_group = new QCheckBox(this);
   cb_plot_errors_group->setText(tr("Group"));
   cb_plot_errors_group->setEnabled( true );
   cb_plot_errors_group->setChecked( false );
   cb_plot_errors_group->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_plot_errors_group->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect( cb_plot_errors_group, SIGNAL( clicked() ), SLOT( set_plot_errors_group() ) );

   plot_errors->setAxisTitle(QwtPlot::yLeft, tr( cb_plot_errors_pct->isChecked() ?
                                                 "%" : "delta I(q)" ) );

   QHBoxLayout *hbl_plot_errors_buttons = new QHBoxLayout( 0 );
   hbl_plot_errors_buttons->addWidget( cb_plot_errors_rev );
   hbl_plot_errors_buttons->addWidget( cb_plot_errors_sd );
   hbl_plot_errors_buttons->addWidget( cb_plot_errors_pct );
   hbl_plot_errors_buttons->addWidget( cb_plot_errors_group );

   l_plot_errors = new QVBoxLayout( 0 );
   l_plot_errors->addWidget( plot_errors );
   l_plot_errors->addLayout( hbl_plot_errors_buttons );

   plot_errors_widgets.push_back( plot_errors );
   plot_errors_widgets.push_back( cb_plot_errors_rev );
   plot_errors_widgets.push_back( cb_plot_errors_sd );
   plot_errors_widgets.push_back( cb_plot_errors_pct );
   plot_errors_widgets.push_back( cb_plot_errors_group );

   hide_widgets( plot_errors_widgets, true );

   pb_wheel_start = new QPushButton(tr("Timeshift on"), this);
   pb_wheel_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_wheel_start->setMinimumHeight(minHeight1);
   pb_wheel_start->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_wheel_start, SIGNAL(clicked()), SLOT(wheel_start()));

   pb_p3d = new QPushButton(tr("3D"), this);
   pb_p3d->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_p3d->setMinimumHeight(minHeight1);
   pb_p3d->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_p3d, SIGNAL(clicked()), SLOT(p3d()));

   lbl_wheel_pos = new QLabel( "0", this );
   lbl_wheel_pos->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_wheel_pos->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_wheel_pos->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   qwtw_wheel = new QwtWheel( this );
   qwtw_wheel->setMass         ( 0.5 );
   // qwtw_wheel->setRange        ( -1000, 1000, 1 );
   qwtw_wheel->setMinimumHeight( minHeight1 );
   // qwtw_wheel->setTotalAngle( 3600.0 );
   qwtw_wheel->setEnabled      ( false );
   connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );

   pb_errors = new QPushButton(tr("Residuals"), this);
   pb_errors->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_errors->setMinimumHeight(minHeight1);
   pb_errors->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_errors->setEnabled(false);
   connect(pb_errors, SIGNAL(clicked()), SLOT(errors()));

   pb_wheel_cancel = new QPushButton(tr("Cancel"), this);
   pb_wheel_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_wheel_cancel->setMinimumHeight(minHeight1);
   pb_wheel_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_wheel_cancel->setEnabled(false);
   connect(pb_wheel_cancel, SIGNAL(clicked()), SLOT(wheel_cancel()));

   pb_wheel_save = new QPushButton(tr("Keep"), this);
   pb_wheel_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_wheel_save->setMinimumHeight(minHeight1);
   pb_wheel_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_wheel_save->setEnabled(false);
   connect(pb_wheel_save, SIGNAL(clicked()), SLOT(wheel_save()));

   pb_gauss_start = new QPushButton(tr("Gaussians"), this);
   pb_gauss_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_gauss_start->setMinimumHeight(minHeight1);
   pb_gauss_start->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_gauss_start, SIGNAL(clicked()), SLOT(gauss_start()));

   pb_gauss_clear = new QPushButton(tr("Clear"), this);
   pb_gauss_clear->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_gauss_clear->setMinimumHeight(minHeight1);
   pb_gauss_clear->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_gauss_clear->setEnabled( false );
   connect(pb_gauss_clear, SIGNAL(clicked()), SLOT(gauss_clear()));

   pb_gauss_new = new QPushButton(tr("New"), this);
   pb_gauss_new->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_gauss_new->setMinimumHeight(minHeight1);
   pb_gauss_new->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_gauss_new->setEnabled( false );
   connect(pb_gauss_new, SIGNAL(clicked()), SLOT(gauss_new()));

   pb_gauss_delete = new QPushButton(tr("Del"), this);
   pb_gauss_delete->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_gauss_delete->setMinimumHeight(minHeight1);
   pb_gauss_delete->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_gauss_delete->setEnabled( false );
   connect(pb_gauss_delete, SIGNAL(clicked()), SLOT(gauss_delete()));

   pb_gauss_prev = new QPushButton( "<", this);
   pb_gauss_prev->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_gauss_prev->setMinimumHeight(minHeight1);
   pb_gauss_prev->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_gauss_prev->setEnabled( false );
   connect(pb_gauss_prev, SIGNAL(clicked()), SLOT(gauss_prev()));

   lbl_gauss_pos = new QLabel( " 0 of 0 ", this );
   lbl_gauss_pos->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_gauss_pos->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_gauss_pos->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   pb_gauss_next = new QPushButton( ">", this);
   pb_gauss_next->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_gauss_next->setMinimumHeight(minHeight1);
   pb_gauss_next->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_gauss_next->setEnabled( false );
   connect(pb_gauss_next, SIGNAL(clicked()), SLOT(gauss_next()));


   le_gauss_pos = new mQLineEdit(this, "le_gauss_pos Line Edit");
   le_gauss_pos->setText( "" );
   le_gauss_pos->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_gauss_pos->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_gauss_pos->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_gauss_pos->setEnabled( false );
   {
      QDoubleValidator *qdv = new QDoubleValidator( le_gauss_pos );
      qdv->setDecimals( UHSH_VAL_DEC );
      // le_gauss_pos->setValidator( qdv );
   }
   // le_gauss_pos->validator()->setDecimals( UHSH_VAL_DEC );
   connect( le_gauss_pos, SIGNAL( textChanged( const QString & ) ), SLOT( gauss_pos_text( const QString & ) ) );
   connect( le_gauss_pos, SIGNAL( focussed ( bool ) )             , SLOT( gauss_pos_focus( bool ) ) );

   le_gauss_pos_width = new mQLineEdit(this, "le_gauss_pos_width Line Edit");
   le_gauss_pos_width->setText( "" );
   le_gauss_pos_width->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_gauss_pos_width->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_gauss_pos_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_gauss_pos_width->setEnabled( false );
   {
      QDoubleValidator *qdv = new QDoubleValidator( le_gauss_pos_width );
      qdv->setDecimals( UHSH_VAL_DEC );
      // le_gauss_pos_width->setValidator( qdv );
   }
   // le_gauss_pos_width->setValidator( new QDoubleValidator( le_gauss_pos_width ) );
   // le_gauss_pos_width->validator()->setDecimals( UHSH_VAL_DEC );
   connect( le_gauss_pos_width, SIGNAL( textChanged( const QString & ) ), SLOT( gauss_pos_width_text( const QString & ) ) );
   connect( le_gauss_pos_width, SIGNAL( focussed ( bool ) )             , SLOT( gauss_pos_width_focus( bool ) ) );

   le_gauss_pos_height = new mQLineEdit(this, "le_gauss_pos_height Line Edit");
   le_gauss_pos_height->setText( "" );
   le_gauss_pos_height->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_gauss_pos_height->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_gauss_pos_height->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_gauss_pos_height->setEnabled( false );
   {
      QDoubleValidator *qdv = new QDoubleValidator( le_gauss_pos_height );
      qdv->setDecimals( UHSH_VAL_DEC );
      // le_gauss_pos_height->setValidator( qdv );
   }
   // le_gauss_pos_height->setValidator( new QDoubleValidator( le_gauss_pos_height ) );
   // le_gauss_pos_height->validator()->setDecimals( UHSH_VAL_DEC );
   connect( le_gauss_pos_height, SIGNAL( textChanged( const QString & ) ), SLOT( gauss_pos_height_text( const QString & ) ) );
   connect( le_gauss_pos_height, SIGNAL( focussed ( bool ) )             , SLOT( gauss_pos_height_focus( bool ) ) );

   cb_sd_weight = new QCheckBox(this);
   cb_sd_weight->setText(tr("SD  "));
   cb_sd_weight->setEnabled( true );
   cb_sd_weight->setChecked( false );
   cb_sd_weight->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_sd_weight->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect( cb_sd_weight, SIGNAL( clicked() ), SLOT( set_sd_weight() ) );

   cb_fix_width = new QCheckBox(this);
   cb_fix_width->setText(tr("Eq width  "));
   cb_fix_width->setEnabled( true );
   cb_fix_width->setChecked( false );
   cb_fix_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_fix_width->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect( cb_fix_width, SIGNAL( clicked() ), SLOT( set_fix_width() ) );

   pb_gauss_fit = new QPushButton(tr("Fit"), this);
   pb_gauss_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_gauss_fit->setMinimumHeight(minHeight1);
   pb_gauss_fit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_gauss_fit->setEnabled( false );
   connect(pb_gauss_fit, SIGNAL(clicked()), SLOT(gauss_fit()));

   lbl_gauss_fit = new QLabel( "", this );
   lbl_gauss_fit->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_gauss_fit->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_gauss_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_gauss_fit_start = new mQLineEdit(this, "le_gauss_fit_start Line Edit");
   le_gauss_fit_start->setText( "" );
   le_gauss_fit_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_gauss_fit_start->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_gauss_fit_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_gauss_fit_start->setEnabled( false );
   le_gauss_fit_start->setValidator( new QDoubleValidator( le_gauss_fit_start ) );
   connect( le_gauss_fit_start, SIGNAL( textChanged( const QString & ) ), SLOT( gauss_fit_start_text( const QString & ) ) );
   connect( le_gauss_fit_start, SIGNAL( focussed ( bool ) )             , SLOT( gauss_fit_start_focus( bool ) ) );

   le_gauss_fit_end = new mQLineEdit(this, "le_gauss_fit_end Line Edit");
   le_gauss_fit_end->setText( "" );
   le_gauss_fit_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_gauss_fit_end->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_gauss_fit_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_gauss_fit_end->setEnabled( false );
   le_gauss_fit_end->setValidator( new QDoubleValidator( le_gauss_fit_end ) );
   connect( le_gauss_fit_end, SIGNAL( textChanged( const QString & ) ), SLOT( gauss_fit_end_text( const QString & ) ) );
   connect( le_gauss_fit_end, SIGNAL( focussed ( bool ) )             , SLOT( gauss_fit_end_focus( bool ) ) );

   pb_gauss_save = new QPushButton(tr("Save"), this);
   pb_gauss_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_gauss_save->setMinimumHeight(minHeight1);
   pb_gauss_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_gauss_save->setEnabled( false );
   connect(pb_gauss_save, SIGNAL(clicked()), SLOT(gauss_save()));

   pb_ggauss_start = new QPushButton(tr("Global gaussians"), this);
   pb_ggauss_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_ggauss_start->setMinimumHeight(minHeight1);
   pb_ggauss_start->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_ggauss_start, SIGNAL(clicked()), SLOT(ggauss_start()));

   pb_ggauss_rmsd = new QPushButton(tr("RMSD"), this);
   pb_ggauss_rmsd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_ggauss_rmsd->setMinimumHeight(minHeight1);
   pb_ggauss_rmsd->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_ggauss_rmsd->setEnabled( false );
   connect(pb_ggauss_rmsd, SIGNAL(clicked()), SLOT(ggauss_rmsd()));

   pb_ggauss_results = new QPushButton(tr("Make result curves"), this);
   pb_ggauss_results->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_ggauss_results->setMinimumHeight(minHeight1);
   pb_ggauss_results->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_ggauss_results->setEnabled( false );
   connect(pb_ggauss_results, SIGNAL(clicked()), SLOT(ggauss_results()));
   // pb_ggauss_results->hide();

   pb_gauss_as_curves = new QPushButton(tr("Produce Gaussians as curves"), this);
   pb_gauss_as_curves->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_gauss_as_curves->setMinimumHeight(minHeight1);
   pb_gauss_as_curves->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_gauss_as_curves->setEnabled( false );
   connect(pb_gauss_as_curves, SIGNAL(clicked()), SLOT(gauss_as_curves()));

   le_baseline_start_s = new mQLineEdit(this, "le_baseline_start_s Line Edit");
   le_baseline_start_s->setText( "" );
   le_baseline_start_s->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_baseline_start_s->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_baseline_start_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_baseline_start_s->setEnabled( false );
   le_baseline_start_s->setValidator( new QDoubleValidator( le_baseline_start_s ) );
   connect( le_baseline_start_s, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_start_s_text( const QString & ) ) );
   connect( le_baseline_start_s, SIGNAL( focussed ( bool ) )             , SLOT( baseline_start_s_focus( bool ) ) );

   le_baseline_start = new mQLineEdit(this, "le_baseline_start Line Edit");
   le_baseline_start->setText( "" );
   le_baseline_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_baseline_start->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_baseline_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_baseline_start->setEnabled( false );
   le_baseline_start->setValidator( new QDoubleValidator( le_baseline_start ) );
   connect( le_baseline_start, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_start_text( const QString & ) ) );
   connect( le_baseline_start, SIGNAL( focussed ( bool ) )             , SLOT( baseline_start_focus( bool ) ) );

   le_baseline_start_e = new mQLineEdit(this, "le_baseline_start_e Line Edit");
   le_baseline_start_e->setText( "" );
   le_baseline_start_e->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_baseline_start_e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_baseline_start_e->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_baseline_start_e->setEnabled( false );
   le_baseline_start_e->setValidator( new QDoubleValidator( le_baseline_start_e ) );
   connect( le_baseline_start_e, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_start_e_text( const QString & ) ) );
   connect( le_baseline_start_e, SIGNAL( focussed ( bool ) )             , SLOT( baseline_start_e_focus( bool ) ) );

   le_baseline_end_s = new mQLineEdit(this, "le_baseline_end_s Line Edit");
   le_baseline_end_s->setText( "" );
   le_baseline_end_s->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_baseline_end_s->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_baseline_end_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_baseline_end_s->setEnabled( false );
   le_baseline_end_s->setValidator( new QDoubleValidator( le_baseline_end_s ) );
   connect( le_baseline_end_s, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_s_text( const QString & ) ) );
   connect( le_baseline_end_s, SIGNAL( focussed ( bool ) )             , SLOT( baseline_end_s_focus( bool ) ) );

   le_baseline_end = new mQLineEdit(this, "le_baseline_end Line Edit");
   le_baseline_end->setText( "" );
   le_baseline_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_baseline_end->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_baseline_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_baseline_end->setEnabled( false );
   le_baseline_end->setValidator( new QDoubleValidator( le_baseline_end ) );
   connect( le_baseline_end, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_text( const QString & ) ) );
   connect( le_baseline_end, SIGNAL( focussed ( bool ) )             , SLOT( baseline_end_focus( bool ) ) );

   le_baseline_end_e = new mQLineEdit(this, "le_baseline_end_e Line Edit");
   le_baseline_end_e->setText( "" );
   le_baseline_end_e->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_baseline_end_e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_baseline_end_e->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_baseline_end_e->setEnabled( false );
   le_baseline_end_e->setValidator( new QDoubleValidator( le_baseline_end_e ) );
   connect( le_baseline_end_e, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_e_text( const QString & ) ) );
   connect( le_baseline_end_e, SIGNAL( focussed ( bool ) )             , SLOT( baseline_end_e_focus( bool ) ) );

   pb_baseline_start = new QPushButton(tr("Baseline"), this);
   pb_baseline_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_baseline_start->setMinimumHeight(minHeight1);
   pb_baseline_start->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_baseline_start, SIGNAL(clicked()), SLOT(baseline_start()));

   pb_baseline_apply = new QPushButton(tr("Compute and subtract baseline"), this);
   pb_baseline_apply->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_baseline_apply->setMinimumHeight(minHeight1);
   pb_baseline_apply->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_baseline_apply, SIGNAL(clicked()), SLOT(baseline_apply()));

   pb_select_vis = new QPushButton(tr("Select Visible"), this);
   pb_select_vis->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_select_vis->setMinimumHeight(minHeight1);
   pb_select_vis->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_select_vis, SIGNAL(clicked()), SLOT(select_vis()));

   pb_remove_vis = new QPushButton(tr("Remove Vis"), this);
   pb_remove_vis->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_remove_vis->setMinimumHeight(minHeight1);
   pb_remove_vis->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_remove_vis, SIGNAL(clicked()), SLOT(remove_vis()));

   pb_crop_common = new QPushButton(tr("Crop Common"), this);
   pb_crop_common->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_crop_common->setMinimumHeight(minHeight1);
   pb_crop_common->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_crop_common, SIGNAL(clicked()), SLOT(crop_common()));

   pb_crop_vis = new QPushButton(tr("Crop Vis"), this);
   pb_crop_vis->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_crop_vis->setMinimumHeight(minHeight1);
   pb_crop_vis->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_crop_vis, SIGNAL(clicked()), SLOT(crop_vis()));

   pb_crop_zero = new QPushButton(tr("Crop Zeros"), this);
   pb_crop_zero->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_crop_zero->setMinimumHeight(minHeight1);
   pb_crop_zero->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_crop_zero, SIGNAL(clicked()), SLOT(crop_zero()));

   pb_crop_left = new QPushButton(tr("Crop Left"), this);
   pb_crop_left->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_crop_left->setMinimumHeight(minHeight1);
   pb_crop_left->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_crop_left, SIGNAL(clicked()), SLOT(crop_left()));

   pb_crop_undo = new QPushButton(tr("Undo"), this);
   pb_crop_undo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_crop_undo->setMinimumHeight(minHeight1);
   pb_crop_undo->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_crop_undo, SIGNAL(clicked()), SLOT(crop_undo()));

   pb_crop_right = new QPushButton(tr("Crop Right"), this);
   pb_crop_right->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_crop_right->setMinimumHeight(minHeight1);
   pb_crop_right->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_crop_right, SIGNAL(clicked()), SLOT(crop_right()));

   pb_legend = new QPushButton(tr("Legend"), this);
   pb_legend->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_legend->setMinimumHeight(minHeight1);
   pb_legend->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_legend, SIGNAL(clicked()), SLOT(legend()));

   pb_axis_x = new QPushButton(tr("X"), this);
   pb_axis_x->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_axis_x->setMinimumHeight(minHeight1);
   pb_axis_x->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_axis_x, SIGNAL(clicked()), SLOT(axis_x()));

   pb_axis_y = new QPushButton(tr("Y"), this);
   pb_axis_y->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_axis_y->setMinimumHeight(minHeight1);
   pb_axis_y->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_axis_y, SIGNAL(clicked()), SLOT(axis_y()));

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // build layout
   QBoxLayout *hbl_file_buttons = new QHBoxLayout( 0 );
   hbl_file_buttons->addWidget ( pb_add_files );
   hbl_file_buttons->addWidget ( pb_similar_files );
   hbl_file_buttons->addWidget ( pb_conc);
   hbl_file_buttons->addWidget ( pb_clear_files );

   QBoxLayout *hbl_file_buttons_1 = new QHBoxLayout( 0 );
   hbl_file_buttons_1->addWidget ( pb_regex_load );
   hbl_file_buttons_1->addWidget ( le_regex );
   hbl_file_buttons_1->addWidget ( le_regex_args );
   hbl_file_buttons_1->addWidget ( pb_save_state );

   QBoxLayout *hbl_file_buttons_2 = new QHBoxLayout( 0 );
   hbl_file_buttons_2->addWidget ( pb_select_all );
   hbl_file_buttons_2->addWidget ( pb_invert );
   hbl_file_buttons_2->addWidget ( pb_adjacent );
   hbl_file_buttons_2->addWidget ( pb_select_nth );
   // hbl_file_buttons_2->addWidget ( pb_color_rotate );
   // hbl_file_buttons_2->addWidget ( pb_to_saxs );
   hbl_file_buttons_2->addWidget ( pb_view );
   hbl_file_buttons_2->addWidget ( pb_rescale );

   QBoxLayout *hbl_file_buttons_2b = new QHBoxLayout( 0 );
   hbl_file_buttons_2b->addWidget( pb_stack_push_all );
   hbl_file_buttons_2b->addWidget( pb_stack_push_sel );
   hbl_file_buttons_2b->addWidget( lbl_stack );
   hbl_file_buttons_2b->addWidget( pb_stack_copy );
   hbl_file_buttons_2b->addWidget( pb_stack_pcopy );
   hbl_file_buttons_2b->addWidget( pb_stack_paste );
   hbl_file_buttons_2b->addWidget( pb_stack_drop );
   hbl_file_buttons_2b->addWidget( pb_stack_join );
   hbl_file_buttons_2b->addWidget( pb_stack_rot_up );
   hbl_file_buttons_2b->addWidget( pb_stack_rot_down );
   hbl_file_buttons_2b->addWidget( pb_stack_swap );

   if ( !((US_Hydrodyn *)us_hydrodyn)->advanced_config.expert_mode )
   {
      pb_regex_load->hide();
      le_regex->hide();
      le_regex_args->hide();
      pb_save_state->hide();

      pb_stack_push_all->hide();
      pb_stack_push_sel->hide();
      lbl_stack->hide();
      pb_stack_copy->hide();
      pb_stack_pcopy->hide();
      pb_stack_paste->hide();
      pb_stack_drop->hide();
      pb_stack_join->hide();
      pb_stack_rot_up->hide();
      pb_stack_rot_down->hide();
      pb_stack_swap->hide();

      pb_add->hide();

      // pb_conc->hide();
      // pb_normalize->hide();
   }
   pb_conc_avg->hide();

   QBoxLayout *hbl_file_buttons_3 = new QHBoxLayout( 0 );
   hbl_file_buttons_3->addWidget ( pb_conc_avg );
   hbl_file_buttons_3->addWidget ( pb_normalize );
   hbl_file_buttons_3->addWidget ( pb_avg );
   hbl_file_buttons_3->addWidget ( pb_add );
   hbl_file_buttons_3->addWidget ( pb_to_saxs );
   hbl_file_buttons_3->addWidget ( pb_color_rotate );

   QBoxLayout *hbl_file_buttons_4 = new QHBoxLayout( 0 );
   hbl_file_buttons_4->addWidget ( pb_smooth );
   hbl_file_buttons_4->addWidget ( pb_repeak );
   hbl_file_buttons_4->addWidget ( pb_create_i_of_t );
   hbl_file_buttons_4->addWidget ( pb_create_i_of_q );

   QBoxLayout *hbl_conc_file = new QHBoxLayout( 0 );
   hbl_conc_file->addWidget ( pb_conc_file );
   hbl_conc_file->addWidget ( pb_detector );

   //    QBoxLayout *hbl_hplc = new QHBoxLayout( 0 );
   //    hbl_hplc->addWidget ( pb_set_hplc );
   //    hbl_hplc->addWidget ( lbl_hplc );

   //    QBoxLayout *hbl_empty = new QHBoxLayout( 0 );
   //    hbl_empty->addWidget ( pb_set_empty );
   //    hbl_empty->addWidget ( lbl_empty );

   //    QBoxLayout *hbl_signal = new QHBoxLayout( 0 );
   //    hbl_signal->addWidget ( pb_set_signal );
   //    hbl_signal->addWidget ( lbl_signal );

   QBoxLayout *hbl_created = new QHBoxLayout( 0 );
   hbl_created->addWidget ( pb_select_all_created );
   hbl_created->addWidget ( pb_invert_all_created );
   hbl_created->addWidget ( pb_adjacent_created );
   hbl_created->addWidget ( pb_remove_created );
   hbl_created->addWidget ( pb_save_created_csv );
   hbl_created->addWidget ( pb_save_created );

   QBoxLayout *hbl_created_2 = new QHBoxLayout( 0 );
   hbl_created_2->addWidget ( pb_show_created );
   hbl_created_2->addWidget ( pb_show_only_created );

   QBoxLayout *vbl_editor_group = new QVBoxLayout(0);
   vbl_editor_group->addWidget (frame);
   vbl_editor_group->addWidget (editor);

   QHBoxLayout *hbl_dir = new QHBoxLayout( 0 );
   hbl_dir->addWidget( cb_lock_dir );
   hbl_dir->addWidget( lbl_dir );

   QGridLayout *gl_files = new QGridLayout( 0 );
   {
      unsigned int j = 0;
      gl_files->addWidget( lbl_files, j, 0 ); j++;
      gl_files->addLayout( hbl_dir, j, 0 ); j++;
      gl_files->addLayout( hbl_file_buttons, j, 0 ); j++;
      gl_files->addLayout( hbl_file_buttons_1 , j, 0 ); j++;
      gl_files->addWidget( lb_files , j, 0 ); j++;
      gl_files->addWidget( lbl_selected, j, 0 ); j++;
      gl_files->addLayout( hbl_file_buttons_2 , j, 0 ); j++;
      gl_files->addLayout( hbl_file_buttons_2b , j, 0 ); j++;
      gl_files->addLayout( hbl_file_buttons_3 , j, 0 ); j++;
      gl_files->addLayout( hbl_file_buttons_4 , j, 0 ); j++;
      gl_files->addLayout( hbl_conc_file, j, 0 ); j++;
      gl_files->addWidget( lbl_conc_file, j, 0 ); j++;
      //       gl_files->addLayout( hbl_hplc, j, 0 ); j++;
      //       gl_files->addLayout( hbl_empty, j, 0 ); j++;
      //       gl_files->addLayout( hbl_signal, j, 0 ); j++;
      gl_files->addWidget( lbl_created_files , j, 0 ); j++;
      gl_files->addWidget( lbl_created_dir , j, 0 ); j++;
      gl_files->addWidget( lb_created_files, j, 0 ); j++;
      gl_files->addWidget( lbl_selected_created, j, 0 ); j++;
      gl_files->addLayout( hbl_created , j, 0 ); j++;
      gl_files->addLayout( hbl_created_2, j, 0 ); j++;
      gl_files->addLayout( vbl_editor_group , j, 0 ); j++;
   }

   /* old way
      QBoxLayout *vbl_files = new QVBoxLayout( 0 );
      vbl_files->addWidget( lbl_files );
      vbl_files->addLayout( hbl_dir );
      vbl_files->addLayout( hbl_file_buttons );
      vbl_files->addLayout( hbl_file_buttons_1 );
      //   vbl_files->addLayout( hbl_file_buttons_1b );
      vbl_files->addWidget( lb_files );
      vbl_files->addWidget( lbl_selected );
      vbl_files->addLayout( hbl_file_buttons_2 );
      vbl_files->addLayout( hbl_file_buttons_3 );
      vbl_files->addLayout( hbl_file_buttons_4 );
      vbl_files->addLayout( hbl_hplc );
      vbl_files->addLayout( hbl_empty );
      vbl_files->addLayout( hbl_signal );
      vbl_files->addWidget( lbl_created_files );
      vbl_files->addWidget( lbl_created_dir );
      vbl_files->addWidget( lb_created_files );
      vbl_files->addWidget( lbl_selected_created );
      vbl_files->addLayout( hbl_created );
      vbl_files->addLayout( hbl_created_2 );
      vbl_files->addLayout( vbl_editor_group );
   */

   QBoxLayout *hbl_plot_buttons = new QHBoxLayout(0);
   hbl_plot_buttons->addWidget( pb_select_vis );
   hbl_plot_buttons->addWidget( pb_remove_vis );
   hbl_plot_buttons->addWidget( pb_crop_common );
   hbl_plot_buttons->addWidget( pb_crop_vis );
   hbl_plot_buttons->addWidget( pb_crop_zero );
   hbl_plot_buttons->addWidget( pb_crop_left );
   hbl_plot_buttons->addWidget( pb_crop_undo );
   hbl_plot_buttons->addWidget( pb_crop_right );
   hbl_plot_buttons->addWidget( pb_legend );
   hbl_plot_buttons->addWidget( pb_axis_x );
   hbl_plot_buttons->addWidget( pb_axis_y );

   QGridLayout *gl_wheel = new QGridLayout(0);
   gl_wheel->addMultiCellWidget( pb_wheel_start , 0, 0, 0, 1 );
   gl_wheel->addWidget         ( lbl_wheel_pos  , 0, 2 );
   gl_wheel->addMultiCellWidget( qwtw_wheel     , 0, 0, 3, 7 );
   gl_wheel->addWidget         ( pb_errors      , 0, 8 );
   gl_wheel->addWidget         ( pb_wheel_cancel, 0, 9 );
   gl_wheel->addWidget         ( pb_wheel_save  , 0, 10 );

   QGridLayout *gl_gauss = new QGridLayout(0);
   { 
      int ofs = 1;
      gl_gauss->addMultiCellWidget( pb_gauss_start      , 0, 0, 0, ofs++ );
      gl_gauss->addWidget         ( pb_p3d              , 0, ofs++ );
      gl_gauss->addWidget         ( pb_gauss_clear      , 0, ofs++ );
      gl_gauss->addWidget         ( pb_gauss_new        , 0, ofs++ );
      gl_gauss->addWidget         ( pb_gauss_delete     , 0, ofs++ );
      gl_gauss->addWidget         ( pb_gauss_prev       , 0, ofs++ );
      gl_gauss->addWidget         ( lbl_gauss_pos       , 0, ofs++ );
      gl_gauss->addWidget         ( pb_gauss_next       , 0, ofs++ );
      gl_gauss->addWidget         ( le_gauss_pos        , 0, ofs++ );
      gl_gauss->addWidget         ( le_gauss_pos_width  , 0, ofs++ );
      gl_gauss->addWidget         ( le_gauss_pos_height , 0, ofs++ );
      //       gl_gauss->addWidget         ( pb_gauss_fit        , 0, ofs++ );
      //       gl_gauss->addWidget         ( lbl_gauss_fit       , 0, ofs++ );
      //       gl_gauss->addWidget         ( le_gauss_fit_start  , 0, ofs++ );
      //       gl_gauss->addWidget         ( le_gauss_fit_end    , 0, ofs++ );
      gl_gauss->addWidget         ( pb_gauss_save       , 0, ofs++ );
   }

   QGridLayout *gl_gauss2 = new QGridLayout(0);
   { 
      int ofs = 1;
      gl_gauss2->addMultiCellWidget( pb_ggauss_start     , 0, 0, 0, ofs++ );
      gl_gauss2->addWidget         ( cb_sd_weight        , 0, ofs++ );
      gl_gauss2->addWidget         ( cb_fix_width        , 0, ofs++ );
      gl_gauss2->addWidget         ( pb_gauss_fit        , 0, ofs++ );
      gl_gauss2->addWidget         ( pb_ggauss_rmsd      , 0, ofs++ );
      gl_gauss2->addWidget         ( lbl_gauss_fit       , 0, ofs++ );
      gl_gauss2->addWidget         ( le_gauss_fit_start  , 0, ofs++ );
      gl_gauss2->addWidget         ( le_gauss_fit_end    , 0, ofs++ );
      gl_gauss2->addWidget         ( pb_ggauss_results   , 0, ofs++ );
      gl_gauss2->addWidget         ( pb_gauss_as_curves  , 0, ofs++ );
   }

   QHBoxLayout *hbl_baseline = new QHBoxLayout( 0 );
   hbl_baseline->addWidget( pb_baseline_start   );
   hbl_baseline->addWidget( le_baseline_start_s );
   hbl_baseline->addWidget( le_baseline_start   );
   hbl_baseline->addWidget( le_baseline_start_e );
   hbl_baseline->addWidget( le_baseline_end_s   );
   hbl_baseline->addWidget( le_baseline_end     );
   hbl_baseline->addWidget( le_baseline_end_e   );
   hbl_baseline->addWidget( pb_baseline_apply   );

   QBoxLayout *vbl_plot_group = new QVBoxLayout(0);
   vbl_plot_group->addWidget ( plot_dist );
   vbl_plot_group->addLayout ( l_plot_errors );
   vbl_plot_group->addLayout ( gl_wheel  );
   vbl_plot_group->addLayout ( gl_gauss  );
   vbl_plot_group->addLayout ( gl_gauss2  );
   vbl_plot_group->addLayout ( hbl_baseline );
   vbl_plot_group->addLayout ( hbl_plot_buttons );

//    QBoxLayout *hbl_files_plot = new QHBoxLayout( 0 );
//    // hbl_files_plot->addLayout( vbl_files );
//    hbl_files_plot->addLayout( gl_files );
//    hbl_files_plot->addLayout( vbl_plot_group );

   QGridLayout *gl_files_plot = new QGridLayout( 0 );
   gl_files_plot->addLayout( gl_files      , 0, 0 );
   gl_files_plot->addLayout( vbl_plot_group, 0, 1 );
   gl_files_plot->setColStretch( 0, 0 );
   gl_files_plot->setColStretch( 1, 1 );

   QGridLayout *gl_bottom = new QGridLayout( 0 );
   gl_bottom->addWidget( pb_help  , 0, 0 );
   gl_bottom->addWidget( progress , 0, 1 );
   gl_bottom->addWidget( pb_cancel, 0, 2 );
   
   QVBoxLayout *background = new QVBoxLayout(this);
   background->addSpacing( 1 );
   // background->addWidget ( lbl_title );
   // background->addSpacing( 1 );
   background->addLayout ( gl_files_plot );
   background->addSpacing( 1 );
   background->addLayout ( gl_bottom );
   background->addSpacing( 1 );
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
   fn = QFileDialog::getSaveFileName(QString::null, QString::null,this );
   if(!fn.isEmpty() )
   {
      QString text = editor->text();
      QFile f( fn );
      if ( !f.open( IO_WriteOnly | IO_Translate) )
      {
         return;
      }
      QTextStream t( &f );
      t << text;
      f.close();
      editor->setModified( false );
      setCaption( fn );
   }
}

void US_Hydrodyn_Saxs_Hplc::update_enables()
{
   if ( running )
   {
      cout << "update_enables return (running)\n";
      return;
   }
   cout << "update_enables\n";

   // cout << "US_Hydrodyn_Saxs_Hplc::update_enables()\n";
   // cout << QString("saxs_window->qsl_plotted_iq_names.size() %1\n").arg(saxs_window->qsl_plotted_iq_names.size());

   pb_add_files          ->setEnabled( true );
   pb_regex_load         ->setEnabled( true );

   lb_files              ->setEnabled( true );
   lb_created_files      ->setEnabled( true );

   unsigned int files_selected_count                      = 0;
   // unsigned int non_hplc_non_empty_files_selected_count = 0;
   unsigned int last_selected_pos                         = 0;

   map < QString, bool > selected_map;

   QString last_selected_file;

   QStringList selected_files;

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         selected_files << lb_files->text( i );
         selected_map[ lb_files->text( i ) ] = true;
         last_selected_pos = i;
         last_selected_file = lb_files->text( i );
         files_selected_count++;
         //          if ( lb_files->text( i ) != lbl_hplc->text() &&
         //               lb_files->text( i ) != lbl_empty->text() )
         //          {
         //             non_hplc_non_empty_files_selected_count++;
         //          }
      }
   }

   bool files_compatible = compatible_files( selected_files );
   bool files_are_time   = type_files      ( selected_files );

   lbl_selected->setText( QString( tr( "%1 of %2 files selected" ) )
                          .arg( files_selected_count )
                          .arg( lb_files->numRows() ) );

   unsigned int files_created_selected_not_saved_count = 0;
   unsigned int files_created_selected_count           = 0;
   unsigned int files_created_selected_not_shown_count = 0;
   map < QString, bool > created_selected_map;

   QString last_created_selected_file;

   for ( int i = 0; i < lb_created_files->numRows(); i++ )
   {
      if ( lb_created_files->isSelected( i ) )
      {
         last_created_selected_file = lb_created_files->text( i );
         created_selected_map[ lb_created_files->text( i ) ] = true;
         files_created_selected_count++;
         if ( !selected_map.count( lb_created_files->text( i ) ) )
         {
            files_created_selected_not_shown_count++;
         } 
         if ( created_files_not_saved.count( lb_created_files->text( i ) ) )
         {
            files_created_selected_not_saved_count++;
         }
      }
   }

   lbl_selected_created->setText( QString( tr( "%1 of %2 files selected" ) )
                                  .arg( files_created_selected_count )
                                  .arg( lb_created_files->numRows() ) );

   unsigned int files_selected_not_created           = 0;
   for ( map < QString, bool >::iterator it = selected_map.begin();
         it != selected_map.end();
         it++ )
   {
      if ( !created_selected_map.count( it->first ) )
      {
         files_selected_not_created++;
      }
   }

   pb_wheel_start        ->setEnabled( files_selected_count > 0 && files_compatible && files_are_time );
   pb_gauss_start        ->setEnabled( files_selected_count == 1 && files_are_time );
   pb_ggauss_start       ->setEnabled( files_selected_count > 1 && files_are_time && gaussians.size() );
   cb_sd_weight          ->setEnabled( files_selected_count && files_are_time && gaussians.size() );
   cb_fix_width          ->setEnabled( files_selected_count && files_are_time && gaussians.size() );
   pb_baseline_start     ->setEnabled( files_selected_count == 1 && files_are_time );
   pb_baseline_apply     ->setEnabled( files_selected_count && 
                                       files_are_time && 
                                       le_baseline_start->text().toDouble() < le_baseline_end->text().toDouble() );

   pb_similar_files      ->setEnabled( files_selected_count == 1 );
   pb_conc               ->setEnabled( lb_files->numRows() > 0 );
   pb_clear_files        ->setEnabled( files_selected_count > 0 );
   pb_conc_avg           ->setEnabled( all_selected_have_nonzero_conc() && files_compatible && !files_are_time );
   pb_normalize          ->setEnabled( all_selected_have_nonzero_conc() && files_compatible && !files_are_time );
   pb_add                ->setEnabled( files_selected_count > 1 && files_compatible );
   pb_avg                ->setEnabled( files_selected_count > 1 && files_compatible && !files_are_time );
   pb_repeak             ->setEnabled( files_selected_count > 1 && files_compatible && files_are_time );
   pb_smooth             ->setEnabled( files_selected_count );
   pb_create_i_of_t      ->setEnabled( files_selected_count > 1 && files_compatible && !files_are_time );
   pb_create_i_of_q      ->setEnabled( files_selected_count > 1 && files_compatible && files_are_time && gaussians.size() );
   pb_conc_file          ->setEnabled( files_selected_count == 1 );
   pb_detector           ->setEnabled( true );

   //                                        );
   //    pb_set_hplc           ->setEnabled( files_selected_count == 1 && 
   //                                        lb_files->text( last_selected_pos ) != lbl_hplc->text() &&
   //                                        lb_files->text( last_selected_pos ) != lbl_empty ->text() &&
   //                                        lb_files->text( last_selected_pos ) != lbl_signal->text()
   //                                        );
   //    pb_set_signal         ->setEnabled( files_selected_count == 1 && 
   //                                        lb_files->text( last_selected_pos ) != lbl_hplc->text() &&
   //                                        lb_files->text( last_selected_pos ) != lbl_empty ->text() &&
   //                                        lb_files->text( last_selected_pos ) != lbl_signal->text() );
   //    pb_set_empty          ->setEnabled( files_selected_count == 1 && 
   //                                        lb_files->text( last_selected_pos ) != lbl_hplc->text() &&
   //                                        lb_files->text( last_selected_pos ) != lbl_empty ->text() &&
   //                                        lb_files->text( last_selected_pos ) != lbl_signal->text() );
   pb_select_all         ->setEnabled( lb_files->numRows() > 0 );
   pb_select_nth         ->setEnabled( lb_files->numRows() > 2 );
   pb_invert             ->setEnabled( lb_files->numRows() > 0 );
   pb_color_rotate       ->setEnabled( files_selected_count );
   //    pb_join               ->setEnabled( files_selected_count == 2 && files_compatible && !files_are_time );
   pb_adjacent           ->setEnabled( lb_files->numRows() > 1 );
   pb_to_saxs            ->setEnabled( files_selected_count && files_compatible && !files_are_time );
   pb_view               ->setEnabled( files_selected_count && files_selected_count <= 10 );
   pb_rescale            ->setEnabled( files_selected_count > 0 );

   pb_select_all_created ->setEnabled( lb_created_files->numRows() > 0 );
   pb_invert_all_created ->setEnabled( lb_created_files->numRows() > 0 );
   pb_adjacent_created   ->setEnabled( lb_created_files->numRows() > 1 );
   pb_remove_created     ->setEnabled( files_created_selected_count > 0 );
   pb_save_created_csv   ->setEnabled( files_created_selected_count > 0 );
   pb_save_created       ->setEnabled( files_created_selected_not_saved_count > 0 );

   pb_show_created       ->setEnabled( files_created_selected_not_shown_count > 0 );
   pb_show_only_created  ->setEnabled( files_created_selected_count > 0 &&
                                       files_selected_not_created > 0 );

   pb_select_vis       ->setEnabled( 
                                    files_selected_count &&
                                    plot_dist_zoomer && 
                                    plot_dist_zoomer->zoomRect() != plot_dist_zoomer->zoomBase() 
                                    );
   pb_remove_vis       ->setEnabled( 
                                    files_selected_count &&
                                    plot_dist_zoomer && 
                                    plot_dist_zoomer->zoomRect() != plot_dist_zoomer->zoomBase() 
                                    );
   pb_crop_common      ->setEnabled( files_selected_count && files_compatible );

   pb_crop_vis         ->setEnabled( 
                                    files_selected_count &&
                                    plot_dist_zoomer && 
                                    plot_dist_zoomer->zoomRect() != plot_dist_zoomer->zoomBase()
                                    );
   pb_crop_zero         ->setEnabled( 
                                    files_selected_count
                                    );
   pb_crop_left        ->setEnabled( 
                                    files_selected_count &&
                                    plot_dist_zoomer && 
                                    plot_dist_zoomer->zoomRect() != plot_dist_zoomer->zoomBase()
                                    );
   pb_crop_undo        ->setEnabled( crop_undos.size() );
   pb_crop_right       ->setEnabled( 
                                    files_selected_count &&
                                    plot_dist_zoomer && 
                                    plot_dist_zoomer->zoomRect() != plot_dist_zoomer->zoomBase()
                                    );
   pb_legend           ->setEnabled( lb_files->numRows() && files_selected_count <= 20 );
   pb_axis_x           ->setEnabled( lb_files->numRows() );
   pb_axis_y           ->setEnabled( lb_files->numRows() );
   // cb_guinier          ->setEnabled( files_selected_count );
   legend_set();

   pb_stack_push_all   ->setEnabled( lb_files->numRows() );
   pb_stack_push_sel   ->setEnabled( files_selected_count );
   pb_stack_pcopy      ->setEnabled( files_selected_count  && clipboard.files.size() );
   pb_stack_copy       ->setEnabled( files_selected_count );
   pb_stack_paste      ->setEnabled( clipboard.files.size() );
   pb_stack_drop       ->setEnabled( stack_data.size() );
   pb_stack_join       ->setEnabled( stack_data.size() );
   pb_stack_rot_up     ->setEnabled( stack_data.size() > 1 );
   pb_stack_rot_down   ->setEnabled( stack_data.size() > 1 );
   pb_stack_swap       ->setEnabled( stack_data.size() );

   if ( files_selected_count == 2 && files_compatible )
   {
      pb_errors           ->setEnabled( true );
   } else {
      pb_errors           ->setEnabled( false );
      hide_widgets( plot_errors_widgets, true );
   }

   pb_save_state       ->setEnabled( lb_files->numRows() || stack_data.size() );

   pb_p3d              ->setEnabled( files_selected_count > 1 && files_compatible && files_are_time );
}

void US_Hydrodyn_Saxs_Hplc::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->color();
   editor->setColor(color);
   editor->append(msg);
   editor->setColor(save_color);
}

void US_Hydrodyn_Saxs_Hplc::editor_msg_qc( QColor qcolor, QString msg )
{
   QColor save_color = editor->color();
   editor->setColor(qcolor);
   editor->append(msg);
   editor->setColor(save_color);
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
void US_Hydrodyn_Saxs_Hplc::clear_files( QStringList files )
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
         editor_msg( "black", QString( tr( "Removed %1" ) ).arg( lb_files->text( i ) ) );
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
         f_conc     .erase( lb_files->text( i ) );
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

   if ( *saxs_widget )
   {
      if ( cb_lock_dir->isChecked() )
      {
         saxs_window->add_to_directory_history( lbl_dir->text() );
      }
      saxs_window->select_from_directory_history( use_dir, this );
      raise();
   }

   QStringList filenames = QFileDialog::getOpenFileNames(
                                                         "dat files [foxs / other] (*.dat);;"
                                                         "All files (*);;"
                                                         "ssaxs files (*.ssaxs);;"
                                                         // "csv files (*.csv);;"
                                                         // "int files [crysol] (*.int);;"
                                                         // "fit files [crysol] (*.fit);;"
                                                         "txt files [specify q, I, sigma columns] (*.txt);;"
                                                         , use_dir
                                                         , this
                                                         , "open file dialog"
                                                         , "Add files" // ? "Set files for grid files"
                                                         );
   
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
   if ( conc_widget )
   {
      conc_window->refresh( csv_conc );
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::plot_files()
{
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

   if ( all_selected_files().size() > 20 &&
#ifndef QT4
        plot_dist->autoLegend() 
#else
        legend_vis
#endif
        )
   {
      legend();
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
   plot_dist->replot();
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

   minx = f_qs[ file ][ 0 ];
   maxx = f_qs[ file ][ f_qs[ file ].size() - 1 ];

   miny = f_Is[ file ][ 0 ];
   maxy = f_Is[ file ][ 0 ];
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

#ifndef QT4
   plot_dist->setCurveData( Iq, 
                            /* cb_guinier->isChecked() ? (double *)&(plotted_q2[p][0]) : */
                            (double *)&( f_qs[ file ][ 0 ] ),
                            (double *)&( f_Is[ file ][ 0 ] ),
                            q_points
                            );
   plot_dist->setCurvePen( Iq, QPen( plot_colors[ f_pos[ file ] % plot_colors.size()], 1, SolidLine));
#else
   curve->setData(
                  /* cb_guinier->isChecked() ?
                     (double *)&(plotted_q2[p][0]) : */
                  (double *)&( f_qs[ file ][ 0 ] ),
                  (double *)&( f_Is[ file ][ 0 ] ),
                  q_points
                  );

   curve->setPen( QPen( plot_colors[ f_pos[ file ] % plot_colors.size() ], 1, Qt::SolidLine ) );
   curve->attach( plot_dist );
#endif
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
   if ( !f_qs_string .count( file ) ||
        !f_qs        .count( file ) ||
        !f_Is        .count( file ) ||
        !f_pos       .count( file ) )
   {
      // editor_msg( "red", QString( tr( "Internal error: requested %1, but not found in data" ) ).arg( file ) );
      return false;
   }

   minx = f_qs[ file ][ 0 ];
   maxx = f_qs[ file ][ f_qs[ file ].size() - 1 ];

   miny = f_Is[ file ][ 0 ];
   maxy = f_Is[ file ][ 0 ];
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
   return true;
}

void US_Hydrodyn_Saxs_Hplc::invert()
{
   lb_files->invertSelection();
}

void US_Hydrodyn_Saxs_Hplc::select_nth()
{
   bool ok;
   int n = QInputDialog::getInteger(
                                    tr( "SOMO: HPLC select every Nth file" ),
                                    tr( "Select every Nth file\nEnter N:" ),
                                    2, 
                                    2,
                                    lb_files->numRows(),
                                    1, 
                                    &ok, 
                                    this 
                                    );
   if ( !ok ) {
      return;
   }

   int ofs = QInputDialog::getInteger(
                                    tr( "SOMO: HPLC select every Nth file: enter offset" ),
                                    tr( "Select the starting curve offset\nEnter offset:" ),
                                    1, 
                                    1,
                                    lb_files->numRows(),
                                    1, 
                                    &ok, 
                                    this 
                                    );
   if ( !ok ) {
      return;
   }

   disable_updates = true;
   lb_files->clearSelection();
   for ( int i = ofs - 1; i < lb_files->numRows(); i += n )
   {
      lb_files->setSelected( i, true );
   }
   disable_updates = false;
   plot_files();
   update_enables();
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
                         // "out|"
                         "ssaxs)$" );

   if ( rx_valid_ext.search( ext ) == -1 )
   {
      errormsg = QString("Error: %1 unsupported file extension %2").arg( filename ).arg( ext );
      return false;
   }
      
   if ( !f.open( IO_ReadOnly ) )
   {
      errormsg = QString("Error: can not open %1, check permissions ").arg( filename );
      return false;
   }

   QTextStream ts(&f);
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

   if ( ext == "dat" )
   {
      QRegExp rx_conc      ( "Conc:\\s*(\\S+)(\\s|$)" );
      QRegExp rx_psv       ( "PSV:\\s*(\\S+)(\\s|$)" );
      if ( rx_conc.search( qv[ 0 ] ) )
      {
         this_conc = rx_conc.cap( 1 ).toDouble();
      }
      if ( rx_psv.search( qv[ 0 ] ) )
      {
         this_psv = rx_psv.cap( 1 ).toDouble();
      }
   }

   // we should make some configuration for matches & offsets or column mapping
   // just an ad-hoc fix for APS 5IDD
   int q_offset   = 0;
   int I_offset   = 1;
   int e_offset   = 2;
   int row_offset = 1;
   if ( ext == "dat" && qv[ 0 ].contains( "Frame data" ) )
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

   if ( ext == "dat" && qv[ 0 ].contains( " Global State file" ) )
   {
      QRegExp rx_dir         ( "^# __dir: (\\S+)\\s*$" );
      QRegExp rx_lock_dir    ( "^# __lock_dir\\s*$" );
      QRegExp rx_created_dir ( "^# __created_dir: (\\S+)\\s*$" );
      QRegExp rx_files       ( "^# __files\\s*$" );
      QRegExp rx_end         ( "^# __end\\s*$" );
      QRegExp rx_gaussians   ( "^# __gaussians\\s*$" );
      QRegExp rx_f_gaussians ( "^# __f_gaussians: (\\S+)\\s*$" );
      QRegExp rx_push        ( "^# __push_stack\\s*$" );

      QRegExp rx_uv         ( "^# __detector_uv: (\\S+)\\s*$" );
      QRegExp rx_ri         ( "^# __detector_ri: (\\S+)\\s*$" );
      QRegExp rx_uv_set     ( "^# __detector_uv_set\\s*$" );
      QRegExp rx_ri_set     ( "^# __detector_ri_set\\s*$" );

      clear_files( all_files() );

      cb_lock_dir->setChecked( false );
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
                  if ( tokens.size() != 3 )
                  {
                     errormsg = QString("Error: Gaussian file %1 incorrect format line %2").arg( filename ).arg( i + 1 );
                     return false;
                  }

                  gaussians.push_back( tokens[ 0 ].toDouble() );
                  gaussians.push_back( tokens[ 1 ].toDouble() );
                  gaussians.push_back( tokens[ 2 ].toDouble() );

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
                  if ( tokens.size() != 3 )
                  {
                     errormsg = QString("Error: loading file specific Gaussians from file %1 incorrect format line %2").arg( filename ).arg( i + 1 );
                     return false;
                  }

                  tmp_g.push_back( tokens[ 0 ].toDouble() );
                  tmp_g.push_back( tokens[ 1 ].toDouble() );
                  tmp_g.push_back( tokens[ 2 ].toDouble() );

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
         
               if ( tokens.size() != 3 )
               {
                  errormsg = QString("Error: Multiple Gaussian file %1 incorrect format line %2").arg( filename ).arg( i + 1 );
                  return false;
               }

               g.push_back( tokens[ 0 ].toDouble() );
               g.push_back( tokens[ 1 ].toDouble() );
               g.push_back( tokens[ 2 ].toDouble() );
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
         
            if ( tokens.size() != 3 )
            {
               errormsg = QString("Error: Gaussian file %1 incorrect format line %2").arg( filename ).arg( i + 1 );
               return false;
            }

            gaussians.push_back( tokens[ 0 ].toDouble() );
            gaussians.push_back( tokens[ 1 ].toDouble() );
            gaussians.push_back( tokens[ 2 ].toDouble() );
         }

         editor_msg( "black", QString( "%1 Gaussians loaded from file %2" ).arg( gaussians.size() / 3 ).arg( filename ) );
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

   if ( this_conc != 0e0 )
   {
      update_csv_conc();

      for ( unsigned int i = 0; i < csv_conc.data.size(); i++ )
      {
         if ( csv_conc.data[ i ].size() > 1 &&
              csv_conc.data[ i ][ 0 ] == basename )
         {
            csv_conc.data[ i ][ 1 ] = QString( "%1" ).arg( this_conc );
         }
      }
      if ( conc_widget )
      {
         conc_window->refresh( csv_conc );
      }
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

void US_Hydrodyn_Saxs_Hplc::avg( QStringList files )
{
   // create average of selected

   vector < QString > avg_qs_string;
   vector < double >  avg_qs;
   vector < double >  avg_Is;
   vector < double >  avg_Is2;

   QStringList selected_files;

   unsigned int selected_count = 0;

   update_csv_conc();
   map < QString, double > concs = current_concs();
   double avg_conc = 0e0;
   double avg_psv  = 0e0;

   // copies for potential cropping:

   map < QString, vector < QString > > t_qs_string;
   map < QString, vector < double > >  t_qs;
   map < QString, vector < double > >  t_Is;
   map < QString, vector < double > >  t_errors;

   bool first = true;
   bool crop  = false;
   unsigned int min_q_len = 0;

   bool all_nonzero_errors = true;

   for ( int i = 0; i < (int)files.size(); i++ )
   {
      QString this_file = files[ i ];

      t_qs_string[ this_file ] = f_qs_string[ this_file ];
      t_qs       [ this_file ] = f_qs       [ this_file ];
      t_Is       [ this_file ] = f_Is       [ this_file ];
      if ( f_errors[ this_file ].size() )
      {
         t_errors [ this_file ] = f_errors[ this_file ];
      } else {
         all_nonzero_errors = false;
      }

      if ( first )
      {
         first = false;
         min_q_len = t_qs[ this_file ].size();
      } else {
         if ( min_q_len > t_qs[ this_file ].size() )
         {
            min_q_len = t_qs[ this_file ].size();
            crop = true;
         } else {
            if ( min_q_len != t_qs[ this_file ].size() )
            {
               crop = true;
            }
         }  
      }
   }

   if ( crop )
   {
      editor_msg( "dark red", QString( tr( "Notice: averaging requires cropping to %1 points" ) ).arg( min_q_len ) );
      for ( map < QString, vector < double > >::iterator it = t_qs.begin();
            it != t_qs.end();
            it++ )
      {
         t_qs_string[ it->first ].resize( min_q_len );
         t_qs       [ it->first ].resize( min_q_len );
         t_Is       [ it->first ].resize( min_q_len );
         if ( t_errors[ it->first ].size() )
         {
            t_errors   [ it->first ].resize( min_q_len );
         } 
      }
   } 

   if ( all_nonzero_errors )
   {
      for ( map < QString, vector < double > >::iterator it = t_qs.begin();
            it != t_qs.end();
            it++ )
      {
         if ( t_errors[ it->first ].size() )
         {
            if ( all_nonzero_errors &&
                 !is_nonzero_vector ( t_errors[ it->first ] ) )
            {
               all_nonzero_errors = false;
               break;
            }
         } else {
            all_nonzero_errors = false;
            break;
         }
      }      
   }

   first = true;

   vector < double > sum_weight;
   vector < double > sum_weight2;

   if ( all_nonzero_errors )
   {
      editor_msg( "black" ,
                  tr( "Notice: using standard deviation in mean calculation" ) );
   } else {
      editor_msg( "black" ,
                  tr( "Notice: NOT using standard deviation in mean calculation, since some sd's were zero or missing" ) );
   }

   for ( int i = 0; i < (int)files.size(); i++ )
   {
      QString this_file = files[ i ];

      selected_count++;
      selected_files << this_file;
      if ( all_nonzero_errors )
      {
         for ( int j = 0; j < (int)t_Is[ this_file ].size(); j++ )
         {
            t_Is[ this_file ][ j ] /= t_errors[ this_file ][ j ] * t_errors[ this_file ][ j ];
         }
      }         

      if ( first )
      {
         first = false;
         avg_qs_string = t_qs_string[ this_file ];
         avg_qs        = t_qs       [ this_file ];
         avg_Is        = t_Is       [ this_file ];
         avg_Is2       .resize( t_Is[ this_file ].size() );
         sum_weight    .resize( avg_qs.size() );
         sum_weight2   .resize( avg_qs.size() );
         for ( int j = 0; j < (int)t_Is[ this_file ].size(); j++ )
         {
            double weight    = all_nonzero_errors ? 1e0 / ( t_errors[ this_file ][ j ] * t_errors[ this_file ][ j ] ) : 1e0;
            sum_weight[ j ]  = weight;
            sum_weight2[ j ] = weight * weight;
            avg_Is2[ j ]     = t_Is[ this_file ][ j ] * t_Is[ this_file ][ j ];
         }
         avg_conc = 
            concs.count( this_file ) ?
            concs[ this_file ] :
            0e0;
         avg_psv = f_psv.count( this_file ) ? f_psv[ this_file ] : 0e0;
      } else {
         if ( avg_qs.size() != t_qs[ this_file ].size() )
         {
            editor_msg( "red", tr( "Error: incompatible grids, the files selected do not have the same number of points" ) );
            return;
         }
         for ( int j = 0; j < (int)t_Is[ this_file ].size(); j++ )
         {
            if ( fabs( avg_qs[ j ] - t_qs[ this_file ][ j ] ) > Q_VAL_TOL )
            {
               editor_msg( "red", tr( "Error: incompatible grids, the q values differ between selected files" ) );
               cout << QString( "val1 %1 val2 %2 fabs diff %3\n" ).arg( avg_qs[ j ] ).arg(  t_qs[ this_file ][ j ]  ).arg(fabs( avg_qs[ j ] - t_qs[ this_file ][ j ] ) );
               return;
            }
            avg_Is [ j ]     += t_Is[ this_file ][ j ];
            double weight    = all_nonzero_errors ? 1e0 / ( t_errors[ this_file ][ j ] * t_errors[ this_file ][ j ] ) : 1e0;
            sum_weight[ j ]  += weight;
            sum_weight2[ j ] += weight * weight;
            avg_Is2[ j ]     += t_Is[ this_file ][ j ] * t_Is[ this_file ][ j ];
         }
         avg_conc +=
            concs.count( this_file ) ?
            concs[ this_file ] :
            0e0;
         avg_psv += f_psv.count( this_file ) ? f_psv[ this_file ] : 0e0;
      }            
   }

   if ( selected_count < 2 )
   {
      editor_msg( "red", tr( "Error: not at least 2 files selected so there is nothing to average" ) );
      return;
   }      

   vector < double > avg_sd( avg_qs.size() );
   for ( int i = 0; i < (int)avg_qs.size(); i++ )
   {
      avg_Is[ i ] /= sum_weight[ i ];

      double sum = 0e0;
      for ( int j = 0; j < (int)files.size(); j++ )
      {
         QString this_file = files[ j ];
         double weight  = all_nonzero_errors ? 1e0 / ( t_errors[ this_file ][ i ] * t_errors[ this_file ][ i ] ) : 1e0;
         double invweight  = all_nonzero_errors ? ( t_errors[ this_file ][ i ] * t_errors[ this_file ][ i ] ) : 1e0;
         sum += weight * ( invweight * t_Is[ this_file ][ i ] - avg_Is[ i ] ) * ( invweight * t_Is[ this_file ][ i ] - avg_Is[ i ] );
      }
      sum *= sum_weight[ i ] / ( sum_weight[ i ] * sum_weight[ i ] - sum_weight2[ i ] );
      avg_sd[ i ] = sqrt( sum ) * sqrt( 1e0 / (double) files.size() );
   }

   avg_conc /= files.size();
   avg_psv  /= files.size();

   // determine name
   // find common header & tail substrings

   QString head = qstring_common_head( selected_files, true );
   QString tail = qstring_common_tail( selected_files, true );

   int ext = 0;

   if ( !head.isEmpty() &&
        !head.contains( QRegExp( "_$" ) ) )
   {
      head += "_";
   }
   if ( !tail.isEmpty() &&
        !tail.contains( QRegExp( "^_" ) ) )
   {
      tail = "_" + tail;
   }

   QString avg_name = head + "avg" + tail;

   map < QString, bool > current_files;
   for ( int i = 0; i < (int)lb_files->numRows(); i++ )
   {
      current_files[ lb_files->text( i ) ] = true;
   }

   while ( current_files.count( avg_name ) )
   {
      avg_name = head + QString( "avg-%1" ).arg( ++ext ) + tail;
   }

   lb_created_files->insertItem( avg_name );
   lb_created_files->setBottomItem( lb_created_files->numRows() - 1 );
   lb_files->insertItem( avg_name );
   lb_files->setBottomItem( lb_files->numRows() - 1 );
   created_files_not_saved[ avg_name ] = true;
   
   f_pos       [ avg_name ] = f_qs.size();
   f_qs_string [ avg_name ] = avg_qs_string;
   f_qs        [ avg_name ] = avg_qs;
   f_Is        [ avg_name ] = avg_Is;
   f_errors    [ avg_name ] = avg_sd;
   f_is_time   [ avg_name ] = false;
   f_conc      [ avg_name ] = avg_conc;
   f_psv       [ avg_name ] = avg_psv;

   {
      vector < double > tmp;
      f_gaussians  [ avg_name ] = tmp;
   }
   
   // we could check if it has changed and then delete
   // if ( plot_dist_zoomer )
   // {
   // delete plot_dist_zoomer;
   // plot_dist_zoomer = (ScrollZoomer *) 0;
   // }
   update_csv_conc();
   for ( int i = 0; i < (int)csv_conc.data.size(); i++ )
   {
      if ( csv_conc.data[ i ].size() > 1 &&
           csv_conc.data[ i ][ 0 ] == avg_name )
      {
         csv_conc.data[ i ][ 1 ] = QString( "%1" ).arg( avg_conc );
      }
   }

   if ( conc_widget )
   {
      conc_window->refresh( csv_conc );
   }
   update_enables();
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
      if ( lb_created_files->isSelected( i ) && 
           created_files_not_saved.count( lb_created_files->text( i ) ) )
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
   if ( !f.open( IO_WriteOnly ) )
   {
      editor_msg( "red", QString( tr( "Error: can not open %1 for writing" ) ).arg( use_filename ) );
      return false;
   }

   QTextStream ts( &f );

   // copies for potential cropping:

   map < QString, vector < QString > > t_qs_string;
   map < QString, vector < double > >  t_qs;
   map < QString, vector < double > >  t_Is;
   map < QString, vector < double > >  t_errors;

   bool first = true;
   bool crop  = false;
   unsigned int min_q_len = 0;

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

      t_qs_string[ this_file ] = f_qs_string[ this_file ];
      t_qs       [ this_file ] = f_qs       [ this_file ];
      t_Is       [ this_file ] = f_Is       [ this_file ];
      t_errors   [ this_file ] = f_errors   [ this_file ];
      if ( first )
      {
         first = false;
         min_q_len = t_qs[ this_file ].size();
      } else {
         if ( min_q_len > t_qs[ this_file ].size() )
         {
            min_q_len = t_qs[ this_file ].size();
            crop = true;
         } else {
            if ( min_q_len != t_qs[ this_file ].size() )
            {
               crop = true;
            }
         }  
      }
   }

   if ( crop )
   {
      editor_msg( "dark red", QString( tr( "Notice: output contains versions cropped to %1 points for compatibility" ) ).arg( min_q_len ) );
      for ( map < QString, vector < double > >::iterator it = t_qs.begin();
            it != t_qs.end();
            it++ )
      {
         t_qs_string[ it->first ].resize( min_q_len );
         t_qs       [ it->first ].resize( min_q_len );
         t_Is       [ it->first ].resize( min_q_len );
         if ( t_errors[ it->first ].size() )
         {
            t_errors   [ it->first ].resize( min_q_len );
         }
      }
   }

   if ( !t_qs.count( files[ 0 ] ) )
   {
      editor_msg( "red", QString( tr( "Internal error: requested %1, but not found in data" ) ).arg( files[ 0 ] ) );
      f.close();
      return false;
   }

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
      if ( t_errors.count( files[ i ] ) && t_errors[ files[ i ] ].size() )
      {
         ts << 
            QString( "\"%1\",\"%2\",%3\n" )
            .arg( files[ i ] )
            .arg( "I(q) sd" )
            .arg( vector_double_to_csv( t_errors[ files[ i ] ] ) );
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
      use_filename = f_name[ file ];
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
   if ( !f.open( IO_WriteOnly ) )
   {
      editor_msg( "red", QString( tr( "Error: can not open %1 for writing" ) ).arg( use_filename ) );
      return false;
   }

   QTextStream ts( &f );

   ts << QString( tr( "US-SOMO Hplc %1data: %2%3%4\n" ) )
      .arg( ( f_is_time.count( file ) && f_is_time[ file ] ? "Frame " : "" ) )
      .arg( file )
      .arg( f_psv.count( file ) ? QString( " PSV:%1" ).arg( f_psv[ file ] ) : QString( "" ) )
      .arg( f_conc.count( file ) ? QString( " Conc:%1" ).arg( f_conc[ file ] ) : QString( "" ) )
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
                              tr( "US-SOMO: SAXS Hplc" ),
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
      {
         vector < double > tmp;
         f_gaussians  [ fname ] = tmp;
      }
   }      
   update_enables();
}


void US_Hydrodyn_Saxs_Hplc::conc_avg( QStringList files )
{
   // create average of selected
   vector < QString > avg_qs_string;
   vector < double >  avg_qs;
   vector < double >  avg_Is;
   vector < double >  avg_Is2;

   QStringList selected_files;

   unsigned int selected_count = 0;

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

   double avg_conc = 0e0;
   double avg_psv  = 0e0;

   vector < double > nIs;

   double tot_conc;
   double tot_conc2;

   // copies for potential cropping:

   map < QString, vector < QString > > t_qs_string;
   map < QString, vector < double > >  t_qs;
   map < QString, vector < double > >  t_Is;
   map < QString, vector < double > >  t_errors;

   bool first = true;
   bool crop  = false;
   unsigned int min_q_len = 0;

   bool all_nonzero_errors = true;

   for ( int i = 0; i < (int)files.size(); i++ )
   {
      QString this_file = files[ i ];
      t_qs_string[ this_file ] = f_qs_string[ this_file ];
      t_qs       [ this_file ] = f_qs       [ this_file ];
      t_Is       [ this_file ] = f_Is       [ this_file ];
      if ( f_errors[ this_file ].size() )
      {
         t_errors [ this_file ] = f_errors[ this_file ];
      } else {
         all_nonzero_errors = false;
      }

      if ( first )
      {
         first = false;
         min_q_len = t_qs[ this_file ].size();
      } else {
         if ( min_q_len > t_qs[ this_file ].size() )
         {
            min_q_len = t_qs[ this_file ].size();
            crop = true;
         } else {
            if ( min_q_len != t_qs[ this_file ].size() )
            {
               crop = true;
            }
         }  
      }
   }

   if ( crop )
   {
      editor_msg( "dark red", QString( tr( "Notice: averaging requires cropping to %1 points" ) ).arg( min_q_len ) );
      for ( map < QString, vector < double > >::iterator it = t_qs.begin();
            it != t_qs.end();
            it++ )
      {
         t_qs_string[ it->first ].resize( min_q_len );
         t_qs       [ it->first ].resize( min_q_len );
         t_Is       [ it->first ].resize( min_q_len );
         if ( t_errors[ it->first ].size() )
         {
            t_errors   [ it->first ].resize( min_q_len );
         }
      }
   }

   if ( all_nonzero_errors )
   {
      for ( map < QString, vector < double > >::iterator it = t_qs.begin();
            it != t_qs.end();
            it++ )
      {
         if ( t_errors[ it->first ].size() )
         {
            if ( all_nonzero_errors &&
                 !is_nonzero_vector ( t_errors[ it->first ] ) )
            {
               all_nonzero_errors = false;
               break;
            }
         } else {
            all_nonzero_errors = false;
            break;
         }
      }      
   }

   first = true;

   vector < double > sum_weight;
   vector < double > sum_weight2;

   if ( all_nonzero_errors )
   {
      editor_msg( "black" ,
                  tr( "Notice: using standard deviation in mean calculation" ) );
   } else {
      editor_msg( "black" ,
                  tr( "Notice: NOT using standard deviation in mean calculation, since some sd's were zero or missing" ) );
   }

   for ( int i = 0; i < (int)files.size(); i++ )
   {
      QString this_file = files[ i ];

      selected_count++;
      selected_files << this_file;

      if ( all_nonzero_errors )
      {
         for ( int j = 0; j < (int)t_Is[ this_file ].size(); j++ )
         {
            t_Is[ this_file ][ j ] /= t_errors[ this_file ][ j ] * t_errors[ this_file ][ j ];
         }
      }         

      if ( !inv_concs.count( this_file ) )
      {
         editor_msg( "red", QString( tr( "Error: found zero or no concentration for %1" ) ).arg( this_file ) );
         return;
      }
      if ( first )
      {
         first = false;
         tot_conc  = inv_concs[ this_file ];
         tot_conc2 = tot_conc * tot_conc;
         avg_qs_string = t_qs_string[ this_file ];
         avg_qs        = t_qs       [ this_file ];
         nIs           = t_Is       [ this_file ];
         sum_weight    .resize( avg_qs.size() );
         sum_weight2   .resize( avg_qs.size() );
         for ( int j = 0; j < (int)nIs.size(); j++ )
         {
            nIs[ j ] *= inv_concs[ this_file ];
         }
         
         avg_Is        = nIs;
         avg_Is2       .resize( nIs.size() );
         for ( int j = 0; j < (int)nIs.size(); j++ )
         {
            double weight    = all_nonzero_errors ? 1e0 / ( t_errors[ this_file ][ j ] * t_errors[ this_file ][ j ] ) : 1e0;
            sum_weight[ j ]  = weight;
            sum_weight2[ j ] = weight * weight;
            avg_Is2[ j ] = nIs[ j ] * nIs[ j ];
         }
         avg_conc = 
            concs.count( this_file ) ?
            concs[ this_file ] :
            0e0;
         avg_psv = f_psv.count( this_file ) ? f_psv[ this_file ] : 0e0;
      } else {
         if ( avg_qs.size() != t_qs[ this_file ].size() )
         {
            editor_msg( "red", tr( "Error: incompatible grids, the files selected do not have the same number of points" ) );
            return;
         }
         tot_conc  += inv_concs[ this_file ];
         tot_conc2 += inv_concs[ this_file ] * inv_concs[ this_file ];
         nIs       = t_Is     [ this_file ];
         for ( int j = 0; j < (int)nIs.size(); j++ )
         {
            if ( fabs( avg_qs[ j ] - t_qs[ this_file ][ j ] ) > Q_VAL_TOL )
            {
               cout << QString( "val1 %1 val2 %2 fabs diff %3\n" ).arg( avg_qs[ j ] ).arg(  t_qs[ this_file ][ j ]  ).arg(fabs( avg_qs[ j ] - t_qs[ this_file ][ j ] ) );
               editor_msg( "red", tr( "Error: incompatible grids, the q values differ between selected files" ) );
               return;
            }
            nIs[ j ] *= inv_concs[ this_file ];
            avg_Is [ j ] += nIs[ j ];
            double weight    = all_nonzero_errors ? 1e0 / ( t_errors[ this_file ][ j ] * t_errors[ this_file ][ j ] ) : 1e0;
            sum_weight[ j ]  += weight;
            sum_weight2[ j ] += weight * weight;
            avg_Is2[ j ] += nIs[ j ] * nIs[ j ];
         }
         avg_conc +=
            concs.count( this_file ) ?
            concs[ this_file ] :
            0e0;
         avg_psv += f_psv.count( this_file ) ? f_psv[ this_file ] : 0e0;
      }            
   }

   if ( selected_count < 2 )
   {
      editor_msg( "red", tr( "Error: not at least 2 files selected so there is nothing to average" ) );
      return;
   }      

   vector < double > avg_sd( avg_qs.size() );

   avg_conc /= files.size();
   avg_psv  /= files.size();

   for ( int i = 0; i < (int)avg_qs.size(); i++ )
   {
      avg_Is[ i ] /= sum_weight[ i ];

      double sum = 0e0;
      for ( int j = 0; j < (int)files.size(); j++ )
      {
         QString this_file = files[ j ];
         double weight    = all_nonzero_errors ? 1e0 / ( t_errors[ this_file ][ i ] * t_errors[ this_file ][ i ] ) : 1e0;
         double invweight = all_nonzero_errors ? ( t_errors[ this_file ][ i ] * t_errors[ this_file ][ i ] ) : 1e0;
         sum += weight 
            * ( invweight * inv_concs[ this_file ] * t_Is[ this_file ][ i ] - avg_Is[ i ] ) 
            * ( invweight * inv_concs[ this_file ] * t_Is[ this_file ][ i ] - avg_Is[ i ] );
      }
      sum *= sum_weight[ i ] / ( sum_weight[ i ] * sum_weight[ i ] - sum_weight2[ i ] );
      avg_sd[ i ] = sqrt( sum ) * avg_conc * sqrt( 1e0 / (double) files.size() );

      avg_Is[ i ] *= avg_conc;
   }

   // determine name
   // find common header & tail substrings

   QString head = qstring_common_head( selected_files, true );
   QString tail = qstring_common_tail( selected_files, true );

   unsigned int ext = 0;

   if ( !head.isEmpty() &&
        !head.contains( QRegExp( "_$" ) ) )
   {
      head += "_";
   }
   if ( !tail.isEmpty() &&
        !tail.contains( QRegExp( "^_" ) ) )
   {
      tail = "_" + tail;
   }

   QString avg_name = head + "cnavg" + tail;

   map < QString, bool > current_files;
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      current_files[ lb_files->text( i ) ] = true;
   }

   while ( current_files.count( avg_name ) )
   {
      avg_name = head + QString( "cnavg-%1" ).arg( ++ext ) + tail;
   }

   lb_created_files->insertItem( avg_name );
   lb_created_files->setBottomItem( lb_created_files->numRows() - 1 );
   lb_files->insertItem( avg_name );
   lb_files->setBottomItem( lb_files->numRows() - 1 );
   created_files_not_saved[ avg_name ] = true;
   
   f_pos       [ avg_name ] = f_qs.size();
   f_qs_string [ avg_name ] = avg_qs_string;
   f_qs        [ avg_name ] = avg_qs;
   f_Is        [ avg_name ] = avg_Is;
   f_errors    [ avg_name ] = avg_sd;
   f_is_time   [ avg_name ] = false;
   f_conc      [ avg_name ] = avg_conc;
   f_psv       [ avg_name ] = avg_psv;
   {
      vector < double > tmp;
      f_gaussians  [ avg_name ] = tmp;
   }
   
   // we could check if it has changed and then delete
   // if ( plot_dist_zoomer )
   // {
   // delete plot_dist_zoomer;
   // plot_dist_zoomer = (ScrollZoomer *) 0;
   // }
   update_csv_conc();
   // cout << QString( "trying to set csv_conc to conc %1 for %2\n" ).arg( avg_conc ).arg( avg_name );
   for ( unsigned int i = 0; i < csv_conc.data.size(); i++ )
   {
      if ( csv_conc.data[ i ].size() > 1 &&
           csv_conc.data[ i ][ 0 ] == avg_name )
      {
         csv_conc.data[ i ][ 1 ] = QString( "%1" ).arg( avg_conc );
         // cout << QString( "Found & set csv_conc to conc %1 for %2\n" ).arg( avg_conc ).arg( avg_name );
      }
   }

   if ( conc_widget )
   {
      conc_window->refresh( csv_conc );
   }
   update_enables();
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
   plot_dist->replot();
   if ( !gaussian_mode &&
        !ggaussian_mode && 
        !baseline_mode &&
        !timeshift_mode )
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

void US_Hydrodyn_Saxs_Hplc::to_saxs()
{
   // copy selected to saxs window
   if ( !activate_saxs_window() )
   {
      return;
   }
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         QString this_file = lb_files->text( i );
         if ( f_qs.count( this_file ) &&
              f_Is.count( this_file ) )
         {
            if ( f_errors.count( this_file ) &&
                 f_errors[ this_file ].size() )
            {
               saxs_window->plot_one_iqq( f_qs    [ this_file ],
                                          f_Is    [ this_file ],
                                          f_errors[ this_file ],
                                          this_file );
            } else {
               saxs_window->plot_one_iqq( f_qs    [ this_file ],
                                          f_Is    [ this_file ],
                                          this_file );
            }
         } else {
            editor_msg( "red", QString( tr( "Internal error: requested %1, but not found in data" ) ).arg( this_file ) );
         }
      }
   }
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
         edit->setPalette ( QPalette( USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal ) );
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

void US_Hydrodyn_Saxs_Hplc::axis_y()
{
   axis_y_log = !axis_y_log;

   if ( axis_y_log )
   {
      plot_dist->setAxisTitle(QwtPlot::yLeft, tr("Log10 I(q) or I(t) or OD or RI") );
#ifndef QT4
      plot_dist->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::Logarithmic);
#else
      plot_dist->setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);
#endif
   } else {
      plot_dist->setAxisTitle(QwtPlot::yLeft, tr("I(q) or I(t) or OD or RI") );
#ifndef QT4
      plot_dist->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::None);
#else
      // actually need to test this, not sure what the correct version is
      plot_dist->setAxisScaleEngine(QwtPlot::yLeft, new QwtScaleEngine );
#endif
   }
   plot_dist->replot();
}

void US_Hydrodyn_Saxs_Hplc::axis_x()
{
   axis_x_log = !axis_x_log;
   if ( axis_x_log )
   {
      plot_dist->setAxisTitle(QwtPlot::xBottom,  tr("Log10 q (1/Angstrom) or Time or Frame") );
#ifndef QT4
      plot_dist->setAxisOptions(QwtPlot::xBottom, QwtAutoScale::Logarithmic);
#else
      plot_dist->setAxisScaleEngine(QwtPlot::xBottom, new QwtLog10ScaleEngine);
#endif
   } else {
      plot_dist->setAxisTitle(QwtPlot::xBottom,  tr("q (1/Angstrom) or Time or Frame") );
#ifndef QT4
      plot_dist->setAxisOptions(QwtPlot::xBottom, QwtAutoScale::None);
#else
      // actually need to test this, not sure what the correct version is
      plot_dist->setAxisScaleEngine(QwtPlot::xBottom, new QwtScaleEngine );
#endif
   }
   plot_dist->replot();
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

void US_Hydrodyn_Saxs_Hplc::rename_created( QListBoxItem *lbi, const QPoint & )
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

#define UHSH_WHEEL_RES 10000000

void US_Hydrodyn_Saxs_Hplc::adjust_wheel( double pos )
{
   cout << QString("pos is now %1 wheel step is %2\n").arg(pos, 0, 'f', 8 ).arg( qwtw_wheel->step() );
   if ( gaussian_mode )
   {
      if ( le_gauss_pos->hasFocus() )
      {
         cout << "aw: pos focus\n";
         le_last_focus = le_gauss_pos;
      }
      if ( le_gauss_pos_width->hasFocus() )
      {
         cout << "aw: pos width focus\n";
         le_last_focus = le_gauss_pos_width;
      }
      if ( le_gauss_pos_height->hasFocus() )
      {
         cout << "aw: pos height focus\n";
         le_last_focus = le_gauss_pos_height;
      }
      if ( le_gauss_fit_start->hasFocus() )
      {
         cout << "aw: fit start focus\n";
         le_last_focus = le_gauss_fit_start;
      }
      if ( le_gauss_fit_end->hasFocus() )
      {
         cout << "aw: fit end focus\n";
         le_last_focus = le_gauss_fit_end;
      }

      if ( !le_last_focus )
      {
         le_last_focus = le_gauss_pos;
         cout << "aw: pos focus, since no last\n";
         disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
         qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], 
                               f_qs[ wheel_file ].back(), 
                               ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
         connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
         return;
      }

      le_last_focus->setText( QString( "%1" ).arg( pos ) );

      lbl_wheel_pos->setText( QString( "%1" ).arg( pos ) );

   } else {
      if ( baseline_mode )
      {
         if ( le_baseline_start_s->hasFocus() )
         {
            cout << "aw: baseline start_s focus\n";
            le_last_focus = le_baseline_start_s;
         }
         if ( le_baseline_start->hasFocus() )
         {
            cout << "aw: baseline start focus\n";
            le_last_focus = le_baseline_start;
         }
         if ( le_baseline_start_e->hasFocus() )
         {
            cout << "aw: baseline start_e focus\n";
            le_last_focus = le_baseline_start_e;
         }

         if ( le_baseline_end_s->hasFocus() )
         {
            cout << "aw: baseline end_s focus\n";
            le_last_focus = le_baseline_end_s;
         }
         if ( le_baseline_end->hasFocus() )
         {
            cout << "aw: baseline end focus\n";
            le_last_focus = le_baseline_end;
         }
         if ( le_baseline_end_e->hasFocus() )
         {
            cout << "aw: baseline end_e focus\n";
            le_last_focus = le_baseline_end_e;
         }

         if ( !le_last_focus )
         {
            cout << "aw: no last focus in baseline mode\n";
            return;
         }

         le_last_focus->setText( QString( "%1" ).arg( pos ) );

         lbl_wheel_pos->setText( QString( "%1" ).arg( pos ) );
      } else {
         if ( ggaussian_mode )
         {
            lbl_gauss_fit ->setText( "?" );
            plot_errors      ->clear();
            plot_errors      ->replot();
            plot_errors_grid  .clear();
            plot_errors_target.clear();
            plot_errors_fit   .clear();
            plot_errors_errors.clear();
            pb_ggauss_rmsd->setEnabled( true );

            if ( le_gauss_pos->hasFocus() )
            {
               cout << "aw: pos focus\n";
               le_last_focus = le_gauss_pos;
            }
            if ( cb_fix_width->isChecked() &&
                 le_gauss_pos_width->hasFocus() )
            {
               cout << "aw: pos width focus\n";
               le_last_focus = le_gauss_pos_width;
            }
            if ( le_gauss_fit_start->hasFocus() )
            {
               cout << "aw: fit start focus\n";
               le_last_focus = le_gauss_fit_start;
            }
            if ( le_gauss_fit_end->hasFocus() )
            {
               cout << "aw: fit end focus\n";
               le_last_focus = le_gauss_fit_end;
            }

            if ( !le_last_focus )
            {
               cout << "aw: pos focus, since no last\n";
               le_last_focus = le_gauss_pos;
            }

            le_last_focus->setText( QString( "%1" ).arg( pos ) );

            lbl_wheel_pos->setText( QString( "%1" ).arg( pos ) );
            
         } else {
            // timeshift mode
            // adjust selected time
            pb_wheel_save->setEnabled( pos != 0e0 );

            lbl_wheel_pos->setText( QString( "%1" ).arg( pos ) );

            vector < double > offset_q = f_qs[ wheel_file ];
            for ( unsigned int i = 0; i < ( unsigned int ) offset_q.size(); i++ )
            {
               offset_q[ i ] += pos;
            }
#ifndef QT4
            plot_dist->setCurveData( wheel_curve, 
                                     /* cb_guinier->isChecked() ? (double *)&(plotted_q2[p][0]) : */
                                     (double *)&( offset_q[ 0 ] ),
                                     (double *)&( f_Is[ wheel_file ][ 0 ] ),
                                     offset_q.size()
                                     );
#else
            wheel_curve->setData(
                                 /* cb_guinier->isChecked() ?
                                    (double *)&(plotted_q2[p][0]) : */
                                 (double *)&( offset_q[ 0 ] ),
                                 (double *)&( f_Is[ wheel_file ][ 0 ] ),
                                 offset_q.size()
                                 );
#endif
            plot_dist->replot();
         }
      }
   }
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

   gaussian_mode  = false;
   ggaussian_mode = false;
   baseline_mode  = false;

   if ( !plotted_curves.count( wheel_file ) )
   {
      editor_msg( "red", QString( tr( "Internal error: request to timeshift %1, but not found in data" ) ).arg( wheel_file ) );
      return;
   }

   wheel_curve           = plotted_curves[ wheel_file ];

   running               = true;

   disable_all();
   timeshift_mode = true;

   plot_errors->clear();
   
   pb_rescale            ->setEnabled( true );
   pb_wheel_start        ->setEnabled( false );
   pb_wheel_cancel       ->setEnabled( true );
   qwtw_wheel            ->setEnabled( true );
   qwtw_wheel            ->setRange  ( -100, 100, .1 );
   qwtw_wheel            ->setValue  ( 0 );
}

void US_Hydrodyn_Saxs_Hplc::disable_all()
{
   cout << "disable all\n";
   pb_similar_files      ->setEnabled( false );
   pb_conc               ->setEnabled( false );
   pb_clear_files        ->setEnabled( false );
   pb_avg                ->setEnabled( false );
   pb_normalize          ->setEnabled( false );
   pb_conc_avg           ->setEnabled( false );
   pb_repeak             ->setEnabled( false );
   pb_smooth             ->setEnabled( false );
   pb_create_i_of_t      ->setEnabled( false );
   pb_create_i_of_q      ->setEnabled( false );
   pb_conc_file          ->setEnabled( false );
   pb_detector           ->setEnabled( false );
   //    pb_set_hplc           ->setEnabled( false );
   //    pb_set_signal         ->setEnabled( false );
   //    pb_set_empty          ->setEnabled( false );
   pb_select_all         ->setEnabled( false );
   pb_invert             ->setEnabled( false );
   pb_select_nth         ->setEnabled( false );
   pb_color_rotate       ->setEnabled( false );
   //    pb_join               ->setEnabled( false );
   pb_adjacent           ->setEnabled( false );
   pb_to_saxs            ->setEnabled( false );
   pb_view               ->setEnabled( false );
   pb_rescale            ->setEnabled( false );
   pb_select_all_created ->setEnabled( false );
   pb_adjacent_created   ->setEnabled( false );
   pb_save_created_csv   ->setEnabled( false );
   pb_save_created       ->setEnabled( false );
   pb_show_created       ->setEnabled( false );
   pb_show_only_created  ->setEnabled( false );
   pb_select_vis         ->setEnabled( false );
   pb_remove_vis         ->setEnabled( false ); 
   pb_crop_common        ->setEnabled( false ); 
   pb_crop_vis           ->setEnabled( false ); 
   pb_crop_zero          ->setEnabled( false ); 
   pb_crop_left          ->setEnabled( false ); 
   pb_crop_undo          ->setEnabled( false );
   pb_crop_right         ->setEnabled( false ); 
   pb_legend             ->setEnabled( false );
   pb_axis_x             ->setEnabled( false );
   pb_axis_y             ->setEnabled( false );

   pb_add_files          ->setEnabled( false );
   pb_regex_load         ->setEnabled( false );

   lb_files              ->setEnabled( false );
   lb_created_files      ->setEnabled( false );

   pb_wheel_start        ->setEnabled( false );

   pb_gauss_start        ->setEnabled( false );
   pb_gauss_clear        ->setEnabled( false );
   pb_gauss_new          ->setEnabled( false );
   pb_gauss_delete       ->setEnabled( false );
   pb_gauss_prev         ->setEnabled( false );
   pb_gauss_next         ->setEnabled( false );
   pb_gauss_fit          ->setEnabled( false );
   pb_gauss_save         ->setEnabled( false );
   pb_wheel_cancel       ->setEnabled( false );

   le_gauss_pos          ->setEnabled( false );
   le_gauss_pos_width    ->setEnabled( false );
   le_gauss_pos_height   ->setEnabled( false );
   le_gauss_fit_start    ->setEnabled( false );
   le_gauss_fit_end      ->setEnabled( false );

   pb_baseline_start     ->setEnabled( false );
   le_baseline_start_s   ->setEnabled( false );
   le_baseline_start     ->setEnabled( false );
   le_baseline_start_e   ->setEnabled( false );
   le_baseline_end_s     ->setEnabled( false );
   le_baseline_end       ->setEnabled( false );
   le_baseline_end_e     ->setEnabled( false );
   pb_baseline_apply     ->setEnabled( false );

   pb_ggauss_start       ->setEnabled( false );
   pb_ggauss_rmsd        ->setEnabled( false );
   pb_ggauss_results     ->setEnabled( false );

   pb_gauss_as_curves    ->setEnabled( false );
   cb_sd_weight          ->setEnabled( false );
   cb_fix_width          ->setEnabled( false );

   pb_errors             ->setEnabled( false );

   pb_stack_push_all     ->setEnabled( false );
   pb_stack_push_sel     ->setEnabled( false );
   pb_stack_copy         ->setEnabled( false );
   pb_stack_pcopy        ->setEnabled( false );
   pb_stack_paste        ->setEnabled( false );
   pb_stack_drop         ->setEnabled( false );
   pb_stack_join         ->setEnabled( false );
   pb_stack_rot_up       ->setEnabled( false );
   pb_stack_rot_down     ->setEnabled( false );
   pb_stack_swap         ->setEnabled( false );

   pb_save_state         ->setEnabled( false );
   pb_invert_all_created ->setEnabled( false );
   pb_remove_created     ->setEnabled( false );

   pb_add                ->setEnabled( false );
   pb_p3d                ->setEnabled( false );
}

void US_Hydrodyn_Saxs_Hplc::wheel_cancel()
{
   errors_were_on = plot_errors->isVisible();
   hide_widgets( plot_errors_widgets, true );

   disable_all();

   if ( ggaussian_mode )
   {
      f_gaussians = org_f_gaussians;
      gaussians = org_gaussians;
      gauss_delete_markers();
      plotted_markers.clear();
      plot_dist->replot();
   } else {
      if ( gaussian_mode )
      {
         gaussians = org_gaussians;
         gauss_delete_markers();
         plotted_markers.clear();
         gauss_delete_gaussians();
         plotted_gaussians.clear();
         plotted_gaussian_sum.clear();
         if ( plotted_curves.count( wheel_file ) &&
              f_pos.count( wheel_file ) )
         {
#ifndef QT4
            plot_dist->setCurvePen( plotted_curves[ wheel_file ], QPen( plot_colors[ f_pos[ wheel_file ] % plot_colors.size()], 1, SolidLine));
#else
            plotted_curves[ wheel_file ]->setPen( QPen( plot_colors[ f_pos[ wheel_file ] % plot_colors.size() ], 1, Qt::SolidLine ) );
#endif
         }
      } else {
         if ( baseline_mode )
         {
            le_baseline_start_s->setText( QString( "%1" ).arg( org_baseline_start_s ) );
            le_baseline_start  ->setText( QString( "%1" ).arg( org_baseline_start   ) );
            le_baseline_start_e->setText( QString( "%1" ).arg( org_baseline_start_e ) );
            le_baseline_end_s  ->setText( QString( "%1" ).arg( org_baseline_end_s   ) );
            le_baseline_end    ->setText( QString( "%1" ).arg( org_baseline_end     ) );
            le_baseline_end_e  ->setText( QString( "%1" ).arg( org_baseline_end_e   ) );
            gauss_delete_markers();
            plotted_markers.clear();
            for ( unsigned int i = 0; i < ( unsigned int ) plotted_baseline.size(); i++ )
            {
#ifndef QT4
               plot_dist->removeCurve( plotted_baseline[ i ] );
#else
#warn check how to do this in qt4
#endif
            }
            if ( plotted_curves.count( wheel_file ) &&
                 f_pos.count( wheel_file ) )
            {
#ifndef QT4
               plot_dist->setCurvePen( plotted_curves[ wheel_file ], QPen( plot_colors[ f_pos[ wheel_file ] % plot_colors.size()], 1, SolidLine));
#else
               plotted_curves[ wheel_file ]->setPen( QPen( plot_colors[ f_pos[ file ] % plot_colors.size() ], 1, Qt::SolidLine ) );
#endif
            }
         } else {
            lbl_wheel_pos->setText( QString( "%1" ).arg( 0 ) );
#ifndef QT4
            plot_dist->setCurveData( wheel_curve, 
                                     /* cb_guinier->isChecked() ? (double *)&(plotted_q2[p][0]) : */
                                     (double *)&( f_qs[ wheel_file ][ 0 ] ),
                                     (double *)&( f_Is[ wheel_file ][ 0 ] ),
                                     f_qs[ wheel_file ].size()
                                     );
#else
            wheel_curve->setData(
                                 /* cb_guinier->isChecked() ?
                                    (double *)&(plotted_q2[p][0]) : */
                                 (double *)&( f_qs[ wheel_file ][ 0 ] ),
                                 (double *)&( f_Is[ wheel_file ][ 0 ] ),
                                 f_qs[ wheel_file ].size()
                                 );
#endif
         }
      }
   }

   plot_dist->replot();

   gaussian_mode         = false;
   ggaussian_mode        = false;
   baseline_mode         = false;
   timeshift_mode        = false;

   qwtw_wheel            ->setEnabled( false );
   pb_wheel_save         ->setEnabled( false );
   pb_wheel_cancel       ->setEnabled( false );

   running               = false;

   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::wheel_save()
{
   if ( ggaussian_mode )
   {
      // org_gaussians = gaussians;
      // ? f_gaussians[ wheel_file ] = gaussians;
      if ( unified_ggaussian_ok )
      {
         if ( cb_fix_width->isChecked() )
         {
            for ( unsigned int i = 0; i < ( unsigned int ) unified_ggaussian_files.size(); i++ )
            {
               vector < double > g;
               unsigned int  index = 2 * unified_ggaussian_gaussians_size + i * unified_ggaussian_gaussians_size;

               for ( unsigned int j = 0; j < unified_ggaussian_gaussians_size; j++ )
               {
                  g.push_back( unified_ggaussian_params[ index + j + 0 ] );
                  g.push_back( unified_ggaussian_params[ 2 * j + 0 ] );
                  g.push_back( unified_ggaussian_params[ 2 * j + 1 ] );
               }
               f_gaussians[ unified_ggaussian_files[ i ] ] = g;
            }
         } else {
            for ( unsigned int i = 0; i < ( unsigned int ) unified_ggaussian_files.size(); i++ )
            {
               vector < double > g;
               unsigned int  index = unified_ggaussian_gaussians_size + i * 2 * unified_ggaussian_gaussians_size;

               for ( unsigned int j = 0; j < unified_ggaussian_gaussians_size; j++ )
               {
                  g.push_back( unified_ggaussian_params[ index + 2 * j + 0 ] );
                  g.push_back( unified_ggaussian_params[ j ] );
                  g.push_back( unified_ggaussian_params[ index + 2 * j + 1 ] );
               }
               f_gaussians[ unified_ggaussian_files[ i ] ] = g;
            }
         }            

         org_f_gaussians = f_gaussians;
         org_gaussians   = f_gaussians[ wheel_file ];
      }               

      wheel_cancel();
      return;
   }

   if ( gaussian_mode )
   {
      org_gaussians = gaussians;
      if ( f_gaussians.count( wheel_file ) )
      {
         f_gaussians[ wheel_file ] = gaussians;
      }
      wheel_cancel();
      return;
   }

   if ( baseline_mode )
   {
      org_baseline_start_s = le_baseline_start_s->text().toDouble();
      org_baseline_start   = le_baseline_start  ->text().toDouble();
      org_baseline_start_e = le_baseline_start_e->text().toDouble();
      org_baseline_end_s   = le_baseline_end_s  ->text().toDouble();
      org_baseline_end     = le_baseline_end    ->text().toDouble();
      org_baseline_end_e   = le_baseline_end_e  ->text().toDouble();
      wheel_cancel();
      return;
   }

   qwtw_wheel            ->setEnabled( false );
   pb_wheel_save         ->setEnabled( false );
   pb_wheel_cancel       ->setEnabled( false );
   lbl_wheel_pos->setText( QString( "%1" ).arg( qwtw_wheel->value() ) );

   // save time adjusted selected as new
   map < QString, bool > current_files;

   int wheel_pos = -1;

   for ( int i = 0; i < (int)lb_files->numRows(); i++ )
   {
      current_files[ lb_files->text( i ) ] = true;
      if ( lb_files->text( i ) == wheel_file )
      {
         wheel_pos = i;
      }
   }

   QString save_name = wheel_file + QString( "_ts%1" ).arg( qwtw_wheel->value() ).replace( ".", "_" );

   int ext = 0;
   while ( current_files.count( save_name ) )
   {
      save_name = wheel_file + QString( "_ts%1-%2" ).arg( qwtw_wheel->value() ).arg( ++ext ).replace( ".", "_" );
   }
   
   cout << QString( "new name is %1\n" ).arg( save_name );

   lb_created_files->insertItem( save_name );
   lb_created_files->setBottomItem( lb_created_files->numRows() - 1 );
   lb_files->insertItem( save_name );
   lb_files->setBottomItem( lb_files->numRows() - 1 );
   created_files_not_saved[ save_name ] = true;

   f_pos       [ save_name ] = f_qs.size();
   f_qs        [ save_name ] = f_qs        [ wheel_file ];
   f_qs_string [ save_name ] = f_qs_string [ wheel_file ];

   for ( unsigned int i = 0; i < ( unsigned int ) f_qs[ save_name ].size(); i++ )
   {
      f_qs       [ save_name ][ i ] += qwtw_wheel->value();
      f_qs_string[ save_name ][ i ] = QString( "%1" ).arg( f_qs[ save_name ][ i ] );
   }

   f_Is        [ save_name ] = f_Is        [ wheel_file ];
   f_errors    [ save_name ] = f_errors    [ wheel_file ];
   f_is_time   [ save_name ] = true;
   f_conc      [ save_name ] = f_conc.count( wheel_file ) ? f_conc[ wheel_file ] : 0e0;
   f_psv       [ save_name ] = f_psv .count( wheel_file ) ? f_psv [ wheel_file ] : 0e0;
   {
      vector < double > tmp;
      f_gaussians  [ save_name ] = tmp;
   }

   lb_files->setSelected( f_pos[ save_name ], true );
   if ( wheel_pos != -1 )
   {
      lb_files->setSelected( wheel_pos, false );
   }

   plot_dist->replot();

   qwtw_wheel            ->setEnabled( false );
   pb_wheel_save         ->setEnabled( false );
   pb_wheel_cancel       ->setEnabled( false );

   timeshift_mode        = false;
   running               = false;

   update_enables();

}

void US_Hydrodyn_Saxs_Hplc::gaussian_enables()
{
   unsigned int sizeover3 = ( unsigned int )gaussians.size() / 3;

   pb_gauss_start      ->setEnabled( false );
   pb_gauss_clear      ->setEnabled( sizeover3 );
   pb_gauss_new        ->setEnabled( true );
   pb_gauss_delete     ->setEnabled( sizeover3 );
   pb_gauss_prev       ->setEnabled( sizeover3 > 1 && gaussian_pos > 0 );
   pb_gauss_next       ->setEnabled( sizeover3 > 1 && gaussian_pos < sizeover3 - 1 );
   cb_sd_weight        ->setEnabled( sizeover3 && le_gauss_fit_start->text().toDouble() < le_gauss_fit_end->text().toDouble() );
   pb_gauss_fit        ->setEnabled( sizeover3 && le_gauss_fit_start->text().toDouble() < le_gauss_fit_end->text().toDouble() );
   pb_wheel_cancel     ->setEnabled( true );
   pb_wheel_save       ->setEnabled( true );
   le_gauss_pos        ->setEnabled( sizeover3 && gaussian_pos < sizeover3 );
   le_gauss_pos_width  ->setEnabled( sizeover3 && gaussian_pos < sizeover3 );
   le_gauss_pos_height ->setEnabled( sizeover3 && gaussian_pos < sizeover3 );
   le_gauss_fit_start  ->setEnabled( sizeover3 && gaussian_pos < sizeover3 );
   le_gauss_fit_end    ->setEnabled( sizeover3 && gaussian_pos < sizeover3 );
   pb_gauss_save       ->setEnabled( sizeover3 );
   pb_gauss_as_curves  ->setEnabled( sizeover3 );
   qwtw_wheel          ->setEnabled( sizeover3 && gaussian_pos < sizeover3 );
   pb_rescale          ->setEnabled( true );
   pb_view             ->setEnabled( true );
   pb_errors           ->setEnabled( true );
}

void US_Hydrodyn_Saxs_Hplc::update_gauss_pos()
{
   if ( gaussian_mode )
   {
      if ( gaussians.size() )
      {
         lbl_gauss_pos      ->setText( QString( " %1 of %2 " ).arg( gaussian_pos + 1 ).arg( gaussians.size() / 3 ) );

         disconnect( le_gauss_pos       , SIGNAL( textChanged( const QString & ) ), 0, 0 );
         disconnect( le_gauss_pos_width , SIGNAL( textChanged( const QString & ) ), 0, 0 );
         disconnect( le_gauss_pos_height, SIGNAL( textChanged( const QString & ) ), 0, 0 );

         le_gauss_pos_height->setText( QString( "%1" ).arg( gaussians[ 0 + 3 * gaussian_pos ], 0, 'f', 6 ) );
         le_gauss_pos       ->setText( QString( "%1" ).arg( gaussians[ 1 + 3 * gaussian_pos ], 0, 'f', 6 ) );
         le_gauss_pos_width ->setText( QString( "%1" ).arg( gaussians[ 2 + 3 * gaussian_pos ], 0, 'f', 6 ) );

         connect( le_gauss_pos       , SIGNAL( textChanged( const QString & ) ), SLOT( gauss_pos_text       ( const QString & ) ) );
         connect( le_gauss_pos_width , SIGNAL( textChanged( const QString & ) ), SLOT( gauss_pos_width_text ( const QString & ) ) );
         connect( le_gauss_pos_height, SIGNAL( textChanged( const QString & ) ), SLOT( gauss_pos_height_text( const QString & ) ) );

         if ( le_gauss_pos->hasFocus() ||
              !( le_gauss_pos_height->hasFocus() ||
                 le_gauss_pos_width ->hasFocus() ||
                 le_gauss_fit_start ->hasFocus() ||
                 le_gauss_fit_end   ->hasFocus() ) )
         {
            qwtw_wheel   ->setValue( gaussians[ 1 + 3 * gaussian_pos ] );
         }
         if ( le_gauss_pos_height->hasFocus() )
         {
            qwtw_wheel   ->setValue( gaussians[ 0 + 3 * gaussian_pos ] );
         }
         if ( le_gauss_pos_width->hasFocus() )
         {
            qwtw_wheel   ->setValue( gaussians[ 2 + 3 * gaussian_pos ] );
         }

         for ( unsigned int i = 2; i < ( unsigned int ) plotted_markers.size(); i++ )
         {
            if ( gaussian_pos + 2 == i && !le_gauss_fit_start->hasFocus() && !le_gauss_fit_end->hasFocus() )
            {
#ifndef QT4
               plot_dist->setMarkerPen       ( plotted_markers[ i ], QPen( Qt::magenta, 2, DashDotDotLine));
#else
#warn check how to do this in qt4 needs ymark symsize
               plotted_markers[ i ]->setSymbol( QwtSymbol( QwtSymbol::VLine,
                                                           QBrush( Qt::white ), QPen( Qt::magenta, 2, Qt::DashLine ),
                                                           QSize( 8, sizeym ) ) );
#endif
            } else {
#ifndef QT4
               plot_dist->setMarkerPen       ( plotted_markers[ i ], QPen( Qt::blue, 2, DashDotDotLine));
#else
#warn check how to do this in qt4 needs ymark symsize
               plotted_markers[ i ]->setSymbol( QwtSymbol( QwtSymbol::VLine,
                                                           QBrush( Qt::white ), QPen( Qt::blue, 2, Qt::DashLine ),
                                                           QSize( 8, sizeym ) ) );
#endif
            }
         }
         plot_dist->replot();
      } else {
         lbl_gauss_pos       ->setText( " 0 of 0 " );
         le_gauss_pos        ->setText( "" );
         le_gauss_pos_width  ->setText( "" );
         le_gauss_pos_height ->setText( "" );
      }      
   } else {
      // global gaussian mode
      {
         if ( cb_fix_width->isChecked() )
         {
            lbl_gauss_pos      ->setText( QString( " %1 of %2 " ).arg( gaussian_pos + 1 ).arg( gaussians.size() / 3 ) );

            disconnect( le_gauss_pos       , SIGNAL( textChanged( const QString & ) ), 0, 0 );
            disconnect( le_gauss_pos_width , SIGNAL( textChanged( const QString & ) ), 0, 0 );

            le_gauss_pos       ->setText( QString( "%1" ).arg( unified_ggaussian_params[ 0 + 2 * gaussian_pos ] ) );
            le_gauss_pos_width ->setText( QString( "%1" ).arg( unified_ggaussian_params[ 1 + 2 * gaussian_pos ] ) );

            connect( le_gauss_pos       , SIGNAL( textChanged( const QString & ) ), SLOT( gauss_pos_text       ( const QString & ) ) );
            connect( le_gauss_pos_width , SIGNAL( textChanged( const QString & ) ), SLOT( gauss_pos_width_text ( const QString & ) ) );

            if ( le_gauss_pos->hasFocus() ||
                 !( 
                   le_gauss_pos_width ->hasFocus() ||
                   le_gauss_fit_start ->hasFocus() ||
                   le_gauss_fit_end   ->hasFocus() ) )
            {
               qwtw_wheel   ->setValue( unified_ggaussian_params[ 0 + 2 * gaussian_pos ] );
            }
            if ( le_gauss_pos_width->hasFocus() )
            {
               qwtw_wheel   ->setValue( unified_ggaussian_params[ 1 + 2 * gaussian_pos ] );
            }

         } else {
            lbl_gauss_pos      ->setText( QString( " %1 of %2 " ).arg( gaussian_pos + 1 ).arg( gaussians.size() / 3 ) );

            disconnect( le_gauss_pos       , SIGNAL( textChanged( const QString & ) ), 0, 0 );

            le_gauss_pos       ->setText( QString( "%1" ).arg( unified_ggaussian_params[ gaussian_pos ] ) );

            connect( le_gauss_pos       , SIGNAL( textChanged( const QString & ) ), SLOT( gauss_pos_text       ( const QString & ) ) );

            if ( le_gauss_pos->hasFocus() ||
                 !( 
                   le_gauss_fit_start ->hasFocus() ||
                   le_gauss_fit_end   ->hasFocus() ) )
            {
               qwtw_wheel   ->setValue( unified_ggaussian_params[ gaussian_pos ] );
            }
         }
         for ( unsigned int i = 2; i < ( unsigned int ) plotted_markers.size(); i++ )
         {
            if ( gaussian_pos + 2 == i && !le_gauss_fit_start->hasFocus() && !le_gauss_fit_end->hasFocus() )
            {
#ifndef QT4
               plot_dist->setMarkerPen       ( plotted_markers[ i ], QPen( Qt::magenta, 2, DashDotDotLine));
#else
#warn check how to do this in qt4 needs ymark symsize
               plotted_markers[ i ]->setSymbol( QwtSymbol( QwtSymbol::VLine,
                                                           QBrush( Qt::white ), QPen( Qt::magenta, 2, Qt::DashLine ),
                                                           QSize( 8, sizeym ) ) );
#endif
            } else {
#ifndef QT4
               plot_dist->setMarkerPen       ( plotted_markers[ i ], QPen( Qt::blue, 2, DashDotDotLine));
#else
#warn check how to do this in qt4 needs ymark symsize
               plotted_markers[ i ]->setSymbol( QwtSymbol( QwtSymbol::VLine,
                                                           QBrush( Qt::white ), QPen( Qt::blue, 2, Qt::DashLine ),
                                                           QSize( 8, sizeym ) ) );
#endif
            }
         }
         plot_dist->replot();
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

   if ( gauss_max_height <= 0e0 )
   {
      editor_msg( "red", QString( tr( "Error: maximum y value of signal %1 is not positive" ) ).arg( wheel_file ) );
      return;
   }

#ifndef QT4
   plot_dist->setCurvePen( plotted_curves[ wheel_file ], QPen( Qt::cyan, 1, SolidLine));
#else
   plotted_curves[ wheel_file ]->setPen( QPen( plot_colors[ Qt::cyan, 1, Qt::SolidLine ) ) );
#endif

   org_gaussians = gaussians;

   if ( f_gaussians.count( wheel_file ) &&
        f_gaussians[ wheel_file ].size() )
   {
      cout << "using file specific gaussians\n";
      gaussians = f_gaussians[ wheel_file ];
   }

   opt_repeak_gaussians( wheel_file );

   gaussian_mode = true;

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

   if ( gaussian_pos >= gaussians.size() / 3 )
   {
      gaussian_pos = ( gaussians.size() / 3 ) - 1;
   }

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
   gaussians.push_back( gauss_max_height / 2 );
   gaussians.push_back( 0e0 );
   gaussians.push_back( 2e0 );
   gaussian_pos = ( gaussians.size() / 3 ) - 1;
   gauss_add_marker( 0e0, Qt::blue, QString( "%1" ).arg( gaussian_pos + 1 ) );
   gauss_add_gaussian( 0e0, 1e0, gauss_max_height / 2, Qt::green );
   disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
   update_gauss_pos();
   connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
   plot_dist->replot();
   gaussian_enables();
   if ( ggaussian_mode )
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
   if ( ggaussian_mode )
   {
      ggaussian_enables();
   } else {
      gaussian_enables();
   }
}

void US_Hydrodyn_Saxs_Hplc::gauss_next()
{
   if ( gaussian_pos < ( gaussians.size() / 3 ) - 1 )
   {
      gaussian_pos++;
   }
   update_gauss_pos();
   if ( ggaussian_mode )
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

   if ( gaussian_mode )
   {
      QString use_filename = wheel_file + "-gauss.dat";
   
      if ( QFile::exists( use_filename ) )
      {
         use_filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( use_filename, 0, this );
         raise();
      }

      QFile f( use_filename );
      if ( !f.open( IO_WriteOnly ) )
      {
         editor_msg( "red", QString( tr( "Error: can not open %1 for writing" ) ).arg( use_filename ) );
      }

      QTextStream ts( &f );

      ts << QString( "US-SOMO Hplc Gaussians: %1\n" ).arg( wheel_file );

      ts << QString( "%1 %2\n" )
         .arg( le_gauss_fit_start->text() )
         .arg( le_gauss_fit_end  ->text() )
         ;

      for ( unsigned int i = 0; i < ( unsigned int ) gaussians.size(); i += 3 )
      {
         ts << QString( "%1 %2 %3\n" )
            .arg( gaussians[ 0 + i ], 0, 'g', 10 )
            .arg( gaussians[ 1 + i ], 0, 'g', 10 )
            .arg( gaussians[ 2 + i ], 0, 'g', 10 );
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
         if ( !f.open( IO_WriteOnly ) )
         {
            editor_msg( "red", QString( tr( "Error: can not open %1 for writing" ) ).arg( use_filename ) );
         }

         QTextStream ts( &f );

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
   if ( gaussian_mode )
   {
      cout << QString( "gauss_pos_text <%1>\n" ).arg( text );
      gaussians[ 1 + 3 * gaussian_pos ] = text.toDouble();
   } else {
      if ( cb_fix_width->isChecked() )
      {
         unified_ggaussian_params[ 0 + 2 * gaussian_pos ] = text.toDouble();

         if ( plotted_hlines.size() > gaussian_pos )
         {
            puts( "gg pos text do adjust" );

            double center = unified_ggaussian_params[ 2 * gaussian_pos + 0 ];
            double width  = unified_ggaussian_params[ 2 * gaussian_pos + 1 ];
            double fwhm   = 2.354820045e0 * width;

            vector < double > x( 2 );
            vector < double > y( 2 );

            x[ 0 ] = center - fwhm / 2e0;
            x[ 1 ] = center + fwhm / 2e0;
            y[ 0 ] = gauss_max_height / 3e0 + ( gauss_max_height * (double) gaussian_pos / 100e0 );
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
#warn check how to do this in qt4 needs ymark
   plotted_markers[ 2 + gaussian_pos ]->setValue( pos, ymark );
#endif
   if ( qwtw_wheel->value() != text.toDouble() )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setValue( text.toDouble() );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
   }

   if ( gaussian_mode )
   {
      gauss_replot_gaussian();
   }
   plot_dist->replot();
}

void US_Hydrodyn_Saxs_Hplc::gauss_pos_width_text( const QString & text )
{
   if ( gaussian_mode )
   {
      gaussians[ 2 + 3 * gaussian_pos ] = text.toDouble();
   } else {
      if ( ggaussian_mode && 
           cb_fix_width->isChecked() )
      {
         unified_ggaussian_params[ 1 + 2 * gaussian_pos ] = text.toDouble();

         if ( plotted_hlines.size() > gaussian_pos )
         {
            puts( "gg width text do adjust" );

            double center = unified_ggaussian_params[ 2 * gaussian_pos + 0 ];
            double width  = unified_ggaussian_params[ 2 * gaussian_pos + 1 ];
            double fwhm   = 2.354820045e0 * width;

            vector < double > x( 2 );
            vector < double > y( 2 );

            x[ 0 ] = center - fwhm / 2e0;
            x[ 1 ] = center + fwhm / 2e0;
            y[ 0 ] = gauss_max_height / 3e0 + ( gauss_max_height * (double) gaussian_pos / 100e0 );
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

   if ( gaussian_mode )
   {
      gauss_replot_gaussian();
   }
   plot_dist->replot();
}

void US_Hydrodyn_Saxs_Hplc::gauss_pos_height_text( const QString & text )
{
   gaussians[ 0 + 3 * gaussian_pos ] = text.toDouble();
   if ( qwtw_wheel->value() != text.toDouble() )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setValue( text.toDouble() );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
   }
   gauss_replot_gaussian();
   plot_dist->replot();
}

void US_Hydrodyn_Saxs_Hplc::gauss_fit_start_text( const QString & text )
{
#ifndef QT4
   plot_dist->setMarkerPos( plotted_markers[ 0 ], text.toDouble(), 0e0 );
#else
#warn check how to do this in qt4 needs ymark
   plotted_markers[ 0 ]->setValue( pos, ymark );
#endif
   if ( qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
   if ( gaussian_mode )
   {
      replot_gaussian_sum();
   }
   plot_dist->replot();
}

void US_Hydrodyn_Saxs_Hplc::gauss_fit_end_text( const QString & text )
{
#ifndef QT4
   plot_dist->setMarkerPos( plotted_markers[ 1 ], text.toDouble(), 0e0 );
#else
#warn check how to do this in qt4 needs ymark
   plotted_markers[ 1 ]->setValue( pos, ymark );
#endif
   if ( qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
   if ( gaussian_mode )
   {
      replot_gaussian_sum();
   }
   plot_dist->replot();
}

void US_Hydrodyn_Saxs_Hplc::gauss_add_marker( double pos, QColor color, QString text, int align )
{
#ifndef QT4
   long marker = plot_dist->insertMarker();
   plot_dist->setMarkerLineStyle ( marker, QwtMarker::VLine );
   plot_dist->setMarkerPos       ( marker, pos, 0e0 );
   plot_dist->setMarkerLabelAlign( marker, align );
   plot_dist->setMarkerPen       ( marker, QPen( color, 2, DashDotDotLine));
   plot_dist->setMarkerFont      ( marker, QFont("Helvetica", 11, QFont::Bold));
   plot_dist->setMarkerLabelText ( marker, text );
#else
#warn check how to do this in qt4 needs ymark symsize
   QwtPlotMarker* marker = new QwtPlotMarker;
   marker->setSymbol( QwtSymbol( QwtSymbol::VLine,
                                 QBrush( Qt::white ), QPen( color, 2, Qt::DashLine ),
                                 QSize( 8, sizeym ) ) );
   marker->setValue( pos, ymark );
   marker->setLabelAlignment( align );
   marker->setLabel( text );
   marker->attach( plot_dist );
#endif
   plotted_markers.push_back( marker );
}   

void US_Hydrodyn_Saxs_Hplc::gauss_add_hline( double center, double width )
{
   double fwhm = 2.354820045e0 * width;

   vector < double > x( 2 );
   vector < double > y( 2 );

   x[ 0 ] = center - fwhm / 2e0;
   x[ 1 ] = center + fwhm / 2e0;
   y[ 0 ] = gauss_max_height / 3e0 + ( gauss_max_height * (double) plotted_hlines.size() / 100e0 );
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
   QwtPlotCurve *curve;
   QwtPlotCurve *curve = new QwtPlotCurve( file );
   curve->setStyle( QwtPlotCurve::Lines );
#endif

   plotted_hlines.push_back( curve );

#ifndef QT4
   plot_dist->setCurvePen( curve, QPen( Qt::green, 1, Qt::SolidLine ) );
   plot_dist->setCurveData( curve,
                            (double *)&x[ 0 ],
                            (double *)&y[ 0 ],
                            2
                            );
#else
   curve->setPen( QPen( Qt::green, 1, Qt::SolidLine ) );
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

   if ( gaussian_mode )
   {
      for ( unsigned int i = 0; i < ( unsigned int ) gaussians.size() / 3; i++ )
      {
         gauss_add_marker( gaussians[ 1 + 3 * i ], Qt::blue, QString( "%1" ).arg( i + 1 ) );
      }
   } else {
      // global gaussian
      if ( unified_ggaussian_ok )
      {
         if ( cb_fix_width->isChecked() )
         {
            for ( unsigned int i = 0; i < unified_ggaussian_gaussians_size; i++ )
            {
               gauss_add_marker( unified_ggaussian_params[ 2 * i + 0 ], Qt::blue, QString( "%1" ).arg( i + 1 ) );
               gauss_add_hline ( unified_ggaussian_params[ 2 * i + 0 ], unified_ggaussian_params[ 2 * i + 1 ]  );
            }
         } else {

            for ( unsigned int i = 0; i < unified_ggaussian_gaussians_size; i++ )
            {
               gauss_add_marker( unified_ggaussian_params[ i ], Qt::blue, QString( "%1" ).arg( i + 1 ) );
            }
         }
      }
   }
      
   plot_dist->replot();
}


void US_Hydrodyn_Saxs_Hplc::gauss_delete_markers()
{
#ifndef QT4
   plot_dist->removeMarkers();
#else
#warn check how to do this in qt4
#endif
   for ( unsigned int i = 0; i < ( unsigned int )plotted_hlines.size(); i++ )
   {
#ifndef QT4
      plot_dist->removeCurve( plotted_hlines[ i ] );
#else
#warn check how to do this in qt4
#endif
      
   }
}

vector < double > US_Hydrodyn_Saxs_Hplc::gaussian( double height, double center, double width )
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

   if ( width == 0e0 )
   {
      width = 1e-3;
   }

   result.resize( f_qs[ wheel_file ].size() );
   for ( unsigned int i = 0; i < f_qs[ wheel_file ].size(); i++ )
   {
      double tmp = ( f_qs[ wheel_file ][ i ] - center ) / width;
      result[ i ] = height * exp( - tmp * tmp / 2 );
   }
   return result;
}

void US_Hydrodyn_Saxs_Hplc::gauss_delete()
{
   vector < double > new_gaussians;
   for ( unsigned int i = 0; i < ( unsigned int ) gaussians.size() / 3; i++ )
   {
      if ( i != gaussian_pos )
      {
         new_gaussians.push_back( gaussians[ 0 + 3 * i ] );
         new_gaussians.push_back( gaussians[ 1 + 3 * i ] );
         new_gaussians.push_back( gaussians[ 2 + 3 * i ] );
      }
   }
   gaussians = new_gaussians;
   if ( gaussian_pos == ( unsigned int ) gaussians.size() / 3 )
   {
      gaussian_pos = ( unsigned int ) gaussians.size() / 3 - 1;
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
   vector < double > y = gaussian( gaussians[ 0 + gaussian_pos * 3 ],
                                   gaussians[ 1 + gaussian_pos * 3 ],
                                   gaussians[ 2 + gaussian_pos * 3 ] );

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

void US_Hydrodyn_Saxs_Hplc::gauss_add_gaussian( double height, double center, double width, QColor color )
{
   vector < double > x = f_qs[ wheel_file ];
   vector < double > y = gaussian( height, center, width );

#ifndef QT4
   long curve;
   curve = plot_dist->insertCurve( "gaussian" );
   plot_dist->setCurveStyle( curve, QwtCurve::Lines );
#else
   QwtPlotCurve *curve;
   QwtPlotCurve *curve = new QwtPlotCurve( file );
   curve->setStyle( QwtPlotCurve::Lines );
#endif

   plotted_gaussians.push_back( curve );

#ifndef QT4
   plot_dist->setCurveData( curve, 
                            (double *)&x[ 0 ],
                            (double *)&y[ 0 ],
                            x.size()
                            );
   plot_dist->setCurvePen( curve, QPen( color , 1, Qt::DashLine ) );
#else
   curve->setData(
                  (double *)&x[ 0 ],
                  (double *)&y[ 0 ],
                  x.size()
                  );

   curve->setPen( QPen( color, 1, Qt::DashLine ) );
   curve->attach( plot_dist );
#endif
}

void US_Hydrodyn_Saxs_Hplc::gauss_init_gaussians()
{
   gauss_delete_gaussians();
   plotted_gaussians.clear();
   plotted_gaussian_sum.clear();

   for ( unsigned int i = 0; i < ( unsigned int ) gaussians.size(); i += 3 )
   {
      gauss_add_gaussian( gaussians[ 0 + i ],
                          gaussians[ 1 + i ],
                          gaussians[ 2 + i ],
                          Qt::green );
   }

   plot_gaussian_sum();
   plot_dist->replot();
}

void US_Hydrodyn_Saxs_Hplc::gauss_delete_gaussians()
{
   for ( unsigned int i = 0; i < ( unsigned int ) plotted_gaussians.size(); i++ )
   {
#ifndef QT4
      plot_dist->removeCurve( plotted_gaussians[ i ] );
#else
#warn check how to do this in qt4
#endif
   }
   for ( unsigned int i = 0; i < ( unsigned int ) plotted_gaussian_sum.size(); i++ )
   {
#ifndef QT4
      plot_dist->removeCurve( plotted_gaussian_sum[ i ] );
#else
#warn check how to do this in qt4
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
#warn check how to do this in qt4
#endif
      }

      for ( unsigned int i = 1; i < plotted_gaussians.size(); i++ )
      {
         for ( unsigned int j = 0; j < x.size(); j++ )
         {
#ifndef QT4
            y[ j ] += plot_dist->curve( plotted_gaussians[ i ] )->y( j );
#else
#warn check how to do this in qt4
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
   QwtPlotCurve *curve;
   QwtPlotCurve *curve = new QwtPlotCurve( file );
   curve->setStyle( QwtPlotCurve::Lines );
#endif

   plotted_gaussian_sum.push_back( curve );

#ifndef QT4
   plot_dist->setCurveData( curve, 
                            (double *)&x[ 0 ],
                            (double *)&y[ 0 ],
                            x.size()
                            );
   plot_dist->setCurvePen( curve, QPen( Qt::yellow , 1, Qt::DashLine ) );
#else
   curve->setData(
                  (double *)&x[ 0 ],
                  (double *)&y[ 0 ],
                  x.size()
                  );

   curve->setPen( QPen( Qt::yellow, 1, Qt::DashLine ) );
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
#warn check how to do this in qt4
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
#warn check how to do this in qt4
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

static unsigned int lm_fit_gauss_size;

static double compute_gaussian_f( double t, const double *par )
{
   double result = 0e0;

   for ( unsigned int i = 0; i < lm_fit_gauss_size; i += 3 )
   {
      if ( par[ i ] <= 0.001 || par[ 2 + i ] <= 0.001 )
      {
         return 1e99;
      }
      double tmp = ( t - par[ 1 + i ] ) / par[ 2 + i ];
      result += par[ 0 + i ] * exp( - tmp * tmp / 2 );
   }

   return result;
}

void US_Hydrodyn_Saxs_Hplc::gauss_fit()
{
   check_fit_range();

   if ( ggaussian_mode )
   {
      disable_all();
      double peak;
      get_peak( wheel_file, peak );
      gauss_max_height = peak * 1.2;

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
}

void US_Hydrodyn_Saxs_Hplc::baseline_enables()
{
   pb_baseline_start   ->setEnabled( false );
   pb_wheel_cancel     ->setEnabled( true );
   pb_wheel_save       ->setEnabled( 
                                    le_baseline_start_s->text().toDouble() != org_baseline_start_s ||
                                    le_baseline_start  ->text().toDouble() != org_baseline_start   ||
                                    le_baseline_start_e->text().toDouble() != org_baseline_start_e ||
                                    le_baseline_end_s  ->text().toDouble() != org_baseline_end_s   ||
                                    le_baseline_end    ->text().toDouble() != org_baseline_end     ||   
                                    le_baseline_end_e  ->text().toDouble() != org_baseline_end_e
                                    );
   le_baseline_start_s ->setEnabled( true );
   le_baseline_start   ->setEnabled( true );
   le_baseline_start_e ->setEnabled( true );
   le_baseline_end_s   ->setEnabled( true );
   le_baseline_end     ->setEnabled( true );
   le_baseline_end_e   ->setEnabled( true );
   qwtw_wheel          ->setEnabled( 
                                    le_baseline_start_s->hasFocus() || 
                                    le_baseline_start  ->hasFocus() || 
                                    le_baseline_start_e->hasFocus() || 
                                    le_baseline_end_s  ->hasFocus() ||
                                    le_baseline_end    ->hasFocus() ||
                                    le_baseline_end_e  ->hasFocus()
                                    );
   pb_rescale          ->setEnabled( true );
   pb_view             ->setEnabled( true );
}

void US_Hydrodyn_Saxs_Hplc::baseline_start()
{
   org_baseline_start_s = le_baseline_start_s->text().toDouble();
   org_baseline_start   = le_baseline_start  ->text().toDouble();
   org_baseline_start_e = le_baseline_start_e->text().toDouble();
   org_baseline_end_s   = le_baseline_end_s  ->text().toDouble();
   org_baseline_end     = le_baseline_end    ->text().toDouble();
   org_baseline_end_e   = le_baseline_end_e  ->text().toDouble();

   le_last_focus = (mQLineEdit *) 0;
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

   if ( f_qs[ wheel_file ].size() < 2 )
   {
      editor_msg( "red", QString( tr( "Internal error: %1 almost empty data" ) ).arg( wheel_file ) );
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

#ifndef QT4
   plot_dist->setCurvePen( plotted_curves[ wheel_file ], QPen( Qt::cyan, 1, SolidLine));
#else
   plotted_curves[ wheel_file ]->setPen( QPen( plot_colors[ Qt::cyan, 1, Qt::SolidLine ) ) );
#endif

   baseline_mode = true;
   running       = true;
   qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], 
                         f_qs[ wheel_file ].back(), 
                         ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );

   double q_len       = f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ];
   double q_len_delta = q_len * 0.05;

   if ( le_baseline_start_s->text().isEmpty() ||
        le_baseline_start_s->text() == "0" ||
        le_baseline_start_s->text().toDouble() < f_qs[ wheel_file ][ 0 ] )
   {
      disconnect( le_baseline_start_s, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_baseline_start_s->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ 0 ] ) );
      connect( le_baseline_start_s, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_start_s_text( const QString & ) ) );
   }

   if ( le_baseline_start->text().isEmpty() ||
        le_baseline_start->text() == "0" ||
        le_baseline_start->text().toDouble() < f_qs[ wheel_file ][ 0 ] )
   {
      disconnect( le_baseline_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_baseline_start->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ 0 ] /* + q_len_delta */ ) );
      connect( le_baseline_start, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_start_text( const QString & ) ) );
   }

   if ( le_baseline_start_e->text().isEmpty() ||
        le_baseline_start_e->text() == "0" ||
        le_baseline_start_e->text().toDouble() < f_qs[ wheel_file ][ 0 ] )
   {
      disconnect( le_baseline_start_e, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_baseline_start_e->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ 0 ] + 2e0 * q_len_delta) );
      connect( le_baseline_start_e, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_start_e_text( const QString & ) ) );
   }

   if ( le_baseline_end_s->text().isEmpty() ||
        le_baseline_end_s->text() == "0" ||
        le_baseline_end_s->text().toDouble() > f_qs[ wheel_file ].back() )
   {
      disconnect( le_baseline_end_s, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_baseline_end_s->setText( QString( "%1" ).arg( f_qs[ wheel_file ].back() - 2e0 * q_len_delta ) );
      connect( le_baseline_end_s, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_s_text( const QString & ) ) );
   }

   if ( le_baseline_end->text().isEmpty() ||
        le_baseline_end->text() == "0" ||
        le_baseline_end->text().toDouble() > f_qs[ wheel_file ].back() )
   {
      disconnect( le_baseline_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_baseline_end->setText( QString( "%1" ).arg( f_qs[ wheel_file ].back() /* - q_len_delta */ ) );
      connect( le_baseline_end, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_text( const QString & ) ) );
   }

   if ( le_baseline_end_e->text().isEmpty() ||
        le_baseline_end_e->text() == "0" ||
        le_baseline_end_e->text().toDouble() > f_qs[ wheel_file ].back() )
   {
      disconnect( le_baseline_end_e, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_baseline_end_e->setText( QString( "%1" ).arg( f_qs[ wheel_file ].back() ) );
      connect( le_baseline_end_e, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_e_text( const QString & ) ) );
   }

   baseline_init_markers();
   replot_baseline();
   disable_all();
   baseline_enables();
}

void US_Hydrodyn_Saxs_Hplc::baseline_apply()
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
   baseline_apply( all_selected_files() );
}

void US_Hydrodyn_Saxs_Hplc::baseline_apply( QStringList files )
{
   map < QString, bool > current_files;
   for ( int i = 0; i < (int)lb_files->numRows(); i++ )
   {
      current_files[ lb_files->text( i ) ] = true;
   }

   map < QString, bool > select_files;

   double start_s = le_baseline_start_s->text().toDouble();
   double start   = le_baseline_start  ->text().toDouble();
   double start_e = le_baseline_start_e->text().toDouble();
   double end_s   = le_baseline_end_s  ->text().toDouble();
   double end     = le_baseline_end    ->text().toDouble();
   double end_e   = le_baseline_end_e  ->text().toDouble();

   // redo this to compute from best linear fit over ranges

   for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
   {

      unsigned int before_start = 0;
      unsigned int after_start  = 1;
      unsigned int before_end   = 0;
      unsigned int after_end    = 1;

      vector < double > start_q;
      vector < double > start_I;

      vector < double > end_q;
      vector < double > end_I;

      {
         unsigned int j = 0;
         if ( f_qs[ files[ i ] ][ j ] >= start_s &&
              f_qs[ files[ i ] ][ j ] <= start_e )
         {
            start_q.push_back( f_qs[ files[ i ] ][ j ] );
            start_I.push_back( f_Is[ files[ i ] ][ j ] );
         }
         if ( f_qs[ files[ i ] ][ j ] >= end_s &&
              f_qs[ files[ i ] ][ j ] <= end_e )
         {
            end_q.push_back( f_qs[ files[ i ] ][ j ] );
            end_I.push_back( f_Is[ files[ i ] ][ j ] );
         }
      }

      for ( unsigned int j = 1; j < f_qs[ files[ i ] ].size(); j++ )
      {
         if ( f_qs[ files[ i ] ][ j ] >= start_s &&
              f_qs[ files[ i ] ][ j ] <= start_e )
         {
            start_q.push_back( f_qs[ files[ i ] ][ j ] );
            start_I.push_back( f_Is[ files[ i ] ][ j ] );
         }
         if ( f_qs[ files[ i ] ][ j ] >= end_s &&
              f_qs[ files[ i ] ][ j ] <= end_e )
         {
            end_q.push_back( f_qs[ files[ i ] ][ j ] );
            end_I.push_back( f_Is[ files[ i ] ][ j ] );
         }

         if ( f_qs[ files[ i ] ][ j - 1 ] <= start &&
              f_qs[ files[ i ] ][ j     ] >= start )
         {
            before_start = j - 1;
            after_start  = j;
         }
         if ( f_qs[ files[ i ] ][ j - 1 ] <= end &&
              f_qs[ files[ i ] ][ j     ] >= end )
         {
            before_end = j - 1;
            after_end  = j;
         }
      }

      bool   set_start = ( start_q.size() > 1 );
      bool   set_end   = ( end_q  .size() > 1 );
      double start_intercept = 0e0;
      double start_slope     = 0e0;
      double end_intercept   = 0e0;
      double end_slope       = 0e0;

      double start_y;
      double end_y;

      double siga;
      double sigb;
      double chi2;

      if ( set_start && set_end )
      {
         // linear fit on each
         usu->linear_fit( start_q, start_I, start_intercept, start_slope, siga, sigb, chi2 );
         usu->linear_fit( end_q  , end_I  , end_intercept  , end_slope  , siga, sigb, chi2 );

         // find intercepts for baseline

         start_y = start_intercept + start_slope * start;
         end_y   = end_intercept   + end_slope   * end;
      } else {
         if ( set_start )
         {
            usu->linear_fit( start_q, start_I, start_intercept, start_slope, siga, sigb, chi2 );

            start_y = start_intercept + start_slope * start;

            double end_t;

            if ( f_qs[ files[ i ] ][ after_end  ] != f_qs[ files[ i ] ][ before_end ] )
            {
               end_t = ( f_qs[ files[ i ] ][ after_end ] - end )
                  / ( f_qs[ files[ i ] ][ after_end  ] -
                      f_qs[ files[ i ] ][ before_end ] );
            } else {
               end_t = 0.5e0;
            }

            end_y = 
               ( end_t ) * f_Is[ files[ i ] ][ before_end ] +
               ( 1e0 - end_t ) * f_Is[ files[ i ] ][ after_end ];
         } else {
            if ( set_end )
            {
               usu->linear_fit( end_q, end_I, end_intercept, end_slope, siga, sigb, chi2 );

               end_y = end_intercept + end_slope * end;

               double start_t;
               if ( f_qs[ files[ i ] ][ after_start  ] != f_qs[ files[ i ] ][ before_start ] )
               {
                  start_t = 
                     ( f_qs[ files[ i ] ][ after_start ] - start )
                     / ( f_qs[ files[ i ] ][ after_start  ] -
                         f_qs[ files[ i ] ][ before_start ] );
               } else {
                  start_t = 0.5e0;
               }

               start_y = 
                  ( start_t ) * f_Is[ files[ i ] ][ before_start ] +
                  ( 1e0 - start_t ) * f_Is[ files[ i ] ][ after_start ];
            
            } else {
               // for now, we are going to do this way for all conditions

               double start_t;
               double end_t;

               if ( f_qs[ files[ i ] ][ after_start  ] != f_qs[ files[ i ] ][ before_start ] )
               {
                  start_t = 
                     ( f_qs[ files[ i ] ][ after_start ] - start )
                     / ( f_qs[ files[ i ] ][ after_start  ] -
                         f_qs[ files[ i ] ][ before_start ] );
               } else {
                  start_t = 0.5e0;
               }
      
               if ( f_qs[ files[ i ] ][ after_end  ] != f_qs[ files[ i ] ][ before_end ] )
               {
                  end_t = ( f_qs[ files[ i ] ][ after_end ] - end )
                     / ( f_qs[ files[ i ] ][ after_end  ] -
                         f_qs[ files[ i ] ][ before_end ] );
               } else {
                  end_t = 0.5e0;
               }

               start_y = 
                  ( start_t ) * f_Is[ files[ i ] ][ before_start ] +
                  ( 1e0 - start_t ) * f_Is[ files[ i ] ][ after_start ];

               end_y = 
                  ( end_t ) * f_Is[ files[ i ] ][ before_end ] +
                  ( 1e0 - end_t ) * f_Is[ files[ i ] ][ after_end ];
            }
         }
      }

      baseline_slope     = ( end_y - start_y ) / ( end - start );
      baseline_intercept = 
         ( ( start_y + end_y ) -
           baseline_slope * ( start + end ) ) / 2e0;

      vector < double > bl_I = f_Is[ files[ i ] ];

      for ( unsigned int j = 0; j < bl_I.size(); j++ )
      {
         bl_I[ j ] -= baseline_slope * f_qs[ files[ i ] ][ j ] + baseline_intercept;
      }

      int ext = 0;
      QString bl_name = files[ i ] + QString( "-bl%1-%2s" ).arg( baseline_slope, 0, 'g', 8 ).arg( baseline_intercept, 0, 'g', 8 ).replace( ".", "_" );
      while ( current_files.count( bl_name ) )
      {
         bl_name = files[ i ] + QString( "-bl%1-%2s-%3" ).arg( baseline_slope ).arg( baseline_intercept ).arg( ++ext ).replace( ".", "_" );
      }

      select_files[ bl_name ] = true;

      lb_created_files->insertItem( bl_name );
      lb_created_files->setBottomItem( lb_created_files->numRows() - 1 );
      lb_files->insertItem( bl_name );
      lb_files->setBottomItem( lb_files->numRows() - 1 );
      created_files_not_saved[ bl_name ] = true;
   
      f_pos       [ bl_name ] = f_qs.size();
      f_qs_string [ bl_name ] = f_qs_string[ files[ i ] ];
      f_qs        [ bl_name ] = f_qs       [ files[ i ] ];
      f_Is        [ bl_name ] = bl_I;
      f_errors    [ bl_name ] = f_errors   [ files[ i ] ];
      f_is_time   [ bl_name ] = f_is_time  [ files[ i ] ];
      f_psv       [ bl_name ] = f_psv.count( files[ i ] ) ? f_psv[ files[ i ] ] : 0e0;
      f_conc      [ bl_name ] = f_conc.count( files[ i ] ) ? f_conc[ files[ i ] ] : 0e0;
      {
         vector < double > tmp;
         f_gaussians  [ bl_name ] = tmp;
      }
      editor_msg( "gray", QString( "Created %1\n" ).arg( bl_name ) );
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

static QColor start_color( 255, 165, 0 );
static QColor end_color  ( 255, 160, 122 );

void US_Hydrodyn_Saxs_Hplc::replot_baseline()
{
   // cout << "replot baseline\n";
   // compute & plot baseline
   // baseline_slope =
   // baseline_intercept =
   // plot over range from start to end
   // also find closest f_qs[ wheel_file ] to start/end
   unsigned int before_start = 0;
   unsigned int after_start  = 1;
   unsigned int before_end   = 0;
   unsigned int after_end    = 1;

   double start_s = le_baseline_start_s->text().toDouble();
   double start   = le_baseline_start  ->text().toDouble();
   double start_e = le_baseline_start_e->text().toDouble();
   double end_s   = le_baseline_end_s  ->text().toDouble();
   double end     = le_baseline_end    ->text().toDouble();
   double end_e   = le_baseline_end_e  ->text().toDouble();

   vector < double > start_q;
   vector < double > start_I;

   vector < double > end_q;
   vector < double > end_I;

   {
      unsigned int i = 0;
      if ( f_qs[ wheel_file ][ i ] >= start_s &&
           f_qs[ wheel_file ][ i ] <= start_e )
      {
         start_q.push_back( f_qs[ wheel_file ][ i ] );
         start_I.push_back( f_Is[ wheel_file ][ i ] );
      }
      if ( f_qs[ wheel_file ][ i ] >= end_s &&
           f_qs[ wheel_file ][ i ] <= end_e )
      {
         end_q.push_back( f_qs[ wheel_file ][ i ] );
         end_I.push_back( f_Is[ wheel_file ][ i ] );
      }
   }

   for ( unsigned int i = 1; i < f_qs[ wheel_file ].size(); i++ )
   {
      if ( f_qs[ wheel_file ][ i ] >= start_s &&
           f_qs[ wheel_file ][ i ] <= start_e )
      {
         start_q.push_back( f_qs[ wheel_file ][ i ] );
         start_I.push_back( f_Is[ wheel_file ][ i ] );
      }
      if ( f_qs[ wheel_file ][ i ] >= end_s &&
           f_qs[ wheel_file ][ i ] <= end_e )
      {
         end_q.push_back( f_qs[ wheel_file ][ i ] );
         end_I.push_back( f_Is[ wheel_file ][ i ] );
      }

      if ( f_qs[ wheel_file ][ i - 1 ] <= start &&
           f_qs[ wheel_file ][ i     ] >= start )
      {
         before_start = i - 1;
         after_start  = i;
      }
      if ( f_qs[ wheel_file ][ i - 1 ] <= end &&
           f_qs[ wheel_file ][ i     ] >= end )
      {
         before_end = i - 1;
         after_end  = i;
      }
   }

   bool   set_start = ( start_q.size() > 1 );
   bool   set_end   = ( end_q  .size() > 1 );
   double start_intercept = 0e0;
   double start_slope     = 0e0;
   double end_intercept   = 0e0;
   double end_slope       = 0e0;

   double start_y;
   double end_y;

   double siga;
   double sigb;
   double chi2;

   if ( set_start && set_end )
   {
      // linear fit on each
      usu->linear_fit( start_q, start_I, start_intercept, start_slope, siga, sigb, chi2 );
      usu->linear_fit( end_q  , end_I  , end_intercept  , end_slope  , siga, sigb, chi2 );

      // find intercepts for baseline

      start_y = start_intercept + start_slope * start;
      end_y   = end_intercept   + end_slope   * end;
   } else {
      if ( set_start )
      {
         usu->linear_fit( start_q, start_I, start_intercept, start_slope, siga, sigb, chi2 );

         start_y = start_intercept + start_slope * start;

         double end_t;

         if ( f_qs[ wheel_file ][ after_end  ] != f_qs[ wheel_file ][ before_end ] )
         {
            end_t = ( f_qs[ wheel_file ][ after_end ] - end )
               / ( f_qs[ wheel_file ][ after_end  ] -
                   f_qs[ wheel_file ][ before_end ] );
         } else {
            end_t = 0.5e0;
         }

         end_y = 
            ( end_t ) * f_Is[ wheel_file ][ before_end ] +
            ( 1e0 - end_t ) * f_Is[ wheel_file ][ after_end ];
      } else {
         if ( set_end )
         {
            usu->linear_fit( end_q, end_I, end_intercept, end_slope, siga, sigb, chi2 );

            end_y = end_intercept + end_slope * end;

            double start_t;
            if ( f_qs[ wheel_file ][ after_start  ] != f_qs[ wheel_file ][ before_start ] )
            {
               start_t = 
                  ( f_qs[ wheel_file ][ after_start ] - start )
                  / ( f_qs[ wheel_file ][ after_start  ] -
                      f_qs[ wheel_file ][ before_start ] );
            } else {
               start_t = 0.5e0;
            }

            start_y = 
               ( start_t ) * f_Is[ wheel_file ][ before_start ] +
               ( 1e0 - start_t ) * f_Is[ wheel_file ][ after_start ];
            
         } else {
            // for now, we are going to do this way for all conditions

            double start_t;
            double end_t;

            if ( f_qs[ wheel_file ][ after_start  ] != f_qs[ wheel_file ][ before_start ] )
            {
               start_t = 
                  ( f_qs[ wheel_file ][ after_start ] - start )
                  / ( f_qs[ wheel_file ][ after_start  ] -
                      f_qs[ wheel_file ][ before_start ] );
            } else {
               start_t = 0.5e0;
            }
      
            if ( f_qs[ wheel_file ][ after_end  ] != f_qs[ wheel_file ][ before_end ] )
            {
               end_t = ( f_qs[ wheel_file ][ after_end ] - end )
                  / ( f_qs[ wheel_file ][ after_end  ] -
                      f_qs[ wheel_file ][ before_end ] );
            } else {
               end_t = 0.5e0;
            }

            start_y = 
               ( start_t ) * f_Is[ wheel_file ][ before_start ] +
               ( 1e0 - start_t ) * f_Is[ wheel_file ][ after_start ];

            end_y = 
               ( end_t ) * f_Is[ wheel_file ][ before_end ] +
               ( 1e0 - end_t ) * f_Is[ wheel_file ][ after_end ];
         }
      }
   }

   baseline_slope     = ( end_y - start_y ) / ( end - start );
   baseline_intercept = 
      ( ( start_y + end_y ) -
        baseline_slope * ( start + end ) ) / 2e0;

   vector < double > x( 2 );
   vector < double > y( 2 );

   x[ 0 ] = f_qs[ wheel_file ][ 0 ];
   x[ 1 ] = f_qs[ wheel_file ].back();

   y[ 0 ] = baseline_slope * x[ 0 ] + baseline_intercept;
   y[ 1 ] = baseline_slope * x[ 1 ] + baseline_intercept;

   // remove any baseline curves

   for ( unsigned int i = 0; i < ( unsigned int ) plotted_baseline.size(); i++ )
   {
#ifndef QT4
      plot_dist->removeCurve( plotted_baseline[ i ] );
#else
#warn check how to do this in qt4
#endif
   }
   plotted_baseline.clear();

   // the baseline
   {
      // cout << QString( "baseline slope %1 intercept %2\n" ).arg( baseline_slope ).arg( baseline_intercept );
      // printvector( "baseline x", x );
      // printvector( "baseline y", y );

#ifndef QT4
      long curve;
      curve = plot_dist->insertCurve( "baseline" );
      plot_dist->setCurveStyle( curve, QwtCurve::Lines );
#else
      QwtPlotCurve *curve;
      QwtPlotCurve *curve = new QwtPlotCurve( file );
      curve->setStyle( QwtPlotCurve::Lines );
#endif

      plotted_baseline.push_back( curve );

#ifndef QT4
      plot_dist->setCurvePen( curve, QPen( Qt::green , 1, Qt::DashLine ) );
      plot_dist->setCurveData( plotted_baseline[ 0 ],
                               (double *)&x[ 0 ],
                               (double *)&y[ 0 ],
                               2
                               );
#else
      curve->setPen( QPen( Qt:green, 1, Qt::DashLine ) );
      plotted_baseline[ 0 ]->setData(
                                     (double *)&x[ 0 ],
                                     (double *)&y[ 0 ],
                                     2
                                     );
      curve->attach( plot_dist );
#endif
   }
   if ( set_start )
   {
      y[ 0 ] = start_slope * f_qs[ wheel_file ][ 0 ] + start_intercept;
      y[ 1 ] = start_slope * f_qs[ wheel_file ].back() + start_intercept;

      // cout << QString( "start slope %1 intercept %2\n" ).arg( start_slope ).arg( start_intercept );
      // printvector( "start x", x );
      // printvector( "start y", y );

#ifndef QT4
      long curve;
      curve = plot_dist->insertCurve( "baseline s" );
      plot_dist->setCurveStyle( curve, QwtCurve::Lines );
#else
      QwtPlotCurve *curve;
      QwtPlotCurve *curve = new QwtPlotCurve( file );
      curve->setStyle( QwtPlotCurve::Lines );
#endif

      plotted_baseline.push_back( curve );

#ifndef QT4
      plot_dist->setCurvePen( curve, QPen( start_color, 1, Qt::DashLine ) );
      plot_dist->setCurveData( plotted_baseline.back(),
                               (double *)&x[ 0 ],
                               (double *)&y[ 0 ],
                               2
                               );
#else
      curve->setPen( QPen( QColor( start_color, 1, Qt::DashLine ) );
      plotted_baseline.back()->setData(
                                       (double *)&x[ 0 ],
                                       (double *)&y[ 0 ],
                                       2
                                     );
      curve->attach( plot_dist );
#endif
   }

   if ( set_end )
   {
      y[ 0 ] = end_slope * f_qs[ wheel_file ][ 0 ] + end_intercept;
      y[ 1 ] = end_slope * f_qs[ wheel_file ].back() + end_intercept;

      // cout << QString( "end slope %1 intercept %2\n" ).arg( end_slope ).arg( end_intercept );
      // printvector( "end x", x );
      // printvector( "end y", y );

#ifndef QT4
      long curve;
      curve = plot_dist->insertCurve( "baseline e" );
      plot_dist->setCurveStyle( curve, QwtCurve::Lines );
#else
      QwtPlotCurve *curve;
      QwtPlotCurve *curve = new QwtPlotCurve( file );
      curve->setStyle( QwtPlotCurve::Lines );
#endif

      plotted_baseline.push_back( curve );

#ifndef QT4
      plot_dist->setCurvePen( curve, QPen( end_color, 1, Qt::DashLine ) );
      plot_dist->setCurveData( plotted_baseline.back(),
                               (double *)&x[ 0 ],
                               (double *)&y[ 0 ],
                               2
                               );
#else
      curve->setPen( QPen( end_color, 1, Qt::DashLine ) );
      plotted_baseline.back()->setData(
                                       (double *)&x[ 0 ],
                                       (double *)&y[ 0 ],
                                       2
                                     );
      curve->attach( plot_dist );
#endif
   }

   plot_dist->replot();
}

void US_Hydrodyn_Saxs_Hplc::baseline_start_s_text( const QString & text )
{
#ifndef QT4
   plot_dist->setMarkerPos( plotted_markers[ 0 ], text.toDouble(), 0e0 );
#else
#warn check how to do this in qt4 needs ymark
   plotted_markers[ 0 ]->setValue( pos, ymark );
#endif
   if ( qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
   replot_baseline();
   plot_dist->replot();
   baseline_enables();
}

void US_Hydrodyn_Saxs_Hplc::baseline_start_text( const QString & text )
{
#ifndef QT4
   plot_dist->setMarkerPos( plotted_markers[ 1 ], text.toDouble(), 0e0 );
#else
#warn check how to do this in qt4 needs ymark
   plotted_markers[ 1 ]->setValue( pos, ymark );
#endif
   if ( qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
   replot_baseline();
   plot_dist->replot();
   baseline_enables();
}

void US_Hydrodyn_Saxs_Hplc::baseline_start_e_text( const QString & text )
{
#ifndef QT4
   plot_dist->setMarkerPos( plotted_markers[ 2 ], text.toDouble(), 0e0 );
#else
#warn check how to do this in qt4 needs ymark
   plotted_markers[ 2 ]->setValue( pos, ymark );
#endif
   if ( qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
   replot_baseline();
   plot_dist->replot();
   baseline_enables();
}

void US_Hydrodyn_Saxs_Hplc::baseline_end_s_text( const QString & text )
{
#ifndef QT4
   plot_dist->setMarkerPos( plotted_markers[ 3 ], text.toDouble(), 0e0 );
#else
#warn check how to do this in qt4 needs ymark
   plotted_markers[ 3 ]->setValue( pos, ymark );
#endif
   if ( qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
   replot_baseline();
   plot_dist->replot();
   baseline_enables();
}

void US_Hydrodyn_Saxs_Hplc::baseline_end_text( const QString & text )
{
#ifndef QT4
   plot_dist->setMarkerPos( plotted_markers[ 4 ], text.toDouble(), 0e0 );
#else
#warn check how to do this in qt4 needs ymark
   plotted_markers[ 4 ]->setValue( pos, ymark );
#endif
   if ( qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
   replot_baseline();
   plot_dist->replot();
   baseline_enables();
}

void US_Hydrodyn_Saxs_Hplc::baseline_end_e_text( const QString & text )
{
#ifndef QT4
   plot_dist->setMarkerPos( plotted_markers[ 5 ], text.toDouble(), 0e0 );
#else
#warn check how to do this in qt4 needs ymark
   plotted_markers[ 5 ]->setValue( pos, ymark );
#endif
   if ( qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
   replot_baseline();
   plot_dist->replot();
   baseline_enables();
}

void US_Hydrodyn_Saxs_Hplc::baseline_init_markers()
{
   gauss_delete_markers();

   plotted_markers.clear();
   plotted_baseline.clear();

   gauss_add_marker( le_baseline_start_s->text().toDouble(), Qt::magenta, tr( "\nLFS\nStart"   ) );
   gauss_add_marker( le_baseline_start  ->text().toDouble(), Qt::red,     tr( "Start"          ) );
   gauss_add_marker( le_baseline_start_e->text().toDouble(), Qt::magenta, tr( "\n\n\nLFS\nEnd" ) );
   gauss_add_marker( le_baseline_end_s  ->text().toDouble(), Qt::magenta, tr( "\nLFE\nStart"   ), Qt::AlignLeft | Qt::AlignTop );
   gauss_add_marker( le_baseline_end    ->text().toDouble(), Qt::red,     tr( "End"            ), Qt::AlignLeft | Qt::AlignTop );
   gauss_add_marker( le_baseline_end_e  ->text().toDouble(), Qt::magenta, tr( "\n\n\nLFE\nEnd" ), Qt::AlignLeft | Qt::AlignTop );

   plot_dist->replot();
}

void US_Hydrodyn_Saxs_Hplc::baseline_start_s_focus( bool hasFocus )
{
   cout << QString( "baseline_start_s_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], f_qs[ wheel_file ].back(), 
                            ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_baseline_start_s->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs_Hplc::baseline_start_focus( bool hasFocus )
{
   cout << QString( "baseline_start_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], f_qs[ wheel_file ].back(), 
                            ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_baseline_start->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs_Hplc::baseline_start_e_focus( bool hasFocus )
{
   cout << QString( "baseline_start_e_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], f_qs[ wheel_file ].back(), 
                            ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_baseline_start_e->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs_Hplc::baseline_end_s_focus( bool hasFocus )
{
   cout << QString( "baseline_end_s_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], 
                            f_qs[ wheel_file ].back(), 
                            ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_baseline_end_s->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs_Hplc::baseline_end_focus( bool hasFocus )
{
   cout << QString( "baseline_end_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], 
                            f_qs[ wheel_file ].back(), 
                            ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_baseline_end->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs_Hplc::baseline_end_e_focus( bool hasFocus )
{
   cout << QString( "baseline_end_e_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], 
                            f_qs[ wheel_file ].back(), 
                            ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_baseline_end_e->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs_Hplc::create_i_of_q()
{
   disable_all();

   QStringList files = all_selected_files();
   create_i_of_q( files );

   update_enables();
}

vector < double > US_Hydrodyn_Saxs_Hplc::compute_gaussian_sum( vector < double > t, vector < double > g )
{
   vector < double > result( t.size() );
   for ( unsigned int i = 0; i < ( unsigned int ) result.size(); i++ )
   {
      result[ i ] = 0e0;
   }

   for ( unsigned int i = 0; i < ( unsigned int ) t.size(); i++ )
   {
      for ( unsigned int j = 0; j < ( unsigned int ) g.size(); j += 3 )
      {
         double tmp = ( t[ i ] - g[ 1 + j ] ) / g[ 2 + j ];
         result[ i ] += g[ 0 + j ] * exp( - tmp * tmp / 2 );
      }
   }
   return result;
}


vector < double > US_Hydrodyn_Saxs_Hplc::compute_gaussian( vector < double > t, vector < double > g )
{
   vector < double > result( t.size() );

   for ( unsigned int i = 0; i < ( unsigned int ) t.size(); i++ )
   {
      double tmp = ( t[ i ] - g[ 1 ] ) / g[ 2 ];
      result[ i ] = g[ 0 ] * exp( - tmp * tmp / 2 );
   }
   return result;
}

double US_Hydrodyn_Saxs_Hplc::compute_gaussian_peak( QString file, vector < double > g )
{
   cout << QString( "gaussian peak file %1\n" ).arg( file );
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

bool US_Hydrodyn_Saxs_Hplc::compute_f_gaussians( QString file, QWidget *hplc_fit_widget )
{
   // take current gaussians & compute for this curve
   // find peak of curve
   US_Hydrodyn_Saxs_Hplc_Fit *fit = (US_Hydrodyn_Saxs_Hplc_Fit *)hplc_fit_widget;

   double peak;
   if ( !get_peak( file, peak ) )
   {
      return false;
   }

   // printvector( "cfg: org_gauss", org_gaussians );
   
   double gmax = compute_gaussian_peak( file, org_gaussians );
   
   double scale = peak / gmax;   

   gauss_max_height = peak * 1.2;

   // printvector( "cfg: org_gauss 2", org_gaussians );
   gaussians = org_gaussians;
   for ( unsigned int i = 0; i < ( unsigned int ) gaussians.size(); i += 3 )
   {
      gaussians[ 0 + i ] *= scale;
   }

   // printvector( "cfg: gaussians", gaussians );

   double gmax2 = compute_gaussian_peak( file, gaussians );

   cout << QString( "cfg: %1 org_gaussian peak %2, curve peak %3, scaling %4 new gaussian peak %5\n" )
      .arg( file )
      .arg( gmax )
      .arg( peak )
      .arg( scale )
      .arg( gmax2 )
      ;

   // now setup and fit
   wheel_file = file;
   fit->gaussians_undo.clear();
   fit->gaussians_undo.push_back( gaussians );
   fit->le_epsilon->setText( QString( "%1" ).arg( peak / 1e6 < 0.001 ? peak / 1e6 : 0.001 ) );

   fit->cb_fix_center    ->setChecked( true );
   fit->cb_fix_width     ->setChecked( true );
   fit->cb_fix_amplitude ->setChecked( false );

   fit->lm();
   // printvector( "cfg: after fit gaussians", gaussians );

   if ( !cb_fix_width->isChecked() )
   {
      fit->cb_fix_width     ->setChecked( false );
      fit->lm();
      // printvector( "cfg: after fit2 gaussians", gaussians );
   }

   f_gaussians[ file ] = gaussians;
   gaussians = org_gaussians;
   return true;
}


QString US_Hydrodyn_Saxs_Hplc::pad_zeros( int val, int max )
{
   unsigned int len = QString( "%1" ).arg( max ).length();
   QString      s   = QString( "%1" ).arg( val );
   while ( s.length() < len )
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
      editor_msg( "dark red", 
                  cb_fix_width->isChecked() ?
                  tr( "NOTICE: Some files selected have Gaussians with varying centers or widths or a different number of Gaussians or centers or widths that do not match the last Gaussians, these will be reset to the last Gaussian settings" )
                  :
                  tr( "NOTICE: Some files selected have Gaussians with varying centers or a different number of Gaussians or centers that do not match the last Gaussians, these will be reset to the last Gaussian settings" ) 
                  );
   }

   if ( !create_unified_ggaussian_target() )
   {
      update_enables();
      return;
   }

   // add_ggaussian_curve( "unified_ggaussian_target", unified_ggaussian_I );
   // add_ggaussian_curve( "unified_ggaussian_sum",    compute_ggaussian_gaussian_sum() );

   ggaussian_mode = true;

   lbl_gauss_fit->setText( QString( "%1" ).arg( ggaussian_rmsd(), 0, 'g', 5 ) );
   wheel_file = unified_ggaussian_files[ 0 ];

   get_peak( wheel_file, gauss_max_height );
   gauss_max_height *= 1.2;
      
   org_f_gaussians = f_gaussians;

   running        = true;

   if ( gaussian_pos >= unified_ggaussian_gaussians_size )
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
   return files;
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
   

bool US_Hydrodyn_Saxs_Hplc::ggaussian_compatible( QStringList & files, bool check_against_global )
{
   // see if all the centers match
   if ( files.size() < 2 )
   {
      return true;
   }

   vector < double > centers;
   vector < double > widths;

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
            for ( unsigned int j = 0; j < f_gaussians[ files[ i ] ].size(); j+= 3 )
            {
               centers.push_back( f_gaussians[ files[ i ] ][ 1 + j ] );
               widths .push_back( f_gaussians[ files[ i ] ][ 2 + j ] );
            }
         } else {
            vector < double > tmp_centers;
            vector < double > tmp_widths;
            for ( unsigned int j = 0; j < f_gaussians[ files[ i ] ].size(); j+= 3 )
            {
               tmp_centers.push_back( f_gaussians[ files[ i ] ][ 1 + j ] );
               tmp_widths .push_back( f_gaussians[ files[ i ] ][ 2 + j ] );
            }
            if ( tmp_centers != centers ||
                 ( cb_fix_width->isChecked() && tmp_widths != widths ) )
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
      for ( unsigned int j = 0; j < gaussians.size(); j+= 3 )
      {
         tmp_centers.push_back( gaussians[ 1 + j ] );
         tmp_widths .push_back( gaussians[ 2 + j ] );
      }
      if ( tmp_centers != centers ||
           ( cb_fix_width->isChecked() && tmp_widths != widths ) )
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

vector < double > US_Hydrodyn_Saxs_Hplc::compute_ggaussian_gaussian_sum()
{
   vector < double > result;
   if ( !unified_ggaussian_ok )
   {
      editor_msg( "red", tr( "Internal error: gaussian rmsd called but unified gaussians not ok" ) );
      return result;
   }
   result.resize( unified_ggaussian_I.size() );

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
            result[ i ] += height * exp( - tmp * tmp / 2 );
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
            result[ i ] += height * exp( - tmp * tmp / 2 );
         }
      }
   }
   // printvector( "cggs:", result );

   return result;
}

double US_Hydrodyn_Saxs_Hplc::ggaussian_rmsd()
{
   if ( !unified_ggaussian_ok )
   {
      editor_msg( "red", tr( "Internal error: gaussian rmsd called but unified gaussians not ok" ) );
      return 1e99;
   }

   vector < double > result = compute_ggaussian_gaussian_sum();

   double rmsd = 0e0;

   if ( unified_ggaussian_use_errors && cb_sd_weight->isChecked() )
   {
      for ( unsigned int i = 0; i < ( unsigned int ) result.size(); i++ )
      {
         double tmp = ( result[ i ] - unified_ggaussian_I[ i ] ) / unified_ggaussian_e[ i ];
         rmsd += tmp * tmp;
      }
   } else {
      for ( unsigned int i = 0; i < ( unsigned int ) result.size(); i++ )
      {
         double tmp = result[ i ] - unified_ggaussian_I[ i ];
         rmsd += tmp * tmp;
      }
   }

   //    printvector( "rmsd, ugq", unified_ggaussian_q );

   update_plot_errors( unified_ggaussian_t, unified_ggaussian_I, result, unified_ggaussian_e );
   plot_errors_jump_markers();

   return sqrt( rmsd );
}

void US_Hydrodyn_Saxs_Hplc::ggauss_rmsd()
{
   if ( ggauss_recompute() )
   {
      lbl_gauss_fit->setText( QString( "%1" ).arg( ggaussian_rmsd(), 0, 'g', 5 ) );
   }
}

bool US_Hydrodyn_Saxs_Hplc::initial_ggaussian_fit( QStringList & files )
{

   wheel_file = files[ 0 ];

   US_Hydrodyn_Saxs_Hplc_Fit *hplc_fit_window = 
      new US_Hydrodyn_Saxs_Hplc_Fit(
                                    this,
                                    this );

   hplc_fit_window->update_hplc = false;
   

   for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
   {
      progress->setProgress( i, files.size() * 1.2 );
      qApp->processEvents();
      if ( !compute_f_gaussians( files[ i ], (QWidget *) hplc_fit_window ) )
      {
         return false;
      }
   }

   delete hplc_fit_window;
   return true;
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
   QString s = QFileDialog::getExistingDirectory(
                                                 lbl_dir->text(),
                                                 this,
                                                 "get existing directory",
                                                 tr( "Choose a new base directory" ),
                                                 true );
   if ( !s.isEmpty() )
   {
      QDir::setCurrent( s );
      lbl_dir->setText(  QDir::currentDirPath() );
      if ( *saxs_widget )
      {
         saxs_window->add_to_directory_history( s );
      }
   }
}

void US_Hydrodyn_Saxs_Hplc::created_dir_pressed()
{
   QString s = QFileDialog::getExistingDirectory(
                                                 lbl_dir->text(),
                                                 this,
                                                 "get existing directory",
                                                 tr( "Choose a new base directory for saving files" ),
                                                 true );
   if ( !s.isEmpty() )
   {
      lbl_created_dir->setText( s );
      if ( *saxs_widget )
      {
         saxs_window->add_to_directory_history( s );
      }
   }
}

void US_Hydrodyn_Saxs_Hplc::gauss_as_curves()
{
   if ( gaussian_mode )
   {
      for ( unsigned int i = 0; i < ( unsigned int ) gaussians.size(); i+= 3 )
      {
         vector < double > tmp_g(3);
         tmp_g[ 0 ] = gaussians[ 0 + i ];
         tmp_g[ 1 ] = gaussians[ 1 + i ];
         tmp_g[ 2 ] = gaussians[ 2 + i ];
         add_plot( wheel_file + QString( "_pk%1" ).arg( ( i / 3 ) + 1 ),
                   f_qs[ wheel_file ],
                   compute_gaussian( f_qs[ wheel_file ], tmp_g ),
                   true,
                   false );
      }
      add_plot( wheel_file + QString( "_pksum" ),
                f_qs[ wheel_file ],
                compute_gaussian_sum( f_qs[ wheel_file ], gaussians ),
                true,
                false );
   } else {
      // ggaussian mode
      for ( unsigned int i = 0; i < ( unsigned int ) unified_ggaussian_files.size(); i++ )
      {
         vector < double > g;
         if ( cb_fix_width->isChecked() )
         {
            unsigned int  index = 2 * unified_ggaussian_gaussians_size + i * unified_ggaussian_gaussians_size;

            for ( unsigned int j = 0; j < unified_ggaussian_gaussians_size; j++ )
            {
               g.push_back( unified_ggaussian_params[ index + j + 0 ] );
               g.push_back( unified_ggaussian_params[ 2 * j + 0 ] );
               g.push_back( unified_ggaussian_params[ 2 * j + 1 ] );
            }
            for ( unsigned int j = 0; j < ( unsigned int ) g.size(); j+= 3 )
            {
               vector < double > tmp_g(3);
               tmp_g[ 0 ] = g[ 0 + j ];
               tmp_g[ 1 ] = g[ 1 + j ];
               tmp_g[ 2 ] = g[ 2 + j ];
               add_plot( unified_ggaussian_files[ i ] + QString( "_pk%1" ).arg( ( j / 3 ) + 1 ),
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
            for ( unsigned int j = 0; j < ( unsigned int ) g.size(); j+= 3 )
            {
               vector < double > tmp_g(3);
               tmp_g[ 0 ] = g[ 0 + j ];
               tmp_g[ 1 ] = g[ 1 + j ];
               tmp_g[ 2 ] = g[ 2 + j ];
               add_plot( unified_ggaussian_files[ i ] + QString( "_pk%1" ).arg( ( j / 3 ) + 1 ),
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
      }
   }
}

void US_Hydrodyn_Saxs_Hplc::set_sd_weight()
{
   if ( gaussian_mode )
   {
      replot_gaussian_sum();
   } else {
      plot_errors->clear();
      lbl_gauss_fit ->setText( "?" );
      if ( ggaussian_mode )
      {
         pb_ggauss_rmsd->setEnabled( true );
         plot_errors      ->clear();
         plot_errors      ->replot();
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

bool US_Hydrodyn_Saxs_Hplc::check_fit_range()
{
   if ( !gaussians.size() )
   {
      return true;
   }

   double fit_start = le_gauss_fit_start->text().toDouble();
   double fit_end   = le_gauss_fit_end  ->text().toDouble();

   unsigned int count = 0;

   for ( unsigned int i = 0; i < ( unsigned int ) gaussians.size(); i += 3 )
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
                            .arg( gaussians.size() / 3 ) );
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

   if ( *saxs_widget )
   {
      if ( cb_lock_dir->isChecked() )
      {
         saxs_window->add_to_directory_history( lbl_dir->text() );
      }
      saxs_window->select_from_directory_history( use_dir, this );
      raise();
   }

   QString fn = QFileDialog::getSaveFileName( use_dir, "*.dat", this, this->caption() + tr( " Save State" ),
                                              tr( "Select a name to save the state" ) );
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
   if ( !f.open( IO_WriteOnly ) )
   {
      return;
   }

   QTextStream ts( &f );

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
            ts << "# __gaussians\n";
            ts << QString( "%1 %2\n" )
               .arg( le_gauss_fit_start->text() )
               .arg( le_gauss_fit_end  ->text() )
               ;

            for ( unsigned int i = 0; i < ( unsigned int ) stack_data[ j ].gaussians.size(); i += 3 )
            {
               ts << QString( "%1 %2 %3\n" )
                  .arg( stack_data[ j ].gaussians[ 0 + i ], 0, 'g', 10 )
                  .arg( stack_data[ j ].gaussians[ 1 + i ], 0, 'g', 10 )
                  .arg( stack_data[ j ].gaussians[ 2 + i ], 0, 'g', 10 );
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
               for ( unsigned int i = 0; i < ( unsigned int) it->second.size(); i += 3 )
               {
                  ts << 
                     QString( "%1 %2 %3\n" )
                     .arg( it->second[ i + 0 ], 0, 'g', 10 )
                     .arg( it->second[ i + 1 ], 0, 'g', 10 )
                     .arg( it->second[ i + 2 ], 0, 'g', 10 )
                     ;
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

      for ( unsigned int i = 0; i < ( unsigned int ) gaussians.size(); i += 3 )
      {
         ts << QString( "%1 %2 %3\n" )
            .arg( gaussians[ 0 + i ], 0, 'g', 10 )
            .arg( gaussians[ 1 + i ], 0, 'g', 10 )
            .arg( gaussians[ 2 + i ], 0, 'g', 10 );
      }
      ts << "# __end\n";
   }

   for ( map < QString, vector < double > >::iterator it = f_gaussians.begin();
         it != f_gaussians.end();
         it++ )
   {
      ts << "# __f_gaussians: " << it->first << endl;
      for ( unsigned int i = 0; i < ( unsigned int) it->second.size(); i += 3 )
      {
         ts << 
            QString( "%1 %2 %3\n" )
            .arg( it->second[ i + 0 ], 0, 'g', 10 )
            .arg( it->second[ i + 1 ], 0, 'g', 10 )
            .arg( it->second[ i + 2 ], 0, 'g', 10 )
            ;
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
