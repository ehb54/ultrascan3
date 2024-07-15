#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_mals_saxs_options.h"
#include "../include/us_hydrodyn_mals_saxs_dctr.h"
#include "../include/us_hydrodyn_mals_saxs_parameters.h"
#include "../include/us_hydrodyn_mals_saxs.h"
#include "../include/us_mqt.h"
//Added by qt3to4:
#include <QHBoxLayout>
#include <QCloseEvent>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>

US_Hydrodyn_Mals_Saxs_Options::US_Hydrodyn_Mals_Saxs_Options(
                                                             map < QString, QString > * parameters,
                                                             US_Hydrodyn              * us_hydrodyn,
                                                             QWidget *                  p
                                                             ) : QDialog( p )
{
   this->mals_saxs_win    = p;
   this->parameters  = parameters;
   this->us_hydrodyn = us_hydrodyn;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle( us_tr( "US-SOMO: MALS+SAXS : Options" ) );

   setupGUI();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );

   update_enables();
}

US_Hydrodyn_Mals_Saxs_Options::~US_Hydrodyn_Mals_Saxs_Options()
{
}

void US_Hydrodyn_Mals_Saxs_Options::setupGUI()
{
   int minHeight1  = 28;

   lbl_title =  new QLabel      ( us_tr( "US-SOMO: MALS+SAXS : Options" ), this );
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
   rb_linear->setChecked( parameters->count( "mals_saxs_bl_linear" ) && (*parameters)[ "mals_saxs_bl_linear" ] == "true" );
   rb_linear->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_linear->setPalette( PALET_NORMAL );
   AUTFBACK( rb_linear );
   connect( rb_linear, SIGNAL( clicked() ), SLOT( update_enables() ) );

   rb_integral = new QRadioButton( us_tr("Integral of I(t) baseline removal"), this);
   rb_integral->setEnabled(true);
   rb_integral->setChecked( parameters->count( "mals_saxs_bl_integral" ) && (*parameters)[ "mals_saxs_bl_integral" ] == "true" );
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
   cb_save_bl->setChecked( parameters->count( "mals_saxs_bl_save" ) && (*parameters)[ "mals_saxs_bl_save" ] == "true" );
   cb_save_bl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_save_bl->setPalette( PALET_NORMAL );
   AUTFBACK( cb_save_bl );

   lbl_smooth =  new QLabel      ( us_tr( "Smoothing:" ), this );
   lbl_smooth -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_smooth -> setPalette      ( PALET_LABEL );
   AUTFBACK( lbl_smooth );
   lbl_smooth -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_smooth = new QLineEdit( this );    le_smooth->setObjectName( "le_smooth Line Edit" );
   le_smooth->setText( parameters->count( "mals_saxs_bl_smooth" ) ? (*parameters)[ "mals_saxs_bl_smooth" ] : "10" );
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
   le_start_region->setText( parameters->count( "mals_saxs_bl_start_region" ) ? (*parameters)[ "mals_saxs_bl_start_region" ] : "10" );
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
   le_i_power->setText( parameters->count( "mals_saxs_bl_i_power" ) ? (*parameters)[ "mals_saxs_bl_i_power" ] : "1" );
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
   le_reps->setText( parameters->count( "mals_saxs_bl_reps" ) ? (*parameters)[ "mals_saxs_bl_reps" ] : "1" );
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
   le_epsilon->setText( parameters->count( "mals_saxs_bl_epsilon" ) ? (*parameters)[ "mals_saxs_bl_epsilon" ] : "1" );
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

   lbl_cormap_maxq =  new QLabel      ( us_tr( "Global PVP Analysis maximum q [A^-1]:" ), this );
   lbl_cormap_maxq -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_cormap_maxq -> setPalette( PALET_LABEL );
   AUTFBACK( lbl_cormap_maxq );
   lbl_cormap_maxq -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_cormap_maxq = new QLineEdit( this );    le_cormap_maxq->setObjectName( "le_cormap_maxq Line Edit" );
   le_cormap_maxq->setText( parameters->count( "mals_saxs_cormap_maxq" ) ? (*parameters)[ "mals_saxs_cormap_maxq" ] : "0.05" );
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

   lbl_cormap_alpha =  new QLabel      ( us_tr( "Global PVP Analysis alpha:" ), this );
   lbl_cormap_alpha -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_cormap_alpha -> setPalette( PALET_LABEL );
   AUTFBACK( lbl_cormap_alpha );
   lbl_cormap_alpha -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_cormap_alpha = new QLineEdit( this );    le_cormap_alpha->setObjectName( "le_cormap_alpha Line Edit" );
   le_cormap_alpha->setText( parameters->count( "mals_saxs_cormap_alpha" ) ? (*parameters)[ "mals_saxs_cormap_alpha" ] : "0.01" );
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
   pb_clear_gauss -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1) );
   pb_clear_gauss -> setMinimumHeight( minHeight1 );
   pb_clear_gauss -> setPalette      ( PALET_PUSHB );
   connect( pb_clear_gauss, SIGNAL( clicked() ), SLOT( clear_gauss() ) );

   lbl_other_options = new QLabel ( us_tr( "Miscellaneous options" ), this);
   lbl_other_options->setAlignment( Qt::AlignCenter | Qt::AlignVCenter);
   lbl_other_options->setPalette  ( PALET_FRAME );
   AUTFBACK( lbl_other_options );
   lbl_other_options->setFont     ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   lbl_interp_method = new QLabel( us_tr( " Interpolation method:" ) );
   lbl_interp_method->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_interp_method->setPalette( PALET_LABEL );
   AUTFBACK( lbl_interp_method );
   lbl_interp_method-> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   cb_interp_method = new QComboBox( this );
   cb_interp_method->setPalette( PALET_NORMAL );
   AUTFBACK( cb_interp_method );
   cb_interp_method->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   cb_interp_method->setEnabled(true);
   cb_interp_method->setMinimumHeight( minHeight1 );
   cb_interp_method->setMaxVisibleItems( 1 );

   cb_interp_method->addItem( us_tr( "Linear" ), INTERP_METHOD_LINEAR );
   cb_interp_method->addItem( us_tr( "Quadratic" ), INTERP_METHOD_QUADRATIC );
   cb_interp_method->addItem( us_tr( "Cubic Spline" ), INTERP_METHOD_CUBIC_SPLINE );

   {
      int index = 0;
      if ( parameters->count( "mals_saxs_interp_method" ) ) {
         switch ( (*parameters)[ "mals_saxs_interp_method" ].toInt() ) {
         case INTERP_METHOD_LINEAR :
            index = 0;
            break;
         case INTERP_METHOD_QUADRATIC :
            index = 1;
            break;
         case INTERP_METHOD_CUBIC_SPLINE :
            index = 2;
            break;
         default:
            break;
         }
      }
      
      cb_interp_method->setCurrentIndex( index );
   }

   pb_detector = new QPushButton(us_tr("Concentration Detector Properties"), this);
   pb_detector->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_detector->setMinimumHeight(minHeight1);
   pb_detector->setPalette( PALET_PUSHB );
   connect(pb_detector, SIGNAL(clicked()), SLOT(set_detector()));

   pb_mals_saxs_parameters = new QPushButton(us_tr("MALS+SAXS Processing Parameters"), this);
   pb_mals_saxs_parameters->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_mals_saxs_parameters->setMinimumHeight(minHeight1);
   pb_mals_saxs_parameters->setPalette( PALET_PUSHB );
   connect(pb_mals_saxs_parameters, SIGNAL(clicked()), SLOT(set_mals_saxs_parameters()));

   pb_fasta_file = new QPushButton(us_tr("Load FASTA sequence from file"), this);
   pb_fasta_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ));
   pb_fasta_file->setMinimumHeight(minHeight1);
   pb_fasta_file->setPalette( PALET_PUSHB );
   connect(pb_fasta_file, SIGNAL(clicked()), SLOT(fasta_file()));

   lbl_fasta_pH = new QLabel( us_tr( " pH:" ) );
   lbl_fasta_pH->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_fasta_pH->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_fasta_pH );
   lbl_fasta_pH->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_fasta_pH = new QLineEdit( this );
   le_fasta_pH->setObjectName( "le_fasta_pH Line Edit" );
   le_fasta_pH->setText( QString( "" ).sprintf( "%4.2f", ((US_Hydrodyn *)us_hydrodyn)->hydro.pH ) );
   le_fasta_pH->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   le_fasta_pH->setPalette( PALET_EDIT );
   AUTFBACK( le_fasta_pH );
   le_fasta_pH->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   connect(le_fasta_pH, SIGNAL(textChanged(const QString &)), SLOT(update_fasta_pH(const QString &)));

   lbl_fasta_value = new QLabel( us_tr( " PSV [cm^3/g]:" ) );
   lbl_fasta_value->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_fasta_value->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_fasta_value );
   lbl_fasta_value->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   le_fasta_value = new QLineEdit( this );
   le_fasta_value->setObjectName( "le_fasta_value Line Edit" );
   le_fasta_value->setText( "" );
   le_fasta_value->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   le_fasta_value->setPalette( PALET_EDIT );
   AUTFBACK( le_fasta_value );
   le_fasta_value->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_fasta_value->setEnabled( false );
   le_fasta_value->setReadOnly( true );

   cb_csv_transposed = new QCheckBox(this);
   cb_csv_transposed->setText(us_tr( "Save CSV transposed"));
   cb_csv_transposed->setEnabled( true );
   cb_csv_transposed->setChecked( (*parameters)[ "mals_saxs_csv_transposed" ] == "true" );
   cb_csv_transposed->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_csv_transposed->setPalette( PALET_NORMAL );
   AUTFBACK( cb_csv_transposed );

   lbl_zi_window =  new QLabel      ( us_tr( "I(t) negative integral check window :" ), this );
   lbl_zi_window -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_zi_window -> setPalette( PALET_LABEL );
   AUTFBACK( lbl_zi_window );
   lbl_zi_window -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_zi_window = new QLineEdit( this );    le_zi_window->setObjectName( "le_zi_window Line Edit" );
   le_zi_window->setText( (*parameters)[ "mals_saxs_zi_window" ] );
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
   cb_discard_it_sd_mult->setChecked( (*parameters)[ "mals_saxs_cb_discard_it_sd_mult" ] == "true" );
   cb_discard_it_sd_mult->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_discard_it_sd_mult->setPalette( PALET_NORMAL );
   AUTFBACK( cb_discard_it_sd_mult );
   connect( cb_discard_it_sd_mult, SIGNAL( clicked() ), SLOT( update_enables() ) );

   le_discard_it_sd_mult = new QLineEdit( this );    le_discard_it_sd_mult->setObjectName( "le_discard_it_sd_mult Line Edit" );
   le_discard_it_sd_mult->setText( (*parameters)[ "mals_saxs_discard_it_sd_mult" ] );
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
   cb_guinier_qrgmax->setChecked( (*parameters)[ "mals_saxs_cb_guinier_qrgmax" ] == "true" );
   cb_guinier_qrgmax->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_guinier_qrgmax->setPalette( PALET_NORMAL );
   AUTFBACK( cb_guinier_qrgmax );
   connect( cb_guinier_qrgmax, SIGNAL( clicked() ), SLOT( update_enables() ) );

   le_guinier_qrgmax = new QLineEdit( this );    le_guinier_qrgmax->setObjectName( "le_guinier_qrgmax Line Edit" );
   le_guinier_qrgmax->setText( (*parameters)[ "mals_saxs_guinier_qrgmax" ] );
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

   cb_gg_smooth = new QCheckBox(this);
   cb_gg_smooth->setText(us_tr( "Experimental: Global Gaussian initialization smoothing. Maximum smoothing points: "));
   cb_gg_smooth->setEnabled( true );
   cb_gg_smooth->setChecked( (*parameters)[ "mals_saxs_cb_gg_smooth" ] == "true" );
   cb_gg_smooth->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_gg_smooth->setPalette( PALET_NORMAL );
   AUTFBACK( cb_gg_smooth );
   connect( cb_gg_smooth, SIGNAL( clicked() ), SLOT( update_enables() ) );

   le_gg_smooth = new QLineEdit( this );    le_gg_smooth->setObjectName( "le_gg_smooth Line Edit" );
   le_gg_smooth->setText( (*parameters)[ "mals_saxs_gg_smooth" ] );
   le_gg_smooth->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_gg_smooth->setPalette( PALET_NORMAL );
   AUTFBACK( le_gg_smooth );
   le_gg_smooth->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QIntValidator *qiv = new QIntValidator( 1, 50, le_gg_smooth );
      le_gg_smooth->setValidator( qiv );
   }
   connect( le_gg_smooth, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_gg_smooth->setMinimumWidth( 60 );

   cb_gg_cyclic = new QCheckBox(this);
   cb_gg_cyclic->setText(us_tr( "Experimental: Global Gaussian Gaussian cyclic fit"));
   cb_gg_cyclic->setEnabled( true );
   cb_gg_cyclic->setChecked( (*parameters)[ "mals_saxs_cb_gg_cyclic" ] == "true" );
   cb_gg_cyclic->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_gg_cyclic->setPalette( PALET_NORMAL );
   AUTFBACK( cb_gg_cyclic );
   connect( cb_gg_cyclic, SIGNAL( clicked() ), SLOT( update_enables() ) );

   cb_gg_oldstyle = new QCheckBox(this);
   cb_gg_oldstyle->setText(us_tr( "Experimental: Global Gaussian - Enable old style Gaussian fit display"));
   cb_gg_oldstyle->setEnabled( true );
   cb_gg_oldstyle->setChecked( (*parameters)[ "mals_saxs_cb_gg_oldstyle" ] == "true" );
   cb_gg_oldstyle->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_gg_oldstyle->setPalette( PALET_NORMAL );
   AUTFBACK( cb_gg_oldstyle );
   connect( cb_gg_oldstyle, SIGNAL( clicked() ), SLOT( update_enables() ) );

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
   le_dist_max->setText( (*parameters)[ "mals_saxs_dist_max" ] );
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
   le_ampl_width_min->setText( (*parameters)[ "mals_saxs_ampl_width_min" ] );
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
   cb_lock_min_retry->setChecked( (*parameters)[ "mals_saxs_lock_min_retry" ] == "true" );
   cb_lock_min_retry->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_lock_min_retry->setPalette( PALET_NORMAL );
   AUTFBACK( cb_lock_min_retry );

   le_lock_min_retry_mult = new QLineEdit( this );    le_lock_min_retry_mult->setObjectName( "le_lock_min_retry_mult Line Edit" );
   le_lock_min_retry_mult->setText( (*parameters)[ "mals_saxs_lock_min_retry_mult" ] );
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
   cb_maxfpk_restart->setChecked( (*parameters)[ "mals_saxs_maxfpk_restart" ] == "true" );
   cb_maxfpk_restart->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_maxfpk_restart->setPalette( PALET_NORMAL );
   AUTFBACK( cb_maxfpk_restart );

   // le_maxfpk_restart_tries = new QLineEdit( this );  le_maxfpk_restart_tries->setObjectName( "le_maxfpk_restart_tries Line Edit" );
   // le_maxfpk_restart_tries->setText( (*parameters)[ "mals_saxs_maxfpk_restart_tries" ] );
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
   le_maxfpk_restart_pct->setText( (*parameters)[ "mals_saxs_maxfpk_restart_pct" ] );
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
   cb_makeiq_cutmax_pct->setChecked( (*parameters)[ "mals_saxs_cb_makeiq_cutmax_pct" ] == "true" );
   cb_makeiq_cutmax_pct->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_makeiq_cutmax_pct->setPalette( PALET_NORMAL );
   AUTFBACK( cb_makeiq_cutmax_pct );
   connect( cb_makeiq_cutmax_pct, SIGNAL( clicked() ), SLOT( update_enables() ) );

   le_makeiq_cutmax_pct = new QLineEdit( this );    le_makeiq_cutmax_pct->setObjectName( "le_makeiq_cutmax_pct Line Edit" );
   le_makeiq_cutmax_pct->setText( (*parameters)[ "mals_saxs_makeiq_cutmax_pct" ] );
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
   cb_makeiq_avg_peaks->setChecked( (*parameters)[ "mals_saxs_cb_makeiq_avg_peaks" ] == "true" );
   cb_makeiq_avg_peaks->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_makeiq_avg_peaks->setPalette( PALET_NORMAL );
   AUTFBACK( cb_makeiq_avg_peaks );
   connect( cb_makeiq_avg_peaks, SIGNAL( clicked() ), SLOT( update_enables() ) );
   cb_makeiq_avg_peaks->hide();

   le_makeiq_avg_peaks = new QLineEdit( this );    le_makeiq_avg_peaks->setObjectName( "le_makeiq_avg_peaks Line Edit" );
   le_makeiq_avg_peaks->setText( (*parameters)[ "mals_saxs_makeiq_avg_peaks" ] );
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

   {
      QHBoxLayout * hbl = new QHBoxLayout( 0 );
      hbl->setContentsMargins( 0, 0, 0, 0 );
      hbl->setSpacing( 0 );
      
      hbl->addWidget( rb_gauss );
      hbl->addWidget( rb_gmg );
      hbl->addWidget( rb_emg );
      hbl->addWidget( rb_emggmg );

      background->addLayout( hbl );
   }
   
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

   {
      QHBoxLayout * hbl = new QHBoxLayout( 0 );
      hbl->setContentsMargins( 0, 0, 0, 0 );
      hbl->setSpacing( 0 );
      
      hbl->addWidget( pb_detector );
      hbl->addWidget( pb_mals_saxs_parameters );
      
      background->addLayout( hbl );
   }

   {
      QBoxLayout * bl_fasta = new QHBoxLayout( 0 );
      bl_fasta->setContentsMargins( 0, 0, 0, 0 );
      bl_fasta->setSpacing( 0 );

      bl_fasta->addWidget( pb_fasta_file );
      bl_fasta->addWidget( lbl_fasta_pH );
      bl_fasta->addWidget( le_fasta_pH );
      bl_fasta->addWidget( lbl_fasta_value );
      bl_fasta->addWidget( le_fasta_value );

      background->addLayout( bl_fasta );
   }
      
   background->addWidget( cb_csv_transposed );

   {
      QGridLayout * gl_other = new QGridLayout( 0 ); gl_other->setContentsMargins( 0, 0, 0, 0 ); gl_other->setSpacing( 0 );

      int row = 0;

      gl_other->addWidget         ( lbl_interp_method , row, 0 );
      gl_other->addWidget         ( cb_interp_method  , row, 1 );

      gl_other->addWidget         ( lbl_zi_window , ++row, 0 );
      gl_other->addWidget         ( le_zi_window  , row, 1 );

      gl_other->addWidget         ( cb_discard_it_sd_mult , ++row, 0 );
      gl_other->addWidget         ( le_discard_it_sd_mult , row, 1 );

      gl_other->addWidget         ( cb_guinier_qrgmax , ++row, 0 );
      gl_other->addWidget         ( le_guinier_qrgmax , row, 1 );

      gl_other->addWidget         ( cb_gg_smooth , ++row, 0 );
      gl_other->addWidget         ( le_gg_smooth , row, 1 );

      ++row;  gl_other->addWidget ( cb_gg_cyclic , row, 0, 1, 2 );

      ++row;  gl_other->addWidget ( cb_gg_oldstyle , row, 0, 1, 2 );

      gl_other->addWidget         ( lbl_mwt_k , ++row, 0 );
      gl_other->addWidget         ( le_mwt_k  , row, 1 );

      gl_other->addWidget         ( lbl_mwt_c , ++row, 0 );
      gl_other->addWidget         ( le_mwt_c  , row, 1 );

      gl_other->addWidget         ( lbl_mwt_qmax , ++row, 0 );
      gl_other->addWidget         ( le_mwt_qmax  , row, 1 );

      gl_other->addWidget         ( cb_makeiq_cutmax_pct , ++row, 0 );
      gl_other->addWidget         ( le_makeiq_cutmax_pct , row, 1 );

      gl_other->addWidget         ( cb_makeiq_avg_peaks , ++row, 0 );
      gl_other->addWidget         ( le_makeiq_avg_peaks , row, 1 );

      background->addLayout( gl_other );
   }

   if ( !parameters->count( "gaussian_type" ) )
   {
      rb_gauss->setChecked( true );
   } else {
      switch( (*parameters)[ "gaussian_type" ].toInt() )
      {
      case US_Hydrodyn_Mals_Saxs::GMG :
         rb_gmg->setChecked( true );
         break;
      case US_Hydrodyn_Mals_Saxs::EMG :
         rb_emg->setChecked( true );
         break;
      case US_Hydrodyn_Mals_Saxs::EMGGMG :
         rb_emggmg->setChecked( true );
         break;
      case US_Hydrodyn_Mals_Saxs::GAUSS :
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

   always_hide_widgets.insert(
                              {
                                 lbl_baseline
                                    ,rb_linear
                                    ,rb_integral
                                    ,lbl_smooth
                                    ,le_smooth
                                    ,lbl_reps
                                    ,le_reps
                                    ,lbl_epsilon
                                    ,le_epsilon
                                    ,lbl_cormap_maxq
                                    ,le_cormap_maxq
                                    ,lbl_cormap_alpha
                                    ,le_cormap_alpha
                                    ,lbl_gaussian_type
                                    ,rb_gauss
                                    ,rb_gmg
                                    ,rb_emg
                                    ,rb_emggmg
                                    ,lbl_dist_max
                                    ,le_dist_max
                                    ,pb_clear_gauss
                                    ,pb_detector
                                    ,pb_mals_saxs_parameters
                                    ,pb_fasta_file
                                    ,lbl_fasta_pH
                                    ,le_fasta_pH
                                    ,lbl_fasta_value
                                    ,le_fasta_value
                                    ,lbl_zi_window
                                    ,le_zi_window
                                    ,cb_discard_it_sd_mult
                                    ,le_discard_it_sd_mult
                                    ,cb_guinier_qrgmax
                                    ,le_guinier_qrgmax
                                    ,cb_gg_smooth
                                    ,le_gg_smooth
                                    ,cb_gg_cyclic
                                    ,cb_gg_oldstyle
                                    ,lbl_mwt_k
                                    ,le_mwt_k
                                    ,lbl_mwt_c
                                    ,le_mwt_c
                                    ,lbl_mwt_qmax
                                    ,le_mwt_qmax
                                    ,cb_makeiq_cutmax_pct
                                    ,le_makeiq_cutmax_pct
                                    }
                              );
   
   ShowHide::hide_widgets( always_hide_widgets );
}

void US_Hydrodyn_Mals_Saxs_Options::quit()
{
   close();
}

bool US_Hydrodyn_Mals_Saxs_Options::any_changes()
{
   QString gaussian_type = "";
   if ( rb_gauss->isChecked() ) {
      gaussian_type = QString( "%1" ).arg( US_Hydrodyn_Mals_Saxs::GAUSS );
   } else if ( rb_gmg->isChecked() ) {
      gaussian_type = QString( "%1" ).arg( US_Hydrodyn_Mals_Saxs::GMG );
   } else if ( rb_emg->isChecked() ) {
      gaussian_type = QString( "%1" ).arg( US_Hydrodyn_Mals_Saxs::EMG );
   } else if ( rb_emggmg->isChecked() ) {
      gaussian_type = QString( "%1" ).arg( US_Hydrodyn_Mals_Saxs::EMGGMG );
   }

   return
      (*parameters)[ "mals_saxs_bl_linear"               ] != ( rb_linear              ->isChecked() ? "true" : "false" )
      || (*parameters)[ "mals_saxs_bl_integral"             ] != ( rb_integral            ->isChecked() ? "true" : "false" )
      || (*parameters)[ "mals_saxs_bl_save"                 ] != ( cb_save_bl             ->isChecked() ? "true" : "false" )
      || (*parameters)[ "mals_saxs_bl_smooth"               ] != ( le_smooth              ->text() )
      || (*parameters)[ "mals_saxs_bl_start_region"         ] != ( le_start_region        ->text() )
      || (*parameters)[ "mals_saxs_bl_i_power"              ] != ( le_i_power             ->text() )
      || (*parameters)[ "mals_saxs_bl_reps"                 ] != ( le_reps                ->text() )
      || (*parameters)[ "mals_saxs_bl_epsilon"              ] != ( le_epsilon             ->text() )
      || (*parameters)[ "mals_saxs_cormap_maxq"             ] != ( le_cormap_maxq         ->text() )
      || (*parameters)[ "mals_saxs_cormap_alpha"            ] != ( le_cormap_alpha        ->text() )
      || (*parameters)[ "mals_saxs_zi_window"               ] != ( le_zi_window           ->text() )
      || (*parameters)[ "mals_saxs_cb_discard_it_sd_mult"   ] != ( cb_discard_it_sd_mult  ->isChecked() ? "true" : "false" )
      || (*parameters)[ "mals_saxs_discard_it_sd_mult"      ] != ( le_discard_it_sd_mult  ->text() )
      || (*parameters)[ "mals_saxs_cb_guinier_qrgmax"       ] != ( cb_guinier_qrgmax      ->isChecked() ? "true" : "false" )
      || (*parameters)[ "mals_saxs_guinier_qrgmax"          ] != ( le_guinier_qrgmax      ->text() )
      || (*parameters)[ "mals_saxs_cb_gg_smooth"            ] != ( cb_gg_smooth  ->isChecked() ? "true" : "false" )
      || (*parameters)[ "mals_saxs_gg_smooth"               ] != ( le_gg_smooth  ->text() )
      || (*parameters)[ "mals_saxs_cb_gg_cyclic"            ] != ( cb_gg_cyclic  ->isChecked() ? "true" : "false" )
      || (*parameters)[ "mals_saxs_cb_gg_oldstyle"          ] != ( cb_gg_oldstyle  ->isChecked() ? "true" : "false" )
      || (*parameters)[ "mals_saxs_dist_max"                ] != ( le_dist_max            ->text() )
      || (*parameters)[ "guinier_mwt_k"                ] != ( le_mwt_k               ->text() )
      || (*parameters)[ "guinier_mwt_c"                ] != ( le_mwt_c               ->text() )
      || (*parameters)[ "guinier_mwt_qmax"             ] != ( le_mwt_qmax            ->text() )
      || (*parameters)[ "mals_saxs_cb_makeiq_cutmax_pct"    ] != ( cb_makeiq_cutmax_pct   ->isChecked() ? "true" : "false" )
      || (*parameters)[ "mals_saxs_makeiq_cutmax_pct"       ] != ( le_makeiq_cutmax_pct   ->text() )
      || (*parameters)[ "mals_saxs_cb_makeiq_avg_peaks"     ] != ( cb_makeiq_avg_peaks    ->isChecked() ? "true" : "false" )
      || (*parameters)[ "mals_saxs_makeiq_avg_peaks"        ] != ( le_makeiq_avg_peaks    ->text() )
      || (*parameters)[ "mals_saxs_csv_transposed"          ] != ( cb_csv_transposed->isChecked() ? "true" : "false" )
      || (*parameters)[ "mals_saxs_ampl_width_min"          ] != ( le_ampl_width_min      ->text() )
      || (*parameters)[ "mals_saxs_lock_min_retry"          ] != ( cb_lock_min_retry->isChecked() ? "true" : "false" )
      || (*parameters)[ "mals_saxs_lock_min_retry_mult"     ] != ( le_lock_min_retry_mult ->text() )
      || (*parameters)[ "mals_saxs_maxfpk_restart"          ] != ( cb_maxfpk_restart->isChecked() ? "true" : "false" )
      || (*parameters)[ "mals_saxs_maxfpk_restart_pct"      ] != ( le_maxfpk_restart_pct  ->text() )
      || (*parameters)[ "gaussian_type"                ] != ( gaussian_type )
      ;
}

void US_Hydrodyn_Mals_Saxs_Options::ok()
{
   (*parameters)[ "ok" ] = "true";

   (*parameters)[ "mals_saxs_bl_linear"               ] = rb_linear              ->isChecked() ? "true" : "false";
   (*parameters)[ "mals_saxs_bl_integral"             ] = rb_integral            ->isChecked() ? "true" : "false";
   (*parameters)[ "mals_saxs_bl_save"                 ] = cb_save_bl             ->isChecked() ? "true" : "false";
   (*parameters)[ "mals_saxs_bl_smooth"               ] = le_smooth              ->text();
   (*parameters)[ "mals_saxs_bl_start_region"         ] = le_start_region        ->text();
   (*parameters)[ "mals_saxs_bl_i_power"              ] = le_i_power             ->text();
   (*parameters)[ "mals_saxs_bl_reps"                 ] = le_reps                ->text();
   (*parameters)[ "mals_saxs_bl_epsilon"              ] = le_epsilon             ->text();
   (*parameters)[ "mals_saxs_cormap_maxq"             ] = le_cormap_maxq         ->text();
   (*parameters)[ "mals_saxs_cormap_alpha"            ] = le_cormap_alpha        ->text();
   (*parameters)[ "mals_saxs_zi_window"               ] = le_zi_window           ->text();
   (*parameters)[ "mals_saxs_cb_discard_it_sd_mult"   ] = cb_discard_it_sd_mult  ->isChecked() ? "true" : "false";
   (*parameters)[ "mals_saxs_discard_it_sd_mult"      ] = le_discard_it_sd_mult  ->text();
   (*parameters)[ "mals_saxs_cb_guinier_qrgmax"       ] = cb_guinier_qrgmax      ->isChecked() ? "true" : "false";
   (*parameters)[ "mals_saxs_guinier_qrgmax"          ] = le_guinier_qrgmax      ->text();
   (*parameters)[ "mals_saxs_cb_gg_smooth"            ] = cb_gg_smooth  ->isChecked() ? "true" : "false";
   (*parameters)[ "mals_saxs_gg_smooth"               ] = le_gg_smooth  ->text();
   (*parameters)[ "mals_saxs_cb_gg_cyclic"            ] = cb_gg_cyclic  ->isChecked() ? "true" : "false";
   (*parameters)[ "mals_saxs_cb_gg_oldstyle"          ] = cb_gg_oldstyle  ->isChecked() ? "true" : "false";
   (*parameters)[ "mals_saxs_dist_max"                ] = le_dist_max            ->text();
   (*parameters)[ "guinier_mwt_k"                     ] = le_mwt_k               ->text();
   (*parameters)[ "guinier_mwt_c"                     ] = le_mwt_c               ->text();
   (*parameters)[ "guinier_mwt_qmax"                  ] = le_mwt_qmax            ->text();
   (*parameters)[ "mals_saxs_cb_makeiq_cutmax_pct"    ] = cb_makeiq_cutmax_pct   ->isChecked() ? "true" : "false";
   (*parameters)[ "mals_saxs_makeiq_cutmax_pct"       ] = le_makeiq_cutmax_pct   ->text();
   (*parameters)[ "mals_saxs_cb_makeiq_avg_peaks"     ] = cb_makeiq_avg_peaks    ->isChecked() ? "true" : "false";
   (*parameters)[ "mals_saxs_makeiq_avg_peaks"        ] = le_makeiq_avg_peaks    ->text();
   (*parameters)[ "mals_saxs_interp_method"           ] = QString( "%1" ).arg( cb_interp_method->currentData().toInt() );

   if ( rb_gauss->isChecked() )
   {
      (*parameters)[ "gaussian_type" ] = QString( "%1" ).arg( US_Hydrodyn_Mals_Saxs::GAUSS );
   }
   if ( rb_gmg->isChecked() )
   {
      (*parameters)[ "gaussian_type" ] = QString( "%1" ).arg( US_Hydrodyn_Mals_Saxs::GMG );
   }
   if ( rb_emg->isChecked() )
   {
      (*parameters)[ "gaussian_type" ] = QString( "%1" ).arg( US_Hydrodyn_Mals_Saxs::EMG );
   }
   if ( rb_emggmg->isChecked() )
   {
      (*parameters)[ "gaussian_type" ] = QString( "%1" ).arg( US_Hydrodyn_Mals_Saxs::EMGGMG );
   }

   (*parameters)[ "mals_saxs_csv_transposed" ] = cb_csv_transposed->isChecked() ? "true" : "false";
   
   (*parameters)[ "mals_saxs_ampl_width_min"       ] = le_ampl_width_min      ->text();

   (*parameters)[ "mals_saxs_lock_min_retry"       ] = cb_lock_min_retry->isChecked() ? "true" : "false";
   (*parameters)[ "mals_saxs_lock_min_retry_mult"  ] = le_lock_min_retry_mult ->text();

   (*parameters)[ "mals_saxs_maxfpk_restart"       ] = cb_maxfpk_restart->isChecked() ? "true" : "false";
   // (*parameters)[ "mals_saxs_maxfpk_restart_tries" ] = le_maxfpk_restart_tries->text();
   (*parameters)[ "mals_saxs_maxfpk_restart_pct"   ] = le_maxfpk_restart_pct  ->text();

   close();
}

void US_Hydrodyn_Mals_Saxs_Options::help()
{
   US_Help *online_help;
   online_help = new US_Help( this );
   online_help->show_help("manual/somo/mals_saxs_options.html");
}

void US_Hydrodyn_Mals_Saxs_Options::closeEvent( QCloseEvent *e )
{
   if ( any_changes() ) {
      QMessageBox mb( this->windowTitle(), 
                      us_tr("Attention:\nAre you sure you wish to discard your changes?"),
                      QMessageBox::Information,
                      QMessageBox::Yes | QMessageBox::Default,
                      QMessageBox::Cancel | QMessageBox::Escape,
                      QMessageBox::NoButton);
      mb.setButtonText(QMessageBox::Yes, us_tr("Yes"));
      mb.setButtonText(QMessageBox::Cancel, us_tr("Cancel"));
      switch(mb.exec())
      {
      case QMessageBox::Cancel:
         {
            e->ignore();
            return;
         }
      }
   }

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Mals_Saxs_Options::update_enables()
{
   le_smooth               ->setEnabled( rb_integral->isChecked() );
   le_reps                 ->setEnabled( rb_integral->isChecked() );
   cb_save_bl              ->setEnabled( rb_integral->isChecked() );
   le_epsilon              ->setEnabled( rb_integral->isChecked() );
   pb_clear_gauss          ->setEnabled( ((US_Hydrodyn_Mals_Saxs *)mals_saxs_win)->any_gaussians() );
   le_epsilon              ->setEnabled( rb_integral->isChecked() );
   le_discard_it_sd_mult   ->setEnabled( cb_discard_it_sd_mult->isChecked() );
   le_guinier_qrgmax       ->setEnabled( cb_guinier_qrgmax->isChecked() );
   le_gg_smooth            ->setEnabled( cb_gg_smooth->isChecked() );
   le_makeiq_cutmax_pct    ->setEnabled( cb_makeiq_cutmax_pct->isChecked() );
   le_makeiq_avg_peaks     ->setEnabled( cb_makeiq_avg_peaks->isChecked() );
   if ( cb_makeiq_cutmax_pct->isEnabled() &&
        cb_makeiq_cutmax_pct->isChecked() &&
        le_makeiq_avg_peaks->text().toDouble() > 100.0 - le_makeiq_cutmax_pct->text().toDouble() ) {
      le_makeiq_avg_peaks->setText( QString( "%1" ).arg( 100.0 - le_makeiq_cutmax_pct->text().toDouble() ) );
   }
}

void US_Hydrodyn_Mals_Saxs_Options::clear_gauss()
{
   if ( QMessageBox::Yes == QMessageBox::question(
                                                  this,
                                                  windowTitle() + us_tr( ": Clear cached Gaussian values" ),
                                                  us_tr("Are you sure you want to clear all cached Gaussian values?" ),
                                                  QMessageBox::Yes, 
                                                  QMessageBox::No | QMessageBox::Default
                                                  ) )
   {
      ((US_Hydrodyn_Mals_Saxs *)mals_saxs_win)->clear_gaussians();
   }
   update_enables();
}

void US_Hydrodyn_Mals_Saxs_Options::set_detector()
{
   {
      map < QString, QString > parameters;
      parameters[ "uv_conv" ] = QString( "%1" ).arg( ((US_Hydrodyn_Mals_Saxs *)mals_saxs_win)->detector_uv_conv, 0, 'g', 8 );
      parameters[ "ri_conv" ] = QString( "%1" ).arg( ((US_Hydrodyn_Mals_Saxs *)mals_saxs_win)->detector_ri_conv, 0, 'g', 8 );
      if ( ((US_Hydrodyn_Mals_Saxs *)mals_saxs_win)->detector_uv )
      {
         parameters[ "uv" ] = "true";
      } else {
         if ( ((US_Hydrodyn_Mals_Saxs *)mals_saxs_win)->detector_ri )
         {
            parameters[ "ri" ] = "true";
         }
      }

      US_Hydrodyn_Mals_Saxs_Dctr *mals_saxs_dctr = 
         new US_Hydrodyn_Mals_Saxs_Dctr(
                                        this,
                                        & parameters,
                                        this );
      US_Hydrodyn::fixWinButtons( mals_saxs_dctr );
      mals_saxs_dctr->exec();
      delete mals_saxs_dctr;

      if ( !parameters.count( "keep" ) )
      {
         update_enables();
         return;
      }

      ((US_Hydrodyn_Mals_Saxs *)mals_saxs_win)->detector_uv      = ( parameters.count( "uv" ) && parameters[ "uv" ] == "true" ) ? true : false;
      ((US_Hydrodyn_Mals_Saxs *)mals_saxs_win)->detector_ri      = ( parameters.count( "ri" ) && parameters[ "ri" ] == "true" ) ? true : false;
      ((US_Hydrodyn_Mals_Saxs *)mals_saxs_win)->detector_ri_conv = parameters[ "ri_conv" ].toDouble();
      ((US_Hydrodyn_Mals_Saxs *)mals_saxs_win)->detector_uv_conv = parameters[ "uv_conv" ].toDouble();
   }
}

void US_Hydrodyn_Mals_Saxs_Options::set_mals_saxs_parameters()
{
   {
      map < QString, QString > parameters;
      parameters[ "mals_saxs_param_lambda"            ] = QString( "%1" ).arg( ((US_Hydrodyn_Mals_Saxs *)mals_saxs_win)->mals_saxs_param_lambda, 0, 'g', 8 );
      parameters[ "mals_saxs_param_n"                 ] = QString( "%1" ).arg( ((US_Hydrodyn_Mals_Saxs *)mals_saxs_win)->mals_saxs_param_n, 0, 'g', 8 );
      parameters[ "mals_saxs_param_g_dndc"            ] = QString( "%1" ).arg( ((US_Hydrodyn_Mals_Saxs *)mals_saxs_win)->mals_saxs_param_g_dndc, 0, 'g', 8 );
      parameters[ "mals_saxs_param_g_extinction_coef" ] = QString( "%1" ).arg( ((US_Hydrodyn_Mals_Saxs *)mals_saxs_win)->mals_saxs_param_g_extinction_coef, 0, 'g', 8 );
      parameters[ "mals_saxs_param_g_conc"            ] = QString( "%1" ).arg( ((US_Hydrodyn_Mals_Saxs *)mals_saxs_win)->mals_saxs_param_g_conc, 0, 'g', 8 );
      parameters[ "mals_saxs_param_DLS_detector"      ] = QString( "%1" ).arg( ((US_Hydrodyn_Mals_Saxs *)mals_saxs_win)->mals_saxs_param_DLS_detector );


      US_Hydrodyn_Mals_Saxs_Parameters *mals_saxs_parameters = 
         new US_Hydrodyn_Mals_Saxs_Parameters(
                                         this,
                                         & parameters,
                                         this );
      US_Hydrodyn::fixWinButtons( mals_saxs_parameters );
      mals_saxs_parameters->exec();
      delete mals_saxs_parameters;

      if ( !parameters.count( "keep" ) )
      {
         update_enables();
         return;
      }

      ((US_Hydrodyn_Mals_Saxs *)mals_saxs_win)->mals_saxs_param_lambda            = parameters[ "mals_saxs_param_lambda"            ].toDouble();
      ((US_Hydrodyn_Mals_Saxs *)mals_saxs_win)->mals_saxs_param_n                 = parameters[ "mals_saxs_param_n"                 ].toDouble();
      ((US_Hydrodyn_Mals_Saxs *)mals_saxs_win)->mals_saxs_param_g_dndc            = parameters[ "mals_saxs_param_g_dndc"            ].toDouble();
      ((US_Hydrodyn_Mals_Saxs *)mals_saxs_win)->mals_saxs_param_g_extinction_coef = parameters[ "mals_saxs_param_g_extinction_coef" ].toDouble();
      ((US_Hydrodyn_Mals_Saxs *)mals_saxs_win)->mals_saxs_param_g_conc            = parameters[ "mals_saxs_param_g_conc"            ].toDouble();
      ((US_Hydrodyn_Mals_Saxs *)mals_saxs_win)->mals_saxs_param_DLS_detector      = parameters[ "mals_saxs_param_DLS_detector"      ].toInt();
   }
}

void US_Hydrodyn_Mals_Saxs_Options::fasta_file() {
   // load file

   QString use_dir = QDir::currentPath();
   ((US_Hydrodyn  *)us_hydrodyn)->select_from_directory_history( use_dir, this );
   raise();

   QString filename = QFileDialog::getOpenFileName(
                                                   this,
                                                   windowTitle() + us_tr( "Load FASTA sequence File" ),
                                                   use_dir,
                                                   "FASTA files ( *.fasta.txt *.fasta );;All files (*)"
                                                   );
   
   if ( filename.isEmpty() ) {
      return;
   }

   ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( filename );

   QFile f( filename );
   
   if ( !f.open( QIODevice::ReadOnly ) ) {
      QMessageBox::warning(
                           this, 
                           windowTitle()+ us_tr( ": Compute PSV from FASTA" ),
                           QString( us_tr( "Could not open file '%1' for reading. Possibly a permissions issue." ) ).arg( filename )
                           );
      return;
   }
      
   QStringList qsl;

   QTextStream ts( &f );
   QRegExp rx_empty( "^\\s*$" );
   QRegExp rx_newseq( "^\\s*>" );

   QString seq;
   int chains = 0;
   
   QString seq_names;

   // find >seq lines

   while ( !ts.atEnd() ) {
      QString qs = ts.readLine().trimmed();
      
      if ( rx_empty.exactMatch( qs ) ) {
         continue;
      }

      if ( rx_newseq.indexIn( qs ) > -1 ) {
         seq_names += qs.replace( rx_newseq, "" ) + "\n";
         chains++;
         continue;
      }
      
      seq += qs;
   }

   f.close();

   if ( seq.isEmpty() ) {
      QMessageBox::warning(
                           this, 
                           windowTitle()+ us_tr( ": Compute PSV from FASTA" ),
                           QString( us_tr( "File '%1' no sequence information found." ) ).arg( filename )
                           );
      return;
   }
      
   QStringList seq_chars = seq.split( "" );

#if defined( DEBUG_FASTA_SEQ )
   {
      QTextStream ts( stdout );
      ts << QString( "%1 sequences %2:\n%3" ).arg( chains ).arg( seq_names ).arg( seq );
      ts << seq_chars.join( "\n" );
   }
#endif
   
   ((US_Hydrodyn *)us_hydrodyn)->reset_ionized_residue_vectors();

   double psv;
   QString msgs;
   if ( !((US_Hydrodyn  *)us_hydrodyn)->calc_fasta_vbar( seq_chars, psv, msgs ) ) {
      QMessageBox::warning(
                           this, 
                           windowTitle()+ us_tr( ": Compute PSV from FASTA" ),
                           msgs );
      return;
   }

   QMessageBox::information(
                            this, 
                            windowTitle()+ us_tr( ": Compute PSV from FASTA" ),
                            QString( us_tr( "FASTA computed from the following sequence names:\n" ) )
                            + seq_names
                            );

   le_fasta_value->setText( QString( "" ).sprintf( "%.3f", psv ) );
   le_fasta_value->setEnabled( true );
   return;
}

void US_Hydrodyn_Mals_Saxs_Options::update_fasta_pH( const QString &str ) {
   if ( ((US_Hydrodyn *)us_hydrodyn)->hydro.pH == str.toDouble() ) {
      return;
   }
   le_fasta_value->setText("");
   ((US_Hydrodyn *)us_hydrodyn)->set_disabled();
   
   ((US_Hydrodyn *)us_hydrodyn)->hydro.pH = str.toDouble();
   ((US_Hydrodyn *)us_hydrodyn)->le_pH->setText( QString( "" ).sprintf( "%4.2f", ((US_Hydrodyn *)us_hydrodyn)->hydro.pH ) );
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}
