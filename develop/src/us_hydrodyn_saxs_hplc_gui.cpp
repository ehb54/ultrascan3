#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs_hplc.h"
#include <qsplitter.h>
//Added by qt3to4:
#include <Q3BoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <Q3GridLayout>
#include <Q3TextStream>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <Q3Frame>
#include <Q3PopupMenu>

#define UHSH_VAL_DEC 8

void US_Hydrodyn_Saxs_Hplc::setupGUI()
{
   int minHeight1 = 24;
   int minHeight3 = 25;

   started_in_expert_mode = ((US_Hydrodyn *)us_hydrodyn)->advanced_config.expert_mode;

   QColorGroup cg_magenta = USglobal->global_colors.cg_normal;
   cg_magenta.setBrush( QColorGroup::Base, QBrush( QColor( "magenta" ), Qt::SolidPattern ) );

   /*
   cg_magenta.setBrush( QColorGroup::Foreground, QBrush( QColor( "magenta" ), Qt::SolidPattern ) );
   cg_magenta.setBrush( QColorGroup::Button, QBrush( QColor( "blue" ), Qt::SolidPattern ) );
   cg_magenta.setBrush( QColorGroup::Light, QBrush( QColor( "darkcyan" ), Qt::SolidPattern ) );
   cg_magenta.setBrush( QColorGroup::Midlight, QBrush( QColor( "darkblue" ), Qt::SolidPattern ) );
   cg_magenta.setBrush( QColorGroup::Dark, QBrush( QColor( "yellow" ), Qt::SolidPattern ) );
   cg_magenta.setBrush( QColorGroup::Mid, QBrush( QColor( "darkred" ), Qt::SolidPattern ) );
   cg_magenta.setBrush( QColorGroup::Text, QBrush( QColor( "green" ), Qt::SolidPattern ) );
   cg_magenta.setBrush( QColorGroup::BrightText, QBrush( QColor( "darkgreen" ), Qt::SolidPattern ) );
   cg_magenta.setBrush( QColorGroup::ButtonText, QBrush( QColor( "cyan" ), Qt::SolidPattern ) );
   cg_magenta.setBrush( QColorGroup::Base, QBrush( QColor( "gray" ), Qt::SolidPattern ) );
   cg_magenta.setBrush( QColorGroup::Shadow, QBrush( QColor( "magenta" ), Qt::SolidPattern ) );
   cg_magenta.setBrush( QColorGroup::Highlight, QBrush( QColor( "darkyellow" ), Qt::SolidPattern ) );
   cg_magenta.setBrush( QColorGroup::HighlightedText, QBrush( QColor( "darkred" ), Qt::SolidPattern ) );
   */

   QColorGroup cg_red = cg_magenta;
   cg_red.setBrush( QColorGroup::Base, QBrush( QColor( "red" ), Qt::SolidPattern ) );

   lbl_title = new QLabel("Developed by Emre Brookes, Javier Pérez, Patrice Vachette and Mattia Rocco (see J. App. Cryst. 46:1823-1833, 2013)", this);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette( PALET_LABEL );
   AUTFBACK( lbl_title );
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_files = new mQLabel("Data files", this);
   lbl_files->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_files->setMinimumHeight(minHeight1);
   lbl_files->setPalette( PALET_LABEL );
   AUTFBACK( lbl_files );
   lbl_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   connect( lbl_files, SIGNAL( pressed() ), SLOT( hide_files() ) );

   cb_lock_dir = new QCheckBox(this);
   cb_lock_dir->setText(tr("Lock "));
   cb_lock_dir->setEnabled( true );
   cb_lock_dir->setChecked( false );
   cb_lock_dir->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ) );
   cb_lock_dir->setPalette( PALET_NORMAL );
   AUTFBACK( cb_lock_dir );

   lbl_dir = new mQLabel( QDir::currentDirPath(), this );
   lbl_dir->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_dir->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_dir );
   lbl_dir->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2));
   connect( lbl_dir, SIGNAL(pressed()), SLOT( dir_pressed() ));

   pb_add_files = new QPushButton(tr("Add files"), this);
   pb_add_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_add_files->setMinimumHeight(minHeight3);
   pb_add_files->setPalette( PALET_PUSHB );
   connect(pb_add_files, SIGNAL(clicked()), SLOT(add_files()));

   pb_similar_files = new QPushButton(tr("Similar"), this);
   pb_similar_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_similar_files->setMinimumHeight(minHeight3);
   pb_similar_files->setPalette( PALET_PUSHB );
   connect(pb_similar_files, SIGNAL(clicked()), SLOT(similar_files()));

   pb_conc = new QPushButton(tr("Concentrations"), this);
   pb_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_conc->setMinimumHeight(minHeight3);
   pb_conc->setPalette( PALET_PUSHB );
   connect(pb_conc, SIGNAL(clicked()), SLOT(conc()));

   pb_clear_files = new QPushButton(tr("Remove files"), this);
   pb_clear_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_clear_files->setMinimumHeight(minHeight3);
   pb_clear_files->setPalette( PALET_PUSHB );
   connect(pb_clear_files, SIGNAL(clicked()), SLOT(clear_files()));

   pb_regex_load = new QPushButton(tr("RL"), this);
   pb_regex_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_regex_load->setMinimumHeight(minHeight3);
   pb_regex_load->setPalette( PALET_PUSHB );
   connect(pb_regex_load, SIGNAL(clicked()), SLOT(regex_load()));

   le_regex = new QLineEdit(this, "le_regex Line Edit");
   le_regex->setText( "" );
   le_regex->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_regex->setPalette( PALET_NORMAL );
   AUTFBACK( le_regex );
   le_regex->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_regex_args = new QLineEdit(this, "le_regex_args Line Edit");
   le_regex_args->setText( "" );
   le_regex_args->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_regex_args->setPalette( PALET_NORMAL );
   AUTFBACK( le_regex_args );
   le_regex_args->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   pb_save_state = new QPushButton(tr("SS"), this);
   pb_save_state->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_save_state->setMinimumHeight(minHeight3);
   pb_save_state->setPalette( PALET_PUSHB );
   connect(pb_save_state, SIGNAL(clicked()), SLOT(save_state()));

   lb_files = new Q3ListBox(this, "files files listbox" );
   lb_files->setPalette( PALET_NORMAL );
   AUTFBACK( lb_files );
   lb_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lb_files->setEnabled(true);
   lb_files->setSelectionMode( Q3ListBox::Extended );
   lb_files->setMinimumHeight( minHeight1 * 8 );
   connect( lb_files, SIGNAL( selectionChanged() ), SLOT( update_files() ) );

   lbl_selected = new QLabel("0 files selected", this );
   lbl_selected->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_selected->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_selected );
   lbl_selected->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2));

   pb_select_all = new mQPushButton(tr("Select all"), this);
   pb_select_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_select_all->setMinimumHeight(minHeight1);
   pb_select_all->setPalette( PALET_PUSHB );
   connect(pb_select_all, SIGNAL(clicked()), SLOT(select_all()));

   pb_invert = new QPushButton(tr("Invert"), this);
   pb_invert->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_invert->setMinimumHeight(minHeight1);
   pb_invert->setPalette( PALET_PUSHB );
   connect(pb_invert, SIGNAL(clicked()), SLOT(invert()));

   pb_select_nth = new mQPushButton(tr("Select"), this);
   pb_select_nth->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_select_nth->setMinimumHeight(minHeight1);
   pb_select_nth->setPalette( PALET_PUSHB );
   connect(pb_select_nth, SIGNAL(clicked()), SLOT(select_nth()));

   pb_line_width = new QPushButton(tr("Width"), this);
   pb_line_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_line_width->setMinimumHeight(minHeight1);
   pb_line_width->setMaximumWidth ( minHeight1 * 2 );
   pb_line_width->setPalette( PALET_PUSHB );
   connect(pb_line_width, SIGNAL(clicked()), SLOT(line_width()));

   pb_color_rotate = new QPushButton(tr("Color"), this);
   pb_color_rotate->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_color_rotate->setMinimumHeight(minHeight1);
   pb_color_rotate->setMaximumWidth ( minHeight1 * 2 );
   pb_color_rotate->setPalette( PALET_PUSHB );
   connect(pb_color_rotate, SIGNAL(clicked()), SLOT(color_rotate()));

   //    pb_join = new QPushButton(tr("J"), this);
   //    pb_join->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   //    pb_join->setMinimumHeight( minHeight1 );
   //    pb_join->setMaximumWidth ( minHeight1 * 2 );
   //    pb_join->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   //    connect(pb_join, SIGNAL(clicked()), SLOT(join()));

   //    pb_adjacent = new QPushButton(tr("Similar"), this);
   //    pb_adjacent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   //    pb_adjacent->setMinimumHeight(minHeight1);
   //    pb_adjacent->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   //    connect(pb_adjacent, SIGNAL(clicked()), SLOT(adjacent()));

   pb_to_saxs = new QPushButton(tr("To SOMO/SAS"), this);
   pb_to_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_to_saxs->setMinimumHeight( minHeight1 );
   // pb_to_saxs->setMaximumWidth ( minHeight1 * 2 );
   pb_to_saxs->setPalette( PALET_PUSHB );
   connect(pb_to_saxs, SIGNAL(clicked()), SLOT(to_saxs()));

   pb_view = new QPushButton(tr("View"), this);
   pb_view->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_view->setMinimumHeight( minHeight1 );
   pb_view->setMaximumWidth ( minHeight1 * 4 );
   pb_view->setPalette( PALET_PUSHB );
   connect(pb_view, SIGNAL(clicked()), SLOT( view() ));

   pb_movie = new mQPushButton(tr("M"), this);
   pb_movie->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_movie->setMinimumHeight(minHeight1);
   pb_movie->setPalette( PALET_PUSHB );
   connect(pb_movie, SIGNAL(clicked()), SLOT(movie()));

   pb_rescale = new QPushButton(tr("Rescale"), this);
   pb_rescale->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_rescale->setMinimumHeight(minHeight1);
   pb_rescale->setPalette( PALET_PUSHB );
   connect(pb_rescale, SIGNAL(clicked()), SLOT(rescale()));

   pb_ag = new QPushButton(tr("AG"), this);
   pb_ag->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_ag->setMinimumHeight(minHeight1);
   pb_ag->setPalette( PALET_PUSHB );
   connect( pb_ag, SIGNAL( clicked() ), SLOT( artificial_gaussians() ) );

   pb_stack_push_all = new QPushButton(tr("Psh"), this);
   pb_stack_push_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_push_all->setMinimumHeight(minHeight1);
   pb_stack_push_all->setPalette( PALET_PUSHB );
   connect(pb_stack_push_all, SIGNAL(clicked()), SLOT(stack_push_all()));

   pb_stack_push_sel = new QPushButton(tr("Psl"), this);
   pb_stack_push_sel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_push_sel->setMinimumHeight(minHeight1);
   pb_stack_push_sel->setPalette( PALET_PUSHB );
   connect(pb_stack_push_sel, SIGNAL(clicked()), SLOT(stack_push_sel()));

   lbl_stack = new QLabel( "", this );
   lbl_stack->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_stack->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_stack );
   lbl_stack->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2));

   pb_stack_copy = new QPushButton(tr("Cpy"), this);
   pb_stack_copy->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_copy->setMinimumHeight(minHeight1);
   pb_stack_copy->setPalette( PALET_PUSHB );
   connect(pb_stack_copy, SIGNAL(clicked()), SLOT(stack_copy()));

   pb_stack_pcopy = new QPushButton(tr("Pcp"), this);
   pb_stack_pcopy->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_pcopy->setMinimumHeight(minHeight1);
   pb_stack_pcopy->setPalette( PALET_PUSHB );
   connect(pb_stack_pcopy, SIGNAL(clicked()), SLOT(stack_pcopy()));

   pb_stack_paste = new QPushButton(tr("Pst"), this);
   pb_stack_paste->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_paste->setMinimumHeight(minHeight1);
   pb_stack_paste->setPalette( PALET_PUSHB );
   connect(pb_stack_paste, SIGNAL(clicked()), SLOT(stack_paste()));

   pb_stack_drop = new QPushButton(tr("Drp"), this);
   pb_stack_drop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_drop->setMinimumHeight(minHeight1);
   pb_stack_drop->setPalette( PALET_PUSHB );
   connect(pb_stack_drop, SIGNAL(clicked()), SLOT(stack_drop()));

   pb_stack_join = new QPushButton(tr("Jn"), this);
   pb_stack_join->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_join->setMinimumHeight(minHeight1);
   pb_stack_join->setPalette( PALET_PUSHB );
   connect(pb_stack_join, SIGNAL(clicked()), SLOT(stack_join()));

   pb_stack_rot_up = new QPushButton(tr("Rdn"), this);
   pb_stack_rot_up->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_rot_up->setMinimumHeight(minHeight1);
   pb_stack_rot_up->setPalette( PALET_PUSHB );
   connect(pb_stack_rot_up, SIGNAL(clicked()), SLOT(stack_rot_up()));

   pb_stack_rot_down = new QPushButton(tr("Rup"), this);
   pb_stack_rot_down->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_rot_down->setMinimumHeight(minHeight1);
   pb_stack_rot_down->setPalette( PALET_PUSHB );
   connect(pb_stack_rot_down, SIGNAL(clicked()), SLOT(stack_rot_down()));

   pb_stack_swap = new QPushButton(tr("Swp"), this);
   pb_stack_swap->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_swap->setMinimumHeight(minHeight1);
   pb_stack_swap->setPalette( PALET_PUSHB );
   connect(pb_stack_swap, SIGNAL(clicked()), SLOT(stack_swap()));

   // pb_plot_files = new QPushButton(tr("Plot"), this);
   // pb_plot_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   // pb_plot_files->setMinimumHeight(minHeight1);
   // pb_plot_files->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   // connect(pb_plot_files, SIGNAL(clicked()), SLOT(plot_files()));

   pb_conc_avg = new QPushButton(tr("Conc. norm. avg."), this);
   pb_conc_avg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_conc_avg->setMinimumHeight(minHeight1);
   pb_conc_avg->setPalette( PALET_PUSHB );
   connect(pb_conc_avg, SIGNAL(clicked()), SLOT(conc_avg()));

   pb_normalize = new QPushButton(tr("Normalize"), this);
   pb_normalize->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_normalize->setMinimumHeight(minHeight1);
   pb_normalize->setPalette( PALET_PUSHB );
   connect(pb_normalize, SIGNAL(clicked()), SLOT(normalize()));

   pb_add = new QPushButton(tr("Add"), this);
   pb_add->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_add->setMinimumHeight(minHeight1);
   pb_add->setPalette( PALET_PUSHB );
   connect(pb_add, SIGNAL(clicked()), SLOT(add()));

   pb_avg = new QPushButton(tr("Average"), this);
   pb_avg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_avg->setMinimumHeight(minHeight1);
   pb_avg->setPalette( PALET_PUSHB );
   connect(pb_avg, SIGNAL(clicked()), SLOT(avg()));

   pb_smooth = new QPushButton(tr("Smooth"), this);
   pb_smooth->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_smooth->setMinimumHeight(minHeight1);
   pb_smooth->setPalette( PALET_PUSHB );
   connect(pb_smooth, SIGNAL(clicked()), SLOT(smooth()));

   pb_repeak = new QPushButton(tr("Repeak"), this);
   pb_repeak->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_repeak->setMinimumHeight(minHeight1);
   pb_repeak->setPalette( PALET_PUSHB );
   connect(pb_repeak, SIGNAL(clicked()), SLOT(repeak()));

   pb_svd = new QPushButton(tr("SVD"), this);
   pb_svd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_svd->setMinimumHeight(minHeight1);
   pb_svd->setPalette( PALET_PUSHB );
   connect(pb_svd, SIGNAL(clicked()), SLOT(svd()));

   pb_create_i_of_t = new QPushButton(tr("Make I(t)"), this);
   pb_create_i_of_t->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_create_i_of_t->setMinimumHeight(minHeight1);
   pb_create_i_of_t->setPalette( PALET_PUSHB );
   connect(pb_create_i_of_t, SIGNAL(clicked()), SLOT(create_i_of_t()));

   pb_test_i_of_t = new QPushButton(tr("Test I(t)"), this);
   pb_test_i_of_t->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_test_i_of_t->setMinimumHeight(minHeight1);
   pb_test_i_of_t->setPalette( PALET_PUSHB );
   connect(pb_test_i_of_t, SIGNAL(clicked()), SLOT(test_i_of_t()));

   pb_create_i_of_q = new QPushButton(tr("Make I(q)"), this);
   pb_create_i_of_q->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_create_i_of_q->setMinimumHeight(minHeight1);
   pb_create_i_of_q->setPalette( PALET_PUSHB );
   connect(pb_create_i_of_q, SIGNAL(clicked()), SLOT(create_i_of_q()));

   pb_conc_file = new QPushButton(tr("Set concentration file"), this);
   pb_conc_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_conc_file->setMinimumHeight(minHeight1);
   pb_conc_file->setPalette( PALET_PUSHB );
   connect(pb_conc_file, SIGNAL(clicked()), SLOT(set_conc_file()));

   lbl_conc_file = new QLabel("", this );
   lbl_conc_file->setMinimumHeight(minHeight1);
   lbl_conc_file->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_conc_file->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_conc_file );
   lbl_conc_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   pb_detector = new QPushButton(tr("Concentration detector"), this);
   pb_detector->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_detector->setMinimumHeight(minHeight1);
   pb_detector->setPalette( PALET_PUSHB );
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

   lbl_created_files = new mQLabel("Produced Data", this);
   lbl_created_files->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_created_files->setMinimumHeight(minHeight1);
   lbl_created_files->setPalette( PALET_LABEL );
   AUTFBACK( lbl_created_files );
   lbl_created_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   connect( lbl_created_files, SIGNAL( pressed() ), SLOT( hide_created_files() ) );

   lbl_created_dir = new mQLabel( QDir::currentDirPath(), this );
   lbl_created_dir->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_created_dir->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_created_dir );
   lbl_created_dir->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2));
   connect( lbl_created_dir, SIGNAL(pressed()), SLOT( created_dir_pressed() ));

   lb_created_files = new Q3ListBox(this, "created_files created_files listbox" );
   lb_created_files->setPalette( PALET_NORMAL );
   AUTFBACK( lb_created_files );
   lb_created_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lb_created_files->setEnabled(true);
   lb_created_files->setSelectionMode( Q3ListBox::Extended );
   lb_created_files->setMinimumHeight( minHeight1 * 3 );
   connect( lb_created_files, SIGNAL( selectionChanged() ), SLOT( update_created_files() ) );
   connect( lb_created_files, 
            SIGNAL( rightButtonClicked( Q3ListBoxItem *, const QPoint & ) ),
            SLOT  ( rename_created    ( Q3ListBoxItem *, const QPoint & ) ) );

   lbl_selected_created = new QLabel("0 files selected", this );
   lbl_selected_created->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_selected_created->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_selected_created );
   lbl_selected_created->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2));

   pb_select_all_created = new QPushButton(tr("Select all"), this);
   pb_select_all_created->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize  - 1));
   pb_select_all_created->setMinimumHeight(minHeight1);
   pb_select_all_created->setPalette( PALET_PUSHB );
   connect(pb_select_all_created, SIGNAL(clicked()), SLOT(select_all_created()));

   pb_invert_all_created = new QPushButton(tr("Invert"), this);
   pb_invert_all_created->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize  - 1));
   pb_invert_all_created->setMinimumHeight(minHeight1);
   pb_invert_all_created->setPalette( PALET_PUSHB );
   connect(pb_invert_all_created, SIGNAL(clicked()), SLOT(invert_all_created()));

   pb_adjacent_created = new QPushButton(tr("Similar"), this);
   pb_adjacent_created->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_adjacent_created->setMinimumHeight(minHeight1);
   pb_adjacent_created->setPalette( PALET_PUSHB );
   connect(pb_adjacent_created, SIGNAL(clicked()), SLOT(adjacent_created()));

   pb_remove_created = new QPushButton(tr("Remove"), this);
   pb_remove_created->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_remove_created->setMinimumHeight(minHeight1);
   pb_remove_created->setPalette( PALET_PUSHB );
   connect(pb_remove_created, SIGNAL(clicked()), SLOT(remove_created()));

   pb_save_created_csv = new QPushButton( "", this);
   pb_save_created_csv->setText( (( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "hplc_csv_transposed" ) &&
                                 (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_csv_transposed" ] == "true" ?
                                 tr( "Save CSV Tr" ) : tr( " Save CSV " ) );
   pb_save_created_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_save_created_csv->setMinimumHeight(minHeight1);
   pb_save_created_csv->setPalette( PALET_PUSHB );
   connect(pb_save_created_csv, SIGNAL(clicked()), SLOT(save_created_csv()));

   pb_save_created = new QPushButton(tr("Save"), this);
   pb_save_created->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_save_created->setMinimumHeight(minHeight1);
   pb_save_created->setPalette( PALET_PUSHB );
   connect(pb_save_created, SIGNAL(clicked()), SLOT(save_created()));

   pb_show_created = new QPushButton(tr("Show"), this);
   pb_show_created->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_show_created->setMinimumHeight(minHeight1);
   pb_show_created->setPalette( PALET_PUSHB );
   connect(pb_show_created, SIGNAL(clicked()), SLOT(show_created()));

   pb_show_only_created = new QPushButton(tr("Show only"), this);
   pb_show_only_created->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_show_only_created->setMinimumHeight(minHeight1);
   pb_show_only_created->setPalette( PALET_PUSHB );
   connect(pb_show_only_created, SIGNAL(clicked()), SLOT(show_only_created()));

   // models

   lbl_model_files = new QLabel("Model files", this);
   lbl_model_files->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_model_files->setMinimumHeight(minHeight1);
   lbl_model_files->setPalette( PALET_LABEL );
   AUTFBACK( lbl_model_files );
   lbl_model_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   model_widgets.push_back( lbl_model_files );

   lb_model_files = new Q3ListBox(this, "model_files model_files listbox" );
   lb_model_files->setPalette( PALET_NORMAL );
   AUTFBACK( lb_model_files );
   lb_model_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lb_model_files->setEnabled(true);
   lb_model_files->setSelectionMode( Q3ListBox::Extended );
   lb_model_files->setMinimumHeight( minHeight1 * 2 );
   connect( lb_model_files, SIGNAL( selectionChanged() ), SLOT( update_enables() ) );
   model_widgets.push_back( lb_model_files );

   pb_model_select_all = new mQPushButton(tr("Select all"), this);
   pb_model_select_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_model_select_all->setMinimumHeight(minHeight1);
   pb_model_select_all->setPalette( PALET_PUSHB );
   connect(pb_model_select_all, SIGNAL(clicked()), SLOT(model_select_all()));
   model_widgets.push_back( pb_model_select_all );

   pb_model_text = new QPushButton(tr("Text"), this);
   pb_model_text->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_model_text->setMinimumHeight(minHeight1);
   pb_model_text->setPalette( PALET_PUSHB );
   connect(pb_model_text, SIGNAL(clicked()), SLOT(model_text()));
   model_widgets.push_back( pb_model_text );

   pb_model_view = new QPushButton(tr("View"), this);
   pb_model_view->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_model_view->setMinimumHeight(minHeight1);
   pb_model_view->setPalette( PALET_PUSHB );
   connect(pb_model_view, SIGNAL(clicked()), SLOT(model_view()));
   model_widgets.push_back( pb_model_view );

   pb_model_remove = new QPushButton(tr("Remove"), this);
   pb_model_remove->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_model_remove->setMinimumHeight(minHeight1);
   pb_model_remove->setPalette( PALET_PUSHB );
   connect(pb_model_remove, SIGNAL(clicked()), SLOT(model_remove()));
   model_widgets.push_back( pb_model_remove );

   pb_model_save = new QPushButton(tr("Save"), this);
   pb_model_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_model_save->setMinimumHeight(minHeight1);
   pb_model_save->setPalette( PALET_PUSHB );
   connect(pb_model_save, SIGNAL(clicked()), SLOT(model_save()));
   model_widgets.push_back( pb_model_save );

   le_dummy = new QLineEdit(this, "le_dummy Line Edit");
   le_dummy->setText( "" );
   le_dummy->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_dummy->setPalette( PALET_NORMAL );
   AUTFBACK( le_dummy );
   le_dummy->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 3));
   le_dummy->setMaximumHeight( 3 );
   le_dummy->setEnabled( false );

   progress = new Q3ProgressBar(this, "Progress");
   // progress->setMinimumHeight(minHeight1);
   progress->setPalette( PALET_NORMAL );
   AUTFBACK( progress );
   progress->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   progress->reset();

   lbl_editor = new mQLabel("Messages", this);
   lbl_editor->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_editor->setMinimumHeight(minHeight1);
   lbl_editor->setPalette( PALET_LABEL );
   AUTFBACK( lbl_editor );
   lbl_editor->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   connect( lbl_editor, SIGNAL( pressed() ), SLOT( hide_editor() ) );
   // lbl_editor->hide();

   editor = new Q3TextEdit(this);
   editor->setPalette( PALET_NORMAL );
   AUTFBACK( editor );
   editor->setReadOnly(true);
   editor->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ));


