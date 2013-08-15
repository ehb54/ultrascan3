#include "../include/us_hydrodyn_saxs_hplc_svd.h"

#ifdef QT4
#include <qwt_scale_engine.h>
#endif
#include <qpalette.h>

// note: this program uses cout and/or cerr and this should be replaced

#define SLASH QDir::separator()

US_Hydrodyn_Saxs_Hplc_Svd::US_Hydrodyn_Saxs_Hplc_Svd(
                                                     US_Hydrodyn_Saxs_Hplc *hplc_win,
                                                     map < QString, int >  &selected_files,
                                                     QWidget *p, 
                                                     const char *name
                                                     ) : QFrame(p, name)
{
   this->hplc_win       = hplc_win;
   this->selected_files = selected_files;
   this->ush_win        = (US_Hydrodyn *)(hplc_win->us_hydrodyn);

   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("US-SOMO: SAXS HPLC SVD"));

   if ( !selected_files.size() )
   {
      QMessageBox::warning( this, 
                            caption(),
                            tr( "Internal error: HPLC SVD called with no curves selected" ) );
      close();
      return;
   }

   for ( map < QString, int >::iterator it = selected_files.begin();
         it != selected_files.end();
         ++it )
   {
      QString this_name = it->first;
      
      f_qs_string[ this_name ] = hplc_win->f_qs_string[ this_name ];
      f_qs       [ this_name ] = hplc_win->f_qs       [ this_name ];
      f_Is       [ this_name ] = hplc_win->f_Is       [ this_name ];
      f_errors   [ this_name ] = hplc_win->f_errors   [ this_name ];
      f_is_time  [ this_name ] = false;  // must all be I(q)
   }

   plot_data_zoomer      = (ScrollZoomer *) 0;
   // plot_errors_zoomer = (ScrollZoomer *) 0;

   le_last_focus      = (mQLineEdit *) 0;
   
   disable_updates = false;

   setupGUI();

   puts( "m5" );
   axis_y_log = false;
   axis_x_log = false;

   puts( "m6" );
   update_enables();
   puts( "m7" );

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry(global_Xpos, global_Ypos, 0, 0 );
   puts( "m8" );
}

US_Hydrodyn_Saxs_Hplc_Svd::~US_Hydrodyn_Saxs_Hplc_Svd()
{
}

void US_Hydrodyn_Saxs_Hplc_Svd::setupGUI()
{
   int minHeight1 = 24;
   int minHeight3 = 25;

   // ------ data section 

   lbl_data = new mQLabel("Data files", this);
   lbl_data->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_data->setMinimumHeight(minHeight1);
   lbl_data->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_data->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   connect( lbl_data, SIGNAL( pressed() ), SLOT( hide_data() ) );

   lv_data = new QListView( this );
   lv_data->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lv_data->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lv_data->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lv_data->setEnabled(true);
   lv_data->setMinimumWidth( 175 );

   lv_data->addColumn( "Source" );
   lv_data->addColumn( "Type" );
   lv_data->addColumn( "Curves" );

   lv_data->setSorting        ( -1 );
   lv_data->setRootIsDecorated( true );
   lv_data->setSelectionMode  ( QListView::Extended );
   connect(lv_data, SIGNAL(selectionChanged()), SLOT( data_selection_changed() ));

   QListViewItem *element = new QListViewItem( lv_data, QString( tr( "Original data" ) ) );
   QListViewItem *iq = new QListViewItem( element, "I(q)" );
   QListViewItem *it = new QListViewItem( element, "I(t)" );

   for ( map < QString, int >::iterator it = selected_files.begin();
         it != selected_files.end();
         ++it )
   {
      new QListViewItem( iq, it->first );
   }

   // make i(t), add also

   data_widgets.push_back( lv_data );
   
   pb_to_hplc = new QPushButton(tr("Copy selected to HPLC produced data"), this);
   pb_to_hplc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_to_hplc->setMinimumHeight(minHeight3);
   pb_to_hplc->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_to_hplc, SIGNAL(clicked()), SLOT(to_hplc()));
   data_widgets.push_back( pb_to_hplc );

   // ------ editor section 

   lbl_editor = new mQLabel("Messages", this);
   lbl_editor->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_editor->setMinimumHeight(minHeight1);
   lbl_editor->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_editor->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   connect( lbl_editor, SIGNAL( pressed() ), SLOT( hide_editor() ) );

   editor = new QTextEdit(this);
   editor->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   editor->setReadOnly(true);
   editor->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ));

   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight3);
   editor_widgets.push_back( frame );

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

   editor_widgets.push_back( editor );

   // ------ plot section

   plot_data = new QwtPlot(this);
