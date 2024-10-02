#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_saxs_buffer.h"
#include "../include/us_hydrodyn_saxs_buffer_nth.h"
#if QT_VERSION >= 0x040000
#include <qwt_scale_engine.h>
//Added by qt3to4:
#include <QBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
 //#include <Q3PopupMenu>
#include <QHBoxLayout>
#include <QTextStream>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QGridLayout>
#endif

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#define SLASH QDir::separator()
#define Q_VAL_TOL 5e-6

US_Hydrodyn_Saxs_Buffer::US_Hydrodyn_Saxs_Buffer(
                                               csv csv1,
                                               void *us_hydrodyn, 
                                               QWidget *p, 
                                               const char *
                                               ) : QFrame( p )
{
   this->csv1 = csv1;
   this->us_hydrodyn = us_hydrodyn;
   USglobal = new US_Config();
   title = us_tr( "US-SOMO: SAXS: Data Utility" );
   setPalette( PALET_FRAME );
   setWindowTitle( title );
   order_ascending = false;
   conc_widget     = false;
#if QT_VERSION >= 0x040000
   legend_vis      = false;
#endif

   QDir::setCurrent( ((US_Hydrodyn *)us_hydrodyn)->somo_dir + QDir::separator() + "saxs" );

   saxs_widget = &(((US_Hydrodyn *) us_hydrodyn)->saxs_plot_widget);
   saxs_window = ((US_Hydrodyn *) us_hydrodyn)->saxs_plot_window;
   ((US_Hydrodyn *) us_hydrodyn)->saxs_buffer_widget = true;
   plot_dist_zoomer = (ScrollZoomer *) 0;

   best_fitness = 1e99;
   disable_updates = false;
   setupGUI();
   running = false;
   axis_y_log = true;
   axis_x_log = false;
   join_adjust_lowq = true;

   update_enables();

   global_Xpos += 30;
   global_Ypos += 30;

   t_csv->horizontalHeader()->setMaximumHeight( 23 );
   unsigned int csv_height = 38;
   unsigned int csv_width = t_csv->columnWidth(0) + 49;
   for ( int i = 0; i < t_csv->rowCount(); i++ )
   {
      csv_height += t_csv->rowHeight(i);
   }
   for ( int i = 1; i < t_csv->columnCount(); i++ )
   {
      csv_width += t_csv->columnWidth(i);
   }
   if ( csv_height > 800 )
   {
      csv_height = 800;
   }
   if ( csv_width > 1100 )
   {
      csv_width = 1100;
   }

   t_csv->setMinimumHeight( csv_height );

   // cout << QString("csv size %1 %2\n").arg(csv_height).arg(csv_width);
#if defined(DOES_WORK)
   lb_files        ->setMaximumWidth( 3 * csv_width / 7 );
   lb_created_files->setMaximumWidth( 3 * csv_width / 7 );
   editor          ->setMaximumWidth( 3 * csv_width / 7 );
   plot_dist    ->setMinimumWidth( 2 * csv_width / 3 );
#endif
   //    lb_files        ->setMaximumWidth( csv_width / 3 );
   //    lb_created_files->setMaximumWidth( csv_width / 3 );
   //    editor          ->setMaximumWidth( csv_width / 3 );

   int percharwidth = 1 + ( 7 * ( USglobal->config_list.fontSize - 1 ) / 10 );

   {
      vector < QPushButton * > pbs;
      // pbs.push_back( pb_add_files );
      // pbs.push_back( pb_conc );
      // pbs.push_back( pb_clear_files );

      // pbs.push_back( pb_select_all );
      pbs.push_back( pb_similar_files );
      pbs.push_back( pb_regex_load );
      pbs.push_back( pb_invert );
      // pbs.push_back( pb_adjacent );
      pbs.push_back( pb_select_nth );
      pbs.push_back( pb_color_rotate );
      pbs.push_back( pb_to_saxs );
      pbs.push_back( pb_view );
      pbs.push_back( pb_rescale );
      pbs.push_back( pb_normalize );

      pbs.push_back( pb_legend );
      pbs.push_back( pb_axis_x );
      pbs.push_back( pb_axis_y );
      pbs.push_back( pb_wheel_cancel );
      pbs.push_back( pb_wheel_save );
      pbs.push_back( pb_wheel_save );
      pbs.push_back( pb_crop_undo );
      pbs.push_back( pb_join_start );
      pbs.push_back( pb_join_swap );
      pbs.push_back( pb_join_fit_scaling );
      pbs.push_back( pb_join_fit_linear );

      // pbs.push_back( pb_avg );
      // pbs.push_back( pb_conc_avg );
      // pbs.push_back( pb_select_all_created );
      // pbs.push_back( pb_adjacent_created );
      // pbs.push_back( pb_save_created_csv );
      // pbs.push_back( pb_save_created );
      // pbs.push_back( pb_show_created );
      // pbs.push_back( pb_show_only_created );
        
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
   pb_set_buffer->setMaximumWidth ( pb_select_all->width() + 10 );
   pb_set_empty ->setMaximumWidth ( pb_select_all->width() + 10 );
   pb_set_signal->setMaximumWidth ( pb_select_all->width() + 10 );
   plot_colors.clear( );

   QColor bgc = plot_dist->canvasBackground().color();

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
}

US_Hydrodyn_Saxs_Buffer::~US_Hydrodyn_Saxs_Buffer()
{
   ((US_Hydrodyn *)us_hydrodyn)->saxs_buffer_widget = false;
}

void US_Hydrodyn_Saxs_Buffer::setupGUI()
{
   int minHeight1 = 24;
   int minHeight3 = 25;

   // lbl_title = new QLabel(csv1.name.left(80), this);
   // lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   // lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   // lbl_title->setMinimumHeight(minHeight1);
   // lbl_title->setPalette( PALET_FRAME );
   // lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_files = new QLabel("Data files", this);
   lbl_files->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_files->setMinimumHeight(minHeight1);
   lbl_files->setPalette( PALET_LABEL );
   AUTFBACK( lbl_files );
   lbl_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cb_lock_dir = new QCheckBox(this);
   cb_lock_dir->setText(us_tr("Lock "));
   cb_lock_dir->setEnabled( true );
   cb_lock_dir->setChecked( false );
   cb_lock_dir->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ) );
   cb_lock_dir->setPalette( PALET_NORMAL );
   AUTFBACK( cb_lock_dir );

   lbl_dir = new mQLabel( QDir::currentPath(), this );
   lbl_dir->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_dir->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_dir );
   lbl_dir->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2));
   connect( lbl_dir, SIGNAL(pressed()), SLOT( dir_pressed() ));

   pb_add_files = new QPushButton(us_tr("Add files"), this);
   pb_add_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_add_files->setMinimumHeight(minHeight3);
   pb_add_files->setPalette( PALET_PUSHB );
   connect(pb_add_files, SIGNAL(clicked()), SLOT(add_files()));

   pb_similar_files = new QPushButton(us_tr("Similar"), this);
   pb_similar_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_similar_files->setMinimumHeight(minHeight3);
   pb_similar_files->setPalette( PALET_PUSHB );
   connect(pb_similar_files, SIGNAL(clicked()), SLOT(similar_files()));

   pb_conc = new QPushButton(us_tr("Concentrations"), this);
   pb_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_conc->setMinimumHeight(minHeight3);
   pb_conc->setPalette( PALET_PUSHB );
   connect(pb_conc, SIGNAL(clicked()), SLOT(conc()));

   pb_clear_files = new QPushButton(us_tr("Remove files"), this);
   pb_clear_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_clear_files->setMinimumHeight(minHeight3);
   pb_clear_files->setPalette( PALET_PUSHB );
   connect(pb_clear_files, SIGNAL(clicked()), SLOT(clear_files()));

   pb_regex_load = new QPushButton(us_tr("RL"), this);
   pb_regex_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_regex_load->setMinimumHeight(minHeight3);
   pb_regex_load->setPalette( PALET_PUSHB );
   connect(pb_regex_load, SIGNAL(clicked()), SLOT(regex_load()));

   le_regex = new QLineEdit( this );    le_regex->setObjectName( "le_regex Line Edit" );
   le_regex->setText( "" );
   le_regex->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_regex->setPalette( PALET_NORMAL );
   AUTFBACK( le_regex );
   le_regex->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_regex_args = new QLineEdit( this );    le_regex_args->setObjectName( "le_regex_args Line Edit" );
   le_regex_args->setText( "" );
   le_regex_args->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_regex_args->setPalette( PALET_NORMAL );
   AUTFBACK( le_regex_args );
   le_regex_args->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   lb_files = new QListWidget( this );
   lb_files->setPalette( PALET_NORMAL );
   AUTFBACK( lb_files );
   lb_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lb_files->setEnabled(true);
   lb_files->setSelectionMode( QAbstractItemView::ExtendedSelection );
   lb_files->setMinimumHeight( minHeight1 * 8 );
   connect( lb_files, SIGNAL( itemSelectionChanged() ), SLOT( update_files() ) );

   lbl_selected = new QLabel("0 files selected", this );
   lbl_selected->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_selected->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_selected );
   lbl_selected->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2));

   pb_select_all = new QPushButton(us_tr("Select all"), this);
   pb_select_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_select_all->setMinimumHeight(minHeight1);
   pb_select_all->setPalette( PALET_PUSHB );
   connect(pb_select_all, SIGNAL(clicked()), SLOT(select_all()));

   pb_invert = new QPushButton(us_tr("Invert"), this);
   pb_invert->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_invert->setMinimumHeight(minHeight1);
   pb_invert->setPalette( PALET_PUSHB );
   connect(pb_invert, SIGNAL(clicked()), SLOT(invert()));

   pb_color_rotate = new QPushButton(us_tr("C"), this);
   pb_color_rotate->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_color_rotate->setMinimumHeight(minHeight1);
   pb_color_rotate->setMaximumWidth ( minHeight1 * 2 );
   pb_color_rotate->setPalette( PALET_PUSHB );
   connect(pb_color_rotate, SIGNAL(clicked()), SLOT(color_rotate()));

   //    pb_join = new QPushButton(us_tr("J"), this);
   //    pb_join->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   //    pb_join->setMinimumHeight( minHeight1 );
   //    pb_join->setMaximumWidth ( minHeight1 * 2 );
   //    pb_join->setPalette( PALET_PUSHB );
   //    connect(pb_join, SIGNAL(clicked()), SLOT(join()));

   pb_select_nth = new QPushButton(us_tr("Select"), this);
   pb_select_nth->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_select_nth->setMinimumHeight(minHeight1);
   pb_select_nth->setPalette( PALET_PUSHB );
   connect(pb_select_nth, SIGNAL(clicked()), SLOT(select_nth()));

   // pb_adjacent = new QPushButton(us_tr("Similar"), this);
   // pb_adjacent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   // pb_adjacent->setMinimumHeight(minHeight1);
   // pb_adjacent->setPalette( PALET_PUSHB );
   // connect(pb_adjacent, SIGNAL(clicked()), SLOT(adjacent()));

   pb_to_saxs = new QPushButton(us_tr("S"), this);
   pb_to_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_to_saxs->setMinimumHeight( minHeight1 );
   pb_to_saxs->setMaximumWidth ( minHeight1 * 2 );
   pb_to_saxs->setPalette( PALET_PUSHB );
   connect(pb_to_saxs, SIGNAL(clicked()), SLOT(to_saxs()));

   pb_view = new QPushButton(us_tr("View"), this);
   pb_view->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_view->setMinimumHeight( minHeight1 );
   pb_view->setMaximumWidth ( minHeight1 * 4 );
   pb_view->setPalette( PALET_PUSHB );
   connect(pb_view, SIGNAL(clicked()), SLOT( view() ));

   pb_rescale = new QPushButton(us_tr("Rescale"), this);
   pb_rescale->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_rescale->setMinimumHeight(minHeight1);
   pb_rescale->setPalette( PALET_PUSHB );
   connect(pb_rescale, SIGNAL(clicked()), SLOT(rescale()));

   // pb_plot_files = new QPushButton(us_tr("Plot"), this);
   // pb_plot_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   // pb_plot_files->setMinimumHeight(minHeight1);
   // pb_plot_files->setPalette( PALET_PUSHB );
   // connect(pb_plot_files, SIGNAL(clicked()), SLOT(plot_files()));

   pb_asum = new QPushButton(us_tr("WS"), this);
   pb_asum->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_asum->setMinimumHeight(minHeight1);
   pb_asum->setPalette( PALET_PUSHB );
   connect(pb_asum, SIGNAL(clicked()), SLOT(asum()));

   pb_avg = new QPushButton(us_tr("Average"), this);
   pb_avg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_avg->setMinimumHeight(minHeight1);
   pb_avg->setPalette( PALET_PUSHB );
   connect(pb_avg, SIGNAL(clicked()), SLOT(avg()));

   pb_normalize = new QPushButton(us_tr("N"), this);
   pb_normalize->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_normalize->setMinimumHeight(minHeight1);
   pb_normalize->setPalette( PALET_PUSHB );
   connect(pb_normalize, SIGNAL(clicked()), SLOT(normalize()));

   pb_conc_avg = new QPushButton(us_tr("Concentration normalized average"), this);
   pb_conc_avg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_conc_avg->setMinimumHeight(minHeight1);
   pb_conc_avg->setPalette( PALET_PUSHB );
   connect(pb_conc_avg, SIGNAL(clicked()), SLOT(conc_avg()));

   pb_set_buffer = new QPushButton(us_tr("Set buffer"), this);
   pb_set_buffer->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_set_buffer->setMinimumHeight(minHeight1);
   pb_set_buffer->setPalette( PALET_PUSHB );
   connect(pb_set_buffer, SIGNAL(clicked()), SLOT(set_buffer()));

   lbl_buffer = new QLabel("", this );
   lbl_buffer->setMinimumHeight(minHeight1);
   lbl_buffer->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_buffer->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_buffer );
   lbl_buffer->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   pb_set_empty = new QPushButton(us_tr("Set blank"), this);
   pb_set_empty->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_set_empty->setMinimumHeight(minHeight1);
   pb_set_empty->setPalette( PALET_PUSHB );
   connect(pb_set_empty, SIGNAL(clicked()), SLOT(set_empty()));

   lbl_empty = new QLabel("", this );
   lbl_empty->setMinimumHeight(minHeight1);
   lbl_empty->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_empty->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_empty );
   lbl_empty->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   pb_set_signal = new QPushButton(us_tr("Set solution"), this);
   pb_set_signal->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_set_signal->setMinimumHeight(minHeight1);
   pb_set_signal->setPalette( PALET_PUSHB );
   connect(pb_set_signal, SIGNAL(clicked()), SLOT(set_signal()));

   lbl_signal = new QLabel("", this );
   lbl_signal->setMinimumHeight(minHeight1);
   lbl_signal->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_signal->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_signal );
   lbl_signal->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   lbl_created_files = new QLabel("Produced Data", this);
   lbl_created_files->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_created_files->setMinimumHeight(minHeight1);
   lbl_created_files->setPalette( PALET_LABEL );
   AUTFBACK( lbl_created_files );
   lbl_created_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_created_dir = new mQLabel( QDir::currentPath(), this );
   lbl_created_dir->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_created_dir->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_created_dir );
   lbl_created_dir->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2));
   connect( lbl_created_dir, SIGNAL(pressed()), SLOT( created_dir_pressed() ));

   lb_created_files = new QListWidget( this );
   lb_created_files->setPalette( PALET_NORMAL );
   AUTFBACK( lb_created_files );
   lb_created_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lb_created_files->setEnabled(true);
   lb_created_files->setSelectionMode( QAbstractItemView::ExtendedSelection );
   lb_created_files->setMinimumHeight( minHeight1 * 3 );
   connect( lb_created_files, SIGNAL( itemSelectionChanged() ), SLOT( update_created_files() ) );
#if QT_VERSION < 0x040000
   connect( lb_created_files, 
            SIGNAL( rightButtonClicked( QListWidgetItem *, const QPoint & ) ),
            SLOT  ( rename_created    ( QListWidgetItem *, const QPoint & ) ) );
#else
   connect( lb_created_files, 
            SIGNAL( customContextMenuRequested( const QPoint & ) ),
            SLOT  ( rename_from_context ( const QPoint & ) )
            );
   lb_created_files->setContextMenuPolicy( Qt::CustomContextMenu );
#endif

   lbl_selected_created = new QLabel("0 files selected", this );
   lbl_selected_created->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_selected_created->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_selected_created );
   lbl_selected_created->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2));

   pb_select_all_created = new QPushButton(us_tr("Select all"), this);
   pb_select_all_created->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize  - 1));
   pb_select_all_created->setMinimumHeight(minHeight1);
   pb_select_all_created->setPalette( PALET_PUSHB );
   connect(pb_select_all_created, SIGNAL(clicked()), SLOT(select_all_created()));

   pb_adjacent_created = new QPushButton(us_tr("Similar"), this);
   pb_adjacent_created->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_adjacent_created->setMinimumHeight(minHeight1);
   pb_adjacent_created->setPalette( PALET_PUSHB );
   connect(pb_adjacent_created, SIGNAL(clicked()), SLOT(adjacent_created()));

   pb_save_created_csv = new QPushButton(us_tr("Save CSV"), this);
   pb_save_created_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_save_created_csv->setMinimumHeight(minHeight1);
   pb_save_created_csv->setPalette( PALET_PUSHB );
   connect(pb_save_created_csv, SIGNAL(clicked()), SLOT(save_created_csv()));

   pb_save_created = new QPushButton(us_tr("Save"), this);
   pb_save_created->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_save_created->setMinimumHeight(minHeight1);
   pb_save_created->setPalette( PALET_PUSHB );
   connect(pb_save_created, SIGNAL(clicked()), SLOT(save_created()));

   pb_show_created = new QPushButton(us_tr("Show"), this);
   pb_show_created->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_show_created->setMinimumHeight(minHeight1);
   pb_show_created->setPalette( PALET_PUSHB );
   connect(pb_show_created, SIGNAL(clicked()), SLOT(show_created()));

   pb_show_only_created = new QPushButton(us_tr("Show only"), this);
   pb_show_only_created->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_show_only_created->setMinimumHeight(minHeight1);
   pb_show_only_created->setPalette( PALET_PUSHB );
   connect(pb_show_only_created, SIGNAL(clicked()), SLOT(show_only_created()));

   progress = new QProgressBar( this );
   // progress->setMinimumHeight(minHeight1);
   progress->setPalette( PALET_NORMAL );
   AUTFBACK( progress );
   progress->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   progress->reset();

   pb_start = new QPushButton(us_tr("Buffer subtraction over range"), this);
   pb_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_start->setMinimumHeight(minHeight1);
   pb_start->setPalette( PALET_PUSHB );
   connect(pb_start, SIGNAL(clicked()), SLOT(start()));

   pb_run_current = new QPushButton(us_tr("Current value buffer subtraction"), this);
   pb_run_current->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_run_current->setMinimumHeight(minHeight1);
   pb_run_current->setPalette( PALET_PUSHB );
   connect(pb_run_current, SIGNAL(clicked()), SLOT(run_current()));

   pb_run_divide = new QPushButton(us_tr("Current value buffer divide"), this);
   pb_run_divide->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_run_divide->setMinimumHeight(minHeight1);
   pb_run_divide->setPalette( PALET_PUSHB );
   connect(pb_run_divide, SIGNAL(clicked()), SLOT(run_divide()));

   pb_run_best = new QPushButton(us_tr("Best buffer subtraction"), this);
   pb_run_best->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_run_best->setMinimumHeight(minHeight1);
   pb_run_best->setPalette( PALET_PUSHB );
   connect(pb_run_best, SIGNAL(clicked()), SLOT(run_best()));

   pb_stop = new QPushButton(us_tr("Stop"), this);
   pb_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stop->setMinimumHeight(minHeight1);
   pb_stop->setPalette( PALET_PUSHB );
   connect(pb_stop, SIGNAL(clicked()), SLOT(stop()));

   editor = new QTextEdit(this);
   editor->setPalette( PALET_NORMAL );
   AUTFBACK( editor );
   editor->setReadOnly(true);
   editor->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ));

#if QT_VERSION < 0x040000
# if QT_VERSION >= 0x040000 && defined(Q_OS_MAC)
   {
 //      Q3PopupMenu * file = new Q3PopupMenu;
      file->insertItem( us_tr("&Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
      file->insertItem( us_tr("&Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );
      file->insertItem( us_tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );

      QMenuBar *menu = new QMenuBar( this );
      AUTFBACK( menu );

      menu->insertItem(us_tr("&Messages"), file );
   }
# else
   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight3);

   m = new QMenuBar( frame );    m->setObjectName( "menu" );
   m->setMinimumHeight(minHeight1 - 5);
   m->setPalette( PALET_NORMAL );
   AUTFBACK( m );
 //   Q3PopupMenu * file = new Q3PopupMenu(editor);
   m->insertItem( us_tr("&File"), file );
   file->insertItem( us_tr("Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
   file->insertItem( us_tr("Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );
   file->insertItem( us_tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );
# endif
#else
# if defined(Q_OS_MAC)
   m = new QMenuBar( this );
   m->setObjectName( "menu" );
# else
   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight3);
   frame->setPalette( PALET_NORMAL );
   AUTFBACK( frame );

   m = new QMenuBar( frame );    m->setObjectName( "menu" );
# endif
   m->setMinimumHeight(minHeight1 - 5);
   m->setPalette( PALET_NORMAL );
   AUTFBACK( m );

   {
      QMenu * new_menu = m->addMenu( us_tr( "&File" ) );

      QAction *qa1 = new_menu->addAction( us_tr( "Font" ) );
      qa1->setShortcut( Qt::ALT+Qt::Key_F );
      connect( qa1, SIGNAL(triggered()), this, SLOT( update_font() ) );

      QAction *qa2 = new_menu->addAction( us_tr( "Save" ) );
      qa2->setShortcut( Qt::ALT+Qt::Key_S );
      connect( qa2, SIGNAL(triggered()), this, SLOT( save() ) );

      QAction *qa3 = new_menu->addAction( us_tr( "Clear Display" ) );
      qa3->setShortcut( Qt::ALT+Qt::Key_X );
      connect( qa3, SIGNAL(triggered()), this, SLOT( clear_display() ) );
   }
#endif

   editor->setWordWrapMode (QTextOption::WordWrap);
   editor->setMinimumHeight( minHeight1 * 3 );

//   plot_dist = new QwtPlot(this);
   usp_plot_dist = new US_Plot( plot_dist, "", "", "", this );
   connect( (QWidget *)plot_dist->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_dist( const QPoint & ) ) );
   ((QWidget *)plot_dist->titleLabel())->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_dist->axisWidget( QwtPlot::yLeft ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_dist( const QPoint & ) ) );
   ((QWidget *)plot_dist->axisWidget( QwtPlot::yLeft ))->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_dist->axisWidget( QwtPlot::xBottom ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_dist( const QPoint & ) ) );
   ((QWidget *)plot_dist->axisWidget( QwtPlot::xBottom ))->setContextMenuPolicy( Qt::CustomContextMenu );
#if QT_VERSION < 0x040000
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
   plot_dist->setPalette( PALET_NORMAL );
   AUTFBACK( plot_dist );
#if QT_VERSION < 0x040000
   plot_dist->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot_dist->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   grid_saxs->setMajorPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid_saxs->setMinorPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid_saxs->attach( plot_dist );
#endif
   plot_dist->setAxisTitle(QwtPlot::xBottom, /* cb_guinier->isChecked() ? us_tr("q^2 (1/Angstrom^2)") : */  us_tr("q (1/Angstrom)"));
   plot_dist->setAxisTitle(QwtPlot::yLeft, us_tr("I(q) (log scale)"));
#if QT_VERSION < 0x040000
   plot_dist->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   plot_dist->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_dist->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   plot_dist->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_dist->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   plot_dist->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_dist->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
//    plot_dist->setMargin(USglobal->config_list.margin);
   plot_dist->setTitle("");
#if QT_VERSION < 0x040000
   plot_dist->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::Logarithmic);
#else
   plot_dist->setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine(10));
#endif
   plot_dist->setCanvasBackground(USglobal->global_colors.plot);

#if QT_VERSION < 0x040000
   plot_dist->setAutoLegend( false );
   plot_dist->setLegendFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ) );
