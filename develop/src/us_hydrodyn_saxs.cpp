#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_hydrodyn_saxs_options.h"
#include "../include/us_hydrodyn_saxs_load_csv.h"
#include "../include/us_hydrodyn_saxs_mw.h"
#ifndef QT4
#  include "../include/us_hydrodyn_saxs_screen.h"
#endif
#include "../include/us_hydrodyn_saxs_search.h"
#include "../include/us_hydrodyn_saxs_buffer.h"
#include "../include/us_saxs_util.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_math.h"
#include "qwt_symbol.h"
#ifdef QT4
# include <qwt_scale_engine.h>
#endif

#define SLASH "/"
#if defined(WIN32)
#  include <dos.h>
#  include <stdlib.h>
#  include <float.h>
#  undef SLASH
#  define SLASH "\\"
#  define isnan _isnan
#endif

// note: this program uses cout and/or cerr and this should be replaced

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
                                   const char                     *name
                                   ) : QFrame(p, name)
{
   rasmol = NULL;
   this->saxs_widget = saxs_widget;
   *saxs_widget = true;
   this->our_saxs_options = our_saxs_options;
   model_filepathname = filepathname;
   guinier_cutoff = 0.2;
   last_used_mw = 0.0;

   plot_pr_zoomer   = (ScrollZoomer *)0;
   plot_saxs_zoomer = (ScrollZoomer *)0;

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

   USglobal=new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("SAXS/SANS Plotting Functions"));
   reset_search_csv();
   reset_screen_csv();
   reset_buffer_csv();
   setupGUI();
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
         QMessageBox mb(tr("UltraScan"),
                        tr("The source file has not been defined, please try again..."), QMessageBox::Critical,
                           QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
         if (mb.exec())
         {
            exit(-1);
         }
      }
   }
   if ( !selected_models.size() )
   {
      bead_model_ok_for_saxs = false;
   }
   pb_plot_saxs_sans->setEnabled(bead_model_ok_for_saxs);
   te_filename2->setText(filename);
   model_filename = filename;
   atom_filename = USglobal->config_list.system_dir + SLASH + "etc" + SLASH + "somo.atom";
   hybrid_filename = USglobal->config_list.system_dir + SLASH + "etc" + SLASH + "somo.hybrid";
   saxs_filename =  USglobal->config_list.system_dir + SLASH + "etc" + SLASH + "somo.saxs_atoms";
   select_saxs_file(saxs_filename);
   select_hybrid_file(hybrid_filename);
   select_atom_file(atom_filename);
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
   stopFlag = false;
   pb_stop->setEnabled(false);
   plot_colors.clear();
   plot_colors.push_back(Qt::yellow);
   plot_colors.push_back(Qt::green);
   plot_colors.push_back(Qt::cyan);
   plot_colors.push_back(Qt::blue);
   plot_colors.push_back(Qt::red);
   plot_colors.push_back(Qt::magenta);
   plot_colors.push_back(Qt::darkYellow);
   plot_colors.push_back(Qt::darkGreen);
   plot_colors.push_back(Qt::darkCyan);
   plot_colors.push_back(Qt::darkBlue);
   //   plot_colors.push_back(Qt::darkRed);
   plot_colors.push_back(Qt::darkMagenta);
   plot_colors.push_back(Qt::white);
   saxs_search_update_enables();
   add_to_directory_history( ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "saxs" + SLASH + "x" );
   add_to_directory_history( ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "x" );
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
         QMessageBox mb(tr("UltraScan"),
                        tr("The source file has not been defined, please try again..."), QMessageBox::Critical,
                           QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
         if (mb.exec())
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
   started_in_expert_mode = ((US_Hydrodyn *)us_hydrodyn)->advanced_config.expert_mode;

   int minHeight0 = 22;
   int minHeight1 = 30;
   lbl_info = new QLabel(tr("SAXS/SANS I(q) Plotting Functions:"), this);
   Q_CHECK_PTR(lbl_info);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_filename1 = new QLabel(tr(""), this);
   Q_CHECK_PTR(lbl_filename1);
   lbl_filename1->setMinimumHeight(minHeight1);
   lbl_filename1->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_filename1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_filename1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   te_filename2 = new QTextEdit(this, "");
   Q_CHECK_PTR(te_filename2);
   te_filename2->setMinimumHeight(minHeight1);
   te_filename2->setMaximumHeight(minHeight1);
   te_filename2->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   te_filename2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   te_filename2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   te_filename2->setMinimumWidth(200);

   pb_select_atom_file = new QPushButton(tr("Load Atom Definition File"), this);
   Q_CHECK_PTR(pb_select_atom_file);
   pb_select_atom_file->setMinimumHeight(minHeight1);
   pb_select_atom_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_select_atom_file->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_select_atom_file, SIGNAL(clicked()), SLOT(select_atom_file()));

   pb_select_hybrid_file = new QPushButton(tr("Load Hybridization File"), this);
   Q_CHECK_PTR(pb_select_hybrid_file);
   pb_select_hybrid_file->setMinimumHeight(minHeight1);
   pb_select_hybrid_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_select_hybrid_file->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_select_hybrid_file, SIGNAL(clicked()), SLOT(select_hybrid_file()));

   pb_select_saxs_file = new QPushButton(tr("Load SAXS Coefficients File"), this);
   Q_CHECK_PTR(pb_select_saxs_file);
   pb_select_saxs_file->setMinimumHeight(minHeight1);
   pb_select_saxs_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_select_saxs_file->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_select_saxs_file, SIGNAL(clicked()), SLOT(select_saxs_file()));

   rb_saxs = new QRadioButton(tr("SAXS"), this);
   rb_saxs->setEnabled(true);
   rb_saxs->setChecked(!our_saxs_options->saxs_sans);
   rb_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_saxs->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   rb_sans = new QRadioButton(tr("SANS"), this);
   rb_sans->setEnabled(true);
   rb_sans->setChecked(our_saxs_options->saxs_sans);
   rb_sans->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_sans->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   bg_saxs_sans = new QButtonGroup(1, Qt::Horizontal, 0);
   bg_saxs_sans->setRadioButtonExclusive(true);
   bg_saxs_sans->insert(rb_saxs);
   bg_saxs_sans->insert(rb_sans);
   connect(bg_saxs_sans, SIGNAL(clicked(int)), SLOT(set_saxs_sans(int)));

   rb_saxs_iq_native_debye = new QRadioButton(tr("Full"), this);
   rb_saxs_iq_native_debye->setEnabled(true);
   rb_saxs_iq_native_debye->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_saxs_iq_native_debye->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   if ( started_in_expert_mode )
   {
      rb_saxs_iq_native_hybrid = new QRadioButton(tr("Hybrid   "), this);
      rb_saxs_iq_native_hybrid->setEnabled(true);
      rb_saxs_iq_native_hybrid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      rb_saxs_iq_native_hybrid->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

      rb_saxs_iq_native_hybrid2 = new QRadioButton(tr("H2"), this);
      rb_saxs_iq_native_hybrid2->setEnabled(true);
      rb_saxs_iq_native_hybrid2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      rb_saxs_iq_native_hybrid2->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   }

   rb_saxs_iq_native_hybrid3 = new QRadioButton(tr("H3"), this);
   rb_saxs_iq_native_hybrid3->setEnabled(true);
   rb_saxs_iq_native_hybrid3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_saxs_iq_native_hybrid3->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   rb_saxs_iq_native_fast = new QRadioButton(tr("Fast"), this);
   rb_saxs_iq_native_fast->setEnabled(true);
   rb_saxs_iq_native_fast->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_saxs_iq_native_fast->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   rb_saxs_iq_foxs = new QRadioButton(tr("FoXS"), this);
   rb_saxs_iq_foxs->setEnabled(true);
   rb_saxs_iq_foxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_saxs_iq_foxs->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   rb_saxs_iq_crysol = new QRadioButton(tr("Crysol"), this);
   rb_saxs_iq_crysol->setEnabled(true);
   rb_saxs_iq_crysol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_saxs_iq_crysol->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   bg_saxs_iq = new QButtonGroup(1, Qt::Horizontal, 0);
   bg_saxs_iq->setRadioButtonExclusive(true);
   bg_saxs_iq->insert(rb_saxs_iq_native_debye);
   if ( started_in_expert_mode )
   {
      bg_saxs_iq->insert(rb_saxs_iq_native_hybrid);
      bg_saxs_iq->insert(rb_saxs_iq_native_hybrid2);
   }
   bg_saxs_iq->insert(rb_saxs_iq_native_hybrid3);
   bg_saxs_iq->insert(rb_saxs_iq_native_fast);
   bg_saxs_iq->insert(rb_saxs_iq_foxs);
   bg_saxs_iq->insert(rb_saxs_iq_crysol);
   connect(bg_saxs_iq, SIGNAL(clicked(int)), SLOT(set_saxs_iq(int)));

   rb_sans_iq_native_debye = new QRadioButton(tr("Full"), this);
   rb_sans_iq_native_debye->setEnabled(true);
   rb_sans_iq_native_debye->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_sans_iq_native_debye->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   if ( started_in_expert_mode )
   {
      rb_sans_iq_native_hybrid = new QRadioButton(tr("Hybrid   "), this);
      rb_sans_iq_native_hybrid->setEnabled(true);
      rb_sans_iq_native_hybrid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      rb_sans_iq_native_hybrid->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

      rb_sans_iq_native_hybrid2 = new QRadioButton(tr("H2"), this);
      rb_sans_iq_native_hybrid2->setEnabled(true);
      rb_sans_iq_native_hybrid2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      rb_sans_iq_native_hybrid2->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   }

   rb_sans_iq_native_hybrid3 = new QRadioButton(tr("H3"), this);
   rb_sans_iq_native_hybrid3->setEnabled(true);
   rb_sans_iq_native_hybrid3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_sans_iq_native_hybrid3->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   rb_sans_iq_native_fast = new QRadioButton(tr("Fast"), this);
   rb_sans_iq_native_fast->setEnabled(true);
   rb_sans_iq_native_fast->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_sans_iq_native_fast->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   rb_sans_iq_cryson = new QRadioButton(tr("Cryson"), this);
   rb_sans_iq_cryson->setEnabled(true);
   rb_sans_iq_cryson->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_sans_iq_cryson->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   bg_sans_iq = new QButtonGroup(1, Qt::Horizontal, 0);
   bg_sans_iq->setRadioButtonExclusive(true);
   bg_sans_iq->insert(rb_sans_iq_native_debye);
   if ( started_in_expert_mode )
   {
      bg_sans_iq->insert(rb_sans_iq_native_hybrid);
      bg_sans_iq->insert(rb_sans_iq_native_hybrid2);
   }
   bg_sans_iq->insert(rb_sans_iq_native_hybrid3);
   bg_sans_iq->insert(rb_sans_iq_native_fast);
   bg_sans_iq->insert(rb_sans_iq_cryson);
   connect(bg_sans_iq, SIGNAL(clicked(int)), SLOT(set_sans_iq(int)));

   lbl_iqq_suffix = new QLabel(tr(" File suffix: "), this);
   lbl_iqq_suffix->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_iqq_suffix->setMinimumHeight(minHeight1);
   lbl_iqq_suffix->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_iqq_suffix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_iqq_manual_suffix = new QLineEdit(this, "iqq_manual_suffix Line Edit");
   le_iqq_manual_suffix->setText(tr(""));
   le_iqq_manual_suffix->setMinimumHeight(minHeight1);
   le_iqq_manual_suffix->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_iqq_manual_suffix->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_iqq_manual_suffix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   le_iqq_full_suffix = new QLineEdit(this, "iqq_full_suffix Line Edit");
   le_iqq_full_suffix->setText(tr(""));
   le_iqq_full_suffix->setMinimumHeight(minHeight1);
   le_iqq_full_suffix->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_iqq_full_suffix->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_iqq_full_suffix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_iqq_full_suffix->setReadOnly(true);

   lbl_atom_table = new QLabel(tr(" not selected"),this);
   lbl_atom_table->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_atom_table->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_atom_table->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lbl_atom_table->setMinimumHeight(minHeight1);
   lbl_atom_table->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_hybrid_table = new QLabel(tr(" not selected"),this);
   lbl_hybrid_table->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_hybrid_table->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_hybrid_table->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lbl_hybrid_table->setMinimumHeight(minHeight1);
   lbl_hybrid_table->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_saxs_table = new QLabel(tr(" not selected"),this);
   lbl_saxs_table->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_saxs_table->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_saxs_table->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lbl_saxs_table->setMinimumHeight(minHeight1);
   lbl_saxs_table->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   pb_plot_saxs_sans = new QPushButton("", this);
   Q_CHECK_PTR(pb_plot_saxs_sans);
   pb_plot_saxs_sans->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_plot_saxs_sans->setMinimumHeight(minHeight1);
   pb_plot_saxs_sans->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_plot_saxs_sans, SIGNAL(clicked()), SLOT(show_plot_saxs_sans()));

   pb_load_saxs_sans = new QPushButton("", this);
   Q_CHECK_PTR(pb_load_saxs_sans);
   pb_load_saxs_sans->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_load_saxs_sans->setMinimumHeight(minHeight1);
   pb_load_saxs_sans->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_load_saxs_sans, SIGNAL(clicked()), SLOT(load_saxs_sans()));

   pb_load_plot_saxs = new QPushButton(tr("Load Plotted"), this);
   pb_load_plot_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_load_plot_saxs->setMinimumHeight(minHeight1);
   pb_load_plot_saxs->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_load_plot_saxs, SIGNAL(clicked()), SLOT(load_plot_saxs()));

   pb_set_grid = new QPushButton(tr("Set Grid"), this);
   pb_set_grid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_set_grid->setMinimumHeight(minHeight1);
   pb_set_grid->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_set_grid, SIGNAL(clicked()), SLOT(set_grid()));

   pb_clear_plot_saxs = new QPushButton("", this);
   pb_clear_plot_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_clear_plot_saxs->setMinimumHeight(minHeight1);
   pb_clear_plot_saxs->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_clear_plot_saxs, SIGNAL(clicked()), SLOT(clear_plot_saxs()));

   pb_saxs_legend = new QPushButton( "Legend", this);
   pb_saxs_legend->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_saxs_legend->setMinimumHeight(minHeight1);
   pb_saxs_legend->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_saxs_legend, SIGNAL(clicked()), SLOT(saxs_legend()));

   cb_create_native_saxs = new QCheckBox(this);
   cb_create_native_saxs->setText(tr(" Create standard output files"));
   cb_create_native_saxs->setEnabled(true);
   cb_create_native_saxs->setChecked(create_native_saxs);
   cb_create_native_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_create_native_saxs->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_create_native_saxs, SIGNAL(clicked()), SLOT(set_create_native_saxs()));

   pb_load_gnom = new QPushButton("Load GNOM File", this);
   pb_load_gnom->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_load_gnom->setMinimumHeight(minHeight1);
   pb_load_gnom->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_load_gnom, SIGNAL(clicked()), SLOT(load_gnom()));

   pb_saxs_search = new QPushButton("Search", this);
   pb_saxs_search->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_saxs_search->setMinimumHeight(minHeight1);
   pb_saxs_search->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_saxs_search, SIGNAL(clicked()), SLOT(saxs_search()));

   if ( started_in_expert_mode )
   {
      pb_saxs_screen = new QPushButton("Screen", this);
      pb_saxs_screen->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
      pb_saxs_screen->setMinimumHeight(minHeight1);
      pb_saxs_screen->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
      connect(pb_saxs_screen, SIGNAL(clicked()), SLOT(saxs_screen()));
   }
   
   cb_guinier = new QCheckBox(this);
   cb_guinier->setText(tr(" Guinier plot    q^2 range:"));
   // cb_guinier->setMinimumHeight(minHeight1);
   cb_guinier->setEnabled(true);
   cb_guinier->setChecked(false);
   cb_guinier->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_guinier->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_guinier, SIGNAL(clicked()), SLOT(set_guinier()));

   le_guinier_lowq2 = new QLineEdit(this, "guinier_lowq2 Line Edit");
   le_guinier_lowq2->setText("");
   // le_guinier_lowq2->setMinimumHeight(minHeight1);
   le_guinier_lowq2->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_lowq2->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_guinier_lowq2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_guinier_lowq2, SIGNAL(textChanged(const QString &)), SLOT(update_guinier_lowq2(const QString &)));

   le_guinier_highq2 = new QLineEdit(this, "guinier_highq2 Line Edit");
   le_guinier_highq2->setText("");
   // le_guinier_highq2->setMinimumHeight(minHeight1);
   le_guinier_highq2->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_highq2->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_guinier_highq2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_guinier_highq2, SIGNAL(textChanged(const QString &)), SLOT(update_guinier_highq2(const QString &)));

   cb_user_range = new QCheckBox(this);
   cb_user_range->setText(tr(" Standard"));
   // cb_user_range->setMinimumHeight(minHeight1);
   cb_user_range->setEnabled(true);
   cb_user_range->setChecked(false);
   cb_user_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_user_range->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_user_range, SIGNAL(clicked()), SLOT(set_user_range()));

   cb_kratky = new QCheckBox(this);
   cb_kratky->setText(tr(" Kratky plot  q range:"));
   // cb_kratky->setMinimumHeight(minHeight1);
   cb_kratky->setEnabled(true);
   cb_kratky->setChecked(false);
   cb_kratky->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_kratky->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_kratky, SIGNAL(clicked()), SLOT(set_kratky()));

   le_user_lowq = new QLineEdit(this, "user_lowq Line Edit");
   le_user_lowq->setText("");
   // le_user_lowq->setMinimumHeight(minHeight1);
   le_user_lowq->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_user_lowq->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_user_lowq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_user_lowq, SIGNAL(textChanged(const QString &)), SLOT(update_user_lowq(const QString &)));

   le_user_highq = new QLineEdit(this, "user_highq Line Edit");
   le_user_highq->setText("");
   // le_user_highq->setMinimumHeight(minHeight1);
   le_user_highq->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_user_highq->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_user_highq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_user_highq, SIGNAL(textChanged(const QString &)), SLOT(update_user_highq(const QString &)));
   
   if ( started_in_expert_mode )
   {
      pb_saxs_buffer = new QPushButton("Buffer", this);
      pb_saxs_buffer->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
      pb_saxs_buffer->setMinimumHeight(minHeight1);
      pb_saxs_buffer->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
      connect(pb_saxs_buffer, SIGNAL(clicked()), SLOT(saxs_buffer()));
   } 

   pb_guinier_analysis = new QPushButton("Guinier Analysis", this);
   pb_guinier_analysis->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_guinier_analysis->setMinimumHeight(minHeight1);
   pb_guinier_analysis->setEnabled(true);
   pb_guinier_analysis->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_guinier_analysis, SIGNAL(clicked()), SLOT(run_guinier_analysis()));

