#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_saxs_buffer.h"

#define SLASH QDir::separator()

US_Hydrodyn_Saxs_Buffer::US_Hydrodyn_Saxs_Buffer(
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
   setCaption(tr("US-SOMO: SAXS Buffer Subtraction Utility"));
   order_ascending = false;
   conc_widget     = false;

   QDir::setCurrent( ((US_Hydrodyn *)us_hydrodyn)->somo_dir + QDir::separator() + "saxs" );

   saxs_widget = &(((US_Hydrodyn *) us_hydrodyn)->saxs_plot_widget);
   saxs_window = ((US_Hydrodyn *) us_hydrodyn)->saxs_plot_window;
   ((US_Hydrodyn *) us_hydrodyn)->saxs_buffer_widget = true;
   plot_dist_zoomer = (ScrollZoomer *) 0;

   best_fitness = 1e99;
   disable_updates = false;

   setupGUI();
   running = false;

   for ( unsigned int i = 0; i < saxs_window->plotted_I_error.size(); i++ )
   {
      qs.      push_back( saxs_window->plotted_q           [ i ] );
      Is.      push_back( saxs_window->plotted_I           [ i ] );
      I_errors.push_back( saxs_window->plotted_I_error     [ i ] );
      names.   push_back( saxs_window->qsl_plotted_iq_names[ i ] );
   }

   set_target();
   update_enables();

   global_Xpos += 30;
   global_Ypos += 30;

   unsigned int csv_height = 45; // header height
   unsigned int csv_width = t_csv->columnWidth(0) + 49;
   for ( int i = 0; i < t_csv->numRows(); i++ )
   {
      csv_height += t_csv->rowHeight(i);
   }
   for ( int i = 1; i < t_csv->numCols(); i++ )
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
   lb_files        ->setMaximumWidth( csv_width / 3 );
   lb_created_files->setMaximumWidth( csv_width / 3 );
   editor          ->setMaximumWidth( csv_width / 3 );
   // progress        ->setMaximumWidth( csv_width / 3 );
   pb_help         ->setMinimumWidth( csv_width / 3 );
   pb_cancel       ->setMinimumWidth( csv_width / 3 );

   setGeometry(global_Xpos, global_Ypos, csv_width, 100 + csv_height );
   pb_set_buffer->setMaximumWidth ( pb_select_all->width() + 10 );
   pb_set_empty ->setMaximumWidth ( pb_select_all->width() + 10 );
   pb_set_signal->setMaximumWidth ( pb_select_all->width() + 10 );
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
}

US_Hydrodyn_Saxs_Buffer::~US_Hydrodyn_Saxs_Buffer()
{
   ((US_Hydrodyn *)us_hydrodyn)->saxs_buffer_widget = false;
}