#else
   QwtLegend* legend_pd = new QwtLegend;
   legend_pd->setFrameStyle( QFrame::Box | QFrame::Sunken );
   plot_dist->insertLegend( legend_pd, QwtPlot::BottomLegend );
#endif
#if QT_VERSION < 0x040000
   connect( plot_dist->canvas(), SIGNAL( mouseReleased( const QMouseEvent & ) ), SLOT( plot_mouse( const QMouseEvent & ) ) );
#endif

   t_csv = new QTableWidget(csv1.data.size(), csv1.header.size(), this);
   t_csv->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   t_csv->setMinimumHeight(minHeight1 * ( 1 + csv1.data.size() ) );
   t_csv->setMaximumHeight(minHeight1 * ( 2 + csv1.data.size() ) );
   // t_csv->setMinimumWidth(minWidth1);
   t_csv->setPalette( PALET_EDIT );
   AUTFBACK( t_csv );
   t_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   t_csv->setEnabled(true);

   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      for ( unsigned int j = 0; j < csv1.data[i].size(); j++ )
      {
         if ( csv1.data[i][j] == "Y" || csv1.data[i][j] == "N" )
         {
            t_csv->setCellWidget( i, j, new QCheckBox() );
            ((QCheckBox *)(t_csv->cellWidget( i, j )))->setChecked( csv1.data[i][j] == "Y" );
         } else {
            t_csv->setItem( i, j, new QTableWidgetItem( csv1.data[i][j] ) );
         }
      }
   }

   for ( unsigned int i = 0; i < csv1.header.size(); i++ )
   {
      t_csv->setHorizontalHeaderItem(i, new QTableWidgetItem( csv1.header[i]));
   }

   t_csv->setSortingEnabled(false);
    t_csv->verticalHeader()->setSectionsMovable(false);
    t_csv->horizontalHeader()->setSectionsMovable(false);
   //  t_csv->setReadOnly(false);

   t_csv->setColumnWidth(0, 330);
   { for ( int i = 0; i < t_csv->rowCount(); ++i ) { t_csv->item( i, 0 )->setFlags( t_csv->item( i, 0 )->flags() ^ Qt::ItemIsEditable ); } };
   { for ( int i = 0; i < t_csv->rowCount(); ++i ) { t_csv->item( i, t_csv->columnCount() - 1 )->setFlags( t_csv->item( i, t_csv->columnCount() - 1 )->flags() ^ Qt::ItemIsEditable ); } };

   // probably I'm not understanding something, but these next two lines don't seem to do anything
   // t_csv->horizontalHeader()->adjustHeaderSize();
   t_csv->adjustSize();

   recompute_interval_from_points();

   connect(t_csv, SIGNAL(valueChanged(int, int)), SLOT(table_value(int, int )));

   lbl_wheel_pos = new QLabel( "0", this );
   lbl_wheel_pos->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_wheel_pos->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_wheel_pos );
   lbl_wheel_pos->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   qwtw_wheel = new QwtWheel( this );
   qwtw_wheel->setMass         ( 0.5 );
   // qwtw_wheel->setRange( -1000, 1000); qwtw_wheel->setSingleStep( 1 );
   qwtw_wheel->setMinimumHeight( minHeight1 );
   // qwtw_wheel->setTotalAngle( 3600.0 );
   qwtw_wheel->setEnabled      ( false );
   connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
#if QT_VERSION >= 0x050000
   connect( qwtw_wheel, SIGNAL( wheelPressed() ), SLOT( wheel_pressed() ) );
   connect( qwtw_wheel, SIGNAL( wheelReleased() ), SLOT( wheel_released() ) );
#endif

   pb_wheel_cancel = new QPushButton(us_tr("Cancel"), this);
   pb_wheel_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_wheel_cancel->setMinimumHeight(minHeight1);
   pb_wheel_cancel->setPalette( PALET_PUSHB );
   pb_wheel_cancel->setEnabled(false);
   connect(pb_wheel_cancel, SIGNAL(clicked()), SLOT(wheel_cancel()));

   pb_wheel_save = new QPushButton(us_tr("Keep"), this);
   pb_wheel_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_wheel_save->setMinimumHeight(minHeight1);
   pb_wheel_save->setPalette( PALET_PUSHB );
   pb_wheel_save->setEnabled(false);
   connect(pb_wheel_save, SIGNAL(clicked()), SLOT(wheel_save()));

   pb_join_start = new QPushButton(us_tr("Join"), this);
   pb_join_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_join_start->setMinimumHeight(minHeight1);
   pb_join_start->setPalette( PALET_PUSHB );
   connect(pb_join_start, SIGNAL(clicked()), SLOT(join_start()));

   pb_join_swap = new QPushButton( join_adjust_lowq ? us_tr("Scale high-q") : us_tr("Scale low-q"), this);
   pb_join_swap->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_join_swap->setMinimumHeight(minHeight1);
   pb_join_swap->setPalette( PALET_PUSHB );
   pb_join_swap->setEnabled(false);
   connect(pb_join_swap, SIGNAL(clicked()), SLOT(join_swap()));

   lbl_join_offset = new QLabel( us_tr( "Linear offset:" ), this );
   lbl_join_offset->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_join_offset->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_join_offset );
   lbl_join_offset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_join_offset = new mQLineEdit( this );    le_join_offset->setObjectName( "le_join_offset Line Edit" );
   le_join_offset->setText( "" );
   le_join_offset->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_join_offset->setPalette( PALET_NORMAL );
   AUTFBACK( le_join_offset );
   le_join_offset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_join_offset->setEnabled( false );
   le_join_offset->setValidator( new QDoubleValidator( le_join_offset ) );
   connect( le_join_offset, SIGNAL( textChanged( const QString & ) ), SLOT( join_offset_text( const QString & ) ) );
   connect( le_join_offset, SIGNAL( focussed ( bool ) )             , SLOT( join_offset_focus( bool ) ) );

   lbl_join_mult = new QLabel( us_tr( "Multiplier:" ), this );
   lbl_join_mult->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_join_mult->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_join_mult );
   lbl_join_mult->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_join_mult = new mQLineEdit( this );    le_join_mult->setObjectName( "le_join_mult Line Edit" );
   le_join_mult->setText( "" );
   le_join_mult->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_join_mult->setPalette( PALET_NORMAL );
   AUTFBACK( le_join_mult );
   le_join_mult->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_join_mult->setEnabled( false );
   le_join_mult->setValidator( new QDoubleValidator( le_join_mult ) );
   connect( le_join_mult, SIGNAL( textChanged( const QString & ) ), SLOT( join_mult_text( const QString & ) ) );
   connect( le_join_mult, SIGNAL( focussed ( bool ) )             , SLOT( join_mult_focus( bool ) ) );

   lbl_join_start = new QLabel( us_tr( "Start:" ), this );
   lbl_join_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_join_start->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_join_start );
   lbl_join_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_join_start = new mQLineEdit( this );    le_join_start->setObjectName( "le_join_start Line Edit" );
   le_join_start->setText( "" );
   le_join_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_join_start->setPalette( PALET_NORMAL );
   AUTFBACK( le_join_start );
   le_join_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_join_start->setEnabled( false );
   le_join_start->setValidator( new QDoubleValidator( le_join_start ) );
   connect( le_join_start, SIGNAL( textChanged( const QString & ) ), SLOT( join_start_text( const QString & ) ) );
   connect( le_join_start, SIGNAL( focussed ( bool ) )             , SLOT( join_start_focus( bool ) ) );

   lbl_join_point = new QLabel( us_tr( "Cut:" ), this );
   lbl_join_point->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_join_point->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_join_point );
   lbl_join_point->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_join_point = new mQLineEdit( this );    le_join_point->setObjectName( "le_join_point Line Edit" );
   le_join_point->setText( "" );
   le_join_point->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_join_point->setPalette( PALET_NORMAL );
   AUTFBACK( le_join_point );
   le_join_point->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_join_point->setEnabled( false );
   le_join_point->setValidator( new QDoubleValidator( le_join_point ) );
   connect( le_join_point, SIGNAL( textChanged( const QString & ) ), SLOT( join_point_text( const QString & ) ) );
   connect( le_join_point, SIGNAL( focussed ( bool ) )             , SLOT( join_point_focus( bool ) ) );

   lbl_join_end = new QLabel( us_tr( "End:" ), this );
   lbl_join_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_join_end->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_join_end );
   lbl_join_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_join_end = new mQLineEdit( this );    le_join_end->setObjectName( "le_join_end Line Edit" );
   le_join_end->setText( "" );
   le_join_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_join_end->setPalette( PALET_NORMAL );
   AUTFBACK( le_join_end );
   le_join_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_join_end->setEnabled( false );
   le_join_end->setValidator( new QDoubleValidator( le_join_end ) );
   connect( le_join_end, SIGNAL( textChanged( const QString & ) ), SLOT( join_end_text( const QString & ) ) );
   connect( le_join_end, SIGNAL( focussed ( bool ) )             , SLOT( join_end_focus( bool ) ) );

   pb_join_fit_scaling = new QPushButton(us_tr("Fit"), this);
   pb_join_fit_scaling->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_join_fit_scaling->setMinimumHeight(minHeight1);
   pb_join_fit_scaling->setPalette( PALET_PUSHB );
   pb_join_fit_scaling->setEnabled( false );
   connect(pb_join_fit_scaling, SIGNAL(clicked()), SLOT(join_fit_scaling()));

   pb_join_fit_linear = new QPushButton(us_tr("Linear"), this);
   pb_join_fit_linear->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_join_fit_linear->setMinimumHeight(minHeight1);
   pb_join_fit_linear->setPalette( PALET_PUSHB );
   pb_join_fit_linear->setEnabled( false );
   connect(pb_join_fit_linear, SIGNAL(clicked()), SLOT(join_fit_linear()));

   lbl_join_rmsd = new QLabel( "", this );
   lbl_join_rmsd->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_join_rmsd->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_join_rmsd );
   lbl_join_rmsd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   pb_select_vis = new QPushButton(us_tr("Select Visible"), this);
   pb_select_vis->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_select_vis->setMinimumHeight(minHeight1);
   pb_select_vis->setPalette( PALET_PUSHB );
   connect(pb_select_vis, SIGNAL(clicked()), SLOT(select_vis()));

   pb_remove_vis = new QPushButton(us_tr("Remove Visible"), this);
   pb_remove_vis->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_remove_vis->setMinimumHeight(minHeight1);
   pb_remove_vis->setPalette( PALET_PUSHB );
   connect(pb_remove_vis, SIGNAL(clicked()), SLOT(remove_vis()));

   pb_crop_common = new QPushButton(us_tr("Crop Common"), this);
   pb_crop_common->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_crop_common->setMinimumHeight(minHeight1);
   pb_crop_common->setPalette( PALET_PUSHB );
   connect(pb_crop_common, SIGNAL(clicked()), SLOT(crop_common()));

   pb_crop_vis = new QPushButton(us_tr("Crop Visible"), this);
   pb_crop_vis->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_crop_vis->setMinimumHeight(minHeight1);
   pb_crop_vis->setPalette( PALET_PUSHB );
   connect(pb_crop_vis, SIGNAL(clicked()), SLOT(crop_vis()));

   pb_crop_zero = new QPushButton(us_tr("Crop Zeros"), this);
   pb_crop_zero->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_crop_zero->setMinimumHeight(minHeight1);
   pb_crop_zero->setPalette( PALET_PUSHB );
   connect(pb_crop_zero, SIGNAL(clicked()), SLOT(crop_zero()));

   pb_crop_left = new QPushButton(us_tr("Crop Left"), this);
   pb_crop_left->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_crop_left->setMinimumHeight(minHeight1);
   pb_crop_left->setPalette( PALET_PUSHB );
   connect(pb_crop_left, SIGNAL(clicked()), SLOT(crop_left()));

   pb_crop_undo = new QPushButton(us_tr("Undo"), this);
   pb_crop_undo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_crop_undo->setMinimumHeight(minHeight1);
   pb_crop_undo->setPalette( PALET_PUSHB );
   connect(pb_crop_undo, SIGNAL(clicked()), SLOT(crop_undo()));

   pb_crop_right = new QPushButton(us_tr("Crop Right"), this);
   pb_crop_right->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_crop_right->setMinimumHeight(minHeight1);
   pb_crop_right->setPalette( PALET_PUSHB );
   connect(pb_crop_right, SIGNAL(clicked()), SLOT(crop_right()));

   pb_legend = new QPushButton(us_tr("Legend"), this);
   pb_legend->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_legend->setMinimumHeight(minHeight1);
   pb_legend->setPalette( PALET_PUSHB );
   connect(pb_legend, SIGNAL(clicked()), SLOT(legend()));

   pb_axis_x = new QPushButton(us_tr("X"), this);
   pb_axis_x->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_axis_x->setMinimumHeight(minHeight1);
   pb_axis_x->setPalette( PALET_PUSHB );
   connect(pb_axis_x, SIGNAL(clicked()), SLOT(axis_x()));

   pb_axis_y = new QPushButton(us_tr("Y"), this);
   pb_axis_y->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_axis_y->setMinimumHeight(minHeight1);
   pb_axis_y->setPalette( PALET_PUSHB );
   connect(pb_axis_y, SIGNAL(clicked()), SLOT(axis_y()));

   cb_guinier = new QCheckBox(this);
   cb_guinier->setText(us_tr(" Guinier"));
   cb_guinier->setChecked(false);
   cb_guinier->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   cb_guinier->setPalette( PALET_NORMAL );
   AUTFBACK( cb_guinier );
   connect(cb_guinier, SIGNAL(clicked()), SLOT(guinier()));

   lbl_guinier = new QLabel( "", this );
   lbl_guinier->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_guinier->setPalette( PALET_LABEL );
   AUTFBACK( lbl_guinier );
   lbl_guinier->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));

   lbl_np = new QLabel( "Buffer subtraction non-positive:    ", this );
   lbl_np->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_np->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_np );
   lbl_np->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));

   rb_np_crop = new QRadioButton( us_tr("Crop "), this);
   rb_np_crop->setEnabled(true);
   rb_np_crop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   rb_np_crop->setPalette( PALET_NORMAL );
   AUTFBACK( rb_np_crop );

   rb_np_min = new QRadioButton( us_tr("Set to minimum "), this);
   rb_np_min->setEnabled(true);
   rb_np_min->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   rb_np_min->setPalette( PALET_NORMAL );
   AUTFBACK( rb_np_min );

   rb_np_ignore = new QRadioButton( us_tr("Ignore (log of non-positive not defined)"), this);
   rb_np_ignore->setEnabled(true);
   rb_np_ignore->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   rb_np_ignore->setPalette( PALET_NORMAL );
   AUTFBACK( rb_np_ignore );

   rb_np_ask = new QRadioButton( us_tr("Ask (blocks mass processing) "), this);
   rb_np_ask->setEnabled(true);
   rb_np_ask->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   rb_np_ask->setPalette( PALET_NORMAL );
   AUTFBACK( rb_np_ask );

#if QT_VERSION < 0x040000
   bg_np = new QGroupBox(1, Qt::Horizontal, 0);
   bg_np->setRadioButtonExclusive(true);
   bg_np->insert(rb_np_crop);
   bg_np->insert(rb_np_min);
   bg_np->insert(rb_np_ignore);
   bg_np->insert(rb_np_ask);
#else
   bg_np = new QGroupBox();
   bg_np->setFlat( true );

   {
      QHBoxLayout * bl = new QHBoxLayout; bl->setContentsMargins( 0, 0, 0, 0 ); bl->setSpacing( 0 );
      bl->addWidget( rb_np_crop );
      bl->addWidget( rb_np_min );
      bl->addWidget( rb_np_ignore );
      bl->addWidget( rb_np_ask );
      bg_np->setLayout( bl );
   }