#if defined(ADD_GUINIER)      
      lbl_guinier_cutoff = new QLabel(tr("Guinier cutoff\n(1/Angstrom^2) : "), this);
      Q_CHECK_PTR(lbl_guinier_cutoff);
      lbl_guinier_cutoff->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
      lbl_guinier_cutoff->setMinimumHeight(minHeight1);
      lbl_guinier_cutoff->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
      lbl_guinier_cutoff->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

      cnt_guinier_cutoff= new QwtCounter(this);
      US_Hydrodyn::sizeArrows( cnt_guinier_cutoff );
      Q_CHECK_PTR(cnt_guinier_cutoff);
      cnt_guinier_cutoff->setRange(0.01, 100, 0.01);
      cnt_guinier_cutoff->setValue(guinier_cutoff);
      cnt_guinier_cutoff->setMinimumHeight(minHeight1);
      cnt_guinier_cutoff->setEnabled(true);
      cnt_guinier_cutoff->setNumButtons(3);
      cnt_guinier_cutoff->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      cnt_guinier_cutoff->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      connect(cnt_guinier_cutoff, SIGNAL(valueChanged(double)), SLOT(update_guinier_cutoff(double)));
#endif

   lbl_info_prr = new QLabel(tr("P(r) vs. r  Plotting Functions:"), this);
   Q_CHECK_PTR(lbl_info_prr);
   lbl_info_prr->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info_prr->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info_prr->setMinimumHeight(minHeight1);
   lbl_info_prr->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info_prr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_bin_size = new QLabel(tr(" Bin size (Angstrom): "), this);
   Q_CHECK_PTR(lbl_bin_size);
   lbl_bin_size->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_bin_size->setMinimumHeight(minHeight1);
   lbl_bin_size->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_bin_size->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_bin_size= new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_bin_size );
   Q_CHECK_PTR(cnt_bin_size);
   cnt_bin_size->setRange(0.01, 100, 0.01);
   cnt_bin_size->setValue(our_saxs_options->bin_size);
   cnt_bin_size->setMinimumHeight(minHeight1);
   cnt_bin_size->setEnabled(true);
   cnt_bin_size->setNumButtons(3);
   cnt_bin_size->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_bin_size->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_bin_size, SIGNAL(valueChanged(double)), SLOT(update_bin_size(double)));

   rb_curve_raw = new QRadioButton(tr("Raw"), this);
   rb_curve_raw->setEnabled(true);
   rb_curve_raw->setChecked(our_saxs_options->curve == 0);
   rb_curve_raw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_curve_raw->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   //   rb_curve_saxs_dry = new QRadioButton(tr("SAXS (unc)"), this);
   //   rb_curve_saxs_dry->setEnabled(true);
   //   rb_curve_saxs_dry->setChecked(our_saxs_options->curve == 1);
   //   rb_curve_saxs_dry->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   rb_curve_saxs_dry->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   rb_curve_saxs = new QRadioButton(tr("SAXS"), this);
   rb_curve_saxs->setEnabled(true);
   rb_curve_saxs->setChecked(our_saxs_options->curve == 1);
   rb_curve_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_curve_saxs->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   rb_curve_sans = new QRadioButton(tr("SANS"), this);
   rb_curve_sans->setEnabled(true);
   rb_curve_sans->setChecked(our_saxs_options->curve == 2);
   rb_curve_sans->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_curve_sans->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   bg_curve = new QButtonGroup(1, Qt::Horizontal, 0);
   bg_curve->setRadioButtonExclusive(true);
   bg_curve->insert(rb_curve_raw);
   //   bg_curve->insert(rb_curve_saxs_dry);
   bg_curve->insert(rb_curve_saxs);
   bg_curve->insert(rb_curve_sans);
   connect(bg_curve, SIGNAL(clicked(int)), SLOT(set_curve(int)));

   cb_normalize = new QCheckBox(this);
   cb_normalize->setText(tr(" Normalize"));
   cb_normalize->setEnabled(true);
   cb_normalize->setChecked(true);
   cb_normalize->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_normalize->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_pr_contrib = new QCheckBox(this);
   cb_pr_contrib->setText(tr(" Residue contrib.  range (Angstrom):"));
   cb_pr_contrib->setEnabled(true);
   cb_pr_contrib->setChecked(false);
   cb_pr_contrib->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   cb_pr_contrib->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_pr_contrib, SIGNAL(clicked()), SLOT(set_pr_contrib()));

   le_pr_contrib_low = new QLineEdit(this, "pr_contrib_low Line Edit");
   le_pr_contrib_low->setText("");
   le_pr_contrib_low->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_pr_contrib_low->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_pr_contrib_low->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_pr_contrib_low, SIGNAL(textChanged(const QString &)), SLOT(update_pr_contrib_low(const QString &)));

   le_pr_contrib_high = new QLineEdit(this, "pr_contrib_high Line Edit");
   le_pr_contrib_high->setText("");
   le_pr_contrib_high->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_pr_contrib_high->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_pr_contrib_high->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_pr_contrib_high, SIGNAL(textChanged(const QString &)), SLOT(update_pr_contrib_high(const QString &)));

   pb_pr_contrib = new QPushButton(tr("Display"), this);
   Q_CHECK_PTR(pb_pr_contrib);
   pb_pr_contrib->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_pr_contrib->setMinimumHeight(minHeight1);
   pb_pr_contrib->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_pr_contrib->setEnabled(false);
   connect(pb_pr_contrib, SIGNAL(clicked()), SLOT(show_pr_contrib()));

   pb_plot_pr = new QPushButton(tr("Compute P(r) Distribution"), this);
   Q_CHECK_PTR(pb_plot_pr);
   pb_plot_pr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_plot_pr->setMinimumHeight(minHeight1);
   pb_plot_pr->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_plot_pr, SIGNAL(clicked()), SLOT(show_plot_pr()));

   pb_load_pr = new QPushButton(tr("Load P(r) Distribution"), this);
   Q_CHECK_PTR(pb_load_pr);
   pb_load_pr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_load_pr->setMinimumHeight(minHeight1);
   pb_load_pr->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_load_pr, SIGNAL(clicked()), SLOT(load_pr()));

   pb_load_plot_pr = new QPushButton(tr("Load Plotted P(r)"), this);
   Q_CHECK_PTR(pb_load_plot_pr);
   pb_load_plot_pr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_load_plot_pr->setMinimumHeight(minHeight1);
   pb_load_plot_pr->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_load_plot_pr, SIGNAL(clicked()), SLOT(load_plot_pr()));

   pb_clear_plot_pr = new QPushButton(tr("Clear P(r) Distribution"), this);
   pb_clear_plot_pr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_clear_plot_pr->setMinimumHeight(minHeight1);
   pb_clear_plot_pr->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_clear_plot_pr, SIGNAL(clicked()), SLOT(clear_plot_pr()));

   pb_pr_legend = new QPushButton(tr("Legend"), this);
   pb_pr_legend->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_pr_legend->setMinimumHeight(minHeight1);
   pb_pr_legend->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_pr_legend, SIGNAL(clicked()), SLOT(pr_legend()));

   pb_cancel = new QPushButton(tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_stop = new QPushButton(tr("Stop"), this);
   Q_CHECK_PTR(pb_stop);
   pb_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_stop->setMinimumHeight(minHeight1);
   pb_stop->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_stop, SIGNAL(clicked()), SLOT(stop()));

   pb_options = new QPushButton(tr("Open Options Panel"), this);
   Q_CHECK_PTR(pb_options);
   pb_options->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_options->setMinimumHeight(minHeight1);
   pb_options->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_options, SIGNAL(clicked()), SLOT(options()));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   plot_saxs = new QwtPlot(this);
#ifndef QT4
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
   plot_saxs->setPalette( QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
#ifndef QT4
   plot_saxs->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot_saxs->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   grid_saxs->setMajPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid_saxs->setMinPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid_saxs->attach( plot_saxs );
#endif
   plot_saxs->setAxisTitle( QwtPlot::xBottom, cb_guinier->isChecked() ? tr( "q^2 (1/Angstrom^2)" ) : tr( "q (1/Angstrom)" ) );
   plot_saxs->setAxisTitle( QwtPlot::yLeft,   cb_kratky ->isChecked() ? tr( " q^2 * I(q)"        ) : tr( "Log10 I(q)"     ) );
#ifndef QT4
   plot_saxs->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   plot_saxs->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_saxs->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#ifndef QT4
   plot_saxs->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_saxs->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#ifndef QT4
   plot_saxs->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_saxs->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_saxs->setMargin(USglobal->config_list.margin);
   plot_saxs->setTitle("");
#ifndef QT4
   plot_saxs->setAxisOptions(QwtPlot::yLeft, 
                             cb_kratky->isChecked() ? 
                             QwtAutoScale::None :
                             QwtAutoScale::Logarithmic
                             );
#else
   plot_saxs->setAxisScaleEngine(QwtPlot::yLeft, 
                                 cb_kratky->isChecked() ?
                                 new QwtLog10ScaleEngine :  // fix this
                                 new QwtLog10ScaleEngine);
   // plot_saxs->setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);
#endif
   plot_saxs->setCanvasBackground(USglobal->global_colors.plot);
#ifndef QT4
   plot_saxs->setAutoLegend( false );
   plot_saxs->setLegendFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ) );
   connect( plot_saxs, SIGNAL( legendClicked( long ) ), SLOT( plot_saxs_clicked( long ) ) );
#else
   QwtLegend* legend_sa = new QwtLegend;
   legend_sa->setFrameStyle( QFrame::Box | QFrame::Sunken );
   plot_saxs->insertLegend( legend_sa, QwtPlot::BottomLegend );
   legend_sa->setItemMode( QwtLegend::ClickableItem );
   connect( plot_saxs, SIGNAL( legendClicked( QwtPlotItem* ) ),
                SLOT( plot_saxs_item_clicked( QwtPlotItem* ) ) );
#endif

   plot_pr = new QwtPlot(this);
