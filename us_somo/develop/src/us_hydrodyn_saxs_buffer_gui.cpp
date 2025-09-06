#include "../include/us_hydrodyn_saxs_buffer.h"

void US_Hydrodyn_Saxs_Buffer::setupGUI()
{
   int minHeight1 = 21;
   int minHeight3 = 23;

   // lbl_title = new QLabel(csv1.name.left(80), this);
   // lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   // lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   // lbl_title->setMinimumHeight(minHeight1);
   // lbl_title->setPalette( PALET_FRAME );
   // lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

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

   QFrame * fit_frame = new QFrame();
   fit_frame->setPalette( PALET_FRAME );
   fit_frame->setWindowTitle( windowTitle() + us_tr( " : Fitting" ) );
   
   pb_start = new QPushButton(us_tr("Buffer subtraction over range"), fit_frame );
   pb_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_start->setMinimumHeight(minHeight1);
   pb_start->setPalette( PALET_PUSHB );
   connect(pb_start, SIGNAL(clicked()), SLOT(start()));

   pb_run_current = new QPushButton(us_tr("Current value buffer subtraction"), fit_frame );
   pb_run_current->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_run_current->setMinimumHeight(minHeight1);
   pb_run_current->setPalette( PALET_PUSHB );
   connect(pb_run_current, SIGNAL(clicked()), SLOT(run_current()));

   pb_run_divide = new QPushButton(us_tr("Current value buffer divide"), fit_frame );
   pb_run_divide->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_run_divide->setMinimumHeight(minHeight1);
   pb_run_divide->setPalette( PALET_PUSHB );
   connect(pb_run_divide, SIGNAL(clicked()), SLOT(run_divide()));

   pb_run_best = new QPushButton(us_tr("Best buffer subtraction"), fit_frame );
   pb_run_best->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_run_best->setMinimumHeight(minHeight1);
   pb_run_best->setPalette( PALET_PUSHB );
   connect(pb_run_best, SIGNAL(clicked()), SLOT(run_best()));

   pb_stop = new QPushButton(us_tr("Stop"), fit_frame );
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

   t_csv = new QTableWidget(csv1.data.size(), csv1.header.size(), fit_frame );
   t_csv->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   t_csv->setMinimumHeight(minHeight1 * ( 1 + csv1.data.size() ) );
   t_csv->setMaximumHeight(minHeight1 * ( 2 + csv1.data.size() ) );
   // t_csv->setMinimumWidth(minWidth1);
   t_csv->setPalette( PALET_EDIT );
   AUTFBACK( t_csv );
   t_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2, QFont::Bold));
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

   t_csv->verticalHeader()->setDefaultSectionSize( 18 );
   t_csv->verticalHeader()->sectionResizeMode( QHeaderView::Fixed );

   recompute_interval_from_points();

   connect(t_csv, SIGNAL(cellChanged(int, int)), SLOT(table_value(int, int )));

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

   lbl_np = new QLabel( "Buffer subtraction non-positive:    ", fit_frame );
   lbl_np->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_np->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_np );
   lbl_np->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));

   rb_np_crop = new QRadioButton( us_tr("Crop "), fit_frame );
   rb_np_crop->setEnabled(true);
   rb_np_crop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   rb_np_crop->setPalette( PALET_NORMAL );
   AUTFBACK( rb_np_crop );

   rb_np_min = new QRadioButton( us_tr("Set to minimum "), fit_frame );;
   rb_np_min->setEnabled(true);
   rb_np_min->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   rb_np_min->setPalette( PALET_NORMAL );
   AUTFBACK( rb_np_min );

   rb_np_ignore = new QRadioButton( us_tr("Ignore (log of non-positive not defined)"), fit_frame );
   rb_np_ignore->setEnabled(true);
   rb_np_ignore->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   rb_np_ignore->setPalette( PALET_NORMAL );
   AUTFBACK( rb_np_ignore );

   rb_np_ask = new QRadioButton( us_tr("Ask (blocks mass processing) "), fit_frame );
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
   bg_np = new QGroupBox( fit_frame );
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

   cb_multi_sub = new QCheckBox( fit_frame );
   cb_multi_sub->setText(us_tr(" Subtract buffer from every selected file (exepting set buffer and set blank) " ) );
   cb_multi_sub->setChecked(false);
   cb_multi_sub->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   cb_multi_sub->setPalette( PALET_NORMAL );
   AUTFBACK( cb_multi_sub );
   connect( cb_multi_sub, SIGNAL( clicked() ), SLOT( update_enables() ) );

   cb_multi_sub_avg = new QCheckBox( this );
   cb_multi_sub_avg->setText(us_tr(" Average" ) );
   cb_multi_sub_avg->setChecked(false);
   cb_multi_sub_avg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   cb_multi_sub_avg->setPalette( PALET_NORMAL );
   AUTFBACK( cb_multi_sub_avg );

   cb_multi_sub_conc_avg = new QCheckBox( this );
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

   /*
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
   */
   
   QGridLayout * gl_bottom = new QGridLayout( 0 ); gl_bottom->setContentsMargins( 0, 0, 0, 0 ); gl_bottom->setSpacing( 0 );
   gl_bottom->addWidget( pb_help  , 0, 0 );
   gl_bottom->addWidget( progress , 0, 1 );
   gl_bottom->addWidget( pb_cancel, 0, 2 );
   
   QVBoxLayout * background = new QVBoxLayout(this); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );
   background->addSpacing( 1 );
   // background->addWidget ( lbl_title );
   // background->addSpacing( 1 );
   background->addLayout ( gl_files_plot );
   /*
   background->addWidget ( t_csv );
   background->addSpacing( 1 );
   background->addLayout ( hbl_np );
   background->addSpacing( 1 );
   background->addLayout ( hbl_multi );
   background->addSpacing( 1 );
   background->addLayout ( hbl_controls );
   */
   background->addSpacing( 1 );
   background->addLayout ( gl_bottom );
   background->addSpacing( 1 );

   if ( !U_EXPT )
   {
      pb_asum->hide();
   }

   // setup fit widget

   // QFrame * fit_frame = new QFrame();
   // fit_frame->setWindowTitle( windowTitle() + us_tr( " : Fitting" ) );
   // QFormLayout form(fit_frame);
   
   // form.addRow( new QLabel(
   //                         us_tr(
   //                               "Fitting controls\n"
   //                               )
   //                         ) );

   // needs geometry
   fit_frame->setGeometry( 100, 100, 0, 0 );
   // fit_frame->setStyleSheet("QFrame { border: 1px solid black; }");

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

   QVBoxLayout * fit_background = new QVBoxLayout( fit_frame ); fit_background->setContentsMargins( 0, 0, 0, 0 ); fit_background->setSpacing( 0 );

   fit_background->addWidget ( t_csv );
   fit_background->addSpacing( 1 );
   fit_background->addLayout ( hbl_np );
   fit_background->addSpacing( 1 );
   fit_background->addLayout ( hbl_multi );
   fit_background->addSpacing( 1 );
   fit_background->addLayout ( hbl_controls );

   fit_frame->adjustSize();
   fit_frame->show();
   fit_frame->raise();

   fit_widget = (QWidget *) fit_frame;

   //   fit_frame->setWindowFlags( fit_frame->windowFlags() & ~Qt::WindowCloseButtonHint );
}