#endif
   rb_np_crop->setChecked( true );

   cb_multi_sub = new QCheckBox(this);
   cb_multi_sub->setText(us_tr(" Subtract buffer from every selected file (exepting set buffer and set blank) " ) );
   cb_multi_sub->setChecked(false);
   cb_multi_sub->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   cb_multi_sub->setPalette( PALET_NORMAL );
   AUTFBACK( cb_multi_sub );
   connect( cb_multi_sub, SIGNAL( clicked() ), SLOT( update_enables() ) );

   cb_multi_sub_avg = new QCheckBox(this);
   cb_multi_sub_avg->setText(us_tr(" Average" ) );
   cb_multi_sub_avg->setChecked(false);
   cb_multi_sub_avg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   cb_multi_sub_avg->setPalette( PALET_NORMAL );
   AUTFBACK( cb_multi_sub_avg );

   cb_multi_sub_conc_avg = new QCheckBox(this);
   cb_multi_sub_conc_avg->setText(us_tr(" Concentration normalized average" ) );
   cb_multi_sub_conc_avg->setChecked(false);
   cb_multi_sub_conc_avg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   cb_multi_sub_conc_avg->setPalette( PALET_NORMAL );
   AUTFBACK( cb_multi_sub_conc_avg );

   pb_help = new QPushButton(us_tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(us_tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // build layout
   QBoxLayout * hbl_file_buttons = new QHBoxLayout(); hbl_file_buttons->setContentsMargins( 0, 0, 0, 0 ); hbl_file_buttons->setSpacing( 0 );
   hbl_file_buttons->addWidget ( pb_add_files );
   hbl_file_buttons->addWidget ( pb_similar_files );
   hbl_file_buttons->addWidget ( pb_conc);
   hbl_file_buttons->addWidget ( pb_clear_files );

   QBoxLayout * hbl_file_buttons_1 = new QHBoxLayout(); hbl_file_buttons_1->setContentsMargins( 0, 0, 0, 0 ); hbl_file_buttons_1->setSpacing( 0 );
   hbl_file_buttons_1->addWidget ( pb_regex_load );
   hbl_file_buttons_1->addWidget ( le_regex );
   hbl_file_buttons_1->addWidget ( le_regex_args );

   QBoxLayout * hbl_file_buttons_2 = new QHBoxLayout(); hbl_file_buttons_2->setContentsMargins( 0, 0, 0, 0 ); hbl_file_buttons_2->setSpacing( 0 );
   hbl_file_buttons_2->addWidget ( pb_select_all );
   hbl_file_buttons_2->addWidget ( pb_invert );
   // hbl_file_buttons_2->addWidget ( pb_adjacent );
   hbl_file_buttons_2->addWidget ( pb_select_nth );
   hbl_file_buttons_2->addWidget ( pb_color_rotate );
   hbl_file_buttons_2->addWidget ( pb_to_saxs );
   hbl_file_buttons_2->addWidget ( pb_view );
   hbl_file_buttons_2->addWidget ( pb_axis_x );
   hbl_file_buttons_2->addWidget ( pb_axis_y );
   hbl_file_buttons_2->addWidget ( pb_rescale );

   QBoxLayout * hbl_file_buttons_3 = new QHBoxLayout(); hbl_file_buttons_3->setContentsMargins( 0, 0, 0, 0 ); hbl_file_buttons_3->setSpacing( 0 );
   hbl_file_buttons_3->addWidget ( pb_conc_avg );
   hbl_file_buttons_3->addWidget ( pb_normalize );
   hbl_file_buttons_3->addWidget ( pb_asum );
   hbl_file_buttons_3->addWidget ( pb_avg );

   QBoxLayout * hbl_buffer = new QHBoxLayout(); hbl_buffer->setContentsMargins( 0, 0, 0, 0 ); hbl_buffer->setSpacing( 0 );
   hbl_buffer->addWidget ( pb_set_buffer );
   hbl_buffer->addWidget ( lbl_buffer );

   QBoxLayout * hbl_empty = new QHBoxLayout(); hbl_empty->setContentsMargins( 0, 0, 0, 0 ); hbl_empty->setSpacing( 0 );
   hbl_empty->addWidget ( pb_set_empty );
   hbl_empty->addWidget ( lbl_empty );

   QBoxLayout * hbl_signal = new QHBoxLayout(); hbl_signal->setContentsMargins( 0, 0, 0, 0 ); hbl_signal->setSpacing( 0 );
   hbl_signal->addWidget ( pb_set_signal );
   hbl_signal->addWidget ( lbl_signal );

   QBoxLayout * hbl_created = new QHBoxLayout(); hbl_created->setContentsMargins( 0, 0, 0, 0 ); hbl_created->setSpacing( 0 );
   hbl_created->addWidget ( pb_select_all_created );
   hbl_created->addWidget ( pb_adjacent_created );
   hbl_created->addWidget ( pb_save_created_csv );
   hbl_created->addWidget ( pb_save_created );

   QBoxLayout * hbl_created_2 = new QHBoxLayout(); hbl_created_2->setContentsMargins( 0, 0, 0, 0 ); hbl_created_2->setSpacing( 0 );
   hbl_created_2->addWidget ( pb_show_created );
   hbl_created_2->addWidget ( pb_show_only_created );

   QBoxLayout * vbl_editor_group = new QVBoxLayout(0); vbl_editor_group->setContentsMargins( 0, 0, 0, 0 ); vbl_editor_group->setSpacing( 0 );
#if QT_VERSION < 0x040000 || !defined(Q_OS_MAC)
   vbl_editor_group->addWidget (frame);
#endif
   vbl_editor_group->addWidget (editor);

   QHBoxLayout * hbl_dir = new QHBoxLayout(); hbl_dir->setContentsMargins( 0, 0, 0, 0 ); hbl_dir->setSpacing( 0 );
   hbl_dir->addWidget( cb_lock_dir );
   hbl_dir->addWidget( lbl_dir );

   QBoxLayout * vbl_files = new QVBoxLayout( 0 ); vbl_files->setContentsMargins( 0, 0, 0, 0 ); vbl_files->setSpacing( 0 );
   vbl_files->addWidget( lbl_files );
   vbl_files->addLayout( hbl_dir );
   vbl_files->addLayout( hbl_file_buttons );
   vbl_files->addLayout( hbl_file_buttons_1 );
   //   vbl_files->addLayout( hbl_file_buttons_1b );
   vbl_files->addWidget( lb_files );
   vbl_files->addWidget( lbl_selected );
   vbl_files->addLayout( hbl_file_buttons_2 );
   vbl_files->addLayout( hbl_file_buttons_3 );
   vbl_files->addLayout( hbl_buffer );
   vbl_files->addLayout( hbl_empty );
   vbl_files->addLayout( hbl_signal );
   vbl_files->addWidget( lbl_created_files );
   vbl_files->addWidget( lbl_created_dir );
   vbl_files->addWidget( lb_created_files );
   vbl_files->addWidget( lbl_selected_created );
   vbl_files->addLayout( hbl_created );
   vbl_files->addLayout( hbl_created_2 );
   vbl_files->addLayout( vbl_editor_group );

   QGridLayout * gl_wheel = new QGridLayout(0); gl_wheel->setContentsMargins( 0, 0, 0, 0 ); gl_wheel->setSpacing( 0 );
   gl_wheel->addWidget( pb_join_start   , 0 , 0 , 1 + ( 0 ) - ( 0 ) , 1 + ( 0  ) - ( 0 ) );
   gl_wheel->addWidget( pb_join_swap    , 0 , 1 , 1 + ( 0 ) - ( 0 ) , 1 + ( 1  ) - ( 1 ) );
   gl_wheel->addWidget( lbl_wheel_pos   , 0 , 2 , 1 + ( 0 ) - ( 0 ) , 1 + ( 2  ) - ( 2 ) );
   gl_wheel->addWidget( qwtw_wheel      , 0 , 3 , 1 + ( 0 ) - ( 0 ) , 1 + ( 8  ) - ( 3 ) );
   gl_wheel->addWidget         ( pb_wheel_cancel, 0, 9 );
   gl_wheel->addWidget         ( pb_wheel_save  , 0, 10 );

   QHBoxLayout * hbl_join = new QHBoxLayout(); hbl_join->setContentsMargins( 0, 0, 0, 0 ); hbl_join->setSpacing( 0 );
   hbl_join->addWidget( lbl_join_offset );
   hbl_join->addWidget( le_join_offset );
   hbl_join->addWidget( lbl_join_mult );
   hbl_join->addWidget( le_join_mult );
   hbl_join->addWidget( lbl_join_start );
   hbl_join->addWidget( le_join_start );
   hbl_join->addWidget( lbl_join_point );
   hbl_join->addWidget( le_join_point );
   hbl_join->addWidget( lbl_join_end );
   hbl_join->addWidget( le_join_end );
   hbl_join->addWidget( pb_join_fit_scaling );
   hbl_join->addWidget( pb_join_fit_linear );
   hbl_join->addWidget( lbl_join_rmsd );

   QBoxLayout * hbl_plot_buttons = new QHBoxLayout(); hbl_plot_buttons->setContentsMargins( 0, 0, 0, 0 ); hbl_plot_buttons->setSpacing( 0 );
   hbl_plot_buttons->addWidget( pb_select_vis );
   hbl_plot_buttons->addWidget( pb_remove_vis );
   hbl_plot_buttons->addWidget( pb_crop_common );
   hbl_plot_buttons->addWidget( pb_crop_vis );
   hbl_plot_buttons->addWidget( pb_crop_zero );
   hbl_plot_buttons->addWidget( pb_crop_left );
   hbl_plot_buttons->addWidget( pb_crop_undo );
   hbl_plot_buttons->addWidget( pb_crop_right );
   hbl_plot_buttons->addWidget( pb_legend );

   QBoxLayout * hbl_plot_buttons_2 = new QHBoxLayout(); hbl_plot_buttons_2->setContentsMargins( 0, 0, 0, 0 ); hbl_plot_buttons_2->setSpacing( 0 );
   hbl_plot_buttons_2->addWidget( cb_guinier );
   hbl_plot_buttons_2->addWidget( lbl_guinier );

   QBoxLayout * vbl_plot_group = new QVBoxLayout(0); vbl_plot_group->setContentsMargins( 0, 0, 0, 0 ); vbl_plot_group->setSpacing( 0 );
   vbl_plot_group->addWidget ( plot_dist );
   vbl_plot_group->addLayout ( gl_wheel  );
   vbl_plot_group->addLayout ( hbl_join  );
   vbl_plot_group->addLayout ( hbl_plot_buttons );
   vbl_plot_group->addLayout ( hbl_plot_buttons_2 );

   // QBoxLayout * hbl_files_plot = new QHBoxLayout(); hbl_files_plot->setContentsMargins( 0, 0, 0, 0 ); hbl_files_plot->setSpacing( 0 );
   // hbl_files_plot->addLayout( vbl_files );
   // hbl_files_plot->addLayout( vbl_plot_group );

   QGridLayout * gl_files_plot = new QGridLayout( 0 ); gl_files_plot->setContentsMargins( 0, 0, 0, 0 ); gl_files_plot->setSpacing( 0 );
   gl_files_plot->addLayout( vbl_files     , 0, 0 );
   gl_files_plot->addLayout( vbl_plot_group, 0, 1 );
   gl_files_plot->setColumnStretch( 0, 0 );
   gl_files_plot->setColumnStretch( 1, 1 );

   QHBoxLayout * hbl_np = new QHBoxLayout(); hbl_np->setContentsMargins( 0, 0, 0, 0 ); hbl_np->setSpacing( 0 );
   hbl_np->addWidget ( lbl_np );
   hbl_np->addWidget ( rb_np_crop );
   hbl_np->addWidget ( rb_np_min );
   hbl_np->addWidget ( rb_np_ignore );
   hbl_np->addWidget ( rb_np_ask );

   QHBoxLayout * hbl_multi = new QHBoxLayout(); hbl_multi->setContentsMargins( 0, 0, 0, 0 ); hbl_multi->setSpacing( 0 );
   hbl_multi->addWidget ( cb_multi_sub );
   hbl_multi->addWidget ( cb_multi_sub_avg );
   hbl_multi->addWidget ( cb_multi_sub_conc_avg );

   QHBoxLayout * hbl_controls = new QHBoxLayout(); hbl_controls->setContentsMargins( 0, 0, 0, 0 ); hbl_controls->setSpacing( 0 );
   hbl_controls->addSpacing( 1 );
   hbl_controls->addWidget (pb_start);
   hbl_controls->addSpacing( 1 );
   hbl_controls->addWidget (pb_run_current);
   hbl_controls->addSpacing( 1 );
   hbl_controls->addWidget (pb_run_divide);
   hbl_controls->addSpacing( 1 );
   hbl_controls->addWidget (pb_run_best);
   hbl_controls->addSpacing( 1 );
   hbl_controls->addWidget (pb_stop);
   hbl_controls->addSpacing( 1 );

   QGridLayout * gl_bottom = new QGridLayout( 0 ); gl_bottom->setContentsMargins( 0, 0, 0, 0 ); gl_bottom->setSpacing( 0 );
   gl_bottom->addWidget( pb_help  , 0, 0 );
   gl_bottom->addWidget( progress , 0, 1 );
   gl_bottom->addWidget( pb_cancel, 0, 2 );
   
   QVBoxLayout * background = new QVBoxLayout(this); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );
   background->addSpacing( 1 );
   // background->addWidget ( lbl_title );
   // background->addSpacing( 1 );
   background->addLayout ( gl_files_plot );
   background->addWidget ( t_csv );
   background->addSpacing( 1 );
   background->addLayout ( hbl_np );
   background->addSpacing( 1 );
   background->addLayout ( hbl_multi );
   background->addSpacing( 1 );
   background->addLayout ( hbl_controls );
   background->addSpacing( 1 );
   background->addLayout ( gl_bottom );
   background->addSpacing( 1 );

   if ( !U_EXPT )
   {
      pb_asum->hide();
   }
}

void US_Hydrodyn_Saxs_Buffer::cancel()
{
   close();
}

void US_Hydrodyn_Saxs_Buffer::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo/somo_saxs_buffer.html");
}

void US_Hydrodyn_Saxs_Buffer::closeEvent(QCloseEvent *e)
{
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
                                    title,
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

   ((US_Hydrodyn *)us_hydrodyn)->saxs_buffer_widget = false;
   ((US_Hydrodyn *)us_hydrodyn)->last_saxs_buffer_csv = current_csv();
   if ( conc_widget )
   {
      conc_window->close();
   }

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Saxs_Buffer::table_value( int row, int col )
{
   cout << "table value\n";
   if ( col == 2 || col == 3 || col == 5 || col == 6 )
   {
      if (
          !t_csv->item( row, col )->text().isEmpty() &&
          t_csv->item( row, col )->text() != QString( "%1" ).arg(  t_csv->item( row, col )->text().toDouble() )
          )
      {
         t_csv->setItem( row, col , new QTableWidgetItem( QString( "%1" ).arg(  t_csv->item( row, col )->text().toDouble() ) ) );
      }
   }
   if ( col == 4 )
   {
      if (
          !t_csv->item( row, col )->text().isEmpty() &&
          t_csv->item( row, col )->text() != QString( "%1" ).arg(  t_csv->item( row, col )->text().toUInt() )
          )
      {
         t_csv->setItem( row, col , new QTableWidgetItem( QString( "%1" ).arg(  t_csv->item( row, col )->text().toUInt() ) ) );
      }
   }

   if ( col == 4 || col == 2 || col == 3 )
   {
      recompute_interval_from_points();
   }
   if ( col == 5 )
   {
      recompute_points_from_interval();
   }
   if ( col == 1 )
   {
      if ( ( row == 1 || row == 2 ) &&
           ((QCheckBox *)(t_csv->cellWidget( row, 1 )))->isChecked() &&
           ((QCheckBox *)(t_csv->cellWidget( 0, 1 )))->isChecked()  )
      {
         ((QCheckBox *)(t_csv->cellWidget( 0, 1 )))->setChecked( false );
      }
      if ( row == 0 && ((QCheckBox *)(t_csv->cellWidget( row, 1 )))->isChecked() )
      {
         if ( ((QCheckBox *)(t_csv->cellWidget( 1, 1 )))->isChecked() )
         {
            ((QCheckBox *)(t_csv->cellWidget( 1, 1 )))->setChecked( false );
         }
         if ( ((QCheckBox *)(t_csv->cellWidget( 2, 1 )))->isChecked() )
         {
            ((QCheckBox *)(t_csv->cellWidget( 2, 1 )))->setChecked( false );
         }
      }
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Buffer::clear_display()
{
   editor->clear( );
   editor->append("\n\n");
}

void US_Hydrodyn_Saxs_Buffer::update_font()
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

void US_Hydrodyn_Saxs_Buffer::save()
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

void US_Hydrodyn_Saxs_Buffer::start()
{
   if ( !validate() ||
        !any_to_run() )
   {
      return;
   }

   running = true;
   update_enables();

   map < unsigned int, double >       starts;
   map < unsigned int, double >       ends;
   map < unsigned int, unsigned int > points;
   map < unsigned int, double >       increments;
   map < unsigned int, unsigned int > offsets;
   map < unsigned int, unsigned int > next_offsets;

   unsigned int current_offset = 0;

   for ( unsigned int i = 0; i < (unsigned int)t_csv->rowCount(); i++ )
   {
      if ( ((QCheckBox *)(t_csv->cellWidget( i, 1 )))->isChecked() )
      {
         starts    [i]   =  t_csv->item(i, 2)->text().toDouble();
         ends      [i]   =  t_csv->item(i, 3)->text().toDouble();
         points    [i]   =  t_csv->item(i, 4)->text().toUInt();
         offsets   [i]   =  current_offset;
         current_offset +=  points[i];
         next_offsets[i] =  current_offset;

         if ( points[i] > 1 )
         {
            increments[i] = (ends[i] - starts[i]) / ( points[i] - 1 );
         } else {
            increments[i] = 0;
         }
      }
   }
   
   unsigned int total_points = 1;
   for ( map < unsigned int, unsigned int >::iterator it = points.begin();
         it != points.end();
         it++ )
   {
      total_points *= it->second;
   }

   editor_msg("black", QString( us_tr( "Total points %1\n").arg( total_points ) ) );

   // linearization of an arbitrary number of loops

   for ( unsigned int i = 0; i < total_points; i++ )
   {
      if ( !running )
      {
         break;
      }
      progress->setValue( i ); progress->setMaximum( total_points );
      unsigned int pos = i;
      QString msg = "";
      for ( map < unsigned int, unsigned int >::iterator it = points.begin();
            it != points.end();
            it++ )
      {
         {
            QString val = QString("%1").arg(starts[it->first] + ( pos % it->second ) * increments[it->first]);
            t_csv->setItem(it->first, 6, new QTableWidgetItem( val ) );
         }
         pos /= it->second;
         msg += QString("%1 %2; ").arg(t_csv->item(it->first, 0)->text()).arg(t_csv->item(it->first, 6)->text());
      }
      editor_msg( "black", us_tr( "Running:" ) + msg );
      run_current();
   }

   running = false;
   progress->setValue( 1 ); progress->setMaximum( 1 );
   update_enables();
}

void US_Hydrodyn_Saxs_Buffer::run_current()
{
   if ( cb_multi_sub->isChecked() )
   {
      bool is_running = running;
      if ( !is_running )
      {
         running = true;
         update_enables();
      }

      QString save_signal = lbl_signal->text();
      map < QString, bool > selected_non_buffer_non_empty;
      QStringList created_files;
      
      for ( int i = 0; i < lb_files->count(); i++ )
      {
         if ( lb_files->item( i )->isSelected() && 
              lb_files->item( i )->text() != lbl_buffer->text() &&
              lb_files->item( i )->text() != lbl_empty->text() )
         {
            selected_non_buffer_non_empty[ lb_files->item( i )->text() ] = true;
         }
      }
      unsigned int total_points = selected_non_buffer_non_empty.size() + 1;
      unsigned int pos = 1;
      for ( map < QString, bool >::iterator it = selected_non_buffer_non_empty.begin();
            it != selected_non_buffer_non_empty.end();
            it++ )
      {
         if ( !is_running )
         {
            progress->setValue( pos++ ); progress->setMaximum( total_points );
         }
         lbl_signal->setText( it->first );
         qApp->processEvents();
         run_one( false );
         if ( !last_created_file.isEmpty() )
         {
            created_files << last_created_file;
         }
         if ( !running )
         {
            lbl_signal->setText( save_signal );
            return;
         }
      }
      lbl_signal->setText( save_signal );
      if ( cb_multi_sub_avg->isChecked() )
      {
         avg( created_files );
      }
      if ( cb_multi_sub_conc_avg->isChecked() )
      {
         conc_avg( created_files );
      }
      if ( plot_dist_zoomer )
      {
         delete plot_dist_zoomer;
         plot_dist_zoomer = (ScrollZoomer *) 0;
      }
      plot_files();

      if ( !is_running )
      {
         running = false;
         update_enables();
         progress->setValue( 1 ); progress->setMaximum( 1 );
      }

   } else {
      run_one();
   }
}

void US_Hydrodyn_Saxs_Buffer::run_divide()
{
   if ( cb_multi_sub->isChecked() )
   {
      bool is_running = running;
      if ( !is_running )
      {
         running = true;
         update_enables();
      }

      QString save_signal = lbl_signal->text();
      map < QString, bool > selected_non_buffer_non_empty;
      QStringList created_files;
      
      for ( int i = 0; i < lb_files->count(); i++ )
      {
         if ( lb_files->item( i )->isSelected() && 
              lb_files->item( i )->text() != lbl_buffer->text() &&
              lb_files->item( i )->text() != lbl_empty->text() )
         {
            selected_non_buffer_non_empty[ lb_files->item( i )->text() ] = true;
         }
      }
      unsigned int total_points = selected_non_buffer_non_empty.size() + 1;
      unsigned int pos = 1;
      for ( map < QString, bool >::iterator it = selected_non_buffer_non_empty.begin();
            it != selected_non_buffer_non_empty.end();
            it++ )
      {
         if ( !is_running )
         {
            progress->setValue( pos++ ); progress->setMaximum( total_points );
         }
         lbl_signal->setText( it->first );
         qApp->processEvents();
         run_one_divide();
         if ( !last_created_file.isEmpty() )
         {
            created_files << last_created_file;
         }
         if ( !running )
         {
            lbl_signal->setText( save_signal );
            return;
         }
      }
      lbl_signal->setText( save_signal );
      if ( cb_multi_sub_avg->isChecked() )
      {
         avg( created_files );
      }
      if ( cb_multi_sub_conc_avg->isChecked() )
      {
         conc_avg( created_files );
      }
      if ( !is_running )
      {
         running = false;
         update_enables();
         progress->setValue( 1 ); progress->setMaximum( 1 );
      }
   } else {
      run_one_divide();
   }
}

void US_Hydrodyn_Saxs_Buffer::run_one( bool do_plot )
{
   // subtract buffer
   QString buffer   = lbl_buffer  ->text();
   QString solution = lbl_signal  ->text();
   QString empty    = lbl_empty   ->text();
   last_created_file = "";

   map < QString, bool > current_files;

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      QString this_file = lb_files->item( i )->text();
      current_files[ this_file ] = true;
   }

   if ( !current_files.count( buffer ) )
   {
      editor_msg( "red", QString( us_tr( "Error: no data found for buffer %1" ) ).arg( buffer ) );
      return;
   } 

   if ( !current_files.count( solution ) )
   {
      editor_msg( "red", QString( us_tr( "Error: no data found for solution %1" ) ).arg( solution ) );
      return;
   } 

   if ( !empty.isEmpty() && !current_files.count( empty ) )
   {
      editor_msg( "red", QString( us_tr( "Error: no data found for blank %1" ) ).arg( empty ) );
      return;
   } 

   if ( f_qs[ buffer ].size() != f_qs[ solution ].size() ||
        ( !empty.isEmpty() && f_qs[ buffer ].size() != f_qs[ empty ].size() ) )
   {
      editor_msg( "red", us_tr( "Error: incompatible grids, the files selected do not have the same number of points" ) );
      return;
   }

   vector < QString > bsub_q_string = f_qs_string [ solution ];
   vector < double >  bsub_q        = f_qs        [ solution ];
   vector < double >  bsub_I        = f_Is        [ solution ];
   vector < double >  bsub_error    = f_errors    [ solution ];

   bool solution_has_errors = f_errors[ solution ].size();
   bool buffer_has_errors   = f_errors[ buffer   ].size();
   bool empty_has_errors    = !empty.isEmpty() && f_errors[ empty ].size();

   for ( unsigned int j = 0; j < f_Is[ buffer ].size(); j++ )
   {
      if ( fabs( bsub_q[ j ] - f_qs[ buffer ][ j ] ) > Q_VAL_TOL ||
           ( !empty.isEmpty() && bsub_q[ j ] != f_qs[ empty ][ j ] ) )
      {
         editor_msg( "red", us_tr( "Error: incompatible grids, the q values differ between selected files" ) );
         return;
      }
   }
   
   // determine parameters
   bool use_alpha = ((QCheckBox *)(t_csv->cellWidget( 0, 1 )))->isChecked();
   bool use_psv   = ((QCheckBox *)(t_csv->cellWidget( 1, 1 )))->isChecked();

   if ( ( !use_alpha && !use_psv ) || ( use_alpha && use_psv ) )
   {
      editor_msg( "red", us_tr( "Internal error: both alpha & psv methods active" ) );
      return;
   }

   if (
       ( use_alpha && t_csv->item( 0, 6 )->text().isEmpty() ) ||
       (
        use_psv && ( t_csv->item( 1, 6 )->text().isEmpty() ||
                     t_csv->item( 2, 6 )->text().isEmpty() )
        )
       )
   {
      editor_msg( "red", us_tr( "Internal error: method selected does not have current values" ) );
      return;
   }
      
   double alpha  = t_csv->item( 0, 6 )->text().toDouble();
   double psv    = t_csv->item( 1, 6 )->text().toDouble();
   double gamma  = t_csv->item( 2, 6 )->text().toDouble();

   map < QString, double > concs = current_concs();
   double this_conc = concs.count( solution ) ? concs[ solution ] : 0e0;

   QString msg;
   QString tag;

   if ( use_psv )
   {
      if ( !concs.count( solution ) || concs[ solution ] == 0e0 )
      {
         editor_msg( "dark red", us_tr( "Warning: the solution has zero concentration" ) );
      }
         
      alpha = 1e0 - gamma * this_conc * psv / 1000;
      msg = QString( us_tr( "alpha %1 gamma %2 conc %3 psv %4" ) )
         .arg( alpha )
         .arg( gamma )
         .arg( this_conc )
         .arg( psv );
   } else {
      msg = QString( us_tr( "alpha %1" ) ).arg( alpha );
   }

   // assuming zero covariance for now
   if ( buffer_has_errors && !solution_has_errors )
   {
      editor_msg( "dark red", us_tr( "Warning: the buffer has errors defined but not the solution" ) );
      
      bsub_error = f_errors[ buffer ];
   } else {
      if ( !buffer_has_errors && !solution_has_errors && empty_has_errors )
      {
         // this is a strange case
         editor_msg( "dark red", us_tr( "Warning: the blank has errors defined but not the solution or buffer!" ) );
         bsub_error = f_errors[ empty ];
      }
   }

   for ( unsigned int i = 0; i < bsub_q.size(); i++ )
   {
      bsub_I[ i ] -= alpha * f_Is[ buffer ][ i ];
      if ( solution_has_errors && buffer_has_errors )
      {
         bsub_error[ i ] = sqrt( bsub_error[ i ] * bsub_error[ i ] +
                                 alpha * alpha * f_errors[ buffer ][ i ] * f_errors[ buffer ][ i ] );
      } else {
         if ( buffer_has_errors )
         {
            bsub_error[ i ] *= alpha;
         }
      }
            
      if ( !empty.isEmpty() )
      {
         bsub_I[ i ] -= ( 1e0 - alpha ) * f_Is[ empty ][ i ];
         if ( ( buffer_has_errors || solution_has_errors ) && empty_has_errors )
         {
            bsub_error[ i ] = sqrt( bsub_error[ i ] * bsub_error[ i ] +
                                    ( 1e0 - alpha ) * ( 1e0 - alpha )
                                    * f_errors[ empty ][ i ] * f_errors[ empty ][ i ] );
         }
      }
   }         

   // ok now we have a bsub!
   bool         any_negative   = false;
   unsigned int negative_pos   = 0;
   unsigned int negative_count = 0;
   double       minimum_positive = bsub_I[ 0 ];

   for ( unsigned int i = 0; i < bsub_I.size(); i++ )
   {
      if ( minimum_positive > bsub_I[ i ] )
      {
         minimum_positive = bsub_I[ i ];
      }
      if ( bsub_I[ i ] <= 0e0 )
      {
         negative_count++;
         if ( !any_negative )
         {
            negative_pos = i;
            any_negative = true;
         }
      }
   }

   if ( any_negative )
   {
      int result;
      if ( rb_np_crop->isChecked() )
      {
         editor_msg( "dark red", QString( us_tr( "Warning: non-positive values caused cropping at q = %1" ) ).arg( bsub_q[ negative_pos ] ) );
         result = 0;
      }
      if ( rb_np_min->isChecked() )
      {
         editor_msg( "dark red", QString( us_tr( "Warning: non-positive values caused %1 minimum values set" ) ).arg( negative_count ) );
         result = 1;
      }
      if ( rb_np_ignore->isChecked() )
      {
         editor_msg( "dark red", QString( us_tr( "Warning: %1 non-positive values ignored" ) ).arg( negative_count ) );
         result = 2;
      }
      if ( rb_np_ask->isChecked() )
      {
         result = QMessageBox::warning(this, 
                                       title,
                                       QString( us_tr( "Please note:\n\n"
                                                    "The buffer subtraction causes %1 points be non-positive\n"
                                                    "Starting at a q value of %2\n\n"
                                                    "What would you like to do?\n" ) )
                                       .arg( negative_count )
                                       .arg( bsub_q[ negative_pos ] ),
                                       us_tr( "&Crop the data" ), 
                                       us_tr( "&Set to the minimum positive value" ), 
                                       us_tr( "&Leave them negative or zero" ), 
                                       0, // Stop == button 0
                                       0 // Escape == button 0
                                       );
      }
         
      switch( result )
      {
      case 0 : // crop
         {
            if ( negative_pos < 2 )
            {
               if ( rb_np_ask->isChecked() )
               {
                  QMessageBox::warning(this, 
                                       title,
                                       us_tr("Insufficient data left after cropping"));
               } else {
                  editor_msg( "red", us_tr( "Notice: Cropping left nothing" ) );
               }
               return;
            }
               
            bsub_q_string.resize( negative_pos );
            bsub_q       .resize( negative_pos );
            bsub_I       .resize( negative_pos );
            if ( bsub_error.size() )
            {
               bsub_error.resize( negative_pos );
            }
         }
         break;
      case 1 : // use absolute value
         for ( unsigned int i = 0; i < bsub_I.size(); i++ )
         {
            if ( bsub_I[ i ] <= 0e0 )
            {
               bsub_I[ i ] = minimum_positive;
            }
         }
         break;
      case 2 : // ignore
         break;
      }
   }

   QString head = solution + QString( "_bsub_a%1" ).arg( alpha ).replace( ".", "_" );
   unsigned int ext = 0;

   QString bsub_name = head;

   while ( current_files.count( bsub_name ) )
   {
      bsub_name = head + QString( "-%1" ).arg( ++ext );
   }

   lb_created_files->addItem( bsub_name );
   lb_created_files->scrollToItem( lb_created_files->item( lb_created_files->count() - 1 ) );
   lb_files->addItem( bsub_name );
   lb_files->scrollToItem( lb_files->item( lb_files->count() - 1 ) );
   created_files_not_saved[ bsub_name ] = true;
   last_created_file = bsub_name;
   
   f_pos       [ bsub_name ] = f_qs.size();
   f_qs_string [ bsub_name ] = bsub_q_string;
   f_qs        [ bsub_name ] = bsub_q;
   f_Is        [ bsub_name ] = bsub_I;
   f_errors    [ bsub_name ] = bsub_error;
   
   // we could check if it has changed and then delete
   if ( do_plot )
   {
      if ( plot_dist_zoomer )
      {
         delete plot_dist_zoomer;
         plot_dist_zoomer = (ScrollZoomer *) 0;
      }
      plot_files();
   }

   update_csv_conc();
   for ( unsigned int i = 0; i < csv_conc.data.size(); i++ )
   {
      if ( csv_conc.data[ i ].size() > 1 &&
           csv_conc.data[ i ][ 0 ] == bsub_name )
      {
         csv_conc.data[ i ][ 1 ] = QString( "%1" ).arg( this_conc );
      }
   }

   if ( conc_widget )
   {
      conc_window->refresh( csv_conc );
   }

   if ( !running )
   {
      update_enables();
   }
}

void US_Hydrodyn_Saxs_Buffer::run_best()
{
}

void US_Hydrodyn_Saxs_Buffer::stop()
{
   running = false;
   saxs_window->stopFlag = true;
   editor_msg("red", "Stopped by user request\n");
   update_enables();
}

void US_Hydrodyn_Saxs_Buffer::update_enables()
{
   // cout << "US_Hydrodyn_Saxs_Buffer::update_enables()\n";
   // cout << QString("saxs_window->qsl_plotted_iq_names.size() %1\n").arg(saxs_window->qsl_plotted_iq_names.size());

   unsigned int files_selected_count                      = 0;
   unsigned int non_buffer_non_empty_files_selected_count = 0;
   unsigned int last_selected_pos                         = 0;

   map < QString, bool > selected_map;

   QString last_selected_file;

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         selected_map[ lb_files->item( i )->text() ] = true;
         last_selected_pos = i;
         last_selected_file = lb_files->item( i )->text();
         files_selected_count++;
         if ( lb_files->item( i )->text() != lbl_buffer->text() &&
              lb_files->item( i )->text() != lbl_empty->text() )
         {
            non_buffer_non_empty_files_selected_count++;
         }
      }
   }

   lbl_selected->setText( QString( us_tr( "%1 of %2 files selected" ) )
                          .arg( files_selected_count )
                          .arg( lb_files->count() ) );

   unsigned int files_created_selected_not_saved_count = 0;
   unsigned int files_created_selected_count           = 0;
   unsigned int files_created_selected_not_shown_count = 0;
   map < QString, bool > created_selected_map;

   QString last_created_selected_file;

   for ( int i = 0; i < lb_created_files->count(); i++ )
   {
      if ( lb_created_files->item( i )->isSelected() )
      {
         last_created_selected_file = lb_created_files->item( i )->text();
         created_selected_map[ lb_created_files->item( i )->text() ] = true;
         files_created_selected_count++;
         if ( !selected_map.count( lb_created_files->item( i )->text() ) )
         {
            files_created_selected_not_shown_count++;
         } 
         if ( created_files_not_saved.count( lb_created_files->item( i )->text() ) )
         {
            files_created_selected_not_saved_count++;
         }
      }
   }

   lbl_selected_created->setText( QString( us_tr( "%1 of %2 files selected" ) )
                                  .arg( files_created_selected_count )
                                  .arg( lb_created_files->count() ) );

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

   pb_similar_files      ->setEnabled( files_selected_count == 1 );
   pb_conc               ->setEnabled( lb_files->count() > 0 );
   pb_clear_files        ->setEnabled( files_selected_count > 0 );
   pb_avg                ->setEnabled( files_selected_count > 1 );
   pb_asum               ->setEnabled( files_selected_count );
   pb_normalize          ->setEnabled( all_selected_have_nonzero_conc() );
   pb_conc_avg           ->setEnabled( all_selected_have_nonzero_conc() );
   pb_set_buffer         ->setEnabled( files_selected_count == 1 && 
                                       lb_files->item( last_selected_pos )->text() != lbl_buffer->text() &&
                                       lb_files->item( last_selected_pos )->text() != lbl_empty ->text() &&
                                       lb_files->item( last_selected_pos )->text() != lbl_signal->text()
                                       );
   pb_set_signal         ->setEnabled( files_selected_count == 1 && 
                                       lb_files->item( last_selected_pos )->text() != lbl_buffer->text() &&
                                       lb_files->item( last_selected_pos )->text() != lbl_empty ->text() &&
                                       lb_files->item( last_selected_pos )->text() != lbl_signal->text() );
   pb_set_empty          ->setEnabled( files_selected_count == 1 && 
                                       lb_files->item( last_selected_pos )->text() != lbl_buffer->text() &&
                                       lb_files->item( last_selected_pos )->text() != lbl_empty ->text() &&
                                       lb_files->item( last_selected_pos )->text() != lbl_signal->text() );
   pb_select_all         ->setEnabled( lb_files->count() > 0 );
   pb_invert             ->setEnabled( lb_files->count() > 0 );
   pb_color_rotate       ->setEnabled( files_selected_count );
   //    pb_join               ->setEnabled( files_selected_count == 2 );
   pb_join_start         ->setEnabled( files_selected_count == 2 );
   // pb_adjacent           ->setEnabled( lb_files->count() > 1 );
   pb_select_nth         ->setEnabled( lb_files->count() > 2 );
   pb_to_saxs            ->setEnabled( files_selected_count );
   pb_view               ->setEnabled( files_selected_count && files_selected_count <= 10 );
   pb_rescale            ->setEnabled( files_selected_count > 0 );

   pb_select_all_created ->setEnabled( lb_created_files->count() > 0 );
   pb_adjacent_created   ->setEnabled( lb_created_files->count() > 1 );
   pb_save_created_csv   ->setEnabled( files_created_selected_count > 0 );
   pb_save_created       ->setEnabled( files_created_selected_not_saved_count > 0 );

   pb_show_created       ->setEnabled( files_created_selected_not_shown_count > 0 );
   pb_show_only_created  ->setEnabled( files_created_selected_count > 0 &&
                                       files_selected_not_created > 0 );

   bool any_best_empty    = false;
   bool any_current_empty = false;
   bool any_selected      = 
      ( ((QCheckBox *)(t_csv->cellWidget( 0, 1 )))->isChecked() ||
        ( ((QCheckBox *)(t_csv->cellWidget( 1, 1 )))->isChecked() &&
          ( ((QCheckBox *)(t_csv->cellWidget( 2, 1 )))->isChecked() ||
            !t_csv->item( 2, 6 )->text().isEmpty() ) )
        );
   if ( !running )
   {
      for ( unsigned int i = 0; i < (unsigned int)t_csv->rowCount(); i++ )
      {
         if ( ((QCheckBox *)(t_csv->cellWidget( i, 1 )))->isChecked() )
         {
            if ( t_csv->item(i, 7)->text().isEmpty() )
            {
               any_best_empty = true;
            }
            if ( t_csv->item(i, 6)->text().isEmpty() )
            {
               any_current_empty = true;
            }
         }
      }
   }
   pb_start            ->setEnabled( !running && any_selected &&
                                     ( !lbl_signal->text().isEmpty() ||
                                       ( cb_multi_sub->isChecked() && 
                                         non_buffer_non_empty_files_selected_count > 0 ) ) &&
                                     !lbl_buffer->text().isEmpty() );
   pb_run_current      ->setEnabled( !running && any_selected &&
                                     ( !lbl_signal->text().isEmpty() ||
                                       ( cb_multi_sub->isChecked() && 
                                         non_buffer_non_empty_files_selected_count > 0 ) ) &&
                                     !lbl_buffer->text().isEmpty() &&
                                     !any_current_empty
                                     );
   pb_run_divide       ->setEnabled( !running && any_selected &&
                                     ( !lbl_signal->text().isEmpty() ||
                                       ( cb_multi_sub->isChecked() && 
                                         non_buffer_non_empty_files_selected_count > 0 ) ) &&
                                     !lbl_buffer->text().isEmpty() &&
                                     !any_current_empty
                                     );
   pb_run_best         ->setEnabled( !running && 
                                     ( !lbl_signal->text().isEmpty() ||
                                       ( cb_multi_sub->isChecked() && 
                                         non_buffer_non_empty_files_selected_count > 0 ) ) &&
                                     !lbl_buffer->text().isEmpty() &&
                                     !any_best_empty && any_selected);
   pb_stop             ->setEnabled( running );

   cb_multi_sub          ->setEnabled( !running );
   cb_multi_sub_avg      ->setEnabled( !running && cb_multi_sub->isChecked() );
   cb_multi_sub_conc_avg ->setEnabled( !running && cb_multi_sub->isChecked() );

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
   pb_crop_common      ->setEnabled( files_selected_count );
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
   pb_legend           ->setEnabled( lb_files->count() && files_selected_count <= 20 );
   pb_axis_x           ->setEnabled( lb_files->count() );
   pb_axis_y           ->setEnabled( lb_files->count() );
   pb_color_rotate     ->setEnabled( true );
   // cb_guinier          ->setEnabled( files_selected_count );
   legend_set();

   if ( *saxs_widget )
   {
      saxs_window->update_iqq_suffix();
   }
}

