#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_saxs_hplc.h"
#include "../include/us_hydrodyn_saxs_hplc_fit_global.h"
#include "../include/us_lm.h"
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <QLabel>
#include <Q3GridLayout>
#include <Q3Frame>
#include <Q3VBoxLayout>
#include <QCloseEvent>
// #include <assert.h>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

US_Hydrodyn_Saxs_Hplc_Fit_Global::US_Hydrodyn_Saxs_Hplc_Fit_Global(
                                                                   US_Hydrodyn_Saxs_Hplc *hplc_win,
                                                                   QWidget *p, 
                                                                   const char *name
                                                                   ) : QDialog(p, name)
{
   this->hplc_win = hplc_win;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setCaption( tr( "US-SOMO: SAXS Hplc: Global Gaussian Fit" ) );

   update_hplc = true;
   running = false;

   gaussian_type      = hplc_win->gaussian_type;
   gaussian_type_size = hplc_win->gaussian_type_size;

   switch ( gaussian_type )
   {
   case US_Hydrodyn_Saxs_Hplc::EMGGMG :
      dist1_active = true;
      dist2_active = true;
      break;
   case US_Hydrodyn_Saxs_Hplc::EMG :
   case US_Hydrodyn_Saxs_Hplc::GMG :
      dist1_active = true;
      dist2_active = false;
      break;
   case US_Hydrodyn_Saxs_Hplc::GAUSS :
      dist1_active = false;
      dist2_active = false;
      break;
   }

   common_size   = 0;
   per_file_size = 0;

   is_common.clear(); // is common maps the offsets to layout of the regular file specific gaussians
   offset.clear();

   // height:
   is_common.push_back( false           );  // height always variable
   offset   .push_back( per_file_size++ );  // first variable entry

   // center
   is_common.push_back( true            );  // center always common
   offset   .push_back( common_size++   );  // first common entry

   // width
   if ( hplc_win->cb_fix_width->isChecked() )
   {
      is_common.push_back( true );
      offset   .push_back( common_size++   );  // first common entry
   } else {
      is_common.push_back( false );
      offset   .push_back( per_file_size++ );  // first variable entry
   }

   if ( dist1_active )
   {
      if ( hplc_win->cb_fix_dist1->isChecked() )
      {
         is_common.push_back( true );
         offset   .push_back( common_size++   );  // first common entry
      } else {
         is_common.push_back( false );
         offset   .push_back( per_file_size++ );  // first variable entry
      }
      if ( dist2_active )
      {
         if ( hplc_win->cb_fix_dist2->isChecked() )
         {
            is_common.push_back( true );
            offset   .push_back( common_size++   );  // first common entry
         } else {
            is_common.push_back( false );
            offset   .push_back( per_file_size++ );  // first variable entry
         }
      }
   }

   cout << QString( "hfg:common size %1 per file per gaussian size %2\n" ).arg( common_size ).arg( per_file_size );

   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;

   gaussians_undo.clear();
   gaussians_undo.push_back( hplc_win->unified_ggaussian_params );

   setGeometry(global_Xpos, global_Ypos, 0, 0 );

   use_errors = hplc_win->cb_sd_weight->isChecked();
   if ( use_errors &&
        ( hplc_win->unified_ggaussian_e.size() != hplc_win->unified_ggaussian_q.size() ||
          !hplc_win->is_nonzero_vector ( hplc_win->unified_ggaussian_e ) ) )
   {
      use_errors = false;
      QMessageBox::information( this, this->caption(),
                                tr( "SD weighting requested, but the errors associated\n"
                                    "with the selected files are not all non-zero.\n" 
                                    "SD weighting turned off"
                                    ) );

   }
   if ( use_errors )
   {
      // check t vector for all non-negative integers
      vector < double > x = hplc_win->unified_ggaussian_t;
      for ( unsigned i = 0; i < x.size(); i++ )
      {
         unsigned int xui = ( unsigned int ) fabs( x[ i ] );
         if ( ( double ) xui != x[ i ] )
         {
            use_errors = false;
            QMessageBox::information( this, this->caption(),
                                      tr( "SD weighting requested, but this currently\n"
                                          "does not support fractional or negative frame numbers.\n"
                                          "SD weighting turned off."
                                          ) );
            break;
         }
      }
   }

   update_enables();
}

US_Hydrodyn_Saxs_Hplc_Fit_Global::~US_Hydrodyn_Saxs_Hplc_Fit_Global()
{
}

