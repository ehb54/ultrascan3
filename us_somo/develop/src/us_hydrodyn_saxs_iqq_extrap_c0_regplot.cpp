#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_saxs_util.h"
#include "../include/us_hydrodyn_saxs_iqq_extrap_c0_regplot.h"
//Added by qt3to4:
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCloseEvent>
#include <QFrame>
#include <QLabel>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDir>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QImageReader>
#include <qwt_symbol.h>

// ffmpeg for Make movie: next to the SOMO program first, then on the PATH, then where the macOS
// package managers put it (a program started from the Finder does not get the shell's PATH)
static QString us_extrap_c0_find_ffmpeg()
{
   QString exe = QStandardPaths::findExecutable( "ffmpeg", QStringList() << QCoreApplication::applicationDirPath() );
   if ( exe.isEmpty() )
   {
      exe = QStandardPaths::findExecutable( "ffmpeg" );
   }
   if ( exe.isEmpty() )
   {
      exe = QStandardPaths::findExecutable( "ffmpeg", QStringList() << "/opt/homebrew/bin" << "/usr/local/bin" << "/opt/local/bin" );
   }
   return exe;
}

US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot(
                                                                                void *                          us_hydrodyn,
                                                                                QString                         y_axis_title,
                                                                                double                          merge_q,
                                                                                int                             fit_broaden,
                                                                                int                             model,
                                                                                vector < double >               reg_q,
                                                                                vector < vector < double > >    reg_x,
                                                                                vector < vector < double > >    reg_y,
                                                                                vector < vector < double > >    reg_e,
                                                                                vector < double >               reg_a,
                                                                                vector < double >               reg_b,
                                                                                vector < double >               reg_c,
                                                                                vector < double >               reg_siga,
                                                                                double                          excl_conc,
                                                                                vector < double >               reg_excl_y,
                                                                                QString                         excl_name,
                                                                                vector < vector < int > >       reg_ci,
                                                                                vector < vector < double > >    reg_sx,
                                                                                vector < vector < double > >    reg_sy,
                                                                                vector < vector < double > >    reg_se,
                                                                                vector < vector < int > >       reg_sci,
                                                                                US_Hydrodyn_Saxs_Extrap_C0_Pending pending,
                                                                                void *                          saxs_window,
                                                                                QWidget *                       p,
                                                                                const char *
                                                                                ) : QFrame( p )
{
   this->us_hydrodyn  = us_hydrodyn;
   this->y_axis_title = y_axis_title;
   this->merge_q      = merge_q;
   this->fit_broaden  = fit_broaden;
   this->model        = model;
   this->reg_q        = reg_q;
   this->reg_x        = reg_x;
   this->reg_y        = reg_y;
   this->reg_e        = reg_e;
   this->reg_a        = reg_a;
   this->reg_b        = reg_b;
   this->reg_c        = reg_c;
   this->reg_siga     = reg_siga;
   this->excl_conc    = excl_conc;
   this->reg_excl_y   = reg_excl_y;
   this->excl_name    = excl_name;
   this->reg_ci       = reg_ci;
   this->reg_sx       = reg_sx;
   this->reg_sy       = reg_sy;
   this->reg_se       = reg_se;
   this->reg_sci      = reg_sci;
   this->pending      = pending;
   this->saxs_window  = saxs_window;
   pending_open       = ( saxs_window != 0 );

   cur_index   = 0;
   plot_zoomer = (ScrollZoomer *) 0;

   n_frames_saved = 0;
   n_frames_kept  = 0;
   movie_timer_ms = 250;
   US_Hydrodyn * ush = (US_Hydrodyn *) us_hydrodyn;
   if ( ush && ush->gparams.count( "extrap_c0_movie_timer_ms" ) )
   {
      movie_timer_ms = qBound( 50, ush->gparams[ "extrap_c0_movie_timer_ms" ].toInt(), 5000 );
   }

   making_movie            = false;
   encode_cancelled        = false;
   save_frames_before_make = false;
   ffmpeg_exe              = us_extrap_c0_find_ffmpeg();
   ffmpeg_proc             = new QProcess( this );
   ffmpeg_proc->setProcessChannelMode( QProcess::MergedChannels );
   connect( ffmpeg_proc, SIGNAL( finished( int, QProcess::ExitStatus ) ), SLOT( encode_done( int, QProcess::ExitStatus ) ) );
   connect( ffmpeg_proc, SIGNAL( errorOccurred( QProcess::ProcessError ) ), SLOT( encode_error( QProcess::ProcessError ) ) );

   movie_timer = new QTimer( this );
   connect( movie_timer, SIGNAL( timeout() ), SLOT( movie_tick() ) );

   USglobal = new US_Config();
   // top-level window, not a child widget embedded in the SAS window
   setWindowFlags( Qt::Window );
   setPalette( PALET_FRAME );
   setWindowTitle( us_tr( "US-SOMO: SAXS: Extrapolation to zero concentration - per-q regression" ) );

   setupGUI();

   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry( global_Xpos, global_Ypos, 760, 700 );

   set_index( 0 );
}

