#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_dad.h"
// #include <qsplitter.h>
//Added by qt3to4:
#include <QBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QGridLayout>
#include <QTextStream>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
 //#include <Q3PopupMenu>
#include "../include/us_eigen.h"


// #define UHSH_VAL_DEC 8

// #define ALLOW_GUOS_CARUANAS

#define POWERFIT_COLOR_Q QColor( 255, 153, 153 )
#define BASELINE2_COLOR_Q QColor( 255, 153, 153 )

void US_Hydrodyn_Dad::setupGUI()
{
   int minHeight1 = 22;
   int minHeight3 = 24;

   powerfit_color_q  = POWERFIT_COLOR_Q;
   baseline2_color_q = POWERFIT_COLOR_Q;

   QPalette cg_magenta = USglobal->global_colors.cg_normal;
   cg_magenta.setBrush( QPalette::Base, QBrush( QColor( "magenta" ), Qt::SolidPattern ) );

   /*
     cg_magenta.setBrush( QPalette::WindowText, QBrush( QColor( "magenta" ), Qt::SolidPattern ) );
     cg_magenta.setBrush( QPalette::Button, QBrush( QColor( "blue" ), Qt::SolidPattern ) );
     cg_magenta.setBrush( QPalette::Light, QBrush( QColor( "darkcyan" ), Qt::SolidPattern ) );
     cg_magenta.setBrush( QPalette::Midlight, QBrush( QColor( "darkblue" ), Qt::SolidPattern ) );
     cg_magenta.setBrush( QPalette::Dark, QBrush( QColor( "yellow" ), Qt::SolidPattern ) );
     cg_magenta.setBrush( QPalette::Mid, QBrush( QColor( "darkred" ), Qt::SolidPattern ) );
     cg_magenta.setBrush( QPalette::Text, QBrush( QColor( "green" ), Qt::SolidPattern ) );
     cg_magenta.setBrush( QPalette::BrightText, QBrush( QColor( "darkgreen" ), Qt::SolidPattern ) );
     cg_magenta.setBrush( QPalette::ButtonText, QBrush( QColor( "cyan" ), Qt::SolidPattern ) );
     cg_magenta.setBrush( QPalette::Base, QBrush( QColor( "gray" ), Qt::SolidPattern ) );
     cg_magenta.setBrush( QPalette::Shadow, QBrush( QColor( "magenta" ), Qt::SolidPattern ) );
     cg_magenta.setBrush( QPalette::Highlight, QBrush( QColor( "darkyellow" ), Qt::SolidPattern ) );
     cg_magenta.setBrush( QPalette::HighlightedText, QBrush( QColor( "darkred" ), Qt::SolidPattern ) );
   */

   QPalette cg_red = cg_magenta;
   cg_red.setBrush( QPalette::Base, QBrush( QColor( "red" ), Qt::SolidPattern ) );

   QPalette cg_fit_1 = cg_magenta;
   cg_fit_1.setBrush( QPalette::Base, QBrush( powerfit_color_q, Qt::SolidPattern ) );

   lbl_title = new QLabel("Developed by Emre Brookes and Mattia Rocco (see TBD., 2024)", this);

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
   cb_lock_dir->setText(us_tr("Lock "));
   cb_lock_dir->setEnabled( true );
   cb_lock_dir->setChecked( false );
   cb_lock_dir->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ) );
   cb_lock_dir->setPalette( PALET_NORMAL );
   AUTFBACK( cb_lock_dir );

   // lbl_dir = new mQLabel( QDir::currentPath(), this );
   // lbl_dir->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   // lbl_dir->setPalette( PALET_NORMAL );
   // lbl_dir->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2));
   // connect( lbl_dir, SIGNAL(pressed()), SLOT( dir_pressed() ));


   le_dir = new mQLineEdit( this );
   le_dir->setText( QDir::currentPath() );
   le_dir->setFrame( false );
   le_dir->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_dir->setPalette( PALET_NORMAL );
   AUTFBACK( le_dir );
   le_dir->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2));
   connect( le_dir, SIGNAL(pressed()), SLOT( dir_pressed() ));

   pb_add_files = new QPushButton(us_tr("Add files"), this);
   pb_add_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_add_files->setMinimumHeight(minHeight3);
   pb_add_files->setPalette( PALET_PUSHB );
   connect(pb_add_files, SIGNAL(clicked()), SLOT(add_files()));

   pb_add_dir = new QPushButton(us_tr("Add Dir."), this);
   pb_add_dir->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_add_dir->setMinimumHeight(minHeight3);
   pb_add_dir->setPalette( PALET_PUSHB );
   connect(pb_add_dir, SIGNAL(clicked()), SLOT(add_dir()));

   pb_similar_files = new QPushButton(us_tr("Similar"), this);
   pb_similar_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_similar_files->setMinimumHeight(minHeight3);
   pb_similar_files->setPalette( PALET_PUSHB );
   connect(pb_similar_files, SIGNAL(clicked()), SLOT(similar_files()));
   pb_similar_files->hide();

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

   pb_save_state = new QPushButton(us_tr("SS"), this);
   pb_save_state->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_save_state->setMinimumHeight(minHeight3);
   pb_save_state->setPalette( PALET_PUSHB );
   connect(pb_save_state, SIGNAL(clicked()), SLOT(save_state()));

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

   pb_select_all = new mQPushButton(us_tr("Sel. all"), this);
   pb_select_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_select_all->setMinimumHeight(minHeight1);
   pb_select_all->setPalette( PALET_PUSHB );
   connect(pb_select_all, SIGNAL(clicked()), SLOT(select_all()));

   pb_invert = new QPushButton(us_tr("Sel. Unsel."), this);
   pb_invert->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_invert->setMinimumHeight(minHeight1);
   pb_invert->setPalette( PALET_PUSHB );
   connect(pb_invert, SIGNAL(clicked()), SLOT(invert()));

   pb_select_nth = new mQPushButton(us_tr("Adv. Sel."), this);
   pb_select_nth->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_select_nth->setMinimumHeight(minHeight1);
   pb_select_nth->setPalette( PALET_PUSHB );
   connect(pb_select_nth, SIGNAL(clicked()), SLOT(select_nth()));

   pb_line_width = new QPushButton(us_tr("Width"), this);
   pb_line_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_line_width->setMinimumHeight(minHeight1);
   // pb_line_width->setMaximumWidth ( minHeight1 * 2 );
   pb_line_width->setPalette( PALET_PUSHB );
   connect(pb_line_width, SIGNAL(clicked()), SLOT(line_width()));

   pb_color_rotate = new QPushButton(us_tr("Color"), this);
   pb_color_rotate->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_color_rotate->setMinimumHeight(minHeight1);
   // pb_color_rotate->setMaximumWidth ( minHeight1 * 2 );
   pb_color_rotate->setPalette( PALET_PUSHB );
   connect(pb_color_rotate, SIGNAL(clicked()), SLOT(color_rotate()));

   //    pb_join = new QPushButton(us_tr("J"), this);
   //    pb_join->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   //    pb_join->setMinimumHeight( minHeight1 );
   //    pb_join->setMaximumWidth ( minHeight1 * 2 );
   //    pb_join->setPalette( PALET_PUSHB );
   //    connect(pb_join, SIGNAL(clicked()), SLOT(join()));

   //    pb_adjacent = new QPushButton(us_tr("Similar"), this);
   //    pb_adjacent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   //    pb_adjacent->setMinimumHeight(minHeight1);
   //    pb_adjacent->setPalette( PALET_PUSHB );
   //    connect(pb_adjacent, SIGNAL(clicked()), SLOT(adjacent()));

   pb_to_saxs = new QPushButton(us_tr("To SOMO/SAS"), this);
   pb_to_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_to_saxs->setMinimumHeight( minHeight1 );
   // pb_to_saxs->setMaximumWidth ( minHeight1 * 2 );
   pb_to_saxs->setPalette( PALET_PUSHB );
   connect(pb_to_saxs, SIGNAL(clicked()), SLOT(to_saxs()));

   pb_view = new QPushButton(us_tr("View Selected"), this);
   pb_view->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_view->setMinimumHeight( minHeight1 );
   // pb_view->setMaximumWidth ( minHeight1 * 4 );
   pb_view->setPalette( PALET_PUSHB );
   connect(pb_view, SIGNAL(clicked()), SLOT( view() ));

   pb_movie = new mQPushButton(us_tr("Movie"), this);
   pb_movie->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_movie->setMinimumHeight(minHeight1);
   pb_movie->setPalette( PALET_PUSHB );
   connect(pb_movie, SIGNAL(clicked()), SLOT(movie()));

   cb_eb = new QCheckBox(this);
   cb_eb->setText(us_tr("Err "));
   //width cb_eb->setMaximumWidth ( minHeight1 * 3 );
   cb_eb->setChecked( false );
   cb_eb->setMinimumHeight( minHeight1 );
   cb_eb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ) );
   cb_eb->setPalette( PALET_NORMAL );
   AUTFBACK( cb_eb );
   connect( cb_eb, SIGNAL( clicked() ), SLOT( set_eb() ) );

   cb_dots = new QCheckBox(this);
   cb_dots->setText(us_tr("Dots "));
   //width cb_dots->setMaximumWidth ( minHeight1 * 3 );
   cb_dots->setChecked( false );
   cb_dots->setMinimumHeight( minHeight1 );
   cb_dots->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ) );
   cb_dots->setPalette( PALET_NORMAL );
   AUTFBACK( cb_dots );
   connect( cb_dots, SIGNAL( clicked() ), SLOT( set_dots() ) );

   pb_rescale = new QPushButton(us_tr("Rescale XY"), this);
   pb_rescale->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_rescale->setMinimumHeight(minHeight1);
   pb_rescale->setPalette( PALET_PUSHB );
   connect(pb_rescale, SIGNAL(clicked()), SLOT(rescale()));

   pb_rescale_y = new QPushButton(us_tr("Rescale Y"), this);
   pb_rescale_y->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_rescale_y->setMinimumHeight(minHeight1);
   pb_rescale_y->setPalette( PALET_PUSHB );
   connect(pb_rescale_y, SIGNAL(clicked()), SLOT(rescale_y()));

   pb_ag = new QPushButton(us_tr("AG"), this);
   pb_ag->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_ag->setMinimumHeight(minHeight1);
   pb_ag->setPalette( PALET_PUSHB );
   connect( pb_ag, SIGNAL( clicked() ), SLOT( artificial_gaussians() ) );

   pb_stack_push_all = new QPushButton(us_tr("Psh"), this);
   pb_stack_push_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_push_all->setMinimumHeight(minHeight1);
   pb_stack_push_all->setPalette( PALET_PUSHB );
   connect(pb_stack_push_all, SIGNAL(clicked()), SLOT(stack_push_all()));

   pb_stack_push_sel = new QPushButton(us_tr("Psl"), this);
   pb_stack_push_sel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_push_sel->setMinimumHeight(minHeight1);
   pb_stack_push_sel->setPalette( PALET_PUSHB );
   connect(pb_stack_push_sel, SIGNAL(clicked()), SLOT(stack_push_sel()));

   lbl_stack = new QLabel( "", this );
   lbl_stack->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_stack->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_stack );
   lbl_stack->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2));

   pb_stack_copy = new QPushButton(us_tr("Cpy"), this);
   pb_stack_copy->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_copy->setMinimumHeight(minHeight1);
   pb_stack_copy->setPalette( PALET_PUSHB );
   connect(pb_stack_copy, SIGNAL(clicked()), SLOT(stack_copy()));

   pb_stack_pcopy = new QPushButton(us_tr("Pcp"), this);
   pb_stack_pcopy->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_pcopy->setMinimumHeight(minHeight1);
   pb_stack_pcopy->setPalette( PALET_PUSHB );
   connect(pb_stack_pcopy, SIGNAL(clicked()), SLOT(stack_pcopy()));

   pb_stack_paste = new QPushButton(us_tr("Pst"), this);
   pb_stack_paste->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_paste->setMinimumHeight(minHeight1);
   pb_stack_paste->setPalette( PALET_PUSHB );
   connect(pb_stack_paste, SIGNAL(clicked()), SLOT(stack_paste()));

   pb_stack_drop = new QPushButton(us_tr("Drp"), this);
   pb_stack_drop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_drop->setMinimumHeight(minHeight1);
   pb_stack_drop->setPalette( PALET_PUSHB );
   connect(pb_stack_drop, SIGNAL(clicked()), SLOT(stack_drop()));

   pb_stack_join = new QPushButton(us_tr("Jn"), this);
   pb_stack_join->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_join->setMinimumHeight(minHeight1);
   pb_stack_join->setPalette( PALET_PUSHB );
   connect(pb_stack_join, SIGNAL(clicked()), SLOT(stack_join()));

   pb_stack_rot_up = new QPushButton(us_tr("Rdn"), this);
   pb_stack_rot_up->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_rot_up->setMinimumHeight(minHeight1);
   pb_stack_rot_up->setPalette( PALET_PUSHB );
   connect(pb_stack_rot_up, SIGNAL(clicked()), SLOT(stack_rot_up()));

   pb_stack_rot_down = new QPushButton(us_tr("Rup"), this);
   pb_stack_rot_down->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_rot_down->setMinimumHeight(minHeight1);
   pb_stack_rot_down->setPalette( PALET_PUSHB );
   connect(pb_stack_rot_down, SIGNAL(clicked()), SLOT(stack_rot_down()));

   pb_stack_swap = new QPushButton(us_tr("Swp"), this);
   pb_stack_swap->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stack_swap->setMinimumHeight(minHeight1);
   pb_stack_swap->setPalette( PALET_PUSHB );
   connect(pb_stack_swap, SIGNAL(clicked()), SLOT(stack_swap()));

   // pb_plot_files = new QPushButton(us_tr("Plot"), this);
   // pb_plot_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   // pb_plot_files->setMinimumHeight(minHeight1);
   // pb_plot_files->setPalette( PALET_PUSHB );
   // connect(pb_plot_files, SIGNAL(clicked()), SLOT(plot_files()));

   pb_conc_avg = new QPushButton(us_tr("Conc. norm. avg."), this);
   pb_conc_avg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_conc_avg->setMinimumHeight(minHeight1);
   pb_conc_avg->setPalette( PALET_PUSHB );
   connect(pb_conc_avg, SIGNAL(clicked()), SLOT(conc_avg()));

   pb_normalize = new QPushButton(us_tr("Conc. Norm."), this);
   pb_normalize->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_normalize->setMinimumHeight(minHeight1);
   pb_normalize->setPalette( PALET_PUSHB );
   connect(pb_normalize, SIGNAL(clicked()), SLOT(normalize()));

   pb_add = new QPushButton(us_tr("Add"), this);
   pb_add->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_add->setMinimumHeight(minHeight1);
   pb_add->setPalette( PALET_PUSHB );
   connect(pb_add, SIGNAL(clicked()), SLOT(add()));

   pb_avg = new QPushButton(us_tr("Average"), this);
   pb_avg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_avg->setMinimumHeight(minHeight1);
   pb_avg->setPalette( PALET_PUSHB );
   connect(pb_avg, SIGNAL(clicked()), SLOT(avg()));

   pb_bin = new QPushButton(us_tr("Bin"), this);
   pb_bin->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_bin->setMinimumHeight(minHeight1);
   pb_bin->setPalette( PALET_PUSHB );
   connect(pb_bin, SIGNAL(clicked()), SLOT(bin()));

   pb_smooth = new QPushButton(us_tr("Smooth"), this);
   pb_smooth->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_smooth->setMinimumHeight(minHeight1);
   pb_smooth->setPalette( PALET_PUSHB );
   connect(pb_smooth, SIGNAL(clicked()), SLOT(smooth()));

   // powerfit start

   pb_powerfit = new QPushButton(us_tr("Concentration"), this);
   pb_powerfit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_powerfit->setMinimumHeight(minHeight1);
   pb_powerfit->setPalette( PALET_PUSHB );
   connect(pb_powerfit, SIGNAL(clicked()), SLOT(powerfit()));

   lbl_powerfit_msg = new QLabel( "", this );
   lbl_powerfit_msg->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_powerfit_msg->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_powerfit_msg );
   lbl_powerfit_msg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_powerfit_msg->setTextInteractionFlags(Qt::TextSelectableByMouse);
   lbl_powerfit_msg->setMinimumWidth( QFontMetrics(lbl_powerfit_msg->font()).averageCharWidth() * 40 );
   lbl_powerfit_msg->setCursor(QCursor(Qt::IBeamCursor));

   pb_powerfit_fit = new QPushButton(us_tr("Fit"), this);
   pb_powerfit_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_powerfit_fit->setMinimumHeight(minHeight1);
   pb_powerfit_fit->setPalette( PALET_PUSHB );
   connect(pb_powerfit_fit, SIGNAL(clicked()), SLOT(powerfit_fit()));

   // lbl_powerfit_fit_curve = new QLabel( "Fit curve ", this );
   lbl_powerfit_fit_curve = new QLabel( "Fit controls: ", this );
   lbl_powerfit_fit_curve->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_powerfit_fit_curve->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_powerfit_fit_curve );
   lbl_powerfit_fit_curve->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   cb_powerfit_fit_curve = new QComboBox( this );
   cb_powerfit_fit_curve->setPalette( PALET_NORMAL );
   AUTFBACK( cb_powerfit_fit_curve );
   cb_powerfit_fit_curve->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   cb_powerfit_fit_curve->setEnabled(true);
   cb_powerfit_fit_curve->setMinimumHeight( minHeight1 );
   cb_powerfit_fit_curve->setMaxVisibleItems( 1 );

   cb_powerfit_fit_curve->addItem( us_tr( "2nd degree Polynomial" ), POWERFIT_FIT_CURVE_P2 );
   cb_powerfit_fit_curve->addItem( us_tr( "3rd degree Polynomial" ), POWERFIT_FIT_CURVE_P3 );
   cb_powerfit_fit_curve->addItem( us_tr( "4th degree Polynomial" ), POWERFIT_FIT_CURVE_P4 );
   cb_powerfit_fit_curve->addItem( us_tr( "5th degree Polynomial" ), POWERFIT_FIT_CURVE_P5 );
   cb_powerfit_fit_curve->addItem( us_tr( "6th degree Polynomial" ), POWERFIT_FIT_CURVE_P6 );
   cb_powerfit_fit_curve->addItem( us_tr( "7th degree Polynomial" ), POWERFIT_FIT_CURVE_P7 );
   cb_powerfit_fit_curve->addItem( us_tr( "8th degree Polynomial" ), POWERFIT_FIT_CURVE_P8 );
   cb_powerfit_fit_curve->setCurrentIndex( 1 );
   connect( cb_powerfit_fit_curve, SIGNAL( currentIndexChanged( QString ) ), SLOT( powerfit_fit_curve_index( ) ) );

   cb_powerfit_fit_alg = new QComboBox( this );
   cb_powerfit_fit_alg->setPalette( PALET_NORMAL );
   AUTFBACK( cb_powerfit_fit_alg );
   cb_powerfit_fit_alg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   cb_powerfit_fit_alg->setEnabled(true);
   cb_powerfit_fit_alg->setMinimumHeight( minHeight1 );
   cb_powerfit_fit_alg->setMaxVisibleItems( 1 );

   cb_powerfit_fit_alg->addItem( us_tr( "SVD BDC" )                       , US_Eigen::EIGEN_SVD_BDC );
   cb_powerfit_fit_alg->addItem( us_tr( "SVD Jacobi" )                    , US_Eigen::EIGEN_SVD_JACOBI );
   cb_powerfit_fit_alg->addItem( us_tr( "QR Householder full pivoting" )  , US_Eigen::EIGEN_HOUSEHOLDER_QR_PIVOT_FULL );
   cb_powerfit_fit_alg->addItem( us_tr( "QR Householder column pivoting" ), US_Eigen::EIGEN_HOUSEHOLDER_QR_PIVOT_COL);
   cb_powerfit_fit_alg->addItem( us_tr( "QR Householder" )                , US_Eigen::EIGEN_HOUSEHOLDER_QR );
   cb_powerfit_fit_alg->addItem( us_tr( "LR" )                            , US_Eigen::EIGEN_NORMAL );
   cb_powerfit_fit_alg->setCurrentIndex( 5 );
   connect( cb_powerfit_fit_alg, SIGNAL( currentIndexChanged( QString ) ), SLOT( powerfit_fit_alg_index( ) ) );

   cb_powerfit_fit_alg_weight = new QComboBox( this );
   cb_powerfit_fit_alg_weight->setPalette( PALET_NORMAL );
   AUTFBACK( cb_powerfit_fit_alg_weight );
   cb_powerfit_fit_alg_weight->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   cb_powerfit_fit_alg_weight->setEnabled(true);
   cb_powerfit_fit_alg_weight->setMinimumHeight( minHeight1 );
   cb_powerfit_fit_alg_weight->setMaxVisibleItems( 1 );

   cb_powerfit_fit_alg_weight->addItem( us_tr( "no weighting" ), US_Eigen::EIGEN_NO_WEIGHTS );
   cb_powerfit_fit_alg_weight->addItem( us_tr( "1/amount" )    , US_Eigen::EIGEN_1_OVER_AMOUNT );
   cb_powerfit_fit_alg_weight->addItem( us_tr( "1/amount^2" )  , US_Eigen::EIGEN_1_OVER_AMOUNT_SQ );
   cb_powerfit_fit_alg_weight->addItem( us_tr( "1/SD" )        , US_Eigen::EIGEN_1_OVER_SD );
   cb_powerfit_fit_alg_weight->addItem( us_tr( "1/SD^2" )      , US_Eigen::EIGEN_1_OVER_SD_SQ );

   cb_powerfit_fit_alg_weight->setCurrentIndex( 3 ); // US_Eigen::EIGEN_1_OVER_SD
   connect( cb_powerfit_fit_alg_weight, SIGNAL( currentIndexChanged( QString ) ), SLOT( powerfit_fit_alg_weight_index( ) ) );

   pb_powerfit_reset = new QPushButton(us_tr("Reset"), this);
   pb_powerfit_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_powerfit_reset->setMinimumHeight(minHeight1);
   pb_powerfit_reset->setPalette( PALET_PUSHB );
   connect(pb_powerfit_reset, SIGNAL(clicked()), SLOT(powerfit_reset()));

   pb_powerfit_create_adjusted_curve = new QPushButton(us_tr("Create adjusted curve"), this);
   pb_powerfit_create_adjusted_curve->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_powerfit_create_adjusted_curve->setMinimumHeight(minHeight1);
   pb_powerfit_create_adjusted_curve->setPalette( PALET_PUSHB );
   connect(pb_powerfit_create_adjusted_curve, SIGNAL(clicked()), SLOT(powerfit_create_adjusted_curve()));
   
   lbl_powerfit_q_range = new QLabel( us_tr( " Fit range: " ), this );
   lbl_powerfit_q_range->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_powerfit_q_range->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_powerfit_q_range );
   lbl_powerfit_q_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_powerfit_q_range->hide();

   le_powerfit_q_start = new mQLineEdit( this );    le_powerfit_q_start->setObjectName( "le_powerfit_q_start Line Edit" );
   le_powerfit_q_start->setText( "350" );
   le_powerfit_q_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_powerfit_q_start->setPalette( cg_fit_1 );
   le_powerfit_q_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_powerfit_q_start->setEnabled( false );
   le_powerfit_q_start->setValidator( new QDoubleValidator( le_powerfit_q_start ) );
   le_powerfit_q_start->hide();
   connect( le_powerfit_q_start, SIGNAL( textChanged( const QString & ) ), SLOT( powerfit_q_start_text( const QString & ) ) );
   connect( le_powerfit_q_start, SIGNAL( focussed ( bool ) )             , SLOT( powerfit_q_start_focus( bool ) ) );

   le_powerfit_q_end = new mQLineEdit( this );    le_powerfit_q_end->setObjectName( "le_powerfit_q_end Line Edit" );
   le_powerfit_q_end->setText( "" );
   le_powerfit_q_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_powerfit_q_end->setPalette( cg_fit_1 );
   le_powerfit_q_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_powerfit_q_end->setEnabled( false );
   le_powerfit_q_end->setValidator( new QDoubleValidator( le_powerfit_q_end ) );
   le_powerfit_q_end->hide();
   connect( le_powerfit_q_end, SIGNAL( textChanged( const QString & ) ), SLOT( powerfit_q_end_text( const QString & ) ) );
   connect( le_powerfit_q_end, SIGNAL( focussed ( bool ) )             , SLOT( powerfit_q_end_focus( bool ) ) );

   lbl_powerfit_a = new QLabel( us_tr( " A: " ), this );
   lbl_powerfit_a->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_powerfit_a->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_powerfit_a );
   lbl_powerfit_a->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_powerfit_a->hide();

   le_powerfit_a = new mQLineEdit( this );    le_powerfit_a->setObjectName( "le_powerfit_a Line Edit" );
   le_powerfit_a->setText( "0" );
   le_powerfit_a->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_powerfit_a->setPalette( PALET_NORMAL );
   le_powerfit_a->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_powerfit_a->setEnabled( false );
   le_powerfit_a->setValidator( new QDoubleValidator( le_powerfit_a ) );
   le_powerfit_a->hide();
   connect( le_powerfit_a, SIGNAL( textChanged( const QString & ) ), SLOT( powerfit_a_text( const QString & ) ) );
   connect( le_powerfit_a, SIGNAL( focussed ( bool ) )             , SLOT( powerfit_a_focus( bool ) ) );
   
   lbl_powerfit_b = new QLabel( us_tr( " B: " ), this );
   lbl_powerfit_b->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_powerfit_b->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_powerfit_b );
   lbl_powerfit_b->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_powerfit_b->hide();

   le_powerfit_b = new mQLineEdit( this );    le_powerfit_b->setObjectName( "le_powerfit_b Line Edit" );
   le_powerfit_b->setText( "1" );
   le_powerfit_b->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_powerfit_b->setPalette( PALET_NORMAL );
   le_powerfit_b->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_powerfit_b->setEnabled( false );
   le_powerfit_b->setValidator( new QDoubleValidator( le_powerfit_b ) );
   le_powerfit_b->hide();
   connect( le_powerfit_b, SIGNAL( textChanged( const QString & ) ), SLOT( powerfit_b_text( const QString & ) ) );
   connect( le_powerfit_b, SIGNAL( focussed ( bool ) )             , SLOT( powerfit_b_focus( bool ) ) );
   
   lbl_powerfit_c = new QLabel( us_tr( " C,Min.,Max.: " ), this );
   lbl_powerfit_c->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_powerfit_c->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_powerfit_c );
   lbl_powerfit_c->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_powerfit_c->hide();

   le_powerfit_c = new mQLineEdit( this );    le_powerfit_c->setObjectName( "le_powerfit_c Line Edit" );
   le_powerfit_c->setText( "4" );
   le_powerfit_c->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_powerfit_c->setPalette( PALET_NORMAL );
   le_powerfit_c->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_powerfit_c->setEnabled( false );
   le_powerfit_c->setValidator( new QDoubleValidator( le_powerfit_c ) );
   le_powerfit_c->hide();
   connect( le_powerfit_c, SIGNAL( textChanged( const QString & ) ), SLOT( powerfit_c_text( const QString & ) ) );
   connect( le_powerfit_c, SIGNAL( focussed ( bool ) )             , SLOT( powerfit_c_focus( bool ) ) );

   le_powerfit_c_min = new mQLineEdit( this );    le_powerfit_c_min->setObjectName( "le_powerfit_c_min Line Edit" );
   le_powerfit_c_min->setText( "3.9" );
   le_powerfit_c_min->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_powerfit_c_min->setPalette( PALET_NORMAL );
   le_powerfit_c_min->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_powerfit_c_min->setEnabled( false );
   le_powerfit_c_min->setValidator( new QDoubleValidator( le_powerfit_c_min ) );
   le_powerfit_c_min->hide();
   connect( le_powerfit_c_min, SIGNAL( textChanged( const QString & ) ), SLOT( powerfit_c_min_text( const QString & ) ) );
   connect( le_powerfit_c_min, SIGNAL( focussed ( bool ) )             , SLOT( powerfit_c_min_focus( bool ) ) );
   
   le_powerfit_c_max = new mQLineEdit( this );    le_powerfit_c_max->setObjectName( "le_powerfit_c_max Line Edit" );
   le_powerfit_c_max->setText( "4.1" );
   le_powerfit_c_max->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_powerfit_c_max->setPalette( PALET_NORMAL );
   le_powerfit_c_max->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_powerfit_c_max->setEnabled( false );
   le_powerfit_c_max->setValidator( new QDoubleValidator( le_powerfit_c_max ) );
   le_powerfit_c_max->hide();
   connect( le_powerfit_c_max, SIGNAL( textChanged( const QString & ) ), SLOT( powerfit_c_max_text( const QString & ) ) );
   connect( le_powerfit_c_max, SIGNAL( focussed ( bool ) )             , SLOT( powerfit_c_max_focus( bool ) ) );
   
   lbl_powerfit_fit_epsilon = new QLabel( us_tr( UNICODE_EPSILON_QS + " : " ), this );
   lbl_powerfit_fit_epsilon->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_powerfit_fit_epsilon->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_powerfit_fit_epsilon );
   lbl_powerfit_fit_epsilon->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_powerfit_fit_epsilon->hide();

   le_powerfit_fit_epsilon = new mQLineEdit( this );    le_powerfit_fit_epsilon->setObjectName( "le_powerfit_fit_epsilon Line Edit" );
   le_powerfit_fit_epsilon->setText( "1e-7" );
   le_powerfit_fit_epsilon->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_powerfit_fit_epsilon->setPalette( PALET_NORMAL );
   le_powerfit_fit_epsilon->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_powerfit_fit_epsilon->setEnabled( false );
   le_powerfit_fit_epsilon->setValidator( new QDoubleValidator( le_powerfit_fit_epsilon ) );
   le_powerfit_fit_epsilon->hide();
   connect( le_powerfit_fit_epsilon, SIGNAL( textChanged( const QString & ) ), SLOT( powerfit_fit_epsilon_text( const QString & ) ) );
   connect( le_powerfit_fit_epsilon, SIGNAL( focussed ( bool ) )             , SLOT( powerfit_fit_epsilon_focus( bool ) ) );

   lbl_powerfit_fit_iterations = new QLabel( us_tr( " Iter.: " ), this );
   lbl_powerfit_fit_iterations->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_powerfit_fit_iterations->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_powerfit_fit_iterations );
   lbl_powerfit_fit_iterations->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_powerfit_fit_iterations->hide();

   le_powerfit_fit_iterations = new mQLineEdit( this );    le_powerfit_fit_iterations->setObjectName( "le_powerfit_fit_iterations Line Edit" );
   le_powerfit_fit_iterations->setText( "1000" );
   le_powerfit_fit_iterations->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_powerfit_fit_iterations->setPalette( PALET_NORMAL );
   le_powerfit_fit_iterations->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_powerfit_fit_iterations->setEnabled( false );
   le_powerfit_fit_iterations->setValidator( new QIntValidator( 2, 1000000, le_powerfit_fit_iterations ) );
   le_powerfit_fit_iterations->hide();
   connect( le_powerfit_fit_iterations, SIGNAL( textChanged( const QString & ) ), SLOT( powerfit_fit_iterations_text( const QString & ) ) );
   connect( le_powerfit_fit_iterations, SIGNAL( focussed ( bool ) )             , SLOT( powerfit_fit_iterations_focus( bool ) ) );

   lbl_powerfit_fit_max_calls = new QLabel( us_tr( " Max. calls: " ), this );
   lbl_powerfit_fit_max_calls->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_powerfit_fit_max_calls->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_powerfit_fit_max_calls );
   lbl_powerfit_fit_max_calls->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_powerfit_fit_max_calls->hide();

   le_powerfit_fit_max_calls = new mQLineEdit( this );    le_powerfit_fit_max_calls->setObjectName( "le_powerfit_fit_max_calls Line Edit" );
   le_powerfit_fit_max_calls->setText( "10000" );
   le_powerfit_fit_max_calls->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_powerfit_fit_max_calls->setPalette( PALET_NORMAL );
   le_powerfit_fit_max_calls->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_powerfit_fit_max_calls->setEnabled( false );
   le_powerfit_fit_max_calls->setValidator( new QIntValidator( 2, 1000000, le_powerfit_fit_max_calls ) );
   le_powerfit_fit_max_calls->hide();
   connect( le_powerfit_fit_max_calls, SIGNAL( textChanged( const QString & ) ), SLOT( powerfit_fit_max_calls_text( const QString & ) ) );
   connect( le_powerfit_fit_max_calls, SIGNAL( focussed ( bool ) )             , SLOT( powerfit_fit_max_calls_focus( bool ) ) );
   
   lbl_powerfit_uncorrected_conc = new QLabel( us_tr( " Uncorr., Scat. corr.,Full corr. c [mg/mL]: " ), this );
   lbl_powerfit_uncorrected_conc->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_powerfit_uncorrected_conc->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_powerfit_uncorrected_conc );
   lbl_powerfit_uncorrected_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_powerfit_uncorrected_conc->hide();

   le_powerfit_uncorrected_conc = new mQLineEdit( this );    le_powerfit_uncorrected_conc->setObjectName( "le_powerfit_uncorrected_conc Line Edit" );
   le_powerfit_uncorrected_conc->setText( "" );
   le_powerfit_uncorrected_conc->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_powerfit_uncorrected_conc->setPalette( PALET_NORMAL );
   le_powerfit_uncorrected_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_powerfit_uncorrected_conc->setEnabled( true );
   le_powerfit_uncorrected_conc->setReadOnly( true );
   le_powerfit_uncorrected_conc->hide();
   connect( le_powerfit_uncorrected_conc, SIGNAL( textChanged( const QString & ) ), SLOT( powerfit_uncorrected_conc_text( const QString & ) ) );
   connect( le_powerfit_uncorrected_conc, SIGNAL( focussed ( bool ) )             , SLOT( powerfit_uncorrected_conc_focus( bool ) ) );

   lbl_powerfit_scat_conc = new QLabel( us_tr( "," ), this );
   lbl_powerfit_scat_conc->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_powerfit_scat_conc->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_powerfit_scat_conc );
   lbl_powerfit_scat_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_powerfit_scat_conc->hide();

   le_powerfit_scat_conc = new mQLineEdit( this );    le_powerfit_scat_conc->setObjectName( "le_powerfit_scat_conc Line Edit" );
   le_powerfit_scat_conc->setText( "" );
   le_powerfit_scat_conc->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_powerfit_scat_conc->setPalette( PALET_NORMAL );
   le_powerfit_scat_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_powerfit_scat_conc->setEnabled( true );
   le_powerfit_scat_conc->setReadOnly( true );
   le_powerfit_scat_conc->hide();
   connect( le_powerfit_scat_conc, SIGNAL( textChanged( const QString & ) ), SLOT( powerfit_scat_conc_text( const QString & ) ) );
   connect( le_powerfit_scat_conc, SIGNAL( focussed ( bool ) )             , SLOT( powerfit_scat_conc_focus( bool ) ) );

   lbl_powerfit_computed_conc = new QLabel( us_tr( "," ), this );
   lbl_powerfit_computed_conc->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_powerfit_computed_conc->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_powerfit_computed_conc );
   lbl_powerfit_computed_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_powerfit_computed_conc->hide();

   le_powerfit_computed_conc = new mQLineEdit( this );    le_powerfit_computed_conc->setObjectName( "le_powerfit_computed_conc Line Edit" );
   le_powerfit_computed_conc->setText( "" );
   le_powerfit_computed_conc->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_powerfit_computed_conc->setPalette( PALET_NORMAL );
   le_powerfit_computed_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_powerfit_computed_conc->setEnabled( true );
   le_powerfit_computed_conc->setReadOnly( true );
   le_powerfit_computed_conc->hide();
   connect( le_powerfit_computed_conc, SIGNAL( textChanged( const QString & ) ), SLOT( powerfit_computed_conc_text( const QString & ) ) );
   connect( le_powerfit_computed_conc, SIGNAL( focussed ( bool ) )             , SLOT( powerfit_computed_conc_focus( bool ) ) );
   
   lbl_powerfit_lambda = new QLabel( us_tr( QString( "%1(abs) [nm]:" ).arg( UNICODE_LAMBDA ) ), this );
   lbl_powerfit_lambda->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_powerfit_lambda->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_powerfit_lambda );
   lbl_powerfit_lambda->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_powerfit_lambda->hide();

   le_powerfit_lambda = new mQLineEdit( this );    le_powerfit_lambda->setObjectName( "le_powerfit_lambda Line Edit" );
   le_powerfit_lambda->setText( "" );
   le_powerfit_lambda->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_powerfit_lambda->setPalette( PALET_NORMAL );
   le_powerfit_lambda->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_powerfit_lambda->setValidator( new QDoubleValidator( le_powerfit_lambda ) );
   le_powerfit_lambda->setEnabled( true );
   le_powerfit_lambda->setMaxLength(8);
   le_powerfit_lambda->setMaximumWidth( QFontMetrics(le_powerfit_lambda->font()).averageCharWidth() * le_powerfit_lambda->maxLength() );
   le_powerfit_lambda->hide();
   connect( le_powerfit_lambda, SIGNAL( textChanged( const QString & ) ), SLOT( powerfit_lambda_text( const QString & ) ) );
   connect( le_powerfit_lambda, SIGNAL( focussed ( bool ) )             , SLOT( powerfit_lambda_focus( bool ) ) );
   
   lbl_powerfit_lambda_abs = new QLabel( us_tr( QString( "A(%1,abs) [a.u.]:" ).arg( UNICODE_LAMBDA ) ), this );
   lbl_powerfit_lambda_abs->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_powerfit_lambda_abs->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_powerfit_lambda_abs );
   lbl_powerfit_lambda_abs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_powerfit_lambda_abs->hide();

   le_powerfit_lambda_abs = new mQLineEdit( this );    le_powerfit_lambda_abs->setObjectName( "le_powerfit_lambda_abs Line Edit" );
   le_powerfit_lambda_abs->setText( "" );
   le_powerfit_lambda_abs->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_powerfit_lambda_abs->setPalette( PALET_NORMAL );
   le_powerfit_lambda_abs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_powerfit_lambda_abs->setValidator( new QDoubleValidator( le_powerfit_lambda_abs ) );
   le_powerfit_lambda_abs->setMaxLength(10);
   le_powerfit_lambda_abs->setMaximumWidth( QFontMetrics(le_powerfit_lambda_abs->font()).averageCharWidth() * le_powerfit_lambda_abs->maxLength() );
   le_powerfit_lambda_abs->setEnabled( true );
   le_powerfit_lambda_abs->setReadOnly( true );
   le_powerfit_lambda_abs->hide();
   connect( le_powerfit_lambda_abs, SIGNAL( textChanged( const QString & ) ), SLOT( powerfit_lambda_abs_text( const QString & ) ) );
   connect( le_powerfit_lambda_abs, SIGNAL( focussed ( bool ) )             , SLOT( powerfit_lambda_abs_focus( bool ) ) );

   lbl_powerfit_lambda2 = new QLabel( us_tr( QString( "%1(scat) [nm]:" ).arg( UNICODE_LAMBDA ) ), this );
   lbl_powerfit_lambda2->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_powerfit_lambda2->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_powerfit_lambda2 );
   lbl_powerfit_lambda2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_powerfit_lambda2->hide();

   le_powerfit_lambda2 = new mQLineEdit( this );    le_powerfit_lambda2->setObjectName( "le_powerfit_lambda2 Line Edit" );
   le_powerfit_lambda2->setText( "" );
   le_powerfit_lambda2->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_powerfit_lambda2->setPalette( PALET_NORMAL );
   le_powerfit_lambda2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_powerfit_lambda2->setValidator( new QDoubleValidator( le_powerfit_lambda2 ) );
   le_powerfit_lambda2->setMaxLength(8);
   le_powerfit_lambda2->setMaximumWidth( QFontMetrics(le_powerfit_lambda2->font()).averageCharWidth() * le_powerfit_lambda2->maxLength() );
   le_powerfit_lambda2->setEnabled( true );
   le_powerfit_lambda2->hide();
   connect( le_powerfit_lambda2, SIGNAL( textChanged( const QString & ) ), SLOT( powerfit_lambda2_text( const QString & ) ) );
   connect( le_powerfit_lambda2, SIGNAL( focussed ( bool ) )             , SLOT( powerfit_lambda2_focus( bool ) ) );

   lbl_powerfit_lambda2_abs = new QLabel( us_tr( QString( "A(%1,scat) [a.u.]:" ).arg( UNICODE_LAMBDA ) ), this );
   lbl_powerfit_lambda2_abs->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_powerfit_lambda2_abs->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_powerfit_lambda2_abs );
   lbl_powerfit_lambda2_abs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_powerfit_lambda2_abs->hide();

   le_powerfit_lambda2_abs = new mQLineEdit( this );    le_powerfit_lambda2_abs->setObjectName( "le_powerfit_lambda2_abs Line Edit" );
   le_powerfit_lambda2_abs->setText( "" );
   le_powerfit_lambda2_abs->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_powerfit_lambda2_abs->setPalette( PALET_NORMAL );
   le_powerfit_lambda2_abs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_powerfit_lambda2_abs->setValidator( new QDoubleValidator( le_powerfit_lambda2_abs ) );
   le_powerfit_lambda2_abs->setMaxLength(10);
   le_powerfit_lambda2_abs->setMaximumWidth( QFontMetrics(le_powerfit_lambda2_abs->font()).averageCharWidth() * le_powerfit_lambda2_abs->maxLength() );
   le_powerfit_lambda2_abs->setReadOnly( true );
   le_powerfit_lambda2_abs->setEnabled( true );
   le_powerfit_lambda2_abs->hide();
   connect( le_powerfit_lambda2_abs, SIGNAL( textChanged( const QString & ) ), SLOT( powerfit_lambda2_abs_text( const QString & ) ) );
   connect( le_powerfit_lambda2_abs, SIGNAL( focussed ( bool ) )             , SLOT( powerfit_lambda2_abs_focus( bool ) ) );

   lbl_powerfit_extinction_coef = new QLabel( us_tr( " Extc. coef. [mL/(mg*cm)]: " ), this );
   lbl_powerfit_extinction_coef->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_powerfit_extinction_coef->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_powerfit_extinction_coef );
   lbl_powerfit_extinction_coef->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_powerfit_extinction_coef->hide();

   le_powerfit_extinction_coef = new mQLineEdit( this );    le_powerfit_extinction_coef->setObjectName( "le_powerfit_extinction_coef Line Edit" );
   le_powerfit_extinction_coef->setText( "" );
   le_powerfit_extinction_coef->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_powerfit_extinction_coef->setPalette( PALET_NORMAL );
   le_powerfit_extinction_coef->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_powerfit_extinction_coef->setValidator( new QDoubleValidator( le_powerfit_extinction_coef ) );
   le_powerfit_extinction_coef->setMaxLength(10);
   le_powerfit_extinction_coef->setMaximumWidth( QFontMetrics(le_powerfit_extinction_coef->font()).averageCharWidth() * le_powerfit_extinction_coef->maxLength() );
   le_powerfit_extinction_coef->setEnabled( true );
   le_powerfit_extinction_coef->hide();
   connect( le_powerfit_extinction_coef, SIGNAL( textChanged( const QString & ) ), SLOT( powerfit_extinction_coef_text( const QString & ) ) );
   connect( le_powerfit_extinction_coef, SIGNAL( focussed ( bool ) )             , SLOT( powerfit_extinction_coef_focus( bool ) ) );
   
   cb_powerfit_dispersion_correction = new QCheckBox(this);
   cb_powerfit_dispersion_correction->setText(us_tr("Disp. Corr."));
   cb_powerfit_dispersion_correction->setChecked( false );
   cb_powerfit_dispersion_correction->setMinimumHeight( minHeight1 );
   cb_powerfit_dispersion_correction->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_powerfit_dispersion_correction->setPalette( PALET_NORMAL );
   AUTFBACK( cb_powerfit_dispersion_correction );
   connect( cb_powerfit_dispersion_correction, SIGNAL( clicked() ), SLOT( powerfit_dispersion_correction_clicked() ) );

   // powerfit end

   // baseline2 start

   pb_baseline2_start = new QPushButton(us_tr("Baseline"), this);
   pb_baseline2_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_baseline2_start->setMinimumHeight(minHeight1);
   pb_baseline2_start->setPalette( PALET_PUSHB );
   connect(pb_baseline2_start, SIGNAL(clicked()), SLOT(baseline2_start()));

   pb_baseline2_apply = new QPushButton(us_tr("Baseline apply"), this);
   pb_baseline2_apply->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_baseline2_apply->setMinimumHeight(minHeight1);
   pb_baseline2_apply->setPalette( PALET_PUSHB );
   connect(pb_baseline2_apply, SIGNAL(clicked()), SLOT(baseline2_apply()));

   lbl_baseline2_msg = new QLabel( "", this );
   lbl_baseline2_msg->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_baseline2_msg->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_baseline2_msg );
   lbl_baseline2_msg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_baseline2_msg->setTextInteractionFlags(Qt::TextSelectableByMouse);
   lbl_baseline2_msg->setMinimumWidth( QFontMetrics(lbl_baseline2_msg->font()).averageCharWidth() * 40 );
   lbl_baseline2_msg->setCursor(QCursor(Qt::IBeamCursor));

   pb_baseline2_fit = new QPushButton(us_tr("Fit"), this);
   pb_baseline2_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_baseline2_fit->setMinimumHeight(minHeight1);
   pb_baseline2_fit->setPalette( PALET_PUSHB );
   connect(pb_baseline2_fit, SIGNAL(clicked()), SLOT(baseline2_fit()));

   pb_baseline2_create_adjusted_curve = new QPushButton(us_tr("Create baseline subtracted Curve"), this);
   pb_baseline2_create_adjusted_curve->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_baseline2_create_adjusted_curve->setMinimumHeight(minHeight1);
   pb_baseline2_create_adjusted_curve->setPalette( PALET_PUSHB );
   connect(pb_baseline2_create_adjusted_curve, SIGNAL(clicked()), SLOT(baseline2_create_adjusted_curve()));

   lbl_baseline2_q_range = new QLabel( us_tr( " Fit range: " ), this );
   lbl_baseline2_q_range->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_baseline2_q_range->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_baseline2_q_range );
   lbl_baseline2_q_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_baseline2_q_range->hide();

   le_baseline2_q_start = new mQLineEdit( this );    le_baseline2_q_start->setObjectName( "le_baseline2_q_start Line Edit" );
   le_baseline2_q_start->setText( "" );
   le_baseline2_q_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_baseline2_q_start->setPalette( cg_fit_1 );
   le_baseline2_q_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_baseline2_q_start->setEnabled( false );
   le_baseline2_q_start->setValidator( new QDoubleValidator( le_baseline2_q_start ) );
   le_baseline2_q_start->hide();
   connect( le_baseline2_q_start, SIGNAL( textChanged( const QString & ) ), SLOT( baseline2_q_start_text( const QString & ) ) );
   connect( le_baseline2_q_start, SIGNAL( focussed ( bool ) )             , SLOT( baseline2_q_start_focus( bool ) ) );

   le_baseline2_q_end = new mQLineEdit( this );    le_baseline2_q_end->setObjectName( "le_baseline2_q_end Line Edit" );
   le_baseline2_q_end->setText( "" );
   le_baseline2_q_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_baseline2_q_end->setPalette( cg_fit_1 );
   le_baseline2_q_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_baseline2_q_end->setEnabled( false );
   le_baseline2_q_end->setValidator( new QDoubleValidator( le_baseline2_q_end ) );
   le_baseline2_q_end->hide();
   connect( le_baseline2_q_end, SIGNAL( textChanged( const QString & ) ), SLOT( baseline2_q_end_text( const QString & ) ) );
   connect( le_baseline2_q_end, SIGNAL( focussed ( bool ) )             , SLOT( baseline2_q_end_focus( bool ) ) );


   // baseline2 end

   pb_svd = new QPushButton(us_tr("SVD / EFA"), this);
   pb_svd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_svd->setMinimumHeight(minHeight1);
   pb_svd->setPalette( PALET_PUSHB );
   connect(pb_svd, SIGNAL(clicked()), SLOT(svd()));
   pb_svd->hide();

   pb_create_i_of_t = new QPushButton(us_tr("Make A(t)"), this);
   pb_create_i_of_t->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_create_i_of_t->setMinimumHeight(minHeight1);
   pb_create_i_of_t->setPalette( PALET_PUSHB );
   connect(pb_create_i_of_t, SIGNAL(clicked()), SLOT(create_i_of_t()));

   pb_test_i_of_t = new QPushButton(us_tr("Test I(t)"), this);
   pb_test_i_of_t->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_test_i_of_t->setMinimumHeight(minHeight1);
   pb_test_i_of_t->setPalette( PALET_PUSHB );
   connect(pb_test_i_of_t, SIGNAL(clicked()), SLOT(test_i_of_t()));

   pb_create_i_of_q = new QPushButton(us_tr("Make A(" + UNICODE_LAMBDA_QS + ")"), this);
   pb_create_i_of_q->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_create_i_of_q->setMinimumHeight(minHeight1);
   pb_create_i_of_q->setPalette( PALET_PUSHB );
   connect(pb_create_i_of_q, SIGNAL(clicked()), SLOT(create_i_of_q()));

   pb_create_ihash_t = new QPushButton(us_tr("Make I#(t)"), this);
   pb_create_ihash_t->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_create_ihash_t->setMinimumHeight(minHeight1);
   pb_create_ihash_t->setPalette( PALET_PUSHB );
   connect(pb_create_ihash_t, SIGNAL(clicked()), SLOT(create_ihash_t()));

   pb_create_istar_q = new QPushButton(us_tr("Make I*(q)"), this);
   pb_create_istar_q->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_create_istar_q->setMinimumHeight(minHeight1);
   pb_create_istar_q->setPalette( PALET_PUSHB );
   connect(pb_create_istar_q, SIGNAL(clicked()), SLOT(create_istar_q()));
   
   pb_load_conc = new QPushButton(us_tr("Conc. File Load"), this);
   pb_load_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_load_conc->setMinimumHeight(minHeight1);
   pb_load_conc->setPalette( PALET_PUSHB );
   connect(pb_load_conc, SIGNAL(clicked()), SLOT(load_conc()));

   pb_repeak = new QPushButton(us_tr("Repeak"), this);
   pb_repeak->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_repeak->setMinimumHeight(minHeight1);
   pb_repeak->setPalette( PALET_PUSHB );
   connect(pb_repeak, SIGNAL(clicked()), SLOT(repeak()));

   pb_conc_file = new QPushButton(us_tr("Set"), this);
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

   pb_detector = new QPushButton(us_tr("Detector"), this);
   pb_detector->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_detector->setMinimumHeight(minHeight1);
   pb_detector->setPalette( PALET_PUSHB );
   connect(pb_detector, SIGNAL(clicked()), SLOT(set_detector()));
   pb_detector->hide();

   //    pb_set_dad = new QPushButton(us_tr("Set buffer"), this);
   //    pb_set_dad->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   //    pb_set_dad->setMinimumHeight(minHeight1);
   //    pb_set_dad->setPalette( PALET_PUSHB );
   //    connect(pb_set_dad, SIGNAL(clicked()), SLOT(set_dad()));
   
   //    lbl_dad = new QLabel("", this );
   //    lbl_dad->setMinimumHeight(minHeight1);
   //    lbl_dad->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   //    lbl_dad->setPalette( PALET_NORMAL );
   //    lbl_dad->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   //    pb_set_empty = new QPushButton(us_tr("Set blank"), this);
   //    pb_set_empty->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   //    pb_set_empty->setMinimumHeight(minHeight1);
   //    pb_set_empty->setPalette( PALET_PUSHB );
   //    connect(pb_set_empty, SIGNAL(clicked()), SLOT(set_empty()));

   //    lbl_empty = new QLabel("", this );
   //    lbl_empty->setMinimumHeight(minHeight1);
   //    lbl_empty->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   //    lbl_empty->setPalette( PALET_NORMAL );
   //    lbl_empty->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   //    pb_set_signal = new QPushButton(us_tr("Set solution"), this);
   //    pb_set_signal->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   //    pb_set_signal->setMinimumHeight(minHeight1);
   //    pb_set_signal->setPalette( PALET_PUSHB );
   //    connect(pb_set_signal, SIGNAL(clicked()), SLOT(set_signal()));

   //    lbl_signal = new QLabel("", this );
   //    lbl_signal->setMinimumHeight(minHeight1);
   //    lbl_signal->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   //    lbl_signal->setPalette( PALET_NORMAL );
   //    lbl_signal->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   lbl_created_files = new mQLabel("Produced Data", this);
   lbl_created_files->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_created_files->setMinimumHeight(minHeight1);
   lbl_created_files->setPalette( PALET_LABEL );
   AUTFBACK( lbl_created_files );
   lbl_created_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   connect( lbl_created_files, SIGNAL( pressed() ), SLOT( hide_created_files() ) );

   // lbl_created_dir = new mQLabel( QDir::currentPath(), this );
   // lbl_created_dir->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   // lbl_created_dir->setPalette( PALET_NORMAL );
   // lbl_created_dir->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2));
   // connect( lbl_created_dir, SIGNAL(pressed()), SLOT( created_dir_pressed() ));

   le_created_dir = new mQLineEdit( this );
   le_created_dir->setText( QDir::currentPath() );
   le_created_dir->setFrame( false );
   le_created_dir->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_created_dir->setPalette( PALET_NORMAL );
   AUTFBACK( le_created_dir );
   le_created_dir->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2));
   connect( le_created_dir, SIGNAL(pressed()), SLOT( created_dir_pressed() ));


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

   pb_invert_all_created = new QPushButton(us_tr("Sel. Unsel."), this);
   pb_invert_all_created->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize  - 1));
   pb_invert_all_created->setMinimumHeight(minHeight1);
   pb_invert_all_created->setPalette( PALET_PUSHB );
   connect(pb_invert_all_created, SIGNAL(clicked()), SLOT(invert_all_created()));

   pb_adjacent_created = new QPushButton(us_tr("Similar"), this);
   pb_adjacent_created->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_adjacent_created->setMinimumHeight(minHeight1);
   pb_adjacent_created->setPalette( PALET_PUSHB );
   connect(pb_adjacent_created, SIGNAL(clicked()), SLOT(adjacent_created()));
   pb_adjacent_created->hide();

   pb_remove_created = new QPushButton(us_tr("Remove"), this);
   pb_remove_created->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_remove_created->setMinimumHeight(minHeight1);
   pb_remove_created->setPalette( PALET_PUSHB );
   connect(pb_remove_created, SIGNAL(clicked()), SLOT(remove_created()));

   pb_save_created_csv = new QPushButton( "", this);
   pb_save_created_csv->setText( (( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "dad_csv_transposed" ) &&
                                 (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_csv_transposed" ] == "true" ?
                                 us_tr( "Save CSV Tr" ) : us_tr( " Save CSV " ) );
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

   // models

   lbl_model_files = new QLabel("Model files", this);
   lbl_model_files->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_model_files->setMinimumHeight(minHeight1);
   lbl_model_files->setPalette( PALET_LABEL );
   AUTFBACK( lbl_model_files );
   lbl_model_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   model_widgets.push_back( lbl_model_files );

   lb_model_files = new QListWidget( this );
   lb_model_files->setPalette( PALET_NORMAL );
   AUTFBACK( lb_model_files );
   lb_model_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lb_model_files->setEnabled(true);
   lb_model_files->setSelectionMode( QAbstractItemView::ExtendedSelection );
   lb_model_files->setMinimumHeight( minHeight1 * 2 );
   connect( lb_model_files, SIGNAL( itemSelectionChanged() ), SLOT( update_enables() ) );
   model_widgets.push_back( lb_model_files );

   pb_model_select_all = new mQPushButton(us_tr("Select all"), this);
   pb_model_select_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_model_select_all->setMinimumHeight(minHeight1);
   pb_model_select_all->setPalette( PALET_PUSHB );
   connect(pb_model_select_all, SIGNAL(clicked()), SLOT(model_select_all()));
   model_widgets.push_back( pb_model_select_all );

   pb_model_text = new QPushButton(us_tr("Text"), this);
   pb_model_text->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_model_text->setMinimumHeight(minHeight1);
   pb_model_text->setPalette( PALET_PUSHB );
   connect(pb_model_text, SIGNAL(clicked()), SLOT(model_text()));
   model_widgets.push_back( pb_model_text );

   pb_model_view = new QPushButton(us_tr("View"), this);
   pb_model_view->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_model_view->setMinimumHeight(minHeight1);
   pb_model_view->setPalette( PALET_PUSHB );
   connect(pb_model_view, SIGNAL(clicked()), SLOT(model_view()));
   model_widgets.push_back( pb_model_view );

   pb_model_remove = new QPushButton(us_tr("Remove"), this);
   pb_model_remove->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_model_remove->setMinimumHeight(minHeight1);
   pb_model_remove->setPalette( PALET_PUSHB );
   connect(pb_model_remove, SIGNAL(clicked()), SLOT(model_remove()));
   model_widgets.push_back( pb_model_remove );

   pb_model_save = new QPushButton(us_tr("Save"), this);
   pb_model_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_model_save->setMinimumHeight(minHeight1);
   pb_model_save->setPalette( PALET_PUSHB );
   connect(pb_model_save, SIGNAL(clicked()), SLOT(model_save()));
   model_widgets.push_back( pb_model_save );

   le_dummy = new QLineEdit( this );    le_dummy->setObjectName( "le_dummy Line Edit" );
   le_dummy->setText( "" );
   le_dummy->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_dummy->setPalette( PALET_NORMAL );
   AUTFBACK( le_dummy );
   le_dummy->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 3));
   // le_dummy->setMaximumHeight( 3 );
   le_dummy->setEnabled( false );

   progress = new QProgressBar( this );
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
   editor_widgets.push_back( frame );

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
   editor_widgets.push_back( frame );

   m = new QMenuBar( frame );    m->setObjectName( "menu" );
