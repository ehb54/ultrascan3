#include "../include/us_hydrodyn_saxs_hplc_dctr.h"

US_Hydrodyn_Saxs_Hplc_Dctr::US_Hydrodyn_Saxs_Hplc_Dctr(
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
   setCaption( tr( "US-SOMO: SAXS HPLC : Concentration Detector" ) );

   setupGUI();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );
}

US_Hydrodyn_Saxs_Hplc_Dctr::~US_Hydrodyn_Saxs_Hplc_Dctr()
{
}

void US_Hydrodyn_Saxs_Hplc_Dctr::setupGUI()
{
   int minHeight1  = 30;

   lbl_title =  new QLabel      ( tr( "US-SOMO: SAXS HPLC : Concentration Detector" ), this );
   lbl_title -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_title -> setMinimumHeight( minHeight1 );
   lbl_title -> setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_title -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold ) );

   lbl_type = new QLabel( tr( "Detector type : " ), this );
   lbl_type->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_type->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_type->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));

   cb_uv = new QCheckBox(this);
   cb_uv->setText( "UV  " );
   cb_uv->setEnabled( true );
   connect( cb_uv, SIGNAL( clicked() ), SLOT( set_uv() ) );
   cb_uv->setChecked( parameters->count( "uv" ) );
   cb_uv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_uv->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_uv -> setMinimumHeight( minHeight1 );

   lbl_uv_conv = new QLabel( tr( "Conversion factor (signal -> concentration) : " ), this );
   lbl_uv_conv->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_uv_conv->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_uv_conv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_uv_conv-> setMinimumHeight( minHeight1 );
   
   le_uv_conv = new QLineEdit(this, "le_uv_conv Line Edit");
   le_uv_conv->setText( (*parameters)[ "uv_conv" ] );
   le_uv_conv->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_uv_conv->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_uv_conv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( le_uv_conv );
      qdv->setDecimals( 8 );
      le_uv_conv->setValidator( qdv );
   }
   connect( le_uv_conv, SIGNAL( textChanged( const QString & ) ), SLOT( uv_conv_text( const QString & ) ) );
   le_uv_conv->setMinimumWidth( 200 );
   le_uv_conv-> setMinimumHeight( minHeight1 );

   cb_ri = new QCheckBox(this);
   cb_ri->setText( "RI  " );
   cb_ri->setEnabled( true );
   connect( cb_ri, SIGNAL( clicked() ), SLOT( set_ri() ) );
   cb_ri->setChecked( parameters->count( "ri" ) );
   cb_ri->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_ri->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_ri-> setMinimumHeight( minHeight1 );

   lbl_ri_conv = new QLabel( tr( "Conversion factor (signal -> concentration) : " ), this );
   lbl_ri_conv->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_ri_conv->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_ri_conv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   lbl_ri_conv-> setMinimumHeight( minHeight1 );

   le_ri_conv = new QLineEdit(this, "le_ri_conv Line Edit");
   le_ri_conv->setText( (*parameters)[ "ri_conv" ] );
   le_ri_conv->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_ri_conv->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_ri_conv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( le_ri_conv );
      qdv->setDecimals( 8 );
      le_ri_conv->setValidator( qdv );
   }
   connect( le_ri_conv, SIGNAL( textChanged( const QString & ) ), SLOT( ri_conv_text( const QString & ) ) );
   le_ri_conv->setMinimumWidth( 200 );
   le_ri_conv-> setMinimumHeight( minHeight1 );

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

   pb_save =  new QPushButton ( tr( "Save" ), this );
   pb_save -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_save -> setMinimumHeight( minHeight1 );
   pb_save -> setPalette      ( QPalette( USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active ) );
   connect( pb_save, SIGNAL( clicked() ), SLOT( save() ) );

   QVBoxLayout *background = new QVBoxLayout( this );
   background->addSpacing(4);

   background->addWidget( lbl_title );
   background->addSpacing( 2 );
   background->addWidget( lbl_type );

   QGridLayout * gbl = new QGridLayout( 0 );

   gbl->addWidget( cb_uv, 0, 0 );
   gbl->addWidget( lbl_uv_conv, 0, 1 );
   gbl->addWidget( le_uv_conv, 0, 2 );

   gbl->addWidget( cb_ri, 1, 0 );
   gbl->addWidget( lbl_ri_conv, 1, 1 );
   gbl->addWidget( le_ri_conv, 1, 2 );

   background->addLayout( gbl );

   QHBoxLayout *hbl_bottom = new QHBoxLayout( 0 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addWidget ( pb_quit );
   hbl_bottom->addWidget ( pb_save );

   background->addLayout ( hbl_bottom );
}

void US_Hydrodyn_Saxs_Hplc_Dctr::quit()
{
   close();
}

void US_Hydrodyn_Saxs_Hplc_Dctr::save()
{
   (*parameters)[ "save" ] = "set";
   close();
}

void US_Hydrodyn_Saxs_Hplc_Dctr::help()
{
   US_Help *online_help;
   online_help = new US_Help( this );
   online_help->show_help("manual/saxs_hplc_dctr.html");
}

void US_Hydrodyn_Saxs_Hplc_Dctr::closeEvent( QCloseEvent *e )
{

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Saxs_Hplc_Dctr::set_uv()
{
   (*parameters)[ "uv" ] = cb_uv->isChecked() ? "true" : "false";
   if ( cb_uv->isChecked() )
   {
      cb_ri->setChecked( false );
      parameters->erase( "ri" );
   }
}

void US_Hydrodyn_Saxs_Hplc_Dctr::set_ri()
{
   (*parameters)[ "ri" ] = cb_ri->isChecked() ? "true" : "false";
   if ( cb_ri->isChecked() )
   {
      cb_uv->setChecked( false );
      parameters->erase( "uv" );
   }
}

void US_Hydrodyn_Saxs_Hplc_Dctr::uv_conv_text( const QString & text )
{
   (*parameters)[ "uv_conv" ] = text;
}

void US_Hydrodyn_Saxs_Hplc_Dctr::ri_conv_text( const QString & text )
{
   (*parameters)[ "ri_conv" ] = text;
}
