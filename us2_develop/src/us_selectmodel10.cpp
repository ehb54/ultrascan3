#include "../include/us_selectmodel10.h"

US_SelectModel10::US_SelectModel10(float *temp_stoich1, QWidget *p, const char *name) : QDialog(p, name, true)
{
   int buttonh   = 26;
   int buttonw   = 102;
   int column1   = 208;
   int column2   = 100;
   int border   = 4;
   int spacing   = 2;
   int width   = 310;
   int xpos      = border;
   int ypos      = border;
   QString str;
   stoich1   = temp_stoich1;

   USglobal = new US_Config();


   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("Stoichiometry Selection - UltraScan Equilibrium Analysis"));

   lbl_info = new QLabel(tr("Please provide the Stoichiometry\nfor the Monomer- N-mer Equilibrium:"), this);
   Q_CHECK_PTR(lbl_info);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(AlignCenter|AlignVCenter);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setGeometry(xpos, ypos, width, 2 * buttonh + spacing);
   lbl_info->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   
   ypos += 2 + 2 * buttonh + 2 * spacing;
   xpos = border;

   lbl_stoich1 = new QLabel(tr("N-mer:"),this);
   lbl_stoich1->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_stoich1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_stoich1->setGeometry(xpos, ypos, column1, buttonh);
   lbl_stoich1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += column1 + spacing;

   le_stoich1 = new QLineEdit(this, "stoich1");
   le_stoich1->setGeometry(xpos, ypos, column2, buttonh);
   le_stoich1->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_stoich1->setText(" 2");
   *stoich1 = 2;
   connect(le_stoich1, SIGNAL(textChanged(const QString &)), 
           SLOT(update_stoich1(const QString &)));   

   ypos += 2 + buttonh + spacing;
   xpos = border;

   pb_ok = new QPushButton(tr("OK"), this);
   Q_CHECK_PTR(pb_ok);
   pb_ok->setAutoDefault(false);
   pb_ok->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_ok->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_ok->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_ok, SIGNAL(clicked()), SLOT(check()));

   xpos += buttonw + spacing;

   pb_cancel = new QPushButton(tr("Cancel"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setAutoDefault(false);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_cancel->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   xpos += buttonw + spacing;

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setAutoDefault(false);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_help->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   ypos += buttonh + border;

   global_Xpos += 30;
   global_Ypos += 30;
   
   setMinimumSize(width+8, ypos);
   setGeometry(global_Xpos, global_Ypos, width+8, ypos);
}

US_SelectModel10::~US_SelectModel10()
{
}

void US_SelectModel10::closeEvent(QCloseEvent *e)
{
   e->accept();
   global_Xpos -= 30;
   global_Ypos -= 30;
}

void US_SelectModel10::cancel()
{
   *stoich1 = 2;
   reject();
}

void US_SelectModel10::check()
{
   accept();
}

void US_SelectModel10::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/equil2.html");
}

void US_SelectModel10::update_stoich1(const QString &str)
{
   *stoich1 = str.toFloat();
}
