#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_saxs_util.h"
#include "../include/us_hydrodyn_saxs_cormap.h"
#include "../include/us_csv.h"
#include <qsplitter.h>
//#include <q3grid.h>
//Added by qt3to4:
#include <QTextStream>
#include <QHBoxLayout>
#include <QLabel>
#include <QGridLayout>
#include <QPixmap>
#include <QFrame>
#include <QEventLoop>
 //#include <Q3PopupMenu>
#include <QVBoxLayout>
#include <QCloseEvent>

US_Hydrodyn_Saxs_Cormap::US_Hydrodyn_Saxs_Cormap(
                                                 void                         *          us_hydrodyn,
                                                 map < QString, QString >                parameters,
                                                 vector < vector < double > >            pvaluepairs,
                                                 vector < vector < double > >            adjpvaluepairs,
                                                 vector < QString >                      selected_files,
                                                 QWidget *                               p,
                                                 const char *                            
                                                 ) : QFrame(  p )
{
   this->us_hydrodyn                          = us_hydrodyn;
   this->parameters                           = parameters;
   this->pvaluepairs                          = pvaluepairs;
   this->adjpvaluepairs                       = adjpvaluepairs;
   this->selected_files                       = selected_files;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle( us_tr( "US-SOMO: PVP Analysis" ) );

   plot_zoomer         = ( ScrollZoomer * )0;
   plot_cluster_zoomer = ( ScrollZoomer * )0;

   last_width  = 0;
   last_height = 0;
   last_mode   = -1;

   setupGUI();
   ((US_Hydrodyn*)us_hydrodyn)->fixWinButtons( this );

   global_Xpos += 30;
   global_Ypos += 30;

   // global_Xpos = 0;
   // global_Ypos = 0;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );
   displayData();
   imageResized();
}

US_Hydrodyn_Saxs_Cormap::~US_Hydrodyn_Saxs_Cormap()
{
}

#define UHSC_IMG_MIN 200
#define UHSC_IMG_MAX 768

void US_Hydrodyn_Saxs_Cormap::setupGUI()
{
   int minHeight1  = 30;

   QSplitter *qs  = new QSplitter( Qt::Vertical  , this );
   QSplitter *qs2 = new QSplitter( Qt::Horizontal, qs );


   lbl_title = new QLabel( parameters.count( "title" ) ? parameters[ "title" ] : QString( "" ), this ); 
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette( PALET_LABEL );
   AUTFBACK( lbl_title );
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   // QGridLayout * gl_image = new QGridLayout( 0 ); gl_image->setContentsMargins( 0, 0, 0, 0 ); gl_image->setSpacing( 0 );


   lbl_image = new mQLabel( qs2 );
   // lbl_image->setScaledContents( true );
   lbl_image->setAlignment( Qt::AlignHCenter );
   lbl_image->setMinimumHeight( UHSC_IMG_MIN );
   lbl_image->setMinimumWidth( UHSC_IMG_MIN );
   connect( lbl_image, SIGNAL( resized() ), SLOT( imageResized() ) );

   // setup frame for off screen imaging
   {
      f_brookesmap = new QFrame( 0 );
      QGridLayout *gl = new QGridLayout( f_brookesmap );
      gl->setMargin( 0 );
      gl->setSpacing( 0 );
   
      lbl_f_title = new QLabel( "", f_brookesmap ); 
      lbl_f_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
      lbl_f_title->setMinimumHeight(minHeight1);
      lbl_f_title->setMaximumHeight(minHeight1);
      lbl_f_title->setPalette( PALET_LABEL );
      AUTFBACK( lbl_f_title );
      lbl_f_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

      lbl_f_none = new QLabel( "", f_brookesmap ); 
      lbl_f_none->setPalette( PALET_LABEL );
      AUTFBACK( lbl_f_none );

      lbl_f_image = new QLabel( "", f_brookesmap );
      // lbl_image->setScaledContents( true );
      // lbl_f_image->setAlignment( Qt::AlignHCenter );
      // lbl_f_image->setMinimumHeight( UHSC_IMG_MIN );
      // lbl_f_image->setMinimumWidth( UHSC_IMG_MIN );

      if ( parameters.count( "as_pairs" ) ) {
         f_thermo_top = new QwtThermo( f_brookesmap );
#if QT_VERSION >= 0x040000
         f_thermo_top->setScalePosition( QwtThermo::TrailingScale );
# if QT_VERSION >= 0x050000
         f_thermo_top->setOrientation( Qt::Horizontal );
# endif
#else
         f_thermo_top->setScalePosition( QwtThermo::Top );
#endif
         f_thermo_top->setScale( 1, pvaluepairs.size() ? pvaluepairs[ 0 ].size() : 1 );
         f_thermo_top->setPipeWidth( 1 );
         f_thermo_top->setBorderWidth( 0 );
//          f_thermo_top->setMargin( 0 );
         f_thermo_top->setFillBrush( QColor( Qt::black ) );
         f_thermo_top->setPalette( PALET_LABEL );
         AUTFBACK( f_thermo_top );
         f_thermo_top->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ) );
         f_thermo_top->setScaleMaxMinor( 0 );

         f_thermo_left = (QwtThermo *)0;

         gl->addWidget( lbl_f_title, 0, 0 );
         gl->addWidget( f_thermo_top , 1, 0 );
         gl->addWidget( lbl_f_image  , 2, 0 );

      } else {
         f_thermo_top = new QwtThermo( f_brookesmap );
#if QT_VERSION >= 0x040000
         f_thermo_top->setScalePosition( QwtThermo::TrailingScale );
# if QT_VERSION >= 0x050000
         f_thermo_top->setOrientation( Qt::Horizontal );
# endif
#else
         f_thermo_top->setScalePosition( QwtThermo::Top );
#endif
         f_thermo_top->setScale( 1, pvaluepairs.size() );
         f_thermo_top->setPipeWidth( 1 );
         f_thermo_top->setBorderWidth( 0 );
//          f_thermo_top->setMargin( 0 );
         f_thermo_top->setFillBrush( QColor( Qt::black ) );
         f_thermo_top->setPalette( PALET_LABEL );
         AUTFBACK( f_thermo_top );
         f_thermo_top->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ) );
         f_thermo_top->setScaleMaxMinor( 0 );
         
         f_thermo_left = new QwtThermo( f_brookesmap );
#if QT_VERSION >= 0x040000
         f_thermo_left->setScalePosition( QwtThermo::TrailingScale );
#else
         f_thermo_left->setScalePosition( QwtThermo::Left );
#endif
         f_thermo_left->setScale( pvaluepairs.size(), 1 );
         f_thermo_left->setPipeWidth( 1 );
         f_thermo_left->setBorderWidth( 0 );
//          f_thermo_left->setMargin( 0 );
         f_thermo_left->setFillBrush( QColor( Qt::black ) );
         f_thermo_left->setPalette( PALET_LABEL );
         AUTFBACK( f_thermo_left );
         f_thermo_left->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ) );
         f_thermo_left->setScaleMaxMinor( 0 );

         gl->addWidget( lbl_f_title , 0 , 0 , 1 + ( 0 ) - ( 0 ) , 1 + ( 1  ) - ( 0 ) );
         gl->addWidget( lbl_f_none   , 1, 0 );
         gl->addWidget( f_thermo_top , 1, 1 );
         gl->addWidget( f_thermo_left, 2, 0 );
         gl->addWidget( lbl_f_image  , 2, 1 );
      }
   }

//   plot = new QwtPlot( qs2 );
   usp_plot = new US_Plot( plot, "", "", "", qs2 );
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
   plot->setAxisTitle(QwtPlot::xBottom, us_tr( "Ref."));
   plot->setAxisTitle(QwtPlot::yLeft  , us_tr( "Red %" ) );
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
#if QT_VERSION < 0x050000
   plot->setTitle( us_tr( "Red pair % histogram\n(Lines represent average, ±1 SD)" ) );
#else
   plot->setTitle( us_tr( "Red pair % histogram\n(Lines represent average, \u00b11 SD)" ) );
#endif
#if QT_VERSION < 0x040000
   plot->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::None);
#else
   plot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine );
#endif
   plot->setCanvasBackground(USglobal->global_colors.plot);

//   plot_cluster = new QwtPlot( qs2 );
   usp_plot_cluster = new US_Plot( plot_cluster, "", "", "", qs2 );
   connect( (QWidget *)plot_cluster->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_cluster( const QPoint & ) ) );
   ((QWidget *)plot_cluster->titleLabel())->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_cluster->axisWidget( QwtPlot::yLeft ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_cluster( const QPoint & ) ) );
   ((QWidget *)plot_cluster->axisWidget( QwtPlot::yLeft ))->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot_cluster->axisWidget( QwtPlot::xBottom ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_cluster( const QPoint & ) ) );
   ((QWidget *)plot_cluster->axisWidget( QwtPlot::xBottom ))->setContextMenuPolicy( Qt::CustomContextMenu );
#if QT_VERSION < 0x040000
   plot_cluster->enableGridXMin();
   plot_cluster->enableGridYMin();
#else
   plot_cluster_grid = new QwtPlotGrid;
   plot_cluster_grid->enableXMin( true );
   plot_cluster_grid->enableYMin( true );
#endif
   plot_cluster->setPalette( PALET_NORMAL );
   AUTFBACK( plot_cluster );
#if QT_VERSION < 0x040000
   plot_cluster->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot_cluster->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   plot_cluster_grid->setMajorPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   plot_cluster_grid->setMinorPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   plot_cluster_grid->attach( plot );
#endif
   plot_cluster->setAxisTitle(QwtPlot::xBottom, us_tr( "Red cluster size"));
   plot_cluster->setAxisTitle(QwtPlot::yLeft  , us_tr( "Count" ) );
#if QT_VERSION < 0x040000
   plot_cluster->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   plot_cluster->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_cluster->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   plot_cluster->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_cluster->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   plot_cluster->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_cluster->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
//    plot_cluster->setMargin(USglobal->config_list.margin);
   plot_cluster->setTitle( us_tr( "\nRed cluster size histogram" ) );
#if QT_VERSION < 0x040000
   plot_cluster->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::None);
#else
   plot_cluster->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine );
