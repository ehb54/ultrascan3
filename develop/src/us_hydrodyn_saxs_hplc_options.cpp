#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_saxs_hplc_options.h"
//Added by qt3to4:
#include <QHBoxLayout>
#include <QCloseEvent>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>

US_Hydrodyn_Saxs_Hplc_Options::US_Hydrodyn_Saxs_Hplc_Options(
                                                             map < QString, QString > * parameters,
                                                             QWidget *                  p,
                                                             const char *               name
                                                             ) : QDialog( p )
{
   this->hplc_win   = p;
   this->parameters = parameters;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle( us_tr( "US-SOMO: SAXS HPLC : Options" ) );

   setupGUI();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );

   update_enables();
}

US_Hydrodyn_Saxs_Hplc_Options::~US_Hydrodyn_Saxs_Hplc_Options()
{
}

void US_Hydrodyn_Saxs_Hplc_Options::setupGUI()
{
   int minHeight1  = 30;

   lbl_title =  new QLabel      ( us_tr( "US-SOMO: SAXS HPLC : Options" ), this );
   lbl_title -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_title -> setMinimumHeight( minHeight1 );
   lbl_title -> setPalette      ( PALET_LABEL );
   AUTFBACK( lbl_title );
   lbl_title -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold ) );

   lbl_baseline = new QLabel ( us_tr( "Baseline removal" ), this);
   lbl_baseline->setAlignment( Qt::AlignCenter | Qt::AlignVCenter);
   lbl_baseline->setPalette  ( PALET_FRAME );
   AUTFBACK( lbl_baseline );
   lbl_baseline->setFont     ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   rb_linear = new QRadioButton( us_tr("Linear baseline removal"), this);
   rb_linear->setEnabled(true);
   rb_linear->setChecked( parameters->count( "hplc_bl_linear" ) && (*parameters)[ "hplc_bl_linear" ] == "true" );
   rb_linear->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_linear->setPalette( PALET_NORMAL );
   AUTFBACK( rb_linear );
   connect( rb_linear, SIGNAL( clicked() ), SLOT( update_enables() ) );

   rb_integral = new QRadioButton( us_tr("Integral of I(t) baseline removal"), this);
   rb_integral->setEnabled(true);
   rb_integral->setChecked( parameters->count( "hplc_bl_integral" ) && (*parameters)[ "hplc_bl_integral" ] == "true" );
   rb_integral->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_integral->setPalette( PALET_NORMAL );
   AUTFBACK( rb_integral );
   connect( rb_integral, SIGNAL( clicked() ), SLOT( update_enables() ) );

#if 1 // QT_VERSION < 0x040000
   bg_bl_type = new QButtonGroup( this );
   int bg_pos = 0;
   bg_bl_type->setExclusive(true);
   bg_bl_type->addButton( rb_linear, bg_pos++ );
   bg_bl_type->addButton( rb_integral, bg_pos++ );
   // connect( bg_bl_type, SIGNAL( buttonClicked( int id ) ), SLOT( update_enables() ) );
#else
   bg_bl_type = new QGroupBox();
   bg_bl_type->setFlat( true );

   {
      QHBoxLayout * bl = new QHBoxLayout; bl->setContentsMargins( 0, 0, 0, 0 ); bl->setSpacing( 0 );
      bl->addWidget( rb_linear );
      bl->addWidget( rb_integral );
      bg_bl_type->setLayout( bl );
   }
