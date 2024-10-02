#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_cluster.h"
#include "../include/us_hydrodyn_cluster_config.h"
#include "../include/us_hydrodyn_cluster_config_server.h"
#include "../include/us_json.h"
//Added by qt3to4:
#include <QHBoxLayout>
#include <QLabel>
#include <QGridLayout>
#include <QFrame>
#include <QVBoxLayout>
#include <QCloseEvent>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#define SLASH QDir::separator()

US_Hydrodyn_Cluster_Config::US_Hydrodyn_Cluster_Config(
                                         void *us_hydrodyn, 
                                         QWidget *p, 
                                         const char *
                                         ) : QDialog( p )
{
   this->us_hydrodyn = us_hydrodyn;
   setWindowTitle(us_tr("US-SOMO: Cluster Config"));
   USglobal = new US_Config();
   cluster_window = (void *)p;
   check_tried = false;
   comm_active = false;

   cluster_systems = ((US_Hydrodyn_Cluster *)cluster_window)->cluster_systems;
   QString pkg_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "cluster";
   QDir::setCurrent( pkg_dir );

   http_access_manager = new QNetworkAccessManager( this );

   setupGUI();
   update_enables();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );

   pb_edit         ->setEnabled( false );
   pb_delete_system->setEnabled( false );
}

US_Hydrodyn_Cluster_Config::~US_Hydrodyn_Cluster_Config()
{
}