#if defined(QT4) && defined(Q_WS_MAC)
   {
      Q3PopupMenu * file = new Q3PopupMenu;
      file->insertItem( tr("&Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
      file->insertItem( tr("&Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );
      file->insertItem( tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );

      QMenuBar *menu = new QMenuBar( this );
      AUTFBACK( menu );

      menu->insertItem(tr("&Messages"), file );
   }
#else
   Q3Frame *frame;
   frame = new Q3Frame(this);
   frame->setMinimumHeight(minHeight3);
   editor_widgets.push_back( frame );

   m = new QMenuBar(frame, "menu" );
   m->setMinimumHeight(minHeight1 - 5);
   m->setPalette( PALET_NORMAL );
   AUTFBACK( m );
   Q3PopupMenu * file = new Q3PopupMenu(editor);
   m->insertItem( tr("&File"), file );
   file->insertItem( tr("Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
   file->insertItem( tr("Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );
   file->insertItem( tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );
#endif

   editor->setWordWrap (Q3TextEdit::WidgetWidth);
   editor->setMinimumHeight( minHeight1 * 3 );
   editor_widgets.push_back( editor );

   QSplitter *qs = new QSplitter( Qt::Vertical, this );

   plot_dist = new QwtPlot( qs );
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
   plot_dist->setPalette( PALET_NORMAL );
   AUTFBACK( plot_dist );
#ifndef QT4
   plot_dist->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot_dist->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   grid_saxs->setMajPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid_saxs->setMinPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid_saxs->attach( plot_dist );
#endif
   plot_dist->setAxisTitle(QwtPlot::xBottom, /* cb_guinier->isChecked() ? tr("q^2 (1/Angstrom^2)") : */  tr("q [1/Angstrom]" )); // or Time or Frame"));
   plot_dist->setAxisTitle(QwtPlot::yLeft, tr("Intensity [a.u.] (log scale)"));
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
   {
      QwtLegend* legend_pd = new QwtLegend;
      legend_pd->setFrameStyle( Q3Frame::Box | Q3Frame::Sunken );
      plot_dist->insertLegend( legend_pd, QwtPlot::BottomLegend );
   }
#endif
   connect( plot_dist->canvas(), SIGNAL( mouseReleased( const QMouseEvent & ) ), SLOT( plot_mouse(  const QMouseEvent & ) ) );

   plot_ref = new QwtPlot( qs );
#ifndef QT4
   // plot_ref->enableOutline(true);
   // plot_ref->setOutlinePen(Qt::white);
   // plot_ref->setOutlineStyle(Qwt::VLine);
   plot_ref->enableGridXMin();
   plot_ref->enableGridYMin();
#else
   grid_ref = new QwtPlotGrid;
   grid_ref->enableXMin( true );
   grid_ref->enableYMin( true );
#endif
   plot_ref->setPalette( PALET_NORMAL );
   AUTFBACK( plot_ref );
#ifndef QT4
   plot_ref->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot_ref->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   grid_ref->setMajPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid_ref->setMinPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid_ref->attach( plot_ref );
#endif
   plot_ref->setAxisTitle(QwtPlot::xBottom, tr( "Time [a.u.]" ) );
   plot_ref->setAxisTitle(QwtPlot::yLeft, tr("Intensity [a.u.]"));
#ifndef QT4
   plot_ref->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   plot_ref->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_ref->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#ifndef QT4
   plot_ref->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_ref->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#ifndef QT4
   plot_ref->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_ref->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_ref->setMargin(USglobal->config_list.margin);
   plot_ref->setTitle("");
#ifndef QT4
   // plot_ref->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::Logarithmic);
#else
   // plot_ref->setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);
#endif
   plot_ref->setCanvasBackground(USglobal->global_colors.plot);
   plot_ref->hide();
   plot_ref->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding)); 
   plot_errors = new QwtPlot( this );
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
   plot_errors->setPalette( PALET_NORMAL );
   AUTFBACK( plot_errors );
#ifndef QT4
   plot_errors->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot_errors->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   grid_errors->setMajPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid_errors->setMinPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid_errors->attach( plot_errors );
#endif
   // plot_errors->setAxisTitle(QwtPlot::xBottom, /* cb_guinier->isChecked() ? tr("q^2 (1/Angstrom^2)") : */  tr("q (1/Angstrom) or Frame"));
   // plot_errors->setAxisTitle(QwtPlot::yLeft, tr("I(q) (log scale)"));
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
   cb_plot_errors_rev->setPalette( PALET_NORMAL );
   AUTFBACK( cb_plot_errors_rev );
   connect( cb_plot_errors_rev, SIGNAL( clicked() ), SLOT( set_plot_errors_rev() ) );

   cb_plot_errors_sd = new QCheckBox(this);
   cb_plot_errors_sd->setText(tr("Use standard deviations  "));
   cb_plot_errors_sd->setEnabled( true );
   cb_plot_errors_sd->setChecked( false );
   cb_plot_errors_sd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_plot_errors_sd->setPalette( PALET_NORMAL );
   AUTFBACK( cb_plot_errors_sd );
   connect( cb_plot_errors_sd, SIGNAL( clicked() ), SLOT( set_plot_errors_sd() ) );

   cb_plot_errors_pct = new QCheckBox(this);
   cb_plot_errors_pct->setText(tr("By percent "));
   cb_plot_errors_pct->setEnabled( true );
   cb_plot_errors_pct->setChecked( false );
   cb_plot_errors_pct->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_plot_errors_pct->setPalette( PALET_NORMAL );
   AUTFBACK( cb_plot_errors_pct );
   connect( cb_plot_errors_pct, SIGNAL( clicked() ), SLOT( set_plot_errors_pct() ) );

   cb_plot_errors_group = new QCheckBox(this);
   cb_plot_errors_group->setText(tr("Group"));
   cb_plot_errors_group->setEnabled( true );
   cb_plot_errors_group->setChecked( false );
   cb_plot_errors_group->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_plot_errors_group->setPalette( PALET_NORMAL );
   AUTFBACK( cb_plot_errors_group );
   connect( cb_plot_errors_group, SIGNAL( clicked() ), SLOT( set_plot_errors_group() ) );

   plot_errors->setAxisTitle(QwtPlot::yLeft, tr( cb_plot_errors_pct->isChecked() ?
                                                 "%" : "delta I(q)" ) );

   Q3HBoxLayout *hbl_plot_errors_buttons = new Q3HBoxLayout( 0 );
   hbl_plot_errors_buttons->addWidget( cb_plot_errors_rev );
   hbl_plot_errors_buttons->addWidget( cb_plot_errors_sd );
   hbl_plot_errors_buttons->addWidget( cb_plot_errors_pct );
   hbl_plot_errors_buttons->addWidget( cb_plot_errors_group );

   l_plot_errors = new Q3VBoxLayout( 0 );
   l_plot_errors->addWidget( plot_errors );
   l_plot_errors->addLayout( hbl_plot_errors_buttons );

   plot_errors_widgets.push_back( plot_errors );
   plot_errors_widgets.push_back( cb_plot_errors_rev );
   plot_errors_widgets.push_back( cb_plot_errors_sd );
   plot_errors_widgets.push_back( cb_plot_errors_pct );
   plot_errors_widgets.push_back( cb_plot_errors_group );

   hide_widgets( plot_errors_widgets, true );

   pb_wheel_start = new QPushButton(tr("Timeshift"), this);
   pb_wheel_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_wheel_start->setMinimumHeight(minHeight1);
   pb_wheel_start->setPalette( PALET_PUSHB );
   connect(pb_wheel_start, SIGNAL(clicked()), SLOT(wheel_start()));

   pb_p3d = new QPushButton(tr("3D"), this);
   pb_p3d->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_p3d->setMinimumHeight(minHeight1);
   pb_p3d->setPalette( PALET_PUSHB );
   connect(pb_p3d, SIGNAL(clicked()), SLOT(p3d()));

   lbl_blank1 = new QLabel( "", this );
   lbl_blank1->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_blank1->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_blank1 );
   lbl_blank1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   lbl_wheel_pos = new QLabel( "", this );
   lbl_wheel_pos->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_wheel_pos->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_wheel_pos );
   lbl_wheel_pos->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   qwtw_wheel = new QwtWheel( this );
   qwtw_wheel->setMass         ( 0.5 );
   // qwtw_wheel->setRange        ( -1000, 1000, 1 );
   qwtw_wheel->setMinimumHeight( minHeight1 );
   // qwtw_wheel->setTotalAngle( 3600.0 );
   qwtw_wheel->setEnabled      ( false );
   connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );

   pb_ref = new QPushButton(tr("Concentration reference"), this);
   pb_ref->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_ref->setMinimumHeight(minHeight1);
   pb_ref->setPalette( PALET_PUSHB );
   pb_ref->setEnabled(false);
   connect(pb_ref, SIGNAL(clicked()), SLOT(ref()));

   pb_errors = new QPushButton(tr("Residuals"), this);
   pb_errors->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_errors->setMinimumHeight(minHeight1);
   pb_errors->setPalette( PALET_PUSHB );
   pb_errors->setEnabled(false);
   connect(pb_errors, SIGNAL(clicked()), SLOT(errors()));

   pb_wheel_cancel = new QPushButton(tr("Cancel"), this);
   pb_wheel_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_wheel_cancel->setMinimumHeight(minHeight1);
   pb_wheel_cancel->setPalette( PALET_PUSHB );
   pb_wheel_cancel->setEnabled(false);
   connect(pb_wheel_cancel, SIGNAL(clicked()), SLOT(wheel_cancel()));

   pb_wheel_save = new QPushButton(tr("Keep"), this);
   pb_wheel_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_wheel_save->setMinimumHeight(minHeight1);
   pb_wheel_save->setPalette( PALET_PUSHB );
   pb_wheel_save->setEnabled(false);
   connect(pb_wheel_save, SIGNAL(clicked()), SLOT(wheel_save()));

   pb_gauss_start = new QPushButton(tr("Gaussians"), this);
   pb_gauss_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_gauss_start->setMinimumHeight(minHeight1);
   pb_gauss_start->setPalette( PALET_PUSHB );
   connect(pb_gauss_start, SIGNAL(clicked()), SLOT(gauss_start()));

   pb_gauss_clear = new QPushButton(tr("Clear"), this);
   pb_gauss_clear->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_gauss_clear->setMinimumHeight(minHeight1);
   pb_gauss_clear->setPalette( PALET_PUSHB );
   pb_gauss_clear->setEnabled( false );
   connect(pb_gauss_clear, SIGNAL(clicked()), SLOT(gauss_clear()));

   pb_gauss_new = new QPushButton(tr("New"), this);
   pb_gauss_new->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_gauss_new->setMinimumHeight(minHeight1);
   pb_gauss_new->setPalette( PALET_PUSHB );
   pb_gauss_new->setEnabled( false );
   connect(pb_gauss_new, SIGNAL(clicked()), SLOT(gauss_new()));

   pb_gauss_delete = new QPushButton(tr("Del"), this);
   pb_gauss_delete->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_gauss_delete->setMinimumHeight(minHeight1);
   pb_gauss_delete->setPalette( PALET_PUSHB );
   pb_gauss_delete->setEnabled( false );
   connect(pb_gauss_delete, SIGNAL(clicked()), SLOT(gauss_delete()));

   pb_gauss_prev = new QPushButton( "<", this);
   pb_gauss_prev->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_gauss_prev->setMinimumHeight(minHeight1);
   pb_gauss_prev->setPalette( PALET_PUSHB );
   pb_gauss_prev->setEnabled( false );
   connect(pb_gauss_prev, SIGNAL(clicked()), SLOT(gauss_prev()));

   lbl_gauss_pos = new QLabel( " 0 of 0 ", this );
   lbl_gauss_pos->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_gauss_pos->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_gauss_pos );
   lbl_gauss_pos->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   pb_gauss_next = new QPushButton( ">", this);
   pb_gauss_next->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_gauss_next->setMinimumHeight(minHeight1);
   pb_gauss_next->setPalette( PALET_PUSHB );
   pb_gauss_next->setEnabled( false );
   connect(pb_gauss_next, SIGNAL(clicked()), SLOT(gauss_next()));


   le_gauss_pos = new mQLineEdit(this, "le_gauss_pos Line Edit");
   le_gauss_pos->setText( "" );
   le_gauss_pos->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_gauss_pos->setPalette( PALET_NORMAL );
   AUTFBACK( le_gauss_pos );
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
   le_gauss_pos_width->setPalette( PALET_NORMAL );
   AUTFBACK( le_gauss_pos_width );
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
   le_gauss_pos_height->setPalette( PALET_NORMAL );
   AUTFBACK( le_gauss_pos_height );
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

   le_gauss_pos_dist1 = new mQLineEdit(this, "le_gauss_pos_dist1 Line Edit");
   le_gauss_pos_dist1->setText( "" );
   le_gauss_pos_dist1->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_gauss_pos_dist1->setPalette( PALET_NORMAL );
   AUTFBACK( le_gauss_pos_dist1 );
   le_gauss_pos_dist1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_gauss_pos_dist1->setEnabled( false );
   {
      QDoubleValidator *qdv = new QDoubleValidator( le_gauss_pos_dist1 );
      qdv->setDecimals( UHSH_VAL_DEC );
      // le_gauss_pos_dist1->setValidator( qdv );
   }
   // le_gauss_pos_dist1->setValidator( new QDoubleValidator( le_gauss_pos_dist1 ) );
   // le_gauss_pos_dist1->validator()->setDecimals( UHSH_VAL_DEC );
   connect( le_gauss_pos_dist1, SIGNAL( textChanged( const QString & ) ), SLOT( gauss_pos_dist1_text( const QString & ) ) );
   connect( le_gauss_pos_dist1, SIGNAL( focussed ( bool ) )             , SLOT( gauss_pos_dist1_focus( bool ) ) );

   le_gauss_pos_dist2 = new mQLineEdit(this, "le_gauss_pos_dist2 Line Edit");
   le_gauss_pos_dist2->setText( "" );
   le_gauss_pos_dist2->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_gauss_pos_dist2->setPalette( PALET_NORMAL );
   AUTFBACK( le_gauss_pos_dist2 );
   le_gauss_pos_dist2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_gauss_pos_dist2->setEnabled( false );
   {
      QDoubleValidator *qdv = new QDoubleValidator( le_gauss_pos_dist2 );
      qdv->setDecimals( UHSH_VAL_DEC );
      // le_gauss_pos_dist2->setValidator( qdv );
   }
   // le_gauss_pos_dist2->setValidator( new QDoubleValidator( le_gauss_pos_dist2 ) );
   // le_gauss_pos_dist2->validator()->setDecimals( UHSH_VAL_DEC );
   connect( le_gauss_pos_dist2, SIGNAL( textChanged( const QString & ) ), SLOT( gauss_pos_dist2_text( const QString & ) ) );
   connect( le_gauss_pos_dist2, SIGNAL( focussed ( bool ) )             , SLOT( gauss_pos_dist2_focus( bool ) ) );

   cb_gauss_match_amplitude = new QCheckBox(this);
   cb_gauss_match_amplitude->setText(tr("Match "));
   cb_gauss_match_amplitude->setEnabled( true );
   cb_gauss_match_amplitude->setChecked( false );
   cb_gauss_match_amplitude->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_gauss_match_amplitude->setPalette( PALET_NORMAL );
   AUTFBACK( cb_gauss_match_amplitude );
   connect( cb_gauss_match_amplitude, SIGNAL( clicked() ), SLOT( gauss_match_amplitude() ) );

   cb_sd_weight = new QCheckBox(this);
   cb_sd_weight->setText(tr("SD  "));
   cb_sd_weight->setEnabled( true );
   cb_sd_weight->setChecked( false );
   cb_sd_weight->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_sd_weight->setPalette( PALET_NORMAL );
   AUTFBACK( cb_sd_weight );
   connect( cb_sd_weight, SIGNAL( clicked() ), SLOT( set_sd_weight() ) );

   cb_fix_width = new QCheckBox(this);
   cb_fix_width->setText(tr("Eq width  "));
   cb_fix_width->setEnabled( U_EXPT );
   cb_fix_width->setChecked( true );
   cb_fix_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_fix_width->setPalette( PALET_NORMAL );
   AUTFBACK( cb_fix_width );
   connect( cb_fix_width, SIGNAL( clicked() ), SLOT( set_fix_width() ) );
   // if ( !U_EXPT )
   // {
   // cb_fix_width->hide();
   // }

   cb_fix_dist1 = new QCheckBox(this);
   cb_fix_dist1->setText(tr("Eq dist1  "));
   cb_fix_dist1->setEnabled( U_EXPT );
   cb_fix_dist1->setChecked( true );
   cb_fix_dist1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_fix_dist1->setPalette( PALET_NORMAL );
   AUTFBACK( cb_fix_dist1 );
   connect( cb_fix_dist1, SIGNAL( clicked() ), SLOT( set_fix_dist1() ) );

   cb_fix_dist2 = new QCheckBox(this);
   cb_fix_dist2->setText(tr("Eq dist2  "));
   cb_fix_dist2->setEnabled( U_EXPT );
   cb_fix_dist2->setChecked( true );
   cb_fix_dist2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_fix_dist2->setPalette( PALET_NORMAL );
   AUTFBACK( cb_fix_dist2 );
   connect( cb_fix_dist2, SIGNAL( clicked() ), SLOT( set_fix_dist2() ) );

   pb_gauss_fit = new QPushButton(tr("Fit"), this);
   pb_gauss_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_gauss_fit->setMinimumHeight(minHeight1);
   pb_gauss_fit->setPalette( PALET_PUSHB );
   pb_gauss_fit->setEnabled( false );
   connect(pb_gauss_fit, SIGNAL(clicked()), SLOT(gauss_fit()));

   lbl_gauss_fit = new QLabel( "", this );
   lbl_gauss_fit->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_gauss_fit->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_gauss_fit );
   lbl_gauss_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_gauss_fit_start = new mQLineEdit(this, "le_gauss_fit_start Line Edit");
   le_gauss_fit_start->setText( "" );
   le_gauss_fit_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_gauss_fit_start->setPalette(QPalette( cg_red, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_gauss_fit_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_gauss_fit_start->setEnabled( false );
   le_gauss_fit_start->setValidator( new QDoubleValidator( le_gauss_fit_start ) );
   connect( le_gauss_fit_start, SIGNAL( textChanged( const QString & ) ), SLOT( gauss_fit_start_text( const QString & ) ) );
   connect( le_gauss_fit_start, SIGNAL( focussed ( bool ) )             , SLOT( gauss_fit_start_focus( bool ) ) );

   le_gauss_fit_end = new mQLineEdit(this, "le_gauss_fit_end Line Edit");
   le_gauss_fit_end->setText( "" );
   le_gauss_fit_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_gauss_fit_end->setPalette(QPalette( cg_red, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_gauss_fit_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_gauss_fit_end->setEnabled( false );
   le_gauss_fit_end->setValidator( new QDoubleValidator( le_gauss_fit_end ) );
   connect( le_gauss_fit_end, SIGNAL( textChanged( const QString & ) ), SLOT( gauss_fit_end_text( const QString & ) ) );
   connect( le_gauss_fit_end, SIGNAL( focussed ( bool ) )             , SLOT( gauss_fit_end_focus( bool ) ) );

   pb_gauss_save = new QPushButton(tr("Save"), this);
   pb_gauss_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_gauss_save->setMinimumHeight(minHeight1);
   pb_gauss_save->setPalette( PALET_PUSHB );
   pb_gauss_save->setEnabled( false );
   connect(pb_gauss_save, SIGNAL(clicked()), SLOT(gauss_save()));

   pb_ggauss_start = new QPushButton(tr("Global gaussians"), this);
   pb_ggauss_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_ggauss_start->setMinimumHeight(minHeight1);
   pb_ggauss_start->setPalette( PALET_PUSHB );
   connect(pb_ggauss_start, SIGNAL(clicked()), SLOT(ggauss_start()));

   pb_ggauss_rmsd = new QPushButton(tr("RMSD"), this);
   pb_ggauss_rmsd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_ggauss_rmsd->setMinimumHeight(minHeight1);
   pb_ggauss_rmsd->setPalette( PALET_PUSHB );
   pb_ggauss_rmsd->setEnabled( false );
   connect(pb_ggauss_rmsd, SIGNAL(clicked()), SLOT(ggauss_rmsd()));

   pb_ggauss_results = new QPushButton(tr("Make result curves"), this);
   pb_ggauss_results->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_ggauss_results->setMinimumHeight(minHeight1);
   pb_ggauss_results->setPalette( PALET_PUSHB );
   pb_ggauss_results->setEnabled( false );
   connect(pb_ggauss_results, SIGNAL(clicked()), SLOT(ggauss_results()));
   // pb_ggauss_results->hide();

   pb_gauss_as_curves = new QPushButton(tr("To produced data"), this);
   pb_gauss_as_curves->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_gauss_as_curves->setMinimumHeight(minHeight1);
   pb_gauss_as_curves->setPalette( PALET_PUSHB );
   pb_gauss_as_curves->setEnabled( false );
   connect(pb_gauss_as_curves, SIGNAL(clicked()), SLOT(gauss_as_curves()));

   // baseline

   pb_baseline_start = new QPushButton(tr("Baseline"), this);
   pb_baseline_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_baseline_start->setMinimumHeight(minHeight1);
   pb_baseline_start->setPalette( PALET_PUSHB );
   connect(pb_baseline_start, SIGNAL(clicked()), SLOT(baseline_start()));

   pb_baseline_apply = new QPushButton(tr("Baseline apply"), this);
   pb_baseline_apply->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_baseline_apply->setMinimumHeight(minHeight1);
   pb_baseline_apply->setPalette( PALET_PUSHB );
   connect(pb_baseline_apply, SIGNAL(clicked()), SLOT(baseline_apply()));

   cb_baseline_start_zero = new QCheckBox(this);
   cb_baseline_start_zero->setText(tr("Zero base  "));
   cb_baseline_start_zero->setChecked( true );
   cb_baseline_start_zero->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_baseline_start_zero->setPalette( PALET_NORMAL );
   AUTFBACK( cb_baseline_start_zero );
   connect( cb_baseline_start_zero, SIGNAL( clicked() ), SLOT( set_baseline_start_zero() ) );

   le_baseline_start_s = new mQLineEdit(this, "le_baseline_start_s Line Edit");
   le_baseline_start_s->setText( "" );
   le_baseline_start_s->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_baseline_start_s->setPalette(QPalette( cg_magenta, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_baseline_start_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_baseline_start_s->setEnabled( false );
   le_baseline_start_s->setValidator( new QDoubleValidator( le_baseline_start_s ) );
   connect( le_baseline_start_s, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_start_s_text( const QString & ) ) );
   connect( le_baseline_start_s, SIGNAL( focussed ( bool ) )             , SLOT( baseline_start_s_focus( bool ) ) );

   le_baseline_start = new mQLineEdit(this, "le_baseline_start Line Edit");
   le_baseline_start->setText( "" );
   le_baseline_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_baseline_start->setPalette(QPalette( cg_red, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_baseline_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_baseline_start->setEnabled( false );
   le_baseline_start->setValidator( new QDoubleValidator( le_baseline_start ) );
   connect( le_baseline_start, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_start_text( const QString & ) ) );
   connect( le_baseline_start, SIGNAL( focussed ( bool ) )             , SLOT( baseline_start_focus( bool ) ) );

   le_baseline_start_e = new mQLineEdit(this, "le_baseline_start_e Line Edit");
   le_baseline_start_e->setText( "" );
   le_baseline_start_e->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_baseline_start_e->setPalette(QPalette( cg_magenta, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_baseline_start_e->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_baseline_start_e->setEnabled( false );
   le_baseline_start_e->setValidator( new QDoubleValidator( le_baseline_start_e ) );
   connect( le_baseline_start_e, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_start_e_text( const QString & ) ) );
   connect( le_baseline_start_e, SIGNAL( focussed ( bool ) )             , SLOT( baseline_start_e_focus( bool ) ) );

   le_baseline_end_s = new mQLineEdit(this, "le_baseline_end_s Line Edit");
   le_baseline_end_s->setText( "" );
   le_baseline_end_s->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_baseline_end_s->setPalette(QPalette( cg_magenta, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_baseline_end_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_baseline_end_s->setEnabled( false );
   le_baseline_end_s->setValidator( new QDoubleValidator( le_baseline_end_s ) );
   connect( le_baseline_end_s, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_s_text( const QString & ) ) );
   connect( le_baseline_end_s, SIGNAL( focussed ( bool ) )             , SLOT( baseline_end_s_focus( bool ) ) );

   le_baseline_end = new mQLineEdit(this, "le_baseline_end Line Edit");
   le_baseline_end->setText( "" );
   le_baseline_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_baseline_end->setPalette(QPalette( cg_red, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_baseline_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_baseline_end->setEnabled( false );
   le_baseline_end->setValidator( new QDoubleValidator( le_baseline_end ) );
   connect( le_baseline_end, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_text( const QString & ) ) );
   connect( le_baseline_end, SIGNAL( focussed ( bool ) )             , SLOT( baseline_end_focus( bool ) ) );

   le_baseline_end_e = new mQLineEdit(this, "le_baseline_end_e Line Edit");
   le_baseline_end_e->setText( "" );
   le_baseline_end_e->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_baseline_end_e->setPalette(QPalette( cg_magenta, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_baseline_end_e->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_baseline_end_e->setEnabled( false );
   le_baseline_end_e->setValidator( new QDoubleValidator( le_baseline_end_e ) );
   connect( le_baseline_end_e, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_e_text( const QString & ) ) );
   connect( le_baseline_end_e, SIGNAL( focussed ( bool ) )             , SLOT( baseline_end_e_focus( bool ) ) );

   // select

   pb_select_vis = new QPushButton(tr("Select Visible"), this);
   pb_select_vis->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_select_vis->setMinimumHeight(minHeight1);
   pb_select_vis->setPalette( PALET_PUSHB );
   connect(pb_select_vis, SIGNAL(clicked()), SLOT(select_vis()));

   pb_remove_vis = new QPushButton(tr("Remove Vis"), this);
   pb_remove_vis->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_remove_vis->setMinimumHeight(minHeight1);
   pb_remove_vis->setPalette( PALET_PUSHB );
   connect(pb_remove_vis, SIGNAL(clicked()), SLOT(remove_vis()));

   pb_crop_common = new QPushButton(tr("Crop Common"), this);
   pb_crop_common->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_crop_common->setMinimumHeight(minHeight1);
   pb_crop_common->setPalette( PALET_PUSHB );
   connect(pb_crop_common, SIGNAL(clicked()), SLOT(crop_common()));

   pb_crop_vis = new QPushButton(tr("Crop Vis"), this);
   pb_crop_vis->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_crop_vis->setMinimumHeight(minHeight1);
   pb_crop_vis->setPalette( PALET_PUSHB );
   connect(pb_crop_vis, SIGNAL(clicked()), SLOT(crop_vis()));

   pb_crop_zero = new QPushButton(tr("Crop Zeros"), this);
   pb_crop_zero->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_crop_zero->setMinimumHeight(minHeight1);
   pb_crop_zero->setPalette( PALET_PUSHB );
   connect(pb_crop_zero, SIGNAL(clicked()), SLOT(crop_zero()));

   pb_crop_left = new QPushButton(tr("Crop Left"), this);
   pb_crop_left->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_crop_left->setMinimumHeight(minHeight1);
   pb_crop_left->setPalette( PALET_PUSHB );
   connect(pb_crop_left, SIGNAL(clicked()), SLOT(crop_left()));

   pb_crop_undo = new QPushButton(tr("Undo"), this);
   pb_crop_undo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_crop_undo->setMinimumHeight(minHeight1);
   pb_crop_undo->setPalette( PALET_PUSHB );
   connect(pb_crop_undo, SIGNAL(clicked()), SLOT(crop_undo()));

   pb_crop_right = new QPushButton(tr("Crop Right"), this);
   pb_crop_right->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_crop_right->setMinimumHeight(minHeight1);
   pb_crop_right->setPalette( PALET_PUSHB );
   connect(pb_crop_right, SIGNAL(clicked()), SLOT(crop_right()));

   pb_legend = new QPushButton(tr("Legend"), this);
   pb_legend->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_legend->setMinimumHeight(minHeight1);
   pb_legend->setPalette( PALET_PUSHB );
   connect(pb_legend, SIGNAL(clicked()), SLOT(legend()));

   pb_axis_x = new QPushButton(tr("X"), this);
   pb_axis_x->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_axis_x->setMinimumHeight(minHeight1);
   pb_axis_x->setPalette( PALET_PUSHB );
   connect(pb_axis_x, SIGNAL(clicked()), SLOT(axis_x()));

   pb_axis_y = new QPushButton(tr("Y"), this);
   pb_axis_y->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_axis_y->setMinimumHeight(minHeight1);
   pb_axis_y->setPalette( PALET_PUSHB );
   connect(pb_axis_y, SIGNAL(clicked()), SLOT(axis_y()));

   lbl_mode_title = new QLabel( "", this );
   lbl_mode_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_mode_title->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_mode_title );
   lbl_mode_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold ));
   lbl_mode_title->hide();

   // scale mode

   pb_scale = new QPushButton(tr("Scale"), this);
   pb_scale->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_scale->setMinimumHeight(minHeight1);
   pb_scale->setPalette( PALET_PUSHB );
   connect(pb_scale, SIGNAL(clicked()), SLOT(scale()));
   pb_scale->setEnabled( false );

   lbl_scale_low_high = new QLabel( tr( "Scale to: " ), this );
   lbl_scale_low_high->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_scale_low_high->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_scale_low_high );
   lbl_scale_low_high->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   rb_scale_low =  new QRadioButton( tr( "Minimum " ), this );
   rb_scale_low -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_scale_low );
   rb_scale_low -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_scale_low, SIGNAL( clicked() ), SLOT( scale_enables() ) );
                                    
   rb_scale_high =  new QRadioButton( tr( "Maximum " ), this );
   rb_scale_high -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_scale_high );
   rb_scale_high -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_scale_high, SIGNAL( clicked() ), SLOT( scale_enables() ) );

   bg_scale_low_high = new QButtonGroup( this );
   int bg_pos = 0;
   bg_scale_low_high->setExclusive(true);
   bg_scale_low_high->addButton( rb_scale_low, bg_pos++ );
   bg_scale_low_high->addButton( rb_scale_high, bg_pos++ );
   rb_scale_high->setChecked( true );
   
   cb_scale_sd = new QCheckBox(this);
   cb_scale_sd->setText(tr("SD "));
   cb_scale_sd->setChecked( true );
   cb_scale_sd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_scale_sd->setPalette( PALET_NORMAL );
   AUTFBACK( cb_scale_sd );
   connect( cb_scale_sd, SIGNAL( clicked() ), SLOT( scale_enables() ) );
   cb_scale_sd->hide();

   cb_scale_save_intp = new QCheckBox(this);
   cb_scale_save_intp->setText(tr("Save interpolated to target"));
   cb_scale_save_intp->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_scale_save_intp->setPalette( PALET_NORMAL );
   AUTFBACK( cb_scale_save_intp );
   connect( cb_scale_save_intp, SIGNAL( clicked() ), SLOT( scale_enables() ) );

   lbl_scale_q_range = new QLabel( tr( "q range for scaling: " ), this );
   lbl_scale_q_range->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_scale_q_range->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_scale_q_range );
   lbl_scale_q_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_scale_q_start = new mQLineEdit(this, "le_scale_q_start Line Edit");
   le_scale_q_start->setText( "" );
   le_scale_q_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_scale_q_start->setPalette(QPalette( cg_red, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_scale_q_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_scale_q_start->setEnabled( false );
   le_scale_q_start->setValidator( new QDoubleValidator( le_scale_q_start ) );
   connect( le_scale_q_start, SIGNAL( textChanged( const QString & ) ), SLOT( scale_q_start_text( const QString & ) ) );
   connect( le_scale_q_start, SIGNAL( focussed ( bool ) )             , SLOT( scale_q_start_focus( bool ) ) );

   le_scale_q_end = new mQLineEdit(this, "le_scale_q_end Line Edit");
   le_scale_q_end->setText( "" );
   le_scale_q_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_scale_q_end->setPalette(QPalette( cg_red, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_scale_q_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_scale_q_end->setEnabled( false );
   le_scale_q_end->setValidator( new QDoubleValidator( le_scale_q_end ) );
   connect( le_scale_q_end, SIGNAL( textChanged( const QString & ) ), SLOT( scale_q_end_text( const QString & ) ) );
   connect( le_scale_q_end, SIGNAL( focussed ( bool ) )             , SLOT( scale_q_end_focus( bool ) ) );

   pb_scale_q_reset = new QPushButton(tr("Reset q range"), this);
   pb_scale_q_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_scale_q_reset->setMinimumHeight(minHeight1);
   pb_scale_q_reset->setPalette( PALET_PUSHB );
   connect(pb_scale_q_reset, SIGNAL(clicked()), SLOT(scale_q_reset()));
   pb_scale_q_reset->setEnabled( false );

   pb_scale_reset = new QPushButton(tr("Reset scaling"), this);
   pb_scale_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_scale_reset->setMinimumHeight(minHeight1);
   pb_scale_reset->setPalette( PALET_PUSHB );
   connect(pb_scale_reset, SIGNAL(clicked()), SLOT(scale_reset()));
   pb_scale_reset->setEnabled( false );

   pb_scale_apply = new QPushButton(tr("Apply"), this);
   pb_scale_apply->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_scale_apply->setMinimumHeight(minHeight1);
   pb_scale_apply->setPalette( PALET_PUSHB );
   connect(pb_scale_apply, SIGNAL(clicked()), SLOT(scale_apply()));
   pb_scale_apply->setEnabled( false );

   pb_scale_create = new QPushButton(tr("Create scaled set"), this);
   pb_scale_create->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_scale_create->setMinimumHeight(minHeight1);
   pb_scale_create->setPalette( PALET_PUSHB );
   connect(pb_scale_create, SIGNAL(clicked()), SLOT(scale_create()));
   pb_scale_create->setEnabled( false );

   // testiq mode

   pb_testiq = new QPushButton(tr("Test I(q)"), this);
   pb_testiq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_testiq->setMinimumHeight(minHeight1);
   pb_testiq->setPalette( PALET_PUSHB );
   connect(pb_testiq, SIGNAL(clicked()), SLOT(testiq()));
   pb_testiq->setEnabled( false );

   lbl_testiq_q_range = new QLabel( tr( "Time range for I(q): " ), this );
   lbl_testiq_q_range->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_testiq_q_range->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_testiq_q_range );
   lbl_testiq_q_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_testiq_q_start = new mQLineEdit(this, "le_testiq_q_start Line Edit");
   le_testiq_q_start->setText( "" );
   le_testiq_q_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_testiq_q_start->setPalette(QPalette( cg_red, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_testiq_q_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_testiq_q_start->setEnabled( false );
   le_testiq_q_start->setValidator( new QDoubleValidator( le_testiq_q_start ) );
   connect( le_testiq_q_start, SIGNAL( textChanged( const QString & ) ), SLOT( testiq_q_start_text( const QString & ) ) );
   connect( le_testiq_q_start, SIGNAL( focussed ( bool ) )             , SLOT( testiq_q_start_focus( bool ) ) );

   le_testiq_q_end = new mQLineEdit(this, "le_testiq_q_end Line Edit");
   le_testiq_q_end->setText( "" );
   le_testiq_q_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_testiq_q_end->setPalette(QPalette( cg_red, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_testiq_q_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_testiq_q_end->setEnabled( false );
   le_testiq_q_end->setValidator( new QDoubleValidator( le_testiq_q_end ) );
   connect( le_testiq_q_end, SIGNAL( textChanged( const QString & ) ), SLOT( testiq_q_end_text( const QString & ) ) );
   connect( le_testiq_q_end, SIGNAL( focussed ( bool ) )             , SLOT( testiq_q_end_focus( bool ) ) );

   cb_testiq_from_gaussian = new QCheckBox(this);
   cb_testiq_from_gaussian->setText( tr( "as pure Gaussian" ) );
   cb_testiq_from_gaussian->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_testiq_from_gaussian->setPalette( PALET_NORMAL );
   AUTFBACK( cb_testiq_from_gaussian );
   cb_testiq_from_gaussian->setChecked( false );

   hbl_testiq_gaussians = new Q3HBoxLayout( 0 );

   lbl_testiq_gaussians = new QLabel( tr( "I(q) from Gaussian:  " ), this );
   lbl_testiq_gaussians->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_testiq_gaussians->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_testiq_gaussians );
   lbl_testiq_gaussians->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   rb_testiq_from_i_t =  new QRadioButton( tr( "None " ), this );
   rb_testiq_from_i_t -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_testiq_from_i_t );
   rb_testiq_from_i_t -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_testiq_from_i_t, SIGNAL( clicked() ), SLOT( testiq_gauss_line() ) );

   bg_testiq_gaussians = new QButtonGroup( this );
   bg_pos = 0;
   bg_testiq_gaussians->setExclusive( true );
   bg_testiq_gaussians->addButton( rb_testiq_from_i_t, bg_pos++ );

   pb_testiq_visrange = new QPushButton(tr("Vis. range"), this);
   pb_testiq_visrange->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_testiq_visrange->setMinimumHeight(minHeight1);
   pb_testiq_visrange->setPalette( PALET_PUSHB );
   connect(pb_testiq_visrange, SIGNAL(clicked()), SLOT(testiq_visrange()));

   pb_testiq_testset = new QPushButton(tr("Create I(q) set"), this);
   pb_testiq_testset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_testiq_testset->setMinimumHeight(minHeight1);
   pb_testiq_testset->setPalette( PALET_PUSHB );
   connect(pb_testiq_testset, SIGNAL(clicked()), SLOT(testiq_testset()));
   pb_testiq_testset->setEnabled( false );

   // guinier mode

   pb_guinier = new QPushButton(tr("Guinier"), this);
   pb_guinier->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_guinier->setMinimumHeight(minHeight1);
   pb_guinier->setPalette( PALET_PUSHB );
   connect(pb_guinier, SIGNAL(clicked()), SLOT(guinier()));
   pb_guinier->setEnabled( false );

   pb_guinier_plot_rg = new QPushButton(tr("Rg plot"), this);
   pb_guinier_plot_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_guinier_plot_rg->setMinimumHeight(minHeight1);
   pb_guinier_plot_rg->setPalette( PALET_PUSHB );
   pb_guinier_plot_rg->setEnabled( true );
   connect(pb_guinier_plot_rg, SIGNAL(clicked()), SLOT(guinier_plot_rg_toggle()));

   lbl_guinier_q_range = new QLabel( tr( "q range: " ), this );
   lbl_guinier_q_range->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_guinier_q_range->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_guinier_q_range );
   lbl_guinier_q_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_guinier_q_start = new mQLineEdit(this, "le_guinier_q_start Line Edit");
   le_guinier_q_start->setText( "" );
   le_guinier_q_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_q_start->setPalette(QPalette( cg_red, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_guinier_q_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_guinier_q_start->setEnabled( false );
   le_guinier_q_start->setValidator( new QDoubleValidator( le_guinier_q_start ) );
   connect( le_guinier_q_start, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_q_start_text( const QString & ) ) );
   connect( le_guinier_q_start, SIGNAL( focussed ( bool ) )             , SLOT( guinier_q_start_focus( bool ) ) );

   le_guinier_q_end = new mQLineEdit(this, "le_guinier_q_end Line Edit");
   le_guinier_q_end->setText( "" );
   le_guinier_q_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_q_end->setPalette(QPalette( cg_red, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_guinier_q_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_guinier_q_end->setEnabled( false );
   le_guinier_q_end->setValidator( new QDoubleValidator( le_guinier_q_end ) );
   connect( le_guinier_q_end, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_q_end_text( const QString & ) ) );
   connect( le_guinier_q_end, SIGNAL( focussed ( bool ) )             , SLOT( guinier_q_end_focus( bool ) ) );

   lbl_guinier_q2_range = new QLabel( tr( "q^2 range: " ), this );
   lbl_guinier_q2_range->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_guinier_q2_range->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_guinier_q2_range );
   lbl_guinier_q2_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_guinier_q2_range->hide();

   le_guinier_q2_start = new mQLineEdit(this, "le_guinier_q2_start Line Edit");
   le_guinier_q2_start->setText( "" );
   le_guinier_q2_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_q2_start->setPalette(QPalette( cg_red, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_guinier_q2_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_guinier_q2_start->setEnabled( false );
   le_guinier_q2_start->setValidator( new QDoubleValidator( le_guinier_q2_start ) );
   le_guinier_q2_start->hide();
   connect( le_guinier_q2_start, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_q2_start_text( const QString & ) ) );
   connect( le_guinier_q2_start, SIGNAL( focussed ( bool ) )             , SLOT( guinier_q2_start_focus( bool ) ) );

   le_guinier_q2_end = new mQLineEdit(this, "le_guinier_q2_end Line Edit");
   le_guinier_q2_end->setText( "" );
   le_guinier_q2_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_q2_end->setPalette(QPalette( cg_red, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_guinier_q2_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_guinier_q2_end->setEnabled( false );
   le_guinier_q2_end->setValidator( new QDoubleValidator( le_guinier_q2_end ) );
   le_guinier_q2_end->hide();
   connect( le_guinier_q2_end, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_q2_end_text( const QString & ) ) );
   connect( le_guinier_q2_end, SIGNAL( focussed ( bool ) )             , SLOT( guinier_q2_end_focus( bool ) ) );

   lbl_guinier_delta_range = new QLabel( tr( " plot extension: " ), this );
   lbl_guinier_delta_range->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_guinier_delta_range->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_guinier_delta_range );
   lbl_guinier_delta_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_guinier_delta_start = new mQLineEdit(this, "le_guinier_delta_start Line Edit");
   le_guinier_delta_start->setText( "" );
   le_guinier_delta_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_delta_start->setPalette( PALET_NORMAL );
   AUTFBACK( le_guinier_delta_start );
   le_guinier_delta_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_guinier_delta_start->setEnabled( false );
   le_guinier_delta_start->setValidator( new QDoubleValidator( le_guinier_delta_start ) );
   connect( le_guinier_delta_start, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_delta_start_text( const QString & ) ) );
   connect( le_guinier_delta_start, SIGNAL( focussed ( bool ) )             , SLOT( guinier_delta_start_focus( bool ) ) );

   le_guinier_delta_end = new mQLineEdit(this, "le_guinier_delta_end Line Edit");
   le_guinier_delta_end->setText( "" );
   le_guinier_delta_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_delta_end->setPalette( PALET_NORMAL );
   AUTFBACK( le_guinier_delta_end );
   le_guinier_delta_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_guinier_delta_end->setEnabled( false );
   le_guinier_delta_end->setValidator( new QDoubleValidator( le_guinier_delta_end ) );
   connect( le_guinier_delta_end, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_delta_end_text( const QString & ) ) );
   connect( le_guinier_delta_end, SIGNAL( focussed ( bool ) )             , SLOT( guinier_delta_end_focus( bool ) ) );

   lbl_guinier_qrgmax = new QLabel( tr( "q*Rg maximum: " ), this );
   lbl_guinier_qrgmax->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_guinier_qrgmax->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_guinier_qrgmax );
   lbl_guinier_qrgmax->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_guinier_qrgmax->hide();

   le_guinier_qrgmax = new mQLineEdit(this, "le_guinier_qrgmax Line Edit");
   le_guinier_qrgmax->setText( "1.3" );
   le_guinier_qrgmax->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_qrgmax->setPalette( PALET_NORMAL );
   AUTFBACK( le_guinier_qrgmax );
   le_guinier_qrgmax->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_guinier_qrgmax->setEnabled( true );
   le_guinier_qrgmax->setValidator( new QDoubleValidator( le_guinier_qrgmax ) );
   ((QDoubleValidator *)le_guinier_qrgmax->validator())->setBottom( 1 );
   connect( le_guinier_qrgmax, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_qrgmax_text( const QString & ) ) );
   le_guinier_qrgmax->hide();

   cb_guinier_sd = new QCheckBox(this);
   cb_guinier_sd->setText(tr("SD "));
   cb_guinier_sd->setChecked( true );
   cb_guinier_sd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_guinier_sd->setPalette( PALET_NORMAL );
   AUTFBACK( cb_guinier_sd );
   connect( cb_guinier_sd, SIGNAL( clicked() ), SLOT( guinier_sd() ) );

   rb_guinier_resid_diff =  new QRadioButton( tr( "Difference" ), this );
   rb_guinier_resid_diff -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_guinier_resid_diff );
   rb_guinier_resid_diff -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_guinier_resid_diff, SIGNAL( clicked() ), SLOT( guinier_residuals_update() ) );

   rb_guinier_resid_sd =  new QRadioButton( tr( "Standard deviation" ), this );
   rb_guinier_resid_sd -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_guinier_resid_sd );
   rb_guinier_resid_sd -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_guinier_resid_sd, SIGNAL( clicked() ), SLOT( guinier_residuals_update() ) );

   rb_guinier_resid_pct =  new QRadioButton( tr( "Percent" ), this );
   rb_guinier_resid_pct -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_guinier_resid_pct );
   rb_guinier_resid_pct -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_guinier_resid_pct, SIGNAL( clicked() ), SLOT( guinier_residuals_update() ) );

   bg_guinier_resid_type = new QButtonGroup( this );
   bg_pos = 0;
   bg_guinier_resid_type->setExclusive(true);
   bg_guinier_resid_type->addButton( rb_guinier_resid_diff, bg_pos++ );
   bg_guinier_resid_type->addButton( rb_guinier_resid_sd, bg_pos++ );
   bg_guinier_resid_type->addButton( rb_guinier_resid_pct, bg_pos++ );
   rb_guinier_resid_diff->setChecked( true );

   lbl_guinier_stats = new QLabel( "", this );
   lbl_guinier_stats->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
   lbl_guinier_stats->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_guinier_stats );
   // lbl_guinier_stats->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_guinier_stats->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   guinier_plot = new QwtPlot( qs );
