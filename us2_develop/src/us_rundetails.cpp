#include "../include/us_rundetails.h"
#include <algorithm>
#include <functional>


RunDetails_F::RunDetails_F(int ed, int stat, struct runinfo *temp_run_inf, QWidget *p , const char *name)
   : QFrame(p, name)
{
   close_flag = false;    // flag is needed to tell closeEvent if we can close (cancel) without defining run_id
   // if true, we can cancel without defining run_id
   unsigned int i, j, count=0;
   run_inf = temp_run_inf;
   status = stat;
   if (status == -1)
   {
      GUI = false;
   }
   else
   {
      GUI = true;
   }

   /*
     cout << "entering RunDetails().. " << endl;
     {
     cout << "Directory: " << (*run_inf).data_dir << endl;
     cout << "Run Id: " <<  (*run_inf).run_id << endl;
     cout << "Run duration: " << (*run_inf).duration << endl;
     cout << "Total scans: " << (*run_inf).total_scans << endl;
     cout << "Delta r: " << (*run_inf).delta_r << endl;
     for (i=0; i<8; i++)
     {
     cout << "(*run_inf).centerpiece[" << i << "]: " << (*run_inf).centerpiece[i] << endl;
     cout << "(*run_inf).cell_id[" << i << "]: " << (*run_inf).cell_id[i] << endl;
     cout << "(*run_inf).wavelength_count[" << i << "]: " << (*run_inf).wavelength_count[i] << endl;
     }
     for (i=0; i<8; i++)
     {
     for (j=0; j<3; j++)
     {
     cout << "(*run_inf).wavelength[" << i << "][" << j << "]: " << (*run_inf).wavelength[i][j] << endl;
     cout << "(*run_inf).scans[" << i << "][" << j << "]: " << (*run_inf).scans[i][j] << endl;
     if ((*run_inf).centerpiece[i] >= 0 && (*run_inf).scans[i][j] > 0)
     {
     for (unsigned int k=0; k<1; k++)
     {
     cout << "(*run_inf).range_left[" << i << "][" << j << "][" << k << "]: " << (*run_inf).range_left[i][j][k] << endl;
     cout << "(*run_inf).range_right[" << i << "][" << j << "][" << k << "]: " << (*run_inf).range_right[i][j][k] << endl;
     cout << "(*run_inf).point_density[" << i << "][" << j << "][" << k << "]: " << (*run_inf).point_density[i][j][k] << endl;
     }
     }
     }
     }
     for (i=0; i<8; i++)
     {
     cout << "wavelength_count[" << i << "]: " << (*run_inf).wavelength_count[i] << endl;
     for (j=0; j<(*run_inf).wavelength_count[i]; j++)
     {
     cout << "scans[" << i << "][" << j << "]: " << (*run_inf).scans[i][j] << endl;
     for (unsigned int k=0; k<(*run_inf).scans[i][j]; k++)
     {
     cout << "rpm[" << i << "][" << j << "][" << k << "]: " << (*run_inf).rpm[i][j][k] << endl;
     cout << "temperature[" << i << "][" << j << "][" << k << "]: " << (*run_inf).temperature[i][j][k] << endl;
     cout << "time[" << i << "][" << j << "][" << k << "]: " << (*run_inf).time[i][j][k] << endl;
     cout << "omega[" << i << "][" << j << "][" << k << "]: " << (*run_inf).omega_s_t[i][j][k] << endl;
     }
     }
     }
     cout << "Rotor: " << (*run_inf).rotor << endl;
     }
   */
   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   edit_type = ed;
   pm = new US_Pixmap();
   if ((edit_type == 1 || edit_type == 3 || edit_type == 5) && GUI)
   {
      setCaption(tr("Details for Velocity Data"));
   }
   else
   {
      setCaption(tr("Details for Equilibrium Data"));
   }
   unsigned int **scan_counter;   // 2 dimensional array which counts the scans for each cell and each wavelength
   QString str;
   plot_counter=0;      // 0: temperature vs. scans, 1: delta_time vs. scans, 3: speed vs. scans
   sym1.setStyle(QwtSymbol::Ellipse);
   sym1.setPen(blue);
   sym1.setBrush(white);
   sym1.setSize(8);
   sym2.setStyle(QwtSymbol::Rect);
   sym2.setPen(Qt::red);
   sym2.setBrush(Qt::white);
   sym2.setSize(8);
   temp_sum=0;
   correction_sum=0;
   blink=1;
   scan_counter = new unsigned int* [8];   // keeps track of the scan number for each cell and each wavelength
   for (i=0; i<8; i++)
   {
      scan_counter[i] = new unsigned int [3];
   }
   for (i=0; i<8; i++)
   {
      for (j=0; j<3; j++)
      {
         scan_counter[i][j] = 0;
      }
   }
   scans = new double [(*run_inf).total_scans];   // x-variable for the plot
   tempy1 = new double [(*run_inf).total_scans];
   tempy2 = new double [(*run_inf).total_scans - 1];   //the delta_t's need one less
   tempy3 = new double [(*run_inf).total_scans];
   first_cell=0;
   for (i=0; i<8; i++)
   {
      has_data[i]=false;
      if ((*run_inf).scans[i][0] != 0)
      {
         has_data[i]=true;
      }
   }
   i = 0;
   while(!has_data[i] && i<8)
   {
      i ++;
   }
   if (i == 8)
   {
      QString str = ("Could not find any data!\n\n"
                     "Please make sure you selected\n"
                     "the correct directory...");
      if (GUI)
      {
         QMessageBox::message(tr("Warning"), str);
      }
      else
      {
         cout << str.latin1() << endl;
      }
      return;
   }
   first_cell = i;      // first cell with data in it, needed for rpm.   if (edit_type == 1 || edit_type == 3)
   //cout << "Omega: " << omega << endl;
   for (i=0; i<(*run_inf).total_scans; i++)
   {
      scans[i] = (double) i+1;
   }
   count = 0;
   while (count < (*run_inf).total_scans)
   {
      for (i=0; i<8; i++)   //cell
      {
         for (j=0; j<3; j++)
         {
            if ((*run_inf).wavelength_count[i] > j)
            {
               if (scan_counter[i][j] < (*run_inf).scans[i][j])
               {
                  tempy3[count] = (double) (*run_inf).rpm[i][j][scan_counter[i][j]];
                  scan_counter[i][j]++;
                  count++;
               }
            }
         }
      }
   }
   for (i=0; i<8; i++)
   {
      for (j=0; j<3; j++)
      {
         scan_counter[i][j] = 0;
      }
   }
   avg_rpm = 0.0;
   for (unsigned int i=0; i<(*run_inf).total_scans; i++)
   {
      avg_rpm += tempy3[i];
   }
   avg_rpm /= (float) (*run_inf).total_scans;
   omega = (M_PI / 30) * avg_rpm;
   vector <unsigned int> time_v;    // temporary storage for the time of scans
   count = 0;
   while (count < (*run_inf).total_scans)
   {
      //cout << "pt 1, count=" << count << ", total_scans=" << (*run_inf).total_scans<< "\n";
      for (i=0; i<8; i++)   //cell
      {
         //cout << "cell=" << i << "\n";
         for (j=0; j<3; j++)
         {
            //cout << "Cell: " << i << ", Lambda=" << j << ", lambda_count=" << (*run_inf).wavelength_count[i] << "\n";
            if ((*run_inf).wavelength_count[i] > j)
            {
               //cout << "i=" << i << ", lambda_count=" << (*run_inf).wavelength_count[i] << ", j=" << j << "\n";
               if (scan_counter[i][j] < (*run_inf).scans[i][j])
               {
                  //cout << "Time: " << (*run_inf).time[i][j][scan_counter[i][j]] << endl;
                  time_v.push_back((*run_inf).time[i][j][scan_counter[i][j]]);
                  tempy1[count] = (double) (*run_inf).temperature[i][j][scan_counter[i][j]];
                  tempy3[count] = (double) (*run_inf).rpm[i][j][scan_counter[i][j]]/1000;
                  temp_sum += (*run_inf).temperature[i][j][scan_counter[i][j]];

                  // this doesn't have to be in order for cases where scans have been deleted during editing:
                  //cout << "Time: " << (*run_inf).time[i][j][scan_counter[i][j]] << endl;
                  correction_sum += (*run_inf).time[i][j][scan_counter[i][j]] -
                     ((*run_inf).omega_s_t[i][j][scan_counter[i][j]] / (float) pow (omega, 2));
                  //cout << "Correction_sum: " << correction_sum << ", omega_s_t: " << (*run_inf).omega_s_t[i][j][scan_counter[i][j]] << ", time: " << (*run_inf).time[i][j][scan_counter[i][j]] << endl;
                  last_cell = i;
                  last_wavelength = j;
                  last_scan = scan_counter[i][j];
                  scan_counter[i][j]++;
                  count++;
                  //cout << "Count: " << count << endl;
               }
            }
         }
      }
   }
   for (unsigned int i=0; i<8; i++)
   {
      delete [] scan_counter[i];      //no longer needed
   }
   delete [] scan_counter;
   sort(time_v.begin(), time_v.end(), less<unsigned int>());   // sort the time vector, shortest time first
   vector <unsigned int>::iterator v_it;
   count = 0;
   for (v_it=time_v.begin(); v_it != time_v.end()-1; v_it++)
   {
      tempy2[count] = (double) ((*(v_it+1) - *v_it)/60.0);   //assign the delta_t's in minutes to tempy2
      count++;
   }
   (*run_inf).temperature_check = 1;
   if ((qwtGetMax(tempy1, count) - qwtGetMin(tempy1, count)) > USglobal->config_list.temperature_tol)
   {
      (*run_inf).temperature_check = -1;      // difference is too large
   }
   if (GUI)
   {
      create_gui();
   }
   update_screen();
}

