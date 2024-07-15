#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_mals_saxs_svd.h"
#include "../include/us_svd.h"

#define UHSHS_MAX_SV_FOR_EFA 12 // max SVs for EFA
#define SVD_THRESH 0.9

#include <qwt_scale_engine.h>
#include <qpalette.h>
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

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#define SLASH QDir::separator()

US_Hydrodyn_Mals_Saxs_Svd::US_Hydrodyn_Mals_Saxs_Svd(
                                                     US_Hydrodyn_Mals_Saxs *mals_saxs_win,
                                                     vector < QString > mals_saxs_selected_files,
                                                     QWidget *p
                                                     ) : QFrame( p )
{
   this->mals_saxs_win                = mals_saxs_win;
   this->mals_saxs_selected_files     = mals_saxs_selected_files;
   this->ush_win                 = (US_Hydrodyn *)(mals_saxs_win->us_hydrodyn);
   this->plot_colors             = mals_saxs_win->plot_colors;
   this->use_line_width          = mals_saxs_win->use_line_width;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle(us_tr("US-SOMO: MALS_SAXS SVD"));

   cg_red = USglobal->global_colors.cg_label;
   cg_red.setBrush( QPalette::WindowText, QBrush( QColor( "red" ),  Qt::SolidPattern ) );

   efa_range_processing = false;
   
   if ( !mals_saxs_selected_files.size() )
   {
      QMessageBox::warning( this, 
                            windowTitle(),
                            us_tr( "Internal error: HPLC SVD called with no curves selected" ) );
      close();
      return;
   }

   norm_name_map[ NORM_NOT ] = "not normed";
   norm_name_map[ NORM_PW  ] = "normed by p.w. errors";
   norm_name_map[ NORM_AVG ] = "normed by avg. errors";

   QStringList original_data;

   mode_i_of_t = false;

   if( mals_saxs_win->f_is_time[ this->mals_saxs_selected_files[ 0 ] ] ) {
      QStringList qsl_mals_saxs_selected_files;
      for ( int i = 0; i < (int) this->mals_saxs_selected_files.size(); ++i ) {
         qsl_mals_saxs_selected_files << this->mals_saxs_selected_files[ i ];;
      }

      QString error_msg;
      if ( !convert_it_to_iq( qsl_mals_saxs_selected_files, original_data, error_msg ) ) {
         QMessageBox::warning( this, 
                               windowTitle(),
                               us_tr( "Error: converting I(t)->I(q) " + error_msg ) );
         close();
         return;
      }
      this->mals_saxs_selected_files.clear();
      for ( int i = 0; i < (int) original_data.size(); ++i ) {
         this->mals_saxs_selected_files.push_back( original_data[ i ] );
      }
   } else {
      for ( int i = 0; i < (int) this->mals_saxs_selected_files.size(); ++i ) {
         QString this_name = this->mals_saxs_selected_files[ i ];
         original_data.push_back( this_name );
      
         f_pos      [ this_name ] = f_pos.size();
         f_qs_string[ this_name ] = mals_saxs_win->f_qs_string[ this_name ];
         f_qs       [ this_name ] = mals_saxs_win->f_qs       [ this_name ];
         f_Is       [ this_name ] = mals_saxs_win->f_Is       [ this_name ];
         f_errors   [ this_name ] = mals_saxs_win->f_errors   [ this_name ];
         f_is_time  [ this_name ] = mode_i_of_t;  // false;  // must all be I(q)
      }
   }

   setup_norm();

   subset_data.insert( original_data.join( "\n" ) );

   plot_data_zoomer       = (ScrollZoomer *) 0;
   plot_errors_zoomer     = (ScrollZoomer *) 0;
   plot_ac_zoomer         = (ScrollZoomer *) 0;
   plot_svd_zoomer        = (ScrollZoomer *) 0;
   plot_lefa_zoomer       = (ScrollZoomer *) 0;
   plot_refa_zoomer       = (ScrollZoomer *) 0;
   plot_efa_decomp_zoomer = (ScrollZoomer *) 0;

   le_last_focus      = (mQLineEdit *) 0;
   efa_plot_count         = 0;
   
   running            = false;

   setupGUI();

   if ( !norm_ok ) {
      editor_msg( "dark red", us_tr( "Missing or zero associated errors so normalization is disabled" ) );
   }

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

   setGeometry(global_Xpos, global_Ypos, 1024, 768 );

   show();

   add_i_of_q_or_t( us_tr( "Original data" ), original_data );

   // #define DEBUG_TRANSPOSE

#if defined( DEBUG_TRANSPOSE )
   {
      vector < vector < double > > A(3);
      for ( int i = 0; i < (int) A.size(); ++i ) {
         A[ i ].resize( 5 );
         for ( int j = 0; j < 5; ++j ) {
            A[ i ][ j ] = i + j;
         }
      }
      vector < vector < double > > result = transpose( A );
   }
#endif
}

US_Hydrodyn_Mals_Saxs_Svd::~US_Hydrodyn_Mals_Saxs_Svd()
{
}

void US_Hydrodyn_Mals_Saxs_Svd::setupGUI()
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

   lv_data = new QTreeWidget( this );
   lv_data->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lv_data->setPalette( PALET_EDIT );
   AUTFBACK( lv_data );
   lv_data->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lv_data->setEnabled(true);
   // lv_data->setMinimumWidth();

   lv_data->setColumnCount( 1 );
   lv_data->setHeaderLabels( QStringList()
                             << us_tr( "Source" )
                             );
   
   lv_data->setSortingEnabled        ( false );
   lv_data->setRootIsDecorated( true );
   lv_data->setSelectionMode( QAbstractItemView::ExtendedSelection );
   connect(lv_data, SIGNAL(itemSelectionChanged()), SLOT( data_selection_changed() ));

   QTreeWidgetItem *element = new QTreeWidgetItem( QStringList() << us_tr( "Original data" ) );
   lv_data->addTopLevelItem( element );
   QTreeWidgetItem *iq = new QTreeWidgetItem( QStringList() << QString( mode_i_of_t ? "I(t)" : "I(q)" ) );
   element->addChild( iq );
   QTreeWidgetItem *it = new QTreeWidgetItem( element, iq );
   it->setText( 0,  mode_i_of_t ? "I(q)" : "I(t)" );

   QTreeWidgetItem *lvi = iq;

   for ( int i = 0; i < (int) mals_saxs_selected_files.size(); ++i )
   {
      lvi = new QTreeWidgetItem( iq, lvi );
      lvi->setText( 0, mals_saxs_selected_files[ i ] );
   }
   
   // make i(t), add also

   data_widgets.push_back( lv_data );
   
   pb_clear = new QPushButton(us_tr("Clear"), this);
   pb_clear->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_clear->setMinimumHeight(minHeight3);
   pb_clear->setPalette( PALET_PUSHB );
   connect(pb_clear, SIGNAL(clicked( )), SLOT(clear( )));
   data_widgets.push_back( pb_clear );

   pb_to_mals_saxs = new QPushButton(us_tr("To HPLC window"), this);
   pb_to_mals_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_to_mals_saxs->setMinimumHeight(minHeight3);
   pb_to_mals_saxs->setPalette( PALET_PUSHB );
   connect(pb_to_mals_saxs, SIGNAL(clicked()), SLOT(to_mals_saxs()));
   data_widgets.push_back( pb_to_mals_saxs );

   pb_save_plots = new QPushButton(us_tr("Save Plots"), this);
   pb_save_plots->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_save_plots->setMinimumHeight(minHeight3);
   pb_save_plots->setPalette( PALET_PUSHB );
   connect(pb_save_plots, SIGNAL(clicked()), SLOT(save_plots()));
   data_widgets.push_back( pb_save_plots );

   pb_color_rotate = new QPushButton(us_tr("Color"), this);
   pb_color_rotate->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_color_rotate->setMinimumHeight(minHeight3);
   pb_color_rotate->setPalette( PALET_PUSHB );
   connect(pb_color_rotate, SIGNAL(clicked()), SLOT(color_rotate()));
   data_widgets.push_back( pb_color_rotate );

   pb_replot = new QPushButton(us_tr("Replot"), this);
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

   editor = new QTextEdit(this);
   editor->setPalette( PALET_NORMAL );
   AUTFBACK( editor );
   editor->setReadOnly(true);
   editor->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ));

   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight3);

   mb_editor = new QMenuBar( frame );    mb_editor->setObjectName( "menu" );
   mb_editor->setMinimumHeight(minHeight1 - 5);
   mb_editor->setPalette( PALET_NORMAL );
   AUTFBACK( mb_editor );

   {
      QMenu * new_menu = mb_editor->addMenu( us_tr( "&File" ) );

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

   editor->setWordWrapMode (QTextOption::WordWrap);
   editor->setMinimumHeight( minHeight1 * 3 );

   editor_widgets.push_back( editor );

   // ------ mode controls section

   lbl_modes = new QLabel( us_tr( "Plot mode:" ) );
   lbl_modes->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_modes->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_modes );
   lbl_modes->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   bg_modes = new QButtonGroup( this );

   rb_mode_iqit = new QRadioButton( "Data", this ); 
   rb_mode_iqit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   rb_mode_iqit->setMinimumHeight(minHeight3);
   rb_mode_iqit->setPalette( PALET_NORMAL );
   AUTFBACK( rb_mode_iqit );
   connect(rb_mode_iqit, SIGNAL(clicked( )), SLOT( set_mode_iqit( )));

   rb_mode_svd = new QRadioButton( "SVD", this ); 
   rb_mode_svd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   rb_mode_svd->setMinimumHeight(minHeight3);
   rb_mode_svd->setPalette( PALET_NORMAL );
   AUTFBACK( rb_mode_svd );
   connect(rb_mode_svd, SIGNAL(clicked( )), SLOT( set_mode_svd( )));

   rb_mode_efa = new QRadioButton( "EFA", this ); 
   rb_mode_efa->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   rb_mode_efa->setMinimumHeight(minHeight3);
   rb_mode_efa->setPalette( PALET_NORMAL );
   AUTFBACK( rb_mode_efa );
   connect(rb_mode_efa, SIGNAL(clicked( )), SLOT( set_mode_efa( )));

   rb_mode_efa_decomp = new QRadioButton( "EFA Components", this ); 
   rb_mode_efa_decomp->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   rb_mode_efa_decomp->setMinimumHeight(minHeight3);
   rb_mode_efa_decomp->setPalette( PALET_NORMAL );
   AUTFBACK( rb_mode_efa_decomp );
   connect(rb_mode_efa_decomp, SIGNAL(clicked( )), SLOT( set_mode_efa_decomp( )));

   // ------ plot section
   // ----------- plot data

//   plot_data = new QwtPlot(this);
   usp_plot_data = new US_Plot( plot_data, "", "", "", this );
   connect( (QWidget *)plot_data->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_data( const QPoint & ) ) );
   ((QWidget *)plot_data->titleLabel())->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_data->axisWidget( QwtPlot::yLeft ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_data( const QPoint & ) ) );
   ((QWidget *)plot_data->axisWidget( QwtPlot::yLeft ))->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_data->axisWidget( QwtPlot::xBottom ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_data( const QPoint & ) ) );
   ((QWidget *)plot_data->axisWidget( QwtPlot::xBottom ))->setContextMenuPolicy( Qt::CustomContextMenu );
   grid_data = new QwtPlotGrid;
   grid_data->enableXMin( true );
   grid_data->enableYMin( true );
   plot_data->setPalette( PALET_NORMAL );
   AUTFBACK( plot_data );
   grid_data->setMajorPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid_data->setMinorPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid_data->attach( plot_data );
   plot_data->setAxisTitle(QwtPlot::xBottom, /* cb_guinier->isChecked() ? us_tr("q^2 (1/Angstrom^2)") : */  us_tr("q [1/Angstrom]" )); // or Time or Frame"));
   plot_data->setAxisTitle(QwtPlot::yLeft, us_tr("Intensity [a.u.] (log scale)"));
   plot_data->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_data->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_data->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
//    plot_data->setMargin(USglobal->config_list.margin);
   plot_data->setTitle("");
   plot_data->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine );
   plot_data->setCanvasBackground(USglobal->global_colors.plot);
   plot_info[ "SVD Data" ] = plot_data;

   iqit_widgets.push_back( plot_data );
   // connect( plot_data_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_data_zoomed( const QRectF & ) ) );

   // ----------- plot errors

//   plot_errors = new QwtPlot(this);
   usp_plot_errors = new US_Plot( plot_errors, "", "", "", this );
   connect( (QWidget *)plot_errors->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_errors( const QPoint & ) ) );
   ((QWidget *)plot_errors->titleLabel())->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_errors->axisWidget( QwtPlot::yLeft ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_errors( const QPoint & ) ) );
   ((QWidget *)plot_errors->axisWidget( QwtPlot::yLeft ))->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_errors->axisWidget( QwtPlot::xBottom ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_errors( const QPoint & ) ) );
   ((QWidget *)plot_errors->axisWidget( QwtPlot::xBottom ))->setContextMenuPolicy( Qt::CustomContextMenu );
   grid_errors = new QwtPlotGrid;
   grid_errors->enableXMin( true );
   grid_errors->enableYMin( true );
   plot_errors->setPalette( PALET_NORMAL );
   AUTFBACK( plot_errors );
   grid_errors->setMajorPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid_errors->setMinorPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid_errors->attach( plot_errors );
   // plot_errors->setAxisTitle(QwtPlot::xBottom, /* cb_guinier->isChecked() ? us_tr("q^2 (1/Angstrom^2)") : */  us_tr("q [1/Angstrom]" )); // or Time or Frame"));
   plot_errors->setAxisTitle(QwtPlot::yLeft, us_tr("Delta Intensity [a.u.]"));
   plot_errors->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_errors->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_errors->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
//    plot_errors->setMargin(USglobal->config_list.margin);
   plot_errors->setTitle("");
   plot_errors->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine );
   plot_errors->setCanvasBackground(USglobal->global_colors.plot);

   // connect( plot_errors_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_errors_zoomed( const QRectF & ) ) );
   errors_widgets.push_back( plot_errors );
   plot_info[ "SVD Errors" ] = plot_errors;

   cb_plot_errors = new QCheckBox(this);
   cb_plot_errors->setText(us_tr("Residuals "));
   cb_plot_errors->setEnabled( true );
   cb_plot_errors->setChecked( false );
   cb_plot_errors->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_plot_errors->setPalette( PALET_NORMAL );
   AUTFBACK( cb_plot_errors );
   connect( cb_plot_errors, SIGNAL( clicked() ), SLOT( set_plot_errors() ) );
   // errors_widgets.push_back( cb_plot_errors );

   cb_plot_errors_rev = new QCheckBox(this);
   cb_plot_errors_rev->setText(us_tr("Reverse "));
   cb_plot_errors_rev->setEnabled( true );
   cb_plot_errors_rev->setChecked( false );
   cb_plot_errors_rev->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_plot_errors_rev->setPalette( PALET_NORMAL );
   AUTFBACK( cb_plot_errors_rev );
   connect( cb_plot_errors_rev, SIGNAL( clicked() ), SLOT( set_plot_errors_rev() ) );
   errors_widgets.push_back( cb_plot_errors_rev );

   cb_plot_errors_sd = new QCheckBox(this);
   cb_plot_errors_sd->setText(us_tr("Use SDs "));
   cb_plot_errors_sd->setEnabled( true );
   cb_plot_errors_sd->setChecked( false );
   cb_plot_errors_sd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_plot_errors_sd->setPalette( PALET_NORMAL );
   AUTFBACK( cb_plot_errors_sd );
   connect( cb_plot_errors_sd, SIGNAL( clicked() ), SLOT( set_plot_errors_sd() ) );
   errors_widgets.push_back( cb_plot_errors_sd );

   cb_plot_errors_pct = new QCheckBox(this);
   cb_plot_errors_pct->setText(us_tr("By percent "));
   cb_plot_errors_pct->setEnabled( true );
   cb_plot_errors_pct->setChecked( false );
   cb_plot_errors_pct->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_plot_errors_pct->setPalette( PALET_NORMAL );
   AUTFBACK( cb_plot_errors_pct );
   connect( cb_plot_errors_pct, SIGNAL( clicked() ), SLOT( set_plot_errors_pct() ) );
   errors_widgets.push_back( cb_plot_errors_pct );

   cb_plot_errors_ref = new QCheckBox(this);
   cb_plot_errors_ref->setText(us_tr("Reference "));
   cb_plot_errors_ref->setEnabled( true );
   cb_plot_errors_ref->setChecked( false );
   cb_plot_errors_ref->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_plot_errors_ref->setPalette( PALET_NORMAL );
   AUTFBACK( cb_plot_errors_ref );
   connect( cb_plot_errors_ref, SIGNAL( clicked() ), SLOT( set_plot_errors_ref() ) );
   errors_widgets.push_back( cb_plot_errors_ref );

   cb_plot_errors_group = new QCheckBox(this);
   cb_plot_errors_group->setText(us_tr("Group"));
   cb_plot_errors_group->setEnabled( true );
   cb_plot_errors_group->setChecked( false );
   cb_plot_errors_group->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_plot_errors_group->setPalette( PALET_NORMAL );
   AUTFBACK( cb_plot_errors_group );
   connect( cb_plot_errors_group, SIGNAL( clicked() ), SLOT( set_plot_errors_group() ) );
   errors_widgets.push_back( cb_plot_errors_group );

   // ----------- plot svd

//   plot_svd = new QwtPlot(this);
   usp_plot_svd = new US_Plot( plot_svd, "", "", "", this );
   connect( (QWidget *)plot_svd->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_svd( const QPoint & ) ) );
   ((QWidget *)plot_svd->titleLabel())->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_svd->axisWidget( QwtPlot::yLeft ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_svd( const QPoint & ) ) );
   ((QWidget *)plot_svd->axisWidget( QwtPlot::yLeft ))->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_svd->axisWidget( QwtPlot::xBottom ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_svd( const QPoint & ) ) );
   ((QWidget *)plot_svd->axisWidget( QwtPlot::xBottom ))->setContextMenuPolicy( Qt::CustomContextMenu );
   grid_data = new QwtPlotGrid;
   grid_data->enableXMin( true );
   grid_data->enableYMin( true );
   plot_svd->setPalette( PALET_NORMAL );
   AUTFBACK( plot_svd );
   grid_data->setMajorPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid_data->setMinorPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid_data->attach( plot_svd );
   plot_svd->setAxisTitle(QwtPlot::xBottom, us_tr( "Number" ) );
   plot_svd->setAxisTitle(QwtPlot::yLeft, us_tr("S.V.'s [log scale]"));
   plot_svd->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_svd->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_svd->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
//    plot_svd->setMargin(USglobal->config_list.margin);
   set_title( plot_svd, "SVD" );
   
   plot_svd->setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine(10));
   plot_svd->setCanvasBackground(USglobal->global_colors.plot);

   // connect( plot_svd_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_svd_zoomed( const QRectF & ) ) );
   plot_info[ "SVD SVD" ] = plot_svd;

   svd_widgets.push_back( plot_svd );

   // ----------- plot autocor

//   plot_ac = new QwtPlot(this);
   usp_plot_ac = new US_Plot( plot_ac, "", "", "", this );
   connect( (QWidget *)plot_ac->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_ac( const QPoint & ) ) );
   ((QWidget *)plot_ac->titleLabel())->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_ac->axisWidget( QwtPlot::yLeft ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_ac( const QPoint & ) ) );
   ((QWidget *)plot_ac->axisWidget( QwtPlot::yLeft ))->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_ac->axisWidget( QwtPlot::xBottom ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_ac( const QPoint & ) ) );
   ((QWidget *)plot_ac->axisWidget( QwtPlot::xBottom ))->setContextMenuPolicy( Qt::CustomContextMenu );
   grid_ac = new QwtPlotGrid;
   grid_ac->enableXMin( true );
   grid_ac->enableYMin( true );
   plot_ac->setPalette( PALET_NORMAL );
   AUTFBACK( plot_ac );
   grid_ac->setMajorPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid_ac->setMinorPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid_ac->attach( plot_ac );
   plot_ac->setAxisTitle(QwtPlot::xBottom, us_tr( "Number" ) );
   plot_ac->setAxisTitle(QwtPlot::yLeft, us_tr("Absolute value"));
   plot_ac->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_ac->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_ac->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
//    plot_ac->setMargin(USglobal->config_list.margin);
   set_title( plot_ac, "Autocorrelation" );
   
   plot_ac->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine );
   plot_ac->setCanvasBackground(USglobal->global_colors.plot);
   {
      QwtLegend* legend_pd = new QwtLegend;
      legend_pd->setFrameStyle( QFrame::Box | QFrame::Sunken );
      plot_ac->insertLegend( legend_pd, QwtPlot::BottomLegend );
   }
   // connect( plot_ac_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_ac_zoomed( const QRectF & ) ) );
   plot_info[ "SVD Autocorrelation" ] = plot_ac;

   svd_widgets.push_back( plot_ac );

   // ----------- plot efa

//   plot_lefa = new QwtPlot(this);
   usp_plot_lefa = new US_Plot( plot_lefa, "", "", "", this );
   connect( (QWidget *)plot_lefa->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_lefa( const QPoint & ) ) );
   ((QWidget *)plot_lefa->titleLabel())->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_lefa->axisWidget( QwtPlot::yLeft ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_lefa( const QPoint & ) ) );
   ((QWidget *)plot_lefa->axisWidget( QwtPlot::yLeft ))->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_lefa->axisWidget( QwtPlot::xBottom ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_lefa( const QPoint & ) ) );
   ((QWidget *)plot_lefa->axisWidget( QwtPlot::xBottom ))->setContextMenuPolicy( Qt::CustomContextMenu );
   grid_lefa = new QwtPlotGrid;
   grid_lefa->enableXMin( true );
   grid_lefa->enableYMin( true );
   plot_lefa->setPalette( PALET_NORMAL );
   AUTFBACK( plot_lefa );
   grid_lefa->setMajorPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid_lefa->setMinorPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid_lefa->attach( plot_lefa );
   plot_lefa->setAxisTitle(QwtPlot::xBottom, us_tr("Time [a.u.]") );
   plot_lefa->setAxisTitle(QwtPlot::yLeft, us_tr("Singular Value"));
   plot_lefa->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_lefa->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_lefa->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
//    plot_lefa->setMargin(USglobal->config_list.margin);
   set_title( plot_lefa, us_tr( "Forward EFA" ) );
   plot_lefa->setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine(10));
   plot_lefa->setCanvasBackground(USglobal->global_colors.plot);

   // connect( plot_lefa_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_lefa_zoomed( const QRectF & ) ) );
   efa_widgets.push_back( plot_lefa );
   plot_info[ "SVD Forward EFA" ] = plot_lefa;

//   plot_refa = new QwtPlot(this);
   usp_plot_refa = new US_Plot( plot_refa, "", "", "", this );
   connect( (QWidget *)plot_refa->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_refa( const QPoint & ) ) );
   ((QWidget *)plot_refa->titleLabel())->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_refa->axisWidget( QwtPlot::yLeft ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_refa( const QPoint & ) ) );
   ((QWidget *)plot_refa->axisWidget( QwtPlot::yLeft ))->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_refa->axisWidget( QwtPlot::xBottom ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_refa( const QPoint & ) ) );
   ((QWidget *)plot_refa->axisWidget( QwtPlot::xBottom ))->setContextMenuPolicy( Qt::CustomContextMenu );
   grid_refa = new QwtPlotGrid;
   grid_refa->enableXMin( true );
   grid_refa->enableYMin( true );
   plot_refa->setPalette( PALET_NORMAL );
   AUTFBACK( plot_refa );
   grid_refa->setMajorPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid_refa->setMinorPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid_refa->attach( plot_refa );
   plot_refa->setAxisTitle(QwtPlot::xBottom, us_tr("Time [a.u.]") );
   plot_refa->setAxisTitle(QwtPlot::yLeft, us_tr("Singular Value"));
   plot_refa->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_refa->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_refa->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
//    plot_refa->setMargin(USglobal->config_list.margin);

   set_title( plot_refa, us_tr( "Backward EFA" ) );
   plot_refa->setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine(10));
   plot_refa->setCanvasBackground(USglobal->global_colors.plot);

   // connect( plot_refa_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_refa_zoomed( const QRectF & ) ) );
   efa_widgets.push_back( plot_refa );
   
   lbl_efas = new QLabel( us_tr( "Number of SV's for EFA plots and decomposition:" ), this );
   lbl_efas->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_efas->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_efas );
   lbl_efas->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   efa_widgets.push_back( lbl_efas );
   svd_widgets.push_back( lbl_efas );
   plot_info[ "SVD Backward EFA" ] = plot_refa;

   qwtc_efas = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( qwtc_efas );
   qwtc_efas->setEnabled(true);
   qwtc_efas->setNumButtons(1);
   qwtc_efas->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   qwtc_efas->setPalette( PALET_NORMAL );
   qwtc_efas->setSingleStep( 1 );
   qwtc_efas->setValue( 1 );
   
   AUTFBACK( qwtc_efas );
   connect( qwtc_efas, SIGNAL( valueChanged( double ) ), SLOT( update_efas( double ) ) );

   efa_widgets.push_back( qwtc_efas );
   svd_widgets.push_back( qwtc_efas );

   for ( int i = 0; i < UHSHS_MAX_SV_FOR_EFA; ++i ) {
      {
         QLabel * tmp_lbl = new QLabel( QString( us_tr( "S.V. %1 Start:" ) ).arg( i + 1 ), this );
         tmp_lbl->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
         tmp_lbl->setPalette( PALET_NORMAL );
         AUTFBACK( tmp_lbl );
         tmp_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
         tmp_lbl->hide();
         efa_range_labels.push_back( tmp_lbl );
      }
      
      {
         QwtCounter * tmp_qwtc = new QwtCounter( this );
         US_Hydrodyn::sizeArrows( tmp_qwtc );
         tmp_qwtc->setEnabled(true);
         tmp_qwtc->setNumButtons( 2 );
         tmp_qwtc->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
         tmp_qwtc->setPalette( PALET_NORMAL );
         tmp_qwtc->setSingleStep( 1 );
         efa_range_start.push_back( tmp_qwtc );
         AUTFBACK( tmp_qwtc );
         tmp_qwtc->hide();
         connect( tmp_qwtc, SIGNAL( valueChanged( double ) ), SLOT( update_efa_range_start( double ) ) );
      }

      {
         QLabel * tmp_lbl = new QLabel( QString( us_tr( "    End:" ) ), this );
         tmp_lbl->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
         tmp_lbl->setPalette( PALET_NORMAL );
         AUTFBACK( tmp_lbl );
         tmp_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
         tmp_lbl->hide();
         efa_range_labels2.push_back( tmp_lbl );
      }

      {
         QwtCounter * tmp_qwtc = new QwtCounter( this );
         US_Hydrodyn::sizeArrows( tmp_qwtc );
         tmp_qwtc->setEnabled(true);
         tmp_qwtc->setNumButtons( 2 );
         tmp_qwtc->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
         tmp_qwtc->setPalette( PALET_NORMAL );
         tmp_qwtc->setSingleStep( 1 );
         efa_range_end.push_back( tmp_qwtc );
         AUTFBACK( tmp_qwtc );
         tmp_qwtc->hide();
         connect( tmp_qwtc, SIGNAL( valueChanged( double ) ), SLOT( update_efa_range_end( double ) ) );
      }
   }

   // ----------- plot efa decomp

