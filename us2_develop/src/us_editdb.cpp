#include "../include/us_editdb.h"

US_Edit_DB::US_Edit_DB(QWidget *p , const char *name) 
   : EditData_Win(p, name)
{
   delete pb_spikes;
   delete pb_editscan;
   delete pb_exsingle;
   delete pb_exrange;
   delete pb_sub_baseline;
   delete pb_invert;
   delete cnt_exsingle;
   delete cnt_exrange;
   delete cnt_edit;
   delete pb_exclude_profile;
   database = true;
   
   exp_type.velocity = true;
   exp_type.equilibrium = false;
   exp_type.diffusion = false;
   exp_type.interference = false;
   exp_type.absorbance = true;
   exp_type.fluorescence = false;
   exp_type.intensity = false;
   exp_type.wavelength = false;
   
   pb_accept->setText(tr("Review Dataset"));

   cb_data_uv = new QCheckBox(tr(" Absorbance"), this);
   cb_data_uv->setChecked(exp_type.absorbance);
   cb_data_uv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cb_data_uv->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_data_uv, SIGNAL(clicked()), SLOT(update_data_uv()));

   cb_data_if = new QCheckBox(tr(" Interference"), this);
   cb_data_if->setChecked(exp_type.interference);
   cb_data_if->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cb_data_if->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_data_if, SIGNAL(clicked()), SLOT(update_data_if()));

   cb_data_fl = new QCheckBox(tr(" Fluorescence"), this);
   cb_data_fl->setChecked(exp_type.fluorescence);
   cb_data_fl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cb_data_fl->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_data_fl, SIGNAL(clicked()), SLOT(update_data_fl()));

   cb_data_veloc = new QCheckBox(tr(" Velocity"), this);
   cb_data_veloc->setChecked(exp_type.velocity);
   cb_data_veloc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cb_data_veloc->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_data_veloc, SIGNAL(clicked()), SLOT(update_data_veloc()));

   cb_data_equil = new QCheckBox(tr(" Equilibrium"), this);
   cb_data_equil->setChecked(exp_type.equilibrium);
   cb_data_equil->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cb_data_equil->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_data_equil, SIGNAL(clicked()), SLOT(update_data_equil()));

   cb_data_diff = new QCheckBox(tr(" Diffusion"),this);
   cb_data_diff->setChecked(exp_type.diffusion);
   cb_data_diff->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cb_data_diff->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_data_diff, SIGNAL(clicked()), SLOT(update_data_diff()));
   disconnect(pb_select_dir, SIGNAL(clicked()), this, SLOT(directory()));
   connect(pb_select_dir, SIGNAL(clicked()), this, SLOT(review()));

   cb_data_wv = new QCheckBox(tr(" Wavelength"),this);
   cb_data_wv->setChecked(exp_type.wavelength);
   cb_data_wv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cb_data_wv->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_data_wv, SIGNAL(clicked()), SLOT(update_data_wv()));

   cb_data_intensity = new QCheckBox(tr(" Intensity"),this);
   cb_data_intensity->setChecked(exp_type.intensity);
   cb_data_intensity->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cb_data_intensity->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_data_intensity, SIGNAL(clicked()), SLOT(update_data_intensity()));

   lbl_instructions1->setAlignment(AlignCenter|AlignVCenter);
   
   edit_type = 1;
   extension1 = "RA";
   extension2 = "ra";
   edit_plt->setTitle(tr("Absorbance Velocity Data"));
   edit_plt->setAxisTitle(QwtPlot::yLeft, tr("Absorbance"));

   setup_GUI();
}

