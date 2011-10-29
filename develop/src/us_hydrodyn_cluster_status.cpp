#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_cluster.h"
#include "../include/us_hydrodyn_cluster_status.h"

#define SLASH QDir::separator()

US_Hydrodyn_Cluster_Status::US_Hydrodyn_Cluster_Status(
                                         void *us_hydrodyn, 
                                         QWidget *p, 
                                         const char *name
                                         ) : QDialog(p, name)
{
   this->us_hydrodyn = us_hydrodyn;
   setCaption(tr("US-SOMO: Cluster Status"));
   cluster_window = (void *)p;
   USglobal = new US_Config();

   stopFlag = false;
   comm_active = false;
   system_proc_active = false;
   processing_active = false;
   disable_updates = false;

   pkg_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "cluster";
   submitted_dir = pkg_dir + SLASH + "submitted";
   completed_dir = pkg_dir  + SLASH + "completed";
   QDir::setCurrent( submitted_dir );

   cluster_id      = ((US_Hydrodyn_Cluster *) cluster_window )->cluster_id;

   submit_url      = ((US_Hydrodyn_Cluster *) cluster_window )->submit_url;
   submit_url_host = submit_url;
   submit_url_port = submit_url;

   submit_url_host.replace( QRegExp( ":.*$" ), "" );
   submit_url_port.replace( QRegExp( "^.*:" ), "" );

   stage_url       = ((US_Hydrodyn_Cluster *) cluster_window )->stage_url;
   stage_path      = stage_url;
   stage_url_path  = stage_url;

   stage_url     .replace( QRegExp( ":.*$" ), "" );
   stage_path    .replace( QRegExp( "^.*:" ), "" );
   stage_url_path += QString( "%1%2%3" ).arg( QDir::separator() ).arg( cluster_id ).arg( QDir::separator() );

   if ( !update_files( false ) )
   {
      QMessageBox::information( this, 
                                tr("US-SOMO: Cluster Status"),
                                tr("No unretrieved submitted jobs found"),
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
   files.clear();

   // traverse directory and build up files
   QDir::setCurrent( submitted_dir );
   QDir qd;
   QStringList tgz_files = qd.entryList( "*.tgz" );
   QStringList tar_files = qd.entryList( "*.tar" );
   QStringList all_files = QStringList::split( "\n", 
                                               tgz_files.join("\n") + 
                                               ( tgz_files.size() ? "\n" : "" ) +
                                               tar_files.join("\n") );

   for ( unsigned int i = 0; i < all_files.size(); i++ )
   {
      if ( !all_files[ i ].contains( "_out", false ) )
      {
         files << all_files[ i ];
      }
   }
   
   if ( set_lv_files )
   {
      lv_files->clear();
      for ( unsigned int i = 0; i < files.size(); i++ )
      {
         new QListViewItem( lv_files, files[ i ], "unknown", "", QFileInfo( files[ i ] ).created().toString() );
      }
   }

   disable_updates = false;
   return files.size();
}

void US_Hydrodyn_Cluster_Status::setupGUI()
{
   int minHeight1 = 30;

   lbl_title = new QLabel( tr( " Cluster job status" ), this);
   lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lv_files = new QListView(this);
   lv_files->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lv_files->setMinimumHeight(minHeight1 * 3);
   lv_files->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lv_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   lv_files->setEnabled(true);
   lv_files->setSelectionMode( QListView::Multi );

   lv_files->addColumn( tr( "Name" ) );
   lv_files->addColumn( tr( "Status" ) );
   lv_files->addColumn( tr( "Additional Info" ) );
   lv_files->addColumn( tr( "Date created" ) );
   connect( lv_files, SIGNAL( selectionChanged() ), SLOT( update_enables() ) );

   pb_refresh = new QPushButton(tr("Refresh Status"), this);
   pb_refresh->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_refresh->setMinimumHeight(minHeight1);
   pb_refresh->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect( pb_refresh, SIGNAL( clicked() ), SLOT( refresh() ) );

   pb_remove = new QPushButton(tr("Cancel jobs"), this);
   pb_remove->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_remove->setMinimumHeight(minHeight1);
   pb_remove->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect( pb_remove, SIGNAL( clicked() ), SLOT( remove() ) );

   pb_retrieve = new QPushButton( tr("Retrieve results" ), this);
   pb_retrieve->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_retrieve->setMinimumHeight(minHeight1);
   pb_retrieve->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect( pb_retrieve, SIGNAL( clicked() ), SLOT( retrieve() ) );
   
   editor = new QTextEdit(this);
   editor->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   editor->setReadOnly(true);

   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight1);

   m = new QMenuBar(frame, "menu" );
   m->setMinimumHeight(minHeight1 - 5);
   m->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   QPopupMenu * file = new QPopupMenu(editor);
   m->insertItem( tr("&File"), file );
   file->insertItem( tr("Font"),  this, SLOT(update_font()),    ALT+Key_F );
   file->insertItem( tr("Save"),  this, SLOT(save()),    ALT+Key_S );
   file->insertItem( tr("Clear Display"), this, SLOT(clear_display()),   ALT+Key_X );
   editor->setWordWrap (QTextEdit::WidgetWidth);
   editor->setMinimumHeight(100);

   pb_stop = new QPushButton(tr("Stop"), this);
   pb_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_stop->setMinimumHeight(minHeight1);
   pb_stop->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_stop, SIGNAL(clicked()), SLOT(stop()));
   pb_stop->setEnabled( false );

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // build layout

   QHBoxLayout *hbl_buttons1 = new QHBoxLayout( 0 );
   hbl_buttons1->addSpacing( 4 );
   hbl_buttons1->addWidget ( pb_refresh );
   hbl_buttons1->addSpacing( 4 );
   hbl_buttons1->addWidget ( pb_remove );
   hbl_buttons1->addSpacing( 4 );
   hbl_buttons1->addWidget ( pb_retrieve );
   hbl_buttons1->addSpacing( 4 );

   QHBoxLayout *hbl_bottom = new QHBoxLayout( 0 );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_stop );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_cancel );
   hbl_bottom->addSpacing( 4 );

   QBoxLayout *vbl_editor_group = new QVBoxLayout(0);
   vbl_editor_group->addWidget(frame);
   vbl_editor_group->addWidget(editor);

   QVBoxLayout *background = new QVBoxLayout( this );
   background->addSpacing( 4);
   background->addWidget ( lbl_title );
   background->addSpacing( 4 );
   background->addWidget ( lv_files );
   background->addSpacing( 4 );
   background->addLayout ( hbl_buttons1 );
   background->addSpacing( 4 );
   background->addLayout ( vbl_editor_group );
   background->addSpacing( 4 );
   background->addLayout ( hbl_bottom );
   background->addSpacing( 4);
}

