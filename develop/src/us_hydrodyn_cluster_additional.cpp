#include "../include/us_hydrodyn_cluster.h"
#include "../include/us_hydrodyn_cluster_additional.h"
#include "../include/us_hydrodyn_cluster_dammin.h"
#include "../include/us_hydrodyn.h"

#define SLASH QDir::separator()

US_Hydrodyn_Cluster_Additional::US_Hydrodyn_Cluster_Additional(
                                                               void *us_hydrodyn, 
                                                               QWidget *p, 
                                                               const char *name
                                                               ) : QDialog(p, name)
{
   cluster_window = (void *)p;

   this->us_hydrodyn = us_hydrodyn;

   options_active   = ( ( ( US_Hydrodyn * ) us_hydrodyn )->cluster_additional_methods_options_active   );
   options_selected = ( ( ( US_Hydrodyn * ) us_hydrodyn )->cluster_additional_methods_options_selected );

   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption( tr( "US-SOMO: Cluster: Other Methods" ) );

   setupGUI();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry(global_Xpos, global_Ypos, 0, 0 );
   update_enables();
}

US_Hydrodyn_Cluster_Additional::~US_Hydrodyn_Cluster_Additional()
{
}

void US_Hydrodyn_Cluster_Additional::setupGUI()
{
   int minHeight1 = 30;

   lbl_title = new QLabel( tr( "    US-SOMO: Cluster: Other Methods    " ), this );
   lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   cb_bfnb = new QCheckBox(this);
   cb_bfnb->setText(tr(" Active "));
   cb_bfnb->setChecked( options_active.count( "bfnb" ) && options_active[ "bfnb" ] );
   cb_bfnb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_bfnb->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_bfnb->setMinimumHeight( minHeight1 );
   connect( cb_bfnb, SIGNAL( clicked() ), this, SLOT( set_bfnb() ) );

   pb_bfnb = new QPushButton( "BFNB", this );
   pb_bfnb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_bfnb->setMinimumHeight(minHeight1);
   pb_bfnb->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect( pb_bfnb, SIGNAL( clicked() ), SLOT( bfnb() ) );

   cb_csa = new QCheckBox(this);
   cb_csa->setText(tr(" Active "));
   cb_csa->setChecked( options_active.count( "csa" ) && options_active[ "csa" ] );
   cb_csa->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_csa->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_csa->setMinimumHeight( minHeight1 );
   connect( cb_csa, SIGNAL( clicked() ), this, SLOT( set_csa() ) );

   pb_csa = new QPushButton( "CSA", this );
   pb_csa->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_csa->setMinimumHeight(minHeight1);
   pb_csa->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect( pb_csa, SIGNAL( clicked() ), SLOT( csa() ) );

   cb_dammin = new QCheckBox(this);
   cb_dammin->setText(tr(" Active "));
   cb_dammin->setChecked( options_active.count( "dammin" ) && options_active[ "dammin" ] );
   cb_dammin->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_dammin->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_dammin->setMinimumHeight( minHeight1 );
   connect( cb_dammin, SIGNAL( clicked() ), this, SLOT( set_dammin() ) );

   pb_dammin = new QPushButton( "DAMMIN", this );
   pb_dammin->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_dammin->setMinimumHeight(minHeight1);
   pb_dammin->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect( pb_dammin, SIGNAL( clicked() ), SLOT( dammin() ) );

   cb_dammif = new QCheckBox(this);
   cb_dammif->setText(tr(" Active "));
   cb_dammif->setChecked( options_active.count( "dammif" ) && options_active[ "dammif" ] );
   cb_dammif->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_dammif->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_dammif->setMinimumHeight( minHeight1 );
   connect( cb_dammif, SIGNAL( clicked() ), this, SLOT( set_dammif() ) );

   pb_dammif = new QPushButton( "DAMMIF", this );
   pb_dammif->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_dammif->setMinimumHeight(minHeight1);
   pb_dammif->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect( pb_dammif, SIGNAL( clicked() ), SLOT( dammif() ) );

   cb_gasbor = new QCheckBox(this);
   cb_gasbor->setText(tr(" Active "));
   cb_gasbor->setChecked( options_active.count( "gasbor" ) && options_active[ "gasbor" ] );
   cb_gasbor->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_gasbor->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_gasbor->setMinimumHeight( minHeight1 );
   connect( cb_gasbor, SIGNAL( clicked() ), this, SLOT( set_gasbor() ) );

   pb_gasbor = new QPushButton( "GASBOR", this );
   pb_gasbor->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_gasbor->setMinimumHeight(minHeight1);
   pb_gasbor->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect( pb_gasbor, SIGNAL( clicked() ), SLOT( gasbor() ) );

   pb_cancel = new QPushButton(tr("Cancel"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_ok = new QPushButton( tr("Close"), this);
   pb_ok->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_ok->setMinimumHeight(minHeight1);
   pb_ok->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_ok, SIGNAL(clicked()), SLOT(ok()));

   // build layout

   QGridLayout *gl_options = new QGridLayout( 0 );

   int j = 0;

   gl_options->addWidget( cb_bfnb  , j, 0 );
   gl_options->addWidget( pb_bfnb  , j, 1 );
   j++;

   gl_options->addWidget( cb_csa   , j, 0 );
   gl_options->addWidget( pb_csa   , j, 1 );
   j++;

   gl_options->addWidget( cb_dammin, j, 0 );
   gl_options->addWidget( pb_dammin, j, 1 );
   j++;

   gl_options->addWidget( cb_dammif, j, 0 );
   gl_options->addWidget( pb_dammif, j, 1 );
   j++;

   gl_options->addWidget( cb_gasbor, j, 0 );
   gl_options->addWidget( pb_gasbor, j, 1 );
   j++;

   QHBoxLayout *hbl_bottom = new QHBoxLayout(0);
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_cancel );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_ok );
   hbl_bottom->addSpacing( 4 );


   QVBoxLayout *background = new QVBoxLayout(this);
   background->addSpacing( 4 );
   background->addWidget ( lbl_title );
   background->addSpacing( 4 );
   background->addLayout ( gl_options );
   background->addSpacing( 4 );
   background->addLayout ( hbl_bottom );
   background->addSpacing( 4 );
}

