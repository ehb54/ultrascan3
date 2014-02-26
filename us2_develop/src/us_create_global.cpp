#include "../include/us_create_global.h"

US_CreateGlobal::US_CreateGlobal(QWidget *parent, const char* name) : QFrame(parent, name)
{
   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("Create Global Distribution"));
   unsigned int minHeight1 = 26, minHeight2 = 50;
   iterations = 0;

   distro.clear();

   lbl_info = new QLabel(tr("Combine Distributions for Global Fit"), this);
   lbl_info->setAlignment(AlignCenter|AlignVCenter);
   lbl_info->setFrameStyle(QFrame::WinPanel|Raised);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setMinimumHeight(minHeight2);
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lb_distro = new QListBox(this, "Distro");
   lb_distro->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lb_distro->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   connect(lb_distro, SIGNAL(selected(int)), SLOT(remove(int)));

   pb_add = new QPushButton(tr("Add Distributions"), this);
   Q_CHECK_PTR(pb_add);
   pb_add->setAutoDefault(false);
   pb_add->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_add->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_add->setMinimumHeight(minHeight1);
   connect(pb_add, SIGNAL(clicked()), SLOT(add()));

   pb_reset = new QPushButton(tr("Reset List"), this);
   Q_CHECK_PTR(pb_reset);
   pb_reset->setAutoDefault(false);
   pb_reset->setEnabled(false);
   pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_reset->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_reset->setMinimumHeight(minHeight1);
   connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setAutoDefault(false);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_help->setMinimumHeight(minHeight1);
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_quit = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_quit);
   pb_quit->setAutoDefault(false);
   pb_quit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_quit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_quit->setMinimumHeight(minHeight1);
   connect(pb_quit, SIGNAL(clicked()), SLOT(close()));

   pb_save = new QPushButton(tr("Save Global Distro"), this);
   Q_CHECK_PTR(pb_save);
   pb_save->setAutoDefault(false);
   pb_save->setEnabled(false);
   pb_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_save->setMinimumHeight(minHeight1);
   connect(pb_save, SIGNAL(clicked()), SLOT(save()));

   global_Xpos += 30;
   global_Ypos += 30;
   setup_GUI();
   move(global_Xpos, global_Ypos);
}

US_CreateGlobal::~US_CreateGlobal()
{
}

void US_CreateGlobal::setup_GUI()
{
   int j=0;

   QGridLayout * background = new QGridLayout(this, 3, 2, 2);
   background->addMultiCellWidget(lbl_info, j, j, 0, 1);
   j++;
   background->addMultiCellWidget(lb_distro, j, j, 0, 1);
   j++;
   background->addWidget(pb_add, j, 0);
   background->addWidget(pb_reset, j, 1);
   j++;
   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_save, j, 1);
   j++;
   background->addMultiCellWidget(pb_quit, j, j, 0, 1);
}

void US_CreateGlobal::closeEvent(QCloseEvent *e)
{
   e->accept();
   global_Xpos -= 30;
   global_Ypos -= 30;
}

void US_CreateGlobal::add()
{
   struct single_distro temp_distro;
   bool MonteCarlo = false, flag = false;
   unsigned int iter;
   temp_distro.line.clear();
   QString filename, filter = "*.fe_dis.* *.cofs_dis.* *.sa2d_dis.* *.ga_dis.* *.ga_mc_dis.* *.sa2d_mc_dis.* *.global_dis.* *.sa2d_mw_mc_dis.* *.ga_mw_mc_dis.* *.sa2d_mw_dis.* *.ga_mw_dis.*";
   QStringList sl = QFileDialog::getOpenFileNames(filter, USglobal->config_list.result_dir, 0, 0);
   for (QStringList::Iterator it=sl.begin(); it != sl.end(); it++)
   {
      filename = *it;
      QFileInfo fi(*it);
      if (filename.contains("_mc_", false))
      {
         MonteCarlo = true;
         flag = true;
      }
      temp_distro.name = fi.fileName();
      if (!filename.isEmpty())
      {
         lb_distro->insertItem(temp_distro.name, -1);
         pb_save->setEnabled(true);
         pb_reset->setEnabled(true);
         QFile f;
         f.setName(filename);
         if (f.open(IO_ReadOnly))
         {
            QTextStream ts(&f);
            ts.readLine();
            if (MonteCarlo) // we need to discard an additional line (the line with the number of MC iterations)
            {
               ts >> iter;
               cout << ts.readLine();
               iterations += iter;
               flag = true;
            }
            while (!ts.atEnd())
            {
               temp_distro.line.push_back(ts.readLine());
            }
            distro.push_back(temp_distro);
            if (!flag)
            {
               iterations ++;
            }
            f.close();
         }
      }
   }
}

void US_CreateGlobal::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/create_global.html");
}

void US_CreateGlobal::reset()
{
   distro.clear();
   lb_distro->clear();
   iterations = 0;
   pb_save->setEnabled(false);
   pb_reset->setEnabled(false);
}

void US_CreateGlobal::save()
{
   QString str;
   str = distro[distro.size()-1].name;
   int index = str.find(".");
   QString filename;
   if (iterations >= 5)
   {
      filename = str.left(index) + ".global_mc_dis.dat";
   }
   else
   {
      filename = str.left(index) + ".global_dis.dat";
   }
   OneLiner ol_descr(tr("Please confirm or\nmodify the file name\nfor the GA distribution:"));
   ol_descr.show();
   ol_descr.parameter1->setText(filename);
   if (ol_descr.exec())
   {
      filename = ol_descr.string;
   }
   else
   {
      return;
   }
   QFile f;
   f.setName(USglobal->config_list.result_dir + "/" + filename);
   if (f.open(IO_WriteOnly| IO_Translate))
   {
      QTextStream ts(&f);
      ts << "S_apparent\tS_20,W    \tD_apparent\tD_20,W    \tMW        \tFrequency\tf/f0(20,W)\n";
      if (iterations > 5)
      {
         ts << iterations << "\t\t# Number of Monte Carlo Iterations\n";
      }
      for (unsigned int i=0; i<distro.size(); i++)
      {
         for (unsigned int j=0; j<distro[i].line.size(); j++)
         {
            ts << distro[i].line[j] << endl;
         }
      }
      f.close();
   }
}

void US_CreateGlobal::remove(int val)
{
   lb_distro->removeItem(val);
   distro.erase(distro.begin() + val);
}