#endif
   
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
   editor_widgets.push_back( editor );

   qs_plots = new QSplitter( Qt::Vertical, this );

//   plot_dist = new QwtPlot( qs_plots );
   usp_plot_dist = new US_Plot( plot_dist, "", "", "", qs_plots );
   connect( (QWidget *)plot_dist->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_dist( const QPoint & ) ) );
   ((QWidget *)plot_dist->titleLabel())->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_dist->axisWidget( QwtPlot::yLeft ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_dist( const QPoint & ) ) );
   ((QWidget *)plot_dist->axisWidget( QwtPlot::yLeft ))->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_dist->axisWidget( QwtPlot::xBottom ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_dist( const QPoint & ) ) );
   ((QWidget *)plot_dist->axisWidget( QwtPlot::xBottom ))->setContextMenuPolicy( Qt::CustomContextMenu );
   plot_info[ "HPLC SAXS Main" ] = plot_dist;
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
   plot_dist->setAxisTitle(QwtPlot::xBottom, /* cb_guinier->isChecked() ? us_tr("q^2 (1/Angstrom^2)") : */  us_tr("q [1/Angstrom]" )); // or Time or Frame"));
   plot_dist->setAxisTitle(QwtPlot::yLeft, us_tr("Intensity [a.u.] (log scale)"));
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
   {
      QwtLegend* legend_pd = new QwtLegend;
      legend_pd->setFrameStyle( QFrame::Box | QFrame::Sunken );
      plot_dist->insertLegend( legend_pd, QwtPlot::BottomLegend );
   }
#endif

   // plot_dist->canvas()->setCursor(QCursor(QPixmap(":/resources/red_cursor.png")));

   plot_dist_zoomer = new ScrollZoomer(plot_dist->canvas());
   plot_dist_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
   plot_dist_zoomer->setTrackerPen(QPen(Qt::red));
   connect( plot_dist_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_zoomed( const QRectF & ) ) );

//   plot_ref = new QwtPlot( qs_plots );
   usp_plot_ref = new US_Plot( plot_ref, "", "", "", qs_plots );
   connect( (QWidget *)plot_ref->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_ref( const QPoint & ) ) );
   ((QWidget *)plot_ref->titleLabel())->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_ref->axisWidget( QwtPlot::yLeft ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_ref( const QPoint & ) ) );
   ((QWidget *)plot_ref->axisWidget( QwtPlot::yLeft ))->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_ref->axisWidget( QwtPlot::xBottom ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_ref( const QPoint & ) ) );
   ((QWidget *)plot_ref->axisWidget( QwtPlot::xBottom ))->setContextMenuPolicy( Qt::CustomContextMenu );
   plot_info[ "HPLC SAXS Reference" ] = plot_ref;

#if QT_VERSION < 0x040000
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
#if QT_VERSION < 0x040000
   plot_ref->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot_ref->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   grid_ref->setMajorPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid_ref->setMinorPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid_ref->attach( plot_ref );
#endif
   plot_ref->setAxisTitle(QwtPlot::xBottom, us_tr( "Time [a.u.]" ) );
   plot_ref->setAxisTitle(QwtPlot::yLeft, us_tr("Intensity [a.u.]"));
#if QT_VERSION < 0x040000
   plot_ref->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   plot_ref->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_ref->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   plot_ref->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_ref->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   plot_ref->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_ref->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
//    plot_ref->setMargin(USglobal->config_list.margin);
   plot_ref->setTitle("");
#if QT_VERSION < 0x040000
   // plot_ref->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::Logarithmic);
#else
   // plot_ref->setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine(10));
#endif
   plot_ref->setCanvasBackground(USglobal->global_colors.plot);
   plot_ref->hide();
   plot_ref->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding)); 
//   plot_errors = new QwtPlot( this );
   usp_plot_errors = new US_Plot( plot_errors, "", "", "", this );
   connect( (QWidget *)plot_errors->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_errors( const QPoint & ) ) );
   ((QWidget *)plot_errors->titleLabel())->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_errors->axisWidget( QwtPlot::yLeft ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_errors( const QPoint & ) ) );
   ((QWidget *)plot_errors->axisWidget( QwtPlot::yLeft ))->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_errors->axisWidget( QwtPlot::xBottom ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_errors( const QPoint & ) ) );
   ((QWidget *)plot_errors->axisWidget( QwtPlot::xBottom ))->setContextMenuPolicy( Qt::CustomContextMenu );
   plot_info[ "HPLC SAXS Errors" ] = plot_errors;
#if QT_VERSION < 0x040000
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
#if QT_VERSION < 0x040000
   plot_errors->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot_errors->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   grid_errors->setMajorPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid_errors->setMinorPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid_errors->attach( plot_errors );
#endif
   // plot_errors->setAxisTitle(QwtPlot::xBottom, /* cb_guinier->isChecked() ? us_tr("q^2 (1/Angstrom^2)") : */  us_tr("q (1/Angstrom) or Frame"));
   // plot_errors->setAxisTitle(QwtPlot::yLeft, us_tr("I(q) (log scale)"));
#if QT_VERSION < 0x040000
   // plot_errors->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   // plot_errors->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_errors->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   // plot_errors->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_errors->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   // plot_errors->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   // plot_errors->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
//    plot_errors->setMargin(USglobal->config_list.margin);
   plot_errors->setTitle("");
   plot_errors->setCanvasBackground(USglobal->global_colors.plot);

   plot_errors_zoomer = new ScrollZoomer(plot_errors->canvas());
   plot_errors_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
   plot_errors_zoomer->setTrackerPen(QPen(Qt::red));
   plot_errors_zoomer->symmetric_rescale = true;
   connect( plot_errors_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_zoomed( const QRectF & ) ) );

   connect(((QObject*)plot_dist  ->axisWidget(QwtPlot::xBottom)) , SIGNAL(scaleDivChanged () ), usp_plot_errors, SLOT(scaleDivChangedXSlot () ), Qt::UniqueConnection );
   connect(((QObject*)plot_errors->axisWidget(QwtPlot::xBottom)) , SIGNAL(scaleDivChanged () ), usp_plot_dist  , SLOT(scaleDivChangedXSlot () ), Qt::UniqueConnection );

   cb_plot_errors_rev = new QCheckBox(this);
   cb_plot_errors_rev->setText(us_tr("Reverse"));
   cb_plot_errors_rev->setEnabled( true );
   cb_plot_errors_rev->setChecked( false );
   cb_plot_errors_rev->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_plot_errors_rev->setPalette( PALET_NORMAL );
   AUTFBACK( cb_plot_errors_rev );
   connect( cb_plot_errors_rev, SIGNAL( clicked() ), SLOT( set_plot_errors_rev() ) );

   cb_plot_errors_sd = new QCheckBox(this);
   cb_plot_errors_sd->setText(us_tr("Use standard deviations  "));
   cb_plot_errors_sd->setEnabled( true );
   cb_plot_errors_sd->setChecked( false );
   cb_plot_errors_sd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_plot_errors_sd->setPalette( PALET_NORMAL );
   AUTFBACK( cb_plot_errors_sd );
   connect( cb_plot_errors_sd, SIGNAL( clicked() ), SLOT( set_plot_errors_sd() ) );

   cb_plot_errors_pct = new QCheckBox(this);
   cb_plot_errors_pct->setText(us_tr("By percent "));
   cb_plot_errors_pct->setEnabled( true );
   cb_plot_errors_pct->setChecked( false );
   cb_plot_errors_pct->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_plot_errors_pct->setPalette( PALET_NORMAL );
   AUTFBACK( cb_plot_errors_pct );
   connect( cb_plot_errors_pct, SIGNAL( clicked() ), SLOT( set_plot_errors_pct() ) );

   cb_plot_errors_group = new QCheckBox(this);
   cb_plot_errors_group->setText(us_tr("Group"));
   cb_plot_errors_group->setEnabled( true );
   cb_plot_errors_group->setChecked( false );
   cb_plot_errors_group->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_plot_errors_group->setPalette( PALET_NORMAL );
   AUTFBACK( cb_plot_errors_group );
   connect( cb_plot_errors_group, SIGNAL( clicked() ), SLOT( set_plot_errors_group() ) );

   plot_errors->setAxisTitle(QwtPlot::yLeft, us_tr( cb_plot_errors_pct->isChecked() ?
                                                 "%" : "delta I(q)" ) );

   QHBoxLayout * hbl_plot_errors_buttons = new QHBoxLayout(); hbl_plot_errors_buttons->setContentsMargins( 0, 0, 0, 0 ); hbl_plot_errors_buttons->setSpacing( 0 );
   hbl_plot_errors_buttons->addWidget( cb_plot_errors_rev );
   hbl_plot_errors_buttons->addWidget( cb_plot_errors_sd );
   hbl_plot_errors_buttons->addWidget( cb_plot_errors_pct );
   // hbl_plot_errors_buttons->addWidget( cb_plot_errors_group );
   cb_plot_errors_group->hide();
   cb_plot_errors_group->setChecked( true );

   l_plot_errors = new QVBoxLayout( 0 );
   l_plot_errors->addWidget( plot_errors );
   l_plot_errors->addLayout( hbl_plot_errors_buttons );

   plot_errors_widgets.push_back( plot_errors );
   plot_errors_widgets.push_back( cb_plot_errors_rev );
   plot_errors_widgets.push_back( cb_plot_errors_sd );
   plot_errors_widgets.push_back( cb_plot_errors_pct );
   plot_errors_widgets.push_back( cb_plot_errors_group );

   hide_widgets( plot_errors_widgets, true );

   pb_timeshift = new QPushButton(us_tr("Timeshift"), this);
   pb_timeshift->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_timeshift->setMinimumHeight(minHeight1);
   pb_timeshift->setPalette( PALET_PUSHB );
   connect(pb_timeshift, SIGNAL(clicked()), SLOT( timeshift() ));

   pb_timescale = new QPushButton(us_tr("Timescale"), this);
   pb_timescale->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_timescale->setMinimumHeight(minHeight1);
   pb_timescale->setPalette( PALET_PUSHB );
   connect(pb_timescale, SIGNAL(clicked()), SLOT( timescale() ));

   pb_p3d = new QPushButton(us_tr("3D"), this);
   pb_p3d->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_p3d->setMinimumHeight(minHeight1);
   pb_p3d->setPalette( PALET_PUSHB );
   connect(pb_p3d, SIGNAL(clicked()), SLOT(p3d()));
   pb_p3d->hide();

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

   lbl_blank2 = new QLabel( "", this );
   lbl_blank2->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_blank2->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_blank2 );
   lbl_blank2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   lbl_wheel_pos_below = new QLabel( "", this );
   lbl_wheel_pos_below->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_wheel_pos_below->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_wheel_pos_below );
   lbl_wheel_pos_below->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   lbl_wheel_Pcolor = new QLabel( "", this );
   lbl_wheel_Pcolor->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_wheel_Pcolor->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_wheel_Pcolor );
   lbl_wheel_Pcolor->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   qi_green = new QImage( 1, 1, QImage::Format_RGB32 );
   qi_green->setPixel( 0, 0, qRgb( 0, 255, 0 ) );

   qi_yellow = new QImage( 1, 1, QImage::Format_RGB32 );
   qi_yellow->setPixel( 0, 0, qRgb( 255, 255, 0 ) );

   qi_red = new QImage( 1, 1, QImage::Format_RGB32 );
   qi_red->setPixel( 0, 0, qRgb( 255, 0, 0 ) );

   pb_wheel_dec = new QPushButton(us_tr("<"), this);
   pb_wheel_dec->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_wheel_dec->setEnabled( false );
   pb_wheel_dec->setMinimumHeight(minHeight1);
   pb_wheel_dec->setPalette( PALET_PUSHB );
   connect(pb_wheel_dec, SIGNAL(clicked()), SLOT( wheel_dec() ));
   
   qwtw_wheel = new QwtWheel( this );
   // qwtw_wheel->setMass         ( 0.5 );
   // qwtw_wheel->setRange( -1000, 1000); qwtw_wheel->setSingleStep( 1 );
   qwtw_wheel->setMinimumHeight( minHeight1 );
   // qwtw_wheel->setTotalAngle( 3600.0 );
   qwtw_wheel->setEnabled      ( false );
   connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
#if QT_VERSION > 0x050000
   connect( qwtw_wheel, SIGNAL( wheelPressed() ), SLOT( wheel_pressed() ) );
   connect( qwtw_wheel, SIGNAL( wheelReleased() ), SLOT( wheel_released() ) );