void US_Hydrodyn_Saxs_Hplc_Fit_Global::restore()
{
   gaussians_undo.resize( 1 );
   hplc_win->unified_ggaussian_params = gaussians_undo[ 0 ];
   if ( update_hplc )
   {
      hplc_win->gauss_init_markers();
      hplc_win->update_gauss_pos();
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc_Fit_Global::undo()
{
   if ( gaussians_undo.size() > 1 )
   {
      gaussians_undo.pop_back();
   }

   hplc_win->unified_ggaussian_params = gaussians_undo.back();
   if ( update_hplc )
   {
      hplc_win->gauss_init_markers();
      hplc_win->update_gauss_pos();
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc_Fit_Global::setupGUI()
{
   int minHeight1 = 30;

   lbl_title = new QLabel( tr( "US-SOMO: SAXS Hplc: Global Gaussian Fit" ), this);
   lbl_title->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette( PALET_FRAME );
   AUTFBACK( lbl_title );
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize , QFont::Bold));

   cb_fix_center = new QCheckBox(this);
   cb_fix_center->setText(tr(" Fix Gaussian centers" ) );
   cb_fix_center->setEnabled(true);
   cb_fix_center->setChecked( false );
   cb_fix_center->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_fix_center->setPalette( PALET_NORMAL );
   AUTFBACK( cb_fix_center );
   connect(cb_fix_center, SIGNAL( clicked() ), SLOT( update_enables() ) );

   cb_pct_center = new QCheckBox(this);
   cb_pct_center->setText(tr(" % variation" ) );
   cb_pct_center->setEnabled(true);
   cb_pct_center->setChecked( false );
   cb_pct_center->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pct_center->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pct_center );
   connect(cb_pct_center, SIGNAL( clicked() ), SLOT( update_enables() ) );

   le_pct_center = new mQLineEdit(this, "le_pct_center Line Edit");
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
   cb_pct_center_from_init->setText(tr(" From initial value" ) );
   cb_pct_center_from_init->setEnabled(true);
   cb_pct_center_from_init->setChecked( false );
   cb_pct_center_from_init->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pct_center_from_init->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pct_center_from_init );
   connect(cb_pct_center_from_init, SIGNAL( clicked() ), SLOT( update_enables() ) );

   cb_fix_width = new QCheckBox(this);
   cb_fix_width->setText(tr(" Fix Gaussian widths" ) );
   cb_fix_width->setEnabled(true);
   cb_fix_width->setChecked( false );
   cb_fix_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_fix_width->setPalette( PALET_NORMAL );
   AUTFBACK( cb_fix_width );
   connect(cb_fix_width, SIGNAL( clicked() ), SLOT( update_enables() ) );

   cb_pct_width = new QCheckBox(this);
   cb_pct_width->setText(tr(" % variation" ) );
   cb_pct_width->setEnabled(true);
   cb_pct_width->setChecked( false );
   cb_pct_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pct_width->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pct_width );
   connect(cb_pct_width, SIGNAL( clicked() ), SLOT( update_enables() ) );

   le_pct_width = new mQLineEdit(this, "le_pct_width Line Edit");
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
   cb_pct_width_from_init->setText(tr(" From initial value" ) );
   cb_pct_width_from_init->setEnabled(true);
   cb_pct_width_from_init->setChecked( false );
   cb_pct_width_from_init->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pct_width_from_init->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pct_width_from_init );
   connect(cb_pct_width_from_init, SIGNAL( clicked() ), SLOT( update_enables() ) );

   cb_fix_amplitude = new QCheckBox(this);
   cb_fix_amplitude->setText(tr(" Fix Gaussian amplitudes" ) );
   cb_fix_amplitude->setEnabled(true);
   cb_fix_amplitude->setChecked( false );
   cb_fix_amplitude->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_fix_amplitude->setPalette( PALET_NORMAL );
   AUTFBACK( cb_fix_amplitude );
   connect(cb_fix_amplitude, SIGNAL( clicked() ), SLOT( update_enables() ) );

   cb_pct_amplitude = new QCheckBox(this);
   cb_pct_amplitude->setText(tr(" % variation" ) );
   cb_pct_amplitude->setEnabled(true);
   cb_pct_amplitude->setChecked( false );
   cb_pct_amplitude->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pct_amplitude->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pct_amplitude );
   connect(cb_pct_amplitude, SIGNAL( clicked() ), SLOT( update_enables() ) );

   le_pct_amplitude = new mQLineEdit(this, "le_pct_amplitude Line Edit");
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
   cb_pct_amplitude_from_init->setText(tr(" From initial value" ) );
   cb_pct_amplitude_from_init->setEnabled(true);
   cb_pct_amplitude_from_init->setChecked( false );
   cb_pct_amplitude_from_init->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pct_amplitude_from_init->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pct_amplitude_from_init );
   connect(cb_pct_amplitude_from_init, SIGNAL( clicked() ), SLOT( update_enables() ) );

   cb_fix_dist1 = new QCheckBox(this);
   cb_fix_dist1->setText(tr(" Fix distortion 1" ) );
   cb_fix_dist1->setEnabled(true);
   cb_fix_dist1->setChecked( false );
   cb_fix_dist1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_fix_dist1->setPalette( PALET_NORMAL );
   AUTFBACK( cb_fix_dist1 );
   connect(cb_fix_dist1, SIGNAL( clicked() ), SLOT( update_enables() ) );

   cb_pct_dist1 = new QCheckBox(this);
   cb_pct_dist1->setText(tr(" % variation" ) );
   cb_pct_dist1->setEnabled(true);
   cb_pct_dist1->setChecked( false );
   cb_pct_dist1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pct_dist1->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pct_dist1 );
   connect(cb_pct_dist1, SIGNAL( clicked() ), SLOT( update_enables() ) );

   le_pct_dist1 = new mQLineEdit(this, "le_pct_dist1 Line Edit");
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
   cb_pct_dist1_from_init->setText(tr(" From initial value" ) );
   cb_pct_dist1_from_init->setEnabled(true);
   cb_pct_dist1_from_init->setChecked( false );
   cb_pct_dist1_from_init->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pct_dist1_from_init->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pct_dist1_from_init );
   connect(cb_pct_dist1_from_init, SIGNAL( clicked() ), SLOT( update_enables() ) );

   cb_fix_dist2 = new QCheckBox(this);
   cb_fix_dist2->setText(tr(" Fix distortion 2" ) );
   cb_fix_dist2->setEnabled(true);
   cb_fix_dist2->setChecked( false );
   cb_fix_dist2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_fix_dist2->setPalette( PALET_NORMAL );
   AUTFBACK( cb_fix_dist2 );
   connect(cb_fix_dist2, SIGNAL( clicked() ), SLOT( update_enables() ) );

   cb_pct_dist2 = new QCheckBox(this);
   cb_pct_dist2->setText(tr(" % variation" ) );
   cb_pct_dist2->setEnabled(true);
   cb_pct_dist2->setChecked( false );
   cb_pct_dist2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pct_dist2->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pct_dist2 );
   connect(cb_pct_dist2, SIGNAL( clicked() ), SLOT( update_enables() ) );

   le_pct_dist2 = new mQLineEdit(this, "le_pct_dist2 Line Edit");
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
   cb_pct_dist2_from_init->setText(tr(" From initial value" ) );
   cb_pct_dist2_from_init->setEnabled(true);
   cb_pct_dist2_from_init->setChecked( false );
   cb_pct_dist2_from_init->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pct_dist2_from_init->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pct_dist2_from_init );
   connect(cb_pct_dist2_from_init, SIGNAL( clicked() ), SLOT( update_enables() ) );

   lbl_fix_curves = new QLabel(tr(" Fix Gaussians: "), this);
   lbl_fix_curves->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   // lbl_fix_curves->setMinimumHeight(minHeight1);
   lbl_fix_curves->setPalette( PALET_LABEL );
   AUTFBACK( lbl_fix_curves );
   lbl_fix_curves->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));


   /* old way
      QString qs_rx = QString( "^(|(\\d+)|(\\d+(,\\d+){0,%1}))$" ).arg( hplc_win->gaussians.size() / 3 - 1 );
      cout << "qs_rx:" << qs_rx << endl;
      QRegExp rx_fix_curves( qs_rx );
      QRegExpValidator *rx_val_fix_curves = new QRegExpValidator( rx_fix_curves, this );
      le_fix_curves = new mQLineEdit(this, "le_fix_curves Line Edit");
      le_fix_curves->setText( "" );
      le_fix_curves->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
      le_fix_curves->setPalette( PALET_NORMAL );
      AUTFBACK( le_fix_curves );
      le_fix_curves->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
      le_fix_curves->setEnabled( false );
      le_fix_curves->setValidator( rx_val_fix_curves );
      connect( le_fix_curves, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   */

   for ( unsigned int i = 0; i < ( unsigned int ) hplc_win->gaussians.size() / gaussian_type_size; i++ )
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

   lbl_epsilon = new QLabel(tr(" Epsilon: "), this);
   lbl_epsilon->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   // lbl_epsilon->setMinimumHeight(minHeight1);
   lbl_epsilon->setPalette( PALET_LABEL );
   AUTFBACK( lbl_epsilon );
   lbl_epsilon->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_epsilon = new mQLineEdit(this, "le_epsilon Line Edit");
   double peak = hplc_win->compute_gaussian_peak( hplc_win->wheel_file, hplc_win->gaussians );
   le_epsilon->setText( QString( "%1" ).arg( peak / 1e6 < 0.001 ? peak / 1e6 : 0.001 ) );
   le_epsilon->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_epsilon->setPalette( PALET_NORMAL );
   AUTFBACK( le_epsilon );
   le_epsilon->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   le_epsilon->setEnabled( false );
   le_epsilon->setValidator( new QDoubleValidator( le_epsilon ) );
   ( (QDoubleValidator *)le_epsilon->validator() )->setRange( 0e-3, 10, 3 );

   lbl_iterations = new QLabel(tr(" Iterations: "), this);
   lbl_iterations->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   // lbl_iterations->setMinimumHeight(minHeight1);
   lbl_iterations->setPalette( PALET_LABEL );
   AUTFBACK( lbl_iterations );
   lbl_iterations->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_iterations = new mQLineEdit(this, "le_iterations Line Edit");
   le_iterations->setText( "100" );
   le_iterations->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_iterations->setPalette( PALET_NORMAL );
   AUTFBACK( le_iterations );
   le_iterations->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   le_iterations->setEnabled( false );
   le_iterations->setValidator( new QIntValidator( le_iterations ) );
   ( (QIntValidator *)le_iterations->validator() )->setRange( 1, 10000 );

   lbl_population = new QLabel(tr(" Maximum calls/Population/Grid: "), this);
   lbl_population->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   // lbl_population->setMinimumHeight(minHeight1);
   lbl_population->setPalette( PALET_LABEL );
   AUTFBACK( lbl_population );
   lbl_population->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_population = new mQLineEdit(this, "le_population Line Edit");
   le_population->setText( "100" );
   le_population->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_population->setPalette( PALET_NORMAL );
   AUTFBACK( le_population );
   le_population->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   le_population->setEnabled( false );
   le_population->setValidator( new QIntValidator( le_population ) );
   ( (QIntValidator *)le_population->validator() )->setRange( 2, 10000 );

   pb_restore = new QPushButton(tr("Restore to initial values"), this);
   pb_restore->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_restore->setMinimumHeight(minHeight1);
   pb_restore->setPalette( PALET_PUSHB );
   connect(pb_restore, SIGNAL(clicked()), SLOT(restore()));

   pb_undo = new QPushButton(tr("Undo"), this);
   pb_undo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_undo->setMinimumHeight(minHeight1);
   pb_undo->setPalette( PALET_PUSHB );
   connect(pb_undo, SIGNAL(clicked()), SLOT(undo()));

   pb_lm = new QPushButton(tr("LM"), this);
   pb_lm->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_lm->setMinimumHeight(minHeight1);
   pb_lm->setPalette( PALET_PUSHB );
   connect(pb_lm, SIGNAL(clicked()), SLOT(lm()));

   pb_gsm_sd = new QPushButton(tr("GS SD"), this);
   pb_gsm_sd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_gsm_sd->setMinimumHeight(minHeight1);
   pb_gsm_sd->setPalette( PALET_PUSHB );
   connect(pb_gsm_sd, SIGNAL(clicked()), SLOT(gsm_sd()));

   pb_gsm_ih = new QPushButton(tr("GS IH"), this);
   pb_gsm_ih->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_gsm_ih->setMinimumHeight(minHeight1);
   pb_gsm_ih->setPalette( PALET_PUSHB );
   connect(pb_gsm_ih, SIGNAL(clicked()), SLOT(gsm_ih()));

   pb_gsm_cg = new QPushButton(tr("GS CG"), this);
   pb_gsm_cg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_gsm_cg->setMinimumHeight(minHeight1);
   pb_gsm_cg->setPalette( PALET_PUSHB );
   connect(pb_gsm_cg, SIGNAL(clicked()), SLOT(gsm_cg()));

   pb_ga = new QPushButton(tr("GA"), this);
   pb_ga->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_ga->setMinimumHeight(minHeight1);
   pb_ga->setPalette( PALET_PUSHB );
   connect(pb_ga, SIGNAL(clicked()), SLOT(ga()));

   pb_grid = new QPushButton(tr("Grid"), this);
   pb_grid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_grid->setMinimumHeight(minHeight1);
   pb_grid->setPalette( PALET_PUSHB );
   connect(pb_grid, SIGNAL(clicked()), SLOT(grid()));

   progress = new Q3ProgressBar(this, "Progress");
   // progress->setMinimumHeight(minHeight1);
   progress->setPalette( PALET_NORMAL );
   AUTFBACK( progress );
   progress->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize  ));
   progress->reset();

   pb_stop = new QPushButton(tr("Stop"), this);
   pb_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_stop->setMinimumHeight(minHeight1);
   pb_stop->setPalette( PALET_PUSHB );
   connect(pb_stop, SIGNAL(clicked()), SLOT(stop()));

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));


   // build layout
   Q3GridLayout *gl_main = new Q3GridLayout( 0 );
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

   gl_main->addWidget         ( lbl_fix_curves, row, 0 );
   // gl_main->addMultiCellWidget( le_fix_curves , row, row, 1, 3 );
   Q3HBoxLayout *hbl_fix_curves = new Q3HBoxLayout;
   for ( unsigned int i = 0; i < ( unsigned int ) cb_fix_curves.size(); i++ )
   {
      hbl_fix_curves->addWidget( cb_fix_curves[ i ] );
   }
   gl_main->addMultiCellLayout( hbl_fix_curves , row, row, 1, 3 );

   row++;

   gl_main->addWidget         ( lbl_epsilon, row, 0 );
   gl_main->addMultiCellWidget( le_epsilon , row, row, 1, 3 );
   row++;

   gl_main->addWidget         ( lbl_iterations, row, 0 );
   gl_main->addMultiCellWidget( le_iterations , row, row, 1, 3 );
   row++;

   gl_main->addWidget         ( lbl_population, row, 0 );
   gl_main->addMultiCellWidget( le_population , row, row, 1, 3 );
   row++;

   Q3HBoxLayout *hbl_restore_undo = new Q3HBoxLayout;
   hbl_restore_undo->addWidget ( pb_restore );
   hbl_restore_undo->addWidget ( pb_undo );

   Q3HBoxLayout *hbl_runs = new Q3HBoxLayout;
   hbl_runs->addWidget ( pb_lm );
   hbl_runs->addWidget ( pb_gsm_sd );
   hbl_runs->addWidget ( pb_gsm_ih );
   hbl_runs->addWidget ( pb_gsm_cg );
   hbl_runs->addWidget ( pb_ga );
   hbl_runs->addWidget ( pb_grid );

   Q3HBoxLayout *hbl_prog = new Q3HBoxLayout;
   hbl_prog->addWidget ( progress );
   hbl_prog->addWidget ( pb_stop );

   Q3HBoxLayout *hbl_bottom = new Q3HBoxLayout;
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addWidget ( pb_cancel );

   Q3VBoxLayout *background = new Q3VBoxLayout( this );
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

   void * us_hydrodyn = hplc_win->us_hydrodyn;

   if ( !U_EXPT )
   {
      pb_gsm_sd->hide();
      pb_gsm_ih->hide();
      pb_gsm_cg->hide();
      pb_ga    ->hide();
      pb_grid  ->hide();
      lbl_population->setText( tr( "Maximum calls" ) );
   }

   if ( !dist1_active )
   {
      cb_fix_dist1->hide();
      cb_pct_dist1->hide();
      le_pct_dist1->hide();
      cb_pct_dist1_from_init->hide();
   }      
   if ( !dist2_active )
   {
      cb_fix_dist2->hide();
      cb_pct_dist2->hide();
      le_pct_dist2->hide();
      cb_pct_dist2->hide();
      cb_pct_dist2_from_init->hide();
   }      
}

void US_Hydrodyn_Saxs_Hplc_Fit_Global::cancel()
{
   close();
}

void US_Hydrodyn_Saxs_Hplc_Fit_Global::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_saxs_hplc_fit_global.html");
}

