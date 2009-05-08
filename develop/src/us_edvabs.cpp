#include "../include/us_edvabs.h"

EditAbsVeloc_Win::EditAbsVeloc_Win(QWidget *p , const char *name)
   : EditData_Win(p, name)
{
   edit_type = 1;
   extension1 = "RA";
   extension2 = "ra";
   edit_plt->setTitle(tr("Absorbance Velocity Data"));
   edit_plt->setAxisTitle(QwtPlot::yLeft, tr("Absorbance"));

   absorbance_integral = NULL;
   residuals = NULL;
   residuals_defined = false;

   run_inf.exp_type.velocity = true;
   run_inf.exp_type.equilibrium = false;
   run_inf.exp_type.diffusion = false;
   run_inf.exp_type.simulation = false;
   run_inf.exp_type.interference = false;
   run_inf.exp_type.absorbance = true;
   run_inf.exp_type.fluorescence = false;
   run_inf.exp_type.intensity = false;
   run_inf.exp_type.wavelength = false;

   pb_subtract = new QPushButton(tr("Subtract Residuals"), this);
   Q_CHECK_PTR(pb_subtract);
   pb_subtract->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_subtract->setAutoDefault(false);
   pb_subtract->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_subtract->setEnabled(false);
   connect(pb_subtract, SIGNAL(clicked()), SLOT(subtract_residuals()));
   pb_subtract->setEnabled(false);

   pb_continue = new QPushButton(tr("Continue"), this);
   Q_CHECK_PTR(pb_continue);
   pb_continue->setAutoDefault(false);
   pb_continue->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_continue->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_continue->setEnabled(false);
   connect(pb_continue, SIGNAL(clicked()), SLOT(next_step()));

   ct_order = new QwtCounter(this);
   ct_order->setNumButtons(1);
   ct_order->setRange(0, 20, 1);
   ct_order->setValue(4);
   ct_order->setEnabled(false);
   ct_order->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   ct_order->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   connect(ct_order, SIGNAL(valueChanged(double)), SLOT(fit_ls(double)));

   lbl_subtract_ri_noise = new QLabel(tr("Subtract RI Noise:"), this);
   lbl_subtract_ri_noise->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_subtract_ri_noise->setAlignment(AlignCenter|AlignVCenter);
   lbl_subtract_ri_noise->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_subtract_ri_noise->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   connect(this, SIGNAL(absorbance_changed()), this, SLOT(update_oldscan()));
   setup_GUI();
}

EditAbsVeloc_Win::~EditAbsVeloc_Win()
{
}