//   plot_efa_decomp = new QwtPlot(this);
   usp_plot_efa_decomp = new US_Plot( plot_efa_decomp, "", "", "", this );
   connect( (QWidget *)plot_efa_decomp->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_efa_decomp( const QPoint & ) ) );
   ((QWidget *)plot_efa_decomp->titleLabel())->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_efa_decomp->axisWidget( QwtPlot::yLeft ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_efa_decomp( const QPoint & ) ) );
   ((QWidget *)plot_efa_decomp->axisWidget( QwtPlot::yLeft ))->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_efa_decomp->axisWidget( QwtPlot::xBottom ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_efa_decomp( const QPoint & ) ) );
   ((QWidget *)plot_efa_decomp->axisWidget( QwtPlot::xBottom ))->setContextMenuPolicy( Qt::CustomContextMenu );
   grid_efa_decomp = new QwtPlotGrid;
   grid_efa_decomp->enableXMin( true );
   grid_efa_decomp->enableYMin( true );
   plot_efa_decomp->setPalette( PALET_NORMAL );
   AUTFBACK( plot_efa_decomp );
   grid_efa_decomp->setMajorPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid_efa_decomp->setMinorPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid_efa_decomp->attach( plot_efa_decomp );
   plot_efa_decomp->setAxisTitle(QwtPlot::xBottom, us_tr( "Frame" ) );
   plot_efa_decomp->setAxisTitle(QwtPlot::yLeft, us_tr("Intensity [a.u.]"));
   plot_efa_decomp->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_efa_decomp->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_efa_decomp->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
//    plot_efa_decomp->setMargin(USglobal->config_list.margin);
   set_title( plot_efa_decomp, "EFA Deconvolution" );
   
   plot_efa_decomp->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine );
   plot_efa_decomp->setCanvasBackground(USglobal->global_colors.plot);

   // connect( plot_efa_decomp_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_efa_decomp_zoomed( const QRectF & ) ) );
   plot_info[ "SVD Autocorrelation" ] = plot_efa_decomp;

   efa_decomp_widgets.push_back( plot_efa_decomp );

   cb_efa_decomp_force_positive = new QCheckBox(this);
   cb_efa_decomp_force_positive->setText(us_tr("Force positive"));
   cb_efa_decomp_force_positive->setEnabled( true );
   cb_efa_decomp_force_positive->setChecked( false );
   cb_efa_decomp_force_positive->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_efa_decomp_force_positive->setPalette( PALET_NORMAL );
   AUTFBACK( cb_efa_decomp_force_positive );
   connect( cb_efa_decomp_force_positive, SIGNAL( clicked() ), SLOT( set_efa_decomp_force_positive() ) );
   efa_decomp_widgets.push_back( cb_efa_decomp_force_positive );

   pb_efa_decomp_to_mals_saxs_saxs = new QPushButton(us_tr("To HPLC window"), this);
   pb_efa_decomp_to_mals_saxs_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_efa_decomp_to_mals_saxs_saxs->setMinimumHeight(minHeight1);
   pb_efa_decomp_to_mals_saxs_saxs->setPalette( PALET_PUSHB );
   connect(pb_efa_decomp_to_mals_saxs_saxs, SIGNAL(clicked()), SLOT(efa_decomp_to_mals_saxs_saxs()));
   efa_decomp_widgets.push_back( pb_efa_decomp_to_mals_saxs_saxs );

   iq_it_state = mode_i_of_t;
   pb_iq_it = new QPushButton( us_tr( mode_i_of_t ? "Show I(q)" : "Show I(t)" ), this);
   pb_iq_it->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_iq_it->setMinimumHeight(minHeight1);
   pb_iq_it->setPalette( PALET_PUSHB );
   connect(pb_iq_it, SIGNAL(clicked()), SLOT(iq_it()));
   iqit_widgets.push_back( pb_iq_it );

   pb_axis_x = new QPushButton(us_tr("X"), this);
   pb_axis_x->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_axis_x->setMinimumHeight(minHeight1);
   pb_axis_x->setPalette( PALET_PUSHB );
   connect(pb_axis_x, SIGNAL(clicked()), SLOT(axis_x()));
   iqit_widgets.push_back( pb_axis_x );

   pb_axis_y = new QPushButton(us_tr("Y"), this);
   pb_axis_y->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_axis_y->setMinimumHeight(minHeight1);
   pb_axis_y->setPalette( PALET_PUSHB );
   connect(pb_axis_y, SIGNAL(clicked()), SLOT(axis_y()));
   iqit_widgets.push_back( pb_axis_y );

   // ------ process section 

   lbl_process = new mQLabel("Process", this);
   lbl_process->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_process->setMinimumHeight(minHeight1);
   lbl_process->setPalette( PALET_LABEL );
   AUTFBACK( lbl_process );
   lbl_process->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   connect( lbl_process, SIGNAL( pressed() ), SLOT( hide_process() ) );

   cb_norm_pw = new QCheckBox(this);
   cb_norm_pw->setText(us_tr("Normalize by errors pointwise "));
   cb_norm_pw->setEnabled( true );
   cb_norm_pw->setChecked( false );
   cb_norm_pw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_norm_pw->setPalette( PALET_NORMAL );
   AUTFBACK( cb_norm_pw );
   connect( cb_norm_pw, SIGNAL( clicked() ), SLOT( set_norm_pw() ) );
   if ( norm_ok ) {
      process_widgets.push_back( cb_norm_pw );
   } else {
      cb_norm_pw->hide();
   }

   cb_norm_avg = new QCheckBox(this);
   cb_norm_avg->setText(us_tr("Normalize by average errors "));
   cb_norm_avg->setEnabled( true );
   cb_norm_avg->setChecked( false );
   cb_norm_avg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_norm_avg->setPalette( PALET_NORMAL );
   AUTFBACK( cb_norm_avg );
   connect( cb_norm_avg, SIGNAL( clicked() ), SLOT( set_norm_avg() ) );
   if ( norm_ok ) {
      cb_norm_avg->setChecked( true );
      process_widgets.push_back( cb_norm_avg );
   } else {
      cb_norm_avg->hide();
   }

   lbl_q_range = new QLabel( us_tr( "Active q range:" ), this );
   lbl_q_range->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_q_range->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_q_range );
   lbl_q_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   // process_widgets.push_back( lbl_q_range );
   lbl_q_range->hide();

   le_q_start = new mQLineEdit( this );    le_q_start->setObjectName( "le_q_start Line Edit" );
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

   le_q_end = new mQLineEdit( this );    le_q_end->setObjectName( "le_q_end Line Edit" );
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

   //    lbl_t_range = new QLabel( us_tr( "Active Time range:" ), this );
   //    lbl_t_range->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   //    lbl_t_range->setPalette( PALET_NORMAL );
   //    lbl_t_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   //    process_widgets.push_back( lbl_t_range );

   //    le_t_start = new mQLineEdit( this );     le_t_start->setObjectName( "le_t_start Line Edit" );
   //    le_t_start->setText( "" );
   //    le_t_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   //    le_t_start->setPalette( PALET_NORMAL );
   //    le_t_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   //    le_t_start->setEnabled( false );
   //    le_t_start->setValidator( new QDoubleValidator( le_t_start ) );
   //    connect( le_t_start, SIGNAL( textChanged( const QString & ) ), SLOT( t_start_text( const QString & ) ) );
   //    process_widgets.push_back( le_t_start );

   //    le_t_end = new mQLineEdit( this );     le_t_end->setObjectName( "le_t_end Line Edit" );
   //    le_t_end->setText( "" );
   //    le_t_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   //    le_t_end->setPalette( PALET_NORMAL );
   //    le_t_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   //    le_t_end->setEnabled( false );
   //    le_t_end->setValidator( new QDoubleValidator( le_t_end ) );
   //    connect( le_t_end, SIGNAL( textChanged( const QString & ) ), SLOT( t_end_text( const QString & ) ) );
   //    process_widgets.push_back( le_t_end );

   lbl_ev = new QLabel( us_tr( "Singular value list:" ), this );
   lbl_ev->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_ev->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_ev );
   lbl_ev->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   process_widgets.push_back( lbl_ev );

   lb_ev = new QListWidget( this );
   lb_ev->setPalette( PALET_NORMAL );
   AUTFBACK( lb_ev );
   lb_ev->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lb_ev->setEnabled(true);
   lb_ev->setSelectionMode( QAbstractItemView::ExtendedSelection );
   // lb_ev->setColumnMode   ( QListBox::FitToWidth );
   connect( lb_ev, SIGNAL( itemSelectionChanged() ), SLOT( sv_selection_changed() ) );
   process_widgets.push_back( lb_ev );

   pb_svd = new QPushButton(us_tr("Compute SVD"), this);
   pb_svd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_svd->setMinimumHeight(minHeight3);
   pb_svd->setPalette( PALET_PUSHB );
   pb_svd->setToolTip( us_tr( "SVD is only available in I(q) mode, adjust the plot type to use." ) );
   connect(pb_svd, SIGNAL(clicked()), SLOT(svd()));
   process_widgets.push_back( pb_svd );

   pb_efa = new QPushButton(us_tr("Compute EFA"), this);
   pb_efa->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_efa->setMinimumHeight(minHeight3);
   pb_efa->setPalette( PALET_PUSHB );
   connect(pb_efa, SIGNAL(clicked()), SLOT(efa()));
   process_widgets.push_back( pb_efa );

   pb_efa_decomp = new QPushButton(us_tr("Compute EFA Components"), this);
   pb_efa_decomp->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_efa_decomp->setMinimumHeight(minHeight3);
   pb_efa_decomp->setPalette( PALET_PUSHB );
   connect(pb_efa_decomp, SIGNAL(clicked()), SLOT(efa_decomp()));
   process_widgets.push_back( pb_efa_decomp );

   pb_stop = new QPushButton(us_tr("Stop"), this);
   pb_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_stop->setMinimumHeight(minHeight3);
   pb_stop->setPalette( PALET_PUSHB );
   connect(pb_stop, SIGNAL(clicked()), SLOT(stop()));
   process_widgets.push_back( pb_stop );

   // pb_svd_plot = new QPushButton(us_tr("Plot SVs"), this);
   // pb_svd_plot->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   // pb_svd_plot->setMinimumHeight(minHeight3);
   // pb_svd_plot->setPalette( PALET_PUSHB );
   // connect(pb_svd_plot, SIGNAL(clicked()), SLOT(svd_plot()));
   // process_widgets.push_back( pb_svd_plot );

   pb_svd_save = new QPushButton(us_tr("Save SVs"), this);
   pb_svd_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_svd_save->setMinimumHeight(minHeight3);
   pb_svd_save->setPalette( PALET_PUSHB );
   connect(pb_svd_save, SIGNAL(clicked()), SLOT(svd_save()));
   process_widgets.push_back( pb_svd_save );

   pb_recon = new QPushButton(us_tr("TSVD reconstruction"), this);
   pb_recon->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_recon->setMinimumHeight(minHeight3);
   pb_recon->setPalette( PALET_PUSHB );
   pb_recon->setToolTip( us_tr( "Reconstruction is only enabled when all normalization off and SVs are selected" ) );
   connect(pb_recon, SIGNAL(clicked()), SLOT(recon()));
   process_widgets.push_back( pb_recon );

   pb_indiv_recon = new QPushButton(us_tr("Individual TSVD recon."), this);
   pb_indiv_recon->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_indiv_recon->setMinimumHeight(minHeight3);
   pb_indiv_recon->setPalette( PALET_PUSHB );
   pb_indiv_recon->setToolTip( us_tr( "Reconstruction is only enabled when all normalization off and SVs are selected" ) );
   connect(pb_indiv_recon, SIGNAL(clicked()), SLOT(indiv_recon()));
   process_widgets.push_back( pb_indiv_recon );

   pb_inc_recon = new QPushButton(us_tr("Incremental TSVD recon."), this);
   pb_inc_recon->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_inc_recon->setMinimumHeight(minHeight3);
   pb_inc_recon->setPalette( PALET_PUSHB );
   pb_inc_recon->setToolTip( us_tr( "Reconstruction is only enabled when all normalization off and SVs are selected" ) );
   connect(pb_inc_recon, SIGNAL(clicked()), SLOT(inc_recon()));
   process_widgets.push_back( pb_inc_recon );

   pb_inc_chi_plot = new QPushButton(us_tr("Plot Chi"), this);
   pb_inc_chi_plot->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_inc_chi_plot->setMinimumHeight(minHeight3);
   pb_inc_chi_plot->setPalette( PALET_PUSHB );
   connect(pb_inc_chi_plot, SIGNAL(clicked()), SLOT(inc_chi_plot()));
   // process_widgets.push_back( pb_inc_chi_plot );
   pb_inc_chi_plot->hide();

   pb_inc_rmsd_plot = new QPushButton(us_tr("Plot RMSDs"), this);
   pb_inc_rmsd_plot->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_inc_rmsd_plot->setMinimumHeight(minHeight3);
   pb_inc_rmsd_plot->setPalette( PALET_PUSHB );
   connect(pb_inc_rmsd_plot, SIGNAL(clicked()), SLOT(inc_rmsd_plot()));
   process_widgets.push_back( pb_inc_rmsd_plot );

   pb_rmsd_save = new QPushButton(us_tr("Save RMSDs"), this);
   pb_rmsd_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_rmsd_save->setMinimumHeight(minHeight3);
   pb_rmsd_save->setPalette( PALET_PUSHB );
   connect(pb_rmsd_save, SIGNAL(clicked()), SLOT(rmsd_save()));
   process_widgets.push_back( pb_rmsd_save );

   // -------- bottom section

   pb_help = new QPushButton(us_tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   progress = new QProgressBar( this );
   // progress->setMinimumHeight(minHeight1);
   progress->setPalette( PALET_NORMAL );
   AUTFBACK( progress );
   progress->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   progress->reset();

   pb_cancel = new QPushButton(us_tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // -------- build layout


   QVBoxLayout * background = new QVBoxLayout(this);
   background->setContentsMargins( 0, 0, 0, 0 );
   background->setSpacing( 0 );

   QHBoxLayout * top = new QHBoxLayout( 0 );
   top->setContentsMargins( 0, 0, 0, 0 );
   top->setSpacing( 0 );

   // ----- left side
   {
      QBoxLayout * bl = new QVBoxLayout( 0 ); bl->setContentsMargins( 0, 0, 0, 0 ); bl->setSpacing( 0 );
      bl->addWidget( lbl_data );
      bl->addWidget( lv_data );

      {
         QBoxLayout * bl_buttons = new QHBoxLayout(); bl_buttons->setContentsMargins( 0, 0, 0, 0 ); bl_buttons->setSpacing( 0 );
         bl_buttons->addWidget( pb_clear );
         bl_buttons->addWidget( pb_replot );
         bl->addLayout( bl_buttons );
      }
      {
         QBoxLayout * bl_buttons = new QHBoxLayout(); bl_buttons->setContentsMargins( 0, 0, 0, 0 ); bl_buttons->setSpacing( 0 );
         bl_buttons->addWidget( pb_to_mals_saxs );
         bl_buttons->addWidget( pb_save_plots );
         bl_buttons->addWidget( pb_color_rotate );
         bl->addLayout( bl_buttons );
      }

      bl->addWidget( lbl_process );
      {
         QGridLayout * gl = new QGridLayout( 0 ); gl->setContentsMargins( 0, 0, 0, 0 ); gl->setSpacing( 0 );
         
         gl->addWidget( lbl_q_range  , 0, 0 );
         gl->addWidget( le_q_start   , 0, 1 );
         gl->addWidget( le_q_end     , 0, 2 );

         // gl->addWidget( lbl_t_range  , 1, 0 );
         // gl->addWidget( le_t_start   , 1, 1 );
         // gl->addWidget( le_t_end     , 1, 2 );

         // gl->addWidget( lbl_ev       , 3, 0 );
         // gl->addWidget( lb_ev         , 3 , 1 , 1 + ( 3 ) - ( 3 ) , 1 + ( 2  ) - ( 1 ) );

         bl->addLayout( gl );
      }

      {
         QBoxLayout * bl_buttons = new QHBoxLayout(); bl_buttons->setContentsMargins( 0, 0, 0, 0 ); bl_buttons->setSpacing( 0 );
         bl_buttons->addWidget( cb_norm_pw );
         bl_buttons->addWidget( cb_norm_avg );
         bl->addLayout( bl_buttons );
      }

      {
         QBoxLayout * bl_buttons = new QHBoxLayout(); bl_buttons->setContentsMargins( 0, 0, 0, 0 ); bl_buttons->setSpacing( 0 );
         bl_buttons->addWidget( pb_svd );
         bl_buttons->addWidget( pb_efa );
         bl_buttons->addWidget( pb_efa_decomp );
         bl_buttons->addWidget( pb_stop );
         bl->addLayout( bl_buttons );
      }

      bl->addWidget( lbl_ev );
      bl->addWidget( lb_ev );

      {
         QBoxLayout * bl_buttons = new QHBoxLayout(); bl_buttons->setContentsMargins( 0, 0, 0, 0 ); bl_buttons->setSpacing( 0 );
         // bl_buttons->addWidget( pb_svd_plot );
         bl_buttons->addWidget( pb_svd_save );
         bl_buttons->addWidget( pb_recon );
         bl->addLayout( bl_buttons );
      }

      {
         QBoxLayout * bl_buttons = new QHBoxLayout(); bl_buttons->setContentsMargins( 0, 0, 0, 0 ); bl_buttons->setSpacing( 0 );
         bl_buttons->addWidget( pb_indiv_recon );
         bl_buttons->addWidget( pb_inc_recon );
         bl->addLayout( bl_buttons );
      }

      {
         QBoxLayout * bl_buttons = new QHBoxLayout(); bl_buttons->setContentsMargins( 0, 0, 0, 0 ); bl_buttons->setSpacing( 0 );
         bl_buttons->addWidget( pb_inc_chi_plot );
         bl_buttons->addWidget( pb_inc_rmsd_plot );
         bl_buttons->addWidget( pb_rmsd_save );
         bl->addLayout( bl_buttons );
      }

      bl->addWidget( lbl_editor );
#if QT_VERSION < 0x040000 || !defined(Q_OS_MAC)
      bl->addWidget( frame );
#endif
      bl->addWidget( editor );

      top->addLayout ( bl, 0 );
   }      

   // ----- right side
   {
      QBoxLayout * bl = new QVBoxLayout( 0 ); bl->setContentsMargins( 0, 0, 0, 0 ); bl->setSpacing( 0 );
      
      {
         QBoxLayout * bl_buttons = new QHBoxLayout();
         bl_buttons->setContentsMargins( 0, 0, 0, 0 );
         bl_buttons->setSpacing( 0 );
         bl_buttons->addWidget( lbl_modes );
         bl_buttons->addWidget( rb_mode_iqit );
         bl_buttons->addWidget( rb_mode_svd );
         bl_buttons->addWidget( rb_mode_efa );
         bl_buttons->addWidget( rb_mode_efa_decomp );
         bl->addLayout( bl_buttons );
      }

      bl->addWidget( plot_data );
      bl->addWidget( plot_errors );

      bl->addWidget( plot_svd );
      bl->addWidget( plot_ac );

      bl->addWidget( plot_lefa );
      bl->addWidget( plot_refa );

      {
         for ( int i = 0; i < UHSHS_MAX_SV_FOR_EFA; ++i ) {
            QBoxLayout * bl_buttons = new QHBoxLayout();
            bl_buttons->setContentsMargins( 0, 0, 0, 0 );
            bl_buttons->setSpacing( 0 );
            bl_buttons->addWidget( efa_range_labels [ i ] );
            bl_buttons->addWidget( efa_range_start  [ i ] );
            bl_buttons->addWidget( efa_range_labels2[ i ] );
            bl_buttons->addWidget( efa_range_end    [ i ] );
            bl->addLayout( bl_buttons );
         }
      }

      {
         QBoxLayout * bl_buttons = new QHBoxLayout();
         bl_buttons->setContentsMargins( 0, 0, 0, 0 );
         bl_buttons->setSpacing( 0 );

         bl_buttons->addWidget( lbl_efas );
         bl_buttons->addWidget( qwtc_efas );
         bl->addLayout( bl_buttons );
      }

      bl->addWidget( plot_efa_decomp );
      {
         QBoxLayout * bl_buttons = new QHBoxLayout();
         bl_buttons->setContentsMargins( 0, 0, 0, 0 );
         bl_buttons->setSpacing( 0 );

         bl_buttons->addWidget( cb_efa_decomp_force_positive );
         bl_buttons->addWidget( pb_efa_decomp_to_mals_saxs_saxs );
         bl->addLayout( bl_buttons );
      }

      //       {
      //          QGridLayout * gl = new QGridLayout( 0 ); gl->setContentsMargins( 0, 0, 0, 0 ); gl->setSpacing( 0 );
      //          gl -> addWidget( cb_plot_errors       , 0, 0 );
      //          gl -> addWidget( cb_plot_errors_sd    , 0, 1 );
      //          gl -> addWidget( cb_plot_errors_pct   , 0, 2 );
      //          gl -> addWidget( cb_plot_errors_rev   , 1, 0 );
      //          gl -> addWidget( cb_plot_errors_ref   , 1, 1 );
      //          gl -> addWidget( cb_plot_errors_group , 1, 2 );

      //          bl -> addLayout( gl );
      //       }

      {
         QBoxLayout * bl_buttons = new QHBoxLayout(); bl_buttons->setContentsMargins( 0, 0, 0, 0 ); bl_buttons->setSpacing( 0 );
         bl_buttons->addWidget( cb_plot_errors );
         bl_buttons->addWidget( cb_plot_errors_sd );
         bl_buttons->addWidget( cb_plot_errors_pct );
         bl->addLayout( bl_buttons );
      }

      {
         QBoxLayout * bl_buttons = new QHBoxLayout(); bl_buttons->setContentsMargins( 0, 0, 0, 0 ); bl_buttons->setSpacing( 0 );
         bl_buttons->addWidget( cb_plot_errors_rev );
         bl_buttons->addWidget( cb_plot_errors_ref );
         bl_buttons->addWidget( cb_plot_errors_group );
         bl->addLayout( bl_buttons );
      }

      {
         QBoxLayout * bl_buttons = new QHBoxLayout(); bl_buttons->setContentsMargins( 0, 0, 0, 0 ); bl_buttons->setSpacing( 0 );
         bl_buttons->addWidget( pb_iq_it );
         bl_buttons->addWidget( pb_axis_x );
         bl_buttons->addWidget( pb_axis_y );
         bl->addLayout( bl_buttons );
      }

      top->addLayout( bl, 2 );
   }
   
   background->addLayout( top );

   {
      QBoxLayout * bottom = new QHBoxLayout(); bottom->setContentsMargins( 0, 0, 0, 0 ); bottom->setSpacing( 0 );
      bottom->addWidget( pb_help );
      bottom->addWidget( progress );
      bottom->addWidget( pb_cancel );
      
      background->addSpacing( 2 );
      background->addLayout( bottom );
   }
   
   hide_widgets( data_widgets, 
                 !ush_win->gparams.count( "mals_saxs_svd_data_widgets" ) || ush_win->gparams[ "mals_saxs_svd_data_widgets" ] == "false" ? false : true );
   hide_widgets( editor_widgets, 
                 !ush_win->gparams.count( "mals_saxs_svd_editor_widgets" ) || ush_win->gparams[ "mals_saxs_svd_editor_widgets" ] == "false" ? false : true );
   hide_widgets( process_widgets,
                 !ush_win->gparams.count( "mals_saxs_svd_process_widgets" ) || ush_win->gparams[ "mals_saxs_svd_process_widgets" ] == "false" ? false : true );
   mode_select( MODE_IQIT );
}

void US_Hydrodyn_Mals_Saxs_Svd::cancel()
{
   close();
}

void US_Hydrodyn_Mals_Saxs_Svd::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo/somo_mals_saxs_svd.html");
}

void US_Hydrodyn_Mals_Saxs_Svd::closeEvent(QCloseEvent *e)
{
   e->accept();
}

void US_Hydrodyn_Mals_Saxs_Svd::clear_display()
{
   editor->clear( );
   editor->append("\n\n");
}

void US_Hydrodyn_Mals_Saxs_Svd::update_font()
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

void US_Hydrodyn_Mals_Saxs_Svd::save()
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

void US_Hydrodyn_Mals_Saxs_Svd::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->textColor();
   editor->setTextColor(color);
   editor->append(msg);
   editor->setTextColor(save_color);

   if ( !editor_widgets[ 0 ]->isVisible() && color == "red" && !msg.trimmed().isEmpty() )
   {
      lbl_editor->setPalette( cg_red );
   }
}