#ifndef QT4
   guinier_plot->enableGridXMin();
   guinier_plot->enableGridYMin();
#else
   guinier_plot_grid = new QwtPlotGrid;
   guinier_plot_grid->enableXMin( true );
   guinier_plot_grid->enableYMin( true );
#endif
   guinier_plot->setPalette( PALET_NORMAL );
   AUTFBACK( guinier_plot );
#ifndef QT4
   guinier_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   guinier_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   guinier_plot_grid->setMajPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   guinier_plot_grid->setMinPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   guinier_plot_grid->attach( guinier_plot );
#endif
   guinier_plot->setAxisTitle(QwtPlot::xBottom, tr( "q^2 [1/Angstrom^2]" ) );
   guinier_plot->setAxisTitle(QwtPlot::yLeft, tr("Intensity [a.u.] (log scale)"));
#ifndef QT4
   guinier_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   guinier_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   guinier_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#ifndef QT4
   guinier_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   guinier_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#ifndef QT4
   guinier_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   guinier_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   guinier_plot->setMargin(USglobal->config_list.margin);
   guinier_plot->setTitle("");
#ifndef QT4
   guinier_plot->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::Logarithmic);
#else
   guinier_plot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);
#endif
   guinier_plot->setCanvasBackground(USglobal->global_colors.plot);

