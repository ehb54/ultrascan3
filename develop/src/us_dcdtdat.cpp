#include "../include/us_dcdtdat.h"

#ifndef WIN32
#include <unistd.h>
#endif

dcdt_dat_W::dcdt_dat_W(QWidget *p, const char *name) : Data_Control_W(1, p, name)
{
   baseline_flag = true;
   bd_range=90;
   bd_position = 5;
   range_counter->setValue(bd_range);
   position_counter->setValue(bd_position);
   pb_second_plot->setText(tr("Avg. dC/dt Plot"));
   dcdt_plots = false;
   xpos = border;
   ypos = 438;
   setup_already = false;
   pm = new US_Pixmap();
   smin = 0.0;
   smax = 1000.0;
   
   sval_lbl = new QLabel(tr(" S-value Cutoff:"), this);
   Q_CHECK_PTR(sval_lbl);
   sval_lbl->setAlignment(AlignLeft|AlignVCenter);
   sval_lbl->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   sval_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   sval_lbl->setGeometry(xpos, ypos, buttonw, buttonh);

   xpos += buttonw + spacing;

   sval_counter= new QwtCounter(this);
   Q_CHECK_PTR(sval_counter);
   sval_counter->setRange(0, 1000, 10);
   sval_counter->setValue(1000);
   sval_counter->setNumButtons(3);
   sval_counter->setGeometry(xpos, ypos, buttonw, buttonh);
   sval_counter->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   //connect(sval_counter, SIGNAL(buttonReleased(double)), SLOT(update_sval(double)));
   connect(sval_counter, SIGNAL(valueChanged(double)), SLOT(update_sval(double)));
   

   ypos = 410;
   
   blank_lbl = new QLabel("",this);
   blank_lbl->setFrameStyle(QFrame::WinPanel|Raised);
   blank_lbl->setGeometry(xpos, ypos, buttonw, buttonh);
   blank_lbl->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   blank_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   bt_r = new QRadioButton(this);
   bt_r->setText("x: r");
   bt_r->setGeometry(xpos+10, ypos+4, 50, buttonh-8);
   bt_r->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   bt_r->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(bt_r, SIGNAL(clicked()), SLOT(update_r_button()));

   xpos += 65;

   bt_s = new QRadioButton(this);
   bt_s->setText("x: S");
   bt_s->setGeometry(xpos, ypos+4, 50, buttonh-8);
   bt_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   bt_s->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(bt_s, SIGNAL(clicked()), SLOT(update_s_button()));
   
   dcdt_plots = true;
   bt_s->setChecked(false);
   bt_r->setChecked(true);
   
   setup_GUI();
}

dcdt_dat_W::~dcdt_dat_W()
{
}

