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

   cb_save_sum = new QCheckBox(this);
   cb_save_sum->setText(tr( "Create sum of peaks curves" ) );
   cb_save_sum->setEnabled( true );
   cb_save_sum->setChecked( false );
   cb_save_sum->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_save_sum->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_sd_source = new QCheckBox(this);
   cb_sd_source->setText( tr( "Compute standard deviations as a difference between the sum of Gaussians and original I(q)" ) );
   cb_sd_source->setEnabled( true );
   connect( cb_sd_source, SIGNAL( clicked() ), SLOT( set_sd_source() ) );
   cb_sd_source->setChecked( false );
   cb_sd_source->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_sd_source->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   QLabel * lbl_sd_zeros_found = new QLabel( tr( "If zeros are produced when computing S.D.'s:  " ), this );
   lbl_sd_zeros_found->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_sd_zeros_found->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_sd_zeros_found->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ));

   cb_sd_zero_avg_local_sd = new QCheckBox(this);
   cb_sd_zero_avg_local_sd->setText( tr( "Average adjacent S.D.'s  " ) );
   cb_sd_zero_avg_local_sd->setEnabled( true );
   connect( cb_sd_zero_avg_local_sd, SIGNAL( clicked() ), SLOT( set_sd_zero_avg_local_sd() ) );
   cb_sd_zero_avg_local_sd->setChecked( true );
   cb_sd_zero_avg_local_sd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_sd_zero_avg_local_sd->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_sd_zero_keep_as_zeros = new QCheckBox(this);
   cb_sd_zero_keep_as_zeros->setText( tr( "Leave as zero  " ) );
   cb_sd_zero_keep_as_zeros->setEnabled( true );
   connect( cb_sd_zero_keep_as_zeros, SIGNAL( clicked() ), SLOT( set_sd_zero_keep_as_zeros() ) );
   cb_sd_zero_keep_as_zeros->setChecked( false );
   cb_sd_zero_keep_as_zeros->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_sd_zero_keep_as_zeros->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_sd_zero_set_to_pt1pct = new QCheckBox(this);
   cb_sd_zero_set_to_pt1pct->setText( tr( "Set to 0.1 % of peak's I(q)  " ) );
   cb_sd_zero_set_to_pt1pct->setEnabled( true );
   connect( cb_sd_zero_set_to_pt1pct, SIGNAL( clicked() ), SLOT( set_sd_zero_set_to_pt1pct() ) );
   cb_sd_zero_set_to_pt1pct->setChecked( false );
   cb_sd_zero_set_to_pt1pct->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_sd_zero_set_to_pt1pct->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   ws_sd_zeros.push_back( lbl_sd_zeros_found );
   ws_sd_zeros.push_back( cb_sd_zero_avg_local_sd );
   ws_sd_zeros.push_back( cb_sd_zero_keep_as_zeros );
   ws_sd_zeros.push_back( cb_sd_zero_set_to_pt1pct );

   QLabel * lbl_zeros_found = new mQLabel( tr( "I(t) contains S.D.'s of zeros. When computing I(q) S.D.'s : " ), this );
   lbl_zeros_found->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_zeros_found->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_zeros_found->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ));
   connect( lbl_zeros_found, SIGNAL(pressed()), SLOT( zeros_found() ));

   cb_zero_drop_points = new QCheckBox(this);
   cb_zero_drop_points->setText( tr( "Drop points  " ) );
   cb_zero_drop_points->setEnabled( true );
   connect( cb_zero_drop_points, SIGNAL( clicked() ), SLOT( set_zero_drop_points() ) );
   cb_zero_drop_points->setChecked( false );
   cb_zero_drop_points->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_zero_drop_points->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_zero_avg_local_sd = new QCheckBox(this);
   cb_zero_avg_local_sd->setText( tr( "Average adjacent S.D.'s  " ) );
   cb_zero_avg_local_sd->setEnabled( true );
   connect( cb_zero_avg_local_sd, SIGNAL( clicked() ), SLOT( set_zero_avg_local_sd() ) );
   cb_zero_avg_local_sd->setChecked( true );
   cb_zero_avg_local_sd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_zero_avg_local_sd->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_zero_keep_as_zeros = new QCheckBox(this);
   cb_zero_keep_as_zeros->setText( tr( "Leave as zeros " ) );
   cb_zero_keep_as_zeros->setEnabled( true );
   connect( cb_zero_keep_as_zeros, SIGNAL( clicked() ), SLOT( set_zero_keep_as_zeros() ) );
   cb_zero_keep_as_zeros->setChecked( false );
   cb_zero_keep_as_zeros->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_zero_keep_as_zeros->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   ws_zeros.push_back( lbl_zeros_found );
   ws_zeros.push_back( cb_zero_drop_points );
   ws_zeros.push_back( cb_zero_avg_local_sd );
   ws_zeros.push_back( cb_zero_keep_as_zeros );

   cb_normalize = new QCheckBox(this);
   cb_normalize->setText( tr( "Normalize resulting I(q) by concentration" ) );
   cb_normalize->setEnabled( true );
   connect( cb_normalize, SIGNAL( clicked() ), SLOT( set_normalize() ) );
   cb_normalize->setChecked( false );
   cb_normalize->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_normalize->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   lbl_error = new QLabel( parameters->count( "error" ) ? (*parameters)[ "error" ] : "", this );
   lbl_error->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_error->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_error->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold ));

   lbl_conc = new QLabel( tr( "Concentrations will be computed and will be written along with PSVs to the output I(q) curves" ), this );
   lbl_conc->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_conc->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold ));

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
   vbl->addWidget( cb_save_sum );

   vbl->addWidget( cb_sd_source );

   QHBoxLayout * hbl_sd_zeros = new QHBoxLayout( 0 );
   for ( unsigned int i = 0; i < ( unsigned int )ws_sd_zeros.size(); i++ )
   {
      hbl_sd_zeros->addWidget( ws_sd_zeros[ i ] );
   }
   vbl->addLayout( hbl_sd_zeros );

   QHBoxLayout * hbl_zeros = new QHBoxLayout( 0 );
   for ( unsigned int i = 0; i < ( unsigned int )ws_zeros.size(); i++ )
   {
      hbl_zeros->addWidget( ws_zeros[ i ] );
   }
   vbl->addLayout( hbl_zeros );

   vbl->addWidget( cb_normalize );
   
   QGridLayout * gl = new QGridLayout( 0 );

   int j = 0;
   gl->addMultiCellWidget( lbl_error, j, j, 0, 2 );
   j++;

   gl->addMultiCellWidget( lbl_conc, j, j, 0, 2 );
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
      lbl_conc    ->hide();
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
      // cb_normalize->hide();
   } else {
      lbl_error    ->hide();
      if ( lbl_gaussian_id.size() <= 1 )
      {
         pb_global    ->hide();
      }
   }
   cb_normalize->hide();

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
   (*parameters)[ "save_sum"  ] = cb_save_sum ->isChecked() ? "true" : "false";

   (*parameters)[ "sd_zero_avg_local_sd"  ] = cb_sd_zero_avg_local_sd ->isChecked() ? "true" : "false";
   (*parameters)[ "sd_zero_keep_as_zeros" ] = cb_sd_zero_keep_as_zeros->isChecked() ? "true" : "false";
   (*parameters)[ "sd_zero_set_to_pt1pct" ] = cb_sd_zero_set_to_pt1pct->isChecked() ? "true" : "false";

   (*parameters)[ "zero_drop_points"   ] = cb_zero_drop_points  ->isChecked() ? "true" : "false";
   (*parameters)[ "zero_avg_local_sd"  ] = cb_zero_avg_local_sd ->isChecked() ? "true" : "false";
   (*parameters)[ "zero_keep_as_zeros" ] = cb_zero_keep_as_zeros->isChecked() ? "true" : "false";

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
   update_enables();
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

