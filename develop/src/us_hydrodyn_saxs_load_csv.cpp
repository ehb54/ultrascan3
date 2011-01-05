#include "../include/us_hydrodyn_saxs_load_csv.h"

US_Hydrodyn_Saxs_Load_Csv::US_Hydrodyn_Saxs_Load_Csv(
                                                     QString msg,
                                                     QStringList *qsl_names,
                                                     QStringList *qsl_sel_names,
                                                     QWidget *p,
                                                     const char *name
                                                     ) : QDialog(p, name)
{
   this->msg = msg;
   this->qsl_names = qsl_names;
   this->qsl_sel_names = qsl_sel_names;

   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption("Load CSV style SAXS results");
   setupGUI();
   global_Xpos = 200;
   global_Ypos = 150;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_Saxs_Load_Csv::~US_Hydrodyn_Saxs_Load_Csv()
{
}

void US_Hydrodyn_Saxs_Load_Csv::setupGUI()
{
   int minWidth1 = 600;
   int minHeight1 = 30;
   int minHeight2 = 30;

   lbl_info = new QLabel(msg, this);
   lbl_info->setAlignment(AlignCenter|AlignVCenter);
   lbl_info->setMinimumHeight(minHeight2);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   lb_names = new QListBox(this);
   lb_names->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lb_names->setMinimumHeight(minHeight1 * 15);
   lb_names->setMinimumWidth(minWidth1);
   lb_names->insertStringList(*qsl_names);
   lb_names->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lb_names->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   lb_names->setSelectionMode(QListBox::Multi);
   lb_names->setEnabled(true);
   connect(lb_names, SIGNAL(selectionChanged()), SLOT(update_selected()));

   pb_select_all = new QPushButton(tr("Select All"), this);
   pb_select_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_select_all->setMinimumHeight(minHeight1);
   pb_select_all->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_select_all, SIGNAL(clicked()), SLOT(select_all()));

   pb_cancel = new QPushButton(tr("Cancel"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton("Help", this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight2);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_ok = new QPushButton(tr("Close"), this);
   pb_ok->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_ok->setMinimumHeight(minHeight1);
   pb_ok->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_ok, SIGNAL(clicked()), SLOT(ok()));

   // build layout
   // left box / possible

   QHBoxLayout *hbl_bottom = new QHBoxLayout;
   hbl_bottom->addSpacing(5);
   hbl_bottom->addWidget(pb_select_all);
   hbl_bottom->addSpacing(5);
   hbl_bottom->addWidget(pb_cancel);
   hbl_bottom->addSpacing(5);
   hbl_bottom->addWidget(pb_help);
   hbl_bottom->addSpacing(5);
   hbl_bottom->addWidget(pb_ok);
   hbl_bottom->addSpacing(5);

   QVBoxLayout *background = new QVBoxLayout(this);
   background->addSpacing(5);
   background->addWidget(lbl_info);
   background->addSpacing(5);
   background->addWidget(lb_names);
   background->addSpacing(5);
   background->addLayout(hbl_bottom);
   background->addSpacing(5);
}

void US_Hydrodyn_Saxs_Load_Csv::select_all()
{
   // if there are any, not selected, select all
   // if all are selected, unselect all
   bool select_all = false;

   // are any unselected ?
   for ( int i = 0; i < lb_names->numRows(); i++ )
   {
      if ( !lb_names->isSelected(i) )
      {
         select_all = true;
         break;
      }
   }

   for ( int i = 0; i < lb_names->numRows(); i++ )
   {
      lb_names->setSelected(i, select_all);
   }
}

void US_Hydrodyn_Saxs_Load_Csv::ok()
{
   close();
}

void US_Hydrodyn_Saxs_Load_Csv::cancel()
{
   qsl_sel_names->clear();
   close();
}

void US_Hydrodyn_Saxs_Load_Csv::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_saxs_load_csv.html");
}

void US_Hydrodyn_Saxs_Load_Csv::update_selected()
{
   qsl_sel_names->clear();
   for ( int i = 0; i < lb_names->numRows(); i++ )
   {
      if ( lb_names->isSelected(i) )
      {
         *qsl_sel_names << lb_names->text(i);
      }
   }
}
