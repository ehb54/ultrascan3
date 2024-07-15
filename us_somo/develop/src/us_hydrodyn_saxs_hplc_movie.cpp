#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_saxs_hplc_movie.h"
//Added by qt3to4:
#include <QHBoxLayout>
#include <QCloseEvent>
#include <QBoxLayout>
#include <QPixmap>
#include <QLabel>
#include <QVBoxLayout>

// design movie controller interface

// still some hiccup on gauss replots w/residuals show:hide under suppress_replot control

#define SLASH QDir::separator()

US_Hydrodyn_Saxs_Hplc_Movie::US_Hydrodyn_Saxs_Hplc_Movie(
                                                         US_Hydrodyn_Saxs_Hplc *hplc_win,
                                                         QWidget *p, 
                                                         const char *
                                                         ) : QDialog ( p )
{
   this->hplc_win                = hplc_win;
   this->hplc_selected_files     = hplc_selected_files;
   this->ush_win                 = (US_Hydrodyn *)(hplc_win->us_hydrodyn);

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle(us_tr("US-SOMO: HPLC/KIN MOVIE"));


   for ( int i = 0; i < hplc_win->lb_files->count(); i++ )
   {
      if ( hplc_win->lb_files->item( i )->isSelected() )
      {
         hplc_selected_files    .push_back( i );
      }
   }

   if ( !hplc_selected_files.size() )
   {
      QMessageBox::warning( this, 
                            windowTitle(),
                            us_tr( "Internal error: HPLC MOVIE called with no curves selected" ) );
      close();
      return;
   }

   hplc_win->lb_files->clearSelection();

   pos = 0;
   last_pos = -1;

   timer_msec = 
      ush_win->gparams.count( "hplc_movie_timer_ms" ) ?
      ush_win->gparams[ "hplc_movie_timer_ms" ].toInt() : 1000;

   last_show_gauss =
      ush_win->gparams.count( "hplc_movie_show_gauss" ) && ush_win->gparams[ "hplc_movie_show_gauss" ] == "true" ?
      true : false;

   last_show_ref =
      ush_win->gparams.count( "hplc_movie_show_ref" ) && ush_win->gparams[ "hplc_movie_show_ref" ] == "true" ?
      true : false;

   last_mono =
      ush_win->gparams.count( "hplc_movie_mono" ) && ush_win->gparams[ "hplc_movie_mono" ] == "true" ?
      true : false;

   setupGUI();
   
   timer      = new QTimer( this );
   connect( timer, SIGNAL(timeout()), this, SLOT( next() ) );

   if ( timer_msec < 50 )
   {
      timer_msec = 50;
   }
      
   update_enables();
   
   global_Xpos += 30;
   global_Ypos += 30;
   
   setGeometry(global_Xpos, global_Ypos, 0, 0 );
   hplc_win->suppress_replot = true;
   plot_colors = hplc_win->plot_colors;
   set_mono();
   update_plot();
   show();
   //    cout << "doin it\n";
   //    QLabel lb( "hithere" , 0 );
   QPainter paint( this );
   //    paint.drawPixmap( 10, 10, &lb->grab() );
   paint.drawText( 0, 0, "HELLO THERE!" );
}

US_Hydrodyn_Saxs_Hplc_Movie::~US_Hydrodyn_Saxs_Hplc_Movie()
{
}