void US_Hydrodyn_Saxs_Hplc_Ciq::set_sd_zero_avg_local_sd()
{
   (*parameters)[ "sd_zero_avg_local_sd" ] = cb_sd_zero_avg_local_sd->isChecked() ? "true" : "false";
   if ( cb_sd_zero_avg_local_sd->isChecked() )
   {
      cb_sd_zero_keep_as_zeros->setChecked( false );
      cb_sd_zero_set_to_pt1pct->setChecked( false );
   }
}

void US_Hydrodyn_Saxs_Hplc_Ciq::set_sd_zero_keep_as_zeros()
{
   (*parameters)[ "sd_zero_keep_as_zeros" ] = cb_sd_zero_keep_as_zeros->isChecked() ? "true" : "false";
   if ( cb_sd_zero_keep_as_zeros->isChecked() )
   {
      cb_sd_zero_avg_local_sd ->setChecked( false );
      cb_sd_zero_set_to_pt1pct->setChecked( false );
   }
}

void US_Hydrodyn_Saxs_Hplc_Ciq::set_sd_zero_set_to_pt1pct()
{
   (*parameters)[ "sd_zero_set_to_pt1pct" ] = cb_sd_zero_set_to_pt1pct->isChecked() ? "true" : "false";
   if ( cb_sd_zero_set_to_pt1pct->isChecked() )
   {
      cb_sd_zero_avg_local_sd ->setChecked( false );
      cb_sd_zero_keep_as_zeros->setChecked( false );
   }
}

