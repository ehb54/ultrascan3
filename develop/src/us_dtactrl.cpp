#include "../include/us_dtactrl.h"

#ifndef WIN32
#include <unistd.h>
#endif

Data_Control_W::Data_Control_W(const int temp_run_type, QWidget *p, const char *name)
   : QFrame( p, name)
   //: QFrame( p, name, WDestructiveClose)
{
   // run_type: 0=no run, just use member functions (ushydro, uswin)
   //           1=velocity
   //           2=equilibrium
   //           3=export velocity data
   //           4=export equilibrium data
   //           5=print velocity data
   //           6=print equilibrium data
   //           7=generate report for velocity data
   //           8=generate report for equilibrium data
   //           9=change cell ID's for velocity data
   //           10=change cell ID's for equilibrium data
   //           11=copy velocity data (change run_id)
   //           12=copy equilibrium data (change run_id)
   //             13=velocity run without displaying the vbar value (finite element)
   //             14=velocity run with one extra field
   //             17=velocity run with four extra fields (sa2d)
   //             19=velocity run with two less fields (secdat)
   //             21=velocity run with two less fields (sa2d)
   //             101=velocity run without GUI (terminal based Monte Carlo/fe_nnls)
   //           31=copy velocity data to a new name and subtract time- and radially invariant noise
   //           32=update meniscus of velocity experiment
   baseline_flag = false;
   run_type = temp_run_type;
   run_inf.temperature = NULL;
   run_inf.time = NULL;
   run_inf.omega_s_t = NULL;
   run_inf.plateau = NULL;
   run_inf.rpm = NULL;
   fn = "";
   if (run_type == 0 || run_type == 101)
   {
      vbarDisplay = false;
      viscosityDisplay = false;
      densityDisplay = false;
   }
   else
   {
      vbarDisplay = true;
      viscosityDisplay = true;
      densityDisplay = true;
   }
   extraCounter = 0;
   if (run_type == 13)
   {
      vbarDisplay = false; // this is for finite element windows where we don't want the vbar text field
      // filled out from the database since it goes into the model window
      run_type = 1;         // just set it to a normal velocity run.
   }
   if (run_type == 14)
   {
      extraCounter = 1;   // this is for us_vhwenhanced where we need an additional field to
      // control the diffusion coefficient sensititvity.
      run_type = 1;            // just set it to a normal velocity run.
   }
   if (run_type == 16)
   {
      extraCounter = 3;   // this is for us_cofs where we need 3 additional fields to
      // control the s grid and a fit field for the f/f0 ratio.
      run_type = 1;            // just set it to a normal velocity run.
   }
   if (run_type == 17)
   {
      extraCounter = 4;   // this is for us_sa2d where we need 4 additional fields to
      // control the s and f/f0 grid.
      run_type = 1;            // just set it to a normal velocity run.
   }
   if (run_type == 18)
   {
      extraCounter = 5;   // this is for us_sa2d where we need 5 additional fields to
      // control the s and f/f0 grid.
      run_type = 1;            // just set it to a normal velocity run.
   }
   if (run_type == 19)
   {
      extraCounter = -2;   // this is for us_secdat
      run_type = 1;            // just set it to a normal velocity run.
   }
   if (run_type == 21)
   {
      extraCounter = -1;   // this is for us_sa2d
      run_type = 1;            // just set it to a normal velocity run.
   }
   GUI = true;   // by default, use all GUI functions
   if (run_type >= 2)
   {
      viscosityDisplay = false;
   }
   if (run_type >= 3)
   {
      densityDisplay = false;
   }
   if (run_type >= 3)
   {
      vbarDisplay = false;
   }
   if (run_type == 0)
   {
      GUI = false;
   }
   if (run_type == 101)
   {
      GUI = false;   // this is a velocity run, but we don't need the GUI (terminal-based Monte Carlo)
      run_type = 1;
   }
   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   xpos = 2;
   ypos = 5;
   buttonw = 172;   //make divisible by 3 and by 2
   buttonh = 26;
   border = 2;
   reset_flag = false;
   exclude_flag = false;
   smooth = 1;
   step = 0;
   start = true;
   already = false;
   vbar_from_seq = false;
   buffer_from_seq = false;
   dont_plot = false;
   vbar20 = (float) 0.72;
   scan_loaded = false;
   run_loaded = false;
   scan_copied = false;
   print_bw = false;
   print_inv = true;
   buffer_from_db = false;
   vbar_from_db = false;

   bd_range=100;
   bd_position = 0;
   for (i=0; i<8; i++)   // initialize all static run_inf struct members:
   {
      run_inf.centerpiece[i] = -2;
      run_inf.cell_id[i] = "";
      run_inf.wavelength_count[i] = 0;
      run_inf.meniscus[i] = 0.0;
      for (j=0; j<4; j++)
      {
         run_inf.buffer_serialnumber[i][j] = -1;
         for(int k=0; k<3; k++)
         {
            run_inf.peptide_serialnumber[i][j][k] = -1;
            run_inf.DNA_serialnumber[i][j][k] = -1;
         }
      }
      for (j=0; j<3; j++)
      {
         run_inf.wavelength[i][j] = 0;
         run_inf.scans[i][j] = 0;
         run_inf.baseline[i][j] = 0;
         for (k=0; k<4; k++)
         {
            run_inf.range_left[i][j][k] = 0.0;
            run_inf.range_right[i][j][k] = 0.0;
            run_inf.points[i][j][k] = 0;
            run_inf.point_density[i][j][k] = 0.0;
         }
      }
   }

   for(int i=0; i<8; i++)
   {
      for(int j=0; j<4; j++)
      {
         Density  [i][j] = (float) DENS_20W;
         Viscosity[i][j] = (float) (100.0 * VISC_20W);

         for(int k=0; k<3; k++)
         {
            Vbar  [i][j][k] = (float) 0.72;
            Vbar20[i][j][k] = (float) 0.72;
         }
      }
   }
   if (GUI)
   {
      create_GUI();
   }
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
}

Data_Control_W::~Data_Control_W()
{
   if (scan_loaded)
   {
      cleanup_loaded_scan();
   }
   if (scan_copied)
   {
      cleanup_copied_scan();
   }
}

