#include "../include/us_radial_correction.h"


US_RadialCorrection::US_RadialCorrection(QWidget *p , const char *name) : QFrame(p, name)
{
   USglobal = new US_Config();
   correction = 0.0;
   
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("UltraScan: Radial Correction Utility"));

   unsigned int i;
   QString str;
   speed = 30000.0;
   rotor = 0;
   centerpiece = 0;
   top_measured = 5.800;
   top_target = 5.800;
   bottom_measured = 7.200;
   stretch = 0.00;
   scaled = false;
   trim = false;
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
   
   lbl_banner1 = new QLabel(tr(" Radial Calibration\nCorrection Routine "), this);
   Q_CHECK_PTR(lbl_banner1);
   lbl_banner1->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_banner1->setAlignment(AlignCenter|AlignVCenter);
   lbl_banner1->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_banner1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   lbl_banner1->setMinimumHeight(70);
   lbl_banner1->setMaximumHeight(70);

   pb_load = new QPushButton(tr("Select Scans"), this);
   Q_CHECK_PTR(pb_load);
   pb_load->setAutoDefault(false);
   pb_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_load->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_load, SIGNAL(clicked()), SLOT(load()));

   lbl_scans = new QLabel(tr(""), this);
   Q_CHECK_PTR(lbl_scans);
   lbl_scans->setAlignment(AlignLeft|AlignVCenter);
   lbl_scans->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_scans->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   bg_correction = new QButtonGroup();
   bg_correction->setRadioButtonExclusive(true);
   connect(bg_correction, SIGNAL(clicked(int)), SLOT(update_scaled(int)) );

   rb_constant = new QRadioButton(this);
   rb_constant->setText(tr(" Constant Radial Correction: "));
   rb_constant->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1)); //, QFont::Bold));
   rb_constant->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   bg_correction->insert(rb_constant);

   cnt_correction= new QwtCounter(this);
   Q_CHECK_PTR(cnt_correction);
   cnt_correction->setRange(-1.0, 1.0, 0.0001);
   cnt_correction->setValue(correction);
   cnt_correction->setNumButtons(3);
   cnt_correction->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_correction, SIGNAL(valueChanged(double)), SLOT(set_correction(double)));

   rb_scaled = new QRadioButton(this);
   rb_scaled->setText(tr(" Scaled Radial Correction"));
   rb_scaled->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1)); //, QFont::Bold));
   rb_scaled->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   bg_correction->insert(rb_scaled);

   bg_correction->setButton(scaled);

   cb_trim = new QCheckBox(tr(" Trim unneeded data"), this);
   cb_trim->setChecked(trim);
   cb_trim->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_trim->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_trim, SIGNAL(clicked()), SLOT(update_trim()));

   lbl_top_measured = new QLabel(tr(" Cell Top (measured, cm): "), this);
   Q_CHECK_PTR(lbl_top_measured);
   lbl_top_measured->setAlignment(AlignLeft|AlignVCenter);
   lbl_top_measured->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_top_measured->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cnt_top_measured = new QwtCounter(this);
   Q_CHECK_PTR(cnt_top_measured);
   cnt_top_measured->setRange(5.0, 7.0, 0.0001);
   cnt_top_measured->setValue(top_measured);
   cnt_top_measured->setNumButtons(3);
   cnt_top_measured->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_top_measured, SIGNAL(valueChanged(double)), SLOT(set_top_measured(double)));

   lbl_top_target = new QLabel(tr(" Cell Top Target (cm): "), this);
   Q_CHECK_PTR(lbl_top_target);
   lbl_top_target->setAlignment(AlignLeft|AlignVCenter);
   lbl_top_target->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_top_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cnt_top_target = new QwtCounter(this);
   Q_CHECK_PTR(cnt_top_target);
   cnt_top_target->setRange(5.8, 6.0, 0.0001);
   cnt_top_target->setValue(top_target);
   cnt_top_target->setNumButtons(3);
   cnt_top_target->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_top_target, SIGNAL(valueChanged(double)), SLOT(set_top_target(double)));

   lbl_bottom_measured = new QLabel(tr(" Cell Bottom (measured, cm): "), this);
   Q_CHECK_PTR(lbl_bottom_measured);
   lbl_bottom_measured->setAlignment(AlignLeft|AlignVCenter);
   lbl_bottom_measured->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_bottom_measured->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cnt_bottom_measured = new QwtCounter(this);
   Q_CHECK_PTR(cnt_bottom_measured);
   cnt_bottom_measured->setRange(7, 7.3, 0.0001);
   cnt_bottom_measured->setValue(bottom_measured);
   cnt_bottom_measured->setNumButtons(3);
   cnt_bottom_measured->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_bottom_measured, SIGNAL(valueChanged(double)), SLOT(set_bottom_measured(double)));

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
   lb_centerpiece->setVariableWidth(true);
   int width = lb_centerpiece->contentsWidth();
   lb_centerpiece->setMinimumWidth(width);
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

   pb_process = new QPushButton(tr(" Apply Radial Correction "), this);
   Q_CHECK_PTR(pb_process);
   pb_process->setAutoDefault(false);
   pb_process->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_process->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_process, SIGNAL(clicked()), SLOT(process()));

   pgb_progress = new QProgressBar(this, "iteration progress");
   pgb_progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   pgb_progress->setMinimumHeight(26);

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setAutoDefault(false);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_reset = new QPushButton(tr("Reset File Selection"), this);
   Q_CHECK_PTR(pb_reset);
   pb_reset->setAutoDefault(false);
   pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_reset->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));
   
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

