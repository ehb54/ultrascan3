#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_mals_saxs.h"
#include "../include/us_hydrodyn_mals_saxs_fit.h"
#include "../include/us_lm.h"
//Added by qt3to4:
#include <QHBoxLayout>
#include <QLabel>
#include <QGridLayout>
#include <QFrame>
#include <QVBoxLayout>
#include <QBoxLayout>
#include <QCloseEvent>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

// #define UHHSF_DEBUG_PLOT
// #define UHHSF_FULL_DEBUG
// #define UHHSF_PARAMS_DEBUG
// #defie

US_Hydrodyn_Mals_Saxs_Fit::US_Hydrodyn_Mals_Saxs_Fit(
                                                     US_Hydrodyn_Mals_Saxs *mals_saxs_win,
                                                     bool set_comm_dist,
                                                     QWidget *p, 
                                                     const char *
                                                     ) : QDialog( p )
{
   // us_qdebug( "hf0" );
   this->mals_saxs_win      = mals_saxs_win;
   this->set_comm_dist = set_comm_dist;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle( us_tr( "US-SOMO: MALS_SAXS: Gaussian Fit" ) );

   update_mals_saxs = true;
   running = false;
   gaussian_type      = mals_saxs_win->gaussian_type;
   gaussian_type_size = mals_saxs_win->gaussian_type_size;

   mals_saxs_ampl_width_min       = ( ( US_Hydrodyn * ) ( mals_saxs_win->us_hydrodyn ) )->gparams[ "mals_saxs_ampl_width_min"        ].toDouble();
   mals_saxs_lock_min_retry       = ( ( US_Hydrodyn * ) ( mals_saxs_win->us_hydrodyn ) )->gparams[ "mals_saxs_lock_min_retry"        ] == "true" ? true : false;
   mals_saxs_lock_min_retry_mult  = ( ( US_Hydrodyn * ) ( mals_saxs_win->us_hydrodyn ) )->gparams[ "mals_saxs_lock_min_retry_mult"   ].toDouble();
   mals_saxs_maxfpk_restart       = ( ( US_Hydrodyn * ) ( mals_saxs_win->us_hydrodyn ) )->gparams[ "mals_saxs_maxfpk_restart"        ] == "true" ? true : false;
   mals_saxs_maxfpk_restart_tries = ( ( US_Hydrodyn * ) ( mals_saxs_win->us_hydrodyn ) )->gparams[ "mals_saxs_maxfpk_restart_tries"  ].toUInt();
   mals_saxs_maxfpk_restart_pct   = ( ( US_Hydrodyn * ) ( mals_saxs_win->us_hydrodyn ) )->gparams[ "mals_saxs_maxfpk_restart_pct"    ].toDouble();

   switch ( gaussian_type )
   {
   case US_Hydrodyn_Mals_Saxs::EMGGMG :
      dist1_active = true;
      dist2_active = true;
      break;
   case US_Hydrodyn_Mals_Saxs::EMG :
   case US_Hydrodyn_Mals_Saxs::GMG :
      dist1_active = true;
      dist2_active = false;
      break;
   case US_Hydrodyn_Mals_Saxs::GAUSS :
      dist1_active = false;
      dist2_active = false;
      break;
   }

   // us_qdebug( "hf1" );
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;

   // us_qdebug( "hf2" );
   gaussians_undo.clear( );
   gaussians_undo.push_back( mals_saxs_win->gaussians );

   redo_settings();

   setGeometry(global_Xpos, global_Ypos, 0, 0 );

   // us_qdebug( "hf3" );
   set_comm_dist ? update_common() : update_enables();
   // us_qdebug( "hf4" );
   // update_enables();

   if ( mals_saxs_win->conc_files.count( mals_saxs_win->wheel_file ) )
   {
      cb_pct_center          ->setChecked( dist1_active );
      cb_pct_center_from_init->setChecked( true );
      le_pct_center          ->setText   ( "2"  );
      if ( gaussian_type != US_Hydrodyn_Mals_Saxs::EMGGMG )
      {
         cb_conc_test->hide();
      } else {
         for ( int i = gaussian_type_size; i < (int) mals_saxs_win->gaussians.size(); i += gaussian_type_size )
         {
            conc_ratios.push_back( mals_saxs_win->gaussians[ 2 + i ] / mals_saxs_win->gaussians[ 2 ] );
         }
         // us_qdebug( US_Vector::qs_vector( "gaussians", mals_saxs_win->gaussians ) );
         // us_qdebug( US_Vector::qs_vector( "width ratios", conc_ratios ) );
      }
   } else {
      cb_conc_test->hide();
   }
}

US_Hydrodyn_Mals_Saxs_Fit::~US_Hydrodyn_Mals_Saxs_Fit()
{
}

void US_Hydrodyn_Mals_Saxs_Fit::redo_settings()
{
   use_errors = mals_saxs_win->cb_sd_weight->isChecked();
   if ( use_errors &&
        ( !mals_saxs_win->f_errors.count( mals_saxs_win->wheel_file ) ||
          mals_saxs_win->f_errors[ mals_saxs_win->wheel_file ].size() != mals_saxs_win->f_qs[ mals_saxs_win->wheel_file ].size() ||
          !mals_saxs_win->is_nonzero_vector ( mals_saxs_win->f_errors[ mals_saxs_win->wheel_file ] ) ) )
   {
      use_errors = false;

      //       QMessageBox::information( this, this->windowTitle(),
      //                                 us_tr( "SD weighting requested, but the errors associated\n"
      //                                     "with the selected file are not all non-zero.\n" 
      //                                     "SD weighting turned off"
      //                                     ) );

      mals_saxs_win->editor_msg( "dark red",  QString( us_tr( "SD weighting requested, but the errors associated "
                                                      "with the selected file %1 are not all non-zero. " 
                                                      "SD weighting turned off for this file." ) ).arg( mals_saxs_win->wheel_file ) );
   }
   if ( use_errors )
   {
      // check t vector for all non-negative integers
      vector < double > x = mals_saxs_win->f_qs[ mals_saxs_win->wheel_file ];
      for ( unsigned i = 0; i < x.size(); i++ )
      {
         unsigned int xui = ( unsigned int ) fabs( x[ i ] );
         if ( ( double ) xui != x[ i ] )
         {
            use_errors = false;

            //             QMessageBox::information( this, this->windowTitle(),
            //                                       us_tr( "SD weighting requested, but this currently\n"
            //                                           "does not support fractional or negative frame numbers.\n"
            //                                           "SD weighting turned off."
            //                                           ) );

            mals_saxs_win->editor_msg( "dark red",  
                                  us_tr( "SD weighting requested, but this currently\n"
                                      "does not support fractional or negative frame numbers.\n"
                                      "SD weighting turned off." ) );
            break;
         }
      }
   }
   for ( int i = 0; i < (int) cb_fix_curves.size(); ++i ) {
      cb_fix_curves[ i ]->setChecked( false );
   }
}

void US_Hydrodyn_Mals_Saxs_Fit::restore()
{
   gaussians_undo.resize( 1 );
   mals_saxs_win->gaussians = gaussians_undo[ 0 ];
   if ( update_mals_saxs )
   {
      mals_saxs_win->gauss_init_markers();
      mals_saxs_win->gauss_init_gaussians();
      mals_saxs_win->update_gauss_pos();
   }
   update_enables();
}

void US_Hydrodyn_Mals_Saxs_Fit::undo()
{
   if ( gaussians_undo.size() > 1 )
   {
      gaussians_undo.pop_back();
   }

   mals_saxs_win->gaussians = gaussians_undo.back();
   if ( update_mals_saxs )
   {
      mals_saxs_win->gauss_init_markers();
      mals_saxs_win->gauss_init_gaussians();
      mals_saxs_win->update_gauss_pos();
   }
   update_enables();
}

