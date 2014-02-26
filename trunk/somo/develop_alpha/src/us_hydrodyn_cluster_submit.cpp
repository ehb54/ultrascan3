#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_cluster.h"
#include "../include/us_hydrodyn_cluster_submit.h"
//Added by qt3to4:
#include <Q3TextStream>
#include <Q3HBoxLayout>
#include <QLabel>
#include <Q3Frame>
#include <Q3PopupMenu>
#include <Q3VBoxLayout>
#include <Q3BoxLayout>
#include <QCloseEvent>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#define SLASH QDir::separator()

US_Hydrodyn_Cluster_Submit::US_Hydrodyn_Cluster_Submit(
                                         void *us_hydrodyn, 
                                         QWidget *p, 
                                         const char *name
                                         ) : QDialog(p, name)
{
   this->us_hydrodyn = us_hydrodyn;
   setCaption(tr("US-SOMO: Cluster Submit"));
   cluster_window = (void *)p;
   USglobal = new US_Config();

   comm_active = false;
   submit_active = false;
   system_proc_active = false;
   disable_updates = false;

   ftp_file = ( QFile * ) 0;

   pkg_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "cluster";
   QDir::setCurrent( pkg_dir );

   cluster_id      = ((US_Hydrodyn_Cluster *) cluster_window )->cluster_config[ "userid" ];
   cluster_pw      = ((US_Hydrodyn_Cluster *) cluster_window )->cluster_config[ "userpw" ];

   submit_url      = ((US_Hydrodyn_Cluster *) cluster_window )->cluster_config[ "server" ];
   submit_url_host = submit_url;
   submit_url_port = submit_url;

   submit_url_host.replace( QRegExp( ":.*$" ), "" );
   submit_url_port.replace( QRegExp( "^.*:" ), "" );
   cout << submit_url_host << endl;
   cout << submit_url_port << endl;

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
                                tr("US-SOMO: Cluster Jobs"),
                                tr("Cluster credentials must be set in Cluster Config before continuing"),
                                0 );
      return;
   }

   if ( !update_files( false ) )
   {
      QMessageBox::information( this, 
                                tr("US-SOMO: Cluster Jobs"),
                                tr("No unsubmitted jobs found"),
                                0 );
   }

   setupGUI();
   editor_msg("blue", "THIS WINDOW IS UNDER DEVELOPMENT." );

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

   update_files();
   update_enables();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 700, 600 );
}

US_Hydrodyn_Cluster_Submit::~US_Hydrodyn_Cluster_Submit()
{
}

unsigned int US_Hydrodyn_Cluster_Submit::update_files( bool set_lv_files )
{
   files.clear();

   // traverse directory and build up files
   QDir::setCurrent( pkg_dir );
   QDir qd;
   QStringList tgz_files = qd.entryList( "*.tgz" );
   QStringList tar_files = qd.entryList( "*.tar" );
   QStringList all_files = QStringList::split( "\n", 
                                               tgz_files.join("\n") + 
                                               ( tgz_files.size() ? "\n" : "" ) +
                                               tar_files.join("\n") );

   for ( unsigned int i = 0; i < ( unsigned int ) all_files.size(); i++ )
   {
      if ( !all_files[ i ].contains( "_out", false ) )
      {
         files << all_files[ i ];
      }
   }
   
   if ( set_lv_files )
   {
      lv_files->clear();
      for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
      {
         cout << "files: " << files[ i ] << endl;
         // qt3 QFileInfo::size() is incorrect uint is too small
         // so use fstat()

         new Q3ListViewItem( lv_files, 
                            files[ i ], 
                            QString( " %1 " ).arg( QFileInfo( files[ i ] ).created().toString() ),
                            QString( " %1 bytes " ).arg( QFileInfo( files[ i ] ).size() )
                            );
      }
   }

   return files.size();
}