void Data_Control_W::create_GUI()
{
   QString str;
   pb_load = new QPushButton(tr("Load Data"), this);
   Q_CHECK_PTR(pb_load);
   pb_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_load->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   //   pb_load->setGeometry(xpos, ypos, buttonw, buttonh);
   pb_load->setAutoDefault(false);
   connect(pb_load, SIGNAL(clicked()), SLOT(load_data()));

   xpos += buttonw + spacing;

   pb_details = new QPushButton(tr("Run Details"), this);
   Q_CHECK_PTR(pb_details);
   pb_details->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_details->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   //   pb_details->setGeometry(xpos, ypos, buttonw, buttonh);
   pb_details->setAutoDefault(false);
   pb_details->setEnabled(false);
   connect(pb_details, SIGNAL(clicked()), SLOT(details()));

   xpos = border;
   ypos += buttonh + spacing;

   if (run_type == 1 || run_type == 2)
   {
      pb_second_plot = new QPushButton("", this);
      Q_CHECK_PTR(pb_second_plot);
      pb_second_plot->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      pb_second_plot->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
      //      pb_second_plot->setGeometry(xpos, ypos, buttonw, buttonh);
      pb_second_plot->setAutoDefault(false);
      pb_second_plot->setEnabled(false);
      connect(pb_second_plot, SIGNAL(clicked()), SLOT(second_plot()));

      xpos += buttonw + spacing;

      pb_save = new QPushButton(tr("Save Data"), this);
      Q_CHECK_PTR(pb_save);
      pb_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      pb_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
      //      pb_save->setGeometry(xpos, ypos, buttonw, buttonh);
      pb_save->setAutoDefault(false);
      pb_save->setEnabled(false);
      connect(pb_save, SIGNAL(clicked()), SLOT(save()));

      xpos = border;
      ypos += buttonh + spacing;
   }
   pb_print = new QPushButton("", this);
   Q_CHECK_PTR(pb_print);
   pb_print->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_print->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_print->setAutoDefault(false);
   pb_print->setEnabled(false);
   //   pb_print->setGeometry(xpos, ypos, buttonw, buttonh);
   switch (run_type)
   {
   case 1:   // Velocity
      {
         pb_print->setText(tr("Print Data"));
         connect(pb_print, SIGNAL(clicked()), SLOT(print()));
         break;
      }
   case 2:   // Equilibrium
      {
         pb_print->setText(tr("Print Data"));
         connect(pb_print, SIGNAL(clicked()), SLOT(print()));
         break;
      }
   case 3:   // Export Velocity
      {
         pb_print->setText(tr("Export Data"));
         connect(pb_print, SIGNAL(clicked()), SLOT(save()));
         break;
      }
   case 4:   // Export Equilibrium
      {
         pb_print->setText(tr("Export Data"));
         connect(pb_print, SIGNAL(clicked()), SLOT(save()));
         break;
      }
   case 5:   // Print Velocity
      {
         pb_print->setText(tr("Print Data"));
         connect(pb_print, SIGNAL(clicked()), SLOT(print()));
         break;
      }
   case 6:   // Print Equilibrium
      {
         pb_print->setText(tr("Print Data"));
         connect(pb_print, SIGNAL(clicked()), SLOT(print()));
         break;
      }
   }

   if (run_type == 1 || run_type == 2)
   {
      xpos += buttonw + spacing;

      pb_view = new QPushButton(tr("View Data Report"), this);
      Q_CHECK_PTR(pb_view);
      pb_view->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      pb_view->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
      //      pb_view->setGeometry(xpos, ypos, buttonw, buttonh);
      pb_view->setAutoDefault(false);
      pb_view->setEnabled(false);
      connect(pb_view, SIGNAL(clicked()), SLOT(view()));
   }
   if ((run_type == 3 || run_type == 4 || run_type == 5 || run_type == 6) && extraCounter >= 0)
   {
      xpos += buttonw + spacing;

      pb_reset = new QPushButton(tr("Reset Data"), this);
      Q_CHECK_PTR(pb_reset);
      pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      pb_reset->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
      //      pb_reset->setGeometry(xpos, ypos, buttonw, buttonh);
      pb_reset->setAutoDefault(false);
      pb_reset->setEnabled(false);
      connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));
   }
   xpos = border;
   ypos += buttonh + spacing;
   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   //   pb_help->setGeometry(xpos, ypos, buttonw, buttonh);
   pb_help->setAutoDefault(false);
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   xpos += buttonw + spacing;

   pb_close = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_close);
   pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   //   pb_close->setGeometry(xpos, ypos, buttonw, buttonh);
   pb_close->setAutoDefault(false);
   connect(pb_close, SIGNAL(clicked()), SLOT(quit()));

   xpos = border;
   ypos += buttonh + spacing + 2;

   banner1 = new QLabel(tr("Information for this Run:"),this);
   banner1->setFrameStyle(QFrame::WinPanel|Raised);
   banner1->setAlignment(AlignCenter|AlignVCenter);
   banner1->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
   //   banner1->setGeometry(xpos-border, ypos, buttonw*2+spacing + 2 * border, buttonh);
   banner1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   ypos += buttonh + spacing + 2;

   lbl_run_id1 = new QLabel(tr(" Run Id:"),this);
   lbl_run_id1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   //   lbl_run_id1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_run_id1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos += buttonw + spacing;

   lbl_run_id2 = new QLabel(tr(" not selected"),this);
   lbl_run_id2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_run_id2->setAlignment(AlignCenter|AlignVCenter);
   lbl_run_id2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   //   lbl_run_id2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_run_id2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   xpos = border;
   ypos += buttonh + spacing;

   lbl_temperature1 = new QLabel(tr(" Temperature:"),this);
   lbl_temperature1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   //   lbl_temperature1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_temperature1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos += buttonw + spacing;

   lbl_temperature2 = new QLabel(tr(" not selected"),this);
   lbl_temperature2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_temperature2->setAlignment(AlignCenter|AlignVCenter);
   lbl_temperature2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   //   lbl_temperature2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_temperature2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   xpos = border;
   ypos += buttonh + spacing;

   lbl_cell_info1 = new QLabel(tr(" Available Cells:"),this);
   lbl_cell_info1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   //   lbl_cell_info1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_cell_info1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos += buttonw + spacing;

   lbl_cell_info2 = new QLabel(tr(" not selected"),this);
   lbl_cell_info2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_cell_info2->setAlignment(AlignCenter|AlignVCenter);
   lbl_cell_info2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   //   lbl_cell_info2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_cell_info2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   xpos = border;
   ypos += buttonh + spacing +3;

   /*   lbl_cell_descr = new QLabel(tr(" no data available"),this);
        lbl_cell_descr->setFrameStyle(QFrame::WinPanel|Sunken);
        lbl_cell_descr->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
        lbl_cell_descr->setAlignment(AlignCenter|AlignVCenter);
        lbl_cell_descr->setGeometry(xpos, ypos, 2 * buttonw + spacing, buttonh);
        lbl_cell_descr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   */
   lbl_cell_descr = new QTextEdit(this);
   lbl_cell_descr->setMaximumHeight(buttonh*2);
   lbl_cell_descr->setWordWrap(QTextEdit::WidgetWidth);
   lbl_cell_descr->setText("no data available");
   lbl_cell_descr->setReadOnly(true);
   lbl_cell_descr->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_cell_descr->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lbl_cell_descr->setAlignment(AlignCenter|AlignVCenter);
   //   lbl_cell_descr->setGeometry(xpos, ypos, 2 * buttonw + spacing, buttonh);
   lbl_cell_descr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   xpos = border;
   ypos += buttonh;

   selected_cell = 0;
   cell_select = new QListBox(this, "Cell");
   if (run_type == 1 || run_type == 3 || run_type == 5)
   {
      //      cell_select->setGeometry(xpos, ypos, buttonw, 55);
   }
   if (run_type == 2 || run_type == 4 || run_type == 6) // Equilibrium
   {
      //      cell_select->setGeometry(xpos, ypos, ((2*buttonw)/3)-1 , 55);
   }
   cell_select->insertItem(tr("Cell 1"));
   cell_select->insertItem(tr("Cell 2"));
   cell_select->insertItem(tr("Cell 3"));
   cell_select->insertItem(tr("Cell 4"));
   cell_select->insertItem(tr("Cell 5"));
   cell_select->insertItem(tr("Cell 6"));
   cell_select->insertItem(tr("Cell 7"));
   cell_select->insertItem(tr("Cell 8"));
   cell_select->setSelected(0, true);
   cell_select->setMaximumHeight(buttonh*8);
   cell_select->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cell_select->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   connect(cell_select, SIGNAL(highlighted(int)), SLOT(show_cell(int)));

   selected_lambda = 0;
   lambda_select = new QListBox(this, "Wavelength");
   if (run_type == 1 || run_type == 3 || run_type == 5)
   {
      xpos += buttonw + spacing;
      //      lambda_select->setGeometry(xpos, ypos, buttonw, 55);
   }
   if (run_type == 2 || run_type == 4 || run_type == 6)
   {
      xpos += ((2*buttonw)/3) - 1 + spacing;
      //      lambda_select->setGeometry(xpos, ypos, ((2*buttonw)/3), 55);
   }
   lambda_select->insertItem(tr("Wavelength 1"));
   lambda_select->insertItem(tr("Wavelength 2"));
   lambda_select->insertItem(tr("Wavelength 3"));
   lambda_select->setSelected(0, true);
   lambda_select->setMaximumHeight(buttonh*8);
   lambda_select->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lambda_select->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   connect(lambda_select, SIGNAL(highlighted(int)), SLOT(show_lambda(int)));

   if (run_type == 2 || run_type == 4 || run_type == 6)
   {
      selected_channel = 0;
      xpos += ((2*buttonw)/3) + spacing;
      channel_select = new QListBox(this, "Channel");
      //      channel_select->setGeometry(xpos, ypos, ((2*buttonw)/3)-1, 55);
      channel_select->insertItem(tr("Channel 1"));
      channel_select->insertItem(tr("Channel 2"));
      channel_select->insertItem(tr("Channel 3"));
      channel_select->setSelected(0, true);
      channel_select->setMaximumHeight(buttonh*8);
      channel_select->setEnabled(false);  // default setting is disabled (1 channel only), if more than one channel, enabled below
      channel_select->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      channel_select->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   }

   xpos = border;
   ypos += 55 + spacing + 2;
   if (run_type == 1 || run_type == 2)
   {
      banner2 = new QLabel(tr("Experimental Parameters (at 20ºC):"), this);
      banner2->setFrameStyle(QFrame::WinPanel|Raised);
      banner2->setAlignment(AlignCenter|AlignVCenter);
      banner2->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
      //      banner2->setGeometry(xpos-border, ypos, buttonw*2+spacing + 2 * border, buttonh);
      banner2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

      ypos += buttonh + spacing + 2;

      density = (float) DENS_20W;
      pb_density = new QPushButton(tr(" Density:"), this);
      Q_CHECK_PTR(pb_density);
      pb_density->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      pb_density->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
      pb_density->setAutoDefault(false);
      //      pb_density->setGeometry(xpos, ypos, buttonw/2-spacing/2+20, 26);
      connect(pb_density, SIGNAL(clicked()), SLOT(get_buffer()));

      xpos += buttonw/2+spacing/2;

      density_le = new QLineEdit(this);
      //      density_le->setGeometry(xpos+20, ypos, buttonw/2-spacing/2-20, buttonh);
      density_le->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
      density_le->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      density_le->setText(str.sprintf("%8.6f", DENS_20W));
      connect(density_le, SIGNAL(returnPressed()), SLOT(update_density()));

      xpos += buttonw/2+spacing/2;
   }

   if (run_type == 1)
   {
      viscosity = (float) (100.0 * VISC_20W);
      pb_viscosity = new QPushButton(tr(" Viscosity:"), this);
      Q_CHECK_PTR(pb_viscosity);
      pb_viscosity->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      pb_viscosity->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
      pb_viscosity->setAutoDefault(false);
      connect(pb_viscosity, SIGNAL(clicked()), SLOT(get_buffer()));

      xpos += buttonw/2+spacing/2;

      viscosity_le = new QLineEdit(this);
      viscosity_le->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
      viscosity_le->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      viscosity_le->setText(str.sprintf("%8.6f", viscosity));
      connect(viscosity_le, SIGNAL(returnPressed()), SLOT(update_viscosity()));

      xpos = border;
      ypos += buttonh + spacing;
   }

   if (run_type == 1 || run_type == 2)
   {
      pb_vbar = new QPushButton(tr(" vbar:"), this);
      Q_CHECK_PTR(pb_vbar);
      pb_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      pb_vbar->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
      pb_vbar->setAutoDefault(false);
      connect(pb_vbar, SIGNAL(clicked()), SLOT(read_vbar()));

      xpos += buttonw/2+spacing/2;

      vbar_le = new QLineEdit(this);
      vbar_le->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
      vbar_le->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      if (vbarDisplay)
      {
         vbar_le->setText(" .7200");
      }
      connect(vbar_le, SIGNAL(returnPressed()), SLOT(update_vbar()));
   }
   if (run_type == 1)
   {
      xpos += buttonw/2+spacing/2;
      lbl1_excluded = new QLabel(tr(" Skipped:"),this);
      lbl1_excluded->setFrameStyle(QFrame::WinPanel|Sunken);
      lbl1_excluded->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
      //      lbl1_excluded->setGeometry(xpos, ypos, buttonw/2-spacing/2+20, buttonh);
      lbl1_excluded->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

      xpos += buttonw/2+spacing/2;

      lbl2_excluded = new QLabel("0", this);
      lbl2_excluded->setFrameStyle(QFrame::WinPanel|Sunken);
      //      lbl2_excluded->setGeometry(xpos+20, ypos, buttonw/2-spacing/2-20,  buttonh);
      lbl2_excluded->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
      lbl2_excluded->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
      lbl2_excluded->setAlignment(AlignCenter|AlignVCenter);
   }
   if ((run_type == 1 || run_type == 2) && extraCounter >= 0)
   {
      ypos += buttonh + spacing + 3;
      xpos = border;

      pb_reset = new QPushButton(tr("Reset Data"), this);
      Q_CHECK_PTR(pb_reset);
      pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      pb_reset->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
      //      pb_reset->setGeometry(xpos, ypos, buttonw, buttonh);
      pb_reset->setAutoDefault(false);
      pb_reset->setEnabled(false);
      connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));

      ypos += buttonh + spacing ;

      // divisions and other analysis specific controls go here


      xpos = border;

      switch (run_type)
      {
      case 1:
         {
            ypos += buttonh + spacing;
            break;
         }
      case 2:
         {
            ypos += 2 * (buttonh + spacing);
            break;
         }
      }
   }
   if (extraCounter < 0)
   {
      ypos += buttonh + spacing;
      xpos = border;
   }
   if (extraCounter >= 0)
   {
      ypos += extraCounter * (buttonh + spacing); // various methods need space for extra counters
   }
   if (extraCounter != -1)
   {
      smoothing_lbl = new QLabel(tr(" Data Smoothing:"), this);
      Q_CHECK_PTR(smoothing_lbl);
      smoothing_lbl->setAlignment(AlignLeft|AlignVCenter);
      smoothing_lbl->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
      smoothing_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

      xpos += buttonw + spacing;

      smoothing_counter= new QwtCounter(this);
      Q_CHECK_PTR(smoothing_counter);
      smoothing_counter->setRange(1, 50, 1);
      smoothing_counter->setNumButtons(2);
      smoothing_counter->setValue(1);
      smoothing_counter->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      smoothing_counter->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      connect(smoothing_counter, SIGNAL(valueChanged(double)), SLOT(update_smoothing(double)));

      xpos = border;
      ypos += buttonh + spacing;

      range_lbl = new QLabel(tr(" % of Boundary:"), this);
      Q_CHECK_PTR(range_lbl);
      range_lbl->setAlignment(AlignLeft|AlignVCenter);
      range_lbl->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
      range_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

      xpos += buttonw + spacing;

      range_counter= new QwtCounter(this);
      Q_CHECK_PTR(range_counter);
      range_counter->setRange(10, 100, 0.1);
      range_counter->setValue(bd_range);
      range_counter->setNumButtons(3);
      range_counter->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      range_counter->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      connect(range_counter, SIGNAL(valueChanged(double)), SLOT(update_boundary_range(double)));

      xpos = border;
      ypos += buttonh + spacing;

      position_lbl = new QLabel(tr(" Boundary Pos. (%):"), this);
      Q_CHECK_PTR(position_lbl);
      position_lbl->setAlignment(AlignLeft|AlignVCenter);
      position_lbl->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
      position_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

      xpos += buttonw + spacing;

      position_counter= new QwtCounter(this);
      Q_CHECK_PTR(position_counter);
      position_counter->setRange(0, 100-bd_position, 0.1);
      position_counter->setNumButtons(3);
      position_counter->setValue(bd_position);
      position_counter->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      position_counter->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      connect(position_counter, SIGNAL(valueChanged(double)), SLOT(update_boundary_position(double)));

      ypos += buttonh + spacing;
   }
   xpos = border;
   if (extraCounter == -1)
   {
      ypos += buttonh * 5 + 2 * spacing;
   }

   pb_exsingle = new QPushButton(tr("Excl. Single Scan"), this);
   Q_CHECK_PTR(pb_exsingle);
   pb_exsingle->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_exsingle->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   //   pb_exsingle->setGeometry(xpos, ypos, buttonw, buttonh);
   pb_exsingle->setAutoDefault(false);
   pb_exsingle->setEnabled(false);
   connect(pb_exsingle, SIGNAL(clicked()), SLOT(ex_single()));

   xpos += buttonw + spacing;

   exclude_single=1;
   cnt_exsingle = new QwtCounter(this);
   Q_CHECK_PTR(cnt_exsingle);
   cnt_exsingle->setRange(1, 100, 1);
   cnt_exsingle->setNumButtons(3);
   cnt_exsingle->setValue(1);
   cnt_exsingle->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cnt_exsingle->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(cnt_exsingle, SIGNAL(valueChanged(double)), SLOT(update_single(double)));


   xpos = border;
   ypos += buttonh + spacing;

   pb_exrange = new QPushButton(tr("Excl. Scan Range"), this);
   Q_CHECK_PTR(pb_exrange);
   pb_exrange->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_exrange->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_exrange->setAutoDefault(false);
   pb_exrange->setEnabled(false);
   connect(pb_exrange, SIGNAL(clicked()), SLOT(ex_range()));

   xpos += buttonw + spacing;

   exclude_range=1;
   cnt_exrange = new QwtCounter(this);
   cnt_exrange->setNumButtons(3);
   cnt_exrange->setRange(1, 100, 1);
   cnt_exrange->setValue(1);
   cnt_exrange->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cnt_exrange->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(cnt_exrange, SIGNAL(valueChanged(double)), SLOT(update_range(double)));

   if (run_type == 1 || run_type == 2)
   {
      analysis_plot = new QwtPlot(this);
      Q_CHECK_PTR(analysis_plot);
      analysis_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
      analysis_plot->enableGridXMin();
      analysis_plot->enableGridYMin();
      analysis_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
      analysis_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
      analysis_plot->setCanvasBackground(USglobal->global_colors.plot);      //new version
      analysis_plot->enableOutline(true);
      analysis_plot->setAxisTitle(QwtPlot::xBottom, " ");
      analysis_plot->setAxisTitle(QwtPlot::yLeft, " ");
      analysis_plot->setTitle(tr("Experimental Data"));
      analysis_plot->setOutlinePen(white);
      analysis_plot->setOutlineStyle(Qwt::Cross);
      analysis_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
      analysis_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
      analysis_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
      analysis_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
      analysis_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
      analysis_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
      analysis_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
      analysis_plot->setMargin(USglobal->config_list.margin);
      analysis_plot->setMinimumSize(550, 350);

      connect(analysis_plot, SIGNAL(plotMouseReleased(const QMouseEvent &)),
              SLOT(getAnalysisPlotMouseReleased(const QMouseEvent &)));
      connect(analysis_plot, SIGNAL(plotMousePressed(const QMouseEvent &)),
              SLOT(getAnalysisPlotMousePressed(const QMouseEvent &)));
   }

   edit_plot = new QwtPlot(this);
   edit_plot->enableOutline(true);
   edit_plot->setOutlinePen(white);
   edit_plot->setOutlineStyle(Qwt::VLine);
   edit_plot->enableGridXMin();
   edit_plot->enableGridYMin();
   edit_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
   edit_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   edit_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
   edit_plot->setCanvasBackground(USglobal->global_colors.plot);      //new version
   edit_plot->setAxisTitle(QwtPlot::xBottom, tr("Radius (in cm)"));
   edit_plot->setAxisTitle(QwtPlot::yLeft, tr("Absorbance"));
   edit_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   edit_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   edit_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   edit_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   edit_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   edit_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   edit_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   edit_plot->setMargin(USglobal->config_list.margin);
   edit_plot->setMinimumSize(550, 300);

   if (run_type == 1 || run_type == 3 || run_type == 5)
   {
      edit_plot->setTitle(tr("Velocity Data"));
   }
   if (run_type == 2 || run_type == 4 || run_type == 6)
   {
      edit_plot->setTitle(tr("Equilibrium Data"));
   }

   xpos = border;
   ypos += buttonh + spacing;

   if (run_type == 1 || run_type == 2)
   {
      ypos += 3;
      lbl_status = new QLabel(tr("  Status:"),this);
      lbl_status->setAlignment(AlignLeft|AlignVCenter);
      lbl_status->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
      //      lbl_status->setGeometry(xpos, ypos, 65, buttonh);
      lbl_status->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

      progress = new QProgressBar(this, "Loading Progress");
      //      progress->setGeometry(xpos+65, ypos, 2*buttonw+spacing-65, 26);
      progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      ypos += buttonh + spacing;
   }
   if (run_type == 1 || run_type == 2 || run_type == 3 || run_type == 4 || run_type == 5 || run_type == 6)
   {
      global_Xpos += 30;
      global_Ypos += 30;
      //      setMinimumSize(860,ypos+5);
      //      setGeometry(global_Xpos, global_Ypos, 860, ypos);
   }

}