void US_Hydrodyn_Saxs_Hplc_Ciq::set_zero_drop_points()
{
   (*parameters)[ "zero_drop_points" ] = cb_zero_drop_points->isChecked() ? "true" : "false";
   if ( cb_zero_drop_points->isChecked() )
   {
      cb_zero_avg_local_sd ->setChecked( false );
      cb_zero_keep_as_zeros->setChecked( false );
   }
}

void US_Hydrodyn_Saxs_Hplc_Ciq::set_zero_avg_local_sd()
{
   (*parameters)[ "zero_avg_local_sd" ] = cb_zero_avg_local_sd->isChecked() ? "true" : "false";
   if ( cb_zero_avg_local_sd->isChecked() )
   {
      cb_zero_drop_points  ->setChecked( false );
      cb_zero_keep_as_zeros->setChecked( false );
   }
}

void US_Hydrodyn_Saxs_Hplc_Ciq::set_zero_keep_as_zeros()
{
   (*parameters)[ "zero_keep_as_zeros" ] = cb_zero_keep_as_zeros->isChecked() ? "true" : "false";
   if ( cb_zero_keep_as_zeros->isChecked() )
   {
      cb_zero_drop_points  ->setChecked( false );
      cb_zero_avg_local_sd ->setChecked( false );
   }
}


void US_Hydrodyn_Saxs_Hplc_Ciq::update_enables()
{
   bool no_go = false;

   ws_hide( ws_sd_zeros, !cb_sd_source->isChecked() );
   ws_hide( ws_zeros   , 
            cb_sd_source->isChecked() || 
            ( (*parameters)[ "no_errors" ].isEmpty() && 
              (*parameters)[ "zero_points" ].isEmpty() ) );

   if ( !parameters->count( "error" ) ) //  && cb_normalize->isChecked() )
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

void US_Hydrodyn_Saxs_Hplc_Ciq::ws_hide( vector < QWidget * > ws, bool hide )
{
   for ( unsigned int i = 0; i < ( unsigned int )ws.size(); i++ )
   {
      hide ? ws[ i ]->hide() : ws[ i ]->show();
   }
}

void US_Hydrodyn_Saxs_Hplc_Ciq::zeros_found()
{
   QMessageBox::information( this,
                             caption() + tr( ": S.D. Zeros" ),
                             QString( tr( "Please note:\n\n"
                                          "%1"
                                          "%2"
                                          "\n" ) )
                             .arg( (*parameters)[ "no_errors" ].isEmpty() ?
                                   "" : QString( tr( "These files have no associated errors:\n%1\n\n" ) ).arg( (*parameters)[ "no_errors" ] ) )
                             .arg( (*parameters)[ "zero_points" ].isEmpty() ?
                                   "" : QString( tr( "These files have zero points:\n%1\n\n" ) ).arg( (*parameters)[ "zero_points" ] ) ),
                             QMessageBox::Ok
                             );
}                             
