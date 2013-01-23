#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_saxs_hplc.h"
#include "../include/us_hydrodyn_saxs_hplc_fit.h"
#include "../include/us_lm.h"

US_Hydrodyn_Saxs_Hplc_Fit::US_Hydrodyn_Saxs_Hplc_Fit(
                                                     US_Hydrodyn_Saxs_Hplc *hplc_win,
                                                     QWidget *p, 
                                                     const char *name
                                                     ) : QDialog(p, name)
{
   this->hplc_win = hplc_win;

   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption( tr( "US-SOMO: SAXS Hplc: Gaussian Fit" ) );

   running = false;
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry(global_Xpos, global_Ypos, 0, 0 );
   update_enables();
}

US_Hydrodyn_Saxs_Hplc_Fit::~US_Hydrodyn_Saxs_Hplc_Fit()
{
}

void US_Hydrodyn_Saxs_Hplc_Fit::setupGUI()
{
   int minHeight1 = 30;

   lbl_title = new QLabel( tr( "US-SOMO: SAXS Hplc: Gaussian Fit" ), this);
   lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize , QFont::Bold));

   cb_fix_center = new QCheckBox(this);
   cb_fix_center->setText(tr(" Fix Gaussian centers" ) );
   cb_fix_center->setEnabled(true);
   cb_fix_center->setChecked( false );
   cb_fix_center->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_fix_center->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_fix_center, SIGNAL( clicked() ), SLOT( update_enables() ) );

   cb_pct_center = new QCheckBox(this);
   cb_pct_center->setText(tr(" % variation" ) );
   cb_pct_center->setEnabled(true);
   cb_pct_center->setChecked( false );
   cb_pct_center->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pct_center->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_pct_center, SIGNAL( clicked() ), SLOT( update_enables() ) );

   le_pct_center = new mQLineEdit(this, "le_pct_center Line Edit");
   le_pct_center->setText( "50" );
   le_pct_center->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_pct_center->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_pct_center->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   le_pct_center->setEnabled( false );
   le_pct_center->setValidator( new QDoubleValidator( le_pct_center ) );
   ( (QDoubleValidator *)le_pct_center->validator() )->setRange( 0, 100, 1 );

   cb_fix_width = new QCheckBox(this);
   cb_fix_width->setText(tr(" Fix Gaussian widths" ) );
   cb_fix_width->setEnabled(true);
   cb_fix_width->setChecked( false );
   cb_fix_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_fix_width->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_fix_width, SIGNAL( clicked() ), SLOT( update_enables() ) );

   cb_pct_width = new QCheckBox(this);
   cb_pct_width->setText(tr(" % variation" ) );
   cb_pct_width->setEnabled(true);
   cb_pct_width->setChecked( false );
   cb_pct_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pct_width->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_pct_width, SIGNAL( clicked() ), SLOT( update_enables() ) );

   le_pct_width = new mQLineEdit(this, "le_pct_width Line Edit");
   le_pct_width->setText( "50" );
   le_pct_width->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_pct_width->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_pct_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   le_pct_width->setEnabled( false );
   le_pct_width->setValidator( new QDoubleValidator( le_pct_width ) );
   ( (QDoubleValidator *)le_pct_width->validator() )->setRange( 0, 100, 1 );

   cb_fix_amplitude = new QCheckBox(this);
   cb_fix_amplitude->setText(tr(" Fix Gaussian amplitudes" ) );
   cb_fix_amplitude->setEnabled(true);
   cb_fix_amplitude->setChecked( false );
   cb_fix_amplitude->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_fix_amplitude->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_fix_amplitude, SIGNAL( clicked() ), SLOT( update_enables() ) );

   cb_pct_amplitude = new QCheckBox(this);
   cb_pct_amplitude->setText(tr(" % variation" ) );
   cb_pct_amplitude->setEnabled(true);
   cb_pct_amplitude->setChecked( false );
   cb_pct_amplitude->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pct_amplitude->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_pct_amplitude, SIGNAL( clicked() ), SLOT( update_enables() ) );

   le_pct_amplitude = new mQLineEdit(this, "le_pct_amplitude Line Edit");
   le_pct_amplitude->setText( "50" );
   le_pct_amplitude->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_pct_amplitude->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_pct_amplitude->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   le_pct_amplitude->setEnabled( false );
   le_pct_amplitude->setValidator( new QDoubleValidator( le_pct_amplitude ) );
   ( (QDoubleValidator *)le_pct_amplitude->validator() )->setRange( 0, 100, 1 );

   lbl_fix_curves = new QLabel(tr(" Fix Gaussians: "), this);
   lbl_fix_curves->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   // lbl_fix_curves->setMinimumHeight(minHeight1);
   lbl_fix_curves->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_fix_curves->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));


   /* old way
   QString qs_rx = QString( "^(|(\\d+)|(\\d+(,\\d+){0,%1}))$" ).arg( hplc_win->gaussians.size() / 3 - 1 );
   cout << "qs_rx:" << qs_rx << endl;
   QRegExp rx_fix_curves( qs_rx );
   QRegExpValidator *rx_val_fix_curves = new QRegExpValidator( rx_fix_curves, this );
   le_fix_curves = new mQLineEdit(this, "le_fix_curves Line Edit");
   le_fix_curves->setText( "" );
   le_fix_curves->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_fix_curves->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_fix_curves->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   le_fix_curves->setEnabled( false );
   le_fix_curves->setValidator( rx_val_fix_curves );
   connect( le_fix_curves, SIGNAL( textChanged( const QString & ) ), SLOT( update_enables() ) );
   */

   for ( unsigned int i = 0; i < ( unsigned int ) hplc_win->gaussians.size() / 3; i++ )
   {
      QCheckBox *cb_tmp;
      cb_tmp = new QCheckBox(this);
      cb_tmp->setText( QString( " %1 " ).arg( i + 1 ) );
      cb_tmp->setEnabled( true );
      cb_tmp->setChecked( false );
      cb_tmp->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      cb_tmp->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      connect(cb_tmp, SIGNAL( clicked() ), SLOT( update_enables() ) );
      cb_fix_curves.push_back( cb_tmp );
   }

   lbl_epsilon = new QLabel(tr(" Epsilon: "), this);
   lbl_epsilon->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   // lbl_epsilon->setMinimumHeight(minHeight1);
   lbl_epsilon->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_epsilon->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_epsilon = new mQLineEdit(this, "le_epsilon Line Edit");
   le_epsilon->setText( "0.001" );
   le_epsilon->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_epsilon->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_epsilon->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   le_epsilon->setEnabled( false );
   le_epsilon->setValidator( new QDoubleValidator( le_epsilon ) );
   ( (QDoubleValidator *)le_epsilon->validator() )->setRange( 0e-3, 10, 3 );

   lbl_iterations = new QLabel(tr(" Iterations: "), this);
   lbl_iterations->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   // lbl_iterations->setMinimumHeight(minHeight1);
   lbl_iterations->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_iterations->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_iterations = new mQLineEdit(this, "le_iterations Line Edit");
   le_iterations->setText( "100" );
   le_iterations->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_iterations->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_iterations->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   le_iterations->setEnabled( false );
   le_iterations->setValidator( new QIntValidator( le_iterations ) );
   ( (QIntValidator *)le_iterations->validator() )->setRange( 1, 10000 );

   lbl_population = new QLabel(tr(" Population/Grid: "), this);
   lbl_population->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   // lbl_population->setMinimumHeight(minHeight1);
   lbl_population->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_population->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_population = new mQLineEdit(this, "le_population Line Edit");
   le_population->setText( "100" );
   le_population->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_population->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_population->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   le_population->setEnabled( false );
   le_population->setValidator( new QIntValidator( le_population ) );
   ( (QIntValidator *)le_population->validator() )->setRange( 2, 10000 );

   pb_lm = new QPushButton(tr("LM"), this);
   pb_lm->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_lm->setMinimumHeight(minHeight1);
   pb_lm->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_lm, SIGNAL(clicked()), SLOT(lm()));

   pb_gsm_sd = new QPushButton(tr("GS SD"), this);
   pb_gsm_sd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_gsm_sd->setMinimumHeight(minHeight1);
   pb_gsm_sd->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_gsm_sd, SIGNAL(clicked()), SLOT(gsm_sd()));

   pb_gsm_ih = new QPushButton(tr("GS IH"), this);
   pb_gsm_ih->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_gsm_ih->setMinimumHeight(minHeight1);
   pb_gsm_ih->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_gsm_ih, SIGNAL(clicked()), SLOT(gsm_ih()));

   pb_gsm_cg = new QPushButton(tr("GS CG"), this);
   pb_gsm_cg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_gsm_cg->setMinimumHeight(minHeight1);
   pb_gsm_cg->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_gsm_cg, SIGNAL(clicked()), SLOT(gsm_cg()));

   pb_ga = new QPushButton(tr("GA"), this);
   pb_ga->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_ga->setMinimumHeight(minHeight1);
   pb_ga->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_ga, SIGNAL(clicked()), SLOT(ga()));

   pb_grid = new QPushButton(tr("Grid"), this);
   pb_grid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_grid->setMinimumHeight(minHeight1);
   pb_grid->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_grid, SIGNAL(clicked()), SLOT(grid()));

   progress = new QProgressBar(this, "Progress");
   // progress->setMinimumHeight(minHeight1);
   progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   progress->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize  ));
   progress->reset();

   pb_stop = new QPushButton(tr("Stop"), this);
   pb_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_stop->setMinimumHeight(minHeight1);
   pb_stop->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_stop, SIGNAL(clicked()), SLOT(stop()));

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));


   // build layout
   QGridLayout *gl_main = new QGridLayout( 0 );
   int row = 0;

   gl_main->addWidget( cb_fix_center , row, 0 );
   gl_main->addWidget( cb_pct_center , row, 1 );
   gl_main->addWidget( le_pct_center , row, 2 );
   row++;

   gl_main->addWidget( cb_fix_width , row, 0 );
   gl_main->addWidget( cb_pct_width , row, 1 );
   gl_main->addWidget( le_pct_width , row, 2 );
   row++;

   gl_main->addWidget( cb_fix_amplitude , row, 0 );
   gl_main->addWidget( cb_pct_amplitude , row, 1 );
   gl_main->addWidget( le_pct_amplitude , row, 2 );
   row++;

   gl_main->addWidget         ( lbl_fix_curves, row, 0 );
   // gl_main->addMultiCellWidget( le_fix_curves , row, row, 1, 2 );
   QHBoxLayout *hbl_fix_curves = new QHBoxLayout;
   for ( unsigned int i = 0; i < ( unsigned int ) cb_fix_curves.size(); i++ )
   {
      hbl_fix_curves->addWidget( cb_fix_curves[ i ] );
   }
   gl_main->addMultiCellLayout( hbl_fix_curves , row, row, 1, 2 );

   row++;

   gl_main->addWidget         ( lbl_epsilon, row, 0 );
   gl_main->addMultiCellWidget( le_epsilon , row, row, 1, 2 );
   row++;

   gl_main->addWidget         ( lbl_iterations, row, 0 );
   gl_main->addMultiCellWidget( le_iterations , row, row, 1, 2 );
   row++;

   gl_main->addWidget         ( lbl_population, row, 0 );
   gl_main->addMultiCellWidget( le_population , row, row, 1, 2 );
   row++;

   QHBoxLayout *hbl_runs = new QHBoxLayout;
   hbl_runs->addWidget ( pb_lm );
   hbl_runs->addWidget ( pb_gsm_sd );
   hbl_runs->addWidget ( pb_gsm_ih );
   hbl_runs->addWidget ( pb_gsm_cg );
   hbl_runs->addWidget ( pb_ga );
   hbl_runs->addWidget ( pb_grid );

   QHBoxLayout *hbl_prog = new QHBoxLayout;
   hbl_prog->addWidget ( progress );
   hbl_prog->addWidget ( pb_stop );

   QHBoxLayout *hbl_bottom = new QHBoxLayout;
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addWidget ( pb_cancel );

   QVBoxLayout *background = new QVBoxLayout( this );
   background->addWidget ( lbl_title );
   background->addSpacing( 4 );
   background->addLayout ( gl_main  );
   background->addSpacing( 4 );
   background->addLayout ( hbl_runs );
   background->addSpacing( 4 );
   background->addLayout ( hbl_prog );
   background->addSpacing( 4 );
   background->addLayout ( hbl_bottom );
}