void Data_Control_W::closeEvent(QCloseEvent *e)
{
   qApp->processEvents();      // take care of unfinished business before shutting down
   if (scan_loaded)
   {
      cleanup_loaded_scan();
   }
   if (scan_copied)
   {
      cleanup_copied_scan();
   }
   if (run_loaded)
   {
      cleanup_loaded_run();
   }
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void Data_Control_W::show_cell(int val)
{
   reset();
   selected_cell = val;
   density = hydro_inf.Density[selected_cell][selected_channel];
   viscosity = hydro_inf.Viscosity[selected_cell][selected_channel];
   vbar = hydro_inf.Vbar[selected_cell][selected_channel][0];
   vbar20 = hydro_inf.Vbar20[selected_cell][selected_channel][0];
   calc_correction(run_inf.avg_temperature);

   if (scan_copied)
   {
      cleanup_copied_scan();
   }
   if (step == 0)
   {
      QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n      Click on \"Load Data\"\n"));
      return;
   }
   if (run_type == 2 || run_type == 4 || run_type == 6)
   {
      channel_select->setSelected(0, true);
      channel_select->setEnabled(false); //set selection widget disabled by default, only enable below
      disconnect(channel_select, SIGNAL(highlighted(int)), 0, 0);

      if (cp_list[run_inf.centerpiece[selected_cell]].channels > 1) //enable the channel selection dialog
      {
         channel_select->setEnabled(true);
         connect(channel_select, SIGNAL(highlighted(int)), SLOT(show_channel(int)));
      }
   }

   if(run_inf.cell_id[val] != "" && run_inf.wavelength[selected_cell][selected_lambda] != 0)
   {
      lbl_cell_descr->setText(run_inf.cell_id[val]);
      if (load_scan() < 0)
      {
         return;
      }
      updateLabels();
   }
   else
   {
      lbl_cell_descr->setText(tr(" no data available"));
      edit_plot->clear();
      edit_plot->replot();
      if (run_type == 1 || run_type == 2)      // we dont want to plot analysis for run_types 3-6
      {
         analysis_plot->clear();
         analysis_plot->replot();
      }
   }
   emit datasetChanged();
   if (GUI)
   {
      plot_analysis();
      plot_edit();
   }
}

void Data_Control_W::show_lambda(int val)
{
   reset();
   if (step == 0)
   {
      QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n      Click on \"Load Data\"\n"));
      return;
   }
   if (scan_copied)
   {
      cleanup_copied_scan();
   }

   selected_lambda = val;
   if(run_inf.cell_id[val] != "" && run_inf.wavelength[selected_cell][selected_lambda] != 0)
   {
      lbl_cell_descr->setText(run_inf.cell_id[selected_cell]);
      if (load_scan() < 0)
      {
         return;
      }
      updateLabels();
      plot_edit();
      if (step != 0)
      {
         // cout << "Step 2\n";
         plot_analysis();
      }
   }
   else
   {
      lbl_cell_descr->setText(tr(" no data available"));
      edit_plot->clear();
      edit_plot->replot();
      //edit_plot->updatePlot();      //no updatePlot() function in new version
      if (run_type == 1 || run_type == 2)      // we dont want to plot analysis for run_types 3-6
      {
         analysis_plot->clear();
         analysis_plot->replot();
      }
   }
   emit datasetChanged();
   if (GUI)
   {
      plot_analysis();
      plot_edit();
   }
}

void Data_Control_W::show_channel(int val)
{
   reset();
   selected_channel = val;
   density = hydro_inf.Density[selected_cell][selected_channel];
   viscosity = hydro_inf.Viscosity[selected_cell][selected_channel];
   vbar = hydro_inf.Vbar[selected_cell][selected_channel][0];
   vbar20 = hydro_inf.Vbar20[selected_cell][selected_channel][0];
   calc_correction(run_inf.avg_temperature);

   if (scan_copied)
   {
      cleanup_copied_scan();
   }

   if (step == 0)
   {
      QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n      Click on \"Load Data\"\n"));
      return;
   }
   //   run_inf.centerpiece[selected_cell] contains the serial number of the centerpiece, which is also the array element of the list
   if (cp_list[run_inf.centerpiece[selected_cell]].channels > 1 && run_inf.wavelength[selected_cell][selected_lambda] != 0)
   {
      lbl_cell_descr->setText(run_inf.cell_id[selected_cell]);
      if (load_scan() < 0)
      {
         return;
      }
      updateLabels();
      plot_edit();
      // cout << "Step 3\n";

      plot_analysis();
   }
   else
   {
      lbl_cell_descr->setText(tr(" no data available"));
      edit_plot->clear();
      edit_plot->replot();
      //edit_plot->updatePlot();      //no updatePlot() function in new version
      if (run_type == 1 || run_type == 2)      // we dont want to plot analysis for run_types 3-6
      {
         analysis_plot->clear();
         analysis_plot->replot();
      }
   }
   emit datasetChanged();
   if (GUI)
   {
      plot_analysis();
      plot_edit();
   }
}

void Data_Control_W::updateLabels()
{
   if (GUI)
   {
      QString str;
      str.sprintf("%8.6f", density);
      if (densityDisplay)
      {
         //         density_le->disconnect();
         density_le->setText(str);
         //         connect(density_le, SIGNAL(returnPressed()), SLOT(update_density()));
      }
      str.sprintf("%8.6f", viscosity);
      if (viscosityDisplay)
      {
         //         viscosity_le->disconnect();
         viscosity_le->setText(str);
         //         connect(viscosity_le, SIGNAL(returnPressed()), SLOT(update_viscosity()));
      }
      //cout << vbar20 << endl;
      str.sprintf("%6.4f", vbar20);
      if (vbarDisplay)
      {
         //         vbar_le->disconnect();
         vbar_le->setText(str);
         qApp->processEvents();
         //         connect(vbar_le, SIGNAL(returnPressed()), SLOT(update_vbar()));
      }
   }
}

bool Data_Control_W::createHtmlDir()
{
   htmlDir = USglobal->config_list.html_dir + "/" + run_inf.run_id;
   QDir d(htmlDir);
   if (d.exists())
   {
      return true;
   }
   else
   {
      if (d.mkdir(htmlDir, true))
      {
         return true;
      }
      else
      {
         return false;
      }
   }
}

