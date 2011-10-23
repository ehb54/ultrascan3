#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_cluster.h"
#include "../include/us_hydrodyn_cluster_submit.h"

#define SLASH QDir::separator()

US_Hydrodyn_Cluster_Submit::US_Hydrodyn_Cluster_Submit(
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
   submit_active = false;

   pkg_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "cluster";
   QDir::setCurrent( pkg_dir );

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
                                tr("US-SOMO: Cluster Jobs"),
                                tr("No unsubmitted jobs found"),
                                0 );
   }

   setupGUI();

   submitted_dir = pkg_dir + SLASH + "submitted";
   QDir dir1( submitted_dir );
   if ( !dir1.exists() )
   {
      editor_msg( "black", QString( tr( "Created directory %1" ) ).arg( submitted_dir ) );
      dir1.mkdir( submitted_dir );
   }

   tmp_dir = pkg_dir + SLASH + "tmp";
   QDir dir2( tmp_dir );
   if ( !dir2.exists() )
   {
      editor_msg( "black", QString( tr( "Created directory %1" ) ).arg( tmp_dir ) );
      dir2.mkdir( tmp_dir );
   }

   update_enables();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 700, 600 );

   editor_msg("blue", "THIS WINDOW IS UNDER DEVELOPMENT. SUBMISSION CURRENTLY DOES NOT WORK!" );
}

US_Hydrodyn_Cluster_Submit::~US_Hydrodyn_Cluster_Submit()
{
}

unsigned int US_Hydrodyn_Cluster_Submit::update_files( bool set_lb_files )
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
   
   if ( set_lb_files )
   {
      lb_files->clear();
      lb_files->insertStringList( files );
      lb_files->setCurrentItem(0);
      lb_files->setSelected(0, false);
   }

   return files.size();
}

void US_Hydrodyn_Cluster_Submit::setupGUI()
{
   int minHeight1 = 30;

   lbl_title = new QLabel( tr( "Manage results from cluster jobs" ), this);
   lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_files = new QLabel(tr("Available jobs:"), this);
   lbl_files->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_files->setMinimumHeight(minHeight1);
   lbl_files->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   lb_files = new QListBox(this);
   lb_files->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lb_files->setMinimumHeight(minHeight1 * 3);
   lb_files->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lb_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   lb_files->setEnabled(true);

   lb_files->insertStringList( files );
   lb_files->setCurrentItem(0);
   lb_files->setSelected(0, false);
   lb_files->setSelectionMode(QListBox::Multi);

   connect(lb_files, SIGNAL( selectionChanged()), SLOT( update_enables() ));

   pb_select_all = new QPushButton(tr("Select all"), this);
   pb_select_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_select_all->setMinimumHeight(minHeight1);
   pb_select_all->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_select_all, SIGNAL(clicked()), SLOT(select_all()));

   pb_submit = new QPushButton(tr("Submit"), this);
   pb_submit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_submit->setMinimumHeight(minHeight1);
   pb_submit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_submit, SIGNAL(clicked()), SLOT(submit()));
   
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

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // build layout

   QHBoxLayout *hbl_buttons1 = new QHBoxLayout( 0 );
   hbl_buttons1->addSpacing( 4 );
   hbl_buttons1->addWidget ( pb_select_all);
   hbl_buttons1->addSpacing( 4 );
   hbl_buttons1->addWidget ( pb_submit );
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
   background->addWidget ( lbl_files );
   background->addSpacing( 4 );
   background->addWidget ( lb_files );
   background->addSpacing( 4 );
   background->addLayout ( hbl_buttons1 );
   background->addSpacing( 4 );
   background->addLayout ( vbl_editor_group );
   background->addSpacing( 4 );
   background->addLayout ( hbl_bottom );
   background->addSpacing( 4);
}

void US_Hydrodyn_Cluster_Submit::cancel()
{
   close();
}

void US_Hydrodyn_Cluster_Submit::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_cluster_submit.html");
}

void US_Hydrodyn_Cluster_Submit::closeEvent(QCloseEvent *e)
{
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}


void US_Hydrodyn_Cluster_Submit::clear_display()
{
   editor->clear();
   editor->append("\n\n");
}

void US_Hydrodyn_Cluster_Submit::update_font()
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

void US_Hydrodyn_Cluster_Submit::save()
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

void US_Hydrodyn_Cluster_Submit::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->color();
   editor->setColor(color);
   editor->append(msg);
   editor->setColor(save_color);
   editor->scrollToBottom();
   qApp->processEvents();
}
   