#endif
   
   cb_save_bl = new QCheckBox(this);
   // cb_save_bl->setText( us_tr( "Produce separate baseline curves " ) );
   cb_save_bl->setText( us_tr( "Test alt integral baseline " ) );
   cb_save_bl->setEnabled( true );
   cb_save_bl->setChecked( parameters->count( "hplc_bl_save" ) && (*parameters)[ "hplc_bl_save" ] == "true" );
   cb_save_bl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_save_bl->setPalette( PALET_NORMAL );
   AUTFBACK( cb_save_bl );

   lbl_smooth =  new QLabel      ( us_tr( "Smoothing:" ), this );
   lbl_smooth -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_smooth -> setPalette      ( PALET_LABEL );
   AUTFBACK( lbl_smooth );
   lbl_smooth -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_smooth = new QLineEdit( this );    le_smooth->setObjectName( "le_smooth Line Edit" );
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

   lbl_start_region =  new QLabel      ( us_tr( "Start frames for test integral offset:" ), this );
   lbl_start_region -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_start_region -> setPalette      ( PALET_LABEL );
   AUTFBACK( lbl_start_region );
   lbl_start_region -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_start_region = new QLineEdit( this );    le_start_region->setObjectName( "le_start_region Line Edit" );
   le_start_region->setText( parameters->count( "hplc_bl_start_region" ) ? (*parameters)[ "hplc_bl_start_region" ] : "10" );
   le_start_region->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_start_region->setPalette( PALET_NORMAL );
   AUTFBACK( le_start_region );
   le_start_region->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QIntValidator *qdv = new QIntValidator( 0, 99, le_start_region );
      le_start_region->setValidator( qdv );
   }
   connect( le_start_region, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_start_region->setMinimumWidth( 60 );


   lbl_i_power =  new QLabel      ( us_tr( "Intensity exponent:" ), this );
   lbl_i_power -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_i_power -> setPalette      ( PALET_LABEL );
   AUTFBACK( lbl_i_power );
   lbl_i_power -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_i_power = new QLineEdit( this );    le_i_power->setObjectName( "le_i_power Line Edit" );
   le_i_power->setText( parameters->count( "hplc_bl_i_power" ) ? (*parameters)[ "hplc_bl_i_power" ] : "1" );
   le_i_power->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_i_power->setPalette( PALET_NORMAL );
   AUTFBACK( le_i_power );
   le_i_power->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 0, 3, 3, le_i_power );
      le_i_power->setValidator( qdv );
   }
   connect( le_i_power, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_i_power->setMinimumWidth( 60 );

   lbl_reps =  new QLabel      ( us_tr( "Maximum iterations:" ), this );
   lbl_reps -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_reps -> setPalette( PALET_LABEL );
   AUTFBACK( lbl_reps );
   lbl_reps -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_reps = new QLineEdit( this );    le_reps->setObjectName( "le_reps Line Edit" );
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

   lbl_epsilon =  new QLabel      ( us_tr( "Epsilon early termination limit:" ), this );
   lbl_epsilon -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_epsilon -> setPalette( PALET_LABEL );
   AUTFBACK( lbl_epsilon );
   lbl_epsilon -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_epsilon = new QLineEdit( this );    le_epsilon->setObjectName( "le_epsilon Line Edit" );
   le_epsilon->setText( parameters->count( "hplc_bl_epsilon" ) ? (*parameters)[ "hplc_bl_epsilon" ] : "1" );
   le_epsilon->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_epsilon->setPalette( PALET_NORMAL );
   AUTFBACK( le_epsilon );
   le_epsilon->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 0, 1, 3, le_epsilon );
      le_epsilon->setValidator( qdv );
   }
   connect( le_epsilon, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_epsilon->setMinimumWidth( 60 );

   lbl_cormap_maxq =  new QLabel      ( us_tr( "Global CorMap Analysis maximum q [A^-1]:" ), this );
   lbl_cormap_maxq -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_cormap_maxq -> setPalette( PALET_LABEL );
   AUTFBACK( lbl_cormap_maxq );
   lbl_cormap_maxq -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_cormap_maxq = new QLineEdit( this );    le_cormap_maxq->setObjectName( "le_cormap_maxq Line Edit" );
   le_cormap_maxq->setText( parameters->count( "hplc_cormap_maxq" ) ? (*parameters)[ "hplc_cormap_maxq" ] : "0.05" );
   le_cormap_maxq->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_cormap_maxq->setPalette( PALET_NORMAL );
   AUTFBACK( le_cormap_maxq );
   le_cormap_maxq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 0, 3, 3, le_cormap_maxq );
      le_cormap_maxq->setValidator( qdv );
   }
   connect( le_cormap_maxq, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_cormap_maxq->setMinimumWidth( 60 );

   lbl_cormap_alpha =  new QLabel      ( us_tr( "Global CorMap Analysis alpha:" ), this );
   lbl_cormap_alpha -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_cormap_alpha -> setPalette( PALET_LABEL );
   AUTFBACK( lbl_cormap_alpha );
   lbl_cormap_alpha -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_cormap_alpha = new QLineEdit( this );    le_cormap_alpha->setObjectName( "le_cormap_alpha Line Edit" );
   le_cormap_alpha->setText( parameters->count( "hplc_cormap_alpha" ) ? (*parameters)[ "hplc_cormap_alpha" ] : "0.01" );
   le_cormap_alpha->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_cormap_alpha->setPalette( PALET_NORMAL );
   AUTFBACK( le_cormap_alpha );
   le_cormap_alpha->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 0, 0.5, 3, le_cormap_alpha );
      le_cormap_alpha->setValidator( qdv );
   }
   connect( le_cormap_alpha, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_cormap_alpha->setMinimumWidth( 60 );

   lbl_gaussian_type = new QLabel ( us_tr( "Gaussian Mode" ), this);
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

#if 1 // QT_VERSION < 0x040000
   bg_gaussian_type = new QButtonGroup( this );
   bg_pos = 0;
   bg_gaussian_type->setExclusive(true);
   bg_gaussian_type->addButton( rb_gauss, bg_pos++ );
   bg_gaussian_type->addButton( rb_gmg, bg_pos++ );
   bg_gaussian_type->addButton( rb_emg, bg_pos++ );
   bg_gaussian_type->addButton( rb_emggmg, bg_pos++ );
#else
   bg_gaussian_type = new QGroupBox();
   bg_gaussian_type->setFlat( true );
   
   {
      QHBoxLayout * bl = new QHBoxLayout; bl->setContentsMargins( 0, 0, 0, 0 ); bl->setSpacing( 0 );
      bl->addWidget( rb_gauss );
      bl->addWidget( rb_gmg );
      bl->addWidget( rb_emg );
      bl->addWidget( rb_emggmg );
      bg_gaussian_type->setLayout( bl );
   }
#endif

   pb_clear_gauss =  new QPushButton ( us_tr( "Clear cached Gaussian values" ), this );
   pb_clear_gauss -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_clear_gauss -> setMinimumHeight( minHeight1 );
   pb_clear_gauss -> setPalette      ( PALET_PUSHB );
   connect( pb_clear_gauss, SIGNAL( clicked() ), SLOT( clear_gauss() ) );

   lbl_other_options = new QLabel ( us_tr( "Miscellaneous options" ), this);
   lbl_other_options->setAlignment( Qt::AlignCenter | Qt::AlignVCenter);
   lbl_other_options->setPalette  ( PALET_FRAME );
   AUTFBACK( lbl_other_options );
   lbl_other_options->setFont     ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   cb_csv_transposed = new QCheckBox(this);
   cb_csv_transposed->setText(us_tr( "Save CSV transposed"));
   cb_csv_transposed->setEnabled( true );
   cb_csv_transposed->setChecked( (*parameters)[ "hplc_csv_transposed" ] == "true" );
   cb_csv_transposed->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_csv_transposed->setPalette( PALET_NORMAL );
   AUTFBACK( cb_csv_transposed );

   lbl_zi_window =  new QLabel      ( us_tr( "I(t) negative integral check window :" ), this );
   lbl_zi_window -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_zi_window -> setPalette( PALET_LABEL );
   AUTFBACK( lbl_zi_window );
   lbl_zi_window -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_zi_window = new QLineEdit( this );    le_zi_window->setObjectName( "le_zi_window Line Edit" );
   le_zi_window->setText( (*parameters)[ "hplc_zi_window" ] );
   le_zi_window->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_zi_window->setPalette( PALET_NORMAL );
   AUTFBACK( le_zi_window );
   le_zi_window->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QIntValidator *qdv = new QIntValidator( 5, 200, le_zi_window );
      le_zi_window->setValidator( qdv );
   }
   connect( le_zi_window, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_zi_window->setMinimumWidth( 60 );

   cb_discard_it_sd_mult = new QCheckBox(this);
   cb_discard_it_sd_mult->setText(us_tr( "On Make I(t), discard I(t) with no signal above std. dev. multiplied by: "));
   cb_discard_it_sd_mult->setEnabled( true );
   cb_discard_it_sd_mult->setChecked( (*parameters)[ "hplc_cb_discard_it_sd_mult" ] == "true" );
   cb_discard_it_sd_mult->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_discard_it_sd_mult->setPalette( PALET_NORMAL );
   AUTFBACK( cb_discard_it_sd_mult );
   connect( cb_discard_it_sd_mult, SIGNAL( clicked() ), SLOT( update_enables() ) );

   le_discard_it_sd_mult = new QLineEdit( this );    le_discard_it_sd_mult->setObjectName( "le_discard_it_sd_mult Line Edit" );
   le_discard_it_sd_mult->setText( (*parameters)[ "hplc_discard_it_sd_mult" ] );
   le_discard_it_sd_mult->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_discard_it_sd_mult->setPalette( PALET_NORMAL );
   AUTFBACK( le_discard_it_sd_mult );
   le_discard_it_sd_mult->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 1, 20, 3, le_discard_it_sd_mult );
      le_discard_it_sd_mult->setValidator( qdv );
   }
   connect( le_discard_it_sd_mult, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_discard_it_sd_mult->setMinimumWidth( 60 );

   cb_guinier_qrgmax = new QCheckBox(this);
   cb_guinier_qrgmax->setText( us_tr( "Limit Guinier Maximum q*Rg" ) );
   cb_guinier_qrgmax->setEnabled( true );
   cb_guinier_qrgmax->setChecked( (*parameters)[ "hplc_cb_guinier_qrgmax" ] == "true" );
   cb_guinier_qrgmax->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_guinier_qrgmax->setPalette( PALET_NORMAL );
   AUTFBACK( cb_guinier_qrgmax );
   connect( cb_guinier_qrgmax, SIGNAL( clicked() ), SLOT( update_enables() ) );

   le_guinier_qrgmax = new QLineEdit( this );    le_guinier_qrgmax->setObjectName( "le_guinier_qrgmax Line Edit" );
   le_guinier_qrgmax->setText( (*parameters)[ "hplc_guinier_qrgmax" ] );
   le_guinier_qrgmax->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_qrgmax->setPalette( PALET_NORMAL );
   AUTFBACK( le_guinier_qrgmax );
   le_guinier_qrgmax->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 1, 20, 3, le_guinier_qrgmax );
      le_guinier_qrgmax->setValidator( qdv );
   }
   connect( le_guinier_qrgmax, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_guinier_qrgmax->setMinimumWidth( 60 );

   lbl_mwt_k = new QLabel(us_tr(" MW[RT] k : "), this);
   lbl_mwt_k -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_mwt_k -> setPalette( PALET_LABEL );
   AUTFBACK( lbl_mwt_k );
   lbl_mwt_k -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_mwt_k = new QLineEdit( this );    le_mwt_k->setObjectName( "le_mwt_k Line Edit" );
   le_mwt_k->setText( parameters->count( "guinier_mwt_k" ) ? (*parameters)[ "guinier_mwt_k" ] : "0.973" );
   le_mwt_k->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_mwt_k->setPalette( PALET_NORMAL );
   AUTFBACK( le_mwt_k );
   le_mwt_k->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 0.001, 3, 6, le_mwt_k );
      le_mwt_k->setValidator( qdv );
   }
   connect( le_mwt_k, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_mwt_k->setMinimumWidth( 60 );

   lbl_mwt_c = new QLabel(us_tr(" MW[RT] c : "), this);
   lbl_mwt_c -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_mwt_c -> setPalette( PALET_LABEL );
   AUTFBACK( lbl_mwt_c );
   lbl_mwt_c -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_mwt_c = new QLineEdit( this );    le_mwt_c->setObjectName( "le_mwt_c Line Edit" );
   le_mwt_c->setText( parameters->count( "guinier_mwt_c" ) ? (*parameters)[ "guinier_mwt_c" ] : "-1.878" );
   le_mwt_c->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_mwt_c->setPalette( PALET_NORMAL );
   AUTFBACK( le_mwt_c );
   le_mwt_c->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( -3, 0, 6, le_mwt_c );
      le_mwt_c->setValidator( qdv );
   }
   connect( le_mwt_c, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_mwt_c->setMinimumWidth( 60 );

   lbl_mwt_qmax = new QLabel(us_tr(" MW[RT] qmax cut-off : "), this);
   lbl_mwt_qmax -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_mwt_qmax -> setPalette( PALET_LABEL );
   AUTFBACK( lbl_mwt_qmax );
   lbl_mwt_qmax -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_mwt_qmax = new QLineEdit( this );    le_mwt_qmax->setObjectName( "le_mwt_qmax Line Edit" );
   le_mwt_qmax->setText( parameters->count( "guinier_mwt_qmax" ) ? (*parameters)[ "guinier_mwt_qmax" ] : "0.5" );
   le_mwt_qmax->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_mwt_qmax->setPalette( PALET_NORMAL );
   AUTFBACK( le_mwt_qmax );
   le_mwt_qmax->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 0.05, 1.0, 3, le_mwt_qmax );
      le_mwt_qmax->setValidator( qdv );
   }
   connect( le_mwt_qmax, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_mwt_qmax->setMinimumWidth( 60 );


   lbl_dist_max =  new QLabel      ( us_tr( "Maximum absolute value of EMG and GMG distortions:" ), this );
   lbl_dist_max -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_dist_max -> setPalette( PALET_LABEL );
   AUTFBACK( lbl_dist_max );
   lbl_dist_max -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_dist_max = new QLineEdit( this );    le_dist_max->setObjectName( "le_dist_max Line Edit" );
   le_dist_max->setText( (*parameters)[ "hplc_dist_max" ] );
   le_dist_max->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_dist_max->setPalette( PALET_NORMAL );
   AUTFBACK( le_dist_max );
   le_dist_max->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 10, 1000, 1, le_dist_max );
      le_dist_max->setValidator( qdv );
   }
   connect( le_dist_max, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_dist_max->setMinimumWidth( 60 );

   lbl_ampl_width_min =  new QLabel      ( us_tr( "Global minimum value for amplitude and width:" ), this );
   lbl_ampl_width_min -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_ampl_width_min -> setPalette( PALET_LABEL );
   AUTFBACK( lbl_ampl_width_min );
   lbl_ampl_width_min -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_ampl_width_min = new QLineEdit( this );    le_ampl_width_min->setObjectName( "le_ampl_width_min Line Edit" );
   le_ampl_width_min->setText( (*parameters)[ "hplc_ampl_width_min" ] );
   le_ampl_width_min->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_ampl_width_min->setPalette( PALET_NORMAL );
   AUTFBACK( le_ampl_width_min );
   le_ampl_width_min->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 1e-50, 1e-3, 1, le_ampl_width_min );
      le_ampl_width_min->setValidator( qdv );
   }
   connect( le_ampl_width_min, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_ampl_width_min->setMinimumWidth( 60 );

   cb_lock_min_retry = new QCheckBox(this);
   cb_lock_min_retry->setText( us_tr( "Lock curves and retry when minimum amplitude or width is pegged\nby the global minimum times this value:" ) );
   cb_lock_min_retry->setEnabled( true );
   cb_lock_min_retry->setChecked( (*parameters)[ "hplc_lock_min_retry" ] == "true" );
   cb_lock_min_retry->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_lock_min_retry->setPalette( PALET_NORMAL );
   AUTFBACK( cb_lock_min_retry );

   le_lock_min_retry_mult = new QLineEdit( this );    le_lock_min_retry_mult->setObjectName( "le_lock_min_retry_mult Line Edit" );
   le_lock_min_retry_mult->setText( (*parameters)[ "hplc_lock_min_retry_mult" ] );
   le_lock_min_retry_mult->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_lock_min_retry_mult->setPalette( PALET_NORMAL );
   AUTFBACK( le_lock_min_retry_mult );
   le_lock_min_retry_mult->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 1, 2, 3, le_lock_min_retry_mult );
      le_lock_min_retry_mult->setValidator( qdv );
   }
   connect( le_lock_min_retry_mult, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_lock_min_retry_mult->setMinimumWidth( 60 );

   cb_maxfpk_restart = new QCheckBox(this);
   cb_maxfpk_restart->setText( us_tr( "Retry fits by increasing largest free peak amplitude by percent:" ) );
   cb_maxfpk_restart->setEnabled( true );
   cb_maxfpk_restart->setChecked( (*parameters)[ "hplc_maxfpk_restart" ] == "true" );
   cb_maxfpk_restart->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_maxfpk_restart->setPalette( PALET_NORMAL );
   AUTFBACK( cb_maxfpk_restart );

   // le_maxfpk_restart_tries = new QLineEdit( this );  le_maxfpk_restart_tries->setObjectName( "le_maxfpk_restart_tries Line Edit" );
   // le_maxfpk_restart_tries->setText( (*parameters)[ "hplc_maxfpk_restart_tries" ] );
   // le_maxfpk_restart_tries->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   // le_maxfpk_restart_tries->setPalette( PALET_NORMAL );
   // le_maxfpk_restart_tries->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // {
   //    QIntValidator *qiv = new QIntValidator( 1, 10, le_maxfpk_restart_tries );
   //    le_maxfpk_restart_tries->setValidator( qiv );
   // }
   // connect( le_maxfpk_restart_tries, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   // le_maxfpk_restart_tries->setMinimumWidth( 60 );

   le_maxfpk_restart_pct = new QLineEdit( this );    le_maxfpk_restart_pct->setObjectName( "le_maxfpk_restart_pct Line Edit" );
   le_maxfpk_restart_pct->setText( (*parameters)[ "hplc_maxfpk_restart_pct" ] );
   le_maxfpk_restart_pct->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_maxfpk_restart_pct->setPalette( PALET_NORMAL );
   AUTFBACK( le_maxfpk_restart_pct );
   le_maxfpk_restart_pct->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 1, 20, 1, le_maxfpk_restart_pct );
      le_maxfpk_restart_pct->setValidator( qdv );
   }
   connect( le_maxfpk_restart_pct, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_maxfpk_restart_pct->setMinimumWidth( 60 );

   cb_makeiq_cutmax_pct = new QCheckBox(this);
   cb_makeiq_cutmax_pct->setText( us_tr( "Make I(q): drop curves below % of Gaussian max intensity" ) );
   cb_makeiq_cutmax_pct->setEnabled( true );
   cb_makeiq_cutmax_pct->setChecked( (*parameters)[ "hplc_cb_makeiq_cutmax_pct" ] == "true" );
   cb_makeiq_cutmax_pct->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_makeiq_cutmax_pct->setPalette( PALET_NORMAL );
   AUTFBACK( cb_makeiq_cutmax_pct );
   connect( cb_makeiq_cutmax_pct, SIGNAL( clicked() ), SLOT( update_enables() ) );

   le_makeiq_cutmax_pct = new QLineEdit( this );    le_makeiq_cutmax_pct->setObjectName( "le_makeiq_cutmax_pct Line Edit" );
   le_makeiq_cutmax_pct->setText( (*parameters)[ "hplc_makeiq_cutmax_pct" ] );
   le_makeiq_cutmax_pct->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_makeiq_cutmax_pct->setPalette( PALET_NORMAL );
   AUTFBACK( le_makeiq_cutmax_pct );
   le_makeiq_cutmax_pct->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 0, 75, 1, le_makeiq_cutmax_pct );
      le_makeiq_cutmax_pct->setValidator( qdv );
   }
   connect( le_makeiq_cutmax_pct, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_makeiq_cutmax_pct->setMinimumWidth( 60 );

   cb_makeiq_avg_peaks = new QCheckBox(this);
   cb_makeiq_avg_peaks->setText( us_tr( "Make I(q): average and normalize top % of max intensity" ) );
   cb_makeiq_avg_peaks->setEnabled( true );
   cb_makeiq_avg_peaks->setChecked( (*parameters)[ "hplc_cb_makeiq_avg_peaks" ] == "true" );
   cb_makeiq_avg_peaks->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_makeiq_avg_peaks->setPalette( PALET_NORMAL );
   AUTFBACK( cb_makeiq_avg_peaks );
   connect( cb_makeiq_avg_peaks, SIGNAL( clicked() ), SLOT( update_enables() ) );
   cb_makeiq_avg_peaks->hide();

   le_makeiq_avg_peaks = new QLineEdit( this );    le_makeiq_avg_peaks->setObjectName( "le_makeiq_avg_peaks Line Edit" );
   le_makeiq_avg_peaks->setText( (*parameters)[ "hplc_makeiq_avg_peaks" ] );
   le_makeiq_avg_peaks->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_makeiq_avg_peaks->setPalette( PALET_NORMAL );
   AUTFBACK( le_makeiq_avg_peaks );
   le_makeiq_avg_peaks->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 1, 75, 1, le_makeiq_avg_peaks );
      le_makeiq_avg_peaks->setValidator( qdv );
   }
   connect( le_makeiq_avg_peaks, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_makeiq_avg_peaks->setMinimumWidth( 60 );
   le_makeiq_avg_peaks->hide();

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
   background->addWidget( lbl_title );

   background->addWidget( lbl_baseline );

   background->addWidget( rb_linear );
   background->addWidget( rb_integral );

   QGridLayout * gl_bl = new QGridLayout( 0 ); gl_bl->setContentsMargins( 0, 0, 0, 0 ); gl_bl->setSpacing( 0 );

   gl_bl->addWidget         ( lbl_smooth , 0, 0 );
   gl_bl->addWidget         ( le_smooth  , 0, 1 );
   gl_bl->addWidget         ( lbl_reps   , 1, 0 );
   gl_bl->addWidget         ( le_reps    , 1, 1 );
   gl_bl->addWidget         ( lbl_epsilon, 2, 0 );
   gl_bl->addWidget         ( le_epsilon , 2, 1 );
   gl_bl->addWidget         ( lbl_cormap_maxq  , 3, 0 );
   gl_bl->addWidget         ( le_cormap_maxq   , 3, 1 );
   gl_bl->addWidget         ( lbl_cormap_alpha , 4, 0 );
   gl_bl->addWidget         ( le_cormap_alpha  , 4, 1 );
   gl_bl->addWidget         ( lbl_start_region , 5, 0 );
   gl_bl->addWidget         ( le_start_region  , 5, 1 );
   gl_bl->addWidget         ( lbl_i_power , 6, 0 );
   gl_bl->addWidget         ( le_i_power  , 6, 1 );

   background->addLayout( gl_bl );
   background->addWidget( cb_save_bl );

   background->addWidget( lbl_gaussian_type );
   background->addWidget( rb_gauss );
   background->addWidget( rb_gmg );
   background->addWidget( rb_emg );
   background->addWidget( rb_emggmg );
   {
      QGridLayout * gl_other = new QGridLayout( 0 ); gl_other->setContentsMargins( 0, 0, 0, 0 ); gl_other->setSpacing( 0 );
      gl_other->addWidget         ( lbl_dist_max , 0, 0 );
      gl_other->addWidget         ( le_dist_max  , 0, 1 );

      gl_other->addWidget         ( lbl_ampl_width_min , 1, 0 );
      gl_other->addWidget         ( le_ampl_width_min  , 1, 1 );

      gl_other->addWidget         ( cb_lock_min_retry       , 2, 0 );
      gl_other->addWidget         ( le_lock_min_retry_mult  , 2, 1 );

      gl_other->addWidget         ( cb_maxfpk_restart       , 3, 0 );
      {
         QHBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
         // hbl->addWidget     ( le_maxfpk_restart_tries );
         hbl->addWidget     ( le_maxfpk_restart_pct );
         gl_other->addLayout( hbl , 3, 1 );
      }
      background->addLayout( gl_other );
   }

   background->addWidget( pb_clear_gauss );

   background->addWidget( lbl_other_options );
   background->addWidget( cb_csv_transposed );

   {
      QGridLayout * gl_other = new QGridLayout( 0 ); gl_other->setContentsMargins( 0, 0, 0, 0 ); gl_other->setSpacing( 0 );

      gl_other->addWidget         ( lbl_zi_window , 0, 0 );
      gl_other->addWidget         ( le_zi_window  , 0, 1 );

      gl_other->addWidget         ( cb_discard_it_sd_mult , 1, 0 );
      gl_other->addWidget         ( le_discard_it_sd_mult , 1, 1 );

      gl_other->addWidget         ( cb_guinier_qrgmax , 2, 0 );
      gl_other->addWidget         ( le_guinier_qrgmax , 2, 1 );

      gl_other->addWidget         ( lbl_mwt_k , 3, 0 );
      gl_other->addWidget         ( le_mwt_k  , 3, 1 );

      gl_other->addWidget         ( lbl_mwt_c , 4, 0 );
      gl_other->addWidget         ( le_mwt_c  , 4, 1 );

      gl_other->addWidget         ( lbl_mwt_qmax , 5, 0 );
      gl_other->addWidget         ( le_mwt_qmax  , 5, 1 );

      gl_other->addWidget         ( cb_makeiq_cutmax_pct , 6, 0 );
      gl_other->addWidget         ( le_makeiq_cutmax_pct , 6, 1 );

      gl_other->addWidget         ( cb_makeiq_avg_peaks , 7, 0 );
      gl_other->addWidget         ( le_makeiq_avg_peaks , 7, 1 );

      background->addLayout( gl_other );
   }

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

   if ( !parameters->count( "expert_mode" ) )
   {
      // lbl_baseline     ->hide();
      // rb_linear        ->hide();
      // rb_integral      ->hide();
      // lbl_smooth       ->hide();
      // le_smooth        ->hide();
      // lbl_reps         ->hide();
      // le_reps          ->hide();
      // lbl_epsilon      ->hide();
      // le_epsilon       ->hide();

      // lbl_gaussian_type->hide();
      // rb_gauss         ->hide();
      // rb_gmg           ->hide();
      // rb_emg           ->hide();
      // rb_emggmg        ->hide();

      lbl_ampl_width_min        ->hide();
      le_ampl_width_min         ->hide();
      cb_lock_min_retry         ->hide();
      le_lock_min_retry_mult    ->hide();
      cb_maxfpk_restart         ->hide();
      // le_maxfpk_restart_tries   ->hide();
      le_maxfpk_restart_pct     ->hide();

      lbl_i_power         ->hide();
      le_i_power          ->hide();
   }

   cb_save_bl       ->hide();
   lbl_start_region ->hide();
   le_start_region  ->hide();
}

