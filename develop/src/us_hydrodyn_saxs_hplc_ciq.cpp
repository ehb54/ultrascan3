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
   update_enables();

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

   cb_sd_source = new QCheckBox(this);
   cb_sd_source->setText( tr( "Compute standard deviations as a difference between the sum of Gaussians and original I(q)" ) );
   cb_sd_source->setEnabled( true );
   connect( cb_sd_source, SIGNAL( clicked() ), SLOT( set_sd_source() ) );
   cb_sd_source->setChecked( false );
   cb_sd_source->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_sd_source->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_normalize = new QCheckBox(this);
   cb_normalize->setText( tr( "Normalize resulting I(q) by concentration" ) );
   cb_normalize->setEnabled( true );
   connect( cb_normalize, SIGNAL( clicked() ), SLOT( set_normalize() ) );
   cb_normalize->setChecked( true );
   cb_normalize->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_normalize->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   lbl_error = new QLabel( parameters->count( "error" ) ? (*parameters)[ "error" ] : "", this );
   lbl_error->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_error->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_error->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold ));

   pb_global =  new QPushButton ( tr( "Duplicate Gaussian 1 values globally" ), this );
   pb_global -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_global -> setMinimumHeight( minHeight1 );
   pb_global -> setPalette      ( QPalette( USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active ) );
   connect( pb_global, SIGNAL( clicked() ), SLOT( global() ) );

   lbl_gaussian = new QLabel( tr( "Gaussian" ), this );
   lbl_gaussian->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_gaussian->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_gaussian->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));

   lbl_conv = new QLabel( tr( (*parameters).count( "uv" ) ? "Extinction coefficient (ml mg^-1 cm^-1)" : "Differential RI increment [dn/dc] (ml/g)" ), this );
   lbl_conv->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_conv->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_conv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));

   lbl_psv = new QLabel( tr( "Partial specific volume (ml/g)" ), this );
   lbl_psv->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_psv->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_psv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));


   for ( unsigned int i = 0; i < (* parameters)[ "gaussians" ].toUInt(); i++ )
   {
      QLabel * lbl_tmp = new QLabel( QString( "%1" ).arg( i + 1 ), this );
      lbl_tmp->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
      lbl_tmp->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      lbl_tmp->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
      lbl_gaussian_id.push_back( lbl_tmp );

      QLineEdit * le_tmp = new QLineEdit(this, "le_tmp Line Edit");
      le_tmp->setText( "" );
      le_tmp->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
      le_tmp->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      le_tmp->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
      {
         QDoubleValidator *qdv = new QDoubleValidator( le_tmp );
         qdv->setDecimals( 8 );
         le_tmp->setValidator( qdv );
      }
      connect( le_tmp, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
      le_tmp->setMinimumWidth( 200 );
      le_tmp-> setMinimumHeight( minHeight1 );
      le_conv.push_back( le_tmp );

      le_tmp = new QLineEdit(this, "le_tmp Line Edit");
      le_tmp->setText( "" );
      le_tmp->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
      le_tmp->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      le_tmp->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
      {
         QDoubleValidator *qdv = new QDoubleValidator( le_tmp );
         qdv->setDecimals( 8 );
         le_tmp->setValidator( qdv );
      }
      connect( le_tmp, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
      le_tmp->setMinimumWidth( 200 );
      le_tmp-> setMinimumHeight( minHeight1 );
      le_psv.push_back( le_tmp );
   }

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
   vbl->addWidget( cb_sd_source );
   vbl->addWidget( cb_normalize );
   
   QGridLayout * gl = new QGridLayout( 0 );

   int j = 0;
   gl->addMultiCellWidget( lbl_error, j, j, 0, 2 );
   j++;
   
   gl->addWidget( lbl_gaussian, j, 0 );
   gl->addWidget( lbl_conv    , j, 1 );
   gl->addWidget( lbl_psv     , j, 2 );
   j++;

   for ( unsigned int i = 0; i < ( unsigned int ) lbl_gaussian_id.size(); i++ )
   {
      gl->addWidget( lbl_gaussian_id[ i ], j, 0 );
      gl->addWidget( le_conv        [ i ], j, 1 );
      gl->addWidget( le_psv         [ i ], j, 2 );
      j++;
   }

   if ( parameters->count( "error" ) )
   {
      lbl_gaussian->hide();
      lbl_conv    ->hide();
      lbl_psv     ->hide();
      for ( unsigned int i = 0; i < ( unsigned int ) lbl_gaussian_id.size(); i++ )
      {
         lbl_gaussian_id[ i ]->hide();
         le_conv        [ i ]->hide();
         le_psv         [ i ]->hide();
      }
      pb_global   ->hide();
      cb_normalize->hide();
   } else {
      lbl_error    ->hide();
      if ( lbl_gaussian_id.size() <= 1 )
      {
         pb_global    ->hide();
      }
   }

   QHBoxLayout *hbl_bottom = new QHBoxLayout( 0 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addWidget ( pb_quit );
   hbl_bottom->addWidget ( pb_go );

   background->addLayout ( vbl );
   background->addLayout ( gl );
   background->addWidget ( pb_global);
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
   (*parameters)[ "normalize" ] = cb_normalize->isChecked() ? "true" : "false";
   (*parameters)[ "sd_source" ] = cb_sd_source->isChecked() ? "difference" : "original";
   for ( unsigned int i = 0; i < ( unsigned int ) lbl_gaussian_id.size(); i++ )
   {
      (*parameters)[ QString( "conv %1" ).arg( i ) ] = le_conv[ i ]->text();
      (*parameters)[ QString( "psv %1" ) .arg( i ) ] = le_psv [ i ]->text();
   }
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

void US_Hydrodyn_Saxs_Hplc_Ciq::set_sd_source()
{
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc_Ciq::set_normalize()
{
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc_Ciq::set_save_as_pct_iq()
{
   (*parameters)[ "save_as_pct_iq" ] = cb_save_as_pct_iq->isChecked() ? "true" : "false";
   cb_save_as_pct_iq->isChecked() ? ( cb_sd_source->setChecked( false ), cb_sd_source->hide() ) : cb_sd_source->show();
}

void US_Hydrodyn_Saxs_Hplc_Ciq::global()
{
   for ( unsigned int i = 1; i < ( unsigned int ) lbl_gaussian_id.size(); i++ )
   {
      le_conv[ i ]->setText( le_conv[ 0 ]->text() );
      le_psv [ i ]->setText( le_psv [ 0 ]->text() );
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc_Ciq::update_enables()
{
   bool no_go = false;

   if ( !parameters->count( "error" ) )
   {
      for ( unsigned int i = 0; i < ( unsigned int ) lbl_gaussian_id.size(); i++ )
      {
         if ( !le_conv[ i ]->text().toDouble() )
         {
            no_go = true;
            break;
         }
         if ( !le_psv [ i ]->text().toDouble() )
         {
            no_go = true;
            break;
         }
      }
   }

   pb_go->setEnabled( !no_go );
}
