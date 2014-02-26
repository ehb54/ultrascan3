#include "../include/us_montecarlostats.h"

US_MonteCarloStats_W::US_MonteCarloStats_W(struct MonteCarloStats *temp_stats, bool *temp_stats_widget, QWidget *parent, const char *name) 
   : QFrame(parent, name, true)
{
   QString str;
   stats = temp_stats;
   stats_widget = temp_stats_widget;
   *stats_widget = true;   
   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   int border = 4;
   int xpos = border;
   int ypos = border;
   int spacing = 2;
   int buttonh = 26;
   int buttonw = 145;
   int span = 4 * buttonw + 3 * spacing;
   
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   str = tr("Monte Carlo Statistics for Parameter ") + (*stats).parameter_name;
   setCaption(tr("Monte Carlo Statistics"));
   
   lbl_banner1 = new QLabel(str, this);
   lbl_banner1->setAlignment(AlignCenter|AlignVCenter);
   lbl_banner1->setFrameStyle(QFrame::WinPanel|Raised);
   lbl_banner1->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_banner1->setGeometry(xpos, ypos, span, buttonh);
   lbl_banner1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   
   ypos += buttonh + 3 * spacing;
   

   lbl_high1 = new QLabel(tr(" Maximum Value:"), this);
   Q_CHECK_PTR(lbl_high1);
   lbl_high1->setAlignment(AlignLeft|AlignVCenter);
   lbl_high1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_high1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_high1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += buttonw + spacing;

   lbl_high2 = new QLabel("", this);
   Q_CHECK_PTR(lbl_high2);
   lbl_high2->setAlignment(AlignLeft|AlignVCenter);
   lbl_high2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_high2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_high2->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos += buttonw + spacing;
      
   lbl_low1 = new QLabel(tr(" Minimum Value:"), this);
   Q_CHECK_PTR(lbl_low1);
   lbl_low1->setAlignment(AlignLeft|AlignVCenter);
   lbl_low1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_low1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_low1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += buttonw + spacing;

   lbl_low2 = new QLabel("", this);
   Q_CHECK_PTR(lbl_low2);
   lbl_low2->setAlignment(AlignLeft|AlignVCenter);
   lbl_low2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_low2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_low2->setGeometry(xpos, ypos, buttonw, buttonh);
   
   ypos += buttonh + spacing;
   xpos = border;

   lbl_mean1 = new QLabel(tr(" Mean Value:"), this);
   Q_CHECK_PTR(lbl_mean1);
   lbl_mean1->setAlignment(AlignLeft|AlignVCenter);
   lbl_mean1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_mean1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_mean1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += buttonw + spacing;

   lbl_mean2 = new QLabel("", this);
   Q_CHECK_PTR(lbl_mean2);
   lbl_mean2->setAlignment(AlignLeft|AlignVCenter);
   lbl_mean2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_mean2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_mean2->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos += buttonw + spacing;
      
   lbl_median1 = new QLabel(tr(" Median Value:"), this);
   Q_CHECK_PTR(lbl_median1);
   lbl_median1->setAlignment(AlignLeft|AlignVCenter);
   lbl_median1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_median1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_median1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += buttonw + spacing;

   lbl_median2 = new QLabel("", this);
   Q_CHECK_PTR(lbl_median2);
   lbl_median2->setAlignment(AlignLeft|AlignVCenter);
   lbl_median2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_median2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_median2->setGeometry(xpos, ypos, buttonw, buttonh);
   
   ypos += buttonh + spacing;
   xpos = border;

   lbl_skew1 = new QLabel(tr(" Skew:"), this);
   Q_CHECK_PTR(lbl_skew1);
   lbl_skew1->setAlignment(AlignLeft|AlignVCenter);
   lbl_skew1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_skew1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_skew1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += buttonw + spacing;

   lbl_skew2 = new QLabel("", this);
   Q_CHECK_PTR(lbl_skew2);
   lbl_skew2->setAlignment(AlignLeft|AlignVCenter);
   lbl_skew2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_skew2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_skew2->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos += buttonw + spacing;
      
   lbl_kurtosis1 = new QLabel(tr(" Kurtosis:"), this);
   Q_CHECK_PTR(lbl_kurtosis1);
   lbl_kurtosis1->setAlignment(AlignLeft|AlignVCenter);
   lbl_kurtosis1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_kurtosis1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_kurtosis1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += buttonw + spacing;

   lbl_kurtosis2 = new QLabel("", this);
   Q_CHECK_PTR(lbl_kurtosis2);
   lbl_kurtosis2->setAlignment(AlignLeft|AlignVCenter);
   lbl_kurtosis2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_kurtosis2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_kurtosis2->setGeometry(xpos, ypos, buttonw, buttonh);
   
   ypos += buttonh + spacing;
   xpos = border;

   lbl_mode_low1 = new QLabel(tr(" Lower Limit of Mode:"), this);
   Q_CHECK_PTR(lbl_mode_low1);
   lbl_mode_low1->setAlignment(AlignLeft|AlignVCenter);
   lbl_mode_low1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_mode_low1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_mode_low1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += buttonw + spacing;

   lbl_mode_low2 = new QLabel("", this);
   Q_CHECK_PTR(lbl_mode_low2);
   lbl_mode_low2->setAlignment(AlignLeft|AlignVCenter);
   lbl_mode_low2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_mode_low2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_mode_low2->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos += buttonw + spacing;
      
   lbl_mode_high1 = new QLabel(tr(" Upper Limit of Mode:"), this);
   Q_CHECK_PTR(lbl_mode_high1);
   lbl_mode_high1->setAlignment(AlignLeft|AlignVCenter);
   lbl_mode_high1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_mode_high1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_mode_high1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += buttonw + spacing;

   lbl_mode_high2 = new QLabel("", this);
   Q_CHECK_PTR(lbl_mode_high2);
   lbl_mode_high2->setAlignment(AlignLeft|AlignVCenter);
   lbl_mode_high2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_mode_high2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_mode_high2->setGeometry(xpos, ypos, buttonw, buttonh);
   
   ypos += buttonh + spacing;
   xpos = border;

   lbl_std_dev1 = new QLabel(tr(" Standard Deviation:"), this);
   Q_CHECK_PTR(lbl_std_dev1);
   lbl_std_dev1->setAlignment(AlignLeft|AlignVCenter);
   lbl_std_dev1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_std_dev1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_std_dev1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += buttonw + spacing;

   lbl_std_dev2 = new QLabel("", this);
   Q_CHECK_PTR(lbl_std_dev2);
   lbl_std_dev2->setAlignment(AlignLeft|AlignVCenter);
   lbl_std_dev2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_std_dev2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_std_dev2->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos += buttonw + spacing;
      
   lbl_std_err1 = new QLabel(tr(" Standard Error:"), this);
   Q_CHECK_PTR(lbl_std_err1);
   lbl_std_err1->setAlignment(AlignLeft|AlignVCenter);
   lbl_std_err1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_std_err1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_std_err1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += buttonw + spacing;

   lbl_std_err2 = new QLabel("", this);
   Q_CHECK_PTR(lbl_std_err2);
   lbl_std_err2->setAlignment(AlignLeft|AlignVCenter);
   lbl_std_err2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_std_err2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_std_err2->setGeometry(xpos, ypos, buttonw, buttonh);
   
   ypos += buttonh + spacing;
   xpos = border;

   lbl_variance1 = new QLabel(tr(" Variance:"), this);
   Q_CHECK_PTR(lbl_variance1);
   lbl_variance1->setAlignment(AlignLeft|AlignVCenter);
   lbl_variance1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_variance1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_variance1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += buttonw + spacing;

   lbl_variance2 = new QLabel("", this);
   Q_CHECK_PTR(lbl_variance2);
   lbl_variance2->setAlignment(AlignLeft|AlignVCenter);
   lbl_variance2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_variance2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_variance2->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos += buttonw + spacing;
      
   lbl_correlation1 = new QLabel(tr(" Correlation Coeff.:"), this);
   Q_CHECK_PTR(lbl_correlation1);
   lbl_correlation1->setAlignment(AlignLeft|AlignVCenter);
   lbl_correlation1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_correlation1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_correlation1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += buttonw + spacing;

   lbl_correlation2 = new QLabel("", this);
   Q_CHECK_PTR(lbl_correlation2);
   lbl_correlation2->setAlignment(AlignLeft|AlignVCenter);
   lbl_correlation2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_correlation2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_correlation2->setGeometry(xpos, ypos, buttonw, buttonh);
   
   ypos += buttonh + spacing;
   xpos = border;

   lbl_area1 = new QLabel(tr(" Gaussian Area:"), this);
   Q_CHECK_PTR(lbl_area1);
   lbl_area1->setAlignment(AlignLeft|AlignVCenter);
   lbl_area1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_area1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_area1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += buttonw + spacing;

   lbl_area2 = new QLabel("", this);
   Q_CHECK_PTR(lbl_area2);
   lbl_area2->setAlignment(AlignLeft|AlignVCenter);
   lbl_area2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_area2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_area2->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos += buttonw + spacing;
      
   lbl_points1 = new QLabel(tr(" Monte Carlo Iterations:"), this);
   Q_CHECK_PTR(lbl_points1);
   lbl_points1->setAlignment(AlignLeft|AlignVCenter);
   lbl_points1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_points1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_points1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += buttonw + spacing;

   lbl_points2 = new QLabel("", this);
   Q_CHECK_PTR(lbl_points2);
   lbl_points2->setAlignment(AlignLeft|AlignVCenter);
   lbl_points2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_points2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_points2->setGeometry(xpos, ypos, buttonw, buttonh);

   ypos += buttonh + spacing;
   xpos = border;

   lbl_mode1 = new QLabel(tr(" Mode:"), this);
   Q_CHECK_PTR(lbl_mode1);
   lbl_mode1->setAlignment(AlignLeft|AlignVCenter);
   lbl_mode1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_mode1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_mode1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += buttonw + spacing;

   lbl_mode2 = new QLabel("", this);
   Q_CHECK_PTR(lbl_mode2);
   lbl_mode2->setAlignment(AlignLeft|AlignVCenter);
   lbl_mode2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_mode2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_mode2->setGeometry(xpos, ypos, buttonw, buttonh);

   xpos += buttonw + spacing;

   lbl_limits1 = new QLabel(tr(" 95% Limits:"), this);
   Q_CHECK_PTR(lbl_limits1);
   lbl_limits1->setAlignment(AlignLeft|AlignVCenter);
   lbl_limits1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_limits1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_limits1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += buttonw + spacing;

   lbl_limits2 = new QLabel("", this);
   Q_CHECK_PTR(lbl_limits2);
   lbl_limits2->setAlignment(AlignLeft|AlignVCenter);
   lbl_limits2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_limits2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_limits2->setGeometry(xpos, ypos, buttonw, buttonh);

   ypos += buttonh + spacing;
   xpos = border;

   lbl_95conf1 = new QLabel(tr(" 95% Confidence Limits:"), this);
   Q_CHECK_PTR(lbl_95conf1);
   lbl_95conf1->setAlignment(AlignLeft|AlignVCenter);
   lbl_95conf1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_95conf1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_95conf1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += buttonw + spacing;

   lbl_95conf2 = new QLabel("", this);
   Q_CHECK_PTR(lbl_95conf2);
   lbl_95conf2->setAlignment(AlignLeft|AlignVCenter);
   lbl_95conf2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_95conf2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_95conf2->setGeometry(xpos, ypos, 2 * buttonw + spacing, buttonh);

   xpos += 2 * buttonw + 2 * spacing;

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setAutoDefault(false);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_help->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));
   
   ypos += buttonh + spacing;
   xpos = border;

   lbl_99conf1 = new QLabel(tr(" 99% Confidence Limits:"), this);
   Q_CHECK_PTR(lbl_99conf1);
   lbl_99conf1->setAlignment(AlignLeft|AlignVCenter);
   lbl_99conf1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_99conf1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_99conf1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += buttonw + spacing;

   lbl_99conf2 = new QLabel("", this);
   Q_CHECK_PTR(lbl_99conf2);
   lbl_99conf2->setAlignment(AlignLeft|AlignVCenter);
   lbl_99conf2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_99conf2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_99conf2->setGeometry(xpos, ypos, 2 * buttonw + spacing, buttonh);

   xpos += 2 * buttonw + 2 * spacing;
   
   pb_quit = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_quit);
   pb_quit->setAutoDefault(false);
   pb_quit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_quit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_quit->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_quit, SIGNAL(clicked()), SLOT(quit()));

   ypos += buttonh + spacing + border;
   
   update_labels();

   global_Xpos += 30;
   global_Ypos += 30;

   setMinimumSize(span + 2 * border, ypos);
   setMaximumSize(span + 2 * border, ypos);
   setGeometry(global_Xpos, global_Ypos, span + 2 * border, ypos);
}


