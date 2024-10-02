#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_best.h"
// #include "../include/us_vector.h"
#include "../include/us_lm.h"

#if defined(WIN32)
#  include <dos.h>
#  include <stdlib.h>
#  include <float.h>
//Added by qt3to4:
#include <QBoxLayout>
#include <QLabel>
#include <QCloseEvent>
#include <QTextStream>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
 //#include <Q3PopupMenu>
// #  define isnan _isnan
#endif

namespace BFIT
{
   double compute_f( double x, const double *par )
   {
      return par[ 0 ] + par[ 1 ] * exp( x * par[ 2 ] );
   }
}

US_Hydrodyn_Best::US_Hydrodyn_Best(
                                   void                     *              us_hydrodyn,
                                   bool                     *              best_widget,
                                   QWidget *                               p,
                                   const char *                            
                                   ) : QFrame(  p )
{
   this->us_hydrodyn                          = us_hydrodyn;
   this->best_widget                          = best_widget;
   if ( !best_widget )
   {
      return;
   }
   *best_widget = true;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle( us_tr( "US-SOMO: BEST results analysis tool" ) );

   cg_red = USglobal->global_colors.cg_label;
   cg_red.setBrush( QPalette::WindowText, QBrush( QColor( "red" ),  Qt::SolidPattern ) );

   plot_data_zoomer      = (ScrollZoomer *) 0;

   tau_inputs
      << "EIGENVALUES OF Drr TENSOR (1/s) [1]"
      << "EIGENVALUES OF Drr TENSOR (1/s) [2]"
      << "EIGENVALUES OF Drr TENSOR (1/s) [3]"
      ;

   for ( int i = 0; i < (int) tau_inputs.size(); ++i )
   {
      tau_input_set.insert( tau_inputs[ i ] );
   }

   tau_msg
      << "Tau (1) (ns)"
      << "Tau (2) (ns)"
      << "Tau (3) (ns)"
      << "Tau (4) (ns)"
      << "Tau (5) (ns)"
      << "Tau (h) (ns)"
      << "Tau (m) (ns)"
      ;

   Qc95.resize( 11 );
   Qc95[ 0  ] = 0e0;
   Qc95[ 1  ] = 0e0;
   Qc95[ 2  ] = 0e0;
   Qc95[ 3  ] = 0.941e0;
   Qc95[ 4  ] = 0.765e0;
   Qc95[ 5  ] = 0.642e0;
   Qc95[ 6  ] = 0.56e0;
   Qc95[ 7  ] = 0.507e0;
   Qc95[ 8  ] = 0.47e0;
   Qc95[ 9  ] = 0.44e0;
   Qc95[ 10 ] = 0.41e0;

   Qc80.resize( 11 );
   Qc80[ 0  ] = 0e0;
   Qc80[ 1  ] = 0e0;
   Qc80[ 2  ] = 0e0;
   Qc80[ 3  ] = 0.781e0;
   Qc80[ 4  ] = 0.560e0;
   Qc80[ 5  ] = 0.451e0;
   Qc80[ 6  ] = 0.386e0;
   Qc80[ 7  ] = 0.344e0;
   Qc80[ 8  ] = 0.385e0;
   Qc80[ 9  ] = 0.352e0;
   Qc80[ 10 ] = 0.325e1;

   Qc70.resize( 11 );
   Qc70[ 0  ] = 0e0;
   Qc70[ 1  ] = 0e0;
   Qc70[ 2  ] = 0e0;
   Qc70[ 3  ] = 0.684e0;
   Qc70[ 4  ] = 0.471e0;
   Qc70[ 5  ] = 0.373e0;
   Qc70[ 6  ] = 0.318e0;
   Qc70[ 7  ] = 0.281e0;
   Qc70[ 8  ] = 0.318e0;
   Qc70[ 9  ] = 0.288e0;
   Qc70[ 10 ] = 0.265e1;

   setupGUI();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 800, 600 );
}

US_Hydrodyn_Best::~US_Hydrodyn_Best()
{
}

void US_Hydrodyn_Best::setupGUI()
{
   int minHeight1 = 24;
#if QT_VERSION < 0x040000 || !defined(Q_OS_MAC)
   int minHeight3 = 25;
#endif

   lbl_credits_1 =  new QLabel      ( "Cite: Aragon, S.R. \"A precise boundary element method for macromolecular transport properties\", J. Comp. Chem, (2004) 25, 1191-1205", this );
   lbl_credits_1 -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_credits_1 -> setMinimumHeight( minHeight1 );
   lbl_credits_1 -> setPalette      ( PALET_LABEL );
   AUTFBACK( lbl_credits_1 );
   lbl_credits_1 -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   lbl_credits_2 =  new QLabel      ( "Cite: Connolly, M.L. \"The molecular surface package\", J. Mol. Graph. (1993) 11: 139-141", this );
   lbl_credits_2 -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_credits_2 -> setMinimumHeight( minHeight1 );
   lbl_credits_2 -> setPalette      ( PALET_LABEL );
   AUTFBACK( lbl_credits_2 );
   lbl_credits_2 -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   // ------ input section 
   lbl_input = new mQLabel("Data fields", this);
   lbl_input->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_input->setMinimumHeight(minHeight1);
   lbl_input->setPalette( PALET_LABEL );
   AUTFBACK( lbl_input );
   lbl_input->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   connect( lbl_input, SIGNAL( pressed() ), SLOT( hide_input() ) );

   lb_data = new QListWidget( this );
   lb_data->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lb_data->setPalette( PALET_EDIT );
   AUTFBACK( lb_data );
   lb_data->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lb_data->setEnabled(true);
   connect( lb_data, SIGNAL( itemSelectionChanged() ), SLOT( data_selected() ) );

   input_widgets.push_back( lb_data );

   pb_load =  new QPushButton ( us_tr( "Load CSV" ), this );
   pb_load -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_load -> setMinimumHeight( minHeight1 );
   pb_load -> setPalette      ( PALET_PUSHB );
   connect( pb_load, SIGNAL( clicked() ), SLOT( load() ) );

   input_widgets.push_back( pb_load );

   pb_join_results =  new QPushButton ( us_tr( "Join results" ), this );
   pb_join_results -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_join_results -> setMinimumHeight( minHeight1 );
   pb_join_results -> setPalette      ( PALET_PUSHB );
   connect( pb_join_results, SIGNAL( clicked() ), SLOT( join_results() ) );

   input_widgets.push_back( pb_join_results );

   pb_save_results =  new QPushButton ( us_tr( "Save Results" ), this );
   pb_save_results -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_save_results -> setMinimumHeight( minHeight1 );
   pb_save_results -> setPalette      ( PALET_PUSHB );
   pb_save_results -> setEnabled      ( false );
   connect( pb_save_results, SIGNAL( clicked() ), SLOT( save_results() ) );

   input_widgets.push_back( pb_save_results );

   le_last_file = new QLineEdit(this);
   save_last_file = "";
   le_last_file->setText( save_last_file );
   le_last_file->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_last_file->setMinimumHeight(minHeight1);
   le_last_file->setPalette( PALET_NORMAL );
   AUTFBACK( le_last_file );
   le_last_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   connect( le_last_file, SIGNAL( textChanged( const QString & ) ), SLOT( set_last_file( const QString & ) ) );

   input_widgets.push_back( le_last_file );

   cb_plus_lm = new QCheckBox( this );
   cb_plus_lm->setPalette( PALET_NORMAL );
   AUTFBACK( cb_plus_lm );
   cb_plus_lm->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   cb_plus_lm->setText( us_tr( "EXP fit with LM refinement" ) );
   cb_plus_lm->setChecked( false );
   cb_plus_lm->setEnabled( true );
   // cb_plus_lm->show();
   cb_plus_lm->hide();
   connect( cb_plus_lm, SIGNAL( clicked() ), SLOT( data_selected() ) );
   //   input_widgets.push_back( cb_plus_lm );

   cb_errorlines = new QCheckBox( this );
   cb_errorlines->setPalette( PALET_NORMAL );
   AUTFBACK( cb_errorlines );
   cb_errorlines->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   cb_errorlines->setText( us_tr( "Display error lines (+/- 1 sigma of linear fit)" ) );
   cb_errorlines->setChecked( false );
   cb_errorlines->setEnabled( true );
   cb_errorlines->show();
   connect( cb_errorlines, SIGNAL( clicked() ), SLOT( data_selected() ) );
   input_widgets.push_back( cb_errorlines );

   cb_manual_rejection = new QCheckBox( this );
   cb_manual_rejection->setPalette( PALET_NORMAL );
   AUTFBACK( cb_manual_rejection );
   cb_manual_rejection->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   cb_manual_rejection->setText( us_tr( "Allow manual point rejection" ) );
   cb_manual_rejection->setChecked( false );
   cb_manual_rejection->setEnabled( true );
   cb_manual_rejection->show();
   connect( cb_manual_rejection, SIGNAL( clicked() ), SLOT( set_manual_rejection() ) );
   input_widgets.push_back( cb_manual_rejection );

   pb_apply_qtest =  new QPushButton ( us_tr( "Apply Q test criterion" ), this );
   pb_apply_qtest -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_apply_qtest -> setMinimumHeight( minHeight1 );
   pb_apply_qtest -> setPalette      ( PALET_PUSHB );
   pb_apply_qtest -> setEnabled      ( false );
   connect( pb_apply_qtest, SIGNAL( clicked() ), SLOT( apply_qtest() ) );
   input_widgets.push_back( pb_apply_qtest );

   pb_reset_qtest =  new QPushButton ( us_tr( "Reset" ), this );
   pb_reset_qtest -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_reset_qtest -> setMinimumHeight( minHeight1 );
   pb_reset_qtest -> setPalette      ( PALET_PUSHB );
   pb_reset_qtest -> setEnabled      ( false );
   connect( pb_reset_qtest, SIGNAL( clicked() ), SLOT( reset_qtest() ) );
   input_widgets.push_back( pb_reset_qtest );

   rb_90_qtest = new QRadioButton ( this );
   rb_90_qtest->setPalette( PALET_NORMAL );
   AUTFBACK( rb_90_qtest );
   rb_90_qtest->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   rb_90_qtest->setText( us_tr( "90%" ) );
   rb_90_qtest->setChecked( true );
   rb_90_qtest->setEnabled( true );
   rb_90_qtest->show();
   input_widgets.push_back( rb_90_qtest );

   rb_80_qtest = new QRadioButton ( this );
   rb_80_qtest->setPalette( PALET_NORMAL );
   AUTFBACK( rb_80_qtest );
   rb_80_qtest->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   rb_80_qtest->setText( us_tr( "80%" ) );
   rb_80_qtest->setEnabled( true );
   rb_80_qtest->show();
   connect( rb_80_qtest, SIGNAL( clicked() ), SLOT( set_loose_qtest() ) );
   input_widgets.push_back( rb_80_qtest );

   rb_70_qtest = new QRadioButton ( this );
   rb_70_qtest->setPalette( PALET_NORMAL );
   AUTFBACK( rb_70_qtest );
   rb_70_qtest->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   rb_70_qtest->setText( us_tr( "70% test level" ) );
   rb_70_qtest->setEnabled( true );
   rb_70_qtest->show();
   connect( rb_70_qtest, SIGNAL( clicked() ), SLOT( set_loose_qtest() ) );
   input_widgets.push_back( rb_70_qtest );

#if QT_VERSION < 0x040000
   bg_qtest_level = new QGroupBox( this );
   int bg_pos = 0;
   bg_qtest_level->setExclusive(true);
   bg_qtest_level->addButton( rb_90_qtest, bg_pos++ );
   bg_qtest_level->addButton( rb_80_qtest, bg_pos++ );
   bg_qtest_level->addButton( rb_70_qtest, bg_pos++ );
#else
   bg_qtest_level = new QGroupBox();
   bg_qtest_level->setFlat( true );

   {
      QVBoxLayout * bl = new QVBoxLayout; bl->setContentsMargins( 0, 0, 0, 0 ); bl->setSpacing( 0 );
      bl->addWidget( rb_90_qtest );
      bl->addWidget( rb_80_qtest );
      bl->addWidget( rb_70_qtest );
      bg_qtest_level->setLayout( bl );
   }
#endif

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

      mb_editor = new QMenuBar( this );
      AUTFBACK( mb_editor );

      mb_editor->insertItem(us_tr("&Messages"), file );
   }