#ifndef QT4
   // plot_data->enableOutline(true);
   // plot_data->setOutlinePen(Qt::white);
   // plot_data->setOutlineStyle(Qwt::VLine);
   plot_data->enableGridXMin();
   plot_data->enableGridYMin();
#else
   grid_saxs = new QwtPlotGrid;
   grid_saxs->enableXMin( true );
   grid_saxs->enableYMin( true );
#endif
   plot_data->setPalette( QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
#ifndef QT4
   plot_data->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot_data->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   grid_saxs->setMajPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid_saxs->setMinPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid_saxs->attach( plot_data );
#endif
   plot_data->setAxisTitle(QwtPlot::xBottom, /* cb_guinier->isChecked() ? tr("q^2 (1/Angstrom^2)") : */  tr("q [1/Angstrom]" )); // or Time or Frame"));
   plot_data->setAxisTitle(QwtPlot::yLeft, tr("Intensity [a.u.] (log scale)"));
#ifndef QT4
   plot_data->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   plot_data->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_data->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#ifndef QT4
   plot_data->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_data->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#ifndef QT4
   plot_data->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_data->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_data->setMargin(USglobal->config_list.margin);
   plot_data->setTitle("");
#ifndef QT4
   plot_data->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::Logarithmic);
#else
   plot_data->setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);
