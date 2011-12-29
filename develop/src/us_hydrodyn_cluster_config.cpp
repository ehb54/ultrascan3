#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_cluster.h"
#include "../include/us_hydrodyn_cluster_config.h"
#include "../include/us_hydrodyn_cluster_config_server.h"

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

   cluster_systems = ((US_Hydrodyn_Cluster *)cluster_window)->cluster_systems;
   QString pkg_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "cluster";
   QDir::setCurrent( pkg_dir );

   setupGUI();

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
   Q_CHECK_PTR(pb_cancel);
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

   QHBoxLayout *hbl_cluster_id = new QHBoxLayout( 0 );
   hbl_cluster_id->addSpacing( 4 );
   hbl_cluster_id->addWidget ( lbl_cluster_id );
   hbl_cluster_id->addSpacing( 4 );
   hbl_cluster_id->addWidget ( le_cluster_id );
   hbl_cluster_id->addSpacing( 4 );

   QHBoxLayout *hbl_submit_url = new QHBoxLayout( 0 );
   hbl_submit_url->addSpacing( 4 );
   hbl_submit_url->addWidget ( lbl_submit_url );
   hbl_submit_url->addSpacing( 4 );
   hbl_submit_url->addWidget ( le_submit_url );
   hbl_submit_url->addSpacing( 4 );

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
   background->addLayout ( hbl_cluster_id );
   background->addSpacing( 4 );
   background->addLayout ( hbl_submit_url );
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
                                  tr( lbl_title->text() ),
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

   ((US_Hydrodyn_Cluster *)cluster_window)->cluster_config[ "userid" ] = le_cluster_id->text();
   ((US_Hydrodyn_Cluster *)cluster_window)->cluster_config[ "server" ] = le_submit_url->text();
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
                            tr( lbl_title->text() ),
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