void US_Hydrodyn_Saxs_Hplc_Fit::cancel()
{
   close();
}

void US_Hydrodyn_Saxs_Hplc_Fit::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_saxs_hplc_fit.html");
}

void US_Hydrodyn_Saxs_Hplc_Fit::closeEvent(QCloseEvent *e)
{
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Saxs_Hplc_Fit::update_enables()
{
   puts( "hf: ue()" );
   bool run_ok = setup_run();

   cb_fix_center            ->setEnabled( !running );
   cb_pct_center            ->setEnabled( !running && !cb_fix_center->isChecked() );
   le_pct_center            ->setEnabled( !running && !cb_fix_center->isChecked() && cb_pct_center->isChecked() );

   cb_fix_width             ->setEnabled( !running );
   cb_pct_width             ->setEnabled( !running && !cb_fix_width->isChecked() );
   le_pct_width             ->setEnabled( !running && !cb_fix_width->isChecked()  && cb_pct_width->isChecked() );

   cb_fix_amplitude         ->setEnabled( !running );
   cb_pct_amplitude         ->setEnabled( !running && !cb_fix_amplitude->isChecked() );
   le_pct_amplitude         ->setEnabled( !running && !cb_fix_amplitude->isChecked() && cb_pct_amplitude->isChecked() );

   for ( unsigned int i = 0; i < ( unsigned int ) cb_fix_curves.size(); i++ )
   {
      cb_fix_curves[ i ]      ->setEnabled( !running );
   }

   // le_fix_curves            ->setEnabled( !running );
   le_epsilon               ->setEnabled( !running );
   le_iterations            ->setEnabled( !running );
   le_population            ->setEnabled( !running );

   bool variations_set      = 
      ( cb_fix_center   ->isChecked() || cb_pct_center   ->isChecked() ) &&
      ( cb_fix_width    ->isChecked() || cb_pct_width    ->isChecked() ) &&
      ( cb_fix_amplitude->isChecked() || cb_pct_amplitude->isChecked() )
      ;


   pb_lm                    ->setEnabled( !running && run_ok );
   pb_gsm_sd                ->setEnabled( !running && run_ok );
   pb_gsm_ih                ->setEnabled( !running && run_ok );
   pb_gsm_cg                ->setEnabled( !running && run_ok );
   pb_ga                    ->setEnabled( !running && run_ok && variations_set );
   pb_grid                  ->setEnabled( !running && run_ok && variations_set );

   pb_stop                  ->setEnabled( running );
}

namespace HFIT 
{
   vector < double       > init_params;    // variable param initial values

   vector < double       > fixed_params;   // the fixed params
   vector < unsigned int > param_pos;      // index into fixed params or variable params
   vector < bool         > param_fixed;    
   vector < double       > param_min;      // minimum values for variable params
   vector < double       > param_max;      // maximum values for variable params

   double compute_gaussian_f( double t, const double *par )
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
         result += height * exp( - tmp * tmp / 2 );
      }
      
      return result;
   }

   void printvector( QString qs, vector < double > x )
   {
      cout << QString( "%1: size %2:" ).arg( qs ).arg( x.size() );
      for ( unsigned int i = 0; i < x.size(); i++ )
      {
         cout << QString( " %1" ).arg( x[ i ] );
      }
      cout << endl;
   }

   void printvector( QString qs, vector < unsigned int > x )
   {
      cout << QString( "%1: size %2:" ).arg( qs ).arg( x.size() );
      for ( unsigned int i = 0; i < x.size(); i++ )
      {
         cout << QString( " %1" ).arg( x[ i ] );
      }
      cout << endl;
   }

   void printvector( QString qs, vector < bool > x )
   {
      cout << QString( "%1: size %2:" ).arg( qs ).arg( x.size() );
      for ( unsigned int i = 0; i < x.size(); i++ )
      {
         cout << QString( " %1" ).arg( x[ i ] ? "true" : "false" );
      }
      cout << endl;
   }

   void list_params()
   {
      printvector( "init_params ", init_params  );
      printvector( "fixed_params", fixed_params );
      printvector( "param_pos   ", param_pos    );
      printvector( "param_fixed ", param_fixed  );
      printvector( "param_min   ", param_min    );
      printvector( "param_max   ", param_max    );
   }
};