RunDetails_F::~RunDetails_F()
{
}

void RunDetails_F::create_gui()
{
   unsigned int i, j, k;
   QString str;

   diagnose_plt = new QwtPlot(this);
   diagnose_plt->enableOutline(true);
   diagnose_plt->setOutlinePen(white);
   diagnose_plt->setOutlineStyle(Qwt::Cross);
   diagnose_plt->enableGridYMin();
   diagnose_plt->enableGridXMin();
   diagnose_plt->setMinimumHeight(285);
   diagnose_plt->setPalette( QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
   diagnose_plt->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   diagnose_plt->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
   diagnose_plt->setCanvasBackground(USglobal->global_colors.plot);      //new version
   diagnose_plt->setMargin(USglobal->config_list.margin);
   diagnose_plt->enableAxis(QwtPlot::yLeft);
   diagnose_plt->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   diagnose_plt->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   diagnose_plt->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   diagnose_plt->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   diagnose_plt->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   diagnose_plt->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   diagnose_plt->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setAutoDefault(false);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(tr("Cancel"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setAutoDefault(false);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));
   connect(pb_cancel, SIGNAL(clicked()), SIGNAL(clicked()));

   pb_accept = new QPushButton(tr("Accept"), this);
   Q_CHECK_PTR(pb_accept);
   pb_accept->setAutoDefault(false);
   pb_accept->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_accept->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_accept, SIGNAL(clicked()), SLOT(accept()));
   connect(pb_accept, SIGNAL(clicked()), SIGNAL(clicked()));

   pb_plot_type = new QPushButton(tr("Time Plot"), this);
   Q_CHECK_PTR(pb_plot_type);
   pb_plot_type->setAutoDefault(false);
   pb_plot_type->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_plot_type->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_plot_type, SIGNAL(clicked()), SLOT(plot_type()));
   connect(pb_plot_type, SIGNAL(clicked()), SIGNAL(clicked()));

   directory_lbl1 = new QLabel(tr(" Data Directory:"), this);
   directory_lbl1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   directory_lbl1->setAlignment(AlignLeft|AlignVCenter);
   directory_lbl1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   directory_le = new QLineEdit(this);
   directory_le->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   directory_le->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   directory_le->setReadOnly(true);

   temperature_lbl = new QLabel(tr(" Temperature Check:"), this);
   temperature_lbl->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   temperature_lbl->setAlignment(AlignLeft|AlignVCenter);
   temperature_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lcd1 = new QLabel("", this);
   lcd1->setFixedSize(20, 16);
   lcd1->setFrameStyle(QFrame::WinPanel|Sunken);
   lcd1->setPalette( QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green) );

   lcd2 = new QLabel("", this);
   lcd2->setFixedSize(20, 16);
   lcd2->setFrameStyle(QFrame::WinPanel|Sunken);
   lcd2->setPalette( QPalette(USglobal->global_colors.cg_dk_red, USglobal->global_colors.cg_dk_red, USglobal->global_colors.cg_dk_red) );

   run_id_lbl1 = new QLabel(tr(" Run Identification:"), this);
   run_id_lbl1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   run_id_lbl1->setAlignment(AlignLeft|AlignVCenter);
   run_id_lbl1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   run_id_le = new QLineEdit(this);
   run_id_le->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   run_id_le->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   run_id_le->setText((*run_inf).run_id);
   if (status == 0 || status == 2 || status == 3)   // we are editing data, we want to be able to change the ID
   {
      connect(run_id_le, SIGNAL(textChanged(const QString &)), this, SLOT(update_id(const QString &)));
      connect(run_id_le, SIGNAL(returnPressed()), SLOT(accept()));
      connect(run_id_le, SIGNAL(returnPressed()), SIGNAL(returnPressed()));
   }
   else
   {
      run_id_le->setReadOnly(true);
   }

   data_cells_lbl1 = new QLabel(tr(" Cells with Data:"), this);
   data_cells_lbl1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   data_cells_lbl1->setAlignment(AlignLeft|AlignVCenter);
   data_cells_lbl1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   data_cells_lbl2 = new QLabel("", this);
   data_cells_lbl2->setFrameStyle(QFrame::WinPanel|Sunken);
   data_cells_lbl2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   data_cells_lbl2->setAlignment(AlignLeft|AlignVCenter);
   data_cells_lbl2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   avg_temperature_lbl1 = new QLabel(tr(" Avg. Temperature:"), this);
   avg_temperature_lbl1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   avg_temperature_lbl1->setAlignment(AlignLeft|AlignVCenter);
   avg_temperature_lbl1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   avg_temperature_lbl2 = new QLabel("", this);
   avg_temperature_lbl2->setFrameStyle(QFrame::WinPanel|Sunken);
   avg_temperature_lbl2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   avg_temperature_lbl2->setAlignment(AlignLeft|AlignVCenter);
   avg_temperature_lbl2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   cell_description_lbl1 = new QLabel(tr(" Cell Description:"), this);
   cell_description_lbl1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   cell_description_lbl1->setAlignment(AlignLeft|AlignVCenter);
   cell_description_lbl1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cell_description = new QLineEdit(this);
   cell_description->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   cell_description->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cell_description, SIGNAL(textChanged(const QString &)), this, SLOT(update_cell(const QString &)));

   run_length_lbl1 = new QLabel(tr(" Length of Run:"), this);
   run_length_lbl1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   run_length_lbl1->setAlignment(AlignLeft|AlignVCenter);
   run_length_lbl1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   run_length_lbl2 = new QLabel("", this);
   run_length_lbl2->setFrameStyle(QFrame::WinPanel|Sunken);
   run_length_lbl2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   run_length_lbl2->setAlignment(AlignLeft|AlignVCenter);
   run_length_lbl2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   cell_select = new QListBox(this, "Model");
   cell_select->insertItem(tr("Cell 1"));
   cell_select->insertItem(tr("Cell 2"));
   cell_select->insertItem(tr("Cell 3"));
   cell_select->insertItem(tr("Cell 4"));
   cell_select->insertItem(tr("Cell 5"));
   cell_select->insertItem(tr("Cell 6"));
   cell_select->insertItem(tr("Cell 7"));
   cell_select->insertItem(tr("Cell 8"));
   cell_select->setFixedSize(100, 97);
   cell_select->setSelected(0, TRUE);
   cell_select->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cell_select->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   connect(cell_select, SIGNAL(highlighted(int)), SLOT(show_cell(int)));

   wavelength1_lbl1 = new QLabel(tr(" Wavelength 1:"), this);
   wavelength1_lbl1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   wavelength1_lbl1->setAlignment(AlignLeft|AlignVCenter);
   wavelength1_lbl1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   wavelength1_lbl2 = new QLabel(tr("no Data available"), this);
   wavelength1_lbl2->setFrameStyle(QFrame::WinPanel|Sunken);
   wavelength1_lbl2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   wavelength1_lbl2->setAlignment(AlignLeft|AlignVCenter);
   wavelength1_lbl2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   scans1_lbl1 = new QLabel(tr(" Scans:"), this);
   scans1_lbl1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   scans1_lbl1->setAlignment(AlignLeft|AlignVCenter);
   scans1_lbl1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   scans1_lbl2 = new QLabel("", this);
   scans1_lbl2->setFrameStyle(QFrame::WinPanel|Sunken);
   scans1_lbl2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   scans1_lbl2->setAlignment(AlignLeft|AlignVCenter);
   scans1_lbl2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   if (edit_type == 1 || edit_type == 3 || edit_type == 5)
   {
      time_correction_lbl1 = new QLabel(tr(" Time Correction:"), this);
      time_correction_lbl1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
      time_correction_lbl1->setAlignment(AlignLeft|AlignVCenter);
      time_correction_lbl1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

      time_correction_lbl2 = new QLabel("", this);
      time_correction_lbl2->setFrameStyle(QFrame::WinPanel|Sunken);
      time_correction_lbl2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
      time_correction_lbl2->setAlignment(AlignLeft|AlignVCenter);
      time_correction_lbl2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   }
   else if (edit_type == 2 || edit_type == 4)
   {
      uint test_rpm = 0;
      rpm_list_lbl = new QLabel(tr(" Available\n Rotor\n Speeds:"), this);
      rpm_list_lbl->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
      rpm_list_lbl->setAlignment(AlignLeft|AlignVCenter);
      rpm_list_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
      rpm_list = new QListBox(this, "Speeds");
      rpm_list->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      rpm_list->setFixedSize(104, 58);
      rpm_list->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
      connect(rpm_list, SIGNAL(highlighted(const QString &)), SLOT(show_speed(const QString &)));
      for (i=0; i<8; i++)
      {
         for (j=0; j<3; j++)
         {
            for (k=0; k<(*run_inf).scans[i][j]; k++)
            {
               if ((*run_inf).rpm[i][j][k] > test_rpm)
               {
                  test_rpm = (*run_inf).rpm[i][j][k];
                  str.sprintf("%d rpm", test_rpm);
                  rpm_list->insertItem(str);
               }
            }
         }
      }
      cell_select->setSelected(0, TRUE);
   }

   wavelength2_lbl1 = new QLabel(tr(" Wavelength 2:"), this);
   wavelength2_lbl1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   wavelength2_lbl1->setAlignment(AlignLeft|AlignVCenter);
   wavelength2_lbl1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   wavelength2_lbl2 = new QLabel(tr("no Data available"), this);
   wavelength2_lbl2->setFrameStyle(QFrame::WinPanel|Sunken);
   wavelength2_lbl2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   wavelength2_lbl2->setAlignment(AlignLeft|AlignVCenter);
   wavelength2_lbl2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   scans2_lbl1 = new QLabel(tr(" Scans:"), this);
   scans2_lbl1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   scans2_lbl1->setAlignment(AlignLeft|AlignVCenter);
   scans2_lbl1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   scans2_lbl2 = new QLabel("", this);
   scans2_lbl2->setFrameStyle(QFrame::WinPanel|Sunken);
   scans2_lbl2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   scans2_lbl2->setAlignment(AlignLeft|AlignVCenter);
   scans2_lbl2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   if (edit_type == 1 || edit_type == 3 || edit_type == 5)
   {
      speed_lbl1 = new QLabel(tr(" Rotor Speed:"), this);
      speed_lbl1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
      speed_lbl1->setAlignment(AlignLeft|AlignVCenter);
      speed_lbl1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

      speed_lbl2 = new QLabel(tr("0 rpm"), this);
      speed_lbl2->setFrameStyle(QFrame::WinPanel|Sunken);
      speed_lbl2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
      speed_lbl2->setAlignment(AlignLeft|AlignVCenter);
      speed_lbl2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   }

   cellbottom_lbl1 = new QLabel(tr(" Bottom for this cell:"), this);
   cellbottom_lbl1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   cellbottom_lbl1->setAlignment(AlignLeft|AlignVCenter);
   cellbottom_lbl1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cellbottom_lbl2 = new QLabel(tr(""), this);
   cellbottom_lbl2->setFrameStyle(QFrame::WinPanel|Sunken);
   cellbottom_lbl2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   cellbottom_lbl2->setAlignment(AlignLeft|AlignVCenter);
   cellbottom_lbl2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   
   wavelength3_lbl1 = new QLabel(tr(" Wavelength 3:"), this);
   wavelength3_lbl1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   wavelength3_lbl1->setAlignment(AlignLeft|AlignVCenter);
   wavelength3_lbl1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   wavelength3_lbl2 = new QLabel(tr("no Data available"), this);
   wavelength3_lbl2->setFrameStyle(QFrame::WinPanel|Sunken);
   wavelength3_lbl2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   wavelength3_lbl2->setAlignment(AlignLeft|AlignVCenter);
   wavelength3_lbl2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   scans3_lbl1 = new QLabel(tr(" Scans:"), this);
   scans3_lbl1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   scans3_lbl1->setAlignment(AlignLeft|AlignVCenter);
   scans3_lbl1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   scans3_lbl2 = new QLabel("", this);
   scans3_lbl2->setFrameStyle(QFrame::WinPanel|Sunken);
   scans3_lbl2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   scans3_lbl2->setAlignment(AlignLeft|AlignVCenter);
   scans3_lbl2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   show_cell(0);
   setup_GUI();
   qApp->processEvents();
   plot_type();
}

