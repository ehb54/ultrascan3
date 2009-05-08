#include "../include/us_beowulf.h"

US_Beowulf::US_Beowulf(QString temp_fileName, int temp_experiment, bool *temp_beowulf_widget, QWidget *p, const char *name) : QDialog(p, name, true)
{
   int buttonh   = 26;
   int buttonw   = 170;
   int column1   = 200;
   int border   = 4;
   int spacing   = 2;
   int span      = spacing + buttonw + column1;
   int xpos      = border;
   int ypos      = border;
   shell = "rsh";
   
   fileName = temp_fileName;
   experiment = temp_experiment;
   beowulf_widget = temp_beowulf_widget;
   *beowulf_widget = true;
   USglobal = new US_Config();
   QString str;

   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("Beowulf Control Panel"));

   lbl_banner = new QLabel(tr("Beowulf Control Panel:"), this);
   Q_CHECK_PTR(lbl_banner);
   lbl_banner->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_banner->setAlignment(AlignCenter|AlignVCenter);
   lbl_banner->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_banner->setGeometry(xpos, ypos, span, buttonh + 2 * spacing);
   lbl_banner->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   
   ypos += buttonh + 4 * spacing;
   xpos += spacing + buttonw;
   
   lbl_info1 = new QLabel(tr("Host Information:"), this);
   Q_CHECK_PTR(lbl_info1);
   lbl_info1->setAlignment(AlignCenter|AlignVCenter);
   lbl_info1->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info1->setGeometry(xpos, ypos, column1, buttonh);
   lbl_info1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   
   ypos += buttonh + spacing;
   xpos = border;

   pb_add = new QPushButton(tr("Add Host"), this);
   Q_CHECK_PTR(pb_add);
   pb_add->setAutoDefault(false);
   pb_add->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_add->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_add->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_add, SIGNAL(clicked()), SLOT(add_host()));

   xpos += buttonw + spacing;

   le_edit_host = new QLineEdit(this, "edit_host");
   le_edit_host->setGeometry(xpos, ypos, column1, buttonh);
   le_edit_host->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_edit_host, SIGNAL(textChanged(const QString &)), 
           SLOT(edit_host(const QString &)));   

   ypos += buttonh + spacing;
   xpos = border;

   pb_delete = new QPushButton(tr("Delete Host"), this);
   Q_CHECK_PTR(pb_delete);
   pb_delete->setAutoDefault(false);
   pb_delete->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_delete->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_delete->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_delete, SIGNAL(clicked()), SLOT(delete_host()));

   xpos += spacing + buttonw;

   lb_hosts = new QListBox(this, "Hosts");
   lb_hosts->setGeometry(xpos, ypos, column1, 6 * buttonh + 5 * spacing);
   lb_hosts->setSelected(0, true);
   lb_hosts->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lb_hosts->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   QFile f(USglobal->config_list.root_dir + "/hosts.beowulf");
   if (f.exists())
   {
      f.open(IO_ReadOnly);
      QTextStream ts (&f);
      while (!ts.eof())
      {
         str = ts.readLine();
         lb_hosts->insertItem(str);
      }
      f.close();
   }
   lb_hosts->setSelected(0, true);
   lb_hosts->setCurrentItem(0);
   connect(lb_hosts, SIGNAL(selected(int)), SLOT(show_host(int)));

   ypos += buttonh + spacing;
   xpos = border;

   pb_save = new QPushButton(tr("Save Setup"), this);
   Q_CHECK_PTR(pb_save);
   pb_save->setAutoDefault(false);
   pb_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_save->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_save, SIGNAL(clicked()), SLOT(save()));

   ypos += buttonh + spacing;
   xpos = border;

   pb_run = new QPushButton(tr("Start Run"), this);
   Q_CHECK_PTR(pb_run);
   pb_run->setAutoDefault(false);
   pb_run->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_run->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_run->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_run, SIGNAL(clicked()), SLOT(run()));

   ypos += buttonh + spacing;
   xpos = border;

   pb_merge = new QPushButton(tr("Merge Output Files"), this);
   Q_CHECK_PTR(pb_merge);
   pb_merge->setAutoDefault(false);
   pb_merge->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_merge->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_merge->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_merge, SIGNAL(clicked()), SLOT(mergeAll()));

   ypos += buttonh + spacing;
   xpos = border;

   pb_kill = new QPushButton(tr("Stop all MC Processes"), this);
   Q_CHECK_PTR(pb_kill);
   pb_kill->setAutoDefault(false);
   pb_kill->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_kill->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_kill->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_kill, SIGNAL(clicked()), SLOT(killAll()));

   ypos += buttonh + spacing;
   xpos = border;

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setAutoDefault(false);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_help->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   ypos += buttonh + spacing;
   xpos = border;

   pb_quit = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_quit);
   pb_quit->setAutoDefault(false);
   pb_quit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_quit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_quit->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_quit, SIGNAL(clicked()), SLOT(quit()));
   
   xpos += buttonw + spacing;

   cb_ssh = new QCheckBox(this);
   Q_CHECK_PTR(cb_ssh);
   cb_ssh->setGeometry(xpos + 5 , ypos + 5, 14, 14);
   cb_ssh->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cb_ssh->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_ssh->setChecked(true);
   connect(cb_ssh, SIGNAL(clicked()), SLOT(change_ssh()));

   xpos += 24;

   lbl_ssh = new QLabel(tr("Use ssh"),this);
   lbl_ssh->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_ssh->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_ssh->setGeometry(xpos, ypos, 76, buttonh);
   lbl_ssh->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   xpos += 76;

   cb_rsh = new QCheckBox(this);
   Q_CHECK_PTR(cb_rsh);
   cb_rsh->setGeometry(xpos + 5 , ypos + 5, 14, 14);
   cb_rsh->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cb_rsh->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_rsh->setChecked(false);
   connect(cb_rsh, SIGNAL(clicked()), SLOT(change_rsh()));

   xpos += 24;

   lbl_rsh = new QLabel(tr("Use rsh"),this);
   lbl_rsh->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_rsh->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_rsh->setGeometry(xpos, ypos, 76, buttonh);
   lbl_rsh->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   ypos += buttonh + spacing;
   xpos = border;
   
   lbl_display = new QLabel(tr(" Use X-Display:"),this);
   lbl_display->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_display->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_display->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_display->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));   
   lbl_display->setAlignment(AlignCenter|AlignVCenter);

   xpos += buttonw + spacing;

   le_display = new QLineEdit(this, "display");
   le_display->setGeometry(xpos, ypos, column1, buttonh);
   le_display->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_display, SIGNAL(textChanged(const QString &)), 
           SLOT(change_display(const QString &)));
   get_localhost();
   le_display->setText(Xdisplay);

   ypos += buttonh + border;

   global_Xpos += 30;
   global_Ypos += 30;
   
   setMinimumSize(span+8, ypos);
   setGeometry(global_Xpos, global_Ypos, span+8, ypos);
   change_rsh();
}

