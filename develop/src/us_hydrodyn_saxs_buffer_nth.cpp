#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_saxs_buffer_nth.h"
#include <set>
//Added by qt3to4:
#include <Q3TextStream>
#include <Q3HBoxLayout>
#include <QLabel>
#include <Q3GridLayout>
#include <Q3VBoxLayout>
#include <Q3BoxLayout>
#include <QCloseEvent>

US_Hydrodyn_Saxs_Buffer_Nth::US_Hydrodyn_Saxs_Buffer_Nth(
                                                     void                     *              us_hydrodyn_saxs_buffer,
                                                     map < QString, QString > *              parameters,
                                                     QWidget *                               p,
                                                     const char *                            name
                                                     ) : QDialog( p, name )
{
   this->us_hydrodyn_saxs_buffer              = us_hydrodyn_saxs_buffer;
   this->us_hydrodyn                          = ((US_Hydrodyn_Saxs_Buffer *)us_hydrodyn_saxs_buffer)->us_hydrodyn;
   this->parameters                           = parameters;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setCaption( tr( "US-SOMO: SAXS Data Utility : Select curves" ) );

   plot_data_zoomer      = (ScrollZoomer *) 0;

#ifdef QT4
   plot_marker           = (QwtPlotMarker *) 0;
#else
   plot_marker           = (long *) 0;
#endif

   setupGUI();
   pc                    = new PC( plot_data->canvasBackground() );

   update_enables();
   update_files_selected();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );
}

US_Hydrodyn_Saxs_Buffer_Nth::~US_Hydrodyn_Saxs_Buffer_Nth()
{
}

