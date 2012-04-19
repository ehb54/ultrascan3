#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_cluster.h"
#include "../include/us_hydrodyn_cluster_config.h"
#include "../include/us_hydrodyn_cluster_config_server.h"
#include "../include/us_json.h"

// note: this program uses cout and/or cerr and this should be replaced

#define SLASH QDir::separator()

US_Hydrodyn_Cluster_Config::US_Hydrodyn_Cluster_Config(
                                         void *us_hydrodyn, 
                                         QWidget *p, 
                                         const char *name
                                         ) : QDialog(p, name)
{
   this->us_hydrodyn = us_hydrodyn;
   setCaption(tr("US-SOMO: Cluster Config"));
   USglobal = new US_Config();
   cluster_window = (void *)p;
   check_tried = false;
   comm_active = false;

   cluster_systems = ((US_Hydrodyn_Cluster *)cluster_window)->cluster_systems;
   QString pkg_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "cluster";
   QDir::setCurrent( pkg_dir );

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

   lbl_title = new QLabel( tr( "Cluster configuration" ), this);
   lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_cluster_id = new QLabel(tr("Assigned cluster id"), this);
   lbl_cluster_id->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_cluster_id->setMinimumHeight(minHeight1);
   lbl_cluster_id->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_cluster_id->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   le_cluster_id = new QLineEdit(this, "csv_filename Line Edit");
   le_cluster_id->setText( ((US_Hydrodyn_Cluster *)cluster_window)->cluster_config[ "userid" ] );
   le_cluster_id->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_cluster_id->setMinimumWidth(150);
   le_cluster_id->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_cluster_id->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   connect( le_cluster_id, SIGNAL( textChanged( const QString & ) ), SLOT( update_cluster_id(const QString & ) ) );

   lbl_cluster_pw = new QLabel(tr("Password"), this);
   lbl_cluster_pw->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_cluster_pw->setMinimumHeight(minHeight1);
   lbl_cluster_pw->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_cluster_pw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   le_cluster_pw = new QLineEdit(this, "csv_filename Line Edit");
   le_cluster_pw->setText( ((US_Hydrodyn_Cluster *)cluster_window)->cluster_config[ "userpw" ] );
   le_cluster_pw->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_cluster_pw->setMinimumWidth(150);
   le_cluster_pw->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_cluster_pw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_cluster_pw->setEchoMode( QLineEdit::Password );
   connect( le_cluster_pw, SIGNAL( textChanged( const QString & ) ), SLOT( update_cluster_pw( const QString & ) ) );

   lbl_cluster_pw2 = new QLabel(tr("Repeat password"), this);
   lbl_cluster_pw2->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_cluster_pw2->setMinimumHeight(minHeight1);
   lbl_cluster_pw2->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_cluster_pw2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   le_cluster_pw2 = new QLineEdit(this, "csv_filename Line Edit");
   le_cluster_pw2->setText( ((US_Hydrodyn_Cluster *)cluster_window)->cluster_config[ "userpw" ] );
   le_cluster_pw2->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_cluster_pw2->setMinimumWidth(150);
   le_cluster_pw2->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_cluster_pw2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_cluster_pw2->setEchoMode( QLineEdit::Password );
   connect( le_cluster_pw2, SIGNAL( textChanged( const QString & ) ), SLOT( update_cluster_pw2( const QString & ) ) );

   lbl_cluster_email = new QLabel(tr("User email address"), this);
   lbl_cluster_email->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_cluster_email->setMinimumHeight(minHeight1);
   lbl_cluster_email->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_cluster_email->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   le_cluster_email = new QLineEdit(this, "csv_filename Line Edit");
   le_cluster_email->setText( ((US_Hydrodyn_Cluster *)cluster_window)->cluster_config[ "useremail" ] );
   le_cluster_email->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_cluster_email->setMinimumWidth(150);
   le_cluster_email->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_cluster_email->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   connect( le_cluster_email, SIGNAL( textChanged( const QString & ) ), SLOT( update_cluster_email( const QString & ) ) );

   lbl_submit_url = new QLabel(tr("Job submission URL"), this);
   lbl_submit_url->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_submit_url->setMinimumHeight(minHeight1);
   lbl_submit_url->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_submit_url->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   le_submit_url = new QLineEdit(this, "csv_filename Line Edit");
   le_submit_url->setText( ((US_Hydrodyn_Cluster *)cluster_window)->cluster_config[ "server" ] );
   le_submit_url->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_submit_url->setMinimumWidth(150);
   le_submit_url->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_submit_url->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   lbl_manage_url = new QLabel(tr("Job management URL"), this);
   lbl_manage_url->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_manage_url->setMinimumHeight(minHeight1);
   lbl_manage_url->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_manage_url->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   le_manage_url = new QLineEdit(this, "csv_filename Line Edit");
   le_manage_url->setText( ((US_Hydrodyn_Cluster *)cluster_window)->cluster_config[ "manage" ] );
   le_manage_url->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_manage_url->setMinimumWidth(150);
   le_manage_url->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_manage_url->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   connect( le_manage_url, SIGNAL( textChanged( const QString & ) ), SLOT( update_manage_url( const QString & ) ) );

   pb_check_user = new QPushButton(tr("Check user status"), this);
   pb_check_user->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_check_user->setMinimumHeight(minHeight1);
   pb_check_user->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_check_user, SIGNAL(clicked()), SLOT(check_user()));

   pb_add_user = new QPushButton(tr("Add new user"), this);
   pb_add_user->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_add_user->setMinimumHeight(minHeight1);
   pb_add_user->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_add_user, SIGNAL(clicked()), SLOT(add_user()));
   pb_add_user->setEnabled( false );

   pb_reset = new QPushButton(tr("Reset"), this);
   pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_reset->setMinimumHeight(minHeight1);
   pb_reset->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));

   lbl_systems = new QLabel(tr("Systems"), this);
   lbl_systems->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_systems->setMinimumHeight(minHeight1);
   lbl_systems->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_systems->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   lb_systems = new QListBox(this);
   lb_systems->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lb_systems->setMinimumHeight(minHeight1 * 10);
   lb_systems->setMinimumWidth( 500 );
   lb_systems->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lb_systems->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   lb_systems->setEnabled(true);

   for ( map < QString, map < QString, QString > >::iterator it = cluster_systems.begin();
         it != cluster_systems.end();
         it++ )
   {
      lb_systems->insertItem( it->first );
   }

   lb_systems->setCurrentItem(0);
   lb_systems->setSelected(0, false);
   lb_systems->setSelectionMode( QListBox::Single );
   connect( lb_systems, SIGNAL( selectionChanged() ), SLOT( systems() ) );

   pb_add_new = new QPushButton(tr("Add new system"), this);
   pb_add_new->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_add_new->setMinimumHeight(minHeight1);
   pb_add_new->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_add_new, SIGNAL(clicked()), SLOT(add_new()));

   pb_edit = new QPushButton(tr("Edit system details"), this);
   pb_edit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_edit->setMinimumHeight(minHeight1);
   pb_edit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_edit, SIGNAL(clicked()), SLOT(edit()));

   pb_delete_system = new QPushButton(tr("Delete system"), this);
   pb_delete_system->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_delete_system->setMinimumHeight(minHeight1);
   pb_delete_system->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_delete_system, SIGNAL(clicked()), SLOT(delete_system()));

   pb_cancel = new QPushButton(tr("Cancel"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_save_config = new QPushButton(tr("Close"), this);
   pb_save_config->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_save_config->setMinimumHeight(minHeight1);
   pb_save_config->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_save_config, SIGNAL(clicked()), SLOT(save_config()));

   // build layout

   QGridLayout *gl_grid = new QGridLayout( 0 );

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

   QHBoxLayout *hbl_check_add_user = new QHBoxLayout( 0 );
   hbl_check_add_user->addWidget( pb_check_user );
   hbl_check_add_user->addWidget( pb_add_user );
   hbl_check_add_user->addWidget( pb_reset );

   QHBoxLayout *hbl_add_del = new QHBoxLayout( 0 );
   hbl_add_del->addSpacing( 4 );
   hbl_add_del->addWidget ( pb_add_new );
   hbl_add_del->addSpacing( 4 );
   hbl_add_del->addWidget ( pb_edit );
   hbl_add_del->addSpacing( 4 );
   hbl_add_del->addWidget ( pb_delete_system );
   hbl_add_del->addSpacing( 4 );

   QHBoxLayout *hbl_bottom = new QHBoxLayout( 0 );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_cancel );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_save_config );
   hbl_bottom->addSpacing( 4 );

   QVBoxLayout *background = new QVBoxLayout( this );
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
      submit_http.abort();
   }
   close();
}