void Data_Control_W::newMessage(QString str, int i)
{
   QString str1;
   if (i < 0)
   {
      str1.sprintf(tr("Fatal Error in UltraScan:\n\n" + str
                      + "The module exited with return code: %d\n\nPlease start over."), i);
      QMessageBox::warning(this, tr("UltraScan Warning"), str1,
                           QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
   }
   else
   {
      str1.sprintf(tr("Non-fatal Error in UltraScan:\n\n" + str
                      + "The module exited with return code: %d\n\nPlease start over."), i);
      QMessageBox::warning(this, tr("UltraScan Warning"), str1,
                           QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
   }
}

void Data_Control_W::newMessage(QString str)
{
   QMessageBox::warning(this, tr("UltraScan Warning"),
                        tr("Error in UltraScan:\n\n" + str +"\n\nPlease start over."),
                        QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
}

int Data_Control_W::load_data()
{
   step = 0;
   if (GUI)
   {
      if (run_type == 2 || run_type == 4 || run_type == 6 || run_type == 8 || run_type == 10 || run_type == 12)
      {
         fn = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*.us.e", 0);
      }
      else
      {
         fn = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*.us.v", 0);
      }
   }
   return(load_data(fn));
}

//some functions don't need to know hydrodynamic info and can load a local
//file without requirement for database functions, and can pass db_flag=false

int Data_Control_W::load_data(bool db_flag)
{
   step = 0;
   if (GUI)
   {
      if (run_type == 2 || run_type == 4 || run_type == 6 || run_type == 8 || run_type == 10 || run_type == 12)
      {
         fn = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*.us.e", 0);
      }
      else
      {
         fn = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*.us.v", 0);
      }
   }
   return(load_data(fn, db_flag));
}

int Data_Control_W::load_data(const QString &fileName)
{
   US_Data_IO *data_io;
   data_io = new US_Data_IO(&run_inf, baseline_flag);
   connect(data_io, SIGNAL(newMessage(QString, int)), this, SLOT(newMessage(QString, int)));
   QString version, str;
   unsigned int i, j, k;
   int result;
   // first clean up before loading a new data set if one has already been loaded
   plateaus_corrected = false;
   rad_corrected = false;
   if (scan_loaded)
   {
      cleanup_loaded_scan();
   }
   if (scan_copied)
   {
      cleanup_copied_scan();
   }
   if (run_loaded)
   {
      cleanup_loaded_run();
   }
   start = true;
   if (run_type == 1 || run_type == 3 || run_type == 5 || run_type == 7 || run_type == 9 || run_type == 11 || run_type == 31 || run_type == 32)
   {
      if ( !fileName.isEmpty() )
      {
         result = data_io->load_run(fileName, run_type, has_data, &cp_list);
         temp_run.temperature.clear();
         temp_run.rpm.clear();
         temp_run.time.clear();
         temp_run.omega_s_t.clear();
         temp_run.plateau.clear();
         temp_run.temperature.resize(8);
         temp_run.rpm.resize(8);
         temp_run.time.resize(8);
         temp_run.omega_s_t.resize(8);
         temp_run.plateau.resize(8);
         for (i=0; i<8; i++)
         {
            temp_run.temperature[i].resize(run_inf.wavelength_count[i]);
            temp_run.rpm[i].resize(run_inf.wavelength_count[i]);
            temp_run.time[i].resize(run_inf.wavelength_count[i]);
            temp_run.omega_s_t[i].resize(run_inf.wavelength_count[i]);
            temp_run.plateau[i].resize(run_inf.wavelength_count[i]);
            for (j=0; j<run_inf.wavelength_count[i]; j++)
            {
               temp_run.temperature[i][j].resize(run_inf.scans[i][j]);
               temp_run.rpm[i][j].resize(run_inf.scans[i][j]);
               temp_run.time[i][j].resize(run_inf.scans[i][j]);
               temp_run.omega_s_t[i][j].resize(run_inf.scans[i][j]);
               temp_run.plateau[i][j].resize(run_inf.scans[i][j]);
            }
         }
         for (i=0; i<8; i++)
         {
            for (j=0; j<run_inf.wavelength_count[i]; j++)
            {
               for (k=0; k<run_inf.scans[i][j]; k++)
               {
                  temp_run.temperature[i][j][k] = run_inf.temperature[i][j][k];
                  temp_run.rpm[i][j][k] = run_inf.rpm[i][j][k];
                  temp_run.time[i][j][k] = run_inf.time[i][j][k];
                  temp_run.omega_s_t[i][j][k] = run_inf.omega_s_t[i][j][k];
                  temp_run.plateau[i][j][k] = run_inf.plateau[i][j][k];
               }
            }
         }
         for (i=0; i<8; i++)
         {
            for (j=0; j<3; j++)
            {
               temp_run.scans[i][j] = run_inf.scans[i][j];
            }
         }

         //cout << "Loading data RETURNED result code: " << result << endl;
         if (result < 0)
         {
            str.sprintf(tr("Please note:\n\nThere was an error reading\nthe selected velocity datafile!\n\nThe program returned code %d"), result);
            QMessageBox::warning(this, tr("UltraScan Warning"), str,
                                 QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
            cout << "Loading data failed with result code: " << result << endl;
            return(result);
         }
      }
      else   // didn't select any data, so let's go home.
      {
         delete data_io;
         return(-2);   // -2 error code = no data selected
      }
   }
   if (run_type == 2 || run_type == 4 || run_type == 6 || run_type == 8 || run_type == 10 || run_type == 12)
   {
      if ( !fileName.isEmpty() )
      {
         result = data_io->load_run(fileName, 2, has_data, &cp_list);
         if (result < 0)
         {
            str.sprintf(tr("Please note:\n\nThere was an error reading\nthe selected equilibrium datafile!\n\nThe program returned code %d"), result);
            QMessageBox::warning(this, tr("UltraScan Warning"), str,
                                 QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
            cout << tr("Loading data failed with result code: ") << result << endl;
            return(result);
         }
      }
      else   // didn't select any data, so let's go home.
      {
         delete data_io;
         return (-2);
      }
   }
   run_loaded = true;
   createHtmlDir();    // create the directory for html reports
   i=0;
   while (!has_data[i])   //let's find the first cell with data in it
   {
      i++;
   }
   first_cell = i;
   selected_channel = 0;
   selected_lambda = 0;
   selected_cell = i;
   omega_s = pow((double) ((run_inf.rpm[first_cell][0][0]/30)*M_PI), (double) 2);
   step = 1;
   if (GUI)   // if we are not resetting, we want to see the detail window
   {
      details();
      qApp->processEvents();
   }
   int return_flag = data_io->load_hydrodynamics(&hydro_inf);
   if ( return_flag < 0)
   {
      exit(return_flag);
   }

   //???? check the density and viscosity values for each cell ????
   for(i=0; i<8; i++)
   {
      for(j=0; j<4; j++)
      {
         Density[i][j] = hydro_inf.Density[i][j];
         Viscosity[i][j] = hydro_inf.Viscosity[i][j];

         //cout << "Cell " << i << ", channel " << j << ": Density=" << Density[i][j] << ", viscosity=" << Viscosity[i][j] << endl;
         for(k=0; k<3; k++)
         {
            Vbar[i][j][k] = hydro_inf.Vbar[i][j][k];
            Vbar20[i][j][k] = hydro_inf.Vbar20[i][j][k];
         }
      }
   }
   density = hydro_inf.Density[selected_cell][selected_channel];
   viscosity = hydro_inf.Viscosity[selected_cell][selected_channel];
   vbar = hydro_inf.Vbar[selected_cell][selected_channel][0];
   vbar20 = hydro_inf.Vbar20[selected_cell][selected_channel][0];
   calc_correction(run_inf.avg_temperature);
   //cout << "vbar: " << vbar << ", vbar20: " << vbar20 << endl;
   if ((run_type != 7
        && run_type != 8
        && run_type != 0
        && run_type != 9
        && run_type != 10
        && run_type != 11
        && run_type != 12
        && run_type != 31
        && run_type != 32) && GUI)
   {
      update_screen();
      //
      // if we are not resetting, set the selection to the first dataset and update the
      // listbox, else keep the current setting:
      //
      if ((run_type == 2 || run_type ==4 || run_type == 6) && GUI)
      {
         channel_select->setSelected(0, true);
         if (cp_list[run_inf.centerpiece[selected_cell]].channels > 1) //enable the channel selection dialog
         {
            channel_select->setEnabled(true);
            connect(channel_select, SIGNAL(highlighted(int)), SLOT(show_channel(int)));
         }
      }
      cell_select->setSelected(selected_cell, true);
      lambda_select->setSelected(selected_lambda, true);
      if ((run_type == 2 || run_type ==4 || run_type == 6) && GUI)
      {
         channel_select->setSelected(selected_channel, true);
      }
      updateLabels();
      if (load_scan() < 0)
      {
         return(-3); // -3 error code = scan file is missing
      }
      if (GUI)
      {
         plot_edit();
         if (!already)
         {
            // cout << "Step 4\n";
            plot_analysis();
         }
      }
   }
   /********************************************************************************/
   // if we want to load a scan but no GUI (Terminal based Monte Carlo)
   if (run_type != 7
       && run_type != 8
       && run_type != 0
       && run_type != 9
       && run_type != 10
       && run_type != 11
       && run_type != 12
       && run_type != 31
       && run_type != 32
       && !GUI)
   {
      if (load_scan() < 0)
      {
         return(-3); // -3 error code = scan file is missing
      }
   }
   if ((run_type == 1 || run_type == 2) && GUI)
   {
      pb_details->setEnabled(true);
      pb_second_plot->setEnabled(true);
      pb_save->setEnabled(true);
      pb_print->setEnabled(true);
      pb_view->setEnabled(true);
      if (extraCounter > 0) // there is no reset button, the second plot button is used for reset
      {
         pb_reset->setEnabled(true);
      }
      pb_exsingle->setEnabled(true);
      pb_exrange->setEnabled(true);
      updateButtons();
   }
   if ((run_type == 3 || run_type == 4 || run_type == 5 || run_type == 6) && GUI)
   {
      pb_details->setEnabled(true);
      pb_print->setEnabled(true);
      if (extraCounter > 0)// there is no reset button, the second plot button is used for reset
      {
         pb_reset->setEnabled(true);
      }
      pb_exsingle->setEnabled(true);
      pb_exrange->setEnabled(true);
      updateButtons();
   }
   emit dataLoaded();
   delete data_io;
   return(0);
}

//some functions don't need to know hydrodynamic info and can load a local
//file without requirement for database functions, and can pass db_flag=false

int Data_Control_W::load_data(const QString &fileName, bool db_flag)
{
   US_Data_IO *data_io;
   data_io = new US_Data_IO(&run_inf, baseline_flag);
   connect(data_io, SIGNAL(newMessage(QString, int)), this, SLOT(newMessage(QString, int)));
   QString version, str;
   unsigned int i, j, k;
   int result;
   // first clean up before loading a new data set if one has already been loaded
   plateaus_corrected = false;
   rad_corrected = false;
   if (scan_loaded)
   {
      cleanup_loaded_scan();
   }
   if (scan_copied)
   {
      cleanup_copied_scan();
   }
   if (run_loaded)
   {
      cleanup_loaded_run();
   }
   start = true;
   if (run_type == 1 || run_type == 3 || run_type == 5 || run_type == 7 || run_type == 9 || run_type == 11 || run_type == 31 || run_type == 32)
   {
      if ( !fileName.isEmpty() )
      {
         result = data_io->load_run(fileName, run_type, has_data, &cp_list);
         temp_run.temperature.clear();
         temp_run.rpm.clear();
         temp_run.time.clear();
         temp_run.omega_s_t.clear();
         temp_run.plateau.clear();
         temp_run.temperature.resize(8);
         temp_run.rpm.resize(8);
         temp_run.time.resize(8);
         temp_run.omega_s_t.resize(8);
         temp_run.plateau.resize(8);
         for (i=0; i<8; i++)
         {
            temp_run.temperature[i].resize(run_inf.wavelength_count[i]);
            temp_run.rpm[i].resize(run_inf.wavelength_count[i]);
            temp_run.time[i].resize(run_inf.wavelength_count[i]);
            temp_run.omega_s_t[i].resize(run_inf.wavelength_count[i]);
            temp_run.plateau[i].resize(run_inf.wavelength_count[i]);
            for (j=0; j<run_inf.wavelength_count[i]; j++)
            {
               temp_run.temperature[i][j].resize(run_inf.scans[i][j]);
               temp_run.rpm[i][j].resize(run_inf.scans[i][j]);
               temp_run.time[i][j].resize(run_inf.scans[i][j]);
               temp_run.omega_s_t[i][j].resize(run_inf.scans[i][j]);
               temp_run.plateau[i][j].resize(run_inf.scans[i][j]);
            }
         }
         for (i=0; i<8; i++)
         {
            for (j=0; j<run_inf.wavelength_count[i]; j++)
            {
               for (k=0; k<run_inf.scans[i][j]; k++)
               {
                  temp_run.temperature[i][j][k] = run_inf.temperature[i][j][k];
                  temp_run.rpm[i][j][k] = run_inf.rpm[i][j][k];
                  temp_run.time[i][j][k] = run_inf.time[i][j][k];
                  temp_run.omega_s_t[i][j][k] = run_inf.omega_s_t[i][j][k];
                  temp_run.plateau[i][j][k] = run_inf.plateau[i][j][k];
               }
            }
         }
         for (i=0; i<8; i++)
         {
            for (j=0; j<3; j++)
            {
               temp_run.scans[i][j] = run_inf.scans[i][j];
            }
         }

         //cout << "Loading data RETURNED result code: " << result << endl;
         if (result < 0)
         {
            str.sprintf(tr("Please note:\n\nThere was an error reading\nthe selected velocity datafile!\n\nThe program returned code %d"), result);
            QMessageBox::warning(this, tr("UltraScan Warning"), str,
                                 QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
            cout << "Loading data failed with result code: " << result << endl;
            return(result);
         }
      }
      else   // didn't select any data, so let's go home.
      {
         delete data_io;
         return(-2);   // -2 error code = no data selected
      }
   }
   if (run_type == 2 || run_type == 4 || run_type == 6 || run_type == 8 || run_type == 10 || run_type == 12)
   {
      if ( !fileName.isEmpty() )
      {
         result = data_io->load_run(fileName, 2, has_data, &cp_list);
         if (result < 0)
         {
            str.sprintf(tr("Please note:\n\nThere was an error reading\nthe selected equilibrium datafile!\n\nThe program returned code %d"), result);
            QMessageBox::warning(this, tr("UltraScan Warning"), str,
                                 QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
            cout << tr("Loading data failed with result code: ") << result << endl;
            return(result);
         }
      }
      else   // didn't select any data, so let's go home.
      {
         delete data_io;
         return (-2);
      }
   }

   run_loaded = true;
   createHtmlDir();    // create the directory for html reports
   i=0;
   while (!has_data[i])   //let's find the first cell with data in it
   {
      i++;
   }
   first_cell = i;
   selected_channel = 0;
   selected_lambda = 0;
   selected_cell = i;
   omega_s = pow((double) ((run_inf.rpm[first_cell][0][0]/30)*M_PI), (double) 2);
   step = 1;
   if (GUI)   // if we are not resetting, we want to see the detail window
   {
      details();
      qApp->processEvents();
   }
   if (db_flag)
   {
      int return_flag = data_io->load_hydrodynamics(&hydro_inf);
      if ( return_flag < 0)
      {
         exit(return_flag);
      }
      for(i=0; i<8; i++)
      {
         for(j=0; j<4; j++)
         {
            Density[i][j] = hydro_inf.Density[i][j];
            Viscosity[i][j] = hydro_inf.Viscosity[i][j];
            for(k=0; k<3; k++)
            {
               Vbar[i][j][k] = hydro_inf.Vbar[i][j][k];
               Vbar20[i][j][k] = hydro_inf.Vbar20[i][j][k];
            }
         }
      }
      density = hydro_inf.Density[selected_cell][selected_channel];
      viscosity = hydro_inf.Viscosity[selected_cell][selected_channel];
      vbar = hydro_inf.Vbar[selected_cell][selected_channel][0];
      vbar20 = hydro_inf.Vbar20[selected_cell][selected_channel][0];
      calc_correction(run_inf.avg_temperature);
   }

   if ((run_type != 7
        && run_type != 8
        && run_type != 0
        && run_type != 9
        && run_type != 10
        && run_type != 11
        && run_type != 12
        && run_type != 31
        && run_type != 32) && GUI)
   {
      update_screen();
      //
      // if we are not resetting, set the selection to the first dataset and update the
      // listbox, else keep the current setting:
      //
      if ((run_type == 2 || run_type ==4 || run_type == 6) && GUI)
      {
         channel_select->setSelected(0, true);
         if (cp_list[run_inf.centerpiece[selected_cell]].channels > 1) //enable the channel selection dialog
         {
            channel_select->setEnabled(true);
            connect(channel_select, SIGNAL(highlighted(int)), SLOT(show_channel(int)));
         }
      }
      cell_select->setSelected(selected_cell, true);
      lambda_select->setSelected(selected_lambda, true);
      if ((run_type == 2 || run_type ==4 || run_type == 6) && GUI)
      {
         channel_select->setSelected(selected_channel, true);
      }
      updateLabels();
      if (load_scan() < 0)
      {
         return(-3); // -3 error code = scan file is missing
      }
      if (GUI)
      {
         plot_edit();
         if (!already)
         {
            // cout << "Step 4\n";
            plot_analysis();
         }
      }
   }
   /********************************************************************************/
   // if we want to load a scan but no GUI (Terminal based Monte Carlo)
   if (run_type != 7
       && run_type != 8
       && run_type != 0
       && run_type != 9
       && run_type != 10
       && run_type != 11
       && run_type != 12
       && run_type != 31
       && run_type != 32
       && !GUI)
   {
      if (load_scan() < 0)
      {
         return(-3); // -3 error code = scan file is missing
      }
   }
   if ((run_type == 1 || run_type == 2) && GUI)
   {
      pb_details->setEnabled(true);
      pb_second_plot->setEnabled(true);
      pb_save->setEnabled(true);
      pb_print->setEnabled(true);
      pb_view->setEnabled(true);
      if (extraCounter > 0) // there is no reset button, the second plot button is used for reset
      {
         pb_reset->setEnabled(true);
      }
      pb_exsingle->setEnabled(true);
      pb_exrange->setEnabled(true);
      updateButtons();
   }
   if ((run_type == 3 || run_type == 4 || run_type == 5 || run_type == 6) && GUI)
   {
      pb_details->setEnabled(true);
      pb_print->setEnabled(true);
      if (extraCounter > 0)// there is no reset button, the second plot button is used for reset
      {
         pb_reset->setEnabled(true);
      }
      pb_exsingle->setEnabled(true);
      pb_exrange->setEnabled(true);
      updateButtons();
   }
   emit dataLoaded();
   delete data_io;
   return(0);
}

int Data_Control_W::write_data()
{
   if (fn.isEmpty())
   {
      return (-22);
   }

   if (run_type == 11)
   {
      QString source_file, dest_file, str1;
      for (i=1; i<9; i++)
      {
         for (j=1; j<4; j++)
         {
            source_file = USglobal->config_list.result_dir + "/" + runID_old + str1.sprintf(".veloc.%d%d", i, j);
            dest_file = USglobal->config_list.result_dir + "/" + run_inf.run_id + str1.sprintf(".veloc.%d%d", i, j);
            copy(source_file, dest_file);
         }
      }
   }
   if (run_type == 12)
   {
      QString source_file, dest_file, str1;
      for (i=1; i<9; i++)
      {
         for (j=1; j<4; j++)
         {
            for (k=1; k<5; k++)
            {
               source_file = USglobal->config_list.result_dir + "/" + runID_old + str1.sprintf(".equil.%d%d%d", i, j, k);
               dest_file = USglobal->config_list.result_dir + "/" + run_inf.run_id + str1.sprintf(".equil.%d%d%d", i, j, k);
               copy(source_file, dest_file);
            }
         }
      }
   }
   if (run_type == 31)
   {
      QString source_file, dest_file, str1;
      vector <float> ti_noise, ri_noise, abs;
      unsigned int i, j, k, l;
      float fval;
      bool ri=false, ti=false;
      QFile f1, f2;
      for (i=1; i<9; i++)
      {
         for (j=1; j<4; j++)
         {
            abs.clear();
            f1.setName(USglobal->config_list.result_dir + "/" + runID_old + str1.sprintf(".ti_noise.%d%d", i, j));
            f2.setName(USglobal->config_list.result_dir + "/" + runID_old + str1.sprintf(".ri_noise.%d%d", i, j));
            if (f1.exists())
            {
               ti = true;
            }
            else
            {
               ti = false;
            }
            if (f2.exists())
            {
               ri = true;
            }
            else
            {
               ri = false;
            }
            if(ti)
            {
               ti_noise.clear();
               if (f1.open(IO_ReadOnly))
               {
                  QTextStream ts(&f1);
                  while(!ts.eof())
                  {
                     ts >> fval;
                     ti_noise.push_back(fval);
                  }
                  f1.close();
               }
            }
            if(ri)
            {
               ri_noise.clear();
               if (f2.open(IO_ReadOnly))
               {
                  QTextStream ts(&f2);
                  while(!ts.eof())
                  {
                     ts >> fval;
                     ri_noise.push_back(fval);
                  }
                  f2.close();
               }
               if (ri_noise.size() != run_inf.scans[i-1][j-1]) // we cannot subtract ri noise if the number of scans don't match
               {
                  ri = false;
               }
            }
            f1.setName(USglobal->config_list.result_dir + "/" + runID_old + str1.sprintf(".veloc.%d%d", i, j));
            if (f1.open(IO_ReadOnly))
            {
               float **data;
               data = new float *[run_inf.scans[i-1][j-1]];
               for (l=0; l<run_inf.scans[i-1][j-1]; l++)
               {
                  data[l] = new float [run_inf.points[i-1][j-1][0]];
               }
               QDataStream s(&f1);
               abs.clear();
               for (l=0; l<run_inf.scans[i-1][j-1]; l++)
               {
                  for (k=0; k<run_inf.points[i-1][j-1][0]; k++)
                  {
                     s >> fval;
                     if (ri)
                     {
                        fval -= ri_noise[l];
                     }
                     if (ti)
                     {
                        fval -= ti_noise[k];
                     }
                     data[l][k] = fval;
                     abs.push_back(fval);
                  }
               }
               if (ri || ti)
               {
                  for (l=0; l<run_inf.scans[i-1][j-1]; l++)
                  {
                     run_inf.plateau[i-1][j-1][l] = 0.0;
                     for (k=run_inf.points[i-1][j-1][0]-100; k<run_inf.points[i-1][j-1][0]-80; k++)
                     {
                        run_inf.plateau[i-1][j-1][l] += data[l][k];
                     }
                     run_inf.plateau[i-1][j-1][l] /= 20.0;
                     //                     cout << "Plateau in ti/ri routine: " << run_inf.plateau[i-1][j-1][l] << endl;
                  }
               }
               run_inf.baseline[i-1][j-1] = 0.0;
               for (k=0; k<50; k++)
               {
                  run_inf.baseline[i-1][j-1] += data[run_inf.scans[i-1][j-1]-1][k];
               }
               run_inf.baseline[i-1][j-1] /= 50.0;
               //               cout << "baseline[" << i << "][" << j << "]: " <<  run_inf.baseline[i-1][j-1] << endl;
               for (l=0; l<run_inf.scans[i-1][j-1]; l++)
               {
                  delete [] data[l];
               }
               delete [] data;
               f1.close();
               f2.setName(USglobal->config_list.result_dir + "/" + run_inf.run_id + str1.sprintf(".veloc.%d%d", i, j));
               if (ri || ti)
               {
                  //cout << "rewriting cell " << i << endl;
                  if (f2.open(IO_WriteOnly))
                  {
                     QDataStream s(&f2);
                     {
                        for (k=0; k<abs.size(); k++)
                        {
                           s << abs[k];
                        }
                     }
                     f2.close();
                  }
               }
               else
               {
                  if (runID_old != run_inf.run_id)
                  {
                     source_file = USglobal->config_list.result_dir + "/" + runID_old + str1.sprintf(".veloc.%d%d", i, j);
                     dest_file = USglobal->config_list.result_dir + "/" + run_inf.run_id + str1.sprintf(".veloc.%d%d", i, j);
                     copy(source_file, dest_file);
                  }
               }
            }
         }
      }
   }
   if (run_type == 9 || run_type == 11 || run_type == 31 || run_type == 32)
   {
      QFile f(fn);
      f.remove();
      if (!f.open(IO_WriteOnly))
      {
         QString str = tr("UltraScan encountered a Problem when trying to open the edited data file.\n\n"
                          "Please make sure that the file: \"")  + fn +
            tr("\"\ncan be written to your results directory.\n\n"
               "Current Results Directory: \"")
            + USglobal->config_list.result_dir + "\"";
         QMessageBox::message(tr("Attention:"), str);
         return(-20);   // -20 error code = file can't be written
      }
      QDataStream ts (&f);
      ts << US_Version;
      ts << run_inf.data_dir;
      ts << run_inf.run_id;
      ts << run_inf.avg_temperature;
      ts << run_inf.temperature_check;
      ts << run_inf.time_correction;
      ts << run_inf.duration;
      ts << run_inf.total_scans;
      ts << run_inf.delta_r;
      ts << run_inf.expdata_id;
      ts << run_inf.investigator;
      ts << run_inf.date;
      ts << run_inf.description;
      ts << run_inf.dbname;
      ts << run_inf.dbhost;
      ts << run_inf.dbdriver;
      ts << (int)run_inf.exp_type.velocity;
      ts << (int)run_inf.exp_type.equilibrium;
      ts << (int)run_inf.exp_type.diffusion;
      ts << (int)run_inf.exp_type.simulation;
      ts << (int)run_inf.exp_type.interference;
      ts << (int)run_inf.exp_type.absorbance;
      ts << (int)run_inf.exp_type.fluorescence;
      ts << (int)run_inf.exp_type.intensity;
      ts << (int)run_inf.exp_type.wavelength;
      for (i=0; i<8; i++)
      {
         ts << run_inf.centerpiece[i];
         ts << run_inf.meniscus[i];
         ts << run_inf.cell_id[i];
         ts << run_inf.wavelength_count[i];
      }
      for (i=0; i<8; i++)
      {
         for (j=0; j<4; j++)   // one for each channel
         {
            ts << run_inf.buffer_serialnumber[i][j];
            for(int k=0; k<3;k++)
            {
               ts << run_inf.peptide_serialnumber[i][j][k];
               ts << run_inf.DNA_serialnumber[i][j][k];
            }
         }
         for (j=0; j<3; j++)
         {
            ts << run_inf.wavelength[i][j];
            ts << run_inf.scans[i][j];
            ts << run_inf.baseline[i][j];
            if (run_inf.centerpiece[i] >= 0)
            {
               for (k=0; k<cp_list[run_inf.centerpiece[i]].channels; k++)
               {
                  ts << run_inf.range_left[i][j][k];
                  ts << run_inf.range_right[i][j][k];
                  ts << run_inf.points[i][j][k];
                  ts << run_inf.point_density[i][j][k];
               }
            }
            selected_channel = 0;
         }
      }
      for (i=0; i<8; i++)
      {
         for (j=0; j<run_inf.wavelength_count[i]; j++)
         {
            for (k=0; k<run_inf.scans[i][j]; k++)
            {
               ts << run_inf.rpm[i][j][k];
               ts << run_inf.temperature[i][j][k];
               // add the time correction back on before writing the data back to disk:
               run_inf.time[i][j][k] += (unsigned int) (run_inf.time_correction + 0.5);
               ts << run_inf.time[i][j][k];
               ts << run_inf.omega_s_t[i][j][k];
               //cout << "cell: " << i + 1 << ", lambda: " << j+1 << ", channel: " << k +1 << ": " << run_inf.plateau[i][j][k] << endl;;
               ts << run_inf.plateau[i][j][k];
               //               cout << "writing plateau: [i=" << i << "][j=" << j << "][k=" << k << "]: " << temp_run.plateau[i][j][k] << endl;
            }
         }
      }
      ts << run_inf.rotor;
   }
   if (run_type == 10 || run_type == 12)
   {
      QFile f(fn);
      f.open(IO_WriteOnly);
      QDataStream ts (&f);
      ts << US_Version;
      ts << run_inf.data_dir;
      ts << run_inf.run_id;
      ts << run_inf.duration;
      ts << run_inf.total_scans;
      ts << run_inf.delta_r;
      ts << run_inf.expdata_id;
      ts << run_inf.investigator;
      ts << run_inf.date;
      ts << run_inf.description;
      ts << run_inf.dbname;
      ts << run_inf.dbhost;
      ts << run_inf.dbdriver;
      ts << (int)run_inf.exp_type.velocity;
      ts << (int)run_inf.exp_type.equilibrium;
      ts << (int)run_inf.exp_type.diffusion;
      ts << (int)run_inf.exp_type.simulation;
      ts << (int)run_inf.exp_type.interference;
      ts << (int)run_inf.exp_type.absorbance;
      ts << (int)run_inf.exp_type.fluorescence;
      ts << (int)run_inf.exp_type.intensity;
      ts << (int)run_inf.exp_type.wavelength;
      for (i=0; i<8; i++)
      {
         ts << run_inf.centerpiece[i];
         ts << run_inf.cell_id[i];
         ts << run_inf.wavelength_count[i];
      }
      for (i=0; i<8; i++)
      {
         for (j=0; j<4; j++)   // one for each channel
         {
            ts << run_inf.buffer_serialnumber[i][j];
            for(int k=0; k<3;k++)
            {
               ts << run_inf.peptide_serialnumber[i][j][k];
               ts << run_inf.DNA_serialnumber[i][j][k];
            }
         }
         for (j=0; j<3; j++)
         {
            ts << run_inf.wavelength[i][j];
            ts << run_inf.scans[i][j];
            if (run_inf.centerpiece[i] >= 0  && run_inf.scans[i][j] != 0)
            {
               for (k=0; k<cp_list[run_inf.centerpiece[i]].channels; k++)
               {
                  ts << run_inf.range_left[i][j][k];
                  ts << run_inf.range_right[i][j][k];
                  ts << run_inf.points[i][j][k];
                  ts << run_inf.point_density[i][j][k];
               }
            }
         }
      }
      for (i=0; i<8; i++)
      {
         for (j=0; j<run_inf.wavelength_count[i]; j++)
         {
            for (k=0; k<run_inf.scans[i][j]; k++)
            {
               ts << run_inf.rpm[i][j][k];
               ts << run_inf.temperature[i][j][k];
               ts << run_inf.time[i][j][k];
               ts << run_inf.omega_s_t[i][j][k];
            }
         }
      }
      for (i=0; i<8; i++)
      {
         has_data[i]=false;
         if (run_inf.scans[i][0] != 0)
         {
            has_data[i] = true;
         }
      }
      ts << run_inf.rotor;
      f.close();
   }
   run_loaded = true;
   if (scan_loaded)
   {
      cleanup_loaded_scan();
   }
   if (scan_copied)
   {
      cleanup_copied_scan();
   }
   if (run_loaded)
   {
      cleanup_loaded_run();
   }
   return(0);
}

void Data_Control_W::cleanup_loaded_run()
{
   run_loaded = false;
}

int Data_Control_W::load_scan()
{
   float val;
   if (scan_loaded)
   {
      cleanup_loaded_scan();
   }
   unsigned int i, j, k, scan;
   if (run_type == 1 || run_type == 3 || run_type == 5 || run_type == 7 || run_type == 9 || run_type == 11 || run_type == 31 || run_type == 32)
   {
      for (i=0; i<8; i++)
      {
         for (j=0; j<run_inf.wavelength_count[i]; j++)
         {
            for (k=0; k<temp_run.scans[i][j]; k++)
            {
               run_inf.temperature[i][j][k] = temp_run.temperature[i][j][k];
               run_inf.rpm[i][j][k] = temp_run.rpm[i][j][k];
               run_inf.time[i][j][k] = temp_run.time[i][j][k];
               run_inf.omega_s_t[i][j][k] = temp_run.omega_s_t[i][j][k];
               //               cout << run_inf.omega_s_t[i][j][k] << endl;
               run_inf.plateau[i][j][k] = temp_run.plateau[i][j][k];
               //               cout << "loading scan plateau: [i=" << i << "][j=" << j << "][k=" << k << "]: " << temp_run.plateau[i][j][k] << endl;
            }
         }
      }
      for (i=0; i<8; i++)
      {
         for (j=0; j<3; j++)
         {
            run_inf.scans[i][j] = temp_run.scans[i][j];
         }
      }
   }
   QString filename, str;
   if (GUI)
   {
      cnt_exrange->setRange(1, run_inf.scans[selected_cell][selected_lambda], 1);
      cnt_exsingle->setRange(1, run_inf.scans[selected_cell][selected_lambda], 1);
   }
   filename = USglobal->config_list.result_dir;
   filename.append("/");
   filename.append(run_inf.run_id);
   if (run_type == 1 || run_type == 3 || run_type == 5 || run_type == 7)
   {
      str.sprintf(".veloc.%d%d",selected_cell+1, selected_lambda+1);
   }
   if (run_type == 2 || run_type == 4 || run_type == 6 || run_type == 8)
   {
      str.sprintf(".equil.%d%d%d",selected_cell+1, selected_lambda+1, selected_channel+1);
   }
   filename.append(str);
   QFile f(filename);
   if (!f.open(IO_ReadOnly))
   {
      QString str1;
      str1.sprintf(tr("Sorry, a requested file is missing:\n\n"));
      str1.append(filename);
      str1.append(tr("\n\nPlease restore the file, then try again."));
      if (GUI)
      {
         QMessageBox::message(tr("Attention:"), str1);
      }
      else
      {
         cout << str1.latin1() << endl;
      }
      return (-1);
   }
   QDataStream s( &f );
   points = run_inf.points[selected_cell][selected_lambda][selected_channel];
   abs         = new float*  [run_inf.scans[selected_cell][selected_lambda]];
   rad         = new float  [points];
   absorbance = new double* [run_inf.scans[selected_cell][selected_lambda]];
   radius     = new double [points];
   for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
   {
      abs[i]         = new float  [points];
      absorbance[i] = new double [points];
   }
   for(scan=0; scan<run_inf.scans[selected_cell][selected_lambda]; scan++)
   {
      if (run_type == 2 || run_type == 4 || run_type == 6)
      {
         s >> val;
         run_inf.equil_meniscus.push_back(val);
      }
      for (i=0; i<points; i++)
      {
         s >> abs[scan][i];
         /*
           if (i==0)
           {
           cout << abs[scan][i] << endl;
           }
         */
         if (run_type == 1 && baseline_flag)
         {
            //            cout << "subtracting baseline in datactl...\n";
            abs[scan][i] -= run_inf.baseline[selected_cell][selected_lambda];
         }
         absorbance[scan][i] = (double) abs[scan][i];
         rad[i] = run_inf.range_left[selected_cell][selected_lambda][0] + i * run_inf.delta_r;
         radius[i] = (double) rad[i];
      }
   }
   f.close();
   curve = new uint [run_inf.scans[selected_cell][selected_lambda]*3];
   included = new bool [run_inf.scans[selected_cell][selected_lambda]];
   for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
   {
      included[i] = true;
   }
   scan_loaded = true;
   old_selected_cell = selected_cell;
   old_selected_lambda = selected_lambda;
   plateaus_corrected = false;
   rad_corrected = false;
   if (!smooth_flag) // if we are smoothing we don't want to reset
   {
      if (smoothing_counter != NULL)
      {
         smoothing_counter->disconnect();
         smoothing_counter->setRange(1, 50, 1);
         smoothing_counter->setValue(1);
         connect(smoothing_counter, SIGNAL(valueChanged(double)), SLOT(update_smoothing(double)));
      }
      if (range_counter != NULL)
      {
         range_counter->disconnect();
         position_counter->disconnect();
         bd_range=90;
         range_counter->setValue(bd_range);
         connect(range_counter, SIGNAL(valueChanged(double)), SLOT(update_boundary_range(double)));
         connect(position_counter, SIGNAL(valueChanged(double)), SLOT(update_boundary_position(double)));
      }
   }
   if (!smooth_flag)
   {
      plot_edit();
      plot_analysis();
   }
   return(0);
}

void Data_Control_W::cleanup_loaded_scan()
{
   delete [] rad;
   delete [] radius;
   for (i=0; i<run_inf.scans[old_selected_cell][old_selected_lambda]; i++)
   {
      delete [] abs[i];
      delete [] absorbance[i];
   }
   delete [] abs;
   delete [] absorbance;
   delete [] curve;
   delete [] included;
   scan_loaded = false;
}

void Data_Control_W::details()
{
   if (step == 0)
   {
      QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n      Click on \"Load Data\"\n"));
      return;
   }
   if ((run_type == 1 || run_type == 3 || run_type == 5) && !GUI) // need to call with status = -1 to prevent using GUI functions in rundetails
   {
      run_details = new RunDetails_F(1, -1, &run_inf);
   }
   if ((run_type == 2 || run_type == 4 || run_type == 6) && !GUI) // need to call with status = -1 to prevent using GUI functions in rundetails
   {
      run_details = new RunDetails_F(2, -1, &run_inf);
   }
   if (run_type == 1 || run_type == 3 || run_type == 5) // || run_type == 7)
   {
      run_details = new RunDetails_F(1, 1, &run_inf);
      run_details->show();
      run_details->raise();
      qApp->processEvents();
   }
   if (run_type == 2 || run_type == 4 || run_type == 6) // || run_type == 8)
   {
      run_details = new RunDetails_F(2, 1, &run_inf);
      run_details->show();
      run_details->raise();
      qApp->processEvents();
   }
   if (run_type == 9) // we want to be able to change the cell id's
   {
      run_details = new RunDetails_F(1, 2, &run_inf);
      run_details->show();
      run_details->raise();
      qApp->processEvents();
      connect(run_details->pb_accept, SIGNAL(clicked()), SLOT(write_data()));
   }
   if (run_type == 10)
   {
      run_details = new RunDetails_F(2, 2, &run_inf);
      run_details->show();
      run_details->raise();
      qApp->processEvents();
      connect(run_details->pb_accept, SIGNAL(clicked()), SLOT(write_data()));
   }
   if (run_type == 11) // we want to change the velocity run ID and copy the data
   {
      QString message = tr("Please enter a new Run Identification\nfor the Velocity Data Copy:");
      runID_old = run_inf.run_id;
      OneLiner ol_descr(message);
      ol_descr.show();
      if (ol_descr.exec())
      {
         run_inf.run_id = ol_descr.string;
      }
      else
      {
         return;
      }
      fn = USglobal->config_list.result_dir + "/" + run_inf.run_id + ".us.v";
      run_details = new RunDetails_F(1, 3, &run_inf);
      run_details->run_id_le->setText(run_inf.run_id);
      run_details->show();
      run_details->raise();
      qApp->processEvents();
      connect(run_details->pb_accept, SIGNAL(clicked()), SLOT(write_data()));
   }
   if (run_type == 31) // we want to change the velocity run ID and copy the data, subtracting ri/ti noise
   {
      QString message = tr("Please enter a new run identification.\n"
                           "This process will subtract time- and\n"
                           "radially invariant noise from your\n"
                           "edited velocity data. You can keep the\n"
                           "name the same, or create a new name.\n"
                           "If you keep the name the same, the\n"
                           "edited data will be overwritten.");
      runID_old = run_inf.run_id;
      OneLiner ol_descr(message);
      ol_descr.show();
      ol_descr.parameter1->setText(run_inf.run_id);
      if (ol_descr.exec())
      {
         run_inf.run_id = ol_descr.string;
      }
      else
      {
         return;
      }
      fn = USglobal->config_list.result_dir + "/" + run_inf.run_id + ".us.v";
      run_details = new RunDetails_F(1, 3, &run_inf);
      run_details->run_id_le->setText(run_inf.run_id);
      run_details->show();
      run_details->raise();
      qApp->processEvents();
      connect(run_details->pb_accept, SIGNAL(clicked()), SLOT(write_data()));
   }
   if (run_type == 32) // we want to update the meniscus for a velocity run
   {
      float *mval;
      mval = new float [8];
      for (i=0; i<8; i++)
      {
         mval[i] = run_inf.meniscus[i];
      }
      US_MeniscusDialog *meniscus_dlg;
      meniscus_dlg = new US_MeniscusDialog(mval, run_inf.run_id);
      meniscus_dlg->show();
      if (meniscus_dlg->exec())
      {
         for (i=0; i<8; i++)
         {
            run_inf.meniscus[i] = mval[i]; // data was changed/accepted, update meniscus
         }
         write_data();
         delete [] mval;
         return;
      }
      else
      {
         delete [] mval;
         return;
      }
   }
   if (run_type == 12) // we want to change the equilibrium run ID and copy the data
   {
      QString message = tr("Please enter a new Run\nIdentification for the Equilibrium Data Copy:");
      runID_old = run_inf.run_id;
      OneLiner ol_descr(message);
      ol_descr.show();
      if (ol_descr.exec())
      {
         run_inf.run_id = ol_descr.string;
      }
      else
      {
         return;
      }
      fn = USglobal->config_list.result_dir + "/" + run_inf.run_id + ".us.e";
      run_details = new RunDetails_F(2, 3, &run_inf);
      run_details->run_id_le->setText(run_inf.run_id);
      run_details->show();
      run_details->raise();
      qApp->processEvents();
      connect(run_details->pb_accept, SIGNAL(clicked()), SLOT(write_data()));
   }
   //   run_details->setCaption("Details for Velocity Absorbance Data");
}

void Data_Control_W::calc_correction(float t)
{
   double density_wt, density_wt_20;
   density_wt    = 1.000028e-3 * ((999.83952 + 16.945176 * t) / (1 + 16.879850e-3 * t))
      - 1.000028e-3 * ((7.9870401e-3 * pow((double) t, (double) 2.0) + 46.170461e-6 * pow((double) t, (double) 3.0))
                       / (1.0 + 16.87985e-3 * t))
      + 1.000028e-3 * ((105.56302e-9 * pow((double) t, (double) 4.0) - 280.54253e-12 * pow((double) t, (double) 5.0))
                       / (1.0 + 16.87985e-3 * t));
   density_wt_20 = DENS_20W;
   //cout << "density_w_20: " << density_wt_20 << endl;
   if (t < 20.0)
   {
      viscosity_wt = 100.0 * pow((double) 10.0, (double) (1301 / (998.333 + 8.1855 * (t - 20.0)
                                                                  + 5.85e-3 * pow((double) (t-20.0), (double) 2.0)) - 3.302));
   }
   else
   {
      viscosity_wt = (100.0 * VISC_20W) * pow((double) 10.0, (double) ((1.3272 * (20.0 - t) - 1.053e-3 * pow((double) (t - 20.0), (double) 2.0))
                                                                       / (t + 105.0)));
   }

   density_tb   = density * density_wt / density_wt_20;
   viscosity_tb = viscosity * viscosity_wt / (100.0 * VISC_20W);
   cout.precision(8);
   //cout << "density tb: " << density_tb << ", viscosity_tb: " << viscosity_tb << ", vbar_t: " << vbar << endl;
   //cout << "density: " << density << ", viscosity: " << viscosity << ", density_wt_20: " << density_wt_20 << ", viscosity_w: " << viscosity_wt << endl;
   //cout << "Temperature: " << t << ", vbar: " << vbar << ", vbar20: " << vbar20 << endl;
   buoyancyb    = 1.0 - vbar * density_tb;
   buoyancyw    = 1.0 - vbar20 * density_wt_20;
   correction   = (buoyancyw / buoyancyb) * (viscosity_tb / (100.0 * VISC_20W));
   //cout << "buoyancyb: " << buoyancyb << ", buoyancyw: " << buoyancyw << ", vbar: " << vbar << ", vbar20: " << vbar20 << endl;
   //cout << "density_w_20: " << density_wt_20 << endl;

   /*
     cerr << "in Data Control: densitytb:" <<   density_tb
     << ", viscosity tb: " << viscosity_tb
     << ", temperature: " << t
     << ", viscosity: " << viscosity
     << ", viscosity_wt: " << viscosity_wt
     << ", buoyancyb: " << buoyancyb
     << ", buoyancyw: " << buoyancyw
     << ", vbar: " << vbar
     << ", vbar20: " << vbar20
     << ", correction: " << correction << endl;
   */
}

void Data_Control_W::update_screen()
{
   QString str, str1, str2;
   lbl_run_id2->setText(run_inf.run_id);
   lbl_temperature2->setText(str1.sprintf(" %5.3f ºC", run_inf.avg_temperature));
   int i;
   str = "";
   for (i=0; i<8; i++)
   {
      if (run_inf.cell_id[i] != "")
      {
         has_data[i]=true;
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
      lbl_cell_info2->setText(str);
   }
   lbl_cell_descr->setText(run_inf.cell_id[selected_cell]);
   if (run_inf.exp_type.interference)
   {
      str.sprintf(tr("Fringes at %d nm"), run_inf.wavelength[selected_cell][selected_lambda]);
   }
   if (run_inf.exp_type.absorbance)
   {
      str.sprintf(tr("Absorbance at %d nm"), run_inf.wavelength[selected_cell][selected_lambda]);
   }
   edit_plot->setAxisTitle(QwtPlot::yLeft, str);
}

void Data_Control_W::quit()
{
   close();
}

void Data_Control_W::update_density(const QString &val)
{
   if (step == 0)
   {
      QMessageBox::message(tr("Attention:"),tr("You need to load some data first!\n\nClick on \"Load Data\""));
      return;
   }
   density = val.toFloat();
   calc_correction(run_inf.avg_temperature);
   if (!dont_plot)
   {
      if (step != 0)
      {
         // cout << "Step 6\n";
         plot_analysis();
      }
   }
}

void Data_Control_W::update_viscosity()
{
   update_viscosity(viscosity_le->text());
}

void Data_Control_W::update_density()
{
   update_density(density_le->text());
}

void Data_Control_W::update_vbar()
{
   update_vbar(vbar_le->text());
}

void Data_Control_W::update_viscosity(const QString &val)
{
   if (step == 0)
   {
      QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n      Click on \"Load Data\"\n"));
      return;
   }
   viscosity = val.toFloat();
   calc_correction(run_inf.avg_temperature);
   //cout << "Viscosity: " << viscosity << endl;
   if (!dont_plot)
   {
      if (step != 0)
      {
         // cout << "Step 7\n";
         plot_analysis();
      }
   }
}

void Data_Control_W::update_vbar(const QString &val)
{
   if (!vbar_from_seq)
   {
      vbar20 = val.toFloat();
      vbar = vbar20 + (4.25e-4 * (run_inf.avg_temperature - 20));
      emit vbarChanged(vbar, vbar20);
      if (step != 0 && !dont_plot)
      {
         // cout << "Step 7A\n";

         plot_analysis();
      }
   }
   vbar_from_seq = false;
}

void Data_Control_W::update_vbar_lbl(float val, float val20)
{
   vbar = val;
   vbar20 = val20;
   QString str;
   str.sprintf("%6.4f", vbar20);
   if (vbarDisplay)
   {
      vbar_le->setText(str);
   }
   calc_correction(run_inf.avg_temperature);
   // cout << "Step 9\n";
   plot_analysis();
}

void Data_Control_W::update_buffer_lbl(float dens, float visc)
{
   dont_plot = true;
   QString str, str2;
   density = dens;
   viscosity = visc;
   str.sprintf("%8.6f", density);
   if (densityDisplay)
   {
      density_le->setText(str);
   }
   str.sprintf("%8.6f", viscosity);
   if (viscosityDisplay)
   {
      viscosity_le->setText(str);
   }
   dont_plot = false;
   calc_correction(run_inf.avg_temperature);
   //update_viscosity(viscosity_le->text());
   //update_density(density_le->text());
   //cout << "Viscosity: " << viscosity << ", density: " << density << endl;
   // cout << "Step 10\n";
   plot_analysis();
}

void Data_Control_W::read_vbar()
{
   if (step == 0)
   {
      QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n      Click on \"Load Data\"\n"));
      return;
   }
   vbar_from_seq = true;
   vbar_dlg = new US_Vbar_DB(run_inf.avg_temperature, &vbar, &vbar20, true, false, run_inf.investigator);
   vbar_dlg->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   vbar_dlg->setCaption(tr("V-bar Calculation"));
   connect(vbar_dlg, SIGNAL(valueChanged(float, float)), SLOT(update_vbar_signal(float, float)));
   connect(vbar_dlg, SIGNAL(mwChanged(float)), SLOT(update_mw_signal(float)));
   if (run_type == 0 || run_type == 7 || run_type == 8) // dont want to update labels that don't exist if run type = 0 (no run)
   {
      return;
   }
   else
   {
      connect(vbar_dlg, SIGNAL(valueChanged(float, float)), SLOT(update_vbar_lbl(float, float)));
   }
   vbar_dlg->exec();
}

void Data_Control_W::get_buffer()
{
   if (step == 0)
   {
      QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n      Click on \"Load Data\"\n"));
      return;
   }
   buffer_from_seq = true;
   buffer_dlg = new US_Buffer_DB(false, run_inf.investigator);
   buffer_dlg->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   buffer_dlg->setCaption(tr("Buffer Calculation"));
   //buffer_dlg->show();
   connect(buffer_dlg, SIGNAL(valueChanged(float, float)), SLOT(update_buffer_signal(float, float)));
   if (run_type == 0 || run_type == 7 || run_type == 8) // dont want to update labels that don't exist if run type = 0 (no run)
   {
      return;
   }
   else
   {
      connect(buffer_dlg, SIGNAL(valueChanged(float, float)), this, SLOT(update_buffer_lbl(float, float)));
   }
   buffer_dlg->exec();
}