US_Edit_DB::~US_Edit_DB()
{
   
}
void US_Edit_DB::setup_GUI()
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
      
   subGrid1->addWidget(pb_select_dir,j,0);
   subGrid1->addMultiCellWidget(lbl_directory,j,j,1,2);   
   subGrid1->addWidget(cb_data_uv,j,3);
   subGrid1->addWidget(cb_data_veloc,j,4);
   j++;
   subGrid1->addWidget(pb_details,j,0);
   subGrid1->addMultiCellWidget(lbl_details,j,j,1,2);   
   subGrid1->addWidget(cb_data_if,j,3);
   subGrid1->addWidget(cb_data_equil,j,4);
   j++;
   subGrid1->addWidget(pb_accept,j,0);
   subGrid1->addWidget(lbl_run_id1,j,1);
   subGrid1->addWidget(lbl_run_id2,j,2);   
   subGrid1->addWidget(cb_data_fl,j,3);
   subGrid1->addWidget(cb_data_diff,j,4);
   j++;
   subGrid1->addWidget(pb_help,j,0);
   subGrid1->addWidget(lbl_current_cell1,j,1);
   subGrid1->addWidget(lbl_current_cell2,j,2);   
   subGrid1->addWidget(cb_data_intensity,j,3);
   subGrid1->addWidget(cb_data_wv,j,4);
   j++;
   subGrid1->addWidget(pb_close,j,0);
   subGrid1->addWidget(lbl_status1,j,1);
   subGrid1->addMultiCellWidget(progress,j,j,2,4);
   j++;
   subGrid1->addMultiCellWidget(lbl_instructions1,j,j+4,0,0);
   subGrid1->addMultiCellWidget(lbl_instructions2,j,j+4,1,4);
   j=j+5;
   subGrid1->addWidget(lbl_centerpiece,j,0);
   subGrid1->addMultiCellWidget(cmbb_centerpiece,j,j,1,2);
   subGrid1->addWidget(lbl_rotor,j,3);
   subGrid1->addWidget(cmbb_rotor,j,4);
   
   background->addWidget(edit_plt,0,0);
   background->addLayout(subGrid1,1,0);
   
   qApp->processEvents();
   QRect r = background->cellGeometry(0, 0);

   global_Xpos += 30;
   global_Ypos += 30;
   
   this->setGeometry(global_Xpos, global_Ypos, r.width(), r.height()+250);
}
void US_Edit_DB::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/editdb.html");
}

void US_Edit_DB::update_data_uv()
{
   extension1 = "RA";
   extension2 = "ra";
   exp_type.absorbance = true;
   exp_type.interference = false;
   exp_type.fluorescence = false;
   exp_type.intensity = false;
   cb_data_uv->setChecked(exp_type.absorbance);
   cb_data_if->setChecked(exp_type.interference);
   cb_data_fl->setChecked(exp_type.fluorescence);
   cb_data_intensity->setChecked(exp_type.intensity);
   set_plot_axes();
}

void US_Edit_DB::update_data_if()
{
   extension1 = "IP";
   extension2 = "ip";
   exp_type.absorbance = false;
   exp_type.interference = true;
   exp_type.fluorescence = false;
   exp_type.intensity = false;
   exp_type.wavelength = false;
   cb_data_uv->setChecked(exp_type.absorbance);
   cb_data_if->setChecked(exp_type.interference);
   cb_data_fl->setChecked(exp_type.fluorescence);
   cb_data_intensity->setChecked(exp_type.intensity);
   cb_data_wv->setChecked(exp_type.wavelength);
   set_plot_axes();
}

void US_Edit_DB::update_data_fl()
{
   extension1 = "FI";
   extension2 = "fi";
   exp_type.absorbance = false;
   exp_type.interference = false;
   exp_type.fluorescence = true;
   exp_type.intensity = false;
   exp_type.wavelength = false;
   cb_data_uv->setChecked(exp_type.absorbance);
   cb_data_if->setChecked(exp_type.interference);
   cb_data_fl->setChecked(exp_type.fluorescence);
   cb_data_intensity->setChecked(exp_type.intensity);
   cb_data_wv->setChecked(exp_type.wavelength);
   set_plot_axes();
}
void US_Edit_DB::update_data_intensity()
{

   extension1 = "WI";
   extension2 = "wi";
   exp_type.absorbance = false;
   exp_type.interference = false;
   exp_type.fluorescence = false;
   exp_type.intensity = true;
   cb_data_uv->setChecked(exp_type.absorbance);
   cb_data_if->setChecked(exp_type.interference);
   cb_data_fl->setChecked(exp_type.fluorescence);
   cb_data_intensity->setChecked(exp_type.intensity);
   set_plot_axes();
}

