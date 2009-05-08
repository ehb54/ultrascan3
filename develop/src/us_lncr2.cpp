#include "../include/us_lncr2.h"
#ifndef WIN32
#include <unistd.h>
#endif

US_lncr2_W::US_lncr2_W(QWidget *p, const char *name) : Data_Control_W(2, p, name)
{
   //   pb_second_plot->setText("Distribution Plot");

   xpos = border;
   ypos = 438;

   baseline_lbl = new QLabel(tr(" Adjust Baseline:"), this);
   Q_CHECK_PTR(baseline_lbl);
   baseline_lbl->setAlignment(AlignLeft|AlignVCenter);
   baseline_lbl->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   baseline_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   baseline_lbl->setGeometry(xpos, ypos, buttonw, buttonh);

   xpos += buttonw + spacing;

   baseline = 0.0;
   baseline_counter= new QwtCounter(this);
   Q_CHECK_PTR(baseline_counter);
   baseline_counter->setRange(-100, 100, 0.0001);
   baseline_counter->setValue(0.0);
   baseline_counter->setNumButtons(3);
   baseline_counter->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   baseline_counter->setGeometry(xpos, ypos, buttonw, buttonh);
   //   connect(baseline_counter, SIGNAL(buttonReleased(double)), SLOT(update_baseline(double)));
   connect(baseline_counter, SIGNAL(valueChanged(double)), SLOT(update_baseline(double)));
   setup_GUI();
}

US_lncr2_W::~US_lncr2_W()
{
}

void US_lncr2_W::setup_GUI()
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
   QGridLayout *listboxes = new QGridLayout(3,3,2);
   listboxes->addWidget(cell_select,0,0);
   listboxes->addWidget(lambda_select,0,1);
   listboxes->addWidget(channel_select,0,2);

   subGrid1->addMultiCell(listboxes,j,j+2,0,3);
      
   rows = 10, columns = 4, spacing = 2, j=0;
   QGridLayout * subGrid2 = new QGridLayout(rows, columns, spacing);
   for (int i=0; i<rows; i++)
   {
      subGrid2->setRowSpacing(i, 26);
   }

   subGrid2->addMultiCellWidget(banner2,j,j,0,3);
   j++;
   subGrid2->addWidget(pb_density,j,0);
   subGrid2->addWidget(density_le,j,1);
   subGrid2->addWidget(pb_vbar,j,2);
   subGrid2->addWidget(vbar_le,j,3);
   j++;
   subGrid2->addMultiCellWidget(pb_reset,j,j,0,1);
   j++;
   subGrid2->addMultiCellWidget(baseline_lbl,j,j,0,1);
   subGrid2->addMultiCellWidget(baseline_counter,j,j,2,3);
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

void US_lncr2_W::second_plot()
{
   if (step == 0)
   {
      QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n      Click on \"Load Data\"\n"));
      return;
   }
}

void US_lncr2_W::save()
{
   if (step == 0)
   {
      QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n      Click on \"Load Data\"\n"));
      return;
   }
   write_lncr2();
   write_res();
}

void US_lncr2_W::update_baseline(double val)
{
   baseline = val;
   plot_analysis();
}

void US_lncr2_W::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/lncr2.html");
}

