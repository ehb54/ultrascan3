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
   setCaption(tr("US-SOMO: Cluster Results"));
   cluster_window = (void *)p;
   USglobal = new US_Config();

   comm_active = false;

   pkg_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "cluster";
   submitted_dir = pkg_dir + SLASH + "submitted";
   QDir::setCurrent( submitted_dir );

   cluster_id      = ((US_Hydrodyn_Cluster *) cluster_window )->cluster_id;

   submit_url      = ((US_Hydrodyn_Cluster *) cluster_window )->submit_url;
   submit_url_host = submit_url;
   submit_url_port = submit_url;

   submit_url_host.replace( QRegExp( ":.*$" ), "" );
   submit_url_port.replace( QRegExp( "^.*:" ), "" );
   cout << submit_url_host << endl;
   cout << submit_url_port << endl;

   stage_url       = ((US_Hydrodyn_Cluster *) cluster_window )->stage_url;
   stage_path      = stage_url;
   stage_url_path  = stage_url;

   stage_url     .replace( QRegExp( ":.*$" ), "" );
   stage_path    .replace( QRegExp( "^.*:" ), "" );
   stage_url_path += QString( "%1%2%3" ).arg( QDir::separator() ).arg( cluster_id ).arg( QDir::separator() );

   cout << stage_url << endl;
   cout << stage_path << endl;
   cout << stage_url_path << endl;

   if ( !update_files( false ) )
   {
      QMessageBox::information( this, 
                                tr("US-SOMO: Cluster Status"),
                                tr("No unretrieved submitted jobs found"),
                                0 );
   }

   setupGUI();

   update_files();

   update_enables();
   refresh();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 700, 600 );

   editor_msg("blue", "THIS WINDOW IS UNDER DEVELOPMENT." );
}

US_Hydrodyn_Cluster_Status::~US_Hydrodyn_Cluster_Status()
{
}

unsigned int US_Hydrodyn_Cluster_Status::update_files( bool set_lv_files )
{
   files.clear();

   // traverse directory and build up files
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
         cout << "files: " << files[ i ] << endl;
         new QListViewItem( lv_files, files[ i ], "unknown", "", QFileInfo( files[ i ] ).created().toString() );
      }
   }

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
   qApp->processEvents();
}
   
void US_Hydrodyn_Cluster_Status::update_enables()
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

   pb_remove  ->setEnabled( !comm_active && any_selected  );
   pb_retrieve->setEnabled( !comm_active && any_completed );
}

void US_Hydrodyn_Cluster_Status::refresh()
{
   get_status();
   update_enables();
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
}

void US_Hydrodyn_Cluster_Status::cancel_selected()
{
   update_enables();
   cout << "setup jobs for cancel selected\n";
   comm_mode = "cancel";
   jobs.clear();
   
   QStringList qsl_submit;
   QListViewItem *lvi = lv_files->firstChild();
   if ( lvi )
   {
      do {
         if ( lvi->isSelected() )
         {
            cout << "cancel job: " << lvi->text( 0 ) << endl;
            jobs[ lvi ] = "cancel job";
         }
      } while ( ( lvi = lvi->nextSibling() ) );
      emit next_status();
   }
}

void US_Hydrodyn_Cluster_Status::get_status()
{
   update_enables();
   cout << "setup jobs for get status\n";
   comm_mode = "status";
   jobs.clear();
   
   QStringList qsl_submit;
   QListViewItem *lvi = lv_files->firstChild();
   if ( lvi )
   {
      do {
         cout << "get status: " << lvi->text( 0 ) << endl;
         jobs[ lvi ] = "get status";
      } while ( ( lvi = lvi->nextSibling() ) );
      
      emit next_status();
   }
}

void US_Hydrodyn_Cluster_Status::next_status()
{
   cout << "next status\n";
   if ( comm_active )
   {
      cout << "comm active\n";
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
   }

   if ( ok )
   {
      if ( comm_mode == "status" )
      {
         editor_msg( "black", tr( "done refreshing job status" ) );
      } 
      if ( comm_mode == "cancel" )
      {
         editor_msg( "black", tr( "done canceling jobs" ) );
         complete_remove();
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
   }

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
   update_enables();
   refresh();
}

void US_Hydrodyn_Cluster_Status::get_next_status()
{
   cout << "get next status\n";
   if ( comm_mode == "cancel" )
   {
      next_to_process->setText( 2, "Canceling" );
   }
   if ( comm_mode == "status" )
   {
      editor_msg( "black", QString( "refreshing status for %1" ).arg( next_to_process->text( 0 ) ) );
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
   cout << "http_get\n";
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

void US_Hydrodyn_Cluster_Status::http_stateChanged ( int /* estate */ )
{
   // editor_msg( "blue", QString( "http state %1" ).arg( state ) );
}

void US_Hydrodyn_Cluster_Status::http_responseHeaderReceived ( const QHttpResponseHeader & resp )
{
   cout << resp.reasonPhrase() << endl;
}

void US_Hydrodyn_Cluster_Status::http_readyRead( const QHttpResponseHeader & resp )
{
   cout << "http: readyRead\n";
   cout << resp.reasonPhrase() << endl;
   current_http_response = QString( "%1" ).arg( submit_http.readAll() );
   cout << current_http_response << endl;

   if ( comm_mode == "status" )
   {
      QString status = current_http_response;
      status.replace( QRegExp( "^.*<status>" ), "" );
      status.replace( QRegExp( "</status>.*$" ), "" );
      next_to_process->setText( 1, status );
      QString message = current_http_response;
      if ( message.contains( "<message>" ) )
      {
         message.replace( QRegExp( "^.*<message>" ), "" );
         message.replace( QRegExp( "</message>.*$" ), "" );
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

void US_Hydrodyn_Cluster_Status::http_requestFinished ( int id, bool error )
{
   cout << "http: requestFinished " << id << " " << error << "\n";
}

void US_Hydrodyn_Cluster_Status::http_done ( bool error )
{
   cout << "http: done " << error << "\n";
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
