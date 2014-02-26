#include "../include/us_reorder.h"

US_ReOrder::US_ReOrder(QWidget *parent, const char* name) : QFrame(parent, name)
{
   
   USglobal = new US_Config();
   QString item;
   int i, minHeight1 = 26, minHeight2 = 30;
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));

   //   data_dir = new QDir;
   data_type = 0;
   setCaption(tr("Re-order Scanfile Sequence"));

   lbl_header = new QLabel(tr("Scanfile Re-ordering Utility"), this);
   lbl_header->setAlignment(AlignCenter|AlignVCenter);
   lbl_header->setFrameStyle(QFrame::WinPanel|Raised);
   lbl_header->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_header->setMinimumHeight(minHeight2);
   lbl_header->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   cmb_datatype = new QComboBox(false, this, "Data Type" );
   cmb_datatype->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cmb_datatype->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cmb_datatype->setSizeLimit(5);
   cmb_datatype->setMinimumHeight(minHeight2);
   cmb_datatype->insertItem("Absorbance Data", -1);
   cmb_datatype->insertItem("Interference Data", -1);
   cmb_datatype->insertItem("Fluorescence Data", -1);
   cmb_datatype->setCurrentItem(0);
   connect(cmb_datatype, SIGNAL(activated(int)), this, SLOT(update_datatype(int)));

   pb_select_dir = new QPushButton(tr("Select Directory"), this);
   Q_CHECK_PTR(pb_select_dir);
   pb_select_dir->setAutoDefault(false);
   pb_select_dir->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_select_dir->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_select_dir->setMinimumHeight(minHeight1);
   connect(pb_select_dir, SIGNAL(clicked()), SLOT(select_dir()));

   lbl_directory = new QLabel(tr("<not selected>"),this);
   lbl_directory->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_directory->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_directory->setMinimumHeight(minHeight1);
   lbl_directory->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setAutoDefault(false);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_help->setMinimumHeight(minHeight1);
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   lbl_cells1 = new QLabel(tr("Total Cells:"),this);
   lbl_cells1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_cells1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_cells1->setMinimumHeight(minHeight1);
   lbl_cells1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_cells2 = new QLabel(tr("<not selected>"),this);
   lbl_cells2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_cells2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_cells2->setMinimumHeight(minHeight1);
   lbl_cells2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   lbl_line1 = new QLabel("", this);
   lbl_line1->setFrameStyle(QFrame::WinPanel|Raised);
   lbl_line1->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_line1->setMinimumHeight(minHeight1);

   lbl_cell_details = new QLabel(tr("Please Select a Cell:"),this);
   lbl_cell_details->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_cell_details->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_cell_details->setMinimumHeight(minHeight1);
   lbl_cell_details->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_progress = new QLabel(tr("Status:"),this);
   lbl_progress->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_progress->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_progress->setMinimumHeight(minHeight1);
   lbl_progress->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   pgb_progress = new QProgressBar(this, "iteration progress");
   pgb_progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   pgb_progress->setMinimumHeight(minHeight2);

   selected_cell = 0;
   lb_cells = new QListBox(this, "Cell");
   lb_cells->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   for (i=0; i<8; i++)
   {
      item.sprintf(tr("Cell %d is empty"), i+1);
      lb_cells->insertItem(item);
   }
   lb_cells->setSelected(0, TRUE);
   lb_cells->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   connect(lb_cells, SIGNAL(highlighted(int)), SLOT(show_cell(int)));

   lbl_message = new QLabel(tr("Please select a Directory by\nclicking on \"Select Directory\" now."),this);
   lbl_message->setAlignment(AlignCenter|AlignVCenter);
   lbl_message->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_message->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_message->setMinimumHeight(minHeight1);
   lbl_message->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   lbl_info1 = new QLabel(tr("Cell Contents:"),this);
   lbl_info1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_info1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_info1->setMinimumHeight(minHeight1);
   lbl_info1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_info2 = new QLabel(tr("<not selected>"),this);
   lbl_info2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_info2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_info2->setMinimumHeight(minHeight1);
   lbl_info2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   lbl_line2 = new QLabel("", this);
   lbl_line2->setFrameStyle(QFrame::WinPanel|Raised);
   lbl_line2->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_line2->setMinimumHeight(minHeight1);

   pb_check_lambda = new QPushButton(tr("Check Wavelength(s)"), this);
   Q_CHECK_PTR(pb_check_lambda);
   pb_check_lambda->setAutoDefault(false);
   pb_check_lambda->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_check_lambda->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_check_lambda->setMinimumHeight(minHeight1);
   connect(pb_check_lambda, SIGNAL(clicked()), SLOT(check_lambda()));

   lbl_lambda1 = new QLabel(tr("Wavelength(s):"),this);
   lbl_lambda1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_lambda1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_lambda1->setMinimumHeight(minHeight1);
   lbl_lambda1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_lambda2 = new QLabel(tr("<not checked>"),this);
   lbl_lambda2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_lambda2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_lambda2->setMinimumHeight(minHeight1);
   lbl_lambda2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   pb_order_cell = new QPushButton(tr("Order This Cell"), this);
   Q_CHECK_PTR(pb_order_cell);
   pb_order_cell->setAutoDefault(false);
   pb_order_cell->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_order_cell->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_order_cell->setMinimumHeight(minHeight1);
   connect(pb_order_cell, SIGNAL(clicked()), SLOT(order_cell()));

   lbl_scans1 = new QLabel(tr("Total Scans:"),this);
   lbl_scans1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_scans1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_scans1->setMinimumHeight(minHeight1);
   lbl_scans1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_scans2 = new QLabel(tr("<not selected>"),this);
   lbl_scans2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_scans2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_scans2->setMinimumHeight(minHeight1);
   lbl_scans2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   pb_order_all = new QPushButton(tr("Order All Cells"), this);
   Q_CHECK_PTR(pb_order_all);
   pb_order_all->setAutoDefault(false);
   pb_order_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_order_all->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_order_all->setMinimumHeight(minHeight1);
   connect(pb_order_all, SIGNAL(clicked()), SLOT(order_all()));

   lbl_first_scan1 = new QLabel(tr("First Scanfile:"),this);
   lbl_first_scan1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_first_scan1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_first_scan1->setMinimumHeight(minHeight1);
   lbl_first_scan1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_first_scan2 = new QLabel(tr("<not selected>"),this);
   lbl_first_scan2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_first_scan2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_first_scan2->setMinimumHeight(minHeight1);
   lbl_first_scan2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   pb_quit = new QPushButton(tr("Quit"), this);
   Q_CHECK_PTR(pb_quit);
   pb_quit->setAutoDefault(false);
   pb_quit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_quit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_quit->setMinimumHeight(minHeight1);
   connect(pb_quit, SIGNAL(clicked()), SLOT(quit()));

   lbl_last_scan1 = new QLabel(tr("Last Scanfile:"),this);
   lbl_last_scan1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_last_scan1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_last_scan1->setMinimumHeight(minHeight1);
   lbl_last_scan1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_last_scan2 = new QLabel(tr("<not selected>"),this);
   lbl_last_scan2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_last_scan2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_last_scan2->setMinimumHeight(minHeight1);
   lbl_last_scan2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   global_Xpos += 30;
   global_Ypos += 30;
   
   move(global_Xpos, global_Ypos);
   
   setup_GUI();
}