#endif

   pb_wheel_inc = new QPushButton(us_tr(">"), this);
   pb_wheel_inc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_wheel_inc->setEnabled( false );
   pb_wheel_inc->setMinimumHeight(minHeight1);
   pb_wheel_inc->setPalette( PALET_PUSHB );
   connect(pb_wheel_inc, SIGNAL(clicked()), SLOT( wheel_inc() ));

   pb_ref = new QPushButton(us_tr("Concentration reference"), this);
   pb_ref->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_ref->setMinimumHeight(minHeight1);
   pb_ref->setPalette( PALET_PUSHB );
   pb_ref->setEnabled(false);
   connect(pb_ref, SIGNAL(clicked()), SLOT(ref()));

   pb_errors = new QPushButton(us_tr("Residuals"), this);
   pb_errors->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_errors->setMinimumHeight(minHeight1);
   pb_errors->setPalette( PALET_PUSHB );
   pb_errors->setEnabled(false);
   connect(pb_errors, SIGNAL(clicked()), SLOT(errors()));

   pb_ggqfit = new QPushButton(us_tr("Global fit by q"), this);
   pb_ggqfit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_ggqfit->setMinimumHeight(minHeight1);
   pb_ggqfit->setPalette( PALET_PUSHB );
   pb_ggqfit->setEnabled(false);
   connect(pb_ggqfit, SIGNAL(clicked()), SLOT(ggqfit()));

   pb_cormap = new QPushButton(us_tr("PVP Analysis"), this);
   pb_cormap->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_cormap->setMinimumHeight(minHeight1);
   pb_cormap->setPalette( PALET_PUSHB );
   pb_cormap->setEnabled( true );
   connect(pb_cormap, SIGNAL(clicked()), SLOT(cormap()));

   pb_pp = new QPushButton(us_tr("Save Plots"), this);
   pb_pp->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_pp->setMinimumHeight(minHeight1);
   pb_pp->setPalette( PALET_PUSHB );
   pb_pp->setEnabled( true );
   connect(pb_pp, SIGNAL(clicked()), SLOT(pp()));

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

   pb_gauss_mode = new QPushButton(us_tr("Gaussian options"), this);
   pb_gauss_mode->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_gauss_mode->setMinimumHeight(minHeight1);
   pb_gauss_mode->setPalette( PALET_PUSHB );
   connect(pb_gauss_mode, SIGNAL(clicked()), SLOT(gauss_mode()));

   pb_gauss_start = new QPushButton(us_tr("Gaussians"), this);
   pb_gauss_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_gauss_start->setMinimumHeight(minHeight1);
   pb_gauss_start->setPalette( PALET_PUSHB );
   connect(pb_gauss_start, SIGNAL(clicked()), SLOT(gauss_start()));

   pb_gauss_clear = new QPushButton(us_tr("Clear"), this);
   pb_gauss_clear->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_gauss_clear->setMinimumHeight(minHeight1);
   pb_gauss_clear->setPalette( PALET_PUSHB );
   pb_gauss_clear->setEnabled( false );
   connect(pb_gauss_clear, SIGNAL(clicked( )), SLOT(gauss_clear( )));

   pb_gauss_new = new QPushButton(us_tr("New"), this);
   pb_gauss_new->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_gauss_new->setMinimumHeight(minHeight1);
   pb_gauss_new->setPalette( PALET_PUSHB );
   pb_gauss_new->setEnabled( false );
   connect(pb_gauss_new, SIGNAL(clicked()), SLOT(gauss_new()));

   pb_gauss_delete = new QPushButton(us_tr("Del"), this);
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


   le_gauss_pos = new mQLineEdit( this );    le_gauss_pos->setObjectName( "le_gauss_pos Line Edit" );
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

   le_gauss_pos_width = new mQLineEdit( this );    le_gauss_pos_width->setObjectName( "le_gauss_pos_width Line Edit" );
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

   le_gauss_pos_height = new mQLineEdit( this );    le_gauss_pos_height->setObjectName( "le_gauss_pos_height Line Edit" );
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

   le_gauss_pos_dist1 = new mQLineEdit( this );    le_gauss_pos_dist1->setObjectName( "le_gauss_pos_dist1 Line Edit" );
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

   le_gauss_pos_dist2 = new mQLineEdit( this );    le_gauss_pos_dist2->setObjectName( "le_gauss_pos_dist2 Line Edit" );
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
   cb_gauss_match_amplitude->setText(us_tr("Match "));
   cb_gauss_match_amplitude->setEnabled( true );
   cb_gauss_match_amplitude->setChecked( false );
   cb_gauss_match_amplitude->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_gauss_match_amplitude->setPalette( PALET_NORMAL );
   AUTFBACK( cb_gauss_match_amplitude );
   connect( cb_gauss_match_amplitude, SIGNAL( clicked() ), SLOT( gauss_match_amplitude() ) );

   cb_sd_weight = new QCheckBox(this);
   cb_sd_weight->setText(us_tr("SD  "));
   cb_sd_weight->setEnabled( true );
   cb_sd_weight->setChecked( false );
   cb_sd_weight->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_sd_weight->setPalette( PALET_NORMAL );
   AUTFBACK( cb_sd_weight );
   connect( cb_sd_weight, SIGNAL( clicked() ), SLOT( set_sd_weight() ) );

   cb_fix_width = new QCheckBox(this);
   cb_fix_width->setText(us_tr("Eq width  "));
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
   cb_fix_dist1->setText(us_tr("Eq dist1  "));
   cb_fix_dist1->setEnabled( U_EXPT );
   cb_fix_dist1->setChecked( true );
   cb_fix_dist1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_fix_dist1->setPalette( PALET_NORMAL );
   AUTFBACK( cb_fix_dist1 );
   connect( cb_fix_dist1, SIGNAL( clicked() ), SLOT( set_fix_dist1() ) );

   cb_fix_dist2 = new QCheckBox(this);
   cb_fix_dist2->setText(us_tr("Eq dist2  "));
   cb_fix_dist2->setEnabled( U_EXPT );
   cb_fix_dist2->setChecked( true );
   cb_fix_dist2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_fix_dist2->setPalette( PALET_NORMAL );
   AUTFBACK( cb_fix_dist2 );
   connect( cb_fix_dist2, SIGNAL( clicked() ), SLOT( set_fix_dist2() ) );

   pb_gauss_fit = new QPushButton(us_tr("Fit"), this);
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

   le_gauss_fit_start = new mQLineEdit( this );    le_gauss_fit_start->setObjectName( "le_gauss_fit_start Line Edit" );
   le_gauss_fit_start->setText( "" );
   le_gauss_fit_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_gauss_fit_start->setPalette( cg_red );
   le_gauss_fit_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_gauss_fit_start->setEnabled( false );
   le_gauss_fit_start->setValidator( new QDoubleValidator( le_gauss_fit_start ) );
   connect( le_gauss_fit_start, SIGNAL( textChanged( const QString & ) ), SLOT( gauss_fit_start_text( const QString & ) ) );
   connect( le_gauss_fit_start, SIGNAL( focussed ( bool ) )             , SLOT( gauss_fit_start_focus( bool ) ) );

   le_gauss_fit_end = new mQLineEdit( this );    le_gauss_fit_end->setObjectName( "le_gauss_fit_end Line Edit" );
   le_gauss_fit_end->setText( "" );
   le_gauss_fit_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_gauss_fit_end->setPalette( cg_red );
   le_gauss_fit_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_gauss_fit_end->setEnabled( false );
   le_gauss_fit_end->setValidator( new QDoubleValidator( le_gauss_fit_end ) );
   connect( le_gauss_fit_end, SIGNAL( textChanged( const QString & ) ), SLOT( gauss_fit_end_text( const QString & ) ) );
   connect( le_gauss_fit_end, SIGNAL( focussed ( bool ) )             , SLOT( gauss_fit_end_focus( bool ) ) );

   pb_gauss_save = new QPushButton(us_tr("Save"), this);
   pb_gauss_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_gauss_save->setMinimumHeight(minHeight1);
   pb_gauss_save->setPalette( PALET_PUSHB );
   pb_gauss_save->setEnabled( false );
   connect(pb_gauss_save, SIGNAL(clicked()), SLOT(gauss_save()));

   le_gauss_local_pts = new mQLineEdit( this );    le_gauss_local_pts->setObjectName( "le_gauss_local_pts Line Edit" );
   le_gauss_local_pts->setText( "" );
   le_gauss_local_pts->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_gauss_local_pts->setPalette( PALET_NORMAL );
   AUTFBACK( le_gauss_local_pts );
   le_gauss_local_pts->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_gauss_local_pts->setEnabled( false );
   {
      QValidator *qiv = new QIntValidator( 3, 100, this );
      le_gauss_local_pts->setValidator( qiv );
   }
   le_gauss_local_pts->setToolTip( us_tr( "Number of points for Gaussian peak fit" ) );
   
   pb_gauss_local_caruanas = new QPushButton(us_tr("Caruanas"), this);
   pb_gauss_local_caruanas->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_gauss_local_caruanas->setMinimumHeight(minHeight1);
   pb_gauss_local_caruanas->setPalette( PALET_PUSHB );
   pb_gauss_local_caruanas->setToolTip( us_tr( "Gaussian peak fit by Caruanas method" ) );
   connect(pb_gauss_local_caruanas, SIGNAL(clicked()), SLOT(gauss_local_caruanas()));

   pb_gauss_local_guos = new QPushButton(us_tr("Guos"), this);
   pb_gauss_local_guos->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_gauss_local_guos->setMinimumHeight(minHeight1);
   pb_gauss_local_guos->setPalette( PALET_PUSHB );
   pb_gauss_local_guos->setToolTip( us_tr( "Gaussian peak fit by Guos method" ) );
   connect(pb_gauss_local_guos, SIGNAL(clicked()), SLOT(gauss_local_guos()));

#if !defined( ALLOW_GUOS_CARUANAS )
   le_gauss_local_pts     ->hide();
   pb_gauss_local_caruanas->hide();
   pb_gauss_local_guos    ->hide();
#endif

   pb_ggauss_start = new QPushButton(us_tr("Global Gaussians"), this);
   pb_ggauss_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_ggauss_start->setMinimumHeight(minHeight1);
   pb_ggauss_start->setPalette( PALET_PUSHB );
   connect(pb_ggauss_start, SIGNAL(clicked()), SLOT(ggauss_start()));

   pb_ggauss_rmsd = new QPushButton(us_tr("Recompute RMSD"), this);
   pb_ggauss_rmsd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_ggauss_rmsd->setMinimumHeight(minHeight1);
   pb_ggauss_rmsd->setPalette( PALET_PUSHB );
   pb_ggauss_rmsd->setEnabled( false );
   connect(pb_ggauss_rmsd, SIGNAL(clicked()), SLOT(ggauss_rmsd()));

   pb_ggauss_results = new QPushButton(us_tr("Make result curves"), this);
   pb_ggauss_results->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_ggauss_results->setMinimumHeight(minHeight1);
   pb_ggauss_results->setPalette( PALET_PUSHB );
   pb_ggauss_results->setEnabled( false );
   connect(pb_ggauss_results, SIGNAL(clicked()), SLOT(ggauss_results()));
   // pb_ggauss_results->hide();

   pb_gauss_as_curves = new QPushButton(us_tr("To produced data"), this);
   pb_gauss_as_curves->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_gauss_as_curves->setMinimumHeight(minHeight1);
   pb_gauss_as_curves->setPalette( PALET_PUSHB );
   pb_gauss_as_curves->setEnabled( false );
   connect(pb_gauss_as_curves, SIGNAL(clicked()), SLOT(gauss_as_curves()));

   pb_ggauss_as_curves = new QPushButton(us_tr("To produced data"), this);
   pb_ggauss_as_curves->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_ggauss_as_curves->setMinimumHeight(minHeight1);
   pb_ggauss_as_curves->setPalette( PALET_PUSHB );
   pb_ggauss_as_curves->setEnabled( false );
   connect(pb_ggauss_as_curves, SIGNAL(clicked()), SLOT(gauss_as_curves()));

   // wyatt

   pb_wyatt_start = new QPushButton(us_tr("SD eval."), this);
   pb_wyatt_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_wyatt_start->setMinimumHeight(minHeight1);
   pb_wyatt_start->setPalette( PALET_PUSHB );
   connect(pb_wyatt_start, SIGNAL(clicked()), SLOT(wyatt_start()));

   pb_wyatt_apply = new QPushButton(us_tr("SD apply"), this);
   pb_wyatt_apply->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_wyatt_apply->setMinimumHeight(minHeight1);
   pb_wyatt_apply->setPalette( PALET_PUSHB );
   connect(pb_wyatt_apply, SIGNAL(clicked()), SLOT(wyatt_apply()));

   le_wyatt_start = new mQLineEdit( this );    le_wyatt_start->setObjectName( "le_wyatt_start Line Edit" );
   le_wyatt_start->setText( "" );
   le_wyatt_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_wyatt_start->setPalette( cg_red );
   le_wyatt_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_wyatt_start->setEnabled( false );
   le_wyatt_start->setValidator( new QDoubleValidator( le_wyatt_start ) );
   connect( le_wyatt_start, SIGNAL( textChanged( const QString & ) ), SLOT( wyatt_start_text( const QString & ) ) );
   connect( le_wyatt_start, SIGNAL( focussed ( bool ) )             , SLOT( wyatt_start_focus( bool ) ) );

   le_wyatt_end = new mQLineEdit( this );    le_wyatt_end->setObjectName( "le_wyatt_end Line Edit" );
   le_wyatt_end->setText( "" );
   le_wyatt_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_wyatt_end->setPalette( cg_red );
   le_wyatt_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_wyatt_end->setEnabled( false );
   le_wyatt_end->setValidator( new QDoubleValidator( le_wyatt_end ) );
   connect( le_wyatt_end, SIGNAL( textChanged( const QString & ) ), SLOT( wyatt_end_text( const QString & ) ) );
   connect( le_wyatt_end, SIGNAL( focussed ( bool ) )             , SLOT( wyatt_end_focus( bool ) ) );

   le_wyatt_start2 = new mQLineEdit( this );    le_wyatt_start2->setObjectName( "le_wyatt_start2 Line Edit" );
   le_wyatt_start2->setText( "" );
   le_wyatt_start2->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_wyatt_start2->setPalette( cg_magenta );
   le_wyatt_start2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_wyatt_start2->setEnabled( false );
   le_wyatt_start2->setValidator( new QDoubleValidator( le_wyatt_start2 ) );
   connect( le_wyatt_start2, SIGNAL( textChanged( const QString & ) ), SLOT( wyatt_start2_text( const QString & ) ) );
   connect( le_wyatt_start2, SIGNAL( focussed ( bool ) )             , SLOT( wyatt_start2_focus( bool ) ) );

   le_wyatt_end2 = new mQLineEdit( this );    le_wyatt_end2->setObjectName( "le_wyatt_end2 Line Edit" );
   le_wyatt_end2->setText( "" );
   le_wyatt_end2->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_wyatt_end2->setPalette( cg_magenta );
   le_wyatt_end2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_wyatt_end2->setEnabled( false );
   le_wyatt_end2->setValidator( new QDoubleValidator( le_wyatt_end2 ) );
   connect( le_wyatt_end2, SIGNAL( textChanged( const QString & ) ), SLOT( wyatt_end2_text( const QString & ) ) );
   connect( le_wyatt_end2, SIGNAL( focussed ( bool ) )             , SLOT( wyatt_end2_focus( bool ) ) );

   cb_wyatt_2 = new QCheckBox(this);
   cb_wyatt_2->setText(us_tr("2 regions "));
   cb_wyatt_2->setChecked( false );
   cb_wyatt_2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_wyatt_2->setPalette( PALET_NORMAL );
   AUTFBACK( cb_wyatt_2 );
   connect( cb_wyatt_2, SIGNAL( clicked() ), SLOT( wyatt_2() ) );

   // blanks

   pb_blanks_start = new QPushButton(us_tr("Blanks analysis"), this);
   pb_blanks_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_blanks_start->setMinimumHeight(minHeight1);
   pb_blanks_start->setPalette( PALET_PUSHB );
   connect(pb_blanks_start, SIGNAL(clicked()), SLOT(blanks_start()));

   pb_blanks_params = new QPushButton(us_tr("Analyze"), this);
   pb_blanks_params->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_blanks_params->setMinimumHeight(minHeight1);
   pb_blanks_params->setPalette( PALET_PUSHB );
   connect(pb_blanks_params, SIGNAL(clicked()), SLOT(blanks_params()));
   pb_blanks_params->hide();

   // blanks & baseline auto increment

   pb_bb_cm_inc = new QPushButton(us_tr("Auto inc+coremap"), this);
   pb_bb_cm_inc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_bb_cm_inc->setMinimumHeight(minHeight1);
   pb_bb_cm_inc->setPalette( PALET_PUSHB );
   connect(pb_bb_cm_inc, SIGNAL(clicked()), SLOT(bb_cm_inc()));
   pb_bb_cm_inc->hide();

   // baseline

   pb_baseline_start = new QPushButton(us_tr("Baseline"), this);
   pb_baseline_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_baseline_start->setMinimumHeight(minHeight1);
   pb_baseline_start->setPalette( PALET_PUSHB );
   connect(pb_baseline_start, SIGNAL(clicked()), SLOT(baseline_start()));

   pb_baseline_apply = new QPushButton(us_tr("Baseline apply"), this);
   pb_baseline_apply->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_baseline_apply->setMinimumHeight(minHeight1);
   pb_baseline_apply->setPalette( PALET_PUSHB );
   connect(pb_baseline_apply, SIGNAL(clicked()), SLOT(baseline_apply()));

   pb_baseline_test = new QPushButton(us_tr("Test baseline"), this);
   pb_baseline_test->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_baseline_test->setMinimumHeight(minHeight1);
   pb_baseline_test->setPalette( PALET_PUSHB );
   connect(pb_baseline_test, SIGNAL(clicked()), SLOT(baseline_test()));

   cb_baseline_start_zero = new QCheckBox(this);
   cb_baseline_start_zero->setText(us_tr("Zero base  "));
   cb_baseline_start_zero->setChecked( true );
   cb_baseline_start_zero->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_baseline_start_zero->setPalette( PALET_NORMAL );
   AUTFBACK( cb_baseline_start_zero );
   connect( cb_baseline_start_zero, SIGNAL( clicked() ), SLOT( set_baseline_start_zero() ) );

   le_baseline_start_s = new mQLineEdit( this );    le_baseline_start_s->setObjectName( "le_baseline_start_s Line Edit" );
   le_baseline_start_s->setText( "" );
   le_baseline_start_s->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_baseline_start_s->setPalette( cg_magenta );
   le_baseline_start_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_baseline_start_s->setEnabled( false );
   le_baseline_start_s->setValidator( new QDoubleValidator( le_baseline_start_s ) );
   connect( le_baseline_start_s, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_start_s_text( const QString & ) ) );
   connect( le_baseline_start_s, SIGNAL( focussed ( bool ) )             , SLOT( baseline_start_s_focus( bool ) ) );

   le_baseline_start = new mQLineEdit( this );    le_baseline_start->setObjectName( "le_baseline_start Line Edit" );
   le_baseline_start->setText( "" );
   le_baseline_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_baseline_start->setPalette( cg_red );
   le_baseline_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_baseline_start->setEnabled( false );
   le_baseline_start->setValidator( new QDoubleValidator( le_baseline_start ) );
   connect( le_baseline_start, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_start_text( const QString & ) ) );
   connect( le_baseline_start, SIGNAL( focussed ( bool ) )             , SLOT( baseline_start_focus( bool ) ) );

   le_baseline_start_e = new mQLineEdit( this );    le_baseline_start_e->setObjectName( "le_baseline_start_e Line Edit" );
   le_baseline_start_e->setText( "" );
   le_baseline_start_e->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_baseline_start_e->setPalette( cg_magenta );
   le_baseline_start_e->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_baseline_start_e->setEnabled( false );
   le_baseline_start_e->setValidator( new QDoubleValidator( le_baseline_start_e ) );
   connect( le_baseline_start_e, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_start_e_text( const QString & ) ) );
   connect( le_baseline_start_e, SIGNAL( focussed ( bool ) )             , SLOT( baseline_start_e_focus( bool ) ) );

   le_baseline_end_s = new mQLineEdit( this );    le_baseline_end_s->setObjectName( "le_baseline_end_s Line Edit" );
   le_baseline_end_s->setText( "" );
   le_baseline_end_s->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_baseline_end_s->setPalette( cg_magenta );
   le_baseline_end_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_baseline_end_s->setEnabled( false );
   le_baseline_end_s->setValidator( new QDoubleValidator( le_baseline_end_s ) );
   connect( le_baseline_end_s, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_s_text( const QString & ) ) );
   connect( le_baseline_end_s, SIGNAL( focussed ( bool ) )             , SLOT( baseline_end_s_focus( bool ) ) );

   le_baseline_end = new mQLineEdit( this );    le_baseline_end->setObjectName( "le_baseline_end Line Edit" );
   le_baseline_end->setText( "" );
   le_baseline_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_baseline_end->setPalette( cg_red );
   le_baseline_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_baseline_end->setEnabled( false );
   le_baseline_end->setValidator( new QDoubleValidator( le_baseline_end ) );
   connect( le_baseline_end, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_text( const QString & ) ) );
   connect( le_baseline_end, SIGNAL( focussed ( bool ) )             , SLOT( baseline_end_focus( bool ) ) );

   le_baseline_end_e = new mQLineEdit( this );    le_baseline_end_e->setObjectName( "le_baseline_end_e Line Edit" );
   le_baseline_end_e->setText( "" );
   le_baseline_end_e->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_baseline_end_e->setPalette( cg_magenta );
   le_baseline_end_e->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_baseline_end_e->setEnabled( false );
   le_baseline_end_e->setValidator( new QDoubleValidator( le_baseline_end_e ) );
   connect( le_baseline_end_e, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_e_text( const QString & ) ) );
   connect( le_baseline_end_e, SIGNAL( focussed ( bool ) )             , SLOT( baseline_end_e_focus( bool ) ) );

   cb_baseline_fix_width = new QCheckBox(this);
   cb_baseline_fix_width->setText(us_tr(" Fix window width: "));
   cb_baseline_fix_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_baseline_fix_width->setPalette( PALET_NORMAL );
   AUTFBACK( cb_baseline_fix_width );
   connect( cb_baseline_fix_width, SIGNAL( clicked() ), SLOT( set_baseline_fix_width() ) );

   le_baseline_width = new mQLineEdit( this );    le_baseline_width->setObjectName( "le_baseline_width Line Edit" );
   le_baseline_width->setText( "" );
   le_baseline_width->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_baseline_width->setPalette( cg_magenta );
   le_baseline_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_baseline_width->setEnabled( false );
   le_baseline_width->setValidator( new QIntValidator( le_baseline_width ) );
   connect( le_baseline_width, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_width_text( const QString & ) ) );
   connect( le_baseline_width, SIGNAL( focussed ( bool ) )             , SLOT( baseline_width_focus( bool ) ) );

   pb_baseline_best = new QPushButton(us_tr("Find best region"), this);
   pb_baseline_best->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_baseline_best->setMinimumHeight(minHeight1);
   pb_baseline_best->setPalette( PALET_PUSHB );
   connect(pb_baseline_best, SIGNAL(clicked()), SLOT(baseline_best()));

   // select

   pb_select_vis = new QPushButton(us_tr("Select Visible"), this);
   pb_select_vis->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_select_vis->setMinimumHeight(minHeight1);
   pb_select_vis->setPalette( PALET_PUSHB );
   connect(pb_select_vis, SIGNAL(clicked()), SLOT(select_vis()));

   pb_remove_vis = new QPushButton(us_tr("Remove Vis"), this);
   pb_remove_vis->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_remove_vis->setMinimumHeight(minHeight1);
   pb_remove_vis->setPalette( PALET_PUSHB );
   connect(pb_remove_vis, SIGNAL(clicked()), SLOT(remove_vis()));

   pb_crop_common = new QPushButton(us_tr("Crop Common"), this);
   pb_crop_common->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_crop_common->setMinimumHeight(minHeight1);
   pb_crop_common->setPalette( PALET_PUSHB );
   connect(pb_crop_common, SIGNAL(clicked()), SLOT(crop_common()));

   pb_crop_vis = new QPushButton(us_tr("Crop Vis"), this);
   pb_crop_vis->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_crop_vis->setMinimumHeight(minHeight1);
   pb_crop_vis->setPalette( PALET_PUSHB );
   connect(pb_crop_vis, SIGNAL(clicked()), SLOT(crop_vis()));

   pb_crop_to_vis = new QPushButton(us_tr("Crop to Vis"), this);
   pb_crop_to_vis->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_crop_to_vis->setMinimumHeight(minHeight1);
   pb_crop_to_vis->setPalette( PALET_PUSHB );
   connect(pb_crop_to_vis, SIGNAL(clicked()), SLOT(crop_to_vis()));

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

   pb_axis_x = new QPushButton(us_tr("Log X"), this);
   pb_axis_x->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_axis_x->setMinimumHeight(minHeight1);
   pb_axis_x->setPalette( PALET_PUSHB );
   connect(pb_axis_x, SIGNAL(clicked()), SLOT(axis_x()));

   pb_axis_y = new QPushButton(us_tr("Log Y"), this);
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

//   ggqfit_plot = new QwtPlot( this );
   usp_ggqfit_plot = new US_Plot( ggqfit_plot, "", "", "", this );
   connect( (QWidget *)ggqfit_plot->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_ggqfit_plot( const QPoint & ) ) );
   ((QWidget *)ggqfit_plot->titleLabel())->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)ggqfit_plot->axisWidget( QwtPlot::yLeft ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_ggqfit_plot( const QPoint & ) ) );
   ((QWidget *)ggqfit_plot->axisWidget( QwtPlot::yLeft ))->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)ggqfit_plot->axisWidget( QwtPlot::xBottom ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_ggqfit_plot( const QPoint & ) ) );
   ((QWidget *)ggqfit_plot->axisWidget( QwtPlot::xBottom ))->setContextMenuPolicy( Qt::CustomContextMenu );
   plot_info[ "HPLC SAXS Global Gaussian Fit By q" ] = ggqfit_plot;
#if QT_VERSION < 0x040000
   ggqfit_plot->enableGridXMin();
   ggqfit_plot->enableGridYMin();
#else
   ggqfit_plot_grid = new QwtPlotGrid;
   ggqfit_plot_grid->enableXMin( true );
   ggqfit_plot_grid->enableYMin( true );
#endif
   ggqfit_plot->setPalette( PALET_NORMAL );
   AUTFBACK( ggqfit_plot );
#if QT_VERSION < 0x040000
   ggqfit_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   ggqfit_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   ggqfit_plot_grid->setMajorPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   ggqfit_plot_grid->setMinorPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   ggqfit_plot_grid->attach( ggqfit_plot );
#endif
   ggqfit_plot->setAxisTitle(QwtPlot::xBottom, us_tr("q (1/Angstrom)"));
   ggqfit_plot->setAxisTitle(QwtPlot::yLeft, us_tr( "Chi^2 or RMSD" ) );
#if QT_VERSION < 0x040000
   ggqfit_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   ggqfit_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   ggqfit_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   ggqfit_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   ggqfit_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   ggqfit_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   ggqfit_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
//    ggqfit_plot->setMargin(USglobal->config_list.margin);
   ggqfit_plot->setTitle("");
#if QT_VERSION < 0x040000
   ggqfit_plot->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::None);
#else
   ggqfit_plot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine );
#endif
   ggqfit_plot->setCanvasBackground(USglobal->global_colors.plot);

#if QT_VERSION < 0x040000
   ggqfit_plot->setAutoLegend( false );
   ggqfit_plot->setLegendFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ) );
#else
   // {
   //    QwtLegend* legend_pd = new QwtLegend;
   //    legend_pd->setFrameStyle( QFrame::Box | QFrame::Sunken );
   //    ggqfit_plot->insertLegend( legend_pd, QwtPlot::BottomLegend );
   // }
#endif
   //   connect( ggqfit_plot->canvas(), SIGNAL( mouseReleased( const QMouseEvent & ) ), SLOT( plot_mouse(  const QMouseEvent & ) ) );

   ggqfit_plot->enableAxis    ( QwtPlot::yRight , true );
#if QT_VERSION < 0x040000
   ggqfit_plot->setAxisOptions( QwtPlot::yRight, QwtAutoScale::Logarithmic );
#else
   ggqfit_plot->setAxisScaleEngine( QwtPlot::yRight, new QwtLogScaleEngine(10) );
#endif

   cb_ggq_plot_chi2 = new QCheckBox(this);
   cb_ggq_plot_chi2->setText( us_tr("Plot Chi^2/RMSD" ) );
   cb_ggq_plot_chi2->setEnabled( true );
   cb_ggq_plot_chi2->setChecked( true );
   cb_ggq_plot_chi2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_ggq_plot_chi2->setPalette( PALET_NORMAL );
   AUTFBACK( cb_ggq_plot_chi2 );
   connect( cb_ggq_plot_chi2, SIGNAL( clicked() ), SLOT( gg_fit_replot() ) );

   cb_ggq_plot_P = new QCheckBox( this );
   cb_ggq_plot_P->setText( us_tr("Plot P values" ) );
   cb_ggq_plot_P->setEnabled( true );
   cb_ggq_plot_P->setChecked( true );
   cb_ggq_plot_P->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_ggq_plot_P->setPalette( PALET_NORMAL );
   AUTFBACK( cb_ggq_plot_P );
   connect( cb_ggq_plot_P, SIGNAL( clicked() ), SLOT( gg_fit_replot() ) );

   // ggauss scroll

   cb_ggauss_scroll = new QCheckBox(this);
   cb_ggauss_scroll->setText( us_tr("Scroll" ) );
   cb_ggauss_scroll->setEnabled( true );
   cb_ggauss_scroll->setChecked( false );
   cb_ggauss_scroll->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_ggauss_scroll->setPalette( PALET_NORMAL );
   AUTFBACK( cb_ggauss_scroll );
   connect( cb_ggauss_scroll, SIGNAL( clicked() ), SLOT( ggauss_scroll() ) );

   {
      double alpha        = (( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "alpha" ) ? (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "alpha" ].toDouble() : 0.05;
      double alpha_over_5 = 0.2 * alpha;

      cb_ggauss_scroll_p_green = new QCheckBox(this);
      cb_ggauss_scroll_p_green->setText( QString( us_tr("P >= %1 " ) ).arg( alpha ) );
      cb_ggauss_scroll_p_green->setEnabled( false );
      cb_ggauss_scroll_p_green->setChecked( true );
      cb_ggauss_scroll_p_green->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
      cb_ggauss_scroll_p_green->setPalette( PALET_NORMAL );
      AUTFBACK( cb_ggauss_scroll_p_green );
      connect( cb_ggauss_scroll_p_green, SIGNAL( clicked() ), SLOT( ggauss_scroll_p_green() ) );

      cb_ggauss_scroll_p_yellow = new QCheckBox(this);
      cb_ggauss_scroll_p_yellow->setText( QString( us_tr("%1 > P >= %2 " ) ).arg( alpha ).arg( alpha_over_5 ) );
      cb_ggauss_scroll_p_yellow->setEnabled( false );
      cb_ggauss_scroll_p_yellow->setChecked( true );
      cb_ggauss_scroll_p_yellow->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
      cb_ggauss_scroll_p_yellow->setPalette( PALET_NORMAL );
      AUTFBACK( cb_ggauss_scroll_p_yellow );
      connect( cb_ggauss_scroll_p_yellow, SIGNAL( clicked() ), SLOT( ggauss_scroll_p_yellow() ) );

      cb_ggauss_scroll_p_red = new QCheckBox(this);
      cb_ggauss_scroll_p_red->setText( QString( us_tr( "P < %1 " ) ).arg( alpha_over_5 ) );
      cb_ggauss_scroll_p_red->setEnabled( false );
      cb_ggauss_scroll_p_red->setChecked( true );
      cb_ggauss_scroll_p_red->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
      cb_ggauss_scroll_p_red->setPalette( PALET_NORMAL );
      AUTFBACK( cb_ggauss_scroll_p_red );
      connect( cb_ggauss_scroll_p_red, SIGNAL( clicked() ), SLOT( ggauss_scroll_p_red() ) );

      cb_ggauss_scroll_smoothed = new QCheckBox(this);
      cb_ggauss_scroll_smoothed->setText( us_tr( "Smoothed" ) );
      cb_ggauss_scroll_smoothed->setEnabled( false );
      cb_ggauss_scroll_smoothed->setChecked( false );
      cb_ggauss_scroll_smoothed->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
      cb_ggauss_scroll_smoothed->setPalette( PALET_NORMAL );
      AUTFBACK( cb_ggauss_scroll_smoothed );
      connect( cb_ggauss_scroll_smoothed, SIGNAL( clicked() ), SLOT( ggauss_scroll_smoothed() ) );

      cb_ggauss_scroll_oldstyle = new QCheckBox(this);
      cb_ggauss_scroll_oldstyle->setText( us_tr( "Raw fit" ) );
      cb_ggauss_scroll_oldstyle->setEnabled( false );
      cb_ggauss_scroll_oldstyle->setChecked( false );
      cb_ggauss_scroll_oldstyle->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
      cb_ggauss_scroll_oldstyle->setPalette( PALET_NORMAL );
      AUTFBACK( cb_ggauss_scroll_oldstyle );
      connect( cb_ggauss_scroll_oldstyle, SIGNAL( clicked() ), SLOT( ggauss_scroll_oldstyle() ) );
   }
   // scale mode

   pb_scale = new QPushButton(us_tr("Scale Analysis"), this);
   pb_scale->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_scale->setMinimumHeight(minHeight1);
   pb_scale->setPalette( PALET_PUSHB );
   connect(pb_scale, SIGNAL(clicked()), SLOT(scale()));
   pb_scale->setEnabled( false );

   lbl_scale_low_high = new QLabel( us_tr( "Scale to: " ), this );
   lbl_scale_low_high->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_scale_low_high->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_scale_low_high );
   lbl_scale_low_high->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   rb_scale_low =  new QRadioButton( us_tr( "Minimum " ), this );
   rb_scale_low -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_scale_low );
   rb_scale_low -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_scale_low, SIGNAL( clicked() ), SLOT( scale_enables() ) );
                                    
   rb_scale_high =  new QRadioButton( us_tr( "Maximum " ), this );
   rb_scale_high -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_scale_high );
   rb_scale_high -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_scale_high, SIGNAL( clicked() ), SLOT( scale_enables() ) );

#if 1 // QT_VERSION < 0x040000
   bg_scale_low_high = new QButtonGroup( this );
   int bg_pos = 0;
   bg_scale_low_high->setExclusive(true);
   bg_scale_low_high->addButton( rb_scale_low, bg_pos++ );
   bg_scale_low_high->addButton( rb_scale_high, bg_pos++ );
#else
   bg_scale_low_high = new QGroupBox();
   bg_scale_low_high->setFlat( true );

   {
      QVBoxLayout * bl = new QVBoxLayout; bl->setContentsMargins( 0, 0, 0, 0 ); bl->setSpacing( 0 );
      bl->addWidget( rb_scale_low );
      bl->addWidget( rb_scale_high );
      bg_scale_low_high->setLayout( bl );
   }
#endif

   rb_scale_low->setChecked( true );
   
   cb_scale_scroll = new QCheckBox(this);
   cb_scale_scroll->setText(us_tr("Scroll "));
   cb_scale_scroll->setChecked( false );
   cb_scale_scroll->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_scale_scroll->setPalette( PALET_NORMAL );
   AUTFBACK( cb_scale_scroll );
   connect( cb_scale_scroll, SIGNAL( clicked() ), SLOT( scale_scroll() ) );
   cb_scale_scroll->hide();

   cb_scale_sd = new QCheckBox(this);
   cb_scale_sd->setText(us_tr("SD "));
   cb_scale_sd->setChecked( true );
   cb_scale_sd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_scale_sd->setPalette( PALET_NORMAL );
   AUTFBACK( cb_scale_sd );
   connect( cb_scale_sd, SIGNAL( clicked() ), SLOT( scale_enables() ) );
   cb_scale_sd->hide();

   cb_scale_save_intp = new QCheckBox(this);
   cb_scale_save_intp->setText(us_tr("Save interpolated to target"));
   cb_scale_save_intp->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_scale_save_intp->setPalette( PALET_NORMAL );
   AUTFBACK( cb_scale_save_intp );
   connect( cb_scale_save_intp, SIGNAL( clicked() ), SLOT( scale_enables() ) );

   lbl_scale_q_range = new QLabel( us_tr( "q range for scaling: " ), this );
   lbl_scale_q_range->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_scale_q_range->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_scale_q_range );
   lbl_scale_q_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_scale_q_start = new mQLineEdit( this );    le_scale_q_start->setObjectName( "le_scale_q_start Line Edit" );
   le_scale_q_start->setText( "" );
   le_scale_q_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_scale_q_start->setPalette( cg_red );
   le_scale_q_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_scale_q_start->setEnabled( false );
   le_scale_q_start->setValidator( new QDoubleValidator( le_scale_q_start ) );
   connect( le_scale_q_start, SIGNAL( textChanged( const QString & ) ), SLOT( scale_q_start_text( const QString & ) ) );
   connect( le_scale_q_start, SIGNAL( focussed ( bool ) )             , SLOT( scale_q_start_focus( bool ) ) );

   le_scale_q_end = new mQLineEdit( this );    le_scale_q_end->setObjectName( "le_scale_q_end Line Edit" );
   le_scale_q_end->setText( "" );
   le_scale_q_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_scale_q_end->setPalette( cg_red );
   le_scale_q_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_scale_q_end->setEnabled( false );
   le_scale_q_end->setValidator( new QDoubleValidator( le_scale_q_end ) );
   connect( le_scale_q_end, SIGNAL( textChanged( const QString & ) ), SLOT( scale_q_end_text( const QString & ) ) );
   connect( le_scale_q_end, SIGNAL( focussed ( bool ) )             , SLOT( scale_q_end_focus( bool ) ) );

   pb_scale_q_reset = new QPushButton(us_tr("Reset q range"), this);
   pb_scale_q_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_scale_q_reset->setMinimumHeight(minHeight1);
   pb_scale_q_reset->setPalette( PALET_PUSHB );
   connect(pb_scale_q_reset, SIGNAL(clicked()), SLOT(scale_q_reset()));
   pb_scale_q_reset->setEnabled( false );

   pb_scale_reset = new QPushButton(us_tr("Reset scaling"), this);
   pb_scale_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_scale_reset->setMinimumHeight(minHeight1);
   pb_scale_reset->setPalette( PALET_PUSHB );
   connect(pb_scale_reset, SIGNAL(clicked()), SLOT(scale_reset()));
   pb_scale_reset->setEnabled( false );

   pb_scale_apply = new QPushButton(us_tr("Apply"), this);
   pb_scale_apply->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_scale_apply->setMinimumHeight(minHeight1);
   pb_scale_apply->setPalette( PALET_PUSHB );
   connect(pb_scale_apply, SIGNAL(clicked()), SLOT(scale_apply()));
   pb_scale_apply->setEnabled( false );

   pb_scale_create = new QPushButton(us_tr("Create scaled set"), this);
   pb_scale_create->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_scale_create->setMinimumHeight(minHeight1);
   pb_scale_create->setPalette( PALET_PUSHB );
   connect(pb_scale_create, SIGNAL(clicked()), SLOT(scale_create()));
   pb_scale_create->setEnabled( false );

   // testiq mode

   pb_testiq = new QPushButton(us_tr("Trial make I(q)"), this);
   pb_testiq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_testiq->setMinimumHeight(minHeight1);
   pb_testiq->setPalette( PALET_PUSHB );
   connect(pb_testiq, SIGNAL(clicked()), SLOT(testiq()));
   pb_testiq->setEnabled( false );

   lbl_testiq_q_range = new QLabel( us_tr( "Time range for I(q): " ), this );
   lbl_testiq_q_range->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_testiq_q_range->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_testiq_q_range );
   lbl_testiq_q_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_testiq_q_start = new mQLineEdit( this );    le_testiq_q_start->setObjectName( "le_testiq_q_start Line Edit" );
   le_testiq_q_start->setText( "" );
   le_testiq_q_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_testiq_q_start->setPalette( cg_red );
   le_testiq_q_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_testiq_q_start->setEnabled( false );
   le_testiq_q_start->setValidator( new QDoubleValidator( le_testiq_q_start ) );
   connect( le_testiq_q_start, SIGNAL( textChanged( const QString & ) ), SLOT( testiq_q_start_text( const QString & ) ) );
   connect( le_testiq_q_start, SIGNAL( focussed ( bool ) )             , SLOT( testiq_q_start_focus( bool ) ) );

   le_testiq_q_end = new mQLineEdit( this );    le_testiq_q_end->setObjectName( "le_testiq_q_end Line Edit" );
   le_testiq_q_end->setText( "" );
   le_testiq_q_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_testiq_q_end->setPalette( cg_red );
   le_testiq_q_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_testiq_q_end->setEnabled( false );
   le_testiq_q_end->setValidator( new QDoubleValidator( le_testiq_q_end ) );
   connect( le_testiq_q_end, SIGNAL( textChanged( const QString & ) ), SLOT( testiq_q_end_text( const QString & ) ) );
   connect( le_testiq_q_end, SIGNAL( focussed ( bool ) )             , SLOT( testiq_q_end_focus( bool ) ) );

   cb_testiq_from_gaussian = new QCheckBox(this);
   cb_testiq_from_gaussian->setText( us_tr( "as pure Gaussian" ) );
   cb_testiq_from_gaussian->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_testiq_from_gaussian->setPalette( PALET_NORMAL );
   AUTFBACK( cb_testiq_from_gaussian );
   cb_testiq_from_gaussian->setChecked( false );

   hbl_testiq_gaussians = new QHBoxLayout();

   lbl_testiq_gaussians = new QLabel( us_tr( "I(q) from Gaussian:  " ), this );
   lbl_testiq_gaussians->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_testiq_gaussians->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_testiq_gaussians );
   lbl_testiq_gaussians->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   rb_testiq_from_i_t =  new QRadioButton( us_tr( "None " ), this );
   rb_testiq_from_i_t -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_testiq_from_i_t );
   rb_testiq_from_i_t -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_testiq_from_i_t, SIGNAL( clicked() ), SLOT( testiq_gauss_line() ) );