bool US_Hydrodyn_Saxs_Buffer::any_to_run()
{
   for ( unsigned int i = 0; i < (unsigned int)t_csv->rowCount(); i++ )
   {
      if ( ((QCheckBox *)(t_csv->cellWidget( i, 1 )))->isChecked() )
      {
         return true;
      }
   }
   editor_msg("dark red", "Nothing to do: At least one of the rows must have Active set to Y");
   return false;
}

void US_Hydrodyn_Saxs_Buffer::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->textColor();
   editor->setTextColor(color);
   editor->append(msg);
   editor->setTextColor(save_color);
}

void US_Hydrodyn_Saxs_Buffer::editor_msg_qc( QColor qcolor, QString msg )
{
   QColor save_color = editor->textColor();
   editor->setTextColor(qcolor);
   editor->append(msg);
   editor->setTextColor(save_color);
}

bool US_Hydrodyn_Saxs_Buffer::validate()
{
   bool errors = false;

   for ( unsigned int i = 0; i < (unsigned int)t_csv->rowCount(); i++ )
   {
      if ( t_csv->item( i, 2 )->text().toDouble() >
           t_csv->item( i, 3 )->text().toDouble() )
      {
         editor_msg("red", QString("Row %1 column \"Low value\" can not be greater than \"High value\"\n").arg(i));
         errors = true;
      }
      if ( t_csv->item( i, 4 )->text().toInt() < 1 )
      {
         editor_msg("red", QString("Row %1 column \"Points\" must be greater or equal to one\n").arg(i));
         errors = true;
      }
      if ( t_csv->item( i, 4 )->text().toDouble() == 1  &&
           t_csv->item( i, 2 )->text().toDouble() >
           t_csv->item( i, 3 )->text().toDouble() )
      {
         editor_msg("red", QString("Row %1 one \"Points\" requires \"Low value\" equals \"High value\"\n").arg(i));
         errors = true;
      }
   }

   return !errors;
}

bool US_Hydrodyn_Saxs_Buffer::activate_saxs_window()
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

csv US_Hydrodyn_Saxs_Buffer::current_csv()
{
   csv tmp_csv = csv1;
   
   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      for ( unsigned int j = 0; j < csv1.data[i].size(); j++ )
      {
         if ( csv1.data[i][j] == "Y" || csv1.data[i][j] == "N" )
         {
            tmp_csv.data[i][j] = ((QCheckBox *)(t_csv->cellWidget( i, j )))->isChecked() ? "Y" : "N";
         } else {
            tmp_csv.data[i][j] = t_csv->item( i, j )->text();
         }
         tmp_csv.num_data[i][j] = tmp_csv.data[i][j].toDouble();
      }
   }
   return tmp_csv;
}
  
void US_Hydrodyn_Saxs_Buffer::recompute_interval_from_points()
{
   for ( unsigned int i = 0; i < (unsigned int)t_csv->rowCount(); i++ )
   {
      QString toset =
         t_csv->item(i, 4)->text().toDouble() == 0e0 ?
         ""
         :
         QString("%1")
         .arg( ( t_csv->item(i, 3)->text().toDouble() -
                 t_csv->item(i, 2)->text().toDouble() )
               / ( t_csv->item(i, 4)->text().toDouble() - 1e0 ) )
         ;
         
      t_csv->setItem( i, 5, new QTableWidgetItem( toset ) );
   }
}


void US_Hydrodyn_Saxs_Buffer::recompute_points_from_interval()
{
   for ( unsigned int i = 0; i < (unsigned int)t_csv->rowCount(); i++ )
   {
      QString toset =
         t_csv->item(i, 5)->text().toDouble() == 0e0 ?
         ""
         :
         QString("%1")
         .arg( 1 + (unsigned int)(( t_csv->item(i, 3)->text().toDouble() -
                                    t_csv->item(i, 2)->text().toDouble() )
                                  / t_csv->item(i, 5)->text().toDouble() + 0.5) )
         ;
      t_csv->setItem( i, 4, new QTableWidgetItem( toset ) );
   }
}

void US_Hydrodyn_Saxs_Buffer::clear_files()
{
   QStringList files;
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         files << lb_files->item( i )->text();
      }
   }
   clear_files( files );
   update_enables();
}
void US_Hydrodyn_Saxs_Buffer::clear_files( QStringList files )
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
         qsl << QString( us_tr( "... and %1 more not listed" ) ).arg( created_not_saved_list.size() - qsl.size() );
      }

      switch ( QMessageBox::warning(this, 
                                    title + us_tr( " Remove Files" ),
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
         msg += QString( us_tr( "Removed %1\n" ) ).arg( lb_files->item( i )->text() );
         if ( lbl_buffer->text() == lb_files->item( i )->text() )
         {
            lbl_buffer->setText( "" );
         }
         if ( lbl_signal->text() == lb_files->item( i )->text() )
         {
            lbl_signal->setText( "" );
         }
         if ( lbl_empty->text() == lb_files->item( i )->text() )
         {
            lbl_empty->setText( "" );
         }
         f_qs_string.erase( lb_files->item( i )->text() );
         f_qs       .erase( lb_files->item( i )->text() );
         f_Is       .erase( lb_files->item( i )->text() );
         f_errors   .erase( lb_files->item( i )->text() );
         f_pos      .erase( lb_files->item( i )->text() );
         f_name     .erase( lb_files->item( i )->text() );
         f_psv      .erase( lb_files->item( i )->text() );
         f_I0se     .erase( lb_files->item( i )->text() );
         f_conc     .erase( lb_files->item( i )->text() );
         f_extc     .erase( lb_files->item( i )->text() );
         delete lb_files->takeItem( i );
      }
   }

   lb_files->setUpdatesEnabled( true );

   if ( !msg.isEmpty() ) {
      editor_msg( "black", msg );
   }

   disable_updates = false;
   plot_files();
   if ( !lb_files->count() &&
        plot_dist_zoomer )
   {
      delete plot_dist_zoomer;
      plot_dist_zoomer = (ScrollZoomer *) 0;
   }
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

class hplc_sortable_qstring {
public:
   double       x;
   QString      name;
   bool operator < (const hplc_sortable_qstring& objIn) const
   {
      return x < objIn.x;
   }
};

void US_Hydrodyn_Saxs_Buffer::add_files()
{
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
      QDir::setCurrent( lbl_dir->text() );
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
      last_load_dir = QFileInfo( filenames[ 0 ] ).path();
      if ( !cb_lock_dir->isChecked() )
      {
         QDir::setCurrent( last_load_dir );
         lbl_dir        ->setText( QDir::currentPath() );
         lbl_created_dir->setText( QDir::currentPath() + "/produced" ); 
      }
      editor_msg( "black", QString( us_tr( "loaded from %1:" ) ).arg( last_load_dir ) );
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
      us_qdebug( QString( "sort head <%1> tail <%2>  dp %3 " ).arg( head ).arg( tail ).arg( add_dp ? "yes" : "no" ) );
#endif
      
      set < QString > used;

      for ( int i = 0; i < (int) filenames.size(); ++i )
      {
         QString tmp = filenames[ i ].mid( head.length() );
         tmp = tmp.mid( 0, tmp.length() - tail.length() );
         if ( rx_cap.indexIn( tmp ) != -1 )
         {
#ifdef DEBUG_LOAD_REORDER
            us_qdebug( QString( "rx_cap search tmp %1 found" ).arg( tmp ) );
#endif
            tmp = rx_cap.cap( 1 ) + "." + rx_cap.cap( 2 );
#ifdef DEBUG_LOAD_REORDER
         } else {
            us_qdebug( QString( "rx_cap search tmp %1 NOT found" ).arg( tmp ) );
#endif
         }

         if ( add_dp )
         {
            tmp = "0." + tmp;
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

         hplc_sortable_qstring sval;
         sval.x     = tmp.toDouble();
         sval.name  = filenames[ i ];
         svals      .push_back( sval );
#ifdef DEBUG_LOAD_REORDER
         us_qdebug( QString( "sort tmp <%1> xval <%2>" ).arg( tmp ).arg( sval.x ) );
#endif
      }
      if ( reorder )
      {
#ifdef DEBUG_LOAD_REORDER
         us_qdebug( "reordered" );
#endif
         svals.sort();

         filenames.clear( );
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
      if ( *saxs_widget )
      {
         saxs_window->add_to_directory_history( filenames[ i ] );
      }

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

void US_Hydrodyn_Saxs_Buffer::add_files( QStringList filenames )
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
         lbl_dir        ->setText( QDir::currentPath() );
         lbl_created_dir->setText( QDir::currentPath() + "/produced" ); 
      }
      editor_msg( "black", QString( us_tr( "loaded from %1:" ) ).arg( last_load_dir ) );
   }

   QString errors;

   for ( int i = 0; i < (int)filenames.size(); i++ )
   {
      if ( *saxs_widget )
      {
         saxs_window->add_to_directory_history( filenames[ i ] );
      }

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

void US_Hydrodyn_Saxs_Buffer::plot_files()
{
   plot_dist->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_dist->detachItems( QwtPlotItem::Rtti_PlotMarker );;
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

   plotted_curves.clear( );

   if ( all_selected_files().size() > 20 &&
#if QT_VERSION < 0x040000
        plot_dist->autoLegend() 
#else
        legend_vis
#endif
        )
   {
      legend();
   }

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         //any_selected = true;
         if ( plot_file( lb_files->item( i )->text(), file_minx, file_maxx, file_miny, file_maxy ) )
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
         if ( get_min_max( lb_files->item( i )->text(), file_minx, file_maxx, file_miny, file_maxy ) )
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
#if QT_VERSION < 0x040000
      plot_dist_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
      connect( plot_dist_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_zoomed( const QRectF & ) ) );
   }
   
   legend_set();
   plot_dist->replot();
}

bool US_Hydrodyn_Saxs_Buffer::plot_file( QString file,
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
      editor_msg( "red", QString( us_tr( "Internal error: request to plot %1, but not found in data" ) ).arg( file ) );
      return false;
   }

   get_min_max( file, minx, maxx, miny, maxy );

#if QT_VERSION < 0x040000
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
#if QT_VERSION < 0x040000
      plot_dist->setCurveData( Iq, 
                               /* cb_guinier->isChecked() ? (double *)&(plotted_q2[p][0]) : */
                               (double *)&( f_qs[ file ][ 0 ] ),
                               (double *)&( f_Is[ file ][ 0 ] ),
                               q_points
                               );
      plot_dist->setCurvePen( Iq, QPen( plot_colors[ f_pos[ file ] % plot_colors.size()], 1, SolidLine));
