#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_saxs_hplc_gauss_mode.h"
#include "../include/us_hydrodyn_saxs_hplc.h"
//Added by qt3to4:
#include <QHBoxLayout>
#include <QCloseEvent>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>

US_Hydrodyn_Saxs_Hplc_Gauss_Mode::US_Hydrodyn_Saxs_Hplc_Gauss_Mode(
                                                             map < QString, QString > * parameters,
                                                             US_Hydrodyn              * us_hydrodyn,
                                                             QWidget *                  p
                                                             ) : QDialog( p )
{
   this->hplc_win    = p;
   this->parameters  = parameters;
   this->us_hydrodyn = us_hydrodyn;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle( us_tr( "US-SOMO: HPLC/KIN : Gaussian Options" ) );

   setupGUI();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );

   update_enables();
}

US_Hydrodyn_Saxs_Hplc_Gauss_Mode::~US_Hydrodyn_Saxs_Hplc_Gauss_Mode()
{
}

void US_Hydrodyn_Saxs_Hplc_Gauss_Mode::setupGUI()
{
   int minHeight1  = 30;

   lbl_gaussian_type = new QLabel ( us_tr( "Gaussian Options" ), this);
   lbl_gaussian_type->setAlignment( Qt::AlignCenter | Qt::AlignVCenter);
   lbl_gaussian_type->setPalette  ( PALET_FRAME );
   AUTFBACK( lbl_gaussian_type );
   lbl_gaussian_type->setFont     ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   rb_gauss = new QRadioButton( us_tr("Standard Gaussians"), this);
   rb_gauss->setEnabled(true);
   rb_gauss->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_gauss->setPalette( PALET_NORMAL );
   AUTFBACK( rb_gauss );

   rb_gmg = new QRadioButton( us_tr("GMG (Half-Gaussian modified Gaussian)"), this);
   rb_gmg->setEnabled(true);
   rb_gmg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_gmg->setPalette( PALET_NORMAL );
   AUTFBACK( rb_gmg );

   rb_emg = new QRadioButton( us_tr("EMG (Exponentially modified Gaussian)"), this);
   rb_emg->setEnabled(true);
   rb_emg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_emg->setPalette( PALET_NORMAL );
   AUTFBACK( rb_emg );

   rb_emggmg = new QRadioButton( us_tr("EMG+GMG"), this);
   rb_emggmg->setEnabled(true);
   rb_emggmg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_emggmg->setPalette( PALET_NORMAL );
   AUTFBACK( rb_emggmg );

   bg_gaussian_type = new QButtonGroup( this );
   int bg_pos = 0;
   bg_gaussian_type->setExclusive(true);
   bg_gaussian_type->addButton( rb_gauss, bg_pos++ );
   bg_gaussian_type->addButton( rb_gmg, bg_pos++ );
   bg_gaussian_type->addButton( rb_emg, bg_pos++ );
   bg_gaussian_type->addButton( rb_emggmg, bg_pos++ );

   pb_clear_gauss =  new QPushButton ( us_tr( "Clear cached Gaussian values" ), this );
   pb_clear_gauss -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_clear_gauss -> setMinimumHeight( minHeight1 );
   pb_clear_gauss -> setPalette      ( PALET_PUSHB );
   connect( pb_clear_gauss, SIGNAL( clicked() ), SLOT( clear_gauss() ) );

   pb_quit =  new QPushButton ( us_tr( "Quit" ), this );
   pb_quit -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_quit -> setMinimumHeight( minHeight1 );
   pb_quit -> setPalette      ( PALET_PUSHB );
   connect( pb_quit, SIGNAL( clicked() ), SLOT( quit() ) );

   pb_help =  new QPushButton ( us_tr( "Help" ), this );
   pb_help -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_help -> setMinimumHeight( minHeight1 );
   pb_help -> setPalette      ( PALET_PUSHB );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

   pb_ok =  new QPushButton ( us_tr( "Ok" ), this );
   pb_ok -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_ok -> setMinimumHeight( minHeight1 );
   pb_ok -> setPalette      ( PALET_PUSHB );
   connect( pb_ok, SIGNAL( clicked() ), SLOT( ok() ) );

   QVBoxLayout * background = new QVBoxLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );

   background->addWidget( lbl_gaussian_type );
   background->addWidget( rb_gauss );
   background->addWidget( rb_gmg );
   background->addWidget( rb_emg );
   background->addWidget( rb_emggmg );

   background->addWidget( pb_clear_gauss );

   if ( !parameters->count( "gaussian_type" ) )
   {
      rb_gauss->setChecked( true );
   } else {
      switch( (*parameters)[ "gaussian_type" ].toInt() )
      {
      case US_Hydrodyn_Saxs_Hplc::GMG :
         rb_gmg->setChecked( true );
         break;
      case US_Hydrodyn_Saxs_Hplc::EMG :
         rb_emg->setChecked( true );
         break;
      case US_Hydrodyn_Saxs_Hplc::EMGGMG :
         rb_emggmg->setChecked( true );
         break;
      case US_Hydrodyn_Saxs_Hplc::GAUSS :
      default :
         rb_gauss->setChecked( true );
         break;
      }
   }

   QHBoxLayout * hbl_bottom = new QHBoxLayout(); hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addWidget ( pb_quit );
   hbl_bottom->addWidget ( pb_ok );

   background->addLayout ( hbl_bottom );
}

void US_Hydrodyn_Saxs_Hplc_Gauss_Mode::quit()
{
   close();
}

void US_Hydrodyn_Saxs_Hplc_Gauss_Mode::ok()
{
   (*parameters)[ "ok" ] = "true";

   if ( rb_gauss->isChecked() )
   {
      (*parameters)[ "gaussian_type" ] = QString( "%1" ).arg( US_Hydrodyn_Saxs_Hplc::GAUSS );
   }
   if ( rb_gmg->isChecked() )
   {
      (*parameters)[ "gaussian_type" ] = QString( "%1" ).arg( US_Hydrodyn_Saxs_Hplc::GMG );
   }
   if ( rb_emg->isChecked() )
   {
      (*parameters)[ "gaussian_type" ] = QString( "%1" ).arg( US_Hydrodyn_Saxs_Hplc::EMG );
   }
   if ( rb_emggmg->isChecked() )
   {
      (*parameters)[ "gaussian_type" ] = QString( "%1" ).arg( US_Hydrodyn_Saxs_Hplc::EMGGMG );
   }

   close();
}

void US_Hydrodyn_Saxs_Hplc_Gauss_Mode::help()
{
   US_Help *online_help;
   online_help = new US_Help( this );
   online_help->show_help("manual/somo/saxs_hplc_gauss_mode.html");
}

void US_Hydrodyn_Saxs_Hplc_Gauss_Mode::closeEvent( QCloseEvent *e )
{

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Saxs_Hplc_Gauss_Mode::update_enables()
{
}

void US_Hydrodyn_Saxs_Hplc_Gauss_Mode::clear_gauss()
{
   if ( QMessageBox::Yes == QMessageBox::question(
                                                  this,
                                                  windowTitle() + us_tr( ": Clear cached Gaussian values" ),
                                                  us_tr("Are you sure you want to clear all cached Gaussian values?" ),
                                                  QMessageBox::Yes, 
                                                  QMessageBox::No | QMessageBox::Default
                                                  ) )
   {
      ((US_Hydrodyn_Saxs_Hplc *)hplc_win)->clear_gaussians();
   }
   update_enables();
}