bool US_Hydrodyn_Saxs_Hplc_Fit::setup_run()
{
   HFIT::init_params .clear();
   HFIT::fixed_params.clear();
   HFIT::param_pos   .clear();
   HFIT::param_fixed .clear();
   HFIT::param_min   .clear();
   HFIT::param_max   .clear();

   map < unsigned int, bool > fixed_curves;

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
         fixed_curves[ i + 1 ] = true;
      } else {
         any_not_fixed = true;
      }
   }

   if ( !any_not_fixed )
   {
      return false;
   }

   for ( unsigned int i = 0; i < ( unsigned int ) hplc_win->gaussians.size(); i+= 3 )
   {
      unsigned int pos = i / 3;

      if ( cb_fix_amplitude->isChecked() ||
           fixed_curves.count( pos + 1 ) )
      {
         HFIT::param_pos   .push_back( HFIT::fixed_params.size() );
         HFIT::fixed_params.push_back( hplc_win->gaussians[ 0 + i ] );
         HFIT::param_fixed .push_back( true );
      } else {
         HFIT::param_pos   .push_back( HFIT::init_params.size() );
         HFIT::init_params .push_back( hplc_win->gaussians[ 0 + i ] );
         HFIT::param_fixed .push_back( false );

         double ofs;
         double min = 0.001;
         double max = hplc_win->gauss_max_height;
         if ( cb_pct_amplitude->isChecked() )
         {
            ofs = hplc_win->gaussians[ 0 + i ] * le_pct_amplitude->text().toDouble() / 100.0;
            min = hplc_win->gaussians[ 0 + i ] - ofs;
            max = hplc_win->gaussians[ 0 + i ] + ofs;
         }
         if ( min < 0.001 )
         {
            min = 0.001;
         }
         if ( max > hplc_win->gauss_max_height )
         {
            max = hplc_win->gauss_max_height;
         }

         HFIT::param_min   .push_back( min );
         HFIT::param_max   .push_back( max );
      }
         

      if ( cb_fix_center->isChecked() ||
           fixed_curves.count( pos + 1 ) )
      {
         HFIT::param_pos   .push_back( HFIT::fixed_params.size() );
         HFIT::fixed_params.push_back( hplc_win->gaussians[ 1 + i ] );
         HFIT::param_fixed .push_back( true );
      } else {
         HFIT::param_pos   .push_back( HFIT::init_params.size() );
         HFIT::init_params .push_back( hplc_win->gaussians[ 1 + i ] );
         HFIT::param_fixed .push_back( false );

         double ofs;
         double min = -1e99;
         double max = 1e99;
         if ( cb_pct_center->isChecked() )
         {
            ofs = hplc_win->gaussians[ 1 + i ] * le_pct_center->text().toDouble() / 100.0;
            min = hplc_win->gaussians[ 1 + i ] - ofs;
            max = hplc_win->gaussians[ 1 + i ] + ofs;
         }

         HFIT::param_min   .push_back( min );
         HFIT::param_max   .push_back( max );
      }

      if ( cb_fix_width->isChecked() ||
           fixed_curves.count( pos + 1 ) )
      {
         HFIT::param_pos   .push_back( HFIT::fixed_params.size() );
         HFIT::fixed_params.push_back( hplc_win->gaussians[ 2 + i ] );
         HFIT::param_fixed .push_back( true );
      } else {
         HFIT::param_pos   .push_back( HFIT::init_params.size() );
         HFIT::init_params .push_back( hplc_win->gaussians[ 2 + i ] );
         HFIT::param_fixed .push_back( false );

         double ofs;
         double min = 0.0001;
         double max = 1e99;
         if ( cb_pct_width->isChecked() )
         {
            ofs = hplc_win->gaussians[ 2 + i ] * le_pct_width->text().toDouble() / 100.0;
            min = hplc_win->gaussians[ 2 + i ] - ofs;
            max = hplc_win->gaussians[ 2 + i ] + ofs;
         }
         if ( min < 0.0001 )
         {
            min = 0.0001;
         }

         HFIT::param_min   .push_back( min );
         HFIT::param_max   .push_back( max );
      }
   }

   HFIT::list_params();

   if ( !HFIT::init_params.size() )
   {
      return false;
   } else {
      return true;
   }
}

