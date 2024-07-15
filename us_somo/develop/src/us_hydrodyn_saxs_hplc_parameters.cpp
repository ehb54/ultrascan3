#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_saxs_hplc_parameters.h"
//Added by qt3to4:
#include <QGridLayout>
#include <QTextStream>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QCloseEvent>

US_Hydrodyn_Saxs_Hplc_Parameters::US_Hydrodyn_Saxs_Hplc_Parameters(
                                                       void                     *              us_hydrodyn_saxs_hplc,
                                                       map < QString, QString > *              parameters,
                                                       QWidget *                               p,
                                                       const char *                            
                                                       ) : QDialog( p )
{
   this->us_hydrodyn_saxs_hplc                = us_hydrodyn_saxs_hplc;
   this->parameters                           = parameters;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle( us_tr( "US-SOMO: HPLC/KIN : SAXS Processing Parameters" ) );

   setupGUI();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );
}

US_Hydrodyn_Saxs_Hplc_Parameters::~US_Hydrodyn_Saxs_Hplc_Parameters()
{
}

void US_Hydrodyn_Saxs_Hplc_Parameters::setupGUI()
{
   int minHeight1  = 30;

   lbl_title =  new QLabel      ( us_tr( "US-SOMO: HPLC/KIN : SAXS Processing Parameters" ), this );
   lbl_title -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_title -> setMinimumHeight( minHeight1 );
   lbl_title -> setPalette( PALET_LABEL );
   AUTFBACK( lbl_title );
   lbl_title -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold ) );

   lbl_saxs_hplc_param_frame_interval = new QLabel( us_tr( "Frame Interval [s] :" ), this );
   lbl_saxs_hplc_param_frame_interval->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_saxs_hplc_param_frame_interval->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_saxs_hplc_param_frame_interval );
   lbl_saxs_hplc_param_frame_interval->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_saxs_hplc_param_frame_interval-> setMinimumHeight( minHeight1 );
   
   le_saxs_hplc_param_frame_interval = new QLineEdit( this );    le_saxs_hplc_param_frame_interval->setObjectName( "le_saxs_hplc_param_frame_interval Line Edit" );
   le_saxs_hplc_param_frame_interval->setText( (*parameters)[ "saxs_hplc_param_frame_interval" ] );
   le_saxs_hplc_param_frame_interval->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_saxs_hplc_param_frame_interval->setPalette( PALET_NORMAL );
   AUTFBACK( le_saxs_hplc_param_frame_interval );
   le_saxs_hplc_param_frame_interval->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 0.1, 600, 1, le_saxs_hplc_param_frame_interval );
      le_saxs_hplc_param_frame_interval->setValidator( qdv );
   }
   connect( le_saxs_hplc_param_frame_interval, SIGNAL( textChanged( const QString & ) ), SLOT( set_saxs_hplc_param_frame_interval( const QString & ) ) );
   le_saxs_hplc_param_frame_interval->setMinimumWidth( 200 );
   le_saxs_hplc_param_frame_interval-> setMinimumHeight( minHeight1 );

   lbl_saxs_hplc_param_g_conc = new QLabel( us_tr( "Default concentration [mg/mL] :" ), this );
   lbl_saxs_hplc_param_g_conc->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_saxs_hplc_param_g_conc->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_saxs_hplc_param_g_conc );
   lbl_saxs_hplc_param_g_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_saxs_hplc_param_g_conc-> setMinimumHeight( minHeight1 );
   
   le_saxs_hplc_param_g_conc = new QLineEdit( this );    le_saxs_hplc_param_g_conc->setObjectName( "le_saxs_hplc_param_g_conc Line Edit" );
   le_saxs_hplc_param_g_conc->setText( (*parameters)[ "saxs_hplc_param_g_conc" ] );
   le_saxs_hplc_param_g_conc->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_saxs_hplc_param_g_conc->setPalette( PALET_NORMAL );
   AUTFBACK( le_saxs_hplc_param_g_conc );
   le_saxs_hplc_param_g_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // {
   //    QDoubleValidator *qdv = new QDoubleValidator( 400, 600, 1, le_saxs_hplc_param_g_conc );
   //    le_saxs_hplc_param_g_conc->setValidator( qdv );
   // }
   connect( le_saxs_hplc_param_g_conc, SIGNAL( textChanged( const QString & ) ), SLOT( set_saxs_hplc_param_g_conc( const QString & ) ) );
   le_saxs_hplc_param_g_conc->setMinimumWidth( 200 );
   le_saxs_hplc_param_g_conc-> setMinimumHeight( minHeight1 );

   lbl_saxs_hplc_param_g_psv = new QLabel( us_tr( "Default partial specific volume [mL/g] :" ), this );
   lbl_saxs_hplc_param_g_psv->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_saxs_hplc_param_g_psv->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_saxs_hplc_param_g_psv );
   lbl_saxs_hplc_param_g_psv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_saxs_hplc_param_g_psv-> setMinimumHeight( minHeight1 );

   le_saxs_hplc_param_g_psv = new QLineEdit( this );    le_saxs_hplc_param_g_psv->setObjectName( "le_saxs_hplc_param_g_psv Line Edit" );
   le_saxs_hplc_param_g_psv->setText( (*parameters)[ "saxs_hplc_param_g_psv" ] );
   le_saxs_hplc_param_g_psv->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_saxs_hplc_param_g_psv->setPalette( PALET_NORMAL );
   AUTFBACK( le_saxs_hplc_param_g_psv );
   le_saxs_hplc_param_g_psv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // {
   //    QDoubleValidator *qdv = new QDoubleValidator( 400, 600, 1, le_saxs_hplc_param_g_psv );
   //    le_saxs_hplc_param_g_psv->setValidator( qdv );
   // }
   connect( le_saxs_hplc_param_g_psv, SIGNAL( textChanged( const QString & ) ), SLOT( set_saxs_hplc_param_g_psv( const QString & ) ) );
   le_saxs_hplc_param_g_psv->setMinimumWidth( 200 );
   le_saxs_hplc_param_g_psv-> setMinimumHeight( minHeight1 );

   lbl_saxs_hplc_param_I0_exp = new QLabel( us_tr( "Default I0 standard experimental [a.u.] :" ), this );
   lbl_saxs_hplc_param_I0_exp->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_saxs_hplc_param_I0_exp->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_saxs_hplc_param_I0_exp );
   lbl_saxs_hplc_param_I0_exp->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_saxs_hplc_param_I0_exp-> setMinimumHeight( minHeight1 );
   
   le_saxs_hplc_param_I0_exp = new QLineEdit( this );    le_saxs_hplc_param_I0_exp->setObjectName( "le_saxs_hplc_param_I0_exp Line Edit" );
   le_saxs_hplc_param_I0_exp->setText( (*parameters)[ "saxs_hplc_param_I0_exp" ] );
   le_saxs_hplc_param_I0_exp->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_saxs_hplc_param_I0_exp->setPalette( PALET_NORMAL );
   AUTFBACK( le_saxs_hplc_param_I0_exp );
   le_saxs_hplc_param_I0_exp->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 400, 600, 1, le_saxs_hplc_param_I0_exp );
      le_saxs_hplc_param_I0_exp->setValidator( qdv );
   }
   connect( le_saxs_hplc_param_I0_exp, SIGNAL( textChanged( const QString & ) ), SLOT( set_saxs_hplc_param_I0_exp( const QString & ) ) );
   le_saxs_hplc_param_I0_exp->setMinimumWidth( 200 );
   le_saxs_hplc_param_I0_exp-> setMinimumHeight( minHeight1 );

   lbl_saxs_hplc_param_I0_theo = new QLabel( us_tr( "I0 standard theoretical [cm^-1] :" ), this );
   lbl_saxs_hplc_param_I0_theo->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_saxs_hplc_param_I0_theo->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_saxs_hplc_param_I0_theo );
   lbl_saxs_hplc_param_I0_theo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_saxs_hplc_param_I0_theo-> setMinimumHeight( minHeight1 );
   
   le_saxs_hplc_param_I0_theo = new QLineEdit( this );    le_saxs_hplc_param_I0_theo->setObjectName( "le_saxs_hplc_param_I0_theo Line Edit" );
   le_saxs_hplc_param_I0_theo->setText( (*parameters)[ "saxs_hplc_param_I0_theo" ] );
   le_saxs_hplc_param_I0_theo->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_saxs_hplc_param_I0_theo->setPalette( PALET_NORMAL );
   AUTFBACK( le_saxs_hplc_param_I0_theo );
   le_saxs_hplc_param_I0_theo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // {
   //    QDoubleValidator *qdv = new QDoubleValidator( 400, 600, 1, le_saxs_hplc_param_I0_theo );
   //    le_saxs_hplc_param_I0_theo->setValidator( qdv );
   // }
   connect( le_saxs_hplc_param_I0_theo, SIGNAL( textChanged( const QString & ) ), SLOT( set_saxs_hplc_param_I0_theo( const QString & ) ) );
   le_saxs_hplc_param_I0_theo->setMinimumWidth( 200 );
   le_saxs_hplc_param_I0_theo-> setMinimumHeight( minHeight1 );

   lbl_saxs_hplc_param_diffusion_len = new QLabel( us_tr( "Diffusion length [cm] :" ), this );
   lbl_saxs_hplc_param_diffusion_len->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_saxs_hplc_param_diffusion_len->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_saxs_hplc_param_diffusion_len );
   lbl_saxs_hplc_param_diffusion_len->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_saxs_hplc_param_diffusion_len-> setMinimumHeight( minHeight1 );
   
   le_saxs_hplc_param_diffusion_len = new QLineEdit( this );    le_saxs_hplc_param_diffusion_len->setObjectName( "le_saxs_hplc_param_diffusion_len Line Edit" );
   le_saxs_hplc_param_diffusion_len->setText( (*parameters)[ "saxs_hplc_param_diffusion_len" ] );
   le_saxs_hplc_param_diffusion_len->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_saxs_hplc_param_diffusion_len->setPalette( PALET_NORMAL );
   AUTFBACK( le_saxs_hplc_param_diffusion_len );
   le_saxs_hplc_param_diffusion_len->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // {
   //    QDoubleValidator *qdv = new QDoubleValidator( 400, 600, 1, le_saxs_hplc_param_diffusion_len );
   //    le_saxs_hplc_param_diffusion_len->setValidator( qdv );
   // }
   connect( le_saxs_hplc_param_diffusion_len, SIGNAL( textChanged( const QString & ) ), SLOT( set_saxs_hplc_param_diffusion_len( const QString & ) ) );
   le_saxs_hplc_param_diffusion_len->setMinimumWidth( 200 );
   le_saxs_hplc_param_diffusion_len-> setMinimumHeight( minHeight1 );

   lbl_saxs_hplc_param_electron_nucleon_ratio = new QLabel( us_tr( "Electron/nucleon ratio Z/A :" ), this );
   lbl_saxs_hplc_param_electron_nucleon_ratio->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_saxs_hplc_param_electron_nucleon_ratio->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_saxs_hplc_param_electron_nucleon_ratio );
   lbl_saxs_hplc_param_electron_nucleon_ratio->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_saxs_hplc_param_electron_nucleon_ratio-> setMinimumHeight( minHeight1 );
   
   le_saxs_hplc_param_electron_nucleon_ratio = new QLineEdit( this );    le_saxs_hplc_param_electron_nucleon_ratio->setObjectName( "le_saxs_hplc_param_electron_nucleon_ratio Line Edit" );
   le_saxs_hplc_param_electron_nucleon_ratio->setText( (*parameters)[ "saxs_hplc_param_electron_nucleon_ratio" ] );
   le_saxs_hplc_param_electron_nucleon_ratio->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_saxs_hplc_param_electron_nucleon_ratio->setPalette( PALET_NORMAL );
   AUTFBACK( le_saxs_hplc_param_electron_nucleon_ratio );
   le_saxs_hplc_param_electron_nucleon_ratio->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // {
   //    QDoubleValidator *qdv = new QDoubleValidator( 400, 600, 1, le_saxs_hplc_param_electron_nucleon_ratio );
   //    le_saxs_hplc_param_electron_nucleon_ratio->setValidator( qdv );
   // }
   connect( le_saxs_hplc_param_electron_nucleon_ratio, SIGNAL( textChanged( const QString & ) ), SLOT( set_saxs_hplc_param_electron_nucleon_ratio( const QString & ) ) );
   le_saxs_hplc_param_electron_nucleon_ratio->setMinimumWidth( 200 );
   le_saxs_hplc_param_electron_nucleon_ratio-> setMinimumHeight( minHeight1 );

   lbl_saxs_hplc_param_nucleon_mass = new QLabel( us_tr( "Nucleon mass [g] :" ), this );
   lbl_saxs_hplc_param_nucleon_mass->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_saxs_hplc_param_nucleon_mass->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_saxs_hplc_param_nucleon_mass );
   lbl_saxs_hplc_param_nucleon_mass->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_saxs_hplc_param_nucleon_mass-> setMinimumHeight( minHeight1 );
   
   le_saxs_hplc_param_nucleon_mass = new QLineEdit( this );    le_saxs_hplc_param_nucleon_mass->setObjectName( "le_saxs_hplc_param_nucleon_mass Line Edit" );
   le_saxs_hplc_param_nucleon_mass->setText( (*parameters)[ "saxs_hplc_param_nucleon_mass" ] );
   le_saxs_hplc_param_nucleon_mass->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_saxs_hplc_param_nucleon_mass->setPalette( PALET_NORMAL );
   AUTFBACK( le_saxs_hplc_param_nucleon_mass );
   le_saxs_hplc_param_nucleon_mass->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // {
   //    QDoubleValidator *qdv = new QDoubleValidator( 400, 600, 1, le_saxs_hplc_param_nucleon_mass );
   //    le_saxs_hplc_param_nucleon_mass->setValidator( qdv );
   // }
   connect( le_saxs_hplc_param_nucleon_mass, SIGNAL( textChanged( const QString & ) ), SLOT( set_saxs_hplc_param_nucleon_mass( const QString & ) ) );
   le_saxs_hplc_param_nucleon_mass->setMinimumWidth( 200 );
   le_saxs_hplc_param_nucleon_mass-> setMinimumHeight( minHeight1 );

   lbl_saxs_hplc_param_solvent_electron_density = new QLabel( us_tr( "Solvent electron density [e A^-3] :" ), this );
   lbl_saxs_hplc_param_solvent_electron_density->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_saxs_hplc_param_solvent_electron_density->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_saxs_hplc_param_solvent_electron_density );
   lbl_saxs_hplc_param_solvent_electron_density->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_saxs_hplc_param_solvent_electron_density-> setMinimumHeight( minHeight1 );
   
   le_saxs_hplc_param_solvent_electron_density = new QLineEdit( this );    le_saxs_hplc_param_solvent_electron_density->setObjectName( "le_saxs_hplc_param_solvent_electron_density Line Edit" );
   le_saxs_hplc_param_solvent_electron_density->setText( (*parameters)[ "saxs_hplc_param_solvent_electron_density" ] );
   le_saxs_hplc_param_solvent_electron_density->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_saxs_hplc_param_solvent_electron_density->setPalette( PALET_NORMAL );
   AUTFBACK( le_saxs_hplc_param_solvent_electron_density );
   le_saxs_hplc_param_solvent_electron_density->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // {
   //    QDoubleValidator *qdv = new QDoubleValidator( 400, 600, 1, le_saxs_hplc_param_solvent_electron_density );
   //    le_saxs_hplc_param_solvent_electron_density->setValidator( qdv );
   // }
   connect( le_saxs_hplc_param_solvent_electron_density, SIGNAL( textChanged( const QString & ) ), SLOT( set_saxs_hplc_param_solvent_electron_density( const QString & ) ) );
   le_saxs_hplc_param_solvent_electron_density->setMinimumWidth( 200 );
   le_saxs_hplc_param_solvent_electron_density-> setMinimumHeight( minHeight1 );
   
   pb_help =  new QPushButton ( us_tr( "Help" ), this );
   pb_help -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_help -> setMinimumHeight( minHeight1 );
   pb_help -> setPalette      ( PALET_PUSHB );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

   pb_quit =  new QPushButton ( us_tr( "Quit" ), this );
   pb_quit -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_quit -> setMinimumHeight( minHeight1 );
   pb_quit -> setPalette      ( PALET_PUSHB );
   connect( pb_quit, SIGNAL( clicked() ), SLOT( quit() ) );

   pb_save =  new QPushButton ( us_tr( "Save" ), this );
   pb_save -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_save -> setMinimumHeight( minHeight1 );
   pb_save -> setPalette      ( PALET_PUSHB );
   connect( pb_save, SIGNAL( clicked() ), SLOT( save() ) );

   pb_keep =  new QPushButton ( us_tr( "Keep" ), this );
   pb_keep -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_keep -> setMinimumHeight( minHeight1 );
   pb_keep -> setPalette      ( PALET_PUSHB );
   connect( pb_keep, SIGNAL( clicked() ), SLOT( keep() ) );

   QVBoxLayout * background = new QVBoxLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );
   background->addSpacing(4);

   background->addWidget( lbl_title );
   background->addSpacing( 2 );

   QGridLayout * gbl = new QGridLayout( 0 ); gbl->setContentsMargins( 0, 0, 0, 0 ); gbl->setSpacing( 0 );

   {
      int row = 0;
      
      gbl->addWidget( lbl_saxs_hplc_param_frame_interval, row, 0 );
      gbl->addWidget( le_saxs_hplc_param_frame_interval,  row, 1 );
      ++row;

      gbl->addWidget( lbl_saxs_hplc_param_g_conc, row, 0 );
      gbl->addWidget( le_saxs_hplc_param_g_conc,  row, 1 );
      ++row;

      gbl->addWidget( lbl_saxs_hplc_param_g_psv, row, 0 );
      gbl->addWidget( le_saxs_hplc_param_g_psv,  row, 1 );
      ++row;

      gbl->addWidget( lbl_saxs_hplc_param_I0_exp, row, 0 );
      gbl->addWidget( le_saxs_hplc_param_I0_exp,  row, 1 );
      ++row;

      gbl->addWidget( lbl_saxs_hplc_param_I0_theo, row, 0 );
      gbl->addWidget( le_saxs_hplc_param_I0_theo,  row, 1 );
      ++row;

      gbl->addWidget( lbl_saxs_hplc_param_diffusion_len, row, 0 );
      gbl->addWidget( le_saxs_hplc_param_diffusion_len,  row, 1 );
      ++row;

      gbl->addWidget( lbl_saxs_hplc_param_electron_nucleon_ratio, row, 0 );
      gbl->addWidget( le_saxs_hplc_param_electron_nucleon_ratio,  row, 1 );
      ++row;

      gbl->addWidget( lbl_saxs_hplc_param_nucleon_mass, row, 0 );
      gbl->addWidget( le_saxs_hplc_param_nucleon_mass,  row, 1 );
      ++row;

      gbl->addWidget( lbl_saxs_hplc_param_solvent_electron_density, row, 0 );
      gbl->addWidget( le_saxs_hplc_param_solvent_electron_density,  row, 1 );
      ++row;
   }

   background->addLayout( gbl );

   QHBoxLayout * hbl_bottom = new QHBoxLayout(); hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addWidget ( pb_quit );
   hbl_bottom->addWidget ( pb_save );
   hbl_bottom->addWidget ( pb_keep );

   background->addLayout ( hbl_bottom );
}