#if 1 // QT_VERSION < 0x040000
   bg_testiq_gaussians = new QButtonGroup( this );
   bg_pos = 0;
   bg_testiq_gaussians->setExclusive( true );
   bg_testiq_gaussians->addButton( rb_testiq_from_i_t, bg_pos++ );
#else
   bg_testiq_gaussians = new QGroupBox();
   bg_testiq_gaussians->setFlat( true );

   {
      hbl_testiq_gaussians->addWidget( rb_testiq_from_i_t );
      bg_testiq_gaussians->setLayout( hbl_testiq_gaussians );
   }
#endif

   pb_testiq_visrange = new QPushButton(us_tr("Vis. range"), this);
   pb_testiq_visrange->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_testiq_visrange->setMinimumHeight(minHeight1);
   pb_testiq_visrange->setPalette( PALET_PUSHB );
   connect(pb_testiq_visrange, SIGNAL(clicked()), SLOT(testiq_visrange()));

   pb_testiq_testset = new QPushButton(us_tr("Create I(q) set"), this);
   pb_testiq_testset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_testiq_testset->setMinimumHeight(minHeight1);
   pb_testiq_testset->setPalette( PALET_PUSHB );
   connect(pb_testiq_testset, SIGNAL(clicked()), SLOT(testiq_testset()));
   pb_testiq_testset->setEnabled( false );

   // guinier mode

   pb_guinier = new QPushButton(us_tr("Guinier"), this);
   pb_guinier->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_guinier->setMinimumHeight(minHeight1);
   pb_guinier->setPalette( PALET_PUSHB );
   connect(pb_guinier, SIGNAL(clicked()), SLOT(guinier()));
   pb_guinier->setEnabled( false );

   pb_guinier_plot_rg = new QPushButton(us_tr("Rg plot"), this);
   pb_guinier_plot_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_guinier_plot_rg->setMinimumHeight(minHeight1);
   pb_guinier_plot_rg->setPalette( PALET_PUSHB );
   pb_guinier_plot_rg->setEnabled( true );
   connect(pb_guinier_plot_rg, SIGNAL(clicked()), SLOT(guinier_plot_rg_toggle()));

   pb_guinier_plot_mw = new QPushButton(us_tr("Approx. MW plot"), this);
   pb_guinier_plot_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_guinier_plot_mw->setMinimumHeight(minHeight1);
   pb_guinier_plot_mw->setPalette( PALET_PUSHB );
   pb_guinier_plot_mw->setEnabled( true );
   connect(pb_guinier_plot_mw, SIGNAL(clicked()), SLOT(guinier_plot_mw_toggle()));
   pb_guinier_plot_mw->hide();
   
   cb_guinier_scroll = new QCheckBox(this);
   cb_guinier_scroll->setText(us_tr("Scroll "));
   cb_guinier_scroll->setChecked( false );
   cb_guinier_scroll->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_guinier_scroll->setPalette( PALET_NORMAL );
   AUTFBACK( cb_guinier_scroll );
   connect( cb_guinier_scroll, SIGNAL( clicked() ), SLOT( guinier_scroll() ) );
   cb_guinier_scroll->hide();

   lbl_guinier_q_range = new QLabel( us_tr( "q range: " ), this );
   lbl_guinier_q_range->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_guinier_q_range->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_guinier_q_range );
   lbl_guinier_q_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_guinier_q_range->hide();

   le_guinier_q_start = new mQLineEdit( this );    le_guinier_q_start->setObjectName( "le_guinier_q_start Line Edit" );
   le_guinier_q_start->setText( "" );
   le_guinier_q_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_q_start->setPalette( cg_red );
   le_guinier_q_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_guinier_q_start->setEnabled( false );
   le_guinier_q_start->setValidator( new QDoubleValidator( le_guinier_q_start ) );
   connect( le_guinier_q_start, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_q_start_text( const QString & ) ) );
   connect( le_guinier_q_start, SIGNAL( focussed ( bool ) )             , SLOT( guinier_q_start_focus( bool ) ) );
   le_guinier_q_start->hide();

   le_guinier_q_end = new mQLineEdit( this );    le_guinier_q_end->setObjectName( "le_guinier_q_end Line Edit" );
   le_guinier_q_end->setText( "" );
   le_guinier_q_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_q_end->setPalette( cg_red );
   le_guinier_q_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_guinier_q_end->setEnabled( false );
   le_guinier_q_end->setValidator( new QDoubleValidator( le_guinier_q_end ) );
   connect( le_guinier_q_end, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_q_end_text( const QString & ) ) );
   connect( le_guinier_q_end, SIGNAL( focussed ( bool ) )             , SLOT( guinier_q_end_focus( bool ) ) );
   le_guinier_q_end->hide();

   lbl_guinier_q2_range = new QLabel( us_tr( "q^2 range: " ), this );
   lbl_guinier_q2_range->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_guinier_q2_range->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_guinier_q2_range );
   lbl_guinier_q2_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_guinier_q2_range->hide();

   le_guinier_q2_start = new mQLineEdit( this );    le_guinier_q2_start->setObjectName( "le_guinier_q2_start Line Edit" );
   le_guinier_q2_start->setText( "" );
   le_guinier_q2_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_q2_start->setPalette( cg_red );
   le_guinier_q2_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_guinier_q2_start->setEnabled( false );
   le_guinier_q2_start->setValidator( new QDoubleValidator( le_guinier_q2_start ) );
   le_guinier_q2_start->hide();
   connect( le_guinier_q2_start, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_q2_start_text( const QString & ) ) );
   connect( le_guinier_q2_start, SIGNAL( focussed ( bool ) )             , SLOT( guinier_q2_start_focus( bool ) ) );

   le_guinier_q2_end = new mQLineEdit( this );    le_guinier_q2_end->setObjectName( "le_guinier_q2_end Line Edit" );
   le_guinier_q2_end->setText( "" );
   le_guinier_q2_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_q2_end->setPalette( cg_red );
   le_guinier_q2_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_guinier_q2_end->setEnabled( false );
   le_guinier_q2_end->setValidator( new QDoubleValidator( le_guinier_q2_end ) );
   le_guinier_q2_end->hide();
   connect( le_guinier_q2_end, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_q2_end_text( const QString & ) ) );
   connect( le_guinier_q2_end, SIGNAL( focussed ( bool ) )             , SLOT( guinier_q2_end_focus( bool ) ) );

   lbl_guinier_delta_range = new QLabel( us_tr( " plot extension: " ), this );
   lbl_guinier_delta_range->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_guinier_delta_range->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_guinier_delta_range );
   lbl_guinier_delta_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_guinier_delta_start = new mQLineEdit( this );    le_guinier_delta_start->setObjectName( "le_guinier_delta_start Line Edit" );
   le_guinier_delta_start->setText( "" );
   le_guinier_delta_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_delta_start->setPalette( PALET_NORMAL );
   AUTFBACK( le_guinier_delta_start );
   le_guinier_delta_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_guinier_delta_start->setEnabled( false );
   le_guinier_delta_start->setValidator( new QDoubleValidator( le_guinier_delta_start ) );
   connect( le_guinier_delta_start, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_delta_start_text( const QString & ) ) );
   connect( le_guinier_delta_start, SIGNAL( focussed ( bool ) )             , SLOT( guinier_delta_start_focus( bool ) ) );

   le_guinier_delta_end = new mQLineEdit( this );    le_guinier_delta_end->setObjectName( "le_guinier_delta_end Line Edit" );
   le_guinier_delta_end->setText( "" );
   le_guinier_delta_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_delta_end->setPalette( PALET_NORMAL );
   AUTFBACK( le_guinier_delta_end );
   le_guinier_delta_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_guinier_delta_end->setEnabled( false );
   le_guinier_delta_end->setValidator( new QDoubleValidator( le_guinier_delta_end ) );
   connect( le_guinier_delta_end, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_delta_end_text( const QString & ) ) );
   connect( le_guinier_delta_end, SIGNAL( focussed ( bool ) )             , SLOT( guinier_delta_end_focus( bool ) ) );

   cb_guinier_qrgmax = new QCheckBox( this );
   cb_guinier_qrgmax->setText( us_tr( "qmax*Rg limit: " ) );
   cb_guinier_qrgmax->setPalette( PALET_NORMAL );
   AUTFBACK( cb_guinier_qrgmax );
   cb_guinier_qrgmax->setChecked( (( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "dad_cb_guinier_qrgmax" ) &&
                                  (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_cb_guinier_qrgmax" ] == "true" );
   cb_guinier_qrgmax->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   cb_guinier_qrgmax->hide();
   connect( cb_guinier_qrgmax, SIGNAL( clicked() ), SLOT( guinier_qrgmax() ) );

   le_guinier_qrgmax = new mQLineEdit( this );    le_guinier_qrgmax->setObjectName( "le_guinier_qrgmax Line Edit" );
   le_guinier_qrgmax->setText( (( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "dad_guinier_qrgmax" ) ?
                               (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_guinier_qrgmax" ] : "1.3" );
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
   cb_guinier_sd->setText(us_tr("SD "));
   cb_guinier_sd->setChecked( true );
   cb_guinier_sd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_guinier_sd->setPalette( PALET_NORMAL );
   AUTFBACK( cb_guinier_sd );
   connect( cb_guinier_sd, SIGNAL( clicked() ), SLOT( guinier_sd() ) );

   rb_guinier_resid_diff =  new QRadioButton( us_tr( "Difference" ), this );
   rb_guinier_resid_diff -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_guinier_resid_diff );
   rb_guinier_resid_diff -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_guinier_resid_diff, SIGNAL( clicked() ), SLOT( guinier_residuals_update() ) );

   rb_guinier_resid_sd =  new QRadioButton( us_tr( "Standard deviation" ), this );
   rb_guinier_resid_sd -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_guinier_resid_sd );
   rb_guinier_resid_sd -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_guinier_resid_sd, SIGNAL( clicked() ), SLOT( guinier_residuals_update() ) );

   rb_guinier_resid_pct =  new QRadioButton( us_tr( "Percent" ), this );
   rb_guinier_resid_pct -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_guinier_resid_pct );
   rb_guinier_resid_pct -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_guinier_resid_pct, SIGNAL( clicked() ), SLOT( guinier_residuals_update() ) );

#if 1 // QT_VERSION < 0x040000
   bg_guinier_resid_type = new QButtonGroup( this );
   bg_pos = 0;
   bg_guinier_resid_type->setExclusive(true);
   bg_guinier_resid_type->addButton( rb_guinier_resid_diff, bg_pos++ );
   bg_guinier_resid_type->addButton( rb_guinier_resid_sd, bg_pos++ );
   bg_guinier_resid_type->addButton( rb_guinier_resid_pct, bg_pos++ );
#else
   bg_guinier_resid_type = new QGroupBox();
   bg_guinier_resid_type->setFlat( true );

   {
      QVBoxLayout * bl = new QVBoxLayout; bl->setContentsMargins( 0, 0, 0, 0 ); bl->setSpacing( 0 );
      bl->addWidget( rb_guinier_resid_diff );
      bl->addWidget( rb_guinier_resid_sd );
      bl->addWidget( rb_guinier_resid_pct );
      bg_guinier_resid_type->setLayout( bl );
   }
#endif
   rb_guinier_resid_diff->setChecked( true );

   lbl_guinier_stats = new QLabel( "", this );
   lbl_guinier_stats->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
   lbl_guinier_stats->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_guinier_stats );
   // lbl_guinier_stats->setPalette( PALET_LABEL );
   lbl_guinier_stats->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

//   guinier_plot = new QwtPlot( qs_plots );
   usp_guinier_plot = new US_Plot( guinier_plot, "", "", "", qs_plots );
   connect( (QWidget *)guinier_plot->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_guinier_plot( const QPoint & ) ) );
   ((QWidget *)guinier_plot->titleLabel())->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)guinier_plot->axisWidget( QwtPlot::yLeft ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_guinier_plot( const QPoint & ) ) );
   ((QWidget *)guinier_plot->axisWidget( QwtPlot::yLeft ))->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)guinier_plot->axisWidget( QwtPlot::xBottom ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_guinier_plot( const QPoint & ) ) );
   ((QWidget *)guinier_plot->axisWidget( QwtPlot::xBottom ))->setContextMenuPolicy( Qt::CustomContextMenu );
   plot_info[ "HPLC SAXS Guinier" ] = guinier_plot;
#if QT_VERSION < 0x040000
   guinier_plot->enableGridXMin();
   guinier_plot->enableGridYMin();
#else
   guinier_plot_grid = new QwtPlotGrid;
   guinier_plot_grid->enableXMin( true );
   guinier_plot_grid->enableYMin( true );
#endif
   guinier_plot->setPalette( PALET_NORMAL );
   AUTFBACK( guinier_plot );
#if QT_VERSION < 0x040000
   guinier_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   guinier_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   guinier_plot_grid->setMajorPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   guinier_plot_grid->setMinorPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   guinier_plot_grid->attach( guinier_plot );
#endif
   guinier_plot->setAxisTitle(QwtPlot::xBottom, us_tr( "q^2 [1/Angstrom^2]" ) );
   guinier_plot->setAxisTitle(QwtPlot::yLeft, us_tr("I*(q) [g mol^-1]"));
#if QT_VERSION < 0x040000
   guinier_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   guinier_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   guinier_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   guinier_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   guinier_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   guinier_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   guinier_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
//    guinier_plot->setMargin(USglobal->config_list.margin);
   guinier_plot->setTitle("");
#if QT_VERSION < 0x040000
   guinier_plot->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::Logarithmic);
#else
   guinier_plot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine(10));
#endif
   guinier_plot->setCanvasBackground(USglobal->global_colors.plot);

#if QT_VERSION < 0x040000
   guinier_plot->setAutoLegend( false );
   guinier_plot->setLegendFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ) );
#else
   // {
   //    QwtLegend* legend_pd = new QwtLegend;
   //    legend_pd->setFrameStyle( QFrame::Box | QFrame::Sunken );
   //    guinier_plot->insertLegend( legend_pd, QwtPlot::BottomLegend );
   // }
#endif
#if QT_VERSION < 0x040000
   connect( guinier_plot->canvas(), SIGNAL( mouseReleased( const QMouseEvent & ) ), SLOT( plot_mouse(  const QMouseEvent & ) ) );
#endif

//   guinier_plot_errors = new QwtPlot( qs_plots );
   usp_guinier_plot_errors = new US_Plot( guinier_plot_errors, "", "", "", qs_plots );
   connect( (QWidget *)guinier_plot_errors->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_guinier_plot_errors( const QPoint & ) ) );
   ((QWidget *)guinier_plot_errors->titleLabel())->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)guinier_plot_errors->axisWidget( QwtPlot::yLeft ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_guinier_plot_errors( const QPoint & ) ) );
   ((QWidget *)guinier_plot_errors->axisWidget( QwtPlot::yLeft ))->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)guinier_plot_errors->axisWidget( QwtPlot::xBottom ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_guinier_plot_errors( const QPoint & ) ) );
   ((QWidget *)guinier_plot_errors->axisWidget( QwtPlot::xBottom ))->setContextMenuPolicy( Qt::CustomContextMenu );
   plot_info[ "HPLC SAXS Guinier Errors" ] = guinier_plot_errors;
#if QT_VERSION < 0x040000
   guinier_plot_errors->enableGridXMin();
   guinier_plot_errors->enableGridYMin();
#else
   guinier_plot_errors_grid = new QwtPlotGrid;
   guinier_plot_errors_grid->enableXMin( true );
   guinier_plot_errors_grid->enableYMin( true );
#endif
   guinier_plot_errors->setPalette( PALET_NORMAL );
   AUTFBACK( guinier_plot_errors );
#if QT_VERSION < 0x040000
   guinier_plot_errors->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   guinier_plot_errors->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   guinier_plot_errors_grid->setMajorPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   guinier_plot_errors_grid->setMinorPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   guinier_plot_errors_grid->attach( guinier_plot_errors );
#endif
   guinier_plot_errors->setAxisTitle(QwtPlot::xBottom, us_tr( "q^2 [1/Angstrom^2]" ) );
   guinier_plot_errors->setAxisTitle(QwtPlot::yLeft, us_tr("Intensity [a.u.] (log scale)"));
#if QT_VERSION < 0x040000
   guinier_plot_errors->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   guinier_plot_errors->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   guinier_plot_errors->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   guinier_plot_errors->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   guinier_plot_errors->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   guinier_plot_errors->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   guinier_plot_errors->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
//    guinier_plot_errors->setMargin(USglobal->config_list.margin);
   guinier_plot_errors->setTitle("");
#if QT_VERSION < 0x040000
   guinier_plot_errors->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::None);
#else
   guinier_plot_errors->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine );
#endif
   guinier_plot_errors->setCanvasBackground(USglobal->global_colors.plot);

#if QT_VERSION < 0x040000
   guinier_plot_errors->setAutoLegend( false );
   guinier_plot_errors->setLegendFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ) );
#else
   // {
   //    QwtLegend* legend_pd = new QwtLegend;
   //    legend_pd->setFrameStyle( QFrame::Box | QFrame::Sunken );
   //    guinier_plot_errors->insertLegend( legend_pd, QwtPlot::BottomLegend );
   // }
#endif
#if QT_VERSION < 0x040000
   connect( guinier_plot_errors->canvas(), SIGNAL( mouseReleased( const QMouseEvent & ) ), SLOT( plot_mouse(  const QMouseEvent & ) ) );
#endif

//   guinier_plot_rg = new QwtPlot( qs_plots );
   usp_guinier_plot_rg = new US_Plot( guinier_plot_rg, "", "", "", qs_plots );
   connect( (QWidget *)guinier_plot_rg->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_guinier_plot_rg( const QPoint & ) ) );
   ((QWidget *)guinier_plot_rg->titleLabel())->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)guinier_plot_rg->axisWidget( QwtPlot::yLeft ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_guinier_plot_rg( const QPoint & ) ) );
   ((QWidget *)guinier_plot_rg->axisWidget( QwtPlot::yLeft ))->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)guinier_plot_rg->axisWidget( QwtPlot::xBottom ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_guinier_plot_rg( const QPoint & ) ) );
   ((QWidget *)guinier_plot_rg->axisWidget( QwtPlot::xBottom ))->setContextMenuPolicy( Qt::CustomContextMenu );
   plot_info[ "HPLC SAXS Guinier Rg" ] = guinier_plot_rg;
#if QT_VERSION < 0x040000
   guinier_plot_rg->enableGridXMin();
   guinier_plot_rg->enableGridYMin();
#else
   guinier_plot_rg_grid = new QwtPlotGrid;
   guinier_plot_rg_grid->enableXMin( true );
   guinier_plot_rg_grid->enableYMin( true );
#endif
   guinier_plot_rg->setPalette( PALET_NORMAL );
   AUTFBACK( guinier_plot_rg );
#if QT_VERSION < 0x040000
   guinier_plot_rg->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   guinier_plot_rg->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   guinier_plot_rg_grid->setMajorPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   guinier_plot_rg_grid->setMinorPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   guinier_plot_rg_grid->attach( guinier_plot_rg );
#endif
   guinier_plot_rg->setAxisTitle(QwtPlot::xBottom, us_tr( "Time [a.u.]" ) );
   guinier_plot_rg->setAxisTitle(QwtPlot::yLeft, us_tr("Rg [Angstrom]"));
#if QT_VERSION < 0x040000
   guinier_plot_rg->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   guinier_plot_rg->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   guinier_plot_rg->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   guinier_plot_rg->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   guinier_plot_rg->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   guinier_plot_rg->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   guinier_plot_rg->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
//    guinier_plot_rg->setMargin(USglobal->config_list.margin);
   guinier_plot_rg->setTitle("");
#if QT_VERSION < 0x040000
   guinier_plot_rg->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::None);
#else
   guinier_plot_rg->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine );
#endif
   guinier_plot_rg->setCanvasBackground(USglobal->global_colors.plot);

#if QT_VERSION < 0x040000
   guinier_plot_rg->setAutoLegend( false );
   guinier_plot_rg->setLegendFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ) );
#else
   // {
   //    QwtLegend* legend_pd = new QwtLegend;
   //    legend_pd->setFrameStyle( QFrame::Box | QFrame::Sunken );
   //    guinier_plot_rg->insertLegend( legend_pd, QwtPlot::BottomLegend );
   // }
#endif
#if QT_VERSION < 0x040000
   connect( guinier_plot_rg->canvas(), SIGNAL( mouseReleased( const QMouseEvent & ) ), SLOT( plot_mouse(  const QMouseEvent & ) ) );
#endif


//   guinier_plot_mw = new QwtPlot( qs_plots );
   usp_guinier_plot_mw = new US_Plot( guinier_plot_mw, "", "", "", qs_plots );
   connect( (QWidget *)guinier_plot_mw->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_guinier_plot_mw( const QPoint & ) ) );
   ((QWidget *)guinier_plot_mw->titleLabel())->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)guinier_plot_mw->axisWidget( QwtPlot::yLeft ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_guinier_plot_mw( const QPoint & ) ) );
   ((QWidget *)guinier_plot_mw->axisWidget( QwtPlot::yLeft ))->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)guinier_plot_mw->axisWidget( QwtPlot::xBottom ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_guinier_plot_mw( const QPoint & ) ) );
   ((QWidget *)guinier_plot_mw->axisWidget( QwtPlot::xBottom ))->setContextMenuPolicy( Qt::CustomContextMenu );
   plot_info[ "HPLC SAXS Guinier MW" ] = guinier_plot_mw;
#if QT_VERSION < 0x040000
   guinier_plot_mw->enableGridXMin();
   guinier_plot_mw->enableGridYMin();
#else
   guinier_plot_mw_grid = new QwtPlotGrid;
   guinier_plot_mw_grid->enableXMin( true );
   guinier_plot_mw_grid->enableYMin( true );
#endif
   guinier_plot_mw->setPalette( PALET_NORMAL );
   AUTFBACK( guinier_plot_mw );
#if QT_VERSION < 0x040000
   guinier_plot_mw->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   guinier_plot_mw->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   guinier_plot_mw_grid->setMajorPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   guinier_plot_mw_grid->setMinorPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   guinier_plot_mw_grid->attach( guinier_plot_mw );
#endif
   guinier_plot_mw->setAxisTitle(QwtPlot::xBottom, us_tr( "Time [a.u.]" ) );
   guinier_plot_mw->setAxisTitle(QwtPlot::yLeft, us_tr( started_in_expert_mode ? "Approx. MW [Daltons]" : "MW[RT] [Daltons]" ));
#if QT_VERSION < 0x040000
   guinier_plot_mw->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   guinier_plot_mw->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   guinier_plot_mw->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   guinier_plot_mw->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   guinier_plot_mw->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   guinier_plot_mw->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   guinier_plot_mw->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
//    guinier_plot_mw->setMargin(USglobal->config_list.margin);
   guinier_plot_mw->setTitle("");
#if QT_VERSION < 0x040000
   guinier_plot_mw->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::None);
#else
   guinier_plot_mw->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine );
#endif
   guinier_plot_mw->setCanvasBackground(USglobal->global_colors.plot);

#if QT_VERSION < 0x040000
   guinier_plot_mw->setAutoLegend( started_in_expert_mode );
   guinier_plot_mw->setLegendFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ) );
#else
   // {
   //    QwtLegend* legend_pd = new QwtLegend;
   //    legend_pd->setFrameStyle( QFrame::Box | QFrame::Sunken );
   //    guinier_plot_mw->insertLegend( legend_pd, QwtPlot::BottomLegend );
   // }
#endif
#if QT_VERSION < 0x040000
   connect( guinier_plot_mw->canvas(), SIGNAL( mouseReleased( const QMouseEvent & ) ), SLOT( plot_mouse(  const QMouseEvent & ) ) );
#endif

//   guinier_plot_summary = new QwtPlot( 0 );
   usp_guinier_plot_summary = new US_Plot( guinier_plot_summary, "", "", "", 0 );
   connect( (QWidget *)guinier_plot_summary->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_guinier_plot_summary( const QPoint & ) ) );
   ((QWidget *)guinier_plot_summary->titleLabel())->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)guinier_plot_summary->axisWidget( QwtPlot::yLeft ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_guinier_plot_summary( const QPoint & ) ) );
   ((QWidget *)guinier_plot_summary->axisWidget( QwtPlot::yLeft ))->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)guinier_plot_summary->axisWidget( QwtPlot::xBottom ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_guinier_plot_summary( const QPoint & ) ) );
   ((QWidget *)guinier_plot_summary->axisWidget( QwtPlot::xBottom ))->setContextMenuPolicy( Qt::CustomContextMenu );
   guinier_plot_summary->hide();
   plot_info[ "HPLC SAXS Guinier Summary" ] = guinier_plot_summary;

   lbl_guinier_rg_t_range = new QLabel( us_tr( "Time range for Rg plot: " ), this );
   lbl_guinier_rg_t_range->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_guinier_rg_t_range->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_guinier_rg_t_range );
   lbl_guinier_rg_t_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_guinier_rg_t_start = new mQLineEdit( this );    le_guinier_rg_t_start->setObjectName( "le_guinier_rg_t_start Line Edit" );
   le_guinier_rg_t_start->setText( "" );
   le_guinier_rg_t_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_rg_t_start->setPalette( PALET_NORMAL );
   AUTFBACK( le_guinier_rg_t_start );
   le_guinier_rg_t_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_guinier_rg_t_start->setEnabled( false );
   le_guinier_rg_t_start->setValidator( new QDoubleValidator( le_guinier_rg_t_start ) );
   connect( le_guinier_rg_t_start, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_rg_t_start_text( const QString & ) ) );
   connect( le_guinier_rg_t_start, SIGNAL( focussed ( bool ) )             , SLOT( guinier_rg_t_start_focus( bool ) ) );

   le_guinier_rg_t_end = new mQLineEdit( this );    le_guinier_rg_t_end->setObjectName( "le_guinier_rg_t_end Line Edit" );
   le_guinier_rg_t_end->setText( "" );
   le_guinier_rg_t_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_rg_t_end->setPalette( PALET_NORMAL );
   AUTFBACK( le_guinier_rg_t_end );
   le_guinier_rg_t_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_guinier_rg_t_end->setEnabled( false );
   le_guinier_rg_t_end->setValidator( new QDoubleValidator( le_guinier_rg_t_end ) );
   connect( le_guinier_rg_t_end, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_rg_t_end_text( const QString & ) ) );
   connect( le_guinier_rg_t_end, SIGNAL( focussed ( bool ) )             , SLOT( guinier_rg_t_end_focus( bool ) ) );

   lbl_guinier_rg_rg_range = new QLabel( us_tr( "Rg range: " ), this );
   lbl_guinier_rg_rg_range->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_guinier_rg_rg_range->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_guinier_rg_rg_range );
   lbl_guinier_rg_rg_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_guinier_rg_rg_start = new mQLineEdit( this );    le_guinier_rg_rg_start->setObjectName( "le_guinier_rg_rg_start Line Edit" );
   le_guinier_rg_rg_start->setText( "" );
   le_guinier_rg_rg_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_rg_rg_start->setPalette( PALET_NORMAL );
   AUTFBACK( le_guinier_rg_rg_start );
   le_guinier_rg_rg_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_guinier_rg_rg_start->setEnabled( false );
   le_guinier_rg_rg_start->setValidator( new QDoubleValidator( le_guinier_rg_rg_start ) );
   connect( le_guinier_rg_rg_start, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_rg_rg_start_text( const QString & ) ) );
   connect( le_guinier_rg_rg_start, SIGNAL( focussed ( bool ) )             , SLOT( guinier_rg_rg_start_focus( bool ) ) );

   le_guinier_rg_rg_end = new mQLineEdit( this );    le_guinier_rg_rg_end->setObjectName( "le_guinier_rg_rg_end Line Edit" );
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
   cb_guinier_lock_rg_range->setText(us_tr("Lock range"));
   cb_guinier_lock_rg_range->setChecked( false );
   cb_guinier_lock_rg_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_guinier_lock_rg_range->setPalette( PALET_NORMAL );
   AUTFBACK( cb_guinier_lock_rg_range );

   pb_guinier_replot = new QPushButton(us_tr("Replot"), this);
   pb_guinier_replot->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_guinier_replot->setMinimumHeight(minHeight1);
   pb_guinier_replot->setPalette( PALET_PUSHB );
   pb_guinier_replot->setEnabled( true );
   connect(pb_guinier_replot, SIGNAL(clicked()), SLOT(guinier_replot()));

   lbl_guinier_mw_t_range = new QLabel( us_tr( "Time range for MW plot: " ), this );
   lbl_guinier_mw_t_range->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_guinier_mw_t_range->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_guinier_mw_t_range );
   lbl_guinier_mw_t_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_guinier_mw_t_start = new mQLineEdit( this );    le_guinier_mw_t_start->setObjectName( "le_guinier_mw_t_start Line Edit" );
   le_guinier_mw_t_start->setText( "" );
   le_guinier_mw_t_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_mw_t_start->setPalette( PALET_NORMAL );
   AUTFBACK( le_guinier_mw_t_start );
   le_guinier_mw_t_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_guinier_mw_t_start->setEnabled( false );
   le_guinier_mw_t_start->setValidator( new QDoubleValidator( le_guinier_mw_t_start ) );
   connect( le_guinier_mw_t_start, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_mw_t_start_text( const QString & ) ) );
   connect( le_guinier_mw_t_start, SIGNAL( focussed ( bool ) )             , SLOT( guinier_mw_t_start_focus( bool ) ) );

   le_guinier_mw_t_end = new mQLineEdit( this );    le_guinier_mw_t_end->setObjectName( "le_guinier_mw_t_end Line Edit" );
   le_guinier_mw_t_end->setText( "" );
   le_guinier_mw_t_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_mw_t_end->setPalette( PALET_NORMAL );
   AUTFBACK( le_guinier_mw_t_end );
   le_guinier_mw_t_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_guinier_mw_t_end->setEnabled( false );
   le_guinier_mw_t_end->setValidator( new QDoubleValidator( le_guinier_mw_t_end ) );
   connect( le_guinier_mw_t_end, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_mw_t_end_text( const QString & ) ) );
   connect( le_guinier_mw_t_end, SIGNAL( focussed ( bool ) )             , SLOT( guinier_mw_t_end_focus( bool ) ) );

   lbl_guinier_mw_mw_range = new QLabel( us_tr( "MW range: " ), this );
   lbl_guinier_mw_mw_range->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_guinier_mw_mw_range->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_guinier_mw_mw_range );
   lbl_guinier_mw_mw_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_guinier_mw_mw_start = new mQLineEdit( this );    le_guinier_mw_mw_start->setObjectName( "le_guinier_mw_mw_start Line Edit" );
   le_guinier_mw_mw_start->setText( "" );
   le_guinier_mw_mw_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_mw_mw_start->setPalette( PALET_NORMAL );
   AUTFBACK( le_guinier_mw_mw_start );
   le_guinier_mw_mw_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_guinier_mw_mw_start->setEnabled( false );
   le_guinier_mw_mw_start->setValidator( new QDoubleValidator( le_guinier_mw_mw_start ) );
   connect( le_guinier_mw_mw_start, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_mw_mw_start_text( const QString & ) ) );
   connect( le_guinier_mw_mw_start, SIGNAL( focussed ( bool ) )             , SLOT( guinier_mw_mw_start_focus( bool ) ) );

   le_guinier_mw_mw_end = new mQLineEdit( this );    le_guinier_mw_mw_end->setObjectName( "le_guinier_mw_mw_end Line Edit" );
   le_guinier_mw_mw_end->setText( "" );
   le_guinier_mw_mw_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_mw_mw_end->setPalette( PALET_NORMAL );
   AUTFBACK( le_guinier_mw_mw_end );
   le_guinier_mw_mw_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_guinier_mw_mw_end->setEnabled( false );
   le_guinier_mw_mw_end->setValidator( new QDoubleValidator( le_guinier_mw_mw_end ) );
   connect( le_guinier_mw_mw_end, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_mw_mw_end_text( const QString & ) ) );
   connect( le_guinier_mw_mw_end, SIGNAL( focussed ( bool ) )             , SLOT( guinier_mw_mw_end_focus( bool ) ) );

   cb_guinier_lock_mw_range = new QCheckBox(this);
   cb_guinier_lock_mw_range->setText(us_tr("Lock range"));
   cb_guinier_lock_mw_range->setChecked( false );
   cb_guinier_lock_mw_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_guinier_lock_mw_range->setPalette( PALET_NORMAL );
   AUTFBACK( cb_guinier_lock_mw_range );

   pb_guinier_mw_replot = new QPushButton(us_tr("Replot"), this);
   pb_guinier_mw_replot->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_guinier_mw_replot->setMinimumHeight(minHeight1);
   pb_guinier_mw_replot->setPalette( PALET_PUSHB );
   pb_guinier_mw_replot->setEnabled( true );
   connect(pb_guinier_mw_replot, SIGNAL(clicked()), SLOT(guinier_replot()));


   // rgc mode

   pb_rgc = new QPushButton(us_tr("Rg utility"), this);
   pb_rgc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_rgc->setMinimumHeight(minHeight1);
   pb_rgc->setPalette( PALET_PUSHB );
   connect(pb_rgc, SIGNAL(clicked()), SLOT(rgc()));
   pb_rgc->setEnabled( true );

   lbl_rgc_mw = new QLabel( us_tr( "MW [kDalton]" ), this );
   lbl_rgc_mw->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_rgc_mw->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_rgc_mw );
   lbl_rgc_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_rgc_mw = new mQLineEdit( this );    le_rgc_mw->setObjectName( "le_rgc_mw Line Edit" );
   le_rgc_mw->setText( "" );
   le_rgc_mw->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_rgc_mw->setPalette( PALET_NORMAL );
   AUTFBACK( le_rgc_mw );
   le_rgc_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_rgc_mw->setEnabled( true );
   le_rgc_mw->setValidator( new QDoubleValidator( le_rgc_mw ) );
   ((QDoubleValidator *)le_rgc_mw->validator())->setBottom( 1 );
   connect( le_rgc_mw, SIGNAL( textChanged( const QString & ) ), SLOT( rgc_mw_text( const QString & ) ) );

   lbl_rgc_vol = new QLabel( us_tr( "Volume [A^3]" ), this );
   lbl_rgc_vol->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_rgc_vol->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_rgc_vol );
   lbl_rgc_vol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_rgc_vol = new mQLineEdit( this );    le_rgc_vol->setObjectName( "le_rgc_vol Line Edit" );
   le_rgc_vol->setText( "" );
   le_rgc_vol->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_rgc_vol->setPalette( PALET_NORMAL );
   AUTFBACK( le_rgc_vol );
   le_rgc_vol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_rgc_vol->setEnabled( true );
   le_rgc_vol->setValidator( new QDoubleValidator( le_rgc_vol ) );
   // ((QDoubleValidator *)le_rgc_vol->validator())->setBottom( 1 );
   connect( le_rgc_vol, SIGNAL( textChanged( const QString & ) ), SLOT( rgc_vol_text( const QString & ) ) );

   lbl_rgc_rho = new QLabel( us_tr( "Density [g/cm^3]" ), this );
   lbl_rgc_rho->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_rgc_rho->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_rgc_rho );
   lbl_rgc_rho->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_rgc_rho = new mQLineEdit( this );    le_rgc_rho->setObjectName( "le_rgc_rho Line Edit" );
   le_rgc_rho->setText( "1.4" );
   le_rgc_rho->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_rgc_rho->setPalette( PALET_NORMAL );
   AUTFBACK( le_rgc_rho );
   le_rgc_rho->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_rgc_rho->setEnabled( true );
   le_rgc_rho->setValidator( new QDoubleValidator( le_rgc_rho ) );
   ((QDoubleValidator *)le_rgc_rho->validator())->setRange( 1.2, 1.5 );
   connect( le_rgc_rho, SIGNAL( textChanged( const QString & ) ), SLOT( rgc_rho_text( const QString & ) ) );

   rb_rgc_shape_sphere =  new QRadioButton( us_tr( "Sphere " ), this );
   rb_rgc_shape_sphere -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_rgc_shape_sphere );
   rb_rgc_shape_sphere -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_rgc_shape_sphere, SIGNAL( clicked() ), SLOT( rgc_shape() ) );

   rb_rgc_shape_oblate =  new QRadioButton( us_tr( "Oblate " ), this );
   rb_rgc_shape_oblate -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_rgc_shape_oblate );
   rb_rgc_shape_oblate -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_rgc_shape_oblate, SIGNAL( clicked() ), SLOT( rgc_shape() ) );

   rb_rgc_shape_prolate =  new QRadioButton( us_tr( "Prolate " ), this );
   rb_rgc_shape_prolate -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_rgc_shape_prolate );
   rb_rgc_shape_prolate -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_rgc_shape_prolate, SIGNAL( clicked() ), SLOT( rgc_shape() ) );

   rb_rgc_shape_ellipsoid =  new QRadioButton( us_tr( "Ellipsoid " ), this );
   rb_rgc_shape_ellipsoid -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_rgc_shape_ellipsoid );
   rb_rgc_shape_ellipsoid -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_rgc_shape_ellipsoid, SIGNAL( clicked() ), SLOT( rgc_shape() ) );

