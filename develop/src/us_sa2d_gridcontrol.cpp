#include "../include/us_sa2d_gridcontrol.h"

US_SA2D_GridControl::US_SA2D_GridControl(struct sa2d_data *SA2D_Params, QWidget *p, const char *name) : QDialog( p, name)
{
   USglobal=new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("2D Spectrum Analysis"));
   this->SA2D_Params = SA2D_Params;

   lbl_info1 = new QLabel(tr("2-dimensional Spectrum Analysis\nGrid Control Window"), this);
   Q_CHECK_PTR(lbl_info1);
   lbl_info1->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info1->setAlignment(AlignCenter|AlignVCenter);
   lbl_info1->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_ff0_min = new QLabel(tr(" f/f0 minimum: "), this);
   Q_CHECK_PTR(lbl_ff0_min);
   lbl_ff0_min->setAlignment(AlignLeft|AlignVCenter);
   lbl_ff0_min->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_ff0_min->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cnt_ff0_min= new QwtCounter(this);
   Q_CHECK_PTR(cnt_ff0_min);
   cnt_ff0_min->setRange(1, 49, 0.01);
   cnt_ff0_min->setValue((*SA2D_Params).ff0_min);
   cnt_ff0_min->setNumButtons(3);
   cnt_ff0_min->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_ff0_min, SIGNAL(valueChanged(double)), SLOT(update_ff0_min(double)));

   lbl_ff0_max = new QLabel(tr(" f/f0 maximum: "), this);
   Q_CHECK_PTR(lbl_ff0_max);
   lbl_ff0_max->setAlignment(AlignLeft|AlignVCenter);
   lbl_ff0_max->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_ff0_max->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cnt_ff0_max= new QwtCounter(this);
   Q_CHECK_PTR(cnt_ff0_max);
   cnt_ff0_max->setRange(1, 50, 0.01);
   cnt_ff0_max->setValue((*SA2D_Params).ff0_max);
   cnt_ff0_max->setNumButtons(3);
   cnt_ff0_max->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_ff0_max, SIGNAL(valueChanged(double)), SLOT(update_ff0_max(double)));

   lbl_ff0_resolution = new QLabel(tr(" f/f0 resolution: "), this);
   Q_CHECK_PTR(lbl_ff0_resolution);
   lbl_ff0_resolution->setAlignment(AlignLeft|AlignVCenter);
   lbl_ff0_resolution->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_ff0_resolution->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cnt_ff0_resolution= new QwtCounter(this);
   Q_CHECK_PTR(cnt_ff0_resolution);
   cnt_ff0_resolution->setRange(1, 100000, 1);
   cnt_ff0_resolution->setValue((*SA2D_Params).ff0_resolution);
   cnt_ff0_resolution->setNumButtons(3);
   cnt_ff0_resolution->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_ff0_resolution, SIGNAL(valueChanged(double)), SLOT(update_ff0_resolution(double)));

   lbl_s_min = new QLabel(tr(" Minimum s-value: "), this);
   Q_CHECK_PTR(lbl_s_min);
   lbl_s_min->setAlignment(AlignLeft|AlignVCenter);
   lbl_s_min->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_s_min->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cnt_s_min= new QwtCounter(this);
   Q_CHECK_PTR(cnt_s_min);
   cnt_s_min->setRange(0.01, 9999, 0.1);
   cnt_s_min->setValue((*SA2D_Params).s_min);
   cnt_s_min->setNumButtons(3);
   cnt_s_min->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_s_min, SIGNAL(valueChanged(double)), SLOT(update_s_min(double)));

   lbl_s_max = new QLabel(tr(" Maximum s-value: "), this);
   Q_CHECK_PTR(lbl_s_max);
   lbl_s_max->setAlignment(AlignLeft|AlignVCenter);
   lbl_s_max->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_s_max->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cnt_s_max= new QwtCounter(this);
   Q_CHECK_PTR(cnt_s_max);
   cnt_s_max->setRange(1, 10000, 1);
   cnt_s_max->setValue((*SA2D_Params).s_max);
   cnt_s_max->setNumButtons(3);
   cnt_s_max->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_s_max, SIGNAL(valueChanged(double)), SLOT(update_s_max(double)));

   lbl_s_resolution = new QLabel(tr(" s-value Resolution: "), this);
   Q_CHECK_PTR(lbl_s_resolution);
   lbl_s_resolution->setAlignment(AlignLeft|AlignVCenter);
   lbl_s_resolution->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_s_resolution->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cnt_s_resolution= new QwtCounter(this);
   Q_CHECK_PTR(cnt_s_resolution);
   cnt_s_resolution->setRange(1, 10000, 1);
   cnt_s_resolution->setValue((*SA2D_Params).s_resolution);
   cnt_s_resolution->setNumButtons(3);
   cnt_s_resolution->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_s_resolution, SIGNAL(valueChanged(double)), SLOT(update_s_resolution(double)));

   lbl_regularization = new QLabel(tr(" Regularization: "), this);
   Q_CHECK_PTR(lbl_regularization);
   lbl_regularization->setAlignment(AlignLeft|AlignVCenter);
   lbl_regularization->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_regularization->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cnt_regularization= new QwtCounter(this);
   Q_CHECK_PTR(cnt_regularization);
   cnt_regularization->setRange(0, 1, 0.01);
   cnt_regularization->setValue((*SA2D_Params).regularization);
   cnt_regularization->setNumButtons(3);
   cnt_regularization->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_regularization, SIGNAL(valueChanged(double)), SLOT(update_regularization(double)));

   lbl_uniform_grid_repetition = new QLabel(tr(" Uniform grid repetitions: "), this);
   Q_CHECK_PTR(lbl_uniform_grid_repetition);
   lbl_uniform_grid_repetition->setAlignment(AlignLeft|AlignVCenter);
   lbl_uniform_grid_repetition->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_uniform_grid_repetition->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cnt_uniform_grid_repetition= new QwtCounter(this);
   Q_CHECK_PTR(cnt_uniform_grid_repetition);
   cnt_uniform_grid_repetition->setRange(1, 20, 1);
   cnt_uniform_grid_repetition->setValue((*SA2D_Params).uniform_grid_repetition);
   cnt_uniform_grid_repetition->setNumButtons(3);
   cnt_uniform_grid_repetition->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_uniform_grid_repetition, SIGNAL(valueChanged(double)), SLOT(update_uniform_grid_repetition(double)));

   cb_meniscus = new QCheckBox(this);
   cb_meniscus->setText(tr(" Float Meniscus Position "));
   cb_meniscus->setChecked((*SA2D_Params).fit_meniscus);
   cb_meniscus->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_meniscus->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_meniscus, SIGNAL(clicked()), SLOT(set_meniscus()));

   lbl_meniscus_range = new QLabel(tr(" Meniscus Fit Range (cm): "), this);
   Q_CHECK_PTR(lbl_meniscus_range);
   lbl_meniscus_range->setAlignment(AlignLeft|AlignVCenter);
   lbl_meniscus_range->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_meniscus_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cnt_meniscus_range= new QwtCounter(this);
   Q_CHECK_PTR(cnt_meniscus_range);
   cnt_meniscus_range->setRange(0, 0.01, 1e-4);
   cnt_meniscus_range->setValue((*SA2D_Params).meniscus_range);
   cnt_meniscus_range->setNumButtons(3);
   cnt_meniscus_range->setEnabled((*SA2D_Params).fit_meniscus);
   cnt_meniscus_range->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_meniscus_range, SIGNAL(valueChanged(double)), SLOT(update_meniscus_range(double)));

   lbl_meniscus_gridpoints = new QLabel(tr(" # of Meniscus Grid Points: "), this);
   Q_CHECK_PTR(lbl_meniscus_gridpoints);
   lbl_meniscus_gridpoints->setAlignment(AlignLeft|AlignVCenter);
   lbl_meniscus_gridpoints->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_meniscus_gridpoints->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cnt_meniscus_gridpoints= new QwtCounter(this);
   Q_CHECK_PTR(cnt_meniscus_gridpoints);
   cnt_meniscus_gridpoints->setRange(1, 50, 1);
   cnt_meniscus_gridpoints->setValue((*SA2D_Params).meniscus_gridpoints);
   cnt_meniscus_gridpoints->setNumButtons(3);
   cnt_meniscus_gridpoints->setEnabled((*SA2D_Params).fit_meniscus);
   cnt_meniscus_gridpoints->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_meniscus_gridpoints, SIGNAL(valueChanged(double)), SLOT(update_meniscus_gridpoints(double)));

   cb_iterations = new QCheckBox(this);
   cb_iterations->setText(tr(" Use Iterative Method "));
   cb_iterations->setChecked((*SA2D_Params).use_iterative);
   cb_iterations->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iterations->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_iterations, SIGNAL(clicked()), SLOT(set_iterations()));

   lbl_max_iterations = new QLabel(tr(" Maximum Iterations: "), this);
   Q_CHECK_PTR(lbl_max_iterations);
   lbl_max_iterations->setAlignment(AlignLeft|AlignVCenter);
   lbl_max_iterations->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_max_iterations->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cnt_max_iterations= new QwtCounter(this);
   Q_CHECK_PTR(cnt_max_iterations);
   cnt_max_iterations->setRange(1, 10, 1);
   cnt_max_iterations->setValue((*SA2D_Params).max_iterations);
   cnt_max_iterations->setNumButtons(2);
   cnt_max_iterations->setEnabled((*SA2D_Params).use_iterative);
   cnt_max_iterations->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_max_iterations, SIGNAL(valueChanged(double)), SLOT(update_max_iterations(double)));

   pb_load_distro = new QPushButton(tr(" Load Distribution "), this);
   Q_CHECK_PTR(pb_load_distro);
   pb_load_distro->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_load_distro->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_load_distro->setAutoDefault(false);
   connect(pb_load_distro, SIGNAL(clicked()), SLOT(load_distro()));

   pb_help = new QPushButton(tr(" Help "), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_help->setAutoDefault(false);
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_close = new QPushButton(tr(" Accept "), this);
   Q_CHECK_PTR(pb_close);
   pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_close->setAutoDefault(false);
   connect(pb_close, SIGNAL(clicked()), SLOT(accept()));

   pb_reject = new QPushButton(tr(" Cancel "), this);
   Q_CHECK_PTR(pb_reject);
   pb_reject->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_reject->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_reject->setAutoDefault(false);
   connect(pb_reject, SIGNAL(clicked()), SLOT(reject()));

   setup_GUI();

   global_Xpos += 30;
   global_Ypos += 30;
   move(global_Xpos, global_Ypos);
}