void dcdt_dat_W::setup_GUI()
{
   int j=0;
   int rows = 13, columns = 4, spacing = 2;
   
   QGridLayout * background = new QGridLayout(this,2,2,spacing);   
   QGridLayout * subGrid1 = new QGridLayout(rows, columns, spacing);
   for (int i=0; i<rows; i++)
   {
      subGrid1->setRowSpacing(i, 26);
   }
   subGrid1->addMultiCellWidget(pb_load,j,j,0,1);
   subGrid1->addMultiCellWidget(pb_details,j,j,2,3);
   j++;
   subGrid1->addMultiCellWidget(pb_second_plot,j,j,0,1);
   subGrid1->addMultiCellWidget(pb_save,j,j,2,3);
   j++;
   subGrid1->addMultiCellWidget(pb_print,j,j,0,1);
   subGrid1->addMultiCellWidget(pb_view,j,j,2,3);
   j++;
   subGrid1->addMultiCellWidget(pb_help,j,j,0,1);
   subGrid1->addMultiCellWidget(pb_close,j,j,2,3);
   j++;
   subGrid1->addMultiCellWidget(banner1,j,j,0,3);
   j++;
   subGrid1->addMultiCellWidget(lbl_run_id1,j,j,0,1);
   subGrid1->addMultiCellWidget(lbl_run_id2,j,j,2,3);
   j++;
   subGrid1->addMultiCellWidget(lbl_temperature1,j,j,0,1);
   subGrid1->addMultiCellWidget(lbl_temperature2,j,j,2,3);
   j++;
   subGrid1->addMultiCellWidget(lbl_cell_info1,j,j,0,1);
   subGrid1->addMultiCellWidget(lbl_cell_info2,j,j,2,3);
   j++;
   subGrid1->addMultiCellWidget(lbl_cell_descr,j,j,0,3);
   j++;
   subGrid1->addMultiCellWidget(cell_select,j,j+2,0,1);
   subGrid1->addMultiCellWidget(lambda_select,j,j+2,2,3);
      
   rows = 11, columns = 4, spacing = 2, j=0;
   QGridLayout * subGrid2 = new QGridLayout(rows, columns, spacing);
   for (int i=0; i<rows; i++)
   {
      subGrid2->setRowSpacing(i, 26);
   }

   subGrid2->addMultiCellWidget(banner2,j,j,0,3);
   j++;
   subGrid2->addWidget(pb_density,j,0);
   subGrid2->addWidget(density_le,j,1);
   subGrid2->addWidget(pb_viscosity,j,2);
   subGrid2->addWidget(viscosity_le,j,3);
   j++;
   subGrid2->addWidget(pb_vbar,j,0);
   subGrid2->addWidget(vbar_le,j,1);
   subGrid2->addWidget(lbl1_excluded,j,2);
   subGrid2->addWidget(lbl2_excluded,j,3);
   j++;
   subGrid2->addMultiCellWidget(pb_reset,j,j,0,1);
   subGrid2->addMultiCellWidget(blank_lbl,j,j,2,3);
   
   QGridLayout * radiobar = new QGridLayout(subGrid1);
   radiobar->addWidget(bt_r,0,0);
   radiobar->addWidget(bt_s,0,1);
   subGrid2->addMultiCell(radiobar,j,j,2,3);
   j++;
   subGrid2->addMultiCellWidget(sval_lbl,j,j,0,1);
   subGrid2->addMultiCellWidget(sval_counter,j,j,2,3);
   j++;
   subGrid2->addMultiCellWidget(smoothing_lbl,j,j,0,1);
   subGrid2->addMultiCellWidget(smoothing_counter,j,j,2,3);
   j++;
   subGrid2->addMultiCellWidget(range_lbl,j,j,0,1);
   subGrid2->addMultiCellWidget(range_counter,j,j,2,3);
   j++;
   subGrid2->addMultiCellWidget(position_lbl,j,j,0,1);
   subGrid2->addMultiCellWidget(position_counter,j,j,2,3);
   j++;
   subGrid2->addMultiCellWidget(pb_exsingle,j,j,0,1);
   subGrid2->addMultiCellWidget(cnt_exsingle,j,j,2,3);
   j++;
   subGrid2->addMultiCellWidget(pb_exrange,j,j,0,1);
   subGrid2->addMultiCellWidget(cnt_exrange,j,j,2,3);
   j++;
   subGrid2->addWidget(lbl_status,j,0);
   subGrid2->addMultiCellWidget(progress,j,j,1,3);
      
   background->addLayout(subGrid1,0,0);
   background->addWidget(analysis_plot,0,1);
   background->addLayout(subGrid2,1,0);
   background->addWidget(edit_plot,1,1);
   background->setColStretch(0,1);
   background->setColStretch(1,4);
   background->setColSpacing(0,350);
   background->setColSpacing(1,550);

   qApp->processEvents();
   QRect r = background->cellGeometry(0, 0);

   global_Xpos += 30;
   global_Ypos += 30;
   
   this->setGeometry(global_Xpos, global_Ypos, r.width()+550+spacing*3, this->height());

}

void dcdt_dat_W::update_s_button()
{
   if (step == 0)
   {
      QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n      Click on \"Load Data\"\n"));
      return;
   }
   dcdt_plots = false;   
   bt_s->setChecked(true);
   bt_r->setChecked(false);
   plot_analysis();   
}