US_ReOrder::~US_ReOrder()
{
}

void US_ReOrder::setup_GUI()
{
   int j=0;
   QGridLayout * background = new QGridLayout(this,14,3,2);
   background->addMultiCellWidget(lbl_header,j,j,0,2);
   j++;
   background->addMultiCellWidget(cmb_datatype,j,j,0,2);
   j++;
   background->addWidget(pb_select_dir,j,0);
   background->addMultiCellWidget(lbl_directory,j,j,1,2);
   j++;
   background->addWidget(pb_help,j,0);
   background->addWidget(lbl_cells1,j,1);
   background->addWidget(lbl_cells2,j,2);
   j++;
   background->addMultiCellWidget(lbl_line1,j,j,0,2);
   j++;
   background->addWidget(lbl_cell_details,j,0);
   background->addWidget(lbl_progress,j,1);
   background->addWidget(pgb_progress,j,2);
   j++;
   background->addMultiCellWidget(lb_cells,j,j+3,0,0);
   background->addMultiCellWidget(lbl_message,j,j+3,1,2);
   j=j+4;
   background->addWidget(lbl_info1,j,0);
   background->addMultiCellWidget(lbl_info2,j,j,1,2);
   j++;
   background->addMultiCellWidget(lbl_line2,j,j,0,2);
   j++;
   background->addWidget(pb_check_lambda,j,0);
   background->addWidget(lbl_lambda1,j,1);
   background->addWidget(lbl_lambda2,j,2);
   j++;
   background->addWidget(pb_order_cell,j,0);
   background->addWidget(lbl_scans1,j,1);
   background->addWidget(lbl_scans2,j,2);
   j++;
   background->addWidget(pb_order_all,j,0);
   background->addWidget(lbl_first_scan1,j,1);
   background->addWidget(lbl_first_scan2,j,2);
   j++;
   background->addWidget(pb_quit,j,0);
   background->addWidget(lbl_last_scan1,j,1);
   background->addWidget(lbl_last_scan2,j,2);

   
}