#ifndef QT4
   guinier_plot->setAutoLegend( false );
   guinier_plot->setLegendFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ) );
#else
   // {
   //    QwtLegend* legend_pd = new QwtLegend;
   //    legend_pd->setFrameStyle( QFrame::Box | QFrame::Sunken );
   //    guinier_plot->insertLegend( legend_pd, QwtPlot::BottomLegend );
   // }
#endif
   connect( guinier_plot->canvas(), SIGNAL( mouseReleased( const QMouseEvent & ) ), SLOT( plot_mouse(  const QMouseEvent & ) ) );

   guinier_plot_rg = new QwtPlot( qs );
#ifndef QT4
   guinier_plot_rg->enableGridXMin();
   guinier_plot_rg->enableGridYMin();
#else
   guinier_plot_rg_grid = new QwtPlotGrid;
   guinier_plot_rg_grid->enableXMin( true );
   guinier_plot_rg_grid->enableYMin( true );
#endif
   guinier_plot_rg->setPalette( PALET_NORMAL );
   AUTFBACK( guinier_plot_rg );
#ifndef QT4
   guinier_plot_rg->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   guinier_plot_rg->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   guinier_plot_rg_grid->setMajPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   guinier_plot_rg_grid->setMinPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   guinier_plot_rg_grid->attach( guinier_plot_rg );
#endif
   guinier_plot_rg->setAxisTitle(QwtPlot::xBottom, tr( "Time [a.u.]" ) );
   guinier_plot_rg->setAxisTitle(QwtPlot::yLeft, tr("Rg [Angstrom]"));
#ifndef QT4
   guinier_plot_rg->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   guinier_plot_rg->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   guinier_plot_rg->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#ifndef QT4
   guinier_plot_rg->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   guinier_plot_rg->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#ifndef QT4
   guinier_plot_rg->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   guinier_plot_rg->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   guinier_plot_rg->setMargin(USglobal->config_list.margin);
   guinier_plot_rg->setTitle("");
#ifndef QT4
   guinier_plot_rg->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::None);
#else
   guinier_plot_rg->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine );
#endif
   guinier_plot_rg->setCanvasBackground(USglobal->global_colors.plot);

#ifndef QT4
   guinier_plot_rg->setAutoLegend( false );
   guinier_plot_rg->setLegendFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ) );
#else
   // {
   //    QwtLegend* legend_pd = new QwtLegend;
   //    legend_pd->setFrameStyle( QFrame::Box | QFrame::Sunken );
   //    guinier_plot_rg->insertLegend( legend_pd, QwtPlot::BottomLegend );
   // }
#endif
   connect( guinier_plot_rg->canvas(), SIGNAL( mouseReleased( const QMouseEvent & ) ), SLOT( plot_mouse(  const QMouseEvent & ) ) );

   lbl_guinier_rg_t_range = new QLabel( tr( "Time range for Rg plot: " ), this );
   lbl_guinier_rg_t_range->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_guinier_rg_t_range->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_guinier_rg_t_range );
   lbl_guinier_rg_t_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_guinier_rg_t_start = new mQLineEdit(this, "le_guinier_rg_t_start Line Edit");
   le_guinier_rg_t_start->setText( "" );
   le_guinier_rg_t_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_rg_t_start->setPalette( PALET_NORMAL );
   AUTFBACK( le_guinier_rg_t_start );
   le_guinier_rg_t_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_guinier_rg_t_start->setEnabled( false );
   le_guinier_rg_t_start->setValidator( new QDoubleValidator( le_guinier_rg_t_start ) );
   connect( le_guinier_rg_t_start, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_rg_t_start_text( const QString & ) ) );
   connect( le_guinier_rg_t_start, SIGNAL( focussed ( bool ) )             , SLOT( guinier_rg_t_start_focus( bool ) ) );

   le_guinier_rg_t_end = new mQLineEdit(this, "le_guinier_rg_t_end Line Edit");
   le_guinier_rg_t_end->setText( "" );
   le_guinier_rg_t_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_rg_t_end->setPalette( PALET_NORMAL );
   AUTFBACK( le_guinier_rg_t_end );
   le_guinier_rg_t_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_guinier_rg_t_end->setEnabled( false );
   le_guinier_rg_t_end->setValidator( new QDoubleValidator( le_guinier_rg_t_end ) );
   connect( le_guinier_rg_t_end, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_rg_t_end_text( const QString & ) ) );
   connect( le_guinier_rg_t_end, SIGNAL( focussed ( bool ) )             , SLOT( guinier_rg_t_end_focus( bool ) ) );

   lbl_guinier_rg_rg_range = new QLabel( tr( "Rg range: " ), this );
   lbl_guinier_rg_rg_range->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_guinier_rg_rg_range->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_guinier_rg_rg_range );
   lbl_guinier_rg_rg_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_guinier_rg_rg_start = new mQLineEdit(this, "le_guinier_rg_rg_start Line Edit");
   le_guinier_rg_rg_start->setText( "" );
   le_guinier_rg_rg_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_rg_rg_start->setPalette( PALET_NORMAL );
   AUTFBACK( le_guinier_rg_rg_start );
   le_guinier_rg_rg_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_guinier_rg_rg_start->setEnabled( false );
   le_guinier_rg_rg_start->setValidator( new QDoubleValidator( le_guinier_rg_rg_start ) );
   connect( le_guinier_rg_rg_start, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_rg_rg_start_text( const QString & ) ) );
   connect( le_guinier_rg_rg_start, SIGNAL( focussed ( bool ) )             , SLOT( guinier_rg_rg_start_focus( bool ) ) );

   le_guinier_rg_rg_end = new mQLineEdit(this, "le_guinier_rg_rg_end Line Edit");
   le_guinier_rg_rg_end->setText( "" );
   le_guinier_rg_rg_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_rg_rg_end->setPalette( PALET_NORMAL );
   AUTFBACK( le_guinier_rg_rg_end );
   le_guinier_rg_rg_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_guinier_rg_rg_end->setEnabled( false );
   le_guinier_rg_rg_end->setValidator( new QDoubleValidator( le_guinier_rg_rg_end ) );
   connect( le_guinier_rg_rg_end, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_rg_rg_end_text( const QString & ) ) );
   connect( le_guinier_rg_rg_end, SIGNAL( focussed ( bool ) )             , SLOT( guinier_rg_rg_end_focus( bool ) ) );

   cb_guinier_lock_rg_range = new QCheckBox(this);
   cb_guinier_lock_rg_range->setText(tr("Lock range"));
   cb_guinier_lock_rg_range->setChecked( false );
   cb_guinier_lock_rg_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_guinier_lock_rg_range->setPalette( PALET_NORMAL );
   AUTFBACK( cb_guinier_lock_rg_range );

   guinier_plot_errors = new QwtPlot( qs );
