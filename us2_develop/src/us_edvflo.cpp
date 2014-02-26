#include "../include/us_edvflo.h"

EditFloVeloc_Win::EditFloVeloc_Win(QWidget *p , const char *name) 
   : EditData_Win(p, name)
{
   edit_type = 5;
   extension1 = "FI";
   extension2 = "fi";
   edit_plt->setTitle(tr("Fluorescence Velocity Data"));
   edit_plt->setAxisTitle(QwtPlot::yLeft, tr("Absorbance"));
   run_inf.exp_type.velocity = true;
   run_inf.exp_type.equilibrium = false;
   run_inf.exp_type.diffusion = false;
   run_inf.exp_type.simulation = false;
   run_inf.exp_type.interference = false;
   run_inf.exp_type.absorbance = false;
   run_inf.exp_type.fluorescence = true;
   run_inf.exp_type.intensity = false;
   run_inf.exp_type.wavelength = false;
   pb_invert->hide();
   setup_GUI();
}

EditFloVeloc_Win::~EditFloVeloc_Win()
{
}

void EditFloVeloc_Win::setup_GUI()
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
   subGrid1->addWidget(pb_exclude_profile,j,0);
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

void EditFloVeloc_Win::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/edit_veloc_flo.html");
}

void EditFloVeloc_Win::get_x(const QMouseEvent &e)
{
   uint line;
   double absorbance_integral=0;
   unsigned int i, j, scan=0, count=0;
   int ik, jk, kk;
   double r[2], y[2], maximum=-100, sum;
   double m, b, newpoint=0;
   float rad;
   unsigned int temp_points = 0;
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
   case 8:   // pick baseline
      {
         scan = run_inf.scans[cell][lambda]-1;
         j=find_indexf(current_R, &radius[scan], points[scan]);
         sum = 0;
         if (j > points[scan]+4)
         {
            str.sprintf   (tr("There appears to be a problem with scan %d of your\n"), scan+1);
            str1.sprintf(tr("data that showed up in module edvflo.cpp:get_x::CASE %d.\n\n"), step);
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
            str.sprintf   (tr("There appears to be a problem with scan %d of your\n"), scan+1);
            str1.sprintf(tr("data that showed up in module edvflo.cpp:get_x::CASE %d.\n\n"), step);
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
         unsigned int *count;    // write all scans from this dataset to file
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
         count = new unsigned int [run_inf.scans[cell][lambda]];
         for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
         {
            count[scan]=0;
         }
         run_inf.points[cell][lambda][current_channel] = 1 + (unsigned int) ((run_inf.range_right[cell][lambda][current_channel]
                                                                              - run_inf.range_left[cell][lambda][current_channel])/run_inf.delta_r);
         for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
         {
            for (i=0; i<run_inf.points[cell][lambda][current_channel]; i++)
            {
               rad = run_inf.range_left[cell][lambda][current_channel] + i * run_inf.delta_r;
               // only compare the first 3 significant digits
               // Sometimes there are increments smaller than 0.001, so we need to skip those 
               //               while ((unsigned int) (radius[scan][count[scan]] * 1000) < (unsigned int) (rad * 1000))
               while (radius[scan][count[scan]] < rad)
               {
                  count[scan] ++;
               }
               // if the first point was smaller, but the second point was larger, we need to go back one. If the
               // first point was the first point in the scan and it is larger than required we cannot go back and we got to keep the value
               //               if ((unsigned int) (radius[scan][count[scan]] * 1000) > (unsigned int) (rad * 1000) && count[scan] !=0)
               if (radius[scan][count[scan]] > rad && count[scan] != 0)
               {
                  count[scan] --;
               }
               //               if ((unsigned int) (radius[scan][count[scan]] * 1000) == (unsigned int) (rad * 1000))
               if (radius[scan][count[scan]] == rad)
               {
                  s << (float) (absorbance[scan][count[scan]]);
                  count[scan] ++;
               }
               else
               {
                  // if we are at the beginning of the scan, and the first datapoints is past "rad", fill in with 
                  // the first data point's value:
                  if (count[scan] == 0)
                  {
                     newpoint = absorbance[scan][0];
                  }
                  // if we are in the middle of the scan, we linearly interpolate missing datapoints (newpoint = m * rad + b)
                  // make sure that we are not outside bounds
                  if ((count[scan] < (unsigned int) points[scan] - 2))
                  {
                     m = (absorbance[scan][count[scan]+1] - absorbance[scan][count[scan]]) /
                        (radius[scan][count[scan]+1] - radius[scan][count[scan]]);
                     b = absorbance[scan][count[scan]] - m * radius[scan][count[scan]];
                     newpoint = m * rad + b;
                  }
                  if (count[scan] == (unsigned int) points[scan] - 1)
                  {
                     newpoint = absorbance[scan][points[scan]-1]; // we are at the end of the scan
                  }
                  s << (float) newpoint;
               }
            }
         }
         f.flush();
         f.close();
         cleanup_dataset();
         load_dataset();
         return;
      }
   case 7:   // pick plateau x
      {
         for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
         {
            j=find_indexr(current_R, &radius[scan], points[scan]);
            sum = 0;
            if (j > points[scan]+4)
            {
               str.sprintf   (tr("There appears to be a problem with scan %d of your\n"), scan+1);
               str1.sprintf(tr("data that showed up in module edvflo.cpp:get_x::CASE %d.\n\n"), step);
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
               str.sprintf   (tr("There appears to be a problem with scan %d of your\n"), scan+1);
               str1.sprintf(tr("data that showed up in module edvflo.cpp:get_x::CASE %d.\n\n"), step);
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
         step=8;
         for (scan = 0; scan < run_inf.scans[cell][lambda]-1; scan++)
         {
            edit_plt->removeCurve(curve[scan]);
         }
         edit_plt->replot();
         lbl_instructions2->setText(tr("Please click on a point that describes best the baseline absorbance\n"));
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
         for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
         {
            ik = find_indexf(limit_left, &radius[scan], points[scan]);
            jk = find_indexr(limit_right, &radius[scan], points[scan]);
            if ((ik < 0) || (jk < 0) || (current_R > radius[scan][points[scan]-1]))
            {
               str.sprintf   (tr("There appears to be a problem with scan %d of your\n"), scan+1);
               str1.sprintf(tr("data that showed up in module edvflo.cpp:get_x::CASE %d.\n\n"), step);
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
            if (jk < ik)
            {
               QMessageBox::message(tr("Attention:"), tr("You need to pick first the left limit,\n"
                                                         "then the right. Please try again now..."));
               step = 5;
               return;
            }
            count=0;
            absorbance_integral = 0;
            for (kk=ik; kk<jk; kk++)
            {
               radius[scan][count] = radius[scan][kk]; // throw away points outside the limits
               if (centerpiece.sector == 3)
               {
                  absorbance_integral      += absorbance[scan][kk] 
                     * ((radius[scan][kk+1] - radius[scan][kk]) + (radius[scan][kk] - radius[scan][kk-1]))/2;
                  absorbance[scan][count] = absorbance_integral;
               }
               else
               {
                  absorbance[scan][count] = absorbance[scan][kk];
               }
               count++;
            }
            points[scan]=count-1;
         }
         edit_plt->clear();
         edit_plt->setAxisAutoScale(QwtPlot::xBottom);
         edit_plt->setAxisAutoScale(QwtPlot::yLeft);
         plot_dataset();
         lbl_instructions2->setText(tr("Now exclude and/or edit any scans, if necessary. Then click on a\n"
                                       " point that describes best a stable plateau for all data in this\n"
                                       " dataset. Please note that a stable plateau is not required for all\n"
                                       " scans. Certain analysis methods do not require a stable plateau."));
         step=7;
         break;
      }
   case 5:   // pick range-left
      {
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
         limit_right = current_R;
         for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
         {
            ik = find_indexf(limit_left, &radius[scan], points[scan]);
            jk = find_indexf(limit_right, &radius[scan], points[scan]);
            if ((ik < 0) || (jk < 0))
            {
               str.sprintf   (tr("There appears to be a problem with scan %d of your\n"), scan+1);
               str1.sprintf(tr("data that showed up in module edvflo.cpp:get_x::CASE %d.\n\n"), step);
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