void US_Hydrodyn_Mals_Saxs_Fit::setupGUI()
{
   int minHeight1 = 30;

   lbl_title = new QLabel( us_tr( "US-SOMO: MALS_SAXS: Gaussian Fit" ), this);
   lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette( PALET_FRAME );
   AUTFBACK( lbl_title );
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize , QFont::Bold));

   cb_fix_center = new QCheckBox(this);
   cb_fix_center->setText(us_tr(" Fix Gaussian centers" ) );
   cb_fix_center->setEnabled(true);
   cb_fix_center->setChecked( false );
   cb_fix_center->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_fix_center->setPalette( PALET_NORMAL );
   AUTFBACK( cb_fix_center );
   connect(cb_fix_center, SIGNAL( clicked() ), SLOT( update_enables() ) );

   cb_pct_center = new QCheckBox(this);
   cb_pct_center->setText(us_tr(" % variation" ) );
   cb_pct_center->setEnabled(true);
   cb_pct_center->setChecked( false );
   cb_pct_center->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pct_center->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pct_center );
   connect(cb_pct_center, SIGNAL( clicked() ), SLOT( update_enables() ) );

   le_pct_center = new mQLineEdit( this );    le_pct_center->setObjectName( "le_pct_center Line Edit" );
   le_pct_center->setText( "5" );
   le_pct_center->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_pct_center->setPalette( PALET_NORMAL );
   AUTFBACK( le_pct_center );
   le_pct_center->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   le_pct_center->setEnabled( false );
   le_pct_center->setMinimumWidth( 50 );
   le_pct_center->setValidator( new QDoubleValidator( le_pct_center ) );
   ( (QDoubleValidator *)le_pct_center->validator() )->setRange( 0, 100, 1 );

   cb_pct_center_from_init = new QCheckBox(this);
   cb_pct_center_from_init->setText(us_tr(" From initial value" ) );
   cb_pct_center_from_init->setEnabled(true);
   cb_pct_center_from_init->setChecked( false );
   cb_pct_center_from_init->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pct_center_from_init->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pct_center_from_init );
   connect(cb_pct_center_from_init, SIGNAL( clicked() ), SLOT( update_enables() ) );

   cb_fix_width = new QCheckBox(this);
   cb_fix_width->setText(us_tr(" Fix Gaussian widths" ) );
   cb_fix_width->setEnabled(true);
   cb_fix_width->setChecked( false );
   cb_fix_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_fix_width->setPalette( PALET_NORMAL );
   AUTFBACK( cb_fix_width );
   connect(cb_fix_width, SIGNAL( clicked() ), SLOT( update_enables() ) );

   cb_pct_width = new QCheckBox(this);
   cb_pct_width->setText(us_tr(" % variation" ) );
   cb_pct_width->setEnabled(true);
   cb_pct_width->setChecked( false );
   cb_pct_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pct_width->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pct_width );
   connect(cb_pct_width, SIGNAL( clicked() ), SLOT( update_enables() ) );

   le_pct_width = new mQLineEdit( this );    le_pct_width->setObjectName( "le_pct_width Line Edit" );
   le_pct_width->setText( "5" );
   le_pct_width->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_pct_width->setPalette( PALET_NORMAL );
   AUTFBACK( le_pct_width );
   le_pct_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   le_pct_width->setEnabled( false );
   le_pct_width->setMinimumWidth( 50 );
   le_pct_width->setValidator( new QDoubleValidator( le_pct_width ) );
   ( (QDoubleValidator *)le_pct_width->validator() )->setRange( 0, 100, 1 );

   cb_pct_width_from_init = new QCheckBox(this);
   cb_pct_width_from_init->setText(us_tr(" From initial value" ) );
   cb_pct_width_from_init->setEnabled(true);
   cb_pct_width_from_init->setChecked( false );
   cb_pct_width_from_init->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pct_width_from_init->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pct_width_from_init );
   connect(cb_pct_width_from_init, SIGNAL( clicked() ), SLOT( update_enables() ) );

   cb_fix_amplitude = new QCheckBox(this);
   cb_fix_amplitude->setText(us_tr(" Fix Gaussian amplitudes" ) );
   cb_fix_amplitude->setEnabled(true);
   cb_fix_amplitude->setChecked( false );
   cb_fix_amplitude->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_fix_amplitude->setPalette( PALET_NORMAL );
   AUTFBACK( cb_fix_amplitude );
   connect(cb_fix_amplitude, SIGNAL( clicked() ), SLOT( update_enables() ) );

   cb_pct_amplitude = new QCheckBox(this);
   cb_pct_amplitude->setText(us_tr(" % variation" ) );
   cb_pct_amplitude->setEnabled(true);
   cb_pct_amplitude->setChecked( false );
   cb_pct_amplitude->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pct_amplitude->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pct_amplitude );
   connect(cb_pct_amplitude, SIGNAL( clicked() ), SLOT( update_enables() ) );

   le_pct_amplitude = new mQLineEdit( this );    le_pct_amplitude->setObjectName( "le_pct_amplitude Line Edit" );
   le_pct_amplitude->setText( "5" );
   le_pct_amplitude->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_pct_amplitude->setPalette( PALET_NORMAL );
   AUTFBACK( le_pct_amplitude );
   le_pct_amplitude->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   le_pct_amplitude->setEnabled( false );
   le_pct_amplitude->setMinimumWidth( 50 );
   le_pct_amplitude->setValidator( new QDoubleValidator( le_pct_amplitude ) );
   ( (QDoubleValidator *)le_pct_amplitude->validator() )->setRange( 0, 100, 1 );

   cb_pct_amplitude_from_init = new QCheckBox(this);
   cb_pct_amplitude_from_init->setText(us_tr(" From initial value" ) );
   cb_pct_amplitude_from_init->setEnabled(true);
   cb_pct_amplitude_from_init->setChecked( false );
   cb_pct_amplitude_from_init->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pct_amplitude_from_init->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pct_amplitude_from_init );
   connect(cb_pct_amplitude_from_init, SIGNAL( clicked() ), SLOT( update_enables() ) );

   cb_fix_dist1 = new QCheckBox(this);
   cb_fix_dist1->setText(us_tr(" Fix distortion 1" ) );
   cb_fix_dist1->setEnabled(true);
   cb_fix_dist1->setChecked( false );
   cb_fix_dist1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_fix_dist1->setPalette( PALET_NORMAL );
   AUTFBACK( cb_fix_dist1 );
   connect(cb_fix_dist1, SIGNAL( clicked() ), SLOT( update_enables() ) );

   cb_pct_dist1 = new QCheckBox(this);
   cb_pct_dist1->setText(us_tr(" % variation" ) );
   cb_pct_dist1->setEnabled(true);
   cb_pct_dist1->setChecked( false );
   cb_pct_dist1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pct_dist1->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pct_dist1 );
   connect(cb_pct_dist1, SIGNAL( clicked() ), SLOT( update_enables() ) );

   le_pct_dist1 = new mQLineEdit( this );    le_pct_dist1->setObjectName( "le_pct_dist1 Line Edit" );
   le_pct_dist1->setText( "5" );
   le_pct_dist1->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_pct_dist1->setPalette( PALET_NORMAL );
   AUTFBACK( le_pct_dist1 );
   le_pct_dist1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   le_pct_dist1->setEnabled( false );
   le_pct_dist1->setMinimumWidth( 50 );
   le_pct_dist1->setValidator( new QDoubleValidator( le_pct_dist1 ) );
   ( (QDoubleValidator *)le_pct_dist1->validator() )->setRange( 0, 100, 1 );

   cb_pct_dist1_from_init = new QCheckBox(this);
   cb_pct_dist1_from_init->setText(us_tr(" From initial value" ) );
   cb_pct_dist1_from_init->setEnabled(true);
   cb_pct_dist1_from_init->setChecked( false );
   cb_pct_dist1_from_init->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pct_dist1_from_init->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pct_dist1_from_init );
   connect(cb_pct_dist1_from_init, SIGNAL( clicked() ), SLOT( update_enables() ) );

   cb_fix_dist2 = new QCheckBox(this);
   cb_fix_dist2->setText(us_tr(" Fix distortion 2" ) );
   cb_fix_dist2->setEnabled(true);
   cb_fix_dist2->setChecked( false );
   cb_fix_dist2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_fix_dist2->setPalette( PALET_NORMAL );
   AUTFBACK( cb_fix_dist2 );
   connect(cb_fix_dist2, SIGNAL( clicked() ), SLOT( update_enables() ) );

   cb_pct_dist2 = new QCheckBox(this);
   cb_pct_dist2->setText(us_tr(" % variation" ) );
   cb_pct_dist2->setEnabled(true);
   cb_pct_dist2->setChecked( false );
   cb_pct_dist2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pct_dist2->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pct_dist2 );
   connect(cb_pct_dist2, SIGNAL( clicked() ), SLOT( update_enables() ) );

   le_pct_dist2 = new mQLineEdit( this );    le_pct_dist2->setObjectName( "le_pct_dist2 Line Edit" );
   le_pct_dist2->setText( "5" );
   le_pct_dist2->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_pct_dist2->setPalette( PALET_NORMAL );
   AUTFBACK( le_pct_dist2 );
   le_pct_dist2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   le_pct_dist2->setEnabled( false );
   le_pct_dist2->setMinimumWidth( 50 );
   le_pct_dist2->setValidator( new QDoubleValidator( le_pct_dist2 ) );
   ( (QDoubleValidator *)le_pct_dist2->validator() )->setRange( 0, 100, 1 );

   cb_pct_dist2_from_init = new QCheckBox(this);
   cb_pct_dist2_from_init->setText(us_tr(" From initial value" ) );
   cb_pct_dist2_from_init->setEnabled(true);
   cb_pct_dist2_from_init->setChecked( false );
   cb_pct_dist2_from_init->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pct_dist2_from_init->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pct_dist2_from_init );
   connect(cb_pct_dist2_from_init, SIGNAL( clicked() ), SLOT( update_enables() ) );

   cb_comm_dist1 = new QCheckBox(this);
   cb_comm_dist1->setText(us_tr(" Common distortion 1" ) );
   cb_comm_dist1->setEnabled( true );
   cb_comm_dist1->setChecked( false );
   cb_comm_dist1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_comm_dist1->setPalette( PALET_NORMAL );
   AUTFBACK( cb_comm_dist1 );
   connect(cb_comm_dist1, SIGNAL( clicked() ), SLOT( update_common() ) );

   cb_comm_dist2 = new QCheckBox(this);
   cb_comm_dist2->setText(us_tr(" Common distortion 2" ) );
   cb_comm_dist2->setEnabled(true);
   cb_comm_dist2->setChecked( false );
   cb_comm_dist2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_comm_dist2->setPalette( PALET_NORMAL );
   AUTFBACK( cb_comm_dist2 );
   connect(cb_comm_dist2, SIGNAL( clicked() ), SLOT( update_common() ) );

   lbl_fix_curves = new QLabel(us_tr(" Fix Gaussians: "), this);
   lbl_fix_curves->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   // lbl_fix_curves->setMinimumHeight(minHeight1);
   lbl_fix_curves->setPalette( PALET_LABEL );
   AUTFBACK( lbl_fix_curves );
   lbl_fix_curves->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   /* old way
   QString qs_rx = QString( "^(|(\\d+)|(\\d+(,\\d+){0,%1}))$" ).arg( mals_saxs_win->gaussians.size() / 3 - 1 );
   cout << "qs_rx:" << qs_rx << endl;
   QRegExp rx_fix_curves( qs_rx );
   QRegExpValidator *rx_val_fix_curves = new QRegExpValidator( rx_fix_curves, this );
   le_fix_curves = new mQLineEdit( this );    le_fix_curves->setObjectName( "le_fix_curves Line Edit" );
   le_fix_curves->setText( "" );
   le_fix_curves->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_fix_curves->setPalette( PALET_NORMAL );
   AUTFBACK( le_fix_curves );
   le_fix_curves->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   le_fix_curves->setEnabled( false );
   le_fix_curves->setValidator( rx_val_fix_curves );
   connect( le_fix_curves, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   */

   for ( unsigned int i = 0; i < ( unsigned int ) mals_saxs_win->gaussians.size() / gaussian_type_size; i++ )
   {
      QCheckBox *cb_tmp;
      cb_tmp = new QCheckBox(this);
      cb_tmp->setText( QString( " %1 " ).arg( i + 1 ) );
      cb_tmp->setEnabled( true );
      cb_tmp->setChecked( false );
      cb_tmp->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      cb_tmp->setPalette( PALET_NORMAL );
      AUTFBACK( cb_tmp );
      connect(cb_tmp, SIGNAL( clicked() ), SLOT( update_enables() ) );
      cb_fix_curves.push_back( cb_tmp );
   }

   cb_conc_test = new QCheckBox(this);
   cb_conc_test->setText(us_tr(" Fix width ratios" ) );
   cb_conc_test->setEnabled( true );
   cb_conc_test->setChecked( false );
   cb_conc_test->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_conc_test->setPalette( PALET_NORMAL );
   AUTFBACK( cb_conc_test );
   connect(cb_conc_test, SIGNAL( clicked() ), SLOT( update_enables() ) );

   lbl_epsilon = new QLabel(us_tr(" Epsilon: "), this);
   lbl_epsilon->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   // lbl_epsilon->setMinimumHeight(minHeight1);
   lbl_epsilon->setPalette( PALET_LABEL );
   AUTFBACK( lbl_epsilon );
   lbl_epsilon->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_epsilon = new mQLineEdit( this );    le_epsilon->setObjectName( "le_epsilon Line Edit" );
   double peak = mals_saxs_win->compute_gaussian_peak( mals_saxs_win->wheel_file, mals_saxs_win->gaussians );
   le_epsilon->setText( QString( "%1" ).arg( peak / 1e6 < 0.001 ? peak / 1e6 : 0.001 ) );
   le_epsilon->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_epsilon->setPalette( PALET_NORMAL );
   AUTFBACK( le_epsilon );
   le_epsilon->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   le_epsilon->setEnabled( false );
   le_epsilon->setValidator( new QDoubleValidator( le_epsilon ) );
   ( (QDoubleValidator *)le_epsilon->validator() )->setRange( 0e-3, 10, 3 );

   lbl_iterations = new QLabel(us_tr(" Iterations: "), this);
   lbl_iterations->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   // lbl_iterations->setMinimumHeight(minHeight1);
   lbl_iterations->setPalette( PALET_LABEL );
   AUTFBACK( lbl_iterations );
   lbl_iterations->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_iterations = new mQLineEdit( this );    le_iterations->setObjectName( "le_iterations Line Edit" );
   le_iterations->setText( "100" );
   le_iterations->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_iterations->setPalette( PALET_NORMAL );
   AUTFBACK( le_iterations );
   le_iterations->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   le_iterations->setEnabled( false );
   le_iterations->setValidator( new QIntValidator( le_iterations ) );
   ( (QIntValidator *)le_iterations->validator() )->setRange( 1, 10000 );

   lbl_population = new QLabel(us_tr(" Maximum calls/Population/Grid: "), this);
   lbl_population->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   // lbl_population->setMinimumHeight(minHeight1);
   lbl_population->setPalette( PALET_LABEL );
   AUTFBACK( lbl_population );
   lbl_population->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_population = new mQLineEdit( this );    le_population->setObjectName( "le_population Line Edit" );
   le_population->setText( "100" );
   le_population->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_population->setPalette( PALET_NORMAL );
   AUTFBACK( le_population );
   le_population->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   le_population->setEnabled( false );
   le_population->setValidator( new QIntValidator( le_population ) );
   ( (QIntValidator *)le_population->validator() )->setRange( 2, 10000 );

   pb_restore = new QPushButton(us_tr("Restore to initial values"), this);
   pb_restore->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_restore->setMinimumHeight(minHeight1);
   pb_restore->setPalette( PALET_PUSHB );
   connect(pb_restore, SIGNAL(clicked()), SLOT(restore()));

   pb_undo = new QPushButton(us_tr("Undo"), this);
   pb_undo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_undo->setMinimumHeight(minHeight1);
   pb_undo->setPalette( PALET_PUSHB );
   connect(pb_undo, SIGNAL(clicked()), SLOT(undo()));

   pb_lm = new QPushButton(us_tr("LM"), this);
   pb_lm->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_lm->setMinimumHeight(minHeight1);
   pb_lm->setPalette( PALET_PUSHB );
   connect(pb_lm, SIGNAL(clicked()), SLOT(lm()));

   pb_gsm_sd = new QPushButton(us_tr("GS SD"), this);
   pb_gsm_sd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_gsm_sd->setMinimumHeight(minHeight1);
   pb_gsm_sd->setPalette( PALET_PUSHB );
   connect(pb_gsm_sd, SIGNAL(clicked()), SLOT(gsm_sd()));

   pb_gsm_ih = new QPushButton(us_tr("GS IH"), this);
   pb_gsm_ih->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_gsm_ih->setMinimumHeight(minHeight1);
   pb_gsm_ih->setPalette( PALET_PUSHB );
   connect(pb_gsm_ih, SIGNAL(clicked()), SLOT(gsm_ih()));

   pb_gsm_cg = new QPushButton(us_tr("GS CG"), this);
   pb_gsm_cg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_gsm_cg->setMinimumHeight(minHeight1);
   pb_gsm_cg->setPalette( PALET_PUSHB );
   connect(pb_gsm_cg, SIGNAL(clicked()), SLOT(gsm_cg()));

   pb_ga = new QPushButton(us_tr("GA"), this);
   pb_ga->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_ga->setMinimumHeight(minHeight1);
   pb_ga->setPalette( PALET_PUSHB );
   connect(pb_ga, SIGNAL(clicked()), SLOT(ga()));

   pb_grid = new QPushButton(us_tr("Grid"), this);
   pb_grid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_grid->setMinimumHeight(minHeight1);
   pb_grid->setPalette( PALET_PUSHB );
   connect(pb_grid, SIGNAL(clicked()), SLOT(grid()));

   progress = new QProgressBar( this );
   // progress->setMinimumHeight(minHeight1);
   progress->setPalette( PALET_NORMAL );
   AUTFBACK( progress );
   progress->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize  ));
   progress->reset();

   pb_stop = new QPushButton(us_tr("Stop"), this);
   pb_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_stop->setMinimumHeight(minHeight1);
   pb_stop->setPalette( PALET_PUSHB );
   connect(pb_stop, SIGNAL(clicked()), SLOT(stop()));

   pb_help = new QPushButton(us_tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(us_tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));


   // build layout
   QGridLayout * gl_main = new QGridLayout( 0 ); gl_main->setContentsMargins( 0, 0, 0, 0 ); gl_main->setSpacing( 0 );
   int row = 0;

   gl_main->addWidget( cb_fix_center           , row, 0 );
   gl_main->addWidget( cb_pct_center           , row, 1 );
   gl_main->addWidget( le_pct_center           , row, 2 );
   gl_main->addWidget( cb_pct_center_from_init , row, 3 );
   row++;

   gl_main->addWidget( cb_fix_width           , row, 0 );
   gl_main->addWidget( cb_pct_width           , row, 1 );
   gl_main->addWidget( le_pct_width           , row, 2 );
   gl_main->addWidget( cb_pct_width_from_init , row, 3 );
   row++;

   gl_main->addWidget( cb_fix_amplitude           , row, 0 );
   gl_main->addWidget( cb_pct_amplitude           , row, 1 );
   gl_main->addWidget( le_pct_amplitude           , row, 2 );
   gl_main->addWidget( cb_pct_amplitude_from_init , row, 3 );
   row++;

   gl_main->addWidget( cb_fix_dist1           , row, 0 );
   gl_main->addWidget( cb_pct_dist1           , row, 1 );
   gl_main->addWidget( le_pct_dist1           , row, 2 );
   gl_main->addWidget( cb_pct_dist1_from_init , row, 3 );
   row++;

   gl_main->addWidget( cb_fix_dist2           , row, 0 );
   gl_main->addWidget( cb_pct_dist2           , row, 1 );
   gl_main->addWidget( le_pct_dist2           , row, 2 );
   gl_main->addWidget( cb_pct_dist2_from_init , row, 3 );
   row++;

   {
      QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( cb_comm_dist1 );
      hbl->addWidget( cb_comm_dist2 );
      gl_main->addLayout( hbl , row , 0 , 1 + ( row ) - ( row ) , 1 + ( 3  ) - ( 0 ) );
      row++;
   }

   gl_main->addWidget         ( lbl_fix_curves, row, 0 );
   // gl_main->addWidget( le_fix_curves  , row , 1 , 1 + ( row ) - ( row ) , 1 + ( 3  ) - ( 1 ) );
   QHBoxLayout * hbl_fix_curves = new QHBoxLayout; hbl_fix_curves->setContentsMargins( 0, 0, 0, 0 ); hbl_fix_curves->setSpacing( 0 );
   for ( unsigned int i = 0; i < ( unsigned int ) cb_fix_curves.size(); i++ )
   {
      hbl_fix_curves->addWidget( cb_fix_curves[ i ] );
   }
   gl_main->addLayout( hbl_fix_curves  , row , 1 , 1 + ( row ) - ( row ) , 1 + ( 3  ) - ( 1 ) );
   row++;

   gl_main->addWidget( cb_conc_test , row , 0 , 1 + ( row ) - ( row ) , 1 + ( 3  ) - ( 0 ) );
   row++;

   gl_main->addWidget         ( lbl_epsilon, row, 0 );
   gl_main->addWidget( le_epsilon  , row , 1 , 1 + ( row ) - ( row ) , 1 + ( 3  ) - ( 1 ) );
   row++;

   gl_main->addWidget         ( lbl_iterations, row, 0 );
   gl_main->addWidget( le_iterations  , row , 1 , 1 + ( row ) - ( row ) , 1 + ( 3  ) - ( 1 ) );
   row++;

   gl_main->addWidget         ( lbl_population, row, 0 );
   gl_main->addWidget( le_population  , row , 1 , 1 + ( row ) - ( row ) , 1 + ( 3  ) - ( 1 ) );
   row++;

   QHBoxLayout * hbl_restore_undo = new QHBoxLayout; hbl_restore_undo->setContentsMargins( 0, 0, 0, 0 ); hbl_restore_undo->setSpacing( 0 );
   hbl_restore_undo->addWidget ( pb_restore );
   hbl_restore_undo->addWidget ( pb_undo );

   QHBoxLayout * hbl_runs = new QHBoxLayout; hbl_runs->setContentsMargins( 0, 0, 0, 0 ); hbl_runs->setSpacing( 0 );
   hbl_runs->addWidget ( pb_lm );
   hbl_runs->addWidget ( pb_gsm_sd );
   hbl_runs->addWidget ( pb_gsm_ih );
   hbl_runs->addWidget ( pb_gsm_cg );
   hbl_runs->addWidget ( pb_ga );
   hbl_runs->addWidget ( pb_grid );

   QHBoxLayout * hbl_prog = new QHBoxLayout; hbl_prog->setContentsMargins( 0, 0, 0, 0 ); hbl_prog->setSpacing( 0 );
   hbl_prog->addWidget ( progress );
   hbl_prog->addWidget ( pb_stop );

   QHBoxLayout * hbl_bottom = new QHBoxLayout; hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addWidget ( pb_cancel );

   QVBoxLayout * background = new QVBoxLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );
   background->addWidget ( lbl_title );
   background->addSpacing( 4 );
   background->addLayout ( gl_main  );
   background->addSpacing( 4 );
   background->addLayout ( hbl_restore_undo );
   background->addSpacing( 4 );
   background->addLayout ( hbl_runs );
   background->addSpacing( 4 );
   background->addLayout ( hbl_prog );
   background->addSpacing( 4 );
   background->addLayout ( hbl_bottom );

   // void * us_hydrodyn = mals_saxs_win->us_hydrodyn;

   if ( !U_EXPT )
   {
      // pb_gsm_sd->hide();
      // pb_gsm_ih->hide();
      // pb_gsm_cg->hide();
      pb_ga    ->hide();
      pb_grid  ->hide();
      lbl_population->setText( us_tr( "Maximum calls: " ) );
   }

   if ( !dist1_active )
   {
      cb_fix_dist1->hide();
      cb_pct_dist1->hide();
      le_pct_dist1->hide();
      cb_pct_dist1_from_init->hide();
      cb_comm_dist1->hide();
   }
   if ( !dist2_active )
   {
      cb_fix_dist2->hide();
      cb_pct_dist2->hide();
      le_pct_dist2->hide();
      cb_pct_dist2_from_init->hide();
      cb_comm_dist2->hide();
   }
   if ( cb_fix_curves.size() < 2 )
   {
      cb_comm_dist1->hide();
      cb_comm_dist2->hide();
   } else {
      cb_comm_dist1->setChecked( set_comm_dist && dist1_active );
      cb_comm_dist2->setChecked( set_comm_dist && dist2_active );
   }
}

