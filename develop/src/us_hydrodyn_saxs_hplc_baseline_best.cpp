#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_saxs_util.h"
#include "../include/us_hydrodyn_saxs_hplc_baseline_best.h"

#include <qsplitter.h>
//Added by qt3to4:
#include <Q3TextStream>
#include <Q3HBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <Q3Frame>
#include <Q3PopupMenu>
#include <Q3VBoxLayout>
#include <QCloseEvent>

US_Hydrodyn_Saxs_Hplc_Baseline_Best::US_Hydrodyn_Saxs_Hplc_Baseline_Best(
                                                                         void                         *          us_hydrodyn,
                                                                         map < QString, QString >                parameters,
                                                                         map < QString, double >                 dparameters,
                                                                         map < QString, vector < double > >      vdparameters,
                                                                         QWidget *                               p,
                                                                         const char *                            name
                                                                         ) : Q3Frame( p, name )
{
   this->us_hydrodyn                          = us_hydrodyn;
   this->parameters                           = parameters;
   this->dparameters                          = dparameters;
   this->vdparameters                         = vdparameters;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setCaption( tr( "US-SOMO: HPLC SAXS Baseline Best Region Analysis" ) +
               ( parameters.count( "name" ) ? QString( " : %1" ).arg( parameters[ "name" ] ) : QString( "" ) ) );

   plot_zoomer         = ( ScrollZoomer * )0;

   setupGUI();
   ((US_Hydrodyn*)us_hydrodyn)->fixWinButtons( this );

   global_Xpos += 30;
   global_Ypos += 30;

   // global_Xpos = 0;
   // global_Ypos = 0;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );

   displayData();
}

US_Hydrodyn_Saxs_Hplc_Baseline_Best::~US_Hydrodyn_Saxs_Hplc_Baseline_Best()
{
}

#define UHSC_IMG_MIN 200
#define UHSC_IMG_MAX 768

void US_Hydrodyn_Saxs_Hplc_Baseline_Best::setupGUI()
{
   int minHeight1  = 30;

   QSplitter *qs  = new QSplitter( Qt::Vertical  , this );

   lbl_title = new QLabel( "", this ); 

   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette( PALET_LABEL );
   AUTFBACK( lbl_title );
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   plot = new QwtPlot( qs );
#ifndef QT4
   plot->enableGridXMin();
   plot->enableGridYMin();
#else
   plot_grid = new QwtPlotGrid;
   plot_grid->enableXMin( true );
   plot_grid->enableYMin( true );
#endif
   plot->setPalette( PALET_NORMAL );
   AUTFBACK( plot );
#ifndef QT4
   plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   plot_grid->setMajPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   plot_grid->setMinPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   plot_grid->attach( plot );
#endif
   plot->setAxisTitle(QwtPlot::xBottom, parameters.count( "xlegend" ) ? parameters[ "xlegend" ] : tr( "Start Frame" ) );
   plot->setAxisTitle(QwtPlot::yLeft  , tr( "Average Red Cluster Size" ) );
#ifndef QT4
   plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#ifndef QT4
   plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#ifndef QT4
   plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot->setMargin(USglobal->config_list.margin);
   plot->setTitle("");
#ifndef QT4
   plot->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::None);
#else
   plot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine );
#endif
   plot->setCanvasBackground(USglobal->global_colors.plot);

#ifndef QT4
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

   //   QFrame *editor_frame = new QFrame( qs );

   editor = new Q3TextEdit( qs );
   editor->setPalette( PALET_NORMAL );
   AUTFBACK( editor );
   editor->setReadOnly(true);
   editor->setFont( QFont( "Courier", USglobal->config_list.fontSize ) );