void US_Hydrodyn_Saxs_Hplc_Fit::lm()
{
   setup_run();
   puts( "lm" );
   cout << "gauss fit start\n";

   LM::lm_control_struct control = LM::lm_control_double;
   control.printflags = 0; // 3; // monitor status (+1) and parameters (+2)
   control.epsilon    = le_epsilon   ->text().toDouble();
   control.stepbound  = le_iterations->text().toInt();
   control.maxcall    = le_population->text().toInt();

   LM::lm_status_struct status;

   vector < double > x = hplc_win->f_qs[ hplc_win->wheel_file ];
   vector < double > t;
   vector < double > y;

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

   vector < double >    org_params = HFIT::init_params;
   double org_rmsd = 0e0;
   {
      vector < double >    yp( x.size() );

      for ( unsigned int j = 0; j < t.size(); j++ )
      {
         yp[ j ]  = HFIT::compute_gaussian_f( t[ j ], (double *)(&HFIT::init_params[ 0 ]) );
         org_rmsd += ( y[ j ] - yp[ j ] ) * ( y[ j ] - yp[ j ] );
      }
      org_rmsd = sqrt( org_rmsd );
   }

   vector < double > par = HFIT::init_params;
   HFIT::printvector( QString( "par start" ), par );

   LM::lmcurve_fit_rmsd( ( int )      par.size(),
                         ( double * ) &( par[ 0 ] ),
                         ( int )      t.size(),
                         ( double * ) &( t[ 0 ] ),
                         ( double * ) &( y[ 0 ] ),
                         HFIT::compute_gaussian_f,
                         (const LM::lm_control_struct *)&control,
                         &status );
   
   HFIT::printvector( QString( "par is now (norm %1)" ).arg( status.fnorm ), par );

   

   if ( org_rmsd > status.fnorm )
   {

      for ( unsigned int i = 0; i < HFIT::param_fixed.size(); i++ )
      {
         if ( !HFIT::param_fixed[ i ] )
         {
            hplc_win->gaussians[ i ] = par[ HFIT::param_pos[ i ] ];
         }
      }
   } else {
      cout << "no improvement, reverting to original values\n";
   }
      
   hplc_win->gauss_init_markers();
   hplc_win->gauss_init_gaussians();
   hplc_win->update_gauss_pos();
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc_Fit::gsm_sd()
{
   puts( "gsm_sd" );

   gsm_setup();

   vector < double >    org_params = HFIT::init_params;
   double org_rmsd = 0e0;
   for ( unsigned int j = 0; j < gsm_t.size(); j++ )
   {
      gsm_yp[ j ]  = HFIT::compute_gaussian_f( gsm_t[ j ], (double *)(&HFIT::init_params[ 0 ]) );
      org_rmsd += ( gsm_y[ j ] - gsm_yp[ j ] ) * ( gsm_y[ j ] - gsm_yp[ j ] );
   }
   
   org_rmsd = sqrt( org_rmsd );

   our_vector *v = new_our_vector( HFIT::init_params.size() );
   for ( int i = 0; i < v->len; i++ )
   {
      v->d[ i ] = HFIT::init_params[ i ];
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
      for ( unsigned int i = 0; i < HFIT::param_fixed.size(); i++ )
      {
         if ( !HFIT::param_fixed[ i ] )
         {
            hplc_win->gaussians[ i ] = v->d[ HFIT::param_pos[ i ] ];
         }
      }
   } else {
      cout << "no improvement, reverting to original values\n";
   }
      
   free_our_vector( v );

   hplc_win->gauss_init_markers();
   hplc_win->gauss_init_gaussians();
   hplc_win->update_gauss_pos();
   progress->reset();
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc_Fit::gsm_ih()
{
   puts( "gsm_ih" );

   gsm_setup();

   vector < double >    org_params = HFIT::init_params;
   double org_rmsd = 0e0;
   for ( unsigned int j = 0; j < gsm_t.size(); j++ )
   {
      gsm_yp[ j ]  = HFIT::compute_gaussian_f( gsm_t[ j ], (double *)(&HFIT::init_params[ 0 ]) );
      org_rmsd += ( gsm_y[ j ] - gsm_yp[ j ] ) * ( gsm_y[ j ] - gsm_yp[ j ] );
   }
   
   org_rmsd = sqrt( org_rmsd );

   our_vector *v = new_our_vector( HFIT::init_params.size() );
   for ( int i = 0; i < v->len; i++ )
   {
      v->d[ i ] = HFIT::init_params[ i ];
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
      for ( unsigned int i = 0; i < HFIT::param_fixed.size(); i++ )
      {
         if ( !HFIT::param_fixed[ i ] )
         {
            hplc_win->gaussians[ i ] = v->d[ HFIT::param_pos[ i ] ];
         }
      }
   } else {
      cout << "no improvement, reverting to original values\n";
   }
      
   free_our_vector( v );

   hplc_win->gauss_init_markers();
   hplc_win->gauss_init_gaussians();
   hplc_win->update_gauss_pos();
   progress->reset();
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc_Fit::gsm_cg()
{
   puts( "gsm_cg" );

   gsm_setup();

   vector < double >    org_params = HFIT::init_params;
   double org_rmsd = 0e0;
   for ( unsigned int j = 0; j < gsm_t.size(); j++ )
   {
      gsm_yp[ j ]  = HFIT::compute_gaussian_f( gsm_t[ j ], (double *)(&HFIT::init_params[ 0 ]) );
      org_rmsd += ( gsm_y[ j ] - gsm_yp[ j ] ) * ( gsm_y[ j ] - gsm_yp[ j ] );
   }
   
   org_rmsd = sqrt( org_rmsd );

   our_vector *v = new_our_vector( HFIT::init_params.size() );
   for ( int i = 0; i < v->len; i++ )
   {
      v->d[ i ] = HFIT::init_params[ i ];
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
      for ( unsigned int i = 0; i < HFIT::param_fixed.size(); i++ )
      {
         if ( !HFIT::param_fixed[ i ] )
         {
            hplc_win->gaussians[ i ] = v->d[ HFIT::param_pos[ i ] ];
         }
      }
   } else {
      cout << "no improvement, reverting to original values\n";
   }
      
   free_our_vector( v );

   hplc_win->gauss_init_markers();
   hplc_win->gauss_init_gaussians();
   hplc_win->update_gauss_pos();
   progress->reset();
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc_Fit::ga()
{
   puts( "ga" );
   gsm_setup();

   vector < double >    org_params = HFIT::init_params;
   double org_rmsd = 0e0;
   for ( unsigned int j = 0; j < gsm_t.size(); j++ )
   {
      gsm_yp[ j ]  = HFIT::compute_gaussian_f( gsm_t[ j ], (double *)(&HFIT::init_params[ 0 ]) );
      org_rmsd += ( gsm_y[ j ] - gsm_yp[ j ] ) * ( gsm_y[ j ] - gsm_yp[ j ] );
   }
   
   our_vector *v = new_our_vector( HFIT::init_params.size() );
   for ( int i = 0; i < v->len; i++ )
   {
      v->d[ i ] = HFIT::init_params[ i ];
   }

   double cmp_rmsd = gsm_f( v );

   cout << QString( "org rmsd %1 gsm_f %2\n" ).arg( org_rmsd ).arg( cmp_rmsd );

   double rmsd;
   if ( ga_run( rmsd ) &&
        rmsd < org_rmsd )
   {
      vector < double > par = HFIT::init_params;

      cout << QString( "ga rmsd %1\n" ).arg( rmsd );
      HFIT::printvector( "after ga par is", par );

      for ( unsigned int i = 0; i < HFIT::param_fixed.size(); i++ )
      {
         if ( !HFIT::param_fixed[ i ] )
         {
            hplc_win->gaussians[ i ] = par[ HFIT::param_pos[ i ] ];
         }
      }
   } else {
      cout << "no improvement, reverting to original values\n";
   }

   hplc_win->gauss_init_markers();
   hplc_win->gauss_init_gaussians();
   hplc_win->update_gauss_pos();
   progress->reset();
   update_enables();
}


void US_Hydrodyn_Saxs_Hplc_Fit::stop()
{
   running = false;
}

void US_Hydrodyn_Saxs_Hplc_Fit::grid()
{
   setup_run();
   puts( "grid" );
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

   vector < double > par = HFIT::init_params;

   HFIT::printvector( QString( "par start" ), par );

   // determine total count

   unsigned int pop_size    = le_population->text().toUInt();
   unsigned int total_count = ( unsigned int ) pow( (int) pop_size, HFIT::init_params.size() );

   cout << QString( "total points %1\n" ).arg( total_count );

   vector < double > use_par( par.size() );
   vector < double > ofs_per( par.size() );

   for ( unsigned int j = 0; j < ( unsigned int ) HFIT::init_params.size(); j++ )
   {
      ofs_per[ j ] = ( HFIT::param_max[ j ] - HFIT::param_min[ j ] ) / ( pop_size - 1 );
   }
      
   double best_rmsd     = 1e99;
   vector < double >    best_params;

   vector < double >    org_params = HFIT::init_params;
   double org_rmsd = 0e0;
   for ( unsigned int j = 0; j < t.size(); j++ )
   {
      yp[ j ]  = HFIT::compute_gaussian_f( t[ j ], (double *)(&HFIT::init_params[ 0 ]) );
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

      for ( unsigned int j = 0; j < ( unsigned int ) HFIT::init_params.size(); j++ )
      {
         unsigned int pos = remainder % pop_size;
         use_par[ j ] = HFIT::param_min[ j ] + pos * ofs_per[ j ];
         remainder /= pop_size;
      }

      // compute new y
      double rmsd = 0e0;

      for ( unsigned int j = 0; j < t.size(); j++ )
      {
         yp[ j ]  = HFIT::compute_gaussian_f( t[ j ], (double *)(&use_par[ 0 ]) );
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

      HFIT::printvector( "after grid par is", par );

      for ( unsigned int i = 0; i < HFIT::param_fixed.size(); i++ )
      {
         if ( !HFIT::param_fixed[ i ] )
         {
            hplc_win->gaussians[ i ] = par[ HFIT::param_pos[ i ] ];
         }
      }
   } else {
      cout << "no improvement, reverting to original values\n";
   }

   hplc_win->gauss_init_markers();
   hplc_win->gauss_init_gaussians();
   hplc_win->update_gauss_pos();
   progress->reset();
   update_enables();
}