US_RadialCorrection::~US_RadialCorrection()
{
}

void US_RadialCorrection::setup_GUI()
{
   int j=0;
   int rows = 15, columns = 2, spacing = 2;
   
   QGridLayout * grid = new QGridLayout(this, rows, columns, spacing);
   grid->setMargin(spacing);
   for (int i=0; i<rows; i++)
   {
      grid->setRowSpacing(i, 26);
   }
   grid->addMultiCellWidget(lbl_banner1, j, j, 0, 1);
   j++;
   grid->addWidget(pb_load, j, 0);
   grid->addWidget(lbl_scans, j, 1);
   j++;
   grid->addWidget(rb_constant, j, 0);
   grid->addWidget(cnt_correction, j, 1);
   j++;
   grid->addWidget(rb_scaled, j, 0);
   grid->addWidget(cb_trim, j, 1);
   j++;
   grid->addWidget(lbl_top_measured, j, 0);
   grid->addWidget(cnt_top_measured, j, 1);
   j++;
   grid->addWidget(lbl_top_target, j, 0);
   grid->addWidget(cnt_top_target, j, 1);
   j++;
   grid->addWidget(lbl_bottom_measured, j, 0);
   grid->addWidget(cnt_bottom_measured, j, 1);
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
   grid->addMultiCellWidget(pgb_progress, j, j, 0, 1);
   j++;
   grid->addWidget(pb_process, j, 0);
   grid->addWidget(pb_reset, j, 1);
   j++;
   grid->addWidget(pb_help, j, 0);
   grid->addWidget(pb_close, j, 1);

   qApp->processEvents();
   QRect r = grid->cellGeometry(0, 0);
   
   global_Xpos += 30;
   global_Ypos += 30;
   
   this->setGeometry(global_Xpos, global_Ypos, r.width()+2*spacing, r.height()+2*spacing);
}

void US_RadialCorrection::reset()
{
   filenames.clear();
   lbl_scans->setText("");
   set_correction(0);
   cnt_correction->setValue(0);
   correction = 0.0;
	pgb_progress->reset();
}

void US_RadialCorrection::load()
{
   QString filter, fileName, str, str1, ext, c[8];
   QStringList sl;
   unsigned int k;
   QFile f;
   filter = "*.RA? *.ra?";
   reset();
   for (k=0; k<8; k++)
   {
      c[k]="";
   }
   // allow multiple files to be selected:

   sl = QFileDialog::getOpenFileNames(filter, USglobal->config_list.data_dir, 0, 0);
   for (QStringList::Iterator it=sl.begin(); it!=sl.end(); it++)
   {
      fileName = *it;
      
      ext = fileName.right(1);
      k = ext.toUInt();
      c[k-1] = ext;
      if (!fileName.isEmpty())
      {
         filenames.push_back(fileName);
         QFile f;
         f.setName(fileName);
         if (f.open(IO_ReadOnly))
         {
            QTextStream ts(&f);
            str1.sprintf(tr("Scan %d: "), filenames.size());
            str1 += ts.readLine();
            f.close();
         }
         else
         {
            QMessageBox::message(tr("UltraScan Error:"),   tr("The radial absorbance file:\n\n")
                                 + fileName
                                 + tr("\n\ncannot be read.\n\n"
                                      "Please check to make sure that you have\n"
                                      "read access to this file."));
            return;
         }
      }
   }
   ext = "";
   for (k=0; k<8; k++)
   {
      if (c[k] != "")
      {
         ext += c[k] + " "; 
      }
   }
   lbl_scans->setText("Scans from cell " + ext + "are selected...");
}