void EditAbsVeloc_Win::setup_GUI()
{
   int j=0;
   int rows = 8, columns = 5, spacing = 2;

   QGridLayout * background = new QGridLayout(this,3,1,spacing*2);
   QGridLayout * subGrid1 = new QGridLayout(rows, columns, spacing);
   for (int i=0; i<rows; i++)
   {
      subGrid1->setRowSpacing(i, 26);
   }
   subGrid1->setColStretch(2,2);
   subGrid1->setColStretch(4,2);

   subGrid1->addWidget(pb_select_dir,j,0);
   subGrid1->addMultiCellWidget(lbl_directory,j,j,1,2);
   subGrid1->addWidget(pb_exsingle,j,3);
   subGrid1->addWidget(cnt_exsingle,j,4);
   j++;
   subGrid1->addWidget(pb_details,j,0);
   subGrid1->addMultiCellWidget(lbl_details,j,j,1,2);
   subGrid1->addWidget(pb_exrange,j,3);
   subGrid1->addWidget(cnt_exrange,j,4);
   j++;
   subGrid1->addWidget(pb_accept,j,0);
   subGrid1->addWidget(lbl_run_id1,j,1);
   subGrid1->addWidget(lbl_run_id2,j,2);
   subGrid1->addWidget(pb_editscan,j,3);
   subGrid1->addWidget(cnt_edit,j,4);
   j++;
   subGrid1->addWidget(pb_exclude_profile,j,0);
   subGrid1->addWidget(lbl_current_cell1,j,1);
   subGrid1->addWidget(lbl_current_cell2,j,2);
   QHBoxLayout *status_bar = new QHBoxLayout(2);
   status_bar->addWidget(lbl_status1);
   status_bar->addWidget(progress);
   subGrid1->addMultiCell(status_bar,j,j,3,4);
   j++;
   subGrid1->addWidget(pb_invert,j,0);
   subGrid1->addMultiCellWidget(lbl_instructions1,j,j+5,1,1);
   subGrid1->addMultiCellWidget(lbl_instructions2,j,j+5,2,4);
   j++;
   subGrid1->addWidget(pb_sub_baseline,j,0);
   j++;
   subGrid1->addWidget(pb_spikes,j,0);
   j++;
   subGrid1->addWidget(pb_help,j,0);
   j++;
   subGrid1->addWidget(pb_close,j,0);

   QGridLayout * subGrid2 = new QGridLayout(2, 4);

   subGrid2->addWidget(lbl_subtract_ri_noise, 0, 0);
   subGrid2->addWidget(ct_order, 0, 1);
   subGrid2->addWidget(pb_subtract, 0, 2);
   subGrid2->addWidget(pb_continue, 0, 3);

   subGrid2->addWidget(lbl_centerpiece, 1, 0);
   subGrid2->addWidget(cmbb_centerpiece, 1, 1);
   subGrid2->addWidget(lbl_rotor, 1, 2);
   subGrid2->addWidget(cmbb_rotor, 1, 3);

   background->addWidget(edit_plt,0,0);
   background->addLayout(subGrid1,1,0);
   background->addLayout(subGrid2,2,0);

   qApp->processEvents();
   QRect r = background->cellGeometry(0, 0);

   global_Xpos += 30;
   global_Ypos += 30;

   this->setGeometry(global_Xpos, global_Ypos, r.width(), r.height()+250);
}

void EditAbsVeloc_Win::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/edit_veloc_abs.html");
}