void US_Hydrodyn_Cluster_Config::setupGUI()
{
   int minHeight1 = 30;

   lbl_title = new QLabel( us_tr( "Cluster configuration" ), this);
   lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette( PALET_FRAME );
   AUTFBACK( lbl_title );
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_cluster_id = new QLabel(us_tr("Assigned cluster id"), this);
   lbl_cluster_id->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_cluster_id->setMinimumHeight(minHeight1);
   lbl_cluster_id->setPalette( PALET_LABEL );
   AUTFBACK( lbl_cluster_id );
   lbl_cluster_id->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   le_cluster_id = new QLineEdit( this );    le_cluster_id->setObjectName( "csv_filename Line Edit" );
   le_cluster_id->setText( ((US_Hydrodyn_Cluster *)cluster_window)->cluster_config[ "userid" ] );
   le_cluster_id->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_cluster_id->setMinimumWidth(250);
   le_cluster_id->setPalette( PALET_NORMAL );
   AUTFBACK( le_cluster_id );
   le_cluster_id->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   connect( le_cluster_id, SIGNAL( textChanged( const QString & ) ), SLOT( update_cluster_id(const QString & ) ) );

   lbl_cluster_pw = new QLabel(us_tr("Password"), this);
   lbl_cluster_pw->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_cluster_pw->setMinimumHeight(minHeight1);
   lbl_cluster_pw->setPalette( PALET_LABEL );
   AUTFBACK( lbl_cluster_pw );
   lbl_cluster_pw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   le_cluster_pw = new QLineEdit( this );    le_cluster_pw->setObjectName( "csv_filename Line Edit" );
   le_cluster_pw->setText( ((US_Hydrodyn_Cluster *)cluster_window)->cluster_config[ "userpw" ] );
   le_cluster_pw->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_cluster_pw->setMinimumWidth(250);
   le_cluster_pw->setPalette( PALET_NORMAL );
   AUTFBACK( le_cluster_pw );
   le_cluster_pw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_cluster_pw->setEchoMode( QLineEdit::Password );
   connect( le_cluster_pw, SIGNAL( textChanged( const QString & ) ), SLOT( update_cluster_pw( const QString & ) ) );

   lbl_cluster_pw2 = new QLabel(us_tr("Repeat password"), this);
   lbl_cluster_pw2->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_cluster_pw2->setMinimumHeight(minHeight1);
   lbl_cluster_pw2->setPalette( PALET_LABEL );
   AUTFBACK( lbl_cluster_pw2 );
   lbl_cluster_pw2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   le_cluster_pw2 = new QLineEdit( this );    le_cluster_pw2->setObjectName( "csv_filename Line Edit" );
   le_cluster_pw2->setText( ((US_Hydrodyn_Cluster *)cluster_window)->cluster_config[ "userpw" ] );
   le_cluster_pw2->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_cluster_pw2->setMinimumWidth(250);
   le_cluster_pw2->setPalette( PALET_NORMAL );
   AUTFBACK( le_cluster_pw2 );
   le_cluster_pw2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_cluster_pw2->setEchoMode( QLineEdit::Password );
   connect( le_cluster_pw2, SIGNAL( textChanged( const QString & ) ), SLOT( update_cluster_pw2( const QString & ) ) );

   lbl_cluster_email = new QLabel(us_tr("User email address"), this);
   lbl_cluster_email->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_cluster_email->setMinimumHeight(minHeight1);
   lbl_cluster_email->setPalette( PALET_LABEL );
   AUTFBACK( lbl_cluster_email );
   lbl_cluster_email->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   le_cluster_email = new QLineEdit( this );    le_cluster_email->setObjectName( "csv_filename Line Edit" );
   le_cluster_email->setText( ((US_Hydrodyn_Cluster *)cluster_window)->cluster_config[ "useremail" ] );
   le_cluster_email->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_cluster_email->setMinimumWidth(250);
   le_cluster_email->setPalette( PALET_NORMAL );
   AUTFBACK( le_cluster_email );
   le_cluster_email->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   connect( le_cluster_email, SIGNAL( textChanged( const QString & ) ), SLOT( update_cluster_email( const QString & ) ) );

   lbl_submit_url = new QLabel(us_tr("Job submission URL"), this);
   lbl_submit_url->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_submit_url->setMinimumHeight(minHeight1);
   lbl_submit_url->setPalette( PALET_LABEL );
   AUTFBACK( lbl_submit_url );
   lbl_submit_url->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   le_submit_url = new QLineEdit( this );    le_submit_url->setObjectName( "csv_filename Line Edit" );
   le_submit_url->setText( ((US_Hydrodyn_Cluster *)cluster_window)->cluster_config[ "server" ] );
   le_submit_url->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_submit_url->setMinimumWidth(250);
   le_submit_url->setPalette( PALET_NORMAL );
   AUTFBACK( le_submit_url );
   le_submit_url->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   lbl_submit_url->hide();
   le_submit_url->hide();

   lbl_manage_url = new QLabel(us_tr("Job management URL"), this);
   lbl_manage_url->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_manage_url->setMinimumHeight(minHeight1);
   lbl_manage_url->setPalette( PALET_LABEL );
   AUTFBACK( lbl_manage_url );
   lbl_manage_url->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   le_manage_url = new QLineEdit( this );    le_manage_url->setObjectName( "csv_filename Line Edit" );
   le_manage_url->setText( ((US_Hydrodyn_Cluster *)cluster_window)->cluster_config[ "manage" ] );
   le_manage_url->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_manage_url->setMinimumWidth(250);
   le_manage_url->setPalette( PALET_NORMAL );
   AUTFBACK( le_manage_url );
   le_manage_url->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   connect( le_manage_url, SIGNAL( textChanged( const QString & ) ), SLOT( update_manage_url( const QString & ) ) );

   pb_check_user = new QPushButton(us_tr("Check user status"), this);
   pb_check_user->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_check_user->setMinimumHeight(minHeight1);
   pb_check_user->setPalette( PALET_PUSHB );
   connect(pb_check_user, SIGNAL(clicked()), SLOT(check_user()));

   pb_add_user = new QPushButton(us_tr("Add new user"), this);
   pb_add_user->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_add_user->setMinimumHeight(minHeight1);
   pb_add_user->setPalette( PALET_PUSHB );
   connect(pb_add_user, SIGNAL(clicked()), SLOT(add_user()));
   pb_add_user->setEnabled( false );

   pb_reset = new QPushButton(us_tr("Reset"), this);
   pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_reset->setMinimumHeight(minHeight1);
   pb_reset->setPalette( PALET_PUSHB );
   connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));

   lbl_systems = new QLabel(us_tr("Systems"), this);
   lbl_systems->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_systems->setMinimumHeight(minHeight1);
   lbl_systems->setPalette( PALET_LABEL );
   AUTFBACK( lbl_systems );
   lbl_systems->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   lb_systems = new QListWidget(this);
   lb_systems->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lb_systems->setMinimumHeight(minHeight1 * 10);
   lb_systems->setMinimumWidth( 500 );
   lb_systems->setPalette( PALET_EDIT );
   AUTFBACK( lb_systems );
   lb_systems->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   lb_systems->setEnabled(true);

   for ( map < QString, map < QString, QString > >::iterator it = cluster_systems.begin();
         it != cluster_systems.end();
         it++ )
   {
      lb_systems->addItem( it->first );
   }

   lb_systems->setCurrentItem( lb_systems->item(0) );
   lb_systems->item(0)->setSelected( false);
   lb_systems->setSelectionMode( QAbstractItemView::SingleSelection );
   connect( lb_systems, SIGNAL( itemSelectionChanged() ), SLOT( systems() ) );

   pb_add_new = new QPushButton(us_tr("Add new system"), this);
   pb_add_new->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_add_new->setMinimumHeight(minHeight1);
   pb_add_new->setPalette( PALET_PUSHB );
   connect(pb_add_new, SIGNAL(clicked()), SLOT(add_new()));

   pb_edit = new QPushButton(us_tr("Edit system details"), this);
   pb_edit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_edit->setMinimumHeight(minHeight1);
   pb_edit->setPalette( PALET_PUSHB );
   connect(pb_edit, SIGNAL(clicked()), SLOT(edit()));

   pb_delete_system = new QPushButton(us_tr("Delete system"), this);
   pb_delete_system->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_delete_system->setMinimumHeight(minHeight1);
   pb_delete_system->setPalette( PALET_PUSHB );
   connect(pb_delete_system, SIGNAL(clicked()), SLOT(delete_system()));

   lbl_systems      ->hide();
   lb_systems       ->hide();
   pb_add_new       ->hide();
   pb_edit          ->hide();
   pb_delete_system ->hide();
   
   pb_cancel = new QPushButton(us_tr("Cancel"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(us_tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_save_config = new QPushButton(us_tr("Close"), this);
   pb_save_config->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_save_config->setMinimumHeight(minHeight1);
   pb_save_config->setPalette( PALET_PUSHB );
   connect(pb_save_config, SIGNAL(clicked()), SLOT(save_config()));

   // build layout

   QGridLayout * gl_grid = new QGridLayout( 0 ); gl_grid->setContentsMargins( 0, 0, 0, 0 ); gl_grid->setSpacing( 0 );

   int j = 0;
   gl_grid->addWidget( lbl_cluster_id      , j, 0 );
   gl_grid->addWidget( le_cluster_id       , j, 1 );
   j++;
   gl_grid->addWidget( lbl_cluster_pw      , j, 0 );
   gl_grid->addWidget( le_cluster_pw       , j, 1 );
   j++;
   gl_grid->addWidget( lbl_cluster_pw2     , j, 0 );
   gl_grid->addWidget( le_cluster_pw2      , j, 1 );
   j++;
   gl_grid->addWidget( lbl_cluster_email   , j, 0 );
   gl_grid->addWidget( le_cluster_email    , j, 1 );
   j++;
   gl_grid->addWidget( lbl_submit_url      , j, 0 );
   gl_grid->addWidget( le_submit_url       , j, 1 );
   j++;
   gl_grid->addWidget( lbl_manage_url      , j, 0 );
   gl_grid->addWidget( le_manage_url       , j, 1 );
   j++;

   QHBoxLayout * hbl_check_add_user = new QHBoxLayout(); hbl_check_add_user->setContentsMargins( 0, 0, 0, 0 ); hbl_check_add_user->setSpacing( 0 );
   hbl_check_add_user->addWidget( pb_check_user );
   hbl_check_add_user->addWidget( pb_add_user );
   hbl_check_add_user->addWidget( pb_reset );

   QHBoxLayout * hbl_add_del = new QHBoxLayout(); hbl_add_del->setContentsMargins( 0, 0, 0, 0 ); hbl_add_del->setSpacing( 0 );
   hbl_add_del->addSpacing( 4 );
   hbl_add_del->addWidget ( pb_add_new );
   hbl_add_del->addSpacing( 4 );
   hbl_add_del->addWidget ( pb_edit );
   hbl_add_del->addSpacing( 4 );
   hbl_add_del->addWidget ( pb_delete_system );
   hbl_add_del->addSpacing( 4 );

   QHBoxLayout * hbl_bottom = new QHBoxLayout(); hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_cancel );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_save_config );
   hbl_bottom->addSpacing( 4 );

   QVBoxLayout * background = new QVBoxLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );
   background->addSpacing( 4 );
   background->addWidget ( lbl_title );
   background->addSpacing( 4 );
   background->addLayout ( gl_grid );
   background->addSpacing( 4 );
   background->addLayout ( hbl_check_add_user );
   background->addSpacing( 4 );
   background->addWidget ( lbl_systems );
   background->addWidget ( lb_systems );
   background->addSpacing( 4 );
   background->addLayout ( hbl_add_del );
   background->addSpacing( 4 );
   background->addLayout ( hbl_bottom );
   background->addSpacing( 4 );
}