#endif
   plot_cluster->setCanvasBackground(USglobal->global_colors.plot);

   cb_adj = new QCheckBox( this );
   cb_adj -> setText( us_tr( "Adjusted P values" ) );
   cb_adj -> setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ) );
   cb_adj -> setPalette( PALET_NORMAL );
   AUTFBACK( cb_adj );
   connect( cb_adj, SIGNAL( clicked() ), SLOT( forceImageResized() ) );

   cb_hb = new QCheckBox( this );
   cb_hb -> setText( us_tr( "Holm-Bonferroni adjusted P values" ) );
   cb_hb -> setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ) );
   cb_hb -> setPalette( PALET_NORMAL );
   AUTFBACK( cb_hb );
   connect( cb_hb, SIGNAL( clicked() ), SLOT( forceImageResized() ) );

   //   QFrame *editor_frame = new QFrame( qs );

   editor = new QTextEdit( qs );
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
   // frame = new QFrame( qs );
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

   // editor->setWordWrapMode (QTextOption::WordWrap);
   editor->setWordWrapMode (QTextOption::NoWrap);
   editor->setMinimumHeight( minHeight1 * 5 );

   pb_help =  new QPushButton ( us_tr( "Help" ), this );
   pb_help -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_help -> setMinimumHeight( minHeight1 );
   pb_help -> setPalette      ( PALET_PUSHB );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

   pb_sliding =  new QPushButton ( us_tr( "Sliding" ), this );
   pb_sliding -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_sliding -> setMinimumHeight( minHeight1 );
   pb_sliding -> setPalette      ( PALET_PUSHB );
   connect( pb_sliding, SIGNAL( clicked() ), SLOT( sliding() ) );
   pb_sliding->hide();

   pb_save_csv =  new QPushButton ( us_tr( "Save" ), this );
   pb_save_csv -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_save_csv -> setMinimumHeight( minHeight1 );
   pb_save_csv -> setPalette      ( PALET_PUSHB );
   connect( pb_save_csv, SIGNAL( clicked() ), SLOT( save_csv() ) );

   pb_load_csv =  new QPushButton ( us_tr( "Load" ), this );
   pb_load_csv -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_load_csv -> setMinimumHeight( minHeight1 );
   pb_load_csv -> setPalette      ( PALET_PUSHB );
   connect( pb_load_csv, SIGNAL( clicked() ), SLOT( load_csv() ) );

   pb_close =  new QPushButton ( us_tr( "Close" ), this );
   pb_close -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_close -> setMinimumHeight( minHeight1 );
   pb_close -> setPalette      ( PALET_PUSHB );
   connect( pb_close, SIGNAL( clicked() ), SLOT( cancel() ) );


   QVBoxLayout * background = new QVBoxLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );
   background->addWidget( lbl_title );
   // background->addWidget( lbl_image );

   //    QBoxLayout *vbl_editor_group = new QVBoxLayout(qs);
   // #if QT_VERSION < 0x040000 || !defined(Q_OS_MAC)
   //    vbl_editor_group->addWidget ( frame );
   // #endif
   //    vbl_editor_group->addWidget ( editor );

   background->addWidget( qs );

   background->addWidget( cb_adj );
   background->addWidget( cb_hb );

   QHBoxLayout * hbl_bottom = new QHBoxLayout(); hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   background->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addWidget ( pb_sliding );
   hbl_bottom->addWidget ( pb_save_csv );
   hbl_bottom->addWidget ( pb_load_csv );
   hbl_bottom->addWidget ( pb_close );
   background->addLayout ( hbl_bottom );
}

void US_Hydrodyn_Saxs_Cormap::cancel()
{
   close();
}

void US_Hydrodyn_Saxs_Cormap::help()
{
   US_Help *online_help;
   online_help = new US_Help( this );
   online_help->show_help("manual/somo/cormap.html");
}

void US_Hydrodyn_Saxs_Cormap::closeEvent( QCloseEvent *e )
{

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Saxs_Cormap::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->textColor();
   editor->setTextColor(color);
   editor->append(msg);
   editor->setTextColor(save_color);
}

void US_Hydrodyn_Saxs_Cormap::clear_display()
{
   editor->clear( );
   editor->append("\n\n");
}

void US_Hydrodyn_Saxs_Cormap::update_font()
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

void US_Hydrodyn_Saxs_Cormap::save()
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

bool US_Hydrodyn_Saxs_Cormap::streak_check( vector < int > & row1, 
                                            vector < int > & row2, 
                                            int & N,
                                            int & S,
                                            int & C,
                                            double & P ) 
{
   if ( row1.size() != row2.size() || !row1.size() ) {
      return false;
   }

   N = (int) row1.size();

   C = 0;
   
   int longest_start       = 0;
   int contiguous_pts      = 0;

   int this_start          = longest_start;
   int this_contiguous_pts = contiguous_pts;

   for ( int i = 0; i < N; ++i ) {
      if ( row1[ i ] == row2[ i ] ) {
         if ( contiguous_pts < this_contiguous_pts ) {
            contiguous_pts = this_contiguous_pts;
            longest_start  = this_start;
         }

         this_start = i;
         this_contiguous_pts = 0;
      } else {
         this_contiguous_pts++;
      }
   }

   if ( contiguous_pts < this_contiguous_pts ) {
      contiguous_pts = this_contiguous_pts;
      longest_start  = this_start;
   }

   S = longest_start + 1;
   C = contiguous_pts;

   P = (double) ((US_Hydrodyn *)us_hydrodyn)->saxs_util->prob_of_streak_f( N, C );
   return true;
}         
      
void US_Hydrodyn_Saxs_Cormap_Cluster_Analysis::cluster_expand( uhs_index_pair x ) {
   // check neighbors
   // cout << QString( "cluster expand [%1,%2]\n" ).arg( x.r ).arg( x.c );
   uhs_index_pair xp;
   {
      xp = x;
      xp.r++;
      if ( cluster_data.count( xp ) && 
           !cluster_marked.count( xp ) &&
           cluster_data[ xp ] == -1 ) {
         cluster_red   .insert( xp );
         cluster_marked.insert( xp );
         cluster_expand( xp );
      }
   }

   {
      xp = x;
      xp.r--;
      if ( cluster_data.count( xp ) && 
           !cluster_marked.count( xp ) &&
           cluster_data[ xp ] == -1 ) {
         cluster_red   .insert( xp );
         cluster_marked.insert( xp );
         cluster_expand( xp );
      }
   }

   {
      xp = x;
      xp.c++;
      if ( cluster_data.count( xp ) && 
           !cluster_marked.count( xp ) &&
           cluster_data[ xp ] == -1 ) {
         cluster_red   .insert( xp );
         cluster_marked.insert( xp );
         cluster_expand( xp );
      }
   }

   {
      xp = x;
      xp.c--;
      if ( cluster_data.count( xp ) && 
           !cluster_marked.count( xp ) &&
           cluster_data[ xp ] == -1 ) {
         cluster_red   .insert( xp );
         cluster_marked.insert( xp );
         cluster_expand( xp );
      }
   }
}

bool US_Hydrodyn_Saxs_Cormap::cluster_analysis() {
   US_Hydrodyn_Saxs_Cormap_Cluster_Analysis ca;

   if ( ca.run( pvaluepairs,
                parameters,
                csv_report,
                this ) ) {
      
      // assemble plot
      if ( ca.cluster_size_histogram.size() ) {
         vector < double > cluster_hist_x;
         vector < double > cluster_hist_y;
         double max_y = 0e0;
         for ( map < int, int >::iterator it = ca.cluster_size_histogram.begin();
               it != ca.cluster_size_histogram.end();
               ++it ) {
            cluster_hist_x.push_back( (double ) it->first );
            cluster_hist_y.push_back( (double ) it->second );
            if ( max_y < it->second ) {
               max_y = it->second;
            }
         }

         {
            int use_line_width = parameters.count( "linewidth" ) ? parameters[ "linewidth" ].toInt() : 1;

            {
#if QT_VERSION < 0x040000
               long curve;
               curve = plot_cluster->insertCurve( "pctred" );
               plot_cluster->setCurveStyle( curve, QwtCurve::Sticks );
#else
               QwtPlotCurve *curve = new QwtPlotCurve( "pctred" );
               curve->setStyle( QwtPlotCurve::Sticks );
#endif

#if QT_VERSION < 0x040000
               plot_cluster->setCurvePen( curve, QPen( Qt::red, 2 * use_line_width, Qt::SolidLine ) );
               plot_cluster->setCurveData( curve,
                                           (double *)&cluster_hist_x[ 0 ],
                                           (double *)&cluster_hist_y[ 0 ],
                                           cluster_hist_x.size()
                                           );
#else
               curve->setPen( QPen( Qt::red, 2 * use_line_width, Qt::SolidLine ) );
               curve->setSamples(
                              (double *)&cluster_hist_x[ 0 ],
                              (double *)&cluster_hist_y[ 0 ],
                              cluster_hist_x.size()
                              );
               curve->attach( plot_cluster );
#endif
            }

            if ( !plot_cluster_zoomer )
            {
               plot_cluster->setAxisScale( QwtPlot::xBottom, cluster_hist_x.front() - 1, cluster_hist_x.back() + 1 );
               plot_cluster->setAxisScale( QwtPlot::yLeft  , 0, max_y * 1.1 );
               plot_cluster_zoomer = new ScrollZoomer(plot_cluster->canvas());
               plot_cluster_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
#if QT_VERSION < 0x040000
               plot_cluster_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
               // connect( plot_cluster_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_cluster_zoomed( const QRectF & ) ) );
            }

         }
      } else {
         if ( !plot_cluster_zoomer )
         {
            plot_cluster->setAxisScale( QwtPlot::xBottom, 0, 1 );
            plot_cluster->setAxisScale( QwtPlot::yLeft  , 0, 1 );
            plot_cluster_zoomer = new ScrollZoomer(plot_cluster->canvas());
            plot_cluster_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
#if QT_VERSION < 0x040000
            plot_cluster_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
            // connect( plot_cluster_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_cluster_zoomed( const QRectF & ) ) );
         }

      }

      plot_cluster->replot();
      plot_cluster->show();

      return true;
   }
   return false;
}

void US_Hydrodyn_Saxs_Cormap::sliding() {
   US_Hydrodyn_Saxs_Cormap_Cluster_Analysis ca;

   map < QString, double >  sliding_results;
   
   ca.sliding( pvaluepairs, parameters, sliding_results, this );
}

bool US_Hydrodyn_Saxs_Cormap_Cluster_Analysis::sliding(
                                                       vector < vector < double > >  & pvaluepairs,
                                                       map < QString, QString >      & parameters,
                                                       map < QString, double >       & sliding_results,
                                                       map < QString, double >       & hb_sliding_results,
                                                       QWidget                       * parent,
                                                       QProgressBar                  * progress
                                                       ) {
   if ( !parameters.count( "hb" ) ) {
      if ( !sliding( pvaluepairs,
                     parameters,
                     sliding_results,
                     parent,
                     progress ) ) {
         return false;
      }

      us_qdebug( "rerunning for hb mode" );

      parameters[ "hb" ] = "true";
      
      if ( !sliding( pvaluepairs,
                     parameters,
                     hb_sliding_results,
                     parent,
                     progress ) ) {
         parameters.erase( "hb" );
         return false;
      }
      parameters.erase( "hb" );
      return true;
   }


   if ( !sliding( pvaluepairs,
                  parameters,
                  sliding_results,
                  parent,
                  progress ) ) {
      hb_sliding_results = sliding_results;
      return false;
   }

   hb_sliding_results = sliding_results;
   return true;
}