#if 1 // QT_VERSION < 0x040000
   bg_rgc_shape = new QButtonGroup( this );
   bg_pos = 0;
   bg_rgc_shape->setExclusive(true);
   bg_rgc_shape->addButton( rb_rgc_shape_sphere, bg_pos++ );
   bg_rgc_shape->addButton( rb_rgc_shape_oblate, bg_pos++ );
   bg_rgc_shape->addButton( rb_rgc_shape_prolate, bg_pos++ );
   bg_rgc_shape->addButton( rb_rgc_shape_ellipsoid, bg_pos++ );
#else
   bg_rgc_shape = new QGroupBox();
   bg_rgc_shape->setFlat( true );

   {
      QHBoxLayout * bl = new QHBoxLayout; bl->setContentsMargins( 0, 0, 0, 0 ); bl->setSpacing( 0 );
      bl->addWidget( rb_rgc_shape_sphere );
      bl->addWidget( rb_rgc_shape_oblate );
      bl->addWidget( rb_rgc_shape_prolate );
      bl->addWidget( rb_rgc_shape_ellipsoid );
      bg_rgc_shape->setLayout( bl );
   }
#endif
   rb_rgc_shape_sphere->setChecked( true );

   lbl_rgc_axis = new QLabel( "", this );
   lbl_rgc_axis->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_rgc_axis->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_rgc_axis );
   lbl_rgc_axis->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_rgc_axis_b = new mQLineEdit( this );    le_rgc_axis_b->setObjectName( "le_rgc_axis_b Line Edit" );
   le_rgc_axis_b->setText( "1" );
   le_rgc_axis_b->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_rgc_axis_b->setPalette( PALET_NORMAL );
   AUTFBACK( le_rgc_axis_b );
   le_rgc_axis_b->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_rgc_axis_b->setEnabled( true );
   le_rgc_axis_b->setValidator( new QDoubleValidator( 0.01, 1.0, 3, le_rgc_axis_b ) );
   connect( le_rgc_axis_b, SIGNAL( textChanged( const QString & ) ), SLOT( rgc_axis_b_text( const QString & ) ) );

   le_rgc_axis_c = new mQLineEdit( this );    le_rgc_axis_c->setObjectName( "le_rgc_axis_c Line Edit" );
   le_rgc_axis_c->setText( "1" );
   le_rgc_axis_c->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_rgc_axis_c->setPalette( PALET_NORMAL );
   AUTFBACK( le_rgc_axis_c );
   le_rgc_axis_c->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_rgc_axis_c->setEnabled( true );
   le_rgc_axis_c->setValidator( new QDoubleValidator( 0.01, 1.0, 3, le_rgc_axis_c ) );
   connect( le_rgc_axis_c, SIGNAL( textChanged( const QString & ) ), SLOT( rgc_axis_c_text( const QString & ) ) );

   lbl_rgc_rg = new QLabel( us_tr( "Rg [A]:" ), this );
   lbl_rgc_rg->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_rgc_rg->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_rgc_rg );
   lbl_rgc_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_rgc_rg = new mQLineEdit( this );    le_rgc_rg->setObjectName( "le_rgc_rg Line Edit" );
   le_rgc_rg->setText( "" );
   le_rgc_rg->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_rgc_rg->setPalette( PALET_NORMAL );
   AUTFBACK( le_rgc_rg );
   le_rgc_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_rgc_rg->setEnabled( false );
   le_rgc_rg->setReadOnly( true );
   connect( le_rgc_rg, SIGNAL( textChanged( const QString & ) ), SLOT( rgc_rg_text( const QString & ) ) );

   lbl_rgc_extents = new QLabel( us_tr( "Axial extents [A]:" ), this );
   lbl_rgc_extents->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_rgc_extents->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_rgc_extents );
   lbl_rgc_extents->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_rgc_extents = new mQLineEdit( this );    le_rgc_extents->setObjectName( "le_rgc_extents Line Edit" );
   le_rgc_extents->setText( "" );
   le_rgc_extents->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_rgc_extents->setPalette( PALET_NORMAL );
   AUTFBACK( le_rgc_extents );
   le_rgc_extents->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_rgc_extents->setEnabled( false );
   le_rgc_extents->setReadOnly( true );

   lbl_rgc_g_qrange = new QLabel( us_tr( "Guinier range q [1/A]:" ), this );
   lbl_rgc_g_qrange->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_rgc_g_qrange->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_rgc_g_qrange );
   lbl_rgc_g_qrange->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_rgc_g_qrange = new mQLineEdit( this );    le_rgc_g_qrange->setObjectName( "le_rgc_g_qrange Line Edit" );
   le_rgc_g_qrange->setText( "" );
   le_rgc_g_qrange->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_rgc_g_qrange->setPalette( PALET_NORMAL );
   AUTFBACK( le_rgc_g_qrange );
   le_rgc_g_qrange->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_rgc_g_qrange->setEnabled( false );
   le_rgc_g_qrange->setReadOnly( true );

   // simulate

   pb_simulate = new QPushButton(us_tr("Simulate"), this);
   pb_simulate->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_simulate->setMinimumHeight(minHeight1);
   pb_simulate->setPalette( PALET_PUSHB );
   connect(pb_simulate, SIGNAL(clicked()), SLOT(simulate()));
   pb_simulate->setEnabled( false );

   // pm mode

   pb_pm = new QPushButton(us_tr("PM"), this);
   pb_pm->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_pm->setMinimumHeight(minHeight1);
   pb_pm->setPalette( PALET_PUSHB );
   connect(pb_pm, SIGNAL(clicked()), SLOT(pm()));
   pb_pm->setEnabled( false );

   rb_pm_shape_sphere =  new QRadioButton( us_tr( "Sphere " ), this );
   rb_pm_shape_sphere -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_pm_shape_sphere );
   rb_pm_shape_sphere -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_pm_shape_sphere, SIGNAL( clicked() ), SLOT( pm_enables() ) );

   rb_pm_shape_spheroid =  new QRadioButton( us_tr( "Spheroid " ), this );
   rb_pm_shape_spheroid -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_pm_shape_spheroid );
   rb_pm_shape_spheroid -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_pm_shape_spheroid, SIGNAL( clicked() ), SLOT( pm_enables() ) );

   rb_pm_shape_ellipsoid =  new QRadioButton( us_tr( "Ellipsoid " ), this );
   rb_pm_shape_ellipsoid -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_pm_shape_ellipsoid );
   rb_pm_shape_ellipsoid -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_pm_shape_ellipsoid, SIGNAL( clicked() ), SLOT( pm_enables() ) );

   rb_pm_shape_cylinder =  new QRadioButton( us_tr( "Cylinder " ), this );
   rb_pm_shape_cylinder -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_pm_shape_cylinder );
   rb_pm_shape_cylinder -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_pm_shape_cylinder, SIGNAL( clicked() ), SLOT( pm_enables() ) );

   rb_pm_shape_torus =  new QRadioButton( us_tr( "Torus " ), this );
   rb_pm_shape_torus -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_pm_shape_torus );
   rb_pm_shape_torus -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_pm_shape_torus, SIGNAL( clicked() ), SLOT( pm_enables() ) );

#if 1 //  QT_VERSION < 0x040000
   bg_pm_shape = new QButtonGroup( this );
   bg_pos = 0;
   bg_pm_shape->setExclusive(true);
   bg_pm_shape->addButton( rb_pm_shape_sphere, bg_pos++ );
   bg_pm_shape->addButton( rb_pm_shape_spheroid, bg_pos++ );
   bg_pm_shape->addButton( rb_pm_shape_ellipsoid, bg_pos++ );
   bg_pm_shape->addButton( rb_pm_shape_cylinder, bg_pos++ );
   bg_pm_shape->addButton( rb_pm_shape_torus, bg_pos++ );
#else
   bg_pm_shape = new QGroupBox();
   bg_pm_shape->setFlat( true );

   {
      QHBoxLayout * bl = new QHBoxLayout; bl->setContentsMargins( 0, 0, 0, 0 ); bl->setSpacing( 0 );
      bl->addWidget( rb_pm_shape_sphere );
      bl->addWidget( rb_pm_shape_spheroid );
      bl->addWidget( rb_pm_shape_ellipsoid );
      bl->addWidget( rb_pm_shape_cylinder );
      bl->addWidget( rb_pm_shape_torus );

      bg_pm_shape->setLayout( bl );
   }
