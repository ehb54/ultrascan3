#include "../include/us_selectmodel3.h"

US_SelectModel3::US_SelectModel3(float *temp_mw_upperLimit, float *temp_mw_lowerLimit,
                                 unsigned int *temp_mw_slots, float *temp_model3_vbar, bool *temp_model3_vbar_flag, int temp_InvID,
                                 QWidget *p, const char *name) : QDialog(p, name, true)
{
   USglobal = new US_Config();

   QString str;
   mwUpperLimit   = temp_mw_upperLimit;
   mwLowerLimit   = temp_mw_lowerLimit;
   mwSlots         = temp_mw_slots;
   model3Vbar      = temp_model3_vbar;
   model3VbarFlag   = temp_model3_vbar_flag;
   InvID            = temp_InvID;
   
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("Fixed Molecular Weight Distribution"));

   lbl_info = new QLabel(tr("Please provide additional Information:"), this);
   Q_CHECK_PTR(lbl_info);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(AlignCenter|AlignVCenter);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setMinimumHeight(30);   
   lbl_info->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   
   lbl_mwSlots = new QLabel(tr("Number of MW Slots:"),this);
   lbl_mwSlots->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_mwSlots->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_mwSlots->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   le_mwSlots = new QLineEdit(this, "mwSlots");
   le_mwSlots->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_mwSlots->setText(" 100");
   le_mwSlots->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   *mwSlots = 100;
   connect(le_mwSlots, SIGNAL(textChanged(const QString &)), 
           SLOT(update_mwSlots(const QString &)));   


   lbl_mwLowerLimit = new QLabel(tr("Lower Limit of MW Range:"),this);
   lbl_mwLowerLimit->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_mwLowerLimit->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_mwLowerLimit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   le_mwLowerLimit = new QLineEdit(this, "mwLowerLimit");
   le_mwLowerLimit->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   *mwLowerLimit = 10000.0;
   str.sprintf(" %2.3e", *mwLowerLimit);
   le_mwLowerLimit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_mwLowerLimit->setText(str);
   connect(le_mwLowerLimit, SIGNAL(textChanged(const QString &)), 
           SLOT(update_mwLowerLimit(const QString &)));   


   lbl_mwUpperLimit = new QLabel(tr("Upper Limit of MW Range:"),this);
   lbl_mwUpperLimit->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_mwUpperLimit->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_mwUpperLimit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   le_mwUpperLimit = new QLineEdit(this, "mwUpperLimit");
   le_mwUpperLimit->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   *mwUpperLimit = 500000;
   str.sprintf(" %2.3e", *mwUpperLimit);
   le_mwUpperLimit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_mwUpperLimit->setText(str);
   connect(le_mwUpperLimit, SIGNAL(textChanged(const QString &)), 
           SLOT(update_mwUpperLimit(const QString &)));   

   lbl_commonVbar = new QLabel(tr("Use common vbar value?"),this);
   lbl_commonVbar->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_commonVbar->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_commonVbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   *model3VbarFlag = true;
   cb_commonVbar = new QCheckBox(this);
   Q_CHECK_PTR(cb_commonVbar);
   cb_commonVbar->setFixedSize(15,15);
   cb_commonVbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cb_commonVbar->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_commonVbar->setChecked(*model3VbarFlag);
   connect(cb_commonVbar, SIGNAL(clicked()), SLOT(change_commonVbar()));


   pb_vbar = new QPushButton(tr("Common vbar Value"), this);
   Q_CHECK_PTR(pb_vbar);
   pb_vbar->setAutoDefault(false);
   pb_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_vbar->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_vbar, SIGNAL(clicked()), SLOT(read_vbar()));
   
   le_vbar = new QLineEdit(this, "vbar");
   le_vbar->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   //   *model3Vbar = 0.72;
   str.sprintf(" %2.3e", *model3Vbar);
   le_vbar->setText(str);
   le_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   connect(le_vbar, SIGNAL(textChanged(const QString &)), 
           SLOT(update_vbar(const QString &)));   

   pb_ok = new QPushButton(tr("OK"), this);
   Q_CHECK_PTR(pb_ok);
   pb_ok->setAutoDefault(false);
   pb_ok->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_ok->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_ok, SIGNAL(clicked()), SLOT(check()));

   pb_cancel = new QPushButton(tr("Cancel"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setAutoDefault(false);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setAutoDefault(false);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   int j=0;
   QGridLayout * background = new QGridLayout(this,6,3,2);
   for (int i=0; i<6; i++)
   {
      background->setRowSpacing(i, 26);
   }
   background->addMultiCellWidget(lbl_info,j,j,0,2);
   j++;
   background->addMultiCellWidget(lbl_mwSlots,j,j,0,1);
   background->addWidget(le_mwSlots,j,2);
   j++;
   background->addMultiCellWidget(lbl_mwLowerLimit,j,j,0,1);
   background->addWidget(le_mwLowerLimit,j,2);
   j++;
   background->addMultiCellWidget(lbl_mwUpperLimit,j,j,0,1);
   background->addWidget(le_mwUpperLimit,j,2);
   j++;
   background->addMultiCellWidget(lbl_commonVbar,j,j,0,1);
   background->addWidget(cb_commonVbar,j,2);
   j++;
   background->addMultiCellWidget(pb_vbar,j,j,0,1);
   background->addWidget(le_vbar,j,2);
   j++;
   background->addWidget(pb_help,j,0);
   background->addWidget(pb_ok,j,1);
   background->addWidget(pb_cancel,j,2);

}

US_SelectModel3::US_SelectModel3(unsigned int *temp_mw_slots, QWidget *p, const char *name)
   : QDialog(p, name, true)
{
   USglobal = new US_Config();

   QString str;
   mwSlots   = temp_mw_slots;
   
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("Fixed Molecular Weight Distribution"));

   lbl_info = new QLabel(tr("Please provide additional Information:"), this);
   Q_CHECK_PTR(lbl_info);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(AlignCenter|AlignVCenter);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setMinimumHeight(30);
   lbl_info->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_mwSlots = new QLabel(tr("Number of MW Slots:"),this);
   lbl_mwSlots->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_mwSlots->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));

   lbl_mwSlots->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   le_mwSlots = new QLineEdit(this, "mwSlots");
   le_mwSlots->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_mwSlots->setText(" 4");
   *mwSlots = 4;
   connect(le_mwSlots, SIGNAL(textChanged(const QString &)), 
           SLOT(update_mwSlots(const QString &)));   

   pb_ok = new QPushButton(tr("OK"), this);
   Q_CHECK_PTR(pb_ok);
   pb_ok->setAutoDefault(false);
   pb_ok->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_ok->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_ok, SIGNAL(clicked()), SLOT(check()));

   pb_cancel = new QPushButton(tr("Cancel"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setAutoDefault(false);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setAutoDefault(false);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   
   int j=0;
   QGridLayout * background = new QGridLayout(this,3,3,2);
   for (int i=0; i<3; i++)
   {
      background->setRowSpacing(i, 26);
   }
   background->addMultiCellWidget(lbl_info,j,j,0,2);
   j++;
   background->addMultiCellWidget(lbl_mwSlots,j,j,0,1);
   background->addWidget(le_mwSlots,j,2);
   j++;
   background->addWidget(pb_help,j,0);
   background->addWidget(pb_ok,j,1);
   background->addWidget(pb_cancel,j,2);

}

US_SelectModel3::~US_SelectModel3()
{
}

void US_SelectModel3::closeEvent(QCloseEvent *e)
{
   e->accept();
   global_Xpos -= 30;
   global_Ypos -= 30;
}

void US_SelectModel3::cancel()
{
   *model3Vbar = (float) 0.72;
   *mwUpperLimit = 0.0;
   *mwLowerLimit = 0.0;
   *mwSlots = 0;
   reject();
}

void US_SelectModel3::check()
{
   accept();
}

void US_SelectModel3::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/equil2.html");
}