void RunDetails_F::setup_GUI()
{
   QGridLayout * background = new QGridLayout(this,3,1,4);
   background->addWidget(diagnose_plt,0,0);
   QHBoxLayout *button= new QHBoxLayout(2);
   button->addWidget(pb_help);
   button->addWidget(pb_cancel);
   button->addWidget(pb_accept);
   button->addWidget(pb_plot_type);
   background->addLayout(button,1,0);

   QGridLayout * panel1 = new QGridLayout(5,2,2);
   for (int i=0; i<5; i++)
   {
      panel1->setRowSpacing(i, 26);
   }
   panel1->addWidget(temperature_lbl,0,0);
   QHBoxLayout *lcd= new QHBoxLayout(0);
   lcd->addWidget(lcd1);
   lcd->addWidget(lcd2);
   panel1->addLayout(lcd,0,1);
   panel1->addWidget(avg_temperature_lbl1,1,0);
   panel1->addWidget(avg_temperature_lbl2,1,1);
   panel1->addWidget(run_length_lbl1,2,0);
   panel1->addWidget(run_length_lbl2,2,1);
   if (edit_type == 1 || edit_type == 3 || edit_type == 5)
   {
      panel1->addWidget(time_correction_lbl1,3,0);
      panel1->addWidget(time_correction_lbl2,3,1);
      panel1->addWidget(speed_lbl1,4,0);
      panel1->addWidget(speed_lbl2,4,1);
      panel1->addWidget(cellbottom_lbl1,5,0);
      panel1->addWidget(cellbottom_lbl2,5,1);
   }
   else if (edit_type == 2 || edit_type == 4)
   {
      panel1->addMultiCellWidget(rpm_list_lbl,3,4,0,0);
      panel1->addMultiCellWidget(rpm_list,3,4,1,1);
   }

   QGridLayout * panel2 = new QGridLayout(2,4,2);
   for (int i=0; i<2; i++)
   {
      panel2->setRowSpacing(i, 26);
   }
   panel2->setColStretch(1, 3);
   panel2->setColStretch(3, 2);
   panel2->addWidget(directory_lbl1,0,0);
   panel2->addMultiCellWidget(directory_le,0,0,1,3);
   panel2->addWidget(run_id_lbl1,1,0);
   panel2->addWidget(run_id_le,1,1);
   panel2->addWidget(data_cells_lbl1,1,2);
   panel2->addWidget(data_cells_lbl2,1,3);
   panel2->addWidget(cell_description_lbl1,2,0);
   panel2->addMultiCellWidget(cell_description,2,2,1,3);

   QGridLayout * panel3 = new QGridLayout(3,5,2);
   for (int i=0; i<3; i++)
   {
      panel3->setRowSpacing(i, 26);
   }
   panel3->setColStretch(2, 3);
   panel3->setColStretch(4, 3);
   panel3->addMultiCellWidget(cell_select,0,2,0,0);
   panel3->addWidget(wavelength1_lbl1,0,1);
   panel3->addWidget(wavelength1_lbl2,0,2);
   panel3->addWidget(scans1_lbl1,0,3);
   panel3->addWidget(scans1_lbl2,0,4);
   panel3->addWidget(wavelength2_lbl1,1,1);
   panel3->addWidget(wavelength2_lbl2,1,2);
   panel3->addWidget(scans2_lbl1,1,3);
   panel3->addWidget(scans2_lbl2,1,4);
   panel3->addWidget(wavelength3_lbl1,2,1);
   panel3->addWidget(wavelength3_lbl2,2,2);
   panel3->addWidget(scans3_lbl1,2,3);
   panel3->addWidget(scans3_lbl2,2,4);


   QVBoxLayout *subGrid1 = new QVBoxLayout(2);
   subGrid1->addLayout(panel2);
   subGrid1->addLayout(panel3);

   QHBoxLayout *subGrid2 = new QHBoxLayout(4);
   subGrid2->addLayout(panel1);
   subGrid2->addLayout(subGrid1);

   background->addLayout(subGrid2,2,0);

   qApp->processEvents();
   QRect r = background->cellGeometry(0, 0);

   global_Xpos += 30;
   global_Ypos += 30;

   this->setGeometry(global_Xpos, global_Ypos, r.width()-300, r.height());

}

