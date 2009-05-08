#include "../include/us_kirkwood.h"

US_Kirkwood::US_Kirkwood(QWidget *parent, const char *name) 
   : QFrame( parent, name)
{
   USglobal = new US_Config();

   int xpos=4, ypos=4;
   int buttonw = 150;
   int buttonh = 26;
   int spacing = 2;
   sample1 = true;
   sample2 = false;
   sample3 = 35000;
   sample4 = 0;
   
   setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));

   lbl_banner = new QLabel(tr(" Sample Settings:"),this);
   lbl_banner->setFrameStyle(QFrame::WinPanel|Raised);
   lbl_banner->setAlignment(AlignCenter|AlignVCenter);
   lbl_banner->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
   lbl_banner->setGeometry(xpos, ypos, buttonw * 2 + spacing, buttonh);
   lbl_banner->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos = 4;
   ypos += buttonh + 2 * spacing;

   lbl_sample1 = new QLabel(tr(" Sample Label 1:"),this);
   lbl_sample1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_sample1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_sample1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_sample1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));

   xpos += buttonw + spacing;
   
   lbl_sample2 = new QLabel(tr(" Sample Label 2:"),this);
   lbl_sample2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_sample2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_sample2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_sample2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));

   xpos = 4;
   ypos += buttonh + spacing;
   
   lbl_sample3 = new QLabel(tr(" Label Sample 3"),this);
   lbl_sample3->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_sample3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_sample3->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_sample3->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));

   xpos += buttonw + spacing;
   
   ct_sample3 = new QwtCounter(this);
   ct_sample3->setRange(500, 1e7, 100);
   ct_sample3->setValue(sample3);
   ct_sample3->setNumButtons(3);
   ct_sample3->setGeometry(xpos, ypos, buttonw, buttonh);
   ct_sample3->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   ct_sample3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   //   connect(ct_sample3, SIGNAL(buttonReleased(double)), SLOT(update_sample3(double)));
   connect(ct_sample3, SIGNAL(valueChanged(double)), SLOT(update_sample3(double)));

   ypos += buttonh + spacing;
   xpos = 4;

   lbl_blank1 = new QLabel("",this);
   lbl_blank1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_blank1->setGeometry(xpos, ypos, (unsigned int) 4 * buttonw / 3, buttonh);
   lbl_blank1->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_blank1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   bt_sample1 = new QRadioButton(this);
   bt_sample1->setText(tr("Sample 1"));
   bt_sample1->setGeometry(xpos+10, ypos+4, (unsigned int) 2 * buttonw / 3 - 12, buttonh-8);
   bt_sample1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   bt_sample1->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   bt_sample1->setChecked(sample1);
   connect(bt_sample1, SIGNAL(clicked()), SLOT(update_sample1()));

   xpos += 2 * buttonw / 3;

   bt_sample2 = new QRadioButton(this);
   bt_sample2->setText(tr("Sample 2"));
   bt_sample2->setGeometry(xpos+10, ypos+4, (unsigned int) 2 * buttonw / 3 - 11, buttonh-8);
   bt_sample2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   bt_sample2->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   bt_sample2->setChecked(sample2);
   connect(bt_sample2, SIGNAL(clicked()), SLOT(update_sample2()));

   ypos += buttonh + spacing;
   xpos = 4;

   lbl_sample4 = new QLabel(tr(" Label Sample 4:"),this);
   lbl_sample4->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_sample4->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_sample4->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_sample4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos += buttonw + spacing;
   
   le_sample4 = new QLineEdit(this);
   le_sample4->setGeometry(xpos, ypos, buttonw, buttonh);
   le_sample4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_sample4->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_sample4->setText( " 120");
   connect(le_sample4, SIGNAL(textChanged(const QString &)), SLOT(update_sample4(const QString &)));

   xpos = 4;
   ypos += buttonh + spacing;

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setAutoDefault(false);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_help->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   xpos += buttonw + spacing;

   pb_close = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_close);
   pb_close->setAutoDefault(false);
   pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_close->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_close, SIGNAL(clicked()), SLOT(quit()));

   setFixedSize(2 * buttonw + 8 + spacing, ypos + buttonh + 4);

   global_Xpos += 30;
   global_Ypos += 30;
   
   move(global_Xpos, global_Ypos);
}

US_Kirkwood::~US_Kirkwood()
{
}

void US_Kirkwood::closeEvent(QCloseEvent *e)
{
   e->accept();
   global_Xpos -= 30;
   global_Ypos -= 30;
}

void US_Kirkwood::update_sample1()
{
   bt_sample2->setChecked(false);
   sample1 = true;
}

void US_Kirkwood::update_sample2()
{
   bt_sample1->setChecked(false);
   sample1 = false;
}

void US_Kirkwood::update_sample3(double val)
{
   sample3 = (float) val;
}

void US_Kirkwood::update_sample4(const QString &val)
{
   sample4 = val.toFloat();
}

void US_Kirkwood::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/sample.html");
}

void US_Kirkwood::quit()
{
   close();
}


