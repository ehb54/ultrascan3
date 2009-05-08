#include "../include/us_selectplot.h"

US_SelectPlot::US_SelectPlot(bool *temp_plotFlag, bool *temp_print_bw, bool *temp_print_inv, 
                             bool temp_change, QString temp_plot1_name, QString temp_plot2_name, QWidget *p, const char *name)
   : QDialog(p, name, true)
{
   int buttonh   = 26;
   int column1   = 200;
   int border   = 4;
   int spacing   = 2;
   int width   = 242;
   int xpos      = border;
   int ypos      = border;
   USglobal = new US_Config();

   plotFlag = temp_plotFlag;
   print_bw = temp_print_bw;
   print_inv = temp_print_inv;
   change = temp_change;
   plot1_name = temp_plot1_name;
   plot2_name = temp_plot2_name;
   if (*print_bw)
   {
      *print_inv = true;
   }
   
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("Select a Plot:"));

   lbl_info = new QLabel(tr("Please Select:"), this);
   Q_CHECK_PTR(lbl_info);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(AlignCenter|AlignVCenter);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setGeometry(xpos, ypos, width, buttonh);
   lbl_info->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   
   ypos += buttonh + 2 * spacing;
   xpos = border;

   lbl_analysis = new QLabel(tr(plot1_name),this);
   lbl_analysis->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_analysis->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_analysis->setGeometry(xpos, ypos, column1, buttonh);
   lbl_analysis->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += column1 + 14;

   cb_analysis = new QCheckBox(this);
   Q_CHECK_PTR(cb_analysis);
   cb_analysis->setGeometry(xpos , ypos+5, 14, 14);
   cb_analysis->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cb_analysis->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_analysis->setChecked(*plotFlag);
   connect(cb_analysis, SIGNAL(clicked()), SLOT(change_analysis()));
   if (!change)
   {
      cb_analysis->setEnabled(false);
   }

   ypos += buttonh + spacing;
   xpos = border;

   lbl_edit = new QLabel(tr(plot2_name),this);
   lbl_edit->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_edit->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_edit->setGeometry(xpos, ypos, column1, buttonh);
   lbl_edit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += column1 + 14;

   cb_edit = new QCheckBox(this);
   Q_CHECK_PTR(cb_edit);
   cb_edit->setGeometry(xpos , ypos+5, 14, 14);
   cb_edit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cb_edit->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_edit->setChecked(!(*plotFlag));
   connect(cb_edit, SIGNAL(clicked()), SLOT(change_edit()));

   ypos += buttonh + spacing;
   xpos = border;

   lbl_bw = new QLabel(tr("Print in Black & White:"),this);
   lbl_bw->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_bw->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_bw->setGeometry(xpos, ypos, column1, buttonh);
   lbl_bw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += column1 + 14;

   cb_bw = new QCheckBox(this);
   Q_CHECK_PTR(cb_bw);
   cb_bw->setGeometry(xpos , ypos+5, 14, 14);
   cb_bw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cb_bw->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_bw->setChecked(*print_bw);
   connect(cb_bw, SIGNAL(clicked()), SLOT(change_bw()));

   ypos += buttonh + spacing;
   xpos = border;

   lbl_color = new QLabel(tr("Print in Color:"),this);
   lbl_color->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_color->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_color->setGeometry(xpos, ypos, column1, buttonh);
   lbl_color->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += column1 + 14;

   cb_color = new QCheckBox(this);
   Q_CHECK_PTR(cb_color);
   cb_color->setGeometry(xpos , ypos+5, 14, 14);
   cb_color->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cb_color->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_color->setChecked(!(*print_bw));
   connect(cb_color, SIGNAL(clicked()), SLOT(change_color()));

   ypos += buttonh + spacing;
   xpos = border;

   lbl_inv = new QLabel(tr("Print inverse Colors:"),this);
   lbl_inv->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_inv->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_inv->setGeometry(xpos, ypos, column1, buttonh);
   lbl_inv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += column1 + 14;

   cb_inv = new QCheckBox(this);
   Q_CHECK_PTR(cb_inv);
   cb_inv->setGeometry(xpos , ypos+5, 14, 14);
   cb_inv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cb_inv->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_inv->setChecked(*print_inv);
   if (*print_bw)
   {
      cb_inv->setEnabled(false);
   }
   connect(cb_inv, SIGNAL(clicked()), SLOT(change_inv()));

   ypos += buttonh + spacing;
   xpos = border;

   pb_ok = new QPushButton(tr("Continue"), this);
   Q_CHECK_PTR(pb_ok);
   pb_ok->setAutoDefault(false);
   pb_ok->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_ok->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_ok->setGeometry(xpos, ypos, width, buttonh);
   connect(pb_ok, SIGNAL(clicked()), SLOT(check()));

   ypos += buttonh + border;

   global_Xpos += 30;
   global_Ypos += 30;
   
   setMinimumSize(width+8, ypos);
   setGeometry(global_Xpos, global_Ypos, width+8, ypos);
}