void Data_Control_W::update_vbar_signal(float val1, float val2)
{
   //   cout<<"OK1"<<endl;
   emit vbarChanged(val1, val2);
}
void Data_Control_W::update_mw_signal(float val)
{
   emit mwChanged(val);
}
void Data_Control_W::update_buffer_signal(float val1, float val2)
{
   emit bufferChanged(val1, val2);
}


void Data_Control_W::update_smoothing(double val)
{
   //cout << "Step: " << step << ", val: " << val <<endl;
   if (step == 0)
   {
      smooth = 1;
      smoothing_counter->setValue(1);
      return;
   }
   unsigned int i, j, k;
   float sum;
   smooth_flag = true;
   load_scan();
   smooth = (unsigned int) val;
   if (smoothing_counter != NULL)
   {
      smoothing_counter->setRange(1, 50, 1);
   }
   if (smooth > 1.0) // we'll apply a normalized Gaussian smoothing kernel that goes out to 2 standard deviations
   {
      float *x_weights, *y_weights;
      float increment = 2.0/(smooth);
      float sum_y;
      int position;
      x_weights       = new float [smooth];
      y_weights       = new float [smooth];
      x_weights[0] = 0.0;
      y_weights[0] = normal_distribution(1.0, 0.0, x_weights[0]); // standard deviation = 1.0, Mean = 0;
      for (i=1; i<smooth; i++) //we only calculate half a Gaussian, since the other side is symmetric
      {
         x_weights[i] = x_weights[i-1] + increment;
         y_weights[i] = normal_distribution(1, 0, x_weights[i]);
      }

      // first, take care of the left border, using an "appearing frame" algorithm,
      // starting with half a frame visible:

      for(i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
      {
         for(j=0; j<smooth; j++)   // loop over all border point centers
         {
            sum = 0.0;
            sum_y = 0.0;
            position = 0;
            for(k=j; k<j+smooth; k++)
            {
               sum += abs[i][k] * y_weights[position];   // sum the weighted points right of center, including center
               sum_y += y_weights[position];
               position ++;
            }
            position = 0;
            for (int m=j-1; m>= 0; m--)
            {
               position ++;
               sum += abs[i][m] * y_weights[position];    // sum all applicable points on the left of center
               sum_y += y_weights[position];
            }
            absorbance[i][j] = (double) (sum/sum_y);   // normalize by the sum of all weights that were used
         }
      }

      // Now deal with all non-border points:

      for(i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
      {
         for(j=smooth; j<points-smooth; j++)   // loop over all non-border points
         {
            sum = 0.0;
            sum_y = 0.0;
            position = 0;
            for(k=j; k<j+smooth; k++)
            {
               sum += abs[i][k] * y_weights[position];   // sum the weighted points right of center, including center
               sum_y += y_weights[position];
               position ++;
            }
            position = 0;
            for (int m = (int) j-1; m >= (int) (j-smooth+1); m--)
            {
               position ++;
               sum += abs[i][m] * y_weights[position];    // sum all applicable points on the left of center
               sum_y += y_weights[position];
            }
            absorbance[i][j] = (double) (sum/sum_y);   // normalize by the sum of all weights that were used
         }
      }

      // Now deal with all points from the right border, using a "disappearing frame" algorithm,
      // starting with a full frame minus 1 point visible:

      for(i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
      {
         for(j=points-smooth; j<points; j++)   // loop over all right-border points
         {
            sum = 0.0;
            sum_y = 0.0;
            position = 0;
            for (int m= (int)j-1; m>=(int) (j-smooth+1); m--)
            {
               position ++;
               sum += abs[i][m] * y_weights[position];    // sum all points on the left of center
               sum_y += y_weights[position];
            }
            position = 0;
            for(k=j; k<points; k++)
            {
               sum += abs[i][k] * y_weights[position];   // sum the weighted points in the disappearing frame
               sum_y += y_weights[position];               // right of center, including center
               position ++;
            }
            absorbance[i][j] = (double) (sum/sum_y);   // normalize by the sum of all weights that were used
         }
      }
      delete[] x_weights;
      delete[] y_weights;
   }
   else
   {
      for(i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
      {
         for(j=0; j<points; j++)
         {
            absorbance[i][j] = abs[i][j];
         }
      }
   }
   plot_edit();
   if (smoothing_counter != NULL)
   {
      // cout << "Step -1" << endl;
      plot_analysis();
   }
   emit controls_updated();
}

void Data_Control_W::find_plateaus()
{
   unsigned int i, j, order = 2;
   unsigned int lower, upper;
   float test_val;
   unsigned int min_points = (int) (30 - smooth); // min_points is the minimum number of points that need to be
   //horizontal. If smoothing is enabled the number can be smaller.
   for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
   {
      // first find the left boundary of the plateau, coming from the bottom:
      lower = 0;
      test_val = -1e6;
      // cout << "Plateau[" << i << "]: " << run_inf.plateau[selected_cell][selected_lambda][i]<< endl;
      //      while (test_val <= run_inf.plateau[selected_cell][selected_lambda][i]-run_inf.plateau[selected_cell][selected_lambda][i]/100)
      while (lower < points && test_val <= run_inf.plateau[selected_cell][selected_lambda][i])
      {
         test_val = absorbance[i][lower];
         lower++;
      }
      // cout << "Lower: " << lower << ", test_val: " << test_val << endl;
      // now find the right boundary of the plateau, coming from the top:
      upper = points - 1;
      test_val = 1e6;
      //      while (test_val >= run_inf.plateau[selected_cell][selected_lambda][i]+run_inf.plateau[selected_cell][selected_lambda][i]/100)
      while (upper > 0 && test_val >= run_inf.plateau[selected_cell][selected_lambda][i])
      {
         test_val = absorbance[i][upper];
         upper --;
      }
      // cout << ", upper: " << upper << ", difference: " << (int) (upper-lower) << endl;
      // now we have the index for the lower and upper boundary of the plateau
      // let's make sure we have at least 50 plateau points that are horizontal:
      if (upper - lower < min_points)
      {
         break;
      }
   }
   j = i;
   if (i < 3 || run_inf.wavelength[selected_cell][selected_lambda] == 999)
   {
      if (run_inf.wavelength[selected_cell][selected_lambda] != 999)
      {

         // now try to find some points that form a linear fit to the plateau values of the first scans
         // keep subtracting scans until the slope doesn't increase any more
         unsigned int max_scan = run_inf.scans[selected_cell][selected_lambda];
         double *slope;
         slope = new double [run_inf.scans[selected_cell][selected_lambda]];
         int counter = 0;
         while (max_scan > 2)
         {
            counter++;
            double *tempx, *tempy, intercept, sigma, correl;
            tempx = new double [max_scan];
            tempy = new double [max_scan];
            for(j=0; j<max_scan; j++)
            {
               tempx[j] = j;
               tempy[j] = run_inf.plateau[selected_cell][selected_lambda][j];
            }
            linefit(&tempx, &tempy, &slope[max_scan-1], &intercept, &sigma, &correl, max_scan);
            //cout << "Slope[" << max_scan << "]: " << slope[max_scan-1] << ", correlation: " << correl << ", std. dev: " << sigma << endl;
            //cout << counter << "\t" << run_inf.plateau[selected_cell][selected_lambda][counter] << "\t" << slope[max_scan-1] << "\t" << correl << "\t" << sigma << endl;
            delete [] tempx;
            delete [] tempy;
            max_scan--;
         }
         delete [] slope;

         QMessageBox::message( tr("Warning"),
                               tr("The program was unable to find a sufficient\n"
                                  "number of scans with long enough horizontal regions\n"
                                  "to make a reliable extrapolation for the plateaus.\n\n"
                                  "Reducing the smoothing value may help, including\n"
                                  "additional early scans is also highly recommended.\n\n"
                                  "Measured plateaus will be used instead - no plateau\n"
                                  "optimization will be performed"));
         double *t, *cp, *coeffs;
         order = 2;
         cp = new double [run_inf.scans[selected_cell][selected_lambda]];
         t = new double [run_inf.scans[selected_cell][selected_lambda]];
         US_lsfit *polyfit;
         coeffs = new double [order];
         for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
         {
            cp[i] = log(run_inf.plateau[selected_cell][selected_lambda][i]);
            t[i] = run_inf.time[selected_cell][selected_lambda][i];
         }
         polyfit = new US_lsfit(coeffs, t, cp, order, run_inf.scans[selected_cell][selected_lambda], false);
         delete [] t;
         delete [] cp;
         delete [] coeffs;
         initial_concentration = exp(coeffs[0]);
         weight_average_s = 0;
         plateaus_corrected = true;
         return;
      }
      else    // if the wavelength is "999" the run is is a simulated run and we can
      {      // estimate the initial concentration from the plateaus

         double *t, *cp, *coeffs;
         order = 5;
         j = run_inf.scans[selected_cell][selected_lambda];
         if (j < 15)
         {
            order = 4;
         }
         if (j < 10)
         {
            order = 3;
         }
         if (j < 5)
         {
            order = 2;
         }
         cp = new double [run_inf.scans[selected_cell][selected_lambda]];
         t = new double [run_inf.scans[selected_cell][selected_lambda]];
         US_lsfit *polyfit;
         coeffs = new double [order];
         for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
         {
            cp[i] = log(run_inf.plateau[selected_cell][selected_lambda][i]);
            t[i] = run_inf.time[selected_cell][selected_lambda][i];
         }
         polyfit = new US_lsfit(coeffs, t, cp, order, run_inf.scans[selected_cell][selected_lambda], false);
         delete [] t;
         delete [] cp;
         delete [] coeffs;
         initial_concentration = exp(coeffs[0]);
         weight_average_s = 0;
         plateaus_corrected = true;
         return;
      }
   }
   double *t, *cp, *coeffs;
   cp = new double [j];
   t = new double [j];
   US_lsfit *polyfit;
   coeffs = new double [order];
   for (i=0; i<j; i++)
   {
      cp[i] = log(run_inf.plateau[selected_cell][selected_lambda][i]);
      t[i] = run_inf.time[selected_cell][selected_lambda][i];
   }
   polyfit = new US_lsfit(coeffs, t, cp, order, j, false);
   initial_concentration = exp(coeffs[0]);
   // reassign the plateau values:
   float temp;
   for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
   {
      temp = 0;
      for (j=0; j<order; j++)
      {
         temp += coeffs[j] * pow((float) run_inf.time[selected_cell][selected_lambda][i], (float) j);
      }
      run_inf.plateau[selected_cell][selected_lambda][i] = exp(temp);
      //cout << "Plateaus: " << exp(temp) << endl;
   }
   delete [] t;
   delete [] cp;
   delete [] coeffs;

   plateaus_corrected = true;
}

void Data_Control_W::update_boundary_position(double val)
{
   if (step == 0)
   {
      bd_position = 5.0;
      return;
   }
   bd_position = val;
   plot_edit();
   // cout << "Step 13\n";
   plot_analysis();
   emit controls_updated();
}

void Data_Control_W::update_boundary_range(double val)
{
   bd_range = val;
   position_counter->setRange(0, 100-bd_range, 0.1);
   bd_position = (100-bd_range)/2;
   position_counter->setValue((100-bd_range)/2);
   emit controls_updated();
   //   update_boundary_position(bd_position);
}

void Data_Control_W::update_single(double val)
{
   if (step == 0)
   {
      cnt_exsingle->setValue(0);
      QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n      Click on \"Load Data\"\n"));
      return;
   }
   unsigned int i;
   cnt_exrange->setRange(val, cnt_exrange->maxValue(), 1);
   cnt_exrange->setValue(val);
   exclude_single = (unsigned int) val;
   edit_plot->clear();
   if (exclude_single == 0)
   {
      for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
      {
         curve[i] = edit_plot->insertCurve("Optical Density");
         edit_plot->setCurvePen(curve[i], yellow);
         edit_plot->setCurveData(curve[i], radius, absorbance[i], points);
      }
   }
   else
   {
      for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
      {
         curve[i] = edit_plot->insertCurve("Optical Density");
         if (i == exclude_single-1)
         {
            edit_plot->setCurvePen(curve[i], red);
            excludeSingleSignal(i);
         }
         else
         {
            edit_plot->setCurvePen(curve[i], yellow);
         }
         edit_plot->setCurveData(curve[i], radius, absorbance[i], points);
      }
   }
   edit_plot->replot();
   emit controls_updated();
}

void Data_Control_W::update_range(double val)
{
   if (step == 0)
   {
      cnt_exrange->setValue(0);
      QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n      Click on \"Load Data\"\n"));
      return;
   }
   unsigned int i;
   cnt_exrange->setValue(val);
   //cout << "passed variable: " << val << endl;
   exclude_range = (unsigned int) val;
   edit_plot->clear();
   for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
   {
      curve[i] = edit_plot->insertCurve("Optical Density");
      edit_plot->setCurvePen(curve[i], yellow);
      edit_plot->setCurveData(curve[i], radius, absorbance[i], points);
   }
   if (exclude_single > 0)
   {
      excludeRangeSignal(exclude_range-1);
      for (i= (unsigned int) exclude_single-1; i<exclude_range; i++)
      {
         edit_plot->setCurvePen(curve[i], red);
      }
   }
   edit_plot->replot();
   emit controls_updated();
}

