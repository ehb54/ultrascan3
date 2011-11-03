#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_cluster.h"
#include "../include/us_hydrodyn_cluster_config.h"
#include "../include/us_hydrodyn_cluster_config_server.h"

#define SLASH QDir::separator()

US_Hydrodyn_Cluster_Config_Server::US_Hydrodyn_Cluster_Config_Server(
                                                                     map < QString, QString > &system_map,
                                                                     QString server_name,
                                                                     void *us_hydrodyn, 
                                                                     QWidget *p, 
                                                                     const char *name
                                                                     ) : QDialog(p, name)
{
   this->system_map  = &system_map;
   our_system_map    = system_map;
   this->server_name = server_name;
   this->us_hydrodyn = us_hydrodyn;

   setCaption(tr("US-SOMO: Cluster Config: Systems"));
   USglobal = new US_Config();
   cluster_window = (void *)( p->parentWidget() );

   QString pkg_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "cluster";
   QDir::setCurrent( pkg_dir );

   setupGUI();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );
}

US_Hydrodyn_Cluster_Config_Server::~US_Hydrodyn_Cluster_Config_Server()
{
}

void US_Hydrodyn_Cluster_Config_Server::setupGUI()
{
   int minHeight1 = 30;

   lbl_title = new QLabel( QString( tr( "Cluster System: %1" ) ).arg( server_name ), this);
   lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   for ( map < QString, QString >::iterator it = our_system_map.begin();
         it != our_system_map.end();
         it++ )
   {
      QLabel *lbl_tmp = new QLabel( it->first, this);
      lbl_tmp->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
      lbl_tmp->setMinimumHeight(minHeight1);
      lbl_tmp->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      lbl_tmp->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));
      lbls.push_back( lbl_tmp );

      QLineEdit *le_tmp = new QLineEdit(this, "csv_filename Line Edit");
      le_tmp->setText( it->second );
      le_tmp->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
      le_tmp->setMinimumWidth(450);
      le_tmp->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      le_tmp->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
      les.push_back( le_tmp );
   }

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

   QGridLayout *background = new QGridLayout( this, 0, 0, 4, 2 );

   unsigned int pos = 0;

   background->addMultiCellWidget( lbl_title, pos, pos, 0, 2 );
   pos++;

   for ( unsigned int i = 0; i < lbls.size(); i++ )
   {
      background->addWidget         ( lbls[ i ], pos, 0 );
      background->addMultiCellWidget( les [ i ], pos, pos, 1, 2 );
      pos++;
   }
   
   background->addWidget( pb_cancel     , pos, 0 );
   background->addWidget( pb_help       , pos, 1 );
   background->addWidget( pb_save_config, pos, 2 );
}

void US_Hydrodyn_Cluster_Config_Server::cancel()
{
   close();
}

void US_Hydrodyn_Cluster_Config_Server::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_cluster_config.html");
}

void US_Hydrodyn_Cluster_Config_Server::closeEvent(QCloseEvent *e)
{
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Cluster_Config_Server::save_config()
{
   for ( unsigned int i = 0; i < lbls.size(); i++ )
   {
      our_system_map[ lbls[ i ]->text() ] = les[ i ]->text();
   }
   *system_map = our_system_map;
   close();
}