void dcdt_dat_W::update_r_button()
{
   if (step == 0)
   {
      QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n      Click on \"Load Data\"\n"));
      return;
   }
   dcdt_plots = true;
   bt_s->setChecked(false);
   bt_r->setChecked(true);
   plot_analysis();
}

void dcdt_dat_W::second_plot()
{
   if (step == 0)
   {
      QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n      Click on \"Load Data\"\n"));
      return;
   }
   int temp_scans = run_inf.scans[selected_cell][selected_lambda] - exclude - 1;
   avg = new US_Average(points, temp_scans, &sval, &dcdt_points, "dcdt", htmlDir, title, selected_cell, selected_lambda);
   avg->setCaption(tr("Time Derivative: Average S Distribution:"));
   avg->show();
}

void dcdt_dat_W::save()
{
   if (step == 0)
   {
      QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n      Click on \"Load Data\"\n"));
      return;
   }
   write_dcdt_avg();
   write_dcdt_scans();
   write_dcdt_sval();
   write_res();
   QPixmap p;
   QString fileName;
   fileName.sprintf(htmlDir + "/dcdt_%d%d.", selected_cell + 1, selected_lambda +1);
   p = QPixmap::grabWidget(analysis_plot, 2, 2, analysis_plot->width() - 4, analysis_plot->height() - 4);
   pm->save_file(fileName, p);
   fileName.sprintf(htmlDir + "/dcdt_edited_%d%d.", selected_cell + 1, selected_lambda +1);
   p = QPixmap::grabWidget(edit_plot, 2, 2, edit_plot->width() - 4, edit_plot->height() - 4);
   pm->save_file(fileName, p);
}

void dcdt_dat_W::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/dcdt.html");
}

int dcdt_dat_W::plot_analysis()
{
   double s_step;
   already = true;
   if (setup_already)
   {
      cleanup();
   }
   if (setups() < 0)
   {
      return (-1);
   }
   analysis_plot->clear();
   title.sprintf(tr("Run ") + run_inf.run_id + tr(": Cell %d (%d nm) - Time Derivative (dC/dt)"),
                 selected_cell + 1, run_inf.wavelength[selected_cell][selected_lambda]);
   analysis_plot->setTitle(title);
   analysis_plot->setAxisTitle(QwtPlot::yLeft, tr("Frequency"));

   for (j=exclude+1; j<run_inf.scans[selected_cell][selected_lambda]; j++)
   {
      for (i=0; i<points; i++)
      {
         //
         // Normalize the plateaus:
         //
         dcdt_points[j-exclude-1][i] = ((absorbance[j-1][i]
                                         / run_inf.plateau[selected_cell][selected_lambda][j-1])
                                        - (absorbance[j][i] / run_inf.plateau[selected_cell][selected_lambda][j]))
            /(run_inf.time[selected_cell][selected_lambda][j] - run_inf.time[selected_cell][selected_lambda][j-1]);
         sval[j-exclude-1][i] = correction * 1e13 * log (radius[i] / run_inf.meniscus[selected_cell])
            / (omega_s * (run_inf.time[selected_cell][selected_lambda][j-1]
                          + run_inf.time[selected_cell][selected_lambda][j]) / 2);
      }
      progress->setProgress(j-exclude);
      plots[j-exclude-1] = analysis_plot->insertCurve("dcdt");
      analysis_plot->setCurveStyle(plots[j-exclude-1], QwtCurve::Lines);
      analysis_plot->setCurvePen(plots[j-exclude-1], yellow);
      if(dcdt_plots)
      {
         analysis_plot->setCurveData(plots[j-exclude-1], radius, dcdt_points[j-exclude-1], points);
         analysis_plot->setAxisTitle(QwtPlot::xBottom, tr("Radius (cm)"));
      }
      else
      {
         analysis_plot->setCurveData(plots[j-exclude-1], sval[j-exclude-1], dcdt_points[j-exclude-1], points);
         analysis_plot->setAxisTitle(QwtPlot::xBottom, tr("Sedimentation Coefficient x 10^(13) sec"));
      }
   }
   if (start)
   {
      get_2d_limits(&sval, &smin, &smax, run_inf.scans[selected_cell][selected_lambda]-exclude-1, points);
      smin = 0;
      s_step = smax/100;
      sval_counter->setRange(0, smax, s_step);
      sval_counter->setValue(smax);
      start = false;
   }
   analysis_plot->setAxisAutoScale(QwtPlot::xBottom);
   analysis_plot->setAxisAutoScale(QwtPlot::yLeft);
   analysis_plot->setAxisTitle(QwtPlot::yLeft, tr("Frequency"));
   analysis_plot->setCaption(tr("Radial Derivative - dC/dr Plot"));
   analysis_plot->replot();
   smax = analysis_plot->axisScale(QwtPlot::xBottom)->hBound();
   smin = analysis_plot->axisScale(QwtPlot::xBottom)->lBound();
   disconnect(sval_counter, 0, 0, 0);
   sval_counter->setRange(smin, smax*3, (smax-smin)/1000.0);
   sval_counter->setValue(smax);   
   connect(sval_counter, SIGNAL(valueChanged(double)), SLOT(update_sval(double)));
   return(0);
}