void US_Hydrodyn_Saxs_Hplc_Parameters::quit()
{
   close();
}

void US_Hydrodyn_Saxs_Hplc_Parameters::save()
{

   // QString use_dir = 
   //    USglobal->config_list.root_dir + QDir::separator() + "etc" + 
   //    QDir::separator() + "somo_saxs_hplc_default_saxs_param.dat" ;

   // QString fn = QFileDialog::getSaveFileName( this , us_tr( "Select a name to save the SAXS parameters" ) , use_dir , "*_saxs_hplc_param.dat" );

   // if ( fn.isEmpty() )
   // {
   //    return;
   // }

   // if ( fn.isEmpty() )
   // {
   //    return;
   // }

   // fn.replace( QRegExp( "(|_saxs_hplc_param)\\.(dat|DAT)$" ), "" );
   // fn += "_saxs_hplc_param.dat";

   // QFile f( fn );
   // if ( !f.open( QIODevice::WriteOnly ) )
   // {
   //    return;
   // }

   // QTextStream ts( &f );

   // ts << "# US-SOMO SAXS_HPLC parameter file\n";

   // if ( !le_saxs_hplc_param_g_conc->text().isEmpty() ) {
   //    ts << "# __saxs_hplc_param_g_conc: " << le_saxs_hplc_param_g_conc->text() << Qt::endl;
   // }

   // if ( !le_saxs_hplc_param_g_psv->text().isEmpty() ) {
   //    ts << "# __saxs_hplc_param_g_psv: " << le_saxs_hplc_param_g_psv->text() << Qt::endl;
   // }

   // if ( !le_saxs_hplc_param_I0_exp->text().isEmpty() ) {
   //    ts << "# __saxs_hplc_param_I0_exp: " << le_saxs_hplc_param_I0_exp->text() << Qt::endl;
   // }

   // if ( !le_saxs_hplc_param_I0_theo->text().isEmpty() ) {
   //    ts << "# __saxs_hplc_param_I0_theo: " << le_saxs_hplc_param_I0_theo->text() << Qt::endl;
   // }

   // if ( !le_saxs_hplc_param_diffusion_len->text().isEmpty() ) {
   //    ts << "# __saxs_hplc_param_diffusion_len: " << le_saxs_hplc_param_diffusion_len->text() << Qt::endl;
   // }

   // if ( !le_saxs_hplc_param_electron_nucleon_ratio->text().isEmpty() ) {
   //    ts << "# __saxs_hplc_param_electron_nucleon_ratio: " << le_saxs_hplc_param_electron_nucleon_ratio->text() << Qt::endl;
   // }

   // if ( !le_saxs_hplc_param_nucleon_mass->text().isEmpty() ) {
   //    ts << "# __saxs_hplc_param_nucleon_mass: " << le_saxs_hplc_param_nucleon_mass->text() << Qt::endl;
   // }

   // if ( !le_saxs_hplc_param_solvent_electron_density->text().isEmpty() ) {
   //    ts << "# __saxs_hplc_param_solvent_electron_density: " << le_saxs_hplc_param_solvent_electron_density->text() << Qt::endl;
   // }

   // f.close();

   // save to disk
   (*parameters)[ "save" ] = "set";
   keep();
}

