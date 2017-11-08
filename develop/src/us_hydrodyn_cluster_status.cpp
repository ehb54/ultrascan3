#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_cluster.h"
#include "../include/us_hydrodyn_cluster_status.h"
//Added by qt3to4:
#include <QTextStream>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
 //#include <Q3PopupMenu>
#include <QVBoxLayout>
#include <QBoxLayout>
#include <QCloseEvent>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#define SLASH QDir::separator()

US_Hydrodyn_Cluster_Status::US_Hydrodyn_Cluster_Status(
                                         void *us_hydrodyn, 
                                         QWidget *p, 
                                         const char *name
                                         ) : QDialog( p )
{
   this->us_hydrodyn = us_hydrodyn;
   setWindowTitle(us_tr("US-SOMO: Cluster Status"));
   cluster_window = (void *)p;
   USglobal = new US_Config();

   stopFlag = false;
   comm_active = false;
   system_proc_active = false;
   processing_active = false;
   disable_updates = false;

   ftp_file = ( QFile * ) 0;

   pkg_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "cluster";
   submitted_dir = pkg_dir + SLASH + "submitted";
   completed_dir = pkg_dir  + SLASH + "completed";
   QDir::setCurrent( submitted_dir );

   cluster_id      = ((US_Hydrodyn_Cluster *) cluster_window )->cluster_config[ "userid" ];
   cluster_pw      = ((US_Hydrodyn_Cluster *) cluster_window )->cluster_config[ "userpw" ];

   submit_url      = ((US_Hydrodyn_Cluster *) cluster_window )->cluster_config[ "server" ];
   submit_url_host = submit_url;
   submit_url_port = submit_url;

   submit_url_host.replace( QRegExp( ":.*$" ), "" );
   submit_url_port.replace( QRegExp( "^.*:" ), "" );

   // staging is now job specific (ugh)
   // stage_url       = ((US_Hydrodyn_Cluster *) cluster_window )->stage_url;
   // stage_path      = stage_url;
   // stage_url_path  = stage_url;

   // stage_url     .replace( QRegExp( ":.*$" ), "" );
   // stage_path    .replace( QRegExp( "^.*:" ), "" );
   // stage_url_path += QString( "%1%2%3" ).arg( QDir::separator() ).arg( cluster_id ).arg( QDir::separator() );

   if ( cluster_id.isEmpty() ||
        cluster_pw.isEmpty() )
   {
      QMessageBox::information( this, 
                                us_tr("US-SOMO: Cluster Jobs"),
                                us_tr("Cluster credentials must be set in Cluster Config before continuing"),
                                0 );
      return;
   }

   if ( !update_files( false ) )
   {
      QMessageBox::information( this, 
                                us_tr("US-SOMO: Cluster Status"),
                                us_tr("No unretrieved submitted jobs found"),
                                0 );
   }

   setupGUI();
   editor_msg("blue", "THIS WINDOW IS UNDER DEVELOPMENT." );

   update_files();
   update_enables();
   refresh();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 700, 600 );

}

US_Hydrodyn_Cluster_Status::~US_Hydrodyn_Cluster_Status()
{
}

unsigned int US_Hydrodyn_Cluster_Status::update_files( bool set_lv_files )
{
   disable_updates = true;
   files.clear( );

   // traverse directory and build up files
   QDir::setCurrent( submitted_dir );
   QDir qd;
   QStringList tgz_files = qd.entryList( QStringList() << "*.tgz" );
   QStringList tar_files = qd.entryList( QStringList() << "*.tar" );

   // QStringList all_files = QStringList::split( "\n", 
   //                                             tgz_files.join("\n") + 
   //                                             ( tgz_files.size() ? "\n" : "" ) +
   //                                             tar_files.join("\n") );

   QStringList all_files;
   {
      QString qs =
         tgz_files.join("\n") + 
         ( tgz_files.size() ? "\n" : "" ) +
         tar_files.join("\n")
         ;
      all_files = (qs ).split( "\n" , QString::SkipEmptyParts );
   }

   for ( unsigned int i = 0; i < (unsigned int)all_files.size(); i++ )
   {
      if ( !all_files[ i ].contains( "_out", Qt::CaseInsensitive ) )
      {
         files << all_files[ i ];
      }
   }
   
   if ( set_lv_files )
   {
      lv_files->clear( );
      for ( unsigned int i = 0; i < (unsigned int)files.size(); i++ )
      {
#if QT_VERSION < 0x040000
         new QTreeWidgetItem( lv_files, files[ i ], "unknown", "", QFileInfo( files[ i ] ).created().toString() );
#else
         lv_files->addTopLevelItem( new QTreeWidgetItem(
                                                        QStringList()
                                                        << files[ i ]
                                                        << "unknown"
                                                        << ""
                                                        << QFileInfo( files[ i ] ).created().toString()
                                                        ) );
#endif
      }
   }

   disable_updates = false;
   return files.size();
}