void EditAbsVeloc_Win::get_x(const QMouseEvent &e)
{
   uint line;
   double absorbance_integral=0;
   unsigned int i, j, scan=0, counter=0;
   int ik, jk, kk;
   double r[2], y[2], maximum=-100, sum;
   double m, b, newpoint=0;
   float rad;
   unsigned int temp_points = 0;
   vector <bool> include_flag;
   bool excludeScans = false;
   QString str, str1;
   include_flag.clear();
   current_R = (edit_plt->invTransform(QwtPlot::xBottom, e.x()));
   r[0] = current_R;
   r[1] = current_R;
   y[0] = ymin + ymax/20;
   y[1] = ymax - ymax/20;
   line = edit_plt->insertCurve("Line");
   edit_plt->setCurvePen(line, QPen(white, 2));
   edit_plt->setCurveData(line, r, y, 2);
   edit_plt->replot();


   switch (step)
   {
   case 9:   // pick baseline
      {
         scan = run_inf.scans[cell][lambda]-1;
         j=find_indexf(current_R, &radius[scan], points[scan]);
         sum = 0;
         if (j > points[scan]+4)
         {
            str.sprintf   (tr("There appears to be a problem at then end of scan %d of your\n"), scan+1);
            str1.sprintf(tr("data that showed up in module edvabs.cpp:get_x::CASE %d.\n\n"), step);
            str.append(str1);
            str.append    (tr("Please manually inspect the file for:\n\n"));
            str1.sprintf(tr("   Scan:         %d\n"), scan+1);
            str.append(str1);
            str1.sprintf(tr("   Wavelength:   %d\n"), lambda);
            str.append(str1);
            str1.sprintf(tr("   Cell:         %d\n\n"), cell+1);
            str.append(str1);
            str.append    (tr("to make sure it doesn't contain any corruption.\n\n"));
            str.append    (tr("If the file doesn't have any corruption, you can try\n"));
            str.append    (tr("to re-edit your data and pick a point that is closer\n"));
            str.append    (tr("to the meniscus."));
            QMessageBox::message(tr("Attention:"), str);
            return;
         }
         if (j < 4)
         {
            str.sprintf   (tr("There appears to be a problem at the beginning of scan %d of your\n"), scan+1);
            str1.sprintf(tr("data that showed up in module edvabs.cpp:get_x::CASE %d.\n\n"), step);
            str.append(str1);
            str.append    (tr("Please manually inspect the file for:\n\n"));
            str1.sprintf(tr("   Scan:         %d\n"), scan+1);
            str.append(str1);
            str1.sprintf(tr("   Wavelength:   %d\n"), lambda);
            str.append(str1);
            str1.sprintf(tr("   Cell:         %d\n\n"), cell+1);
            str.append(str1);
            str.append    (tr("to make sure it doesn't contain any corruption.\n\n"));
            str.append    (tr("If the file doesn't have any corruption, you can try\n"));
            str.append    (tr("to re-edit your data and pick a point that is a little\n"));
            str.append    (tr("further away from the meniscus."));
            QMessageBox::message(tr("Attention:"), str);
            return;
         }
         for (i=j-5; i<=j+5; i++)   // average 5 points to the left and 5 points to the right
         {
            sum += absorbance[scan][i];
         }
         run_inf.centerpiece[cell] = centerpiece.serial_number;
         run_inf.baseline[cell][lambda] = sum/11;
         unsigned int count;
         QString filename;
         filename = USglobal->config_list.result_dir.copy();
         filename.append("/");
         filename.append(run_inf.run_id);
         run_inf.delta_r = (float) 0.001;
         str.sprintf(".veloc.%d%d",cell+1, lambda+1);
         filename.append(str);
         QFile f(filename);
         f.open( IO_WriteOnly );
         QDataStream s( &f );
         // delete all excluded scans from oldscan, keep only those scans whose time matches:
         for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
         {
            while (run_inf.time[cell][lambda][scan] != oldscan[scan].time)
            {
               oldscan.erase(oldscan.begin() + scan);
            }
         }
         run_inf.points[cell][lambda][current_channel] = 1 + (unsigned int) ((run_inf.range_right[cell][lambda][current_channel]
                                                                              - run_inf.range_left[cell][lambda][current_channel])/run_inf.delta_r);
         for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
         {
            count = 0;
            for (i=0; i<run_inf.points[cell][lambda][current_channel]; i++)
            {
               rad = run_inf.range_left[cell][lambda][current_channel] + i * run_inf.delta_r;
               //cout << rad << ": ";
               // only compare the first 3 significant digits
               // Sometimes there are increments smaller than 0.001, so we need to skip those
               while ((int) (oldscan[scan].rad[count] * 1000 + 0.5) < (int) (rad * 1000 + 0.5)
                      && rad <= run_inf.range_right[cell][lambda][current_channel]
                      && count <= oldscan[scan].rad.size())
               {
                  count ++;
               }
               if ((unsigned int) (oldscan[scan].rad[count] * 1000 + 0.5) == (unsigned int) (rad * 1000 + 0.5))
               {
                  s << oldscan[scan].abs[count];
                  //cout << "old point: " << oldscan[scan].rad[count] << ", " << oldscan[scan].abs[count] << endl;
                  //count ++;
               }
               else
               {
                  // we are in the middle of the scan, and we we need to linearly interpolate missing datapoints (newpoint = m * rad + b)
                  m = (oldscan[scan].abs[count] - oldscan[scan].abs[count-1]) /
                     (oldscan[scan].rad[count] - oldscan[scan].rad[count-1]);
                  b = oldscan[scan].abs[count] - m * oldscan[scan].rad[count];
                  newpoint = m * rad + b;
                  s << (float) newpoint;
                  //cout << (unsigned int) (oldscan[scan].rad[count] * 1000 + 0.5) << " and " << (unsigned int) (rad * 1000 + 0.5) << " are not the same\n";
                  //cout << oldscan[scan].rad[count] << ", " << oldscan[scan].rad[count-1] << ", " << oldscan[scan].abs[count] << ", " << oldscan[scan].abs[count] << endl;
               }
            }
         }
         f.flush();
         f.close();
         cleanup_dataset();
         load_dataset();
         return;
      }
   case 8:   // pick plateau x
      {
         for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
         {
            j=find_indexr(current_R, &radius[scan], points[scan]);
            sum = 0;
            if (j > points[scan]+4)
            {
               str.sprintf   (tr("There appears to be a problem with scan %d of your\n"), scan+1);
               str1.sprintf(tr("data that showed up in module edvabs.cpp:get_x::CASE %d.\n\n"), step);
               str.append(str1);
               str.append    (tr("Please manually inspect the file for:\n\n"));
               str1.sprintf(tr("   Scan:         %d\n"), scan+1);
               str.append(str1);
               str1.sprintf(tr("   Wavelength:      %d\n"), lambda);
               str.append(str1);
               str1.sprintf(tr("   Cell:         %d\n\n"), cell+1);
               str.append(str1);
               str.append    (tr("to make sure it doesn't contain any corruption.\n\n"));
               str.append    (tr("If the file doesn't have any corruption, you can try\n"));
               str.append    (tr("to re-edit your data and pick a point that is closer\n"));
               str.append    (tr("to the meniscus."));
               QMessageBox::message(tr("Attention:"), str);
               return;
            }
            if (j < 4)
            {
               str.sprintf   (tr("There appears to be a problem with scan %d of your\n"), scan+1);
               str1.sprintf(tr("data that showed up in module edvabs.cpp:get_x::CASE %d.\n\n"), step);
               str.append(str1);
               str.append    (tr("Please manually inspect the file for:\n\n"));
               str1.sprintf(tr("   Scan:         %d\n"), scan+1);
               str.append(str1);
               str1.sprintf(tr("   Wavelength:   %d\n"), lambda);
               str.append(str1);
               str1.sprintf(tr("   Cell:         %d\n\n"), cell+1);
               str.append(str1);
               str.append    (tr("to make sure it doesn't contain any corruption.\n\n"));
               str.append    (tr("If the file doesn't have any corruption, you can try\n"));
               str.append    (tr("to re-edit your data and pick a point that is a little\n"));
               str.append    (tr("further away from the meniscus."));
               QMessageBox::message(tr("Attention:"), str);
               return;
            }
            for (i=j-5; i<=j+5; i++)   // average 5 points to the left and 5 points to the right
            {
               sum += absorbance[scan][i];
            }
            run_inf.plateau[cell][lambda][scan] = sum/11;
         }
         step=9;
         for (scan = 0; scan < run_inf.scans[cell][lambda]-1; scan++)
         {
            edit_plt->removeCurve(curve[scan]);
         }
         edit_plt->replot();
         lbl_instructions2->setText(tr("Please click on a point that describes best the baseline absorbance\n"));
         break;
      }
   case 7:  // do nothing, user accidentally clicked on screen
      {
         break;
      }
   case 6:  // pick range-right
      {
         i = (unsigned int) (current_R * 1000);   //make sure we only have 3 significant digits
         limit_right = 1e-3 * (float) i;
         run_inf.range_right[cell][lambda][current_channel] = limit_right;
         temp_points = 0;
         for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
         {
            temp_points += points[scan];
         }
         run_inf.point_density[cell][lambda][current_channel] = run_inf.scans[cell][lambda] * (limit_right - limit_left) / temp_points;
         include_flag.resize(run_inf.scans[cell][lambda]);
         excludeScans = false;
         str = tr("The following scan(s) appear to be truncated\nand have been excluded from the experiment:\n\n");
         for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
         {
            include_flag[scan] = true;
            ik = find_indexf(limit_left, &radius[scan], points[scan]);
            jk = find_indexr(limit_right, &radius[scan], points[scan]);
            if ((ik < 0) || (jk < 0) || (current_R > radius[scan][points[scan]-1]))
            {
               include_flag[scan] = false;
               excludeScans = true;
               str += str1.sprintf(tr("Scan %d, Wavelength %d, Cell %d\n"), scan+1, lambda+1, cell+1);
            }
            if (jk < ik)
            {
               QMessageBox::message(tr("Attention:"), tr("You need to pick first the left limit,\n"
                                                         "then the right. Please try again now..."));
               step = 5;
               return;
            }
            counter=0;
            absorbance_integral = 0;
            for (kk=ik; kk<jk; kk++)
            {
               radius[scan][counter] = radius[scan][kk]; // throw away points outside the limits
               if (centerpiece.sector == 3)
               {
                  absorbance_integral      += absorbance[scan][kk]
                     * ((radius[scan][kk+1] - radius[scan][kk]) + (radius[scan][kk] - radius[scan][kk-1]))/2;
                  absorbance[scan][counter] = absorbance_integral;
               }
               else
               {
                  absorbance[scan][counter] = absorbance[scan][kk];
               }
               counter++;
            }
            points[scan]=counter-1;
         }
         if (excludeScans)
         {
            str += str1.sprintf(tr("This error showed up in module edvabs.cpp:get_x::CASE %d."), step);
            final_exclude_profile(include_flag);
            QMessageBox::message(tr("Attention:"), str);
         }
         edit_plt->clear();
         edit_plt->setAxisAutoScale(QwtPlot::xBottom);
         edit_plt->setAxisAutoScale(QwtPlot::yLeft);
         plot_dataset();
         step=7;
         pb_spikes->setEnabled(true);
         ct_order->setEnabled(true);
         pb_continue->setEnabled(true);
         pb_subtract->setEnabled(true);

         lbl_instructions2->setText(tr("Now exclude and/or edit any scans, if necessary. Then you can subtract\n"
                                       "radially invariant noise by fitting the scan's integrals to a\n"
                                       "polynomial. Adjust the order of a polynomial fit to the scan\n"
                                       "integrals. Once you found a smooth polynomial to fit your integrals,\n"
                                       "subtract the residuals from each scan.\n"));
         break;
      }
   case 5:   // pick range-left
      {
         oldscan.clear();
         struct absscan temp_scan; // make a copy of the original data
         for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
         {
            temp_scan.rad.clear();
            temp_scan.abs.clear();
            temp_scan.time = run_inf.time[cell][lambda][scan];
            for (i=0; i<points[scan]; i++)
            {
               temp_scan.abs.push_back(absorbance[scan][i]);
               temp_scan.rad.push_back(radius[scan][i]);
            }
            oldscan.push_back(temp_scan);
         }
         i = (unsigned int) (current_R * 1000);   //make sure we only have 3 significant digits
         limit_left = 1e-3 * (float) i;
         run_inf.range_left[cell][lambda][current_channel] = limit_left;
         step = 6;
         lbl_instructions2->setText(tr("Please click near the bottom of the cell to define the right limit\n"
                                       "for the included data..."));
         break;
      }
   case 4:   //pick meniscus-right
      {
         excludeScans = false;
         limit_right = current_R;
         include_flag.resize(run_inf.scans[cell][lambda]);
         excludeScans = false;
         str = tr("The following scan(s) appear to be truncated\nand have been excluded from the experiment:\n\n");
         for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
         {
            include_flag[scan] = true;
            ik = find_indexf(limit_left, &radius[scan], points[scan]);
            jk = find_indexf(limit_right, &radius[scan], points[scan]);
            if ((ik < 0) || (jk < 0))
            {
               include_flag[scan] = false;
               excludeScans = true;
               str += str1.sprintf(tr("Scan %d, Wavelength %d, Cell %d\n"), scan+1, lambda+1, cell+1);
            }
            if (jk < ik)
            {
               QMessageBox::message(tr("Attention:"), tr("You need to pick first the left limit,\n"
                                                         "then the right. Please try again now..."));
               step = 3;
               return;
            }
            for (kk=ik; kk<jk; kk++)
            {
               if(max(maximum, absorbance[scan][kk]) > maximum)
               {
                  maximum = max(maximum, absorbance[scan][kk]);
                  run_inf.meniscus[cell] = radius[scan][kk];
               }
            }
         }
         if (excludeScans)
         {
            str += str1.sprintf(tr("This error showed up in module edvabs.cpp:get_x::CASE %d."), step);
            final_exclude_profile(include_flag);
            QMessageBox::message(tr("Attention:"), str);
         }
         edit_plt->clear();
         plot_dataset();
         str.sprintf(tr("Meniscus at %5.3f cm"), run_inf.meniscus[cell]);
         uint meniscus_label = edit_plt->insertMarker();
         edit_plt->setMarkerLabel(meniscus_label, str);
         edit_plt->setMarkerPos(meniscus_label, run_inf.meniscus[cell], maximum+(ymax-ymin)/10);
         edit_plt->setMarkerFont(meniscus_label, QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
         edit_plt->setMarkerPen(meniscus_label, QPen(cyan, 0, DashDotLine));

         r[0] = run_inf.meniscus[cell];
         r[1] = run_inf.meniscus[cell];
         y[0] = maximum-(ymax - ymin)/30;
         y[1] = maximum+(ymax - ymin)/30;
         line = edit_plt->insertCurve("Line");
         edit_plt->setCurvePen(line, QPen(white, 2));
         edit_plt->setCurveData(line, r, y, 2);
         edit_plt->replot();

         r[0] = run_inf.meniscus[cell]-(xmax-xmin)/100;
         r[1] = run_inf.meniscus[cell]+(xmax-xmin)/100;
         y[0] = maximum;
         y[1] = maximum;
         line = edit_plt->insertCurve("Line");
         edit_plt->setCurvePen(line, white);
         edit_plt->setCurvePen(line, QPen(white, 2));
         edit_plt->setCurveData(line, r, y, 2);
         edit_plt->replot();
         step = 5;
         str.sprintf(tr("The MENISCUS is now defined. It was found to be at %5.3f cm.\n\n"
                        "Next, please select the data range to be included in the edited data.\n"
                        "Please define a limit to the right of the meniscus for included data."),
                     run_inf.meniscus[cell]);
         lbl_instructions2->setText(str);
         break;
      }
   case 3:   //pick meniscus-left
      {
         limit_left = current_R;
         step = 4;
         lbl_instructions2->setText(tr("Please click to the right of the MENISCUS now..."));
         break;
      }
   }
}

void EditAbsVeloc_Win::fit_ls(double value)
{
   if (value <= 0)
   {
      return;
   }
   order = (unsigned int) value;
   if (absorbance_integral != NULL)
   {
      delete [] absorbance_integral;
   }
   if (residuals != NULL)
   {
      delete [] residuals;
   }
   absorbance_integral = new double [run_inf.scans[cell][lambda]];
   residuals = new double [run_inf.scans[cell][lambda]];
   unsigned int scan, i;
   double *coeffs;
   double *fit;
   double *scan_time;

   US_lsfit *polyfit;
   coeffs = new double [order];
   scan_time = new double [run_inf.scans[cell][lambda]];
   fit = new double [run_inf.scans[cell][lambda]];

   //
   // calculate the integral of each scan which is needed for the least-squares polynomial fit to correct
   // for radially invariant baseline noise. We also keep track of the total integral at each point for
   //
   //
   for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
   {
      absorbance_integral[scan] = 0;
      for (i=0; i<points[scan] - 1; i++)
      {
         absorbance_integral[scan] += absorbance[scan][i] * (radius[scan][i+1] - radius[scan][i]);
         if (centerpiece.sector == 3) //if synthetic boundary centerpiece, we need to integrate data for vHW
         {
            absorbance[scan][i] = absorbance_integral[scan];
         }
      }
      i = points[scan] - 1;
      absorbance_integral[scan] += absorbance[scan][i] * (radius[scan][i] - radius[scan][i-1]);
   }
   //   plot against the time of the scan:
   for (i=0; i<run_inf.scans[cell][lambda]; i++)
   {
      scan_time[i] = (double) run_inf.time[cell][lambda][i];
   }
   residuals_defined = true;
   polyfit = new US_lsfit(coeffs, scan_time, absorbance_integral, order, run_inf.scans[cell][lambda], true);

   for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
   {
      fit[scan] = 0;
      for (i=0; i<order; i++)
      {
         fit[scan] +=  coeffs[i] * pow((double) run_inf.time[cell][lambda][scan], (double) i);
      }
      residuals[scan] = absorbance_integral[scan] - fit[scan];
   }

   uint scn[2];
   edit_plt->removeCurves();
   edit_plt->setAxisAutoScale(QwtPlot::xBottom);
   edit_plt->setAxisAutoScale(QwtPlot::yLeft);
   edit_plt->setAxisTitle(QwtPlot::xBottom, tr("Scan Time (seconds)"));
   scn[0] = edit_plt->insertCurve("Integrals");
   scn[1] = edit_plt->insertCurve("Polynomial Fit");
   edit_plt->setCurvePen(scn[0], QPen(yellow));
   edit_plt->setCurveRawData(scn[0], scan_time, absorbance_integral, run_inf.scans[cell][lambda]);
   edit_plt->setCurvePen(scn[1], QPen(magenta));
   edit_plt->setCurveRawData(scn[1], scan_time, fit, run_inf.scans[cell][lambda]);
   edit_plt->replot();
   pb_subtract->setEnabled(true);
   delete [] coeffs;
   delete [] fit;
   delete [] scan_time;
}

void EditAbsVeloc_Win::subtract_residuals()
{
   if (!residuals_defined)
   {
      QMessageBox::message(tr("Attention:"),    tr("Please define some residuals first by\n"
                                                   "calculating a non-zero polynomial fit\n"
                                                   "to the integrals of the scans!"));
      return;
   }
   unsigned int i, j;
   for (i=0; i<run_inf.scans[cell][lambda]; i++)
   {
      absorbance_integral[i] = 0;
      for (j=0; j<points[i]; j++)
      {
         absorbance[i][j] -= residuals[i];
         absorbance_integral[i] += absorbance[i][j] * run_inf.delta_r;
         if (centerpiece.sector == 3) //if synthetic boundary centerpiece, we need to integrate data for vHW
         {
            absorbance[i][j] = absorbance_integral[i];
         }
      }
   }
   residuals_defined = false;
}

void EditAbsVeloc_Win::next_step()
{
   ct_order->setEnabled(false);
   pb_continue->setEnabled(false);
   pb_subtract->setEnabled(false);
   edit_plt->setAxisTitle(QwtPlot::xBottom, tr("Radius (cm)"));
   edit_plt->clear();
   plot_dataset();
   step = 8;
   lbl_instructions2->setText(tr("Now you need to define the plateau of all scans. Click on a \n"
                                 "point that describes best a stable plateau for all data in this\n"
                                 "dataset. Please note that a stable plateau is not required for all\n"
                                 "scans. Certain analysis methods do not require a stable plateau."));
   return;
}

void EditAbsVeloc_Win::update_oldscan()
{
   unsigned int scan, i, count;
   // delete all excluded scans from oldscan, keep only those scans whose time matches:
   for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
   {
      while (run_inf.time[cell][lambda][scan] != oldscan[scan].time)
      {
         oldscan.erase(oldscan.begin() + scan);
      }
   }
   for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
   {
      count = 0;
      while (oldscan[scan].rad[count] < radius[scan][0])
      {
         count ++;
      }
      for (i=0; i<points[scan]; i++)
      {
         oldscan[scan].abs[count] = absorbance[scan][i];
         count ++;
      }
   }
}