void US_Hydrodyn_Saxs_Hplc_Fit_Global::closeEvent(QCloseEvent *e)
{
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Saxs_Hplc_Fit_Global::update_enables()
{
   bool run_ok = setup_run();

   cb_fix_center                ->setEnabled( !running );
   cb_pct_center                ->setEnabled( !running && !cb_fix_center->isChecked() );
   le_pct_center                ->setEnabled( !running && !cb_fix_center->isChecked() && cb_pct_center->isChecked() );
   cb_pct_center_from_init      ->setEnabled( !running && !cb_fix_center->isChecked() );

   cb_fix_width                 ->setEnabled( !running );
   cb_pct_width                 ->setEnabled( !running && !cb_fix_width->isChecked() );
   le_pct_width                 ->setEnabled( !running && !cb_fix_width->isChecked()  && cb_pct_width->isChecked() );
   cb_pct_width_from_init       ->setEnabled( !running && !cb_fix_width->isChecked() );

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
      cb_fix_curves[ i ]      ->setEnabled( !running );
   }

   // le_fix_curves            ->setEnabled( !running );
   le_epsilon               ->setEnabled( !running );
   le_iterations            ->setEnabled( !running );
   le_population            ->setEnabled( !running );

   // bool variations_set      = 
   //    ( cb_fix_center   ->isChecked() || cb_pct_center   ->isChecked() ) &&
   //    ( cb_fix_width    ->isChecked() || cb_pct_width    ->isChecked() ) &&
   //    ( cb_fix_amplitude->isChecked() || cb_pct_amplitude->isChecked() ) &&
   //    ( !dist1_active || cb_fix_dist1->isChecked() || cb_pct_dist1->isChecked() ) &&
   //    ( !dist2_active || cb_fix_dist2->isChecked() || cb_pct_dist2->isChecked() )
   //    ;

   pb_restore               ->setEnabled( !running && gaussians_undo.size() > 1 );
   pb_undo                  ->setEnabled( !running && gaussians_undo.size() > 1 );

   pb_lm                    ->setEnabled( !running && run_ok );
   pb_gsm_sd                ->setEnabled( false && !running && run_ok && !use_errors );
   pb_gsm_ih                ->setEnabled( false && !running && run_ok && !use_errors );
   pb_gsm_cg                ->setEnabled( false && !running && run_ok && !use_errors );
   pb_ga                    ->setEnabled( false ); // && !running && run_ok && variations_set && !use_errors );
   pb_grid                  ->setEnabled( false ); // && !running && run_ok && variations_set && !use_errors );

   pb_stop                  ->setEnabled( false /* running */ );
}

namespace HFIT_GLOBAL 
{
   vector < double       > init_params;    // variable param initial values
   vector < double       > base_params;    // variable param initial values
   vector < double       > last_params;    // variable param initial values

   vector < double       > fixed_params;   // the fixed params
   vector < unsigned int > param_pos;      // index into fixed params or variable params
   vector < bool         > param_fixed;    
   vector < double       > param_min;      // minimum values for variable params
   vector < double       > param_max;      // maximum values for variable params

   vector < double       > unified_q;                        // copy from hplc win
   vector < unsigned int > unified_ggaussian_param_index;    // copy from hplc win
   unsigned int            unified_ggaussian_gaussians_size; // copy from hplc win
   unsigned int            unified_ggaussian_curves;         // copy from hplc win

   unsigned int            per_file_size;
   unsigned int            common_size;
   vector < bool >         is_common;

   bool                    use_errors;

   vector < double       > errors;
   vector < unsigned int > errors_index;

   double (*compute_gaussian_f)( double, const double * );

