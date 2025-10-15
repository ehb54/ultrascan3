#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_bd_load_results_opts.h"
#include "../include/us_math.h"
//Added by qt3to4:
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>

US_Hydrodyn_BD_Load_Results_Opts::US_Hydrodyn_BD_Load_Results_Opts(
                                                                   QString msg,
                                                                   double *temperature,
                                                                   double *solvent_viscosity,
                                                                   double *solvent_density,
                                                                   QString *solvent_name,
                                                                   QString *solvent_acronym,
                                                                   double *psv,
                                                                   double browflex_temperature,
                                                                   double browflex_solvent_viscosity,
                                                                   bool *check_fix_overlaps,
                                                                   QWidget *p,
                                                                   const char *
                                                                   ) : QDialog( p )
{
   this->msg = msg;
   this->temperature = temperature;
   this->solvent_viscosity = solvent_viscosity;
   this->solvent_density = solvent_density;
   this->solvent_name = solvent_name;
   this->solvent_acronym = solvent_acronym;
   this->psv = psv;
   this->browflex_temperature = browflex_temperature;
   this->browflex_solvent_viscosity = browflex_solvent_viscosity;
   this->check_fix_overlaps = check_fix_overlaps;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle("Set parameters for Browflex generated bead models");
   setupGUI();
   global_Xpos = 200;
   global_Ypos = 150;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_BD_Load_Results_Opts::~US_Hydrodyn_BD_Load_Results_Opts()
{
}

void US_Hydrodyn_BD_Load_Results_Opts::setupGUI()
{
   int minHeight1 = 30;
   int minHeight2 = 30;

   lbl_info = new QLabel(msg, this);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight2);
   lbl_info->setPalette( PALET_LABEL );
   AUTFBACK( lbl_info );
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   lbl_param = new QLabel(us_tr("Parameter  "), this);
   lbl_param->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_param->setMinimumHeight(minHeight2);
   lbl_param->setPalette( PALET_LABEL );
   AUTFBACK( lbl_param );
   lbl_param->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   lbl_browflex = new QLabel(us_tr("  From Browflex  "), this);
   lbl_browflex->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_browflex->setMinimumHeight(minHeight2);
   lbl_browflex->setPalette( PALET_LABEL );
   AUTFBACK( lbl_browflex );
   lbl_browflex->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   lbl_to_use = new QLabel(us_tr("To Use"), this);
   lbl_to_use->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_to_use->setMinimumHeight(minHeight2);
   lbl_to_use->setPalette( PALET_LABEL );
   AUTFBACK( lbl_to_use );
   lbl_to_use->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   lbl_solvent_name = new QLabel(us_tr(" Solvent:"), this);
   Q_CHECK_PTR(lbl_solvent_name);
   lbl_solvent_name->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_solvent_name->setPalette( PALET_LABEL );
   AUTFBACK( lbl_solvent_name );
   lbl_solvent_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_solvent_name = new QLineEdit( this );    le_solvent_name->setObjectName( "Solvent_Name Line Edit" );
   le_solvent_name->setText(*solvent_name);
   le_solvent_name->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
   le_solvent_name->setPalette( PALET_NORMAL );
   AUTFBACK( le_solvent_name );
   le_solvent_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_solvent_name->setEnabled(true);
   connect(le_solvent_name, SIGNAL(textChanged(const QString &)), SLOT(update_solvent_name(const QString &)));

   cb_solvent_defaults = new QCheckBox(this);
   cb_solvent_defaults->setText(us_tr("Set solvent to defaults"));
   cb_solvent_defaults->setEnabled(true);
   cb_solvent_defaults->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_solvent_defaults->setPalette( PALET_NORMAL );
   AUTFBACK( cb_solvent_defaults );
   connect(cb_solvent_defaults, SIGNAL(clicked()), this, SLOT(set_solvent_defaults()));

   cb_set_to_browflex = new QCheckBox(this);
   cb_set_to_browflex->setText("Set Browflex Defaults");
   cb_set_to_browflex->setEnabled(true);
   cb_set_to_browflex->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_set_to_browflex->setPalette( PALET_NORMAL );
   AUTFBACK( cb_set_to_browflex );
   connect(cb_set_to_browflex, SIGNAL(clicked()), this, SLOT(set_to_browflex()));

   cb_check_fix_overlaps = new QCheckBox(this);
   cb_check_fix_overlaps->setText(us_tr(" Check and fix overlaps "));
   cb_check_fix_overlaps->setEnabled(true);
   cb_check_fix_overlaps->setChecked(*check_fix_overlaps);
   cb_check_fix_overlaps->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_check_fix_overlaps->setPalette( PALET_NORMAL );
   AUTFBACK( cb_check_fix_overlaps );
   connect(cb_check_fix_overlaps, SIGNAL(clicked()), this, SLOT(set_check_fix_overlaps()));

   lbl_solvent_acronym = new QLabel(us_tr(" Solvent acronym:"), this);
   //   lbl_solvent_acronym = new QLabel(us_tr(" Solvent acronym (max 5 characters):"), this);
   Q_CHECK_PTR(lbl_solvent_acronym);
   lbl_solvent_acronym->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_solvent_acronym->setPalette( PALET_LABEL );
   AUTFBACK( lbl_solvent_acronym );
   lbl_solvent_acronym->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_solvent_acronym = new QLineEdit( this );    le_solvent_acronym->setObjectName( "Solvent_Acronym Line Edit" );
   le_solvent_acronym->setText(*solvent_acronym);
   le_solvent_acronym->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
   le_solvent_acronym->setPalette( PALET_NORMAL );
   AUTFBACK( le_solvent_acronym );
   le_solvent_acronym->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_solvent_acronym->setEnabled(true);
   connect(le_solvent_acronym, SIGNAL(textChanged(const QString &)), SLOT(update_solvent_acronym(const QString &)));

   lbl_temperature = new QLabel(us_tr( QString( " Temperature (%1C): " ).arg( DEGREE_SYMBOL ) ), this);
   Q_CHECK_PTR(lbl_temperature);
   lbl_temperature->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_temperature->setPalette( PALET_LABEL );
   AUTFBACK( lbl_temperature );
   lbl_temperature->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   lbl_browflex_temperature = new QLabel( QString::asprintf( "%4.2f",browflex_temperature ) , this);
   lbl_browflex_temperature->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
   lbl_browflex_temperature->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_browflex_temperature );
   lbl_browflex_temperature->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   le_temperature = new QLineEdit( this );    le_temperature->setObjectName( "Temperature Line Edit" );
   le_temperature->setText( QString::asprintf( "%4.2f",*temperature ) );
   le_temperature->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
   le_temperature->setPalette( PALET_NORMAL );
   AUTFBACK( le_temperature );
   le_temperature->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_temperature->setEnabled(true);
   connect(le_temperature, SIGNAL(textChanged(const QString &)), SLOT(update_temperature(const QString &)));

   lbl_solvent_viscosity = new QLabel(us_tr(" Solvent viscosity (cP):"), this);
   Q_CHECK_PTR(lbl_solvent_viscosity);
   lbl_solvent_viscosity->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_solvent_viscosity->setPalette( PALET_LABEL );
   AUTFBACK( lbl_solvent_viscosity );
   lbl_solvent_viscosity->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   lbl_browflex_solvent_viscosity = new QLabel( QString::asprintf( "%f",browflex_solvent_viscosity ) , this);
   lbl_browflex_solvent_viscosity->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
   lbl_browflex_solvent_viscosity->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_browflex_solvent_viscosity );
   lbl_browflex_solvent_viscosity->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   le_solvent_viscosity = new QLineEdit( this );    le_solvent_viscosity->setObjectName( "Solvent_Viscosity Line Edit" );
   le_solvent_viscosity->setText( QString::asprintf( "%f",*solvent_viscosity ) );
   le_solvent_viscosity->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
   le_solvent_viscosity->setPalette( PALET_NORMAL );
   AUTFBACK( le_solvent_viscosity );
   le_solvent_viscosity->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_solvent_viscosity->setEnabled(true);
   connect(le_solvent_viscosity, SIGNAL(textChanged(const QString &)), SLOT(update_solvent_viscosity(const QString &)));

   lbl_solvent_density = new QLabel(us_tr(" Solvent density (g/ml):"), this);
   Q_CHECK_PTR(lbl_solvent_density);
   lbl_solvent_density->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_solvent_density->setPalette( PALET_LABEL );
   AUTFBACK( lbl_solvent_density );
   lbl_solvent_density->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_solvent_density = new QLineEdit( this );    le_solvent_density->setObjectName( "Solvent_Density Line Edit" );
   le_solvent_density->setText( QString::asprintf( "%f",*solvent_density ) );
   le_solvent_density->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
   le_solvent_density->setPalette( PALET_NORMAL );
   AUTFBACK( le_solvent_density );
   le_solvent_density->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_solvent_density->setEnabled(true);
   connect(le_solvent_density, SIGNAL(textChanged(const QString &)), SLOT(update_solvent_density(const QString &)));

   lbl_psv = new QLabel(us_tr(" Enter a vbar value:"), this);
   lbl_psv->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
   lbl_psv->setPalette( PALET_LABEL );
   AUTFBACK( lbl_psv );
   lbl_psv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_psv = new QLineEdit( this );    le_psv->setObjectName( "psv Line Edit" );
   le_psv->setText( QString::asprintf( "%5.3f", *psv ) );
   le_psv->setReadOnly(false);
   le_psv->setMinimumWidth(100);
   le_psv->setMinimumHeight(minHeight2);
   le_psv->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_psv->setPalette( PALET_NORMAL );
   AUTFBACK( le_psv );
   le_psv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   connect(le_psv, SIGNAL(textChanged(const QString &)), SLOT(update_psv(const QString &)));

   pb_cancel = new QPushButton(us_tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton("Help", this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight2);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   int j = 0;

   QGridLayout * background = new QGridLayout(this); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );

   background->addWidget( lbl_info , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 2 ) - ( 0 ) );
   j++;
   background->addWidget(lbl_param, j, 0);
   background->addWidget(lbl_browflex, j, 1);
   background->addWidget(lbl_to_use, j, 2);
   j++;
   background->addWidget(lbl_solvent_name, j, 0);
   background->addWidget(le_solvent_name, j, 2);
   j++;
   background->addWidget(lbl_solvent_acronym, j, 0);
   background->addWidget(le_solvent_acronym, j, 2);
   j++;
   background->addWidget(lbl_solvent_viscosity, j, 0);
   background->addWidget(lbl_browflex_solvent_viscosity, j, 1);
   background->addWidget(le_solvent_viscosity, j, 2);
   j++;
   background->addWidget(lbl_solvent_density, j, 0);
   background->addWidget(le_solvent_density, j, 2);
   j++;
   background->addWidget(lbl_temperature, j, 0);
   background->addWidget(lbl_browflex_temperature, j, 1);
   background->addWidget(le_temperature, j, 2);
   j++;
   background->addWidget(lbl_psv, j, 0);
   background->addWidget(le_psv, j, 2);
   j++;
   background->addWidget(cb_check_fix_overlaps, j, 0);
   background->addWidget(cb_set_to_browflex, j, 1);
   background->addWidget(cb_solvent_defaults, j, 2);
   j++;

   QHBoxLayout * hbl = new QHBoxLayout; hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
   hbl->addWidget(pb_help);
   hbl->addWidget(pb_cancel);
   background->addLayout( hbl , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 2 ) - ( 0 ) );
   check_solvent_browflex_defaults();
}