void US_Hydrodyn_Saxs_Hplc_Movie::setupGUI()
{
   int minHeight1 = 24;
   int minHeight3 = 25;
   
   // ------ data section 

   lbl_state = new QLabel( QString( us_tr( "Stopped: %1 of %2" ).arg( pos + 1 ).arg( hplc_selected_files.size() ) ), this);
   lbl_state->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_state->setMinimumHeight(minHeight1);
   lbl_state->setPalette( PALET_LABEL );
   AUTFBACK( lbl_state );
   lbl_state->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_current = new QLabel( hplc_win->lb_files->item( hplc_selected_files[ pos ] )->text(), this);
   lbl_current->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_current->setMinimumHeight(minHeight1);
   lbl_current->setPalette( PALET_LABEL );
   AUTFBACK( lbl_current );
   lbl_current->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   pb_front = new QPushButton( "|<" , this);
   pb_front->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_front->setMinimumHeight(minHeight3);
   pb_front->setPalette( PALET_PUSHB );
   connect(pb_front, SIGNAL(clicked()), SLOT(front()));

   pb_prev = new QPushButton( "<" , this);
   pb_prev->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_prev->setMinimumHeight(minHeight3);
   pb_prev->setPalette( PALET_PUSHB );
   connect(pb_prev, SIGNAL(clicked()), SLOT(prev()));

   pb_slower = new QPushButton( "S-" , this);
   pb_slower->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_slower->setMinimumHeight(minHeight3);
   pb_slower->setPalette( PALET_PUSHB );
   connect(pb_slower, SIGNAL(clicked()), SLOT(slower()));

   pb_start = new QPushButton( "[]" , this);
   pb_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_start->setMinimumHeight(minHeight3);
   pb_start->setPalette( PALET_PUSHB );
   connect(pb_start, SIGNAL(clicked()), SLOT(start()));

   pb_faster = new QPushButton( "S+" , this);
   pb_faster->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_faster->setMinimumHeight(minHeight3);
   pb_faster->setPalette( PALET_PUSHB );
   connect(pb_faster, SIGNAL(clicked()), SLOT(faster()));

   pb_next = new QPushButton( ">" , this);
   pb_next->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_next->setMinimumHeight(minHeight3);
   pb_next->setPalette( PALET_PUSHB );
   connect(pb_next, SIGNAL(clicked()), SLOT(next()));

   pb_end = new QPushButton( ">|" , this);
   pb_end->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_end->setMinimumHeight(minHeight3);
   pb_end->setPalette( PALET_PUSHB );
   connect(pb_end, SIGNAL(clicked()), SLOT(end()));

   cb_show_gauss = new QCheckBox(this);
   cb_show_gauss->setText(us_tr("Show Gaussians "));
   cb_show_gauss->setEnabled( true );
   cb_show_gauss->setChecked( last_show_gauss );
   cb_show_gauss->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_show_gauss->setPalette( PALET_NORMAL );
   AUTFBACK( cb_show_gauss );
   connect( cb_show_gauss, SIGNAL( clicked() ), SLOT( set_show_gauss() ) );

   cb_show_ref = new QCheckBox(this);
   cb_show_ref->setText(us_tr("Show reference "));
   cb_show_ref->setEnabled( true );
   cb_show_ref->setChecked( last_show_ref );
   cb_show_ref->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_show_ref->setPalette( PALET_NORMAL );
   AUTFBACK( cb_show_ref );
   connect( cb_show_ref, SIGNAL( clicked() ), SLOT( set_show_ref() ) );

   cb_mono = new QCheckBox(this);
   cb_mono->setText(us_tr("Monochrome   File save as type:"));
   cb_mono->setEnabled( true );
   cb_mono->setChecked( last_mono );
   cb_mono->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_mono->setPalette( PALET_NORMAL );
   AUTFBACK( cb_mono );
   connect( cb_mono, SIGNAL( clicked() ), SLOT( set_mono() ) );

   cb_save = new QCheckBox(this);
   cb_save->setText(us_tr("Save to file  Prefix:"));
   cb_save->setEnabled( true );
   cb_save->setChecked( false );
   cb_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_save->setPalette( PALET_NORMAL );
   AUTFBACK( cb_save );
   connect( cb_save, SIGNAL( clicked() ), SLOT( set_save() ) );

   le_save = new QLineEdit( this );    le_save->setObjectName( "le_save Line Edit" );
   le_save->setText( "" );
   le_save->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_save->setPalette( PALET_NORMAL );
   AUTFBACK( le_save );
   le_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   rb_save_png = new QRadioButton( us_tr("png"), this);
   rb_save_png->setEnabled(true);
   rb_save_png->setChecked(true);
   rb_save_png->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_save_png->setPalette( PALET_NORMAL );
   AUTFBACK( rb_save_png );

   rb_save_jpeg = new QRadioButton( us_tr("jpeg"), this);
   rb_save_jpeg->setEnabled(true);
   rb_save_jpeg->setChecked(false);
   rb_save_jpeg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_save_jpeg->setPalette( PALET_NORMAL );
   AUTFBACK( rb_save_jpeg );

   rb_save_bmp = new QRadioButton( us_tr("bmp"), this);
   rb_save_bmp->setEnabled(true);
   rb_save_bmp->setChecked(false);
   rb_save_bmp->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_save_bmp->setPalette( PALET_NORMAL );
   AUTFBACK( rb_save_bmp );

#if QT_VERSION < 0x040000
   bg_save = new QGroupBox( this );
   int bg_pos = 0;
   bg_save->setExclusive(true);
   bg_save->addButton( rb_save_png, bg_pos++ );
   bg_save->addButton( rb_save_jpeg, bg_pos++ );
   bg_save->addButton( rb_save_bmp, bg_pos++ );
#else
   bg_save = new QGroupBox();
   bg_save->setFlat( true );

   {
      QHBoxLayout * bl = new QHBoxLayout; bl->setContentsMargins( 0, 0, 0, 0 ); bl->setSpacing( 0 );
      bl->addWidget( rb_save_png );
      bl->addWidget( rb_save_jpeg );
      bl->addWidget( rb_save_bmp );
      bg_save->setLayout( bl );
   }
#endif

   cb_save_overwrite = new QCheckBox(this);
   cb_save_overwrite->setText(us_tr("Overwrite"));
   cb_save_overwrite->setEnabled( true );
   cb_save_overwrite->setChecked( false );
   cb_save_overwrite->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_save_overwrite->setPalette( PALET_NORMAL );
   AUTFBACK( cb_save_overwrite );

   pb_help = new QPushButton(us_tr("Help"), this );
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(us_tr("Close"), this );
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // -------- build layout

   QVBoxLayout * background = new QVBoxLayout(this); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );

   {
      QBoxLayout * bl_tl = new QHBoxLayout(); bl_tl->setContentsMargins( 0, 0, 0, 0 ); bl_tl->setSpacing( 0 );
      bl_tl->addWidget( lbl_state );
      bl_tl->addWidget( lbl_current );
      background->addLayout( bl_tl );
   }

   {
      QBoxLayout * bl_tl = new QHBoxLayout(); bl_tl->setContentsMargins( 0, 0, 0, 0 ); bl_tl->setSpacing( 0 );
      bl_tl->addWidget( pb_front );
      bl_tl->addWidget( pb_prev );
      bl_tl->addWidget( pb_slower );
      bl_tl->addWidget( pb_start );
      bl_tl->addWidget( pb_faster );
      bl_tl->addWidget( pb_next );
      bl_tl->addWidget( pb_end );
      background->addLayout( bl_tl );
   }

   {
      QBoxLayout * bl_tl = new QHBoxLayout(); bl_tl->setContentsMargins( 0, 0, 0, 0 ); bl_tl->setSpacing( 0 );
      bl_tl->addWidget( cb_show_gauss );
      bl_tl->addWidget( cb_show_ref );
      bl_tl->addWidget( cb_mono );
      bl_tl->addWidget( rb_save_png );
      bl_tl->addWidget( rb_save_jpeg );
      bl_tl->addWidget( rb_save_bmp );
      background->addLayout( bl_tl );
   }

   {
      QBoxLayout * bl_tl = new QHBoxLayout(); bl_tl->setContentsMargins( 0, 0, 0, 0 ); bl_tl->setSpacing( 0 );
      bl_tl->addWidget( cb_save );
      bl_tl->addWidget( le_save );
      bl_tl->addWidget( cb_save_overwrite );
      background->addLayout( bl_tl );
   }

   {
      QBoxLayout * bottom = new QHBoxLayout(); bottom->setContentsMargins( 0, 0, 0, 0 ); bottom->setSpacing( 0 );
      bottom->addWidget( pb_help );
      bottom->addWidget( pb_cancel );
      background->addSpacing( 2 );
      background->addLayout( bottom );
   }
}

