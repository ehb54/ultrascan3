#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_cluster.h"
#include "../include/us_hydrodyn_cluster_submit.h"
//Added by qt3to4:
#include <QTextStream>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
 //#include <Q3PopupMenu>
#include <QVBoxLayout>
#include <QBoxLayout>
#include <QCloseEvent>

#define CLUSTER_OVERRIDE_NPROC 1

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#define SLASH QDir::separator()

US_Hydrodyn_Cluster_Submit::US_Hydrodyn_Cluster_Submit(
                                         void *us_hydrodyn, 
                                         QWidget *p, 
                                         const char *
                                         ) : QDialog( p )
{
   this->us_hydrodyn = us_hydrodyn;
   setWindowTitle(us_tr("US-SOMO: Cluster Submit"));
   cluster_window = (void *)p;
   USglobal = new US_Config();

   comm_active = false;
   submit_active = false;
   system_proc_active = false;
   disable_updates = false;

   http_access_manager = new QNetworkAccessManager( this );

   pkg_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "cluster";
   QDir::setCurrent( pkg_dir );

   cluster_id      = ((US_Hydrodyn_Cluster *) cluster_window )->cluster_config[ "userid" ];
   cluster_pw      = ((US_Hydrodyn_Cluster *) cluster_window )->cluster_config[ "userpw" ];

   manage_url      = ((US_Hydrodyn_Cluster *) cluster_window )->cluster_config[ "manage" ];
   manage_url_host = manage_url;
   manage_url_port = manage_url;

   manage_url_host.replace( QRegExp( ":.*$" ), "" );
   manage_url_port.replace( QRegExp( "^.*:" ), "" );
   cout << manage_url_host << endl;
   cout << manage_url_port << endl;

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
                                us_tr("US-SOMO: Cluster Submit"),
                                us_tr("Cluster credentials must be set in Cluster Config before continuing"),
                                0 );
      return;
   }

   if ( !update_files( false ) )
   {
      QMessageBox::information( this, 
                                us_tr("US-SOMO: Cluster Submit"),
                                us_tr("No unsubmitted jobs found"),
                                0 );
   }

   setupGUI();
   editor_msg("blue", "THIS WINDOW IS UNDER DEVELOPMENT." );

   submitted_dir = pkg_dir + SLASH + "submitted";
   QDir dir1( submitted_dir );
   if ( !dir1.exists() )
   {
      editor_msg( "black", QString( us_tr( "Created directory %1" ) ).arg( submitted_dir ) );
      dir1.mkdir( submitted_dir );
   }

   tmp_dir = pkg_dir + SLASH + "tmp";
   QDir dir2( tmp_dir );
   if ( !dir2.exists() )
   {
      editor_msg( "black", QString( us_tr( "Created directory %1" ) ).arg( tmp_dir ) );
      dir2.mkdir( tmp_dir );
   }

   update_files();
   lb_systems->item(0)->setSelected( true );
   update_enables();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 800, 600 );
}

US_Hydrodyn_Cluster_Submit::~US_Hydrodyn_Cluster_Submit()
{
}

unsigned int US_Hydrodyn_Cluster_Submit::update_files( bool set_lv_files )
{
   files.clear( );

   // traverse directory and build up files
   QDir::setCurrent( pkg_dir );
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
      all_files = (qs ).split( "\n" , Qt::SkipEmptyParts );
   }

   for ( unsigned int i = 0; i < ( unsigned int ) all_files.size(); i++ )
   {
      if ( !all_files[ i ].contains( "_out", Qt::CaseInsensitive ) )
      {
         files << all_files[ i ];
      }
   }
   
   if ( set_lv_files )
   {
      lv_files->clear( );
      for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
      {
         cout << "files: " << files[ i ] << endl;
         // qt3 QFileInfo::size() is incorrect uint is too small
         // so use fstat()

#if QT_VERSION < 0x040000
         new QTreeWidgetItem( lv_files, 
                            files[ i ], 
                            QString( " %1 " ).arg( QFileInfo( files[ i ] ).birthTime().toString() ),
                            QString( " %1 bytes " ).arg( QFileInfo( files[ i ] ).size() )
                            );
#else
         lv_files->addTopLevelItem( new QTreeWidgetItem(
                                                        QStringList()
                                                        << files[ i ]
                                                        << QString( " %1 " ).arg( QFileInfo( files[ i ] ).birthTime().toString() )
                                                        << QString( " %1 bytes " ).arg( QFileInfo( files[ i ] ).size() )
                                                        ) );
#endif
      }
   }

   return files.size();
}