#if defined(QT4) && defined(Q_WS_MAC)
   {
      Q3PopupMenu * file = new Q3PopupMenu;
      file->insertItem( tr("&Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
      file->insertItem( tr("&Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );
      file->insertItem( tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );

      QMenuBar *menu = new QMenuBar( this );
      AUTFBACK( menu );

      menu->insertItem(tr("&Messages"), file );
   }
#else
   // QFrame *frame;
   // frame = new QFrame( qs );
   // frame->setMinimumHeight(minHeight1);

   // m = new QMenuBar(frame, "menu" );
   // m->setMinimumHeight(minHeight1 - 5);
   // m->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   // QPopupMenu * file = new QPopupMenu(editor);
   // m->insertItem( tr("&File"), file );
   // file->insertItem( tr("Font"),  this, SLOT(update_font()),    ALT+Key_F );
   // file->insertItem( tr("Save"),  this, SLOT(save()),    ALT+Key_S );
   // file->insertItem( tr("Clear Display"), this, SLOT(clear_display()),   ALT+Key_X );
#endif

   editor->setWordWrap (Q3TextEdit::WidgetWidth);
   editor->setMinimumHeight( minHeight1 * 9 );
   // editor->setTextFormat( Qt::RichText );

   pb_set_best =  new QPushButton ( tr( "Set region in HPLC window" ), this );
   pb_set_best -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_set_best -> setMinimumHeight( minHeight1 );
   pb_set_best -> setPalette      ( PALET_PUSHB );
   connect( pb_set_best, SIGNAL( clicked() ), SLOT( set_best() ) );
   
   if ( !((US_Hydrodyn *)us_hydrodyn)->saxs_hplc_widget ) {
      pb_set_best->hide();
   }

   pb_help =  new QPushButton ( tr( "Help" ), this );
   pb_help -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_help -> setMinimumHeight( minHeight1 );
   pb_help -> setPalette      ( PALET_PUSHB );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

   pb_close =  new QPushButton ( tr( "Close" ), this );
   pb_close -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_close -> setMinimumHeight( minHeight1 );
   pb_close -> setPalette      ( PALET_PUSHB );
   connect( pb_close, SIGNAL( clicked() ), SLOT( cancel() ) );


   Q3VBoxLayout *background = new Q3VBoxLayout( this );
   background->addWidget( lbl_title );

   //    QBoxLayout *vbl_editor_group = new QVBoxLayout(qs);
   // #if !defined(QT4) || !defined(Q_WS_MAC)
   //    vbl_editor_group->addWidget ( frame );
   // #endif
   //    vbl_editor_group->addWidget ( editor );

   background->addWidget( qs );
   background->addSpacing( 2 );
   background->addWidget( pb_set_best );

   Q3HBoxLayout *hbl_bottom = new Q3HBoxLayout( 0 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addWidget ( pb_close );

   background->addSpacing( 4 );
   background->addLayout ( hbl_bottom );
}

void US_Hydrodyn_Saxs_Hplc_Baseline_Best::cancel()
{
   close();
}

void US_Hydrodyn_Saxs_Hplc_Baseline_Best::help()
{
   US_Help *online_help;
   online_help = new US_Help( this );
   online_help->show_help("manual/somo/baseline_best.html");
}

void US_Hydrodyn_Saxs_Hplc_Baseline_Best::closeEvent( QCloseEvent *e )
{

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Saxs_Hplc_Baseline_Best::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->color();
   editor->setColor(color);
   editor->append(msg);
   editor->setColor(save_color);
}

void US_Hydrodyn_Saxs_Hplc_Baseline_Best::editor_ec_msg( QString msg )
{
   QColor save_color = editor->color();
   QStringList qsl = QStringList::split( "~~", msg );

   QRegExp rx_color( "^_(\\S+)_$" );

   map < QString, QColor > colors;
   colors[ "orange"      ] = QColor( 255, 165, 0 );
   colors[ "darkorange"  ] = QColor( 255, 140, 0 );
   colors[ "darkorange2" ] = QColor( 238, 118, 0 );
   colors[ "darkorange3" ] = QColor( 214, 106, 0 );

   for ( int i = 0; i < (int) qsl.size(); ++i ) {
      if ( rx_color.search( qsl[ i ] ) != -1 ) {
         QColor qc;
         if ( colors.count( rx_color.cap( 1 ) ) ) {
            qc = colors[ rx_color.cap( 1 ) ];
         } else {
            qc = QColor( rx_color.cap( 1 ) );
         }
         editor->setColor( qc );
      } else {
         editor->append( qsl[ i ] );
      }
   }

   editor->setColor(save_color);
}

void US_Hydrodyn_Saxs_Hplc_Baseline_Best::clear_display()
{
   editor->clear();
   editor->append("\n\n");
}

void US_Hydrodyn_Saxs_Hplc_Baseline_Best::update_font()
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

void US_Hydrodyn_Saxs_Hplc_Baseline_Best::save()
{
   QString fn;
   fn = QFileDialog::getSaveFileName( this , caption() , QString::null , QString::null );
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

void US_Hydrodyn_Saxs_Hplc_Baseline_Best::displayData() {

   editor      ->clear();
   plot        ->clear();

   lbl_title->setText( parameters.count( "title" ) ? parameters[ "title" ] : QString( tr( "Average red cluster size by start frame" ) ) );

   // plot data and print anything of importance in editor

   editor->append( "<a name=\"top\">" );
   if ( parameters.count( "msg" ) ) {
      editor_ec_msg( parameters[ "msg" ] );
   }
   editor->scrollToAnchor( "top" );

   // plot data

   if ( vdparameters.count( "x" ) &&
        vdparameters.count( "y" ) &&
        vdparameters[ "x" ].size() &&
        vdparameters[ "x" ].size() == vdparameters[ "y" ].size()
        ) {
      int use_line_width = parameters.count( "linewidth" ) ? parameters[ "linewidth" ].toInt() : 1;
      double maxy = 0e0;

      // US_Vector::printvector2( "baseline results", vdparameters[ "x" ], vdparameters[ "y" ] );

      double minyr = 0e0;
      double maxyr = -1e99;
      bool plotted_yright = false;

      if ( vdparameters.count( "sumqx" ) ) {
         plotted_yright = true;
         plot->enableAxis  ( QwtPlot::yRight , true );
         plot->setAxisTitle( QwtPlot::yRight, tr( "\nAverage I(q) [a.u.]" ) );

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
#ifndef QT4
               long curve;
               curve = plot->insertCurve( "baseline_best_sumq", QwtPlot::xBottom, QwtPlot::yRight );
               plot->setCurveStyle( curve, QwtCurve::Lines );
#else
               QwtPlotCurve *curve = new QwtPlotCurve( "baseline_best_sumq" );
               curve->setStyle( QwtPlotCurve::Lines );
#endif

#ifndef QT4
               plot->setCurvePen( curve, QPen( Qt::white, 2 * use_line_width, Qt::SolidLine ) );
               plot->setCurveData( curve,
                                   (double *)&vdparameters[ "sumqx" ][ 0 ],
                                   (double *)&vdparameters[ "sumqy" ][ 0 ],
                                   vdparameters[ "sumqx" ].size()
                                   );
#else
               curve->setPen( QPen( Qt::white, 2 * use_line_width, Qt::SolidLine ) );
               curve->setData(
                              (double *)&vdparameters[ "sumqx" ][ 0 ],
                              (double *)&vdparameters[ "sumqy" ][ 0 ],
                              vdparameters[ "sumqx" ].size()
                              );
               curve->setAxis  ( QwtPlot::xBottom , QwtPlot::yRight );
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

            QString curvename = QString( tr( "Baseline avg. I(q) for q less than %1\n(on right axis)" ) ).arg( dparameters[ "cormap_maxq" ] );
#ifndef QT4
            long curve;
            curve = plot->insertCurve( curvename, QwtPlot::xBottom, QwtPlot::yRight );
            plot->setCurveStyle( curve, QwtCurve::Lines );
#else
            QwtPlotCurve *curve = new QwtPlotCurve( curvename );
            curve->setStyle( QwtPlotCurve::Lines );
            curve->setItemAttribute( QwtPlotItem::Legend, true );
#endif

#ifndef QT4
            plot->setCurvePen( curve, QPen( QColor( 255, 165, 0 ) /* orange */, 2 * use_line_width, Qt::SolidLine ) );
            plot->setCurveData( curve,
                                (double *)&vdparameters[ "sumqx" ][ 0 ],
                                (double *)&vdparameters[ "sumqmaxqy" ][ 0 ],
                                vdparameters[ "sumqx" ].size()
                                );
#else
            curve->setPen( QPen( QColor( 255, 165, 0 ) /* orange */, 2 * use_line_width, Qt::SolidLine ) );
            curve->setData(
                           (double *)&vdparameters[ "sumqx" ][ 0 ],
                           (double *)&vdparameters[ "sumqmaxqy" ][ 0 ],
                           vdparameters[ "sumqx" ].size()
                           );
            curve->setAxis  ( QwtPlot::xBottom , QwtPlot::yRight );
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
                  QString curvename = QString( tr( "Baseline avg. I(q) ±1 SD for q less than %1\n(on right axis)" ) ).arg( dparameters[ "cormap_maxq" ] );
#ifndef QT4
                  long curve;
                  curve = plot->insertCurve( curvename, QwtPlot::xBottom, QwtPlot::yRight );
                  plot->setCurveStyle( curve, QwtCurve::Lines );
#else
                  QwtPlotCurve *curve = new QwtPlotCurve( curvename );
                  curve->setStyle( QwtPlotCurve::Lines );
                  curve->setItemAttribute( QwtPlotItem::Legend, true );
#endif
                  
#ifndef QT4
                  plot->setCurvePen( curve, QPen( QColor( 255, 165, 0 ) /* orange */, 2 * use_line_width, Qt::DotLine ) );
                  plot->setCurveData( curve, 
                                      (double *)&vdparameters[ "sumqx" ][ 0 ],
                                      (double *)&vdparameters[ "sumqmaxqysdm" ][ 0 ],
                                      vdparameters[ "sumqx" ].size()
                                      );
#else
                  curve->setPen( QPen( QColor( 255, 165, 0 ) /* orange */, 2 * use_line_width, Qt::DotLine ) );
                  curve->setData(
                                 (double *)&vdparameters[ "sumqx" ][ 0 ],
                                 (double *)&vdparameters[ "sumqmaxqysdm" ][ 0 ],
                                 vdparameters[ "sumqx" ].size()
                                 );
                  curve->setAxis  ( QwtPlot::xBottom , QwtPlot::yRight );
                  curve->attach( plot );
#endif
               }
               {
                  QString curvename = QString( tr( "Baseline avg. I(q) ±1 SD for q less than %1\n(on right axis)" ) ).arg( dparameters[ "cormap_maxq" ] );
#ifndef QT4
                  long curve;
                  curve = plot->insertCurve( curvename, QwtPlot::xBottom, QwtPlot::yRight );
                  plot->setCurveStyle( curve, QwtCurve::Lines );
                  plot->enableLegend( false, curve );
#else
                  QwtPlotCurve *curve = new QwtPlotCurve( curvename );
                  curve->setStyle( QwtPlotCurve::Lines );
                  curve->setItemAttribute( QwtPlotItem::Legend, false );
#endif
                  
#ifndef QT4
                  plot->setCurvePen( curve, QPen( QColor( 255, 165, 0 ) /* orange */, 2 * use_line_width, Qt::DotLine ) );
                  plot->setCurveData( curve, 
                                      (double *)&vdparameters[ "sumqx" ][ 0 ],
                                      (double *)&vdparameters[ "sumqmaxqysdp" ][ 0 ],
                                      vdparameters[ "sumqx" ].size()
                                      );
#else
                  curve->setPen( QPen( QColor( 255, 165, 0 ) /* orange */, 2 * use_line_width, Qt::DotLine ) );
                  curve->setData(
                                 (double *)&vdparameters[ "sumqx" ][ 0 ],
                                 (double *)&vdparameters[ "sumqmaxqysdp" ][ 0 ],
                                 vdparameters[ "sumqx" ].size()
                                 );
                  curve->setAxis  ( QwtPlot::xBottom , QwtPlot::yRight );
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
#ifndef QT4
            long curve;
            curve = plot->insertCurve( curvename, QwtPlot::xBottom, QwtPlot::yRight );
            plot->setCurveStyle( curve, QwtCurve::Lines );
#else
            QwtPlotCurve *curve = new QwtPlotCurve( curvename );
            curve->setStyle( QwtPlotCurve::Lines );
            curve->setItemAttribute( QwtPlotItem::Legend, true );
#endif

#ifndef QT4
            plot->setCurvePen( curve, QPen( Qt::magenta, 2 * use_line_width, Qt::SolidLine ) );
            plot->setCurveData( curve, x, y, 2 );
#else
            curve->setPen( QPen( Qt::magenta, 2 * use_line_width, Qt::SolidLine ) );
            curve->setData( x, y, 2 );
            curve->setAxis  ( QwtPlot::xBottom , QwtPlot::yRight );
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
               QString curvename = "Average ±1 SD of total blanks intensity\n(on right axis)";
#ifndef QT4
               long curve;
               curve = plot->insertCurve( curvename, QwtPlot::xBottom, QwtPlot::yRight );
               plot->setCurveStyle( curve, QwtCurve::Lines );
#else
               QwtPlotCurve *curve = new QwtPlotCurve( curvename );
               curve->setStyle( QwtPlotCurve::Lines );
               curve->setItemAttribute( QwtPlotItem::Legend, true );
#endif

#ifndef QT4
               plot->setCurvePen( curve, QPen( Qt::magenta, 2 * use_line_width, Qt::DotLine ) );
               plot->setCurveData( curve, x, y, 2 );
#else
               curve->setPen( QPen( Qt::magenta, 2 * use_line_width, Qt::DotLine ) );
               curve->setData( x, y, 2 );
               curve->setAxis  ( QwtPlot::xBottom , QwtPlot::yRight );
               curve->attach( plot );
#endif
            }
            {
               QString curvename = "Average -1 SD of total blanks intensity\n(on right axis)";
               y[ 0 ] = y[ 1 ] = -dparameters[ "blanks_avg_maxq_sd" ];
#ifndef QT4
               long curve;
               curve = plot->insertCurve( curvename, QwtPlot::xBottom, QwtPlot::yRight );
               plot->setCurveStyle( curve, QwtCurve::Lines );
               plot->enableLegend( false, curve );
#else
               QwtPlotCurve *curve = new QwtPlotCurve( curvename );
               curve->setStyle( QwtPlotCurve::Lines );
               curve->setItemAttribute( QwtPlotItem::Legend, false );
#endif

#ifndef QT4
               plot->setCurvePen( curve, QPen( Qt::magenta, 2 * use_line_width, Qt::DotLine ) );
               plot->setCurveData( curve, x, y, 2 );
#else
               curve->setPen( QPen( Qt::magenta, 2 * use_line_width, Qt::DotLine ) );
               curve->setData( x, y, 2 );
               curve->setAxis  ( QwtPlot::xBottom , QwtPlot::yRight );
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

         QString curvename = "Baseline avg. red cluster size\nabove blanks' avg. +1 SD";
#ifndef QT4
         long curve;
         curve = plot->insertCurve( curvename );
         plot->setCurveStyle( curve, QwtCurve::Sticks );
#else
         QwtPlotCurve *curve = new QwtPlotCurve( curvename );
         curve->setStyle( QwtPlotCurve::Sticks );
         curve->setItemAttribute( QwtPlotItem::Legend, true );
#endif

#ifndef QT4
         plot->setCurvePen( curve, QPen( Qt::red, 2 * use_line_width, Qt::SolidLine ) );
         plot->setCurveData( curve,
                             (double *)&vdparameters[ "x" ][ 0 ],
                             (double *)&vdparameters[ "y" ][ 0 ],
                             vdparameters[ "x" ].size()
                             );
         plot->setCurveBaseline( curve, -0.5 );
#else
         curve->setPen( QPen( Qt::red, 2 * use_line_width, Qt::SolidLine ) );
         curve->setData(
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

         QString curvename = "Baseline best avg. red cluster size\nabove blanks' avg. +1 SD";
#ifndef QT4
         long curve;
         curve = plot->insertCurve( curvename );
         plot->setCurveStyle( curve, QwtCurve::Sticks );
#else
         QwtPlotCurve *curve = new QwtPlotCurve( curvename );
         curve->setStyle( QwtPlotCurve::Sticks );
         curve->setItemAttribute( QwtPlotItem::Legend, true );
#endif

#ifndef QT4
         plot->setCurvePen( curve, QPen( Qt::yellow, 2 * use_line_width, Qt::SolidLine ) );
         plot->setCurveData( curve,
                             (double *)&vdparameters[ "yx" ][ 0 ],
                             (double *)&vdparameters[ "yy" ][ 0 ],
                             vdparameters[ "yx" ].size()
                             );
         plot->setCurveBaseline( curve, -0.5 );
#else
         curve->setPen( QPen( Qt::yellow, 2 * use_line_width, Qt::SolidLine ) );
         curve->setData(
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
#ifndef QT4
         long curve;
         curve = plot->insertCurve( curvename );
         plot->setCurveStyle( curve, QwtCurve::Sticks );
#else
         QwtPlotCurve *curve = new QwtPlotCurve( curvename );
         curve->setStyle( QwtPlotCurve::Sticks );
         curve->setItemAttribute( QwtPlotItem::Legend, true );
#endif

#ifndef QT4
         plot->setCurvePen( curve, QPen( Qt::green, 2 * use_line_width, Qt::SolidLine ) );
         plot->setCurveData( curve,
                             (double *)&vdparameters[ "gx" ][ 0 ],
                             (double *)&vdparameters[ "gy" ][ 0 ],
                             vdparameters[ "gx" ].size()
                             );
         plot->setCurveBaseline( curve, -0.5 );
#else
         curve->setPen( QPen( Qt::green, 2 * use_line_width, Qt::SolidLine ) );
         curve->setData(
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

         QString curvename = "Baseline best avg. red cluster size\nequal or below blanks' avg. +1 SD";
#ifndef QT4
         long curve;
         curve = plot->insertCurve( curvename );
         plot->setCurveStyle( curve, QwtCurve::Sticks );
#else
         QwtPlotCurve *curve = new QwtPlotCurve( curvename );
         curve->setStyle( QwtPlotCurve::Sticks );
         curve->setItemAttribute( QwtPlotItem::Legend, true );
#endif

#ifndef QT4
         plot->setCurvePen( curve, QPen( Qt::white, 2 * use_line_width, Qt::SolidLine ) );
         plot->setCurveData( curve,
                             (double *)&vdparameters[ "wx" ][ 0 ],
                             (double *)&vdparameters[ "wy" ][ 0 ],
                             vdparameters[ "wx" ].size()
                             );
         plot->setCurveBaseline( curve, -0.5 );
#else
         curve->setPen( QPen( Qt::white, 2 * use_line_width, Qt::SolidLine ) );
         curve->setData(
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

         QString curvename = tr( "Blanks' avg. red cluster size" );

#ifndef QT4
         long curve;
         curve = plot->insertCurve( curvename );
         plot->setCurveStyle( curve, QwtCurve::Lines );
#else
         QwtPlotCurve *curve = new QwtPlotCurve( curvename );
         curve->setStyle( QwtPlotCurve::Lines );
         curve->setItemAttribute( QwtPlotItem::Legend, true );
#endif

#ifndef QT4
         plot->setCurvePen( curve, QPen( Qt::cyan, 2 * use_line_width, Qt::SolidLine ) );
         plot->setCurveData( curve, x, y, 2 );
#else
         curve->setPen( QPen( Qt::cyan, 2 * use_line_width, Qt::SolidLine ) );
         curve->setData( x, y, 2 );
         curve->attach( plot );
#endif

         if ( dparameters.count( "blanksaveragesd" ) ) {

            y[ 0 ] = y[ 1 ] = y[ 0 ] + dparameters[ "blanksaveragesd" ];

            QString curvename = tr( "Blanks' avg. red cluster size +1 SD" );
#ifndef QT4
            long curve;
            curve = plot->insertCurve( curvename );
            plot->setCurveStyle( curve, QwtCurve::Lines );
#else
            QwtPlotCurve *curve = new QwtPlotCurve( curvename );
            curve->setStyle( QwtPlotCurve::Lines );
            curve->setItemAttribute( QwtPlotItem::Legend, true );
#endif

#ifndef QT4
            plot->setCurvePen( curve, QPen( Qt::cyan, 2 * use_line_width, Qt::DotLine ) );
            plot->setCurveData( curve, x, y, 2 );
#else
            curve->setPen( QPen( Qt::cyan, 2 * use_line_width, Qt::DotLine ) );
            curve->setData( x, y, 2 );
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
            
         plot_zoomer = new ScrollZoomer(plot->canvas());
         plot_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
#ifndef QT4
         plot_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
         // connect( plot_zoomer, SIGNAL( zoomed( const QwtDoubleRect & ) ), SLOT( plot_zoomed( const QwtDoubleRect & ) ) );
      }
      
   }

#ifdef QT4
   {
       QwtLegend* legend_pd = new QwtLegend;
       // QPalette mp = PALET_NORMAL;
       // mp.setColor( QPalette::Normal, QPalette::Window, Qt::black );
       // this doesn't seem to work: mp.setColor( QPalette::Normal, QPalette::WindowText, Qt::white );
       // legend_pd->contentsWidget()->setPalette( mp );
       // AUTFBACK( legend_pd );
       legend_pd->setFrameStyle( Q3Frame::Box | Q3Frame::Sunken );
       plot->insertLegend( legend_pd, QwtPlot::RightLegend );
   }
   plot->legend()->setVisible( true );
#endif

   plot->replot();

   if ( !vdparameters.count( "bestpos" ) ||
        !dparameters.count( "width" ) ) {
      qDebug( "no bestpos or width" );
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
      QPixmap qm = QPixmap::grabWidget( this );
      qm.save( parameters[ "save_png" ], "PNG" );
   }
   if ( parameters.count( "close" ) ) {
      qApp->processEvents();
      this->close();
   }
}

void US_Hydrodyn_Saxs_Hplc_Baseline_Best::set_best() {
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_hplc_widget ) {
      US_Hydrodyn_Saxs_Hplc *hplc_win = ((US_Hydrodyn *)us_hydrodyn)->saxs_hplc_window;

      if ( hplc_win->current_mode == US_Hydrodyn_Saxs_Hplc::MODE_BASELINE &&
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
               QString choice = QInputDialog::getItem(
                                                      caption() + tr( " : Choose starting position for baseline" )
                                                      ,tr( "There are multiple equivalent starting positions for the integral baseline window\n"
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

         hplc_win->le_baseline_end_s->setText( QString( "%1" ).arg( bestpos ) );
         hplc_win->le_baseline_end_e->setText( QString( "%1" ).arg( bestpos + dparameters[ "width" ] - 1 ) );
         hplc_win->le_baseline_width->setText( QString( "%1" ).arg( dparameters[ "width" ] ) );
         hplc_win->cb_baseline_fix_width->setChecked( true );
      }
   }
}