void US_Edit_DB::update_data_veloc()
{
   exp_type.velocity = true;
   exp_type.equilibrium = false;
   exp_type.diffusion = false;
   exp_type.wavelength = false;
   cb_data_veloc->setChecked(exp_type.velocity);
   cb_data_equil->setChecked(exp_type.equilibrium);
   cb_data_diff->setChecked(exp_type.diffusion);
   cb_data_wv->setChecked(exp_type.wavelength);
   set_plot_axes();
}

void US_Edit_DB::update_data_equil()
{
   exp_type.velocity = false;
   exp_type.equilibrium = true;
   exp_type.diffusion = false;
   exp_type.wavelength = false;
   cb_data_veloc->setChecked(exp_type.velocity);
   cb_data_equil->setChecked(exp_type.equilibrium);
   cb_data_diff->setChecked(exp_type.diffusion);
   cb_data_wv->setChecked(exp_type.wavelength);
   set_plot_axes();
}

void US_Edit_DB::update_data_diff()
{
   exp_type.velocity = false;
   exp_type.equilibrium = false;
   exp_type.diffusion = true;
   exp_type.wavelength = false;
   cb_data_veloc->setChecked(exp_type.velocity);
   cb_data_equil->setChecked(exp_type.equilibrium);
   cb_data_diff->setChecked(exp_type.diffusion);
   cb_data_wv->setChecked(exp_type.wavelength);
   set_plot_axes();
}

void US_Edit_DB::update_data_wv()
{
   extension1 = "WA";
   extension2 = "wa";
   exp_type.absorbance = true;
   exp_type.velocity = false;
   exp_type.equilibrium = false;
   exp_type.diffusion = false;
   exp_type.wavelength = true;
   exp_type.interference = false;
   exp_type.fluorescence = false;
   cb_data_veloc->setChecked(exp_type.velocity);
   cb_data_equil->setChecked(exp_type.equilibrium);
   cb_data_diff->setChecked(exp_type.diffusion);
   cb_data_wv->setChecked(exp_type.wavelength);
   cb_data_if->setChecked(exp_type.interference);
   cb_data_fl->setChecked(exp_type.fluorescence);
   set_plot_axes();
}

void US_Edit_DB::review()
{
   pb_accept->setText(tr("Review Dataset"));
   directory();
}