#endif
   rb_pm_shape_sphere->setChecked( true );

   cb_pm_sd = new QCheckBox(this);
   cb_pm_sd->setText(us_tr("SD "));
   cb_pm_sd->setChecked( true );
   cb_pm_sd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_pm_sd->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pm_sd );
   connect( cb_pm_sd, SIGNAL( clicked() ), SLOT( pm_enables() ) );
   cb_pm_sd->hide();

   cb_pm_q_logbin = new QCheckBox(this);
   cb_pm_q_logbin->setText(us_tr("log q bins "));
   cb_pm_q_logbin->setChecked( false );
   cb_pm_q_logbin->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_pm_q_logbin->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pm_q_logbin );
   connect( cb_pm_q_logbin, SIGNAL( clicked() ), SLOT( pm_enables() ) );
   cb_pm_q_logbin->hide();

   lbl_pm_q_range = new QLabel( us_tr( "q range for modeling: " ), this );
   lbl_pm_q_range->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_pm_q_range->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_pm_q_range );
   lbl_pm_q_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_pm_q_start = new mQLineEdit( this );    le_pm_q_start->setObjectName( "le_pm_q_start Line Edit" );
   le_pm_q_start->setText( "" );
   le_pm_q_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_pm_q_start->setPalette( cg_red );
   le_pm_q_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_pm_q_start->setEnabled( false );
   le_pm_q_start->setValidator( new QDoubleValidator( le_pm_q_start ) );
   connect( le_pm_q_start, SIGNAL( textChanged( const QString & ) ), SLOT( pm_q_start_text( const QString & ) ) );
   connect( le_pm_q_start, SIGNAL( focussed ( bool ) )             , SLOT( pm_q_start_focus( bool ) ) );

   le_pm_q_end = new mQLineEdit( this );    le_pm_q_end->setObjectName( "le_pm_q_end Line Edit" );
   le_pm_q_end->setText( "" );
   le_pm_q_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_pm_q_end->setPalette( cg_red );
   le_pm_q_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_pm_q_end->setEnabled( false );
   le_pm_q_end->setValidator( new QDoubleValidator( le_pm_q_end ) );
   connect( le_pm_q_end, SIGNAL( textChanged( const QString & ) ), SLOT( pm_q_end_text( const QString & ) ) );
   connect( le_pm_q_end, SIGNAL( focussed ( bool ) )             , SLOT( pm_q_end_focus( bool ) ) );

   pb_pm_q_reset = new QPushButton(us_tr("Reset q range"), this);
   pb_pm_q_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_pm_q_reset->setMinimumHeight(minHeight1);
   pb_pm_q_reset->setPalette( PALET_PUSHB );
   connect(pb_pm_q_reset, SIGNAL(clicked()), SLOT(pm_q_reset()));
   pb_pm_q_reset->setEnabled( false );

   lbl_pm_samp_e_dens = new QLabel( us_tr( "Sample e density [e/A^3] (Tp. Prot: .41-.44, DNA:.59: RNA:.6 Carb:.49)" ), this );
   lbl_pm_samp_e_dens->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_pm_samp_e_dens->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_pm_samp_e_dens );
   lbl_pm_samp_e_dens->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_pm_samp_e_dens = new QLineEdit( this );    le_pm_samp_e_dens->setObjectName( "le_pm_samp_e_dens Line Edit" );
   le_pm_samp_e_dens->setText( ".425" );
   le_pm_samp_e_dens->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_pm_samp_e_dens->setPalette( PALET_NORMAL );
   AUTFBACK( le_pm_samp_e_dens );
   le_pm_samp_e_dens->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_pm_samp_e_dens->setEnabled( false );
   le_pm_samp_e_dens->setValidator( new QDoubleValidator( le_pm_samp_e_dens ) );
   connect( le_pm_samp_e_dens, SIGNAL( textChanged( const QString & ) ), SLOT( pm_samp_e_dens_text( const QString & ) ) );

   lbl_pm_buff_e_dens = new QLabel( us_tr( "Buffer e density [e/A^3]" ), this );
   lbl_pm_buff_e_dens->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_pm_buff_e_dens->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_pm_buff_e_dens );
   lbl_pm_buff_e_dens->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   
   le_pm_buff_e_dens = new QLineEdit( this );    le_pm_buff_e_dens->setObjectName( "le_pm_buff_e_dens Line Edit" );
   le_pm_buff_e_dens->setText( QString( "%1" ).arg((((US_Hydrodyn *)us_hydrodyn)->saxs_options.water_e_density ) ) );
   le_pm_buff_e_dens->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_pm_buff_e_dens->setPalette( PALET_NORMAL );
   AUTFBACK( le_pm_buff_e_dens );
   le_pm_buff_e_dens->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_pm_buff_e_dens->setEnabled( false );
   le_pm_buff_e_dens->setValidator( new QDoubleValidator( le_pm_buff_e_dens ) );
   connect( le_pm_buff_e_dens, SIGNAL( textChanged( const QString & ) ), SLOT( pm_buff_e_dens_text( const QString & ) ) );

   lbl_pm_grid_size = new QLabel( us_tr( "min. bead radius [A]" ), this );
   lbl_pm_grid_size->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_pm_grid_size->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_pm_grid_size );
   lbl_pm_grid_size->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   
   le_pm_grid_size = new QLineEdit( this );    le_pm_grid_size->setObjectName( "le_pm_grid_size Line Edit" );
   le_pm_grid_size->setText( "4" );
   le_pm_grid_size->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_pm_grid_size->setPalette( PALET_NORMAL );
   AUTFBACK( le_pm_grid_size );
   le_pm_grid_size->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_pm_grid_size->setEnabled( false );
   le_pm_grid_size->setValidator( new QIntValidator( le_pm_grid_size ) );
   connect( le_pm_grid_size, SIGNAL( textChanged( const QString & ) ), SLOT( pm_grid_size_text( const QString & ) ) );

   lbl_pm_q_pts = new QLabel( us_tr( " q points every n-th (1=all)" ), this );
   lbl_pm_q_pts->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_pm_q_pts->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_pm_q_pts );
   lbl_pm_q_pts->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   
   le_pm_q_pts = new QLineEdit( this );    le_pm_q_pts->setObjectName( "le_pm_q_pts Line Edit" );
   le_pm_q_pts->setText( "1" );
   le_pm_q_pts->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_pm_q_pts->setPalette( PALET_NORMAL );
   AUTFBACK( le_pm_q_pts );
   le_pm_q_pts->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_pm_q_pts->setEnabled( false );
   le_pm_q_pts->setValidator( new QIntValidator( le_pm_q_pts ) );
   connect( le_pm_q_pts, SIGNAL( textChanged( const QString & ) ), SLOT( pm_q_pts_text( const QString & ) ) );

   pb_pm_run = new QPushButton(us_tr("Start"), this);
   pb_pm_run->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_pm_run->setMinimumHeight(minHeight1);
   pb_pm_run->setPalette( PALET_PUSHB );
   connect(pb_pm_run, SIGNAL(clicked()), SLOT(pm_run()));
   pb_pm_q_reset->setEnabled( false );

   // fasta

   pb_fasta_file = new QPushButton(us_tr("Load FASTA sequence from file"), this);
   pb_fasta_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_fasta_file->setMinimumHeight(minHeight1);
   pb_fasta_file->setPalette( PALET_PUSHB );
   connect(pb_fasta_file, SIGNAL(clicked()), SLOT(fasta_file()));

   lbl_fasta_value = new QLabel( us_tr( " PSV [cm^3/g]:" ) );
   lbl_fasta_value->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_fasta_value->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_fasta_value );
   lbl_fasta_value->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_fasta_value = new QLineEdit( this );
   le_fasta_value->setObjectName( "le_fasta_value Line Edit" );
   le_fasta_value->setText( "" );
   le_fasta_value->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   le_fasta_value->setPalette( PALET_EDIT );
   AUTFBACK( le_fasta_value );
   le_fasta_value->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_fasta_value->setEnabled( false );
   le_fasta_value->setReadOnly( true );

   // dad
   lbl_dad_lambdas_data = new QLabel( dad_lambdas.summary_rich() );
   lbl_dad_lambdas_data->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_dad_lambdas_data->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_dad_lambdas_data );
   lbl_dad_lambdas_data->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_dad_lambdas_data->setTextInteractionFlags(Qt::TextSelectableByMouse);
   lbl_dad_lambdas_data->setCursor(QCursor(Qt::IBeamCursor));

   // pbmodes

   lbl_pbmode = new QLabel( us_tr( "Plot buttons:" ) );
   lbl_pbmode->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_pbmode->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_pbmode );
   lbl_pbmode->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   rb_pbmode_main = new QRadioButton( "Options", this ); 
   rb_pbmode_main->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   rb_pbmode_main->setMinimumHeight(minHeight3);
   rb_pbmode_main->setPalette( PALET_NORMAL );
   AUTFBACK( rb_pbmode_main );
   connect(rb_pbmode_main, SIGNAL(clicked( )), SLOT( set_pbmode_main( )));

   rb_pbmode_sel = new QRadioButton( "Selections", this ); 
   rb_pbmode_sel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   rb_pbmode_sel->setMinimumHeight(minHeight3);
   rb_pbmode_sel->setPalette( PALET_NORMAL );
   AUTFBACK( rb_pbmode_sel );
   connect(rb_pbmode_sel, SIGNAL(clicked( )), SLOT( set_pbmode_sel( )));

   rb_pbmode_crop = new QRadioButton( "Cropping", this ); 
   rb_pbmode_crop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   rb_pbmode_crop->setMinimumHeight(minHeight3);
   rb_pbmode_crop->setPalette( PALET_NORMAL );
   AUTFBACK( rb_pbmode_crop );
   connect(rb_pbmode_crop, SIGNAL(clicked( )), SLOT( set_pbmode_crop( )));

   rb_pbmode_conc = new QRadioButton( "Conc. Util.", this ); 
   rb_pbmode_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   rb_pbmode_conc->setMinimumHeight(minHeight3);
   rb_pbmode_conc->setPalette( PALET_NORMAL );
   AUTFBACK( rb_pbmode_conc );
   connect(rb_pbmode_conc, SIGNAL(clicked( )), SLOT( set_pbmode_conc( )));

   rb_pbmode_sd = new QRadioButton( "S.D. Util..", this ); 
   rb_pbmode_sd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   rb_pbmode_sd->setMinimumHeight(minHeight3);
   rb_pbmode_sd->setPalette( PALET_NORMAL );
   AUTFBACK( rb_pbmode_sd );
   connect(rb_pbmode_sd, SIGNAL(clicked( )), SLOT( set_pbmode_sd( )));

   rb_pbmode_fasta = new QRadioButton( "PSV Util.", this ); 
   rb_pbmode_fasta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   rb_pbmode_fasta->setMinimumHeight(minHeight3);
   rb_pbmode_fasta->setPalette( PALET_NORMAL );
   AUTFBACK( rb_pbmode_fasta );
   connect(rb_pbmode_fasta, SIGNAL(clicked( )), SLOT( set_pbmode_fasta( )));
   rb_pbmode_fasta->hide();

   rb_pbmode_dad = new QRadioButton( "UV-Vis info.", this ); 
   rb_pbmode_dad->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   rb_pbmode_dad->setMinimumHeight(minHeight3);
   rb_pbmode_dad->setPalette( PALET_NORMAL );
   AUTFBACK( rb_pbmode_dad );
   connect(rb_pbmode_dad, SIGNAL(clicked( )), SLOT( set_pbmode_dad( )));

   rb_pbmode_q_exclude = new QRadioButton( "q exclude", this ); 
   rb_pbmode_q_exclude->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   rb_pbmode_q_exclude->setMinimumHeight(minHeight3);
   rb_pbmode_q_exclude->setPalette( PALET_NORMAL );
   AUTFBACK( rb_pbmode_q_exclude );
   connect(rb_pbmode_q_exclude, SIGNAL(clicked( )), SLOT( set_pbmode_q_exclude( )));

   rb_pbmode_none = new QRadioButton( "None", this ); 
   rb_pbmode_none->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   rb_pbmode_none->setMinimumHeight(minHeight3);
   rb_pbmode_none->setPalette( PALET_NORMAL );
   AUTFBACK( rb_pbmode_none );
   connect(rb_pbmode_none, SIGNAL(clicked( )), SLOT( set_pbmode_none( )));

   bg_pbmode = new QButtonGroup( this );
   bg_pbmode->addButton( rb_pbmode_main );
   bg_pbmode->addButton( rb_pbmode_sel );
   bg_pbmode->addButton( rb_pbmode_crop );
   bg_pbmode->addButton( rb_pbmode_conc );
   bg_pbmode->addButton( rb_pbmode_sd );
   bg_pbmode->addButton( rb_pbmode_fasta );
   bg_pbmode->addButton( rb_pbmode_dad );
   bg_pbmode->addButton( rb_pbmode_q_exclude );
   bg_pbmode->addButton( rb_pbmode_none );

   // q exclude

   pb_q_exclude_vis = new QPushButton(us_tr("Excl. Vis."), this);
   pb_q_exclude_vis->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_q_exclude_vis->setMinimumHeight(minHeight1);
   pb_q_exclude_vis->setPalette( PALET_PUSHB );
   connect(pb_q_exclude_vis, SIGNAL(clicked()), SLOT(q_exclude_vis()));

   pb_q_exclude_left = new QPushButton(us_tr("Excl. Left"), this);
   pb_q_exclude_left->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_q_exclude_left->setMinimumHeight(minHeight1);
   pb_q_exclude_left->setPalette( PALET_PUSHB );
   connect(pb_q_exclude_left, SIGNAL(clicked()), SLOT(q_exclude_left()));

   pb_q_exclude_right = new QPushButton(us_tr("Excl. Right"), this);
   pb_q_exclude_right->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_q_exclude_right->setMinimumHeight(minHeight1);
   pb_q_exclude_right->setPalette( PALET_PUSHB );
   connect(pb_q_exclude_right, SIGNAL(clicked()), SLOT(q_exclude_right()));

   pb_q_exclude_clear = new QPushButton(us_tr("Excl. Clear"), this);
   pb_q_exclude_clear->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_q_exclude_clear->setMinimumHeight(minHeight1);
   pb_q_exclude_clear->setPalette( PALET_PUSHB );
   connect(pb_q_exclude_clear, SIGNAL(clicked()), SLOT(q_exclude_clear()));

   lbl_q_exclude_detail = new QLabel( this );
   lbl_q_exclude_detail->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_q_exclude_detail->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_q_exclude_detail );
   lbl_q_exclude_detail->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_q_exclude_detail->setTextInteractionFlags(Qt::TextSelectableByMouse);
   lbl_q_exclude_detail->setCursor(QCursor(Qt::IBeamCursor));

   q_exclude.clear();
   q_exclude_update_lbl();

   // bottom

   pb_help = new QPushButton(us_tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_options = new QPushButton(us_tr("Options"), this);
   pb_options->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   pb_options->setMinimumHeight(minHeight1);
   pb_options->setPalette( PALET_PUSHB );
   connect(pb_options, SIGNAL(clicked()), SLOT(options()));

   pb_cancel = new QPushButton(us_tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // extra signals
   connect( this, SIGNAL( do_resize_plots() ),         SLOT( resize_plots() ) );
   connect( this, SIGNAL( do_resize_guinier_plots() ), SLOT( resize_guinier_plots() ) );

   // build layout
   QBoxLayout * hbl_file_buttons_0 = new QHBoxLayout();
   {
      hbl_file_buttons_0->setContentsMargins( 0, 0, 0, 0 );
      hbl_file_buttons_0->setSpacing( 0 );
      hbl_file_buttons_0->addWidget ( pb_load_conc );
      hbl_file_buttons_0->addWidget ( pb_conc );
      hbl_file_buttons_0->addWidget ( pb_view );
   }

   QBoxLayout * hbl_file_buttons = new QHBoxLayout(); hbl_file_buttons->setContentsMargins( 0, 0, 0, 0 ); hbl_file_buttons->setSpacing( 0 );
   hbl_file_buttons->addWidget ( pb_add_files );
   hbl_file_buttons->addWidget ( pb_add_dir );
   hbl_file_buttons->addWidget ( pb_similar_files );
   hbl_file_buttons->addWidget ( pb_to_saxs );
   hbl_file_buttons->addWidget ( pb_clear_files );

   files_widgets.push_back( pb_add_files );
   files_widgets.push_back( pb_add_dir );
   // files_widgets.push_back( pb_similar_files );
   files_widgets.push_back( pb_conc );
   files_widgets.push_back( pb_clear_files );

   QBoxLayout * hbl_file_buttons_1 = new QHBoxLayout(); hbl_file_buttons_1->setContentsMargins( 0, 0, 0, 0 ); hbl_file_buttons_1->setSpacing( 0 );
   hbl_file_buttons_1->addWidget ( pb_regex_load );
   hbl_file_buttons_1->addWidget ( le_regex );
   hbl_file_buttons_1->addWidget ( le_regex_args );
   hbl_file_buttons_1->addWidget ( pb_save_state );

   files_expert_widgets.push_back( pb_regex_load );
   files_expert_widgets.push_back( le_regex );
   files_expert_widgets.push_back( le_regex_args );
   files_expert_widgets.push_back( pb_save_state );

   QBoxLayout * hbl_file_buttons_2 = new QHBoxLayout(); hbl_file_buttons_2->setContentsMargins( 0, 0, 0, 0 ); hbl_file_buttons_2->setSpacing( 0 );
   hbl_file_buttons_2->addWidget ( pb_select_all );
   hbl_file_buttons_2->addWidget ( pb_invert );
   // hbl_file_buttons_2->addWidget ( pb_adjacent );
   hbl_file_buttons_2->addWidget ( pb_select_nth );
   // hbl_file_buttons_2->addWidget ( pb_color_rotate );
   // hbl_file_buttons_2->addWidget ( pb_to_saxs );
   // hbl_file_buttons_2->addWidget ( pb_view );
   hbl_file_buttons_2->addWidget ( pb_ag );
   // hbl_file_buttons_2->addWidget ( pb_to_saxs );

   files_widgets.push_back ( pb_select_all );
   files_widgets.push_back ( pb_invert );
   // files_widgets.push_back ( pb_adjacent );
   files_widgets.push_back ( pb_select_nth );
   files_widgets.push_back ( pb_view );
   // files_widgets.push_back ( pb_movie );
   // files_widgets.push_back ( cb_eb );
   // files_widgets.push_back ( cb_dots );
   // files_widgets.push_back ( pb_axis_x );
   // files_widgets.push_back ( pb_axis_y );
   // files_widgets.push_back ( pb_rescale );

   QBoxLayout * hbl_file_buttons_2b = new QHBoxLayout(); hbl_file_buttons_2b->setContentsMargins( 0, 0, 0, 0 ); hbl_file_buttons_2b->setSpacing( 0 );
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
      pb_simulate->hide();
      pb_pm->hide();

      pb_ag->hide();

      // pb_conc->hide();
      // pb_normalize->hide();
      pb_timescale->hide();

      // pb_cormap->hide();
   }
   pb_conc_avg->hide();

   // #if defined( JAC_VERSION )
   // if ( !U_EXPT )
   // {
   //   pb_options->hide();
   // }
   // #endif
   QBoxLayout * l_pbmode = new QHBoxLayout();
   {
      l_pbmode->addWidget( lbl_pbmode );
      l_pbmode->addWidget( rb_pbmode_main );
      l_pbmode->addWidget( rb_pbmode_sel );
      l_pbmode->addWidget( rb_pbmode_crop );
      l_pbmode->addWidget( rb_pbmode_conc );
      l_pbmode->addWidget( rb_pbmode_sd );
      l_pbmode->addWidget( rb_pbmode_fasta );
      l_pbmode->addWidget( rb_pbmode_dad );
      l_pbmode->addWidget( rb_pbmode_q_exclude );
      l_pbmode->addWidget( rb_pbmode_none );
   }

   QBoxLayout * l_pbmode_main = new QHBoxLayout();
   {
      l_pbmode_main->setContentsMargins( 0, 0, 0, 0 );
      l_pbmode_main->setSpacing( 0 );

      l_pbmode_main->addWidget( pb_rescale );
      l_pbmode_main->addWidget( pb_rescale_y );
      l_pbmode_main->addWidget( pb_axis_x );
      l_pbmode_main->addWidget( pb_axis_y );
      l_pbmode_main->addWidget( cb_eb );
      l_pbmode_main->addWidget( cb_dots );
      l_pbmode_main->addWidget( pb_line_width );
      l_pbmode_main->addWidget( pb_color_rotate );
      l_pbmode_main->addWidget( pb_legend );

      pbmode_main_widgets.push_back( pb_rescale );
      pbmode_main_widgets.push_back( pb_rescale_y );
      pbmode_main_widgets.push_back( pb_axis_x );
      pbmode_main_widgets.push_back( pb_axis_y );
      pbmode_main_widgets.push_back( cb_eb );
      pbmode_main_widgets.push_back( cb_dots );
      pbmode_main_widgets.push_back( pb_line_width );
      pbmode_main_widgets.push_back( pb_color_rotate );
      pbmode_main_widgets.push_back( pb_legend );
   }

   QBoxLayout * l_pbmode_sel = new QHBoxLayout();
   {
      l_pbmode_sel->setContentsMargins( 0, 0, 0, 0 );
      l_pbmode_sel->setSpacing( 0 );

      l_pbmode_sel->addWidget( pb_select_vis );
      l_pbmode_sel->addWidget( pb_remove_vis );
      l_pbmode_sel->addWidget( pb_movie );
      l_pbmode_sel->addWidget( pb_pp );

      pbmode_sel_widgets.push_back( pb_select_vis );
      pbmode_sel_widgets.push_back( pb_remove_vis );
      pbmode_sel_widgets.push_back( pb_movie );
      pbmode_sel_widgets.push_back( pb_pp );
   }

   QBoxLayout * l_pbmode_crop = new QHBoxLayout();
   {
      l_pbmode_crop->setContentsMargins( 0, 0, 0, 0 );
      l_pbmode_crop->setSpacing( 0 );

      l_pbmode_crop->addWidget( pb_crop_common );
      l_pbmode_crop->addWidget( pb_crop_vis );
      l_pbmode_crop->addWidget( pb_crop_to_vis );
      l_pbmode_crop->addWidget( pb_crop_zero );
      l_pbmode_crop->addWidget( pb_crop_left );
      l_pbmode_crop->addWidget( pb_crop_undo );
      l_pbmode_crop->addWidget( pb_crop_right );

      pbmode_crop_widgets.push_back( pb_crop_common );
      pbmode_crop_widgets.push_back( pb_crop_vis );
      pbmode_crop_widgets.push_back( pb_crop_to_vis );
      pbmode_crop_widgets.push_back( pb_crop_zero );
      pbmode_crop_widgets.push_back( pb_crop_left );
      pbmode_crop_widgets.push_back( pb_crop_undo );
      pbmode_crop_widgets.push_back( pb_crop_right );
   }

   QBoxLayout * l_pbmode_conc = new QHBoxLayout();
   {
      l_pbmode_conc->setContentsMargins( 0, 0, 0, 0 );
      l_pbmode_conc->setSpacing( 0 );

      l_pbmode_conc->addWidget( pb_repeak );
      l_pbmode_conc->addWidget( pb_conc_file );
      l_pbmode_conc->addWidget( lbl_conc_file );
      l_pbmode_conc->addWidget( pb_timeshift );
      l_pbmode_conc->addWidget( pb_timescale );
      
      pbmode_conc_widgets.push_back( pb_repeak );
      pbmode_conc_widgets.push_back( pb_conc_file );
      pbmode_conc_widgets.push_back( lbl_conc_file );
      pbmode_conc_widgets.push_back( pb_timeshift );
      pbmode_conc_widgets.push_back( pb_timescale );
   }

   QBoxLayout * l_pbmode_sd = new QHBoxLayout();
   {
      l_pbmode_sd->setContentsMargins( 0, 0, 0, 0 );
      l_pbmode_sd->setSpacing( 0 );

      l_pbmode_sd->addWidget( pb_wyatt_start );
      l_pbmode_sd->addWidget( pb_wyatt_apply );

      pbmode_sd_widgets.push_back( pb_wyatt_start );
      pbmode_sd_widgets.push_back( pb_wyatt_apply );
   }

   QBoxLayout * l_pbmode_fasta = new QHBoxLayout();
   {
      l_pbmode_fasta->setContentsMargins( 0, 0, 0, 0 );
      l_pbmode_fasta->setSpacing( 0 );

      l_pbmode_fasta->addWidget( pb_fasta_file );
      l_pbmode_fasta->addWidget( lbl_fasta_value );
      l_pbmode_fasta->addWidget( le_fasta_value );

      pbmode_fasta_widgets.push_back( pb_fasta_file );
      pbmode_fasta_widgets.push_back( lbl_fasta_value );
      pbmode_fasta_widgets.push_back( le_fasta_value );
   }

   QBoxLayout * l_pbmode_dad = new QHBoxLayout();
   {
      l_pbmode_dad->setContentsMargins( 0, 0, 0, 0 );
      l_pbmode_dad->setSpacing( 0 );

      // l_pbmode_dad->addWidget( pb_dad_lambdas_save );
      // l_pbmode_dad->addWidget( lbl_dad_lambdas_data );

      // pbmode_dad_widgets.push_back( pb_dad_lambdas_save );
      pbmode_dad_widgets.push_back( lbl_dad_lambdas_data );
   }
   
   QBoxLayout * l_pbmode_q_exclude = new QHBoxLayout();
   {
      l_pbmode_q_exclude->setContentsMargins( 0, 0, 0, 0 );
      l_pbmode_q_exclude->setSpacing( 0 );

      pbmode_q_exclude_widgets.push_back( pb_q_exclude_vis );
      pbmode_q_exclude_widgets.push_back( pb_q_exclude_left );
      pbmode_q_exclude_widgets.push_back( pb_q_exclude_right );
      pbmode_q_exclude_widgets.push_back( pb_q_exclude_clear );

      for ( auto const & widget : pbmode_q_exclude_widgets ) {
         l_pbmode_q_exclude->addWidget( widget );
      }

      // not in this layout
      pbmode_q_exclude_widgets.push_back( lbl_q_exclude_detail );
   }

   QBoxLayout * hbl_file_buttons_3 = new QHBoxLayout(); hbl_file_buttons_3->setContentsMargins( 0, 0, 0, 0 ); hbl_file_buttons_3->setSpacing( 0 );
   hbl_file_buttons_3->addWidget ( pb_conc_avg );
   hbl_file_buttons_3->addWidget ( pb_normalize );
   hbl_file_buttons_3->addWidget ( pb_avg );
   hbl_file_buttons_3->addWidget ( pb_add );
   hbl_file_buttons_3->addWidget ( pb_bin );
   hbl_file_buttons_3->addWidget ( pb_smooth );

   // files_widgets.push_back ( pb_conc_avg );
   files_widgets.push_back ( pb_normalize );
   files_widgets.push_back ( pb_avg );
   files_expert_widgets.push_back ( pb_add );
   files_widgets.push_back ( pb_to_saxs );
   // files_widgets.push_back ( pb_line_width );
   // files_widgets.push_back ( pb_color_rotate );
   files_expert_widgets.push_back ( pb_ag );

   QBoxLayout * hbl_file_buttons_3b = new QHBoxLayout(); hbl_file_buttons_3b->setContentsMargins( 0, 0, 0, 0 ); hbl_file_buttons_3b->setSpacing( 0 );
   hbl_file_buttons_3b->addWidget ( pb_baseline2_start );
   hbl_file_buttons_3b->addWidget ( pb_baseline2_apply );

   files_widgets.push_back ( pb_baseline2_start );
   files_widgets.push_back ( pb_baseline2_apply );

   QBoxLayout * hbl_file_buttons_4 = new QHBoxLayout(); hbl_file_buttons_4->setContentsMargins( 0, 0, 0, 0 ); hbl_file_buttons_4->setSpacing( 0 );
   hbl_file_buttons_4->addWidget ( pb_powerfit );
   hbl_file_buttons_4->addWidget ( pb_create_i_of_t );
   hbl_file_buttons_4->addWidget ( pb_svd );
   hbl_file_buttons_4->addWidget ( pb_test_i_of_t );
   hbl_file_buttons_4->addWidget ( pb_create_i_of_q );
   hbl_file_buttons_4->addWidget ( pb_create_ihash_t );
   hbl_file_buttons_4->addWidget ( pb_create_istar_q );

   files_widgets.push_back ( pb_bin );
   files_widgets.push_back ( pb_smooth );
   // hidden: files_widgets.push_back ( pb_svd );
   files_widgets.push_back ( pb_powerfit );
   files_widgets.push_back ( pb_create_i_of_t );
   files_widgets.push_back ( pb_test_i_of_t );
   files_widgets.push_back ( pb_create_i_of_q );
   files_widgets.push_back ( pb_create_ihash_t );
   files_widgets.push_back ( pb_create_istar_q );

   QBoxLayout * hbl_conc_file = new QHBoxLayout(); hbl_conc_file->setContentsMargins( 0, 0, 0, 0 ); hbl_conc_file->setSpacing( 0 );
   // hbl_conc_file->addWidget ( pb_repeak );
   // hbl_conc_file->addWidget ( pb_conc_file );
   // hbl_conc_file->addWidget ( pb_detector );

   files_widgets.push_back ( pb_load_conc );
   // files_widgets.push_back ( pb_repeak );
   // files_widgets.push_back ( pb_conc_file );
   // files_widgets.push_back ( pb_detector );

   //    QBoxLayout * hbl_dad = new QHBoxLayout(); hbl_dad->setContentsMargins( 0, 0, 0, 0 ); hbl_dad->setSpacing( 0 );
   //    hbl_dad->addWidget ( pb_set_dad );
   //    hbl_dad->addWidget ( lbl_dad );

   //    QBoxLayout * hbl_empty = new QHBoxLayout(); hbl_empty->setContentsMargins( 0, 0, 0, 0 ); hbl_empty->setSpacing( 0 );
   //    hbl_empty->addWidget ( pb_set_empty );
   //    hbl_empty->addWidget ( lbl_empty );

   //    QBoxLayout * hbl_signal = new QHBoxLayout(); hbl_signal->setContentsMargins( 0, 0, 0, 0 ); hbl_signal->setSpacing( 0 );
   //    hbl_signal->addWidget ( pb_set_signal );
   //    hbl_signal->addWidget ( lbl_signal );

   QBoxLayout * hbl_created = new QHBoxLayout(); hbl_created->setContentsMargins( 0, 0, 0, 0 ); hbl_created->setSpacing( 0 );
   hbl_created->addWidget ( pb_select_all_created );
   hbl_created->addWidget ( pb_invert_all_created );
   hbl_created->addWidget ( pb_adjacent_created );
   hbl_created->addWidget ( pb_remove_created );

   created_files_widgets.push_back( pb_select_all_created );
   created_files_widgets.push_back( pb_invert_all_created );
   // created_files_widgets.push_back( pb_adjacent_created );
   created_files_widgets.push_back( pb_remove_created );
   created_files_widgets.push_back( pb_save_created_csv );
   created_files_widgets.push_back( pb_save_created );

   QBoxLayout * hbl_created_2 = new QHBoxLayout(); hbl_created_2->setContentsMargins( 0, 0, 0, 0 ); hbl_created_2->setSpacing( 0 );
   hbl_created_2->addWidget ( pb_show_created );
   hbl_created_2->addWidget ( pb_show_only_created );
   hbl_created_2->addWidget ( pb_save_created_csv );
   hbl_created_2->addWidget ( pb_save_created );

   created_files_widgets.push_back ( pb_show_created );
   created_files_widgets.push_back ( pb_show_only_created );

   QBoxLayout * vbl_editor_group = new QVBoxLayout(0); vbl_editor_group->setContentsMargins( 0, 0, 0, 0 ); vbl_editor_group->setSpacing( 0 );
   vbl_editor_group->addWidget ( lbl_editor );
#if QT_VERSION < 0x040000 || !defined(Q_OS_MAC)
   vbl_editor_group->addWidget ( frame );
#endif
   vbl_editor_group->addWidget ( editor );

   QHBoxLayout * hbl_dir = new QHBoxLayout(); hbl_dir->setContentsMargins( 0, 0, 0, 0 ); hbl_dir->setSpacing( 0 );
   hbl_dir->addWidget( cb_lock_dir );
   hbl_dir->addWidget( le_dir );

   files_widgets.push_back( cb_lock_dir );
   files_widgets.push_back( le_dir );

   files_widgets.push_back ( lb_files );
   files_widgets.push_back ( lbl_selected );
   // files_widgets.push_back ( lbl_conc_file );

   created_files_widgets.push_back ( lb_created_files );
   created_files_widgets.push_back ( le_created_dir );
   created_files_widgets.push_back ( lbl_selected_created );

   QBoxLayout * vbl_model = new QVBoxLayout( 0 ); vbl_model->setContentsMargins( 0, 0, 0, 0 ); vbl_model->setSpacing( 0 );
   vbl_model->addWidget( lbl_model_files );
   vbl_model->addWidget( lb_model_files );
   {
      QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( pb_model_select_all );
      hbl->addWidget( pb_model_text );
      hbl->addWidget( pb_model_view );
      hbl->addWidget( pb_model_remove );
      hbl->addWidget( pb_model_save );
      vbl_model->addLayout( hbl );
   }      

   QGridLayout * gl_files = new QGridLayout( 0 ); gl_files->setContentsMargins( 0, 0, 0, 0 ); gl_files->setSpacing( 0 );
   {
      unsigned int j = 0;
      gl_files->addWidget( lbl_files, j, 0 ); j++;
      gl_files->addLayout( hbl_dir, j, 0 ); j++;
      gl_files->addLayout( hbl_file_buttons_0, j, 0 ); j++;
      gl_files->addLayout( hbl_file_buttons, j, 0 ); j++;
      gl_files->addLayout( hbl_file_buttons_1 , j, 0 ); j++;
      gl_files->addWidget( lb_files , j, 0 ); j++;
      gl_files->addWidget( lbl_selected, j, 0 ); j++;
      gl_files->addLayout( hbl_file_buttons_2 , j, 0 ); j++;
      gl_files->addLayout( hbl_file_buttons_2b , j, 0 ); j++;
      gl_files->addLayout( hbl_file_buttons_3 , j, 0 ); j++;
      gl_files->addLayout( hbl_file_buttons_3b , j, 0 ); j++;
      gl_files->addLayout( hbl_file_buttons_4 , j, 0 ); j++;
      gl_files->addLayout( hbl_conc_file, j, 0 ); j++;
      // gl_files->addWidget( lbl_conc_file, j, 0 ); j++;
      //       gl_files->addLayout( hbl_dad, j, 0 ); j++;
      //       gl_files->addLayout( hbl_empty, j, 0 ); j++;
      //       gl_files->addLayout( hbl_signal, j, 0 ); j++;
      gl_files->addWidget( lbl_created_files , j, 0 ); j++;
      gl_files->addWidget( le_created_dir , j, 0 ); j++;
      gl_files->addWidget( lb_created_files, j, 0 ); j++;
      gl_files->addWidget( lbl_selected_created, j, 0 ); j++;
      gl_files->addLayout( hbl_created , j, 0 ); j++;
      gl_files->addLayout( hbl_created_2, j, 0 ); j++;
      gl_files->addLayout( vbl_model , j, 0 ); j++;
      gl_files->addLayout( vbl_editor_group , j, 0 ); j++;
   }

   QGridLayout * gl_wheel = new QGridLayout(0); gl_wheel->setContentsMargins( 0, 0, 0, 0 ); gl_wheel->setSpacing( 0 );
   gl_wheel->addWidget         ( lbl_blank1          , 0, 0 );
   gl_wheel->addWidget         ( pb_wheel_dec        , 0, 1 );
   gl_wheel->addWidget( qwtw_wheel           , 0 , 2 , 1 + ( 0 ) - ( 0 ) , 1 + ( 8  ) - ( 2 ) );
   gl_wheel->addWidget         ( pb_wheel_inc        , 0, 9 );
   gl_wheel->addWidget         ( lbl_wheel_pos       , 0, 10 );
   
   QGridLayout * gl_wheel_extra = new QGridLayout( 0 ); gl_wheel_extra->setContentsMargins( 0, 0, 0, 0 ); gl_wheel_extra->setSpacing( 0 );

   gl_wheel_extra->addWidget( lbl_blank2         , 0, 0 );
   gl_wheel_extra->addWidget( lbl_wheel_pos_below, 0, 1 );
   gl_wheel_extra->addWidget( lbl_wheel_Pcolor   , 0, 2 );

   // gl_wheel->addWidget( pb_timeshift  , 0 , 0 , 1 + ( 0 ) - ( 0 ) , 1 + ( 1  ) - ( 0 ) );
   // gl_wheel->addWidget         ( lbl_wheel_pos  , 0, 2 );
   // gl_wheel->addWidget( qwtw_wheel      , 0 , 3 , 1 + ( 0 ) - ( 0 ) , 1 + ( 7  ) - ( 3 ) );
   // gl_wheel->addWidget         ( pb_ref         , 0, 8 );
   // gl_wheel->addWidget         ( pb_errors      , 0, 9 );
   // gl_wheel->addWidget         ( pb_wheel_cancel, 0, 10 );
   // gl_wheel->addWidget         ( pb_wheel_save  , 0, 11 );

   QBoxLayout * hbl_top = new QHBoxLayout(); hbl_top->setContentsMargins( 0, 0, 0, 0 ); hbl_top->setSpacing( 0 );
   hbl_top->addWidget( pb_p3d );
   hbl_top->addWidget( pb_ref );
   hbl_top->addWidget( pb_guinier_plot_rg );
   // hbl_top->addWidget( pb_guinier_plot_mw );
   hbl_top->addWidget( pb_errors );
   hbl_top->addWidget( pb_cormap );
   // hbl_top->addWidget( pb_pp );
   hbl_top->addWidget( pb_ggqfit );
   hbl_top->addWidget( pb_wheel_cancel );
   hbl_top->addWidget( pb_wheel_save );

   // QBoxLayout * hbl_mode = new QHBoxLayout(); hbl_mode->setContentsMargins( 0, 0, 0, 0 ); hbl_mode->setSpacing( 0 );
   // hbl_mode->addWidget( pb_gauss_start );
   // hbl_mode->addWidget( pb_ggauss_start );
   // hbl_mode->addWidget( pb_baseline_start );
   // hbl_mode->addWidget( pb_baseline_apply );
   // hbl_mode->addWidget( pb_wheel_start );
   // hbl_mode->addWidget( pb_scale );
   // hbl_mode->addWidget( pb_rgc );
   // hbl_mode->addWidget( pb_pm );
   // hbl_mode->addWidget( pb_testiq );
   // hbl_mode->addWidget( pb_guinier );
   // hbl_mode->addWidget( pb_wyatt_start );
   // hbl_mode->addWidget( pb_wyatt_apply );

   QBoxLayout * hbl_mode0 = new QHBoxLayout(); hbl_mode0->setContentsMargins( 0, 0, 0, 0 ); hbl_mode0->setSpacing( 0 );
   hbl_mode0->addWidget( pb_blanks_start );
   hbl_mode0->addWidget( pb_baseline_start );
   hbl_mode0->addWidget( pb_baseline_test );
   hbl_mode0->addWidget( pb_baseline_apply );
   // hbl_mode0->addWidget( pb_timeshift );
   // hbl_mode0->addWidget( pb_timescale );
   // hbl_mode0->addWidget( pb_wyatt_start );
   // hbl_mode0->addWidget( pb_wyatt_apply );

   QBoxLayout * hbl_mode = new QHBoxLayout(); hbl_mode->setContentsMargins( 0, 0, 0, 0 ); hbl_mode->setSpacing( 0 );
   hbl_mode->addWidget( pb_gauss_mode );
   hbl_mode->addWidget( pb_gauss_start );
   hbl_mode->addWidget( pb_ggauss_start );
   hbl_mode->addWidget( pb_scale );
   if ( U_EXPT )
   {
      hbl_mode->addWidget( pb_rgc );
      hbl_mode->addWidget( pb_simulate );
      hbl_mode->addWidget( pb_pm );
   }
   hbl_mode->addWidget( pb_testiq );
   hbl_mode->addWidget( pb_guinier );

   // scale

   QBoxLayout * vbl_scale = new QVBoxLayout( 0 ); vbl_scale->setContentsMargins( 0, 0, 0, 0 ); vbl_scale->setSpacing( 0 );
   {
      QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( cb_scale_scroll );
      hbl->addWidget( lbl_scale_q_range );
      hbl->addWidget( le_scale_q_start );
      hbl->addWidget( le_scale_q_end );
      hbl->addWidget( pb_scale_q_reset );
      vbl_scale->addLayout( hbl );
   }      

   {
      QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
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
   QBoxLayout * vbl_testiq = new QVBoxLayout( 0 ); vbl_testiq->setContentsMargins( 0, 0, 0, 0 ); vbl_testiq->setSpacing( 0 );
   {
      QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
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
   QBoxLayout * hbl_guinier_resid = new QHBoxLayout(); hbl_guinier_resid->setContentsMargins( 0, 0, 0, 0 ); hbl_guinier_resid->setSpacing( 0 );
   hbl_guinier_resid->addWidget( rb_guinier_resid_diff );
   hbl_guinier_resid->addWidget( rb_guinier_resid_sd );
   hbl_guinier_resid->addWidget( rb_guinier_resid_pct );

   QBoxLayout * vbl_guinier = new QVBoxLayout( 0 ); vbl_guinier->setContentsMargins( 0, 0, 0, 0 ); vbl_guinier->setSpacing( 0 );
   {
      QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( lbl_guinier_rg_t_range );
      hbl->addWidget( le_guinier_rg_t_start );
      hbl->addWidget( le_guinier_rg_t_end );
      hbl->addWidget( lbl_guinier_rg_rg_range );
      hbl->addWidget( le_guinier_rg_rg_start );
      hbl->addWidget( le_guinier_rg_rg_end );
      hbl->addWidget( cb_guinier_lock_rg_range );
      hbl->addWidget( pb_guinier_replot );
      vbl_guinier->addLayout( hbl );
   }      

   // powerfit

   QBoxLayout * vbl_powerfit = new QVBoxLayout( 0 ); vbl_powerfit->setContentsMargins( 0, 0, 0, 0 ); vbl_powerfit->setSpacing( 0 );
   {
      QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( lbl_powerfit_a );
      hbl->addWidget( le_powerfit_a );
      hbl->addWidget( lbl_powerfit_b );
      hbl->addWidget( le_powerfit_b );
      hbl->addWidget( lbl_powerfit_c );
      hbl->addWidget( le_powerfit_c );
      hbl->addWidget( le_powerfit_c_min );
      hbl->addWidget( le_powerfit_c_max );
      hbl->addWidget( lbl_powerfit_q_range );
      hbl->addWidget( le_powerfit_q_start );
      hbl->addWidget( le_powerfit_q_end );
      vbl_powerfit->addLayout( hbl );
   }

   {
      QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( lbl_powerfit_fit_curve );
      hbl->addWidget( lbl_powerfit_fit_epsilon );
      hbl->addWidget( le_powerfit_fit_epsilon );
      hbl->addWidget( lbl_powerfit_fit_iterations );
      hbl->addWidget( le_powerfit_fit_iterations );
      hbl->addWidget( lbl_powerfit_fit_max_calls );
      hbl->addWidget( le_powerfit_fit_max_calls );
      hbl->addWidget( cb_powerfit_fit_curve );
      hbl->addWidget( cb_powerfit_fit_alg );
      hbl->addWidget( cb_powerfit_fit_alg_weight );
      hbl->addWidget( pb_powerfit_fit );
      hbl->addWidget( pb_powerfit_reset );
      hbl->addWidget( pb_powerfit_create_adjusted_curve );
      vbl_powerfit->addLayout( hbl );
   }


   // {
   //    QGridLayout * gl = new QGridLayout(); gl->setContentsMargins( 0, 0, 0, 0 ); gl->setSpacing( 0 );
   //    int row = 0;
   //    int col = 0;
      
   //    gl->addWidget( lbl_powerfit_lambda          , row, col++ );
   //    gl->addWidget( le_powerfit_lambda           , row, col++ );
   //    gl->addWidget( lbl_powerfit_lambda2         , row, col++ );
   //    gl->addWidget( le_powerfit_lambda2          , row, col++ );
   //    gl->addWidget( lbl_powerfit_extinction_coef , row, col++ );
   //    gl->addWidget( le_powerfit_extinction_coef  , row, col++ );
   //    gl->addWidget( lbl_powerfit_msg             , row, row, col, col + 5 ); col += 5 + 1;

   //    ++row;
   //    col = 0;

   //    gl->addWidget( lbl_powerfit_lambda_abs      , row, col++ );
   //    gl->addWidget( le_powerfit_lambda_abs       , row, col++ );
   //    gl->addWidget( lbl_powerfit_lambda2_abs     , row, col++ );
   //    gl->addWidget( le_powerfit_lambda2_abs      , row, col++ );
   //    gl->addWidget( lbl_powerfit_uncorrected_conc, row, row, col, col + 3 ); col += 3 + 1;
   //    {
   //       QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
   //       hbl->addWidget( le_powerfit_uncorrected_conc );
   //       hbl->addWidget( lbl_powerfit_scat_conc );
   //       hbl->addWidget( le_powerfit_scat_conc );
   //       hbl->addWidget( lbl_powerfit_computed_conc );
   //       hbl->addWidget( le_powerfit_computed_conc );

   //       gl->addLayout( hbl, row, row, col, col + 4 ); col += 4 + 1;
   //    }

   //    vbl_powerfit->addLayout( gl );
   // }
      
   {
      QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( lbl_powerfit_lambda );
      hbl->addWidget( le_powerfit_lambda );
      hbl->addWidget( lbl_powerfit_lambda_abs );
      hbl->addWidget( le_powerfit_lambda_abs );
      hbl->addWidget( lbl_powerfit_lambda2 );
      hbl->addWidget( le_powerfit_lambda2 );
      hbl->addWidget( lbl_powerfit_lambda2_abs );
      hbl->addWidget( le_powerfit_lambda2_abs );
      hbl->addWidget( lbl_powerfit_extinction_coef );
      hbl->addWidget( le_powerfit_extinction_coef );
      hbl->addWidget( cb_powerfit_dispersion_correction );
      hbl->addWidget( lbl_powerfit_msg );
      vbl_powerfit->addLayout( hbl );
   }
   {
      QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( lbl_powerfit_uncorrected_conc );
      hbl->addWidget( le_powerfit_uncorrected_conc );
      hbl->addWidget( lbl_powerfit_scat_conc );
      hbl->addWidget( le_powerfit_scat_conc );
      hbl->addWidget( lbl_powerfit_computed_conc );
      hbl->addWidget( le_powerfit_computed_conc );
      vbl_powerfit->addLayout( hbl );
   }
   // powerfit end
   // baseline2 start

   QBoxLayout * vbl_baseline2 = new QVBoxLayout( 0 ); vbl_baseline2->setContentsMargins( 0, 0, 0, 0 ); vbl_baseline2->setSpacing( 0 );

   {
      QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( lbl_baseline2_q_range );
      hbl->addWidget( le_baseline2_q_start );
      hbl->addWidget( le_baseline2_q_end );
      hbl->addWidget( pb_baseline2_fit );
      hbl->addWidget( pb_baseline2_create_adjusted_curve );
      vbl_baseline2->addLayout( hbl );
   }

   {
      QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( lbl_baseline2_msg );
      vbl_baseline2->addLayout( hbl );
   }

   // baseline2 end
   {
      QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( lbl_guinier_mw_t_range );
      hbl->addWidget( le_guinier_mw_t_start );
      hbl->addWidget( le_guinier_mw_t_end );
      hbl->addWidget( lbl_guinier_mw_mw_range );
      hbl->addWidget( le_guinier_mw_mw_start );
      hbl->addWidget( le_guinier_mw_mw_end );
      hbl->addWidget( cb_guinier_lock_mw_range );
      hbl->addWidget( pb_guinier_mw_replot );
      vbl_guinier->addLayout( hbl );
   }      

   {
      QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( cb_guinier_scroll );
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
      hbl->addWidget( cb_guinier_qrgmax );
      hbl->addWidget( le_guinier_qrgmax );
      vbl_guinier->addLayout( hbl );
   }      
   vbl_guinier->addWidget( lbl_guinier_stats );

   // QBoxLayout * vbl_guinier_plots = new QVBoxLayout( 0 ); vbl_guinier_plots->setContentsMargins( 0, 0, 0, 0 ); vbl_guinier_plots->setSpacing( 0 );
   // vbl_guinier_plots->addWidget( guinier_plot );
   // vbl_guinier_plots->addWidget( guinier_plot_errors );

   // pm
   QBoxLayout * vbl_pm = new QVBoxLayout( 0 ); vbl_pm->setContentsMargins( 0, 0, 0, 0 ); vbl_pm->setSpacing( 0 );
   {
      QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( lbl_pm_q_range );
      hbl->addWidget( le_pm_q_start );
      hbl->addWidget( le_pm_q_end );
      hbl->addWidget( pb_pm_q_reset );
      vbl_pm->addLayout( hbl );
   }      
   {
      QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( rb_pm_shape_sphere );
      hbl->addWidget( rb_pm_shape_spheroid );
      hbl->addWidget( rb_pm_shape_ellipsoid );
      hbl->addWidget( rb_pm_shape_cylinder );
      hbl->addWidget( rb_pm_shape_torus );
      vbl_pm->addLayout( hbl );
   }

   {
      QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( cb_pm_sd );
      hbl->addWidget( cb_pm_q_logbin );
      hbl->addWidget( lbl_pm_q_pts );
      hbl->addWidget( le_pm_q_pts );
      hbl->addWidget( lbl_pm_grid_size );
      hbl->addWidget( le_pm_grid_size );
      vbl_pm->addLayout( hbl );
   }      
   {
      QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( lbl_pm_samp_e_dens );
      hbl->addWidget( le_pm_samp_e_dens );
      hbl->addWidget( lbl_pm_buff_e_dens );
      hbl->addWidget( le_pm_buff_e_dens );
      hbl->addWidget( pb_pm_run );
      vbl_pm->addLayout( hbl );
   }      

   // rgc
   QBoxLayout * vbl_rgc = new QVBoxLayout( 0 ); vbl_rgc->setContentsMargins( 0, 0, 0, 0 ); vbl_rgc->setSpacing( 0 );
   {
      QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( lbl_rgc_mw );
      hbl->addWidget( le_rgc_mw );
      hbl->addWidget( lbl_rgc_vol );
      hbl->addWidget( le_rgc_vol );
      hbl->addWidget( lbl_rgc_rho );
      hbl->addWidget( le_rgc_rho );
      vbl_rgc->addLayout( hbl );
   }      
   {
      QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      
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
      QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( lbl_rgc_rg );
      hbl->addWidget( le_rgc_rg );
      hbl->addWidget( lbl_rgc_extents );
      hbl->addWidget( le_rgc_extents );
      hbl->addWidget( lbl_rgc_g_qrange );
      hbl->addWidget( le_rgc_g_qrange );
      vbl_rgc->addLayout( hbl );
   }      

   QBoxLayout * hbl_ggauss_scroll = new QHBoxLayout(); hbl_ggauss_scroll->setContentsMargins( 0, 0, 0, 0 ); hbl_ggauss_scroll->setSpacing( 0 );
   {
      hbl_ggauss_scroll->addWidget( cb_ggauss_scroll );
      hbl_ggauss_scroll->addWidget( cb_ggauss_scroll_p_green );
      hbl_ggauss_scroll->addWidget( cb_ggauss_scroll_p_yellow );
      hbl_ggauss_scroll->addWidget( cb_ggauss_scroll_p_red );
      hbl_ggauss_scroll->addWidget( cb_ggauss_scroll_smoothed );
      hbl_ggauss_scroll->addWidget( cb_ggauss_scroll_oldstyle );
      hbl_ggauss_scroll->addWidget( pb_ggauss_results );
      hbl_ggauss_scroll->addWidget( pb_ggauss_as_curves );
   }
      
   QGridLayout * gl_gauss = new QGridLayout(0); gl_gauss->setContentsMargins( 0, 0, 0, 0 ); gl_gauss->setSpacing( 0 );
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

   QGridLayout * gl_gauss2 = new QGridLayout(0); gl_gauss2->setContentsMargins( 0, 0, 0, 0 ); gl_gauss2->setSpacing( 0 );
   { 
      int ofs = 1;
      gl_gauss2->addWidget         ( cb_sd_weight            , 0, ofs++ );
      gl_gauss2->addWidget         ( cb_fix_width            , 0, ofs++ );
      gl_gauss2->addWidget         ( cb_fix_dist1            , 0, ofs++ );
      gl_gauss2->addWidget         ( cb_fix_dist2            , 0, ofs++ );
      gl_gauss2->addWidget         ( pb_gauss_fit            , 0, ofs++ );
      gl_gauss2->addWidget         ( pb_ggauss_rmsd          , 0, ofs++ );
      gl_gauss2->addWidget         ( lbl_gauss_fit           , 0, ofs++ );
      gl_gauss2->addWidget         ( le_gauss_fit_start      , 0, ofs++ );
      gl_gauss2->addWidget         ( le_gauss_fit_end        , 0, ofs++ );

      gl_gauss2->addWidget         ( le_gauss_local_pts      , 0, ofs++ );
      gl_gauss2->addWidget         ( pb_gauss_local_caruanas , 0, ofs++ );
      gl_gauss2->addWidget         ( pb_gauss_local_guos     , 0, ofs++ );

      gl_gauss2->addWidget         ( pb_gauss_as_curves      , 0, ofs++ );
   }

   QHBoxLayout * hbl_baseline = new QHBoxLayout(); hbl_baseline->setContentsMargins( 0, 0, 0, 0 ); hbl_baseline->setSpacing( 0 );
   // hbl_baseline->addWidget( pb_baseline_start   );
   hbl_baseline->addWidget( pb_bb_cm_inc );
   // hbl_baseline->addWidget( pb_blanks_params );
   hbl_baseline->addWidget( cb_baseline_start_zero );
   hbl_baseline->addWidget( le_baseline_start_s );
   hbl_baseline->addWidget( le_baseline_start   );
   hbl_baseline->addWidget( le_baseline_start_e );
   hbl_baseline->addWidget( le_baseline_end_s   );
   hbl_baseline->addWidget( le_baseline_end     );
   hbl_baseline->addWidget( le_baseline_end_e   );
   hbl_baseline->addWidget( cb_baseline_fix_width  );
   hbl_baseline->addWidget( le_baseline_width  );
   hbl_baseline->addWidget( pb_baseline_best );
   //   hbl_baseline->addWidget( pb_baseline_apply   );

   QHBoxLayout * hbl_wyatt = new QHBoxLayout(); hbl_wyatt->setContentsMargins( 0, 0, 0, 0 ); hbl_wyatt->setSpacing( 0 );
   hbl_baseline->addWidget( le_wyatt_start   );
   hbl_baseline->addWidget( le_wyatt_end     );
   hbl_baseline->addWidget( le_wyatt_start2  );
   hbl_baseline->addWidget( le_wyatt_end2    );
   hbl_baseline->addWidget( cb_wyatt_2       );

   QBoxLayout * hbl_ggqfit_plot_ctls = new QHBoxLayout(); hbl_ggqfit_plot_ctls->setContentsMargins( 0, 0, 0, 0 ); hbl_ggqfit_plot_ctls->setSpacing( 0 );
   hbl_ggqfit_plot_ctls->addWidget( cb_ggq_plot_chi2 );
   hbl_ggqfit_plot_ctls->addWidget( cb_ggq_plot_P );

   vbl_plot_group = new QVBoxLayout(0); vbl_plot_group->setContentsMargins( 0, 0, 0, 0 ); vbl_plot_group->setSpacing( 0 );
   // vbl_plot_group->addWidget ( plot_dist );
   // vbl_plot_group->addWidget ( plot_ref );
   // vbl_plot_group->addLayout ( vbl_guinier_plots );
   vbl_plot_group->addLayout ( l_pbmode );
   vbl_plot_group->addLayout ( l_pbmode_main );
   vbl_plot_group->addLayout ( l_pbmode_sel );
   vbl_plot_group->addLayout ( l_pbmode_crop );
   vbl_plot_group->addLayout ( l_pbmode_conc );
   vbl_plot_group->addLayout ( l_pbmode_sd );
   vbl_plot_group->addLayout ( l_pbmode_fasta );
   vbl_plot_group->addLayout ( l_pbmode_dad );
   vbl_plot_group->addLayout ( l_pbmode_q_exclude );
   // vbl_plot_group->addWidget ( lbl_dad_lambdas_data, 0, Qt::AlignCenter ); // don't like this
   vbl_plot_group->addWidget ( lbl_dad_lambdas_data );
   vbl_plot_group->addWidget ( lbl_q_exclude_detail );
   vbl_plot_group->addWidget ( qs_plots );
   vbl_plot_group->addLayout ( l_plot_errors );
   vbl_plot_group->addWidget ( ggqfit_plot );
   vbl_plot_group->addLayout ( hbl_ggqfit_plot_ctls );
   vbl_plot_group->addLayout ( hbl_guinier_resid );
   vbl_plot_group->addLayout ( gl_wheel );
   vbl_plot_group->addLayout ( gl_wheel_extra );
   vbl_plot_group->addWidget ( lbl_mode_title );
   vbl_plot_group->addLayout ( hbl_top );
   vbl_plot_group->addLayout ( hbl_mode0 );
   vbl_plot_group->addLayout ( hbl_mode );
   vbl_plot_group->addLayout ( vbl_scale );
   vbl_plot_group->addLayout ( vbl_powerfit );
   vbl_plot_group->addLayout ( vbl_baseline2 );
   vbl_plot_group->addLayout ( vbl_testiq );
   vbl_plot_group->addLayout ( vbl_guinier );
   vbl_plot_group->addLayout ( vbl_rgc );
   vbl_plot_group->addLayout ( vbl_pm );
   vbl_plot_group->addLayout ( hbl_ggauss_scroll );
   vbl_plot_group->addLayout ( gl_gauss );
   // vbl_plot_group->addLayout ( hbl_gauss2 );
   vbl_plot_group->addLayout ( gl_gauss2  );
   vbl_plot_group->addLayout ( hbl_baseline );
   vbl_plot_group->addLayout ( hbl_wyatt );

   vbl_plot_group->addWidget ( le_dummy );


//    QBoxLayout * hbl_files_plot = new QHBoxLayout(); hbl_files_plot->setContentsMargins( 0, 0, 0, 0 ); hbl_files_plot->setSpacing( 0 );
//    // hbl_files_plot->addLayout( vbl_files );
//    hbl_files_plot->addLayout( gl_files );
//    hbl_files_plot->addLayout( vbl_plot_group );

   QGridLayout * gl_files_plot = new QGridLayout( 0 );
   // QHBoxLayout * gl_files_plot = new QHBoxLayout( 0 );
   
   gl_files_plot->setContentsMargins( 0, 0, 0, 0 );
   gl_files_plot->setSpacing( 0 );

   // gl_files_plot->addLayout( gl_files, 0 );
   // gl_files_plot->addLayout( vbl_plot_group, 5 );
   // gl_files_plot->setStretch( 0, 0 );
   // gl_files_plot->setStretch( 1, 1 );
   
   gl_files_plot->addLayout( gl_files      , 0, 0 );
   gl_files_plot->addLayout( vbl_plot_group, 0, 1, -1, 5 );

   // below does not seem to have any effect
   gl_files_plot->setColumnStretch( 0, 1 );
   gl_files_plot->setColumnStretch( 1, 10 );


   // {
   //    QWidget* placeholder = new QWidget;
   //    placeholder->setLayout( gl_files );
   //    //    setCentralWidget( placeholder );
   //    placeholder->setMaximumWidth( 300 );
   // }

   QGridLayout * gl_bottom = new QGridLayout( 0 ); gl_bottom->setContentsMargins( 0, 0, 0, 0 ); gl_bottom->setSpacing( 0 );
   gl_bottom->addWidget( pb_help     , 0, 0 );
   gl_bottom->addWidget( pb_options  , 0, 1 );
   gl_bottom->addWidget( progress    , 0, 2 );
   gl_bottom->addWidget( pb_cancel   , 0, 3 );
   
   QVBoxLayout * background = new QVBoxLayout(this); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );
   background->addWidget ( lbl_title );
   background->addSpacing( 1 );
   // background->addSpacing( 1 );
   background->addLayout ( gl_files_plot );
   background->addSpacing( 1 );
   background->addLayout ( gl_bottom );
   background->addSpacing( 1 );

   //   hide_widgets( files_widgets, 
   //                 !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "dad_files_widgets" ) || ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_files_widgets" ] == "false" ? false : true );
   //   hide_widgets( editor_widgets, 
   //                 !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "dad_editor_widgets" ) || ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_editor_widgets" ] == "false" ? false : true );
   //   hide_widgets( created_files_widgets,
   //                 !( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "dad_created_files_widgets" ) || ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_creaded_files_widgets" ] == "false" ? false : true );

   always_hide_widgets.insert( {
         pb_test_i_of_t
            ,pb_create_ihash_t
            ,pb_create_istar_q
            ,pb_blanks_start
            ,pb_baseline_start
            ,pb_baseline_apply
            ,rb_pbmode_q_exclude
            ,cb_powerfit_fit_curve
            ,cb_powerfit_fit_alg
            ,pb_gauss_mode
            ,pb_gauss_start
            ,pb_ggauss_start
            ,pb_testiq
            ,pb_guinier
            ,pb_scale
            ,pb_load_conc
            ,pb_conc
            ,pb_normalize
            ,pb_to_saxs
            ,pb_ref
            ,rb_pbmode_conc
            ,pb_movie
            // ,lbl_powerfit_lambda_abs
            // ,lbl_powerfit_lambda2_abs
            }
      );

   ShowHide::hide_widgets( always_hide_widgets );
   
   mode_setup_widgets();
}

void US_Hydrodyn_Dad::mode_setup_widgets()
{

   // pb_row_widgets;
   {
      vector < QWidget * > tmp_widgets;
      tmp_widgets.push_back( pb_blanks_start );
      tmp_widgets.push_back( pb_baseline_start );
      tmp_widgets.push_back( pb_baseline_apply );
      // tmp_widgets.push_back( pb_timeshift );
      // tmp_widgets.push_back( pb_timescale );
      // tmp_widgets.push_back( pb_wyatt_start );
      // tmp_widgets.push_back( pb_wyatt_apply );

      pb_row_widgets.push_back( tmp_widgets );
   }

   {
      vector < QWidget * > tmp_widgets;
      tmp_widgets.push_back( pb_gauss_mode );
      tmp_widgets.push_back( pb_gauss_start );
      tmp_widgets.push_back( pb_ggauss_start );
      tmp_widgets.push_back( pb_scale );
      if ( U_EXPT )
      {
         tmp_widgets.push_back( pb_rgc );
         tmp_widgets.push_back( pb_simulate );
         tmp_widgets.push_back( pb_pm );
      }
      tmp_widgets.push_back( pb_testiq );
      tmp_widgets.push_back( pb_guinier );

      pb_row_widgets.push_back( tmp_widgets );
   }

   // plot_widgets;
// #ifndef qt4
//    plot_widgets.push_back( le_dummy );
// #endif
//    plot_widgets.push_back( pb_select_vis );
//    plot_widgets.push_back( pb_remove_vis );
//    plot_widgets.push_back( pb_crop_common );
//    plot_widgets.push_back( pb_crop_vis );
//    plot_widgets.push_back( pb_crop_to_vis );
//    plot_widgets.push_back( pb_crop_zero );
//    plot_widgets.push_back( pb_crop_left );
//    plot_widgets.push_back( pb_crop_undo );
//    plot_widgets.push_back( pb_crop_right );
//    plot_widgets.push_back( pb_legend );

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

#if defined( ALLOW_GUOS_CARUANAS )   
   gaussian_widgets.push_back( le_gauss_local_pts );
   gaussian_widgets.push_back( pb_gauss_local_caruanas );
   gaussian_widgets.push_back( pb_gauss_local_guos );
#endif
   
   gaussian_widgets.push_back( pb_gauss_as_curves );
   gaussian_widgets.push_back( lbl_blank1 );
   gaussian_widgets.push_back( pb_wheel_dec );
   gaussian_widgets.push_back( qwtw_wheel );
   gaussian_widgets.push_back( pb_wheel_inc );
   gaussian_widgets.push_back( lbl_wheel_pos );

   // ggaussian_widgets;

   ggaussian_widgets.push_back( cb_ggauss_scroll );
   ggaussian_widgets.push_back( cb_ggauss_scroll_p_green );
   ggaussian_widgets.push_back( cb_ggauss_scroll_p_yellow );
   ggaussian_widgets.push_back( cb_ggauss_scroll_p_red );
   ggaussian_widgets.push_back( cb_ggauss_scroll_smoothed );
   ggaussian_widgets.push_back( cb_ggauss_scroll_oldstyle );
   ggaussian_widgets.push_back( pb_ggqfit );
   ggaussian_widgets.push_back( ggqfit_plot );
   ggaussian_widgets.push_back( cb_ggq_plot_chi2 );
   ggaussian_widgets.push_back( cb_ggq_plot_P );
   ggaussian_widgets.push_back( pb_gauss_prev );
   ggaussian_widgets.push_back( lbl_gauss_pos );
   ggaussian_widgets.push_back( pb_gauss_next );
   ggaussian_widgets.push_back( cb_fix_width );
   ggaussian_widgets.push_back( cb_sd_weight );
   ggaussian_widgets.push_back( pb_gauss_fit );
   ggaussian_widgets.push_back( le_gauss_pos );
   ggaussian_widgets.push_back( le_gauss_pos_width );
   // ggaussian_widgets.push_back( le_gauss_fit_start );
   // ggaussian_widgets.push_back( le_gauss_fit_end );
   ggaussian_widgets.push_back( pb_ggauss_rmsd );
   ggaussian_widgets.push_back( lbl_gauss_fit );
   ggaussian_widgets.push_back( pb_ggauss_results );
   ggaussian_widgets.push_back( pb_gauss_save );
   ggaussian_widgets.push_back( pb_ggauss_as_curves );
   ggaussian_widgets.push_back( lbl_blank1 );
   ggaussian_widgets.push_back( pb_wheel_dec );
   ggaussian_widgets.push_back( qwtw_wheel );
   ggaussian_widgets.push_back( pb_wheel_inc );
   ggaussian_widgets.push_back( lbl_wheel_pos );
   ggaussian_widgets.push_back( lbl_blank2 );
   ggaussian_widgets.push_back( lbl_wheel_Pcolor );
   ggaussian_widgets.push_back( lbl_wheel_pos_below );

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

   // ggqfit_widgets;
   ggqfit_widgets.push_back( ggqfit_plot );
   ggqfit_widgets.push_back( cb_ggq_plot_chi2 );
   ggqfit_widgets.push_back( cb_ggq_plot_P );

   // wheel_below_widgets;
   wheel_below_widgets.push_back( lbl_blank2 );
   wheel_below_widgets.push_back( lbl_wheel_pos_below );
   wheel_below_widgets.push_back( lbl_wheel_Pcolor );

   // powerfit_widgets;
   powerfit_widgets.push_back( pb_wheel_dec );
   powerfit_widgets.push_back( qwtw_wheel );
   powerfit_widgets.push_back( pb_wheel_inc );
   powerfit_widgets.push_back( lbl_wheel_pos );
   
   powerfit_widgets.push_back( lbl_powerfit_msg );
   powerfit_widgets.push_back( lbl_powerfit_q_range );
   powerfit_widgets.push_back( le_powerfit_q_start );
   powerfit_widgets.push_back( le_powerfit_q_end );
   
   powerfit_widgets.push_back( lbl_powerfit_a );
   powerfit_widgets.push_back( le_powerfit_a );
   powerfit_widgets.push_back( lbl_powerfit_b );
   powerfit_widgets.push_back( le_powerfit_b );
   powerfit_widgets.push_back( lbl_powerfit_c );
   powerfit_widgets.push_back( le_powerfit_c );
   powerfit_widgets.push_back( le_powerfit_c_min );
   powerfit_widgets.push_back( le_powerfit_c_max );

   powerfit_widgets.push_back( lbl_powerfit_fit_epsilon );
   powerfit_widgets.push_back( le_powerfit_fit_epsilon );
   powerfit_widgets.push_back( lbl_powerfit_fit_iterations );
   powerfit_widgets.push_back( le_powerfit_fit_iterations );
   powerfit_widgets.push_back( lbl_powerfit_fit_max_calls );
   powerfit_widgets.push_back( le_powerfit_fit_max_calls );

   powerfit_widgets.push_back( lbl_powerfit_uncorrected_conc );
   powerfit_widgets.push_back( le_powerfit_uncorrected_conc );
   powerfit_widgets.push_back( lbl_powerfit_scat_conc );
   powerfit_widgets.push_back( le_powerfit_scat_conc );
   powerfit_widgets.push_back( lbl_powerfit_computed_conc );
   powerfit_widgets.push_back( le_powerfit_computed_conc );

   powerfit_widgets.push_back( lbl_powerfit_lambda );
   powerfit_widgets.push_back( le_powerfit_lambda );
   powerfit_widgets.push_back( lbl_powerfit_lambda_abs );
   powerfit_widgets.push_back( le_powerfit_lambda_abs );
   powerfit_widgets.push_back( lbl_powerfit_extinction_coef );
   powerfit_widgets.push_back( le_powerfit_extinction_coef );

   powerfit_widgets.push_back( cb_powerfit_dispersion_correction );

   powerfit_widgets.push_back( lbl_powerfit_lambda2 );
   powerfit_widgets.push_back( le_powerfit_lambda2 );
   powerfit_widgets.push_back( lbl_powerfit_lambda2_abs );
   powerfit_widgets.push_back( le_powerfit_lambda2_abs );

   powerfit_widgets.push_back( pb_powerfit_fit );
   powerfit_widgets.push_back( pb_powerfit_reset );
   powerfit_widgets.push_back( pb_powerfit_create_adjusted_curve );
   powerfit_widgets.push_back( lbl_powerfit_fit_curve );
   powerfit_widgets.push_back( cb_powerfit_fit_curve );
   powerfit_widgets.push_back( cb_powerfit_fit_alg );
   powerfit_widgets.push_back( cb_powerfit_fit_alg_weight );

   // baseline2_widgets

   baseline2_widgets.push_back( pb_wheel_dec );
   baseline2_widgets.push_back( qwtw_wheel );
   baseline2_widgets.push_back( pb_wheel_inc );
   baseline2_widgets.push_back( lbl_wheel_pos );
   
   baseline2_widgets.push_back( lbl_baseline2_msg );
   baseline2_widgets.push_back( lbl_baseline2_q_range );
   baseline2_widgets.push_back( le_baseline2_q_start );
   baseline2_widgets.push_back( le_baseline2_q_end );
   
   baseline2_widgets.push_back( pb_baseline2_fit );
   baseline2_widgets.push_back( pb_baseline2_create_adjusted_curve );
   
   // wyatt_widgets;

   wyatt_widgets.push_back( le_wyatt_start );
   wyatt_widgets.push_back( le_wyatt_end );
   wyatt_widgets.push_back( le_wyatt_start2 );
   wyatt_widgets.push_back( le_wyatt_end2 );
   wyatt_widgets.push_back( cb_wyatt_2 );
   wyatt_widgets.push_back( lbl_blank1 );
   wyatt_widgets.push_back( pb_wheel_dec );
   wyatt_widgets.push_back( qwtw_wheel );
   wyatt_widgets.push_back( pb_wheel_inc );
   wyatt_widgets.push_back( lbl_wheel_pos );

   // blanks_widgets;

   if ( started_in_expert_mode ) {
      blanks_widgets.push_back( pb_bb_cm_inc );
   }
   // blanks_widgets.push_back( pb_blanks_params );
   blanks_widgets.push_back( le_baseline_end_s );
   blanks_widgets.push_back( le_baseline_end_e );
   blanks_widgets.push_back( lbl_blank1 );
   blanks_widgets.push_back( pb_wheel_dec );
   blanks_widgets.push_back( qwtw_wheel );
   blanks_widgets.push_back( pb_wheel_inc );
   blanks_widgets.push_back( lbl_wheel_pos );

   // baseline_widgets;

   if ( started_in_expert_mode ) {
      baseline_widgets.push_back( pb_bb_cm_inc );
   }
   baseline_widgets.push_back( pb_baseline_test );
   baseline_widgets.push_back( cb_baseline_start_zero );
   baseline_widgets.push_back( le_baseline_start_s );
   baseline_widgets.push_back( le_baseline_start );
   baseline_widgets.push_back( le_baseline_start_e );
   baseline_widgets.push_back( le_baseline_end_s );
   baseline_widgets.push_back( le_baseline_end );
   baseline_widgets.push_back( le_baseline_end_e );
   baseline_widgets.push_back( cb_baseline_fix_width );
   baseline_widgets.push_back( le_baseline_width );
   baseline_widgets.push_back( pb_baseline_best );
   // baseline_widgets.push_back( pb_baseline_apply );
   baseline_widgets.push_back( pb_baseline_test );
   baseline_widgets.push_back( lbl_blank1 );
   baseline_widgets.push_back( pb_wheel_dec );
   baseline_widgets.push_back( qwtw_wheel );
   baseline_widgets.push_back( pb_wheel_inc );
   baseline_widgets.push_back( lbl_wheel_pos );
   baseline_widgets.push_back( lbl_blank2 );
   baseline_widgets.push_back( lbl_wheel_Pcolor );
   baseline_widgets.push_back( lbl_wheel_pos_below );

   // scale_widgets;

   scale_widgets.push_back( lbl_scale_low_high );
   scale_widgets.push_back( rb_scale_low );
   scale_widgets.push_back( rb_scale_high );
   // scale_widgets.push_back( cb_scale_sd );
   scale_widgets.push_back( cb_scale_scroll );
   scale_widgets.push_back( cb_scale_save_intp );
   scale_widgets.push_back( lbl_scale_q_range );
   scale_widgets.push_back( le_scale_q_start );
   scale_widgets.push_back( le_scale_q_end );
   scale_widgets.push_back( pb_scale_q_reset );
   scale_widgets.push_back( pb_scale_apply );
   scale_widgets.push_back( pb_scale_reset );
   scale_widgets.push_back( pb_scale_create );
   scale_widgets.push_back( lbl_blank1 );
   scale_widgets.push_back( pb_wheel_dec );
   scale_widgets.push_back( qwtw_wheel );
   scale_widgets.push_back( pb_wheel_inc );
   scale_widgets.push_back( lbl_wheel_pos );
   scale_widgets.push_back( lbl_blank2 );
   scale_widgets.push_back( lbl_wheel_Pcolor );
   scale_widgets.push_back( lbl_wheel_pos_below );

   // timeshift_widgets;

   timeshift_widgets.push_back( lbl_blank1 );
   timeshift_widgets.push_back( pb_wheel_dec );
   timeshift_widgets.push_back( qwtw_wheel );
   timeshift_widgets.push_back( pb_wheel_inc );
   timeshift_widgets.push_back( lbl_wheel_pos );
   timeshift_widgets.push_back( le_dummy );
   // timeshift_widgets.push_back( pb_select_vis );
   // timeshift_widgets.push_back( pb_remove_vis );
   // timeshift_widgets.push_back( pb_crop_common );
   // timeshift_widgets.push_back( pb_crop_vis );
   // timeshift_widgets.push_back( pb_crop_to_vis );
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
   testiq_widgets.push_back( pb_wheel_dec );
   testiq_widgets.push_back( qwtw_wheel );
   testiq_widgets.push_back( pb_wheel_inc );
   testiq_widgets.push_back( lbl_wheel_pos );

   // guinier_widgets;

   guinier_widgets.push_back( cb_guinier_scroll );
   // guinier_widgets.push_back( lbl_guinier_q_range );
   // guinier_widgets.push_back( le_guinier_q_start );
   // guinier_widgets.push_back( le_guinier_q_end );
   guinier_widgets.push_back( lbl_guinier_q2_range );
   guinier_widgets.push_back( le_guinier_q2_start );
   guinier_widgets.push_back( le_guinier_q2_end );
   guinier_widgets.push_back( lbl_guinier_delta_range );
   guinier_widgets.push_back( le_guinier_delta_start );
   guinier_widgets.push_back( le_guinier_delta_end );
   guinier_widgets.push_back( cb_guinier_qrgmax );
   guinier_widgets.push_back( le_guinier_qrgmax );
   guinier_widgets.push_back( cb_guinier_sd );
   guinier_widgets.push_back( guinier_plot );
   guinier_widgets.push_back( guinier_plot_rg );
   guinier_widgets.push_back( guinier_plot_mw );
   guinier_widgets.push_back( guinier_plot_errors );
   guinier_widgets.push_back( rb_guinier_resid_diff );
   guinier_widgets.push_back( rb_guinier_resid_sd );
   guinier_widgets.push_back( rb_guinier_resid_pct );
   guinier_widgets.push_back( lbl_guinier_stats );
   guinier_widgets.push_back( lbl_blank1 );
   guinier_widgets.push_back( pb_wheel_dec );
   guinier_widgets.push_back( qwtw_wheel );
   guinier_widgets.push_back( pb_wheel_inc );
   guinier_widgets.push_back( lbl_wheel_pos );
   guinier_widgets.push_back( pb_guinier_plot_rg );
   // guinier_widgets.push_back( pb_guinier_plot_mw );
   guinier_widgets.push_back( lbl_guinier_rg_t_range );
   guinier_widgets.push_back( le_guinier_rg_t_start );
   guinier_widgets.push_back( le_guinier_rg_t_end );
   guinier_widgets.push_back( lbl_guinier_rg_rg_range );
   guinier_widgets.push_back( le_guinier_rg_rg_start );
   guinier_widgets.push_back( le_guinier_rg_rg_end );
   guinier_widgets.push_back( cb_guinier_lock_rg_range );
   guinier_widgets.push_back( pb_guinier_replot );
   guinier_widgets.push_back( guinier_plot_mw );
   guinier_widgets.push_back( lbl_guinier_mw_t_range );
   guinier_widgets.push_back( le_guinier_mw_t_start );
   guinier_widgets.push_back( le_guinier_mw_t_end );
   guinier_widgets.push_back( lbl_guinier_mw_mw_range );
   guinier_widgets.push_back( le_guinier_mw_mw_start );
   guinier_widgets.push_back( le_guinier_mw_mw_end );
   guinier_widgets.push_back( cb_guinier_lock_mw_range );
   guinier_widgets.push_back( pb_guinier_mw_replot );
   guinier_widgets.push_back( lbl_blank2 );
   guinier_widgets.push_back( lbl_wheel_Pcolor );
   guinier_widgets.push_back( lbl_wheel_pos_below );

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
   guinier_rg_widgets.push_back( pb_guinier_replot );

   // not a "mode"
   guinier_mw_widgets.push_back( guinier_plot_mw );
   guinier_mw_widgets.push_back( lbl_guinier_mw_t_range );
   guinier_mw_widgets.push_back( le_guinier_mw_t_start );
   guinier_mw_widgets.push_back( le_guinier_mw_t_end );
   guinier_mw_widgets.push_back( lbl_guinier_mw_mw_range );
   guinier_mw_widgets.push_back( le_guinier_mw_mw_start );
   guinier_mw_widgets.push_back( le_guinier_mw_mw_end );
   guinier_mw_widgets.push_back( cb_guinier_lock_mw_range );
   guinier_mw_widgets.push_back( pb_guinier_mw_replot );

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
   rgc_widgets.push_back( lbl_rgc_g_qrange );
   rgc_widgets.push_back( le_rgc_g_qrange );

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
   pm_widgets.push_back( pb_wheel_dec );
   pm_widgets.push_back( qwtw_wheel );
   pm_widgets.push_back( pb_wheel_inc );
   pm_widgets.push_back( lbl_wheel_pos );
   pm_widgets.push_back( le_dummy );
}   

void US_Hydrodyn_Dad::mode_select()
{
   // QSize cur_size = plot_dist->size();

   ShowHide::hide_widgets( plot_widgets, always_hide_widgets  );
   ShowHide::hide_widgets( gaussian_widgets, always_hide_widgets  );
   ShowHide::hide_widgets( gaussian_4var_widgets, always_hide_widgets  );
   ShowHide::hide_widgets( gaussian_5var_widgets, always_hide_widgets  );
   ShowHide::hide_widgets( ggaussian_widgets, always_hide_widgets  );
   ShowHide::hide_widgets( ggaussian_4var_widgets, always_hide_widgets  );
   ShowHide::hide_widgets( ggaussian_5var_widgets, always_hide_widgets  );
   ShowHide::hide_widgets( powerfit_widgets, always_hide_widgets );
   ShowHide::hide_widgets( baseline2_widgets, always_hide_widgets );
   ShowHide::hide_widgets( wyatt_widgets, always_hide_widgets  );
   ShowHide::hide_widgets( blanks_widgets, always_hide_widgets  );
   ShowHide::hide_widgets( baseline_widgets, always_hide_widgets  );
   ShowHide::hide_widgets( scale_widgets, always_hide_widgets  );
   ShowHide::hide_widgets( timeshift_widgets, always_hide_widgets  );
   ShowHide::hide_widgets( testiq_widgets, always_hide_widgets  );
   ShowHide::hide_widgets( guinier_widgets, always_hide_widgets  );
   ShowHide::hide_widgets( rgc_widgets, always_hide_widgets  );
   ShowHide::hide_widgets( pm_widgets, always_hide_widgets  );
   ShowHide::hide_widgets( ggqfit_widgets, always_hide_widgets  );
   ShowHide::hide_widgets( wheel_below_widgets, always_hide_widgets  );

   switch ( current_mode )
   {
   case MODE_NORMAL    : 
      {
         lbl_wheel_pos->setText( "" ); 
         mode_title( "" ); 
         ShowHide::hide_widgets( plot_widgets, always_hide_widgets, false ); 
         for ( int i = 0; i < (int) pb_row_widgets.size(); ++i )
         {
            ShowHide::hide_widgets( pb_row_widgets[ i ], always_hide_widgets, false );
         }
      }
      break;

   case MODE_GAUSSIAN  : 
      {
         mode_title( pb_gauss_start->text() );
         ShowHide::hide_widgets( gaussian_widgets, always_hide_widgets, false );
         switch ( gaussian_type_size )
         {
         case 4 : ShowHide::hide_widgets( gaussian_4var_widgets, always_hide_widgets, false ); break;
         case 5 : ShowHide::hide_widgets( gaussian_5var_widgets, always_hide_widgets, false ); break;
         default : break;
         }
         ShowHide::only_widgets( pb_row_widgets, 1, always_hide_widgets );
      }
      break;

   case MODE_GGAUSSIAN : 
      {
         mode_title( pb_ggauss_start->text() );
         ShowHide::hide_widgets( ggaussian_widgets, always_hide_widgets, false );
         switch ( gaussian_type_size )
         {
         case 4 : ShowHide::hide_widgets( ggaussian_4var_widgets, always_hide_widgets, false ); break;
         case 5 : ShowHide::hide_widgets( ggaussian_5var_widgets, always_hide_widgets, false ); break;
         default : break;
         }            
         ShowHide::only_widgets( pb_row_widgets, 1, always_hide_widgets );
      }
      break;

   case MODE_WYATT     : mode_title( pb_wyatt_start->text() );    ShowHide::hide_widgets( wyatt_widgets, always_hide_widgets      , false ); ShowHide::only_widgets( pb_row_widgets, 0, always_hide_widgets  );break;
   case MODE_BLANKS    : mode_title( pb_blanks_start->text() );   ShowHide::hide_widgets( blanks_widgets, always_hide_widgets     , false ); ShowHide::only_widgets( pb_row_widgets, 0, always_hide_widgets  );break;
   case MODE_BASELINE  : mode_title( pb_baseline_start->text() ); ShowHide::hide_widgets( baseline_widgets, always_hide_widgets   , false ); ShowHide::only_widgets( pb_row_widgets, 0, always_hide_widgets  );break;
   case MODE_TIMESHIFT : mode_title( pb_timeshift->text() );      ShowHide::hide_widgets( timeshift_widgets, always_hide_widgets  , false ); ShowHide::only_widgets( pb_row_widgets, 0, always_hide_widgets  );break;
   case MODE_SCALE     : mode_title( pb_scale->text() );          ShowHide::hide_widgets( scale_widgets, always_hide_widgets      , false ); ShowHide::only_widgets( pb_row_widgets, 1, always_hide_widgets  );break;
   case MODE_TESTIQ    : mode_title( pb_testiq->text() );         ShowHide::hide_widgets( testiq_widgets, always_hide_widgets     , false ); ShowHide::only_widgets( pb_row_widgets, 1, always_hide_widgets  );break;
   case MODE_GUINIER   : mode_title( pb_guinier->text() );        ShowHide::hide_widgets( guinier_widgets, always_hide_widgets    , false ); ShowHide::only_widgets( pb_row_widgets, 1, always_hide_widgets  );break;
   case MODE_RGC       : mode_title( pb_rgc->text() );            ShowHide::hide_widgets( rgc_widgets, always_hide_widgets        , false ); ShowHide::only_widgets( pb_row_widgets, 1, always_hide_widgets  );break;
   case MODE_PM        : mode_title( pb_pm->text() );             ShowHide::hide_widgets( pm_widgets, always_hide_widgets         , false ); ShowHide::only_widgets( pb_row_widgets, 1, always_hide_widgets  );break;
   case MODE_POWERFIT  : mode_title( pb_powerfit->text() );       ShowHide::hide_widgets( powerfit_widgets, always_hide_widgets   , false ); ShowHide::only_widgets( pb_row_widgets, 9, always_hide_widgets  );break;
   case MODE_BASELINE2 : mode_title( pb_baseline2_start->text() );ShowHide::hide_widgets( baseline2_widgets, always_hide_widgets  , false ); ShowHide::only_widgets( pb_row_widgets, 9, always_hide_widgets  );break;
   default : us_qdebug( "mode select error" ); break;
   }
   // plot_dist->resize( cur_size );
   resize( size() );
}

void US_Hydrodyn_Dad::mode_select( modes mode )
{
   current_mode = mode;
   mode_select();
}

void US_Hydrodyn_Dad::mode_title( QString title )
{
   lbl_mode_title->setText( title );
   if ( title.isEmpty() )
   {
      lbl_mode_title->hide();
   } else {
      lbl_mode_title->show();
   }
}

void US_Hydrodyn_Dad::update_enables()
{
   // qDebug() << "::update_enables()";
   resize_plots();

   if ( running ) {
      // qDebug() << "::update_enables() running, early exit";
      plot_dist->enableAxis( QwtPlot::xBottom, !plot_errors->isVisible() );

      if ( current_mode == MODE_PM )
      {
         model_enables();
         pm_enables();
      //    us_qdebug( "model_enables in update_enables (running)\n" );
      // } else {
      //    us_qdebug( "update_enables return (running)\n" );
      }
      return;
   }

   for ( auto const & widget : pbmode_q_exclude_widgets ) {
      widget->setEnabled( true );
   }
   q_exclude_update_lbl();

   // cout << "update_enables\n";

   // cout << "US_Hydrodyn_Dad::update_enables()\n";
   // cout << QString("saxs_window->qsl_plotted_iq_names.size() %1\n").arg(saxs_window->qsl_plotted_iq_names.size());

   pb_add_files          ->setEnabled( true );
   pb_add_dir            ->setEnabled( true );
   pb_regex_load         ->setEnabled( true );
   pb_options            ->setEnabled( true );

   lb_files              ->setEnabled( true );
   lb_created_files      ->setEnabled( true );

   unsigned int files_selected_count                      = 0;
   // unsigned int non_dad_non_empty_files_selected_count = 0;
   // unsigned int last_selected_pos                         = 0;
   unsigned int conc_selected_count                       = 0;

   map < QString, bool > selected_map;

   QStringList selected_files;

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         selected_files << lb_files->item( i )->text();
         selected_map[ lb_files->item( i )->text() ] = true;
         // last_selected_pos = i;
         last_selected_file = lb_files->item( i )->text();
         files_selected_count++;
         if ( conc_files.count( lb_files->item( i )->text() ) )
         {
            conc_selected_count++;
         }
         //          if ( lb_files->item( i )->text() != lbl_dad->text() &&
         //               lb_files->item( i )->text() != lbl_empty->text() )
         //          {
         //             non_dad_non_empty_files_selected_count++;
         //          }
      }
   }

   bool all_rt           = files_selected_count && files_selected_count == (unsigned int) selected_files.filter( "_Rt_q" ).size();
   bool all_ihasht       = files_selected_count && files_selected_count == (unsigned int) selected_files.filter( "_Ihasht_q" ).size();
   bool all_ihashq       = files_selected_count && files_selected_count == (unsigned int) selected_files.filter( "_Ihashq_" ).size();
   bool all_istarq       = files_selected_count && files_selected_count == (unsigned int) selected_files.filter( "_Istarq_" ).size();
   bool all_UV_Vis          = files_selected_count && files_selected_count == (unsigned int) selected_files.filter( "_UV-Vis_" ).size();
   bool all_atl          = files_selected_count && files_selected_count == (unsigned int) selected_files.filter( "_At_L" ).size();
   bool any_blc          = files_selected_count && files_selected_count == (unsigned int) selected_files.filter( "_blc" ).size();

   bool files_compatible = compatible_files( selected_files );
   bool files_are_time   = type_files      ( selected_files );
   //   bool one_conc_file    = files_selected_count == 1 && files_are_time && conc_files.count( last_selected_file );
   // if ( files_selected_count == 1 )
   // {
   //    us_qdebug( QString( "last_selected_file: %1\nconc_files: %2" ).arg( last_selected_file ).arg( conc_files.size() ) );
   //    for ( set < QString >::iterator it = conc_files.begin();
   //          it != conc_files.end();
   //          ++it )
   //    {
   //       us_qdebug( QString( "conc file: %1").arg( *it ) );
   //    }
   // }
         

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

   baseline_integral = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_bl_integral" ] == "true";

   // pb_timeshift        ->setEnabled( files_selected_count > 0 && files_compatible && files_are_time );
   pb_timeshift          ->setEnabled( (int64_t)files_selected_count - conc_selected_count > 0 && files_compatible && files_are_time && conc_files.size() );
   pb_timescale          ->setEnabled( files_selected_count && files_are_time && conc_selected_count == files_selected_count );
   pb_gauss_mode         ->setEnabled( files_selected_count == 1 && files_are_time );
   pb_gauss_start        ->setEnabled( files_selected_count == 1 && files_are_time );
   pb_ggauss_start       ->setEnabled( files_selected_count > 1 && files_are_time && gaussians.size() );
   cb_sd_weight          ->setEnabled( files_selected_count && files_are_time && gaussians.size() );
   cb_fix_width          ->setEnabled( files_selected_count && files_are_time && gaussians.size() && U_EXPT );
   cb_fix_dist1          ->setEnabled( files_selected_count && files_are_time && gaussians.size() && U_EXPT );
   cb_fix_dist2          ->setEnabled( files_selected_count && files_are_time && gaussians.size() && U_EXPT );
   pb_blanks_start       ->setEnabled( default_blanks.size() || ( files_selected_count > 1 && !files_are_time && files_compatible ) );
   pb_baseline_start     ->setEnabled( !files_are_time && files_compatible && files_selected_count == 1 );
                                       
   pb_baseline_apply     ->setEnabled( files_selected_count && 
                                       files_are_time && 
                                       le_baseline_start->text().toDouble() < le_baseline_end->text().toDouble() &&
                                       !baseline_integral
                                       // ( !baseline_integral || baseline_ready_to_apply ) 
                                       );
   pb_wyatt_start        ->setEnabled( files_selected_count == 1 && files_are_time );
   pb_wyatt_apply        ->setEnabled( files_selected_count && 
                                       files_are_time && 
                                       le_wyatt_start->text().toDouble() < le_wyatt_end->text().toDouble() );

   pb_similar_files      ->setEnabled( files_selected_count == 1 );
   pb_conc               ->setEnabled( lb_files->count() > 0 );
   pb_clear_files        ->setEnabled( files_selected_count > 0 );
   // pb_conc_avg           ->setEnabled( all_selected_have_nonzero_conc() && files_compatible && !files_are_time );
   pb_normalize          ->setEnabled( all_selected_have_nonzero_conc() && files_compatible && !files_are_time );
   pb_add                ->setEnabled( files_selected_count > 1 && files_compatible );
   pb_avg                ->setEnabled( files_selected_count > 1 && files_compatible && !files_are_time );
   pb_bin                ->setEnabled( files_selected_count && files_compatible /* && !files_are_time */ );
   pb_smooth             ->setEnabled( files_selected_count );
   pb_powerfit           ->setEnabled( !files_are_time && files_selected_count == 1 );
   pb_baseline2_start    ->setEnabled( files_are_time && files_selected_count == 1 && all_atl && all_UV_Vis );
   pb_baseline2_apply    ->setEnabled( files_are_time && files_selected_count && all_atl && all_UV_Vis && baseline2_fit_ok && !any_blc );
   pb_svd                ->setEnabled( files_selected_count > 1 && files_compatible ); // && !files_are_time );
   pb_create_i_of_t      ->setEnabled( files_selected_count > 1 && files_compatible && !files_are_time );
   pb_test_i_of_t        ->setEnabled( files_selected_count && files_compatible && files_are_time );
   pb_create_i_of_q      ->setEnabled( files_selected_count > 1 && files_compatible && files_are_time && all_atl && all_UV_Vis /* && gaussians.size() */ );
   pb_create_ihash_t     ->setEnabled( files_selected_count > 1 && files_compatible && files_are_time && dad_param_n && dad_param_lambda && dad_param_g_dndc && selected_files.count() == selected_files.filter( "_Rt_" ).count() );
   pb_create_istar_q     ->setEnabled( files_selected_count > 1 && files_compatible && files_are_time && all_ihasht );
   pb_load_conc          ->setEnabled( true );
   // pb_repeak             ->setEnabled( files_selected_count > 1 && files_compatible && files_are_time );
   pb_repeak             ->setEnabled( files_are_time && conc_files.size() &&
                                       ( ( files_selected_count == 1 && !conc_selected_count ) ||
                                         ( files_selected_count == 2 && conc_selected_count == 1 ) )
                                       );
   pb_conc_file          ->setEnabled( conc_selected_count == 1 );
   // pb_detector           ->setEnabled( true );

   //                                        );
   //    pb_set_dad           ->setEnabled( files_selected_count == 1 && 
   //                                        lb_files->item( last_selected_pos )->text() != lbl_dad->text() &&
   //                                        lb_files->item( last_selected_pos )->text() != lbl_empty ->text() &&
   //                                        lb_files->item( last_selected_pos )->text() != lbl_signal->text()
   //                                        );
   //    pb_set_signal         ->setEnabled( files_selected_count == 1 && 
   //                                        lb_files->item( last_selected_pos )->text() != lbl_dad->text() &&
   //                                        lb_files->item( last_selected_pos )->text() != lbl_empty ->text() &&
   //                                        lb_files->item( last_selected_pos )->text() != lbl_signal->text() );
   //    pb_set_empty          ->setEnabled( files_selected_count == 1 && 
   //                                        lb_files->item( last_selected_pos )->text() != lbl_dad->text() &&
   //                                        lb_files->item( last_selected_pos )->text() != lbl_empty ->text() &&
   //                                        lb_files->item( last_selected_pos )->text() != lbl_signal->text() );
   pb_select_all         ->setEnabled( lb_files->count() > 0 );
   pb_select_nth         ->setEnabled( lb_files->count() > 2 );
   pb_invert             ->setEnabled( lb_files->count() > 0 );
   pb_line_width         ->setEnabled( files_selected_count );
   pb_color_rotate       ->setEnabled( files_selected_count );
   //    pb_join               ->setEnabled( files_selected_count == 2 && files_compatible && !files_are_time );
   // pb_adjacent           ->setEnabled( lb_files->count() > 1 );
   pb_to_saxs            ->setEnabled( files_selected_count && files_compatible && !files_are_time );
   pb_view               ->setEnabled( files_selected_count && files_selected_count <= 10 );
   pb_movie              ->setEnabled( files_selected_count > 1 );
   cb_eb                 ->setEnabled( files_selected_count > 0 && files_selected_count < 10 );
   cb_dots               ->setEnabled( files_selected_count > 0 );
   pb_rescale            ->setEnabled( files_selected_count > 0 );
   pb_rescale_y          ->setEnabled( files_selected_count > 0 );
   pb_ag                 ->setEnabled( files_selected_count == 1 && files_compatible && !files_are_time );

   pb_select_all_created ->setEnabled( lb_created_files->count() > 0 );
   pb_invert_all_created ->setEnabled( lb_created_files->count() > 0 );
   pb_adjacent_created   ->setEnabled( lb_created_files->count() > 1 );
   pb_remove_created     ->setEnabled( files_created_selected_count > 0 );
   pb_save_created_csv   ->setEnabled( files_created_selected_count > 0 && files_compatible );
   pb_save_created       ->setEnabled( files_created_selected_not_saved_count > 0 );

   pb_show_created       ->setEnabled( files_created_selected_not_shown_count > 0 );
   pb_show_only_created  ->setEnabled( files_created_selected_count > 0 &&
                                       files_selected_not_created > 0 );

   pb_q_exclude_vis      ->setEnabled( files_selected_count && files_compatible && !files_are_time );
   pb_q_exclude_left     ->setEnabled( files_selected_count && files_compatible && !files_are_time );
   pb_q_exclude_right    ->setEnabled( files_selected_count && files_compatible && !files_are_time );
   
   // #define DEBUG_SCALING