void dcdt_dat_W::update_sval(double newValue)
{
   smax = newValue;
   analysis_plot->setAxisScale(QwtPlot::xBottom, smin, smax);
   analysis_plot->replot();
}

int dcdt_dat_W::setups()
{
   QString str;
   // 
   // make sure that the first datapoint of each scan has a lower concentration than 
   // the current lower concentration boundary
   //
   exclude=0;
   for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
   {
      if (absorbance[i][0] >= run_inf.plateau[selected_cell][selected_lambda][i] * bd_position/100)
      {
         exclude++;
      }
   }
   lbl2_excluded->setText(str.sprintf("%d", exclude));
   if ((run_inf.scans[selected_cell][selected_lambda] - exclude) < 4)
   {
      QMessageBox::message( tr("Warning"), tr("Attention:\n\n"
                                              "Not enough scans have cleared the meniscus.\n\n"
                                              "In order to perform a meaningful dC/dt Analysis,\n"
                                              "you need at least 4 scans that have cleared the\n"
                                              "meniscus."));
      return(-1);
   }
   progress->setTotalSteps(run_inf.scans[selected_cell][selected_lambda] - exclude-1);
   progress->reset();
   progress->setProgress(0);
   
   frequency = new double [points];
   dcdt_points = new double* [run_inf.scans[selected_cell][selected_lambda] - exclude - 1];
   Q_CHECK_PTR(dcdt_points);
   sval = new double* [run_inf.scans[selected_cell][selected_lambda] - exclude - 1];
   Q_CHECK_PTR(sval);
   for (i=0; i<run_inf.scans[selected_cell][selected_lambda] - exclude - 1; i++)
   {
      dcdt_points[i] = new double [points];
      Q_CHECK_PTR(dcdt_points[i]);
      sval[i] = new double [points];
      Q_CHECK_PTR(sval[i]);
   }
   old_local_scans = run_inf.scans[selected_cell][selected_lambda] - exclude - 1;
   plots = new uint [run_inf.scans[selected_cell][selected_lambda] - exclude - 1];
   Q_CHECK_PTR(plots);
   setup_already = true;
   return(0);
}

void dcdt_dat_W::cleanup()
{
   delete [] frequency;
   for (i=0; i<old_local_scans; i++)
   {
      delete [] dcdt_points[i];
      delete [] sval[i];
   }
   delete [] dcdt_points;
   delete [] sval;
   delete [] plots;
   setup_already = false;
}

void dcdt_dat_W::view()
{
   if (step == 0)
   {
      QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n      Click on \"Load Data\"\n"));
      return;
   }
   QString filestr, filename, temp;
   filename = USglobal->config_list.result_dir.copy();
   filename.append("/");
   filename.append(run_inf.run_id);
   temp.sprintf(".%d%d", selected_cell+1, selected_lambda+1);
   filestr = filename.copy();
   filestr.append(".dcdt_res");
   filestr.append(temp);
   write_res();
   //view_file(filestr);
   TextEdit *e;
   e = new TextEdit();
   e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
   e->load(filestr);
   e->show();

}

