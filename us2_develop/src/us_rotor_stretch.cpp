#include "../include/us_rotor_stretch.h"

US_RotorStretch::US_RotorStretch(QWidget *p , const char *name) : QFrame(p, name)
{
   USglobal = new US_Config();
   unsigned int i;
   QString str;
   speed = 30000.0;
   rotor = 0;
   centerpiece = 0;
   cp_list.clear();
   rotor_list.clear();
   if (!readCenterpieceInfo(&cp_list))
   {
      QMessageBox::critical(0, "UltraScan Fatal Error:", "There was a problem opening the\n"
                            "centerpiece database file:\n\n"
                            + USglobal->config_list.system_dir + "/etc/centerpiece.dat\n\n"
                            "Please install the centerpiece database file\n"
                            "before proceeding.", QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
      exit(-1);
   }
   if (!readRotorInfo(&rotor_list))
   {
      QMessageBox::critical(0, "UltraScan Fatal Error:", "There was a problem opening the\n"
                            "rotor database file:\n\n"
                            + USglobal->config_list.system_dir + "/etc/rotor.dat\n\n"
                            "Please install the rotor database file\n"
                            "before proceeding.", QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
      exit(-1);
   }
   
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("UltraScan: Cell Bottom Calculation Utility:"));

   lbl_banner1 = new QLabel(tr(" Please select a speed and\nrotor and centerpiece combination: "), this);
   Q_CHECK_PTR(lbl_banner1);
   lbl_banner1->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_banner1->setAlignment(AlignCenter|AlignVCenter);
   lbl_banner1->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_banner1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   lbl_banner1->setMinimumHeight(70);
   lbl_banner1->setMaximumHeight(70);

   lbl_speed = new QLabel(tr(" Rotor Speed (rpm): "), this);
   Q_CHECK_PTR(lbl_speed);
   lbl_speed->setAlignment(AlignLeft|AlignVCenter);
   lbl_speed->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_speed->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cnt_speed = new QwtCounter(this);
   Q_CHECK_PTR(cnt_speed);
   cnt_speed->setRange(0, 60000, 10);
   cnt_speed->setValue(speed);
   cnt_speed->setNumButtons(3);
   cnt_speed->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_speed, SIGNAL(valueChanged(double)), SLOT(set_speed(double)));

   lbl_rotor = new QLabel(tr(" Select a Rotor: "), this);
   lbl_rotor->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_rotor->setAlignment(AlignLeft|AlignVCenter);
   lbl_rotor->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_centerpiece = new QLabel(tr(" Select a Centerpiece: "), this);
   lbl_centerpiece->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_centerpiece->setAlignment(AlignLeft|AlignVCenter);
   lbl_centerpiece->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lb_rotor = new QListBox(this);
   for (i=0; i<rotor_list.size(); i++)
   {
      lb_rotor->insertItem(str.sprintf("%d " + rotor_list[i].type, rotor_list[i].serial_number));
   }
   lb_rotor->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lb_rotor->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lb_rotor->setCurrentItem(0);
   connect(lb_rotor, SIGNAL(highlighted(int)), SLOT(update_rotor(int)));

   lb_centerpiece = new QListBox(this);
   for (i=0; i<cp_list.size(); i++)
   {
      str.sprintf(cp_list[i].material + tr(", %d channels, %3.2f cm "),  cp_list[i].channels * 2, cp_list[i].pathlength);
      switch (cp_list[i].sector)
      {
         case 0:
         {
            str += "(standard)";
            break;
         }
         case 1:
         {
            str += "(rectangular)";
            break;
         }
         case 2:
         {
            str += "(circular)";
            break;
         }
         case 3:
         {
            str += "(synthetic)";
            break;
         }
         case 4:
         {
            str += "(band-forming)";
            break;
         }
         case 5:
         {
            str += "(meniscus-matching)";
            break;
         }
      }
      lb_centerpiece->insertItem(str);
   }
   lb_centerpiece->setCurrentItem(0);
   lb_centerpiece->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lb_centerpiece->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   connect(lb_centerpiece, SIGNAL(highlighted(int)), SLOT(update_centerpiece(int)));

   lbl_bottom1 = new QLabel(tr(" Centerpiece Bottom Position: "), this);
   lbl_bottom1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_bottom1->setAlignment(AlignLeft|AlignVCenter);
   lbl_bottom1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_bottom2 = new QLabel("", this);
   lbl_bottom2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_bottom2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lbl_bottom2->setAlignment(AlignLeft|AlignVCenter);
   lbl_bottom2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   lbl_stretch1 = new QLabel(tr(" Rotor Stretch: "), this);
   lbl_stretch1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_stretch1->setAlignment(AlignLeft|AlignVCenter);
   lbl_stretch1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_stretch2 = new QLabel("", this);
   lbl_stretch2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_stretch2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lbl_stretch2->setAlignment(AlignLeft|AlignVCenter);
   lbl_stretch2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setAutoDefault(false);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_close = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_close);
   pb_close->setAutoDefault(false);
   pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_close, SIGNAL(clicked()), SLOT(close()));

   global_Xpos += 30;
   global_Ypos += 30;

   move(global_Xpos, global_Ypos);
   setup_GUI();
   update();
}