void US_Hydrodyn_Cluster_Status::setupGUI()
{
   int minHeight1 = 30;

   lbl_title = new QLabel( us_tr( " Cluster job status" ), this);
   lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette( PALET_FRAME );
   AUTFBACK( lbl_title );
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lv_files = new QTreeWidget(this);
   lv_files->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lv_files->setMinimumHeight(minHeight1 * 3);
   lv_files->setPalette( PALET_EDIT );
   AUTFBACK( lv_files );
   lv_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   lv_files->setEnabled(true);
   lv_files->setSelectionMode( QAbstractItemView::MultiSelection );

#if QT_VERSION < 0x040000
   lv_files->addColumn( us_tr( "Name" ) );
   lv_files->addColumn( us_tr( "Status" ) );
   lv_files->addColumn( us_tr( "Additional Info" ) );
   lv_files->addColumn( us_tr( "Date created" ) );
#else
   lv_files->setColumnCount( 4 );
   lv_files->setHeaderLabels( QStringList()
                              << us_tr( "Name" )
                              << us_tr( "Status" )
                              << us_tr( "Additional Info" )
                              << us_tr( "Date created" )
                              );
#endif   
   connect( lv_files, SIGNAL( itemSelectionChanged() ), SLOT( update_enables() ) );

   pb_refresh = new QPushButton(us_tr("Refresh status"), this);
   pb_refresh->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_refresh->setMinimumHeight(minHeight1);
   pb_refresh->setPalette( PALET_PUSHB );
   connect( pb_refresh, SIGNAL( clicked() ), SLOT( refresh() ) );

   pb_remove = new QPushButton(us_tr("Cancel selected jobs"), this);
   pb_remove->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_remove->setMinimumHeight(minHeight1);
   pb_remove->setPalette( PALET_PUSHB );
   connect( pb_remove, SIGNAL( clicked() ), SLOT( remove() ) );

   pb_retrieve_selected = new QPushButton( us_tr("Retrieve selected results" ), this);
   pb_retrieve_selected->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_retrieve_selected->setMinimumHeight(minHeight1);
   pb_retrieve_selected->setPalette( PALET_PUSHB );
   connect( pb_retrieve_selected, SIGNAL( clicked() ), SLOT( retrieve_selected() ) );

   pb_retrieve = new QPushButton( us_tr("Retrieve all available results" ), this);
   pb_retrieve->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_retrieve->setMinimumHeight(minHeight1);
   pb_retrieve->setPalette( PALET_PUSHB );
   connect( pb_retrieve, SIGNAL( clicked() ), SLOT( retrieve() ) );

   progress = new QProgressBar( this );
   progress->setMinimumHeight( minHeight1 );
   progress->setPalette( PALET_NORMAL );
   AUTFBACK( progress );
   progress->reset();

#if QT_VERSION < 0x050000
   connect( &ftp, SIGNAL( dataTransferProgress ( int, int  ) ), progress, SLOT( setValue        ( int, int  ) ) );
   connect( &ftp, SIGNAL( stateChanged         ( int       ) ), this    , SLOT( ftp_stateChanged   ( int       ) ) );
   connect( &ftp, SIGNAL( commandStarted       ( int       ) ), this    , SLOT( ftp_commandStarted ( int       ) ) );
   connect( &ftp, SIGNAL( commandFinished      ( int, bool ) ), this    , SLOT( ftp_commandFinished( int, bool ) ) );
   connect( &ftp, SIGNAL( done                 ( bool      ) ), this    , SLOT( ftp_done           ( bool      ) ) );
#endif

   editor = new QTextEdit(this);
   editor->setPalette( PALET_NORMAL );
   AUTFBACK( editor );
   editor->setReadOnly(true);

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
   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight1);

   m = new QMenuBar( frame );    m->setObjectName( "menu" );
   m->setMinimumHeight(minHeight1 - 5);
   m->setPalette( PALET_NORMAL );
   AUTFBACK( m );
 //   Q3PopupMenu * file = new Q3PopupMenu(editor);
   m->insertItem( us_tr("&File"), file );
   file->insertItem( us_tr("Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
   file->insertItem( us_tr("Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );
   file->insertItem( us_tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );
# endif
#else
   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight1);

   m = new QMenuBar( frame );    m->setObjectName( "menu" );
   m->setMinimumHeight(minHeight1 - 5);
   m->setPalette( PALET_NORMAL );
   AUTFBACK( m );

   {
      QMenu * new_menu = m->addMenu( us_tr( "&File" ) );

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
   editor->setMinimumHeight(100);

   pb_stop = new QPushButton(us_tr("Stop"), this);
   pb_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_stop->setMinimumHeight(minHeight1);
   pb_stop->setPalette( PALET_PUSHB );
   connect(pb_stop, SIGNAL(clicked()), SLOT(stop()));
   pb_stop->setEnabled( false );

   pb_help = new QPushButton(us_tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(us_tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // build layout

   QHBoxLayout * hbl_buttons1 = new QHBoxLayout(); hbl_buttons1->setContentsMargins( 0, 0, 0, 0 ); hbl_buttons1->setSpacing( 0 );
   hbl_buttons1->addSpacing( 4 );
   hbl_buttons1->addWidget ( pb_refresh );
   hbl_buttons1->addSpacing( 4 );
   hbl_buttons1->addWidget ( pb_remove );
   hbl_buttons1->addSpacing( 4 );
   hbl_buttons1->addWidget ( pb_retrieve_selected );
   hbl_buttons1->addSpacing( 4 );
   hbl_buttons1->addWidget ( pb_retrieve );
   hbl_buttons1->addSpacing( 4 );

   QHBoxLayout * hbl_bottom = new QHBoxLayout(); hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_stop );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_cancel );
   hbl_bottom->addSpacing( 4 );

   QBoxLayout * vbl_editor_group = new QVBoxLayout(0); vbl_editor_group->setContentsMargins( 0, 0, 0, 0 ); vbl_editor_group->setSpacing( 0 );
#if QT_VERSION < 0x040000 || !defined(Q_OS_MAC)
   vbl_editor_group->addWidget(frame);
#endif
   vbl_editor_group->addWidget(editor);

   QVBoxLayout * background = new QVBoxLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );
   background->addSpacing( 4);
   background->addWidget ( lbl_title );
   background->addSpacing( 4 );
   background->addWidget ( lv_files );
   background->addSpacing( 4 );
   background->addLayout ( hbl_buttons1 );
   background->addSpacing( 4 );
   background->addWidget ( progress );
   background->addSpacing( 4 );
   background->addLayout ( vbl_editor_group );
   background->addSpacing( 4 );
   background->addLayout ( hbl_bottom );
   background->addSpacing( 4);
}

void US_Hydrodyn_Cluster_Status::cancel()
{
   if ( comm_active )
   {
#if QT_VERSION < 0x050000
      submit_http.abort();
#endif
   }
   if ( system_proc_active )
   {
      system_proc->terminate();
      QTimer::singleShot( 2500, system_proc, SLOT( kill() ) );
   }
   close();
}

void US_Hydrodyn_Cluster_Status::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo/somo_cluster_status.html");
}

void US_Hydrodyn_Cluster_Status::closeEvent(QCloseEvent *e)
{
   if ( comm_active )
   {
#if QT_VERSION < 0x050000
      submit_http.abort();
#endif
   }
   if ( system_proc_active )
   {
      system_proc->terminate();
      QTimer::singleShot( 2500, system_proc, SLOT( kill() ) );
   }
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}


void US_Hydrodyn_Cluster_Status::clear_display()
{
   editor->clear( );
   editor->append("\n\n");
}

void US_Hydrodyn_Cluster_Status::update_font()
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

void US_Hydrodyn_Cluster_Status::save()
{
   QString fn;
   fn = QFileDialog::getSaveFileName( this , windowTitle() , QString::null , QString::null );
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

void US_Hydrodyn_Cluster_Status::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->textColor();
   editor->setTextColor(color);
   editor->append(msg);
   editor->setTextColor(save_color);
   editor->verticalScrollBar()->setValue(editor->verticalScrollBar()->maximum());
   // qApp->processEvents();
}
   
void US_Hydrodyn_Cluster_Status::update_enables()
{
   if ( !disable_updates )
   {
      bool any_selected = false;
      bool any_completed = false;
      bool any_selected_completed = false;
#if QT_VERSION < 0x040000
      QTreeWidgetItem *lvi = lv_files->firstChild();
      if ( lvi )
      {
         do {
            if ( lvi->isSelected() )
            {
               any_selected = true;
            }
            if ( lvi->text( 1 ) == "COMPLETED" )
            {
               any_completed = true;
               if ( lvi->isSelected() )
               {
                  any_selected_completed = true;
               }
            }
            if ( any_selected_completed )
            {
               break;
            }
         } while ( ( lvi = lvi->nextSibling() ) );
      }
#else
      QTreeWidgetItemIterator it( lv_files );
      QTreeWidgetItem *lvi;

      while ( *it ) {
         lvi = *it;
         if ( lvi->isSelected() )
         {
            any_selected = true;
         }
         if ( lvi->text( 1 ) == "COMPLETED" )
         {
            any_completed = true;
            if ( lvi->isSelected() )
            {
               any_selected_completed = true;
            }
         }
         if ( any_selected_completed )
         {
            break;
         }
         ++it;
      }
#endif      
      pb_refresh          ->setEnabled( !processing_active && !comm_active && !system_proc_active );
      pb_remove           ->setEnabled( !processing_active && !comm_active && !system_proc_active && any_selected  );
      pb_retrieve_selected->setEnabled( !processing_active && !comm_active && !system_proc_active && any_selected_completed );
      pb_retrieve         ->setEnabled( !processing_active && !comm_active && !system_proc_active && any_completed );
   } 
   if ( processing_active )
   {
      pb_refresh          ->setEnabled( false );
      pb_remove           ->setEnabled( false );
      pb_retrieve_selected->setEnabled( false );
      pb_retrieve         ->setEnabled( false );
   }
}

void US_Hydrodyn_Cluster_Status::refresh()
{
   if ( comm_active )
   {
#if QT_VERSION < 0x050000
      submit_http.abort();
#endif
   }
   if ( system_proc_active )
   {
      system_proc->terminate();
      QTimer::singleShot( 2500, system_proc, SLOT( kill() ) );
   }
   get_status();
   //   update_files();
}

void US_Hydrodyn_Cluster_Status::remove()
{
   cancel_selected();
   // also ask to push back jobs to unsubmitted or completely remove
   update_enables();
}

void US_Hydrodyn_Cluster_Status::retrieve()
{
   //   update_files();
   comm_mode = "retrieve";
   jobs.clear( );
   
   QStringList qsl_submit;

#if QT_VERSION < 0x040000
   QTreeWidgetItem *lvi = lv_files->firstChild();
   if ( lvi )
   {
      do {
         if ( lvi->text( 1 ) == "COMPLETED" )
         {
            jobs[ lvi ] = "retrieve results";
         }
      } while ( ( lvi = lvi->nextSibling() ) );
      processing_active = true;
      stopFlag = false;
      pb_stop->setEnabled( true );
      editor_msg( "black", us_tr( "retrieving results" ) );
      update_enables();
      emit next_status();
   } else {
      stopFlag = false;
      pb_stop->setEnabled( false );
   }
#else
   QTreeWidgetItemIterator it( lv_files );
   QTreeWidgetItem *lvi;
   if ( *it )
   {
      while ( *it ) {
         lvi = *it;
         if ( lvi->text( 1 ) == "COMPLETED" )
         {
            jobs[ lvi ] = "retrieve results";
         }
         ++it;
      }
      processing_active = true;
      stopFlag = false;
      pb_stop->setEnabled( true );
      editor_msg( "black", us_tr( "retrieving results" ) );
      update_enables();
      emit next_status();
   } else {
      stopFlag = false;
      pb_stop->setEnabled( false );
   }
#endif
}

void US_Hydrodyn_Cluster_Status::retrieve_selected()
{
   //   update_files();
   comm_mode = "retrieve";
   jobs.clear( );
   
   QStringList qsl_submit;
#if QT_VERSION < 0x040000
   QTreeWidgetItem *lvi = lv_files->firstChild();
   if ( lvi )
   {
      do {
         if ( lvi->text( 1 ) == "COMPLETED" && lvi->isSelected() )
         {
            jobs[ lvi ] = "retrieve results";
         }
      } while ( ( lvi = lvi->nextSibling() ) );
      processing_active = true;
      stopFlag = false;
      pb_stop->setEnabled( true );
      editor_msg( "black", us_tr( "retrieving results" ) );
      update_enables();
      emit next_status();
   } else {
      stopFlag = false;
      pb_stop->setEnabled( false );
   }
#else
   QTreeWidgetItemIterator it( lv_files, QTreeWidgetItemIterator::Selected );
   QTreeWidgetItem *lvi;
   if ( *it )
   {
      while ( *it ) {
         lvi = *it;
         if ( lvi->text( 1 ) == "COMPLETED" )
         {
            jobs[ lvi ] = "retrieve results";
         }
         ++it;
      }
      processing_active = true;
      stopFlag = false;
      pb_stop->setEnabled( true );
      editor_msg( "black", us_tr( "retrieving results" ) );
      update_enables();
      emit next_status();
   } else {
      stopFlag = false;
      pb_stop->setEnabled( false );
   }
#endif   
}

void US_Hydrodyn_Cluster_Status::cancel_selected()
{
   update_enables();
   comm_mode = "cancel";
   jobs.clear( );
   
   QStringList qsl_submit;
#if QT_VERSION < 0x040000
   QTreeWidgetItem *lvi = lv_files->firstChild();
   if ( lvi )
   {
      do {
         if ( lvi->isSelected() )
         {
            jobs[ lvi ] = "cancel job";
         }
      } while ( ( lvi = lvi->nextSibling() ) );
      emit next_status();
   }
#else
   QTreeWidgetItemIterator it( lv_files, QTreeWidgetItemIterator::Selected );
   QTreeWidgetItem *lvi;
   if ( *it )
   {
      while ( *it ) {
         lvi = *it;
         jobs[ lvi ] = "cancel job";
         ++it;
      };
      emit next_status();
   }
#endif
}

void US_Hydrodyn_Cluster_Status::get_status()
{
   // update_enables();
   comm_mode = "status";
   jobs.clear( );
   
   QStringList qsl_submit;
#if QT_VERSION < 0x040000
   QTreeWidgetItem *lvi = lv_files->firstChild();
   if ( lvi )
   {
      do {
         jobs[ lvi ] = "get status";
         lvi->setText( 2, "" );
      } while ( ( lvi = lvi->nextSibling() ) );
      
      emit next_status();
   }
#else
   QTreeWidgetItemIterator it( lv_files );
   QTreeWidgetItem *lvi;
   if ( *it )
   {
      while ( *it ) {
         lvi = *it;
         jobs[ lvi ] = "get status";
         lvi->setText( 2, "" );
         ++it;
      }
      emit next_status();
   }
#endif
}

void US_Hydrodyn_Cluster_Status::next_status()
{
   if ( stopFlag )
   {
      editor_msg( "red", us_tr( "stopped by user request" ) );
      stopFlag = false;
      update_files();
      refresh();
      return;
   }

   if ( comm_active )
   {
      return;
   }
   if ( system_proc_active )
   {
      return;
   }

   bool ok = true;
   for ( map < QTreeWidgetItem *, QString >::iterator it = jobs.begin();
         it != jobs.end();
         it++ )
   {
      if ( it->second.contains( "failed" ) )
      {
         ok = false;
      }
      if ( it->second == "get status" ||
           it->second == "cancel job" )
      {
         next_to_process = it->first;
         emit get_next_status();
         return;
      }
      if ( it->second == "retrieve results" )
      {
         next_to_process = it->first;
         emit get_next_retrieve();
         return;
      }
   }

   if ( ok )
   {
      if ( comm_mode == "status" )
      {
         // editor_msg( "black", us_tr( "done refreshing job status" ) );
      } 
      if ( comm_mode == "cancel" )
      {
         editor_msg( "black", us_tr( "done canceling jobs" ) );
         complete_remove();
      } 
      if ( comm_mode == "retrieve" )
      {
         editor_msg( "black", us_tr( "done retrieving jobs" ) );
         complete_retrieve();
      } 
   } else {
      if ( comm_mode == "status" )
      {
         editor_msg( "red", us_tr( "job status refresh had errors" ) );
      } 
      if ( comm_mode == "cancel" )
      {
         editor_msg( "red", us_tr( "cancel job had errors" ) );
      } 
      if ( comm_mode == "retrieve" )
      {
         editor_msg( "red", us_tr( "retrieve results had errors" ) );
      } 
   }

   processing_active = false;
   update_enables();
}

void US_Hydrodyn_Cluster_Status::complete_remove()
{
   if ( QMessageBox::question(
                              this,
                              us_tr( "US-SOMO: Cluster Status" ),
                              us_tr( "What do you want to do with the canceled jobs?" ),
                              us_tr( "&Push back to unsubmitted" ),
                              us_tr( "&Remove completely" ),
                              QString::null, 0, 1 ) )
   {
      editor_msg( "black", "completely removing" );
      for ( map < QTreeWidgetItem *, QString >::iterator it = jobs.begin();
            it != jobs.end();
            it++ )
      {
         it->first->setText( 2, "Removing" );
         editor_msg( "black", QString( us_tr( "Removing %1" ) ).arg( it->first->text( 0 ) ) );
         if ( !QFile::remove( it->first->text( 0 ) ) )
         {
            editor_msg( "red", QString( us_tr( "Error removing %1" ) ).arg( it->first->text( 0 ) ) );
         }
      }
      editor_msg( "black", "Removing done" );
   } else {
      
      editor_msg( "black", "Push back to unsubmitted" );
      
      US_File_Util ufu;
      
      for ( map < QTreeWidgetItem *, QString >::iterator it = jobs.begin();
            it != jobs.end();
            it++ )
      {
         it->first->setText( 2, "Pushing back to unsubmitted" );
         editor_msg( "black", QString( us_tr( "Pushing back to unsubmitted %1" ) ).arg( it->first->text( 0 ) ) );
         if ( !ufu.move( it->first->text( 0 ), pkg_dir + SLASH ) )
         {
            editor_msg( "red", QString( us_tr( "Error pushing back to unsubmitted %1: %2" ) ).arg( it->first->text( 0 ) ).arg( ufu.errormsg ) );
         }
      }
      editor_msg( "black", "Pushing back done" );
   }

   if ( !update_files() )
   {
      QMessageBox::information( this, 
                                us_tr("US-SOMO: Cluster Status"),
                                us_tr("No further unretrieved submitted jobs found"),
                                0 );
      close();
      return;
   }
   refresh();
}

void US_Hydrodyn_Cluster_Status::complete_retrieve()
{
   editor_msg( "black", "Moving jobs to completed" );
      
   US_File_Util ufu;
   QDir::setCurrent( submitted_dir );
   
   bool allok = true;
   for ( map < QTreeWidgetItem *, QString >::iterator it = jobs.begin();
         it != jobs.end();
         it++ )
   {
      cout << QString( "In complete retrieve: job %1 message %2\n" )
         .arg( it->first->text( 0 ) )
         .arg( it->first->text( 2 ) );

      if ( !it->first->text( 2 ).contains( us_tr( "Error" ) ) )
      {
         it->first->setText( 2, "Moving jobs to completed" );
         editor_msg( "black", QString( us_tr( "Moving jobs to completed %1" ) ).arg( it->first->text( 0 ) ) );
         if ( !ufu.move( it->first->text( 0 ), completed_dir + SLASH ) )
         {
            editor_msg( "red", QString( us_tr( "Error moving job to completed %1: %2" ) ).arg( it->first->text( 0 ) ).arg( ufu.errormsg ) );
         }
      } else {
         allok = false;
      }
   }
   editor_msg( "black", "Moving job to completed done" );

   if ( !allok )
   {
      QMessageBox::information( this, 
                                us_tr("US-SOMO: Cluster Status"),
                                us_tr("Some job's results are missing"),
                                0 );
   }

   if ( !update_files() )
   {
      QMessageBox::information( this, 
                                us_tr("US-SOMO: Cluster Status"),
                                us_tr("No further unretrieved submitted jobs found"),
                                0 );
      close();
      return;
   }

   refresh();
}

void US_Hydrodyn_Cluster_Status::get_next_retrieve()
{
   if ( system_proc_active )
   {
      return;
   }
   // editor_msg( "black", QString( "retrieving %1" ).arg( next_to_process->text( 0 ) ) );
   next_to_process->setText( 2 , "Retrieving results" );
   if ( schedule_retrieve( next_to_process->text( 0 ) ) )
   {
      jobs[ next_to_process ] = "completed";
   } else {
      editor_msg( "red", errormsg );
      jobs[ next_to_process ] = "retrieve failed";
   }
   emit next_status();
}

bool US_Hydrodyn_Cluster_Status::schedule_retrieve( QString file )
{
   errormsg = "";
   if ( !QDir::setCurrent( completed_dir ) )
   {
      errormsg = QString( us_tr( "retrieve: can not change to directory %1" ) ).arg( completed_dir );
      return false;
   }

   cout << QString( "schedule retrieve <%1> <%2>\n" )
      .arg( next_to_process->text( 0 ) )
      .arg(
           job_hostname.count( next_to_process->text( 0 ) ) ?
           job_hostname[ next_to_process->text( 0 ) ] :
           "unknown"
           );

   if ( !job_hostname.count( next_to_process->text( 0 ) ) )
   {
      errormsg = QString( us_tr( "Error: can not determine system host for job %1" ) ).arg( next_to_process->text( 0 ) );
      return false;
   }

   if ( !((US_Hydrodyn_Cluster *) cluster_window )->cluster_stage_to_system.count( job_hostname[ next_to_process->text( 0 ) ] ) )
   {
      errormsg = QString( us_tr( "Error: no configured hosts for job %1 running on %2" ) )
         .arg( next_to_process->text( 0 ) )
         .arg( job_hostname[ next_to_process->text( 0 ) ] );
      return false;
   }

   QString selected_system_name =  ((US_Hydrodyn_Cluster *) cluster_window )->cluster_stage_to_system[ job_hostname[ next_to_process->text( 0 ) ] ];

   cout << 
      QString( us_tr( "hosts for job %1 running on %2 is %3\n" ) )
      .arg( next_to_process->text( 0 ) )
      .arg( job_hostname[ next_to_process->text( 0 ) ] )
      .arg( selected_system_name );

   if ( ((US_Hydrodyn_Cluster *)cluster_window)->cluster_systems.count( selected_system_name ) )
   {
      map < QString, QString > selected_system = ((US_Hydrodyn_Cluster *)cluster_window)->cluster_systems[ selected_system_name ];
      if ( selected_system.count( "ftp" ) )
      {
         // stage_url       = selected_system[ "stage" ];
         // stage_path      = stage_url;
         // stage_url_path  = stage_url;
         // stage_url       .replace( QRegExp( ":.*$" ), "" );
         // stage_path      .replace( QRegExp( "^.*:" ), "" );
         // stage_url_path  += QString( "%1%2%3" ).arg( QDir::separator() ).arg( cluster_id ).arg( QDir::separator() );
         ftp_url         = selected_system[ "ftp" ];
         ftp_url_host    = ftp_url;
         ftp_url_port    = ftp_url;
         ftp_url_host    .replace( QRegExp( ":.*$" ), "" );
         ftp_url_port    .replace( QRegExp( "^.*:" ), "" );
      } else {
         errormsg = QString( us_tr( "The system %1 does not seem to have sufficient configuration information defined" ) ).arg( selected_system_name );
         return false;
      }
   } else {
      errormsg = QString( us_tr( "The system %1 does not seem to have any information" ) ).arg( selected_system_name );
      return false;
   }            

   // move any previously retrieved results
   {
      // are there any?
      QDir::setCurrent( completed_dir );
      QDir qd;
      QStringList previously_retrieved = qd.entryList( QStringList() << QString( "%1_out.t??" )
                                                       .arg( QString( "%1" )
                                                             .arg( next_to_process->text( 0 ) )
                                                             .replace( QRegExp( "\\.(tgz|tar|TGZ|TAR)$" ), "" ) ) );
      if ( previously_retrieved.size() )
      {
         switch ( QMessageBox::question(
                                        this,
                                        us_tr( "US-SOMO: Cluster Status: Retrieve Results" ),
                                        QString(
                                                us_tr( "Results for %1 already exist\n"
                                                    "What to you want to do?" ) ).arg( next_to_process->text( 0 ) ),
                                        us_tr( "&Rename previous results" ),
                                        us_tr( "&Delete previous results" ),
                                        us_tr( "&Stop" ),
                                        0 ) )
         {
         case 0:
            {
               // find unique extension
               unsigned int ext = 1;
               bool any_exist;
               do 
               {
                  any_exist = false;
                  for ( unsigned int i = 0; i < (unsigned int)previously_retrieved.size(); i++ )
                  {
                     QString test_base = previously_retrieved[ i ];
                     QString test_ext  = test_base;
                     test_base.replace( QRegExp( "_out\\.t..$" ), "" );
                     test_ext .replace( QRegExp( "^.*_out\\." ), "_out." );
                     cout << QString( "test base <%1> text num %2 ext <%3>\n" )
                        .arg( test_base )
                        .arg( ext )
                        .arg( test_ext );
                     QString test_file = test_base + QString( "-%1" ).arg( ext ) + test_ext;
                     cout << "test_file: " << test_file << endl;
                     if ( QFile::exists( test_file ) )
                     {
                        any_exist = true;
                        ext++;
                     }
                  }
               } while ( any_exist );
               // now rename them
               for ( unsigned int i = 0; i < (unsigned int)previously_retrieved.size(); i++ )
               {
                  QString test_base = previously_retrieved[ i ];
                  QString test_ext  = test_base;
                  test_base.replace( QRegExp( "_out\\.t..$" ), "" );
                  test_ext .replace( QRegExp( "^.*_out\\." ), "_out." );
                  QString test_file = test_base + QString( "-%1" ).arg( ext ) + test_ext;
                  QDir qd2;
                  if ( !qd2.rename( previously_retrieved[ i ], test_file ) )
                  {
                     errormsg = QString( us_tr( "Error: failed renaming %1 to %2" ) )
                        .arg( previously_retrieved[ i ] )
                        .arg( test_file );
                     return false;
                  }
               }
               editor_msg( "dark red", QString( us_tr( "Notice: previous results files renamed to %1-%2_out" ) )
                           .arg( QFileInfo( next_to_process->text( 0 ) ).baseName() )
                           .arg( ext ) );
            }
            break;
         case 1:
            {
               // delete previous results
               for ( unsigned int i = 0; i < (unsigned int)previously_retrieved.size(); i++ )
               {
                  if ( !QFile::remove( previously_retrieved[ i ] ) )
                  {
                     errormsg = QString( us_tr( "Error: failed to remove %1" ) )
                        .arg( previously_retrieved[ i ] );
                     return false;
                  }
               }
            }
            break;
         case 2:
            errormsg = us_tr( "Stopped by user request" );
            return false;
         }
      }
   }

   // using qftp now:

   QString get_file = file;
   get_file.replace( QRegExp( "\\.(tgz|tar|TGZ|TAR)$" ), "" );
   get_file += QString( "_out.%1" ).arg( get_file.contains( QRegExp( "^(bfnb|bfnbpm|oned)_" ) ) ? "tgz" : "tar" );

   cout << "get file: " << get_file << endl;

   ftp_file = new QFile( get_file );
   if ( !ftp_file->open( QIODevice::WriteOnly ) )
   {
      errormsg = QString( us_tr( "stage: can not open file %1" ) ).arg( file );
      delete ftp_file;
      ftp_file = ( QFile * ) 0;
      return false;
   }

   QString target_dir = QString( "%1" ).arg( next_to_process->text( 0 ) ).replace( QRegExp( "\\.(tar|tgz)$" ), "" );

   cout 
      << QString( "ftp host   : %1\n"
                  "ftp port   : %2\n"
                  "target_dir : %3\n"
                  "file       : %4\n" 
                  "cluser_id  : %5\n" 
                  "cluster_pw : %6\n" 
                  )
      .arg( ftp_url_host )
      .arg( ftp_url_port.toUInt() )
      .arg( target_dir )
      .arg( get_file )
      .arg( cluster_id )
      .arg( cluster_pw );
   
   comm_active = true;

#if QT_VERSION < 0x050000
   ftp.connectToHost( ftp_url_host, ftp_url_port.toUInt() );
   ftp.login        ( cluster_id  , cluster_pw   );
   ftp.cd           ( target_dir );
   ftp.get          ( get_file    , ftp_file       );
   ftp.close        ();
#endif
   
   return true;

   // QStringList cmd;
   // cmd << "scp";
   // cmd << QString( "%1%2/%3" )
   // .arg( stage_url_path )
   // .arg( QString("%1").arg( file ).replace( QRegExp( "\\.(tgz|tar|TGZ|TAR)$" ), "" ) )
   // .arg( QString( "%1_out.t??" )
   // .arg( QString( "%1" ).arg( file ).replace( QRegExp( "\\.(tgz|tar|TGZ|TAR)$" ), "" ) ) );
   //  cmd << QString( "." );

   // return( system_cmd( cmd ) );
}


void US_Hydrodyn_Cluster_Status::get_next_status()
{
   if ( comm_mode == "cancel" )
   {
      next_to_process->setText( 2, "Canceling" );
   }
   if ( comm_mode == "status" )
   {
      // editor_msg( "black", QString( "refreshing status for %1" ).arg( next_to_process->text( 0 ) ) );
      next_to_process->setText( 2, us_tr( "refreshing status" ) );
   }

   if ( send_http_get( next_to_process->text( 0 ) ) )
   {
      jobs[ next_to_process ] = "complete";
   } else {
      editor_msg( "red", errormsg );
      if ( comm_mode == "status" )
      {
         jobs[ next_to_process ] = "get status failed";
      } 
      if ( comm_mode == "cancel" )
      {
         jobs[ next_to_process ] = "cancel job failed";
      } 
   }
   emit next_status();
}

bool US_Hydrodyn_Cluster_Status::send_http_get( QString file )
{
#if QT_VERSION < 0x050000
   // need to do a post & get to submit_url slash stuff
   // its going to require opening a socket etc
   // 
   comm_active = true;
   // editor_msg( "black", "updating status" );

   current_http_response = "";

   connect( &submit_http, SIGNAL( stateChanged ( int ) ), this, SLOT( http_stateChanged ( int ) ) );
   connect( &submit_http, SIGNAL( responseHeaderReceived ( const QHttpResponseHeader & ) ), this, SLOT( http_responseHeaderReceived ( const QHttpResponseHeader & ) ) );
   connect( &submit_http, SIGNAL( readyRead ( const QHttpResponseHeader & ) ), this, SLOT( http_readyRead ( const QHttpResponseHeader & ) ) );
   connect( &submit_http, SIGNAL( dataSendProgress ( int, int ) ), this, SLOT( http_dataSendProgress ( int, int ) ) );
   connect( &submit_http, SIGNAL( dataReadProgress ( int, int ) ), this, SLOT( http_dataReadProgress ( int, int ) ) );
   connect( &submit_http, SIGNAL( requestStarted ( int ) ), this, SLOT( http_requestStarted ( int ) ) );
   connect( &submit_http, SIGNAL( requestFinished ( int, bool ) ), this, SLOT( http_requestFinished ( int, bool ) ) );
   connect( &submit_http, SIGNAL( done ( bool ) ), this, SLOT( http_done ( bool ) ) );

   if ( comm_mode == "status" )
   {
      submit_http.setHost( submit_url_host, submit_url_port.toUInt() );
      submit_http.get( QString( "/ogce-rest/job/jobstatus/%1-%2" )
                       .arg( cluster_id )
                       .arg( file ) );
   }

   if ( comm_mode == "cancel" )
   {
      submit_http.setHost( submit_url_host, submit_url_port.toUInt() );
      submit_http.get( QString( "/ogce-rest/job/canceljob/%1-%2" )
                       .arg( cluster_id )
                       .arg( file ) );
   }

#endif
   return true;
}

void US_Hydrodyn_Cluster_Status::http_stateChanged ( int /* state */ )
{
   // editor_msg( "blue", QString( "http state %1" ).arg( state ) );
}

#if QT_VERSION < 0x050000
void US_Hydrodyn_Cluster_Status::http_responseHeaderReceived ( const QHttpResponseHeader & resp )
{
   cout << resp.reasonPhrase() << endl;
}

void US_Hydrodyn_Cluster_Status::http_readyRead( const QHttpResponseHeader & resp )
{
   cout << "http: readyRead\n" << endl << flush;
   cout << resp.reasonPhrase() << endl;
   // current_http_response = QString( "%1" ).arg( submit_http.readAll() );
   current_http_response = QString( submit_http.readAll() );
   cout << "http response:";
   cout << current_http_response << endl;

   if ( comm_mode == "status" &&
        current_http_response.contains( "<status>" ) )
   {
      QString status = current_http_response;
      status.replace( QRegExp( "^.*<status>" ), "" );
      status.replace( QRegExp( "</status>.*$" ), "" );
      status.replace( QRegExp( "\n|\r" ), " " );
      status.replace( QRegExp( "\\s+" ), " " );
      status.replace( QRegExp( "\\s+$" ), "" );
      status.replace( "SUBMITED", "SUBMITTED" );
      cout << "status:" << status << endl;
      next_to_process->setText( 1, status );
      QString message = current_http_response;
      if ( message.contains( "<message>" ) )
      {
         message.replace( QRegExp( "^.*<message>" ), "" );
         message.replace( QRegExp( "</message>.*$" ), "" );
         message.replace( QRegExp( "\n|\r" ), " " ) ;
         message.replace( QRegExp( "\\s+" ), " " );
         message.replace( QRegExp( "\\s+$" ), "" );
         message.replace( QRegExp( "RSL =.*" ), "" );
         message.replace( QRegExp( "Finished launching job, Host = " ), "" );
         message.replace( QRegExp( "\\s+$" ), "" );
         cout << "message: " << message << endl;
         job_hostname[ next_to_process->text( 0 ) ] = message;
      } else {
         message = "";
      }
      next_to_process->setText( 2, message );
   }
}
#endif

void US_Hydrodyn_Cluster_Status::http_dataSendProgress ( int done, int total )
{
   cout << "http: datasendprogress " << done << " " << total << "\n";
}
void US_Hydrodyn_Cluster_Status::http_dataReadProgress ( int done, int total )
{
   cout << "http: datareadprogress " << done << " " << total << "\n";
}

void US_Hydrodyn_Cluster_Status::http_requestStarted ( int id )
{
   cout << "http: requestStarted " << id << "\n";
}

void US_Hydrodyn_Cluster_Status::http_requestFinished ( int id, bool error  )
{
   cout << "http: requestFinished " << id << " " << error << "\n";
}

void US_Hydrodyn_Cluster_Status::http_done ( bool error )
{
#if QT_VERSION < 0x050000
   disconnect( &submit_http, SIGNAL( stateChanged ( int ) ), 0, 0 );
   disconnect( &submit_http, SIGNAL( responseHeaderReceived ( const QHttpResponseHeader & ) ), 0, 0 );
   disconnect( &submit_http, SIGNAL( readyRead ( const QHttpResponseHeader & ) ), 0, 0 );
   disconnect( &submit_http, SIGNAL( dataSendProgress ( int, int ) ), 0, 0 );
   disconnect( &submit_http, SIGNAL( dataReadProgress ( int, int ) ), 0, 0 );
   disconnect( &submit_http, SIGNAL( requestStarted ( int ) ), 0, 0 );
   disconnect( &submit_http, SIGNAL( requestFinished ( int, bool ) ), 0, 0 );
   disconnect( &submit_http, SIGNAL( done ( bool ) ), 0, 0 );
   comm_active = false;
   QString current_http_error;
   if ( error )
   {
      switch( submit_http.error() )
      {
      case QHttp::NoError :
         current_http_error = us_tr( "No error occurred." );
         break;

      case QHttp::HostNotFound:
         current_http_error = us_tr( "The host name lookup failed." );
         break;

      case QHttp::ConnectionRefused:
         current_http_error = us_tr( "The server refused the connection." );
         break;

      case QHttp::UnexpectedClose:
         current_http_error = us_tr( "The server closed the connection unexpectedly." );
         break;

      case QHttp::InvalidResponseHeader:
         current_http_error = us_tr( "The server sent an invalid response header." );
         break;

      case QHttp::WrongContentLength:
         current_http_error = us_tr( "The client could not read the content correctly because an error with respect to the content length occurred." );
         break;

      case QHttp::Aborted:
         current_http_error = us_tr( "The request was aborted with abort()." );
         break;

      case QHttp::UnknownError:
      default:
         current_http_error = us_tr( "Unknown Error." );
         break;
      }
      QMessageBox::warning( this,
                            us_tr("US-SOMO: Cluster Config"), 
                            us_tr( QString( "There was a error with the management server:\n%1" )
                                .arg( current_http_error ) ),
                            QMessageBox::Ok,
                            QMessageBox::NoButton
                            );
      processing_active = false;
      editor_msg( "red", us_tr( "Error: " ) + current_http_error );
      update_enables();
      return;
   }
   emit next_status();
#endif
}

bool US_Hydrodyn_Cluster_Status::system_cmd( QStringList cmd )
{
   errormsg = "";
   cout << "syscmd: " << cmd.join( ":" ) << endl;

   if ( !cmd.size() )
   {
      errormsg = us_tr( "system_cmd called with no command" );
      return false;
   }

   system_proc = new QProcess( this );

#if QT_VERSION < 0x040000
   system_proc->setArguments( cmd );
#else
   QString prog = cmd.front();
   cmd.pop_front();
   QStringList args = cmd;
#endif

   system_proc_active = true;

   connect( system_proc, SIGNAL(readyReadStandardOutput()), this, SLOT(system_proc_readFromStdout()) );
   connect( system_proc, SIGNAL(readyReadStandardError()), this, SLOT(system_proc_readFromStderr()) );
   connect( system_proc, SIGNAL(finished( int, QProcess::ExitStatus )),   this, SLOT(system_proc_finished( int, QProcess::ExitStatus )) );
   connect( system_proc, SIGNAL(started()),  this, SLOT(system_proc_started()) );


#if QT_VERSION < 0x040000
   return system_proc->start();
#else
   system_proc->start( prog, args );
   return system_proc->waitForStarted();
#endif
}

void US_Hydrodyn_Cluster_Status::system_proc_readFromStdout()
{
#if QT_VERSION < 0x040000
   while ( system_proc->canReadLineStdout() )
   {
      editor_msg("brown", system_proc->readLineStdout());
   }
#else
   editor_msg( "brown", QString( system_proc->readAllStandardOutput() ) );
#endif   
}
   
void US_Hydrodyn_Cluster_Status::system_proc_readFromStderr()
{
#if QT_VERSION < 0x040000
   while ( system_proc->canReadLineStderr() )
   {
      editor_msg("red", system_proc->readLineStderr());
   }
#else
   editor_msg( "red", QString( system_proc->readAllStandardError() ) );
#endif   
}
   
void US_Hydrodyn_Cluster_Status::system_proc_finished( int, QProcess::ExitStatus )
{
   system_proc_readFromStderr();
   system_proc_readFromStdout();

   // disconnect( system_proc, SIGNAL(readyReadStandardOutput()), 0, 0);
   // disconnect( system_proc, SIGNAL(readyReadStandardError()), 0, 0);
   // disconnect( system_proc, SIGNAL(finished( int, QProcess::ExitStatus )), 0, 0);

   // editor->append("System_Proc finished.");
   
   system_proc_active = false;
   delete system_proc;

   // did we get any files ?
   QDir::setCurrent( completed_dir );
   QDir qd;
   QStringList retrieved = qd.entryList( QStringList() << QString( "%1_out.t??" )
                                         .arg( QString( "%1" )
                                               .arg( next_to_process->text( 0 ) )
                                               .replace( QRegExp( "\\.(tgz|tar|TGZ|TAR)$" ), "" ) ) );

   next_to_process->setText( 2,
                             retrieved.size() ? 
                             QString( us_tr( "Retrieved %1 result file%2" ) )
                             .arg( retrieved.size() ) 
                             .arg( retrieved.size() > 1 ? "s" : "" )
                             :
                             us_tr( "Error: no results retrieved" )
                             );
   emit next_status();
}
   
void US_Hydrodyn_Cluster_Status::system_proc_started()
{
   // neditor_msg("brown", "System_Proc launch exited");
   disconnect( system_proc, SIGNAL(started()), 0, 0);
}

void US_Hydrodyn_Cluster_Status::stop()
{
   stopFlag = true;
   pb_stop->setEnabled( false );
   if ( comm_active )
   {
#if QT_VERSION < 0x050000
      submit_http.abort();
      ftp.abort();
#endif
   }
   if ( system_proc_active )
   {
      system_proc->terminate();
      QTimer::singleShot( 2500, system_proc, SLOT( kill() ) );
   }
   update_enables();
}

void US_Hydrodyn_Cluster_Status::ftp_stateChanged ( int state )
{
   cout << "ftp state changed: " << state << endl;
#if QT_VERSION < 0x050000
   if ( state == QFtp::Unconnected )
   {
      cout << "- There is no connection to the host. \n";
   }
   if ( state == QFtp::HostLookup )
   {
      cout << "- A host name lookup is in progress. \n";
   }
   if ( state == QFtp::Connecting )
   {
      cout << "- An attempt to connect to the host is in progress. \n";
   }
   if ( state == QFtp::Connected )
   {
      cout << "- Connection to the host has been achieved. \n";
   }
   if ( state == QFtp::LoggedIn )
   {
      cout << "- Connection and user login have been achieved. \n";
   }
   if ( state == QFtp::Closing )
   {
      cout << "- The connection is closing down, but it is not yet closed.\n";
   }
#endif
}

void US_Hydrodyn_Cluster_Status::ftp_commandStarted ( int id )
{
   cout << "ftp command started: " << id << endl;
}

void US_Hydrodyn_Cluster_Status::ftp_commandFinished ( int id, bool error )
{
#if QT_VERSION < 0x050000
   cout << "ftp command finished" << id 
        << QString( " %1\n" ).arg( error ? QString( "error %1" ).arg( ftp.errorString() ) : "" );
#endif
}

void US_Hydrodyn_Cluster_Status::ftp_done ( bool error )
{
#if QT_VERSION < 0x050000
   cout << "ftp done" 
        << QString( " %1\n" ).arg( error ? QString( "error %1" ).arg( ftp.errorString() ) : "" );
   
   if ( error &&
        jobs[ next_to_process ] == "completed" )
   {
      jobs[ next_to_process ] = "failed";
      editor_msg( "red", QString( "Error: %1" ).arg( ftp.errorString() ) );
   }

   if ( ftp_file )
   {
      ftp_file->close();
      ftp_file = ( QFile * ) 0;
   }

   comm_active = false;
   emit next_status();
#endif
}
