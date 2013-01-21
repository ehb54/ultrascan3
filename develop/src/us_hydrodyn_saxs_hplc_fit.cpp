#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_saxs_hplc.h"
#include "../include/us_hydrodyn_saxs_hplc_fit.h"

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

   lbl_pct_center = new QLabel(tr(" % variation: "), this);
   lbl_pct_center->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   // lbl_pct_center->setMinimumHeight(minHeight1);
   lbl_pct_center->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_pct_center->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_pct_center = new mQLineEdit(this, "le_pct_center Line Edit");
   le_pct_center->setText( "25" );
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

   lbl_pct_width = new QLabel(tr(" % variation: "), this);
   lbl_pct_width->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   // lbl_pct_width->setMinimumHeight(minHeight1);
   lbl_pct_width->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_pct_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_pct_width = new mQLineEdit(this, "le_pct_width Line Edit");
   le_pct_width->setText( "25" );
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

   lbl_pct_amplitude = new QLabel(tr(" % variation: "), this);
   lbl_pct_amplitude->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   // lbl_pct_amplitude->setMinimumHeight(minHeight1);
   lbl_pct_amplitude->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_pct_amplitude->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_pct_amplitude = new mQLineEdit(this, "le_pct_amplitude Line Edit");
   le_pct_amplitude->setText( "25" );
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
   le_population->setText( "10" );
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

   pb_cancel = new QPushButton(tr("Cancel"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   // pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));


   // build layout
   QGridLayout *gl_main = new QGridLayout( 0 );
   int row = 0;

   gl_main->addWidget( cb_fix_center , row, 0 );
   gl_main->addWidget( lbl_pct_center, row, 1 );
   gl_main->addWidget( le_pct_center , row, 2 );
   row++;

   gl_main->addWidget( cb_fix_width , row, 0 );
   gl_main->addWidget( lbl_pct_width, row, 1 );
   gl_main->addWidget( le_pct_width , row, 2 );
   row++;

   gl_main->addWidget( cb_fix_amplitude , row, 0 );
   gl_main->addWidget( lbl_pct_amplitude, row, 1 );
   gl_main->addWidget( le_pct_amplitude , row, 2 );
   row++;

   gl_main->addWidget         ( lbl_fix_curves, row, 0 );
   gl_main->addMultiCellWidget( le_fix_curves , row, row, 1, 2 );
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
   hbl_bottom->addWidget ( pb_cancel );
   hbl_bottom->addWidget ( pb_help );

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
   cb_fix_center            ->setEnabled( !running );
   le_pct_center            ->setEnabled( !running && !cb_fix_center->isChecked() );

   cb_fix_width             ->setEnabled( !running );
   le_pct_width             ->setEnabled( !running && !cb_fix_width->isChecked() );

   cb_fix_amplitude         ->setEnabled( !running );
   le_pct_amplitude         ->setEnabled( !running && !cb_fix_amplitude->isChecked() );

   le_fix_curves            ->setEnabled( !running );
   le_epsilon               ->setEnabled( !running );
   le_iterations            ->setEnabled( !running );
   le_population            ->setEnabled( !running );

   pb_lm                    ->setEnabled( !running );
   pb_gsm_sd                ->setEnabled( !running );
   pb_gsm_ih                ->setEnabled( !running );
   pb_gsm_cg                ->setEnabled( !running );
   pb_ga                    ->setEnabled( !running );
   pb_grid                  ->setEnabled( !running );

   pb_stop                  ->setEnabled( running );
}

void US_Hydrodyn_Saxs_Hplc_Fit::lm()
{
   puts( "lm" );
}

void US_Hydrodyn_Saxs_Hplc_Fit::gsm_sd()
{
   puts( "gsm_sd" );
}

void US_Hydrodyn_Saxs_Hplc_Fit::gsm_ih()
{
   puts( "gsm_ih" );
}

void US_Hydrodyn_Saxs_Hplc_Fit::gsm_cg()
{
   puts( "gsm_cg" );
}

void US_Hydrodyn_Saxs_Hplc_Fit::ga()
{
   puts( "ga" );
}

void US_Hydrodyn_Saxs_Hplc_Fit::grid()
{
   puts( "grid" );
}

void US_Hydrodyn_Saxs_Hplc_Fit::stop()
{
   running = false;
   update_enables();
}