void US_Hydrodyn_Mals_Saxs_Svd::plot_files()
{
   // puts( "plot_files" );
   plot_data->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_data->detachItems( QwtPlotItem::Rtti_PlotMarker );;
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

   QStringList files = selected_files();

   for ( int i = 0; i < (int) files.size(); ++i ) {
      if ( plot_file( files[ i ], file_minx, file_maxx, file_miny, file_maxy ) ) {
         if ( first ) {
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
   
   if ( !plot_data_zoomer ) {
      // puts( "redoing zoomer" );
      plot_data->setAxisScale( QwtPlot::xBottom, minx, maxx );
      plot_data->setAxisScale( QwtPlot::yLeft  , miny * 0.9e0 , maxy * 1.1e0 );
      plot_data_zoomer = new ScrollZoomer(plot_data->canvas());
      plot_data_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
      plot_data_zoomer->setTrackerPen(QPen(Qt::red));
      // connect( plot_data_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_data_zoomed( const QRectF & ) ) );
   }
   
   // plot_data->replot();
   rescale( false );
   update_plot_errors();
}

void US_Hydrodyn_Mals_Saxs_Svd::plot_files( QStringList add_files, norms norm_mode )
{
   // puts( "plot_files" );
   plot_data->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_data->detachItems( QwtPlotItem::Rtti_PlotMarker );;
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
      if ( plot_file( add_files[ i ], file_minx, file_maxx, file_miny, file_maxy, norm_mode ) )
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
      // connect( plot_data_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_data_zoomed( const QRectF & ) ) );
   }
   
   // plot_data->replot();
   rescale();
   // update_plot_errors();
}

bool US_Hydrodyn_Mals_Saxs_Svd::plot_file( QString file,
                                           double &minx,
                                           double &maxx,
                                           double &miny,
                                           double &maxy,
                                           norms norm_mode )
{
   map < QString, vector < double > >  * f_Iuse = &f_Is;

   switch( norm_mode ) {
   case NORM_PW :
      f_Iuse = &f_Is_norm_pw;
      break;

   case NORM_AVG :
      f_Iuse = &f_Is_norm_avg;
      break;

   default :
      break;
   }

   if ( !f_qs_string .count( file ) ||
        !f_qs        .count( file ) ||
        !(*f_Iuse)   .count( file ) ||
        !f_pos       .count( file ) )
   {
      editor_msg( "red", QString( us_tr( "Internal error: request to plot %1, but not found in data" ) ).arg( file ) );
      return false;
   }

   get_min_max( file, minx, maxx, miny, maxy, norm_mode );

   QwtPlotCurve *curve = new QwtPlotCurve( file );
   plotted_curves[ file ] = curve;
   curve->setStyle( QwtPlotCurve::Lines );

   unsigned int q_points = f_qs[ file ].size();

   if ( !axis_y_log )
   {
      curve->setSamples(
                     (double *)&( f_qs[ file ][ 0 ] ),
                     (double *)&( (*f_Iuse)[ file ][ 0 ] ),
                     q_points
                     );

      curve->setPen( QPen( plot_colors[ f_pos[ file ] % plot_colors.size() ], use_line_width, Qt::SolidLine ) );
      curve->attach( plot_data );
   } else {
      vector < double > q;
      vector < double > I;
      for ( unsigned int i = 0; i < q_points; i++ )
      {
         if ( (*f_Iuse)[ file ][ i ] > 0e0 )
         {
            q.push_back( f_qs[ file ][ i ] );
            I.push_back( (*f_Iuse)[ file ][ i ] );
         }
      }
      q_points = ( unsigned int )q.size();
      curve->setSamples(
                     /* cb_guinier->isChecked() ?
                        (double *)&(plotted_q2[p][0]) : */
                     (double *)&( q[ 0 ] ),
                     (double *)&( I[ 0 ] ),
                     q_points
                     );

      curve->setPen( QPen( plot_colors[ f_pos[ file ] % plot_colors.size() ], use_line_width, Qt::SolidLine ) );
      curve->attach( plot_data );
   }
   return true;
}

bool US_Hydrodyn_Mals_Saxs_Svd::get_min_max( QString file,
                                             double &minx,
                                             double &maxx,
                                             double &miny,
                                             double &maxy,
                                             norms norm_mode )
{
   map < QString, vector < double > >  * f_Iuse = &f_Is;

   switch( norm_mode ) {
   case NORM_PW :
      f_Iuse = &f_Is_norm_pw;
      break;

   case NORM_AVG :
      f_Iuse = &f_Is_norm_avg;
      break;

   default :
      break;
   }

   if ( 
       !(*f_Iuse)   .count( file ) ||
       !f_pos       .count( file ) )
   {
      // editor_msg( "red", QString( us_tr( "Internal error: requested %1, but not found in data" ) ).arg( file ) );
      return false;
   }

   minx = f_qs[ file ][ 0 ];
   maxx = f_qs[ file ].back();

   miny = (*f_Iuse)[ file ][ 0 ];
   maxy = (*f_Iuse)[ file ][ 0 ];

   if ( axis_y_log )
   {
      unsigned int i = 0;
      while ( miny <= 0e0 && i < (*f_Iuse)[ file ].size() )
      {
         miny = (*f_Iuse)[ file ][ i ];
         maxy = (*f_Iuse)[ file ][ i ];
         minx = f_qs[ file ][ i ];
         maxx = f_qs[ file ][ i ];
         i++;
      }
      for ( ; i < (*f_Iuse)[ file ].size(); i++ )
      {
         if ( miny > (*f_Iuse)[ file ][ i ] && (*f_Iuse)[ file ][ i ] > 0e0 )
         {
            miny = (*f_Iuse)[ file ][ i ];
         }
         if ( maxy < (*f_Iuse)[ file ][ i ] )
         {
            maxy = (*f_Iuse)[ file ][ i ];
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
      for ( unsigned int i = 1; i < (*f_Iuse)[ file ].size(); i++ )
      {
         if ( miny > (*f_Iuse)[ file ][ i ] )
         {
            miny = (*f_Iuse)[ file ][ i ];
         }
         if ( maxy < (*f_Iuse)[ file ][ i ] )
         {
            maxy = (*f_Iuse)[ file ][ i ];
         }
      }
   }

   return true;
}

// void US_Hydrodyn_Mals_Saxs_Svd::plot_data_zoomed( const QRectF & /* rect */ )
// {
//    //   cout << QString( "zoomed: %1 %2 %3 %4\n" )
//    // .arg( rect.x1() )
//    // .arg( rect.x2() )
//    // .arg( rect.y1() )
//    // .arg( rect.y2() );
// }

// void US_Hydrodyn_Mals_Saxs_Svd::plot_errors_zoomed( const QRectF & /* rect */ )
// {
//    //   cout << QString( "zoomed: %1 %2 %3 %4\n" )
//    // .arg( rect.x1() )
//    // .arg( rect.x2() )
//    // .arg( rect.y1() )
//    // .arg( rect.y2() );
// }

void US_Hydrodyn_Mals_Saxs_Svd::disable_all()
{
   lv_data            ->setEnabled( false );
   pb_clear           ->setEnabled( false );
   pb_to_mals_saxs         ->setEnabled( false );
   pb_save_plots      ->setEnabled( false );
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
   pb_efa             ->setEnabled( false );
   pb_efa_decomp      ->setEnabled( false );
   pb_stop            ->setEnabled( running );
   // pb_svd_plot        ->setEnabled( false );
   pb_svd_save        ->setEnabled( false );
   pb_recon           ->setEnabled( false );
   pb_inc_rmsd_plot   ->setEnabled( false );
   pb_rmsd_save       ->setEnabled( false );
   pb_inc_chi_plot    ->setEnabled( false );
   pb_inc_recon       ->setEnabled( false );
   pb_indiv_recon     ->setEnabled( false );

   rb_mode_iqit       ->setEnabled( false );
   rb_mode_svd        ->setEnabled( false );
   rb_mode_efa        ->setEnabled( false );
   rb_mode_efa_decomp ->setEnabled( false );


   qApp               ->processEvents();
   
}

void US_Hydrodyn_Mals_Saxs_Svd::q_start_text( const QString & )
{
}

void US_Hydrodyn_Mals_Saxs_Svd::q_end_text( const QString & )
{
}

// void US_Hydrodyn_Mals_Saxs_Svd::t_start_text( const QString & )
// {
// }

// void US_Hydrodyn_Mals_Saxs_Svd::t_end_text( const QString & )
// {
// }

void US_Hydrodyn_Mals_Saxs_Svd::replot( bool keep_mode )
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

   if ( keep_mode ) {
      svd_plot( false );
      efa_plot();
      efa_decomp_plot();
      update_enables();
      return;
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
      // hide_widgets( svd_widgets, true );
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

   if ( ( mode_i_of_t && !iq_it_state ) ||
        ( !mode_i_of_t && iq_it_state ) ) {
      set_title( plot_data, QString( "I(t) of %1" ).arg( get_name() ) );
   } else {
      set_title( plot_data, QString( "I(q) of %1" ).arg( get_name() ) );
   }
   
   plot_files();
   mode_select( MODE_IQIT );
   update_enables();
}

void US_Hydrodyn_Mals_Saxs_Svd::iq_it()
{
   disable_all();

   if ( plot_data_zoomer ) {
      plot_data_zoomer->zoom ( 0 );
      delete plot_data_zoomer;
      plot_data_zoomer = (ScrollZoomer *) 0;
   }

   if ( plot_errors_zoomer ) {
      plot_errors_zoomer->zoom ( 0 );
      delete plot_errors_zoomer;
      plot_errors_zoomer = (ScrollZoomer *) 0;
   }

   if ( sv_plot || chi_plot || rmsd_plot ) {
      axis_x_log = last_axis_x_log;
      axis_y_log = last_axis_y_log;
   }

   sv_plot   = false;
   rmsd_plot = false;
   chi_plot  = false;
   iq_it_state = !iq_it_state;
   pb_iq_it->setText( mode_i_of_t ? 
                      us_tr( iq_it_state ? "Show I(t)" : "Show I(q)" ) :
                      us_tr( iq_it_state ? "Show I(q)" : "Show I(t)" )
                      );

   if ( ( mode_i_of_t && !iq_it_state ) ||
        ( !mode_i_of_t && iq_it_state ) ) {
      axis_x_log = false;
      axis_y_log = false;
   } else {
      axis_x_log = false;
      axis_y_log = true;
   }

   axis_x_title();
   axis_y_title();
   
   replot();
   update_enables();
}

void US_Hydrodyn_Mals_Saxs_Svd::to_mals_saxs()
{
   if ( !ush_win->mals_saxs_widget )
   {
      editor_msg( "red", us_tr( "US-SOMO MALS_SAXS window has been closed" ) );
      return;
   }

   QStringList files = selected_files();

   disable_all();
   for ( int i = 0; i < (int) files.size(); ++i )
   {
      mals_saxs_win->add_plot( files[ i ],
                          f_qs[ files[ i ] ],
                          f_Is[ files[ i ] ],
                          f_errors[ files[ i ] ],
                          iq_it_state,
                          false );
   }
   update_enables();
}

void US_Hydrodyn_Mals_Saxs_Svd::axis_x()
{
   axis_x_log = !axis_x_log;

   axis_x_title();

   plot_data->replot();
   plot_svd->replot();
}

void US_Hydrodyn_Mals_Saxs_Svd::axis_y()
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
   plot_svd->replot();
}

void US_Hydrodyn_Mals_Saxs_Svd::hide_data()
{
   cout << QString( "hide data size %1\n" ).arg( data_widgets.size() );
   hide_widgets( data_widgets, data_widgets[ 0 ]->isVisible() );
      
   ush_win->gparams[ "mals_saxs_svd_data_widgets" ] = data_widgets[ 0 ]->isVisible() ? "visible" : "hidden";
}

void US_Hydrodyn_Mals_Saxs_Svd::hide_editor()
{
   hide_widgets( editor_widgets, editor_widgets[ 0 ]->isVisible() );
   if ( editor_widgets[ 0 ]->isVisible() )
   {
      cout << "resetting editor palette\n";
      lbl_editor->setPalette( PALET_LABEL );
      AUTFBACK( lbl_editor );
   }

   ush_win->gparams[ "mals_saxs_svd_editor_widgets" ] = editor_widgets[ 0 ]->isVisible() ? "visible" : "hidden";
}

void US_Hydrodyn_Mals_Saxs_Svd::hide_process()
{
   cout << QString( "hide process size %1\n" ).arg( process_widgets.size() );

   hide_widgets( process_widgets, process_widgets[ 0 ]->isVisible() );
      
   ush_win->gparams[ "mals_saxs_svd_process_widgets" ] = process_widgets[ 0 ]->isVisible() ? "visible" : "hidden";
}

void US_Hydrodyn_Mals_Saxs_Svd::hide_widgets( vector < QWidget *> widgets, bool hide )
{
   for ( unsigned int i = 0; i < ( unsigned int ) widgets.size(); i++ )
   {
      hide ? widgets[ i ]->hide() : widgets[ i ]->show();
   }
}

void US_Hydrodyn_Mals_Saxs_Svd::update_enables()
{
   // count selections
   if ( running )
   {
      return;
   }

   progress->reset();
   
   int sv_items = 0;
   for ( int i = 0; i < (int) lb_ev->count(); ++i )
   {
      if ( lb_ev->item( i )->isSelected() )
      {
         sv_items++;
      }
   }

   QStringList files = selected_files();
   set < QString > sources = get_selected_sources();

   lv_data            ->setEnabled( true );
   pb_clear           ->setEnabled( true );
   pb_to_mals_saxs         ->setEnabled( files.size() && !sources.count( us_tr( "Original data" ) ) && ush_win->mals_saxs_widget );
   pb_save_plots      ->setEnabled( true );
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

   pb_svd             ->setEnabled( files.size() && mode_i_of_t == iq_it_state && sources.size() == 1 && sources.count( us_tr( "Original data" ) ) );
   pb_efa             ->setEnabled( lb_ev->count() && pb_svd->isEnabled() && sources.count( us_tr( "Original data" ) ) );
   pb_efa_decomp      ->setEnabled( efa_lsv.size() && pb_efa->isEnabled() && sources.count( us_tr( "Original data" ) ) );
   pb_stop            ->setEnabled( false );
   // pb_svd_plot        ->setEnabled( lb_ev->count() );
   pb_svd_save        ->setEnabled( lb_ev->count() );

   bool no_norm = !cb_norm_pw->isChecked() && !cb_norm_avg->isChecked();

   pb_recon           ->setEnabled( sv_items && no_norm );

   pb_inc_rmsd_plot   ->setEnabled( rmsd_x.size() );
   pb_rmsd_save       ->setEnabled( rmsd_x.size() );
   pb_inc_chi_plot    ->setEnabled( chi_x.size() );
   pb_inc_recon       ->setEnabled( sv_items && no_norm );
   pb_indiv_recon     ->setEnabled( sv_items && no_norm );

   rb_mode_iqit       ->setEnabled( true );
   rb_mode_svd        ->setEnabled( true );
   rb_mode_efa        ->setEnabled( true );
   rb_mode_efa_decomp ->setEnabled( true );

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

void US_Hydrodyn_Mals_Saxs_Svd::data_selection_changed()
{
   update_enables();
}

void US_Hydrodyn_Mals_Saxs_Svd::sv_selection_changed()
{
   update_enables();
}

void US_Hydrodyn_Mals_Saxs_Svd::clear( )
{
   lv_data->clearSelection();
   update_enables();
}

QStringList US_Hydrodyn_Mals_Saxs_Svd::selected_files()
{
   QStringList result;

   QString iq_or_it = iq_it_state ? "I(t)" : "I(q)";

   QTreeWidgetItemIterator it( lv_data );
   while ( (*it) ) {
      QTreeWidgetItem *item = (*it);
      if ( is_selected( item ) &&
           US_Static::lvi_depth( item ) == 2 &&
           item->parent()->text( 0 ) == iq_or_it ) {
         result << item->text( 0 );
      }
      ++it;
   }
   return result;
}

set < QString > US_Hydrodyn_Mals_Saxs_Svd::get_current_files()
{
   set < QString > result;

   QTreeWidgetItemIterator it( lv_data );
   while ( (*it) ) 
   {
      QTreeWidgetItem *item = (*it);
      if ( US_Static::lvi_depth( item ) == 2 && 
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

set < QString > US_Hydrodyn_Mals_Saxs_Svd::get_sources()
{
   set < QString > result;

   QTreeWidgetItemIterator it( lv_data );
   while ( (*it) ) 
   {
      QTreeWidgetItem *item = (*it);
      if ( US_Static::lvi_depth( item ) == 0 )
      {
         result.insert( item->text( 0 ) );
      }
      ++it;
   }
   return result;
}

set < QString > US_Hydrodyn_Mals_Saxs_Svd::get_selected_sources()
{
   set < QString > result;
   QString iq_or_it = iq_it_state ? "I(t)" : "I(q)";

   // QStringList qsl;
   QTreeWidgetItemIterator it( lv_data );
   while ( (*it) ) 
   {
      QTreeWidgetItem *item = (*it);
      if ( is_selected( item ) &&
           US_Static::lvi_depth( item ) == 2 &&
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

QTreeWidgetItem * US_Hydrodyn_Mals_Saxs_Svd::get_source_item( QString source )
{
   QTreeWidgetItemIterator it( lv_data );
   while ( (*it) ) 
   {
      QTreeWidgetItem *item = (*it);
      if ( US_Static::lvi_depth( item ) == 0 &&
           item->text( 0 ) == source )
      {
         return item;
      }
      ++it;
   }
   return (QTreeWidgetItem *) 0;
}

int US_Hydrodyn_Mals_Saxs_Svd::selected_sources()
{
   QString iq_or_it = iq_it_state ? "I(t)" : "I(q)";

   set < QString > sources;

   QTreeWidgetItemIterator it( lv_data );
   while ( (*it) ) 
   {
      QTreeWidgetItem *item = (*it);
      if ( is_selected( item ) &&
           US_Static::lvi_depth( item ) == 2 &&
           item->parent()->text( 0 ) == iq_or_it )
      {
         sources.insert( item->parent()->parent()->text( 0 ) );
      }
      ++it;
   }
   return (int) sources.size();
}

void US_Hydrodyn_Mals_Saxs_Svd::clean_selected()
{
   // if a parent is selected, set selected on all children
   // & if all children are selected, set parents selected

   QTreeWidgetItemIterator it( lv_data );
   while ( (*it) ) 
   {
      QTreeWidgetItem *item = (*it);
      if ( !item->isSelected() )
      {
         if ( is_selected( item ) || all_children_selected( item ) )
         {
            item->setSelected( true );
         } 
      }
      ++it;
   }

   // lv_data->triggerUpdate();
}

bool US_Hydrodyn_Mals_Saxs_Svd::all_children_selected( QTreeWidgetItem *lvi )
{
#if QT_VERSION < 0x040000
   if ( lvi->childCount() )
   {
      QTreeWidgetItem *myChild = lvi->firstChild();
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
#else
   int children = lvi->childCount();
   if ( children ) { 
      for ( int i = 0; i < children; ++i ) {
         QTreeWidgetItem *myChild = lvi->child( i );
         if( myChild ) {
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
         }
      }
   } else {
      return lvi->isSelected();
   }
#endif

   return true;
}

bool US_Hydrodyn_Mals_Saxs_Svd::is_selected( QTreeWidgetItem *lvi )
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

void US_Hydrodyn_Mals_Saxs_Svd::axis_x_title()
{
   QString title = us_tr( iq_it_state ? "Time [a.u.]" : "q [1/Angstrom]" );
   if ( sv_plot || rmsd_plot || chi_plot )
   {
      title = us_tr( "Number" );
   }

   if ( axis_x_log )
   {
      plot_data->setAxisTitle(QwtPlot::xBottom,  title + us_tr(" (log scale)") );
      plot_data->setAxisScaleEngine(QwtPlot::xBottom, new QwtLogScaleEngine(10));
   } else {
      plot_data->setAxisTitle(QwtPlot::xBottom,  title );
      // actually need to test this, not sure what the correct version is
      plot_data->setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine );
   }
}

void US_Hydrodyn_Mals_Saxs_Svd::axis_y_title()
{
   QString title = us_tr( iq_it_state ? "I(t) [a.u.]" : "I(q) [a.u.]" );
   if ( sv_plot )
   {
      title = us_tr( "Singular values" );
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
      plot_data->setAxisTitle(QwtPlot::yLeft, title + us_tr( " (log scale)") );
      plot_data->setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine(10));
   } else {
      plot_data->setAxisTitle(QwtPlot::yLeft, title );
      // actually need to test this, not sure what the correct version is
      plot_data->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine );
   }
}

bool US_Hydrodyn_Mals_Saxs_Svd::plotted_matches_selected()
{
   set < QString > plotted; 
   set < QString > selected;

   QStringList files = selected_files();
   for ( int i = 0; i < (int) files.size(); ++i )
   {
      selected.insert( files[ i ] );
   }

   for ( 
        map < QString, QwtPlotCurve * >::iterator it = plotted_curves.begin();
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

void US_Hydrodyn_Mals_Saxs_Svd::add_i_of_q_or_t( QString source, QStringList files, bool do_update_enables )
{
   editor_msg( "blue", QString( us_tr(  "Making I(%1) for source %2" ) ).arg( mode_i_of_t ? "q" : "t" ).arg( source ) );

   QTreeWidgetItem * source_item = get_source_item( source );

   if ( !source_item )
   {
      editor_msg( "red", QString( us_tr( "Internal error: source item not defined %1" ) ).arg( source ) );
      return;
   }

   QTreeWidgetItem * i_t_child = (QTreeWidgetItem *) 0;
   QTreeWidgetItem * i_q_child = (QTreeWidgetItem *) 0;
      
   disable_all();

   {
      int children = source_item->childCount();
      if ( children ) { 
         for ( int i = 0; i < children; ++i ) {
            QTreeWidgetItem *myChild = source_item->child( i );
            if ( myChild ) {
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
            }
         }
      }
   }

   if ( mode_i_of_t ) {
      if ( !i_q_child ) {
         if ( i_t_child ) {
            i_q_child = new QTreeWidgetItem( source_item, i_t_child );
         } else {
            i_q_child = new QTreeWidgetItem( source_item, source_item );
         }
         i_q_child->setText( 0, "I(q)" );
      }
   } else {
      if ( !i_t_child ) {
         if ( i_q_child ) {
            i_t_child = new QTreeWidgetItem( source_item, i_q_child );
         } else {
            i_t_child = new QTreeWidgetItem( source_item, source_item );
         }
         i_t_child->setText( 0, "I(t)" );
      }
   }

   // find common q 
   QString head = mals_saxs_win->qstring_common_head( files, true );
   QString tail = mals_saxs_win->qstring_common_tail( files, true );

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
         editor_msg( "red", QString( us_tr( "Internal error: request to use %1, but not found in data" ) ).arg( files[ i ] ) );
      } else {
         QString tmp = files[ i ].mid( head.length() );
         tmp = tmp.mid( 0, tmp.length() - tail.length() );
         if ( rx_cap.indexIn( tmp ) != -1 )
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

   QTreeWidgetItem * lvi = mode_i_of_t ? i_q_child : i_t_child;

   for ( unsigned int i = 0; i < ( unsigned int )q.size(); i++ )
   {
      QString basename = QString( "%1_I%2_%3%4" ).arg( head ).arg( mode_i_of_t ? "q" : "t" ).arg( mode_i_of_t ? "t" : "q" ).arg( q[ i ] );
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

#if QT_VERSION < 0x040000
      lvi = new QTreeWidgetItem( mode_i_of_t ? i_q_child : i_t_child, lvi, fname );
#else
      lvi = new QTreeWidgetItem( mode_i_of_t ? i_q_child : i_t_child, lvi );
      lvi->setText( 0, fname );
#endif      
   
      f_pos       [ fname ] = f_qs.size();
      f_qs_string [ fname ] = t_qs;
      f_qs        [ fname ] = t;
      f_Is        [ fname ] = I;
      f_errors    [ fname ] = e;
      f_is_time   [ fname ] = !mode_i_of_t;
   }      
   if ( do_update_enables )
   {
      update_enables();
   }
   editor_msg( "blue", QString( us_tr(  "Done making I(%1) for source %2" ) ).arg( mode_i_of_t ? "q" : "t" ).arg( source ) );
}

void US_Hydrodyn_Mals_Saxs_Svd::rescale( bool do_update_enables )
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
      plot_data_zoomer->setTrackerPen(QPen(Qt::red));
      // connect( plot_data_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_data_zoomed( const QRectF & ) ) );
   }
   
   plot_data->replot();
   if ( do_update_enables )
   {
      update_enables();
   }
}

// class svd_sortable_double {
// public:
//    double       x;
//    int          index;
//    bool operator < (const svd_sortable_double& objIn) const
//    {
//       return x < objIn.x;
//    }
// };

void US_Hydrodyn_Mals_Saxs_Svd::svd_plot( bool axis_change )
{
   plot_svd->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_data->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   plot_ac->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_data->detachItems( QwtPlotItem::Rtti_PlotMarker );;

   if ( plot_svd_zoomer ) {
      plot_svd_zoomer->zoom ( 0 );
      delete plot_svd_zoomer;
      plot_svd_zoomer = (ScrollZoomer *) 0;
   }

   if ( plot_ac_zoomer ) {
      plot_ac_zoomer->zoom ( 0 );
      delete plot_ac_zoomer;
      plot_ac_zoomer = (ScrollZoomer *) 0;
   }

   if ( !svd_x.size() ) {
      return;
   }

   sv_plot   = true;
   rmsd_plot = false;
   chi_plot  = false;

   if ( axis_change ) {
      last_axis_x_log = axis_x_log;
      last_axis_y_log = axis_y_log;

      axis_x_log = false;
      axis_y_log = true;

      axis_x_title();
      axis_y_title();
   }


   {
      QwtPlotCurve *curve = new QwtPlotCurve( "svd" );
      plotted_curves[ "svd" ] = curve;
      curve->setStyle( QwtPlotCurve::Lines );

      curve->setSamples(
                        (double *)&( svd_x[ 0 ] ),
                        (double *)&( svd_y[ 0 ] ),
                        svd_x.size()
                        );

      curve->setSymbol( new QwtSymbol( QwtSymbol::Cross,
                                       QBrush( QColor( plot_colors[ 0 ] ), Qt::SolidPattern ),
                                       QPen( plot_colors[ 0 ], use_line_width + 1 ),
                                       QSize( 4 * ( use_line_width + 2 ), 4 * ( use_line_width + 2 ) ) )
                        );
   
      curve->setPen( QPen( plot_colors[ 0 ], use_line_width, Qt::SolidLine ) );
      curve->attach( plot_svd );

      //   plot_data->setAxisScale( QwtPlot::xBottom, 1, svd_x.size() );
      plot_svd->setAxisScale( QwtPlot::xBottom, 0.5, UHSHS_MAX_SV_FOR_EFA + 0.5 );
      plot_svd->setAxisScale( QwtPlot::yLeft  , svd_y.back() * 0.95e0, svd_y[ 0 ] * 1.1e0 );

      plot_svd_zoomer = new ScrollZoomer(plot_svd->canvas());
      plot_svd_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
      // connect( plot_svd_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_svd_zoomed( const QRectF & ) ) );

      set_title( plot_svd, QString( "SVD of %1" ).arg( last_svd_name ) );
      plot_svd->replot();
   }

   if ( svd_autocor_U.size() < svd_x.size() ) {
      editor_msg( "red", us_tr( "Internal error: size mismatch autocorr U. Please inform the developers" ) );
      update_enables();
      return;
   }
   {

      QwtPlotCurve *curve = new QwtPlotCurve( "q-Space Singular Vectors (U, Left)" );
      curve->setStyle( QwtPlotCurve::Lines );

      curve->setSamples(
                        (double *)&( svd_x[ 0 ] ),
                        (double *)&( svd_autocor_U[ 0 ] ),
                        svd_x.size()
                        );

      curve->setSymbol( new QwtSymbol( QwtSymbol::Cross,
                                       QBrush( QColor( plot_colors[ 0 ] ), Qt::SolidPattern ),
                                       QPen( plot_colors[ 0 ], use_line_width + 1 ),
                                       QSize( 4 * ( use_line_width + 2 ), 4 * ( use_line_width + 2 ) ) )
                        );
   
      curve->setPen( QPen( plot_colors[ 0 ], use_line_width, Qt::SolidLine ) );
      curve->setItemAttribute( QwtPlotItem::Legend, true );
      curve->attach( plot_ac );
   }
   if ( svd_autocor_V.size() < svd_x.size() ) {
      editor_msg( "red", us_tr( "Internal error: size mismatch autocorr V. Please inform the developers" ) );
      update_enables();
      return;
   }
   {
      QwtPlotCurve *curve = new QwtPlotCurve( "Frame-Space (V, Right)" );
      curve->setStyle( QwtPlotCurve::Lines );

      curve->setSamples(
                        (double *)&( svd_x[ 0 ] ),
                        (double *)&( svd_autocor_V[ 0 ] ),
                        svd_x.size()
                        );

      curve->setSymbol( new QwtSymbol( QwtSymbol::Cross,
                                       QBrush( QColor( plot_colors[ 1 ] ), Qt::SolidPattern ),
                                       QPen( plot_colors[ 1 ], use_line_width + 1 ),
                                       QSize( 4 * ( use_line_width + 2 ), 4 * ( use_line_width + 2 ) ) )
                        );
   
      curve->setPen( QPen( plot_colors[ 1 ], use_line_width, Qt::SolidLine ) );
      curve->setItemAttribute( QwtPlotItem::Legend, true );
      curve->attach( plot_ac );
   }
   plot_ac->setAxisScale( QwtPlot::xBottom, 0.5, UHSHS_MAX_SV_FOR_EFA + 0.5 );
   plot_ac->setAxisScale( QwtPlot::yLeft  , -0.1, 1.1 );

   plot_ac_zoomer = new ScrollZoomer(plot_ac->canvas());
   plot_ac_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));

   plot_ac->replot();

   update_enables();
}

QString US_Hydrodyn_Mals_Saxs_Svd::get_name() {
   QTreeWidgetItemIterator it( lv_data );
   while ( (*it) ) {
      QTreeWidgetItem *item = (*it);
      if ( is_selected( item ) ) {
         while ( item->parent() ) {
            item = item->parent();
         }
         // qDebug() << "get_name: is_selected text(0): " << item->text( 0 );
         return item->text( 0 );
      }
      ++it;
   }

   return "unknown";
}

void US_Hydrodyn_Mals_Saxs_Svd::svd() {
   disable_all();

   clear_efa();
   clear_efa_decomp();

   QStringList files = selected_files();

   lb_ev->clear( );

   last_svd_data = files;
   if ( !subset_data.count( files.join( "\n" ) ) )
   {
      files = add_subset_data( files );
   }
   set < QString > sel_sources = get_selected_sources();
   last_svd_name = *(sel_sources.begin());

   int m = (int) f_qs[ files[ 0 ] ].size();
   int n = (int) files.size();

   map < QString, vector < double > >  * f_Iuse = &f_Is;
   check_norm( files );
   QString norm_name = norm_name_map[ NORM_NOT ];
   if ( norm_ok ) {
      if ( cb_norm_pw->isChecked() ) {
         f_Iuse = &f_Is_norm_pw;
         norm_name = norm_name_map[ NORM_PW ];
      }
      if ( cb_norm_avg->isChecked() ) {
         f_Iuse = &f_Is_norm_avg;
         norm_name = norm_name_map[ NORM_AVG ];
      }
   }

   last_svd_name += " " + norm_name;

   // if ( 1 || m >= n ) {
   int maxmn = m > n ? m : n;

   vector < vector < double > > F       ( maxmn );
   vector < vector < double > > F_errors;
   vector < double * > a( maxmn );

   svd_F_nonzero = true;
   
   for ( int i = 0; i < m; ++i ) {
      F[ i ].resize( maxmn );
      for ( int j = 0; j < n; ++j ) {
         F[ i ][ j ] = (*f_Iuse)[ files[ j ] ][ i ];
         if ( !F[ i ][ j ] ) {
            svd_F_nonzero = false;
         }
      }
      a[ i ] = &(F[ i ][ 0 ]);
   }
   for ( int i = m; i < n; ++i ) {
      F[ i ].resize( maxmn );
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
      
   editor_msg( "blue", us_tr( "SVD: computing SVD on " ) + last_svd_name );
   if ( !SVD::dsvd( &(a[ 0 ]), m, n, &(w[ 0 ]), &(v[ 0 ]) ) )
   {
      editor_msg( "red", us_tr( SVD::errormsg ) );
      update_enables();
      return;
   }

   F.resize( m );
   for ( int i = 0; i < m; ++i ) {
      F[ i ].resize( m );
   }

   list < svd_sortable_double > svals;

   svd_sortable_double sval;
   for ( int i = 0; i < n; i++ )
   {
      sval.x     = w[ i ];
      sval.index = i;
      if ( sval.x ) {
         svals.push_back( sval );
      }
   }
   svals.sort();
   svals.reverse();

   svd_x.clear( );
   svd_y.clear( );

   if ( m < n ) {
      svd_U = transpose( F );
      svd_V = transpose( V );
   } else {
      svd_U = F;
      svd_V = V;
   }      
   svd_D = W;

   svd_index.resize( svals.size() );

   for ( list < svd_sortable_double >::iterator it = svals.begin();
         it != svals.end();
         ++it )
   {
      svd_index[ (int) svd_x.size() ] = it->index;
      svd_x.push_back( (double) svd_x.size() + 1e0 );
      svd_y.push_back( it->x );
      lb_ev->addItem( QString( "%1" ).arg( it->x ) );
   }

   // } else {

   //    vector < vector < double > > F       ( n );
   //    vector < vector < double > > F_errors;
   //    vector < double * > a( n );

   //    svd_F_nonzero = true;
   
   //    for ( int i = 0; i < n; ++i ) {
   //       F[ i ].resize( m );
   //       for ( int j = 0; j < m; ++j ) {
   //          F[ i ][ j ] = (*f_Iuse)[ files[ i ] ][ j ];
   //          if ( !F[ i ][ j ] ) {
   //             svd_F_nonzero = false;
   //          }
   //       }
   //       a[ i ] = &(F[ i ][ 0 ]);
   //    }

   //    svd_F        = F;
   //    svd_F_errors = F_errors;
   //    vector < double > W( m );
   //    double *w = &(W[ 0 ]);
   //    vector < double * > v( m );

   //    vector < vector < double > > V( m );
   //    for ( int j = 0; j < m; ++j )
   //    {
   //       V[ j ].resize( m );
   //       v[ j ] = &(V[ j ][ 0 ]);
   //    }
      
   //    editor_msg( "blue", us_tr( "SVD: computing SVD" ) );
   //    if ( !SVD::dsvd( &(a[ 0 ]), n, m, &(w[ 0 ]), &(v[ 0 ]) ) )
   //    {
   //       editor_msg( "red", us_tr( SVD::errormsg ) );
   //       update_enables();
   //       return;
   //    }

   //    list < svd_sortable_double > svals;

   //    svd_sortable_double sval;
   //    for ( int i = 0; i < m; i++ )
   //    {
   //       sval.x     = w[ i ];
   //       sval.index = i;
   //       svals.push_back( sval );
   //    }
   //    svals.sort();
   //    svals.reverse();

   //    svd_x.clear( );
   //    svd_y.clear( );

   //    svd_U = transpose( V );
   //    svd_D = W;
   //    svd_V = transpose( F );
   //    svd_index.resize( svals.size() );

   //    for ( list < svd_sortable_double >::iterator it = svals.begin();
   //          it != svals.end();
   //          ++it )
   //    {
   //       svd_index[ (int) svd_x.size() ] = it->index;
   //       svd_x.push_back( (double) svd_x.size() + 1e0 );
   //       svd_y.push_back( it->x );
   //       lb_ev->addItem( QString( "%1" ).arg( it->x ) );
   //    }
   // }      

   lbl_ev->setText( QString( us_tr( "Singular value list for %1" ) ).arg( last_svd_name ) );

   // SVD::cout_vvd( "svd U",  svd_U );
   // SVD::cout_vvd( "svd V",  svd_V );

   svd_autocor_U = autocor( svd_U );
   svd_autocor_V = autocor( svd_V );

#define DEBUG_MN
#if defined( DEBUG_MN )
   qDebug() << "US_Hydrodyn_Mals_Saxs_Svd::svd rows " << m << " cols " << n;
   matrix_info( "svd_U", svd_U );
   matrix_info( "svd_V", svd_V );
   qDebug() << "svd_D size " << svd_D.size();
   qDebug() << "svd_x size " << svd_x.size();
   qDebug() << "svd_autocor_U size " << svd_autocor_U.size();
   qDebug() << "svd_autocor_V size " << svd_autocor_V.size();
#endif

   // #define DEBUG_AUTOCOR
#if defined( DEBUG_AUTOCOR )
   US_Vector::printvector3( "SVD x autocor U, V", svd_x, svd_autocor_U, svd_autocor_V );
#endif

   svd_plot();
   set_number_of_svs_for_efa();
   editor_msg( "blue", us_tr( "SVD: Done computing SVD on " ) + last_svd_name );
   mode_select( MODE_SVD );
   update_enables();
}

void US_Hydrodyn_Mals_Saxs_Svd::recon()
{
   editor_msg( "blue", us_tr( "Start TSVD reconstruction" ) );
   disable_all();
   recon_mode = "";
   do_recon();
   update_enables();
   editor_msg( "blue", us_tr( "Done TSVD reconstruction" ) );
}

QTreeWidgetItem * US_Hydrodyn_Mals_Saxs_Svd::lvi_last_depth( int d )
{
   QTreeWidgetItem * result = (QTreeWidgetItem *) 0;

   QTreeWidgetItemIterator it( lv_data );
   while ( (*it) ) 
   {
      QTreeWidgetItem *item = (*it);
      if ( US_Static::lvi_depth( item ) == d )
      {
         result = item;
      }
      
      ++it;
   }

   return result;
}

void US_Hydrodyn_Mals_Saxs_Svd::color_rotate()
{
   vector < QColor >  new_plot_colors;

   for ( int i = 1; i < (int) plot_colors.size(); i++ )
   {
      new_plot_colors.push_back( plot_colors[ i ] );
   }
   new_plot_colors.push_back( plot_colors[ 0 ] );
   plot_colors = new_plot_colors;
   replot( true );
}

void US_Hydrodyn_Mals_Saxs_Svd::inc_rmsd_plot( bool axis_change ) {
   plot_data->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_data->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   plotted_curves.clear( );
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

   QwtPlotCurve *curve = new QwtPlotCurve( "rmsd" );
   plotted_curves[ "rmsd" ] = curve;
   curve->setStyle( QwtPlotCurve::Lines );

   curve->setSamples(
                  (double *)&( rmsd_x[ 0 ] ),
                  (double *)&( rmsd_y[ 0 ] ),
                  rmsd_x.size()
                  );

   curve->setPen( QPen( plot_colors[ 0 ], use_line_width, Qt::SolidLine ) );
   curve->attach( plot_data );

   plot_data->setAxisScale( QwtPlot::xBottom, 1, rmsd_x.size() );
   plot_data->setAxisScale( QwtPlot::yLeft  , vmin( rmsd_y ) * 0.9e0 , vmax( rmsd_y ) * 1.1e0 );

   plot_data_zoomer = new ScrollZoomer(plot_data->canvas());
   plot_data_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
   // connect( plot_data_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_data_zoomed( const QRectF & ) ) );

   plot_data->replot();
   update_enables();
}

void US_Hydrodyn_Mals_Saxs_Svd::inc_chi_plot( bool axis_change )
{
   plot_data->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_data->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   plotted_curves.clear( );
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

   QwtPlotCurve *curve = new QwtPlotCurve( "chi" );
   plotted_curves[ "chi" ] = curve;
   curve->setStyle( QwtPlotCurve::Lines );

   curve->setSamples(
                  (double *)&( chi_x[ 0 ] ),
                  (double *)&( chi_y[ 0 ] ),
                  chi_x.size()
                  );

   curve->setPen( QPen( plot_colors[ 0 ], use_line_width, Qt::SolidLine ) );
   curve->attach( plot_data );

   plot_data->setAxisScale( QwtPlot::xBottom, 1, chi_x.size() );
   plot_data->setAxisScale( QwtPlot::yLeft  , vmin( chi_y ) * 0.9e0 , vmax( chi_y ) * 1.1e0 );

   plot_data_zoomer = new ScrollZoomer(plot_data->canvas());
   plot_data_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
   // connect( plot_data_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_data_zoomed( const QRectF & ) ) );

   plot_data->replot();
   update_enables();
}

void US_Hydrodyn_Mals_Saxs_Svd::inc_recon()
{
   editor_msg( "blue", us_tr( "Start incremental TSVD" ) );
   recon_mode = "Inc. ";
   disable_all();
   pb_stop->setEnabled( true );
   running = true;

   last_recon_tag = QString( us_tr( "\"Incremental TSVD on SVD of %1\",\"RMSD of fit\",\"Singular values\"" ) ).arg( last_svd_name );
   last_recon_evs.clear( );

   rmsd_x.clear( );
   rmsd_y.clear( );

   chi_x.clear( );
   chi_y.clear( );
   
   vector < int > evs_selected;

   for ( int i = 0; i < (int) lb_ev->count(); ++i )
   {
      if ( lb_ev->item( i )->isSelected() )
      {
         evs_selected.push_back( i );
      }
   }

   disconnect( lb_ev, SIGNAL( itemSelectionChanged() ), 0, 0 );

   for ( int i = 0; i < (int) evs_selected.size(); ++i )
   {
      if ( !running )
      {
         editor_msg( "red", us_tr( "Processing stopped" ) );
         update_enables();
         return;
      }

      progress->setValue( i ); progress->setMaximum( evs_selected.size() );

      lb_ev->clearSelection();

      QString evs_used;
      for ( int j = 0; j <= i; ++j )
      {
         lb_ev->item( evs_selected[ j ])->setSelected( true );
         evs_used += QString( "%1 ").arg( lb_ev->item( evs_selected[ j ] )->text() );
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

   connect( lb_ev, SIGNAL( itemSelectionChanged() ), SLOT( sv_selection_changed() ) );

   running = false;
   update_enables();
   editor_msg( "blue", us_tr( "Done TSVD reconstruction" ) );
}

void US_Hydrodyn_Mals_Saxs_Svd::do_recon()
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
      if ( !lb_ev->item( i )->isSelected() )
      {
         D[ svd_index[ i ] ] = 0e0;
      } else {
         sv_count++;
         last_ev = lb_ev->item( i )->text();
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

   QTreeWidgetItem * lvi = new QTreeWidgetItem( lv_data, lvi_last_depth( 0 ) );
   lvi->setText( 0, name );
   svd_data_map[ name ] = last_svd_data;

   QTreeWidgetItem * lvn = new QTreeWidgetItem( lvi, lvi );
   lvn->setText( 0, "SVD of: " + last_svd_name );
   QTreeWidgetItem * evs = new QTreeWidgetItem( lvi, lvn );
   evs->setText( 0, "SVs used" );
   QTreeWidgetItem * lvinext = evs;
   for ( int i = 0; i < (int) lb_ev->count(); ++i )
   {
      if ( lb_ev->item( i )->isSelected() )
      {
         lvinext = new QTreeWidgetItem( evs, lvinext );
         lvinext->setText( 0, lb_ev->item( i )->text() );
      }
   }

   QTreeWidgetItem * iqs = new QTreeWidgetItem( lvi, US_Static::lv_lastItem( lv_data ) );
   iqs->setText( 0, mode_i_of_t ? "I(t)" : "I(q)" );

   // add I(q)
   // contained in columns of F, reference file names from last_svd_data

   vector < double >  q        = f_qs       [ last_svd_data[ 0 ] ];
   vector < QString > q_string = f_qs_string[ last_svd_data[ 0 ] ];
   vector < double >  e;

   set < QString > current_files = get_current_files();

   QString tag = QString( "TSVD%1_%2" ).arg( sv_count ).arg( sv_count == 1 ? QString( "%1_").arg( last_ev ).replace( ".","_" ) : QString( "" ) );
   // QTextStream( stdout ) <<
   //    QString( "do_recon():1 tag '%1' last_ev '%2'\n" )
   //    .arg( tag )
   //    .arg( last_ev )
   //    ;

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
   // QTextStream( stdout ) <<
   //    QString( "do_recon():2 tag '%1'\n" )
   //    .arg( tag )
   //    ;

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
      // QTextStream( stdout ) <<
      //    QString( "do_recon():d i %1 tag '%2' last_svd_data[i] %3\n" )
      //    .arg( i )
      //    .arg( tag )
      //    .arg( last_svd_data[ i ] )
      //    ;
      
      lvinext = new QTreeWidgetItem( iqs, lvinext );
      lvinext->setText( 0, this_name );

      final_files << this_name;

      f_pos      [ this_name ] = f_pos.size();
      f_qs_string[ this_name ] = q_string;
      f_qs       [ this_name ] = q;
      f_Is       [ this_name ] = I;
      f_errors   [ this_name ] = e;
      f_is_time  [ this_name ] = mode_i_of_t;
   }

   last_recon_rmsd = sqrt( rmsd2 ) / ( n * m - 1e0 );

   if ( svd_F_nonzero )
   {
#if QT_VERSION < 0x040000
      lvinext = new QTreeWidgetItem( lvi, evs, QString( "RMSD %1" ).arg( last_recon_rmsd ) );
#else
      lvinext = new QTreeWidgetItem( lvi, evs );
      lvinext->setText( 0, QString( "RMSD %1" ).arg( last_recon_rmsd ) );
#endif
      last_recon_chi = sqrt( chi2 ) / ( n * m - 1e0 );
      // new QListViewItem( lvi, lvinext, QString( "Chi %1" ).arg( last_recon_chi ) );
   }      

   add_i_of_q_or_t( name, final_files, false );
}

double US_Hydrodyn_Mals_Saxs_Svd::vmin( vector < double > &x )
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

double US_Hydrodyn_Mals_Saxs_Svd::vmax( vector < double > &x )
{
   if ( !x.size() ) {
      editor_msg( "red", us_tr( "Internal error: vmax called with empty vector. Please inform the developers." ) );
      return -1e99;
   }

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

void US_Hydrodyn_Mals_Saxs_Svd::stop()
{
   pb_stop->setEnabled( false );
   running = false;
   editor_msg( "red", us_tr( "Processing stop requested" ) );
   qApp->processEvents();
}

void US_Hydrodyn_Mals_Saxs_Svd::indiv_recon()
{
   recon_mode = "Indiv. ";
   editor_msg( "blue", us_tr( "Start individual TSVD" ) );
   disable_all();
   pb_stop->setEnabled( true );
   running = true;

   last_recon_tag = QString( us_tr( "\"Individual TSVD on SVD of %1\",\"RMSD of fit\",\"Singular value\"" ) ).arg( last_svd_name );
   last_recon_evs.clear( );
                            

   rmsd_x.clear( );
   rmsd_y.clear( );

   chi_x.clear( );
   chi_y.clear( );
   
   vector < int > evs_selected;

   for ( int i = 0; i < (int) lb_ev->count(); ++i )
   {
      if ( lb_ev->item( i )->isSelected() )
      {
         evs_selected.push_back( i );
      }
   }

   disconnect( lb_ev, SIGNAL( itemSelectionChanged() ), 0, 0 );

   for ( int i = 0; i < (int) evs_selected.size(); ++i )
   {
      if ( !running )
      {
         editor_msg( "red", us_tr( "Processing stopped" ) );
         update_enables();
         return;
      }

      progress->setValue( i ); progress->setMaximum( evs_selected.size() );

      lb_ev->clearSelection();

      lb_ev->item( evs_selected[ i ])->setSelected( true );

      qApp->processEvents();

      do_recon();

      last_recon_evs << QString( "%1" ).arg( lb_ev->item( evs_selected[ i ] )->text() );
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
      lb_ev->item( evs_selected[ i ])->setSelected( true );
   }

   connect( lb_ev, SIGNAL( itemSelectionChanged() ), SLOT( sv_selection_changed() ) );

   running = false;
   update_enables();
   editor_msg( "blue", us_tr( "Done TSVD reconstruction" ) );
}

void US_Hydrodyn_Mals_Saxs_Svd::set_plot_errors()
{
   // qDebug() << "set_plot_errors()";
   if ( cb_plot_errors->isChecked() )
   {
      hide_widgets( errors_widgets, false );
      update_plot_errors();
   } else {
      hide_widgets( errors_widgets, true );
   }
   update_enables();
}

void US_Hydrodyn_Mals_Saxs_Svd::set_plot_errors_rev()
{
   update_plot_errors();
}

void US_Hydrodyn_Mals_Saxs_Svd::set_plot_errors_sd()
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

void US_Hydrodyn_Mals_Saxs_Svd::set_plot_errors_pct()
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

void US_Hydrodyn_Mals_Saxs_Svd::set_plot_errors_group()
{
   if ( plot_errors_zoomer )
   {
      plot_errors_zoomer->zoom ( 0 );
      delete plot_errors_zoomer;
      plot_errors_zoomer = (ScrollZoomer *) 0;
   }
   
   update_plot_errors();
}

void US_Hydrodyn_Mals_Saxs_Svd::set_plot_errors_ref()
{
   if ( !cb_plot_errors_ref->isChecked() )
   {
      plot_files();
   } else {
      update_plot_errors();
   }
}

void US_Hydrodyn_Mals_Saxs_Svd::update_plot_errors( bool do_update_enables )
{
   // qDebug() << "update_plot_errors()";
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

QStringList US_Hydrodyn_Mals_Saxs_Svd::add_subset_data( QStringList files )
{
   QStringList result;

   set < QString > sources     = get_sources();
   set < QString > sel_sources = get_selected_sources();

   if ( sel_sources.size() != 1 )
   {
      editor_msg( "red", QString( us_tr( "Internal error: add_subset_data with more than one source (%1)" ) ).arg( sel_sources.size() ) );
      return result;
   }

   QString name = QString( us_tr( "Subset of %1" ) ).arg( *(sel_sources.begin()) );

   {
      int ext = 0;
      while ( sources.count( name ) )
      {
         name = QString( us_tr( "Subset %1 of %2" ) ).arg( ++ext ).arg( *(sel_sources.begin()) );
      }
   }

#if QT_VERSION < 0x040000
   QTreeWidgetItem * lvi = new QTreeWidgetItem( lv_data, lvi_last_depth( 0 ), name );

   // copy over I(q), ignore SVs, rmsd since these are not computed

   QTreeWidgetItem * iqs = new QTreeWidgetItem( lvi, US_Static::lv_lastItem( lv_data ), mode_i_of_t ? "I(t)" : "I(q)" );
#else
   QTreeWidgetItem * lvi = new QTreeWidgetItem( lv_data, lvi_last_depth( 0 ) );
   lvi->setText( 0, name );
   // copy over I(q), ignore SVs, rmsd since these are not computed

   QTreeWidgetItem * iqs = new QTreeWidgetItem( lvi, US_Static::lv_lastItem( lv_data ) );
   iqs->setText( 0, mode_i_of_t ? "I(t)" : "I(q)" );
#endif

   QString head = mals_saxs_win->qstring_common_head( files, true );
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

   QTreeWidgetItem * lvinext = iqs;

   for ( int i = 0; i < (int)files.size(); ++i )
   {
      QString this_name = tag + last_svd_data[ i ].mid( head.length(), files[ i ].length() - head.length() );
#if QT_VERSION < 0x040000
      lvinext = new QTreeWidgetItem( iqs, lvinext, this_name );
#else
      lvinext = new QTreeWidgetItem( iqs, lvinext );
      lvinext->setText( 0, this_name );
#endif
      result << this_name;
      
      f_pos      [ this_name ] = f_pos.size();
      f_qs_string[ this_name ] = f_qs_string[ files[ i ] ];
      f_qs       [ this_name ] = f_qs[ files[ i ] ];
      f_Is       [ this_name ] = f_Is[ files[ i ] ];
      f_errors   [ this_name ] = f_errors[ files[ i ] ];
      f_is_time  [ this_name ] = mode_i_of_t;
   }

   disconnect(lv_data, SIGNAL(itemSelectionChanged()), 0, 0 );
   lv_data->clearSelection();
   lvi->setSelected( true );
   connect(lv_data, SIGNAL(itemSelectionChanged()), SLOT( data_selection_changed() ));

   add_i_of_q_or_t( name, result, false );

   return result;
}

QString US_Hydrodyn_Mals_Saxs_Svd::get_related_source_name( QString name )
{
   // find "SVD of: " in current source

   QTreeWidgetItemIterator it( lv_data );
   while ( (*it) ) 
   {
      QTreeWidgetItem *item = (*it);
      if ( US_Static::lvi_depth( item ) == 1 &&
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

QStringList US_Hydrodyn_Mals_Saxs_Svd::get_files_by_name( QString name )
{
   // QTextStream( stdout ) << "get_files_by_name( " << name << " )\n";
   QString odata = us_tr( "Original data" );
   if ( name.left( odata.length() ) == odata ) {
      name = odata;
   }

   QStringList result;
   QString iq_or_it = iq_it_state ? "I(t)" : "I(q)";

   QTreeWidgetItemIterator it( lv_data );
   while ( (*it) ) 
   {
      QTreeWidgetItem *item = (*it);
      if ( 
          US_Static::lvi_depth( item ) == 2 &&
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

bool US_Hydrodyn_Mals_Saxs_Svd::get_name_norm_mode( QString fullname, QString &name, norms &norm_mode ) {
   // QTextStream( stdout ) << "get_name_norm_mode( " << fullname << " );\n";
   for ( int i = NORM_NOT; i <= NORM_AVG; ++i ) {
      QRegExp rx = QRegExp( " " + norm_name_map[ static_cast<norms>(i) ] + "$" );
      // QTextStream( stdout ) << "get_name_norm_mode: checking for '" << norm_name_map[ static_cast<norms>(i) ] << "'\n";
      if ( fullname.contains( rx ) ) {
         norm_mode = static_cast<norms>(i);
         name = fullname.replace( rx, "" );
         // QTextStream( stdout ) << "get_name_norm_mode() matched, returning true, name is now: '" << name << "'\n";
         return true;
      }
   }
   name = fullname;
   norm_mode = NO_NORM;
   // QTextStream( stdout ) << "get_name_norm_mode() dropping out, returning false, name is now: '" << name << "'\n";
   return false;
}

bool US_Hydrodyn_Mals_Saxs_Svd::get_plot_files( QStringList &use_list, QStringList &use_ref_list, norms &ref_norm_mode )
{
   // qDebug() << "get_plot_files()";
   set < QString > sources = get_selected_sources();
   if ( !sources.size() )
   {
      return false;
   }
   if ( sources.size() > 1 )
   {
      // editor_msg( "red", QString( us_tr( "Internal error: get_plot_files() with more than one source (%1)" ) ).arg( sources.size() ) );
      return false;
   }

   QString source     = *( sources.begin() );
   QString ref_source;
   (void) get_name_norm_mode( get_related_source_name( source ), ref_source, ref_norm_mode );
   // QTextStream( stdout ) << "ref source:" << ref_source << "  ref_norm_mode:" << norm_name_map[ ref_norm_mode ] << "\n";
   
   if ( ref_source.isEmpty() )
   {
      if ( source != us_tr( "Original data" ) ) {
         editor_msg( "red", QString( us_tr( "Internal error: get_plot_files() could not find related name for source %1" ) ).arg( source ) );
      }
      return false;
   }

   // cout << QString( "ref source <%1>\n" ).arg( ref_source );

   // find related curves

   QStringList files = selected_files();

   QStringList full_list     = get_files_by_name( source     );
   QStringList full_ref_list = get_files_by_name( ref_source );
 
   // qDebug() << "get_plot_files(): after get_files_by_name()";
   if ( full_list.size() != full_ref_list.size() )
   {
      editor_msg( "red", QString( us_tr( "Internal error: get_plot_files() file list count mismatch %1 %2" ) ).arg( full_list.size() ).arg( full_ref_list.size() ) );
      return false;
   }
      
   set < QString > selected;

   for ( int i = 0; i < (int) files.size(); ++i )
   {
      selected.insert( files[ i ] );
   }

   use_list.clear( );
   use_ref_list.clear( );

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
   
void US_Hydrodyn_Mals_Saxs_Svd::do_plot_errors()
{
   // qDebug() << "do_plot_errors()\n";
   
   QStringList use_list;
   QStringList use_ref_list;
   norms ref_norm_mode;
   if ( !get_plot_files( use_list, use_ref_list, ref_norm_mode ) )
   {
      return;
   }
   
   map < QString, vector < double > >  * ref_f_Iuse = &f_Is;

   switch( ref_norm_mode ) {
   case NORM_PW :
      ref_f_Iuse = &f_Is_norm_pw;
      break;

   case NORM_AVG :
      ref_f_Iuse = &f_Is_norm_avg;
      break;

   default :
      break;
   }

   plot_errors->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_errors->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   plot_errors_reference.clear( );

   vector < double > grid;
   vector < double > target;
   vector < double > errors;
   vector < double > fit;

   double grid_ofs = f_qs[ use_list[ 0 ] ][ 0 ];
   plot_errors_jumps.clear( );
   plot_errors_jumps.push_back( grid_ofs );

   bool use_errors = true;

   vector < int >    use_pos;
   vector < QColor > use_color( (int) use_list.size() );

   if ( use_ref_list.size() != use_list.size() ) {
      qDebug() << "do_plot_errors(): list sizes differ";
   }

   for ( int i = 0; i < (int) use_list.size(); ++i )
   {
      // QTextStream( stdout )
      //    <<
      //    QString( "use %1 list name '%2' fe %4 qs %5 qss %6 ref list name '%3' fe %7 qs %8 qss %9" )
      //    .arg( i )
      //    .arg( use_list[ i ] )
      //    .arg( use_ref_list[ i ] )
      //    .arg( f_errors.count( use_list[ i ] ) ? "yes" : "NO" )
      //    .arg( f_qs.count( use_list[ i ] ) ? "yes" : "NO" )
      //    .arg( f_qs_string.count( use_list[ i ] ) ? "yes" : "NO" )
      //    .arg( f_errors.count( use_ref_list[ i ] ) ? "yes" : "NO" )
      //    .arg( f_qs.count( use_ref_list[ i ] ) ? "yes" : "NO" )
      //    .arg( f_qs_string.count( use_ref_list[ i ] ) ? "yes" : "NO" )
      //    <<
      //    QString( "f_Is[ use_list ] %1 (*ref_f_Iuse)[use_ref_list] %2" )
      //    .arg( f_Is.count( use_list[ i ] ) ? "yes" : "NO" )
      //    .arg( (*ref_f_Iuse).count( use_ref_list[ i ] ) ? "yes" : "NO" )
      //    <<
      //    "\n";


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
         QTextStream( stdout )
            <<
            QString( "do_plot_errors() i=%1 use_ref_list[i] = %2 use_list[i] = %3\n" )
            .arg( i )
            .arg( use_ref_list[ i ] )
            .arg( use_list[ i ] )

            <<
            US_Vector::qs_vector2( "f_qs[use_ref_list[i]] f_qs[ref_list][i]",
                                   f_qs[ use_ref_list[ i ] ],
                                   f_qs[ use_list[ i ] ] )

            <<
            US_Vector::qs_vector2( "f_qs_string[use_ref_list[i]] f_qs_string[ref_list][i]",
                                   f_qs_string[ use_ref_list[ i ] ],
                                   f_qs_string[ use_list[ i ] ] )

            <<
            "\n"
            ;

         editor_msg( "red", QString( us_tr( "Error: plot residuals: curve grid mismatch" ) ) );
         return;
      }

      for ( int j = 0; j < (int) f_qs[ use_list[ i ] ].size(); ++j )
      {
         grid   .push_back( f_qs[ use_list[ i ] ][ j ] - f_qs[ use_list[ i ] ][ 0 ] + grid_ofs );
         fit    .push_back( f_Is[ use_list[ i ] ][ j ] );
         target .push_back( (*ref_f_Iuse)[ use_ref_list[ i ] ][ j ] );
         use_pos.push_back( i );   
         if ( use_errors )
         {
            errors.push_back( f_errors[ use_ref_list[ i ] ][ j ] );
         }
      }

      grid_ofs += f_qs[ use_list[ i ] ].back() - f_qs[ use_list[ i ] ][ 0 ];
      // QTextStream( stdout )
      //    <<
      //    QString( "grid_ofs for i = %1 is %2; f_pos is %3 color pos %4 colors size %5\n" )
      //    .arg( i )
      //    .arg( grid_ofs )
      //    .arg( f_pos[ use_list[ i ] ] )
      //    .arg( f_pos[ use_list[ i ] ] % plot_colors.size() )
      //    .arg( plot_colors.size() )
      //    ;
      plot_errors_jumps.push_back( grid_ofs );
   }
   plot_errors_jumps.pop_back();

   if ( cb_plot_errors_rev->isChecked() )
   {
      vector < double > tmp = fit;
      fit = target;
      target = tmp;
   }

   // check grid
   QTextStream( stdout ) << QString( "checking grid size %1\n" ).arg( grid.size() );
   
   for ( int i = 1; i < (int) grid.size(); ++i ) {
      if ( grid[ i - 1 ] > grid[ i ] ) {
         QTextStream( stdout ) << QString( "grid error grid[%1]=%2 > grid[%3]=%4\n" )
            .arg( i - 1 )
            .arg( grid[ i - 1 ] )
            .arg( i )
            .arg( grid[ i ] )
            ;
      }
   }
   QTextStream( stdout ) << QString( "grid check done\n" );

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
      for ( unsigned int i = 0; i < ( unsigned int ) e[ j ].size(); i++ )
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


   // QTextStream( stdout ) << US_Vector::qs_vector2( "use_pos, grid", use_pos, grid ) << "\n";
   
   for ( int j = 0; j < (int) x.size(); ++j )
   {
      // US_Vector::printvector2( "x[j], e[j]", x[j], e[j] );
      {
         QwtPlotCurve *curve = new QwtPlotCurve( "base" );
         curve->setStyle( QwtPlotCurve::Lines );
         curve->setPen( QPen( Qt::green, use_line_width, Qt::SolidLine ) );
         curve->setSamples(
                        (double *)&x[ j ][ 0 ],
                        (double *)&y[ j ][ 0 ],
                        x[ j ].size()
                        );
         curve->attach( plot_errors );
      }

      {
         QwtPlotCurve *curve = new QwtPlotCurve( "errors" );
         curve->setStyle( QwtPlotCurve::Lines );
         curve->setPen( QPen( use_color[ j ], use_line_width, Qt::SolidLine ) );
         curve->setSamples(
                        (double *)&x[ j ][ 0 ],
                        (double *)&e[ j ][ 0 ],
                        x[ j ].size()
                        );
         curve->setStyle( QwtPlotCurve::Sticks );
         curve->attach( plot_errors );
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
                                                        us_tr( cb_plot_errors_pct->isChecked() ?
                                                            "% difference %1" :
                                                            ( cb_plot_errors_sd->isChecked() ?
                                                              "delta I%1/sd" : "delta I%1" 
                                                              ) ) ).arg( iq_it_state ? "I(t)" : "I(q)" ) );

      // plot_errors->setAxisScale( QwtPlot::xBottom, x[ 0 ][ 0 ], x.back().back() );
      plot_errors->setAxisScale( QwtPlot::xBottom, grid[ 0 ], grid.back() );
      plot_errors->setAxisScale( QwtPlot::yLeft  , -maxy * 1.2e0 , maxy * 1.2e0 );

      plot_errors_zoomer = new ScrollZoomer(plot_errors->canvas());
      plot_errors_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
      // connect( plot_errors_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_errors_zoomed( const QRectF & ) ) );
   }

   plot_errors_jump_markers();

   plot_errors->replot();

   if ( cb_plot_errors_ref->isChecked() )
   {
      for ( int i = 0; i < (int) use_ref_list.size(); ++i )
      {
         plot_errors_reference.insert( use_ref_list[ i ] );
      }
      plot_files( use_ref_list, ref_norm_mode );
   }
}

void US_Hydrodyn_Mals_Saxs_Svd::plot_errors_jump_markers()
{
   if ( cb_plot_errors_group->isChecked() ||
        plot_errors_jumps.size() > 25 )
   {
      return;
   }

   for ( int i = 0; i < (int) plot_errors_jumps.size(); i++ )
   {
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
   }
   plot_errors->replot();
}

void US_Hydrodyn_Mals_Saxs_Svd::do_plot_errors_group()
{
   // qDebug() << "do_plot_errors_group()\n";

   QStringList use_list;
   QStringList use_ref_list;
   norms       ref_norm_mode;
   if ( !get_plot_files( use_list, use_ref_list, ref_norm_mode ) )
   {
      return;
   }

   map < QString, vector < double > >  * ref_f_Iuse = &f_Is;
   map < QString, vector < double > >  * org_f_Iuse = &f_Is;

   switch( ref_norm_mode ) {
   case NORM_PW :
      ref_f_Iuse = &f_Is_norm_pw;
      break;

   case NORM_AVG :
      ref_f_Iuse = &f_Is_norm_avg;
      break;

   default :
      break;
   }

   plot_errors->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_errors->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   plot_errors_reference.clear( );

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

   if ( cb_plot_errors_rev->isChecked() ) {
      map < QString, vector < double > >  * tmp_f_Iuse = org_f_Iuse;
      org_f_Iuse = ref_f_Iuse;
      ref_f_Iuse = tmp_f_Iuse;
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
               if ( (*ref_f_Iuse)[ this_ref_file ][ i ] != 0e0 )
               {
                  x.push_back( f_qs[ this_file ][ i ] );
                  y.push_back( 0e0 );
                  e.push_back( 100.0 * ( (*ref_f_Iuse)[ this_ref_file ][ i ] - (*org_f_Iuse)[ this_file ][ i ] ) / (*ref_f_Iuse)[ this_ref_file ][ i ] );
               }
            }
         } else {
            // cout << "pct mode, not using errors\n";

            for ( int i = 0; i < (int) f_qs[ this_file ].size(); ++i )
            {
               if ( (*ref_f_Iuse)[ this_ref_file ][ i ] != 0e0 )
               {
                  x.push_back( f_qs[ this_file ][ i ] );
                  y.push_back( 0e0 );
                  e.push_back( 100.0 * ( (*ref_f_Iuse)[ this_ref_file ][ i ] - (*org_f_Iuse)[ this_file ][ i ] ) / (*ref_f_Iuse)[ this_ref_file ][ i ] );
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
               e.push_back( ( (*ref_f_Iuse)[ this_ref_file ][ i ] - (*org_f_Iuse)[ this_file ][ i ] ) / f_errors[ this_ref_errors ][ i ] );
            }
         } else {
            // cout << "errors not ok & not used\n";
            for ( int i = 0; i < (int) f_qs[ this_file ].size(); ++i )
            {
               x.push_back( f_qs[ this_file ][ i ] );
               y.push_back( 0e0 );
               e.push_back( (*ref_f_Iuse)[ this_ref_file ][ i ] - (*org_f_Iuse)[ this_file ][ i ] );
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
         QwtPlotCurve *curve = new QwtPlotCurve( "base" );
         curve->setStyle( QwtPlotCurve::Lines );
         curve->setPen( QPen( Qt::green, use_line_width, Qt::SolidLine ) );
         curve->setSamples(
                        (double *)&x[ 0 ],
                        (double *)&y[ 0 ],
                        x.size()
                        );
         curve->attach( plot_errors );
      }

      {
         QwtPlotCurve *curve = new QwtPlotCurve( "errors" );
         curve->setStyle( QwtPlotCurve::Lines );
         curve->setPen( QPen( plot_colors[ f_pos[ this_color_file ] % plot_colors.size() ], use_line_width, Qt::SolidLine ) );
         curve->setSamples(
                        (double *)&x[ 0 ],
                        (double *)&e[ 0 ],
                        x.size()
                        );
         curve->setStyle( QwtPlotCurve::Sticks );
         curve->attach( plot_errors );
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
                                                        us_tr( cb_plot_errors_pct->isChecked() ?
                                                            "% difference %1" :
                                                            ( cb_plot_errors_sd->isChecked() ?
                                                              "delta I%1/sd" : "delta I%1" 
                                                              ) ) ).arg( iq_it_state ? "I(t)" : "I(q)" ) );

      plot_errors->setAxisScale( QwtPlot::xBottom, minx, maxx );
      plot_errors->setAxisScale( QwtPlot::yLeft  , -maxy * 1.2e0 , maxy * 1.2e0 );

      plot_errors_zoomer = new ScrollZoomer(plot_errors->canvas());
      plot_errors_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
      plot_errors_zoomer->setTrackerPen(QPen(Qt::red));
      // connect( plot_errors_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_zoomed( const QRectF & ) ) );
   }

   plot_errors->replot();

   if ( cb_plot_errors_ref->isChecked() )
   {
      for ( int i = 0; i < (int) use_ref_list.size(); ++i )
      {
         plot_errors_reference.insert( use_ref_list[ i ] );
      }
      plot_files( use_ref_list, ref_norm_mode );
   }
}

bool US_Hydrodyn_Mals_Saxs_Svd::setup_save( QString tag, QString & fname )
{
   QString use_dir = QDir::currentPath();

   ush_win->select_from_directory_history( use_dir, this );
   raise();

   fname = QFileDialog::getSaveFileName( this , us_tr( "Select a file name to " ) + tag , use_dir , "Text files (*.txt *.TXT);;"
                                        "CSV files (*.csv *.CSV);;"
                                        "All Files (*)" );

   if ( fname.isEmpty() )
   {
      return false;
   }

   if ( QFile::exists( fname ) )
   {
      fname = ush_win->fileNameCheck( fname, 0, this );
      raise();
   }
   ush_win->add_to_directory_history( fname );

   return true;
}

void US_Hydrodyn_Mals_Saxs_Svd::rmsd_save()
{
   QString fname;
   if ( !setup_save( us_tr( "save last RMSD values" ), fname ) )
   {
      return;
   }

   QFile f( fname );

   if ( !f.open( QIODevice::WriteOnly ) )
   {
      editor_msg( "red", QString( us_tr( "Error: can not open %1 for writing" ) ).arg( fname ) );
      return;
   }

   QString out = last_recon_tag + "\n";
   for ( int i = 0; i < (int) rmsd_x.size(); ++i )
   {
      out += QString( "%1,%2,%3" ).arg( rmsd_x[ i ] ).arg( rmsd_y[ i ] ).arg( last_recon_evs[ i ] ) + "\n";
   }

   if ( fname.toLower().contains( QRegExp( "\\.txt$" ) ) )
   {
      out.replace( "\"", "" ).replace( ",", "\t" );
   }

   QTextStream ts( &f );
   ts << out;
   f.close();
   editor_msg( "blue", QString( us_tr( "RMSDs saved in %1" ) ).arg( fname ) );
}

void US_Hydrodyn_Mals_Saxs_Svd::svd_save()
{
   QString fname;
   if ( !setup_save( us_tr( "save last SVD values" ), fname ) )
   {
      return;
   }

   QFile f( fname );

   if ( !f.open( QIODevice::WriteOnly ) )
   {
      editor_msg( "red", QString( us_tr( "Error: can not open %1 for writing" ) ).arg( fname ) );
      return;
   }

   QString out = QString( us_tr( "\"SVD of %1\",\"Singular value\"" ) ).arg( last_svd_name ) + "\n";
   for ( int i = 0; i < (int) lb_ev->count(); ++i )
   {
      out += QString( "%1,%2" ).arg( i + 1 ).arg( lb_ev->item( i )->text() ) + "\n";
   }

   if ( fname.toLower().contains( QRegExp( "\\.txt$" ) ) )
   {
      out.replace( "\"", "" ).replace( ",", "\t" );
   }

   QTextStream ts( &f );
   ts << out;
   f.close();
   editor_msg( "blue", QString( us_tr( "RMSDs saved in %1" ) ).arg( fname ) );
}

void US_Hydrodyn_Mals_Saxs_Svd::usp_config_plot_data( const QPoint & ) {
   US_PlotChoices *uspc = new US_PlotChoices( usp_plot_data );
   uspc->exec();
   delete uspc;
}

void US_Hydrodyn_Mals_Saxs_Svd::usp_config_plot_svd( const QPoint & ) {
   US_PlotChoices *uspc = new US_PlotChoices( usp_plot_svd );
   uspc->exec();
   delete uspc;
}

void US_Hydrodyn_Mals_Saxs_Svd::usp_config_plot_errors( const QPoint & ) {
   US_PlotChoices *uspc = new US_PlotChoices( usp_plot_errors );
   uspc->exec();
   delete uspc;
}

void US_Hydrodyn_Mals_Saxs_Svd::usp_config_plot_lefa( const QPoint & ) {
   US_PlotChoices *uspc = new US_PlotChoices( usp_plot_lefa );
   uspc->exec();
   delete uspc;
}

void US_Hydrodyn_Mals_Saxs_Svd::usp_config_plot_refa( const QPoint & ) {
   US_PlotChoices *uspc = new US_PlotChoices( usp_plot_refa );
   uspc->exec();
   delete uspc;
}

void US_Hydrodyn_Mals_Saxs_Svd::usp_config_plot_ac( const QPoint & ) {
   US_PlotChoices *uspc = new US_PlotChoices( usp_plot_ac );
   uspc->exec();
   delete uspc;
}

void US_Hydrodyn_Mals_Saxs_Svd::usp_config_plot_efa_decomp( const QPoint & ) {
   US_PlotChoices *uspc = new US_PlotChoices( usp_plot_efa_decomp );
   uspc->exec();
   delete uspc;
}

void US_Hydrodyn_Mals_Saxs_Svd::set_norm_pw() {
   if ( cb_norm_avg->isChecked() ) {
      cb_norm_avg->setChecked( false );
   }
   clear_svd();
   clear_efa();
   clear_efa_decomp();
   mode_select( MODE_IQIT );
}

void US_Hydrodyn_Mals_Saxs_Svd::set_norm_avg() {
   if ( cb_norm_pw->isChecked() ) {
      cb_norm_pw->setChecked( false );
   }
   clear_svd();
   clear_efa();
   clear_efa_decomp();
   mode_select( MODE_IQIT );
}

void US_Hydrodyn_Mals_Saxs_Svd::clear_svd() {
   plot_svd->detachItems( QwtPlotItem::Rtti_PlotCurve );
   plot_svd->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   plot_svd->replot();

   plot_ac->detachItems( QwtPlotItem::Rtti_PlotCurve );
   plot_ac->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   plot_ac->replot();

   svd_F.clear();
   svd_F_errors.clear();
   svd_U.clear();
   svd_V.clear();
   svd_D.clear();
   svd_index.clear();

   svd_x.clear();
   svd_y.clear();

   svd_autocor_U.clear();
   svd_autocor_V.clear();

   lbl_ev->setText( "" );
   lb_ev->clear();
   
   clear_efa();
   clear_efa_decomp();
}

void US_Hydrodyn_Mals_Saxs_Svd::clear_efa() {
   plot_lefa->detachItems( QwtPlotItem::Rtti_PlotCurve );
   plot_lefa->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   plot_lefa->replot();

   plot_refa->detachItems( QwtPlotItem::Rtti_PlotCurve );
   plot_refa->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   plot_refa->replot();

   efa_lsv.clear();
   efa_rsv.clear();

   plotted_efa_x.clear();
   plotted_efa_lsv.clear();
   plotted_efa_rsv.clear();

   clear_efa_decomp();
}

void US_Hydrodyn_Mals_Saxs_Svd::clear_efa_decomp() {
   plot_efa_decomp->detachItems( QwtPlotItem::Rtti_PlotCurve );
   plot_efa_decomp->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   plot_efa_decomp->replot();

   efa_decomp_x.clear();
   efa_decomp_Ct.clear();
}

void US_Hydrodyn_Mals_Saxs_Svd::efa() {
   disable_all();

   clear_efa_decomp();

   QStringList files = selected_files();

   last_efa_data = files;
   if ( !subset_data.count( files.join( "\n" ) ) )
   {
      files = add_subset_data( files );
   }
   set < QString > sel_sources = get_selected_sources();
   last_efa_name = *(sel_sources.begin());

   int m = (int) f_qs[ files[ 0 ] ].size();
   int n = (int) files.size();

   vector < vector < double > > A( n );

   map < QString, vector < double > >  * f_Iuse = &f_Is;
   check_norm( files );
   QString norm_name = norm_name_map[ NORM_NOT ];
   if ( norm_ok ) {
      if ( cb_norm_pw->isChecked() ) {
         f_Iuse = &f_Is_norm_pw;
         norm_name = norm_name_map[ NORM_PW ];
      }
      if ( cb_norm_avg->isChecked() ) {
         f_Iuse = &f_Is_norm_avg;
         norm_name = norm_name_map[ NORM_AVG ];
      }
   }
   last_efa_name += " " + norm_name;
   editor_msg( "blue", us_tr( "EFA: computing EFA on " ) + last_efa_name );

   for ( int i = 0; i < n; ++i ) {
      A[ i ] = (*f_Iuse)[ files[ i ] ];
      if ( (int) A[ i ].size() != m ) {
         editor_msg( "red", QString( us_tr( "Inconsistent data length (e.g. 1st file has %1 data points but file %2 has %3 data points" )
                                     .arg( m )
                                     .arg( i )
                                     .arg( (int) A[ i ].size() )
                                     )
                     );
         update_enables();
         return;
      }
   }

   US_Efa us_efa;

   unsigned int threads = USglobal->config_list.numThreads;
   progress->setValue( 0 );
   progress->setMaximum( 2 );

   if ( threads > 1 ) {
      if ( !us_efa.Efa_t( threads, A, efa_lsv ) ) {
         editor_msg( "red", us_tr( us_efa.errors ) );
         update_enables();
         return;
      }
      progress->setValue( 1 );
      qApp->processEvents();
      if ( !us_efa.Efa_t( threads, A, efa_rsv, true ) ) {
         editor_msg( "red", us_tr( us_efa.errors ) );
         update_enables();
         return;
      }
   } else {
      if ( !us_efa.Efa( A, efa_lsv ) ) {
         editor_msg( "red", us_tr( us_efa.errors ) );
         update_enables();
         return;
      }
      progress->setValue( 1 );
      qApp->processEvents();
      if ( !us_efa.Efa( A, efa_rsv, true ) ) {
         editor_msg( "red", us_tr( us_efa.errors ) );
         update_enables();
         return;
      }
   }      

   if ( efa_lsv.size() != efa_rsv.size() ) {
      editor_msg( "red", us_tr( "Unexpected error: Forward EFA and Backward EFA size mismatch" ) );
   }

   make_plotted_efas();

   disconnect( qwtc_efas, SIGNAL( valueChanged( double ) ), 0, 0 );
   int max_range = (int) ( efa_rsv.size() > UHSHS_MAX_SV_FOR_EFA ? UHSHS_MAX_SV_FOR_EFA : efa_rsv.size() );
   qwtc_efas->setRange( 1, max_range );
   if ( qwtc_efas->value() > max_range ) {
      qwtc_efas->setValue( max_range );
   }
   connect( qwtc_efas, SIGNAL( valueChanged( double ) ), SLOT( update_efas( double ) ) );
   update_efas( qwtc_efas->value() );
   init_efa_values();
   mode_select( MODE_EFA );
   editor_msg( "blue", us_tr( "EFA: Done computing EFA on " ) + last_efa_name );
   update_enables();
}

void US_Hydrodyn_Mals_Saxs_Svd::make_plotted_efas() {
   int n = (int) efa_lsv.size();

   plotted_efa_x.clear();
   plotted_efa_lsv.clear();
   plotted_efa_rsv.clear();

   plotted_efa_lsv.resize( n );
   plotted_efa_rsv.resize( n );

   for ( int n_use = 0; n_use < n; ++n_use ) { 
      plotted_efa_x.push_back( n_use );
   }

   for ( int j = 0; j < n; ++j ) {
      plotted_efa_lsv[ j ].resize( n );
      plotted_efa_rsv[ j ].resize( n );
   }

   for ( int n_use = 0; n_use < n; ++n_use ) {
      vector < double > lrpt = efa_lsv[ n_use ];
      vector < double > rrpt = efa_rsv[ n_use ];
      lrpt.resize( n );
      rrpt.resize( n );
         
      for ( int j = 0; j < n; ++j ) {
         plotted_efa_lsv[ j ][ n_use ] = lrpt[ j ];
         plotted_efa_rsv[ j ][ n - n_use - 1 ] = rrpt[ j ];
      }
   }

   int min_range = (int) plotted_efa_x.front();
   int max_range = (int) plotted_efa_x.back();
   for ( int i = 0; i < UHSHS_MAX_SV_FOR_EFA; ++i ) {
      efa_range_start [ i ]->setRange( min_range, max_range );
      efa_range_end   [ i ]->setRange( min_range, max_range );
   }
}   

void US_Hydrodyn_Mals_Saxs_Svd::update_efas( double val ) {
   // qDebug() << "US_Hydrodyn_Mals_Saxs_Svd::update_efas( " << val << " )";
   // efa_info( "update_efas" );
   // check range
   int ival = (int) val;
   efa_plot_count = ival;

   int n = (int) efa_lsv.size();
   if ( !n ) {
      update_efas_ranges();
      return;
   }

   if ( n != (int) efa_rsv.size() ) {
      editor_msg( "red", us_tr( "Unexpected error: Forward EFA and Backward EFA size mismatch in plot" ) );
      return;
   }

   if ( ival > n ) {
      efa_plot_count = n;
      editor_msg( "red", us_tr( "Unexpected error: requested greater than available SVs for EFA plot" ) );
      return;
   }
      
   // update efa plots
   efa_plot();
   update_efas_ranges();
}

void US_Hydrodyn_Mals_Saxs_Svd::efa_plot() {
   int n = (int) efa_lsv.size();
   if ( !n ) {
      update_efas_ranges();
      return;
   }

   // clear plots, data
   plot_lefa->detachItems( QwtPlotItem::Rtti_PlotCurve );
   plot_lefa->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   plot_refa->detachItems( QwtPlotItem::Rtti_PlotCurve );
   plot_refa->detachItems( QwtPlotItem::Rtti_PlotMarker );;

   qwtpm_rsv_range_marker.clear();
   qwtpm_lsv_range_marker.clear();

   if ( plot_lefa_zoomer ) {
      plot_lefa_zoomer->zoom ( 0 );
      delete plot_lefa_zoomer;
      plot_lefa_zoomer = (ScrollZoomer *) 0;
   }

   if ( plot_refa_zoomer ) {
      plot_refa_zoomer->zoom ( 0 );
      delete plot_refa_zoomer;
      plot_refa_zoomer = (ScrollZoomer *) 0;
   }

   // draw curves 

   // #define DEBUG_EFA_PLOT
#if defined( DEBUG_EFA_PLOT )
   bool leftEfa = false;

   QTextStream ts( stdout );
   ts << "Forward EFA:\n";

   for ( int n_use = 0; n_use < n; ++n_use ) { 
      vector < double > rpt = efa_lsv[ n_use ];
      rpt.resize( efa_plot_count );
         
      ts << ( leftEfa ? n - n_use - 1 : n_use ) << " ";
      for ( int j = 0; j < efa_plot_count; ++j ) {
         ts << rpt[ j ] << " ";
      }
      ts << "\n";
   }

   leftEfa = true;
   ts << "Backward EFA:\n";
   
   for ( int n_use = 0; n_use < n; ++n_use ) { 
      vector < double > rpt = efa_rsv[ n_use ];
      rpt.resize( efa_plot_count );
         
      ts << ( leftEfa ? n - n_use - 1 : n_use ) << " ";
      for ( int j = 0; j < efa_plot_count; ++j ) {
         ts << rpt[ j ] << " ";
      }
      ts << "\n";
   }
#endif

   // build up plot data

   // plotted_efa_x.clear();
   // plotted_efa_lsv.clear();
   // plotted_efa_rsv.clear();

   // plotted_efa_lsv.resize( efa_plot_count );
   // plotted_efa_rsv.resize( efa_plot_count );

   // // for ( int n_use = 1; n_use <= n; ++n_use ) { 
   // for ( int n_use = 0; n_use < n; ++n_use ) { 
   //    plotted_efa_x.push_back( n_use );
   // }

   // for ( int j = 0; j < efa_plot_count; ++j ) {
   //    plotted_efa_lsv[ j ].resize( n );
   //    plotted_efa_rsv[ j ].resize( n );
   // }

   double lminy = 1e99;
   double lmaxy = 0e0;
   double rminy = 1e99;
   double rmaxy = 0e0;

   for ( int n_use = 0; n_use < n; ++n_use ) {
      vector < double > lrpt = efa_lsv[ n_use ];
      vector < double > rrpt = efa_rsv[ n_use ];
      lrpt.resize( n );
      rrpt.resize( n );
         
      for ( int j = 0; j < efa_plot_count; ++j ) {
         // plotted_efa_lsv[ j ][ n_use ] = lrpt[ j ];
         // plotted_efa_rsv[ j ][ n - n_use - 1 ] = rrpt[ j ];
         if ( lrpt[ j ] ) {
            if ( lminy > lrpt[ j ] ) {
               lminy = lrpt[ j ];
            }
            if ( lmaxy < lrpt[ j ] ) {
               lmaxy = lrpt[ j ];
            }
         }
         if ( rrpt[ j ] ) {
            if ( rminy > rrpt[ j ] ) {
               rminy = rrpt[ j ];
            }
            if ( rmaxy < rrpt[ j ] ) {
               rmaxy = rrpt[ j ];
            }
         }
      }
   }      

#if defined( DEBUG_EFA_PLOT )
   for ( int j = 0; j < efa_plot_count; ++j ) {
      US_Vector::printvector( QString( "lefa %1" ).arg( j + 1 ), plotted_efa_lsv[ j ] );
      US_Vector::printvector( QString( "refa %1" ).arg( j + 1 ), plotted_efa_rsv[ j ] );
   }
#endif

   for ( int i = 0; i < efa_plot_count; ++i) { 
      {
         QwtPlotCurve *curve = new QwtPlotCurve( QString( "Forward EFA:%1" ).arg( i + 1 ) );
         curve->setStyle( QwtPlotCurve::Lines );
   
         curve->setSamples(
                           (double *)&( plotted_efa_x[ 0 ] ),
                           (double *)&( plotted_efa_lsv[ i ][ 0 ] ),
                           n
                           );

         curve->setPen( QPen( plot_colors[ i % plot_colors.size() ], use_line_width, Qt::SolidLine ) );
         curve->attach( plot_lefa );
      }
      {
         QwtPlotCurve *curve = new QwtPlotCurve( QString( "Backward EFA:%1" ).arg( i + 1 ) );
         curve->setStyle( QwtPlotCurve::Lines );

         curve->setSamples(
                           (double *)&( plotted_efa_x[ 0 ] ),
                           (double *)&( plotted_efa_rsv[ i ][ 0 ] ),
                           n
                           );

         curve->setPen( QPen( plot_colors[ i % plot_colors.size() ], use_line_width, Qt::SolidLine ) );
         curve->attach( plot_refa );
      }

      // and the range points

      {
         if ( (int) efa_range_start.size() <= i ) {
            editor_msg( "red", us_tr( "Internal error: start range point value not found. please inform the developers" ) );
         } else {
            int pos = (int) efa_range_start[ i ]->value();

            if ( (int) plotted_efa_lsv[ i ].size() <= pos ) {
               editor_msg( "red", us_tr( "Internal error: start range point value exceeds known values. please inform the developers" ) );
            } else {
               QwtPlotMarker* m = new QwtPlotMarker();
               m->setSymbol( new QwtSymbol(
                                           QwtSymbol::Cross,
                                           QBrush( plot_colors[ i % plot_colors.size() ] ),
                                           QPen( plot_colors[ i % plot_colors.size() ], use_line_width + 3 ),
                                           QSize( 4 * ( use_line_width + 2 ), 4 * ( use_line_width + 2 ) )
                                           ) );

               m->setValue( QPointF( plotted_efa_x[ pos ], plotted_efa_lsv[ i ][ pos ] ) );
               m->attach( plot_lefa );

               qwtpm_lsv_range_marker.push_back( m );
            }
         }
      }         
      {
         if ( (int) efa_range_end.size() <= i ) {
            editor_msg( "red", us_tr( "Internal error: end range point value not found. please inform the developers" ) );
         } else {
            int pos = (int) efa_range_end[ i ]->value();

            if ( (int) plotted_efa_rsv[ i ].size() <= pos ) {
               editor_msg( "red", us_tr( "Internal error: end range point value exceeds known values. please inform the developers" ) );
            } else {
               QwtPlotMarker* m = new QwtPlotMarker();
               m->setSymbol( new QwtSymbol( QwtSymbol::Cross,
                                            QBrush( plot_colors[ i % plot_colors.size() ] ),
                                            QPen( plot_colors[ i % plot_colors.size() ], use_line_width + 3 ),
                                            QSize( 4 * ( use_line_width + 2 ), 4 * ( use_line_width + 2 ) )
                                            ) );
               m->setValue( QPointF( plotted_efa_x[ pos ], plotted_efa_rsv[ i ][ pos ] ) );
               m->attach( plot_refa );

               qwtpm_rsv_range_marker.push_back( m );
            }
         }
      }         
   }

#if defined( DEBUG_EFA_PLOT_LAYOUT )
   qDebug() << QString( "lmaxy %1 rmaxy %2" ).arg( lmaxy ).arg( rmaxy );
#endif

   if ( !plot_lefa_zoomer ) {
      plot_lefa->setAxisScale( QwtPlot::xBottom, -0.5, n + 0.5 );
      plot_lefa->setAxisScale( QwtPlot::yLeft  , lminy * 0.7 , lmaxy * 1.3e0 );
      plot_lefa_zoomer = new ScrollZoomer(plot_lefa->canvas());
      plot_lefa_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
      // connect( plot_lefa_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_lefa_zoomed( const QRectF & ) ) );
   }
   if ( !plot_refa_zoomer ) {
      plot_refa->setAxisScale( QwtPlot::xBottom, -0.5, n + 0.5 );
      plot_refa->setAxisScale( QwtPlot::yLeft  , rminy * 0.7, rmaxy * 1.3e0 );
      plot_refa_zoomer = new ScrollZoomer(plot_refa->canvas());
      plot_refa_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
      // connect( plot_refa_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_refa_zoomed( const QRectF & ) ) );
   }

   plot_lefa->replot();
   plot_refa->replot();

   //   mode_select( MODE_EFA );
}

void US_Hydrodyn_Mals_Saxs_Svd::setup_norm() {
   unsigned int q_points = (unsigned int) f_qs[ mals_saxs_selected_files[ 0 ] ].size();

   vector < double > avg_errors( q_points );

   for ( int i = 0; i < (int) mals_saxs_selected_files.size(); ++i ) {
      QString this_name = mals_saxs_selected_files[ i ];
      
      unsigned int this_q_points = (unsigned int) f_qs[ this_name ].size();

      if ( this_q_points != q_points ) {
         QMessageBox::warning( this, 
                               windowTitle(),
                               us_tr( "Internal error: HPLC SVD called but curves have varying lengths" ) );
         close();
         return;
      }

      if ( q_points != (unsigned int) f_errors[ this_name ].size() ) {
         norm_ok = false;
         return;
      }

      for ( unsigned j = 0; j < q_points; ++j ) {
         if ( !f_errors[ this_name ][ j ] ) {
            norm_ok = false;
            return;
         }
         double recip_error = 1e0 / f_errors[ this_name ][ j ];
         f_Is_norm_pw[ this_name ].push_back( f_Is[ this_name ][ j ] * recip_error );
         avg_errors[ j ] += recip_error;
      }
   }

   double recip_fc = 1e0 / ( double ) mals_saxs_selected_files.size();

   for ( unsigned j = 0; j < q_points; ++j ) {
      avg_errors[ j ] *= recip_fc;
   }
      
   for ( int i = 0; i < (int) mals_saxs_selected_files.size(); ++i ) {
      QString this_name = mals_saxs_selected_files[ i ];
      
      for ( unsigned j = 0; j < q_points; ++j ) {
         f_Is_norm_avg[ this_name ].push_back( f_Is[ this_name ][ j ] * avg_errors[ j ] );
      }
   }

   // #define DEBUG_NORM
#if defined( DEBUG_NORM )

   US_Vector::printvector3(
   "file 0 I, errors, I_norm_pw"
      ,f_Is[ mals_saxs_selected_files[ 0 ] ]
      ,f_errors[ mals_saxs_selected_files[ 0 ] ]
      ,f_Is_norm_pw[ mals_saxs_selected_files[ 0 ] ]
      );

   US_Vector::printvector3(
   "file 0 I, avg_errors, I_norm_avg"
      ,f_Is[ mals_saxs_selected_files[ 0 ] ]
      ,avg_errors
      ,f_Is_norm_avg[ mals_saxs_selected_files[ 0 ] ]
      );

#endif

   org_avg_errors = avg_errors;
   norm_ok = true;
}

void US_Hydrodyn_Mals_Saxs_Svd::check_norm( QStringList & files ) {
   if ( !files.size() || !norm_ok ) {
      return;
   }

   if ( f_Is_norm_pw.count( files[ 0 ] ) &&
        f_Is_norm_avg.count( files[ 0 ] )
        ) {
      return;
   }

   unsigned int q_points = ( unsigned int ) org_avg_errors.size();

   for ( int i = 0; i < (int) files.size(); ++i ) {
      QString this_name = files[ i ];
      
      if ( f_Is_norm_pw.count( this_name ) ||
           f_Is_norm_avg.count( this_name )
           ) {
         editor_msg( "red", us_tr( "Internal error: unexpected condition 1 - inconsistency, turning off normalization, please inform the developers" ) );
         norm_ok = false;
         return;
      }

      unsigned int this_q_points = (unsigned int) f_qs[ this_name ].size();

      if ( this_q_points != q_points ) {
         editor_msg( "red", us_tr( "Internal error: unexpected condition 2 - length mismatch, turning off normalization, please inform the developers" ) );
         norm_ok = false;
         return;
      }

      for ( unsigned j = 0; j < q_points; ++j ) {
         if ( !f_errors[ this_name ][ j ] ) {
            editor_msg( "red", us_tr( "Internal error: unexpected condition 3 - zero error, turning off normalization, please inform the developers" ) );

            norm_ok = false;
            return;
         }

         f_Is_norm_pw [ this_name ].push_back( f_Is[ this_name ][ j ] * org_avg_errors[ j ] );
         f_Is_norm_avg[ this_name ].push_back( f_Is[ this_name ][ j ] / f_errors[ this_name ][ j ] );
      }
   }
}

double US_Hydrodyn_Mals_Saxs_Svd::autocor1( vector < double > &x ) {
   int len = (int) x.size();

#if defined( DEBUG_AUTOCOR_DETAIL )
   US_Vector::printvector( "US_Hydrodyn_Mals_Saxs_Svd::autocor1 x", x );
#endif


   // in case we ever need full:
   // for ( int k = -len + 1; k < len; ++k ) {

   int k = 1;
   double sum = 0e0;
   for ( int n = 0; n < len; ++n ) {
      if ( k + n >= 0 && k + n < len ) {
         sum += x[ k + n ] * x[ n ];
      }
   }
   return fabs( sum );
}

vector < double > US_Hydrodyn_Mals_Saxs_Svd::autocor( vector < vector < double > > &A ) {
   vector < double > result;
   if ( !A.size() ) {
      return result;
   }

   int rows = (int) A.size();
   int cols = (int) A[ 0 ].size();
   //   int stopat = rows < cols ? rows : cols;

#if defined( DEBUG_AUTOCOR_DETAIL )
   {
      QTextStream ts( stdout );
      ts << "US_Hydrodyn_Mals_Saxs_Svd::autocor A" << Qt::endl;
      for ( int i = 0; i < rows; ++i ) {
         ts << i << ":";
         for ( int j = 0; j < cols; ++j ) {
            ts << "\t" << A[ i ][ j ];
         }
         ts << "\n";
      }
      ts << "\n";
   }
#endif

   for ( int i = 0; i < cols; ++i ) {
      vector < double > x;
      for ( int j = 0; j < rows; ++j ) {
         x.push_back( A[ j ][ i ] );
      }
      result.push_back( autocor1( x ) );
   }

   return result;
}

vector < vector < int > > US_Hydrodyn_Mals_Saxs_Svd::transpose( vector < vector < int > > &A ) {

   //   qDebug() << "US_Hydrodyn_Mals_Saxs_Svd::transpose( vvi ) start";

   vector < vector < int > > result;
   if ( !A.size() ) {
      return result;
   }

   int rows = (int) A.size();
   int cols = (int) A[0].size();

   //   qDebug() << "US_Hydrodyn_Mals_Saxs_Svd::transpose( vvi ) rows " << rows << " cols " << cols;
   
   result.resize( cols );
   for ( int i = 0; i < cols; ++i ) {
      result[ i ].resize( rows );
      for ( int j = 0; j < rows; ++j ) {
         result[ i ][ j ] = A[ j ][ i ];
      }
   }
   //   qDebug() << "US_Hydrodyn_Mals_Saxs_Svd::transpose( vvi ) return";
   return result;
}

vector < vector < double > > US_Hydrodyn_Mals_Saxs_Svd::transpose( vector < vector < double > > &A ) {

   //   qDebug() << "US_Hydrodyn_Mals_Saxs_Svd::transpose( vvd ) start";

   vector < vector < double > > result;
   if ( !A.size() ) {
      return result;
   }

   int rows = (int) A.size();
   int cols = (int) A[0].size();

   //   qDebug() << "US_Hydrodyn_Mals_Saxs_Svd::transpose( vvd ) rows " << rows << " cols " << cols;
   
   result.resize( cols );
   for ( int i = 0; i < cols; ++i ) {
      result[ i ].resize( rows );
      for ( int j = 0; j < rows; ++j ) {
         result[ i ][ j ] = A[ j ][ i ];
      }
   }

   // #define DEBUG_TRANSPOSE

#if defined( DEBUG_TRANSPOSE )
   
   QTextStream ts( stdout );

   {
      ts << "US_Hydrodyn_Saxs_Svd::transpose A:\n";

      for ( int i = 0; i < rows; ++i ) {
         ts << i << ":";
         for ( int j = 0; j < cols; ++j ) {
            ts << "\t" << A[ i ][ j ];
         }
         ts << "\n";
      }
      ts << "\n";
   }
   {
      ts << "US_Hydrodyn_Saxs_Svd::transpose result:\n";

      for ( int i = 0; i < cols; ++i ) {
         ts << i << ":";
         for ( int j = 0; j < rows; ++j ) {
            ts << "\t" << result[ i ][ j ];
         }
         ts << "\n";
      }
      ts << "\n";
   }
#endif

   //   qDebug() << "US_Hydrodyn_Mals_Saxs_Svd::transpose( vvd ) return";

   return result;
}

double US_Hydrodyn_Mals_Saxs_Svd::vvd_min( vector < vector < double > > &A ) {
   double result = -1e99;
   int rows = (int) A.size();
   if ( !rows ) {
      editor_msg( "red", us_tr( "Internal error: vvd_min called with an empty vvd, please inform the developers" ) );
      return result;
   }
   int cols = (int) A[ 0 ].size();
   if ( !cols ) {
      editor_msg( "red", us_tr( "Internal error: vvd_min called with a columnless vvd, please inform the developers" ) );
      return result;
   }
   
   result = A[0][0];
   for ( int i = 0; i < rows; ++i ) {
      for ( int j = 0; j < cols; ++j ) {
         if ( result > A[i][j] ) {
            result = A[i][j];
         }
      }
   }
   return result;
}

double US_Hydrodyn_Mals_Saxs_Svd::vvd_max( vector < vector < double > > &A ) {
   double result = 1e99;
   int rows = (int) A.size();
   if ( !rows ) {
      editor_msg( "red", us_tr( "Internal error: vvd_max called with an empty vvd, please inform the developers" ) );
      return result;
   }
   int cols = (int) A[ 0 ].size();
   if ( !cols ) {
      editor_msg( "red", us_tr( "Internal error: vvd_max called with a columnless vvd, please inform the developers" ) );
      return result;
   }
   
   result = A[0][0];
   for ( int i = 0; i < rows; ++i ) {
      for ( int j = 0; j < cols; ++j ) {
         if ( result < A[i][j] ) {
            result = A[i][j];
         }
      }
   }
   return result;
}


void US_Hydrodyn_Mals_Saxs_Svd::vvd_smult( vector < vector < double > > &A, double x ) {
   int rows = (int) A.size();
   if ( !rows ) {
      editor_msg( "red", us_tr( "Internal error: vdd_smult called with an empty vvd, please inform the developers" ) );
      return;
   }
   int cols = (int) A[ 0 ].size();
   if ( !cols ) {
      editor_msg( "red", us_tr( "Internal error: vdd_smult called with a columnless vvd, please inform the developers" ) );
      return;
   }
   
   for ( int i = 0; i < rows; ++i ) {
      for ( int j = 0; j < cols; ++j ) {
         A[ i ][ j ] *= x;
      }
   }
}

vector < vector < double > > US_Hydrodyn_Mals_Saxs_Svd::vivd_pwmult( vector < vector < int > > &A,
                                                                     vector < vector < double > > &B ) {
   vector < vector < double > > result;
   int arows = (int) A.size();
   int brows = (int) B.size();
   if ( !arows || !brows ) {
      editor_msg( "red", us_tr( "Internal error: vivd_pwmult called with an empty vvd, please inform the developers" ) );
      return result;
   }
   if ( arows != brows ) {
      SVD::cout_vvi( "pwmult A", A );
      SVD::cout_vvd( "pwmult B", B );
      editor_msg( "red", us_tr( "Internal error: vivd_pwmult called with incompatible vi & vd, please inform the developers" ) );
      return result;
   }
   int acols = (int) A[0].size();
   int bcols = (int) B[0].size();
   if ( acols != bcols ) {
      editor_msg( "red", us_tr( "Internal error: vivd_pwmult called with incompatible vi & vd colwise, please inform the developers" ) );
      return result;
   }
   
   result.resize( arows );

   for ( int i = 0; i < arows; ++i ) {
      result[ i ].resize( acols );

      for ( int j = 0; j < acols; ++j ) {
         result[ i ][ j ] = ((double) A[ i ][ j ]) * B[ i ][ j ];
      }
   }
   return result;
}

void US_Hydrodyn_Mals_Saxs_Svd::vvd_cnorm( vector < vector < double > > &A ) {
   int rows = (int) A.size();
   if ( !rows ) {
      editor_msg( "red", us_tr( "Internal error: vvd_cnorm called with an empty vvd, please inform the developers" ) );
      return;
   }
   int cols = (int) A[ 0 ].size();
   if ( !cols ) {
      editor_msg( "red", us_tr( "Internal error: vvd_cnorm called with a columnless vvd, please inform the developers" ) );
      return;
   }
   vector < double > csum( rows );
   
   for ( int i = 0; i < rows; ++i ) {
      for ( int j = 0; j < cols; ++j ) {
         csum[ i ] += A[ i ][ j ];
      }
      if ( csum[ i ] ) {
         csum[ i ] = 1e0 / csum[ 1 ];
      } else {
         editor_msg( "red", us_tr( "Internal error: vvd_cnorm called with a zero sum column, please inform the developers" ) );
         return;
      }
   }
   for ( int i = 0; i < rows; ++i ) {
      for ( int j = 0; j < cols; ++j ) {
         A[ i ][ j ] *=  csum[ i ];
      }
   }
}

void US_Hydrodyn_Mals_Saxs_Svd::vvd_cnorm( vector < vector < double > > &A,
                                           vector < vector < double > > &B ) {
   // norms A by the colsum of B
   int arows = (int) A.size();
   int brows = (int) B.size();
   if ( !arows || !brows ) {
      editor_msg( "red", us_tr( "Internal error: vvd_cnorm called with an empty vvd, please inform the developers" ) );
      return;
   }
   if ( arows != brows ) {
      editor_msg( "red", us_tr( "Internal error: vvd_cnorm called with incompatible vvds, please inform the developers" ) );
      return;
   }

   int acols = (int) A[ 0 ].size();
   int bcols = (int) B[ 0 ].size();
   if ( !acols || !bcols ) {
      editor_msg( "red", us_tr( "Internal error: vvd_cnorm called with a columnless vvd, please inform the developers" ) );
      return;
   }
   if ( acols != bcols ) {
      editor_msg( "red", us_tr( "Internal error: vvd_cnorm called with incompatible vvds, please inform the developers" ) );
      return;
   }

   vector < double > csum( acols );
   
   for ( int j = 0; j < acols; ++j ) {
      csum[ j ] = 0e0;
      for ( int i = 0; i < arows; ++i ) {
         csum[ j ] += B[ i ][ j ];
      }
      if ( csum[ j ] ) {
         csum[ j ] = 1e0 / csum[ j ];
      } else {
         SVD::cout_vvd( "cnorm A", A );
         SVD::cout_vvd( "cnorm B", B );
         SVD::cout_vd( "csum", csum );
         editor_msg( "red", us_tr( "Internal error: vvd_cnorm called with a zero sum column, please inform the developers" ) );
         return;
      }
   }
   for ( int i = 0; i < arows; ++i ) {
      for ( int j = 0; j < acols; ++j ) {
         A[ i ][ j ] *=  csum[ j ];
      }
   }
}

vector < vector < double > > US_Hydrodyn_Mals_Saxs_Svd::dot( vector < vector < double > > &A,
                                                             vector < vector < double > > &B ) {
   vector < vector < double > > result;
   int arows = (int) A.size();
   int brows = (int) B.size();
   if ( !arows || !brows ) {
      editor_msg( "red", us_tr( "Internal error: dot called with an empty vvd's, please inform the developers" ) );
      return result;
   }

   int acols = (int) A[0].size();
   int bcols = (int) B[0].size();

   if ( acols != brows ) {
      editor_msg( "red", us_tr( "Internal error: dot called with incompatible vvd's, please inform the developers" ) );
      return result;
   }

   result.resize( arows );

   for ( int i = 0; i < arows; ++i ) {
      result[ i ].resize( bcols );
      for ( int j = 0; j < bcols; ++j ) {
         result[ i ][ j ] = 0e0;
         for ( int k = 0; k < acols; ++k ) {
            result[ i ][ j ] += A[ i ][ k ] * B[ k ][ j ];
         }
      }
   }
   return result;
}

void US_Hydrodyn_Mals_Saxs_Svd::matrix_info( QString qs, vector < vector < double > > &A ) {
   if ( !A.size() ) {
      qDebug() << "US_Hydrodyn_Mals_Saxs_Svd::matrix_info() " << qs << " empty!";
      return;
   }
   qDebug() << "US_Hydrodyn_Mals_Saxs_Svd::matrix_info() " << qs << " rows " << A.size() << " cols " << A[0].size();
}   

void US_Hydrodyn_Mals_Saxs_Svd::set_title( QwtPlot *plot, QString title ) {
   QwtText qwtt_title( title );
   qwtt_title.setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   plot->setTitle( qwtt_title );
}
   
void US_Hydrodyn_Mals_Saxs_Svd::save_plots() {
   QString use_dir = QDir::currentPath();
   ush_win->select_from_directory_history( use_dir, this );

   QString fn = 
      QFileDialog::getSaveFileName( this , us_tr( "Select a prefix name to save the plot data" ) , use_dir , "*.csv" );

   if ( fn.isEmpty() )
   {
      return;
   }

   fn = QFileInfo( fn ).path() + QDir::separator() + QFileInfo( fn ).completeBaseName();

   QString errors;
   QString messages;

   map < QString, QwtPlot *>  use_plot_info = plot_info;

   if ( !US_Plot_Util::printtofile( fn, plot_info, errors, messages ) )
   {
      editor_msg( "red", errors );
   } else {
      editor_msg( "blue", messages );
   }
}

void US_Hydrodyn_Mals_Saxs_Svd::mode_select() {
   // first hide all
   hide_widgets( iqit_widgets       , true );
   hide_widgets( svd_widgets        , true );
   hide_widgets( efa_widgets        , true );
   hide_widgets( efa_decomp_widgets , true );
   update_efas_ranges();   

   // then shoe (this allows same widget in mulitple lists)
   switch ( current_mode )
   {
   case MODE_IQIT    : 
      {
         rb_mode_iqit->setChecked( true );
         hide_widgets( iqit_widgets      , false ); 
         hide_widgets( errors_widgets    , true ); 
      }
      break;

   case MODE_SVD    : 
      {
         rb_mode_svd->setChecked( true );
         hide_widgets( svd_widgets       , false ); 
      }
      break;

   case MODE_EFA   : 
      {
         rb_mode_efa->setChecked( true );
         hide_widgets( efa_widgets       , false ); 
      }
      break;

   case MODE_EFA_DECOMP   : 
      {
         rb_mode_efa_decomp->setChecked( true );
         hide_widgets( efa_decomp_widgets, false ); 
      }
      break;
   }
   // update_enables();
}

void US_Hydrodyn_Mals_Saxs_Svd::mode_select( modes mode ) {
   current_mode = mode;
   mode_select();
   update_enables();
}

void US_Hydrodyn_Mals_Saxs_Svd::set_mode_iqit() {
   mode_select( MODE_IQIT );
}

void US_Hydrodyn_Mals_Saxs_Svd::set_mode_svd() {
   mode_select( MODE_SVD );
}

void US_Hydrodyn_Mals_Saxs_Svd::set_mode_efa() {
   mode_select( MODE_EFA );
}

void US_Hydrodyn_Mals_Saxs_Svd::set_mode_efa_decomp() {
   mode_select( MODE_EFA_DECOMP );
}

void US_Hydrodyn_Mals_Saxs_Svd::update_efa_range_start( double ) {
   // qDebug() << "US_Hydrodyn_Mals_Saxs_Svd::update_efa_range_start";
   // {
   //    QTextStream ts( stdout );
   //    for ( int i = 0; i < UHSHS_MAX_SV_FOR_EFA; ++i ) {
   //       ts << "US_Hydrodyn_Mals_Saxs_Svd::update_efa_range_start SV " << i << " val " << efa_range_start[ i ]->value() << Qt::endl;
   //    }
   //    if ( efa_range_processing ) {
   //       ts << "early return, processing\n";
   //    }
   // }     

   if ( efa_range_processing ) {
      return;
   }

   clear_efa_decomp();

   if ( (int) qwtc_efas->value() != (int) qwtpm_lsv_range_marker.size() ) {
      editor_msg( "red", us_tr( "Internal error: inconsistency in start range marker size & efas values. please inform the developers" ) );
      return;
   }
      
   for ( int i = 0; i < (int) qwtc_efas->value(); ++i ) {
      if ( (int) efa_range_start.size() <= i ) {
         editor_msg( "red", us_tr( "Internal error: start range point value not found. please inform the developers" ) );
         return;
      } else {
         int pos = (int) efa_range_start[ i ]->value();
         if ( (int) plotted_efa_lsv[ i ].size() <= pos ) {
            editor_msg( "red", us_tr( "Internal error: start range point value exceeds known values. please inform the developers" ) );
            return;
         } else {
            qwtpm_lsv_range_marker[ i ]->setValue( QPointF( plotted_efa_x[ pos ], plotted_efa_lsv[ i ][ pos ] ) );
         }
      }
   }
   plot_lefa->replot();
}

void US_Hydrodyn_Mals_Saxs_Svd::update_efa_range_end( double ) {
   // qDebug() << "US_Hydrodyn_Mals_Saxs_Svd::update_efa_range_end()";
   if ( efa_range_processing ) {
      return;
   }

   if ( (int) qwtc_efas->value() != (int) qwtpm_rsv_range_marker.size() ) {
      editor_msg( "red", us_tr( "Internal error: inconsistency in end range marker size & efas values. please inform the developers" ) );
      return;
   }
      
   clear_efa_decomp();

   for ( int i = 0; i < (int) qwtc_efas->value(); ++i ) {
      if ( (int) efa_range_end.size() <= i ) {
         editor_msg( "red", us_tr( "Internal error: end range point value not found. please inform the developers" ) );
         return;
      } else {
         int pos = (int) efa_range_end[ i ]->value();
         if ( (int) plotted_efa_rsv[ i ].size() <= pos ) {
            editor_msg( "red", us_tr( "Internal error: end range point value exceeds known values. please inform the developers" ) );
            return;
         } else {
            qwtpm_rsv_range_marker[ i ]->setValue( QPointF( plotted_efa_x[ pos ], plotted_efa_rsv[ i ][ pos ] ) );
         }
      }
   }
   plot_refa->replot();
}

void US_Hydrodyn_Mals_Saxs_Svd::update_efas_ranges() {
   // qDebug() << "US_Hydrodyn_Mals_Saxs_Svd::update_efas_ranges()";

   // show/hide appropriately

   if ( current_mode != MODE_EFA || !efa_lsv.size() ) {
      for ( int i = 0; i < UHSHS_MAX_SV_FOR_EFA; ++i ) {
         efa_range_labels [ i ]->hide();
         efa_range_start  [ i ]->hide();
         efa_range_labels2[ i ]->hide();
         efa_range_end    [ i ]->hide();
      }
      return;
   }

   for ( int i = 0; i < (int) qwtc_efas->value(); ++i ) {
      efa_range_labels [ i ]->show();
      efa_range_start  [ i ]->show();
      efa_range_labels2[ i ]->show();
      efa_range_end    [ i ]->show();
   }

   for ( int i = (int) qwtc_efas->value(); i < UHSHS_MAX_SV_FOR_EFA; ++i ) {
      efa_range_labels [ i ]->hide();
      efa_range_labels2[ i ]->hide();
      efa_range_start  [ i ]->hide();
      efa_range_end    [ i ]->hide();
   }
}      

void US_Hydrodyn_Mals_Saxs_Svd::efa_info( QString tag ) {
   QTextStream ts( stdout );

   ts << " efa_info: " << tag << " : efa_lsv.size() : " << efa_lsv.size() << Qt::endl;

   for ( int i = 0; i < (int) efa_lsv.size(); ++i ) {
      ts << " efa_info: " << tag << ": efa_lsv[ " << i << " ].size() : " << efa_lsv[ i ].size() << Qt::endl;
   }

   
   ts << " efa_info: " << tag << " : efa_rsv.size() : " << efa_rsv.size() << Qt::endl;

   for ( int i = 0; i < (int) efa_rsv.size(); ++i ) {
      ts << " efa_info: " << tag << ": efa_rsv[ " << i << " ].size() : " << efa_rsv[ i ].size() << Qt::endl;
   }

}      

vector < double > US_Hydrodyn_Mals_Saxs_Svd::gradient( vector < double > & y ) {
   int len = (int) y.size();
   int lenm1 = len - 1;
   
   // qDebug() << "gradient len" << len;

   vector < double > g;

   if ( len < 2 ) {
      editor_msg( "red", us_tr( "Internal error: gradient called with emptyish vector. Please inform the developers." ) );
      return g;
   }
   g.resize( len );

   g[ 0 ] = y[ 1 ] - y[ 0 ];
   for ( int i = 1; i < lenm1; ++i ) {
      g[ i ] = ( y[ i + 1 ] - y[ i - 1 ] ) * 0.5;
   }
   g[ lenm1 ] = y[ lenm1 ] - y[ lenm1 - 1 ];
   return g;
}

void US_Hydrodyn_Mals_Saxs_Svd::init_efa_values() {
   if ( plotted_efa_lsv.size() != plotted_efa_rsv.size() ) {
      editor_msg( "red", us_tr( "Internal error: init efa values size mismatch. Please inform the developers." ) );
      return;
   }
   if ( !plotted_efa_lsv.size() ) {
      editor_msg( "red", us_tr( "Internal error: init efa values efa empty. Please inform the developers." ) );
      return;
   }

   int n = (int) plotted_efa_lsv.size();

   if ( n < 3 ) {
      editor_msg( "red", us_tr( "Error: Too little data for EFA." ) );
      return;
   }

   int max_range = (int) ( n > UHSHS_MAX_SV_FOR_EFA ? UHSHS_MAX_SV_FOR_EFA : n );
   if ( max_range > lb_ev->count() ) {
      max_range = lb_ev->count();
   }

   int prev_value = 0;

   efa_range_processing = true;

   // #define DEBUG_INIT_EFA_RANGES

   for ( int i = 0; i < max_range; ++i ) {
      {
         vector < double > use_v = plotted_efa_lsv[ i ];

         vector < double > g = gradient( use_v );
         double gmax = vmax( g );
         double gmaxrecip = 1e0 / gmax;

         vector < double > results;

         for ( int j = 0; j < n; ++j ) {
            double gnorm = g[ j ] * gmaxrecip;
            // qDebug() << "gnorm[ " << j << " ] : " << gnorm;
            if ( gnorm > 0.05 + 0.05 * i ) {
               results.push_back( j );
            }
         }

#if defined( DEBUG_INIT_EFA_RANGES )
         {
            QTextStream ts( stdout );
            US_Vector::printvector( QString( "plotted_efa_lsv[%1]" ).arg( i ), use_v );
            US_Vector::printvector( QString( "gradient of plotted_efa_lsv[%1]" ).arg( i ), g );
            ts << "gmax : " << gmax;
            ts << "gmaxrecip : " << gmaxrecip;
            US_Vector::printvector( QString( "init efa forward sv %1" ).arg( i+1 ), results );
         }
#endif
            
         int use_value = prev_value;

         if ( !results.size() ) {
            use_value = prev_value + 1;
            if ( use_value >= n ) {
               use_value = n - 1;
            }
            prev_value = use_value;
            efa_range_start[ i ]->setValue( use_value );
            // set to this prev_value
            continue;
         }

         use_value = results[ 0 ];

         if ( results.size() > 1 ) {
            int j = 1;
         
            while ( use_value < prev_value && j < (int) results.size() ) {
               use_value = results[ j ];
               ++j;
            }

            if ( j == (int) results.size() ) {
               use_value = results[ 0 ];
            }
         }
         
         prev_value = use_value;
         efa_range_start[ i ]->setValue( use_value );

#if defined( DEBUG_INIT_EFA_RANGES )
         QTextStream ts( stdout );
         ts << QString( "init_efa forward sv %1 use pos %2" ).arg( i ).arg( use_value );
#endif
      }
   }

   prev_value = n - 1;

   for ( int i = 0; i < max_range; ++i ) {
      {
         vector < double > use_v = plotted_efa_rsv[ i ];

         vector < double > g = gradient( use_v );
         double gmin = vmin( g );
         double gminrecip = 1e0 / gmin;

         vector < double > results;

         for ( int j = 0; j < n; ++j ) {
            double gnorm = g[ j ] * gminrecip;
            // qDebug() << "gnorm[ " << j << " ] : " << gnorm;
            if ( gnorm > 0.05 + 0.05 * i ) {
               results.push_back( j );
            }
         }

#if defined( DEBUG_INIT_EFA_RANGES )
         {
            QTextStream ts( stdout );
            US_Vector::printvector( QString( "plotted_efa_rsv[%1]" ).arg( i ), use_v );
            US_Vector::printvector( QString( "gradient of plotted_efa_rsv[%1]" ).arg( i ), g );
            ts << "gmin : " << gmin;
            ts << "gminrecip : " << gminrecip;
            US_Vector::printvector( QString( "init efa forward sv %1" ).arg( i+1 ), results );
         }
#endif
         int use_value = prev_value;

         if ( !results.size() ) {
            use_value = prev_value - 1;
            if ( use_value < 0 ) {
               use_value = 0;
            }
            prev_value = use_value;
            efa_range_end[ i ]->setValue( use_value );
            // set to this prev_value
            continue;
         }

         use_value = results.back();

         if ( results.size() > 1 ) {
            int j = (int) results.size() - 1;
         
            while ( use_value > prev_value && j >= 0 ) {
               use_value = results[ j ];
               --j;
            }

            if ( j < 0 ) {
               use_value = results.back();
            }
         }
         
         prev_value = use_value;
         efa_range_end[ i ]->setValue( use_value );

#if defined( DEBUG_INIT_EFA_RANGES )
         QTextStream ts( stdout );
         ts << QString( "init_efa forward sv %1 use pos %2" ).arg( i ).arg( use_value );
#endif
      }
   }

   efa_range_processing = false;

   update_efa_range_start( 0e0 );
   update_efa_range_end( 0e0 );
}

void US_Hydrodyn_Mals_Saxs_Svd::set_number_of_svs_for_efa() {
   int max_range = (int) ( svd_D.size() > UHSHS_MAX_SV_FOR_EFA ? UHSHS_MAX_SV_FOR_EFA : svd_D.size() );
   if ( max_range < 1 ) {
      max_range = 1;
   }
   qwtc_efas->setRange( 1, max_range );

   int use_sv_count;
   int use_sv_U_count = 0;
   int use_sv_V_count = 0;

   for ( ; use_sv_U_count < (int) svd_autocor_U.size(); ++use_sv_U_count ) {
      if ( svd_autocor_U[ use_sv_U_count ] < SVD_THRESH ) {
         break;
      }
   }

   for ( ; use_sv_V_count < (int) svd_autocor_V.size(); ++use_sv_V_count ) {
      if ( svd_autocor_V[ use_sv_V_count ] < SVD_THRESH ) {
         break;
      }
   }

   use_sv_count = use_sv_U_count > use_sv_V_count ? use_sv_U_count : use_sv_V_count;
   if ( use_sv_count < 1 ) {
      use_sv_count = 1;
   }

   // US_Vector::printvector2( "US_Hydrodyn_Mals_Saxs_Svd::set_number_of_svs_for_efa svd_autocor_U, V", svd_autocor_U, svd_autocor_V );

   disconnect( qwtc_efas, SIGNAL( valueChanged( double ) ), 0, 0 );
   qwtc_efas->setValue( use_sv_count );
   connect( qwtc_efas, SIGNAL( valueChanged( double ) ), SLOT( update_efas( double ) ) );
}

bool US_Hydrodyn_Mals_Saxs_Svd::convert_it_to_iq( QStringList files, QStringList & created_files, QString & error_msg ) {

   double t_min = 0e0;
   double t_max = 1e99;

   QString head = mals_saxs_win->qstring_common_head( files, true );

   head = head.replace( QRegExp( "__It_q\\d*_$" ), "" );
   head = head.replace( QRegExp( "_q\\d*_$" ), "" );

   QRegExp rx_q     ( "_q(\\d+_\\d+)" );
   QRegExp rx_bl    ( "-bl(.\\d*_\\d+(|e.\\d+))-(.\\d*_\\d+(|e.\\d+))s" );
   QRegExp rx_bi    ( "-bi(.\\d*_\\d+(|e.\\d+))-(.\\d*_\\d+(|e.\\d+))s" );

   vector < QString > q_string;
   vector < double  > q;

   bool         any_bl = false;
   bool         any_bi = false;

   // get q 

   // map: [ timestamp ][ q_value ] = intensity

   map < double, map < double , double > > I_values;
   map < double, map < double , double > > e_values;

   map < double, bool > used_t;
   list < double >      tl;

   map < double, bool > used_q;
   list < double >      ql;

   bool                 use_errors = true;

   map < QString, bool >    no_errors;
   map < QString, QString > zero_points;
   QStringList              qsl_no_errors;
   QStringList              qsl_zero_points;

   bool                     mode_testiq = true;

   for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
   {
      if ( rx_q.indexIn( files[ i ] ) == -1 )
      {
         error_msg = QString( us_tr( "Error: Can not find q value in file name for %1" ) ).arg( files[ i ] );
         return false;
      }
      ql.push_back( rx_q.cap( 1 ).replace( "_", "." ).toDouble() );
      if ( used_q.count( ql.back() ) )
      {
         error_msg = QString( us_tr( "Error: Duplicate q value in file name for %1" ) ).arg( files[ i ] );
         return false;
      }
      used_q[ ql.back() ] = true;
         
      if ( rx_bl.indexIn( files[ i ] ) != -1 )
      {
         any_bl = true;
      }
      if ( rx_bi.indexIn( files[ i ] ) != -1 )
      {
         any_bi = true;
      }

      if ( !mals_saxs_win->f_qs.count( files[ i ] ) )
      {
         // error_msg = QString( us_tr( "Internal error: request to use %1, but not found in data" ) ).arg( files[ i ] );
      } else {
         for ( unsigned int j = 0; j < ( unsigned int ) mals_saxs_win->f_qs[ files[ i ] ].size(); j++ )
         {
            if ( !mode_testiq || ( mals_saxs_win->f_qs[ files[ i ] ][ j ] >= t_min && mals_saxs_win->f_qs[ files[ i ] ][ j ] <= t_max ) )
            {
               I_values[ mals_saxs_win->f_qs[ files[ i ] ][ j ] ][ ql.back() ] = mals_saxs_win->f_Is[ files[ i ] ][ j ];
               if ( use_errors && mals_saxs_win->f_errors[ files[ i ] ].size() == mals_saxs_win->f_qs[ files[ i ] ].size() )
               {
                  e_values[ mals_saxs_win->f_qs[ files[ i ] ][ j ] ][ ql.back() ] = mals_saxs_win->f_errors[ files[ i ] ][ j ];
               } else {
                  if ( use_errors )
                  {
                     use_errors = false;
                     // editor_msg( "dark red", QString( us_tr( "Notice: missing errors, first noticed in %1, so no errors at all" ) )
                     //             .arg( files[ i ] ) );
                  }
               }
               if ( !used_t.count( mals_saxs_win->f_qs[ files[ i ] ][ j ] ) )
               {
                  tl.push_back( mals_saxs_win->f_qs[ files[ i ] ][ j ] );
                  used_t[ mals_saxs_win->f_qs[ files[ i ] ][ j ] ] = true;
               }
            }
         }

         if ( !mals_saxs_win->f_errors.count( files[ i ] ) ||
              mals_saxs_win->f_errors[ files[ i ] ].size() != mals_saxs_win->f_Is[ files[ i ] ].size() )
         {
            no_errors[ files[ i ] ] = true;
            qsl_no_errors           << files[ i ];
         } else {
            if ( !mals_saxs_win->is_nonzero_vector( mals_saxs_win->f_errors[ files[ i ] ] ) )
            {
               unsigned int zero_pts = 0;
               for ( unsigned int j = 0; j < ( unsigned int ) mals_saxs_win->f_errors[ files[ i ] ].size(); j++ )
               {
                  if ( !mode_testiq || ( mals_saxs_win->f_qs[ files[ i ] ][ j ] >= t_min && mals_saxs_win->f_qs[ files[ i ] ][ j ] <= t_max ) )
                  {
                     if ( us_isnan( mals_saxs_win->f_errors[ files[ i ] ][ j ] ) || mals_saxs_win->f_errors[ files[ i ] ][ j ] == 0e0 )
                     {
                        zero_pts++;
                     }
                  }
               }
               zero_points[ files[ i ] ] = QString( "%1: %2 of %3 points" ).arg( files[ i ] ).arg( zero_pts ).arg( mals_saxs_win->f_errors[ files[ i ] ].size() );
               qsl_zero_points           << zero_points[ files[ i ] ];
            }
         }

      }
   }

   tl.sort();

   vector < double > tv;
   for ( list < double >::iterator it = tl.begin();
         it != tl.end();
         it++ )
   {
      tv.push_back( *it );
   }


   ql.sort();

   vector < double  > qv;
   vector < QString > qv_string;
   for ( list < double >::iterator it = ql.begin();
         it != ql.end();
         it++ )
   {
      qv.push_back( *it );
      qv_string.push_back( QString( "%1" ).arg( *it ) );
   }

   QString qs_no_errors;
   QString qs_zero_points;

   if ( zero_points.size() || no_errors.size() )
   {
      unsigned int used = 0;

      QStringList qsl_list_no_errors;

      for ( unsigned int i = 0; i < ( unsigned int ) qsl_no_errors.size() && i < 12; i++ )
      {
         qsl_list_no_errors << qsl_no_errors[ i ];
         used++;
      }
      if ( qsl_list_no_errors.size() < qsl_no_errors.size() )
      {
         qsl_list_no_errors << QString( us_tr( "... and %1 more not listed" ) ).arg( qsl_no_errors.size() - qsl_list_no_errors.size() );
      }
      qs_no_errors = qsl_list_no_errors.join( "\n" );
      
      QStringList qsl_list_zero_points;
      for ( unsigned int i = 0; i < ( unsigned int ) qsl_zero_points.size() && i < 24 - used; i++ )
      {
         qsl_list_zero_points << qsl_zero_points[ i ];
      }
      if ( qsl_list_zero_points.size() < qsl_zero_points.size() )
      {
         qsl_list_zero_points << QString( us_tr( "... and %1 more not listed" ) ).arg( qsl_zero_points.size() - qsl_list_zero_points.size() );
      }
      qs_zero_points = qsl_list_zero_points.join( "\n" );
   }

   // bool   normalize_by_conc = false;
   // bool   conc_ok           = false;

   // double conv = 0e0;
   // double psv  = 0e0;
   // double I0se = 0e0;
   // double conc_repeak = 1e0;
   
   vector < double > conc_spline_x;
   vector < double > conc_spline_y;
   vector < double > conc_spline_y2;

   running = true;

   // now for each I(t) distribute the I for each frame 

   // build up resulting curves

   // for each time, tv[ t ] 

   map < QString, bool > current_files;

   for ( unsigned int t = 0; t < tv.size(); t++ )
   {
      // progress->setValue( files.size() + t ); progress->setMaximum( files.size() + tv.size() );

      // build up an I(q)

      QString name = head + QString( "%1%2" )
         .arg( (any_bl || any_bi) ? "_bs" : "" )
         .arg( mals_saxs_win->pad_zeros( tv[ t ], (int) tv.size() ) )
         .replace( ".", "_" )
         ;

      {
         int ext = 0;
         QString use_name = name;
         while ( current_files.count( use_name ) )
         {
            use_name = name + QString( "-%1" ).arg( ++ext );
         }
         name = use_name;
      }
         
      // cout << QString( "name %1\n" ).arg( name );

      // now go through all the files to pick out the I values and errors and distribute amoungst the various gaussian peaks
      // we could also reassemble the original sum of gaussians curves as a comparative

      vector < double > I;
      vector < double > e;
      // vector < double > G;

      // vector < double > I_recon;
      // vector < double > G_recon;

      vector < double > this_used_pcts;
      // double conc_factor = 0e0;
      // if ( conc_ok ) {
      //    if ( !usu->apply_natural_spline( conc_spline_x, conc_spline_y, conc_spline_y2, tv[ t ], conc_factor ) ) {
      //       // editor_msg( "red", QString( us_tr( "Error getting concentration from spline for frame %1, concentration set to zero." ) ).arg( tv[ t ] ) );
      //       conc_factor = 0e0;
      //    }
      // }

      for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
      {
         if ( !I_values.count( tv[ t ] ) )
         {
            // editor_msg( "dark red", QString( us_tr( "Notice: I values missing frame/time = %1" ) ).arg( tv[ t ] ) );
         }

         if ( !I_values[ tv[ t ] ].count( qv[ i ] ) )
         {
            // editor_msg( "red", QString( us_tr( "Notice: I values missing q = %1" ) ).arg( qv[ i ] ) );
            continue;
         }

         double tmp_I       = I_values[ tv[ t ] ][ qv[ i ] ];
         double tmp_e       = 0e0;

         if ( use_errors )
         {
            if ( !e_values.count( tv[ t ] ) )
            {
               // editor_msg( "red", QString( us_tr( "Internal error: error values missing t %1" ) ).arg( tv[ t ] ) );
               // running = false;
               // update_enables();
               // progress->reset();
               error_msg = QString( us_tr( "Internal error: error values missing t %1" ) ).arg( tv[ t ] );
               return false;
            }

            if ( !e_values[ tv[ t ] ].count( qv[ i ] ) )
            {
               // editor_msg( "red", QString( us_tr( "Internal error: error values missing q %1" ) ).arg( qv[ i ] ) );
               // running = false;
               // update_enables();
               // progress->reset();
               error_msg = QString( us_tr( "Internal error: error values missing q %1" ) ).arg( qv[ i ] );
               return false;
            }

            tmp_e = e_values[ tv[ t ] ][ qv[ i ] ];
         }
            
         I      .push_back( tmp_I );
         e      .push_back( tmp_e );
      } // for each file
         
      /* from original
         QString this_name = mals_saxs_selected_files[ i ];
         original_data.push_back( this_name );
      
         f_pos      [ this_name ] = f_pos.size();
         f_qs_string[ this_name ] = mals_saxs_win->f_qs_string[ this_name ];
         f_qs       [ this_name ] = mals_saxs_win->f_qs       [ this_name ];
         f_Is       [ this_name ] = mals_saxs_win->f_Is       [ this_name ];
         f_errors   [ this_name ] = mals_saxs_win->f_errors   [ this_name ];
         f_is_time  [ this_name ] = mode_i_of_t;  // false;  // must all be I(q)
      */

      created_files.push_back( name );
      
      f_pos      [ name ] = f_pos.size();
      f_qs_string[ name ].clear();
      for ( int i = 0; i < (int) qv.size(); ++i ) {
         f_qs_string[ name ].push_back( QString( "%1" ).arg( qv[ i ] ) );
      }
      
      f_qs       [ name ] = qv;
      f_Is       [ name ] = I;
      f_errors   [ name ] = e;
      f_is_time  [ name ] = false;

   } // for each q value

   return true;
}


void US_Hydrodyn_Mals_Saxs_Svd::runExplicitEFARotation(
                                                       vector < vector < int > >    & M,
                                                       vector < vector < double > > & /* D */,
                                                       bool                         & failed, // also a return
                                                       vector < vector < double > > & C,
                                                       vector < vector < double > > & V_bar,
                                                       vector < vector < double > > & T,
                                                       int                            /* niter */,
                                                       double                         /* tol */,
                                                       vector < int >               & /* force_pos */,
                                                       // returns
                                                       vector < vector < double > > & /* C_ret */,
                                                       bool                         & converged
                                                       ) {
   /*
def runExplicitEFARotation(M, D, failed, C, V_bar, T, niter, tol, force_pos):
    print "SASCalc:runExplicitEFARotation"
    num_sv = M.shape[1]

    for i in range(num_sv):
        V_i_0 = V_bar[np.logical_not(M[:,i]),:]

        T[i,1:num_sv] = -np.dot(V_i_0[:,0].T, np.linalg.pinv(V_i_0[:,1:num_sv].T))

    C = np.dot(T, V_bar.T)

    C = C.T

    if -1*C.min() > C.max():
        C = C*-1

    converged = True

    csum = np.sum(M*C, axis = 0)
    if int(np.__version__.split('.')[0]) >= 1 and int(np.__version__.split('.')[1])>=10:
        C = C/np.broadcast_to(csum, C.shape) #normalizes by the sum of each column
    else:
        norm = np.array([csum for i in range(C.shape[0])])

        C = C/norm #normalizes by the sum of each column

    return C, failed, converged, None, None
   */
   // ----------------------------------------------------------------------------------------------------

   // qDebug() << "US_Hydrodyn_Mals_Saxs_Svd::runExplicitEFARotation\n";

   // ----------------------------------------------------------------------------------------------------
   int rows = (int) M.size();
   if ( !rows ) {
      editor_msg( "red", "Internal error: no rows [301]" );
      failed = true;
      return;
   }
      
   int num_sv = (int) M[0].size();
   QTextStream ts( stdout );

   for ( int i = 0; i < num_sv; ++i ) {
      vector < vector < double > > V_i_0;
      for ( int j = 0; j < rows; ++j ) {
         if ( !M[ j ][ i ] ) { 
            V_i_0.push_back( V_bar[ j ] );
         }
      }

      ts << "i = " << i << Qt::endl;
      SVD::cout_vvd( "V_i_0", V_i_0 );

      int vi0rowsize = (int) V_i_0.size();

      vector < vector < double > > Vitoinv( vi0rowsize );
      for ( int k = 0; k < vi0rowsize; ++k ) {
         for ( int l = 1; l < num_sv; ++l ) {
            Vitoinv[ k ].push_back( V_i_0[ k ][ l ] );
         }
      }

      Vitoinv = transpose( Vitoinv );

      vector < vector < double > > Vinv;

      // SVD::cout_vvd( "Vitoinv", Vitoinv );

      SVD::pinv( Vitoinv, Vinv );

      SVD::cout_vvd( "Vinv", Vinv );

      // T[i,1:num_sv] = -np.dot(V_i_0[:,0].T, np.linalg.pinv(V_i_0[:,1:num_sv].T))

      // compute multiple the 1st column of V_i_0 by the columns of Vinv

      for ( int k = 1; k < num_sv; ++k ) {
         T[ i ][ k ] = 0e0;
         for ( int l = 0; l < vi0rowsize; ++l ) {
            T[ i ][ k ] -= V_i_0[ l ][ 0 ] * Vinv[ l ][ k -1 ];
         }
      }

      SVD::cout_vvd( "T", T );
   }

   vector < vector < double > > V_barT = transpose( V_bar );
   
   C = dot( T, V_barT );

   C = transpose( C );

   SVD::cout_vvd( "C", C );

   if ( -1 * vvd_min( C ) > vvd_max( C ) ) {
      vvd_smult( C, -1 );
   }

   converged = true;
   
   SVD::cout_vvi( "M", M );
   SVD::cout_vvd( "C after min/max", C );
   vector < vector < double > > MC = vivd_pwmult( M, C );
   SVD::cout_vvd( "M*C", MC );

   vvd_cnorm( C, MC );

   SVD::cout_vvd( "Final C", C );
}


void US_Hydrodyn_Mals_Saxs_Svd::efa_decomp() {
   // init efa decomp bits
   // qDebug() << "efa_decomp()";
   editor_msg( "blue", us_tr( "EFA Components: Computing on " ) + last_svd_name );

   clear_efa_decomp();

   // TODO push to user interface
   int               niter      = 10000;
   double            tol        = 1e-12;
   efa_decomp_method method     = METHOD_HYBRID;
   // efa_decomp_method method     = METHOD_ITERATIVE;
   // efa_decomp_method method     = METHOD_EXPLICIT;

   int               num_sv     = (int) qwtc_efas->value();
   
   QStringList files = selected_files();

   last_efa_data = files;
   if ( !subset_data.count( files.join( "\n" ) ) )
   {
      files = add_subset_data( files );
   }
   set < QString > sel_sources = get_selected_sources();
   last_efa_name = *(sel_sources.begin());

   int m = (int) f_qs[ files[ 0 ] ].size();
   int n = (int) files.size();

   // need to transpose

   vector < vector < double > > D( n );

   disable_all();
   
   map < QString, vector < double > >  * f_Iuse = &f_Is;
   check_norm( files );
   QString norm_name = norm_name_map[ NORM_NOT ];
   if ( norm_ok ) {
      if ( cb_norm_pw->isChecked() ) {
         f_Iuse = &f_Is_norm_pw;
         norm_name = norm_name_map[ NORM_PW ];
      }
      if ( cb_norm_avg->isChecked() ) {
         f_Iuse = &f_Is_norm_avg;
         norm_name = norm_name_map[ NORM_AVG ];
      }
   }

   for ( int i = 0; i < n; ++i ) {
      D[ i ] = (*f_Iuse)[ files[ i ] ];
      if ( (int) D[ i ].size() != m ) {
         editor_msg( "red", QString( us_tr( "Inconsistent data length (e.g. 1st file has %1 data points but file %2 has %3 data points" )
                                     .arg( m )
                                     .arg( i )
                                     .arg( (int) D[ i ].size() )
                                     )
                     );
         update_enables();
         return;
      }
   }

   D = transpose( D );

   // qDebug() << "efa_decomp 1 num_sv " << num_sv;
   // qDebug() << "efa_decomp 1 efa_range_start.size() " << efa_range_start.size();
   // qDebug() << "efa_decomp 1 efa_range_end.size() " << efa_range_end.size();
   
   vector < vector < int > > M( num_sv );

   // compute component indices

   vector < int > start_vals;
   vector < int > end_vals;

   for ( int i = 0; i < num_sv; ++i ) {
      start_vals.push_back( efa_range_start[ i ]->value() );
      end_vals.push_back( efa_range_end[ i ]->value() );
   }
      
   sort( start_vals.begin(), start_vals.end() );
   sort( end_vals.begin(), end_vals.end() );

   US_Vector::printvector2( "start value, end vals", start_vals, end_vals );

   for ( int i = 0; i < num_sv; ++i ) {
      if ( start_vals[ i ]  >= end_vals[ i ] ) {
         editor_msg( "red", QString( us_tr( "Ranges for component %1 is empty or negative.  Please correct.  Note that the component ranges are in reverse order to the S.V's ranges (i.e. the start frame for this component is S.V. %2 and the end frame is S.V. %3.)" ) ).arg( i + 1 ).arg( i + 1 ).arg( num_sv - i ) );
         update_enables();
         return;
      }
   }

   for ( int i = 0; i < num_sv; ++i ) {
      M[ i ].resize( n );
      for ( int j = 0; j < n; ++j ) {
         M[ i ][ j ] = ( j >= start_vals[ i ] && j <= end_vals[ i ] ) ? 1 : 0;
      }
   }
   // qDebug() << "efa_decomp 2";
   M = transpose( M );
   // qDebug() << "efa_decomp 3";

   // TODO : might have an extra transpose here
   vector < vector < double > > V_bar = transpose( svd_V );
   V_bar.resize( num_sv );
   V_bar = transpose( V_bar );
   
   vector < vector < double > > C = V_bar;
   vector < vector < double > > C_ret;
   vector < vector < double > > T;

   bool converged = false;
   bool failed    = false;
   
   /*
    print "SASCalc:initHydridEFA M:", M.shape, M.tolist()
    print "SASCalc:initHydridEFA D:", D.shape, D
    print "SASCalc:initHydridEFA C:", C.shape, C
    print "SASCalc:initHydridEFA V_bar:", V_bar.shape, V_bar
    print "SASCalc:initHydridEFA num_sv:", num_sv
    print "SASCalc:initHydridEFA converged:", converged
   */

   SVD::cout_vvi( "efa_decomp before call M", M );
   //   SVD::cout_vvd( "efa_decomp before call D", D );
   SVD::cout_vvd( "efa_decomp before call C", C );
   SVD::cout_vvd( "efa_decomp before call V_bar", V_bar );
   {
      QTextStream ts( stdout );
      ts << "efa_decomp before call num_sv " << num_sv << Qt::endl;
   }

   vector < int >     force_pos( num_sv );
   vector < double >  dc;
   int                k;
   
   if ( cb_efa_decomp_force_positive->isChecked() ) {
      vector < int >  force_pos1( num_sv, 1 );
      force_pos = force_pos1;
   }

   switch ( method ) {
   case METHOD_HYBRID :
      {
         initHybridEFA( M, num_sv, D, C, converged, V_bar, niter, tol, force_pos, failed, C_ret, T );
         if ( !failed ) {
            runIterativeEFARotation( M, D, failed, C, V_bar, T, niter, tol, force_pos, C_ret, converged, dc, k );
         }
      }
      break;
   case METHOD_ITERATIVE :
      {
         initIterativeEFA( M, num_sv, D, C, converged, V_bar, failed, C_ret );
         if ( !failed ) {
            runIterativeEFARotation( M, D, failed, C, V_bar, T, niter, tol, force_pos, C_ret, converged, dc, k );
         }
      }
      break;
   case METHOD_EXPLICIT :
      {
         initExplicitEFA( M, num_sv, D, C, converged, V_bar, failed, T );
         if ( !failed ) {
            runExplicitEFARotation( M, D, failed, C, V_bar, T, niter, tol, force_pos, C_ret, converged );
         }
       }
      break;
   }

   // plot

   efa_decomp_Ct = transpose( C );

   for ( int i = 0; i < (int) C.size(); ++i ) {
      efa_decomp_x.push_back( i );
   }

   efa_decomp_plot();
      
   mode_select( MODE_EFA_DECOMP );

   if ( !converged ) {
      editor_msg( "red", us_tr( "EFA Components: failed to converge" ) );
   }
      
   editor_msg( "blue", us_tr( "EFA Components: Done computing on " ) + last_svd_name );

   update_enables();
}


void US_Hydrodyn_Mals_Saxs_Svd::efa_decomp_plot() {
   if ( !efa_decomp_x.size() ) {
      return;
   }

   if ( plot_efa_decomp_zoomer )
   {
      // cout << QString( "plot zoomer stack size %1\n" ).arg( plot_efa_decomp_zoomer->zoomRectIndex() );
      if ( !plot_efa_decomp_zoomer->zoomRectIndex() )
      {
         plot_efa_decomp_zoomer->zoom ( 0 );
         delete plot_efa_decomp_zoomer;
         plot_efa_decomp_zoomer = (ScrollZoomer *) 0;
      }
   }
   
   for ( int i = 0; i < (int) efa_decomp_Ct.size(); ++i ) {
      QwtPlotCurve *curve = new QwtPlotCurve( QString( "EFA Deconvolution SV %1" ).arg( i + 1 ) );
      curve->setStyle( QwtPlotCurve::Lines );


      curve->setSamples(
                        (double *)&( efa_decomp_x[ 0 ] ),
                        (double *)&( efa_decomp_Ct[ i ][ 0 ] ),
                        svd_x.size()
                        );

      curve->setPen( QPen( plot_colors[ i % plot_colors.size() ], use_line_width, Qt::SolidLine ) );
      curve->attach( plot_efa_decomp );
   }

   //   plot_data->setAxisScale( QwtPlot::xBottom, 1, svd_x.size() );
   plot_efa_decomp->setAxisScale( QwtPlot::xBottom, efa_decomp_x[0] - 0.5, efa_decomp_x.back() + 0.5 );
   plot_efa_decomp->setAxisScale( QwtPlot::yLeft  , vvd_min( efa_decomp_Ct ) - 0.05, vvd_max( efa_decomp_Ct ) + 0.05 );

   if ( !plot_efa_decomp_zoomer ) {
      plot_efa_decomp_zoomer = new ScrollZoomer(plot_efa_decomp->canvas());
      plot_efa_decomp_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
      // connect( plot_efa_decomp_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_efa_decomp_zoomed( const QRectF & ) ) );
   }

   set_title( plot_efa_decomp, QString( "EFA Deconvolution of %1" ).arg( last_svd_name ) );
   plot_efa_decomp->replot();
}

void US_Hydrodyn_Mals_Saxs_Svd::initExplicitEFA(
                                                vector < vector < int > >    & /* M */,
                                                int                            num_sv,
                                                vector < vector < double > > & /* D */,
                                                vector < vector < double > > & /* C */,
                                                bool                         & /* converged */,
                                                vector < vector < double > > & /* V_bar */,
                                                // returns
                                                bool                         & /* failed */,
                                                vector < vector < double > > & T_ret
                                                 ) {
   /*
def initExplicitEFA(M, num_sv, D, C, converged, V_bar):
    print "SASCalc:initExplicitEFA"

    T = np.ones((num_sv, num_sv))

    failed = False

    return failed, None, T
   */
   // qDebug() << "US_Hydrodyn_Mals_Saxs_Svd::initExplicitEFA\n";
   T_ret.resize( num_sv );
   for ( int i = 0; i < num_sv; ++i ) {
      T_ret[ i ].resize( num_sv );
      for ( int j = 0; j < num_sv; ++j ) {
         T_ret[ i ][ j ] = 1e0;
      }
   }
}


void US_Hydrodyn_Mals_Saxs_Svd::initIterativeEFA(
                                                 vector < vector < int > >   & M,
                                                 int                           /* num_sv */,
                                                 vector < vector < double > > & D,
                                                 vector < vector < double > > & C,
                                                 bool                         & /* converged */,
                                                 vector < vector < double > > & /* V_bar */,
                                                 // returns
                                                 bool                         & /* failed */,
                                                 vector < vector < double > > & C_ret
                                                 ) {
   /*
def initIterativeEFA(M, num_sv, D, C, converged, V_bar):
    print "SASCalc:initIterativeEFA"

    #Set a variable to test whether the rotation fails for a numerical reason
    failed = False

    #Do an initial rotation
    try:
        C = EFAFirstRotation(M, C, D)
    except np.linalg.linalg.LinAlgError:
        failed = True

    return failed, C, None
   */
   // qDebug() << "US_Hydrodyn_Mals_Saxs_Svd::initIterativeEFA\n";
   EFAFirstRotation( M, C, D, C_ret );
}

void US_Hydrodyn_Mals_Saxs_Svd::EFAFirstRotation(
                                                 vector < vector < int > >    & M,
                                                 vector < vector < double > > & C,
                                                 vector < vector < double > > & D,
                                                 // returns
                                                 vector < vector < double > > & Cnew
                                                 ) {
   /*
def EFAFirstRotation(M,C,D):
    print "SASCalc:EFAFirstRotation"
    #Have to run an initial rotation without forcing C>=0 or things typically fail to converge (usually the SVD fails)
    S = np.dot(D, np.linalg.pinv(np.transpose(M*C)))

    Cnew = np.transpose(np.dot(np.linalg.pinv(S), D))

    csum = np.sum(M*Cnew, axis = 0)
    if int(np.__version__.split('.')[0]) >= 1 and int(np.__version__.split('.')[1])>=10:
        Cnew = Cnew/np.broadcast_to(csum, Cnew.shape) #normalizes by the sum of each column
    else:
        norm = np.array([csum for i in range(Cnew.shape[0])])

        Cnew = Cnew/norm #normalizes by the sum of each column

    return Cnew
   */
   // qDebug() << "US_Hydrodyn_Mals_Saxs_Svd::EFAFirstRotation\n";
   // SVD::cout_vvi( "FirstRotation M", M );
   // SVD::cout_vvd( "FirstRotation C", C );

   vector < vector < double > > MCT = vivd_pwmult( M, C );
   MCT = transpose( MCT );
   vector < vector < double > > MCTpinv;
   SVD::pinv( MCT, MCTpinv );

   // SVD::cout_vvd( "FirstRotation MCTpinv", MCTpinv );
   
   vector < vector < double > > S = dot( D, MCTpinv );
   vector < vector < double > > Spinv;
   SVD::pinv( S, Spinv );

   // SVD::cout_vvd( "FirstRotation S", S );

   Cnew = dot( Spinv, D );
   Cnew = transpose( Cnew );
   
   //   SVD::cout_vvd( "FirstRotation Cnew", Cnew );
}

void US_Hydrodyn_Mals_Saxs_Svd::initHybridEFA(
                                              vector < vector < int > >    & M,
                                              int                            num_sv,
                                              vector < vector < double > > & D,
                                              vector < vector < double > > & C,
                                              bool                         & converged,
                                              vector < vector < double > > & V_bar,
                                              int                            niter,
                                              double                         tol,
                                              vector < int >               & force_pos,
                                              // returns
                                              bool                         & failed,
                                              vector < vector < double > > & C_ret,
                                              vector < vector < double > > & /* T_ret */
                                              ) {
   /*
def initHybridEFA(M, num_sv, D, C, converged, V_bar):
    print "SASCalc:initHydridEFA"
    failed = False

    if not converged:
        failed, temp, T = initExplicitEFA(M, num_sv, D, C, converged, V_bar)
        C, failed, temp1, temp2, temp3 = runExplicitEFARotation(M, None, None, None, V_bar, T, None, None, None)

    return failed, C, None
   */
   // qDebug() << "US_Hydrodyn_Mals_Saxs_Svd::initHybridEFA\n";

   vector < vector < double > > T;

   initExplicitEFA(M, num_sv, D, C, converged, V_bar, failed, T );
   runExplicitEFARotation( M, D, failed, C, V_bar, T, niter, tol, force_pos, C_ret, converged );
}

   
void US_Hydrodyn_Mals_Saxs_Svd::runIterativeEFARotation(
                                                        vector < vector < int > >    & M,
                                                        vector < vector < double > > & D,
                                                        bool                         & failed, // also a return
                                                        vector < vector < double > > & C,
                                                        vector < vector < double > > & /* V_bar */,
                                                        vector < vector < double > > & /* T */,
                                                        int                            niter,
                                                        double                         tol,
                                                        vector < int >               & force_pos,
                                                        // returns
                                                        vector < vector < double > > & /* C_ret */,
                                                        bool                         & converged,
                                                        vector < double >            & dc,
                                                        int                          & k
                                                        ) {
   /*
def runIterativeEFARotation(M, D, failed, C, V_bar, T, niter, tol, force_pos):
    print "SASCalc:runIterativeEFARotation"
    #Carry out the calculation to convergence
    k = 0
    converged = False

    dc = []

    while k < niter and not converged and not failed:
        k = k+1
        try:
            Cnew = EFAUpdateRotation(M, C, D, force_pos )
        except np.linalg.linalg.LinAlgError:
           failed = True

        dck = np.sum(np.abs(Cnew - C))

        dc.append(dck)

        C = Cnew

        if dck < tol:
            converged = True

    return C, failed, converged, dc, k
   */
   // qDebug() << "US_Hydrodyn_Mals_Saxs_Svd::runIterativeEFARotation\n";
   k = 0;
   converged = false;
   failed = false;

   dc.clear();

   vector < vector < double > > Cnew;

   int m = C.size();
   if ( !C.size() ) {
      editor_msg( "red", us_tr( "Internal error: empty C in efa components routine, please inform the developers" ) );
      failed = true;
      return;
   }

   int n = C[ 0 ].size();

   while ( k < niter && !converged && !failed ) {
      ++k;
      EFAUpdateRotation( M, C, D, force_pos, Cnew );

      double dck = 0e0;
      for ( int i = 0; i < m; ++i ) {
         for ( int j = 0; j < n; ++j ) {
            dck += fabs( C[ i ][ j ] - Cnew[ i ][ j ] );
         }
      }

      dc.push_back( dck );

      C = Cnew;

      if ( dck < tol ) {
         converged = true;
      }
   }

   // SVD::cout_vvd( "runIterativeEFARotation final EFAUpdateRotation C", C );
}

void US_Hydrodyn_Mals_Saxs_Svd::EFAUpdateRotation(
                                                  vector < vector < int > >    & M,
                                                  vector < vector < double > > & C,
                                                  vector < vector < double > > & D,
                                                  vector < int >               & force_pos,
                                                  // returns
                                                  vector < vector < double > > & Cnew
                                                  ) {
   /*
def EFAUpdateRotation(M,C,D, force_pos):
    print "SASCalc:EFAUpdateRotation"
    S = np.dot(D, np.linalg.pinv(np.transpose(M*C)))

    Cnew = np.transpose(np.dot(np.linalg.pinv(S), D))

    for i, fp in enumerate(force_pos):
        if fp:
            Cnew[Cnew[:,i] < 0,i] = 0

    csum = np.sum(M*Cnew, axis = 0)

    if int(np.__version__.split('.')[0]) >= 1 and int(np.__version__.split('.')[1])>=10:
        Cnew = Cnew/np.broadcast_to(csum, Cnew.shape) #normalizes by the sum of each column
    else:
        norm = np.array([csum for i in range(Cnew.shape[0])])

        Cnew = Cnew/norm #normalizes by the sum of each column

    return Cnew
   */
   // qDebug() << "US_Hydrodyn_Mals_Saxs_Svd::EFAUpdateRotation\n";

   // SVD::cout_vvi( "UpdateRotation M", M );
   // SVD::cout_vvd( "UpdateRotation C", C );

   vector < vector < double > > MCT = vivd_pwmult( M, C );
   MCT = transpose( MCT );
   vector < vector < double > > MCTpinv;
   SVD::pinv( MCT, MCTpinv );

   // SVD::cout_vvd( "UpdateRotation MCTpinv", MCTpinv );
   
   vector < vector < double > > S = dot( D, MCTpinv );
   vector < vector < double > > Spinv;
   SVD::pinv( S, Spinv );

   // SVD::cout_vvd( "UpdateRotation S", S );

   Cnew = dot( Spinv, D );
   Cnew = transpose( Cnew );
   
   // SVD::cout_vvd( "UpdateRotation Cnew", Cnew );

   {
      int rows = (int) Cnew.size();
      for ( int i = 0; i < (int) force_pos.size(); ++i ) {
         if ( force_pos[ i ] ) {
            for ( int j = 0; j < rows; ++j ) {
               if ( Cnew[ j ][ i ] < 0e0 ) {
                  Cnew[ j ][ i ] = 0e0;
               }
            }
         }
      }
   }

   vector < vector < double > > MCnew = vivd_pwmult( M, Cnew );
   
   vvd_cnorm( Cnew, MCnew );

   // SVD::cout_vvd( "UpdateRotation end Cnew", Cnew );
}

void US_Hydrodyn_Mals_Saxs_Svd::efa_decomp_to_mals_saxs_saxs() {
   // qDebug() << "efa_decomp_to_mals_saxs_saxs";
   for ( int i = 0; i < (int) efa_decomp_Ct.size(); ++i ) {
      QString this_name = QString( "EFA_of_%1_component_%2" ).arg( last_efa_name.replace( " ", "_" ) ).arg( i + 1 );
      mals_saxs_win->add_plot( this_name,
                          efa_decomp_x,
                          efa_decomp_Ct[ i ],
                          true );
      editor_msg( "dark blue", QString( "Added %1 to HPLC window" ).arg( this_name ) );
   }
}

void US_Hydrodyn_Mals_Saxs_Svd::set_efa_decomp_force_positive() {
   // qDebug() << "efa_decomp_force_positive";
   efa_decomp();
}