void US_SelectModel3::update_mwSlots(const QString &str)
{
   *mwSlots = str.toUInt();
}

void US_SelectModel3::update_mwUpperLimit(const QString &str)
{
   *mwUpperLimit = str.toFloat();
}

void US_SelectModel3::update_mwLowerLimit(const QString &str)
{
   *mwLowerLimit = str.toFloat();
}

void US_SelectModel3::change_commonVbar()
{
   if(*model3VbarFlag)
   {
      *model3VbarFlag = false;
   }
   else
   {
      *model3VbarFlag = true;
   }
}

void US_SelectModel3::update_vbar(const QString &str)
{
   *model3Vbar = str.toFloat();
}

void US_SelectModel3::read_vbar()
{
   float vbar = *model3Vbar;
   vbar_dlg = new US_Vbar_DB(20.0, &vbar, &(*model3Vbar),true, false, InvID, this);
   vbar_dlg->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   vbar_dlg->setCaption(tr("V-bar Calculation"));
   //   setGeometry(global_Xpos, global_Ypos, vbar_dlg->mySize_x, vbar_dlg->mySize_y);
   //   vbar_dlg->setGeometry(0, 0, vbar_dlg->mySize_x, vbar_dlg->mySize_y);
   vbar_dlg->show();
   if (vbar_dlg)
   {
      vbar_dlg->raise();
   }
   connect(vbar_dlg, SIGNAL(valueChanged(float, float)), this, SLOT(update_vbar_lbl(float, float)));
   //connect(vbar_dlg, SIGNAL(destroyed()), this, SLOT(resize_me()));
}

void US_SelectModel3::update_vbar_lbl(float, float val20)
{
   *model3Vbar = val20;
   QString str;
   str.sprintf("%6.4f", val20);
   le_vbar->setText(str);
}

void US_SelectModel3::resize_me()
{
   setGeometry(global_Xpos, global_Ypos, mySize_x, mySize_y);
}