void US_Hydrodyn_Cluster_Additional::ok()
{
   ( ( ( US_Hydrodyn * ) us_hydrodyn )->cluster_additional_methods_options_active   ) = options_active;
   ( ( ( US_Hydrodyn * ) us_hydrodyn )->cluster_additional_methods_options_selected ) = options_selected;
   close();
}

void US_Hydrodyn_Cluster_Additional::cancel()
{
   close();
}

void US_Hydrodyn_Cluster_Additional::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_cluster_additional.html");
}

void US_Hydrodyn_Cluster_Additional::closeEvent(QCloseEvent *e)
{
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Cluster_Additional::update_enables()
{
   pb_bfnb  ->setEnabled( cb_bfnb  ->isChecked() );
   pb_csa   ->setEnabled( cb_csa   ->isChecked() );
   pb_dammin->setEnabled( cb_dammin->isChecked() );
   pb_dammif->setEnabled( cb_dammif->isChecked() );
   pb_gasbor->setEnabled( cb_gasbor->isChecked() );
   
   // disabled for now:
   cb_bfnb  ->setEnabled( false );
   cb_csa   ->setEnabled( false );
   // cb_dammin->setEnabled( false );
   cb_dammif->setEnabled( false );
   cb_gasbor->setEnabled( false );
}

void US_Hydrodyn_Cluster_Additional::set_bfnb()
{
   options_active[ "bfnb" ] = cb_bfnb->isChecked();
   update_enables();
}

void US_Hydrodyn_Cluster_Additional::bfnb()
{
}

void US_Hydrodyn_Cluster_Additional::set_csa()
{
   options_active[ "csa" ] = cb_csa->isChecked();
   update_enables();
}

void US_Hydrodyn_Cluster_Additional::csa()
{
}

void US_Hydrodyn_Cluster_Additional::set_dammin()
{
   options_active[ "dammin" ] = cb_dammin->isChecked();
   update_enables();
}

void US_Hydrodyn_Cluster_Additional::dammin()
{
   map < QString, QString > parameters;
   if ( options_selected.count( "dammin" ) )
   {
      parameters = options_selected[ "dammin" ];
   }
   US_Hydrodyn_Cluster_Dammin *hc = 
      new US_Hydrodyn_Cluster_Dammin(
                                     us_hydrodyn,
                                     &parameters,
                                     this 
                                     );
   US_Hydrodyn::fixWinButtons( hc );
   hc->exec();
   delete hc;
   options_selected[ "dammin" ] = parameters;
}

void US_Hydrodyn_Cluster_Additional::set_dammif()
{
   options_active[ "dammif" ] = cb_dammif->isChecked();
   update_enables();
}

void US_Hydrodyn_Cluster_Additional::dammif()
{
}

void US_Hydrodyn_Cluster_Additional::set_gasbor()
{
   options_active[ "gasbor" ] = cb_gasbor->isChecked();
   update_enables();
}

void US_Hydrodyn_Cluster_Additional::gasbor()
{
}