#else
      curve->setSamples(
                     /* cb_guinier->isChecked() ?
                        (double *)&(plotted_q2[p][0]) : */
                     (double *)&( f_qs[ file ][ 0 ] ),
                     (double *)&( f_Is[ file ][ 0 ] ),
                     q_points
                     );

      curve->setPen( QPen( plot_colors[ f_pos[ file ] % plot_colors.size() ], 1, Qt::SolidLine ) );
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
#if QT_VERSION < 0x040000
      plot_dist->setCurveData( Iq, 
                               /* cb_guinier->isChecked() ? (double *)&(plotted_q2[p][0]) : */
                               (double *)&( q[ 0 ] ),
                               (double *)&( I[ 0 ] ),
                               q_points
                               );
      plot_dist->setCurvePen( Iq, QPen( plot_colors[ f_pos[ file ] % plot_colors.size()], 1, SolidLine));
#else
      curve->setSamples(
                     /* cb_guinier->isChecked() ?
                        (double *)&(plotted_q2[p][0]) : */
                     (double *)&( q[ 0 ] ),
                     (double *)&( I[ 0 ] ),
                     q_points
                     );

      curve->setPen( QPen( plot_colors[ f_pos[ file ] % plot_colors.size() ], 1, Qt::SolidLine ) );
      curve->attach( plot_dist );
#endif
   }
   return true;
}


void US_Hydrodyn_Saxs_Buffer::update_files()
{
   if ( !disable_updates )
   {
      plot_files();
      update_enables();
   }
}

bool US_Hydrodyn_Saxs_Buffer::get_min_max( QString file,
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
      // editor_msg( "red", QString( us_tr( "Internal error: requested %1, but not found in data" ) ).arg( file ) );
      return false;
   }

   minx = f_qs[ file ][ 0 ];
   maxx = f_qs[ file ][ f_qs[ file ].size() - 1 ];

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
   return true;
}

void US_Hydrodyn_Saxs_Buffer::invert()
{
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

void US_Hydrodyn_Saxs_Buffer::select_all()
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

void US_Hydrodyn_Saxs_Buffer::select_all_created()
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

bool US_Hydrodyn_Saxs_Buffer::load_file( QString filename )
{
   errormsg = "";
   QFile f( filename );
   if ( !f.exists() )
   {
      errormsg = QString("Error: %1 does not exist").arg( filename );
      return false;
   }
   //   cout << QString( "opening %1\n" ).arg( filename ) << flush;
   
   QString ext = QFileInfo( filename ).suffix().toLower();

   QRegExp rx_valid_ext (
                         "^("
                         "dat|"
                         "int|"
                         "txt|"
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

   double this_conc = 0e0;
   double this_psv  = 0e0;
   double this_I0se = 0e0;

   double use_units = ( ( US_Hydrodyn * ) us_hydrodyn )->saxs_options.iq_scale_angstrom ? 1.0 : 0.1;

   if ( ext == "dat" )
   {
      QRegExp rx_conc      ( "Conc:\\s*(\\S+)(\\s|$)" );
      QRegExp rx_psv       ( "PSV:\\s*(\\S+)(\\s|$)" );
      QRegExp rx_I0se      ( "I0se:\\s*(\\S+)(\\s|$)" );
      QRegExp rx_unit      ( "Units:\\s*(\\S+)(\\s|$)" );
      if ( rx_unit.indexIn( qv[ 0 ] ) != -1 )
      {
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
      if ( rx_conc.indexIn( qv[ 0 ] ) != -1 )
      {
         this_conc = rx_conc.cap( 1 ).toDouble();
         // cout << QString( "found conc %1\n" ).arg( this_conc );
      }
      if ( rx_psv.indexIn( qv[ 0 ] ) != -1 )
      {
         this_psv = rx_psv.cap( 1 ).toDouble();
      }
      if ( rx_I0se.indexIn( qv[ 0 ] ) != -1 )
      {
         this_I0se = rx_I0se.cap( 1 ).toDouble();
      }
   }

   // we should make some configuration for matches & offsets or column mapping
   // just an ad-hoc fix for APS 5IDD
   int offset = 0;
   if ( ext == "txt" && qv[ 0 ].contains( "# File Encoding (File origin in Excel)" ) )
   {
      offset = 1;
   }      

   vector < QString > q_string;
   vector < double >  q;
   vector < double >  I;
   vector < double >  e;

   QRegExp rx_ok_line("^(\\s+(-|)|\\d+|\\.|\\d(E|e)(\\+|-|\\d))+$");
   rx_ok_line.setMinimal( true );
   for ( int i = 1; i < (int) qv.size(); i++ )
   {
      if ( qv[i].contains(QRegExp("^#")) ||
           rx_ok_line.indexIn( qv[i] ) == -1 )
      {
         continue;
      }
      
      // QStringList tokens = (qv[i].replace(QRegExp("^\\s+").split( QRegExp("\\s+") , Qt::SkipEmptyParts ),""));
      QStringList tokens;
      {
         QString qs = qv[i].replace(QRegExp("^\\s+"),"");
         tokens = (qs ).split( QRegExp("\\s+") , Qt::SkipEmptyParts );
      }

      if ( (int)tokens.size() > 1 + offset )
      {
         QString this_q_string = tokens[ 0 + offset ];
         double this_q         = tokens[ 0 + offset ].toDouble();
         double this_I         = tokens[ 1 + offset ].toDouble();
         double this_e = 0e0;
         if ( use_units != 1 ) {
            this_q *= use_units;
            this_q_string = QString( "%1" ).arg( this_q );
         }
         if ( (int)tokens.size() > 2 + offset)
         {
            this_e = tokens[ 2 + offset ].toDouble();
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
         if ( this_I != 0e0 )
         {
            q_string.push_back( this_q_string );
            q       .push_back( this_q );
            I       .push_back( this_I );
            if ( (int)tokens.size() > 2 + offset && this_e )
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

   // cout << QString( "opened %1\n" ).arg( filename ) << flush;
   QString basename = QFileInfo( filename ).completeBaseName();
   f_name      [ basename ] = filename;
   f_pos       [ basename ] = f_qs.size();
   f_qs_string [ basename ] = q_string;
   f_qs        [ basename ] = q;
   f_Is        [ basename ] = I;

   if ( this_conc )
   {
      f_conc[ basename ] = this_conc;
      update_csv_conc();
   }
   if ( this_psv )
   {
      f_psv [ basename ] = this_psv;
   }
   if ( this_I0se )
   {
      f_I0se[ basename ] = this_I0se;
   }
      
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
   return true;
}

void US_Hydrodyn_Saxs_Buffer::set_buffer()
{
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         lbl_buffer->setText( lb_files->item( i )->text() );
      }
   }
   update_csv_conc();
   if ( conc_widget )
   {
      conc_window->refresh( csv_conc );
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Buffer::set_signal()
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

void US_Hydrodyn_Saxs_Buffer::set_empty()
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

void US_Hydrodyn_Saxs_Buffer::update_created_files()
{
   if ( !disable_updates )
   {
      update_enables();
   }
}

void US_Hydrodyn_Saxs_Buffer::avg()
{
   QStringList files;
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() && 
           lb_files->item( i )->text() != lbl_buffer->text() &&
           lb_files->item( i )->text() != lbl_empty->text() )
      {
         files << lb_files->item( i )->text();
      }
   }
   avg( files );
}

void US_Hydrodyn_Saxs_Buffer::asum()
{
   QStringList files;
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         files << lb_files->item( i )->text();
      }
   }
   asum( files );
}

void US_Hydrodyn_Saxs_Buffer::avg( QStringList files )
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
   double avg_I0se = 0e0;

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
      editor_msg( "dark red", QString( us_tr( "Notice: averaging requires cropping to %1 points" ) ).arg( min_q_len ) );
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
                  us_tr( "Notice: using standard deviation in mean calculation" ) );
   } else {
      editor_msg( "black" ,
                  us_tr( "Notice: NOT using standard deviation in mean calculation, since some sd's were zero or missing" ) );
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
         avg_psv  = f_psv.count( this_file ) ? f_psv[ this_file ] : 0e0;
         avg_I0se = f_I0se.count( this_file ) ? f_I0se[ this_file ] : 0e0;
      } else {
         if ( avg_qs.size() != t_qs[ this_file ].size() )
         {
            editor_msg( "red", us_tr( "Error: incompatible grids, the files selected do not have the same number of points" ) );
            return;
         }
         for ( int j = 0; j < (int)t_Is[ this_file ].size(); j++ )
         {
            if ( fabs( avg_qs[ j ] - t_qs[ this_file ][ j ] ) > Q_VAL_TOL )
            {
               editor_msg( "red", us_tr( "Error: incompatible grids, the q values differ between selected files" ) );
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
         avg_psv  += f_psv.count( this_file ) ? f_psv[ this_file ] : 0e0;
         avg_I0se += f_I0se.count( this_file ) ? f_I0se[ this_file ] : 0e0;
      }            
   }

   if ( selected_count < 2 )
   {
      editor_msg( "red", us_tr( "Error: not at least 2 files selected so there is nothing to average" ) );
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
      // sum *= sum_weight[ i ] / ( sum_weight[ i ] * sum_weight[ i ] - sum_weight2[ i ] );
      avg_sd[ i ] = sqrt( sum / ( ( (double) files.size() - 1e0 ) * ( sum_weight[ i ]  / (double) files.size() ) ) );
   }

   avg_conc /= files.size();
   avg_psv  /= files.size();
   avg_I0se /= files.size();

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
   for ( int i = 0; i < (int)lb_files->count(); i++ )
   {
      current_files[ lb_files->item( i )->text() ] = true;
   }

   while ( current_files.count( avg_name ) )
   {
      avg_name = head + QString( "avg-%1" ).arg( ++ext ) + tail;
   }

   lb_created_files->addItem( avg_name );
   lb_created_files->scrollToItem( lb_created_files->item( lb_created_files->count() - 1 ) );
   lb_files->addItem( avg_name );
   lb_files->scrollToItem( lb_files->item( lb_files->count() - 1 ) );
   created_files_not_saved[ avg_name ] = true;
   
   f_pos       [ avg_name ] = f_qs.size();
   f_qs_string [ avg_name ] = avg_qs_string;
   f_qs        [ avg_name ] = avg_qs;
   f_Is        [ avg_name ] = avg_Is;
   f_errors    [ avg_name ] = avg_sd;
   
   if ( avg_psv )
   {
      f_psv       [ avg_name ] = avg_psv;
   }
   if ( avg_I0se )
   {
      f_I0se      [ avg_name ] = avg_I0se;
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


QString US_Hydrodyn_Saxs_Buffer::qstring_common_head( QStringList qsl, bool strip_digits )
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

QString US_Hydrodyn_Saxs_Buffer::qstring_common_tail( QStringList qsl, bool strip_digits )
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
      
QString US_Hydrodyn_Saxs_Buffer::qstring_common_head( QString s1, 
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

QString US_Hydrodyn_Saxs_Buffer::qstring_common_tail( QString s1, 
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

void US_Hydrodyn_Saxs_Buffer::save_created()
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

void US_Hydrodyn_Saxs_Buffer::save_created_csv()
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

bool US_Hydrodyn_Saxs_Buffer::save_files_csv( QStringList files )
{
   if ( !files.size() )
   {
      editor_msg( "red", us_tr( "Internal error: save_files_csv called empty" ) );
      return false;
   }

   {
      QDir dir1( lbl_created_dir->text() );
      if ( !dir1.exists() )
      {
         if ( dir1.mkdir( lbl_created_dir->text() ) )
         {
            editor_msg( "black", QString( us_tr( "Created directory %1" ) ).arg( lbl_created_dir->text() ) );
         } else {
            editor_msg( "red", QString( us_tr( "Error: Can not create directory %1 Check permissions." ) ).arg( lbl_created_dir->text() ) );
            return false;
         }
      }
   }         

   if ( !QDir::setCurrent( lbl_created_dir->text() ) )
   {
      editor_msg( "red", QString( us_tr( "Error: can not set directory %1" ) ).arg( lbl_created_dir->text() ) );
      return false;
   }

   for ( int i = 0; i < (int)files.size(); i++ )
   {
      if ( !f_qs.count( files[ i ] ) )
      {
         editor_msg( "red", QString( us_tr( "Error: no data found for %1" ) ).arg( files[ i ] ) );
         return false;
      } 
   }

   QString head = qstring_common_head( files, true );

   QString use_filename = head + ".csv";

   editor_msg( "black", QString( us_tr( "Current directory is %1" ) ).arg( QDir::current().canonicalPath() ) );

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

   QStringList qline;
   for ( unsigned int i = 0; i < t_qs_string[ files[ 0 ] ].size(); i++ )
   {
      qline << t_qs_string[ files[ 0 ] ][ i ];
   }

   ts << 
      QString( "\"Name\",\"Type; q:\",%1,\"%2%3\"\n" )
      .arg( qline.join( "," ) )
      .arg( title + us_tr( " utility output" ) )
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
   editor_msg( "black", QString( us_tr( "%1 written as %2" ) )
               .arg( files.join( " " ) )
               .arg( use_filename ) );
   return true;
}

bool US_Hydrodyn_Saxs_Buffer::save_files( QStringList files )
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

bool US_Hydrodyn_Saxs_Buffer::save_file( QString file, bool &cancel, bool &overwrite_all )
{
   if ( !f_qs.count( file ) )
   {
      editor_msg( "red", QString( us_tr( "Error: no data found for %1" ) ).arg( file ) );
      return false;
   } 

   {
      QDir dir1( lbl_created_dir->text() );
      if ( !dir1.exists() )
      {
         if ( dir1.mkdir( lbl_created_dir->text() ) )
         {
            editor_msg( "black", QString( us_tr( "Created directory %1" ) ).arg( lbl_created_dir->text() ) );
         } else {
            editor_msg( "red", QString( us_tr( "Error: Can not create directory %1 Check permissions." ) ).arg( lbl_created_dir->text() ) );
            return false;
         }
      }
   }         

   if ( !QDir::setCurrent( lbl_created_dir->text() ) )
   {
      editor_msg( "red", QString( us_tr( "Error: can not set directory %1" ) ).arg( lbl_created_dir->text() ) );
      return false;
   }

   QString use_filename;
   if ( f_name.count( file ) )
   {
      use_filename = QFileInfo( f_name[ file ] ).fileName();
   } else {
      use_filename = file + ".dat";
   }

   if ( QFile::exists( use_filename ) )
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
      editor_msg( "red", QString( us_tr( "Error: can not open %1 for writing" ) ).arg( use_filename ) );
      return false;
   }

   QTextStream ts( &f );

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

   ts << QString( title + us_tr( " output: %1%2%3%4 Units:1/a\n" ) )
      .arg( file )
      .arg( f_psv .count( file ) ? QString( " PSV:%1"  ).arg( f_psv [ file ] ) : QString( "" ) )
      .arg( f_I0se.count( file ) ? QString( " I0se:%1" ).arg( f_I0se[ file ] ) : QString( "" ) )
      .arg( use_conc )
      ;

   bool use_errors = ( f_errors.count( file ) && 
                       f_errors[ file ].size() > 0 );

   if ( use_errors )
   {
      ts << "q                 \tI(q)         \tsd\n";
   } else {
      ts << "q                 \tI(q)\n";
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
   return true;
}

void US_Hydrodyn_Saxs_Buffer::update_csv_conc()
{
   map < QString, bool > skip;
   if ( !lbl_buffer->text().isEmpty() )
   {
      skip[ lbl_buffer->text() ] = true;
   }
   if ( !lbl_empty->text().isEmpty() )
   {
      skip[ lbl_empty->text() ] = true;
   }

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

void US_Hydrodyn_Saxs_Buffer::conc()
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
      conc_window = new US_Hydrodyn_Saxs_Buffer_Conc( csv_conc,
                                                      this );
      conc_window->show();
   }
}

map < QString, double > US_Hydrodyn_Saxs_Buffer::current_concs( bool quiet )
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
                               title,
                               us_tr( "There are unsaved updates in the open Solution Concentration window\n"
                                   "This will cause the concentration values used by the current calculation\n"
                                   "to differ from those shown in the Solution Concentration window\n"
                                   "You probably want to save the values in the Solution Concentration window and repeat the computation."
                                   ) );
      }
   }
   return concs;
}

map < QString, double > US_Hydrodyn_Saxs_Buffer::window_concs()
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
   
void US_Hydrodyn_Saxs_Buffer::conc_avg()
{
   QStringList files;
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() && 
           lb_files->item( i )->text() != lbl_buffer->text() &&
           lb_files->item( i )->text() != lbl_empty->text() )
      {
         files << lb_files->item( i )->text();
      }
   }
   conc_avg( files );
}

void US_Hydrodyn_Saxs_Buffer::conc_avg( QStringList files )
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
      editor_msg( "dark red", QString( us_tr( "Notice: averaging requires cropping to %1 points" ) ).arg( min_q_len ) );
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
                  us_tr( "Notice: using standard deviation in mean calculation" ) );
   } else {
      editor_msg( "black" ,
                  us_tr( "Notice: NOT using standard deviation in mean calculation, since some sd's were zero or missing" ) );
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
         editor_msg( "red", QString( us_tr( "Error: found zero or no concentration for %1" ) ).arg( this_file ) );
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
      } else {
         if ( avg_qs.size() != t_qs[ this_file ].size() )
         {
            editor_msg( "red", us_tr( "Error: incompatible grids, the files selected do not have the same number of points" ) );
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
               editor_msg( "red", us_tr( "Error: incompatible grids, the q values differ between selected files" ) );
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
      }            
   }

   if ( selected_count < 2 )
   {
      editor_msg( "red", us_tr( "Error: not at least 2 files selected so there is nothing to average" ) );
      return;
   }      

   vector < double > avg_sd( avg_qs.size() );

   avg_conc /= files.size();

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
      // sum *= sum_weight[ i ] / ( sum_weight[ i ] * sum_weight[ i ] - sum_weight2[ i ] );
      avg_sd[ i ] = avg_conc * sqrt( sum / ( ( (double) files.size() - 1e0 ) * ( sum_weight[ i ]  / (double) files.size() ) ) );

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
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      current_files[ lb_files->item( i )->text() ] = true;
   }

   while ( current_files.count( avg_name ) )
   {
      avg_name = head + QString( "cnavg-%1" ).arg( ++ext ) + tail;
   }

   lb_created_files->addItem( avg_name );
   lb_created_files->scrollToItem( lb_created_files->item( lb_created_files->count() - 1 ) );
   lb_files->addItem( avg_name );
   lb_files->scrollToItem( lb_files->item( lb_files->count() - 1 ) );
   created_files_not_saved[ avg_name ] = true;
   
   f_pos       [ avg_name ] = f_qs.size();
   f_qs_string [ avg_name ] = avg_qs_string;
   f_qs        [ avg_name ] = avg_qs;
   f_Is        [ avg_name ] = avg_Is;
   f_errors    [ avg_name ] = avg_sd;
   
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

bool US_Hydrodyn_Saxs_Buffer::all_selected_have_nonzero_conc()
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
      if ( lb_files->item( i )->isSelected()  && 
           lb_files->item( i )->text() != lbl_buffer->text() &&
           lb_files->item( i )->text() != lbl_empty->text() )
      {
         selected_count++;
         if ( !nonzero_concs.count( lb_files->item( i )->text() ) )
         {
            return false;
         }
      }
   }

   return selected_count > 1;
}

void US_Hydrodyn_Saxs_Buffer::delete_zoomer_if_ranges_changed()
{
}

void US_Hydrodyn_Saxs_Buffer::show_created()
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

void US_Hydrodyn_Saxs_Buffer::show_only_created()
{
   disable_updates = true;
   lb_files->clearSelection();
   show_created();
}


QString US_Hydrodyn_Saxs_Buffer::vector_double_to_csv( vector < double > vd )
{
   QString result;
   for ( unsigned int i = 0; i < vd.size(); i++ )
   {
      result += QString("%1,").arg(vd[i]);
   }
   return result;
}

void US_Hydrodyn_Saxs_Buffer::rescale()
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
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         //any_selected = true;
         if ( get_min_max( lb_files->item( i )->text(), file_minx, file_maxx, file_miny, file_maxy ) )
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
#if QT_VERSION < 0x040000
   plot_dist_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
   connect( plot_dist_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_zoomed( const QRectF & ) ) );
   
   legend_set();
   plot_dist->replot();
   update_enables();
}

bool US_Hydrodyn_Saxs_Buffer::adjacent_ok( QString name )
{
   if ( name.contains( "_bsub_a" ) ||
        name.contains( QRegExp( "\\d+$" ) ) )
   {

      return true;
   }
   return false;
}

// void US_Hydrodyn_Saxs_Buffer::adjacent()
// {
//    us_qdebug( "adjacent" );
//    QString match_name;
//    int     match_pos = 0;
//    QStringList turn_on;

//    disable_all();

//    us_qdebug( "adjacent 0" );
//    for ( int i = 0; i < lb_files->count(); i++ )
//    {
//       if ( lb_files->item( i )->isSelected() )
//       {
//          match_name = lb_files->item( i )->text();
//          turn_on << match_name;
//          match_pos = i;
//          break;
//       }
//    }
//    us_qdebug( "adjacent 1" );

//    QRegExp rx;

//    bool found = false;
//    // if we have bsub
//    if ( match_name.contains( "_bsub_a" ) )
//    {
//       found = true;
//       rx.setPattern(
//                     QString( "^%1" )
//                     .arg( match_name )
//                     .replace( QRegExp( "_bsub_a.*$" ), "" )
//                     .replace( QRegExp( "\\d+$" ), "\\d+" )
//                     + 
//                     QString( "%1$" )
//                     .arg( match_name )
//                     .replace( QRegExp( "^.*_bsub" ), "_bsub" ) 
//                     );
//    }

//    us_qdebug( "adjacent 2" );
//    if ( !found && match_name.contains( QRegExp( "_cn\\d+.*$" ) ) )
//    {
//       found = true;
//       rx.setPattern(
//                     QString( "^%1" )
//                     .arg( match_name )
//                     .replace( QRegExp( "_cn\\d+.*$" ), "" )
//                     );
//    }

//    us_qdebug( "adjacent 3" );
//    if ( !found && match_name.contains( QRegExp( "\\d+$" ) ) )
//    {
//       found = true;
//       rx.setPattern(
//                     QString( "^%1" )
//                     .arg( match_name )
//                     .replace( QRegExp( "\\d+$" ), "" ) 
//                     );
//    }

//    // cout << "rx: " << rx.pattern() << endl;

//    unsigned int newly_set = 0;
//    us_qdebug( "adjacent 4" );

//    if ( found )
//    {
//       disable_updates = true;
      
//       for ( int i = match_pos - 1; i >= 0; i-- )
//       {
//          if ( lb_files->item( i )->text().contains( rx ) )
//          {
//             if ( !lb_files->item( i )->isSelected() )
//             {
//                lb_files->item( i)->setSelected( true );
//                newly_set++;
//             }
//          }
//       }
      