void US_Hydrodyn_Cluster_Submit::setupGUI()
{
   int minHeight1 = 30;

   lbl_title = new QLabel( tr( "Submit jobs to cluster" ), this);
   lbl_title->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette( PALET_FRAME );
   AUTFBACK( lbl_title );
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_files = new QLabel(tr("Available jobs:"), this);
   lbl_files->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_files->setMinimumHeight(minHeight1);
   lbl_files->setPalette( PALET_LABEL );
   AUTFBACK( lbl_files );
   lbl_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   lv_files = new Q3ListView(this);
   lv_files->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   lv_files->setMinimumHeight(minHeight1 * 3);
   lv_files->setPalette( PALET_EDIT );
   AUTFBACK( lv_files );
   lv_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   lv_files->setEnabled(true);
   lv_files->setSelectionMode( Q3ListView::Multi );

   lv_files->addColumn( tr( "Name" ) );
   lv_files->addColumn( tr( "Created" ) );
   lv_files->addColumn( tr( "Size" ) );
   lv_files->addColumn( tr( "Status" ) );

   connect( lv_files, SIGNAL( selectionChanged() ), SLOT( update_enables() ) );

   lbl_systems = new QLabel(tr("Systems"), this);
   lbl_systems->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_systems->setMinimumHeight(minHeight1);
   lbl_systems->setPalette( PALET_LABEL );
   AUTFBACK( lbl_systems );
   lbl_systems->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   lb_systems = new Q3ListBox(this);
   lb_systems->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   lb_systems->setMinimumHeight(minHeight1 * 
                                ( ((US_Hydrodyn_Cluster *)cluster_window)->cluster_systems.size() > 8 ?
                                  8 : ((US_Hydrodyn_Cluster *)cluster_window)->cluster_systems.size() ) );
   lb_systems->setMinimumWidth( 500 );
   lb_systems->setPalette( PALET_EDIT );
   AUTFBACK( lb_systems );
   lb_systems->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   lb_systems->setEnabled(true);

   for ( map < QString, map < QString, QString > >::iterator it = 
            ((US_Hydrodyn_Cluster *)cluster_window)->cluster_systems.begin();
         it != ((US_Hydrodyn_Cluster *)cluster_window)->cluster_systems.end();
         it++ )
   {
      lb_systems->insertItem( it->first );
   }

   lb_systems->setCurrentItem(0);
   lb_systems->setSelected(0, false);
   lb_systems->setSelectionMode( Q3ListBox::Single );
   // lb_systems->setColumnMode( QListBox::FitToWidth );
   connect( lb_systems, SIGNAL( selectionChanged() ), SLOT( systems() ) );

   pb_select_all = new QPushButton(tr("Select all jobs"), this);
   pb_select_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_select_all->setMinimumHeight(minHeight1);
   pb_select_all->setPalette( PALET_PUSHB );
   connect(pb_select_all, SIGNAL(clicked()), SLOT(select_all()));

   pb_remove = new QPushButton(tr("Remove selected jobs"), this);
   pb_remove->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_remove->setMinimumHeight(minHeight1);
   pb_remove->setPalette( PALET_PUSHB );
   connect(pb_remove, SIGNAL(clicked()), SLOT( remove() ));

   pb_submit = new QPushButton(tr("Submit selected jobs"), this);
   pb_submit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_submit->setMinimumHeight(minHeight1);
   pb_submit->setPalette( PALET_PUSHB );
   connect(pb_submit, SIGNAL(clicked()), SLOT( submit()) );
   
   progress = new Q3ProgressBar(this, "FTP Progress");
   progress->setMinimumHeight( minHeight1 );
   progress->setPalette( PALET_NORMAL );
   AUTFBACK( progress );
   progress->reset();

   connect( &ftp, SIGNAL( dataTransferProgress ( int, int  ) ), progress, SLOT( setProgress        ( int, int  ) ) );
   connect( &ftp, SIGNAL( stateChanged         ( int       ) ), this    , SLOT( ftp_stateChanged   ( int       ) ) );
   connect( &ftp, SIGNAL( commandStarted       ( int       ) ), this    , SLOT( ftp_commandStarted ( int       ) ) );
   connect( &ftp, SIGNAL( commandFinished      ( int, bool ) ), this    , SLOT( ftp_commandFinished( int, bool ) ) );
   connect( &ftp, SIGNAL( done                 ( bool      ) ), this    , SLOT( ftp_done           ( bool      ) ) );

   editor = new Q3TextEdit(this);
   editor->setPalette( PALET_NORMAL );
   AUTFBACK( editor );
   editor->setReadOnly(true);

   Q3Frame *frame;
   frame = new Q3Frame(this);
   frame->setMinimumHeight(minHeight1);

   m = new QMenuBar(frame, "menu" );
   m->setMinimumHeight(minHeight1 - 5);
   m->setPalette( PALET_NORMAL );
   AUTFBACK( m );
   Q3PopupMenu * file = new Q3PopupMenu(editor);
   m->insertItem( tr("&File"), file );
   file->insertItem( tr("Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
   file->insertItem( tr("Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );
   file->insertItem( tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );
   editor->setWordWrap (Q3TextEdit::WidgetWidth);
   editor->setMinimumHeight(100);

   pb_stop = new QPushButton(tr("Stop"), this);
   pb_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_stop->setMinimumHeight(minHeight1);
   pb_stop->setPalette( PALET_PUSHB );
   connect(pb_stop, SIGNAL(clicked()), SLOT(stop()));
   pb_stop->setEnabled( false );

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // build layout

   Q3HBoxLayout *hbl_buttons1 = new Q3HBoxLayout( 0 );
   hbl_buttons1->addSpacing( 4 );
   hbl_buttons1->addWidget ( pb_select_all);
   hbl_buttons1->addSpacing( 4 );
   hbl_buttons1->addWidget ( pb_remove );
   hbl_buttons1->addSpacing( 4 );
   hbl_buttons1->addWidget ( pb_submit );
   hbl_buttons1->addSpacing( 4 );

   Q3HBoxLayout *hbl_bottom = new Q3HBoxLayout( 0 );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_stop );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_cancel );
   hbl_bottom->addSpacing( 4 );

   Q3BoxLayout *vbl_editor_group = new Q3VBoxLayout(0);
   vbl_editor_group->addWidget(frame);
   vbl_editor_group->addWidget(editor);

   Q3VBoxLayout *background = new Q3VBoxLayout( this );
   background->addSpacing( 4);
   background->addWidget ( lbl_title );
   background->addSpacing( 4 );
   background->addWidget ( lbl_files );
   background->addSpacing( 4 );
   background->addWidget ( lv_files );
   background->addSpacing( 4 );
   background->addWidget ( lbl_systems );
   background->addSpacing( 4 );
   background->addWidget ( lb_systems );
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

void US_Hydrodyn_Cluster_Submit::systems()
{
   for ( int i = 0; i < lb_systems->numRows(); i++ )
   {
      if ( lb_systems->isSelected(i) )
      {
         selected_system_name = lb_systems->text( i );
         cout << "run config systems for " << selected_system_name << "\n";
         
         if ( ((US_Hydrodyn_Cluster *)cluster_window)->cluster_systems.count( selected_system_name ) )
         {
            selected_system = ((US_Hydrodyn_Cluster *)cluster_window)->cluster_systems[ selected_system_name ];
            if ( selected_system.count( "stage" ) &&
                 selected_system.count( "type" ) &&
                 selected_system.count( "corespernode" ) &&
                 selected_system.count( "ftp" ) &&
                 selected_system.count( "maxcores" ) &&
                 selected_system.count( "queue" ) &&
                 selected_system.count( "runtime" ) )
            {
               stage_url       = selected_system[ "stage" ];
               stage_path      = stage_url;
               stage_url_path  = stage_url;
               stage_url       .replace( QRegExp( ":.*$" ), "" );
               stage_path      .replace( QRegExp( "^.*:" ), "" );
               stage_url_path  += QString( "%1%2%3" ).arg( QDir::separator() ).arg( cluster_id ).arg( QDir::separator() );
               ftp_url         = selected_system[ "ftp" ];
               ftp_url_host    = ftp_url;
               ftp_url_port    = ftp_url;
               ftp_url_host    .replace( QRegExp( ":.*$" ), "" );
               ftp_url_port    .replace( QRegExp( "^.*:" ), "" );
            } else {
               QMessageBox::warning( this, 
                                     tr( "US-SOMO: Cluster config" ), 
                                     QString( tr( "The system %1 does not seem to have sufficient configuration information defined" ) ).arg( lb_systems->text( i ) ) );
               lb_systems->clearSelection();
            } 
         } else {
            QMessageBox::warning( this, 
                                  tr( "US-SOMO: Cluster config" ), 
                                  QString( tr( "The system %1 does not seem to have any information" ) ).arg( lb_systems->text( i ) ) );
            lb_systems->clearSelection();
         }            
      }
   }

   update_enables();
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
   fn = Q3FileDialog::getSaveFileName(QString::null, QString::null, this );
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

void US_Hydrodyn_Cluster_Submit::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->color();
   editor->setColor(color);
   editor->append(msg);
   editor->setColor(save_color);
   editor->scrollToBottom();
}
   
void US_Hydrodyn_Cluster_Submit::update_enables()
{
   bool any_systems = false;

   for ( int i = 0; i < lb_systems->numRows(); i++ )
   {
      if ( lb_systems->isSelected( i ) )
      {
         cout << lb_systems->text( i ) << endl;
         any_systems = true;
      }
   }

   bool running = comm_active || submit_active;
   pb_stop      ->setEnabled( running );
   pb_select_all->setEnabled( !running );
   pb_remove    ->setEnabled( !running );
   pb_submit    ->setEnabled( !running );
   pb_help      ->setEnabled( !running );
   pb_cancel    ->setEnabled( !running );
   lv_files     ->setEnabled( !running );

   if ( !running && !disable_updates )
   {
      bool any_selected = false;
      Q3ListViewItem *lvi = lv_files->firstChild();
      if ( lvi )
      {
         do {
            if ( lvi->isSelected() )
            {
               any_selected = true;
            }
         } while ( ( lvi = lvi->nextSibling() ) );
      }
      pb_remove->setEnabled( any_selected );
      pb_submit->setEnabled( any_selected && any_systems );
   }
}

void US_Hydrodyn_Cluster_Submit::select_all()
{
   bool any_not_selected = false;
   Q3ListViewItem *lvi = lv_files->firstChild();
   if ( lvi )
   {
      do {
         if ( !lvi->isSelected() )
         {
            any_not_selected = true;
         }
      } while ( ( lvi = lvi->nextSibling() ) );
   }

   disable_updates = true;
   lvi = lv_files->firstChild();
   if ( lvi )
   {
      do {
         lv_files->setSelected( lvi, any_not_selected );
      } while ( ( lvi = lvi->nextSibling() ) );
   }
   disable_updates = false;
   update_enables();
}

bool US_Hydrodyn_Cluster_Submit::submit_xml( QString file, QString &xml )
{
   errormsg = "";
   cout << "submit_xml\n";
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

   if ( !selected_system[ "corespernode" ].toUInt() )
   {
      errormsg = QString( tr( "submit_xml: The selected system %1 does not have a positive cores per node defined" ) )
         .arg( selected_system_name );
      return false;
   }

   if ( !selected_system[ "maxcores" ].toUInt() )
   {
      errormsg = QString( tr( "submit_xml: The selected system %1 does not have a positive max cores defined" ) )
         .arg( selected_system_name );
      return false;
   }

   if ( !selected_system[ "runtime" ].toUInt() )
   {
      errormsg = QString( tr( "submit_xml: The selected system %1 does not have a positive run time defined" ) )
         .arg( selected_system_name );
      return false;
   }

   unsigned int common_count = ( unsigned int ) tar_list.grep( QRegExp( "^common_" ) ).size();


   unsigned int job_count = ( unsigned int ) tar_list.size() - common_count - 1;
   
   {
      QRegExp rx( "^(bfnb|bfnbpm|oned|best)_p(\\d+)_" );
      if ( rx.search( file ) != -1 )
      {
         job_count = rx.cap( 2 ).toUInt();
         cout << QString( "host count, %1\n" ).arg( job_count );
         if ( job_count < 2 )
         {
            job_count = 2;
         }
      }
   }

   unsigned int host_count      = 
      ( job_count / selected_system[ "corespernode" ].toUInt() ) + 1;

   unsigned int processor_count = host_count * selected_system[ "corespernode" ].toUInt();
   if ( processor_count > selected_system[ "maxcores" ].toUInt() )
   {
      processor_count = selected_system[ "maxcores" ].toUInt();
      host_count      = processor_count / selected_system[ "corespernode" ].toUInt();
   }

   QString target_dir = QString( "%1" ).arg( next_to_process->text( 0 ) ).replace( QRegExp( "\\.(tar|tgz)$" ), "" );

   xml = QString( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                  "<Message>"
                  "<Header>" );
   xml += QString( "<userdn>%1</userdn>" ).arg( cluster_id );


   QString job_type = "iq";
   if ( file.contains( QRegExp( "^bfnb_" ) ) )
   {
      job_type = "nsa";
   }
   if ( file.contains( QRegExp( "^bfnbpm_" ) ) )
   {
      job_type = "pm";
   }
   if ( file.contains( QRegExp( "^oned_" ) ) )
   {
      job_type = "1d";
   }

   xml += QString( 
                  "<experimentid>%1</experimentid>"
                  "<hostname>%2</hostname>"
                  "<processorcount>%3</processorcount>"
                  "%4"
                  "<walltime>%5</walltime>"
                  "<userdn>%6</userdn>"
                  "%7"
                  "</Header>"
                  "<Body>"
                  "<Method>run</Method>"
                  "<input>"
                  "<parameters>"
                  "<name>param</name>"
                  "<value>%8</value>"
                  "</parameters>"
                  "<parameters>"
                  "<name>inputfile</name>"
                  "<value>%9</value>"
                  "</parameters>"
                  "</input>"
                  "</Body>"
                  "</Message>"
                  "\n" )

      .arg( QString( "%1-%2" ).arg( cluster_id ).arg( file ) )
      .arg( stage_url )
      .arg( processor_count )
      .arg( selected_system[ "queue" ].isEmpty() ? "" : QString( "<queuename>%1</queuename>" ).arg( selected_system[ "queue" ] ) )
      .arg( selected_system[ "runtime" ].toUInt() )
      .arg( cluster_id )
      .arg( selected_system[ "executable" ].isEmpty() ? "" : QString( "<executable>%1</executable>" ).arg( selected_system[ "executable" ] ) )
      .arg( job_type )
      .arg( QString( "%1/%2" )
            .arg( target_dir )
            .arg( file ) );

   cout << xml << endl;
   if ( !submit_active )
   {
      errormsg = tr( "Stopped by user request" );
      update_files();
      update_enables();
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
      submit_http.abort();
      ftp.abort();
   }
   if ( system_proc_active )
   {
      system_proc->tryTerminate();
      QTimer::singleShot( 2500, system_proc, SLOT( kill() ) );
   }
   update_enables();
}

void US_Hydrodyn_Cluster_Submit::submit()
{
   submit_active = true;
   update_enables();
   cout << "setup jobs\n";
   jobs.clear();
   
   QStringList qsl_submit;
   Q3ListViewItem *lvi = lv_files->firstChild();
   if ( lvi )
   {
      do {
         if ( lvi->isSelected() )
         {
            jobs[ lvi ] = "prepare stage";
         }
      } while ( ( lvi = lvi->nextSibling() ) );
      
      emit process_next();
   }
}

void US_Hydrodyn_Cluster_Submit::process_next()
{
   if ( !submit_active )
   {
      editor_msg( "red", "stopped by user request" );
      update_files();
      update_enables();
      return;
   }

   cout << "process next\n";
   if ( comm_active || system_proc_active )
   {
      cout << "comm or system proc active\n";
      return;
   }
   process_list();

   bool ok = true;
   for ( map < Q3ListViewItem *, QString >::iterator it = jobs.begin();
         it != jobs.end();
         it++ )
   {
      if ( it->second.contains( "failed" ) )
      {
         ok = false;
      }
      if ( it->second == "prepare stage" )
      {
         next_to_process = it->first;
         emit process_prepare_stage();
         return;
      }
      if ( it->second == "stage" )
      {
         next_to_process = it->first;
         emit process_stage();
         return;
      }
      if ( it->second == "submit" )
      {
         next_to_process = it->first;
         emit process_submit();
         return;
      }
      if ( it->second == "move" )
      {
         next_to_process = it->first;
         emit process_move();
         return;
      }
   }

   if ( ok )
   {
      editor_msg( "black", tr( "submission complete" ) );
   } else {
      editor_msg( "red", tr( "submission had errors" ) );
   }
      
   submit_active = false;

   if ( !update_files() )
   {
      QMessageBox::information( this, 
                                tr("US-SOMO: Cluster Jobs"),
                                tr("No further unsubmitted jobs found"),
                                0 );
      close();
   }

   update_enables();
}

void US_Hydrodyn_Cluster_Submit::process_prepare_stage()
{
   editor_msg( "black", QString( "preparing stage %1" ).arg( next_to_process->text( 0 ) ) );
   next_to_process->setText( 3, tr( "Preparing to stage" ) );
   cout << "process prepare stage\n";
   if ( prepare_stage( next_to_process->text( 0 ) ) )
   {
      jobs[ next_to_process ] = "stage";
   } else {
      editor_msg( "red", errormsg );
      next_to_process->setText( 3, tr( "Error: Prepare to stage failed" ) );
      jobs[ next_to_process ] = "prepare stage failed";
   }
   emit process_next();
}

void US_Hydrodyn_Cluster_Submit::process_stage()
{
   editor_msg( "black", QString( "staging %1" ).arg( next_to_process->text( 0 ) ) );
   next_to_process->setText( 3, tr( "Staging" ) );
   cout << "process stage\n";
   if ( stage( next_to_process->text( 0 ) ) )
   {
      jobs[ next_to_process ] = "submit";
   } else {
      editor_msg( "red", errormsg );
      next_to_process->setText( 3, tr( "Error: Staging failed" ) );
      jobs[ next_to_process ] = "stage failed";
   }
   emit process_next();
}

void US_Hydrodyn_Cluster_Submit::process_submit()
{
   editor_msg( "black", QString( "submitting %1" ).arg( next_to_process->text( 0 ) ) );
   next_to_process->setText( 3, tr( "Submitting" ) );
   cout << "process submit\n";
   if ( job_submit( next_to_process->text( 0 ) ) )
   {
      jobs[ next_to_process ] = "move";
   } else {
      editor_msg( "red", errormsg );
      next_to_process->setText( 3, tr( "Error: Submit failed" ) );
      jobs[ next_to_process ] = "submit failed";
   }
   emit process_next();
}

void US_Hydrodyn_Cluster_Submit::process_move()
{
   next_to_process->setText( 3, tr( "Moving to submitted" ) );
   editor_msg( "black", QString( "move %1 to submitted/" ).arg( next_to_process->text( 0 ) ) );
   cout << "process move\n";
   if ( move_file( next_to_process->text( 0 ) ) )
   {
      next_to_process->setText( 3, tr( "Moved to submitted" ) );
      jobs[ next_to_process ] = "complete";
   } else {
      editor_msg( "red", errormsg );
      next_to_process->setText( 3, tr( "Error: Moving to submitted failed" ) );
      jobs[ next_to_process ] = "move failed";
   }
   emit process_next();
}

bool US_Hydrodyn_Cluster_Submit::move_file( QString file )
{
   US_File_Util ufu;
   cout << "move file " << file << endl;

   if ( !QDir::setCurrent( pkg_dir ) )
   {
      errormsg = QString( tr( "move: can not change to directory %1" ) ).arg( pkg_dir );
      return false;
   }

   if ( !ufu.move( file, submitted_dir, true ) )
   {
      editor_msg( "red", ufu.errormsg );
      return false;
   }
   return true;
}

void US_Hydrodyn_Cluster_Submit::process_list()
{
   for ( map < Q3ListViewItem *, QString >::iterator it = jobs.begin();
         it != jobs.end();
         it++ )
   {
      editor_msg( "blue", QString("%1 %2").arg( it->first->text( 0 ) ).arg( it->second ) );
   }
}

bool US_Hydrodyn_Cluster_Submit::job_submit( QString file )
{
   QString xml;
   if ( !submit_xml( file, xml ) )
   { 
      return false;
   }

   // start comm process
   
   return send_http_post( xml );
}

bool US_Hydrodyn_Cluster_Submit::system_cmd( QStringList cmd )
{
   errormsg = "";
   cout << "syscmd: " << cmd.join( ":" ) << endl;

   if ( !cmd.size() )
   {
      errormsg = tr( "system_cmd called with no command" );
      return false;
   }

   system_proc = new Q3Process( this );

   system_proc->setArguments( cmd );

   system_proc_active = true;

   connect( system_proc, SIGNAL(readyReadStdout()), this, SLOT(system_proc_readFromStdout()) );
   connect( system_proc, SIGNAL(readyReadStderr()), this, SLOT(system_proc_readFromStderr()) );
   connect( system_proc, SIGNAL(processExited()),   this, SLOT(system_proc_processExited()) );
   connect( system_proc, SIGNAL(launchFinished()),  this, SLOT(system_proc_launchFinished()) );

   return system_proc->start();
}

void US_Hydrodyn_Cluster_Submit::system_proc_readFromStdout()
{
   while ( system_proc->canReadLineStdout() )
   {
      editor_msg("brown", system_proc->readLineStdout());
   }
}
   
void US_Hydrodyn_Cluster_Submit::system_proc_readFromStderr()
{
   while ( system_proc->canReadLineStderr() )
   {
      editor_msg("red", system_proc->readLineStderr());
   }
}
   
void US_Hydrodyn_Cluster_Submit::system_proc_processExited()
{
   cout << "system_proc exit\n";
   system_proc_readFromStderr();
   system_proc_readFromStdout();

   disconnect( system_proc, SIGNAL(readyReadStdout()), 0, 0);
   disconnect( system_proc, SIGNAL(readyReadStderr()), 0, 0);
   disconnect( system_proc, SIGNAL(processExited()), 0, 0);

   // editor->append("System_Proc finished.");
   system_proc_active = false;
   delete system_proc;
   emit process_next();
}
   
void US_Hydrodyn_Cluster_Submit::system_proc_launchFinished()
{
   editor_msg("brown", "System_Proc launch exited");
   disconnect( system_proc, SIGNAL(launchFinished()), 0, 0);
}

bool US_Hydrodyn_Cluster_Submit::prepare_stage( QString file )
{
   errormsg = "";
   if ( !QDir::setCurrent( pkg_dir ) )
   {
      errormsg = QString( tr( "stage: can not change to directory %1" ) ).arg( pkg_dir );
      return false;
   }

   if ( !QFile::exists( file ) )
   {
      errormsg = QString( tr( "stage: can not find file %1" ) ).arg( file );
      return false;
   }

   // using qftp now
   // need new method to clear dir
   // probably login, if directory exists, change,
   // list all and remove all
   // will ftp.rm( "*" ) work?
   // followed by ftp.rmdir( );
   comm_active = true;

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
      .arg( file )
      .arg( cluster_id )
      .arg( cluster_pw );
   
   ftp.connectToHost( ftp_url_host, ftp_url_port.toUInt() );
   ftp.login        ( cluster_id  , cluster_pw   );
   ftp.mkdir        ( target_dir );
   ftp.close        ();

   return true;
}


//   // right now we are using scp, need to come up with a better method

//   // make sure dest dir is made & clean!

//   QStringList cmd;
//   cmd << "ssh";
//   cmd << stage_url;
//   cmd << QString( "rm -fr %1/%2/%3; mkdir -p %4/%5/%6" )
//      .arg( stage_path )
//      .arg( cluster_id )
//      .arg( QString("%1").arg( file ).replace( QRegExp( "\\.(tgz|tar|TGZ|TAR)$" ), "" ) )
//      .arg( stage_path )
//      .arg( cluster_id )
//      .arg( QString("%1").arg( file ).replace( QRegExp( "\\.(tgz|tar|TGZ|TAR)$" ), "" ) );

//   return( system_cmd( cmd ) );
// }

bool US_Hydrodyn_Cluster_Submit::stage( QString file )
{
   errormsg = "";
   if ( !QDir::setCurrent( pkg_dir ) )
   {
      errormsg = QString( tr( "stage: can not change to directory %1" ) ).arg( pkg_dir );
      return false;
   }

   if ( !QFile::exists( file ) )
   {
      errormsg = QString( tr( "stage: can not find file %1" ) ).arg( file );
      return false;
   }


   ftp_file = new QFile( file );
   if ( !ftp_file->open( QIODevice::ReadOnly ) )
   {
      errormsg = QString( tr( "stage: can not open file %1" ) ).arg( file );
      delete ftp_file;
      ftp_file = ( QFile * ) 0;
      return false;
   }

   // setup stuff for ftp
   comm_active = true;

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
      .arg( file )
      .arg( cluster_id )
      .arg( cluster_pw );
   
   ftp.connectToHost( ftp_url_host, ftp_url_port.toUInt() );
   ftp.login        ( cluster_id  , cluster_pw   );
   ftp.cd           ( target_dir );
   ftp.put          ( ftp_file    , file         );
   ftp.close        ();

   return true;
}


   //   QStringList cmd;
   //   cmd << "scp";
   //   cmd << QString( "%1%2%3" )
   //      .arg( pkg_dir )
   //      .arg( QDir::separator() )
   //      .arg( file );

   //   cmd << QString( "%1%2/" )
   //      .arg( stage_url_path )
   //      .arg( QString("%1").arg( file ).replace( QRegExp( "\\.(tgz|tar|TGZ|TAR)$" ), "" ) );

   //   return( system_cmd( cmd ) );
   //}


void US_Hydrodyn_Cluster_Submit::remove()
{
   bool any_selected = false;
   Q3ListViewItem *lvi = lv_files->firstChild();
   if ( lvi )
   {
      do {
         if ( lvi->isSelected() )
         {
            any_selected = true;
         }
      } while ( ( lvi = lvi->nextSibling() ) );
   }

   if ( !any_selected )
   {
      return;
   }

   if ( QMessageBox::question(
                              this,
                              tr("US-SOMO: Cluster Jobs: Remove"),
                              tr( "The jobs will be permenantly removed.\n"
                                  "Are you sure? " ),
                              tr("&Yes"), tr("&No"),
                              QString::null, 0, 1 ) )
   {
      editor_msg( "black", tr( "Remove canceled by user" ) );
      return;
   }

   if ( !QDir::setCurrent( pkg_dir ) )
   {
      editor_msg( "red" , QString( tr( "can not change to directory %1" ) ).arg( pkg_dir ) );
      return;
   }

   lvi = lv_files->firstChild();

   if ( lvi )
   {
      do {
         if ( lvi->isSelected() )
         {
            if ( !QFile::remove( lvi->text( 0 ) ) )
            {
               editor_msg( "red" , QString( tr( "can not remove file %1" ) ).arg( lvi->text( 0 ) ) );
            } else {
               editor_msg( "black" , QString( tr( "Removed file: %1" ) ).arg( lvi->text( 0 ) ) );
            }
         }
      } while ( ( lvi = lvi->nextSibling() ) );
   }
   if ( !update_files() )
   {
      QMessageBox::information( this, 
                                tr("US-SOMO: Cluster Jobs"),
                                tr("No further unsubmitted jobs found"),
                                0 );
      close();
   }
}

bool US_Hydrodyn_Cluster_Submit::send_http_post( QString xml )
{
   // need to do a post & get to submit_url slash stuff
   // its going to require opening a socket etc
   // 
   comm_active = true;
   cout << "http_get\n";
   // editor_msg( "black", "updating status" );

   current_http_response = "";

   connect( &submit_http, SIGNAL( stateChanged ( int ) ), this, SLOT( http_stateChanged ( int ) ) );
   connect( &submit_http, SIGNAL( responseHeaderReceived ( const Q3HttpResponseHeader & ) ), this, SLOT( http_responseHeaderReceived ( const Q3HttpResponseHeader & ) ) );
   connect( &submit_http, SIGNAL( readyRead ( const Q3HttpResponseHeader & ) ), this, SLOT( http_readyRead ( const Q3HttpResponseHeader & ) ) );
   connect( &submit_http, SIGNAL( dataSendProgress ( int, int ) ), this, SLOT( http_dataSendProgress ( int, int ) ) );
   connect( &submit_http, SIGNAL( dataReadProgress ( int, int ) ), this, SLOT( http_dataReadProgress ( int, int ) ) );
   connect( &submit_http, SIGNAL( requestStarted ( int ) ), this, SLOT( http_requestStarted ( int ) ) );
   connect( &submit_http, SIGNAL( requestFinished ( int, bool ) ), this, SLOT( http_requestFinished ( int, bool ) ) );
   connect( &submit_http, SIGNAL( done ( bool ) ), this, SLOT( http_done ( bool ) ) );

   Q3HttpRequestHeader header("POST", "/ogce-rest/job/runjob/async" );
   header.setValue( "Host", submit_url_host );
   header.setContentType( "application/xml" );
   submit_http.setHost( submit_url_host, submit_url_port.toUInt() );
   // without the qba below, QHttp:request will send a null
   QByteArray qba = xml.utf8();
   qba.resize( qba.size() - 1 );
   submit_http.request( header, qba );

   return true;
}

void US_Hydrodyn_Cluster_Submit::http_stateChanged ( int /* estate */ )
{
   // editor_msg( "blue", QString( "http state %1" ).arg( state ) );
}

void US_Hydrodyn_Cluster_Submit::http_responseHeaderReceived ( const Q3HttpResponseHeader & resp )
{
   cout << resp.reasonPhrase() << endl;
}

void US_Hydrodyn_Cluster_Submit::http_readyRead( const Q3HttpResponseHeader & resp )
{
   cout << "http: readyRead\n";
   cout << resp.reasonPhrase() << endl;
   // current_http_response = QString( "%1" ).arg( submit_http.readAll() );
   current_http_response = QString( submit_http.readAll() );
   cout << current_http_response << endl;

   /*   if ( comm_mode == "status" )
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
      } */
}

void US_Hydrodyn_Cluster_Submit::http_dataSendProgress ( int done, int total )
{
   cout << "http: datasendprogress " << done << " " << total << "\n";

}
void US_Hydrodyn_Cluster_Submit::http_dataReadProgress ( int done, int total )
{
   cout << "http: datareadprogress " << done << " " << total << "\n";
}

void US_Hydrodyn_Cluster_Submit::http_requestStarted ( int id )
{
   cout << "http: requestStarted " << id << "\n";
}

void US_Hydrodyn_Cluster_Submit::http_requestFinished ( int id, bool error )
{
   cout << "http: requestFinished " << id << " " << error << "\n";
}

void US_Hydrodyn_Cluster_Submit::http_done ( bool error )
{
   cout << "http: done " << error << "\n";
   disconnect( &submit_http, SIGNAL( stateChanged ( int ) ), 0, 0 );
   disconnect( &submit_http, SIGNAL( responseHeaderReceived ( const Q3HttpResponseHeader & ) ), 0, 0 );
   disconnect( &submit_http, SIGNAL( readyRead ( const Q3HttpResponseHeader & ) ), 0, 0 );
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
      case Q3Http::NoError :
         current_http_error = tr( "No error occurred." );
         break;

      case Q3Http::HostNotFound:
         current_http_error = tr( "The host name lookup failed." );
         break;

      case Q3Http::ConnectionRefused:
         current_http_error = tr( "The server refused the connection." );
         break;

      case Q3Http::UnexpectedClose:
         current_http_error = tr( "The server closed the connection unexpectedly." );
         break;

      case Q3Http::InvalidResponseHeader:
         current_http_error = tr( "The server sent an invalid response header." );
         break;

      case Q3Http::WrongContentLength:
         current_http_error = tr( "The client could not read the content correctly because an error with respect to the content length occurred." );
         break;

      case Q3Http::Aborted:
         current_http_error = tr( "The request was aborted with abort()." );
         break;

      case Q3Http::UnknownError:
      default:
         current_http_error = tr( "Unknown Error." );
         break;
      }
      cout << current_http_error << endl;
      QMessageBox::warning( this,
                            tr("US-SOMO: Cluster Config"), 
                            tr( QString( "There was a error with the management server:\n%1" )
                                .arg( current_http_error ) ),
                            QMessageBox::Ok,
                            QMessageBox::NoButton
                            );
      submit_active = false;
      editor_msg( "red", tr( "Error: " ) + current_http_error );
      update_enables();
      return;
   }
   emit process_next();
}

void US_Hydrodyn_Cluster_Submit::ftp_stateChanged ( int state )
{
   cout << "ftp state changed: " << state << endl;
   if ( state == Q3Ftp::Unconnected )
   {
      cout << "- There is no connection to the host. \n";
   }
   if ( state == Q3Ftp::HostLookup )
   {
      cout << "- A host name lookup is in progress. \n";
   }
   if ( state == Q3Ftp::Connecting )
   {
      cout << "- An attempt to connect to the host is in progress. \n";
   }
   if ( state == Q3Ftp::Connected )
   {
      cout << "- Connection to the host has been achieved. \n";
   }
   if ( state == Q3Ftp::LoggedIn )
   {
      cout << "- Connection and user login have been achieved. \n";
   }
   if ( state == Q3Ftp::Closing )
   {
      cout << "- The connection is closing down, but it is not yet closed.\n";
   }
}

void US_Hydrodyn_Cluster_Submit::ftp_commandStarted ( int id )
{
   cout << "ftp command started: " << id << endl;
}

void US_Hydrodyn_Cluster_Submit::ftp_commandFinished ( int id, bool error )
{
   cout << "ftp command finished" << id 
        << QString( " %1\n" ).arg( error ? QString( "error %1" ).arg( ftp.errorString() ) : "" );
}

void US_Hydrodyn_Cluster_Submit::ftp_done ( bool error )
{
   cout << "ftp done" 
        << QString( " %1\n" ).arg( error ? QString( "error %1" ).arg( ftp.errorString() ) : "" );
   if ( error &&
        jobs[ next_to_process ] == "submit" )
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
   emit process_next();
}
