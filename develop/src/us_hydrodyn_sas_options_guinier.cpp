#include "../include/us_hydrodyn_asa.h"
#include "../include/us_hydrodyn.h"

#define SLASH "/"
#if defined(WIN32)
#  undef SLASH
#  define SLASH "\\"
#endif

US_Hydrodyn_SasOptionsGuinier::US_Hydrodyn_SasOptionsGuinier(struct saxs_options *saxs_options, bool *sas_options_guinier_widget, void *us_hydrodyn, QWidget *p, const char *name) : QFrame(p, name)
{
   this->sas_options_guinier_widget = sas_options_guinier_widget;
   this->saxs_options = saxs_options;
   this->us_hydrodyn = us_hydrodyn;
   *sas_options_guinier_widget = true;
   USglobal=new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("US-SOMO SAS Guinier Options"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_SasOptionsGuinier::~US_Hydrodyn_SasOptionsGuinier()
{
   *sas_options_guinier_widget = false;
}

void US_Hydrodyn_SasOptionsGuinier::setupGUI()
{
   int minHeight1 = 30;

   lbl_guinier = new QLabel(tr("Guinier Options:"), this);
   lbl_guinier->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_guinier->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_guinier->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_guinier->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_qRgmax = new QLabel(tr(" Maximum q * Rg : "), this);
   lbl_qRgmax->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_qRgmax->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_qRgmax->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_qRgmax = new QLineEdit(this);
   le_qRgmax->setValidator( new QDoubleValidator( le_qRgmax) );
   ( (QDoubleValidator *)le_qRgmax->validator() )->setRange( 0.5, 3, 3 );
   le_qRgmax->setText( QString( "%1" ).arg( (*saxs_options).qRgmax ) );
   le_qRgmax->setEnabled(true);
   le_qRgmax->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_qRgmax->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_qRgmax, SIGNAL( textChanged( const QString & )), SLOT(update_qRgmax( const QString & )));

   lbl_qstart = new QLabel(tr(" Minimum q value : "), this);
   lbl_qstart->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_qstart->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_qstart->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_qstart = new QLineEdit(this);
   le_qstart->setValidator( new QDoubleValidator( le_qstart ) );
   ( (QDoubleValidator *)le_qstart->validator() )->setRange( 0, 1, 3 );
   le_qstart->setText( QString( "%1" ).arg( (*saxs_options).qstart ) );
   le_qstart->setEnabled(true);
   le_qstart->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_qstart->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_qstart, SIGNAL( textChanged( const QString & )), SLOT(update_qstart( const QString & )));

   lbl_qend = new QLabel(tr(" Maximum q value : "), this);
   lbl_qend->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_qend->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_qend->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_qend = new QLineEdit(this);
   le_qend->setValidator( new QDoubleValidator( le_qend ) );
   ( (QDoubleValidator *)le_qend->validator() )->setRange( 0, 1, 3 );
   le_qend->setText( QString( "%1" ).arg( (*saxs_options).qend ) );
   le_qend->setEnabled(true);
   le_qend->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_qend->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_qend, SIGNAL( textChanged( const QString & )), SLOT(update_qend( const QString & )));

   lbl_cs_guinier = new QLabel(tr("CS Guinier Options:"), this);
   lbl_cs_guinier->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_cs_guinier->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_cs_guinier->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_cs_guinier->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_cs_qRgmax = new QLabel(tr(" Maximum q * Rc : "), this);
   lbl_cs_qRgmax->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_cs_qRgmax->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_cs_qRgmax->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_cs_qRgmax = new QLineEdit(this);
   le_cs_qRgmax->setValidator( new QDoubleValidator( le_cs_qRgmax) );
   ( (QDoubleValidator *)le_qRgmax->validator() )->setRange( 0.5, 3, 3 );
   le_cs_qRgmax->setText( QString( "%1" ).arg( (*saxs_options).cs_qRgmax ) );
   le_cs_qRgmax->setEnabled(true);
   le_cs_qRgmax->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_cs_qRgmax->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_cs_qRgmax, SIGNAL( textChanged( const QString & )), SLOT(update_cs_qRgmax( const QString & )));

   lbl_cs_qstart = new QLabel(tr(" Minimum q^2 value : "), this);
   lbl_cs_qstart->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_cs_qstart->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_cs_qstart->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_cs_qstart = new QLineEdit(this);
   le_cs_qstart->setValidator( new QDoubleValidator( le_cs_qstart ) );
   ( (QDoubleValidator *)le_cs_qstart->validator() )->setRange( 0, 1, 3 );
   le_cs_qstart->setText( QString( "%1" ).arg( (*saxs_options).cs_qstart ) );
   le_cs_qstart->setEnabled(true);
   le_cs_qstart->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_cs_qstart->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_cs_qstart, SIGNAL( textChanged( const QString & )), SLOT(update_qstart( const QString & )));

   lbl_cs_qend = new QLabel(tr(" Maximum q^2 value : "), this);
   lbl_cs_qend->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_cs_qend->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_cs_qend->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_cs_qend = new QLineEdit(this);
   le_cs_qend->setValidator( new QDoubleValidator( le_cs_qend ) );
   ( (QDoubleValidator *)le_cs_qend->validator() )->setRange( 0, 1, 3 );
   le_cs_qend->setText( QString( "%1" ).arg( (*saxs_options).cs_qend ) );
   le_cs_qend->setEnabled(true);
   le_cs_qend->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_cs_qend->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_cs_qend, SIGNAL( textChanged( const QString & )), SLOT(update_qend( const QString & )));

   lbl_guinier_and_cs_guinier = new QLabel(tr("Guinier and CS Guinier Options:"), this);
   lbl_guinier_and_cs_guinier->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_guinier_and_cs_guinier->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_guinier_and_cs_guinier->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_guinier_and_cs_guinier->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_pointsmin = new QLabel(tr(" Minimum number of points : "), this);
   lbl_pointsmin->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_pointsmin->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_pointsmin->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_pointsmin = new QLineEdit(this);
   le_pointsmin->setValidator( new QIntValidator( le_pointsmin ) );
   ( (QIntValidator *)le_pointsmin->validator() )->setRange( 4, 50 );
   le_pointsmin->setText( QString( "%1" ).arg( (*saxs_options).pointsmin ) );
   le_pointsmin->setEnabled(true);
   le_pointsmin->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_pointsmin->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_pointsmin, SIGNAL( textChanged( const QString & )), SLOT(update_pointsmin( const QString & )));

   lbl_pointsmax = new QLabel(tr(" Maximum number of points : "), this);
   lbl_pointsmax->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_pointsmax->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_pointsmax->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_pointsmax = new QLineEdit(this);
   le_pointsmax->setValidator( new QIntValidator( le_pointsmax ) );
   ( (QIntValidator *)le_pointsmax->validator() )->setRange( 10, 100 );
   le_pointsmax->setText( QString( "%1" ).arg( (*saxs_options).pointsmax ) );
   le_pointsmax->setEnabled(true);
   le_pointsmax->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_pointsmax->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_pointsmax, SIGNAL( textChanged( const QString & ) ), SLOT(update_pointsmax( const QString & )));

   cb_guinier_use_sd = new QCheckBox(this);
   cb_guinier_use_sd->setText(tr(" Use SDs for fitting "));
   cb_guinier_use_sd->setEnabled(true);
   cb_guinier_use_sd->setChecked((*saxs_options).guinier_use_sd);
   cb_guinier_use_sd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_guinier_use_sd->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_guinier_use_sd, SIGNAL(clicked()), this, SLOT(set_guinier_use_sd()));

   cb_guinier_outlier_reject = new QCheckBox(this);
   cb_guinier_outlier_reject->setText(tr(" Repeat the analysis after discarding points\n over the regression line by more than SD of "));
   cb_guinier_outlier_reject->setEnabled(true);
   cb_guinier_outlier_reject->setChecked((*saxs_options).guinier_outlier_reject);
   cb_guinier_outlier_reject->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_guinier_outlier_reject->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_guinier_outlier_reject, SIGNAL(clicked()), this, SLOT(set_guinier_outlier_reject()));

   le_guinier_outlier_reject_dist = new QLineEdit(this);
   le_guinier_outlier_reject_dist->setValidator( new QDoubleValidator( le_guinier_outlier_reject_dist ) );
   ( (QDoubleValidator *)le_guinier_outlier_reject_dist->validator() )->setRange( 0, 1, 2 );
   le_guinier_outlier_reject_dist->setText( QString( "%1" ).arg( (*saxs_options).guinier_outlier_reject_dist ) );
   le_guinier_outlier_reject_dist->setEnabled(true);
   le_guinier_outlier_reject_dist->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_guinier_outlier_reject_dist->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_guinier_outlier_reject_dist, SIGNAL( textChanged( const QString & )), SLOT(update_qstart( const QString & )));

   cb_guinier_csv = new QCheckBox(this);
   cb_guinier_csv->setText(tr(" Save Guinier results to csv file: "));
   cb_guinier_csv->setEnabled(true);
   cb_guinier_csv->setChecked((*saxs_options).guinier_csv);
   cb_guinier_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_guinier_csv->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_guinier_csv, SIGNAL(clicked()), this, SLOT(set_guinier_csv()));

   le_guinier_csv_filename = new QLineEdit(this, "guinier_csv_filename Line Edit");
   le_guinier_csv_filename->setText((*saxs_options).guinier_csv_filename);
   le_guinier_csv_filename->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_csv_filename->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_guinier_csv_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_guinier_csv_filename, SIGNAL(textChanged(const QString &)), SLOT(update_guinier_csv_filename(const QString &)));

   lbl_conc_header = new QLabel(tr("Concentration computation options:"), this);
   lbl_conc_header->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_conc_header->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_conc_header->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_conc_header->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_conc = new QLabel(tr(" Concentration (mg/ml) : "), this);
   lbl_conc->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_conc->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_conc = new QLineEdit(this);
   le_conc->setValidator( new QDoubleValidator( le_conc ) );
   le_conc->setText( QString( "%1" ).arg( (*saxs_options).conc ) );
   le_conc->setEnabled(true);
   le_conc->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_conc->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_conc, SIGNAL( textChanged( const QString & )), SLOT(update_conc( const QString &)));

   lbl_psv = new QLabel( tr(" Partial specific volume (ml/g): "), this);
   lbl_psv->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_psv->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_psv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_psv = new QLineEdit(this);
   le_psv->setValidator( new QDoubleValidator( le_psv ) );
   le_psv->setText( QString( "%1" ).arg( (*saxs_options).psv ) );
   le_psv->setEnabled(true);
   le_psv->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_psv->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_psv, SIGNAL( textChanged( const QString & )), SLOT(update_psv( const QString &)));

   cb_use_cs_psv = new QCheckBox(this);
   cb_use_cs_psv->setText( tr(" Partial specific volume override for CS (ml/g): ") );
   cb_use_cs_psv->setEnabled( true );
   connect( cb_use_cs_psv, SIGNAL( clicked() ), SLOT( set_use_cs_psv() ) );
   cb_use_cs_psv->setChecked((*saxs_options).use_cs_psv );
   cb_use_cs_psv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_use_cs_psv->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   le_cs_psv = new QLineEdit(this);
   le_cs_psv->setValidator( new QDoubleValidator( le_cs_psv ) );
   le_cs_psv->setText( QString( "%1" ).arg( (*saxs_options).cs_psv ) );
   le_cs_psv->setEnabled(true);
   le_cs_psv->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_cs_psv->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_cs_psv, SIGNAL( textChanged( const QString & )), SLOT(update_cs_psv( const QString &)));

   lbl_diffusion_len = new QLabel(tr(" Diffusion length (cm) : "), this);
   lbl_diffusion_len->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_diffusion_len->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_diffusion_len->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_diffusion_len = new QLineEdit(this);
   le_diffusion_len->setValidator( new QDoubleValidator( le_diffusion_len ) );
   le_diffusion_len->setText( QString( "%1" ).arg( (*saxs_options).diffusion_len ) );
   le_diffusion_len->setEnabled(true);
   le_diffusion_len->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_diffusion_len->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_diffusion_len, SIGNAL( textChanged( const QString & )), SLOT(update_diffusion_len( const QString &)));

   lbl_nucleon_mass = new QLabel(tr(" Nucleon mass (g) : "), this);
   lbl_nucleon_mass->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_nucleon_mass->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_nucleon_mass->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_nucleon_mass = new QLineEdit(this);
   le_nucleon_mass->setValidator( new QDoubleValidator( le_nucleon_mass ) );
   le_nucleon_mass->setText( QString( "%1" ).arg( (*saxs_options).nucleon_mass ) );
   le_nucleon_mass->setEnabled(true);
   le_nucleon_mass->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_nucleon_mass->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_nucleon_mass, SIGNAL( textChanged( const QString & )), SLOT(update_nucleon_mass( const QString &)));

   cb_guinier_use_standards = new QCheckBox(this);
   cb_guinier_use_standards->setText( tr(" Use I0 standards for normalization") );
   cb_guinier_use_standards->setEnabled( true );
   connect( cb_guinier_use_standards, SIGNAL( clicked() ), SLOT( set_guinier_use_standards() ) );
   cb_guinier_use_standards->setChecked((*saxs_options).guinier_use_standards );
   cb_guinier_use_standards->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_guinier_use_standards->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   lbl_I0_exp = new QLabel(tr(" I0 standard experimental (a.u.) : "), this);
   lbl_I0_exp->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_I0_exp->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_I0_exp->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_I0_exp = new QLineEdit(this);
   le_I0_exp->setValidator( new QDoubleValidator( le_I0_exp ) );
   le_I0_exp->setText( QString( "%1" ).arg( (*saxs_options).I0_exp ) );
   le_I0_exp->setEnabled(true);
   le_I0_exp->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_I0_exp->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_I0_exp, SIGNAL( textChanged( const QString & )), SLOT(update_I0_exp( const QString &)));

   lbl_I0_theo = new QLabel(tr(" I0 standard theoretical (cm^-1) : "), this);
   lbl_I0_theo->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_I0_theo->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_I0_theo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_I0_theo = new QLineEdit(this);
   le_I0_theo->setValidator( new QDoubleValidator( le_I0_theo ) );
   le_I0_theo->setText( QString( "%1" ).arg( (*saxs_options).I0_theo ) );
   le_I0_theo->setEnabled(true);
   le_I0_theo->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_I0_theo->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_I0_theo, SIGNAL( textChanged( const QString & )), SLOT(update_I0_theo( const QString &)));

   pb_guinier = new QPushButton(tr("Process Guinier"), this);
   pb_guinier->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_guinier->setMinimumHeight(minHeight1);
   pb_guinier->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_guinier, SIGNAL(clicked()), SLOT(guinier()));

   pb_cs_guinier = new QPushButton(tr("Process CS Guinier"), this);
   pb_cs_guinier->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cs_guinier->setMinimumHeight(minHeight1);
   pb_cs_guinier->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cs_guinier, SIGNAL(clicked()), SLOT(cs_guinier()));

   pb_cancel = new QPushButton(tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   int rows = 0, columns = 2, spacing = 2, j=0, margin=4;
   QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);

   background->addMultiCellWidget(lbl_guinier, j, j, 0, 1);
   j++;
   background->addWidget(lbl_qRgmax, j, 0);
   background->addWidget(le_qRgmax, j, 1);
   j++;

   background->addMultiCellWidget(lbl_cs_guinier, j, j, 0, 1);
   j++;
   background->addWidget(lbl_cs_qRgmax, j, 0);
   background->addWidget(le_cs_qRgmax, j, 1);
   j++;

   background->addMultiCellWidget(lbl_guinier_and_cs_guinier, j, j, 0, 1);
   j++;
   background->addWidget(lbl_qstart, j, 0);
   background->addWidget(le_qstart, j, 1);
   j++;
   background->addWidget(lbl_qend, j, 0);
   background->addWidget(le_qend, j, 1);
   j++;
   background->addWidget(lbl_cs_qstart, j, 0);
   background->addWidget(le_cs_qstart, j, 1);
   j++;
   background->addWidget(lbl_cs_qend, j, 0);
   background->addWidget(le_cs_qend, j, 1);
   j++;
   background->addWidget(lbl_pointsmin, j, 0);
   background->addWidget(le_pointsmin, j, 1);
   j++;
   background->addWidget(lbl_pointsmax, j, 0);
   background->addWidget(le_pointsmax, j, 1);
   j++;
   background->addMultiCellWidget(cb_guinier_use_sd, j, j, 0, 1);
   j++;
   background->addWidget(cb_guinier_outlier_reject, j, 0);
   background->addWidget(le_guinier_outlier_reject_dist, j, 1);
   j++;
   background->addWidget(cb_guinier_csv, j, 0);
   background->addWidget(le_guinier_csv_filename, j, 1);
   j++;

   background->addMultiCellWidget(lbl_conc_header, j, j, 0, 1);
   j++;

   background->addWidget(lbl_conc, j, 0);
   background->addWidget(le_conc, j, 1);
   j++;

   background->addWidget(lbl_psv, j, 0);
   background->addWidget(le_psv, j, 1);
   j++;

   background->addWidget(cb_use_cs_psv, j, 0);
   background->addWidget(le_cs_psv, j, 1);
   j++;

   background->addWidget(lbl_diffusion_len, j, 0);
   background->addWidget(le_diffusion_len, j, 1);
   j++;

   background->addWidget(lbl_nucleon_mass, j, 0);
   background->addWidget(le_nucleon_mass, j, 1);
   j++;

   background->addMultiCellWidget( cb_guinier_use_standards, j, j, 0, 1);
   j++;

   background->addWidget(lbl_I0_exp, j, 0);
   background->addWidget(le_I0_exp, j, 1);
   j++;

   background->addWidget(lbl_I0_theo, j, 0);
   background->addWidget(le_I0_theo, j, 1);
   j++;



   {
      QGridLayout * gl2 = new QGridLayout( 0 );
      
      gl2->addWidget( pb_guinier    , 0, 0 );
      gl2->addWidget( pb_cs_guinier , 0, 1 );

      gl2->addWidget( pb_help       , 1, 0 );
      gl2->addWidget( pb_cancel     , 1, 1 );

      background->addMultiCellLayout( gl2, j, j, 0, 1);
      j++;
   }

   setMinimumWidth( 400 );
}