US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::~US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot()
{
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::setupGUI()
{
   int minHeight1 = 30;

   QString info_txt =
      ( merge_q > 0e0 )
      ? us_tr( "Each point is one input curve at its concentration; the line is the linear "
               "fit whose c=0 intercept is the extrapolated value. Below the merging point "
               "the output is this intercept; at/above it the reference curve is used.\n"
               "Drag the wheel to scroll q; use Prev/Next to step one q at a time." )
      : us_tr( "Each point is one input curve at its concentration; the line is the linear "
               "fit whose intercept at c=0 is the extrapolated value.\n"
               "Drag the wheel to scroll q; use Prev/Next to step one q at a time." );
   if ( fit_broaden > 1 )
   {
      info_txt += QString( us_tr( "\nFit broadening on: the fit line's slope is smoothed over a %1-point q-window." ) )
         .arg( fit_broaden );
   }
   if ( excl_conc > 0e0 && !excl_name.isEmpty() )
   {
      info_txt += QString( us_tr( "\nOutlier QC: curve \"%1\" (conc %2) was excluded from the fit and is shown as a red x." ) )
         .arg( excl_name ).arg( excl_conc );
   }
   {
      int n_spk = 0;
      for ( int i = 0; i < (int) reg_sx.size(); i++ )
      {
         n_spk += (int) reg_sx[ i ].size();
      }
      if ( n_spk )
      {
         info_txt += QString( us_tr( "\nSpike check: %1 point(s) were fitted at their neighbours' local-fit value (cyan); "
                                     "their measured values are shown in magenta." ) ).arg( n_spk );
      }
   }
   info_txt += us_tr( "\nMovie: [] starts or stops stepping through the q range set below (S- / S+ change the speed); "
                      "with Save frames on,\neach q shown is also saved as an image. "
                      "Make movie saves the range's frames and joins them into a video (needs ffmpeg)." );
   if ( pending_open )
   {
      info_txt += us_tr( "\nKeep adds the extrapolated curve to the SAS plot (closing this window does the same); "
                         "Discard drops it. Save CSV writes these per-q fits to a file." );
   }
   lbl_info = new QLabel( info_txt, this );
   lbl_info->setAlignment( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_info->setMinimumHeight( minHeight1 );
   lbl_info->setPalette( PALET_LABEL );
   AUTFBACK( lbl_info );
   lbl_info->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );

   usp_plot = new US_Plot( plot, "", "", "", this );
   connect( (QWidget *)plot->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot( const QPoint & ) ) );
   ((QWidget *)plot->titleLabel())->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot->axisWidget( QwtPlot::yLeft ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot( const QPoint & ) ) );
   ((QWidget *)plot->axisWidget( QwtPlot::yLeft ))->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot->axisWidget( QwtPlot::xBottom ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot( const QPoint & ) ) );
   ((QWidget *)plot->axisWidget( QwtPlot::xBottom ))->setContextMenuPolicy( Qt::CustomContextMenu );

   grid = new QwtPlotGrid;
   grid->enableXMin( true );
   grid->enableYMin( true );
   plot->setPalette( PALET_NORMAL );
   AUTFBACK( plot );
   grid->setMajorPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid->setMinorPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid->attach( plot );

   plot->setAxisTitle( QwtPlot::xBottom, us_tr( "concentration [mg/mL]" ) );
   plot->setAxisTitle( QwtPlot::yLeft, y_axis_title );
   plot->setAxisFont( QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   plot->setAxisFont( QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   plot->setCanvasBackground( USglobal->global_colors.plot );

   double wheel_max = reg_q.size() ? (double)( reg_q.size() - 1 ) : 0e0;
   qwtw_wheel = new QwtWheel( this );
   qwtw_wheel->setOrientation( Qt::Horizontal );
   qwtw_wheel->setRange( 0, wheel_max );
   // continuous/fine so dragging scrolls smoothly across the whole q range (the
   // plot snaps to the nearest q); Prev/Next do the integer stepping
   qwtw_wheel->setSingleStep( wheel_max > 0e0 ? wheel_max / 1000e0 : 1e0 );
   qwtw_wheel->setMass( 1e0 );                 // inertia: a flick keeps spinning
   qwtw_wheel->setTracking( true );
   qwtw_wheel->setMinimumHeight( minHeight1 );
   qwtw_wheel->setMinimumWidth( 200 );
   qwtw_wheel->setToolTip( us_tr( "Drag to scroll smoothly through the q grid" ) );
   connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( wheel_changed( double ) ) );

   lbl_results = new QLabel( "", this );
   lbl_results->setAlignment( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_results->setMinimumHeight( minHeight1 );
   lbl_results->setPalette( PALET_LABEL );
   AUTFBACK( lbl_results );
   lbl_results->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   pb_prev = new QPushButton( us_tr( "Prev" ), this );
   pb_prev->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );
   pb_prev->setMinimumHeight( minHeight1 );
   pb_prev->setPalette( PALET_PUSHB );
   connect( pb_prev, SIGNAL( clicked() ), SLOT( prev() ) );

   pb_next = new QPushButton( us_tr( "Next" ), this );
   pb_next->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );
   pb_next->setMinimumHeight( minHeight1 );
   pb_next->setPalette( PALET_PUSHB );
   connect( pb_next, SIGNAL( clicked() ), SLOT( next() ) );

   // movie controls (the same layout as the HPLC/KIN movie)
   pb_movie_front = new QPushButton( "|<", this );
   pb_movie_front->setToolTip( us_tr( "Go to the first q of the movie range" ) );
   connect( pb_movie_front, SIGNAL( clicked() ), SLOT( movie_front() ) );

   pb_movie_slower = new QPushButton( "S-", this );
   pb_movie_slower->setToolTip( us_tr( "Go slower" ) );
   connect( pb_movie_slower, SIGNAL( clicked() ), SLOT( movie_slower() ) );

   pb_movie_start = new QPushButton( "[]", this );
   pb_movie_start->setToolTip( us_tr( "Start [] or stop || the movie: steps through the q range from the current point to\n"
                                      "the end of the range (from its start if outside it); || also stops Make movie" ) );
   connect( pb_movie_start, SIGNAL( clicked() ), SLOT( movie_start() ) );

   pb_movie_faster = new QPushButton( "S+", this );
   pb_movie_faster->setToolTip( us_tr( "Go faster" ) );
   connect( pb_movie_faster, SIGNAL( clicked() ), SLOT( movie_faster() ) );

   pb_movie_end = new QPushButton( ">|", this );
   pb_movie_end->setToolTip( us_tr( "Go to the last q of the movie range" ) );
   connect( pb_movie_end, SIGNAL( clicked() ), SLOT( movie_end() ) );

   QPushButton * movie_buttons[] = { pb_movie_front, pb_movie_slower, pb_movie_start, pb_movie_faster, pb_movie_end };
   for ( int k = 0; k < 5; k++ )
   {
      movie_buttons[ k ]->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
      movie_buttons[ k ]->setMinimumHeight( minHeight1 );
      movie_buttons[ k ]->setPalette( PALET_PUSHB );
   }

   lbl_movie_state = new QLabel( "", this );
   lbl_movie_state->setAlignment( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_movie_state->setMinimumHeight( minHeight1 );
   lbl_movie_state->setMinimumWidth( 200 );
   lbl_movie_state->setPalette( PALET_LABEL );
   AUTFBACK( lbl_movie_state );
   lbl_movie_state->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold ) );

   cb_save_frames = new QCheckBox( this );
   cb_save_frames->setText( us_tr( "Save frames  Prefix:" ) );
   cb_save_frames->setChecked( false );
   cb_save_frames->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_save_frames->setPalette( PALET_NORMAL );
   AUTFBACK( cb_save_frames );
   cb_save_frames->setToolTip( us_tr( "Save every q shown (by the movie, Prev/Next or the wheel) as an image of the plot and\n"
                                      "its results line, numbered by the q point, in a folder <prefix>_frames next to the\n"
                                      "input curves. Make movie saves the frames of the q range and joins them into a video." ) );
   connect( cb_save_frames, SIGNAL( clicked() ), SLOT( set_save_frames() ) );

   le_save_prefix = new QLineEdit( this );
   le_save_prefix->setText( pending.base_name.isEmpty() ? QString( "extrap_c0" ) : pending.base_name );
   le_save_prefix->setPalette( PALET_NORMAL );
   AUTFBACK( le_save_prefix );
   le_save_prefix->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   le_save_prefix->setMinimumHeight( minHeight1 );
   le_save_prefix->setToolTip( us_tr( "Prefix for the frame files, their folder (<prefix>_frames) and the movie (<prefix>.mp4)" ) );

   QString fmt = "png";
   {
      US_Hydrodyn * ush = (US_Hydrodyn *) us_hydrodyn;
      if ( ush && ush->gparams.count( "extrap_c0_movie_format" ) )
      {
         fmt = ush->gparams[ "extrap_c0_movie_format" ];
      }
   }
   rb_save_png = new QRadioButton( us_tr( "png" ), this );
   rb_save_png->setToolTip( us_tr( "Save as PNG files" ) );
   rb_save_jpeg = new QRadioButton( us_tr( "jpeg" ), this );
   rb_save_jpeg->setToolTip( us_tr( "Save as JPEG files" ) );
   rb_save_bmp = new QRadioButton( us_tr( "bmp" ), this );
   rb_save_bmp->setToolTip( us_tr( "Save as BMP files" ) );
   QRadioButton * fmt_buttons[] = { rb_save_png, rb_save_jpeg, rb_save_bmp };
   for ( int k = 0; k < 3; k++ )
   {
      fmt_buttons[ k ]->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
      fmt_buttons[ k ]->setPalette( PALET_NORMAL );
      AUTFBACK( fmt_buttons[ k ] );
   }
   rb_save_png ->setChecked( fmt != "jpeg" && fmt != "bmp" );
   rb_save_jpeg->setChecked( fmt == "jpeg" );
   rb_save_bmp ->setChecked( fmt == "bmp" );

   cb_save_overwrite = new QCheckBox( this );
   cb_save_overwrite->setText( us_tr( "Overwrite" ) );
   cb_save_overwrite->setChecked( false );
   cb_save_overwrite->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_save_overwrite->setPalette( PALET_NORMAL );
   AUTFBACK( cb_save_overwrite );
   cb_save_overwrite->setToolTip( us_tr( "Replace frame files, and the movie file of Make movie, that already exist\n"
                                         "(otherwise existing frames are kept and an existing movie is not replaced)" ) );

   // the q range of the movie and of Make movie: q from .. to, every n-th q point
   QLabel * lbl_q_from = new QLabel( us_tr( "Movie range:  q from" ), this );
   QLabel * lbl_q_to   = new QLabel( us_tr( "to" ), this );
   QLabel * lbl_every  = new QLabel( us_tr( "every" ), this );
   QLabel * lbl_every2 = new QLabel( us_tr( "q point(s)" ), this );
   QLabel * lbl_fps    = new QLabel( us_tr( "at" ), this );
   QLabel * range_labels[] = { lbl_q_from, lbl_q_to, lbl_every, lbl_every2, lbl_fps };
   for ( int k = 0; k < 5; k++ )
   {
      range_labels[ k ]->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
      range_labels[ k ]->setPalette( PALET_NORMAL );
      AUTFBACK( range_labels[ k ] );
   }

   le_movie_q_from = new QLineEdit( this );
   le_movie_q_from->setText( reg_q.size() ? QString::number( reg_q.front(), 'g', 6 ) : QString( "" ) );
   le_movie_q_from->setToolTip( us_tr( "First q of the movie range (the nearest q point at or above it is used)" ) );
   le_movie_q_to = new QLineEdit( this );
   le_movie_q_to->setText( reg_q.size() ? QString::number( reg_q.back(), 'g', 6 ) : QString( "" ) );
   le_movie_q_to->setToolTip( us_tr( "Last q of the movie range (the nearest q point at or below it is used)" ) );
   QLineEdit * range_edits[] = { le_movie_q_from, le_movie_q_to };
   for ( int k = 0; k < 2; k++ )
   {
      range_edits[ k ]->setPalette( PALET_NORMAL );
      AUTFBACK( range_edits[ k ] );
      range_edits[ k ]->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
      range_edits[ k ]->setMinimumHeight( minHeight1 );
      range_edits[ k ]->setMaximumWidth( 100 );
      connect( range_edits[ k ], SIGNAL( textChanged( const QString & ) ), SLOT( update_range_info() ) );
   }

   sb_movie_every = new QSpinBox( this );
   sb_movie_every->setRange( 1, qMax( 1, (int) reg_q.size() ) );
   sb_movie_every->setValue( 1 );
   sb_movie_every->setToolTip( us_tr( "Step through every n-th q point of the range" ) );
   sb_movie_every->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   sb_movie_every->setMinimumHeight( minHeight1 );
   sb_movie_every->setPalette( PALET_NORMAL );
   AUTFBACK( sb_movie_every );
   connect( sb_movie_every, SIGNAL( valueChanged( int ) ), SLOT( update_range_info() ) );

   lbl_movie_range_info = new QLabel( "", this );
   lbl_movie_range_info->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   lbl_movie_range_info->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_movie_range_info );

   US_Hydrodyn * ushp = (US_Hydrodyn *) us_hydrodyn;

   pb_make_movie = new QPushButton( ffmpeg_exe.isEmpty() ? us_tr( "Make movie (ffmpeg not found)" ) : us_tr( "Make movie" ), this );
   pb_make_movie->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   pb_make_movie->setMinimumHeight( minHeight1 );
   pb_make_movie->setPalette( PALET_PUSHB );
   pb_make_movie->setToolTip(
      ffmpeg_exe.isEmpty()
      ? us_tr( "Make movie runs ffmpeg, which was not found next to the SOMO program, on the PATH, or in\n"
               "/opt/homebrew/bin, /usr/local/bin or /opt/local/bin. Install ffmpeg to make movies here;\n"
               "Save frames works without it." )
      : QString( us_tr( "Save the frames of the movie range (existing ones are kept unless Overwrite is checked)\n"
                        "and join them into <prefix>.mp4 next to the input curves, using\n%1\n"
                        "|| stops it." ) ).arg( ffmpeg_exe ) );
   pb_make_movie->setEnabled( !ffmpeg_exe.isEmpty() );
   connect( pb_make_movie, SIGNAL( clicked() ), SLOT( make_movie() ) );

   sb_movie_fps = new QSpinBox( this );
   sb_movie_fps->setRange( 1, 60 );
   sb_movie_fps->setValue( ( ushp && ushp->gparams.count( "extrap_c0_movie_fps" ) )
                           ? qBound( 1, ushp->gparams[ "extrap_c0_movie_fps" ].toInt(), 60 ) : 10 );
   sb_movie_fps->setSuffix( us_tr( " frames/s" ) );
   sb_movie_fps->setToolTip( us_tr( "Frames per second of the movie file" ) );
   sb_movie_fps->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   sb_movie_fps->setMinimumHeight( minHeight1 );
   sb_movie_fps->setPalette( PALET_NORMAL );
   AUTFBACK( sb_movie_fps );
   sb_movie_fps->setEnabled( !ffmpeg_exe.isEmpty() );

   cb_remove_frames = new QCheckBox( this );
   cb_remove_frames->setText( us_tr( "Remove frames afterwards" ) );
   cb_remove_frames->setChecked( ushp && ushp->gparams.count( "extrap_c0_movie_remove_frames" )
                                 && ushp->gparams[ "extrap_c0_movie_remove_frames" ] == "1" );
   cb_remove_frames->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_remove_frames->setPalette( PALET_NORMAL );
   AUTFBACK( cb_remove_frames );
   cb_remove_frames->setToolTip( us_tr( "Once the movie is made, delete the frame files it was made from, and the\n"
                                        "<prefix>_frames folder if nothing else is left in it" ) );
   cb_remove_frames->setEnabled( !ffmpeg_exe.isEmpty() );

   pb_help = new QPushButton( us_tr( "Help" ), this );
   pb_help->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );
   pb_help->setMinimumHeight( minHeight1 );
   pb_help->setPalette( PALET_PUSHB );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

   pb_csv = new QPushButton( us_tr( "Save CSV" ), this );
   pb_csv->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );
   pb_csv->setMinimumHeight( minHeight1 );
   pb_csv->setPalette( PALET_PUSHB );
   pb_csv->setToolTip( us_tr( "Write the per-q fits to a CSV file: for every q the points used (per curve: value on the\n"
                              "fit axis, its error, and whether it was replaced as a spike), the fit coefficients, the\n"
                              "fit's R\302\262 and \317\207\302\262, and the extrapolated value with its error." ) );
   connect( pb_csv, SIGNAL( clicked() ), SLOT( save_csv() ) );

   pb_discard = new QPushButton( us_tr( "Discard" ), this );
   pb_discard->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );
   pb_discard->setMinimumHeight( minHeight1 );
   pb_discard->setPalette( PALET_PUSHB );
   pb_discard->setToolTip( us_tr( "Close this window WITHOUT adding the extrapolated curve to the SAS plot." ) );
   connect( pb_discard, SIGNAL( clicked() ), SLOT( discard() ) );

   pb_keep = new QPushButton( pending_open ? us_tr( "Keep" ) : us_tr( "Close" ), this );
   pb_keep->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );
   pb_keep->setMinimumHeight( minHeight1 );
   pb_keep->setPalette( PALET_PUSHB );
   pb_keep->setToolTip( us_tr( "Close this window and add the extrapolated curve to the SAS plot (then, if chosen, save it)." ) );
   connect( pb_keep, SIGNAL( clicked() ), SLOT( keep() ) );
   if ( !pending_open )
   {
      pb_discard->hide();
   }

   QHBoxLayout * hbl_wheel = new QHBoxLayout; hbl_wheel->setContentsMargins( 4, 0, 4, 0 ); hbl_wheel->setSpacing( 6 );
   hbl_wheel->addWidget( pb_prev );
   hbl_wheel->addWidget( qwtw_wheel );
   hbl_wheel->addWidget( pb_next );

   QHBoxLayout * hbl_movie = new QHBoxLayout; hbl_movie->setContentsMargins( 0, 0, 0, 0 ); hbl_movie->setSpacing( 0 );
   hbl_movie->addWidget( pb_movie_front );
   hbl_movie->addWidget( pb_movie_slower );
   hbl_movie->addWidget( pb_movie_start );
   hbl_movie->addWidget( pb_movie_faster );
   hbl_movie->addWidget( pb_movie_end );
   hbl_movie->addWidget( lbl_movie_state, 1 );

   QHBoxLayout * hbl_range = new QHBoxLayout; hbl_range->setContentsMargins( 4, 0, 4, 0 ); hbl_range->setSpacing( 4 );
   hbl_range->addWidget( lbl_q_from );
   hbl_range->addWidget( le_movie_q_from );
   hbl_range->addWidget( lbl_q_to );
   hbl_range->addWidget( le_movie_q_to );
   hbl_range->addWidget( lbl_every );
   hbl_range->addWidget( sb_movie_every );
   hbl_range->addWidget( lbl_every2 );
   hbl_range->addSpacing( 8 );
   hbl_range->addWidget( lbl_movie_range_info, 1 );

   QHBoxLayout * hbl_frames = new QHBoxLayout; hbl_frames->setContentsMargins( 4, 0, 4, 0 ); hbl_frames->setSpacing( 4 );
   hbl_frames->addWidget( cb_save_frames );
   hbl_frames->addWidget( le_save_prefix, 1 );
   hbl_frames->addWidget( rb_save_png );
   hbl_frames->addWidget( rb_save_jpeg );
   hbl_frames->addWidget( rb_save_bmp );
   hbl_frames->addWidget( cb_save_overwrite );

   QHBoxLayout * hbl_make = new QHBoxLayout; hbl_make->setContentsMargins( 4, 0, 4, 0 ); hbl_make->setSpacing( 4 );
   hbl_make->addWidget( pb_make_movie );
   hbl_make->addWidget( lbl_fps );
   hbl_make->addWidget( sb_movie_fps );
   hbl_make->addSpacing( 8 );
   hbl_make->addWidget( cb_remove_frames );
   hbl_make->addStretch( 1 );

   QHBoxLayout * hbl_bottom = new QHBoxLayout; hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addWidget( pb_help );
   hbl_bottom->addWidget( pb_csv );
   hbl_bottom->addWidget( pb_discard );
   hbl_bottom->addWidget( pb_keep );

   QVBoxLayout * background = new QVBoxLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );
   background->addWidget( lbl_info );
   background->addWidget( plot );
   background->addLayout( hbl_wheel );
   background->addWidget( lbl_results );
   background->addLayout( hbl_movie );
   background->addLayout( hbl_range );
   background->addLayout( hbl_frames );
   background->addLayout( hbl_make );
   background->addLayout( hbl_bottom );

   update_range_info();
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::clear_plot_items()
{
   for ( int i = 0; i < (int) plot_curves.size(); i++ )
   {
      plot_curves[ i ]->detach();
      delete plot_curves[ i ];
   }
   plot_curves.clear();

   for ( int i = 0; i < (int) plot_markers.size(); i++ )
   {
      plot_markers[ i ]->detach();
      delete plot_markers[ i ];
   }
   plot_markers.clear();
}

