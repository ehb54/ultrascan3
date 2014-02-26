#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_saxs_hplc_svd.h"
#include "../include/us_svd.h"

#ifdef QT4
#include <qwt_scale_engine.h>
#endif
#include <qpalette.h>
//Added by qt3to4:
#include <Q3BoxLayout>
#include <QLabel>
#include <QCloseEvent>
#include <Q3GridLayout>
#include <Q3TextStream>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <Q3Frame>
#include <Q3PopupMenu>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#define SLASH QDir::separator()

US_Hydrodyn_Saxs_Hplc_Svd::US_Hydrodyn_Saxs_Hplc_Svd(
                                                     US_Hydrodyn_Saxs_Hplc *hplc_win,
                                                     vector < QString > hplc_selected_files,
                                                     QWidget *p, 
                                                     const char *name
                                                     ) : Q3Frame(p, name)
{
   this->hplc_win                = hplc_win;
   this->hplc_selected_files     = hplc_selected_files;
   this->ush_win                 = (US_Hydrodyn *)(hplc_win->us_hydrodyn);
   this->plot_colors             = hplc_win->plot_colors;
   this->use_line_width          = hplc_win->use_line_width;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setCaption(tr("US-SOMO: SAXS HPLC SVD"));

   cg_red = USglobal->global_colors.cg_label;
   cg_red.setBrush( QColorGroup::Foreground, QBrush( QColor( "red" ),  Qt::SolidPattern ) );

   if ( !hplc_selected_files.size() )
   {
      QMessageBox::warning( this, 
                            caption(),
                            tr( "Internal error: HPLC SVD called with no curves selected" ) );
      close();
      return;
   }

   QStringList original_data;

   for ( int i = 0; i < (int) hplc_selected_files.size(); ++i )
   {
      QString this_name = hplc_selected_files[ i ];
      original_data.push_back( this_name );
      
      f_pos      [ this_name ] = f_pos.size();
      f_qs_string[ this_name ] = hplc_win->f_qs_string[ this_name ];
      f_qs       [ this_name ] = hplc_win->f_qs       [ this_name ];
      f_Is       [ this_name ] = hplc_win->f_Is       [ this_name ];
      f_errors   [ this_name ] = hplc_win->f_errors   [ this_name ];
      f_is_time  [ this_name ] = false;  // must all be I(q)
   }

   subset_data.insert( original_data.join( "\n" ) );

   plot_data_zoomer      = (ScrollZoomer *) 0;
   plot_errors_zoomer    = (ScrollZoomer *) 0;

   le_last_focus      = (mQLineEdit *) 0;
   
   running            = false;

   setupGUI();

   axis_y_log = false;
   axis_x_log = false;
   sv_plot    = false;
   rmsd_plot  = false;
   chi_plot   = false;

   last_axis_x_log = axis_x_log;
   last_axis_y_log = axis_y_log;

   update_enables();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry(global_Xpos, global_Ypos, 0, 0 );

   show();

   add_i_of_t( tr( "Original data" ), original_data );
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
   lbl_data->setPalette( PALET_LABEL );
   AUTFBACK( lbl_data );
   lbl_data->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   connect( lbl_data, SIGNAL( pressed() ), SLOT( hide_data() ) );

   lv_data = new Q3ListView( this );
   lv_data->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   lv_data->setPalette( PALET_EDIT );
   AUTFBACK( lv_data );
   lv_data->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lv_data->setEnabled(true);
   // lv_data->setMinimumWidth(  );

   lv_data->addColumn( "Source" );

   lv_data->setSorting        ( -1 );
   lv_data->setRootIsDecorated( true );
   lv_data->setSelectionMode  ( Q3ListView::Extended );
   connect(lv_data, SIGNAL(selectionChanged()), SLOT( data_selection_changed() ));

   Q3ListViewItem *element = new Q3ListViewItem( lv_data, QString( tr( "Original data" ) ) );
   Q3ListViewItem *iq = new Q3ListViewItem( element, "I(q)" );
   /* QListViewItem *it = */ new Q3ListViewItem( element, iq, "I(t)" );

   Q3ListViewItem *lvi = iq;

   for ( int i = 0; i < (int) hplc_selected_files.size(); ++i )
   {
      lvi = new Q3ListViewItem( iq, lvi, hplc_selected_files[ i ] );
   }

   // make i(t), add also

   data_widgets.push_back( lv_data );
   
   pb_clear = new QPushButton(tr("Clear"), this);
   pb_clear->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_clear->setMinimumHeight(minHeight3);
   pb_clear->setPalette( PALET_PUSHB );
   connect(pb_clear, SIGNAL(clicked()), SLOT(clear()));
   data_widgets.push_back( pb_clear );

   pb_to_hplc = new QPushButton(tr("To HPLC window"), this);
   pb_to_hplc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_to_hplc->setMinimumHeight(minHeight3);
   pb_to_hplc->setPalette( PALET_PUSHB );
   connect(pb_to_hplc, SIGNAL(clicked()), SLOT(to_hplc()));
   data_widgets.push_back( pb_to_hplc );

   pb_color_rotate = new QPushButton(tr("Color"), this);
   pb_color_rotate->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_color_rotate->setMinimumHeight(minHeight3);
   pb_color_rotate->setPalette( PALET_PUSHB );
   connect(pb_color_rotate, SIGNAL(clicked()), SLOT(color_rotate()));
   data_widgets.push_back( pb_color_rotate );

   pb_replot = new QPushButton(tr("Replot"), this);
   pb_replot->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_replot->setMinimumHeight(minHeight3);
   pb_replot->setPalette( PALET_PUSHB );
   connect(pb_replot, SIGNAL(clicked()), SLOT(replot()));
   data_widgets.push_back( pb_replot );

   // ------ editor section 

   lbl_editor = new mQLabel("Messages", this);
   lbl_editor->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_editor->setMinimumHeight(minHeight1);
   lbl_editor->setPalette( PALET_LABEL );
   AUTFBACK( lbl_editor );
   lbl_editor->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   connect( lbl_editor, SIGNAL( pressed() ), SLOT( hide_editor() ) );

   editor = new Q3TextEdit(this);
   editor->setPalette( PALET_NORMAL );
   AUTFBACK( editor );
   editor->setReadOnly(true);
   editor->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ));

   Q3Frame *frame;
   frame = new Q3Frame(this);
   frame->setMinimumHeight(minHeight3);
   editor_widgets.push_back( frame );

   mb_editor = new QMenuBar(frame, "menu" );
   mb_editor->setMinimumHeight(minHeight1 - 5);
   mb_editor->setPalette( PALET_NORMAL );
   AUTFBACK( mb_editor );

   Q3PopupMenu * file = new Q3PopupMenu(editor);
   mb_editor->insertItem( tr("&File"), file );
   file->insertItem( tr("Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
   file->insertItem( tr("Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );
   file->insertItem( tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );
   editor->setWordWrap (Q3TextEdit::WidgetWidth);
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
   grid_data = new QwtPlotGrid;
   grid_data->enableXMin( true );
   grid_data->enableYMin( true );
#endif
   plot_data->setPalette( PALET_NORMAL );
   AUTFBACK( plot_data );
#ifndef QT4
   plot_data->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot_data->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   grid_data->setMajPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid_data->setMinPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid_data->attach( plot_data );
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
   plot_data->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::None);
#else
   plot_data->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine );
#endif
   plot_data->setCanvasBackground(USglobal->global_colors.plot);

   // connect( plot_data_zoomer, SIGNAL( zoomed( const QwtDoubleRect & ) ), SLOT( plot_data_zoomed( const QwtDoubleRect & ) ) );

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
   // plot_errors->setAxisTitle(QwtPlot::xBottom, /* cb_guinier->isChecked() ? tr("q^2 (1/Angstrom^2)") : */  tr("q [1/Angstrom]" )); // or Time or Frame"));
   plot_errors->setAxisTitle(QwtPlot::yLeft, tr("Delta Intensity [a.u.]"));
#ifndef QT4
   plot_errors->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   plot_errors->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_errors->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#ifndef QT4
   plot_errors->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_errors->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#ifndef QT4
   plot_errors->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_errors->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_errors->setMargin(USglobal->config_list.margin);
   plot_errors->setTitle("");
#ifndef QT4
   plot_errors->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::None);
#else
   plot_errors->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine );
