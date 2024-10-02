#include "../include/us3_defines.h"
// #include "../include/us_sas_dammin.h"
#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_hydrodyn_saxs_options.h"
#include "../include/us_hydrodyn_saxs_load_csv.h"
#include "../include/us_hydrodyn_saxs_mw.h"
#include "../include/us_hydrodyn_saxs_screen.h"
#include "../include/us_hydrodyn_saxs_search.h"
#include "../include/us_hydrodyn_saxs_buffer.h"
#include "../include/us_hydrodyn_saxs_hplc.h"
#include "../include/us_hydrodyn_saxs_1d.h"
#include "../include/us_hydrodyn_saxs_2d.h"
#include "../include/us_hydrodyn_xsr.h"
#include "../include/us_saxs_util.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_math.h"
#include <qwaitcondition.h>
#include "qwt_symbol.h"
//Added by qt3to4:
#include <QBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
 //#include <Q3PopupMenu>
#include <QHBoxLayout>
#include <QTextStream>
#include <QCloseEvent>
#include <QGridLayout>

#define SLASH "/"
#if defined(WIN32)
#  include <dos.h>
#  include <stdlib.h>
#  include <float.h>
#  undef SLASH
#  define SLASH "\\"
// #  define isnan _isnan
#endif

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#define SET_WIDTH_FROM_TEXT_LEN_PAD(widget,pad) (widget->setMaximumWidth(QFontMetrics(widget->font()).averageCharWidth()*1.4*(pad+widget->text().length())))
#define SET_WIDTH_FROM_TEXT_LEN(widget) SET_WIDTH_FROM_TEXT_LEN_PAD(widget,3)

// #define SAXS_DEBUG
// #define SAXS_DEBUG2
// #define SAXS_DEBUG_F
// #define SAXS_DEBUG_FV
// #define BUG_DEBUG
// #define RESCALE_B
#define SAXS_MIN_Q 1e-6
// #define ONLY_PHYSICAL_F
// #define I_MULT_2
// #define PR_DEBUG

US_Hydrodyn_Saxs::US_Hydrodyn_Saxs(
                                   bool                           *saxs_widget,
                                   saxs_options                   *our_saxs_options,
                                   QString                        filename, 
                                   QString                        filepathname, 
                                   vector < residue >             residue_list,
                                   vector < PDB_model >           model_vector,
                                   vector < vector <PDB_atom> >   bead_models,
                                   vector < unsigned int >        selected_models,
                                   map < QString, vector <int> >  multi_residue_map,
                                   map < QString, QString >       residue_atom_hybrid_map,
                                   int                            source,
                                   bool                           create_native_saxs,
                                   void                           *us_hydrodyn,
                                   QWidget                        *p, 
                                   const char                     *
                                   ) : QFrame( p )
{
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

   last_selected_pdb_filename = "";
   external_running = false;
   rasmol = NULL;
   this->saxs_widget = saxs_widget;
   *saxs_widget = true;
   this->our_saxs_options = our_saxs_options;
   model_filepathname = filepathname;
   guinier_cutoff = 0.2;
   last_used_mw = 0.0;

   pen_width = 1;

   plot_pr_zoomer    = (ScrollZoomer *)0;
   plot_saxs_zoomer  = (ScrollZoomer *)0;
   plot_resid_zoomer = (ScrollZoomer *)0;

   saxs_legend_vis   = false;
   pr_legend_vis     = false;

   saxs_residuals_widget = false;

   // note changes to this section should be updated in US_Hydrodyn_SaxsOptions::update_q()
   if ( our_saxs_options->wavelength == 0 )
   {
      our_saxs_options->start_q = 
         our_saxs_options->end_q = 
         our_saxs_options->delta_q = 0;
   }
   else
   {
      our_saxs_options->start_q = 4.0 * M_PI * 
         sin(our_saxs_options->start_angle * M_PI / 360.0) / 
         our_saxs_options->wavelength;
      our_saxs_options->start_q =  floor(our_saxs_options->start_q * SAXS_Q_ROUNDING + 0.5) / SAXS_Q_ROUNDING;
      our_saxs_options->end_q = 4.0 * M_PI * 
         sin(our_saxs_options->end_angle * M_PI / 360.0) / 
         our_saxs_options->wavelength;
      our_saxs_options->end_q =  floor(our_saxs_options->end_q * SAXS_Q_ROUNDING + 0.5) / SAXS_Q_ROUNDING;
      our_saxs_options->delta_q = 4.0 * M_PI * 
         sin(our_saxs_options->delta_angle * M_PI / 360.0) / 
         our_saxs_options->wavelength;
      our_saxs_options->delta_q =  floor(our_saxs_options->delta_q * SAXS_Q_ROUNDING + 0.5) / SAXS_Q_ROUNDING;
   }         

   this->residue_list = residue_list;
   this->model_vector = model_vector;
   this->bead_models = bead_models;
   this->selected_models = selected_models;
   this->multi_residue_map = multi_residue_map;
   this->residue_atom_hybrid_map = residue_atom_hybrid_map;
   this->source = source;
   this->create_native_saxs = create_native_saxs;
   this->us_hydrodyn = us_hydrodyn;
   this->remember_mw = &(((US_Hydrodyn *)us_hydrodyn)->dammix_remember_mw);
   this->remember_mw_source = &(((US_Hydrodyn *)us_hydrodyn)->dammix_remember_mw_source);
   this->match_remember_mw = &(((US_Hydrodyn *)us_hydrodyn)->dammix_match_remember_mw);

   use_SDs_for_fitting_iqq = true;
   use_SDs_for_fitting_prr = false;
   nnls_plot_contrib       = false;
   
   USglobal=new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle( us_tr( "US-SOMO: " + us_tr( "SAS Functions" ) ) );
   reset_search_csv();
   reset_screen_csv();
   reset_buffer_csv();
   reset_hplc_csv();
   reset_dad_csv();
   reset_mals_csv();
   reset_mals_saxs_csv();
   setupGUI();

   // QTextStream(stdout) << US_Vector::qs_mapqsqs( "saxs residue_atom_hybrid_map\n", residue_atom_hybrid_map );

   fix_sas_options();

   editor->append("\n\n");
   QFileInfo fi(filename);
   bead_model_ok_for_saxs = true;
   switch ( source )
   {
      case 0: // the source is a PDB file
      {
         lbl_filename1->setText(" PDB Filename: ");
         break;
      }
      case 1: // the source is a Bead Model file
      {
         lbl_filename1->setText(" Bead Model Filename: ");
         set_bead_model_ok_for_saxs();
         break;
      }
      default: // undefined
      {
         // QMessageBox mb(us_tr("UltraScan"),
         //                us_tr("The source file has not been defined, please try again..."),
         //                QMessageBox::Critical,
         //                   QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
         // if (mb.exec())
         QMessageBox::critical( this,
                                us_tr("UltraScan"),
                                us_tr("The source file has not been defined, please try again..."),
                                Qt::NoButton, Qt::NoButton, Qt::NoButton );
         {
            exit(-1);
         }
      }
   }
   if ( !selected_models.size() )
   {
      bead_model_ok_for_saxs = false;
   }
   for ( int i = 0; i < (int) model_vector.size(); i++ )
   {
      cout << QString( "summary info for model %1\n" ).arg( i + 1 );

      cout << US_Saxs_Util::list_atom_summary_counts( &model_vector[ i ],
                                                      residue_atom_hybrid_map,
                                                      our_saxs_options
                                                      );
   }
   pb_plot_saxs_sans->setEnabled(bead_model_ok_for_saxs);
   te_filename2->setText(filename);
   model_filename = filename;
   atom_filename = our_saxs_options->default_atom_filename;
   hybrid_filename = our_saxs_options->default_hybrid_filename;
   saxs_filename = our_saxs_options->default_saxs_filename;
   select_saxs_file(saxs_filename);

   /*
   // check saxs map for sign
   for ( unsigned int i = 0; i < saxs_list.size(); i++ )
   {
      double sum4 = saxs_list[ i ].c;
      for ( unsigned int j = 0; j < 4; j++ )
      {
         sum4 += saxs_list[ i ].a[ j ];
      }
      cout << QString( "saxs entry %1 4 term q=0 value %2 %3\n" )
         .arg( saxs_list[ i ].saxs_name )
         .arg( sum4 )
         .arg( sum4 < 0e0 ? "NEGATIVE" : "positive" )
         ;
      double sum5 = saxs_list[ i ].c5;
      for ( unsigned int j = 0; j < 5; j++ )
      {
         sum5 += saxs_list[ i ].a5[ j ];
      }
      cout << QString( "saxs entry %1 5 term q=0 value %2 %3\n" )
         .arg( saxs_list[ i ].saxs_name )
         .arg( sum5 )
         .arg( sum5 < 0e0 ? "NEGATIVE" : "positive" )
         ;
   }
   */

   select_hybrid_file(hybrid_filename);
   select_atom_file(atom_filename);
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
   stopFlag = false;
   pb_stop->setEnabled(false);

   QColor bgc = plot_saxs->canvasBackground().color();

   plot_colors.clear( );
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

   saxs_search_update_enables();
   add_to_directory_history( ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "saxs", false );
   add_to_directory_history( ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH, false );

   sync_conc_csv();
#if QT_VERSION >= 0x040000
   set_saxs_legend();
   set_pr_legend();
#endif

   // #define TEST_PR_SD_ZEROS
#if defined( TEST_PR_SD_ZEROS )
   {
      vector < double > r       { 0,   1,   2,   3,   4, 5, 6, 7, 8 };
      vector < double > pr      { 0,   1,   4,   5,   4, 0, 0, 0, 0 };
      vector < double > pr_error{ 0, 0.1, 0.0, 0.0, 0.2, 0, 0, 0, 0 };

      US_Vector::printvector3( "test_pr_sd_zeros init r, pr, pr_error", r, pr, pr_error );

      prop_pr_sd_tail( pr_error, (int) pr_error.size() );

      US_Vector::printvector3( "test_pr_sd_zeros after prop_pr_sd_tail r, pr, pr_error", r, pr, pr_error );

      set_pr_sd( r, pr, pr_error );
      
      US_Vector::printvector3( "test_pr_sd_zeros after set_pr_sd r, pr, pr_error", r, pr, pr_error );
      
      crop_pr_tail( r, pr, pr_error );
      
      US_Vector::printvector3( "test_pr_sd_zeros after crop_pr_tail_pr_sd r, pr, pr_error", r, pr, pr_error );
   }
#endif

   // #define TEST_SPLINE
#if defined( TEST_SPLINE )
   {
      vector < double > r       { 0,0.5,1,1.5,2,2.5,3,3.5,4,4.5,5,5.5,6,6.5,7,7.5,8,8.5,9,9.5,10,10.5,11,11.5,12,12.5,13,13.5,14,14.5,15,15.5,16,16.5,17,17.5,18,18.5,19,19.5,20,20.5,21,21.5,22,22.5,23,23.5,24,24.5,25,25.5,26,26.5,27,27.5,28,28.5,29,29.5,30,30.5,31,31.5,32,32.5,33,33.5,34,34.5,35,35.5,36,36.5,37,37.5,38,38.5,39,39.5,40,40.5,41,41.5,42,42.5,43,43.5,44,44.5,45,45.5,46,46.5,47,47.5,48 };
      vector < double > r2      { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48 };
      vector < double > pr      { 0,214.113,2899.98,4356.94,5064.47,5996.28,8773.47,9142.83,11972.1,14382.3,16433.9,17822.1,19401.3,21125.9,22991.6,23459.2,23826.9,24262.5,24312,24970.1,23414.5,24176.2,22509,21580.1,18877.5,16910.8,14819.3,12569.9,11300.5,9325.03,8177.13,6942.54,6054.87,4466.99,3982.04,2886.17,2360.64,2167.71,1586.22,1371.81,913.865,626.773,388.171,268.683,103.848,83.4387,42.6576,8.68127,6.99182 };
      vector < double > npr;

      natural_spline_interpolate( r, r2, pr, npr );

      US_Vector::printvector2( "before  interp r2, pr", r2, pr, 8, 10 );
      US_Vector::printvector2( "after  nat spline interp r, npr", r, npr, 8, 10 );

      npr = interpolate( r, r2, pr );
      US_Vector::printvector2( "after  'interpolate' r, npr", r, npr, 8, 10 );
      
   }
#endif

}

void US_Hydrodyn_Saxs::push_back_color_if_ok( QColor bg, QColor set )
{
   double sum = 
      fabs( (float) bg.red  () - (float) set.red  () ) +
      fabs( (float) bg.green() - (float) set.green() ) +
      fabs( (float) bg.blue () - (float) set.blue () );
   if ( sum > 200 )
   {
      if ( plot_colors.size() )
      {
         bg = plot_colors.back();
         double sum = 
            fabs( (float) bg.red  () - (float) set.red  () ) +
            fabs( (float) bg.green() - (float) set.green() ) +
            fabs( (float) bg.blue () - (float) set.blue () );
         if ( sum > 150 )
         {
            plot_colors.push_back( set );
         }
      } else {
         plot_colors.push_back( set );
      }
   }
}

US_Hydrodyn_Saxs::~US_Hydrodyn_Saxs()
{
   *saxs_widget = false;
}

void US_Hydrodyn_Saxs::refresh(
                               QString                        filename, 
                               QString                        filepathname, 
                               vector < residue >             residue_list,
                               vector < PDB_model >           model_vector,
                               vector < vector <PDB_atom> >   bead_models,
                               vector < unsigned int >        selected_models,
                               map < QString, vector <int> >  multi_residue_map,
                               map < QString, QString >       residue_atom_hybrid_map,
                               int                            source,
                               bool                           create_native_saxs
                               )
{
   this->residue_list = residue_list;
   this->model_vector = model_vector;
   this->bead_models = bead_models;
   this->selected_models = selected_models;
   this->multi_residue_map = multi_residue_map;
   this->residue_atom_hybrid_map = residue_atom_hybrid_map;
   this->source = source;
   this->create_native_saxs = create_native_saxs;
   model_filepathname = filepathname;
   last_selected_pdb_filename = "";
   QFileInfo fi(filename);
   bead_model_ok_for_saxs = true;
   switch (source)
   {
      case 0: // the source is a PDB file
      {
         lbl_filename1->setText(" PDB Filename: ");
         break;
      }
      case 1: // the source is a Bead Model file
      {
         lbl_filename1->setText(" Bead Model Filename: ");
         set_bead_model_ok_for_saxs();
         break;
      }
      default: // undefined
      {
         // QMessageBox mb(us_tr("UltraScan"),
         //                us_tr("The source file has not been defined, please try again..."),
         //                QMessageBox::Critical,
         //                   QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
         // if (mb.exec())
         QMessageBox::critical( this,
                                us_tr("UltraScan"),
                                us_tr("The source file has not been defined, please try again..."),
                                Qt::NoButton, Qt::NoButton, Qt::NoButton );
         {
            exit(-1);
         }
      }
   }
   if ( !selected_models.size() )
   {
      bead_model_ok_for_saxs = false;
   }

   if ( source )
   {
      our_saxs_options->curve = 0;
      rb_curve_raw->setChecked(true);
      // rb_curve_saxs_dry->setChecked(false);
      rb_curve_saxs->setChecked(false);
      rb_curve_sans->setChecked(false);
      rb_curve_raw->setEnabled(false);
      // rb_curve_saxs_dry->setEnabled(false);
      rb_curve_saxs->setEnabled(false);
      rb_curve_sans->setEnabled(false);
      if ( bead_model_has_electrons() ) {
         rb_curve_raw->setEnabled(true);
         rb_curve_saxs->setChecked(true);
         rb_curve_saxs->setEnabled(true);
      }
   } else {
      rb_curve_raw->setEnabled(true);
      // rb_curve_saxs_dry->setEnabled(true);
      rb_curve_saxs->setEnabled(true);
      rb_curve_sans->setEnabled(true);
   }
      
   pb_plot_saxs_sans->setEnabled(bead_model_ok_for_saxs);
   te_filename2->setText(filename);
   model_filename = filename;
   pb_stop->setEnabled(false);
   cb_create_native_saxs->setChecked(create_native_saxs);
   saxs_search_update_enables();
}

void US_Hydrodyn_Saxs::setupGUI()
{
   started_in_expert_mode = U_EXPT;

   int minHeight0 = 18;
   int minHeight1 = 22;
   int maxWidth   = 14;

   // ************ initial file ***********

   lbl_filename1 = new QLabel(us_tr(""), this);
   lbl_filename1->setMinimumHeight(minHeight1);
   lbl_filename1->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_filename1->setPalette( PALET_LABEL );
   AUTFBACK( lbl_filename1 );
   lbl_filename1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   te_filename2 = new QLineEdit( this );    te_filename2->setObjectName( "" );
   te_filename2->setMinimumHeight(minHeight1);
   te_filename2->setMaximumHeight(minHeight1);
   te_filename2->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   te_filename2->setPalette( PALET_EDIT );
   AUTFBACK( te_filename2 );
   te_filename2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   // te_filename2->setMinimumWidth(200);
   // te_filename2->setReadOnly( true );

   // ************ settings ***********

   lbl_settings = new mQLabel(us_tr("Definition files:"), this);
   lbl_settings->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_settings->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_settings->setMinimumHeight(minHeight1);
   lbl_settings->setPalette( PALET_FRAME );
   AUTFBACK( lbl_settings );
   lbl_settings->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   connect( lbl_settings, SIGNAL( pressed() ), SLOT( hide_settings() ) );

   pb_select_atom_file = new QPushButton(us_tr("Load Atom Definition File"), this);
   pb_select_atom_file->setMinimumHeight(minHeight1);
   pb_select_atom_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_select_atom_file->setPalette( PALET_PUSHB );
   connect(pb_select_atom_file, SIGNAL(clicked()), SLOT(select_atom_file()));
   settings_widgets.push_back( pb_select_atom_file );

   pb_select_hybrid_file = new QPushButton(us_tr("Load Hybridization File"), this);
   pb_select_hybrid_file->setMinimumHeight(minHeight1);
   pb_select_hybrid_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_select_hybrid_file->setPalette( PALET_PUSHB );
   connect(pb_select_hybrid_file, SIGNAL(clicked()), SLOT(select_hybrid_file()));
   settings_widgets.push_back( pb_select_hybrid_file );

   pb_select_saxs_file = new QPushButton(us_tr("Load SAXS Coefficients File"), this);
   pb_select_saxs_file->setMinimumHeight(minHeight1);
   pb_select_saxs_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_select_saxs_file->setPalette( PALET_PUSHB );
   connect(pb_select_saxs_file, SIGNAL(clicked()), SLOT(select_saxs_file()));
   settings_widgets.push_back( pb_select_saxs_file );

   lbl_atom_table = new QLabel(us_tr(" not selected"),this);
   lbl_atom_table->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_atom_table->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_atom_table->setPalette( PALET_EDIT );
   AUTFBACK( lbl_atom_table );
   lbl_atom_table->setMinimumHeight(minHeight1);
   lbl_atom_table->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   settings_widgets.push_back( lbl_atom_table );

   lbl_hybrid_table = new QLabel(us_tr(" not selected"),this);
   lbl_hybrid_table->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_hybrid_table->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_hybrid_table->setPalette( PALET_EDIT );
   AUTFBACK( lbl_hybrid_table );
   lbl_hybrid_table->setMinimumHeight(minHeight1);
   lbl_hybrid_table->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   settings_widgets.push_back( lbl_hybrid_table );

   lbl_saxs_table = new QLabel(us_tr(" not selected"),this);
   lbl_saxs_table->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_saxs_table->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_saxs_table->setPalette( PALET_EDIT );
   AUTFBACK( lbl_saxs_table );
   lbl_saxs_table->setMinimumHeight(minHeight1);
   lbl_saxs_table->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   settings_widgets.push_back( lbl_saxs_table );

   // ************ SAS ***********

   lbl_iq = new mQLabel(us_tr("SAS I(q) Functions:"), this);
   lbl_iq->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_iq->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_iq->setMinimumHeight(minHeight1);
   lbl_iq->setPalette( PALET_FRAME );
   AUTFBACK( lbl_iq );
   lbl_iq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   connect( lbl_iq, SIGNAL( pressed() ), SLOT( hide_iq() ) );

   rb_saxs = new QRadioButton(us_tr("SAXS"), this);
   rb_saxs->setEnabled(true);
   rb_saxs->setChecked(!our_saxs_options->saxs_sans);
   rb_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_saxs->setPalette( PALET_NORMAL );
   AUTFBACK( rb_saxs );
   iq_widgets.push_back( rb_saxs );

   rb_sans = new QRadioButton(us_tr("SANS"), this);
   rb_sans->setEnabled(true);
   rb_sans->setChecked(our_saxs_options->saxs_sans);
   rb_sans->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_sans->setPalette( PALET_NORMAL );
   AUTFBACK( rb_sans );
   iq_widgets.push_back( rb_sans );

#if 1 // QT_VERSION < 0x040000
   bg_saxs_sans = new QButtonGroup( this );
   int bg_pos = 0;
   bg_saxs_sans->setExclusive(true);
   bg_saxs_sans->addButton( rb_saxs, bg_pos++ );
   bg_saxs_sans->addButton( rb_sans, bg_pos++ );
   connect(bg_saxs_sans, SIGNAL(buttonClicked(int)), SLOT(set_saxs_sans(int)));
   // iq_widgets.push_back( bg_saxs_sans );
#else
   bg_saxs_sans = new QGroupBox();
   bg_saxs_sans->setFlat( true );

   connect( rb_saxs, SIGNAL( clicked() ), this, SLOT( set_saxs_sans() ) );
   connect( rb_sans, SIGNAL( clicked() ), this, SLOT( set_saxs_sans() ) );
   {
      QHBoxLayout * bl = new QHBoxLayout; bl->setContentsMargins( 0, 0, 0, 0 ); bl->setSpacing( 0 );
      bl->addWidget( rb_saxs );
      bl->addWidget( rb_sans );
      bg_saxs_sans->setLayout( bl );
   }
#endif

   rb_saxs_iq_native_debye = new QRadioButton(us_tr("F-DB"), this);
   rb_saxs_iq_native_debye->setEnabled(true);
   rb_saxs_iq_native_debye->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_saxs_iq_native_debye->setPalette( PALET_NORMAL );
   AUTFBACK( rb_saxs_iq_native_debye );
   iq_widgets.push_back( rb_saxs_iq_native_debye );

   rb_saxs_iq_native_sh = new QRadioButton(us_tr("SH-DB"), this);
   rb_saxs_iq_native_sh->setEnabled(true);
   rb_saxs_iq_native_sh->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_saxs_iq_native_sh->setPalette( PALET_NORMAL );
   AUTFBACK( rb_saxs_iq_native_sh );
   iq_widgets.push_back( rb_saxs_iq_native_sh );

   if ( started_in_expert_mode )
   {
      rb_saxs_iq_native_hybrid = new QRadioButton(us_tr("Hybrid   "), this);
      rb_saxs_iq_native_hybrid->setEnabled(true);
      rb_saxs_iq_native_hybrid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      rb_saxs_iq_native_hybrid->setPalette( PALET_NORMAL );
      AUTFBACK( rb_saxs_iq_native_hybrid );
      iq_widgets.push_back( rb_saxs_iq_native_hybrid );

      rb_saxs_iq_native_hybrid2 = new QRadioButton(us_tr("H2"), this);
      rb_saxs_iq_native_hybrid2->setEnabled(true);
      rb_saxs_iq_native_hybrid2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      rb_saxs_iq_native_hybrid2->setPalette( PALET_NORMAL );
      AUTFBACK( rb_saxs_iq_native_hybrid2 );
      iq_widgets.push_back( rb_saxs_iq_native_hybrid2 );

      rb_saxs_iq_native_hybrid3 = new QRadioButton(us_tr("H3"), this);
      rb_saxs_iq_native_hybrid3->setEnabled(true);
      rb_saxs_iq_native_hybrid3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      rb_saxs_iq_native_hybrid3->setPalette( PALET_NORMAL );
      AUTFBACK( rb_saxs_iq_native_hybrid3 );
      iq_widgets.push_back( rb_saxs_iq_native_hybrid3 );
   }

   rb_saxs_iq_native_fast = new QRadioButton(us_tr("Q-DB"), this);
   rb_saxs_iq_native_fast->setEnabled(true);
   rb_saxs_iq_native_fast->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_saxs_iq_native_fast->setPalette( PALET_NORMAL );
   AUTFBACK( rb_saxs_iq_native_fast );
   iq_widgets.push_back( rb_saxs_iq_native_fast );

   if ( started_in_expert_mode )
   {
      rb_saxs_iq_foxs = new QRadioButton(us_tr("FoXS"), this);
      rb_saxs_iq_foxs->setEnabled(true);
      rb_saxs_iq_foxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      rb_saxs_iq_foxs->setPalette( PALET_NORMAL );
      AUTFBACK( rb_saxs_iq_foxs );
      iq_widgets.push_back( rb_saxs_iq_foxs );
   }

   rb_saxs_iq_crysol = new QRadioButton(us_tr("Crysol"), this);
   rb_saxs_iq_crysol->setEnabled(true);
   rb_saxs_iq_crysol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_saxs_iq_crysol->setPalette( PALET_NORMAL );
   AUTFBACK( rb_saxs_iq_crysol );
   iq_widgets.push_back( rb_saxs_iq_crysol );

   if ( started_in_expert_mode )
   {
      rb_saxs_iq_sastbx = new QRadioButton(us_tr("Sastbx"), this);
      rb_saxs_iq_sastbx->setEnabled(true);
      rb_saxs_iq_sastbx->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      rb_saxs_iq_sastbx->setPalette( PALET_NORMAL );
      AUTFBACK( rb_saxs_iq_sastbx );
      iq_widgets.push_back( rb_saxs_iq_sastbx );
   }

#if 1 // QT_VERSION < 0x040000
   bg_saxs_iq = new QButtonGroup( this );
   bg_pos = 0;
   bg_saxs_iq->setExclusive(true);
   bg_saxs_iq->addButton( rb_saxs_iq_native_debye, bg_pos++ );
   bg_saxs_iq->addButton( rb_saxs_iq_native_sh, bg_pos++ );
   if ( started_in_expert_mode )
   {
      bg_saxs_iq->addButton( rb_saxs_iq_native_hybrid, bg_pos++ );
      bg_saxs_iq->addButton( rb_saxs_iq_native_hybrid2, bg_pos++ );
      bg_saxs_iq->addButton( rb_saxs_iq_native_hybrid3, bg_pos++ );
   }
   bg_saxs_iq->addButton( rb_saxs_iq_native_fast, bg_pos++ );
   if ( started_in_expert_mode )
   {
      bg_saxs_iq->addButton( rb_saxs_iq_foxs, bg_pos++ );
   }
   bg_saxs_iq->addButton( rb_saxs_iq_crysol, bg_pos++ );

   if ( started_in_expert_mode )
   {
      bg_saxs_iq->addButton( rb_saxs_iq_sastbx, bg_pos++ );
   }
   connect(bg_saxs_iq, SIGNAL(buttonClicked(int)), SLOT(set_saxs_iq(int)));
   // iq_widgets.push_back( bg_saxs_iq );
#else
   bg_saxs_iq = new QGroupBox();
   bg_saxs_iq->setFlat( true );

   connect( rb_saxs_iq_native_debye, SIGNAL( clicked() ), this, SLOT( set_saxs_iq() ) );
   connect( rb_saxs_iq_native_sh, SIGNAL( clicked() ), this, SLOT( set_saxs_iq() ) );
   if ( started_in_expert_mode ) {
      connect( rb_saxs_iq_native_hybrid, SIGNAL( clicked() ), this, SLOT( set_saxs_iq() ) );
      connect( rb_saxs_iq_native_hybrid2, SIGNAL( clicked() ), this, SLOT( set_saxs_iq() ) );
      connect( rb_saxs_iq_native_hybrid3, SIGNAL( clicked() ), this, SLOT( set_saxs_iq() ) );
   }
   connect( rb_saxs_iq_native_fast, SIGNAL( clicked() ), this, SLOT( set_saxs_iq() ) );
   if ( started_in_expert_mode ) {
      connect( rb_saxs_iq_foxs, SIGNAL( clicked() ), this, SLOT( set_saxs_iq() ) );
   }
   connect( rb_saxs_iq_crysol, SIGNAL( clicked() ), this, SLOT( set_saxs_iq() ) );
   if ( started_in_expert_mode ) {
      connect( rb_saxs_iq_sastbx, SIGNAL( clicked() ), this, SLOT( set_saxs_iq() ) );
   }
      
   {
      QHBoxLayout * bl = new QHBoxLayout; bl->setContentsMargins( 0, 0, 0, 0 ); bl->setSpacing( 0 );
      bl->addWidget( rb_saxs_iq_native_debye );
      bl->addWidget( rb_saxs_iq_native_sh );
      if ( started_in_expert_mode ) {
         bl->addWidget( rb_saxs_iq_native_hybrid );
         bl->addWidget( rb_saxs_iq_native_hybrid2 );
         bl->addWidget( rb_saxs_iq_native_hybrid3 );
      }
      bl->addWidget( rb_saxs_iq_native_fast );
      if ( started_in_expert_mode ) {
         bl->addWidget( rb_saxs_iq_foxs );
      }
      bl->addWidget( rb_saxs_iq_crysol );
      if ( started_in_expert_mode ) {
         bl->addWidget( rb_saxs_iq_sastbx );
      }
      bg_saxs_iq->setLayout( bl );
   }
#endif

   rb_sans_iq_native_debye = new QRadioButton(us_tr("F-DB"), this);
   rb_sans_iq_native_debye->setEnabled(true);
   rb_sans_iq_native_debye->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_sans_iq_native_debye->setPalette( PALET_NORMAL );
   AUTFBACK( rb_sans_iq_native_debye );
   iq_widgets.push_back( rb_sans_iq_native_debye );

   rb_sans_iq_native_sh = new QRadioButton(us_tr("SH-DB"), this);
   rb_sans_iq_native_sh->setEnabled(true);
   rb_sans_iq_native_sh->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_sans_iq_native_sh->setPalette( PALET_NORMAL );
   AUTFBACK( rb_sans_iq_native_sh );
   iq_widgets.push_back( rb_sans_iq_native_sh );

   if ( started_in_expert_mode )
   {
      rb_sans_iq_native_hybrid = new QRadioButton(us_tr("Hybrid   "), this);
      rb_sans_iq_native_hybrid->setEnabled(true);
      rb_sans_iq_native_hybrid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      rb_sans_iq_native_hybrid->setPalette( PALET_NORMAL );
      AUTFBACK( rb_sans_iq_native_hybrid );
      iq_widgets.push_back( rb_sans_iq_native_hybrid );

      rb_sans_iq_native_hybrid2 = new QRadioButton(us_tr("H2"), this);
      rb_sans_iq_native_hybrid2->setEnabled(true);
      rb_sans_iq_native_hybrid2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      rb_sans_iq_native_hybrid2->setPalette( PALET_NORMAL );
      AUTFBACK( rb_sans_iq_native_hybrid2 );
      iq_widgets.push_back( rb_sans_iq_native_hybrid2 );

      rb_sans_iq_native_hybrid3 = new QRadioButton(us_tr("H3"), this);
      rb_sans_iq_native_hybrid3->setEnabled(true);
      rb_sans_iq_native_hybrid3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      rb_sans_iq_native_hybrid3->setPalette( PALET_NORMAL );
      AUTFBACK( rb_sans_iq_native_hybrid3 );
      iq_widgets.push_back( rb_sans_iq_native_hybrid3 );
   }

   rb_sans_iq_native_fast = new QRadioButton(us_tr("Q-DB"), this);
   rb_sans_iq_native_fast->setEnabled(true);
   rb_sans_iq_native_fast->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_sans_iq_native_fast->setPalette( PALET_NORMAL );
   AUTFBACK( rb_sans_iq_native_fast );
   iq_widgets.push_back( rb_sans_iq_native_fast );

   rb_sans_iq_cryson = new QRadioButton(us_tr("Cryson"), this);
   rb_sans_iq_cryson->setEnabled(true);
   rb_sans_iq_cryson->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_sans_iq_cryson->setPalette( PALET_NORMAL );
   AUTFBACK( rb_sans_iq_cryson );
   iq_widgets.push_back( rb_sans_iq_cryson );

#if 1 // QT_VERSION < 0x040000
   bg_sans_iq = new QButtonGroup( this );
   bg_pos = 0;
   bg_sans_iq->setExclusive(true);
   bg_sans_iq->addButton( rb_sans_iq_native_debye, bg_pos++ );
   bg_sans_iq->addButton( rb_sans_iq_native_sh, bg_pos++ );
   if ( started_in_expert_mode )
   {
      bg_sans_iq->addButton( rb_sans_iq_native_hybrid, bg_pos++ );
      bg_sans_iq->addButton( rb_sans_iq_native_hybrid2, bg_pos++ );
      bg_sans_iq->addButton( rb_sans_iq_native_hybrid3, bg_pos++ );
   }
   bg_sans_iq->addButton( rb_sans_iq_native_fast, bg_pos++ );
   bg_sans_iq->addButton( rb_sans_iq_cryson, bg_pos++ );
   connect(bg_sans_iq, SIGNAL(buttonClicked(int)), SLOT(set_sans_iq(int)));
   // iq_widgets.push_back( bg_sans_iq );
#else
   bg_sans_iq = new QGroupBox();
   bg_sans_iq->setFlat( true );

   connect( rb_sans_iq_native_debye, SIGNAL( clicked() ), this, SLOT( set_sans_iq() ) );
   connect( rb_sans_iq_native_sh, SIGNAL( clicked() ), this, SLOT( set_sans_iq() ) );
   if ( started_in_expert_mode ) {
      connect( rb_sans_iq_native_hybrid, SIGNAL( clicked() ), this, SLOT( set_sans_iq() ) );
      connect( rb_sans_iq_native_hybrid2, SIGNAL( clicked() ), this, SLOT( set_sans_iq() ) );
      connect( rb_sans_iq_native_hybrid3, SIGNAL( clicked() ), this, SLOT( set_sans_iq() ) );
   }
   connect( rb_sans_iq_native_fast, SIGNAL( clicked() ), this, SLOT( set_sans_iq() ) );
   connect( rb_sans_iq_cryson, SIGNAL( clicked() ), this, SLOT( set_sans_iq() ) );

   {
      QHBoxLayout * bl = new QHBoxLayout; bl->setContentsMargins( 0, 0, 0, 0 ); bl->setSpacing( 0 );
      bl->addWidget( rb_sans_iq_native_debye );
      bl->addWidget( rb_sans_iq_native_sh );
      if ( started_in_expert_mode ) {
         bl->addWidget( rb_sans_iq_native_hybrid );
         bl->addWidget( rb_sans_iq_native_hybrid2 );
         bl->addWidget( rb_sans_iq_native_hybrid3 );
      }
      bl->addWidget( rb_sans_iq_native_fast );
      bl->addWidget( rb_sans_iq_cryson );
      bg_sans_iq->setLayout( bl );
   }
#endif

   lbl_iqq_suffix = new QLabel(us_tr(" File suffix: "), this);
   lbl_iqq_suffix->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_iqq_suffix->setMinimumHeight(minHeight1);
   lbl_iqq_suffix->setPalette( PALET_LABEL );
   AUTFBACK( lbl_iqq_suffix );
   lbl_iqq_suffix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));
   iq_widgets.push_back( lbl_iqq_suffix );

   le_iqq_manual_suffix = new QLineEdit( this );    le_iqq_manual_suffix->setObjectName( "iqq_manual_suffix Line Edit" );
   le_iqq_manual_suffix->setText(us_tr(""));
   le_iqq_manual_suffix->setMinimumHeight(minHeight1);
   le_iqq_manual_suffix->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_iqq_manual_suffix->setPalette( PALET_NORMAL );
   AUTFBACK( le_iqq_manual_suffix );
   le_iqq_manual_suffix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   iq_widgets.push_back( le_iqq_manual_suffix );

   le_iqq_full_suffix = new QLineEdit( this );    le_iqq_full_suffix->setObjectName( "iqq_full_suffix Line Edit" );
   le_iqq_full_suffix->setText(us_tr(""));
   le_iqq_full_suffix->setMinimumHeight(minHeight1);
   le_iqq_full_suffix->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_iqq_full_suffix->setPalette( PALET_NORMAL );
   AUTFBACK( le_iqq_full_suffix );
   le_iqq_full_suffix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_iqq_full_suffix->setReadOnly(true);
   iq_widgets.push_back( le_iqq_full_suffix );

   pb_plot_saxs_sans = new QPushButton("", this);
   Q_CHECK_PTR(pb_plot_saxs_sans);
   pb_plot_saxs_sans->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_plot_saxs_sans->setMinimumHeight(minHeight1);
   pb_plot_saxs_sans->setPalette( PALET_PUSHB );
   connect(pb_plot_saxs_sans, SIGNAL(clicked()), SLOT(show_plot_saxs_sans()));
   iq_widgets.push_back( pb_plot_saxs_sans );

   pb_load_saxs_sans = new QPushButton("", this);
   Q_CHECK_PTR(pb_load_saxs_sans);
   pb_load_saxs_sans->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_load_saxs_sans->setMinimumHeight(minHeight1);
   pb_load_saxs_sans->setPalette( PALET_PUSHB );
   connect(pb_load_saxs_sans, SIGNAL(clicked()), SLOT(load_saxs_sans()));
   iq_widgets.push_back( pb_load_saxs_sans );

   pb_load_plot_saxs = new QPushButton(us_tr("Load Plotted"), this);
   pb_load_plot_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_load_plot_saxs->setMinimumHeight(minHeight1);
   SET_WIDTH_FROM_TEXT_LEN( pb_load_plot_saxs );
   pb_load_plot_saxs->setPalette( PALET_PUSHB );
   connect(pb_load_plot_saxs, SIGNAL(clicked()), SLOT(load_plot_saxs()));
   iq_widgets.push_back( pb_load_plot_saxs );

   pb_set_grid = new QPushButton(us_tr("Set Grid"), this);
   pb_set_grid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_set_grid->setMinimumHeight(minHeight1);
   SET_WIDTH_FROM_TEXT_LEN( pb_set_grid );
   pb_set_grid->setPalette( PALET_PUSHB );
   connect(pb_set_grid, SIGNAL(clicked()), SLOT(set_grid()));
   iq_widgets.push_back( pb_set_grid );

   pb_clear_plot_saxs = new QPushButton("", this);
   pb_clear_plot_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_clear_plot_saxs->setMinimumHeight(minHeight1);
   pb_clear_plot_saxs->setPalette( PALET_PUSHB );
   connect(pb_clear_plot_saxs, SIGNAL(clicked()), SLOT(clear_plot_saxs()));
   iq_widgets.push_back( pb_clear_plot_saxs );

   pb_width = new QPushButton(us_tr( "Width" ), this);
   pb_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_width->setMinimumHeight(minHeight1);
   SET_WIDTH_FROM_TEXT_LEN( pb_width );
   pb_width->setPalette( PALET_PUSHB );
   connect(pb_width, SIGNAL(clicked()), SLOT(set_width()));
   iq_widgets.push_back( pb_width );

   pb_rescale = new QPushButton(us_tr("Rescale XY"), this);
   pb_rescale->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   pb_rescale->setMinimumHeight(minHeight1);
   SET_WIDTH_FROM_TEXT_LEN( pb_rescale );
   pb_rescale->setPalette( PALET_PUSHB );
   connect(pb_rescale, SIGNAL(clicked()), SLOT(do_rescale()));
   pb_rescale->hide();

   pb_rescale_y = new QPushButton(us_tr("Rescale Y"), this);
   pb_rescale_y->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   pb_rescale_y->setMinimumHeight(minHeight1);
   SET_WIDTH_FROM_TEXT_LEN( pb_rescale_y );
   pb_rescale_y->setPalette( PALET_PUSHB );
   connect(pb_rescale_y, SIGNAL(clicked()), SLOT(do_rescale_y()));
   pb_rescale_y->hide();
   // resid_widgets.push_back( pb_rescale_y );

   cb_eb = new QCheckBox(this);
   cb_eb->setText(us_tr("Err "));
   cb_eb->setMaximumWidth ( minHeight1 * 2 );
   cb_eb->setChecked( false );
   cb_eb->setMinimumHeight( minHeight1 );
   cb_eb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ) );
   cb_eb->setPalette( PALET_NORMAL );
   AUTFBACK( cb_eb );
   connect( cb_eb, SIGNAL( clicked() ), SLOT( set_eb() ) );
   iq_widgets.push_back( cb_eb );

   pb_pp = new QPushButton(" Save plots", this);
   pb_pp->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_pp->setMinimumHeight(minHeight1);
   SET_WIDTH_FROM_TEXT_LEN( pb_pp );
   pb_pp->setPalette( PALET_PUSHB );
   connect(pb_pp, SIGNAL(clicked()), SLOT(pp()));
   iq_widgets.push_back( pb_pp );

   pb_saxs_legend = new QPushButton( "Legend", this);
   pb_saxs_legend->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_saxs_legend->setMinimumHeight(minHeight1);
   SET_WIDTH_FROM_TEXT_LEN( pb_saxs_legend );
   pb_saxs_legend->setPalette( PALET_PUSHB );
   connect(pb_saxs_legend, SIGNAL(clicked()), SLOT(saxs_legend()));
   iq_widgets.push_back( pb_saxs_legend );

   cb_create_native_saxs = new QCheckBox(this);
   cb_create_native_saxs->setText(us_tr(" Create standard output files"));
   cb_create_native_saxs->setEnabled(true);
   cb_create_native_saxs->setChecked(create_native_saxs);
   cb_create_native_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_create_native_saxs->setPalette( PALET_NORMAL );
   AUTFBACK( cb_create_native_saxs );
   connect(cb_create_native_saxs, SIGNAL(clicked()), SLOT(set_create_native_saxs()));
   iq_widgets.push_back( cb_create_native_saxs );

   pb_load_gnom = new QPushButton("Load GNOM File", this);
   pb_load_gnom->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_load_gnom->setMinimumHeight(minHeight1);
   SET_WIDTH_FROM_TEXT_LEN( pb_load_gnom );
   pb_load_gnom->setPalette( PALET_PUSHB );
   connect(pb_load_gnom, SIGNAL(clicked()), SLOT(load_gnom()));
   iq_widgets.push_back( pb_load_gnom );

   pb_ift = new QPushButton("IFT", this);
   pb_ift->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_ift->setMinimumHeight(minHeight1);
   SET_WIDTH_FROM_TEXT_LEN(pb_ift);
   pb_ift->setPalette( PALET_PUSHB );
   connect(pb_ift, SIGNAL(clicked()), SLOT(call_ift()));
   iq_widgets.push_back( pb_ift );

   pb_saxs_search = new QPushButton("Search", this);
   pb_saxs_search->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_saxs_search->setMinimumHeight(minHeight1);
   SET_WIDTH_FROM_TEXT_LEN( pb_saxs_search );
   pb_saxs_search->setPalette( PALET_PUSHB );
   connect(pb_saxs_search, SIGNAL(clicked()), SLOT(saxs_search()));
   iq_widgets.push_back( pb_saxs_search );

   if ( started_in_expert_mode )
   {
      pb_saxs_screen = new QPushButton("Screen", this);
      pb_saxs_screen->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      pb_saxs_screen->setMinimumHeight(minHeight1);
      SET_WIDTH_FROM_TEXT_LEN( pb_saxs_screen );
      pb_saxs_screen->setPalette( PALET_PUSHB );
      connect(pb_saxs_screen, SIGNAL(clicked()), SLOT(saxs_screen()));
      iq_widgets.push_back( pb_saxs_screen );
   }
   
   cb_guinier = new QCheckBox(this);
   cb_guinier->setText(us_tr(" Guinier "));
   // cb_guinier->setMinimumHeight(minHeight1);
   cb_guinier->setEnabled(true);
   cb_guinier->setChecked(false);
   cb_guinier->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_guinier->setPalette( PALET_NORMAL );
   AUTFBACK( cb_guinier );
   connect(cb_guinier, SIGNAL(clicked()), SLOT(set_guinier()));
   iq_widgets.push_back( cb_guinier );

   cb_cs_guinier = new QCheckBox(this);
   cb_cs_guinier->setText(us_tr("CS"));
   // cb_cs_guinier->setMinimumHeight(minHeight1);
   cb_cs_guinier->setEnabled(true);
   cb_cs_guinier->setChecked(false);
   cb_cs_guinier->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_cs_guinier->setPalette( PALET_NORMAL );
   AUTFBACK( cb_cs_guinier );
   connect(cb_cs_guinier, SIGNAL(clicked()), SLOT(set_cs_guinier()));
   iq_widgets.push_back( cb_cs_guinier );

   cb_Rt_guinier = new QCheckBox(this);
   cb_Rt_guinier->setText(us_tr("TV    q^2 range:"));
   // cb_Rt_guinier->setMinimumHeight(minHeight1);
   cb_Rt_guinier->setEnabled(true);
   cb_Rt_guinier->setChecked(false);
   cb_Rt_guinier->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_Rt_guinier->setPalette( PALET_NORMAL );
   AUTFBACK( cb_Rt_guinier );
   connect(cb_Rt_guinier, SIGNAL(clicked()), SLOT(set_Rt_guinier()));
   iq_widgets.push_back( cb_Rt_guinier );

   le_guinier_lowq2 = new QLineEdit( this );    le_guinier_lowq2->setObjectName( "guinier_lowq2 Line Edit" );
   le_guinier_lowq2->setText("");
   // le_guinier_lowq2->setMinimumHeight(minHeight1);
   le_guinier_lowq2->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_lowq2->setPalette( PALET_NORMAL );
   AUTFBACK( le_guinier_lowq2 );
   le_guinier_lowq2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_guinier_lowq2, SIGNAL(textChanged(const QString &)), SLOT(update_guinier_lowq2(const QString &)));
   iq_widgets.push_back( le_guinier_lowq2 );

   le_guinier_highq2 = new QLineEdit( this );    le_guinier_highq2->setObjectName( "guinier_highq2 Line Edit" );
   le_guinier_highq2->setText("");
   // le_guinier_highq2->setMinimumHeight(minHeight1);
   le_guinier_highq2->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_highq2->setPalette( PALET_NORMAL );
   AUTFBACK( le_guinier_highq2 );
   le_guinier_highq2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_guinier_highq2, SIGNAL(textChanged(const QString &)), SLOT(update_guinier_highq2(const QString &)));
   iq_widgets.push_back( le_guinier_highq2 );

   cb_user_range = new QCheckBox(this);
   cb_user_range->setText(us_tr(" Standard "));
   // cb_user_range->setMinimumHeight(minHeight1);
   cb_user_range->setEnabled(true);
   cb_user_range->setChecked(false);
   cb_user_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_user_range->setPalette( PALET_NORMAL );
   AUTFBACK( cb_user_range );
   connect(cb_user_range, SIGNAL(clicked()), SLOT(set_user_range()));
   iq_widgets.push_back( cb_user_range );

   cb_kratky = new QCheckBox(this);
   cb_kratky->setText(us_tr("Kratky plot     q range:"));
   // cb_kratky->setMinimumHeight(minHeight1);
   cb_kratky->setEnabled(true);
   cb_kratky->setChecked(false);
   cb_kratky->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_kratky->setPalette( PALET_NORMAL );
   AUTFBACK( cb_kratky );
   connect(cb_kratky, SIGNAL(clicked()), SLOT(set_kratky()));
   iq_widgets.push_back( cb_kratky );

   le_user_lowq = new QLineEdit( this );    le_user_lowq->setObjectName( "user_lowq Line Edit" );
   le_user_lowq->setText("");
   // le_user_lowq->setMinimumHeight(minHeight1);
   le_user_lowq->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_user_lowq->setPalette( PALET_NORMAL );
   AUTFBACK( le_user_lowq );
   le_user_lowq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_user_lowq, SIGNAL(textChanged(const QString &)), SLOT(update_user_lowq(const QString &)));
   iq_widgets.push_back( le_user_lowq );

   le_user_highq = new QLineEdit( this );    le_user_highq->setObjectName( "user_highq Line Edit" );
   le_user_highq->setText("");
   // le_user_highq->setMinimumHeight(minHeight1);
   le_user_highq->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_user_highq->setPalette( PALET_NORMAL );
   AUTFBACK( le_user_highq );
   le_user_highq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_user_highq, SIGNAL(textChanged(const QString &)), SLOT(update_user_highq(const QString &)));
   iq_widgets.push_back( le_user_highq );

   le_user_lowI = new QLineEdit( this );    le_user_lowI->setObjectName( "user_lowI Line Edit" );
   le_user_lowI->setText("");
   // le_user_lowI->setMinimumHeight(minHeight1);
   le_user_lowI->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_user_lowI->setPalette( PALET_NORMAL );
   AUTFBACK( le_user_lowI );
   le_user_lowI->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_user_lowI, SIGNAL(textChanged(const QString &)), SLOT(update_user_lowI(const QString &)));
   iq_widgets.push_back( le_user_lowI );

   le_user_highI = new QLineEdit( this );    le_user_highI->setObjectName( "user_highI Line Edit" );
   le_user_highI->setText("");
   // le_user_highI->setMinimumHeight(minHeight1);
   le_user_highI->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_user_highI->setPalette( PALET_NORMAL );
   AUTFBACK( le_user_highI );
   le_user_highI->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_user_highI, SIGNAL(textChanged(const QString &)), SLOT(update_user_highI(const QString &)));
   iq_widgets.push_back( le_user_highI );

   pb_saxs_buffer = new QPushButton("Data", this);
   pb_saxs_buffer->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_saxs_buffer->setMinimumHeight(minHeight1);
   SET_WIDTH_FROM_TEXT_LEN( pb_saxs_buffer );
   pb_saxs_buffer->setPalette( PALET_PUSHB );
   connect(pb_saxs_buffer, SIGNAL(clicked()), SLOT(saxs_buffer()));
   iq_widgets.push_back( pb_saxs_buffer );

   pb_dad = new QPushButton("UV-Vis", this);
   pb_dad->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_dad->setMinimumHeight(minHeight1);
   SET_WIDTH_FROM_TEXT_LEN( pb_dad );
   pb_dad->setPalette( PALET_PUSHB );
   connect(pb_dad, SIGNAL(clicked()), SLOT(dad()));
   iq_widgets.push_back( pb_dad );

   pb_saxs_hplc = new QPushButton("HPLC/KIN", this);
   pb_saxs_hplc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_saxs_hplc->setMinimumHeight(minHeight1);
   SET_WIDTH_FROM_TEXT_LEN( pb_saxs_hplc );
   pb_saxs_hplc->setPalette( PALET_PUSHB );
   connect(pb_saxs_hplc, SIGNAL(clicked()), SLOT(saxs_hplc()));
   iq_widgets.push_back( pb_saxs_hplc );

   pb_mals = new QPushButton("MALS", this);
   pb_mals->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_mals->setMinimumHeight(minHeight1);
   SET_WIDTH_FROM_TEXT_LEN( pb_mals );
   pb_mals->setPalette( PALET_PUSHB );
   connect(pb_mals, SIGNAL(clicked()), SLOT(mals()));
   iq_widgets.push_back( pb_mals );

   pb_mals_saxs = new QPushButton("MALS+SAXS", this);
   pb_mals_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_mals_saxs->setMinimumHeight(minHeight1);
   SET_WIDTH_FROM_TEXT_LEN( pb_mals_saxs );
   pb_mals_saxs->setPalette( PALET_PUSHB );
   connect(pb_mals_saxs, SIGNAL(clicked()), SLOT(mals_saxs()));
   iq_widgets.push_back( pb_mals_saxs );

   if ( started_in_expert_mode )
   {

      pb_saxs_xsr = new QPushButton("CSA", this);
      pb_saxs_xsr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      pb_saxs_xsr->setMinimumHeight(minHeight1);
      pb_saxs_xsr->setPalette( PALET_PUSHB );
      SET_WIDTH_FROM_TEXT_LEN( pb_saxs_xsr );
      connect(pb_saxs_xsr, SIGNAL(clicked()), SLOT(saxs_xsr()));
      iq_widgets.push_back( pb_saxs_xsr );

      pb_saxs_1d = new QPushButton("1d", this);
      pb_saxs_1d->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      pb_saxs_1d->setMinimumHeight(minHeight1);
      pb_saxs_1d->setPalette( PALET_PUSHB );
      SET_WIDTH_FROM_TEXT_LEN( pb_saxs_1d );
      connect(pb_saxs_1d, SIGNAL(clicked()), SLOT(saxs_1d()));
      iq_widgets.push_back( pb_saxs_1d );

      pb_saxs_2d = new QPushButton("2d", this);
      pb_saxs_2d->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      pb_saxs_2d->setMinimumHeight(minHeight1);
      pb_saxs_2d->setPalette( PALET_PUSHB );
      SET_WIDTH_FROM_TEXT_LEN( pb_saxs_2d );
      connect(pb_saxs_2d, SIGNAL(clicked()), SLOT(saxs_2d()));
      iq_widgets.push_back( pb_saxs_2d );
   } 

   pb_guinier_analysis = new QPushButton("Guinier", this);
   pb_guinier_analysis->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_guinier_analysis->setMinimumHeight(minHeight1);
   SET_WIDTH_FROM_TEXT_LEN( pb_guinier_analysis );
   pb_guinier_analysis->setEnabled(true);
   pb_guinier_analysis->setPalette( PALET_PUSHB );
   connect(pb_guinier_analysis, SIGNAL(clicked()), SLOT( guinier_window() ) );
   iq_widgets.push_back( pb_guinier_analysis );

   pb_guinier_cs = new QPushButton("CS-Guinier", this);
   pb_guinier_cs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_guinier_cs->setMinimumHeight(minHeight1);
   SET_WIDTH_FROM_TEXT_LEN( pb_guinier_cs );
   pb_guinier_cs->setEnabled(true);
   pb_guinier_cs->setPalette( PALET_PUSHB );
   connect(pb_guinier_cs, SIGNAL(clicked()), SLOT(run_guinier_cs()));
   pb_guinier_cs->hide(); // iq_widgets.push_back( pb_guinier_cs );

#if defined(ADD_GUINIER)      
   lbl_guinier_cutoff = new QLabel(us_tr("Guinier cutoff\n(1/Angstrom^2) : "), this);
   lbl_guinier_cutoff->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_guinier_cutoff->setMinimumHeight(minHeight1);
   lbl_guinier_cutoff->setPalette( PALET_LABEL );
   AUTFBACK( lbl_guinier_cutoff );
   lbl_guinier_cutoff->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));
   iq_widgets.push_back( lbl_guinier_cutoff );

   cnt_guinier_cutoff= new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_guinier_cutoff );
   cnt_guinier_cutoff->setRange(0.01, 100); cnt_guinier_cutoff->setSingleStep( 0.01);
   cnt_guinier_cutoff->setValue(guinier_cutoff);
   cnt_guinier_cutoff->setMinimumHeight(minHeight1);
   cnt_guinier_cutoff->setEnabled(true);
   cnt_guinier_cutoff->setNumButtons(3);
   cnt_guinier_cutoff->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_guinier_cutoff->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_guinier_cutoff );
   connect(cnt_guinier_cutoff, SIGNAL(valueChanged(double)), SLOT(update_guinier_cutoff(double)));
   iq_widgets.push_back( cnt_guinier_cutoff );
#endif

   lbl_pr = new mQLabel(us_tr("P(r) vs. r Functions:"), this);
   lbl_pr->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_pr->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_pr->setMinimumHeight(minHeight1);
   lbl_pr->setPalette( PALET_FRAME );
   AUTFBACK( lbl_pr );
   lbl_pr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   connect( lbl_pr, SIGNAL( pressed() ), SLOT( hide_pr() ) );

   lbl_bin_size = new QLabel(us_tr(" Bin size (Angstrom): "), this);
   lbl_bin_size->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_bin_size->setMinimumHeight(minHeight1);
   lbl_bin_size->setPalette( PALET_LABEL );
   AUTFBACK( lbl_bin_size );
   lbl_bin_size->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));
   pr_widgets.push_back( lbl_bin_size );

   cnt_bin_size = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_bin_size );
   // #warning DIST change for testing
   // cnt_bin_size->setRange(0.000001, 100); cnt_bin_size->setSingleStep( 0.000001 );
   cnt_bin_size->setRange(0.01, 100); cnt_bin_size->setSingleStep( 0.01 );
   cnt_bin_size->setValue(our_saxs_options->bin_size);
   cnt_bin_size->setMinimumHeight(minHeight1);
   cnt_bin_size->setEnabled(true);
   cnt_bin_size->setNumButtons(3);
   cnt_bin_size->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_bin_size->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_bin_size );
   connect(cnt_bin_size, SIGNAL(valueChanged(double)), SLOT(update_bin_size(double)));
   pr_widgets.push_back( cnt_bin_size );


   lbl_smooth = new QLabel(us_tr(" Smoothing: "), this);
   lbl_smooth->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_smooth->setMinimumHeight(minHeight1);
   lbl_smooth->setPalette( PALET_LABEL );
   AUTFBACK( lbl_smooth );
   lbl_smooth->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));
   pr_widgets.push_back( lbl_smooth );

   cnt_smooth = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_smooth );
   cnt_smooth->setRange(0, 99); cnt_smooth->setSingleStep( 1);
   cnt_smooth->setValue(our_saxs_options->smooth);
   cnt_smooth->setMinimumHeight(minHeight1);
   cnt_smooth->setEnabled(true);
   cnt_smooth->setNumButtons(2);
   cnt_smooth->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_smooth->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_smooth );
   connect(cnt_smooth, SIGNAL(valueChanged(double)), SLOT(update_smooth(double)));
   pr_widgets.push_back( cnt_smooth );

   rb_curve_raw = new QRadioButton(us_tr("Raw"), this);
   rb_curve_raw->setEnabled(true);
   rb_curve_raw->setChecked(our_saxs_options->curve == 0);
   rb_curve_raw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_curve_raw->setPalette( PALET_NORMAL );
   AUTFBACK( rb_curve_raw );
   pr_widgets.push_back( rb_curve_raw );

   //   rb_curve_saxs_dry = new QRadioButton(us_tr("SAXS (unc)"), this);
   //   rb_curve_saxs_dry->setEnabled(true);
   //   rb_curve_saxs_dry->setChecked(our_saxs_options->curve == 1);
   //   rb_curve_saxs_dry->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   rb_curve_saxs_dry->setPalette( PALET_NORMAL );
   // pr_widgets.push_back( rb_curve_saxs_dry );

   rb_curve_saxs = new QRadioButton(us_tr("SAXS"), this);
   rb_curve_saxs->setEnabled(true);
   rb_curve_saxs->setChecked(our_saxs_options->curve == 1);
   rb_curve_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_curve_saxs->setPalette( PALET_NORMAL );
   AUTFBACK( rb_curve_saxs );
   pr_widgets.push_back( rb_curve_saxs );

   rb_curve_sans = new QRadioButton(us_tr("SANS"), this);
   rb_curve_sans->setEnabled(true);
   rb_curve_sans->setChecked(our_saxs_options->curve == 2);
   rb_curve_sans->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_curve_sans->setPalette( PALET_NORMAL );
   AUTFBACK( rb_curve_sans );
   pr_widgets.push_back( rb_curve_sans );

#if 1 // QT_VERSION < 0x040000
   bg_curve = new QButtonGroup( this );
   bg_pos = 0;
   bg_curve->setExclusive(true);
   bg_curve->addButton( rb_curve_raw, bg_pos++ );
   //   bg_curve->addButton( rb_curve_saxs_dry, bg_pos++ );
   bg_curve->addButton( rb_curve_saxs, bg_pos++ );
   bg_curve->addButton( rb_curve_sans, bg_pos++ );
   connect(bg_curve, SIGNAL(buttonClicked(int)), SLOT(set_curve(int)));
   // pr_widgets.push_back( bg_curve );
#else
   bg_curve = new QGroupBox();
   bg_curve->setFlat( true );

   connect( rb_curve_raw, SIGNAL( clicked() ), this, SLOT( set_curve() ) );
   connect( rb_curve_saxs, SIGNAL( clicked() ), this, SLOT( set_curve() ) );
   connect( rb_curve_sans, SIGNAL( clicked() ), this, SLOT( set_curve() ) );
   {
      QHBoxLayout * bl = new QHBoxLayout; bl->setContentsMargins( 0, 0, 0, 0 ); bl->setSpacing( 0 );
      bl->addWidget( rb_curve_raw );
      bl->addWidget( rb_curve_saxs );
      bl->addWidget( rb_curve_sans );
      bg_curve->setLayout( bl );
   }
#endif
   cb_normalize = new QCheckBox(this);
   cb_normalize->setText(us_tr(" Normalize"));
   cb_normalize->setEnabled(true);
   cb_normalize->setChecked(true);
   cb_normalize->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_normalize->setPalette( PALET_NORMAL );
   AUTFBACK( cb_normalize );
   pr_widgets.push_back( cb_normalize );

   cb_pr_contrib = new QCheckBox(this);
   cb_pr_contrib->setText(us_tr(" Residue contrib.  range (Angstrom):"));
   cb_pr_contrib->setEnabled(true);
   cb_pr_contrib->setChecked(false);
   cb_pr_contrib->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   cb_pr_contrib->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pr_contrib );
   connect(cb_pr_contrib, SIGNAL(clicked()), SLOT(set_pr_contrib()));
   pr_widgets.push_back( cb_pr_contrib );

   le_pr_contrib_low = new QLineEdit( this );    le_pr_contrib_low->setObjectName( "pr_contrib_low Line Edit" );
   le_pr_contrib_low->setText("");
   le_pr_contrib_low->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_pr_contrib_low->setPalette( PALET_NORMAL );
   AUTFBACK( le_pr_contrib_low );
   le_pr_contrib_low->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_pr_contrib_low, SIGNAL(textChanged(const QString &)), SLOT(update_pr_contrib_low(const QString &)));
   pr_widgets.push_back( le_pr_contrib_low );

   le_pr_contrib_high = new QLineEdit( this );    le_pr_contrib_high->setObjectName( "pr_contrib_high Line Edit" );
   le_pr_contrib_high->setText("");
   le_pr_contrib_high->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_pr_contrib_high->setPalette( PALET_NORMAL );
   AUTFBACK( le_pr_contrib_high );
   le_pr_contrib_high->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_pr_contrib_high, SIGNAL(textChanged(const QString &)), SLOT(update_pr_contrib_high(const QString &)));
   pr_widgets.push_back( le_pr_contrib_high );

   pb_pr_contrib = new QPushButton(us_tr("Display"), this);
   pb_pr_contrib->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_pr_contrib->setMinimumHeight(minHeight1);
   SET_WIDTH_FROM_TEXT_LEN( pb_pr_contrib );
   pb_pr_contrib->setPalette( PALET_PUSHB );
   pb_pr_contrib->setEnabled(false);
   connect(pb_pr_contrib, SIGNAL(clicked()), SLOT(show_pr_contrib()));
   pr_widgets.push_back( pb_pr_contrib );

   pb_plot_pr = new QPushButton(us_tr("Compute P(r) Distribution"), this);
   pb_plot_pr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_plot_pr->setMinimumHeight(minHeight1);
   SET_WIDTH_FROM_TEXT_LEN( pb_plot_pr );
   pb_plot_pr->setPalette( PALET_PUSHB );
   connect(pb_plot_pr, SIGNAL(clicked()), SLOT(show_plot_pr()));
   pr_widgets.push_back( pb_plot_pr );

   pb_pr_to_iq = new QPushButton( QString( "P(r) %1 I(q)" ).arg( u8"\u2192" ), this);
   pb_pr_to_iq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_pr_to_iq->setMinimumHeight(minHeight1);
   SET_WIDTH_FROM_TEXT_LEN( pb_pr_to_iq );
   pb_pr_to_iq->setPalette( PALET_PUSHB );
   connect(pb_pr_to_iq, SIGNAL(clicked()), SLOT(pr_to_iq()));
#warning restore when back to pr->iq testing
   // pr_widgets.push_back( pb_pr_to_iq );
   pb_pr_to_iq->hide();

   pb_load_pr = new QPushButton(us_tr("Load P(r) Distribution"), this);
   pb_load_pr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_load_pr->setMinimumHeight(minHeight1);
   SET_WIDTH_FROM_TEXT_LEN( pb_load_pr );
   pb_load_pr->setPalette( PALET_PUSHB );
   connect(pb_load_pr, SIGNAL(clicked()), SLOT(load_pr()));
   pr_widgets.push_back( pb_load_pr );

   pb_load_plot_pr = new QPushButton(us_tr("Load Plotted P(r)"), this);
   pb_load_plot_pr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_load_plot_pr->setMinimumHeight(minHeight1);
   SET_WIDTH_FROM_TEXT_LEN( pb_load_plot_pr );
   pb_load_plot_pr->setPalette( PALET_PUSHB );
   connect(pb_load_plot_pr, SIGNAL(clicked()), SLOT(load_plot_pr()));
   pr_widgets.push_back( pb_load_plot_pr );

   pb_clear_plot_pr = new QPushButton(us_tr("Clear P(r) Distribution"), this);
   pb_clear_plot_pr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_clear_plot_pr->setMinimumHeight(minHeight1);
   SET_WIDTH_FROM_TEXT_LEN( pb_clear_plot_pr );
   pb_clear_plot_pr->setPalette( PALET_PUSHB );
   connect(pb_clear_plot_pr, SIGNAL(clicked()), SLOT(clear_plot_pr()));
   pr_widgets.push_back( pb_clear_plot_pr );

   pb_pr_legend = new QPushButton(us_tr("Legend"), this);
   pb_pr_legend->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_pr_legend->setMinimumHeight(minHeight1);
   SET_WIDTH_FROM_TEXT_LEN( pb_pr_legend );
   pb_pr_legend->setPalette( PALET_PUSHB );
   connect(pb_pr_legend, SIGNAL(clicked()), SLOT(pr_legend()));
   pr_widgets.push_back( pb_pr_legend );

   pb_width2 = new QPushButton(us_tr( "Width" ), this);
   pb_width2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_width2->setMinimumHeight(minHeight1);
   SET_WIDTH_FROM_TEXT_LEN( pb_width2 );
   pb_width2->setPalette( PALET_PUSHB );
   connect(pb_width2, SIGNAL(clicked()), SLOT(set_width()));
   pr_widgets.push_back( pb_width2 );

   pb_pr_info = new QPushButton(us_tr( "info" ), this);
   pb_pr_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_pr_info->setMinimumHeight(minHeight1);
   SET_WIDTH_FROM_TEXT_LEN( pb_pr_info );
   pb_pr_info->setPalette( PALET_PUSHB );
   connect(pb_pr_info, SIGNAL(clicked()), SLOT(pr_info()));
   // pr_widgets.push_back( pb_pr_info );
#warning restore info when back to vdw tests
   pb_pr_info->hide();

   pb_pr_info2 = new QPushButton(us_tr( "info2" ), this);
   pb_pr_info2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_pr_info2->setMinimumHeight(minHeight1);
   SET_WIDTH_FROM_TEXT_LEN( pb_pr_info2 );
   pb_pr_info2->setPalette( PALET_PUSHB );
   connect(pb_pr_info2, SIGNAL(clicked()), SLOT(pr_info2()));
   // pr_widgets.push_back( pb_pr_info2 );
#warning restore info2 when back to vdw tests
   pb_pr_info2->hide();

   cb_pr_eb = new QCheckBox(this);
   cb_pr_eb->setText(us_tr("Err "));
   cb_pr_eb->setMaximumWidth ( minHeight1 * 2 );
   cb_pr_eb->setChecked( false );
   cb_pr_eb->setMinimumHeight( minHeight1 );
   cb_pr_eb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ) );
   cb_pr_eb->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pr_eb );
   connect( cb_pr_eb, SIGNAL( clicked() ), SLOT( set_pr_eb() ) );
   pr_widgets.push_back( cb_pr_eb );

   pb_stop = new QPushButton(us_tr("Stop"), this);
   pb_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_stop->setMinimumHeight(minHeight1);
   pb_stop->setPalette( PALET_PUSHB );
   connect(pb_stop, SIGNAL(clicked()), SLOT(stop()));

   pb_options = new QPushButton(us_tr("Open Options Panel"), this);
   pb_options->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_options->setMinimumHeight(minHeight1);
   pb_options->setPalette( PALET_PUSHB );
   connect(pb_options, SIGNAL(clicked()), SLOT(options()));

   pb_help = new QPushButton(us_tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(us_tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

//   plot_saxs = new QwtPlot(this);
   usp_plot_saxs = new US_Plot( plot_saxs, "", "", "", this );
   connect( (QWidget *)plot_saxs->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_saxs( const QPoint & ) ) );
   ((QWidget *)plot_saxs->titleLabel())->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_saxs->axisWidget( QwtPlot::yLeft ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_saxs( const QPoint & ) ) );
   ((QWidget *)plot_saxs->axisWidget( QwtPlot::yLeft ))->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_saxs->axisWidget( QwtPlot::xBottom ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_saxs( const QPoint & ) ) );
   ((QWidget *)plot_saxs->axisWidget( QwtPlot::xBottom ))->setContextMenuPolicy( Qt::CustomContextMenu );
   plot_info[ "US-SOMO SAXS Main" ] = plot_saxs;

   iq_widgets.push_back( plot_saxs );
#if QT_VERSION < 0x040000
   // plot_saxs->enableOutline(true);
   plot_saxs->setOutlinePen(Qt::white);
   plot_saxs->setOutlineStyle(Qwt::VLine);
   plot_saxs->enableGridXMin();
   plot_saxs->enableGridYMin();
#else
   grid_saxs = new QwtPlotGrid;
   grid_saxs->enableXMin( true );
   grid_saxs->enableYMin( true );
#endif
   plot_saxs->setPalette( PALET_NORMAL );
   AUTFBACK( plot_saxs );
#if QT_VERSION < 0x040000
   plot_saxs->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot_saxs->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   grid_saxs->setMajorPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid_saxs->setMinorPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid_saxs->attach( plot_saxs );
#endif
   plot_saxs->setAxisTitle( QwtPlot::xBottom, cb_guinier->isChecked() ? us_tr( "q^2 (1/Angstrom^2)" ) : us_tr( "q (1/Angstrom)" ) );
   plot_saxs->setAxisTitle( QwtPlot::yLeft,   
                            cb_kratky ->isChecked() ? 
                            us_tr( " q^2 * I(q)"        ) : 
                            ( cb_guinier->isChecked() && cb_cs_guinier->isChecked() ? us_tr( "q*I(q) (log scale)" ) : 
                              ( cb_guinier->isChecked() && cb_Rt_guinier->isChecked() ? us_tr( "q^2*I(q) (log scale)" ) : us_tr( "I(q) (log scale)" ) ) )
                            );
#if QT_VERSION < 0x040000
   plot_saxs->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 2, QFont::Bold));
   plot_saxs->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_saxs->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   plot_saxs->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
#endif
   plot_saxs->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   plot_saxs->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
#endif
   plot_saxs->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
//    plot_saxs->setMargin(USglobal->config_list.margin);
   plot_saxs->setTitle("");

#if QT_VERSION < 0x040000
   plot_saxs->setAxisOptions(QwtPlot::yLeft, 
                             cb_kratky->isChecked() ? 
                             QwtAutoScale::None :
                             QwtAutoScale::Logarithmic
                             );
#else
   plot_saxs->setAxisScaleEngine(QwtPlot::yLeft, 
                                 cb_kratky->isChecked() ?
                                 new QwtLogScaleEngine(10) :  // fix this
                                 new QwtLogScaleEngine(10));
   // plot_saxs->setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine(10));
#endif
   plot_saxs->setCanvasBackground(USglobal->global_colors.plot);
#if QT_VERSION < 0x040000
   plot_saxs->setAutoLegend( false );
   plot_saxs->setLegendFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ) );
   connect( plot_saxs, SIGNAL( legendClicked( long ) ), SLOT( plot_saxs_clicked( long ) ) );
#endif
   plot_saxs->setAxisScale( QwtPlot::xBottom, 0e0, 1e0 );

   plot_saxs_zoomer = new ScrollZoomer(plot_saxs->canvas());
   plot_saxs_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
   
//   plot_pr = new QwtPlot(this);
   usp_plot_pr = new US_Plot( plot_pr, "", "", "", this );
   connect( (QWidget *)plot_pr->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_pr( const QPoint & ) ) );
   ((QWidget *)plot_pr->titleLabel())->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_pr->axisWidget( QwtPlot::yLeft ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_pr( const QPoint & ) ) );
   ((QWidget *)plot_pr->axisWidget( QwtPlot::yLeft ))->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_pr->axisWidget( QwtPlot::xBottom ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_pr( const QPoint & ) ) );
   ((QWidget *)plot_pr->axisWidget( QwtPlot::xBottom ))->setContextMenuPolicy( Qt::CustomContextMenu );
   plot_info[ "US-SOMO SAXS p(r)" ] = plot_pr;
   pr_widgets.push_back( plot_pr );
#if QT_VERSION < 0x040000
   // plot_pr->enableOutline(true);
   plot_pr->setOutlinePen(Qt::white);
   plot_pr->setOutlineStyle(Qwt::VLine);
   plot_pr->enableGridXMin();
   plot_pr->enableGridYMin();
#else
   grid_pr = new QwtPlotGrid;
   grid_pr->enableXMin( true );
   grid_pr->enableYMin( true );
#endif
   plot_pr->setPalette( PALET_NORMAL );
   AUTFBACK( plot_pr );
#if QT_VERSION < 0x040000
   plot_pr->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot_pr->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   grid_pr->setMajorPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid_pr->setMinorPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid_pr->attach( plot_pr );
#endif
   plot_pr->setAxisTitle(QwtPlot::xBottom, us_tr("Distance (Angstrom)"));
   plot_pr->setAxisTitle(QwtPlot::yLeft, us_tr("Frequency"));
#if QT_VERSION < 0x040000
   plot_pr->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 2, QFont::Bold));
   plot_pr->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_pr->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   plot_pr->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
#endif
   plot_pr->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   plot_pr->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
#endif
   plot_pr->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
//    plot_pr->setMargin(USglobal->config_list.margin);
   plot_pr->setTitle(us_tr("P(r) Distribution Curve"));
   plot_pr->setCanvasBackground(USglobal->global_colors.plot);

#if QT_VERSION < 0x040000
   plot_pr->setAutoLegend( false );
   plot_pr->setLegendFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ) );
   connect( plot_pr, SIGNAL( legendClicked( long ) ), SLOT( plot_pr_clicked( long ) ) );
#endif

//   plot_resid = new QwtPlot(this);
   usp_plot_resid = new US_Plot( plot_resid, "", "", "", this );
   connect( (QWidget *)plot_resid->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_resid( const QPoint & ) ) );
   ((QWidget *)plot_resid->titleLabel())->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_resid->axisWidget( QwtPlot::yLeft ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_resid( const QPoint & ) ) );
   ((QWidget *)plot_resid->axisWidget( QwtPlot::yLeft ))->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_resid->axisWidget( QwtPlot::xBottom ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_resid( const QPoint & ) ) );
   ((QWidget *)plot_resid->axisWidget( QwtPlot::xBottom ))->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( plot_resid, SIGNAL( resized() ), SLOT( resid_resized() ) );
   connect( plot_saxs,  SIGNAL( resized() ), SLOT( resid_resized() ) );

   // doesn't seem useful
   // plot_resid->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine );
   plot_info[ "US-SOMO SAXS resid" ] = plot_resid;
#if QT_VERSION < 0x040000
   // plot_resid->enableOutline(true);
   // plot_resid->setOutlinePen(Qt::white);
   // plot_resid->setOutlineStyle(Qwt::VLine);
   plot_resid->enableGridXMin();
   plot_resid->enableGridYMin();
#else
   grid_resid = new QwtPlotGrid;
   grid_resid->enableXMin( true );
   grid_resid->enableYMin( true );
#endif
   plot_resid->setPalette( PALET_NORMAL );
   AUTFBACK( plot_resid );
#if QT_VERSION < 0x040000
   plot_resid->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot_resid->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   grid_resid->setMajorPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid_resid->setMinorPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid_resid->attach( plot_resid );
#endif
   // plot_resid->setAxisTitle(QwtPlot::xBottom, /* cb_guinier->isChecked() ? us_tr("q^2 (1/Angstrom^2)") : */  us_tr("q (1/Angstrom) or Frame"));
   // plot_resid->setAxisTitle(QwtPlot::yLeft, us_tr("I(q) (log scale)"));
   plot_resid->setAxisTitle( QwtPlot::xBottom, plot_saxs->axisTitle( QwtPlot::xBottom ) );

   plot_resid->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_resid->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_resid->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
//    plot_resid->setMargin(USglobal->config_list.margin);
   plot_resid->setTitle("");
   plot_resid->setCanvasBackground(USglobal->global_colors.plot);

   plot_resid_zoomer = new ScrollZoomer(plot_resid->canvas());
   plot_resid_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
   plot_resid->hide();
   plot_resid_zoomer->symmetric_rescale = true;
   resid_widgets.push_back( plot_resid );

   cb_resid_show = new QCheckBox(this);
   cb_resid_show->setText(us_tr(" Show residuals "));
   cb_resid_show->setEnabled(true);
   cb_resid_show->setChecked( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "saxs_cb_resid_show" ) &&
                              ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_cb_resid_show" ] == "1" );
   cb_resid_show->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_resid_show->setPalette( PALET_NORMAL );
   AUTFBACK( cb_resid_show );
   connect(cb_resid_show, SIGNAL(clicked()), SLOT(set_resid_show()));
   resid_widgets.push_back( cb_resid_show );
   cb_resid_show->hide();

   cb_resid_show_errorbars = new QCheckBox(this);
   cb_resid_show_errorbars->setText(us_tr(" SD error bars "));
   cb_resid_show_errorbars->setEnabled(true);
   cb_resid_show_errorbars->setChecked( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "saxs_cb_resid_show_errorbars" ) &&
                                        ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_cb_resid_show_errorbars" ] == "1" );
   cb_resid_show_errorbars->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_resid_show_errorbars->setPalette( PALET_NORMAL );
   AUTFBACK( cb_resid_show_errorbars );
   connect(cb_resid_show_errorbars, SIGNAL(clicked()), SLOT(set_resid_show_errorbars()));
   resid_widgets.push_back( cb_resid_show_errorbars );
   cb_resid_show_errorbars->hide();

   cb_manual_guinier = new QCheckBox(this);
   cb_manual_guinier->setText(us_tr(" Manual Guinier "));
   cb_manual_guinier->setEnabled(true);
   cb_manual_guinier->setChecked( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "saxs_cb_manual_guinier" ) &&
                              ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_cb_manual_guinier" ] == "1" );
   cb_manual_guinier->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_manual_guinier->setPalette( PALET_NORMAL );
   AUTFBACK( cb_manual_guinier );
   connect(cb_manual_guinier, SIGNAL(clicked()), SLOT(set_manual_guinier()));
   resid_widgets.push_back( cb_manual_guinier );
   cb_manual_guinier->hide();

   cb_resid_pct = new QCheckBox(this);
   cb_resid_pct->setText(us_tr(" By percent"));
   cb_resid_pct->setEnabled(true);
   cb_resid_pct->setChecked( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "saxs_cb_resid_pct" ) &&
                             ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_cb_resid_pct" ] == "1" );
   cb_resid_pct->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_resid_pct->setPalette( PALET_NORMAL );
   AUTFBACK( cb_resid_pct );
   connect(cb_resid_pct, SIGNAL(clicked()), SLOT(set_resid_pct()));
   resid_widgets.push_back( cb_resid_pct );
   cb_resid_pct->hide();

   cb_resid_sd = new QCheckBox(this);
   cb_resid_sd->setText(us_tr(" Use standard deviations "));
   cb_resid_sd->setEnabled(true);
   cb_resid_sd->setChecked( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "saxs_cb_resid_sd" ) &&
                            ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_cb_resid_sd" ] == "1" );
   cb_resid_sd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_resid_sd->setPalette( PALET_NORMAL );
   AUTFBACK( cb_resid_sd );
   connect(cb_resid_sd, SIGNAL(clicked()), SLOT(set_resid_sd()));
   resid_widgets.push_back( cb_resid_sd );
   cb_resid_sd->hide();

   le_manual_guinier_fit_start = new mQLineEdit( this );    le_manual_guinier_fit_start->setObjectName( "manual_guinier_fit_start Line Edit" );
   le_manual_guinier_fit_start->setText(us_tr(""));
   le_manual_guinier_fit_start->setMinimumHeight(minHeight1);
   le_manual_guinier_fit_start->setMaximumWidth( maxWidth * 6 );
   le_manual_guinier_fit_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_manual_guinier_fit_start->setPalette( PALET_NORMAL );
   AUTFBACK( le_manual_guinier_fit_start );
   le_manual_guinier_fit_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_manual_guinier_fit_start->setReadOnly(true);
   manual_guinier_widgets.push_back( le_manual_guinier_fit_start );
   connect( le_manual_guinier_fit_start, SIGNAL( textChanged( const QString & ) ), SLOT( manual_guinier_fit_start_text( const QString & ) ) );
   connect( le_manual_guinier_fit_start, SIGNAL( focussed ( bool ) )             , SLOT( manual_guinier_fit_start_focus( bool ) ) );

   le_manual_guinier_fit_end = new mQLineEdit( this );    le_manual_guinier_fit_end->setObjectName( "manual_guinier_fit_end Line Edit" );
   le_manual_guinier_fit_end->setText(us_tr(""));
   le_manual_guinier_fit_end->setMinimumHeight(minHeight1);
   le_manual_guinier_fit_end->setMaximumWidth( maxWidth * 6 );
   le_manual_guinier_fit_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_manual_guinier_fit_end->setPalette( PALET_NORMAL );
   AUTFBACK( le_manual_guinier_fit_end );
   le_manual_guinier_fit_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_manual_guinier_fit_end->setReadOnly(true);
   manual_guinier_widgets.push_back( le_manual_guinier_fit_end );
   connect( le_manual_guinier_fit_end, SIGNAL( textChanged( const QString & ) ), SLOT( manual_guinier_fit_end_text( const QString & ) ) );
   connect( le_manual_guinier_fit_end, SIGNAL( focussed ( bool ) )             , SLOT( manual_guinier_fit_end_focus( bool ) ) );

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
   manual_guinier_widgets.push_back( qwtw_wheel );
   qwtw_wheel->setMinimumWidth ( 350 );
   
   pb_manual_guinier_process = new QPushButton(us_tr("Process"), this);
   pb_manual_guinier_process->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_manual_guinier_process->setMinimumHeight(minHeight1);
   pb_manual_guinier_process->setPalette( PALET_PUSHB );
   connect(pb_manual_guinier_process, SIGNAL(clicked()), SLOT(manual_guinier_process()));
   manual_guinier_widgets.push_back( pb_manual_guinier_process );

   hide_widgets( manual_guinier_widgets );

   progress_saxs = new mQProgressBar( this );
   progress_saxs->setMinimumHeight(minHeight1);
   progress_saxs->setPalette( PALET_NORMAL );
   progress_saxs->set_cli_progress( ( ( US_Hydrodyn * ) us_hydrodyn )->cli_progress );
   AUTFBACK( progress_saxs );
   progress_saxs->reset();
   iq_widgets.push_back( progress_saxs );

   progress_pr = new mQProgressBar( this );
   progress_pr->setMinimumHeight(minHeight1);
   progress_pr->setPalette( PALET_NORMAL );
   progress_pr->set_cli_progress( ( ( US_Hydrodyn * ) us_hydrodyn )->cli_progress );
   AUTFBACK( progress_pr );
   progress_pr->reset();
   pr_widgets.push_back( progress_pr );

   editor = new mQTextEdit(this);
   editor->setPalette( PALET_NORMAL );
   editor->set_cli_progress( ( ( US_Hydrodyn * ) us_hydrodyn )->cli_progress );
   editor->set_cli_prefix( "saxs" );
   AUTFBACK( editor );
   editor->setReadOnly(true);
   editor->setMinimumWidth(300);
   editor->setMinimumHeight(minHeight1 * 6);

#if QT_VERSION < 0x040000
# if QT_VERSION >= 0x040000 && defined(Q_OS_MAC)
   {
 //      Q3PopupMenu * file = new Q3PopupMenu;
      file->insertItem( us_tr("&Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
      file->insertItem( us_tr("&Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );
#  ifndef NO_EDITOR_PRINT
      file->insertItem( us_tr("&Print"), this, SLOT(print()),   Qt::ALT+Qt::Key_P );
#  endif
      file->insertItem( us_tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );

      QMenuBar *menu = new QMenuBar( this );
      AUTFBACK( menu );

      menu->insertItem(us_tr("&Messages"), file );
   }
# else
   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight0);

   m = new QMenuBar( frame );    m->setObjectName( "menu" );
   m->setMinimumHeight(minHeight1 - 5);
   m->setPalette( PALET_NORMAL );
   AUTFBACK( m );
 //   Q3PopupMenu * file = new Q3PopupMenu(editor);
   m->insertItem( us_tr("&File"), file );
   file->insertItem( us_tr("Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
   file->insertItem( us_tr("Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );
#  ifndef NO_EDITOR_PRINT
   file->insertItem( us_tr("Print"), this, SLOT(print()),   Qt::ALT+Qt::Key_P );
#  endif
   file->insertItem( us_tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );
# endif
#else

# if defined(Q_OS_MAC)
   m = new QMenuBar( this );
   m->setObjectName( "menu" );
# else
   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight0);
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
   // editor->setWordWrapMode (QTextOption::NoWrap);

   lbl_core_progress = new QLabel("", this);
   Q_CHECK_PTR(lbl_core_progress);
   lbl_core_progress->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
   lbl_core_progress->setMinimumHeight(minHeight0);
   lbl_core_progress->setPalette( PALET_LABEL );
   AUTFBACK( lbl_core_progress );
   lbl_core_progress->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   int /* rows=13, columns = 3, */ spacing = 2, j=0, margin=4;
   QGridLayout * background = new QGridLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 ); background->setSpacing( spacing ); background->setContentsMargins( margin, margin, margin, margin );

   background->addWidget(lbl_filename1, j, 0);
   background->addWidget(te_filename2, j, 1);
   j++;

   background->addWidget( lbl_settings , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   j++;

   background->addWidget(pb_select_atom_file, j, 0);
   background->addWidget(lbl_atom_table, j, 1);
   j++;
   background->addWidget(pb_select_hybrid_file, j, 0);
   background->addWidget(lbl_hybrid_table, j, 1);
   j++;
   background->addWidget(pb_select_saxs_file, j, 0);
   background->addWidget(lbl_saxs_table, j, 1);
   j++;

   background->addWidget( lbl_iq , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   j++;


   QBoxLayout * hbl_load_saxs = new QHBoxLayout(); hbl_load_saxs->setContentsMargins( 0, 0, 0, 0 ); hbl_load_saxs->setSpacing( 0 );
   hbl_load_saxs->addWidget(pb_load_saxs_sans);
   hbl_load_saxs->addWidget(pb_load_gnom);
   hbl_load_saxs->addWidget(pb_load_plot_saxs);
   hbl_load_saxs->addWidget(pb_set_grid);
   hbl_load_saxs->addWidget(pb_clear_plot_saxs);
   hbl_load_saxs->addWidget(pb_width);
   hbl_load_saxs->addWidget(cb_eb);
   hbl_load_saxs->addWidget(pb_rescale);
   background->addLayout( hbl_load_saxs , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   j++;

   QBoxLayout * hbl_various_0 = new QHBoxLayout(); hbl_various_0->setContentsMargins( 0, 0, 0, 0 ); hbl_various_0->setSpacing( 0 );
   hbl_various_0->addWidget(pb_ift);
   hbl_various_0->addWidget(pb_saxs_search);
   if ( started_in_expert_mode )
   {
      hbl_various_0->addWidget(pb_saxs_screen);
   }

   hbl_various_0->addWidget(pb_saxs_buffer);
   hbl_various_0->addWidget(pb_dad);
   hbl_various_0->addWidget(pb_saxs_hplc);
   hbl_various_0->addWidget(pb_mals);
   hbl_various_0->addWidget(pb_mals_saxs);

   if ( started_in_expert_mode )
   {
      hbl_various_0->addWidget(pb_saxs_xsr);
      hbl_various_0->addWidget(pb_saxs_1d);
      hbl_various_0->addWidget(pb_saxs_2d);
   }

   hbl_various_0->addWidget(pb_guinier_analysis);
   hbl_various_0->addWidget(pb_guinier_cs);
   hbl_various_0->addWidget(pb_saxs_legend);
   hbl_various_0->addWidget(pb_pp);
   background->addLayout( hbl_various_0 , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   j++;


   QGridLayout * gl_plot_ctls = new QGridLayout( 0 ); gl_plot_ctls->setContentsMargins( 0, 0, 0, 0 ); gl_plot_ctls->setSpacing( 0 );

   gl_plot_ctls->addWidget( cb_guinier    , 0, 0 );
   gl_plot_ctls->addWidget( cb_cs_guinier , 0, 1 );
   gl_plot_ctls->addWidget( cb_Rt_guinier , 0, 2 );
   gl_plot_ctls->addWidget( cb_user_range , 1, 0 );
   gl_plot_ctls->addWidget( cb_kratky , 1 , 1 , 1 + ( 1 ) - ( 1 ) , 1 + ( 2  ) - ( 1 ) );

   //    QHBoxLayout * hbl_guinier = new QHBoxLayout; hbl_guinier->setContentsMargins( 0, 0, 0, 0 ); hbl_guinier->setSpacing( 0 );
   //    hbl_guinier->addWidget( cb_guinier );
   //    hbl_guinier->addWidget( cb_cs_guinier );
   //    background->addLayout(hbl_guinier, j, 0);

   QHBoxLayout * hbl_guinier_range = new QHBoxLayout; hbl_guinier_range->setContentsMargins( 0, 0, 0, 0 ); hbl_guinier_range->setSpacing( 0 );
   hbl_guinier_range->addWidget( le_guinier_lowq2 );
   hbl_guinier_range->addWidget( le_guinier_highq2 );
   background->addLayout(hbl_guinier_range, j, 1);
   j++;

#if defined(ADD_GUINIER)      
   hbl_guinier->addWidget(lbl_guinier_cutoff);
   hbl_guinier->addWidget(cnt_guinier_cutoff);
#endif

   //    QHBoxLayout * hbl_cb_std_kratky = new QHBoxLayout; hbl_cb_std_kratky->setContentsMargins( 0, 0, 0, 0 ); hbl_cb_std_kratky->setSpacing( 0 );
   //    hbl_cb_std_kratky->addWidget( cb_user_range );
   //    hbl_cb_std_kratky->addWidget( cb_kratky );
   //    background->addLayout( hbl_cb_std_kratky, j, 0 );

   background->addLayout( gl_plot_ctls , j-1 , 0 , 1 + ( j ) - ( j-1 ) , 1 + ( 0  ) - ( 0 ) );

   QHBoxLayout * hbl_user_range = new QHBoxLayout; hbl_user_range->setContentsMargins( 0, 0, 0, 0 ); hbl_user_range->setSpacing( 0 );
   hbl_user_range->addWidget(le_user_lowq);
   hbl_user_range->addWidget(le_user_highq);
   hbl_user_range->addWidget(le_user_lowI);
   hbl_user_range->addWidget(le_user_highI);
   background->addLayout(hbl_user_range, j, 1);
   j++;

   QHBoxLayout * hbl_tools = new QHBoxLayout; hbl_tools->setContentsMargins( 0, 0, 0, 0 ); hbl_tools->setSpacing( 0 );
   hbl_tools->addWidget(cb_create_native_saxs);
   background->addLayout( hbl_tools , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   j++;

   QBoxLayout * hbl_saxs_iq = new QHBoxLayout(); hbl_saxs_iq->setContentsMargins( 0, 0, 0, 0 ); hbl_saxs_iq->setSpacing( 0 );
   hbl_saxs_iq->addWidget(rb_saxs);
   hbl_saxs_iq->addWidget(rb_saxs_iq_native_debye);
   hbl_saxs_iq->addWidget(rb_saxs_iq_native_sh);
   if ( started_in_expert_mode )
   {
      hbl_saxs_iq->addWidget(rb_saxs_iq_native_hybrid);
      hbl_saxs_iq->addWidget(rb_saxs_iq_native_hybrid2);
      hbl_saxs_iq->addWidget(rb_saxs_iq_native_hybrid3);
   }
   hbl_saxs_iq->addWidget(rb_saxs_iq_native_fast);
   if ( started_in_expert_mode )
   {
      hbl_saxs_iq->addWidget(rb_saxs_iq_foxs);
   }
   hbl_saxs_iq->addWidget(rb_saxs_iq_crysol);
   if ( started_in_expert_mode )
   {
      hbl_saxs_iq->addWidget(rb_saxs_iq_sastbx);
   }
   background->addLayout( hbl_saxs_iq , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   j++;

   QBoxLayout * hbl_sans_iq = new QHBoxLayout(); hbl_sans_iq->setContentsMargins( 0, 0, 0, 0 ); hbl_sans_iq->setSpacing( 0 );
   hbl_sans_iq->addWidget(rb_sans);
   hbl_sans_iq->addWidget(rb_sans_iq_native_debye);
   hbl_sans_iq->addWidget(rb_sans_iq_native_sh);
   if ( started_in_expert_mode )
   {
      hbl_sans_iq->addWidget(rb_sans_iq_native_hybrid);
      hbl_sans_iq->addWidget(rb_sans_iq_native_hybrid2);
      hbl_sans_iq->addWidget(rb_sans_iq_native_hybrid3);
   }
   hbl_sans_iq->addWidget(rb_sans_iq_native_fast);
   hbl_sans_iq->addWidget(rb_sans_iq_cryson);
   background->addLayout( hbl_sans_iq , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   j++;
   
   QBoxLayout * hbl_iqq_suffix = new QHBoxLayout(); hbl_iqq_suffix->setContentsMargins( 0, 0, 0, 0 ); hbl_iqq_suffix->setSpacing( 0 );
   hbl_iqq_suffix->addWidget(lbl_iqq_suffix);
   hbl_iqq_suffix->addWidget(le_iqq_manual_suffix);
   hbl_iqq_suffix->addWidget(le_iqq_full_suffix);
   background->addLayout( hbl_iqq_suffix , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   j++;

   background->addWidget(pb_plot_saxs_sans, j, 0);
   background->addWidget(progress_saxs, j, 1);
   j++;

   background->addWidget( lbl_pr , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   j++;
   QBoxLayout * hbl_plot_pr = new QHBoxLayout(); hbl_plot_pr->setContentsMargins( 0, 0, 0, 0 ); hbl_plot_pr->setSpacing( 0 );
   hbl_plot_pr->addWidget(pb_load_pr);
   hbl_plot_pr->addWidget(pb_load_plot_pr);
   hbl_plot_pr->addWidget(pb_clear_plot_pr);
   hbl_plot_pr->addWidget(pb_pr_legend);
   hbl_plot_pr->addWidget(pb_width2);
   hbl_plot_pr->addWidget(cb_pr_eb);
   hbl_plot_pr->addWidget(pb_pr_info);
   hbl_plot_pr->addWidget(pb_pr_info2);
   background->addLayout( hbl_plot_pr , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   j++;
   background->addWidget(lbl_bin_size, j, 0);
   background->addWidget(cnt_bin_size, j, 1);
   j++;
   background->addWidget(lbl_smooth, j, 0);
   background->addWidget(cnt_smooth, j, 1);
   j++;
   QBoxLayout * bl = new QHBoxLayout(); bl->setContentsMargins( 0, 0, 0, 0 ); bl->setSpacing( 0 );
   bl->addWidget(rb_curve_raw);
   //   bl->addWidget(rb_curve_saxs_dry);
   bl->addWidget(rb_curve_saxs);
   bl->addWidget(rb_curve_sans);
   bl->addWidget(cb_normalize);
   background->addLayout( bl , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   j++;
   
   QBoxLayout * hbl_contrib = new QHBoxLayout(); hbl_contrib->setContentsMargins( 0, 0, 0, 0 ); hbl_contrib->setSpacing( 0 );
   hbl_contrib->addWidget(cb_pr_contrib);
   hbl_contrib->addWidget(le_pr_contrib_low);
   hbl_contrib->addWidget(le_pr_contrib_high);
   hbl_contrib->addWidget(pb_pr_contrib);

   background->addLayout( hbl_contrib , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   j++;

   {
      QBoxLayout * hbl = new QHBoxLayout( 0 ); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( pb_plot_pr );
      hbl->addWidget( pb_pr_to_iq );
      hbl->addWidget( progress_pr );
      background->addLayout( hbl, j, 0, 1, 2 );
   }
   // background->addWidget(pb_plot_pr, j, 0);
   // background->addWidget(pb_plot_pr, j, 0);
   // background->addWidget(progress_pr, j, 1);
   j++;
   //   background->addWidget(pb_load_pr, j, 0);
   // background->addWidget(pb_clear_plot_pr, j, 1);
   
#if QT_VERSION < 0x040000 || !defined(Q_OS_MAC)
   background->addWidget( frame , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   j++;
#endif
   background->addWidget( editor , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   //   background->addWidget( editor , j , 0 , 1 + ( j+3 ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   // background->addWidget( plot_pr , j-2 , 2 , 1 + ( j+3 ) - ( j-2 ) , 1 + ( 2 ) - ( 2 ) );
   // background->addWidget( plot_pr , j , 2 , 1 + ( j ) - ( j ) , 1 + ( 2 ) - ( 2 ) );
   j++;
   background->addWidget(pb_stop, j, 0);
   background->addWidget(pb_options, j, 1);
   j++;
   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_cancel, j, 1);
   // background->addWidget(lbl_core_progress, j, 2);

   QHBoxLayout * hbl_plot_resid_buttons = new QHBoxLayout(); hbl_plot_resid_buttons->setContentsMargins( 0, 0, 0, 0 ); hbl_plot_resid_buttons->setSpacing( 0 );
   hbl_plot_resid_buttons->addWidget( cb_resid_show );
   hbl_plot_resid_buttons->addWidget( cb_manual_guinier );
   hbl_plot_resid_buttons->addWidget( cb_resid_show_errorbars );
   hbl_plot_resid_buttons->addWidget( cb_resid_sd );
   hbl_plot_resid_buttons->addWidget( cb_resid_pct );
   hbl_plot_resid_buttons->addWidget( pb_rescale_y );

   QGridLayout * gl_manual_guinier = new QGridLayout( 0 ); gl_manual_guinier->setContentsMargins( 0, 0, 0, 0 ); gl_manual_guinier->setSpacing( 0 );
   gl_manual_guinier->addWidget( le_manual_guinier_fit_start, 0, 0 );
   gl_manual_guinier->addWidget( le_manual_guinier_fit_end  , 0, 1 );
   gl_manual_guinier->addWidget( qwtw_wheel, 0, 2 );
   gl_manual_guinier->addWidget( pb_manual_guinier_process, 0, 3 );
   gl_manual_guinier->setColumnStretch( 0, 1 );
   gl_manual_guinier->setColumnStretch( 1, 1 );
   gl_manual_guinier->setColumnStretch( 2, 2 );
   gl_manual_guinier->setColumnStretch( 3, 0 );

   QBoxLayout * qbl_resid = new QVBoxLayout( 0 ); qbl_resid->setContentsMargins( 0, 0, 0, 0 ); qbl_resid->setSpacing( 0 );
   qbl_resid->addWidget( plot_resid );
   qbl_resid->addLayout( hbl_plot_resid_buttons );
   qbl_resid->addLayout( gl_manual_guinier );

   qbl_plots = new QVBoxLayout(0); qbl_plots->setContentsMargins( 0, 0, 0, 0 ); qbl_plots->setSpacing( 0 );
   qbl_plots->addWidget( plot_saxs );
   qbl_plots->addLayout( qbl_resid );
   qbl_plots->addWidget( plot_pr );
   qbl_plots->addWidget( lbl_core_progress );
   background->addLayout( qbl_plots , 0 , 2 , 1 + ( j ) - ( 0 ) , 1 + ( 2 ) - ( 2 ) );

   background->setColumnMinimumWidth(2, 600);
   //   for ( int j = 0; j < 15; j++ )
   //   {
   //      background->setRowStretch(j, 0);
   //   }
   //   background->setRowStretch(13, 1);

   background->setColumnStretch(0, 1);
   background->setColumnStretch(1, 2);
   background->setColumnStretch(2, 10);
   update_saxs_sans();
   clear_plot_saxs();
   clear_plot_pr( true );
   if ( source )
   {
      our_saxs_options->curve = 0;
      rb_curve_raw->setChecked(true);
      rb_curve_saxs->setChecked(false);
      rb_curve_sans->setChecked(false);
      rb_curve_raw->setEnabled(false);
      rb_curve_saxs->setEnabled(false);
      rb_curve_sans->setEnabled(false);
      if ( bead_model_has_electrons() ) {
         rb_curve_raw->setEnabled(true);
         rb_curve_saxs->setChecked(true);
         rb_curve_saxs->setEnabled(true);
      }
   } else {
      rb_curve_raw->setEnabled(true);
      rb_curve_saxs->setEnabled(true);
      rb_curve_sans->setEnabled(true);
   }

   if ( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "saxs_settings" ) && ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_settings" ] == "hidden" )
   {
      hide_widgets( settings_widgets, true );
   }
   if ( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "saxs_pr" ) && ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_pr" ] == "hidden" )
   {
      hide_widgets( pr_widgets, true );
   }
   if ( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "saxs_iq" ) && ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_iq" ] == "hidden" )
   {
      hide_widgets( iq_widgets, true );
   }
   if ( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "saxs_plot_saxs" ) && ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_plot_saxs" ] == "hidden" )
   {
      plot_saxs->hide();
   } else {
      plot_saxs->show();
   }
      
   if ( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "saxs_plot_pr" ) && ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_plot_pr" ] == "hidden" )
   {
      plot_pr->hide();
   } else {
      plot_pr->show();
   }
}

void US_Hydrodyn_Saxs::cancel()
{
   close();
}

void US_Hydrodyn_Saxs::options()
{
   ((US_Hydrodyn *)us_hydrodyn)->show_saxs_options();
}

void US_Hydrodyn_Saxs::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo/somo_saxs.html");
}

void US_Hydrodyn_Saxs::stop()
{
   stopFlag = true;
   pb_stop->setEnabled(false);
}

void US_Hydrodyn_Saxs::closeEvent(QCloseEvent *e)
{
   *saxs_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

//--------- thread for saxs p(r) plot -----------

// #define DEBUG_THREAD

saxs_pr_thr_t::saxs_pr_thr_t(int a_thread) : QThread()
{
   thread = a_thread;
   work_to_do = 0;
   work_done = 1;
   work_to_do_waiters = 0;
   work_done_waiters = 0;
}

void saxs_pr_thr_t::saxs_pr_thr_setup(
                                      vector < saxs_atom > *atoms,
                                      vector < float > *hist,
                                      double delta,
                                      unsigned int threads,
                                      mQProgressBar *progress,
                                      QLabel *lbl_core_progress,
                                      bool *stopFlag,
                                      float b_bar_inv2
                                      )
{
   /* this starts up a new work load for the thread */
   this->atoms = atoms;
   this->hist = hist;
   this->delta = delta;
   this->threads = threads;
   this->progress = progress;
   this->lbl_core_progress = lbl_core_progress;
   this->stopFlag = stopFlag;
   this->b_bar_inv2 = b_bar_inv2;

   work_mutex.lock();
   work_to_do = 1;
   work_done = 0;
   work_mutex.unlock();
   cond_work_to_do.wakeOne();
#if defined(DEBUG_THREAD)
   cerr << "thread " << thread << " has new work to do\n";
#endif
}

void saxs_pr_thr_t::saxs_pr_thr_shutdown()
{
   /* this signals the thread to exit the run method */
   work_mutex.lock();
   work_to_do = -1;
   work_mutex.unlock();
   cond_work_to_do.wakeOne();

#if defined(DEBUG_THREAD)
   cerr << "thread " << thread << " shutdown requested\n";
#endif
}

void saxs_pr_thr_t::saxs_pr_thr_wait()
{
   /* this is for the master thread to wait until the work is done */
   work_mutex.lock();

#if defined(DEBUG_THREAD)
   cerr << "thread " << thread << " has a waiter\n";
#endif

   while(!work_done) {
      cond_work_done.wait(&work_mutex);
   }
   work_done = 0;
   work_mutex.unlock();

#if defined(DEBUG_THREAD)
   cerr << "thread " << thread << " waiter released\n";
#endif
}

int saxs_pr_thr_t::saxs_pr_thr_work_status()
{
   work_mutex.lock();
   int retval = work_done;
   work_mutex.unlock();
   return retval;
}

void saxs_pr_thr_t::run()
{
   while(1)
   {
      work_mutex.lock();
#if defined(DEBUG_THREAD)
      cerr << "thread " << thread << " waiting for work\n";
#endif
      work_to_do_waiters++;
      while(!work_to_do)
      {
         cond_work_to_do.wait(&work_mutex);
      }
      if(work_to_do == -1)
      {
#if defined(DEBUG_THREAD)
         cerr << "thread " << thread << " shutting down\n";
#endif
         work_mutex.unlock();
         return;
      }

      work_to_do_waiters = 0;
      work_mutex.unlock();
#if defined(DEBUG_THREAD)
      cerr << "thread " << thread << " starting work\n";
#endif
      
      unsigned int as = (*atoms).size();
      unsigned int as1 = as - 1;
      unsigned int pos;
      double rik; // distance from atom i to k 
      if ( !thread ) 
      {
         progress->setMaximum((int)(1.15f * as1 / threads));
      }
#if defined(DEBUG_THREAD)
      cerr << "thread " << thread << " as1 = " << as1 
           << endl;
#endif

      for ( unsigned int i = thread; i < as1; i += threads )
      {
#if defined(DEBUG_THREAD)
         cerr << "thread " << thread << " i = " << i << endl;
#endif
         if ( !thread ) 
         {
            // lbl_core_progress->setText(QString("Atom %1 of %2\n").arg(i+1).arg(as));
            progress->setValue(i+1);
            // qApp->processEvents();
         }
         if ( *stopFlag ) 
         {
            break;
         }
            
         for ( unsigned int k = i + 1; k < as; k++ )
         {
            rik = 
               sqrt(
                    ((*atoms)[i].pos[0] - (*atoms)[k].pos[0]) *
                    ((*atoms)[i].pos[0] - (*atoms)[k].pos[0]) +
                    ((*atoms)[i].pos[1] - (*atoms)[k].pos[1]) *
                    ((*atoms)[i].pos[1] - (*atoms)[k].pos[1]) +
                    ((*atoms)[i].pos[2] - (*atoms)[k].pos[2]) *
                    ((*atoms)[i].pos[2] - (*atoms)[k].pos[2])
                    );
#if defined(SAXS_DEBUG_F)
            cout << "dist atoms:  "
                 << i
                 << " "
                 << (*atoms)[i].saxs_name
                 << ","
                 << k
                 << " "
                 << (*atoms)[k].saxs_name
                 << " "
                 << rik
                 << endl;
#endif
            pos = (unsigned int)floor(rik / delta);
            if ( hist->size() <= pos )
            {
               hist->resize(pos + 128);
            }
            if ( b_bar_inv2 )
            {
               (*hist)[pos] += (*atoms)[i].b * (*atoms)[k].b * b_bar_inv2;
            } else {
               (*hist)[pos]++;
            }
         }
      }

#if defined(DEBUG_THREAD)
      cerr << "thread " << thread << " finished work\n";
#endif
      work_mutex.lock();
      work_done = 1;
      work_to_do = 0;
      work_mutex.unlock();
      cond_work_done.wakeOne();
   }
}

//--------- end thread for saxs p(r) plot -----------


void US_Hydrodyn_Saxs::update_bin_size(double val)
{
   our_saxs_options->bin_size = (float) val;
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Saxs::update_smooth(double val)
{
   our_saxs_options->smooth = ( unsigned int ) val;
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Saxs::update_guinier_cutoff(double val)
{
   guinier_cutoff = val;
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Saxs::update_guinier_lowq2(const QString &)
{
   set_guinier();
}

void US_Hydrodyn_Saxs::update_guinier_highq2(const QString &)
{
   set_guinier();
}

void US_Hydrodyn_Saxs::update_user_lowq(const QString &)
{
   set_guinier();
}

void US_Hydrodyn_Saxs::update_user_highq(const QString &)
{
   set_guinier();
}

void US_Hydrodyn_Saxs::update_user_lowI(const QString &)
{
   set_guinier();
}

void US_Hydrodyn_Saxs::update_user_highI(const QString &)
{
   set_guinier();
}

void US_Hydrodyn_Saxs::update_pr_contrib_low(const QString &str)
{
   pr_contrib_low = str.toDouble();
}

void US_Hydrodyn_Saxs::update_pr_contrib_high(const QString &str)
{
   pr_contrib_high = str.toDouble();
}

void US_Hydrodyn_Saxs::show_pr_contrib()
{
   // here we want to rasmol the structure with the contrib range colored
   if ( !contrib_pdb_atom.size() ||
        !contrib_array.size() ) 
   {
      editor_msg( "red", "Plot contributions: Nothing to plot\n" );
      return;
   }

   if ( pr_contrib_low >= pr_contrib_high ) 
   {
      editor_msg( "red", "Plot contributions: Range error\n");
      return;
   }
   progress_pr->reset();
   progress_pr->setMaximum(3);
   progress_pr->setValue(0);
   // sum up all the atoms that contrib:
   // cout << "trying to sum up\n";
   map < QString, double > contrib_sums;
   QRegExp rx2("^(\\d+):(\\d+)$");

   // compute prpos limits:
   int poslow = (unsigned int)floor(pr_contrib_low / contrib_delta);
   int poshi = (unsigned int)ceil(pr_contrib_high / contrib_delta);
   if ( poslow < 0 )
   {
      poslow = 0;
   }
   if ( (unsigned int) poshi >= contrib_array[0].size() )
   {
      poshi = contrib_array[0].size() - 1;
   }

   for ( unsigned int i = 0; i < contrib_array.size(); i++ )
   {
      for ( int j = poslow; j <= poshi ; j++ )
      {
         if ( contrib_array[i][j] > 0.0 )
         {
            QString id =
               QString("%1%2%3")
               .arg(contrib_pdb_atom[i]->resSeq)
               .arg(contrib_pdb_atom[i]->chainID != "" ? ":" : "")
               .arg(contrib_pdb_atom[i]->chainID);
            contrib_sums[id] += contrib_array[i][j];
         }
      }
   }
   progress_pr->setValue(1);

   // if all went ok, we should now have a weighted list in contrib_sums
   // normalize to max 1 (this will later set the color)
   double max = 0e0;
   for ( map < QString, double >::iterator it = contrib_sums.begin();
         it != contrib_sums.end();
         it++ )
   {
      if ( contrib_sums[it->first] > max )
      {
         max = contrib_sums[it->first];
      }
   }
   progress_pr->setValue(2);
   // divide all by max to put in range of [0,1]
   for ( map < QString, double >::iterator it = contrib_sums.begin();
         it != contrib_sums.end();
         it++ )
   {
      contrib_sums[it->first] /= max;
   }
   progress_pr->setValue(3);

   // setup plot:
#define CONTRIB_GRADIENT_SIZE 3

   QString gradient[] = 
      {
         "blue",
         //         "greenblue",
         "cyan",
         "yellow",
         "yellow",
      };

   QString error_msg;

   if ( !write_temp_pdb_selected_models( error_msg ) ) {
      editor_msg( "dark red", error_msg );
      return;
   }

   QString out = QString("load %1\nselect all\ncolor gray\n").arg( QFileInfo( last_selected_pdb_filename ).fileName() );
   for ( map < QString, double >::iterator it = contrib_sums.begin();
         it != contrib_sums.end();
         it++ )
   {
      out += 
         QString("select %1\ncolour %2\n")
         .arg(it->first)
         //         .arg((int)(128 + contrib_sums[it->first] * 127));
         .arg(gradient[(int)(contrib_sums[it->first] * CONTRIB_GRADIENT_SIZE)]);
   }
   //   cout << "\n" << out << endl;
   // put "out" into spt file:
   out += "select all\n";

   QDir::setCurrent( last_selected_pdb_filename );
   QString fname = last_selected_pdb_filename + ".spt";
   QFile f(fname);
   if ( !f.open( QIODevice::WriteOnly ) ) {
      editor_msg( "red", "Error creating file " + fname + "\n");
      return;
   }
   QTextStream t( &f );
   t << out;
   f.close();
   ((US_Hydrodyn *)us_hydrodyn)->model_viewer( fname, "-script" );
}

void US_Hydrodyn_Saxs::set_saxs_sans() {
   if ( rb_saxs->isChecked() ) {
      return set_saxs_sans( 0 );
   }
   if ( rb_sans->isChecked() ) {
      return set_saxs_sans( 1 );
   }
}

void US_Hydrodyn_Saxs::set_saxs_sans(int val)
{
   if ( our_saxs_options->saxs_sans != val )
   {
      clear_plot_saxs();
      our_saxs_options->saxs_sans = val;
      if ( !source )
      {
         rb_curve_raw->setChecked(false);
         rb_curve_saxs->setChecked(!val);
         rb_curve_sans->setChecked(val);
         set_curve(val + 1);
      }
      update_saxs_sans();
   }
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Saxs::set_curve() {
   if ( rb_curve_raw->isChecked() ) {
      return set_curve( 0 );
   }
   if ( rb_curve_saxs->isChecked() ) {
      return set_curve( 1 );
   }
   if ( rb_curve_sans->isChecked() ) {
      return set_curve( 2 );
   }
}

void US_Hydrodyn_Saxs::set_curve(int val)
{
   our_saxs_options->curve = val;
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Saxs::set_create_native_saxs()
{
   create_native_saxs = cb_create_native_saxs->isChecked();
}

void US_Hydrodyn_Saxs::set_pr_contrib()
{
}

void US_Hydrodyn_Saxs::show_plot_pr()
{
   if ( !selected_models.size() ) {
      return;
   }
   
   // QTextStream(stdout)
   //    << info_remember_mw( ": show_plot_pr() start" )
   //    ;

   progress_pr->set_cli_prefix( "pr" );
   pb_pr_contrib->setEnabled(false);
   stopFlag = false;
   pb_stop->setEnabled(true);
   pb_plot_pr->setEnabled(false);
   pb_plot_saxs_sans->setEnabled(false);
   progress_pr->reset();
   vector < double > hist;
   double delta = (double) our_saxs_options->bin_size;

#if defined(BUG_DEBUG)
   qApp->processEvents();
   cout << " sleep 1 a" << endl;
   sleep(1);
   cout << " sleep 1 a done" << endl;
#endif
   
   // for ( auto it = atom_map.begin();
   //       it != atom_map.end();
   //       ++it ) {
   //    QTextStream(stdout) << it->first << " atom name " << it->second.name << " hybrid name " << it->second.hybrid.name << "\n";
   // }

   if ( (double) our_saxs_options->water_e_density ) {
      atom_map[ "OW~O2H2" ].saxs_excl_vol = 10 / ( (double) our_saxs_options->water_e_density + (double) our_saxs_options->crysol_hydration_shell_contrast );
      editor_msg( "darkblue", QString( "Exclude volume of solvent atom: %1\n" ).arg( atom_map[ "OW~O2H2" ].saxs_excl_vol ) );
   } else {
      atom_map[ "OW~O2H2" ].saxs_excl_vol = 0;
   }

   QString use_name  = QFileInfo(model_filename).fileName();
   map < double, double > pr_exact;
#warning set pr_exact to true to resurrect pr->iq
   bool do_pr_exact = false;
   bool do_make_iq = false;

   for ( unsigned int i = 0; i < selected_models.size(); i++ )
   {
#warning pr_to_iq off except for bead model source
      current_model = selected_models[i];
#if defined(PR_DEBUG)
      printf("creating pr %u\n", current_model); fflush(stdout);
#endif
      editor_msg( "black", 
                  QString("\n\nPreparing file %1 model %2 for p(r) vs r plot in %3 mode%4.\n\n")
                  .arg(te_filename2->text())
                  .arg(current_model + 1)
                  .arg(rb_curve_raw->isChecked() ? "Raw" :
                       ( rb_curve_saxs->isChecked() ? "SAXS" : "SANS" ) )
                  .arg(cb_normalize->isChecked() ? ", Normalized" : "")
                  );
      qApp->processEvents();
      if ( stopFlag ) 
      {
         editor_msg( "dark red", us_tr("Terminated by user request.\n" ) );
         progress_pr->reset();
         lbl_core_progress->setText("");
         pb_plot_saxs_sans->setEnabled(bead_model_ok_for_saxs);
         // pb_plot_saxs_sans->setEnabled(true);
         pb_plot_pr->setEnabled(true);
         return;
      }
         
      vector < saxs_atom > atoms;
      saxs_atom new_atom;

      double b_bar = 0.0;
      double b_bar_inv2 = 0.0;
      int b_count = 0;

      contrib_pdb_atom.clear( );

      bool include_0_dist_pairs =
         !(
           ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "vdw_saxs_skip_pr0pair" ) &&
           ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "vdw_saxs_skip_pr0pair" ] == "true"
           )
         ;

      if ( !include_0_dist_pairs ) {
         editor_msg( "darkblue", us_tr( "experimental: zero distance pairs skipped\n" ) );
      }
      
      if ( source )
      {
         // bead models
         do_make_iq = true;

         bool saxs_water_beads =
            ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "vdw_saxs_water_beads" ) &&
            ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "vdw_saxs_water_beads" ] == "true"
            ;
         
         for (unsigned int j = 0; j < bead_models[current_model].size(); j++) {
            PDB_atom *this_atom = &(bead_models[current_model][j]);
            new_atom.pos[0]    = this_atom->bead_coordinate.axis[0];
            new_atom.pos[1]    = this_atom->bead_coordinate.axis[1];
            new_atom.pos[2]    = this_atom->bead_coordinate.axis[2];
            new_atom.b =
               this_atom->num_elect
               // - (double) our_saxs_options->water_e_density * (4/3) * M_PI * pow( this_atom->bead_computed_radius, 3.0 )
               - (double) our_saxs_options->water_e_density * this_atom->saxs_excl_vol
               ;
            b_count++;
            b_bar += new_atom.b;

            new_atom.atom_name     = this_atom->name;
            new_atom.hybrid_name   = this_atom->resName;
            new_atom.electrons     = this_atom->num_elect;
            // new_atom.excl_vol      = (4/3) * M_PI * pow( this_atom->bead_computed_radius, 3.0 );
            new_atom.excl_vol      = this_atom->saxs_excl_vol;

            atoms.push_back(new_atom);
            contrib_pdb_atom.push_back(this_atom);

            if ( this_atom->bead_hydration && saxs_water_beads ) {
               int addbeads = (int)(this_atom->bead_hydration + 0.5 );
               new_atom.atom_name   = "OW";
               new_atom.hybrid_name = "O2H2";

               if ( !hybrid_map.count( new_atom.hybrid_name ) ) {
                  QTextStream(stdout) << QString( "--> %1 missing from hybrid_map\n" ).arg( new_atom.hybrid_name );
               }
               new_atom.electrons     = hybrid_map[ new_atom.hybrid_name ].num_elect;
               new_atom.excl_vol      = atom_map[ "OW~O2H2" ].saxs_excl_vol;
               new_atom.b             =
                  new_atom.electrons
                  - (double) our_saxs_options->water_e_density * new_atom.excl_vol
                  ;
               
               QTextStream(stdout) <<
                  QString( "pr calc bead model : atom %1 %2 e %3 bead_hydration %4 rounded %5\n" )
                  .arg( this_atom->name )
                  .arg( this_atom->resName )
                  .arg( new_atom.electrons )
                  .arg( this_atom->bead_hydration )
                  .arg( addbeads )
                  ;

               for ( int i = 0; i < addbeads; ++i ) {
                  atoms.push_back( new_atom );
               }
            }
         }
      }
      else 
      {
         // compute b[0] based upon current values
         map < QString, double > b;
         if ( rb_curve_sans->isChecked() )
         {
            // for each entry in hybrid_map, compute b
            for (map < QString, hybridization >::iterator it = hybrid_map.begin();
                 it != hybrid_map.end();
                 it++)
            {
               // it->second.scat_len, .exch_prot
               b[it->first] = 
                  it->second.scat_len + 
                  it->second.exch_prot * 
                  our_saxs_options->d2o_conc * 
                  (our_saxs_options->d2o_scat_len_dens - our_saxs_options->h2o_scat_len_dens) * 1;
               // UPDATE to 1-fraction of exchanged peptide H for peptide bond groups NH only
#if defined(BUG_DEBUG)
               printf("hybrid name %s b %e\n",
                      it->first.toLatin1().data(),
                      b[it->first]);
#endif
               // special exchange handling for aa pb 
               if ( it->first == "N3H1" )
               {
                  b[it->first + "-aa"] = 
                     it->second.scat_len + 
                     
                     it->second.exch_prot * 
                     our_saxs_options->d2o_conc * 
                     (our_saxs_options->d2o_scat_len_dens - our_saxs_options->h2o_scat_len_dens) *
                     (1 - our_saxs_options->frac_of_exch_pep);
#if defined(BUG_DEBUG)
                  printf("hybrid name %s b %e\n",
                         QString(it->first + "-aa").toLatin1().data(),
                         b[it->first + "-aa"]);
#endif
               }
            }
         }
         if ( rb_curve_saxs->isChecked() 
              // || rb_curve_saxs_dry->isChecked() 
              )
         {
            // for each entry in hybrid_map, compute b
            // if ( !saxs_map.count("H") )
            // {
            // US_Static::us_message(us_tr("No Hydrogens:"), us_tr("No Hydrogen defined in SAXS Atom table"));
            // return;
            // }
            // float excl_volH = saxs_map["H"].volume;
            for (map < QString, hybridization >::iterator it = hybrid_map.begin();
                 it != hybrid_map.end();
                 it++)
            {
// #define DEBUG_PR_COEFF
#if defined(DEBUG_PR_COEFF)               
               QTextStream(stdout)  << " computing b for " << it->first 
                                    << " hydrogens " << it->second.hydrogens 
                                    << " exch_prot " << it->second.exch_prot
                                    << " num elect " << it->second.num_elect
                                    << " saxs_name " << it->second.saxs_name
                                    << " saxs_excl_vol noH " << saxs_map[it->second.saxs_name].volume
                  // << " saxs_excl_vol  " << ( saxs_map[it->second.saxs_name].volume + it->second.hydrogens * excl_volH )
                                    << endl
                  ;
#endif
               b[it->first] = 
                  it->second.num_elect;
            }
         }            
         // pdb files
         for (unsigned int j = 0; j < model_vector[current_model].molecule.size(); j++)
         {
            for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++)
            {
               PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);
               
               new_atom.pos[0] = this_atom->coordinate.axis[0];
               new_atom.pos[1] = this_atom->coordinate.axis[1];
               new_atom.pos[2] = this_atom->coordinate.axis[2];
               
               if ( rb_curve_sans->isChecked() )
               {
                  QString mapkey = QString("%1|%2").arg(this_atom->resName).arg(this_atom->name);
                  double solvent_b = 
                     our_saxs_options->h2o_scat_len_dens * ( 1e0 - our_saxs_options->d2o_conc ) +
                     our_saxs_options->d2o_scat_len_dens * our_saxs_options->d2o_conc
                     ;
                     
                  if ( this_atom->name == "OXT" )
                  {
                     mapkey = "OXT|OXT";
                  }
                  QString hybrid_name = residue_atom_hybrid_map[mapkey];
                  QString this_atom_name = hybrid_name == "ABB" ? "ABB" : this_atom->name;
                  if ( !atom_map.count( this_atom_name + "~" + hybrid_name ) )
                  {
                     QTextStream( stdout ) << QString( "atom_map missing %1\n" ).arg( this_atom_name + "~" + hybrid_name );
                     continue;
                     // if ( !((US_Hydrodyn *)us_hydrodyn)->gui_script ) {
                     //    US_Static::us_message( us_tr("Missing Atom:"), 
                     //                           QString( us_tr("Atom %1 not defined") ).arg( this_atom_name ) );
                     // }
                     // progress_pr->reset();
                     // lbl_core_progress->setText("");
                     // pb_plot_saxs_sans->setEnabled(bead_model_ok_for_saxs);
                     // // pb_plot_saxs_sans->setEnabled(true);
                     // pb_plot_pr->setEnabled(true);
                     // return;
                  }
                  new_atom.b = b[hybrid_name] - solvent_b * atom_map[this_atom_name + "~" + hybrid_name].saxs_excl_vol;
                  b_count++;
                  b_bar += new_atom.b;
#if defined(BUG_DEBUG) || defined(DEBUG_PR_COEFF)
                  printf("atom %d %d hybrid name %s, atom name %s b %e mapkey %s hybrid name %s\n",
                         j, k, 
                         hybrid_name.toLatin1().data(),
                         this_atom_name.toLatin1().data(),
                         new_atom.b,
                         mapkey.toLatin1().data(),
                         hybrid_name.toLatin1().data()
                         );
#endif
               }

               if ( rb_curve_saxs->isChecked() 
                    // || rb_curve_saxs_dry->isChecked() 
                    )
               {
                  QString mapkey = QString("%1|%2").arg(this_atom->resName).arg(this_atom->name);
                  if ( this_atom->name == "OXT" )
                  {
                     mapkey = "OXT|OXT";
                  }
#if defined(DEBUG_PR_COEFF)
                  QTextStream(stdout)
                     << "mapkey " << mapkey
                     << Qt::endl
                     ;
#endif

                  QString hybrid_name = residue_atom_hybrid_map[mapkey];
                  // double radius = 0e0;
                  if ( rb_curve_saxs->isChecked() )
                  {
                     if ( !hybrid_map.count(hybrid_name) )
                     {
                        cout << "error: hybrid_map name missing for hybrid_name " << hybrid_name << endl;
                        editor_msg( "red", 
                                    QString("%1Molecule %2 Residue %3 %4 Hybrid %5 name missing from Hybrid file. Assuming zero radius!.\n")
                                    .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                                    .arg(j+1)
                                    .arg(this_atom->resName)
                                    .arg(this_atom->resSeq)
                                    .arg(hybrid_name)
                                    );
                        qApp->processEvents();
                     } else {
                        // radius = hybrid_map[hybrid_name].radius;
                     }
                  }
                  QString this_atom_name = hybrid_name == "ABB" ? "ABB" : this_atom->name;
                  if ( !atom_map.count( this_atom_name + "~" + hybrid_name ) )
                  {
                     QTextStream( stdout ) << QString( "atom_map missing %1\n" ).arg( this_atom_name + "~" + hybrid_name );
                     continue;
                     // if ( !((US_Hydrodyn *)us_hydrodyn)->gui_script ) {
                     //    US_Static::us_message( us_tr("Missing Atom:"), 
                     //                           QString( us_tr("Atom %1 not defined") ).arg( this_atom_name ) );
                     // }
                     // progress_pr->reset();
                     // lbl_core_progress->setText("");
                     // pb_plot_saxs_sans->setEnabled(bead_model_ok_for_saxs);
                     // // pb_plot_saxs_sans->setEnabled(true);
                     // pb_plot_pr->setEnabled(true);
                     // return;
                  }
                  // cout << "atom " << this_atom->name 
                  // << " hybrid " << this_atom->hybrid_name
                  // << " excl_vol " <<  atom_map[this_atom_name + "~" + hybrid_name].saxs_excl_vol 
                  // << " radius cubed " << radius * radius * radius 
                  // << " pi radius cubed " << M_PI * radius * radius * radius 
                  // << endl;
#if defined(DEBUG_PR_COEFF)
                  QTextStream(stdout)
                     << "atom " << this_atom_name
                     << " hybrid " << hybrid_name
                     << " electrons " << hybrid_map[hybrid_name].num_elect
                     << Qt::endl
                     ;
#endif
                  new_atom.b = b[hybrid_name] - (double) our_saxs_options->water_e_density * atom_map[this_atom_name + "~" + hybrid_name].saxs_excl_vol;
                  b_count++;
                  b_bar += new_atom.b;
#if defined(BUG_DEBUG)
                  printf("atom %d %d hybrid name %s, atom name %s b %e correction %e mapkey %s hybrid name %s\n",
                         j, k, 
                         hybrid_name.toLatin1().data(),
                         this_atom_name.toLatin1().data(),
                         new_atom.b,
                         our_saxs_options->water_e_density * radius * radius * radius,
                         mapkey.toLatin1().data(),
                         hybrid_name.toLatin1().data()
                         );
#endif
                  new_atom.atom_name = this_atom_name;
                  new_atom.hybrid_name = hybrid_name;
                  new_atom.electrons     = b[hybrid_name];
                  new_atom.excl_vol      = atom_map[this_atom_name + "~" + hybrid_name].saxs_excl_vol;
               }
                  
               atoms.push_back(new_atom);
               contrib_pdb_atom.push_back(this_atom);
            }
         }
      }
      if ( !rb_curve_raw->isChecked() )
      {
         b_bar /= b_count;
         if ( b_bar )
         {
            b_bar_inv2 = 1.0 / (b_bar * b_bar);
         } else {
            b_bar_inv2 = 0.0;
            rb_curve_raw->setChecked(true);
            rb_curve_saxs->setChecked(false);
            rb_curve_sans->setChecked(false);
            editor_msg( "red", us_tr("WARNING: < b > is zero! Reverting to RAW mode for p(r) vs r computation.") );
         }
      }

#if defined(BUG_DEBUG)
      qApp->processEvents();
      cout << "atoms size " << atoms.size() << endl;
      cout << " sleep 1 b" << endl;
      sleep(1);
      cout << " sleep 1 b done" << endl;
#endif
      // ok now we have all the atoms
      if ( 0 ) {
         for ( auto it = atoms.begin();
               it != atoms.end();
               ++it ) {
            //    // QTextStream( stdout ) << QString( "saxs_name %1 hybrid_name %2 residue_name %3 atom_name %4 b %5\n" )
            //    //    .arg( it->saxs_name )
            //    //    .arg( it->hybrid_name )
            //    //    .arg( it->residue_name )
            //    //    .arg( it->atom_name )
            //    //    .arg( it->b )
            //    //    ;
            QTextStream( stdout ) << QString( "%1 %2 [%3,%4,%5] b %6 elect %7 exclvol %8\n" )
               .arg( it->atom_name )
               .arg( it->hybrid_name )
               .arg( it->pos[0] )
               .arg( it->pos[1] )
               .arg( it->pos[2] )
               .arg( it->b )
               .arg( it->electrons )
               .arg( it->excl_vol )
               ;
         }
      }

      editor_msg( "black", 
                  QString("Number of atoms %1. Bin size %2.\n")
                  .arg(atoms.size())
                  .arg(delta));
      qApp->processEvents();

      // we want to keep a tally of each atom's contribution to each position
      // contrib.clear( );
      contrib_array.clear( );
      contrib_delta = delta;
      // contrib_file = te_filename2->text();
      contrib_file = ((US_Hydrodyn *)us_hydrodyn)->pdb_file;
      cout << "contrib_file " << contrib_file << endl;

      bool spec_wat_check        = false;
      int ow_wat_count           = 0;
      int ow_wat_count_remaining = 0;
      double ow_cutoff           = 0;
      int ow_wats_kept           = 0;

      // #warning REMOVE BEFORE DIST (WAT controls)
      // controls are hidden outside of expert mode? so ok
      {
         for ( unsigned int i = 0; i < atoms.size(); ++i ) {
            if ( atoms[i].atom_name == "OW" ) {
               ++ow_wat_count;
            }
         }

         if ( ow_wat_count > 1 ) {
            // by count
            {
               bool ok;
               int ow_user_count =
                  QInputDialog::getInt(this
                                       ,windowTitle() + " : WAT cutoff"
                                       ,tr("Closest number of WATs to keep (0 or CANCEL to keep all) : ")
                                       ,ow_wat_count
                                       ,1
                                       ,ow_wat_count
                                       ,1
                                       ,&ok );
               if (ok && ow_user_count < ow_wat_count) {
                  // remove atoms with cutoff > ow_cutoff2
                  map < int, double > min_dist2_to_prot;
                  
                  // step 1 find minimum distance
                  editor_msg( "darkblue", "Finding distance of WATs to non-WATs\n" );
                  qApp->processEvents();

                  progress_pr->setMaximum(atoms.size());
                  
                  for ( unsigned int i = 0; i < atoms.size(); ++i ) {
                     progress_pr->setValue(i);
                     qApp->processEvents();
                     if ( atoms[i].atom_name == "OW" ) {
                        for ( unsigned int j = 0; j < atoms.size(); ++j ) {
                           if ( i != j && atoms[j].atom_name != "OW" ) {
                              double rik2 = 
                                 (atoms[i].pos[0] - atoms[j].pos[0]) *
                                 (atoms[i].pos[0] - atoms[j].pos[0]) +
                                 (atoms[i].pos[1] - atoms[j].pos[1]) *
                                 (atoms[i].pos[1] - atoms[j].pos[1]) +
                                 (atoms[i].pos[2] - atoms[j].pos[2]) *
                                 (atoms[i].pos[2] - atoms[j].pos[2])
                                 ;
                              if ( !min_dist2_to_prot.count( i )
                                   || min_dist2_to_prot[ i ] > rik2
                                   ) {
                                 min_dist2_to_prot[ i ] = rik2;
                              }
                           }
                        }
                     }
                  }
                                 
                  // order by distance
                  map < double, set < int > > min_dist2_to_atoms;
                  for ( auto it = min_dist2_to_prot.begin();
                        it != min_dist2_to_prot.end();
                        ++it ) {
                     min_dist2_to_atoms[ it->second ].insert( it->first );
                  }

                  // keep ow_user_count
                  set < int > min_dist2_to_prot_keep;
                  int elements_so_far = 0;
                  for ( auto it = min_dist2_to_atoms.begin();
                        it != min_dist2_to_atoms.end();
                        ++it ) {
                     min_dist2_to_prot_keep.insert( it->second.begin(), it->second.end() );
                     elements_so_far += (int)it->second.size();
                     if ( elements_so_far >= ow_user_count ) {
                        break;
                     }
                  }
                  ow_wats_kept = elements_so_far;

                  // recreate atoms
                  
                  vector < saxs_atom > new_atoms;
                  for ( unsigned int i = 0; i < atoms.size(); ++i ) {
                     if ( !min_dist2_to_prot.count( i )
                          || min_dist2_to_prot_keep.count( i ) ) {
                        new_atoms.push_back( atoms[i] );
                        if ( atoms[i].atom_name == "OW" ) {
                           ++ow_wat_count_remaining;
                        }
                     }
                  }

                  editor_msg( "darkblue", QString( "removed %1 of %2 WATs\n" ).arg( atoms.size() - new_atoms.size() ).arg( ow_wat_count ) );
                  progress_pr->reset();
                  
                  if ( new_atoms.size() != atoms.size() ) {
                     switch( QMessageBox::question(this
                                                   ,windowTitle() + " : WAT treatment" 
                                                   ,QString( us_tr("Write the modified PDB?") )
                                                   ) ) {
                     case QMessageBox::Yes :
                        {
                           // build up coordinate map of removed WATs
                           set < point > keep_WAT;
                           for ( int i = 0; i < (int) new_atoms.size(); ++i ) {
                              if ( new_atoms[ i ].atom_name == "OW" ) {
                                 point p;
                                 p.axis[0] = new_atoms[ i ].pos[0];
                                 p.axis[1] = new_atoms[ i ].pos[1];
                                 p.axis[2] = new_atoms[ i ].pos[2];
                                 
                                 keep_WAT.insert( p );
                              }
                           }
                           
                           PDB_model tmp_model = model_vector[current_model];
                           for ( unsigned int j = 0; j < model_vector[current_model].molecule.size(); ++j ) {
                              tmp_model.molecule[j].atom.clear();
                              for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++) {
                                 PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);
                                 if ( this_atom->name != "OW" ) {
                                    tmp_model.molecule[j].atom.push_back( *this_atom );
                                 } else {
                                    if ( keep_WAT.count( this_atom->coordinate ) ) {
                                       tmp_model.molecule[j].atom.push_back( *this_atom );
                                    }
                                 }
                              }
                           }
                           {
                              QString errors;
                              QString writtenname;
                              if ( 
                                  ((US_Hydrodyn *)us_hydrodyn)->write_pdb_from_model( tmp_model
                                                                                      ,errors
                                                                                      ,writtenname
                                                                                      ,QString( " WATs further than %1 [A] removed" ).arg( ow_cutoff )
                                                                                      ,QString( "_wc%1" ).arg( ow_wats_kept )
                                                                                      ,model_filepathname
                                                                                      ) ) {
                                 editor_msg( "black", QString( us_tr( "File %1 created\n" ) ).arg( writtenname ) );
                              } else {
                                 editor_msg( "red", QString( us_tr( "Error writing File %1. %2\n" ) ).arg( writtenname ).arg( errors ) );
                              }
                           }
                        }
                        break;
                     default:
                        break;
                     }
                     atoms = new_atoms;
                  } else {
                     ow_cutoff = 0;
                  }
               }
            }
            {
               double ow_cutoff2 = 0;
               if ( !ow_wats_kept ) {
                  bool ok;
                  ow_cutoff = QInputDialog::getDouble(this
                                                      ,windowTitle() + " : WAT cutoff"
                                                      ,tr("Cutoff WAT to non-WAT distance [A] (0 or CANCEL for no cutoff) : ")
                                                      ,10
                                                      ,0
                                                      ,100
                                                      ,3
                                                      ,&ok
                                                      ,Qt::WindowFlags(), 1);
                  if (ok) {
                     ow_cutoff2 = ow_cutoff * ow_cutoff;
                  }

                  if ( ow_cutoff2 ) {
                     // remove atoms with cutoff > ow_cutoff2
                     map < int, double > min_dist2_to_prot;
                  
                     // step 1 find minimum distance
                     editor_msg( "darkblue", "Finding distance of WATs to non-WATs\n" );
                     qApp->processEvents();

                     progress_pr->setMaximum(atoms.size());

                     for ( unsigned int i = 0; i < atoms.size(); ++i ) {
                        progress_pr->setValue(i);
                        qApp->processEvents();
                        if ( atoms[i].atom_name == "OW" ) {
                           for ( unsigned int j = 0; j < atoms.size(); ++j ) {
                              if ( i != j && atoms[j].atom_name != "OW" ) {
                                 double rik2 = 
                                    (atoms[i].pos[0] - atoms[j].pos[0]) *
                                    (atoms[i].pos[0] - atoms[j].pos[0]) +
                                    (atoms[i].pos[1] - atoms[j].pos[1]) *
                                    (atoms[i].pos[1] - atoms[j].pos[1]) +
                                    (atoms[i].pos[2] - atoms[j].pos[2]) *
                                    (atoms[i].pos[2] - atoms[j].pos[2])
                                    ;
                                 if ( !min_dist2_to_prot.count( i )
                                      || min_dist2_to_prot[ i ] > rik2
                                      ) {
                                    min_dist2_to_prot[ i ] = rik2;
                                 }
                              }
                           }
                        }
                     }
                                 
                     vector < saxs_atom > new_atoms;
                     for ( unsigned int i = 0; i < atoms.size(); ++i ) {
                        if ( !min_dist2_to_prot.count( i )
                             || min_dist2_to_prot[ i ] <= ow_cutoff2 ) {
                           new_atoms.push_back( atoms[i] );
                           if ( atoms[i].atom_name == "OW" ) {
                              ++ow_wat_count_remaining;
                           }
                        }
                     }

                     editor_msg( "darkblue", QString( "removed %1 of %2 WATs\n" ).arg( atoms.size() - new_atoms.size() ).arg( ow_wat_count ) );
                     progress_pr->reset();
                     if ( new_atoms.size() != atoms.size() ) {
                        switch( QMessageBox::question(this
                                                      ,windowTitle() + " : WAT treatment" 
                                                      ,QString( us_tr("Write the modified PDB?") )
                                                      ) ) {
                        case QMessageBox::Yes :
                           {
                              // build up coordinate map of removed WATs
                              set < point > keep_WAT;
                              for ( int i = 0; i < (int) new_atoms.size(); ++i ) {
                                 if ( new_atoms[ i ].atom_name == "OW" ) {
                                    point p;
                                    p.axis[0] = new_atoms[ i ].pos[0];
                                    p.axis[1] = new_atoms[ i ].pos[1];
                                    p.axis[2] = new_atoms[ i ].pos[2];
                                 
                                    keep_WAT.insert( p );
                                 }
                              }
                           
                              PDB_model tmp_model = model_vector[current_model];
                              for ( unsigned int j = 0; j < model_vector[current_model].molecule.size(); ++j ) {
                                 tmp_model.molecule[j].atom.clear();
                                 for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++) {
                                    PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);
                                    if ( this_atom->name != "OW" ) {
                                       tmp_model.molecule[j].atom.push_back( *this_atom );
                                    } else {
                                       if ( keep_WAT.count( this_atom->coordinate ) ) {
                                          tmp_model.molecule[j].atom.push_back( *this_atom );
                                       }
                                    }
                                 }
                              }
                              {
                                 QString errors;
                                 QString writtenname;
                                 if ( 
                                     ((US_Hydrodyn *)us_hydrodyn)->write_pdb_from_model( tmp_model
                                                                                         ,errors
                                                                                         ,writtenname
                                                                                         ,QString( " WATs further than %1 [A] removed" ).arg( ow_cutoff )
                                                                                         ,QString( "_co%1" ).arg( ow_cutoff ).replace(".","_")
                                                                                         ,model_filepathname
                                                                                         ) ) {
                                    editor_msg( "black", QString( us_tr( "File %1 created\n" ) ).arg( writtenname ) );
                                 } else {
                                    editor_msg( "red", QString( us_tr( "Error writing File %1. %2\n" ) ).arg( writtenname ).arg( errors ) );
                                 }
                              }
                           }
                           break;
                        default:
                           break;
                        }
                        atoms = new_atoms;
                     } else {
                        ow_cutoff = 0;
                     }
                  }
               }
            }

            switch( QMessageBox::question(this
                                          ,windowTitle() + " : WAT treatment" 
                                          ,QString( us_tr("Exclude WAT WAT from P(r)?") )
                                    ) ) {
            case QMessageBox::Yes :
               spec_wat_check = true;
               break;
            default:
               break;
            }
         }
      }

      // restore threading later
      if ( 0 && USglobal->config_list.numThreads > 1 )
      {
#if defined( USE_THREAD )
         // threaded
         
         unsigned int j;
         unsigned int threads = USglobal->config_list.numThreads;
         editor_msg( "blue", QString("Using %1 threads.\n").arg(threads));
         vector < saxs_pr_thr_t* > saxs_pr_thr_threads(threads);
         for ( j = 0; j < threads; j++ )
         {
            saxs_pr_thr_threads[j] = new saxs_pr_thr_t(j);
            saxs_pr_thr_threads[j]->start();
         }
         vector < vector < double > > hists;
         hists.resize(threads);
         for ( j = 0; j < threads; j++ )
         {
# if defined(DEBUG_THREAD)
            cout << "thread " << j << endl;
# endif            
            saxs_pr_thr_threads[j]->saxs_pr_thr_setup(
                                                      &atoms,
                                                      &((float)hists)[j],
                                                      delta,
                                                      threads,
                                                      progress_pr,
                                                      lbl_core_progress,
                                                      &stopFlag,
                                                      b_bar_inv2
                                                      );
         }
         // sleep app loop
         {
            int all_done;
            do {
               all_done = threads;
               for ( j = 0; j < threads; j++ )
               {
                  all_done -= saxs_pr_thr_threads[j]->saxs_pr_thr_work_status();
               }
               qApp->processEvents();
               mQThread::msleep( 333 );
            } while(all_done);
         }
         
         // wait for work to complete

         for ( j = 0; j < threads; j++ )
         {
            saxs_pr_thr_threads[j]->saxs_pr_thr_wait();
         }

         // destroy
         
         for ( j = 0; j < threads; j++ )
         {
            saxs_pr_thr_threads[j]->saxs_pr_thr_shutdown();
         }
         
         for ( j = 0; j < threads; j++ )
         {
            saxs_pr_thr_threads[j]->wait();
         }
         
         for ( j = 0; j < threads; j++ )
         {
            delete saxs_pr_thr_threads[j];
         }
         
         // merge results
         for ( j = 0; j < threads; j++ )
         {
            if (hist.size() < hists[j].size() )
            {
               hist.resize(hists[j].size());
            }
            for ( unsigned int k = 0; k < hists[j].size(); k++ )
            {
               hist[k] += hists[j][k];
            }
         }
#endif

      } // end threaded
      else
      {
#if defined(BUG_DEBUG)
         cout << "non-threaded run\n";
#endif
         // non-threaded
         double rik; 
         unsigned int pos;
         progress_pr->setMaximum((int)(atoms.size()));
         QTextStream(stdout) << QString( "ready to compute pr, atoms.size() %1\n" ).arg( atoms.size() );
         if ( cb_pr_contrib->isChecked() &&
              !source &&
              contrib_file.contains(QRegExp("(PDB|pdb)$")) )
         {
            // contrib version
            QTextStream(stdout) << "running contrib pr\n";
            
            contrib_array.resize(atoms.size());
            for ( unsigned int i = 0; i < atoms.size() - 1; i++ )
            {
               progress_pr->setValue(i+1);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(us_tr("Terminated by user request.\n"));
                  progress_pr->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(bead_model_ok_for_saxs);
                  // pb_plot_saxs_sans->setEnabled(false);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
               if ( spec_wat_check ) {
                  for ( unsigned int j = i + 1; j < atoms.size(); j++ )
                  {
                     if ( atoms[i].atom_name != "OW" || atoms[j].atom_name != "OW" ) {
                        rik = 
                           sqrt(
                                (atoms[i].pos[0] - atoms[j].pos[0]) *
                                (atoms[i].pos[0] - atoms[j].pos[0]) +
                                (atoms[i].pos[1] - atoms[j].pos[1]) *
                                (atoms[i].pos[1] - atoms[j].pos[1]) +
                                (atoms[i].pos[2] - atoms[j].pos[2]) *
                                (atoms[i].pos[2] - atoms[j].pos[2])
                                );
                        if ( include_0_dist_pairs || rik ) {
                           pos = (unsigned int)floor(rik / delta);
                           if ( hist.size() <= pos )
                           {
                              hist.resize(pos + 1024);
                              // if ( cb_guinier->isChecked() )
                              // {
                              for ( unsigned int k = 0; k < atoms.size(); k++ )
                              {
                                 contrib_array[k].resize(pos + 1024);
                              }
                              // }
                           }
                           if ( rb_curve_raw->isChecked() )
                           {
                              hist[pos]++;
                              // if ( cb_guinier->isChecked() )
                              // {
                              contrib_array[i][pos]++;
                              contrib_array[j][pos]++;
                              // } else {
                              //   contrib[QString("%1:%2").arg(i).arg(pos)]++;
                              //  contrib[QString("%1:%2").arg(j).arg(pos)]++;
                              // }
                           } else {
                              // good for both saxs & sans
                              double this_pr = (double) atoms[i].b * atoms[j].b * b_bar_inv2;
                              hist[pos] += this_pr;
                              // if ( cb_guinier->isChecked() )
                              // {
                              contrib_array[i][pos] += this_pr;
                              contrib_array[j][pos] += this_pr;
                              // } else {
                              //   contrib[QString("%1:%2").arg(i).arg(pos)] += this_pr;
                              //  contrib[QString("%1:%2").arg(j).arg(pos)] += this_pr;
                              // }
                           }
                        }
                     }
                  }
               } else {
                  for ( unsigned int j = i + 1; j < atoms.size(); j++ )
                  {
                     rik = 
                        sqrt(
                             (atoms[i].pos[0] - atoms[j].pos[0]) *
                             (atoms[i].pos[0] - atoms[j].pos[0]) +
                             (atoms[i].pos[1] - atoms[j].pos[1]) *
                             (atoms[i].pos[1] - atoms[j].pos[1]) +
                             (atoms[i].pos[2] - atoms[j].pos[2]) *
                             (atoms[i].pos[2] - atoms[j].pos[2])
                             );
                     if ( include_0_dist_pairs || rik ) {
                        pos = (unsigned int)floor(rik / delta);
                        if ( hist.size() <= pos )
                        {
                           hist.resize(pos + 1024);
                           // if ( cb_guinier->isChecked() )
                           // {
                           for ( unsigned int k = 0; k < atoms.size(); k++ )
                           {
                              contrib_array[k].resize(pos + 1024);
                           }
                           // }
                        }
                        if ( rb_curve_raw->isChecked() )
                        {
                           hist[pos]++;
                           // if ( cb_guinier->isChecked() )
                           // {
                           contrib_array[i][pos]++;
                           contrib_array[j][pos]++;
                           // } else {
                           //   contrib[QString("%1:%2").arg(i).arg(pos)]++;
                           //  contrib[QString("%1:%2").arg(j).arg(pos)]++;
                           // }
                        } else {
                           // good for both saxs & sans
                           double this_pr = (double) atoms[i].b * atoms[j].b * b_bar_inv2;
                           hist[pos] += this_pr;
                           // if ( cb_guinier->isChecked() )
                           // {
                           contrib_array[i][pos] += this_pr;
                           contrib_array[j][pos] += this_pr;
                           // } else {
                           //   contrib[QString("%1:%2").arg(i).arg(pos)] += this_pr;
                           //  contrib[QString("%1:%2").arg(j).arg(pos)] += this_pr;
                           // }
                        }
                     }
                  }
               }
            }
            pb_pr_contrib->setEnabled(true);
         } else {
            // non contrib version:
            printf( "atoms.size() %d\n", (int) atoms.size() );
            QTextStream(stdout) << QString( "running not contrib pr, b_bar_inv2 %1\n" ).arg( b_bar_inv2 );
            int total_terms = 0;
            for ( unsigned int i = 0; i < atoms.size() - 1; i++ )
            {
               progress_pr->setValue(i+1);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(us_tr("Terminated by user request.\n"));
                  progress_pr->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(bead_model_ok_for_saxs);
                  // pb_plot_saxs_sans->setEnabled(false);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
               if ( spec_wat_check ) {
                  for ( unsigned int j = i + 1; j < atoms.size(); j++ )
                  {
                     if ( atoms[i].atom_name != "OW" || atoms[j].atom_name != "OW" ) {
                        rik = 
                           sqrt(
                                (atoms[i].pos[0] - atoms[j].pos[0]) *
                                (atoms[i].pos[0] - atoms[j].pos[0]) +
                                (atoms[i].pos[1] - atoms[j].pos[1]) *
                                (atoms[i].pos[1] - atoms[j].pos[1]) +
                                (atoms[i].pos[2] - atoms[j].pos[2]) *
                                (atoms[i].pos[2] - atoms[j].pos[2])
                                );
                        if ( include_0_dist_pairs || rik ) {
                           ++total_terms;

                           pos = (unsigned int)floor(rik / delta);
                           if ( hist.size() <= pos )
                           {
                              hist.resize(pos + 128);
                           }
                           if ( rb_curve_raw->isChecked() )
                           {
                              hist[pos]++;
                              if ( do_pr_exact ) {
                                 if ( pr_exact.count( rik ) ) {
                                    pr_exact[ rik ]++;
                                 } else {
                                    pr_exact[ rik ] = 1;
                                 }
                              }
                           } else {
                              // good for both saxs & sans
                              hist[pos] += (double) atoms[i].b * atoms[j].b * b_bar_inv2;
                              if ( do_pr_exact ) {
                                 if ( pr_exact.count( rik ) ) {
                                    pr_exact[ rik ] += (double) atoms[i].b * atoms[j].b * b_bar_inv2;
                                 } else {
                                    pr_exact[ rik ] = (double) atoms[i].b * atoms[j].b * b_bar_inv2;
                                 }
                              }
                           }
                        }
                     }
                  }
               } else {
                  for ( unsigned int j = i + 1; j < atoms.size(); j++ )
                  {
                     rik = 
                        sqrt(
                             (atoms[i].pos[0] - atoms[j].pos[0]) *
                             (atoms[i].pos[0] - atoms[j].pos[0]) +
                             (atoms[i].pos[1] - atoms[j].pos[1]) *
                             (atoms[i].pos[1] - atoms[j].pos[1]) +
                             (atoms[i].pos[2] - atoms[j].pos[2]) *
                             (atoms[i].pos[2] - atoms[j].pos[2])
                             );
                     if ( include_0_dist_pairs || rik ) {
                        ++total_terms;
                        pos = (unsigned int)floor(rik / delta);
                        if ( hist.size() <= pos )
                        {
                           hist.resize(pos + 128);
                        }
                        if ( rb_curve_raw->isChecked() )
                        {
                           hist[pos]++;
                           if ( do_pr_exact ) {
                              if ( pr_exact.count( rik ) ) {
                                 pr_exact[ rik ]++;
                              } else {
                                 pr_exact[ rik ] = 1;
                              }
                           }
                        } else {
                           // good for both saxs & sans
                           hist[pos] += (double) atoms[i].b * atoms[j].b * b_bar_inv2;
                           if ( do_pr_exact ) {
                              if ( pr_exact.count( rik ) ) {
                                 pr_exact[ rik ] += (double) atoms[i].b * atoms[j].b * b_bar_inv2;
                              } else {
                                 pr_exact[ rik ] = (double) atoms[i].b * atoms[j].b * b_bar_inv2;
                              }
                           }
                        }
                     }
                  }
               }
            }
            // #warning REMOVE BEFORE DIST
            // editor_msg( "darkblue",
            //             QString(
            //                     "Computed cross-terms %1\n"
            //                     "total scatter count  %2\n"
            //                     "total non-OW  count  %3\n"
            //                     "OW count             %4\n"
            //                     )
            //             .arg( total_terms )
            //             .arg( atoms.size() )
            //             .arg( (int) atoms.size() - ow_wat_count_remaining )
            //             .arg( ow_wat_count_remaining )
            //             );
         }
      } // end non-threaded
         
#if defined(BUG_DEBUG)
      cout << "contrib info:\n";
      for ( map < QString, double >::iterator it = contrib.begin();
            it != contrib.end();
            it++ )
      {
         cout << "." << flush;
      }
      cout << endl;
#endif

#if defined(BUG_DEBUG)
      qApp->processEvents();
      cout << " sleep 1 bb" << endl;
      sleep(1);
      cout << " sleep 1 bb done" << endl;
#endif
      // trim hist
#if defined(PR_DEBUG)
      cout << "hist.size() " << hist.size() << endl;
#endif
      // while( hist.size() && !hist[hist.size()-1] ) 
      while( hist.size() && hist[hist.size()-1] <= 0 ) 
      {
         hist.pop_back();
      }
      // set remaining negatives to zero
      for ( int i = 0; i < (int) hist.size(); ++i ) {
         if ( hist[i] < 0 ) {
            hist[i] = 0;
         }
      }
      if ( contrib_array.size() ) 
      {
         for ( unsigned int k = 0; k < contrib_array.size(); k++ )
         {
            contrib_array[k].resize(hist.size());
         }
      }

#if defined(PR_DEBUG)
      cout << "hist.size() after " << hist.size() << endl;
#endif
      if ( our_saxs_options->smooth )
      {
         US_Saxs_Util usu;
         vector < double > x  ( hist.size() );
         vector < double > spr;

         for ( unsigned int k = 0; k < hist.size(); k++ )
         {
            x[ k ] = hist[ k ];
         }

         if ( usu.smooth( x, spr, our_saxs_options->smooth ) )
         {
            for ( unsigned int k = 0; k < hist.size(); k++ )
            {
               hist[ k ] = spr[ k ];
            }
         } else {
            editor_msg( "red", "smoothing error: " + usu.errormsg );
         }
      }

      // save the data to a file
      if ( create_native_saxs )
      {
         QString append = "";
         if ( ow_wat_count ) {
            if ( ow_cutoff ) {
               append += QString( "_co%1" ).arg( ow_cutoff ).replace(".","_");
            }
            if ( ow_wats_kept ) {
               append += QString( "_wc%1" ).arg( ow_wats_kept );
            }
            append +=
               QString("_hs%1").arg( QString("%1").arg( our_saxs_options->crysol_hydration_shell_contrast ).replace(".", "_" ) );
            if ( spec_wat_check ) {
               append += "_noWW";
            }
         }

         QString fpr_name = 
            USglobal->config_list.root_dir + 
            SLASH + "somo" + SLASH + "saxs" + SLASH + sprr_filestring( append );
         
         bool ok_to_write = true;
         if ( QFile::exists(fpr_name) &&
              !((US_Hydrodyn *)us_hydrodyn)->overwrite ) 
         {
            fpr_name = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fpr_name, 0, this);
            ok_to_write = true;
#if defined(OLD_WAY)

            switch( QMessageBox::information( this, 
                                              us_tr("Overwrite file:") + "SAXS P(r) vs. r" + us_tr("output file"),
                                              us_tr("The P(r) curve file \"") + 
                                              sprr_filestring() +
                                              us_tr("\" will be overwriten"),
                                              "&Ok",  "&Cancel", 0,
                                              0,      // Enter == button 0
                                              1 ) ) { // Escape == button 2
            case 0: // just go ahead
               ok_to_write = true;
               break;
            case 1: // Cancel clicked or Escape pressed
               ok_to_write = false;
               break;
            }
#endif
         }

         // QTextStream(stdout)
         //    << info_remember_mw( ": show_plot_pr() before ok_to_write" )
         //    ;
         
         if ( ok_to_write )
         {
            use_name = QFileInfo(fpr_name).baseName();
            FILE *fpr = us_fopen(fpr_name, "w");
            if ( fpr ) 
            {
               editor->append(us_tr("P(r) curve file: ") + fpr_name + us_tr(" created.\n"));
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_mw = get_mw(te_filename2->text(), false);
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear( );
               vector < double > r;
               vector < double > pr;
               vector < double > pr_n;
               vector < double > pr_error;
               r.resize(hist.size());
               pr.resize(hist.size());
               pr_n.resize(hist.size());
               for ( unsigned int i = 0; i < hist.size(); i++) 
               {
                  r[i] = i * delta;
                  pr[i] = (double) hist[i];
                  pr_n[i] = (double) hist[i];
               }
               cout << QString( "get mw <%1>\n" ).arg( te_filename2->text() );
               normalize_pr(r, &pr_n, &pr_error, get_mw(te_filename2->text(), false));
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header =
               QString("")
                  .sprintf(
                           "SOMO p(r) vs r data generated from %s by US_SOMO %s %s bin size %f mw %.2f Daltons area %.2f\n"
                           , model_filename.toLatin1().data()
                           , US_Version.toLatin1().data()
                           , REVISION
                           , delta
                           , get_mw(te_filename2->text(), false)
                           , compute_pr_area(pr, r)
                           );
               fprintf(fpr, "%s",
                       ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header.toLatin1().data() );
               fprintf(fpr, "r\tp(r)\tnorm. p(r)\n");
               for ( unsigned int i = 0; i < hist.size(); i++ )
               {
                  // allow zeros in output ... esp important for intermediates...
                  // if ( hist[i] ) {
                  fprintf(fpr, "%.6e\t%.6e\t%.6e\n", r[i], pr[i], pr_n[i]);
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.push_back(r[i]);
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.push_back(pr[i]);
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.push_back(pr_n[i]);
                  // }
               }
               fclose(fpr);
            }
            else
            {
#if defined(PR_DEBUG)
               cout << "can't create " << fpr_name << endl;
#endif
               editor->append(us_tr("WARNING: Could not create PR curve file: ") + fpr_name + "\n");
               // QMessageBox mb(us_tr("UltraScan Warning"),
               //                us_tr("The output file ") + fpr_name + us_tr(" could not be created."), 
               //                QMessageBox::Critical,
               //                QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
               // mb.exec();
               QMessageBox::critical( this,
                                      us_tr("UltraScan Warning"),
                                      us_tr("The output file ") + fpr_name + us_tr(" could not be created."), 
                                      QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton );
            }
         }
      } else {
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_mw = get_mw(te_filename2->text(), false);
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear( );
         vector < double > r;
         vector < double > pr;
         vector < double > pr_n;
         r.resize(hist.size());
         pr.resize(hist.size());
         pr_n.resize(hist.size());
         for ( unsigned int i = 0; i < hist.size(); i++) 
         {
            r[i] = i * delta;
            pr[i] = (double) hist[i];
            pr_n[i] = (double) hist[i];
         }
         normalize_pr(r, &pr_n, get_mw(te_filename2->text(), false));
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header =
            QString("")
            .sprintf(
                     "SOMO p(r) vs r data generated from %s by US_SOMO %s %s bin size %f mw %.2f Daltons area %.2f\n"
                     , model_filename.toLatin1().data()
                     , US_Version.toLatin1().data()
                     , REVISION
                     , delta
, get_mw(te_filename2->text(), false)
                     , compute_pr_area(pr, r)
                     );
         for ( unsigned int i = 0; i < hist.size(); i++ )
         {
            // allow zeros in output ... esp important for intermediates...
            // if ( hist[i] ) {
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.push_back(r[i]);
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.push_back(pr[i]);
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.push_back(pr_n[i]);
            // }
         }
      }
   } // models

   vector < double > r;
   vector < double > pr;
   vector < double > pr_error;
   r.resize(hist.size());
   pr.resize(hist.size());
   for ( unsigned int i = 0; i < hist.size(); i++) 
   {
      r[i] = i * delta;
      pr[i] = (double) hist[i];
#if defined(PR_DEBUG)
      printf("%e %e\n", r[i], pr[i]);
#endif
   }
   plotted_pr_not_normalized      .push_back(pr);
   plotted_pr_not_normalized_error.push_back( pr_error );
   
   plotted_pr_mw.push_back((float)get_mw(te_filename2->text()));
   if ( cb_normalize->isChecked() ) {
      normalize_pr(r, &pr, &pr_error, get_mw(te_filename2->text(),false));
   }

   plotted_r         .push_back(r);
   plotted_pr        .push_back(pr);
   plotted_pr_error  .push_back(pr_error);

   QString plot_name = use_name;
   int extension     = 0;
   while ( dup_plotted_pr_name_check.count(plot_name) )
   {
      plot_name = QString("%1-%2").arg(use_name).arg(++extension);
   }
   qsl_plotted_pr_names << plot_name;
   dup_plotted_pr_name_check[plot_name] = true;
   unsigned int p = plotted_r.size() - 1;

   QwtPlotCurve *curve = new QwtPlotCurve( plot_name );
   curve->setStyle( QwtPlotCurve::Lines );
   curve->setSamples(
                  (double *)&( r[ 0 ] ), 
                  (double *)&( pr[ 0 ] ),
                  (int)r.size()
                  );
   curve->setPen( QPen( plot_colors[ p % plot_colors.size() ], pen_width, Qt::SolidLine ) );
   curve->attach( plot_pr );

   if ( plot_pr_zoomer )
   {
      delete plot_pr_zoomer;
   }
   double minx;
   double maxx;
   double miny;
   double maxy;
   set_plot_pr_range( minx, maxx, miny, maxy );
   plot_pr->setAxisScale( QwtPlot::xBottom, minx, maxx );
   plot_pr->setAxisScale( QwtPlot::yLeft  , miny, maxy );

   plot_pr_zoomer = new ScrollZoomer(plot_pr->canvas());
   plot_pr_zoomer->setRubberBandPen( QPen(Qt::red, 1, Qt::DotLine ) );
   plot_pr_zoomer->setTrackerPen( QPen( Qt::red ) );

   plot_pr->replot();

   compute_rg_to_progress( r, pr, use_name );

   if ( do_make_iq ) {
      pr_to_iq( plotted_r.size() - 1, plot_name );
   }
   if ( do_pr_exact ) {
      pr_to_iq( pr_exact, QString( "%1_m%2_exact" ).arg( plot_name ).arg( our_saxs_options->fast_modulation ).replace( ".", "_" ) );
   }

   progress_pr->setMaximum(1);
   progress_pr->setValue(1);
   pb_plot_saxs_sans->setEnabled(bead_model_ok_for_saxs);
   // pb_plot_saxs_sans->setEnabled(false);
   pb_plot_pr->setEnabled(true);

   editor_msg( plot_colors[p % plot_colors.size()], plot_saxs->canvasBackground().color(), QString("P(r): Bin size: %1 \"%2\"\n").arg(delta).arg(QFileInfo(model_filename).fileName() ) );
}

void US_Hydrodyn_Saxs::load_plot_pr()
{
   load_pr(true);
}

//--------- thread for saxs I(q) plot -----------

// #define DEBUG_THREAD

saxs_Iq_thr_t::saxs_Iq_thr_t(int a_thread) : QThread()
{
   thread = a_thread;
   work_to_do = 0;
   work_done = 1;
   work_to_do_waiters = 0;
   work_done_waiters = 0;
}

void saxs_Iq_thr_t::saxs_Iq_thr_setup(
                                      vector < saxs_atom > *atoms,
                                      vector < vector < double > > *f,
                                      vector < vector < double > > *fc,
                                      vector < vector < double > > *fp,
                                      vector < double > *I,
                                      vector < double > *Ia,
                                      vector < double > *Ic,
                                      vector < double > *q,
                                      unsigned int threads,
                                      mQProgressBar *progress,
                                      QLabel *lbl_core_progress,
                                      bool *stopFlag
                                      )
{
   /* this starts up a new work load for the thread */
   this->atoms = atoms;
   this->f = f;
   this->fc = fc;
   this->fp = fp;
   this->I = I;
   this->Ia = Ia;
   this->Ic = Ic;
   this->q = q;
   this->threads = threads;
   this->progress = progress;
   this->lbl_core_progress = lbl_core_progress;
   this->stopFlag = stopFlag;

   work_mutex.lock();
   work_to_do = 1;
   work_done = 0;
   work_mutex.unlock();
   cond_work_to_do.wakeOne();
#if defined(DEBUG_THREAD)
   cerr << "thread " << thread << " has new work to do\n";
#endif
}

void saxs_Iq_thr_t::saxs_Iq_thr_shutdown()
{
   /* this signals the thread to exit the run method */
   work_mutex.lock();
   work_to_do = -1;
   work_mutex.unlock();
   cond_work_to_do.wakeOne();

#if defined(DEBUG_THREAD)
   cerr << "thread " << thread << " shutdown requested\n";
#endif
}

void saxs_Iq_thr_t::saxs_Iq_thr_wait()
{
   /* this is for the master thread to wait until the work is done */
   work_mutex.lock();

#if defined(DEBUG_THREAD)
   cerr << "thread " << thread << " has a waiter\n";
#endif

   while(!work_done) {
      cond_work_done.wait(&work_mutex);
   }
   work_done = 0;
   work_mutex.unlock();

#if defined(DEBUG_THREAD)
   cerr << "thread " << thread << " waiter released\n";
#endif
}

int saxs_Iq_thr_t::saxs_Iq_thr_work_status()
{
   work_mutex.lock();
   int retval = work_done;
   work_mutex.unlock();
   return retval;
}

void saxs_Iq_thr_t::run()
{
   while(1)
   {
      work_mutex.lock();
#if defined(DEBUG_THREAD)
      cerr << "thread " << thread << " waiting for work\n";
#endif
      work_to_do_waiters++;
      while(!work_to_do)
      {
         cond_work_to_do.wait(&work_mutex);
      }
      if(work_to_do == -1)
      {
#if defined(DEBUG_THREAD)
         cerr << "thread " << thread << " shutting down\n";
#endif
         work_mutex.unlock();
         return;
      }

      work_to_do_waiters = 0;
      work_mutex.unlock();
#if defined(DEBUG_THREAD)
      cerr << "thread " << thread << " starting work\n";
#endif
      
      unsigned int as = (*atoms).size();
      unsigned int as1 = as - 1;
      double rik; // distance from atom i to k 
      double qrik; // q * rik
      double sqrikd; // sin * q * rik / qrik
      unsigned int q_points = (*q).size();
      if ( !thread ) 
      {
         progress->setMaximum((int)(1.15f * as1 / threads));
      }
#if defined(DEBUG_THREAD)
      cerr << "thread " << thread << " as1 = " << as1 
           << " q_points " << q_points
           << endl;
#endif

      for ( unsigned int i = thread; i < as1; i += threads )
      {
#if defined(DEBUG_THREAD)
         cerr << "thread " << thread << " i = " << i << endl;
#endif
         if ( !thread ) 
         {
            // lbl_core_progress->setText(QString("Atom %1 of %2\n").arg(i+1).arg(as));
            progress->setValue(i+1);
            // qApp->processEvents();
         }
         if ( *stopFlag ) 
         {
            break;
         }
            
         for ( unsigned int k = i + 1; k < as; k++ )
         {
            rik = 
               sqrt(
                    ((*atoms)[i].pos[0] - (*atoms)[k].pos[0]) *
                    ((*atoms)[i].pos[0] - (*atoms)[k].pos[0]) +
                    ((*atoms)[i].pos[1] - (*atoms)[k].pos[1]) *
                    ((*atoms)[i].pos[1] - (*atoms)[k].pos[1]) +
                    ((*atoms)[i].pos[2] - (*atoms)[k].pos[2]) *
                    ((*atoms)[i].pos[2] - (*atoms)[k].pos[2])
                    );
#if defined(SAXS_DEBUG_F)
            cout << "dist atoms:  "
                 << i
                 << " "
                 << (*atoms)[i].saxs_name
                 << ","
                 << k
                 << " "
                 << (*atoms)[k].saxs_name
                 << " "
                 << rik
                 << endl;
#endif
            for ( unsigned int j = 0; j < q_points; j++ )
            {
               qrik = rik * (*q)[j];
               sqrikd = sin(qrik) / qrik;
               (*I)[j] += (*fp)[j][i] * (*f)[j][k] * sqrikd;
               (*Ia)[j] += (*f)[j][i] * (*f)[j][k] * sqrikd;
               (*Ic)[j] += (*fc)[j][i] * (*fc)[j][k] * sqrikd;
            }
         }
      }

#if defined(DEBUG_THREAD)
      cerr << "thread " << thread << " finished work\n";
#endif
      work_mutex.lock();
      work_done = 1;
      work_to_do = 0;
      work_mutex.unlock();
      cond_work_done.wakeOne();
   }
}

//--------- end thread for saxs I(q) plot -----------

void US_Hydrodyn_Saxs::show_plot_saxs()
{
   external_running = false;
   specname = "";

   if ( our_saxs_options->iqq_ask_target_grid &&
        plotted_q.size() )
   {
      ask_iq_target_grid();
   }

   if ( our_saxs_options->swh_excl_vol )
   {
      editor_msg("dark red", QString("WAT set to %1\n").arg( our_saxs_options->swh_excl_vol ));
   }

   if ( !source && our_saxs_options->saxs_iq_foxs ) 
   {
      // cout << model_filepathname << endl;
      run_saxs_iq_foxs( model_filepathname );
      while ( external_running )
      {
         mQThread::msleep( 333 );
         cout << "an event\n" << flush;
         qApp->processEvents();
      }
      return;
   }
   if ( !source && our_saxs_options->saxs_iq_crysol ) 
   {
      run_saxs_iq_crysol( model_filepathname );
      
      while ( external_running )
      {
         mQThread::msleep( 333 );
         cout << "an event\n" << flush;
         qApp->processEvents();
      }
      return;
   }
   if ( !source && our_saxs_options->saxs_iq_sastbx ) 
   {
      run_saxs_iq_sastbx( model_filepathname );
      while ( external_running )
      {
         mQThread::msleep( 333 );
         cout << "an event\n" << flush;
         qApp->processEvents();
      }
      return;
   }

   if ( our_saxs_options->alt_ff )
   {
      editor_msg( "blue", "Alternate FF method on\n" );
   }

   if ( our_saxs_options->use_somo_ff )
   {
      editor_msg( "blue", "Use somo ff (precomputed ff) on\n" );
      ff_sent_msg1.clear( );
      load_ff_table( our_saxs_options->default_ff_filename );
   } else {
      if ( our_saxs_options->five_term_gaussians )
      {
         editor_msg( "blue", "Using 5 term Gaussians\n" );
      }
   }

   if ( our_saxs_options->multiply_iq_by_atomic_volume )
   {
      editor_msg("blue", us_tr( "NOTICE: Multiplying I(q) by atomic volume\n" ) );
   }

   if ( our_saxs_options->saxs_iq_native_fast ) 
   {
      // calc_saxs_iq_native_debye();
      source ? calc_saxs_iq_native_fast_bead_model() : calc_saxs_iq_native_fast();
      return;
   }
   if ( our_saxs_options->saxs_iq_native_debye ) 
   {
#if defined( UHS_SAXSCMDS_SUPPORT )
      QFile f( ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "tmp" + SLASH + "saxscmds" );
      if ( f.exists() && f.open( QIODevice::ReadOnly ) )
      {
         editor_msg( "blue", "found somo/tmp/saxscmds" );
         QTextStream ts( &f );
         unsigned int line = 0;
         QRegExp rx_blank  ( "^\\s*$" );
         QRegExp rx_comment( "#.*$" );

         while ( !ts.atEnd() )
         {
            QString     qs = ts.readLine();
            line++;
            if ( qs.contains( rx_blank ) || qs.contains( rx_comment ) )
            {
               continue;
            }
            QStringList qsl = (qs ).split( QRegExp( "\\s+" ) , Qt::SkipEmptyParts );
            if ( qsl[ 0 ] == "run" )
            {
               editor_msg( "blue", QString( "saxscmds: running for: %1" ).arg( specname ) );
               source ? calc_saxs_iq_native_debye_bead_model() : calc_saxs_iq_native_debye();
               specname = "";
               continue;
            }
            if ( qsl[ 0 ] == "pos" &&
                 qsl.size() == 7 )
            {
               unsigned int nmodel = qsl[ 1 ].toUInt();
               unsigned int nchain = qsl[ 2 ].toUInt();
               unsigned int natom  = qsl[ 3 ].toUInt();
               if ( model_vector.size() > nmodel &&
                    model_vector[ nmodel ].molecule.size() > nchain &&
                    model_vector[ nmodel ].molecule[ nchain ].atom.size() > natom )
               {
                  model_vector[ nmodel ].molecule[ nchain ].atom[ natom ].coordinate.axis[ 0 ] = qsl[ 4 ].toFloat();
                  model_vector[ nmodel ].molecule[ nchain ].atom[ natom ].coordinate.axis[ 1 ] = qsl[ 5 ].toFloat();
                  model_vector[ nmodel ].molecule[ nchain ].atom[ natom ].coordinate.axis[ 2 ] = qsl[ 6 ].toFloat();
                  specname += QString( "_pos_%1_%2_%3_%4_%5_%6" )
                     .arg( qsl[ 1 ] )
                     .arg( qsl[ 2 ] )
                     .arg( qsl[ 3 ] )
                     .arg( qsl[ 4 ] )
                     .arg( qsl[ 5 ] )
                     .arg( qsl[ 6 ] );
               } else {
                  editor_msg( "red", QString( "atom not found line %1, running once & quitting\n" ).arg( line ) );
                  source ? calc_saxs_iq_native_debye_bead_model() : calc_saxs_iq_native_debye();
                  specname = "";
                  return;
               }
               continue;
            }
            editor_msg( "red", QString( "unrecogized or misformatted command in somo/tmp/saxscmds line %1\n" ).arg( line ) );
            return;
         }
      } else 
#endif
      {
         source ? calc_saxs_iq_native_debye_bead_model() : calc_saxs_iq_native_debye();
      }
      return;
   }
   if ( our_saxs_options->saxs_iq_native_hybrid ||
        our_saxs_options->saxs_iq_native_hybrid2 ||
        our_saxs_options->saxs_iq_native_hybrid3 ) 
   {
      source ? calc_saxs_iq_native_hybrid2_bead_model() : calc_saxs_iq_native_hybrid2();
      return;
   }
   if ( our_saxs_options->saxs_iq_native_sh )
   {
      source ? calc_saxs_iq_native_sh_bead_model() : calc_saxs_iq_native_sh();
      return;
   }
   QMessageBox::information(this, 
                            us_tr("Method not supported:"), 
                            QString(us_tr("The selected method is not supported for this model")));
   return;
   
   // don't forget to later merge deleted waters into model_vector
   // right now we are going with first residue map entry
   stopFlag = false;
   pb_stop->setEnabled(true);
   pb_plot_saxs_sans->setEnabled(false);
   pb_plot_pr->setEnabled(false);
   progress_saxs->reset();

#if defined(BUG_DEBUG)
   qApp->processEvents();
   cout << " sleep 1 a" << endl;
   sleep(1);
   cout << " sleep 1 a done" << endl;
#endif
   
   for ( unsigned int i = 0; i < selected_models.size(); i++ )
   {
      current_model = selected_models[i];
#if defined(SAXS_DEBUG)
      printf("creating sax_atoms %u\n", current_model);
#endif
      editor->append(QString("\n\nPreparing file %1 model %2 for SAXS plot.\n\n")
                     .arg(te_filename2->text())
                     .arg(current_model + 1));
      qApp->processEvents();
      if ( stopFlag ) 
      {
         editor->append(us_tr("Terminated by user request.\n"));
         progress_saxs->reset();
         lbl_core_progress->setText("");
         pb_plot_saxs_sans->setEnabled(true);
         pb_plot_pr->setEnabled(true);
         return;
      }
         
      vector < saxs_atom > atoms;
      saxs_atom new_atom;
      for (unsigned int j = 0; j < model_vector[current_model].molecule.size(); j++)
      {
         for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++)
         {
            PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);
            new_atom.pos[0] = this_atom->coordinate.axis[0];
            new_atom.pos[1] = this_atom->coordinate.axis[1];
            new_atom.pos[2] = this_atom->coordinate.axis[2];

            QString mapkey = QString("%1|%2").arg(this_atom->resName).arg(this_atom->name);
            if ( this_atom->name == "OXT" )
            {
               mapkey = "OXT|OXT";
            }

            QString hybrid_name = residue_atom_hybrid_map[mapkey];

            if ( hybrid_name.isEmpty() || !hybrid_name.length() )
            {
               cout << "error: hybrid name missing for " << this_atom->resName << "|" << this_atom->name << endl; 
               editor_msg( "red", QString("%1Molecule %2 Residue %3 %4 Hybrid name missing. Atom skipped.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(this_atom->resName)
                           .arg(this_atom->resSeq));
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(us_tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
               continue;
            }

            if ( !hybrid_map.count(hybrid_name) )
            {
               cout << "error: hybrid_map name missing for hybrid_name " << hybrid_name << endl;
               editor_msg( "red", 
                           QString("%1Molecule %2 Residue %3 %4 Hybrid %5 name missing from Hybrid file. Atom skipped.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(this_atom->resName)
                           .arg(this_atom->resSeq)
                           .arg(hybrid_name)
                           );
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(us_tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
               continue;
            }

            QString this_atom_name = hybrid_name == "ABB" ? "ABB" : this_atom->name;
            if ( !atom_map.count(this_atom_name + "~" + hybrid_name) )
            {
               cout << "error: (saxs 1) atom_map missing for hybrid_name "
                    << hybrid_name 
                    << " atom name "
                    << this_atom_name
                    << endl;
               editor_msg( "red",
                           QString("%1Molecule %2 Atom %3 Residue %4 %5 Hybrid %6 name missing from Atom file. Atom skipped.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(this_atom->name)
                           .arg(this_atom->resName)
                           .arg(this_atom->resSeq)
                           .arg(hybrid_name)
                           );
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(us_tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
               continue;
            }

            new_atom.excl_vol = atom_map[this_atom_name + "~" + hybrid_name].saxs_excl_vol;

            new_atom.saxs_name = hybrid_map[hybrid_name].saxs_name; 

            if ( !saxs_map.count(hybrid_map[hybrid_name].saxs_name) )
            {
               cout << "error: saxs_map missing for hybrid_name "
                    << hybrid_name 
                    << " saxs name "
                    << hybrid_map[hybrid_name].saxs_name
                    << endl;
               editor_msg( "red", 
                           QString("%1Molecule %2 Residue %3 %4 Hybrid %5 Saxs name %6 name missing from SAXS atom file. Atom skipped.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(this_atom->resName)
                           .arg(this_atom->resSeq)
                           .arg(hybrid_name)
                           .arg(hybrid_map[hybrid_name].saxs_name)
                           );
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(us_tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
               continue;
            }

#if defined(SAXS_DEBUG2)
            cout << "Atom: "
                 << this_atom->name
                 << " Residue: "
                 << this_atom->resName
                 << " SAXS atom: "
                 << new_atom.saxs_name 
                 << " Coordinates: "
                 << new_atom.pos[0] << " , "
                 << new_atom.pos[1] << " , "
                 << new_atom.pos[2] 
                 << " Excl vol: "
                 << new_atom.excl_vol
                 << endl;
#endif
            atoms.push_back(new_atom);
         }
      }
      
      // save the atoms to a temporary file
      QString fsaxs_atoms_name = 
         USglobal->config_list.root_dir + 
         SLASH "somo" + SLASH "saxs" + "SLASH" + "tmp" + SLASH + QString("%1").arg(te_filename2->text()) +
         QString("_%1").arg(current_model + 1) + 
         ".atoms";

      FILE *fsaxs_atoms = us_fopen(fsaxs_atoms_name, "w");
      if ( fsaxs_atoms ) 
      {
         for ( unsigned int i = 0; i < atoms.size(); i++ )
         {
            fprintf(fsaxs_atoms, "%s %.3f %.3f %.3f %.2f\n"
                    , atoms[i].saxs_name.toLatin1().data()
                    , atoms[i].pos[0]
                    , atoms[i].pos[1]
                    , atoms[i].pos[2]
                    , atoms[i].excl_vol);
         }
         fclose(fsaxs_atoms);
      }
         
#if defined(BUG_DEBUG)
      qApp->processEvents();
      cout << " sleep 1 b" << endl;
      sleep(1);
      cout << " sleep 1 b done" << endl;
#endif
      // ok now we have all the atoms
      unsigned int q_points = 
         (unsigned int)floor(((our_saxs_options->end_q - our_saxs_options->start_q) / our_saxs_options->delta_q) + .5) + 1;
         
      editor->append(QString("Number of atoms %1.\n"
                             "q range %2 to %3 with a stepsize of %4 giving %5 q-points.\n")
                     .arg(atoms.size())
                     .arg(our_saxs_options->start_q)
                     .arg(our_saxs_options->end_q)
                     .arg(our_saxs_options->delta_q)
                     .arg(q_points));
      qApp->processEvents();
      if ( stopFlag ) 
      {
         editor->append(us_tr("Terminated by user request.\n"));
         progress_saxs->reset();
         lbl_core_progress->setText("");
         pb_plot_saxs_sans->setEnabled(true);
         pb_plot_pr->setEnabled(true);
         return;
      }
#if defined(SAXS_DEBUG)
      cout << "start q:" << our_saxs_options->start_q
           << " end q:" << our_saxs_options->end_q
           << " delta q:" << our_saxs_options->delta_q
           << " q points:" << q_points
           << endl;
#endif
      vector < vector < double > > f;  // f(q,i) / atomic
      vector < vector < double > > fc;  // excluded volume
      vector < vector < double > > fp;  // f - fc
      f.resize(q_points);
      fc.resize(q_points);
      fp.resize(q_points);
      vector < double > q;  // store q grid
      vector < double > q2; // store q^2
      q.resize(q_points);
      q2.resize(q_points);

      for ( unsigned int j = 0; j < q_points; j++ )
      {
         f[j].resize(atoms.size());
         fc[j].resize(atoms.size());
         fp[j].resize(atoms.size());
         q[j] = our_saxs_options->start_q + j * our_saxs_options->delta_q;
         if ( q[j] < SAXS_MIN_Q ) 
         {
            q[j] = SAXS_MIN_Q;
         }
         q2[j] = q[j] * q[j];
      }

      double m_pi_vi23; // - pi * pow(v,2/3)
      float vi; // excluded water vol
      float vie; // excluded water * e density

#if defined(BUG_DEBUG)
      qApp->processEvents();
      cout << " sleep 1 c" << endl;
      sleep(1);
      cout << " sleep 1 c done" << endl;
#endif
#if defined(SAXS_DEBUG_F)
      cout << "atom #\tsaxs name\tq:";
      for ( unsigned int j = 0; j < q_points; j++ )
      {
         if (1 || (q[j] > .0099 && q[j] < .0101)) {
            cout << q[j] << "\t";
         }
      }
      cout << endl;
      cout << "\t\tq^2:";
      for ( unsigned int j = 0; j < q_points; j++ )
      {
         if (1 || (q[j] > .0099 && q[j] < .0101)) {
            cout << q2[j] << "\t";
         }
      }
      cout << endl;
#endif
      for ( unsigned int i = 0; i < atoms.size(); i++ )
      {
         saxs saxs = saxs_map[atoms[i].saxs_name];
         vi = atoms[i].excl_vol;
         vie = vi * our_saxs_options->water_e_density;
         m_pi_vi23 = -M_PI * pow((double)vi,2.0/3.0); // - pi * pow(v,2/3)
#if defined(SAXS_DEBUG_F)
         cout << i << "\t"
              << atoms[i].saxs_name << "\t";
         cout << QString("").sprintf("a1 %f b1 %f a2 %f b2 %f a3 %f b3 %f a4 %f b4 %f c %f\n"
                                     , saxs.a[0] , saxs.b[0]
                                     , saxs.a[1] , saxs.b[1]
                                     , saxs.a[2] , saxs.b[2]
                                     , saxs.a[3] , saxs.b[3]
                                     , saxs.c);
#endif
         
         for ( unsigned int j = 0; j < q_points; j++ )
         {
            // note: since there are only a few 'saxs' coefficient sets
            // the saxs.c + saxs.a[i] * exp() can be precomputed
            // possibly saving time... but this isn't our most computationally intensive step
            // so I'm holding off for now.

            f[j][i] = saxs.c + 
               saxs.a[0] * exp(-saxs.b[0] * q2[j]) +
               saxs.a[1] * exp(-saxs.b[1] * q2[j]) +
               saxs.a[2] * exp(-saxs.b[2] * q2[j]) +
               saxs.a[3] * exp(-saxs.b[3] * q2[j]);
            fc[j][i] =  vie * exp(m_pi_vi23 * q2[j]);
            fp[j][i] = f[j][i] - fc[j][i];
#if defined(SAXS_DEBUG_F)
            if (1 || (q[j] > .0099 && q[j] < .0101)) {
               cout << q[j] 
                    << "\t" 
                    << q2[j] 
                    << "\t" 
                    << f[j][i]
                    << "\n";
            }
#endif
#if defined(SAXS_DEBUG_FV)
            if (1 || (q[j] > .0099 && q[j] < .0101)) {
               cout << q[j] 
                    << "\t" 
                    << q2[j] 
                    << "\t" 
                    << vi
                    << "\t" 
                    << vie
                    << "\t" 
                    << m_pi_vi23
                    << "\t" 
                    << m_pi_vi23 * q2[j]
                    << "\t" 
                    << vie * exp(m_pi_vi23 * q2[j])
                    << "\t" 
                    << fp[j][i]
                    << "\n";
            }
#endif
#if defined(ONLY_PHYSICAL_F)
            if ( fp[j][i] < 0.0f ) 
            {
               fp[j][i] = 0.0f;
            }
#endif
         }
#if defined(SAXS_DEBUG_F)
         cout << endl;
#endif
      }
#if defined(SAXS_DEBUG)
      cout << "f' computed, now compute I\n";
#endif
      editor->append("f' computed, starting computation of I(q)\n");
      qApp->processEvents();
#if defined(BUG_DEBUG)
      qApp->processEvents();
      cout << " sleep 1 d" << endl;
      sleep(1);
      cout << " sleep 1 d done" << endl;
#endif
      if ( stopFlag ) 
      {
         editor->append(us_tr("Terminated by user request.\n"));
         progress_saxs->reset();
         lbl_core_progress->setText("");
         pb_plot_saxs_sans->setEnabled(true);
         pb_plot_pr->setEnabled(true);
         return;
      }
      vector < double > I;
      vector < double > Ia;
      vector < double > Ic;
      I.resize(q_points);
      Ia.resize(q_points);
      Ic.resize(q_points);
      for ( unsigned int j = 0; j < q_points; j++ )
      {
         I[j] = 0.0f;
         Ia[j] = 0.0f;
         Ic[j] = 0.0f;
      }
#if defined(BUG_DEBUG)
      qApp->processEvents();
      cout << " sleep 1 d.1" << endl;
      sleep(1);
      cout << " sleep 1 d.1 done" << endl;
#endif
      if ( 0 && // disabled for now
           ((US_Hydrodyn *)us_hydrodyn)->advanced_config.experimental_threads &&
           USglobal->config_list.numThreads > 1 )
      {
         unsigned int j;
         unsigned int threads = USglobal->config_list.numThreads;
         editor->append(QString("Using %1 threads.\n").arg(threads));
         vector < saxs_Iq_thr_t* > saxs_Iq_thr_threads(threads);
         for ( j = 0; j < threads; j++ )
         {
            saxs_Iq_thr_threads[j] = new saxs_Iq_thr_t(j);
            saxs_Iq_thr_threads[j]->start();
            
         }
         vector < vector < double > > It;
         vector < vector < double > > Ita;
         vector < vector < double > > Itc;
         It.resize(threads);
         Ita.resize(threads);
         Itc.resize(threads);
         for ( j = 0; j < threads; j++ )
         {
# if defined(DEBUG_THREAD)
            cout << "thread " << j << endl;
# endif
            It[j].resize(q_points);
            Ita[j].resize(q_points);
            Itc[j].resize(q_points);
            for ( unsigned int k = 0; k < q_points; k++ )
            {
               It[j][k] = 0.0f;
               Ita[j][k] = 0.0f;
               Itc[j][k] = 0.0f;
            }
            
            saxs_Iq_thr_threads[j]->saxs_Iq_thr_setup(
                                                      &atoms,
                                                      &f,
                                                      &fc,
                                                      &fp,
                                                      &It[j],
                                                      &Ita[j],
                                                      &Itc[j],
                                                      &q,
                                                      threads,
                                                      progress_saxs,
                                                      lbl_core_progress,
                                                      &stopFlag
                                                      );

         }
         // sleep app loop
         {
            int all_done;
            do {
               all_done = threads;
               for ( j = 0; j < threads; j++ )
               {
                  all_done -= saxs_Iq_thr_threads[j]->saxs_Iq_thr_work_status();
               }
               qApp->processEvents();
               mQThread::msleep( 333 );
            } while(all_done);
         }
         
         // wait for work to complete

         for ( j = 0; j < threads; j++ )
         {
            saxs_Iq_thr_threads[j]->saxs_Iq_thr_wait();
         }

         // destroy
         
         for ( j = 0; j < threads; j++ )
         {
            saxs_Iq_thr_threads[j]->saxs_Iq_thr_shutdown();
         }
         
         for ( j = 0; j < threads; j++ )
         {
            saxs_Iq_thr_threads[j]->wait();
         }
         
         for ( j = 0; j < threads; j++ )
         {
            delete saxs_Iq_thr_threads[j];
         }
         
         if ( stopFlag ) 
         {
            editor->append(us_tr("Terminated by user request.\n"));
            progress_saxs->reset();
            lbl_core_progress->setText("");
            pb_plot_saxs_sans->setEnabled(true);
            pb_plot_pr->setEnabled(true);
            return;
         }

         // merge results
         for ( j = 0; j < threads; j++ )
         {
            for ( unsigned int k = 0; k < q_points; k++ )
            {
               I[k] += It[j][k];
               Ia[k] += Ita[j][k];
               Ic[k] += Itc[j][k];
            }
         }
      }
      else
      { 
         // not threaded
         unsigned int as = atoms.size();
         unsigned int as1 = as - 1;
         double rik; // distance from atom i to k 
         double qrik; // q * rik
         double sqrikd; // sin * q * rik / qrik
         progress_saxs->setMaximum((int)(as1 * 1.15));
         for ( unsigned int i = 0; i < as1; i++ )
         {
            // QString lcp = QString("Atom %1 of %2").arg(i+1).arg(as);
            // cout << lcp << endl;
            // lbl_core_progress->setText(lcp);
            progress_saxs->setValue(i+1);
            qApp->processEvents();
            if ( stopFlag ) 
            {
               editor->append(us_tr("Terminated by user request.\n"));
               progress_saxs->reset();
               lbl_core_progress->setText("");
               pb_plot_saxs_sans->setEnabled(true);
               pb_plot_pr->setEnabled(true);
               return;
            }
            for ( unsigned int k = i + 1; k < as; k++ )
            {
               rik = 
                  sqrt(
                       (atoms[i].pos[0] - atoms[k].pos[0]) *
                       (atoms[i].pos[0] - atoms[k].pos[0]) +
                       (atoms[i].pos[1] - atoms[k].pos[1]) *
                       (atoms[i].pos[1] - atoms[k].pos[1]) +
                       (atoms[i].pos[2] - atoms[k].pos[2]) *
                       (atoms[i].pos[2] - atoms[k].pos[2])
                       );
#if defined(SAXS_DEBUG_F)
               cout << "dist atoms:  "
                    << i
                    << " "
                    << atoms[i].saxs_name
                    << ","
                    << k
                    << " "
                    << atoms[k].saxs_name
                    << " "
                    << rik
                    << endl;
#endif
               for ( unsigned int j = 0; j < q_points; j++ )
               {
                  qrik = rik * q[j];
                  sqrikd = sin(qrik) / qrik;
                  I[j] += fp[j][i] * fp[j][k] * sqrikd;
                  Ia[j] += f[j][i] * f[j][k] * sqrikd;
                  Ic[j] += fc[j][i] * fc[j][k] * sqrikd;
#if defined(SAXS_DEBUG_F)
                  cout << QString("").sprintf("I[%f] += (%f * %f) * (sin(%f) / %f) == %f\n"
                                              , q[j]
                                              , fp[j][i]
                                              , fp[j][k]
                                              , qrik
                                              , qrik
                                              , I[j]);
#endif
                  
                  
                  
               }
            }
         }
      } // end threads logic

#if defined(I_MULT_2)
      for ( unsigned int j = 0; j < q_points; j++ )
      {
         I[j] *= 2; // we only computed one symmetric side
         Ia[j] *= 2; // we only computed one symmetric side
         Ic[j] *= 2; // we only computed one symmetric side
#if defined(SAXS_DEBUG_F)
         cout << QString("").sprintf("I[%f] = %f\n",
                                     q[j],
                                     I[j]);
#endif
      }
#endif
#if defined(BUG_DEBUG)
      qApp->processEvents();
      cout << " sleep 1 d.2" << endl;
      sleep(1);
      cout << " sleep 1 d.2 done" << endl;
#endif
      lbl_core_progress->setText("");
      qApp->processEvents();
      progress_saxs->reset();
#if defined(SAXS_DEBUG)
      cout << "I computed\n";
#endif
      editor->append("I(q) computed.\n");
#if defined(BUG_DEBUG)
      qApp->processEvents();
      cout << " sleep 1 e" << endl;
      sleep(1);
      cout << " sleep 1 e done" << endl;
#endif
#if QT_VERSION < 0x040000
      long Iq = plot_saxs->insertCurve("I(q) vs q");
#else
      QwtPlotCurve *curve = new QwtPlotCurve( "I(q) vs q" );
#endif

      QString name = 
         QString("%1_%2")
         .arg(QFileInfo(te_filename2->text()).fileName())
         .arg(current_model + 1);
      QString plot_name = name;
      int extension = 0;

      while ( dup_plotted_iq_name_check.count(plot_name) )
      {
         plot_name = QString("%1-%2").arg(name).arg(++extension);
      }
      qsl_plotted_iq_names << plot_name;
      dup_plotted_iq_name_check[plot_name] = true;

      plotted_iq_names_to_pos[plot_name] = plotted_Iq.size();

#if QT_VERSION < 0x040000
      plotted_Iq.push_back(Iq);
      plot_saxs->setCurveStyle(Iq, QwtCurve::Lines);
#else
      plotted_Iq.push_back( curve );
      curve->setStyle( QwtPlotCurve::Lines );
#endif
      plotted_q.push_back(q);
      {
         vector < double > q2(q.size());
         for ( unsigned int i = 0; i < q.size(); i++ )
         {
            q2[i] = q[i] * q[i];
         }
         plotted_q2.push_back(q2);
      }
      plotted_I.push_back(I);
      push_back_zero_I_error();      
      unsigned int p = plotted_q.size() - 1;
#if defined(SAXS_DEBUG)
      cout << "plot # " << p << endl;
#endif
      for ( unsigned int i = 0; i < plotted_I[p].size(); i++ ) 
      {
         plotted_I[p][i] = log10(plotted_I[p][i]);
      }
#if QT_VERSION < 0x040000
      plot_saxs->setCurveData(Iq, 
                              cb_guinier->isChecked() ?
                              (double *)&(plotted_q2[p][0]) : (double *)&(plotted_q[p][0]), 
                              (double *)&(plotted_I[p][0]), q_points);
      plot_saxs->setCurvePen(Iq, QPen(plot_colors[p % plot_colors.size()], pen_width, SolidLine));
#else
      curve->setSamples(
                     cb_guinier->isChecked() ?
                     (double *)&(plotted_q2[p][0]) : (double *)&(plotted_q[p][0]), 
                     (double *)&(plotted_I[p][0]),
                     q_points
                     );
      curve->setPen( QPen( plot_colors[ p % plot_colors.size() ], pen_width, Qt::SolidLine ) );
      curve->attach( plot_saxs );
#endif
      plot_saxs->replot();

      // save the data to a file
      if ( create_native_saxs )
      {
         QString fsaxs_name = 
            USglobal->config_list.root_dir + 
            SLASH + "somo" + SLASH + "saxs" + SLASH + saxs_filestring();
#if defined(SAXS_DEBUG)
         cout << "output file " << fsaxs_name << endl;
#endif
         bool ok_to_write = true;
         if ( QFile::exists(fsaxs_name) &&
              !((US_Hydrodyn *)us_hydrodyn)->overwrite )
         {
            fsaxs_name = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fsaxs_name, 0, this);
            ok_to_write = true;
            
#if defined(OLD_WAY)
            switch( QMessageBox::information( this, 
                                              us_tr("Overwrite file:") + "SAXS I(q) vs. q" + us_tr("output file"),
                                              us_tr("The file named \"") + 
                                              saxs_filestring() +
                                              + us_tr("\" will be overwriten"),
                                              "&Ok",  "&Cancel", 0,
                                              0,      // Enter == button 0
                                              1 ) ) { // Escape == button 1
            case 0: // just go ahead
               ok_to_write = true;
               break;
            case 1: // Cancel clicked or Escape pressed
               ok_to_write = false;
               break;
            }
#endif
         }
         
         if ( ok_to_write )
         {
            FILE *fsaxs = us_fopen(fsaxs_name, "w");
            if ( fsaxs ) 
            {
#if defined(SAXS_DEBUG)
               cout << "writing " << fsaxs_name << endl;
#endif
               editor->append(us_tr("SAXS curve file: ") + fsaxs_name + us_tr(" created.\n"));
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header =
                  QString("")
                  .sprintf(
                           "Simulated SAXS data generated from %s by US_SOMO %s %s q(%.3f:%.3f) step %.3f\n"
                           , model_filename.toLatin1().data()
                           , US_Version.toLatin1().data()
                           , REVISION
                           , our_saxs_options->start_q
                           , our_saxs_options->end_q
                           , our_saxs_options->delta_q
                           );
               fprintf(fsaxs, "%s",
                       ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header.toLatin1().data() );
               for ( unsigned int i = 0; i < q.size(); i++ )
               {
                  fprintf(fsaxs, "%.6e\t%.6e\t%.6e\t%.6e\n", q[i], I[i], Ia[i], Ic[i]);
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.push_back(q[i]);
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.push_back(I[i]);
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.push_back(Ia[i]);
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.push_back(Ic[i]);
               }
               fclose(fsaxs);
            } 
            else
            {
#if defined(SAXS_DEBUG)
               cout << "can't create " << fsaxs_name << endl;
#endif
               editor->append(us_tr("WARNING: Could not create SAXS curve file: ") + fsaxs_name + "\n");
               // QMessageBox mb(us_tr("UltraScan Warning"),
               //                us_tr("The output file ") + fsaxs_name + us_tr(" could not be created."), 
               //                QMessageBox::Critical,
               //                QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
               // mb.exec();
               QMessageBox::critical( this,
                                      us_tr("UltraScan Warning"),
                                      us_tr("The output file ") + fsaxs_name + us_tr(" could not be created."), 
                                      QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton );
            }
         }
      } else {
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header =
            QString("")
            .sprintf(
                     "Simulated SAXS data generated from %s by US_SOMO %s %s q(%.3f:%.3f) step %.3f\n"
                     , model_filename.toLatin1().data()
                     , US_Version.toLatin1().data()
                     , REVISION
                     , our_saxs_options->start_q
                     , our_saxs_options->end_q
                     , our_saxs_options->delta_q
                     );
         for ( unsigned int i = 0; i < q.size(); i++ )
         {
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.push_back(q[i]);
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.push_back(I[i]);
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.push_back(Ia[i]);
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.push_back(Ic[i]);
         }
      }
   }
   pb_plot_saxs_sans->setEnabled(true);
   pb_plot_pr->setEnabled(true);
}

void US_Hydrodyn_Saxs::print()
{
#ifndef NO_EDITOR_PRINT
   const int MARGIN = 10;
   printer.setPageSize(QPrinter::Letter);

   if ( printer.setup(this) ) {      // opens printer dialog
      QPainter p;
      p.begin( &printer );         // paint on printer
      p.setFont(editor->font() );
      int yPos      = 0;         // y position for each line
      QFontMetrics fm = p.fontMetrics();
      //  QPaintDeviceMetrics metrics( &printer ); // need width/height
      // of printer surface
      for( int i = 0 ; i < editor->lines() ; i++ ) {
         if ( MARGIN + yPos > printer.height() - MARGIN ) {
            printer.newPage();      // no more room on this page
            yPos = 0;         // back to top of page
         }
         p.drawText( MARGIN, MARGIN + yPos,
                     printer.width(), fm.lineSpacing(),
                                   ExpandTabs | DontClip,
                                   editor->toPlainText( i ) );
         yPos = yPos + fm.lineSpacing();
      }
      p.end();            // send job to printer
   }
#endif
}

void US_Hydrodyn_Saxs::load_plot_saxs()
{
   rb_sans->isChecked() ? load_sans( "", true ) : load_saxs( "", true );
}

void US_Hydrodyn_Saxs::set_grid()
{
   if ( everything_plotted_has_same_grid_as_set() )
   {
      if ( plotted_q.size() )
      {
         QMessageBox::information(this, 
                                  us_tr("US-SOMO: Set grid"),
                                  QString(us_tr("NOTE: Everything plotted already matches the set grid")));
      } else {
         QMessageBox::information(this, 
                                  us_tr("US-SOMO: Set grid"),
                                  QString(us_tr("Nothing plotted")));
         return;
      }
   }
   ask_iq_target_grid( true );
}

void US_Hydrodyn_Saxs::clear_plot_saxs_data()
{
   qsl_plotted_iq_names.clear( );
   dup_plotted_iq_name_check.clear( );
   plotted_iq_names_to_pos.clear( );
   plotted_Iq.clear( );
   plotted_q.clear( );
   plotted_q2.clear( );
   plotted_I.clear( );
   plotted_I_error.clear( );
   plot_saxs->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_saxs->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   plot_saxs->replot();
#if QT_VERSION < 0x040000
   plotted_Gp.clear( );
   plotted_cs_Gp.clear( );
   plotted_Rt_Gp.clear( );
#else
   plotted_Gp_curves.clear( );
   plotted_cs_Gp_curves.clear( );
   plotted_Rt_Gp_curves.clear( );
#endif
   plotted_guinier_valid.clear( );
   plotted_guinier_plotted.clear( );
   plotted_guinier_lowq2.clear( );
   plotted_guinier_highq2.clear( );
   plotted_guinier_a.clear( );
   plotted_guinier_b.clear( );
   plotted_guinier_x.clear( );
   plotted_guinier_y.clear( );
   plotted_cs_guinier_valid.clear( );
   plotted_cs_guinier_plotted.clear( );
   plotted_cs_guinier_lowq2.clear( );
   plotted_cs_guinier_highq2.clear( );
   plotted_cs_guinier_a.clear( );
   plotted_cs_guinier_b.clear( );
   plotted_cs_guinier_x.clear( );
   plotted_cs_guinier_y.clear( );
   plotted_Rt_guinier_valid.clear( );
   plotted_Rt_guinier_plotted.clear( );
   plotted_Rt_guinier_lowq2.clear( );
   plotted_Rt_guinier_highq2.clear( );
   plotted_Rt_guinier_a.clear( );
   plotted_Rt_guinier_b.clear( );
   plotted_Rt_guinier_x.clear( );
   plotted_Rt_guinier_y.clear( );

#if QT_VERSION >= 0x040000
   saxs_legend_vis = false;
   set_saxs_legend(); 
#endif
}

void US_Hydrodyn_Saxs::clear_plot_saxs_and_replot_experimental()
{
   // save 1st data experimental data

   vector < vector < double > > qs;
   vector < vector < double > > Is;
   vector < vector < double > > I_errors;
   vector < QString >           names;

   if ( iq_plot_only_experimental_present() )
   {
      for ( unsigned int i = 0; i < plotted_I_error.size() - 1; i++ )
      {
         qs.      push_back( plotted_q           [ i ] );
         Is.      push_back( plotted_I           [ i ] );
         I_errors.push_back( plotted_I_error     [ i ] );
         names.   push_back( qsl_plotted_iq_names[ i ] );
      }
   } else {
      for ( unsigned int i = 0; i < plotted_I_error.size(); i++ )
      {
         if ( is_nonzero_vector( plotted_I_error[ i ] ) )
         {
            qs.      push_back( plotted_q           [ i ] );
            Is.      push_back( plotted_I           [ i ] );
            I_errors.push_back( plotted_I_error     [ i ] );
            names.   push_back( qsl_plotted_iq_names[ i ] );
         }
      }
   }

   clear_plot_saxs_data();

   plotted = false;
   for ( unsigned int i = 0; i < names.size(); i++ )
   {
      // puts("replotting");
      plot_one_iqq( qs[ i ], Is[ i ], I_errors[ i ], names[ i ] );
   }
   if ( plotted )
   {
      editor_msg( "black", "I(q) plot done\n" );
      plotted = false;
   }
   rescale_plot();
}

void US_Hydrodyn_Saxs::clear_plot_saxs( bool quiet )
{
   if ( !quiet &&
        iq_plot_experimental_and_calculated_present() )
   {
      clear_plot_saxs_and_replot_experimental();
      set_eb();
   } else {
      clear_plot_saxs_data();
   }
      
   bool any_to_close = false;
   for ( map < QString, bool >::iterator it = saxs_iqq_residuals_widgets.begin();
         it != saxs_iqq_residuals_widgets.end();
         it++ )
   {
      if ( it->second == true )
      {
         any_to_close = true;
         break;
      }
   }

   bool close_windows = quiet;
   if ( any_to_close && !quiet )
   {
      switch( QMessageBox::information( this, 
                                        us_tr("Close I(q) vs q residual windows"),
                                        us_tr("Do you want to close the I(q) vs q residual window(s)?\n"
                                           "Note: if you choose not to close them now, they must be manually closed\n"
                                           ),
                                        "&Yes",  "&No", 0,
                                        0,      // Enter == button 0
                                        1 ) ) { // Escape == button 2
            case 0: // just go ahead
               close_windows = true;
               break;
            default: // Cancel clicked or Escape pressed
               break;
      }
   }

   for ( map < QString, bool >::iterator it = saxs_iqq_residuals_widgets.begin();
         it != saxs_iqq_residuals_widgets.end();
         it++ )
   {
      if ( it->second == true )
      {
         it->second = false;
         if ( saxs_iqq_residuals_windows.count(it->first) )
         {
            if ( close_windows )
            {
               saxs_iqq_residuals_windows[it->first]->close();
            } else {
               saxs_iqq_residuals_windows[it->first]->detached = true;
            }
         }
      }
   }
   saxs_search_update_enables();
}

void US_Hydrodyn_Saxs::saxs_search_update_enables()
{
   if ( ((US_Hydrodyn*)us_hydrodyn)->saxs_search_widget )
   {
      ((US_Hydrodyn*)us_hydrodyn)->saxs_search_window->update_enables();
   }
   if ( ((US_Hydrodyn*)us_hydrodyn)->saxs_screen_widget )
   {
      ((US_Hydrodyn*)us_hydrodyn)->saxs_screen_window->update_enables();
   }
}

void US_Hydrodyn_Saxs::show_plot_sans()
{
   if ( our_saxs_options->iqq_ask_target_grid &&
        plotted_q.size() )
   {
      ask_iq_target_grid();
   }

   if ( our_saxs_options->swh_excl_vol )
   {
      editor_msg("dark red", QString("WAT set to %1\n").arg( our_saxs_options->swh_excl_vol ));
   }

   if ( !source && our_saxs_options->sans_iq_cryson ) 
   {
      run_sans_iq_cryson( model_filepathname );
      while ( external_running )
      {
         mQThread::msleep( 333 );
         cout << "an event\n" << flush;
         qApp->processEvents();
      }
      return;
   }

   QMessageBox::information(this, 
                            us_tr("Method not implemented:"), 
                            QString(us_tr("The selected method is not yet implemented.")));
}

void US_Hydrodyn_Saxs::show_plot_saxs_sans()
{
   cb_guinier->setChecked( false );
   set_guinier();
   rb_sans->isChecked() ? show_plot_sans() : show_plot_saxs();
   saxs_search_update_enables();
}

void US_Hydrodyn_Saxs::load_saxs_sans()
{
   cb_guinier->setChecked( false );
   set_guinier();
   rb_sans->isChecked() ? load_sans() : load_saxs();
}
      
void US_Hydrodyn_Saxs::update_saxs_sans()
{
   set_current_method_buttons();
   if ( rb_sans->isChecked() ) 
   {
      pb_plot_saxs_sans->setText(us_tr("Compute SANS Curve"));
      pb_load_saxs_sans->setText(us_tr("Load SANS Curve"));
      SET_WIDTH_FROM_TEXT_LEN( pb_load_saxs_sans );
      pb_clear_plot_saxs->setText(us_tr("Clear SANS Curve"));
      SET_WIDTH_FROM_TEXT_LEN( pb_clear_plot_saxs );
      plot_saxs->setTitle((cb_guinier->isChecked() ? 
                           ( cb_cs_guinier->isChecked() ?
                             "CS Guinier " : 
                             ( cb_Rt_guinier->isChecked() ? "Transverse Guinier " : "Guinier " ) )
                           : "") + us_tr("SANS Curve"));

      rb_saxs_iq_native_debye->setEnabled(false);
      rb_saxs_iq_native_sh   ->setEnabled(false);
      rb_saxs_iq_native_fast ->setEnabled(false);

      rb_saxs_iq_crysol      ->setEnabled(false);

      if ( started_in_expert_mode )
      {
         rb_saxs_iq_native_hybrid ->setEnabled(false);
         rb_saxs_iq_native_hybrid2->setEnabled(false);
         rb_saxs_iq_foxs          ->setEnabled(false);
         rb_saxs_iq_foxs          ->setEnabled(false);
         rb_saxs_iq_sastbx        ->setEnabled(false);
      }

      rb_sans_iq_native_debye->setEnabled(true);
      rb_sans_iq_native_sh   ->setEnabled(true);
      rb_sans_iq_native_fast ->setEnabled(true);
      rb_sans_iq_cryson      ->setEnabled(true);
   } else {
      pb_plot_saxs_sans->setText(us_tr("Compute SAXS Curve"));
      pb_load_saxs_sans->setText(us_tr("Load SAXS Curve"));
      SET_WIDTH_FROM_TEXT_LEN( pb_load_saxs_sans );
      pb_clear_plot_saxs->setText(us_tr("Clear SAXS Curve"));
      SET_WIDTH_FROM_TEXT_LEN( pb_clear_plot_saxs );
      plot_saxs->setTitle((cb_guinier->isChecked() ? 
                           ( cb_cs_guinier->isChecked() ?
                             "CS Guinier " : 
                             ( cb_Rt_guinier->isChecked() ? "Transverse Guinier " : "Guinier " ) )
                           : "") + us_tr("SAXS Curve"));

      rb_saxs_iq_native_debye->setEnabled(true);
      rb_saxs_iq_native_sh   ->setEnabled(true);
      rb_saxs_iq_native_fast ->setEnabled(true);
      rb_saxs_iq_crysol      ->setEnabled(true);

      if ( started_in_expert_mode )
      {
         rb_saxs_iq_native_hybrid ->setEnabled(true);
         rb_saxs_iq_native_hybrid2->setEnabled(true);
         rb_saxs_iq_foxs          ->setEnabled(true);
         rb_saxs_iq_foxs          ->setEnabled(true);
         rb_saxs_iq_sastbx        ->setEnabled(true);
      }

      rb_sans_iq_native_debye->setEnabled(false);
      rb_sans_iq_native_sh   ->setEnabled(false);
      rb_sans_iq_native_fast ->setEnabled(false);
      rb_sans_iq_cryson      ->setEnabled(false);
   }
   update_iqq_suffix();
}

void US_Hydrodyn_Saxs::clear_display()
{
   editor->clear( );
   editor->append("\n\n");
}

void US_Hydrodyn_Saxs::update_font()
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

void US_Hydrodyn_Saxs::save()
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

void US_Hydrodyn_Saxs::select_atom_file()
{
   QString old_filename = atom_filename;
   atom_filename = QFileDialog::getOpenFileName( this , windowTitle() , US_Config::get_home_dir() + "etc" , "*.atom *.ATOM" );
   if (atom_filename.isEmpty())
   {
      atom_filename = old_filename;
      return;
   }
   else
   {
      select_atom_file(atom_filename);
   }
}

void US_Hydrodyn_Saxs::select_atom_file(const QString &filename)
{
   QString str1;
   QFileInfo fi(filename);
   lbl_atom_table->setText(fi.baseName() + "." + fi.completeSuffix());
   atom_list.clear( );
   atom_map.clear( );
   QFile f(filename);
   if (f.open(QIODevice::ReadOnly|QIODevice::Text))
   {
      QTextStream ts(&f);
      while (!ts.atEnd())
      {
         ts >> current_atom.name;
         ts >> current_atom.hybrid.name;
         ts >> current_atom.hybrid.mw;
         ts >> current_atom.hybrid.radius;
         ts >> current_atom.saxs_excl_vol;
         str1 = ts.readLine(); // read rest of line
         if (!current_atom.name.isEmpty() && current_atom.hybrid.radius > 0.0 && current_atom.hybrid.mw > 0.0)
         {
            atom_list.push_back(current_atom);
            atom_map[current_atom.name + "~" + current_atom.hybrid.name] = current_atom;
         }
      }
      f.close();
   }
   // add generic ABB
   {
      atom abb_atom;
      abb_atom.name                    = "ABB";
      abb_atom.hybrid.name             = "ABB";
      abb_atom.hybrid.mw               = ((US_Hydrodyn *)us_hydrodyn)->misc.avg_mass;
      abb_atom.hybrid.ionized_mw_delta = 0;
      abb_atom.hybrid.radius           = ((US_Hydrodyn *)us_hydrodyn)->misc.avg_radius;
      abb_atom.hybrid.scat_len         = 0;
      abb_atom.hybrid.saxs_name        = "ABB";
      abb_atom.hybrid.num_elect        = ((US_Hydrodyn *)us_hydrodyn)->misc.avg_num_elect;
      abb_atom.hybrid.protons          = ((US_Hydrodyn *)us_hydrodyn)->misc.avg_protons;
      abb_atom.saxs_excl_vol           = (4/3)*M_PI*pow(((US_Hydrodyn *)us_hydrodyn)->misc.avg_radius, 3 );

      atom_list.push_back(abb_atom);
      atom_map[abb_atom.name + "~" + abb_atom.hybrid.name] = abb_atom;
   }   
}

void US_Hydrodyn_Saxs::select_hybrid_file()
{
   QString old_filename = hybrid_filename;
   hybrid_filename = QFileDialog::getOpenFileName( this , windowTitle() , US_Config::get_home_dir() + "etc" , "*.hybrid *.HYBRID" );

   if (hybrid_filename.isEmpty())
   {
      hybrid_filename = old_filename;
      return;
   }
   else
   {
      select_hybrid_file(hybrid_filename);
   }
}

void US_Hydrodyn_Saxs::select_hybrid_file(const QString &filename)
{
   QString str1;
   QFileInfo fi(filename);
   lbl_hybrid_table->setText(fi.baseName() + "." + fi.completeSuffix());
   QFile f(filename);
   hybrid_list.clear( );
   hybrid_map.clear( );
   QRegExp count_hydrogens("H(\\d)");
   if (f.open(QIODevice::ReadOnly|QIODevice::Text))
   {
      QTextStream ts(&f);
      while (!ts.atEnd())
      {
         ts >> current_hybrid.saxs_name;
         ts >> current_hybrid.name;
         ts >> current_hybrid.mw;
         ts >> current_hybrid.radius;
         ts >> current_hybrid.scat_len;
         ts >> current_hybrid.exch_prot;
         ts >> current_hybrid.num_elect;
         current_hybrid.hydrogens = 0;
         if ( count_hydrogens.indexIn( current_hybrid.name ) != -1 )
         {
            current_hybrid.hydrogens = count_hydrogens.cap(1).toUInt();
         }

         str1 = ts.readLine(); // read rest of line
         if (!current_hybrid.name.isEmpty() && current_hybrid.radius > 0.0 && current_hybrid.mw > 0.0)
         {
            hybrid_list.push_back(current_hybrid);
            hybrid_map[current_hybrid.name] = current_hybrid;
         }
      }
      f.close();
   }
   // add ABB defaults
   {
      hybridization abb_hybrid;
      abb_hybrid.saxs_name        = "ABB";
      abb_hybrid.name             = "ABB";
      abb_hybrid.mw               = ((US_Hydrodyn *)us_hydrodyn)->misc.avg_mass;
      abb_hybrid.ionized_mw_delta = 0;
      abb_hybrid.radius           = ((US_Hydrodyn *)us_hydrodyn)->misc.avg_radius;
      abb_hybrid.scat_len         = 0;
      abb_hybrid.num_elect        = ((US_Hydrodyn *)us_hydrodyn)->misc.avg_num_elect;
      abb_hybrid.protons          = ((US_Hydrodyn *)us_hydrodyn)->misc.avg_protons;
      hybrid_list.push_back( abb_hybrid );
      hybrid_map[ abb_hybrid.name ] = abb_hybrid;
   }
}

void US_Hydrodyn_Saxs::select_saxs_file()
{
   QString old_filename = saxs_filename;
      
   saxs_filename = QFileDialog::getOpenFileName( this , windowTitle() , US_Config::get_home_dir() + "etc" , "*.saxs_atoms *.SAXS_ATOMS" );
   if (saxs_filename.isEmpty())
   {
      saxs_filename = old_filename;
      return;
   }
   else
   {
      select_saxs_file(saxs_filename);
   }
}

void US_Hydrodyn_Saxs::select_saxs_file(const QString &filename)
{
   QString str1;
   QFileInfo fi(filename);
   lbl_saxs_table->setText(fi.baseName() + "." + fi.completeSuffix());
   QFile f(filename);
   saxs_list.clear( );
   saxs_map.clear( );
   if (f.open(QIODevice::ReadOnly|QIODevice::Text))
   {
      QTextStream ts(&f);
      int line = 0;
      while (!ts.atEnd())
      {
         QString    qs  = ts.readLine();
         line++;
         if ( qs.contains( QRegExp( "^\\s+#" ) ) )
         {
            continue;
         }
         qs.trimmed();
         QStringList qsl = (qs ).split( QRegExp( "\\s+" ) , Qt::SkipEmptyParts );
         int pos = 0;
         if ( qsl.size() == 11 )
         {
            current_saxs.saxs_name = qsl[ pos++ ];
            if ( saxs_map.count( current_saxs.saxs_name ) )
            {
               current_saxs = saxs_map[ current_saxs.saxs_name ];
            }
            for ( int j = 0; j < 4; j++ )
            {
               current_saxs.a[ j ] = qsl[ pos++ ].toFloat();
               current_saxs.b[ j ] = qsl[ pos++ ].toFloat();
            }
            current_saxs.c      = qsl[ pos++ ].toFloat();
            current_saxs.volume = qsl[ pos++ ].toFloat();
            saxs_map[ current_saxs.saxs_name ] = current_saxs;
            continue;
         } 

         if ( qsl.size() == 13 )
         {
            current_saxs.saxs_name = qsl[ pos++ ];
            if ( saxs_map.count( current_saxs.saxs_name ) )
            {
               current_saxs = saxs_map[ current_saxs.saxs_name ];
            }
            for ( int j = 0; j < 5; j++ )
            {
               current_saxs.a5[ j ] = qsl[ pos++ ].toFloat();
               current_saxs.b5[ j ] = qsl[ pos++ ].toFloat();
            }
            current_saxs.c5     = qsl[ pos++ ].toFloat();
            current_saxs.volume = qsl[ pos++ ].toFloat();
            saxs_map[ current_saxs.saxs_name ] = current_saxs;
            continue;
         } 

         editor_msg( "red", 
                     QString( us_tr( "Warning: %1 on line %2, invalid number of tokens, ignoring" ) )
                     .arg( filename )
                     .arg( line ) );
      }
      
      our_saxs_options->dummy_saxs_names.clear( );
      for ( map < QString, saxs >::iterator it = saxs_map.begin();
            it != saxs_map.end();
            it++ )
      {
         it->second.si = 
            it->second.c +
            it->second.a[ 0 ] +
            it->second.a[ 1 ] +
            it->second.a[ 2 ] +
            it->second.a[ 3 ];
         it->second.si *= it->second.si;
         saxs_list.push_back( it->second );
         our_saxs_options->dummy_saxs_names.push_back( it->first );
      }
      f.close();
      for ( map < QString, saxs >::iterator it = ((US_Hydrodyn *)us_hydrodyn)->extra_saxs_coefficients.begin();
            it != ((US_Hydrodyn *)us_hydrodyn)->extra_saxs_coefficients.end();
            it++ )
      {
         if ( saxs_map.count( it->first ) )
         {
            editor_msg( "dark red", 
                        QString( us_tr( "NOTICE: saxs coefficients for %1 replaced by newly loaded values\n" ) )
                        .arg( it->first ) );
         } else {
            saxs_list.push_back( it->second );
            editor_msg( "dark blue", 
                        QString( us_tr( "NOTICE: added coefficients for %1 from newly loaded values\n" ) )
                        .arg( it->first ) );
         }
         saxs_map[ it->first ] = it->second;
      } 

      if ( !saxs_map.count( our_saxs_options->dummy_saxs_name ) )
      {
         if ( our_saxs_options->dummy_saxs_names.size() )
         {
            editor_msg( "red", QString( us_tr( "WARNING: default dummy atom name %1 was not defined in %2."
                                            " Set now to %3" ) )
                        .arg( our_saxs_options->dummy_saxs_name )
                        .arg( filename )
                        .arg( our_saxs_options->dummy_saxs_names.last() ) );
            our_saxs_options->dummy_saxs_name = our_saxs_options->dummy_saxs_names.last();
         } else {
            editor_msg( "red", QString( us_tr( "ERROR: no saxs structure factors found in %1." ) )
                        .arg( filename ) );
         }
      }
   }

   // setup for ff calc of hybridizations
   // later push this out to config file

   hybrid_coords.clear( );
   point p;

   // #define UHS_ALEXANDROS_POINTS

#if defined( UHS_ALEXANDROS_POINTS )

   p.axis[ 0 ] = 0.0;
   p.axis[ 1 ] = 0.0;
   p.axis[ 2 ] = 0.0;
   hybrid_coords[ "C" ].push_back( p );

   p.axis[ 0 ] = 0.005;
   p.axis[ 1 ] = 1.012;
   p.axis[ 2 ] = 0.341;
   hybrid_coords[ "C" ].push_back( p );

   p.axis[ 0 ] = 1.032;
   p.axis[ 1 ] = -0.268;
   p.axis[ 2 ] = -0.099;
   hybrid_coords[ "C" ].push_back( p );

   p.axis[ 0 ] = -0.469;
   p.axis[ 1 ] = -0.652;
   p.axis[ 2 ] = 0.706;
   hybrid_coords[ "C" ].push_back( p );

   p.axis[ 0 ] = 0.000;
   p.axis[ 1 ] = 0.067;
   p.axis[ 2 ] = 0.000;
   hybrid_coords[ "N" ].push_back( p );

   p.axis[ 0 ] = 0.438;
   p.axis[ 1 ] = -0.312;
   p.axis[ 2 ] = 0.825;
   hybrid_coords[ "N" ].push_back( p );

   p.axis[ 0 ] = 0.495;
   p.axis[ 1 ] = -0.312;
   p.axis[ 2 ] = -0.792;
   hybrid_coords[ "N" ].push_back( p );

   p.axis[ 0 ] = -0.934;
   p.axis[ 1 ] = -0.312;
   p.axis[ 2 ] = -0.033;
   hybrid_coords[ "N" ].push_back( p );

   p.axis[ 0 ] = 0.000;
   p.axis[ 1 ] = 0.000;
   p.axis[ 2 ] = 0.000;
   hybrid_coords[ "O" ].push_back( p );

   p.axis[ 0 ] = 0.000;
   p.axis[ 1 ] = 0.000;
   p.axis[ 2 ] = 0.960;
   hybrid_coords[ "O" ].push_back( p );

   p.axis[ 0 ] = 0.000;
   p.axis[ 1 ] = 0.000;
   p.axis[ 2 ] = 0.000;
   hybrid_coords[ "S" ].push_back( p );

   p.axis[ 0 ] = 0.000;
   p.axis[ 1 ] = 0.000;
   p.axis[ 2 ] = 1.340;
   hybrid_coords[ "S" ].push_back( p );
#else
   p.axis[ 0 ] = 0.0;
   p.axis[ 1 ] = 0.0;
   p.axis[ 2 ] = 0.0;
   hybrid_coords[ "C" ].push_back( p );

   p.axis[ 0 ] = 1.070f;
   p.axis[ 1 ] = 0.0f;
   p.axis[ 2 ] = 0.0f;
   hybrid_coords[ "C" ].push_back( p );

   p.axis[ 0 ] = -0.366f;
   p.axis[ 1 ] = 1.039f;
   p.axis[ 2 ] = 0.000f;
   hybrid_coords[ "C" ].push_back( p );

   p.axis[ 0 ] = -0.222f;
   p.axis[ 1 ] = -0.722f;
   p.axis[ 2 ] = -0.724f;
   hybrid_coords[ "C" ].push_back( p );

   p.axis[ 0 ] = 0.000f;
   p.axis[ 1 ] = 0.000f;
   p.axis[ 2 ] = 0.000f;
   hybrid_coords[ "N" ].push_back( p );

   p.axis[ 0 ] = 1.000f;
   p.axis[ 1 ] = 0.0f;
   p.axis[ 2 ] = 0.0f;
   hybrid_coords[ "N" ].push_back( p );

   p.axis[ 0 ] = -0.531f;
   p.axis[ 1 ] = 0.868f;
   p.axis[ 2 ] = 0.000f;
   hybrid_coords[ "N" ].push_back( p );

   p.axis[ 0 ] = -0.314f;
   p.axis[ 1 ] = 1.039f;
   p.axis[ 2 ] = -0.414f;
   hybrid_coords[ "N" ].push_back( p );

   p.axis[ 0 ] = 0.000f;
   p.axis[ 1 ] = 0.000f;
   p.axis[ 2 ] = 0.000f;
   hybrid_coords[ "O" ].push_back( p );

   p.axis[ 0 ] = 0.960f;
   p.axis[ 1 ] = 0.000f;
   p.axis[ 2 ] = 0.00f;
   hybrid_coords[ "O" ].push_back( p );

   p.axis[ 0 ] = -0.240f;
   p.axis[ 1 ] = 0.929f;
   p.axis[ 2 ] = 0.00f;
   hybrid_coords[ "O" ].push_back( p );

   p.axis[ 0 ] = 0.000f;
   p.axis[ 1 ] = 0.000f;
   p.axis[ 2 ] = 0.000f;
   hybrid_coords[ "S" ].push_back( p );

   p.axis[ 0 ] = 1.000f;
   p.axis[ 1 ] = 0.000f;
   p.axis[ 2 ] = 0.000f;
   hybrid_coords[ "S" ].push_back( p );
#endif
   // compute pairwise distances
   // just duplicate for ionizations for now
   {
      vector < QString > tmp;
      
      for ( auto it = hybrid_coords.begin();
            it != hybrid_coords.end();
            ++it ) {
         tmp.push_back( it->first );
      }
      for ( int j = 0; j < (int) tmp.size(); ++j ) {
         QString atomname = tmp[j];
         for ( int i = -2; i <= 3; ++i ) {
            
            if ( i < 0 ) {
               hybrid_coords[ QString( "%1%2" ).arg( atomname ).arg( i ) ] = hybrid_coords[ atomname ];
            } else if ( i > 0 ) {
               hybrid_coords[ QString( "%1+%2" ).arg( atomname ).arg( i ) ] = hybrid_coords[ atomname ];
            }
         }
      }
      for ( auto it = hybrid_coords.begin();
            it != hybrid_coords.end();
            ++it ) {
         QTextStream( stdout ) << "hybrid coord key " << it->first << Qt::endl;
      }
   }               

   hybrid_r.clear( );

   for ( map < QString, vector < point > >::iterator it = hybrid_coords.begin();
         it != hybrid_coords.end();
         it++ )
   {
      for ( unsigned int i = 0; i < ( unsigned int ) it->second.size(); i++ )
      {
         for ( unsigned int j = 0; j < ( unsigned int ) it->second.size(); j++ )
         {
            hybrid_r[ it->first ][ i ][ j ] =
               sqrt( 
                    ( it->second[ i ].axis[ 0 ] - it->second[ j ].axis[ 0 ] ) *
                    ( it->second[ i ].axis[ 0 ] - it->second[ j ].axis[ 0 ] ) 
                    +
                    ( it->second[ i ].axis[ 1 ] - it->second[ j ].axis[ 1 ] ) *
                    ( it->second[ i ].axis[ 1 ] - it->second[ j ].axis[ 1 ] ) 
                    +
                    ( it->second[ i ].axis[ 2 ] - it->second[ j ].axis[ 2 ] ) *
                    ( it->second[ i ].axis[ 2 ] - it->second[ j ].axis[ 2 ] ) 
                    );
         }
      }                    
   }
}

QString US_Hydrodyn_Saxs::saxs_filestring()
{
   QString result = 
      QString("%1").arg(te_filename2->text()) +
      QString("_%1").arg( model_vector[ current_model ].model_id ) + 
      iqq_suffix() +
      ".ssaxs";
   return result;
}

QString US_Hydrodyn_Saxs::sprr_filestring( const QString & append )
{
   QString result = 
      QString("%1_%2b%3")
      .arg(te_filename2->text())
      .arg( model_vector[ current_model ].model_id )
      .arg(our_saxs_options->bin_size)
      .replace( ".", "_" );

   if ( our_saxs_options->smooth ) {
      result += QString( "_sm%1" ).arg( our_saxs_options->smooth ).replace( ".", "_" );
   }

   result += append;

   if ( rb_curve_sans->isChecked() )
   {
      result += 
         QString("D%1")
         .arg(our_saxs_options->d2o_conc * 100);
   }

   result +=
      QString(".sprr_%1")
      .arg( ((US_Hydrodyn *)us_hydrodyn)->saxs_sans_ext() );
   // rb_curve_raw->isChecked() ? "r" :
   //            ( rb_curve_saxs->isChecked() ? "x" : "n" ) );
   return result;
}

QString US_Hydrodyn_Saxs::vector_double_to_csv( vector < double > vd )
{
   QString result;
   for ( unsigned int i = 0; i < vd.size(); i++ )
   {
      result += QString("%1,").arg(vd[i]);
   }
   return result;
}

double US_Hydrodyn_Saxs::compute_pr_area( vector < double > vd, vector < double > r )
{
   double sum = 0e0;
   for ( unsigned int i = 0; i < vd.size(); i++ )
   {
      sum += vd[i];
   }

   // assuming constant delta!
   double delta = 0e0;
   if ( r.size() > 1 )
   {
      delta = r[1] - r[0];
   }
   if ( delta == 0e0 )
   {
      cerr << QString("compute_pr_area zero delta! sum %1\nvd: %2\nr: %3\n")
         .arg(sum)
         .arg(vector_double_to_csv(vd))
         .arg(vector_double_to_csv(r))
         ;
   }
   return sum * delta;
}

void US_Hydrodyn_Saxs::saxs_search()
{
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_search_widget )
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_search_window->isVisible() )
      {
         ((US_Hydrodyn *)us_hydrodyn)->saxs_search_window->raise();
      }
      else
      {
         ((US_Hydrodyn *)us_hydrodyn)->saxs_search_window->show();
      }
   }
   else
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->last_saxs_search_csv.name != "__empty__" )
      {
         search_csv = ((US_Hydrodyn *)us_hydrodyn)->last_saxs_search_csv;
      } 
      ((US_Hydrodyn *)us_hydrodyn)->saxs_search_window = new US_Hydrodyn_Saxs_Search( search_csv, us_hydrodyn );
      US_Hydrodyn::fixWinButtons( ((US_Hydrodyn *)us_hydrodyn)->saxs_search_window );
      ((US_Hydrodyn *)us_hydrodyn)->saxs_search_window->show();
   }
}

void US_Hydrodyn_Saxs::reset_search_csv()
{
   if ( ((US_Hydrodyn *)us_hydrodyn)->last_saxs_search_csv.name != "__empty__" )
   {
      search_csv = ((US_Hydrodyn *)us_hydrodyn)->last_saxs_search_csv;
      return;
   } 

   search_csv.name = "SAXS I(q) Search";

   search_csv.header.clear( );
   search_csv.header_map.clear( );
   search_csv.data.clear( );
   search_csv.num_data.clear( );
   search_csv.prepended_names.clear( );

   search_csv.header.push_back("Parameter");
   search_csv.header.push_back("Active");
   search_csv.header.push_back("Low value");
   search_csv.header.push_back("High value");
   search_csv.header.push_back("Points");
   search_csv.header.push_back("Interval");
   search_csv.header.push_back("Current value");
   search_csv.header.push_back("Best value");

   vector < QString > tmp_data;
   
   tmp_data.push_back("Buffer electron density");
   tmp_data.push_back("N");
   tmp_data.push_back("0.1");
   tmp_data.push_back("0.9");
   tmp_data.push_back("11");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   search_csv.prepended_names.push_back(tmp_data[0]);
   search_csv.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("Scaling excluded volume");
   tmp_data.push_back("N");
   tmp_data.push_back(".95");
   tmp_data.push_back("1.05");
   tmp_data.push_back("11");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   search_csv.prepended_names.push_back(tmp_data[0]);
   search_csv.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("WAT excluded volume");
   tmp_data.push_back("N");
   tmp_data.push_back("15");
   tmp_data.push_back("30");
   tmp_data.push_back("11");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   search_csv.prepended_names.push_back(tmp_data[0]);
   search_csv.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("Crysol: average atomic radius");
   tmp_data.push_back("N");
   tmp_data.push_back("1.5");
   tmp_data.push_back("1.7");
   tmp_data.push_back("11");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   search_csv.prepended_names.push_back(tmp_data[0]);
   search_csv.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("Crysol: Excluded volume");
   tmp_data.push_back("N");
   tmp_data.push_back("10000");
   tmp_data.push_back("20000");
   tmp_data.push_back("11");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   search_csv.prepended_names.push_back(tmp_data[0]);
   search_csv.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("Crysol: contrast of hydration shell");
   tmp_data.push_back("N");
   tmp_data.push_back("0.01");
   tmp_data.push_back("0.05");
   tmp_data.push_back("11");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   search_csv.prepended_names.push_back(tmp_data[0]);
   search_csv.data.push_back(tmp_data);

   for ( unsigned int i = 0; i < search_csv.data.size(); i++ )
   {
      vector < double > tmp_num_data;
      for ( unsigned int j = 0; j < search_csv.data[i].size(); j++ )
      {
         tmp_num_data.push_back(search_csv.data[i][j].toDouble());
      }
      search_csv.num_data.push_back(tmp_num_data);
   }
}

void US_Hydrodyn_Saxs::saxs_screen()
{
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_screen_widget )
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_screen_window->isVisible() )
      {
         ((US_Hydrodyn *)us_hydrodyn)->saxs_screen_window->raise();
      }
      else
      {
         ((US_Hydrodyn *)us_hydrodyn)->saxs_screen_window->show();
      }
   }
   else
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->last_saxs_screen_csv.name != "__empty__" )
      {
         screen_csv = ((US_Hydrodyn *)us_hydrodyn)->last_saxs_screen_csv;
      } 
      ((US_Hydrodyn *)us_hydrodyn)->saxs_screen_window = new US_Hydrodyn_Saxs_Screen( screen_csv, us_hydrodyn );
      US_Hydrodyn::fixWinButtons( ((US_Hydrodyn *)us_hydrodyn)->saxs_screen_window );
      ((US_Hydrodyn *)us_hydrodyn)->saxs_screen_window->show();
   }
}

void US_Hydrodyn_Saxs::reset_screen_csv()
{
   if ( ((US_Hydrodyn *)us_hydrodyn)->last_saxs_screen_csv.name != "__empty__" )
   {
      screen_csv = ((US_Hydrodyn *)us_hydrodyn)->last_saxs_screen_csv;
      return;
   } 

   screen_csv.name = "SAXS I(q) Screen";

   screen_csv.header.clear( );
   screen_csv.header_map.clear( );
   screen_csv.data.clear( );
   screen_csv.num_data.clear( );
   screen_csv.prepended_names.clear( );

   screen_csv.header.push_back("Parameter");
   screen_csv.header.push_back("Active");
   screen_csv.header.push_back("Low value");
   screen_csv.header.push_back("High value");
   screen_csv.header.push_back("Points");
   screen_csv.header.push_back("Interval");

   vector < QString > tmp_data;
   
   tmp_data.push_back("Radius (A)");
   tmp_data.push_back("Y");
   tmp_data.push_back(".5");
#if QT_VERSION < 0x040000
   tmp_data.push_back("1000");
   tmp_data.push_back("1001");
#else
   tmp_data.push_back("100");
   tmp_data.push_back("101");
#endif
   tmp_data.push_back("");

   screen_csv.prepended_names.push_back(tmp_data[0]);
   screen_csv.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("Delta rho (A^-3)");
   tmp_data.push_back("N");
   tmp_data.push_back(".3");
   tmp_data.push_back(".8");
   tmp_data.push_back("101");
   tmp_data.push_back("");

   screen_csv.prepended_names.push_back(tmp_data[0]);
   screen_csv.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("Ending q (A^-1)");
   tmp_data.push_back("N");
   tmp_data.push_back(".1");
   tmp_data.push_back("1.0");
   tmp_data.push_back("101");
   tmp_data.push_back("");

   screen_csv.prepended_names.push_back(tmp_data[0]);
   screen_csv.data.push_back(tmp_data);

   for ( unsigned int i = 0; i < screen_csv.data.size(); i++ )
   {
      vector < double > tmp_num_data;
      for ( unsigned int j = 0; j < screen_csv.data[i].size(); j++ )
      {
         tmp_num_data.push_back(screen_csv.data[i][j].toDouble());
      }
      screen_csv.num_data.push_back(tmp_num_data);
   }
}

void US_Hydrodyn_Saxs::saxs_buffer()
{
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_buffer_widget )
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_buffer_window->isVisible() )
      {
         ((US_Hydrodyn *)us_hydrodyn)->saxs_buffer_window->raise();
      }
      else
      {
         ((US_Hydrodyn *)us_hydrodyn)->saxs_buffer_window->show();
      }
   }
   else
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->last_saxs_buffer_csv.name != "__empty__" )
      {
         buffer_csv = ((US_Hydrodyn *)us_hydrodyn)->last_saxs_buffer_csv;
      } 
      ((US_Hydrodyn *)us_hydrodyn)->saxs_buffer_window = new US_Hydrodyn_Saxs_Buffer( buffer_csv, us_hydrodyn );
      US_Hydrodyn::fixWinButtons( ((US_Hydrodyn *)us_hydrodyn)->saxs_buffer_window );
      ((US_Hydrodyn *)us_hydrodyn)->saxs_buffer_window->show();
   }

   for ( unsigned int i = 0; i < plotted_q.size(); i++ )
   {
      if ( plotted_I_error[ i ].size() == plotted_I[ i ].size() )
      {
         ((US_Hydrodyn *)us_hydrodyn)->saxs_buffer_window->add_plot( qsl_plotted_iq_names[ i ],
                                                                     plotted_q[ i ],
                                                                     plotted_I[ i ],
                                                                     plotted_I_error[ i ] );
      } else {
         ((US_Hydrodyn *)us_hydrodyn)->saxs_buffer_window->add_plot( qsl_plotted_iq_names[ i ],
                                                                     plotted_q[ i ],
                                                                     plotted_I[ i ] );
      }
   }
}

void US_Hydrodyn_Saxs::saxs_hplc()
{
   // qDebug() << "saxs_hplc";
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_hplc_widget )
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_hplc_window->isVisible() )
      {
         ((US_Hydrodyn *)us_hydrodyn)->saxs_hplc_window->raise();
      }
      else
      {
         ((US_Hydrodyn *)us_hydrodyn)->saxs_hplc_window->show();
      }
   }
   else
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->last_saxs_hplc_csv.name != "__empty__" )
      {
         hplc_csv = ((US_Hydrodyn *)us_hydrodyn)->last_saxs_hplc_csv;
      } 
      ((US_Hydrodyn *)us_hydrodyn)->saxs_hplc_window = new US_Hydrodyn_Saxs_Hplc( hplc_csv, us_hydrodyn );
      US_Hydrodyn::fixWinButtons( ((US_Hydrodyn *)us_hydrodyn)->saxs_hplc_window );
      ((US_Hydrodyn *)us_hydrodyn)->saxs_hplc_window->show();
   }

   ((US_Hydrodyn *)us_hydrodyn)->saxs_hplc_window->disable_all();
   for ( unsigned int i = 0; i < plotted_q.size(); i++ )
   {
      if ( !( i % 500 ) ) {
         qDebug() << "saxs_hplc plotting curve " << i;
      }
      if ( plotted_I_error[ i ].size() == plotted_I[ i ].size() )
      {
         ((US_Hydrodyn *)us_hydrodyn)->saxs_hplc_window->add_plot( qsl_plotted_iq_names[ i ],
                                                                   plotted_q[ i ],
                                                                   plotted_I[ i ],
                                                                   plotted_I_error[ i ] );
      } else {
         ((US_Hydrodyn *)us_hydrodyn)->saxs_hplc_window->add_plot( qsl_plotted_iq_names[ i ],
                                                                   plotted_q[ i ],
                                                                   plotted_I[ i ] );
      }
   }
   ((US_Hydrodyn *)us_hydrodyn)->saxs_hplc_window->update_enables();
}

void US_Hydrodyn_Saxs::dad() {
   qDebug() << "dad";
   if ( ((US_Hydrodyn *)us_hydrodyn)->dad_widget )
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->dad_window->isVisible() )
      {
         ((US_Hydrodyn *)us_hydrodyn)->dad_window->raise();
      }
      else
      {
         ((US_Hydrodyn *)us_hydrodyn)->dad_window->show();
      }
   }
   else
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->last_dad_csv.name != "__empty__" )
      {
         hplc_csv = ((US_Hydrodyn *)us_hydrodyn)->last_dad_csv;
      } 
      ((US_Hydrodyn *)us_hydrodyn)->dad_window = new US_Hydrodyn_Dad( hplc_csv, us_hydrodyn );
      US_Hydrodyn::fixWinButtons( ((US_Hydrodyn *)us_hydrodyn)->dad_window );
      ((US_Hydrodyn *)us_hydrodyn)->dad_window->show();
   }
}

void US_Hydrodyn_Saxs::mals()
{
   // qDebug() << "mals";
   if ( ((US_Hydrodyn *)us_hydrodyn)->mals_widget )
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->mals_window->isVisible() )
      {
         ((US_Hydrodyn *)us_hydrodyn)->mals_window->raise();
      }
      else
      {
         ((US_Hydrodyn *)us_hydrodyn)->mals_window->show();
      }
   }
   else
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->last_mals_csv.name != "__empty__" )
      {
         hplc_csv = ((US_Hydrodyn *)us_hydrodyn)->last_mals_csv;
      } 
      ((US_Hydrodyn *)us_hydrodyn)->mals_window = new US_Hydrodyn_Mals( hplc_csv, us_hydrodyn );
      US_Hydrodyn::fixWinButtons( ((US_Hydrodyn *)us_hydrodyn)->mals_window );
      ((US_Hydrodyn *)us_hydrodyn)->mals_window->show();
   }
}

void US_Hydrodyn_Saxs::mals_saxs()
{
   // qDebug() << "mals_saxs";
   if ( ((US_Hydrodyn *)us_hydrodyn)->mals_saxs_widget )
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->mals_saxs_window->isVisible() )
      {
         ((US_Hydrodyn *)us_hydrodyn)->mals_saxs_window->raise();
      }
      else
      {
         ((US_Hydrodyn *)us_hydrodyn)->mals_saxs_window->show();
      }
   }
   else
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->last_mals_saxs_csv.name != "__empty__" )
      {
         hplc_csv = ((US_Hydrodyn *)us_hydrodyn)->last_mals_saxs_csv;
      } 
      ((US_Hydrodyn *)us_hydrodyn)->mals_saxs_window = new US_Hydrodyn_Mals_Saxs( hplc_csv, us_hydrodyn );
      US_Hydrodyn::fixWinButtons( ((US_Hydrodyn *)us_hydrodyn)->mals_saxs_window );
      ((US_Hydrodyn *)us_hydrodyn)->mals_saxs_window->show();
   }
}


void US_Hydrodyn_Saxs::reset_buffer_csv()
{
   if ( ((US_Hydrodyn *)us_hydrodyn)->last_saxs_buffer_csv.name != "__empty__" )
   {
      buffer_csv = ((US_Hydrodyn *)us_hydrodyn)->last_saxs_buffer_csv;
      return;
   } 

   buffer_csv.name = "SAXS I(q) Buffer";

   buffer_csv.header.clear( );
   buffer_csv.header_map.clear( );
   buffer_csv.data.clear( );
   buffer_csv.num_data.clear( );
   buffer_csv.prepended_names.clear( );

   buffer_csv.header.push_back("Parameter");
   buffer_csv.header.push_back("Active");
   buffer_csv.header.push_back("Low value");
   buffer_csv.header.push_back("High value");
   buffer_csv.header.push_back("Points");
   buffer_csv.header.push_back("Interval");
   buffer_csv.header.push_back("Current value");
   buffer_csv.header.push_back("Best value");

   vector < QString > tmp_data;
   
   tmp_data.clear( );
   tmp_data.push_back("Alpha (I=Isol-Alpha*Ibuf-(1-Alpha)*Iblank)");
   tmp_data.push_back("Y");
   tmp_data.push_back("0.95");
   tmp_data.push_back("1");
   tmp_data.push_back("51");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   buffer_csv.prepended_names.push_back(tmp_data[0]);
   buffer_csv.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("PSV");
   tmp_data.push_back("N");
   tmp_data.push_back("0.5");
   tmp_data.push_back("0.8");
   tmp_data.push_back("51");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   buffer_csv.prepended_names.push_back(tmp_data[0]);
   buffer_csv.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("Gamma (Alpha=1-Gamma*Conc*PSV/1000)");
   tmp_data.push_back("N");
   tmp_data.push_back("0.95");
   tmp_data.push_back("1.05");
   tmp_data.push_back("51");
   tmp_data.push_back("");
   tmp_data.push_back("1");
   tmp_data.push_back("");

   buffer_csv.prepended_names.push_back(tmp_data[0]);
   buffer_csv.data.push_back(tmp_data);

   for ( unsigned int i = 0; i < buffer_csv.data.size(); i++ )
   {
      vector < double > tmp_num_data;
      for ( unsigned int j = 0; j < buffer_csv.data[i].size(); j++ )
      {
         tmp_num_data.push_back(buffer_csv.data[i][j].toDouble());
      }
      buffer_csv.num_data.push_back(tmp_num_data);
   }
}

void US_Hydrodyn_Saxs::reset_hplc_csv()
{
   if ( ((US_Hydrodyn *)us_hydrodyn)->last_saxs_hplc_csv.name != "__empty__" )
   {
      hplc_csv = ((US_Hydrodyn *)us_hydrodyn)->last_saxs_hplc_csv;
      return;
   } 

   hplc_csv.name = "SAXS I(q) Hplc";

   hplc_csv.header.clear( );
   hplc_csv.header_map.clear( );
   hplc_csv.data.clear( );
   hplc_csv.num_data.clear( );
   hplc_csv.prepended_names.clear( );

   hplc_csv.header.push_back("Parameter");
   hplc_csv.header.push_back("Active");
   hplc_csv.header.push_back("Low value");
   hplc_csv.header.push_back("High value");
   hplc_csv.header.push_back("Points");
   hplc_csv.header.push_back("Interval");
   hplc_csv.header.push_back("Current value");
   hplc_csv.header.push_back("Best value");

   vector < QString > tmp_data;
   
   tmp_data.clear( );
   tmp_data.push_back("Alpha (I=Isol-Alpha*Ibuf-(1-Alpha)*Iblank)");
   tmp_data.push_back("Y");
   tmp_data.push_back("0.95");
   tmp_data.push_back("1");
   tmp_data.push_back("51");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   hplc_csv.prepended_names.push_back(tmp_data[0]);
   hplc_csv.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("PSV");
   tmp_data.push_back("N");
   tmp_data.push_back("0.5");
   tmp_data.push_back("0.8");
   tmp_data.push_back("51");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   hplc_csv.prepended_names.push_back(tmp_data[0]);
   hplc_csv.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("Gamma (Alpha=1-Gamma*Conc*PSV/1000)");
   tmp_data.push_back("N");
   tmp_data.push_back("0.95");
   tmp_data.push_back("1.05");
   tmp_data.push_back("51");
   tmp_data.push_back("");
   tmp_data.push_back("1");
   tmp_data.push_back("");

   hplc_csv.prepended_names.push_back(tmp_data[0]);
   hplc_csv.data.push_back(tmp_data);

   for ( unsigned int i = 0; i < hplc_csv.data.size(); i++ )
   {
      vector < double > tmp_num_data;
      for ( unsigned int j = 0; j < hplc_csv.data[i].size(); j++ )
      {
         tmp_num_data.push_back(hplc_csv.data[i][j].toDouble());
      }
      hplc_csv.num_data.push_back(tmp_num_data);
   }
}

void US_Hydrodyn_Saxs::reset_dad_csv()
{
   if ( ((US_Hydrodyn *)us_hydrodyn)->last_dad_csv.name != "__empty__" )
   {
      hplc_csv = ((US_Hydrodyn *)us_hydrodyn)->last_dad_csv;
      return;
   } 

   dad_csv.name = "SAXS I(q) UV-Vis";

   dad_csv.header.clear( );
   dad_csv.header_map.clear( );
   dad_csv.data.clear( );
   dad_csv.num_data.clear( );
   dad_csv.prepended_names.clear( );

   dad_csv.header.push_back("Parameter");
   dad_csv.header.push_back("Active");
   dad_csv.header.push_back("Low value");
   dad_csv.header.push_back("High value");
   dad_csv.header.push_back("Points");
   dad_csv.header.push_back("Interval");
   dad_csv.header.push_back("Current value");
   dad_csv.header.push_back("Best value");

   vector < QString > tmp_data;
   
   tmp_data.clear( );
   tmp_data.push_back("Alpha (I=Isol-Alpha*Ibuf-(1-Alpha)*Iblank)");
   tmp_data.push_back("Y");
   tmp_data.push_back("0.95");
   tmp_data.push_back("1");
   tmp_data.push_back("51");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   dad_csv.prepended_names.push_back(tmp_data[0]);
   dad_csv.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("PSV");
   tmp_data.push_back("N");
   tmp_data.push_back("0.5");
   tmp_data.push_back("0.8");
   tmp_data.push_back("51");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   dad_csv.prepended_names.push_back(tmp_data[0]);
   dad_csv.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("Gamma (Alpha=1-Gamma*Conc*PSV/1000)");
   tmp_data.push_back("N");
   tmp_data.push_back("0.95");
   tmp_data.push_back("1.05");
   tmp_data.push_back("51");
   tmp_data.push_back("");
   tmp_data.push_back("1");
   tmp_data.push_back("");

   dad_csv.prepended_names.push_back(tmp_data[0]);
   dad_csv.data.push_back(tmp_data);

   for ( unsigned int i = 0; i < dad_csv.data.size(); i++ )
   {
      vector < double > tmp_num_data;
      for ( unsigned int j = 0; j < dad_csv.data[i].size(); j++ )
      {
         tmp_num_data.push_back(dad_csv.data[i][j].toDouble());
      }
      dad_csv.num_data.push_back(tmp_num_data);
   }
}

void US_Hydrodyn_Saxs::reset_mals_csv()
{
   if ( ((US_Hydrodyn *)us_hydrodyn)->last_mals_csv.name != "__empty__" )
   {
      hplc_csv = ((US_Hydrodyn *)us_hydrodyn)->last_mals_csv;
      return;
   } 

   mals_csv.name = "SAXS I(q) MALS";

   mals_csv.header.clear( );
   mals_csv.header_map.clear( );
   mals_csv.data.clear( );
   mals_csv.num_data.clear( );
   mals_csv.prepended_names.clear( );

   mals_csv.header.push_back("Parameter");
   mals_csv.header.push_back("Active");
   mals_csv.header.push_back("Low value");
   mals_csv.header.push_back("High value");
   mals_csv.header.push_back("Points");
   mals_csv.header.push_back("Interval");
   mals_csv.header.push_back("Current value");
   mals_csv.header.push_back("Best value");

   vector < QString > tmp_data;
   
   tmp_data.clear( );
   tmp_data.push_back("Alpha (I=Isol-Alpha*Ibuf-(1-Alpha)*Iblank)");
   tmp_data.push_back("Y");
   tmp_data.push_back("0.95");
   tmp_data.push_back("1");
   tmp_data.push_back("51");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   mals_csv.prepended_names.push_back(tmp_data[0]);
   mals_csv.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("PSV");
   tmp_data.push_back("N");
   tmp_data.push_back("0.5");
   tmp_data.push_back("0.8");
   tmp_data.push_back("51");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   mals_csv.prepended_names.push_back(tmp_data[0]);
   mals_csv.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("Gamma (Alpha=1-Gamma*Conc*PSV/1000)");
   tmp_data.push_back("N");
   tmp_data.push_back("0.95");
   tmp_data.push_back("1.05");
   tmp_data.push_back("51");
   tmp_data.push_back("");
   tmp_data.push_back("1");
   tmp_data.push_back("");

   mals_csv.prepended_names.push_back(tmp_data[0]);
   mals_csv.data.push_back(tmp_data);

   for ( unsigned int i = 0; i < mals_csv.data.size(); i++ )
   {
      vector < double > tmp_num_data;
      for ( unsigned int j = 0; j < mals_csv.data[i].size(); j++ )
      {
         tmp_num_data.push_back(mals_csv.data[i][j].toDouble());
      }
      mals_csv.num_data.push_back(tmp_num_data);
   }
}

void US_Hydrodyn_Saxs::reset_mals_saxs_csv()
{
   if ( ((US_Hydrodyn *)us_hydrodyn)->last_mals_saxs_csv.name != "__empty__" )
   {
      hplc_csv = ((US_Hydrodyn *)us_hydrodyn)->last_mals_saxs_csv;
      return;
   } 

   mals_saxs_csv.name = "SAXS I(q) MALS+SAXS";

   mals_saxs_csv.header.clear( );
   mals_saxs_csv.header_map.clear( );
   mals_saxs_csv.data.clear( );
   mals_saxs_csv.num_data.clear( );
   mals_saxs_csv.prepended_names.clear( );

   mals_saxs_csv.header.push_back("Parameter");
   mals_saxs_csv.header.push_back("Active");
   mals_saxs_csv.header.push_back("Low value");
   mals_saxs_csv.header.push_back("High value");
   mals_saxs_csv.header.push_back("Points");
   mals_saxs_csv.header.push_back("Interval");
   mals_saxs_csv.header.push_back("Current value");
   mals_saxs_csv.header.push_back("Best value");

   vector < QString > tmp_data;
   
   tmp_data.clear( );
   tmp_data.push_back("Alpha (I=Isol-Alpha*Ibuf-(1-Alpha)*Iblank)");
   tmp_data.push_back("Y");
   tmp_data.push_back("0.95");
   tmp_data.push_back("1");
   tmp_data.push_back("51");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   mals_saxs_csv.prepended_names.push_back(tmp_data[0]);
   mals_saxs_csv.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("PSV");
   tmp_data.push_back("N");
   tmp_data.push_back("0.5");
   tmp_data.push_back("0.8");
   tmp_data.push_back("51");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   mals_saxs_csv.prepended_names.push_back(tmp_data[0]);
   mals_saxs_csv.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("Gamma (Alpha=1-Gamma*Conc*PSV/1000)");
   tmp_data.push_back("N");
   tmp_data.push_back("0.95");
   tmp_data.push_back("1.05");
   tmp_data.push_back("51");
   tmp_data.push_back("");
   tmp_data.push_back("1");
   tmp_data.push_back("");

   mals_saxs_csv.prepended_names.push_back(tmp_data[0]);
   mals_saxs_csv.data.push_back(tmp_data);

   for ( unsigned int i = 0; i < mals_saxs_csv.data.size(); i++ )
   {
      vector < double > tmp_num_data;
      for ( unsigned int j = 0; j < mals_saxs_csv.data[i].size(); j++ )
      {
         tmp_num_data.push_back(mals_saxs_csv.data[i][j].toDouble());
      }
      mals_saxs_csv.num_data.push_back(tmp_num_data);
   }
}

vector < double > US_Hydrodyn_Saxs::interpolate( vector < double > to_r, 
                                                 vector < double > from_r, 
                                                 vector < double > from_pr )
{
   vector < double > to_data;
   interpolate( to_r, from_r, from_pr, to_data );
   return to_data;
}

bool US_Hydrodyn_Saxs::interpolate( vector < double > to_r, 
                                    vector < double > from_r, 
                                    vector < double > from_pr,
                                    vector < double > &to_data )
{
   US_Saxs_Util usu;

   vector < double > new_from_r;
   vector < double > new_from_pr;
   new_from_r.push_back(-1);
   new_from_pr.push_back(0);

   for ( unsigned int i = 0; i < from_r.size(); i++ )
   {
      new_from_r.push_back(from_r[i]);
      new_from_pr.push_back(from_pr[i]);
   }
   new_from_r.push_back(1e99);
   new_from_pr.push_back(0);
   
   usu.wave["from"].q = new_from_r;
   usu.wave["from"].r = new_from_pr;
   usu.wave["from"].s = new_from_pr;
   usu.wave["to"].q = to_r;
   usu.wave["to"].r = to_r;

   if ( !usu.interpolate( "out", "to", "from" ) )
   {
      cout << usu.errormsg;
      return false;
   }
   to_data = usu.wave["out"].r;
   return true;
}

bool US_Hydrodyn_Saxs::natural_spline_interpolate( vector < double > to_grid, 
                                                   vector < double > from_grid, 
                                                   vector < double > from_data,
                                                   vector < double > &to_data )
{
   US_Saxs_Util usu;

   vector < double > y2;
   usu.natural_spline( from_grid, from_data, y2 );
   to_data.resize( to_grid.size() );

   for ( unsigned int i = 0; i < to_grid.size(); i++ )
   {
      if ( !usu.apply_natural_spline( from_grid, from_data, y2, to_grid[ i ], to_data[ i ] ) )
      {
         return false;
      }
   }
   
   return true;
}

vector < double > US_Hydrodyn_Saxs::rescale( vector < double > x )
{
   // rescales x to add up to 1
   double sum = 0e0;
   for ( unsigned int i = 0; i < x.size(); i++ )
   {
      sum += x[i];
   }
   if ( sum != 0 )
   {
      for ( unsigned int i = 0; i < x.size(); i++ )
      {
         x[i] /= sum;
      }
   }
   return x;
}

int US_Hydrodyn_Saxs::file_curve_type(QString filename)
{
   QRegExp rx("sprr_(.)");
   if ( rx.indexIn(filename) == -1 )
   {
      return -1;
   }
   if ( rx.cap(1) == "r" )
   {
      return 0;
   }
   if ( rx.cap(1) == "x" )
   {
      return 1;
   }
   if ( rx.cap(1) == "n" )
   {
      return 2;
   }
   return -1;
}

QString US_Hydrodyn_Saxs::curve_type_string(int curve)
{
   QString result;
   switch ( curve )
   {
   case 0 : 
      result = "\"Raw\"";
      break;
   case 1 : 
      result = "\"SAXS\"";
      break;
   case 2 : 
      result = "\"SANS\"";
      break;
   default : 
      result = "unknown";
      break;
   }
   return result;
}

void US_Hydrodyn_Saxs::rescale_plot()
{
   // qDebug() << "_Saxs::rescale_plot()";
   double lowq;
   double highq;
   double lowI;
   double highI;
   plot_domain( lowq, highq );
   plot_range ( lowq, highq, lowI, highI );

   /*
   cout << "rescale plot "
           << lowq << ":" << highq << "  "
           << lowI << ":" << highI << endl;
   */

   lowI = US_Plot_Util::round_digits( lowI );
   highI = US_Plot_Util::round_digits( highI );

   plot_saxs->setAxisScale( QwtPlot::xBottom, lowq, highq );
   plot_saxs->setAxisScale( QwtPlot::yLeft,   lowI, highI );

   // if ( plot_saxs_zoomer )
   // {
   //    delete plot_saxs_zoomer;
   // }
   // plot_saxs_zoomer = new ScrollZoomer(plot_saxs->canvas());
   // plot_saxs_zoomer->setRubberBandPen( QPen( Qt::red, 1, Qt::DotLine ) );
   // plot_saxs_zoomer->setTrackerPen( QPen( Qt::red ) );

   plot_saxs_zoomer->setZoomBase();
   plot_resid->setAxisScale( QwtPlot::xBottom, lowq, highq );
   plot_resid_zoomer->setZoomBase();

   plot_saxs->replot();
}

void US_Hydrodyn_Saxs::set_kratky()
{

   if ( cb_kratky->isChecked() &&
        cb_user_range->isChecked() )
   {
      cb_user_range->setChecked( false );
   }

   if ( cb_kratky->isChecked() &&
        cb_guinier->isChecked() )
   {
      cb_guinier->setChecked( false );
   }

   set_guinier();
}

void US_Hydrodyn_Saxs::set_user_range()
{
   cout << Iq_plotted_summary();

#if defined(TEST_GET_QUADRATIC_INTERPOLATION_COEFFICIENTS)
   // test get_quadratic_interpolation_coefficients
   {

      US_Saxs_Util usu;
      vector < double > x(3);
      vector < double > y(3);
      vector < double > c(3);

      for ( unsigned int i = 0; i < 3; i++ )
      {
         x[i] = i;
         y[i] = x[i] * x[i]; // simple y=x^2
      }
      if ( !usu.get_quadratic_interpolation_coefficients(x, y, c) )
      {
         cout << usu.errormsg << endl;
      } else {
         for ( unsigned int i = 0; i < 3; i++ )
         {
            cout << QString("c[%1] = %2 ").arg(i).arg(c[i]);
         }
         cout << endl;
      }
   }
#endif

   if ( cb_user_range->isChecked() &&
        ( cb_guinier->isChecked() ||
          cb_kratky->isChecked() ) )
   {
      cb_guinier->setChecked(false);
      cb_kratky ->setChecked(false);
      set_guinier();
   } else {
      plot_saxs->setAxisTitle  ( QwtPlot::yLeft,   
                                 cb_kratky ->isChecked() ?
                                 us_tr( " q^2 * I(q)"        ) : 
                                 ( cb_cs_guinier->isChecked() ?                                 
                                   us_tr( "q*I(q) (log scale)" ) : 
                                   ( cb_Rt_guinier->isChecked() ?
                                     us_tr( "q^2*I(q) (log scale)" ) :
                                     us_tr( "I(q) (log scale)" ) ) )
                                 );
      plot_saxs->setAxisScaleEngine(QwtPlot::yLeft, 
                                    cb_kratky->isChecked() ?
                                    new QwtLogScaleEngine(10) :  // fix this
                                    new QwtLogScaleEngine(10));

      rescale_plot();
   }
}
      
void US_Hydrodyn_Saxs::set_cs_guinier()
{
   if ( cb_cs_guinier->isChecked() )
   {
      disconnect(cb_Rt_guinier, SIGNAL(clicked()) );
      cb_Rt_guinier->setChecked( false );
      connect(cb_Rt_guinier, SIGNAL(clicked()), SLOT(set_Rt_guinier()));
   }
   set_guinier();
}

void US_Hydrodyn_Saxs::set_Rt_guinier()
{
   if ( cb_Rt_guinier->isChecked() )
   {
      disconnect(cb_cs_guinier, SIGNAL(clicked()) );
      cb_cs_guinier->setChecked( false );
      connect(cb_cs_guinier, SIGNAL(clicked()), SLOT(set_cs_guinier()));
   }
   set_guinier();
}

// sets lowq & highq based upon current, plot settings (could be q^2 if in guinier)
void US_Hydrodyn_Saxs::plot_domain( 
                                  double &lowq, 
                                  double &highq
                                  )
{
   if ( cb_guinier->isChecked() )
   {
      // arbitrary defaults

      double lowq2 = 1e-4;
      double highq2 = 1e-1;
      
      bool any_guinier = false;

      if ( cb_cs_guinier->isChecked() )
      {

         // first compute available guinier range
         // this could be calculated once upon adding curves

         for ( unsigned int i = 0; 
               i < plotted_Iq.size();
               i++ )
         {
            if ( plotted_cs_guinier_valid.count(i) &&
                 plotted_cs_guinier_valid[i] == true )
            {
               if ( !any_guinier )
               {
                  lowq2 = plotted_cs_guinier_lowq2[i];
                  highq2 = plotted_cs_guinier_highq2[i];
                  any_guinier = true;
               } else {
                  if ( lowq2 > plotted_cs_guinier_lowq2[i] )
                  {
                     lowq2 = plotted_cs_guinier_lowq2[i];
                  }
                  if ( highq2 < plotted_cs_guinier_highq2[i] )
                  {
                     highq2 = plotted_cs_guinier_highq2[i];
                  }
               }
            }
            if ( any_guinier )
            {
               lowq2 *= .75;
               highq2 *= 1.2;
            }
      
            // override with user settings

            lowq = 
               le_guinier_lowq2->text().toDouble() ?
               le_guinier_lowq2->text().toDouble() :
               lowq2;

            highq = 
               le_guinier_highq2->text().toDouble() ?
               le_guinier_highq2->text().toDouble() :
               highq2;
            // cout << "guinier domain " << lowq << ":" << highq << endl;
         }
      } else {
         if ( cb_Rt_guinier->isChecked() )
         {

            // first compute available guinier range
            // this could be calculated once upon adding curves

            for ( unsigned int i = 0; 
                  i < plotted_Iq.size();
                  i++ )
            {
               if ( plotted_Rt_guinier_valid.count(i) &&
                    plotted_Rt_guinier_valid[i] == true )
               {
                  if ( !any_guinier )
                  {
                     lowq2 = plotted_Rt_guinier_lowq2[i];
                     highq2 = plotted_Rt_guinier_highq2[i];
                     any_guinier = true;
                  } else {
                     if ( lowq2 > plotted_Rt_guinier_lowq2[i] )
                     {
                        lowq2 = plotted_Rt_guinier_lowq2[i];
                     }
                     if ( highq2 < plotted_Rt_guinier_highq2[i] )
                     {
                        highq2 = plotted_Rt_guinier_highq2[i];
                     }
                  }
               }
               if ( any_guinier )
               {
                  lowq2 *= .75;
                  highq2 *= 1.2;
               }
      
               // override with user settings

               lowq = 
                  le_guinier_lowq2->text().toDouble() ?
                  le_guinier_lowq2->text().toDouble() :
                  lowq2;

               highq = 
                  le_guinier_highq2->text().toDouble() ?
                  le_guinier_highq2->text().toDouble() :
                  highq2;
               // cout << "guinier domain " << lowq << ":" << highq << endl;
            }
         } else {
            // first compute available guinier range
            // this could be calculated once upon adding curves

            for ( unsigned int i = 0; 
                  i < plotted_Iq.size();
                  i++ )
            {
               if ( plotted_guinier_valid.count(i) &&
                    plotted_guinier_valid[i] == true )
               {
                  if ( !any_guinier )
                  {
                     lowq2 = plotted_guinier_lowq2[i];
                     highq2 = plotted_guinier_highq2[i];
                     any_guinier = true;
                  } else {
                     if ( lowq2 > plotted_guinier_lowq2[i] )
                     {
                        lowq2 = plotted_guinier_lowq2[i];
                     }
                     if ( highq2 < plotted_guinier_highq2[i] )
                     {
                        highq2 = plotted_guinier_highq2[i];
                     }
                  }
               }
            }
            if ( any_guinier )
            {
               lowq2 *= .75;
               highq2 *= 1.2;
            }
      
            // override with user settings

            lowq = 
               le_guinier_lowq2->text().toDouble() ?
               le_guinier_lowq2->text().toDouble() :
               lowq2;

            highq = 
               le_guinier_highq2->text().toDouble() ?
               le_guinier_highq2->text().toDouble() :
               highq2;
            // cout << "guinier domain " << lowq << ":" << highq << endl;
         }
      }
      return;
   }

   // not guinier mode

   // set arbitrary defaults
   lowq = 1e-2;
   highq = 5e0;
   
   // compute full non-guinier domain
   // this could be calculated once upon adding curves

   bool any_plots = false;
   for ( unsigned int i = 0; 
         i < plotted_Iq.size();
         i++ )
   {
      if ( !any_plots )
      {
         lowq = plotted_q[i][0];
         highq = plotted_q[i][plotted_q[i].size() - 1];
         any_plots = true;
      } else {
         if ( lowq > plotted_q[i][0] )
         {
            lowq = plotted_q[i][0];
         }
         if ( highq < plotted_q[i][plotted_q[i].size() - 1] )
         {
            highq = plotted_q[i][plotted_q[i].size() - 1];
         }
      }
   }

   if ( cb_user_range->isChecked() )
   {
      lowq = 
         le_user_lowq->text().toDouble() ?
         le_user_lowq->text().toDouble() :
         lowq;
      highq = 
         le_user_highq->text().toDouble() ?
         le_user_highq->text().toDouble() :
         highq;
   }
   //   cout << "non guinier domain " << lowq << ":" << highq << endl;
}

// sets lowI & highI based upon range
void US_Hydrodyn_Saxs::plot_range( 
                                  double lowq, 
                                  double highq,
                                  double &lowI, 
                                  double &highI
                                  )
{
   // arbitrary defaults
   lowI = 1e-3;
   highI = 1;

   if ( !plotted_q.size() ) 
   {
      return;
   }

   bool any_plots = false;

   // for each plot, 
   if ( cb_guinier->isChecked() )
   {
      if ( cb_cs_guinier->isChecked() )
      {
         // for each plot
         for ( unsigned int i = 0;
               i < plotted_Iq.size();
               i++ )      
         {
            // scan the q range
            for ( unsigned int j = 0; j < plotted_q2[i].size(); j++ )
            {
               if ( plotted_q2[i][j] >= lowq &&
                    plotted_q2[i][j] <= highq && 
                    plotted_I[i][j] > 0e0 )
               {
                  if ( !any_plots )
                  {
                     lowI = plotted_q[i][j] * plotted_I[i][j];
                     highI = plotted_q[i][j] * plotted_I[i][j];
                     any_plots = true;
                  } else {
                     if ( lowI > plotted_q[i][j] * plotted_I[i][j] )
                     {
                        lowI = plotted_q[i][j] * plotted_I[i][j];
                     }
                     if ( highI < plotted_q[i][j] * plotted_I[i][j] )
                     {
                        highI = plotted_q[i][j] * plotted_I[i][j];
                     }
                  }
               }
            }
         }
      } else {
         if ( cb_Rt_guinier->isChecked() )
         {
            // for each plot
            for ( unsigned int i = 0;
                  i < plotted_Iq.size();
                  i++ )      
            {
               // scan the q range
               for ( unsigned int j = 0; j < plotted_q2[i].size(); j++ )
               {
                  if ( plotted_q2[i][j] >= lowq &&
                       plotted_q2[i][j] <= highq && 
                       plotted_I[i][j] > 0e0 )
                  {
                     if ( !any_plots )
                     {
                        lowI = plotted_q[i][j] * plotted_q[i][j] * plotted_I[i][j];
                        highI = plotted_q[i][j] * plotted_q[i][j] * plotted_I[i][j];
                        any_plots = true;
                     } else {
                        if ( lowI > plotted_q[i][j] * plotted_q[i][j] * plotted_I[i][j] )
                        {
                           lowI = plotted_q[i][j] * plotted_q[i][j] * plotted_I[i][j];
                        }
                        if ( highI < plotted_q[i][j] * plotted_q[i][j] * plotted_I[i][j] )
                        {
                           highI = plotted_q[i][j] * plotted_q[i][j] * plotted_I[i][j];
                        }
                     }
                  }
               }
            }
         } else {
            // for each plot
            for ( unsigned int i = 0;
                  i < plotted_Iq.size();
                  i++ )      
            {
               // scan the q range
               for ( unsigned int j = 0; j < plotted_q2[i].size(); j++ )
               {
                  if ( plotted_q2[i][j] >= lowq &&
                       plotted_q2[i][j] <= highq &&
                       plotted_I[i][j] > 0e0 )
                  {
                     if ( !any_plots )
                     {
                        lowI = plotted_I[i][j];
                        highI = plotted_I[i][j];
                        any_plots = true;
                     } else {
                        if ( lowI > plotted_I[i][j] )
                        {
                           lowI = plotted_I[i][j];
                        }
                        if ( highI < plotted_I[i][j] )
                        {
                           highI = plotted_I[i][j];
                        }
                     }
                  }
               }
            }
         }
      }
   } else { // !guinier
      // for each plot
      for ( unsigned int i = 0; 
            i < plotted_Iq.size();
            i++ )      
      {
         // scan the q range
         for ( unsigned int j = 0; j < plotted_q[i].size(); j++ )
         {
            if ( plotted_q[i][j] >= lowq &&
                 plotted_q[i][j] <= highq &&
                 ( plotted_I[i][j] > 0e0 || cb_kratky->isChecked() ) )
            {
               if ( !any_plots )
               {
                  if ( cb_kratky->isChecked() )
                  {
                     lowI  = plotted_q2[ i ][ j ] * plotted_I[ i ][ j ];
                     highI = lowI;
                  } else {
                     lowI  = plotted_I[i][j];
                     highI = plotted_I[i][j];
                  }
                  any_plots = true;
               } else {
                  if ( cb_kratky->isChecked() )
                  {
                     double this_I = plotted_q2[ i ][ j ] * plotted_I[ i ][ j ];
                     if ( lowI > this_I )
                     {
                        lowI = this_I;
                     }
                     if ( highI < this_I )
                     {
                        highI = this_I;
                     }
                  } else {
                     if ( lowI > plotted_I[i][j] )
                     {
                        lowI = plotted_I[i][j];
                     }
                     if ( highI < plotted_I[i][j] )
                     {
                        highI = plotted_I[i][j];
                     }
                  }
               }
            }
         }
      }
   }
   if ( cb_user_range->isChecked() )
   {
      lowI = 
         le_user_lowI->text().toDouble() ?
         le_user_lowI->text().toDouble() :
         lowI;
      highI = 
         le_user_highI->text().toDouble() ?
         le_user_highI->text().toDouble() :
         highI;
   }

   if ( any_plots )
   {
      lowI *= .6;
      highI *= cb_kratky->isChecked() ? 1.2 : 1.7;
   }

   // cout << "plot range " << lowI << ":" << highI << endl;
}

void US_Hydrodyn_Saxs::crop_iq_data( vector < double > &q,
                                     vector < double > &I )
{
   vector < double > new_q;
   vector < double > new_I;

   if ( !q.size() )
   {
      return;
   }

   unsigned int p = 0;
   if ( q[p] == 0 )
   {
      p++;
   }

   // start copying 1 pt before 1st decrease:
   
   for ( p++ ; p < q.size(); p++ )
   {
      if ( I[p] < I[p - 1] )
      {
         //         p--;
         break;
      }
   }

   for (; p < q.size(); p++ )
   {
      if ( I[p] > 0 )
      {
         new_q.push_back(q[p]);
         new_I.push_back(I[p]);
      }
   }
   if ( new_q.size() > 10 )
   {
      q = new_q;
      I = new_I;
   }
}

void US_Hydrodyn_Saxs::crop_iq_data( vector < double > &q,
                                     vector < double > &I,
                                     vector < double > &I_errors )
{
   vector < double > new_q;
   vector < double > new_I;
   vector < double > new_I_errors;

   if ( !q.size() )
   {
      return;
   }

   unsigned int p = 0;
   if ( q[p] == 0 )
   {
      p++;
   }

   // start copying 1 pt before 1st decrease:
   
   for ( p++ ; p < q.size(); p++ )
   {
      if ( I[p] < I[p - 1] )
      {
         //         p--;
         break;
      }
   }

   for (; p < q.size(); p++ )
   {
      if ( I[p] > 0 )
      {
         new_q.push_back(q[p]);
         new_I.push_back(I[p]);
         new_I_errors.push_back(I_errors[p]);
      }
   }
   if ( new_q.size() > 10 )
   {
      q = new_q;
      I = new_I;
      I_errors = new_I_errors;
   }
}

void US_Hydrodyn_Saxs::set_current_method_buttons() 
{
   rb_saxs_iq_native_debye  ->setChecked(our_saxs_options->saxs_iq_native_debye);
   rb_saxs_iq_native_sh     ->setChecked(our_saxs_options->saxs_iq_native_sh);
   if ( started_in_expert_mode )
   {
      rb_saxs_iq_native_hybrid ->setChecked(our_saxs_options->saxs_iq_native_hybrid);
      rb_saxs_iq_native_hybrid2->setChecked(our_saxs_options->saxs_iq_native_hybrid2);
      rb_saxs_iq_native_hybrid3->setChecked(our_saxs_options->saxs_iq_native_hybrid3);
   }
   rb_saxs_iq_native_fast   ->setChecked(our_saxs_options->saxs_iq_native_fast);
   if ( started_in_expert_mode )
   {
      rb_saxs_iq_foxs          ->setChecked(our_saxs_options->saxs_iq_foxs);
   }
   rb_saxs_iq_crysol        ->setChecked(our_saxs_options->saxs_iq_crysol);
   if ( started_in_expert_mode )
   {
      rb_saxs_iq_sastbx        ->setChecked(our_saxs_options->saxs_iq_sastbx);
   }
   rb_sans_iq_native_debye  ->setChecked(our_saxs_options->sans_iq_native_debye);
   rb_sans_iq_native_sh     ->setChecked(our_saxs_options->sans_iq_native_sh);
   if ( started_in_expert_mode )
   {
      rb_sans_iq_native_hybrid ->setChecked(our_saxs_options->sans_iq_native_hybrid);
      rb_sans_iq_native_hybrid2->setChecked(our_saxs_options->sans_iq_native_hybrid2);
      rb_sans_iq_native_hybrid3->setChecked(our_saxs_options->sans_iq_native_hybrid3);
   }
   rb_sans_iq_native_fast   ->setChecked(our_saxs_options->sans_iq_native_fast);
   rb_sans_iq_cryson        ->setChecked(our_saxs_options->sans_iq_cryson);

   update_iqq_suffix();
}

void US_Hydrodyn_Saxs::set_saxs_iq() {
   if ( started_in_expert_mode ) {
      if ( rb_saxs_iq_native_debye->isChecked() ) {
         return set_saxs_iq( 0 );
      }
      if ( rb_saxs_iq_native_sh->isChecked() ) {
         return set_saxs_iq( 1 );
      }
      if ( rb_saxs_iq_native_hybrid->isChecked() ) {
         return set_saxs_iq( 2 );
      }
      if ( rb_saxs_iq_native_hybrid2->isChecked() ) {
         return set_saxs_iq( 3 );
      }
      if ( rb_saxs_iq_native_hybrid3->isChecked() ) {
         return set_saxs_iq( 4 );
      }
      if ( rb_saxs_iq_native_fast->isChecked() ) {
         return set_saxs_iq( 5 );
      }
      if ( rb_saxs_iq_foxs->isChecked() ) {
         return set_saxs_iq( 6 );
      }
      if ( rb_saxs_iq_crysol->isChecked() ) {
         return set_saxs_iq( 7 );
      }
      if ( rb_saxs_iq_sastbx->isChecked() ) {
         return set_saxs_iq( 8 );
      }
   } else {
      if ( rb_saxs_iq_native_debye->isChecked() ) {
         return set_saxs_iq( 0 );
      }
      if ( rb_saxs_iq_native_sh->isChecked() ) {
         return set_saxs_iq( 1 );
      }
      if ( rb_saxs_iq_native_fast->isChecked() ) {
         return set_saxs_iq( 2 );
      }
      if ( rb_saxs_iq_crysol->isChecked() ) {
         return set_saxs_iq( 3 );
      }
   }
}

void US_Hydrodyn_Saxs::set_saxs_iq(int val)
{
   int ref = 0;
   rb_saxs_iq_native_debye  ->setChecked( val == ref ); ref++;
   rb_saxs_iq_native_sh     ->setChecked( val == ref ); ref++;

   if ( started_in_expert_mode )
   {
      rb_saxs_iq_native_hybrid ->setChecked( val == ref ); ref++;
      rb_saxs_iq_native_hybrid2->setChecked( val == ref ); ref++;
      rb_saxs_iq_native_hybrid3->setChecked( val == ref ); ref++;
   }

   rb_saxs_iq_native_fast   ->setChecked( val == ref ); ref++;

   if ( started_in_expert_mode )
   {
      rb_saxs_iq_foxs          ->setChecked( val == ref ); ref++;
   }

   rb_saxs_iq_crysol        ->setChecked( val == ref ); ref++;

   if ( started_in_expert_mode )
   {
      rb_saxs_iq_sastbx        ->setChecked( val == ref ); ref++;
   }

   our_saxs_options->saxs_iq_native_debye   = rb_saxs_iq_native_debye  ->isChecked();
   our_saxs_options->saxs_iq_native_sh      = rb_saxs_iq_native_sh     ->isChecked();
   if ( started_in_expert_mode )
   {
      our_saxs_options->saxs_iq_native_hybrid  = rb_saxs_iq_native_hybrid ->isChecked();
      our_saxs_options->saxs_iq_native_hybrid2 = rb_saxs_iq_native_hybrid2->isChecked();
      our_saxs_options->saxs_iq_native_hybrid3 = rb_saxs_iq_native_hybrid3->isChecked();
   } else {
      our_saxs_options->saxs_iq_native_hybrid  = false;
      our_saxs_options->saxs_iq_native_hybrid2 = false;
      our_saxs_options->saxs_iq_native_hybrid3 = false;
   }
      

   our_saxs_options->saxs_iq_native_fast    = rb_saxs_iq_native_fast   ->isChecked();
   if ( started_in_expert_mode )
   {
      our_saxs_options->saxs_iq_foxs           = rb_saxs_iq_foxs          ->isChecked();
   } else {
      our_saxs_options->saxs_iq_foxs           = false;
   }      
   our_saxs_options->saxs_iq_crysol         = rb_saxs_iq_crysol        ->isChecked();
   if ( started_in_expert_mode )
   {
      our_saxs_options->saxs_iq_sastbx         = rb_saxs_iq_sastbx        ->isChecked();
   } else {
      our_saxs_options->saxs_iq_sastbx         = false;
   }      

   update_iqq_suffix();
}

void US_Hydrodyn_Saxs::set_sans_iq() {
   if ( started_in_expert_mode ) {
      if ( rb_sans_iq_native_debye->isChecked() ) {
         return set_sans_iq( 0 );
      }
      if ( rb_sans_iq_native_sh->isChecked() ) {
         return set_sans_iq( 1 );
      }
      if ( rb_sans_iq_native_hybrid->isChecked() ) {
         return set_sans_iq( 2 );
      }
      if ( rb_sans_iq_native_hybrid2->isChecked() ) {
         return set_sans_iq( 3 );
      }
      if ( rb_sans_iq_native_hybrid3->isChecked() ) {
         return set_sans_iq( 4 );
      }
      if ( rb_sans_iq_native_fast->isChecked() ) {
         return set_sans_iq( 5 );
      }
      if ( rb_sans_iq_cryson->isChecked() ) {
         return set_sans_iq( 6 );
      }
   } else {
      if ( rb_sans_iq_native_debye->isChecked() ) {
         return set_sans_iq( 0 );
      }
      if ( rb_sans_iq_native_sh->isChecked() ) {
         return set_sans_iq( 1 );
      }
      if ( rb_sans_iq_native_fast->isChecked() ) {
         return set_sans_iq( 2 );
      }
      if ( rb_sans_iq_cryson->isChecked() ) {
         return set_sans_iq( 3 );
      }
   }
}

void US_Hydrodyn_Saxs::set_sans_iq(int val)
{
   int ref = 0;
   rb_sans_iq_native_debye  ->setChecked( val == ref ); ref++;
   rb_sans_iq_native_sh     ->setChecked( val == ref ); ref++;
   if ( started_in_expert_mode )
   {
      rb_sans_iq_native_hybrid ->setChecked( val == ref ); ref++;
      rb_sans_iq_native_hybrid2->setChecked( val == ref ); ref++;
      rb_sans_iq_native_hybrid3->setChecked( val == ref ); ref++;
   }
   rb_sans_iq_native_fast   ->setChecked( val == ref ); ref++;
   rb_sans_iq_cryson        ->setChecked( val == ref ); ref++;

   our_saxs_options->sans_iq_native_debye   = rb_sans_iq_native_debye  ->isChecked();
   our_saxs_options->sans_iq_native_sh      = rb_sans_iq_native_sh     ->isChecked();
   if ( started_in_expert_mode )
   {
      our_saxs_options->sans_iq_native_hybrid  = rb_sans_iq_native_hybrid ->isChecked();
      our_saxs_options->sans_iq_native_hybrid2 = rb_sans_iq_native_hybrid2->isChecked();
      our_saxs_options->sans_iq_native_hybrid3 = rb_sans_iq_native_hybrid3->isChecked();
   } else {
      our_saxs_options->sans_iq_native_hybrid  = false;
      our_saxs_options->sans_iq_native_hybrid2 = false;
      our_saxs_options->sans_iq_native_hybrid3 = false;
   }
   our_saxs_options->sans_iq_native_fast    = rb_sans_iq_native_fast   ->isChecked();
   our_saxs_options->sans_iq_cryson         = rb_sans_iq_cryson        ->isChecked();

   update_iqq_suffix();
}
   
void US_Hydrodyn_Saxs::set_bead_model_ok_for_saxs()
{
//    for ( unsigned int i = 0; i < selected_models.size(); i++ )
//    {
//       for ( unsigned int j = 0; j < bead_models[selected_models[i]].size(); j++ )
//       {
//          // cout << "saxs name [" << j << "] = " << bead_models[i][j].saxs_data.saxs_name << endl;
//          if ( bead_models[i][j].active &&
//               bead_models[i][j].saxs_data.saxs_name.isEmpty() )
//          {
//             cout << "bead model " << i << " is NOT ok for saxs\n";
//             bead_model_ok_for_saxs = false;
//             return;
//          }
//       }
//    }
   cout << "bead models are ok for saxs\n";
   bead_model_ok_for_saxs = true;
}


bool US_Hydrodyn_Saxs::bead_model_has_electrons() {
   for ( unsigned int i = 0; i < selected_models.size(); i++ ) {
      for ( unsigned int j = 0; j < bead_models[selected_models[i]].size(); j++ ) {
         // cout << "saxs name [" << j << "] = " << bead_models[i][j].saxs_data.saxs_name << endl;
         if ( bead_models[i][j].active &&
              bead_models[i][j].num_elect <= 0 ){
            return false;
         }
      }
   }
   return true;
}

// #define DEBUG_RESID

void US_Hydrodyn_Saxs::display_iqq_residuals( QString title,
                                              vector < double > q,
                                              vector < double > I1, 
                                              vector < double > I2,
                                              QColor            plot_color,
                                              vector < double > I_errors)
{
   if ( !use_SDs_for_fitting_iqq && I_errors.size() ) {
      editor_msg( "dark red", us_tr( "Ignoring experimental errors" ) );
      I_errors.clear( );
   }

   // make sure things aren't to big
   unsigned int min_len = q.size();
   if ( I1.size() <  min_len ) 
   {
      min_len = I1.size();
   }
   if ( I2.size() <  min_len ) 
   {
      min_len = I2.size();
   }

   bool use_errors = is_nonzero_vector( I_errors ) && I_errors.size();
#if defined( DEBUG_RESID )
   cout << 
      QString("US_Hydrodyn_Saxs::display_iqq_residuals %1 errors\n")
         .arg( use_errors ? "using" : "no" );
#endif
   if ( use_errors && I_errors.size() <  min_len ) 
   {
      min_len = I_errors.size();
   }
   q.resize(min_len);

   vector < double > difference( q.size() );
   vector < double > difference_no_errors( q.size() );
   vector < double > log_difference( q.size() );
   vector < double > log_I1 ( I1.size() );
   vector < double > log_I2 ( I2.size() );

   double       avg_std_dev_frac        = 0e0;
   unsigned int avg_std_dev_frac_count  = 0;
   vector < double > std_dev_frac( q.size() );

#if defined( DEBUG_RESID )
   cout <<
      QString("q\ttarget\tcalc\terror\tdifference\tdifference/error\n");

   for ( unsigned int i = 0; i < q.size(); i++ )
   {
      cout <<
         QString("%1\t%2\t%3\t%4\t%5\t%6\n")
         .arg(q[i])
         .arg(I1[i])
         .arg(I2[i])
         .arg(I_errors[i])
         .arg(I2[i] - I1[i])
         .arg(I_errors[i] != 0 ? ( I2[i] - I1[i] ) / I_errors[i] : ( I2[i] - I1[i] ) );
   }
#endif

   for ( unsigned int i = 0; i < q.size(); i++ )
   {
      difference[ i ] = I2[ i ] - I1[ i ];
      difference_no_errors[ i ] = difference[ i ];
      if ( use_errors ) 
      {
         difference[ i ] /= I_errors[ i ];
         if ( I1[ i ] )
         {
            avg_std_dev_frac += I_errors[ i ] / I1[ i ];
            avg_std_dev_frac_count++;
            std_dev_frac[ i ] = I_errors[ i ] / I1[ i ];
         } else {
            std_dev_frac[ i ] = 0e0;
         }
      }
      log_I1[ i ] = log10( I1[ i ] );
      log_I2[ i ] = log10( I2[ i ] );
      if ( us_isnan( log_I1[ i ] ) )
      {
         log_I1[ i ] = 1e-34;
      }
      if ( us_isnan( log_I2[ i ] ) )
      {
         log_I2[ i ] = 1e-34;
      }
      log_difference[ i ] = log_I2[ i ] - log_I1[ i ];
   }

   if ( use_errors ) 
   {
      avg_std_dev_frac /= ( double ) avg_std_dev_frac_count;
   }

   // #ifndef WIN32
   if ( saxs_iqq_residuals_widgets.count(title) &&
        saxs_iqq_residuals_widgets[title] &&
        saxs_iqq_residuals_windows.count(title)
        )
   {
      // cout << "residuals add\n";
      saxs_iqq_residuals_windows[title]->add(
                                             plot_saxs->width(),
                                             q,
                                             difference,
                                             difference_no_errors,
                                             I2,
                                             log_difference,
                                             log_I2,
                                             plot_color
                                            );
                                            
      if (saxs_iqq_residuals_windows[title]->isVisible())
      {
         saxs_iqq_residuals_windows[title]->raise();
      } else {
         saxs_iqq_residuals_windows[title]->show();
      }

   } else {
      // cout << "residuals new\n";
      saxs_iqq_residuals_widgets[title] = true;
      saxs_iqq_residuals_windows[title] = 
         new US_Hydrodyn_Saxs_Iqq_Residuals(
                                            &saxs_iqq_residuals_widgets[title],
                                            plot_saxs->width(),
                                            us_tr("Residuals & difference targeting:\n") + title,
                                            q,
                                            difference,
                                            difference_no_errors,
                                            I2,
                                            log_difference,
                                            log_I2,
                                            plot_color,
                                            use_errors,
                                            false,
                                            true,
                                            !use_errors,
                                            avg_std_dev_frac,
                                            std_dev_frac,
                                            pen_width
                                            );
      US_Hydrodyn::fixWinButtons( saxs_iqq_residuals_windows[ title ] );
      saxs_iqq_residuals_windows[title]->show();
   }
   // #endif
}

void US_Hydrodyn_Saxs::update_iqq_suffix()
{
   QString qs = "";
   // don't forget to update US_Hydrodyn_Saxs_Options update to call this
   // don't forget about US_Hydrodyn_Batch::iqq_suffix

   // the method:
   if ( rb_saxs->isChecked() )
   {
      if ( our_saxs_options->saxs_iq_crysol )
      {
         qs += "cr";
         if ( our_saxs_options->crysol_version_3
              && our_saxs_options->crysol_water_dummy_beads ) {
            qs += QString("_h%1_sw_hs%2")
               .arg( our_saxs_options->sh_max_harmonics )
               .arg( QString("%1").arg( our_saxs_options->crysol_hydration_shell_contrast ).replace(".", "_" ) );
         } else {
            qs += QString("_h%1_g%2_hs%3")
               .arg( our_saxs_options->sh_max_harmonics )
               .arg( our_saxs_options->sh_fibonacci_grid_order )
               .arg( QString("%1").arg( our_saxs_options->crysol_hydration_shell_contrast ).replace(".", "_" ) );
         }
         if ( U_EXPT &&
              (( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "sas_crysol_ra" ) &&
              (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_ra" ].toDouble() > 0e0 )
         {
            qs += QString( "_ra%1" ).arg( (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_ra" ] ).replace(".", "_" );
         }
         if ( U_EXPT &&
              (( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "sas_crysol_vol" ) &&
              (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_vol" ].toDouble() > 0e0 )
         {
            qs += QString( "_ev%1" ).arg( (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_vol" ] ).replace(".", "_" );
         }
      } else {
         if ( our_saxs_options->saxs_iq_foxs )
         {
            qs += "fx";
         } else {
            if ( our_saxs_options->saxs_iq_native_debye )
            {
               qs += "db";
            }
            if ( our_saxs_options->saxs_iq_native_fast )
            {
               qs += "qd";
            }
            if ( our_saxs_options->saxs_iq_native_hybrid )
            {
               qs += "hy";
            }
            if ( our_saxs_options->saxs_iq_native_hybrid2 )
            {
               qs += "h2";
            }
            if ( our_saxs_options->saxs_iq_native_hybrid3 )
            {
               qs += "h3";
            }
            if ( our_saxs_options->saxs_iq_sastbx )
            {
   
               QString method;
               switch ( our_saxs_options->sastbx_method )
               {
               case 1:
                  method = "debye";
                  break;
               case 2:
                  method = "zernike";
                  break;
               case 0:
               default:
                  method = "she";
                  break;
               }
               qs += "st";
               qs += QString( "_%1__h%2_g%3_hs%4" )
                  .arg( method )
                  .arg( our_saxs_options->sh_max_harmonics )
                  .arg( our_saxs_options->sh_fibonacci_grid_order )
                  .arg( QString( "%1" ).arg( our_saxs_options->crysol_hydration_shell_contrast ).replace(".", "_" ) )
                  ;
            }
            if ( our_saxs_options->saxs_iq_native_sh )
            {
               qs += "sh";
               qs += QString( "_h%1" )
                  .arg( our_saxs_options->sh_max_harmonics )
                  ;
            }
            if ( ( our_saxs_options->saxs_iq_native_hybrid ||
                   our_saxs_options->saxs_iq_native_hybrid2 ||
                   our_saxs_options->saxs_iq_native_hybrid3 ) && 
                 our_saxs_options->saxs_iq_hybrid_adaptive )
            {
               qs += "a";
            }
            if ( our_saxs_options->scale_excl_vol != 1e0 )
            {
               qs += QString("_evs%1")
                  .arg( QString("%1").arg( our_saxs_options->scale_excl_vol ).replace(".", "_" ) );
            }
            if ( !our_saxs_options->autocorrelate )
            {
               qs += "_nac";
            }
            if ( our_saxs_options->swh_excl_vol != 0e0 )
            {
               qs += QString("_swh%1")
                  .arg( QString("%1").arg( our_saxs_options->swh_excl_vol ).replace(".", "_" ) );
            }
         }
      }
   }
   if ( rb_sans->isChecked() ) 
   {
      qs += "n";
      if ( our_saxs_options->sans_iq_cryson )
      {
         qs += "cr";
         qs += QString("_h%1_g%2_hs%3")
            .arg( our_saxs_options->cryson_sh_max_harmonics )
            .arg( our_saxs_options->cryson_sh_fibonacci_grid_order )
            .arg( QString("%1").arg( our_saxs_options->cryson_hydration_shell_contrast ).replace(".", "_" ) );
      }
      if ( our_saxs_options->sans_iq_native_debye )
      {
         qs += "db";
      }
      if ( our_saxs_options->sans_iq_native_fast )
      {
         qs += "qd";
      }
      if ( our_saxs_options->sans_iq_native_hybrid )
      {
         qs += "hy";
      }
      if ( our_saxs_options->sans_iq_native_hybrid2 )
      {
         qs += "h2";
      }
      if ( our_saxs_options->sans_iq_native_hybrid3 )
      {
         qs += "h3";
      }
      if ( our_saxs_options->sans_iq_native_sh )
      {
         qs += "sh";
         qs += QString( "_h%1" )
            .arg( our_saxs_options->sh_max_harmonics )
            ;
      }
      if ( ( our_saxs_options->sans_iq_native_hybrid ||
             our_saxs_options->sans_iq_native_hybrid2 ||
             our_saxs_options->sans_iq_native_hybrid3 ) && 
           our_saxs_options->sans_iq_hybrid_adaptive )
      {
         qs += "a";
      }
      if ( our_saxs_options->scale_excl_vol != 1e0 )
      {
         qs += QString("_evs%1")
            .arg( QString("%1").arg( our_saxs_options->scale_excl_vol ).replace(".", "_" ) );
      }
      if ( !our_saxs_options->autocorrelate )
      {
         qs += "_nac";
      }
      if ( our_saxs_options->swh_excl_vol != 0e0 )
      {
         qs += QString("_swh%1")
            .arg( QString("%1").arg( our_saxs_options->swh_excl_vol ).replace(".", "_" ) );
      }
   }

   // cout << "qs is now " << qs << endl;
   le_iqq_full_suffix->setText(qs);
}

QString US_Hydrodyn_Saxs::iqq_suffix()
{
   update_iqq_suffix();
   QString qs = 
      le_iqq_manual_suffix->text() +
      ( le_iqq_manual_suffix->text().length() ? "-" : "" ) +
      le_iqq_full_suffix->text();
   return qs.length() ? ( "-" + qs ) : "";
}

void US_Hydrodyn_Saxs::ask_iq_target_grid( bool force )
{
   if ( !force && everything_plotted_has_same_grid_as_set() )
   {
      return;
   }

   unsigned int q_points = 
      (unsigned int)floor(((our_saxs_options->end_q - our_saxs_options->start_q) / our_saxs_options->delta_q) + .5) + 1;
   
   QString gridmsg = 
      QString(us_tr("Current grid:"
                 "q range %1 to %2 with a stepsize of %3 giving %4 q-points.\n") )
      .arg(our_saxs_options->start_q)
      .arg(our_saxs_options->end_q)
      .arg(our_saxs_options->delta_q)
      .arg(q_points);
   editor_msg("dark blue", gridmsg);
      
   if ( !qsl_plotted_iq_names.size() )
   {
      return;
   }

   // display a list of the plotted grids and choose one or cancel

   bool ok;
   QString grid_target = US_Static::getItem(
                                               us_tr("Set I(q) Grid"),
                                               us_tr("Select the target plotted data to set the I(q) grid:\n"
                                                  "or Cancel of you do not wish to change the I(q) grid ")
                                               , 
                                               qsl_plotted_iq_names, 
                                               0, 
                                               false, 
                                               &ok,
                                               this );
   if ( !ok )
   {
      return;
   }
   if ( grid_target.isEmpty() ||
        !plotted_iq_names_to_pos.count( grid_target ) )
   {
      QMessageBox::warning( this, "Set I(q) grid",
                            QString(us_tr("Error: could not find the grid for target %1")). arg( grid_target ) );
      return;
   }

   // set the grid

   unsigned int pos = plotted_iq_names_to_pos[ grid_target ];

   if ( plotted_q[ pos ].size() < 2 )
   {
      QMessageBox::warning( this, "Set I(q) grid",
                            QString(us_tr("Error: Too few data points (%!) to set the grid for target %2"))
                            .arg( plotted_q[ pos ].size() ) 
                            .arg( grid_target ) 
                            );
      return;
   }


   // regrid question ...

   {
      switch( QMessageBox::question(this
                                    ,windowTitle() + " : Rebin" 
                                    ,QString(us_tr("Do you wish to rebin the selected target %1?") )
                                    .arg( grid_target )
                                    ) ) {
      case QMessageBox::Yes :
         {
            bool ok;
            int intervals = US_Static::getInteger(
                                                  windowTitle() + " : Log Rebin Rebin" 
                                                  ,QString( us_tr( 
                                                                  "Enter the number of log bins"
                                                                   ) )
                                                  ,100
                                                  ,10
                                                  ,10000
                                                  ,1
                                                  ,&ok
                                                  ,this 
                                                  );
            if ( ok ) {
               vector < double > rebin_q;
               vector < double > rebin_I;
               vector < double > rebin_e;
               QString errors;

               if (
                   log_rebin(
                             intervals
                             ,plotted_q[ pos ]
                             ,plotted_I[ pos ]
                             ,plotted_I_error[ pos ]
                             ,rebin_q
                             ,rebin_I
                             ,rebin_e
                             ,errors
                             )
                   ) {
                  QString new_target = grid_target;
                  new_target.replace( QRegularExpression( "\\.[^.]+$" ), "" );
                  plot_one_iqq( rebin_q, rebin_I, rebin_e, QString( "%1-lb%2" ).arg( new_target ).arg( intervals ) );
                  set_guinier();
               } else {
                  QMessageBox::critical( this
                                         ,windowTitle() + " : Log Rebin Rebin"
                                         ,errors );
               }
            }
            return;
         }
         break;
      default:
         break;
      }
   }

   int last_result = 1;

   float save_start_q      = our_saxs_options->start_q;
   float save_end_q        = our_saxs_options->end_q;
   float save_delta_q      = our_saxs_options->delta_q;
   float save_start_angle  = our_saxs_options->start_angle;
   float save_end_angle    = our_saxs_options->end_angle;
   float save_delta_angle  = our_saxs_options->delta_angle;

   vector < double > save_exact_q = exact_q;
   vector < double > new_exact_q  = plotted_q[ pos ];
   
   do {
      float start_q = plotted_q[ pos ][ 0 ];
      float end_q   = plotted_q[ pos ][ plotted_q[ pos ].size() -1 ];
      float delta_q = plotted_q[ pos ][ 1 ] - start_q;
      if ( delta_q <= 0e0 )
      {
         editor_msg("red", "NOTICE: plotted delta q is negative or zero: delta q set to 1e-6");
         delta_q = (float)1e-6;
      }

      start_q -= delta_q;
      if ( start_q < 0.0 )
      {
         start_q += delta_q;
      }
      
      our_saxs_options->start_q = start_q;
      our_saxs_options->end_q   = end_q;
      our_saxs_options->delta_q = delta_q * last_result;
      
      our_saxs_options->start_angle = 
         floor(
               (asin(our_saxs_options->wavelength * our_saxs_options->start_q / 
                     (4.0 * M_PI)) * 360.0 / M_PI) * SAXS_Q_ROUNDING + 0.5
               ) / SAXS_Q_ROUNDING;
      
      our_saxs_options->end_angle = 
         floor(
               (asin(our_saxs_options->wavelength * our_saxs_options->end_q / 
                     (4.0 * M_PI)) * 360.0 / M_PI) * SAXS_Q_ROUNDING + 0.5
               ) / SAXS_Q_ROUNDING;
      
      our_saxs_options->delta_angle = 
         floor(
               (asin(our_saxs_options->wavelength * our_saxs_options->delta_q / 
                     (4.0 * M_PI)) * 360.0 / M_PI) * SAXS_Q_ROUNDING + 0.5
               ) / SAXS_Q_ROUNDING;

      new_exact_q.clear( );
      for ( int i = 0; i < ( int ) plotted_q[ pos ].size(); i += last_result )
      {
         new_exact_q.push_back( plotted_q[ pos ][ i ] );
      }

      if ( ((US_Hydrodyn *)us_hydrodyn)->sas_options_curve_widget )
      {
         ((US_Hydrodyn *)us_hydrodyn)->sas_options_curve_window->cnt_start_q    ->setValue(our_saxs_options->start_q);
         ((US_Hydrodyn *)us_hydrodyn)->sas_options_curve_window->cnt_end_q      ->setValue(our_saxs_options->end_q);
         ((US_Hydrodyn *)us_hydrodyn)->sas_options_curve_window->cnt_delta_q    ->setValue(our_saxs_options->delta_q);
         ((US_Hydrodyn *)us_hydrodyn)->sas_options_curve_window->cnt_start_angle->setValue(our_saxs_options->start_angle);
         ((US_Hydrodyn *)us_hydrodyn)->sas_options_curve_window->cnt_end_angle  ->setValue(our_saxs_options->end_angle);
         ((US_Hydrodyn *)us_hydrodyn)->sas_options_curve_window->cnt_delta_angle->setValue(our_saxs_options->delta_angle);
      }

      unsigned int grid_points = 
         (unsigned int)floor(((our_saxs_options->end_q - our_saxs_options->start_q) / our_saxs_options->delta_q) + .5) + 1;

      int res = US_Static::getInteger(
                                         "SOMO: Confirm grid",
                                         QString( us_tr( 
                                                     "Original number of grid points %1\n"
                                                     "Target number of grid points using the divisor below is %2\n"
                                                     "Exact number of grid points using the divisor below is %3\n"
                                                     "If this is acceptable, press OK\n"
                                                     "You can enter a divisor below and press OK\n"
                                                     "Press Cancel to revert to the original grid\n"
                                                     ) )
                                         .arg( q_points )
                                         .arg( grid_points )
                                         .arg( new_exact_q.size() )
                                         , 
                                         last_result, 
                                         1,
                                         100000, 
                                         1, 
                                         &ok, 
                                         this 
                                         );
      if ( ok ) {
         if ( res != last_result )
         {
            ok = false;
            last_result = res;
            // recompute grid using n'th values
         }
      } else {
         // resort to original grid
         new_exact_q = save_exact_q;
         our_saxs_options->start_q     = save_start_q;
         our_saxs_options->end_q       = save_end_q;
         our_saxs_options->delta_q     = save_delta_q;
         our_saxs_options->start_angle = save_start_angle;
         our_saxs_options->end_angle   = save_end_angle;
         our_saxs_options->delta_angle = save_delta_angle;
         unsigned int grid_points = 
            (unsigned int)floor(((our_saxs_options->end_q - our_saxs_options->start_q) / our_saxs_options->delta_q) + .5) + 1;
         if ( !((US_Hydrodyn *)us_hydrodyn)->gui_script ) {
            US_Static::us_message(us_tr("SOMO: Grid restored"), 
                                  QString(us_tr("Reverted to orignal grid containing %1 points")).arg( grid_points ));
         }
         ok = true;
      }
   } while ( !ok );

   if ( ((US_Hydrodyn *)us_hydrodyn)->sas_options_curve_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_curve_window->cnt_start_q    ->setValue(our_saxs_options->start_q);
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_curve_window->cnt_end_q      ->setValue(our_saxs_options->end_q);
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_curve_window->cnt_delta_q    ->setValue(our_saxs_options->delta_q);
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_curve_window->cnt_start_angle->setValue(our_saxs_options->start_angle);
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_curve_window->cnt_end_angle  ->setValue(our_saxs_options->end_angle);
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_curve_window->cnt_delta_angle->setValue(our_saxs_options->delta_angle);
   }

   //    if ( new_exact_q != exact_q )
   exact_q = new_exact_q;
   {
      QString gridmsg = 
         QString( us_tr( "Current exact grid is now:"
                      "q range %1 to %2 with %3 q-points.\n" ) )
         .arg( exact_q[ 0 ] )
         .arg( exact_q.back() )
         .arg( exact_q.size() )
         ;
      editor_msg("blue", gridmsg);
   }      

   if ( 
        our_saxs_options->start_q != save_start_q ||
        our_saxs_options->end_q   != save_end_q   ||
        our_saxs_options->delta_q != save_delta_q
        )
   {
      unsigned int q_points = 
         (unsigned int)floor(((our_saxs_options->end_q - our_saxs_options->start_q) / our_saxs_options->delta_q) + .5) + 1;
      
      QString gridmsg = 
         QString(us_tr("Current grid is now:"
                    "q range %1 to %2 with a stepsize of %3 giving %4 q-points.\n") )
         .arg(our_saxs_options->start_q)
         .arg(our_saxs_options->end_q)
         .arg(our_saxs_options->delta_q)
         .arg(q_points);
      editor_msg("blue", gridmsg);
   }
}

bool US_Hydrodyn_Saxs::iq_plot_experimental_and_calculated_present()
{
   if ( plotted_q.size() <= 1 )
   {
      return false;
   }

   bool any_experimental = false;
   bool any_calculated   = false;

   for ( unsigned int i = 0; i < plotted_I_error.size(); i++ )
   {
      if ( is_nonzero_vector( plotted_I_error[ i ] ) )
      {
         any_experimental = true;
      } else {
         any_calculated   = true;
      }
      if ( any_experimental && any_calculated )
      {
         return true;
      }
   }  
   if ( any_experimental )
   {
      return true;
   }
   return false;
}

bool US_Hydrodyn_Saxs::iq_plot_only_experimental_present()
{
   if ( plotted_q.size() <= 1 )
   {
      return false;
   }

   bool any_experimental = false;
   bool any_calculated   = false;

   for ( unsigned int i = 0; i < plotted_I_error.size(); i++ )
   {
      if ( is_nonzero_vector( plotted_I_error[ i ] ) )
      {
         any_experimental = true;
      } else {
         any_calculated   = true;
      }
   }  
   if ( any_experimental && !any_calculated )
   {
      return true;
   }
   return false;
}

bool US_Hydrodyn_Saxs::everything_plotted_has_same_grid()
{
   if ( plotted_q.size() <= 1 )
   {
      return true;
   }

   double start_q = plotted_q[ 0 ][ 0 ];
   double end_q   = plotted_q[ 0 ][ plotted_q[ 0 ].size() - 1 ];
   double delta_q = plotted_q[ 0 ][ 1 ] - start_q;

   for ( unsigned int i = 1; i < plotted_q.size(); i++ )
   {
      if ( 
          QString("%1").arg(start_q) != 
          QString("%1").arg(plotted_q[ i ][ 0 ])
          ||
          QString("%1").arg(end_q)   != 
          QString("%1").arg(plotted_q[ i ][ plotted_q[ i ].size() -1 ])
          ||
          QString("%1").arg(delta_q) != 
          QString("%1").arg(plotted_q[ i ][ 1 ] - plotted_q[ i ][ 0 ]) )
      {
         return false;
      }
   }
   return true;
}

bool US_Hydrodyn_Saxs::everything_plotted_has_same_grid_as_set()
{
   if ( plotted_q.size() == 0 )
   {
      return true;
   }

   if ( !everything_plotted_has_same_grid() )
   {
      return false;
   }

   if ( 
       QString("%1").arg(our_saxs_options->start_q + our_saxs_options->delta_q) == 
       QString("%1").arg((float) plotted_q[ 0 ][ 0 ])
       &&
       QString("%1").arg(our_saxs_options->end_q)   == 
       QString("%1").arg((float) plotted_q[ 0 ][ plotted_q[ 0 ].size() - 1 ])
       &&
       QString("%1").arg(our_saxs_options->delta_q) == 
       QString("%1").arg((float) (plotted_q[ 0 ][ 1 ] - plotted_q[ 0 ][ 0 ]) ) )
   {
      return true;
   }

   // puts("ephsgas: false, details:");
   // cout <<  QString("%1 %2\n").arg(our_saxs_options->start_q + our_saxs_options->delta_q).arg((float) plotted_q[ 0 ][ 0 ]);
   // cout <<  QString("%1 %2\n").arg(our_saxs_options->end_q).arg((float) plotted_q[ 0 ][ plotted_q[ 0 ].size() - 1 ]);
   // cout <<  QString("%1 %2\n").arg(our_saxs_options->delta_q).arg((float) (plotted_q[ 0 ][ 1 ] - plotted_q[ 0 ][ 0 ]) );
   return false;
}

void US_Hydrodyn_Saxs::fix_sas_options()
{
   
   // cout << "fix_sas_options\n";
   bool any_selected = false;

   if ( our_saxs_options->saxs_iq_native_debye )
   {
      any_selected = true;
   }

   if ( our_saxs_options->saxs_iq_native_sh )
   {
      if ( any_selected )
      {
         our_saxs_options->saxs_iq_native_sh = false;
      }
      any_selected = true;
   }
   if ( our_saxs_options->saxs_iq_native_hybrid  )
   {
      if ( any_selected )
      {
         our_saxs_options->saxs_iq_native_hybrid = false;
      }
      any_selected = true;
   }
   if ( our_saxs_options->saxs_iq_native_hybrid2 )
   {
      if ( any_selected )
      {
         our_saxs_options->saxs_iq_native_hybrid2 = false;
      }
      any_selected = true;
   }
   if ( our_saxs_options->saxs_iq_native_hybrid3 )
   {
      if ( any_selected )
      {
         our_saxs_options->saxs_iq_native_hybrid3 = false;
      }
      any_selected = true;
   }
   if ( our_saxs_options->saxs_iq_native_fast )
   {
      if ( any_selected )
      {
         our_saxs_options->saxs_iq_native_fast = false;
      }
      any_selected = true;
   }
   if ( our_saxs_options->saxs_iq_native_fast_compute_pr )
   {
      if ( any_selected )
      {
         our_saxs_options->saxs_iq_native_fast_compute_pr = false;
      }
      any_selected = true;
   }
   if ( our_saxs_options->saxs_iq_foxs )
   {
      if ( any_selected )
      {
         our_saxs_options->saxs_iq_foxs = false;
      }
      any_selected = true;
   }
   if ( our_saxs_options->saxs_iq_crysol )
   {
      if ( any_selected )
      {
         our_saxs_options->saxs_iq_crysol = false;
      }
      any_selected = true;
   }
   if ( our_saxs_options->saxs_iq_sastbx )
   {
      if ( any_selected )
      {
         our_saxs_options->saxs_iq_sastbx = false;
      }
      any_selected = true;
   }

   if ( !any_selected )
   {
      our_saxs_options->saxs_iq_native_debye = true;
   }

   any_selected = false;

   if ( our_saxs_options->sans_iq_native_debye )
   {
      any_selected = true;
   }
   if ( our_saxs_options->sans_iq_native_sh )
   {
      if ( any_selected )
      {
         our_saxs_options->sans_iq_native_sh = false;
      }
      any_selected = true;
   }
   if ( our_saxs_options->sans_iq_native_hybrid )
   {
      if ( any_selected )
      {
         our_saxs_options->sans_iq_native_hybrid = false;
      }
      any_selected = true;
   }
   if ( our_saxs_options->sans_iq_native_hybrid2 )
   {
      if ( any_selected )
      {
         our_saxs_options->sans_iq_native_hybrid2 = false;
      }
      any_selected = true;
   }
   if ( our_saxs_options->sans_iq_native_hybrid3 )
   {
      if ( any_selected )
      {
         our_saxs_options->sans_iq_native_hybrid3 = false;
      }
      any_selected = true;
   }
   if ( our_saxs_options->sans_iq_native_fast )
   {
      if ( any_selected )
      {
         our_saxs_options->sans_iq_native_fast = false;
      }
      any_selected = true;
   }
   if ( our_saxs_options->sans_iq_native_fast_compute_pr )
   {
      if ( any_selected )
      {
         our_saxs_options->sans_iq_native_fast_compute_pr = false;
      }
      any_selected = true;
   }
   if ( our_saxs_options->sans_iq_cryson )
   {
      if ( any_selected )
      {
         our_saxs_options->sans_iq_cryson = false;
      }
      any_selected = true;
   }

   if ( !any_selected )
   {
      our_saxs_options->sans_iq_native_debye = true;
   }
   set_current_method_buttons();
}

void US_Hydrodyn_Saxs::hide_widgets( vector < QWidget * > w, bool hide )
{
   for ( unsigned int i = 0; i < ( unsigned int )w.size(); i++ )
   {
      hide ? w[ i ]->hide() : w[ i ]->show();
   }
}

void US_Hydrodyn_Saxs::hide_pr()
{
   hide_widgets( pr_widgets, pr_widgets[ 0 ]->isVisible() );
   if ( !iq_widgets[ 0 ]->isVisible() && !pr_widgets[ 0 ]->isVisible() )
   {
      plot_pr->show();
   } else {
      if ( pr_widgets[ 0 ]->isVisible() && !iq_widgets[ 0 ]->isVisible() )
      {
         plot_saxs->hide();
      }
   }
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_pr" ]        = pr_widgets[ 0 ]->isVisible() ? "visible" : "hidden";
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_plot_saxs" ] = plot_saxs      ->isVisible() ? "visible" : "hidden";
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_plot_pr"   ] = plot_pr        ->isVisible() ? "visible" : "hidden";
}

void US_Hydrodyn_Saxs::hide_iq()
{
   hide_widgets( iq_widgets, iq_widgets[ 0 ]->isVisible() );
   if ( !iq_widgets[ 0 ]->isVisible() && !pr_widgets[ 0 ]->isVisible() )
   {
      plot_saxs->show();
   } else {
      if ( iq_widgets[ 0 ]->isVisible() && !pr_widgets[ 0 ]->isVisible() )
      {
         plot_pr->hide();
      }
   }
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_iq" ]        = iq_widgets[ 0 ]->isVisible() ? "visible" : "hidden";
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_plot_saxs" ] = plot_saxs      ->isVisible() ? "visible" : "hidden";
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_plot_pr"   ] = plot_pr        ->isVisible() ? "visible" : "hidden";
}

void US_Hydrodyn_Saxs::hide_settings()
{
   hide_widgets( settings_widgets, settings_widgets[ 0 ]->isVisible() );
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_settings" ] = settings_widgets[ 0 ]->isVisible() ? "visible" : "hidden";
}

void US_Hydrodyn_Saxs::check_mwt_constants( bool force ) 
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

void US_Hydrodyn_Saxs::wheel_pressed() {
   // qDebug() << "wheel_pressed()";
   wheel_is_pressed = true;
}

void US_Hydrodyn_Saxs::wheel_released() {
   // qDebug() << "wheel_released()";
   wheel_is_pressed = false;
}

void US_Hydrodyn_Saxs::usp_config_plot_saxs( const QPoint & ) {
   US_PlotChoices *uspc = new US_PlotChoices( usp_plot_saxs );
   uspc->exec();
   delete uspc;
}

void US_Hydrodyn_Saxs::usp_config_plot_pr( const QPoint & ) {
   US_PlotChoices *uspc = new US_PlotChoices( usp_plot_pr );
   uspc->exec();
   delete uspc;
}

void US_Hydrodyn_Saxs::usp_config_plot_resid( const QPoint & ) {
   US_PlotChoices *uspc = new US_PlotChoices( usp_plot_resid );
   uspc->exec();
   delete uspc;
}

bool US_Hydrodyn_Saxs::write_temp_pdb_selected_models( QString & error_msg ) {
   if ( !last_selected_pdb_filename.isEmpty() ) {
      return true;
   }

   QString use_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_tmp_dir + QDir::separator();
   unsigned int pos = 0;

   do {
      //      last_selected_pdb_filename = QString("%1%2-%3.pdb").arg( use_dir ).arg( QFileInfo( contrib_file ).baseName() ).arg( pos );
      last_selected_pdb_filename = QString("%1%2-%3.pdb").arg( use_dir ).arg( "temp" ).arg( pos );
      pos++;
   } while( QFile::exists( last_selected_pdb_filename ) );

   QFile f( last_selected_pdb_filename );
   if ( !f.open( QIODevice::WriteOnly ) ) {
      error_msg = QString( us_tr("can not open file %1 for writing" ) ).arg( last_selected_pdb_filename );
      last_selected_pdb_filename = "";
      return false;
   }

   QString pdb_header = "REMARK US-SOMO temporary file for visualization\n";
   QString pdb_text   = "";

   for ( int h = 0; h < (int) selected_models.size(); ++h ) {
      int i = selected_models[ h ];
      pdb_text += QString( "MODEL     %1\n" ).arg( i + 1 );
      for (unsigned int j = 0; j < (unsigned int) model_vector[i].molecule.size (); j++) {
         for (unsigned int k = 0; k < (unsigned int) model_vector[i].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);

            pdb_text +=
               QString("")
               .sprintf(     
                        "ATOM  %5d%5s%4s %1s%4d    %8.3f%8.3f%8.3f%6.2f%6.2f          %2s\n",
                        this_atom->serial,
                        this_atom->orgName.toLatin1().data(),
                        this_atom->resName.toLatin1().data(),
                        this_atom->chainID.toLatin1().data(),
                        this_atom->resSeq.toUInt(),
                        this_atom->coordinate.axis[ 0 ],
                        this_atom->coordinate.axis[ 1 ],
                        this_atom->coordinate.axis[ 2 ],
                        this_atom->occupancy,
                        this_atom->tempFactor,
                        this_atom->element.toLatin1().data()
                             );
         }
      }

      pdb_text +=
         "TER\nENDMDL\n";
   }

   QTextStream ts( &f );
   ts << pdb_header;
   ts << pdb_text;
   ts << "END\n";
   f.close();

   // qDebug() << QString( us_tr( "File %1 created\n" ) ).arg( last_selected_pdb_filename );
   return true;
}

QString US_Hydrodyn_Saxs::info_remember_mw( const QString & msg ) {
   return
      QString( "--------------------------------------------------------------------------------\n" )
      + QString( "info_remember_mw() %1\n" ).arg( msg )
      + "--------------------------------------------------------------------------------\n"
      + US_Vector::qs_mapqsfloat( "remember_mw", *remember_mw )
      + "\n"
      + US_Vector::qs_mapqsfloat( "match_remember_mw", *match_remember_mw )
      + "\n"
      + US_Vector::qs_mapqsqs( "remember_mw_source", *remember_mw_source )
      + "\n"
      + "--------------------------------------------------------------------------------\n"
      ;
}