US_RotorStretch::~US_RotorStretch()
{
}

void US_RotorStretch::setup_GUI()
{
   int j=0;
   int rows = 5, columns = 2, spacing = 2;
   
   QGridLayout * grid = new QGridLayout(this, rows, columns, spacing);
   grid->setMargin(spacing);
   for (int i=0; i<rows; i++)
   {
      grid->setRowSpacing(i, 26);
   }
   grid->addMultiCellWidget(lbl_banner1, j, j, 0, 1);
   j++;
   grid->addWidget(lbl_speed, j, 0);
   grid->addWidget(cnt_speed, j, 1);
   j++;
   grid->addWidget(lbl_rotor, j, 0);
   grid->addWidget(lbl_centerpiece, j, 1);
   j++;
   grid->addWidget(lb_rotor, j, 0);
   grid->addWidget(lb_centerpiece, j, 1);
   j++;
   grid->addWidget(lbl_stretch1, j, 0);
   grid->addWidget(lbl_stretch2, j, 1);
   j++;
   grid->addWidget(lbl_bottom1, j, 0);
   grid->addWidget(lbl_bottom2, j, 1);
   j++;
   grid->addWidget(pb_help, j, 0);
   grid->addWidget(pb_close, j, 1);

   qApp->processEvents();
   QRect r = grid->cellGeometry(0, 0);
   
   global_Xpos += 30;
   global_Ypos += 30;
   
   this->setGeometry(global_Xpos, global_Ypos, r.width()+2*spacing, r.height()+2*spacing);
}

void US_RotorStretch::update()
{
   QString str_bottom, str1;
   //float bottom = calc_bottom(rotor_list, cp_list, rotor, centerpiece, 0, speed);
   float stretch = stretch_with_rotor_list(rotor, (unsigned int) speed, &rotor_list);
   str_bottom = "";
   for (unsigned int i=0; i<cp_list[centerpiece].channels; i++)
   {
      str_bottom += str1.sprintf("%6.4f cm ", cp_list[centerpiece].bottom_position[i]+stretch);
   }
   lbl_bottom2->setText(str_bottom);
   lbl_stretch2->setText(str1.sprintf("%6.4e cm ", stretch));
}

void US_RotorStretch::set_speed(double val)
{
   speed = val;
   update();
}

void US_RotorStretch::update_rotor(int val)
{
   rotor = val;
   update();
}

void US_RotorStretch::update_centerpiece(int val)
{
   centerpiece = val;
   update();
}

void US_RotorStretch::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/rotor_stretch.html");
}