void RunDetails_F::closeEvent(QCloseEvent *e)
{
   if ((*run_inf).run_id == "" && !close_flag)
   {
      QMessageBox::message(tr("Attention:"),tr("Please enter a unique run\n"
                                               "identification for this data set\n"
                                               "before proceeding..."));
      return;
   }
   else
   {
      e->accept();
      emit isClosed();
      global_Xpos -= 30;
      global_Ypos -= 30;
      delete [] tempy1;
      delete [] tempy2;
      delete [] tempy3;
   }
}

void RunDetails_F::update_id(const QString &newtext)
{
   QString test = newtext;
   if(test.contains(".")>0)
   {
      QMessageBox::message(tr("Attention:"),tr("The unique run identification can NOT contain periods,\n"
                                               "Please remove any periods before proceeding..."));
      return;
   }
   if(test.contains(" ")>0)
   {
      QMessageBox::message(tr("Attention:"),tr("The unique run identification can NOT contain white space,\n"
                                               "Please remove white space before proceeding..."));
      return;
   }
   if(test.contains("/")>0)
   {
      QMessageBox::message(tr("Attention:"),tr("The unique run identification can NOT contain '/',\n"
                                               "Please remove '/' before proceeding..."));
      return;
   }
   if(test.contains("~")>0)
   {
      QMessageBox::message(tr("Attention:"),tr("The unique run identification can NOT contain '~',\n"
                                               "Please remove '~' before proceeding..."));
      return;
   }
   if(test.contains("!")>0)
   {
      QMessageBox::message(tr("Attention:"),tr("The unique run identification can NOT contain '!',\n"
                                               "Please remove '!' before proceeding..."));
      return;
   }
   if(test.contains("@")>0)
   {
      QMessageBox::message(tr("Attention:"),tr("The unique run identification can NOT contain '@',\n"
                                               "Please remove '@' before proceeding..."));
      return;
   }
   if(test.contains("$")>0)
   {
      QMessageBox::message(tr("Attention:"),tr("The unique run identification can NOT contain '$',\n"
                                               "Please remove '$' before proceeding..."));
      return;
   }
   if(test.contains("^")>0)
   {
      QMessageBox::message(tr("Attention:"),tr("The unique run identification can NOT contain '^',\n"
                                               "Please remove '^' before proceeding..."));
      return;
   }
   if(test.contains("&")>0)
   {
      QMessageBox::message(tr("Attention:"),tr("The unique run identification can NOT contain '&',\n"
                                               "Please remove '&' before proceeding..."));
      return;
   }
   if(test.contains("*")>0)
   {
      QMessageBox::message(tr("Attention:"),tr("The unique run identification can NOT contain '*',\n"
                                               "Please remove '@' before proceeding..."));
      return;
   }
   if(test.contains("=")>0)
   {
      QMessageBox::message(tr("Attention:"),tr("The unique run identification can NOT contain '=',\n"
                                               "Please remove '=' before proceeding..."));
      return;
   }
   if(test.contains("|")>0)
   {
      QMessageBox::message(tr("Attention:"),tr("The unique run identification can NOT contain '|',\n"
                                               "Please remove '|' before proceeding..."));
      return;
   }
   if(test.contains("?")>0)
   {
      QMessageBox::message(tr("Attention:"),tr("The unique run identification can NOT contain '?',\n"
                                               "Please remove '?' before proceeding..."));
      return;
   }
   if(test.contains(",")>0)
   {
      QMessageBox::message(tr("Attention:"),tr("The unique run identification can NOT contain ',',\n"
                                               "Please remove ',' before proceeding..."));
      return;
   }
   if(test.contains(";")>0)
   {
      QMessageBox::message(tr("Attention:"),tr("The unique run identification can NOT contain ';',\n"
                                               "Please remove ';' before proceeding..."));
      return;
   }
   if(test.contains("(")>0 || test.contains(")")>0 )
   {
      QMessageBox::message(tr("Attention:"),tr("The unique run identification can NOT contain '(' or ')',\n"
                                               "Please remove '(' or ')' before proceeding..."));
      return;
   }
   if(test.contains("<")>0 || test.contains(">")>0 )
   {
      QMessageBox::message(tr("Attention:"),tr("The unique run identification can NOT contain '<' or '>',\n"
                                               "Please remove '<' or '>' before proceeding..."));
      return;
   }
   if(test.contains("{")>0 || test.contains("}")>0 )
   {
      QMessageBox::message(tr("Attention:"),tr("The unique run identification can NOT contain '{' or '}',\n"
                                               "Please remove '{' or '}' before proceeding..."));
      return;
   }

   (*run_inf).run_id = newtext;
}

