#include "../include/us_radial_correction.h"


US_RadialCorrection::US_RadialCorrection(QWidget *p , const char *name) : QFrame(p, name)
{
   USglobal = new US_Config();
   correction = 0.0;
   
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("UltraScan: Radial Correction Utility"));

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

   lbl_correction = new QLabel(tr(" Radial Calibration Correction distance (cm): "), this);
   Q_CHECK_PTR(lbl_correction);
   lbl_correction->setAlignment(AlignLeft|AlignVCenter);
   lbl_correction->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_correction->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cnt_correction= new QwtCounter(this);
   Q_CHECK_PTR(cnt_correction);
   cnt_correction->setRange(-1.0, 1.0, 0.001);
   cnt_correction->setValue(correction);
   cnt_correction->setNumButtons(3);
   cnt_correction->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_correction, SIGNAL(valueChanged(double)), SLOT(set_correction(double)));

   pb_process = new QPushButton(tr(" Apply Radial Correction "), this);
   Q_CHECK_PTR(pb_process);
   pb_process->setAutoDefault(false);
   pb_process->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_process->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_process, SIGNAL(clicked()), SLOT(process()));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setAutoDefault(false);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_reset = new QPushButton(tr("Reset"), this);
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

   pgb_progress = new QProgressBar(this, "iteration progress");
   pgb_progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   pgb_progress->setMinimumHeight(26);


   global_Xpos += 30;
   global_Ypos += 30;

   move(global_Xpos, global_Ypos);
   
   setup_GUI();
}

US_RadialCorrection::~US_RadialCorrection()
{
}

void US_RadialCorrection::setup_GUI()
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
   grid->addWidget(pb_load, j, 0);
   grid->addWidget(lbl_scans, j, 1);
   j++;
   grid->addWidget(lbl_correction, j, 0);
   grid->addWidget(cnt_correction, j, 1);
   j++;
   grid->addWidget(pb_process, j, 0);
   grid->addWidget(pb_reset, j, 1);
   j++;
   grid->addMultiCellWidget(pgb_progress, j, j, 0, 1);
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
            val += correction;
            x.push_back(val);
            ts >> val;
            y.push_back(val);
            ts >> val;
            z.push_back(val);
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