//       for ( int i = match_pos + 1; i < lb_files->count(); i++ )
//       {
//          if ( lb_files->item( i )->text().contains( rx ) )
//          {
//             if ( !lb_files->item( i )->isSelected() )
//             {
//                lb_files->item( i)->setSelected( true );
//                newly_set++;
//             }
//          }
//       }
      
//       if ( !newly_set )
//       {
//          adjacent_select( lb_files, match_name );
//          return;
//       }
//       disable_updates = false;
//       update_files();
//    } else {
//       adjacent_select( lb_files, match_name );
//       return;
//    }      
//    update_enables();
// }

void US_Hydrodyn_Saxs_Buffer::adjacent_created()
{
   QString match_name;
   int     match_pos = 0;
   QStringList turn_on;

   disable_all();

   for ( int i = 0; i < lb_created_files->count(); i++ )
   {
      if ( lb_created_files->item( i )->isSelected() )
      {
         match_name = lb_created_files->item( i )->text();
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
         if ( lb_created_files->item( i )->text().contains( rx ) )
         {
            if ( !lb_created_files->item( i )->isSelected() )
            {
               lb_created_files->item( i)->setSelected( true );
               newly_set++;
            }
         }
      }
      
      for ( int i = match_pos + 1; i < lb_created_files->count(); i++ )
      {
         if ( lb_created_files->item( i )->text().contains( rx ) )
         {
            if ( !lb_created_files->item( i )->isSelected() )
            {
               lb_created_files->item( i)->setSelected( true );
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

bool US_Hydrodyn_Saxs_Buffer::adjacent_select( QListWidget *lb, QString match )
{
   bool ok;
   static QString last_match;
   if ( match.isEmpty() )
   {
      match = last_match;
   }

   match = US_Static::getText(
                                 windowTitle() + us_tr( ": Select by pattern" ), 
                                 us_tr( "Regular expression search\n"
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

   for ( int i = 0; i < lb->count(); i++ )
   {
      if ( lb->item( i )->text().contains( rx ) )
      {
         if ( !lb->item( i )->isSelected() )
         {
            lb->item( i)->setSelected( true );
            any_set = true;
         }
      }
   }
   disable_updates = false;
   update_files();
   update_enables();
   return any_set;
}

void US_Hydrodyn_Saxs_Buffer::join()
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
                                           title + us_tr( ": Join" ),
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
   if ( avg_conc )
   {
      f_conc      [ use_basename ] = avg_conc;
   }
   if ( avg_psv )
   {
      f_psv       [ use_basename ] = avg_psv;
   }
   if ( avg_I0se )
   {
      f_I0se      [ use_basename ] = avg_I0se;
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
}

void US_Hydrodyn_Saxs_Buffer::to_saxs()
{
   // copy selected to saxs window
   if ( !activate_saxs_window() )
   {
      return;
   }

   update_csv_conc();
   map < QString, double > concs = current_concs();

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         QString this_file = lb_files->item( i )->text();
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

            saxs_window->update_conc_csv( 
                                         saxs_window->qsl_plotted_iq_names.back(), 
                                         ( concs.count( this_file ) && concs[ this_file ] != 0e0 ) ? concs[ this_file ] : ((US_Hydrodyn *)us_hydrodyn)->saxs_options.conc
                                         );
            

         } else {
            editor_msg( "red", QString( us_tr( "Internal error: requested %1, but not found in data" ) ).arg( this_file ) );
         }
      }
   }
   saxs_window->rescale_plot();
}

void US_Hydrodyn_Saxs_Buffer::plot_zoomed( const QRectF & /* rect */ )
{
   //   cout << QString( "zoomed: %1 %2 %3 %4\n" )
   // .arg( rect.x1() )
   // .arg( rect.x2() )
   // .arg( rect.y1() )
   // .arg( rect.y2() );
   update_enables();
}


void US_Hydrodyn_Saxs_Buffer::zoom_info()
{
   if ( plot_dist_zoomer )
   {
#if QT_VERSION < 0x040000
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

void US_Hydrodyn_Saxs_Buffer::plot_mouse( const QMouseEvent & /* me */ )
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

void US_Hydrodyn_Saxs_Buffer::select_vis()
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
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( selected_files.count( lb_files->item( i )->text() ) )
      {
         lb_files->item( i)->setSelected( true );
      }
   }
   disable_updates = false;
   update_files();
}

void US_Hydrodyn_Saxs_Buffer::remove_vis()
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
   cout << "select visible\n";
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

void US_Hydrodyn_Saxs_Buffer::crop_left()
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
       maxy > plot_dist_zoomer->zoomRect().y2()
#else
       minx < plot_dist_zoomer->zoomRect().left()  ||
       maxx > plot_dist_zoomer->zoomRect().right() ||
       miny < plot_dist_zoomer->zoomRect().top()   ||
       maxy > plot_dist_zoomer->zoomRect().bottom()
#endif
       )
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
         cout << QString( "just move to %1 %2\n" ).arg( newx ).arg( newy );
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

void US_Hydrodyn_Saxs_Buffer::crop_right()
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
         cout << QString( "just move to %1 %2\n" ).arg( newx ).arg( newy );
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

void US_Hydrodyn_Saxs_Buffer::crop_undo()
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

void US_Hydrodyn_Saxs_Buffer::guinier()
{
}

void US_Hydrodyn_Saxs_Buffer::crop_common()
{
   map < QString, bool > selected_files;

   vector < vector < double > > grids;

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         QString this_file = lb_files->item( i )->text();
         if ( f_qs.count( this_file ) &&
              f_Is.count( this_file ) &&
              f_qs[ this_file ].size() &&
              f_Is[ this_file ].size() )
         {
            selected_files[ this_file ] = true;
            grids.push_back( f_qs[ lb_files->item( i )->text() ] );
         }
      }
   }

   vector < double > v_union = US_Vector::vunion( grids );
   vector < double > v_int   = US_Vector::intersection( grids );

   editor_msg( "black", 
               QString( us_tr( "Crop common:\n"
                            "Current selected files have a maximal q-range of (%1:%2) with %3 points\n"
                            "Current selected files have a common  q-range of (%4:%5) with %6 points\n"
                            ) )
               .arg( v_union.size() ? QString( "%1" ).arg( v_union[ 0 ] ) : QString( "empty" ) )
               .arg( v_union.size() ? QString( "%1" ).arg( v_union.back() ) : QString( "empty" ) )
               .arg( v_union.size() )
               .arg( v_int.size() ? QString( "%1" ).arg( v_int[ 0 ] ) : QString( "empty" ) )
               .arg( v_int.size() ? QString( "%1" ).arg( v_int.back() ) : QString( "empty" ) )
               .arg( v_int.size() )
               );

   bool any_differences = v_union != v_int;

   if ( !any_differences )
   {
      editor_msg( "black", us_tr( "Crop common: no differences between selected grids" ) );
      return;
   }

   if ( !v_int.size() )
   {
      editor_msg( "black", us_tr( "Crop common: grids have no common points" ) );
      return;
   }

   map < double, bool > map_int;
   for ( unsigned int i = 0; i < ( unsigned int )v_int.size(); i++ )
   {
      map_int[ v_int[ i ] ] = true;
   }

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
         if ( map_int.count( f_qs[ it->first ][ i ] ) )
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
   editor_msg( "blue", us_tr( "Crop common: done" ) );

   update_files();
}

void US_Hydrodyn_Saxs_Buffer::view()
{
   int dsp_count = 0;

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         QString file = lb_files->item( i )->text();

         QString text;

         text += QString( title + us_tr( " output: %1\n" ) ).arg( file );

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
                                          f_qs_string[ file ][ i ].toLatin1().data(),
                                          f_Is       [ file ][ i ],
                                          f_errors   [ file ][ i ] );
            } else {
               text += QString("").sprintf( "%-18s\t%.6e\n",
                                          f_qs_string[ file ][ i ].toLatin1().data(),
                                          f_Is       [ file ][ i ] );
            }
         }

#if QT_VERSION < 0x040000
         TextEdit *edit;
         edit = new TextEdit( this, qPrintable( file ) );
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
#else
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
#endif
      }
   }
}

bool US_Hydrodyn_Saxs_Buffer::is_nonzero_vector( vector < double > &v )
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

void US_Hydrodyn_Saxs_Buffer::to_created( QString file )
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

void US_Hydrodyn_Saxs_Buffer::crop_vis()
{
   // find curves within zoomRect & select only them
#if QT_VERSION < 0x040000
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
      editor_msg( "red", us_tr( "Crop visible: The current visible plot is empty" ) );
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
         editor_msg( "red", us_tr( "Crop visible: error: you can not crop out the middle of a curve" ) );
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
            editor_msg( "dark red", us_tr( "Crop visible: warning: you are cropping the left of some curves and the right of others. " ) );
            break;
         }
      }
   }
   
   editor_msg( "black",
               QString( us_tr( "Crop visible:\n"
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
   editor_msg( "blue", us_tr( "Crop visible: done" ) );

   update_files();

   if ( plot_dist_zoomer &&
        plot_dist_zoomer->zoomRectIndex() )
   {
      plot_dist_zoomer->zoom( -1 );
   }
}

void US_Hydrodyn_Saxs_Buffer::legend()
{
#if QT_VERSION < 0x040000
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

void US_Hydrodyn_Saxs_Buffer::axis_y()
{
   axis_y_log = !axis_y_log;

   if ( axis_y_log )
   {
      plot_dist->setAxisTitle(QwtPlot::yLeft, us_tr("I(q) (log scale)") );
#if QT_VERSION < 0x040000
      plot_dist->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::Logarithmic);
#else
      plot_dist->setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine(10));
#endif
   } else {
      plot_dist->setAxisTitle(QwtPlot::yLeft, us_tr("I(q)") );
#if QT_VERSION < 0x040000
      plot_dist->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::None);
#else
      // actually need to test this, not sure what the correct version is
      plot_dist->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine );
#endif
   }
   if ( plot_dist_zoomer )
   {
      plot_dist_zoomer->zoom ( 0 );
      delete plot_dist_zoomer;
      plot_dist_zoomer = (ScrollZoomer *) 0;
   }
   plot_files();
   plot_dist->replot();
}

void US_Hydrodyn_Saxs_Buffer::axis_x()
{
   axis_x_log = !axis_x_log;
   if ( axis_x_log )
   {
      plot_dist->setAxisTitle(QwtPlot::xBottom,  us_tr("q (1/Angstrom) (log scale)") );
#if QT_VERSION < 0x040000
      plot_dist->setAxisOptions(QwtPlot::xBottom, QwtAutoScale::Logarithmic);
#else
      plot_dist->setAxisScaleEngine(QwtPlot::xBottom, new QwtLogScaleEngine(10));
#endif
   } else {
      plot_dist->setAxisTitle(QwtPlot::xBottom,  us_tr("q (1/Angstrom)") );
#if QT_VERSION < 0x040000
      plot_dist->setAxisOptions(QwtPlot::xBottom, QwtAutoScale::None);
#else
      // actually need to test this, not sure what the correct version is
      plot_dist->setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine );
#endif
   }
   plot_dist->replot();
}

void US_Hydrodyn_Saxs_Buffer::legend_set()
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

void US_Hydrodyn_Saxs_Buffer::similar_files()
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

   QString similar = QFileInfo( f_name[ selected[ 0 ] ] ).fileName();
   QString dir     = QFileInfo( f_name[ selected[ 0 ] ] ).path();
   QString match   = similar;
   match.replace( QRegExp( "\\d{2,}" ), "\\d+" );

   cout << QString( "select to match <%1> in directory <%2> using regexp <%3>\n" )
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
      lbl_dir->setText( QDir::currentPath() );
   }
   QDir qd;
   add_files( qd.entryList( QStringList() << "*" ).filter( QRegExp( match ) ) );
}

void US_Hydrodyn_Saxs_Buffer::regex_load()
{
   // make list of lbl_dir all files
   // filter with regex for each # in list
   QString dir     = lbl_dir->text();

   QDir::setCurrent( dir );
   lbl_dir->setText( QDir::currentPath() );
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

void US_Hydrodyn_Saxs_Buffer::rename_from_context( const QPoint & pos ) {
#if QT_VERSION >= 0x040000
   QListWidgetItem * lwi = lb_created_files->itemAt( pos );
   if ( lwi ) {
      return rename_created( lwi, pos );
   }
#endif
}

void US_Hydrodyn_Saxs_Buffer::rename_created( QListWidgetItem *lbi, const QPoint & )
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

void US_Hydrodyn_Saxs_Buffer::normalize()
{
   QStringList files;
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() && 
           lb_files->item( i )->text() != lbl_buffer->text() &&
           lb_files->item( i )->text() != lbl_empty->text() )
      {
         files << lb_files->item( i )->text();
      }
   }

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

void US_Hydrodyn_Saxs_Buffer::add_plot( QString           name,
                                        vector < double > q,
                                        vector < double > I )
{
   vector < double > errors( I.size() );
   for ( unsigned int i = 0; i < ( unsigned int ) errors.size(); i++ )
   {
      errors[ i ] = 0e0;
   }
   add_plot( name, q, I, errors );
}

void US_Hydrodyn_Saxs_Buffer::add_plot( QString           name,
                                        vector < double > q,
                                        vector < double > I,
                                        vector < double > errors )
{
   name.replace( QRegExp( "(\\s+|\"|'|\\/|\\.)" ), "_" );
   
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
   
   // we could check if it has changed and then delete
   if ( plot_dist_zoomer )
   {
      delete plot_dist_zoomer;
      plot_dist_zoomer = (ScrollZoomer *) 0;
   }
   plot_files();
   update_enables();
}

void US_Hydrodyn_Saxs_Buffer::run_one_divide()
{
   // subtract buffer
   QString buffer   = lbl_buffer  ->text();
   QString solution = lbl_signal  ->text();
   QString empty    = lbl_empty   ->text();
   last_created_file = "";

   map < QString, bool > current_files;

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      QString this_file = lb_files->item( i )->text();
      current_files[ this_file ] = true;
   }

   if ( !current_files.count( buffer ) )
   {
      editor_msg( "red", QString( us_tr( "Error: no data found for buffer %1" ) ).arg( buffer ) );
      return;
   } 

   if ( !current_files.count( solution ) )
   {
      editor_msg( "red", QString( us_tr( "Error: no data found for solution %1" ) ).arg( solution ) );
      return;
   } 

   if ( !empty.isEmpty() && !current_files.count( empty ) )
   {
      editor_msg( "red", QString( us_tr( "Error: no data found for blank %1" ) ).arg( empty ) );
      return;
   } 

   if ( f_qs[ buffer ].size() != f_qs[ solution ].size() ||
        ( !empty.isEmpty() && f_qs[ buffer ].size() != f_qs[ empty ].size() ) )
   {
      editor_msg( "red", us_tr( "Error: incompatible grids, the files selected do not have the same number of points" ) );
      return;
   }

   vector < QString > bsub_q_string = f_qs_string [ solution ];
   vector < double >  bsub_q        = f_qs        [ solution ];
   vector < double >  bsub_I        = f_Is        [ solution ];
   vector < double >  bsub_error    = f_errors    [ solution ];

   bool solution_has_errors = f_errors[ solution ].size();
   bool buffer_has_errors   = f_errors[ buffer   ].size();
   bool empty_has_errors    = !empty.isEmpty() && f_errors[ empty ].size();

   for ( unsigned int j = 0; j < f_Is[ buffer ].size(); j++ )
   {
      if ( fabs( bsub_q[ j ] - f_qs[ buffer ][ j ] ) > Q_VAL_TOL ||
           ( !empty.isEmpty() && bsub_q[ j ] != f_qs[ empty ][ j ] ) )
      {
         editor_msg( "red", us_tr( "Error: incompatible grids, the q values differ between selected files" ) );
         return;
      }
   }
   
   // determine parameters
   bool use_alpha = ((QCheckBox *)(t_csv->cellWidget( 0, 1 )))->isChecked();
   bool use_psv   = ((QCheckBox *)(t_csv->cellWidget( 1, 1 )))->isChecked();

   if ( ( !use_alpha && !use_psv ) || ( use_alpha && use_psv ) )
   {
      editor_msg( "red", us_tr( "Internal error: both alpha & psv methods active" ) );
      return;
   }

   if ( ( use_alpha && t_csv->item( 0, 6 )->text().isEmpty() ) ||
        ( use_psv && ( t_csv->item( 1, 6 )->text().isEmpty() ||
                       t_csv->item( 2, 6 )->text().isEmpty() ) ) )
   {
      editor_msg( "red", us_tr( "Internal error: method selected does not have current values" ) );
      return;
   }
      
   double alpha  = t_csv->item( 0, 6 )->text().toDouble();
   double psv    = t_csv->item( 1, 6 )->text().toDouble();
   double gamma  = t_csv->item( 2, 6 )->text().toDouble();

   map < QString, double > concs = current_concs();
   double this_conc = concs.count( solution ) ? concs[ solution ] : 0e0;

   QString msg;
   QString tag;

   if ( use_psv )
   {
      if ( !concs.count( solution ) || concs[ solution ] == 0e0 )
      {
         editor_msg( "dark red", us_tr( "Warning: the solution has zero concentration" ) );
      }
         
      alpha = 1e0 - gamma * this_conc * psv / 1000;
      msg = QString( us_tr( "alpha %1 gamma %2 conc %3 psv %4" ) )
         .arg( alpha )
         .arg( gamma )
         .arg( this_conc )
         .arg( psv );
   } else {
      msg = QString( us_tr( "alpha %1" ) ).arg( alpha );
   }

   // assuming zero covariance for now
   if ( buffer_has_errors && !solution_has_errors )
   {
      editor_msg( "dark red", us_tr( "Warning: the buffer has errors defined but not the solution" ) );
      
      bsub_error = f_errors[ buffer ];
   } else {
      if ( !buffer_has_errors && !solution_has_errors && empty_has_errors )
      {
         // this is a strange case
         editor_msg( "dark red", us_tr( "Warning: the blank has errors defined but not the solution or buffer!" ) );
         bsub_error = f_errors[ empty ];
      }
   }

   for ( unsigned int i = 0; i < bsub_q.size(); i++ )
   {
      bsub_I[ i ] /= alpha * f_Is[ buffer ][ i ];
      if ( solution_has_errors && buffer_has_errors )
      {
         bsub_error[ i ] = sqrt( bsub_error[ i ] * bsub_error[ i ] +
                                 alpha * alpha * f_errors[ buffer ][ i ] * f_errors[ buffer ][ i ] );
      } else {
         if ( buffer_has_errors )
         {
            bsub_error[ i ] *= alpha;
         }
      }
            
      if ( !empty.isEmpty() )
      {
         bsub_I[ i ] -= ( 1e0 - alpha ) * f_Is[ empty ][ i ];
         if ( ( buffer_has_errors || solution_has_errors ) && empty_has_errors )
         {
            bsub_error[ i ] = sqrt( bsub_error[ i ] * bsub_error[ i ] +
                                    ( 1e0 - alpha ) * ( 1e0 - alpha )
                                    * f_errors[ empty ][ i ] * f_errors[ empty ][ i ] );
         }
      }
   }         

   // ok now we have a bsub!
   bool         any_negative   = false;
   unsigned int negative_pos   = 0;
   unsigned int negative_count = 0;
   double       minimum_positive = bsub_I[ 0 ];

   for ( unsigned int i = 0; i < bsub_I.size(); i++ )
   {
      if ( minimum_positive > bsub_I[ i ] )
      {
         minimum_positive = bsub_I[ i ];
      }
      if ( bsub_I[ i ] <= 0e0 )
      {
         negative_count++;
         if ( !any_negative )
         {
            negative_pos = i;
            any_negative = true;
         }
      }
   }

   if ( any_negative )
   {
      int result;
      if ( rb_np_crop->isChecked() )
      {
         editor_msg( "dark red", QString( us_tr( "Warning: non-positive values caused cropping at q = %1" ) ).arg( bsub_q[ negative_pos ] ) );
         result = 0;
      }
      if ( rb_np_min->isChecked() )
      {
         editor_msg( "dark red", QString( us_tr( "Warning: non-positive values caused %1 minimum values set" ) ).arg( negative_count ) );
         result = 1;
      }
      if ( rb_np_ignore->isChecked() )
      {
         editor_msg( "dark red", QString( us_tr( "Warning: %1 non-positive values ignored" ) ).arg( negative_count ) );
         result = 2;
      }
      if ( rb_np_ask->isChecked() )
      {
         result = QMessageBox::warning(this, 
                                       title,
                                       QString( us_tr( "Please note:\n\n"
                                                    "The buffer subtraction causes %1 points be non-positive\n"
                                                    "Starting at a q value of %2\n\n"
                                                    "What would you like to do?\n" ) )
                                       .arg( negative_count )
                                       .arg( bsub_q[ negative_pos ] ),
                                       us_tr( "&Crop the data" ), 
                                       us_tr( "&Set to the minimum positive value" ), 
                                       us_tr( "&Leave them negative or zero" ), 
                                       0, // Stop == button 0
                                       0 // Escape == button 0
                                       );
      }
         
      switch( result )
      {
      case 0 : // crop
         {
            if ( negative_pos < 2 )
            {
               if ( rb_np_ask->isChecked() )
               {
                  QMessageBox::warning(this, 
                                       title,
                                       us_tr("Insufficient data left after cropping"));
               } else {
                  editor_msg( "red", us_tr( "Notice: Cropping left nothing" ) );
               }
               return;
            }
               
            bsub_q_string.resize( negative_pos );
            bsub_q       .resize( negative_pos );
            bsub_I       .resize( negative_pos );
            if ( bsub_error.size() )
            {
               bsub_error.resize( negative_pos );
            }
         }
         break;
      case 1 : // use absolute value
         for ( unsigned int i = 0; i < bsub_I.size(); i++ )
         {
            if ( bsub_I[ i ] <= 0e0 )
            {
               bsub_I[ i ] = minimum_positive;
            }
         }
         break;
      case 2 : // ignore
         break;
      }
   }

   QString head = solution + QString( "_bdiv_a%1" ).arg( alpha ).replace( ".", "_" );
   unsigned int ext = 0;

   QString bsub_name = head;

   while ( current_files.count( bsub_name ) )
   {
      bsub_name = head + QString( "-%1" ).arg( ++ext );
   }

   lb_created_files->addItem( bsub_name );
   lb_created_files->scrollToItem( lb_created_files->item( lb_created_files->count() - 1 ) );
   lb_files->addItem( bsub_name );
   lb_files->scrollToItem( lb_files->item( lb_files->count() - 1 ) );
   created_files_not_saved[ bsub_name ] = true;
   last_created_file = bsub_name;
   
   f_pos       [ bsub_name ] = f_qs.size();
   f_qs_string [ bsub_name ] = bsub_q_string;
   f_qs        [ bsub_name ] = bsub_q;
   f_Is        [ bsub_name ] = bsub_I;
   f_errors    [ bsub_name ] = bsub_error;
   
   // we could check if it has changed and then delete
   if ( plot_dist_zoomer )
   {
      delete plot_dist_zoomer;
      plot_dist_zoomer = (ScrollZoomer *) 0;
   }
   plot_files();

   update_csv_conc();
   for ( unsigned int i = 0; i < csv_conc.data.size(); i++ )
   {
      if ( csv_conc.data[ i ].size() > 1 &&
           csv_conc.data[ i ][ 0 ] == bsub_name )
      {
         csv_conc.data[ i ][ 1 ] = QString( "%1" ).arg( this_conc );
      }
   }

   if ( conc_widget )
   {
      conc_window->refresh( csv_conc );
   }

   if ( !running )
   {
      update_enables();
   }
}