void US_Hydrodyn_Cluster_Submit::update_enables()
{
   bool running = comm_active || submit_active;
   pb_stop      ->setEnabled( running );
   pb_select_all->setEnabled( !running );
   pb_submit    ->setEnabled( !running );
   pb_help      ->setEnabled( !running );
   pb_cancel    ->setEnabled( !running );
   lb_files     ->setEnabled( !running );

   if ( !running && !disable_updates )
   {
      bool any_selected = false;
      for ( int i = 0; i < lb_files->numRows(); i++ )
      {
         if ( lb_files->isSelected(i) )
         {
            any_selected = true;
            break;
         }
      }
      pb_submit->setEnabled( any_selected );
   }
}

void US_Hydrodyn_Cluster_Submit::select_all()
{
   bool any_not_selected = false;
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( !lb_files->isSelected(i) )
      {
         any_not_selected = true;
      }
   }

   disable_updates = true;
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      lb_files->setSelected(i, any_not_selected);
   }
   disable_updates = false;
   update_files();
}

void US_Hydrodyn_Cluster_Submit::submit()
{
   submit_active = true;
   update_enables();

   QStringList qsl_submit;
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         qsl_submit << lb_files->item( i )->text();
      }
   }
   
   if ( !stage( files ) )
   {
      submit_active = false;
      update_enables();
      return;
   }

   if ( !submit_jobs( files ) )
   {
      submit_active = false;
      update_enables();
      editor_msg( "red", errormsg );
      return;
   }

   US_File_Util ufu;
   if ( !ufu.move( qsl_submit, submitted_dir, true ) )
   {
      editor_msg( "red", ufu.errormsg );
   }

   QMessageBox::information( this, 
                             tr("US-SOMO: Cluster Results"),
                             QString( tr("%1 jobs submitted for processing" ) ).arg( files.size() ),
                             0 );


   if ( !update_files() )
   {
      QMessageBox::information( this, 
                                tr("US-SOMO: Cluster Results"),
                                tr("No further unsubmitted jobs available"),
                                0 );
      close();
   }
   submit_active = false;
   update_enables();
}

bool US_Hydrodyn_Cluster_Submit::stage( QStringList files )
{
   for ( unsigned int i = 0; i < files.size(); i++ )
   {
      if ( !stage( files[ i ] ) )
      {
         return false;
      }
   }
   return true;
}

bool US_Hydrodyn_Cluster_Submit::stage( QString file )
{
   errormsg = "";
   if ( !QFile::exists( file ) )
   {
      errormsg = QString( tr( "stage: can not find file %1" ) ).arg( file );
      return false;
   }

   // right now we are using scp, need to come up with a better method

   // make sure dest dir is made:
   if ( !QDir::setCurrent( tmp_dir ) )
   {
      errormsg = QString( tr( "stage: can not change to directory %1" ) ).arg( tmp_dir );
      return false;
   }

   QString cmd;

   cmd = QString( "ssh %1 mkdir -p %2/%3" )
      .arg( stage_url )
      .arg( stage_path )
      .arg( cluster_id );

   editor_msg( "dark gray", QString( tr( "initializing communication with %1" ) ).arg( stage_url ) );
   if ( !run_in_tmp( cmd ) )
   {
      editor_msg( "red", errormsg );
      return false;
   }
   editor_msg( "dark gray", QString( tr( "communication initialized with %1" ) ).arg( stage_url ) );

   cmd = QString( "scp %1%2%3 %4" )
      .arg( pkg_dir )
      .arg( QDir::separator() )
      .arg( file )
      .arg( stage_url_path );

   editor_msg( "dark gray", QString( tr( "transferring %1" ) ).arg( file ) );
   if ( !run_in_tmp( cmd ) )
   {
      editor_msg( "red", errormsg );
      return false;
   }
   editor_msg( "dark gray", QString( tr( "transfered %1" ) ).arg( file ) );
   if ( !submit_active )
   {
      editor_msg( "red", tr( "Stopped by user request" ) );
      return false;
   }
   return true;
}