void US_ReOrder::closeEvent(QCloseEvent *e)
{
   e->accept();
   global_Xpos -= 30;
   global_Ypos -= 30;
}

void US_ReOrder::select_dir()
{
   int i, j, cell_count, count;
   unsigned int scans_upper = 0, scans_lower = 0;
   QFileDialog *fd;
   QFile f;
   bool out_of_sequence = false;
   QString trashcan, item, message;
   QString directory="";
   reset();
   QString str, str1, str2, str3;
   switch (data_type)
   {
   case 0:
      {
         extension1 = "RA";
         extension2 = "ra";
         break;
      }
   case 1:
      {
         extension1 = "IP";
         extension2 = "ip";
         break;
      }
   case 2:
      {
         extension1 = "FI";
         extension2 = "fi";
         break;
      }
   }
   for (i=0; i<8; i++)
   {
      scan_count[i] = 0;
   }
   str = "*." + extension1 + "?, *." + extension2 + "?";
   fd = new QFileDialog(USglobal->config_list.data_dir, str, 0, 0, TRUE);
   directory = fd->getExistingDirectory(USglobal->config_list.data_dir, 0, 0, "AUC Data", false, true);
   lbl_directory->setText(directory);
   message = tr("The following cells of this run\n contain out-of-sequence scans:\nCell ");
   if (!directory.isEmpty())
   {
      data_dir.setPath(directory);
      extension = extension1; // first count the files with uppercase extensions
      for (i=0; i<8; i++)      // find the number of scan files for each cell
      { 
         str.sprintf("*." + extension + "%d", i+1);
         data_dir.setNameFilter(str);
         scans_upper += data_dir.count();
      }
      extension = extension2; // next, count the files with lowercase extensions
      for (i=0; i<8; i++)      // find the number of scan files for each cell
      { 
         str.sprintf("*." + extension + "%d", i+1);
         data_dir.setNameFilter(str);
         scans_lower += data_dir.count();
      }
      if (scans_upper >= scans_lower) // this system does not allow for mixed case scenarios!
      {
         extension = extension1;
      }
      else
      {
         extension = extension2;
      }
      for (i=0; i<8; i++)
      {
         str = "*." + extension;
         str1.sprintf("%d", i+1);
         str.append(str1);
         data_dir.setNameFilter(str);
         scan_count[i] = data_dir.count();
      }
      cell_count = 0;
      for (i=0; i<8; i++)
      {
         if(scan_count[i] > 0)
         {
            item.sprintf(tr("Cell %d contains data"), i+1);
            lb_cells->changeItem(item, i);
            cell_count++;
         }
         str.sprintf(tr("%d Cells with Data"), cell_count);
         lbl_cells2->setText(str);
      }
      i=0;
      while (scan_count[i] < 1)
      {
         i++;
      }
      if (i > 7)
      {
         directory = "";
         lbl_message->setText(tr("Sorry - this directory doesn't contain\nany scanfiles.\nPlease try again..."));
         lbl_first_scan2->setText(tr("<not selected>"));
         lbl_scans2->setText(tr("<not selected>"));
         lbl_cells2->setText(tr("<not selected>"));
         lbl_directory->setText(tr("<not selected>"));
         lbl_last_scan2->setText(tr("<not selected>"));
         lbl_lambda2->setText(tr("<not checked>"));
         for (i=0; i<8; i++)
         {
            item.sprintf(tr("Cell %d is empty"), i+1);
            lb_cells->changeItem(item, i);
         }
         return;
      }
      else
      {
         lb_cells->setCurrentItem(i);
         for (j=0; j<8; j++)
         {
            if (scan_count[j] > 0)
            {
               str = "*.";
               str.append(extension);
               str1.sprintf("%d", j+1);
               str.append(str1);
               data_dir.setNameFilter(str);
               data_dir.setSorting(QDir::Name);
               str2 = data_dir[scan_count[j]-1];
               QString str3;
               if (data_type == 2)
               {
                  str3 = str2.mid(1,5);
               }
               else
               {
                  str3 = str2.left(5);
               }
               str2 = str3;
               count = str2.toInt();
               if (count != scan_count[j])
               {
                  out_of_sequence = true;
                  str3.sprintf("%d, ", j+1);
                  message.append(str3);
               }
            }
         }
         if (out_of_sequence)
         {
            message.truncate(message.length() - 2);
            lbl_message->setText(message);
         }
         else
         {
            lbl_message->setText(tr("All scanfiles of this run are in\n the proper sequence.\nNo reordering is necessary."));
         }
         show_cell(i);
      }
   }
   else
   {
      reset();
   }
}