int US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::clamp_index( int i )
{
   if ( i < 0 )
   {
      i = 0;
   }
   if ( i > (int) reg_q.size() - 1 )
   {
      i = (int) reg_q.size() - 1;
   }
   return i;
}

// used by Prev/Next and initial show: snaps the wheel to the exact index
void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::set_index( int i )
{
   if ( !reg_q.size() )
   {
      return;
   }
   cur_index = clamp_index( i );

   qwtw_wheel->blockSignals( true );
   qwtw_wheel->setValue( (double) cur_index );
   qwtw_wheel->blockSignals( false );

   update_plot();
}

// driven by dragging the wheel: update the plot to the nearest q WITHOUT snapping
// the wheel back, so it scrolls smoothly (and keeps its inertia)
void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::wheel_changed( double val )
{
   if ( !reg_q.size() )
   {
      return;
   }
   int i = clamp_index( (int) qRound( val ) );
   if ( i != cur_index )
   {
      cur_index = i;
      update_plot();
   }
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::prev()
{
   set_index( cur_index - 1 );
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::next()
{
   set_index( cur_index + 1 );
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::update_plot()
{
   clear_plot_items();

   if ( plot_zoomer )
   {
      delete plot_zoomer;
      plot_zoomer = (ScrollZoomer *) 0;
   }

   int i = cur_index;
   if ( i < 0 || i >= (int) reg_q.size() || !reg_x[ i ].size() )
   {
      plot->replot();
      after_redraw();
      return;
   }

   const vector < double > & x = reg_x[ i ];
   const vector < double > & y = reg_y[ i ];
   const vector < double > & e = reg_e[ i ];
   double a    = reg_a[ i ];
   double b    = reg_b[ i ];
   double siga = reg_siga[ i ];

   double x_max = 0e0;
   for ( int j = 0; j < (int) x.size(); j++ )
   {
      if ( x[ j ] > x_max )
      {
         x_max = x[ j ];
      }
   }
   double x_lo = -0.04 * x_max;
   double x_hi = 1.06 * x_max;

   double y_min = a - siga;
   double y_max = a + siga;
   for ( int j = 0; j < (int) y.size(); j++ )
   {
      double ej = ( j < (int) e.size() ) ? e[ j ] : 0e0;
      double lo = y[ j ] - ej;
      double hi = y[ j ] + ej;
      if ( lo < y_min ) y_min = lo;
      if ( hi > y_max ) y_max = hi;
   }
   if ( a < y_min ) y_min = a;
   if ( a > y_max ) y_max = a;

   // include the QC-excluded curve's point (drawn below as a red x) in the autoscale so it
   // stays visible -- seeing where the rejected point sits is the purpose of showing it
   bool have_excl = ( excl_conc > 0e0 && i < (int) reg_excl_y.size() && !us_isnan( reg_excl_y[ i ] ) );
   if ( have_excl )
   {
      if ( reg_excl_y[ i ] < y_min ) y_min = reg_excl_y[ i ];
      if ( reg_excl_y[ i ] > y_max ) y_max = reg_excl_y[ i ];
   }
   // likewise the measured values of replaced spikes (drawn in magenta below)
   int n_spk_here = ( i < (int) reg_sx.size() ) ? (int) reg_sx[ i ].size() : 0;
   for ( int j = 0; j < n_spk_here; j++ )
   {
      double ej = ( j < (int) reg_se[ i ].size() ) ? reg_se[ i ][ j ] : 0e0;
      if ( reg_sy[ i ][ j ] - ej < y_min )
      {
         y_min = reg_sy[ i ][ j ] - ej;
      }
      if ( reg_sy[ i ][ j ] + ej > y_max )
      {
         y_max = reg_sy[ i ][ j ] + ej;
      }
      if ( reg_sx[ i ][ j ] > x_max )
      {
         x_max = reg_sx[ i ][ j ];
         x_hi  = 1.06 * x_max;
         x_lo  = -0.04 * x_max;
      }
   }

   // fitted curve across the full x-range (so the c=0 intercept is visible). A straight
   // line for the additive/reciprocal fits; a sampled parabola for the 2nd-order virial.
   double c_coef = ( i < (int) reg_c.size() ) ? reg_c[ i ] : 0e0;
   {
      const int NS = 60;
      vector < double > lx( NS ), ly( NS );
      for ( int k = 0; k < NS; k++ )
      {
         double xx = x_lo + ( x_hi - x_lo ) * (double) k / ( NS - 1 );
         lx[ k ] = xx;
         ly[ k ] = a + b * xx + c_coef * xx * xx;
         if ( ly[ k ] < y_min ) y_min = ly[ k ];
         if ( ly[ k ] > y_max ) y_max = ly[ k ];
      }
      QwtPlotCurve * curve = new QwtPlotCurve( "fit" );
      curve->setStyle( QwtPlotCurve::Lines );
      curve->setPen( QPen( Qt::green, 1, Qt::SolidLine ) );
      curve->setSamples( &lx[ 0 ], &ly[ 0 ], NS );
      curve->attach( plot );
      plot_curves.push_back( curve );
   }

   // per-point error bars
   for ( int j = 0; j < (int) x.size(); j++ )
   {
      double ej = ( j < (int) e.size() ) ? e[ j ] : 0e0;
      if ( ej <= 0e0 )
      {
         continue;
      }
      double ex[ 2 ] = { x[ j ], x[ j ] };
      double ey[ 2 ] = { y[ j ] - ej, y[ j ] + ej };
      QwtPlotCurve * curve = new QwtPlotCurve( "errorbar" );
      curve->setStyle( QwtPlotCurve::Lines );
      curve->setPen( QPen( Qt::darkCyan, 1, Qt::SolidLine ) );
      curve->setSamples( ex, ey, 2 );
      curve->attach( plot );
      plot_curves.push_back( curve );
   }

   // data points
   {
      QwtPlotCurve * curve = new QwtPlotCurve( "data" );
      curve->setStyle( QwtPlotCurve::NoCurve );
      curve->setSymbol( new QwtSymbol( QwtSymbol::Ellipse, QBrush( Qt::cyan ), QPen( Qt::cyan ), QSize( 8, 8 ) ) );
      curve->setSamples( (double *) &( x[ 0 ] ), (double *) &( y[ 0 ] ), x.size() );
      curve->attach( plot );
      plot_curves.push_back( curve );
   }

   // measured values of points replaced in this q's fit as single-point spikes: magenta, with their
   // error bars, so it is visible why each was replaced (the fit used the cyan point at the same c)
   for ( int j = 0; j < n_spk_here; j++ )
   {
      double ej = ( j < (int) reg_se[ i ].size() ) ? reg_se[ i ][ j ] : 0e0;
      if ( ej > 0e0 )
      {
         double ex[ 2 ] = { reg_sx[ i ][ j ], reg_sx[ i ][ j ] };
         double ey[ 2 ] = { reg_sy[ i ][ j ] - ej, reg_sy[ i ][ j ] + ej };
         QwtPlotCurve * curve = new QwtPlotCurve( "spike errorbar" );
         curve->setStyle( QwtPlotCurve::Lines );
         curve->setPen( QPen( Qt::magenta, 1, Qt::SolidLine ) );
         curve->setSamples( ex, ey, 2 );
         curve->attach( plot );
         plot_curves.push_back( curve );
      }
      QwtPlotMarker * marker = new QwtPlotMarker;
      marker->setSymbol( new QwtSymbol( QwtSymbol::Ellipse, QBrush( Qt::magenta ), QPen( Qt::magenta ), QSize( 9, 9 ) ) );
      marker->setValue( reg_sx[ i ][ j ], reg_sy[ i ][ j ] );
      marker->attach( plot );
      plot_markers.push_back( marker );
   }

   // QC-excluded curve's point at this q: a red x (the same convention as removed Guinier
   // points), not part of the fit -- so the user can see what was rejected and where it sat
   if ( have_excl )
   {
      QwtPlotMarker * marker = new QwtPlotMarker;
      marker->setSymbol( new QwtSymbol( QwtSymbol::XCross, QBrush( Qt::red ), QPen( Qt::red, 2 ), QSize( 12, 12 ) ) );
      marker->setValue( excl_conc, reg_excl_y[ i ] );
      marker->attach( plot );
      plot_markers.push_back( marker );
   }

   // result marker: for Zimm this is the c=0 intercept; for absolute-scale it is the merged
   // (level) value. Draw its error bar + a highlighted diamond at x=0.
   {
      double ix[ 2 ] = { 0e0, 0e0 };
      double iy[ 2 ] = { a - siga, a + siga };
      if ( siga > 0e0 )
      {
         QwtPlotCurve * curve = new QwtPlotCurve( "result errorbar" );
         curve->setStyle( QwtPlotCurve::Lines );
         curve->setPen( QPen( Qt::red, 2, Qt::SolidLine ) );
         curve->setSamples( ix, iy, 2 );
         curve->attach( plot );
         plot_curves.push_back( curve );
      }

      QwtPlotMarker * marker = new QwtPlotMarker;
      marker->setValue( 0e0, a );
      marker->setSymbol( new QwtSymbol( QwtSymbol::Diamond, QBrush( Qt::red ), QPen( Qt::red ), QSize( 11, 11 ) ) );
      marker->attach( plot );
      plot_markers.push_back( marker );

      // vertical reference line at c=0 (marks where the fit extrapolates to); white so the
      // red intercept error bar drawn on top of it stays clearly visible
      QwtPlotMarker * vline = new QwtPlotMarker;
      vline->setLineStyle( QwtPlotMarker::VLine );
      vline->setLinePen( QPen( Qt::white, 1, Qt::DashDotLine ) );
      vline->setXValue( 0e0 );
      vline->attach( plot );
      plot_markers.push_back( vline );
   }

   double y_pad = 0.06 * ( y_max - y_min );
   if ( y_pad <= 0e0 )
   {
      y_pad = 0.5 * ( qAbs( y_max ) + 1e0 );
   }
   plot->setAxisScale( QwtPlot::xBottom, x_lo, x_hi );
   plot->setAxisScale( QwtPlot::yLeft, y_min - y_pad, y_max + y_pad );

   bool at_ref = ( merge_q > 0e0 && reg_q[ i ] >= merge_q );
   QString mode_note;
   if ( merge_q > 0e0 )
   {
      mode_note = at_ref
         ? us_tr( "  -- output: reference curve (q >= merge point)" )
         : us_tr( "  -- output: extrapolated intercept" );
   }

   plot->setTitle( QString( us_tr( "q = %1  (point %2 of %3)%4" ) )
                   .arg( reg_q[ i ], 0, 'g', 6 )
                   .arg( i + 1 )
                   .arg( (int) reg_q.size() )
                   .arg( mode_note ) );

   // goodness of fit at this q: R^2 (fraction of variance explained) and, when the data
   // carry error bars, the reduced chi^2 of the points about the plotted fit
   double r2, chi2, red_chi2;
   int    have_err, nparam;
   fit_quality( i, r2, chi2, red_chi2, have_err, nparam );

   // for the reciprocal/virial models the plotted intercept is u = 1/I0; report both
   QString fit_desc;
   if ( model >= 1 )
   {
      double I0  = ( a != 0e0 ) ? 1e0 / a : 0e0;
      double I0e = ( a != 0e0 ) ? siga / ( a * a ) : 0e0;
      fit_desc = QString( us_tr( "u(c=0) = %1 ± %2   \342\206\222  I(0) = 1/u = %3 ± %4" ) )
                 .arg( a, 0, 'g', 6 ).arg( siga, 0, 'g', 4 )
                 .arg( I0, 0, 'g', 6 ).arg( I0e, 0, 'g', 4 );
   }
   else
   {
      fit_desc = QString( us_tr( "I(0) = %1 ± %2" ) ).arg( a, 0, 'g', 6 ).arg( siga, 0, 'g', 4 );
   }
   QString qof = ( have_err > nparam )
      ? QString( us_tr( "R\302\262 = %1   reduced \317\207\302\262 = %2" ) ).arg( r2, 0, 'f', 4 ).arg( red_chi2, 0, 'g', 4 )
      : QString( us_tr( "R\302\262 = %1" ) ).arg( r2, 0, 'f', 4 );

   lbl_results->setText( QString( us_tr( "q = %1    %2    %3    n = %4 curves%5%6" ) )
                         .arg( reg_q[ i ], 0, 'g', 6 )
                         .arg( fit_desc )
                         .arg( qof )
                         .arg( (int) x.size() )
                         .arg( n_spk_here ? QString( us_tr( "   (%1 spike replaced)" ) ).arg( n_spk_here ) : QString( "" ) )
                         .arg( at_ref ? us_tr( "   [taken from reference]" ) : QString( "" ) ) );

   plot_zoomer = new ScrollZoomer( plot->canvas() );
   plot_zoomer->setRubberBandPen( QPen( Qt::yellow, 0, Qt::DotLine ) );
   plot_zoomer->setTrackerPen( QPen( Qt::red ) );

   plot->replot();
   after_redraw();
}

// after every redraw: the frame when frames are being saved, and the movie position
void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::after_redraw()
{
   if ( cb_save_frames->isChecked() && !save_frame() )
   {
      movie_stop();
      cb_save_frames->setChecked( false );
      set_save_frames();
   }
   update_movie_state();
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::update_movie_state()
{
   if ( ffmpeg_proc->state() != QProcess::NotRunning )
   {
      lbl_movie_state->setText( QString( us_tr( "Encoding the movie from %1 frame(s) ..." ) ).arg( movie_frames.size() ) );
      return;
   }
   if ( making_movie )
   {
      int first, last, step;
      int n = movie_range( first, last, step ) ? ( last - first ) / step + 1 : 0;
      lbl_movie_state->setText( QString( us_tr( "Making the movie: frame %1 of %2" ) ).arg( movie_frames.size() ).arg( n ) );
      return;
   }
   lbl_movie_state->setText( QString( us_tr( "%1: %2 of %3   (%4 ms per frame)" ) )
                             .arg( movie_timer->isActive() ? us_tr( "Running" ) : us_tr( "Stopped" ) )
                             .arg( reg_q.size() ? cur_index + 1 : 0 )
                             .arg( (int) reg_q.size() )
                             .arg( movie_timer_ms ) );
}

// the q points of the movie: the first at or above "q from", then every n-th point up to "to"
// (the fields show q to 6 digits, so a q within that rounding of the typed value counts);
// false when the fields are not numbers or no q point is in the range
bool US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::movie_range( int & first, int & last, int & step )
{
   first = 0;
   last  = (int) reg_q.size() - 1;
   step  = qMax( 1, sb_movie_every->value() );
   if ( !reg_q.size() )
   {
      return false;
   }
   bool ok_from;
   bool ok_to;
   double q_from = le_movie_q_from->text().trimmed().toDouble( &ok_from );
   double q_to   = le_movie_q_to  ->text().trimmed().toDouble( &ok_to );
   if ( !ok_from || !ok_to )
   {
      return false;
   }
   double tol_from = 1e-5 * qAbs( q_from );
   double tol_to   = 1e-5 * qAbs( q_to );
   int f = -1;
   int l = -1;
   for ( int i = 0; i < (int) reg_q.size(); i++ )
   {
      if ( f < 0 && reg_q[ i ] >= q_from - tol_from )
      {
         f = i;
      }
      if ( reg_q[ i ] <= q_to + tol_to )
      {
         l = i;
      }
   }
   if ( f < 0 || l < f )
   {
      return false;
   }
   first = f;
   last  = f + ( ( l - f ) / step ) * step;
   return true;
}

// the movie's next q index after i (the range's first if i is before it); -1 at the end of the range
int US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::movie_next( int i )
{
   int first, last, step;
   if ( !movie_range( first, last, step ) )
   {
      return -1;
   }
   if ( i < first )
   {
      return first;
   }
   int n = first + ( ( i - first ) / step + 1 ) * step;
   return n <= last ? n : -1;
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::update_range_info()
{
   int first, last, step;
   if ( !movie_range( first, last, step ) )
   {
      lbl_movie_range_info->setText( us_tr( "no q point in this range" ) );
      return;
   }
   lbl_movie_range_info->setText( QString( us_tr( "%1 frame(s): q points %2 to %3" ) )
                                  .arg( ( last - first ) / step + 1 ).arg( first + 1 ).arg( last + 1 ) );
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::movie_front()
{
   int first, last, step;
   set_index( movie_range( first, last, step ) ? first : 0 );
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::movie_end()
{
   int first, last, step;
   set_index( movie_range( first, last, step ) ? last : (int) reg_q.size() - 1 );
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::movie_slower()
{
   movie_timer_ms = qMin( 5000, movie_timer_ms * 2 );
   if ( movie_timer->isActive() )
   {
      movie_timer->setInterval( movie_timer_ms );
   }
   update_movie_state();
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::movie_faster()
{
   movie_timer_ms = qMax( 50, movie_timer_ms / 2 );
   if ( movie_timer->isActive() )
   {
      movie_timer->setInterval( movie_timer_ms );
   }
   update_movie_state();
}

// start / stop the movie over the q range: from the current point if it is in the range (from the
// range's start if it is outside it or at its end); for Make movie always from the start, with no
// pause between frames. While Make movie is encoding, || stops ffmpeg.
void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::movie_start()
{
   if ( ffmpeg_proc->state() != QProcess::NotRunning )
   {
      encode_cancelled = true;
      ffmpeg_proc->kill();                       // encode_done reports it
      return;
   }
   if ( movie_timer->isActive() )
   {
      movie_stop();
      return;
   }
   if ( !reg_q.size() )
   {
      return;
   }
   int first, last, step;
   if ( !movie_range( first, last, step ) )
   {
      lbl_movie_state->setText( us_tr( "No q point is in the movie range" ) );
      return;
   }
   movie_timer->start( making_movie ? 0 : movie_timer_ms );
   pb_movie_start->setText( "||" );
   if ( making_movie || cur_index < first || cur_index >= last )
   {
      set_index( first );                        // saves the frame, if saving
   }
   else if ( ( cur_index - first ) % step == 0 )
   {
      after_redraw();                            // the frame already shown
   }
   else
   {
      set_index( movie_next( cur_index ) );
   }
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::movie_stop()
{
   if ( movie_timer->isActive() )
   {
      movie_timer->stop();
   }
   pb_movie_start->setText( "[]" );
   bool was_making = making_movie;
   making_movie = false;
   update_movie_state();
   report_frames();
   if ( was_making )
   {
      end_make( false, us_tr( "Make movie stopped before the end of the q range: no movie was made; the frames saved are kept" ) );
   }
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::movie_tick()
{
   int i = movie_next( cur_index );
   if ( i >= 0 )
   {
      set_index( i );
      return;
   }
   // end of the range
   if ( making_movie )
   {
      movie_timer->stop();
      start_encode();
      return;
   }
   // stop, and turn saving off so later stepping does not add or replace frames
   movie_stop();
   cb_save_frames->setChecked( false );
   cb_save_overwrite->setChecked( false );
   set_save_frames();
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::set_save_frames()
{
   if ( !cb_save_frames->isChecked() )
   {
      report_frames();
   }
}

// the frame and movie file prefix (path separators and ':' made '_')
QString US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::frames_prefix()
{
   QString prefix = le_save_prefix->text().trimmed();
   prefix.replace( QRegularExpression( "[/\\\\:]" ), "_" );
   if ( prefix.isEmpty() )
   {
      prefix = pending.base_name.isEmpty() ? QString( "extrap_c0" ) : pending.base_name;
   }
   return prefix;
}

// the folder the input curves were loaded from (the SOMO saxs/tmp folder if none): the frames folder
// and the movie go there
QString US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::frames_base_dir()
{
   US_Hydrodyn_Saxs * saxs = (US_Hydrodyn_Saxs *) saxs_window;
   US_Hydrodyn      * ush  = (US_Hydrodyn *) us_hydrodyn;
   if ( saxs && !saxs->our_saxs_options->path_load_saxs_curve.isEmpty()
        && QDir( saxs->our_saxs_options->path_load_saxs_curve ).exists() )
   {
      return saxs->our_saxs_options->path_load_saxs_curve;
   }
   if ( ush )
   {
      return ush->somo_dir + "/saxs/tmp";
   }
   return QDir::currentPath();
}

// <folder the input curves were loaded from>/<prefix>_frames
QString US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::frames_dir()
{
   return frames_base_dir() + "/" + frames_prefix() + "_frames";
}

// the plot with its results line below it, as <prefix>_<q point, 5 digits>.<format>
bool US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::save_frame()
{
   if ( !reg_q.size() )
   {
      return true;
   }
   QString dir = frames_dir();
   if ( !QDir().mkpath( dir ) )
   {
      QMessageBox::warning( this, windowTitle(),
                            QString( us_tr( "Could not create the folder %1 for the frames; frame saving is turned off." ) ).arg( dir ) );
      return false;
   }
   QString prefix = QFileInfo( dir ).fileName();
   prefix.chop( QString( "_frames" ).length() );

   QString ext = rb_save_jpeg->isChecked() ? "jpeg" : rb_save_bmp->isChecked() ? "bmp" : "png";
   QString fn  = QString( "%1/%2_%3.%4" ).arg( dir ).arg( prefix ).arg( cur_index + 1, 5, 10, QChar( '0' ) ).arg( ext );
   frames_dir_used = dir;
   // a frame already written in this run (e.g. the one shown when the movie is started) is not
   // counted again; a file from before this run is kept unless Overwrite is checked
   bool this_run = run_files.contains( fn );
   if ( !cb_save_overwrite->isChecked() && QFile::exists( fn ) )
   {
      if ( !this_run )
      {
         n_frames_kept++;
         run_files.insert( fn );
      }
      if ( making_movie && ( movie_frames.isEmpty() || movie_frames.last() != fn ) )
      {
         movie_frames << fn;
      }
      return true;
   }

   QPixmap pix_plot = plot->grab();
   QPixmap pix_res  = lbl_results->grab();
   if ( pix_plot.isNull() )
   {
      return true;                               // nothing drawn yet; not an error
   }
   QPixmap frame( qMax( pix_plot.width(), pix_res.width() ), pix_plot.height() + pix_res.height() );
   frame.fill( USglobal->global_colors.plot );
   {
      QPainter painter( &frame );
      painter.drawPixmap( 0, 0, pix_plot );
      if ( !pix_res.isNull() )
      {
         painter.drawPixmap( 0, pix_plot.height(), pix_res );
      }
      painter.end();
   }
   if ( !frame.save( fn, ext == "jpeg" ? "JPEG" : ext == "bmp" ? "BMP" : "PNG" ) )
   {
      QMessageBox::warning( this, windowTitle(),
                            QString( us_tr( "Could not write the frame %1; frame saving is turned off." ) ).arg( fn ) );
      return false;
   }
   if ( !this_run )
   {
      n_frames_saved++;
      run_files.insert( fn );
   }
   if ( making_movie && ( movie_frames.isEmpty() || movie_frames.last() != fn ) )
   {
      movie_frames << fn;
   }
   return true;
}

// Make movie: save the frames of the q range (movie_tick then calls start_encode)
void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::make_movie()
{
   if ( ffmpeg_exe.isEmpty() || making_movie || ffmpeg_proc->state() != QProcess::NotRunning )
   {
      return;
   }
   if ( movie_timer->isActive() )
   {
      movie_stop();
   }
   int first, last, step;
   if ( !movie_range( first, last, step ) )
   {
      QMessageBox::warning( this, windowTitle(), us_tr( "No q point is in the movie range (q from / to); no movie was made." ) );
      return;
   }
   movie_file = frames_base_dir() + "/" + frames_prefix() + ".mp4";
   if ( QFile::exists( movie_file ) && !cb_save_overwrite->isChecked() )
   {
      QMessageBox::warning( this, windowTitle(),
                            QString( us_tr( "The movie file %1 already exists.\n"
                                            "Check Overwrite to replace it, or change the prefix." ) ).arg( movie_file ) );
      return;
   }
   if ( cb_save_frames->isChecked() )
   {
      report_frames();                           // frames saved before, on their own log line
   }
   save_frames_before_make = cb_save_frames->isChecked();
   movie_frames.clear();
   encode_cancelled = false;
   making_movie     = true;
   cb_save_frames->setChecked( true );
   set_making_controls( true );
   movie_start();
}

// while Make movie runs only || (stop) is active
void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::set_making_controls( bool making )
{
   QWidget * ws[] = { pb_prev, pb_next, qwtw_wheel, pb_movie_front, pb_movie_slower, pb_movie_faster, pb_movie_end,
                      le_movie_q_from, le_movie_q_to, sb_movie_every, cb_save_frames, le_save_prefix,
                      rb_save_png, rb_save_jpeg, rb_save_bmp, cb_save_overwrite, pb_csv, pb_discard, pb_keep };
   for ( int k = 0; k < (int) ( sizeof( ws ) / sizeof( ws[ 0 ] ) ); k++ )
   {
      ws[ k ]->setEnabled( !making );
   }
   pb_make_movie   ->setEnabled( !making && !ffmpeg_exe.isEmpty() );
   sb_movie_fps    ->setEnabled( !making && !ffmpeg_exe.isEmpty() );
   cb_remove_frames->setEnabled( !making && !ffmpeg_exe.isEmpty() );
}

// the frames are saved: join them with ffmpeg into a temporary file next to the movie (renamed to
// <prefix>.mp4 when ffmpeg succeeds, so a failed run leaves an existing movie as it was)
void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::start_encode()
{
   report_frames();
   if ( movie_frames.isEmpty() )
   {
      end_make( false, us_tr( "Make movie: no frame was saved, so no movie was made" ) );
      return;
   }
   movie_frames_dir = QFileInfo( movie_frames.first() ).absolutePath();

   // the movie's frame size is the first frame's, made even (H.264 needs even sizes); frames of
   // another size (window resized, or kept from an earlier run) are scaled and padded to it
   QSize sz = QImageReader( movie_frames.first() ).size();
   if ( !sz.isValid() )
   {
      end_make( false, QString( us_tr( "Make movie: could not read the frame %1; no movie was made" ) ).arg( movie_frames.first() ) );
      return;
   }
   int w = sz.width()  + ( sz.width()  % 2 );
   int h = sz.height() + ( sz.height() % 2 );

   // frames are numbered by q point, so every n-th point leaves gaps in the numbers: give ffmpeg
   // the list of frames (its concat format, names relative to the list's folder)
   movie_list_file = movie_frames_dir + "/" + frames_prefix() + "_movie.ffconcat";
   {
      QFile lf( movie_list_file );
      if ( !lf.open( QIODevice::WriteOnly ) )      // "\n" line ends on every platform
      {
         end_make( false, QString( us_tr( "Make movie: could not write %1; no movie was made" ) ).arg( movie_list_file ) );
         return;
      }
      QTextStream ts( &lf );
      QString dur = QString::number( 1e0 / (double) sb_movie_fps->value(), 'g', 10 );
      QString nm;
      ts << "ffconcat version 1.0\n";
      for ( int k = 0; k < (int) movie_frames.size(); k++ )
      {
         nm = QFileInfo( movie_frames[ k ] ).fileName();
         nm.replace( "'", "'\\''" );
         ts << "file '" << nm << "'\nduration " << dur << "\n";
      }
      // ffmpeg's concat reader ignores the last entry's duration: list the last frame once more
      ts << "file '" << nm << "'\n";
      lf.close();
   }

   // the encoder: H.264 if this ffmpeg has it, else MPEG-4 part 2 (built into every ffmpeg)
   if ( ffmpeg_encoder.isEmpty() )
   {
      QProcess p;
      p.start( ffmpeg_exe, QStringList() << "-hide_banner" << "-encoders" );
      ffmpeg_encoder = "mpeg4";
      if ( p.waitForFinished( 10000 )
           && QString::fromLocal8Bit( p.readAllStandardOutput() ).contains( QRegularExpression( "\\slibx264\\s" ) ) )
      {
         ffmpeg_encoder = "libx264";
      }
   }

   QString pad = "0x" + USglobal->global_colors.plot.name().mid( 1 );
   QString vf  = QString( "fps=%1,scale=%2:%3:force_original_aspect_ratio=decrease,pad=%2:%3:(ow-iw)/2:(oh-ih)/2:color=%4" )
      .arg( sb_movie_fps->value() ).arg( w ).arg( h ).arg( pad );
   movie_tmp_file = movie_file + ".part";
   QFile::remove( movie_tmp_file );

   QStringList args;
   args << "-hide_banner" << "-nostdin" << "-loglevel" << "error" << "-y"
        << "-f" << "concat" << "-safe" << "0" << "-i" << movie_list_file
        << "-vf" << vf
        << "-c:v" << ffmpeg_encoder;
   if ( ffmpeg_encoder == "libx264" )
   {
      args << "-crf" << "18";
   }
   else
   {
      args << "-q:v" << "2";
   }
   args << "-pix_fmt" << "yuv420p" << "-f" << "mp4" << movie_tmp_file;

   ffmpeg_proc->setWorkingDirectory( movie_frames_dir );
   ffmpeg_proc->start( ffmpeg_exe, args );
   update_movie_state();
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::encode_error( QProcess::ProcessError err )
{
   // a started ffmpeg that fails or is stopped reports through finished(); this is for one that never ran
   if ( err != QProcess::FailedToStart )
   {
      return;
   }
   QFile::remove( movie_list_file );
   end_make( false, QString( us_tr( "Make movie: could not run %1; no movie was made, the frames are kept in %2" ) )
             .arg( ffmpeg_exe ).arg( movie_frames_dir ) );
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::encode_done( int exit_code, QProcess::ExitStatus status )
{
   QString out = QString::fromLocal8Bit( ffmpeg_proc->readAll() ).trimmed();
   QFile::remove( movie_list_file );
   if ( encode_cancelled )
   {
      QFile::remove( movie_tmp_file );
      end_make( false, QString( us_tr( "Make movie stopped while encoding: no movie was made; the frames are kept in %1" ) )
                .arg( movie_frames_dir ) );
      return;
   }
   if ( status != QProcess::NormalExit || exit_code != 0
        || !QFile::exists( movie_tmp_file ) || QFileInfo( movie_tmp_file ).size() == 0 )
   {
      QFile::remove( movie_tmp_file );
      QStringList lines = out.split( "\n" );
      while ( lines.size() > 10 )
      {
         lines.removeFirst();
      }
      end_make( false, QString( us_tr( "Make movie: ffmpeg failed (exit code %1); no movie was made, the frames are kept in %2%3" ) )
                .arg( exit_code ).arg( movie_frames_dir )
                .arg( lines.join( "\n" ).trimmed().isEmpty() ? QString( "" ) : ":\n" + lines.join( "\n" ).trimmed() ) );
      return;
   }
   if ( QFile::exists( movie_file ) && !QFile::remove( movie_file ) )
   {
      end_make( false, QString( us_tr( "Make movie: could not replace %1; the new movie is %2" ) ).arg( movie_file ).arg( movie_tmp_file ) );
      return;
   }
   if ( !QFile::rename( movie_tmp_file, movie_file ) )
   {
      end_make( false, QString( us_tr( "Make movie: could not rename %1 to %2" ) ).arg( movie_tmp_file ).arg( movie_file ) );
      return;
   }
   QString msg = QString( us_tr( "Fit viewer movie: made %1 from %2 frame(s) at %3 frames/s (%4)" ) )
      .arg( movie_file ).arg( movie_frames.size() ).arg( sb_movie_fps->value() ).arg( ffmpeg_encoder );
   if ( cb_remove_frames->isChecked() )
   {
      // only the frames the movie was made from; the folder only if that left it empty
      int n_removed = 0;
      for ( int k = 0; k < (int) movie_frames.size(); k++ )
      {
         if ( QFile::remove( movie_frames[ k ] ) )
         {
            n_removed++;
         }
      }
      msg += QString( us_tr( "; removed its %1 frame file(s)" ) ).arg( n_removed );
      if ( QDir().rmdir( movie_frames_dir ) )
      {
         msg += QString( us_tr( " and the folder %1" ) ).arg( movie_frames_dir );
      }
      else
      {
         msg += QString( us_tr( "; the folder %1 was kept, it holds other files" ) ).arg( movie_frames_dir );
      }
   }
   else
   {
      msg += QString( us_tr( "; the frames are in %1" ) ).arg( movie_frames_dir );
   }
   end_make( true, msg );
}

// Make movie is over (made, failed or stopped): saving back as it was before, Overwrite off (as at the
// end of the movie), the controls back, and one log line
void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::end_make( bool ok, const QString & msg )
{
   making_movie = false;
   if ( movie_timer->isActive() )
   {
      movie_timer->stop();
   }
   pb_movie_start->setText( "[]" );
   set_making_controls( false );
   cb_save_frames->setChecked( save_frames_before_make );
   cb_save_overwrite->setChecked( false );
   set_save_frames();
   US_Hydrodyn_Saxs * saxs = (US_Hydrodyn_Saxs *) saxs_window;
   if ( saxs )
   {
      saxs->editor_msg( ok ? "black" : "dark red", msg + "\n" );
   }
   update_movie_state();
}

// one log line per run of saved frames (on stop, when saving is turned off, and on close)
void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::report_frames()
{
   if ( !n_frames_saved && !n_frames_kept )
   {
      return;
   }
   US_Hydrodyn_Saxs * saxs = (US_Hydrodyn_Saxs *) saxs_window;
   if ( saxs )
   {
      QString msg = QString( us_tr( "Fit viewer movie: saved %1 frame(s) in %2" ) ).arg( n_frames_saved ).arg( frames_dir_used );
      if ( n_frames_kept )
      {
         msg += QString( us_tr( "; %1 frame file(s) already there were kept (check Overwrite to replace them)" ) ).arg( n_frames_kept );
      }
      saxs->editor_msg( n_frames_kept ? "dark red" : "black", msg + "\n" );
   }
   n_frames_saved = 0;
   n_frames_kept  = 0;
   run_files.clear();
}

// goodness of the fit at q index i, about the fitted line / parabola: R^2 (unweighted, the fraction of
// the points' variance explained), chi^2 of the points with their own error bars (n_err = how many
// points carry one) and the reduced chi^2 = chi^2 / ( n_err - nparam ) (0 when n_err <= nparam)
void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::fit_quality( int i, double & r2, double & chi2, double & red_chi2,
                                                          int & n_err, int & nparam )
{
   r2       = 1e0;
   chi2     = 0e0;
   red_chi2 = 0e0;
   n_err    = 0;
   nparam   = 2;
   if ( i < 0 || i >= (int) reg_q.size() )
   {
      return;
   }
   const vector < double > & x = reg_x[ i ];
   const vector < double > & y = reg_y[ i ];
   const vector < double > & e = reg_e[ i ];
   double a      = reg_a[ i ];
   double b      = reg_b[ i ];
   double c_coef = ( i < (int) reg_c.size() ) ? reg_c[ i ] : 0e0;

   double ss_res = 0e0, ss_tot = 0e0, ybar_s = 0e0;
   int    nfit = (int) qMin( x.size(), y.size() );
   for ( int j = 0; j < nfit; j++ )
   {
      ybar_s += y[ j ];
   }
   if ( nfit )
   {
      ybar_s /= (double) nfit;
   }
   for ( int j = 0; j < nfit; j++ )
   {
      double yp = a + b * x[ j ] + c_coef * x[ j ] * x[ j ];
      double r  = y[ j ] - yp;
      ss_res += r * r;
      ss_tot += ( y[ j ] - ybar_s ) * ( y[ j ] - ybar_s );
      double ej = ( j < (int) e.size() ) ? e[ j ] : 0e0;
      if ( ej > 0e0 )
      {
         chi2 += ( r / ej ) * ( r / ej );
         n_err++;
      }
   }
   nparam   = ( c_coef != 0e0 ) ? 3 : 2;
   r2       = ( ss_tot > 0e0 ) ? 1e0 - ss_res / ss_tot : 1e0;
   red_chi2 = ( n_err > nparam ) ? chi2 / ( n_err - nparam ) : 0e0;
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::usp_config_plot( const QPoint & )
{
   US_PlotChoices * uspc = new US_PlotChoices( usp_plot );
   uspc->exec();
   delete uspc;
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::help()
{
   US_Help * online_help;
   online_help = new US_Help( this );
   online_help->show_help( "manual/somo/somo_saxs_extrap_c0.html" );
}

static QString us_extrap_c0_csv_quote( const QString & s )
{
   QString t = s;
   t.replace( "\"", "\"\"" );
   return "\"" + t + "\"";
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::save_csv()
{
   US_Hydrodyn_Saxs * saxs = (US_Hydrodyn_Saxs *) saxs_window;
   QString use_dir = ( saxs && !saxs->our_saxs_options->path_load_saxs_curve.isEmpty() )
      ? saxs->our_saxs_options->path_load_saxs_curve : QDir::currentPath();
   QString base = pending.base_name.isEmpty() ? QString( "extrap_c0" ) : pending.base_name;
   QString fn = QFileDialog::getSaveFileName( this,
                                              us_tr( "Save the per-q fits as CSV" ),
                                              use_dir + "/" + base + "_fits.csv",
                                              "*.csv" );
   if ( fn.isEmpty() )
   {
      return;
   }
   if ( QFileInfo( fn ).suffix().isEmpty() )
   {
      fn += ".csv";
   }
   QFile f( fn );
   if ( !f.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      QMessageBox::warning( this, windowTitle(),
                            QString( us_tr( "Could not open %1 for writing:\n%2" ) ).arg( fn ).arg( f.errorString() ) );
      return;
   }

   // the output curve by q (the same grid values the fits were made on)
   map < double, int > out_at;
   for ( int k = 0; k < (int) pending.q.size(); k++ )
   {
      out_at[ pending.q[ k ] ] = k;
   }
   vector < int > cols;                          // curves in the fit
   for ( int c = 0; c < (int) pending.curve_names.size(); c++ )
   {
      if ( c < (int) pending.curve_concs.size() && pending.curve_concs[ c ] > 0e0 )
      {
         cols.push_back( c );
      }
   }
   bool have_excl = ( excl_conc > 0e0 && !excl_name.isEmpty() );

   QTextStream ts( &f );
   ts << "# US-SOMO extrapolation to zero concentration: per-q fits\n";
   ts << "# curve," << us_extrap_c0_csv_quote( base ) << "\n";
   ts << "# model," << us_extrap_c0_csv_quote( pending.model_desc ) << "\n";
   ts << "# fit axis," << us_extrap_c0_csv_quote( y_axis_title ) << "\n";
   ts << "# switchover q," << ( merge_q > 0e0 ? QString::number( merge_q, 'g', 8 ) : QString( "none" ) ) << "\n";
   ts << "# fit_* columns are the per-q fit on the fit axis (for c/I fits the intercept is 1/I(0));"
         " fit_r2 is the (unweighted) R^2 of the fitted points about the fit, fit_chi2 their chi^2 with"
         " their own errors and fit_reduced_chi2 = fit_chi2 / (points with an error - fitted parameters),"
         " blank when not defined;"
         " output_I and output_sd are the extrapolated curve as added (after any splice and scale);"
         " per curve: value on the fit axis used in the fit, its error, status (fit = measured value used;"
         " replaced = a single-point spike fitted at its neighbours' local-fit value; blank = not available"
         " at this q) and, for a replaced point, its measured value\n";
   for ( int k = 0; k < (int) cols.size(); k++ )
   {
      ts << "# input," << us_extrap_c0_csv_quote( pending.curve_names[ cols[ k ] ] )
         << ",conc," << QString::number( pending.curve_concs[ cols[ k ] ], 'g', 8 ) << "\n";
   }
   if ( have_excl )
   {
      ts << "# qc_discarded," << us_extrap_c0_csv_quote( excl_name ) << ",conc," << QString::number( excl_conc, 'g', 8 ) << "\n";
   }
   QStringList hdr;
   hdr << "q" << "fit_intercept" << "fit_intercept_sd" << "fit_slope" << "fit_quadratic" << "n_fit"
       << "fit_r2" << "fit_chi2" << "fit_reduced_chi2"
       << "output_I" << "output_sd";
   for ( int k = 0; k < (int) cols.size(); k++ )
   {
      QString nm = pending.curve_names[ cols[ k ] ];
      hdr << us_extrap_c0_csv_quote( nm + " y" ) << us_extrap_c0_csv_quote( nm + " sd" )
          << us_extrap_c0_csv_quote( nm + " status" ) << us_extrap_c0_csv_quote( nm + " measured" );
   }
   if ( have_excl )
   {
      hdr << us_extrap_c0_csv_quote( excl_name + " y (qc_discarded)" );
   }
   ts << hdr.join( "," ) << "\n";

   for ( int i = 0; i < (int) reg_q.size(); i++ )
   {
      QStringList row;
      row << QString::number( reg_q[ i ], 'g', 10 )
          << QString::number( reg_a[ i ], 'g', 10 )
          << QString::number( i < (int) reg_siga.size() ? reg_siga[ i ] : 0e0, 'g', 10 )
          << QString::number( reg_b[ i ], 'g', 10 )
          << QString::number( i < (int) reg_c.size() ? reg_c[ i ] : 0e0, 'g', 10 )
          << QString::number( (int) reg_x[ i ].size() );
      {
         double r2, chi2, red_chi2;
         int    n_err, nparam;
         fit_quality( i, r2, chi2, red_chi2, n_err, nparam );
         bool   have_pts = reg_x[ i ].size() > 0;
         row << ( have_pts ? QString::number( r2, 'g', 10 ) : QString( "" ) )
             << ( n_err > 0 ? QString::number( chi2, 'g', 10 ) : QString( "" ) )
             << ( n_err > nparam ? QString::number( red_chi2, 'g', 10 ) : QString( "" ) );
      }
      map < double, int >::iterator it = out_at.find( reg_q[ i ] );
      if ( it != out_at.end() )
      {
         int k = it->second;
         row << QString::number( pending.I[ k ], 'g', 10 )
             << QString::number( k < (int) pending.I_err.size() ? pending.I_err[ k ] : 0e0, 'g', 10 );
      }
      else
      {
         row << "" << "";
      }
      for ( int k = 0; k < (int) cols.size(); k++ )
      {
         int     c = cols[ k ];
         QString yv, ev, st, mv;
         for ( int j = 0; i < (int) reg_ci.size() && j < (int) reg_ci[ i ].size(); j++ )
         {
            if ( reg_ci[ i ][ j ] == c )
            {
               yv = QString::number( reg_y[ i ][ j ], 'g', 10 );
               ev = ( j < (int) reg_e[ i ].size() ) ? QString::number( reg_e[ i ][ j ], 'g', 10 ) : QString( "" );
               st = "fit";
            }
         }
         for ( int j = 0; i < (int) reg_sci.size() && j < (int) reg_sci[ i ].size(); j++ )
         {
            if ( reg_sci[ i ][ j ] == c )
            {
               mv = QString::number( reg_sy[ i ][ j ], 'g', 10 );
               st = "replaced";
            }
         }
         row << yv << ev << st << mv;
      }
      if ( have_excl )
      {
         row << ( ( i < (int) reg_excl_y.size() && !us_isnan( reg_excl_y[ i ] ) )
                  ? QString::number( reg_excl_y[ i ], 'g', 10 ) : QString( "" ) );
      }
      ts << row.join( "," ) << "\n";
   }
   f.close();
   if ( saxs )
   {
      saxs->editor_msg( "black", QString( us_tr( "Saved the per-q fits of \"%1\" to %2\n" ) ).arg( base ).arg( fn ) );
   }
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::discard()
{
   if ( pending_open )
   {
      pending_open = false;
      US_Hydrodyn_Saxs * saxs = (US_Hydrodyn_Saxs *) saxs_window;
      if ( saxs )
      {
         saxs->editor_msg( "dark red",
                           QString( us_tr( "Discarded the extrapolated curve \"%1\" (not added to the plot).\n" ) )
                           .arg( pending.base_name ) );
      }
   }
   close();
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::keep()
{
   close();                                      // closeEvent adds the curve
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::closeEvent( QCloseEvent * e )
{
   if ( ffmpeg_proc->state() != QProcess::NotRunning )
   {
      encode_cancelled = true;
      ffmpeg_proc->kill();
      ffmpeg_proc->waitForFinished( 5000 );      // encode_done reports it
   }
   movie_stop();
   {
      US_Hydrodyn * ush = (US_Hydrodyn *) us_hydrodyn;
      if ( ush )
      {
         ush->gparams[ "extrap_c0_movie_timer_ms" ] = QString::number( movie_timer_ms );
         ush->gparams[ "extrap_c0_movie_format"   ] =
            rb_save_jpeg->isChecked() ? "jpeg" : rb_save_bmp->isChecked() ? "bmp" : "png";
         ush->gparams[ "extrap_c0_movie_fps"           ] = QString::number( sb_movie_fps->value() );
         ush->gparams[ "extrap_c0_movie_remove_frames" ] = cb_remove_frames->isChecked() ? "1" : "0";
      }
   }
   // closing the viewer keeps the curve; only Discard drops it
   if ( pending_open )
   {
      pending_open = false;
      US_Hydrodyn_Saxs * saxs = (US_Hydrodyn_Saxs *) saxs_window;
      if ( saxs )
      {
         saxs->extrap_c0_commit( pending.base_name, pending.q, pending.I, pending.I_err, pending.conc_tag,
                                 pending.psv, pending.i0se, pending.hdr_extra, pending.offer_save, pending.msg_tail );
      }
   }
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}