# else
   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight3);
   editor_widgets.push_back( frame );

   mb_editor = new QMenuBar( frame );    mb_editor->setObjectName( "menu" );
   mb_editor->setMinimumHeight(minHeight1 - 5);
   mb_editor->setPalette( PALET_NORMAL );
   AUTFBACK( mb_editor );

 //   Q3PopupMenu * file = new Q3PopupMenu(editor);
   mb_editor->insertItem( us_tr("&File"), file );
   file->insertItem( us_tr("Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
   file->insertItem( us_tr("Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );
   file->insertItem( us_tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );
# endif
#else
# if defined(Q_OS_MAC)
   mb_editor = new QMenuBar( this );
   mb_editor->setObjectName( "menu" );
# else
   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight3);
   frame->setPalette( PALET_NORMAL );
   AUTFBACK( frame );

   editor_widgets.push_back( frame );

   mb_editor = new QMenuBar( frame );    mb_editor->setObjectName( "menu" );
#endif
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
#endif

   editor->setWordWrapMode (QTextOption::WordWrap);
   editor->setMinimumHeight( minHeight1 * 3 );

   editor_widgets.push_back( editor );

   // ------ plot section
//   plot_data = new QwtPlot(this);
   usp_plot_data = new US_Plot( plot_data, "", "", "", this );
   connect( (QWidget *)plot_data->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_data( const QPoint & ) ) );
   ((QWidget *)plot_data->titleLabel())->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_data->axisWidget( QwtPlot::yLeft ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_data( const QPoint & ) ) );
   ((QWidget *)plot_data->axisWidget( QwtPlot::yLeft ))->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_data->axisWidget( QwtPlot::xBottom ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_data( const QPoint & ) ) );
   ((QWidget *)plot_data->axisWidget( QwtPlot::xBottom ))->setContextMenuPolicy( Qt::CustomContextMenu );
#if QT_VERSION < 0x040000
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
#if QT_VERSION < 0x040000
   plot_data->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot_data->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   grid_data->setMajorPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid_data->setMinorPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid_data->attach( plot_data );
#endif
   plot_data->setAxisTitle(QwtPlot::xBottom, us_tr( "1/Triangles"      ) ); 
   plot_data->setAxisTitle(QwtPlot::yLeft,   us_tr( "Parameter [a.u.]" ) );
#if QT_VERSION < 0x040000
   plot_data->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   plot_data->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_data->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   plot_data->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_data->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   plot_data->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_data->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
//    plot_data->setMargin(USglobal->config_list.margin);
   plot_data->setTitle("");
#if QT_VERSION < 0x040000
   plot_data->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::None);
#else
   plot_data->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine );
#endif
   plot_data->setCanvasBackground(USglobal->global_colors.plot);

   lbl_points = new mQLabel( "Linear:", this );
   lbl_points->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_points );
   lbl_points->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_points->show();
   connect( lbl_points, SIGNAL( pressed() ), SLOT( toggle_points() ) );

   lbl_points_ln = new mQLabel( "Log:   ", this );
   lbl_points_ln->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_points_ln );
   lbl_points_ln->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   // lbl_points_ln->show();
   lbl_points_ln->hide();
   connect( lbl_points_ln, SIGNAL( pressed() ), SLOT( toggle_points_ln() ) );

   lbl_points_exp = new mQLabel( "Exp:   ", this );
   lbl_points_exp->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_points_exp );
   lbl_points_exp->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   // lbl_points_exp->show();
   lbl_points_exp->hide();
   connect( lbl_points_exp, SIGNAL( pressed() ), SLOT( toggle_points_exp() ) );

   // connect( plot_data_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_data_zoomed( const QRectF & ) ) );

   pb_help =  new QPushButton ( us_tr( "Help" ), this );
   pb_help -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_help -> setMinimumHeight( minHeight1 );
   pb_help -> setPalette      ( PALET_PUSHB );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

   pb_close =  new QPushButton ( us_tr( "Close" ), this );
   pb_close -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_close -> setMinimumHeight( minHeight1 );
   pb_close -> setPalette      ( PALET_PUSHB );
   connect( pb_close, SIGNAL( clicked() ), SLOT( cancel() ) );

   // -------- build layout

   QVBoxLayout * background = new QVBoxLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );
   QHBoxLayout * top = new QHBoxLayout(); top->setContentsMargins( 0, 0, 0, 0 ); top->setSpacing( 0 );

   // ----- left side
   {
      QBoxLayout * bl = new QVBoxLayout( 0 ); bl->setContentsMargins( 0, 0, 0, 0 ); bl->setSpacing( 0 );
      bl->addWidget( lbl_input );
      bl->addWidget( lb_data );
      {
         QHBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
         hbl->addWidget( pb_load );
         hbl->addWidget( pb_join_results );
         hbl->addWidget( pb_save_results );
         bl->addLayout( hbl );
      }

      bl->addWidget( le_last_file );
      bl->addWidget( cb_plus_lm );
      bl->addWidget( cb_errorlines );
      bl->addWidget( cb_manual_rejection );
      {
         QHBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
         hbl->addWidget( pb_apply_qtest );
         hbl->addWidget( pb_reset_qtest );
         hbl->addWidget( rb_90_qtest );
         hbl->addWidget( rb_80_qtest );
         hbl->addWidget( rb_70_qtest );
         bl->addLayout( hbl );
      }
      bl->addWidget( lbl_editor );

#if QT_VERSION < 0x040000 || !defined(Q_OS_MAC)
      bl->addWidget( frame );
#endif
      bl->addWidget( editor );
      
      top->addLayout( bl );
   }

   // ----- right side
   {
      QBoxLayout * bl = new QVBoxLayout( 0 ); bl->setContentsMargins( 0, 0, 0, 0 ); bl->setSpacing( 0 );
      bl->addWidget( plot_data );
      // needs layout for dynamic cb_'s 
      hbl_points = new QHBoxLayout();
      hbl_points->addWidget( lbl_points );
      bl->addLayout( hbl_points );
      hbl_points_ln = new QHBoxLayout();
      hbl_points_ln->addWidget( lbl_points_ln );
      bl->addLayout( hbl_points_ln );
      hbl_points_exp = new QHBoxLayout();
      hbl_points_exp->addWidget( lbl_points_exp );
      bl->addLayout( hbl_points_exp );
      top->addLayout( bl );
   }

   background->addWidget( lbl_credits_1 );
   background->addWidget( lbl_credits_2 );
   background->addLayout( top );
   background->addSpacing( 4 );

   {
      QHBoxLayout * hbl_bottom = new QHBoxLayout(); hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
      hbl_bottom->addSpacing( 4 );
      hbl_bottom->addWidget ( pb_help );
      hbl_bottom->addSpacing( 4 );
      hbl_bottom->addWidget ( pb_close );
      hbl_bottom->addSpacing( 4 );
      background->addLayout ( hbl_bottom );
   }

   if ( !((US_Hydrodyn *)us_hydrodyn)->advanced_config.expert_mode )
   {
      rb_90_qtest->hide();
      rb_80_qtest->hide();
      rb_70_qtest->hide();
   }
}

void US_Hydrodyn_Best::cancel()
{
   close();
}

void US_Hydrodyn_Best::help()
{
   US_Help *online_help;
   online_help = new US_Help( this );
   online_help->show_help("manual/somo/somo_BEST_Analysis.html");
}

