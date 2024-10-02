#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_saxs_screen.h"
//Added by qt3to4:
#include <QBoxLayout>
#include <QLabel>
#include <QCloseEvent>
#include <QGridLayout>
#include <QTextStream>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
 //#include <Q3PopupMenu>

#define SLASH QDir::separator()

US_Hydrodyn_Saxs_Screen::US_Hydrodyn_Saxs_Screen(
                                               csv csv1,
                                               void *us_hydrodyn, 
                                               QWidget *p, 
                                               const char *
                                               ) : QFrame( p )
{
   this->csv1 = csv1;
   this->us_hydrodyn = us_hydrodyn;
   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle(us_tr("US-SOMO: SAXS Screen Control"));
   order_ascending = false;

   saxs_widget = &(((US_Hydrodyn *) us_hydrodyn)->saxs_plot_widget);
   saxs_window = ((US_Hydrodyn *) us_hydrodyn)->saxs_plot_window;
   ((US_Hydrodyn *) us_hydrodyn)->saxs_screen_widget = true;
   plot_dist_zoomer = (ScrollZoomer *)0;

   best_fitness = 1e99;
   pen_width = 1;

   setupGUI();
   running = false;

   for ( unsigned int i = 0; i < (unsigned int) saxs_window->plotted_I_error.size(); i++ )
   {
      qs.      push_back( saxs_window->plotted_q           [ i ] );
      Is.      push_back( saxs_window->plotted_I           [ i ] );
      I_errors.push_back( saxs_window->plotted_I_error     [ i ] );
      names.   push_back( saxs_window->qsl_plotted_iq_names[ i ] );
   }

   clear_plot_all();

   set_target();
   update_enables();

   editor_msg("blue", "THIS WINDOW IS UNDER DEVELOPMENT" );

   global_Xpos += 30;
   global_Ypos += 30;

   unsigned int csv_height = t_csv->rowHeight(0) + 30;
   unsigned int csv_width = t_csv->columnWidth(0) + 45;
   for ( int i = 0; i < t_csv->rowCount(); i++ )
   {
      csv_height += t_csv->rowHeight(i);
   }
   for ( int i = 1; i < t_csv->columnCount(); i++ )
   {
      csv_width += t_csv->columnWidth(i);
   }
   if ( csv_height > 700 )
   {
      csv_height = 700;
   }
   if ( csv_width > 1000 )
   {
      csv_width = 1000;
   }

   t_csv->setMaximumHeight( t_csv->height() );
#if QT_VERSION < 0x040000
   editor->setMaximumWidth( editor->width() + editor->width() / 20 );
#else
   editor->setMaximumWidth( ( (int)csv_width * 35 ) / 100 );
#endif

   max_y_range = 0e0;

   // cout << QString("csv size %1 %2\n").arg(csv_height).arg(csv_width);

   setGeometry(global_Xpos, global_Ypos, csv_width, 100 + csv_height );
}

US_Hydrodyn_Saxs_Screen::~US_Hydrodyn_Saxs_Screen()
{
   ((US_Hydrodyn *)us_hydrodyn)->saxs_screen_widget = false;
}

void US_Hydrodyn_Saxs_Screen::setupGUI()
{
   int minHeight1  = 30;
   int minHeight1b = 75;
   int minHeight2  = 45;
#if QT_VERSION < 0x040000 || !defined(Q_OS_MAC)
   int minHeight3  = 30;
#endif

   lbl_title = new QLabel(csv1.name.left(80), this);
   lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette( PALET_FRAME );
   AUTFBACK( lbl_title );
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   t_csv = new QTableWidget(csv1.data.size(), csv1.header.size(), this);
   t_csv->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   // t_csv->setMinimumHeight(minHeight1 * 3);
   // t_csv->setMinimumWidth(minWidth1);
   t_csv->setPalette( PALET_EDIT );
   AUTFBACK( t_csv );
   t_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   t_csv->setEnabled(true);

   for ( unsigned int i = 0; i < csv1.num_data.size(); i++ )
   {
      for ( unsigned int j = 0; j < csv1.num_data[i].size(); j++ )
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

   t_csv->setColumnWidth(0, 200);
   { for ( int i = 0; i < t_csv->rowCount(); ++i ) { t_csv->item( i, 0 )->setFlags( t_csv->item( i, 0 )->flags() ^ Qt::ItemIsEditable ); } };
   // { for ( int i = 0; i < t_csv->rowCount(); ++i ) { t_csv->item( i, t_csv->columnCount() - 1 )->setFlags( t_csv->item( i, t_csv->columnCount() - 1 )->flags() ^ Qt::ItemIsEditable ); } };

   // probably I'm not understanding something, but these next two lines don't seem to do anything
   // t_csv->horizontalHeader()->adjustHeaderSize();
   t_csv->adjustSize();

   recompute_interval_from_points();

   connect(t_csv, SIGNAL(valueChanged(int, int)), SLOT(table_value(int, int )));

   progress = new QProgressBar( this );
   progress->setMinimumHeight(minHeight1);
   progress->setPalette( PALET_NORMAL );
   AUTFBACK( progress );
   progress->reset();

   progress2 = new QProgressBar( this );
   progress2->setMinimumHeight(minHeight1);
   progress2->setPalette( PALET_NORMAL );
   AUTFBACK( progress2 );
   progress2->reset();

   pb_clear_plot_all = new QPushButton(us_tr("Clear all"), this);
   pb_clear_plot_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   // pb_clear_plot_all->setMinimumHeight(minHeight1);
   pb_clear_plot_all->setPalette( PALET_PUSHB );
   connect(pb_clear_plot_all, SIGNAL(clicked()), SLOT(clear_plot_all()));

   pb_clear_plot_row = new QPushButton(us_tr("Clear row"), this);
   pb_clear_plot_row->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   // pb_clear_plot_row->setMinimumHeight(minHeight1);
   pb_clear_plot_row->setPalette( PALET_PUSHB );
   connect(pb_clear_plot_row, SIGNAL(clicked()), SLOT(clear_plot_row()));

   pb_push = new QPushButton(us_tr("New row"), this);
   pb_push->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   // pb_push->setMinimumHeight(minHeight1);
   pb_push->setPalette( PALET_PUSHB );
   connect(pb_push, SIGNAL(clicked()), SLOT(push()));

   pb_load_plot = new QPushButton(us_tr("Load"), this);
   pb_load_plot->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   // pb_load_plot->setMinimumHeight(minHeight1);
   pb_load_plot->setPalette( PALET_PUSHB );
   connect(pb_load_plot, SIGNAL(clicked()), SLOT(load_plot()));

   pb_save_plot = new QPushButton(us_tr("Save"), this);
   pb_save_plot->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   // pb_save_plot->setMinimumHeight(minHeight1);
   pb_save_plot->setPalette( PALET_PUSHB );
   connect(pb_save_plot, SIGNAL(clicked()), SLOT(save_plot()));

   cb_plot_average = new QCheckBox(this);
   cb_plot_average->setText(us_tr("Average"));
   cb_plot_average->setChecked(true);
   cb_plot_average->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_plot_average->setPalette( PALET_NORMAL );
   AUTFBACK( cb_plot_average );
   connect(cb_plot_average, SIGNAL(clicked()), SLOT(replot()));

   cb_plot_best = new QCheckBox(this);
   cb_plot_best->setText(us_tr("Best"));
   cb_plot_best->setChecked(true);
   cb_plot_best->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_plot_best->setPalette( PALET_NORMAL );
   AUTFBACK( cb_plot_best );
   connect(cb_plot_best, SIGNAL(clicked()), SLOT(replot()));

   cb_plot_rg = new QCheckBox(this);
   cb_plot_rg->setText(us_tr("Rg"));
   cb_plot_rg->setChecked(true);
   cb_plot_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_plot_rg->setPalette( PALET_NORMAL );
   AUTFBACK( cb_plot_rg );
   connect(cb_plot_rg, SIGNAL(clicked()), SLOT(replot()));

   cb_plot_chi2 = new QCheckBox(this);
   cb_plot_chi2->setText(us_tr("nchi/nrmsd"));
   cb_plot_chi2->setChecked(true);
   cb_plot_chi2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_plot_chi2->setPalette( PALET_NORMAL );
   AUTFBACK( cb_plot_chi2 );
   connect(cb_plot_chi2, SIGNAL(clicked()), SLOT(replot()));

   pb_replot_saxs = new QPushButton(us_tr("Replot stored I(q)"), this);
   pb_replot_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_replot_saxs->setMinimumHeight(minHeight1);
   pb_replot_saxs->setPalette( PALET_PUSHB );
   connect(pb_replot_saxs, SIGNAL(clicked()), SLOT(replot_saxs()));

   pb_save_saxs_plot = new QPushButton(us_tr("Store current I(q) plot"), this);
   pb_save_saxs_plot->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_save_saxs_plot->setMinimumHeight(minHeight1);
   pb_save_saxs_plot->setPalette( PALET_PUSHB );
   connect(pb_save_saxs_plot, SIGNAL(clicked()), SLOT(save_saxs_plot()));

   pb_set_target = new QPushButton(us_tr("Set Target"), this);
   pb_set_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_set_target->setMinimumHeight(minHeight1);
   pb_set_target->setPalette( PALET_PUSHB );
   connect(pb_set_target, SIGNAL(clicked()), SLOT(set_target()));

   lbl_current_target = new QLabel("", this);
   lbl_current_target->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_current_target->setMinimumHeight(minHeight1);
   lbl_current_target->setPalette( PALET_LABEL );
   AUTFBACK( lbl_current_target );
   lbl_current_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   cb_normalize = new QCheckBox(this);
   cb_normalize->setText(us_tr(" Normalize"));
   cb_normalize->setChecked(true);
   cb_normalize->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_normalize->setPalette( PALET_NORMAL );
   AUTFBACK( cb_normalize );

   pb_start = new QPushButton(us_tr("Start"), this);
   pb_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_start->setMinimumHeight(minHeight1);
   pb_start->setPalette( PALET_PUSHB );
   connect(pb_start, SIGNAL(clicked()), SLOT(start()));

   pb_run_all_targets = new QPushButton(us_tr("Start for all plotted I(q)"), this);
   pb_run_all_targets->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_run_all_targets->setMinimumHeight(minHeight1);
   pb_run_all_targets->setPalette( PALET_PUSHB );
   connect(pb_run_all_targets, SIGNAL(clicked()), SLOT(run_all_targets()));

   pb_stop = new QPushButton(us_tr("Stop"), this);
   pb_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_stop->setMinimumHeight(minHeight1);
   pb_stop->setPalette( PALET_PUSHB );
   connect(pb_stop, SIGNAL(clicked()), SLOT(stop()));

   editor = new QTextEdit(this);
   editor->setPalette( PALET_NORMAL );
   AUTFBACK( editor );
   editor->setReadOnly(true);

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
   editor->setMinimumHeight(300);

//   plot_dist = new QwtPlot(this);
   usp_plot_dist = new US_Plot( plot_dist, "", "", "", this );
   connect( (QWidget *)plot_dist->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_dist( const QPoint & ) ) );
   ((QWidget *)plot_dist->titleLabel())->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_dist->axisWidget( QwtPlot::yLeft ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_dist( const QPoint & ) ) );
   ((QWidget *)plot_dist->axisWidget( QwtPlot::yLeft ))->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_dist->axisWidget( QwtPlot::xBottom ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_dist( const QPoint & ) ) );
   ((QWidget *)plot_dist->axisWidget( QwtPlot::xBottom ))->setContextMenuPolicy( Qt::CustomContextMenu );
   // plot_dist->enableOutline(true);
   // plot_dist->setOutlinePen(Qt::white);
   // plot_dist->setOutlineStyle(Qwt::VLine);