void US_Hydrodyn_Saxs_Hplc_Options::quit()
{
   close();
}

void US_Hydrodyn_Saxs_Hplc_Options::ok()
{
   (*parameters)[ "ok" ] = "true";

   (*parameters)[ "hplc_bl_linear"               ] = rb_linear              ->isChecked() ? "true" : "false";
   (*parameters)[ "hplc_bl_integral"             ] = rb_integral            ->isChecked() ? "true" : "false";
   (*parameters)[ "hplc_bl_save"                 ] = cb_save_bl             ->isChecked() ? "true" : "false";
   (*parameters)[ "hplc_bl_smooth"               ] = le_smooth              ->text();
   (*parameters)[ "hplc_bl_start_region"         ] = le_start_region        ->text();
   (*parameters)[ "hplc_bl_i_power"              ] = le_i_power             ->text();
   (*parameters)[ "hplc_bl_reps"                 ] = le_reps                ->text();
   (*parameters)[ "hplc_bl_epsilon"              ] = le_epsilon             ->text();
   (*parameters)[ "hplc_cormap_maxq"             ] = le_cormap_maxq         ->text();
   (*parameters)[ "hplc_cormap_alpha"            ] = le_cormap_alpha        ->text();
   (*parameters)[ "hplc_zi_window"               ] = le_zi_window           ->text();
   (*parameters)[ "hplc_cb_discard_it_sd_mult"   ] = cb_discard_it_sd_mult  ->isChecked() ? "true" : "false";
   (*parameters)[ "hplc_discard_it_sd_mult"      ] = le_discard_it_sd_mult  ->text();
   (*parameters)[ "hplc_cb_guinier_qrgmax"       ] = cb_guinier_qrgmax      ->isChecked() ? "true" : "false";
   (*parameters)[ "hplc_guinier_qrgmax"          ] = le_guinier_qrgmax      ->text();
   (*parameters)[ "hplc_dist_max"                ] = le_dist_max            ->text();
   (*parameters)[ "guinier_mwt_k"                ] = le_mwt_k               ->text();
   (*parameters)[ "guinier_mwt_c"                ] = le_mwt_c               ->text();
   (*parameters)[ "guinier_mwt_qmax"             ] = le_mwt_qmax            ->text();
   (*parameters)[ "hplc_cb_makeiq_cutmax_pct"    ] = cb_makeiq_cutmax_pct   ->isChecked() ? "true" : "false";
   (*parameters)[ "hplc_makeiq_cutmax_pct"       ] = le_makeiq_cutmax_pct   ->text();
   (*parameters)[ "hplc_cb_makeiq_avg_peaks"     ] = cb_makeiq_avg_peaks    ->isChecked() ? "true" : "false";
   (*parameters)[ "hplc_makeiq_avg_peaks"        ] = le_makeiq_avg_peaks    ->text();

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
   
   (*parameters)[ "hplc_ampl_width_min"       ] = le_ampl_width_min      ->text();

   (*parameters)[ "hplc_lock_min_retry"       ] = cb_lock_min_retry->isChecked() ? "true" : "false";
   (*parameters)[ "hplc_lock_min_retry_mult"  ] = le_lock_min_retry_mult ->text();

   (*parameters)[ "hplc_maxfpk_restart"       ] = cb_maxfpk_restart->isChecked() ? "true" : "false";
   // (*parameters)[ "hplc_maxfpk_restart_tries" ] = le_maxfpk_restart_tries->text();
   (*parameters)[ "hplc_maxfpk_restart_pct"   ] = le_maxfpk_restart_pct  ->text();

   close();
}