US_SelectPlot::US_SelectPlot(bool *temp_print_bw, bool *temp_print_inv, 
                             QWidget *p, const char *name) : QDialog(p, name, true)
{
   int buttonh   = 26;
   int column1   = 200;
   int border   = 4;
   int spacing   = 2;
   int width   = 242;
   int xpos      = border;
   int ypos      = border;
   USglobal = new US_Config();

   print_bw = temp_print_bw;
   print_inv = temp_print_inv;
   if (*print_bw)
   {
      *print_inv = true;
   }
   
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("Select a Plot:"));

   lbl_info = new QLabel(tr("Please Select:"), this);
   Q_CHECK_PTR(lbl_info);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(AlignCenter|AlignVCenter);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setGeometry(xpos, ypos, width, buttonh);
   lbl_info->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   
   ypos += buttonh + 2 * spacing;
   xpos = border;

   lbl_bw = new QLabel(tr("Print in Black & White:"),this);
   lbl_bw->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_bw->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_bw->setGeometry(xpos, ypos, column1, buttonh);
   lbl_bw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += column1 + 14;

   cb_bw = new QCheckBox(this);
   Q_CHECK_PTR(cb_bw);
   cb_bw->setGeometry(xpos , ypos+5, 14, 14);
   cb_bw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cb_bw->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_bw->setChecked(*print_bw);
   connect(cb_bw, SIGNAL(clicked()), SLOT(change_bw()));

   ypos += buttonh + spacing;
   xpos = border;

   lbl_color = new QLabel(tr("Print in Color:"),this);
   lbl_color->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_color->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_color->setGeometry(xpos, ypos, column1, buttonh);
   lbl_color->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += column1 + 14;

   cb_color = new QCheckBox(this);
   Q_CHECK_PTR(cb_color);
   cb_color->setGeometry(xpos , ypos+5, 14, 14);
   cb_color->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cb_color->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_color->setChecked(!(*print_bw));
   connect(cb_color, SIGNAL(clicked()), SLOT(change_color()));

   ypos += buttonh + spacing;
   xpos = border;

   lbl_inv = new QLabel(tr("Print inverse Colors:"),this);
   lbl_inv->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_inv->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_inv->setGeometry(xpos, ypos, column1, buttonh);
   lbl_inv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += column1 + 14;

   cb_inv = new QCheckBox(this);
   Q_CHECK_PTR(cb_inv);
   cb_inv->setGeometry(xpos , ypos+5, 14, 14);
   cb_inv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cb_inv->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_inv->setChecked(*print_inv);
   if (*print_bw)
   {
      cb_inv->setEnabled(false);
   }
   connect(cb_inv, SIGNAL(clicked()), SLOT(change_inv()));

   ypos += buttonh + spacing;
   xpos = border;

   pb_ok = new QPushButton(tr("Continue"), this);
   Q_CHECK_PTR(pb_ok);
   pb_ok->setAutoDefault(false);
   pb_ok->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_ok->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_ok->setGeometry(xpos, ypos, width, buttonh);
   connect(pb_ok, SIGNAL(clicked()), SLOT(check()));

   ypos += buttonh + border;

   global_Xpos += 30;
   global_Ypos += 30;
   
   setMinimumSize(width+8, ypos);
   setGeometry(global_Xpos, global_Ypos, width+8, ypos);
}

US_SelectPlot::~US_SelectPlot()
{
}

void US_SelectPlot::closeEvent(QCloseEvent *e)
{
   e->accept();
   global_Xpos -= 30;
   global_Ypos -= 30;
}

void US_SelectPlot::check()
{
   accept();
}

void US_SelectPlot::change_analysis()
{
   *plotFlag = true;
   cb_analysis->setChecked(*plotFlag);
   cb_edit->setChecked(!(*plotFlag));
}

void US_SelectPlot::change_edit()
{
   *plotFlag = false;
   cb_analysis->setChecked(*plotFlag);
   cb_edit->setChecked(!(*plotFlag));
}

void US_SelectPlot::change_bw()
{
   *print_bw = true;
   cb_bw->setChecked(*print_bw);
   cb_color->setChecked(!(*print_bw));
   cb_inv->setChecked(true);
   cb_inv->setEnabled(false);
}

void US_SelectPlot::change_color()
{
   *print_bw = false;
   cb_bw->setChecked(*print_bw);
   cb_color->setChecked(!(*print_bw));
   cb_inv->setEnabled(true);
}

void US_SelectPlot::change_inv()
{
   if (*print_inv)
   {
      *print_inv = false;
   }
   else
   {
      *print_inv = true;
   }
   cb_inv->setChecked(*print_inv);
}