US_MonteCarloStats_W::~US_MonteCarloStats_W()
{
} 

void US_MonteCarloStats_W::closeEvent(QCloseEvent *e)
{
   global_Xpos -= 30;
   global_Ypos -= 30;
   *stats_widget = false;
   e->accept();
}

void US_MonteCarloStats_W::update_labels()
{
   QString str;
   float center, conf99low, conf99high, conf95low, conf95high;
   center = ((*stats).mode1 + (*stats).mode2)/2;
   conf99low = (*stats).mean - 2.576 * (*stats).std_deviation;
   conf99high = (*stats).mean + 2.576 * (*stats).std_deviation;
   conf95low = (*stats).mean - 1.96 * (*stats).std_deviation;
   conf95high = (*stats).mean + 1.96 * (*stats).std_deviation;

   str.sprintf(" %e", (*stats).high);
   lbl_high2->setText(str);
   str.sprintf(" %e", (*stats).low);
   lbl_low2->setText(str);
   str.sprintf(" %e", (*stats).mean);               // the average value of the distribution
   lbl_mean2->setText(str);
   str.sprintf(" %e", (*stats).median);            // the half-way value in the distribution
   lbl_median2->setText(str);
   str.sprintf(" %e", (*stats).skew);               // the skew of the distribution
   lbl_skew2->setText(str);
   str.sprintf(" %e", (*stats).mode1);            // the lower limit of the bin with the most datapoints in the distribution
   lbl_mode_low2->setText(str);
   str.sprintf(" %e", (*stats).mode2);            // the upper limit of the bin with the most datapoints in the distribution
   lbl_mode_high2->setText(str);
   str.sprintf(" %e", (*stats).kurtosis);         // the kurtosis of the distribution
   lbl_kurtosis2->setText(str);
   str.sprintf(" %e", (*stats).correlation);      // the correlation coefficient of the distribution
   lbl_correlation2->setText(str);
   str.sprintf(" %e", (*stats).std_deviation);      // the standard deviation of the distribution
   lbl_std_dev2->setText(str);
   str.sprintf(" %e", (*stats).std_error);         // the standard error of the distribution
   lbl_std_err2->setText(str);
   str.sprintf(" %e", (*stats).variance);
   lbl_variance2->setText(str);
   str.sprintf(" %e", (*stats).area);         // the standard error of the distribution
   lbl_area2->setText(str);
   str.sprintf(" %e", center); // the center of the mode (most likely value)
   lbl_mode2->setText(str);
   str.sprintf(" %d", (*stats).points);
   lbl_points2->setText(str);
   str.sprintf(tr(" %e (low),  %e (high)"), conf95low, conf95high);   // the standard error of the distribution
   lbl_95conf2->setText(str);
   str.sprintf(tr(" %e (low),  %e (high)"), conf99low, conf99high);   // the standard error of the distribution
   lbl_99conf2->setText(str);
   str = tr("Monte Carlo Statistics for Parameter ") + (*stats).parameter_name;
   lbl_banner1->setText(str);
   str.sprintf(" +%5.2e, -%5.2e", conf95high - center, center - conf95low); // the center of the mode (most likely value)
   lbl_limits2->setText(str);
}

void US_MonteCarloStats_W::quit()
{
   close();
}

void US_MonteCarloStats_W::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/monte_carlo_stats.html");
}

