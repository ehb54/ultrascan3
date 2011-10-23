#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_cluster.h"
#include "../include/us_hydrodyn_cluster_config.h"

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

   QString pkg_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "cluster";
   QDir::setCurrent( pkg_dir );

   setupGUI();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );
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
   le_cluster_id->setText( ((US_Hydrodyn_Cluster *)cluster_window)->cluster_id );
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
   le_submit_url->setText( ((US_Hydrodyn_Cluster *)cluster_window)->submit_url );
   le_submit_url->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_submit_url->setMinimumWidth(150);
   le_submit_url->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_submit_url->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   lbl_stage_url = new QLabel(tr("File staging URL"), this);
   lbl_stage_url->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_stage_url->setMinimumHeight(minHeight1);
   lbl_stage_url->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_stage_url->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   le_stage_url = new QLineEdit(this, "csv_filename Line Edit");
   le_stage_url->setText( ((US_Hydrodyn_Cluster *)cluster_window)->stage_url );
   le_stage_url->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_stage_url->setMinimumWidth( 400 );
   le_stage_url->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_stage_url->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   pb_save_config = new QPushButton(tr("Save"), this);
   pb_save_config->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_save_config->setMinimumHeight(minHeight1);
   pb_save_config->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_save_config, SIGNAL(clicked()), SLOT(save_config()));

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

   QHBoxLayout *hbl_stage_url = new QHBoxLayout( 0 );
   hbl_stage_url->addSpacing( 4 );
   hbl_stage_url->addWidget ( lbl_stage_url );
   hbl_stage_url->addSpacing( 4 );
   hbl_stage_url->addWidget ( le_stage_url );
   hbl_stage_url->addSpacing( 4 );

   QHBoxLayout *hbl_bottom = new QHBoxLayout( 0 );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_save_config );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_cancel );
   hbl_bottom->addSpacing( 4 );

   QVBoxLayout *background = new QVBoxLayout( this );
   background->addSpacing( 4 );
   background->addWidget ( lbl_title );
   background->addSpacing( 4 );
   background->addLayout ( hbl_cluster_id );
   background->addSpacing( 4 );
   background->addLayout ( hbl_submit_url );
   background->addSpacing( 4 );
   background->addLayout ( hbl_stage_url );
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
   ((US_Hydrodyn_Cluster *)cluster_window)->cluster_id = le_cluster_id->text();
   ((US_Hydrodyn_Cluster *)cluster_window)->submit_url = le_submit_url->text();
   ((US_Hydrodyn_Cluster *)cluster_window)->stage_url  = le_stage_url->text();
   if ( !((US_Hydrodyn_Cluster *)cluster_window)->write_config() )
   {
      QMessageBox::warning( this, 
                            tr( lbl_title->text() ),
                            tr( ((US_Hydrodyn_Cluster *)cluster_window)->errormsg ) );
   } else {
      close();
   }
}