void US_RadialCorrection::process()
{
   QString str1, str2, str3;
   QFile f;
   double val;
   vector <double> x, y, z;
   unsigned int i, j;
   pgb_progress->setTotalSteps(filenames.size());
   for (i=0; i<filenames.size(); i++)
   {
      x.clear();
      y.clear();
      z.clear();
      f.setName(filenames[i]);
      if (f.open(IO_ReadOnly))
      {
         lbl_scans->setText("Updating " + filenames[i]);
         qApp->processEvents();
         QTextStream ts(&f);
         str1 = ts.readLine();
         str2 = ts.readLine();
         while (!ts.eof())
         {
            ts >> val;
            //cout << val << " ";
            if (scaled)
            {
               val = top_target + (val-top_measured) * (bottom_target - top_target)/(bottom_measured - top_measured);
               //cout << val << endl;
            }
            else
            {
               val += correction;
            }
            if (trim)
            {
               if (val > top_target && val < bottom_target)
               {
                  x.push_back(val);
                  ts >> val;
                  y.push_back(val);
                  ts >> val;
                  z.push_back(val);
               }
            }
            else
            {
               x.push_back(val);
               ts >> val;
               y.push_back(val);
               ts >> val;
               z.push_back(val);
            }
            ts.readLine();
         }
         f.close();
         f.remove();
         if (f.open(IO_ReadWrite))
         {
            QTextStream ts(&f);
            ts << str1 << endl;
            ts << str2 << endl;
            for (j=0; j<x.size(); j++)
            {
               str3.sprintf("   %6.4f  %7.5e  %7.5e\n", x[j], y[j], z[j]);
               ts << str3;
            }
            f.close();
            pgb_progress->setProgress(i+1);
         }
         else
         {
            QMessageBox::message(tr("UltraScan Error:"),   tr("The scanfile:\n\n")
                                 + filenames[i]
                                 + tr("\n\ncannot be written.\n\n"
                                      "Please check to make sure that you have\n"
                                      "write access to this file."));
         }
      }
      else
      {
         QMessageBox::message(tr("UltraScan Error:"),   tr("The scanfile:\n\n")
                              + filenames[i]
                              + tr("\n\ncannot be read.\n\n"
                                   "Please check to make sure that you have\n"
                                   "read access to this file."));
      }

   }
}

void US_RadialCorrection::set_correction(double val)
{
   correction = val;
}

void US_RadialCorrection::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/radial_correction.html");
}

void US_RadialCorrection::update()
{
   QString str_bottom, str1;
   //float bottom = calc_bottom(rotor_list, cp_list, rotor, centerpiece, 0, speed);
   stretch = stretch_with_rotor_list(rotor, (unsigned int) speed, &rotor_list);
   str_bottom = "";
   for (unsigned int i=0; i<cp_list[centerpiece].channels; i++)
   {
      bottom_target = cp_list[centerpiece].bottom_position[i]+stretch; // overwrite bottom_target until the last channel - which is used for the correction
      str_bottom += str1.sprintf("%6.4f cm ", bottom_target);
   }
   lbl_bottom2->setText(str_bottom);
   lbl_stretch2->setText(str1.sprintf("%6.4e cm ", stretch));
}

void US_RadialCorrection::set_speed(double val)
{
   speed = val;
   update();
}

void US_RadialCorrection::set_top_measured(double val)
{
   top_measured = val;
}

void US_RadialCorrection::set_bottom_measured(double val)
{
   bottom_measured = val;
}

void US_RadialCorrection::set_top_target(double val)
{
   top_target = val;
}

void US_RadialCorrection::update_rotor(int val)
{
   rotor = val;
   update();
}

void US_RadialCorrection::update_scaled(int val)
{
   scaled = val;
}

void US_RadialCorrection::update_trim()
{
   trim = cb_trim->isChecked();
}

void US_RadialCorrection::update_centerpiece(int val)
{
   centerpiece = val;
   update();
}