bool US_Hydrodyn_Cluster_Submit::run_in_tmp( QString cmd )
{
   errormsg = "";
   if ( !QDir::setCurrent( tmp_dir ) )
   {
      errormsg = QString( tr( "stage: can not change do directory %1" ) ).arg( tmp_dir );
      return false;
   }

   QFile::remove( "log1" );
   QFile::remove( "log2" );

   // assuming bash, probably should check shell

   cmd += " >log1 2>log2";

   cout << cmd << endl;
   system( cmd );
   
   last_stdout.clear();
   last_stderr.clear();

   QFile f1( "log1" );
   QFile f2( "log2" );

   if ( !f1.exists() )
   {
      errormsg = QString( tr( "stage: no output file found for command %1" ) ).arg( cmd );
      if ( !QDir::setCurrent( pkg_dir ) )
      {
         errormsg += QString( tr( "\nstage: can not change do directory %1" ) ).arg( pkg_dir );
      }
      return false;
   }

   if ( !f2.exists() )
   {
      errormsg = QString( tr( "stage: no error output file found for command %1" ) ).arg( cmd );
      if ( !QDir::setCurrent( pkg_dir ) )
      {
         errormsg += QString( tr( "\nstage: can not change do directory %1" ) ).arg( pkg_dir );
      }
      return false;
   }

   if ( !f1.open( IO_ReadOnly ) )
   {
      errormsg = QString( tr( "stage: could not open output file for command %1" ) ).arg( cmd );
      if ( !QDir::setCurrent( pkg_dir ) )
      {
         errormsg += QString( tr( "\nstage: can not change do directory %1" ) ).arg( pkg_dir );
      }
      return false;
   }

   QTextStream ts1( &f1 );
   while( !ts1.atEnd() )
   {
      last_stdout << ts1.readLine();
   };
   f1.close();

   if ( !f2.open( IO_ReadOnly ) )
   {
      errormsg = QString( tr( "stage: could not open error output file for command %1" ) ).arg( cmd );
      if ( !QDir::setCurrent( pkg_dir ) )
      {
         errormsg += QString( tr( "\nstage: can not change do directory %1" ) ).arg( pkg_dir );
      }
      return false;
   }

   QTextStream ts2( &f2 );
   while( !ts2.atEnd() )
   {
      last_stderr << ts2.readLine();
   };
   f2.close();

   // after debugging is done, we should remove the droppings
   // f1.remove();
   // f2.remove();

   if ( !QDir::setCurrent( pkg_dir ) )
   {
      errormsg = QString( tr( "stage: can not change do directory %1" ) ).arg( pkg_dir );
      return false;
   }

   return true;
}

bool US_Hydrodyn_Cluster_Submit::send_xml( QString xml )
{
   // need to do a post & get to submit_url slash stuff
   // its going to require opening a socket etc
   // 
   comm_active = true;
   cout << "send_xml\n";

   update_enables(); 

   QString header = QString( "POST /ogce-rest/job/runjob/async HTTP/1.0\n"
                             "Content-Type: application/xml\n"
                             "Connection: Keep-Alive\n"
                             "Content-Length: %1\n"
                             "\n" ).arg( xml.length() );

   current_xml = header + xml;
   current_xml_response = "";

   cout << "connect_to_host\n";
   submit_socket.connectToHost( submit_url_host, submit_url_port.toUInt() );
   cout << "after connect_to_host\n";

   connect( &submit_socket, SIGNAL( error( int ) ), this, SLOT( socket_error( int ) ) );
   cout << "after connect1\n";
   connect( &submit_socket, SIGNAL( connected() ), this, SLOT( socket_connected() ) );
   cout << "after connect2\n";
   connect( &submit_socket, SIGNAL( readyRead() ), this, SLOT( socket_readyRead() ) );
   cout << "after connect3\n";
   connect( &submit_socket, SIGNAL( connectionClosed() ), this, SLOT( socket_connectionClosed() ) );
   cout << "after connect4\n";
   
   return false;
}

void US_Hydrodyn_Cluster_Submit::socket_error( int error_no )
{
   cout << "socket error: " << error_no << "\n";
   disconnect( &submit_socket, SIGNAL( error( int ) ), 0, 0 );
   disconnect( &submit_socket, SIGNAL( connected() ),0, 0 );
   disconnect( &submit_socket, SIGNAL( readyRead() ), 0, 0 );
   disconnect( &submit_socket, SIGNAL( connectionClosed() ), 0, 0 );

   comm_active = false;
   update_enables(); 
}

void US_Hydrodyn_Cluster_Submit::socket_connected()
{
   cout << "socket connected\n";
   // transfer the request
   QTextStream os( &submit_socket );
   os << current_xml;
   //    if ( current_xml.length() != 
   //         (unsigned int)submit_socket.writeBlock( current_xml, current_xml.length() ) )
   //   {
   //      cout << "didn't write the requested length!";
   //   }
   cout << "transferred request:" << current_xml << "\n";
}

