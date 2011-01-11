#include "../include/us_meniscus_dlg.h"

US_MeniscusDialog::US_MeniscusDialog(float *meniscus, QString runid, QWidget *p, const char *name) : QDialog(p, name)
{
   this->meniscus = meniscus;
   this->runid = runid;
   QString str;
   USglobal=new US_Config();
   current_meniscus = 1;
   for (int i=0; i<8; i++)
   {
      meniscus_ok1[i] = true;
      meniscus_ok2[i] = true;
   }
   
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   
   setCaption(tr("Meniscus Dialog"));

   lbl_info = new QLabel(tr("  Meniscus Update Dialog for: "), this);
   Q_CHECK_PTR(lbl_info);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(AlignCenter|AlignVCenter);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_runid = new QLabel(runid, this);
   Q_CHECK_PTR(lbl_runid);
   lbl_runid->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_runid->setAlignment(AlignCenter|AlignVCenter);
   lbl_runid->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_runid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_meniscus1 = new QLabel("  Meniscus Cell 1: ",this);
   lbl_meniscus1->setAlignment(AlignCenter|AlignVCenter);
   //   lbl_meniscus1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_meniscus1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_meniscus1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_meniscus2 = new QLabel("  Meniscus Cell 2: ",this);
   lbl_meniscus2->setAlignment(AlignCenter|AlignVCenter);
   //   lbl_meniscus2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_meniscus2->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_meniscus2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_meniscus3 = new QLabel("  Meniscus Cell 3: ",this);
   lbl_meniscus3->setAlignment(AlignCenter|AlignVCenter);
   //   lbl_meniscus3->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_meniscus3->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_meniscus3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_meniscus4 = new QLabel("  Meniscus Cell 4: ",this);
   lbl_meniscus4->setAlignment(AlignCenter|AlignVCenter);
   //   lbl_meniscus4->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_meniscus4->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_meniscus4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_meniscus5 = new QLabel("  Meniscus Cell 5: ",this);
   lbl_meniscus5->setAlignment(AlignCenter|AlignVCenter);
   //   lbl_meniscus5->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_meniscus5->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_meniscus5->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_meniscus6 = new QLabel("  Meniscus Cell 6: ",this);
   lbl_meniscus6->setAlignment(AlignCenter|AlignVCenter);
   //   lbl_meniscus6->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_meniscus6->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_meniscus6->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_meniscus7 = new QLabel("  Meniscus Cell 7: ",this);
   lbl_meniscus7->setAlignment(AlignCenter|AlignVCenter);
   //   lbl_meniscus7->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_meniscus7->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_meniscus7->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_meniscus8 = new QLabel("  Meniscus Cell 8: ",this);
   lbl_meniscus8->setAlignment(AlignCenter|AlignVCenter);
   //   lbl_meniscus8->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_meniscus8->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_meniscus8->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_meniscus1 = new QLineEdit( this, "meniscus1" );
   le_meniscus1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_meniscus1->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   if ((int) meniscus[0] == 0)
   {
      le_meniscus1->setEnabled(false);
      le_meniscus1->setReadOnly(true);
   }
   le_meniscus1->setText(str.sprintf("%8.5f", meniscus[0]));
   connect(le_meniscus1, SIGNAL(textChanged(const QString &)), SLOT(update_meniscus1(const QString &)));

   le_meniscus2 = new QLineEdit( this, "meniscus2" );
   le_meniscus2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_meniscus2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   if ((int) meniscus[1] == 0)
   {
      le_meniscus2->setEnabled(false);
      le_meniscus2->setReadOnly(true);
   }
   le_meniscus2->setText(str.sprintf("%8.5f", meniscus[1]));
   connect(le_meniscus2, SIGNAL(textChanged(const QString &)), SLOT(update_meniscus2(const QString &)));

   le_meniscus3 = new QLineEdit( this, "meniscus3" );
   le_meniscus3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_meniscus3->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   if ((int) meniscus[2] == 0)
   {
      le_meniscus3->setEnabled(false);
      le_meniscus3->setReadOnly(true);
   }
   le_meniscus3->setText(str.sprintf("%8.5f", meniscus[2]));
   connect(le_meniscus3, SIGNAL(textChanged(const QString &)), SLOT(update_meniscus3(const QString &)));

   le_meniscus4 = new QLineEdit( this, "meniscus4" );
   le_meniscus4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_meniscus4->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   if ((int) meniscus[3] == 0)
   {
      le_meniscus4->setEnabled(false);
      le_meniscus4->setReadOnly(true);
   }
   le_meniscus4->setText(str.sprintf("%8.5f", meniscus[3]));
   connect(le_meniscus4, SIGNAL(textChanged(const QString &)), SLOT(update_meniscus4(const QString &)));

   le_meniscus5 = new QLineEdit( this, "meniscus5" );
   le_meniscus5->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_meniscus5->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   if ((int) meniscus[4] == 0)
   {
      le_meniscus5->setEnabled(false);
      le_meniscus5->setReadOnly(true);
   }
   le_meniscus5->setText(str.sprintf("%8.5f", meniscus[4]));
   connect(le_meniscus5, SIGNAL(textChanged(const QString &)), SLOT(update_meniscus5(const QString &)));

   le_meniscus6 = new QLineEdit( this, "meniscus6" );
   le_meniscus6->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_meniscus6->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   if ((int) meniscus[5] == 0)
   {
      le_meniscus6->setEnabled(false);
      le_meniscus6->setReadOnly(true);
   }
   le_meniscus6->setText(str.sprintf("%8.5f", meniscus[5]));
   connect(le_meniscus6, SIGNAL(textChanged(const QString &)), SLOT(update_meniscus6(const QString &)));

   le_meniscus7 = new QLineEdit( this, "meniscus7" );
   le_meniscus7->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_meniscus7->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   if ((int) meniscus[6] == 0)
   {
      le_meniscus7->setEnabled(false);
      le_meniscus7->setReadOnly(true);
   }
   le_meniscus7->setText(str.sprintf("%8.5f", meniscus[6]));
   connect(le_meniscus7, SIGNAL(textChanged(const QString &)), SLOT(update_meniscus7(const QString &)));

   le_meniscus8 = new QLineEdit( this, "meniscus8" );
   le_meniscus8->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_meniscus8->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   if ((int) meniscus[7] == 0)
   {
      le_meniscus8->setEnabled(false);
      le_meniscus8->setReadOnly(true);
   }
   le_meniscus8->setText(str.sprintf("%8.5f", meniscus[7]));
   connect(le_meniscus8, SIGNAL(textChanged(const QString &)), SLOT(update_meniscus8(const QString &)));

   pb_save = new QPushButton(tr("Accept"), this);
   Q_CHECK_PTR(pb_save);
   pb_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_save, SIGNAL(clicked()), SLOT(ok()));

   pb_cancel = new QPushButton(tr("Cancel"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
   
   setup_GUI();
}

US_MeniscusDialog::~US_MeniscusDialog()
{
}

void US_MeniscusDialog::cancel()
{
   close();
}

void US_MeniscusDialog::ok()
{
   for (int i=0; i<8; i++)
   {
      current_meniscus = i+1;
      if (!check_meniscus()) return;
   }
   accept();
}

void US_MeniscusDialog::setup_GUI()
{
   QBoxLayout *topbox=new QVBoxLayout(this, 3);
   topbox->addWidget(lbl_info);
   topbox->addWidget(lbl_runid);

   int rows = 9, columns = 2, spacing = 2, j=0;
   QGridLayout *controlGrid = new QGridLayout(topbox, rows, columns, spacing);
   for (int i=0; i<rows; i++)
   {
      controlGrid->setRowSpacing(i, 26);
   }
   controlGrid->setColStretch(0, 0);
   controlGrid->setColStretch(1, 1);

   controlGrid->addWidget(lbl_meniscus1, j, 0);
   controlGrid->addWidget(le_meniscus1, j, 1);
   j++;

   controlGrid->addWidget(lbl_meniscus2, j, 0);
   controlGrid->addWidget(le_meniscus2, j, 1);
   j++;

   controlGrid->addWidget(lbl_meniscus3, j, 0);
   controlGrid->addWidget(le_meniscus3, j, 1);
   j++;

   controlGrid->addWidget(lbl_meniscus4, j, 0);
   controlGrid->addWidget(le_meniscus4, j, 1);
   j++;

   controlGrid->addWidget(lbl_meniscus5, j, 0);
   controlGrid->addWidget(le_meniscus5, j, 1);
   j++;

   controlGrid->addWidget(lbl_meniscus6, j, 0);
   controlGrid->addWidget(le_meniscus6, j, 1);
   j++;

   controlGrid->addWidget(lbl_meniscus7, j, 0);
   controlGrid->addWidget(le_meniscus7, j, 1);
   j++;

   controlGrid->addWidget(lbl_meniscus8, j, 0);
   controlGrid->addWidget(le_meniscus8, j, 1);
   j++;

   controlGrid->addWidget(pb_cancel, j, 0);
   controlGrid->addWidget(pb_save, j, 1);
}

void US_MeniscusDialog::update_meniscus1(const QString &str)
{
   bool ok;
   QString str2 = str.stripWhiteSpace();
   current_meniscus = 1;
   meniscus_ok1[0] = true;
   meniscus_ok2[0] = true;
   
   meniscus[0] = str2.toFloat(&ok);
   if (!ok)
   {
      meniscus_ok1[0] = false;
   }
   else if (meniscus[0] > 7.2 | meniscus[0] < 5.75)
   {
      meniscus_ok2[0] = false;
   }
}

void US_MeniscusDialog::update_meniscus2(const QString &str)
{
   bool ok;
   QString str2 = str.stripWhiteSpace();
   current_meniscus = 2;
   meniscus_ok1[1] = true;
   meniscus_ok2[1] = true;
   
   meniscus[1] = str2.toFloat(&ok);
   if (!ok)
   {
      meniscus_ok1[1] = false;
   }
   else if (meniscus[1] > 7.2 | meniscus[1] < 5.75)
   {
      meniscus_ok2[1] = false;
   }
}

void US_MeniscusDialog::update_meniscus3(const QString &str)
{
   bool ok;
   QString str2 = str.stripWhiteSpace();
   current_meniscus = 3;
   meniscus_ok1[2] = true;
   meniscus_ok2[2] = true;
   
   meniscus[2] = str2.toFloat(&ok);
   if (!ok)
   {
      meniscus_ok1[2] = false;
   }
   else if (meniscus[2] > 7.2 | meniscus[2] < 5.75)
   {
      meniscus_ok2[2] = false;
   }
}

void US_MeniscusDialog::update_meniscus4(const QString &str)
{
   bool ok;
   QString str2 = str.stripWhiteSpace();
   current_meniscus = 4;
   meniscus_ok1[3] = true;
   meniscus_ok2[3] = true;
   
   meniscus[3] = str2.toFloat(&ok);
   if (!ok)
   {
      meniscus_ok1[3] = false;
   }
   else if (meniscus[3] > 7.2 | meniscus[3] < 5.75)
   {
      meniscus_ok2[3] = false;
   }
}

void US_MeniscusDialog::update_meniscus5(const QString &str)
{
   bool ok;
   QString str2 = str.stripWhiteSpace();
   current_meniscus = 5;
   meniscus_ok1[4] = true;
   meniscus_ok2[4] = true;
   meniscus[4] = str2.toFloat(&ok);
   if (!ok)
   {
      meniscus_ok1[4] = false;
   }
   else if (meniscus[4] > 7.2 | meniscus[4] < 5.75)
   {
      meniscus_ok2[4] = false;
   }
}

void US_MeniscusDialog::update_meniscus6(const QString &str)
{
   bool ok;
   QString str2 = str.stripWhiteSpace();
   current_meniscus = 6;
   meniscus_ok1[5] = true;
   meniscus_ok2[5] = true;
   
   meniscus[5] = str2.toFloat(&ok);
   if (!ok)
   {
      meniscus_ok1[5] = false;
   }
   else if (meniscus[5] > 7.2 | meniscus[5] < 5.75)
   {
      meniscus_ok2[5] = false;
   }
}

void US_MeniscusDialog::update_meniscus7(const QString &str)
{
   bool ok;
   QString str2 = str.stripWhiteSpace();
   current_meniscus = 7;
   meniscus_ok1[6] = true;
   meniscus_ok2[6] = true;
   
   meniscus[6] = str2.toFloat(&ok);
   if (!ok)
   {
      meniscus_ok1[6] = false;
   }
   else if (meniscus[6] > 7.2 | meniscus[6] < 5.75)
   {
      meniscus_ok2[6] = false;
   }
}

void US_MeniscusDialog::update_meniscus8(const QString &str)
{
   bool ok;
   QString str2 = str.stripWhiteSpace();
   current_meniscus = 8;
   meniscus_ok1[7] = true;
   meniscus_ok2[7] = true;
   
   meniscus[7] = str2.toFloat(&ok);
   if (!ok)
   {
      meniscus_ok1[7] = false;
   }
   else if (meniscus[7] > 7.2 | meniscus[7] < 5.75)
   {
      meniscus_ok2[7] = false;
   }
}

bool US_MeniscusDialog::check_meniscus()
{
   bool ok=true;
   if (!meniscus_ok1[current_meniscus-1] && meniscus_ok2[current_meniscus-1])
   {
      QString str;
      QMessageBox::message(tr("Attention: Data Input Error:"), tr("Invalid meniscus value for meniscus ") + str.sprintf("%d", current_meniscus) + tr("\nPlease try again!"));
      ok=false;
   }
   if (!meniscus_ok2[current_meniscus-1] && meniscus_ok1[current_meniscus-1])
   {
      QString str;
      QMessageBox::message(tr("Attention: Data Input Error:"), tr("Invalid range for meniscus ") + str.sprintf("%d", current_meniscus) + tr("\nPlease try again!"));
      ok=false;
   }
   if (!meniscus_ok2[current_meniscus-1] && !meniscus_ok1[current_meniscus-1])
   {
      QString str;
      QMessageBox::message(tr("Attention: Data Input Error:"), tr("Invalid value and range for meniscus ") + str.sprintf("%d", current_meniscus) + tr("\nPlease try again!"));
      ok=false;
   }
   return ok;
}