#ifndef QT4
   guinier_plot_errors->enableGridXMin();
   guinier_plot_errors->enableGridYMin();
#else
   guinier_plot_errors_grid = new QwtPlotGrid;
   guinier_plot_errors_grid->enableXMin( true );
   guinier_plot_errors_grid->enableYMin( true );
#endif
   guinier_plot_errors->setPalette( PALET_NORMAL );
   AUTFBACK( guinier_plot_errors );
#ifndef QT4
   guinier_plot_errors->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   guinier_plot_errors->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   guinier_plot_errors_grid->setMajPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   guinier_plot_errors_grid->setMinPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   guinier_plot_errors_grid->attach( guinier_plot_errors );
#endif
   guinier_plot_errors->setAxisTitle(QwtPlot::xBottom, tr( "q^2 [1/Angstrom^2]" ) );
   guinier_plot_errors->setAxisTitle(QwtPlot::yLeft, tr("Intensity [a.u.] (log scale)"));
#ifndef QT4
   guinier_plot_errors->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   guinier_plot_errors->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   guinier_plot_errors->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#ifndef QT4
   guinier_plot_errors->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   guinier_plot_errors->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#ifndef QT4
   guinier_plot_errors->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   guinier_plot_errors->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   guinier_plot_errors->setMargin(USglobal->config_list.margin);
   guinier_plot_errors->setTitle("");
#ifndef QT4
   guinier_plot_errors->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::None);
#else
   guinier_plot_errors->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine );
#endif
   guinier_plot_errors->setCanvasBackground(USglobal->global_colors.plot);

#ifndef QT4
   guinier_plot_errors->setAutoLegend( false );
   guinier_plot_errors->setLegendFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ) );
#else
   // {
   //    QwtLegend* legend_pd = new QwtLegend;
   //    legend_pd->setFrameStyle( QFrame::Box | QFrame::Sunken );
   //    guinier_plot_errors->insertLegend( legend_pd, QwtPlot::BottomLegend );
   // }
#endif
   connect( guinier_plot_errors->canvas(), SIGNAL( mouseReleased( const QMouseEvent & ) ), SLOT( plot_mouse(  const QMouseEvent & ) ) );


   // rgc mode

   pb_rgc = new QPushButton(tr("Rg utility"), this);
   pb_rgc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_rgc->setMinimumHeight(minHeight1);
   pb_rgc->setPalette( PALET_PUSHB );
   connect(pb_rgc, SIGNAL(clicked()), SLOT(rgc()));
   pb_rgc->setEnabled( true );

   lbl_rgc_mw = new QLabel( tr( "MW [kDalton]" ), this );
   lbl_rgc_mw->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_rgc_mw->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_rgc_mw );
   lbl_rgc_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_rgc_mw = new mQLineEdit(this, "le_rgc_mw Line Edit");
   le_rgc_mw->setText( "" );
   le_rgc_mw->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_rgc_mw->setPalette( PALET_NORMAL );
   AUTFBACK( le_rgc_mw );
   le_rgc_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_rgc_mw->setEnabled( true );
   le_rgc_mw->setValidator( new QDoubleValidator( le_rgc_mw ) );
   ((QDoubleValidator *)le_rgc_mw->validator())->setBottom( 1 );
   connect( le_rgc_mw, SIGNAL( textChanged( const QString & ) ), SLOT( rgc_mw_text( const QString & ) ) );

   lbl_rgc_vol = new QLabel( tr( "Volume [A^3]" ), this );
   lbl_rgc_vol->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_rgc_vol->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_rgc_vol );
   lbl_rgc_vol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_rgc_vol = new mQLineEdit(this, "le_rgc_vol Line Edit");
   le_rgc_vol->setText( "" );
   le_rgc_vol->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_rgc_vol->setPalette( PALET_NORMAL );
   AUTFBACK( le_rgc_vol );
   le_rgc_vol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_rgc_vol->setEnabled( true );
   le_rgc_vol->setValidator( new QDoubleValidator( le_rgc_vol ) );
   // ((QDoubleValidator *)le_rgc_vol->validator())->setBottom( 1 );
   connect( le_rgc_vol, SIGNAL( textChanged( const QString & ) ), SLOT( rgc_vol_text( const QString & ) ) );

   lbl_rgc_rho = new QLabel( tr( "Density [g/cm^3]" ), this );
   lbl_rgc_rho->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_rgc_rho->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_rgc_rho );
   lbl_rgc_rho->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_rgc_rho = new mQLineEdit(this, "le_rgc_rho Line Edit");
   le_rgc_rho->setText( "1.4" );
   le_rgc_rho->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_rgc_rho->setPalette( PALET_NORMAL );
   AUTFBACK( le_rgc_rho );
   le_rgc_rho->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_rgc_rho->setEnabled( true );
   le_rgc_rho->setValidator( new QDoubleValidator( le_rgc_rho ) );
   ((QDoubleValidator *)le_rgc_rho->validator())->setRange( 1.2, 1.5 );
   connect( le_rgc_rho, SIGNAL( textChanged( const QString & ) ), SLOT( rgc_rho_text( const QString & ) ) );

   rb_rgc_shape_sphere =  new QRadioButton( tr( "Sphere " ), this );
   rb_rgc_shape_sphere -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_rgc_shape_sphere );
   rb_rgc_shape_sphere -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_rgc_shape_sphere, SIGNAL( clicked() ), SLOT( rgc_shape() ) );

   rb_rgc_shape_oblate =  new QRadioButton( tr( "Oblate " ), this );
   rb_rgc_shape_oblate -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_rgc_shape_oblate );
   rb_rgc_shape_oblate -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_rgc_shape_oblate, SIGNAL( clicked() ), SLOT( rgc_shape() ) );

   rb_rgc_shape_prolate =  new QRadioButton( tr( "Prolate " ), this );
   rb_rgc_shape_prolate -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_rgc_shape_prolate );
   rb_rgc_shape_prolate -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_rgc_shape_prolate, SIGNAL( clicked() ), SLOT( rgc_shape() ) );

   rb_rgc_shape_ellipsoid =  new QRadioButton( tr( "Ellipsoid " ), this );
   rb_rgc_shape_ellipsoid -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_rgc_shape_ellipsoid );
   rb_rgc_shape_ellipsoid -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_rgc_shape_ellipsoid, SIGNAL( clicked() ), SLOT( rgc_shape() ) );

   bg_rgc_shape = new QButtonGroup( this );
   bg_pos = 0;
   bg_rgc_shape->setExclusive(true);
   bg_rgc_shape->addButton( rb_rgc_shape_sphere, bg_pos++ );
   bg_rgc_shape->addButton( rb_rgc_shape_oblate, bg_pos++ );
   bg_rgc_shape->addButton( rb_rgc_shape_prolate, bg_pos++ );
   bg_rgc_shape->addButton( rb_rgc_shape_ellipsoid, bg_pos++ );
   rb_rgc_shape_sphere->setChecked( true );

   lbl_rgc_axis = new QLabel( "", this );
   lbl_rgc_axis->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_rgc_axis->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_rgc_axis );
   lbl_rgc_axis->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_rgc_axis_b = new mQLineEdit(this, "le_rgc_axis_b Line Edit");
   le_rgc_axis_b->setText( "1" );
   le_rgc_axis_b->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_rgc_axis_b->setPalette( PALET_NORMAL );
   AUTFBACK( le_rgc_axis_b );
   le_rgc_axis_b->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_rgc_axis_b->setEnabled( true );
   le_rgc_axis_b->setValidator( new QDoubleValidator( 0.01, 1.0, 3, le_rgc_axis_b ) );
   connect( le_rgc_axis_b, SIGNAL( textChanged( const QString & ) ), SLOT( rgc_axis_b_text( const QString & ) ) );

   le_rgc_axis_c = new mQLineEdit(this, "le_rgc_axis_c Line Edit");
   le_rgc_axis_c->setText( "1" );
   le_rgc_axis_c->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_rgc_axis_c->setPalette( PALET_NORMAL );
   AUTFBACK( le_rgc_axis_c );
   le_rgc_axis_c->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_rgc_axis_c->setEnabled( true );
   le_rgc_axis_c->setValidator( new QDoubleValidator( 0.01, 1.0, 3, le_rgc_axis_c ) );
   connect( le_rgc_axis_c, SIGNAL( textChanged( const QString & ) ), SLOT( rgc_axis_c_text( const QString & ) ) );

   lbl_rgc_rg = new QLabel( tr( "Rg [A]:" ), this );
   lbl_rgc_rg->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_rgc_rg->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_rgc_rg );
   lbl_rgc_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_rgc_rg = new mQLineEdit(this, "le_rgc_rg Line Edit");
   le_rgc_rg->setText( "" );
   le_rgc_rg->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_rgc_rg->setPalette( PALET_NORMAL );
   AUTFBACK( le_rgc_rg );
   le_rgc_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_rgc_rg->setEnabled( false );
   le_rgc_rg->setReadOnly( true );
   connect( le_rgc_rg, SIGNAL( textChanged( const QString & ) ), SLOT( rgc_rg_text( const QString & ) ) );

   lbl_rgc_extents = new QLabel( tr( "Axial extents [A]:" ), this );
   lbl_rgc_extents->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_rgc_extents->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_rgc_extents );
   lbl_rgc_extents->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_rgc_extents = new mQLineEdit(this, "le_rgc_extents Line Edit");
   le_rgc_extents->setText( "" );
   le_rgc_extents->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_rgc_extents->setPalette( PALET_NORMAL );
   AUTFBACK( le_rgc_extents );
   le_rgc_extents->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_rgc_extents->setEnabled( false );
   le_rgc_extents->setReadOnly( true );

   // pm mode

   pb_pm = new QPushButton(tr("PM"), this);
   pb_pm->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_pm->setMinimumHeight(minHeight1);
   pb_pm->setPalette( PALET_PUSHB );
   connect(pb_pm, SIGNAL(clicked()), SLOT(pm()));
   pb_pm->setEnabled( false );

   rb_pm_shape_sphere =  new QRadioButton( tr( "Sphere " ), this );
   rb_pm_shape_sphere -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_pm_shape_sphere );
   rb_pm_shape_sphere -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_pm_shape_sphere, SIGNAL( clicked() ), SLOT( pm_enables() ) );

   rb_pm_shape_spheroid =  new QRadioButton( tr( "Spheroid " ), this );
   rb_pm_shape_spheroid -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_pm_shape_spheroid );
   rb_pm_shape_spheroid -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_pm_shape_spheroid, SIGNAL( clicked() ), SLOT( pm_enables() ) );

   rb_pm_shape_ellipsoid =  new QRadioButton( tr( "Ellipsoid " ), this );
   rb_pm_shape_ellipsoid -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_pm_shape_ellipsoid );
   rb_pm_shape_ellipsoid -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_pm_shape_ellipsoid, SIGNAL( clicked() ), SLOT( pm_enables() ) );

   rb_pm_shape_cylinder =  new QRadioButton( tr( "Cylinder " ), this );
   rb_pm_shape_cylinder -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_pm_shape_cylinder );
   rb_pm_shape_cylinder -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_pm_shape_cylinder, SIGNAL( clicked() ), SLOT( pm_enables() ) );

   rb_pm_shape_torus =  new QRadioButton( tr( "Torus " ), this );
   rb_pm_shape_torus -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_pm_shape_torus );
   rb_pm_shape_torus -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_pm_shape_torus, SIGNAL( clicked() ), SLOT( pm_enables() ) );

   bg_pm_shape = new QButtonGroup( this );
   bg_pos = 0;
   bg_pm_shape->setExclusive(true);
   bg_pm_shape->addButton( rb_pm_shape_sphere, bg_pos++ );
   bg_pm_shape->addButton( rb_pm_shape_spheroid, bg_pos++ );
   bg_pm_shape->addButton( rb_pm_shape_ellipsoid, bg_pos++ );
   bg_pm_shape->addButton( rb_pm_shape_cylinder, bg_pos++ );
   bg_pm_shape->addButton( rb_pm_shape_torus, bg_pos++ );
   rb_pm_shape_sphere->setChecked( true );

   cb_pm_sd = new QCheckBox(this);
   cb_pm_sd->setText(tr("SD "));
   cb_pm_sd->setChecked( true );
   cb_pm_sd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_pm_sd->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pm_sd );
   connect( cb_pm_sd, SIGNAL( clicked() ), SLOT( pm_enables() ) );
   cb_pm_sd->hide();

   cb_pm_q_logbin = new QCheckBox(this);
   cb_pm_q_logbin->setText(tr("log q bins "));
   cb_pm_q_logbin->setChecked( false );
   cb_pm_q_logbin->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_pm_q_logbin->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pm_q_logbin );
   connect( cb_pm_q_logbin, SIGNAL( clicked() ), SLOT( pm_enables() ) );
   cb_pm_q_logbin->hide();

   lbl_pm_q_range = new QLabel( tr( "q range for modeling: " ), this );
   lbl_pm_q_range->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_pm_q_range->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_pm_q_range );
   lbl_pm_q_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_pm_q_start = new mQLineEdit(this, "le_pm_q_start Line Edit");
   le_pm_q_start->setText( "" );
   le_pm_q_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_pm_q_start->setPalette(QPalette( cg_red, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_pm_q_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_pm_q_start->setEnabled( false );
   le_pm_q_start->setValidator( new QDoubleValidator( le_pm_q_start ) );
   connect( le_pm_q_start, SIGNAL( textChanged( const QString & ) ), SLOT( pm_q_start_text( const QString & ) ) );
   connect( le_pm_q_start, SIGNAL( focussed ( bool ) )             , SLOT( pm_q_start_focus( bool ) ) );

   le_pm_q_end = new mQLineEdit(this, "le_pm_q_end Line Edit");
   le_pm_q_end->setText( "" );
   le_pm_q_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_pm_q_end->setPalette(QPalette( cg_red, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_pm_q_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_pm_q_end->setEnabled( false );
   le_pm_q_end->setValidator( new QDoubleValidator( le_pm_q_end ) );
   connect( le_pm_q_end, SIGNAL( textChanged( const QString & ) ), SLOT( pm_q_end_text( const QString & ) ) );
   connect( le_pm_q_end, SIGNAL( focussed ( bool ) )             , SLOT( pm_q_end_focus( bool ) ) );

   pb_pm_q_reset = new QPushButton(tr("Reset q range"), this);
   pb_pm_q_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_pm_q_reset->setMinimumHeight(minHeight1);
   pb_pm_q_reset->setPalette( PALET_PUSHB );
   connect(pb_pm_q_reset, SIGNAL(clicked()), SLOT(pm_q_reset()));
   pb_pm_q_reset->setEnabled( false );

   lbl_pm_samp_e_dens = new QLabel( tr( "Sample e density [e/A^3] (Tp. Prot: .41-.44, DNA:.59: RNA:.6 Carb:.49)" ), this );
   lbl_pm_samp_e_dens->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_pm_samp_e_dens->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_pm_samp_e_dens );
   lbl_pm_samp_e_dens->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_pm_samp_e_dens = new QLineEdit(this, "le_pm_samp_e_dens Line Edit");
   le_pm_samp_e_dens->setText( ".425" );
   le_pm_samp_e_dens->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_pm_samp_e_dens->setPalette( PALET_NORMAL );
   AUTFBACK( le_pm_samp_e_dens );
   le_pm_samp_e_dens->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_pm_samp_e_dens->setEnabled( false );
   le_pm_samp_e_dens->setValidator( new QDoubleValidator( le_pm_samp_e_dens ) );
   connect( le_pm_samp_e_dens, SIGNAL( textChanged( const QString & ) ), SLOT( pm_samp_e_dens_text( const QString & ) ) );

   lbl_pm_buff_e_dens = new QLabel( tr( "Buffer e density [e/A^3]" ), this );
   lbl_pm_buff_e_dens->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_pm_buff_e_dens->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_pm_buff_e_dens );
   lbl_pm_buff_e_dens->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   
   le_pm_buff_e_dens = new QLineEdit(this, "le_pm_buff_e_dens Line Edit");
   le_pm_buff_e_dens->setText( QString( "%1" ).arg((((US_Hydrodyn *)us_hydrodyn)->saxs_options.water_e_density ) ) );
   le_pm_buff_e_dens->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_pm_buff_e_dens->setPalette( PALET_NORMAL );
   AUTFBACK( le_pm_buff_e_dens );
   le_pm_buff_e_dens->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_pm_buff_e_dens->setEnabled( false );
   le_pm_buff_e_dens->setValidator( new QDoubleValidator( le_pm_buff_e_dens ) );
   connect( le_pm_buff_e_dens, SIGNAL( textChanged( const QString & ) ), SLOT( pm_buff_e_dens_text( const QString & ) ) );

   lbl_pm_grid_size = new QLabel( tr( "min. bead radius [A]" ), this );
   lbl_pm_grid_size->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_pm_grid_size->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_pm_grid_size );
   lbl_pm_grid_size->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   
   le_pm_grid_size = new QLineEdit(this, "le_pm_grid_size Line Edit");
   le_pm_grid_size->setText( "4" );
   le_pm_grid_size->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_pm_grid_size->setPalette( PALET_NORMAL );
   AUTFBACK( le_pm_grid_size );
   le_pm_grid_size->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_pm_grid_size->setEnabled( false );
   le_pm_grid_size->setValidator( new QIntValidator( le_pm_grid_size ) );
   connect( le_pm_grid_size, SIGNAL( textChanged( const QString & ) ), SLOT( pm_grid_size_text( const QString & ) ) );

   lbl_pm_q_pts = new QLabel( tr( " q points every n-th (1=all)" ), this );
   lbl_pm_q_pts->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_pm_q_pts->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_pm_q_pts );
   lbl_pm_q_pts->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   
   le_pm_q_pts = new QLineEdit(this, "le_pm_q_pts Line Edit");
   le_pm_q_pts->setText( "1" );
   le_pm_q_pts->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_pm_q_pts->setPalette( PALET_NORMAL );
   AUTFBACK( le_pm_q_pts );
   le_pm_q_pts->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_pm_q_pts->setEnabled( false );
   le_pm_q_pts->setValidator( new QIntValidator( le_pm_q_pts ) );
   connect( le_pm_q_pts, SIGNAL( textChanged( const QString & ) ), SLOT( pm_q_pts_text( const QString & ) ) );

   pb_pm_run = new QPushButton(tr("Start"), this);
   pb_pm_run->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_pm_run->setMinimumHeight(minHeight1);
   pb_pm_run->setPalette( PALET_PUSHB );
   connect(pb_pm_run, SIGNAL(clicked()), SLOT(pm_run()));
   pb_pm_q_reset->setEnabled( false );

   // bottom

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_options = new QPushButton(tr("Options"), this);
   pb_options->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   pb_options->setMinimumHeight(minHeight1);
   pb_options->setPalette( PALET_PUSHB );
   connect(pb_options, SIGNAL(clicked()), SLOT(options()));

   pb_cancel = new QPushButton(tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // build layout

   Q3BoxLayout *hbl_file_buttons = new Q3HBoxLayout( 0 );
   hbl_file_buttons->addWidget ( pb_add_files );
   hbl_file_buttons->addWidget ( pb_similar_files );
   hbl_file_buttons->addWidget ( pb_conc);
   hbl_file_buttons->addWidget ( pb_clear_files );

   files_widgets.push_back( pb_add_files );
   files_widgets.push_back( pb_similar_files );
   files_widgets.push_back( pb_conc );
   files_widgets.push_back( pb_clear_files );

   Q3BoxLayout *hbl_file_buttons_1 = new Q3HBoxLayout( 0 );
   hbl_file_buttons_1->addWidget ( pb_regex_load );
   hbl_file_buttons_1->addWidget ( le_regex );
   hbl_file_buttons_1->addWidget ( le_regex_args );
   hbl_file_buttons_1->addWidget ( pb_save_state );

   files_expert_widgets.push_back( pb_regex_load );
   files_expert_widgets.push_back( le_regex );
   files_expert_widgets.push_back( le_regex_args );
   files_expert_widgets.push_back( pb_save_state );

   Q3BoxLayout *hbl_file_buttons_2 = new Q3HBoxLayout( 0 );
   hbl_file_buttons_2->addWidget ( pb_select_all );
   hbl_file_buttons_2->addWidget ( pb_invert );
   // hbl_file_buttons_2->addWidget ( pb_adjacent );
   hbl_file_buttons_2->addWidget ( pb_select_nth );
   // hbl_file_buttons_2->addWidget ( pb_color_rotate );
   // hbl_file_buttons_2->addWidget ( pb_to_saxs );
   hbl_file_buttons_2->addWidget ( pb_view );
   hbl_file_buttons_2->addWidget ( pb_movie );
   hbl_file_buttons_2->addWidget ( pb_ag );
   hbl_file_buttons_2->addWidget ( pb_axis_x );
   hbl_file_buttons_2->addWidget ( pb_axis_y );
   hbl_file_buttons_2->addWidget ( pb_rescale );

   files_widgets.push_back ( pb_select_all );
   files_widgets.push_back ( pb_invert );
   // files_widgets.push_back ( pb_adjacent );
   files_widgets.push_back ( pb_select_nth );
   files_widgets.push_back ( pb_view );
   files_widgets.push_back ( pb_movie );
   files_widgets.push_back ( pb_axis_x );
   files_widgets.push_back ( pb_axis_y );
   files_widgets.push_back ( pb_rescale );

   Q3BoxLayout *hbl_file_buttons_2b = new Q3HBoxLayout( 0 );
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

   files_expert_widgets.push_back( pb_stack_push_all );
   files_expert_widgets.push_back( pb_stack_push_sel );
   files_expert_widgets.push_back( lbl_stack );
   files_expert_widgets.push_back( pb_stack_copy );
   files_expert_widgets.push_back( pb_stack_pcopy );
   files_expert_widgets.push_back( pb_stack_paste );
   files_expert_widgets.push_back( pb_stack_drop );
   files_expert_widgets.push_back( pb_stack_join );
   files_expert_widgets.push_back( pb_stack_rot_up );
   files_expert_widgets.push_back( pb_stack_rot_down );
   files_expert_widgets.push_back( pb_stack_swap );

   if ( !U_EXPT )
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

      pb_rgc->hide();
      pb_pm->hide();

      pb_ag->hide();

      // pb_conc->hide();
      // pb_normalize->hide();
   }
   pb_conc_avg->hide();

   // #if defined( JAC_VERSION )
   // if ( !U_EXPT )
   // {
   //   pb_options->hide();
   // }
   // #endif

   Q3BoxLayout *hbl_file_buttons_3 = new Q3HBoxLayout( 0 );
   hbl_file_buttons_3->addWidget ( pb_conc_avg );
   hbl_file_buttons_3->addWidget ( pb_normalize );
   hbl_file_buttons_3->addWidget ( pb_avg );
   hbl_file_buttons_3->addWidget ( pb_add );
   hbl_file_buttons_3->addWidget ( pb_to_saxs );
   hbl_file_buttons_3->addWidget ( pb_line_width );
   hbl_file_buttons_3->addWidget ( pb_color_rotate );

   files_widgets.push_back ( pb_conc_avg );
   files_widgets.push_back ( pb_normalize );
   files_widgets.push_back ( pb_avg );
   files_expert_widgets.push_back ( pb_add );
   files_widgets.push_back ( pb_to_saxs );
   files_widgets.push_back ( pb_line_width );
   files_widgets.push_back ( pb_color_rotate );
   files_expert_widgets.push_back ( pb_ag );

   Q3BoxLayout *hbl_file_buttons_4 = new Q3HBoxLayout( 0 );
   hbl_file_buttons_4->addWidget ( pb_smooth );
   hbl_file_buttons_4->addWidget ( pb_repeak );
   hbl_file_buttons_4->addWidget ( pb_svd );
   hbl_file_buttons_4->addWidget ( pb_create_i_of_t );
   hbl_file_buttons_4->addWidget ( pb_test_i_of_t );
   hbl_file_buttons_4->addWidget ( pb_create_i_of_q );

   files_widgets.push_back ( pb_smooth );
   files_widgets.push_back ( pb_repeak );
   files_widgets.push_back ( pb_svd );
   files_widgets.push_back ( pb_create_i_of_t );
   files_widgets.push_back ( pb_test_i_of_t );
   files_widgets.push_back ( pb_create_i_of_q );

   Q3BoxLayout *hbl_conc_file = new Q3HBoxLayout( 0 );
   hbl_conc_file->addWidget ( pb_conc_file );
   hbl_conc_file->addWidget ( pb_detector );

   files_widgets.push_back ( pb_conc_file );
   files_widgets.push_back ( pb_detector );

   //    QBoxLayout *hbl_hplc = new QHBoxLayout( 0 );
   //    hbl_hplc->addWidget ( pb_set_hplc );
   //    hbl_hplc->addWidget ( lbl_hplc );

   //    QBoxLayout *hbl_empty = new QHBoxLayout( 0 );
   //    hbl_empty->addWidget ( pb_set_empty );
   //    hbl_empty->addWidget ( lbl_empty );

   //    QBoxLayout *hbl_signal = new QHBoxLayout( 0 );
   //    hbl_signal->addWidget ( pb_set_signal );
   //    hbl_signal->addWidget ( lbl_signal );

   Q3BoxLayout *hbl_created = new Q3HBoxLayout( 0 );
   hbl_created->addWidget ( pb_select_all_created );
   hbl_created->addWidget ( pb_invert_all_created );
   hbl_created->addWidget ( pb_adjacent_created );
   hbl_created->addWidget ( pb_remove_created );
   hbl_created->addWidget ( pb_save_created_csv );
   hbl_created->addWidget ( pb_save_created );

   created_files_widgets.push_back( pb_select_all_created );
   created_files_widgets.push_back( pb_invert_all_created );
   created_files_widgets.push_back( pb_adjacent_created );
   created_files_widgets.push_back( pb_remove_created );
   created_files_widgets.push_back( pb_save_created_csv );
   created_files_widgets.push_back( pb_save_created );

   Q3BoxLayout *hbl_created_2 = new Q3HBoxLayout( 0 );
   hbl_created_2->addWidget ( pb_show_created );
   hbl_created_2->addWidget ( pb_show_only_created );

   created_files_widgets.push_back ( pb_show_created );
   created_files_widgets.push_back ( pb_show_only_created );

   Q3BoxLayout *vbl_editor_group = new Q3VBoxLayout(0);
   vbl_editor_group->addWidget ( lbl_editor );