int US_lncr2_W::plot_analysis()
{
   QString s1, s2, s3;
   already = true;
   bool break_flag = false;
   newcount      = new int [run_inf.scans[selected_cell][selected_lambda]];
   runs         = new float [run_inf.scans[selected_cell][selected_lambda]];
   mw            = new float [run_inf.scans[selected_cell][selected_lambda]];
   slope         = new double [run_inf.scans[selected_cell][selected_lambda]];
   intercept   = new double [run_inf.scans[selected_cell][selected_lambda]];
   sigma         = new double [run_inf.scans[selected_cell][selected_lambda]];
   correl      = new double [run_inf.scans[selected_cell][selected_lambda]];
   tempx       = new double * [run_inf.scans[selected_cell][selected_lambda]];
   tempy       = new double * [run_inf.scans[selected_cell][selected_lambda]];
   lines_x       = new double [2];
   lines_y       = new double [2];
   lines         = new uint [run_inf.scans[selected_cell][selected_lambda]];
   line_fits   = new uint [run_inf.scans[selected_cell][selected_lambda]];
   unsigned int i, j, count, start;
   for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
   {
      tempx[i] = new double [points];
      tempy[i] = new double [points];
   }
   
   analysis_plot->clear();
   s1.sprintf(tr(": Cell %d (%d nm) - ln(Absorbance) vs. r^2 Plot"), selected_cell+1, run_inf.wavelength[selected_cell][selected_lambda]);
   s2 = tr("Run ");
   s2.append(run_inf.run_id);
   s2.append(s1);
   analysis_plot->setTitle(s2);
   analysis_plot->setAxisTitle(QwtPlot::xBottom, tr("Radius^2"));
   analysis_plot->setAxisTitle(QwtPlot::yLeft, tr("ln(Absorbance)"));
   /*
     analysis_plot->enableGridXMin();
     analysis_plot->enableGridYMin();
     analysis_plot->setGridMajPen(QPen(colorGroup().light(), 0, DotLine));
     analysis_plot->setGridMinPen(QPen(colorGroup().mid(), 0 , DotLine));
   */
   for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
   {
      j=0;
      count=0;
      omega_s = pow((run_inf.rpm[selected_cell][selected_lambda][i]/30)*M_PI, 2);
      while (absorbance[i][j] < lower_limit)
      {
         j++;
      }
      start = j;
      while (absorbance[i][j] < upper_limit && j < points)
      {
         j++;
         count++;
      }
      newcount[i] = 0;
      for (j=0; j<count; j++)   // log of raw data
      {
         if ((absorbance[i][start + j]-baseline) > 0) // only if the data is > 0 can we take the log, so ignore other points.
         {
            tempx[i][newcount[i]] = pow(radius[start + j], 2) - pow(radius[start], 2); // increment X only for the good points.
            tempy[i][newcount[i]] = log(absorbance[i][j + start]-baseline); 
            newcount[i]++;   //counts only the good points to be used for plot
         }
      }
      count = newcount[i];
      if (newcount[i] > 5)
      {
         linefit(&tempx[i], &tempy[i], &slope[i], &intercept[i], &sigma[i], &correl[i], newcount[i]);
         vbar = vbar20 + 4.25e-4 * (run_inf.temperature[selected_cell][selected_lambda][i] - 20);
         calc_correction(run_inf.temperature[selected_cell][selected_lambda][i]);
         mw[i] = (slope[i] * 2 * R * (K0+run_inf.temperature[selected_cell][selected_lambda][i]))
            /(omega_s * buoyancyb);
         runs[i] = 0;
         for (j=1; j<count; j++)
         {
            if (((tempy[i][j] > slope[i]*tempx[i][j]+intercept[i]) 
                 && (tempy[i][j-1] < slope[i]*tempx[i][j-1]+intercept[i]))
                || ((tempy[i][j] < slope[i]*tempx[i][j]+intercept[i]) 
                    && (tempy[i][j-1] > slope[i]*tempx[i][j-1]+intercept[i])))
            {
               runs[i]++;
            }
         }
         lines_x[0] = tempx[i][0];
         lines_x[1] = tempx[i][newcount[i]-1];
         lines_y[0] = slope[i]*lines_x[0] + intercept[i];
         lines_y[1] = slope[i]*lines_x[1] + intercept[i];

         lines[i] = analysis_plot->insertCurve("ln(C) vs. r^2");
         analysis_plot->setCurveStyle(lines[i], QwtCurve::Lines);
         analysis_plot->setCurvePen(lines[i], yellow);
         analysis_plot->setCurveData(lines[i], tempx[i], tempy[i], newcount[i]);

         line_fits[i] = analysis_plot->insertCurve("Linear Fit");
         analysis_plot->setCurveStyle(line_fits[i], QwtCurve::Lines);
         analysis_plot->setCurvePen(line_fits[i], cyan);
         analysis_plot->setCurveData(line_fits[i], lines_x, lines_y, 2);
      }
      else
      {
         QString str;
         str.sprintf(tr("There aren't enough points in scan %d\n"), i+1);
         str.append( tr("to do a useful equilibrium analysis!\n\n"
                        "Try to reduce the baseline or increase\n"
                        "the boundary range."));
         QMessageBox::warning(0, tr("Attention:"), str, 1,0,0);
      }

   }
   if (break_flag)
   {
      analysis_plot->clear();
   }
   {
      analysis_plot->replot();
   }
   return(0);
}