void RunDetails_F::plot_type()
{
   switch (plot_counter)
   {
   case 0:
      {
         diagnose_plt->clear();
         diagnose_plt->setTitle(tr("Temperature Variation Throughout Run"));
         diagnose_plt->setAxisTitle(QwtPlot::xBottom, tr("Scan Number"));
         diagnose_plt->setAxisTitle(QwtPlot::yLeft, tr("Temperature in ºC"));
         temperature_points = diagnose_plt->insertCurve("Temperature");
         diagnose_plt->setCurveYAxis(temperature_points, QwtPlot::yLeft);
         diagnose_plt->setCurveSymbol(temperature_points, QwtSymbol(sym1));
         diagnose_plt->setCurveData(temperature_points, scans, tempy1, (*run_inf).total_scans);
         diagnose_plt->setCurvePen(temperature_points, Qt::yellow);
         pb_plot_type->setText(tr("Time Plot"));
         break;
      }
   case 1:
      {
         diagnose_plt->clear();
         diagnose_plt->setTitle(tr("Recorded Time Differences between Scans"));
         diagnose_plt->setAxisTitle(QwtPlot::xBottom, tr("Scan Number"));
         diagnose_plt->setAxisTitle(QwtPlot::yLeft, tr("Time between Scans (min)"));
         time_points = diagnose_plt->insertCurve("Delta Time");
         diagnose_plt->setCurveYAxis(time_points, QwtPlot::yLeft);
         diagnose_plt->setCurveSymbol(time_points, QwtSymbol(sym2));
         diagnose_plt->setCurveData(time_points, scans, tempy2, (*run_inf).total_scans-1);
         diagnose_plt->setCurvePen(time_points, Qt::red);
         pb_plot_type->setText(tr("Speed Plot"));
         break;
      }
   case 2:
      {
         diagnose_plt->clear();
         diagnose_plt->setTitle(tr("Rotor Speed for each Scan"));
         diagnose_plt->setAxisTitle(QwtPlot::xBottom, tr("Scan Number"));
         diagnose_plt->setAxisTitle(QwtPlot::yLeft, tr("RPM (x1000)"));
         speed_points = diagnose_plt->insertCurve("Rotor Speed");
         diagnose_plt->setCurveYAxis(speed_points, QwtPlot::yLeft);
         diagnose_plt->setCurveSymbol(speed_points, QwtSymbol(sym1));
         diagnose_plt->setCurveData(speed_points, scans, tempy3, (*run_inf).total_scans);
         diagnose_plt->setCurvePen(speed_points, green);
         pb_plot_type->setText(tr("Temp. Plot"));
         break;
      }
   }
   diagnose_plt->replot();
   if (status == 1 || status == 2 || status == 3)   // if the run name is already available we can make the plots
   {                  // we don't want to use this function for data that has not been excluded yet
      // during editing, only after the run has been written to the disk as edited data
      qApp->processEvents(); // let's wait until the pixmap is updated
      QString htmlDir = USglobal->config_list.html_dir + "/" + (*run_inf).run_id;
      QDir d(htmlDir);
      QPixmap p;
      QString fileName;
      if (!d.exists())
      {
         d.mkdir(htmlDir, true);
      }

      switch(plot_counter)
      {
      case 0:
         {
            fileName = "/temperature_data.";
            break;
         }
      case 1:
         {
            fileName = "/time_data.";
            break;
         }
      case 2:
         {
            fileName = "/rotorspeed_data.";
            break;
         }
      }
      diagnose_plt->raise();
      diagnose_plt->replot();
      qApp->processEvents();
      fileName = htmlDir + fileName;
      p = QPixmap::grabWidget(diagnose_plt, 2, 2, diagnose_plt->width() - 4, diagnose_plt->height() - 4);
      pm->save_file(fileName, p);
      qApp->processEvents();
   }
   plot_counter ++;
   if (plot_counter > 2)
   {
      plot_counter = 0;
   }
}