#if !defined(QT4) || !defined(Q_WS_MAC)
   vbl_editor_group->addWidget ( frame );
#endif
   vbl_editor_group->addWidget ( editor );

   Q3HBoxLayout *hbl_dir = new Q3HBoxLayout( 0 );
   hbl_dir->addWidget( cb_lock_dir );
   hbl_dir->addWidget( lbl_dir );

   files_widgets.push_back( cb_lock_dir );
   files_widgets.push_back( lbl_dir );

   files_widgets.push_back ( lb_files );
   files_widgets.push_back ( lbl_selected );
   files_widgets.push_back ( lbl_conc_file );

   created_files_widgets.push_back ( lb_created_files );
   created_files_widgets.push_back ( lbl_created_dir );
   created_files_widgets.push_back ( lbl_selected_created );

   Q3BoxLayout *vbl_model = new Q3VBoxLayout( 0 );
   vbl_model->addWidget( lbl_model_files );
   vbl_model->addWidget( lb_model_files );
   {
      Q3BoxLayout *hbl = new Q3HBoxLayout( 0 );
      hbl->addWidget( pb_model_select_all );
      hbl->addWidget( pb_model_text );
      hbl->addWidget( pb_model_view );
      hbl->addWidget( pb_model_remove );
      hbl->addWidget( pb_model_save );
      vbl_model->addLayout( hbl );
   }      

   Q3GridLayout *gl_files = new Q3GridLayout( 0 );
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
      gl_files->addLayout( vbl_model , j, 0 ); j++;
      gl_files->addLayout( vbl_editor_group , j, 0 ); j++;
   }

   Q3BoxLayout *hbl_plot_buttons = new Q3HBoxLayout(0);
   hbl_plot_buttons->addWidget( pb_select_vis );
   hbl_plot_buttons->addWidget( pb_remove_vis );
   hbl_plot_buttons->addWidget( pb_crop_common );
   hbl_plot_buttons->addWidget( pb_crop_vis );
   hbl_plot_buttons->addWidget( pb_crop_zero );
   hbl_plot_buttons->addWidget( pb_crop_left );
   hbl_plot_buttons->addWidget( pb_crop_undo );
   hbl_plot_buttons->addWidget( pb_crop_right );
   hbl_plot_buttons->addWidget( pb_legend );

   Q3GridLayout *gl_wheel = new Q3GridLayout(0);
   gl_wheel->addWidget         ( lbl_blank1     , 0, 0 );
   gl_wheel->addMultiCellWidget( qwtw_wheel     , 0, 0, 1, 9 );
   gl_wheel->addWidget         ( lbl_wheel_pos  , 0, 10 );

   // gl_wheel->addMultiCellWidget( pb_wheel_start , 0, 0, 0, 1 );
   // gl_wheel->addWidget         ( lbl_wheel_pos  , 0, 2 );
   // gl_wheel->addMultiCellWidget( qwtw_wheel     , 0, 0, 3, 7 );
   // gl_wheel->addWidget         ( pb_ref         , 0, 8 );
   // gl_wheel->addWidget         ( pb_errors      , 0, 9 );
   // gl_wheel->addWidget         ( pb_wheel_cancel, 0, 10 );
   // gl_wheel->addWidget         ( pb_wheel_save  , 0, 11 );

   Q3BoxLayout *hbl_top = new Q3HBoxLayout( 0 );
   hbl_top->addWidget( pb_p3d );
   hbl_top->addWidget( pb_ref );
   hbl_top->addWidget( pb_guinier_plot_rg );
   hbl_top->addWidget( pb_errors );
   hbl_top->addWidget( pb_wheel_cancel );
   hbl_top->addWidget( pb_wheel_save );

   Q3BoxLayout *hbl_mode = new Q3HBoxLayout( 0 );
   hbl_mode->addWidget( pb_gauss_start );
   hbl_mode->addWidget( pb_ggauss_start );
   hbl_mode->addWidget( pb_baseline_start );
   hbl_mode->addWidget( pb_baseline_apply );
   hbl_mode->addWidget( pb_wheel_start );
   hbl_mode->addWidget( pb_scale );
   hbl_mode->addWidget( pb_rgc );
   hbl_mode->addWidget( pb_pm );
   hbl_mode->addWidget( pb_testiq );
   hbl_mode->addWidget( pb_guinier );

   // scale

   Q3BoxLayout *vbl_scale = new Q3VBoxLayout( 0 );
   {
      Q3BoxLayout *hbl = new Q3HBoxLayout( 0 );
      hbl->addWidget( lbl_scale_q_range );
      hbl->addWidget( le_scale_q_start );
      hbl->addWidget( le_scale_q_end );
      hbl->addWidget( pb_scale_q_reset );
      vbl_scale->addLayout( hbl );
   }      

   {
      Q3BoxLayout *hbl = new Q3HBoxLayout( 0 );
      hbl->addWidget( lbl_scale_low_high );
      hbl->addWidget( rb_scale_low );
      hbl->addWidget( rb_scale_high );
      hbl->addWidget( cb_scale_sd );
      hbl->addWidget( cb_scale_save_intp );
      hbl->addWidget( pb_scale_reset );
      hbl->addWidget( pb_scale_apply );
      hbl->addWidget( pb_scale_create );
      vbl_scale->addLayout( hbl );
   }      

   // testiq
   Q3BoxLayout *vbl_testiq = new Q3VBoxLayout( 0 );
   {
      Q3BoxLayout *hbl = new Q3HBoxLayout( 0 );
      hbl->addWidget( lbl_testiq_q_range );
      hbl->addWidget( le_testiq_q_start );
      hbl->addWidget( le_testiq_q_end );
      hbl->addWidget( lbl_testiq_gaussians );
      hbl->addWidget( rb_testiq_from_i_t );
      hbl->addLayout( hbl_testiq_gaussians );
      hbl->addWidget( cb_testiq_from_gaussian );
      hbl->addWidget( pb_testiq_visrange );
      hbl->addWidget( pb_testiq_testset );
      vbl_testiq->addLayout( hbl );
   }      

   // guinier
   Q3BoxLayout *hbl_guinier_resid = new Q3HBoxLayout( 0 );
   hbl_guinier_resid->addWidget( rb_guinier_resid_diff );
   hbl_guinier_resid->addWidget( rb_guinier_resid_sd );
   hbl_guinier_resid->addWidget( rb_guinier_resid_pct );

   Q3BoxLayout *vbl_guinier = new Q3VBoxLayout( 0 );
   {
      Q3BoxLayout *hbl = new Q3HBoxLayout( 0 );
      hbl->addWidget( lbl_guinier_rg_t_range );
      hbl->addWidget( le_guinier_rg_t_start );
      hbl->addWidget( le_guinier_rg_t_end );
      hbl->addWidget( lbl_guinier_rg_rg_range );
      hbl->addWidget( le_guinier_rg_rg_start );
      hbl->addWidget( le_guinier_rg_rg_end );
      hbl->addWidget( cb_guinier_lock_rg_range );
      vbl_guinier->addLayout( hbl );
   }      
      
   {
      Q3BoxLayout *hbl = new Q3HBoxLayout( 0 );
      hbl->addWidget( lbl_guinier_q_range );
      hbl->addWidget( le_guinier_q_start );
      hbl->addWidget( le_guinier_q_end );
      hbl->addWidget( lbl_guinier_q2_range );
      hbl->addWidget( le_guinier_q2_start );
      hbl->addWidget( le_guinier_q2_end );
      hbl->addWidget( lbl_guinier_delta_range );
      hbl->addWidget( le_guinier_delta_start );
      hbl->addWidget( le_guinier_delta_end );
      hbl->addWidget( cb_guinier_sd );
      hbl->addWidget( lbl_guinier_qrgmax );
      hbl->addWidget( le_guinier_qrgmax );
      vbl_guinier->addLayout( hbl );
   }      
   vbl_guinier->addWidget( lbl_guinier_stats );

   // QBoxLayout * vbl_guinier_plots = new QVBoxLayout( 0 );
   // vbl_guinier_plots->addWidget( guinier_plot );
   // vbl_guinier_plots->addWidget( guinier_plot_errors );

   // pm
   Q3BoxLayout *vbl_pm = new Q3VBoxLayout( 0 );
   {
      Q3BoxLayout *hbl = new Q3HBoxLayout( 0 );
      hbl->addWidget( lbl_pm_q_range );
      hbl->addWidget( le_pm_q_start );
      hbl->addWidget( le_pm_q_end );
      hbl->addWidget( pb_pm_q_reset );
      vbl_pm->addLayout( hbl );
   }      
   {
      Q3BoxLayout *hbl = new Q3HBoxLayout( 0 );
      hbl->addWidget( rb_pm_shape_sphere );
      hbl->addWidget( rb_pm_shape_spheroid );
      hbl->addWidget( rb_pm_shape_ellipsoid );
      hbl->addWidget( rb_pm_shape_cylinder );
      hbl->addWidget( rb_pm_shape_torus );
      vbl_pm->addLayout( hbl );
   }

   {
      Q3BoxLayout *hbl = new Q3HBoxLayout( 0 );
      hbl->addWidget( cb_pm_sd );
      hbl->addWidget( cb_pm_q_logbin );
      hbl->addWidget( lbl_pm_q_pts );
      hbl->addWidget( le_pm_q_pts );
      hbl->addWidget( lbl_pm_grid_size );
      hbl->addWidget( le_pm_grid_size );
      vbl_pm->addLayout( hbl );
   }      
   {
      Q3BoxLayout *hbl = new Q3HBoxLayout( 0 );
      hbl->addWidget( lbl_pm_samp_e_dens );
      hbl->addWidget( le_pm_samp_e_dens );
      hbl->addWidget( lbl_pm_buff_e_dens );
      hbl->addWidget( le_pm_buff_e_dens );
      hbl->addWidget( pb_pm_run );
      vbl_pm->addLayout( hbl );
   }      

   // rgc
   Q3BoxLayout *vbl_rgc = new Q3VBoxLayout( 0 );
   {
      Q3BoxLayout *hbl = new Q3HBoxLayout( 0 );
      hbl->addWidget( lbl_rgc_mw );
      hbl->addWidget( le_rgc_mw );
      hbl->addWidget( lbl_rgc_vol );
      hbl->addWidget( le_rgc_vol );
      hbl->addWidget( lbl_rgc_rho );
      hbl->addWidget( le_rgc_rho );
      vbl_rgc->addLayout( hbl );
   }      
   {
      Q3BoxLayout *hbl = new Q3HBoxLayout( 0 );
      
      hbl->addWidget( rb_rgc_shape_sphere );
      hbl->addWidget( rb_rgc_shape_oblate );
      hbl->addWidget( rb_rgc_shape_prolate );
      hbl->addWidget( rb_rgc_shape_ellipsoid );
      hbl->addWidget( lbl_rgc_axis );
      hbl->addWidget( le_rgc_axis_b );
      hbl->addWidget( le_rgc_axis_c );
      vbl_rgc->addLayout( hbl );
   }      

   {
      Q3BoxLayout *hbl = new Q3HBoxLayout( 0 );
      hbl->addWidget( lbl_rgc_rg );
      hbl->addWidget( le_rgc_rg );
      hbl->addWidget( lbl_rgc_extents );
      hbl->addWidget( le_rgc_extents );
      vbl_rgc->addLayout( hbl );
   }      

   Q3GridLayout *gl_gauss = new Q3GridLayout(0);
   { 
      int ofs = 1;
      gl_gauss->addWidget         ( pb_gauss_clear              , 0, ofs++ );
      gl_gauss->addWidget         ( pb_gauss_new                , 0, ofs++ );
      gl_gauss->addWidget         ( pb_gauss_delete             , 0, ofs++ );
      gl_gauss->addWidget         ( pb_gauss_prev               , 0, ofs++ );
      gl_gauss->addWidget         ( lbl_gauss_pos               , 0, ofs++ );
      gl_gauss->addWidget         ( pb_gauss_next               , 0, ofs++ );
      gl_gauss->addWidget         ( cb_gauss_match_amplitude    , 0, ofs++ );
      gl_gauss->addWidget         ( le_gauss_pos                , 0, ofs++ );
      gl_gauss->addWidget         ( le_gauss_pos_width          , 0, ofs++ );
      gl_gauss->addWidget         ( le_gauss_pos_height         , 0, ofs++ );
      gl_gauss->addWidget         ( le_gauss_pos_dist1          , 0, ofs++ );
      gl_gauss->addWidget         ( le_gauss_pos_dist2          , 0, ofs++ );
      gl_gauss->addWidget         ( pb_gauss_save               , 0, ofs++ );
   }

   Q3GridLayout *gl_gauss2 = new Q3GridLayout(0);
   { 
      int ofs = 1;
      gl_gauss2->addWidget         ( cb_sd_weight        , 0, ofs++ );
      gl_gauss2->addWidget         ( cb_fix_width        , 0, ofs++ );
      gl_gauss2->addWidget         ( cb_fix_dist1        , 0, ofs++ );
      gl_gauss2->addWidget         ( cb_fix_dist2        , 0, ofs++ );
      gl_gauss2->addWidget         ( pb_gauss_fit        , 0, ofs++ );
      gl_gauss2->addWidget         ( pb_ggauss_rmsd      , 0, ofs++ );
      gl_gauss2->addWidget         ( lbl_gauss_fit       , 0, ofs++ );
      gl_gauss2->addWidget         ( le_gauss_fit_start  , 0, ofs++ );
      gl_gauss2->addWidget         ( le_gauss_fit_end    , 0, ofs++ );
      gl_gauss2->addWidget         ( pb_ggauss_results   , 0, ofs++ );
      gl_gauss2->addWidget         ( pb_gauss_as_curves  , 0, ofs++ );
   }

   Q3HBoxLayout *hbl_baseline = new Q3HBoxLayout( 0 );
   // hbl_baseline->addWidget( pb_baseline_start   );
   hbl_baseline->addWidget( cb_baseline_start_zero );
   hbl_baseline->addWidget( le_baseline_start_s );
   hbl_baseline->addWidget( le_baseline_start   );
   hbl_baseline->addWidget( le_baseline_start_e );
   hbl_baseline->addWidget( le_baseline_end_s   );
   hbl_baseline->addWidget( le_baseline_end     );
   hbl_baseline->addWidget( le_baseline_end_e   );
   //   hbl_baseline->addWidget( pb_baseline_apply   );

   Q3BoxLayout *vbl_plot_group = new Q3VBoxLayout(0);
   // vbl_plot_group->addWidget ( plot_dist );
   // vbl_plot_group->addWidget ( plot_ref );
   // vbl_plot_group->addLayout ( vbl_guinier_plots );
   vbl_plot_group->addWidget ( qs );
   vbl_plot_group->addLayout ( l_plot_errors );
   vbl_plot_group->addLayout ( hbl_guinier_resid );
   vbl_plot_group->addLayout ( gl_wheel );
   vbl_plot_group->addWidget ( lbl_mode_title );
   vbl_plot_group->addLayout ( hbl_top );
   vbl_plot_group->addLayout ( hbl_mode );
   vbl_plot_group->addLayout ( vbl_scale );
   vbl_plot_group->addLayout ( vbl_testiq );
   vbl_plot_group->addLayout ( vbl_guinier );
   vbl_plot_group->addLayout ( vbl_rgc );
   vbl_plot_group->addLayout ( vbl_pm );
   vbl_plot_group->addLayout ( gl_gauss );
   // vbl_plot_group->addLayout ( hbl_gauss2 );
   vbl_plot_group->addLayout ( gl_gauss2  );
   vbl_plot_group->addLayout ( hbl_baseline );

   vbl_plot_group->addWidget ( le_dummy );

   vbl_plot_group->addLayout ( hbl_plot_buttons );

