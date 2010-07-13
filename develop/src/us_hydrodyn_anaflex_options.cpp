#include "../include/us_hydrodyn_anaflex_options.h"
#include "../include/us_hydrodyn.h"

US_Hydrodyn_Anaflex_Options::US_Hydrodyn_Anaflex_Options(Anaflex_Options *anaflex_options, bool *anaflex_widget, void *us_hydrodyn, QWidget *p, const char *name) : QFrame(p, name)
{
   this->anaflex_options = anaflex_options;
   this->anaflex_widget = anaflex_widget;
   this->us_hydrodyn = us_hydrodyn;
   *anaflex_widget = true;
   USglobal=new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("Anaflex Options"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_Anaflex_Options::~US_Hydrodyn_Anaflex_Options()
{
   *anaflex_widget = false;
}

void US_Hydrodyn_Anaflex_Options::setupGUI()
{
   int minHeight1 = 30;
   lbl_info = new QLabel(tr("Anaflex Options:"), this);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(AlignCenter|AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   cb_run_anaflex = new QCheckBox(this);
   cb_run_anaflex->setText(tr(" Run Anaflex "));
   cb_run_anaflex->setChecked((*anaflex_options).run_anaflex);
   cb_run_anaflex->setEnabled(true);
   //   cb_run_anaflex->setMinimumHeight(minHeight1);
   cb_run_anaflex->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_anaflex->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_anaflex, SIGNAL(clicked()), SLOT(set_run_anaflex()));

   cb_instprofiles = new QCheckBox(this);
   cb_instprofiles->setText(tr(" Store time profiles "));
   cb_instprofiles->setChecked((*anaflex_options).instprofiles);
   cb_instprofiles->setEnabled(true);
   //   cb_instprofiles->setMinimumHeight(minHeight1);
   cb_instprofiles->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_instprofiles->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_instprofiles, SIGNAL(clicked()), SLOT(set_instprofiles()));

   bg_run_mode = new QButtonGroup(5, Qt::Vertical, " Run mode ", this);
   Q_CHECK_PTR(bg_run_mode);
   bg_run_mode->setExclusive(true);
   bg_run_mode->setAlignment(Qt::AlignHCenter);
   bg_run_mode->setInsideMargin(3);
   bg_run_mode->setInsideSpacing(0);
   connect(bg_run_mode, SIGNAL(clicked(int)), this, SLOT(set_run_mode(int)));

   cb_run_mode_1 = new QCheckBox(bg_run_mode);
   cb_run_mode_1->setText(tr(" Compute steady state properties "));
   cb_run_mode_1->setChecked((*anaflex_options).run_mode_1);
   cb_run_mode_1->setEnabled(true);
   //   cb_run_mode_1->setMinimumHeight(minHeight1);
   cb_run_mode_1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_1->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   // connect(cb_run_mode_1, SIGNAL(clicked()), SLOT(set_run_mode_1()));

   cb_run_mode_2 = new QCheckBox(bg_run_mode);
   cb_run_mode_2->setText(tr(" Compute time-dependent properties "));
   cb_run_mode_2->setChecked((*anaflex_options).run_mode_2);
   cb_run_mode_2->setEnabled(true);
   //   cb_run_mode_2->setMinimumHeight(minHeight1);
   cb_run_mode_2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_2->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   // connect(cb_run_mode_2, SIGNAL(clicked()), SLOT(set_run_mode_2()));

   cb_run_mode_3 = new QCheckBox(bg_run_mode);
   cb_run_mode_3->setText(tr(" Compute correlation function "));
   cb_run_mode_3->setChecked((*anaflex_options).run_mode_3);
   cb_run_mode_3->setEnabled(true);
   //   cb_run_mode_3->setMinimumHeight(minHeight1);
   cb_run_mode_3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_3->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   // connect(cb_run_mode_3, SIGNAL(clicked()), SLOT(set_run_mode_3()));

   cb_run_mode_4 = new QCheckBox(bg_run_mode);
   cb_run_mode_4->setText(tr(" Graphical display "));
   cb_run_mode_4->setChecked((*anaflex_options).run_mode_4);
   cb_run_mode_4->setEnabled(true);
   //   cb_run_mode_4->setMinimumHeight(minHeight1);
   cb_run_mode_4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_4->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   // connect(cb_run_mode_4, SIGNAL(clicked()), SLOT(set_run_mode_4()));

   cb_run_mode_9 = new QCheckBox(bg_run_mode);
   cb_run_mode_9->setText(tr(" Create text file "));
   cb_run_mode_9->setChecked((*anaflex_options).run_mode_9);
   cb_run_mode_9->setEnabled(true);
   //   cb_run_mode_9->setMinimumHeight(minHeight1);
   cb_run_mode_9->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_9->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   // connect(cb_run_mode_9, SIGNAL(clicked()), SLOT(set_run_mode_9()));

   lbl_nfrec = new QLabel(tr(" Trajectory sampling frequency: "), this);
   lbl_nfrec->setAlignment(AlignLeft|AlignVCenter);
   lbl_nfrec->setMinimumHeight(minHeight1);
   lbl_nfrec->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_nfrec->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_nfrec = new QLineEdit(this, "Nfrec Line Edit");
   le_nfrec->setText(QString("").sprintf("%d",(*anaflex_options).nfrec));
   le_nfrec->setAlignment(AlignVCenter);
   le_nfrec->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_nfrec->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_nfrec->setEnabled(true);
   connect(le_nfrec, SIGNAL(textChanged(const QString &)), SLOT(update_nfrec(const QString &)));

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

   QVBoxLayout *vbl_top = new QVBoxLayout;

   vbl_top->addWidget(lbl_info);
   vbl_top->addSpacing(3);
   vbl_top->addWidget(cb_run_anaflex);
   vbl_top->addWidget(cb_instprofiles);
   vbl_top->addWidget(bg_run_mode);
   //   vbl_top->addWidget(cb_run_mode_1);
   //   vbl_top->addWidget(cb_run_mode_2);
   //   vbl_top->addWidget(cb_run_mode_3);
   //   vbl_top->addWidget(cb_run_mode_4);
   //   vbl_top->addWidget(cb_run_mode_9);

   QGridLayout *gl_ana_1 = new QGridLayout;

   int j = 0;
   gl_ana_1->addWidget(lbl_nfrec, j, 0);
   gl_ana_1->addWidget(le_nfrec, j, 1); j++;

   vbl_top->addSpacing(3);
   vbl_top->addLayout(gl_ana_1);

   QHBoxLayout *hbl_buttons = new QHBoxLayout;

   hbl_buttons->addWidget(pb_help);
   hbl_buttons->addWidget(pb_cancel);
   
   vbl_top->addSpacing(3);
   vbl_top->addLayout(hbl_buttons);

   QHBoxLayout *background = new QHBoxLayout(this);
   background->addLayout(vbl_top);
}

void US_Hydrodyn_Anaflex_Options::cancel()
{
   close();
}

void US_Hydrodyn_Anaflex_Options::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_anaflex_options.html");
}

void US_Hydrodyn_Anaflex_Options::closeEvent(QCloseEvent *e)
{
   *anaflex_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Anaflex_Options::set_run_anaflex()
{
   (*anaflex_options).run_anaflex = cb_run_anaflex->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_instprofiles()
{
   (*anaflex_options).instprofiles = cb_instprofiles->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_1()
{
   (*anaflex_options).run_mode_1 = cb_run_mode_1->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_2()
{
   (*anaflex_options).run_mode_2 = cb_run_mode_2->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_3()
{
   (*anaflex_options).run_mode_3 = cb_run_mode_3->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_4()
{
   (*anaflex_options).run_mode_4 = cb_run_mode_4->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_9()
{
   (*anaflex_options).run_mode_9 = cb_run_mode_9->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::update_nfrec(const QString &str)
{
   (*anaflex_options).nfrec = str.toInt();
   //   le_nfrec->setText(QString("").sprintf("%4.2f",(*hydro).nfrec));
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode( int val )
{
   (*anaflex_options).run_mode = val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}