void US_Hydrodyn_Cluster_Status::cancel()
{
   close();
}

void US_Hydrodyn_Cluster_Status::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_cluster_status.html");
}

void US_Hydrodyn_Cluster_Status::closeEvent(QCloseEvent *e)
{
   if ( comm_active )
   {
      submit_http.abort();
   }
   if ( system_proc_active )
   {
      system_proc->tryTerminate();
      QTimer::singleShot( 2500, system_proc, SLOT( kill() ) );
   }
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}


void US_Hydrodyn_Cluster_Status::clear_display()
{
   editor->clear();
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
   fn = QFileDialog::getSaveFileName(QString::null, QString::null, this );
   if(!fn.isEmpty() )
   {
      QString text = editor->text();
      QFile f( fn );
      if ( !f.open( IO_WriteOnly | IO_Translate) )
      {
         return;
      }
      QTextStream t( &f );
      t << text;
      f.close();
      editor->setModified( false );
      setCaption( fn );
   }
}

void US_Hydrodyn_Cluster_Status::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->color();
   editor->setColor(color);
   editor->append(msg);
   editor->setColor(save_color);
   editor->scrollToBottom();
   // qApp->processEvents();
}
   
void US_Hydrodyn_Cluster_Status::update_enables()
{
   if ( !disable_updates )
   {
      bool any_selected = false;
      bool any_completed = false;
      QListViewItem *lvi = lv_files->firstChild();
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
            }
            if ( any_selected && any_completed )
            {
               break;
            }
         } while ( ( lvi = lvi->nextSibling() ) );
      }
      pb_refresh  ->setEnabled( !processing_active && !comm_active && !system_proc_active );
      pb_remove  ->setEnabled( !processing_active && !comm_active && !system_proc_active && any_selected  );
      pb_retrieve->setEnabled( !processing_active && !comm_active && !system_proc_active && any_completed );
   } 
   if ( processing_active )
   {
      pb_refresh ->setEnabled( false );
      pb_remove  ->setEnabled( false );
      pb_retrieve->setEnabled( false );
   }
}