void US_Hydrodyn_Cluster_Config::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_cluster_config.html");
}

void US_Hydrodyn_Cluster_Config::closeEvent(QCloseEvent *e)
{
   if ( comm_active )
   {
      submit_http.abort();
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
                                  QString( tr( "Staging locations for duplicate physical servers must be identical\n"
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

   ((US_Hydrodyn_Cluster *)cluster_window)->cluster_stage_to_system.clear();

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
                            tr( ((US_Hydrodyn_Cluster *)cluster_window)->errormsg ) );
   } else {
      cout << ((US_Hydrodyn_Cluster *)cluster_window)->list_config();
      close();
   }
}

void US_Hydrodyn_Cluster_Config::systems()
{
   bool any_selected = false;
   for ( int i = 0; i < lb_systems->numRows(); i++ )
   {
      if ( lb_systems->isSelected(i) )
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
   QString text = QInputDialog::getText(
                                        tr( "US-SOMO: Cluster Config: Add new system" ),
                                        tr( "Enter the unique system name:" ), 
                                        QLineEdit::Normal,
                                        QString::null, 
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
                            tr( "US-SOMO: Cluster config: Add new" ),
                            QString( tr( "The system %1 already exists" ) ).arg( text ) );
      return;
   }
    
   bool any_selected = false;
   unsigned int pos;
   for ( int i = 0; i < lb_systems->numRows(); i++ )
   {
      if ( lb_systems->isSelected(i) )
      {
         pos = i;
         any_selected = true;
         break;
      }
   }

   bool do_blank = true;
   if ( any_selected && 
        cluster_systems.count( lb_systems->text( pos ) ) )
   {
      switch ( QMessageBox::question(
                                     this,
                                     tr( "US-SOMO: Cluster config: Add new" ),
                                     QString(
                                             tr( "Do you want to copy system data from the selected system '%1?'" )
                                             ).arg( lb_systems->text( pos ) ),
                                     tr( "&Yes" ),
                                     tr( "&No" ),
                                     QString::null,
                                     0,
                                     1
                                     ) )
      {
      case 0 : 
         cluster_systems[ text ] = cluster_systems[ lb_systems->text( pos ) ];
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
      
      for ( unsigned int i = 0; i < params.size(); i++ )
      {
         tmp_system[ params[ i ] ] = "";
      }
      cluster_systems[ text ] = tmp_system;
   }

   lb_systems->clear();
   
   for ( map < QString, map < QString, QString > >::iterator it = cluster_systems.begin();
         it != cluster_systems.end();
         it++ )
   {
      lb_systems->insertItem( it->first );
   }
}

void US_Hydrodyn_Cluster_Config::delete_system()
{
   for ( int i = 0; i < lb_systems->numRows(); i++ )
   {
      if ( lb_systems->isSelected(i) )
      {
         if ( cluster_systems.count( lb_systems->text( i ) ) )
         {
            cluster_systems.erase( lb_systems->text( i ) );
            lb_systems->clear();
            
            for ( map < QString, map < QString, QString > >::iterator it = cluster_systems.begin();
                  it != cluster_systems.end();
                  it++ )
            {
               lb_systems->insertItem( it->first );
            }
         } else {
            QMessageBox::warning( this, 
                                  tr( "US-SOMO: Cluster config" ), 
                                  QString( tr( "Internal error: The system %1 does not seem to exist?" ) ).arg( lb_systems->text( i ) ) );
         }
      }
   }
   systems();
}

void US_Hydrodyn_Cluster_Config::edit()
{
   for ( int i = 0; i < lb_systems->numRows(); i++ )
   {
      if ( lb_systems->isSelected(i) )
      {
         cout << "run config systems for " << lb_systems->text( i ) << "\n";
         if ( cluster_systems.count( lb_systems->text( i ) ) )
         {
            US_Hydrodyn_Cluster_Config_Server *hccs = 
               new US_Hydrodyn_Cluster_Config_Server(
                                                cluster_systems[ lb_systems->text( i ) ],
                                                lb_systems->text( i ),
                                                us_hydrodyn,
                                                this );
            hccs->exec();
            delete hccs;
         } else {
            QMessageBox::warning( this, 
                                  tr( "US-SOMO: Cluster config" ), 
                                  QString( tr( "The system %1 does not seem to have any information" ) ).arg( lb_systems->text( i ) ) );
         }
      }
   }
}

void US_Hydrodyn_Cluster_Config::http_stateChanged ( int /* state */ )
{
   // editor_msg( "blue", QString( "http state %1" ).arg( state ) );
}

void US_Hydrodyn_Cluster_Config::http_responseHeaderReceived ( const QHttpResponseHeader & resp )
{
   cout << resp.reasonPhrase() << endl;
}

void US_Hydrodyn_Cluster_Config::http_readyRead( const QHttpResponseHeader & resp )
{
   cout << "http: readyRead\n" << endl << flush;
   cout << resp.reasonPhrase() << endl;
   // current_http_response = QString( "%1" ).arg( submit_http.readAll() );
   current_http_response = QString( submit_http.readAll() );
   cout << "http response:";
   cout << current_http_response << endl;

   if ( comm_mode == "check_user" )
   {
      map < QString, QString > readJson = US_Json::split( current_http_response );
      if ( readJson.count( "json parsing error" ) )
      {
         cout << QString( "json parsing error:%1\n" ).arg( readJson[ "json parsing error" ] );
      }

      if ( readJson.count( "username" ) &&
           readJson[ "username" ] == le_cluster_id->text() )
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
      // http response:<?xml version="1.0" encoding="UTF-8" standalone="yes"?><userresponse><status>Success</status></userresponse>
      
      current_response_status = current_http_response;
      current_response_status.replace( QRegExp( "^.*<status>" ), "" );
      current_response_status.replace( QRegExp( "</status>.*$" ), "" );
   }
}

void US_Hydrodyn_Cluster_Config::http_dataSendProgress ( int done, int total )
{
   cout << "http: datasendprogress " << done << " " << total << "\n";
   check_tried = true;
}

void US_Hydrodyn_Cluster_Config::http_dataReadProgress ( int done, int total )
{
   cout << "http: datareadprogress " << done << " " << total << "\n";
}

void US_Hydrodyn_Cluster_Config::http_requestStarted ( int id )
{
   cout << "http: requestStarted " << id << "\n";
}

void US_Hydrodyn_Cluster_Config::http_requestFinished ( int id, bool error  )
{
   cout << "http: requestFinished " << id << " " << error << "\n";
}

void US_Hydrodyn_Cluster_Config::http_done ( bool error )
{
   cout << "http_done error " << error << endl;
   if ( error )
   {
      switch( submit_http.error() )
      {
      case QHttp::NoError :
         current_http_error = tr( "No error occurred." );
         break;

      case QHttp::HostNotFound:
         current_http_error = tr( "The host name lookup failed." );
         break;

      case QHttp::ConnectionRefused:
         current_http_error = tr( "The server refused the connection." );
         break;

      case QHttp::UnexpectedClose:
         current_http_error = tr( "The server closed the connection unexpectedly." );
         break;

      case QHttp::InvalidResponseHeader:
         current_http_error = tr( "The server sent an invalid response header." );
         break;

      case QHttp::WrongContentLength:
         current_http_error = tr( "The client could not read the content correctly because an error with respect to the content length occurred." );
         break;

      case QHttp::Aborted:
         current_http_error = tr( "The request was aborted with abort()." );
         break;

      case QHttp::UnknownError:
      default:
         current_http_error = tr( "Unknown Error." );
         break;
      }
      cout << current_http_error << endl;
   }
   disconnect( &submit_http, SIGNAL( stateChanged ( int ) ), 0, 0 );
   disconnect( &submit_http, SIGNAL( responseHeaderReceived ( const QHttpResponseHeader & ) ), 0, 0 );
   disconnect( &submit_http, SIGNAL( readyRead ( const QHttpResponseHeader & ) ), 0, 0 );
   disconnect( &submit_http, SIGNAL( dataSendProgress ( int, int ) ), 0, 0 );
   disconnect( &submit_http, SIGNAL( dataReadProgress ( int, int ) ), 0, 0 );
   disconnect( &submit_http, SIGNAL( requestStarted ( int ) ), 0, 0 );
   disconnect( &submit_http, SIGNAL( requestFinished ( int, bool ) ), 0, 0 );
   disconnect( &submit_http, SIGNAL( done ( bool ) ), 0, 0 );
   comm_active = false;
   if ( error )
   {
         QMessageBox::warning( this,
                               tr("US-SOMO: Cluster Config"), 
                               tr( QString( "There was a error with the management server:\n%1" )
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
                                   tr("US-SOMO: Cluster Config: Add user"),
                                   QString( tr( "User \"%1\" successfully added" ) ).arg( le_cluster_id->text() ) );
      } else {
         QMessageBox::warning( this,
                               tr("US-SOMO: Cluster Config: Add user"), 
                               QString( tr( "There was an error trying to add User \"%1\"\nWarning: %2" ) )
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
                                   tr("US-SOMO: Cluster Config: Check user"),
                                   QString( tr( "User \"%1\" does not exist.\nPress \"Add user\" to add." ) ).arg( le_cluster_id->text() ) );
      } else {
         map < QString, QString > readJson = US_Json::split( current_http_response );
         QString errors;
         QString email = le_cluster_email->text();
         bool password_ok = false;
         if ( !readJson.count( "password" ) ||
              readJson[ "password" ] != le_cluster_pw->text() )
         {
            errors += QString( tr( "Incorrect password.  Possibly another user has registered the same cluster id.\n" ) );
         } else {
            password_ok = true;
         }

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
               errors += QString( tr( "The email doesn't match, resetting\n" ) );
            } else {
               errors += QString( tr( "The email doesn't match\n" ) );
            }
         } 

         if ( errors.isEmpty() )
         {
            QMessageBox::information( this,
                                      tr("US-SOMO: Cluster Config: Check user"),
                                      QString( tr( "User \"%1\" exists." ) ).arg( le_cluster_id->text() ) );
         } else {
            QMessageBox::warning( this,
                                  tr("US-SOMO: Cluster Config: Check user"), 
                                  QString( tr( "The user \"%1\" exists, but:\n%2" ) )
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
                            tr("US-SOMO: Cluster Config: Check user"), 
                            tr( "The passwords do not match" ),
                            QMessageBox::Ok,
                            QMessageBox::NoButton
                            );
      return;
   }

   if ( le_cluster_pw->text().length() < 6 )
   {
      QMessageBox::warning( this,
                            tr("US-SOMO: Cluster Config: Check user"), 
                            tr( "Enter a longer password" ),
                            QMessageBox::Ok,
                            QMessageBox::NoButton
                            );
      return;
   }

   if ( le_cluster_id->text().length() < 4 )
   {
      QMessageBox::warning( this,
                            tr("US-SOMO: Cluster Config: Check user"), 
                            tr( "Enter a longer cluster id" ),
                            QMessageBox::Ok,
                            QMessageBox::NoButton
                            );
      return;
   }

   if ( le_cluster_email->text().isEmpty() )
   {
      QMessageBox::warning( this,
                            tr("US-SOMO: Cluster Config: Check user"), 
                            tr( "The email address is empty" ),
                            QMessageBox::Ok,
                            QMessageBox::NoButton
                            );
      return;
   }

   if ( !le_cluster_email->text().contains( QRegExp( "^\\w+@\\S+\\.\\w+$" ) ) )
   {
      QMessageBox::warning( this,
                            tr("US-SOMO: Cluster Config: Check user"), 
                            tr( "The email address is improperly formatted" ),
                            QMessageBox::Ok,
                            QMessageBox::NoButton
                            );
      return;
   }

   if ( le_manage_url->text().isEmpty() )
   {
      QMessageBox::warning( this,
                            tr("US-SOMO: Cluster Config: Check user"), 
                            tr( "The Job Management URL is empty" ),
                            QMessageBox::Ok,
                            QMessageBox::NoButton
                            );
      return;
   }

   if ( le_submit_url->text().isEmpty() )
   {
      QMessageBox::warning( this,
                            tr("US-SOMO: Cluster Config: Check user"), 
                            tr( "The Job Submit URL is empty" ),
                            QMessageBox::Ok,
                            QMessageBox::NoButton
                            );
      return;
   }

   if ( !le_manage_url->text().contains( QRegExp( "^\\S+:\\d+/portal/app/idservice/user$" ) ) )
   {
      QMessageBox::warning( this,
                            tr("US-SOMO: Cluster Config: Check user"), 
                            tr( "The Job Management URL is improperly formatted" ),
                            QMessageBox::Ok,
                            QMessageBox::NoButton
                            );
      return;
   }

   if ( !le_submit_url->text().contains( QRegExp( "^\\S+:\\d+$" ) ) )
   {
      QMessageBox::warning( this,
                            tr("US-SOMO: Cluster Config: Check user"), 
                            tr( "The Job Submit URL is improperly formatted" ),
                            QMessageBox::Ok,
                            QMessageBox::NoButton
                            );
      return;
   }
   
   current_http_response = "";
   current_http_error = "";

   QString msg_request = 
      QString( "%1/%2.json" )
      .arg( QString( "%1" )
            .arg( le_manage_url->text() )
            .replace( QRegExp( "^.*:(\\d+)" ), "/" ) )
      .arg( le_cluster_id->text() )
      .replace ( QRegExp( "^//" ), "/" );

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

   connect( &submit_http, SIGNAL( stateChanged ( int ) ), this, SLOT( http_stateChanged ( int ) ) );
   connect( &submit_http, SIGNAL( responseHeaderReceived ( const QHttpResponseHeader & ) ), this, SLOT( http_responseHeaderReceived ( const QHttpResponseHeader & ) ) );
   connect( &submit_http, SIGNAL( readyRead ( const QHttpResponseHeader & ) ), this, SLOT( http_readyRead ( const QHttpResponseHeader & ) ) );
   connect( &submit_http, SIGNAL( dataSendProgress ( int, int ) ), this, SLOT( http_dataSendProgress ( int, int ) ) );
   connect( &submit_http, SIGNAL( dataReadProgress ( int, int ) ), this, SLOT( http_dataReadProgress ( int, int ) ) );
   connect( &submit_http, SIGNAL( requestStarted ( int ) ), this, SLOT( http_requestStarted ( int ) ) );
   connect( &submit_http, SIGNAL( requestFinished ( int, bool ) ), this, SLOT( http_requestFinished ( int, bool ) ) );
   connect( &submit_http, SIGNAL( done ( bool ) ), this, SLOT( http_done ( bool ) ) );

   submit_http.setHost( manage_url_host, manage_url_port.toUInt() );
   submit_http.get( msg_request );

   return;

   // QHttpRequestHeader header("POST", "/" );
   // header.setValue( "Host", submit_url_host );
   // header.setContentType( "application/xml" );
   // submit_http.setHost( submit_url_host, submit_url_port.toUInt() );
   // without the qba below, QHttp:request will send a null
   // QByteArray qba = xml.utf8();
   // qba.resize( qba.size() - 1 );
   // submit_http.request( header, qba );
}

void US_Hydrodyn_Cluster_Config::add_user()
{
   comm_active = true;
   comm_mode   = "add_user";
   current_response_status = "";
   update_enables();

   map < QString, QString > mqq;

   mqq[ "username" ] = le_cluster_id   ->text();
   mqq[ "password" ] = le_cluster_pw   ->text();
   mqq[ "email"    ] = le_cluster_email->text();

   QString post_data = US_Json::compose( mqq );

   current_http_response = "";

   QString msg_request = 
      QString( "%1/newaccount" )
      .arg( QString( "%1" )
            .arg( le_manage_url->text() )
            .replace( QRegExp( "^.*:(\\d+)" ), "/" ) )
      .arg( le_cluster_id->text() );

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
   cout << "post data is "   << post_data << endl; 

   connect( &submit_http, SIGNAL( stateChanged ( int ) ), this, SLOT( http_stateChanged ( int ) ) );
   connect( &submit_http, SIGNAL( responseHeaderReceived ( const QHttpResponseHeader & ) ), this, SLOT( http_responseHeaderReceived ( const QHttpResponseHeader & ) ) );
   connect( &submit_http, SIGNAL( readyRead ( const QHttpResponseHeader & ) ), this, SLOT( http_readyRead ( const QHttpResponseHeader & ) ) );
   connect( &submit_http, SIGNAL( dataSendProgress ( int, int ) ), this, SLOT( http_dataSendProgress ( int, int ) ) );
   connect( &submit_http, SIGNAL( dataReadProgress ( int, int ) ), this, SLOT( http_dataReadProgress ( int, int ) ) );
   connect( &submit_http, SIGNAL( requestStarted ( int ) ), this, SLOT( http_requestStarted ( int ) ) );
   connect( &submit_http, SIGNAL( requestFinished ( int, bool ) ), this, SLOT( http_requestFinished ( int, bool ) ) );
   connect( &submit_http, SIGNAL( done ( bool ) ), this, SLOT( http_done ( bool ) ) );

   QHttpRequestHeader header("POST", msg_request );
   header.setValue( "Host", manage_url_host );
   header.setContentType( "application/json" );
   submit_http.setHost( manage_url_host, manage_url_port.toUInt() );
   // without the qba below, QHttp:request will send a null
   QByteArray qba = post_data.utf8();
   qba.resize( qba.size() - 1 );
   submit_http.request( header, qba );
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

void US_Hydrodyn_Cluster_Config::check_socket( QString /* name */, QString /* port */ )
{
   comm_active = true;
   socket_hostfound = false;
   socket_is_connected = false;
   update_enables();

   connect( &test_socket, SIGNAL( socket_hostFound() ), this, SLOT( socket_socket_hostFound() ) );
   connect( &test_socket, SIGNAL( socket_connected() ), this, SLOT( socket_socket_connected() ) );
   connect( &test_socket, SIGNAL( socket_connectionClosed() ), this, SLOT( socket_socket_connectionClosed() ) );
   connect( &test_socket, SIGNAL( socket_delayedCloseFinished() ), this, SLOT( socket_socket_delayedCloseFinished() ) );
   connect( &test_socket, SIGNAL( socket_readyRead() ), this, SLOT( socket_socket_readyRead() ) );
   connect( &test_socket, SIGNAL( socket_bytesWritten ( int nbytes ) ), this, SLOT( socket_socket_bytesWritten ( int nbytes ) ) );
   connect( &test_socket, SIGNAL( socket_error ( int ) ), this, SLOT( socket_socket_error ( int ) ) );
}

void US_Hydrodyn_Cluster_Config::socket_hostFound ()
{
   socket_hostfound = true;
   cout << "socket_hostFound\n";
}

void US_Hydrodyn_Cluster_Config::socket_connected ()
{
   socket_is_connected = true;
   cout << "socket_connected\n";
}

void US_Hydrodyn_Cluster_Config::socket_connectionClosed ()
{
   cout << "socket_connectionClosed\n";
   comm_active = false;
   disconnect( &submit_http, SIGNAL( stateChanged ( int ) ), 0, 0 );
   disconnect( &test_socket, SIGNAL( socket_hostFound() ), 0, 0 );
   disconnect( &test_socket, SIGNAL( socket_connected() ), 0, 0 );
   disconnect( &test_socket, SIGNAL( socket_connectionClosed() ), 0, 0 );
   disconnect( &test_socket, SIGNAL( socket_delayedCloseFinished() ), 0, 0 );
   disconnect( &test_socket, SIGNAL( socket_readyRead() ), 0, 0 );
   disconnect( &test_socket, SIGNAL( socket_bytesWritten ( int nbytes ) ), 0, 0 );
   disconnect( &test_socket, SIGNAL( socket_error ( int ) ), 0, 0 );
   update_enables();
}

void US_Hydrodyn_Cluster_Config::socket_delayedCloseFinished ()
{
   cout << "socket_delayedCloseFinished\n";
}

void US_Hydrodyn_Cluster_Config::socket_readyRead ()
{
   cout << "socket_readyRead\n";
}

void US_Hydrodyn_Cluster_Config::socket_bytesWritten ( int nbytes )
{
   cout << "socket_bytesWritten " << nbytes << "\n";
}

void US_Hydrodyn_Cluster_Config::socket_error ( int error )
{
   cout << "socket_error " << error << "\n";
   test_socket.close();
   comm_active = false;
   update_enables();
}

void US_Hydrodyn_Cluster_Config::reset()
{
   if ( le_manage_url->text().contains( QRegExp( "^\\S+:\\d+/portal/app/idservice/user$" ) ) &&
        le_submit_url->text().contains( QRegExp( "^\\S+:\\d+$" ) ) &&
        QMessageBox::Yes != QMessageBox::question(
                                                  this,
                                                  lbl_title->text(),
                                                  tr("Resetting will replace the configuration with the defaults.\n"
                                                     "Are you sure?" ),
                                                  QMessageBox::Yes, 
                                                  QMessageBox::No | QMessageBox::Default
                                                  ) )
   {
      return;
   }
   if( !QFile::remove( "config" ) )
   {
      QMessageBox::warning( this,
                            lbl_title->text(),
                            tr( "Could not remove existing config file" ),
                            QMessageBox::Ok,
                            QMessageBox::NoButton );
      return;
   }
   ((US_Hydrodyn_Cluster *)cluster_window)->read_config();
   close();
}
