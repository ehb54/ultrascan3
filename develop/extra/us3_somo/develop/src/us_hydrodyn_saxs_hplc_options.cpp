#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_saxs_hplc_options.h"
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <QLabel>
#include <QCloseEvent>

US_Hydrodyn_Saxs_Hplc_Options::US_Hydrodyn_Saxs_Hplc_Options(
                                                             map < QString, QString > * parameters,
                                                             QWidget *                  p,
                                                             const char *               name
                                                             ) : QDialog( p, name )
{
   this->hplc_win   = hplc_win;
   this->parameters = parameters;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
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
   lbl_title -> setPalette      ( PALET_LABEL );
   AUTFBACK( lbl_title );
   lbl_title -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold ) );

   lbl_baseline = new QLabel ( tr( "Baseline removal" ), this);
   lbl_baseline->setAlignment( Qt::AlignCenter | Qt::AlignVCenter);
   lbl_baseline->setPalette  ( PALET_FRAME );
   AUTFBACK( lbl_baseline );
   lbl_baseline->setFont     ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   rb_linear = new QRadioButton( tr("Linear baseline removal"), this);
   rb_linear->setEnabled(true);
   rb_linear->setChecked( parameters->count( "hplc_bl_linear" ) && (*parameters)[ "hplc_bl_linear" ] == "true" );
   rb_linear->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_linear->setPalette( PALET_NORMAL );
   AUTFBACK( rb_linear );
   connect( rb_linear, SIGNAL( clicked() ), SLOT( update_enables() ) );

   rb_integral = new QRadioButton( tr("Integral of I(t) baseline removal"), this);
   rb_integral->setEnabled(true);
   rb_integral->setChecked( parameters->count( "hplc_bl_integral" ) && (*parameters)[ "hplc_bl_integral" ] == "true" );
   rb_integral->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_integral->setPalette( PALET_NORMAL );
   AUTFBACK( rb_integral );
   connect( rb_integral, SIGNAL( clicked() ), SLOT( update_enables() ) );

   bg_bl_type = new QButtonGroup( this );
   int bg_pos = 0;
   bg_bl_type->setExclusive(true);
   bg_bl_type->addButton( rb_linear, bg_pos++ );
   bg_bl_type->addButton( rb_integral, bg_pos++ );
   // connect( bg_bl_type, SIGNAL( buttonClicked( int id ) ), SLOT( update_enables() ) );

   cb_save_bl = new QCheckBox(this);
   cb_save_bl->setText( tr( "Produce separate baseline curves " ) );
   cb_save_bl->setEnabled( true );
   cb_save_bl->setChecked( parameters->count( "hplc_bl_save" ) && (*parameters)[ "hplc_bl_save" ] == "true" );
   cb_save_bl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_save_bl->setPalette( PALET_NORMAL );
   AUTFBACK( cb_save_bl );

   lbl_smooth =  new QLabel      ( tr( "Smoothing:" ), this );
   lbl_smooth -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_smooth -> setPalette      ( PALET_LABEL );
   AUTFBACK( lbl_smooth );
   lbl_smooth -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_smooth = new QLineEdit(this, "le_smooth Line Edit");
   le_smooth->setText( parameters->count( "hplc_bl_smooth" ) ? (*parameters)[ "hplc_bl_smooth" ] : "10" );
   le_smooth->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_smooth->setPalette( PALET_NORMAL );
   AUTFBACK( le_smooth );
   le_smooth->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QIntValidator *qdv = new QIntValidator( 0, 99, le_smooth );
      le_smooth->setValidator( qdv );
   }
   connect( le_smooth, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_smooth->setMinimumWidth( 60 );

   lbl_reps =  new QLabel      ( tr( "Maximum iterations:" ), this );
   lbl_reps -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_reps -> setPalette( PALET_LABEL );
   AUTFBACK( lbl_reps );
   lbl_reps -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_reps = new QLineEdit(this, "le_reps Line Edit");
   le_reps->setText( parameters->count( "hplc_bl_reps" ) ? (*parameters)[ "hplc_bl_reps" ] : "1" );
   le_reps->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_reps->setPalette( PALET_NORMAL );
   AUTFBACK( le_reps );
   le_reps->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QIntValidator *qdv = new QIntValidator( 1, 20, le_reps );
      le_reps->setValidator( qdv );
   }
   connect( le_reps, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_reps->setMinimumWidth( 60 );

   lbl_alpha =  new QLabel      ( tr( "alpha early termination limit:" ), this );
   lbl_alpha -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_alpha -> setPalette( PALET_LABEL );
   AUTFBACK( lbl_alpha );
   lbl_alpha -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_alpha = new QLineEdit(this, "le_alpha Line Edit");
   le_alpha->setText( parameters->count( "hplc_bl_alpha" ) ? (*parameters)[ "hplc_bl_alpha" ] : "1" );
   le_alpha->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_alpha->setPalette( PALET_NORMAL );
   AUTFBACK( le_alpha );
   le_alpha->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 0, 1, 3, le_alpha );
      le_alpha->setValidator( qdv );
   }
   connect( le_alpha, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_alpha->setMinimumWidth( 60 );

   lbl_gaussian_type = new QLabel ( tr( "Gaussian Mode" ), this);
   lbl_gaussian_type->setAlignment( Qt::AlignCenter | Qt::AlignVCenter);
   lbl_gaussian_type->setPalette  ( PALET_FRAME );
   AUTFBACK( lbl_gaussian_type );
   lbl_gaussian_type->setFont     ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   rb_gauss = new QRadioButton( tr("Standard Gaussians"), this);
   rb_gauss->setEnabled(true);
   rb_gauss->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_gauss->setPalette( PALET_NORMAL );
   AUTFBACK( rb_gauss );

   rb_gmg = new QRadioButton( tr("GMG (Half-Gaussian modified Gaussian)"), this);
   rb_gmg->setEnabled(true);
   rb_gmg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_gmg->setPalette( PALET_NORMAL );
   AUTFBACK( rb_gmg );

   rb_emg = new QRadioButton( tr("EMG (Exponentially modified Gaussian)"), this);
   rb_emg->setEnabled(true);
   rb_emg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_emg->setPalette( PALET_NORMAL );
   AUTFBACK( rb_emg );

   rb_emggmg = new QRadioButton( tr("EMG+GMG"), this);
   rb_emggmg->setEnabled(true);
   rb_emggmg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_emggmg->setPalette( PALET_NORMAL );
   AUTFBACK( rb_emggmg );

   bg_gaussian_type = new QButtonGroup( this );
   bg_pos = 0;
   bg_gaussian_type->setExclusive(true);
   bg_gaussian_type->addButton( rb_gauss, bg_pos++ );
   bg_gaussian_type->addButton( rb_gmg, bg_pos++ );
   bg_gaussian_type->addButton( rb_emg, bg_pos++ );
   bg_gaussian_type->addButton( rb_emggmg, bg_pos++ );

   lbl_other_options = new QLabel ( tr( "Miscellaneous options" ), this);
   lbl_other_options->setAlignment( Qt::AlignCenter | Qt::AlignVCenter);
   lbl_other_options->setPalette  ( PALET_FRAME );
   AUTFBACK( lbl_other_options );
   lbl_other_options->setFont     ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   cb_csv_transposed = new QCheckBox(this);
   cb_csv_transposed->setText(tr( "Save CSV transposed"));
   cb_csv_transposed->setEnabled( true );
   cb_csv_transposed->setChecked( (*parameters)[ "hplc_csv_transposed" ] == "true" );
   cb_csv_transposed->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_csv_transposed->setPalette( PALET_NORMAL );
   AUTFBACK( cb_csv_transposed );

   pb_quit =  new QPushButton ( tr( "Quit" ), this );
   pb_quit -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_quit -> setMinimumHeight( minHeight1 );
   pb_quit -> setPalette      ( PALET_PUSHB );
   connect( pb_quit, SIGNAL( clicked() ), SLOT( quit() ) );

   pb_help =  new QPushButton ( tr( "Help" ), this );
   pb_help -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_help -> setMinimumHeight( minHeight1 );
   pb_help -> setPalette      ( PALET_PUSHB );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

   pb_ok =  new QPushButton ( tr( "Ok" ), this );
   pb_ok -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_ok -> setMinimumHeight( minHeight1 );
   pb_ok -> setPalette      ( PALET_PUSHB );
   connect( pb_ok, SIGNAL( clicked() ), SLOT( ok() ) );

   Q3VBoxLayout *background = new Q3VBoxLayout( this );
   background->addWidget( lbl_title );

   background->addWidget( lbl_baseline );

   background->addWidget( rb_linear );
   background->addWidget( rb_integral );

   Q3GridLayout *gl_bl = new Q3GridLayout( 0 );

   gl_bl->addWidget         ( lbl_smooth , 0, 0 );
   gl_bl->addWidget         ( le_smooth  , 0, 1 );
   gl_bl->addWidget         ( lbl_reps   , 1, 0 );
   gl_bl->addWidget         ( le_reps    , 1, 1 );
   gl_bl->addWidget         ( lbl_alpha  , 2, 0 );
   gl_bl->addWidget         ( le_alpha   , 2, 1 );

   background->addLayout( gl_bl );
   background->addWidget( cb_save_bl );

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

   Q3HBoxLayout *hbl_bottom = new Q3HBoxLayout( 0 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addWidget ( pb_quit );
   hbl_bottom->addWidget ( pb_ok );

   background->addLayout ( hbl_bottom );

   if ( !parameters->count( "expert_mode" ) )
   {
      lbl_baseline     ->hide();
      rb_linear        ->hide();
      rb_integral      ->hide();
      lbl_smooth       ->hide();
      le_smooth        ->hide();
      lbl_reps         ->hide();
      le_reps          ->hide();
      lbl_alpha        ->hide();
      le_alpha         ->hide();
      cb_save_bl       ->hide();

      lbl_gaussian_type->hide();
      rb_gauss         ->hide();
      rb_gmg           ->hide();
      rb_emg           ->hide();
      rb_emggmg        ->hide();
   }
}

void US_Hydrodyn_Saxs_Hplc_Options::quit()
{
   close();
}

void US_Hydrodyn_Saxs_Hplc_Options::ok()
{
   (*parameters)[ "ok" ] = "true";

   (*parameters)[ "hplc_bl_linear"   ]   = rb_linear  ->isChecked() ? "true" : "false";
   (*parameters)[ "hplc_bl_integral" ]   = rb_integral->isChecked() ? "true" : "false";
   (*parameters)[ "hplc_bl_save"     ]   = cb_save_bl ->isChecked() ? "true" : "false";
   (*parameters)[ "hplc_bl_smooth"   ]   = le_smooth  ->text();
   (*parameters)[ "hplc_bl_reps"     ]   = le_reps    ->text();
   (*parameters)[ "hplc_bl_alpha"    ]   = le_alpha   ->text();

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

void US_Hydrodyn_Saxs_Hplc_Options::update_enables()
{
   le_smooth ->setEnabled( rb_integral->isChecked() );
   le_reps   ->setEnabled( rb_integral->isChecked() );
   cb_save_bl->setEnabled( rb_integral->isChecked() );
}