US_SA2D_GridControl::~US_SA2D_GridControl()
{
}

void US_SA2D_GridControl::setup_GUI()
{
   int rows = 15, columns = 2, spacing = 2, j=0;
   QGridLayout *background=new QGridLayout(this, 2, 1, spacing);
   background->setRowSpacing(0, 40);
   background->addWidget(lbl_info1, 0, 0);
   QGridLayout *controlGrid = new QGridLayout(background, rows, columns, spacing);

   for (int i=0; i<rows-1; i++)
   {
      controlGrid->setRowSpacing(i, 26);
   }
   controlGrid->setColSpacing(0, 120);
   controlGrid->setColSpacing(1, 120);

   controlGrid->addWidget(lbl_s_min, j, 0);
   controlGrid->addWidget(cnt_s_min, j, 1);
   j++;
   controlGrid->addWidget(lbl_s_max, j, 0);
   controlGrid->addWidget(cnt_s_max, j, 1);
   j++;
   controlGrid->addWidget(lbl_s_resolution, j, 0);
   controlGrid->addWidget(cnt_s_resolution, j, 1);
   j++;
   controlGrid->addWidget(lbl_ff0_min, j, 0);
   controlGrid->addWidget(cnt_ff0_min, j, 1);
   j++;
   controlGrid->addWidget(lbl_ff0_max, j, 0);
   controlGrid->addWidget(cnt_ff0_max, j, 1);
   j++;
   controlGrid->addWidget(lbl_ff0_resolution, j, 0);
   controlGrid->addWidget(cnt_ff0_resolution, j, 1);
   j++;
   controlGrid->addWidget(lbl_regularization, j, 0);
   controlGrid->addWidget(cnt_regularization, j, 1);
   j++;
   controlGrid->addWidget(lbl_uniform_grid_repetition, j, 0);
   controlGrid->addWidget(cnt_uniform_grid_repetition, j, 1);
   j++;
   controlGrid->addWidget(cb_meniscus, j, 0);
   j++;
   controlGrid->addWidget(lbl_meniscus_range, j, 0);
   controlGrid->addWidget(cnt_meniscus_range, j, 1);
   j++;
   controlGrid->addWidget(lbl_meniscus_gridpoints, j, 0);
   controlGrid->addWidget(cnt_meniscus_gridpoints, j, 1);
   j++;
   controlGrid->addWidget(cb_iterations, j, 0);
   j++;
   controlGrid->addWidget(lbl_max_iterations, j, 0);
   controlGrid->addWidget(cnt_max_iterations, j, 1);
   j++;
   controlGrid->addWidget(pb_help, j, 0);
   controlGrid->addWidget(pb_load_distro, j, 1);
   j++;
   controlGrid->addWidget(pb_reject, j, 0);
   controlGrid->addWidget(pb_close, j, 1);
}