void US_Hydrodyn_Cluster_Status::refresh()
{
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
   jobs.clear();
   
   QStringList qsl_submit;
   QListViewItem *lvi = lv_files->firstChild();
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
      editor_msg( "black", tr( "retrieving results" ) );
      update_enables();
      emit next_status();
   } else {
      stopFlag = false;
      pb_stop->setEnabled( false );
   }
}

void US_Hydrodyn_Cluster_Status::cancel_selected()
{
   update_enables();
   comm_mode = "cancel";
   jobs.clear();
   
   QStringList qsl_submit;
   QListViewItem *lvi = lv_files->firstChild();
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
}

void US_Hydrodyn_Cluster_Status::get_status()
{
   // update_enables();
   comm_mode = "status";
   jobs.clear();
   
   QStringList qsl_submit;
   QListViewItem *lvi = lv_files->firstChild();
   if ( lvi )
   {
      do {
         jobs[ lvi ] = "get status";
      } while ( ( lvi = lvi->nextSibling() ) );
      
      emit next_status();
   }
}

void US_Hydrodyn_Cluster_Status::next_status()
{
   if ( stopFlag )
   {
      editor_msg( "red", tr( "stopped by user request" ) );
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
   for ( map < QListViewItem *, QString >::iterator it = jobs.begin();
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
         // editor_msg( "black", tr( "done refreshing job status" ) );
      } 
      if ( comm_mode == "cancel" )
      {
         editor_msg( "black", tr( "done canceling jobs" ) );
         complete_remove();
      } 
      if ( comm_mode == "retrieve" )
      {
         editor_msg( "black", tr( "done retrieving jobs" ) );
         complete_retrieve();
      } 
   } else {
      if ( comm_mode == "status" )
      {
         editor_msg( "red", tr( "job status refresh had errors" ) );
      } 
      if ( comm_mode == "cancel" )
      {
         editor_msg( "red", tr( "cancel job had errors" ) );
      } 
      if ( comm_mode == "retrieve" )
      {
         editor_msg( "red", tr( "retrieve results had errors" ) );
      } 
   }

   processing_active = false;
   update_enables();
}