void US_Edit_DB::set_plot_axes()
{
   if(exp_type.absorbance && exp_type.velocity)
   {
      edit_type = 1;
      edit_plt->setTitle(tr("Absorbance Velocity Data"));
      edit_plt->setAxisTitle(QwtPlot::yLeft, tr("Absorbance"));
   }
   else if (exp_type.interference && exp_type.velocity)
   {
      edit_type = 3;
      edit_plt->setTitle(tr("Interference Velocity Data"));
      edit_plt->setAxisTitle(QwtPlot::yLeft, tr("Fringes"));
   }
   else if (exp_type.fluorescence && exp_type.velocity)
   {
      edit_type = 5;
      edit_plt->setTitle(tr("Fluorescence Velocity Data"));
      edit_plt->setAxisTitle(QwtPlot::yLeft, tr("Fluorescence Intensity"));
   }
   if(exp_type.absorbance && exp_type.equilibrium)
   {
      edit_type = 2;
      edit_plt->setTitle(tr("Absorbance Equilibrium Data"));
      edit_plt->setAxisTitle(QwtPlot::yLeft, tr("Absorbance"));
   }
   else if (exp_type.interference && exp_type.equilibrium)
   {
      edit_type = 4;
      edit_plt->setTitle(tr("Interference Equilibrium Data"));
      edit_plt->setAxisTitle(QwtPlot::yLeft, tr("Fringes"));
   }
   else if (exp_type.fluorescence && exp_type.equilibrium)
   {
      edit_type = 6;
      edit_plt->setTitle(tr("Fluorescence Equilibrium Data"));
      edit_plt->setAxisTitle(QwtPlot::yLeft, tr("Fluorescence"));
   }
   if(exp_type.absorbance && exp_type.diffusion)
   {
      edit_type = 7;
      edit_plt->setTitle(tr("Absorbance Diffusion Data"));
      edit_plt->setAxisTitle(QwtPlot::yLeft, tr("Absorbance"));
   }
   else if (exp_type.interference && exp_type.diffusion)
   {
      edit_type = 8;
      edit_plt->setTitle(tr("Interference Diffusion Data"));
      edit_plt->setAxisTitle(QwtPlot::yLeft, tr("Fringes"));
   }
   else if (exp_type.fluorescence && exp_type.diffusion)
   {
      edit_type = 9;
      edit_plt->setTitle(tr("Fluorescence Diffusion Data"));
      edit_plt->setAxisTitle(QwtPlot::yLeft, tr("Fluorescence"));
   }
   if(exp_type.absorbance && exp_type.wavelength)
   {
      edit_type = 10;
      edit_plt->setTitle(tr("Absorbance Wavelength Data"));
      edit_plt->setAxisTitle(QwtPlot::yLeft, tr("Absorbance"));
   }
   else if(exp_type.intensity && exp_type.wavelength)
   {
      edit_type = 11;
      edit_plt->setTitle(tr("Intensity Wavelength Data"));
      edit_plt->setAxisTitle(QwtPlot::yLeft, tr("Intensity"));
   }
   else if(exp_type.intensity && exp_type.velocity)
   {
      edit_type = 12;
      edit_plt->setTitle(tr("Intensity Velocity Data"));
      edit_plt->setAxisTitle(QwtPlot::yLeft, tr("Intensity"));
   }
   else if(exp_type.intensity && exp_type.equilibrium)
   {
      edit_type = 13;
      edit_plt->setTitle(tr("Intensity Equilibrium Data"));
      edit_plt->setAxisTitle(QwtPlot::yLeft, tr("Intensity"));
   }
   else if(exp_type.intensity && exp_type.diffusion)
   {
      edit_type = 14;
      edit_plt->setTitle(tr("Intensity Diffusion Data"));
      edit_plt->setAxisTitle(QwtPlot::yLeft, tr("Intensity"));
   }
}