void US_Hydrodyn_Saxs_Hplc_Parameters::keep()
{
   (*parameters)[ "keep" ] = "set";
   close();
}

void US_Hydrodyn_Saxs_Hplc_Parameters::help()
{
   US_Help *online_help;
   online_help = new US_Help( this );
   online_help->show_help("manual/somo/saxs_hplc_parameters.html");
}

void US_Hydrodyn_Saxs_Hplc_Parameters::closeEvent( QCloseEvent *e )
{

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Saxs_Hplc_Parameters::set_saxs_hplc_param_frame_interval( const QString & text )
{
   (*parameters)[ "saxs_hplc_param_frame_interval" ] = text;
}

void US_Hydrodyn_Saxs_Hplc_Parameters::set_saxs_hplc_param_g_conc( const QString & text )
{
   (*parameters)[ "saxs_hplc_param_g_conc" ] = text;
}

void US_Hydrodyn_Saxs_Hplc_Parameters::set_saxs_hplc_param_g_psv( const QString & text )
{
   (*parameters)[ "saxs_hplc_param_g_psv" ] = text;
}

void US_Hydrodyn_Saxs_Hplc_Parameters::set_saxs_hplc_param_I0_exp( const QString & text )
{
   (*parameters)[ "saxs_hplc_param_I0_exp" ] = text;
}

void US_Hydrodyn_Saxs_Hplc_Parameters::set_saxs_hplc_param_I0_theo( const QString & text )
{
   (*parameters)[ "saxs_hplc_param_I0_theo" ] = text;
}

void US_Hydrodyn_Saxs_Hplc_Parameters::set_saxs_hplc_param_diffusion_len( const QString & text )
{
   (*parameters)[ "saxs_hplc_param_diffusion_len" ] = text;
}

void US_Hydrodyn_Saxs_Hplc_Parameters::set_saxs_hplc_param_electron_nucleon_ratio( const QString & text )
{
   (*parameters)[ "saxs_hplc_param_electron_nucleon_ratio" ] = text;
}

void US_Hydrodyn_Saxs_Hplc_Parameters::set_saxs_hplc_param_nucleon_mass( const QString & text )
{
   (*parameters)[ "saxs_hplc_param_nucleon_mass" ] = text;
}

void US_Hydrodyn_Saxs_Hplc_Parameters::set_saxs_hplc_param_solvent_electron_density( const QString & text )
{
   (*parameters)[ "saxs_hplc_param_solvent_electron_density" ] = text;
}