void US_Hydrodyn_Cluster_Status::complete_remove()
{
   if ( QMessageBox::question(
                              this,
                              tr( "US-SOMO: Cluster Status" ),
                              tr( "What do you want to do with the canceled jobs?" ),
                              tr( "&Push back to unsubmitted" ),
                              tr( "&Remove completely" ),
                              QString::null, 0, 1 ) )
   {
      editor_msg( "black", "completely removing" );
      for ( map < QListViewItem *, QString >::iterator it = jobs.begin();
            it != jobs.end();
            it++ )
      {
         it->first->setText( 2, "Removing" );
         editor_msg( "black", QString( tr( "Removing %1" ) ).arg( it->first->text( 0 ) ) );
         if ( !QFile::remove( it->first->text( 0 ) ) )
         {
            editor_msg( "red", QString( tr( "Error removing %1" ) ).arg( it->first->text( 0 ) ) );
         }
      }
      editor_msg( "black", "Removing done" );
   } else {
      
      editor_msg( "black", "Push back to unsubmitted" );
      
      US_File_Util ufu;
      
      for ( map < QListViewItem *, QString >::iterator it = jobs.begin();
            it != jobs.end();
            it++ )
      {
         it->first->setText( 2, "Pushing back to unsubmitted" );
         editor_msg( "black", QString( tr( "Pushing back to unsubmitted %1" ) ).arg( it->first->text( 0 ) ) );
         if ( !ufu.move( it->first->text( 0 ), pkg_dir + SLASH ) )
         {
            editor_msg( "red", QString( tr( "Error pushing back to unsubmitted %1: %2" ) ).arg( it->first->text( 0 ) ).arg( ufu.errormsg ) );
         }
      }
      editor_msg( "black", "Pushing back done" );
   }

   if ( !update_files() )
   {
      QMessageBox::information( this, 
                                tr("US-SOMO: Cluster Status"),
                                tr("No further unretrieved submitted jobs found"),
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
   for ( map < QListViewItem *, QString >::iterator it = jobs.begin();
         it != jobs.end();
         it++ )
   {
      if ( !it->first->text( 2 ).contains( tr( "Error" ) ) )
      {
         it->first->setText( 2, "Moving jobs to completed" );
         editor_msg( "black", QString( tr( "Moving jobs to completed %1" ) ).arg( it->first->text( 0 ) ) );
         if ( !ufu.move( it->first->text( 0 ), completed_dir + SLASH ) )
         {
            editor_msg( "red", QString( tr( "Error moving job to completed %1: %2" ) ).arg( it->first->text( 0 ) ).arg( ufu.errormsg ) );
         }
      } else {
         allok = false;
      }
   }
   editor_msg( "black", "Moving job to completed done" );

   if ( !allok )
   {
      QMessageBox::information( this, 
                                tr("US-SOMO: Cluster Status"),
                                tr("Some job's results are missing"),
                                0 );
   }

   if ( !update_files() )
   {
      QMessageBox::information( this, 
                                tr("US-SOMO: Cluster Status"),
                                tr("No further unretrieved submitted jobs found"),
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
      errormsg = QString( tr( "retrieve: can not change to directory %1" ) ).arg( completed_dir );
      return false;
   }

   QStringList cmd;
   cmd << "scp";
   cmd << QString( "%1%2/%3" )
      .arg( stage_url_path )
      .arg( QString("%1").arg( file ).replace( QRegExp( "\\.(tgz|tar|TGZ|TAR)$" ), "" ) )
      .arg( QString( "%1_out.t??" )
            .arg( QString( "%1" ).arg( file ).replace( QRegExp( "\\.(tgz|tar|TGZ|TAR)$" ), "" ) ) );
   cmd << QString( "." );

   return( system_cmd( cmd ) );
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
      next_to_process->setText( 2, tr( "refreshing status" ) );
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

   return true;
}

void US_Hydrodyn_Cluster_Status::http_stateChanged ( int /* state */ )
{
   // editor_msg( "blue", QString( "http state %1" ).arg( state ) );
}

void US_Hydrodyn_Cluster_Status::http_responseHeaderReceived ( const QHttpResponseHeader & resp )
{
   cout << resp.reasonPhrase() << endl;
}

void US_Hydrodyn_Cluster_Status::http_readyRead( const QHttpResponseHeader & resp )
{
   cout << "http: readyRead\n" << endl << flush;
   cout << resp.reasonPhrase() << endl;
   current_http_response = QString( "%1" ).arg( submit_http.readAll() );
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
         cout << "message: " << message << endl;
      } else {
         message = "";
      }
      next_to_process->setText( 2, message );
   }
}

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

void US_Hydrodyn_Cluster_Status::http_done ( bool /* error */ )
{
   disconnect( &submit_http, SIGNAL( stateChanged ( int ) ), 0, 0 );
   disconnect( &submit_http, SIGNAL( responseHeaderReceived ( const QHttpResponseHeader & ) ), 0, 0 );
   disconnect( &submit_http, SIGNAL( readyRead ( const QHttpResponseHeader & ) ), 0, 0 );
   disconnect( &submit_http, SIGNAL( dataSendProgress ( int, int ) ), 0, 0 );
   disconnect( &submit_http, SIGNAL( dataReadProgress ( int, int ) ), 0, 0 );
   disconnect( &submit_http, SIGNAL( requestStarted ( int ) ), 0, 0 );
   disconnect( &submit_http, SIGNAL( requestFinished ( int, bool ) ), 0, 0 );
   disconnect( &submit_http, SIGNAL( done ( bool ) ), 0, 0 );
   comm_active = false;
   emit next_status();
}

bool US_Hydrodyn_Cluster_Status::system_cmd( QStringList cmd )
{
   errormsg = "";
   cout << "syscmd: " << cmd.join( ":" ) << endl;

   if ( !cmd.size() )
   {
      errormsg = tr( "system_cmd called with no command" );
      return false;
   }

   system_proc = new QProcess( this );

   system_proc->setArguments( cmd );

   system_proc_active = true;

   connect( system_proc, SIGNAL(readyReadStdout()), this, SLOT(system_proc_readFromStdout()) );
   connect( system_proc, SIGNAL(readyReadStderr()), this, SLOT(system_proc_readFromStderr()) );
   connect( system_proc, SIGNAL(processExited()),   this, SLOT(system_proc_processExited()) );
   connect( system_proc, SIGNAL(launchFinished()),  this, SLOT(system_proc_launchFinished()) );

   return system_proc->start();
}

void US_Hydrodyn_Cluster_Status::system_proc_readFromStdout()
{
   while ( system_proc->canReadLineStdout() )
   {
      editor_msg("brown", system_proc->readLineStdout());
   }
}
   
void US_Hydrodyn_Cluster_Status::system_proc_readFromStderr()
{
   while ( system_proc->canReadLineStderr() )
   {
      editor_msg("red", system_proc->readLineStderr());
   }
}
   
void US_Hydrodyn_Cluster_Status::system_proc_processExited()
{
   system_proc_readFromStderr();
   system_proc_readFromStdout();

   // disconnect( system_proc, SIGNAL(readyReadStdout()), 0, 0);
   // disconnect( system_proc, SIGNAL(readyReadStderr()), 0, 0);
   // disconnect( system_proc, SIGNAL(processExited()), 0, 0);

   // editor->append("System_Proc finished.");
   
   system_proc_active = false;
   delete system_proc;

   // did we get any files ?
   QDir::setCurrent( completed_dir );
   QDir qd;
   QStringList retrieved = qd.entryList( QString( "%1_out.t??" )
                                         .arg( QString( "%1" )
                                               .arg( next_to_process->text( 0 ) )
                                               .replace( QRegExp( "\\.(tgz|tar|TGZ|TAR)$" ), "" ) ) );

   next_to_process->setText( 2, 
                             retrieved.size() ? 
                             QString( tr( "Retrieved %1 result file%2" ) )
                             .arg( retrieved.size() ) 
                             .arg( retrieved.size() > 1 ? "s" : "" )
                             :
                             tr( "Error: no results retrieved" )
                             );
   emit next_status();
}
   
void US_Hydrodyn_Cluster_Status::system_proc_launchFinished()
{
   // neditor_msg("brown", "System_Proc launch exited");
   disconnect( system_proc, SIGNAL(launchFinished()), 0, 0);
}

void US_Hydrodyn_Cluster_Status::stop()
{
   stopFlag = true;
   pb_stop->setEnabled( false );
   if ( comm_active )
   {
      submit_http.abort();
   }
   if ( system_proc_active )
   {
      system_proc->tryTerminate();
      QTimer::singleShot( 2500, system_proc, SLOT( kill() ) );
   }
   update_enables();
}