void Data_Control_W::excludeSingleSignal(unsigned int)
{
}

void Data_Control_W::excludeRangeSignal(unsigned int)
{
}

void Data_Control_W::ex_single()
{
   if (step == 0 && GUI)
   {
      QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n      Click on \"Load Data\"\n"));
      return;
   }
   unsigned int i;
   exclude_flag = true;
   if (exclude_single == 0)
   {
      return;
   }
   for (i=(unsigned int) exclude_single-1; i<(unsigned int) run_inf.scans[selected_cell][selected_lambda]-1; i++)
   {
      absorbance[i]                           = absorbance[i+1];
      run_inf.time[selected_cell][selected_lambda][i]          = run_inf.time[selected_cell][selected_lambda][i+1];
      if (run_type == 1 || run_type == 3 || run_type == 5)
      {
         run_inf.plateau[selected_cell][selected_lambda][i]      = run_inf.plateau[selected_cell][selected_lambda][i+1];
      }
      run_inf.temperature[selected_cell][selected_lambda][i]   = run_inf.temperature[selected_cell][selected_lambda][i+1];
      run_inf.omega_s_t[selected_cell][selected_lambda][i]      = run_inf.omega_s_t[selected_cell][selected_lambda][i+1];
      run_inf.rpm[selected_cell][selected_lambda][i]            = run_inf.rpm[selected_cell][selected_lambda][i+1];
   }
   run_inf.scans[selected_cell][selected_lambda] --;
   exclude_single = 1;
   exclude_range = 1;
   if (GUI)
   {
      plot_edit();
      // cout << "Step 14\n";
      plot_analysis();
      cnt_exrange->setValue(1);
      cnt_exsingle->setValue(1);
      cnt_exsingle->setRange(1, run_inf.scans[selected_cell][selected_lambda], 1);
      cnt_exrange->setRange(1, run_inf.scans[selected_cell][selected_lambda], 1);
   }
   emit excludeUpdated();
   emit controls_updated();
}