void US_Edit_DB::plot_dataset()
{
   QString header;
   unsigned int i;
   double *temp_max, *temp_min, dummy;
   header = tr("Sedimentation Data for ");
   header.append(run_inf.cell_id[cell]);
   edit_plt->setTitle(header);
   if (edit_type == 1 || edit_type == 2 || edit_type == 7 || edit_type == 10)
   {
      str.sprintf(tr("Absorbance at %d nm"), run_inf.wavelength[cell][lambda]);
   }
   if (edit_type == 3 || edit_type == 4 || edit_type == 8)
   {
      str.sprintf(tr("Fringes"));
   }
   if (edit_type == 5 || edit_type == 6 || edit_type == 9)
   {
      str.sprintf(tr("Fluorescence"));
   }
   if (edit_type == 11 || edit_type == 12 || edit_type == 13 || edit_type == 14)
   {
      str.sprintf(tr("Intensity"));
   }
   edit_plt->setAxisTitle(QwtPlot::yLeft, str);
   if (edit_type == 10) //Wavelength data
   {
      edit_plt->setAxisTitle(QwtPlot::xBottom, "Wavelength (nm)");
      edit_plt->setAxisTitle(QwtPlot::yLeft, "Absorbance");
   }

   temp_max = new double [run_inf.scans[cell][lambda]];
   temp_min = new double [run_inf.scans[cell][lambda]];

   //
   // find the absorbance maximum and minimum in each scan and collect them in 
   // temp_max and temp_min:   
   //

   for (i=0; i<run_inf.scans[cell][lambda]; i++)
   {
      get_1d_limits(&absorbance[i], &temp_min[i], &temp_max[i], points[i]);
   }
   get_1d_limits(&temp_max, &dummy, &ymax, run_inf.scans[cell][lambda]);
   get_1d_limits(&temp_min, &ymin, &dummy, run_inf.scans[cell][lambda]);
   ymax = ymax + (ymax - ymin)/5;
   for (i=0; i<run_inf.scans[cell][lambda]; i++)
   {
      temp_max[i] = radius[i][points[i]-1];
   }
   get_1d_limits(&temp_max, &dummy, &xmax, run_inf.scans[cell][lambda]);
   xmin = radius[0][0];
   curve = new unsigned int [run_inf.scans[cell][lambda]];
   edit_plt->clear();
   for (i=0; i<run_inf.scans[cell][lambda]; i++)
   {
      curve[i] = edit_plt->insertCurve("Optical Density");
      edit_plt->setCurvePen(curve[i], yellow);
      edit_plt->setCurveData(curve[i], radius[i], absorbance[i], points[i]);
   }
   if ((ymax - ymin) > 0.2 && (ymax - ymin) < 1.2)
   {
      edit_plt->setAxisScale(QwtPlot::yLeft, ymin, ymax, 0.2);
   }
   else
   {
      edit_plt->setAxisScale(QwtPlot::yLeft, ymin, ymax, 0);
   }
   if ((xmax - xmin) > 0.1)
   {
      edit_plt->setAxisScale(QwtPlot::xBottom, xmin, xmax, 0.1);
   }
   else
   {
      edit_plt->setAxisScale(QwtPlot::xBottom, xmin, xmax, 0);
   }
   edit_plt->replot();
   
   delete [] temp_max;
   delete [] temp_min;
   if (dataset_counter != count2)   // if dataset_counter == count2, we're all done editing!
   {
      pb_accept->setEnabled(true);
      pb_accept->setText(tr("Next Cell"));
   }
}