void US_SA2D_GridControl::load_distro()
{
   QFile f;
   float temp1, temp2;
   QString filename;
   sval.clear();
   frequency.clear();
   ff0.clear();
   filename = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*.vhw_his.* *.cofs_dis.* *.sa2d_dis.*", 0);
   if (filename.isEmpty())
   {
      return;
   }
   f.setName(filename);
   if (filename.contains("vhw_his", false))
   {
      if(f.open(IO_ReadOnly))
      {
         QTextStream ts(&f);
         if (!ts.eof())
         {
            ts.readLine(); // discard header line
         }
         while (!ts.eof())
         {
            ts >> temp1;
            ts >> temp2;
            if (!ts.eof() && temp2 > 0.01)
            {
               sval.push_back(temp1);
               frequency.push_back(temp2);
            }
            ts >> temp1;
            ts >> temp2;
         }
      }
   }
   if (filename.contains("cofs_dis", false))
   {
      if(f.open(IO_ReadOnly))
      {
         QTextStream ts(&f);
         if (!ts.eof())
         {
            ts.readLine(); // discard header line
         }
         while (!ts.eof())
         {
            ts >> temp1;
            ts >> temp1;
            ts >> temp2;
            ts >> temp2;
            ts >> temp2;
            ts >> temp2;
            if (!ts.eof())
            {
               sval.push_back(temp1*1.0e13);
               frequency.push_back(temp2);
            }
         }
      }
   }
   if (filename.contains("sa2d_dis", false))
   {
      if(f.open(IO_ReadOnly))
      {
         QTextStream ts(&f);
         if (!ts.eof())
         {
            ts.readLine(); // discard header line
         }
         while (!ts.eof())
         {
            ts >> temp2;
            ts >> temp1;
            ts >> temp1;
            if (!ts.eof())
            {
               sval.push_back(temp1*1.0e13);
               frequency.push_back(temp2);
            }
            ts >> temp1;
            ts >> temp2;
            if (!ts.eof())
            {
               ff0.push_back(temp2);
            }
            ts >> temp2;
         }
         sort(ff0.begin(), ff0.end());
         temp1 = ff0[ff0.size()-1] - ff0[0];
         temp2 = ff0[0] - temp1/10.0;
         if (temp2 < 1.0)
         {
            (*SA2D_Params).ff0_min = 1.0;
         }
         else
         {
            (*SA2D_Params).ff0_min = temp2;
         }
         (*SA2D_Params).ff0_max = ff0[ff0.size()-1] + (ff0[0] - temp1)/10.0;
         cnt_ff0_min->setValue((*SA2D_Params).ff0_min);
         cnt_ff0_max->setValue((*SA2D_Params).ff0_max);
      }
   }
   sort(sval.begin(), sval.end());
   temp2 = sval[0] - sval[0]/10.0;
   if (temp2 < 0.01)
   {
      (*SA2D_Params).s_min = 0.01;
   }
   else
   {
      (*SA2D_Params).s_min = temp2;
   }
   (*SA2D_Params).s_max = sval[sval.size()-1] + (sval[sval.size()-1])/10.0;
   cnt_s_min->setValue((*SA2D_Params).s_min);
   cnt_s_max->setValue((*SA2D_Params).s_max);
}

