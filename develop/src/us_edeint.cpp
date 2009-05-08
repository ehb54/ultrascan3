#include "../include/us_edeint.h"

US_EditInterferenceEquil::US_EditInterferenceEquil(QWidget *p , const char *name) 
   : EditData_Win(p, name)
{
   edit_type = 4;
   extension1 = "IP";
   extension2 = "ip";
   edit_plt->setTitle(tr("Interference Equilibrium Data"));
   edit_plt->setAxisTitle(QwtPlot::yLeft, "Interference");
   run_inf.exp_type.velocity = false;
   run_inf.exp_type.equilibrium = true;
   run_inf.exp_type.diffusion = false;
   run_inf.exp_type.simulation = false;
   run_inf.exp_type.interference = true;
   run_inf.exp_type.absorbance = false;
   run_inf.exp_type.fluorescence = false;
   run_inf.exp_type.intensity = false;
   run_inf.exp_type.wavelength = false;
   
   setup_GUI();
}

US_EditInterferenceEquil::~US_EditInterferenceEquil()
{
}

void US_EditInterferenceEquil::setup_GUI()
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
   subGrid1->addWidget(pb_sub_baseline,j,0);
   subGrid1->addWidget(lbl_current_cell1,j,1);
   subGrid1->addWidget(lbl_current_cell2,j,2);   
   QHBoxLayout *status_bar = new QHBoxLayout(2);
   status_bar->addWidget(lbl_status1);
   status_bar->addWidget(progress);
   subGrid1->addMultiCell(status_bar,j,j,3,4);
   j++;
   subGrid1->addWidget(pb_invert,j,0);
   subGrid1->addMultiCellWidget(lbl_instructions1,j,j+4,1,1);
   subGrid1->addMultiCellWidget(lbl_instructions2,j,j+4,2,4);
   j++;
   subGrid1->addWidget(pb_spikes,j,0);
   j++;
   subGrid1->addWidget(pb_help,j,0);
   j++;
   subGrid1->addWidget(pb_close,j,0);

   QHBoxLayout * subGrid2 = new QHBoxLayout(2);
   subGrid2->addWidget(lbl_centerpiece);
   subGrid2->addWidget(cmbb_centerpiece);
   subGrid2->addWidget(lbl_rotor);
   subGrid2->addWidget(cmbb_rotor);
   
   background->addWidget(edit_plt,0,0);
   background->addLayout(subGrid1,1,0);
   background->addLayout(subGrid2,2,0);
   
   qApp->processEvents();
   QRect r = background->cellGeometry(0, 0);

   global_Xpos += 30;
   global_Ypos += 30;
   
   this->setGeometry(global_Xpos, global_Ypos, r.width(), r.height()+250);
}


void US_EditInterferenceEquil::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/edit_equil_if.html");
}