void dcdt_dat_W::write_dcdt_avg()
{
   QString filestr, filename, temp;
   filename = USglobal->config_list.result_dir.copy();
   filename.append("/");
   filename.append(run_inf.run_id);
   temp.sprintf(".%d%d", selected_cell+1, selected_lambda+1);
   filestr = filename.copy();
   filestr.append(".dcdt_avg");
   filestr.append(temp);
   QFile dcdt_f(filestr);
   int temp_scans = run_inf.scans[selected_cell][selected_lambda] - exclude - 1;
   US_Average *avg1;
   avg1 = new US_Average(points, temp_scans, &sval, &dcdt_points, "dcdt", htmlDir, title, selected_cell, selected_lambda);
   unsigned int i;   
   if (dcdt_f.open(IO_WriteOnly | IO_Translate))
   {
      QTextStream ts(&dcdt_f);
      ts.setf(QTextStream::scientific);
      for (i=0; i<points; i++)
      {
         ts << avg1->sval[i] << "\t" << avg1->sum[i] << "\n";
      }
      dcdt_f.close();
   }
   delete avg1;
}

void dcdt_dat_W::write_dcdt_scans()
{

   QString filestr, filename, temp;
   unsigned int temp_scans = run_inf.scans[selected_cell][selected_lambda] - exclude - 2;
   filename = USglobal->config_list.result_dir.copy();
   filename.append("/");
   filename.append(run_inf.run_id);
   temp.sprintf(".%d%d", selected_cell+1, selected_lambda+1);
   filestr = filename.copy();
   filestr.append(".dcdt_scans");
   filestr.append(temp);
   QFile scans_f(filestr);
   if (scans_f.open(IO_WriteOnly | IO_Translate))
   {
      QTextStream ts(&scans_f);
      ts.setf(QTextStream::scientific);
      for (i=0; i<points; i++)
      {
         ts << radius[i] << "\t";
         for (j=0; j<temp_scans; j++)
         {
            ts << dcdt_points[j][i] << "\t";
         }
         ts << dcdt_points[temp_scans][i] << "\n";
      }
      scans_f.close();
   }
}

void dcdt_dat_W::write_dcdt_sval()
{

   QString filestr, filename, temp;
   unsigned int temp_scans = run_inf.scans[selected_cell][selected_lambda] - exclude - 2;
   filename = USglobal->config_list.result_dir.copy();
   filename.append("/");
   filename.append(run_inf.run_id);
   temp.sprintf(".%d%d", selected_cell+1, selected_lambda+1);
   filestr = filename.copy();
   filestr.append(".dcdt_sval");
   filestr.append(temp);
   QFile scans_f(filestr);
   if (scans_f.open(IO_WriteOnly | IO_Translate))
   {
      QTextStream ts(&scans_f);
      ts.setf(QTextStream::scientific);
      for (i=0; i<points; i++)
      {
         for (j=0; j<temp_scans; j++)
         {
            ts << sval[j][i] << "\t" << dcdt_points[j][i] << "\t";
         }
         ts << sval[temp_scans][i] << "\t" << dcdt_points[temp_scans][i] << "\n";
      }
      scans_f.close();
   }
}