void RunDetails_F::update_cell(const QString &newtext)
{
   if (has_data[active_cell])
   {
      (*run_inf).cell_id[active_cell] = newtext;
   }
   else
   {
      (*run_inf).cell_id[active_cell] = ""; // if there is not data, keep the cell id empty
   }
}


void RunDetails_F::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   if (status == 2) // rename cell id's
   {
      online_help->show_help("manual/rename.html");
      return;
   }
   if (status == 3) // rename cell id's and copy run
   {
      online_help->show_help("manual/copy_run.html");
      return;
   }
   if (edit_type == 1 || edit_type == 3 || edit_type == 5)
   {
      online_help->show_help("manual/details_veloc.html");
      return;
   }
   if (edit_type == 2 || edit_type == 4)
   {
      online_help->show_help("manual/details_equil.html");
      return;
   }
}

void RunDetails_F::cancel()
{
   close_flag = true;
   close();
}

void RunDetails_F::accept()
{
   if ((*run_inf).run_id == "")
   {
      QMessageBox::message(tr("Attention:"),tr("Please enter a unique run\n"
                                               "identification for this data set\n"
                                               "before proceeding..."));
   }
   else
   {
      close();
   }
}

void RunDetails_F::show_speed(const QString &newtext)
{
   QString str1(newtext), str2(tr("The following scans have been measured at ")), str3;
   str2.append(str1);
   str2.append(":\n");
   str1.truncate(str1.length() - 4);   // 3 for the extension and one for the null character
   uint test_rpm = str1.toUInt();

   unsigned int i, j, k;
   for (i=0; i<8; i++)
   {
      for (j=0; j<3; j++)
      {
         for (k=0; k<(*run_inf).scans[i][j]; k++)
         {
            if ((*run_inf).rpm[i][j][k] == test_rpm)
            {
               str2 += "\n";
               str3.sprintf(tr("Cell: %d, Scan: %d, Wavelength: %d"), i+1, k+1, j+1);
               str2 += (str3);
            }
         }
      }
   }
   QMessageBox::message(tr("Speed Information"), str2);
}