void US_Hydrodyn_Saxs_Buffer::crop_zero()
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
   if ( plot_dist_zoomer )
   {
      delete plot_dist_zoomer;
      plot_dist_zoomer = (ScrollZoomer *) 0;
   }
   plot_files();
}

void US_Hydrodyn_Saxs_Buffer::adjust_wheel( double pos )
{
   cout << QString("pos is now %1\n").arg(pos);

   if ( le_join_offset->hasFocus() )
   {
      cout << "aw: join offset\n";
      le_last_focus = le_join_offset;
   }
   if ( le_join_mult->hasFocus() )
   {
      cout << "aw: join mult\n";
      le_last_focus = le_join_mult;
   }
   if ( le_join_start->hasFocus() )
   {
      cout << "aw: join start\n";
      le_last_focus = le_join_start;
   }
   if ( le_join_point->hasFocus() )
   {
      cout << "aw: join point\n";
      le_last_focus = le_join_point;
   }
   if ( le_join_end->hasFocus() )
   {
      cout << "aw: join end\n";
      le_last_focus = le_join_end;
   }

   if ( !le_last_focus )
   {
      return;
   }

   le_last_focus->setText( QString( "%1" ).arg( pos ) );

   lbl_wheel_pos->setText( QString( "%1" ).arg( pos ) );

}