void US_ReOrder::reset()
{
   unsigned int i;
   QString item;
   lbl_message->setText(tr("Please select a Directory by\nclicking on \"Select Directory\" now."));
   lbl_first_scan2->setText(tr("<not selected>"));
   lbl_scans2->setText(tr("<not selected>"));
   lbl_cells2->setText(tr("<not selected>"));
   lbl_directory->setText(tr("<not selected>"));
   lbl_last_scan2->setText(tr("<not selected>"));
   lbl_lambda2->setText(tr("<not checked>"));
   for (i=0; i<8; i++)
   {
      item.sprintf(tr("Cell %d is empty"), i+1);
      lb_cells->changeItem(item, i);
   }
}

void US_ReOrder::update_datatype(int val)
{
   data_type = val;
}

void US_ReOrder::show_cell(int cell)
{
   QString str, str1, filename;
   lbl_lambda2->setText(tr("<not checked>"));
   selected_cell = cell;
   str.sprintf(tr("%d Scans in Cell %d"), scan_count[selected_cell], selected_cell+1);
   lbl_scans2->setText(str);
   str = "*.";
   str.append(extension);
   str1.sprintf("%d", selected_cell+1);
   str.append(str1);
   data_dir.setNameFilter(str);
   data_dir.setSorting(QDir::Name);
   filename = data_dir[0];
   filename.prepend("/");
   filename.prepend(data_dir.absPath());
   QFile scanfile(filename);
   if (scanfile.open(IO_ReadOnly))
   {
      QTextStream ts(&scanfile);
      if (!ts.eof())
      {
         lbl_info2->setText(ts.readLine());
      }
      scanfile.close();
   }
   else
   {
      lbl_info2->setText(tr("Could not read the first file of this cell"));
   }
   if (scan_count[selected_cell] > 0)
   {
      lbl_first_scan2->setText(data_dir[0]);
      flo_channel = data_dir[0].left(1);
      lbl_last_scan2->setText(data_dir[scan_count[selected_cell]-1]);
   }
   else
   {
      lbl_first_scan2->setText(tr("no Files"));
      lbl_last_scan2->setText(tr("no Files"));
   }
}

void US_ReOrder::order_cell()
{
   QString source, s1, s2, target, temp, command;
   int i;
   pgb_progress->setTotalSteps(scan_count[selected_cell]);
   pgb_progress->reset();
   temp.sprintf(tr("Ordering Scanfiles for:\nCell %d"), selected_cell+1);
   lbl_message->setText(temp);
   for (i=1; i<scan_count[selected_cell]+1; i++)
   {
      s1 = "*.";
      s1.append(extension);
      s2.sprintf("%d", selected_cell+1);
      s1.append(s2);
      data_dir.setNameFilter(s1);
      data_dir.setSorting(QDir::Name);
      source = data_dir[i-1];
      source.prepend("/");
      source.prepend(data_dir.absPath());
      if (i < 10)
      {
         if (data_type == 2)
         {
            target.sprintf(flo_channel + "0000%d.", i);
         }
         else
         {
            target.sprintf("0000%d.", i);
         }
      }
      else if ( i > 9 && i < 100)
      {
         if (data_type == 2)
         {
            target.sprintf(flo_channel + "000%d.", i);
         }
         else
         {
            target.sprintf("000%d.", i);
         }
      }
      else if (i > 99 && i < 1000)
      {
         if (data_type == 2)
         {
            target.sprintf(flo_channel + "00%d.", i);
         }
         else
         {
            target.sprintf("00%d.", i);
         }
      }
      else if (i > 999 && i < 10000)
      {
         if (data_type == 2)
         {
            target.sprintf(flo_channel + "0%d.", i);
         }
         else
         {
            target.sprintf("0%d.", i);
         }
      }
      else if (i > 9999)
      {
         if (data_type == 2)
         {
            target.sprintf(flo_channel + "%d.", i);
         }
         else
         {
            target.sprintf("%d.", i);
         }
      }
      target.append(extension);
      target.append(s2);
      target.prepend("/");
      target.prepend(data_dir.absPath());
      if (source != target)
      {
         Move(source, target);   
      }
      pgb_progress->setProgress(i);
      qApp->processEvents();
   }
   show_cell(selected_cell);
   lbl_message->setText(tr("Ordering completed"));
}