US_Beowulf::~US_Beowulf()
{
}

void US_Beowulf::closeEvent(QCloseEvent *e)
{
   *beowulf_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Beowulf::quit()
{
   close();
}

void US_Beowulf::mergeAll()
{
   QString str;
   str = "rm " + fileName;
   system(str);
   str = "cat " + fileName + ".* >> " + fileName;
   system(str);
}

void US_Beowulf::killAll()
{
   QString str;
   
   for (unsigned int i=0; i<lb_hosts->count(); i++)
   {
      switch(experiment)
      {
      case 1:
         {
            str = shell + " " + lb_hosts->text(i) + " \"killall -9 us_equilmc_t\"";
            cout << str.latin1() << endl;
            break;
         }
      case 2:
         {
            str = shell + " " + lb_hosts->text(i) + " \"killall -9 us_velocdudmc_t\"";
            cout << str.latin1() << endl;
            break;
         }
      case 3:
         {
            str = shell + " " + lb_hosts->text(i) + " \"killall -9 us_velocadmc_t\"";
            cout << str.latin1() << endl;
            break;
         }
      }
      system(str);
   }
}

void US_Beowulf::change_ssh()
{
   shell = "ssh";
   cb_rsh->setChecked(false);
   cb_ssh->setChecked(true);
}

void US_Beowulf::change_rsh()
{
   shell = "rsh";
   cb_rsh->setChecked(true);
   cb_ssh->setChecked(false);
}

void US_Beowulf::change_display(const QString &str)
{
   Xdisplay = str;
}

void US_Beowulf::add_host()
{
   if (le_edit_host->text().stripWhiteSpace() != "")
   {
      lb_hosts->insertItem(le_edit_host->text().stripWhiteSpace());
      lb_hosts->sort(true);
   }
}

void US_Beowulf::show_host(int item)
{
   le_edit_host->setText(lb_hosts->text(item));
}

void US_Beowulf::delete_host()
{
   for (unsigned int i=0; i<lb_hosts->count(); i++)
   {
      if(le_edit_host->text() == lb_hosts->text(i))
      {
         lb_hosts->removeItem(i);
         break;
      }
   }
}

void US_Beowulf::edit_host(const QString &hostname)
{
   currentHost = hostname;
}

bool US_Beowulf::get_localhost()
{
   QString str, display;
   //   char *host = "localhost", *display;
   //   host = new char[1000];
   //   display = new char[1000];
   //   host = getenv("HOSTNAME");
   //   Xdisplay = (QString) getenv("HOSTNAME");
   Xdisplay += (QString) getenv("DISPLAY");
   if (Xdisplay == NULL)
   {
      QMessageBox::message(tr("UltraScan Error"),   tr("Attention:\n\nUltraScan was unable to determine\n"
                                                       "the display environment variable. Please\n"
                                                       "set the DISPLAY environment variable\n"
                                                       "before starting UltraScan and try again."));
      return(false);
   }
   /*
     if (host == NULL)
     {
     QMessageBox::message(tr("UltraScan Error"),   tr("Attention:\n\nUltraScan was unable to determine\n"
     "the hostname environment variable. Please\n"
     "set the HOSTNAME environment variable\n"
     "before starting UltraScan and try again."));
     return(false);
     }
   */
   return(true);
}

void US_Beowulf::run()
{
   QString str;
   
   if (shell == "ssh")
   {
      shell += " -X";
   }
   str = shell + " <host_name> " + tr(" \"export DISPLAY=") + Xdisplay + "; ";
   switch(experiment)
   {
   case 0:
      {
         break;
      }
   case 1:
      {
         //         str += "source ~/.bash_login; " + USglobal->config_list.system_dir + "/bin/us_equilmc_t " 
         str += "us_equilmc_t " 
            + USglobal->config_list.root_dir + "/beowulf.start\" < /dev/null > /tmp/hostname.stdout 2> /tmp/hostname.stderr &";
         break;
      }
   case 2:
      {
         str += USglobal->config_list.system_dir + "/bin/us_velocdudmc_t " 
            + USglobal->config_list.root_dir + "/beowulf.start\" &";
         break;
      }
   case 3:
      {
         str += USglobal->config_list.system_dir + "/bin/us_velocadmc_t " 
            + USglobal->config_list.root_dir + "/beowulf.start\" &";
         break;
      }
   }
   QMessageBox::message(tr("UltraScan Information:"), 
                        tr("The following system command will be issued to the Beowulf cluster:\n") + str);
   for (unsigned int i=0; i<lb_hosts->count(); i++)
   {

      str = shell + " " + lb_hosts->text(i) + tr(" \"export DISPLAY=") + Xdisplay + "; ";
      switch(experiment)
      {
      case 0:
         {
            break;
         }
      case 1:
         {
            //            str += "source ~/.bash_login; " + USglobal->config_list.system_dir + "/bin/us_equilmc_t " 
            str += "us_equilmc_t " +  USglobal->config_list.root_dir 
               + "/beowulf.start\" < /dev/null > /tmp/" + lb_hosts->text(i) 
               + ".stderr 2> /tmp/" + lb_hosts->text(i) + ".stderr &";
            break;
         }
      case 2:
         {
            str += USglobal->config_list.system_dir + "/bin/us_velocdudmc_t " 
               + USglobal->config_list.root_dir + "/beowulf.start\" &";
            break;
         }
      case 3:
         {
            str += USglobal->config_list.system_dir + "/bin/us_velocadmc_t " 
               + USglobal->config_list.root_dir + "/beowulf.start\" &";
            break;
         }
      }
      system(str);
      cout << str << endl;
   }
}

void US_Beowulf::save()
{
   QFile f(USglobal->config_list.root_dir + "/hosts.beowulf");
   if (f.open(IO_WriteOnly | IO_Translate))
   {
      QTextStream ts (&f);
      for (unsigned int i=0; i<lb_hosts->count(); i++)
      {
         ts << lb_hosts->text(i) << endl;
      }
   }
   else
   {
      QMessageBox::message(tr("Attention:"), tr("You don't appear to have write permission\n"
                                                "for this file.\n\n"
                                                "Contact your system administrator for assistance."));
   }
}

void US_Beowulf::help()
{
   US_Help *online_help; 
   online_help = new US_Help(this);
   online_help->show_help("manual/beowulf.html");
}