//    QBoxLayout *hbl_files_plot = new QHBoxLayout( 0 );
//    // hbl_files_plot->addLayout( vbl_files );
//    hbl_files_plot->addLayout( gl_files );
//    hbl_files_plot->addLayout( vbl_plot_group );

   Q3GridLayout *gl_files_plot = new Q3GridLayout( 0 );
   gl_files_plot->addLayout( gl_files      , 0, 0 );
   gl_files_plot->addLayout( vbl_plot_group, 0, 1 );
   gl_files_plot->setColStretch( 0, 0 );
   gl_files_plot->setColStretch( 1, 1 );

   Q3GridLayout *gl_bottom = new Q3GridLayout( 0 );
   gl_bottom->addWidget( pb_help     , 0, 0 );
   gl_bottom->addWidget( pb_options  , 0, 1 );
   gl_bottom->addWidget( progress    , 0, 2 );
   gl_bottom->addWidget( pb_cancel   , 0, 3 );
   
   Q3VBoxLayout *background = new Q3VBoxLayout(this);
   background->addWidget ( lbl_title );
   background->addSpacing( 1 );
   // background->addSpacing( 1 );
   background->addLayout ( gl_files_plot );
   background->addSpacing( 1 );
   background->addLayout ( gl_bottom );
   background->addSpacing( 1 );

   //   hide_widgets( files_widgets, 
   //                 !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "hplc_files_widgets" ) || ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_files_widgets" ] == "false" ? false : true );
   //   hide_widgets( editor_widgets, 
   //                 !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "hplc_editor_widgets" ) || ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_editor_widgets" ] == "false" ? false : true );
   //   hide_widgets( created_files_widgets,
   //                 !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "hplc_created_files_widgets" ) || ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_creaded_files_widgets" ] == "false" ? false : true );

   mode_setup_widgets();
}

void US_Hydrodyn_Saxs_Hplc::mode_setup_widgets()
{
   // plot_widgets;

#ifndef qt4
   plot_widgets.push_back( le_dummy );
#endif
   plot_widgets.push_back( pb_select_vis );
   plot_widgets.push_back( pb_remove_vis );
   plot_widgets.push_back( pb_crop_common );
   plot_widgets.push_back( pb_crop_vis );
   plot_widgets.push_back( pb_crop_zero );
   plot_widgets.push_back( pb_crop_left );
   plot_widgets.push_back( pb_crop_undo );
   plot_widgets.push_back( pb_crop_right );
   plot_widgets.push_back( pb_legend );

   // gaussian_widgets;

   gaussian_widgets.push_back( pb_gauss_new );
   gaussian_widgets.push_back( pb_gauss_delete );
   gaussian_widgets.push_back( pb_gauss_clear );
   gaussian_widgets.push_back( pb_gauss_prev );
   gaussian_widgets.push_back( lbl_gauss_pos );
   gaussian_widgets.push_back( pb_gauss_next );
   gaussian_widgets.push_back( cb_sd_weight );
   gaussian_widgets.push_back( cb_gauss_match_amplitude );
   gaussian_widgets.push_back( pb_gauss_fit );
   gaussian_widgets.push_back( lbl_gauss_fit );
   gaussian_widgets.push_back( le_gauss_pos );
   gaussian_widgets.push_back( le_gauss_pos_width );
   gaussian_widgets.push_back( le_gauss_pos_height );
   gaussian_widgets.push_back( le_gauss_fit_start );
   gaussian_widgets.push_back( le_gauss_fit_end );
   gaussian_widgets.push_back( pb_gauss_save );
   gaussian_widgets.push_back( pb_gauss_as_curves );
   gaussian_widgets.push_back( lbl_blank1 );
   gaussian_widgets.push_back( qwtw_wheel );
   gaussian_widgets.push_back( lbl_wheel_pos );

   // ggaussian_widgets;

   ggaussian_widgets.push_back( pb_gauss_prev );
   ggaussian_widgets.push_back( lbl_gauss_pos );
   ggaussian_widgets.push_back( pb_gauss_next );
   ggaussian_widgets.push_back( cb_fix_width );
   ggaussian_widgets.push_back( cb_sd_weight );
   ggaussian_widgets.push_back( pb_gauss_fit );
   ggaussian_widgets.push_back( le_gauss_pos );
   ggaussian_widgets.push_back( le_gauss_pos_width );
   ggaussian_widgets.push_back( le_gauss_fit_start );
   ggaussian_widgets.push_back( le_gauss_fit_end );
   ggaussian_widgets.push_back( pb_ggauss_rmsd );
   ggaussian_widgets.push_back( lbl_gauss_fit );
   ggaussian_widgets.push_back( pb_ggauss_results );
   ggaussian_widgets.push_back( pb_gauss_save );
   ggaussian_widgets.push_back( pb_gauss_as_curves );
   ggaussian_widgets.push_back( lbl_blank1 );
   ggaussian_widgets.push_back( qwtw_wheel );
   ggaussian_widgets.push_back( lbl_wheel_pos );

   // gaussian_4var_widgets;
   gaussian_4var_widgets.push_back( le_gauss_pos_dist1 );

   // gaussian_5var_widgets;
   gaussian_5var_widgets.push_back( le_gauss_pos_dist1 );
   gaussian_5var_widgets.push_back( le_gauss_pos_dist2 );

   // ggaussian_4var_widgets;
   ggaussian_4var_widgets.push_back( cb_fix_dist1 );
   ggaussian_4var_widgets.push_back( le_gauss_pos_dist1 );

   // ggaussian_5var_widgets;
   ggaussian_5var_widgets.push_back( cb_fix_dist1 );
   ggaussian_5var_widgets.push_back( cb_fix_dist2 );
   ggaussian_5var_widgets.push_back( le_gauss_pos_dist1 );
   ggaussian_5var_widgets.push_back( le_gauss_pos_dist2 );

   // baseline_widgets;

   baseline_widgets.push_back( cb_baseline_start_zero );
   baseline_widgets.push_back( le_baseline_start_s );
   baseline_widgets.push_back( le_baseline_start );
   baseline_widgets.push_back( le_baseline_start_e );
   baseline_widgets.push_back( le_baseline_end_s );
   baseline_widgets.push_back( le_baseline_end );
   baseline_widgets.push_back( le_baseline_end_e );
   // baseline_widgets.push_back( pb_baseline_apply );
   baseline_widgets.push_back( lbl_blank1 );
   baseline_widgets.push_back( qwtw_wheel );
   baseline_widgets.push_back( lbl_wheel_pos );

   // scale_widgets;

   scale_widgets.push_back( lbl_scale_low_high );
   scale_widgets.push_back( rb_scale_low );
   scale_widgets.push_back( rb_scale_high );
   // scale_widgets.push_back( cb_scale_sd );
   scale_widgets.push_back( cb_scale_save_intp );
   scale_widgets.push_back( lbl_scale_q_range );
   scale_widgets.push_back( le_scale_q_start );
   scale_widgets.push_back( le_scale_q_end );
   scale_widgets.push_back( pb_scale_q_reset );
   scale_widgets.push_back( pb_scale_apply );
   scale_widgets.push_back( pb_scale_reset );
   scale_widgets.push_back( pb_scale_create );
   scale_widgets.push_back( lbl_blank1 );
   scale_widgets.push_back( qwtw_wheel );
   scale_widgets.push_back( lbl_wheel_pos );

   // timeshift_widgets;

   timeshift_widgets.push_back( lbl_blank1 );
   timeshift_widgets.push_back( qwtw_wheel );
   timeshift_widgets.push_back( lbl_wheel_pos );
   timeshift_widgets.push_back( le_dummy );
   // timeshift_widgets.push_back( pb_select_vis );
   // timeshift_widgets.push_back( pb_remove_vis );
   // timeshift_widgets.push_back( pb_crop_common );
   // timeshift_widgets.push_back( pb_crop_vis );
   // timeshift_widgets.push_back( pb_crop_zero );
   // timeshift_widgets.push_back( pb_crop_left );
   // timeshift_widgets.push_back( pb_crop_undo );
   // timeshift_widgets.push_back( pb_crop_right );
   // timeshift_widgets.push_back( pb_legend );

   // testiq_widgets;

   testiq_widgets.push_back( lbl_testiq_q_range );
   testiq_widgets.push_back( le_testiq_q_start );
   testiq_widgets.push_back( le_testiq_q_end );
   testiq_widgets.push_back( lbl_testiq_gaussians );
   testiq_widgets.push_back( rb_testiq_from_i_t );
   testiq_widgets.push_back( cb_testiq_from_gaussian );
   testiq_widgets.push_back( pb_testiq_visrange );
   testiq_widgets.push_back( pb_testiq_testset );
   testiq_widgets.push_back( lbl_blank1 );
   testiq_widgets.push_back( qwtw_wheel );
   testiq_widgets.push_back( lbl_wheel_pos );

   // guinier_widgets;

   guinier_widgets.push_back( lbl_guinier_q_range );
   guinier_widgets.push_back( le_guinier_q_start );
   guinier_widgets.push_back( le_guinier_q_end );
   // guinier_widgets.push_back( lbl_guinier_q2_range );
   // guinier_widgets.push_back( le_guinier_q2_start );
   // guinier_widgets.push_back( le_guinier_q2_end );
   guinier_widgets.push_back( lbl_guinier_delta_range );
   guinier_widgets.push_back( le_guinier_delta_start );
   guinier_widgets.push_back( le_guinier_delta_end );
   // guinier_widgets.push_back( lbl_guinier_qrgmax );
   // guinier_widgets.push_back( le_guinier_qrgmax );
   guinier_widgets.push_back( cb_guinier_sd );
   guinier_widgets.push_back( guinier_plot );
   guinier_widgets.push_back( guinier_plot_rg );
   guinier_widgets.push_back( guinier_plot_errors );
   guinier_widgets.push_back( rb_guinier_resid_diff );
   guinier_widgets.push_back( rb_guinier_resid_sd );
   guinier_widgets.push_back( rb_guinier_resid_pct );
   guinier_widgets.push_back( lbl_guinier_stats );
   guinier_widgets.push_back( lbl_blank1 );
   guinier_widgets.push_back( qwtw_wheel );
   guinier_widgets.push_back( lbl_wheel_pos );
   guinier_widgets.push_back( pb_guinier_plot_rg );
   guinier_widgets.push_back( lbl_guinier_rg_t_range );
   guinier_widgets.push_back( le_guinier_rg_t_start );
   guinier_widgets.push_back( le_guinier_rg_t_end );
   guinier_widgets.push_back( lbl_guinier_rg_rg_range );
   guinier_widgets.push_back( le_guinier_rg_rg_start );
   guinier_widgets.push_back( le_guinier_rg_rg_end );
   guinier_widgets.push_back( cb_guinier_lock_rg_range );


   // not a "mode"
   guinier_errors_widgets.push_back( guinier_plot_errors );
   guinier_errors_widgets.push_back( rb_guinier_resid_diff );
   guinier_errors_widgets.push_back( rb_guinier_resid_sd );
   guinier_errors_widgets.push_back( rb_guinier_resid_pct );

   // not a "mode"
   guinier_rg_widgets.push_back( guinier_plot_rg );
   guinier_rg_widgets.push_back( lbl_guinier_rg_t_range );
   guinier_rg_widgets.push_back( le_guinier_rg_t_start );
   guinier_rg_widgets.push_back( le_guinier_rg_t_end );
   guinier_rg_widgets.push_back( lbl_guinier_rg_rg_range );
   guinier_rg_widgets.push_back( le_guinier_rg_rg_start );
   guinier_rg_widgets.push_back( le_guinier_rg_rg_end );
   guinier_rg_widgets.push_back( cb_guinier_lock_rg_range );

   // rgc_widgets;
   rgc_widgets.push_back( lbl_rgc_mw );
   rgc_widgets.push_back( le_rgc_mw );
   rgc_widgets.push_back( lbl_rgc_vol );
   rgc_widgets.push_back( le_rgc_vol );
   rgc_widgets.push_back( lbl_rgc_rho );
   rgc_widgets.push_back( le_rgc_rho );
   rgc_widgets.push_back( rb_rgc_shape_sphere );
   rgc_widgets.push_back( rb_rgc_shape_oblate );
   rgc_widgets.push_back( rb_rgc_shape_prolate );
   rgc_widgets.push_back( rb_rgc_shape_ellipsoid );
   rgc_widgets.push_back( lbl_rgc_axis );
   rgc_widgets.push_back( le_rgc_axis_b );
   rgc_widgets.push_back( le_rgc_axis_c );
   rgc_widgets.push_back( lbl_rgc_rg );
   rgc_widgets.push_back( le_rgc_rg );
   rgc_widgets.push_back( lbl_rgc_extents );
   rgc_widgets.push_back( le_rgc_extents );

   // pm_widgets;
   pm_widgets.push_back( rb_pm_shape_sphere );
   pm_widgets.push_back( rb_pm_shape_spheroid );
   pm_widgets.push_back( rb_pm_shape_ellipsoid );
   pm_widgets.push_back( rb_pm_shape_cylinder );
   pm_widgets.push_back( rb_pm_shape_torus );
   pm_widgets.push_back( cb_pm_sd );
   pm_widgets.push_back( cb_pm_q_logbin );
   pm_widgets.push_back( lbl_pm_q_range );
   pm_widgets.push_back( le_pm_q_start );
   pm_widgets.push_back( le_pm_q_end );
   pm_widgets.push_back( pb_pm_q_reset );
   pm_widgets.push_back( lbl_pm_samp_e_dens );
   pm_widgets.push_back( le_pm_samp_e_dens );
   pm_widgets.push_back( lbl_pm_buff_e_dens );
   pm_widgets.push_back( le_pm_buff_e_dens );
   pm_widgets.push_back( lbl_pm_q_pts );
   pm_widgets.push_back( le_pm_q_pts );
   pm_widgets.push_back( lbl_pm_grid_size );
   pm_widgets.push_back( le_pm_grid_size );
   pm_widgets.push_back( pb_pm_run );
   pm_widgets.push_back( lbl_blank1 );
   pm_widgets.push_back( qwtw_wheel );
   pm_widgets.push_back( lbl_wheel_pos );
   pm_widgets.push_back( le_dummy );
}   

