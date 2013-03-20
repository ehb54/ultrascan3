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
   lbl_info = new QLabel(tr("US-SOMO Guinier Options:"), this);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   // lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   cb_guinier_csv = new QCheckBox(this);
   cb_guinier_csv->setText(tr(" Save Guinier results to csv file: "));
   cb_guinier_csv->setEnabled(true);
   cb_guinier_csv->setChecked((*saxs_options).guinier_csv);
   cb_guinier_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_guinier_csv->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_guinier_csv, SIGNAL(clicked()), this, SLOT(set_guinier_csv()));

   le_guinier_csv_filename = new QLineEdit(this, "guinier_csv_filename Line Edit");
   le_guinier_csv_filename->setText((*saxs_options).guinier_csv_filename);
   // le_guinier_csv_filename->setMinimumHeight(minHeight1);
   le_guinier_csv_filename->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_csv_filename->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_guinier_csv_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_guinier_csv_filename, SIGNAL(textChanged(const QString &)), SLOT(update_guinier_csv_filename(const QString &)));

   lbl_qRgmax = new QLabel(tr(" Maximum q * Rg : "), this);
   lbl_qRgmax->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   // lbl_qRgmax->setMinimumHeight(minHeight1);
   lbl_qRgmax->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_qRgmax->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_qRgmax = new QLineEdit(this);
   // le_qRgmax->setRange(0.5, 3, 0.001);
   le_qRgmax->setValidator( new QDoubleValidator( le_qRgmax) );
   ( (QDoubleValidator *)le_qRgmax->validator() )->setRange( 0.5, 3, 3 );
   le_qRgmax->setText( QString( "%1" ).arg( (*saxs_options).qRgmax ) );
   // le_qRgmax->setMinimumHeight(minHeight1);
   le_qRgmax->setEnabled(true);
   // le_qRgmax->setNumButtons(SAXS_Q_BUTTONS);
   le_qRgmax->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_qRgmax->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_qRgmax, SIGNAL( textChanged( const QString & )), SLOT(update_qRgmax( const QString & )));

   lbl_qstart = new QLabel(tr(" Minimum q value for Guinier search : "), this);
   lbl_qstart->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   // lbl_qstart->setMinimumHeight(minHeight1);
   lbl_qstart->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_qstart->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_qstart = new QLineEdit(this);
   // le_qstart->setRange(0, 1, 0.001);
   le_qstart->setValidator( new QDoubleValidator( le_qstart ) );
   ( (QDoubleValidator *)le_qstart->validator() )->setRange( 0, 1, 3 );
   le_qstart->setText( QString( "%1" ).arg( (*saxs_options).qstart ) );
   // le_qstart->setMinimumHeight(minHeight1);
   le_qstart->setEnabled(true);
   // le_qstart->setNumButtons(SAXS_Q_BUTTONS);
   le_qstart->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_qstart->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_qstart, SIGNAL( textChanged( const QString & )), SLOT(update_qstart( const QString & )));

   lbl_qend = new QLabel(tr(" Maximum q value for Guinier search : "), this);
   lbl_qend->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   // lbl_qend->setMinimumHeight(minHeight1);
   lbl_qend->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_qend->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_qend = new QLineEdit(this);
   // le_qend->setRange(0, 1, 0.001);
   le_qend->setValidator( new QDoubleValidator( le_qend ) );
   ( (QDoubleValidator *)le_qend->validator() )->setRange( 0, 1, 3 );
   le_qend->setText( QString( "%1" ).arg( (*saxs_options).qend ) );
   // le_qend->setMinimumHeight(minHeight1);
   le_qend->setEnabled(true);
   // le_qend->setNumButtons(SAXS_Q_BUTTONS);
   le_qend->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_qend->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_qend, SIGNAL( textChanged( const QString & )), SLOT(update_qend( const QString & )));

   lbl_pointsmin = new QLabel(tr(" Minimum number of points : "), this);
   lbl_pointsmin->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   // lbl_pointsmin->setMinimumHeight(minHeight1);
   lbl_pointsmin->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_pointsmin->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_pointsmin = new QLineEdit(this);
   // le_pointsmin->setRange(4, 50, 1);
   le_pointsmin->setValidator( new QIntValidator( le_pointsmin ) );
   ( (QIntValidator *)le_pointsmin->validator() )->setRange( 4, 50 );
   le_pointsmin->setText( QString( "%1" ).arg( (*saxs_options).pointsmin ) );
   // le_pointsmin->setMinimumHeight(minHeight1);
   le_pointsmin->setEnabled(true);
   // le_pointsmin->setNumButtons(SAXS_Q_BUTTONS);
   le_pointsmin->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_pointsmin->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_pointsmin, SIGNAL( textChanged( const QString & )), SLOT(update_pointsmin( const QString & )));

   lbl_pointsmax = new QLabel(tr(" Maximum number of points : "), this);
   lbl_pointsmax->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   // lbl_pointsmax->setMinimumHeight(minHeight1);
   lbl_pointsmax->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_pointsmax->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_pointsmax = new QLineEdit(this);
   // le_pointsmax->setRange(10, 100, 1);
   le_pointsmax->setValidator( new QIntValidator( le_pointsmax ) );
   ( (QIntValidator *)le_pointsmax->validator() )->setRange( 10, 100 );
   le_pointsmax->setText( QString( "%1" ).arg( (*saxs_options).pointsmax ) );
   // le_pointsmax->setMinimumHeight(minHeight1);
   le_pointsmax->setEnabled(true);
   // le_pointsmax->setNumButtons(SAXS_Q_BUTTONS);
   le_pointsmax->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_pointsmax->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_pointsmax, SIGNAL( textChanged( const QString & ) ), SLOT(update_pointsmax( const QString & )));

   lbl_conc = new QLabel(tr("Concentration computation options:"), this);
   lbl_conc->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_conc->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   // lbl_conc->setMinimumHeight(minHeight1);
   lbl_conc->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_I0_exp = new QLabel(tr(" I0 experimental (arbitrary) : "), this);
   lbl_I0_exp->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   // lbl_I0_exp->setMinimumHeight(minHeight1);
   lbl_I0_exp->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_I0_exp->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_I0_exp = new QLineEdit(this);
   le_I0_exp->setValidator( new QDoubleValidator( le_I0_exp ) );
   le_I0_exp->setText( QString( "%1" ).arg( (*saxs_options).I0_exp ) );
   // le_I0_exp->setMinimumHeight(minHeight1);
   le_I0_exp->setEnabled(true);
   le_I0_exp->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_I0_exp->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_I0_exp, SIGNAL( textChanged( const QString & )), SLOT(update_I0_exp( const QString &)));

   lbl_I0_theo = new QLabel(tr(" I0 theoretical (cm^-1) : "), this);
   lbl_I0_theo->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   // lbl_I0_theo->setMinimumHeight(minHeight1);
   lbl_I0_theo->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_I0_theo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_I0_theo = new QLineEdit(this);
   le_I0_theo->setValidator( new QDoubleValidator( le_I0_theo ) );
   le_I0_theo->setText( QString( "%1" ).arg( (*saxs_options).I0_theo ) );
   // le_I0_theo->setMinimumHeight(minHeight1);
   le_I0_theo->setEnabled(true);
   le_I0_theo->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_I0_theo->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_I0_theo, SIGNAL( textChanged( const QString & )), SLOT(update_I0_theo( const QString &)));

   lbl_diffusion_len = new QLabel(tr(" Diffusion length (cm) : "), this);
   lbl_diffusion_len->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   // lbl_diffusion_len->setMinimumHeight(minHeight1);
   lbl_diffusion_len->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_diffusion_len->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_diffusion_len = new QLineEdit(this);
   le_diffusion_len->setValidator( new QDoubleValidator( le_diffusion_len ) );
   le_diffusion_len->setText( QString( "%1" ).arg( (*saxs_options).diffusion_len ) );
   // le_diffusion_len->setMinimumHeight(minHeight1);
   le_diffusion_len->setEnabled(true);
   le_diffusion_len->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_diffusion_len->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_diffusion_len, SIGNAL( textChanged( const QString & )), SLOT(update_diffusion_len( const QString &)));

   lbl_nuclear_mass = new QLabel(tr(" Nuclear mass (g) : "), this);
   lbl_nuclear_mass->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   // lbl_nuclear_mass->setMinimumHeight(minHeight1);
   lbl_nuclear_mass->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_nuclear_mass->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_nuclear_mass = new QLineEdit(this);
   le_nuclear_mass->setValidator( new QDoubleValidator( le_nuclear_mass ) );
   le_nuclear_mass->setText( QString( "%1" ).arg( (*saxs_options).nuclear_mass ) );
   // le_nuclear_mass->setMinimumHeight(minHeight1);
   le_nuclear_mass->setEnabled(true);
   le_nuclear_mass->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_nuclear_mass->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_nuclear_mass, SIGNAL( textChanged( const QString & )), SLOT(update_nuclear_mass( const QString &)));

   //    lbl_cs_psv = new QLabel(tr(" CS partial specific volume (ml/g): "), this);
   //    lbl_cs_psv->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   //    // lbl_cs_psv->setMinimumHeight(minHeight1);
   //    lbl_cs_psv->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   //    lbl_cs_psv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cb_use_cs_psv = new QCheckBox(this);
   cb_use_cs_psv->setText( tr(" Partial specific volume overide for CS (ml/g): ") );
   cb_use_cs_psv->setEnabled( true );
   connect( cb_use_cs_psv, SIGNAL( clicked() ), SLOT( set_use_cs_psv() ) );
   cb_use_cs_psv->setChecked( false );
   cb_use_cs_psv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_use_cs_psv->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   // cb_use_cs_psv->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));

   le_cs_psv = new QLineEdit(this);
   le_cs_psv->setValidator( new QDoubleValidator( le_cs_psv ) );
   le_cs_psv->setText( QString( "%1" ).arg( (*saxs_options).cs_psv ) );
   // le_cs_psv->setMinimumHeight(minHeight1);
   le_cs_psv->setEnabled(true);
   le_cs_psv->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_cs_psv->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_cs_psv, SIGNAL( textChanged( const QString & )), SLOT(update_cs_psv( const QString &)));

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

   background->addMultiCellWidget(lbl_info, j, j, 0, 1);
   j++;

   background->addWidget(cb_guinier_csv, j, 0);
   background->addWidget(le_guinier_csv_filename, j, 1);
   j++;
   background->addWidget(lbl_qRgmax, j, 0);
   background->addWidget(le_qRgmax, j, 1);
   j++;
   background->addWidget(lbl_qstart, j, 0);
   background->addWidget(le_qstart, j, 1);
   j++;
   background->addWidget(lbl_qend, j, 0);
   background->addWidget(le_qend, j, 1);
   j++;
   background->addWidget(lbl_pointsmin, j, 0);
   background->addWidget(le_pointsmin, j, 1);
   j++;
   background->addWidget(lbl_pointsmax, j, 0);
   background->addWidget(le_pointsmax, j, 1);
   j++;

   background->addMultiCellWidget(lbl_conc, j, j, 0, 1);
   j++;

   background->addWidget(lbl_I0_exp, j, 0);
   background->addWidget(le_I0_exp, j, 1);
   j++;

   background->addWidget(lbl_I0_theo, j, 0);
   background->addWidget(le_I0_theo, j, 1);
   j++;


   background->addWidget(lbl_diffusion_len, j, 0);
   background->addWidget(le_diffusion_len, j, 1);
   j++;

   background->addWidget(lbl_nuclear_mass, j, 0);
   background->addWidget(le_nuclear_mass, j, 1);
   j++;

   background->addWidget(cb_use_cs_psv, j, 0);
   background->addWidget(le_cs_psv, j, 1);
   j++;

   background->addWidget( pb_help  , j, 0 );
   background->addWidget( pb_cancel, j, 1 );

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
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_qend( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).qend = val;
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

void US_Hydrodyn_SasOptionsGuinier::update_nuclear_mass( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).nuclear_mass = val;
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