void US_Edit_DB::load_dataset()
{
   unsigned int i, scan, scan_counter=0;
   pb_accept->setEnabled(false); // disable during loading

   cmbb_centerpiece->setEnabled(true);
   cmbb_rotor->setEnabled(true);

   //
   // Now we need to check if we have defined the last dataset. that condition is
   // true if we have dataset_counter == count2, except for the case of multiple channels,
   // where we also have to wait for the last channel:
   //
   if (dataset_counter == count2)   // we're all done editing!
   {
      emit variablesUpdated();
      str = tr("Information for Run ");
      str.append(run_inf.run_id);
      str.append(":\n\n");
      str.append(tr("Data Review complete\n"));
      str.append(tr("--------------------------------"));
      str1 = tr("Information for ");
      str1.append(run_inf.run_id);
      QMessageBox::message(str1, str);
      close();
      return;
   }

   QFile f;
   //
   // what's our current cell? 
   // the extension of the filename has this in the second digit (of 2)
   //
   cell = active_set[dataset_counter]/10;
   //
   // what's our current wavelength? 
   // the extension of the filename has this in the second digit (of 2)
   //
   lambda = active_set[dataset_counter] - (cell * 10);
   dataset_counter++;   //get ready for the next data set (we need that counter)
   if (dataset_counter == 1)
   {
      radius     = new double* [run_inf.scans[cell][lambda]];
      absorbance = new double* [run_inf.scans[cell][lambda]];
      points     = new unsigned int [run_inf.scans[cell][lambda]];
      temp_radius     = new double* [run_inf.scans[cell][lambda]];
      temp_absorbance = new double* [run_inf.scans[cell][lambda]];
      temp_points     = new unsigned int [run_inf.scans[cell][lambda]];
   }
   progress->setTotalSteps(run_inf.scans[cell][lambda]);
   progress->reset();
   str.sprintf(tr("Please wait...\n\nLoading Data for Dataset %d"), dataset_counter);
   lbl_instructions2->setText(str);
   str.sprintf(tr("Dataset %d"), dataset_counter);
   lbl_current_cell2->setText(str);

   // start with the correct wavelength scan index 
   // (need to use +1 because this is our index for the file names.
   scan = lambda+1;
   //
   // Note: "scan" is a variable that serves as the scan number index for the files
   // scan_counter counts the actual files. Those numbers are different by one for cases
   // where there is a single wavlength, otherwise the difference is larger
   //
   while (scan_counter < run_inf.scans[cell][lambda])
   {
      radius[scan_counter]= new double [2048];
      absorbance[scan_counter]= new double [2048];
      temp_radius[scan_counter]= new double [2048];
      temp_absorbance[scan_counter]= new double [2048];
      if (extension == "FI" || extension == "fi")
      {
         if (scan<10)
         {
            str.sprintf(edited_channel + "0000%d." + extension + "%d", scan, cell+1);
         }
         else if (scan>9 && scan<100)
         {
            str.sprintf(edited_channel + "000%d." + extension + "%d", scan, cell+1);
         }
         else if (scan>99 && scan<1000)
         {
            str.sprintf(edited_channel + "00%d." + extension + "%d", scan, cell+1);
         }
         else if (scan>999 && scan<10000)
         {
            str.sprintf(edited_channel + "0%d." + extension + "%d", scan, cell+1);
         }
         else if (scan>9999 && scan<100000)
         {
            str.sprintf(edited_channel + "%d." + extension + "%d", scan, cell+1);
         }
      }
      else
      {
         if (scan<10)
         {
            str.sprintf("0000%d." + extension + "%d", scan, cell+1);
         }
         else if (scan>9 && scan<100)
         {
            str.sprintf("000%d." + extension + "%d", scan, cell+1);
         }
         else if (scan>99 && scan<1000)
         {
            str.sprintf("00%d." + extension + "%d", scan, cell+1);
         }
         else if (scan>999 && scan<10000)
         {
            str.sprintf("0%d." + extension + "%d", scan, cell+1);
         }
         else if (scan>9999 && scan<100000)
         {
            str.sprintf("%d." + extension + "%d", scan, cell+1);
         }
      }
      str.prepend(run_inf.data_dir);
      cout << str << endl;
      f.setName(str);
      if (f.open(IO_ReadOnly))
      {
         QTextStream ts(&f);
         if (!ts.eof())
         {
            for (i=0; i<2; i++)   // i is a row counter
            {
               ts.readLine();   // read the 2 header lines
            }
         }
         i=0;   // i is a row counter
         while (!ts.eof() && i<2048)   //load maximally 2048 points (limit of XLA)
         {
            ts >> radius[scan_counter][i];
            ts >> absorbance[scan_counter][i];
            temp_radius[scan_counter][i] = radius[scan_counter][i]; 
            temp_absorbance[scan_counter][i] = absorbance[scan_counter][i];
            if (edit_type == 1 || edit_type == 2 || edit_type == 5)    // get rid of third column (std.dev)
            {
               ts >> trashcan;
            }
            i++;
         }
         points[scan_counter] = i - 1;
         temp_points[scan_counter] = (unsigned int) i - 1;
         f.close();
         qApp->processEvents();
         progress->setProgress(scan_counter+1);
      }


      //
      // increment scan according to wavelength count, if more than
      //   one wavelength is present, we need to skip scans. Also, we need to add one.
      //
      scan += run_inf.wavelength_count[cell];
      //
      // scan_counter simply is incremented to denote the next scan (used as index to arrays
      // radius, absorbance and points).
      //
      scan_counter++;
   }
   run_inf.centerpiece[cell] = current_centerpiece;   // we need to set the centerpiece for each cell at this point
   if (dataset_counter == count2)   // we're all done editing!
   {
      str.sprintf(tr("Please select the appropriate centerpiece, rotor\nand experiment type, and then commit the information\nto the database..."));
      lbl_instructions2->setText(str);
      pb_accept->setEnabled(true);
      pb_accept->setText("Commit to Database");
      plot_dataset();
      return;
   }
   else
   {
      str.sprintf(tr("Please confirm the Centerpiece and Rotor Settings below,\n"
                     "then proceed by clicking on \"Next Cell\""));
      lbl_instructions2->setText(str);
      plot_dataset();
   }
}