void US_Hydrodyn_Cluster_Submit::setupGUI()
{
   int minHeight1 = 30;

   lbl_title = new QLabel( us_tr( "Submit jobs to cluster" ), this);
   lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette( PALET_FRAME );
   AUTFBACK( lbl_title );
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_files = new QLabel(us_tr("Available jobs:"), this);
   lbl_files->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_files->setMinimumHeight(minHeight1);
   lbl_files->setPalette( PALET_LABEL );
   AUTFBACK( lbl_files );
   lbl_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   lv_files = new QTreeWidget(this);
   lv_files->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lv_files->setMinimumHeight(minHeight1 * 3);
   lv_files->setPalette( PALET_EDIT );
   AUTFBACK( lv_files );
   lv_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   lv_files->setEnabled(true);
   lv_files->setSelectionMode( QAbstractItemView::MultiSelection );

   lv_files->setColumnCount( 4 );
   lv_files->setHeaderLabels( QStringList()
                              << us_tr( "Name" )
                              << us_tr( "Created" )
                              << us_tr( "Size" )
                              << us_tr( "Status" )
                              );

   lv_files->setColumnWidth( 0, 250 );
   lv_files->setColumnWidth( 1, 220 );
   lv_files->setColumnWidth( 2, 150 );
   lv_files->setColumnWidth( 3, 80 );

   connect( lv_files, SIGNAL( itemSelectionChanged() ), SLOT( update_enables() ) );

   lbl_systems = new QLabel(us_tr("Systems"), this);
   lbl_systems->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_systems->setMinimumHeight(minHeight1);
   lbl_systems->setPalette( PALET_LABEL );
   AUTFBACK( lbl_systems );
   lbl_systems->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   lb_systems = new QListWidget(this);
   lb_systems->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lb_systems->setMinimumHeight(minHeight1 * 
                                ( ((US_Hydrodyn_Cluster *)cluster_window)->cluster_systems.size() > 8 ?
                                  8 : ((US_Hydrodyn_Cluster *)cluster_window)->cluster_systems.size() ) );
   lb_systems->setPalette( PALET_EDIT );
   AUTFBACK( lb_systems );
   lb_systems->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   lb_systems->setEnabled(true);

   for ( map < QString, map < QString, QString > >::iterator it = 
            ((US_Hydrodyn_Cluster *)cluster_window)->cluster_systems.begin();
         it != ((US_Hydrodyn_Cluster *)cluster_window)->cluster_systems.end();
         it++ )
   {
      lb_systems->addItem( it->first );
   }


   lb_systems->setCurrentItem( lb_systems->item(0) );
   lb_systems->item(0)->setSelected( false );
   lb_systems->setSelectionMode( QAbstractItemView::SingleSelection );
   // lb_systems->setColumnMode( QListBox::FitToWidth );
   connect( lb_systems, SIGNAL( itemSelectionChanged() ), SLOT( systems() ) );
   lb_systems->hide();
   lbl_systems->hide();

   pb_select_all = new QPushButton(us_tr("Select all jobs"), this);
   pb_select_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_select_all->setMinimumHeight(minHeight1);
   pb_select_all->setPalette( PALET_PUSHB );
   connect(pb_select_all, SIGNAL(clicked()), SLOT(select_all()));

   pb_remove = new QPushButton(us_tr("Remove selected jobs"), this);
   pb_remove->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_remove->setMinimumHeight(minHeight1);
   pb_remove->setPalette( PALET_PUSHB );
   connect(pb_remove, SIGNAL(clicked()), SLOT( remove() ));

   pb_submit = new QPushButton(us_tr("Submit selected jobs"), this);
   pb_submit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_submit->setMinimumHeight(minHeight1);
   pb_submit->setPalette( PALET_PUSHB );
   connect(pb_submit, SIGNAL(clicked()), SLOT( submit()) );
   
   progress = new QProgressBar( this );
   progress->setMinimumHeight( minHeight1 );
   progress->setPalette( PALET_NORMAL );
   AUTFBACK( progress );
   progress->reset();
   progress->setRange( 0, 100 );

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
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // build layout

   QHBoxLayout * hbl_buttons1 = new QHBoxLayout(); hbl_buttons1->setContentsMargins( 0, 0, 0, 0 ); hbl_buttons1->setSpacing( 0 );
   hbl_buttons1->addSpacing( 4 );
   hbl_buttons1->addWidget ( pb_select_all);
   hbl_buttons1->addSpacing( 4 );
   hbl_buttons1->addWidget ( pb_remove );
   hbl_buttons1->addSpacing( 4 );
   hbl_buttons1->addWidget ( pb_submit );
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
   for ( int i = 0; i < lb_systems->count(); i++ )
   {
      if ( lb_systems->item(i)->isSelected() )
      {
         selected_system_name = lb_systems->item( i )->text();
         cout << "run config systems for " << selected_system_name << "\n";
         
         if ( ((US_Hydrodyn_Cluster *)cluster_window)->cluster_systems.count( selected_system_name ) )
         {
            selected_system = ((US_Hydrodyn_Cluster *)cluster_window)->cluster_systems[ selected_system_name ];
            if ( selected_system.count( "stage" ) &&
                 selected_system.count( "type" ) &&
                 selected_system.count( "corespernode" ) &&
                 selected_system.count( "maxcores" ) &&
                 selected_system.count( "queue" ) &&
                 selected_system.count( "runtime" ) )
            {
               stage_url       = selected_system[ "stage" ];
               // stage_path      = stage_url;
               // stage_url_path  = stage_url;
               stage_url       .replace( QRegExp( ":.*$" ), "" );
               // stage_path      .replace( QRegExp( "^.*:" ), "" );
               // stage_url_path  += QString( "%1%2%3" ).arg( QDir::separator() ).arg( cluster_id ).arg( QDir::separator() );
            } else {
               QMessageBox::warning( this, 
                                     us_tr( "US-SOMO: Cluster Submit" ), 
                                     QString( us_tr( "The system %1 does not seem to have sufficient configuration information defined" ) ).arg( lb_systems->item( i )->text() ) );
               lb_systems->clearSelection();
            } 
         } else {
            QMessageBox::warning( this, 
                                  us_tr( "US-SOMO: Cluster Submit" ), 
                                  QString( us_tr( "The system %1 does not seem to have any information" ) ).arg( lb_systems->item( i )->text() ) );
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
   online_help->show_help("manual/somo/somo_cluster_submit.html");
}

void US_Hydrodyn_Cluster_Submit::closeEvent(QCloseEvent *e)
{
   if ( comm_active )
   {
      http_reply->abort();
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


void US_Hydrodyn_Cluster_Submit::clear_display()
{
   editor->clear( );
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

void US_Hydrodyn_Cluster_Submit::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->textColor();
   editor->setTextColor(color);
   editor->append(msg);
   editor->setTextColor(save_color);
   editor->verticalScrollBar()->setValue(editor->verticalScrollBar()->maximum());
}
   
void US_Hydrodyn_Cluster_Submit::update_enables()
{
   bool any_systems = false;

   for ( int i = 0; i < lb_systems->count(); i++ )
   {
      if ( lb_systems->item( i )->isSelected() )
      {
         cout << lb_systems->item( i )->text() << endl;
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
      bool any_selected = US_Static::lv_any_selected( lv_files );
      pb_remove->setEnabled( any_selected );
      pb_submit->setEnabled( any_selected && any_systems );
   }
}

void US_Hydrodyn_Cluster_Submit::select_all()
{
   disable_updates = true;
   US_Static::lv_select_all_or_none( lv_files );
   disable_updates = false;
   update_enables();
}

bool US_Hydrodyn_Cluster_Submit::submit_url_body( QString file, QString &url, QString &body )
{
   errormsg = "";
   cout << "submit_url_body\n";
   if ( !QFile::exists( file ) )
   {
      errormsg = QString( us_tr( "submit_url_body: File %1 does not exist" ) ).arg( file );
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
         errormsg = QString( us_tr( "submit_url_body: Listing tar archive %1 failed %2" ) )
            .arg( file )
            .arg( ust.explain( result ) );
         return false;
      }
   }

   if ( !selected_system[ "corespernode" ].toUInt() )
   {
      errormsg = QString( us_tr( "submit_url_body: The selected system %1 does not have a positive cores per node defined" ) )
         .arg( selected_system_name );
      return false;
   }

   if ( !selected_system[ "maxcores" ].toUInt() )
   {
      errormsg = QString( us_tr( "submit_url_body: The selected system %1 does not have a positive max cores defined" ) )
         .arg( selected_system_name );
      return false;
   }

   if ( !selected_system[ "runtime" ].toUInt() )
   {
      errormsg = QString( us_tr( "submit_url_body: The selected system %1 does not have a positive run time defined" ) )
         .arg( selected_system_name );
      return false;
   }

   unsigned int common_count = ( unsigned int ) tar_list.filter( QRegExp( "^common_" ) ).size();


   unsigned int job_count = ( unsigned int ) tar_list.size() - common_count - 1;
   
   {
      QRegExp rx( "^(bfnb|bfnbpm|oned|best)_p(\\d+)_" );
      if ( rx.indexIn( file ) != -1 )
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

   if ( processor_count == 0 ) {
      processor_count = 1;
   }

   QString target_dir = QString( "%1" ).arg( next_to_process->text( 0 ) ).replace( QRegExp( "\\.(tar|tgz)$" ), "" );

   // url = QString( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
   //                "<Message>"
   //                "<Header>" );
   // url += QString( "<userdn>%1</userdn>" ).arg( cluster_id );


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

   // url += QString( 
   //                "<experimentid>%1</experimentid>"
   //                "<hostname>%2</hostname>"
   //                "<processorcount>%3</processorcount>"
   //                "%4"
   //                "<walltime>%5</walltime>"
   //                "<userdn>%6</userdn>"
   //                "%7"
   //                "</Header>"
   //                "<Body>"
   //                "<Method>run</Method>"
   //                "<input>"
   //                "<parameters>"
   //                "<name>param</name>"
   //                "<value>%8</value>"
   //                "</parameters>"
   //                "<parameters>"
   //                "<name>inputfile</name>"
   //                "<value>%9</value>"
   //                "</parameters>"
   //                "</input>"
   //                "</Body>"
   //                "</Message>"
   //                "\n" )

   //    .arg( QString( "%1-%2" ).arg( cluster_id ).arg( file ) )
   //    .arg( stage_url )
   //    .arg( processor_count )
   //    .arg( selected_system[ "queue" ].isEmpty() ? "" : QString( "<queuename>%1</queuename>" ).arg( selected_system[ "queue" ] ) )
   //    .arg( selected_system[ "runtime" ].toUInt() )
   //    .arg( cluster_id )
   //    .arg( selected_system[ "executable" ].isEmpty() ? "" : QString( "<executable>%1</executable>" ).arg( selected_system[ "executable" ] ) )
   //    .arg( job_type )
   //    .arg( QString( "%1/%2" )
   //          .arg( target_dir )
   //          .arg( file ) );

   // cout << url << endl;

   url =
      QString( "/jobsubmit?" )
      + QString( "menu=ussomo"         )
      + QString( "&module=somoapi_mpi" )
      + QString( "&_clobber=1"         )
      + QString( "&user=%1"            ).arg( cluster_id )
      + QString( "&pw=%1"              ).arg( cluster_pw )
      + QString( "&id=%1"              ).arg( QString( "%1-%2" ).arg( cluster_id ).arg( file ) )
      + QString( "&resource=%1"        ).arg( selected_system_name )
      + QString( "&host=%1"            ).arg( stage_url )
      + QString( "&np=%1"              ).arg( CLUSTER_OVERRIDE_NPROC ) // processor_count )
      + QString( "&queue=%1"           ).arg( selected_system[ "queue" ].isEmpty() ? "" : QString( "%1" ).arg( selected_system[ "queue" ] ) )
      + QString( "&wall=%1"            ).arg( selected_system[ "runtime" ].toUInt() )
      + QString( "&param=json"         )
      + QString( "&dir=%1"             ).arg( target_dir )
      + QString( "&file=%1"            ).arg( file )
      + QString( "&_uuid=%1"           ).arg( QString( "%1-%2" ).arg( cluster_id ).arg( file ) )
      ;
   
   cout << "submit url\n" << url << endl;

   // build json input
   {
      map < QString, QString > body_map;
      body_map[ "numproc" ] = QString( "%1" ).arg( CLUSTER_OVERRIDE_NPROC ); // processor_count );
      body_map[ "param"   ] = "json";
      body_map[ job_type  ] = "1";
      body_map[ "file"    ] = file;
      body = US_Json::compose( body_map );
   }

   cout << "submit body\n" << body << endl;

   if ( !submit_active )
   {
      errormsg = us_tr( "Stopped by user request" );
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
      http_reply->abort();
   }
   if ( system_proc_active )
   {
      system_proc->terminate();
      QTimer::singleShot( 2500, system_proc, SLOT( kill() ) );
   }
   update_enables();
}

void US_Hydrodyn_Cluster_Submit::submit()
{
   submit_active = true;
   update_enables();
   cout << "setup jobs\n";
   jobs.clear( );
   
   QStringList qsl_submit;
   QTreeWidgetItemIterator it( lv_files, QTreeWidgetItemIterator::Selected );
   QTreeWidgetItem *lvi;
   if ( *it ) {
      while ( *it ) {
         lvi = *it;
         // jobs[ lvi ] = "prepare stage";
         jobs[ lvi ] = "submit";
         ++it;
      }
      
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
   for ( map < QTreeWidgetItem *, QString >::iterator it = jobs.begin();
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
      editor_msg( "black", us_tr( "submission complete" ) );
   } else {
      editor_msg( "red", us_tr( "submission had errors" ) );
   }
      
   submit_active = false;

   if ( !update_files() )
   {
      QMessageBox::information( this, 
                                us_tr("US-SOMO: Cluster Jobs"),
                                us_tr("No further unsubmitted jobs found"),
                                0 );
      close();
   }

   update_enables();
}

void US_Hydrodyn_Cluster_Submit::process_prepare_stage()
{
   editor_msg( "black", QString( "preparing stage %1" ).arg( next_to_process->text( 0 ) ) );
   next_to_process->setText( 3, us_tr( "Preparing to stage" ) );
   cout << "process prepare stage\n";
   if ( prepare_stage( next_to_process->text( 0 ) ) )
   {
      jobs[ next_to_process ] = "stage";
   } else {
      editor_msg( "red", errormsg );
      next_to_process->setText( 3, us_tr( "Error: Prepare to stage failed" ) );
      jobs[ next_to_process ] = "prepare stage failed";
   }
   emit process_next();
}

void US_Hydrodyn_Cluster_Submit::process_stage()
{
   editor_msg( "black", QString( "staging %1" ).arg( next_to_process->text( 0 ) ) );
   next_to_process->setText( 3, us_tr( "Staging" ) );
   cout << "process stage\n";
   if ( stage( next_to_process->text( 0 ) ) )
   {
      jobs[ next_to_process ] = "submit";
   } else {
      editor_msg( "red", errormsg );
      next_to_process->setText( 3, us_tr( "Error: Staging failed" ) );
      jobs[ next_to_process ] = "stage failed";
   }
   emit process_next();
}

void US_Hydrodyn_Cluster_Submit::process_submit()
{
   editor_msg( "black", QString( "submitting %1" ).arg( next_to_process->text( 0 ) ) );
   next_to_process->setText( 3, us_tr( "Submitting" ) );
   cout << "process submit\n";
   if ( job_submit( next_to_process->text( 0 ) ) )
   {
      jobs[ next_to_process ] = "move";
   } else {
      editor_msg( "red", errormsg );
      next_to_process->setText( 3, us_tr( "Error: Submit failed" ) );
      jobs[ next_to_process ] = "submit failed";
   }
   emit process_next();
}

void US_Hydrodyn_Cluster_Submit::process_move()
{
   next_to_process->setText( 3, us_tr( "Moving to submitted" ) );
   editor_msg( "black", QString( "move %1 to submitted/" ).arg( next_to_process->text( 0 ) ) );
   cout << "process move\n";
   if ( move_file( next_to_process->text( 0 ) ) )
   {
      next_to_process->setText( 3, us_tr( "Moved to submitted" ) );
      jobs[ next_to_process ] = "complete";
   } else {
      editor_msg( "red", errormsg );
      next_to_process->setText( 3, us_tr( "Error: Moving to submitted failed" ) );
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
      errormsg = QString( us_tr( "move: can not change to directory %1" ) ).arg( pkg_dir );
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
   for ( map < QTreeWidgetItem *, QString >::iterator it = jobs.begin();
         it != jobs.end();
         it++ )
   {
      editor_msg( "blue", QString("%1 %2").arg( it->first->text( 0 ) ).arg( it->second ) );
   }
}

bool US_Hydrodyn_Cluster_Submit::job_submit( QString file )
{
   QString url;
   QString body;
   if ( !submit_url_body( file, url, body ) )
   { 
      return false;
   }

   // start comm process
   
   return send_http_post( file, url, body );
}

bool US_Hydrodyn_Cluster_Submit::system_cmd( QStringList cmd )
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

void US_Hydrodyn_Cluster_Submit::system_proc_readFromStdout()
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
   
void US_Hydrodyn_Cluster_Submit::system_proc_readFromStderr()
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
   
void US_Hydrodyn_Cluster_Submit::system_proc_finished( int, QProcess::ExitStatus )
{
   cout << "system_proc exit\n";
   system_proc_readFromStderr();
   system_proc_readFromStdout();

   disconnect( system_proc, SIGNAL(readyReadStandardOutput()), 0, 0);
   disconnect( system_proc, SIGNAL(readyReadStandardError()), 0, 0);
   disconnect( system_proc, SIGNAL(finished( int, QProcess::ExitStatus )), 0, 0);

   // editor->append("System_Proc finished.");
   system_proc_active = false;
   delete system_proc;
   emit process_next();
}
   
void US_Hydrodyn_Cluster_Submit::system_proc_started()
{
   editor_msg("brown", "System_Proc launch exited");
   disconnect( system_proc, SIGNAL(started()), 0, 0);
}

bool US_Hydrodyn_Cluster_Submit::prepare_stage( QString file )
{
   errormsg = "";
   if ( !QDir::setCurrent( pkg_dir ) )
   {
      errormsg = QString( us_tr( "stage: can not change to directory %1" ) ).arg( pkg_dir );
      return false;
   }

   if ( !QFile::exists( file ) )
   {
      errormsg = QString( us_tr( "stage: can not find file %1" ) ).arg( file );
      return false;
   }

//    // using qftp now
//    // need new method to clear dir
//    // probably login, if directory exists, change,
//    // list all and remove all
//    // will ftp.rm( "*" ) work?
//    // followed by ftp.rmdir();
//    comm_active = true;

//    QString target_dir = QString( "%1" ).arg( next_to_process->text( 0 ) ).replace( QRegExp( "\\.(tar|tgz)$" ), "" );

//    cout 
//       << QString( "ftp host   : %1\n"
//                   "ftp port   : %2\n"
//                   "target_dir : %3\n"
//                   "file       : %4\n" 
//                   "cluser_id  : %5\n" 
//                   "cluster_pw : %6\n" 
//                   )
//       .arg( ftp_url_host )
//       .arg( ftp_url_port.toUInt() )
//       .arg( target_dir )
//       .arg( file )
//       .arg( cluster_id )
//       .arg( cluster_pw );
   
// #if QT_VERSION < 0x050000
//    ftp.connectToHost( ftp_url_host, ftp_url_port.toUInt() );
//    ftp.login        ( cluster_id  , cluster_pw   );
//    ftp.mkdir        ( target_dir );
//    ftp.close        ();
// #endif
   return true;
}



bool US_Hydrodyn_Cluster_Submit::stage( QString file )
{
   errormsg = "";
   if ( !QDir::setCurrent( pkg_dir ) )
   {
      errormsg = QString( us_tr( "stage: can not change to directory %1" ) ).arg( pkg_dir );
      return false;
   }

   if ( !QFile::exists( file ) )
   {
      errormsg = QString( us_tr( "stage: can not find file %1" ) ).arg( file );
      return false;
   }


//    ftp_file = new QFile( file );
//    if ( !ftp_file->open( QIODevice::ReadOnly ) )
//    {
//       errormsg = QString( us_tr( "stage: can not open file %1" ) ).arg( file );
//       delete ftp_file;
//       ftp_file = ( QFile * ) 0;
//       return false;
//    }

//    // setup stuff for ftp
//    comm_active = true;

//    QString target_dir = QString( "%1" ).arg( next_to_process->text( 0 ) ).replace( QRegExp( "\\.(tar|tgz)$" ), "" );

//    cout 
//       << QString( "ftp host   : %1\n"
//                   "ftp port   : %2\n"
//                   "target_dir : %3\n"
//                   "file       : %4\n" 
//                   "cluser_id  : %5\n" 
//                   "cluster_pw : %6\n" 
//                   )
//       .arg( ftp_url_host )
//       .arg( ftp_url_port.toUInt() )
//       .arg( target_dir )
//       .arg( file )
//       .arg( cluster_id )
//       .arg( cluster_pw );
   
// #if QT_VERSION < 0x050000
//    ftp.connectToHost( ftp_url_host, ftp_url_port.toUInt() );
//    ftp.login        ( cluster_id  , cluster_pw   );
//    ftp.cd           ( target_dir );
//    ftp.put          ( ftp_file    , file         );
//    ftp.close        ();
// #endif

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
   bool any_selected = US_Static::lv_any_selected( lv_files );

   if ( !any_selected )
   {
      return;
   }

   if ( QMessageBox::question(
                              this,
                              us_tr("US-SOMO: Cluster Jobs: Remove"),
                              us_tr( "The jobs will be permenantly removed.\n"
                                  "Are you sure? " ),
                              us_tr("&Yes"), us_tr("&No"),
                              QString(), 0, 1 ) )
   {
      editor_msg( "black", us_tr( "Remove canceled by user" ) );
      return;
   }

   if ( !QDir::setCurrent( pkg_dir ) )
   {
      editor_msg( "red" , QString( us_tr( "can not change to directory %1" ) ).arg( pkg_dir ) );
      return;
   }

#if QT_VERSION < 0x040000
   QTreeWidgetItem *lvi = lv_files->firstChild();

   if ( lvi )
   {
      do {
         if ( lvi->isSelected() )
         {
            if ( !QFile::remove( lvi->text( 0 ) ) )
            {
               editor_msg( "red" , QString( us_tr( "can not remove file %1" ) ).arg( lvi->text( 0 ) ) );
            } else {
               editor_msg( "black" , QString( us_tr( "Removed file: %1" ) ).arg( lvi->text( 0 ) ) );
            }
         }
      } while ( ( lvi = lvi->nextSibling() ) );
   }
#else
   QTreeWidgetItemIterator it( lv_files, QTreeWidgetItemIterator::Selected );
   QTreeWidgetItem *lvi;
   while ( *it ) {
      lvi = *it;
      if ( !QFile::remove( lvi->text( 0 ) ) )
      {
         editor_msg( "red" , QString( us_tr( "can not remove file %1" ) ).arg( lvi->text( 0 ) ) );
      } else {
         editor_msg( "black" , QString( us_tr( "Removed file: %1" ) ).arg( lvi->text( 0 ) ) );
      }
      ++it;
   }
#endif
   if ( !update_files() )
   {
      QMessageBox::information( this, 
                                us_tr("US-SOMO: Cluster Jobs"),
                                us_tr("No further unsubmitted jobs found"),
                                0 );
      close();
   }
}

bool US_Hydrodyn_Cluster_Submit::send_http_post( QString file, QString url, QString body )
{
   // need to do a post & get to manage_url slash stuff
   // its going to require opening a socket etc
   // 
   // 1st read file into into qba,
   // if not, return false, etc.

   errormsg = "";
   if ( !QDir::setCurrent( pkg_dir ) )
   {
      errormsg = QString( us_tr( "send_http_post: can not change to directory %1" ) ).arg( pkg_dir );
      return false;
   }

   if ( !QFile::exists( file ) )
   {
      errormsg = QString( us_tr( "send_http_post: can not find file %1" ) ).arg( file );
      return false;
   }


   // QFile f( file );
   // if ( !f.open( QIODevice::ReadOnly ) ) {
   //    errormsg = QString( us_tr( "send_http_post: can not open file %1 for reading" ) ).arg( file );
   //    return false;
   // }
   // QByteArray qba = f.readAll();
   // f.close();
   
   http_multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

   // add the body

   {
      QHttpPart textPart;
      textPart.setHeader( QNetworkRequest::ContentTypeHeader, QVariant( "text/plain" ) );
      textPart.setHeader( QNetworkRequest::ContentDispositionHeader, QVariant( "form-data; name=\"text\"" ) );
      textPart.setBody( body.toUtf8() );
      http_multiPart->append( textPart );
   }
      
   // add the data (file)

   {
      QHttpPart dataPart;
      dataPart.setHeader( QNetworkRequest::ContentTypeHeader, QVariant( "application/octet-stream" ) );
      dataPart.setHeader( QNetworkRequest::ContentDispositionHeader, QVariant( QString( "form-data; name=\"%1\"" ).arg( file ) ) );
      // dataPart.setHeader( QNetworkRequest::ContentDispositionHeader, QVariant( QString( "form-data; name=\"datafile\"" ) ) );
      QFile *f = new QFile( file );
      if ( !f->open(QIODevice::ReadOnly) ) {
         delete http_multiPart;
         errormsg = QString( us_tr( "send_http_post: can not open file %1 for reading" ) ).arg( file );
         return false;
      }
      
      dataPart.setBodyDevice( f );
      f->setParent( http_multiPart ); // we cannot delete the file now, so delete it with the multiPart

      // !!!!!!! ******** need to make sure no memory leaks, i.e. close file etc

      http_multiPart->append( dataPart );
   }

   comm_active = true;
   cout << "http_get\n";
   // editor_msg( "black", "updating status" );

   current_http_response = "";

   http_request.setUrl( QUrl( QString( "http://%1%2" ).arg( manage_url ).arg( url ) ) );
   http_reply = http_access_manager->post( http_request, http_multiPart );

   connect( http_reply, SIGNAL( downloadProgress ( qint64, qint64 ) ), this, SLOT( http_downloadProgress( qint64, qint64 ) ) );
   connect( http_reply, SIGNAL( uploadProgress ( qint64, qint64 ) ), this, SLOT( http_uploadProgress( qint64, qint64 ) ) );
   connect( http_reply, SIGNAL( finished () ), this, SLOT( http_finished() ) );
   connect( http_reply, SIGNAL( error ( QNetworkReply::NetworkError ) ), this, SLOT( http_error ( QNetworkReply::NetworkError ) ) ); 

   return true;
}

void US_Hydrodyn_Cluster_Submit::http_error( QNetworkReply::NetworkError /* code */ ) {
   qDebug() << "http: error";
   current_http_error = http_reply->errorString();
   http_done( true );
}

void US_Hydrodyn_Cluster_Submit::http_finished() {
   cout << "http: finished\n";
   current_http_response = QString( http_reply->readAll() );
   cout << current_http_response << endl;
   
   {
      map < QString, QString > readJson = US_Json::split( current_http_response );
      if ( readJson.count( "error" ) ) {
         current_http_error = readJson[ "error" ];
         http_done( true );
      }
      if ( readJson.count( "status" ) ) {
         next_to_process->setText( 1, readJson[ "status" ] );
      }
      if ( readJson.count( "message" ) ) {
         next_to_process->setText( 2, readJson[ "message" ] );
      }
   }

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
   // current_http_error = "not fully implemented in ussomo";
   // http_done( true );
   http_done( false );
}

void US_Hydrodyn_Cluster_Submit::http_uploadProgress ( qint64 done, qint64 total )
{
   cout << "http: uploadProgress " << done << " " << total << "\n";
   progress->setValue( 100.0 * (double) done / (double) total );
}

void US_Hydrodyn_Cluster_Submit::http_downloadProgress ( qint64 done, qint64 total )
{
   cout << "http: downloadProgress " << done << " " << total << "\n";
}

void US_Hydrodyn_Cluster_Submit::http_done ( bool error )
{
   cout << "http: done " << error << "\n";
   disconnect( http_reply, SIGNAL( downloadProgress ( qint64, qint64 ) ), 0, 0 );
   disconnect( http_reply, SIGNAL( uploadProgress ( qint64, qint64 ) ), 0, 0 );
   disconnect( http_reply, SIGNAL( finished () ), 0, 0 );
   disconnect( http_reply, SIGNAL( error ( QNetworkReply::NetworkError ) ), 0, 0 );
   comm_active = false;
   http_reply->deleteLater();
   http_multiPart->deleteLater();
   if ( error )
   {
      cout << current_http_error << endl;
      QMessageBox::warning( this,
                            us_tr("US-SOMO: Cluster Submit"), 
                            us_tr( QString( "There was a error with the management server:\n%1" )
                                .arg( current_http_error ) ),
                            QMessageBox::Ok,
                            QMessageBox::NoButton
                            );
      submit_active = false;
      editor_msg( "red", us_tr( "Error: " ) + current_http_error );
      update_enables();
      return;
   }
   emit process_next();
}
