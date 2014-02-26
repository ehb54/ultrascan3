#include "../include/us_edeflo.h"

EditFloEquil_Win::EditFloEquil_Win(QWidget *p , const char *name) 
   : EditData_Win(p, name)
{
   edit_type = 2;
   extension1 = "FI";
   extension2 = "fi";
   edit_plt->setTitle(tr("Fluorescence Equilibrium Data"));
   edit_plt->setAxisTitle(QwtPlot::yLeft, "Absorbance");
   run_inf.exp_type.velocity = false;
   run_inf.exp_type.equilibrium = true;
   run_inf.exp_type.diffusion = false;
   run_inf.exp_type.simulation = false;
   run_inf.exp_type.interference = false;
   run_inf.exp_type.absorbance = false;
   run_inf.exp_type.fluorescence = true;
   run_inf.exp_type.intensity = false;
   run_inf.exp_type.wavelength = false;
   
   setup_GUI();
}

EditFloEquil_Win::~EditFloEquil_Win()
{
}

void EditFloEquil_Win::setup_GUI()
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

void EditFloEquil_Win::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/edit_equil_flo.html");
}

void EditFloEquil_Win::get_x(const QMouseEvent &e)
{
   unsigned int scan, count=0, line, temp_points = 0;
   int i, j, k;
   double r[2], y[2];
   double maximum;
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
         switch (step)
         {
         case 6:  // pick range-right
            {
               i = (unsigned int) (current_R * 1000);   //make sure we only have 3 significant digits
               limit_right = 1e-3 * (float) i;
               run_inf.range_right[cell][lambda][current_channel] = limit_right;
               for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
               {
                  i = find_indexf(limit_left, &radius[scan], points[scan]);
                  j = find_indexr(limit_right, &radius[scan], points[scan]);
                  if ((i < 0) || (j < 0))
                  {
                     str.sprintf   (tr("There appears to be a problem with scan %d of your\n"), scan+1);
                     str1.sprintf(tr("data that showed up in module edeabs.cpp:get_x::CASE %d.\n\n"), step);
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
                  if (j < i)
                  {
                     QMessageBox::message(tr("Attention:"), tr("First, you need to pick the left limit,\n"
                                                               "then the right. Please try again now..."));
                     step = 6;
                     return;
                  }
                  count=0;
                  for (k=i; k<j; k++)
                  {
                     radius[scan][count]        =       radius[scan][k];
                     absorbance[scan][count]    = absorbance[scan][k];
                     count++;
                  }
                  points[scan] = count;
                  run_inf.points[cell][lambda][current_channel] = count;
               }
               temp_points = 0;
               for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
               {
                  temp_points += points[scan];
               }
               run_inf.point_density[cell][lambda][current_channel] = run_inf.scans[cell][lambda] * (limit_right - limit_left) / temp_points;
               edit_plt->clear();
               for (unsigned int ik=0; ik<run_inf.scans[cell][lambda]; ik++)
               {
                  curve[ik] = edit_plt->insertCurve("Optical Density");
                  edit_plt->setCurvePen(curve[ik], yellow);
                  edit_plt->setCurveData(curve[ik], radius[ik], absorbance[ik], points[ik]);
               }
               edit_plt->setAxisAutoScale(QwtPlot::xBottom);
               edit_plt->setAxisAutoScale(QwtPlot::yLeft);
               plot_dataset();
               pb_accept->setText(tr("Save"));
               save_button = true;
               lbl_instructions2->setText(tr("Now exclude and/or edit any scans, if necessary. Then click on\n"
                                             "\"SAVE\" to save the subset of data."));
               pb_accept->setEnabled(true);
               step=3;
               break;
            }
         case 5:   // pick range-left
            {
               i = (int) (current_R * 1000);   //make sure we only have 3 significant digits
               limit_left = 1e-3 * (float) i;
               run_inf.range_left[cell][lambda][current_channel] = limit_left;
               step = 6;
               str.sprintf(tr("Please click near the bottom of channel %d to define the\n"
                              "right limit for the included data..."), current_channel + 1);
               lbl_instructions2->setText(str);
               break;
            }
         case 4:   //pick meniscus-right
            {
               limit_right = current_R;
               for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
               {
                  i = find_indexf(limit_left, &radius[scan], points[scan]);
                  j = find_indexf(limit_right, &radius[scan], points[scan]);
                  if ((i < 0) || (j < 0))
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
                     str.append    (tr("to re-edit your data and pick a point that is closer\n"));
                     str.append    (tr("to the meniscus."));
                     QMessageBox::message(tr("Attention:"), str);
                     return;
                  }
                  if (j < i)
                  {
                     QMessageBox::message(tr("Attention:"), tr("You need to pick first the left limit,\n"
                                                               "then the right. Please try again now..."));
                     step = 3;
                     return;
                  }
                  maximum = 0.0;
                  int maxk = 0;
                  for (k=i; k<j; k++)
                  {
                     if (maximum <= absorbance[scan][k])
                     {
                        maxk = k; // save index of the data point with the largest absorbance
                        maximum = absorbance[scan][k]; // reassign maximum to be the data point with the largest absorbance
                     }
                  }
                  run_inf.equil_meniscus.push_back(radius[scan][maxk]); // assign the radius of largest point to meniscus
                  // there is one meniscus entry for each scan in this channel
               }
               step = 5;
               str.sprintf(tr("The Menisci for each scan are now defined.\n\n"
                              "Next, please select the data range to be included in the edited data.\n"
                              "Please define a limit to the right of the meniscus of channel %d where\n"
                              "the included data starts."), current_channel + 1);
               lbl_instructions2->setText(str);
               edit_plt->clear();
               plot_dataset();
               break;
            }
         case 3:   //pick meniscus-left
            {
               limit_left = current_R;
               step = 4;
               lbl_instructions2->setText(tr("Please click to the right of the MENISCUS now..."));
               cmbb_centerpiece->setEnabled(false); //once we started editing, the settings for centerpieces shouldn't be changed
               cmbb_rotor->setEnabled(false); //once we started editing, the settings for centerpieces shouldn't be changed
               break;
            }
         }
   }
}




