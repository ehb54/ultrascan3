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
   lbl_info->setMinimumHeight(minHeight1);
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
   lbl_qRgmax->setMinimumHeight(minHeight1);
   lbl_qRgmax->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_qRgmax->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_qRgmax = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_qRgmax );
   cnt_qRgmax->setRange(0.5, 3, 0.001);
   cnt_qRgmax->setValue((*saxs_options).qRgmax);
   cnt_qRgmax->setMinimumHeight(minHeight1);
   cnt_qRgmax->setEnabled(true);
   cnt_qRgmax->setNumButtons(SAXS_Q_BUTTONS);
   cnt_qRgmax->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_qRgmax->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_qRgmax, SIGNAL(valueChanged(double)), SLOT(update_qRgmax(double)));

   lbl_qend = new QLabel(tr(" Maximum q value for Guinier search : "), this);
   lbl_qend->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_qend->setMinimumHeight(minHeight1);
   lbl_qend->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_qend->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_qend = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_qend );
   cnt_qend->setRange(0, 1, 0.001);
   cnt_qend->setValue((*saxs_options).qend);
   cnt_qend->setMinimumHeight(minHeight1);
   cnt_qend->setEnabled(true);
   cnt_qend->setNumButtons(SAXS_Q_BUTTONS);
   cnt_qend->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_qend->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_qend, SIGNAL(valueChanged(double)), SLOT(update_qend(double)));

   lbl_pointsmin = new QLabel(tr(" Minimum number of points : "), this);
   lbl_pointsmin->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_pointsmin->setMinimumHeight(minHeight1);
   lbl_pointsmin->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_pointsmin->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_pointsmin = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_pointsmin );
   cnt_pointsmin->setRange(4, 50, 1);
   cnt_pointsmin->setValue((*saxs_options).pointsmin);
   cnt_pointsmin->setMinimumHeight(minHeight1);
   cnt_pointsmin->setEnabled(true);
   cnt_pointsmin->setNumButtons(SAXS_Q_BUTTONS);
   cnt_pointsmin->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_pointsmin->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_pointsmin, SIGNAL(valueChanged(double)), SLOT(update_pointsmin(double)));

   lbl_pointsmax = new QLabel(tr(" Maximum number of points : "), this);
   lbl_pointsmax->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_pointsmax->setMinimumHeight(minHeight1);
   lbl_pointsmax->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_pointsmax->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_pointsmax = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_pointsmax );
   cnt_pointsmax->setRange(10, 100, 1);
   cnt_pointsmax->setValue((*saxs_options).pointsmax);
   cnt_pointsmax->setMinimumHeight(minHeight1);
   cnt_pointsmax->setEnabled(true);
   cnt_pointsmax->setNumButtons(SAXS_Q_BUTTONS);
   cnt_pointsmax->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_pointsmax->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_pointsmax, SIGNAL(valueChanged(double)), SLOT(update_pointsmax(double)));

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
   background->addWidget(cnt_qRgmax, j, 1);
   j++;
   background->addWidget(lbl_qend, j, 0);
   background->addWidget(cnt_qend, j, 1);
   j++;
   background->addWidget(lbl_pointsmin, j, 0);
   background->addWidget(cnt_pointsmin, j, 1);
   j++;
   background->addWidget(lbl_pointsmax, j, 0);
   background->addWidget(cnt_pointsmax, j, 1);
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

void US_Hydrodyn_SasOptionsGuinier::update_qRgmax(double val)
{
   (*saxs_options).qRgmax = val;
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_qend(double val)
{
   (*saxs_options).qend = val;
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}
void US_Hydrodyn_SasOptionsGuinier::update_pointsmin(double val)
{
   (*saxs_options).pointsmin = (unsigned int) val;
   if ( (*saxs_options).pointsmax < (unsigned int) val )
   {
      (*saxs_options).pointsmax = (unsigned int) val;
      cnt_pointsmax->setValue(val);
   }
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}
void US_Hydrodyn_SasOptionsGuinier::update_pointsmax(double val)
{
   (*saxs_options).pointsmax = (unsigned int) val;
   if ( (*saxs_options).pointsmin > (unsigned int) val )
   {
      (*saxs_options).pointsmin = (unsigned int) val;
      cnt_pointsmin->setValue(val);
   }
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}