void Data_Control_W::ex_range()
{
   if (step == 0)
   {
      QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n      Click on \"Load Data\"\n"));
      return;
   }
   unsigned int i, excluded_scans;
   exclude_flag = true;
   if (exclude_single == 0)
   {
      return;
   }
   excluded_scans = exclude_range - exclude_single + 1;
   for (i=exclude_single-1; i<run_inf.scans[selected_cell][selected_lambda]-excluded_scans; i++)
   {
      absorbance[i] = absorbance[i+excluded_scans];
      run_inf.time[selected_cell][selected_lambda][i]          = run_inf.time[selected_cell][selected_lambda][i+excluded_scans];
      if (run_type == 1 || run_type == 3 || run_type == 5)
      {
         run_inf.plateau[selected_cell][selected_lambda][i]      = run_inf.plateau[selected_cell][selected_lambda][i+excluded_scans];
      }
      run_inf.temperature[selected_cell][selected_lambda][i]   = run_inf.temperature[selected_cell][selected_lambda][i+excluded_scans];
      run_inf.omega_s_t[selected_cell][selected_lambda][i]      = run_inf.omega_s_t[selected_cell][selected_lambda][i+excluded_scans];
      run_inf.rpm[selected_cell][selected_lambda][i]            = run_inf.rpm[selected_cell][selected_lambda][i+excluded_scans];
   }
   run_inf.scans[selected_cell][selected_lambda] -= excluded_scans;
   run_inf.total_scans -= excluded_scans;
   plot_edit();
   cnt_exsingle->setRange(1, run_inf.scans[selected_cell][selected_lambda], 1);
   cnt_exrange->setRange(1, run_inf.scans[selected_cell][selected_lambda], 1);
   cnt_exrange->setValue(1);
   cnt_exsingle->setValue(1);
   exclude_single = 1;
   exclude_range = 1;
   // cout << "Step 15\n";
   plot_analysis();
   emit excludeUpdated();
   emit controls_updated();
}