#ifndef QT4
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
   plot_pr->setPalette( QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
#ifndef QT4
   plot_pr->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot_pr->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   grid_pr->setMajPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid_pr->setMinPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid_pr->attach( plot_pr );
#endif
   plot_pr->setAxisTitle(QwtPlot::xBottom, tr("Distance (Angstrom)"));
   plot_pr->setAxisTitle(QwtPlot::yLeft, tr("Frequency"));
#ifndef QT4
   plot_pr->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   plot_pr->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_pr->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#ifndef QT4
   plot_pr->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_pr->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#ifndef QT4
   plot_pr->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_pr->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_pr->setMargin(USglobal->config_list.margin);
   plot_pr->setTitle(tr("P(r) Distribution Curve"));
   plot_pr->setCanvasBackground(USglobal->global_colors.plot);

#ifndef QT4
   plot_pr->setAutoLegend( false );
   plot_pr->setLegendFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ) );
   connect( plot_pr, SIGNAL( legendClicked( long ) ), SLOT( plot_pr_clicked( long ) ) );
#else
   QwtLegend* legend_pr = new QwtLegend;
   legend_pr->setFrameStyle( QFrame::Box | QFrame::Sunken );
   plot_pr->insertLegend( legend_pr, QwtPlot::BottomLegend );
   legend_pr->setItemMode( QwtLegend::ClickableItem );
   connect( plot_pr, SIGNAL( legendClicked( QwtPlotItem* ) ),
                SLOT( plot_pr_item_clicked( QwtPlotItem* ) ) );
#endif

   progress_saxs = new QProgressBar(this, "SAXS Progress");
   progress_saxs->setMinimumHeight(minHeight1);
   progress_saxs->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   progress_saxs->reset();

   progress_pr = new QProgressBar(this, "P(r) Progress");
   progress_pr->setMinimumHeight(minHeight1);
   progress_pr->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   progress_pr->reset();

   editor = new QTextEdit(this);
   editor->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   editor->setReadOnly(true);
   editor->setMinimumWidth(300);
   editor->setMinimumHeight(minHeight1 * 6);

   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight0);

   m = new QMenuBar(frame, "menu" );
   m->setMinimumHeight(minHeight1 - 5);
   m->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   QPopupMenu * file = new QPopupMenu(editor);
   m->insertItem( tr("&File"), file );
   file->insertItem( tr("Font"),  this, SLOT(update_font()),    ALT+Key_F );
   file->insertItem( tr("Save"),  this, SLOT(save()),    ALT+Key_S );
#ifndef NO_EDITOR_PRINT
   file->insertItem( tr("Print"), this, SLOT(print()),   ALT+Key_P );
#endif
   file->insertItem( tr("Clear Display"), this, SLOT(clear_display()),   ALT+Key_X );
   editor->setWordWrap (QTextEdit::WidgetWidth);
   // editor->setWordWrap (QTextEdit::NoWrap);

   lbl_core_progress = new QLabel("", this);
   Q_CHECK_PTR(lbl_core_progress);
   lbl_core_progress->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
   lbl_core_progress->setMinimumHeight(minHeight0);
   lbl_core_progress->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_core_progress->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   int rows=13, columns = 3, spacing = 2, j=0, margin=4;
   QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);

   background->addWidget(lbl_filename1, j, 0);
   background->addWidget(te_filename2, j, 1);
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

   background->addMultiCellWidget(lbl_info, j, j, 0, 1);
   j++;

   QBoxLayout *hbl_saxs_iq = new QHBoxLayout(0);
   hbl_saxs_iq->addWidget(rb_saxs);
   hbl_saxs_iq->addWidget(rb_saxs_iq_native_debye);
   if ( started_in_expert_mode )
   {
      hbl_saxs_iq->addWidget(rb_saxs_iq_native_hybrid);
      hbl_saxs_iq->addWidget(rb_saxs_iq_native_hybrid2);
   }
   hbl_saxs_iq->addWidget(rb_saxs_iq_native_hybrid3);
   hbl_saxs_iq->addWidget(rb_saxs_iq_native_fast);
   hbl_saxs_iq->addWidget(rb_saxs_iq_foxs);
   hbl_saxs_iq->addWidget(rb_saxs_iq_crysol);
   background->addMultiCellLayout(hbl_saxs_iq, j, j, 0, 1);
   j++;

   QBoxLayout *hbl_sans_iq = new QHBoxLayout(0);
   hbl_sans_iq->addWidget(rb_sans);
   hbl_sans_iq->addWidget(rb_sans_iq_native_debye);
   if ( started_in_expert_mode )
   {
      hbl_sans_iq->addWidget(rb_sans_iq_native_hybrid);
      hbl_sans_iq->addWidget(rb_sans_iq_native_hybrid2);
   }
   hbl_sans_iq->addWidget(rb_sans_iq_native_hybrid3);
   hbl_sans_iq->addWidget(rb_sans_iq_native_fast);
   hbl_sans_iq->addWidget(rb_sans_iq_cryson);
   background->addMultiCellLayout(hbl_sans_iq, j, j, 0, 1);
   j++;
   
   QBoxLayout *hbl_iqq_suffix = new QHBoxLayout(0);
   hbl_iqq_suffix->addWidget(lbl_iqq_suffix);
   hbl_iqq_suffix->addWidget(le_iqq_manual_suffix);
   hbl_iqq_suffix->addWidget(le_iqq_full_suffix);
   background->addMultiCellLayout(hbl_iqq_suffix, j, j, 0, 1);
   j++;

   background->addWidget(pb_plot_saxs_sans, j, 0);
   background->addWidget(progress_saxs, j, 1);
   j++;

   QBoxLayout *hbl_load_saxs = new QHBoxLayout(0);
   hbl_load_saxs->addWidget(pb_load_saxs_sans);
   hbl_load_saxs->addWidget(pb_load_plot_saxs);
   hbl_load_saxs->addWidget(pb_set_grid);
   hbl_load_saxs->addWidget(pb_clear_plot_saxs);
   hbl_load_saxs->addWidget(pb_saxs_legend);
   background->addMultiCellLayout(hbl_load_saxs, j, j, 0, 1);
   j++;

   QBoxLayout *hbl_various_0 = new QHBoxLayout(0);
   hbl_various_0->addWidget(pb_load_gnom);
   hbl_various_0->addWidget(pb_saxs_search);
   if ( started_in_expert_mode )
   {
      hbl_various_0->addWidget(pb_saxs_screen);
      hbl_various_0->addWidget(pb_saxs_buffer);
   }

   hbl_various_0->addWidget(pb_guinier_analysis);
   background->addMultiCellLayout(hbl_various_0, j, j, 0, 1);
   j++;

   QHBoxLayout *hbl_tools = new QHBoxLayout;
   hbl_tools->addWidget(cb_create_native_saxs);
   background->addMultiCellLayout(hbl_tools, j, j, 0, 1);
   j++;

   background->addWidget(cb_guinier, j, 0);
   QHBoxLayout *hbl_guinier = new QHBoxLayout;
   hbl_guinier->addWidget(le_guinier_lowq2);
   hbl_guinier->addWidget(le_guinier_highq2);
   background->addLayout(hbl_guinier, j, 1);
   j++;

#if defined(ADD_GUINIER)      
   hbl_guinier->addWidget(lbl_guinier_cutoff);
   hbl_guinier->addWidget(cnt_guinier_cutoff);
#endif

   QHBoxLayout *hbl_cb_std_kratky = new QHBoxLayout;
   hbl_cb_std_kratky->addWidget( cb_user_range );
   hbl_cb_std_kratky->addWidget( cb_kratky );
   background->addLayout( hbl_cb_std_kratky, j, 0 );
   QHBoxLayout *hbl_user_range = new QHBoxLayout;
   hbl_user_range->addWidget(le_user_lowq);
   hbl_user_range->addWidget(le_user_highq);
   background->addLayout(hbl_user_range, j, 1);
   j++;

   background->addMultiCellWidget(lbl_info_prr, j, j, 0, 1);
   j++;
   background->addWidget(lbl_bin_size, j, 0);
   background->addWidget(cnt_bin_size, j, 1);
   j++;
   QBoxLayout *bl = new QHBoxLayout(0);
   bl->addWidget(rb_curve_raw);
   //   bl->addWidget(rb_curve_saxs_dry);
   bl->addWidget(rb_curve_saxs);
   bl->addWidget(rb_curve_sans);
   bl->addWidget(cb_normalize);
   background->addMultiCellLayout(bl, j, j, 0, 1);
   j++;
   
   QBoxLayout *hbl_contrib = new QHBoxLayout(0);
   hbl_contrib->addWidget(cb_pr_contrib);
   hbl_contrib->addWidget(le_pr_contrib_low);
   hbl_contrib->addWidget(le_pr_contrib_high);
   hbl_contrib->addWidget(pb_pr_contrib);

   background->addMultiCellLayout(hbl_contrib,j,j,0,1);
   j++;

   background->addWidget(pb_plot_pr, j, 0);
   background->addWidget(progress_pr, j, 1);
   j++;
   //   background->addWidget(pb_load_pr, j, 0);
   // background->addWidget(pb_clear_plot_pr, j, 1);

   QBoxLayout *hbl_plot_pr = new QHBoxLayout(0);
   hbl_plot_pr->addWidget(pb_load_pr);
   hbl_plot_pr->addWidget(pb_load_plot_pr);
   hbl_plot_pr->addWidget(pb_clear_plot_pr);
   hbl_plot_pr->addWidget(pb_pr_legend);
   background->addMultiCellLayout(hbl_plot_pr, j, j, 0, 1);
   j++;
   
   background->addMultiCellWidget(frame, j, j, 0, 1);
   j++;
   background->addMultiCellWidget(editor, j, j, 0, 1);
   //   background->addMultiCellWidget(editor, j, j+3, 0, 1);
   // background->addMultiCellWidget(plot_pr, j-2, j+3, 2, 2);
   // background->addMultiCellWidget(plot_pr, j, j, 2, 2);
   j++;
   background->addWidget(pb_stop, j, 0);
   background->addWidget(pb_options, j, 1);
   j++;
   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_cancel, j, 1);
   // background->addWidget(lbl_core_progress, j, 2);

   QVBoxLayout *vbl = new QVBoxLayout(0);
   vbl->addWidget(plot_saxs);
   vbl->addSpacing(4);
   vbl->addWidget(plot_pr);
   vbl->addWidget(lbl_core_progress);
   background->addMultiCellLayout(vbl, 0, j, 2, 2);

   background->setColSpacing(2, 600);
   //   for ( int j = 0; j < 15; j++ )
   //   {
   //      background->setRowStretch(j, 0);
   //   }
   //   background->setRowStretch(13, 1);

   background->setColStretch(0, 1);
   background->setColStretch(1, 2);
   background->setColStretch(2, 10);
   update_saxs_sans();
   clear_plot_saxs();
   clear_plot_pr();
   if ( source )
   {
      our_saxs_options->curve = 0;
      rb_curve_raw->setChecked(true);
      rb_curve_saxs->setChecked(false);
      rb_curve_sans->setChecked(false);
      rb_curve_raw->setEnabled(false);
      rb_curve_saxs->setEnabled(false);
      rb_curve_sans->setEnabled(false);
   } else {
      rb_curve_raw->setEnabled(true);
      rb_curve_saxs->setEnabled(true);
      rb_curve_sans->setEnabled(true);
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
   online_help->show_help("manual/somo_saxs.html");
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
                                      QProgressBar *progress,
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
         progress->setTotalSteps((int)(1.15f * as1 / threads));
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
            progress->setProgress(i+1);
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
      editor->append("Plot contributions: Nothing to plot\n");
      return;
   }

   if ( pr_contrib_low >= pr_contrib_high ) 
   {
      editor->append("Plot contributions: Range error\n");
      return;
   }
   progress_pr->reset();
   progress_pr->setTotalSteps(3);
   progress_pr->setProgress(0);
   // sum up all the atoms that contrib:
   cout << "trying to sum up\n";
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
   progress_pr->setProgress(1);

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
   progress_pr->setProgress(2);
   // divide all by max to put in range of [0,1]
   for ( map < QString, double >::iterator it = contrib_sums.begin();
         it != contrib_sums.end();
         it++ )
   {
      contrib_sums[it->first] /= max;
   }
   progress_pr->setProgress(3);

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

   QString out = QString("load %1\nselect all\ncolor gray\n").arg(contrib_file);
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
   QString fname = 
      ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "tmp" + SLASH + QFileInfo(contrib_file).baseName() + ".spt";
   QFile f(fname);
   if ( !f.open( IO_WriteOnly ) )
   {
      editor->append("Error creating file " + fname + "\n");
      return;
   }
   QTextStream t( &f );
   t << out;
   f.close();
   QStringList argument;
#if !defined(WIN32) && !defined(MAC)
   argument.append("xterm");
   argument.append("-e");
#endif
#if defined(BIN64)
   argument.append(USglobal->config_list.system_dir + "/bin64/rasmol");
#else
   argument.append(USglobal->config_list.system_dir + "/bin/rasmol");