void US_Hydrodyn_Cluster_Submit::socket_readyRead()
{
   cout << "socket: readyRead\n";
   while ( submit_socket.canReadLine() )
   {
      current_xml_response += submit_socket.readLine();
   }
   sleep(1);
   while ( submit_socket.canReadLine() )
   {
      current_xml_response += submit_socket.readLine();
   }
   cout << "response: " << current_xml_response << endl;
   // submit_socket.close();
}

void US_Hydrodyn_Cluster_Submit::socket_connectionClosed()
{
   cout << "socket connection closed\n";
   if ( submit_socket.state() == QSocket::Closing ) 
   {
      cout << "socked delayed close\n";
      connect( &submit_socket, SIGNAL( connectionClosed() ), this, SLOT( socket_connectionClosed() ) );
      connect( &submit_socket, SIGNAL( delayedCloseFinished() ), this, SLOT( socket_delayedCloseFinished() ) );
   } else {
      disconnect( &submit_socket, SIGNAL( error( int ) ), 0, 0 );
      disconnect( &submit_socket, SIGNAL( connected() ),0, 0 );
      disconnect( &submit_socket, SIGNAL( readyRead() ), 0, 0 );
      disconnect( &submit_socket, SIGNAL( connectionClosed() ), 0, 0 );
      comm_active = false;
      update_enables(); 
   }
}

void US_Hydrodyn_Cluster_Submit::socket_delayedCloseFinished()
{
   cout << "socket delayed close finished\n";
   disconnect( &submit_socket, SIGNAL( delayedCloseFinished() ), 0, 0 );
   disconnect( &submit_socket, SIGNAL( error( int ) ), 0, 0 );
   disconnect( &submit_socket, SIGNAL( connected() ),0, 0 );
   disconnect( &submit_socket, SIGNAL( readyRead() ), 0, 0 );
   disconnect( &submit_socket, SIGNAL( connectionClosed() ), 0, 0 );
   comm_active = false;
   update_enables(); 
}

bool US_Hydrodyn_Cluster_Submit::submit_jobs( QStringList files )
{
   errormsg = "";
   for ( unsigned int i = 0; i < files.size(); i++ )
   {
      QString xml;
      if ( !submit_xml( files[ i ], xml ) ||
           !send_xml( xml ) )
      {
         return false;
      }
   }
   return true;
}

bool US_Hydrodyn_Cluster_Submit::submit_xml( QString file, QString &xml )
{
   errormsg = "";
   if ( !QFile::exists( file ) )
   {
      errormsg = QString( tr( "submit_xml: File %1 does not exist" ) ).arg( file );
      return false;
   }

   // get a count of the files in a tar file
   QStringList tar_list;
   if ( file.contains( QRegExp( "\\.(tar|TAR)$" ) ) )
   {
      US_Tar ust;
      int result = ust.list( file, tar_list, true );
      if ( result != TAR_OK )
      {
         errormsg = QString( tr( "submit_xml: Listing tar archive %1 failed %2" ) )
            .arg( file )
            .arg( ust.explain( result ) );
         return false;
      }
   }

   // right now, just lonestar 12 ppn
   unsigned int host_count      = ( tar_list.size() / 12 ) + 1;
   unsigned int processor_count = host_count * 12;

   xml = QString( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                  "<Message>"
                  "<Header>"
                  "<experimentid>%1</experimentid>"
                  "<hostname>%2</hostname>"
                  "<processorcount>%3</processorcount>"
                  "<hostcount>%4</hostcount>"
                  "<queuename>normal</queuename>"
                  "<walltime>%5</walltime>"
                  "</Header>"
                  "<Body>"
                  "<Method>Run</Method>"
                  "<input>"
                  "<parameters>"
                  "<name>param</name>"
                  "<value>iq</value>"
                  "</parameters>"
                  "<parameters>  "
                  "<name>inputfile</name>"
                  "<value>%6</value>"
                  "</parameters>"
                  "</input>"
                  "</Body>"
                  "</Message>"
                  "\n" )
      .arg( QString( "US-SOMO-%1-%2" ).arg( cluster_id ).arg( file ) )
      .arg( stage_url )
      .arg( processor_count )
      .arg( host_count )
      .arg( 600 ) // for now, we should determine expected run times
      .arg( QString( "%1/%2/%3" ).arg( stage_path ).arg( cluster_id ).arg( file ) )
      ;
   cout << xml << endl;
   if ( !submit_active )
   {
      errormsg = tr( "Stopped by user request" );
      return false;
   }
   return true;
}

void US_Hydrodyn_Cluster_Submit::stop()
{
   if ( submit_active )
   {
      submit_active = false;
   }
   if ( comm_active )
   {
      submit_socket.close();
   }
   update_enables();
}
