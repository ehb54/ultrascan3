#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_mals_ciq.h"
//Added by qt3to4:
#include <QHBoxLayout>
#include <QCloseEvent>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>

US_Hydrodyn_Mals_Ciq::US_Hydrodyn_Mals_Ciq(
                                                     void                     *              us_hydrodyn_mals,
                                                     map < QString, QString > *              parameters,
                                                     QWidget *                               p,
                                                     const char *                            
                                                     ) : QDialog( p )
{
   this->us_hydrodyn_mals                = us_hydrodyn_mals;
   this->parameters                           = parameters;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle( us_tr( "US-SOMO: MALS : Make I*(q)" ) );

   setupGUI();
   update_enables();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );
}

US_Hydrodyn_Mals_Ciq::~US_Hydrodyn_Mals_Ciq()
{
}

void US_Hydrodyn_Mals_Ciq::setupGUI()
{

   QPalette qcg_normal_redtext = USglobal->global_colors.cg_normal;
   qcg_normal_redtext.setColor( QPalette::WindowText, Qt::red );

   // qcg_normal_redtext.setColor( QPalette::Window, Qt::yellow );
   //   qcg_normal_redtext.setColor( QPalette::WindowText, Qt::dRed );
   // qcg_normal_redtext.setColor( QPalette::Base      , Qt::cyan );
   qcg_normal_redtext.setColor( QPalette::Text      , Qt::red );
   // qcg_normal_redtext.setColor( QPalette::Button    , Qt::red );
   // qcg_normal_redtext.setColor( QPalette::ButtonText, Qt::magenta );

   int minHeight1  = 30;
   int minHeight2  = 25;

   lbl_title =  new QLabel      ( parameters->count( "ngmode" ) ?  us_tr( "US-SOMO: MALS : Make I*(q) without Gaussians" ) :  us_tr( "US-SOMO: MALS : Make I*(q)" ), this );
   // lbl_title -> setFrameStyle   ( QFrame::WinPanel | QFrame::Raised );
   lbl_title -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_title -> setMinimumHeight( minHeight1 );
   lbl_title ->setPalette( PALET_LABEL );
   AUTFBACK( lbl_title );
   // lbl_title -> setPalette      ( PALET_FRAME );
   lbl_title -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold ) );
   // lbl_title -> hide();

   cb_add_bl = new QCheckBox(this);
   cb_add_bl->setText( parameters->count( "baseline" ) ? (*parameters)[ "baseline" ] : "" );
   cb_add_bl->setEnabled( true );
   connect( cb_add_bl, SIGNAL( clicked() ), SLOT( set_add_bl() ) );
   cb_add_bl->setChecked( false );
   cb_add_bl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_add_bl->setPalette( PALET_NORMAL );
   AUTFBACK( cb_add_bl );
   if ( !parameters->count( "expert_mode" ) || !parameters->count( "baseline" ) )
   {
      cb_add_bl->hide();
   }

   cb_save_as_pct_iq = new QCheckBox(this);
   cb_save_as_pct_iq->setText(us_tr( "Resulting I(q) created as a percent of the original I(q) ( if unchecked, I(q) will be created from the Gaussians )" ) );
   cb_save_as_pct_iq->setEnabled( true );
   cb_save_as_pct_iq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_save_as_pct_iq->setPalette( PALET_NORMAL );
   AUTFBACK( cb_save_as_pct_iq );
   if ( !parameters->count( "expert_mode" ) || parameters->count( "ngmode" ) )
   {
      cb_save_as_pct_iq->hide();
   }

   cb_save_sum = new QCheckBox(this);
   cb_save_sum->setText(us_tr( "Create sum of peaks curves" ) );
   cb_save_sum->setEnabled( true );
   cb_save_sum->setChecked( false );
   cb_save_sum->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_save_sum->setPalette( PALET_NORMAL );
   AUTFBACK( cb_save_sum );
   if ( parameters->count( "ngmode" ) ) {
      cb_save_sum->hide();
   }

   QLabel * lbl_save_sum_bi = (QLabel *)0;
   if ( parameters->count( "integralbaseline" ) ) {
      lbl_save_sum_bi = new QLabel(
                 us_tr( "     Warning: If you have changed the starting time/frame since integral baseline correction, creation of the\n"
                     "      sum of peaks curves with integral baseline will differ due to missing time/frame scattering intensity." ),
                 this );
      lbl_save_sum_bi->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
      lbl_save_sum_bi->setPalette( PALET_NORMAL );
      AUTFBACK( lbl_save_sum_bi );
      lbl_save_sum_bi->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ));
   }
                                              
   cb_sd_source = new QCheckBox(this);
   cb_sd_source->setText( us_tr(
                              // "Compute standard deviations as a difference between the sum of Gaussians and original I(q)"
                              "Add SD computed %-wise from the difference between the sum of Gaussians and the original I(q)"
                              ) );
   cb_sd_source->setEnabled( true );
   cb_sd_source->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_sd_source->setPalette( PALET_NORMAL );
   AUTFBACK( cb_sd_source );
   if ( parameters->count( "ngmode" ) ) {
      cb_sd_source->hide();
   }


   cb_makeiq_avg_peaks = new QCheckBox(this);
   cb_makeiq_avg_peaks->setText( us_tr( "Average and normalize resulting I(q) curves by Gaussian, using top % of max. intensity" ) );
   cb_makeiq_avg_peaks->setEnabled( true );
   cb_makeiq_avg_peaks->setChecked( (*parameters)[ "mals_cb_makeiq_avg_peaks" ] == "true" );
   cb_makeiq_avg_peaks->setChecked( false );
   cb_makeiq_avg_peaks->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_makeiq_avg_peaks->setPalette( PALET_NORMAL );
   AUTFBACK( cb_makeiq_avg_peaks );
   connect( cb_makeiq_avg_peaks, SIGNAL( clicked() ), SLOT( update_enables() ) );
   if ( parameters->count( "ngmode" ) ) {
      cb_makeiq_avg_peaks->hide();
   }

   le_makeiq_avg_peaks = new QLineEdit( this );    le_makeiq_avg_peaks->setObjectName( "le_makeiq_avg_peaks Line Edit" );
   le_makeiq_avg_peaks->setText( (*parameters)[ "mals_makeiq_avg_peaks" ] );
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
   if ( parameters->count( "ngmode" ) ) {
      le_makeiq_avg_peaks->hide();
   }

   // cb's co dependent
   connect( cb_save_as_pct_iq, SIGNAL( clicked() ), SLOT( set_save_as_pct_iq() ) );
   connect( cb_sd_source, SIGNAL( clicked() ), SLOT( set_sd_source() ) );
   cb_sd_source->setChecked( false );
   cb_save_as_pct_iq->setChecked( true );

   QLabel * lbl_sd_zeros_found = new QLabel( us_tr( "If zeros are produced when computing SDs:  " ), this );
   lbl_sd_zeros_found->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_sd_zeros_found->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_sd_zeros_found );
   lbl_sd_zeros_found->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ));
   if ( parameters->count( "ngmode" ) ) {
      lbl_sd_zeros_found->hide();
   }

   cb_sd_zero_avg_local_sd = new QCheckBox(this);
   cb_sd_zero_avg_local_sd->setText( us_tr( "Average adjacent SDs  " ) );
   cb_sd_zero_avg_local_sd->setEnabled( true );
   connect( cb_sd_zero_avg_local_sd, SIGNAL( clicked() ), SLOT( set_sd_zero_avg_local_sd() ) );
   cb_sd_zero_avg_local_sd->setChecked( true );
   cb_sd_zero_avg_local_sd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_sd_zero_avg_local_sd->setPalette( PALET_NORMAL );
   AUTFBACK( cb_sd_zero_avg_local_sd );
   if ( parameters->count( "ngmode" ) ) {
      cb_sd_zero_avg_local_sd->hide();
   }

   cb_sd_zero_keep_as_zeros = new QCheckBox(this);
   cb_sd_zero_keep_as_zeros->setText( us_tr( "Leave as zero  " ) );
   cb_sd_zero_keep_as_zeros->setEnabled( true );
   connect( cb_sd_zero_keep_as_zeros, SIGNAL( clicked() ), SLOT( set_sd_zero_keep_as_zeros() ) );
   cb_sd_zero_keep_as_zeros->setChecked( false );
   cb_sd_zero_keep_as_zeros->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_sd_zero_keep_as_zeros->setPalette( PALET_NORMAL );
   AUTFBACK( cb_sd_zero_keep_as_zeros );
   cb_sd_zero_keep_as_zeros->hide();
   if ( parameters->count( "ngmode" ) ) {
      cb_sd_zero_keep_as_zeros->hide();
   }

   cb_sd_zero_set_to_pt1pct = new QCheckBox(this);
   cb_sd_zero_set_to_pt1pct->setText( us_tr( "Set to 0.1 % of peak's I(q)  " ) );
   cb_sd_zero_set_to_pt1pct->setEnabled( true );
   connect( cb_sd_zero_set_to_pt1pct, SIGNAL( clicked() ), SLOT( set_sd_zero_set_to_pt1pct() ) );
   cb_sd_zero_set_to_pt1pct->setChecked( false );
   cb_sd_zero_set_to_pt1pct->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_sd_zero_set_to_pt1pct->setPalette( PALET_NORMAL );
   AUTFBACK( cb_sd_zero_set_to_pt1pct );
   if ( parameters->count( "ngmode" ) ) {
      cb_sd_zero_set_to_pt1pct->hide();
   }

   ws_sd_zeros.push_back( lbl_sd_zeros_found );
   ws_sd_zeros.push_back( cb_sd_zero_avg_local_sd );
   // ws_sd_zeros.push_back( cb_sd_zero_keep_as_zeros );
   ws_sd_zeros.push_back( cb_sd_zero_set_to_pt1pct );

   QLabel * lbl_zeros_found = new mQLabel( us_tr( "I(t) is missing SDs at some points. When computing I(q) SDs : " ), this );
   lbl_zeros_found->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_zeros_found->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_zeros_found );
   lbl_zeros_found->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ));
   connect( lbl_zeros_found, SIGNAL(pressed()), SLOT( zeros_found() ));
   if ( parameters->count( "ngmode" ) ) {
      lbl_zeros_found->hide();
   }

   cb_zero_drop_points = new QCheckBox(this);
   cb_zero_drop_points->setText( us_tr( "Drop points  " ) );
   cb_zero_drop_points->setEnabled( true );
   connect( cb_zero_drop_points, SIGNAL( clicked() ), SLOT( set_zero_drop_points() ) );
   cb_zero_drop_points->setChecked( false );
   cb_zero_drop_points->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_zero_drop_points->setPalette( PALET_NORMAL );
   AUTFBACK( cb_zero_drop_points );
   if ( parameters->count( "ngmode" ) ) {
      cb_zero_drop_points->hide();
   }

   cb_zero_avg_local_sd = new QCheckBox(this);
   cb_zero_avg_local_sd->setText( us_tr( "Average adjacent SDs  " ) );
   cb_zero_avg_local_sd->setEnabled( true );
   connect( cb_zero_avg_local_sd, SIGNAL( clicked() ), SLOT( set_zero_avg_local_sd() ) );
   cb_zero_avg_local_sd->setChecked( true );
   cb_zero_avg_local_sd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_zero_avg_local_sd->setPalette( PALET_NORMAL );
   AUTFBACK( cb_zero_avg_local_sd );
   if ( parameters->count( "ngmode" ) ) {
      cb_zero_avg_local_sd->hide();
   }

   cb_zero_keep_as_zeros = new QCheckBox(this);
   cb_zero_keep_as_zeros->setText( us_tr( "Leave as zeros " ) );
   cb_zero_keep_as_zeros->setEnabled( true );
   connect( cb_zero_keep_as_zeros, SIGNAL( clicked() ), SLOT( set_zero_keep_as_zeros() ) );
   cb_zero_keep_as_zeros->setChecked( false );
   cb_zero_keep_as_zeros->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_zero_keep_as_zeros->setPalette( PALET_NORMAL );
   AUTFBACK( cb_zero_keep_as_zeros );
   cb_zero_keep_as_zeros->hide();
   if ( parameters->count( "ngmode" ) ) {
      cb_zero_keep_as_zeros->hide();
   }

   ws_zeros.push_back( lbl_zeros_found );
   ws_zeros.push_back( cb_zero_drop_points );
   ws_zeros.push_back( cb_zero_avg_local_sd );
   //   ws_zeros.push_back( cb_zero_keep_as_zeros );

   cb_normalize = new QCheckBox(this);
   cb_normalize->setText( us_tr( "Normalize resulting I(q) by concentration" ) );
   cb_normalize->setEnabled( true );
   connect( cb_normalize, SIGNAL( clicked() ), SLOT( set_normalize() ) );
   cb_normalize->setChecked( false );
   cb_normalize->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_normalize->setPalette( PALET_NORMAL );
   AUTFBACK( cb_normalize );
   if ( parameters->count( "ngmode" ) ) {
      cb_normalize->hide();
   }

   cb_I0se = new QCheckBox(this);
   cb_I0se->setText( us_tr( "I0 standard experimental value (a.u.) : " ) );
   cb_I0se->setEnabled( true );
   connect( cb_I0se, SIGNAL( clicked() ), SLOT( set_I0se() ) );
   cb_I0se->setChecked( true );
   cb_I0se->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_I0se->setPalette( PALET_NORMAL );
   AUTFBACK( cb_I0se );
   cb_I0se-> setMinimumHeight( minHeight2 );
   cb_I0se->setToolTip( us_tr( "<html><head/><body><p>Note: This value will be stored in the produced files and used in SOMO-SAS MW computations.</p><p>If your data is already properly normalized, you should leave this checked with a value of 1.</p></body></html>" ) );
   cb_I0se->hide();

   le_I0se = new QLineEdit( this );    le_I0se->setObjectName( "le_I0se Line Edit" );
   le_I0se->setText( "1" );
   le_I0se->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_I0se->setPalette( PALET_NORMAL );
   AUTFBACK( le_I0se );
   le_I0se->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( le_I0se );
      qdv->setDecimals( 8 );
      le_I0se->setValidator( qdv );
   }
   connect( le_I0se, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   le_I0se->setMinimumWidth( 60 );
   le_I0se->setMinimumHeight( minHeight2 );
   le_I0se->setToolTip( us_tr( "<html><head/><body><p>Note: This value will be stored in the produced files and used in SOMO-SAS MW computations.</p><p>If your data is already properly normalized, you should leave this checked with a value of 1.</p></body></html>" ) );
   le_I0se->hide();
   
   lbl_error = new QLabel( parameters->count( "error" ) ? (*parameters)[ "error" ] : "", this );
   lbl_error->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_error->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_error );
   lbl_error->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold ));

   cb_conc_to_saxs = new QCheckBox(this);
   cb_conc_to_saxs->setText( us_tr( 
                                "Do you want to set the concentration file Gaussians centers, widths and skewness to the SAXS-optimized values,\nadjusting the amplitudes and keeping the areas constant?\n"
                                 ) );
   cb_conc_to_saxs->setEnabled( true );
   connect( cb_conc_to_saxs, SIGNAL( clicked() ), SLOT( update_enables() ) );
   cb_conc_to_saxs->setChecked( true );
   cb_conc_to_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_conc_to_saxs->setPalette( PALET_NORMAL );
   AUTFBACK( cb_conc_to_saxs );
   if ( parameters->count( "ngmode" ) ) {
      cb_conc_to_saxs->hide();
   }

   lbl_conc_to_saxs_info1 = new QLabel( us_tr(
                                           "       This implies that all the species that were defined as Gaussians contributing to the SAXS signal also contribute to the concentration signal."
                                           ), this );
   lbl_conc_to_saxs_info1->setAlignment(Qt::AlignJustify|Qt::AlignVCenter);
   lbl_conc_to_saxs_info1->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_conc_to_saxs_info1 );
   lbl_conc_to_saxs_info1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));

   lbl_conc_to_saxs_info2 = new QLabel( us_tr( 
                                           "       Be aware that this option will result in an apparent mass artificially approximately constant along each of the deconvoluted Gaussian peaks,\n"
                                           "       reflecting just the oscillations in the original SAXS data." 
                                            ), this );
   lbl_conc_to_saxs_info2->setAlignment(Qt::AlignJustify|Qt::AlignVCenter);
   lbl_conc_to_saxs_info2->setPalette( qcg_normal_redtext );
   lbl_conc_to_saxs_info2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );

   lbl_conc_to_saxs_info3 = new QLabel( us_tr(
                                           "       However, the apparent average mass for each peak should be a closer approximation to the real value when significant band broadening occurs\n"
                                           "       between the concentration and the SAXS detectors."
                                           ), this );
   lbl_conc_to_saxs_info3->setAlignment(Qt::AlignJustify|Qt::AlignVCenter);
   lbl_conc_to_saxs_info3->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_conc_to_saxs_info3 );
   lbl_conc_to_saxs_info3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   if ( parameters->count( "ngmode" ) ) {
      lbl_conc_to_saxs_info1->hide();
      lbl_conc_to_saxs_info2->hide();
      lbl_conc_to_saxs_info3->hide();
   }

   lbl_conc = new QLabel( us_tr( "Concentrations will be computed and will be written along with dn/dc values to the output I(q) curves" ), this );
   lbl_conc->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_conc->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_conc );
   lbl_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold ));

   pb_global =  new QPushButton ( us_tr( "Duplicate Gaussian 1 values globally" ), this );
   pb_global -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   pb_global -> setMinimumHeight( minHeight1 );
   pb_global -> setPalette      ( PALET_PUSHB );
   connect( pb_global, SIGNAL( clicked() ), SLOT( global() ) );
   if ( parameters->count( "ngmode" ) ) {
      pb_global->hide();
   }

   lbl_gaussian = new QLabel( us_tr( "Gaussian" ), this );
   lbl_gaussian->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_gaussian->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_gaussian );
   lbl_gaussian->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   if ( parameters->count( "ngmode" ) ) {
      lbl_gaussian->hide();
   }

   lbl_conv = new QLabel( us_tr( (*parameters).count( "uv" ) ? "Extinction coefficient (ml mg^-1 cm^-1)" : "Differential RI increment [dn/dc] (ml/g)" ), this );
   lbl_conv->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_conv->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_conv );
   lbl_conv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));

   lbl_dndc = new QLabel( us_tr( "dn/dc (ml/g)" ), this );
   lbl_dndc->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_dndc->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_dndc );
   lbl_dndc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));

   for ( unsigned int i = 0; i < (* parameters)[ "gaussians" ].toUInt(); i++ )
   {
      QLabel * lbl_tmp = new QLabel( QString( "%1" ).arg( i + 1 ), this );
      lbl_tmp->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
      lbl_tmp->setPalette( PALET_NORMAL );
      AUTFBACK( lbl_tmp );
      lbl_tmp->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
      lbl_gaussian_id.push_back( lbl_tmp );
      if ( parameters->count( "ngmode" ) ) {
         lbl_tmp->hide();
      }

      QLineEdit * le_tmp = new QLineEdit( this );  le_tmp->setObjectName( "le_tmp Line Edit" );
      le_tmp->setText( parameters->count( "default_extc" ) ? (*parameters)[ "default_extc" ] : "" );
      le_tmp->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
      le_tmp->setPalette( PALET_NORMAL );
      AUTFBACK( le_tmp );
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

      le_tmp = new QLineEdit( this );       le_tmp->setObjectName( "le_tmp Line Edit" );
      le_tmp->setText( parameters->count( "default_dndc" ) ? (*parameters)[ "default_dndc" ] : "" );
      le_tmp->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
      le_tmp->setPalette( PALET_NORMAL );
      AUTFBACK( le_tmp );
      le_tmp->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
      {
         QDoubleValidator *qdv = new QDoubleValidator( le_tmp );
         qdv->setDecimals( 8 );
         le_tmp->setValidator( qdv );
      }
      connect( le_tmp, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
      le_tmp->setMinimumWidth( 200 );
      le_tmp-> setMinimumHeight( minHeight1 );
      le_dndc.push_back( le_tmp );
   }

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

   pb_create_ng =  new QPushButton ( us_tr( "Make I*(q) without Gaussians" ), this );
   pb_create_ng -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_create_ng -> setMinimumHeight( minHeight1 );
   pb_create_ng -> setPalette      ( PALET_PUSHB );
   connect( pb_create_ng, SIGNAL( clicked() ), SLOT( create_ng() ) );
   if ( parameters->count( "ngmode" ) ) {
      pb_create_ng->hide();
   }

   pb_go =  new QPushButton ( us_tr( "Continue" ), this );
   pb_go -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_go -> setMinimumHeight( minHeight1 );
   pb_go -> setPalette      ( PALET_PUSHB );
   connect( pb_go, SIGNAL( clicked() ), SLOT( go() ) );

   QVBoxLayout * background = new QVBoxLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );
   // background->addSpacing(4);

   background->addWidget( lbl_title );
   // background->addSpacing( 4 );

   QVBoxLayout * vbl = new QVBoxLayout( 0 ); vbl->setContentsMargins( 0, 0, 0, 0 ); vbl->setSpacing( 0 );
   vbl->addWidget( cb_add_bl );
   vbl->addWidget( cb_save_as_pct_iq );
   vbl->addWidget( cb_save_sum );
   if ( lbl_save_sum_bi ) {
      vbl->addWidget( lbl_save_sum_bi );
   }

   vbl->addWidget( cb_sd_source );

   QHBoxLayout * hbl_sd_zeros = new QHBoxLayout(); hbl_sd_zeros->setContentsMargins( 0, 0, 0, 0 ); hbl_sd_zeros->setSpacing( 0 );
   for ( unsigned int i = 0; i < ( unsigned int )ws_sd_zeros.size(); i++ )
   {
      hbl_sd_zeros->addWidget( ws_sd_zeros[ i ] );
   }
   vbl->addLayout( hbl_sd_zeros );

   QHBoxLayout * hbl_zeros = new QHBoxLayout(); hbl_zeros->setContentsMargins( 0, 0, 0, 0 ); hbl_zeros->setSpacing( 0 );
   for ( unsigned int i = 0; i < ( unsigned int )ws_zeros.size(); i++ )
   {
      hbl_zeros->addWidget( ws_zeros[ i ] );
   }
   vbl->addLayout( hbl_zeros );

   {
      QHBoxLayout * hbl_avg_peaks = new QHBoxLayout(); hbl_avg_peaks->setContentsMargins( 0, 0, 0, 0 ); hbl_avg_peaks->setSpacing( 0 );
      hbl_avg_peaks->addWidget( cb_makeiq_avg_peaks );
      hbl_avg_peaks->addWidget( le_makeiq_avg_peaks );
      vbl->addLayout( hbl_avg_peaks );
   }

   vbl->addWidget( cb_normalize );


   QGridLayout * gl = new QGridLayout( 0 ); gl->setContentsMargins( 0, 0, 0, 0 ); gl->setSpacing( 0 );

   int j = 0;
   gl->addWidget( lbl_error , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 2  ) - ( 0 ) );
   j++;

   gl->addWidget( cb_conc_to_saxs , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 2  ) - ( 0 ) );
   j++;
   gl->addWidget( lbl_conc_to_saxs_info1 , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 2  ) - ( 0 ) );
   j++;
   gl->addWidget( lbl_conc_to_saxs_info2 , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 2  ) - ( 0 ) );
   j++;
   gl->addWidget( lbl_conc_to_saxs_info3 , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 2  ) - ( 0 ) );
   j++;
   
   QHBoxLayout * hbl_I0se = new QHBoxLayout(); hbl_I0se->setContentsMargins( 0, 0, 0, 0 ); hbl_I0se->setSpacing( 0 );
   hbl_I0se->addWidget( cb_I0se );
   hbl_I0se->addWidget( le_I0se );
   gl->addLayout( hbl_I0se , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 2  ) - ( 0 ) );
   j++;

   gl->addWidget( lbl_conc , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 2  ) - ( 0 ) );
   j++;

   gl->addWidget( lbl_gaussian, j, 0 );
   gl->addWidget( lbl_conv    , j, 1 );
   gl->addWidget( lbl_dndc     , j, 2 );
   j++;

   for ( unsigned int i = 0; i < ( unsigned int ) lbl_gaussian_id.size(); i++ )
   {
      gl->addWidget( lbl_gaussian_id[ i ], j, 0 );
      gl->addWidget( le_conv        [ i ], j, 1 );
      gl->addWidget( le_dndc         [ i ], j, 2 );
      j++;
   }

   if ( parameters->count( "error" ) )
   {
      cb_conc_to_saxs        ->hide();
      lbl_conc_to_saxs_info1 ->hide();
      lbl_conc_to_saxs_info2 ->hide();
      lbl_conc_to_saxs_info3 ->hide();

      lbl_conc               ->hide();
      lbl_gaussian           ->hide();
      lbl_conv               ->hide();
      lbl_dndc                ->hide();
      for ( unsigned int i = 0; i < ( unsigned int ) lbl_gaussian_id.size(); i++ )
      {
         lbl_gaussian_id[ i ]->hide();
         le_conv        [ i ]->hide();
         le_dndc         [ i ]->hide();
      }
      pb_global   ->hide();

      cb_I0se->hide();
      le_I0se->hide();

      cb_makeiq_avg_peaks->setText( us_tr( "Average resulting I(q) curves by Gaussian, using top % of max. intensity" ) );

      // cb_normalize->hide();
   } else {
      lbl_error    ->hide();
      if ( lbl_gaussian_id.size() <= 1 )
      {
         pb_global    ->hide();
      }
   }
   cb_normalize->hide();

   if ( parameters->count( "ngmode" ) ) {
      // any other settings we need?
      cb_save_as_pct_iq->setChecked( false );
   }

   QHBoxLayout * hbl_bottom = new QHBoxLayout(); hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addWidget ( pb_quit );
   hbl_bottom->addWidget ( pb_create_ng );
   hbl_bottom->addWidget ( pb_go );

   background->addLayout ( vbl );
   background->addLayout ( gl );
   background->addWidget ( pb_global);
   background->addLayout ( hbl_bottom );
   // background->addSpacing( 4 );
   set_save_as_pct_iq();
}