void US_Hydrodyn_Cluster_Config::cancel()
{
   if ( comm_active )
   {
      http_reply->abort();
   }
   close();
}

void US_Hydrodyn_Cluster_Config::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo/somo_cluster_config.html");
}

void US_Hydrodyn_Cluster_Config::closeEvent(QCloseEvent *e)
{
   if ( comm_active )
   {
      http_reply->abort();
   }
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Cluster_Config::save_config()
{
   // check for  stage to system inconsistancies
   // i.e. systems with identical staging domain names's 
   // have different staging directories

   map < QString, QString > domain_to_dir;

   for ( map < QString, map < QString, QString > >::iterator it = cluster_systems.begin();
         it != cluster_systems.end();
         it++ )
   {
      if ( it->second.count( "stage" ) )
      {
         QString domain = it->second[ "stage" ];
         QString dir    = domain;
         domain.replace( QRegExp( ":.*$" ), "" );
         dir   .replace( QRegExp( "^.*:" ), "" );
         if ( domain_to_dir.count( domain ) &&
              domain_to_dir[ domain ] != dir )
         {
            QMessageBox::warning( this, 
                                  lbl_title->text(),
                                  QString( us_tr( "Staging locations for duplicate physical servers must be identical\n"
                                               "There is an inconsistancy with stage '%1' found with server '%2'\n"
                                                "Can not save unless this is corrected\n" ) )
                                  .arg( it->second[ "stage" ] )
                                  .arg( it->first ) );
            return;
         } else {
            domain_to_dir[ domain ] = dir;
         }
      }
   }

   ((US_Hydrodyn_Cluster *)cluster_window)->cluster_config[ "userid"     ] = le_cluster_id->text();
   ((US_Hydrodyn_Cluster *)cluster_window)->cluster_config[ "userpw"     ] = le_cluster_pw->text();
   ((US_Hydrodyn_Cluster *)cluster_window)->cluster_config[ "useremail"  ] = le_cluster_email->text();
   ((US_Hydrodyn_Cluster *)cluster_window)->cluster_config[ "server"     ] = le_submit_url->text();
   ((US_Hydrodyn_Cluster *)cluster_window)->cluster_config[ "manage"     ] = le_manage_url->text();
   ((US_Hydrodyn_Cluster *)cluster_window)->cluster_systems = cluster_systems;

   ((US_Hydrodyn_Cluster *)cluster_window)->cluster_stage_to_system.clear( );

   for ( map < QString, map < QString, QString > >::iterator it = cluster_systems.begin();
         it != cluster_systems.end();
         it++ )
   {
      if ( it->second.count( "stage" ) )
      {
         QString system = it->second[ "stage" ];
         system.replace( QRegExp( ":.*$" ), "" );
         ((US_Hydrodyn_Cluster *)cluster_window)->cluster_stage_to_system[ system ] = it->first;
      }
   }

   if ( !((US_Hydrodyn_Cluster *)cluster_window)->write_config() )
   {
      QMessageBox::warning( this, 
                            lbl_title->text(),
                            us_tr( ((US_Hydrodyn_Cluster *)cluster_window)->errormsg ) );
   } else {
      cout << ((US_Hydrodyn_Cluster *)cluster_window)->list_config();
      close();
   }
}

void US_Hydrodyn_Cluster_Config::systems()
{
   bool any_selected = false;
   for ( int i = 0; i < lb_systems->count(); i++ )
   {
      if ( lb_systems->item(i)->isSelected() )
      {
         any_selected = true;
         break;
      }
   }

   pb_edit         ->setEnabled( any_selected );
   pb_delete_system->setEnabled( any_selected );

}

void US_Hydrodyn_Cluster_Config::add_new()
{
   bool ok;
   QString text = US_Static::getText(
                                        us_tr( "US-SOMO: Cluster Config: Add new system" ),
                                        us_tr( "Enter the unique system name:" ), 
                                        QLineEdit::Normal,
                                        QString(), 
                                        &ok, 
                                        this );
   if ( !ok )
   {
      return;
   }

   // check to see if it already exists
   if ( cluster_systems.count( text ) )
   {
      QMessageBox::warning( this, 
                            us_tr( "US-SOMO: Cluster config: Add new" ),
                            QString( us_tr( "The system %1 already exists" ) ).arg( text ) );
      return;
   }
    
   bool any_selected = false;
   unsigned int pos;
   for ( int i = 0; i < lb_systems->count(); i++ )
   {
      if ( lb_systems->item(i)->isSelected() )
      {
         pos = i;
         any_selected = true;
         break;
      }
   }

   bool do_blank = true;
   if ( any_selected && 
        cluster_systems.count( lb_systems->item( pos )->text() ) )
   {
      switch ( QMessageBox::question(
                                     this,
                                     us_tr( "US-SOMO: Cluster config: Add new" ),
                                     QString(
                                             us_tr( "Do you want to copy system data from the selected system '%1?'" )
                                             ).arg( lb_systems->item( pos )->text() ),
                                     us_tr( "&Yes" ),
                                     us_tr( "&No" ),
                                     QString(),
                                     0,
                                     1
                                     ) )
      {
      case 0 : 
         cluster_systems[ text ] = cluster_systems[ lb_systems->item( pos )->text() ];
         do_blank = false;
         break;

      case 1 : 
         do_blank = true;
         break;
      }
   } 
      
   if ( do_blank )
   {
      // setup a blank system
      map < QString, QString > tmp_system;
      QStringList params;
      params 
         << "corespernode"
         << "maxcores"
         << "maxruntime"
         << "runtime"
         << "stage"
         << "ftp"
         << "type"
         << "executable"
         << "queue";
      
      for ( unsigned int i = 0; i < (unsigned int) params.size(); i++ )
      {
         tmp_system[ params[ i ] ] = "";
      }
      cluster_systems[ text ] = tmp_system;
   }

   lb_systems->clear( );
   
   for ( map < QString, map < QString, QString > >::iterator it = cluster_systems.begin();
         it != cluster_systems.end();
         it++ )
   {
      lb_systems->addItem( it->first );
   }
}

void US_Hydrodyn_Cluster_Config::delete_system()
{
   for ( int i = 0; i < lb_systems->count(); i++ )
   {
      if ( lb_systems->item(i)->isSelected() )
      {
         if ( cluster_systems.count( lb_systems->item( i )->text() ) )
         {
            cluster_systems.erase( lb_systems->item( i )->text() );
            lb_systems->clear( );
            
            for ( map < QString, map < QString, QString > >::iterator it = cluster_systems.begin();
                  it != cluster_systems.end();
                  it++ )
            {
               lb_systems->addItem( it->first );
            }
         } else {
            QMessageBox::warning( this, 
                                  us_tr( "US-SOMO: Cluster config" ), 
                                  QString( us_tr( "Internal error: The system %1 does not seem to exist?" ) ).arg( lb_systems->item( i )->text() ) );
         }
      }
   }
   systems();
}

void US_Hydrodyn_Cluster_Config::edit()
{
   for ( int i = 0; i < lb_systems->count(); i++ )
   {
      if ( lb_systems->item(i)->isSelected() )
      {
         cout << "run config systems for " << lb_systems->item( i )->text() << "\n";
         if ( cluster_systems.count( lb_systems->item( i )->text() ) )
         {
            US_Hydrodyn_Cluster_Config_Server *hccs = 
               new US_Hydrodyn_Cluster_Config_Server(
                                                cluster_systems[ lb_systems->item( i )->text() ],
                                                lb_systems->item( i )->text(),
                                                us_hydrodyn,
                                                this );
            US_Hydrodyn::fixWinButtons( hccs );
            hccs->exec();
            delete hccs;
         } else {
            QMessageBox::warning( this, 
                                  us_tr( "US-SOMO: Cluster config" ), 
                                  QString( us_tr( "The system %1 does not seem to have any information" ) ).arg( lb_systems->item( i )->text() ) );
         }
      }
   }
}

void US_Hydrodyn_Cluster_Config::http_error( QNetworkReply::NetworkError /* code */ ) {
   qDebug() << "http: error";
   current_http_error = http_reply->errorString();
   http_done( true );
}

void US_Hydrodyn_Cluster_Config::http_finished() {
   cout << "http: finished\n" << endl << flush;
   current_http_response = QString( http_reply->readAll() );
   cout << "http response:";
   cout << current_http_response << endl;

   if ( comm_mode == "check_user" )
   {
      map < QString, QString > readJson = US_Json::split( current_http_response );
      if ( readJson.count( "json parsing error" ) )
      {
         cout << QString( "json parsing error:%1\n" ).arg( readJson[ "json parsing error" ] );
      }

      if ( readJson.count( "name" ) &&
           readJson[ "name" ] == le_cluster_id->text() )
      {
         check_not_ok = false;
      } else {
         check_not_ok = true;
      }
   }
   if ( comm_mode == "add_user" )
   {
      // check for error
      check_not_ok = true;
      check_tried = false;
      
      map < QString, QString > readJson = US_Json::split( current_http_response );
      if ( readJson.count( "json parsing error" ) )
      {
         cout << QString( "json parsing error:%1\n" ).arg( readJson[ "json parsing error" ] );
      }

      current_response_status = readJson.count( "info" ) ?
         ( readJson[ "info" ].contains( QRegExp( "added$" ) ) ? QString( "Success" ) : readJson[ "info" ] )
           : QString( "Error" );
   }
   http_done( false );
}

void US_Hydrodyn_Cluster_Config::http_uploadProgress ( qint64 done, qint64 total )
{
   cout << "http: uploadProgress " << done << " " << total << "\n";
}

void US_Hydrodyn_Cluster_Config::http_downloadProgress ( qint64 done, qint64 total )
{
   cout << "http: downloadProgress " << done << " " << total << "\n";
   check_tried = true;
}

void US_Hydrodyn_Cluster_Config::http_done ( bool error )
{
   qDebug() << "http_done";

   disconnect( http_reply, SIGNAL( downloadProgress ( qint64, qint64 ) ), 0, 0 );
   disconnect( http_reply, SIGNAL( uploadProgress ( qint64, qint64 ) ), 0, 0 );
   disconnect( http_reply, SIGNAL( finished () ), 0, 0 );
   disconnect( http_reply, SIGNAL( error ( QNetworkReply::NetworkError ) ), 0, 0 );
   comm_active = false;
   http_reply->deleteLater();
   if ( error )
   {
         QMessageBox::warning( this,
                               us_tr("US-SOMO: Cluster Config"), 
                               us_tr( QString( "There was a error with the management server:\n%1" )
                                   .arg( current_http_error ) ),
                               QMessageBox::Ok,
                               QMessageBox::NoButton
                               );
         update_enables();
         return;
   }                               
                             
   if ( comm_mode == "add_user" )
   {
      if ( current_response_status == "Success" )
      {
         QMessageBox::information( this,
                                   us_tr("US-SOMO: Cluster Config: Add user"),
                                   QString( us_tr( "User \"%1\" successfully added" ) ).arg( le_cluster_id->text() ) );
      } else {
         QMessageBox::warning( this,
                               us_tr("US-SOMO: Cluster Config: Add user"), 
                               QString( us_tr( "There was an error trying to add User \"%1\"\nWarning: %2" ) )
                               .arg( le_cluster_id->text() )
                               .arg( current_http_response ),
                               QMessageBox::Ok,
                               QMessageBox::NoButton
                               );
      }
   }
  
   if ( comm_mode == "check_user" )
   {
      if ( check_not_ok )
      {
         QMessageBox::information( this,
                                   us_tr("US-SOMO: Cluster Config: Check user"),
                                   QString( us_tr( "User \"%1\" does not exist.\nPress \"Add user\" to add." ) ).arg( le_cluster_id->text() ) );
      } else {
         map < QString, QString > readJson = US_Json::split( current_http_response );
         QString errors;
         QString email = le_cluster_email->text();
         // bool password_ok = false;
         // if ( !readJson.count( "password" ) ||
         //      readJson[ "password" ] != le_cluster_pw->text() )
         // {
         //    errors += QString( us_tr( "Incorrect password.  Possibly another user has registered the same cluster id.\n" ) );
         // } else {
         //    password_ok = true;
         // }
         bool password_ok = true;

         if ( !readJson.count( "email" ) ||
              readJson[ "email" ] != le_cluster_email->text() )
         {
            if ( password_ok )
            {
               email = "";
               if ( readJson.count( "email" ) )
               {
                  email = readJson[ "email" ];
               }
               errors += QString( us_tr( "The email doesn't match, resetting\n" ) );
            } else {
               errors += QString( us_tr( "The email doesn't match\n" ) );
            }
         } 

         if ( errors.isEmpty() )
         {
            QMessageBox::information( this,
                                      us_tr("US-SOMO: Cluster Config: Check user"),
                                      QString( us_tr( "User \"%1\" exists." ) ).arg( le_cluster_id->text() ) );
         } else {
            QMessageBox::warning( this,
                                  us_tr("US-SOMO: Cluster Config: Check user"), 
                                  QString( us_tr( "The user \"%1\" exists, but:\n%2" ) )
                                  .arg( le_cluster_id->text() )
                                  .arg( errors ),
                                  QMessageBox::Ok,
                                  QMessageBox::NoButton
                                  );
            le_cluster_email->setText( email );
         }         
      }         
   }         

   update_enables();
}

void US_Hydrodyn_Cluster_Config::update_enables()
{
   pb_check_user    ->setEnabled( !comm_active && !le_cluster_id->text().isEmpty() && !le_cluster_pw->text().isEmpty() );
   pb_add_user      ->setEnabled( !comm_active && check_tried && check_not_ok );
   pb_reset         ->setEnabled( !comm_active );
   le_cluster_id    ->setEnabled( !comm_active );
   le_cluster_pw    ->setEnabled( !comm_active );
   le_cluster_pw2   ->setEnabled( !comm_active );
   le_cluster_email ->setEnabled( !comm_active );
   le_submit_url    ->setEnabled( !comm_active );
   le_manage_url    ->setEnabled( !comm_active );
   lb_systems       ->setEnabled( !comm_active );
   pb_edit          ->setEnabled( !comm_active );
   pb_add_new       ->setEnabled( !comm_active );
   pb_delete_system ->setEnabled( !comm_active );
   pb_save_config   ->setEnabled( !comm_active );
}

void US_Hydrodyn_Cluster_Config::check_user()
{
   if ( le_cluster_pw->text() != le_cluster_pw2->text() )
   {
      QMessageBox::warning( this,
                            us_tr("US-SOMO: Cluster Config: Check user"), 
                            us_tr( "The passwords do not match" ),
                            QMessageBox::Ok,
                            QMessageBox::NoButton
                            );
      return;
   }

   if ( le_cluster_pw->text().length() < 6 )
   {
      QMessageBox::warning( this,
                            us_tr("US-SOMO: Cluster Config: Check user"), 
                            us_tr( "Enter a longer password" ),
                            QMessageBox::Ok,
                            QMessageBox::NoButton
                            );
      return;
   }

   if ( le_cluster_id->text().length() < 4 )
   {
      QMessageBox::warning( this,
                            us_tr("US-SOMO: Cluster Config: Check user"), 
                            us_tr( "Enter a longer cluster id" ),
                            QMessageBox::Ok,
                            QMessageBox::NoButton
                            );
      return;
   }

   if ( le_cluster_email->text().isEmpty() )
   {
      QMessageBox::warning( this,
                            us_tr("US-SOMO: Cluster Config: Check user"), 
                            us_tr( "The email address is empty" ),
                            QMessageBox::Ok,
                            QMessageBox::NoButton
                            );
      return;
   }

   if ( !le_cluster_email->text().contains( QRegExp( "^\\S+@\\S+\\.\\w+$" ) ) )
   {
      QMessageBox::warning( this,
                            us_tr("US-SOMO: Cluster Config: Check user"), 
                            us_tr( "The email address is improperly formatted" ),
                            QMessageBox::Ok,
                            QMessageBox::NoButton
                            );
      return;
   }

   if ( le_manage_url->text().isEmpty() )
   {
      QMessageBox::warning( this,
                            us_tr("US-SOMO: Cluster Config: Check user"), 
                            us_tr( "The Job Management URL is empty" ),
                            QMessageBox::Ok,
                            QMessageBox::NoButton
                            );
      return;
   }

   if ( le_submit_url->text().isEmpty() )
   {
      QMessageBox::warning( this,
                            us_tr("US-SOMO: Cluster Config: Check user"), 
                            us_tr( "The Job Submit URL is empty" ),
                            QMessageBox::Ok,
                            QMessageBox::NoButton
                            );
      return;
   }

   // if ( !le_manage_url->text().contains( QRegExp( "^\\S+:\\d+/portal/app/idservice/user$" ) ) )
   // {
   //    QMessageBox::warning( this,
   //                          us_tr("US-SOMO: Cluster Config: Check user"), 
   //                          us_tr( "The Job Management URL is improperly formatted" ),
   //                          QMessageBox::Ok,
   //                          QMessageBox::NoButton
   //                          );
   //    return;
   // }

   // if ( !le_submit_url->text().contains( QRegExp( "^\\S+:\\d+$" ) ) )
   // {
   //    QMessageBox::warning( this,
   //                          us_tr("US-SOMO: Cluster Config: Check user"), 
   //                          us_tr( "The Job Submit URL is improperly formatted" ),
   //                          QMessageBox::Ok,
   //                          QMessageBox::NoButton
   //                          );
   //    return;
   // }
   
   current_http_response = "";
   current_http_error = "";

   // QString msg_request = 
   //    QString( "%1/%2.json" )
   //    .arg( QString( "%1" )
   //          .arg( le_manage_url->text() )
   //          .replace( QRegExp( "^.*:(\\d+)" ), "/" ) )
   //    .arg( le_cluster_id->text() )
   //    .replace ( QRegExp( "^//" ), "/" );

   QString msg_request =
      QString( "/userstatus?user=%1" ).arg( le_cluster_id->text() );

   QString manage_url_host =
      QString( "%1" )
      .arg( le_manage_url->text() )
      .replace( QRegExp( ":.*$" ), "" );

   QString manage_url_port =
      QString( "%1" )
      .arg( le_manage_url->text() )
      .replace( QRegExp( "^.*:" ), "" )
      .replace( QRegExp( "/.*$" ), "" );

   cout << "msg_request is " << msg_request << endl;
   cout << "manage_url_host is " << manage_url_host << endl;
   cout << "manage_url_port is " << manage_url_port << endl;

   comm_active = true;
   comm_mode   = "check_user";
   check_not_ok = true;
   current_response_status = "";
   update_enables();

   http_request.setUrl( QUrl( QString( "http://%1%2" ).arg( le_manage_url->text() ).arg( msg_request ) ) );
   http_reply = http_access_manager->get( http_request );

   connect( http_reply, SIGNAL( downloadProgress ( qint64, qint64 ) ), this, SLOT( http_downloadProgress( qint64, qint64 ) ) );
   connect( http_reply, SIGNAL( uploadProgress ( qint64, qint64 ) ), this, SLOT( http_uploadProgress( qint64, qint64 ) ) );
   connect( http_reply, SIGNAL( finished () ), this, SLOT( http_finished() ) );
   connect( http_reply, SIGNAL( error ( QNetworkReply::NetworkError ) ), this, SLOT( http_error ( QNetworkReply::NetworkError ) ) ); 

   return;
}

void US_Hydrodyn_Cluster_Config::add_user()
{
   comm_active = true;
   comm_mode   = "add_user";
   current_response_status = "";
   update_enables();

   // map < QString, QString > mqq;

   // mqq[ "username" ] = le_cluster_id   ->text();
   // mqq[ "password" ] = le_cluster_pw   ->text();
   // mqq[ "email"    ] = le_cluster_email->text();

   // QString post_data = US_Json::compose( mqq );

   current_http_response = "";

   // QString msg_request = 
   //    QString( "%1/newaccount" )
   //    .arg( QString( "%1" )
   //          .arg( le_manage_url->text() )
   //          .replace( QRegExp( "^.*:(\\d+)" ), "/" ) )
   //    .arg( le_cluster_id->text() );

   QString msg_request =
      QString( "/useradd?user=%1&email=%2&pw=%3" )
      .arg( le_cluster_id->text() )
      .arg( le_cluster_email->text() )
      .arg( le_cluster_pw->text() )
      ;

   QString manage_url_host =
      QString( "%1" )
      .arg( le_manage_url->text() )
      .replace( QRegExp( ":.*$" ), "" );

   QString manage_url_port =
      QString( "%1" )
      .arg( le_manage_url->text() )
      .replace( QRegExp( "^.*:" ), "" )
      .replace( QRegExp( "/.*$" ), "" );

   cout << "msg_request is " << msg_request << endl;
   cout << "manage_url_host is " << manage_url_host << endl;
   cout << "manage_url_port is " << manage_url_port << endl;
   //   cout << "post data is "   << post_data << endl; 


   http_request.setUrl( QUrl( QString( "http://%1%2" ).arg( le_manage_url->text() ).arg( msg_request ) ) );
   http_reply = http_access_manager->get( http_request );

   connect( http_reply, SIGNAL( downloadProgress ( qint64, qint64 ) ), this, SLOT( http_downloadProgress( qint64, qint64 ) ) );
   connect( http_reply, SIGNAL( uploadProgress ( qint64, qint64 ) ), this, SLOT( http_uploadProgress( qint64, qint64 ) ) );
   connect( http_reply, SIGNAL( finished () ), this, SLOT( http_finished() ) );
   connect( http_reply, SIGNAL( error ( QNetworkReply::NetworkError ) ), this, SLOT( http_error ( QNetworkReply::NetworkError ) ) ); 
}

void US_Hydrodyn_Cluster_Config::update_cluster_id( const QString & )
{
   check_tried = false;
   update_enables();
}

void US_Hydrodyn_Cluster_Config::update_cluster_pw( const QString & )
{
   check_tried = false;
   update_enables();
}

void US_Hydrodyn_Cluster_Config::update_cluster_pw2( const QString & )
{
   check_tried = false;
   update_enables();
}

void US_Hydrodyn_Cluster_Config::update_cluster_email( const QString & )
{
   check_tried = false;
   update_enables();
}

void US_Hydrodyn_Cluster_Config::update_manage_url( const QString & )
{
   check_tried = false;
   update_enables();
}

void US_Hydrodyn_Cluster_Config::reset()
{
   if (
       le_manage_url->text().contains( QRegExp( "^\\S+:\\d+$" ) ) &&
       // le_manage_url->text().contains( QRegExp( "^\\S+:\\d+/portal/app/idservice/user$" ) ) &&
       // le_submit_url->text().contains( QRegExp( "^\\S+:\\d+$" ) ) &&
       QMessageBox::Yes != QMessageBox::question(
                                                  this,
                                                  lbl_title->text(),
                                                  us_tr("Resetting will replace the configuration with the defaults.\n"
                                                     "Are you sure?" ),
                                                  QMessageBox::Yes, 
                                                  QMessageBox::No | QMessageBox::Default
                                                  )
       )
   {
      return;
   }
   if( !QFile::remove( "config" ) )
   {
      QMessageBox::warning( this,
                            lbl_title->text(),
                            us_tr( "Could not remove existing config file" ),
                            QMessageBox::Ok,
                            QMessageBox::NoButton );
      return;
   }
   ((US_Hydrodyn_Cluster *)cluster_window)->read_config();
   close();
}