void RunDetails_F::show_cell(int current_cell)
{
   QString str;
   active_cell = current_cell;
   if ((*run_inf).wavelength[current_cell][0] == 0)
   {
      cell_description->setText(tr("No Data available for this cell"));
      wavelength1_lbl2->setText(tr("No Data available"));
      scans1_lbl2->setText(tr("No Scans"));
      wavelength2_lbl2->setText(tr("No Data available"));
      scans2_lbl2->setText(tr("No Scans"));
      wavelength3_lbl2->setText(tr("No Data available"));
      scans3_lbl2->setText(tr("No Scans"));
      if (edit_type == 1 || edit_type == 3 || edit_type == 5)
      {
         cellbottom_lbl2->setText("");
      }      
   }
   else
   {
      cell_description->setText((*run_inf).cell_id[current_cell]);
      if (edit_type == 1 || edit_type == 3 || edit_type == 5)
      {
         vector < rotorInfo > rotor_list;
         vector <struct centerpieceInfo> cp_list;
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
         float bottom = calc_bottom(rotor_list, cp_list, (*run_inf).rotor, (*run_inf).centerpiece[current_cell],
               0, (*run_inf).rpm[current_cell][0][0]);
         cellbottom_lbl2->setText(str.sprintf(" %6.4f cm", bottom));
      }
      wavelength1_lbl2->setText(str.sprintf("%d nm", (*run_inf).wavelength[current_cell][0]));
      scans1_lbl2->setText(str.sprintf("%d", (*run_inf).scans[current_cell][0]));
      if ((*run_inf).wavelength[current_cell][1] != 0)
      {
         wavelength2_lbl2->setText(str.sprintf("%d nm", (*run_inf).wavelength[current_cell][1]));
         scans2_lbl2->setText(str.sprintf("%d", (*run_inf).scans[current_cell][1]));
      }
      else
      {
         wavelength2_lbl2->setText(tr("No Data available"));
         scans2_lbl2->setText(tr("No Scans"));
      }
      if ((*run_inf).wavelength[current_cell][2] != 0)
      {
         wavelength3_lbl2->setText(str.sprintf("%d nm", (*run_inf).wavelength[current_cell][2]));
         scans3_lbl2->setText(str.sprintf("%d", (*run_inf).scans[current_cell][2]));
      }
      else
      {
         wavelength3_lbl2->setText(tr("No Data available"));
         scans3_lbl2->setText(tr("No Scans"));
      }
   }
}