bool US_Hydrodyn_Saxs_Cormap_Cluster_Analysis::sliding(
                                                       vector < vector < double > >  & pvaluepairs,
                                                       map < QString, QString >      & parameters,
                                                       map < QString, double >       & sliding_results,
                                                       QWidget                       * parent,
                                                       QProgressBar                  * progress
                                                       ) {

   double alpha        = parameters.count( "alpha" ) ? parameters[ "alpha" ].toDouble() : 0.05;
   double alpha_over_5 = parameters.count( "alpha_over_5" ) ? parameters[ "alpha_over_5" ].toDouble() : 0.2 * alpha;

   // us_qdebug( QString( "sliding alpha %1 over5 %2" ).arg( alpha ).arg( alpha_over_5 ) );

   sliding_results.clear( );

   int pc = (int) pvaluepairs.size();

   if ( pc != (int) pvaluepairs[ 0 ].size() ) {
      errormsg = us_tr( "Internal error: cormap of brookes selected, but pvaluepairs not square" );
      if ( parent ) {
         QMessageBox::warning( parent, parent->windowTitle(), errormsg );
      }
      return false;
   }

   int start_size =
      parameters.count( "sliding_minimum_size" ) ?
      parameters[ "sliding_minimum_size" ].toInt() : 10;

   if ( start_size > pc ) {
      errormsg = us_tr( "Brookes map sliding cluster analysis: too few frames for analysis" );
      if ( parent ) {
         QMessageBox::warning( parent, parent->windowTitle(), errormsg );
      }
      return false;
   }
      
   int pos_base = parameters.count( "sliding_baseline_pos_base" ) ? parameters[ "sliding_baseline_pos_base" ].toInt() : 0;

   // build sub matricies of pvaluepairs and average red cluster size over

   int largest_window = parameters.count( "sliding_maximum_size" ) ? parameters[ "sliding_maximum_size" ].toInt() : pc;
   if ( largest_window > pc ) {
      largest_window = pc;
   }

   if ( progress ) {
      progress->setValue( 0 ); progress->setMaximum( largest_window - start_size + 1 );
   }

   QString save_alpha = parameters.count( "alpha" ) ? parameters[ "alpha" ] : "0.05";

   for ( int i = start_size; i <= largest_window; ++i ) {
      if ( progress ) {
         progress->setValue( i - start_size );
         qApp->processEvents();
      }
      vector < vector < double > > sliding_window( i );
      for ( int j = 0; j < i; ++j ) {
         sliding_window[ j ].resize( i );
      }

      QString prefix    = QString( "%1:"      ).arg( i );
      QString prefix2   = QString( "%1 sd:"   ).arg( i );
      QString prefixcnt = QString( "%1 count" ).arg( i );

      int     count   = 0;

      map < QString, QString >  csv_report;

      for ( int j = 0; j <= pc - i; ++j ) {
         // us_qdebug( QString( "start sliding analysis size %1 pos %2" ).arg( i ).arg( j ) );
         for ( int k = 0; k < i; ++k ) {
            for ( int l = 0; l < i; ++l ) {
               sliding_window[ k ][ l ] = pvaluepairs[ j + k ][ j + l ];
            }
         }
         
         csv_report.clear( );

         if ( parameters.count( "hb" ) ) {
            vector < double > P;
            int sls = (int) sliding_window.size();
            for ( int ii = 0; ii < sls; ++ii ) {
               if ( ii + 1 < sls ) {
                  for ( int jj = ii + 1; jj < sls; ++jj ) {
                     P.push_back( sliding_window[ ii ][ jj ] );
                  }
               }
            }
            parameters[ "alpha" ]        = QString( "%1" ).arg( US_Saxs_Util::holm_bonferroni( P, alpha ) );
            parameters[ "alpha_over_5" ] = QString( "%1" ).arg( US_Saxs_Util::holm_bonferroni( P, alpha_over_5 ) );
         }

         if ( !run( sliding_window,
                    parameters,
                    csv_report,
                    parent ) ) {
            errormsg = us_tr( "Brookes map sliding cluster analysis:" ) + errormsg;
            if ( parent ) {
               QMessageBox::warning( parent, parent->windowTitle(), errormsg );
            }
            return false;
         }

         // us_qdebug( "csv_report:" );
         // for ( map < QString, QString >::iterator it = csv_report.begin();
         //       it != csv_report.end();
         //       ++it ) {
         //    cout << "\t" << it->first << " => " << it->second << endl;
         // }

         if ( parameters.count( "sliding_baseline_mode" ) ) {
            for ( map < QString, QString >::iterator it = csv_report.begin();
                  it != csv_report.end();
                  ++it ) {
               QString this_tag  = QString( "%1:" ).arg( pos_base + j ) + it->first;
               sliding_results[ this_tag  ] = it->second.toDouble();
            }
         } else {
            for ( map < QString, QString >::iterator it = csv_report.begin();
                  it != csv_report.end();
                  ++it ) {
               QString this_tag  = prefix  + it->first;
               QString this_tag2 = prefix2 + it->first;
               double  value    = it->second.toDouble();

               if ( sliding_results.count( this_tag ) ) {
                  sliding_results[ this_tag  ] += value;
                  sliding_results[ this_tag2 ] += value * value;
               } else {
                  sliding_results[ this_tag  ] = value;
                  sliding_results[ this_tag2 ] = value * value;
               }
            }
         }
         count++;
      }

      sliding_results[ prefixcnt ] = (double) count;

      if ( !count ) {
         errormsg = us_tr( "Brookes map sliding cluster analysis: internal error count is zero?" );
         if ( parent ) {
            QMessageBox::warning( parent, parent->windowTitle(), errormsg );
         }
         return false;
      }         

      if ( !parameters.count( "sliding_baseline_mode" ) ) {
         for ( map < QString, QString >::iterator it = csv_report.begin();
               it != csv_report.end();
               ++it ) {
            QString this_tag  = prefix  + it->first;
            QString this_tag2 = prefix2 + it->first;

            double countinv   = 1e0 / (double) count;
            if ( count > 1 ) {
               double countm1inv = 1e0 / ((double) count - 1 );
               sliding_results[ this_tag2 ] = sqrt( countm1inv * ( sliding_results[ this_tag2 ] - countinv * sliding_results[ this_tag ] * sliding_results[ this_tag ] ) );
            } else {
               sliding_results.erase( this_tag2 );
            }
            sliding_results[ this_tag ] *= countinv;
         }
      }
   }

   parameters[ "alpha" ] = save_alpha;
   if ( parameters.count( "alpha_over_5" ) ) {
      parameters.erase( "alpha_over_5" );
   }

   if ( progress ) {
      progress->reset();
   }

   // us_qdebug( "sliding_results" );
   // for ( map < QString, double >::iterator it = sliding_results.begin();
   //       it != sliding_results.end();
   //       ++it ) {
   //    cout << "\t" << it->first << " => " << it->second << endl;
   // }
   return true;
}