void US_Hydrodyn_BD_Load_Results_Opts::cancel()
{
   close();
}

void US_Hydrodyn_BD_Load_Results_Opts::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo/somo_bd_load_results_opts.html");
}

void US_Hydrodyn_BD_Load_Results_Opts::update_solvent_name(const QString &str)
{
   *solvent_name = str;
   check_solvent_browflex_defaults();
}

void US_Hydrodyn_BD_Load_Results_Opts::update_solvent_acronym(const QString &str)
{
   *solvent_acronym = str.left(5);
   le_solvent_acronym->setText(*solvent_acronym);
   check_solvent_browflex_defaults();
}

void US_Hydrodyn_BD_Load_Results_Opts::update_temperature(const QString &str)
{
   *temperature = str.toDouble();
   check_solvent_browflex_defaults();
}

void US_Hydrodyn_BD_Load_Results_Opts::update_solvent_viscosity(const QString &str)
{
   *solvent_viscosity = str.toDouble();
   // le_solvent_viscosity->setText( QString::asprintf( "%f",*solvent_viscosity ) );
   check_solvent_browflex_defaults();
}

void US_Hydrodyn_BD_Load_Results_Opts::update_solvent_density(const QString &str)
{
   *solvent_density = str.toDouble();
   // le_solvent_density->setText( QString::asprintf( "%f",*solvent_density ) );
   check_solvent_browflex_defaults();
}