#if QT_VERSION < 0x040000
   plot_dist->enableGridXMin();
   plot_dist->enableGridYMin();
   plot_dist->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot_dist->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   QwtPlotGrid* grid = new QwtPlotGrid;
   grid->enableXMin( true );
   grid->enableYMin( true );
   grid->setMajorPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine));
   grid->setMinorPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine));
   grid->attach( plot_dist );
#endif
   plot_dist->setPalette( PALET_NORMAL );
   AUTFBACK( plot_dist );
   plot_dist->setAxisTitle(QwtPlot::xBottom, us_tr("Radius (A)"));
   plot_dist->setAxisTitle(QwtPlot::yLeft, us_tr("Intensity"));
   plot_dist->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_dist->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_dist->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   plot_dist->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   plot_dist->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   plot_dist->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   plot_dist->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
//    plot_dist->setMargin(USglobal->config_list.margin);
   plot_dist->setTitle("");
   plot_dist->setCanvasBackground(USglobal->global_colors.plot);
   
   lbl_pos_range = new QLabel("0 of 0", this);
   lbl_pos_range->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
   lbl_pos_range->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_pos_range );
   lbl_pos_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   lbl_pos_range2 = new QLabel("1\nof\n1", this);
   lbl_pos_range2->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
   lbl_pos_range2->setMaximumHeight( minHeight1b );
   lbl_pos_range2->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_pos_range2 );
   lbl_pos_range2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   lbl_message = new QLabel("", this);
   lbl_message->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
   lbl_message->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_message );
   lbl_message->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   lbl_message2 = new QLabel("", this);
   lbl_message2->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
   lbl_message2->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_message2 );
   lbl_message2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   lbl_message3 = new QLabel("", this);
   lbl_message3->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
   lbl_message3->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_message3 );
   lbl_message3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   lbl_message4 = new QLabel("", this);
   lbl_message4->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
   lbl_message4->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_message4 );
   lbl_message4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   qwtw_wheel = new QwtWheel( this );
   qwtw_wheel->setMass         ( 1.0 );
   qwtw_wheel->setRange( 0.0, 0.0); qwtw_wheel->setSingleStep( 1 );
   qwtw_wheel->setMinimumHeight( minHeight2 );
   // qwtw_wheel->setTotalAngle( 3600.0 );
   connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );

   qwtw_wheel2 = new QwtWheel( this );
   qwtw_wheel2->setOrientation  ( Qt::Vertical );
   qwtw_wheel2->setMass         ( 1.0 );
   qwtw_wheel2->setRange( 1.0, 1.0); qwtw_wheel2->setSingleStep( 1 );
   qwtw_wheel2->setMinimumWidth ( minHeight2 );
   // qwtw_wheel->setTotalAngle( 3600.0 );
   connect( qwtw_wheel2, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel2( double ) ) );

   pb_help = new QPushButton(us_tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(us_tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // build layout
   QHBoxLayout * hbl_target = new QHBoxLayout(); hbl_target->setContentsMargins( 0, 0, 0, 0 ); hbl_target->setSpacing( 0 );
   hbl_target->addSpacing(4);
   hbl_target->addWidget(pb_replot_saxs);
   hbl_target->addSpacing(4);
   hbl_target->addWidget(pb_save_saxs_plot);
   hbl_target->addSpacing(4);
   hbl_target->addWidget(pb_set_target);
   hbl_target->addSpacing(4);
   hbl_target->addWidget(lbl_current_target);
   hbl_target->addSpacing(4);
   hbl_target->addWidget(cb_normalize);
   hbl_target->addSpacing(4);

   QHBoxLayout * hbl_plot_buttons = new QHBoxLayout(); hbl_plot_buttons->setContentsMargins( 0, 0, 0, 0 ); hbl_plot_buttons->setSpacing( 0 );
   // hbl_plot_buttons->addSpacing(4);
   hbl_plot_buttons->addWidget(pb_load_plot);
   hbl_plot_buttons->addSpacing(4);
   hbl_plot_buttons->addWidget(pb_save_plot);
   hbl_plot_buttons->addSpacing(4);
   hbl_plot_buttons->addWidget(pb_clear_plot_all);
   hbl_plot_buttons->addSpacing(4);
   hbl_plot_buttons->addWidget(pb_clear_plot_row);
   hbl_plot_buttons->addSpacing(4);
   hbl_plot_buttons->addWidget(pb_push);
   // hbl_plot_buttons->addSpacing(4);

   QHBoxLayout * hbl_plot_checks = new QHBoxLayout(); hbl_plot_checks->setContentsMargins( 0, 0, 0, 0 ); hbl_plot_checks->setSpacing( 0 );
   hbl_plot_checks->addWidget(cb_plot_average);
   hbl_plot_checks->addWidget(cb_plot_best);
   hbl_plot_checks->addWidget(cb_plot_rg);
   hbl_plot_checks->addWidget(cb_plot_chi2);

   QHBoxLayout * hbl_controls = new QHBoxLayout(); hbl_controls->setContentsMargins( 0, 0, 0, 0 ); hbl_controls->setSpacing( 0 );
   hbl_controls->addSpacing(4);
   hbl_controls->addWidget(pb_start);
   hbl_controls->addSpacing(4);
   hbl_controls->addWidget(pb_run_all_targets);
   hbl_controls->addSpacing(4);
   hbl_controls->addWidget(pb_stop);
   hbl_controls->addSpacing(4);

   QHBoxLayout * hbl_bottom = new QHBoxLayout(); hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addSpacing(4);
   hbl_bottom->addWidget(pb_help);
   hbl_bottom->addSpacing(4);
   hbl_bottom->addWidget(pb_cancel);
   hbl_bottom->addSpacing(4);

   QBoxLayout * vbl_editor_group = new QVBoxLayout(0); vbl_editor_group->setContentsMargins( 0, 0, 0, 0 ); vbl_editor_group->setSpacing( 0 );
#if QT_VERSION < 0x040000 || !defined(Q_OS_MAC)
   vbl_editor_group->addWidget(frame);
#endif
   vbl_editor_group->addWidget(editor);

   QGridLayout * gbl_messages = new QGridLayout( 0 ); gbl_messages->setContentsMargins( 0, 0, 0, 0 ); gbl_messages->setSpacing( 0 );
   gbl_messages->addWidget( lbl_message , 0, 0 );
   gbl_messages->addWidget( lbl_message2, 0, 1 );
   gbl_messages->addWidget( lbl_message3, 1, 0 );
   gbl_messages->addWidget( lbl_message4, 1, 1 );

   QBoxLayout * vbl_plot_group = new QVBoxLayout(0); vbl_plot_group->setContentsMargins( 0, 0, 0, 0 ); vbl_plot_group->setSpacing( 0 );
   vbl_plot_group->addWidget(plot_dist);
   vbl_plot_group->addLayout(gbl_messages);
   vbl_plot_group->addWidget(qwtw_wheel);
   vbl_plot_group->addWidget(lbl_pos_range);

   QBoxLayout * vbl_wheel2 = new QVBoxLayout(0); vbl_wheel2->setContentsMargins( 0, 0, 0, 0 ); vbl_wheel2->setSpacing( 0 );
   vbl_wheel2->addWidget(qwtw_wheel2);
   vbl_wheel2->addWidget(lbl_pos_range2);
   
   QHBoxLayout * hbl_plot_g1 = new QHBoxLayout(); hbl_plot_g1->setContentsMargins( 0, 0, 0, 0 ); hbl_plot_g1->setSpacing( 0 );
   hbl_plot_g1->addLayout(vbl_plot_group);
   hbl_plot_g1->addLayout(vbl_wheel2);

   QVBoxLayout * vbl_plot_g2 = new QVBoxLayout(0); vbl_plot_g2->setContentsMargins( 0, 0, 0, 0 ); vbl_plot_g2->setSpacing( 0 );
   vbl_plot_g2->addLayout(hbl_plot_g1);
   vbl_plot_g2->addLayout(hbl_plot_checks);
   vbl_plot_g2->addLayout(hbl_plot_buttons);

   QHBoxLayout * hbl_editor_plot = new QHBoxLayout(); hbl_editor_plot->setContentsMargins( 0, 0, 0, 0 ); hbl_editor_plot->setSpacing( 0 );
   hbl_editor_plot->addLayout(vbl_editor_group);
   hbl_editor_plot->addLayout(vbl_plot_g2);

   QVBoxLayout * background = new QVBoxLayout(this); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );
   background->addSpacing(4);
   background->addWidget(lbl_title);
   background->addSpacing(4);
   background->addWidget(t_csv);
   background->addLayout(hbl_editor_plot);
   background->addSpacing(4);
   background->addWidget(progress2);
   background->addSpacing(4);
   background->addWidget(progress);
   background->addSpacing(4);
   background->addLayout(hbl_target);
   background->addSpacing(4);
   // background->addLayout(hbl_plot_buttons);
   // background->addSpacing(4);
   background->addLayout(hbl_controls);
   background->addSpacing(4);
   background->addLayout(hbl_bottom);
   background->addSpacing(4);
}