void US_Hydrodyn_Saxs_Buffer::setupGUI()
{
   int minHeight1 = 24;
   int minHeight3 = 25;

   lbl_title = new QLabel(csv1.name.left(80), this);
   lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_files = new QLabel("Data files", this);
   lbl_files->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_files->setMinimumHeight(minHeight1);
   lbl_files->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   pb_add_files = new QPushButton(tr("Add files"), this);
   pb_add_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_add_files->setMinimumHeight(minHeight1);
   pb_add_files->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_add_files, SIGNAL(clicked()), SLOT(add_files()));

   pb_conc = new QPushButton(tr("Concentrations"), this);
   pb_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_conc->setMinimumHeight(minHeight1);
   pb_conc->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_conc, SIGNAL(clicked()), SLOT(conc()));

   pb_clear_files = new QPushButton(tr("Remove files"), this);
   pb_clear_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_clear_files->setMinimumHeight(minHeight1);
   pb_clear_files->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_clear_files, SIGNAL(clicked()), SLOT(clear_files()));

   lb_files = new QListBox(this, "files files listbox" );
   lb_files->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lb_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lb_files->setEnabled(true);
   lb_files->setSelectionMode( QListBox::Multi );
   lb_files->setMinimumHeight( minHeight1 * 8 );
   connect( lb_files, SIGNAL( selectionChanged() ), SLOT( update_files() ) );

   lbl_selected = new QLabel("0 files selected", this );
   lbl_selected->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_selected->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_selected->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   pb_select_all = new QPushButton(tr("Select all"), this);
   pb_select_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_select_all->setMinimumHeight(minHeight1);
   pb_select_all->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_select_all, SIGNAL(clicked()), SLOT(select_all()));

   pb_invert = new QPushButton(tr("Invert"), this);
   pb_invert->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_invert->setMinimumHeight(minHeight1);
   pb_invert->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_invert, SIGNAL(clicked()), SLOT(invert()));

   // pb_plot_files = new QPushButton(tr("Plot"), this);
   // pb_plot_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   // pb_plot_files->setMinimumHeight(minHeight1);
   // pb_plot_files->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   // connect(pb_plot_files, SIGNAL(clicked()), SLOT(plot_files()));

   pb_save_avg = new QPushButton(tr("Average"), this);
   pb_save_avg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_save_avg->setMinimumHeight(minHeight1);
   pb_save_avg->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_save_avg, SIGNAL(clicked()), SLOT(save_avg()));

   pb_conc_avg = new QPushButton(tr("Concentration normalized average"), this);
   pb_conc_avg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_conc_avg->setMinimumHeight(minHeight1);
   pb_conc_avg->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_conc_avg, SIGNAL(clicked()), SLOT(conc_avg()));

   pb_set_buffer = new QPushButton(tr("Set buffer"), this);
   pb_set_buffer->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_set_buffer->setMinimumHeight(minHeight1);
   pb_set_buffer->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_set_buffer, SIGNAL(clicked()), SLOT(set_buffer()));

   lbl_buffer = new QLabel("", this );
   lbl_buffer->setMinimumHeight(minHeight1);
   lbl_buffer->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_buffer->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_buffer->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   pb_set_empty = new QPushButton(tr("Set blank"), this);
   pb_set_empty->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_set_empty->setMinimumHeight(minHeight1);
   pb_set_empty->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_set_empty, SIGNAL(clicked()), SLOT(set_empty()));

   lbl_empty = new QLabel("", this );
   lbl_empty->setMinimumHeight(minHeight1);
   lbl_empty->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_empty->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_empty->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   pb_set_signal = new QPushButton(tr("Set solution"), this);
   pb_set_signal->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_set_signal->setMinimumHeight(minHeight1);
   pb_set_signal->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_set_signal, SIGNAL(clicked()), SLOT(set_signal()));

   lbl_signal = new QLabel("", this );
   lbl_signal->setMinimumHeight(minHeight1);
   lbl_signal->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_signal->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_signal->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   lbl_created_files = new QLabel("Produced Files", this);
   lbl_created_files->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_created_files->setMinimumHeight(minHeight1);
   lbl_created_files->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_created_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lb_created_files = new QListBox(this, "created_files created_files listbox" );
   lb_created_files->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lb_created_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lb_created_files->setEnabled(true);
   lb_created_files->setSelectionMode( QListBox::Multi );
   lb_created_files->setMinimumHeight( minHeight1 * 2 );
   connect( lb_created_files, SIGNAL( selectionChanged() ), SLOT( update_created_files() ) );

   pb_select_all_created = new QPushButton(tr("Select all"), this);
   pb_select_all_created->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize  - 1));
   pb_select_all_created->setMinimumHeight(minHeight1);
   pb_select_all_created->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_select_all_created, SIGNAL(clicked()), SLOT(select_all_created()));

   pb_save_created_csv = new QPushButton(tr("Save as CSV"), this);
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

   cb_save_to_csv = new QCheckBox(this);
   cb_save_to_csv->setText(tr(" Combined I(q) Results File:"));
   cb_save_to_csv->setChecked(false);
   cb_save_to_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   cb_save_to_csv->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_save_to_csv, SIGNAL(clicked()), SLOT(save_to_csv()));

   le_csv_filename = new QLineEdit(this, "csv_filename Line Edit");
   le_csv_filename->setText("search_results");
   le_csv_filename->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_csv_filename->setMinimumWidth(150);
   le_csv_filename->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_csv_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   cb_individual_files = new QCheckBox(this);
   cb_individual_files->setText(tr(" Create individual SAXS Results Files"));
   cb_individual_files->setChecked(false);
   cb_individual_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   cb_individual_files->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   pb_replot_saxs = new QPushButton(tr("Replot stored I(q)"), this);
   pb_replot_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_replot_saxs->setMinimumHeight(minHeight1);
   pb_replot_saxs->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_replot_saxs, SIGNAL(clicked()), SLOT(replot_saxs()));

   pb_save_saxs_plot = new QPushButton(tr("Store current I(q) plot"), this);
   pb_save_saxs_plot->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_save_saxs_plot->setMinimumHeight(minHeight1);
   pb_save_saxs_plot->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_save_saxs_plot, SIGNAL(clicked()), SLOT(save_saxs_plot()));

   pb_set_target = new QPushButton(tr("Set Target"), this);
   pb_set_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_set_target->setMinimumHeight(minHeight1);
   pb_set_target->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_set_target, SIGNAL(clicked()), SLOT(set_target()));

   lbl_current_target = new QLabel("", this);
   lbl_current_target->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_current_target->setMinimumHeight(minHeight1);
   lbl_current_target->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_current_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   pb_start = new QPushButton(tr("Start buffer subtraction"), this);
   pb_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_start->setMinimumHeight(minHeight1);
   pb_start->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_start, SIGNAL(clicked()), SLOT(start()));

   pb_run_current = new QPushButton(tr("Current buffer subtraction"), this);
   pb_run_current->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_run_current->setMinimumHeight(minHeight1);
   pb_run_current->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_run_current, SIGNAL(clicked()), SLOT(run_current()));

   pb_run_best = new QPushButton(tr("Best buffer subtraction"), this);
   pb_run_best->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_run_best->setMinimumHeight(minHeight1);
   pb_run_best->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_run_best, SIGNAL(clicked()), SLOT(run_best()));

   pb_stop = new QPushButton(tr("Stop"), this);
   pb_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stop->setMinimumHeight(minHeight1);
   pb_stop->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_stop, SIGNAL(clicked()), SLOT(stop()));

   editor = new QTextEdit(this);
   editor->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   editor->setReadOnly(true);

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
   plot_dist->setAxisTitle(QwtPlot::xBottom, /* cb_guinier->isChecked() ? tr("q^2 (1/Angstrom^2)") : */  tr("q (1/Angstrom)"));
   plot_dist->setAxisTitle(QwtPlot::yLeft, tr("Log10 I(q)"));
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

   t_csv = new QTable(csv1.data.size(), csv1.header.size(), this);
   t_csv->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   t_csv->setMinimumHeight(minHeight1 * ( 1 + csv1.data.size() ) );
   t_csv->setMaximumHeight(minHeight1 * ( 2 + csv1.data.size() ) );
   // t_csv->setMinimumWidth(minWidth1);
   t_csv->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   t_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   t_csv->setEnabled(true);

   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      for ( unsigned int j = 0; j < csv1.data[i].size(); j++ )
      {
         if ( csv1.data[i][j] == "Y" || csv1.data[i][j] == "N" )
         {
            t_csv->setItem( i, j, new QCheckTableItem( t_csv, "" ) );
            ((QCheckTableItem *)(t_csv->item( i, j )))->setChecked( csv1.data[i][j] == "Y" );
         } else {
            t_csv->setText( i, j, csv1.data[i][j] );
         }
      }
   }

   for ( unsigned int i = 0; i < csv1.header.size(); i++ )
   {
      t_csv->horizontalHeader()->setLabel(i, csv1.header[i]);
   }

   t_csv->setSorting(false);
   t_csv->setRowMovingEnabled(false);
   t_csv->setColumnMovingEnabled(false);
   t_csv->setReadOnly(false);

   t_csv->setColumnWidth(0, 330);
   t_csv->setColumnReadOnly(0, true);
   t_csv->setColumnReadOnly(t_csv->numCols() - 1, true);

   // probably I'm not understanding something, but these next two lines don't seem to do anything
   t_csv->horizontalHeader()->adjustHeaderSize();
   t_csv->adjustSize();

   recompute_interval_from_points();

   connect(t_csv, SIGNAL(valueChanged(int, int)), SLOT(table_value(int, int )));

   lbl_np = new QLabel( "Buffer subtraction non-positive:    ", this );
   lbl_np->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_np->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_np->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));

   rb_np_crop = new QRadioButton( tr("Crop "), this);
   rb_np_crop->setEnabled(true);
   rb_np_crop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   rb_np_crop->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   rb_np_min = new QRadioButton( tr("Set to minimum "), this);
   rb_np_min->setEnabled(true);
   rb_np_min->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   rb_np_min->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   rb_np_ignore = new QRadioButton( tr("Ignore (log of negative not defined)"), this);
   rb_np_ignore->setEnabled(true);
   rb_np_ignore->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   rb_np_ignore->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   rb_np_ask = new QRadioButton( tr("Ask (blocks mass processing) "), this);
   rb_np_ask->setEnabled(true);
   rb_np_ask->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   rb_np_ask->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   bg_np = new QButtonGroup(1, Qt::Horizontal, 0);
   bg_np->setRadioButtonExclusive(true);
   bg_np->insert(rb_np_crop);
   bg_np->insert(rb_np_min);
   bg_np->insert(rb_np_ignore);
   bg_np->insert(rb_np_ask);
   rb_np_crop->setChecked( true );

   cb_multi_sub = new QCheckBox(this);
   cb_multi_sub->setText(tr(" Subtract buffer from every selected file (exepting set buffer and set blank) " ) );
   cb_multi_sub->setChecked(false);
   cb_multi_sub->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   cb_multi_sub->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect( cb_multi_sub, SIGNAL( clicked() ), SLOT( update_enables() ) );

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
   hbl_file_buttons->addWidget ( pb_conc);
   hbl_file_buttons->addWidget ( pb_clear_files );

   QBoxLayout *hbl_file_buttons_2 = new QHBoxLayout( 0 );
   hbl_file_buttons_2->addWidget ( pb_select_all );
   hbl_file_buttons_2->addWidget ( pb_invert );
   hbl_file_buttons_2->addWidget ( pb_save_avg );

   QBoxLayout *hbl_buffer = new QHBoxLayout( 0 );
   hbl_buffer->addWidget ( pb_set_buffer );
   hbl_buffer->addWidget ( lbl_buffer );

   QBoxLayout *hbl_empty = new QHBoxLayout( 0 );
   hbl_empty->addWidget ( pb_set_empty );
   hbl_empty->addWidget ( lbl_empty );

   QBoxLayout *hbl_signal = new QHBoxLayout( 0 );
   hbl_signal->addWidget ( pb_set_signal );
   hbl_signal->addWidget ( lbl_signal );

   QBoxLayout *hbl_created = new QHBoxLayout( 0 );
   hbl_created->addWidget ( pb_select_all_created );
   hbl_created->addWidget ( pb_save_created_csv );
   hbl_created->addWidget ( pb_save_created );

   QBoxLayout *hbl_created_2 = new QHBoxLayout( 0 );
   hbl_created_2->addWidget ( pb_show_created );
   hbl_created_2->addWidget ( pb_show_only_created );

   QBoxLayout *vbl_editor_group = new QVBoxLayout(0);
   vbl_editor_group->addWidget (frame);
   vbl_editor_group->addWidget (editor);

   QBoxLayout *vbl_files = new QVBoxLayout( 0 );
   vbl_files->addWidget( lbl_files );
   vbl_files->addLayout( hbl_file_buttons );
   vbl_files->addWidget( lb_files );
   vbl_files->addWidget( lbl_selected );
   vbl_files->addLayout( hbl_file_buttons_2 );
   vbl_files->addWidget( pb_conc_avg );
   vbl_files->addLayout( hbl_buffer );
   vbl_files->addLayout( hbl_empty );
   vbl_files->addLayout( hbl_signal );
   vbl_files->addWidget( lbl_created_files );
   vbl_files->addWidget( lb_created_files );
   vbl_files->addLayout( hbl_created );
   vbl_files->addLayout( hbl_created_2 );
   vbl_files->addLayout( vbl_editor_group );

   QBoxLayout *vbl_plot_group = new QVBoxLayout(0);
   vbl_plot_group->addWidget ( plot_dist );

   QBoxLayout *hbl_files_plot = new QHBoxLayout( 0 );
   hbl_files_plot->addLayout( vbl_files );
   hbl_files_plot->addLayout( vbl_plot_group );

   QHBoxLayout *hbl_csv = new QHBoxLayout(0);
   hbl_csv->addSpacing( 2 );
   hbl_csv->addWidget ( cb_save_to_csv );
   hbl_csv->addSpacing( 2 );
   hbl_csv->addWidget ( le_csv_filename );
   hbl_csv->addSpacing( 2 );
   hbl_csv->addWidget ( cb_individual_files );
   hbl_csv->addSpacing( 2 );

   QHBoxLayout *hbl_np = new QHBoxLayout(0);
   hbl_np->addWidget ( lbl_np );
   hbl_np->addWidget ( rb_np_crop );
   hbl_np->addWidget ( rb_np_min );
   hbl_np->addWidget ( rb_np_ignore );
   hbl_np->addWidget ( rb_np_ask );

   QHBoxLayout *hbl_target = new QHBoxLayout(0);
   hbl_target->addSpacing( 2 );
   hbl_target->addWidget (pb_replot_saxs);
   hbl_target->addSpacing( 2 );
   hbl_target->addWidget (pb_save_saxs_plot);
   hbl_target->addSpacing( 2 );
   hbl_target->addWidget (pb_set_target);
   hbl_target->addSpacing( 2 );
   hbl_target->addWidget (lbl_current_target);
   hbl_target->addSpacing( 2 );

   QHBoxLayout *hbl_controls = new QHBoxLayout(0);
   hbl_controls->addSpacing( 2 );
   hbl_controls->addWidget (pb_start);
   hbl_controls->addSpacing( 2 );
   hbl_controls->addWidget (pb_run_current);
   hbl_controls->addSpacing( 2 );
   hbl_controls->addWidget (pb_run_best);
   hbl_controls->addSpacing( 2 );
   hbl_controls->addWidget (pb_stop);
   hbl_controls->addSpacing( 2 );

   // QHBoxLayout *hbl_bottom = new QHBoxLayout(0);
   // hbl_bottom->addSpacing( 2 );
   // hbl_bottom->addWidget ( pb_help );
   // hbl_bottom->addSpacing( 2 );
   // hbl_bottom->addWidget ( progress );
   // hbl_bottom->addSpacing( 2 );
   // hbl_bottom->addWidget ( pb_cancel );
   // hbl_bottom->addSpacing( 2 );
   QGridLayout *gl_bottom = new QGridLayout( 0 );
   gl_bottom->addWidget( pb_help  , 0, 0 );
   gl_bottom->addWidget( progress , 0, 1 );
   gl_bottom->addWidget( pb_cancel, 0, 2 );
   
   QVBoxLayout *background = new QVBoxLayout(this);
   background->addSpacing( 2 );
   background->addWidget ( lbl_title );
   background->addSpacing( 2 );
   background->addLayout ( hbl_files_plot );
   background->addWidget ( t_csv );
   background->addSpacing( 2 );
   background->addLayout ( hbl_np );
   background->addSpacing( 2 );
   background->addWidget ( cb_multi_sub );
   background->addSpacing( 2 );
   background->addLayout ( hbl_csv );
   background->addSpacing( 2 );
   background->addLayout ( hbl_target );
   background->addSpacing( 2 );
   background->addLayout ( hbl_controls );
   background->addSpacing( 2 );
   // background->addLayout ( hbl_bottom );
   background->addLayout ( gl_bottom );
   background->addSpacing( 2 );
}