void US_EditInterferenceEquil::get_x(const QMouseEvent &e)
{
   unsigned int scan, count=0, line;
   int k;
   double r[2], y[2];
   if (!save_button) //we don't want to use mouseclicks before the data were saved
   {
      current_R = (edit_plt->invTransform(QwtPlot::xBottom, e.x()));
      r[0] = current_R;
      r[1] = current_R;
      y[0] = ymin + ymax/20;
      y[1] = ymax - ymax/20;
      line = edit_plt->insertCurve("Line");
      edit_plt->setCurvePen(line, QPen(white, 2));
      edit_plt->setCurveData(line, r, y, 2);
      edit_plt->replot();
      if (!save_button) //we don't want to use
         //      cout << "Current Scan: " << current_scan << ", current_cell: " << cell << ", step: " << step << endl;
         switch (step)
         {
         case 5:   // pick meniscus
            {
               double *temp_r, *temp_y, min_y, max_y;
               unsigned int temp_points, temp_curve;
               run_inf.equil_meniscus.push_back(current_R);
               // since all interference scans have the same index at the same radius point,
               // we can take the initial index for the bottom of the channel for all of them:
               count = find_indexf(run_inf.range_left[cell][lambda][current_channel],
                                   &radius[current_scan], points[current_scan]);
            
               str.sprintf(tr("The meniscus for scan %d has been found at %1.3f cm.\n\n"
                              "Next, please select the meniscus for scan %d.\n"),
                           current_scan + 1, run_inf.equil_meniscus[current_scan], current_scan + 2);
               lbl_instructions2->setText(str);
               step = 5;
               current_scan ++;
               temp_points = index2 - index1 + 1;
               temp_r = new double [temp_points];
               temp_y = new double [temp_points];
               count = 0;
               min_y = 10000;
               max_y = -10000;
               //      cout << "Current Scan: " << current_scan << ", current_cell: " << cell << ", step: " << step << endl;
               for (k=index1; k<=index2; k++)
               {
                  temp_r[count] = radius[current_scan][k];
                  temp_y[count] = absorbance[current_scan][k];
                  min_y = min(min_y, temp_y[count]);
                  max_y = max(max_y, temp_y[count]);
                  //      cout << "temp_r: " << temp_r[count] << ", temp_y[count]: " << temp_y[count] << endl;
                  count ++;
               } 
               edit_plt->clear();
               temp_curve = edit_plt->insertCurve("Fringes");
               edit_plt->setCurvePen(temp_curve, yellow);
               edit_plt->setCurveData(temp_curve, temp_r, temp_y, temp_points);
               edit_plt->setAxisScale(QwtPlot::xBottom, temp_r[0], temp_r[temp_points - 1], 0);
               edit_plt->setAxisScale(QwtPlot::yLeft, min_y, max_y, 0);
               edit_plt->replot();
               delete [] temp_r;
               delete [] temp_y;
               max_y = -1000;
               if (current_scan == run_inf.scans[cell][lambda] - 1)
               {
                  for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
                  {// find the meniscus farthest to the right:
                     max_y = max(max_y, (double)run_inf.equil_meniscus[scan]);
                  }// add ~40 points to shift it to the right (20 pts ~=0.025 cm), and assign it to range_left
                  run_inf.range_left[cell][lambda][current_channel] = max_y + 0.025;
                  // find the index of range_left
                  index1 = find_indexf(run_inf.range_left[cell][lambda][current_channel],
                                       &radius[current_scan], points[current_scan]);
                  // and use both indices to calculate the number of points in this channel (same for all scans):
                  run_inf.points[cell][lambda][current_channel] = index2 - index1 + 1;
                  run_inf.point_density[cell][lambda][current_channel]
                     = (run_inf.range_right[cell][lambda][current_channel]
                        - run_inf.range_left[cell][lambda][current_channel])/(index2 - index1 + 1);
                  edit_plt->setAxisAutoScale(QwtPlot::xBottom);
                  edit_plt->setAxisAutoScale(QwtPlot::yLeft);
                  plot_dataset();
                  save_button = true;
                  lbl_instructions2->setText(tr("Now exclude and/or edit any scans, if necessary. Then click on\n"
                                                "\"SAVE\" to save the subset of data."));
                  // now that we have a single range_left and a single range_right for this group of
                  // scans from this channel, we can calculate the number of points for these scans:
                  //               load_dataset();
                  step = 3;
                  load_dataset();
               }
               break;
            }
         case 4:   //pick right bracket of channel
            {
               cnt_exsingle->setEnabled(false);
               cnt_exrange->setEnabled(false);
               pb_exsingle->setEnabled(false);
               pb_exrange->setEnabled(false);
               run_inf.range_right[cell][lambda][current_channel] = current_R;
               double *temp_r, *temp_y, min_y, max_y;
               unsigned int temp_points, temp_curve;
               index1 = find_indexf(left_bracket, &radius[current_scan], points[current_scan]);
               index2 = find_indexf(current_R, &radius[current_scan], points[current_scan]);
               //index2 is the same for all scans since interference scans have identical delta_r.
               if ((index1 < 0) || (index2 < 0))
               {
                  str.sprintf   (tr("There appears to be a problem with scan %d of your\n"), current_scan+1);
                  str1.sprintf(tr("data that showed up in module edvabs.cpp:get_x::CASE %d.\n\n"), step);
                  str.append(str1);
                  str.append    (tr("Please manually inspect the file for:\n\n"));
                  str1.sprintf(tr("   Scan:         %d\n"), current_scan+1);
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
               if (index2 < index1)
               {
                  QMessageBox::message(tr("Attention:"), tr("You need to pick first the left limit,\n"
                                                            "then the right. Please try again now..."));
                  step = 3;
                  return;
               }
               temp_points = index2 - index1 + 1;
               temp_r = new double [temp_points];
               temp_y = new double [temp_points];
               count = 0;
               min_y = 10000;
               max_y = -10000;
               for (k=index1; k<=index2; k++)
               {
                  temp_r[count] = radius[current_scan][k];
                  temp_y[count] = absorbance[current_scan][k];
                  min_y = min(min_y, temp_y[count]);
                  max_y = max(max_y, temp_y[count]);
                  count ++;
               } 
               edit_plt->clear();
               temp_curve = edit_plt->insertCurve("Fringes");
               edit_plt->setCurvePen(temp_curve, yellow);
               edit_plt->setCurveData(temp_curve, temp_r, temp_y, temp_points);
               edit_plt->setAxisScale(QwtPlot::xBottom, temp_r[0], temp_r[temp_points - 1], 0);
               edit_plt->setAxisScale(QwtPlot::yLeft, min_y, max_y, 0);
               edit_plt->replot();
               delete [] temp_r;
               delete [] temp_y;
            
               str.sprintf(tr("Now click on the point that best describes the meniscus of scan %d.\n"
                              "This is generally just to the left of the smooth data.\n"), 
                           current_scan + 1);
               lbl_instructions2->setText(str);
               step = 5;
               break;
            }
         case 3:   //pick left bracket of channel
            {
               left_bracket = current_R;
               current_scan = 0;
               step = 4;
               str.sprintf(tr("Optional: If desired, you can now delete unwanted scans from\nthis channel.\n"
                              "Then click to the right of the bottom of channel %d to\nbracket the data range."), current_channel + 1);
               lbl_instructions2->setText(str);
               cmbb_centerpiece->setEnabled(false); //once we started editing, the settings for centerpieces shouldn't be changed
               cmbb_rotor->setEnabled(false); //once we started editing, the settings for centerpieces shouldn't be changed
               break;
            }
         }
   }
}