void US_Hydrodyn_Saxs_Hplc::mode_select()
{
   // QSize cur_size = plot_dist->size();

   ShowHide::hide_widgets( plot_widgets );
   ShowHide::hide_widgets( gaussian_widgets );
   ShowHide::hide_widgets( gaussian_4var_widgets );
   ShowHide::hide_widgets( gaussian_5var_widgets );
   ShowHide::hide_widgets( ggaussian_widgets );
   ShowHide::hide_widgets( ggaussian_4var_widgets );
   ShowHide::hide_widgets( ggaussian_5var_widgets );
   ShowHide::hide_widgets( baseline_widgets );
   ShowHide::hide_widgets( scale_widgets );
   ShowHide::hide_widgets( timeshift_widgets );
   ShowHide::hide_widgets( testiq_widgets );
   ShowHide::hide_widgets( guinier_widgets );
   ShowHide::hide_widgets( rgc_widgets );
   ShowHide::hide_widgets( pm_widgets );

   switch ( current_mode )
   {
   case MODE_NORMAL    : lbl_wheel_pos->setText( "" ); mode_title( "" ); ShowHide::hide_widgets( plot_widgets      , false ); break;
   case MODE_GAUSSIAN  : 
      {
         mode_title( pb_gauss_start->text() );
         ShowHide::hide_widgets( gaussian_widgets  , false );
         switch ( gaussian_type_size )
         {
         case 4 : ShowHide::hide_widgets( gaussian_4var_widgets , false ); break;
         case 5 : ShowHide::hide_widgets( gaussian_5var_widgets , false ); break;
         default : break;
         }
      }
      break;

   case MODE_GGAUSSIAN : 
      {
         mode_title( pb_ggauss_start->text() );
         ShowHide::hide_widgets( ggaussian_widgets , false );
         switch ( gaussian_type_size )
         {
         case 4 : ShowHide::hide_widgets( ggaussian_4var_widgets , false ); break;
         case 5 : ShowHide::hide_widgets( ggaussian_5var_widgets , false ); break;
         default : break;
         }            
      }
      break;

   case MODE_BASELINE  : mode_title( pb_baseline_start->text() ); ShowHide::hide_widgets( baseline_widgets  , false ); break;
   case MODE_TIMESHIFT : mode_title( pb_wheel_start->text() );    ShowHide::hide_widgets( timeshift_widgets , false ); break;
   case MODE_SCALE     : mode_title( pb_scale->text() );          ShowHide::hide_widgets( scale_widgets     , false ); break;
   case MODE_TESTIQ    : mode_title( pb_testiq->text() );         ShowHide::hide_widgets( testiq_widgets    , false ); break;
   case MODE_GUINIER   : mode_title( pb_guinier->text() );        ShowHide::hide_widgets( guinier_widgets   , false ); break;
   case MODE_RGC       : mode_title( pb_rgc->text() );            ShowHide::hide_widgets( rgc_widgets       , false ); break;
   case MODE_PM        : mode_title( pb_pm->text() );             ShowHide::hide_widgets( pm_widgets        , false ); break;
   default : qDebug( "mode select error" ); break;
   }
   // plot_dist->resize( cur_size );
   resize( size() );
}

void US_Hydrodyn_Saxs_Hplc::mode_select( modes mode )
{
   current_mode = mode;
   mode_select();
}

void US_Hydrodyn_Saxs_Hplc::mode_title( QString title )
{
   lbl_mode_title->setText( title );
   if ( title.isEmpty() )
   {
      lbl_mode_title->hide();
   } else {
      lbl_mode_title->show();
   }
}

void US_Hydrodyn_Saxs_Hplc::update_enables()
{
   if ( running )
   {
      if ( current_mode == MODE_PM )
      {
         model_enables();
         pm_enables();
      //    qDebug( "model_enables in update_enables (running)\n" );
      // } else {
      //    qDebug( "update_enables return (running)\n" );
      }
      return;
   }
   // cout << "update_enables\n";

   // cout << "US_Hydrodyn_Saxs_Hplc::update_enables()\n";
   // cout << QString("saxs_window->qsl_plotted_iq_names.size() %1\n").arg(saxs_window->qsl_plotted_iq_names.size());

   pb_add_files          ->setEnabled( true );
   pb_regex_load         ->setEnabled( true );
   pb_options            ->setEnabled( true );

   lb_files              ->setEnabled( true );
   lb_created_files      ->setEnabled( true );

   unsigned int files_selected_count                      = 0;
   // unsigned int non_hplc_non_empty_files_selected_count = 0;
   // unsigned int last_selected_pos                         = 0;

   map < QString, bool > selected_map;

   QStringList selected_files;

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         selected_files << lb_files->text( i );
         selected_map[ lb_files->text( i ) ] = true;
         // last_selected_pos = i;
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
   cb_fix_width          ->setEnabled( files_selected_count && files_are_time && gaussians.size() && U_EXPT );
   cb_fix_dist1          ->setEnabled( files_selected_count && files_are_time && gaussians.size() && U_EXPT );
   cb_fix_dist2          ->setEnabled( files_selected_count && files_are_time && gaussians.size() && U_EXPT );
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
   pb_smooth             ->setEnabled( files_selected_count );
   pb_repeak             ->setEnabled( files_selected_count > 1 && files_compatible && files_are_time );
   pb_svd                ->setEnabled( files_selected_count > 1 && files_compatible && !files_are_time );
   pb_create_i_of_t      ->setEnabled( files_selected_count > 1 && files_compatible && !files_are_time );
   pb_test_i_of_t        ->setEnabled( files_selected_count && files_compatible && files_are_time );
   pb_create_i_of_q      ->setEnabled( files_selected_count > 1 && files_compatible && files_are_time /* && gaussians.size() */ );
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
   pb_line_width         ->setEnabled( files_selected_count );
   pb_color_rotate       ->setEnabled( files_selected_count );
   //    pb_join               ->setEnabled( files_selected_count == 2 && files_compatible && !files_are_time );
   // pb_adjacent           ->setEnabled( lb_files->numRows() > 1 );
   pb_to_saxs            ->setEnabled( files_selected_count && files_compatible && !files_are_time );
   pb_view               ->setEnabled( files_selected_count && files_selected_count <= 10 );
   pb_movie              ->setEnabled( files_selected_count > 1 );
   pb_rescale            ->setEnabled( files_selected_count > 0 );
   pb_ag                 ->setEnabled( files_selected_count == 1 && files_compatible && !files_are_time );

   pb_select_all_created ->setEnabled( lb_created_files->numRows() > 0 );
   pb_invert_all_created ->setEnabled( lb_created_files->numRows() > 0 );
   pb_adjacent_created   ->setEnabled( lb_created_files->numRows() > 1 );
   pb_remove_created     ->setEnabled( files_created_selected_count > 0 );
   pb_save_created_csv   ->setEnabled( files_created_selected_count > 0 && files_compatible );
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

   pb_scale            ->setEnabled( files_selected_count > 1 && files_compatible );
   pb_rgc              ->setEnabled( true );
   pb_pm               ->setEnabled( files_selected_count == 1 && files_compatible && !files_are_time );
   pb_testiq           ->setEnabled( files_selected_count > 4 && files_compatible && files_are_time );
   pb_guinier          ->setEnabled( files_selected_count && files_compatible && !files_are_time );

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

   pb_ref              ->setEnabled( files_selected_count == 1 && !files_are_time && !lbl_conc_file->text().isEmpty() );
   if ( !suppress_replot && plot_ref->isVisible() && !pb_ref->isEnabled() )
   {
      plot_ref->hide();
   }

   if ( files_selected_count == 2 && files_compatible )
   {
      pb_errors           ->setEnabled( true );
   } else {
      pb_errors           ->setEnabled( false );
      hide_widgets( plot_errors_widgets, true );
   }

   pb_save_state       ->setEnabled( ( lb_files->numRows() || stack_data.size() ) && !files_created_selected_not_saved_count );

   pb_p3d              ->setEnabled( files_selected_count > 1 && files_compatible && files_are_time );

   {
      QString title;
      if ( !files_compatible )
      {
         title = tr( "q [1/Angstrom] or Time [a.u.]" );
      } else {
         if ( type_files( selected_files ) )
         {
            title = tr( "Time [a.u.]" );
         } else {
            title = tr( "q [1/Angstrom]" );
         }
      }
      if ( axis_x_log )
      {
         plot_dist->setAxisTitle(QwtPlot::xBottom,  title + tr(" (log scale)") );
      } else {
         plot_dist->setAxisTitle(QwtPlot::xBottom,  title );
      }
   }
   {
      QString title;
      if ( !files_compatible )
      {
         title = tr( "Intensity [a.u.]" );
      } else {
         if ( type_files( selected_files ) )
         {
            title = tr( "I(t) [a.u.]" );
         } else {
            title = tr( "I(q) [a.u.]" );
         }
      }

      if ( axis_y_log )
      {
         plot_dist->setAxisTitle(QwtPlot::yLeft, title + tr( " (log scale)") );
      } else {
         plot_dist->setAxisTitle(QwtPlot::yLeft, title );
      }
   }
   model_enables();
}

void US_Hydrodyn_Saxs_Hplc::model_enables()
{
   if ( lb_model_files->count() )
   {
      ShowHide::hide_widgets( model_widgets, false );
      pb_model_select_all->setEnabled( true );
      lb_model_files     ->setEnabled( true );
      bool any_model_selected           = false;
      bool any_model_selected_not_saved = false;
      for ( int i = 0; i < (int) lb_model_files->count(); ++i )
      {
         if ( lb_model_files->isSelected( i ) )
         {
            any_model_selected = true;
            if ( models_not_saved.count( lb_model_files->text( i ) ) )
            {
               any_model_selected_not_saved = true;
               break;
            }
         }
      }
      pb_model_text  ->setEnabled( any_model_selected );
      pb_model_view  ->setEnabled( any_model_selected );
      pb_model_remove->setEnabled( any_model_selected );
      pb_model_save  ->setEnabled( any_model_selected_not_saved );
   } else {
      ShowHide::hide_widgets( model_widgets, true );
   }      
}

void US_Hydrodyn_Saxs_Hplc::disable_all()
{
   // cout << "disable all\n";
   pb_similar_files      ->setEnabled( false );
   pb_conc               ->setEnabled( false );
   pb_clear_files        ->setEnabled( false );
   pb_avg                ->setEnabled( false );
   pb_normalize          ->setEnabled( false );
   pb_conc_avg           ->setEnabled( false );
   pb_smooth             ->setEnabled( false );
   pb_repeak             ->setEnabled( false );
   pb_svd                ->setEnabled( false );
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
   pb_line_width         ->setEnabled( false );
   pb_color_rotate       ->setEnabled( false );
   //    pb_join               ->setEnabled( false );
   // pb_adjacent           ->setEnabled( false );
   pb_to_saxs            ->setEnabled( false );
   pb_view               ->setEnabled( false );
   pb_movie              ->setEnabled( false );
   pb_ag                 ->setEnabled( false );
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
   le_gauss_pos_dist1    ->setEnabled( false );
   le_gauss_pos_dist2    ->setEnabled( false );
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
   cb_fix_dist1          ->setEnabled( false );
   cb_fix_dist2          ->setEnabled( false );

   pb_ref                ->setEnabled( false );
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
   pb_options            ->setEnabled( false );

   pb_scale              ->setEnabled( false );
   pb_rgc                ->setEnabled( false );
   pb_pm                 ->setEnabled( false );
   pb_testiq             ->setEnabled( false );
   pb_guinier            ->setEnabled( false );

   le_pm_q_start         ->setEnabled( false );
   le_pm_q_end           ->setEnabled( false );
   le_pm_q_pts           ->setEnabled( false );
   le_pm_grid_size       ->setEnabled( false );
   le_pm_samp_e_dens     ->setEnabled( false );
   le_pm_buff_e_dens     ->setEnabled( false );
   pb_pm_q_reset         ->setEnabled( false );

   lb_model_files        ->setEnabled( false );
   pb_model_select_all   ->setEnabled( false );
   pb_model_save         ->setEnabled( false );
   pb_model_text         ->setEnabled( false );
   pb_model_view         ->setEnabled( false );
   pb_model_remove       ->setEnabled( false );

   pb_testiq_visrange    ->setEnabled( false );
   pb_testiq_testset     ->setEnabled( false );
}

void US_Hydrodyn_Saxs_Hplc::model_select_all()
{
   bool all_selected = true;
   for ( int i = 0; i < lb_model_files->numRows(); i++ )
   {
      if ( !lb_model_files->isSelected( i ) )
      {
         all_selected = false;
         break;
      }
   }

   disable_updates = true;
   for ( int i = 0; i < lb_model_files->numRows(); i++ )
   {
      lb_model_files->setSelected( i, !all_selected );
   }
   disable_updates = false;
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::model_save()
{
   disable_all();
   model_save( MQT::get_lb_qsl( lb_model_files, true ) );
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::model_text()
{
   disable_all();
   model_text( MQT::get_lb_qsl( lb_model_files, true ) );
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::model_view()
{
   disable_all();
   model_view( MQT::get_lb_qsl( lb_model_files, true ) );
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::model_remove()
{
   disable_all();
   model_remove( MQT::get_lb_qsl( lb_model_files, true ) );
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::model_remove( QStringList files )
{
   disable_updates = true;

   QStringList           model_not_saved_list;
   map < QString, bool > model_not_saved_map;
   map < QString, bool > selected_map;

   for ( int i = 0; i < (int)files.size(); i++ )
   {
      QString this_file = files[ i ];
      selected_map[ this_file ] = true;
      if ( models_not_saved.count( this_file ) )
      {
         model_not_saved_list << this_file;
         model_not_saved_map[ this_file ] = true;
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
                                    tr( "&Remove them anyway" ), 
                                    tr( "&Quit from removing files" ), 
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

   // remove them now
   for ( int i = lb_model_files->numRows(); i >= 0; i-- )
   {
      if ( selected_map.count( lb_model_files->text( i ) ) )
      {
         models_not_saved.erase( lb_model_files->text( i ) );
         lb_model_files->removeItem( i );
      }
   }

   for ( int i = lb_model_files->numRows() - 1; i >= 0; i-- )
   {
      if ( selected_map.count( lb_model_files->text( i ) ) )
      {
         editor_msg( "black", QString( tr( "Removed model %1" ) ).arg( lb_model_files->text( i ) ) );
         models.erase( lb_model_files->text( i ) );
         lb_model_files->removeItem( i );
      }
   }

   update_enables();

}

void US_Hydrodyn_Saxs_Hplc::model_view( QStringList files )
{
   // run rasmol on models

   QString tmpd = USglobal->config_list.root_dir + "/somo/saxs/tmp";
   QDir dir( tmpd );
   if (!dir.exists())
   {
      if ( dir.mkdir( tmpd ) )
      {
         editor_msg( "red", QString( tr( "Error: could not create temporary directory %1" ) ).arg( tmpd ) );
         return;
      }
   }

   for ( int i = 0; i < (int) files.size(); ++i )
   {
      int bead_count;
      QStringList qsl0 = QStringList::split( "\n", models[ files[ i ] ] );

      if ( qsl0.size() < 1 )
      {
         editor_msg( "red", QString( tr( "Error: insufficient model info for file %1 [a]" ) ).arg( files[ i ] ) );
         return;
      }
         
      {
         QStringList qsl = QStringList::split( QRegExp( "\\s+" ), qsl0[ 0 ] );
         if ( qsl.size() < 1 )
         {
            editor_msg( "red", QString( tr( "Error: insufficient model info for file %1 [b]" ) ).arg( files[ i ] ) );
            return;
         }
         bead_count = qsl[ 0 ].toDouble();
      }

      QString bms;
      QString spt;

      double scale = 5e0;
      int linepos    = 0;

      bms += 
         QString( "%1\n%2\n" ).arg( bead_count ).arg( files[ i ] );

      spt += 
         QString( "load xyz %1.bms\nselect all\nwireframe off\nset background white\n" ).arg( files[ i ] );
         
      for ( int j = 1; j < (int) qsl0.size() && linepos < bead_count; ++j )
      {
         bms += "Pb ";

         QStringList qsl = QStringList::split( QRegExp( "\\s+" ), qsl0[ j ] );

         if ( qsl.size() < 4 )
         {
            editor_msg( "red", QString( tr( "Error: insufficient model info for file %1 line %2 [c]" ) ).arg( files[ i ] ).arg( linepos ) );
            return;
         }

         for ( int i = 0; i < 3; i++ )
         {
            bms += QString( " %1" ).arg( qsl[ i ].toDouble() / scale );
            if ( !linepos )
            {
               bms += QString( " %1" ).arg( qsl[ i ].toDouble() / scale );
            }
         }
         bms += "\n";

         ++linepos;
         spt += QString( "select atomno=%1\nspacefill %2\ncolour redorange\n" )
            .arg( linepos )
            .arg( qsl[ 3 ].toDouble() / scale );
      }         

      {
         QFile f( tmpd + "/" + files[ i ] + ".spt" );
         if ( !f.open( QIODevice::WriteOnly ) )
         {
            editor_msg( "red", QString( tr( "Error: could not create output file %1" ) ).arg( f.name() ) );
            return;
         }
         Q3TextStream ts( &f );
         ts << spt;
         f.close();
      }
      {
         QFile f( tmpd + "/" + files[ i ] + ".bms" );
         if ( !f.open( QIODevice::WriteOnly ) )
         {
            editor_msg( "red", QString( tr( "Error: could not create output file %1" ) ).arg( f.name() ) );
            return;
         }
         Q3TextStream ts( &f );
         ts << bms;
         f.close();
      }

      {
         QStringList argument;
#if !defined(WIN32) && !defined(MAC)
         // maybe we should make this a user defined terminal window?
         argument.append("xterm");
         argument.append("-e");
#endif
#if defined(BIN64)
         argument.append(USglobal->config_list.system_dir + "/bin64/rasmol");
#else
         argument.append(USglobal->config_list.system_dir + "/bin/rasmol");
#endif
         argument.append("-script");
         argument.append( files[ i ] + ".spt" ); 
         
         Q3Process * rasmol = new Q3Process;
         rasmol->setWorkingDirectory( tmpd );

         rasmol->setArguments(argument);
         if (!rasmol->start())
         {
            editor_msg( "red", tr("There was a problem starting RASMOL: check to make sure RASMOL is properly installed" ) );
            return;
         }
      }
   }
}

void US_Hydrodyn_Saxs_Hplc::model_text( QStringList files )
{
   
   QString tmpd = USglobal->config_list.root_dir + "/somo/saxs/tmp";
   QDir dir( tmpd );
   if (!dir.exists())
   {
      if ( dir.mkdir( tmpd ) )
      {
         editor_msg( "red", QString( tr( "Error: could not create temporary directory %1" ) ).arg( tmpd ) );
         return;
      }
   }

   for ( int i = 0; i < (int) files.size(); ++i )
   {
      QString file = files[ i ] + ".bead_model";
      QFile f( tmpd + "/" + file );
      if ( !f.open( QIODevice::WriteOnly ) )
      {
         editor_msg( "red", QString( tr( "Error: could not create output file %1" ) ).arg( f.name() ) );
         return;
      }
      Q3TextStream ts( &f );
      ts << models[ files[ i ] ];
      f.close();

      TextEdit *edit;
      edit = new TextEdit( this, f.name() );
      edit->setFont    ( QFont( "Courier" ) );
      edit->setPalette ( PALET_NORMAL );
      AUTFBACK( edit );
      edit->setGeometry( global_Xpos + 30, global_Ypos + 30, 685, 600 );
      edit->load( f.name() );
      edit->show();
   }
}

bool US_Hydrodyn_Saxs_Hplc::model_save( QStringList files )
{
   bool errors = false;
   bool overwrite_all = false;
   bool cancel        = false;
   for ( int i = 0; i < (int)files.size(); ++i )
   {
      if ( !model_save( files[ i ], cancel, overwrite_all ) )
      {
         errors = true;
      }
      if ( cancel )
      {
         editor_msg( "red", tr( "save cancelled" ) );
         return false;
         break;
      }
   }
   return !errors;
}

bool US_Hydrodyn_Saxs_Hplc::model_save( QString file, bool & cancel, bool & overwrite_all )
{
   if ( !models.count( file ) )
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
      use_filename = file + ".bead_model";
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

   ts << models[ file ];
   f.close();
   editor_msg( "black", QString( tr( "%1 written as %2" ) )
               .arg( file )
               .arg( use_filename ) );
   models_not_saved.erase( file );
   return true;
}
