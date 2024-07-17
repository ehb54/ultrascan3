#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_saxs_util.h"
#include "../include/us_hydrodyn_mals_saxs_baseline_best.h"
//Added by qt3to4:
#include <QTextStream>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QFrame>
 //#include <Q3PopupMenu>
#include <QVBoxLayout>
#include <QCloseEvent>

US_Hydrodyn_Mals_Saxs_Baseline_Best::US_Hydrodyn_Mals_Saxs_Baseline_Best(
                                                                         void                         *          us_hydrodyn,
                                                                         map < QString, QString >                parameters,
                                                                         map < QString, double >                 dparameters,
                                                                         map < QString, vector < double > >      vdparameters,
                                                                         QWidget *                               p,
                                                                         const char *                            
                                                                         ) : QFrame(  p )
{
   this->us_hydrodyn                          = us_hydrodyn;
   this->parameters                           = parameters;
   this->dparameters                          = dparameters;
   this->vdparameters                         = vdparameters;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle( us_tr( "US-SOMO: HPLC SAXS Baseline Best Region Analysis" ) +
               ( parameters.count( "name" ) ? QString( " : %1" ).arg( parameters[ "name" ] ) : QString( "" ) ) );

   plot_zoomer         = ( ScrollZoomer * )0;
   hb_plot_zoomer      = ( ScrollZoomer * )0;

   setupGUI();
   ((US_Hydrodyn*)us_hydrodyn)->fixWinButtons( this );

   global_Xpos += 30;
   global_Ypos += 30;

   // global_Xpos = 0;
   // global_Ypos = 0;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );

   update_enables();
   displayData();
}

US_Hydrodyn_Mals_Saxs_Baseline_Best::~US_Hydrodyn_Mals_Saxs_Baseline_Best()
{
}

#define UHSC_IMG_MIN 200
#define UHSC_IMG_MAX 768

void US_Hydrodyn_Mals_Saxs_Baseline_Best::setupGUI()
{
   int minHeight1  = 30;

   qs_left   = new QSplitter( Qt::Vertical  , this );
   qs_right  = new QSplitter( Qt::Vertical  , this );

   lbl_title = new QLabel( "", this ); 

   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette( PALET_LABEL );
   AUTFBACK( lbl_title );
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_hb_title = new QLabel( "", this ); 

   lbl_hb_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_hb_title->setMinimumHeight(minHeight1);
   lbl_hb_title->setPalette( PALET_LABEL );
   AUTFBACK( lbl_hb_title );
   lbl_hb_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

//   plot = new QwtPlot( qs_left );
   usp_plot = new US_Plot( plot, "", "", "", qs_left );
   connect( (QWidget *)plot->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot( const QPoint & ) ) );
   ((QWidget *)plot->titleLabel())->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot->axisWidget( QwtPlot::yLeft ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot( const QPoint & ) ) );
   ((QWidget *)plot->axisWidget( QwtPlot::yLeft ))->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot->axisWidget( QwtPlot::xBottom ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot( const QPoint & ) ) );
   ((QWidget *)plot->axisWidget( QwtPlot::xBottom ))->setContextMenuPolicy( Qt::CustomContextMenu );
#if QT_VERSION < 0x040000
   plot->enableGridXMin();
   plot->enableGridYMin();
#else
   plot_grid = new QwtPlotGrid;
   plot_grid->enableXMin( true );
   plot_grid->enableYMin( true );
#endif
   plot->setPalette( PALET_NORMAL );
   AUTFBACK( plot );
#if QT_VERSION < 0x040000
   plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   plot_grid->setMajorPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   plot_grid->setMinorPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   plot_grid->attach( plot );
#endif
   plot->setAxisTitle(QwtPlot::xBottom, parameters.count( "xlegend" ) ? parameters[ "xlegend" ] : us_tr( "Start Frame" ) );
   plot->setAxisTitle(QwtPlot::yLeft  , us_tr( "Average Red Cluster Size" ) );
#if QT_VERSION < 0x040000
   plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
//    plot->setMargin(USglobal->config_list.margin);
   plot->setTitle("");
#if QT_VERSION < 0x040000
   plot->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::None);
#else
   plot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine );
#endif
   plot->setCanvasBackground(USglobal->global_colors.plot);

#if QT_VERSION < 0x040000
   plot->setAutoLegend( true );
   plot->setLegendPosition( QwtPlot::Right );
   plot->setLegendFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );

#else
   // {
   //    QwtLegend* legend_pd = new QwtLegend;
   //    legend_pd->setFrameStyle( QFrame::Box | QFrame::Sunken );
   //    guinier_plot->insertLegend( legend_pd, QwtPlot::BottomLegend );
   // }
#endif

   plot_name = windowTitle().replace( "US-SOMO:", "" ).replace( " ", "_" );
   if ( ((US_Hydrodyn *)us_hydrodyn)->mals_saxs_widget ) {
      US_Hydrodyn_Mals_Saxs *mals_saxs_win = ((US_Hydrodyn *)us_hydrodyn)->mals_saxs_window;
      mals_saxs_win->plot_info[ plot_name ] = plot;
   }

//   hb_plot = new QwtPlot( qs_right );
   usp_hb_plot = new US_Plot( hb_plot, "", "", "", qs_right );
   connect( (QWidget *)hb_plot->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_hb_plot( const QPoint & ) ) );
   ((QWidget *)hb_plot->titleLabel())->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)hb_plot->axisWidget( QwtPlot::yLeft ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_hb_plot( const QPoint & ) ) );
   ((QWidget *)hb_plot->axisWidget( QwtPlot::yLeft ))->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)hb_plot->axisWidget( QwtPlot::xBottom ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_hb_plot( const QPoint & ) ) );
   ((QWidget *)hb_plot->axisWidget( QwtPlot::xBottom ))->setContextMenuPolicy( Qt::CustomContextMenu );
#if QT_VERSION < 0x040000
   hb_plot->enableGridXMin();
   hb_plot->enableGridYMin();
#else
   hb_plot_grid = new QwtPlotGrid;
   hb_plot_grid->enableXMin( true );
   hb_plot_grid->enableYMin( true );
#endif
   hb_plot->setPalette( PALET_NORMAL );
   AUTFBACK( hb_plot );
#if QT_VERSION < 0x040000
   hb_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   hb_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   hb_plot_grid->setMajorPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   hb_plot_grid->setMinorPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   hb_plot_grid->attach( hb_plot );
#endif
   hb_plot->setAxisTitle(QwtPlot::xBottom, parameters.count( "xlegend" ) ? parameters[ "xlegend" ] : us_tr( "Start Frame" ) );
   hb_plot->setAxisTitle(QwtPlot::yLeft  , us_tr( "Average Red Cluster Size" ) );
#if QT_VERSION < 0x040000
   hb_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   hb_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   hb_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   hb_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   hb_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   hb_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   hb_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
//    hb_plot->setMargin(USglobal->config_list.margin);
   hb_plot->setTitle("");
#if QT_VERSION < 0x040000
   hb_plot->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::None);
#else
   hb_plot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine );
#endif
   hb_plot->setCanvasBackground(USglobal->global_colors.plot);

#if QT_VERSION < 0x040000
   hb_plot->setAutoLegend( true );
   hb_plot->setLegendPosition( QwtPlot::Right );
   hb_plot->setLegendFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );

#else
   // {
   //    QwtLegend* legend_pd = new QwtLegend;
   //    legend_pd->setFrameStyle( QFrame::Box | QFrame::Sunken );
   //    guinier_plot->insertLegend( legend_pd, QwtPlot::BottomLegend );
   // }