void US_lncr2_W::view()
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
   temp.sprintf(".%d%d%d", selected_cell+1, selected_lambda+1, selected_channel+1);
   filestr = filename.copy();
   filestr.append(".lncr2_res");
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

void US_lncr2_W::write_lncr2()
{
   QString filestr, filename, temp;
   filename = USglobal->config_list.result_dir.copy();
   filename.append("/");
   filename.append(run_inf.run_id);
   temp.sprintf(".%d%d%d", selected_cell+1, selected_lambda+1, selected_channel+1);
   filestr = filename.copy();
   filestr.append(".lncr2_dat");
   filestr.append(temp);
   QFile lncr2_f(filestr);
   unsigned int i;
   if (lncr2_f.open(IO_WriteOnly | IO_Translate))
   {
      QTextStream ts(&lncr2_f);
      //
      // Arrange the file as follows: First, write to the file all apparent sedimentation
      // values, for each scan time one row which contains all divisions, starting at the 
      // lowest concentration. The first column has the times of each scan, then follow
      // the sedimentation coefficients for each division for that time:
      //
      ts << tr("Scan:\tMol.W.:\tSlope:\tIntercept:\tSigma:\tCorrelation:\tPoints:\tRuns:\tRuns_Points_Ratio:\n");
      for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
      {   
         ts << (i+1) << "\t" << mw[i] << "\t" << slope[i] << "\t" << intercept[i] << "\t" << sigma[i] 
            << "\t" << correl[i] << "\t" << newcount[i] << "\t" << runs[i] << "\t" << 100*(runs[i]/newcount[i]) 
            << "\n";
      }
      lncr2_f.close();
   }
}