void dcdt_dat_W::write_res()
{
   QString filestr, filename, temp;
   filename = USglobal->config_list.result_dir.copy();
   filename.append("/");
   filename.append(run_inf.run_id);
   temp.sprintf(".%d%d", selected_cell+1, selected_lambda+1);
   filestr = filename.copy();
   filestr.append(".dcdt_res");
   filestr.append(temp);
   QFile res_f(filestr);
   if (res_f.open(IO_WriteOnly | IO_Translate))
   {
      QTextStream ts(&res_f);
      ts << "***************************************************\n";
      ts << tr("*       Time - Derivative (dC/dt) Analysis        *\n");
      ts << "***************************************************\n\n\n";
      ts << tr("Data Report for Run \"") << run_inf.run_id << tr("\", Cell ") << (selected_cell + 1) 
         << tr(", Wavelength ") << (selected_lambda + 1) << "\n\n";
      ts << tr("Detailed Run Information:\n\n");
      ts << tr("Cell Description:        ") << run_inf.cell_id[selected_cell] << "\n";
      ts << tr("Raw Data Directory:      ") << run_inf.data_dir << "\n";
      ts << tr("Rotor Speed:             ") << run_inf.rpm[first_cell][0][0]  << " rpm\n";
      ts << tr("Average Temperature:     ") << run_inf.avg_temperature << " ºC\n" ;
      if (run_inf.temperature_check == 1)
      {
         ts << tr("Temperature Variation:   Within Tolerance\n");
      }
      if (run_inf.temperature_check == -1)
      {
         ts << tr("Temperature Variation: (!) OUTSIDE TOLERANCE (!)\n");
      }
      i = (unsigned int) (run_inf.time_correction / 60);
      j = (unsigned int) (0.5 + (run_inf.time_correction - (i * 60)));
      ts << tr("Time Correction:         ") << i << tr(" minute(s) ") << j << tr(" second(s)\n");
      i = (unsigned int) (run_inf.duration / 3600);
      j = (unsigned int) (0.5 + (run_inf.duration - i * 3600) / 60);
      ts << tr("Run Duration:            ") << i << tr(" hour(s) ") << j << tr(" minute(s)\n");
      ts << tr("Wavelength:              ") << run_inf.wavelength[selected_cell][selected_lambda] << " nm\n";
      ts << tr("Baseline Absorbance:     ") << run_inf.baseline[selected_cell][selected_lambda] << " OD\n";
      ts << tr("Meniscus Position:       ") << run_inf.meniscus[selected_cell] << " cm\n";
      ts << tr("Edited Data starts at:   ") << run_inf.range_left[selected_cell][selected_lambda][0] << " cm\n";
      ts << tr("Edited Data stops at:    ") << run_inf.range_right[selected_cell][selected_lambda][0] << " cm\n\n\n";
      ts << tr("Hydrodynamic Settings:\n\n");
      ts << tr("Viscosity correction:    ") << viscosity << "\n" ;
      ts << tr("Viscosity (absolute):    ") << viscosity_tb << "\n" ;
      ts << tr("Density correction:      ") << density << " g/ccm\n" ;
      ts << tr("Density (absolute):      ") << density_tb << " g/ccm\n" ;
      ts << tr("Vbar:                    ") << vbar << " ccm/g\n" ;
      ts << tr("Vbar corrected for 20ºC: ") << vbar20 << " ccm/g\n" ;
      ts << tr("Buoyancy (Water, 20ºC) : ") << buoyancyw << "\n" ;
      ts << tr("Buoyancy (absolute)      ") << buoyancyb << "\n" ;
      ts << tr("Correction Factor:       ") << correction << "\n\n\n";
      ts << tr("Data Analysis Settings:\n\n");
      ts << tr("Smoothing Frame:         ") << smooth << "\n";
      ts << tr("Analyzed Boundary:       ") << bd_range << " %\n";
      ts << tr("Boundary Position:       ") << bd_position << " %\n";
      ts << tr("Early Scans skipped:     ") << exclude << tr(" Scans \n\n\n");
      ts << tr("Scan Information: \n\n");
      ts << tr("Scan:     Corrected Time:  Plateau Concentration: \n\n");
      for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
      {
         if (i+1<10)
         {
            ts << "  " << (i+1) << ":    ";
         }
         else if (i+1 < 100 && i+1 > 9)
         {
            ts << " " << (i+1) << ":    ";
         }
         else if (i+1 > 99)
         {
            ts << (i+1) << ":    ";
         }
         k = (unsigned int) (run_inf.time[selected_cell][selected_lambda][i]/ 60);
         j = (unsigned int) (0.5 + (run_inf.time[selected_cell][selected_lambda][i] - (k * 60)));
         ts.width(5);
         ts << k << tr(" min");
         ts.width(3);
         ts << j << tr(" sec     ");
         ts.width(11);
         ts << run_inf.plateau[selected_cell][selected_lambda][i] << " OD\n";
      }
      res_f.close();
   }
}