void RunDetails_F::update_screen()
{

   QString str, str2;
   unsigned int i;
   int l, m;
   if (edit_type == 1 || edit_type == 3 || edit_type == 5 && GUI)
   {
      speed_lbl2->setText(str.sprintf("%6.1f rpm", avg_rpm));
   }
   str = "";
   for (i=0; i<8; i++)
   {
      if ((*run_inf).scans[i][0] != 0)
      {
         if (str == "")
         {
            str.sprintf("%d", i+1);
         }
         else
         {
            str2.sprintf(", %d", i+1);
            str.append(str2);
         }
      }
      else
      {
         has_data[i]=false;
      }
   }
   if (GUI)
   {
      data_cells_lbl2->setText(str);

      QTimer *flash_time = new QTimer(this);
      if (((*run_inf).temperature_check < 1) && (edit_type == 1 || edit_type == 3 || edit_type == 5))
      {
         QMessageBox::message(tr("Attention:"), tr("The temperature in this run varied over the course\n"
                                                   "of the run to a larger extent than allowed by the\n"
                                                   "current threshold. The accuracy of experimental\n"
                                                   "results may be affected significantly."));
         connect(flash_time, SIGNAL(timeout()), SLOT(animate()));
         flash_time->start(200);
      }
      else
      {
         lcd1->setPalette( QPalette(USglobal->global_colors.cg_green, USglobal->global_colors.cg_green, USglobal->global_colors.cg_green) );
      }
   }
   (*run_inf).avg_temperature = temp_sum / (*run_inf).total_scans;
   if (GUI)
   {
      avg_temperature_lbl2->setText(str.sprintf("%5.3f ºC", (*run_inf).avg_temperature));
      directory_le->setText((*run_inf).data_dir);
   }
   if ((status == 0) && (edit_type == 1 || edit_type == 3 || edit_type == 5))
   {
      (*run_inf).time_correction = correction_sum / (*run_inf).total_scans;
      //      cout << "Time Corrections: " << (*run_inf).time_correction << endl;
      if ((*run_inf).time_correction < 0 && GUI)
      {
         QMessageBox::message(tr("Attention:"),tr("This dataset appears to have been aquired\n"
                                                  "with buggy Beckman Data Acquisition software.\n\n"
                                                  "If the time differences between scans are too\n"
                                                  "low, you need to enable the Beckman Bug correction\n"
                                                  "feature in the Configuration panel under the Filemenu.\n\n"
                                                  "Also: Make sure the speed was constant throughout\n"
                                                  "the experiment. Velocity experiments need to be done\n"
                                                  "at constant rotor speed"));
      }
      if ((*run_inf).time_correction > 1800.0 && USglobal->config_list.beckman_bug && GUI)
      {
         QMessageBox::message(tr("Attention:"),tr("The Beckman Buggy Data Acquisition Flag is set\n"
                                                  "and this dataset appears to have an unusually large\n"
                                                  "time correction.\n\n"
                                                  "Unless you are sure that these data were in fact acquired\n"
                                                  "with a buggy Beckman Data Acquisition software, you need\n"
                                                  "to disable the Beckman Bug correction feature in the\n"
                                                  "Configuration panel under the Filemenu before proceeding.\n\n"
                                                  "In that case you will also have to restart the editing\n"
                                                  "program and reload the data."));
      }
   }
   if (edit_type == 1 || edit_type == 3 || edit_type == 5)   // only for velocity experiments
   {
      l = (int) ((*run_inf).time_correction / 60);
      m = (int) (0.5 + ((*run_inf).time_correction - (l * 60)));
      if (GUI)
      {
         time_correction_lbl2->setText(str.sprintf("%d min %d sec", l, m));
      }
   }
   (*run_inf).duration = (*run_inf).time[last_cell][last_wavelength][last_scan];
   l = (int) ((*run_inf).time[last_cell][last_wavelength][last_scan] / 3600);
   m = (int) (0.5 + ((*run_inf).time[last_cell][last_wavelength][last_scan] - l * 3600) / 60);
   if (GUI)
   {
      run_length_lbl2->setText(str.sprintf("%d h %d min", l, m));
   }
}

void RunDetails_F::animate()
{

   blink = -blink;
   if (blink > 0)
   {
      lcd2->setPaletteForegroundColor (Qt::red);
      lcd2->setPaletteBackgroundColor (Qt::red);
   }
   else
   {
      lcd2->setPalette( QPalette(USglobal->global_colors.cg_dk_red, USglobal->global_colors.cg_dk_red, USglobal->global_colors.cg_dk_red) );
   }
}
