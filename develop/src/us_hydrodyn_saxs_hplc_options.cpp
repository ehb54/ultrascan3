#include "../include/us_hydrodyn_saxs_hplc_options.h"

US_Hydrodyn_Saxs_Hplc_Options::US_Hydrodyn_Saxs_Hplc_Options(
                                                             map < QString, QString > * parameters,
                                                             QWidget *                  p,
                                                             const char *               name
                                                             ) : QDialog( p, name )
{
   this->hplc_win   = hplc_win;
   this->parameters = parameters;

   USglobal = new US_Config();
   setPalette( QPalette( USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame ) );
   setCaption( tr( "US-SOMO: SAXS HPLC : Options" ) );

   setupGUI();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );
}

US_Hydrodyn_Saxs_Hplc_Options::~US_Hydrodyn_Saxs_Hplc_Options()
{
}

void US_Hydrodyn_Saxs_Hplc_Options::setupGUI()
{
   int minHeight1  = 30;

   lbl_title =  new QLabel      ( tr( "US-SOMO: SAXS HPLC : Options" ), this );
   lbl_title -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_title -> setMinimumHeight( minHeight1 );
   lbl_title -> setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_title -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold ) );

   lbl_gaussian_type = new QLabel ( tr( "Gaussian Mode" ), this);
   lbl_gaussian_type->setAlignment( Qt::AlignCenter | Qt::AlignVCenter);
   lbl_gaussian_type->setPalette  ( QPalette( USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame ) );
   lbl_gaussian_type->setFont     ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   rb_gauss = new QRadioButton( tr("Standard Gaussians"), this);
   rb_gauss->setEnabled(true);
   rb_gauss->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_gauss->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   rb_gmg = new QRadioButton( tr("GMG (Half-Gaussian modified Gaussian)"), this);
   rb_gmg->setEnabled(true);
   rb_gmg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_gmg->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   rb_emg = new QRadioButton( tr("EMG (Exponentially modified Gaussian)"), this);
   rb_emg->setEnabled(true);
   rb_emg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_emg->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   rb_emggmg = new QRadioButton( tr("EMG+GMG"), this);
   rb_emggmg->setEnabled(true);
   rb_emggmg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_emggmg->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   bg_gaussian_type = new QButtonGroup(1, Qt::Horizontal, 0);
   bg_gaussian_type->setRadioButtonExclusive(true);
   bg_gaussian_type->insert( rb_gauss );
   bg_gaussian_type->insert( rb_gmg );
   bg_gaussian_type->insert( rb_emg );
   bg_gaussian_type->insert( rb_emggmg );

   lbl_other_options = new QLabel ( tr( "Miscellaneous options" ), this);
   lbl_other_options->setAlignment( Qt::AlignCenter | Qt::AlignVCenter);
   lbl_other_options->setPalette  ( QPalette( USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame ) );
   lbl_other_options->setFont     ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   cb_csv_transposed = new QCheckBox(this);
   cb_csv_transposed->setText(tr( "Save CSV transposed"));
   cb_csv_transposed->setEnabled( true );
   cb_csv_transposed->setChecked( (*parameters)[ "hplc_csv_transposed" ] == "true" );
   cb_csv_transposed->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_csv_transposed->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   pb_quit =  new QPushButton ( tr( "Quit" ), this );
   pb_quit -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_quit -> setMinimumHeight( minHeight1 );
   pb_quit -> setPalette      ( QPalette( USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active ) );
   connect( pb_quit, SIGNAL( clicked() ), SLOT( quit() ) );

   pb_help =  new QPushButton ( tr( "Help" ), this );
   pb_help -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_help -> setMinimumHeight( minHeight1 );
   pb_help -> setPalette      ( QPalette( USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

   pb_ok =  new QPushButton ( tr( "Ok" ), this );
   pb_ok -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_ok -> setMinimumHeight( minHeight1 );
   pb_ok -> setPalette      ( QPalette( USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active ) );
   connect( pb_ok, SIGNAL( clicked() ), SLOT( ok() ) );

   QVBoxLayout *background = new QVBoxLayout( this );
   background->addWidget( lbl_title );
   background->addWidget( lbl_gaussian_type );
   background->addWidget( rb_gauss );
   background->addWidget( rb_gmg );
   background->addWidget( rb_emg );
   background->addWidget( rb_emggmg );
   background->addWidget( lbl_other_options );
   background->addWidget( cb_csv_transposed );

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

   QHBoxLayout *hbl_bottom = new QHBoxLayout( 0 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addWidget ( pb_quit );
   hbl_bottom->addWidget ( pb_ok );

   background->addLayout ( hbl_bottom );
}

void US_Hydrodyn_Saxs_Hplc_Options::quit()
{
   close();
}

void US_Hydrodyn_Saxs_Hplc_Options::ok()
{
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

   (*parameters)[ "hplc_csv_transposed" ] = cb_csv_transposed->isChecked() ? "true" : "false";
   close();
}

void US_Hydrodyn_Saxs_Hplc_Options::help()
{
   US_Help *online_help;
   online_help = new US_Help( this );
   online_help->show_help("manual/saxs_hplc_options.html");
}

void US_Hydrodyn_Saxs_Hplc_Options::closeEvent( QCloseEvent *e )
{

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