void US_Hydrodyn_Saxs_Hplc_Movie::cancel()
{
   close();
}

void US_Hydrodyn_Saxs_Hplc_Movie::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo/somo_saxs_hplc_movie.html");
}

void US_Hydrodyn_Saxs_Hplc_Movie::closeEvent(QCloseEvent *e)
{
   hplc_win->plot_colors = plot_colors;
   hplc_win->disable_updates = true;
   if ( hplc_win->current_mode == hplc_win->MODE_GAUSSIAN )
   {
      hplc_win->wheel_cancel();
   }
   hplc_win->lb_files->clearSelection();
   for ( int i = 0; i < (int) hplc_selected_files.size(); ++i )
   {
      hplc_win->lb_files->item( hplc_selected_files[ i ])->setSelected( true );
   }
   hplc_win->suppress_replot = false;
   hplc_win->plot_files();
   hplc_win->disable_updates = false;

   ush_win->gparams[ "hplc_movie_timer_ms"   ] = QString( "%1" ).arg( timer_msec );
   ush_win->gparams[ "hplc_movie_show_gauss" ] = cb_show_gauss->isChecked() ? "true" : "false";
   ush_win->gparams[ "hplc_movie_show_ref"   ] = cb_show_ref->isChecked() ? "true" : "false";
   e->accept();
}