#endif
   plot_data->setCanvasBackground(USglobal->global_colors.plot);

   // connect( plot_data_zoomer, SIGNAL( zoomed( const QwtDoubleRect & ) ), SLOT( plot_data_zoomed( const QwtDoubleRect & ) ) );

   pb_iq_it = new QPushButton( tr("I(q)/I(t) toggle"), this);
   pb_iq_it->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_iq_it->setMinimumHeight(minHeight1);
   pb_iq_it->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_iq_it, SIGNAL(clicked()), SLOT(iq_it()));

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


   // ------ process section 

   lbl_process = new mQLabel("Process", this);
   lbl_process->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_process->setMinimumHeight(minHeight1);
   lbl_process->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_process->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   connect( lbl_process, SIGNAL( pressed() ), SLOT( hide_process() ) );

   lbl_q_range = new QLabel( tr( "Active q range:" ), this );
   lbl_q_range->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_q_range->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_q_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   process_widgets.push_back( lbl_q_range );

   le_q_start = new mQLineEdit(this, "le_q_start Line Edit");
   le_q_start->setText( "" );
   le_q_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_q_start->setPalette(QPalette( USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_q_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_q_start->setEnabled( false );
   le_q_start->setValidator( new QDoubleValidator( le_q_start ) );
   connect( le_q_start, SIGNAL( textChanged( const QString & ) ), SLOT( q_start_text( const QString & ) ) );
   process_widgets.push_back( le_q_start );

   le_q_end = new mQLineEdit(this, "le_q_end Line Edit");
   le_q_end->setText( "" );
   le_q_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_q_end->setPalette(QPalette( USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_q_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_q_end->setEnabled( false );
   le_q_end->setValidator( new QDoubleValidator( le_q_end ) );
   connect( le_q_end, SIGNAL( textChanged( const QString & ) ), SLOT( q_end_text( const QString & ) ) );
   process_widgets.push_back( le_q_end );

   lbl_t_range = new QLabel( tr( "Active Time range:" ), this );
   lbl_t_range->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_t_range->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_t_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   process_widgets.push_back( lbl_t_range );

   le_t_start = new mQLineEdit(this, "le_t_start Line Edit");
   le_t_start->setText( "" );
   le_t_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_t_start->setPalette(QPalette( USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_t_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_t_start->setEnabled( false );
   le_t_start->setValidator( new QDoubleValidator( le_t_start ) );
   connect( le_t_start, SIGNAL( textChanged( const QString & ) ), SLOT( t_start_text( const QString & ) ) );
   process_widgets.push_back( le_t_start );

   le_t_end = new mQLineEdit(this, "le_t_end Line Edit");
   le_t_end->setText( "" );
   le_t_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_t_end->setPalette(QPalette( USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_t_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_t_end->setEnabled( false );
   le_t_end->setValidator( new QDoubleValidator( le_t_end ) );
   connect( le_t_end, SIGNAL( textChanged( const QString & ) ), SLOT( t_end_text( const QString & ) ) );
   process_widgets.push_back( le_t_end );

   lbl_ev_count = new QLabel( tr( "Number of Eigenvalues to use for reconstruction:" ), this );
   lbl_ev_count->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_ev_count->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_ev_count->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   process_widgets.push_back( lbl_ev_count );

   le_ev_count = new mQLineEdit(this, "le_ev_count Line Edit");
   le_ev_count->setText( "" );
   le_ev_count->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_ev_count->setPalette(QPalette( USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_ev_count->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_ev_count->setEnabled( false );
   le_ev_count->setValidator( new QIntValidator( 1, 100, le_ev_count ) );
   connect( le_ev_count, SIGNAL( textChanged( const QString & ) ), SLOT( ev_count_text( const QString & ) ) );
   process_widgets.push_back( le_ev_count );

   lbl_ev = new QLabel( tr( "Eigenvalue list:" ), this );
   lbl_ev->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_ev->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_ev->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   process_widgets.push_back( lbl_ev );

   lb_ev = new QListBox(this, "files files listbox" );
   lb_ev->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lb_ev->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lb_ev->setEnabled(true);
   lb_ev->setSelectionMode( QListBox::Single );
   lb_ev->setColumnMode   ( QListBox::FitToWidth );
   connect( lb_ev, SIGNAL( selectionChanged() ), SLOT( ev_selection_changed() ) );
   process_widgets.push_back( lb_ev );

   pb_svd = new QPushButton(tr("Compute SVD"), this);
   pb_svd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_svd->setMinimumHeight(minHeight3);
   pb_svd->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_svd, SIGNAL(clicked()), SLOT(svd()));
   process_widgets.push_back( pb_svd );

   pb_recon = new QPushButton(tr("TSVD reconstruction"), this);
   pb_recon->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_recon->setMinimumHeight(minHeight3);
   pb_recon->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_recon, SIGNAL(clicked()), SLOT(recon()));
   process_widgets.push_back( pb_recon );

   // -------- bottom section

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   progress = new QProgressBar(this, "Progress");
   // progress->setMinimumHeight(minHeight1);
   progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   progress->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   progress->reset();

   pb_cancel = new QPushButton(tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // -------- build layout


   QVBoxLayout *background = new QVBoxLayout(this);
   QHBoxLayout *top        = new QHBoxLayout(this);

   // ----- left side
   {
      QBoxLayout *bl = new QVBoxLayout( 0 );
      bl->addWidget( lbl_data );
      bl->addWidget( lv_data );

      {
         QBoxLayout *bl_buttons = new QHBoxLayout( 0 );
         bl_buttons->addWidget( pb_to_hplc );
         bl->addLayout( bl_buttons );
      }

      bl->addWidget( lbl_editor );
      bl->addWidget( frame );
      bl->addWidget( editor );

      top->addLayout ( bl );
   }      


   // ----- right side
   {
      QBoxLayout *bl = new QVBoxLayout( 0 );
      bl->addWidget( plot_data );

      {
         QBoxLayout *bl_buttons = new QHBoxLayout( 0 );
         bl_buttons->addWidget( pb_iq_it );
         bl_buttons->addWidget( pb_axis_x );
         bl_buttons->addWidget( pb_axis_y );
         bl->addLayout( bl_buttons );
      }

      bl->addWidget( lbl_process );
      {
         QGridLayout *gl = new QGridLayout( 0 );
         
         gl->addWidget( lbl_q_range  , 0, 0 );
         gl->addWidget( le_q_start   , 0, 1 );
         gl->addWidget( le_q_end     , 0, 2 );

         gl->addWidget( lbl_t_range  , 1, 0 );
         gl->addWidget( le_t_start   , 1, 1 );
         gl->addWidget( le_t_end     , 1, 2 );

         gl->addWidget( lbl_ev_count , 2, 0 );
         gl->addMultiCellWidget( le_ev_count  , 2, 2, 1, 2 );

         gl->addWidget( lbl_ev       , 3, 0 );
         gl->addMultiCellWidget( lb_ev        , 3, 3, 1, 2 );

         bl->addLayout( gl );
      }

      {
         QBoxLayout *bl_buttons = new QHBoxLayout( 0 );
         bl_buttons->addWidget( pb_svd );
         bl_buttons->addWidget( pb_recon );
         bl->addLayout( bl_buttons );
      }

      top->addLayout( bl );
   }
   

   background->addLayout( top );

   {
      QBoxLayout *bottom = new QHBoxLayout( 0 );
      bottom->addWidget( pb_help );
      bottom->addWidget( progress );
      bottom->addWidget( pb_cancel );
      
      background->addSpacing( 2 );
      background->addLayout( bottom );
   }
   
   hide_widgets( data_widgets, 
                 !ush_win->gparams.count( "hplc_svd_data_widgets" ) || ush_win->gparams[ "hpld_svd_data_widgets" ] == "false" ? false : true );
   hide_widgets( editor_widgets, 
                 !ush_win->gparams.count( "hplc_svd_editor_widgets" ) || ush_win->gparams[ "hpld_svd_editor_widgets" ] == "false" ? false : true );
   hide_widgets( process_widgets,
                 !ush_win->gparams.count( "hplc_svd_process_widgets" ) || ush_win->gparams[ "hpld_svd_process_widgets" ] == "false" ? false : true );
}

void US_Hydrodyn_Saxs_Hplc_Svd::cancel()
{
   close();
}

void US_Hydrodyn_Saxs_Hplc_Svd::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_saxs_hplc_svd.html");
}

void US_Hydrodyn_Saxs_Hplc_Svd::closeEvent(QCloseEvent *e)
{
   e->accept();
}

void US_Hydrodyn_Saxs_Hplc_Svd::clear_display()
{
   editor->clear();
   editor->append("\n\n");
}

void US_Hydrodyn_Saxs_Hplc_Svd::update_font()
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

void US_Hydrodyn_Saxs_Hplc_Svd::save()
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


void US_Hydrodyn_Saxs_Hplc_Svd::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->color();
   editor->setColor(color);
   editor->append(msg);
   editor->setColor(save_color);
}

void US_Hydrodyn_Saxs_Hplc_Svd::plot_files()
{
#if defined( WORKING )
   // puts( "plot_files" );
   plot_data->clear();
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
        plot_data->autoLegend() 
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
   
   if ( !plot_data_zoomer )
   {
      // puts( "redoing zoomer" );
      plot_data->setAxisScale( QwtPlot::xBottom, minx, maxx );
      plot_data->setAxisScale( QwtPlot::yLeft  , miny * 0.9e0 , maxy * 1.1e0 );
      plot_data_zoomer = new ScrollZoomer(plot_data->canvas());
      plot_data_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
#ifndef QT4
      plot_data_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
      connect( plot_data_zoomer, SIGNAL( zoomed( const QwtDoubleRect & ) ), SLOT( plot_zoomed( const QwtDoubleRect & ) ) );
   }
   
   plot_data->replot();
#endif
}

bool US_Hydrodyn_Saxs_Hplc_Svd::plot_file( QString file,
                                           double &minx,
                                           double &maxx,
                                           double &miny,
                                           double &maxy )
{
#if defined( WORKING )
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
   long Iq = plot_data->insertCurve( file );
   plotted_curves[ file ] = Iq;
   plot_data->setCurveStyle( Iq, QwtCurve::Lines );
#else
   QwtPlotCurve *curve = new QwtPlotCurve( file );
   plotted_curves[ file ] = curve;
   curve->setStyle( QwtPlotCurve::Lines );
#endif

   unsigned int q_points = f_qs[ file ].size();

   if ( !axis_y_log )
   {
#ifndef QT4
      plot_data->setCurveData( Iq, 
                               (double *)&( f_qs[ file ][ 0 ] ),
                               (double *)&( f_Is[ file ][ 0 ] ),
                               q_points
                               );
      plot_data->setCurvePen( Iq, QPen( hplc_win->plot_colors[ f_pos[ file ] % hplc_win->plot_colors.size()], 1, SolidLine));
#else
      curve->setData(
                     (double *)&( f_qs[ file ][ 0 ] ),
                     (double *)&( f_Is[ file ][ 0 ] ),
                     q_points
                     );

      curve->setPen( QPen( hplc_win->plot_colors[ f_pos[ file ] % hplc_win->plot_colors.size() ], 1, Qt::SolidLine ) );
      curve->attach( plot_data );
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
      plot_data->setCurveData( Iq, 
                               /* cb_guinier->isChecked() ? (double *)&(plotted_q2[p][0]) : */
                               (double *)&( q[ 0 ] ),
                               (double *)&( I[ 0 ] ),
                               q_points
                               );
      plot_data->setCurvePen( Iq, QPen( hplc_win->plot_colors[ f_pos[ file ] % hplc_win->plot_colors.size()], 1, SolidLine));
#else
      curve->setData(
                     /* cb_guinier->isChecked() ?
                        (double *)&(plotted_q2[p][0]) : */
                     (double *)&( q[ 0 ] ),
                     (double *)&( I[ 0 ] ),
                     q_points
                     );

      curve->setPen( QPen( hplc_win->plot_colors[ f_pos[ file ] % hplc_win->plot_colors.size() ], 1, Qt::SolidLine ) );
      curve->attach( plot_data );
#endif
   }
#endif
            
   return true;
}

bool US_Hydrodyn_Saxs_Hplc_Svd::get_min_max( QString file,
                                             double &minx,
                                             double &maxx,
                                             double &miny,
                                             double &maxy )
{
#if defined( WORKING )
   if ( 
        !f_Is        .count( file ) ||
        !f_pos       .count( file ) )
   {
      // editor_msg( "red", QString( tr( "Internal error: requested %1, but not found in data" ) ).arg( file ) );
      return false;
   }

   minx = f_qs[ 0 ];
   maxx = f_qs.back();

   miny = f_Is[ file ][ 0 ];
   maxy = f_Is[ file ][ 0 ];

   if ( axis_y_log )
   {
      unsigned int i = 0;
      while ( miny <= 0e0 && i < f_Is[ file ].size() )
      {
         miny = f_Is[ file ][ i ];
         maxy = f_Is[ file ][ i ];
         minx = f_qs[ i ];
         maxx = f_qs[ i ];
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
#endif
   return true;
}

void US_Hydrodyn_Saxs_Hplc_Svd::plot_data_zoomed( const QwtDoubleRect & /* rect */ )
{
   //   cout << QString( "zoomed: %1 %2 %3 %4\n" )
   // .arg( rect.x1() )
   // .arg( rect.x2() )
   // .arg( rect.y1() )
   // .arg( rect.y2() );
}

void US_Hydrodyn_Saxs_Hplc_Svd::disable_all()
{
   lv_data    ->setEnabled( false );
   pb_to_hplc ->setEnabled( false );
   pb_iq_it   ->setEnabled( false );
   pb_axis_x  ->setEnabled( false );
   pb_axis_y  ->setEnabled( false );
   le_q_start ->setEnabled( false );
   le_q_end   ->setEnabled( false );
   le_t_start ->setEnabled( false );
   le_t_end   ->setEnabled( false );
   le_ev_count->setEnabled( false );
   pb_svd     ->setEnabled( false );
   pb_recon   ->setEnabled( false );
}

void US_Hydrodyn_Saxs_Hplc_Svd::q_start_text( const QString & )
{
}

void US_Hydrodyn_Saxs_Hplc_Svd::q_end_text( const QString & )
{
}

void US_Hydrodyn_Saxs_Hplc_Svd::t_start_text( const QString & )
{
}

void US_Hydrodyn_Saxs_Hplc_Svd::t_end_text( const QString & )
{
}

void US_Hydrodyn_Saxs_Hplc_Svd::ev_count_text( const QString & )
{
}

int US_Hydrodyn_Saxs_Hplc_Svd::count_selected()
{
   return 0;
}

void US_Hydrodyn_Saxs_Hplc_Svd::clean_selected()
{
}

void US_Hydrodyn_Saxs_Hplc_Svd::svd()
{
}

void US_Hydrodyn_Saxs_Hplc_Svd::iq_it()
{
}

void US_Hydrodyn_Saxs_Hplc_Svd::to_hplc()
{
}

void US_Hydrodyn_Saxs_Hplc_Svd::recon()
{
}

void US_Hydrodyn_Saxs_Hplc_Svd::axis_x()
{
}

void US_Hydrodyn_Saxs_Hplc_Svd::axis_y()
{
}

void US_Hydrodyn_Saxs_Hplc_Svd::hide_data()
{
   cout << QString( "hide data size %1\n" ).arg( data_widgets.size() );
   hide_widgets( data_widgets, data_widgets[ 0 ]->isVisible() );
      
   ush_win->gparams[ "hplc_svd_data_widgets" ] = data_widgets[ 0 ]->isVisible() ? "visible" : "hidden";
}

void US_Hydrodyn_Saxs_Hplc_Svd::hide_editor()
{
   cout << QString( "hide editor size %1\n" ).arg( editor_widgets.size() );
   hide_widgets( editor_widgets, editor_widgets[ 0 ]->isVisible() );
      
   ush_win->gparams[ "hplc_svd_editor_widgets" ] = editor_widgets[ 0 ]->isVisible() ? "visible" : "hidden";
}

void US_Hydrodyn_Saxs_Hplc_Svd::hide_process()
{
   cout << QString( "hide process size %1\n" ).arg( process_widgets.size() );

   hide_widgets( process_widgets, process_widgets[ 0 ]->isVisible() );
      
   ush_win->gparams[ "hplc_svd_process_widgets" ] = process_widgets[ 0 ]->isVisible() ? "visible" : "hidden";
}

void US_Hydrodyn_Saxs_Hplc_Svd::hide_widgets( vector < QWidget *> widgets, bool hide )
{
   for ( unsigned int i = 0; i < ( unsigned int ) widgets.size(); i++ )
   {
      hide ? widgets[ i ]->hide() : widgets[ i ]->show();
   }
}

void US_Hydrodyn_Saxs_Hplc_Svd::update_enables()
{
   // count selections
   if ( disable_updates )
   {
      return;
   }

   lv_data    ->setEnabled( true );
   pb_to_hplc ->setEnabled( false );
   pb_iq_it   ->setEnabled( false );
   pb_axis_x  ->setEnabled( false );
   pb_axis_y  ->setEnabled( false );
   le_q_start ->setEnabled( false );
   le_q_end   ->setEnabled( false );
   le_t_start ->setEnabled( false );
   le_t_end   ->setEnabled( false );
   le_ev_count->setEnabled( false );
   pb_svd     ->setEnabled( false );
   pb_recon   ->setEnabled( lb_ev->count() > 0 && le_ev_count->text().toInt() <= (int) lb_ev->count() );
}

void US_Hydrodyn_Saxs_Hplc_Svd::data_selection_changed()
{
}


void US_Hydrodyn_Saxs_Hplc_Svd::ev_selection_changed()
{
}