void US_SA2D_GridControl::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/sa2d_gridcontrol.html");
}

void US_SA2D_GridControl::update_regularization(double val)
{
   (*SA2D_Params).regularization = (float) val;
}

void US_SA2D_GridControl::update_meniscus_range(double val)
{
   (*SA2D_Params).meniscus_range = (float) val;
}

void US_SA2D_GridControl::update_meniscus_gridpoints(double val)
{
   (*SA2D_Params).meniscus_gridpoints = (unsigned int) val;
}

void US_SA2D_GridControl::update_max_iterations(double val)
{
   (*SA2D_Params).max_iterations = (unsigned int) val;
}

void US_SA2D_GridControl::update_ff0_min(double val)
{
   (*SA2D_Params).ff0_min = (float) val;
   if ((*SA2D_Params).ff0_min > (*SA2D_Params).ff0_max)
   {
      (*SA2D_Params).ff0_max = (*SA2D_Params).ff0_min;
      cnt_ff0_max->setValue((*SA2D_Params).ff0_max);
   }
}

void US_SA2D_GridControl::update_ff0_max(double val)
{
   (*SA2D_Params).ff0_max = (float) val;
   if ((*SA2D_Params).ff0_max < (*SA2D_Params).ff0_min)
   {
      (*SA2D_Params).ff0_min = (*SA2D_Params).ff0_max;
      cnt_ff0_min->setValue((*SA2D_Params).ff0_min);
   }
}