#endif
   plot_errors->setCanvasBackground(USglobal->global_colors.plot);

   // connect( plot_errors_zoomer, SIGNAL( zoomed( const QwtDoubleRect & ) ), SLOT( plot_errors_zoomed( const QwtDoubleRect & ) ) );
   errors_widgets.push_back( plot_errors );

   cb_plot_errors = new QCheckBox(this);
   cb_plot_errors->setText(tr("Residuals "));
   cb_plot_errors->setEnabled( true );
   cb_plot_errors->setChecked( false );
   cb_plot_errors->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_plot_errors->setPalette( PALET_NORMAL );
   AUTFBACK( cb_plot_errors );
   connect( cb_plot_errors, SIGNAL( clicked() ), SLOT( set_plot_errors() ) );
   // errors_widgets.push_back( cb_plot_errors );

   cb_plot_errors_rev = new QCheckBox(this);
   cb_plot_errors_rev->setText(tr("Reverse "));
   cb_plot_errors_rev->setEnabled( true );
   cb_plot_errors_rev->setChecked( false );
   cb_plot_errors_rev->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_plot_errors_rev->setPalette( PALET_NORMAL );
   AUTFBACK( cb_plot_errors_rev );
   connect( cb_plot_errors_rev, SIGNAL( clicked() ), SLOT( set_plot_errors_rev() ) );
   errors_widgets.push_back( cb_plot_errors_rev );

   cb_plot_errors_sd = new QCheckBox(this);
   cb_plot_errors_sd->setText(tr("Use SDs "));
   cb_plot_errors_sd->setEnabled( true );
   cb_plot_errors_sd->setChecked( false );
   cb_plot_errors_sd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_plot_errors_sd->setPalette( PALET_NORMAL );
   AUTFBACK( cb_plot_errors_sd );
   connect( cb_plot_errors_sd, SIGNAL( clicked() ), SLOT( set_plot_errors_sd() ) );
   errors_widgets.push_back( cb_plot_errors_sd );

   cb_plot_errors_pct = new QCheckBox(this);
   cb_plot_errors_pct->setText(tr("By percent "));
   cb_plot_errors_pct->setEnabled( true );
   cb_plot_errors_pct->setChecked( false );
   cb_plot_errors_pct->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_plot_errors_pct->setPalette( PALET_NORMAL );
   AUTFBACK( cb_plot_errors_pct );
   connect( cb_plot_errors_pct, SIGNAL( clicked() ), SLOT( set_plot_errors_pct() ) );
   errors_widgets.push_back( cb_plot_errors_pct );

   cb_plot_errors_ref = new QCheckBox(this);
   cb_plot_errors_ref->setText(tr("Reference "));
   cb_plot_errors_ref->setEnabled( true );
   cb_plot_errors_ref->setChecked( false );
   cb_plot_errors_ref->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_plot_errors_ref->setPalette( PALET_NORMAL );
   AUTFBACK( cb_plot_errors_ref );
   connect( cb_plot_errors_ref, SIGNAL( clicked() ), SLOT( set_plot_errors_ref() ) );
   errors_widgets.push_back( cb_plot_errors_ref );

   cb_plot_errors_group = new QCheckBox(this);
   cb_plot_errors_group->setText(tr("Group"));
   cb_plot_errors_group->setEnabled( true );
   cb_plot_errors_group->setChecked( false );
   cb_plot_errors_group->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_plot_errors_group->setPalette( PALET_NORMAL );
   AUTFBACK( cb_plot_errors_group );
   connect( cb_plot_errors_group, SIGNAL( clicked() ), SLOT( set_plot_errors_group() ) );
   errors_widgets.push_back( cb_plot_errors_group );

   iq_it_state = false;
   pb_iq_it = new QPushButton( tr("Show I(t)"), this);
   pb_iq_it->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_iq_it->setMinimumHeight(minHeight1);
   pb_iq_it->setPalette( PALET_PUSHB );
   connect(pb_iq_it, SIGNAL(clicked()), SLOT(iq_it()));

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


   // ------ process section 

   lbl_process = new mQLabel("Process", this);
   lbl_process->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_process->setMinimumHeight(minHeight1);
   lbl_process->setPalette( PALET_LABEL );
   AUTFBACK( lbl_process );
   lbl_process->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   connect( lbl_process, SIGNAL( pressed() ), SLOT( hide_process() ) );

   lbl_q_range = new QLabel( tr( "Active q range:" ), this );
   lbl_q_range->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_q_range->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_q_range );
   lbl_q_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   // process_widgets.push_back( lbl_q_range );
   lbl_q_range->hide();

   le_q_start = new mQLineEdit(this, "le_q_start Line Edit");
   le_q_start->setText( "" );
   le_q_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_q_start->setPalette( PALET_NORMAL );
   AUTFBACK( le_q_start );
   le_q_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_q_start->setEnabled( false );
   le_q_start->setValidator( new QDoubleValidator( le_q_start ) );
   connect( le_q_start, SIGNAL( textChanged( const QString & ) ), SLOT( q_start_text( const QString & ) ) );
   // process_widgets.push_back( le_q_start );
   le_q_start->hide();

   le_q_end = new mQLineEdit(this, "le_q_end Line Edit");
   le_q_end->setText( "" );
   le_q_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_q_end->setPalette( PALET_NORMAL );
   AUTFBACK( le_q_end );
   le_q_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_q_end->setEnabled( false );
   le_q_end->setValidator( new QDoubleValidator( le_q_end ) );
   connect( le_q_end, SIGNAL( textChanged( const QString & ) ), SLOT( q_end_text( const QString & ) ) );
   // process_widgets.push_back( le_q_end );
   le_q_end->hide();

   //    lbl_t_range = new QLabel( tr( "Active Time range:" ), this );
   //    lbl_t_range->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   //    lbl_t_range->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   //    lbl_t_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   //    process_widgets.push_back( lbl_t_range );

   //    le_t_start = new mQLineEdit(this, "le_t_start Line Edit");
   //    le_t_start->setText( "" );
   //    le_t_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   //    le_t_start->setPalette(QPalette( USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   //    le_t_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   //    le_t_start->setEnabled( false );
   //    le_t_start->setValidator( new QDoubleValidator( le_t_start ) );
   //    connect( le_t_start, SIGNAL( textChanged( const QString & ) ), SLOT( t_start_text( const QString & ) ) );
   //    process_widgets.push_back( le_t_start );

   //    le_t_end = new mQLineEdit(this, "le_t_end Line Edit");
   //    le_t_end->setText( "" );
   //    le_t_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   //    le_t_end->setPalette(QPalette( USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   //    le_t_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   //    le_t_end->setEnabled( false );
   //    le_t_end->setValidator( new QDoubleValidator( le_t_end ) );
   //    connect( le_t_end, SIGNAL( textChanged( const QString & ) ), SLOT( t_end_text( const QString & ) ) );
   //    process_widgets.push_back( le_t_end );

   lbl_ev = new QLabel( tr( "Singular value list:" ), this );
   lbl_ev->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_ev->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_ev );
   lbl_ev->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   process_widgets.push_back( lbl_ev );

   lb_ev = new Q3ListBox(this, "files files listbox" );
   lb_ev->setPalette( PALET_NORMAL );
   AUTFBACK( lb_ev );
   lb_ev->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lb_ev->setEnabled(true);
   lb_ev->setSelectionMode( Q3ListBox::Extended );
   lb_ev->setColumnMode   ( Q3ListBox::FitToWidth );
   connect( lb_ev, SIGNAL( selectionChanged() ), SLOT( sv_selection_changed() ) );
   process_widgets.push_back( lb_ev );

   pb_svd = new QPushButton(tr("Compute SVD"), this);
   pb_svd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_svd->setMinimumHeight(minHeight3);
   pb_svd->setPalette( PALET_PUSHB );
   connect(pb_svd, SIGNAL(clicked()), SLOT(svd()));
   process_widgets.push_back( pb_svd );

   pb_stop = new QPushButton(tr("Stop"), this);
   pb_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stop->setMinimumHeight(minHeight3);
   pb_stop->setPalette( PALET_PUSHB );
   connect(pb_stop, SIGNAL(clicked()), SLOT(stop()));
   process_widgets.push_back( pb_stop );

   pb_svd_plot = new QPushButton(tr("Plot SVs"), this);
   pb_svd_plot->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_svd_plot->setMinimumHeight(minHeight3);
   pb_svd_plot->setPalette( PALET_PUSHB );
   connect(pb_svd_plot, SIGNAL(clicked()), SLOT(svd_plot()));
   process_widgets.push_back( pb_svd_plot );

   pb_svd_save = new QPushButton(tr("Save SVs"), this);
   pb_svd_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_svd_save->setMinimumHeight(minHeight3);
   pb_svd_save->setPalette( PALET_PUSHB );
   connect(pb_svd_save, SIGNAL(clicked()), SLOT(svd_save()));
   process_widgets.push_back( pb_svd_save );

   pb_recon = new QPushButton(tr("TSVD reconstruction"), this);
   pb_recon->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_recon->setMinimumHeight(minHeight3);
   pb_recon->setPalette( PALET_PUSHB );
   connect(pb_recon, SIGNAL(clicked()), SLOT(recon()));
   process_widgets.push_back( pb_recon );

   pb_indiv_recon = new QPushButton(tr("Individual TVSD recon."), this);
   pb_indiv_recon->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_indiv_recon->setMinimumHeight(minHeight3);
   pb_indiv_recon->setPalette( PALET_PUSHB );
   connect(pb_indiv_recon, SIGNAL(clicked()), SLOT(indiv_recon()));
   process_widgets.push_back( pb_indiv_recon );

   pb_inc_recon = new QPushButton(tr("Incremental TVSD recon."), this);
   pb_inc_recon->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_inc_recon->setMinimumHeight(minHeight3);
   pb_inc_recon->setPalette( PALET_PUSHB );
   connect(pb_inc_recon, SIGNAL(clicked()), SLOT(inc_recon()));
   process_widgets.push_back( pb_inc_recon );

   pb_inc_chi_plot = new QPushButton(tr("Plot Chi"), this);
   pb_inc_chi_plot->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_inc_chi_plot->setMinimumHeight(minHeight3);
   pb_inc_chi_plot->setPalette( PALET_PUSHB );
   connect(pb_inc_chi_plot, SIGNAL(clicked()), SLOT(inc_chi_plot()));
   // process_widgets.push_back( pb_inc_chi_plot );
   pb_inc_chi_plot->hide();

   pb_inc_rmsd_plot = new QPushButton(tr("Plot RMSDs"), this);
   pb_inc_rmsd_plot->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_inc_rmsd_plot->setMinimumHeight(minHeight3);
   pb_inc_rmsd_plot->setPalette( PALET_PUSHB );
   connect(pb_inc_rmsd_plot, SIGNAL(clicked()), SLOT(inc_rmsd_plot()));
   process_widgets.push_back( pb_inc_rmsd_plot );

   pb_rmsd_save = new QPushButton(tr("Save RMSDs"), this);
   pb_rmsd_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_rmsd_save->setMinimumHeight(minHeight3);
   pb_rmsd_save->setPalette( PALET_PUSHB );
   connect(pb_rmsd_save, SIGNAL(clicked()), SLOT(rmsd_save()));
   process_widgets.push_back( pb_rmsd_save );

   // -------- bottom section

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   progress = new Q3ProgressBar(this, "Progress");
   // progress->setMinimumHeight(minHeight1);
   progress->setPalette( PALET_NORMAL );
   AUTFBACK( progress );
   progress->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   progress->reset();

   pb_cancel = new QPushButton(tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // -------- build layout


   Q3VBoxLayout *background = new Q3VBoxLayout(this);
   Q3HBoxLayout *top        = new Q3HBoxLayout( 0 );

   // ----- left side
   {
      Q3BoxLayout *bl = new Q3VBoxLayout( 0 );
      bl->addWidget( lbl_data );
      bl->addWidget( lv_data );

      {
         Q3BoxLayout *bl_buttons = new Q3HBoxLayout( 0 );
         bl_buttons->addWidget( pb_clear );
         bl_buttons->addWidget( pb_replot );
         bl->addLayout( bl_buttons );
      }
      {
         Q3BoxLayout *bl_buttons = new Q3HBoxLayout( 0 );
         bl_buttons->addWidget( pb_to_hplc );
         bl_buttons->addWidget( pb_color_rotate );
         bl->addLayout( bl_buttons );
      }

      bl->addWidget( lbl_process );
      {
         Q3GridLayout *gl = new Q3GridLayout( 0 );
         
         gl->addWidget( lbl_q_range  , 0, 0 );
         gl->addWidget( le_q_start   , 0, 1 );
         gl->addWidget( le_q_end     , 0, 2 );

         // gl->addWidget( lbl_t_range  , 1, 0 );
         // gl->addWidget( le_t_start   , 1, 1 );
         // gl->addWidget( le_t_end     , 1, 2 );

         // gl->addWidget( lbl_ev       , 3, 0 );
         // gl->addMultiCellWidget( lb_ev        , 3, 3, 1, 2 );

         bl->addLayout( gl );
      }

      {
         Q3BoxLayout *bl_buttons = new Q3HBoxLayout( 0 );
         bl_buttons->addWidget( pb_svd );
         bl_buttons->addWidget( pb_stop );
         bl->addLayout( bl_buttons );
      }

      bl->addWidget( lbl_ev );
      bl->addWidget( lb_ev );

      {
         Q3BoxLayout *bl_buttons = new Q3HBoxLayout( 0 );
         bl_buttons->addWidget( pb_svd_plot );
         bl_buttons->addWidget( pb_svd_save );
         bl_buttons->addWidget( pb_recon );
         bl->addLayout( bl_buttons );
      }

      {
         Q3BoxLayout *bl_buttons = new Q3HBoxLayout( 0 );
         bl_buttons->addWidget( pb_indiv_recon );
         bl_buttons->addWidget( pb_inc_recon );
         bl->addLayout( bl_buttons );
      }

      {
         Q3BoxLayout *bl_buttons = new Q3HBoxLayout( 0 );
         bl_buttons->addWidget( pb_inc_chi_plot );
         bl_buttons->addWidget( pb_inc_rmsd_plot );
         bl_buttons->addWidget( pb_rmsd_save );
         bl->addLayout( bl_buttons );
      }

      bl->addWidget( lbl_editor );
      bl->addWidget( frame );
      bl->addWidget( editor );

      top->addLayout ( bl );
   }      

   // ----- right side
   {
      Q3BoxLayout *bl = new Q3VBoxLayout( 0 );
      bl->addWidget( plot_data );
      bl->addWidget( plot_errors );

      //       {
      //          QGridLayout * gl = new QGridLayout( 0 );
      //          gl -> addWidget( cb_plot_errors       , 0, 0 );
      //          gl -> addWidget( cb_plot_errors_sd    , 0, 1 );
      //          gl -> addWidget( cb_plot_errors_pct   , 0, 2 );
      //          gl -> addWidget( cb_plot_errors_rev   , 1, 0 );
      //          gl -> addWidget( cb_plot_errors_ref   , 1, 1 );
      //          gl -> addWidget( cb_plot_errors_group , 1, 2 );

      //          bl -> addLayout( gl );
      //       }

      {
         Q3BoxLayout *bl_buttons = new Q3HBoxLayout( 0 );
         bl_buttons->addWidget( cb_plot_errors );
         bl_buttons->addWidget( cb_plot_errors_sd );
         bl_buttons->addWidget( cb_plot_errors_pct );
         bl->addLayout( bl_buttons );
      }

      {
         Q3BoxLayout *bl_buttons = new Q3HBoxLayout( 0 );
         bl_buttons->addWidget( cb_plot_errors_rev );
         bl_buttons->addWidget( cb_plot_errors_ref );
         bl_buttons->addWidget( cb_plot_errors_group );
         bl->addLayout( bl_buttons );
      }

      {
         Q3BoxLayout *bl_buttons = new Q3HBoxLayout( 0 );
         bl_buttons->addWidget( pb_iq_it );
         bl_buttons->addWidget( pb_axis_x );
         bl_buttons->addWidget( pb_axis_y );
         bl->addLayout( bl_buttons );
      }

      top->addLayout( bl );
   }
   
   background->addLayout( top );

   {
      Q3BoxLayout *bottom = new Q3HBoxLayout( 0 );
      bottom->addWidget( pb_help );
      bottom->addWidget( progress );
      bottom->addWidget( pb_cancel );
      
      background->addSpacing( 2 );
      background->addLayout( bottom );
   }
   
   hide_widgets( data_widgets, 
                 !ush_win->gparams.count( "hplc_svd_data_widgets" ) || ush_win->gparams[ "hplc_svd_data_widgets" ] == "false" ? false : true );
   hide_widgets( editor_widgets, 
                 !ush_win->gparams.count( "hplc_svd_editor_widgets" ) || ush_win->gparams[ "hplc_svd_editor_widgets" ] == "false" ? false : true );
   hide_widgets( process_widgets,
                 !ush_win->gparams.count( "hplc_svd_process_widgets" ) || ush_win->gparams[ "hplc_svd_process_widgets" ] == "false" ? false : true );
   hide_widgets( errors_widgets, true );
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
   fn = Q3FileDialog::getSaveFileName(QString::null, QString::null,this );
   if(!fn.isEmpty() )
   {
      QString text = editor->text();
      QFile f( fn );
      if ( !f.open( QIODevice::WriteOnly | QIODevice::Text) )
      {
         return;
      }
      Q3TextStream t( &f );
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

   if ( !editor_widgets[ 0 ]->isVisible() && color == "red" && !msg.stripWhiteSpace().isEmpty() )
   {
      lbl_editor->setPalette(QPalette(cg_red, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   }
}

void US_Hydrodyn_Saxs_Hplc_Svd::plot_files()
{
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

   QStringList files = selected_files();

   for ( int i = 0; i < (int) files.size(); ++i )
   {
      if ( plot_file( files[ i ], file_minx, file_maxx, file_miny, file_maxy ) )
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

   // cout << QString( "org plot range x [%1:%2] y [%3:%4]\n" ).arg(minx).arg(maxx).arg(miny).arg(maxy);

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
      // connect( plot_data_zoomer, SIGNAL( zoomed( const QwtDoubleRect & ) ), SLOT( plot_data_zoomed( const QwtDoubleRect & ) ) );
   }
   
   // plot_data->replot();
   rescale( false );
   update_plot_errors();
}

void US_Hydrodyn_Saxs_Hplc_Svd::plot_files( QStringList add_files )
{
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

   QStringList files = selected_files();

   for ( int i = 0; i < (int) files.size(); ++i )
   {
      if ( plot_file( files[ i ], file_minx, file_maxx, file_miny, file_maxy ) )
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

   for ( int i = 0; i < (int) add_files.size(); ++i )
   {
      if ( plot_file( add_files[ i ], file_minx, file_maxx, file_miny, file_maxy ) )
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

   // cout << QString( "org plot range x [%1:%2] y [%3:%4]\n" ).arg(minx).arg(maxx).arg(miny).arg(maxy);

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
      // connect( plot_data_zoomer, SIGNAL( zoomed( const QwtDoubleRect & ) ), SLOT( plot_data_zoomed( const QwtDoubleRect & ) ) );
   }
   
   // plot_data->replot();
   rescale();
   // update_plot_errors();
}

bool US_Hydrodyn_Saxs_Hplc_Svd::plot_file( QString file,
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
      plot_data->setCurvePen( Iq, QPen( plot_colors[ f_pos[ file ] % plot_colors.size() ], use_line_width, SolidLine));
#else
      curve->setData(
                     (double *)&( f_qs[ file ][ 0 ] ),
                     (double *)&( f_Is[ file ][ 0 ] ),
                     q_points
                     );

      curve->setPen( QPen( plot_colors[ f_pos[ file ] % plot_colors.size() ], use_line_width, Qt::SolidLine ) );
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
      plot_data->setCurvePen( Iq, QPen( plot_colors[ f_pos[ file ] % plot_colors.size()], use_line_width, SolidLine));
#else
      curve->setData(
                     /* cb_guinier->isChecked() ?
                        (double *)&(plotted_q2[p][0]) : */
                     (double *)&( q[ 0 ] ),
                     (double *)&( I[ 0 ] ),
                     q_points
                     );

      curve->setPen( QPen( plot_colors[ f_pos[ file ] % plot_colors.size() ], use_line_width, Qt::SolidLine ) );
      curve->attach( plot_data );
#endif
   }
   return true;
}

bool US_Hydrodyn_Saxs_Hplc_Svd::get_min_max( QString file,
                                             double &minx,
                                             double &maxx,
                                             double &miny,
                                             double &maxy )
{
   if ( 
        !f_Is        .count( file ) ||
        !f_pos       .count( file ) )
   {
      // editor_msg( "red", QString( tr( "Internal error: requested %1, but not found in data" ) ).arg( file ) );
      return false;
   }

   minx = f_qs[ file ][ 0 ];
   maxx = f_qs[ file ].back();

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
      if ( miny <= 0e0 )
      {
         miny = 1e0;
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

// void US_Hydrodyn_Saxs_Hplc_Svd::plot_data_zoomed( const QwtDoubleRect & /* rect */ )
// {
//    //   cout << QString( "zoomed: %1 %2 %3 %4\n" )
//    // .arg( rect.x1() )
//    // .arg( rect.x2() )
//    // .arg( rect.y1() )
//    // .arg( rect.y2() );
// }

// void US_Hydrodyn_Saxs_Hplc_Svd::plot_errors_zoomed( const QwtDoubleRect & /* rect */ )
// {
//    //   cout << QString( "zoomed: %1 %2 %3 %4\n" )
//    // .arg( rect.x1() )
//    // .arg( rect.x2() )
//    // .arg( rect.y1() )
//    // .arg( rect.y2() );
// }

void US_Hydrodyn_Saxs_Hplc_Svd::disable_all()
{
   lv_data            ->setEnabled( false );
   pb_clear           ->setEnabled( false );
   pb_to_hplc         ->setEnabled( false );
   pb_color_rotate    ->setEnabled( false );
   pb_replot          ->setEnabled( false );
   pb_iq_it           ->setEnabled( false );
   pb_axis_x          ->setEnabled( false );
   pb_axis_y          ->setEnabled( false );
   le_q_start         ->setEnabled( false );
   le_q_end           ->setEnabled( false );
   // le_t_start       ->setEnabled( false );
   // le_t_end         ->setEnabled( false );
   lb_ev              ->setEnabled( false );
   pb_svd             ->setEnabled( false );
   pb_stop            ->setEnabled( running );
   pb_svd_plot        ->setEnabled( false );
   pb_svd_save        ->setEnabled( false );
   pb_recon           ->setEnabled( false );
   pb_inc_rmsd_plot   ->setEnabled( false );
   pb_rmsd_save       ->setEnabled( false );
   pb_inc_chi_plot    ->setEnabled( false );
   pb_inc_recon       ->setEnabled( false );
   pb_indiv_recon     ->setEnabled( false );


   qApp               ->processEvents();
   
}

void US_Hydrodyn_Saxs_Hplc_Svd::q_start_text( const QString & )
{
}

void US_Hydrodyn_Saxs_Hplc_Svd::q_end_text( const QString & )
{
}

// void US_Hydrodyn_Saxs_Hplc_Svd::t_start_text( const QString & )
// {
// }

// void US_Hydrodyn_Saxs_Hplc_Svd::t_end_text( const QString & )
// {
// }

void US_Hydrodyn_Saxs_Hplc_Svd::replot()
{
   disable_all();

   if ( plot_data_zoomer )
   {
      // cout << QString( "plot zoomer stack size %1\n" ).arg( plot_data_zoomer->zoomRectIndex() );
      if ( !plot_data_zoomer->zoomRectIndex() )
      {
         plot_data_zoomer->zoom ( 0 );
         delete plot_data_zoomer;
         plot_data_zoomer = (ScrollZoomer *) 0;
      }
   }

   if ( plot_errors_zoomer )
   {
      // cout << QString( "plot zoomer stack size %1\n" ).arg( plot_errors_zoomer->zoomRectIndex() );
      if ( !plot_errors_zoomer->zoomRectIndex() )
      {
         plot_errors_zoomer->zoom ( 0 );
         delete plot_errors_zoomer;
         plot_errors_zoomer = (ScrollZoomer *) 0;
      }
   }

   if ( sv_plot || chi_plot || rmsd_plot )
   {
      axis_x_log = last_axis_x_log;
      axis_y_log = last_axis_y_log;
      sv_plot   = false;
      rmsd_plot = false;
      chi_plot  = false;
      axis_x_title();
      axis_y_title();
      if ( plot_data_zoomer )
      {
         // cout << QString( "plot zoomer stack size %1\n" ).arg( plot_data_zoomer->zoomRectIndex() );
         if ( !plot_data_zoomer->zoomRectIndex() )
         {
            plot_data_zoomer->zoom ( 0 );
            delete plot_data_zoomer;
            plot_data_zoomer = (ScrollZoomer *) 0;
         }
      }
   }

   plot_files();
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc_Svd::iq_it()
{
   disable_all();

   if ( plot_data_zoomer )
   {
      plot_data_zoomer->zoom ( 0 );
      delete plot_data_zoomer;
      plot_data_zoomer = (ScrollZoomer *) 0;
   }

   if ( plot_errors_zoomer )
   {
      plot_errors_zoomer->zoom ( 0 );
      delete plot_errors_zoomer;
      plot_errors_zoomer = (ScrollZoomer *) 0;
   }

   if ( sv_plot || chi_plot || rmsd_plot )
   {
      axis_x_log = last_axis_x_log;
      axis_y_log = last_axis_y_log;
   }

   sv_plot   = false;
   rmsd_plot = false;
   chi_plot  = false;
   iq_it_state = !iq_it_state;
   pb_iq_it->setText( tr( iq_it_state ? "Show I(q)" : "Show I(t)" ) );
   axis_x_title();
   axis_y_title();
   
   replot();
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc_Svd::to_hplc()
{
   if ( !ush_win->saxs_hplc_widget )
   {
      editor_msg( "red", tr( "US-SOMO SAXS HPLC window has been closed" ) );
      return;
   }

   QStringList files = selected_files();

   disable_all();
   for ( int i = 0; i < (int) files.size(); ++i )
   {
      hplc_win->add_plot( files[ i ],
                          f_qs[ files[ i ] ],
                          f_Is[ files[ i ] ],
                          f_errors[ files[ i ] ],
                          iq_it_state,
                          false );
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc_Svd::axis_x()
{
   axis_x_log = !axis_x_log;

   axis_x_title();

   plot_data->replot();
}

void US_Hydrodyn_Saxs_Hplc_Svd::axis_y()
{
   axis_y_log = !axis_y_log;

   axis_y_title();

   if ( plot_data_zoomer )
   {
      plot_data_zoomer->zoom ( 0 );
      delete plot_data_zoomer;
      plot_data_zoomer = (ScrollZoomer *) 0;
   }
   if ( sv_plot )
   {
      svd_plot( false );
   } else {
      if ( rmsd_plot )
      {
         inc_rmsd_plot( false );
      } else {
         if ( chi_plot )
         {
            inc_chi_plot( false );
         } else {
            replot();
         }
      }
   }

   plot_data->replot();
}

void US_Hydrodyn_Saxs_Hplc_Svd::hide_data()
{
   cout << QString( "hide data size %1\n" ).arg( data_widgets.size() );
   hide_widgets( data_widgets, data_widgets[ 0 ]->isVisible() );
      
   ush_win->gparams[ "hplc_svd_data_widgets" ] = data_widgets[ 0 ]->isVisible() ? "visible" : "hidden";
}

void US_Hydrodyn_Saxs_Hplc_Svd::hide_editor()
{
   hide_widgets( editor_widgets, editor_widgets[ 0 ]->isVisible() );
   if ( editor_widgets[ 0 ]->isVisible() )
   {
      cout << "resetting editor palette\n";
      lbl_editor->setPalette( PALET_LABEL );
      AUTFBACK( lbl_editor );
   }

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
   if ( running )
   {
      return;
   }

   int sv_items = 0;
   for ( int i = 0; i < (int) lb_ev->count(); ++i )
   {
      if ( lb_ev->isSelected( i ) )
      {
         sv_items++;
      }
   }

   QStringList files = selected_files();
   set < QString > sources = get_selected_sources();

   lv_data            ->setEnabled( true );
   pb_clear           ->setEnabled( true );
   pb_to_hplc         ->setEnabled( files.size() && !sources.count( tr( "Original data" ) ) && ush_win->saxs_hplc_widget );
   pb_color_rotate    ->setEnabled( true );
   pb_replot          ->setEnabled( !plotted_matches_selected() );
   pb_iq_it           ->setEnabled( true );
   pb_axis_x          ->setEnabled( true );
   pb_axis_y          ->setEnabled( true );
   le_q_start         ->setEnabled( false );
   le_q_end           ->setEnabled( false );
   // le_t_start     ->setEnabled( false );
   // le_t_end       ->setEnabled( false );
   lb_ev              ->setEnabled( lb_ev->count() );

   pb_svd             ->setEnabled( files.size() && !iq_it_state && sources.size() == 1 );
   pb_stop            ->setEnabled( false );
   pb_svd_plot        ->setEnabled( lb_ev->count() );
   pb_svd_save        ->setEnabled( lb_ev->count() );
   pb_recon           ->setEnabled( sv_items );

   pb_inc_rmsd_plot   ->setEnabled( rmsd_x.size() );
   pb_rmsd_save       ->setEnabled( rmsd_x.size() );
   pb_inc_chi_plot    ->setEnabled( chi_x.size() );
   pb_inc_recon       ->setEnabled( sv_items );
   pb_indiv_recon     ->setEnabled( sv_items );

   if ( sources.size() == 1 &&
        sources.begin()->contains( "reconstruction" ) )
   {
      cb_plot_errors->show();
      if ( cb_plot_errors->isChecked() &&
           !errors_widgets[ 0 ]->isVisible() )
      {
         hide_widgets( errors_widgets, false );
      }
   } else {
      cb_plot_errors->hide();
      if ( errors_widgets[ 0 ]->isVisible() )
      {
         hide_widgets( errors_widgets, true );
      }
   }
}

void US_Hydrodyn_Saxs_Hplc_Svd::data_selection_changed()
{
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc_Svd::sv_selection_changed()
{
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc_Svd::clear()
{
   lv_data->clearSelection();
   update_enables();
}

QStringList US_Hydrodyn_Saxs_Hplc_Svd::selected_files()
{
   QStringList result;

   QString iq_or_it = iq_it_state ? "I(t)" : "I(q)";

   Q3ListViewItemIterator it( lv_data );
   while ( it.current() ) 
   {
      Q3ListViewItem *item = it.current();
      if ( is_selected( item ) &&
           item->depth() == 2 &&
           item->parent()->text( 0 ) == iq_or_it )
      {
         result << item->text( 0 );
      }
      ++it;
   }
   return result;
}

set < QString > US_Hydrodyn_Saxs_Hplc_Svd::get_current_files()
{
   set < QString > result;

   Q3ListViewItemIterator it( lv_data );
   while ( it.current() ) 
   {
      Q3ListViewItem *item = it.current();
      if ( item->depth() == 2 && 
           ( item->parent()->text( 0 ) == "I(q)" ||
             item->parent()->text( 0 ) == "I(t)" ) 
           )
      {
         result.insert( item->text( 0 ) );
      }
      ++it;
   }
   return result;
}

set < QString > US_Hydrodyn_Saxs_Hplc_Svd::get_sources()
{
   set < QString > result;

   Q3ListViewItemIterator it( lv_data );
   while ( it.current() ) 
   {
      Q3ListViewItem *item = it.current();
      if ( item->depth() == 0 )
      {
         result.insert( item->text( 0 ) );
      }
      ++it;
   }
   return result;
}

set < QString > US_Hydrodyn_Saxs_Hplc_Svd::get_selected_sources()
{
   set < QString > result;
   QString iq_or_it = iq_it_state ? "I(t)" : "I(q)";

   // QStringList qsl;
   Q3ListViewItemIterator it( lv_data );
   while ( it.current() ) 
   {
      Q3ListViewItem *item = it.current();
      if ( is_selected( item ) &&
           item->depth() == 2 &&
           item->parent()->text( 0 ) == iq_or_it )
      {
         result.insert( item->parent()->parent()->text( 0 ) );
         // qsl << item->parent()->parent()->text( 0 );
      }
      ++it;
   }
   // cout << "GSS:" << qsl.join( "\n" ) << "\n:SSG\n";
   return result;
}

Q3ListViewItem * US_Hydrodyn_Saxs_Hplc_Svd::get_source_item( QString source )
{
   Q3ListViewItemIterator it( lv_data );
   while ( it.current() ) 
   {
      Q3ListViewItem *item = it.current();
      if ( item->depth() == 0 &&
           item->text( 0 ) == source )
      {
         return item;
      }
      ++it;
   }
   return (Q3ListViewItem *) 0;
}

int US_Hydrodyn_Saxs_Hplc_Svd::selected_sources()
{
   QString iq_or_it = iq_it_state ? "I(t)" : "I(q)";

   set < QString > sources;

   Q3ListViewItemIterator it( lv_data );
   while ( it.current() ) 
   {
      Q3ListViewItem *item = it.current();
      if ( is_selected( item ) &&
           item->depth() == 2 &&
           item->parent()->text( 0 ) == iq_or_it )
      {
         sources.insert( item->parent()->parent()->text( 0 ) );
      }
      ++it;
   }
   return (int) sources.size();
}

void US_Hydrodyn_Saxs_Hplc_Svd::clean_selected()
{
   // if a parent is selected, set selected on all children
   // & if all children are selected, set parents selected

   Q3ListViewItemIterator it( lv_data );
   while ( it.current() ) 
   {
      Q3ListViewItem *item = it.current();
      if ( !item->isSelected() )
      {
         if ( is_selected( item ) || all_children_selected( item ) )
         {
            item->setSelected( true );
         } 
      }
      ++it;
   }

   lv_data->triggerUpdate();
}

bool US_Hydrodyn_Saxs_Hplc_Svd::all_children_selected( Q3ListViewItem *lvi )
{
   if ( lvi->childCount() )
   {
      Q3ListViewItem *myChild = lvi->firstChild();
      while( myChild ) 
      {
         if ( myChild->childCount() )
         {
            if ( !all_children_selected( myChild ) )
            {
               return false;
            }
         } else {
            if ( !is_selected( myChild ) )
            {
               return false;
            }
         }
         myChild = myChild->nextSibling();
      }
   } else {
      return lvi->isSelected();
   }

   return true;
}

bool US_Hydrodyn_Saxs_Hplc_Svd::is_selected( Q3ListViewItem *lvi )
{
   if ( lvi->isSelected() )
   {
      return true;
   }
   while ( lvi->parent() )
   {
      lvi = lvi->parent();
      if ( lvi->isSelected() )
      {
         return true;
      } 
   }
   return false;
}

void US_Hydrodyn_Saxs_Hplc_Svd::axis_x_title()
{
   QString title = tr( iq_it_state ? "Time [a.u.]" : "q [1/Angstrom]" );
   if ( sv_plot || rmsd_plot || chi_plot )
   {
      title = tr( "Number" );
   }

   if ( axis_x_log )
   {
      plot_data->setAxisTitle(QwtPlot::xBottom,  title + tr(" (log scale)") );
#ifndef QT4
      plot_data->setAxisOptions(QwtPlot::xBottom, QwtAutoScale::Logarithmic);
#else
      plot_data->setAxisScaleEngine(QwtPlot::xBottom, new QwtLog10ScaleEngine);
#endif
   } else {
      plot_data->setAxisTitle(QwtPlot::xBottom,  title );
#ifndef QT4
      plot_data->setAxisOptions(QwtPlot::xBottom, QwtAutoScale::None);
#else
      // actually need to test this, not sure what the correct version is
      plot_data->setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine );
#endif
   }
}

void US_Hydrodyn_Saxs_Hplc_Svd::axis_y_title()
{
   QString title = tr( iq_it_state ? "I(t) [a.u.]" : "I(q) [a.u.]" );
   if ( sv_plot )
   {
      title = tr( "Singular values" );
   }

   if ( rmsd_plot )
   {
      title = "RMSD";
   }

   if ( chi_plot )
   {
      title = "Chi";
   }

   if ( axis_y_log )
   {
      plot_data->setAxisTitle(QwtPlot::yLeft, title + tr( " (log scale)") );
#ifndef QT4
      plot_data->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::Logarithmic);
#else
      plot_data->setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);
#endif
   } else {
      plot_data->setAxisTitle(QwtPlot::yLeft, title );
#ifndef QT4
      plot_data->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::None);
#else
      // actually need to test this, not sure what the correct version is
      plot_data->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine );
#endif
   }
}

bool US_Hydrodyn_Saxs_Hplc_Svd::plotted_matches_selected()
{
   set < QString > plotted; 
   set < QString > selected;

   QStringList files = selected_files();
   for ( int i = 0; i < (int) files.size(); ++i )
   {
      selected.insert( files[ i ] );
   }

   for ( 
#ifdef QT4
        map < QString, QwtPlotCurve * >::iterator it = plotted_curves.begin();
#else
        map < QString, long >          ::iterator it = plotted_curves.begin();
#endif
        it != plotted_curves.end();
        ++it )
   {
      plotted.insert( it->first );
   }

   if ( cb_plot_errors->isChecked() &&
        cb_plot_errors_ref->isChecked() &&
        plot_errors->isVisible() )
   {
      for ( set < QString >::iterator it = plot_errors_reference.begin();
            it != plot_errors_reference.end();
            ++it )
      {
         selected.insert( *it );
      }
   }

   return plotted == selected;
}

void US_Hydrodyn_Saxs_Hplc_Svd::add_i_of_t( QString source, QStringList files, bool do_update_enables )
{
   editor_msg( "blue", QString( tr(  "Making I(t) for source %1" ) ).arg( source ) );

   Q3ListViewItem * source_item = get_source_item( source );

   if ( !source_item )
   {
      editor_msg( "red", QString( tr( "Internal error: source item not defined %1" ) ).arg( source ) );
      return;
   }

   Q3ListViewItem * i_t_child = (Q3ListViewItem *) 0;
   Q3ListViewItem * i_q_child = (Q3ListViewItem *) 0;
      
   disable_all();

   if ( source_item->childCount() )
   {
      Q3ListViewItem * myChild = source_item->firstChild();
      while ( myChild )
      {
         if ( myChild->text( 0 ) == "I(t)" )
         {
            i_t_child = myChild;
         }
         if ( myChild->text( 0 ) == "I(q)" )
         {
            i_q_child = myChild;
         }
         if ( i_q_child &&
              i_t_child )
         {
            break;
         }
         myChild = myChild->nextSibling();
      }
   }

   if ( !i_t_child )
   {
      if ( i_q_child )
      {
         i_t_child = new Q3ListViewItem( source_item, i_q_child,  "I(t)" );
      } else {
         i_t_child = new Q3ListViewItem( source_item, source_item,  "I(t)" );
      }
   }

   // find common q 
   QString head = hplc_win->qstring_common_head( files, true );
   QString tail = hplc_win->qstring_common_tail( files, true );

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

   set < QString > current_files = get_current_files();

   Q3ListViewItem * lvi = i_t_child;

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
      // editor_msg( "gray", fname );
      current_files.insert( fname );

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

      lvi = new Q3ListViewItem( i_t_child, lvi, fname );
   
      f_pos       [ fname ] = f_qs.size();
      f_qs_string [ fname ] = t_qs;
      f_qs        [ fname ] = t;
      f_Is        [ fname ] = I;
      f_errors    [ fname ] = e;
      f_is_time   [ fname ] = true;
   }      
   if ( do_update_enables )
   {
      update_enables();
   }
   editor_msg( "blue", QString( tr(  "Done making I(t) for source %1" ) ).arg( source ) );
}

void US_Hydrodyn_Saxs_Hplc_Svd::rescale( bool do_update_enables )
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
   QStringList files = selected_files();

   for ( int i = 0; i < (int) files.size(); ++i )
   {
      if ( 1 )
      {
         //any_selected = true;
         if ( get_min_max( files[ i ], file_minx, file_maxx, file_miny, file_maxy ) )
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
   
   //    if ( plot_data_zoomer )
   //    {
   //       plot_data_zoomer->zoom ( 0 );
   //       delete plot_data_zoomer;
   //    }

   // cout << QString( "rescale plot range x [%1:%2] y [%3:%4]\n" ).arg(minx).arg(maxx).arg(miny).arg(maxy);

   if ( !plot_data_zoomer )
   {
      plot_data->setAxisScale( QwtPlot::xBottom, minx, maxx );
      plot_data->setAxisScale( QwtPlot::yLeft  , miny * 0.9e0 , maxy * 1.1e0 );
      plot_data_zoomer = new ScrollZoomer(plot_data->canvas());
      plot_data_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
#ifndef QT4
      plot_data_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
      // connect( plot_data_zoomer, SIGNAL( zoomed( const QwtDoubleRect & ) ), SLOT( plot_data_zoomed( const QwtDoubleRect & ) ) );
   }
   
   plot_data->replot();
   if ( do_update_enables )
   {
      update_enables();
   }
}

class svd_sortable_double {
public:
   double       x;
   int          index;
   bool operator < (const svd_sortable_double& objIn) const
   {
      return x < objIn.x;
   }
};

void US_Hydrodyn_Saxs_Hplc_Svd::svd_plot( bool axis_change )
{
   plot_data->clear();
   plotted_curves.clear();
   if ( plot_data_zoomer )
   {
      plot_data_zoomer->zoom ( 0 );
      delete plot_data_zoomer;
      plot_data_zoomer = (ScrollZoomer *) 0;
   }

   sv_plot   = true;
   rmsd_plot = false;
   chi_plot  = false;

   if ( axis_change )
   {
      last_axis_x_log = axis_x_log;
      last_axis_y_log = axis_y_log;

      axis_x_log = true;
      axis_y_log = false;

      axis_x_title();
      axis_y_title();
   }


#ifndef QT4
   long Iq = plot_data->insertCurve( "svd" );
   plotted_curves[ "svd" ] = Iq;
   plot_data->setCurveStyle( Iq, QwtCurve::Lines );
#else
   QwtPlotCurve *curve = new QwtPlotCurve( "svd" );
   plotted_curves[ "svd" ] = curve;
   curve->setStyle( QwtPlotCurve::Lines );
#endif

#ifndef QT4
   plot_data->setCurveData( Iq, 
                            (double *)&( svd_x[ 0 ] ),
                            (double *)&( svd_y[ 0 ] ),
                            svd_x.size()
                            );
   plot_data->setCurvePen( Iq, QPen( plot_colors[ 0 ], use_line_width, SolidLine));
#else
   curve->setData(
                  (double *)&( svd_x[ 0 ] ),
                  (double *)&( svd_y[ 0 ] ),
                  svd_x.size()
                  );

   curve->setPen( QPen( plot_colors[ 0 ], use_line_width, Qt::SolidLine ) );
   curve->attach( plot_data );
#endif

   plot_data->setAxisScale( QwtPlot::xBottom, 1, svd_x.size() );
   plot_data->setAxisScale( QwtPlot::yLeft  , svd_y[ 0 ] * 0.9e0 , svd_y.back() * 1.1e0 );

   plot_data_zoomer = new ScrollZoomer(plot_data->canvas());
   plot_data_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
#ifndef QT4
   plot_data_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
   // connect( plot_data_zoomer, SIGNAL( zoomed( const QwtDoubleRect & ) ), SLOT( plot_data_zoomed( const QwtDoubleRect & ) ) );

   plot_data->replot();
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc_Svd::svd()
{
   disable_all();

   QStringList files = selected_files();

   lb_ev->clear();

   last_svd_data = files;
   if ( !subset_data.count( files.join( "\n" ) ) )
   {
      files = add_subset_data( files );
   }
   set < QString > sel_sources = get_selected_sources();
   last_svd_name = *(sel_sources.begin());

   int m = (int) f_qs[ files[ 0 ] ].size();
   int n = (int) files.size();

   vector < vector < double > > F       ( m );
   vector < vector < double > > F_errors;
   vector < double * > a( m );

   svd_F_nonzero = true;

   for ( int i = 0; i < m; ++i )
   {
      F[ i ].resize( n );
      for ( int j = 0; j < n; ++j )
      {
         F[ i ][ j ] = f_Is[ files[ j ] ][ i ];
         if ( !F[ i ][ j ] )
         {
            svd_F_nonzero = false;
         }
      }
      a[ i ] = &(F[ i ][ 0 ]);
   }

   svd_F        = F;
   svd_F_errors = F_errors;
   vector < double > W( n );
   double *w = &(W[ 0 ]);
   vector < double * > v( n );

   vector < vector < double > > V( n );
   for ( int j = 0; j < n; ++j )
   {
      V[ j ].resize( n );
      v[ j ] = &(V[ j ][ 0 ]);
   }
      
   editor_msg( "blue", tr( "SVD: matrix F created, computing SVD" ) );
   if ( !SVD::dsvd( &(a[ 0 ]), m, n, &(w[ 0 ]), &(v[ 0 ]) ) )
   {
      editor_msg( "red", tr( SVD::errormsg ) );
      update_enables();
      return;
   }

   list < svd_sortable_double > svals;

   svd_sortable_double sval;
   for ( int i = 0; i < n; i++ )
   {
      sval.x     = w[ i ];
      sval.index = i;
      svals.push_back( sval );
   }
   svals.sort();
   svals.reverse();

   svd_x.clear();
   svd_y.clear();

   svd_U = F;
   svd_D = W;
   svd_V = V;
   svd_index.resize( svals.size() );

   for ( list < svd_sortable_double >::iterator it = svals.begin();
         it != svals.end();
         ++it )
   {
      svd_index[ (int) svd_x.size() ] = it->index;
      svd_x.push_back( (double) svd_x.size() + 1e0 );
      svd_y.push_back( it->x );
      lb_ev->insertItem( QString( "%1" ).arg( it->x ) );
   }

   update_enables();
}

void US_Hydrodyn_Saxs_Hplc_Svd::recon()
{
   editor_msg( "blue", tr( "Start TSVD reconstruction" ) );
   disable_all();
   recon_mode = "";
   do_recon();
   update_enables();
   editor_msg( "blue", tr( "Done TSVD reconstruction" ) );
}



Q3ListViewItem * US_Hydrodyn_Saxs_Hplc_Svd::lvi_last_depth( int d )
{
   Q3ListViewItem * result = (Q3ListViewItem *) 0;

   Q3ListViewItemIterator it( lv_data );
   while ( it.current() ) 
   {
      Q3ListViewItem *item = it.current();
      if ( item->depth() == d )
      {
         result = item;
      }
      
      ++it;
   }

   return result;
}

void US_Hydrodyn_Saxs_Hplc_Svd::color_rotate()
{
   vector < QColor >  new_plot_colors;

   for ( int i = 1; i < (int) plot_colors.size(); i++ )
   {
      new_plot_colors.push_back( plot_colors[ i ] );
   }
   new_plot_colors.push_back( plot_colors[ 0 ] );
   plot_colors = new_plot_colors;
   replot();
}

void US_Hydrodyn_Saxs_Hplc_Svd::inc_rmsd_plot( bool axis_change )
{
   plot_data->clear();
   plotted_curves.clear();
   if ( plot_data_zoomer )
   {
      plot_data_zoomer->zoom ( 0 );
      delete plot_data_zoomer;
      plot_data_zoomer = (ScrollZoomer *) 0;
   }

   sv_plot   = false;
   rmsd_plot = true;
   chi_plot  = false;

   if ( axis_change )
   {
      last_axis_x_log = axis_x_log;
      last_axis_y_log = axis_y_log;

      axis_x_log = true;
      axis_y_log = false;

      axis_x_title();
      axis_y_title();
   }

#ifndef QT4
   long Iq = plot_data->insertCurve( "rmsd" );
   plotted_curves[ "rmsd" ] = Iq;
   plot_data->setCurveStyle( Iq, QwtCurve::Lines );
#else
   QwtPlotCurve *curve = new QwtPlotCurve( "rmsd" );
   plotted_curves[ "rmsd" ] = curve;
   curve->setStyle( QwtPlotCurve::Lines );
#endif

#ifndef QT4
   plot_data->setCurveData( Iq, 
                            (double *)&( rmsd_x[ 0 ] ),
                            (double *)&( rmsd_y[ 0 ] ),
                            rmsd_x.size()
                            );
   plot_data->setCurvePen( Iq, QPen( plot_colors[ 0 ], use_line_width, SolidLine));
#else
   curve->setData(
                  (double *)&( rmsd_x[ 0 ] ),
                  (double *)&( rmsd_y[ 0 ] ),
                  rmsd_x.size()
                  );

   curve->setPen( QPen( plot_colors[ 0 ], use_line_width, Qt::SolidLine ) );
   curve->attach( plot_data );
#endif

   plot_data->setAxisScale( QwtPlot::xBottom, 1, rmsd_x.size() );
   plot_data->setAxisScale( QwtPlot::yLeft  , vmin( rmsd_y ) * 0.9e0 , vmax( rmsd_y ) * 1.1e0 );

   plot_data_zoomer = new ScrollZoomer(plot_data->canvas());
   plot_data_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
#ifndef QT4
   plot_data_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
   // connect( plot_data_zoomer, SIGNAL( zoomed( const QwtDoubleRect & ) ), SLOT( plot_data_zoomed( const QwtDoubleRect & ) ) );

   plot_data->replot();
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc_Svd::inc_chi_plot( bool axis_change )
{
   plot_data->clear();
   plotted_curves.clear();
   if ( plot_data_zoomer )
   {
      plot_data_zoomer->zoom ( 0 );
      delete plot_data_zoomer;
      plot_data_zoomer = (ScrollZoomer *) 0;
   }

   sv_plot   = false;
   rmsd_plot = false;
   chi_plot  = true;

   if ( axis_change )
   {
      last_axis_x_log = axis_x_log;
      last_axis_y_log = axis_y_log;

      axis_x_log = true;
      axis_y_log = false;

      axis_x_title();
      axis_y_title();
   }

#ifndef QT4
   long Iq = plot_data->insertCurve( "chi" );
   plotted_curves[ "chi" ] = Iq;
   plot_data->setCurveStyle( Iq, QwtCurve::Lines );
#else
   QwtPlotCurve *curve = new QwtPlotCurve( "chi" );
   plotted_curves[ "chi" ] = curve;
   curve->setStyle( QwtPlotCurve::Lines );
#endif

#ifndef QT4
   plot_data->setCurveData( Iq, 
                            (double *)&( chi_x[ 0 ] ),
                            (double *)&( chi_y[ 0 ] ),
                            chi_x.size()
                            );
   plot_data->setCurvePen( Iq, QPen( plot_colors[ 0 ], use_line_width, SolidLine));
#else
   curve->setData(
                  (double *)&( chi_x[ 0 ] ),
                  (double *)&( chi_y[ 0 ] ),
                  chi_x.size()
                  );

   curve->setPen( QPen( plot_colors[ 0 ], use_line_width, Qt::SolidLine ) );
   curve->attach( plot_data );
#endif

   plot_data->setAxisScale( QwtPlot::xBottom, 1, chi_x.size() );
   plot_data->setAxisScale( QwtPlot::yLeft  , vmin( chi_y ) * 0.9e0 , vmax( chi_y ) * 1.1e0 );

   plot_data_zoomer = new ScrollZoomer(plot_data->canvas());
   plot_data_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
#ifndef QT4
   plot_data_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
   // connect( plot_data_zoomer, SIGNAL( zoomed( const QwtDoubleRect & ) ), SLOT( plot_data_zoomed( const QwtDoubleRect & ) ) );

   plot_data->replot();
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc_Svd::inc_recon()
{
   editor_msg( "blue", tr( "Start incremental TSVD" ) );
   recon_mode = "Inc. ";
   disable_all();
   pb_stop->setEnabled( true );
   running = true;

   last_recon_tag = QString( tr( "\"Incremental TSVD on SVD of %1\",\"RMSD of fit\",\"Singular values\"" ) ).arg( last_svd_name );
   last_recon_evs.clear();

   rmsd_x.clear();
   rmsd_y.clear();

   chi_x.clear();
   chi_y.clear();
   
   vector < int > evs_selected;

   for ( int i = 0; i < (int) lb_ev->count(); ++i )
   {
      if ( lb_ev->isSelected( i ) )
      {
         evs_selected.push_back( i );
      }
   }

   disconnect( lb_ev, SIGNAL( selectionChanged() ), 0, 0 );

   for ( int i = 0; i < (int) evs_selected.size(); ++i )
   {
      if ( !running )
      {
         editor_msg( "red", tr( "Processing stopped" ) );
         update_enables();
         return;
      }

      progress->setProgress( i, evs_selected.size() );

      lb_ev->clearSelection();

      QString evs_used;
      for ( int j = 0; j <= i; ++j )
      {
         lb_ev->setSelected( evs_selected[ j ], true );
         evs_used += QString( "%1 ").arg( lb_ev->text( evs_selected[ j ] ) );
      }
      qApp->processEvents();

      do_recon();

      last_recon_evs << QString( "\"%1\"" ).arg( evs_used );
      rmsd_x.push_back( i + 1e0 );
      rmsd_y.push_back( last_recon_rmsd );

      if ( svd_F_nonzero )
      {
         chi_x .push_back( i + 1e0 );
         chi_y .push_back( last_recon_chi );
      }
   }

   progress->reset();

   connect( lb_ev, SIGNAL( selectionChanged() ), SLOT( sv_selection_changed() ) );

   running = false;
   update_enables();
   editor_msg( "blue", tr( "Done TSVD reconstruction" ) );
}

void US_Hydrodyn_Saxs_Hplc_Svd::do_recon()
{
   
   // build new data set I(q) & then make I(t) from selected SV's
   // F = U*D*V^T
   int n = (int) svd_D.size();
   int m = (int) svd_U.size();

   // cout << QString( "svd recon: m %1 n %2\n" ).arg( m ).arg( n ) << flush;

   vector < double > D = svd_D;

   int sv_count = 0;
   QString last_ev;
   for ( int i = 0; i < (int) lb_ev->count(); ++i )
   {
      if ( !lb_ev->isSelected( i ) )
      {
         D[ svd_index[ i ] ] = 0e0;
      } else {
         sv_count++;
         last_ev = lb_ev->text( i );
      }
   }
   
   // US_Vector::printvector( "SVD D zeroed", D );

   // multiply D 
   
   vector < vector < double > > DV = svd_V;

   for ( int i = 0; i < n; ++i )
   {
      for ( int j = 0; j < n; ++j )
      {
         DV[ i ][ j ] = D[ i ] * svd_V[ j ][ i ];
      }
   }

   vector < vector < double > > F( m );

   for ( int i = 0; i < m; ++i )
   {
      F[ i ].resize( n );
      for ( int j = 0; j < n; ++j )
      {
         F[ i ][ j ] = 0;
         for ( int k = 0; k < n; ++k )
         {
            F[ i ][ j ] += svd_U[ i ][ k ] * DV[ k ][ j ];
         }
      }
   }

   // make 
   set < QString > sources = get_sources();

   QString name = QString( "TSVD %1reconstruction %2 SVs%3" ).arg( recon_mode ).arg( sv_count ).arg( sv_count == 1 ? QString( " %1").arg( last_ev ) : QString( "" ) );

   {
      int ext = 0;
      while ( sources.count( name ) )
      {
         name = QString( "TSVD %1reconstruction %2 SVs%3 Trial %4" ).arg( recon_mode ).arg( sv_count ).arg( sv_count == 1 ? QString( " %1").arg( last_ev ) : QString( "" ) ).arg( ++ext );
      }
   }

   Q3ListViewItem * lvi = new Q3ListViewItem( lv_data, lvi_last_depth( 0 ), name );
   svd_data_map[ name ] = last_svd_data;

   Q3ListViewItem * lvn = new Q3ListViewItem( lvi, lvi, "SVD of: " + last_svd_name );
   Q3ListViewItem * evs = new Q3ListViewItem( lvi, lvn, "SVs used" );
   Q3ListViewItem * lvinext = evs;
   for ( int i = 0; i < (int) lb_ev->count(); ++i )
   {
      if ( lb_ev->isSelected( i ) )
      {
         lvinext = new Q3ListViewItem( evs, lvinext, lb_ev->text( i ) );
      }
   }

   Q3ListViewItem * iqs = new Q3ListViewItem( lvi, lv_data->lastItem(), "I(q)" );

   // add I(q)
   // contained in columns of F, reference file names from last_svd_data

   vector < double >  q        = f_qs       [ last_svd_data[ 0 ] ];
   vector < QString > q_string = f_qs_string[ last_svd_data[ 0 ] ];
   vector < double >  e;

   set < QString > current_files = get_current_files();

   QString tag = QString( "TSVD%1_%2" ).arg( sv_count ).arg( sv_count == 1 ? QString( "%1_").arg( last_ev ).replace( ".","_" ) : QString( "" ) );
   int ext = 0;

   // find consistent ext
   
   bool any_found;

   do {
      any_found = false;
      for ( int i = 0; i < n; ++i )
      {
         QString this_name = tag + ( ext ? QString( "%1_" ).arg( ext ) : QString( "" ) ) + last_svd_data[ i ];
         if ( current_files.count( this_name ) )
         {
            ext++;
            any_found = true;
            break;
         }
      }
   } while( any_found );
   tag += ext ? QString( "%1_" ).arg( ext ) : QString( "" );
   //   cout << QString( "tag <%1> any_found %2 %3\n" ).arg( tag ).arg( any_found ? "true" : "false" ).arg( ext );

   QStringList final_files;

   lvinext = iqs;

   // compute rmsd, chi2
   double rmsd2    = 0e0;
   last_recon_rmsd = 0e0;
   double chi2     = 0e0;
   last_recon_chi  = 0e0;
   double tmp;
   double tmp2;

   for ( int i = 0; i < n; ++i )
   {
      vector < double > I( m );
      if ( svd_F_nonzero )
      {
         for ( int j = 0; j < m; ++j )
         {
            I[ j ] = F[ j ][ i ];
            tmp = ( F[ j ][ i ] - svd_F[ j ][ i ] );
            rmsd2 += tmp * tmp;
            tmp2 = tmp / svd_F[ j ][ i ];
            chi2  += tmp2 * tmp2;
         }
      } else {
         for ( int j = 0; j < m; ++j )
         {
            I[ j ] = F[ j ][ i ];
            tmp = F[ j ][ i ] - svd_F[ j ][ i ];
            rmsd2 += tmp * tmp;
         }
      }         

      QString this_name = tag + last_svd_data[ i ];
      lvinext = new Q3ListViewItem( iqs, lvinext, this_name );
      final_files << this_name;

      f_pos      [ this_name ] = f_pos.size();
      f_qs_string[ this_name ] = q_string;
      f_qs       [ this_name ] = q;
      f_Is       [ this_name ] = I;
      f_errors   [ this_name ] = e;
      f_is_time  [ this_name ] = false;  // must all be I(q)
   }

   last_recon_rmsd = sqrt( rmsd2 ) / ( n * m - 1e0 );

   if ( svd_F_nonzero )
   {
      lvinext = new Q3ListViewItem( lvi, evs, QString( "RMSD %1" ).arg( last_recon_rmsd ) );
      last_recon_chi = sqrt( chi2 ) / ( n * m - 1e0 );
      // new QListViewItem( lvi, lvinext, QString( "Chi %1" ).arg( last_recon_chi ) );
   }      

   add_i_of_t( name, final_files, false );
}

double US_Hydrodyn_Saxs_Hplc_Svd::vmin( vector < double > &x )
{
   double min = x[ 0 ];
   for ( int i = 1; i < (int) x.size(); ++i )
   {
      if ( min > x[ i ] )
      {
         min = x[ i ];
      }
   }
   return min;
}

double US_Hydrodyn_Saxs_Hplc_Svd::vmax( vector < double > &x )
{
   double max = x[ 0 ];
   for ( int i = 1; i < (int) x.size(); ++i )
   {
      if ( max < x[ i ] )
      {
         max = x[ i ];
      }
   }
   return max;
}

void US_Hydrodyn_Saxs_Hplc_Svd::stop()
{
   pb_stop->setEnabled( false );
   running = false;
   editor_msg( "red", tr( "Processing stop requested" ) );
   qApp->processEvents();
}

void US_Hydrodyn_Saxs_Hplc_Svd::indiv_recon()
{
   recon_mode = "Indiv. ";
   editor_msg( "blue", tr( "Start individual TSVD" ) );
   disable_all();
   pb_stop->setEnabled( true );
   running = true;

   last_recon_tag = QString( tr( "\"Individual TSVD on SVD of %1\",\"RMSD of fit\",\"Singular value\"" ) ).arg( last_svd_name );
   last_recon_evs.clear();
                            

   rmsd_x.clear();
   rmsd_y.clear();

   chi_x.clear();
   chi_y.clear();
   
   vector < int > evs_selected;

   for ( int i = 0; i < (int) lb_ev->count(); ++i )
   {
      if ( lb_ev->isSelected( i ) )
      {
         evs_selected.push_back( i );
      }
   }

   disconnect( lb_ev, SIGNAL( selectionChanged() ), 0, 0 );

   for ( int i = 0; i < (int) evs_selected.size(); ++i )
   {
      if ( !running )
      {
         editor_msg( "red", tr( "Processing stopped" ) );
         update_enables();
         return;
      }

      progress->setProgress( i, evs_selected.size() );

      lb_ev->clearSelection();

      lb_ev->setSelected( evs_selected[ i ], true );

      qApp->processEvents();

      do_recon();

      last_recon_evs << QString( "%1" ).arg( lb_ev->text( evs_selected[ i ] ) );
      rmsd_x.push_back( i + 1e0 );
      rmsd_y.push_back( last_recon_rmsd );

      if ( svd_F_nonzero )
      {
         chi_x .push_back( i + 1e0 );
         chi_y .push_back( last_recon_chi );
      }
   }

   progress->reset();

   lb_ev->clearSelection();
   for ( int i = 0; i < (int) evs_selected.size(); ++i )
   {
      lb_ev->setSelected( evs_selected[ i ], true );
   }

   connect( lb_ev, SIGNAL( selectionChanged() ), SLOT( sv_selection_changed() ) );

   running = false;
   update_enables();
   editor_msg( "blue", tr( "Done TSVD reconstruction" ) );
}

void US_Hydrodyn_Saxs_Hplc_Svd::set_plot_errors()
{
   if ( cb_plot_errors->isChecked() )
   {
      hide_widgets( errors_widgets, false );
      update_plot_errors();
   } else {
      hide_widgets( errors_widgets, true );
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc_Svd::set_plot_errors_rev()
{
   update_plot_errors();
}

void US_Hydrodyn_Saxs_Hplc_Svd::set_plot_errors_sd()
{
   if ( plot_errors_zoomer )
   {
      plot_errors_zoomer->zoom ( 0 );
      delete plot_errors_zoomer;
      plot_errors_zoomer = (ScrollZoomer *) 0;
   }

   if ( cb_plot_errors_sd->isChecked() &&
        cb_plot_errors_pct->isChecked() )
   {
      cb_plot_errors_pct->setChecked( false );
   }
   update_plot_errors();
}

void US_Hydrodyn_Saxs_Hplc_Svd::set_plot_errors_pct()
{
   if ( plot_errors_zoomer )
   {
      plot_errors_zoomer->zoom ( 0 );
      delete plot_errors_zoomer;
      plot_errors_zoomer = (ScrollZoomer *) 0;
   }

   if ( cb_plot_errors_sd->isChecked() &&
        cb_plot_errors_pct->isChecked() )
   {
      cb_plot_errors_sd->setChecked( false );
   }
   update_plot_errors();
}

void US_Hydrodyn_Saxs_Hplc_Svd::set_plot_errors_group()
{
   if ( plot_errors_zoomer )
   {
      plot_errors_zoomer->zoom ( 0 );
      delete plot_errors_zoomer;
      plot_errors_zoomer = (ScrollZoomer *) 0;
   }
   
   update_plot_errors();
}

void US_Hydrodyn_Saxs_Hplc_Svd::set_plot_errors_ref()
{
   if ( !cb_plot_errors_ref->isChecked() )
   {
      plot_files();
   } else {
      update_plot_errors();
   }
}

void US_Hydrodyn_Saxs_Hplc_Svd::update_plot_errors( bool do_update_enables )
{
   disable_all();
   if ( cb_plot_errors->isChecked() )
   {
      // cout << "update plot errors\n";
      cb_plot_errors_group->isChecked() ? do_plot_errors_group() : do_plot_errors();
   }
   if ( do_update_enables )
   {
      update_enables();
   }
}

QStringList US_Hydrodyn_Saxs_Hplc_Svd::add_subset_data( QStringList files )
{
   QStringList result;

   set < QString > sources     = get_sources();
   set < QString > sel_sources = get_selected_sources();

   if ( sel_sources.size() != 1 )
   {
      editor_msg( "red", QString( tr( "Internal error: add_subset_data with more than one source (%1)" ) ).arg( sel_sources.size() ) );
      return result;
   }

   QString name = QString( tr( "Subset of %1" ) ).arg( *(sel_sources.begin()) );

   {
      int ext = 0;
      while ( sources.count( name ) )
      {
         name = QString( tr( "Subset %1 of %2" ) ).arg( ++ext ).arg( *(sel_sources.begin()) );
      }
   }

   Q3ListViewItem * lvi = new Q3ListViewItem( lv_data, lvi_last_depth( 0 ), name );

   // copy over I(q), ignore SVs, rmsd since these are not computed

   Q3ListViewItem * iqs = new Q3ListViewItem( lvi, lv_data->lastItem(), "I(q)" );

   QString head = hplc_win->qstring_common_head( files, true );
   QString tag  = head;
   bool any_found;

   set < QString > current_files = get_current_files();

   int ext = 0;
   do {
      any_found = false;
      for ( int i = 0; i < (int)files.size(); ++i )
      {
         QString this_name = tag + ( ext ? QString( "%1_" ).arg( ext ) : QString( "" ) ) + last_svd_data[ i ].mid( head.length(), files[ i ].length() - head.length() );
         if ( current_files.count( this_name ) )
         {
            ext++;
            any_found = true;
            break;
         }
      }
   } while( any_found );
   tag += ext ? QString( "%1_" ).arg( ext ) : QString( "" );

   Q3ListViewItem * lvinext = iqs;

   for ( int i = 0; i < (int)files.size(); ++i )
   {
      QString this_name = tag + last_svd_data[ i ].mid( head.length(), files[ i ].length() - head.length() );
      lvinext = new Q3ListViewItem( iqs, lvinext, this_name );
      result << this_name;
      
      f_pos      [ this_name ] = f_pos.size();
      f_qs_string[ this_name ] = f_qs_string[ files[ i ] ];
      f_qs       [ this_name ] = f_qs[ files[ i ] ];
      f_Is       [ this_name ] = f_Is[ files[ i ] ];
      f_errors   [ this_name ] = f_errors[ files[ i ] ];
      f_is_time  [ this_name ] = false;  // must all be I(q)
   }

   disconnect(lv_data, SIGNAL(selectionChanged()), 0, 0 );
   lv_data->clearSelection();
   lvi->setSelected( true );
   connect(lv_data, SIGNAL(selectionChanged()), SLOT( data_selection_changed() ));

   add_i_of_t( name, result, false );

   return result;
}

QString US_Hydrodyn_Saxs_Hplc_Svd::get_related_source_name( QString name )
{
   // find "SVD of: " in current source

   Q3ListViewItemIterator it( lv_data );
   while ( it.current() ) 
   {
      Q3ListViewItem *item = it.current();
      if ( item->depth() == 1 &&
           item->text( 0 ).left( 8 ) == "SVD of: " &&
           item->parent()->text( 0 ) == name
           )
      {
         return item->text( 0 ).right( item->text( 0 ).length() - 8 );
      }
      ++it;
   }
   return QString( "" );
}

QStringList US_Hydrodyn_Saxs_Hplc_Svd::get_files_by_name( QString name )
{
   QStringList result;
   QString iq_or_it = iq_it_state ? "I(t)" : "I(q)";

   Q3ListViewItemIterator it( lv_data );
   while ( it.current() ) 
   {
      Q3ListViewItem *item = it.current();
      if ( 
           item->depth() == 2 &&
           item->parent()->text( 0 ) == iq_or_it &&
           item->parent()->parent()->text( 0 ) == name 
           )
      {
         result << item->text( 0 );
      }
      ++it;
   }
   return result;
}

bool US_Hydrodyn_Saxs_Hplc_Svd::get_plot_files( QStringList &use_list, QStringList &use_ref_list )
{
   set < QString > sources = get_selected_sources();
   if ( !sources.size() )
   {
      return false;
   }
   if ( sources.size() > 1 )
   {
      editor_msg( "red", QString( tr( "Internal error: get_plot_files() with more than one source (%1)" ) ).arg( sources.size() ) );
      return false;
   }

   QString source     = *( sources.begin() );
   QString ref_source = get_related_source_name( source );
   
   if ( ref_source.isEmpty() )
   {
      editor_msg( "red", QString( tr( "Internal error: get_plot_files() could not find related name for source %1" ) ).arg( source ) );
      return false;
   }

   // cout << QString( "ref source <%1>\n" ).arg( ref_source );

   // find related curves

   QStringList files = selected_files();

   QStringList full_list     = get_files_by_name( source     );
   QStringList full_ref_list = get_files_by_name( ref_source );

   if ( full_list.size() != full_ref_list.size() )
   {
      editor_msg( "red", QString( tr( "Internal error: get_plot_files() file list count mismatch %1 %2" ) ).arg( full_list.size() ).arg( full_ref_list.size() ) );
      return false;
   }
      
   set < QString > selected;

   for ( int i = 0; i < (int) files.size(); ++i )
   {
      selected.insert( files[ i ] );
   }

   use_list.clear();
   use_ref_list.clear();

   for ( int i = 0; i < (int) full_list.size(); ++i )
   {
      if ( selected.count( full_list[ i ] ) )
      {
         use_list     << full_list    [ i ];
         use_ref_list << full_ref_list[ i ];
         // cout << QString( "errors: compare <%1> <%2>\n" ).arg( full_list[ i ] ).arg( full_ref_list[ i ] );
      }
   }
   return use_list.size() > 0;
}
   
void US_Hydrodyn_Saxs_Hplc_Svd::do_plot_errors()
{
   // cout << "do plot errors\n";
   
   QStringList use_list;
   QStringList use_ref_list;
   if ( !get_plot_files( use_list, use_ref_list ) )
   {
      return;
   }
   
   plot_errors->clear();
   plot_errors_reference.clear();

   vector < double > grid;
   vector < double > target;
   vector < double > errors;
   vector < double > fit;

   double grid_ofs = f_qs[ use_list[ 0 ] ][ 0 ];
   plot_errors_jumps.clear();
   plot_errors_jumps.push_back( grid_ofs );

   bool use_errors = true;

   vector < int >    use_pos;
   vector < QColor > use_color( (int) use_list.size() );

   for ( int i = 0; i < (int) use_list.size(); ++i )
   {
      if ( use_errors &&
           ( f_errors[ use_ref_list[ i ] ].size() !=
             f_qs    [ use_ref_list[ i ] ].size() ||
             !US_Saxs_Util::is_nonzero_vector( f_errors[ use_ref_list[ i ] ] ) ) )
      {
         use_errors = false;
         cb_plot_errors_sd->setChecked( false );
         cb_plot_errors_sd->hide();
      }
      
      use_color[ i ] = plot_colors[ f_pos[ use_list[ i ] ] % plot_colors.size() ];

      if ( f_qs_string[ use_ref_list[ i ] ] != 
           f_qs_string[ use_list    [ i ] ] )
      {
         editor_msg( "red", QString( tr( "Error: plot residuals: curve grid mismatch" ) ) );
         return;
      }

      for ( int j = 0; j < (int) f_qs[ use_list[ i ] ].size(); ++j )
      {
         grid   .push_back( f_qs[ use_list[ i ] ][ j ] - f_qs[ use_list[ i ] ][ 0 ] + grid_ofs );
         fit    .push_back( f_Is[ use_list[ i ] ][ j ] );
         target .push_back( f_Is[ use_ref_list[ i ] ][ j ] );
         use_pos.push_back( i );   
         if ( use_errors )
         {
            errors.push_back( f_errors[ use_ref_list[ i ] ][ j ] );
         }
      }
      grid_ofs += f_qs[ use_list[ i ] ].back() - f_qs[ use_list[ i ] ][ 0 ];
      plot_errors_jumps.push_back( grid_ofs );
   }
   plot_errors_jumps.pop_back();

   if ( cb_plot_errors_rev->isChecked() )
   {
      vector < double > tmp = fit;
      fit = target;
      target = tmp;
   }

   vector < vector < double > > x( (int) use_list.size() );
   vector < vector < double > > y( (int) use_list.size() );
   vector < vector < double > > e( (int) use_list.size() );

   if ( cb_plot_errors_pct->isChecked() )
   {
      if ( cb_plot_errors_sd->isChecked() && errors.size() == target.size() && US_Saxs_Util::is_nonzero_vector( errors ))
      {
         // does % and errors make sense?, I am excluding this for now
         // cout << "pct mode with errors, not acceptable\n";
         for ( unsigned int i = 0; i < ( unsigned int )target.size(); i++ )
         {
            if ( target[ i ] != 0e0 )
            {
               x[ use_pos[ i ] ].push_back( grid[ i ] );
               y[ use_pos[ i ] ].push_back( 0e0 );
               e[ use_pos[ i ] ].push_back( 100.0 * ( target[ i ] - fit[ i ] ) / target[ i ] );
            }
         }
      } else {
         // cout << "pct mode, not using errors\n";

         for ( unsigned int i = 0; i < ( unsigned int )target.size(); i++ )
         {
            if ( target[ i ] != 0e0 )
            {
               x[ use_pos[ i ] ].push_back( grid[ i ] );
               y[ use_pos[ i ] ].push_back( 0e0 );
               e[ use_pos[ i ] ].push_back( 100.0 * ( target[ i ] - fit[ i ] ) / target[ i ] );
            } else {
               cout << QString( "target at pos %1 is zero\n" ).arg( i );
            }
         }
      }         
   } else {
      if ( cb_plot_errors_sd->isChecked() && errors.size() == target.size() && US_Saxs_Util::is_nonzero_vector( errors ))
      {
         // cout << "errors ok & used\n";
         for ( unsigned int i = 0; i < ( unsigned int )target.size(); i++ )
         {
            x[ use_pos[ i ] ].push_back( grid[ i ] );
            y[ use_pos[ i ] ].push_back( 0e0 );
            e[ use_pos[ i ] ].push_back( ( target[ i ] - fit[ i ] ) / errors[ i ] );
         }
      } else {
         // cout << "errors not ok & not used\n";
         for ( unsigned int i = 0; i < ( unsigned int )target.size(); i++ )
         {
            x[ use_pos[ i ] ].push_back( grid[ i ] );
            y[ use_pos[ i ] ].push_back( 0e0 );
            e[ use_pos[ i ] ].push_back( target[ i ] - fit[ i ] );
         }
      }
   }

   for ( int j = 0; j < (int) e.size(); ++j )
   {
      for ( unsigned int i = 0; i < ( unsigned int ) e.size(); i++ )
      {
         if ( e[ j ][ i ] < -50e0 )
         {
            e[ j ][ i ] = -50e0;
         } else {
            if ( e[ j ][ i ] > 50e0 )
            {
               e[ j ][ i ] = 50e0;
            }
         }
      }
   }

   // printvector( "x", x );
   // printvector( "e", e );

   for ( int j = 0; j < (int) x.size(); ++j )
   {
      {
#ifndef QT4
         long curve;
         curve = plot_errors->insertCurve( "base" );
         plot_errors->setCurveStyle( curve, QwtCurve::Lines );
#else
         QwtPlotCurve *curve = new QwtPlotCurve( "base" );
         curve->setStyle( QwtPlotCurve::Lines );
#endif

#ifndef QT4
         plot_errors->setCurvePen( curve, QPen( Qt::green, use_line_width, Qt::SolidLine ) );
         plot_errors->setCurveData( curve,
                                    (double *)&x[ j ][ 0 ],
                                    (double *)&y[ j ][ 0 ],
                                    x[ j ].size()
                                    );
#else
         curve->setPen( QPen( Qt::green, use_line_width, Qt::SolidLine ) );
         curve->setData(
                        (double *)&x[ j ][ 0 ],
                        (double *)&y[ j ][ 0 ],
                        x[ j ].size()
                        );
         curve->attach( plot_errors );
#endif
      }

      {
#ifndef QT4
         long curve;
         curve = plot_errors->insertCurve( "errors" );
         plot_errors->setCurveStyle( curve, QwtCurve::Lines );
#else
         QwtPlotCurve *curve = new QwtPlotCurve( "errors" );
         curve->setStyle( QwtPlotCurve::Lines );
#endif

#ifndef QT4
         plot_errors->setCurvePen( curve, QPen( use_color[ j ], use_line_width, Qt::SolidLine ) );
         plot_errors->setCurveData( curve,
                                    (double *)&x[ j ][ 0 ],
                                    (double *)&e[ j ][ 0 ],
                                    x[ j ].size()
                                    );
         plot_errors->curve( curve )->setStyle( QwtCurve::Sticks );
#else
         curve->setPen( QPen( use_color[ j ], use_line_width, Qt::SolidLine ) );
         curve->setData(
                        (double *)&x[ j ][ 0 ],
                        (double *)&e[ j ][ 0 ],
                        x[ j ].size()
                        );
         curve->setStyle( QwtPlotCurve::Sticks );
         curve->attach( plot_errors );
#endif
      }
   }

   if ( !plot_errors_zoomer )
   {
      double maxy = e[ 0 ][ 0 ];

      for ( int j = 0; j < (int) x.size(); ++j )
      {
         for ( unsigned int i = 1; i < ( unsigned int )e[ j ].size(); i++ )
         {
            if ( maxy < fabs( e[ j ][ i ] ) )
            {
               maxy = fabs( e[ j ][ i ] );
            }
         }            
      }

      plot_errors->setAxisTitle(QwtPlot::yLeft, QString(
                                                        tr( cb_plot_errors_pct->isChecked() ?
                                                            "% difference %1" :
                                                            ( cb_plot_errors_sd->isChecked() ?
                                                              "delta I%1/sd" : "delta I%1" 
                                                              ) ) ).arg( iq_it_state ? "I(t)" : "I(q)" ) );

      plot_errors->setAxisScale( QwtPlot::xBottom, x[ 0 ][ 0 ], x.back().back() );
      plot_errors->setAxisScale( QwtPlot::yLeft  , -maxy * 1.2e0 , maxy * 1.2e0 );

      plot_errors_zoomer = new ScrollZoomer(plot_errors->canvas());
      plot_errors_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
#ifndef QT4
      plot_errors_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
      // connect( plot_errors_zoomer, SIGNAL( zoomed( const QwtDoubleRect & ) ), SLOT( plot_errors_zoomed( const QwtDoubleRect & ) ) );
   }

   plot_errors_jump_markers();

   plot_errors->replot();

   if ( cb_plot_errors_ref->isChecked() )
   {
      for ( int i = 0; i < (int) use_ref_list.size(); ++i )
      {
         plot_errors_reference.insert( use_ref_list[ i ] );
      }
      plot_files( use_ref_list );
   }
}

void US_Hydrodyn_Saxs_Hplc_Svd::plot_errors_jump_markers()
{
   if ( cb_plot_errors_group->isChecked() ||
        plot_errors_jumps.size() > 25 )
   {
      return;
   }

   for ( int i = 0; i < (int) plot_errors_jumps.size(); i++ )
   {
#ifndef QT4
      long marker = plot_errors->insertMarker();
      plot_errors->setMarkerLineStyle ( marker, QwtMarker::VLine );
      plot_errors->setMarkerPos       ( marker, plot_errors_jumps[ i ], 0e0 );
      plot_errors->setMarkerLabelAlign( marker, Qt::AlignRight | Qt::AlignTop );
      plot_errors->setMarkerPen       ( marker, QPen( Qt::cyan, 1, DashDotDotLine));
      plot_errors->setMarkerFont      ( marker, QFont("Helvetica", 11, QFont::Bold) );
      plot_errors->setMarkerLabelText ( marker, QString( "%1" ).arg( i + 1 ) );
#else
      QwtPlotMarker * marker = new QwtPlotMarker;
      marker->setLineStyle       ( QwtPlotMarker::VLine );
      marker->setLinePen         ( QPen( Qt::cyan, 2, Qt::DashDotDotLine ) );
      marker->setLabelOrientation( Qt::Horizontal );
      marker->setXValue          ( plot_errors_jumps[ i ] );
      marker->setLabelAlignment  ( Qt::AlignRight | Qt::AlignTop );
      {
         QwtText qwtt( QString( "%1" ).arg( i + 1 ) );
         qwtt.setFont( QFont("Helvetica", 11, QFont::Bold ) );
         marker->setLabel           ( qwtt );
      }
      marker->attach             ( plot_errors );
#endif
   }
   plot_errors->replot();
}

void US_Hydrodyn_Saxs_Hplc_Svd::do_plot_errors_group()
{
   // cout << "do plot errors group\n";

   QStringList use_list;
   QStringList use_ref_list;
   if ( !get_plot_files( use_list, use_ref_list ) )
   {
      return;
   }

   plot_errors->clear();
   plot_errors_reference.clear();

   bool use_errors = true;

   double maxy = 0e0;
   double minx = 0e0;
   double maxx = 0e0;

   for ( int i = 0; i < (int) use_list.size(); ++i )
   {
      if ( use_errors &&
           ( f_errors[ use_ref_list[ i ] ].size() !=
             f_qs    [ use_ref_list[ i ] ].size() ||
             !US_Saxs_Util::is_nonzero_vector( f_errors[ use_ref_list[ i ] ] ) ) )
      {
         use_errors = false;
         cb_plot_errors_sd->setChecked( false );
         cb_plot_errors_sd->hide();
      }
   }

   for ( int j = 0; j < (int) use_list.size(); ++j )
   {
      vector < double > x;
      vector < double > y;
      vector < double > e;

      QString this_color_file = use_list    [ j ];
      QString this_file       = use_list    [ j ];
      QString this_ref_file   = use_ref_list[ j ];
      QString this_ref_errors = use_ref_list[ j ];

      if ( cb_plot_errors_rev->isChecked() )
      {
         this_file       = use_ref_list[ j ];
         this_ref_file   = use_list    [ j ];
         this_ref_errors = use_ref_list[ j ];
      }

      if ( cb_plot_errors_pct->isChecked() )
      {
         if ( cb_plot_errors_sd->isChecked() )
         {
            // does % and errors make sense?, I am excluding this for now
            // cout << "pct mode with errors, not acceptable\n";
            for ( int i = 0; i < (int) f_qs[ this_file ].size(); ++i )
            {
               if ( f_Is[ this_ref_file ][ i ] != 0e0 )
               {
                  x.push_back( f_qs[ this_file ][ i ] );
                  y.push_back( 0e0 );
                  e.push_back( 100.0 * ( f_Is[ this_ref_file ][ i ] - f_Is[ this_file ][ i ] ) / f_Is[ this_ref_file ][ i ] );
               }
            }
         } else {
            // cout << "pct mode, not using errors\n";

            for ( int i = 0; i < (int) f_qs[ this_file ].size(); ++i )
            {
               if ( f_Is[ this_ref_file ][ i ] != 0e0 )
               {
                  x.push_back( f_qs[ this_file ][ i ] );
                  y.push_back( 0e0 );
                  e.push_back( 100.0 * ( f_Is[ this_ref_file ][ i ] - f_Is[ this_file ][ i ] ) / f_Is[ this_ref_file ][ i ] );
               } else {
                  cout << QString( "target at pos %1 is zero\n" ).arg( i );
               }
            }
         }         
      } else {
         if ( cb_plot_errors_sd->isChecked() && use_errors )
         {
            // cout << "errors ok & used\n";
            for ( int i = 0; i < (int) f_qs[ this_file ].size(); ++i )
            {
               x.push_back( f_qs[ this_file ][ i ] );
               y.push_back( 0e0 );
               e.push_back( ( f_Is[ this_ref_file ][ i ] - f_Is[ this_file ][ i ] ) / f_errors[ this_ref_errors ][ i ] );
            }
         } else {
            // cout << "errors not ok & not used\n";
            for ( int i = 0; i < (int) f_qs[ this_file ].size(); ++i )
            {
               x.push_back( f_qs[ this_file ][ i ] );
               y.push_back( 0e0 );
               e.push_back( f_Is[ this_ref_file ][ i ] - f_Is[ this_file ][ i ] );
            }
         }
      }

      for ( unsigned int i = 0; i < ( unsigned int ) e.size(); i++ )
      {
         if ( e[ i ] < -50e0 )
         {
            e[ i ] = -50e0;
         } else {
            if ( e[ i ] > 50e0 )
            {
               e[ i ] = 50e0;
            }
         }
      }

      //       printvector( QString( "x for curve %1" ).arg( j ), x );
      //       printvector( QString( "e for curve %1" ).arg( j ), e );

      // only need the bar once

      if ( !j ) 
      {
#ifndef QT4
         long curve;
         curve = plot_errors->insertCurve( "base" );
         plot_errors->setCurveStyle( curve, QwtCurve::Lines );
#else
         QwtPlotCurve *curve = new QwtPlotCurve( "base" );
         curve->setStyle( QwtPlotCurve::Lines );
#endif

#ifndef QT4
         plot_errors->setCurvePen( curve, QPen( Qt::green, use_line_width, Qt::SolidLine ) );
         plot_errors->setCurveData( curve,
                                    (double *)&x[ 0 ],
                                    (double *)&y[ 0 ],
                                    x.size()
                                    );
#else
         curve->setPen( QPen( Qt::green, use_line_width, Qt::SolidLine ) );
         curve->setData(
                        (double *)&x[ 0 ],
                        (double *)&y[ 0 ],
                        x.size()
                        );
         curve->attach( plot_errors );
#endif
      }

      {
#ifndef QT4
         long curve;
         curve = plot_errors->insertCurve( "errors" );
         plot_errors->setCurveStyle( curve, QwtCurve::Lines );
#else
         QwtPlotCurve *curve = new QwtPlotCurve( "errors" );
         curve->setStyle( QwtPlotCurve::Lines );
#endif

#ifndef QT4
         plot_errors->setCurvePen( curve, QPen( plot_colors[ f_pos[ this_color_file ] % plot_colors.size() ], use_line_width, Qt::SolidLine ) );
         plot_errors->setCurveData( curve,
                                    (double *)&x[ 0 ],
                                    (double *)&e[ 0 ],
                                    x.size()
                                    );
         plot_errors->curve( curve )->setStyle( QwtCurve::Sticks );
#else
         curve->setPen( QPen( plot_colors[ f_pos[ this_color_file ] % plot_colors.size() ], use_line_width, Qt::SolidLine ) );
         curve->setData(
                        (double *)&x[ 0 ],
                        (double *)&e[ 0 ],
                        x.size()
                        );
         curve->setStyle( QwtPlotCurve::Sticks );
         curve->attach( plot_errors );
#endif
      }

      if ( !j )
      {
         maxy = fabs( e[ 0 ] );
         minx = x[ 0 ];
         maxx = x[ 0 ];
      }

      for ( unsigned int i = 0; i < ( unsigned int )e.size(); i++ )
      {
         if ( maxy < fabs( e[ i ] ) )
         {
            maxy = fabs( e[ i ] );
         }
         if ( minx > x[ i ] )
         {
            minx = x[ i ];
         }
         if ( maxx < x[ i ] )
         {
            maxx = x[ i ];
         }
      }            
   } // for each curve

   if ( !plot_errors_zoomer )
   {
      // cout << "upeg: recreating axis\n";
      plot_errors->setAxisTitle(QwtPlot::yLeft, QString(
                                                        tr( cb_plot_errors_pct->isChecked() ?
                                                            "% difference %1" :
                                                            ( cb_plot_errors_sd->isChecked() ?
                                                              "delta I%1/sd" : "delta I%1" 
                                                              ) ) ).arg( iq_it_state ? "I(t)" : "I(q)" ) );

      plot_errors->setAxisScale( QwtPlot::xBottom, minx, maxx );
      plot_errors->setAxisScale( QwtPlot::yLeft  , -maxy * 1.2e0 , maxy * 1.2e0 );

      plot_errors_zoomer = new ScrollZoomer(plot_errors->canvas());
      plot_errors_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
#ifndef QT4
      plot_errors_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
      // connect( plot_errors_zoomer, SIGNAL( zoomed( const QwtDoubleRect & ) ), SLOT( plot_zoomed( const QwtDoubleRect & ) ) );
   }

   plot_errors->replot();

   if ( cb_plot_errors_ref->isChecked() )
   {
      for ( int i = 0; i < (int) use_ref_list.size(); ++i )
      {
         plot_errors_reference.insert( use_ref_list[ i ] );
      }
      plot_files( use_ref_list );
   }
}

bool US_Hydrodyn_Saxs_Hplc_Svd::setup_save( QString tag, QString & fname )
{
   QString use_dir = QDir::currentDirPath();

   if ( ush_win->saxs_widget )
   {
      ush_win->saxs_plot_window->select_from_directory_history( use_dir, this );
      raise();
   }
   fname = Q3FileDialog::getSaveFileName(
                                        use_dir,
                                        "Text files (*.txt *.TXT);;"
                                        "CSV files (*.csv *.CSV);;"
                                        "All Files (*)",
                                        this,
                                        caption() + " " + tag,
                                        tr( "Select a file name to " ) + tag );
   if ( fname.isEmpty() )
   {
      return false;
   }

   if ( QFile::exists( fname ) )
   {
      fname = ush_win->fileNameCheck( fname, 0, this );
      raise();
   }
   if ( ush_win->saxs_widget )
   {
      ush_win->saxs_plot_window->add_to_directory_history( fname );
   }
   return true;
}

void US_Hydrodyn_Saxs_Hplc_Svd::rmsd_save()
{
   QString fname;
   if ( !setup_save( tr( "save last RMSD values" ), fname ) )
   {
      return;
   }

   QFile f( fname );

   if ( !f.open( QIODevice::WriteOnly ) )
   {
      editor_msg( "red", QString( tr( "Error: can not open %1 for writing" ) ).arg( fname ) );
      return;
   }

   QString out = last_recon_tag + "\n";
   for ( int i = 0; i < (int) rmsd_x.size(); ++i )
   {
      out += QString( "%1,%2,%3" ).arg( rmsd_x[ i ] ).arg( rmsd_y[ i ] ).arg( last_recon_evs[ i ] ) + "\n";
   }

   if ( fname.lower().contains( QRegExp( "\\.txt$" ) ) )
   {
      out.replace( "\"", "" ).replace( ",", "\t" );
   }

   Q3TextStream ts( &f );
   ts << out;
   f.close();
   editor_msg( "blue", QString( tr( "RMSDs saved in %1" ) ).arg( fname ) );
}

void US_Hydrodyn_Saxs_Hplc_Svd::svd_save()
{
   QString fname;
   if ( !setup_save( tr( "save last SVD values" ), fname ) )
   {
      return;
   }

   QFile f( fname );

   if ( !f.open( QIODevice::WriteOnly ) )
   {
      editor_msg( "red", QString( tr( "Error: can not open %1 for writing" ) ).arg( fname ) );
      return;
   }

   QString out = QString( tr( "\"SVD of %1\",\"Singular value\"" ) ).arg( last_svd_name ) + "\n";
   for ( int i = 0; i < (int) lb_ev->count(); ++i )
   {
      out += QString( "%1,%2" ).arg( i + 1 ).arg( lb_ev->text( i ) ) + "\n";
   }

   if ( fname.lower().contains( QRegExp( "\\.txt$" ) ) )
   {
      out.replace( "\"", "" ).replace( ",", "\t" );
   }

   Q3TextStream ts( &f );
   ts << out;
   f.close();
   editor_msg( "blue", QString( tr( "RMSDs saved in %1" ) ).arg( fname ) );
}