void US_Hydrodyn_Saxs_Hplc_Options::help()
{
   US_Help *online_help;
   online_help = new US_Help( this );
   online_help->show_help("manual/somo/saxs_hplc_options.html");
}

void US_Hydrodyn_Saxs_Hplc_Options::closeEvent( QCloseEvent *e )
{

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Saxs_Hplc_Options::update_enables()
{
   le_smooth               ->setEnabled( rb_integral->isChecked() );
   le_reps                 ->setEnabled( rb_integral->isChecked() );
   cb_save_bl              ->setEnabled( rb_integral->isChecked() );
   le_epsilon              ->setEnabled( rb_integral->isChecked() );
   pb_clear_gauss          ->setEnabled( ((US_Hydrodyn_Saxs_Hplc *)hplc_win)->any_gaussians() );
   le_epsilon              ->setEnabled( rb_integral->isChecked() );
   le_discard_it_sd_mult   ->setEnabled( cb_discard_it_sd_mult->isChecked() );
   le_guinier_qrgmax       ->setEnabled( cb_guinier_qrgmax->isChecked() );
   le_makeiq_cutmax_pct    ->setEnabled( cb_makeiq_cutmax_pct->isChecked() );
   le_makeiq_avg_peaks     ->setEnabled( cb_makeiq_avg_peaks->isChecked() );
   if ( cb_makeiq_cutmax_pct->isEnabled() &&
        cb_makeiq_cutmax_pct->isChecked() &&
        le_makeiq_avg_peaks->text().toDouble() > 100.0 - le_makeiq_cutmax_pct->text().toDouble() ) {
      le_makeiq_avg_peaks->setText( QString( "%1" ).arg( 100.0 - le_makeiq_cutmax_pct->text().toDouble() ) );
   }
}

void US_Hydrodyn_Saxs_Hplc_Options::clear_gauss()
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

