#include "../include/us_fefit.h"

US_Fefit_FitControl::US_Fefit_FitControl(bool *temp_constrained_fit, bool *temp_fitting_widget, 
                                         bool *temp_autoconverge, QWidget *parent, const char *name) : QFrame(parent, name)
{
   USglobal = new US_Config();
   constrained_fit = temp_constrained_fit;
   fitting_widget = temp_fitting_widget;
   autoconverge = temp_autoconverge;
   *fitting_widget = true;
   int xpos = 4;
   int ypos = 5;
   int buttonh = 26;
   int column1 = 140;
   int column2 = 93;
   int column3 = 93;
   int span = column1 + column2 + column3 + spacing;
   
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));

   lbl_header = new QLabel("", this);
   lbl_header->setAlignment(AlignCenter|AlignVCenter);
   lbl_header->setFrameStyle(QFrame::WinPanel|Raised);
   lbl_header->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_header->setGeometry(xpos, ypos, span, 2 * buttonh + spacing);
   lbl_header->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   xpos = 4;
   ypos += 2 * buttonh + spacing + 15;

   lbl_iteration1 = new QLabel(tr("Iteration Number:"),this);
   lbl_iteration1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_iteration1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_iteration1->setGeometry(xpos, ypos, column1, buttonh);
   lbl_iteration1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += column1 + spacing;

   lbl_iteration2 = new QLabel("0",this);
   lbl_iteration2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_iteration2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_iteration2->setGeometry(xpos, ypos, column2, buttonh);
   lbl_iteration2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   
   xpos += column2 + spacing;

   lbl_best = new QLabel(tr("Best sofar:"),this);
   lbl_best->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_best->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_best->setGeometry(xpos, ypos, column3, buttonh);
   lbl_best->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos = 4;
   ypos += buttonh + spacing;

   lbl_variance1 = new QLabel(tr("Variance:"),this);
   lbl_variance1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_variance1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_variance1->setGeometry(xpos, ypos, column1, buttonh);
   lbl_variance1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += column1 + spacing;

   lbl_variance2 = new QLabel("0",this);
   lbl_variance2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_variance2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_variance2->setGeometry(xpos, ypos, column2, buttonh);
   lbl_variance2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   xpos += column2 + spacing;

   lbl_variance3 = new QLabel("0",this);
   lbl_variance3->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_variance3->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_variance3->setGeometry(xpos, ypos, column3, buttonh);
   lbl_variance3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   xpos = 4;
   ypos += buttonh + spacing;

   lbl_stddev1 = new QLabel(tr("Std. Deviation:"),this);
   lbl_stddev1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_stddev1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_stddev1->setGeometry(xpos, ypos, column1, buttonh);
   lbl_stddev1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += column1 + spacing;

   lbl_stddev2 = new QLabel("0",this);
   lbl_stddev2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_stddev2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_stddev2->setGeometry(xpos, ypos, column2, buttonh);
   lbl_stddev2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   xpos = 4;
   ypos += buttonh + spacing;

   lbl_difference1 = new QLabel(tr("Improvement:"),this);
   lbl_difference1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_difference1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_difference1->setGeometry(xpos, ypos, column1, buttonh);
   lbl_difference1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += column1 + spacing;

   lbl_difference2 = new QLabel("0",this);
   lbl_difference2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_difference2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_difference2->setGeometry(xpos, ypos, column2, buttonh);
   lbl_difference2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   xpos = 4;
   ypos += buttonh + spacing;

   lbl_evaluations1 = new QLabel(tr("Function Evaluations:"),this);
   lbl_evaluations1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_evaluations1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_evaluations1->setGeometry(xpos, ypos, column1, buttonh);
   lbl_evaluations1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += column1 + spacing;

   lbl_evaluations2 = new QLabel("0",this);
   lbl_evaluations2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_evaluations2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_evaluations2->setGeometry(xpos, ypos, column2, buttonh);
   lbl_evaluations2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   xpos = 4;
   ypos += buttonh + spacing;

   lbl_progress = new QLabel(tr("Current Iteration:"),this);
   lbl_progress->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_progress->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_progress->setGeometry(xpos, ypos, column1, buttonh);
   lbl_progress->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += column1 + spacing;

   pgb_progress = new QProgressBar(this, "iteration progress");
   pgb_progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   pgb_progress->setGeometry(xpos, ypos, column2 + column3 + spacing, buttonh);
   
   xpos = 4;
   ypos += buttonh + spacing;

   lbl_status1 = new QLabel(tr("Status:"),this);
   lbl_status1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_status1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_status1->setGeometry(xpos, ypos, column1, buttonh);
   lbl_status1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += column1 + spacing;

   lbl_status2 = new QLabel(tr("Waiting for Input..."),this);
   lbl_status2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_status2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_status2->setGeometry(xpos, ypos, column2 + column3 + spacing, buttonh);
   lbl_status2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   xpos = 4;
   ypos += buttonh + spacing;

   lbl_blank = new QLabel("",this);
   lbl_blank->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_blank->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_blank->setGeometry(xpos, ypos, column1, buttonh);

   bt_constrained = new QRadioButton(this);
   bt_constrained->setText(tr("Constrained Fit"));
   bt_constrained->setGeometry(xpos+10, ypos+4, column1-10, 18);
   bt_constrained->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1)); //, QFont::Bold));
   bt_constrained->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   if (*constrained_fit)
   {
      bt_constrained->setChecked(true);
   }
   else
   {
      bt_constrained->setChecked(false);
   }
   connect(bt_constrained, SIGNAL(clicked()), SLOT(update_constrained_button()));

   xpos += column1 + spacing;

   lbl_status3 = new QLabel("",this);
   lbl_status3->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_status3->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_status3->setGeometry(xpos, ypos, column2 + column3 + spacing, buttonh);
   lbl_status3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   xpos = 4;
   ypos += buttonh + spacing;

   lbl_blank = new QLabel("",this);
   lbl_blank->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_blank->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_blank->setGeometry(xpos, ypos, column1, buttonh);

   bt_unconstrained = new QRadioButton(this);
   bt_unconstrained->setText(tr("Unconstrained Fit"));
   bt_unconstrained->setGeometry(xpos+10, ypos+4, column1-10, 18);
   bt_unconstrained->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1)); //, QFont::Bold));
   bt_unconstrained->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   if (*constrained_fit)
   {
      bt_unconstrained->setChecked(false);
   }
   else
   {
      bt_unconstrained->setChecked(true);
   }
   connect(bt_unconstrained, SIGNAL(clicked()), SLOT(update_unconstrained_button()));

   xpos += column1 + spacing;

   lbl_status4 = new QLabel("",this);
   lbl_status4->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_status4->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_status4->setGeometry(xpos, ypos, column2 + column3 + spacing, buttonh);
   lbl_status4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   xpos = 4;
   ypos += buttonh + spacing;

   lbl_blank = new QLabel("",this);
   lbl_blank->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_blank->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_blank->setGeometry(xpos, ypos, column1, buttonh);

   bt_autoconverge = new QRadioButton(this);
   bt_autoconverge->setText(tr("Autoconverge"));
   bt_autoconverge->setGeometry(xpos+10, ypos+4, column1-10, 18);
   bt_autoconverge->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1)); //, QFont::Bold));
   bt_autoconverge->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   if (autoconverge)
   {
      bt_autoconverge->setChecked(true);
   }
   connect(bt_autoconverge, SIGNAL(clicked()), SLOT(update_autoconverge_button()));

   xpos = 4;
   ypos += buttonh + 15;

   int buttonw = (unsigned int)((span - (4.0 * spacing)) / 5.0);

   pb_fit = new QPushButton(tr("Fit"), this);
   Q_CHECK_PTR(pb_fit);
   pb_fit->setAutoDefault(false);
   pb_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_fit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_fit->setGeometry(xpos, ypos, buttonw, buttonh);

   xpos += buttonw + spacing;

   pb_pause = new QPushButton(tr("Pause"), this);
   Q_CHECK_PTR(pb_pause);
   pb_pause->setAutoDefault(false);
   pb_pause->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_pause->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_pause->setGeometry(xpos, ypos, buttonw, buttonh);

   xpos += buttonw + spacing;

   pb_resume = new QPushButton(tr("Resume"), this);
   Q_CHECK_PTR(pb_resume);
   pb_resume->setAutoDefault(false);
   pb_resume->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_resume->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_resume->setGeometry(xpos, ypos, buttonw+2, buttonh);

   xpos += buttonw + spacing + 2;

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setAutoDefault(false);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_help->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));
   xpos += buttonw + spacing;

   pb_cancel = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setAutoDefault(false);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_cancel->setGeometry(xpos, ypos, buttonw+1, buttonh);

   ypos += buttonh;
   global_Xpos += 30;
   global_Ypos += 30;
   //cout << "x: " << span + 10 << ", y: " << ypos + 4 << endl;
   setGeometry(global_Xpos, global_Ypos, span + 10, ypos + 4); 
   setMinimumSize(span + 10, ypos + 4);
}

US_Fefit_FitControl::~US_Fefit_FitControl()
{

}

void US_Fefit_FitControl::closeEvent(QCloseEvent *e)
{
   e->accept();
   global_Xpos -= 30;
   global_Ypos -= 30;
   *fitting_widget = false;
}

void US_Fefit_FitControl::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/finite_fit_ctrl.html");
}

void US_Fefit_FitControl::update_constrained_button()
{
   bt_unconstrained->setChecked(false);
   *constrained_fit = true;
}

void US_Fefit_FitControl::update_unconstrained_button()
{
   bt_constrained->setChecked(false);
   *constrained_fit = false;
}

void US_Fefit_FitControl::update_autoconverge_button()
{
   *autoconverge = bt_autoconverge->isChecked();
}

void US_Fefit_FitControl::cancel()
{
   close();
}