void US_Hydrodyn_Mals_Saxs_Fit::cancel()
{
   close();
}

void US_Hydrodyn_Mals_Saxs_Fit::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo/somo_mals_saxs_fit.html");
}

void US_Hydrodyn_Mals_Saxs_Fit::closeEvent(QCloseEvent *e)
{
   global_Xpos -= 3;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Mals_Saxs_Fit::update_common()
{
   if ( !setup_run() )
   {
      update_enables();
      return;
   }

   vector < double > save_gaussians = mals_saxs_win->gaussians;
   for ( unsigned int i = 0; i < MSFIT::param_fixed.size(); i++ )
   {
      if ( !MSFIT::param_fixed[ i ] ||
           MSFIT::comm_backref.count( i ) )
      {
         mals_saxs_win->gaussians[ i ] = MSFIT::init_params[ 
                                                      MSFIT::comm_backref.count( i ) ?
                                                      MSFIT::comm_backref[ i ] :
                                                      MSFIT::param_pos[ i ] 
                                                       ];
         if ( MSFIT::conc_ratios_map.count( i ) )
         {
            mals_saxs_win->gaussians[ i ] *= MSFIT::conc_ratios_map[ i ];
         }
      }
   }
   if ( update_mals_saxs && save_gaussians != mals_saxs_win->gaussians )
   {
      gaussians_undo.push_back( mals_saxs_win->gaussians );
      mals_saxs_win->gauss_init_markers();
      mals_saxs_win->gauss_init_gaussians();
      mals_saxs_win->update_gauss_pos();
   }
   update_enables();
}

void US_Hydrodyn_Mals_Saxs_Fit::update_enables()
{
   // us_qdebug( "update_enables()" );
   // puts( "hf: ue()" );
   bool run_ok = setup_run();
   // cout << QString( "fit::fix center %1\n"
   //                  "fit::fix width  %2\n"
   //                  "fit::fix height %3\n"
   //                  "fit::fix dist1  %4\n"
   //                  "fit::fix dist2  %5\n" )
   //    .arg( cb_fix_center->isChecked() ? "true" : "false" )
   //    .arg( cb_fix_width->isChecked() ? "true" : "false" )
   //    .arg( cb_fix_amplitude->isChecked() ? "true" : "false" )
   //    .arg( cb_fix_dist1->isChecked() ? "true" : "false" )
   //    .arg( cb_fix_dist2->isChecked() ? "true" : "false" )
   //    ;

   cb_fix_center                ->setEnabled( !running );
   cb_pct_center                ->setEnabled( !running && !cb_fix_center->isChecked() );
   le_pct_center                ->setEnabled( !running && !cb_fix_center->isChecked() && cb_pct_center->isChecked() );
   cb_pct_center_from_init      ->setEnabled( !running && !cb_fix_center->isChecked() );

   if ( cb_conc_test->isChecked() )
      // && 
      //        ( cb_fix_width->isChecked() ||
      //          cb_pct_width->isChecked() ) )
   {
      cb_fix_width->setChecked( false );
      cb_pct_width->setChecked( false );
      for ( unsigned int i = 0; i < ( unsigned int ) cb_fix_curves.size(); i++ )
      {
         cb_fix_curves[ i ]      ->setChecked( false );
      }
   }      

   cb_fix_width                 ->setEnabled( !cb_conc_test->isChecked() && !running );
   cb_pct_width                 ->setEnabled( !cb_conc_test->isChecked() && !running && !cb_fix_width->isChecked() );
   le_pct_width                 ->setEnabled( !cb_conc_test->isChecked() && !running && !cb_fix_width->isChecked()  && cb_pct_width->isChecked() );
   cb_pct_width_from_init       ->setEnabled( !cb_conc_test->isChecked() && !running && !cb_fix_width->isChecked() );

   cb_fix_amplitude             ->setEnabled( !running );
   cb_pct_amplitude             ->setEnabled( !running && !cb_fix_amplitude->isChecked() );
   le_pct_amplitude             ->setEnabled( !running && !cb_fix_amplitude->isChecked() && cb_pct_amplitude->isChecked() );
   cb_pct_amplitude_from_init   ->setEnabled( !running && !cb_fix_amplitude->isChecked() );

   cb_fix_dist1                 ->setEnabled( !running );
   cb_pct_dist1                 ->setEnabled( !running && !cb_fix_dist1->isChecked() );
   le_pct_dist1                 ->setEnabled( !running && !cb_fix_dist1->isChecked() && cb_pct_dist1->isChecked() );
   cb_pct_dist1_from_init       ->setEnabled( !running && !cb_fix_dist1->isChecked() );

   cb_fix_dist2                 ->setEnabled( !running );
   cb_pct_dist2                 ->setEnabled( !running && !cb_fix_dist2->isChecked() );
   le_pct_dist2                 ->setEnabled( !running && !cb_fix_dist2->isChecked() && cb_pct_dist2->isChecked() );
   cb_pct_dist2_from_init       ->setEnabled( !running && !cb_fix_dist2->isChecked() );

   for ( unsigned int i = 0; i < ( unsigned int ) cb_fix_curves.size(); i++ )
   {
      cb_fix_curves[ i ]           ->setEnabled( !cb_conc_test->isChecked() && !running );
   }

   cb_comm_dist1                ->setEnabled( !running && !cb_fix_dist1->isChecked() );
   cb_comm_dist2                ->setEnabled( !running && !cb_fix_dist2->isChecked() );
   if ( !running && cb_fix_dist1->isChecked() )
   {
      cb_comm_dist1                ->setChecked( false );
   }
   if ( !running && cb_fix_dist2->isChecked() )
   {
      cb_comm_dist2                ->setChecked( false );
   }

   // le_fix_curves            ->setEnabled( !running );
   le_epsilon               ->setEnabled( !running );
   le_iterations            ->setEnabled( !running );
   le_population            ->setEnabled( !running );

   bool variations_set      = 
      ( cb_fix_center   ->isChecked() || cb_pct_center   ->isChecked() ) &&
      ( cb_fix_width    ->isChecked() || cb_pct_width    ->isChecked() ) &&
      ( cb_fix_amplitude->isChecked() || cb_pct_amplitude->isChecked() ) &&

      ( !dist1_active || cb_fix_dist1->isChecked() || cb_pct_dist1->isChecked() ) &&
      ( !dist2_active || cb_fix_dist2->isChecked() || cb_pct_dist2->isChecked() )
      ;

   pb_restore               ->setEnabled( !running && gaussians_undo.size() > 1 );
   pb_undo                  ->setEnabled( !running && gaussians_undo.size() > 1 );

   pb_lm                    ->setEnabled( !running && run_ok );
   pb_gsm_sd                ->setEnabled( !running && run_ok && !use_errors );
   pb_gsm_ih                ->setEnabled( !running && run_ok && !use_errors );
   pb_gsm_cg                ->setEnabled( !running && run_ok && !use_errors );
   pb_ga                    ->setEnabled( !running && run_ok && variations_set && !use_errors );
   pb_grid                  ->setEnabled( !running && run_ok && variations_set && !use_errors );

   pb_stop                  ->setEnabled( running );
}

namespace MSFIT 
{
   vector < double       > init_params;    // variable param initial values
   vector < double       > base_params;    // variable param initial values
   vector < double       > last_params;    // variable param initial values

   vector < double       > fixed_params;   // the fixed params
   vector < unsigned int > param_pos;      // index into fixed params or variable params
   vector < bool         > param_fixed;    
   vector < double       > param_min;      // minimum values for variable params
   vector < double       > param_max;      // maximum values for variable params

   map < unsigned int, unsigned int > comm_backref; // back reference to variable param position of 1st usage

   bool                    conc_test;
   vector < double >       conc_ratios;
   map < unsigned int,  double >  conc_ratios_map;

   vector < double       > errors;
   vector < unsigned int > errors_index;

   bool                    use_errors;

   double (*compute_gaussian_f)( double, const double * );

   double compute_gaussian_f_GAUSS( double t, const double *par )
   {
      double result = 0e0;
      double height;
      double center;
      double width;

      for ( unsigned int i = 0; i < ( unsigned int ) param_fixed.size(); )
      {
         if ( param_fixed[ i ] )
         {
            height = fixed_params[ param_pos[ i ] ];
         } else {
            height = par         [ param_pos[ i ] ];
            if ( height < param_min[ param_pos[ i ] ] ||
                 height > param_max[ param_pos[ i ] ] )
            {
               return 1e99;
            }
         }

         i++;

         if ( param_fixed[ i ] )
         {
            center = fixed_params[ param_pos[ i ] ];
         } else {
            center = par         [ param_pos[ i ] ];
            if ( center < param_min[ param_pos[ i ] ] ||
                 center > param_max[ param_pos[ i ] ] )
            {
               return 1e99;
            }
         }

         i++;

         if ( param_fixed[ i ] )
         {
            width = fixed_params[ param_pos[ i ] ];
         } else {
            width = par         [ param_pos[ i ] ];
            if ( width < param_min[ param_pos[ i ] ] ||
                 width > param_max[ param_pos[ i ] ] )
            {
               return 1e99;
            }
         }

         i++;

         double tmp = ( t - center ) / width;
         result += height * exp( - tmp * tmp * 5e-1 );
      }
      
      if ( use_errors )
      {
         result /= errors[ errors_index[ (unsigned int) t ] ];
      }
      
      return result;
   }

   double compute_gaussian_f_EMG( double t, const double *par )
   {
      double result = 0e0;
      double height;
      double center;
      double width;
      double dist1;

      for ( unsigned int i = 0; i < ( unsigned int ) param_fixed.size(); )
      {
         if ( param_fixed[ i ] )
         {
            height = fixed_params[ param_pos[ i ] ];
         } else {
            height = par         [ param_pos[ i ] ];
            if ( height < param_min[ param_pos[ i ] ] ||
                 height > param_max[ param_pos[ i ] ] )
            {
               return 1e99;
            }
         }

         i++;

         if ( param_fixed[ i ] )
         {
            center = fixed_params[ param_pos[ i ] ];
         } else {
            center = par         [ param_pos[ i ] ];
            if ( center < param_min[ param_pos[ i ] ] ||
                 center > param_max[ param_pos[ i ] ] )
            {
               return 1e99;
            }
         }

         i++;

         if ( param_fixed[ i ] )
         {
            width = fixed_params[ param_pos[ i ] ];
         } else {
            width = par         [ param_pos[ i ] ];
            if ( width < param_min[ param_pos[ i ] ] ||
                 width > param_max[ param_pos[ i ] ] )
            {
               return 1e99;
            }
         }

         i++;

         if ( param_fixed[ i ] )
         {
            if ( comm_backref.count( i ) )
            {
               dist1 = par[ comm_backref[ i ] ];
            } else {
               dist1 = fixed_params[ param_pos[ i ] ];
            }               
         } else {
            dist1 = par         [ param_pos[ i ] ];
            if ( dist1 < param_min[ param_pos[ i ] ] ||
                 dist1 > param_max[ param_pos[ i ] ] )
            {
               return 1e99;
            }
         }

         i++;

         if ( !dist1 )
         {
            double tmp = ( t - center ) / width;
            result += height * exp( - tmp * tmp * 5e-1 );
         } else {
            double dist1_thresh      = width / ( 5e0 * sqrt(2e0) - 2e0 );
            if ( fabs( dist1 ) < dist1_thresh )
            {
               double frac_gauss = ( dist1_thresh - fabs( dist1 ) ) / dist1_thresh;
               if ( dist1 < 0 )
               {
                  dist1_thresh *= -1e0;
               }

               double area              = height * width * M_SQRT2PI;
               double one_over_a3       = 1e0 / dist1_thresh;
               double emg_coeff         = area * one_over_a3 * 5e-1 * ( 1e0 - frac_gauss );
               double emg_exp_1         = width * width * one_over_a3 * one_over_a3 * 5e-1;
               double emg_erf_2         = width * M_ONE_OVER_SQRT2 * one_over_a3;
               double sign_a3           = dist1_thresh < 0e0 ? -1e0 : 1e0;
               double one_over_sqrt2_a2 = M_ONE_OVER_SQRT2 / width;
               double gauss_coeff       = frac_gauss * height;

               double tmp               = t - center;
               double tmp2              = tmp / width;
               
               result +=
                  emg_coeff * exp( emg_exp_1 - one_over_a3 * tmp ) *
                  ( use_erf( tmp * one_over_sqrt2_a2 - emg_erf_2 ) + sign_a3 ) +
                  gauss_coeff * exp( - tmp2 * tmp2 * 5e-1 );
               ;
            } else {
               double area              = height * width * M_SQRT2PI;
               double one_over_a3       = 1e0 / dist1;
               double emg_coeff         = area * one_over_a3 * 5e-1;
               double emg_exp_1         = width * width * one_over_a3 * one_over_a3 * 5e-1;
               double emg_erf_2         = width * M_ONE_OVER_SQRT2 * one_over_a3;
               double sign_a3           = dist1 < 0e0 ? -1e0 : 1e0;
               double one_over_sqrt2_a2 = M_ONE_OVER_SQRT2 / width;
               double tmp               = t - center;
               result += 
                  emg_coeff * exp( emg_exp_1 - one_over_a3 * tmp ) *
                  ( use_erf( tmp * one_over_sqrt2_a2 - emg_erf_2 ) + sign_a3 );
            }
         }
      }
      
      if ( use_errors )
      {
         result /= errors[ errors_index[ (unsigned int) t ] ];
      }
      
      return result;
   }

   double compute_gaussian_f_GMG( double t, const double *par )
   {
      double result = 0e0;
      double height;
      double center;
      double width;
      double dist1;

      for ( unsigned int i = 0; i < ( unsigned int ) param_fixed.size(); )
      {
         if ( param_fixed[ i ] )
         {
            height = fixed_params[ param_pos[ i ] ];
         } else {
            height = par         [ param_pos[ i ] ];
            if ( height < param_min[ param_pos[ i ] ] ||
                 height > param_max[ param_pos[ i ] ] )
            {
               return 1e99;
            }
         }

         i++;

         if ( param_fixed[ i ] )
         {
            center = fixed_params[ param_pos[ i ] ];
         } else {
            center = par         [ param_pos[ i ] ];
            if ( center < param_min[ param_pos[ i ] ] ||
                 center > param_max[ param_pos[ i ] ] )
            {
               return 1e99;
            }
         }

         i++;

         if ( param_fixed[ i ] )
         {
            width = fixed_params[ param_pos[ i ] ];
         } else {
            width = par         [ param_pos[ i ] ];
            if ( width < param_min[ param_pos[ i ] ] ||
                 width > param_max[ param_pos[ i ] ] )
            {
               return 1e99;
            }
         }

         i++;

         if ( param_fixed[ i ] )
         {
            if ( comm_backref.count( i ) )
            {
               dist1 = par[ comm_backref[ i ] ];
            } else {
               dist1 = fixed_params[ param_pos[ i ] ];
            }               
         } else {
            dist1 = par         [ param_pos[ i ] ];
            if ( dist1 < param_min[ param_pos[ i ] ] ||
                 dist1 > param_max[ param_pos[ i ] ] )
            {
               return 1e99;
            }
         }

         i++;

         if ( !dist1 )
         {
            double tmp = ( t - center ) / width;
            result += height * exp( - tmp * tmp * 5e-1 );
         } else {
            double area                         = height * width * M_SQRT2PI;
            double one_over_width               = 1e0 / width;
            double one_over_a2sq_plus_a3sq      = 1e0 / ( width * width +  dist1 * dist1 );
            double sqrt_one_over_a2sq_plus_a3sq = sqrt( one_over_a2sq_plus_a3sq );
            double gmg_coeff                    = area * M_ONE_OVER_SQRT2PI * sqrt_one_over_a2sq_plus_a3sq;
            double gmg_exp_m1                   = -5e-1 *  one_over_a2sq_plus_a3sq;
            double gmg_erf_m1                   = dist1 * sqrt_one_over_a2sq_plus_a3sq * M_ONE_OVER_SQRT2 * one_over_width;
            double tmp = t - center;
            result += 
               gmg_coeff * exp( gmg_exp_m1 * tmp * tmp ) *
               ( 1e0 + use_erf( gmg_erf_m1 * tmp ) );
         }            
      }
      
      if ( use_errors )
      {
         result /= errors[ errors_index[ (unsigned int) t ] ];
      }
      
      return result;
   }

   double compute_gaussian_f_EMGGMG( double t, const double *par )
   {
      double result = 0e0;
      double height;
      double center;
      double width;
      double dist1;
      double dist2;

      for ( unsigned int i = 0; i < ( unsigned int ) param_fixed.size(); )
      {
         if ( param_fixed[ i ] )
         {
            height = fixed_params[ param_pos[ i ] ];
         } else {
            height = par         [ param_pos[ i ] ];
            if ( height < param_min[ param_pos[ i ] ] ||
                 height > param_max[ param_pos[ i ] ] )
            {
               return 1e99;
            }
         }

         i++;

         if ( param_fixed[ i ] )
         {
            center = fixed_params[ param_pos[ i ] ];
         } else {
            center = par         [ param_pos[ i ] ];
            if ( center < param_min[ param_pos[ i ] ] ||
                 center > param_max[ param_pos[ i ] ] )
            {
               return 1e99;
            }
         }

         i++;

         if ( param_fixed[ i ] )
         {
            if ( comm_backref.count( i ) )
            {
               width = par[ comm_backref[ i ] ] * conc_ratios_map[ i ];
            } else {
               width = fixed_params[ param_pos[ i ] ];
            }
         } else {
            width = par         [ param_pos[ i ] ];
            if ( width < param_min[ param_pos[ i ] ] ||
                 width > param_max[ param_pos[ i ] ] )
            {
               return 1e99;
            }
         }

         i++;

         if ( param_fixed[ i ] )
         {
            if ( comm_backref.count( i ) )
            {
               dist1 = par[ comm_backref[ i ] ];
            } else {
               dist1 = fixed_params[ param_pos[ i ] ];
            }               
         } else {
            dist1 = par         [ param_pos[ i ] ];
            if ( dist1 < param_min[ param_pos[ i ] ] ||
                 dist1 > param_max[ param_pos[ i ] ] )
            {
               return 1e99;
            }
         }

         i++;

         if ( param_fixed[ i ] )
         {
            if ( comm_backref.count( i ) )
            {
               dist2 = par[ comm_backref[ i ] ];
            } else {
               dist2 = fixed_params[ param_pos[ i ] ];
            }               
         } else {
            dist2 = par         [ param_pos[ i ] ];
            if ( dist2 < param_min[ param_pos[ i ] ] ||
                 dist2 > param_max[ param_pos[ i ] ] )
            {
               return 1e99;
            }
         }

         i++;

         if ( !dist1 && !dist2 )
         {
            // just a gaussian
            double tmp = ( t - center ) / width;
            result += height * exp( - tmp * tmp * 5e-1 );
         } else {
            if ( !dist1 )
            {
               // GMG
               double area                         = height * width * M_SQRT2PI;
               double one_over_width               = 1e0 / width;
               double one_over_a2sq_plus_a3sq      = 1e0 / ( width * width + dist2 * dist2 );
               double sqrt_one_over_a2sq_plus_a3sq = sqrt( one_over_a2sq_plus_a3sq );
               double gmg_coeff                    = area * M_ONE_OVER_SQRT2PI * sqrt_one_over_a2sq_plus_a3sq;
               double gmg_exp_m1                   = -5e-1 *  one_over_a2sq_plus_a3sq;
               double gmg_erf_m1                   = dist2 * sqrt_one_over_a2sq_plus_a3sq * M_ONE_OVER_SQRT2 * one_over_width;
               double tmp                          = t - center;
               result += 
                  gmg_coeff * exp( gmg_exp_m1 * tmp * tmp ) *
                  ( 1e0 + use_erf( gmg_erf_m1 * tmp ) );
            } else {
               if ( !dist2 )
               {
                  // same as EMG here
                  double dist1_thresh      = width / ( 5e0 * sqrt(2e0) - 2e0 );
                  if ( fabs( dist1 ) < dist1_thresh )
                  {
                     // EMG averaged with gauss
                     double frac_gauss = ( dist1_thresh - fabs( dist1 ) ) / dist1_thresh;
                     if ( dist1 < 0 )
                     {
                        dist1_thresh *= -1e0;
                     }

                     double area              = height * width * M_SQRT2PI;
                     double one_over_a3       = 1e0 / dist1_thresh;
                     double emg_coeff         = area * one_over_a3 * 5e-1 * (1e0 - frac_gauss );
                     double emg_exp_1         = width * width * one_over_a3 * one_over_a3 * 5e-1;
                     double emg_erf_2         = width * M_ONE_OVER_SQRT2 * one_over_a3;
                     double sign_a3           = dist1_thresh < 0e0 ? -1e0 : 1e0;
                     double one_over_sqrt2_a2 = M_ONE_OVER_SQRT2 / width;
                     double gauss_coeff       = frac_gauss * height;

                     // printf( "EMG t0 %g thresh %g frac gauss %g\n", dist1, dist1_thresh, frac_gauss );

                     double tmp = t - center;
                     double tmp2 =  tmp / width;
               
                     result += 
                        emg_coeff * exp( emg_exp_1 - one_over_a3 * tmp ) *
                        ( use_erf( tmp * one_over_sqrt2_a2 - emg_erf_2 ) + sign_a3 ) +
                        gauss_coeff * exp( - tmp2 * tmp2 * 5e-1 );
                  } else {
                     // pure EMG
                     double area              = height * width * M_SQRT2PI;
                     double one_over_a3       = 1e0 / dist1;
                     double emg_coeff         = area * one_over_a3 * 5e-1;
                     double emg_exp_1         = width * width * one_over_a3 * one_over_a3 * 5e-1;
                     double emg_erf_2         = width * M_ONE_OVER_SQRT2 * one_over_a3;
                     double sign_a3           = dist1 < 0e0 ? -1e0 : 1e0;
                     double one_over_sqrt2_a2 = M_ONE_OVER_SQRT2 / width;
                     double tmp               = t - center;
                     result += 
                        emg_coeff * exp( emg_exp_1 - one_over_a3 * tmp ) *
                        ( use_erf( tmp * one_over_sqrt2_a2 - emg_erf_2 ) + sign_a3 );
                  }
               } else {
                  // real EMGGMG
                  double area                         = height * width * M_SQRT2PI;
                  double one_over_width               = 1e0 / width;
                  double one_over_a2sq_plus_a3sq      = 1e0 / ( width * width + dist2 * dist2 );
                  double sqrt_one_over_a2sq_plus_a3sq = sqrt( one_over_a2sq_plus_a3sq );
                  double gmg_coeff                    = 5e-1 * area * M_ONE_OVER_SQRT2PI * sqrt_one_over_a2sq_plus_a3sq;
                  double gmg_exp_m1                   = -5e-1 * one_over_a2sq_plus_a3sq;
                  double gmg_erf_m1                   = dist2 * sqrt_one_over_a2sq_plus_a3sq * M_ONE_OVER_SQRT2 * one_over_width;

                  double tmp                          = t - center;

                  double dist1_thresh                 = width / ( 5e0 * sqrt(2e0) - 2e0 );

                  if ( fabs( dist1 ) < dist1_thresh )
                  {
                     double frac_gauss = ( dist1_thresh - fabs( dist1 ) ) / dist1_thresh;
                     if ( dist1 < 0 )
                     {
                        dist1_thresh *= -1e0;
                     }

                     double one_over_a3       = 1e0 / dist1_thresh;
                     double emg_coeff         = 5e-1 * area * one_over_a3 * 5e-1 * (1e0 - frac_gauss );
                     double emg_exp_1         = width * width * one_over_a3 * one_over_a3 * 5e-1;
                     double emg_erf_2         = width * M_ONE_OVER_SQRT2 * one_over_a3;

                     double sign_a3           = dist1_thresh < 0e0 ? -1e0 : 1e0;
                     double one_over_sqrt2_a2 = M_ONE_OVER_SQRT2 / width;
                     double gauss_coeff       = 5e-1 * frac_gauss * height;

                     double tmp2              = tmp / width;
               
                     result +=
                        emg_coeff * exp( emg_exp_1 - one_over_a3 * tmp ) *
                        ( use_erf( tmp * one_over_sqrt2_a2 - emg_erf_2 ) + sign_a3 ) +
                        gauss_coeff * exp( - tmp2 * tmp2 * 5e-1 ) +
                        gmg_coeff * exp( gmg_exp_m1 * tmp * tmp ) *
                        ( 1e0 + use_erf( gmg_erf_m1 * tmp ) );
                  } else {
                     // EMG
                     double one_over_a3       = 1e0 / dist1;
                     double emg_coeff         = 5e-1 * area * one_over_a3 * 5e-1;
                     double emg_exp_1         = width * width * one_over_a3 * one_over_a3 * 5e-1;
                     double emg_erf_2         = width * M_ONE_OVER_SQRT2 * one_over_a3;
                     double sign_a3           = dist1 < 0e0 ? -1e0 : 1e0;
                     double one_over_sqrt2_a2 = M_ONE_OVER_SQRT2 / width;

                     result += 
                        emg_coeff * exp( emg_exp_1 - one_over_a3 * tmp ) *
                        ( use_erf( tmp * one_over_sqrt2_a2 - emg_erf_2 ) + sign_a3 ) +
                        gmg_coeff * exp( gmg_exp_m1 * tmp * tmp ) *
                        ( 1e0 + use_erf( gmg_erf_m1 * tmp ) );
                  }
               }
            }
         }
      }
      
      if ( use_errors )
      {
         result /= errors[ errors_index[ (unsigned int) t ] ];
      }
      
      return result;
   }

   void list_params()
   {
      US_Vector::printvector( "init_params ", init_params  );
      US_Vector::printvector( "fixed_params", fixed_params );
      US_Vector::printvector( "param_pos   ", param_pos    );
      US_Vector::printvector( "param_fixed ", param_fixed  );
      US_Vector::printvector( "param_min   ", param_min    );
      US_Vector::printvector( "param_max   ", param_max    );
      for ( map < unsigned int, unsigned int >::iterator it = comm_backref.begin();
            it != comm_backref.end();
            ++it )
      {
         us_qdebug( QString( "backref pos %1 to %2\n" ).arg( it->first ).arg( it->second ) );
      }
   }
};

bool US_Hydrodyn_Mals_Saxs_Fit::setup_run()
{
   // us_qdebug( "setup_run()" );
   MSFIT::init_params .clear( );
   MSFIT::base_params .clear( );
   MSFIT::fixed_params.clear( );
   MSFIT::comm_backref.clear( );
   MSFIT::param_pos   .clear( );
   MSFIT::param_fixed .clear( );
   MSFIT::param_min   .clear( );
   MSFIT::param_max   .clear( );

   map < unsigned int, bool > fixed_curves;

   MSFIT::conc_test        = false;
   MSFIT::conc_ratios_map .clear( );

   switch ( gaussian_type )
   {
   case US_Hydrodyn_Mals_Saxs::EMGGMG :
      MSFIT::compute_gaussian_f = &MSFIT::compute_gaussian_f_EMGGMG;
      gsm_f = &gsm_f_EMGGMG;
      MSFIT::conc_test   = cb_conc_test->isChecked();
      break;
   case US_Hydrodyn_Mals_Saxs::EMG :
      MSFIT::compute_gaussian_f = &MSFIT::compute_gaussian_f_EMG;
      gsm_f = &gsm_f_EMG;
      break;
   case US_Hydrodyn_Mals_Saxs::GMG :
      MSFIT::compute_gaussian_f = &MSFIT::compute_gaussian_f_GMG;
      gsm_f = &gsm_f_GMG;
      break;
   case US_Hydrodyn_Mals_Saxs::GAUSS :
      MSFIT::compute_gaussian_f = &MSFIT::compute_gaussian_f_GAUSS;
      gsm_f = &gsm_f_GAUSS;
      break;
   }

   MSFIT::use_errors = use_errors;

   //    QStringList qsl = (le_fix_curves->text().split( "," , Qt::SkipEmptyParts ) );

   //    for ( unsigned int i = 0; i < ( unsigned int ) qsl.size(); i++ )
   //    {
   //       fixed_curves[ qsl[ i ].toUInt() ] = true;
   //    }

   bool any_not_fixed = false;

   for ( unsigned int i = 0; i < ( unsigned int ) cb_fix_curves.size(); i++ )
   {
      if ( cb_fix_curves[ i ]->isChecked() )
      {
         fixed_curves[ i + 1 ] = true;
      } else {
         any_not_fixed = true;
      }
   }

   if ( !any_not_fixed )
   {
      return false;
   }

   double dist = ( ( US_Hydrodyn * )(mals_saxs_win->us_hydrodyn) )->gparams[ "mals_saxs_dist_max" ].toDouble();

   double base_val;

   unsigned int comm_dist_1_var_pos = 0;
   unsigned int comm_dist_2_var_pos = 0;
   unsigned int conc_width_var_pos  = 0;

   for ( unsigned int i = 0; i < ( unsigned int ) mals_saxs_win->gaussians.size(); i+= gaussian_type_size )
   {
      unsigned int pos = i / gaussian_type_size;

      if ( cb_fix_amplitude->isChecked() ||
           fixed_curves.count( pos + 1 ) )
      {
         MSFIT::param_pos   .push_back( MSFIT::fixed_params.size() );
         MSFIT::fixed_params.push_back( mals_saxs_win->gaussians[ 0 + i ] );
         MSFIT::param_fixed .push_back( true );
      } else {
         MSFIT::param_pos   .push_back( MSFIT::init_params.size() );

         if ( cb_pct_amplitude_from_init->isChecked() )
         {
            base_val = gaussians_undo[ 0 ][ 0 + i ];
         } else {
            base_val = mals_saxs_win->gaussians[ 0 + i ];
         }            

         // us_qdebug( QString( "in setup_run, pos %1 base_val is %2" ).arg( pos ).arg( base_val ) );

         MSFIT::init_params .push_back( base_val );
         MSFIT::base_params .push_back( mals_saxs_win->gaussians[ 0 + i ] );
         MSFIT::param_fixed .push_back( false );

         double ofs;
         double min = mals_saxs_ampl_width_min;
         double max = mals_saxs_win->gauss_max_height;
         // qDebug() << "mals_saxs_fit:: gauss_max_height " << max;
         if ( cb_pct_amplitude->isChecked() )
         {
            ofs = base_val * le_pct_amplitude->text().toDouble() / 100.0;
            min = base_val - ofs;
            max = base_val + ofs;
         }
         if ( min < mals_saxs_ampl_width_min )
         {
            min = mals_saxs_ampl_width_min;
         }
         if ( max > mals_saxs_win->gauss_max_height )
         {
            max = mals_saxs_win->gauss_max_height;
         }
         if ( min > max )
         {
            min = max * 1e-15;
         }

         if ( MSFIT::init_params.back() < min ) {
            // us_qdebug( QString( "min set amp init_params.size() %1 val %2" ).arg( MSFIT::init_params.size() ).arg( MSFIT::init_params.back() ) );
            MSFIT::init_params.back() = min;
         }
         if ( MSFIT::init_params.back() > max ) {
            MSFIT::init_params.back() = max;
         }

         MSFIT::param_min   .push_back( min );
         MSFIT::param_max   .push_back( max );
      }

      if ( cb_fix_center->isChecked() ||
           fixed_curves.count( pos + 1 ) )
      {
         MSFIT::param_pos   .push_back( MSFIT::fixed_params.size() );
         MSFIT::fixed_params.push_back( mals_saxs_win->gaussians[ 1 + i ] );
         MSFIT::param_fixed .push_back( true );
      } else {
         MSFIT::param_pos   .push_back( MSFIT::init_params.size() );

         if ( cb_pct_center_from_init->isChecked() )
         {
            base_val = gaussians_undo[ 0 ][ 1 + i ];
         } else {
            base_val = mals_saxs_win->gaussians[ 1 + i ];
         }            

         MSFIT::init_params .push_back( base_val );
         MSFIT::base_params .push_back( mals_saxs_win->gaussians[ 1 + i ] );
         MSFIT::param_fixed .push_back( false );

         double ofs;
         double min = -1e99;
         double max = 1e99;
         if ( cb_pct_center->isChecked() )
         {
            ofs = base_val * le_pct_center->text().toDouble() / 100.0;
            min = base_val - ofs;
            max = base_val + ofs;
         }

         if ( MSFIT::init_params.back() < min ) {
            MSFIT::init_params.back() = min;
         }
         if ( MSFIT::init_params.back() > max ) {
            MSFIT::init_params.back() = max;
         }

         MSFIT::param_min   .push_back( min );
         MSFIT::param_max   .push_back( max );
      }

      if ( cb_fix_width->isChecked() ||
           fixed_curves.count( pos + 1 ) ||
           ( cb_conc_test->isChecked() && i ) // for the conc_test, the widths of gaussians 1 will be fixed ratios of the 1st param
           )
      {
         MSFIT::param_pos   .push_back( MSFIT::fixed_params.size() );
         if ( i && cb_conc_test->isChecked() && !fixed_curves.count( pos + 1 ) && !fixed_curves.count( 1 ) )
         {
            MSFIT::fixed_params.push_back( mals_saxs_win->gaussians[ 2 + 0 ] );
            MSFIT::comm_backref   [ MSFIT::param_fixed.size() ] = conc_width_var_pos;
            MSFIT::conc_ratios_map[ MSFIT::param_fixed.size() ] = conc_ratios[ pos - 1 ];
         } else {
            MSFIT::fixed_params.push_back( mals_saxs_win->gaussians[ 2 + i ] );
         }
         MSFIT::param_fixed .push_back( true );
      } else {
         conc_width_var_pos = MSFIT::init_params.size();
         MSFIT::param_pos   .push_back( MSFIT::init_params.size() );

         if ( cb_pct_width_from_init->isChecked() )
         {
            base_val = gaussians_undo[ 0 ][ 2 + i ];
         } else {
            base_val = mals_saxs_win->gaussians[ 2 + i ];
         }            

         MSFIT::init_params .push_back( base_val );
         MSFIT::base_params .push_back( mals_saxs_win->gaussians[ 2 + i ] );
         MSFIT::param_fixed .push_back( false );

         double ofs;
         double min = mals_saxs_ampl_width_min;
         double max = 1e99;
         if ( cb_pct_width->isChecked() )
         {
            ofs = base_val * le_pct_width->text().toDouble() / 100.0;
            min = base_val - ofs;
            max = base_val + ofs;
         }
         if ( min <  mals_saxs_ampl_width_min )
         {
            min = mals_saxs_ampl_width_min;
         }
         if ( min > max )
         {
            min = max * 1e-15;
         }

         if ( MSFIT::init_params.back() < min ) {
            MSFIT::init_params.back() = min;
         }
         if ( MSFIT::init_params.back() > max ) {
            MSFIT::init_params.back() = max;
         }

         MSFIT::param_min   .push_back( min );
         MSFIT::param_max   .push_back( max );
      }

      if ( dist1_active )
      {
         if ( cb_fix_dist1->isChecked() ||
              fixed_curves.count( pos + 1 ) ||
              ( cb_comm_dist1->isChecked() && i ) )
         {
            MSFIT::param_pos   .push_back( MSFIT::fixed_params.size() );
            if ( i && cb_comm_dist1->isChecked() && !fixed_curves.count( pos + 1 ) && !fixed_curves.count( 1 ) )
            {
               MSFIT::fixed_params.push_back( mals_saxs_win->gaussians[ 3 + 0 ] );
               MSFIT::comm_backref[ MSFIT::param_fixed.size() ] = comm_dist_1_var_pos;
            } else {
               MSFIT::fixed_params.push_back( mals_saxs_win->gaussians[ 3 + i ] );
            }
            MSFIT::param_fixed .push_back( true );
         } else {
            if ( cb_comm_dist1->isChecked() )
            {
               comm_dist_1_var_pos = MSFIT::init_params.size();
            }
            MSFIT::param_pos   .push_back( MSFIT::init_params.size() );

            if ( cb_pct_dist1_from_init->isChecked() )
            {
               base_val = gaussians_undo[ 0 ][ 3 + i ];
            } else {
               base_val = mals_saxs_win->gaussians[ 3 + i ];
            }            

            MSFIT::init_params .push_back( base_val );
            MSFIT::base_params .push_back( mals_saxs_win->gaussians[ 3 + i ] );
            MSFIT::param_fixed .push_back( false );

            double ofs;
            double min = -dist;
            double max = dist;
            if ( cb_pct_dist1->isChecked() )
            {
               ofs = base_val * le_pct_dist1->text().toDouble() / 100.0;
               min = base_val - ofs;
               max = base_val + ofs;
            }
            if ( min < -dist )
            {
               min = -dist;
            }
            if ( max > dist )
            {
               max = dist;
            }
            if ( max < min )
            {
               double avg = 5e-1 * ( max + min );
               min = avg - 1e-1;
               max = avg + 1e-1;
            }
            
            if ( MSFIT::init_params.back() < min ) {
               MSFIT::init_params.back() = min;
            }
            if ( MSFIT::init_params.back() > max ) {
               MSFIT::init_params.back() = max;
            }

            MSFIT::param_min   .push_back( min );
            MSFIT::param_max   .push_back( max );
         }

         if ( dist2_active )
         {
            if ( cb_fix_dist2->isChecked() ||
                 fixed_curves.count( pos + 1 ) ||
                 ( cb_comm_dist2->isChecked() && i ) )
            {
               MSFIT::param_pos   .push_back( MSFIT::fixed_params.size() );
               if ( i && cb_comm_dist2->isChecked() && !fixed_curves.count( pos + 1 ) && !fixed_curves.count( 1 ) )
               {
                  MSFIT::fixed_params.push_back( mals_saxs_win->gaussians[ 4 + 0 ] );
                  MSFIT::comm_backref[ MSFIT::param_fixed.size() ] = comm_dist_2_var_pos;
               } else {
                  MSFIT::fixed_params.push_back( mals_saxs_win->gaussians[ 4 + i ] );
               }
               MSFIT::fixed_params.push_back( mals_saxs_win->gaussians[ 4 + i ] );
               MSFIT::param_fixed .push_back( true );
            } else {
               if ( cb_comm_dist2->isChecked() )
               {
                  comm_dist_2_var_pos = MSFIT::init_params.size();
               }
               MSFIT::param_pos   .push_back( MSFIT::init_params.size() );

               if ( cb_pct_dist2_from_init->isChecked() )
               {
                  base_val = gaussians_undo[ 0 ][ 4 + i ];
               } else {
                  base_val = mals_saxs_win->gaussians[ 4 + i ];
               }            

               MSFIT::init_params .push_back( base_val );
               MSFIT::base_params .push_back( mals_saxs_win->gaussians[ 4 + i ] );
               MSFIT::param_fixed .push_back( false );

               double ofs;
               double min = -dist;
               double max = dist;
               if ( cb_pct_dist2->isChecked() )
               {
                  ofs = base_val * le_pct_dist2->text().toDouble() / 100.0;
                  min = base_val - ofs;
                  max = base_val + ofs;
               }
               if ( min < -dist )
               {
                  min = -dist;
               }
               if ( max > dist )
               {
                  max = dist;
               }
               if ( max < min )
               {
                  double avg = 5e-1 * ( max + min );
                  min = avg - 1e-1;
                  max = avg + 1e-1;
               }

               if ( MSFIT::init_params.back() < min ) {
                  MSFIT::init_params.back() = min;
               }
               if ( MSFIT::init_params.back() > max ) {
                  MSFIT::init_params.back() = max;
               }

               MSFIT::param_min   .push_back( min );
               MSFIT::param_max   .push_back( max );
            }
         }
      }
   }

#if defined( UHHSF_FULL_DEBUG ) || defined( UHHSF_PARAMS_DEBUG )
   MSFIT::list_params();
#endif

   if ( !MSFIT::init_params.size() )
   {
      return false;
   } else {
      return true;
   }
}

bool US_Hydrodyn_Mals_Saxs_Fit::lock_zeros( vector < double > & par )
{
   // find all gaussians with zero amplitudes and/or weights and set "fixed"
   // return true if changes are made

   // if amplitude & width are fixed, nothing to do
   // us_qdebug( "lock_zeros() start" );

   if ( !mals_saxs_lock_min_retry ) {
      // us_qdebug( "lock_zeros() not enabled" );
      return false;
   }

   if ( cb_fix_amplitude->isChecked() &&
        cb_fix_width->isChecked() ) {
      // us_qdebug( "lock_zeros() fixed width & amplitude checked, skipping" );
      return false;
   }

   // make a temporary copy to check more easily
   vector < double > tmp_gaussians = mals_saxs_win->gaussians;

   for ( unsigned int i = 0; i < MSFIT::param_fixed.size(); i++ )
   {
      if ( !MSFIT::param_fixed[ i ] ||
           MSFIT::comm_backref.count( i ) )
      {
         tmp_gaussians[ i ] = par[ 
                                  MSFIT::comm_backref.count( i ) ?
                                  MSFIT::comm_backref[ i ] :
                                  MSFIT::param_pos[ i ] 
                                   ];
         if ( MSFIT::conc_ratios_map.count( i ) )
         {
            tmp_gaussians[ i ] *= MSFIT::conc_ratios_map[ i ];
         }
      }
   }
   
   set < unsigned int > to_lock;
   
   for ( unsigned int i = 0; i < ( unsigned int ) tmp_gaussians.size(); i+= gaussian_type_size )
   {
      unsigned int pos = i / gaussian_type_size;

      if ( !cb_fix_curves[ pos ]->isChecked() ) {

         if ( !cb_fix_amplitude->isChecked() ) {
            if ( tmp_gaussians[ 0 + i ] <= mals_saxs_ampl_width_min * mals_saxs_lock_min_retry_mult ) {
               to_lock.insert( pos + 1 );
               continue;
            }
         }

         if ( !cb_fix_width->isChecked() ) {
            if ( tmp_gaussians[ 2 + i ] <= mals_saxs_ampl_width_min * mals_saxs_lock_min_retry_mult ) {
               to_lock.insert( pos + 1 );
               continue;
            }
         }
      }
   }

   if ( !to_lock.size() ) {
      // us_qdebug( "lock_zeros() nothing to lock" );
      return false;
   }

   // us_qdebug( "check lock" );
   for ( set < unsigned int >::iterator it = to_lock.begin();
         it != to_lock.end();
         ++it ) {
      us_qdebug( QString( "new lock %1" ).arg( *it ) );
      cb_fix_curves[ *it - 1 ]->setChecked( true );
   }
   // us_qdebug( "done check lock" );
   // us_qdebug( "lock_zeros() end" );
   return true;
}

bool US_Hydrodyn_Mals_Saxs_Fit::max_free_peak_delta( vector < double > & par ) {
   // us_qdebug( "max_free_peak_delta_zeros() start" );

   if ( !mals_saxs_maxfpk_restart ) {
      // us_qdebug( "max_free_peak_delta() not enabled" );
      return false;
   }

   // if amplitude is fixed, skip
   if ( cb_fix_amplitude->isChecked() ) {
      // us_qdebug( "max_free_peak_delta() fixed amplitude checked, skipping" );
      return false;
   }

   // make a temporary copy to check more easily
   vector < double > tmp_gaussians = mals_saxs_win->gaussians;

   for ( unsigned int i = 0; i < MSFIT::param_fixed.size(); i++ )
   {
      if ( !MSFIT::param_fixed[ i ] ||
           MSFIT::comm_backref.count( i ) )
      {
         tmp_gaussians[ i ] = par[ 
                                  MSFIT::comm_backref.count( i ) ?
                                  MSFIT::comm_backref[ i ] :
                                  MSFIT::param_pos[ i ] 
                                   ];
         if ( MSFIT::conc_ratios_map.count( i ) )
         {
            tmp_gaussians[ i ] *= MSFIT::conc_ratios_map[ i ];
         }
      }
   }
   
   bool         any_found = false;
   // unsigned int max_pos   = 0;
   unsigned int max_i     = 0;
   double       max_val   = 0e0;

   for ( unsigned int i = 0; i < ( unsigned int ) tmp_gaussians.size(); i+= gaussian_type_size )
   {
      unsigned int pos = i / gaussian_type_size;

      if ( !cb_fix_curves[ pos ]->isChecked() ) {
         if ( !cb_fix_amplitude->isChecked() ) {
            
            if ( !any_found || tmp_gaussians[ 0 + i ] > max_val ) {
               any_found = true;
               max_val = tmp_gaussians[ 0 + i ];
               // max_pos = pos;
               max_i   = i;
               continue;
            }
         }
      }         
   }

   if ( !any_found ) {
      return false;
   }

   // us_qdebug( QString( "max_free_peak_delta() found max peak pos %1" ).arg( max_pos + 1 ) );

   tmp_gaussians[ 0 + max_i ] *= 1e0 + mals_saxs_maxfpk_restart_pct * 0.01;
   // gaussians_undo.push_back( mals_saxs_win->gaussians );
   mals_saxs_win->gaussians = tmp_gaussians;
   if ( update_mals_saxs )
   {
      mals_saxs_win->gauss_init_markers();
      mals_saxs_win->gauss_init_gaussians();
      mals_saxs_win->update_gauss_pos();
   }
   return true;
}      

void US_Hydrodyn_Mals_Saxs_Fit::lm( bool max_free_peak_delta_run, double prev_rmsd )
{
   update_common();
   setup_run();
   // puts( "lm" );
   // cout << "gauss fit start\n";

   LM::lm_control_struct control = LM::lm_control_double;
   control.printflags = 0; // 3; // monitor status (+1) and parameters (+2)
   control.epsilon    = le_epsilon   ->text().toDouble();
   control.stepbound  = le_iterations->text().toInt();
   control.maxcall    = le_population->text().toInt();

   LM::lm_status_struct status;

   vector < double > x = mals_saxs_win->f_qs[ mals_saxs_win->wheel_file ];
   vector < double > t;
   vector < double > y;
   MSFIT::errors        = mals_saxs_win->f_errors[ mals_saxs_win->wheel_file ];
   MSFIT::errors_index  .clear( );

   double start = mals_saxs_win->le_gauss_fit_start->text().toDouble();
   double end   = mals_saxs_win->le_gauss_fit_end  ->text().toDouble();

   // US_Vector::printvector( QString( "lm %1 q" ).arg( mals_saxs_win->wheel_file ), x );
   // US_Vector::printvector( QString( "lm %1 I" ).arg( mals_saxs_win->wheel_file ), mals_saxs_win->f_Is[ mals_saxs_win->wheel_file ] );
   // US_Vector::printvector( QString( "lm %1 e" ).arg( mals_saxs_win->wheel_file ), mals_saxs_win->f_errors[ mals_saxs_win->wheel_file ] );

   for ( unsigned int j = 0; j < x.size(); j++ )
   {
      if ( x[ j ] >= start && x[ j ] <= end )
      {
         t.push_back( x[ j ] );
         y.push_back( mals_saxs_win->f_Is[ mals_saxs_win->wheel_file ][ j ] );
      }
   }

   if ( use_errors )
   {
      MSFIT::errors_index.resize( ( unsigned int ) t.back() + 1 );
      for ( unsigned int i = 0; i <= ( unsigned int ) t.back(); i++ )
      {
         MSFIT::errors_index[ i ] = 0;
      }
      for ( unsigned int i = 0; i < y.size(); i++ )
      {
         y[ i ] /= MSFIT::errors[ i ];
         MSFIT::errors_index[ ( unsigned int )t[ i ] ] = i;
      }
   }

   vector < double >    org_params = MSFIT::init_params;
   double org_rmsd = 0e0;

   if ( use_errors ) 
   {
      vector < double >    yp( x.size() );

      for ( unsigned int j = 0; j < t.size(); j++ )
      {
         yp[ j ]  = (*MSFIT::compute_gaussian_f)( t[ j ], (double *)(&MSFIT::base_params[ 0 ] ) );
         org_rmsd += ( y[ j ] - yp[ j ] ) * ( y[ j ] - yp[ j ] );
      }
#if defined( UHHSF_FULL_DEBUG )
      US_Vector::printvector3( "sd y, yp, e", y, yp, MSFIT::errors );
#endif
      double nu = (double)( t.size() - (int) mals_saxs_win->gaussians.size() - 1 );
      if ( nu <= 0e0 )
      {
         nu = 1e0;
      }
      org_rmsd /= nu;
      // org_rmsd = sqrt( org_rmsd );
   } else {
      vector < double >    yp( x.size() );

      for ( unsigned int j = 0; j < t.size(); j++ )
      {
         yp[ j ]  = (*MSFIT::compute_gaussian_f)( t[ j ], (double *)(&MSFIT::base_params[ 0 ] ) );
         org_rmsd += ( y[ j ] - yp[ j ] ) * ( y[ j ] - yp[ j ] );
      }
      org_rmsd = sqrt( org_rmsd );
#if defined( UHHSF_FULL_DEBUG )
      US_Vector::printvector3( "no sd y, yp, t", y, yp, t );
#endif
   } 

   vector < double > par = MSFIT::init_params;
   // US_Vector::printvector( QString( "par start (rmsd %1)" ).arg( org_rmsd ), par );
   // us_qdebug( QString( "par start (rmsd %1)\n" ).arg( org_rmsd ) );

   // LM::qpb  = ( QProgressBar * )0;
   // LM::qApp = ( QApplication * )0;

#if defined( UHHSF_FULL_DEBUG )
   US_Vector::printvector2( "start lmcurvefit t, y", t, y );
#endif

   LM::lmcurve_fit_rmsd( ( int )      par.size(),
                         ( double * ) &( par[ 0 ] ),
                         ( int )      t.size(),
                         ( double * ) &( t[ 0 ] ),
                         ( double * ) &( y[ 0 ] ),
                         MSFIT::compute_gaussian_f,
                         (const LM::lm_control_struct *)&control,
                         &status );
   

   // recompute fnorm for sd values

   if ( use_errors ) 
   {
      double new_fnorm = 0e0;
      vector < double >    yp( x.size() );
      for ( unsigned int j = 0; j < t.size(); j++ )
      {
         yp[ j ]  = (*MSFIT::compute_gaussian_f)( t[ j ], (double *)(&par[ 0 ] ) );
         new_fnorm += ( y[ j ] - yp[ j ] ) * ( y[ j ] - yp[ j ] );
      }
      // US_Vector::printvector3( "sd y, yp, e", y, yp, MSFIT::errors );
      double nu = (double)( t.size() - (int) mals_saxs_win->gaussians.size() - 1 );
      if ( nu <= 0e0 )
      {
         nu = 1e0;
      }
      new_fnorm /= nu;
      // us_qdebug( QString( "recomputing fnorm for sd's ... should match display field %1" ).arg( new_fnorm ) );
      // org_rmsd = sqrt( org_rmsd );
      status.fnorm = new_fnorm;
   }

   if ( status.fnorm < 0e0 )
   {
      status.fnorm = 1e99;
      cout << "WARNING: lm() returned negative rmsd\n";
   }

#if defined( UHHSF_FULL_DEBUG )
   US_Vector::printvector( QString( "par after fit (norm %1)" ).arg( status.fnorm ), par );
#endif
   // us_qdebug(QString( "par fit (rmsd %1)\n" ).arg( status.fnorm ) );

   double best_rmsd = org_rmsd;

   if ( max_free_peak_delta_run ) {
      // us_qdebug( QString( "max free peak delta run results org_rmsd %1 prev_rmsd %2 status.fnorm %3" )
      //         .arg( org_rmsd )
      //         .arg( prev_rmsd )
      //         .arg( status.fnorm )
      //         );
      if ( org_rmsd > status.fnorm &&
           prev_rmsd > status.fnorm ) {
         // us_qdebug( "max free peak delta run improvement found" );
         for ( unsigned int i = 0; i < MSFIT::param_fixed.size(); i++ )
         {
            if ( !MSFIT::param_fixed[ i ] ||
                 MSFIT::comm_backref.count( i ) )
            {
               mals_saxs_win->gaussians[ i ] = par[ 
                                              MSFIT::comm_backref.count( i ) ?
                                              MSFIT::comm_backref[ i ] :
                                              MSFIT::param_pos[ i ] 
                                               ];
               if ( MSFIT::conc_ratios_map.count( i ) )
               {
                  mals_saxs_win->gaussians[ i ] *= MSFIT::conc_ratios_map[ i ];
               }
            }
         }
         gaussians_undo.push_back( mals_saxs_win->gaussians );
      } else {
         // we need to pop back the old ones
         // us_qdebug( "max free peak delta run worse then before" );
         // us_qdebug( "undoing old gaussians" );
         return undo();
      }
   } else {
      if ( org_rmsd > status.fnorm )
      {
         best_rmsd = status.fnorm;
         // us_qdebug( "improvement found" );
         for ( unsigned int i = 0; i < MSFIT::param_fixed.size(); i++ )
         {
            if ( !MSFIT::param_fixed[ i ] ||
                 MSFIT::comm_backref.count( i ) )
            {
               mals_saxs_win->gaussians[ i ] = par[ 
                                              MSFIT::comm_backref.count( i ) ?
                                              MSFIT::comm_backref[ i ] :
                                              MSFIT::param_pos[ i ] 
                                               ];
               if ( MSFIT::conc_ratios_map.count( i ) )
               {
                  mals_saxs_win->gaussians[ i ] *= MSFIT::conc_ratios_map[ i ];
               }
            }
         }
         gaussians_undo.push_back( mals_saxs_win->gaussians );
         // check for further improvement if more can be done
         if ( lock_zeros( par ) ) {
            update_enables();
            if ( pb_lm->isEnabled() ) {
               // us_qdebug( "retrying with more fixed curves" );
               // pb_lm->animateClick();
               return lm( max_free_peak_delta_run );
            } else {
               us_qdebug( "all curves locked" );
            }
         }
      } else {
         cout << "no improvement, reverting to original values\n";
         if ( lock_zeros( par ) ) {
            update_enables();
            if ( pb_lm->isEnabled() ) {
               // us_qdebug( "retrying with more fixed curves" );
               // pb_lm->animateClick();
               return lm( max_free_peak_delta_run );
            } else {
               us_qdebug( "all curves locked" );
            }
         }
      }

      if ( mals_saxs_maxfpk_restart &&
           max_free_peak_delta( par ) ) {
         // us_qdebug( "max free peak delta retry" );
         gaussians_undo.push_back( mals_saxs_win->gaussians );
         return lm( true, best_rmsd );
      }
   }

   if ( update_mals_saxs )
   {
      mals_saxs_win->gauss_init_markers();
      mals_saxs_win->gauss_init_gaussians();
      mals_saxs_win->update_gauss_pos();
   }
   update_enables();
}

void US_Hydrodyn_Mals_Saxs_Fit::gsm_sd()
{
   puts( "gsm_sd" );
   update_common();

   gsm_setup();

   vector < double >    org_params = MSFIT::init_params;
   double org_rmsd = 0e0;
   for ( unsigned int j = 0; j < gsm_t.size(); j++ )
   {
      gsm_yp[ j ]  = (*MSFIT::compute_gaussian_f)( gsm_t[ j ], (double *)(&MSFIT::base_params[ 0 ]) );
      org_rmsd += ( gsm_y[ j ] - gsm_yp[ j ] ) * ( gsm_y[ j ] - gsm_yp[ j ] );
   }
   
   org_rmsd = sqrt( org_rmsd );

   our_vector *v = new_our_vector( MSFIT::init_params.size() );
   for ( int i = 0; i < v->len; i++ )
   {
      v->d[ i ] = MSFIT::init_params[ i ];
   }

   double cmp_rmsd = gsm_f( v );

   cout << QString( "org rmsd %1 gsm_f %2\n" ).arg( org_rmsd ).arg( cmp_rmsd );

   min_gsm_5_1     ( v,
                     le_epsilon->text().toDouble(),
                     le_iterations->text().toLong() );

   double rmsd = gsm_f( v );
   
   cout << QString( "final rmsd %1\n" ).arg( rmsd );

   if ( org_rmsd > rmsd )
   {
      for ( unsigned int i = 0; i < MSFIT::param_fixed.size(); i++ )
      {
         if ( !MSFIT::param_fixed[ i ] ||
              MSFIT::comm_backref.count( i ) )
         {
            mals_saxs_win->gaussians[ i ] = v->d[ 
                                            MSFIT::comm_backref.count( i ) ?
                                            MSFIT::comm_backref[ i ] :
                                            MSFIT::param_pos[ i ] 
                                             ];
            if ( MSFIT::conc_ratios_map.count( i ) )
            {
               mals_saxs_win->gaussians[ i ] *= MSFIT::conc_ratios_map[ i ];
            }
         }
      }
      gaussians_undo.push_back( mals_saxs_win->gaussians );
   } else {
      cout << "no improvement, reverting to original values\n";
   }
      
   free_our_vector( v );

   if ( update_mals_saxs )
   {
      mals_saxs_win->gauss_init_markers();
      mals_saxs_win->gauss_init_gaussians();
      mals_saxs_win->update_gauss_pos();
   }
   progress->reset();
   update_enables();
}

void US_Hydrodyn_Mals_Saxs_Fit::gsm_ih()
{
   puts( "gsm_ih" );
   update_common();

   gsm_setup();

   vector < double >    org_params = MSFIT::init_params;
   double org_rmsd = 0e0;
   for ( unsigned int j = 0; j < gsm_t.size(); j++ )
   {
      gsm_yp[ j ]  = (*MSFIT::compute_gaussian_f)( gsm_t[ j ], (double *)(&MSFIT::base_params[ 0 ]) );
      org_rmsd += ( gsm_y[ j ] - gsm_yp[ j ] ) * ( gsm_y[ j ] - gsm_yp[ j ] );
   }
   
   org_rmsd = sqrt( org_rmsd );

   our_vector *v = new_our_vector( MSFIT::init_params.size() );
   for ( int i = 0; i < v->len; i++ )
   {
      v->d[ i ] = MSFIT::init_params[ i ];
   }

   double cmp_rmsd = gsm_f( v );

   cout << QString( "org rmsd %1 gsm_f %2\n" ).arg( org_rmsd ).arg( cmp_rmsd );

   min_hessian_bfgs( v,
                    le_epsilon->text().toDouble(),
                    le_iterations->text().toLong() );

   double rmsd = gsm_f( v );
   
   cout << QString( "final rmsd %1\n" ).arg( rmsd );

   if ( org_rmsd > rmsd )
   {
      for ( unsigned int i = 0; i < MSFIT::param_fixed.size(); i++ )
      {
         if ( !MSFIT::param_fixed[ i ] ||
              MSFIT::comm_backref.count( i ) )
         {
            mals_saxs_win->gaussians[ i ] = v->d[ 
                                            MSFIT::comm_backref.count( i ) ?
                                            MSFIT::comm_backref[ i ] :
                                            MSFIT::param_pos[ i ] 
                                             ];
            if ( MSFIT::conc_ratios_map.count( i ) )
            {
               mals_saxs_win->gaussians[ i ] *= MSFIT::conc_ratios_map[ i ];
            }
         }
      }
      gaussians_undo.push_back( mals_saxs_win->gaussians );
   } else {
      cout << "no improvement, reverting to original values\n";
   }
      
   free_our_vector( v );

   if ( update_mals_saxs )
   {
      mals_saxs_win->gauss_init_markers();
      mals_saxs_win->gauss_init_gaussians();
      mals_saxs_win->update_gauss_pos();
   }
   progress->reset();
   update_enables();
}

void US_Hydrodyn_Mals_Saxs_Fit::gsm_cg()
{
   puts( "gsm_cg" );
   update_common();

   gsm_setup();

   vector < double >    org_params = MSFIT::init_params;
   double org_rmsd = 0e0;
   for ( unsigned int j = 0; j < gsm_t.size(); j++ )
   {
      gsm_yp[ j ]  = (*MSFIT::compute_gaussian_f)( gsm_t[ j ], (double *)(&MSFIT::base_params[ 0 ]) );
      org_rmsd += ( gsm_y[ j ] - gsm_yp[ j ] ) * ( gsm_y[ j ] - gsm_yp[ j ] );
   }
   
   org_rmsd = sqrt( org_rmsd );

   our_vector *v = new_our_vector( MSFIT::init_params.size() );
   for ( int i = 0; i < v->len; i++ )
   {
      v->d[ i ] = MSFIT::init_params[ i ];
   }

   double cmp_rmsd = gsm_f( v );

   cout << QString( "org rmsd %1 gsm_f %2\n" ).arg( org_rmsd ).arg( cmp_rmsd );

   min_fr_pr_cgd( v,
                  le_epsilon->text().toDouble(),
                  le_iterations->text().toLong() );

   double rmsd = gsm_f( v );
   
   cout << QString( "final rmsd %1\n" ).arg( rmsd );

   if ( org_rmsd > rmsd )
   {
      for ( unsigned int i = 0; i < MSFIT::param_fixed.size(); i++ )
      {
         if ( !MSFIT::param_fixed[ i ] ||
              MSFIT::comm_backref.count( i ) )
         {
            mals_saxs_win->gaussians[ i ] = v->d[ 
                                            MSFIT::comm_backref.count( i ) ?
                                            MSFIT::comm_backref[ i ] :
                                            MSFIT::param_pos[ i ] 
                                             ];
            if ( MSFIT::conc_ratios_map.count( i ) )
            {
               mals_saxs_win->gaussians[ i ] *= MSFIT::conc_ratios_map[ i ];
            }
         }
      }
      gaussians_undo.push_back( mals_saxs_win->gaussians );
   } else {
      cout << "no improvement, reverting to original values\n";
   }
      
   free_our_vector( v );

   if ( update_mals_saxs )
   {
      mals_saxs_win->gauss_init_markers();
      mals_saxs_win->gauss_init_gaussians();
      mals_saxs_win->update_gauss_pos();
   }
   progress->reset();
   update_enables();
}

#if !defined( UHSHF_FALSE_GA )
void US_Hydrodyn_Mals_Saxs_Fit::ga()
{
   puts( "ga" );
   update_common();
   gsm_setup();

   vector < double >    org_params = MSFIT::init_params;
   double org_rmsd = 0e0;
   for ( unsigned int j = 0; j < gsm_t.size(); j++ )
   {
      gsm_yp[ j ]  = (*MSFIT::compute_gaussian_f)( gsm_t[ j ], (double *)(&MSFIT::base_params[ 0 ]) );
      org_rmsd += ( gsm_y[ j ] - gsm_yp[ j ] ) * ( gsm_y[ j ] - gsm_yp[ j ] );
   }
   
   our_vector *v = new_our_vector( MSFIT::init_params.size() );
   for ( int i = 0; i < v->len; i++ )
   {
      v->d[ i ] = MSFIT::init_params[ i ];
   }

   double cmp_rmsd = gsm_f( v );

   cout << QString( "org rmsd %1 gsm_f %2\n" ).arg( org_rmsd ).arg( cmp_rmsd );

   double rmsd;
   if ( ga_run( rmsd ) &&
        rmsd < org_rmsd )
   {
      vector < double > par = MSFIT::init_params;

      cout << QString( "ga rmsd %1\n" ).arg( rmsd );
      // US_Vector::printvector( "after ga par is", par );

      for ( unsigned int i = 0; i < MSFIT::param_fixed.size(); i++ )
      {
         if ( !MSFIT::param_fixed[ i ] ||
              MSFIT::comm_backref.count( i ) )
         {
            mals_saxs_win->gaussians[ i ] = par[ 
                                           MSFIT::comm_backref.count( i ) ?
                                           MSFIT::comm_backref[ i ] :
                                           MSFIT::param_pos[ i ] 
                                            ];
            if ( MSFIT::conc_ratios_map.count( i ) )
            {
               mals_saxs_win->gaussians[ i ] *= MSFIT::conc_ratios_map[ i ];
            }
         }
      }
      gaussians_undo.push_back( mals_saxs_win->gaussians );
   } else {
      cout << "no improvement, reverting to original values\n";
   }

   if ( update_mals_saxs )
   {
      mals_saxs_win->gauss_init_markers();
      mals_saxs_win->gauss_init_gaussians();
      mals_saxs_win->update_gauss_pos();
   }
   progress->reset();
   update_enables();
}
#endif

void US_Hydrodyn_Mals_Saxs_Fit::stop()
{
   running = false;
}

void US_Hydrodyn_Mals_Saxs_Fit::grid()
{
   setup_run();
   update_common();
   puts( "grid" );
   cout << "gauss fit start\n";

   vector < double > x = mals_saxs_win->f_qs[ mals_saxs_win->wheel_file ];
   vector < double > t;
   vector < double > y;
   vector < double > yp( x.size() );

   double start = mals_saxs_win->le_gauss_fit_start->text().toDouble();
   double end   = mals_saxs_win->le_gauss_fit_end  ->text().toDouble();

   for ( unsigned int j = 0; j < x.size(); j++ )
   {
      if ( x[ j ] >= start && x[ j ] <= end )
      {
         t.push_back( x[ j ] );
         y.push_back( mals_saxs_win->f_Is[ mals_saxs_win->wheel_file ][ j ] );
      }
   }

   vector < double > par = MSFIT::init_params;

   // US_Vector::printvector( QString( "par start" ), par );

   // determine total count

   unsigned int pop_size    = le_population->text().toUInt();
   unsigned int total_count = ( unsigned int ) pow( (double) pop_size, (int) MSFIT::init_params.size() );

   cout << QString( "total points %1\n" ).arg( total_count );

   vector < double > use_par( par.size() );
   vector < double > ofs_per( par.size() );

   for ( unsigned int j = 0; j < ( unsigned int ) MSFIT::init_params.size(); j++ )
   {
      ofs_per[ j ] = ( MSFIT::param_max[ j ] - MSFIT::param_min[ j ] ) / ( pop_size - 1 );
   }
      
   double best_rmsd     = 1e99;
   vector < double >    best_params;

   vector < double >    org_params = MSFIT::init_params;
   double org_rmsd = 0e0;
   for ( unsigned int j = 0; j < t.size(); j++ )
   {
      yp[ j ]  = (*MSFIT::compute_gaussian_f)( t[ j ], (double *)(&MSFIT::base_params[ 0 ]) );
      org_rmsd += ( y[ j ] - yp[ j ] ) * ( y[ j ] - yp[ j ] );
   }
   
   org_rmsd = sqrt( org_rmsd );

   running = true;
   update_enables();

   for ( unsigned int i = 0; i < total_count; i++ )
   {
      unsigned int remainder = i;
      progress->setValue( i ); progress->setMaximum( total_count );
      qApp->processEvents();

      for ( unsigned int j = 0; j < ( unsigned int ) MSFIT::init_params.size(); j++ )
      {
         unsigned int pos = remainder % pop_size;
         use_par[ j ] = MSFIT::param_min[ j ] + pos * ofs_per[ j ];
         remainder /= pop_size;
      }

      // compute new y
      double rmsd = 0e0;

      for ( unsigned int j = 0; j < t.size(); j++ )
      {
         yp[ j ]  = (*MSFIT::compute_gaussian_f)( t[ j ], (double *)(&use_par[ 0 ]) );
         rmsd += ( y[ j ] - yp[ j ] ) * ( y[ j ] - yp[ j ] );
      }

      rmsd = sqrt( rmsd );

#if defined( UHHSF_DEBUG_PLOT )
      if ( use_par.size() == 1 )
      {
         cout << QString( "%1 %2\n" ).arg( use_par[ 0 ] ).arg( rmsd );
      }
      if ( use_par.size() == 2 )
      {
         if ( !( i % pop_size ) )
         {
            puts( "" );
         }
         cout << QString( "%1 %2 %3\n" ).arg( use_par[ 0 ] ).arg( use_par[ 1 ] ).arg( rmsd );
      }
#endif
      if ( rmsd < best_rmsd )
      {
         best_rmsd = rmsd;
         best_params = use_par;
      }
      if ( !running )
      {
         break;
      }
   } 

   running = false;

   if ( org_rmsd > best_rmsd )
   {

      par = best_params;

      // US_Vector::printvector( "after grid par is", par );

      for ( unsigned int i = 0; i < MSFIT::param_fixed.size(); i++ )
      {
         if ( !MSFIT::param_fixed[ i ] ||
              MSFIT::comm_backref.count( i ) )
         {
            mals_saxs_win->gaussians[ i ] = par[ 
                                           MSFIT::comm_backref.count( i ) ?
                                           MSFIT::comm_backref[ i ] :
                                           MSFIT::param_pos[ i ] 
                                            ];
            if ( MSFIT::conc_ratios_map.count( i ) )
            {
               mals_saxs_win->gaussians[ i ] *= MSFIT::conc_ratios_map[ i ];
            }
         }
      }
      gaussians_undo.push_back( mals_saxs_win->gaussians );
   } else {
      cout << "no improvement, reverting to original values\n";
   }

   if ( update_mals_saxs )
   {
      mals_saxs_win->gauss_init_markers();
      mals_saxs_win->gauss_init_gaussians();
      mals_saxs_win->update_gauss_pos();
   }
   progress->reset();
   update_enables();
}

#if defined( UHSHF_FALSE_GA )
void US_Hydrodyn_Mals_Saxs_Fit::ga()
{
   setup_run();
   puts( "grid" );
   cout << "gauss fit start\n";

   vector < double > x = mals_saxs_win->f_qs[ mals_saxs_win->wheel_file ];
   vector < double > t;
   vector < double > y;
   vector < double > yp( x.size() );

   double start = mals_saxs_win->le_gauss_fit_start->text().toDouble();
   double end   = mals_saxs_win->le_gauss_fit_end  ->text().toDouble();

   for ( unsigned int j = 0; j < x.size(); j++ )
   {
      if ( x[ j ] >= start && x[ j ] <= end )
      {
         t.push_back( x[ j ] );
         y.push_back( mals_saxs_win->f_Is[ mals_saxs_win->wheel_file ][ j ] );
      }
   }

   gsm_setup();

   vector < double > par = MSFIT::init_params;

   // US_Vector::printvector( QString( "par start" ), par );

   // determine total count

   unsigned int pop_size    = le_population->text().toUInt();
   unsigned int total_count = ( unsigned int ) pow( (double) pop_size, (int) MSFIT::init_params.size() );

   cout << QString( "total points %1\n" ).arg( total_count );

   vector < double > use_par( par.size() );
   vector < double > ofs_per( par.size() );

   for ( unsigned int j = 0; j < ( unsigned int ) MSFIT::init_params.size(); j++ )
   {
      ofs_per[ j ] = ( MSFIT::param_max[ j ] - MSFIT::param_min[ j ] ) / ( pop_size - 1 );
   }
      
   double best_rmsd     = 1e99;
   vector < double >    best_params;

   vector < double >    org_params = MSFIT::init_params;
   double org_rmsd = 0e0;
   for ( unsigned int j = 0; j < t.size(); j++ )
   {
      yp[ j ]  = (*MSFIT::compute_gaussian_f)( t[ j ], (double *)(&MSFIT::base_params[ 0 ]) );
      org_rmsd += ( y[ j ] - yp[ j ] ) * ( y[ j ] - yp[ j ] );
   }
   
   org_rmsd = sqrt( org_rmsd );

   running = true;
   update_enables();

   for ( unsigned int i = 0; i < total_count; i++ )
   {
      unsigned int remainder = i;
      progress->setValue( i ); progress->setMaximum( total_count );
      qApp->processEvents();

      for ( unsigned int j = 0; j < ( unsigned int ) MSFIT::init_params.size(); j++ )
      {
         unsigned int pos = remainder % pop_size;
         use_par[ j ] = MSFIT::param_min[ j ] + pos * ofs_per[ j ];
         remainder /= pop_size;
      }

      // compute new y
      double rmsd = 0e0;

      our_vector *v = new_our_vector( MSFIT::init_params.size() );
      for ( int j = 0; j < v->len; j++ )
      {
         v->d[ j ] = use_par[ j ];
      }
      rmsd  = (*gsm_f)( v );

      if ( use_par.size() == 1 )
      {
         cout << QString( "%1 %2\n" ).arg( use_par[ 0 ] ).arg( rmsd );
      }
      if ( use_par.size() == 2 )
      {
         if ( !( i % pop_size ) )
         {
            puts( "" );
         }
         cout << QString( "%1 %2 %3\n" ).arg( use_par[ 0 ] ).arg( use_par[ 1 ] ).arg( rmsd );
      }
      if ( rmsd < best_rmsd )
      {
         best_rmsd = rmsd;
         best_params = use_par;
      }
      if ( !running )
      {
         break;
      }
   } 

   running = false;

   if ( org_rmsd > best_rmsd )
   {

      par = best_params;

      // US_Vector::printvector( "after grid par is", par );

      for ( unsigned int i = 0; i < MSFIT::param_fixed.size(); i++ )
      {
         if ( !MSFIT::param_fixed[ i ] ||
              MSFIT::comm_backref.count( i ) )
         {
            mals_saxs_win->gaussians[ i ] = par[ 
                                           MSFIT::comm_backref.count( i ) ?
                                           MSFIT::comm_backref[ i ] :
                                           MSFIT::param_pos[ i ] 
                                            ];
            if ( MSFIT::conc_ratios_map.count( i ) )
            {
               mals_saxs_win->gaussians[ i ] *= MSFIT::conc_ratios_map[ i ];
            }
         }
      }
      gaussians_undo.push_back( mals_saxs_win->gaussians );
   } else {
      cout << "no improvement, reverting to original values\n";
   }

   if ( update_mals_saxs )
   {
      mals_saxs_win->gauss_init_markers();
      mals_saxs_win->gauss_init_gaussians();
      mals_saxs_win->update_gauss_pos();
   }
   progress->reset();
   update_enables();
}
#endif
