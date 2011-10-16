#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_cluster.h"

#define SLASH QDir::separator()

US_Hydrodyn_Cluster::US_Hydrodyn_Cluster(
                                         void *us_hydrodyn, 
                                         QWidget *p, 
                                         const char *name
                                         ) : QDialog(p, name)
{
   this->us_hydrodyn = us_hydrodyn;
   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("US-SOMO: Cluster"));

   setupGUI();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );
}

US_Hydrodyn_Cluster::~US_Hydrodyn_Cluster()
{
}

void US_Hydrodyn_Cluster::setupGUI()
{
   int minHeight1 = 30;

   lbl_title = new QLabel( tr( "Create file for cluster jobs" ), this);
   lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_target = new QLabel("Grid from experimental data:", this);
   lbl_target->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_target->setMinimumHeight(minHeight1);
   lbl_target->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   pb_set_target = new QPushButton(tr("Set experimental data file"), this);
   pb_set_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_set_target->setMinimumHeight(minHeight1);
   pb_set_target->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_set_target, SIGNAL(clicked()), SLOT(set_target()));

   le_target_file = new QLineEdit(this, "csv_filename Line Edit");
   le_target_file->setText("");
   le_target_file->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_target_file->setMinimumWidth(150);
   le_target_file->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_target_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   pb_create = new QPushButton(tr("Create cluster job files"), this);
   pb_create->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_create->setMinimumHeight(minHeight1);
   pb_create->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_create, SIGNAL(clicked()), SLOT(create()));

   pb_create_pkg = new QPushButton(tr("Create cluster job package"), this);
   pb_create_pkg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_create_pkg->setMinimumHeight(minHeight1);
   pb_create_pkg->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_create_pkg, SIGNAL(clicked()), SLOT(create_pkg()));
   
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
   QHBoxLayout *hbl_target = new QHBoxLayout( 0 );
   hbl_target->addSpacing( 4 );
   hbl_target->addWidget ( lbl_target );
   hbl_target->addSpacing( 4);
   hbl_target->addWidget ( pb_set_target );
   hbl_target->addSpacing( 4 );
   hbl_target->addWidget ( le_target_file );
   hbl_target->addSpacing( 4 );

   QHBoxLayout *hbl_create = new QHBoxLayout( 0 );
   hbl_create->addSpacing( 4 );
   hbl_create->addWidget ( pb_create );
   hbl_create->addSpacing( 4);
   hbl_create->addWidget ( pb_create_pkg );
   hbl_create->addSpacing( 4);

   QHBoxLayout *hbl_bottom = new QHBoxLayout( 0 );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_cancel );
   hbl_bottom->addSpacing( 4 );

   QVBoxLayout *background = new QVBoxLayout( this );
   background->addSpacing( 4);
   background->addWidget ( lbl_title );
   background->addSpacing( 4 );
   background->addLayout ( hbl_target );
   background->addSpacing( 4 );
   background->addLayout ( hbl_create );
   background->addSpacing( 4 );
   background->addLayout ( hbl_bottom );
   background->addSpacing( 4);
}

void US_Hydrodyn_Cluster::cancel()
{
   close();
}

void US_Hydrodyn_Cluster::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_cluster.html");
}

void US_Hydrodyn_Cluster::closeEvent(QCloseEvent *e)
{
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Cluster::set_target()
{
   QString filename = QFileDialog::getOpenFileName(
                                                   ((US_Hydrodyn *)us_hydrodyn)->somo_dir + QDir::separator() + "saxs",
                                                   "All files (*);;"
                                                   "ssaxs files (*.ssaxs);;"
                                                   "csv files (*.csv);;"
                                                   "int files [crysol] (*.int);;"
                                                   "dat files [foxs / other] (*.dat);;"
                                                   "fit files [crysol] (*.fit);;"
                                                   , this
                                                   , "open file dialog"
                                                   , "Set file for grid target"
                                                   );

   le_target_file->setText( filename );
}

void US_Hydrodyn_Cluster::create()
{
   // create the output file
   QString filename = QFileDialog::getSaveFileName(QString::null, QString::null,this );
}

void US_Hydrodyn_Cluster::create_pkg()
{
   // create the output file
   QString filename = QFileDialog::getSaveFileName(QString::null, QString::null,this );
}