   double compute_gaussian_f_GAUSS( double t, const double *par )
   {
      double result = 0e0;
      double height;
      double center;
      double width;
      unsigned int index = unified_ggaussian_param_index[ ( unsigned int ) t ];
      unsigned int var_base;
      unsigned int fix_base;

      // for each gaussian

      for ( unsigned int i = 0; i < unified_ggaussian_gaussians_size; i++ )
      {
         var_base = index + per_file_size * i;
         fix_base = common_size * i;

         // height is always variable
         if ( param_fixed[ var_base ] )
         {
            height = fixed_params[ param_pos[ var_base ] ];
         } else {
            height = par         [ param_pos[ var_base ] ];
            if ( height < param_min[ param_pos[ var_base ] ] ||
                 height > param_max[ param_pos[ var_base ] ] )
            {
               return 1e99;
            }
         }
         var_base++;

         // center is always common
         if ( param_fixed[ fix_base ] )
         {
            center = fixed_params[ param_pos[ fix_base ] ];
         } else {
            center = par         [ param_pos[ fix_base ] ];
            if ( center < param_min[ param_pos[ fix_base ] ] ||
                 center > param_max[ param_pos[ fix_base ] ] )
            {
               return 1e99;
            }
         }
         fix_base++;

         // width
         if ( is_common[ 2 ] )
         {
            if ( param_fixed[ fix_base ] )
            {
               width = fixed_params[ param_pos[ fix_base ] ];
            } else {
               width = par         [ param_pos[ fix_base ] ];
               if ( width < param_min[ param_pos[ fix_base ] ] ||
                    width > param_max[ param_pos[ fix_base ] ] )
               {
                  return 1e99;
               }
            }
            // fix_base++;
         } else {
            if ( param_fixed[ var_base ] )
            {
               width = fixed_params[ param_pos[ var_base ] ];
            } else {
               width = par         [ param_pos[ var_base ] ];
               if ( width < param_min[ param_pos[ var_base ] ] ||
                    width > param_max[ param_pos[ var_base ] ] )
               {
                  return 1e99;
               }
            }
            // var_base++;
         }


         /*
         cout << QString( "for pos %1 t is %2 index %3 gaussian %4 center %5 height %6 width %7\n" )
            .arg( t )
            .arg( unified_q[ ( unsigned int ) t ] )
            .arg( index )
            .arg( i )
            .arg( center )
            .arg( height )
            .arg( width )
            ;
         */

         double tmp = ( unified_q[ ( unsigned int ) t ] - center ) / width;
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
      unsigned int index = unified_ggaussian_param_index[ ( unsigned int ) t ];
      unsigned int var_base;
      unsigned int fix_base;

      // for each gaussian

      for ( unsigned int i = 0; i < unified_ggaussian_gaussians_size; i++ )
      {
         var_base = index + per_file_size * i;
         fix_base = common_size * i;

         // height is always variable
         if ( param_fixed[ var_base ] )
         {
            height = fixed_params[ param_pos[ var_base ] ];
         } else {
            height = par         [ param_pos[ var_base ] ];
            if ( height < param_min[ param_pos[ var_base ] ] ||
                 height > param_max[ param_pos[ var_base ] ] )
            {
               return 1e99;
            }
         }
         var_base++;

         // center is always common
         if ( param_fixed[ fix_base ] )
         {
            center = fixed_params[ param_pos[ fix_base ] ];
         } else {
            center = par         [ param_pos[ fix_base ] ];
            if ( center < param_min[ param_pos[ fix_base ] ] ||
                 center > param_max[ param_pos[ fix_base ] ] )
            {
               return 1e99;
            }
         }
         fix_base++;

         // width
         if ( is_common[ 2 ] )
         {
            if ( param_fixed[ fix_base ] )
            {
               width = fixed_params[ param_pos[ fix_base ] ];
            } else {
               width = par         [ param_pos[ fix_base ] ];
               if ( width < param_min[ param_pos[ fix_base ] ] ||
                    width > param_max[ param_pos[ fix_base ] ] )
               {
                  return 1e99;
               }
            }
            fix_base++;
         } else {
            if ( param_fixed[ var_base ] )
            {
               width = fixed_params[ param_pos[ var_base ] ];
            } else {
               width = par         [ param_pos[ var_base ] ];
               if ( width < param_min[ param_pos[ var_base ] ] ||
                    width > param_max[ param_pos[ var_base ] ] )
               {
                  return 1e99;
               }
            }
            var_base++;
         }

         // dist1
         if ( is_common[ 3 ] )
         {
            if ( param_fixed[ fix_base ] )
            {
               dist1 = fixed_params[ param_pos[ fix_base ] ];
            } else {
               dist1 = par         [ param_pos[ fix_base ] ];
               if ( dist1 < param_min[ param_pos[ fix_base ] ] ||
                    dist1 > param_max[ param_pos[ fix_base ] ] )
               {
                  return 1e99;
               }
            }
            // fix_base++;
         } else {
            if ( param_fixed[ var_base ] )
            {
               dist1 = fixed_params[ param_pos[ var_base ] ];
            } else {
               dist1 = par         [ param_pos[ var_base ] ];
               if ( dist1 < param_min[ param_pos[ var_base ] ] ||
                    dist1 > param_max[ param_pos[ var_base ] ] )
               {
                  return 1e99;
               }
            }
            // var_base++;
         }

         /*
         cout << QString( "for pos %1 t is %2 index %3 gaussian %4 center %5 height %6 width %7 dist1 %8\n" )
            .arg( t )
            .arg( unified_q[ ( unsigned int ) t ] )
            .arg( index )
            .arg( i )
            .arg( center )
            .arg( height )
            .arg( width )
            .arg( dist1 )
         */
                     ;
         if ( !dist1 )
         {
            double tmp = ( unified_q[ ( unsigned int ) t ] - center ) / width;
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

               double tmp               = unified_q[ ( unsigned int ) t ] - center;
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
               double tmp               = unified_q[ ( unsigned int ) t ] - center;
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
      unsigned int index = unified_ggaussian_param_index[ ( unsigned int ) t ];
      unsigned int var_base;
      unsigned int fix_base;

      // for each gaussian

      for ( unsigned int i = 0; i < unified_ggaussian_gaussians_size; i++ )
      {
         var_base = index + per_file_size * i;
         fix_base = common_size * i;

         // height is always variable
         if ( param_fixed[ var_base ] )
         {
            height = fixed_params[ param_pos[ var_base ] ];
         } else {
            height = par         [ param_pos[ var_base ] ];
            if ( height < param_min[ param_pos[ var_base ] ] ||
                 height > param_max[ param_pos[ var_base ] ] )
            {
               return 1e99;
            }
         }
         var_base++;

         // center is always common
         if ( param_fixed[ fix_base ] )
         {
            center = fixed_params[ param_pos[ fix_base ] ];
         } else {
            center = par         [ param_pos[ fix_base ] ];
            if ( center < param_min[ param_pos[ fix_base ] ] ||
                 center > param_max[ param_pos[ fix_base ] ] )
            {
               return 1e99;
            }
         }
         fix_base++;

         // width
         if ( is_common[ 2 ] )
         {
            if ( param_fixed[ fix_base ] )
            {
               width = fixed_params[ param_pos[ fix_base ] ];
            } else {
               width = par         [ param_pos[ fix_base ] ];
               if ( width < param_min[ param_pos[ fix_base ] ] ||
                    width > param_max[ param_pos[ fix_base ] ] )
               {
                  return 1e99;
               }
            }
            fix_base++;
         } else {
            if ( param_fixed[ var_base ] )
            {
               width = fixed_params[ param_pos[ var_base ] ];
            } else {
               width = par         [ param_pos[ var_base ] ];
               if ( width < param_min[ param_pos[ var_base ] ] ||
                    width > param_max[ param_pos[ var_base ] ] )
               {
                  return 1e99;
               }
            }
            var_base++;
         }

         // dist1
         if ( is_common[ 3 ] )
         {
            if ( param_fixed[ fix_base ] )
            {
               dist1 = fixed_params[ param_pos[ fix_base ] ];
            } else {
               dist1 = par         [ param_pos[ fix_base ] ];
               if ( dist1 < param_min[ param_pos[ fix_base ] ] ||
                    dist1 > param_max[ param_pos[ fix_base ] ] )
               {
                  return 1e99;
               }
            }
            // fix_base++;
         } else {
            if ( param_fixed[ var_base ] )
            {
               dist1 = fixed_params[ param_pos[ var_base ] ];
            } else {
               dist1 = par         [ param_pos[ var_base ] ];
               if ( dist1 < param_min[ param_pos[ var_base ] ] ||
                    dist1 > param_max[ param_pos[ var_base ] ] )
               {
                  return 1e99;
               }
            }
            // var_base++;
         }

         /*
         cout << QString( "for pos %1 t is %2 index %3 gaussian %4 center %5 height %6 width %7 dist1 %8\n" )
            .arg( t )
            .arg( unified_q[ ( unsigned int ) t ] )
            .arg( index )
            .arg( i )
            .arg( center )
            .arg( height )
            .arg( width )
            .arg( dist1 )
                     ;
         */

         if ( !dist1 )
         {
            double tmp = ( unified_q[ ( unsigned int ) t ] - center ) / width;
            result += height * exp( - tmp * tmp * 5e-1 );
         } else {
            double area                         = height * width * M_SQRT2PI;
            double one_over_width               = 1e0 / width;
            double one_over_a2sq_plus_a3sq      = 1e0 / ( width * width +  dist1 * dist1 );
            double sqrt_one_over_a2sq_plus_a3sq = sqrt( one_over_a2sq_plus_a3sq );
            double gmg_coeff                    = area * M_ONE_OVER_SQRT2PI * sqrt_one_over_a2sq_plus_a3sq;
            double gmg_exp_m1                   = -5e-1 *  one_over_a2sq_plus_a3sq;
            double gmg_erf_m1                   = dist1 * sqrt_one_over_a2sq_plus_a3sq * M_ONE_OVER_SQRT2 * one_over_width;
            double tmp = unified_q[ ( unsigned int ) t ] - center;
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
      unsigned int index = unified_ggaussian_param_index[ ( unsigned int ) t ];
      unsigned int var_base;
      unsigned int fix_base;

      // for each gaussian

      for ( unsigned int i = 0; i < unified_ggaussian_gaussians_size; i++ )
      {
         var_base = index + per_file_size * i;
         fix_base = common_size * i;

         // height is always variable
         if ( param_fixed[ var_base ] )
         {
            height = fixed_params[ param_pos[ var_base ] ];
         } else {
            height = par         [ param_pos[ var_base ] ];
            if ( height < param_min[ param_pos[ var_base ] ] ||
                 height > param_max[ param_pos[ var_base ] ] )
            {
               return 1e99;
            }
         }
         var_base++;

         // center is always common
         if ( param_fixed[ fix_base ] )
         {
            center = fixed_params[ param_pos[ fix_base ] ];
         } else {
            center = par         [ param_pos[ fix_base ] ];
            if ( center < param_min[ param_pos[ fix_base ] ] ||
                 center > param_max[ param_pos[ fix_base ] ] )
            {
               return 1e99;
            }
         }
         fix_base++;

         // width
         if ( is_common[ 2 ] )
         {
            if ( param_fixed[ fix_base ] )
            {
               width = fixed_params[ param_pos[ fix_base ] ];
            } else {
               width = par         [ param_pos[ fix_base ] ];
               if ( width < param_min[ param_pos[ fix_base ] ] ||
                    width > param_max[ param_pos[ fix_base ] ] )
               {
                  return 1e99;
               }
            }
            fix_base++;
         } else {
            if ( param_fixed[ var_base ] )
            {
               width = fixed_params[ param_pos[ var_base ] ];
            } else {
               width = par         [ param_pos[ var_base ] ];
               if ( width < param_min[ param_pos[ var_base ] ] ||
                    width > param_max[ param_pos[ var_base ] ] )
               {
                  return 1e99;
               }
            }
            var_base++;
         }

         // dist1
         if ( is_common[ 3 ] )
         {
            if ( param_fixed[ fix_base ] )
            {
               dist1 = fixed_params[ param_pos[ fix_base ] ];
            } else {
               dist1 = par         [ param_pos[ fix_base ] ];
               if ( dist1 < param_min[ param_pos[ fix_base ] ] ||
                    dist1 > param_max[ param_pos[ fix_base ] ] )
               {
                  return 1e99;
               }
            }
            fix_base++;
         } else {
            if ( param_fixed[ var_base ] )
            {
               dist1 = fixed_params[ param_pos[ var_base ] ];
            } else {
               dist1 = par         [ param_pos[ var_base ] ];
               if ( dist1 < param_min[ param_pos[ var_base ] ] ||
                    dist1 > param_max[ param_pos[ var_base ] ] )
               {
                  return 1e99;
               }
            }
            var_base++;
         }

         // dist2
         if ( is_common[ 4 ] )
         {
            if ( param_fixed[ fix_base ] )
            {
               dist2 = fixed_params[ param_pos[ fix_base ] ];
            } else {
               dist2 = par         [ param_pos[ fix_base ] ];
               if ( dist2 < param_min[ param_pos[ fix_base ] ] ||
                    dist2 > param_max[ param_pos[ fix_base ] ] )
               {
                  return 1e99;
               }
            }
            // fix_base++;
         } else {
            if ( param_fixed[ var_base ] )
            {
               dist2 = fixed_params[ param_pos[ var_base ] ];
            } else {
               dist2 = par         [ param_pos[ var_base ] ];
               if ( dist2 < param_min[ param_pos[ var_base ] ] ||
                    dist2 > param_max[ param_pos[ var_base ] ] )
               {
                  return 1e99;
               }
            }
            // var_base++;
         }

         /*
         cout << QString( "for pos %1 t is %2 index %3 gaussian %4 center %5 height %6 width %7 dist1 %8 dist2 %9\n" )
            .arg( t )
            .arg( unified_q[ ( unsigned int ) t ] )
            .arg( index )
            .arg( i )
            .arg( center )
            .arg( height )
            .arg( width )
            .arg( dist1 )
            .arg( dist2 )
            ;
         */

         if ( !dist1 && !dist2 )
         {
            // just a gaussian
            double tmp = ( unified_q[ ( unsigned int ) t ] - center ) / width;
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
               double tmp                          = unified_q[ ( unsigned int ) t ] - center;
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

                     double tmp = unified_q[ ( unsigned int ) t ] - center;
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
                     double tmp               = unified_q[ ( unsigned int ) t ] - center;
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

                  double tmp                          = unified_q[ ( unsigned int ) t ] - center;

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


   /* old way
     double compute_gaussian_f_GAUSS_old( double t, const double *par )
     {
     double result = 0e0;
     double height;
     double center;
     double width;
     unsigned int index = unified_ggaussian_param_index[ ( unsigned int ) t ];
     unsigned int base;

     for ( unsigned int i = 0; i < unified_ggaussian_gaussians_size; i++ )
     {
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

     base = index + 2 * i;

     if ( param_fixed[ base + 0 ] )
     {
     height = fixed_params[ param_pos[ base + 0 ] ];
     } else {
     height = par         [ param_pos[ base + 0 ] ];
     if ( height < param_min[ param_pos[ base + 0 ] ] ||
     height > param_max[ param_pos[ base + 0 ] ] )
     {
     return 1e99;
     }
     }

     if ( param_fixed[ base + 1 ] )
     {
     width = fixed_params[ param_pos[ base + 1 ] ];
     } else {
     width = par         [ param_pos[ base + 1 ] ];
     if ( width < param_min[ param_pos[ base + 1 ] ] ||
     width > param_max[ param_pos[ base + 1 ] ] )
     {
     return 1e99;
     }
     }

     //          cout << QString( "for pos %1 t is %2 index %3 gaussian %4 center %5 height %6 width %7\n" )
     //             .arg( t )
     //             .arg( unified_q[ ( unsigned int ) t ] )
     //             .arg( index )
     //             .arg( i )
     //             .arg( center )
     //             .arg( height )
     //             .arg( width )
     //             ;

     double tmp = ( unified_q[ ( unsigned int ) t ] - center ) / width;
     result += height * exp( - tmp * tmp * 5e-1 );
     }
      
     if ( use_errors )
     {
     result /= errors[ errors_index[ (unsigned int) t ] ];
     }

     return result;
     }

     double compute_gaussian_f_eq_width_GAUSS_old( double t, const double *par )
     {
     double result = 0e0;
     double height;
     double center;
     double width;
     unsigned int index = unified_ggaussian_param_index[ ( unsigned int ) t ];
     unsigned int base;

     for ( unsigned int i = 0; i < unified_ggaussian_gaussians_size; i++ )
     {
     if ( param_fixed[ 2 * i + 0 ] )
     {
     center = fixed_params[ param_pos[ 2 * i ] ];
     } else {
     center = par         [ param_pos[ 2 * i ] ];
     if ( center < param_min[ param_pos[ 2 * i ] ] ||
     center > param_max[ param_pos[ 2 * i ] ] )
     {
     return 1e99;
     }
     }

     if ( param_fixed[ 2 * i + 1 ] )
     {
     width = fixed_params[ param_pos[ 2 * i + 1 ] ];
     } else {
     width = par         [ param_pos[ 2 * i + 1 ] ];
     if ( width < param_min[ param_pos[ 2 * i + 1 ] ] ||
     width > param_max[ param_pos[ 2 * i + 1 ] ] )
     {
     return 1e99;
     }
     }

     base = index + i;

     if ( param_fixed[ base + 0 ] )
     {
     height = fixed_params[ param_pos[ base + 0 ] ];
     } else {
     height = par         [ param_pos[ base + 0 ] ];
     if ( height < param_min[ param_pos[ base + 0 ] ] ||
     height > param_max[ param_pos[ base + 0 ] ] )
     {
     return 1e99;
     }
     }

     //          cout << QString( "for pos %1 t is %2 index %3 gaussian %4 center %5 height %6 width %7\n" )
     //             .arg( t )
     //             .arg( unified_q[ ( unsigned int ) t ] )
     //             .arg( index )
     //             .arg( i )
     //             .arg( center )
     //             .arg( height )
     //             .arg( width )
     //             ;

     double tmp = ( unified_q[ ( unsigned int ) t ] - center ) / width;
     result += height * exp( - tmp * tmp * 5e-1 );
     }
      
     if ( use_errors )
     {
     result /= errors[ errors_index[ (unsigned int) t ] ];
     }

     return result;
     }
   */

   void list_params()
   {
      US_Vector::printvector( "init_params ", init_params  );
      US_Vector::printvector( "fixed_params", fixed_params );
      US_Vector::printvector( "param_pos   ", param_pos    );
      US_Vector::printvector( "param_fixed ", param_fixed  );
      US_Vector::printvector( "param_min   ", param_min    );
      US_Vector::printvector( "param_max   ", param_max    );
   }
};


bool US_Hydrodyn_Saxs_Hplc_Fit_Global::setup_run()
{
   HFIT_GLOBAL::init_params                       .clear();
   HFIT_GLOBAL::base_params                       .clear();
   HFIT_GLOBAL::fixed_params                      .clear();
   HFIT_GLOBAL::param_pos                         .clear();
   HFIT_GLOBAL::param_fixed                       .clear();
   HFIT_GLOBAL::param_min                         .clear();
   HFIT_GLOBAL::param_max                         .clear();
   HFIT_GLOBAL::unified_q                         = hplc_win->unified_ggaussian_q;
   HFIT_GLOBAL::unified_ggaussian_param_index     = hplc_win->unified_ggaussian_param_index;
   HFIT_GLOBAL::unified_ggaussian_gaussians_size  = hplc_win->unified_ggaussian_gaussians_size;
   HFIT_GLOBAL::unified_ggaussian_curves          = hplc_win->unified_ggaussian_curves;

   HFIT_GLOBAL::per_file_size                     = per_file_size;
   HFIT_GLOBAL::common_size                       = common_size;
   HFIT_GLOBAL::is_common                         = is_common;

   map < unsigned int, bool > fixed_curves;

   switch ( gaussian_type )
   {
   case US_Hydrodyn_Saxs_Hplc::EMGGMG :
      HFIT_GLOBAL::compute_gaussian_f = &HFIT_GLOBAL::compute_gaussian_f_EMGGMG;
      // gsm_f = &gsm_f_EMGGMG;
      break;
   case US_Hydrodyn_Saxs_Hplc::EMG :
      HFIT_GLOBAL::compute_gaussian_f = &HFIT_GLOBAL::compute_gaussian_f_EMG;
      // gsm_f = &gsm_f_EMG;
      break;
   case US_Hydrodyn_Saxs_Hplc::GMG :
      HFIT_GLOBAL::compute_gaussian_f = &HFIT_GLOBAL::compute_gaussian_f_GMG;
      // gsm_f = &gsm_f_GMG;
      break;
   case US_Hydrodyn_Saxs_Hplc::GAUSS :
      HFIT_GLOBAL::compute_gaussian_f = &HFIT_GLOBAL::compute_gaussian_f_GAUSS;
      // gsm_f = &gsm_f_GAUSS;
      break;
   }

   HFIT_GLOBAL::use_errors = use_errors;

   //    QStringList qsl = QStringList::split( ",", le_fix_curves->text() );

   //    for ( unsigned int i = 0; i < ( unsigned int ) qsl.size(); i++ )
   //    {
   //       fixed_curves[ qsl[ i ].toUInt() ] = true;
   //    }

   bool any_not_fixed = false;

   for ( unsigned int i = 0; i < ( unsigned int ) cb_fix_curves.size(); i++ )
   {
      if ( cb_fix_curves[ i ]->isChecked() )
      {
         fixed_curves[ i ] = true;
      } else {
         any_not_fixed = true;
      }
   }

   if ( !any_not_fixed )
   {
      return false;
   }

   double base_val;

   // layout of unified gaussian_params
   // common portion (shared by all): f=center{,width}{,dist1}{,dist2}
   // f1,f2,...,fn
   // variable portion (per file per gaussian): v=height{,width}{,dist1}{,dist2}
   // v1,v2,...,vn

   // layout of fitting params
   // param_pos   : the index into either fixed_params (non-changing) or init_params (variable during search)
   // fixed_params: all the "non-changing during search params"
   // init_params : all the "variable during search params" // this will be the search vector
   // param_fixed : boolean as to whether or not the specific param is fixed(true) or variable(false)
   // base_params : these are a copy of the init_params without replacement from the hplc_win unified_ggaussians
   //                used for comparing initial rmsds?

   // param_min: minimum value init_params
   // param_max: maximum value init_params

   // example:

   // say we have g gaussians and k files
   // unified ggaussians: f1,f2,..fg,file=1{v1,v2,..,vg},file=2{v1,v2,..,vg},...,file=k{v1,v2,..,vg}
   // variable access for element i of above is ( param_fixed[ i ] ? fixed_params[ param_pos[ i ] ] : init_params[ param_pos[ i ] ] )
   

   // unified_ggaussians have in "fix_width" mode first "unified_ggaussian_gaussians_size*common_size" elements as common centers and common widths
   // and the next unified_ggaussian_gaussians_size * per_file_size * unified_ggaussian_curves elements as the curve specific variables
   // the fixed_params are those that are fixed globally
   // thie 

   // the common params first, since they are not per file

   for ( unsigned int i = 0; i < hplc_win->unified_ggaussian_gaussians_size; i++ )
   {
      // centers always first, always common

      unsigned int ofs = common_size * i;

      if ( cb_fix_center->isChecked() ||
           fixed_curves.count( i ) )
      {
         HFIT_GLOBAL::param_pos   .push_back( HFIT_GLOBAL::fixed_params.size() );
         HFIT_GLOBAL::fixed_params.push_back( hplc_win->unified_ggaussian_params[ ofs ] );
         HFIT_GLOBAL::param_fixed .push_back( true );
      } else {
         HFIT_GLOBAL::param_pos   .push_back( HFIT_GLOBAL::init_params.size() );

         if ( cb_pct_center_from_init->isChecked() )
         {
            base_val = gaussians_undo[ 0 ][ ofs ];
         } else {
            base_val = hplc_win->unified_ggaussian_params[ ofs ];
         }            

         HFIT_GLOBAL::init_params .push_back( base_val );
         HFIT_GLOBAL::base_params .push_back( hplc_win->unified_ggaussian_params[ ofs ] );
         HFIT_GLOBAL::param_fixed .push_back( false );

         double ofs;
         double min = -1e99;
         double max = 1e99;
         if ( cb_pct_center->isChecked() )
         {
            ofs = base_val * le_pct_center->text().toDouble() / 100.0;
            min = base_val - ofs;
            max = base_val + ofs;
         }

         HFIT_GLOBAL::param_min   .push_back( min );
         HFIT_GLOBAL::param_max   .push_back( max );
      }
      ofs++;

      if ( hplc_win->cb_fix_width->isChecked() ) // if widths are common
      {
         if ( cb_fix_width->isChecked() ||
              fixed_curves.count( i ) )
         {
            HFIT_GLOBAL::param_pos   .push_back( HFIT_GLOBAL::fixed_params.size() );
            HFIT_GLOBAL::fixed_params.push_back( hplc_win->unified_ggaussian_params[ ofs ] );
            HFIT_GLOBAL::param_fixed .push_back( true );
         } else {
            HFIT_GLOBAL::param_pos   .push_back( HFIT_GLOBAL::init_params.size() );

            if ( cb_pct_width_from_init->isChecked() )
            {
               base_val = gaussians_undo[ 0 ][ ofs ];
            } else {
               base_val = hplc_win->unified_ggaussian_params[ ofs ];
            }            

            HFIT_GLOBAL::init_params .push_back( base_val );
            HFIT_GLOBAL::base_params .push_back( hplc_win->unified_ggaussian_params[ ofs ] );
            HFIT_GLOBAL::param_fixed .push_back( false );

            double ofs;
            double min = 1e-10;
            double max = 1e99;
            if ( cb_pct_width->isChecked() )
            {
               ofs = base_val * le_pct_width->text().toDouble() / 100.0;
               min = base_val - ofs;
               max = base_val + ofs;
            }
            if ( min < 1e-10 )
            {
               min = 1e-10;
            }
            if ( min > max )
            {
               min = max * 1e-15;
            }

            HFIT_GLOBAL::param_min   .push_back( min );
            HFIT_GLOBAL::param_max   .push_back( max );
         }
         ofs++;
      }

      if ( dist1_active && hplc_win->cb_fix_dist1->isChecked() ) // if dist1s are common
      {
         if ( cb_fix_dist1->isChecked() ||
              fixed_curves.count( i ) )
         {
            HFIT_GLOBAL::param_pos   .push_back( HFIT_GLOBAL::fixed_params.size() );
            HFIT_GLOBAL::fixed_params.push_back( hplc_win->unified_ggaussian_params[ ofs ] );
            HFIT_GLOBAL::param_fixed .push_back( true );
         } else {
            HFIT_GLOBAL::param_pos   .push_back( HFIT_GLOBAL::init_params.size() );

            if ( cb_pct_dist1_from_init->isChecked() )
            {
               base_val = gaussians_undo[ 0 ][ ofs ];
            } else {
               base_val = hplc_win->unified_ggaussian_params[ ofs ];
            }            

            HFIT_GLOBAL::init_params .push_back( base_val );
            HFIT_GLOBAL::base_params .push_back( hplc_win->unified_ggaussian_params[ ofs ] );
            HFIT_GLOBAL::param_fixed .push_back( false );

            double ofs;
            double min = -50e0;
            double max = 50e0;
            if ( cb_pct_dist1->isChecked() )
            {
               ofs = base_val * le_pct_dist1->text().toDouble() / 100.0;
               min = base_val - ofs;
               max = base_val + ofs;
            }
            if ( min < -50e0 )
            {
               min = -50e0;
            }
            if ( max > 50e0 )
            {
               max = 50e0;
            }
            if ( max < min )
            {
               double avg = 5e-1 * ( max + min );
               min = avg - 1e-1;
               max = avg + 1e-1;
            }

            HFIT_GLOBAL::param_min   .push_back( min );
            HFIT_GLOBAL::param_max   .push_back( max );
         }
         ofs++;

         if ( dist2_active && hplc_win->cb_fix_dist2->isChecked() ) // if dist2s are common
         {
            if ( cb_fix_dist2->isChecked() ||
                 fixed_curves.count( i ) )
            {
               HFIT_GLOBAL::param_pos   .push_back( HFIT_GLOBAL::fixed_params.size() );
               HFIT_GLOBAL::fixed_params.push_back( hplc_win->unified_ggaussian_params[ ofs ] );
               HFIT_GLOBAL::param_fixed .push_back( true );
            } else {
               HFIT_GLOBAL::param_pos   .push_back( HFIT_GLOBAL::init_params.size() );

               if ( cb_pct_dist2_from_init->isChecked() )
               {
                  base_val = gaussians_undo[ 0 ][ ofs ];
               } else {
                  base_val = hplc_win->unified_ggaussian_params[ ofs ];
               }            

               HFIT_GLOBAL::init_params .push_back( base_val );
               HFIT_GLOBAL::base_params .push_back( hplc_win->unified_ggaussian_params[ ofs ] );
               HFIT_GLOBAL::param_fixed .push_back( false );

               double ofs;
               double min = -50e0;
               double max = 50e0;
               if ( cb_pct_dist2->isChecked() )
               {
                  ofs = base_val * le_pct_dist2->text().toDouble() / 100.0;
                  min = base_val - ofs;
                  max = base_val + ofs;
               }
               if ( min < -50e0 )
               {
                  min = -50e0;
               }
               if ( max > 50e0 )
               {
                  max = 50e0;
               }
               if ( max < min )
               {
                  double avg = 5e-1 * ( max + min );
                  min = avg - 1e-1;
                  max = avg + 1e-1;
               }

               HFIT_GLOBAL::param_min   .push_back( min );
               HFIT_GLOBAL::param_max   .push_back( max );
            }
            ofs++;
         }
      }
   }
   // end of common portion
   // start of variable portion

   unsigned int base_ofs = hplc_win->unified_ggaussian_gaussians_size * common_size;
   // assert( (unsigned int)HFIT_GLOBAL::param_pos.size() == base_ofs && "common_size*number of gaussians should equal param_pos.size()" );

   for ( unsigned int f = 0; f < hplc_win->unified_ggaussian_curves; f++ )
   {
      unsigned int ofs = base_ofs + f * per_file_size * hplc_win->unified_ggaussian_gaussians_size;

      for ( unsigned int i = 0; i < hplc_win->unified_ggaussian_gaussians_size; i++ )
      {
         // heights are always per file

         if ( cb_fix_amplitude->isChecked() ||
              fixed_curves.count( i ) )
         {
            HFIT_GLOBAL::param_pos   .push_back( HFIT_GLOBAL::fixed_params.size() );
            HFIT_GLOBAL::fixed_params.push_back( hplc_win->unified_ggaussian_params[ ofs ] );
            HFIT_GLOBAL::param_fixed .push_back( true );
         } else {
            HFIT_GLOBAL::param_pos   .push_back( HFIT_GLOBAL::init_params.size() );

            if ( cb_pct_amplitude_from_init->isChecked() )
            {
               base_val = gaussians_undo[ 0 ][ ofs ];
            } else {
               base_val = hplc_win->unified_ggaussian_params[ ofs ];
            }            

            HFIT_GLOBAL::init_params .push_back( base_val );
            HFIT_GLOBAL::base_params .push_back( hplc_win->unified_ggaussian_params[ ofs ] );
            HFIT_GLOBAL::param_fixed .push_back( false );

            double ofs;
            double min = 1e-10;
            double max = hplc_win->gauss_max_height;
            if ( cb_pct_amplitude->isChecked() )
            {
               ofs = base_val * le_pct_amplitude->text().toDouble() / 100.0;
               min = base_val - ofs;
               max = base_val + ofs;
            }
            if ( min < 1e-10 )
            {
               min = 1e-10;
            }
            if ( max > hplc_win->gauss_max_height )
            {
               max = hplc_win->gauss_max_height;
            }
            if ( min > max )
            {
               min = max * 1e-15;
            }

            HFIT_GLOBAL::param_min   .push_back( min );
            HFIT_GLOBAL::param_max   .push_back( max );
         }
         ofs++;

         if ( !hplc_win->cb_fix_width->isChecked() ) // if widths are not common
         {
            if ( cb_fix_width->isChecked() ||
                 fixed_curves.count( i ) )
            {
               HFIT_GLOBAL::param_pos   .push_back( HFIT_GLOBAL::fixed_params.size() );
               HFIT_GLOBAL::fixed_params.push_back( hplc_win->unified_ggaussian_params[ ofs ] );
               HFIT_GLOBAL::param_fixed .push_back( true );
            } else {
               HFIT_GLOBAL::param_pos   .push_back( HFIT_GLOBAL::init_params.size() );

               if ( cb_pct_width_from_init->isChecked() )
               {
                  base_val = gaussians_undo[ 0 ][ ofs ];
               } else {
                  base_val = hplc_win->unified_ggaussian_params[ ofs ];
               }            

               HFIT_GLOBAL::init_params .push_back( base_val );
               HFIT_GLOBAL::base_params .push_back( hplc_win->unified_ggaussian_params[ ofs ] );
               HFIT_GLOBAL::param_fixed .push_back( false );

               double ofs;
               double min = 1e-10;
               double max = 1e99;
               if ( cb_pct_width->isChecked() )
               {
                  ofs = base_val * le_pct_width->text().toDouble() / 100.0;
                  min = base_val - ofs;
                  max = base_val + ofs;
               }
               if ( min < 1e-10 )
               {
                  min = 1e-10;
               }
               if ( min > max )
               {
                  min = max * 1e-15;
               }

               HFIT_GLOBAL::param_min   .push_back( min );
               HFIT_GLOBAL::param_max   .push_back( max );
            }
            ofs++;
         }

         if ( dist1_active && !hplc_win->cb_fix_dist1->isChecked() ) // if dist1s are not common
         {
            if ( cb_fix_dist1->isChecked() ||
                 fixed_curves.count( i ) )
            {
               HFIT_GLOBAL::param_pos   .push_back( HFIT_GLOBAL::fixed_params.size() );
               HFIT_GLOBAL::fixed_params.push_back( hplc_win->unified_ggaussian_params[ ofs ] );
               HFIT_GLOBAL::param_fixed .push_back( true );
            } else {
               HFIT_GLOBAL::param_pos   .push_back( HFIT_GLOBAL::init_params.size() );

               if ( cb_pct_dist1_from_init->isChecked() )
               {
                  base_val = gaussians_undo[ 0 ][ ofs ];
               } else {
                  base_val = hplc_win->unified_ggaussian_params[ ofs ];
               }            

               HFIT_GLOBAL::init_params .push_back( base_val );
               HFIT_GLOBAL::base_params .push_back( hplc_win->unified_ggaussian_params[ ofs ] );
               HFIT_GLOBAL::param_fixed .push_back( false );

               double ofs;
               double min = -50e0;
               double max = 50e0;
               if ( cb_pct_dist1->isChecked() )
               {
                  ofs = base_val * le_pct_dist1->text().toDouble() / 100.0;
                  min = base_val - ofs;
                  max = base_val + ofs;
               }
               if ( max < min )
               {
                  double avg = 5e-1 * ( max + min );
                  min = avg - 1e-1;
                  max = avg + 1e-1;
               }

               HFIT_GLOBAL::param_min   .push_back( min );
               HFIT_GLOBAL::param_max   .push_back( max );
            }
            ofs++;
            if ( dist2_active && !hplc_win->cb_fix_dist2->isChecked() ) // if dist2s are common
            {
               if ( cb_fix_dist2->isChecked() ||
                    fixed_curves.count( i ) )
               {
                  HFIT_GLOBAL::param_pos   .push_back( HFIT_GLOBAL::fixed_params.size() );
                  HFIT_GLOBAL::fixed_params.push_back( hplc_win->unified_ggaussian_params[ ofs ] );
                  HFIT_GLOBAL::param_fixed .push_back( true );
               } else {
                  HFIT_GLOBAL::param_pos   .push_back( HFIT_GLOBAL::init_params.size() );

                  if ( cb_pct_dist2_from_init->isChecked() )
                  {
                     base_val = gaussians_undo[ 0 ][ ofs ];
                  } else {
                     base_val = hplc_win->unified_ggaussian_params[ ofs ];
                  }            

                  HFIT_GLOBAL::init_params .push_back( base_val );
                  HFIT_GLOBAL::base_params .push_back( hplc_win->unified_ggaussian_params[ ofs ] );
                  HFIT_GLOBAL::param_fixed .push_back( false );

                  double ofs;
                  double min = -50e0;
                  double max = 50e0;
                  if ( cb_pct_dist2->isChecked() )
                  {
                     ofs = base_val * le_pct_dist2->text().toDouble() / 100.0;
                     min = base_val - ofs;
                     max = base_val + ofs;
                  }
                  if ( min < -50e0 )
                  {
                     min = -50e0;
                  }
                  if ( max > 50e0 )
                  {
                     max = 50e0;
                  }
                  if ( max < min )
                  {
                     double avg = 5e-1 * ( max + min );
                     min = avg - 1e-1;
                     max = avg + 1e-1;
                  }

                  HFIT_GLOBAL::param_min   .push_back( min );
                  HFIT_GLOBAL::param_max   .push_back( max );
               }
               ofs++;
            }
         }
      } // each gaussian i
   } // each file f
   

   /* old way

   if ( hplc_win->cb_fix_width->isChecked() )
   {
      // unified_ggaussians have in "fix_width" mode first "unified_ggaussian_gaussians_size*2" elements as common centers and common widths
      // and the next unified_ggaussian_gaussians_size * 1 * unified_ggaussian_curves elements as the curve specific
      // heights

      // the centers & widths first:

      for ( unsigned int i = 0; i < hplc_win->unified_ggaussian_gaussians_size; i++ )
      {
         if ( cb_fix_center->isChecked() ||
              fixed_curves.count( i ) )
         {
            HFIT_GLOBAL::param_pos   .push_back( HFIT_GLOBAL::fixed_params.size() );
            HFIT_GLOBAL::fixed_params.push_back( hplc_win->unified_ggaussian_params[ 2 * i + 0 ] );
            HFIT_GLOBAL::param_fixed .push_back( true );
         } else {
            HFIT_GLOBAL::param_pos   .push_back( HFIT_GLOBAL::init_params.size() );

            if ( cb_pct_center_from_init->isChecked() )
            {
               base_val = gaussians_undo[ 0 ][ 2 * i + 0 ];
            } else {
               base_val = hplc_win->unified_ggaussian_params[ 2 * i + 0 ];
            }            

            HFIT_GLOBAL::init_params .push_back( base_val );
            HFIT_GLOBAL::base_params .push_back( hplc_win->unified_ggaussian_params[ 2 * i + 0 ] );
            HFIT_GLOBAL::param_fixed .push_back( false );

            double ofs;
            double min = -1e99;
            double max = 1e99;
            if ( cb_pct_center->isChecked() )
            {
               ofs = base_val * le_pct_center->text().toDouble() / 100.0;
               min = base_val - ofs;
               max = base_val + ofs;
            }

            HFIT_GLOBAL::param_min   .push_back( min );
            HFIT_GLOBAL::param_max   .push_back( max );
         }


         if ( cb_fix_width->isChecked() ||
              fixed_curves.count( i ) )
         {
            HFIT_GLOBAL::param_pos   .push_back( HFIT_GLOBAL::fixed_params.size() );
            HFIT_GLOBAL::fixed_params.push_back( hplc_win->unified_ggaussian_params[ 2 * i + 1 ] );
            HFIT_GLOBAL::param_fixed .push_back( true );
         } else {
            HFIT_GLOBAL::param_pos   .push_back( HFIT_GLOBAL::init_params.size() );

            if ( cb_pct_width_from_init->isChecked() )
            {
               base_val = gaussians_undo[ 0 ][ 2 * i + 1 ];
            } else {
               base_val = hplc_win->unified_ggaussian_params[ 2 * i + 1 ];
            }            

            HFIT_GLOBAL::init_params .push_back( base_val );
            HFIT_GLOBAL::base_params .push_back( hplc_win->unified_ggaussian_params[ 2 * i + 1] );
            HFIT_GLOBAL::param_fixed .push_back( false );

            double ofs;
            double min = 1e-10;
            double max = 1e99;
            if ( cb_pct_width->isChecked() )
            {
               ofs = base_val * le_pct_width->text().toDouble() / 100.0;
               min = base_val - ofs;
               max = base_val + ofs;
            }
            if ( min < 1e-10 )
            {
               min = 1e-10;
            }
            if ( min > max )
            {
               min = max * 1e-15;
            }

            HFIT_GLOBAL::param_min   .push_back( min );
            HFIT_GLOBAL::param_max   .push_back( max );
         }
      }         
      
      for ( unsigned int i = 0; i < hplc_win->unified_ggaussian_gaussians_size * hplc_win->unified_ggaussian_curves; i++ )
      {
         unsigned int pos = i / hplc_win->unified_ggaussian_curves;

         unsigned int height_pos = 2 * hplc_win->unified_ggaussian_gaussians_size + i + 0;

         if ( cb_fix_amplitude->isChecked() ||
              fixed_curves.count( pos ) )
         {
            HFIT_GLOBAL::param_pos   .push_back( HFIT_GLOBAL::fixed_params.size() );
            HFIT_GLOBAL::fixed_params.push_back( hplc_win->unified_ggaussian_params[ height_pos ] );
            HFIT_GLOBAL::param_fixed .push_back( true );
         } else {
            HFIT_GLOBAL::param_pos   .push_back( HFIT_GLOBAL::init_params.size() );

            if ( cb_pct_amplitude_from_init->isChecked() )
            {
               base_val = gaussians_undo[ 0 ][ height_pos ];
            } else {
               base_val = hplc_win->unified_ggaussian_params[ height_pos ];
            }            

            HFIT_GLOBAL::init_params .push_back( base_val );
            HFIT_GLOBAL::base_params .push_back( hplc_win->unified_ggaussian_params[ height_pos ] );
            HFIT_GLOBAL::param_fixed .push_back( false );

            double ofs;
            double min = 1e-10;
            double max = hplc_win->gauss_max_height;
            if ( cb_pct_amplitude->isChecked() )
            {
               ofs = base_val * le_pct_amplitude->text().toDouble() / 100.0;
               min = base_val - ofs;
               max = base_val + ofs;
            }
            if ( min < 1e-10 )
            {
               min = 1e-10;
            }
            if ( max > hplc_win->gauss_max_height )
            {
               max = hplc_win->gauss_max_height;
            }
            if ( min > max )
            {
               min = max * 1e-15;
            }

            HFIT_GLOBAL::param_min   .push_back( min );
            HFIT_GLOBAL::param_max   .push_back( max );
         }
      }

   } else {

      // unified_ggaussians have first "unified_ggaussian_gaussians_size" elements as common centers
      // and the next unified_ggaussian_gaussians_size * 2 * unified_ggaussian_curves elements as the curve specific
      // height, widths

      // the centers first:

      for ( unsigned int i = 0; i < hplc_win->unified_ggaussian_gaussians_size; i++ )
      {
         if ( cb_fix_center->isChecked() ||
              fixed_curves.count( i ) )
         {
            HFIT_GLOBAL::param_pos   .push_back( HFIT_GLOBAL::fixed_params.size() );
            HFIT_GLOBAL::fixed_params.push_back( hplc_win->unified_ggaussian_params[ i ] );
            HFIT_GLOBAL::param_fixed .push_back( true );
         } else {
            HFIT_GLOBAL::param_pos   .push_back( HFIT_GLOBAL::init_params.size() );

            if ( cb_pct_center_from_init->isChecked() )
            {
               base_val = gaussians_undo[ 0 ][ i ];
            } else {
               base_val = hplc_win->unified_ggaussian_params[ i ];
            }            

            HFIT_GLOBAL::init_params .push_back( base_val );
            HFIT_GLOBAL::base_params .push_back( hplc_win->unified_ggaussian_params[ i ] );
            HFIT_GLOBAL::param_fixed .push_back( false );

            double ofs;
            double min = -1e99;
            double max = 1e99;
            if ( cb_pct_center->isChecked() )
            {
               ofs = base_val * le_pct_center->text().toDouble() / 100.0;
               min = base_val - ofs;
               max = base_val + ofs;
            }

            HFIT_GLOBAL::param_min   .push_back( min );
            HFIT_GLOBAL::param_max   .push_back( max );
         }
      }         
      
      for ( unsigned int i = 0; i < hplc_win->unified_ggaussian_gaussians_size * 2 * hplc_win->unified_ggaussian_curves; i+= 2 )
      {
         unsigned int pos = i / ( 2 * hplc_win->unified_ggaussian_curves );

         unsigned int height_pos = hplc_win->unified_ggaussian_gaussians_size + i + 0;
         unsigned int width_pos  = hplc_win->unified_ggaussian_gaussians_size + i + 1;

         if ( cb_fix_amplitude->isChecked() ||
              fixed_curves.count( pos ) )
         {
            HFIT_GLOBAL::param_pos   .push_back( HFIT_GLOBAL::fixed_params.size() );
            HFIT_GLOBAL::fixed_params.push_back( hplc_win->unified_ggaussian_params[ height_pos ] );
            HFIT_GLOBAL::param_fixed .push_back( true );
         } else {
            HFIT_GLOBAL::param_pos   .push_back( HFIT_GLOBAL::init_params.size() );

            if ( cb_pct_amplitude_from_init->isChecked() )
            {
               base_val = gaussians_undo[ 0 ][ height_pos ];
            } else {
               base_val = hplc_win->unified_ggaussian_params[ height_pos ];
            }            

            HFIT_GLOBAL::init_params .push_back( base_val );
            HFIT_GLOBAL::base_params .push_back( hplc_win->unified_ggaussian_params[ height_pos ] );
            HFIT_GLOBAL::param_fixed .push_back( false );

            double ofs;
            double min = 1e-10;
            double max = hplc_win->gauss_max_height;
            if ( cb_pct_amplitude->isChecked() )
            {
               ofs = base_val * le_pct_amplitude->text().toDouble() / 100.0;
               min = base_val - ofs;
               max = base_val + ofs;
            }
            if ( min < 1e-10 )
            {
               min = 1e-10;
            }
            if ( max > hplc_win->gauss_max_height )
            {
               max = hplc_win->gauss_max_height;
            }
            if ( min > max )
            {
               min = max * 1e-15;
            }

            HFIT_GLOBAL::param_min   .push_back( min );
            HFIT_GLOBAL::param_max   .push_back( max );
         }
         
         if ( cb_fix_width->isChecked() ||
              fixed_curves.count( pos + 1 ) )
         {
            HFIT_GLOBAL::param_pos   .push_back( HFIT_GLOBAL::fixed_params.size() );
            HFIT_GLOBAL::fixed_params.push_back( hplc_win->unified_ggaussian_params[ width_pos ] );
            HFIT_GLOBAL::param_fixed .push_back( true );
         } else {
            HFIT_GLOBAL::param_pos   .push_back( HFIT_GLOBAL::init_params.size() );

            if ( cb_pct_width_from_init->isChecked() )
            {
               base_val = gaussians_undo[ 0 ][ width_pos ];
            } else {
               base_val = hplc_win->unified_ggaussian_params[ width_pos ];
            }            

            HFIT_GLOBAL::init_params .push_back( base_val );
            HFIT_GLOBAL::base_params .push_back( hplc_win->unified_ggaussian_params[ width_pos ] );
            HFIT_GLOBAL::param_fixed .push_back( false );

            double ofs;
            double min = 1e-10;
            double max = 1e99;
            if ( cb_pct_width->isChecked() )
            {
               ofs = base_val * le_pct_width->text().toDouble() / 100.0;
               min = base_val - ofs;
               max = base_val + ofs;
            }
            if ( min < 1e-10 )
            {
               min = 1e-10;
            }
            if ( min > max )
            {
               min = max * 1e-15;
            }

            HFIT_GLOBAL::param_min   .push_back( min );
            HFIT_GLOBAL::param_max   .push_back( max );
         }
      }
   }
   */

   // HFIT_GLOBAL::list_params();

   // US_Vector::printvector( "is_common", is_common );

   if ( !HFIT_GLOBAL::init_params.size() )
   {
      return false;
   } else {
      return true;
   }
}

void US_Hydrodyn_Saxs_Hplc_Fit_Global::lm()
{
   running = true;
   update_enables();
   // setup_run();
   cout << "lm fit start\n";

   LM::lm_control_struct control = LM::lm_control_double;
   control.printflags = 0; // 3; // monitor status (+1) and parameters (+2)
   control.epsilon    = le_epsilon   ->text().toDouble();
   control.stepbound  = le_iterations->text().toInt();
   control.maxcall    = le_population->text().toInt();

   LM::lm_status_struct status;

   vector < double > t = hplc_win->unified_ggaussian_t;
   vector < double > y = hplc_win->unified_ggaussian_I;

   vector < double >    org_params = HFIT_GLOBAL::init_params;

   HFIT_GLOBAL::errors        = hplc_win->unified_ggaussian_e;
   HFIT_GLOBAL::errors_index  .clear();

   if ( use_errors )
   {
      HFIT_GLOBAL::errors_index.resize( ( unsigned int ) t.back() + 1 );
      for ( unsigned int i = 0; i <= ( unsigned int ) t.back(); i++ )
      {
         HFIT_GLOBAL::errors_index[ i ] = 0;
      }
      for ( unsigned int i = 0; i < y.size(); i++ )
      {
         y[ i ] /= HFIT_GLOBAL::errors[ i ];
         HFIT_GLOBAL::errors_index[ ( unsigned int )t[ i ] ] = i;
      }
   }

   double org_rmsd = hplc_win->ggaussian_rmsd();

   vector < double > gsum  = hplc_win->compute_ggaussian_gaussian_sum();
   vector < double > gsumf( t.size() );

   for ( unsigned int i = 0; i < ( unsigned int ) t.size(); i++ )
   {
      gsumf[ i ] = (*HFIT_GLOBAL::compute_gaussian_f)( t[ i ], &HFIT_GLOBAL::init_params[ 0 ] );
   }

   if ( gsum != gsumf )
   {
      bool tol_ok = true;
#define TOL 1e-5
      for ( unsigned int i = 0; i < (unsigned int)gsum.size(); ++i )
      {
         if ( fabs( gsum[ i ] - gsumf[ i ] ) > TOL )
         {
            tol_ok = false;
            break;
         }
      }
      if ( tol_ok )
      {
         cout << QString( "NOTICE: gsums within tolernace %1\n" ).arg( TOL );
      } else {
         // US_Vector::printvector( "gsum", gsum );
         // US_Vector::printvector( "gsumf", gsumf );
         cout << "WARNING: gsums don't match\n";
         cout << QString( "WARNING: gsums OUTSIDE tolernace %1\n" ).arg( TOL );
      }
#undef TOL
   } else {
      // US_Vector::printvector( "gsum", gsum );
      cout << "gsums match\n";
   }

   // hplc_win->add_ggaussian_curve( "lm_start", gsumf );

   vector < double > par = HFIT_GLOBAL::init_params;
   // US_Vector::printvector( QString( "par start (rmsd %1)" ).arg( org_rmsd ), par );
   cout << QString( "par start (rmsd %1)\n" ).arg( org_rmsd ).ascii();

   progress->setProgress( 1, 2 );
   qApp->processEvents();
   // LM::qpb  = hplc_win->progress;
   // LM::qApp = qApp;

   LM::lmcurve_fit_rmsd( ( int )      par.size(),
                         ( double * ) &( par[ 0 ] ),
                         ( int )      t.size(),
                         ( double * ) &( t[ 0 ] ),
                         ( double * ) &( y[ 0 ] ),
                         HFIT_GLOBAL::compute_gaussian_f,
                         (const LM::lm_control_struct *)&control,
                         &status );
   
   progress->reset();

   if ( status.fnorm < 0e0 )
   {
      status.fnorm = 1e99;
      cout << "WARNING: lm() returned negative rmsd\n";
   }

   // US_Vector::printvector( QString( "par after fit (norm %1)" ).arg( status.fnorm ), par );
   cout << QString( "par fit (rmsd %1)\n" ).arg( status.fnorm ).ascii();

   if ( org_rmsd > status.fnorm )
   {
      for ( unsigned int i = 0; i < HFIT_GLOBAL::param_fixed.size(); i++ )
      {
         if ( !HFIT_GLOBAL::param_fixed[ i ] )
         {
            hplc_win->unified_ggaussian_params[ i ] = par[ HFIT_GLOBAL::param_pos[ i ] ];
         }
      }
      double new_rmsd = hplc_win->ggaussian_rmsd();
      cout << QString( "back checking rmsd gives %1\n" ).arg( new_rmsd  );
      // hplc_win->add_ggaussian_curve( "lm_after_pushback", hplc_win->compute_ggaussian_gaussian_sum() );
      gaussians_undo.push_back( hplc_win->unified_ggaussian_params );
      if ( update_hplc )
      {
         cout << QString( "new rmsd: %1" ).arg( new_rmsd, 0, 'g', 5 );
         hplc_win->lbl_gauss_fit->setText( QString( "%1" ).arg( new_rmsd, 0, 'g', 5 ) );
      }
   } else {
      cout << "no improvement, reverting to original values\n";
   }
      
   if ( update_hplc )
   {
      hplc_win->gauss_init_markers();
      hplc_win->update_gauss_pos();
   }
   running = false;
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc_Fit_Global::gsm_sd()
{
   return; // have to rewrite this 
   /*
   gsm_setup();

   vector < double >    org_params = HFIT_GLOBAL::init_params;
   double org_rmsd = 0e0;
   for ( unsigned int j = 0; j < gsm_t.size(); j++ )
   {
      gsm_yp[ j ]  = HFIT_GLOBAL::compute_gaussian_f( gsm_t[ j ], (double *)(&HFIT_GLOBAL::base_params[ 0 ]) );
      org_rmsd += ( gsm_y[ j ] - gsm_yp[ j ] ) * ( gsm_y[ j ] - gsm_yp[ j ] );
   }
   
   org_rmsd = sqrt( org_rmsd );

   our_vector *v = new_our_vector( HFIT_GLOBAL::init_params.size() );
   for ( int i = 0; i < v->len; i++ )
   {
      v->d[ i ] = HFIT_GLOBAL::init_params[ i ];
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
      for ( unsigned int i = 0; i < HFIT_GLOBAL::param_fixed.size(); i++ )
      {
         if ( !HFIT_GLOBAL::param_fixed[ i ] )
         {
            hplc_win->gaussians[ i ] = v->d[ HFIT_GLOBAL::param_pos[ i ] ];
         }
      }
      gaussians_undo.push_back( hplc_win->gaussians );
   } else {
      cout << "no improvement, reverting to original values\n";
   }
      
   free_our_vector( v );

   if ( update_hplc )
   {
      hplc_win->gauss_init_markers();
      hplc_win->update_gauss_pos();
   }
   progress->reset();
   */
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc_Fit_Global::gsm_ih()
{
   return; // have to rewrite this 

   /*
   gsm_setup();

   vector < double >    org_params = HFIT_GLOBAL::init_params;
   double org_rmsd = 0e0;
   for ( unsigned int j = 0; j < gsm_t.size(); j++ )
   {
      gsm_yp[ j ]  = HFIT_GLOBAL::compute_gaussian_f( gsm_t[ j ], (double *)(&HFIT_GLOBAL::base_params[ 0 ]) );
      org_rmsd += ( gsm_y[ j ] - gsm_yp[ j ] ) * ( gsm_y[ j ] - gsm_yp[ j ] );
   }
   
   org_rmsd = sqrt( org_rmsd );

   our_vector *v = new_our_vector( HFIT_GLOBAL::init_params.size() );
   for ( int i = 0; i < v->len; i++ )
   {
      v->d[ i ] = HFIT_GLOBAL::init_params[ i ];
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
      for ( unsigned int i = 0; i < HFIT_GLOBAL::param_fixed.size(); i++ )
      {
         if ( !HFIT_GLOBAL::param_fixed[ i ] )
         {
            hplc_win->gaussians[ i ] = v->d[ HFIT_GLOBAL::param_pos[ i ] ];
         }
      }
      gaussians_undo.push_back( hplc_win->gaussians );
   } else {
      cout << "no improvement, reverting to original values\n";
   }
      
   free_our_vector( v );

   if ( update_hplc )
   {
      hplc_win->gauss_init_markers();
      hplc_win->update_gauss_pos();
   }
   progress->reset();
   */
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc_Fit_Global::gsm_cg()
{
   return; // have to rewrite this 

   /*

   gsm_setup();

   vector < double >    org_params = HFIT_GLOBAL::init_params;
   double org_rmsd = 0e0;
   for ( unsigned int j = 0; j < gsm_t.size(); j++ )
   {
      gsm_yp[ j ]  = HFIT_GLOBAL::compute_gaussian_f( gsm_t[ j ], (double *)(&HFIT_GLOBAL::base_params[ 0 ]) );
      org_rmsd += ( gsm_y[ j ] - gsm_yp[ j ] ) * ( gsm_y[ j ] - gsm_yp[ j ] );
   }
   
   org_rmsd = sqrt( org_rmsd );

   our_vector *v = new_our_vector( HFIT_GLOBAL::init_params.size() );
   for ( int i = 0; i < v->len; i++ )
   {
      v->d[ i ] = HFIT_GLOBAL::init_params[ i ];
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
      for ( unsigned int i = 0; i < HFIT_GLOBAL::param_fixed.size(); i++ )
      {
         if ( !HFIT_GLOBAL::param_fixed[ i ] )
         {
            hplc_win->gaussians[ i ] = v->d[ HFIT_GLOBAL::param_pos[ i ] ];
         }
      }
      gaussians_undo.push_back( hplc_win->gaussians );
   } else {
      cout << "no improvement, reverting to original values\n";
   }
      
   free_our_vector( v );

   if ( update_hplc )
   {
      hplc_win->gauss_init_markers();
      hplc_win->update_gauss_pos();
   }
   progress->reset();
   */
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc_Fit_Global::ga()
{
   /*
   gsm_setup();

   vector < double >    org_params = HFIT_GLOBAL::init_params;
   double org_rmsd = 0e0;
   for ( unsigned int j = 0; j < gsm_t.size(); j++ )
   {
      gsm_yp[ j ]  = HFIT_GLOBAL::compute_gaussian_f( gsm_t[ j ], (double *)(&HFIT_GLOBAL::base_params[ 0 ]) );
      org_rmsd += ( gsm_y[ j ] - gsm_yp[ j ] ) * ( gsm_y[ j ] - gsm_yp[ j ] );
   }
   
   our_vector *v = new_our_vector( HFIT_GLOBAL::init_params.size() );
   for ( int i = 0; i < v->len; i++ )
   {
      v->d[ i ] = HFIT_GLOBAL::init_params[ i ];
   }

   double cmp_rmsd = gsm_f( v );

   cout << QString( "org rmsd %1 gsm_f %2\n" ).arg( org_rmsd ).arg( cmp_rmsd );

   double rmsd;
   if ( ga_run( rmsd ) &&
        rmsd < org_rmsd )
   {
      vector < double > par = HFIT_GLOBAL::init_params;

      cout << QString( "ga rmsd %1\n" ).arg( rmsd );
      // US_Vector::printvector( "after ga par is", par );

      for ( unsigned int i = 0; i < HFIT_GLOBAL::param_fixed.size(); i++ )
      {
         if ( !HFIT_GLOBAL::param_fixed[ i ] )
         {
            hplc_win->gaussians[ i ] = par[ HFIT_GLOBAL::param_pos[ i ] ];
         }
      }
      gaussians_undo.push_back( hplc_win->gaussians );
   } else {
      cout << "no improvement, reverting to original values\n";
   }

   if ( update_hplc )
   {
      hplc_win->gauss_init_markers();
      hplc_win->update_gauss_pos();
   }
   progress->reset();
   */
   update_enables();
}


void US_Hydrodyn_Saxs_Hplc_Fit_Global::stop()
{
   running = false;
}

void US_Hydrodyn_Saxs_Hplc_Fit_Global::grid()
{
   return; // have to rewrite this 
   setup_run();
   cout << "gauss fit start\n";

   vector < double > x = hplc_win->f_qs[ hplc_win->wheel_file ];
   vector < double > t;
   vector < double > y;
   vector < double > yp( x.size() );

   double start = hplc_win->le_gauss_fit_start->text().toDouble();
   double end   = hplc_win->le_gauss_fit_end  ->text().toDouble();

   for ( unsigned int j = 0; j < x.size(); j++ )
   {
      if ( x[ j ] >= start && x[ j ] <= end )
      {
         t.push_back( x[ j ] );
         y.push_back( hplc_win->f_Is[ hplc_win->wheel_file ][ j ] );
      }
   }

   vector < double > par = HFIT_GLOBAL::init_params;

   // US_Vector::printvector( QString( "par start" ), par );

   // determine total count

   unsigned int pop_size    = le_population->text().toUInt();
   unsigned int total_count = ( unsigned int ) pow( (double) pop_size, (int) HFIT_GLOBAL::init_params.size() );

   cout << QString( "total points %1\n" ).arg( total_count );

   vector < double > use_par( par.size() );
   vector < double > ofs_per( par.size() );

   for ( unsigned int j = 0; j < ( unsigned int ) HFIT_GLOBAL::init_params.size(); j++ )
   {
      ofs_per[ j ] = ( HFIT_GLOBAL::param_max[ j ] - HFIT_GLOBAL::param_min[ j ] ) / ( pop_size - 1 );
   }
      
   double best_rmsd     = 1e99;
   vector < double >    best_params;

   vector < double >    org_params = HFIT_GLOBAL::init_params;
   double org_rmsd = 0e0;
   for ( unsigned int j = 0; j < t.size(); j++ )
   {
      yp[ j ]  = HFIT_GLOBAL::compute_gaussian_f( t[ j ], (double *)(&HFIT_GLOBAL::base_params[ 0 ]) );
      org_rmsd += ( y[ j ] - yp[ j ] ) * ( y[ j ] - yp[ j ] );
   }
   
   org_rmsd = sqrt( org_rmsd );

   running = true;
   update_enables();

   for ( unsigned int i = 0; i < total_count; i++ )
   {
      unsigned int remainder = i;
      progress->setProgress( i, total_count );
      qApp->processEvents();

      for ( unsigned int j = 0; j < ( unsigned int ) HFIT_GLOBAL::init_params.size(); j++ )
      {
         unsigned int pos = remainder % pop_size;
         use_par[ j ] = HFIT_GLOBAL::param_min[ j ] + pos * ofs_per[ j ];
         remainder /= pop_size;
      }

      // compute new y
      double rmsd = 0e0;

      for ( unsigned int j = 0; j < t.size(); j++ )
      {
         yp[ j ]  = HFIT_GLOBAL::compute_gaussian_f( t[ j ], (double *)(&use_par[ 0 ]) );
         rmsd += ( y[ j ] - yp[ j ] ) * ( y[ j ] - yp[ j ] );
      }

      rmsd = sqrt( rmsd );
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

      for ( unsigned int i = 0; i < HFIT_GLOBAL::param_fixed.size(); i++ )
      {
         if ( !HFIT_GLOBAL::param_fixed[ i ] )
         {
            hplc_win->gaussians[ i ] = par[ HFIT_GLOBAL::param_pos[ i ] ];
         }
      }
      gaussians_undo.push_back( hplc_win->gaussians );
   } else {
      cout << "no improvement, reverting to original values\n";
   }

   if ( update_hplc )
   {
      hplc_win->gauss_init_markers();
      hplc_win->update_gauss_pos();
   }
   progress->reset();
   update_enables();
}