void Data_Control_W::plot_edit()
{
   QColor *dgrey;
   dgrey = new QColor(90, 90, 90);
   QPen greyPen;
   greyPen.setColor(*dgrey);
   if (scan_copied)
   {
      cleanup_copied_scan();
   }
   QString str, header;
   unsigned int i, j, count;
   temp_radius = new double [points];
   temp_abs = new double [points];
   range=0;
   if (run_type == 1 || run_type == 3 || run_type == 5)
   {
      header = tr("Velocity Data for ");
   }
   if (run_type == 2 || run_type == 4 || run_type == 6)
   {
      header = tr("Equilibrium Data for ");
   }
   header.append(run_inf.run_id);
   edit_plot->setTitle(header);
   if (run_inf.exp_type.interference)
   {
      str.sprintf(tr("Fringes at %d nm"), run_inf.wavelength[selected_cell][selected_lambda]);
   }
   if (run_inf.exp_type.absorbance)
   {
      str.sprintf(tr("Absorbance at %d nm"), run_inf.wavelength[selected_cell][selected_lambda]);
   }

   edit_plot->setAxisTitle(QwtPlot::yLeft, str);
   edit_plot->clear();
   for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
   {
      if (included[i])
      {
         count = 0;
         j    = 0;
         if (run_type == 1 || run_type == 3 || run_type == 5)
         {
            lower_limit = run_inf.plateau[selected_cell][selected_lambda][i] * bd_position/100;
            upper_limit  = run_inf.plateau[selected_cell][selected_lambda][i] * bd_range/100 + lower_limit;
            //cout << "ulimit: " << upper_limit << ", llimit: " << lower_limit << ", plateau: " << run_inf.plateau[selected_cell][selected_lambda][i] * bd_range/100 << endl;
         }
         if (run_type == 2 || run_type == 4 || run_type == 6)
         {
            get_2d_limits(&absorbance, &lower_limit, &upper_limit, run_inf.scans[selected_cell][selected_lambda],
                          points, 0, 0); // just find the largest and smallest value in all of them, then scale the range
            range = upper_limit - lower_limit;
            lower_limit = lower_limit + (range * bd_position/100);
            upper_limit  = lower_limit + (range * bd_range/100);
         }
         while (absorbance[i][j] < lower_limit && j < points)
         {
            temp_radius[count] = radius[j];
            temp_abs[count]   = absorbance[i][j];
            //            cout << "Rad: " << temp_radius[count] << ", abs: " << temp_abs[count] << endl;
            j++;
            count++;
         }
         //         cout << i << endl;
         curve[i*3] = edit_plot->insertCurve("Optical Density");
         if (print_bw)
         {
            edit_plot->setCurvePen(curve[i*3], greyPen);
         }
         else
         {
            edit_plot->setCurvePen(curve[i*3], Qt::cyan);
         }
         edit_plot->setCurveData(curve[i*3], temp_radius, temp_abs, count);
         count = 0;
         while ((absorbance[i][j] < upper_limit) && (j < points-1))
         {
            temp_radius[count] = radius[j];
            temp_abs[count]   = absorbance[i][j];
            j++;
            count++;
         }
         curve[i*3+1] = edit_plot->insertCurve("Optical Density");
         if (print_bw)
         {
            edit_plot->setCurvePen(curve[i*3+1], Qt::white);
         }
         else
         {
            edit_plot->setCurvePen(curve[i*3+1], Qt::yellow);
         }
         edit_plot->setCurveData(curve[i*3+1], temp_radius, temp_abs, count);
         count = 0;
         while (j<points)
         {
            temp_radius[count] = radius[j];
            temp_abs[count]   = absorbance[i][j];
            j++;
            count++;
         }
         curve[i*3+2] = edit_plot->insertCurve("Optical Density");
         if (print_bw)
         {
            edit_plot->setCurvePen(curve[i*3+2], greyPen);
         }
         else
         {
            edit_plot->setCurvePen(curve[i*3+2], Qt::cyan);
         }
         edit_plot->setCurveData(curve[i*3+2], temp_radius, temp_abs, count);
      }
   }
   /*
     if (run_inf.wavelength[selected_cell][selected_lambda] == 999) // we have simulated data and need to limit the max y-range
     {
     edit_plot->setAxisScale(QwtPlot::yLeft, -0.05, run_inf.plateau[0][0][0] * 2);
     }
     else
     {
     edit_plot->setAxisAutoScale(QwtPlot::yLeft);
     }
   */
   edit_plot->setAxisAutoScale(QwtPlot::yLeft);
   scan_copied = true;
   edit_plot->replot();
   //edit_plot->updatePlot();      //no updatePlot() function in new version
}

void Data_Control_W::cleanup_copied_scan()
{
   delete [] temp_radius;
   delete [] temp_abs;
   scan_copied = false;
}


//
// virtual functions are listed below:
//

void Data_Control_W::print()
{
   if (step == 0)
   {
      QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n      Click on \"Load Data\"\n"));
      return;
   }
   QPrinter printer;
   bool plotFlag = true;   // true for analysis plot, false for edit plot
   bool change = true;   // true if it is allowed to select between edit and analysis plot
   US_SelectPlot *sp;
   if (run_type == 5 || run_type == 6) //if we are calling dtactrl from the print function in uswin
   {
      change = false;   // false because we only have the edit plot to show,
                        // and can't change the plot flag to analysis plot
      plotFlag = false; // print the edit plot only
   }
   sp = new US_SelectPlot(&plotFlag, &print_bw, &print_inv, change, "Analysis Plot", "Edited Data Plot");
   sp->exec();
   if (plotFlag)
   {
      //      plot_analysis();
      if   (printer.setup(0))
      {
         if (print_bw)
         {
            PrintFilter pf;
            pf.setOptions(QwtPlotPrintFilter::PrintTitle
                          |QwtPlotPrintFilter::PrintMargin
                          |QwtPlotPrintFilter::PrintLegend
                          |QwtPlotPrintFilter::PrintGrid);
            analysis_plot->print(printer, pf);
         }
         else if (print_inv)
         {
            PrintFilterDark pf;
            pf.setOptions(QwtPlotPrintFilter::PrintTitle
                          |QwtPlotPrintFilter::PrintMargin
                          |QwtPlotPrintFilter::PrintLegend
                          |QwtPlotPrintFilter::PrintGrid);
            analysis_plot->print(printer, pf);
         }
         else
         {
            analysis_plot->print(printer);
            /*            QPainter painter(&printer);
                          QRect *rect;
                          //                             left margin, top margin, width, height
                          rect = new QRect(50, 50, 300, 300);
                          analysis_plot->print(&painter, *rect);
            */
         }
      }
   }
   else
   {
      //      plot_edit();
      if   (printer.setup(0))
      {
         if (print_bw)
         {
            PrintFilter pf;
            pf.setOptions(QwtPlotPrintFilter::PrintTitle
                          |QwtPlotPrintFilter::PrintMargin
                          |QwtPlotPrintFilter::PrintLegend
                          |QwtPlotPrintFilter::PrintGrid);
            edit_plot->print(printer, pf);
         }
         else if (print_inv)
         {
            PrintFilterDark pf;
            pf.setOptions(QwtPlotPrintFilter::PrintTitle
                          |QwtPlotPrintFilter::PrintMargin
                          |QwtPlotPrintFilter::PrintLegend
                          |QwtPlotPrintFilter::PrintGrid);
            edit_plot->print(printer, pf);
         }
         else
         {
            edit_plot->print(printer);
         }
      }
   }
   print_bw = false;
   //   plot_analysis();
   //   plot_edit();
}

void Data_Control_W::getAnalysisPlotMousePressed(const QMouseEvent &e)
{
   // store position
   p1 = e.pos();
}

void Data_Control_W::getAnalysisPlotMouseReleased(const QMouseEvent &e)
{
   // store position
   p2 = e.pos();
}

void Data_Control_W::reset()
{
   plateaus_corrected = false;
   rad_corrected = false;
   if (smoothing_counter != NULL)
   {
      smoothing_counter->disconnect();
      smooth=1;
      smoothing_counter->setRange(1, 50, 1);
      smoothing_counter->setValue(1);
      update_smoothing(1.0);
      connect(smoothing_counter, SIGNAL(valueChanged(double)), SLOT(update_smoothing(double)));
   }
   if (range_counter != NULL)
   {
      //range_counter->disconnect();
      bd_range=90;
      //position_counter->disconnect();
      range_counter->setValue(bd_range);
      //connect(range_counter, SIGNAL(valueChanged(double)), SLOT(update_boundary_range(double)));
      //connect(position_counter, SIGNAL(valueChanged(double)), SLOT(update_boundary_position(double)));
   }
}

int Data_Control_W::plot_analysis()
{
   return (0);
}

int Data_Control_W::setups()
{
   return (0);
}

void Data_Control_W::view()
{
}

void Data_Control_W::updateButtons()
{
}

void Data_Control_W::help()
{
}

void Data_Control_W::save()
{
}

void Data_Control_W::second_plot()
{
}