#if defined( DEBUG_SCALING )
   {
      QTextStream tso( stdout );

      tso << "--------------------------------------------------------------------------------\n";
      tso << "plot_dist" << "\n";
      tso << "--------------------------------------------------------------------------------\n";
      tso << QString().sprintf(
                               "plot_dist->axisScaleDiv( QwtPlot::xBottom ).lower,upperBound()     %g\t%g\n"
                               "plot_dist->axisScaleDiv( QwtPlot::yLeft ).lower,upperBound()       %g\t%g\n"

                               ,plot_dist->axisScaleDiv( QwtPlot::xBottom ).lowerBound()
                               ,plot_dist->axisScaleDiv( QwtPlot::xBottom ).upperBound()
                               ,plot_dist->axisScaleDiv( QwtPlot::yLeft ).lowerBound()
                               ,plot_dist->axisScaleDiv( QwtPlot::yLeft ).upperBound()
                               );

      tso << "zoomrect "
          << plot_dist_zoomer->zoomRect().left() << " , "
          << plot_dist_zoomer->zoomRect().right() << " : " 
          << plot_dist_zoomer->zoomRect().bottom() << " , "
          << plot_dist_zoomer->zoomRect().top()
          << "\n"
         ;
      tso << "zoombase "
          << plot_dist_zoomer->zoomBase().left() << " , "
          << plot_dist_zoomer->zoomBase().right() << " : " 
          << plot_dist_zoomer->zoomBase().bottom() << " , "
          << plot_dist_zoomer->zoomBase().top()
          << "\n"
         ;
      tso << "--------------------------------------------------------------------------------\n";
      tso << "plot_errors" << "\n";
      tso << "--------------------------------------------------------------------------------\n";
      tso << QString().sprintf(
                               "plot_errors->axisScaleDiv( QwtPlot::xBottom ).lower,upperBound()     %g\t%g\n"
                               "plot_errors->axisScaleDiv( QwtPlot::yLeft ).lower,upperBound()       %g\t%g\n"

                               ,plot_errors->axisScaleDiv( QwtPlot::xBottom ).lowerBound()
                               ,plot_errors->axisScaleDiv( QwtPlot::xBottom ).upperBound()
                               ,plot_errors->axisScaleDiv( QwtPlot::yLeft ).lowerBound()
                               ,plot_errors->axisScaleDiv( QwtPlot::yLeft ).upperBound()
                               );


      tso << "zoomrect "
          << plot_errors_zoomer->zoomRect().left() << " , "
          << plot_errors_zoomer->zoomRect().right() << " : " 
          << plot_errors_zoomer->zoomRect().bottom() << " , "
          << plot_errors_zoomer->zoomRect().top()
          << "\n"
         ;
      tso << "zoombase "
          << plot_errors_zoomer->zoomBase().left() << " , "
          << plot_errors_zoomer->zoomBase().right() << " : " 
          << plot_errors_zoomer->zoomBase().bottom() << " , "
          << plot_errors_zoomer->zoomBase().top()
          << "\n"
         ;
   }   