void US_Hydrodyn_Saxs_Hplc_Movie::front()
{
   pos = 0;
   update_plot();
}

void US_Hydrodyn_Saxs_Hplc_Movie::prev()
{
   if ( pos > 0 )
   {
      pos--;
      update_plot();
   }
}

void US_Hydrodyn_Saxs_Hplc_Movie::slower()
{
   timer_msec *= 2;
   if ( timer_msec > 5000 )
   {
      timer_msec = 5000;
   }
   if ( timer->isActive() )
   {
      timer->setInterval( timer_msec );
   }
}

void US_Hydrodyn_Saxs_Hplc_Movie::start()
{
   if ( timer->isActive() )
   {
      timer->stop();
      pb_start->setText( "[]" );
      lbl_state  -> setText( QString( us_tr( "%1: %2 of %3" ).arg( us_tr( timer->isActive() ? "Running" : "Stopped" ) ).arg( pos + 1 ).arg( hplc_selected_files.size() ) ) );
   } else {
      timer->start( timer_msec );
      pb_start->setText( "||" );
      lbl_state  -> setText( QString( us_tr( "%1: %2 of %3" ).arg( us_tr( timer->isActive() ? "Running" : "Stopped" ) ).arg( pos + 1 ).arg( hplc_selected_files.size() ) ) );
      if ( cb_save->isChecked() )
      {
         save_plot();
      }
   }
}

void US_Hydrodyn_Saxs_Hplc_Movie::faster()
{
   timer_msec /= 2;
   if ( timer_msec < 50 )
   {
      timer_msec = 50;
   }
   if ( timer->isActive() )
   {
      timer->setInterval( timer_msec );
   }
}

void US_Hydrodyn_Saxs_Hplc_Movie::next()
{
   if ( pos < (int)hplc_selected_files.size() - 1 )
   {
      pos++;
      update_plot();
   } else {
      if ( timer->isActive() )
      {
         start();
         cb_save          ->setChecked( false );
         cb_save_overwrite->setChecked( false );
      }
   }
}

void US_Hydrodyn_Saxs_Hplc_Movie::end()
{
   pos = hplc_selected_files.size() - 1;
   update_plot();
}

