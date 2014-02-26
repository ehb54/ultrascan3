#include "../include/us_exclude_profile.h"

US_ExcludeProfile::US_ExcludeProfile(unsigned int totalScans, QWidget *p, const char *name) : QFrame(p, name)
{
   USglobal = new US_Config();
   this->totalScans = totalScans;
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("Scan Exclusion Profile Editor"));

   start = 1;
   stop = totalScans;
   include = 1;
   excluded = 0;

   QString str;

   lbl_info = new QLabel(tr("Please Create a Scan Exclusion Profile:"), this);
   Q_CHECK_PTR(lbl_info);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(AlignCenter|AlignVCenter);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setMinimumHeight(30);
   lbl_info->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_scanStart = new QLabel(tr("Start Profile at Scan:"),this);
   lbl_scanStart->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_scanStart->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_scanStart->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_scanStop = new QLabel(tr("Stop Profile at Scan:"),this);
   lbl_scanStop->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_scanStop->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_scanStop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_scanInclude = new QLabel(tr("Include every nth Scan:"),this);
   lbl_scanInclude->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_scanInclude->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_scanInclude->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_totalScans1 = new QLabel(tr("Remaining Scans:"),this);
   lbl_totalScans1->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_totalScans1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_totalScans1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_totalScans2 = new QLabel(tr(str.sprintf("%d scans", totalScans)), this);
   lbl_totalScans2->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_totalScans2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lbl_totalScans2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   lbl_excludedScans1 = new QLabel(tr("Excluded Scans:"),this);
   lbl_excludedScans1->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_excludedScans1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_excludedScans1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_excludedScans2 = new QLabel(tr(str.sprintf("%d scans", excluded)), this);
   lbl_excludedScans2->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_excludedScans2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lbl_excludedScans2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
         
   cnt_scanStart = new QwtCounter(this);
   cnt_scanStart->setNumButtons(3);
   cnt_scanStart->setRange(start, totalScans, 1);
   cnt_scanStart->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cnt_scanStart->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cnt_scanStart->setValue(start);
   connect(cnt_scanStart, SIGNAL(valueChanged(double)), SLOT(update_scanStart(double)));

   cnt_scanStop = new QwtCounter(this);
   cnt_scanStop->setNumButtons(3);
   cnt_scanStop->setRange(stop, totalScans, 1);
   cnt_scanStop->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cnt_scanStop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cnt_scanStop->setValue(stop);
   connect(cnt_scanStop, SIGNAL(valueChanged(double)), SLOT(update_scanStop(double)));

   cnt_scanInclude = new QwtCounter(this);
   cnt_scanInclude->setNumButtons(3);
   cnt_scanInclude->setRange(1, totalScans, 1);
   cnt_scanInclude->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cnt_scanInclude->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cnt_scanInclude->setValue(include);
   connect(cnt_scanInclude, SIGNAL(valueChanged(double)), SLOT(update_scanInclude(double)));

   pb_accept = new QPushButton(tr("Process this Profile"), this);
   Q_CHECK_PTR(pb_accept);
   pb_accept->setAutoDefault(false);
   pb_accept->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_accept->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_accept, SIGNAL(clicked()), SLOT(accept()));

   pb_close = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_close);
   pb_close->setAutoDefault(false);
   pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_close, SIGNAL(clicked()), SLOT(close()));

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
   QGridLayout * background = new QGridLayout(this, 6, 2, 2);
   for (int i=0; i<6; i++)
   {
      background->setRowSpacing(i, 26);
   }
   background->addMultiCellWidget(lbl_info, j, j, 0, 1);
   j++;
   background->addWidget(lbl_scanStart, j, 0);
   background->addWidget(cnt_scanStart, j, 1);
   j++;
   background->addWidget(lbl_scanStop, j, 0);
   background->addWidget(cnt_scanStop, j, 1);
   j++;
   background->addWidget(lbl_scanInclude, j, 0);
   background->addWidget(cnt_scanInclude, j, 1);
   j++;
   background->addWidget(lbl_totalScans1, j, 0);
   background->addWidget(lbl_totalScans2, j, 1);
   j++;
   background->addWidget(lbl_excludedScans1, j, 0);
   background->addWidget(lbl_excludedScans2, j, 1);
   j++;
   background->addWidget(pb_accept, j, 0);
   background->addWidget(pb_cancel, j, 1);
   j++;
   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_close, j, 1);
}

US_ExcludeProfile::~US_ExcludeProfile()
{
}

void US_ExcludeProfile::closeEvent(QCloseEvent *e)
{
   e->accept();
   global_Xpos -= 30;
   global_Ypos -= 30;
}

void US_ExcludeProfile::cancel()
{
   close();
}

void US_ExcludeProfile::accept()
{
   if (totalScans - excluded < 10) //at least 10 scans
   {
      QMessageBox::message("Attention:", "Your profile should include\nat least 10 scans...");
      return;
   }
   emit final_exclude_profile(update_exclude_list());
   close();
}

vector <bool> US_ExcludeProfile::update_exclude_list()
{
   vector <bool> include_flag;
   excluded = 0;
   include_flag.resize(totalScans);
   unsigned int count=include;
   scans_remaining = 0;
   for (unsigned int i=0; i<totalScans; i++)
   {
      if (i < start-1 || i >= stop-1)
      {
         include_flag[i] = true;
         scans_remaining ++;
      }
      else
      {
         if (count == include)
         {
            count = 0;
            include_flag[i] = true;
            scans_remaining ++;
         }
         else
         {
            include_flag[i] = false;
            excluded ++;
         }
         count++;
      }
   }
   return(include_flag);
}

void US_ExcludeProfile::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/exclude_profile.html");
}

void US_ExcludeProfile::update_scanStart(double dval)
{
   start = (unsigned int) dval;
   if (stop <= start)
   {
      stop = start;
      stop ++;
      cnt_scanStop->setValue(stop);
   }
   cnt_scanStop->setRange(start+1, totalScans, 1);
   emit update_exclude_profile(update_exclude_list());
   lbl_totalScans2->setText(tr(str.sprintf("%d scans", scans_remaining)));
   lbl_excludedScans2->setText(tr(str.sprintf("%d scans", excluded)));
}

void US_ExcludeProfile::update_scanStop(double dval)
{
   stop = (unsigned int) dval;
   emit update_exclude_profile(update_exclude_list());
   lbl_totalScans2->setText(tr(str.sprintf("%d scans", scans_remaining)));
   lbl_excludedScans2->setText(tr(str.sprintf("%d scans", excluded)));
}

void US_ExcludeProfile::update_scanInclude(double dval)
{
   include = (unsigned int) dval;
   emit update_exclude_profile(update_exclude_list());
   lbl_totalScans2->setText(tr(str.sprintf("%d scans", scans_remaining)));
   lbl_excludedScans2->setText(tr(str.sprintf("%d scans", excluded)));
}

