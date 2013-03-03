#include "../include/us_hydrodyn_saxs_hplc_ciq.h"

US_Hydrodyn_Saxs_Hplc_Ciq::US_Hydrodyn_Saxs_Hplc_Ciq(
                                                     void                     *              us_hydrodyn_saxs_hplc,
                                                     map < QString, QString > *              parameters,
                                                     QWidget *                               p,
                                                     const char *                            name
                                                     ) : QDialog( p, name )
{
   this->us_hydrodyn_saxs_hplc                = us_hydrodyn_saxs_hplc;
   this->parameters                           = parameters;

   USglobal = new US_Config();
   setPalette( QPalette( USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame ) );
   setCaption( tr( "US-SOMO: SAXS HPLC : Make I(q)" ) );

   setupGUI();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );
}

US_Hydrodyn_Saxs_Hplc_Ciq::~US_Hydrodyn_Saxs_Hplc_Ciq()
{
}

void US_Hydrodyn_Saxs_Hplc_Ciq::setupGUI()
{
   int minHeight1  = 30;

   lbl_title =  new QLabel      ( tr( "US-SOMO: SAXS HPLC : Make I(q)" ), this );
   // lbl_title -> setFrameStyle   ( QFrame::WinPanel | QFrame::Raised );
   lbl_title -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_title -> setMinimumHeight( minHeight1 );
   lbl_title ->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   // lbl_title -> setPalette      ( QPalette( USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame ) );
   lbl_title -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold ) );
   // lbl_title -> hide();

   cb_add_bl = new QCheckBox(this);
   cb_add_bl->setText( parameters->count( "baseline" ) ? (*parameters)[ "baseline" ] : "" );
   cb_add_bl->setEnabled( true );
   connect( cb_add_bl, SIGNAL( clicked() ), SLOT( set_add_bl() ) );
   cb_add_bl->setChecked( false );
   cb_add_bl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_add_bl->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   if ( !parameters->count( "baseline" ) )
   {
      cb_add_bl->hide();
   }

   cb_save_as_pct_iq = new QCheckBox(this);
   cb_save_as_pct_iq->setText(tr( "Resulting I(q) created as a percent of the original I(q) ( if unchecked, I(q) will be created from the Gaussians )" ) );
   cb_save_as_pct_iq->setEnabled( true );
   connect( cb_save_as_pct_iq, SIGNAL( clicked() ), SLOT( set_save_as_pct_iq() ) );
   cb_save_as_pct_iq->setChecked( false );
   cb_save_as_pct_iq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_save_as_pct_iq->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));


   pb_help =  new QPushButton ( tr( "Help" ), this );
   pb_help -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_help -> setMinimumHeight( minHeight1 );
   pb_help -> setPalette      ( QPalette( USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

   pb_quit =  new QPushButton ( tr( "Quit" ), this );
   pb_quit -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_quit -> setMinimumHeight( minHeight1 );
   pb_quit -> setPalette      ( QPalette( USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active ) );
   connect( pb_quit, SIGNAL( clicked() ), SLOT( quit() ) );

   pb_go =  new QPushButton ( tr( "Continue" ), this );
   pb_go -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_go -> setMinimumHeight( minHeight1 );
   pb_go -> setPalette      ( QPalette( USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active ) );
   connect( pb_go, SIGNAL( clicked() ), SLOT( go() ) );

   QVBoxLayout *background = new QVBoxLayout( this );
   // background->addSpacing(4);

   background->addWidget( lbl_title );
   // background->addSpacing( 4 );

   QVBoxLayout * vbl = new QVBoxLayout( 0 );
   vbl->addWidget( cb_add_bl );
   vbl->addWidget( cb_save_as_pct_iq );
   
   QHBoxLayout *hbl_bottom = new QHBoxLayout( 0 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addWidget ( pb_quit );
   hbl_bottom->addWidget ( pb_go );

   background->addLayout ( vbl );
   background->addLayout ( hbl_bottom );
   // background->addSpacing( 4 );
}

void US_Hydrodyn_Saxs_Hplc_Ciq::quit()
{
   close();
}

void US_Hydrodyn_Saxs_Hplc_Ciq::go()
{
   (*parameters)[ "go" ] = "true";
   close();
}

void US_Hydrodyn_Saxs_Hplc_Ciq::help()
{
   US_Help *online_help;
   online_help = new US_Help( this );
   online_help->show_help("manual/saxs_hplc_ciq.html");
}

void US_Hydrodyn_Saxs_Hplc_Ciq::closeEvent( QCloseEvent *e )
{

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Saxs_Hplc_Ciq::set_add_bl()
{
   (*parameters)[ "add_baseline" ] = cb_add_bl->isChecked() ? "true" : "false";
}

void US_Hydrodyn_Saxs_Hplc_Ciq::set_save_as_pct_iq()
{
   (*parameters)[ "save_as_pct_iq" ] = cb_save_as_pct_iq->isChecked() ? "true" : "false";
}