void US_Hydrodyn_Saxs_Buffer::join_offset_focus( bool hasFocus )
{
   cout << QString( "join_offset_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      le_last_focus = le_join_offset;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( join_offset_start, join_offset_end); qwtw_wheel->setSingleStep( join_offset_delta );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_join_offset->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs_Buffer::join_mult_focus( bool hasFocus )
{
   cout << QString( "join_mult_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      le_last_focus = le_join_mult;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( join_mult_start, join_mult_end); qwtw_wheel->setSingleStep( join_mult_delta );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_join_mult->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs_Buffer::join_start_focus( bool hasFocus )
{
   cout << QString( "join_start_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      le_last_focus = le_join_start;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( join_low_q, join_high_q); qwtw_wheel->setSingleStep( 0.00005 );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_join_start->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs_Buffer::join_point_focus( bool hasFocus )
{
   cout << QString( "join_point_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      le_last_focus = le_join_point;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( join_low_q, join_high_q); qwtw_wheel->setSingleStep( 0.00005 );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_join_point->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs_Buffer::join_end_focus( bool hasFocus )
{
   cout << QString( "join_end_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      le_last_focus = le_join_end;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( join_low_q, join_high_q); qwtw_wheel->setSingleStep( 0.00005 );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_join_end->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}


void US_Hydrodyn_Saxs_Buffer::wheel_cancel()
{
   lbl_wheel_pos->setText( QString( "%1" ).arg( 0 ) );
#if QT_VERSION < 0x040000
   plot_dist->setCurveData( wheel_curve, 
                            /* cb_guinier->isChecked() ? (double *)&(plotted_q2[p][0]) : */
                            (double *)&( f_qs[ wheel_file ][ 0 ] ),
                            (double *)&( f_Is[ wheel_file ][ 0 ] ),
                            f_qs[ wheel_file ].size()
                            );
   plot_dist->setCurveData( join_curve, 
                            /* cb_guinier->isChecked() ? (double *)&(plotted_q2[p][0]) : */
                            (double *)&( f_qs[ join_file ][ 0 ] ),
                            (double *)&( f_Is[ join_file ][ 0 ] ),
                            f_qs[ join_file ].size()
                            );
#else
   wheel_curve->setSamples(
                        /* cb_guinier->isChecked() ?
                           (double *)&(plotted_q2[p][0]) : */
                        (double *)&( f_qs[ wheel_file ][ 0 ] ),
                        (double *)&( f_Is[ wheel_file ][ 0 ] ),
                        f_qs[ wheel_file ].size()
                        );
   join_curve->setSamples(
                       /* cb_guinier->isChecked() ?
                          (double *)&(plotted_q2[p][0]) : */
                       (double *)&( f_qs[ join_file ][ 0 ] ),
                       (double *)&( f_Is[ join_file ][ 0 ] ),
                       f_qs[ join_file ].size()
                        );
#endif

   join_delete_markers();

   if ( plotted_curves.count( wheel_file ) &&
        f_pos.count( wheel_file ) )
   {
#if QT_VERSION < 0x040000
      plot_dist->setCurvePen( plotted_curves[ wheel_file ], QPen( plot_colors[ f_pos[ wheel_file ] % plot_colors.size()], 1, SolidLine));
#else
      plotted_curves[ wheel_file ]->setPen( QPen( plot_colors[ f_pos[ wheel_file ] % plot_colors.size() ], 1, Qt::SolidLine ) );
#endif
   }

   if ( plotted_curves.count( join_file ) &&
        f_pos.count( join_file ) )
   {
#if QT_VERSION < 0x040000
      plot_dist->setCurvePen( plotted_curves[ join_file ], QPen( plot_colors[ f_pos[ join_file ] % plot_colors.size()], 1, SolidLine));
#else
      plotted_curves[ join_file ]->setPen( QPen( plot_colors[ f_pos[ join_file ] % plot_colors.size() ], 1, Qt::SolidLine ) );
#endif
   }

   plot_dist->replot();

   disable_all();

   qwtw_wheel            ->setEnabled( false );
   pb_wheel_save         ->setEnabled( false );
   pb_wheel_cancel       ->setEnabled( false );

   pb_add_files          ->setEnabled( true );

   lb_files              ->setEnabled( true );
   lb_created_files      ->setEnabled( true );

   running               = false;

   update_enables();
}

void US_Hydrodyn_Saxs_Buffer::join_offset_text( const QString & text )
{
   if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
   join_do_replot();
}

void US_Hydrodyn_Saxs_Buffer::join_do_replot()
{
   vector < double > offset_I = f_Is[ join_adjust_lowq ? wheel_file : join_file ];
   double pos  = le_join_offset->text().toDouble();
   double mult = le_join_mult  ->text().toDouble();
   for ( unsigned int i = 0; i < ( unsigned int ) offset_I.size(); i++ )
   {
      offset_I[ i ] = mult * offset_I[ i ] + pos;
   }
#if QT_VERSION < 0x040000
   plot_dist->setCurveData( join_adjust_lowq ? wheel_curve : join_curve, 
                            /* cb_guinier->isChecked() ? (double *)&(plotted_q2[p][0]) : */
                            (double *)&( f_qs[ wheel_file ][ 0 ] ),
                            (double *)&( offset_I[ 0 ] ),
                            offset_I.size()
                            );
#else
   ( join_adjust_lowq ? wheel_curve : join_curve )->setSamples(
                                                            /* cb_guinier->isChecked() ?
                                                               (double *)&(plotted_q2[p][0]) : */
                                                            (double *)&( f_qs[ wheel_file ][ 0 ] ),
                                                            (double *)&( offset_I[ 0 ] ),
                                                            offset_I.size()
                                                            );
#endif
   plot_dist->replot();
}


void US_Hydrodyn_Saxs_Buffer::join_mult_text( const QString & text )
{
   if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
   join_do_replot();
}

void US_Hydrodyn_Saxs_Buffer::join_start_text( const QString & text )
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
   replot_join();
   plot_dist->replot();
}

void US_Hydrodyn_Saxs_Buffer::join_point_text( const QString & text )
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
   replot_join();
   plot_dist->replot();
}

void US_Hydrodyn_Saxs_Buffer::join_end_text( const QString & text )
{
#if QT_VERSION < 0x040000
   plot_dist->setMarkerPos( plotted_markers[ 2 ], text.toDouble(), 0e0 );
#else
   plotted_markers[ 2 ]->setXValue( text.toDouble() );
#endif
   if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
   replot_join();
   plot_dist->replot();
}


void US_Hydrodyn_Saxs_Buffer::disable_all()
{
   pb_similar_files      ->setEnabled( false );
   pb_conc               ->setEnabled( false );
   pb_clear_files        ->setEnabled( false );
   pb_asum               ->setEnabled( false );
   pb_avg                ->setEnabled( false );
   pb_normalize          ->setEnabled( false );
   pb_conc_avg           ->setEnabled( false );
   pb_set_buffer         ->setEnabled( false );
   pb_set_signal         ->setEnabled( false );
   pb_set_empty          ->setEnabled( false );
   pb_select_all         ->setEnabled( false );
   pb_invert             ->setEnabled( false );
   // pb_join               ->setEnabled( false );
   // pb_adjacent           ->setEnabled( false );
   pb_select_nth         ->setEnabled( false );
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

   lb_files              ->setEnabled( false );
   lb_created_files      ->setEnabled( false );

   qwtw_wheel            ->setEnabled( false );
   pb_wheel_save         ->setEnabled( false );
   pb_wheel_cancel       ->setEnabled( false );

   pb_join_start         ->setEnabled( false );
   pb_join_swap          ->setEnabled( false );
   le_join_offset        ->setEnabled( false );
   le_join_mult          ->setEnabled( false );
   le_join_start         ->setEnabled( false );
   le_join_point         ->setEnabled( false );
   le_join_end           ->setEnabled( false );
   pb_join_fit_scaling   ->setEnabled( false );
   pb_join_fit_linear    ->setEnabled( false );

   pb_color_rotate       ->setEnabled( false );
}


void US_Hydrodyn_Saxs_Buffer::join_enables()
{
   pb_join_start         ->setEnabled( false );
   pb_join_swap          ->setEnabled( true  );
   pb_wheel_cancel       ->setEnabled( true  );
   pb_wheel_save         ->setEnabled( true  );
   le_join_offset        ->setEnabled( true  );
   le_join_mult          ->setEnabled( true  );
   le_join_start         ->setEnabled( true  );
   le_join_point         ->setEnabled( true  );
   le_join_end           ->setEnabled( true  );
   pb_join_fit_scaling   ->setEnabled( true  );
   // pb_join_fit_linear    ->setEnabled( true  );
   qwtw_wheel            ->setEnabled( 
                                      le_join_offset->hasFocus() || 
                                      le_join_mult  ->hasFocus() || 
                                      le_join_start ->hasFocus() || 
                                      le_join_point ->hasFocus() || 
                                      le_join_end   ->hasFocus() 
                                      );
}

void US_Hydrodyn_Saxs_Buffer::join_fit_linear()
{
   // needs common grid or interpolation!

   vector < double > wheel_t;
   vector < double > wheel_y;
   vector < double > wheel_e;

   vector < double > join_t;
   vector < double > join_y;
   vector < double > join_e;

   double join_start = le_join_start->text().toDouble();
   double join_end   = le_join_end  ->text().toDouble();

   bool use_errors = 
      f_errors.count( join_file ) && 
      f_errors[ join_file ].size() > 0 &&
      f_errors.count( wheel_file ) && 
      f_errors[ wheel_file ].size() > 0;

   for ( unsigned int i = 0; i < ( unsigned int ) f_qs[ wheel_file ].size(); i++ )
   {
      if ( f_qs[ wheel_file ][ i ] >= join_start &&
           f_qs[ wheel_file ][ i ] >= join_end )
      {
         wheel_t.push_back( f_qs[ wheel_file ][ i ] );
         wheel_y.push_back( f_Is[ wheel_file ][ i ] );
         if ( use_errors )
         {
            wheel_e.push_back( f_errors[ wheel_file ][ i ] );
         }
      }
   }

   for ( unsigned int i = 0; i < ( unsigned int ) f_qs[ join_file ].size(); i++ )
   {
      if ( f_qs[ join_file ][ i ] >= join_start &&
           f_qs[ join_file ][ i ] >= join_end )
      {
         join_t.push_back( f_qs[ join_file ][ i ] );
         join_y.push_back( f_Is[ join_file ][ i ] );
         if ( use_errors )
         {
            join_e.push_back( f_errors[ join_file ][ i ] );
         }
      }
   }

   if ( wheel_t != join_t )
   {
      editor_msg( "red", "not yet supported: incompatible grids" );
      return;
   }

   US_Saxs_Util usu;
   double a;
   double b;
   double siga;
   double sigb;
   double chi2;

   usu.linear_fit( wheel_y, join_y, a, b, siga, sigb, chi2 );
   le_join_offset->setText( QString( "%1" ).arg( a ) );
   le_join_mult  ->setText( QString( "%1" ).arg( b ) );

   replot_join();
}

void US_Hydrodyn_Saxs_Buffer::join_fit_scaling()
{
   // needs common grid or interpolation!

   vector < double > wheel_t;
   vector < double > wheel_y;
   vector < double > wheel_e;

   vector < double > join_t;
   vector < double > join_y;
   vector < double > join_e;

   double join_start = le_join_start->text().toDouble();
   double join_end   = le_join_end  ->text().toDouble();

   bool use_errors = 
      f_errors.count( join_file ) && 
      f_errors[ join_file ].size() > 0 &&
      f_errors.count( wheel_file ) && 
      f_errors[ wheel_file ].size() > 0;

   for ( unsigned int i = 0; i < ( unsigned int ) f_qs[ wheel_file ].size(); i++ )
   {
      if ( f_qs[ wheel_file ][ i ] >= join_start &&
           f_qs[ wheel_file ][ i ] >= join_end )
      {
         wheel_t.push_back( f_qs[ wheel_file ][ i ] );
         wheel_y.push_back( f_Is[ wheel_file ][ i ] );
         if ( use_errors )
         {
            wheel_e.push_back( f_errors[ wheel_file ][ i ] );
         }
      }
   }

   for ( unsigned int i = 0; i < ( unsigned int ) f_qs[ join_file ].size(); i++ )
   {
      if ( f_qs[ join_file ][ i ] >= join_start &&
           f_qs[ join_file ][ i ] >= join_end )
      {
         join_t.push_back( f_qs[ join_file ][ i ] );
         join_y.push_back( f_Is[ join_file ][ i ] );
         if ( use_errors )
         {
            join_e.push_back( f_errors[ join_file ][ i ] );
         }
      }
   }

   if ( wheel_t != join_t )
   {
      editor_msg( "red", "not yet supported: incompatible grids" );
      return;
   }

   US_Saxs_Util usu;
   double a;
   double chi2;

   if ( join_adjust_lowq )
   {
      if ( use_errors )
      {
         if ( !usu.scaling_fit( wheel_y, join_y, wheel_e, a, chi2 ) )
         {
            editor_msg( "red", "not Fit error: " + usu.errormsg );
            return;
         }
         
      } else {
         if ( !usu.scaling_fit( wheel_y, join_y, a, chi2 ) )
         {
            editor_msg( "red", "not Fit error: " + usu.errormsg );
            return;
         }
      }
   } else {
      if ( use_errors )
      {
         if ( !usu.scaling_fit( join_y, wheel_y, join_e, a, chi2 ) )
         {
            editor_msg( "red", "not Fit error: " + usu.errormsg );
            return;
         }
         
      } else {
         if ( !usu.scaling_fit( join_y, wheel_y, a, chi2 ) )
         {
            editor_msg( "red", "not Fit error: " + usu.errormsg );
            return;
         }
      }
   }

   le_join_offset->setText( "0" );
   le_join_mult  ->setText( QString( "%1" ).arg( a ) );

   replot_join();
}

void US_Hydrodyn_Saxs_Buffer::join_swap()
{

#if QT_VERSION < 0x040000
   plot_dist->setCurveData( wheel_curve, 
                            /* cb_guinier->isChecked() ? (double *)&(plotted_q2[p][0]) : */
                            (double *)&( f_qs[ wheel_file ][ 0 ] ),
                            (double *)&( f_Is[ wheel_file ][ 0 ] ),
                            f_qs[ wheel_file ].size()
                            );
   plot_dist->setCurveData( join_curve, 
                            /* cb_guinier->isChecked() ? (double *)&(plotted_q2[p][0]) : */
                            (double *)&( f_qs[ join_file ][ 0 ] ),
                            (double *)&( f_Is[ join_file ][ 0 ] ),
                            f_qs[ join_file ].size()
                            );
#else
   wheel_curve->setSamples(
                        /* cb_guinier->isChecked() ?
                           (double *)&(plotted_q2[p][0]) : */
                        (double *)&( f_qs[ wheel_file ][ 0 ] ),
                        (double *)&( f_Is[ wheel_file ][ 0 ] ),
                        f_qs[ wheel_file ].size()
                        );
   join_curve->setSamples(
                       /* cb_guinier->isChecked() ?
                          (double *)&(plotted_q2[p][0]) : */
                       (double *)&( f_qs[ join_file ][ 0 ] ),
                       (double *)&( f_Is[ join_file ][ 0 ] ),
                       f_qs[ join_file ].size()
                        );
#endif

   join_adjust_lowq = !join_adjust_lowq;
   pb_join_swap->setText( join_adjust_lowq ? us_tr("Scale high-q") : us_tr("Scale low-q") );
   // disconnect( le_join_offset, SIGNAL( textChanged( const QString & ) ) );
   // disconnect( le_join_mult  , SIGNAL( textChanged( const QString & ) ) );
   le_join_offset->setText( QString( "%1" ).arg( -le_join_offset->text().toDouble() ) );
   le_join_mult  ->setText( QString( "%1" ).arg( 1e0 / le_join_mult->text().toDouble() ) );
   // connect( le_join_offset, SIGNAL( textChanged( const QString & ) ), SLOT( join_offset_text( const QString & ) ) );
   // connect( le_join_mult  , SIGNAL( textChanged( const QString & ) ), SLOT( join_mult_text  ( const QString & ) ) );
   // replot_join();
}

void US_Hydrodyn_Saxs_Buffer::join_set_wheel_range()
{

   // find the range:

   double min_wheel_I = 0e0;
   double max_wheel_I = 0e0;
   double min_join_I  = 0e0;
   double max_join_I  = 0e0;

   bool any_set = false;

   for ( unsigned int i = 0; i < ( unsigned int ) f_qs[ wheel_file ].size(); i++ )
   {
      if ( f_qs[ wheel_file ][ i ] >= join_low_q &&
           f_qs[ wheel_file ][ i ] <= join_high_q )
      {
         if ( !any_set ||  min_wheel_I > f_Is[ wheel_file ][ i ] )
         {
            min_wheel_I = f_Is[ wheel_file ][ i ];
         }           
         if ( !any_set || max_wheel_I < f_Is[ wheel_file ][ i ] )
         {
            max_wheel_I = f_Is[ wheel_file ][ i ];
         }           
         any_set = true;
      }
   }
      
   any_set = false;

   for ( unsigned int i = 0; i < ( unsigned int ) f_qs[ join_file ].size(); i++ )
   {
      if ( f_qs[ join_file ][ i ] >= join_low_q &&
           f_qs[ join_file ][ i ] <= join_high_q )
      {
         if ( !any_set || min_join_I > f_Is[ join_file ][ i ] )
         {
            min_join_I = f_Is[ join_file ][ i ];
         }           
         if ( !any_set || max_join_I < f_Is[ join_file ][ i ] )
         {
            max_join_I = f_Is[ join_file ][ i ];
         }           
         any_set = true;
      }
   }
      
   // maybe recompute based upon range

   if ( join_adjust_lowq )
   {
      join_offset_start = min_join_I - max_wheel_I;
      join_offset_end   = max_join_I - min_wheel_I;
   } else {
      join_offset_start = min_wheel_I - max_join_I;
      join_offset_end   = max_wheel_I - min_join_I;
   }      

   join_offset_delta = ( join_offset_end - join_offset_start ) / 10000000e0;

   join_mult_start = 1e-1;
   join_mult_end   = 1e1;
   join_mult_delta = ( join_mult_end - join_mult_start ) / 10000000e0;

   cout << QString(
                   "join vals:\n"
                   "           join_I %1 %2\n"
                   "          wheel_I %3 %4\n"
                   "     offset range %5 %6\n"
                   "            delta  %7\n" )
      .arg( min_join_I )
      .arg( max_join_I )
      .arg( min_wheel_I )
      .arg( max_wheel_I )
      .arg( join_offset_start )
      .arg( join_offset_end )
      .arg( join_offset_delta )
      ;
      
   if ( le_join_offset->text().isEmpty() )
   {
      disconnect( le_join_offset, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_join_offset->setText( "0" );
      connect( le_join_offset, SIGNAL( textChanged( const QString & ) ), SLOT( join_offset_text( const QString & ) ) );
   }

   if ( le_join_mult->text().isEmpty() )
   {
      disconnect( le_join_mult, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_join_mult->setText( "1" );
      connect( le_join_mult, SIGNAL( textChanged( const QString & ) ), SLOT( join_mult_text( const QString & ) ) );
   }

   if ( le_join_start->text().isEmpty() ||
        le_join_start->text().toDouble() < join_low_q )
   {
      disconnect( le_join_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_join_start->setText( QString( "%1" ).arg( join_low_q ) );
      connect( le_join_start, SIGNAL( textChanged( const QString & ) ), SLOT( join_start_text( const QString & ) ) );
   }

   if ( le_join_point->text().isEmpty() ||
        le_join_point->text().toDouble() < join_low_q )
   {
      disconnect( le_join_point, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_join_point->setText( QString( "%1" ).arg( ( join_low_q + join_high_q / 2 ) ) );
      connect( le_join_point, SIGNAL( textChanged( const QString & ) ), SLOT( join_point_text( const QString & ) ) );
   }

   if ( le_join_end->text().isEmpty() ||
        le_join_end->text().toDouble() > join_high_q )
   {
      cout << "setting join end\n";
      disconnect( le_join_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_join_end->setText( QString( "%1" ).arg( join_high_q ) );
      connect( le_join_end, SIGNAL( textChanged( const QString & ) ), SLOT( join_end_text( const QString & ) ) );
   }
}

void US_Hydrodyn_Saxs_Buffer::join_start()
{
   QStringList selected_files;
   lbl_wheel_pos->setText( QString( "%1" ).arg( 0 ) );
   pb_join_swap->setText( join_adjust_lowq ? us_tr("Scale low-q") : us_tr("Scale high-q") );

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         selected_files << lb_files->item( i )->text();
      }
   }

   if ( selected_files.size() != 2 )
   {
      editor_msg( "red", us_tr( "Internal error: not exactly 2 curves to join" ) );
      return;
   }

   bool ok;

   wheel_file = US_Static::getItem(
                                      us_tr( "SOMO: Buffer join: select file" ),
                                      us_tr("Select the low-q valued curve:\n" ),
                                      selected_files, 
                                      0, 
                                      false, 
                                      &ok,
                                      this );
   if ( !ok ) {
      return;
   }

   join_file = selected_files[ selected_files[ 0 ] == wheel_file ? 1 : 0 ];
      
   le_last_focus = (mQLineEdit *) 0;
   
   for ( unsigned int i = 0; i < ( unsigned int ) selected_files.size(); i++ )
   {
      if ( !plotted_curves.count( selected_files[ i ] ) )
      {
         editor_msg( "red", QString( us_tr( "Internal error: curve %1 not found in data" ) ).arg( selected_files[ i ] ) );
         return;
      }

      if ( !f_qs.count( selected_files[ i ] ) )
      {
         editor_msg( "red", QString( us_tr( "Internal error: %1 not found in data" ) ).arg( selected_files[ i ] ) );
         return;
      }

      if ( f_qs[ selected_files[ i ] ].size() < 2 )
      {
         editor_msg( "red", QString( us_tr( "Internal error: %1 almost empty data" ) ).arg( selected_files[ i ] ) );
         return;
      }

      if ( !f_Is.count( selected_files[ i ] ) )
      {
         editor_msg( "red", QString( us_tr( "Internal error: %1 not found in y data" ) ).arg( selected_files[ i ] ) );
         return;
      }

      if ( !f_Is[ selected_files[ i ] ].size() )
      {
         editor_msg( "red", QString( us_tr( "Internal error: %1 empty y data" ) ).arg( selected_files[ i ] ) );
         return;
      }
   }

   wheel_curve           = plotted_curves[ wheel_file ];
   join_curve            = plotted_curves[ join_file ];

#if QT_VERSION < 0x040000
   plot_dist->setCurvePen( plotted_curves[ wheel_file ], QPen( Qt::cyan  , 1, SolidLine));
   plot_dist->setCurvePen( plotted_curves[ join_file  ], QPen( Qt::yellow, 1, SolidLine));
#else
   plotted_curves[ wheel_file ]->setPen( QPen( Qt::cyan  , 1, Qt::SolidLine ) );
   plotted_curves[ join_file  ]->setPen( QPen( Qt::yellow, 1, Qt::SolidLine ) );
#endif

   join_low_q = 
      f_qs[ wheel_file ][ 0 ] > f_qs[ join_file ][ 0 ] ?
      f_qs[ wheel_file ][ 0 ] : f_qs[ join_file ][ 1 ];

   join_high_q = 
      f_qs[ wheel_file ].back() < f_qs[ join_file ].back() ?
      f_qs[ wheel_file ].back() : f_qs[ join_file ].back();

   if ( join_low_q >= join_high_q )
   {
      editor_msg( "red", QString( us_tr( "Error: no overlap for join" ) ) );
      return;
   }
      
   running       = true;

   join_set_wheel_range();

   join_init_markers();
   replot_join();
   disable_all();
   join_enables();
}


void US_Hydrodyn_Saxs_Buffer::join_delete_markers()
{
#if QT_VERSION < 0x040000
   plot_dist->removeMarkers();
#else
   plot_dist->detachItems( QwtPlotItem::Rtti_PlotMarker );
#endif
}

void US_Hydrodyn_Saxs_Buffer::join_init_markers()
{
   join_delete_markers();

   plotted_markers.clear( );

   join_add_marker( le_join_start->text().toDouble(), Qt::red, us_tr( "Start" ) );
   join_add_marker( le_join_point->text().toDouble(), Qt::magenta, us_tr( "Join point" ) );
   join_add_marker( le_join_end  ->text().toDouble(), Qt::red, us_tr( "End"   ), Qt::AlignLeft | Qt::AlignTop );

   plot_dist->replot();
}

void US_Hydrodyn_Saxs_Buffer::join_add_marker( double pos, 
                                               QColor color, 
                                               QString text, 
#if QT_VERSION < 0x040000
                                               int 
#else
                                               Qt::Alignment
#endif
                                               align )
{
#if QT_VERSION < 0x040000
   long marker = plot_dist->insertMarker();
   plot_dist->setMarkerLineStyle ( marker, QwtMarker::VLine );
   plot_dist->setMarkerPos       ( marker, pos, 0e0 );
   plot_dist->setMarkerLabelAlign( marker, align );
   plot_dist->setMarkerPen       ( marker, QPen( color, 2, DashDotDotLine));
   plot_dist->setMarkerFont      ( marker, QFont("Helvetica", 11, QFont::Bold));
   plot_dist->setMarkerLabelText ( marker, text );
#else
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
   marker->attach             ( plot_dist );
#endif
   plotted_markers.push_back( marker );
}   

void US_Hydrodyn_Saxs_Buffer::replot_join()
{
   // recompute rmsd and display ?
   vector < double > wheel_t;
   vector < double > wheel_y;
   vector < double > wheel_e;

   vector < double > join_t;
   vector < double > join_y;
   vector < double > join_e;

   double join_start = le_join_start->text().toDouble();
   double join_end   = le_join_end  ->text().toDouble();

   double offset  = le_join_offset->text().toDouble();
   double mult    = le_join_mult  ->text().toDouble();

   bool use_errors = 
      f_errors.count( join_file ) && 
      f_errors[ join_file ].size() > 0 &&
      f_errors.count( wheel_file ) && 
      f_errors[ wheel_file ].size() > 0;

   if ( join_adjust_lowq )
   {
      for ( unsigned int i = 0; i < ( unsigned int ) f_qs[ wheel_file ].size(); i++ )
      {
         if ( f_qs[ wheel_file ][ i ] >= join_start &&
              f_qs[ wheel_file ][ i ] >= join_end )
         {
            wheel_t.push_back( f_qs[ wheel_file ][ i ] );
            wheel_y.push_back( mult * f_Is[ wheel_file ][ i ] + offset );
            if ( use_errors )
            {
               // which one's errors to use?, what about compatible grids etc?
               // wheel_y.push_back( ( mult * f_Is[ wheel_file ][ i ] + offset ) / f_errors[ wheel_file ][ i ] );
               wheel_e.push_back( mult * f_errors[ wheel_file ][ i ] );
            }
         }
      }

      for ( unsigned int i = 0; i < ( unsigned int ) f_qs[ join_file ].size(); i++ )
      {
         if ( f_qs[ join_file ][ i ] >= join_start &&
              f_qs[ join_file ][ i ] >= join_end )
         {
            join_t.push_back( f_qs[ join_file ][ i ] );
            join_y.push_back( f_Is[ join_file ][ i ] );
            if ( use_errors )
            {
               join_e.push_back( f_errors[ join_file ][ i ] );
            }
         }
      }
   } else {
      for ( unsigned int i = 0; i < ( unsigned int ) f_qs[ wheel_file ].size(); i++ )
      {
         if ( f_qs[ wheel_file ][ i ] >= join_start &&
              f_qs[ wheel_file ][ i ] >= join_end )
         {
            wheel_t.push_back( f_qs[ wheel_file ][ i ] );
            wheel_y.push_back( f_Is[ wheel_file ][ i ] );
            if ( use_errors )
            {
               // which one's errors to use?, what about compatible grids etc?
               // nwheel_y.push_back( ( mult * f_Is[ wheel_file ][ i ] + offset ) / f_errors[ wheel_file ][ i ] );
               wheel_e.push_back( f_errors[ wheel_file ][ i ] );
            }
         }
      }

      for ( unsigned int i = 0; i < ( unsigned int ) f_qs[ join_file ].size(); i++ )
      {
         if ( f_qs[ join_file ][ i ] >= join_start &&
              f_qs[ join_file ][ i ] >= join_end )
         {
            join_t.push_back( f_qs[ join_file ][ i ] );
            join_y.push_back( mult * f_Is[ join_file ][ i ] + offset );
            if ( use_errors )
            {
               join_e.push_back( mult * f_errors[ join_file ][ i ] );
            }
         }
      }
   }

   lbl_join_rmsd->setText( QString( "%1" ).arg( US_Saxs_Util::calc_rmsd( join_y, wheel_y ) ) );
}

void US_Hydrodyn_Saxs_Buffer::wheel_save()
{
   disable_all();

   if ( le_join_offset->text().toDouble() != 0e0 )
   {
      QMessageBox::information( this,
                                title,
                                us_tr( 
                                   "You have a non-zero linear offset\n"
                                   "Quoting P. Vachette:\n"
                                   "This additional constant is to be avoided and if introduced,\n"
                                   "requires great care because it can hide an experimental problem\n"
                                   "which is thus swept under the carpet but could lead to erroneous interpretation." )
                                );
   }

   lbl_wheel_pos->setText( QString( "%1" ).arg( qwtw_wheel->value() ) );

   // join the adjusted curves and save

   map < QString, bool > current_files;

   int wheel_pos = -1;
   int join_pos = -1;

   for ( int i = 0; i < (int)lb_files->count(); i++ )
   {
      current_files[ lb_files->item( i )->text() ] = true;
      if ( lb_files->item( i )->text() == wheel_file )
      {
         wheel_pos = i;
      }
      if ( lb_files->item( i )->text() == join_file )
      {
         join_pos = i;
      }
   }

   QString save_name = join_file + "-" + wheel_file + 
      QString( "_j%1-%2-%3" )
      .arg( le_join_offset->text() )
      .arg( le_join_mult->text() )
      .arg( le_join_point->text() )
      .replace( ".", "_" );

   int ext = 0;
   while ( current_files.count( save_name ) )
   {
      save_name = join_file + "-" + wheel_file + 
      QString( "_j%1-%2-%3-%4" )
      .arg( le_join_offset->text() )
      .arg( le_join_mult->text() )
      .arg( le_join_point->text() )
      .arg( ++ext )
      .replace( ".", "_" );
   }
   
   cout << QString( "new name is %1\n" ).arg( save_name );

   vector < double >  q;
   vector < QString > q_string;
   vector < double >  I;
   vector < double >  errors;

   bool use_errors = 
      f_errors.count( join_file ) && 
      f_errors[ join_file ].size() > 0 &&
      f_errors.count( wheel_file ) && 
      f_errors[ wheel_file ].size() > 0;

   double offset = le_join_offset->text().toDouble();
   double mult   = le_join_mult  ->text().toDouble();
   double point  = le_join_point ->text().toDouble();

   bool was_equal_point = false;

   if ( join_adjust_lowq )
   {
      for ( unsigned int i = 0; i < ( unsigned int ) f_qs[ wheel_file ].size(); i++ )
      {
         if ( f_qs[ wheel_file ][ i ] <= point )
         {
            if ( f_qs[ wheel_file ][ i ] == point )
            {
               was_equal_point = true;
            }
            q.push_back( f_qs[ wheel_file ][ i ] );
            q_string.push_back( f_qs_string[ wheel_file ][ i ] );
            I.push_back( mult * f_Is[ wheel_file ][ i ] + offset );
            if ( use_errors )
            {
               errors.push_back( mult * f_errors[ wheel_file ][ i ] );
            }
         }
      }

      for ( unsigned int i = 0; i < ( unsigned int ) f_qs[ join_file ].size(); i++ )
      {
         if ( f_qs[ join_file ][ i ] > point ||
              ( !was_equal_point && f_qs[ join_file ][ i ] == point ) )
         {
            q.push_back( f_qs[ join_file ][ i ] );
            q_string.push_back( f_qs_string[ join_file ][ i ] );
            I.push_back( f_Is[ join_file ][ i ] );
            if ( use_errors )
            {
               errors.push_back( f_errors[ join_file ][ i ] );
            }
         }
      }
   } else {
      for ( unsigned int i = 0; i < ( unsigned int ) f_qs[ wheel_file ].size(); i++ )
      {
         if ( f_qs[ wheel_file ][ i ] <= point )
         {
            if ( f_qs[ wheel_file ][ i ] == point )
            {
               was_equal_point = true;
            }
            q.push_back( f_qs[ wheel_file ][ i ] );
            q_string.push_back( f_qs_string[ wheel_file ][ i ] );
            I.push_back( f_Is[ wheel_file ][ i ] );
            if ( use_errors )
            {
               errors.push_back( f_errors[ wheel_file ][ i ] );
            }
         }
      }

      for ( unsigned int i = 0; i < ( unsigned int ) f_qs[ join_file ].size(); i++ )
      {
         if ( f_qs[ join_file ][ i ] > point ||
              ( !was_equal_point && f_qs[ join_file ][ i ] == point ) )
         {
            q.push_back( f_qs[ join_file ][ i ] );
            q_string.push_back( f_qs_string[ join_file ][ i ] );
            I.push_back( mult * f_Is[ join_file ][ i ] + offset );
            if ( use_errors )
            {
               errors.push_back( mult * f_errors[ join_file ][ i ] );
            }
         }
      }
   }

   lb_created_files->addItem( save_name );

   lb_created_files->scrollToItem( lb_created_files->item( lb_created_files->count() - 1 ) );
   lb_files->addItem( save_name );
   lb_files->scrollToItem( lb_files->item( lb_files->count() - 1 ) );
   created_files_not_saved[ save_name ] = true;

   f_pos       [ save_name ] = f_qs.size();
   f_qs        [ save_name ] = q;
   f_qs_string [ save_name ] = q_string;
   f_Is        [ save_name ] = I;
   f_errors    [ save_name ] = errors;

   lb_files->item( f_pos[ save_name ])->setSelected( true );

   if ( wheel_pos != -1 )
   {
      lb_files->item( wheel_pos)->setSelected( false );
   }
   if ( join_pos != -1 )
   {
      lb_files->item( join_pos)->setSelected( false );
   }

   plot_dist->replot();

   pb_add_files          ->setEnabled( true );

   lb_files              ->setEnabled( true );
   lb_created_files      ->setEnabled( true );

   running               = false;

   update_enables();

}

void US_Hydrodyn_Saxs_Buffer::color_rotate()
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

void US_Hydrodyn_Saxs_Buffer::push_back_color_if_ok( QColor bg, QColor set )
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

void US_Hydrodyn_Saxs_Buffer::dir_pressed()
{
   QString use_dir = lbl_dir->text();
   ((US_Hydrodyn  *)us_hydrodyn)->select_from_directory_history( use_dir, this );
   QString s = QFileDialog::getExistingDirectory( this , us_tr( "Choose a new base directory" ) , use_dir , QFileDialog::ShowDirsOnly );

   if ( !s.isEmpty() )
   {
      QDir::setCurrent( s );
      lbl_dir->setText(  QDir::currentPath() );
      ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( s );
   }
}

void US_Hydrodyn_Saxs_Buffer::created_dir_pressed()
{
   QString use_dir = lbl_dir->text();
   ((US_Hydrodyn  *)us_hydrodyn)->select_from_directory_history( use_dir, this );
   QString s = QFileDialog::getExistingDirectory( this , us_tr( "Choose a new base directory for saving files" ) , use_dir , QFileDialog::ShowDirsOnly );

   if ( !s.isEmpty() )
   {
      lbl_created_dir->setText( s );
      ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( s );
   }
}

QStringList US_Hydrodyn_Saxs_Buffer::all_selected_files()
{
   QStringList files;
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         files << lb_files->item( i )->text();
      }
   }
   return files;
}

bool US_Hydrodyn_Saxs_Buffer::is_zero_vector( vector < double > &v )
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

void US_Hydrodyn_Saxs_Buffer::asum( QStringList files )
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
   double avg_I0se = 0e0;

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
      editor_msg( "dark red", QString( us_tr( "Notice: averaging requires cropping to %1 points" ) ).arg( min_q_len ) );
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
                  us_tr( "Notice: using standard deviation in mean calculation" ) );
   } else {
      editor_msg( "black" ,
                  us_tr( "Notice: NOT using standard deviation in mean calculation, since some sd's were zero or missing" ) );
   }

   vector < double > mults;
   // double mults_sum = 0e0;

   for ( int i = 0; i < (int)files.size(); i++ )
   {
      QString txt = QString( us_tr( "Enter a weight for %1" ) ).arg( files[ i ] );

      bool ok;
      double res = US_Static::getDouble(
                                           title + us_tr( ": Weighted sum" ),
                                           txt,
                                           1,
                                           1e-5,
                                           1e8,
                                           5,
                                           &ok,
                                           this
                                           );
      if ( ok ) {
         // user entered something and pressed OK
         mults.push_back( res );
         // mults_sum += res;
      } else {
         // user pressed Cancel
         return;
      }
   }      

   for ( int i = 0; i < (int)files.size(); i++ )
   {
      QString this_file = files[ i ];

      for ( int j = 0; j < (int) t_Is[ this_file ].size(); ++j )
      {
         t_Is[ this_file ][ j ] *= mults[ i ];
         if ( j < (int) t_errors[ this_file ].size() )
         {
            t_errors [ this_file ][ j ] *= mults[ i ];
         }
      }
   }

   vector < double > avg_sd( avg_qs.size() );

   for ( int i = 0; i < (int)files.size(); i++ )
   {
      QString this_file = files[ i ];

      selected_count++;
      selected_files << this_file;

      if ( first )
      {
         first = false;
         avg_qs_string = t_qs_string[ this_file ];
         avg_qs        = t_qs       [ this_file ];
         avg_Is        = t_Is       [ this_file ];
         avg_sd        = t_errors   [ this_file ];
         avg_conc = 
            concs.count( this_file ) ?
            mults[ i ] * concs[ this_file ] :
            0e0;
         avg_psv  = f_psv.count( this_file ) ? f_psv[ this_file ] : 0e0;
         avg_I0se = f_I0se.count( this_file ) ? f_I0se[ this_file ] : 0e0;

      } else {
         if ( avg_qs.size() != t_qs[ this_file ].size() )
         {
            editor_msg( "red", us_tr( "Error: incompatible grids, the files selected do not have the same number of points" ) );
            return;
         }
         for ( int j = 0; j < (int)t_Is[ this_file ].size(); j++ )
         {
            if ( fabs( avg_qs[ j ] - t_qs[ this_file ][ j ] ) > Q_VAL_TOL )
            {
               editor_msg( "red", us_tr( "Error: incompatible grids, the q values differ between selected files" ) );
               cout << QString( "val1 %1 val2 %2 fabs diff %3\n" ).arg( avg_qs[ j ] ).arg(  t_qs[ this_file ][ j ]  ).arg(fabs( avg_qs[ j ] - t_qs[ this_file ][ j ] ) );
               return;
            }
            avg_Is [ j ]     += t_Is[ this_file ][ j ];
            if ( j < (int) t_errors[ this_file ].size() &&
                 j < (int) avg_sd.size() )
            {
               avg_sd[ j ] += t_errors[ this_file ][ j ];
            }
         }
         avg_conc +=
            concs.count( this_file ) ?
            concs[ this_file ] :
            0e0;
         avg_psv  += f_psv.count( this_file ) ? f_psv[ this_file ] : 0e0;
         avg_I0se += f_I0se.count( this_file ) ? f_I0se[ this_file ] : 0e0;
      }            
   }

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

   QString wsum_name = head + "wsum" + tail;

   map < QString, bool > current_files;
   for ( int i = 0; i < (int)lb_files->count(); i++ )
   {
      current_files[ lb_files->item( i )->text() ] = true;
   }

   while ( current_files.count( wsum_name ) )
   {
      wsum_name = head + QString( "wsum-%1" ).arg( ++ext ) + tail;
   }

   lb_created_files->addItem( wsum_name );
   lb_created_files->scrollToItem( lb_created_files->item( lb_created_files->count() - 1 ) );
   lb_files->addItem( wsum_name );
   lb_files->scrollToItem( lb_files->item( lb_files->count() - 1 ) );
   created_files_not_saved[ wsum_name ] = true;
   
   f_pos       [ wsum_name ] = f_qs.size();
   f_qs_string [ wsum_name ] = avg_qs_string;
   f_qs        [ wsum_name ] = avg_qs;
   f_Is        [ wsum_name ] = avg_Is;
   f_errors    [ wsum_name ] = avg_sd;
   
   if ( avg_psv )
   {
      f_psv       [ wsum_name ] = avg_psv;
   }
   if ( avg_I0se )
   {
      f_I0se      [ wsum_name ] = avg_I0se;
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
           csv_conc.data[ i ][ 0 ] == wsum_name )
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

void US_Hydrodyn_Saxs_Buffer::select_these( map < QString, QString > & parameters, bool reenable )
{
   disable_updates = true;
   lb_files->clearSelection();
   for ( int i = 0; i < lb_files->count(); ++i )
   {
      if ( parameters.count( QString( "%1" ).arg( i ) ) )
      {
         lb_files->item( i)->setSelected( true );
      }
   }
   disable_updates = false;
   plot_files();
   if ( reenable )
   {
      update_enables();
   }
}

void US_Hydrodyn_Saxs_Buffer::select_nth()
{
   map < QString, QString > parameters;

   parameters[ "buffer_nth_contains"   ] = 
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "buffer_nth_contains" ) ?
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "buffer_nth_contains" ] : "";

   parameters[ "buffer_nth_shown"  ] = 
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "buffer_nth_shown" ) ?
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "buffer_nth_shown" ] : "";

   US_Hydrodyn_Saxs_Buffer_Nth *buffer_nth = 
      new US_Hydrodyn_Saxs_Buffer_Nth(
                                   this,
                                   & parameters,
                                   this );
   US_Hydrodyn::fixWinButtons( buffer_nth );
   buffer_nth->exec();
   delete buffer_nth;

   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "buffer_nth_shown" ] =
      parameters.count( "buffer_nth_shown" ) ? parameters[ "buffer_nth_shown" ] : "";

   if ( !parameters.count( "go" ) )
   {
      return;
   }

   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "buffer_nth_contains" ] =
      parameters.count( "buffer_nth_contains" ) ? parameters[ "buffer_nth_contains" ] : "";

   select_these( parameters );
   lb_files              ->setEnabled( true );
   lb_created_files      ->setEnabled( true );
}

double US_Hydrodyn_Saxs_Buffer::tot_intensity( QString &file, double q_min, double q_max )
{
   if ( !f_qs_string .count( file ) ||
        !f_qs        .count( file ) ||
        !f_Is        .count( file ) ||
        !f_pos       .count( file ) )
   {
      editor_msg( "red", QString( us_tr( "Total intensity compute internal error: no curve named %1" ) ).arg( file ) );
      return -9e99;
   }

   double tot_i = 0e0;
   for ( int i = 0; i < (int) f_qs[ file ].size(); ++i )
   {
      if ( f_qs[ file ][ i ] >= q_min &&
           f_qs[ file ][ i ] <= q_max )
      {
         tot_i += f_Is[ file ][ i ];
      }
   }
   return tot_i;
}

void US_Hydrodyn_Saxs_Buffer::wheel_pressed() {
   // qDebug() << "wheel_pressed()";
   wheel_is_pressed = true;
}

void US_Hydrodyn_Saxs_Buffer::wheel_released() {
   // qDebug() << "wheel_released()";
   wheel_is_pressed = false;
}

void US_Hydrodyn_Saxs_Buffer::usp_config_plot_dist( const QPoint & ) {
   US_PlotChoices *uspc = new US_PlotChoices( usp_plot_dist );
   uspc->exec();
   delete uspc;
}