#endif

   hb_plot_name = windowTitle().replace( "US-SOMO:", "" ).replace( " ", "_" ) + "_hb";
   if ( ((US_Hydrodyn *)us_hydrodyn)->mals_saxs_widget ) {
      US_Hydrodyn_Mals_Saxs *mals_saxs_win = ((US_Hydrodyn *)us_hydrodyn)->mals_saxs_window;
      mals_saxs_win->plot_info[ hb_plot_name ] = hb_plot;
   }

   //   QFrame *editor_frame = new QFrame( qs_left );

   editor = new QTextEdit( qs_left );
   editor->setPalette( PALET_NORMAL );
   AUTFBACK( editor );
   editor->setReadOnly(true);
   editor->setFont( QFont( "Courier", USglobal->config_list.fontSize ) );

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
   // QFrame *frame;
   // frame = new QFrame( qs_left );
   // frame->setMinimumHeight(minHeight1);

   // m = new QMenuBar( frame );  m->setObjectName( "menu" );
   // m->setMinimumHeight(minHeight1 - 5);
   // m->setPalette( PALET_NORMAL );

   // QPopupMenu * file = new QPopupMenu(editor);
   // m->insertItem( us_tr("&File"), file );
   // file->insertItem( us_tr("Font"),  this, SLOT(update_font()),    ALT+Key_F );
   // file->insertItem( us_tr("Save"),  this, SLOT(save()),    ALT+Key_S );
   // file->insertItem( us_tr("Clear Display"), this, SLOT(clear_display()),   ALT+Key_X );
# endif
#endif
   
   editor->setWordWrapMode (QTextOption::WordWrap);
   editor->setMinimumHeight( minHeight1 * 9 );
   // editor->setTextFormat( Qt::RichText );

   hb_editor = new QTextEdit( qs_right );
   hb_editor->setPalette( PALET_NORMAL );
   AUTFBACK( hb_editor );
   hb_editor->setReadOnly(true);
   hb_editor->setFont( QFont( "Courier", USglobal->config_list.fontSize ) );

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
   // QFrame *frame;
   // frame = new QFrame( qs_right );
   // frame->setMinimumHeight(minHeight1);

   // m = new QMenuBar( frame );  m->setObjectName( "menu" );
   // m->setMinimumHeight(minHeight1 - 5);
   // m->setPalette( PALET_NORMAL );

   // QPopupMenu * file = new QPopupMenu(hb_editor);
   // m->insertItem( us_tr("&File"), file );
   // file->insertItem( us_tr("Font"),  this, SLOT(update_font()),    ALT+Key_F );
   // file->insertItem( us_tr("Save"),  this, SLOT(save()),    ALT+Key_S );
   // file->insertItem( us_tr("Clear Display"), this, SLOT(clear_display()),   ALT+Key_X );
# endif
#endif

   hb_editor->setWordWrapMode (QTextOption::WordWrap);
   hb_editor->setMinimumHeight( minHeight1 * 9 );
   // hb_editor->setTextFormat( Qt::RichText );

   pb_set_best =  new QPushButton ( us_tr( "Set region in HPLC window" ), this );
   pb_set_best -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_set_best -> setMinimumHeight( minHeight1 );
   pb_set_best -> setPalette      ( PALET_PUSHB );
   connect( pb_set_best, SIGNAL( clicked() ), SLOT( set_best() ) );
   
   if ( !((US_Hydrodyn *)us_hydrodyn)->mals_saxs_widget ) {
      pb_set_best->hide();
   }

   pb_set_hb_best =  new QPushButton ( us_tr( "Set region in HPLC window" ), this );
   pb_set_hb_best -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_set_hb_best -> setMinimumHeight( minHeight1 );
   pb_set_hb_best -> setPalette      ( PALET_PUSHB );
   connect( pb_set_hb_best, SIGNAL( clicked() ), SLOT( set_hb_best() ) );
   
   if ( !((US_Hydrodyn *)us_hydrodyn)->mals_saxs_widget ) {
      pb_set_hb_best->hide();
   }

   cb_show_hb = new QCheckBox( this );
   cb_show_hb -> setText( us_tr( "Show Holm-Bonferroni computed analysis" ) );
   cb_show_hb -> setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ) );
   cb_show_hb -> setPalette( PALET_NORMAL );
   AUTFBACK( cb_show_hb );
   connect( cb_show_hb, SIGNAL( clicked() ), SLOT( update_enables() ) );

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

   if ( parameters.count( "hb" ) ) {
      lbl_hb_title  ->hide();
      pb_set_hb_best->hide();
      hb_plot       ->hide();
      hb_editor     ->hide();
      qs_right      ->hide();
      cb_show_hb    ->hide();
   }
   cb_show_hb    ->hide();

   QVBoxLayout * background = new QVBoxLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );

   //    QBoxLayout *vbl_editor_group = new QVBoxLayout(qs_right);
   // #if QT_VERSION < 0x040000 || !defined(Q_OS_MAC)
   //    vbl_editor_group->addWidget ( frame );
   // #endif
   //    vbl_editor_group->addWidget ( editor );

   if ( 1 || parameters.count( "hb" ) ) {
      background->addWidget( lbl_title );
   }

   {
      QHBoxLayout * hb = new QHBoxLayout(); hb->setContentsMargins( 0, 0, 0, 0 ); hb->setSpacing( 0 );
      {

         QVBoxLayout * vbl_left = new QVBoxLayout( 0 ); vbl_left->setContentsMargins( 0, 0, 0, 0 ); vbl_left->setSpacing( 0 );
         if ( 0 && !parameters.count( "hb" ) ) {
            vbl_left->addWidget( lbl_title );
         }

         vbl_left->addWidget( qs_left );
      
         if ( !parameters.count( "hb" ) ) {
            //            vbl_left->addSpacing( 2 );
            vbl_left->addWidget( pb_set_best );
         }
         hb->addLayout( vbl_left );
      }

      if ( 0 ) {
         QVBoxLayout * vbl_right = new QVBoxLayout( 0 ); vbl_right->setContentsMargins( 0, 0, 0, 0 ); vbl_right->setSpacing( 0 );
         vbl_right->addWidget( lbl_hb_title );
         vbl_right->addWidget( qs_right );
         //         vbl_right->addSpacing( 2 );
         vbl_right->addWidget( pb_set_hb_best );

         if ( !parameters.count( "hb" ) ) {
            hb->addLayout( vbl_right );
         }
      }

      background->addLayout( hb );
   }

   if ( parameters.count( "hb" ) ) {
      // background->addSpacing( 2 );
      background->addWidget( pb_set_best );
   }

   background->addWidget( cb_show_hb );

   QHBoxLayout * hbl_bottom = new QHBoxLayout(); hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addWidget ( pb_close );

   background->addSpacing( 4 );
   background->addLayout ( hbl_bottom );
}

void US_Hydrodyn_Mals_Saxs_Baseline_Best::cancel()
{
   close();
}

void US_Hydrodyn_Mals_Saxs_Baseline_Best::help()
{
   US_Help *online_help;
   online_help = new US_Help( this );
   online_help->show_help("manual/somo/baseline_best.html");
}