void US_Hydrodyn_SasOptionsGuinier::cancel()
{
   close();
}

void US_Hydrodyn_SasOptionsGuinier::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   // online_help->show_help("manual/somo_sas_options_guinier.html");
   online_help->show_help("manual/somo_saxs_options.html");
}

void US_Hydrodyn_SasOptionsGuinier::closeEvent(QCloseEvent *e)
{
   *sas_options_guinier_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}


void US_Hydrodyn_SasOptionsGuinier::update_qRgmax( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).qRgmax = val;
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_qstart( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).qstart = val;
   (*saxs_options).cs_qstart = val * val;
   disconnect(le_cs_qstart, SIGNAL( textChanged( const QString & )), 0, 0 );
   le_cs_qstart->setText( QString( "%1" ).arg( (*saxs_options).cs_qstart ) );
   connect(le_cs_qstart, SIGNAL( textChanged( const QString & )), SLOT(update_cs_qstart( const QString & )));
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_qend( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).qend = val;
   (*saxs_options).cs_qend = val * val;
   disconnect(le_cs_qend, SIGNAL( textChanged( const QString & )), 0, 0 );
   le_cs_qend->setText( QString( "%1" ).arg( (*saxs_options).cs_qend ) );
   connect(le_cs_qend, SIGNAL( textChanged( const QString & )), SLOT(update_cs_qend( const QString & )));
   
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_cs_qRgmax( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).cs_qRgmax = val;
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_cs_qstart( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).cs_qstart = val;
   (*saxs_options).qstart = sqrt( val );
   disconnect(le_qstart, SIGNAL( textChanged( const QString & )), 0, 0 );
   le_qstart->setText( QString( "%1" ).arg( (*saxs_options).qstart ) );
   connect(le_qstart, SIGNAL( textChanged( const QString & )), SLOT(update_qstart( const QString & )));
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_cs_qend( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).cs_qend = val;
   (*saxs_options).qend = sqrt( val );
   disconnect(le_qend, SIGNAL( textChanged( const QString & )), 0, 0 );
   le_qend->setText( QString( "%1" ).arg( (*saxs_options).qend ) );
   connect(le_qend, SIGNAL( textChanged( const QString & )), SLOT(update_qend( const QString & )));
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_pointsmin( const QString & str )
{
   unsigned int val = str.toUInt();
   (*saxs_options).pointsmin = (unsigned int) val;
   if ( (*saxs_options).pointsmax < (unsigned int) val )
   {
      (*saxs_options).pointsmax = (unsigned int) val;
      le_pointsmax->setText( QString( "%1" ).arg( val ) );
   }
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_pointsmax( const QString & str )
{
   unsigned int val = str.toUInt();
   (*saxs_options).pointsmax = val;
   if ( (*saxs_options).pointsmin > val )
   {
      (*saxs_options).pointsmin = val;
      le_pointsmin->setText( QString( "%1" ).arg( val ) );
   }
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::set_guinier_csv()
{
   (*saxs_options).guinier_csv = cb_guinier_csv->isChecked();
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_guinier_csv_filename(const QString &str)
{
   (*saxs_options).guinier_csv_filename = str;
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_conc( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).conc = val;
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_psv( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).psv = val;
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::set_guinier_use_sd()
{
   (*saxs_options).guinier_use_sd = cb_guinier_use_sd->isChecked();
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::set_guinier_outlier_reject()
{
   (*saxs_options).guinier_outlier_reject = cb_guinier_outlier_reject->isChecked();
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_guinier_outlier_reject_dist( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).guinier_outlier_reject_dist = val;
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::set_use_cs_psv()
{
   (*saxs_options).use_cs_psv = cb_use_cs_psv->isChecked();
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_cs_psv( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).cs_psv = val;
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::set_guinier_use_standards()
{
   (*saxs_options).guinier_use_standards = cb_guinier_use_standards->isChecked();
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_I0_exp( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).I0_exp = val;
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_I0_theo( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).I0_theo = val;
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_diffusion_len( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).diffusion_len = val;
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_nucleon_mass( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).nucleon_mass = val;
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::guinier()
{
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->isVisible() )
      {
         ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->raise();
      }
      else
      {
         ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->show();
      }
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->run_guinier_analysis();
   } else {
      QMessageBox::message( caption() + ": Process Guinier",
                            tr( "The main SAS window is not active" ) );
   }
}

void US_Hydrodyn_SasOptionsGuinier::cs_guinier()
{
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->isVisible() )
      {
         ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->raise();
      }
      else
      {
         ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->show();
      }
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->run_guinier_cs();
   } else {
      QMessageBox::message( caption() + ": Process Guinier",
                            tr( "The main SAS window is not active" ) );
   }
}