void US_Hydrodyn_Saxs_Hplc_Movie::set_save()
{
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc_Movie::set_show_gauss()
{
   update_plot();
}

void US_Hydrodyn_Saxs_Hplc_Movie::set_show_ref()
{
   update_plot();
}

void US_Hydrodyn_Saxs_Hplc_Movie::set_mono()
{
   if ( cb_mono->isChecked() )
   {
      hplc_win->plot_colors.clear( );
      hplc_win->plot_colors.push_back( plot_colors[ 0 ] );
   } else {
      hplc_win->plot_colors = plot_colors;
   }
   update_plot();
}

void US_Hydrodyn_Saxs_Hplc_Movie::update_enables()
{
   le_save->setEnabled( cb_save->isChecked() );
}

void US_Hydrodyn_Saxs_Hplc_Movie::update_plot()
{
   if ( last_pos != pos ||
        cb_show_gauss->isChecked() != last_show_gauss ||
        cb_show_ref  ->isChecked() != last_show_ref )
   {
      last_pos        = pos;
      last_show_gauss = cb_show_gauss->isChecked();
      last_show_ref   = cb_show_ref  ->isChecked();

      hplc_win->lb_files->clearSelection();
      if ( cb_show_ref->isChecked() )
      {
         hplc_win->plot_ref->show();
      } else {
         hplc_win->plot_ref->hide();
      } 
      hplc_win->lb_files->item( hplc_selected_files[ pos ])->setSelected( true );

      if ( hplc_win->current_mode == hplc_win->MODE_GAUSSIAN )
      {
         hplc_win->wheel_cancel();
      }
      if ( cb_show_gauss->isChecked() &&
           hplc_win->f_is_time.count( hplc_win->lb_files->item( hplc_selected_files[ pos ] )->text() ) &&
           hplc_win->f_is_time[ hplc_win->lb_files->item( hplc_selected_files[ pos ] )->text() ] )
      {
         hplc_win->gauss_start();
      }
      // else {
      // hplc_win->plot_files();
      // }

      hplc_win->plot_dist  ->replot();
      hplc_win->plot_ref   ->replot();
      hplc_win->plot_errors->replot();
      lbl_state  -> setText( QString( us_tr( "%1: %2 of %3" ).arg( us_tr( timer->isActive() ? "Running" : "Stopped" ) ).arg( pos + 1 ).arg( hplc_selected_files.size() ) ) );
      lbl_current-> setText( hplc_win->lb_files->item( hplc_selected_files[ pos ] )->text() );

      if ( cb_save->isChecked() )
      {
         save_plot();
      }
   }
}

void US_Hydrodyn_Saxs_Hplc_Movie::save_plot()
{
   if ( hplc_win->plot_errors->isVisible() )
   {
      if ( hplc_win->plot_ref->isVisible() )
      {
         save_plot( hplc_win->plot_dist, hplc_win->plot_ref, hplc_win->plot_errors, le_save->text() );
      } else {
         save_plot( hplc_win->plot_dist, hplc_win->plot_errors, le_save->text() );
      }
   } else {
      if ( hplc_win->plot_ref->isVisible() )
      {
         save_plot( hplc_win->plot_dist, hplc_win->plot_ref, le_save->text() );
      } else {
         save_plot( hplc_win->plot_dist, le_save->text() );
      }
   }
}
   
void US_Hydrodyn_Saxs_Hplc_Movie::save_plot( QWidget *plot, QString tag )
{
   int mypos = pos;
   QPixmap qPix = plot->grab();
   if( qPix.isNull() )
   {
      // us_qdebug( "Failed to capture the plot for saving\n" );
      return;
   }
   save_plot( qPix, tag, mypos );
}

void US_Hydrodyn_Saxs_Hplc_Movie::join_maps( QPixmap & m1, QPixmap & m2 )
{
   int m1h = m1.height();
   int m2h = m2.height();

#if QT_VERSION < 0x040000
   m1.resize( m1.width() > m2.width() ? m1.width() : m2.width(), m1h + m2h );
#else
   m1 = m1.copy( 0, 0, m1.width() > m2.width() ? m1.width() : m2.width(), m1h + m2h );
#endif
   QPainter paint( &m1 );
   paint.drawPixmap( 0, m1h, m2 );
}

void US_Hydrodyn_Saxs_Hplc_Movie::save_plot( QWidget *plot, QWidget *plot2, QWidget *plot3, QString tag )
{
   int mypos = pos;
   QPixmap qPix  = plot->grab();
   QPixmap qPix2 = plot2->grab();
   QPixmap qPix3 = plot3->grab();
   if( qPix.isNull() || qPix2.isNull() || qPix3.isNull() )
   {
      // us_qdebug( "Failed to capture the plot for saving\n" );
      return;
   }
   join_maps( qPix, qPix2 );
   join_maps( qPix, qPix3 );
   save_plot( qPix, tag, mypos );
}

void US_Hydrodyn_Saxs_Hplc_Movie::save_plot( QWidget *plot, QWidget *plot2, QString tag )
{
   int mypos = pos;
   QPixmap qPix  = plot->grab();
   QPixmap qPix2 = plot2->grab();
   if( qPix.isNull() || qPix2.isNull() )
   {
      // us_qdebug( "Failed to capture the plot for saving\n" );
      return;
   }
   join_maps( qPix, qPix2 );
   save_plot( qPix, tag, mypos );
}

void US_Hydrodyn_Saxs_Hplc_Movie::save_plot( QPixmap & qPix, QString tag, int mypos )
{
   QPainter paint( &qPix );
   paint.setPen( Qt::blue );
   paint.setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold) );
   // paint.drawText( 5, 5, hplc_win->lb_files->item( hplc_selected_files[ mypos ] )->text() );
   paint.drawText( qPix.rect(), Qt::AlignBottom | Qt::AlignLeft, hplc_win->lb_files->item( hplc_selected_files[ mypos ] )->text() );
   QString frame = QString( "%1" ).arg( mypos + 1 );
   while( frame.length() < 5 )
   {
      frame = "0" + frame;
   }

   if ( rb_save_png->isChecked() )
   {
      frame += ".png";
   } else {
      if ( rb_save_jpeg->isChecked() )
      {
         frame += ".jpeg";
      } else {
         frame += ".bmp";
      }
   }

   QString f( ush_win->somo_dir + QDir::separator() + "saxs" + QDir::separator() + "tmp" + QDir::separator() +
              tag + "_" + frame );

   if ( !cb_save_overwrite->isChecked() && QFile::exists( f ) )
   {
      return;
   }

   if ( rb_save_png->isChecked() )
   {
      qPix.save( f, "PNG" );
   } else {
      if ( rb_save_jpeg->isChecked() )
      {
         qPix.save( f, "JPEG" );
      } else {
         qPix.save( f, "BMP" );
      }
   }
}