void US_Hydrodyn_Mals_Saxs_Baseline_Best::closeEvent( QCloseEvent *e )
{
   if ( ((US_Hydrodyn *)us_hydrodyn)->mals_saxs_widget ) {
      US_Hydrodyn_Mals_Saxs *mals_saxs_win = ((US_Hydrodyn *)us_hydrodyn)->mals_saxs_window;
      if ( mals_saxs_win->plot_info.count( plot_name ) &&
           mals_saxs_win->plot_info[ plot_name ] == plot ) {
         mals_saxs_win->plot_info.erase( plot_name );
      }
      if ( mals_saxs_win->plot_info.count( hb_plot_name ) &&
           mals_saxs_win->plot_info[ hb_plot_name ] == hb_plot ) {
         mals_saxs_win->plot_info.erase( hb_plot_name );
      }
   }

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Mals_Saxs_Baseline_Best::editor_hb_msg( QString color, QString msg )
{
   return editor_msg( color, msg, hb_editor );
}

void US_Hydrodyn_Mals_Saxs_Baseline_Best::editor_hb_ec_msg( QString msg )
{
   return editor_ec_msg( msg, hb_editor );
}

void US_Hydrodyn_Mals_Saxs_Baseline_Best::editor_msg( QString color, QString msg, QTextEdit *e )
{
   if ( !e ) {
      e = editor;
   }
   QColor save_color = e->textColor();
   e->setTextColor(color);
   e->append(msg);
   e->setTextColor(save_color);
}

void US_Hydrodyn_Mals_Saxs_Baseline_Best::editor_ec_msg( QString msg, QTextEdit *e )
{
   if ( !e ) {
      e = editor;
   }
   QColor save_color = e->textColor();
   QStringList qsl = (msg ).split( "~~" , Qt::SkipEmptyParts );

   QRegExp rx_color( "^_(\\S+)_$" );

   map < QString, QColor > colors;
   colors[ "orange"      ] = QColor( 255, 165, 0 );
   colors[ "darkorange"  ] = QColor( 255, 140, 0 );
   colors[ "darkorange2" ] = QColor( 238, 118, 0 );
   colors[ "darkorange3" ] = QColor( 214, 106, 0 );
   colors[ "darkyellow1" ] = QColor( 204, 204, 0 );
   colors[ "darkyellow2" ] = QColor( 153, 153, 0 );
   colors[ "darkyellow3" ] = QColor( 102, 102, 0 );
   colors[ "darkyellow4" ] = QColor(  51,  51, 0 );

   for ( int i = 0; i < (int) qsl.size(); ++i ) {
      if ( rx_color.indexIn( qsl[ i ] ) != -1 ) {
         QColor qc;
         if ( colors.count( rx_color.cap( 1 ) ) ) {
            qc = colors[ rx_color.cap( 1 ) ];
         } else {
            qc = QColor( rx_color.cap( 1 ) );
         }
         e->setTextColor( qc );
      } else {
         e->append( qsl[ i ] );
      }
   }

   e->setTextColor(save_color);
}

void US_Hydrodyn_Mals_Saxs_Baseline_Best::clear_display()
{
   editor->clear( );
   editor->append("\n\n");
   hb_editor->clear( );
   hb_editor->append("\n\n");
}

void US_Hydrodyn_Mals_Saxs_Baseline_Best::update_font()
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

void US_Hydrodyn_Mals_Saxs_Baseline_Best::save()
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

void US_Hydrodyn_Mals_Saxs_Baseline_Best::displayData() {

   editor      ->clear( );
   plot->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   hb_plot->detachItems( QwtPlotItem::Rtti_PlotCurve ); hb_plot->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   hb_editor   ->clear( );

   plot->setAxisTitle(QwtPlot::yLeft  , us_tr( 
                                           parameters.count( "hb" ) 
                                           ? "Red pair %"
                                           : "Average Red Cluster Size" 
                                            ) );

   hb_plot->setAxisTitle(QwtPlot::yLeft  , us_tr( "Red pair %" ) );

   lbl_title->setText( parameters.count( "title" ) ? parameters[ "title" ] : QString( us_tr( "Baseline avg. I(q) and average red cluster size by start frame" ) ) );
   lbl_hb_title->setText( parameters.count( "hb_title" ) ? parameters[ "hb_title" ] : QString( us_tr( "Baseline avg. I(q) and red pair % by start frame" ) ) );
   // plot data and print anything of importance in editor

   editor->append( "<a name=\"top\">" );
   if ( parameters.count( "msg" ) ) {
      editor_ec_msg( parameters[ "msg" ] );
   }
   editor->scrollToAnchor( "top" );

   hb_editor->append( "<a name=\"top\">" );
   if ( parameters.count( "hb_msg" ) ) {
      editor_hb_ec_msg( parameters[ "hb_msg" ] );
   }
   hb_editor->scrollToAnchor( "top" );

   // plot data

   if ( vdparameters.count( "x" ) &&
        vdparameters.count( "y" ) &&
        vdparameters[ "x" ].size() &&
        vdparameters[ "x" ].size() == vdparameters[ "y" ].size()
        ) {
      int use_line_width = parameters.count( "linewidth" ) ? parameters[ "linewidth" ].toInt() : 1;
      double maxy = 5e-1;

      // US_Vector::printvector2( "baseline results", vdparameters[ "x" ], vdparameters[ "y" ] );

      double minyr = 0e0;
      double maxyr = -1e99;
      bool plotted_yright = false;

      if ( vdparameters.count( "sumqx" ) ) {
         plotted_yright = true;
         plot->enableAxis  ( QwtPlot::yRight , true );
         plot->setAxisTitle( QwtPlot::yRight, us_tr( "\nAverage I(q) [a.u.]" ) );

         if ( vdparameters.count( "sumqy" ) ) {
            for ( int i = 0; i < (int) vdparameters[ "sumqy" ].size(); ++i ) {
               if ( maxyr < vdparameters[ "sumqy" ][ i ] ) {
                  maxyr = vdparameters[ "sumqy" ][ i ];
               }
               if ( minyr > vdparameters[ "sumqy" ][ i ] ) {
                  minyr = vdparameters[ "sumqy" ][ i ];
               }
            }

            {
#if QT_VERSION < 0x040000
               long curve;
               curve = plot->insertCurve( "baseline_best_sumq", QwtPlot::xBottom, QwtPlot::yRight );
               plot->setCurveStyle( curve, QwtCurve::Lines );
#else
               QwtPlotCurve *curve = new QwtPlotCurve( "baseline_best_sumq" );
               curve->setStyle( QwtPlotCurve::Lines );
#endif

#if QT_VERSION < 0x040000
               plot->setCurvePen( curve, QPen( Qt::white, 2 * use_line_width, Qt::SolidLine ) );
               plot->setCurveData( curve,
                                   (double *)&vdparameters[ "sumqx" ][ 0 ],
                                   (double *)&vdparameters[ "sumqy" ][ 0 ],
                                   vdparameters[ "sumqx" ].size()
                                   );
#else
               curve->setPen( QPen( Qt::white, 2 * use_line_width, Qt::SolidLine ) );
               curve->setSamples(
                              (double *)&vdparameters[ "sumqx" ][ 0 ],
                              (double *)&vdparameters[ "sumqy" ][ 0 ],
                              vdparameters[ "sumqx" ].size()
                              );
               curve->setAxes( QwtPlot::xBottom , QwtPlot::yRight );
               curve->attach( plot );
#endif
            }
         }

         if ( vdparameters.count( "sumqmaxqy" ) ) {

            for ( int i = 0; i < (int) vdparameters[ "sumqmaxqy" ].size(); ++i ) {
               if ( maxyr < vdparameters[ "sumqmaxqy" ][ i ] ) {
                  maxyr = vdparameters[ "sumqmaxqy" ][ i ];
               }
               if ( minyr > vdparameters[ "sumqmaxqy" ][ i ] ) {
                  minyr = vdparameters[ "sumqmaxqy" ][ i ];
               }
            }

            QString curvename = QString( us_tr( "Baseline avg. I(q) for q less than %1\n(on right axis)" ) ).arg( dparameters[ "cormap_maxq" ] );
#if QT_VERSION < 0x040000
            long curve;
            curve = plot->insertCurve( curvename, QwtPlot::xBottom, QwtPlot::yRight );
            plot->setCurveStyle( curve, QwtCurve::Lines );
#else
            QwtPlotCurve *curve = new QwtPlotCurve( curvename );
            curve->setStyle( QwtPlotCurve::Lines );
            curve->setItemAttribute( QwtPlotItem::Legend, true );
#endif

#if QT_VERSION < 0x040000
            plot->setCurvePen( curve, QPen( QColor( 255, 165, 0 ) /* orange */, 2 * use_line_width, Qt::SolidLine ) );
            plot->setCurveData( curve,
                                (double *)&vdparameters[ "sumqx" ][ 0 ],
                                (double *)&vdparameters[ "sumqmaxqy" ][ 0 ],
                                vdparameters[ "sumqx" ].size()
                                );
#else
            curve->setPen( QPen( QColor( 255, 165, 0 ) /* orange */, 2 * use_line_width, Qt::SolidLine ) );
            curve->setSamples(
                           (double *)&vdparameters[ "sumqx" ][ 0 ],
                           (double *)&vdparameters[ "sumqmaxqy" ][ 0 ],
                           vdparameters[ "sumqx" ].size()
                           );
            curve->setAxes( QwtPlot::xBottom , QwtPlot::yRight );
            curve->attach( plot );
#endif
            if ( vdparameters.count( "sumqmaxqysdm" ) &&
                 vdparameters.count( "sumqmaxqysdp" ) ) {
               for ( int i = 0; i < (int) vdparameters[ "sumqx" ].size(); ++i ) {
                  if ( maxyr < vdparameters[ "sumqmaxqysdp" ][ i ] ) {
                     maxyr = vdparameters[ "sumqmaxqysdp" ][ i ];
                  }
                  if ( minyr > vdparameters[ "sumqmaxqysdm" ][ i ] ) {
                     minyr = vdparameters[ "sumqmaxqysdm" ][ i ];
                  }
               }

               {
#if QT_VERSION < 0x050000
                  QString curvename = QString( us_tr( "Baseline avg. I(q) ±1 SD for q less than %1\n(on right axis)" ) ).arg( dparameters[ "cormap_maxq" ] );
#else
                  QString curvename = QString( us_tr( "Baseline avg. I(q) \u00b11 SD for q less than %1\n(on right axis)" ) ).arg( dparameters[ "cormap_maxq" ] );
#endif
#if QT_VERSION < 0x040000
                  long curve;
                  curve = plot->insertCurve( curvename, QwtPlot::xBottom, QwtPlot::yRight );
                  plot->setCurveStyle( curve, QwtCurve::Lines );
#else
                  QwtPlotCurve *curve = new QwtPlotCurve( curvename );
                  curve->setStyle( QwtPlotCurve::Lines );
                  curve->setItemAttribute( QwtPlotItem::Legend, true );
#endif
                  
#if QT_VERSION < 0x040000
                  plot->setCurvePen( curve, QPen( QColor( 255, 165, 0 ) /* orange */, 2 * use_line_width, Qt::DotLine ) );
                  plot->setCurveData( curve, 
                                      (double *)&vdparameters[ "sumqx" ][ 0 ],
                                      (double *)&vdparameters[ "sumqmaxqysdm" ][ 0 ],
                                      vdparameters[ "sumqx" ].size()
                                      );
#else
                  curve->setPen( QPen( QColor( 255, 165, 0 ) /* orange */, 2 * use_line_width, Qt::DotLine ) );
                  curve->setSamples(
                                 (double *)&vdparameters[ "sumqx" ][ 0 ],
                                 (double *)&vdparameters[ "sumqmaxqysdm" ][ 0 ],
                                 vdparameters[ "sumqx" ].size()
                                 );
                  curve->setAxes( QwtPlot::xBottom , QwtPlot::yRight );
                  curve->attach( plot );
#endif
               }
               {
#if QT_VERSION < 0x050000
                  QString curvename = QString( us_tr( "Baseline avg. I(q) ±1 SD for q less than %1\n(on right axis)" ) ).arg( dparameters[ "cormap_maxq" ] );
#else
                  QString curvename = QString( us_tr( "Baseline avg. I(q) \u00b11 SD for q less than %1\n(on right axis)" ) ).arg( dparameters[ "cormap_maxq" ] );
#endif
#if QT_VERSION < 0x040000
                  long curve;
                  curve = plot->insertCurve( curvename, QwtPlot::xBottom, QwtPlot::yRight );
                  plot->setCurveStyle( curve, QwtCurve::Lines );
                  plot->enableLegend( false, curve );
#else
                  QwtPlotCurve *curve = new QwtPlotCurve( curvename );
                  curve->setStyle( QwtPlotCurve::Lines );
                  curve->setItemAttribute( QwtPlotItem::Legend, false );
#endif
                  
#if QT_VERSION < 0x040000
                  plot->setCurvePen( curve, QPen( QColor( 255, 165, 0 ) /* orange */, 2 * use_line_width, Qt::DotLine ) );
                  plot->setCurveData( curve, 
                                      (double *)&vdparameters[ "sumqx" ][ 0 ],
                                      (double *)&vdparameters[ "sumqmaxqysdp" ][ 0 ],
                                      vdparameters[ "sumqx" ].size()
                                      );
#else
                  curve->setPen( QPen( QColor( 255, 165, 0 ) /* orange */, 2 * use_line_width, Qt::DotLine ) );
                  curve->setSamples(
                                 (double *)&vdparameters[ "sumqx" ][ 0 ],
                                 (double *)&vdparameters[ "sumqmaxqysdp" ][ 0 ],
                                 vdparameters[ "sumqx" ].size()
                                 );
                  curve->setAxes( QwtPlot::xBottom , QwtPlot::yRight );
                  curve->attach( plot );
#endif
               }
            }
         }

         {
            double x[ 2 ];
            double y[ 2 ];

            x[ 0 ] = vdparameters[ "x" ].front() - 1;
            x[ 1 ] = vdparameters[ "x" ].back() + 1;

            y[ 0 ] = y[ 1 ] = 0;

            QString curvename = "Zero intensity\n(on right axis)";
#if QT_VERSION < 0x040000
            long curve;
            curve = plot->insertCurve( curvename, QwtPlot::xBottom, QwtPlot::yRight );
            plot->setCurveStyle( curve, QwtCurve::Lines );
#else
            QwtPlotCurve *curve = new QwtPlotCurve( curvename );
            curve->setStyle( QwtPlotCurve::Lines );
            curve->setItemAttribute( QwtPlotItem::Legend, true );
#endif

#if QT_VERSION < 0x040000
            plot->setCurvePen( curve, QPen( Qt::magenta, 2 * use_line_width, Qt::SolidLine ) );
            plot->setCurveData( curve, x, y, 2 );
#else
            curve->setPen( QPen( Qt::magenta, 2 * use_line_width, Qt::SolidLine ) );
            curve->setSamples( x, y, 2 );
            curve->setAxes( QwtPlot::xBottom , QwtPlot::yRight );
            curve->attach( plot );
#endif
         }

         if ( false && dparameters.count( "blanks_avg_maxq_sd" ) ) {
            double x[ 2 ];
            double y[ 2 ];

            x[ 0 ] = vdparameters[ "x" ].front() - 1;
            x[ 1 ] = vdparameters[ "x" ].back() + 1;

            y[ 0 ] = y[ 1 ] = dparameters[ "blanks_avg_maxq_sd" ];
            if ( maxyr < y[ 0 ] ) {
               maxyr = y[ 0 ];
            }

            {
#if QT_VERSION < 0x050000
               QString curvename = "Average ±1 SD of total blanks intensity\n(on right axis)";
#else
               QString curvename = "Average \u00b11 SD of total blanks intensity\n(on right axis)";
#endif

#if QT_VERSION < 0x040000
               long curve;
               curve = plot->insertCurve( curvename, QwtPlot::xBottom, QwtPlot::yRight );
               plot->setCurveStyle( curve, QwtCurve::Lines );
#else
               QwtPlotCurve *curve = new QwtPlotCurve( curvename );
               curve->setStyle( QwtPlotCurve::Lines );
               curve->setItemAttribute( QwtPlotItem::Legend, true );
#endif

#if QT_VERSION < 0x040000
               plot->setCurvePen( curve, QPen( Qt::magenta, 2 * use_line_width, Qt::DotLine ) );
               plot->setCurveData( curve, x, y, 2 );
#else
               curve->setPen( QPen( Qt::magenta, 2 * use_line_width, Qt::DotLine ) );
               curve->setSamples( x, y, 2 );
               curve->setAxes( QwtPlot::xBottom , QwtPlot::yRight );
               curve->attach( plot );
#endif
            }
            {
               QString curvename = "Average -1 SD of total blanks intensity\n(on right axis)";
               y[ 0 ] = y[ 1 ] = -dparameters[ "blanks_avg_maxq_sd" ];
#if QT_VERSION < 0x040000
               long curve;
               curve = plot->insertCurve( curvename, QwtPlot::xBottom, QwtPlot::yRight );
               plot->setCurveStyle( curve, QwtCurve::Lines );
               plot->enableLegend( false, curve );
#else
               QwtPlotCurve *curve = new QwtPlotCurve( curvename );
               curve->setStyle( QwtPlotCurve::Lines );
               curve->setItemAttribute( QwtPlotItem::Legend, false );
#endif

#if QT_VERSION < 0x040000
               plot->setCurvePen( curve, QPen( Qt::magenta, 2 * use_line_width, Qt::DotLine ) );
               plot->setCurveData( curve, x, y, 2 );
#else
               curve->setPen( QPen( Qt::magenta, 2 * use_line_width, Qt::DotLine ) );
               curve->setSamples( x, y, 2 );
               curve->setAxes( QwtPlot::xBottom , QwtPlot::yRight );
               curve->attach( plot );
#endif
            }
         }
      }         

      {
         for ( int i = 0; i < (int) vdparameters[ "y" ].size(); ++i ) {
            if ( maxy < vdparameters[ "y" ][ i ] ) {
               maxy = vdparameters[ "y" ][ i ];
            }
         }

         QString curvename = parameters.count( "hb" )
            ? "Baseline red pair % greater than minimum value"
            : "Baseline avg. red cluster size\nabove blanks' avg. +1 SD"
            ;

#if QT_VERSION < 0x040000
         long curve;
         curve = plot->insertCurve( curvename );
         plot->setCurveStyle( curve, QwtCurve::Sticks );
#else
         QwtPlotCurve *curve = new QwtPlotCurve( curvename );
         curve->setStyle( QwtPlotCurve::Sticks );
         curve->setItemAttribute( QwtPlotItem::Legend, true );
#endif

#if QT_VERSION < 0x040000
         plot->setCurvePen( curve, QPen( Qt::red, 2 * use_line_width, Qt::SolidLine ) );
         plot->setCurveData( curve,
                             (double *)&vdparameters[ "x" ][ 0 ],
                             (double *)&vdparameters[ "y" ][ 0 ],
                             vdparameters[ "x" ].size()
                             );
         plot->setCurveBaseline( curve, -0.5 );
#else
         curve->setPen( QPen( Qt::red, 2 * use_line_width, Qt::SolidLine ) );
         curve->setSamples(
                        (double *)&vdparameters[ "x" ][ 0 ],
                        (double *)&vdparameters[ "y" ][ 0 ],
                        vdparameters[ "x" ].size()

                        );
         curve->setBaseline( -0.5 );
         curve->attach( plot );
#endif
      }

      if ( vdparameters.count( "yx" ) ) {
         for ( int i = 0; i < (int) vdparameters[ "yy" ].size(); ++i ) {
            if ( maxy < vdparameters[ "yy" ][ i ] ) {
               maxy = vdparameters[ "yy" ][ i ];
            }
         }

         QString curvename = parameters.count( "hb" )
            ? "Baseline best red pair % above 0\n"
            : "Baseline best avg. red cluster size\nabove blanks' avg. +1 SD"
            ;

#if QT_VERSION < 0x040000
         long curve;
         curve = plot->insertCurve( curvename );
         plot->setCurveStyle( curve, QwtCurve::Sticks );
#else
         QwtPlotCurve *curve = new QwtPlotCurve( curvename );
         curve->setStyle( QwtPlotCurve::Sticks );
         curve->setItemAttribute( QwtPlotItem::Legend, true );
#endif

#if QT_VERSION < 0x040000
         plot->setCurvePen( curve, QPen( Qt::yellow, 2 * use_line_width, Qt::SolidLine ) );
         plot->setCurveData( curve,
                             (double *)&vdparameters[ "yx" ][ 0 ],
                             (double *)&vdparameters[ "yy" ][ 0 ],
                             vdparameters[ "yx" ].size()
                             );
         plot->setCurveBaseline( curve, -0.5 );
#else
         curve->setPen( QPen( Qt::yellow, 2 * use_line_width, Qt::SolidLine ) );
         curve->setSamples(
                        (double *)&vdparameters[ "yx" ][ 0 ],
                        (double *)&vdparameters[ "yy" ][ 0 ],
                        vdparameters[ "yx" ].size()

                        );
         curve->setBaseline( -0.5 );
         curve->attach( plot );
#endif
      }

      if ( vdparameters.count( "gx" ) ) {
         for ( int i = 0; i < (int) vdparameters[ "gy" ].size(); ++i ) {
            if ( maxy < vdparameters[ "gy" ][ i ] ) {
               maxy = vdparameters[ "gy" ][ i ];
            }
         }

         QString curvename = "Baseline avg. red cluster size\nequal or below blanks' avg. +1 SD";

#if QT_VERSION < 0x040000
         long curve;
         curve = plot->insertCurve( curvename );
         plot->setCurveStyle( curve, QwtCurve::Sticks );
#else
         QwtPlotCurve *curve = new QwtPlotCurve( curvename );
         curve->setStyle( QwtPlotCurve::Sticks );
         curve->setItemAttribute( QwtPlotItem::Legend, true );
#endif

#if QT_VERSION < 0x040000
         plot->setCurvePen( curve, QPen( Qt::cyan, 2 * use_line_width, Qt::SolidLine ) );
         plot->setCurveData( curve,
                             (double *)&vdparameters[ "gx" ][ 0 ],
                             (double *)&vdparameters[ "gy" ][ 0 ],
                             vdparameters[ "gx" ].size()
                             );
         plot->setCurveBaseline( curve, -0.5 );
#else
         curve->setPen( QPen( Qt::cyan, 2 * use_line_width, Qt::SolidLine ) );
         curve->setSamples(
                        (double *)&vdparameters[ "gx" ][ 0 ],
                        (double *)&vdparameters[ "gy" ][ 0 ],
                        vdparameters[ "gx" ].size()

                        );
         curve->setBaseline( -0.5 );
         curve->attach( plot );
#endif
      }

      if ( vdparameters.count( "wx" ) ) {
         for ( int i = 0; i < (int) vdparameters[ "wy" ].size(); ++i ) {
            if ( maxy < vdparameters[ "wy" ][ i ] ) {
               maxy = vdparameters[ "wy" ][ i ];
            }
         }

         QString curvename = parameters.count( "hb" )
            ? "Baseline best red pair %\n"
            : "Baseline best avg. red cluster size\nequal or below blanks' avg. +1 SD"
            ;
#if QT_VERSION < 0x040000
         long curve;
         curve = plot->insertCurve( curvename );
         plot->setCurveStyle( curve, QwtCurve::Sticks );
#else
         QwtPlotCurve *curve = new QwtPlotCurve( curvename );
         curve->setStyle( QwtPlotCurve::Sticks );
         curve->setItemAttribute( QwtPlotItem::Legend, true );
#endif

#if QT_VERSION < 0x040000
         plot->setCurvePen( curve, QPen( Qt::white, 2 * use_line_width, Qt::SolidLine ) );
         plot->setCurveData( curve,
                             (double *)&vdparameters[ "wx" ][ 0 ],
                             (double *)&vdparameters[ "wy" ][ 0 ],
                             vdparameters[ "wx" ].size()
                             );
         plot->setCurveBaseline( curve, -0.5 );
#else
         curve->setPen( QPen( Qt::white, 2 * use_line_width, Qt::SolidLine ) );
         curve->setSamples(
                        (double *)&vdparameters[ "wx" ][ 0 ],
                        (double *)&vdparameters[ "wy" ][ 0 ],
                        vdparameters[ "wx" ].size()

                        );
         curve->setBaseline( -0.5 );
         curve->attach( plot );
#endif
      }

      if ( dparameters.count( "blanksaverage" ) ) {
         double x[ 2 ];
         double y[ 2 ];

         x[ 0 ] = vdparameters[ "x" ].front() - 1;
         x[ 1 ] = vdparameters[ "x" ].back() + 1;

         y[ 0 ] = y[ 1 ] = dparameters[ "blanksaverage" ];

         QString curvename = us_tr( "Blanks' avg. red cluster size" );

#if QT_VERSION < 0x040000
         long curve;
         curve = plot->insertCurve( curvename );
         plot->setCurveStyle( curve, QwtCurve::Lines );
#else
         QwtPlotCurve *curve = new QwtPlotCurve( curvename );
         curve->setStyle( QwtPlotCurve::Lines );
         curve->setItemAttribute( QwtPlotItem::Legend, true );
#endif

#if QT_VERSION < 0x040000
         plot->setCurvePen( curve, QPen( Qt::green, 2 * use_line_width, Qt::SolidLine ) );
         plot->setCurveData( curve, x, y, 2 );
#else
         curve->setPen( QPen( Qt::green, 2 * use_line_width, Qt::SolidLine ) );
         curve->setSamples( x, y, 2 );
         curve->attach( plot );
#endif

         if ( dparameters.count( "blanksaveragesd" ) ) {

            y[ 0 ] = y[ 1 ] = y[ 0 ] + dparameters[ "blanksaveragesd" ];

            QString curvename = us_tr( "Blanks' avg. red cluster size +1 SD" );
#if QT_VERSION < 0x040000
            long curve;
            curve = plot->insertCurve( curvename );
            plot->setCurveStyle( curve, QwtCurve::Lines );
#else
            QwtPlotCurve *curve = new QwtPlotCurve( curvename );
            curve->setStyle( QwtPlotCurve::Lines );
            curve->setItemAttribute( QwtPlotItem::Legend, true );
#endif

#if QT_VERSION < 0x040000
            plot->setCurvePen( curve, QPen( Qt::green, 2 * use_line_width, Qt::DotLine ) );
            plot->setCurveData( curve, x, y, 2 );
#else
            curve->setPen( QPen( Qt::green, 2 * use_line_width, Qt::DotLine ) );
            curve->setSamples( x, y, 2 );
            curve->attach( plot );
#endif
         }
      }

      if ( !plot_zoomer )
      {
         plot->setAxisScale( QwtPlot::xBottom, vdparameters[ "x" ].front() - 1, vdparameters[ "x" ].back() + 1 );
         plot->setAxisScale( QwtPlot::yLeft  , -0.25, maxy * 1.1 );
         if ( plotted_yright ) {
            // plot->setAxisScale( QwtPlot::yRight, minyr - ( fabs( minyr * .1 ) ), maxyr * 1.1 );
            plot->setAxisScale( QwtPlot::yRight, -3e0 * (maxyr - minyr ), maxyr * 1.1 );
            plot->setAxisScale( QwtPlot::yLeft  , -0.25, 1.7 *  maxy );
         }
            
         plot_zoomer = new ScrollZoomer( plot->canvas());
         plot_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
         plot_zoomer->setTrackerPen(QPen(Qt::red));
         // connect( plot_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_zoomed( const QRectF & ) ) );
      }
      
   }
   // hb only size

   if ( !parameters.count( "hb" ) ) {
      if ( vdparameters.count( "hb_x" ) &&
           vdparameters.count( "hb_y" ) &&
           vdparameters[ "hb_x" ].size() &&
           vdparameters[ "hb_x" ].size() == vdparameters[ "hb_y" ].size()
           ) {
         int use_line_width = parameters.count( "linewidth" ) ? parameters[ "linewidth" ].toInt() : 1;
         double maxy = 5e-1;

         // US_Vector::printvector2( "baseline results", vdparameters[ "x" ], vdparameters[ "y" ] );

         double minyr = 0e0;
         double maxyr = -1e99;
         bool plotted_yright = false;

         if ( vdparameters.count( "sumqx" ) ) {
            plotted_yright = true;
            hb_plot->enableAxis  ( QwtPlot::yRight , true );
            hb_plot->setAxisTitle( QwtPlot::yRight, us_tr( "\nAverage I(q) [a.u.]" ) );

            if ( vdparameters.count( "sumqy" ) ) {
               for ( int i = 0; i < (int) vdparameters[ "sumqy" ].size(); ++i ) {
                  if ( maxyr < vdparameters[ "sumqy" ][ i ] ) {
                     maxyr = vdparameters[ "sumqy" ][ i ];
                  }
                  if ( minyr > vdparameters[ "sumqy" ][ i ] ) {
                     minyr = vdparameters[ "sumqy" ][ i ];
                  }
               }

               {
#if QT_VERSION < 0x040000
                  long curve;
                  curve = hb_plot->insertCurve( "baseline_best_sumq", QwtPlot::xBottom, QwtPlot::yRight );
                  hb_plot->setCurveStyle( curve, QwtCurve::Lines );
#else
                  QwtPlotCurve *curve = new QwtPlotCurve( "baseline_best_sumq" );
                  curve->setStyle( QwtPlotCurve::Lines );
#endif

#if QT_VERSION < 0x040000
                  hb_plot->setCurvePen( curve, QPen( Qt::white, 2 * use_line_width, Qt::SolidLine ) );
                  hb_plot->setCurveData( curve,
                                      (double *)&vdparameters[ "sumqx" ][ 0 ],
                                      (double *)&vdparameters[ "sumqy" ][ 0 ],
                                      vdparameters[ "sumqx" ].size()
                                      );
#else
                  curve->setPen( QPen( Qt::white, 2 * use_line_width, Qt::SolidLine ) );
                  curve->setSamples(
                                 (double *)&vdparameters[ "sumqx" ][ 0 ],
                                 (double *)&vdparameters[ "sumqy" ][ 0 ],
                                 vdparameters[ "sumqx" ].size()
                                 );
                  curve->setAxes( QwtPlot::xBottom , QwtPlot::yRight );
                  curve->attach( hb_plot );
#endif
               }
            }

            if ( vdparameters.count( "sumqmaxqy" ) ) {

               for ( int i = 0; i < (int) vdparameters[ "sumqmaxqy" ].size(); ++i ) {
                  if ( maxyr < vdparameters[ "sumqmaxqy" ][ i ] ) {
                     maxyr = vdparameters[ "sumqmaxqy" ][ i ];
                  }
                  if ( minyr > vdparameters[ "sumqmaxqy" ][ i ] ) {
                     minyr = vdparameters[ "sumqmaxqy" ][ i ];
                  }
               }

               QString curvename = QString( us_tr( "Baseline avg. I(q) for q less than %1\n(on right axis)" ) ).arg( dparameters[ "cormap_maxq" ] );
#if QT_VERSION < 0x040000
               long curve;
               curve = hb_plot->insertCurve( curvename, QwtPlot::xBottom, QwtPlot::yRight );
               hb_plot->setCurveStyle( curve, QwtCurve::Lines );
#else
               QwtPlotCurve *curve = new QwtPlotCurve( curvename );
               curve->setStyle( QwtPlotCurve::Lines );
               curve->setItemAttribute( QwtPlotItem::Legend, true );
#endif

#if QT_VERSION < 0x040000
               hb_plot->setCurvePen( curve, QPen( QColor( 255, 165, 0 ) /* orange */, 2 * use_line_width, Qt::SolidLine ) );
               hb_plot->setCurveData( curve,
                                   (double *)&vdparameters[ "sumqx" ][ 0 ],
                                   (double *)&vdparameters[ "sumqmaxqy" ][ 0 ],
                                   vdparameters[ "sumqx" ].size()
                                   );
#else
               curve->setPen( QPen( QColor( 255, 165, 0 ) /* orange */, 2 * use_line_width, Qt::SolidLine ) );
               curve->setSamples(
                              (double *)&vdparameters[ "sumqx" ][ 0 ],
                              (double *)&vdparameters[ "sumqmaxqy" ][ 0 ],
                              vdparameters[ "sumqx" ].size()
                              );
               curve->setAxes( QwtPlot::xBottom , QwtPlot::yRight );
               curve->attach( hb_plot );
#endif
               if ( vdparameters.count( "sumqmaxqysdm" ) &&
                    vdparameters.count( "sumqmaxqysdp" ) ) {
                  for ( int i = 0; i < (int) vdparameters[ "sumqx" ].size(); ++i ) {
                     if ( maxyr < vdparameters[ "sumqmaxqysdp" ][ i ] ) {
                        maxyr = vdparameters[ "sumqmaxqysdp" ][ i ];
                     }
                     if ( minyr > vdparameters[ "sumqmaxqysdm" ][ i ] ) {
                        minyr = vdparameters[ "sumqmaxqysdm" ][ i ];
                     }
                  }

                  {
#if QT_VERSION < 0x050000
                     QString curvename = QString( us_tr( "Baseline avg. I(q) ±1 SD for q less than %1\n(on right axis)" ) ).arg( dparameters[ "cormap_maxq" ] );
#else
                     QString curvename = QString( us_tr( "Baseline avg. I(q) \u00b11 SD for q less than %1\n(on right axis)" ) ).arg( dparameters[ "cormap_maxq" ] );
#endif

#if QT_VERSION < 0x040000
                     long curve;
                     curve = hb_plot->insertCurve( curvename, QwtPlot::xBottom, QwtPlot::yRight );
                     hb_plot->setCurveStyle( curve, QwtCurve::Lines );
#else
                     QwtPlotCurve *curve = new QwtPlotCurve( curvename );
                     curve->setStyle( QwtPlotCurve::Lines );
                     curve->setItemAttribute( QwtPlotItem::Legend, true );
#endif
                  
#if QT_VERSION < 0x040000
                     hb_plot->setCurvePen( curve, QPen( QColor( 255, 165, 0 ) /* orange */, 2 * use_line_width, Qt::DotLine ) );
                     hb_plot->setCurveData( curve, 
                                         (double *)&vdparameters[ "sumqx" ][ 0 ],
                                         (double *)&vdparameters[ "sumqmaxqysdm" ][ 0 ],
                                         vdparameters[ "sumqx" ].size()
                                         );
#else
                     curve->setPen( QPen( QColor( 255, 165, 0 ) /* orange */, 2 * use_line_width, Qt::DotLine ) );
                     curve->setSamples(
                                    (double *)&vdparameters[ "sumqx" ][ 0 ],
                                    (double *)&vdparameters[ "sumqmaxqysdm" ][ 0 ],
                                    vdparameters[ "sumqx" ].size()
                                    );
                     curve->setAxes( QwtPlot::xBottom , QwtPlot::yRight );
                     curve->attach( hb_plot );
#endif
                  }
                  {
#if QT_VERSION < 0x050000
                     QString curvename = QString( us_tr( "Baseline avg. I(q) ±1 SD for q less than %1\n(on right axis)" ) ).arg( dparameters[ "cormap_maxq" ] );
#else
                     QString curvename = QString( us_tr( "Baseline avg. I(q) \u00b11 SD for q less than %1\n(on right axis)" ) ).arg( dparameters[ "cormap_maxq" ] );
#endif

#if QT_VERSION < 0x040000
                     long curve;
                     curve = hb_plot->insertCurve( curvename, QwtPlot::xBottom, QwtPlot::yRight );
                     hb_plot->setCurveStyle( curve, QwtCurve::Lines );
                     hb_plot->enableLegend( false, curve );
#else
                     QwtPlotCurve *curve = new QwtPlotCurve( curvename );
                     curve->setStyle( QwtPlotCurve::Lines );
                     curve->setItemAttribute( QwtPlotItem::Legend, false );
#endif
                  
#if QT_VERSION < 0x040000
                     hb_plot->setCurvePen( curve, QPen( QColor( 255, 165, 0 ) /* orange */, 2 * use_line_width, Qt::DotLine ) );
                     hb_plot->setCurveData( curve, 
                                         (double *)&vdparameters[ "sumqx" ][ 0 ],
                                         (double *)&vdparameters[ "sumqmaxqysdp" ][ 0 ],
                                         vdparameters[ "sumqx" ].size()
                                         );
#else
                     curve->setPen( QPen( QColor( 255, 165, 0 ) /* orange */, 2 * use_line_width, Qt::DotLine ) );
                     curve->setSamples(
                                    (double *)&vdparameters[ "sumqx" ][ 0 ],
                                    (double *)&vdparameters[ "sumqmaxqysdp" ][ 0 ],
                                    vdparameters[ "sumqx" ].size()
                                    );
                     curve->setAxes( QwtPlot::xBottom , QwtPlot::yRight );
                     curve->attach( hb_plot );
#endif
                  }
               }
            }

            {
               double x[ 2 ];
               double y[ 2 ];

               x[ 0 ] = vdparameters[ "hb_x" ].front() - 1;
               x[ 1 ] = vdparameters[ "hb_x" ].back() + 1;

               y[ 0 ] = y[ 1 ] = 0;

               QString curvename = "Zero intensity\n(on right axis)";
#if QT_VERSION < 0x040000
               long curve;
               curve = hb_plot->insertCurve( curvename, QwtPlot::xBottom, QwtPlot::yRight );
               hb_plot->setCurveStyle( curve, QwtCurve::Lines );
#else
               QwtPlotCurve *curve = new QwtPlotCurve( curvename );
               curve->setStyle( QwtPlotCurve::Lines );
               curve->setItemAttribute( QwtPlotItem::Legend, true );
#endif

#if QT_VERSION < 0x040000
               hb_plot->setCurvePen( curve, QPen( Qt::magenta, 2 * use_line_width, Qt::SolidLine ) );
               hb_plot->setCurveData( curve, x, y, 2 );
#else
               curve->setPen( QPen( Qt::magenta, 2 * use_line_width, Qt::SolidLine ) );
               curve->setSamples( x, y, 2 );
               curve->setAxes( QwtPlot::xBottom , QwtPlot::yRight );
               curve->attach( hb_plot );
#endif
            }
         }         

         {
            for ( int i = 0; i < (int) vdparameters[ "hb_y" ].size(); ++i ) {
               if ( maxy < vdparameters[ "hb_y" ][ i ] ) {
                  maxy = vdparameters[ "hb_y" ][ i ];
               }
            }

            QString curvename = "Baseline red pair % greater than minimum value";

#if QT_VERSION < 0x040000
            long curve;
            curve = hb_plot->insertCurve( curvename );
            hb_plot->setCurveStyle( curve, QwtCurve::Sticks );
#else
            QwtPlotCurve *curve = new QwtPlotCurve( curvename );
            curve->setStyle( QwtPlotCurve::Sticks );
            curve->setItemAttribute( QwtPlotItem::Legend, true );
#endif

#if QT_VERSION < 0x040000
            hb_plot->setCurvePen( curve, QPen( Qt::red, 2 * use_line_width, Qt::SolidLine ) );
            hb_plot->setCurveData( curve,
                                (double *)&vdparameters[ "hb_x" ][ 0 ],
                                (double *)&vdparameters[ "hb_y" ][ 0 ],
                                vdparameters[ "hb_x" ].size()
                                );
            hb_plot->setCurveBaseline( curve, -0.5 );
#else
            curve->setPen( QPen( Qt::red, 2 * use_line_width, Qt::SolidLine ) );
            curve->setSamples(
                           (double *)&vdparameters[ "hb_x" ][ 0 ],
                           (double *)&vdparameters[ "hb_y" ][ 0 ],
                           vdparameters[ "hb_x" ].size()

                           );
            curve->setBaseline( -0.5 );
            curve->attach( hb_plot );
#endif
         }

         if ( vdparameters.count( "hb_wx" ) ) {
            for ( int i = 0; i < (int) vdparameters[ "hb_wy" ].size(); ++i ) {
               if ( maxy < vdparameters[ "hb_wy" ][ i ] ) {
                  maxy = vdparameters[ "hb_wy" ][ i ];
               }
            }

            QString curvename = "Baseline best red pair %";
#if QT_VERSION < 0x040000
            long curve;
            curve = hb_plot->insertCurve( curvename );
            hb_plot->setCurveStyle( curve, QwtCurve::Sticks );
#else
            QwtPlotCurve *curve = new QwtPlotCurve( curvename );
            curve->setStyle( QwtPlotCurve::Sticks );
            curve->setItemAttribute( QwtPlotItem::Legend, true );
#endif

#if QT_VERSION < 0x040000
            hb_plot->setCurvePen( curve, QPen( Qt::white, 2 * use_line_width, Qt::SolidLine ) );
            hb_plot->setCurveData( curve,
                                (double *)&vdparameters[ "hb_wx" ][ 0 ],
                                (double *)&vdparameters[ "hb_wy" ][ 0 ],
                                vdparameters[ "hb_wx" ].size()
                                );
            hb_plot->setCurveBaseline( curve, -0.5 );
#else
            curve->setPen( QPen( Qt::white, 2 * use_line_width, Qt::SolidLine ) );
            curve->setSamples(
                           (double *)&vdparameters[ "hb_wx" ][ 0 ],
                           (double *)&vdparameters[ "hb_wy" ][ 0 ],
                           vdparameters[ "hb_wx" ].size()

                           );
            curve->setBaseline( -0.5 );
            curve->attach( hb_plot );
#endif
         }

         if ( !hb_plot_zoomer )
         {
            hb_plot->setAxisScale( QwtPlot::xBottom, vdparameters[ "hb_x" ].front() - 1, vdparameters[ "hb_x" ].back() + 1 );
            hb_plot->setAxisScale( QwtPlot::yLeft  , -0.25, maxy * 1.1 );
            if ( plotted_yright ) {
               // hb_plot->setAxisScale( QwtPlot::yRight, minyr - ( fabs( minyr * .1 ) ), maxyr * 1.1 );
               hb_plot->setAxisScale( QwtPlot::yRight, -3e0 * (maxyr - minyr ), maxyr * 1.1 );
               hb_plot->setAxisScale( QwtPlot::yLeft  , -0.25, 1.7 *  maxy );
            }
            
            hb_plot_zoomer = new ScrollZoomer( hb_plot->canvas());
            hb_plot_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
            hb_plot_zoomer->setTrackerPen(QPen(Qt::red));
            // connect( hb_plot_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( hb_plot_zoomed( const QRectF & ) ) );
         }
      }
   }

#if QT_VERSION >= 0x040000
   {
       QwtLegend* legend_pd = new QwtLegend;
       // QPalette mp = PALET_NORMAL;
       // mp.setColor( QPalette::Normal, QPalette::Window, Qt::black );
       // this doesn't seem to work: mp.setColor( QPalette::Normal, QPalette::WindowText, Qt::white );
       // legend_pd->contentsWidget()->setPalette( mp );
       // AUTFBACK( legend_pd );
       legend_pd->setFrameStyle( QFrame::Box | QFrame::Sunken );
       plot->insertLegend( legend_pd, QwtPlot::RightLegend );
   }

   {
       QwtLegend* legend_pd = new QwtLegend;
       // QPalette mp = PALET_NORMAL;
       // mp.setColor( QPalette::Normal, QPalette::Window, Qt::black );
       // this doesn't seem to work: mp.setColor( QPalette::Normal, QPalette::WindowText, Qt::white );
       // legend_pd->contentsWidget()->setPalette( mp );
       // AUTFBACK( legend_pd );
       legend_pd->setFrameStyle( QFrame::Box | QFrame::Sunken );
       hb_plot->insertLegend( legend_pd, QwtPlot::RightLegend );
   }

   plot->legend()->setVisible( true );
   hb_plot->legend()->setVisible( true );
#endif

   plot->replot();
   hb_plot->replot();

   if ( !vdparameters.count( "bestpos" ) ||
        !dparameters.count( "width" ) ) {
      us_qdebug( "no bestpos or width" );
      pb_set_best->hide();
   }

   QRect geom = geometry();

   if ( parameters.count( "global_width" ) ) {
      geom.setWidth( parameters[ "global_width" ].toInt() );
      // plot->setMaximumWidth( ( parameters[ "global_width" ].toInt() - 20 ) / 4 );
      // plot->setMinimumWidth( ( parameters[ "global_width" ].toInt() - 20 ) / 4 );
   }
   if ( parameters.count( "global_height" ) ) {
      geom.setHeight( parameters[ "global_height" ].toInt() );
   }

   setGeometry( geom );

   QRect editor_geom = editor->geometry();
   if ( parameters.count( "editor_height" ) ) {
      editor_geom.setHeight( parameters[ "editor_height" ].toInt() );
   }
   editor->setGeometry( editor_geom );

   if ( parameters.count( "save_png" ) ) {
      qApp->processEvents();
      // this->repaint();
      QPixmap qm = this->grab();
      qm.save( parameters[ "save_png" ], "PNG" );
   }
   if ( parameters.count( "close" ) ) {
      qApp->processEvents();
      this->close();
   }
}

void US_Hydrodyn_Mals_Saxs_Baseline_Best::set_best() {
   if ( ((US_Hydrodyn *)us_hydrodyn)->mals_saxs_widget ) {
      US_Hydrodyn_Mals_Saxs *mals_saxs_win = ((US_Hydrodyn *)us_hydrodyn)->mals_saxs_window;

      if ( mals_saxs_win->current_mode == US_Hydrodyn_Mals_Saxs::MODE_BASELINE &&
           vdparameters.count( "bestpos" ) &&
           dparameters.count( "width" ) ) {
         double bestpos = vdparameters[ "bestpos" ].front();
         if ( vdparameters[ "bestpos" ].size() > 1 ) {
            QStringList qsl;
            for ( int i = (int) vdparameters[ "bestpos" ].size() - 1; i >= 0; --i ) {
               qsl << QString( "%1" ).arg( vdparameters[ "bestpos" ][ i ] );
            }
            {
               bool ok;
               QString choice = US_Static::getItem(
                                                      windowTitle() + us_tr( " : Choose starting position for baseline" )
                                                      ,us_tr( "There are multiple equivalent starting positions for the integral baseline window\n"
                                                           "Please choose one from this list" )
                                                      ,qsl
                                                      ,0
                                                      ,false
                                                      ,&ok
                                                      ,this
                                                      );
               if ( !ok ) {
                  return;
               }
               bestpos = choice.toDouble();
            }
         }

         mals_saxs_win->le_baseline_end_s->setText( QString( "%1" ).arg( bestpos ) );
         mals_saxs_win->le_baseline_end_e->setText( QString( "%1" ).arg( bestpos + dparameters[ "width" ] - 1 ) );
         mals_saxs_win->le_baseline_width->setText( QString( "%1" ).arg( dparameters[ "width" ] ) );
         mals_saxs_win->cb_baseline_fix_width->setChecked( true );
      }
   }
}

void US_Hydrodyn_Mals_Saxs_Baseline_Best::set_hb_best() {
   if ( ((US_Hydrodyn *)us_hydrodyn)->mals_saxs_widget ) {
      US_Hydrodyn_Mals_Saxs *mals_saxs_win = ((US_Hydrodyn *)us_hydrodyn)->mals_saxs_window;

      if ( mals_saxs_win->current_mode == US_Hydrodyn_Mals_Saxs::MODE_BASELINE &&
           vdparameters.count( "hb_bestpos" ) &&
           dparameters.count( "width" ) ) {
         double bestpos = vdparameters[ "hb_bestpos" ].front();
         if ( vdparameters[ "hb_bestpos" ].size() > 1 ) {
            QStringList qsl;
            for ( int i = (int) vdparameters[ "hb_bestpos" ].size() - 1; i >= 0; --i ) {
               qsl << QString( "%1" ).arg( vdparameters[ "hb_bestpos" ][ i ] );
            }
            {
               bool ok;
               QString choice = US_Static::getItem(
                                                      windowTitle() + us_tr( " : Choose starting position for baseline" )
                                                      ,us_tr( "There are multiple equivalent starting positions for the integral baseline window\n"
                                                           "Please choose one from this list" )
                                                      ,qsl
                                                      ,0
                                                      ,false
                                                      ,&ok
                                                      ,this
                                                      );
               if ( !ok ) {
                  return;
               }
               bestpos = choice.toDouble();
            }
         }

         mals_saxs_win->le_baseline_end_s->setText( QString( "%1" ).arg( bestpos ) );
         mals_saxs_win->le_baseline_end_e->setText( QString( "%1" ).arg( bestpos + dparameters[ "width" ] - 1 ) );
         mals_saxs_win->le_baseline_width->setText( QString( "%1" ).arg( dparameters[ "width" ] ) );
         mals_saxs_win->cb_baseline_fix_width->setChecked( true );
      }
   }
}

void US_Hydrodyn_Mals_Saxs_Baseline_Best::update_enables() {
   if ( !parameters.count( "hb" ) ) {
      if ( !cb_show_hb->isChecked() ) {
         lbl_hb_title  ->hide();
         pb_set_hb_best->hide();
         hb_plot       ->hide();
         hb_editor     ->hide();
         qs_right      ->hide();
      } else {
         lbl_hb_title  ->show();
         pb_set_hb_best->show();
         hb_plot       ->show();
         hb_editor     ->show();
         qs_right      ->show();
      }
   }
}
         

void US_Hydrodyn_Mals_Saxs_Baseline_Best::usp_config_plot( const QPoint & ) {
   US_PlotChoices *uspc = new US_PlotChoices( usp_plot );
   uspc->exec();
   delete uspc;
}

void US_Hydrodyn_Mals_Saxs_Baseline_Best::usp_config_hb_plot( const QPoint & ) {
   US_PlotChoices *uspc = new US_PlotChoices( usp_hb_plot );
   uspc->exec();
   delete uspc;
}