#endif
   argument.append("-script");
   argument.append(fname);
   rasmol = new QProcess(this);
   rasmol->setWorkingDirectory(((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "tmp");
   rasmol->setArguments(argument);
   if (!rasmol->start())
   {
      QMessageBox::message(tr("Please note:"), tr("There was a problem starting RASMOL\n"
                                                  "Please check to make sure RASMOL is properly installed..."));
      return;
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
   pb_pr_contrib->setEnabled(false);
   stopFlag = false;
   pb_stop->setEnabled(true);
   pb_plot_pr->setEnabled(false);
   pb_plot_saxs_sans->setEnabled(false);
   progress_pr->reset();
   vector < float > hist;
   float delta = our_saxs_options->bin_size;

#if defined(BUG_DEBUG)
   qApp->processEvents();
   cout << " sleep 1 a" << endl;
   sleep(1);
   cout << " sleep 1 a done" << endl;
#endif
   
   for ( unsigned int i = 0; i < selected_models.size(); i++ )
   {
      current_model = selected_models[i];
#if defined(PR_DEBUG)
      printf("creating pr %u\n", current_model); fflush(stdout);
#endif
      editor->append(QString("\n\nPreparing file %1 model %2 for p(r) vs r plot in %3 mode%4.\n\n")
                     .arg(te_filename2->text())
                     .arg(current_model + 1)
                     .arg(rb_curve_raw->isChecked() ? "Raw" :
                          ( rb_curve_saxs->isChecked() ? "SAXS" : "SANS" ) )
                     .arg(cb_normalize->isChecked() ? ", Normalized" : "")
                     );
      qApp->processEvents();
      if ( stopFlag ) 
      {
         editor->append(tr("Terminated by user request.\n"));
         progress_pr->reset();
         lbl_core_progress->setText("");
         pb_plot_saxs_sans->setEnabled(bead_model_ok_for_saxs);
         // pb_plot_saxs_sans->setEnabled(true);
         pb_plot_pr->setEnabled(true);
         return;
      }
         
      vector < saxs_atom > atoms;
      saxs_atom new_atom;

      float b_bar = 0.0;
      float b_bar_inv2 = 0.0;
      int b_count = 0;

      contrib_pdb_atom.clear();

      if ( source )
      {
         // bead models
         for (unsigned int j = 0; j < bead_models[current_model].size(); j++)
         {
            PDB_atom *this_atom = &(bead_models[current_model][j]);
            new_atom.pos[0] = this_atom->bead_coordinate.axis[0];
            new_atom.pos[1] = this_atom->bead_coordinate.axis[1];
            new_atom.pos[2] = this_atom->bead_coordinate.axis[2];
            atoms.push_back(new_atom);
            contrib_pdb_atom.push_back(this_atom);
         }
      }
      else 
      {
         // compute b[0] based upon current values
         map < QString, float > b;
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
                      it->first.ascii(),
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
                         QString(it->first + "-aa").ascii(),
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
            // QMessageBox::message(tr("No Hydrogens:"), tr("No Hydrogen defined in SAXS Atom table"));
            // return;
            // }
            // float excl_volH = saxs_map["H"].volume;
            for (map < QString, hybridization >::iterator it = hybrid_map.begin();
                 it != hybrid_map.end();
                 it++)
            {
               // cout << " computing b for " << it->first 
               // << " hydrogens " << it->second.hydrogens 
               // << " exch_prot " << it->second.exch_prot
               // << " num elect " << it->second.num_elect
               // << " saxs_name " << it->second.saxs_name
               // << " saxs_excl_vol noH " << saxs_map[it->second.saxs_name].volume
               // // << " saxs_excl_vol  " << ( saxs_map[it->second.saxs_name].volume + it->second.hydrogens * excl_volH )
               // << endl;
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
                  if ( this_atom->name == "OXT" )
                  {
                     mapkey = "OXT|OXT";
                  }
                  QString hybrid_name = residue_atom_hybrid_map[mapkey];
                  if ( this_atom->name == "N" &&
                       hybrid_name == "N3H1" &&
                       this_atom->p_residue->type == 0 )
                  {
                     hybrid_name += "-aa";
                  }
                  new_atom.b = b[hybrid_name];
                  b_count++;
                  b_bar += new_atom.b;
#if defined(BUG_DEBUG)
                  printf("atom %d %d hybrid name %s, atom name %s b %e mapkey %s hybrid name %s\n",
                         j, k, 
                         hybrid_name.ascii(),
                         this_atom->name.ascii(),
                         new_atom.b,
                         mapkey.ascii(),
                         hybrid_name.ascii()
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
                  QString hybrid_name = residue_atom_hybrid_map[mapkey];
                  double radius = 0e0;
                  if ( rb_curve_saxs->isChecked() )
                  {
                     if ( !hybrid_map.count(hybrid_name) )
                     {
                        cout << "error: hybrid_map name missing for hybrid_name " << hybrid_name << endl;
                        QColor save_color = editor->color();
                        editor->setColor("red");
                        editor->append(QString("%1Molecule %2 Residue %3 %4 Hybrid %5 name missing from Hybrid file. Assuming zero radius!.\n")
                                       .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                                       .arg(j+1)
                                       .arg(this_atom->resName)
                                       .arg(this_atom->resSeq)
                                       .arg(hybrid_name)
                                       );
                        editor->setColor(save_color);
                        qApp->processEvents();
                     } else {
                        radius = hybrid_map[hybrid_name].radius;
                     }
                  }
                  if ( !atom_map.count( this_atom->name + "~" + hybrid_name ) )
                  {
                     QMessageBox::message( tr("Missing Atom:"), 
                                           QString( tr("Atom %1 not defined") ).arg( this_atom->name ) );
                     return;
                  }
                  // cout << "atom " << this_atom->name 
                  // << " hybrid " << this_atom->hybrid_name
                  // << " excl_vol " <<  atom_map[this_atom->name + "~" + hybrid_name].saxs_excl_vol 
                  // << " radius cubed " << radius * radius * radius 
                  // << " pi radius cubed " << M_PI * radius * radius * radius 
                  // << endl;
                  new_atom.b = b[hybrid_name] - our_saxs_options->water_e_density * atom_map[this_atom->name + "~" + hybrid_name].saxs_excl_vol;
                  b_count++;
                  b_bar += new_atom.b;
#if defined(BUG_DEBUG)
                  printf("atom %d %d hybrid name %s, atom name %s b %e correction %e mapkey %s hybrid name %s\n",
                         j, k, 
                         hybrid_name.ascii(),
                         this_atom->name.ascii(),
                         new_atom.b,
                         our_saxs_options->water_e_density * radius * radius * radius,
                         mapkey.ascii(),
                         hybrid_name.ascii()
                         );
#endif
               }
               atoms.push_back(new_atom);
               contrib_pdb_atom.push_back(this_atom);
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
               QColor save_color = editor->color();
               editor->setColor("red");
               editor->append(tr("WARNING: < b > is zero! Reverting to RAW mode for p(r) vs r computation."));
               editor->setColor(save_color);
            }
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

      editor->append(QString("Number of atoms %1. Bin size %2.\n")
                     .arg(atoms.size())
                     .arg(delta));
      qApp->processEvents();

      // we want to keep a tally of each atom's contribution to each position
      // contrib.clear();
      contrib_array.clear();
      contrib_delta = delta;
      // contrib_file = te_filename2->text();
      contrib_file = ((US_Hydrodyn *)us_hydrodyn)->pdb_file;
      cout << "contrib_file " << contrib_file << endl;

      // restore threading later
      if ( 0 && USglobal->config_list.numThreads > 1 )
      {
         // threaded
         
         unsigned int j;
         unsigned int threads = USglobal->config_list.numThreads;
         editor->append(QString("Using %1 threads.\n").arg(threads));
         vector < saxs_pr_thr_t* > saxs_pr_thr_threads(threads);
         for ( j = 0; j < threads; j++ )
         {
            saxs_pr_thr_threads[j] = new saxs_pr_thr_t(j);
            saxs_pr_thr_threads[j]->start();
         }
         vector < vector < float > > hists;
         hists.resize(threads);
         for ( j = 0; j < threads; j++ )
         {
# if defined(DEBUG_THREAD)
            cout << "thread " << j << endl;
# endif            
            saxs_pr_thr_threads[j]->saxs_pr_thr_setup(
                                                      &atoms,
                                                      &hists[j],
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
#if !defined(WIN32)
            timespec ns;
            timespec ns_ret;
            ns.tv_sec = 0;
            ns.tv_nsec = 50000000l;
#endif
            
            do {
               all_done = threads;
               for ( j = 0; j < threads; j++ )
               {
                  all_done -= saxs_pr_thr_threads[j]->saxs_pr_thr_work_status();
               }
               qApp->processEvents();
#if defined(WIN32)
               _sleep(1);
#else
               nanosleep(&ns, &ns_ret);
#endif
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

      } // end threaded
      else
      {
#if defined(BUG_DEBUG)
         cout << "non-threaded run\n";
#endif
         // non-threaded
         float rik; 
         unsigned int pos;
         progress_pr->setTotalSteps((int)(atoms.size()));
         if ( cb_pr_contrib->isChecked() &&
              !source &&
              contrib_file.contains(QRegExp("(PDB|pdb)$")) )
         {
            // contrib version
            contrib_array.resize(atoms.size());
            for ( unsigned int i = 0; i < atoms.size() - 1; i++ )
            {
               progress_pr->setProgress(i+1);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(tr("Terminated by user request.\n"));
                  progress_pr->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(bead_model_ok_for_saxs);
                  // pb_plot_saxs_sans->setEnabled(false);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
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
                     double this_pr = atoms[i].b * atoms[j].b * b_bar_inv2;
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
            pb_pr_contrib->setEnabled(true);
         } else {
            // non contrib version:
            for ( unsigned int i = 0; i < atoms.size() - 1; i++ )
            {
               progress_pr->setProgress(i+1);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(tr("Terminated by user request.\n"));
                  progress_pr->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(bead_model_ok_for_saxs);
                  // pb_plot_saxs_sans->setEnabled(false);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
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
                  pos = (unsigned int)floor(rik / delta);
                  if ( hist.size() <= pos )
                  {
                     hist.resize(pos + 128);
                  }
                  if ( rb_curve_raw->isChecked() )
                  {
                     hist[pos]++;
                  } else {
                     // good for both saxs & sans
                     hist[pos] += atoms[i].b * atoms[j].b * b_bar_inv2;
                  }
               }
            }
         }
      } // end non-threaded
         
#if defined(BUG_DEBUG)
      cout << "contrib info:\n";
      for ( map < QString, float >::iterator it = contrib.begin();
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
      while( hist.size() && !hist[hist.size()-1] ) 
      {
         hist.pop_back();
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

      // save the data to a file
      if ( create_native_saxs )
      {
         QString fpr_name = 
            USglobal->config_list.root_dir + 
            SLASH + "somo" + SLASH + "saxs" + SLASH + sprr_filestring();
         
         bool ok_to_write = true;
         if ( QFile::exists(fpr_name) &&
              !((US_Hydrodyn *)us_hydrodyn)->overwrite ) 
         {

            fpr_name = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fpr_name, 0, this);
            ok_to_write = true;
#if defined(OLD_WAY)

            switch( QMessageBox::information( this, 
                                              tr("Overwrite file:") + "SAXS P(r) vs. r" + tr("output file"),
                                              tr("The P(r) curve file \"") + 
                                              sprr_filestring() +
                                              tr("\" will be overwriten"),
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

         if ( ok_to_write )
         {
            FILE *fpr = fopen(fpr_name, "w");
            if ( fpr ) 
            {
               editor->append(tr("P(r) curve file: ") + fpr_name + tr(" created.\n"));
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_mw = get_mw(te_filename2->text(), false);
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear();
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
                           , model_filename.ascii()
                           , US_Version.ascii()
                           , REVISION
                           , delta
                           , get_mw(te_filename2->text(), false)
                           , compute_pr_area(pr, r)
                           );
               fprintf(fpr, "%s",
                       ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header.ascii() );
               fprintf(fpr, "r\tp(r)\tnorm. p(r)\n");
               for ( unsigned int i = 0; i < hist.size(); i++ )
               {
                  if ( hist[i] ) {
                     fprintf(fpr, "%.6e\t%.6e\t%.6e\n", r[i], pr[i], pr_n[i]);
                     ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.push_back(r[i]);
                     ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.push_back(pr[i]);
                     ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.push_back(pr_n[i]);
                  }
               }
               fclose(fpr);
            }
            else
            {
#if defined(PR_DEBUG)
               cout << "can't create " << fpr_name << endl;
#endif
               editor->append(tr("WARNING: Could not create PR curve file: ") + fpr_name + "\n");
               QMessageBox mb(tr("UltraScan Warning"),
                              tr("The output file ") + fpr_name + tr(" could not be created."), 
                              QMessageBox::Critical,
                              QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
               mb.exec();
            }
         }
      } else {
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_mw = get_mw(te_filename2->text(), false);
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear();
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
                     , model_filename.ascii()
                     , US_Version.ascii()
                     , REVISION
                     , delta
                     , get_mw(te_filename2->text(), false)
                     , compute_pr_area(pr, r)
                     );
         for ( unsigned int i = 0; i < hist.size(); i++ )
         {
            if ( hist[i] ) {
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.push_back(r[i]);
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.push_back(pr[i]);
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.push_back(pr_n[i]);
            }
         }
      }
   } // models

   vector < double > r;
   vector < double > pr;
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
   plotted_pr_not_normalized.push_back(pr);
   plotted_pr_mw.push_back((float)get_mw(te_filename2->text()));
   if ( cb_normalize->isChecked() )
   {
      normalize_pr(r, &pr, get_mw(te_filename2->text(),false));
   }

   plotted_r.push_back(r);
   plotted_pr.push_back(pr);
   QString use_name = QFileInfo(model_filename).fileName();
   QString plot_name = use_name;
   int extension = 0;
   while ( dup_plotted_pr_name_check.count(plot_name) )
   {
      plot_name = QString("%1-%2").arg(use_name).arg(++extension);
   }
   qsl_plotted_pr_names << plot_name;
   dup_plotted_pr_name_check[plot_name] = true;
   unsigned int p = plotted_r.size() - 1;

#ifndef QT4
   long ppr = plot_pr->insertCurve( plot_name );
#else
   QwtPlotCurve *curve = new QwtPlotCurve( plot_name );
#endif
#ifndef QT4
   plot_pr->setCurveStyle(ppr, QwtCurve::Lines);
#else
   curve->setStyle( QwtPlotCurve::Lines );
#endif

#ifndef QT4
   plot_pr->setCurveData(ppr, (double *)&(r[0]), (double *)&(pr[0]), (int)r.size());
   plot_pr->setCurvePen(ppr, QPen(plot_colors[p % plot_colors.size()], 2, SolidLine));
#else
   curve->setData(
                  (double *)&( r[ 0 ] ), 
                  (double *)&( pr[ 0 ] ),
                  (int)r.size()
                  );
   curve->setPen( QPen( plot_colors[ p % plot_colors.size() ], 2, Qt::SolidLine ) );
   curve->attach( plot_pr );
#endif

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
#ifndef QT4
   plot_pr_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
   plot_pr_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#else
   plot_pr_zoomer->setRubberBandPen( QPen(Qt::red, 1, Qt::DotLine ) );
   plot_pr_zoomer->setTrackerPen( QPen( Qt::red ) );
#endif

   plot_pr->replot();

   progress_pr->setTotalSteps(1);
   progress_pr->setProgress(1);
   pb_plot_saxs_sans->setEnabled(bead_model_ok_for_saxs);
   // pb_plot_saxs_sans->setEnabled(false);
   pb_plot_pr->setEnabled(true);

   QColor save_color = editor->color();
   editor->setParagraphBackgroundColor ( editor->paragraphs() - 1, QColor("dark gray") );
   editor->setColor(plot_colors[p % plot_colors.size()]);
   editor->append(QString("P(r): Bin size: %1 \"%2\"\n").arg(delta).arg(QFileInfo(model_filename).fileName()));
   editor->setColor(save_color);
   editor->setParagraphBackgroundColor ( editor->paragraphs() - 1, QColor("white") );
}


void US_Hydrodyn_Saxs::load_plot_pr()
{
   load_pr(true);
}

void US_Hydrodyn_Saxs::clear_plot_pr()
{
   plotted_pr.clear();
   plotted_pr_not_normalized.clear();
   plotted_pr_mw.clear();
   plotted_r.clear();
   qsl_plotted_pr_names.clear();
   dup_plotted_pr_name_check.clear();
   plot_pr->clear();
   plot_pr->replot();
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
                                      QProgressBar *progress,
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
         progress->setTotalSteps((int)(1.15f * as1 / threads));
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
            progress->setProgress(i+1);
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

   if ( our_saxs_options->iqq_ask_target_grid &&
        plotted_q.size() )
   {
      ask_iq_target_grid();
   }

   if ( our_saxs_options->swh_excl_vol )
   {
      editor_msg("dark red", QString("SWH set to %1\n").arg( our_saxs_options->swh_excl_vol ));
   }

   if ( !source && our_saxs_options->saxs_iq_foxs ) 
   {
      // cout << model_filepathname << endl;
      run_saxs_iq_foxs( model_filepathname );
      return;
   }
   if ( !source && our_saxs_options->saxs_iq_crysol ) 
   {
      // cout << model_filepathname << endl;
      run_saxs_iq_crysol( model_filepathname );
      return;
   }
   if ( our_saxs_options->saxs_iq_native_fast ) 
   {
      // calc_saxs_iq_native_debye();
      source ? calc_saxs_iq_native_fast_bead_model() : calc_saxs_iq_native_fast();
      return;
   }
   if ( our_saxs_options->saxs_iq_native_debye ) 
   {
      source ? calc_saxs_iq_native_debye_bead_model() : calc_saxs_iq_native_debye();
      return;
   }
   if ( our_saxs_options->saxs_iq_native_hybrid ||
        our_saxs_options->saxs_iq_native_hybrid2 ||
        our_saxs_options->saxs_iq_native_hybrid3 ) 
   {
      source ? calc_saxs_iq_native_hybrid2_bead_model() : calc_saxs_iq_native_hybrid2();
      return;
   }
   QMessageBox::information(this, 
                            tr("Method not supported:"), 
                            QString(tr("The selected method is not supported for this model")));
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
         editor->append(tr("Terminated by user request.\n"));
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
               QColor save_color = editor->color();
               editor->setColor("red");
               editor->append(QString("%1Molecule %2 Residue %3 %4 Hybrid name missing. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq));
               editor->setColor(save_color);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(tr("Terminated by user request.\n"));
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
               QColor save_color = editor->color();
               editor->setColor("red");
               editor->append(QString("%1Molecule %2 Residue %3 %4 Hybrid %5 name missing from Hybrid file. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq)
                              .arg(hybrid_name)
                              );
               editor->setColor(save_color);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
               continue;
            }

            if ( !atom_map.count(this_atom->name + "~" + hybrid_name) )
            {
               cout << "error: atom_map missing for hybrid_name "
                    << hybrid_name 
                    << " atom name "
                    << this_atom->name
                    << endl;
               QColor save_color = editor->color();
               editor->setColor("red");
               editor->append(QString("%1Molecule %2 Atom %3 Residue %4 %5 Hybrid %6 name missing from Atom file. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->name)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq)
                              .arg(hybrid_name)
                              );
               editor->setColor(save_color);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
               continue;
            }

            new_atom.excl_vol = atom_map[this_atom->name + "~" + hybrid_name].saxs_excl_vol;

            new_atom.saxs_name = hybrid_map[hybrid_name].saxs_name; 

            if ( !saxs_map.count(hybrid_map[hybrid_name].saxs_name) )
            {
               cout << "error: saxs_map missing for hybrid_name "
                    << hybrid_name 
                    << " saxs name "
                    << hybrid_map[hybrid_name].saxs_name
                    << endl;
               QColor save_color = editor->color();
               editor->setColor("red");
               editor->append(QString("%1Molecule %2 Residue %3 %4 Hybrid %5 Saxs name %6 name missing from SAXS atom file. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq)
                              .arg(hybrid_name)
                              .arg(hybrid_map[hybrid_name].saxs_name)
                              );
               editor->setColor(save_color);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(tr("Terminated by user request.\n"));
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

      FILE *fsaxs_atoms = fopen(fsaxs_atoms_name, "w");
      if ( fsaxs_atoms ) 
      {
         for ( unsigned int i = 0; i < atoms.size(); i++ )
         {
            fprintf(fsaxs_atoms, "%s %.3f %.3f %.3f %.2f\n"
                    , atoms[i].saxs_name.ascii()
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
         editor->append(tr("Terminated by user request.\n"));
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
         editor->append(tr("Terminated by user request.\n"));
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
#if !defined(WIN32)
            timespec ns;
            timespec ns_ret;
            ns.tv_sec = 0;
            ns.tv_nsec = 500000000l;
#endif
            do {
               all_done = threads;
               for ( j = 0; j < threads; j++ )
               {
                  all_done -= saxs_Iq_thr_threads[j]->saxs_Iq_thr_work_status();
               }
               qApp->processEvents();
#if defined(WIN32)
               _sleep(1);
#else
               nanosleep(&ns, &ns_ret);
#endif
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
            editor->append(tr("Terminated by user request.\n"));
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
         progress_saxs->setTotalSteps((int)(as1 * 1.15));
         for ( unsigned int i = 0; i < as1; i++ )
         {
            // QString lcp = QString("Atom %1 of %2").arg(i+1).arg(as);
            // cout << lcp << endl;
            // lbl_core_progress->setText(lcp);
            progress_saxs->setProgress(i+1);
            qApp->processEvents();
            if ( stopFlag ) 
            {
               editor->append(tr("Terminated by user request.\n"));
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
#ifndef QT4
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

#ifndef QT4
      plotted_iq_names_to_pos[plot_name] = plotted_Iq.size();
#else
      plotted_iq_names_to_pos[plot_name] = plotted_Iq_curves.size();
#endif

#ifndef QT4
      plotted_Iq.push_back(Iq);
      plot_saxs->setCurveStyle(Iq, QwtCurve::Lines);
#else
      plotted_Iq_curves.push_back( curve );
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
#ifndef QT4
      plot_saxs->setCurveData(Iq, 
                              cb_guinier->isChecked() ?
                              (double *)&(plotted_q2[p][0]) : (double *)&(plotted_q[p][0]), 
                              (double *)&(plotted_I[p][0]), q_points);
      plot_saxs->setCurvePen(Iq, QPen(plot_colors[p % plot_colors.size()], 2, SolidLine));
#else
      curve->setData(
                     cb_guinier->isChecked() ?
                     (double *)&(plotted_q2[p][0]) : (double *)&(plotted_q[p][0]), 
                     (double *)&(plotted_I[p][0]),
                     q_points
                     );
      curve->setPen( QPen( plot_colors[ p % plot_colors.size() ], 2, Qt::SolidLine ) );
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
                                              tr("Overwrite file:") + "SAXS I(q) vs. q" + tr("output file"),
                                              tr("The file named \"") + 
                                              saxs_filestring() +
                                              + tr("\" will be overwriten"),
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
            FILE *fsaxs = fopen(fsaxs_name, "w");
            if ( fsaxs ) 
            {
#if defined(SAXS_DEBUG)
               cout << "writing " << fsaxs_name << endl;
#endif
               editor->append(tr("SAXS curve file: ") + fsaxs_name + tr(" created.\n"));
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header =
                  QString("")
                  .sprintf(
                           "Simulated SAXS data generated from %s by US_SOMO %s %s q(%.3f:%.3f) step %.3f\n"
                           , model_filename.ascii()
                           , US_Version.ascii()
                           , REVISION
                           , our_saxs_options->start_q
                           , our_saxs_options->end_q
                           , our_saxs_options->delta_q
                           );
               fprintf(fsaxs, "%s",
                       ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header.ascii() );
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
               editor->append(tr("WARNING: Could not create SAXS curve file: ") + fsaxs_name + "\n");
               QMessageBox mb(tr("UltraScan Warning"),
                              tr("The output file ") + fsaxs_name + tr(" could not be created."), 
                              QMessageBox::Critical,
                              QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
               mb.exec();
            }
         }
      } else {
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header =
            QString("")
            .sprintf(
                     "Simulated SAXS data generated from %s by US_SOMO %s %s q(%.3f:%.3f) step %.3f\n"
                     , model_filename.ascii()
                     , US_Version.ascii()
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
      QPaintDeviceMetrics metrics( &printer ); // need width/height
      // of printer surface
      for( int i = 0 ; i < editor->lines() ; i++ ) {
         if ( MARGIN + yPos > metrics.height() - MARGIN ) {
            printer.newPage();      // no more room on this page
            yPos = 0;         // back to top of page
         }
         p.drawText( MARGIN, MARGIN + yPos,
                     metrics.width(), fm.lineSpacing(),
                                   ExpandTabs | DontClip,
                                   editor->text( i ) );
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
                                  tr("US-SOMO: Set grid"),
                                  QString(tr("NOTE: Everything plotted already matches the set grid")));
      } else {
         QMessageBox::information(this, 
                                  tr("US-SOMO: Set grid"),
                                  QString(tr("Nothing plotted")));
         return;
      }
   }
   ask_iq_target_grid( true );
}

void US_Hydrodyn_Saxs::clear_plot_saxs_data()
{
   qsl_plotted_iq_names.clear();
   dup_plotted_iq_name_check.clear();
   plotted_iq_names_to_pos.clear();
#ifndef QT4
   plotted_Iq.clear();
#else
   plotted_Iq_curves.clear();
#endif
   plotted_q.clear();
   plotted_q2.clear();
   plotted_I.clear();
   plotted_I_error.clear();
   plot_saxs->clear();
   plot_saxs->replot();
#ifndef QT4
   plotted_Gp.clear();
#else
   plotted_Gp_curves.clear();
#endif
   plotted_guinier_valid.clear();
   plotted_guinier_plotted.clear();
   plotted_guinier_lowq2.clear();
   plotted_guinier_highq2.clear();
   plotted_guinier_a.clear();
   plotted_guinier_b.clear();
   plotted_guinier_x.clear();
   plotted_guinier_y.clear();
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
      editor->setParagraphBackgroundColor ( editor->paragraphs() - 1, QColor("white") );
      editor->append("I(q) plot done\n");
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
                                        tr("Close I(q) vs q residual windows"),
                                        tr("Do you want to close the I(q) vs q residual window(s)?\n"
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
#ifndef QT4
   if ( ((US_Hydrodyn*)us_hydrodyn)->saxs_screen_widget )
   {
      ((US_Hydrodyn*)us_hydrodyn)->saxs_screen_window->update_enables();
   }
#endif
}

void US_Hydrodyn_Saxs::show_plot_sans()
{
   if ( our_saxs_options->sans_iq_cryson ) 
   {
      // cout << model_filepathname << endl;
      run_sans_iq_cryson( model_filepathname );
      return;
   }
   QMessageBox::information(this, 
                            tr("Method not implemented:"), 
                            QString(tr("The selected method is not yet implemented.")));
}

void US_Hydrodyn_Saxs::show_plot_saxs_sans()
{
   rb_sans->isChecked() ? show_plot_sans() : show_plot_saxs();
   saxs_search_update_enables();
}

void US_Hydrodyn_Saxs::load_saxs_sans()
{
   rb_sans->isChecked() ? load_sans() : load_saxs();
}
      
void US_Hydrodyn_Saxs::update_saxs_sans()
{
   set_current_method_buttons();
   if ( rb_sans->isChecked() ) 
   {
      pb_plot_saxs_sans->setText(tr("Compute SANS Curve"));
      pb_load_saxs_sans->setText(tr("Load SANS Curve"));
      pb_clear_plot_saxs->setText(tr("Clear SANS Curve"));
      plot_saxs->setTitle((cb_guinier->isChecked() ? "Guinier " : "") + tr("SANS Curve"));

      rb_saxs_iq_native_debye->setEnabled(false);
      rb_saxs_iq_native_fast ->setEnabled(false);
      rb_saxs_iq_foxs        ->setEnabled(false);
      rb_saxs_iq_crysol      ->setEnabled(false);

      rb_sans_iq_native_debye->setEnabled(true);
      rb_sans_iq_native_fast ->setEnabled(true);
      rb_sans_iq_cryson      ->setEnabled(true);
   } else {
      pb_plot_saxs_sans->setText(tr("Compute SAXS Curve"));
      pb_load_saxs_sans->setText(tr("Load SAXS Curve"));
      pb_clear_plot_saxs->setText(tr("Clear SAXS Curve"));
      plot_saxs->setTitle((cb_guinier->isChecked() ? "Guinier " : "") + tr("SAXS Curve"));

      rb_saxs_iq_native_debye->setEnabled(true);
      rb_saxs_iq_native_fast ->setEnabled(true);
      rb_saxs_iq_foxs        ->setEnabled(true);
      rb_saxs_iq_crysol      ->setEnabled(true);

      rb_sans_iq_native_debye->setEnabled(false);
      rb_sans_iq_native_fast ->setEnabled(false);
      rb_sans_iq_cryson      ->setEnabled(false);
   }
   update_iqq_suffix();
}

void US_Hydrodyn_Saxs::load_sans( QString, bool )
{
}

void US_Hydrodyn_Saxs::clear_display()
{
   editor->clear();
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

void US_Hydrodyn_Saxs::select_atom_file()
{
   QString old_filename = atom_filename;
   atom_filename = QFileDialog::getOpenFileName(USglobal->config_list.system_dir + SLASH + "etc", "*.atom *.ATOM", this);
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
   lbl_atom_table->setText(fi.baseName() + "." + fi.extension());
   atom_list.clear();
   atom_map.clear();
   QFile f(filename);
   if (f.open(IO_ReadOnly|IO_Translate))
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
}

void US_Hydrodyn_Saxs::select_hybrid_file()
{
   QString old_filename = hybrid_filename;
   hybrid_filename = QFileDialog::getOpenFileName(USglobal->config_list.system_dir + SLASH + "etc", 
                                                  "*.hybrid *.HYBRID", this);
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
   lbl_hybrid_table->setText(fi.baseName() + "." + fi.extension());
   QFile f(filename);
   hybrid_list.clear();
   hybrid_map.clear();
   QRegExp count_hydrogens("H(\\d)");
   if (f.open(IO_ReadOnly|IO_Translate))
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
         if ( count_hydrogens.search( current_hybrid.name ) != -1 )
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
}

void US_Hydrodyn_Saxs::select_saxs_file()
{
   QString old_filename = saxs_filename;
      
   saxs_filename = QFileDialog::getOpenFileName(USglobal->config_list.system_dir + SLASH + "etc", "*.saxs_atoms *.SAXS_ATOMS", this);
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
   lbl_saxs_table->setText(fi.baseName() + "." + fi.extension());
   QFile f(filename);
   saxs_list.clear();
   saxs_map.clear();
   if (f.open(IO_ReadOnly|IO_Translate))
   {
      QTextStream ts(&f);
      while (!ts.atEnd())
      {
         ts >> current_saxs.saxs_name;
         ts >> current_saxs.a[0];
         ts >> current_saxs.b[0];
         ts >> current_saxs.a[1];
         ts >> current_saxs.b[1];
         ts >> current_saxs.a[2];
         ts >> current_saxs.b[2];
         ts >> current_saxs.a[3];
         ts >> current_saxs.b[3];
         ts >> current_saxs.c;
         ts >> current_saxs.volume;
         str1 = ts.readLine(); // read rest of line
         if (!current_saxs.saxs_name.isEmpty())
         {
#if defined(SAXS_DEBUG)
            cout << "read saxs file: ["
                 << current_saxs.saxs_name
                 << "] a[0] = "
                 << current_saxs.a[0]
                 << endl;
#endif
#if defined(RESCALE_B)
            for ( unsigned int i = 0; i < 4; i++ )
            {
               current_saxs.b[i] /= 16.0f * M_PI * M_PI;
            }
#endif
            saxs_list.push_back(current_saxs);
            saxs_map[current_saxs.saxs_name] = current_saxs;
         }
      }
      f.close();
   }
}

QString US_Hydrodyn_Saxs::saxs_filestring()
{
   QString result = 
      QString("%1").arg(te_filename2->text()) +
      QString("_%1").arg(current_model + 1) + 
      iqq_suffix() +
      ".ssaxs";
   return result;
}

QString US_Hydrodyn_Saxs::sprr_filestring()
{
   QString result = 
      QString("%1_%2b%3")
      .arg(te_filename2->text())
      .arg(current_model + 1)
      .arg(our_saxs_options->bin_size);

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

   search_csv.header.clear();
   search_csv.header_map.clear();
   search_csv.data.clear();
   search_csv.num_data.clear();
   search_csv.prepended_names.clear();

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

   tmp_data.clear();
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

   tmp_data.clear();
   tmp_data.push_back("SWH excluded volume");
   tmp_data.push_back("N");
   tmp_data.push_back("15");
   tmp_data.push_back("30");
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
#ifndef QT4
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
      ((US_Hydrodyn *)us_hydrodyn)->saxs_screen_window->show();
   }
#endif
}

void US_Hydrodyn_Saxs::reset_screen_csv()
{
#ifndef QT4
   if ( ((US_Hydrodyn *)us_hydrodyn)->last_saxs_screen_csv.name != "__empty__" )
   {
      screen_csv = ((US_Hydrodyn *)us_hydrodyn)->last_saxs_screen_csv;
      return;
   } 

   screen_csv.name = "SAXS I(q) Screen";

   screen_csv.header.clear();
   screen_csv.header_map.clear();
   screen_csv.data.clear();
   screen_csv.num_data.clear();
   screen_csv.prepended_names.clear();

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
   tmp_data.push_back("1000");
   tmp_data.push_back("1001");
   tmp_data.push_back("");

   screen_csv.prepended_names.push_back(tmp_data[0]);
   screen_csv.data.push_back(tmp_data);

   tmp_data.clear();
   tmp_data.push_back("Delta rho (A^-3)");
   tmp_data.push_back("N");
   tmp_data.push_back(".3");
   tmp_data.push_back(".8");
   tmp_data.push_back("101");
   tmp_data.push_back("");

   screen_csv.prepended_names.push_back(tmp_data[0]);
   screen_csv.data.push_back(tmp_data);

   tmp_data.clear();
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
#endif
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
      ((US_Hydrodyn *)us_hydrodyn)->saxs_buffer_window->show();
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

   buffer_csv.header.clear();
   buffer_csv.header_map.clear();
   buffer_csv.data.clear();
   buffer_csv.num_data.clear();
   buffer_csv.prepended_names.clear();

   buffer_csv.header.push_back("Parameter");
   buffer_csv.header.push_back("Active");
   buffer_csv.header.push_back("Low value");
   buffer_csv.header.push_back("High value");
   buffer_csv.header.push_back("Points");
   buffer_csv.header.push_back("Interval");
   buffer_csv.header.push_back("Current value");
   buffer_csv.header.push_back("Best value");

   vector < QString > tmp_data;
   
   tmp_data.clear();
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

   tmp_data.clear();
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

   tmp_data.clear();
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

void US_Hydrodyn_Saxs::load_gnom()
{
   plotted = false;
   QString use_dir = 
      our_saxs_options->path_load_gnom.isEmpty() ?
      USglobal->config_list.root_dir + SLASH + "somo" + SLASH + "saxs" :
      our_saxs_options->path_load_gnom;

   select_from_directory_history( use_dir );

   QString filename = QFileDialog::getOpenFileName(use_dir, "*.out", this);
   if (filename.isEmpty())
   {
      return;
   }
   add_to_directory_history( filename );

   QFile f(filename);
   our_saxs_options->path_load_gnom = QFileInfo(filename).dirPath(true);
   QString ext = QFileInfo(filename).extension(FALSE).lower();
   bool plot_gnom = false;
   vector < double > gnom_Iq_reg;
   vector < double > gnom_Iq_exp;
   vector < double > gnom_q;

   if ( f.open(IO_ReadOnly) )
   {
      QStringList qsl_gnom;
      {
         QTextStream ts(&f);
         while ( !ts.atEnd() )
         {
            qsl_gnom << ts.readLine();
         }
      }
      f.close();
      f.open(IO_ReadOnly);
      bool ask_save_mw_to_gnom = false;
      double gnom_mw = 0e0;

      double units = 1;
      if ( our_saxs_options->iq_scale_ask )
      {
         switch( QMessageBox::information( this, 
                                           tr("UltraScan"),
                                           tr("Is this file in Angstrom or nm units?"),
                                           "&Angstrom", 
                                           "&nm", 0,
                                           0,      // Enter == button 0
                                           1 ) ) { // Escape == button 2
         case 0: // load it as is
            units = 1.0;
            break;
         case 1: // rescale
            units = 0.1;
            break;
         } 
      } else {
         if ( our_saxs_options->iq_scale_angstrom ) 
         {
            units = 1.0;
         } else {
            units = 0.1;
         }
      }
         
      QTextStream ts(&f);
      QRegExp iqqh("^\\s*S\\s+J EXP\\s+ERROR\\s+J REG\\s+I REG\\s*$");
      QRegExp prrh("^\\s*R\\s+P\\(R\\)\\s+ERROR\\s*$");
      QRegExp rx2("^\\s*(\\S*)\\s+(\\S*)\\s*$");
      QRegExp rx3("^\\s*(\\S*)\\s+(\\S*)\\s+(\\S*)\\s*$");
      QRegExp rx5("^\\s*(\\S*)\\s+(\\S*)\\s+(\\S*)\\s+(\\S*)\\s+(\\S*)\\s*$");
      QRegExp rxinputfile("Input file.s. : (\\S+)\\s*$");
      QString tmp;
      vector < QString > datafiles;
      while ( !ts.atEnd() )
      {
         tmp = ts.readLine();
         if ( rxinputfile.search(tmp) != -1 ) 
         {
            // datafiles.push_back(rxinputfile.cap(1).stripWhiteSpace());
            continue;
         }
         if ( iqqh.search(tmp) != -1 )
         {
            vector < double > I_exp;
            vector < double > I_reg;
            vector < double > q;
            // cout << "start of iqq\n";
            ts.readLine(); // blank line
            while ( !ts.atEnd() )
            {
               tmp = ts.readLine();
               if ( rx5.search(tmp) != -1 )
               {
                  q.push_back(rx5.cap(1).toDouble() * units );
                  I_exp.push_back(rx5.cap(2).toDouble());
                  I_reg.push_back(rx5.cap(5).toDouble());
                  // cout << "iqq point: " << rx5.cap(1).toDouble() << " " << rx5.cap(5).toDouble() << endl;
               } else {
                  // end of iqq?
                  if ( rx2.search(tmp) == -1 )
                  {
                     plot_gnom = true;
                     gnom_Iq_reg = I_reg;
                     gnom_Iq_exp = I_exp;
                     gnom_q = q;
                     // plot_one_iqq(q, I, QFileInfo(filename).fileName());
                     // if ( plotted )
                     // {
                     //   editor->setParagraphBackgroundColor ( editor->paragraphs() - 1, QColor("white") );
                     //   editor->append("I(q) vs q plot done\n");
                     //   plotted = false;
                     // }
                     break;
                  } else {
                     // cout << "iqq 2 fielder ignored\n";
                  }
               }
            }
            if ( ts.atEnd() &&
                 q.size() )
            {
               plot_gnom = true;
               gnom_Iq_reg = I_reg;
               gnom_Iq_exp = I_exp;
               gnom_q = q;
            }
            continue;
         }
         if ( prrh.search(tmp) != -1 )
         {
            vector < double > r;
            vector < double > pr;
            // cout << "start of prr\n";
            ts.readLine(); // blank line
            while ( !ts.atEnd() )
            {
               tmp = ts.readLine();
               if ( rx3.search(tmp) != -1 )
               {
                  r.push_back(rx3.cap(1).toDouble() / units);
                  pr.push_back(rx3.cap(2).toDouble());
                  // cout << "prr point: " << rx3.cap(1).toDouble() << " " << rx3.cap(2).toDouble() << endl;
               } else {
                  // end of prr?
                  QRegExp qx_mw("molecular weight (\\d+(|\\.\\d+))",false);
                  QStringList mwline = qsl_gnom.grep(qx_mw);
                  if ( mwline.size() )
                  {
                     if ( qx_mw.search(mwline[0]) == -1 )
                     {
                        // cerr << QString("qx_mw.search of <%1> for molecular weight failed!\n").arg(mwline[0]);
                        gnom_mw = 0e0;
                     } else {
                        // cout << QString("mwline cap 0 <%1> cap 1 <%2>\n").arg(qx_mw.cap(0)).arg(qx_mw.cap(1));
                        gnom_mw = qx_mw.cap(1).toDouble();
                     }
                     if ( mwline.size() > 1 )
                     {
                        QMessageBox::message(tr("Please note:"), 
                                             QString(tr("There are multiple molecular weight lines in the gnom file\n"
                                                        "Using the first one found (%1 Daltons)")).arg(gnom_mw));
                     }
                     if ( gnom_mw > 0e0 )
                     {
                        (*remember_mw)[QFileInfo(filename).fileName()] = gnom_mw;
                        (*remember_mw_source)[QFileInfo(filename).fileName()] = "Found in gnom.out file";
                     }
                  }
                  gnom_mw = get_mw(filename,false);
                  if ( !mwline.size() )
                  {
                     ask_save_mw_to_gnom = true;
                  }
                  if ( cb_normalize->isChecked() )
                  {
                     normalize_pr(r, &pr, get_mw(filename, false));
                  }
                  plot_one_pr(r, pr, QFileInfo(filename).fileName());
                  if ( plotted )
                  {
                     editor->setParagraphBackgroundColor ( editor->paragraphs() - 1, QColor("white") );
                     editor->append("P(r) plot done\n");
                     plotted = false;
                  }
                  break;
               }
            }
         }
      }
      f.close();
      if ( ask_save_mw_to_gnom && gnom_mw )
      {
         switch( QMessageBox::information( this, 
                                           tr("Save GNOM with Molecular Weight"),
                                           QString(tr("Do you want to save the molecular weight entered (%1 Daltons) into the gnom.out file?"))
                                           .arg(gnom_mw),
                                           "&Ok",  
                                           "&Cancel", 
                                           0,
                                           0,      // Enter == button 0
                                           1 ) ) { // Escape == button 2
         case 0: // write the file
            {
               QString fname = filename;
               if ( QFile::exists(fname) )
               {
                  fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fname, 0, this);
               }
               QFile f(fname);
               if ( !f.open( IO_WriteOnly ) )
               {
                  QMessageBox::warning( this, "UltraScan",
                                        QString(tr("Could not open %1 for writing!")).arg(fname) );
               } else {
                  QTextStream t( &f );
                  t << QString("Molecular weight %1 Daltons\n\n").arg(gnom_mw);
                  t << qsl_gnom.join("\n");
                  t << "\n";
                  f.close();
                  editor->append(QString(tr("Created file %1\n")).arg(f.name()));
               }
            }                  
            break;
         case 1: // Cancel clicked or Escape pressed
            break;
         }
      }

      if ( datafiles.size() )
      {
         for ( unsigned int i = 0; i < datafiles.size(); i++ )
         {
            QString datafile = our_saxs_options->path_load_gnom + QDir::separator() + datafiles[i];
            if ( QFileInfo(datafile).exists() )
            {
               switch( QMessageBox::information( this, 
                                                 tr("UltraScan"),
                                                 tr("Found the GNOM associated data file\n") + QFileInfo(datafile).fileName() + "\n" +
                                                 tr("Do you want to load it?"),
                                                 "&Ok", 
                                                 "&No", 0,
                                                 0,      // Enter == button 0
                                                 1 ) ) { // Escape == button 2
               case 0: // load it
                  load_saxs(datafile);
               break;
               case 1: // Cancel clicked or Escape pressed
                  break;
               }
            }
         }
      }
      if ( plot_gnom )
      {
         vector < double > gnom_q_reg = gnom_q;

         crop_iq_data(gnom_q, gnom_Iq_exp);
         plot_one_iqq(gnom_q, gnom_Iq_exp, QFileInfo(filename).fileName() + " Experimental");
         crop_iq_data(gnom_q_reg, gnom_Iq_reg);
         plot_one_iqq(gnom_q_reg, gnom_Iq_reg, QFileInfo(filename).fileName() + " Regularized");
         if ( plotted )
         {
            editor->setParagraphBackgroundColor ( editor->paragraphs() - 1, QColor("white") );
            editor->append("I(q) vs q plot done\n");
            plotted = false;
         }
         cb_guinier->setChecked(false);
         cb_user_range->setChecked(false);
         set_guinier();
      }
   }
}

vector < double > US_Hydrodyn_Saxs::interpolate( vector < double > to_r, 
                                                 vector < double > from_r, 
                                                 vector < double > from_pr )
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
   }
   return usu.wave["out"].r;
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
   if ( rx.search(filename) == -1 )
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
   double lowq;
   double highq;
   double lowI;
   double highI;
   plot_domain( lowq, highq );
   plot_range ( lowq, highq, lowI, highI );

   cout << "rescale plot "
           << lowq << ":" << highq << "  "
           << lowI << ":" << highI << endl;

   plot_saxs->setAxisScale( QwtPlot::xBottom, lowq, highq );
   plot_saxs->setAxisScale( QwtPlot::yLeft,   lowI, highI );

   if ( plot_saxs_zoomer )
   {
      delete plot_saxs_zoomer;
   }
   plot_saxs_zoomer = new ScrollZoomer(plot_saxs->canvas());
#ifndef QT4
   plot_saxs_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
   plot_saxs_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#else
   plot_saxs_zoomer->setRubberBandPen( QPen( Qt::red, 1, Qt::DotLine ) );
   plot_saxs_zoomer->setTrackerPen( QPen( Qt::red ) );
#endif

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
                                 tr( " q^2 * I(q)"        ) : tr( "Log10 I(q)"     ) );
#ifndef QT4
      plot_saxs->setAxisOptions( QwtPlot::yLeft, 
                                 cb_kratky->isChecked()  ? 
                                 QwtAutoScale::None         : QwtAutoScale::Logarithmic );
#else
      plot_saxs->setAxisScaleEngine(QwtPlot::yLeft, 
                                    cb_kratky->isChecked() ?
                                    new QwtLog10ScaleEngine :  // fix this
                                    new QwtLog10ScaleEngine);
#endif

      rescale_plot();
   }
}
      
void US_Hydrodyn_Saxs::set_guinier()
{
   if ( rb_sans->isChecked() ) 
   {
      plot_saxs->setTitle((cb_guinier->isChecked() ? "Guinier " : "") + tr("SANS Curve"));
   } else {
      plot_saxs->setTitle((cb_guinier->isChecked() ? "Guinier " : "") + tr("SAXS Curve"));
   }

   if ( cb_guinier->isChecked() )
   {
      cb_user_range->setChecked(false);
      cb_kratky    ->setChecked(false);
   }

   if ( cb_kratky->isChecked() )
   {
      if ( rb_sans->isChecked() ) 
      {
         plot_saxs->setTitle( "Kratky " + tr("SANS Curve"));
      } else {
         plot_saxs->setTitle( "Kratky " + tr("SAXS Curve"));
      }
   }      

   rescale_plot();

   // clear the plot and replot the curves in correct mode ( kratky or guniner )
   plot_saxs->clear();
   for ( unsigned int p = 0;
#ifndef QT4
         p < plotted_Iq.size();
#else
         p < plotted_Iq_curves.size();
#endif
         p++ )
   {
      unsigned int q_points = plotted_q[ p ].size();
      
      vector < double > q2I;
      if ( cb_kratky->isChecked() )
      {
         for ( unsigned int i = 0; i < plotted_q[ p ].size(); i++ )
         {
            q2I.push_back( plotted_q2[ p ][ i ] * plotted_I[ p ][ i ] );
         }
      }
#ifndef QT4
      long Iq = plot_saxs->insertCurve( qsl_plotted_iq_names[ p ] );
      plot_saxs->setCurveStyle(Iq, QwtCurve::Lines);
      plotted_Iq[ p ] = Iq;
#else
      QwtPlotCurve *curve = new QwtPlotCurve( qsl_plotted_iq_names[ p ] );
      curve->setStyle( QwtPlotCurve::Lines );
      plotted_Iq_curves[ p ] = curve;
#endif
      
      
#ifndef QT4
      plot_saxs->setCurveData(plotted_Iq[ p ],
                              cb_guinier->isChecked() ?
                              (double *)&(plotted_q2[p][0])  : (double *)&(plotted_q[p][0]), 
                              cb_kratky ->isChecked() ?
                              (double *)&(q2I[0])            : (double *)&(plotted_I[p][0]),
                              q_points);
      plot_saxs->setCurvePen( plotted_Iq[ p ], QPen(plot_colors[p % plot_colors.size()], 2, SolidLine));
#else
      plotted_Iq_curves[ p ]->setData(
                                      cb_guinier->isChecked() ?
                                      (double *)&(plotted_q2[p][0])  : (double *)&(plotted_q[p][0]), 
                                      cb_kratky ->isChecked() ?
                                      (double *)&(q2I[0])            : (double *)&(plotted_I[p][0]),
                                      q_points
                                      );
      plotted_Iq_curves[ p ]->setPen( QPen( plot_colors[ p % plot_colors.size() ], 2, Qt::SolidLine ) );
      plotted_Iq_curves[ p ]->attach( plot_saxs );
#endif
   }

#ifndef QT4
   int niqsize = (int)plotted_Iq.size();
#else
   int niqsize = (int)plotted_Iq_curves.size();
#endif
   for ( int i = 0; i < niqsize; i++ )
   {
      if ( cb_guinier->isChecked() )
      {
         // replot the guinier bits
         if ( plotted_guinier_valid.count(i) &&
              plotted_guinier_valid[i] == true )
         {
#ifndef QT4
            plotted_Gp[i] = plot_saxs->insertCurve(QString("Guinier points %1").arg(i));
            plot_saxs->setCurveStyle(plotted_Gp[i], QwtCurve::Lines);
            plot_saxs->setCurveData(plotted_Gp[i], (double *)&(plotted_guinier_x[i][0]), (double *)&(plotted_guinier_y[i][0]), 2);

            plot_saxs->setCurvePen(plotted_Gp[i], QPen("dark red", 2, SolidLine));
#else
            plotted_Gp_curves[i] = new QwtPlotCurve(
                  QString( "Guinier points %1" ).arg( i ) );
            plotted_Gp_curves[i]->setStyle( QwtPlotCurve::Lines );
            plotted_Gp_curves[i]->setData(
                           (double *)&(plotted_guinier_x[i][0]), 
                           (double *)&(plotted_guinier_y[i][0]), 
                           2
                           );
            plotted_Gp_curves[i]->setPen( QPen( QColor( "dark red" ), 2, Qt::SolidLine ) );
            plotted_Gp_curves[i]->attach( plot_saxs );
#endif            
            plotted_guinier_plotted[i] = true;
         }

         // turn regular iqq curves into points

         QwtSymbol sym;
         sym.setStyle(QwtSymbol::Cross);
         sym.setSize(12);
         sym.setPen(QPen(plot_colors[i % plot_colors.size()]));
         sym.setBrush(Qt::white);
#ifndef QT4
         plot_saxs->setCurveStyle(plotted_Iq[i], QwtCurve::NoCurve);
         plot_saxs->setCurveSymbol(plotted_Iq[i], QwtSymbol(sym));
#else
         plotted_Iq_curves[i]->setStyle( QwtPlotCurve::NoCurve );
         plotted_Iq_curves[i]->setSymbol( QwtSymbol( sym ) );
#endif
         
      } else {
         // remove any Guinier lines
         if ( plotted_guinier_plotted.count(i) &&
              plotted_guinier_plotted[i] == true )
         {
            plotted_guinier_plotted[i] = false;
#ifndef QT4
            plot_saxs->removeCurve(plotted_Gp[i]);
#else
            plotted_Gp_curves[i]->detach();
#endif
         }
         
         // remove the symbols & redraw the line
         QwtSymbol sym;
#ifndef QT4
         sym.setStyle(QwtSymbol::None);
         plot_saxs->setCurveSymbol(plotted_Iq[i], sym);
         plot_saxs->setCurveStyle(plotted_Iq[i], QwtCurve::Lines);
         plot_saxs->setCurvePen(plotted_Iq[i], QPen(plot_colors[i % plot_colors.size()], 2, SolidLine));
#else
         sym.setStyle(QwtSymbol::NoSymbol);
         plotted_Iq_curves[i]->setSymbol( QwtSymbol( sym ) );
         plotted_Iq_curves[i]->setStyle( QwtPlotCurve::Lines );
         plotted_Iq_curves[i]->setPen( QPen( plot_colors[ i % plot_colors.size() ], 2, Qt::SolidLine ) );
#endif
      }

      vector < double > q2I;
      if ( cb_kratky->isChecked() )
      {
         for ( unsigned int j = 0; j < plotted_q[ i ].size(); j++ )
         {
            q2I.push_back( plotted_q2[ i ][ j ] * plotted_I[ i ][ j ] );
         }
      }

#ifndef QT4
      plot_saxs->setCurveData(
                              plotted_Iq[i], 
                              cb_guinier->isChecked() ?
                              (double *)&(plotted_q2[i][0]) : (double *)&(plotted_q[i][0]), 
                              cb_kratky ->isChecked() ?
                              (double *)&(q2I[0])           : (double *)&(plotted_I[i][0]),
                              plotted_q[i].size()
                              );
#else
      plotted_Iq_curves[i]->setData(
                                    cb_guinier->isChecked() ?
                                    (double *)&(plotted_q2[i][0]) : (double *)&(plotted_q[i][0]), 
                                    cb_kratky ->isChecked() ?
                                    (double *)&(q2I[0])           : (double *)&(plotted_I[i][0]),
                                    plotted_q[i].size()
                                    );
#endif
   }

   plot_saxs->setAxisTitle  ( QwtPlot::xBottom, 
                              cb_guinier->isChecked() ? 
                              tr( "q^2 (1/Angstrom^2)" ) : tr( "q (1/Angstrom)" ) );
   plot_saxs->setAxisTitle  ( QwtPlot::yLeft,   
                              cb_kratky ->isChecked() ? 
                              tr( " q^2 * I(q)"        ) : tr( "Log10 I(q)"     ) );
#ifndef QT4
   plot_saxs->setAxisOptions( QwtPlot::yLeft, 
                              cb_kratky->isChecked()  ? 
                              QwtAutoScale::None         : QwtAutoScale::Logarithmic );
#else
      plot_saxs->setAxisScaleEngine(QwtPlot::yLeft, 
                                    cb_kratky->isChecked() ?
                                    new QwtLog10ScaleEngine :  // fix this
                                    new QwtLog10ScaleEngine);
#endif

   if ( plot_saxs_zoomer )
   {
      delete plot_saxs_zoomer;
   }
   plot_saxs_zoomer = new ScrollZoomer(plot_saxs->canvas());
#ifndef QT4
   plot_saxs_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
   plot_saxs_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#else
   plot_saxs_zoomer->setRubberBandPen( QPen( Qt::red, 1, Qt::DotLine ) );
   plot_saxs_zoomer->setTrackerPen( QPen( Qt::red ) );
#endif

   plot_saxs->replot();
}

void US_Hydrodyn_Saxs::clear_guinier()
{
   // remove any existing Guinier curves

   for ( unsigned int g = 0;
#ifndef QT4
         g < plotted_Gp.size();
#else
         g < plotted_Gp_curves.size();
#endif
         g++ )
   {
#ifndef QT4
      plot_saxs->removeCurve(plotted_Gp[g]);
#else
      plotted_Gp_curves[g]->detach();
#endif
   }
#ifndef QT4
   plotted_Gp.clear();
#else
   plotted_Gp_curves.clear();
#endif
   plotted_guinier_valid.clear();
   plotted_guinier_lowq2.clear();
   plotted_guinier_highq2.clear();
   plotted_guinier_a.clear();
   plotted_guinier_b.clear();
   plotted_guinier_x.clear();
   plotted_guinier_y.clear();
}


void US_Hydrodyn_Saxs::run_guinier_analysis()
{
   editor->append("Guinier analysis:\n");
   editor->setParagraphBackgroundColor ( editor->paragraphs() - 1, QColor("dark gray") );
   clear_guinier();

   QString csvlog = 
      "\"Source file\","
      "\"Notes\","
      "\"Rg\","
      "\"Rg sd\","
      "\"I(0)\","
      "\"I(0) sd\","
      "\"q min\","
      "\"q max\","
      "\"q*Rg min\","
      "\"q*Rg max\","
      "\"starting point\","
      "\"ending point\","
      "\"points used\","
      "\"chi^2\","
      "\n";
   
   for ( unsigned int i = 0; 
#ifndef QT4
         i < plotted_Iq.size();
#else
         i < plotted_Iq_curves.size();
#endif
         i++ )
   {
      guinier_analysis(i, csvlog);
   }
   editor->setParagraphBackgroundColor ( editor->paragraphs() - 1, QColor("white") );
   cb_guinier->setChecked(true);
   set_guinier();

   if ( our_saxs_options->guinier_csv )
   {
      QFile f(USglobal->config_list.root_dir + SLASH + "somo" + SLASH + "saxs" + SLASH + 
              our_saxs_options->guinier_csv_filename + ".csv");

      if ( !f.open(IO_WriteOnly) )
      {
         editor->append(QString(tr("Can not create file %1\n")).arg(f.name()));
      }
      QTextStream ts(&f);
      ts << csvlog;
      f.close();
      editor->append(QString(tr("Created file %1\n")).arg(f.name()));
   }
}

bool US_Hydrodyn_Saxs::guinier_analysis( unsigned int i, QString &csvlog )
{
   if (
#ifndef QT4 
       i > plotted_Iq.size() 
#else
       i > plotted_Iq_curves.size() 
#endif
       )
   {
      editor->append("internal error: invalid plot selected\n");
      return false;
   }

   US_Saxs_Util usu;
   usu.wave["data"].q = plotted_q[i];
   usu.wave["data"].r = plotted_I[i];
   usu.wave["data"].s = plotted_I[i];
   QString log;

   int pointsmin = our_saxs_options->pointsmin;
   int pointsmax = our_saxs_options->pointsmax;
   double sRgmaxlimit = our_saxs_options->qRgmax;
   double pointweightpower = 3e0;
   double p_guinier_maxq = our_saxs_options->qend;
   
   // these are function output values
   double a;
   double b;
   double siga;
   double sigb;
   double chi2;
   double Rg;
   double Io;
   double smin;
   double smax;
   double sRgmin;
   double sRgmax;

   unsigned int beststart;
   unsigned int bestend;

   bool too_few_points = plotted_q[i].size() <= 25;

   if ( !too_few_points )
   {
      if ( 
          !usu.guinier_plot(
                            "guinier",
                            "data"
                            )   ||
          !usu.guinier_fit2(
                            log,
                            "guinier", 
                            pointsmin,
                            pointsmax,
                            sRgmaxlimit,
                            pointweightpower,
                            p_guinier_maxq,
                            a,
                            b,
                            siga,
                            sigb,
                            chi2,
                            Rg,
                            Io,
                            smax, // don't know why these are flipped
                            smin,
                            sRgmin,
                            sRgmax,
                            beststart,
                            bestend
                            ) )
      {
         editor->append(QString("Error performing Guinier analysis on %1\n" + usu.errormsg + "\n")
                        .arg(qsl_plotted_iq_names[i]));
         return false;
      }

      // cout << "guinier siga " << siga << endl;
      // cout << "guinier sigb " << sigb << endl;
   
      // cout << log;
   }

   QColor save_color = editor->color();
   editor->setColor(plot_colors[i % plot_colors.size()]);

   QString report;
   if ( too_few_points )
   {
      report =
         QString(
                 "Guinier analysis of %1:\n"
                 "**** Could not compute Rg, too few data points %2 ****\n"
                 )
         .arg(plotted_q[i].size());

      csvlog += 
         QString(
                 "\"%1\","
                 "\"Too few data points (%2)\"\n"
                 )
         .arg(qsl_plotted_iq_names[i])
         .arg(plotted_q[i].size());
   } else {
      if ( isnan(Rg) ||
           b >= 0e0 )
      {
         plotted_guinier_valid[i] = false;
         report =
            QString(
                    "Guinier analysis of %1:\n"
                    "**** Could not compute Rg ****\n"
                    )
            .arg(qsl_plotted_iq_names[i]);

         csvlog += 
            QString(
                    "\"%1\","
                    "\"Could not compute Rg\"\n"
                    )
            .arg(qsl_plotted_iq_names[i]);

      } else {
         report = 
            QString("")
            .sprintf(
                     "Guinier analysis of %s:\n"
                     "Rg %.1f (%.1f) (A) I(0) %.2e (%.2e) qRgmin %.3f qRgmax %.3f points used %u chi^2 %.2e\n"
                     
                     , qsl_plotted_iq_names[i].ascii()
                     , Rg
                     , sqrt(3e0) * 5e-1 * (1e0/sqrt(-b)) * sigb 
                     , Io
                     , siga
                     , sRgmin
                     , sRgmax
                     , bestend - beststart + 1
                     , chi2
                     );
         
         plotted_guinier_valid[i] = true;
         plotted_guinier_lowq2[i] = smin * smin;
         plotted_guinier_highq2[i] = smax * smax;
         plotted_guinier_a[i] = a;
         plotted_guinier_b[i] = b;
         plotted_guinier_plotted[i] = false;
         
         plotted_guinier_x[i].clear();
         plotted_guinier_x[i].push_back(plotted_guinier_lowq2[i]);
         plotted_guinier_x[i].push_back(plotted_guinier_highq2[i]);
         
         plotted_guinier_y[i].clear();
         plotted_guinier_y[i].push_back(exp(plotted_guinier_a[i] + plotted_guinier_b[i] * plotted_guinier_lowq2[i]));
         plotted_guinier_y[i].push_back(exp(plotted_guinier_a[i] + plotted_guinier_b[i] * plotted_guinier_highq2[i]));

         csvlog += 
            QString(
                    "\"%1\","
                    "\"Ok\","
                    "%2,"
                    "%3,"
                    "%4,"
                    "%5,"
                    "%6,"
                    "%7,"
                    "%8,"
                    "%9,"
                    "%10,"
                    "%11,"
                    "%12,"
                    "%13"
                    "\n"
                    )
            .arg(qsl_plotted_iq_names[i])
            .arg(Rg)
            .arg( sqrt(3e0) * 5e-1 * (1e0/sqrt(-b)) * sigb )
            .arg(Io)
            .arg(siga)
            .arg(smin)
            .arg(smax)
            .arg(sRgmin)
            .arg(sRgmax)
            .arg(beststart)
            .arg(bestend)
            .arg(bestend - beststart + 1)
            .arg(chi2);
      }
   }
   editor->append(report);
   editor->setColor(save_color);

   //   cout << csvlog;
   return true;
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

      // first compute available guinier range
      // this could be calculated once upon adding curves

      bool any_guinier = false;

      for ( unsigned int i = 0; 
#ifndef QT4
            i < plotted_Iq.size();
#else
            i < plotted_Iq_curves.size();
#endif
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
#ifndef QT4
         i < plotted_Iq.size();
#else
         i < plotted_Iq_curves.size();
#endif
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
      // for each plot
      for ( unsigned int i = 0;
#ifndef QT4
            i < plotted_Iq.size();
#else
            i < plotted_Iq_curves.size();
#endif
            i++ )      
      {
         // scan the q range
         for ( unsigned int j = 0; j < plotted_q2[i].size(); j++ )
         {
            if ( plotted_q2[i][j] >= lowq &&
                 plotted_q2[i][j] <= highq )
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
   } else {
      // for each plot
      for ( unsigned int i = 0; 
#ifndef QT4
            i < plotted_Iq.size();
#else
            i < plotted_Iq_curves.size();
#endif
            i++ )      
      {
         // scan the q range
         for ( unsigned int j = 0; j < plotted_q[i].size(); j++ )
         {
            if ( plotted_q[i][j] >= lowq &&
                 plotted_q[i][j] <= highq )
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
   if ( started_in_expert_mode )
   {
      rb_saxs_iq_native_hybrid ->setChecked(our_saxs_options->saxs_iq_native_hybrid);
      rb_saxs_iq_native_hybrid2->setChecked(our_saxs_options->saxs_iq_native_hybrid2);
   }
   rb_saxs_iq_native_hybrid3->setChecked(our_saxs_options->saxs_iq_native_hybrid3);
   rb_saxs_iq_native_fast   ->setChecked(our_saxs_options->saxs_iq_native_fast);
   rb_saxs_iq_foxs          ->setChecked(our_saxs_options->saxs_iq_foxs);
   rb_saxs_iq_crysol        ->setChecked(our_saxs_options->saxs_iq_crysol);
   rb_sans_iq_native_debye  ->setChecked(our_saxs_options->sans_iq_native_debye);
   if ( started_in_expert_mode )
   {
      rb_sans_iq_native_hybrid ->setChecked(our_saxs_options->sans_iq_native_hybrid);
      rb_sans_iq_native_hybrid2->setChecked(our_saxs_options->sans_iq_native_hybrid2);
   }
   rb_sans_iq_native_hybrid3->setChecked(our_saxs_options->sans_iq_native_hybrid3);
   rb_sans_iq_native_fast   ->setChecked(our_saxs_options->sans_iq_native_fast);
   rb_sans_iq_cryson        ->setChecked(our_saxs_options->sans_iq_cryson);

   update_iqq_suffix();
}


void US_Hydrodyn_Saxs::set_saxs_iq(int val)
{
   int ref = 0;
   rb_saxs_iq_native_debye  ->setChecked( val == ref ); ref++;
   if ( started_in_expert_mode )
   {
      rb_saxs_iq_native_hybrid ->setChecked( val == ref ); ref++;
      rb_saxs_iq_native_hybrid2->setChecked( val == ref ); ref++;
   }
   rb_saxs_iq_native_hybrid3->setChecked( val == ref ); ref++;
   rb_saxs_iq_native_fast   ->setChecked( val == ref ); ref++;
   rb_saxs_iq_foxs          ->setChecked( val == ref ); ref++;
   rb_saxs_iq_crysol        ->setChecked( val == ref ); ref++;

   our_saxs_options->saxs_iq_native_debye   = rb_saxs_iq_native_debye  ->isChecked();
   if ( started_in_expert_mode )
   {
      our_saxs_options->saxs_iq_native_hybrid  = rb_saxs_iq_native_hybrid ->isChecked();
      our_saxs_options->saxs_iq_native_hybrid2 = rb_saxs_iq_native_hybrid2->isChecked();
   }
   our_saxs_options->saxs_iq_native_hybrid3 = rb_saxs_iq_native_hybrid3->isChecked();
   our_saxs_options->saxs_iq_native_fast    = rb_saxs_iq_native_fast   ->isChecked();
   our_saxs_options->saxs_iq_foxs           = rb_saxs_iq_foxs          ->isChecked();
   our_saxs_options->saxs_iq_crysol         = rb_saxs_iq_crysol        ->isChecked();

   update_iqq_suffix();
}

void US_Hydrodyn_Saxs::set_sans_iq(int val)
{
   int ref = 0;
   rb_sans_iq_native_debye  ->setChecked( val == ref ); ref++;
   if ( started_in_expert_mode )
   {
      rb_sans_iq_native_hybrid ->setChecked( val == ref ); ref++;
      rb_sans_iq_native_hybrid2->setChecked( val == ref ); ref++;
   }
   rb_sans_iq_native_hybrid3->setChecked( val == ref ); ref++;
   rb_sans_iq_native_fast   ->setChecked( val == ref ); ref++;
   rb_sans_iq_cryson        ->setChecked( val == ref ); ref++;

   our_saxs_options->sans_iq_native_debye   = rb_sans_iq_native_debye  ->isChecked();
   if ( started_in_expert_mode )
   {
      our_saxs_options->sans_iq_native_hybrid  = rb_sans_iq_native_hybrid ->isChecked();
      our_saxs_options->sans_iq_native_hybrid2 = rb_sans_iq_native_hybrid2->isChecked();
   }
   our_saxs_options->sans_iq_native_hybrid3 = rb_sans_iq_native_hybrid3->isChecked();
   our_saxs_options->sans_iq_native_fast    = rb_sans_iq_native_fast   ->isChecked();
   our_saxs_options->sans_iq_cryson         = rb_sans_iq_cryson        ->isChecked();

   update_iqq_suffix();
}
   
void US_Hydrodyn_Saxs::set_bead_model_ok_for_saxs()
{
   for ( unsigned int i = 0; i < selected_models.size(); i++ )
   {
      for ( unsigned int j = 0; j < bead_models[selected_models[i]].size(); j++ )
      {
         // cout << "saxs name [" << j << "] = " << bead_models[i][j].saxs_data.saxs_name << endl;
         if ( bead_models[i][j].active &&
              bead_models[i][j].saxs_data.saxs_name.isEmpty() )
         {
            cout << "bead model " << i << " is NOT ok for saxs\n";
            bead_model_ok_for_saxs = false;
            return;
         }
      }
   }
   cout << "bead models are ok for saxs\n";
   bead_model_ok_for_saxs = true;
}


// #define DEBUG_RESID

void US_Hydrodyn_Saxs::display_iqq_residuals( QString title,
                                              vector < double > q,
                                              vector < double > I1, 
                                              vector < double > I2,
                                              QColor            plot_color,
                                              vector < double > I_errors)
{
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

   bool use_errors = is_nonzero_vector( I_errors );
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
      if ( isnan( log_I1[ i ] ) )
      {
         log_I1[ i ] = 1e-34;
      }
      if ( isnan( log_I2[ i ] ) )
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
                                            tr("Residuals & difference targeting:\n") + title,
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
                                            std_dev_frac
                                            );
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
         qs += QString("_h%1_g%2_hs%3")
            .arg( our_saxs_options->crysol_max_harmonics )
            .arg( our_saxs_options->crysol_fibonacci_grid_order )
            .arg( QString("%1").arg( our_saxs_options->crysol_hydration_shell_contrast ).replace(".", "_" ) );
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
               qs += "fd";
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
      QString(tr("Current grid:"
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
   QString grid_target = QInputDialog::getItem(
                                               tr("Set I(q) Grid"),
                                               tr("Select the target plotted data to set the I(q) grid:\n"
                                                  "or Cancel of you do not wish to change the I(q) grid ")
                                               , 
                                               qsl_plotted_iq_names, 
                                               0, 
                                               FALSE, 
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
                            QString(tr("Error: could not find the grid for target %1")). arg( grid_target ) );
      return;
   }

   // set the grid

   unsigned int pos = plotted_iq_names_to_pos[ grid_target ];

   if ( plotted_q[ pos ].size() < 2 )
   {
      QMessageBox::warning( this, "Set I(q) grid",
                            QString(tr("Error: Too few data points (%!) to set the grid for target %2"))
                            .arg( plotted_q[ pos ].size() ) 
                            .arg( grid_target ) 
                            );
      return;
   }

   int last_result = 1;

   float save_start_q      = our_saxs_options->start_q;
   float save_end_q        = our_saxs_options->end_q;
   float save_delta_q      = our_saxs_options->delta_q;
   float save_start_angle  = our_saxs_options->start_angle;
   float save_end_angle    = our_saxs_options->end_angle;
   float save_delta_angle  = our_saxs_options->delta_angle;
   
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

      int res = QInputDialog::getInteger(
                                         "SOMO: Confirm grid",
                                         QString( tr( 
                                                     "Original number of grid points %1\n"
                                                     "Target number of grid points using the divisor below is %2\n"
                                                     "If this is acceptable, press OK\n"
                                                     "You can enter a divisor below and press OK\n"
                                                     "Press Cancel to revert to the original grid\n"
                                                     ) )
                                         .arg( q_points )
                                         .arg( grid_points )
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
         our_saxs_options->start_q     = save_start_q;
         our_saxs_options->end_q       = save_end_q;
         our_saxs_options->delta_q     = save_delta_q;
         our_saxs_options->start_angle = save_start_angle;
         our_saxs_options->end_angle   = save_end_angle;
         our_saxs_options->delta_angle = save_delta_angle;
         unsigned int grid_points = 
            (unsigned int)floor(((our_saxs_options->end_q - our_saxs_options->start_q) / our_saxs_options->delta_q) + .5) + 1;
         QMessageBox::message(tr("SOMO: Grid restored"), 
                              QString(tr("Reverted to orignal grid containing %1 points")).arg( grid_points ));
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

   if ( 
        our_saxs_options->start_q != save_start_q ||
        our_saxs_options->end_q   != save_end_q   ||
        our_saxs_options->delta_q != save_delta_q
        )
   {
      unsigned int q_points = 
         (unsigned int)floor(((our_saxs_options->end_q - our_saxs_options->start_q) / our_saxs_options->delta_q) + .5) + 1;
      
      QString gridmsg = 
         QString(tr("Current grid is now:"
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