void US_Hydrodyn_Best::closeEvent( QCloseEvent *e )
{
   *best_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Best::hide_input()
{
   hide_widgets( input_widgets, input_widgets[ 0 ]->isVisible() );
}

void US_Hydrodyn_Best::hide_editor()
{
   hide_widgets( editor_widgets, editor_widgets[ 0 ]->isVisible() );
   if ( editor_widgets[ 0 ]->isVisible() )
   {
      cout << "resetting editor palette\n";
      lbl_editor->setPalette( PALET_LABEL );
      AUTFBACK( lbl_editor );
   }
}

void US_Hydrodyn_Best::editor_msg( QString color, QString msg )
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

void US_Hydrodyn_Best::hide_widgets( vector < QWidget *> widgets, bool hide )
{
   for ( unsigned int i = 0; i < ( unsigned int ) widgets.size(); i++ )
   {
      hide ? widgets[ i ]->hide() : widgets[ i ]->show();
   }
}

void US_Hydrodyn_Best::clear_display()
{
   editor->clear( );
   editor->append("\n\n");
}

void US_Hydrodyn_Best::update_font()
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

void US_Hydrodyn_Best::save()
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

void US_Hydrodyn_Best::clear( )
{
   lb_data  ->clear( );
   plot_data->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_data->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   if ( plot_data_zoomer )
   {
      plot_data_zoomer->zoom ( 0 );
      delete plot_data_zoomer;
      plot_data_zoomer = (ScrollZoomer *) 0;
   }

   points             = 0;
   one_over_triangles .clear( );
   parameter_data     .clear( );
   for ( int i = 0; i < (int) cb_points.size(); ++i )
   {
      hbl_points->removeWidget( cb_points[ i ] );
      delete cb_points[ i ];
   }
   for ( int i = 0; i < (int) cb_points_ln.size(); ++i )
   {
      hbl_points_ln->removeWidget( cb_points_ln[ i ] );
      delete cb_points_ln[ i ];
   }
   for ( int i = 0; i < (int) cb_points_exp.size(); ++i )
   {
      hbl_points_exp->removeWidget( cb_points_exp[ i ] );
      delete cb_points_exp[ i ];
   }
   cb_points             .clear( );
   cb_points_ln          .clear( );
   cb_points_exp         .clear( );
   cb_checked            .clear( );
   cb_checked_ln         .clear( );
   cb_checked_exp        .clear( );
   loaded_csv_trimmed    .clear( );
   loaded_csv_filename    = "";
   last_pts_removed       = "";
   last_lin_extrapolation.clear( );
   last_log_extrapolation.clear( );
   last_exp_extrapolation.clear( );
   tau_csv_addendum_tag  .clear( );
   tau_csv_addendum_val  .clear( );
}      

void US_Hydrodyn_Best::load()
{
   // open file, clear lb_data, clear plot, reload lb_data
   save_last_file = "";
   le_last_file->setText( save_last_file );
   pb_load->setEnabled( false );
   QString use_dir = 
      USglobal->config_list.root_dir + 
      QDir::separator() + "somo" + 
      QDir::separator() + "cluster" +
      QDir::separator() + "results"
      ;

   ((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history( use_dir, this );

   QString filename = QFileDialog::getOpenFileName( this , windowTitle() + us_tr( " Load CSV from BEST results" ) , use_dir , "*.csv *.CSV" );


   if ( filename.isEmpty() )
   {
      pb_load->setEnabled( true );
      return;
   }

   ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( filename );

   clear( );

   QFile f( filename );
   if ( !f.open( QIODevice::ReadOnly ) )
   {
      editor_msg( "red", QString( us_tr( "Error opening file %1 (check permissions)" ) ).arg( f.fileName() ) );
      pb_load->setEnabled( true );
      return;
   }

   save_last_file = filename;
   le_last_file->setText( save_last_file );

   QTextStream ts( &f );
   if ( ts.atEnd() )
   {
      f.close();
      editor_msg( "red", QString( us_tr( "Error on file %1 no data" ) ).arg( f.fileName() ) );
      pb_load->setEnabled( true );
      return;
   }
      

   {
      loaded_csv_trimmed << ts.readLine();
      QStringList qsl = US_Csv::parse_line( loaded_csv_trimmed.back() ).replaceInStrings( "\"", "" );
      
      if ( qsl.size() < 8 )
      {
         f.close();
         editor_msg( "red", QString( us_tr( "Error on file %1 insufficient data or no triangles?" ) ).arg( f.fileName() ) );
         pb_load->setEnabled( true );
         return;
      }
      
      points = (int) qsl.size() - 8;

      // us_qdebug( QString( "points read %1 qsl size %2" ).arg( points ).arg( qsl.size() ) );
      // us_qdebug( QString( "qsl points + 0 %1" ).arg( qsl[ points + 0 ] ) );
      // us_qdebug( QString( "qsl points + 1 %1" ).arg( qsl[ points + 1 ] ) );
      // us_qdebug( QString( "qsl points + 2 %1" ).arg( qsl[ points + 2 ] ) );
      // us_qdebug( QString( "qsl points + 3 %1" ).arg( qsl[ points + 3 ] ) );

      if ( qsl[ points + 1 ] != "Extrapolation to zero triangles (a)" )
      {
         f.close();
         editor_msg( "red", QString( us_tr( "Error on file %1 improper format on line 1" ) ).arg( f.fileName() ) );
         pb_load->setEnabled( true );
         return;
      }
   }         


   loaded_csv_trimmed << ts.readLine(); // triangles used
   {
      loaded_csv_trimmed << ts.readLine();
      QStringList qsl = US_Csv::parse_line( loaded_csv_trimmed.back() ).replaceInStrings( "\"", "" );

      // us_qdebug( "1/triangle line:" + qsl.join( ":" ) + QString( "size %1" ).arg( qsl.size() ) );
      if ( (int) qsl.size() < 1 + points || qsl[ 0 ] != "1/Triangles used" )
      {
         f.close();
         editor_msg( "red", QString( us_tr( "Error on file %1 improper format on line 3" ) ).arg( f.fileName() ) );
         pb_load->setEnabled( true );
         return;
      }

      for ( int i = 1; i <= points; ++i )
      {
         if ( qsl[ i ] == "=-1" )
         {
            f.close();
            editor_msg( "red", QString( us_tr( "Error on file %1 line 3: 1/triangles incorrect" ) ).arg( f.fileName() ) );
            pb_load->setEnabled( true );
            return;
         }
         one_over_triangles.push_back( QString( qsl[ i ] ).replace( QRegExp( "^=" ), "" ).toDouble() );
      }
   }

   int line = 3;
   disconnect( lb_data, SIGNAL( itemSelectionChanged() ), 0, 0 );
   while ( !ts.atEnd() )
   {
      ++line;
      loaded_csv_trimmed << ts.readLine();
      QStringList qsl = US_Csv::parse_line( loaded_csv_trimmed.back() ).replaceInStrings( "\"", "" );
      // us_qdebug( "next" + qsl.join( ":" ) );
      if ( (int) qsl.size() < 1 + points )
      {
         f.close();
         editor_msg( "red", QString( us_tr( "Error on file %1 improper format on line %2" ) ).arg( f.fileName() ).arg( line ) );
         pb_load->setEnabled( true );
         connect( lb_data, SIGNAL( itemSelectionChanged() ), SLOT( data_selected() ) );
         return;
      }
      // us_qdebug( QString( "qsl 0: %1 : size %2" ).arg( qsl[ 0 ] ).arg( qsl.size() ) );
      if ( (int) qsl.size() >= points + 7 )
      {
         // have a linear fittable line
         loaded_csv_trimmed.pop_back();
         QString qs;
         qs += "\"" + qsl[ 0 ] + "\",";
         
         for ( int i = 1; i <= points; ++i )
         {
            qs += QString( "%1," ).arg( qsl[ i ] );
            parameter_data[ qsl[ 0 ] ].push_back( QString( qsl[ i ] ).replace( QRegExp( "^=" ), "" ).toDouble() );
            if ( qsl[ i ] == "?" )
            {
               f.close();
               editor_msg( "red", QString( us_tr( "Error on file %1 line %2 - missing data for column %3" ) )
                           .arg( f.fileName() )
                           .arg( line ) 
                           .arg( i + 1 ) 
                           );
               pb_load->setEnabled( true );
               connect( lb_data, SIGNAL( itemSelectionChanged() ), SLOT( data_selected() ) );
               return;
            }
         }
         loaded_csv_trimmed << qs;
         lb_data->addItem( qsl[ 0 ] );
      }
   }
   editor_msg( "blue", QString( "%1 data columns found" ).arg( points ) );
   for ( int i = 0; i < points; ++i )
   {
      QCheckBox * cb = new QCheckBox( this );
      cb->setPalette( PALET_NORMAL );
      AUTFBACK( cb );
      cb->setText( QString( "%1" ).arg( i + 1 ) );
      cb->setChecked( true );
      cb->setEnabled( cb_manual_rejection->isChecked() );
      cb->show();
      connect( cb, SIGNAL( clicked() ), SLOT( cb_changed() ) );
      cb_points.push_back( cb );
      hbl_points->addWidget( cb );
   }
   for ( int i = 0; i < points; ++i )
   {
      QCheckBox * cb = new QCheckBox( this );
      cb->setPalette( PALET_NORMAL );
      AUTFBACK( cb );
      cb->setText( QString( "%1" ).arg( i + 1 ) );
      cb->setChecked( false );
      cb->setEnabled( cb_manual_rejection->isChecked() );
      // cb->show();
      cb->hide();
      connect( cb, SIGNAL( clicked() ), SLOT( cb_changed_ln() ) );
      cb_points_ln.push_back( cb );
      hbl_points_ln->addWidget( cb );
   }
   for ( int i = 0; i < points; ++i )
   {
      QCheckBox * cb = new QCheckBox( this );
      cb->setPalette( PALET_NORMAL );
      AUTFBACK( cb );
      cb->setText( QString( "%1" ).arg( i + 1 ) );
      cb->setChecked( false );
      cb->setEnabled( cb_manual_rejection->isChecked() );
      // cb->show();
      cb->hide();
      connect( cb, SIGNAL( clicked() ), SLOT( cb_changed_exp() ) );
      cb_points_exp.push_back( cb );
      hbl_points_exp->addWidget( cb );
   }
   
   f.close();
   loaded_csv_filename = f.fileName();
   for ( int i = 0; i < (int) lb_data->count(); ++i )
   {
      lb_data->item( i)->setSelected( true );
      data_selected( false );
   }
   lb_data->item( 0)->setSelected( true );
   recompute_tau();
   connect( lb_data, SIGNAL( itemSelectionChanged() ), SLOT( data_selected() ) );
   cb_changed    ( false );
   cb_changed_ln ( false );
   cb_changed_exp( true  );
   pb_load->setEnabled( true );
   pb_save_results->setEnabled( true );
   pb_apply_qtest->setEnabled( true );
   pb_reset_qtest->setEnabled( true );
}

void US_Hydrodyn_Best::cb_changed( bool do_data )
{
   // us_qdebug( "cb_changed" );
   QString text = lb_data->selectedItems().first()->text();
   cb_checked[ text ].clear( );
   for ( int i = 0; i < (int) cb_points.size(); ++i )
   {
      if ( cb_points[ i ]->isChecked() )
      {
         cb_checked[ text ].insert( i );
      }
   }
   if ( do_data )
   {
      data_selected();
   }
}

void US_Hydrodyn_Best::cb_changed_ln( bool do_data )
{
   // us_qdebug( "cb_changed_ln" );
   QString text = lb_data->selectedItems().first()->text();
   cb_checked_ln[ text ].clear( );
   for ( int i = 0; i < (int) cb_points_ln.size(); ++i )
   {
      if ( cb_points_ln[ i ]->isChecked() )
      {
         cb_checked_ln[ text ].insert( i );
      }
   }
   if ( do_data )
   {
      data_selected();
   }
}

void US_Hydrodyn_Best::cb_changed_exp( bool do_data )
{
   // us_qdebug( "cb_changed_exp" );
   QString text = lb_data->selectedItems().first()->text();
   cb_checked_exp[ text ].clear( );
   for ( int i = 0; i < (int) cb_points_exp.size(); ++i )
   {
      if ( cb_points_exp[ i ]->isChecked() )
      {
         cb_checked_exp[ text ].insert( i );
      }
   }
   if ( do_data )
   {
      data_selected();
   }
}
   
void US_Hydrodyn_Best::data_selected( bool do_recompute_tau )
{
   // us_qdebug( "data_selected" );
   if ( !lb_data->count() )
   {
      return;
   }
   QString text = lb_data->selectedItems().first()->text();
   // us_qdebug( QString( "selected %1 map %2" ).arg( text ).arg( parameter_data[ text ].size() ) );
   plot_data->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_data->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   if ( !cb_checked.count( text ) )
   {
      // set all on
      for ( int i = 0; i < (int) cb_points.size(); ++i )
      {
         cb_checked[ text ].insert( i );
      }
   }
   for ( int i = 0; i < (int) cb_points.size(); ++i )
   {
      cb_points[ i ]->setChecked( cb_checked[ text ].count( i ) );
   }

   // if ( !cb_checked_ln.count( text ) )
   // {
   //    // set all off
   //    for ( int i = 0; i < (int) cb_points_ln.size(); ++i )
   //    {
   //       if ( parameter_data[ text ][ i ] > 0e0 )
   //       {
   //          cb_checked_ln[ text ].insert( i );
   //       }
   //    }
   // }

   for ( int i = 0; i < (int) cb_points_ln.size(); ++i )
   {
      cb_points_ln[ i ]->setChecked( cb_checked_ln[ text ].count( i ) );
      cb_points_ln[ i ]->setEnabled( parameter_data[ text ][ i ] > 0e0 );
   }

   for ( int i = 0; i < (int) cb_points_exp.size(); ++i )
   {
      cb_points_exp[ i ]->setChecked( cb_checked_exp[ text ].count( i ) );
      cb_points_exp[ i ]->setEnabled( parameter_data[ text ][ i ] > 0e0 );
   }

   vector < double > use_one_over_triangles;
   vector < double > use_parameter_data;
   vector < double > skip_one_over_triangles;
   vector < double > skip_parameter_data;

   vector < double > use_one_over_triangles_ln;
   vector < double > use_parameter_data_ln;
   vector < double > skip_one_over_triangles_ln;
   vector < double > skip_parameter_data_ln;

   vector < double > use_one_over_triangles_exp;
   vector < double > use_parameter_data_exp;
   vector < double > skip_one_over_triangles_exp;
   vector < double > skip_parameter_data_exp;

   last_pts_removed     = "";
   last_pts_removed_ln  = "";
   last_pts_removed_exp = "";

   set < int > selected_points;
   set < int > selected_points_ln;
   set < int > selected_points_exp;

   //for ( int i = 0; i < (int) one_over_triangles.size(); ++i )
   for ( int i = (int) one_over_triangles.size() - 1; i >= 0; --i )
   {
      if ( cb_points[ points - i - 1 ]->isChecked() )
      {
         use_one_over_triangles .push_back( one_over_triangles[ i ] );
         use_parameter_data     .push_back( parameter_data[ text ][ i ] );
         selected_points        .insert( i );
      } else {
         skip_one_over_triangles.push_back( one_over_triangles[ i ] );
         skip_parameter_data    .push_back( parameter_data[ text ][ i ] );
         last_pts_removed += QString( "%1 " ).arg( points - i );
      }  
      if ( cb_points_ln[ points - i - 1 ]->isChecked() &&
           parameter_data[ text ][ i ] > 0e0 )
      {
         use_one_over_triangles_ln .push_back( one_over_triangles[ i ] );
         use_parameter_data_ln     .push_back( parameter_data[ text ][ i ] );
         selected_points_ln        .insert( i );
      } else {
         skip_one_over_triangles_ln.push_back( one_over_triangles[ i ] );
         skip_parameter_data_ln    .push_back( parameter_data[ text ][ i ] );
         last_pts_removed_ln += QString( "%1 " ).arg( points - i );
      }  
      if ( cb_points_exp[ points - i - 1 ]->isChecked() &&
           parameter_data[ text ][ i ] > 0e0 )
      {
         use_one_over_triangles_exp .push_back( one_over_triangles[ i ] );
         use_parameter_data_exp     .push_back( parameter_data[ text ][ i ] );
         selected_points_exp        .insert( i );
      } else {
         skip_one_over_triangles_exp.push_back( one_over_triangles[ i ] );
         skip_parameter_data_exp    .push_back( parameter_data[ text ][ i ] );
         last_pts_removed_exp += QString( "%1 " ).arg( points - i );
      }  
   }

   for ( int i = 0; i < (int) one_over_triangles.size(); ++i )
   {
#if QT_VERSION < 0x040000
      long curve = plot_data->insertCurve( "plot" );
      plot_data->setCurveStyle( curve, QwtCurve::Dots );
#else
      QwtPlotCurve *curve = new QwtPlotCurve( "plot" );
      curve->setStyle( QwtPlotCurve::Dots );
#endif

      QwtSymbol sym;
      if ( selected_points   .count( i ) &&
           ( selected_points_ln.count( i ) ||
             selected_points_exp.count( i ) ) )
      {
         sym.setStyle(QwtSymbol::Diamond);
         sym.setSize( 12 );
         sym.setPen  ( QPen( Qt::cyan ) );
         sym.setBrush( Qt::blue );
      }
      if (  selected_points   .count( i ) &&
            !( selected_points_ln.count( i ) ||
               selected_points_exp.count( i ) ) )
      {
         sym.setStyle(QwtSymbol::UTriangle);
         sym.setSize( 10 );
         sym.setPen  ( QPen( Qt::cyan ) );
         sym.setBrush( Qt::blue );
      }
      if ( !selected_points    .count( i ) &&
           ( selected_points_ln .count( i ) ||
             selected_points_exp.count( i ) ) )
      {
         sym.setStyle(QwtSymbol::DTriangle);
         sym.setSize( 10 );
         sym.setPen  ( QPen( Qt::cyan ) );
         sym.setBrush( Qt::blue );
      }
      if ( !selected_points    .count( i ) &&
           !selected_points_ln .count( i ) &&
           !selected_points_exp.count( i ) )
      {
         sym.setStyle(QwtSymbol::XCross);
         sym.setSize( 10 );
         sym.setPen  ( QPen( Qt::red ) );
         sym.setBrush( Qt::red );
      }

#if QT_VERSION < 0x040000
      plot_data->setCurveData( curve, 
                               (double *)&( one_over_triangles[ i ] ),
                               (double *)&( parameter_data[ text ][ i ] ),
                               1
                               );
      plot_data->setCurveStyle( curve, QwtCurve::Lines);
      plot_data->setCurveSymbol( curve, sym );
#else
      curve->setSamples(
                     (double *)&( one_over_triangles[ i ] ),
                     (double *)&( parameter_data[ text ][ i ] ),
                     1
                     );

      // curve->setPen( QPen( Qt::red, 2, Qt::SolidLine ) );
      curve->attach( plot_data );
      curve->setStyle( QwtPlotCurve::Lines );
      curve->setSymbol( new QwtSymbol( sym.style(), sym.brush(), sym.pen(), sym.size() ) );
#endif
   }      
   

   double miny = 0e0;
   double maxy = 0e0;

   if ( parameter_data[ text ].size() )
   {
      miny = parameter_data[ text ][ 0 ];
      maxy = parameter_data[ text ][ 0 ];
   }

   for ( int i = 0; i < (int) parameter_data[ text ].size(); ++i )
   {
      if ( miny > parameter_data[ text ][ i ] )
      {
         miny = parameter_data[ text ][ i ];
      }
      if ( maxy < parameter_data[ text ][ i ] )
      {
         maxy = parameter_data[ text ][ i ];
      }
   }

   // run LR
   last_lin_extrapolation.erase( text );


   {
      last_a    = 0e0;
      last_siga = 0e0;
      last_b    = 0e0;
      last_sigb = 0e0;
      last_chi2 = 0e0;

      if ( use_one_over_triangles.size() > 1 )   
      {
         double a;
         double b;
         double siga;
         double sigb;
         double chi2;


         US_Saxs_Util::linear_fit( use_one_over_triangles,
                                   use_parameter_data,
                                   a,
                                   b,
                                   siga,
                                   sigb,
                                   chi2 );


         last_a    = a;
         last_siga = siga;
         last_b    = b;
         last_sigb = sigb;
         last_chi2 = chi2;

         last_lin_extrapolation[ text ].push_back( last_a );
         last_lin_extrapolation[ text ].push_back( last_siga );
         last_lin_extrapolation[ text ].push_back( last_b );
         last_lin_extrapolation[ text ].push_back( last_sigb );
         last_lin_extrapolation[ text ].push_back( last_chi2 );

         double x[ 2 ];
         double y[ 2 ];

         editor_msg( "blue", 
                     QString( us_tr( "%1: 0 triangle extrapolation=%2 sigma=%3 sigma %=%4 slope=%5 sigma=%6 sigma %=%7 chi^2=%8" ) )
                     .arg( text )
                     .arg( a,    0, 'g', 8 )
                     .arg( siga, 0, 'g', 8 )
                     .arg( a != 0 ? fabs( 100.0 * siga / a ) : (double) 0, 0, 'g', 8 )
                     .arg( b,    0, 'g', 8 )
                     .arg( sigb, 0, 'g', 8 )
                     .arg( b != 0 ? fabs( 100.0 * sigb / b ) : (double) 0, 0, 'g', 8 )
                     .arg( chi2, 0, 'g', 8 )
                     );

         x[ 0 ] = 0e0;
         x[ 1 ] = one_over_triangles[ 0 ] * 1.1;
         y[ 0 ] = a;
         y[ 1 ] = a + x[ 1 ] * b;

         {
#if QT_VERSION < 0x040000
            long curve = plot_data->insertCurve( "plot lr" );
            plot_data->setCurveStyle( curve, QwtCurve::Lines );
#else
            QwtPlotCurve *curve = new QwtPlotCurve( "plot lr" );
            curve->setStyle( QwtPlotCurve::Lines );
#endif

#if QT_VERSION < 0x040000
            plot_data->setCurveData( curve, 
                                     (double *)&( x[ 0 ] ),
                                     (double *)&( y[ 0 ] ),
                                     2
                                     );
            plot_data->setCurvePen( curve, QPen( Qt::green, 2, SolidLine));

#else
            curve->setSamples(
                           (double *)&( x[ 0 ] ),
                           (double *)&( y[ 0 ] ),
                           2
                           );

            curve->setPen( QPen( Qt::green, 2, Qt::SolidLine ) );
            curve->attach( plot_data );
#endif
            double min = y[ 0 ] < y[ 1 ] ? y[ 0 ] : y[ 1 ];
            double max = y[ 0 ] < y[ 1 ] ? y[ 1 ] : y[ 0 ];
            if ( miny > min )
            {
               miny = min;
            }
            if ( maxy < max )
            {
               maxy = max;
            }
         }
         if ( cb_errorlines->isChecked() && use_one_over_triangles.size() > 2 )
         {
            {
               double yp[ 2 ];
               yp[ 0 ] = y[ 0 ] + last_siga;
               yp[ 1 ] = y[ 1 ] + last_siga;
#if QT_VERSION < 0x040000
               long curve = plot_data->insertCurve( "plot lr p" );
               plot_data->setCurveStyle( curve, QwtCurve::Lines );
#else
               QwtPlotCurve *curve = new QwtPlotCurve( "plot lr p" );
               curve->setStyle( QwtPlotCurve::Lines );
#endif

#if QT_VERSION < 0x040000
               plot_data->setCurveData( curve, 
                                        (double *)&( x[ 0 ] ),
                                        (double *)&( yp[ 0 ] ),
                                        2
                                        );
               plot_data->setCurvePen( curve, QPen( Qt::darkGreen, 2, Qt::DashDotLine));

#else
               curve->setSamples(
                              (double *)&( x[ 0 ] ),
                              (double *)&( yp[ 0 ] ),
                              2
                              );

               curve->setPen( QPen( Qt::green, 1, Qt::DashDotLine ) );
               curve->attach( plot_data );
#endif
               // double min = yp[ 0 ] < yp[ 1 ] ? yp[ 0 ] : yp[ 1 ];
               // double max = yp[ 0 ] < yp[ 1 ] ? yp[ 1 ] : yp[ 0 ];
               // if ( miny > min )
               // {
               //    miny = min;
               // }
               // if ( maxy < max )
               // {
               //    maxy = max;
               // }
            }
            {
               double ym[ 2 ];
               ym[ 0 ] = y[ 0 ] - last_siga;
               ym[ 1 ] = y[ 1 ] - last_siga;
#if QT_VERSION < 0x040000
               long curve = plot_data->insertCurve( "plot lr m" );
               plot_data->setCurveStyle( curve, QwtCurve::Lines );
#else
               QwtPlotCurve *curve = new QwtPlotCurve( "plot lr m" );
               curve->setStyle( QwtPlotCurve::Lines );
#endif

#if QT_VERSION < 0x040000
               plot_data->setCurveData( curve, 
                                        (double *)&( x[ 0 ] ),
                                        (double *)&( ym[ 0 ] ),
                                        2
                                        );
               plot_data->setCurvePen( curve, QPen( Qt::darkGreen, 2, Qt::DashDotLine));

#else
               curve->setSamples(
                              (double *)&( x[ 0 ] ),
                              (double *)&( ym[ 0 ] ),
                              2
                              );

               curve->setPen( QPen( Qt::green, 1, Qt::DashDotLine ) );
               curve->attach( plot_data );
#endif
               // double min = ym[ 0 ] < ym[ 1 ] ? ym[ 0 ] : ym[ 1 ];
               // double max = ym[ 0 ] < ym[ 1 ] ? ym[ 1 ] : ym[ 0 ];
               // if ( miny > min )
               // {
               //    miny = min;
               // }
               // if ( maxy < max )
               // {
               //    maxy = max;
               // }
            }
         }
      }
   }

   // run LR ln

   last_log_extrapolation.erase( text );

   {
      last_a_ln    = 0e0;
      last_siga_ln = 0e0;
      last_b_ln    = 0e0;
      last_sigb_ln = 0e0;
      last_chi2_ln = 0e0;

      ln_plot_ok = false;
      if ( use_one_over_triangles_ln.size() > 1 )   
      {
         double a;
         double b;
         double siga;
         double sigb;
         double chi2;

         vector < double > fit_x;
         vector < double > fit_y;
         for ( int i = 0; i < (int) use_one_over_triangles_ln.size(); ++i )
         {
            fit_x.push_back( use_one_over_triangles_ln[ i ] * log( use_one_over_triangles_ln[ i ] ) );
            fit_y.push_back( log( use_parameter_data_ln    [ i ] ) );
         }
            
         US_Saxs_Util::linear_fit( fit_x,
                                   fit_y,
                                   a,
                                   b,
                                   siga,
                                   sigb,
                                   chi2 );

         ln_plot_ok = true;

         last_a_ln    = a;
         last_siga_ln = siga;
         last_b_ln    = b;
         last_sigb_ln = sigb;
         last_chi2_ln = chi2;

         last_log_extrapolation[ text ].push_back( last_a_ln );
         last_log_extrapolation[ text ].push_back( last_siga_ln );
         last_log_extrapolation[ text ].push_back( last_b_ln );
         last_log_extrapolation[ text ].push_back( last_sigb_ln );
         last_log_extrapolation[ text ].push_back( last_chi2_ln );

#define UHB_PTS  200
#define UHB_MINX 1e-20

         double x[ UHB_PTS ];
         double y[ UHB_PTS ];

         editor_msg( "blue", 
                     QString( us_tr( "%1: 0 triangle LN extrapolation=%2 sigma=%3 sigma %=%4 slope=%5 sigma=%6 sigma %=%7 chi^2=%8" ) )
                     .arg( text )
                     .arg( exp( a ),    0, 'g', 8 )
                     .arg( exp( a ) * siga, 0, 'g', 8 )
                     .arg( exp( a ) != 0 ? fabs( 100.0 * exp( a ) * siga / exp( a ) ) : (double) 0, 0, 'g', 8 )
                     .arg( b,    0, 'g', 8 )
                     .arg( sigb, 0, 'g', 8 )
                     .arg( b != 0 ? fabs( 100.0 * sigb / b ) : (double) 0, 0, 'g', 8 )
                     .arg( chi2, 0, 'g', 8 )
                     );

         double deltax = ( one_over_triangles[ 0 ] * 1.1 - UHB_MINX ) / ( UHB_PTS - 1 );
         for ( int i = 0; i < UHB_PTS; ++i )
         {
            x[ i ] = UHB_MINX + deltax * i;
            y[ i ] = exp( a + b * x[ i ] * log( x[ i ] ) );
         }
      
#if QT_VERSION < 0x040000
         long curve = plot_data->insertCurve( "plot lr ln" );
         plot_data->setCurveStyle( curve, QwtCurve::Lines );
#else
         QwtPlotCurve *curve = new QwtPlotCurve( "plot lr ln" );
         curve->setStyle( QwtPlotCurve::Lines );
#endif

#if QT_VERSION < 0x040000
         plot_data->setCurveData( curve, 
                                  (double *)&( x[ 0 ] ),
                                  (double *)&( y[ 0 ] ),
                                  UHB_PTS
                                  );
         plot_data->setCurvePen( curve, QPen( Qt::darkMagenta, 2, SolidLine));

#else
         curve->setSamples(
                        (double *)&( x[ 0 ] ),
                        (double *)&( y[ 0 ] ),
                        UHB_PTS
                        );

         curve->setPen( QPen( Qt::green, 2, Qt::SolidLine ) );
         curve->attach( plot_data );
#endif

         double min = y[ 0 ] < y[ UHB_PTS - 1 ] ? y[ 0 ] : y[ UHB_PTS - 1 ];
         double max = y[ 0 ] < y[ UHB_PTS - 1 ] ? y[ UHB_PTS - 1 ] : y[ 0 ];
         if ( miny > min )
         {
            miny = min;
         }
         if ( maxy < max )
         {
            maxy = max;
         }
      }
   }

   // run y = a + b exp( c x ) fit

   last_exp_extrapolation.erase( text );

   {
      last_a_exp    = 0e0;
      last_siga_exp = 0e0;
      last_b_exp    = 0e0;
      last_sigb_exp = 0e0;
      last_chi2_exp = 0e0;

      exp_plot_ok = false;
      if ( use_one_over_triangles_exp.size() > 2 )   
      {
         vector < double > fit_x;
         vector < double > fit_y;
         for ( int i = 0; i < (int) use_one_over_triangles_exp.size(); ++i )
         {
            fit_x.push_back( use_one_over_triangles_exp[ i ] );
            fit_y.push_back( use_parameter_data_exp    [ i ] );
         }

         // #define DEBUGEXP
#if defined( DEBUGEXP )
         fit_x.clear( );
         fit_y.clear( );
         fit_x.push_back( -.99 ); fit_y.push_back( .418 );
         fit_x.push_back( -.945 ); fit_y.push_back( .412 );
         fit_x.push_back( -.874 ); fit_y.push_back( .452 );
         fit_x.push_back( -.859 ); fit_y.push_back( .48 );
         fit_x.push_back( -.64 ); fit_y.push_back( .453 );
         fit_x.push_back( -.573 ); fit_y.push_back( .501 );
         fit_x.push_back( -.433 ); fit_y.push_back( .619 );
         fit_x.push_back( -.042 ); fit_y.push_back( .9 );
         fit_x.push_back( -.007 ); fit_y.push_back( .911 );
         fit_x.push_back( .054 ); fit_y.push_back( .966 );
         fit_x.push_back( .088 ); fit_y.push_back( .966 );
         fit_x.push_back( .222 ); fit_y.push_back( 1.123 );
         fit_x.push_back( .401 ); fit_y.push_back( 1.414 );
         fit_x.push_back( .465 ); fit_y.push_back( 1.683 );
         fit_x.push_back( .633 ); fit_y.push_back( 2.101 );
         fit_x.push_back( .637 ); fit_y.push_back( 1.94 );
         fit_x.push_back( .735 ); fit_y.push_back( 2.473 );
         fit_x.push_back( .762 ); fit_y.push_back( 2.276 );
         fit_x.push_back( .791 ); fit_y.push_back( 2.352 );
         fit_x.push_back( .981 ); fit_y.push_back( 3.544 );
#endif

         vector < double > S( fit_x.size() );

         // as in Jean Jaquelin Regressions et equations integrals pp 16-18

         // compute S(k)
         S[ 0 ] = 0e0;
         for ( int i = 1; i < (int) fit_x.size(); ++i )
         {
            S[ i ] = S[ i - 1 ] + .5 * ( fit_y[ i ] + fit_y[ i - 1 ] ) * ( fit_x[ i ] - fit_x[ i - 1 ] );
         }

         // US_Vector::printvector3( "exp fit input", fit_x, fit_y, S, 5 );

         double a    = 0e0;
         double b    = 0e0;
         double c    = 0e0;
         double chi2 = 0e0;

         double L11 = 0e0;
         double L12 = 0e0;
         double L22 = 0e0;
         double R1  = 0e0;
         double R2  = 0e0;
         
         for ( int i = 0; i < (int) fit_x.size(); ++i )
         {
            double t1 = fit_x[ i ] - fit_x[ 0 ];
            double t2 = fit_y[ i ] - fit_y[ 0 ];
            L11 += t1 * t1;
            L12 += t1 * S[ i ];
            L22 += S[ i ] * S[ i ];
            R1  += t2 * t1;
            R2  += t2 * S[ i ];
         }

         double det = L11 * L22 - L12 * L12;
         if ( det == 0e0 )
         {
            editor_msg( "red", us_tr( "exponential fit: zero determinant in fit" ) );
         } else {
            double oneoverdet = 1e0 / det;
            // double A1 = oneoverdet * ( L22 * R1 - L12 * R2 );
            double B1 = oneoverdet * ( - L12 * R1 + L11 * R2 );
            if ( B1 == 0e0 )
            {
               editor_msg( "red", us_tr( "exponential fit: zero c" ) );
            } else {
               // double a1 = -A1 / B1;
               double c1 = B1;
               
               double L11 = (double) fit_x.size();
               double L12 = 0e0;
               double L22 = 0e0;
               double R1  = 0e0;
               double R2  = 0e0;
         
               for ( int i = 0; i < (int) fit_x.size(); ++i )
               {
                  double t1 = exp( c1 * fit_x[ i ] );
                  L12 += t1;
                  L22 += t1 * t1;
                  R1  += fit_y[ i ];
                  R2  += fit_y[ i ] * t1;
               }

               double det = L11 * L22 - L12 * L12;

               if ( det == 0e0 )
               {
                  editor_msg( "red", us_tr( "exponential fit: zero second determinant in fit" ) );
               } else {
                  double oneoverdet = 1e0 / det;
                  double a2 = oneoverdet * ( L22 * R1 - L12 * R2 );
                  double b2 = oneoverdet * ( - L12 * R1 + L11 * R2 );

                  a = a2;
                  b = b2;
                  c = c1;
                  // us_qdebug( QString( "ok: a = %1 b = %2 c = %3" ).arg( a ).arg( b ).arg( c ) );
                  exp_plot_ok = true;

               }
            }
         }

         if ( exp_plot_ok )
         {
            chi2 = 0e0;
            for ( int i = 0; i < (int) fit_x.size(); ++i )
            {
               if ( fit_y[ i ] != 0e0 )
               {
                  double t = fit_y[ i ] - a + b * exp( c * fit_x[ i ] ) ;
                  chi2 += t * t / fit_y[ i ];
               }
            }

            if ( cb_plus_lm->isChecked() )
            {
               double org_chi2 = chi2;
               // refine with LM?
               LM::lm_control_struct control = LM::lm_control_double;
               control.printflags = 0; // 3; // monitor status (+1) and parameters (+2)
               control.epsilon    = 1e-2;
               control.stepbound  = 100;
               control.maxcall    = 1000;

               LM::lm_status_struct status;
            
               vector < double > par;
               par.push_back( a );
               par.push_back( b );
               par.push_back( c );

               LM::lmcurve_fit_rmsd( ( int )      par.size(),
                                     ( double * ) &( par[ 0 ] ),
                                     ( int )      fit_x.size(),
                                     ( double * ) &( fit_x[ 0 ] ),
                                     ( double * ) &( fit_y[ 0 ] ),
                                     BFIT::compute_f,
                                     (const LM::lm_control_struct *)&control,
                                     &status );
   
               if ( status.fnorm < 0e0 )
               {
                  // us_qdebug( "WARNING: lm() returned negative rmsd\n" );
               } else {
                  chi2 = 0e0;
                  for ( int i = 0; i < (int) fit_x.size(); ++i )
                  {
                     if ( fit_y[ i ] != 0e0 )
                     {
                        double t = fit_y[ i ] - par[ 0 ] + par[ 1 ] * exp( par[ 2 ] * fit_x[ i ] ) ;
                        chi2 += t * t / fit_y[ i ];
                     }
                  }
                  if ( chi2 < org_chi2 )
                  {
                     a = par[ 0 ];
                     b = par[ 1 ];
                     c = par[ 2 ];
                  } else {
                     editor_msg( "dark red", us_tr( "Notice: LM did not improve the fit, discarded" ) );
                  }
               }
            }

            // compute chi2 from fit(?)
            
            chi2 = 0e0;
            for ( int i = 0; i < (int) fit_x.size(); ++i )
            {
               if ( fit_y[ i ] != 0e0 )
               {
                  double t = fit_y[ i ] - a + b * exp( c * fit_x[ i ] ) ;
                  chi2 += t * t / fit_y[ i ];
               }
            }
            double siga = sqrt( chi2 );

            last_a_exp    = a;
            last_siga_exp = siga;
            last_b_exp    = b;
            last_sigb_exp = 0;
            last_c_exp    = c;
            last_sigc_exp = 0;
            last_chi2_exp = chi2;

            last_exp_extrapolation[ text ].push_back( last_a_exp + last_b_exp );
            last_exp_extrapolation[ text ].push_back( last_siga_exp );
            last_exp_extrapolation[ text ].push_back( last_b_exp );
            last_exp_extrapolation[ text ].push_back( last_sigb_exp );
            last_exp_extrapolation[ text ].push_back( last_c_exp );
            last_exp_extrapolation[ text ].push_back( last_sigc_exp );
            last_exp_extrapolation[ text ].push_back( last_chi2_exp );

            double x[ UHB_PTS ];
            double y[ UHB_PTS ];
            
            editor_msg( "blue", 
                        QString( us_tr( "%1: 0 triangle EXP extrapolation=%2 sigma=%2 b=%3 c=%4 chi^2=%5" ) )
                        .arg( text )
                        .arg( a + b,    0, 'g', 8 )
                        .arg( siga,     0, 'g', 8 )
                        .arg( b,        0, 'g', 8 )
                        .arg( c,        0, 'g', 8 )
                        .arg( chi2,     0, 'g', 8 )
                        );

            double deltax = ( one_over_triangles[ 0 ] * 1.1 ) / ( UHB_PTS - 1 );
            for ( int i = 0; i < UHB_PTS; ++i )
            {
               x[ i ] = deltax * i;
               y[ i ] = a + b * exp( c * x[ i ] );
            }
      
#if QT_VERSION < 0x040000
            long curve = plot_data->insertCurve( "plot lm exp" );
            plot_data->setCurveStyle( curve, QwtCurve::Lines );
#else
            QwtPlotCurve *curve = new QwtPlotCurve( "plot lm exp" );
            curve->setStyle( QwtPlotCurve::Lines );
#endif

#if QT_VERSION < 0x040000
            plot_data->setCurveData( curve, 
                                     (double *)&( x[ 0 ] ),
                                     (double *)&( y[ 0 ] ),
                                     UHB_PTS
                                     );
            plot_data->setCurvePen( curve, QPen( Qt::yellow, 2, SolidLine));

#else
            curve->setSamples(
                           (double *)&( x[ 0 ] ),
                           (double *)&( y[ 0 ] ),
                           UHB_PTS
                           );

            curve->setPen( QPen( Qt::green, 2, Qt::SolidLine ) );
            curve->attach( plot_data );
#endif

            double min = y[ 0 ] < y[ UHB_PTS - 1 ] ? y[ 0 ] : y[ UHB_PTS - 1 ];
            double max = y[ 0 ] < y[ UHB_PTS - 1 ] ? y[ UHB_PTS - 1 ] : y[ 0 ];
            if ( miny > min )
            {
               miny = min;
            }
            if ( maxy < max )
            {
               maxy = max;
            }
         }
      }
   }
           
   // set up axis scale

   {
      plot_data->setAxisScale( QwtPlot::xBottom, 0, one_over_triangles[ 0 ] * 1.1e0  );

                            
      miny < 0 ?
             plot_data->setAxisScale( QwtPlot::yLeft  , miny * 1.03e0 , maxy * .97e0 ) :
         plot_data->setAxisScale( QwtPlot::yLeft  , miny * 0.97e0 , maxy * 1.03e0 ) ;
   }

   if ( plot_data_zoomer )
   {
      delete plot_data_zoomer;
      plot_data_zoomer = (ScrollZoomer *)0;
   }

   plot_data_zoomer = new ScrollZoomer(plot_data->canvas());
   plot_data_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
#if QT_VERSION < 0x040000
   plot_data_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
   if ( do_recompute_tau &&
        tau_input_set.count( text ) )
   {
      // us_qdebug( "data selected & do recompute_tau" );
      recompute_tau();
   }


   plot_data->replot();
}

void US_Hydrodyn_Best::save_results()
{
   QString use_dir = 
      USglobal->config_list.root_dir + 
      QDir::separator() + "somo" + 
      QDir::separator() + "cluster" +
      QDir::separator() + "results"
      ;

   ((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history( use_dir, this );
   use_dir += QDir::separator() + QFileInfo( loaded_csv_filename ).baseName() + "_results.csv";
   // us_qdebug( use_dir );

   QString filename = QFileDialog::getSaveFileName( this , us_tr( "Select a name to save the state" ) , use_dir , "*.csv *.CSV" );


   ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( filename );

   if ( filename.isEmpty() )
   {
      return;
   }

   if ( QFile::exists( filename ) )
   {
      filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( filename, 0, this );
      raise();
   }

   QFile f( filename );
   if ( !f.open( QIODevice::WriteOnly ) )
   {
      editor_msg( "red", QString( us_tr( "Could not open file %1 for writing" ) ).arg( f.fileName() ) );
      return;
   }

   pb_save_results->setEnabled( false );
   pb_join_results->setEnabled( false );
   pb_load        ->setEnabled( false );
   lb_data        ->setEnabled( false );
   disconnect( lb_data, SIGNAL( itemSelectionChanged() ), 0, 0 );

   int cur_selected = lb_data->row( lb_data->selectedItems().first() );

   map < QString, QString > additions;

   bool any_ln_plot  = false;
   bool any_exp_plot = false;
   for ( int i = 0; i < (int) lb_data->count(); ++i )
   {
      lb_data->item( i)->setSelected( true );
      data_selected( false );
      additions[ lb_data->item( i )->text() ] = 
         QString( "=%1,%2%3,%4%5" )
         .arg( last_a,    0, 'g', 8 )
         .arg( us_isnan( last_siga ) ? "=" : "" )
         .arg( last_siga, 0, 'g', 8 )
         .arg( us_isnan( last_siga ) ? "=" : "" )
         .arg( last_a != 0 ? fabs( 100.0 * last_siga / last_a ) : (double) 0, 0, 'g', 8 )
         +
         QString( ",=%1,%2%3,%4%5" )
         .arg( last_b,    0, 'g', 8 )
         .arg( us_isnan( last_sigb ) ? "=" : "" )
         .arg( last_sigb, 0, 'g', 8 )
         .arg( us_isnan( last_sigb ) ? "=" : "" )
         .arg( last_b != 0 ? fabs( 100.0 * last_sigb / last_b ) : (double) 0, 0, 'g', 8 )
         +
         QString( ",=%1,%2" )
         .arg( last_chi2, 0, 'g', 8 )
         .arg( last_pts_removed )
         ;

      if ( ln_plot_ok )
      {
         any_ln_plot = true;
         additions[ lb_data->item( i )->text() ] += 
            QString( ",=%1,%2%3,%4%5" )
            .arg( exp( last_a_ln ),    0, 'g', 8 )
            .arg( ( us_isnan( last_siga_ln ) || us_isnan( last_a_ln ) || us_isnan( exp( last_a_ln ) ) )  ? "=" : "" )
            .arg( exp( last_a_ln ) * last_siga_ln, 0, 'g', 8 )
            .arg( ( us_isnan( last_siga_ln ) || us_isnan( last_a_ln ) )? "=" : "" )
            .arg( exp( last_a_ln ) != 0 ? fabs( 100.0 * exp( last_a_ln ) * last_siga_ln / exp( last_a_ln ) ) : (double) 0, 0, 'g', 8 )
            +
            QString( ",=%1,%2%3,%4%5" )
            .arg( last_b_ln,    0, 'g', 8 )
            .arg( us_isnan( last_sigb_ln ) ? "=" : "" )
            .arg( last_sigb_ln, 0, 'g', 8 )
            .arg( us_isnan( last_sigb_ln ) ? "=" : "" )
            .arg( last_b_ln != 0 ? fabs( 100.0 * last_sigb_ln / last_b_ln ) : (double) 0, 0, 'g', 8 )
            +
            QString( ",=%1,%2" )
            .arg( last_chi2_ln, 0, 'g', 8 )
            .arg( last_pts_removed_ln )
            ;
      }

      if ( exp_plot_ok )
      {
         any_exp_plot = true;
         additions[ lb_data->item( i )->text() ] += 
            QString( ",=%1,%2%3,%4%5" )
            .arg( last_a_exp,    0, 'g', 8 )
            .arg( us_isnan( last_siga_exp ) ? "=" : "" )
            .arg( last_siga_exp, 0, 'g', 8 )
            .arg( us_isnan( last_siga_exp ) ? "=" : "" )
            .arg( last_a_exp != 0 ? fabs( 100.0 * last_siga_exp / last_a_exp ) : (double) 0, 0, 'g', 8 )
            +
            QString( ",=%1,%2%3,%4%5" )
            .arg( last_b_exp,    0, 'g', 8 )
            .arg( us_isnan( last_sigb_exp ) ? "=" : "" )
            .arg( last_sigb_exp, 0, 'g', 8 )
            .arg( us_isnan( last_sigb_exp ) ? "=" : "" )
            .arg( last_b_exp != 0 ? fabs( 100.0 * last_sigb_exp / last_b_exp ) : (double) 0, 0, 'g', 8 )
            +
            QString( ",=%1,%2" )
            .arg( last_chi2_exp, 0, 'g', 8 )
            .arg( last_pts_removed_exp )
            ;
      }
   }      
   recompute_tau();

   QStringList out;
   for ( int i = 0; i < (int)loaded_csv_trimmed.size(); ++i )
   {
      QStringList qsl = US_Csv::parse_line( loaded_csv_trimmed[ i ] ).replaceInStrings( "\"", "" );
      out << loaded_csv_trimmed[ i ];
      if ( qsl.size() && additions.count( qsl[ 0 ] ) )
      {
         out.back() += additions[ qsl[ 0 ] ];
      }
   }
   for ( int i = 0; i < (int) tau_csv_addendum_tag.size(); ++i )
   {
      out << QString( "\"%1\"," ).arg( tau_csv_addendum_tag[ i ] );
      for ( int j = 0; j < points; ++j )
      {
         out.back() += ",";
      }
      out.back() += tau_csv_addendum_val[ i ];
   }

   out[ 0 ] = "\"" + le_last_file->text() + "\"" + out[ 0 ];
   out[ 0 ] += ",\"Points removed (largest number of triangles is point 1)\"";
   if ( any_ln_plot )
   {
      out[ 0 ] +=
         ",\"LN: Extrapolation to zero triangles (a)\",\"Sigma a\",\"Sigma a %\",\"Slope (b)\",\"Sigma b\",\"Sigma b %\",\"chi^2\""
         ;
      out[ 0 ] += ",\"Points removed (largest number of triangles is point 1)\"";
   }
   if ( any_exp_plot )
   {
      out[ 0 ] +=
         ",\"EXP: Extrapolation to zero triangles (a)\",\"Sigma a\",\"Sigma a %\",\"Slope (b)\",\"Sigma b\",\"Sigma b %\",\"chi^2\""
         ;
      out[ 0 ] += ",\"Points removed (largest number of triangles is point 1)\"";
   }
   // us_qdebug( loaded_csv_filename );
   // us_qdebug( loaded_csv_trimmed.join( "\n" ) );
   // us_qdebug( out.join( "\n" ) );
   QTextStream ts( &f );
   ts << out.join( "\n" ) << Qt::endl;
   f.close();

   connect( lb_data, SIGNAL( itemSelectionChanged() ), SLOT( data_selected() ) );
   lb_data->item( cur_selected)->setSelected( true );
   pb_save_results->setEnabled( true );
   pb_join_results->setEnabled( true );
   pb_load        ->setEnabled( true );
   lb_data        ->setEnabled( true );
}

void US_Hydrodyn_Best::recompute_tau()
{

   bool lin_ok = true;
   bool log_ok = true;
   bool exp_ok = true;

   tau_csv_addendum_tag.clear( );
   tau_csv_addendum_val.clear( );

   for ( int i = 0; i < (int) tau_inputs.size(); ++i )
   {
      if ( !last_lin_extrapolation.count( tau_inputs[ i ] ) )
      {
         lin_ok = false;
      }
      if ( !last_log_extrapolation.count( tau_inputs[ i ] ) )
      {
         log_ok = false;
      }
      if ( !last_exp_extrapolation.count( tau_inputs[ i ] ) )
      {
         exp_ok = false;
      }
   }

   if ( lin_ok )
   {
      vector < double > this_tau_results;
      QString msg;
      
      US_Saxs_Util::compute_tau( 
                                last_lin_extrapolation[ tau_inputs[ 0 ] ][ 0 ] * 1e-3,
                                last_lin_extrapolation[ tau_inputs[ 1 ] ][ 0 ] * 1e-3,
                                last_lin_extrapolation[ tau_inputs[ 2 ] ][ 0 ] * 1e-3,
                                .1,
                                this_tau_results );

      for ( int i = 0; i < (int) this_tau_results.size(); ++i )
      {
         tau_csv_addendum_tag << QString( us_tr( "Linear extrapolation of Drr EV (1/s) %1" ) ).arg( tau_msg[ i ] );
         tau_csv_addendum_val << QString( "%1" ).arg( this_tau_results[ i ], 0, 'g', 8 );
         msg += tau_csv_addendum_tag.back() + " " + tau_csv_addendum_val.back() + "\n";
      }
      editor_msg( "dark blue", msg );
   }

   if ( log_ok )
   {
      vector < double > this_tau_results;
      
      US_Saxs_Util::compute_tau( 
                                exp( last_log_extrapolation[ tau_inputs[ 0 ] ][ 0 ] ) * 1e-3,
                                exp( last_log_extrapolation[ tau_inputs[ 1 ] ][ 0 ] ) * 1e-3,
                                exp( last_log_extrapolation[ tau_inputs[ 2 ] ][ 0 ] ) * 1e-3,
                                .1,
                                this_tau_results );

      QString msg;
      for ( int i = 0; i < (int) this_tau_results.size(); ++i )
      {
         tau_csv_addendum_tag << QString( us_tr( "LOG extrapolation of Drr EV (1/s) %1" ) ).arg( tau_msg[ i ] );
         tau_csv_addendum_val << QString( "%1" ).arg( this_tau_results[ i ], 0, 'g', 8 );
         msg += tau_csv_addendum_tag.back() + " " + tau_csv_addendum_val.back() + "\n";
      }
      editor_msg( "dark blue", msg );
   }

   if ( exp_ok )
   {
      vector < double > this_tau_results;
      
      US_Saxs_Util::compute_tau( 
                                last_exp_extrapolation[ tau_inputs[ 0 ] ][ 0 ] * 1e-3,
                                last_exp_extrapolation[ tau_inputs[ 1 ] ][ 0 ] * 1e-3,
                                last_exp_extrapolation[ tau_inputs[ 2 ] ][ 0 ] * 1e-3,
                                .1,
                                this_tau_results );

      QString msg;
      for ( int i = 0; i < (int) this_tau_results.size(); ++i )
      {
         tau_csv_addendum_tag << QString( us_tr( "EXP extrapolation of Drr EV (1/s) %1" ) ).arg( tau_msg[ i ] );
         tau_csv_addendum_val << QString( "%1" ).arg( this_tau_results[ i ], 0, 'g', 8 );
         msg += tau_csv_addendum_tag.back() + " " + tau_csv_addendum_val.back() + "\n";
      }
      editor_msg( "dark blue", msg );
   }
}

void US_Hydrodyn_Best::toggle_points()
{
   bool any_checked = false;
   for ( int i = 0; i < (int) cb_points.size(); ++i )
   {
      if ( cb_points[ i ]->isChecked() )
      {
         any_checked = true;
         break;
      }
   }

   if ( !cb_manual_rejection->isChecked() )
   {
      any_checked = false;
   }

   for ( int i = 0; i < (int) cb_points.size(); ++i )
   {
      disconnect( cb_points[ i ], SIGNAL( clicked() ), 0, 0 );
      cb_points[ i ]->setChecked( !any_checked );
      connect( cb_points[ i ], SIGNAL( clicked() ), SLOT( cb_changed() ) );
   }
   if ( cb_points.size() )
   {
      cb_changed();
   }
   // data_selected();
}

void US_Hydrodyn_Best::toggle_points_ln()
{
   bool any_checked = false;
   for ( int i = 0; i < (int) cb_points_ln.size(); ++i )
   {
      if ( cb_points_ln[ i ]->isChecked() )
      {
         any_checked = true;
         break;
      }
   }

   if ( !cb_manual_rejection->isChecked() )
   {
      any_checked = false;
   }

   for ( int i = 0; i < (int) cb_points_ln.size(); ++i )
   {
      if ( cb_points_ln[ i ]->isEnabled() )
      {
         disconnect( cb_points_ln[ i ], SIGNAL( clicked() ), 0, 0 );
         cb_points_ln[ i ]->setChecked( !any_checked );
         connect( cb_points_ln[ i ], SIGNAL( clicked() ), SLOT( cb_changed_ln() ) );
      }
   }
   cb_changed_ln();
   // data_selected();
}

void US_Hydrodyn_Best::toggle_points_exp()
{
   bool any_checked = false;
   for ( int i = 0; i < (int) cb_points_exp.size(); ++i )
   {
      if ( cb_points_exp[ i ]->isChecked() )
      {
         any_checked = true;
         break;
      }
   }

   if ( !cb_manual_rejection->isChecked() )
   {
      any_checked = false;
   }

   for ( int i = 0; i < (int) cb_points_exp.size(); ++i )
   {
      if ( cb_points_exp[ i ]->isEnabled() )
      {
         disconnect( cb_points_exp[ i ], SIGNAL( clicked() ), 0, 0 );
         cb_points_exp[ i ]->setChecked( !any_checked );
         connect( cb_points_exp[ i ], SIGNAL( clicked() ), SLOT( cb_changed_exp() ) );
      }
   }
   cb_changed_exp();
   // data_selected();
}

void US_Hydrodyn_Best::set_last_file( const QString & str )
{
   if ( str != save_last_file )
   {
      le_last_file->setText( save_last_file );
   }
}

void US_Hydrodyn_Best::join_results()
{
   QStringList join_files;
   QStringList files;
   map < QString, bool > already_listed;

   QString use_dir = 
      USglobal->config_list.root_dir + 
      QDir::separator() + "somo" + 
      QDir::separator() + "cluster" +
      QDir::separator() + "results"
      ;

   ((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history( use_dir, this );
   do 
   {
      files = QFileDialog::getOpenFileNames( this , us_tr( "Select CSV results to join" ) , use_dir , "CSV files (*.csv *.CSV)" );


      for ( unsigned int i = 0; i < (unsigned int)files.size(); i++ )
      {
         if ( !already_listed.count( files[ i ] ) )
         {
            join_files << files[ i ];
            already_listed[ files[ i ] ] = true;
            ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( files[ i ] );
         }
      }
   } while ( files.size() );
   if ( !join_files.size() )
   {
      return;
   }

   if ( join_files.size() == 1 )
   {
      editor_msg( "red", us_tr( "Error: Only one file selected to join." ) );
      return;
   }
   
   QString save_file;
   {
      QString use_dir = 
         USglobal->config_list.root_dir + 
         QDir::separator() + "somo" + 
         QDir::separator() + "cluster" +
         QDir::separator() + "results"
         ;

      ((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history( use_dir, this );

      save_file = QFileDialog::getSaveFileName( this , us_tr( "Choose a name to save the joined CSV results" ) , use_dir , "CSV files (*.csv *.CSV)" );



      if ( save_file.isEmpty() )
      {
         return;
      }
   }


   save_file.replace(  QRegExp( "(-joined|)\\.(csv|CSV)$" ), "" );
   save_file += "-joined.csv";

   if ( QFile::exists( save_file ) )
   {
      save_file = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( save_file, 0, this );
   }

   // parse through all results files, make output file grouping by result

   editor_msg( "dark blue", 
               QString( "Joining %1 as %2" )
               .arg( join_files.join( "\n" ) )
               .arg( save_file ) );

   QFile f_out( save_file );
   if ( !f_out.open( QIODevice::WriteOnly ) )
   {
      editor_msg( "red", QString( us_tr( "Error: Can not open file %1 for writing" ) ).arg( save_file ) );
      return;
   }

   ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( save_file );


   QTextStream ts_out( &f_out );

   map < QString, QStringList > output;

   for ( int i = 0; i < ( int)join_files.size(); ++i )
   {
      editor_msg( "dark gray", QString( us_tr( "Processing %1" ) ).arg( join_files[ i ] ) );
      qApp->processEvents();


      QFile f_in( join_files[ i ] );

      if ( !f_in.open( QIODevice::ReadOnly ) )
      {
         editor_msg( "red", QString( us_tr( "Error: Can not open file %1 for reading" ) ).arg( join_files[ i ] ) );
         f_out.close();
         f_out.remove();
         return;
      }

      QTextStream ts_in( &f_in );

      QStringList qsl = US_Csv::parse_line( ts_in.readLine() ).replaceInStrings( "\"", "" );
      if ( !qsl.size() )
      {
         editor_msg( "red", QString( us_tr( "Error: file %1 error on line 1 (empty)" ) ).arg( join_files[ i ] ) );
         f_in.close();
         f_out.close();
         f_out.remove();
         return;
      }
         
      QString     name = qsl[ 0 ];
      if ( name.isEmpty() )
      {
         name = join_files[ i ];
      }

      int tmp_points;
      for ( tmp_points = 1; tmp_points < (int) qsl.size() && qsl[ tmp_points ] != "Extrapolation to zero triangles (a)"; ++tmp_points ){};
      tmp_points--;
         
      // us_qdebug( QString( "tmp_points %1 qsl size %2" ).arg( tmp_points ).arg( qsl.size() ) );
      if ( (int) qsl.size() <= tmp_points + 1 )
      {
         editor_msg( "red", QString( us_tr( "Error: file %1 error on line 1 (points)" ) ).arg( join_files[ i ] ) );
         f_in.close();
         f_out.close();
         f_out.remove();
         return;
      }
         
      if ( qsl[ tmp_points + 1 ] != "Extrapolation to zero triangles (a)" )
      {
         editor_msg( "red", QString( us_tr( "Error: file %1 error on line 1 (tag)" ) ).arg( join_files[ i ] ) );
         f_in.close();
         f_out.close();
         f_out.remove();
         return;
      }

      while ( !ts_in.atEnd() )
      {
         QStringList qsl = US_Csv::parse_line( ts_in.readLine() ); //.replaceInStrings( "\"", "" );
         if ( (int) qsl.size() >= tmp_points + 2 )
         {
            QStringList qsl_out;
            qsl_out << "\"" + name + "\",\"" + QFileInfo( name ).baseName() + "\"";
            for ( int j = tmp_points + 1; j < (int) qsl.size(); ++j )
            {
               qsl_out << qsl[ j ];
            }
            output[ qsl[ 0 ] ] << qsl_out.join( "," );
         }
      }
      f_in.close();
   }

   ts_out << ",,\"Extrapolation to zero triangles (a)\",\"Sigma a\",\"Sigma a %\",\"Slope (b)\",\"Sigma b\",\"Sigma b %\",\"chi^2\",\"Points removed (largest number of triangles is point 1)\"" << Qt::endl;

   for (  map < QString, QStringList >::iterator it = output.begin();
          it != output.end();
          ++it )
   {
      ts_out << "\n";
      ts_out << ",,\"" + it->first + "\"" << Qt::endl;
      ts_out << it->second.join( "\n" ) << Qt::endl;
   }

   f_out.close();
   editor_msg( "blue", 
               QString( us_tr( "Join results created %1" ) ).arg( f_out.fileName() ) );
}

class uhb_sortable_double {
public:
   double       x;
   int          index;
   bool operator < (const uhb_sortable_double& objIn) const
   {
      return x < objIn.x;
   }
};

void US_Hydrodyn_Best::reset_qtest()
{
   cb_manual_rejection->setChecked( false );
   toggle_points();
}

void US_Hydrodyn_Best::apply_qtest()
{
   // only applies to linear
   
   reset_qtest();

   if ( !lb_data->count() )
   {
      return;
   }
   QString text = lb_data->selectedItems().first()->text();

   vector < double > use_one_over_triangles;
   vector < double > use_parameter_data;
   vector < double > skip_one_over_triangles;
   vector < double > skip_parameter_data;
   vector < int >    point_ref;
   set < int > selected_points;

   for ( int i = (int) one_over_triangles.size() - 1; i >= 0; --i )
   {
      if ( cb_points[ points - i - 1 ]->isChecked() )
      {
         use_one_over_triangles .push_back( one_over_triangles[ i ] );
         use_parameter_data     .push_back( parameter_data[ text ][ i ] );
         selected_points        .insert( i );
         point_ref              .push_back( points - i - 1 );
      } else {
         skip_one_over_triangles.push_back( one_over_triangles[ i ] );
         skip_parameter_data    .push_back( parameter_data[ text ][ i ] );
      }
   }

   if ( use_one_over_triangles.size() < 3 )
   {
      editor_msg( "red", us_tr( "To few points available for Q test" ) );
      return;
   }

   vector < double > Qc = rb_90_qtest->isChecked() ? Qc95 : ( rb_80_qtest->isChecked() ? Qc80 : Qc70 );

   bool use_Qc = use_one_over_triangles.size() <= Qc.size();

   list < uhb_sortable_double > deltas;

   uhb_sortable_double delta;
   for ( int i = 0; i < (int) use_one_over_triangles.size(); ++i )
   {
      delta.x     = use_parameter_data[ i ] - ( last_a + use_one_over_triangles[ i ] * last_b );
      delta.index = point_ref[ i ];
      // us_qdebug( QString( "compute use_parameter_data[ i ] %1\n"
      //                  "        use_one_over_triangles[ i ] %2" )
      //         .arg( use_parameter_data[ i ] )
      //         .arg( use_one_over_triangles[ i ] )
      //         );
      // us_qdebug( QString( "compute delta %1 index %2" ).arg( delta.x ).arg( delta.index ) );
      deltas.push_back( delta );
   }

   deltas.sort();
   if ( fabs( deltas.back().x ) < fabs( deltas.front().x ) )
   {
      deltas.reverse();
   }

   vector < uhb_sortable_double > vdeltas;

   for ( list < uhb_sortable_double >::iterator it = deltas.begin();
         it != deltas.end();
         ++it )
   {
      vdeltas.push_back( *it );
   }

   if ( vdeltas.back().x == vdeltas[ 0 ].x )
   {
      editor_msg( "blue", us_tr( "Q test: all deltas identical, no points can be rejected" ) );
      return;
   }

   int vdelta_end = (int) vdeltas.size() - 1;

   double Q = 
      ( vdeltas[ vdelta_end ].x - vdeltas[ vdelta_end - 1 ].x ) /
      ( vdeltas[ vdelta_end ].x - vdeltas[ 0 ].x );

   QString msg = QString( us_tr( "Q test: Q = %1 " ) ).arg( Q );
      
   double qc_mult =  rb_90_qtest->isChecked() ? 1e0 : ( rb_80_qtest->isChecked() ? .8e0 : .7e0 );

   if ( use_Qc )
   {
      msg += QString( "Qc = %1 " ).arg( Qc[ vdeltas.size() ] );
   }

   if ( ( use_Qc  &&  Qc[ vdeltas.size() ] < Q ) ||
        ( !use_Qc &&  fabs( vdeltas.back().x ) >= 2.6 * qc_mult ) )
   {
      msg += QString( us_tr( "Removing point %1" ) ).arg( vdeltas.back().index + 1 );
      if ( rb_80_qtest->isChecked() )
      {
         editor_msg( "dark red", us_tr( "WARNING: 80% Q test criterion parameters used" ) );
      }
      if ( rb_70_qtest->isChecked() )
      {
         editor_msg( "dark red", us_tr( "WARNING: 70% Q test criterion parameters used" ) );
      }
      editor_msg( "blue", msg);
      cb_points[ vdeltas.back().index ]->setChecked( false );
      cb_changed();
      rb_90_qtest->setChecked( true );
      return;
   }

   msg += us_tr( "No points removed." );

   editor_msg( "blue", msg );
   rb_90_qtest->setChecked( true );
}

void US_Hydrodyn_Best::set_loose_qtest()
{
   if ( !rb_90_qtest->isChecked() )
   {
      switch ( QMessageBox::warning( this
                                     , windowTitle() + us_tr( " : Warning" )
                                     , ( rb_80_qtest->isChecked() ? "80" : "70" ) +
                                       us_tr( "% Q test is not recommended.\n"
                                           "Do you still want to enable?" )
                                     , QMessageBox::Yes
                                     , QMessageBox::No
                                     ) )
      {
      case QMessageBox::Yes :
         break;
      case QMessageBox::No :
         rb_90_qtest->setChecked( true );
         break;
      default :
         break;
      }
   }
}

void US_Hydrodyn_Best::set_manual_rejection()
{
   if ( cb_manual_rejection->isChecked() )
   {
      switch ( QMessageBox::warning( this
                                     , windowTitle() + us_tr( " : Warning" )
                                     , us_tr( "Manual outlier rejection is not recommended.\n"
                                           "'Apply Q test criterion' is preferred.\n"
                                           "Do you still want to enable manual rejection?" )
                                     , QMessageBox::Yes
                                     , QMessageBox::No
                                     ) )
      {
      case QMessageBox::Yes :
         break;
      case QMessageBox::No :
         cb_manual_rejection->setChecked( false );
         break;
      default :
         break;
      }
   }
   for ( int i = 0; i < (int) cb_points.size(); ++i )
   {
      cb_points[ i ]->setEnabled( cb_manual_rejection->isChecked() );
   }
   // for ( int i = 0; i < (int) cb_points_ln.size(); ++i )
   // {
   //    cb_points_ln[ i ]->setEnabled( cb_manual_rejection->isChecked() );
   // }
   // for ( int i = 0; i < (int) cb_points_exp.size(); ++i )
   // {
   //    cb_points_exp[ i ]->setEnabled( cb_manual_rejection->isChecked() );
   // }

   if ( !cb_manual_rejection->isChecked() )
   {
      toggle_points();
      // toggle_points_ln();
      // toggle_points_exp();
   }
}

void US_Hydrodyn_Best::usp_config_plot_data( const QPoint & ) {
   US_PlotChoices *uspc = new US_PlotChoices( usp_plot_data );
   uspc->exec();
   delete uspc;
}