void US_ReOrder::order_all()
{
   QString source, s1, s2, target, temp, command;
   int i, j, all_scans=0, count;

   for (i=0; i<8; i++)
   {
      all_scans += scan_count[i];
   }
   pgb_progress->setTotalSteps(all_scans);
   pgb_progress->reset();
   count = 0;
   for (j=0; j<8; j++)
   {
      temp.sprintf(tr("Ordering Scanfiles for:\nCell %d"), j+1);
      lbl_message->setText(temp);
      for (i=1; i<scan_count[j]+1; i++)
      {
         count++;
         s1 = "*.";
         s1.append(extension);
         s2.sprintf("%d", j+1);
         s1.append(s2);
         data_dir.setNameFilter(s1);
         data_dir.setSorting(QDir::Name);
         source = data_dir[i-1];
         source.prepend("/");
         source.prepend(data_dir.absPath());
         if (i < 10)
         {
            target.sprintf("0000%d.", i);
         }
         else if ( i > 9 && i < 100)
         {
            target.sprintf("000%d.", i);
         }
         else if (i > 99 && i < 1000)
         {
            target.sprintf("00%d.", i);
         }
         else if (i > 999 && i < 10000)
         {
            target.sprintf("0%d.", i);
         }
         else if (i > 9999)
         {
            target.sprintf("%d.", i);
         }
         target.append(extension);
         target.append(s2);
         target.prepend("/");
         target.prepend(data_dir.absPath());
         if (source != target)
         {
            Move(source, target);
         }
         pgb_progress->setProgress(count);
         qApp->processEvents();
      }
   }
   show_cell(selected_cell);
   lbl_message->setText(tr("Ordering completed"));
}

void US_ReOrder::check_lambda()
{
   QString str, str1, filename, label, trashcan;
   int lambda[3], temp, i, j, count=0;;
   bool equal;
   QFile scanfile;

   str = "*.";
   str.append(extension);
   str1.sprintf("%d", selected_cell+1);
   str.append(str1);
   data_dir.setNameFilter(str);
   data_dir.setSorting(QDir::Name);
   pgb_progress->setTotalSteps(scan_count[selected_cell]);
   pgb_progress->reset();

   for (i=0; i<scan_count[selected_cell]; i++)
   {
      filename = data_dir[i];
      filename.prepend("/");
      filename.prepend(data_dir.absPath());
      scanfile.setName(filename);
      if (scanfile.open(IO_ReadOnly))
      {
         QTextStream ts(&scanfile);
         if (!ts.eof())
         {
            ts.readLine();
            ts >> trashcan;   // radial or wavelength scan mode
            ts >> trashcan;   // cell number
            ts >> trashcan;   // temperature
            ts >> trashcan;   //   rotor speed
            ts >> trashcan;   // time in secs
            ts >> trashcan;   // omega-squared-t
            ts >> temp;         // wavelength
         }
         if (i == 0)
         {
            lambda[0] = temp;
            count++;
            label.sprintf("%d nm", temp);
         }
         else if (count < 3)
         {
            equal = false;
            for (j=0; j<=count; j++)
            {
               if (temp == lambda[j])
               {
                  equal = true;
               }
            }
            if (!equal)
            {
               str.sprintf(", %d nm", temp);
               label.append(str);
               lambda[count] = temp;
               count++;
            }
         }
         scanfile.close();
      }
      pgb_progress->setProgress(i+1);
      qApp->processEvents();      // check to see if there is any other pressing business that can't wait
   }
   if (count > 1)
   {
      lbl_message->setText(tr("This cell contains data which was\nmeasured at multiple wavelengths!\nIt is not safe to re-order this cell!"));
   }
   else
   {
      lbl_message->setText(tr("This cell was measured at a single\nwavelength.\nIt is safe to re-order this cell."));
   }
   lbl_lambda2->setText(label);
}

void US_ReOrder::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/reorder.html");
}

void US_ReOrder::quit()
{
   close();
}