void US_Hydrodyn_Mals_Ciq::quit()
{
   close();
}

void US_Hydrodyn_Mals_Ciq::create_ng()
{
   (*parameters)[ "go" ]      = "true";
   (*parameters)[ "make_ng" ] = "true";
   close();
}

void US_Hydrodyn_Mals_Ciq::go()
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

   (*parameters)[ "conc_to_saxs" ] = cb_conc_to_saxs->isChecked() ? "true" : "false";

   (*parameters)[ "mals_cb_makeiq_avg_peaks"     ] = cb_makeiq_avg_peaks    ->isChecked() ? "true" : "false";
   (*parameters)[ "mals_makeiq_avg_peaks"        ] = le_makeiq_avg_peaks    ->text();

   for ( unsigned int i = 0; i < ( unsigned int ) lbl_gaussian_id.size(); i++ )
   {
      (*parameters)[ QString( "conv %1" ).arg( i ) ] = le_conv[ i ]->text();
      (*parameters)[ QString( "dndc %1" ) .arg( i ) ] = le_dndc [ i ]->text();
   }

   if ( cb_I0se->isChecked() )
   {
      (*parameters)[ "I0se" ] = le_I0se->text();
   }
   close();
}

void US_Hydrodyn_Mals_Ciq::help()
{
   US_Help *online_help;
   online_help = new US_Help( this );
   online_help->show_help("manual/somo/mals_ciq.html");
}