void US_Hydrodyn_Saxs_Screen::cancel()
{
   close();
}

void US_Hydrodyn_Saxs_Screen::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo/somo_saxs_screen.html");
}

void US_Hydrodyn_Saxs_Screen::closeEvent(QCloseEvent *e)
{
   if ( anything_plotted() &&
        anything_plotted_since_save )
   {
      raise();
      save_plot();
   }

   ((US_Hydrodyn *)us_hydrodyn)->saxs_screen_widget = false;
   ((US_Hydrodyn *)us_hydrodyn)->last_saxs_screen_csv = current_csv();

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Saxs_Screen::table_value( int /* row */, int col )
{
   if ( col == 4 || col == 2 || col == 3 )
   {
      recompute_interval_from_points();
   }
   if ( col == 5 )
   {
      recompute_points_from_interval();
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Screen::clear_display()
{
   editor->clear( );
   editor->append("\n\n");
}

void US_Hydrodyn_Saxs_Screen::update_font()
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

void US_Hydrodyn_Saxs_Screen::save()
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

void US_Hydrodyn_Saxs_Screen::set_target( QString scaling_target )
{
   lbl_current_target->setText( scaling_target );
   bool target_found = false;
   for ( unsigned int i = 0; i < (unsigned int) names.size(); i++ )
   {
      if ( names[i] == lbl_current_target->text() )
      {
         target_found = true;
         break;
      }
   }
   if ( !target_found )
   {
      save_saxs_plot();
   }

   target_found      = false;
   last_target_found = false;
   unsigned int target_pos;
   for ( unsigned int i = 0; i < (unsigned int) names.size(); i++ )
   {
      if ( names[i] == lbl_current_target->text() )
      {
         target_found = true;
         target_pos   = i;
         last_target_pos   = i;
         last_target_found = true;
         break;
      }
   }

   if ( target_found )
   {
      for ( unsigned int i = 0; i < (unsigned int)t_csv->rowCount(); i++ )
      {
         if ( t_csv->item( i, 0 )->text().contains("Ending q (A^-1)") )
         {
            t_csv->setItem( i, 2, new QTableWidgetItem( QString("%1").arg( qs[ target_pos ][ 0 ]) ) );
            t_csv->setItem( i, 3, new QTableWidgetItem( QString("%1").arg( qs[ target_pos ][ qs[ target_pos ].size() - 1 ] ) ) );
            recompute_interval_from_points();
            break;
         }
      }
   }      

   update_enables();
}

void US_Hydrodyn_Saxs_Screen::set_target()
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

   set_target( scaling_target );
}

void US_Hydrodyn_Saxs_Screen::run_all_targets()
{
   if ( !validate() ||
        !any_to_run() )
   {
      return;
   }
   
   if ( !saxs_window->qsl_plotted_iq_names.size() )
   {
      QMessageBox::information(this, 
                               us_tr("US-SOMO: I(q) screen"),
                               QString(us_tr("No I(q)'s plotted in the SAXS window")));
      return;
   }

   running = true;
   update_enables();

   for ( unsigned int i = 0; i < (unsigned int) saxs_window->qsl_plotted_iq_names.size(); i++ )
   {
      progress2->setValue( i + 1 ); progress2->setMaximum( saxs_window->qsl_plotted_iq_names.size() + 1 );
      editor_msg("blue", QString("running %1\n").arg( saxs_window->qsl_plotted_iq_names[ i ] ) );
      set_target( saxs_window->qsl_plotted_iq_names[ i ] );
      if ( i )
      {
         push();
      }
      start( true );
      if ( !running )
      {
         break;
      }
   }
   if ( running )
   {
      progress2->setValue( 1 ); progress2->setMaximum( 1 );
      running = false;
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Screen::start( bool already_running )
{
   if ( !validate() ||
        !any_to_run() )
   {
      return;
   }
   if ( !already_running )
   {
      progress2->reset();
      running = true;
      update_enables();
   }

   map < unsigned int, double >       starts;
   map < unsigned int, double >       ends;
   map < unsigned int, unsigned int > points;
   map < unsigned int, double >       increments;
   map < unsigned int, unsigned int > offsets;
   map < unsigned int, unsigned int > next_offsets;

   unsigned int current_offset = 0;

   csv_source_name_iqq.clear( );
   saxs_q.clear( );
   saxs_iqq.clear( );

   double min_radius             = 0e0;
   double max_radius             = 0e0;
   double delta_radius           = 1e0;
   unsigned int points_radius    = 0;
   double min_delta_rho          = 0e0;
   double max_delta_rho          = 0e0;
   double delta_delta_rho        = 1e0;
   unsigned int points_delta_rho = 0;

   double min_end_q              = 0e0;
   double max_end_q              = 0e0;
   double delta_end_q            = 1e0;
   unsigned int points_q         = 0;

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

         if ( t_csv->item( i, 0 )->text().contains("Radius (A)") )
         {
            min_radius    = starts[ i ];
            max_radius    = ends[ i ];
            delta_radius  = increments[ i ];
            points_radius = points[ i ];
            editor_msg("dark blue", QString("Radius range %1 to %2 step %3 (A)\n")
                       .arg(min_radius)
                       .arg(max_radius)
                       .arg(delta_radius)
                       );
         }
         if ( t_csv->item( i, 0 )->text().contains("Delta rho (A^-3)") )
         {
            min_delta_rho    = starts[ i ];
            max_delta_rho    = ends[ i ];
            delta_delta_rho  = increments[ i ];
            points_delta_rho = points[ i ];
            editor_msg("dark blue", QString("Delta rho range %1 to %2 step %3 (A^-3)\n")
                       .arg(min_delta_rho)
                       .arg(max_delta_rho)
                       .arg(delta_delta_rho)
                       );
         }

         if ( t_csv->item( i, 0 )->text().contains("Ending q (A^-1)") )
         {
            min_end_q   = starts[ i ];
            max_end_q   = ends[ i ];
            delta_end_q = increments[ i ];
            points_q    = points[ i ];
            editor_msg("dark blue", QString("End q range %1 to %2 step %3 (A^-3)\n")
                       .arg(min_end_q)
                       .arg(max_end_q)
                       .arg(delta_end_q)
                       );
         }
      } else {
         if ( t_csv->item( i, 0 )->text().contains("Radius (A)") )
         {
            min_radius    = t_csv->item(i, 2)->text().toDouble();
            max_radius    = min_radius;
            delta_radius  = 1e0;
            points_radius = 1;
            editor_msg("dark red", QString("Using a fixed radius of %1 (A)\n").arg(min_radius));
         }
         if ( t_csv->item( i, 0 )->text().contains("Delta rho (A^-3)") )
         {
            min_delta_rho    = t_csv->item(i, 2)->text().toDouble();
            max_delta_rho    = min_delta_rho;
            delta_delta_rho  = 1e0;
            points_delta_rho = 1;
            editor_msg("dark red", QString("Using a fixed delta rho of %1 (A^-3)\n").arg(min_delta_rho));
         }
      }         
   }
   
   vector < double > target_q;
   vector < double > target_I;
   vector < double > target_I_errors;

   // recover from target
   bool target_found = false;
   for ( unsigned int i = 0; i < names.size(); i++ )
   {
      if ( names[i] == lbl_current_target->text() )
      {
         target_q        = qs[ i ];
         target_I        = Is[ i ];
         target_I_errors = I_errors[ i ];
         target_found = true;
         break;
      }         
   }

   if ( !target_found )
   {
      editor_msg("red", "could not find data for target!");
      running = false;
      update_enables();
      return;
   }
   unsigned int total_points = 1;
   for ( map < unsigned int, unsigned int >::iterator it = points.begin();
         it != points.end();
         it++ )
   {
      total_points *= it->second;
   }

   if ( !points_q )
   {
      points_q = 1;
      min_end_q = (double) saxs_window->our_saxs_options->end_q;
      max_end_q = (double) saxs_window->our_saxs_options->end_q;
   }

   editor_msg("black", QString("Total spheres tested %1\n").arg(total_points / points_q));
   editor_msg("black", QString("Total q points %1\n").arg(points_q));

   unsigned int pp = 1;
   for ( double use_q = min_end_q; use_q <= max_end_q; use_q += delta_end_q )
   {
      progress->setValue( pp++ ); progress->setMaximum( points_q + 1 );
      qApp->processEvents();

      // linearization of an arbitrary number of loops
      US_Saxs_Util usu;
      
      vector < double >             by_radius;
      vector < double >             by_delta_rho;
      vector < double >             val_radius;
      vector < double >             val_delta_rho;
      map < double, unsigned int >  index_radius;
      map < double, unsigned int >  index_delta_rho;
      double                        best_fit_radius;
      double                        best_fit_delta_rho;
      double                        average_radius;
      double                        average_delta_rho;
      double                        start_q;
      double                        end_q;
      double                        delta_q;
      unsigned int                  use_points_q;

      QString                       best_tag;
      QString                       nnls_tag;

      bool has_error = 
         !usu.iqq_sphere_fit(
                             "saxs_screen",
                             target_q,
                             target_I,
                             target_I_errors,
                             min_radius,
                             max_radius,
                             delta_radius,
                             min_delta_rho,
                             max_delta_rho,
                             delta_delta_rho,
                             (double) saxs_window->our_saxs_options->start_q,
                             use_q,
                             by_radius,
                             by_delta_rho,
                             val_radius,
                             val_delta_rho,
                             index_radius,
                             index_delta_rho,
                             best_fit_radius,
                             best_fit_delta_rho,
                             average_radius,
                             average_delta_rho,
                             start_q,
                             end_q,
                             delta_q,
                             use_points_q,
                             best_tag,
                             nnls_tag,
                             cb_normalize->isChecked()
                             );

      if ( has_error )
      {
         editor_msg("red", usu.errormsg );
      }

      if ( !has_error )
      {
         editor_msg("blue", usu.noticemsg );
         
         double  chi2_best;
         double  chi2_nnls;
         double  k;
         QString target = "saxs_screen_cropped";
         bool    use_chi2 = saxs_window->is_nonzero_vector( usu.wave[ target ].s );

         if ( use_chi2 )
         {
            usu.scaling_fit( 
                            usu.wave[ best_tag ].r,
                            usu.wave[ target ].r,
                            usu.wave[ target ].s,
                            k, 
                            chi2_best
                            );
            usu.scaling_fit( 
                            usu.wave[ nnls_tag ].r,
                            usu.wave[ target ].r,
                            usu.wave[ target ].s,
                            k, 
                            chi2_nnls
                            );
            chi2_best = sqrt( chi2_best / ( usu.wave[ target ].r.size() - 1 ) );
            chi2_nnls = sqrt( chi2_nnls / ( usu.wave[ target ].r.size() - 1 ) );
         } else {
            usu.scaling_fit( 
                            usu.wave[ best_tag ].r,
                            usu.wave[ target ].r,
                            k, 
                            chi2_best
                            );
            usu.scaling_fit( 
                            usu.wave[ best_tag ].r,
                            usu.wave[ target ].r,
                            k, 
                            chi2_nnls
                            );
            chi2_best = chi2_best / usu.wave[ target ].r.size();
            chi2_nnls = chi2_nnls / usu.wave[ target ].r.size();
         }

         // then plot, summarize results
         
         vector < double > radii;
         vector < double > intensity;
         
         for ( unsigned int i = 0; i < by_radius.size(); i++ )
         {
            radii.push_back( val_radius[ i ] );
            intensity.push_back( by_radius[ i ] );
         }
         
         double Rg;
         get_guinier_rg( lbl_current_target->text(), Rg );

         plot_one( 
                  QString( " Target %1 " ).arg( lbl_current_target->text() ),
                  QString( " Radius [ %1 : %2 ] / %3 ( %4 )" )
                  .arg( min_radius )
                  .arg( max_radius )
                  .arg( delta_radius )
                  .arg( points_radius ),
                  QString( " Delta rho [ %1 : %2 ] / %3 ( %4 )" )
                  .arg( min_delta_rho )
                  .arg( max_delta_rho )
                  .arg( delta_delta_rho )
                  .arg( points_delta_rho ),
                  QString( " q [ %1 : %2 ] / %3 ( %4 )" )
                  .arg( start_q , 0, 'g',  4 )
                  .arg( end_q , 0, 'g', 4 )
                  .arg( delta_q, 0, 'g', 4 )
                  .arg( use_points_q ),
                  radii,
                  intensity,
                  best_fit_radius,
                  best_fit_delta_rho,
                  average_radius,
                  average_delta_rho,
                  Rg,
                  use_chi2,
                  chi2_best,
                  chi2_nnls
                  );
         
      }
      if ( !running )
      {
         update_enables();
         return;
      }
   }
      
   progress->setValue( 1 ); progress->setMaximum( 1 );
   if ( !already_running )
   {
      running = false;
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Screen::plot_one( 
                                       QString           message,
                                       QString           message2,
                                       QString           message3,
                                       QString           message4,
                                       vector < double > radii,
                                       vector < double > intensity,
                                       double            best_fit_radius,
                                       double            best_fit_delta_rho,
                                       double            average_radius,
                                       double            average_delta_rho,
                                       double            rg,
                                       bool              use_chi2,
                                       double            chi2_best,
                                       double            chi2_nnls
                                       )
{
   anything_plotted_since_save = true;

   messages           [ current_row ].push_back( message );
   messages2          [ current_row ].push_back( message2 );
   messages3          [ current_row ].push_back( message3 );
   messages4          [ current_row ].push_back( message4 );
   radiis             [ current_row ].push_back( radii );
   intensitys         [ current_row ].push_back( intensity );
   best_fit_radiuss   [ current_row ].push_back( best_fit_radius );
   best_fit_delta_rhos[ current_row ].push_back( best_fit_delta_rho );
   average_radiuss    [ current_row ].push_back( average_radius );
   average_delta_rhos [ current_row ].push_back( average_delta_rho );
   target_rgs         [ current_row ].push_back( rg );
   use_chi2s          [ current_row ].push_back( use_chi2 );
   chi2_bests         [ current_row ].push_back( chi2_best );
   chi2_nnlss         [ current_row ].push_back( chi2_nnls );

   for ( unsigned int i = 0; i < intensity.size(); i++ )
   {
      if ( max_y_range < intensity[ i ] )
      {
         max_y_range = intensity[ i ];
      }
   }
   max_x_range = radii[ radii.size() - 1 ];

   last_plotted_pos = messages[ current_row ].size() - 1;
   update_wheel_range();
   qwtw_wheel->setValue( (double) last_plotted_pos );

   // plot_pos( last_plotted_pos );

   replot();
}

void US_Hydrodyn_Saxs_Screen::update_wheel_range()
{
   unsigned int messages_size = messages[ current_row ].size();
   if ( messages_size )
   {
      messages_size--;
      {
         double rs = 
            ( messages_size * messages_size * 0.1 < 1.0 )
            ? 
            messages_size * messages_size * 0.1 
            :
            1.0
            ;
         qwtw_wheel->setRange( -0.5, (double) messages_size + 0.5); qwtw_wheel->setSingleStep( rs );
      }
   } else {
      qwtw_wheel->setRange( 0.0, 0.0); qwtw_wheel->setSingleStep( 1.0 );
   }
}

void US_Hydrodyn_Saxs_Screen::plot_row( unsigned int i )
{
   if ( messages.size() <= i )
   {
      return;
   }

   lbl_pos_range2->setText( QString( "%1\nof\n%2" ).arg( i + 1 ).arg( messages.size() ) );

   current_row = i;
   if ( last_plotted_pos >= messages[ current_row ].size() )
   {
      if ( !messages[ current_row ].size() )
      {
         last_plotted_pos = 0;
         clear_plot();
      } else {
         last_plotted_pos = messages[ current_row ].size() - 1;
         replot(); // plot_pos( last_plotted_pos );
      }
   } else {
      replot(); // plot_pos( last_plotted_pos );
   }
   if ( messages[ current_row ].size() )
   {
      if ( last_plotted_pos >= messages[ current_row ].size() )
      {
         last_plotted_pos = messages[ current_row ].size() - 1;
      }
   } else {
      last_plotted_pos = 0;
   }
   update_wheel_range();
}

void US_Hydrodyn_Saxs_Screen::plot_pos( unsigned int i )
{
   if ( messages[ current_row ].size() <= i )
   {
      return;
   }

   last_plotted_pos = i;
   lbl_pos_range->setText( QString( "%1 of %2" ).arg( i + 1 ).arg( messages[ current_row ].size() ) );
   
   plot_dist->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_dist->detachItems( QwtPlotItem::Rtti_PlotMarker );;
#if QT_VERSION < 0x040000
   long curvekey = plot_dist->insertCurve("Radii histogram");
   plot_dist->setCurveStyle( curvekey, QwtCurve::Sticks);
   plot_dist->setCurveData ( curvekey, 
                             (double *)&( radiis    [ current_row ][ i ][ 0 ] ),
                             (double *)&( intensitys[ current_row ][ i ][ 0 ] ), 
                             radiis[ current_row ][ i ].size() );
   plot_dist->setCurvePen  ( curvekey, QPen( "yellow", pen_width, SolidLine ) );
#else
   QwtPlotCurve *curve = new QwtPlotCurve( us_tr( "Radii histogram" ) );
   curve->setStyle( QwtPlotCurve::Sticks );
   curve->setSamples( (double *)&( radiis    [ current_row ][ i ][ 0 ] ),
                   (double *)&( intensitys[ current_row ][ i ][ 0 ] ),
                   radiis[ current_row ][ i ].size() );
   curve->setPen( QPen( Qt::yellow, pen_width, Qt::SolidLine ) );
   curve->attach( plot_dist );
   int    sizeym = ( plot_dist->height() * 45 ) / 100;
   double ymark  = ( curve->minYValue() + curve->maxYValue() ) * 0.40;
#endif

   if ( cb_plot_best->isChecked() )
   {
#if QT_VERSION < 0x040000
      long qpmkey = plot_dist->insertMarker();
      plot_dist->setMarkerLineStyle ( qpmkey, QwtMarker::VLine);
      plot_dist->setMarkerPos       ( qpmkey, best_fit_radiuss[ current_row ][ i ], 0e0 );
      plot_dist->setMarkerLabelAlign( qpmkey, AlignRight|AlignTop );
      plot_dist->setMarkerPen       ( qpmkey, QPen( green, 2, DashDotDotLine));
      plot_dist->setMarkerFont      ( qpmkey, QFont("Helvetica", 11, QFont::Bold));
      plot_dist->setMarkerLabelText ( qpmkey, QString("Best individual\nfit at %1%2")
                                      .arg( best_fit_radiuss[ current_row ][ i ] )
                                      .arg( cb_plot_chi2->isChecked() ?
                                            QString("\n%1 = %2")
                                            .arg( use_chi2s[ current_row ][ i ] ? "nchi" : "nrmsd" )
                                            .arg( chi2_bests[ current_row ][ i ] )
                                            :
                                            "" )
                                      );
#else
      QwtPlotMarker* marker1 = new QwtPlotMarker;
      marker1->setSymbol( new QwtSymbol( QwtSymbol::VLine,
         QBrush( Qt::white ), QPen( Qt::green, 2, Qt::DashLine ),
         QSize( 8, sizeym ) ) );
      marker1->setValue( best_fit_radiuss[ current_row ][ i ], ymark );
      marker1->setLabelAlignment( Qt::AlignRight | Qt::AlignTop );
      marker1->setLabel( QString( "Best individual\nfit at %1%2" )
         .arg( best_fit_radiuss[ current_row ][ i ] )
         .arg( cb_plot_chi2->isChecked() ?
         QString("\n%1 = %2")
         .arg( use_chi2s[ current_row ][ i ] ? "nchi" : "nrmsd" )
         .arg( chi2_bests[ current_row ][ i ] ) : "" ) );
      marker1->attach( plot_dist );
#endif
   }

   if ( cb_plot_average->isChecked() )
   {
#if QT_VERSION < 0x040000
      long qpmkey2 = plot_dist->insertMarker();
      plot_dist->setMarkerLineStyle ( qpmkey2, QwtMarker::VLine);
      plot_dist->setMarkerPos       ( qpmkey2, average_radiuss[ current_row ][ i ], 0e0 );
      plot_dist->setMarkerLabelAlign( qpmkey2, cb_plot_best->isChecked() ? AlignLeft|AlignTop : AlignRight|AlignTop );
      plot_dist->setMarkerPen       ( qpmkey2, QPen( QColor( 255, 141, 0 ), 2, DashLine));
      plot_dist->setMarkerFont      ( qpmkey2, QFont("Helvetica", 11, QFont::Bold));
      plot_dist->setMarkerLabelText ( qpmkey2, QString("\n\n\nAverage fit\n at %1%2")
                                      .arg( average_radiuss[ current_row ][ i ] ) 
                                      .arg( cb_plot_chi2->isChecked() ?
                                            QString("\nNNLS %1 = %2")
                                            .arg( use_chi2s[ current_row ][ i ] ? "nchi" : "nrmsd" )
                                            .arg( chi2_nnlss[ current_row ][ i ] )
                                            :
                                            "" )
                                      );
#else
      QwtPlotMarker* marker2 = new QwtPlotMarker;
      marker2->setSymbol( new QwtSymbol( QwtSymbol::VLine,
         QBrush( Qt::white ), QPen( QColor( 255, 141, 0 ), 2, Qt::DashLine ),
         QSize( 8, sizeym ) ) );
      marker2->setValue( average_radiuss[ current_row ][ i ], ymark );
      marker2->setLabelAlignment( cb_plot_best->isChecked() ?
         Qt::AlignLeft | Qt::AlignTop : Qt::AlignRight | Qt::AlignTop );
      marker2->setLabel( QString( "\n\n\nAverage fit\n at %1%2" )
         .arg( average_radiuss[ current_row ][ i ] ) 
         .arg( cb_plot_chi2->isChecked() ?
         QString( "\nNNLS %1 = %2" )
         .arg( use_chi2s[ current_row ][ i ] ? "nchi" : "nrmsd" )
         .arg( chi2_nnlss[ current_row ][ i ] ) : "" ) );
      marker2->attach( plot_dist );
#endif
   }

   if ( cb_plot_rg->isChecked() &&
        target_rgs[ current_row ][ i ] != 0e0 )
   {
#if QT_VERSION < 0x040000
      long qpmkey3 = plot_dist->insertMarker();
      plot_dist->setMarkerLineStyle ( qpmkey3, QwtMarker::VLine);
      plot_dist->setMarkerPos       ( qpmkey3, target_rgs[ current_row ][ i ], 0e0 );
      plot_dist->setMarkerLabelAlign( qpmkey3, AlignRight|AlignCenter );
      plot_dist->setMarkerPen       ( qpmkey3, QPen( "blue", 2, DashLine));
      plot_dist->setMarkerFont      ( qpmkey3, QFont("Helvetica", 11, QFont::Bold));
      plot_dist->setMarkerLabelText ( qpmkey3, QString("Guinier Rg\n%1").arg( target_rgs[ current_row ][ i ] ) );
#else
      QwtPlotMarker* marker3 = new QwtPlotMarker;
      marker3->setSymbol( new QwtSymbol( QwtSymbol::VLine,
         QBrush( Qt::white ), QPen( Qt::cyan, 2, Qt::DashLine ),
         QSize( 8, sizeym ) ) );
      marker3->setValue( target_rgs[ current_row ][ i ], ymark );
      marker3->setLabelAlignment( Qt::AlignRight | Qt::AlignVCenter );
      marker3->setLabel( QString( "Guinier Rg\n%1" )
         .arg( target_rgs[ current_row ][ i ] ) );
      marker3->attach( plot_dist );
#endif
   }

   plot_dist->setAxisScale( QwtPlot::xBottom, 0.0, max_x_range );
   plot_dist->setAxisScale( QwtPlot::yLeft  , 0.0, max_y_range );

   // enable zooming
   
   if ( !plot_dist_zoomer )
   {
      plot_dist_zoomer = new ScrollZoomer(plot_dist->canvas());
      plot_dist_zoomer->setRubberBandPen( QPen( Qt::red, 1, Qt::DotLine ) );
      plot_dist_zoomer->setTrackerPen   ( QPen( Qt::red ) );
   }

   plot_dist->replot();

   lbl_message ->setText( messages [ current_row ][ i ] );
   lbl_message2->setText( messages2[ current_row ][ i ] );
   lbl_message3->setText( messages3[ current_row ][ i ] );
   lbl_message4->setText( messages4[ current_row ][ i ] );
}

void US_Hydrodyn_Saxs_Screen::stop()
{
   running = false;
   editor_msg("red", "Stopped by user request\n");
   update_enables();
}

void US_Hydrodyn_Saxs_Screen::update_enables()
{
   // cout << "US_Hydrodyn_Saxs_Screen::update_enables()\n";
   // cout << QString("saxs_window->qsl_plotted_iq_names.size() %1\n").arg(saxs_window->qsl_plotted_iq_names.size());
   bool any_selected  = false;
   if ( !running )
   {
      for ( unsigned int i = 0; i < (unsigned int)t_csv->rowCount(); i++ )
      {
         if ( ((QCheckBox *)(t_csv->cellWidget( i, 1 )))->isChecked() )
         {
            any_selected = true;
         }
      }
   }
   pb_start            ->setEnabled( !running && any_selected && !lbl_current_target->text().isEmpty() );
   pb_run_all_targets  ->setEnabled( !running && any_selected );
   pb_stop             ->setEnabled( running );
   pb_replot_saxs      ->setEnabled( !running && *saxs_widget && names.size() );
   pb_save_saxs_plot   ->setEnabled( !running && *saxs_widget && saxs_window->qsl_plotted_iq_names.size() );
   pb_set_target       ->setEnabled( !running && *saxs_widget && saxs_window->qsl_plotted_iq_names.size() );
   pb_clear_plot_all   ->setEnabled( !running );
   pb_clear_plot_row   ->setEnabled( !running );
   pb_push             ->setEnabled( !running );
   pb_save_plot        ->setEnabled( !running );
   pb_load_plot        ->setEnabled( !running );
   cb_plot_average     ->setEnabled( !running );
   cb_plot_best        ->setEnabled( !running );
   cb_plot_rg          ->setEnabled( !running );
   cb_normalize        ->setEnabled( !running );
}

bool US_Hydrodyn_Saxs_Screen::any_to_run()
{
   for ( unsigned int i = 0; i < (unsigned int)t_csv->rowCount(); i++ )
   {
      if ( ((QCheckBox *)(t_csv->cellWidget( i, 1 )))->isChecked() &&
           !t_csv->item( i, 0 )->text().contains("Ending q (A^-1)") )
      {
         return true;
      }
   }
   editor_msg("dark red", "Nothing to do: At least one of the non \"Ending q\" rows must have Active set to Y");
   return false;
}

void US_Hydrodyn_Saxs_Screen::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->textColor();
   editor->setTextColor(color);
   editor->append(msg);
   editor->setTextColor(save_color);
   editor->verticalScrollBar()->setValue(editor->verticalScrollBar()->maximum());
}

void US_Hydrodyn_Saxs_Screen::editor_msg_qc( QColor qcolor, QString msg )
{
   QColor save_color = editor->textColor();
   editor->setTextColor(qcolor);
   editor->append(msg);
   editor->setTextColor(save_color);
}

bool US_Hydrodyn_Saxs_Screen::validate()
{
   bool errors = false;

   for ( unsigned int i = 0; i < (unsigned int)t_csv->rowCount(); i++ )
   {
      if ( t_csv->item( i, 4 )->text().toDouble() != 1 &&
           t_csv->item( i, 2 )->text().toDouble() >
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
      // if ( t_csv->item( i, 4 )->text().toDouble() == 1  &&
      // t_csv->text( i, 2 ).toDouble() > t_csv->item( i, 3 )->text().toDouble() )
      // {
      // editor_msg("red", QString("Row %1 one \"Points\" requires \"Low value\" equals \"High value\"\n").arg(i));
      // errors = true;
      // }
   }
   errors |= !validate_saxs_window();
   return !errors;
}

bool US_Hydrodyn_Saxs_Screen::activate_saxs_window()
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

bool US_Hydrodyn_Saxs_Screen::validate_saxs_window()
{
   if ( !activate_saxs_window() )
   {
      return false;
   }

   if ( !lbl_current_target->text().isEmpty() &&
        !saxs_window->qsl_plotted_iq_names.filter(QRegExp(QString("^%1$").arg(lbl_current_target->text()))).size() )
   {
      // need to replot
      editor_msg("dark red", "Current target missing from SAXS window plot, replotting");
      do_replot_saxs();
   }

   return true;
}

void US_Hydrodyn_Saxs_Screen::do_replot_saxs()
{
   saxs_window->clear_plot_saxs(true);
   saxs_window->plotted = false;
   for ( unsigned int i = 0; i < names.size(); i++ )
   {
      saxs_window->plot_one_iqq( qs[ i ], Is[ i ], I_errors[ i ], names[ i ] );
   }
   if ( saxs_window->plotted )
   {
      saxs_window->editor_msg( "black", "I(q) plot done\n");
      saxs_window->plotted = false;
   }
   saxs_window->rescale_plot();
}

void US_Hydrodyn_Saxs_Screen::replot_saxs()
{
   if ( activate_saxs_window() )
   {
      do_replot_saxs();
   }
}

void US_Hydrodyn_Saxs_Screen::save_saxs_plot()
{
   if ( !activate_saxs_window() )
   {
      return;
   }

   if ( !saxs_window->plotted_q.size() )
   {
      QMessageBox::information(this, 
                               us_tr("US-SOMO: I(q) screen"),
                               QString(us_tr("Nothing plotted")));
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
                                           "US_SOMO: I(q) screen",
                                           us_tr("The target is not currently plotted"),
                                           us_tr("Clear target and continue"), 
                                           us_tr("Add target to stored plots"), 
                                           us_tr("Cancel"),
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
                              "US_SOMO: I(q) screen",
                              us_tr("Target not found, clearing"));
         lbl_current_target->setText("");
      }
   }

   qs      .clear( );
   Is      .clear( );
   I_errors.clear( );
   names   .clear( );

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

csv US_Hydrodyn_Saxs_Screen::current_csv()
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
  
void US_Hydrodyn_Saxs_Screen::recompute_interval_from_points()
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

void US_Hydrodyn_Saxs_Screen::recompute_points_from_interval()
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

void US_Hydrodyn_Saxs_Screen::adjust_wheel( double pos )
{
   // cout << QString("pos is now %1\n").arg(pos);
   if ( last_plotted_pos != (unsigned int) pos )
   {
      // cout << "call plot pos\n";
      plot_pos( (unsigned int) pos );
   }
}

void US_Hydrodyn_Saxs_Screen::adjust_wheel2( double pos )
{
   // cout << QString("row is now %1\n").arg(pos);
   if ( current_row != (unsigned int) pos )
   {
      // cout << "call plot row\n";
      plot_row( (unsigned int) pos );
   }
}

void US_Hydrodyn_Saxs_Screen::push()
{
   current_row = messages.size();

   messages           .resize( current_row + 1 );
   messages2          .resize( current_row + 1 );
   messages3          .resize( current_row + 1 );
   messages4          .resize( current_row + 1 );
   radiis             .resize( current_row + 1 );
   intensitys         .resize( current_row + 1 );
   best_fit_radiuss   .resize( current_row + 1 );
   best_fit_delta_rhos.resize( current_row + 1 );
   average_radiuss    .resize( current_row + 1 );
   average_delta_rhos .resize( current_row + 1 );
   target_rgs         .resize( current_row + 1 );
   use_chi2s          .resize( current_row + 1 );
   chi2_bests         .resize( current_row + 1 );
   chi2_nnlss         .resize( current_row + 1 );
   
   qwtw_wheel->setRange( 0.0, 0.0); qwtw_wheel->setSingleStep( 1 );
   {
      double rs = ( current_row * current_row * 0.1 < 1.0 )
         ? 
         current_row * current_row * 0.1 
         :
         1.0
         ;
      qwtw_wheel2->setRange( (double) current_row + 0.5, 0.5); qwtw_wheel2->setSingleStep( rs );
   }
   plot_row( current_row );
}

void US_Hydrodyn_Saxs_Screen::clear_plot()
{
   plot_dist->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_dist->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   plot_dist->replot();
#if QT_VERSION < 0x040000
   if ( plot_dist_zoomer )
   {
      delete plot_dist_zoomer;
      plot_dist_zoomer = (ScrollZoomer *) 0;
   }
#endif
   lbl_message->setText("");
   lbl_message2->setText("");
   lbl_message3->setText("");
   lbl_message4->setText("");
   lbl_pos_range->setText("0 of 0");
}

void US_Hydrodyn_Saxs_Screen::clear_plot_row()
{
   for ( unsigned int i = current_row + 1; i < messages.size(); i++ )
   {
      messages           [ i - 1 ] = messages           [ i ];
      messages2          [ i - 1 ] = messages2          [ i ];
      messages3          [ i - 1 ] = messages3          [ i ];
      messages4          [ i - 1 ] = messages4          [ i ];
      radiis             [ i - 1 ] = radiis             [ i ];
      intensitys         [ i - 1 ] = intensitys         [ i ];
      best_fit_radiuss   [ i - 1 ] = best_fit_radiuss   [ i ];
      best_fit_delta_rhos[ i - 1 ] = best_fit_delta_rhos[ i ];
      average_radiuss    [ i - 1 ] = average_radiuss    [ i ];
      average_delta_rhos [ i - 1 ] = average_delta_rhos [ i ];
      target_rgs         [ i - 1 ] = target_rgs         [ i ];
      use_chi2s          [ i - 1 ] = use_chi2s          [ i ];
      chi2_bests         [ i - 1 ] = chi2_bests         [ i ];
      chi2_nnlss         [ i - 1 ] = chi2_nnlss         [ i ];
   }
      
   if ( messages.size() > 1 )
   {
      messages           .pop_back();
      messages2          .pop_back();
      messages3          .pop_back();
      messages4          .pop_back();
      radiis             .pop_back();
      intensitys         .pop_back();
      best_fit_radiuss   .pop_back();
      best_fit_delta_rhos.pop_back();
      average_radiuss    .pop_back();
      average_delta_rhos .pop_back();
      target_rgs         .pop_back();
      use_chi2s          .pop_back();
      chi2_bests         .pop_back();
      chi2_nnlss         .pop_back();
   } else {
      messages           [ 0 ].clear( );
      messages2          [ 0 ].clear( );
      messages3          [ 0 ].clear( );
      messages4          [ 0 ].clear( );
      radiis             [ 0 ].clear( );
      intensitys         [ 0 ].clear( );
      best_fit_radiuss   [ 0 ].clear( );
      best_fit_delta_rhos[ 0 ].clear( );
      average_radiuss    [ 0 ].clear( );
      average_delta_rhos [ 0 ].clear( );
      target_rgs         [ 0 ].clear( );
      use_chi2s          [ 0 ].clear( );
      chi2_bests         [ 0 ].clear( );
      chi2_nnlss         [ 0 ].clear( );
   }      

   if ( current_row >= messages.size() )
   {
      current_row = messages.size() - 1;
   }
   
   plot_row( current_row );
}

void US_Hydrodyn_Saxs_Screen::clear_plot_all()
{
   anything_plotted_since_save = false;
   clear_plot();
   current_row = 0;
   last_plotted_pos = 0;

   messages           .resize(1);
   messages2          .resize(1);
   messages3          .resize(1);
   messages4          .resize(1);
   radiis             .resize(1);
   intensitys         .resize(1);
   best_fit_radiuss   .resize(1);
   best_fit_delta_rhos.resize(1);
   average_radiuss    .resize(1);
   average_delta_rhos .resize(1);
   target_rgs         .resize(1);
   use_chi2s          .resize(1);
   chi2_bests         .resize(1);
   chi2_nnlss         .resize(1);

   messages           [ 0 ].clear( );
   messages2          [ 0 ].clear( );
   messages3          [ 0 ].clear( );
   messages4          [ 0 ].clear( );
   radiis             [ 0 ].clear( );
   intensitys         [ 0 ].clear( );
   best_fit_radiuss   [ 0 ].clear( );
   best_fit_delta_rhos[ 0 ].clear( );
   average_radiuss    [ 0 ].clear( );
   average_delta_rhos [ 0 ].clear( );
   target_rgs         [ 0 ].clear( );
   use_chi2s          [ 0 ].clear( );
   chi2_bests         [ 0 ].clear( );
   chi2_nnlss         [ 0 ].clear( );

   qwtw_wheel->setRange( 0.0, 0.0); qwtw_wheel->setSingleStep( 1 );
   qwtw_wheel2->setRange( 1.0, 1.0); qwtw_wheel2->setSingleStep( 1 );

   lbl_pos_range2->setText("1\nof\n1");
}

bool US_Hydrodyn_Saxs_Screen::anything_plotted()
{
   for ( unsigned int i = 0; i < messages.size(); i++ )
   {
      if ( messages[ i ].size() )
      {
         return true;
      }
   }
   return false;
}

void US_Hydrodyn_Saxs_Screen::save_plot()
{
   if ( !anything_plotted() )
   {
      QMessageBox::information(this, 
                               us_tr("US-SOMO: SAXS screen save plots "),
                               QString(us_tr("Nothing plotted")));
      return;
   }

   if ( !validate_saxs_window() )
   {
      return;
   }

   QString use_dir = 
      saxs_window->our_saxs_options->path_load_saxs_curve.isEmpty() ?
      USglobal->config_list.root_dir + SLASH + "somo" + SLASH + "saxs" :
      saxs_window->our_saxs_options->path_load_saxs_curve;
   saxs_window->select_from_directory_history( use_dir, this );
   QString filename = QFileDialog::getSaveFileName( this , us_tr("Choose a filename to save the plots") , use_dir , "*.ssc *.SSC" );



   if ( filename.isEmpty() )
   {
      return;
   }

   if ( !filename.contains(QRegExp(".ssc$", Qt::CaseInsensitive )) )
   {
      filename += ".ssc";
   }

   saxs_window->add_to_directory_history( filename );

   csv plot_csv = plots_to_csv();

   if ( QFile::exists(filename) )
   {
      filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( filename, 0, this );
   }

   QFile f(filename);

   if ( !f.open( QIODevice::WriteOnly ) )
   {
      QMessageBox::warning( this, "UltraScan",
                            QString(us_tr("Could not open %1 for writing!")).arg(filename) );
      return;
   }

   QTextStream t( &f );

   QString qs;

   for ( unsigned int i = 0; i < plot_csv.header.size(); i++ )
   {
      qs += QString("%1\"%2\"").arg(i ? "," : "").arg(plot_csv.header[i]);
   }

   t << qs << Qt::endl;

   for ( unsigned int i = 0; i < plot_csv.data.size(); i++ )
   {
      qs = "";
      for ( unsigned int j = 0; j < plot_csv.data[i].size(); j++ )
      {
         qs += QString("%1%2").arg(j ? "," : "").arg(plot_csv.data[i][j]);
      }
      t << qs << Qt::endl;
   }
   f.close();
   editor_msg("black", QString("File %1 written\n").arg( filename ) );
   anything_plotted_since_save = false;
}

void US_Hydrodyn_Saxs_Screen::load_plot()
{
   if ( !validate_saxs_window() )
   {
      return;
   }

   QString use_dir = 
      saxs_window->our_saxs_options->path_load_saxs_curve.isEmpty() ?
      USglobal->config_list.root_dir + SLASH + "somo" + SLASH + "saxs" :
      saxs_window->our_saxs_options->path_load_saxs_curve;
   saxs_window->select_from_directory_history( use_dir, this );
   QString filename = QFileDialog::getOpenFileName( this , windowTitle() , use_dir , "*.ssc *.SSC" );

   if ( filename.isEmpty() )
   {
      return;
   }

   saxs_window->add_to_directory_history( filename );

   csv csv1;

   QString csv_error = "";
   QString csv_warn = "";

   QFile f(filename);

   if ( !f.exists() )
   {
      csv_error = QString(us_tr("File %1 does not exist")).arg(f.fileName());
      return;
   }

   if ( !f.open(QIODevice::ReadOnly) )
   {
      csv_error = QString(us_tr("Can not open file %1.  Check permissions")).arg(f.fileName());
      return;
   }

   QTextStream ts( &f );

   QStringList qsl;

   while( !ts.atEnd() )
   {
      qsl << ts.readLine();
   }

   f.close();

   csv1.name = filename;

   int i = 0;

   QStringList qsl_h = csv_parse_line(qsl[0]);

   for ( QStringList::iterator it = qsl_h.begin();
         it != qsl_h.end();
         it++ )
   {
      QString qs = *it;
      qs.replace("\"","");
      if ( csv1.header_map.count(qs) )
      {
         QMessageBox::information(this, 
                                  us_tr("US-SOMO: SAXS screen load plots "),
                                  QString(us_tr("Duplicate header name \"%1\" found in file %2")).arg(qs).arg(f.fileName()));
         return;
      }
      csv1.header_map[qs] = i++;
      csv1.header.push_back(qs);
   }

   unsigned int row = 1;
   {
      QStringList::iterator it = qsl.begin();
      it++;
      for ( ;
            it != qsl.end();
            it++ )
      {
         row++;
         QStringList qsl_d = csv_parse_line(*it);
         vector < QString > vqs;
         vector < double > vd;
         if ( qsl_d.size() )
         {
            for ( QStringList::iterator it2 = qsl_d.begin();
                  it2 != qsl_d.end();
                  it2++ )
            {
               vqs.push_back(*it2);
               vd.push_back((*it2).toDouble());
            }
            csv1.data.push_back(vqs);
            csv1.num_data.push_back(vd);
         }
      }
   }

   if ( !csv1.data.size() )
   {
      QMessageBox::information(this, 
                               us_tr("US-SOMO: SAXS screen load plots "),
                               QString(us_tr("Error in file")));
      return;
   }
   csv_to_plots( csv1 );
   editor_msg("black", QString("%1 loaded\n").arg( filename ) );
}

csv US_Hydrodyn_Saxs_Screen::plots_to_csv()
{
   csv plot_csv;
   plot_csv.name = "US-SOMO saxs screen";

   plot_csv.header.push_back("Row");
   plot_csv.header.push_back("Column");
   plot_csv.header.push_back("Message 1");
   plot_csv.header.push_back("Message 2");
   plot_csv.header.push_back("Message 3");
   plot_csv.header.push_back("Message 4");
   plot_csv.header.push_back("Best fit radius");
   plot_csv.header.push_back("Best fit delta rho");
   plot_csv.header.push_back("Average radius");
   plot_csv.header.push_back("Average delta rho");
   plot_csv.header.push_back("Rg");
   plot_csv.header.push_back("Use nchi");
   plot_csv.header.push_back("Best fit nchi");
   plot_csv.header.push_back("NNLS nchi");
   plot_csv.header.push_back("Number of points");
   plot_csv.header.push_back("Radius");
   plot_csv.header.push_back("Intensity");

   for ( unsigned int i = 0; i < messages.size(); i++ )
   {
      for ( unsigned int j = 0; j < messages[ i ].size(); j++ )
      {
         vector < QString > data;
         data.push_back( QString("%1").arg( i ) );
         data.push_back( QString("%1").arg( j ) );
         data.push_back( messages[ i ][ j ] );
         data.push_back( messages2[ i ][ j ] );
         data.push_back( messages3[ i ][ j ] );
         data.push_back( messages4[ i ][ j ] );
         data.push_back( QString("%1").arg( best_fit_radiuss[ i ][ j ] ) );
         data.push_back( QString("%1").arg( best_fit_delta_rhos[ i ][ j ] ) );
         data.push_back( QString("%1").arg( average_radiuss[ i ][ j ] ) );
         data.push_back( QString("%1").arg( average_delta_rhos[ i ][ j ] ) );
         data.push_back( QString("%1").arg( target_rgs[ i ][ j ] ) );
         data.push_back( QString("%1").arg( use_chi2s[ i ][ j ] ) );
         data.push_back( QString("%1").arg( chi2_bests[ i ][ j ] ) );
         data.push_back( QString("%1").arg( chi2_nnlss[ i ][ j ] ) );
         data.push_back( QString("%1").arg( radiis[ i ][ j ].size() ) );
         for ( unsigned int k = 0; k < radiis[ i ][ j ].size(); k++ )
         {
            data.push_back( QString("%1").arg( radiis[ i ][ j ][ k ] ) );
            data.push_back( QString("%1").arg( intensitys[ i ][ j ][ k ] ) );
         }
         plot_csv.data.push_back( data );
      }
   }
   return plot_csv;
}

void US_Hydrodyn_Saxs_Screen::csv_to_plots( csv plot_csv )
{
   unsigned int last_row = 0;
   for ( unsigned int i = 0; i < plot_csv.data.size(); i++ )
   {
      if ( plot_csv.data[ i ].size() < 12 )
      {
         QMessageBox::information(this, 
                                  us_tr("US-SOMO: SAXS screen"),
                                  QString(us_tr("US_Hydrodyn_Saxs_Screen::csv_to_plots invalid csv")));
         return;
      }
      
      unsigned pos = 0;
      if ( i && last_row !=  plot_csv.data[ i ][ pos ].toUInt() )
      {
         push();
      }
      last_row = plot_csv.data[ i ][ pos++ ].toUInt();
      pos++;
      
      messages           [ current_row ].push_back( plot_csv.data[ i ][ pos++ ] );
      messages2          [ current_row ].push_back( plot_csv.data[ i ][ pos++ ] );
      messages3          [ current_row ].push_back( plot_csv.data[ i ][ pos++ ] );
      messages4          [ current_row ].push_back( plot_csv.data[ i ][ pos++ ] );

      best_fit_radiuss   [ current_row ].push_back( plot_csv.data[ i ][ pos++ ].toDouble() );
      best_fit_delta_rhos[ current_row ].push_back( plot_csv.data[ i ][ pos++ ].toDouble() );
      average_radiuss    [ current_row ].push_back( plot_csv.data[ i ][ pos++ ].toDouble() );
      average_delta_rhos [ current_row ].push_back( plot_csv.data[ i ][ pos++ ].toDouble() );
      target_rgs         [ current_row ].push_back( plot_csv.data[ i ][ pos++ ].toDouble() );
      use_chi2s          [ current_row ].push_back( (bool) plot_csv.data[ i ][ pos++ ].toInt() );
      chi2_bests         [ current_row ].push_back( plot_csv.data[ i ][ pos++ ].toDouble() );
      chi2_nnlss         [ current_row ].push_back( plot_csv.data[ i ][ pos++ ].toDouble() );

      unsigned int datapoints = plot_csv.data[ i ][ pos++ ].toUInt();

      vector < double > radii    ( datapoints );
      vector < double > intensity( datapoints );

      if ( plot_csv.data[ i ].size() < pos + datapoints * 2 )
      {
         QMessageBox::information(this, 
                                  us_tr("US-SOMO: SAXS screen"),
                                  QString(us_tr("US_Hydrodyn_Saxs_Screen::csv_to_plots invalid csv")));
         return;
      }

      for ( unsigned int j = 0; j < datapoints; j++ )
      {
         radii    .push_back( plot_csv.data[ i ][ pos++ ].toDouble() );
         intensity.push_back( plot_csv.data[ i ][ pos++ ].toDouble() );
      }

      radiis    [ current_row ].push_back( radii );
      intensitys[ current_row ].push_back( intensity );
   }
   if ( messages[ current_row ].size() )
   {
      last_plotted_pos = messages[ current_row ].size() - 1;
   } else {
      last_plotted_pos = 0;
   }
   update_wheel_range();
   qwtw_wheel->setValue( (double) last_plotted_pos );
   replot(); // plot_pos( last_plotted_pos );
}

QStringList US_Hydrodyn_Saxs_Screen::csv_parse_line( QString qs )
{
   // cout << QString("csv_parse_line:\ninital string <%1>\n").arg(qs);
   QStringList qsl;
   if ( qs.isEmpty() )
   {
      // cout << QString("csv_parse_line: empty\n");
      return qsl;
   }
   if ( !qs.contains(",") )
   {
      // cout << QString("csv_parse_line: one token\n");
      qsl << qs;
      return qsl;
   }

   QStringList qsl_chars = (qs).split( "" , Qt::SkipEmptyParts );
   QString token = "";

   bool in_quote = false;

   for ( QStringList::iterator it = qsl_chars.begin();
         it != qsl_chars.end();
         it++ )
   {
      if ( !in_quote && *it == "," )
      {
         qsl << token;
         token = "";
         continue;
      }
      if ( in_quote && *it == "\"" )
      {
         in_quote = false;
         continue;
      }
      if ( !in_quote && *it == "\"" )
      {
         in_quote = true;
         continue;
      }
      if ( !in_quote && *it == "\"" )
      {
         in_quote = false;
         continue;
      }
      token += *it;
   }
   if ( !token.isEmpty() )
   {
      qsl << token;
   }
   // cout << QString("csv_parse_line results:\n<%1>\n").arg(qsl.join(">\n<"));
   return qsl;
}

bool US_Hydrodyn_Saxs_Screen::get_guinier_rg( QString name, double &Rg )
{
   if ( guinier_rgs.count( name ) )
   {
      Rg = guinier_rgs[ name ];
      return true;
   }

   set_target( name );

   if ( !last_target_found )
   {
      Rg = 0e0;
      return false;
   }

   US_Saxs_Util usu;
   usu.wave["data"].q = qs[ last_target_pos ];
   usu.wave["data"].r = Is[ last_target_pos ];
   usu.wave["data"].s = I_errors[ last_target_pos ];
   QString log;

   int pointsmin = saxs_window->our_saxs_options->pointsmin;
   int pointsmax = saxs_window->our_saxs_options->pointsmax;
   double sRgmaxlimit = saxs_window->our_saxs_options->qRgmax;
   double pointweightpower = 3e0;
   double p_guinier_maxq = saxs_window->our_saxs_options->qend;
   
   // these are function output values
   double a;
   double b;
   double siga;
   double sigb;
   double chi2;
   // double Rg;
   double Io;
   double smin;
   double smax;
   double sRgmin;
   double sRgmax;

   unsigned int beststart;
   unsigned int bestend;

   bool too_few_points = qs[ last_target_pos ].size() <= 25;

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
         editor_msg("red", QString("Error performing Guinier analysis on %1\n" + usu.errormsg + "\n")
                        .arg( name ) );
         Rg = 0e0;
         return false;
      }

      // cout << "guinier siga " << siga << endl;
      // cout << "guinier sigb " << sigb << endl;
   
      // cout << log;
   }

   if ( too_few_points )
   {
      editor_msg("red", 
                 QString(
                         "Guinier analysis of %1:\n"
                         "**** Could not compute Rg, too few data points %2 ****\n"
                         )
                 .arg( qs[ last_plotted_pos ].size() ) );
      Rg = 0e0;
      return false;
   } else {
      if ( us_isnan(Rg) ||
           b >= 0e0 )
      {
         editor_msg("red", 
                    QString(
                            "Guinier analysis of %1:\n"
                            "**** Could not compute Rg ****\n"
                            )
                    .arg( name ) );
         Rg = 0e0;
         return false;
      } else {
         editor_msg("blue", 
                    QString("")
                    .sprintf(
                             "Guinier analysis of %s:\n"
                             "Rg %.1f (%.1f) (A) I(0) %.2e (%.2e) qRgmin %.3f qRgmax %.3f points used %u chi^2 %.2e\n"
                             
                             , name.toLatin1().data()
                             , Rg
                             , sqrt(3e0) * 5e-1 * (1e0/sqrt(-b)) * sigb 
                             , Io
                             , siga
                             , sRgmin
                             , sRgmax
                             , bestend - beststart + 1
                             , chi2
                             ) );
         guinier_rgs[ name ] = Rg;
         return true;
      }
   }
}

void US_Hydrodyn_Saxs_Screen::replot()
{
   plot_pos( last_plotted_pos );
}

void US_Hydrodyn_Saxs_Screen::usp_config_plot_dist( const QPoint & ) {
   US_PlotChoices *uspc = new US_PlotChoices( usp_plot_dist );
   uspc->exec();
   delete uspc;
}