void US_SA2D_GridControl::update_ff0_resolution(double val)
{
   (*SA2D_Params).ff0_resolution = (unsigned int) val;
}

void US_SA2D_GridControl::update_s_min(double val)
{
   (*SA2D_Params).s_min = (float) val;
   if ((*SA2D_Params).s_min > (*SA2D_Params).s_max)
   {
      (*SA2D_Params).s_max = (*SA2D_Params).s_min;
      cnt_s_max->setValue((*SA2D_Params).s_max);
   }
}

void US_SA2D_GridControl::update_s_max(double val)
{
   (*SA2D_Params).s_max = (float) val;
   if ((*SA2D_Params).s_max < (*SA2D_Params).s_min)
   {
      (*SA2D_Params).s_min = (*SA2D_Params).s_max;
      cnt_s_min->setValue((*SA2D_Params).s_min);
   }
}

void US_SA2D_GridControl::update_s_resolution(double val)
{
   (*SA2D_Params).s_resolution = (unsigned int) val;
}

void US_SA2D_GridControl::set_meniscus()
{
   if ((*SA2D_Params).fit_meniscus)
   {
      (*SA2D_Params).fit_meniscus = false;
      cnt_meniscus_range->setEnabled(false);
      (*SA2D_Params).meniscus_range = 0.0;
      cnt_meniscus_range->setValue(0.0);
      cnt_meniscus_gridpoints->setEnabled(false);
   }
   else
   {
      (*SA2D_Params).fit_meniscus = true;
      cnt_meniscus_range->setEnabled(true);
      cnt_meniscus_gridpoints->setEnabled(true);
   }
}

void US_SA2D_GridControl::set_iterations()
{
   if ((*SA2D_Params).use_iterative)
   {
      (*SA2D_Params).use_iterative = false;
      cnt_max_iterations->setEnabled(false);
      (*SA2D_Params).max_iterations = 3;
      cnt_max_iterations->setValue(3);
   }
   else
   {
      (*SA2D_Params).use_iterative = true;
      cnt_max_iterations->setEnabled(true);
   }
}

void US_SA2D_GridControl::update_uniform_grid_repetition(double val)
{
   (*SA2D_Params).uniform_grid_repetition = (unsigned int) val;
}

void US_SA2D_GridControl::closeEvent(QCloseEvent *e)
{
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_SA2D_GridControl::accept()
{
   global_Xpos -= 30;
   global_Ypos -= 30;
   QDialog::accept();
}

void US_SA2D_GridControl::reject()
{
   global_Xpos -= 30;
   global_Ypos -= 30;
   QDialog::reject();
}