void US_Hydrodyn_Mals_Ciq::closeEvent( QCloseEvent *e )
{

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Mals_Ciq::set_add_bl()
{
   (*parameters)[ "add_baseline" ] = cb_add_bl->isChecked() ? "true" : "false";
}

void US_Hydrodyn_Mals_Ciq::set_sd_source()
{
   update_enables();
}

void US_Hydrodyn_Mals_Ciq::set_normalize()
{
   update_enables();
}

void US_Hydrodyn_Mals_Ciq::set_I0se()
{
   update_enables();
}

void US_Hydrodyn_Mals_Ciq::set_save_as_pct_iq()
{
   (*parameters)[ "save_as_pct_iq" ] = cb_save_as_pct_iq->isChecked() ? "true" : "false";
   if ( cb_save_as_pct_iq->isChecked() )
   {
      cb_sd_source->show();
   } else {
      cb_sd_source->setChecked( false );
      cb_sd_source->hide();
   }
   update_enables();
}

void US_Hydrodyn_Mals_Ciq::global()
{
   for ( unsigned int i = 1; i < ( unsigned int ) lbl_gaussian_id.size(); i++ )
   {
      le_conv[ i ]->setText( le_conv[ 0 ]->text() );
      le_dndc [ i ]->setText( le_dndc [ 0 ]->text() );
   }
   update_enables();
}

void US_Hydrodyn_Mals_Ciq::set_sd_zero_avg_local_sd()
{
   (*parameters)[ "sd_zero_avg_local_sd" ] = cb_sd_zero_avg_local_sd->isChecked() ? "true" : "false";
   if ( cb_sd_zero_avg_local_sd->isChecked() )
   {
      cb_sd_zero_keep_as_zeros->setChecked( false );
      cb_sd_zero_set_to_pt1pct->setChecked( false );
   }
}

void US_Hydrodyn_Mals_Ciq::set_sd_zero_keep_as_zeros()
{
   (*parameters)[ "sd_zero_keep_as_zeros" ] = cb_sd_zero_keep_as_zeros->isChecked() ? "true" : "false";
   if ( cb_sd_zero_keep_as_zeros->isChecked() )
   {
      cb_sd_zero_avg_local_sd ->setChecked( false );
      cb_sd_zero_set_to_pt1pct->setChecked( false );
   }
}

void US_Hydrodyn_Mals_Ciq::set_sd_zero_set_to_pt1pct()
{
   (*parameters)[ "sd_zero_set_to_pt1pct" ] = cb_sd_zero_set_to_pt1pct->isChecked() ? "true" : "false";
   if ( cb_sd_zero_set_to_pt1pct->isChecked() )
   {
      cb_sd_zero_avg_local_sd ->setChecked( false );
      cb_sd_zero_keep_as_zeros->setChecked( false );
   }
}

void US_Hydrodyn_Mals_Ciq::set_zero_drop_points()
{
   (*parameters)[ "zero_drop_points" ] = cb_zero_drop_points->isChecked() ? "true" : "false";
   if ( cb_zero_drop_points->isChecked() )
   {
      cb_zero_avg_local_sd ->setChecked( false );
      cb_zero_keep_as_zeros->setChecked( false );
   }
}

void US_Hydrodyn_Mals_Ciq::set_zero_avg_local_sd()
{
   (*parameters)[ "zero_avg_local_sd" ] = cb_zero_avg_local_sd->isChecked() ? "true" : "false";
   if ( cb_zero_avg_local_sd->isChecked() )
   {
      cb_zero_drop_points  ->setChecked( false );
      cb_zero_keep_as_zeros->setChecked( false );
   }
}

void US_Hydrodyn_Mals_Ciq::set_zero_keep_as_zeros()
{
   (*parameters)[ "zero_keep_as_zeros" ] = cb_zero_keep_as_zeros->isChecked() ? "true" : "false";
   if ( cb_zero_keep_as_zeros->isChecked() )
   {
      cb_zero_drop_points  ->setChecked( false );
      cb_zero_avg_local_sd ->setChecked( false );
   }
}


void US_Hydrodyn_Mals_Ciq::update_enables()
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
         if ( !le_dndc [ i ]->text().toDouble() )
         {
            no_go = true;
            break;
         }
      }
   }

   le_makeiq_avg_peaks     ->setEnabled( cb_makeiq_avg_peaks->isChecked() );

   le_I0se->setEnabled( cb_I0se->isChecked() );

   pb_go->setEnabled( !no_go );
}

void US_Hydrodyn_Mals_Ciq::ws_hide( vector < QWidget * > ws, bool hide )
{
   for ( unsigned int i = 0; i < ( unsigned int )ws.size(); i++ )
   {
      hide ? ws[ i ]->hide() : ws[ i ]->show();
   }
}

void US_Hydrodyn_Mals_Ciq::zeros_found()
{
   QMessageBox::information( this,
                             windowTitle() + us_tr( ": SD Zeros" ),
                             QString( us_tr( "Please note:\n\n"
                                          "%1"
                                          "%2"
                                          "\n" ) )
                             .arg( (*parameters)[ "no_errors" ].isEmpty() ?
                                   "" : QString( us_tr( "These files have no associated errors:\n%1\n\n" ) ).arg( (*parameters)[ "no_errors" ] ) )
                             .arg( (*parameters)[ "zero_points" ].isEmpty() ?
                                   "" : QString( us_tr( "These files have points with missing SDs:\n%1\n\n" ) ).arg( (*parameters)[ "zero_points" ] ) ),
                             QMessageBox::Ok
                             );
}                             