void US_Hydrodyn_Saxs_Buffer_Nth::setupGUI()
{
   int minHeight1  = 24;

   lbl_title =  new mQLabel     ( caption(), this );
   lbl_title -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_title -> setMinimumHeight( minHeight1 );
   lbl_title -> setPalette( PALET_LABEL );
   AUTFBACK( lbl_title );
   lbl_title -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold ) );
   
   connect( lbl_title, SIGNAL( pressed() ), SLOT( hide_files() ) );

   lbl_files =  new QLabel      ( tr( "Complete list of data files" ), this );
   lbl_files -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_files -> setPalette      ( PALET_NORMAL );
   AUTFBACK( lbl_files );
   lbl_files -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Normal) );
   files_widgets.push_back( lbl_files );

   lb_files =  new Q3ListBox(this, "files files listbox" );
   lb_files -> setPalette( PALET_NORMAL );
   AUTFBACK( lb_files );
   lb_files -> setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lb_files -> setEnabled(true);
   lb_files -> setSelectionMode( Q3ListBox::Extended );
   lb_files -> setMinimumHeight( minHeight1 * 8 );

   for ( int i = 0; i < ((US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->lb_files->numRows(); ++i )
   {
      lb_files->insertItem( QString( "%1 : %2" ).arg( i + 1 ).arg( ((US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->lb_files->text( i ) ) );
      if ( ((US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->lb_files->isSelected( i ) )
      {
         lb_files->setSelected( i , true );
      }
   }
   connect( lb_files, SIGNAL( selectionChanged() ), SLOT( update_files_selected() ) );
   files_widgets.push_back( lb_files );

   lbl_files_sel =  new QLabel      ( tr( "Selected data files" ), this );
   lbl_files_sel -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_files_sel -> setPalette      ( PALET_NORMAL );
   AUTFBACK( lbl_files_sel );
   lbl_files_sel -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Normal) );
   files_widgets.push_back( lbl_files_sel );

   lb_files_sel =  new Q3ListBox(this, "files_sel files_sel listbox" );
   lb_files_sel -> setPalette( PALET_NORMAL );
   AUTFBACK( lb_files_sel );
   lb_files_sel -> setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lb_files_sel -> setEnabled(true);
   lb_files_sel -> setSelectionMode( Q3ListBox::NoSelection );
   lb_files_sel -> setMinimumHeight( minHeight1 * 8 );
   files_widgets.push_back( lb_files_sel );

   lbl_files_selected =  new QLabel      ( "", this );
   lbl_files_selected -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_files_selected -> setPalette      ( PALET_NORMAL );
   AUTFBACK( lbl_files_selected );
   lbl_files_selected -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Normal) );
   files_widgets.push_back( lbl_files_selected );

   // select

   lbl_select_nth = new mQLabel ( tr( "Select every Nth over range" ), this);
   lbl_select_nth->setAlignment( Qt::AlignCenter | Qt::AlignVCenter);
   lbl_select_nth->setPalette( PALET_LABEL );
   AUTFBACK( lbl_select_nth );
   lbl_select_nth->setFont     ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   connect( lbl_select_nth, SIGNAL( pressed() ), SLOT( hide_select() ) );

   lbl_n =  new QLabel      ( tr( "Select every Nth:" ), this );
   lbl_n -> setAlignment    ( Qt::AlignRight | Qt::AlignVCenter );
   lbl_n -> setPalette( PALET_LABEL );
   AUTFBACK( lbl_n );
   lbl_n -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   select_widgets.push_back( lbl_n );

   le_n = new QLineEdit(this, "le_n Line Edit");
   le_n->setText( "1" );
   le_n->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_n->setPalette( PALET_NORMAL );
   AUTFBACK( le_n );
   le_n->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QIntValidator *qdv = new QIntValidator( 1, ((US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->lb_files->numRows(), le_n );
      le_n->setValidator( qdv );
   }
   connect( le_n, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_n->setMinimumWidth( 80 );
   select_widgets.push_back( le_n );

   lbl_start =  new QLabel      ( tr( "Starting curve offset:" ), this );
   lbl_start -> setAlignment    ( Qt::AlignRight | Qt::AlignVCenter );
   lbl_start -> setPalette( PALET_LABEL );
   AUTFBACK( lbl_start );
   lbl_start -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );
   select_widgets.push_back( lbl_start );

   le_start = new QLineEdit(this, "le_start Line Edit");
   le_start->setText( "1" );
   le_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_start->setPalette( PALET_NORMAL );
   AUTFBACK( le_start );
   le_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QIntValidator *qdv = new QIntValidator( 1, ((US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->lb_files->numRows(), le_start );
      le_start->setValidator( qdv );
   }
   connect( le_start, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_start->setMinimumWidth( 80 );
   select_widgets.push_back( le_start );
   
   lbl_start_name =  new QLabel      ( ( (US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->lb_files->text( le_start->text().toInt() - 1 ), this );
   lbl_start_name -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_start_name -> setPalette      ( PALET_NORMAL );
   AUTFBACK( lbl_start_name );
   lbl_start_name -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Normal) );

   select_widgets.push_back( lbl_start_name );

   lbl_end =  new QLabel      ( tr( "Ending curve offset:" ), this );
   lbl_end -> setAlignment    ( Qt::AlignRight | Qt::AlignVCenter );
   lbl_end -> setPalette      ( PALET_LABEL );
   AUTFBACK( lbl_end );
   lbl_end -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   select_widgets.push_back( lbl_end );

   le_end = new QLineEdit(this, "le_end Line Edit");
   le_end->setText( QString( "%1" ).arg( ((US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->lb_files->numRows() ) );
   le_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_end->setPalette( PALET_NORMAL );
   AUTFBACK( le_end );
   le_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QIntValidator *qdv = new QIntValidator( 2, ((US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->lb_files->numRows(), le_end );
      le_end->setValidator( qdv );
   }
   connect( le_end, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_end->setMinimumWidth( 80 );
   select_widgets.push_back( le_end );

   lbl_end_name =  new QLabel      ( ( (US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->lb_files->text( le_end->text().toInt() - 1 ), this );
   lbl_end_name -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_end_name -> setPalette      ( PALET_NORMAL );
   AUTFBACK( lbl_end_name );
   lbl_end_name -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Normal) );

   select_widgets.push_back( lbl_end_name );

   pb_nth_only =  new QPushButton ( tr( "Select Only" ), this );
   pb_nth_only -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_nth_only -> setMinimumHeight( minHeight1 );
   pb_nth_only -> setPalette      ( PALET_PUSHB );
   connect( pb_nth_only, SIGNAL( clicked() ), SLOT( nth_only() ) );
   select_widgets.push_back( pb_nth_only );

   pb_nth_add =  new QPushButton ( tr( "Select Additionally" ), this );
   pb_nth_add -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_nth_add -> setMinimumHeight( minHeight1 );
   pb_nth_add -> setPalette      ( PALET_PUSHB );
   connect( pb_nth_add, SIGNAL( clicked() ), SLOT( nth_add() ) );
   select_widgets.push_back( pb_nth_add );

   // contain

   lbl_contain = new mQLabel ( tr( "Select by name" ), this);
   lbl_contain->setAlignment( Qt::AlignCenter | Qt::AlignVCenter);
   lbl_contain->setPalette( PALET_LABEL );
   AUTFBACK( lbl_contain );
   lbl_contain->setFont     ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   connect( lbl_contain, SIGNAL( pressed() ), SLOT( hide_contain() ) );

   lbl_contains =  new QLabel      ( tr( "Name contains:" ), this );
   lbl_contains -> setAlignment    ( Qt::AlignRight | Qt::AlignVCenter );
   lbl_contains -> setPalette      ( PALET_LABEL );
   AUTFBACK( lbl_contains );
   lbl_contains -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   contain_widgets.push_back( lbl_contains );

   le_contains = new QLineEdit(this, "le_contains Line Edit");
   le_contains->setText( parameters->count( "buffer_nth_contains" ) ? (*parameters)[ "buffer_nth_contains" ] : "" );
   le_contains->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_contains->setPalette( PALET_NORMAL );
   AUTFBACK( le_contains );
   le_contains->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   connect( le_contains, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_contains->setMinimumWidth( 80 );
   contain_widgets.push_back( le_contains );

   pb_contains_only =  new QPushButton ( tr( "Select Only" ), this );
   pb_contains_only -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_contains_only -> setMinimumHeight( minHeight1 );
   pb_contains_only -> setPalette      ( PALET_PUSHB );
   connect( pb_contains_only, SIGNAL( clicked() ), SLOT( contains_only() ) );

   contain_widgets.push_back( pb_contains_only );

   pb_contains_add =  new QPushButton ( tr( "Select Additionally" ), this );
   pb_contains_add -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_contains_add -> setMinimumHeight( minHeight1 );
   pb_contains_add -> setPalette      ( PALET_PUSHB );
   connect( pb_contains_add, SIGNAL( clicked() ), SLOT( contains_add() ) );

   contain_widgets.push_back( pb_contains_add );

   // intensity

   lbl_intensity = new mQLabel ( tr( "Select by intensity" ), this);
   lbl_intensity->setAlignment( Qt::AlignCenter | Qt::AlignVCenter);
   lbl_intensity->setPalette( PALET_LABEL );
   AUTFBACK( lbl_intensity );
   lbl_intensity->setFont     ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   connect( lbl_intensity, SIGNAL( pressed() ), SLOT( hide_intensity() ) );

   cb_q_range =  new QCheckBox   ( tr( "q range [A]:" ), this );
   cb_q_range -> setPalette      ( PALET_NORMAL );
   AUTFBACK( cb_q_range );
   cb_q_range -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( cb_q_range, SIGNAL( clicked() ), SLOT( update_enables() ) );

   intensity_widgets.push_back( cb_q_range );

   // probably compute min/max q range over all

   le_q_start = new QLineEdit(this, "le_q_start Line Edit");
   le_q_start->setText( "0" );
   le_q_start->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_q_start->setPalette( PALET_NORMAL );
   AUTFBACK( le_q_start );
   le_q_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   le_q_start->setValidator( new QDoubleValidator( le_q_start ) );
   ( (QDoubleValidator *)le_q_start->validator() )->setRange( 0, .5, 3 );
   connect( le_q_start, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_q_start->setMinimumWidth( 40 );
   intensity_widgets.push_back( le_q_start );

   le_q_end = new QLineEdit(this, "le_q_end Line Edit");
   le_q_end->setText( "5" );
   le_q_end->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_q_end->setPalette( PALET_NORMAL );
   AUTFBACK( le_q_end );
   le_q_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   le_q_end->setValidator( new QDoubleValidator( le_q_end ) );
   ( (QDoubleValidator *)le_q_end->validator() )->setRange( 0, .5, 3 );
   connect( le_q_end, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_q_end->setMinimumWidth( 40 );
   intensity_widgets.push_back( le_q_end );

   pb_i_avg_all =  new QPushButton ( tr( "Compute average intensity of all" ), this );
   pb_i_avg_all -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_i_avg_all -> setMinimumHeight( minHeight1 );
   pb_i_avg_all -> setPalette      ( PALET_PUSHB );
   connect( pb_i_avg_all, SIGNAL( clicked() ), SLOT( i_avg_all() ) );

   intensity_widgets.push_back( pb_i_avg_all );

   pb_i_avg_sel =  new QPushButton ( tr( "Compute average intensity of selected" ), this );
   pb_i_avg_sel -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_i_avg_sel -> setMinimumHeight( minHeight1 );
   pb_i_avg_sel -> setPalette      ( PALET_PUSHB );
   connect( pb_i_avg_sel, SIGNAL( clicked() ), SLOT( i_avg_sel() ) );

   intensity_widgets.push_back( pb_i_avg_sel );

   te_q = new Q3TextEdit(this, "te_q Line Edit");
   te_q->setText( "" );
   te_q->setPalette( PALET_NORMAL );
   AUTFBACK( te_q );
   te_q->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   te_q->setMinimumWidth( 80 );

   intensity_widgets.push_back( te_q );

   rb_i_above =  new QRadioButton( tr( "Above " ), this );
   rb_i_above -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_i_above );
   rb_i_above -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_i_above, SIGNAL( clicked() ), SLOT( update_enables() ) );

   intensity_widgets.push_back( rb_i_above );

   rb_i_below =  new QRadioButton( tr( "Below intensity level [A.U.]:" ), this );
   rb_i_below -> setPalette      ( PALET_NORMAL );
   AUTFBACK( rb_i_below );
   rb_i_below -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   connect( rb_i_below, SIGNAL( clicked() ), SLOT( update_enables() ) );

   intensity_widgets.push_back( rb_i_below );

   bg_i_above_below = new QButtonGroup( this );
   int bg_pos = 0;
   bg_i_above_below->setExclusive(true);
   bg_i_above_below->addButton( rb_i_above, bg_pos++ );
   bg_i_above_below->addButton( rb_i_below, bg_pos++ );
   rb_i_below->setChecked( true );

   // probably compute min/max q range over all

   le_i_level = new QLineEdit(this, "le_i_level Line Edit");
   le_i_level->setText( "" );
   le_i_level->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_i_level->setPalette( PALET_NORMAL );
   AUTFBACK( le_i_level );
   le_i_level->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   le_i_level->setValidator( new QDoubleValidator( le_i_level ) );
   // ( (QDoubleValidator *)le_i_level->validator() )->setRange( 0, .5, 3 );
   connect( le_i_level, SIGNAL( textChanged( const QString & ) ), SLOT( update_i_level() ) );
   le_i_level->setMinimumWidth( 80 );
   intensity_widgets.push_back( le_i_level );

   pb_i_only =  new QPushButton ( tr( "Select Only" ), this );
   pb_i_only -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_i_only -> setMinimumHeight( minHeight1 );
   pb_i_only -> setPalette      ( PALET_PUSHB );
   connect( pb_i_only, SIGNAL( clicked() ), SLOT( i_only() ) );

   intensity_widgets.push_back( pb_i_only );

   pb_i_add =  new QPushButton ( tr( "Select Additionally" ), this );
   pb_i_add -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_i_add -> setMinimumHeight( minHeight1 );
   pb_i_add -> setPalette      ( PALET_PUSHB );
   connect( pb_i_add, SIGNAL( clicked() ), SLOT( i_add() ) );

   intensity_widgets.push_back( pb_i_add );

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
   plot_data->setAxisTitle(QwtPlot::xBottom, tr( "Curve position" ) );
   plot_data->setAxisTitle(QwtPlot::yLeft, tr("Average Intensity [a.u.]"));
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

   intensity_widgets.push_back( plot_data );

   qwtw_wheel = new QwtWheel( this );
   qwtw_wheel->setMass         ( 0.5 );
   qwtw_wheel->setMinimumHeight( minHeight1 );
   qwtw_wheel->setEnabled      ( false );
   connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );

   intensity_widgets.push_back( qwtw_wheel );

   pb_clear_plot =  new QPushButton ( tr( "Clear" ), this );
   pb_clear_plot -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_clear_plot -> setMinimumHeight( minHeight1 );
   pb_clear_plot -> setPalette      ( PALET_PUSHB );
   connect( pb_clear_plot, SIGNAL( clicked() ), SLOT( clear_plot() ) );

   intensity_widgets.push_back( pb_clear_plot );

   pb_color_rotate =  new QPushButton ( tr( "Rotate colors" ), this );
   pb_color_rotate -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_color_rotate -> setMinimumHeight( minHeight1 );
   pb_color_rotate -> setPalette      ( PALET_PUSHB );
   connect( pb_color_rotate, SIGNAL( clicked() ), SLOT( color_rotate() ) );

   intensity_widgets.push_back( pb_color_rotate );

   pb_save_dat =  new QPushButton ( tr( "Save .DAT" ), this );
   pb_save_dat -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_save_dat -> setMinimumHeight( minHeight1 );
   pb_save_dat -> setPalette      ( PALET_PUSHB );
   connect( pb_save_dat, SIGNAL( clicked() ), SLOT( save_dat() ) );

   intensity_widgets.push_back( pb_save_dat );

   // bottom

   pb_help =  new QPushButton ( tr( "Help" ), this );
   pb_help -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_help -> setMinimumHeight( minHeight1 );
   pb_help -> setPalette      ( PALET_PUSHB );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

   pb_quit =  new QPushButton ( tr( "Quit" ), this );
   pb_quit -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_quit -> setMinimumHeight( minHeight1 );
   pb_quit -> setPalette      ( PALET_PUSHB );
   connect( pb_quit, SIGNAL( clicked() ), SLOT( quit() ) );

   pb_do_select =  new QPushButton ( tr( "Select in main window" ), this );
   pb_do_select -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_do_select -> setMinimumHeight( minHeight1 );
   pb_do_select -> setPalette      ( PALET_PUSHB );
   connect( pb_do_select, SIGNAL( clicked() ), SLOT( do_select() ) );

   pb_go =  new QPushButton ( tr( "Select in main window and exit" ), this );
   pb_go -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_go -> setMinimumHeight( minHeight1 );
   pb_go -> setPalette      ( PALET_PUSHB );
   connect( pb_go, SIGNAL( clicked() ), SLOT( go() ) );

   Q3BoxLayout *background = new Q3HBoxLayout( this );
   Q3VBoxLayout *left = new Q3VBoxLayout( 0 );

   left->addWidget( lbl_title );

   Q3HBoxLayout *hbl_files_pane = new Q3HBoxLayout( 0 );
   {
      Q3VBoxLayout *vbl_files = new Q3VBoxLayout( 0 );
      vbl_files->addWidget( lbl_files );
      vbl_files->addWidget( lb_files );
      hbl_files_pane->addLayout( vbl_files );
   }
   {
      Q3VBoxLayout *vbl_files_sel = new Q3VBoxLayout( 0 );
      vbl_files_sel->addWidget( lbl_files_sel );
      vbl_files_sel->addWidget( lb_files_sel );
      hbl_files_pane->addLayout( vbl_files_sel );
   }

   left->addLayout( hbl_files_pane );
   left->addWidget( lbl_files_selected );

   left->addWidget( lbl_select_nth );
   {
      Q3GridLayout *gl_nth = new Q3GridLayout( 0 );

      gl_nth->addWidget         ( lbl_n          , 0, 0 );
      gl_nth->addWidget         ( le_n           , 0, 1 );
      gl_nth->addWidget         ( lbl_start      , 1, 0 );
      gl_nth->addWidget         ( le_start       , 1, 1 );
      gl_nth->addWidget         ( lbl_start_name , 1, 2 );
      gl_nth->addWidget         ( lbl_end        , 2, 0 );
      gl_nth->addWidget         ( le_end         , 2, 1 );
      gl_nth->addWidget         ( lbl_end_name   , 2, 2 );

      left->addLayout( gl_nth );
      Q3HBoxLayout *hbl_sel = new Q3HBoxLayout( 0 );
      hbl_sel->addWidget ( pb_nth_only );
      hbl_sel->addWidget ( pb_nth_add );
      left->addLayout ( hbl_sel );
   }

   left->addWidget( lbl_contain );
   {
      Q3GridLayout *gl_contains = new Q3GridLayout( 0 );

      gl_contains->addWidget         ( lbl_contains    , 0, 0 );
      gl_contains->addWidget         ( le_contains     , 0, 1 );

      left->addLayout( gl_contains );

      Q3HBoxLayout *hbl_sel = new Q3HBoxLayout( 0 );
      hbl_sel->addWidget ( pb_contains_only );
      hbl_sel->addWidget ( pb_contains_add );
      left->addLayout ( hbl_sel );
   }

   left->addWidget( lbl_intensity );
   {
      Q3GridLayout *gl_intensity = new Q3GridLayout( 0 );

      int j = 0;

      gl_intensity->addWidget         ( cb_q_range    , j, 0 );

      {
         Q3BoxLayout *hbl = new Q3HBoxLayout( 0 );
         hbl->addWidget( le_q_start );
         hbl->addWidget( le_q_end );
         gl_intensity->addMultiCellLayout( hbl, j, j, 1, 2 );
         ++j;
      }
      {
         Q3BoxLayout *hbl = new Q3HBoxLayout( 0 );
         hbl->addWidget( pb_i_avg_all );
         hbl->addWidget( pb_i_avg_sel );
         gl_intensity->addMultiCellLayout( hbl, j, j, 0, 2 );
         ++j;
      }

      gl_intensity->addMultiCellWidget( te_q, j, j + 2, 0, 2);
      j += 3;

      {
         Q3BoxLayout *hbl = new Q3HBoxLayout( 0 );
         hbl->addWidget( rb_i_above );
         hbl->addWidget( rb_i_below );
         gl_intensity->addLayout( hbl, j, 0 );
      }

      gl_intensity->addMultiCellWidget( le_i_level, j, j, 1, 2 );
      ++j;

      left->addLayout( gl_intensity );

      Q3HBoxLayout *hbl_sel = new Q3HBoxLayout( 0 );
      hbl_sel->addWidget ( pb_i_only );
      hbl_sel->addWidget ( pb_i_add );
      left->addLayout ( hbl_sel );
   }

   Q3HBoxLayout *hbl_bottom = new Q3HBoxLayout( 0 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addWidget ( pb_quit );
   hbl_bottom->addWidget ( pb_do_select );
   hbl_bottom->addWidget ( pb_go );

   left->addSpacing( 4 );
   left->addLayout ( hbl_bottom );

   background->addLayout( left );

   {
      Q3BoxLayout * vbl = new Q3VBoxLayout( 0 );
      vbl->addWidget( plot_data );
      vbl->addWidget( qwtw_wheel );

      {
         Q3BoxLayout * hbl = new Q3HBoxLayout( 0 );
         hbl->addWidget( pb_clear_plot );
         hbl->addWidget( pb_color_rotate );
         hbl->addWidget( pb_save_dat );
         vbl->addLayout( hbl );
      }
      background->addLayout( vbl );
   }      

   ShowHide::hide_widgets( select_widgets );
   ShowHide::hide_widgets( contain_widgets );
   ShowHide::hide_widgets( intensity_widgets );
   if ( (*parameters)[ "buffer_nth_shown" ] == "contain" )
   {
      ShowHide::hide_widgets( contain_widgets, false, this );
   } else {
      if ( (*parameters)[ "buffer_nth_shown" ] == "intensity" )
      {
         ShowHide::hide_widgets( intensity_widgets, false, this );
      } else {
         ShowHide::hide_widgets( intensity_widgets, false, this );
      }
   }
   if ( parameters->count( "buffer_nth_files_hidden" ) &&
        (*parameters)[ "buffer_nth_files_hidden" ] == "true" )
   {
      ShowHide::hide_widgets( files_widgets, true, this );
   }
}

void US_Hydrodyn_Saxs_Buffer_Nth::quit()
{
   close();
}

void US_Hydrodyn_Saxs_Buffer_Nth::do_select( bool update )
{

   for ( int i = 0; i < lb_files->numRows(); ++i )
   {
      if ( lb_files->isSelected( i ) )
      {
         (*parameters)[ QString( "%1" ).arg( i ) ] = "1";
      } else {
         if ( parameters->count( QString( "%1" ).arg( i ) ) )
         {
            parameters->erase( QString( "%1" ).arg( i ) );
         }
      }         
   }

   if ( update )
   {
      ((US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->select_these( *parameters );
   }
}

void US_Hydrodyn_Saxs_Buffer_Nth::go()
{
   (*parameters)[ "go"    ] = "true";
   
   do_select( false );

   (*parameters)[ "buffer_nth_contains" ] = le_contains->text();

   close();
}

void US_Hydrodyn_Saxs_Buffer_Nth::help()
{
   US_Help *online_help;
   online_help = new US_Help( this );
   online_help->show_help("manual/saxs_buffer_nth.html");
}

void US_Hydrodyn_Saxs_Buffer_Nth::closeEvent( QCloseEvent *e )
{

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Saxs_Buffer_Nth::update_enables()
{
   lbl_start_name->setText( ( (US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->lb_files->text( le_start->text().toInt() - 1 ) );
   lbl_end_name  ->setText( ( (US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->lb_files->text( le_end  ->text().toInt() - 1 ) );

   int n   = le_n->text().toInt();
   int ofs = le_start->text().toInt();
   int end = le_end->text().toInt();

   set < int > selected;
   set < int > contains;

   for ( int i = 0; i < lb_files->numRows(); ++i )
   {
      if ( lb_files->isSelected( i ) )
      {
         selected.insert( i );
      }
      if ( ((US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->lb_files->text( i ).contains( le_contains->text() ) )
      {
         contains.insert( i );
      }
   }

   int files_selected = (int) selected.size();

   bool any_in_range_not_selected = false;

   set < int > range;
   if ( n > 0 )
   {
      for ( int i = ofs - 1; i < end; i += n )
      {
         range.insert( i );
         if ( !any_in_range_not_selected &&
              !selected.count( i ) )
         {
            any_in_range_not_selected = true;
         }
      }
   }

   bool any_selected_not_in_range = false;
   for ( set < int >::iterator it = selected.begin();
         it != selected.end();
         it++ )
   {
      if ( !range.count( *it ) )
      {
         any_selected_not_in_range = true;
         break;
      }
   }

   pb_nth_only->setEnabled( any_in_range_not_selected || any_selected_not_in_range );
   pb_nth_add ->setEnabled( any_in_range_not_selected && any_selected_not_in_range );

   bool any_selected_not_contains = false;

   for ( set < int >::iterator it = selected.begin();
         it != selected.end();
         it++ )
   {
      if ( !contains.count( *it ) )
      {
         any_selected_not_contains = true;
         break;
      }
   }

   bool any_contains_not_selected = false;

   for ( set < int >::iterator it = contains.begin();
         it != contains.end();
         it++ )
   {
      if ( !selected.count( *it ) )
      {
         any_contains_not_selected = true;
         break;
      }
   }

   pb_contains_only->setEnabled( any_contains_not_selected || any_selected_not_contains );
   pb_contains_add ->setEnabled( any_contains_not_selected && any_selected_not_contains );

   lbl_files_selected->setText( QString( "%1 of %2 selected" ).arg( files_selected ).arg( lb_files->numRows() ) );

   le_q_start  ->setEnabled( cb_q_range->isChecked() );
   le_q_end    ->setEnabled( cb_q_range->isChecked() );
   pb_i_avg_sel->setEnabled( files_selected );


   // compute above / below

   bool any_selected_not_intensity = false;
   bool any_intensity_not_selected = false;

   set < int > intensity = get_intensity_selected();

   for ( set < int >::iterator it = intensity.begin();
         it != intensity.end();
         it++ )
   {
      if ( !selected.count( *it ) )
      {
         any_intensity_not_selected = true;
         break;
      }
   }

   for ( set < int >::iterator it = selected.begin();
         it != selected.end();
         it++ )
   {
      if ( !intensity.count( *it ) )
      {
         any_selected_not_intensity = true;
         break;
      }
   }

   pb_i_only ->setEnabled( !le_i_level->text().isEmpty() && ( any_intensity_not_selected || any_selected_not_intensity ) );
   pb_i_add  ->setEnabled( !le_i_level->text().isEmpty() && any_intensity_not_selected && any_selected_not_intensity );
}

void US_Hydrodyn_Saxs_Buffer_Nth::nth_only()
{
   disconnect( lb_files, SIGNAL( selectionChanged() ), 0, 0 );
   int n   = le_n->text().toInt();
   int ofs = le_start->text().toInt();
   int end = le_end->text().toInt();
   lb_files->clearSelection();
   if ( n > 0 )
   {
      for ( int i = ofs - 1; i < end; i += n )
      {
         lb_files->setSelected( i, true );
      }
   }
   connect( lb_files, SIGNAL( selectionChanged() ), SLOT( update_files_selected() ) );
   update_files_selected();
}

void US_Hydrodyn_Saxs_Buffer_Nth::nth_add()
{
   disconnect( lb_files, SIGNAL( selectionChanged() ), 0, 0 );
   int n   = le_n->text().toInt();
   int ofs = le_start->text().toInt();
   int end = le_end->text().toInt();
   if ( n > 0 )
   {
      for ( int i = ofs - 1; i < end; i += n )
      {
         lb_files->setSelected( i, true );
      }
   }
   connect( lb_files, SIGNAL( selectionChanged() ), SLOT( update_files_selected() ) );
   update_files_selected();
}

void US_Hydrodyn_Saxs_Buffer_Nth::contains_only()
{
   disconnect( lb_files, SIGNAL( selectionChanged() ), 0, 0 );
   lb_files->clearSelection();
   for ( int i = 0; i < lb_files->numRows(); ++i )
   {
      if ( ((US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->lb_files->text( i ).contains( le_contains->text() ) )
      {
         lb_files->setSelected( i, true );
      }
   }
   connect( lb_files, SIGNAL( selectionChanged() ), SLOT( update_files_selected() ) );
   update_files_selected();
}

void US_Hydrodyn_Saxs_Buffer_Nth::contains_add()
{
   disconnect( lb_files, SIGNAL( selectionChanged() ), 0, 0 );
   for ( int i = 0; i < lb_files->numRows(); ++i )
   {
      if ( ((US_Hydrodyn_Saxs_Buffer*)us_hydrodyn_saxs_buffer)->lb_files->text( i ).contains( le_contains->text() ) )
      {
         lb_files->setSelected( i, true );
      }
   }
   connect( lb_files, SIGNAL( selectionChanged() ), SLOT( update_files_selected() ) );
   update_files_selected();
}

void US_Hydrodyn_Saxs_Buffer_Nth::update_files_selected()
{
   lb_files_sel->clear();
   for ( int i = 0; i < lb_files->numRows(); ++i )
   {
      if ( lb_files->isSelected( i ) )
      {
         lb_files_sel->insertItem( lb_files->text( i ) );
      }
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Buffer_Nth::hide_select()
{
   if ( select_widgets.size() )
   {
      ShowHide::hide_widgets( select_widgets, select_widgets[ 0 ]->isVisible(), this );
      if ( select_widgets[ 0 ]->isVisible() )
      {
         (*parameters)[ "buffer_nth_shown" ] = "select";
      }
   }
   ShowHide::hide_widgets( contain_widgets, true, this );
   ShowHide::hide_widgets( intensity_widgets, true, this );
}

void US_Hydrodyn_Saxs_Buffer_Nth::hide_files()
{
   if ( files_widgets.size() )
   {
      ShowHide::hide_widgets( files_widgets, files_widgets[ 0 ]->isVisible(), this );
      if ( !files_widgets[ 0 ]->isVisible() )
      {
         (*parameters)[ "buffer_nth_files_hidden" ] = "true";
      } else {
         if ( parameters->count( "buffer_nth_files_hidden" ) )
         {
            parameters->erase( "buffer_nth_files_hidden" );
         }
      }
   }
}

void US_Hydrodyn_Saxs_Buffer_Nth::hide_contain()
{
   if ( contain_widgets.size() )
   {
      ShowHide::hide_widgets( contain_widgets, contain_widgets[ 0 ]->isVisible(), this );
      if ( contain_widgets[ 0 ]->isVisible() )
      {
         (*parameters)[ "buffer_nth_shown" ] = "contain";
      }
   }
   ShowHide::hide_widgets( select_widgets, true, this );
   ShowHide::hide_widgets( intensity_widgets, true, this );
}
        

void US_Hydrodyn_Saxs_Buffer_Nth::hide_intensity()
{
   if ( intensity_widgets.size() )
   {
      ShowHide::hide_widgets( intensity_widgets, intensity_widgets[ 0 ]->isVisible(), this );
      if ( intensity_widgets[ 0 ]->isVisible() )
      {
         (*parameters)[ "buffer_nth_shown" ] = "intensity";
      }
   }
   ShowHide::hide_widgets( select_widgets, true, this );
   ShowHide::hide_widgets( contain_widgets, true, this );
}
        
void US_Hydrodyn_Saxs_Buffer_Nth::i_avg( QStringList files )
{
   double min_i = 0e0;
   double max_i = 0e0;
   double tot_i = 0e0;

   double q_min = 0e0;
   double q_max = 6e0;

   QString msg;

   if ( !files.size() )
   {
      te_q->setText( tr( "nothing to compute" ) );
      return;
   }

   if ( cb_q_range->isChecked() )
   {
      q_min = le_q_start->text().toDouble();
      q_max = le_q_end  ->text().toDouble();
      msg += QString( tr( "q range clipped to %1 %2\n" ) ).arg( q_min ).arg( q_max );
   }

   vector < double > x;
   vector < double > y;

   for ( int i = 0; i < (int) files.size(); ++i )
   {
      x.push_back( QString( files[ i ] ).replace( QRegExp( " :.*$" ), "" ).toDouble() );
      double this_i = ((US_Hydrodyn_Saxs_Buffer *)us_hydrodyn_saxs_buffer)->tot_intensity( QString( files[ i ] ).replace( QRegExp( "^\\d+ : " ), "" ), q_min, q_max );
      y.push_back( this_i );
      if ( !i || min_i > this_i )
      {
         min_i = this_i;
      }
      if ( !i || max_i < this_i )
      {
         max_i = this_i;
      }
      tot_i += this_i;
   }

   // US_Vector::printvector2( "plot", x, y );

   msg += QString( 
                  "minimum total I : %1\n"
                  "maximum total I : %2\n"
                  "average total I : %3\n" 
                   )
      .arg( min_i )
      .arg( max_i )
      .arg( tot_i / files.size() )
      ;

   te_q->setText( msg );

   qwtw_wheel->setRange( min_i, max_i, ( max_i - min_i ) / 10000000 );
   update_i_level();

   QString plotname = QString( files[ 0 ] ).replace( QRegExp( "^\\d+ : " ), "" ).replace( QRegExp( ".(dat|DAT)$" ), "" );
   plotname += cb_q_range->isChecked() ?
      QString( "_qs%1_qe%2" ).arg( q_min ).arg( q_max ).replace( ".", "_" ) :
      QString( "_q_all" );
      
#ifndef QT4
   long curve = plot_data->insertCurve( plotname );
   plot_data->setCurveStyle( curve, QwtCurve::Lines );
#else
   QwtPlotCurve *curve = new QwtPlotCurve( plotname );
   curve->setStyle( QwtPlotCurve::Lines );
#endif
   plotted_curves.push_back( curve );
   plotted_names.push_back( plotname );
   plotted_x.push_back( x );
   plotted_y.push_back( y );

#ifndef QT4
   plot_data->setCurveData( curve,
                            (double *)&( x[ 0 ] ),
                            (double *)&( y[ 0 ] ),
                            x.size()
                            );
   plot_data->setCurvePen( curve, QPen( pc->color( (int) plotted_curves.size() - 1 ), 1, SolidLine));
#else
   curve->setData(
                  (double *)&( x ),
                  (double *)&( y ),
                  x.size()
                  );

   curve->setPen( QPen( pc->color( (int) plotted_curves.size() - 1 ), 1, Qt::SolidLine ) );

   curve->attach( plot_data );
#endif

   if ( !plot_data_zoomer )
   {
      plot_data_zoomer = new ScrollZoomer(plot_data->canvas());
      plot_data_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
#ifndef QT4
      plot_data_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
   }

   plot_data->setAxisScale( QwtPlot::xBottom , x[ 0 ] - 1, x.back() + 1);
   //   plot_data->setAxisScale( QwtPlot::yLeft   , min_i * 0.90, max_i * 1.1e0 );

   plot_data->replot();
}


void US_Hydrodyn_Saxs_Buffer_Nth::i_avg_all()
{
   i_avg( MQT::get_lb_qsl( lb_files ) );
}

void US_Hydrodyn_Saxs_Buffer_Nth::i_avg_sel()
{
   i_avg( MQT::get_lb_qsl( lb_files, true ) );
}

set < int > US_Hydrodyn_Saxs_Buffer_Nth::get_intensity_selected()
{
   QStringList files = MQT::get_lb_qsl( lb_files );

   set < int > result;

   if ( !files.size() )
   {
      te_q->setText( tr( "nothing to compute" ) );
      return result;
   }

   double q_min = 0e0;
   double q_max = 6e0;

   if ( cb_q_range->isChecked() )
   {
      q_min = le_q_start->text().toDouble();
      q_max = le_q_end  ->text().toDouble();
   }

   double cutoff = le_i_level->text().toDouble();

   for ( int i = 0; i < (int) files.size(); ++i )
   {
      double this_i = ((US_Hydrodyn_Saxs_Buffer *)us_hydrodyn_saxs_buffer)->tot_intensity( QString( files[ i ] ).replace( QRegExp( "^\\d+ : " ), "" ), q_min, q_max );
      if ( rb_i_above->isChecked() ? ( this_i >= cutoff ) : ( this_i <= cutoff ) )
      {
         result.insert( i );
      }
   }
   //   qDebug( QString( "get_intensity_selected files size %1 q_min %2 q_max %3 cutoff %4" ).arg( files.size() ).arg( q_min ).arg( q_max ).arg( cutoff ) );
   // for ( set < int >::iterator it = result.begin();
   //       it != result.end();
   //       it++ )
   // {
   //    qDebug( QString( "get_intensity_selected to select %1" ).arg( *it ) );
   // }
   return result;
}

void US_Hydrodyn_Saxs_Buffer_Nth::i_only()
{
   set < int > to_select = get_intensity_selected();
   disconnect( lb_files, SIGNAL( selectionChanged() ), 0, 0 );
   lb_files->clearSelection();
   for ( set < int >::iterator it = to_select.begin();
         it != to_select.end();
         it++ )
   {
      lb_files->setSelected( *it, true );
   }
   connect( lb_files, SIGNAL( selectionChanged() ), SLOT( update_files_selected() ) );
   update_files_selected();
}

void US_Hydrodyn_Saxs_Buffer_Nth::i_add()
{
   set < int > to_select = get_intensity_selected();
   disconnect( lb_files, SIGNAL( selectionChanged() ), 0, 0 );
   for ( set < int >::iterator it = to_select.begin();
         it != to_select.end();
         it++ )
   {
      lb_files->setSelected( *it, true );
   }
   connect( lb_files, SIGNAL( selectionChanged() ), SLOT( update_files_selected() ) );
   update_files_selected();
}

void US_Hydrodyn_Saxs_Buffer_Nth::clear_plot()
{
   plotted_curves.clear();
   plotted_names .clear();
   plotted_x     .clear();
   plotted_y     .clear();

   plot_data->clear();
   plot_data->replot();

#ifdef QT4
   plot_marker     = (QwtPlotMarker *) 0;
#else
   plot_marker     = (long *) 0;
#endif
   qwtw_wheel      ->setEnabled( false );
}

void US_Hydrodyn_Saxs_Buffer_Nth::color_rotate()
{
   pc->color_rotate();
   for ( int i = 0; i < (int) plotted_curves.size(); ++i )
   {
#ifndef QT4
      plot_data->setCurvePen( plotted_curves[ i ],  QPen( pc->color( i ), 1, SolidLine));
#else
      plotted_curves[ i ]->setPen( QPen( pc->color( i ), 1, Qt::SolidLine ) );
#endif
   }
   plot_data->replot();
}

void US_Hydrodyn_Saxs_Buffer_Nth::save_dat()
{
   if ( !plotted_curves.size() )
   {
      return;
   }


   QString use_dir = QDir::currentDirPath();
   ((US_Hydrodyn  *)us_hydrodyn)->select_from_directory_history( use_dir, this );

   QString s = Q3FileDialog::getExistingDirectory(
                                                 use_dir,
                                                 this,
                                                 "get existing directory",
                                                 tr( "Choose a directory to save the intensity plots" ),
                                                 true );
   if ( !s.isEmpty() )
   {
      use_dir = s;
      QDir::setCurrent( s );
      ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( s );
   }

   bool cancel = false;

   bool overwrite_all = false;

   for ( int i = 0; i < (int) plotted_curves.size(); ++i )
   {
      QString fname = plotted_names[ i ] + ".dat";

      if ( QFile::exists( fname ) )
      {
         fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck2( fname, cancel, overwrite_all, 0, this );
         raise();
         if ( cancel )
         {
            return;
         }
      }
      
      QFile f( fname );
      if ( !f.open( QIODevice::WriteOnly ) )
      {
         QMessageBox::warning( this, 
                               caption(),
                               tr( QString( "could not open %1 for writing" ).arg( f.name() ) ) );
         return;
      }

      Q3TextStream ts( &f );

      ts << QString( "Time: average intensity %1\n" ).arg( plotted_names[ i ] );
      ts << "t I(t)\n";

      for ( int j = 0; j < (int) plotted_x[ i ].size(); ++j )
      {
         ts << plotted_x[ i ][ j ] << " " << plotted_y[ i ][ j ] << endl;
      }

      f.close();
   }

   QMessageBox::information( this, 
                             caption() + tr( " : Write plotted files" ),
                             QString( tr( "%1 file%2 saved in direrctory %3" ) )
                             .arg( plotted_curves.size() )
                             .arg( plotted_curves.size() > 1 ? "s" : "" )
                             .arg( use_dir ) );
}

void US_Hydrodyn_Saxs_Buffer_Nth::update_i_level()
{
   // add plot marker

   if ( !plot_marker )
   {
#ifndef QT4
      plot_marker = new long;
      *plot_marker = plot_data->insertMarker();
      plot_data->setMarkerLineStyle ( *plot_marker, QwtMarker::HLine );
      plot_data->setMarkerPen       ( *plot_marker, QPen( Qt::red, 1, DashDotDotLine));
#else
      plot_marker = new QwtPlotMarker;
      plot_marker->setLineStyle     ( QwtPlotMarker::VLine );
      plot_marker->setLinePen       ( QPen( Qt::red, 1, Qt::DashDotDotLine ) );
      plot_marker->attach           ( plot_data );
#endif
      qwtw_wheel->setEnabled        ( true );
   }

#ifndef QT4
   plot_data->setMarkerPos          ( *plot_marker, 0, le_i_level->text().toDouble() );
#else
   plot_marker->setYValue           ( le_i_level->text().toDouble() );
#endif
   plot_data->replot();

   if ( qwtw_wheel->value() != le_i_level->text().toDouble() )
   {
      qwtw_wheel->setValue( le_i_level->text().toDouble() );
   }

   update_enables();
}

void US_Hydrodyn_Saxs_Buffer_Nth::adjust_wheel( double pos )
{
   le_i_level->setText( QString( "%1" ).arg( pos ) );
}