void US_Hydrodyn_Saxs_Buffer::cancel()
{
   close();
}

void US_Hydrodyn_Saxs_Buffer::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_saxs_buffer.html");
}

void US_Hydrodyn_Saxs_Buffer::closeEvent(QCloseEvent *e)
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
      switch ( QMessageBox::warning(this, 
                                    tr( "US-SOMO: SAXS Buffer Subtraction Utility" ),
                                    QString( tr( "Please note:\n\n"
                                                 "These files were created but not saved as .dat files:\n"
                                                 "%1\n\n"
                                                 "What would you like to do?\n" ) )
                                    .arg( created_not_saved_list.join( "\n" ) ),
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
           ((QCheckTableItem *)(t_csv->item( row, 1 )))->isChecked() &&
           ((QCheckTableItem *)(t_csv->item( 0, 1 )))->isChecked()  )
      {
         ((QCheckTableItem *)(t_csv->item( 0, 1 )))->setChecked( false );
      }
      if ( row == 0 && ((QCheckTableItem *)(t_csv->item( row, 1 )))->isChecked() )
      {
         if ( ((QCheckTableItem *)(t_csv->item( 1, 1 )))->isChecked() )
         {
            ((QCheckTableItem *)(t_csv->item( 1, 1 )))->setChecked( false );
         }
         if ( ((QCheckTableItem *)(t_csv->item( 2, 1 )))->isChecked() )
         {
            ((QCheckTableItem *)(t_csv->item( 2, 1 )))->setChecked( false );
         }
      }
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Buffer::clear_display()
{
   editor->clear();
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

void US_Hydrodyn_Saxs_Buffer::set_target()
{
   QString scaling_target = "";
   if ( *saxs_widget ) 
   { 
      saxs_window->set_scaling_target( scaling_target );
      if ( !scaling_target.isEmpty() )
      {
         saxs_window->ask_iq_target_grid();
      }
   }
   lbl_current_target->setText( scaling_target );
}

void US_Hydrodyn_Saxs_Buffer::start()
{
   running = true;
   update_enables();

   map < unsigned int, double >       starts;
   map < unsigned int, double >       ends;
   map < unsigned int, unsigned int > points;
   map < unsigned int, double >       increments;
   map < unsigned int, unsigned int > offsets;
   map < unsigned int, unsigned int > next_offsets;

   unsigned int current_offset = 0;

   for ( unsigned int i = 0; i < (unsigned int)t_csv->numRows(); i++ )
   {
      if ( ((QCheckTableItem *)(t_csv->item( i, 1 )))->isChecked() )
      {
         starts    [i]   =  t_csv->text(i, 2).toDouble();
         ends      [i]   =  t_csv->text(i, 3).toDouble();
         points    [i]   =  t_csv->text(i, 4).toUInt();
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

   editor_msg("black", QString( tr( "Total points %1\n").arg( total_points ) ) );

   // linearization of an arbitrary number of loops

   for ( unsigned int i = 0; i < total_points; i++ )
   {
      if ( !running )
      {
         break;
      }
      progress->setProgress(i, total_points);
      unsigned int pos = i;
      QString msg = "";
      for ( map < unsigned int, unsigned int >::iterator it = points.begin();
            it != points.end();
            it++ )
      {
         t_csv->setText(it->first, 6, QString("%1").arg(starts[it->first] + ( pos % it->second ) * increments[it->first]));
         pos /= it->second;
         msg += QString("%1 %2; ").arg(t_csv->text(it->first, 0)).arg(t_csv->text(it->first, 6));
      }
      editor_msg( "black", tr( "Running:" ) + msg );
      run_current();
   }

   running = false;
   progress->setProgress(1, 1);
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
      
      for ( int i = 0; i < lb_files->numRows(); i++ )
      {
         if ( lb_files->isSelected( i ) && 
              lb_files->text( i ) != lbl_buffer->text() &&
              lb_files->text( i ) != lbl_empty->text() )
         {
            selected_non_buffer_non_empty[ lb_files->text( i ) ] = true;
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
            progress->setProgress( pos++ , total_points );
         }
         lbl_signal->setText( it->first );
         qApp->processEvents();
         run_one();
         if ( !running )
         {
            lbl_signal->setText( save_signal );
            return;
         }
      }
      if ( !is_running )
      {
         running = false;
         update_enables();
         progress->setProgress(1, 1);
      }
   } else {
      run_one();
   }
}

void US_Hydrodyn_Saxs_Buffer::run_one()
{
   // subtract buffer
   QString buffer   = lbl_buffer  ->text();
   QString solution = lbl_signal  ->text();
   QString empty    = lbl_empty   ->text();

   map < QString, bool > current_files;

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      QString this_file = lb_files->text( i );
      current_files[ this_file ] = true;
   }

   if ( !current_files.count( buffer ) )
   {
      editor_msg( "red", QString( tr( "Error: no data found for buffer %1" ) ).arg( buffer ) );
      return;
   } 

   if ( !current_files.count( solution ) )
   {
      editor_msg( "red", QString( tr( "Error: no data found for solution %1" ) ).arg( solution ) );
      return;
   } 

   if ( !empty.isEmpty() && !current_files.count( empty ) )
   {
      editor_msg( "red", QString( tr( "Error: no data found for blank %1" ) ).arg( empty ) );
      return;
   } 

   if ( f_qs[ buffer ].size() != f_qs[ solution ].size() ||
        ( !empty.isEmpty() && f_qs[ buffer ].size() != f_qs[ empty ].size() ) )
   {
      editor_msg( "red", tr( "Error: incompatible grids, the files selected do not have the same number of points" ) );
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
      if ( fabs( bsub_q[ j ] - f_qs[ buffer ][ j ] ) > 5e-6 ||
           ( !empty.isEmpty() && bsub_q[ j ] != f_qs[ empty ][ j ] ) )
      {
         editor_msg( "red", tr( "Error: incompatible grids, the q values differ between selected files" ) );
         return;
      }
   }
   
   // determine parameters
   bool use_alpha = ((QCheckTableItem *)(t_csv->item( 0, 1 )))->isChecked();
   bool use_psv   = ((QCheckTableItem *)(t_csv->item( 1, 1 )))->isChecked();

   if ( ( !use_alpha && !use_psv ) || ( use_alpha && use_psv ) )
   {
      editor_msg( "red", tr( "Internal error: both alpha & psv methods active" ) );
      return;
   }

   if ( ( use_alpha && t_csv->text( 0, 6 ).isEmpty() ) ||
        ( use_psv && ( t_csv->text( 1, 6 ).isEmpty() ||
                       t_csv->text( 2, 6 ).isEmpty() ) ) )
   {
      editor_msg( "red", tr( "Internal error: method selected does not have current values" ) );
      return;
   }
      
   double alpha  = t_csv->text( 0, 6 ).toDouble();
   double psv    = t_csv->text( 1, 6 ).toDouble();
   double gamma  = t_csv->text( 2, 6 ).toDouble();

   map < QString, double > concs;

   QString msg;
   QString tag;

   if ( use_psv )
   {
      concs = current_concs();
      double this_conc;
      if ( !concs.count( solution ) || concs[ solution ] == 0e0 )
      {
         editor_msg( "dark red", tr( "Warning: the solution has zero concentration" ) );
         this_conc = 0e0;
      } else {
         this_conc = concs[ solution ];
      }
         
      alpha = 1e0 - gamma * this_conc * psv / 1000;
      msg = QString( tr( "alpha %1 gamma %2 conc %3 psv %4" ) )
         .arg( alpha )
         .arg( gamma )
         .arg( this_conc )
         .arg( psv );
   } else {
      msg = QString( tr( "alpha %1" ) ).arg( alpha );
   }

   // assuming zero covariance for now
   if ( buffer_has_errors && !solution_has_errors )
   {
      editor_msg( "dark red", tr( "Warning: the buffer has errors defined but not the solution" ) );
      
      bsub_error = f_errors[ buffer ];
   } else {
      if ( !buffer_has_errors && !solution_has_errors && empty_has_errors )
      {
         // this is a strange case
         editor_msg( "dark red", tr( "Warning: the blank has errors defined but not the solution or buffer!" ) );
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
   unsigned int negative_pos;
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
         editor_msg( "dark red", QString( tr( "Warning: non-positive values caused cropping at q = %1" ) ).arg( bsub_q[ negative_pos ] ) );
         result = 0;
      }
      if ( rb_np_min->isChecked() )
      {
         editor_msg( "dark red", QString( tr( "Warning: non-positive values caused %1 minimum values set" ) ).arg( negative_count ) );
         result = 1;
      }
      if ( rb_np_ignore->isChecked() )
      {
         editor_msg( "dark red", QString( tr( "Warning: %1 non-positive values ignored" ) ).arg( negative_count ) );
         result = 2;
      }
      if ( rb_np_ask->isChecked() )
      {
         result = QMessageBox::warning(this, 
                                       tr( "US-SOMO: SAXS Buffer Subtraction Utility" ),
                                       QString( tr( "Please note:\n\n"
                                                    "The buffer subtraction causes %1 points be non-positive\n"
                                                    "Starting at a q value of %2\n\n"
                                                    "What would you like to do?\n" ) )
                                       .arg( negative_count )
                                       .arg( bsub_q[ negative_pos ] ),
                                       tr( "&Crop the data" ), 
                                       tr( "&Set to the minimum positive value" ), 
                                       tr( "&Leave them negative or zero" ), 
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
                                       tr("US-SOMO: SAXS Buffer Subtraction Utility"),
                                       tr("Insufficient data left after cropping"));
               } else {
                  editor_msg( "red", tr( "Notice: Cropping left nothing" ) );
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

   QString head = solution + QString( "_bsub_a%1" ).arg( alpha );
   unsigned int ext = 0;

   QString bsub_name = head;

   while ( current_files.count( bsub_name ) )
   {
      bsub_name = head + QString( "-%1" ).arg( ++ext );
   }

   lb_created_files->insertItem( bsub_name );
   lb_created_files->setBottomItem( lb_created_files->numRows() - 1 );
   lb_files->insertItem( bsub_name );
   lb_files->setBottomItem( lb_files->numRows() - 1 );
   created_files_not_saved[ bsub_name ] = true;
   
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
   if ( conc_widget )
   {
      conc_window->refresh( csv_conc );
   }
   update_enables();
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
   unsigned int last_selected_pos;

   map < QString, bool > selected_map;

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         selected_map[ lb_files->text( i ) ] = true;
         last_selected_pos = i;
         files_selected_count++;
         if ( lb_files->text( i ) != lbl_buffer->text() &&
              lb_files->text( i ) != lbl_empty->text() )
         {
            non_buffer_non_empty_files_selected_count++;
         }
      }
   }

   lbl_selected->setText( QString( tr( "%1 of %2 files selected" ) )
                          .arg( files_selected_count )
                          .arg( lb_files->numRows() ) );

   unsigned int files_created_selected_not_saved_count = 0;
   unsigned int files_created_selected_count           = 0;
   unsigned int files_created_selected_not_shown_count = 0;
   map < QString, bool > created_selected_map;

   for ( int i = 0; i < lb_created_files->numRows(); i++ )
   {
      if ( lb_created_files->isSelected( i ) )
      {
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

   pb_conc               ->setEnabled( lb_files->numRows() > 0 );
   pb_clear_files        ->setEnabled( files_selected_count > 0 );
   pb_save_avg           ->setEnabled( files_selected_count > 1 );
   pb_conc_avg           ->setEnabled( all_selected_have_nonzero_conc() );
   pb_set_buffer         ->setEnabled( files_selected_count == 1 && 
                                       lb_files->text( last_selected_pos ) != lbl_buffer->text() &&
                                       lb_files->text( last_selected_pos ) != lbl_empty ->text() &&
                                       lb_files->text( last_selected_pos ) != lbl_signal->text()
                                       );
   pb_set_signal         ->setEnabled( files_selected_count == 1 && 
                                       lb_files->text( last_selected_pos ) != lbl_buffer->text() &&
                                       lb_files->text( last_selected_pos ) != lbl_empty ->text() &&
                                       lb_files->text( last_selected_pos ) != lbl_signal->text() );
   pb_set_empty          ->setEnabled( files_selected_count == 1 && 
                                       lb_files->text( last_selected_pos ) != lbl_buffer->text() &&
                                       lb_files->text( last_selected_pos ) != lbl_empty ->text() &&
                                       lb_files->text( last_selected_pos ) != lbl_signal->text() );
   pb_select_all         ->setEnabled( lb_files->numRows() > 0 );
   pb_invert             ->setEnabled( lb_files->numRows() > 0 );

   pb_select_all_created ->setEnabled( lb_created_files->numRows() > 0 );
   pb_save_created_csv   ->setEnabled( files_created_selected_count > 0 );
   pb_save_created       ->setEnabled( files_created_selected_not_saved_count > 0 );

   pb_show_created       ->setEnabled( files_created_selected_not_shown_count > 0 );
   pb_show_only_created  ->setEnabled( files_created_selected_count > 0 &&
                                       files_selected_not_created > 0 );

   bool any_best_empty    = false;
   bool any_current_empty = false;
   bool any_selected      = 
      ( ((QCheckTableItem *)(t_csv->item( 0, 1 )))->isChecked() ||
        ( ((QCheckTableItem *)(t_csv->item( 1, 1 )))->isChecked() &&
          ( ((QCheckTableItem *)(t_csv->item( 2, 1 )))->isChecked() ||
            !t_csv->text( 2, 6 ).isEmpty() ) )
        );
   if ( !running )
   {
      for ( unsigned int i = 0; i < (unsigned int)t_csv->numRows(); i++ )
      {
         if ( ((QCheckTableItem *)(t_csv->item( i, 1 )))->isChecked() )
         {
            if ( t_csv->text(i, 7).isEmpty() )
            {
               any_best_empty = true;
            }
            if ( t_csv->text(i, 6).isEmpty() )
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
   pb_run_best         ->setEnabled( !running && 
                                     ( !lbl_signal->text().isEmpty() ||
                                       ( cb_multi_sub->isChecked() && 
                                         non_buffer_non_empty_files_selected_count > 0 ) ) &&
                                     !lbl_buffer->text().isEmpty() &&
                                     !any_best_empty && any_selected);
   pb_stop             ->setEnabled( running );
   pb_replot_saxs      ->setEnabled( !running && *saxs_widget && names.size() );
   pb_save_saxs_plot   ->setEnabled( !running && *saxs_widget && saxs_window->qsl_plotted_iq_names.size() );
   pb_set_target       ->setEnabled( !running && *saxs_widget && saxs_window->qsl_plotted_iq_names.size() );
   cb_save_to_csv      ->setEnabled( !running );
   le_csv_filename     ->setEnabled( !running && cb_save_to_csv->isChecked() );
   cb_individual_files ->setEnabled( !running );

   if ( *saxs_widget )
   {
      saxs_window->update_iqq_suffix();
   }
}

bool US_Hydrodyn_Saxs_Buffer::any_to_run()
{
   for ( unsigned int i = 0; i < (unsigned int)t_csv->numRows(); i++ )
   {
      if ( ((QCheckTableItem *)(t_csv->item( i, 1 )))->isChecked() )
      {
         return true;
      }
   }
   editor_msg("dark red", "Nothing to do: At least one of the rows must have Active set to Y");
   return false;
}

void US_Hydrodyn_Saxs_Buffer::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->color();
   editor->setColor(color);
   editor->append(msg);
   editor->setColor(save_color);
}

void US_Hydrodyn_Saxs_Buffer::editor_msg_qc( QColor qcolor, QString msg )
{
   QColor save_color = editor->color();
   editor->setColor(qcolor);
   editor->append(msg);
   editor->setColor(save_color);
}

bool US_Hydrodyn_Saxs_Buffer::validate()
{
   bool errors = false;

   for ( unsigned int i = 0; i < (unsigned int)t_csv->numRows(); i++ )
   {
      if ( t_csv->text( i, 2 ).toDouble() > t_csv->text( i, 3 ).toDouble() )
      {
         editor_msg("red", QString("Row %1 column \"Low value\" can not be greater than \"High value\"\n").arg(i));
         errors = true;
      }
      if ( t_csv->text( i, 4 ).toInt() < 1 )
      {
         editor_msg("red", QString("Row %1 column \"Points\" must be greater or equal to one\n").arg(i));
         errors = true;
      }
      if ( t_csv->text( i, 4 ).toDouble() == 1  &&
           t_csv->text( i, 2 ).toDouble() > t_csv->text( i, 3 ).toDouble() )
      {
         editor_msg("red", QString("Row %1 one \"Points\" requires \"Low value\" equals \"High value\"\n").arg(i));
         errors = true;
      }
   }
   errors |= !validate_saxs_window();
   return !errors;
}

bool US_Hydrodyn_Saxs_Buffer::activate_saxs_window()
{
   if ( !*saxs_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->pdb_saxs();
      raise();
      if ( !*saxs_widget )
      {
         editor_msg("red", tr("Could not activate SAXS window!\n"));
         return false;
      }
   }
   return true;
}

bool US_Hydrodyn_Saxs_Buffer::validate_saxs_window()
{
   if ( !activate_saxs_window() )
   {
      return false;
   }

   if ( !lbl_current_target->text().isEmpty() &&
        !saxs_window->qsl_plotted_iq_names.grep(QRegExp(QString("^%1$").arg(lbl_current_target->text()))).size() )
   {
      // need to replot
      editor_msg("dark red", "Current target missing from SAXS window plot, replotting");
      do_replot_saxs();
   }

   if ( !saxs_window->selected_models.size() )
   {
      editor_msg("red", "No model is currently available in the SAXS window\n");
      return false;
   }

   if ( !((US_Hydrodyn *)us_hydrodyn)->overwrite &&
        ( cb_individual_files->isChecked() ||
          ( !saxs_window->our_saxs_options->saxs_sans &&
            ( saxs_window->our_saxs_options->saxs_iq_foxs ||
              saxs_window->our_saxs_options->saxs_iq_crysol ) ) ||
          ( saxs_window->our_saxs_options->saxs_sans &&
            saxs_window->our_saxs_options->saxs_iq_crysol )
          )
        )
   {
      switch ( QMessageBox::warning(this, 
                                    tr("UltraScan Warning"),
                                    QString(tr("Please note:\n\n"
                                               "Overwriting of existing files currently off.\n"
                                               "This could cause Searching to block during processing.\n"
                                               "What would you like to do?\n")),
                                    tr("&Stop"), 
                                    tr("&Turn on overwrite now"),
                                    tr("C&ontinue anyway"),
                                    0, // Stop == button 0
                                    0 // Escape == button 0
                                    ) )
      {
      case 0 : // stop
         return false;
         break;
      case 1 :
         ((US_Hydrodyn *)us_hydrodyn)->overwrite = true;
         ((US_Hydrodyn *)us_hydrodyn)->cb_overwrite->setChecked(true);
         break;
      case 2 : // continue
         break;
      }
   }

   return true;
}

void US_Hydrodyn_Saxs_Buffer::save_to_csv()
{
   update_enables();
}

void US_Hydrodyn_Saxs_Buffer::do_replot_saxs()
{
   saxs_window->clear_plot_saxs(true);
   saxs_window->plotted = false;
   for ( unsigned int i = 0; i < names.size(); i++ )
   {
      saxs_window->plot_one_iqq( qs[ i ], Is[ i ], I_errors[ i ], names[ i ] );
   }
   if ( saxs_window->plotted )
   {
      saxs_window->editor->setParagraphBackgroundColor ( saxs_window->editor->paragraphs() - 1, QColor("white") );
      saxs_window->editor->append("I(q) plot done\n");
      saxs_window->plotted = false;
   }
   saxs_window->rescale_plot();
}

void US_Hydrodyn_Saxs_Buffer::replot_saxs()
{
   if ( activate_saxs_window() )
   {
      do_replot_saxs();
   }
}

void US_Hydrodyn_Saxs_Buffer::save_saxs_plot()
{
   if ( !activate_saxs_window() )
   {
      return;
   }

   if ( !saxs_window->plotted_q.size() )
   {
      QMessageBox::information(this, 
                               tr("US-SOMO: I(q) search"),
                               QString(tr("Nothing plotted")));
      return;
   }

   bool add_target = false;
   if ( !lbl_current_target->text().isEmpty() )
   {
      bool current_target_in_plot = false;
      for ( unsigned int i = 0; i < saxs_window->plotted_I_error.size(); i++ )
      {
         if ( saxs_window->qsl_plotted_iq_names[ i ] == lbl_current_target->text() )
         {
            current_target_in_plot = true;
            break;
         }
      }
      if ( !current_target_in_plot )
      {
         switch( QMessageBox::information( this, 
                                           "US_SOMO: I(q) search",
                                           tr("The target is not currently plotted"),
                                           tr("Clear target and continue"), 
                                           tr("Add target to stored plots"), 
                                           tr("Cancel"),
                                           0,      // Enter == button 0
                                           2 ) ) { // Escape == button 2
         case 0:
            lbl_current_target->setText("");
            break;
         case 1: 
            add_target = true;
            break;
         case 2: // Cancel clicked or Escape pressed
            return;
            break;
         }
      }
   }    
         
   vector < double > add_q;
   vector < double > add_I;
   vector < double > add_I_error;
   if ( add_target )
   {
      add_target = false;
      for ( unsigned int i = 0; i < names.size(); i++ )
      {
         if ( names[i] == lbl_current_target->text() )
         {
            add_q       = qs[ i ];
            add_I       = Is[ i ];
            add_I_error = I_errors[ i ];
            add_target = true;
            break;
         }         
      }
      if ( !add_target )
      {
         QMessageBox::warning(this, 
                              tr("US-SOMO: SAXS Buffer Subtraction Utility"),
                              tr("Target not found, clearing"));
         lbl_current_target->setText("");
      }
   }

   qs      .clear();
   Is      .clear();
   I_errors.clear();
   names   .clear();

   if ( add_target )
   {
      qs.      push_back( add_q );
      Is.      push_back( add_I );
      I_errors.push_back( add_I_error );
      names.   push_back( lbl_current_target->text() );
   }

   for ( unsigned int i = 0; i < saxs_window->plotted_I_error.size(); i++ )
   {
      qs.      push_back( saxs_window->plotted_q           [ i ] );
      Is.      push_back( saxs_window->plotted_I           [ i ] );
      I_errors.push_back( saxs_window->plotted_I_error     [ i ] );
      names.   push_back( saxs_window->qsl_plotted_iq_names[ i ] );
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Buffer::save_csv_saxs_iqq()
{
   QString fname = 
      ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "saxs" + SLASH + 
      le_csv_filename->text() + "_iqq_search.csv";

   if ( QFile::exists(fname) )
      // && !((US_Hydrodyn *)us_hydrodyn)->overwrite ) 
   {
      fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fname);
   }         

   FILE *of = fopen(fname, "wb");
   if ( of )
   {
      //  header: "name","type",q1,q2,...,qn, header info
      fprintf(of, "\"Name\",\"Type; q:\",%s,\"%s\"\n", 
              saxs_window->vector_double_to_csv(saxs_q).ascii(),
              saxs_header_iqq.remove("\n").ascii());
      for ( unsigned int i = 0; i < csv_source_name_iqq.size(); i++ )
      {
         fprintf(of, "\"%s\",\"%s\",%s\n", 
                 csv_source_name_iqq[i].ascii(),
                 "I(q)",
                 saxs_window->vector_double_to_csv(saxs_iqq[i]).ascii());
      }
      fprintf(of, "\n");

      fclose(of);
      editor->append(tr("Created file: " + fname + "\n"));
   } else {
      editor_msg("red",tr("ERROR creating file: " + fname + "\n"));
   }
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
            tmp_csv.data[i][j] = ((QCheckTableItem *)(t_csv->item( i, j )))->isChecked() ? "Y" : "N";
         } else {
            tmp_csv.data[i][j] = t_csv->text( i, j );
         }
         tmp_csv.num_data[i][j] = tmp_csv.data[i][j].toDouble();
      }
   }
   return tmp_csv;
}
  
void US_Hydrodyn_Saxs_Buffer::recompute_interval_from_points()
{
   for ( unsigned int i = 0; i < (unsigned int)t_csv->numRows(); i++ )
   {
      t_csv->setText(
                     i, 5, 
                     ( 
                      t_csv->text(i, 4).toDouble() == 0e0 ?
                      ""
                      :
                      QString("%1")
                      .arg( ( t_csv->text(i, 3).toDouble() -
                              t_csv->text(i, 2).toDouble() )
                             / ( t_csv->text(i, 4).toDouble() - 1e0 ) ) 
                      )
                     );
   }
}


void US_Hydrodyn_Saxs_Buffer::recompute_points_from_interval()
{
   for ( unsigned int i = 0; i < (unsigned int)t_csv->numRows(); i++ )
   {
      t_csv->setText(
                     i, 4, 
                     ( 
                      t_csv->text(i, 5).toDouble() == 0e0 ?
                      ""
                      :
                      QString("%1")
                      .arg( 1 + (unsigned int)(( t_csv->text(i, 3).toDouble() -
                                                 t_csv->text(i, 2).toDouble() )
                                               / t_csv->text(i, 5).toDouble() + 0.5) ) 
                      )
                     );
   }
}

void US_Hydrodyn_Saxs_Buffer::clear_files()
{
   disable_updates = true;

   QStringList           created_not_saved_list;
   map < QString, bool > created_not_saved_map;
   map < QString, bool > selected_map;

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         selected_map[ lb_files->text( i ) ] = true;
         if ( created_files_not_saved.count( lb_files->text( i ) ) )
         {
            created_not_saved_list << lb_files->text( i );
            created_not_saved_map[ lb_files->text( i ) ] = true;
         }
      }
   }

   if ( created_not_saved_list.size() )
   {
      switch ( QMessageBox::warning(this, 
                                    tr( "US-SOMO: SAXS Buffer Subtraction Utility Remove Files" ),
                                    QString( tr( "Please note:\n\n"
                                                 "These files were created but not saved as .dat files:\n"
                                                 "%1\n\n"
                                                 "What would you like to do?\n" ) )
                                    .arg( created_not_saved_list.join( "\n" ) ),
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
      if ( lb_files->isSelected( i ) )
      {
         editor_msg( "black", QString( tr( "Removed %1" ) ).arg( lb_files->text( i ) ) );
         if ( lbl_buffer->text() == lb_files->text( i ) )
         {
            lbl_buffer->setText( "" );
         }
         if ( lbl_signal->text() == lb_files->text( i ) )
         {
            lbl_signal->setText( "" );
         }
         if ( lbl_empty->text() == lb_files->text( i ) )
         {
            lbl_empty->setText( "" );
         }
         f_qs_string.erase( lb_files->text( i ) );
         f_qs       .erase( lb_files->text( i ) );
         f_Is       .erase( lb_files->text( i ) );
         f_errors   .erase( lb_files->text( i ) );
         f_pos      .erase( lb_files->text( i ) );
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
   update_enables();
}

void US_Hydrodyn_Saxs_Buffer::add_files()
{
   map < QString, bool > existing_items;
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      existing_items[ lb_files->text( i ) ] = true;
   }

   QString use_dir = QDir::currentDirPath();
   
   if ( !*saxs_widget )
   {
      // try and activate
      ((US_Hydrodyn *)us_hydrodyn)->pdb_saxs();
   }

   if ( *saxs_widget )
   {
      saxs_window->select_from_directory_history( use_dir );
   }

   QStringList filenames = QFileDialog::getOpenFileNames(
                                                         "dat files [foxs / other] (*.dat);;"
                                                         "All files (*);;"
                                                         // "ssaxs files (*.ssaxs);;"
                                                         // "csv files (*.csv);;"
                                                         // "int files [crysol] (*.int);;"
                                                         // "fit files [crysol] (*.fit);;"
                                                         , use_dir
                                                         , this
                                                         , "open file dialog"
                                                         , "Set files for grid files"
                                                         );
   
   QStringList add_filenames;
   
   if ( filenames.size() )
   {
      last_load_dir = QFileInfo( filenames[ 0 ] ).dirPath();
      QDir::setCurrent( last_load_dir );
      editor_msg( "black", QString( tr( "loaded from %1:" ) ).arg( last_load_dir ) );
   }

   QString errors;

   for ( unsigned int i = 0; i < filenames.size(); i++ )
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
         errors += QString( tr( "Duplicate name not loaded %1" ) ).arg( basename );
      }
   }

   if ( errors.isEmpty() )
   {
      editor_msg( "blue", tr( "Files loaded ok" ) );
   } else {
      editor_msg( "red", errors );
   }

   lb_files->insertStringList( add_filenames );

   if ( add_filenames.size() )
   {
      lb_files->setBottomItem( lb_files->numRows() - 1 );
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
   plot_dist->clear();
   bool any_selected = false;
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
         any_selected = true;
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
      plot_dist_zoomer->setCursorLabelPen(QPen(Qt::yellow));
   }
   
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
   long Iq = plot_dist->insertCurve( "I(q) vs q" );
   plot_dist->setCurveStyle( Iq, QwtCurve::Lines );
#else
   QwtPlotCurve *curve = new QwtPlotCurve( "I(q) vs q" );
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

void US_Hydrodyn_Saxs_Buffer::invert()
{
   lb_files->invertSelection();
}

void US_Hydrodyn_Saxs_Buffer::select_all()
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

void US_Hydrodyn_Saxs_Buffer::select_all_created()
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

bool US_Hydrodyn_Saxs_Buffer::load_file( QString filename )
{
   errormsg = "";
   QFile f( filename );
   if ( !f.exists() )
   {
      errormsg = QString("Error: %1 does not exist").arg( filename );
      return false;
   }
   
   QString ext = QFileInfo( filename ).extension( false ).lower();

   QRegExp rx_valid_ext (
                         "^("
                         "dat|"
                         "int|"
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

   vector < QString > q_string;
   vector < double >  q;
   vector < double >  I;

   QRegExp rx_ok_line("^(\\s+|\\d+|\\.|\\d(E|e)(\\+|-|\\d))+$");
   rx_ok_line.setMinimal( true );
   for ( unsigned int i = 1; i < (unsigned int) qv.size(); i++ )
   {
      if ( qv[i].contains(QRegExp("^#")) ||
           rx_ok_line.search( qv[i] ) == -1 )
      {
         continue;
      }
      
      QStringList tokens = QStringList::split(QRegExp("\\s+"), qv[i].replace(QRegExp("^\\s+"),""));

      if ( tokens.size() > 1 )
      {
         QString this_q_string = tokens[ 0 ];
         double this_q         = tokens[ 0 ].toDouble();
         double this_I         = tokens[ 1 ].toDouble();
         if ( q.size() && this_q <= q[ q.size() - 1 ] )
         {
            cout << QString(" breaking %1 %2\n").arg( this_q ).arg( q[ q.size() - 1 ] );
            break;
         }
         q_string.push_back( this_q_string );
         q       .push_back( this_q );
         I       .push_back( this_I );
      }
   }

   QString basename = QFileInfo( filename ).baseName( true );
   f_pos       [ basename ] = f_qs.size();
   f_qs_string [ basename ] = q_string;
   f_qs        [ basename ] = q;
   f_Is        [ basename ] = I;
   f_errors    .erase( basename );
   return true;
}

void US_Hydrodyn_Saxs_Buffer::set_buffer()
{
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         lbl_buffer->setText( lb_files->text( i ) );
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
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         lbl_signal->setText( lb_files->text( i ) );
      }
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Buffer::set_empty()
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

void US_Hydrodyn_Saxs_Buffer::update_created_files()
{
   if ( !disable_updates )
   {
      update_enables();
   }
}

void US_Hydrodyn_Saxs_Buffer::save_avg()
{
   // create average of selected
   bool first = true;

   vector < QString > avg_qs_string;
   vector < double >  avg_qs;
   vector < double >  avg_Is;
   vector < double >  avg_Is2;

   QStringList selected_files;
   map < QString, bool > current_files;

   unsigned int selected_count = 0;

   update_csv_conc();
   map < QString, double > concs = current_concs();
   double avg_conc;

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      QString this_file = lb_files->text( i );
      current_files[ this_file ] = true;

      if ( lb_files->isSelected( i ) )
      {
         selected_count++;
         selected_files << this_file;
         if ( first )
         {
            first = false;
            avg_qs_string = f_qs_string[ this_file ];
            avg_qs        = f_qs       [ this_file ];
            avg_Is        = f_Is       [ this_file ];
            avg_Is2       .resize( f_Is[ this_file ].size() );
            for ( unsigned int j = 0; j < f_Is[ this_file ].size(); j++ )
            {
               avg_Is2[ j ] = f_Is[ this_file ][ j ] * f_Is[ this_file ][ j ];
            }
            avg_conc = 
               concs.count( this_file ) ?
               concs[ this_file ] :
               0e0;
         } else {
            if ( avg_qs.size() != f_qs[ this_file ].size() )
            {
               editor_msg( "red", tr( "Error: incompatible grids, the files selected do not have the same number of points" ) );
               return;
            }
            for ( unsigned int j = 0; j < f_Is[ this_file ].size(); j++ )
            {
               if ( fabs( avg_qs[ j ] - f_qs[ this_file ][ j ] ) > 5e-6 )
               {
                  editor_msg( "red", tr( "Error: incompatible grids, the q values differ between selected files" ) );
                  return;
               }
               avg_Is [ j ] += f_Is[ this_file ][ j ];
               avg_Is2[ j ] += f_Is[ this_file ][ j ] * f_Is[ this_file ][ j ];
            }
            avg_conc +=
               concs.count( this_file ) ?
               concs[ this_file ] :
               0e0;
         }            
      }
   }

   if ( selected_count < 2 )
   {
      editor_msg( "red", tr( "Error: not at least 2 files selected so there is nothing to average" ) );
      return;
   }      

   vector < double > avg_sd( avg_qs.size() );
   double dsc = ( double ) selected_count;
   double dsc_inv = 1e0 / dsc;
   double dsc_times_dsc_minus_one_inv = 1e0 / ( dsc * ( dsc - 1e0 ) );
   for ( unsigned int i = 0; i < avg_qs.size(); i++ )
   {
      avg_sd[ i ] =
         sqrt( 
              ( dsc * avg_Is2[ i ] - avg_Is[ i ] * avg_Is[ i ] ) 
              * dsc_times_dsc_minus_one_inv 
              );

      avg_Is[ i ] *= dsc_inv;
   }

   avg_conc *= dsc_inv;

   // determine name
   // find common header & tail substrings

   QString head = qstring_common_head( selected_files );
   for ( unsigned int i = 0; i < selected_files.size(); i++ )
   {
      selected_files[ i ] = selected_files[ i ].right( selected_files[ i ].length() - head.length() );
   }
   QString tail = qstring_common_tail( selected_files );

   unsigned int ext = 0;

   if ( !head.isEmpty() )
   {
      head += "_";
   }
   if ( !tail.isEmpty() )
   {
      tail = "_" + tail;
   }

   QString avg_name = head + "avg" + tail;

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
   
   // we could check if it has changed and then delete
   // if ( plot_dist_zoomer )
   // {
   // delete plot_dist_zoomer;
   // plot_dist_zoomer = (ScrollZoomer *) 0;
   // }
   update_csv_conc();
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

QString US_Hydrodyn_Saxs_Buffer::qstring_common_head( QStringList qsl )
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
   for ( unsigned int i = 1; i < qsl.size(); i++ )
   {
      s = qstring_common_head( s, qsl[ i ] );
   }
   return s;
}

QString US_Hydrodyn_Saxs_Buffer::qstring_common_tail( QStringList qsl )
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
   for ( unsigned int i = 1; i < qsl.size(); i++ )
   {
      s = qstring_common_tail( s, qsl[ i ] );
   }
   return s;
}
      
QString US_Hydrodyn_Saxs_Buffer::qstring_common_head( QString s1, 
                                                      QString s2 )
{
   unsigned int min_len = s1.length();
   if ( min_len > s2.length() )
   {
      min_len = s2.length();
   }

   // could do this more efficiently via "divide & conquer"
   // i.e. split the distance in halfs and compare 
   
   unsigned int match_max = 0;
   for ( unsigned int i = 1; i <= min_len; i++ )
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
   unsigned int min_len = s1.length();
   if ( min_len > s2.length() )
   {
      min_len = s2.length();
   }

   // could do this more efficiently via "divide & conquer"
   // i.e. split the distance in halfs and compare 
   
   unsigned int match_max = 0;
   for ( unsigned int i = 1; i < min_len; i++ )
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

void US_Hydrodyn_Saxs_Buffer::save_created_csv()
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

bool US_Hydrodyn_Saxs_Buffer::save_files_csv( QStringList /* files */ )
{
   return false;
}

bool US_Hydrodyn_Saxs_Buffer::save_files( QStringList files )
{

   bool errors = false;
   for ( unsigned int i = 0; i < files.size(); i++ )
   {
      if ( !save_file( files[ i ] ) )
      {
         errors = true;
      }
   }
   update_enables();
   return !errors;
}

bool US_Hydrodyn_Saxs_Buffer::save_file( QString file )
{
   if ( !QDir::setCurrent( last_load_dir ) )
   {
      editor_msg( "red", QString( tr( "Error: can not set directory %1" ) ).arg( last_load_dir ) );
      return false;
   }

   if ( !f_qs.count( file ) )
   {
      editor_msg( "red", QString( tr( "Error: no data found for %1" ) ).arg( file ) );
      return false;
   } 

   QString use_filename = file + ".dat";

   if ( QFile::exists( use_filename ) )
   {
      use_filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( use_filename );
   }

   QFile f( use_filename );
   if ( !f.open( IO_WriteOnly ) )
   {
      editor_msg( "red", QString( tr( "Error: can not open %1 for writing" ) ).arg( use_filename ) );
      return false;
   }

   QTextStream ts( &f );

   ts << QString( tr( "US-SOMO Buffer Subtraction utility output: %1\n" ) ).arg( file );

   bool use_errors = ( f_errors.count( file ) && 
                       f_errors[ file ].size() > 0 );

   if ( use_errors )
   {
      ts << "q\tI(q)\tsd\n";
   } else {
      ts << "q\tI(q)\n";
   }

   for ( unsigned int i = 0; i < f_qs[ file ].size(); i++ )
   {
      if ( use_errors &&
           f_errors[ file ].size() > i )
      {
         ts << QString("").sprintf( "%s\t%.6e\t%.6e\n",
                                    f_qs_string[ file ][ i ].ascii(),
                                    f_Is       [ file ][ i ],
                                    f_errors   [ file ][ i ] );
      } else {
         ts << QString("").sprintf( "%s\t%.6e\n",
                                    f_qs_string[ file ][ i ].ascii(),
                                    f_Is       [ file ][ i ] );
      }
   }

   f.close();
   editor_msg( "black", QString( tr( "%1 written as %2" ) )
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
                              tr( "US-SOMO: SAXS Buffer Subtraction Utility" ),
                              tr( "There are unsaved updates in the open Solution Concentration window\n"
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
   // create average of selected
   bool first = true;

   vector < QString > avg_qs_string;
   vector < double >  avg_qs;
   vector < double >  avg_Is;
   vector < double >  avg_Is2;

   QStringList selected_files;
   map < QString, bool > current_files;

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
         inv_concs[ it->first ] = /* 1e0 / */ it->second;
      }
   }

   double avg_conc;
   vector < double > nIs;

   double tot_conc;
   double tot_conc2;

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      QString this_file = lb_files->text( i );
      current_files[ this_file ] = true;

      if ( lb_files->isSelected( i ) )
      {
         selected_count++;
         selected_files << this_file;
         if ( !inv_concs.count( this_file ) )
         {
            editor_msg( "red", QString( tr( "Error: found zero or no concentration for %1" ) ).arg( this_file ) );
            return;
         }
         cout << QString( "inv conc for %1 is %2\n" ).arg( this_file ).arg( inv_concs[ this_file ] );
         if ( first )
         {
            first = false;
            tot_conc  = inv_concs[ this_file ];
            tot_conc2 = tot_conc * tot_conc;
            avg_qs_string = f_qs_string[ this_file ];
            avg_qs        = f_qs       [ this_file ];
            nIs           = f_Is       [ this_file ];
            for ( unsigned int j = 0; j < nIs.size(); j++ )
            {
               nIs[ j ] *= inv_concs[ this_file ];
            }

            avg_Is        = nIs;
            avg_Is2       .resize( nIs.size() );
            for ( unsigned int j = 0; j < nIs.size(); j++ )
            {
               avg_Is2[ j ] = nIs[ j ] * nIs[ j ];
            }
            avg_conc = 
               concs.count( this_file ) ?
               concs[ this_file ] :
               0e0;
         } else {
            if ( avg_qs.size() != f_qs[ this_file ].size() )
            {
               editor_msg( "red", tr( "Error: incompatible grids, the files selected do not have the same number of points" ) );
               return;
            }
            tot_conc  += inv_concs[ this_file ];
            tot_conc2 += inv_concs[ this_file ] * inv_concs[ this_file ];
            nIs       = f_Is     [ this_file ];
            for ( unsigned int j = 0; j < nIs.size(); j++ )
            {
               if ( fabs( avg_qs[ j ] - f_qs[ this_file ][ j ] ) > 5e-6 )
               {
                  editor_msg( "red", tr( "Error: incompatible grids, the q values differ between selected files" ) );
                  return;
               }
               nIs[ j ] *= inv_concs[ this_file ];
               avg_Is [ j ] += nIs[ j ];
               avg_Is2[ j ] =  nIs[ j ] * nIs[ j ];
            }
            avg_conc +=
               concs.count( this_file ) ?
               concs[ this_file ] :
               0e0;
         }            
      }
   }

   if ( selected_count < 2 )
   {
      editor_msg( "red", tr( "Error: not at least 2 files selected so there is nothing to average" ) );
      return;
   }      

   vector < double > avg_sd( avg_qs.size() );
   double dsc = tot_conc;
   double dsc_inv = 1e0 / dsc;
   // double dsc_times_dsc_minus_one_inv = 1e0 / ( dsc * ( dsc - 1e0 ) );
   for ( unsigned int i = 0; i < avg_qs.size(); i++ )
   {
      if ( !i )
      {
         cout << QString( "points %1 avg %1 sum2 %1\n" )
            .arg( selected_count )
            .arg( avg_Is[ i ] )
            .arg( avg_Is2[ i ] );
      }

      avg_Is[ i ] *= dsc_inv;

      avg_sd[ i ] = 0e0;
      
      for ( unsigned int j = 0; j < selected_files.size(); j++ )
      {
         QString this_file = selected_files[ j ];
         avg_sd[ i ] += inv_concs[ this_file ] * 
            ( f_Is[ this_file ][ i ] - avg_Is[ i ] ) * ( f_Is[ this_file ][ i ] - avg_Is[ i ] );
      }
      avg_sd[ i ] *= ( tot_conc / ( tot_conc * tot_conc - tot_conc2 ) );
   }

   avg_conc *= dsc_inv;

   // determine name
   // find common header & tail substrings

   QString head = qstring_common_head( selected_files );
   for ( unsigned int i = 0; i < selected_files.size(); i++ )
   {
      selected_files[ i ] = selected_files[ i ].right( selected_files[ i ].length() - head.length() );
   }
   QString tail = qstring_common_tail( selected_files );

   unsigned int ext = 0;

   if ( !head.isEmpty() )
   {
      head += "_";
   }
   if ( !tail.isEmpty() )
   {
      tail = "_" + tail;
   }

   QString avg_name = head + "cnavg" + tail;

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
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
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

void US_Hydrodyn_Saxs_Buffer::delete_zoomer_if_ranges_changed()
{
}

void US_Hydrodyn_Saxs_Buffer::show_created()
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

void US_Hydrodyn_Saxs_Buffer::show_only_created()
{
   disable_updates = true;
   lb_files->clearSelection();
   show_created();
}