void US_Hydrodyn_BD_Load_Results_Opts::update_psv(const QString &str)
{
   *psv = str.toDouble();
}

void US_Hydrodyn_BD_Load_Results_Opts::set_solvent_defaults()
{
   if ( cb_solvent_defaults->isChecked() )
   {
      *solvent_name = "Water";
      *solvent_acronym = "w";
      *temperature = K20 - K0;
      *solvent_viscosity = VISC_20W * 100;
      *solvent_density = DENS_20W;
      le_solvent_name->setText(*solvent_name);
      le_solvent_acronym->setText(*solvent_acronym);
      le_temperature->setText( QString::asprintf( "%4.2f",*temperature ) );
      le_solvent_viscosity->setText( QString::asprintf( "%f",*solvent_viscosity ) );
      le_solvent_density->setText( QString::asprintf( "%f",*solvent_density ) );
   }
}

void US_Hydrodyn_BD_Load_Results_Opts::set_check_fix_overlaps()
{
   *check_fix_overlaps = cb_check_fix_overlaps->isChecked();
}

void US_Hydrodyn_BD_Load_Results_Opts::set_to_browflex()
{
   if ( cb_set_to_browflex->isChecked() )
   {
      *temperature = browflex_temperature;
      *solvent_viscosity = browflex_solvent_viscosity;
      le_temperature->setText( QString::asprintf( "%4.2f",*temperature ) );
      le_solvent_viscosity->setText( QString::asprintf( "%f",*solvent_viscosity ) );
   }
   check_solvent_browflex_defaults();
}

void US_Hydrodyn_BD_Load_Results_Opts::check_solvent_browflex_defaults()
{
   cb_solvent_defaults->setChecked(
                                   *solvent_name == "Water" &&
                                   *solvent_acronym == "w" &&
                                   *temperature == K20 - K0 &&
                                   *solvent_viscosity == VISC_20W * 100 &&
                                   *solvent_density == DENS_20W
                                   );
   cb_set_to_browflex->setChecked(
                                   QString::asprintf( "%4.2f",*temperature )  == 
                                   QString::asprintf( "%4.2f",browflex_temperature )  &&
                                   QString::asprintf( "%f",*solvent_viscosity )  == 
                                   QString::asprintf( "%f",browflex_solvent_viscosity ) 
                                  );
}
      
int US_Hydrodyn_BD_Load_Results_Opts::write_conditions_to_file( QString /* filename */ )
{
   return -1;
}

void US_Hydrodyn_BD_Load_Results_Opts::read_conditions_from_file( QString /* filename */ )
{
}