bool US_Hydrodyn_Saxs_Cormap_Cluster_Analysis::run(
                                                   vector < vector < double > >  & pvaluepairs,
                                                   map < QString, QString >      & parameters,
                                                   map < QString, QString >      & csv_report,
                                                   QWidget                       * parent
                                                   ) {
   double alpha        = parameters.count( "alpha" ) ? parameters[ "alpha" ].toDouble() : 0.05;
   double alpha_over_5 = parameters.count( "alpha_over_5" ) ? parameters[ "alpha_over_5" ].toDouble() : 0.2 * alpha;

   // us_qdebug( QString( "cca.run alpha %1 alpha_over_5 %2" ).arg( alpha ).arg( alpha_over_5 ) );

   int pc = (int) pvaluepairs.size();

   if ( pc != (int) pvaluepairs[ 0 ].size() ) {
      errormsg = us_tr( "Internal error: cormap of brookes selected, but pvaluepairs not square" );
      if ( parent ) {
         QMessageBox::warning( parent, parent->windowTitle(), errormsg );
      }
      return false;
   }

   // cout << QString( "cluster analysis pc = %1\n" ).arg( pc );

   cluster_data  .clear( );
   cluster_marked.clear( );

   // build above diagonal of rows

   int hb_count_red    = 0;
   int hb_count_points = 0;

   uhs_index_pair x;
   for ( int i = 0; i < pc - 1; ++i ) {
      for ( int j = i + 1; j < pc; ++j ) {
         x.r = i;
         x.c = j;
         if ( pvaluepairs[ i ][ j ] >= alpha_over_5 ) {
            cluster_data[ x ] = 1;
         } else {
            cluster_data[ x ] = -1;
            ++hb_count_red;
         }
         ++hb_count_points;
      }
   }

   csv_report[ "% red pairs" ] = QString( "" ).sprintf( "%.2f", hb_count_points ? 100.0 * ( (double) hb_count_red / (double) hb_count_points ) : 0e0 );

   // map < int, int >            cluster_size_histogram;
   cluster_size_histogram.clear( );
   map < uhs_index_pair, int > cluster_sizes;
   map < int, uhs_index_pair > cluster_size_to_pos;

   double avg_cluster_size = 0e0;
   double sum2_cluster_size = 0e0;
   double avg_cluster_size_pct = 0e0;
   double sum2_cluster_size_pct = 0e0;

   double dpc = (double) pc;
   double area = ( dpc * dpc - dpc ) * 0.5;

   for ( int i = 0; i < pc - 1; ++i ) {
      for ( int j = i + 1; j < pc; ++j ) {
         x.r = i;
         x.c = j;
         
         // cout << QString( "loop checking point [%1,%2]\n" ).arg( x.r ).arg( x.c );

         if ( !cluster_marked.count( x ) && cluster_data[ x ] == -1 ) {
            cluster_red   .clear( );
            cluster_red   .insert( x );
            cluster_marked.insert( x );
            cluster_expand( x );
            cluster_sizes[ x ] = (int) cluster_red.size();
            avg_cluster_size += (double) cluster_red.size();
            sum2_cluster_size += (double) cluster_red.size() * (double) cluster_red.size();
            if ( area > 0 ) {
               double this_cluster_size_pct = 100.0 * (double) cluster_red.size() / area;
               avg_cluster_size_pct += this_cluster_size_pct;
               sum2_cluster_size_pct += this_cluster_size_pct * this_cluster_size_pct;
            }

            if ( !cluster_size_to_pos.count( (int) cluster_red.size() ) ) {
               cluster_size_to_pos[ (int) cluster_red.size() ] = x;
            }
            cluster_size_histogram[ (int) cluster_red.size() ] =
               1 + 
               ( cluster_size_histogram.count( (int) cluster_red.size() ) ?
                 cluster_size_histogram[ (int) cluster_red.size() ] : 0 );
         }
               
      }
   }

   double avg_cluster_size_sd = 0e0;
   double avg_cluster_size_sd_as_pct = 0e0;
   double avg_cluster_size_pct_sd = 0e0;

   if ( cluster_sizes.size() ) {
      double countinv   = 1e0 / (double) cluster_sizes.size();
      if ( cluster_sizes.size() > 2 ) {
         double countm1inv = 1e0 / ((double) cluster_sizes.size() - 1 );
         avg_cluster_size_sd     = sqrt( countm1inv * ( sum2_cluster_size     - countinv * avg_cluster_size * avg_cluster_size ) );
         avg_cluster_size_pct_sd = sqrt( countm1inv * ( sum2_cluster_size_pct - countinv * avg_cluster_size_pct * avg_cluster_size_pct ) );
         if ( avg_cluster_size > 0 ) {
            avg_cluster_size_sd_as_pct = 100.0 * avg_cluster_size_sd / (avg_cluster_size * countinv);
         }
      }
      avg_cluster_size     *= countinv;
      avg_cluster_size_pct *= countinv;
   }

   // build cluster report
   {
      // find max size

      int max_cluster_size =  
         cluster_size_histogram.rbegin() != cluster_size_histogram.rend() ?
         cluster_size_histogram.rbegin()->first : 0 
         ;

      double avg_cluster_size_pct =
         area > 0 ?
         100e0 * (double) avg_cluster_size / area : 0e0;

      double max_cluster_size_pct =
         area > 0 ?
         100e0 * (double) max_cluster_size / area : 0e0;

      parameters[ "clusterheader" ] =
         cluster_sizes.size() ?
#if QT_VERSION < 0x050000
         QString( us_tr( "Red cluster count %1, average size %2 ±%3 %4, average size as pct of total area %5\% ±%6\n"
#else
         QString( us_tr( "Red cluster count %1, average size %2 \u00b1%3 %4, average size as pct of total area %5\% \u00b1%6\n"
#endif
                      "Red cluster maximum size %7 (%8\%)%9.\n" ) )
         .arg( cluster_sizes.size() )
         .arg( QString( "" ).sprintf( "%.2f", avg_cluster_size ) )
         .arg( QString( "" ).sprintf( "%.2f", avg_cluster_size_sd ) )
         .arg( avg_cluster_size > 0 ? QString( "" ).sprintf( "(%.1f%%)", avg_cluster_size_sd_as_pct ) : QString( "" ) )
         .arg( QString( "" ).sprintf( "%.1f", avg_cluster_size_pct ) )
         .arg( QString( "" ).sprintf( "%.1f", avg_cluster_size_pct_sd ) )
         .arg( max_cluster_size )
         .arg( QString( "" ).sprintf( "%3.1f", max_cluster_size_pct ) )
         .arg( max_cluster_size && cluster_size_to_pos.count( max_cluster_size ) 
               ?
               QString( us_tr( " has %1 occurrence%2 and %3begins at [%4,%5]" ) )
               .arg( cluster_size_histogram.rbegin()->second )
               .arg( cluster_size_histogram.rbegin()->second > 1 ? "s" : "" )
               .arg( cluster_size_histogram.rbegin()->second > 1 ? "first occurrence " : "" )
               .arg( cluster_size_to_pos[ max_cluster_size ].r + 1)
               .arg( cluster_size_to_pos[ max_cluster_size ].c + 1) 
               : QString( "" ) )
         
         :
         QString(  us_tr( "No red clusters found.\n" ) )
         ;
      
      csv_report[ "Red cluster size average"                         ] = QString( "" ).sprintf( "%.3f", avg_cluster_size );
      csv_report[ "Red cluster size average sd"                      ] = QString( "" ).sprintf( "%.3f", avg_cluster_size_sd );
      csv_report[ "Red cluster size average sd as pct"               ] = QString( "" ).sprintf( "%.3f", avg_cluster_size_sd_as_pct );
      csv_report[ "Red cluster size average as pct of total area"    ] = QString( "" ).sprintf( "%.3f", avg_cluster_size_pct );
      csv_report[ "Red cluster size average as pct of total area sd" ] = QString( "" ).sprintf( "%.3f", avg_cluster_size_pct_sd );
      csv_report[ "Red cluster size maximum size"                    ] = QString( "%1" ).arg( max_cluster_size );
      csv_report[ "Red cluster size maximum size as pct"             ] = QString( "" ).sprintf( "%.3f", max_cluster_size_pct );
   }

   // cout << "\nCluster sizes:\n";
   // for ( map < uhs_index_pair, int >::iterator it = cluster_sizes.begin();
   //       it != cluster_sizes.end();
   //       ++it ) {
   //    cout << QString( "[%1,%2] %3\n" )
   //       .arg( it->first.r )
   //       .arg( it->first.c )
   //       .arg( it->second );
   // }

   // cout << "\nCluster size histogram:\n";
   // for ( map < int, int >::iterator it = cluster_size_histogram.begin();
   //       it != cluster_size_histogram.end();
   //       ++it ) {
   //    cout << QString( "%1 %2\n" ).arg( it->first ).arg( it->second );
   // }

   return true;
}

bool US_Hydrodyn_Saxs_Cormap::big_green_box_analysis()
{
   // go along diagonal and from each point, find "biggest quality green box"
   // report the best of them and their quality scores
   return false;
}

void US_Hydrodyn_Saxs_Cormap::save_csv()
{
   // QMessageBox::warning(this, 
   //                      windowTitle() + us_tr( " : Save" ),
   //                      us_tr( "Saving PVP analysis results is not currently functional" ), 
   //                      QMessageBox::Ok | QMessageBox::Default,
   //                      QMessageBox::NoButton
   //                      );

   QString use_dir = QDir::current().canonicalPath();
   ((US_Hydrodyn  *)us_hydrodyn)->select_from_directory_history( use_dir, this );

   if ( parameters.count( "name" ) ) {
      use_dir += "/" + parameters[ "name" ] + ( cb_hb->isChecked() ? "_hb" : "" ) + QString( "_a%1" ).arg( parameters.count( "alpha" ) ?  parameters["alpha" ].toDouble() * 0.2 : 0.01 ) + ".csv";
   }

   if ( cb_adj->isChecked() ) {
      parameters[ "adjusted" ] = "true";
   }

   if ( cb_hb->isChecked() ) {
      parameters[ "hb" ] = "true";
   }

   QString use_filename = QFileDialog::getSaveFileName( this , us_tr( "Select a file name for saving" ) , use_dir , "*.csv" );

   if ( use_filename.isEmpty() ) {
      return;
   }
   use_filename = use_filename.replace( QRegExp( ".csv$", Qt::CaseInsensitive ), "" );
   use_filename += ".csv";

   if ( QFile::exists( use_filename ) ) {
      use_filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( use_filename, 0, this );
      raise();
   }
                                        
   QFile f( use_filename );
   if ( !f.open( QIODevice::WriteOnly ) ) {
      QMessageBox::warning( this
                            , windowTitle() + us_tr( " : Save CSV" )
                            , QString( us_tr( "Error: can not open %1 for writing" ) )
                            .arg( use_filename )
                            ,QMessageBox::Ok | QMessageBox::Default
                            ,QMessageBox::NoButton
                            );
      return;
   }

   ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( use_filename );

   QTextStream ts( &f );

   ts 
      << "\"PVP Analysis\""  << Qt::endl
      ;

   ts
      << QString( "\"P value matrix Rows, Columns:\",%1,%2" ).arg( pvaluepairs.size() ).arg( pvaluepairs.size() ? pvaluepairs.front().size() : 0 ) << Qt::endl;

   for ( int i = 0; i < (int) pvaluepairs.size(); ++i ) {
      for ( int j = 0; j < (int) pvaluepairs[ i ].size(); ++j ) {
         ts << pvaluepairs[ i ][ j ] << ",";
      }
      ts << Qt::endl;
   }
   
   ts << "\"Selected Files:\"";
   for ( int i = 0; i < (int) selected_files.size(); ++i ) {
      ts << ",\"" << selected_files[ i ] << '"';
   }
   ts << Qt::endl;

   ts
      << "\"Parameter\",\"Value\"," << parameters.size() << Qt::endl
      ;


   for ( map < QString, QString >::iterator it = parameters.begin();
         it != parameters.end();
         ++it ) {
      ts << '"' << it->first << "\",\"" 
         << it->second.replace( "\n", "__cr__" ).replace( "\"", "__dqt__" )
         << '"' << Qt::endl;
   }

   f.close();
}

void US_Hydrodyn_Saxs_Cormap::load_csv()
{
   QString use_dir = QDir::current().canonicalPath();
   ((US_Hydrodyn  *)us_hydrodyn)->select_from_directory_history( use_dir, this );

   QString filename = QFileDialog::getOpenFileName( this , windowTitle() , use_dir , "*.csv *.CSV" );

   
   if ( filename.isEmpty() ) {
      return;
   }
   ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( filename );

   if (
       load_csv(
                filename,
                parameters,
                pvaluepairs,
                adjpvaluepairs,
                selected_files,
                this
                ) 
       ) {
      us_qdebug( "load ok" );
      displayData();
   } else {
      us_qdebug( "load failed" );
   }
}

bool US_Hydrodyn_Saxs_Cormap::load_csv(
                                       QString                           filename,
                                       map < QString, QString >       &  parameters,
                                       vector < vector < double > >   &  pvaluepairs,
                                       vector < vector < double > >   &  adjpvaluepairs,
                                       vector < QString >             &  selected_files,
                                       QWidget                        *  parent
                                       ) {
   if ( !QFile::exists( filename ) ) {
      if ( parent ) {
         QMessageBox::warning( parent,
                               parent->windowTitle() + us_tr( " : Load CSV file" ),
                               QString( us_tr( "An error occured when trying to open file\n"
                                            "%1\n"
                                            "The file does not exist" ) )
                               .arg( filename )
                               );
      }
      return false;
   }

   QFile f( filename );

   if ( !f.open( QIODevice::ReadOnly ) ) {
      if ( parent ) {
         QMessageBox::warning( parent,
                               parent->windowTitle() + us_tr( " : Load CSV file" ),
                               QString( us_tr( "An error occured when trying to open file\n"
                                            "%1\n"
                                            "Please check the permissions and try again\n" ) )
                               .arg( filename )
                               );
      }
      return false;
   }

   QTextStream ts( &f );
   vector < QString > qv;

   while ( !ts.atEnd() )
   {
      QString qs = ts.readLine();
      qv.push_back( qs );
   }
   f.close();

   if ( !qv.size() )
   {
      if ( parent ) {
         QMessageBox::warning( parent,
                               parent->windowTitle() + us_tr( " : Load CSV file" ),
                               QString( us_tr( "The CSV file %1 is empty" ) )
                               .arg( filename )
                               );
      }
      return false;
   }

   if ( qv[ 0 ] != "\"PVP Analysis\"" ) {
      if ( parent ) {
         QMessageBox::warning( parent,
                               parent->windowTitle() + " : Load CSV file",
                               QString( us_tr( "The CSV file %1 is not tagged as a PVP Analysis file" ) )
                               .arg( filename )
                               );
      }
      return false;
   }

   us_qdebug( "loading csv file" );

   // get pvaluepair info

   int pos = 1;
      
   // pvalues

   vector < vector < double > > new_pvaluepairs;
   {
      if ( pos >= (int) qv.size() ) {
         csv_corrupt_msg( filename, pos, parent );
         return false;
      }

      QStringList qsl = US_Csv::parse_line( qv[ pos++ ] );
   
      if ( qsl.size() < 3 || qsl[ 0 ] != "P value matrix Rows, Columns:" ) {
         csv_corrupt_msg( filename, pos, parent );
         return false;
      }
      
      {
         int p_rows = qsl[ 1 ].toInt();
         int p_cols = qsl[ 2 ].toInt();
         // us_qdebug( QString( "p_rows %1 p_cols %2" ).arg( p_rows ).arg( p_cols ) );
         for ( int i = 0; i < p_rows; ++i ) {
            if ( pos >= (int) qv.size() ) {
               csv_corrupt_msg( filename, pos, parent );
               return false;
            }
            qsl = US_Csv::parse_line( qv[ pos++ ] );
            if ( (int) qsl.size() != p_cols ) {
               csv_corrupt_msg( filename, pos, parent );
               return false;
            }
            vector < double > this_row;
            for ( int j = 0; j < p_cols; ++j ) {
               this_row.push_back( qsl[ j ].toDouble() );
            }
            new_pvaluepairs.push_back( this_row );
         }
      }
   }
   // US_Vector::printvector2( "new_pvaluepairs 0,1", new_pvaluepairs[ 0 ], new_pvaluepairs[ 1 ] );

   // selected files
   vector < QString > new_selected_files;

   {
      if ( pos >= (int) qv.size() ) {
         csv_corrupt_msg( filename, pos, parent );
         return false;
      }
      QStringList qsl = US_Csv::parse_line( qv[ pos++ ] );
      if ( qsl.size() < 3 || qsl[ 0 ] != "Selected Files:" ) {
         csv_corrupt_msg( filename, pos, parent );
         return false;
      }

      qsl.pop_front();

      while ( qsl.size() ) {
         new_selected_files.push_back( qsl.front() );
         qsl.pop_front();
      }
   }

   // US_Vector::printvector( "new_selected_files", new_selected_files );

   // parameters
   map < QString, QString > new_parameters;
   {
      if ( pos >= (int) qv.size() ) {
         csv_corrupt_msg( filename, pos, parent );
         return false;
      }

      QStringList qsl = US_Csv::parse_line( qv[ pos++ ] );
   
      if ( qsl.size() < 3 || qsl[ 0 ] != "Parameter" ) {
         csv_corrupt_msg( filename, pos, parent );
         return false;
      }
      
      int params = qsl[ 2 ].toDouble();

      for ( int i = 0; i < params; ++i ) {
         if ( pos >= (int) qv.size() ) {
            csv_corrupt_msg( filename, pos, parent );
            return false;
         }
         qsl = US_Csv::parse_line( qv[ pos++ ] );
         if ( qsl.size() != 2 ) {
            csv_corrupt_msg( filename, pos, parent );
            return false;
         }
         new_parameters[ qsl[ 0 ] ] = qsl[ 1 ].replace( "__cr__", "\n" ).replace( "__dqt__", "\"" );
         // cout << QString( "qsl before replace\n----\n%1\n-----\n after\n%2\n----\n" ).arg( qsl[ 1 ] ).arg( qsl[ 1 ].replace( "__cr__", "\n" ) ) << endl;
      }
   }      

   // cout << "\nnew_parameters:\n";
   // for ( map < QString, QString >::iterator it = parameters.begin();
   //       it != parameters.end();
   //       ++it ) {
   //    cout << it->first << " => " << it->second << endl;
   // }

   parameters     = new_parameters;
   pvaluepairs    = new_pvaluepairs;
   adjpvaluepairs = new_pvaluepairs;
   selected_files = new_selected_files;

   return true;
}

void US_Hydrodyn_Saxs_Cormap::csv_corrupt_msg( QString file, int line, QWidget * parent ) {
   if ( parent ) {
      QMessageBox::warning( parent,
                            parent->windowTitle() + " : Load CSV file",
                            QString( us_tr( "The CSV file %1 line %2: ended permaturely or is corrupted" ) )
                            .arg( file ).arg( line + 1 )
                            );
   }
}

void US_Hydrodyn_Saxs_Cormap::displayData() {

   if ( plot_zoomer ) {
      delete plot_zoomer;
      plot_zoomer = (ScrollZoomer *)0;
   }      

   if ( plot_cluster_zoomer ) {
      delete plot_cluster_zoomer;
      plot_cluster_zoomer = (ScrollZoomer *)0;
   }      

   plot_zoomer         = ( ScrollZoomer * )0;
   plot_cluster_zoomer = ( ScrollZoomer * )0;

   cb_adj -> setChecked( parameters.count( "adjusted" ) && !parameters.count( "hide_adjpvalues" ) );
   cb_hb  -> setChecked( parameters.count( "hb" )       && !parameters.count( "hide_hb_pvalues" ) );

   editor      ->clear( );
   plot->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   plot_cluster->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_cluster->detachItems( QwtPlotItem::Rtti_PlotMarker );;

   QString msg;
   QString msg_headers;

   if ( parameters.count( "alpha" ) ) {
      alpha = parameters[ "alpha" ].toDouble();
   } else {
      alpha = 0.05;
      parameters[ "alpha" ] = QString( "%1" ).arg( alpha );
   }
      
   alpha_over_5 = 0.2 * alpha;

   if ( parameters.count( "hb_alpha" ) ) {
      hb_alpha        = parameters[ "hb_alpha" ].toDouble();
   }
   if ( parameters.count( "hb_alpha_over_5" ) ) {
      hb_alpha_over_5 = parameters[ "hb_alpha_over_5" ].toDouble();
   }

   QString pvdefmsg =
      QString( 
              "Alpha is %1\n\n"
              "Pairwise P value map color definitions:\n"
              "  P is the pairwise P value as determined by a PVP analysis\n"
              "  Green corresponds to         P >= %2\n" 
              "  Yellow corresponds to %3 > P >= %4\n" 
              "  Red corresponds to    %5 > P\n"
               )
      .arg( alpha_over_5 )
      .arg( alpha )
      .arg( alpha )
      .arg( alpha_over_5 )
      .arg( alpha_over_5 )
      ;

   if ( parameters.count( "as_pairs" ) ) {
      pvdefmsg += "Axis ticks correspond to Ref. as listed below\n";
   } else {
      pvdefmsg += "Axes ticks correspond to Ref. as listed below\n";
   }

   pvdefmsg += "\n";

   csv_headers 
      << ( parameters.count( "csv_id_header" ) ? parameters[ "csv_id_header" ] : QString( "" ) )
      << "Average one-to-all P value"
      << "Average one-to-all P value sd"
      << "Red points pct"
      << "Red contiguous points average P value"
      << "Red contiguous points average P value sd"
      << "Red contiguous points average pct"
      << "Red contiguous points maximum pct"
      << "Red cluster size average"
      << "Red cluster size average sd"
      << "Red cluster size average sd as pct"
      << "Red cluster size average as pct of total area"
      << "Red cluster size average as pct of total area sd"
      << "Red cluster size maximum size"
      << "Red cluster size maximum size as pct"
      ;

   if ( parameters.count( "save_png" ) ) {
      csv_headers << "Image file";
      csv_report[ "Image file" ] = "\"" + QFileInfo( parameters[ "save_png" ] ).fileName() + "\"";
   }
   
   csv_report[ parameters.count( "csv_id_header" ) ? parameters[ "csv_id_header" ] : QString( "" ) ] =
      parameters.count( "csv_id_data" ) ? parameters[ "csv_id_data" ] : QString( "" );

   int sfs = (int) selected_files.size();

   QString fileheader = parameters.count( "fileheader" ) ? parameters[ "fileheader" ] : "Name";

   int max_file_name_len = fileheader.length() + 1;

   for ( int i = 0; i < sfs; ++i ) {
      if ( max_file_name_len < (int) selected_files[ i ].length() ) {
         max_file_name_len = (int) selected_files[ i ].length();
      }
   }

   if ( parameters.count( "as_pairs" ) ) {
      cb_adj->hide();
      cb_hb->hide();

      int green_c  = 0;
      int yellow_c = 0;
      int red_c    = 0;
      
      if ( pvaluepairs.size() ) {
         int use_height =   
            parameters[ "as_pairs" ].toInt() > 0 ?
            parameters[ "as_pairs" ].toInt() : 1;
         lbl_image->setMinimumHeight( use_height > 100 ?
                                      use_height :
                                      100 );

         int pc = (int) pvaluepairs[ 0 ].size();
         qi     = new QImage( pc, use_height, QImage::Format_RGB32 );
         qi_adj = qi;
         qi_hb  = qi;

         for ( int i = 0; i < pc; ++i ) {
            QRgb this_rgb;

            if ( pvaluepairs[ 0 ][ i ] >= alpha ) {
               green_c++;
               this_rgb = qRgb( 0, 255, 0 );
            } else {
               if ( pvaluepairs[ 0 ][ i ] >= alpha_over_5 ) {
                  yellow_c++;
                  this_rgb = qRgb( 255, 255, 0 );
               } else {
                  red_c++;
                  this_rgb = qRgb( 255, 0, 0 );
               }
            }
            for ( int j = 0; j < use_height; ++j ) {
               qi    ->setPixel( i, j, this_rgb );
            }
         }
      }

      double tot_c_pct     =  100e0 / (double) ( green_c + yellow_c + red_c );

      msg += pvdefmsg;

      msg += QString("").sprintf(
                                "P values:\n"
                                " %5.1f%% green (%.1f%%) + yellow (%.1f%%) pairs\n"
                                " %5.1f%% red pairs\n"
                                ,tot_c_pct * (double) (green_c + yellow_c )
                                ,tot_c_pct * (double) green_c
                                ,tot_c_pct * (double) yellow_c
                                ,tot_c_pct * (double) red_c
                                )
         ;

      msg_headers += pvdefmsg;
      msg_headers += QString("").sprintf(
                                         "P values:\n"
                                         " %5.1f%% green (%.1f%%) + yellow (%.1f%%) pairs\n"
                                         " %5.1f%% red pairs\n"
                                         ,tot_c_pct * (double) (green_c + yellow_c )
                                         ,tot_c_pct * (double) green_c
                                         ,tot_c_pct * (double) yellow_c
                                         ,tot_c_pct * (double) red_c
                                         )
         ;

      msg_headers += "\n";
      csv_report[ "Red points pct" ] = QString( "" ).sprintf( "%.2f", tot_c_pct * (double) red_c );
   } else {
      int green_c  = 0;
      int yellow_c = 0;
      int red_c    = 0;

      int adj_green_c  = 0;
      int adj_yellow_c = 0;
      int adj_red_c    = 0;

      int hb_green_c  = 0;
      int hb_yellow_c = 0;
      int hb_red_c    = 0;

      int pc = (int) pvaluepairs.size();
      qi     = new QImage( pc, pc, QImage::Format_RGB32 );
      qi_adj = new QImage( pc, pc, QImage::Format_RGB32 );
      qi_hb  = new QImage( pc, pc, QImage::Format_RGB32 );

      // compute HB alpha's
      {
         vector < double > P;
         for ( int i = 0; i < pc; ++i ) {
            if ( i + 1 < pc ) {
               for ( int j = i + 1; j < pc; ++j ) {
                  P.push_back( pvaluepairs[ i ][ j ] );
               }
            }
         }
         
         hb_alpha        = US_Saxs_Util::holm_bonferroni( P, alpha );
         hb_alpha_over_5 = US_Saxs_Util::holm_bonferroni( P, alpha_over_5 );
      }


      for ( int i = 0; i < pc; ++i ) {
         qi    ->setPixel( i, i, qRgb( 255, 255, 255 ) );
         qi_adj->setPixel( i, i, qRgb( 255, 255, 255 ) );
         qi_hb ->setPixel( i, i, qRgb( 255, 255, 255 ) );

         if ( i + 1 < pc ) {
            for ( int j = i + 1; j < pc; ++j ) {
               if ( pvaluepairs[ i ][ j ] >= alpha ) {
                  green_c++;
                  qi->setPixel( i, j, qRgb( 0, 255, 0 ) );
                  qi->setPixel( j, i, qRgb( 0, 255, 0 ) );
               } else {
                  if ( pvaluepairs[ i ][ j ] >= alpha_over_5 ) {
                     yellow_c++;
                     qi->setPixel( i, j, qRgb( 255, 255, 0 ) );
                     qi->setPixel( j, i, qRgb( 255, 255, 0 ) );
                  } else {
                     red_c++;
                     qi->setPixel( i, j, qRgb( 255, 0, 0 ) );
                     qi->setPixel( j, i, qRgb( 255, 0, 0 ) );
                  }
               }
               if ( adjpvaluepairs[ i ][ j ] >= alpha ) {
                  adj_green_c++;
                  qi_adj->setPixel( i, j, qRgb( 0, 255, 0 ) );
                  qi_adj->setPixel( j, i, qRgb( 0, 255, 0 ) );
               } else {
                  if ( adjpvaluepairs[ i ][ j ] >= alpha_over_5 ) {
                     adj_yellow_c++;
                     qi_adj->setPixel( i, j, qRgb( 255, 255, 0 ) );
                     qi_adj->setPixel( j, i, qRgb( 255, 255, 0 ) );
                  } else {
                     adj_red_c++;
                     qi_adj->setPixel( i, j, qRgb( 255, 0, 0 ) );
                     qi_adj->setPixel( j, i, qRgb( 255, 0, 0 ) );
                  }
               }
               if ( pvaluepairs[ i ][ j ] >= hb_alpha ) {
                  hb_green_c++;
                  qi_hb->setPixel( i, j, qRgb( 0, 255, 0 ) );
                  qi_hb->setPixel( j, i, qRgb( 0, 255, 0 ) );
               } else {
                  if ( pvaluepairs[ i ][ j ] >= hb_alpha_over_5 ) {
                     hb_yellow_c++;
                     qi_hb->setPixel( i, j, qRgb( 255, 255, 0 ) );
                     qi_hb->setPixel( j, i, qRgb( 255, 255, 0 ) );
                  } else {
                     hb_red_c++;
                     qi_hb->setPixel( i, j, qRgb( 255, 0, 0 ) );
                     qi_hb->setPixel( j, i, qRgb( 255, 0, 0 ) );
                  }
               }
            }
         }
      }
      // QPixmap pm;
      // pm.convertFromImage( qi.smoothScale( 
      //                                     pc < UHSC_IMG_MIN ?
      //                                     UHSC_IMG_MIN : 
      //                                     ( pc > UHSC_IMG_MAX ?
      //                                       UHSC_IMG_MAX : pc )
                                          
      //                                     ,pc < UHSC_IMG_MIN ?
      //                                     UHSC_IMG_MIN : 
      //                                     ( pc > UHSC_IMG_MAX ?
      //                                       UHSC_IMG_MAX : pc )

      //                                     ,QImage::ScaleMin                                           
      //                                      ) );
      // lbl_image->setPixmap( pm );
      // lbl_image->setMinimumHeight( UHSC_IMG_MIN );
      // lbl_image->setMinimumWidth ( UHSC_IMG_MIN );

      double tot_c_pct     =  100e0 / (double) ( green_c + yellow_c + red_c );

      if ( cb_hb->isChecked() ) {
         pvdefmsg =
            QString( 
                    "Alpha is %1\n\n"
                    "Holm-Bonferroni pairwise P value map color definitions:\n"
                    "  P is the pairwise P value as determined by a PVP analysis\n"
                    "  Green corresponds to              P >= %2\n" 
                    "  Yellow corresponds to %3 > P >= %4\n" 
                    "  Red corresponds to    %5 > P\n"
                     )
            .arg( alpha_over_5 )
            .arg( QString("").sprintf( "%6.4g", hb_alpha ) )
            .arg( QString("").sprintf( "%6.4g", hb_alpha ) )
            .arg( QString("").sprintf( "%6.4g", hb_alpha_over_5 ) )
            .arg( QString("").sprintf( "%6.4g", hb_alpha_over_5 ) )
            ;

         pvdefmsg += "Axes ticks correspond to Ref. as listed below\n";
         pvdefmsg += "\n";
      }      
      
      msg += pvdefmsg;
      if ( !parameters.count( "hb" ) ) {
         msg += QString("").sprintf(
                                    "P values:\n"
                                    " %5.1f%% green (%.1f%%) + yellow (%.1f%%) pairs\n"
                                    " %5.1f%% red pairs\n"
                                    ,tot_c_pct * (double) (green_c + yellow_c )
                                    ,tot_c_pct * (double) green_c
                                    ,tot_c_pct * (double) yellow_c
                                    ,tot_c_pct * (double) red_c
                                    )
            ;
      }

      msg_headers += pvdefmsg;
      
      if ( !parameters.count( "hb" ) ) {
         msg_headers += QString("").sprintf(
                                            "P values:\n"
                                            " %5.1f%% green (%.1f%%) + yellow (%.1f%%) pairs\n"
                                            " %5.1f%% red pairs\n"
                                            ,tot_c_pct * (double) (green_c + yellow_c )
                                            ,tot_c_pct * (double) green_c
                                            ,tot_c_pct * (double) yellow_c
                                            ,tot_c_pct * (double) red_c
                                            )
            ;
      }

      csv_report[ "Red points pct" ] = QString( "" ).sprintf( "%.2f", tot_c_pct * (double) red_c );

      if ( !parameters.count( "hide_adjpvalues" ) ) {
         msg += QString("").sprintf(
                                    "Adjusted P values:\n"
                                    " %5.1f%% green (%.1f%%) + yellow (%.1f%%) pairs\n"
                                    " %5.1f%% red pairs\n"
                                    ,tot_c_pct * (double) (adj_green_c + adj_yellow_c )
                                    ,tot_c_pct * (double) adj_green_c
                                    ,tot_c_pct * (double) adj_yellow_c
                                    ,tot_c_pct * (double) adj_red_c
                                    )
         ;

      } else {
         cb_adj->setChecked( false );
         cb_adj->hide();
      }

      if ( !parameters.count( "hide_hb_pvalues" ) ) {
         if ( parameters.count( "hb" ) ) {
            msg += QString("").sprintf(
                                       "Holm-Bonferroni adjusted P values:\n"
                                       // "Yellow HB P value cutoff %.3g Red HB P value cutoff %.3g\n"
                                       " %5.1f%% green (%.1f%%) + yellow (%.1f%%) pairs\n"
                                       " %5.1f%% red pairs\n"
                                       // ,hb_alpha
                                       // ,hb_alpha_over_5
                                       ,tot_c_pct * (double) (hb_green_c + hb_yellow_c )
                                       ,tot_c_pct * (double) hb_green_c
                                       ,tot_c_pct * (double) hb_yellow_c
                                       ,tot_c_pct * (double) hb_red_c
                                       )
               ;
         }
         parameters[ "hb_alpha"        ] = QString( "%1" ).arg( hb_alpha );
         parameters[ "hb_alpha_over_5" ] = QString( "%1" ).arg( hb_alpha_over_5 );
      } else {
         cb_hb->setChecked( false );
         cb_hb->hide();
      }
   }

   plot->hide();

   plot_cluster->hide();

   if ( parameters.count( "clusteranalysis" ) ) {
      if ( cb_hb->isChecked() ) {
         QString save_alpha = parameters[ "alpha" ];
         
         parameters[ "alpha"        ] = parameters[ "hb_alpha" ];
         parameters[ "alpha_over_5" ] = parameters[ "hb_alpha_over_5" ];

         cluster_analysis();

         parameters[ "alpha" ] = save_alpha;
         parameters.erase( "alpha_over_5" );
      } else {
         cluster_analysis();
      }         
   }

   QString cobreport;
   QString cobheader;

   if ( false && parameters.count( "cormap_of_brookes" ) ) {
      int pc = (int) pvaluepairs.size();
      if ( pc != (int) pvaluepairs[ 0 ].size() ) {
         QMessageBox::warning( this, windowTitle(), us_tr( "Internal error: cormap of brookes selected, but pvaluepairs not square" ) );
      } else {
         vector < vector < int > > rows( pc );

         int    N;
         int    S;
         int    C;
         double P;


         double avg_pctred    = 0e0;
         double max_pctred    = 0e0;
         double sum2_pctred   = 0e0;

         double avg_P         = 0e0;
         double sum2_P        = 0e0;

         // build up q & rows

         vector < int > row0( pc - 1, 1 );

         for ( int i = 0; i < pc; ++i ) {
            for ( int j = 0; j < pc; ++j ) {
               if ( i != j ) {
                  rows[ i ].push_back( i == j ? 0 : ( pvaluepairs[ i ][ j ] >= alpha_over_5 ? 1 : -1 ) );
               }
            }
            // US_Vector::printvector( QString( "row %1" ).arg( i ), rows[ i ] );
         }
                    
         // cobreport =
         //    us_tr( "\PVP of Brookes plot analysis:\n" )  +
         //    QString( "%1\t    N  Start point  C   P-value\n" )
         //    .arg( "Row", -7 )
         //    ;

         cobreport = us_tr( "\nMaximum contiguous red analysis:\n" ) +
            QString( "%1\t       P value    Red-streak   \% of points\n" )
            .arg( "Ref.", -7 )
            ;
            
         bool bad_streak = false;
         for ( int i = 0; !bad_streak && i < pc; ++i ) {
               
            if ( !streak_check( row0, rows[ i ], N, S, C, P ) ) {
               bad_streak = true;
               QMessageBox::warning( this, windowTitle(), 
                                     QString( us_tr( "Internal error: cormap of brookes got cormap error %1" ) ).arg( ((US_Hydrodyn *)us_hydrodyn)->saxs_util->errormsg ) );
               break;
            }

            // cobreport +=
            //    QString( "%1\t%2\t%3\t%4\t%5"
            //             // "\t%6"
            //             "\n" )
            //    .arg( i + 1, -7 )
            //    .arg( N, 6 )
            //    .arg( S, 6 )
            //    .arg( C, 6 )
            //    .arg( QString( "" ).sprintf( "%.4g", P ).leftJustified( 12 ) )
            //    ;

            double red_pct = 100e0 * ( double ) C / ( double ) N;
            if ( max_pctred < red_pct ) {
               max_pctred = red_pct;
            }
            avg_pctred += red_pct;
            sum2_pctred += red_pct * red_pct;

            avg_P  += P;
            sum2_P += P * P;

            cobreport += QString( "%1\t %2\t %2\t %4\%\n" )
               .arg( i + 1, -7 )
               .arg( QString( "" ).sprintf( "%.4f", P ).rightJustified( 12 ) )
               .arg( C, 6 )
               .arg( QString( "" ).sprintf( "%3.2f", red_pct ).rightJustified( 12 ) )
               ;
         }

         double countinv   = 1e0 / (double) pc;
         if ( pc > 2 ) {
            double countm1inv = 1e0 / ((double) pc - 1 );
            double pct_red_sd = sqrt( countm1inv * ( sum2_pctred - countinv * avg_pctred * avg_pctred ) );
            double P_sd = sqrt( countm1inv * ( sum2_P - countinv * avg_P * avg_P ) );

            avg_pctred *= countinv;
            avg_P      *= countinv;

#if QT_VERSION < 0x050000
            cobheader += QString( "Contiguous red points P value average %1 ±%2\n" )
#else
            cobheader += QString( "Contiguous red points P value average %1 \u00b1%2\n" )
#endif
               .arg( QString( "" ).sprintf( "%.4f", avg_P ) )
               .arg( QString( "" ).sprintf( "%.4f", P_sd ) )
               ;
               
#if QT_VERSION < 0x050000
            cobheader += QString( "Contiguous red points average %1\% ±%2 %3 maximum %4\%\n" )
#else
            cobheader += QString( "Contiguous red points average %1\% \u00b1%2 %3 maximum %4\%\n" )
#endif
               .arg( QString( "" ).sprintf( "%4.2f", avg_pctred ) )
               .arg( QString( "" ).sprintf( "%4.2f", pct_red_sd ) )
               .arg( avg_pctred > 0 ? QString( "" ).sprintf( "(%.1f%%)", 100.0 * pct_red_sd / avg_pctred ) : QString( "" ) )
               .arg( QString( "" ).sprintf( "%4.2f", max_pctred ) )
               ;

            csv_report[ "Red contiguous points average P value" ] = QString( "" ).sprintf( "%.6f", avg_P );
            csv_report[ "Red contiguous points average P value sd" ] = QString( "" ).sprintf( "%.6f", P_sd );

            csv_report[ "Red contiguous points average pct" ] = QString( "" ).sprintf( "%.2f", avg_pctred );
            csv_report[ "Red contiguous points maximum pct" ] = QString( "" ).sprintf( "%.2f", max_pctred );
         }
      }
   }

   if ( parameters.count( "linewisesummary" ) ) {
      // build summary report per line

      if ( sfs != (int) pvaluepairs.size() ||
           sfs != (int) pvaluepairs[ 0 ].size() ) {
         msg += QString( "Internal error, cormap utility can't compute linewise summary %1 != %2 or !%3\n" )
            .arg( sfs )
            .arg( (int) pvaluepairs.size() )
            .arg( (int) pvaluepairs[0].size() )
            ;
      } else {
         QString linereport;

         linereport += 
            QString( "\n Ref. : %1   Avg. P value    Min. P Value      \% Red\n" )
            .arg( fileheader, -max_file_name_len );

         double avg_avgP    = 0e0;
         double sum2_avgP   = 0e0;
         double avg_pct_red = 0e0;
         double max_pct_red = 0e0;
         double sum2_pct_red = 0e0;

         vector < double > plot_pos;
         vector < double > plot_redpct;
         vector < double > plot_P;

         // add sd's to computation

         double use_alpha_over_5 = cb_hb->isChecked() ? hb_alpha_over_5 : alpha_over_5;

         for ( int i = 0; i < sfs; ++i ) {

            double avgP = 0e0;
            double minP = 1e0;
            int red_count = 0;

            for ( int j = 0; j < sfs; ++j ) {
               if ( j != i ) {
                  avgP += pvaluepairs[ i ][ j ];
                  if ( minP > pvaluepairs[ i ][ j ] ) {
                     minP = pvaluepairs[ i ][ j ];
                  }
                  if ( pvaluepairs[ i ][ j ] < use_alpha_over_5 ) {
                     red_count++;
                  }
               }
            }

            avgP /= (double) (sfs - 1);

            double pct_red = 100e0 * (double) red_count / (double) ( sfs - 1 );

            avg_avgP     += avgP;
            sum2_avgP    += avgP * avgP;
            avg_pct_red  += pct_red;
            sum2_pct_red += pct_red * pct_red;

            linereport += 
               QString( "%1 : %2     %3    %4    %5\%\n" )
               .arg( i + 1, 5 )
               .arg( selected_files[ i ], -max_file_name_len )
               .arg( QString( "" ).sprintf( "%.4g", avgP ).leftJustified( 12 ) )
               .arg( QString( "" ).sprintf( "%.4g", minP ).leftJustified( 12 ) )
               .arg( QString( "" ).sprintf( "%3.1f", pct_red ), 5 )
               ;

            plot_pos   .push_back( (double) ( i + 1 ) );
            plot_redpct.push_back( pct_red );
            plot_P     .push_back( avgP );

            if ( max_pct_red < pct_red ) {
               max_pct_red = pct_red;
            }
         }
         linereport += "\n"; 

         double countinv   = 1e0 / (double) sfs;
         double avgP_sd;
         double pct_red_sd = 0e0;

         if ( sfs > 2 ) {
            double countm1inv = 1e0 / ((double) sfs - 1 );
            avgP_sd    = sqrt( countm1inv * ( sum2_avgP    - countinv * avg_avgP    * avg_avgP    ) );
            pct_red_sd = sqrt( countm1inv * ( sum2_pct_red - countinv * avg_pct_red * avg_pct_red ) );

            avg_avgP    *= countinv;
            avg_pct_red *= countinv;

#if QT_VERSION < 0x050000
            msg += QString( "\nAverage one-to-all P value %1 ±%2 %3 \% red %4\% ±%5 %6\n" )
#else
            msg += QString( "\nAverage one-to-all P value %1 \u00b1%2 %3 \% red %4\% \u00b1%5 %6\n" )
#endif
               .arg( QString( "" ).sprintf( "%.4g", avg_avgP ) )
               .arg( QString( "" ).sprintf( "%.4g", avgP_sd ) )
               .arg( avg_avgP > 0 ? QString( "" ).sprintf( "(%.1f%%)", 100.0 * avgP_sd / avg_avgP ) : QString( "" ) )
               .arg( QString( "" ).sprintf( "%3.1f", avg_pct_red ) )
               .arg( QString( "" ).sprintf( "%3.1f", pct_red_sd ) )
               .arg( avg_pct_red > 0 ? QString( "" ).sprintf( "(%.1f%%)", 100.0 * pct_red_sd / avg_pct_red ) : QString( "" ) )
               + cobheader
               + ( parameters.count( "clusterheader" ) ? parameters[ "clusterheader" ] : "" )
               + cobreport
               + linereport;

            msg_headers += 
#if QT_VERSION < 0x050000
               QString( "\nAverage one-to-all P value %1 ±%2 %3 \% red %4\% ±%5 %6\n" )
#else
               QString( "\nAverage one-to-all P value %1 \u00b1%2 %3 \% red %4\% \u00b1%5 %6\n" )
#endif
               .arg( QString( "" ).sprintf( "%.4g", avg_avgP ) )
               .arg( QString( "" ).sprintf( "%.4g", avgP_sd ) )
               .arg( avg_avgP > 0 ? QString( "" ).sprintf( "(%.1f%%)", 100.0 * avgP_sd / avg_avgP ) : QString( "" ) )
               .arg( QString( "" ).sprintf( "%3.1f", avg_pct_red ) )
               .arg( QString( "" ).sprintf( "%3.1f", pct_red_sd ) )
               .arg( avg_pct_red > 0 ? QString( "" ).sprintf( "(%.1f%%)", 100.0 * pct_red_sd / avg_pct_red ) : QString( "" ) )
               + cobheader
               + ( parameters.count( "clusterheader" ) ? parameters[ "clusterheader" ] : "" )
               ;

            csv_report[ "Average one-to-all P value" ]    = QString( "" ).sprintf( "%.4g", avg_avgP );
            csv_report[ "Average one-to-all P value sd" ] = QString( "" ).sprintf( "%.4g", avgP_sd );

         } else {
            avg_avgP    *= countinv;
            avg_pct_red *= countinv;

            msg += QString( "   average one-to-all P %1 red %2\%\n" )
               .arg( QString( "" ).sprintf( "%.4g", avg_avgP ) )
               .arg( QString( "" ).sprintf( "%3.1f", avg_pct_red ) )
               + cobheader
               + ( parameters.count( "clusterheader" ) ? parameters[ "clusterheader" ] : "" )
               + cobreport
               + linereport;

            msg_headers += 
               QString( "   average one-to-all P %1 red %2\%\n" )
               .arg( QString( "" ).sprintf( "%.4g", avg_avgP ) )
               .arg( QString( "" ).sprintf( "%3.1f", avg_pct_red ) )
               + cobheader
               + ( parameters.count( "clusterheader" ) ? parameters[ "clusterheader" ] : "" );

            csv_report[ "Average one-to-all P value" ]    = QString( "" ).sprintf( "%.4g", avg_avgP );
         }

         // plot data

         {
            int use_line_width = parameters.count( "linewidth" ) ? parameters[ "linewidth" ].toInt() : 1;

            {
#if QT_VERSION < 0x040000
               long curve;
               curve = plot->insertCurve( "pctred" );
               plot->setCurveStyle( curve, QwtCurve::Sticks );
#else
               QwtPlotCurve *curve = new QwtPlotCurve( "pctred" );
               curve->setStyle( QwtPlotCurve::Sticks );
#endif

#if QT_VERSION < 0x040000
               plot->setCurvePen( curve, QPen( Qt::red, 2 * use_line_width, Qt::SolidLine ) );
               plot->setCurveData( curve,
                                   (double *)&plot_pos[ 0 ],
                                   (double *)&plot_redpct[ 0 ],
                                   plot_pos.size()
                                   );
#else
               curve->setPen( QPen( Qt::red, 2 * use_line_width, Qt::SolidLine ) );
               curve->setSamples(
                              (double *)&plot_pos[ 0 ],
                              (double *)&plot_redpct[ 0 ],
                              plot_pos.size()
                              );
               curve->attach( plot );
#endif
            }

            {

               double x[2];
               double y[2];

               x[0] = plot_pos.front() - 1;
               x[1] = plot_pos.back() + 1;

               {
                  y[0] = y[1] = avg_pct_red;

#if QT_VERSION < 0x040000
                  long curve;
                  curve = plot->insertCurve( "avgred" );
                  plot->setCurveStyle( curve, QwtCurve::Lines );
#else
                  QwtPlotCurve *curve = new QwtPlotCurve( "avgred" );
                  curve->setStyle( QwtPlotCurve::Lines );
#endif

#if QT_VERSION < 0x040000
                  plot->setCurvePen( curve, QPen( Qt::green, use_line_width, Qt::DotLine ) );
                  plot->setCurveData( curve, x, y, 2 );
#else
                  curve->setPen( QPen( Qt::green, use_line_width, Qt::DotLine ) );
                  curve->setSamples( x, y, 2 );
                  curve->attach( plot );
#endif
               }

               if ( sfs > 2 ) {
                  {
                     y[0] = y[1] = avg_pct_red + pct_red_sd;;

#if QT_VERSION < 0x040000
                     long curve;
                     curve = plot->insertCurve( "sdredplus" );
                     plot->setCurveStyle( curve, QwtCurve::Lines );
#else
                     QwtPlotCurve *curve = new QwtPlotCurve( "sdredplus" );
                     curve->setStyle( QwtPlotCurve::Lines );
#endif

#if QT_VERSION < 0x040000
                     plot->setCurvePen( curve, QPen( Qt::yellow, use_line_width, Qt::DotLine ) );
                     plot->setCurveData( curve, x, y, 2 );
#else
                     curve->setPen( QPen( Qt::yellow, use_line_width, Qt::DotLine ) );
                     curve->setSamples( x, y, 2 );
                     curve->attach( plot );
#endif
                  }
                  {
                     y[0] = y[1] = avg_pct_red - pct_red_sd;

#if QT_VERSION < 0x040000
                     long curve;
                     curve = plot->insertCurve( "sdredminus" );
                     plot->setCurveStyle( curve, QwtCurve::Lines );
#else
                     QwtPlotCurve *curve = new QwtPlotCurve( "sdredminus" );
                     curve->setStyle( QwtPlotCurve::Lines );
#endif

#if QT_VERSION < 0x040000
                     plot->setCurvePen( curve, QPen( Qt::yellow, use_line_width, Qt::DotLine ) );
                     plot->setCurveData( curve, x, y, 2 );
#else
                     curve->setPen( QPen( Qt::yellow, use_line_width, Qt::DotLine ) );
                     curve->setSamples( x, y, 2 );
                     curve->attach( plot );
#endif
                  }
               }
            }

            if ( !plot_zoomer )
            {
               plot->setAxisScale( QwtPlot::xBottom, plot_pos.front() - 1, plot_pos.back() + 1 );
               plot->setAxisScale( QwtPlot::yLeft  , 0, max_pct_red * 1.1 );
               plot_zoomer = new ScrollZoomer(plot->canvas());
               plot_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
#if QT_VERSION < 0x040000
               plot_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
               // connect( plot_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_zoomed( const QRectF & ) ) );
            }

            plot->replot();
            plot->show();
         }
      }
   }

   editor->append( "<a name=\"top\">" );
   editor_msg( "black", msg );
   parameters[ "report" ] = msg;
   if ( parameters.count( "msg" ) ) {
      editor_msg( "dark blue", parameters[ "msg" ] );
   }
   editor->scrollToAnchor( "top" );

   if ( parameters.count( "save_png" ) ) {
      editor_msg( "black", "\n\n" + msg_headers );
   }

   if ( parameters.count( "save_csv" ) ) {
      QString out;
      if ( !parameters.count( "csv_skip_report_header" ) ) {
         for ( int i = 0; i < (int) csv_headers.size(); ++i ) {
            out += QString( "%1\"%2\"" ).arg( i ? "," : "" ).arg( csv_headers[ i ] );
         }
         out += "\n";
      }
      for ( int i = 0; i < (int) csv_headers.size(); ++i ) {
         out += ( i ? "," : "" ) 
            + ( csv_report.count( csv_headers[ i ] ) ?
                csv_report[ csv_headers[ i ] ] : "" );
      }
      out += "\n";

      
      QFile f( parameters[ "save_csv" ] );
      if ( f.open( QIODevice::WriteOnly | QIODevice::Append ) ) {
         QTextStream tso( &f );
         tso << out;
         f.close();
      }
      // cout << out;
   }

   if ( parameters.count( "data_csv" ) ) {
      if ( !parameters.count( "csv_skip_report_header" ) ) {
         ((US_Hydrodyn *) us_hydrodyn)->data_csv_headers = csv_headers;         
         ((US_Hydrodyn *) us_hydrodyn)->data_csv.clear( );
      }

      for ( int i = 0; i < (int) csv_headers.size(); ++i ) {
         if ( !((US_Hydrodyn *) us_hydrodyn)->data_csv.count( csv_headers[ i ] ) ) {
            map < QString, vector < QString > > tmp_map;
            vector < QString > tmp_vect;
            tmp_map[ parameters[ "data_csv" ] ] = tmp_vect;
            ((US_Hydrodyn *) us_hydrodyn)->data_csv[ csv_headers[ i ] ] = tmp_map;
         }
         ((US_Hydrodyn *) us_hydrodyn)->data_csv[ csv_headers[ i ] ][ parameters[ "data_csv" ] ]
            .push_back( csv_report.count( csv_headers[ i ] ) ?
                        csv_report[ csv_headers[ i ] ] : "" );
      }
   }

   if ( true ) {
      QRect geom = geometry();

      if ( parameters.count( "global_width" ) ) {
         geom.setWidth( parameters[ "global_width" ].toInt() );
         if ( !parameters.count( "as_pairs" ) ) {
            if ( parameters.count( "clusteranalysis" ) ) {
               QRect plot_geom = plot->geometry();
               plot_geom.setWidth( ( parameters[ "global_width" ].toInt() - 20 ) / 4 );
               plot->setGeometry( plot_geom );
               QRect plot_cluster_geom = plot->geometry();
               plot_cluster_geom.setWidth( ( parameters[ "global_width" ].toInt() - 20 ) / 4 );
               plot_cluster->setGeometry( plot_geom );
               
               // plot->setMaximumWidth( ( parameters[ "global_width" ].toInt() - 20 ) / 4 );
               // plot->setMinimumWidth( ( parameters[ "global_width" ].toInt() - 20 ) / 4 );
               // plot_cluster->setMaximumWidth( ( parameters[ "global_width" ].toInt() - 20 ) / 4 );
               // plot_cluster->setMinimumWidth( ( parameters[ "global_width" ].toInt() - 20 ) / 4 );
            } else {
               QRect plot_geom = plot->geometry();
               plot_geom.setWidth( ( parameters[ "global_width" ].toInt() - 20 ) / 2 );
               plot->setGeometry( plot_geom );
               // plot->setMaximumWidth( ( parameters[ "global_width" ].toInt() - 20 ) / 2 );
               // plot->setMinimumWidth( ( parameters[ "global_width" ].toInt() - 20 ) / 2 );
            }
         }
      }
      if ( parameters.count( "global_height" ) ) {
         geom.setHeight( parameters[ "global_height" ].toInt() );
      }

      setGeometry( geom );

      QRect lbl_geom = lbl_image->geometry();
      if ( parameters.count( "image_height" ) ) {
         lbl_geom.setHeight( parameters[ "image_height" ].toInt() );
         lbl_geom.setWidth( parameters[ "image_height" ].toInt() );
      }
      lbl_image->setGeometry( lbl_geom );

      QRect editor_geom = editor->geometry();
      if ( parameters.count( "editor_height" ) ) {
         editor_geom.setHeight( parameters[ "editor_height" ].toInt() );
      }
      editor->setGeometry( editor_geom );
   }

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

void US_Hydrodyn_Saxs_Cormap::forceImageResized() {
   last_width = -1;
   if ( cb_hb->isChecked() ) {
      parameters[ "hb" ] = "true";
   } else {
      parameters.erase( "hb" );
   }
   displayData();
   imageResized();
}

void US_Hydrodyn_Saxs_Cormap::imageResized() {
   // us_qdebug( QString( "image width %1 height %2" ).arg( lbl_image->width() ).arg( lbl_image->height() ) );
   // us_qdebug( QString( "last use width %1 height %2" ).arg( last_width ).arg( last_height ) );
   // us_qdebug( QString( "editor width %1 height %2" ).arg( editor->width() ).arg( editor->height() ) );

   QRect org_geom = geometry();
   QRect org_lbl_geom = lbl_image->geometry();

   int max_width  = lbl_image->width();
   int max_height = lbl_image->height();
   f_brookesmap->repaint();
#if QT_VERSION >= 0x040000
   qApp->processEvents( QEventLoop::AllEvents, 1000 );
#else
   qApp->processEvents( 1000 );
#endif

   int avail_i_width  = max_width  - ( f_thermo_left ? f_thermo_left->width() : 0 );
   int avail_i_height = max_height - f_thermo_top->height() - lbl_f_title->height();

   if ( parameters.count( "as_pairs" ) ) {

      int use_i_width  = avail_i_width;
      int use_i_height = avail_i_height + f_thermo_top->height() + lbl_f_title->height();

      if ( use_i_width  == last_width &&
           use_i_height == last_height ) {
         // us_qdebug( "skipped" );
         return;
      }

      last_width  = use_i_width;
      last_height = use_i_height;

      {
         QPixmap pm;
         lbl_f_image->setPixmap( pm );
         f_brookesmap->repaint();
#if QT_VERSION >= 0x040000
         qApp->processEvents( QEventLoop::AllEvents, 1000 );
#else
         qApp->processEvents( 1000 );
#endif
      }

      f_brookesmap->setMinimumWidth ( use_i_width );
      f_brookesmap->setMaximumWidth ( use_i_width );
      f_brookesmap->setMinimumHeight( use_i_height );
      f_brookesmap->setMaximumHeight( use_i_height );

      f_brookesmap->repaint();
#if QT_VERSION >= 0x040000
      qApp->processEvents( QEventLoop::AllEvents, 1000 );
#else
      qApp->processEvents( 1000 );
#endif

      {
         QPixmap pm;
         lbl_f_title->setText( 
                              parameters.count( "ppvm_title" ) ?
                              parameters[ "ppvm_title" ] :
                              us_tr( "Pairwise P value map" ) 
                               );
         pm.convertFromImage( 
#if QT_VERSION >= 0x040000
                             qi->scaled( 
                                        QSize( use_i_width, avail_i_height )
                                        // ,  Qt::KeepAspectRatio 
                                         )
#else
                             qi->smoothScale(
                                             use_i_width
                                             ,avail_i_height
                                             )
#endif
                              );
         lbl_f_image->setPixmap( pm );
      }
      f_brookesmap->repaint();
#if QT_VERSION >= 0x040000
      qApp->processEvents( QEventLoop::AllEvents, 1000 );
#else
      qApp->processEvents( 1000 );
#endif
      {
         QPixmap pm = f_brookesmap->grab();

         if ( org_geom != geometry() ) {
            // us_qdebug( "geometry changed" );
            last_width = last_height = -1;
            return; //  imageResized();
         }
      
         if ( org_lbl_geom != lbl_image->geometry() ) {
            // us_qdebug( "lbl geometry changed" );
            last_width = last_height = -1;
            return; //  imageResized();
         }

         lbl_image->setPixmap( pm );
      }
      // update();
      // repaint();
      return;
   }


   // us_qdebug( QString( "max width %1 height %2\n"
   //                  "avail_i_width %3 f_thermo width %4\n"
   //                  "avail_i_height %5 thermo height %6 title height %7" )
   //         .arg( max_width )
   //         .arg( max_height )
   //         .arg( avail_i_width )
   //         .arg( f_thermo_left ? f_thermo_left->width() : 0 )
   //         .arg( avail_i_height )
   //         .arg( f_thermo_top->height() )
   //         .arg( lbl_f_title->height() )
   //         );

   int avail_i = avail_i_height > avail_i_width ? avail_i_width : avail_i_height;

   int use_i_width  = avail_i + ( f_thermo_left ? f_thermo_left->width() : 0 );
   int use_i_height = avail_i + f_thermo_top->height() + lbl_f_title->height();
   int use_mode     = (int)cb_adj->isChecked() * 2 + (int)cb_hb->isChecked();
   
   if ( use_i_width  == last_width &&
        use_i_height == last_height &&
        use_mode     == last_mode ) {
      // us_qdebug( "skipped" );
      return;
   }

   last_width  = use_i_width;
   last_height = use_i_height;
   last_mode   = use_mode;

   // us_qdebug( QString( "top height %1" ).arg( f_thermo_top->height() ) );
   // if ( f_thermo_left ) {
   //    us_qdebug( QString( "left width %1" ).arg( f_thermo_left->width() ) );
   // }
   // clear out old pixmap
   {
      QPixmap pm;
      lbl_f_image->setPixmap( pm );
      // f_brookesmap->repaint();
      // qApp->processEvents( 1000 );
   }

   f_brookesmap->setMinimumWidth ( use_i_width );
   f_brookesmap->setMaximumWidth ( use_i_width );
   f_brookesmap->setMinimumHeight( use_i_height );
   f_brookesmap->setMaximumHeight( use_i_height );

   f_brookesmap->repaint();
#if QT_VERSION >= 0x040000
   qApp->processEvents( QEventLoop::AllEvents, 1000 );
#else
   qApp->processEvents( 1000 );
#endif

   // us_qdebug( QString( "use_i width %1 height %2" ).arg( use_i_height ).arg( use_i_width ) );
   // us_qdebug( QString( "lbl_f_image width %1 height %2" ).arg( lbl_f_image->width() ).arg( lbl_f_image->height() ) );

   {
      QPixmap pm;

      if ( parameters.count( "as_pairs" ) ) {
         lbl_f_title->setText( 
                              parameters.count( "ppvm_title" ) ?
                              parameters[ "ppvm_title" ] :
                              us_tr( "Pairwise P value map" ) 
                               );
         pm.convertFromImage( 
#if QT_VERSION >= 0x040000
                                qi->scaled( 
                                           QSize( lbl_f_image->width(), lbl_f_image->height() )
                                           // ,  Qt::KeepAspectRatio 
                                            )
#else
                                qi->smoothScale(
                                                lbl_f_image->width()
                                                ,lbl_f_image->height()
                                                )
#endif
                              );
      } else {
         if ( cb_adj->isChecked() ) {
            lbl_f_title->setText( 
                                 parameters.count( "ppvm_title_adj" ) ?
                                 parameters[ "ppvm_title_adj" ] :
                                 us_tr( "Pairwise adjusted P value map" ) 
                                  );
            pm.convertFromImage( 
#if QT_VERSION >= 0x040000
                                qi_adj->scaled( 
                                               QSize( avail_i, avail_i ),  Qt::KeepAspectRatio
                                                )
#else
                                qi_adj->smoothScale( 
                                                    avail_i
                                                    ,avail_i
                                                    // ,lbl_f_image->width()
                                                    // ,lbl_f_image->height()
                                                    // ,QImage::ScaleMin
                                                     ) 
#endif
                                 );
         } else { 
            if ( cb_hb->isChecked() ) {
               lbl_f_title->setText( 
                                    "Holm-Bonferroni adjusted\n" +
                                    (
                                     parameters.count( "ppvm_title" ) ?
                                     parameters[ "ppvm_title" ] :
                                     us_tr( "Pairwise P value map" ) 
                                     )
                                     );

               pm.convertFromImage( 
#if QT_VERSION >= 0x040000
                                   qi_hb->scaled( 
                                                 QSize( avail_i, avail_i ),  Qt::KeepAspectRatio
                                                  )
#else
                                   qi_hb->smoothScale( 
                                                      avail_i
                                                      ,avail_i
                                                      // ,lbl_f_image->width()
                                                      // ,lbl_f_image->height()
                                                      // ,QImage::ScaleMin
                                                       ) 
#endif
                                    );
            } else {
               lbl_f_title->setText( 
                                    parameters.count( "ppvm_title" ) ?
                                    parameters[ "ppvm_title" ] :
                                    us_tr( "Pairwise P value map" ) 
                                     );

               pm.convertFromImage( 
#if QT_VERSION >= 0x040000
                                   qi->scaled( 
                                              QSize( avail_i, avail_i ),  Qt::KeepAspectRatio
                                               )
#else
                                   qi->smoothScale( 
                                                   avail_i
                                                   ,avail_i
                                                   // ,lbl_f_image->width()
                                                   // ,lbl_f_image->height()
                                                   // ,QImage::ScaleMin
                                                    ) 
#endif
                                    );
            }
         }
      }
      lbl_f_image->setPixmap( pm );
   }

   f_brookesmap->repaint();
#if QT_VERSION >= 0x040000
   qApp->processEvents( QEventLoop::AllEvents, 1000 );
#else
   qApp->processEvents( 1000 );
#endif
   {
      QPixmap pm = f_brookesmap->grab();
      QImage qi = pm.toImage();
      pm.convertFromImage( 
#if QT_VERSION >= 0x040000
                          qi.scaled( 
                                     QSize( use_i_width, use_i_height ),  Qt::KeepAspectRatio
                                      )
#else
                          qi.smoothScale( 
                                         use_i_width
                                         ,use_i_height
                                         ,Qt::KeepAspectRatio
                                          )
#endif
                           );
      if ( org_geom != geometry() ) {
         // us_qdebug( "geometry changed" );
         last_width = last_height = -1;
         return; //  imageResized();
      }
      
      if ( org_lbl_geom != lbl_image->geometry() ) {
         // us_qdebug( "lbl geometry changed" );
         last_width = last_height = -1;
         return; //  imageResized();
      }
      
      lbl_image->setPixmap( pm );
   }
   // update();
   // repaint();
}

void US_Hydrodyn_Saxs_Cormap::usp_config_plot( const QPoint & ) {
   US_PlotChoices *uspc = new US_PlotChoices( usp_plot );
   uspc->exec();
   delete uspc;
}

void US_Hydrodyn_Saxs_Cormap::usp_config_plot_cluster( const QPoint & ) {
   US_PlotChoices *uspc = new US_PlotChoices( usp_plot_cluster );
   uspc->exec();
   delete uspc;
}