void US_lncr2_W::write_res()
{
   int h;
   QString filestr, filename, temp;
   filename = USglobal->config_list.result_dir.copy();
   filename.append("/");
   filename.append(run_inf.run_id);
   temp.sprintf(".%d%d%d", selected_cell+1, selected_lambda+1, selected_channel+1);
   filestr = filename.copy();
   filestr.append(".lncr2_res");
   filestr.append(temp);
   QFile res_f(filestr);
   if (res_f.open(IO_WriteOnly | IO_Translate))
   {
      QTextStream ts(&res_f);
      ts << "***************************************************\n";
      ts << tr("*       ln(C) vs. R^2 Equilibrium Analysis        *\n");
      ts << "***************************************************\n\n\n";
      ts << tr("Data Report for Run \"") << run_inf.run_id << tr("\", Cell ") << (selected_cell + 1) 
         << tr(", Wavelength ") << (selected_lambda + 1) << tr(", Channel ") << (selected_channel+1) << "\n\n";
      ts << tr("Detailed Run Information:\n\n");
      ts << tr("Cell Description:        ") << run_inf.cell_id[selected_cell] << "\n";
      ts << tr("Raw Data Directory:      ") << run_inf.data_dir << "\n";
      ts << tr("Rotor Speed:             ") << run_inf.rpm[first_cell][0][0]  << " rpm\n";
      i = (unsigned int) (run_inf.duration / 3600);
      j = (unsigned int) (0.5 + (run_inf.duration - i * 3600) / 60);
      ts << tr("Run Duration:            ") << i << tr(" hours ") << j << tr(" minutes\n");
      ts << tr("Wavelength:              ") << run_inf.wavelength[selected_cell][selected_lambda] << " nm\n";
      ts << tr("Selected Baseline:       ") << baseline << " OD\n";
      ts << tr("Edited Data starts at:   ") << run_inf.range_left[selected_cell][selected_lambda][selected_channel] << " cm\n";
      ts << tr("Edited Data stops at:    ") << run_inf.range_right[selected_cell][selected_lambda][selected_channel] << " cm\n";
      ts << tr("Vbar corrected for 20ºC: ") << vbar20 << " ccm/g\n\n\n" ;
      ts << tr("Data Analysis Settings:\n\n");
      ts << tr("Smoothing Frame:         ") << smooth << "\n";
      ts << tr("Analyzed Boundary:       ") << bd_range << " %\n";
      ts << tr("Boundary Position:       ") << bd_position << " %\n\n\n";
      ts << tr("Scan Information:\n\n");
      for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
      {
         ts << "---------------------------------------------\n";
         ts << tr("Scan ") << (i+1) << ":\n";
         ts << "---------------------------------------------\n";
         ts << tr("Time of scan:           ");
         h = (unsigned int) (run_inf.time[selected_cell][selected_lambda][i] / 3600);
         k = (unsigned int) ((run_inf.time[selected_cell][selected_lambda][i] - h * 3600) / 60);
         j = (unsigned int) (0.5 + (run_inf.time[selected_cell][selected_lambda][i] - (k * 60) - (h * 3600)));
         ts.width(3);
         ts << h << tr(" hr ");
         ts.width(2);
         ts << k << tr(" min ");
         ts.width(2);
         ts << j << tr(" sec\n");
         ts.width(11);
         ts << tr("Molecular weight:        ") << mw[i] << tr(" Dalton\n");
         ts << tr("Standard Deviation:      ") << sigma[i] << "OD\n";
         ts << tr("Correlation:             ") << correl[i] << "\n";
         ts << tr("Points in this scan:     ") << newcount[i] << "\n";
         ts << tr("Runs in fit:             ") << runs[i] << " (" << 100*(runs[i]/newcount[i]) << "%)\n";
         ts << tr("Estim. Reference Abs.:   ") << exp(intercept[i]) << " OD\n";
         calc_correction(run_inf.temperature[selected_cell][selected_lambda][i]);
         ts << tr("\nHydrodynamic Settings for Scan ") << (i+1) << ":\n";
         ts << tr("Temperature at scantime: ") << run_inf.temperature[selected_cell][selected_lambda][i] << "ºC\n";
         ts << tr("Density correction:      ") << density << " g/ccm\n" ;
         ts << tr("Density (absolute):      ") << density_tb << " g/ccm\n" ;
         vbar = vbar20 + 4.25e-4 * (run_inf.temperature[selected_cell][selected_lambda][i] - 20);
         ts << tr("Vbar at ") << run_inf.temperature[selected_cell][selected_lambda][i] << "ºC:            " << vbar << " ccm/g\n";
         ts << tr("Buoyancy (Water, 20ºC):  ") << buoyancyw << "\n" ;
         ts << tr("Buoyancy (absolute):     ") << buoyancyb << "\n" ;
      }
      res_f.close();
   }
}

void US_lncr2_W::reset()
{
   if (step == 0)
   {
      QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n      Click on \"Load Data\"\n"));
      return;
   }
   reset_flag = true;
   baseline = 0.0;
   baseline_counter->setValue(0.0);
   smoothing_counter->setValue(1);
   smoothing_counter->setRange(1, 55, 2);
   update_smoothing(1.0);
   switch (run_type)
   {
   case 1:
      {
         update_boundary_range(90.0);
         bd_range=90;
         break;
      }
   case 2:
      {
         update_boundary_range(100.0);
         bd_range=100;
         break;
      }
   }
   range_counter->setValue(bd_range);
}