#endif

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
   pb_crop_to_vis      ->setEnabled( 
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
   pb_legend           ->setEnabled( lb_files->count() && files_selected_count <= 20 && !cb_eb->isChecked() );
   pb_axis_x           ->setEnabled( lb_files->count() );
   pb_axis_y           ->setEnabled( lb_files->count() );

   pb_scale            ->setEnabled( files_selected_count > 1 && files_compatible );
   pb_rgc              ->setEnabled( true );
   pb_simulate         ->setEnabled( files_selected_count && files_compatible && !files_are_time );
   pb_pm               ->setEnabled( files_selected_count == 1 && files_compatible && !files_are_time );
   pb_testiq           ->setEnabled( files_selected_count > 4 && files_compatible && files_are_time );
   pb_guinier          ->setEnabled( files_selected_count && files_compatible && !files_are_time && all_istarq );

   // cb_guinier          ->setEnabled( files_selected_count );
   legend_set();

   pb_stack_push_all   ->setEnabled( lb_files->count() );
   pb_stack_push_sel   ->setEnabled( files_selected_count );
   pb_stack_pcopy      ->setEnabled( files_selected_count  && clipboard.files.size() );
   pb_stack_copy       ->setEnabled( files_selected_count );
   pb_stack_paste      ->setEnabled( clipboard.files.size() );
   pb_stack_drop       ->setEnabled( stack_data.size() );
   pb_stack_join       ->setEnabled( stack_data.size() );
   pb_stack_rot_up     ->setEnabled( stack_data.size() > 1 );
   pb_stack_rot_down   ->setEnabled( stack_data.size() > 1 );
   pb_stack_swap       ->setEnabled( stack_data.size() );

   pb_pp               ->setEnabled( true );

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
      resize_plots();
   }

   // if ( files_selected_count > 1 && files_compatible )
   // {
   //    pb_cormap           ->setEnabled( true );
   // } else {
   pb_cormap           ->setEnabled( false );
   // }

   pb_ggqfit           ->setEnabled( false );

   pb_save_state       ->setEnabled( ( lb_files->count() || stack_data.size() ) && !files_created_selected_not_saved_count );

   pb_p3d              ->setEnabled( files_selected_count > 1 && files_compatible && files_are_time );

   {
      QString title;
      if ( !files_compatible )
      {
         title = us_tr( "q [1/Angstrom] or Time [a.u.]" );
      } else {
         if ( type_files( selected_files ) )
         {
            if ( all_UV_Vis ) {
               title = us_tr( "Time [s]" );
            } else {
               title = us_tr( "Time [a.u.]" );
            }
         } else if ( all_UV_Vis ) {
            title = us_tr( UNICODE_LAMBDA_QS + " [nm]" );
         } else {
            title = us_tr( "q [1/Angstrom]" );
         }
      }
      if ( axis_x_log )
      {
         plot_dist  ->setAxisTitle(QwtPlot::xBottom,  title + us_tr(" (log scale)") );
         plot_errors->setAxisTitle(QwtPlot::xBottom,  title + us_tr(" (log scale)") );

      } else {
         plot_dist  ->setAxisTitle(QwtPlot::xBottom,  title );
         plot_errors->setAxisTitle(QwtPlot::xBottom,  title );
      }
      if ( !files_selected_count ) {
         plot_dist  ->setAxisTitle(QwtPlot::xBottom,  "" );
         plot_errors->setAxisTitle(QwtPlot::xBottom,  "" );
      }
      plot_dist->enableAxis( QwtPlot::xBottom, !plot_errors->isVisible() );
      // qDebug() << "::update_enables() plot_errors is " << ( plot_errors->isVisible() ? "visible" : "not visible" );
   }
   {
      QString title;
      if ( !files_compatible ) {
         title = us_tr( "Intensity [a.u.]" );
      } else {
         if ( type_files( selected_files ) ) {
            if ( all_rt ) {
               title = us_tr( "R(q, t) [cm^-1]" );
            } else if ( all_ihasht ) {
               title = us_tr( "I#(t) [g^2 cm^-3 mol^-1]" );
            } else if ( all_UV_Vis ) {
               title = us_tr( "A(t) [a.u.]" );
            } else {
               title = us_tr( "I(t) [a.u.]" );
            }
         } else {
            if ( all_istarq ) {
               title = us_tr( "I*(q) [g mol^-1]" );
            } else if ( all_ihashq ) {
               title = us_tr( "I#(q) [g^2 cm^-3 mol^-1]" );
            } else if ( all_UV_Vis ) {
               title = us_tr( "A(" + UNICODE_LAMBDA_QS + ") [a.u.]" );
            } else {
               title = us_tr( "I(q) [a.u.]" );
            }
         }
      }
      if ( axis_y_log ) {
         plot_dist->setAxisTitle(QwtPlot::yLeft, title + us_tr( " (log scale)") );
      } else {
         plot_dist->setAxisTitle(QwtPlot::yLeft, title );
      }
      if ( !files_selected_count ) {
         plot_dist->setAxisTitle(QwtPlot::yLeft, "" );
      }
   }
   model_enables();

   if ( cb_eb->isChecked() &&
        !cb_eb->isEnabled() )
   {
      cb_eb->setChecked( false );
      set_eb();
   }
   if ( le_last_focus ) {
      le_last_focus->setFocus();
   }
}

void US_Hydrodyn_Dad::model_enables()
{
   if ( lb_model_files->count() )
   {
      ShowHide::hide_widgets( model_widgets, always_hide_widgets, false );
      pb_model_select_all->setEnabled( true );
      lb_model_files     ->setEnabled( true );
      bool any_model_selected           = false;
      bool any_model_selected_not_saved = false;
      for ( int i = 0; i < (int) lb_model_files->count(); ++i )
      {
         if ( lb_model_files->item( i )->isSelected() )
         {
            any_model_selected = true;
            if ( models_not_saved.count( lb_model_files->item( i )->text() ) )
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
      ShowHide::hide_widgets( model_widgets, always_hide_widgets, true );
   }      
}

void US_Hydrodyn_Dad::disable_all()
{
   // cout << "disable all\n";
   // qwtw_wheel            ->setEnabled( false );
   pb_wheel_inc          ->setEnabled( false );
   pb_wheel_dec          ->setEnabled( false );
   pb_wheel_save         ->setEnabled( false );

   pb_similar_files      ->setEnabled( false );
   pb_conc               ->setEnabled( false );
   pb_clear_files        ->setEnabled( false );
   pb_avg                ->setEnabled( false );
   pb_normalize          ->setEnabled( false );
   pb_conc_avg           ->setEnabled( false );
   pb_bin                ->setEnabled( false );
   pb_smooth             ->setEnabled( false );
   pb_repeak             ->setEnabled( false );
   pb_powerfit           ->setEnabled( false );
   pb_baseline2_start    ->setEnabled( false );
   pb_baseline2_apply    ->setEnabled( false );
   pb_svd                ->setEnabled( false );
   pb_create_i_of_t      ->setEnabled( false );
   pb_create_i_of_q      ->setEnabled( false );
   pb_create_ihash_t     ->setEnabled( false );
   pb_load_conc          ->setEnabled( false );
   pb_conc_file          ->setEnabled( false );
   // pb_detector           ->setEnabled( false );
   //    pb_set_dad           ->setEnabled( false );
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
   cb_eb                 ->setEnabled( false );
   cb_dots               ->setEnabled( false );
   pb_rescale            ->setEnabled( false );
   pb_rescale_y          ->setEnabled( false );
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
   pb_crop_to_vis        ->setEnabled( false ); 
   pb_crop_zero          ->setEnabled( false ); 
   pb_crop_left          ->setEnabled( false ); 
   pb_crop_undo          ->setEnabled( false );
   pb_crop_right         ->setEnabled( false ); 
   for ( auto const & widget : pbmode_q_exclude_widgets ) {
      widget->setEnabled( false );
   }

   pb_legend             ->setEnabled( false );
   pb_axis_x             ->setEnabled( false );
   pb_axis_y             ->setEnabled( false );

   pb_add_files          ->setEnabled( false );
   pb_add_dir            ->setEnabled( false );
   pb_regex_load         ->setEnabled( false );

   lb_files              ->setEnabled( false );
   lb_created_files      ->setEnabled( false );

   pb_timeshift          ->setEnabled( false );
   pb_timescale          ->setEnabled( false );

   pb_gauss_mode         ->setEnabled( false );
   pb_gauss_start        ->setEnabled( false );
   pb_gauss_clear        ->setEnabled( false );
   pb_gauss_new          ->setEnabled( false );
   pb_gauss_delete       ->setEnabled( false );
   pb_gauss_prev         ->setEnabled( false );
   pb_gauss_next         ->setEnabled( false );
   pb_gauss_fit          ->setEnabled( false );
   pb_gauss_save         ->setEnabled( false );

   le_gauss_local_pts      ->setEnabled( false );
   pb_gauss_local_caruanas ->setEnabled( false );
   pb_gauss_local_guos     ->setEnabled( false );

   pb_wheel_cancel       ->setEnabled( false );

   le_gauss_pos          ->clearFocus();
   le_gauss_pos_width    ->clearFocus();
   le_gauss_pos_height   ->clearFocus();
   le_gauss_pos_dist1    ->clearFocus();
   le_gauss_pos_dist2    ->clearFocus();
   le_gauss_fit_start    ->clearFocus();
   le_gauss_fit_end      ->clearFocus();

   le_gauss_pos          ->setEnabled( false );
   le_gauss_pos_width    ->setEnabled( false );
   le_gauss_pos_height   ->setEnabled( false );
   le_gauss_pos_dist1    ->setEnabled( false );
   le_gauss_pos_dist2    ->setEnabled( false );
   le_gauss_fit_start    ->setEnabled( false );
   le_gauss_fit_end      ->setEnabled( false );

   pb_wyatt_start        ->setEnabled( false );
   le_wyatt_start        ->clearFocus();
   le_wyatt_end          ->clearFocus();
   le_wyatt_start        ->setEnabled( false );
   le_wyatt_end          ->setEnabled( false );
   pb_wyatt_apply        ->setEnabled( false );

   pb_blanks_start       ->setEnabled( false );
   // pb_blanks_params      ->setEnabled( false );
   pb_bb_cm_inc          ->setEnabled( false );

   pb_baseline_start     ->setEnabled( false );

   le_baseline_start_s   ->clearFocus();
   le_baseline_start     ->clearFocus();
   le_baseline_start_e   ->clearFocus();
   le_baseline_end_s     ->clearFocus();
   le_baseline_end       ->clearFocus();
   le_baseline_end_e     ->clearFocus();
   le_baseline_width     ->clearFocus();

   le_baseline_start_s   ->setEnabled( false );
   le_baseline_start     ->setEnabled( false );
   le_baseline_start_e   ->setEnabled( false );
   le_baseline_end_s     ->setEnabled( false );
   le_baseline_end       ->setEnabled( false );
   le_baseline_end_e     ->setEnabled( false );
   le_baseline_width     ->setEnabled( false );
   pb_baseline_apply     ->setEnabled( false );
   pb_baseline_test      ->setEnabled( false );
   pb_baseline_best      ->setEnabled( false );

   pb_ggauss_start       ->setEnabled( false );
   pb_ggauss_rmsd        ->setEnabled( false );
   pb_ggauss_results     ->setEnabled( false );

   pb_gauss_as_curves    ->setEnabled( false );
   pb_ggauss_as_curves   ->setEnabled( false );
   cb_sd_weight          ->setEnabled( false );
   cb_fix_width          ->setEnabled( false );
   cb_fix_dist1          ->setEnabled( false );
   cb_fix_dist2          ->setEnabled( false );

   pb_ref                ->setEnabled( false );
   pb_errors             ->setEnabled( false );
   pb_cormap             ->setEnabled( false );
   pb_ggqfit             ->setEnabled( false );

   cb_ggq_plot_chi2      ->setEnabled( false );
   cb_ggq_plot_P         ->setEnabled( false );

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

   pb_pp                 ->setEnabled( false );

   pb_scale              ->setEnabled( false );
   pb_rgc                ->setEnabled( false );
   pb_simulate           ->setEnabled( false );
   pb_pm                 ->setEnabled( false );
   pb_testiq             ->setEnabled( false );
   pb_guinier            ->setEnabled( false );

   le_pm_q_start         ->clearFocus();
   le_pm_q_end           ->clearFocus();
   le_pm_q_pts           ->clearFocus();
   le_pm_grid_size       ->clearFocus();
   le_pm_samp_e_dens     ->clearFocus();
   le_pm_buff_e_dens     ->clearFocus();

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

   pb_test_i_of_t        ->setEnabled( false );
   pb_guinier_plot_rg    ->setEnabled( false );
   pb_guinier_plot_mw    ->setEnabled( false );

   // powerfit disables
   pb_powerfit_fit                    ->setEnabled( false );
   pb_powerfit_create_adjusted_curve  ->setEnabled( false );
   pb_powerfit_reset                  ->setEnabled( false );
   le_powerfit_q_start                ->setEnabled( false );
   le_powerfit_q_end                  ->setEnabled( false );

   le_powerfit_a                      ->setEnabled( false );
   le_powerfit_b                      ->setEnabled( false );
   le_powerfit_c                      ->setEnabled( false );
   le_powerfit_c_min                  ->setEnabled( false );
   le_powerfit_c_max                  ->setEnabled( false );

   le_powerfit_fit_epsilon            ->setEnabled( false );
   le_powerfit_fit_iterations         ->setEnabled( false );
   le_powerfit_fit_max_calls          ->setEnabled( false );

   le_powerfit_lambda                 ->setEnabled( false );
   le_powerfit_extinction_coef        ->setEnabled( false );

   cb_powerfit_dispersion_correction  ->setEnabled( false );

   le_powerfit_lambda2                ->setEnabled( false );

   cb_powerfit_fit_curve              ->setEnabled( false );
   cb_powerfit_fit_alg                ->setEnabled( false );
   cb_powerfit_fit_alg_weight         ->setEnabled( false );

   // baseline2 disables
   pb_baseline2_fit                   ->setEnabled( false );
   pb_baseline2_create_adjusted_curve ->setEnabled( false );
   le_baseline2_q_start               ->setEnabled( false );
   le_baseline2_q_end                 ->setEnabled( false );

}

void US_Hydrodyn_Dad::model_select_all()
{
   bool all_selected = true;
   for ( int i = 0; i < lb_model_files->count(); i++ )
   {
      if ( !lb_model_files->item( i )->isSelected() )
      {
         all_selected = false;
         break;
      }
   }

   disable_updates = true;
   for ( int i = 0; i < lb_model_files->count(); i++ )
   {
      lb_model_files->item( i)->setSelected( !all_selected );
   }
   disable_updates = false;
   update_enables();
}

void US_Hydrodyn_Dad::model_save()
{
   disable_all();
   model_save( MQT::get_lb_qsl( lb_model_files, true ) );
   update_enables();
}

void US_Hydrodyn_Dad::model_text()
{
   disable_all();
   model_text( MQT::get_lb_qsl( lb_model_files, true ) );
   update_enables();
}

void US_Hydrodyn_Dad::model_view()
{
   disable_all();
   model_view( MQT::get_lb_qsl( lb_model_files, true ) );
   update_enables();
}

void US_Hydrodyn_Dad::model_remove()
{
   disable_all();
   model_remove( MQT::get_lb_qsl( lb_model_files, true ) );
   update_enables();
}

void US_Hydrodyn_Dad::model_remove( QStringList files )
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
                                    us_tr( "&Remove them anyway" ), 
                                    us_tr( "&Quit from removing files" ), 
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
   for ( int i = lb_model_files->count(); i >= 0; i-- )
   {
      if ( selected_map.count( lb_model_files->item( i )->text() ) )
      {
         models_not_saved.erase( lb_model_files->item( i )->text() );
         delete lb_model_files->takeItem( i );
      }
   }

   for ( int i = lb_model_files->count() - 1; i >= 0; i-- )
   {
      if ( selected_map.count( lb_model_files->item( i )->text() ) )
      {
         editor_msg( "black", QString( us_tr( "Removed model %1" ) ).arg( lb_model_files->item( i )->text() ) );
         models.erase( lb_model_files->item( i )->text() );
         delete lb_model_files->takeItem( i );
      }
   }

   update_enables();

}

void US_Hydrodyn_Dad::model_view( QStringList files )
{
   // run rasmol on models

   QString tmpd = USglobal->config_list.root_dir + "/somo/saxs/tmp";
   QDir dir( tmpd );
   if (!dir.exists())
   {
      if ( dir.mkdir( tmpd ) )
      {
         editor_msg( "red", QString( us_tr( "Error: could not create temporary directory %1" ) ).arg( tmpd ) );
         return;
      }
   }

   for ( int i = 0; i < (int) files.size(); ++i )
   {
      int bead_count;
      QStringList qsl0 = (models[ files[ i ] ] ).split( "\n" , Qt::SkipEmptyParts );

      if ( qsl0.size() < 1 )
      {
         editor_msg( "red", QString( us_tr( "Error: insufficient model info for file %1 [a]" ) ).arg( files[ i ] ) );
         return;
      }
         
      {
         QStringList qsl = (qsl0[ 0 ] ).split( QRegExp( "\\s+" ) , Qt::SkipEmptyParts );
         if ( qsl.size() < 1 )
         {
            editor_msg( "red", QString( us_tr( "Error: insufficient model info for file %1 [b]" ) ).arg( files[ i ] ) );
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

         QStringList qsl = (qsl0[ j ] ).split( QRegExp( "\\s+" ) , Qt::SkipEmptyParts );

         if ( qsl.size() < 4 )
         {
            editor_msg( "red", QString( us_tr( "Error: insufficient model info for file %1 line %2 [c]" ) ).arg( files[ i ] ).arg( linepos ) );
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
            editor_msg( "red", QString( us_tr( "Error: could not create output file %1" ) ).arg( f.fileName() ) );
            return;
         }
         QTextStream ts( &f );
         ts << spt;
         f.close();
      }
      {
         QFile f( tmpd + "/" + files[ i ] + ".bms" );
         if ( !f.open( QIODevice::WriteOnly ) )
         {
            editor_msg( "red", QString( us_tr( "Error: could not create output file %1" ) ).arg( f.fileName() ) );
            return;
         }
         QTextStream ts( &f );
         ts << bms;
         f.close();
      }

      (( US_Hydrodyn * ) us_hydrodyn )->model_viewer( tmpd + QDir::separator() + files[ i ] + ".spt", "-script" );
   }
}

void US_Hydrodyn_Dad::model_text( QStringList files )
{
   
   QString tmpd = USglobal->config_list.root_dir + "/somo/saxs/tmp";
   QDir dir( tmpd );
   if (!dir.exists())
   {
      if ( dir.mkdir( tmpd ) )
      {
         editor_msg( "red", QString( us_tr( "Error: could not create temporary directory %1" ) ).arg( tmpd ) );
         return;
      }
   }

   for ( int i = 0; i < (int) files.size(); ++i )
   {
      QString file = files[ i ] + ".bead_model";
      QFile f( tmpd + "/" + file );
      if ( !f.open( QIODevice::WriteOnly ) )
      {
         editor_msg( "red", QString( us_tr( "Error: could not create output file %1" ) ).arg( f.fileName() ) );
         return;
      }
      QTextStream ts( &f );
      ts << models[ files[ i ] ];
      f.close();

      TextEdit *edit;
      edit = new TextEdit( this, qPrintable( f.fileName() ) );
      edit->setFont    ( QFont( "Courier" ) );
      edit->setPalette ( PALET_NORMAL );
      AUTFBACK( edit );
      edit->setGeometry( global_Xpos + 30, global_Ypos + 30, 685, 600 );
      edit->load( f.fileName() );
      edit->show();
   }
}

bool US_Hydrodyn_Dad::model_save( QStringList files )
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
         editor_msg( "red", us_tr( "save cancelled" ) );
         return false;
         break;
      }
   }
   return !errors;
}

bool US_Hydrodyn_Dad::model_save( QString file, bool & cancel, bool & overwrite_all )
{
   if ( !models.count( file ) )
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
      editor_msg( "red", QString( us_tr( "Error: can not open %1 in directory %2 for writing" ) )
                  .arg( use_filename )
                  .arg( QDir::current().canonicalPath() )
                  );
      return false;
   }

   QTextStream ts( &f );

   ts << models[ file ];
   f.close();
   editor_msg( "black", QString( us_tr( "%1 written as %2" ) )
               .arg( file )
               .arg( use_filename ) );
   models_not_saved.erase( file );
   return true;
}

void US_Hydrodyn_Dad::usp_config_plot_dist( const QPoint & ) {
   US_PlotChoices *uspc = new US_PlotChoices( usp_plot_dist );
   uspc->exec();
   delete uspc;
}

void US_Hydrodyn_Dad::usp_config_plot_ref( const QPoint & ) {
   US_PlotChoices *uspc = new US_PlotChoices( usp_plot_ref );
   uspc->exec();
   delete uspc;
}

void US_Hydrodyn_Dad::usp_config_plot_errors( const QPoint & ) {
   US_PlotChoices *uspc = new US_PlotChoices( usp_plot_errors );
   uspc->exec();
   delete uspc;
}

void US_Hydrodyn_Dad::usp_config_ggqfit_plot( const QPoint & ) {
   US_PlotChoices *uspc = new US_PlotChoices( usp_ggqfit_plot );
   uspc->exec();
   delete uspc;
}

void US_Hydrodyn_Dad::usp_config_guinier_plot( const QPoint & ) {
   US_PlotChoices *uspc = new US_PlotChoices( usp_guinier_plot );
   uspc->exec();
   delete uspc;
}

void US_Hydrodyn_Dad::usp_config_guinier_plot_rg( const QPoint & ) {
   US_PlotChoices *uspc = new US_PlotChoices( usp_guinier_plot_rg );
   uspc->exec();
   delete uspc;
}

void US_Hydrodyn_Dad::usp_config_guinier_plot_mw( const QPoint & ) {
   US_PlotChoices *uspc = new US_PlotChoices( usp_guinier_plot_mw );
   uspc->exec();
   delete uspc;
}

void US_Hydrodyn_Dad::usp_config_guinier_plot_summary( const QPoint & ) {
   US_PlotChoices *uspc = new US_PlotChoices( usp_guinier_plot_summary );
   uspc->exec();
   delete uspc;
}

void US_Hydrodyn_Dad::usp_config_guinier_plot_errors( const QPoint & ) {
   US_PlotChoices *uspc = new US_PlotChoices( usp_guinier_plot_errors );
   uspc->exec();
   delete uspc;
}

void US_Hydrodyn_Dad::set_pbmode_main() {
   pbmode_select( PBMODE_MAIN );
}

void US_Hydrodyn_Dad::set_pbmode_sel() {
   pbmode_select( PBMODE_SEL );
}

void US_Hydrodyn_Dad::set_pbmode_crop() {
   pbmode_select( PBMODE_CROP );
}

void US_Hydrodyn_Dad::set_pbmode_conc() {
   pbmode_select( PBMODE_CONC );
}

void US_Hydrodyn_Dad::set_pbmode_sd() {
   pbmode_select( PBMODE_SD );
}

void US_Hydrodyn_Dad::set_pbmode_fasta() {
   pbmode_select( PBMODE_FASTA );
}

void US_Hydrodyn_Dad::set_pbmode_dad() {
   pbmode_select( PBMODE_DAD );
}

void US_Hydrodyn_Dad::set_pbmode_q_exclude() {
   pbmode_select( PBMODE_Q_EXCLUDE );
}

void US_Hydrodyn_Dad::set_pbmode_none() {
   pbmode_select( PBMODE_NONE );
}

void US_Hydrodyn_Dad::pbmode_select( pbmodes mode ) {

   ShowHide::hide_widgets( pbmode_main_widgets, always_hide_widgets, true );
   ShowHide::hide_widgets( pbmode_sel_widgets, always_hide_widgets, true );
   ShowHide::hide_widgets( pbmode_crop_widgets, always_hide_widgets, true );
   ShowHide::hide_widgets( pbmode_conc_widgets, always_hide_widgets, true );
   ShowHide::hide_widgets( pbmode_sd_widgets, always_hide_widgets, true );
   ShowHide::hide_widgets( pbmode_fasta_widgets, always_hide_widgets, true );
   ShowHide::hide_widgets( pbmode_dad_widgets, always_hide_widgets, true );
   ShowHide::hide_widgets( pbmode_q_exclude_widgets, always_hide_widgets, true );

   switch ( mode ) {
   case PBMODE_MAIN :
      ShowHide::hide_widgets( pbmode_main_widgets, always_hide_widgets, false );
      break;
   case PBMODE_SEL :
      ShowHide::hide_widgets( pbmode_sel_widgets, always_hide_widgets, false );
      break;
   case PBMODE_CROP :
      ShowHide::hide_widgets( pbmode_crop_widgets, always_hide_widgets, false );
      break;
   case PBMODE_CONC :
      ShowHide::hide_widgets( pbmode_conc_widgets, always_hide_widgets, false );
      break;
   case PBMODE_SD :
      ShowHide::hide_widgets( pbmode_sd_widgets, always_hide_widgets, false );
      break;
   case PBMODE_FASTA :
      ShowHide::hide_widgets( pbmode_fasta_widgets, always_hide_widgets, false );
      break;
   case PBMODE_DAD :
      ShowHide::hide_widgets( pbmode_dad_widgets, always_hide_widgets, false );
      break;
   case PBMODE_Q_EXCLUDE :
      ShowHide::hide_widgets( pbmode_q_exclude_widgets, always_hide_widgets, false );
      break;
   case PBMODE_NONE :
      break;
   }
}

void US_Hydrodyn_Dad::fasta_file() {
   // load file

   QString use_dir = QDir::currentPath();
   ((US_Hydrodyn  *)us_hydrodyn)->select_from_directory_history( use_dir, this );
   raise();

   QString filename = QFileDialog::getOpenFileName(
                                                   this,
                                                   windowTitle() + us_tr( "Load FASTA sequence File" ),
                                                   use_dir,
                                                   "FASTA files ( *.fasta.txt *.fasta );;All files (*)"
                                                   );
   
   if ( filename.isEmpty() ) {
      return;
   }

   ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( filename );

   QFile f( filename );
   
   if ( !f.open( QIODevice::ReadOnly ) ) {
      QMessageBox::warning(
                           this, 
                           windowTitle()+ us_tr( ": Compute PSV from FASTA" ),
                           QString( us_tr( "Could not open file '%1' for reading. Possibly a permissions issue." ) ).arg( filename )
                           );
      return;
   }
      
   QStringList qsl;

   QTextStream ts( &f );
   QRegExp rx_empty( "^\\s*$" );
   QRegExp rx_newseq( "^\\s*>" );

   QString seq;
   int chains = 0;
   
   QString seq_names;

   // find >seq lines

   while ( !ts.atEnd() ) {
      QString qs = ts.readLine().trimmed();
      
      if ( rx_empty.exactMatch( qs ) ) {
         continue;
      }

      if ( rx_newseq.indexIn( qs ) > -1 ) {
         seq_names += qs.replace( rx_newseq, "" ) + "\n";
         chains++;
         continue;
      }
      
      seq += qs;
   }

   f.close();

   if ( seq.isEmpty() ) {
      QMessageBox::warning(
                           this, 
                           windowTitle()+ us_tr( ": Compute PSV from FASTA" ),
                           QString( us_tr( "File '%1' no sequence information found." ) ).arg( filename )
                           );
      return;
   }
      
   QStringList seq_chars = seq.split( "" );

#if defined( DEBUG_FASTA_SEQ )
   {
      QTextStream ts( stdout );
      ts << QString( "%1 sequences %2:\n%3" ).arg( chains ).arg( seq_names ).arg( seq );
      ts << seq_chars.join( "\n" );
   }
#endif
   
   double psv;
   QString msgs;
   if ( !((US_Hydrodyn  *)us_hydrodyn)->calc_fasta_vbar( seq_chars, psv, msgs ) ) {
      QMessageBox::warning(
                           this, 
                           windowTitle()+ us_tr( ": Compute PSV from FASTA" ),
                           msgs );
      return;
   }

   QMessageBox::information(
                            this, 
                            windowTitle()+ us_tr( ": Compute PSV from FASTA" ),
                            QString( us_tr( "FASTA computed from the following sequence names:\n" ) )
                            + seq_names
                            );

   le_fasta_value->setText( QString( "" ).sprintf( "%.3f", psv ) );
   le_fasta_value->setEnabled( true );
   return;
}

void US_Hydrodyn_Dad::q_exclude_update_lbl() {
   pb_q_exclude_clear->setEnabled( q_exclude.size() > 0 );

   QString msg = "<hr>";

   if ( !q_exclude.size() )  {
      lbl_q_exclude_detail->setText( msg + us_tr( "<b>Currently no excluded q values</b>" ) + "<br>" );
      plot_files( true );
      return;
   }

   msg += QString( us_tr( "<b>%1 Excluded q value%2:</b>" ) )
      .arg( q_exclude.size() )
      .arg( q_exclude.size() == 1 ? "" : "s" )
      + "<br>";

   static int entries_per_row = 5;
   static int max_rows = 8;

   int entry = 0;
   int row   = 0;

   msg += "<center><table border=1 bgcolor=#FFF cellpadding=1.5>\n<tr>";

   for ( auto & q : q_exclude ) {
      msg += QString( "<td>%1</td>" ).arg( q, 0, 'g', 12 );
      if ( !( ++entry % entries_per_row ) ) {
         msg += "</tr><tr>";
         if ( ++row > max_rows ) {
            msg += "<td>...</td>";
            break;
         }
      }
   }

   // don't leave an empty row
   msg = msg.replace( QRegularExpression( "</tr><tr>$" ), "" );
   
   msg += "</tr></table></center>";
   
   lbl_q_exclude_detail->setText( msg );
   plot_files( true );
}

void US_Hydrodyn_Dad::q_exclude_clear() {
   q_exclude.clear();
   q_exclude_update_lbl();
}

void US_Hydrodyn_Dad::q_exclude_vis() {
   // find curves within zoomRect 
   double minx = plot_dist_zoomer->zoomRect().left();
   double maxx = plot_dist_zoomer->zoomRect().right();
   double miny = plot_dist_zoomer->zoomRect().top();
   double maxy = plot_dist_zoomer->zoomRect().bottom();

   set < QString > selected_files;

   for ( int i = 0; i < lb_files->count(); i++ ) {
      if ( lb_files->item( i )->isSelected() ) {
         QString this_file = lb_files->item( i )->text();
         if ( f_qs.count( this_file ) &&
              f_Is.count( this_file ) ) {
            for ( unsigned int i = 0; i < f_qs[ this_file ].size(); i++ ) {
               if ( f_qs[ this_file ][ i ] >= minx &&
                    f_qs[ this_file ][ i ] <= maxx &&
                    f_Is[ this_file ][ i ] >= miny &&
                    f_Is[ this_file ][ i ] <= maxy ) {
                  selected_files.insert( this_file );
                  break;
               }
            }
         } 
      }
   }

   if ( !selected_files.size() )
   {
      editor_msg( "red", us_tr( "q exclude visible: The current visible plot is empty" ) );
      return;
   }
   
   int added       = 0;
   int preexisting = 0;
   
   set < double > this_q_checked;
      
   for ( auto const & it : selected_files ) {
      for ( auto const & q : f_qs[ it ] ) { 
         if ( q >= minx && q <= maxx ) {
            if ( !this_q_checked.count( q ) ) {
               this_q_checked.insert( q );
               if ( !q_exclude.count( q ) ) {
                  ++added;
                  q_exclude.insert( q );
               } else {
                  ++preexisting;
               }
            }
         }
      }
   }

   QString msg = us_tr( "q exclude visible :" );
   QString color = "darkblue";
   
   if ( added ) {
      msg += QString( us_tr( " %1 selected q value(s) added to the exclusion list." ) ).arg( added );
   }
   if ( preexisting ) {
      msg += QString( us_tr( " %1 selected q value(s) was(were) already excluded." ) ).arg( preexisting );
      if ( !added ) {
         color = "darkred";
      }
   }
   editor_msg( color, msg );
   q_exclude_update_lbl();
}

void US_Hydrodyn_Dad::q_exclude_left() {
   // find curves within zoomRect 
   double minx = plot_dist_zoomer->zoomRect().left();
   double maxx = plot_dist_zoomer->zoomRect().right();
   double miny = plot_dist_zoomer->zoomRect().top();
   double maxy = plot_dist_zoomer->zoomRect().bottom();

   set < QString > selected_files;

   for ( int i = 0; i < lb_files->count(); i++ ) {
      if ( lb_files->item( i )->isSelected() ) {
         QString this_file = lb_files->item( i )->text();
         if ( f_qs.count( this_file ) &&
              f_Is.count( this_file ) ) {
            for ( unsigned int i = 0; i < f_qs[ this_file ].size(); i++ ) {
               if ( f_qs[ this_file ][ i ] >= minx &&
                    f_qs[ this_file ][ i ] <= maxx &&
                    f_Is[ this_file ][ i ] >= miny &&
                    f_Is[ this_file ][ i ] <= maxy ) {
                  selected_files.insert( this_file );
                  break;
               }
            }
         } 
      }
   }

   if ( !selected_files.size() ) {
      editor_msg( "red", us_tr( "q exclude left: The current visible plot is empty" ) );
      return;
   }
   
   set < double > this_q_checked;
   set < double > this_q_new;
   
   for ( auto const & it : selected_files ) {
      for ( auto const & q : f_qs[ it ] ) { 
         if ( q >= minx && q <= maxx ) {
            if ( !this_q_checked.count( q ) ) {
               this_q_checked.insert( q );
               if ( !q_exclude.count( q ) ) {
                  this_q_new.insert( q );
               }
            }
         }
      }
   }

   if ( this_q_new.size() ) {
      q_exclude.insert( *(this_q_new.begin()) );
      editor_msg( "darkblue", us_tr( "q exclude : 1 new visible left q value was added\n" ) );
      q_exclude_update_lbl();
   } else {
      editor_msg( "darkred", us_tr( "q exclude : no new visible left q values were found\n" ) );
   }      
}

void US_Hydrodyn_Dad::q_exclude_right() {
   // find curves within zoomRect 
   double minx = plot_dist_zoomer->zoomRect().left();
   double maxx = plot_dist_zoomer->zoomRect().right();
   double miny = plot_dist_zoomer->zoomRect().top();
   double maxy = plot_dist_zoomer->zoomRect().bottom();

   set < QString > selected_files;

   for ( int i = 0; i < lb_files->count(); i++ ) {
      if ( lb_files->item( i )->isSelected() ) {
         QString this_file = lb_files->item( i )->text();
         if ( f_qs.count( this_file ) &&
              f_Is.count( this_file ) ) {
            for ( unsigned int i = 0; i < f_qs[ this_file ].size(); i++ ) {
               if ( f_qs[ this_file ][ i ] >= minx &&
                    f_qs[ this_file ][ i ] <= maxx &&
                    f_Is[ this_file ][ i ] >= miny &&
                    f_Is[ this_file ][ i ] <= maxy ) {
                  selected_files.insert( this_file );
                  break;
               }
            }
         } 
      }
   }

   if ( !selected_files.size() ) {
      editor_msg( "red", us_tr( "q exclude right: The current visible plot is empty" ) );
      return;
   }
   
   set < double > this_q_checked;
   set < double > this_q_new;
   
   for ( auto const & it : selected_files ) {
      for ( auto const & q : f_qs[ it ] ) { 
         if ( q >= minx && q <= maxx ) {
            if ( !this_q_checked.count( q ) ) {
               this_q_checked.insert( q );
               if ( !q_exclude.count( q ) ) {
                  this_q_new.insert( q );
               }
            }
         }
      }
   }

   if ( this_q_new.size() ) {
      q_exclude.insert( *(--this_q_new.end()) );
      editor_msg( "darkblue", us_tr( "q exclude : 1 new visible right q value was added\n" ) );
      q_exclude_update_lbl();
   } else {
      editor_msg( "darkred", us_tr( "q exclude : no new visible right q values were found\n" ) );
   }      
}

void US_Hydrodyn_Dad::q_exclude_opt_remove_unreferenced() {

   if ( !q_exclude.size() ) {
      // nothing to check
      return;
   }

   // get all qvalues from curves

   set < vector < double > > q_grids;
   
   for ( auto const & f_q : f_qs ) {
      if ( f_is_time.count( f_q.first ) && f_is_time[ f_q.first ] ) {
         continue;
      }
      q_grids.insert( f_q.second );
   }

   set < double > q_refd;

   for ( auto const & qs : q_grids ) {
      for ( auto const & q : qs ) {
         q_refd.insert( q );
      }
   }

   set < double > q_not_refd;

   for ( auto const & q : q_exclude ) {
      if ( !q_refd.count( q ) ) {
         q_not_refd.insert( q );
      }
   }

   if ( !q_not_refd.size() ) {
      return;
   }

   switch ( QMessageBox::question(this, 
                                  windowTitle() + us_tr( " : q excludes" )
                                  , QString( us_tr(
                                                   "%1 excluded q values are no longer referenced by any curves\n"
                                                   "Do you wish to remove the exclusions?"
                                                   ) )
                                  .arg( q_not_refd.size() )
                                  ) )
   {
   case QMessageBox::Yes : 
      for ( auto const & q : q_not_refd ) {
         q_exclude.erase( q );
      }
      q_exclude_update_lbl();
      break;
   default: 
      break;
   }
}   
