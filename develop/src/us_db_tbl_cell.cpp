#include "../include/us_db_tbl_cell.h"

//! Constructor
/*!
  Constractor a new <var>US_Cell_DB</var> interface,
  with <var>parent</var> as a parent and <var>us_tblcell</var> as object name.
  \param temp_exp_info a struct variable of US_ExpData.
  \param temp_cell_table_unfinished An integer variable, if it is great than 0, there are still have cell tables unfinished.
  \param parent Parent widget.
  \param name Widget name.

*/
US_Cell_DB::US_Cell_DB(struct US_ExpData temp_exp_info, int *temp_cell_table_unfinished, QWidget *parent, const char *name) :US_DB(parent, name)
{
   int border=4, spacing=2;
   int xpos = border, ypos = border;
   int buttonw = 150, buttonh = 26;
   QString str;

   from_cell = true;
   for(int i=0; i<4; i++)
   {
      cell_info.BufferID[i] = -1;
      for(int j=0; j<3; j++)
      {
         cell_info.DNAID[i][j] = -1;
         cell_info.PepID[i][j] = -1;
      }
   }
   for(int i=0; i<3; i++)
   {
      cell_info.Wavelength[i] = 0;
      cell_info.Scans[i] = 0;
   }

   exp_info = temp_exp_info;
   cell_table_unfinished = temp_cell_table_unfinished;
   cell_info.ExperimentID = exp_info.ExpdataID;
   cell_info.Position = exp_info.cell_position;
   cell_info.CenterpieceID = exp_info.centerpiece[exp_info.cell_position];
   cp_info_vector.clear();
   readCenterpieceInfo(&cp_info_vector);
   cell_info.InvID = exp_info.Invid;
   cell_info.Description = exp_info.Cell[exp_info.cell_position];
   cell_info.CellID = exp_info.CellID[exp_info.cell_position];
   for(int i=0; i<3; i++)
   {
      cell_info.Wavelength[i] = exp_info.wavelength[exp_info.cell_position][i];
      cell_info.Scans[i] = exp_info.scans[exp_info.cell_position][i];
   }

   select_flag = false;         //use to control select query listbox
   from_db = false;            //use to control delete function
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));

   lbl_sample = new QLabel(tr(" Cell Table ID:"),this);
   lbl_sample->setAlignment(AlignLeft|AlignVCenter);
   lbl_sample->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_sample->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_sample->setGeometry(xpos, ypos, buttonw, buttonh );

   xpos += buttonw + spacing;
   lbl_cellid= new QLabel("",this);
   lbl_cellid->setAlignment(AlignLeft|AlignVCenter);
   lbl_cellid->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_cellid->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_cellid->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   if(cell_info.CellID==0)
   {
      cell_info.CellID = get_newID("tblCell", "CellID");
   }
   lbl_cellid->setNum(cell_info.CellID);

   xpos += buttonw + spacing;
   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setAutoDefault(false);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_help->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   xpos += buttonw + spacing;
   pb_close = new QPushButton(tr("Close"), this);
   pb_close->setAutoDefault(false);
   pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_close->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_close->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_close, SIGNAL(clicked()), SLOT(quit()));

   xpos = border;
   ypos +=buttonh + spacing;
   lbl_investigator = new QLabel(tr(" Investigator ID:"),this);
   lbl_investigator->setAlignment(AlignLeft|AlignVCenter);
   lbl_investigator->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_investigator->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_investigator->setGeometry(xpos, ypos, buttonw, buttonh );

   xpos += buttonw + spacing;
   lbl_invest= new QLabel("",this);
   lbl_invest->setAlignment(AlignLeft|AlignVCenter);
   lbl_invest->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_invest->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_invest->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   if(cell_info.InvID == -1)
   {
      lbl_invest->setText("No ID be selected");
   }
   else
   {
      lbl_invest->setNum(cell_info.InvID);
   }
   xpos += buttonw + spacing;
   pb_add = new QPushButton(tr("Add DB Entry"), this);
   pb_add->setAutoDefault(false);
   pb_add->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_add->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_add->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_add, SIGNAL(clicked()), SLOT(add_db()));

   xpos += buttonw + spacing;
   pb_delete = new QPushButton(tr("Delete DB Entry"), this);
   pb_delete->setAutoDefault(false);
   pb_delete->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_delete->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_delete->setGeometry(xpos, ypos, buttonw, buttonh);
   pb_delete->setEnabled(false);
   connect(pb_delete, SIGNAL(clicked()), SLOT(check_permission()));

   xpos = border;
   ypos += buttonh + spacing;
   lbl_centerpiece = new QLabel(tr(" Centerpiece ID:"),this);
   lbl_centerpiece->setAlignment(AlignLeft|AlignVCenter);
   lbl_centerpiece->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_centerpiece->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_centerpiece->setGeometry(xpos, ypos, buttonw, buttonh );

   xpos += buttonw + spacing;
   lbl_ctpc= new QLabel("",this);
   lbl_ctpc->setAlignment(AlignLeft|AlignVCenter);
   lbl_ctpc->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_ctpc->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_ctpc->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_ctpc->setNum(cell_info.CenterpieceID);
   str.sprintf(cp_info_vector[cell_info.CenterpieceID].material + " %d-Channel (%d)",
               cp_info_vector[cell_info.CenterpieceID].channels * 2, cp_info_vector[cell_info.CenterpieceID].sector);
   cell_info.Num_Channel = cp_info_vector[cell_info.CenterpieceID].channels;
   lbl_ctpc->setText(str);

   xpos += buttonw + spacing;
   pb_reset = new QPushButton(tr("Reset"), this);
   pb_reset->setAutoDefault(false);
   pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_reset->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_reset->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));

   xpos += buttonw + spacing;
   pb_query = new QPushButton(tr("Query DB Entry"), this);
   pb_query->setAutoDefault(false);
   pb_query->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_query->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_query->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_query, SIGNAL(clicked()), SLOT(query_db()));

   xpos = border;
   ypos += buttonh + spacing;
   pb_buffer = new QPushButton(tr("Buffer for Channel"), this);
   Q_CHECK_PTR(pb_buffer);
   pb_buffer->setAutoDefault(false);
   pb_buffer->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_buffer->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_buffer->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_buffer, SIGNAL(clicked()), SLOT(sel_buffer()));

   xpos += buttonw + spacing;
   //   le_buffer= new QLineEdit(this, "buffer");
   le_buffer = new QLabel("not selected",this);
   le_buffer->setAlignment(AlignLeft|AlignVCenter);
   le_buffer->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   le_buffer->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_buffer->setGeometry(xpos, ypos, buttonw, buttonh);
   //   le_buffer->setReadOnly(true);
   //   le_buffer->setText(tr("not selected"));

   xpos += buttonw + spacing;
   lb_query = new QListBox(this, "Query");
   lb_query->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lb_query->setGeometry(xpos, ypos, buttonw*2+spacing, buttonh*6+spacing*5);
   connect(lb_query, SIGNAL(selected(int)), SLOT(sel_query(int)));

   xpos = border;
   ypos += buttonh + spacing;
   pb_peptide1 = new QPushButton(tr("Peptide 1 for Channel"), this);
   Q_CHECK_PTR(pb_peptide1);
   pb_peptide1->setAutoDefault(false);
   pb_peptide1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_peptide1->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_peptide1->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_peptide1, SIGNAL(clicked()), SLOT(sel_vbar1()));

   xpos += buttonw + spacing;
   //   le_peptide1= new QLineEdit(this, "peptide1");
   le_peptide1 = new QLabel("not selected",this);
   le_peptide1->setAlignment(AlignLeft|AlignVCenter);
   le_peptide1->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   le_peptide1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_peptide1->setGeometry(xpos, ypos, buttonw, buttonh);
   //   le_peptide1->setReadOnly(true);
   //   le_peptide1->setText(tr("not selected"));

   xpos = border;
   ypos += buttonh + spacing;
   pb_peptide2 = new QPushButton(tr("Peptide 2 for Channel"), this);
   Q_CHECK_PTR(pb_peptide2);
   pb_peptide2->setAutoDefault(false);
   pb_peptide2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_peptide2->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_peptide2->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_peptide2, SIGNAL(clicked()), SLOT(sel_vbar2()));

   xpos += buttonw + spacing;
   //   le_peptide2= new QLineEdit(this, "peptide2");
   le_peptide2 = new QLabel("not selected",this);
   le_peptide2->setAlignment(AlignLeft|AlignVCenter);
   le_peptide2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   le_peptide2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_peptide2->setGeometry(xpos, ypos, buttonw, buttonh);
   //   le_peptide2->setReadOnly(true);
   //   le_peptide2->setText(tr("not selected"));

   xpos = border;
   ypos += buttonh + spacing;
   pb_peptide3 = new QPushButton(tr("Peptide 3 for Channel"), this);
   Q_CHECK_PTR(pb_peptide3);
   pb_peptide3->setAutoDefault(false);
   pb_peptide3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_peptide3->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_peptide3->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_peptide3, SIGNAL(clicked()), SLOT(sel_vbar3()));

   xpos += buttonw + spacing;
   //   le_peptide3= new QLineEdit(this, "peptide3");
   le_peptide3 = new QLabel("not selected",this);
   le_peptide3->setAlignment(AlignLeft|AlignVCenter);
   le_peptide3->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   le_peptide3->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_peptide3->setGeometry(xpos, ypos, buttonw, buttonh);
   //   le_peptide3->setReadOnly(true);
   //   le_peptide3->setText(tr("not selected"));

   xpos = border;
   ypos += buttonh + spacing;
   pb_DNA1 = new QPushButton(tr("DNA 1 for Channel"), this);
   Q_CHECK_PTR(pb_DNA1);
   pb_DNA1->setAutoDefault(false);
   pb_DNA1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_DNA1->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_DNA1->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_DNA1, SIGNAL(clicked()), SLOT(sel_DNA1()));

   xpos += buttonw + spacing;
   //   le_DNA1= new QLineEdit(this, "DNA1");
   le_DNA1 = new QLabel("not selected",this);
   le_DNA1->setAlignment(AlignLeft|AlignVCenter);
   le_DNA1->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   le_DNA1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_DNA1->setGeometry(xpos, ypos, buttonw, buttonh);
   //   le_DNA1->setReadOnly(true);
   //   le_DNA1->setText(tr("not selected"));

   xpos = border;
   ypos += buttonh + spacing;
   pb_DNA2 = new QPushButton(tr("DNA 2 for Channel"), this);
   Q_CHECK_PTR(pb_DNA2);
   pb_DNA2->setAutoDefault(false);
   pb_DNA2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_DNA2->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_DNA2->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_DNA2, SIGNAL(clicked()), SLOT(sel_DNA2()));

   xpos += buttonw + spacing;
   //   le_DNA2= new QLineEdit(this, "DNA2");
   le_DNA2 = new QLabel("not selected",this);
   le_DNA2->setAlignment(AlignLeft|AlignVCenter);
   le_DNA2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   le_DNA2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_DNA2->setGeometry(xpos, ypos, buttonw, buttonh);
   //   le_DNA2->setReadOnly(true);
   //   le_DNA2->setText(tr("not selected"));

   xpos = border;
   ypos += buttonh + spacing;
   pb_DNA3 = new QPushButton(tr("DNA 3 for Channel"), this);
   Q_CHECK_PTR(pb_DNA3);
   pb_DNA3->setAutoDefault(false);
   pb_DNA3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_DNA3->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_DNA3->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_DNA3, SIGNAL(clicked()), SLOT(sel_DNA3()));

   xpos += buttonw + spacing;
   //   le_DNA3= new QLineEdit(this, "DNA3");
   le_DNA3 = new QLabel("not selected",this);
   le_DNA3->setAlignment(AlignLeft|AlignVCenter);
   le_DNA3->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   le_DNA3->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_DNA3->setGeometry(xpos, ypos, buttonw, buttonh);
   //   le_DNA3->setReadOnly(true);
   //   le_DNA3->setText(tr("not selected"));

   xpos += buttonw + spacing;
   lbl_experiment = new QLabel(tr(" Experiment ID:"),this);
   lbl_experiment->setAlignment(AlignLeft|AlignVCenter);
   lbl_experiment->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_experiment->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_experiment->setGeometry(xpos, ypos, buttonw, buttonh );

   xpos += buttonw + spacing;
   lbl_exp= new QLabel("",this);
   lbl_exp->setAlignment(AlignLeft|AlignVCenter);
   lbl_exp->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_exp->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_exp->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_exp->setNum(cell_info.ExperimentID);

   xpos = border;
   ypos += buttonh + spacing;
   lbl_channel = new QLabel(tr(" Channel:"),this);
   lbl_channel->setAlignment(AlignLeft|AlignVCenter);
   lbl_channel->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_channel->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_channel->setGeometry(xpos, ypos, buttonw, buttonh );

   xpos += buttonw + spacing;
   cmbb_channel = new QComboBox(false, this, "Channel Select");
   cmbb_channel->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cmbb_channel->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   cmbb_channel->setGeometry(xpos, ypos, buttonw, buttonh);
   for (int k=0; k<cell_info.Num_Channel; k++)
   {
      cmbb_channel->insertItem(str.sprintf("Channel %d", k+1));
   }
   connect(cmbb_channel, SIGNAL(activated(int)), SLOT(sel_channel(int)));

   xpos += buttonw + spacing;
   lbl_cell = new QLabel(tr(" Cell Position:"),this);
   lbl_cell->setAlignment(AlignLeft|AlignVCenter);
   lbl_cell->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_cell->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_cell->setGeometry(xpos, ypos, buttonw, buttonh );

   xpos += buttonw + spacing;
   lbl_position= new QLabel("",this);
   lbl_position->setAlignment(AlignLeft|AlignVCenter);
   lbl_position->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_position->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_position->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_position->setNum(cell_info.Position+1);

   xpos = border;
   ypos += buttonh + spacing;
   lbl_wavelength1 = new QLabel(tr(" Wavelenth 1:"),this);
   lbl_wavelength1->setAlignment(AlignLeft|AlignVCenter);
   lbl_wavelength1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_wavelength1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_wavelength1->setGeometry(xpos, ypos, buttonw, buttonh );

   xpos += buttonw + spacing;
   lbl_wl1= new QLabel("",this);
   lbl_wl1->setAlignment(AlignLeft|AlignVCenter);
   lbl_wl1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_wl1->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_wl1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   if(cell_info.Wavelength[0] == 0)
   {
      lbl_wl1->setText("No Data available");
   }
   else
   {
      lbl_wl1->setNum(cell_info.Wavelength[0]);
   }
   xpos += buttonw + spacing;
   lbl_wlscan1 = new QLabel(tr(" # of Scans for WL 1:"),this);
   lbl_wlscan1->setAlignment(AlignLeft|AlignVCenter);
   lbl_wlscan1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_wlscan1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_wlscan1->setGeometry(xpos, ypos, buttonw, buttonh );

   xpos += buttonw + spacing;
   lbl_scan1= new QLabel("",this);
   lbl_scan1->setAlignment(AlignLeft|AlignVCenter);
   lbl_scan1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_scan1->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_scan1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   if(cell_info.Scans[0] == 0)
   {
      lbl_scan1->setText("No Scans");
   }
   else
   {
      lbl_scan1->setNum(cell_info.Scans[0]);
   }

   xpos = border;
   ypos += buttonh + spacing;
   lbl_wavelength2 = new QLabel(tr(" Wavelenth 2:"),this);
   lbl_wavelength2->setAlignment(AlignLeft|AlignVCenter);
   lbl_wavelength2->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_wavelength2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_wavelength2->setGeometry(xpos, ypos, buttonw, buttonh );

   xpos += buttonw + spacing;
   lbl_wl2= new QLabel("",this);
   lbl_wl2->setAlignment(AlignLeft|AlignVCenter);
   lbl_wl2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_wl2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_wl2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   if(cell_info.Wavelength[1] == 0)
   {
      lbl_wl2->setText("No Data available");
   }
   else
   {
      lbl_wl2->setNum(cell_info.Wavelength[1]);
   }

   xpos += buttonw + spacing;
   lbl_wlscan2 = new QLabel(tr(" # of Scans for WL 2:"),this);
   lbl_wlscan2->setAlignment(AlignLeft|AlignVCenter);
   lbl_wlscan2->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_wlscan2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_wlscan2->setGeometry(xpos, ypos, buttonw, buttonh );

   xpos += buttonw + spacing;
   lbl_scan2= new QLabel("",this);
   lbl_scan2->setAlignment(AlignLeft|AlignVCenter);
   lbl_scan2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_scan2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_scan2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   if(cell_info.Scans[1] == 0)
   {
      lbl_scan2->setText("No Scans");
   }
   else
   {
      lbl_scan2->setNum(cell_info.Scans[1]);
   }

   xpos = border;
   ypos += buttonh + spacing;
   lbl_wavelength3 = new QLabel(tr(" Wavelenth 3:"),this);
   lbl_wavelength3->setAlignment(AlignLeft|AlignVCenter);
   lbl_wavelength3->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_wavelength3->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_wavelength3->setGeometry(xpos, ypos, buttonw, buttonh );

   xpos += buttonw + spacing;
   lbl_wl3= new QLabel("",this);
   lbl_wl3->setAlignment(AlignLeft|AlignVCenter);
   lbl_wl3->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_wl3->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_wl3->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   if(cell_info.Wavelength[2] == 0)
   {
      lbl_wl3->setText("No Data available");
   }
   else
   {
      lbl_wl3->setNum(cell_info.Wavelength[2]);
   }
   xpos += buttonw + spacing;
   lbl_wlscan3 = new QLabel(tr(" # of Scans for WL 3:"),this);
   lbl_wlscan3->setAlignment(AlignLeft|AlignVCenter);
   lbl_wlscan3->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_wlscan3->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_wlscan3->setGeometry(xpos, ypos, buttonw, buttonh );

   xpos += buttonw + spacing;
   lbl_scan3= new QLabel("",this);
   lbl_scan3->setAlignment(AlignLeft|AlignVCenter);
   lbl_scan3->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_scan3->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_scan3->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   if(cell_info.Scans[2] == 0)
   {
      lbl_scan3->setText("No Scans");
   }
   else
   {
      lbl_scan3->setNum(cell_info.Scans[2]);
   }

   ypos += buttonh + border;
   xpos = 2 * border + 3 * buttonw + buttonw + 3*spacing;

   global_Xpos += 30;
   global_Ypos += 30;

   setMinimumSize(xpos, ypos);
   setGeometry(global_Xpos, global_Ypos, xpos, ypos);
   setup_GUI();
}
//! Destructor
/*! destroy the <var>US_Cell_DB</var>. */
US_Cell_DB::~US_Cell_DB()
{
}

void US_Cell_DB::setup_GUI()
{
   QGridLayout * Grid = new QGridLayout(this,14,4,4,2);
   Grid->addWidget(lbl_sample,0,0);
   Grid->addWidget(lbl_cellid,0,1);
   Grid->addWidget(pb_help,0,2);
   Grid->addWidget(pb_close,0,3);
   Grid->addWidget(lbl_investigator,1,0);
   Grid->addWidget(lbl_invest,1,1);
   Grid->addWidget(pb_add,1,2);
   Grid->addWidget(pb_delete,1,3);
   Grid->addWidget(lbl_centerpiece,2,0);
   Grid->addWidget(lbl_ctpc,2,1);
   Grid->addWidget(pb_reset,2,2);
   Grid->addWidget(pb_query,2,3);
   Grid->addWidget(pb_buffer,3,0);
   Grid->addWidget(le_buffer,3,1);
   Grid->addWidget(pb_peptide1,4,0);
   Grid->addWidget(le_peptide1,4,1);
   Grid->addWidget(pb_peptide2,5,0);
   Grid->addWidget(le_peptide2,5,1);
   Grid->addWidget(pb_peptide3,6,0);
   Grid->addWidget(le_peptide3,6,1);
   Grid->addWidget(pb_DNA1,7,0);
   Grid->addWidget(le_DNA1,7,1);
   Grid->addWidget(pb_DNA2,8,0);
   Grid->addWidget(le_DNA2,8,1);
   Grid->addMultiCellWidget(lb_query,3,8,2,3);
   Grid->addWidget(pb_DNA3,9,0);
   Grid->addWidget(le_DNA3,9,1);
   Grid->addWidget(lbl_experiment,9,2);
   Grid->addWidget(lbl_exp,9,3);
   Grid->addWidget(lbl_channel,10,0);
   Grid->addWidget(cmbb_channel,10,1);
   Grid->addWidget(lbl_cell,10,2);
   Grid->addWidget(lbl_position,10,3);
   Grid->addWidget(lbl_wavelength1,11,0);
   Grid->addWidget(lbl_wl1,11,1);
   Grid->addWidget(lbl_wlscan1,11,2);
   Grid->addWidget(lbl_scan1,11,3);
   Grid->addWidget(lbl_wavelength2,12,0);
   Grid->addWidget(lbl_wl2,12,1);
   Grid->addWidget(lbl_wlscan2,12,2);
   Grid->addWidget(lbl_scan2,12,3);
   Grid->addWidget(lbl_wavelength3,13,0);
   Grid->addWidget(lbl_wl3,13,1);
   Grid->addWidget(lbl_wlscan3,13,2);
   Grid->addWidget(lbl_scan3,13,3);

   Grid->activate();
}
/*!
  Open a US_Nucleotide_DB interface for choosing DNA 1 information.
*/
void US_Cell_DB::sel_DNA1()
{
   US_Nucleotide_DB *DNA1_dlg;
   DNA1_dlg = new US_Nucleotide_DB(from_cell, cell_info.InvID);
   DNA1_dlg->setCaption("DNA 1 Information");
   DNA1_dlg->pb_quit->setText("Accept");
   DNA1_dlg->show();
   connect(DNA1_dlg, SIGNAL(IdChanged(int)), SLOT(update_DNA1_lbl(int)));
}

/*!
  Open a US_Nucleotide_DB interface for choosing DNA 2 information.
*/
void US_Cell_DB::sel_DNA2()
{
   US_Nucleotide_DB *DNA2_dlg;
   DNA2_dlg = new US_Nucleotide_DB(from_cell, cell_info.InvID);
   DNA2_dlg->setCaption("DNA 2 Information");
   DNA2_dlg->pb_quit->setText("Accept");
   DNA2_dlg->show();
   connect(DNA2_dlg, SIGNAL(IdChanged(int)), SLOT(update_DNA2_lbl(int)));
}

/*!
  Open a US_Nucleotide_DB interface for choosing DNA 3 information.
*/
void US_Cell_DB::sel_DNA3()
{
   US_Nucleotide_DB *DNA3_dlg;
   DNA3_dlg = new US_Nucleotide_DB(from_cell, cell_info.InvID);
   DNA3_dlg->setCaption("DNA 3 Information");
   DNA3_dlg->pb_quit->setText("Accept");
   DNA3_dlg->show();
   connect(DNA3_dlg, SIGNAL(IdChanged(int)), SLOT(update_DNA3_lbl(int)));
}

/*!
  After get the DNA info, this function update <var>le_DNA1</var>,
  show that DNA's DB entry number.
*/
void US_Cell_DB::update_DNA1_lbl(int DNAID)
{
   int item = cmbb_channel->currentItem();
   cell_info.DNAID[item][0] = DNAID;
   le_DNA1->setText(show_DNA(DNAID));
   if(cell_info.DNAID[item][0]<0)
   {
      le_DNA1->setText(tr("not selected"));
   }
}
/*!
  After get the DNA info, this function update <var>le_DNA2</var>,
  show that DNA's DB entry number.
*/
void US_Cell_DB::update_DNA2_lbl(int DNAID)
{
   int item = cmbb_channel->currentItem();
   cell_info.DNAID[item][1] = DNAID;
   le_DNA2->setText(show_DNA(DNAID));
   if(cell_info.DNAID[item][1]<0)
   {
      le_DNA2->setText(tr("not selected"));
   }
}
/*!
  After get the DNA info, this function update <var>le_DNA3</var>,
  show that DNA's DB entry number.
*/
void US_Cell_DB::update_DNA3_lbl(int DNAID)
{
   int item = cmbb_channel->currentItem();
   cell_info.DNAID[item][2] = DNAID;
   le_DNA3->setText(show_DNA(DNAID));
   if(cell_info.DNAID[item][2]<0)
   {
      le_DNA3->setText(tr("not selected"));
   }
}

/*!
  Open a US_Buffer_DB interface for choosing Buffer information.
*/
void US_Cell_DB::sel_buffer()
{
   buffer_dlg = new US_Buffer_DB(from_cell, cell_info.InvID);
   buffer_dlg->setCaption("Buffer Information");
   buffer_dlg->show();
   connect(buffer_dlg, SIGNAL(IDChanged(int)), SLOT(update_buffer_lbl(int)));
}

/*!
  After get the buffer info, this function update <var>le_buffer</var>,
  show that Buffer's DB entry number.
*/
void US_Cell_DB::update_buffer_lbl(int BuffID)
{
   int item = cmbb_channel->currentItem();
   cell_info.BufferID[item] = BuffID;
   le_buffer->setText(show_buffer(BuffID));
   if(cell_info.BufferID[item]<0)
   {
      le_buffer->setText(tr("not selected"));
   }
}

/*!
  Open a US_Vbar_DB interface for choosing Peptide1 information.
*/
void US_Cell_DB::sel_vbar1()
{
   float vbar = (float) 0.72, temp = 20, vbar20 = (float) 0.72;
   US_Vbar_DB *vbar1_dlg;
   vbar1_dlg = new US_Vbar_DB(temp, &vbar, &vbar20, true, from_cell, cell_info.InvID);
   vbar1_dlg->setCaption("Peptide 1 Information");
   vbar1_dlg->show();
   connect(vbar1_dlg, SIGNAL(idChanged(int)), SLOT(update_vbar1_lbl(int)));
}

/*!
  Open a US_Vbar_DB interface for choosing Peptide2 information.
*/
void US_Cell_DB::sel_vbar2()
{
   float vbar = (float) 0.72, temp = 20, vbar20 = (float) 0.72;
   US_Vbar_DB *vbar2_dlg;
   vbar2_dlg = new US_Vbar_DB(temp, &vbar, &vbar20, true, from_cell, cell_info.InvID);
   vbar2_dlg->setCaption("Peptide 2 Information");
   vbar2_dlg->show();
   connect(vbar2_dlg, SIGNAL(idChanged(int)), SLOT(update_vbar2_lbl(int)));
}

/*!
  Open a US_Vbar_DB interface for choosing Peptide3 information.
*/
void US_Cell_DB::sel_vbar3()
{
   float vbar = (float) 0.72, temp = 20, vbar20 = (float) 0.72;
   US_Vbar_DB *vbar3_dlg;
   vbar3_dlg = new US_Vbar_DB(temp, &vbar, &vbar20, true, from_cell, cell_info.InvID);
   vbar3_dlg->setCaption("Peptide 3 Information");
   vbar3_dlg->show();
   connect(vbar3_dlg, SIGNAL(idChanged(int)), SLOT(update_vbar3_lbl(int)));
}

/*!
  After get the peptide info, this function update <var>le_peptide1</var>,
  show that Peptide's DB entry number.
*/
void US_Cell_DB::update_vbar1_lbl(int PepID)
{
   int item = cmbb_channel->currentItem();
   cell_info.PepID[item][0] = PepID;
   le_peptide1->setText(show_peptide(PepID));
   if(cell_info.PepID[item][0]<0)
   {
      le_peptide1->setText(tr("not selected"));
   }
}
/*!
  After get the peptide info, this function update <var>le_peptide2</var>,
  show that Peptide's DB entry number.
*/
void US_Cell_DB::update_vbar2_lbl(int PepID)
{
   int item = cmbb_channel->currentItem();
   cell_info.PepID[item][1] = PepID;
   le_peptide2->setText(show_peptide(PepID));
   if(cell_info.PepID[item][1]<0)
   {
      le_peptide2->setText(tr("not selected"));
   }
}
/*!
  After get the peptide info, this function update <var>le_peptide3</var>,
  show that Peptide's DB entry number.
*/
void US_Cell_DB::update_vbar3_lbl(int PepID)
{
   int item = cmbb_channel->currentItem();
   cell_info.PepID[item][2] = PepID;
   le_peptide3->setText(show_peptide(PepID));
   if(cell_info.PepID[item][2]<0)
   {
      le_peptide3->setText(tr("not selected"));
   }
}

/*!
  Select channel for choose DNA, buffer and peptide info.
*/
void US_Cell_DB::sel_channel(int item)
{
   le_buffer->setText(show_buffer(cell_info.BufferID[item]));
   if(cell_info.BufferID[item]<0)
   {
      le_buffer->setText(tr("not selected"));
   }

   le_DNA1->setText(show_DNA(cell_info.DNAID[item][0]));
   if(cell_info.DNAID[item][0]<0)
   {
      le_DNA1->setText(tr("not selected"));
   }
   le_DNA2->setText(show_DNA(cell_info.DNAID[item][1]));
   if(cell_info.DNAID[item][1]<0)
   {
      le_DNA2->setText(tr("not selected"));
   }
   le_DNA3->setText(show_DNA(cell_info.DNAID[item][2]));
   if(cell_info.DNAID[item][2]<0)
   {
      le_DNA3->setText(tr("not selected"));
   }
   le_peptide1->setText(show_peptide(cell_info.PepID[item][0]));
   if(cell_info.PepID[item][0]<0)
   {
      le_peptide1->setText(tr("not selected"));
   }
   le_peptide2->setText(show_peptide(cell_info.PepID[item][1]));
   if(cell_info.PepID[item][1]<0)
   {
      le_peptide2->setText(tr("not selected"));
   }
   le_peptide3->setText(show_peptide(cell_info.PepID[item][2]));
   if(cell_info.PepID[item][2]<0)
   {
      le_peptide3->setText(tr("not selected"));
   }

}

/*!
  Save the cell info into DB table: <tt>tblCell</tt>.
*/
void US_Cell_DB::add_db()
{
   QString STR, str;
   bool unfinish = false;
   STR = tr("Some fields have not yet been selected:\n"
            "Are you sure you want to leave them blank?\n\n");
   for(int i=0; i<cell_info.Num_Channel; i++)
   {
      if(cell_info.BufferID[i]<0)
      {
         str.sprintf(tr("No Buffer data selected for Channel %d. \n"), i+1);
         STR.append(str);
         unfinish = true;
      }
      if(cell_info.PepID[i][0]<0)
      {
         str.sprintf(tr("No Peptide 1 data selected for Channel %d.\n"), i+1);
         STR.append(str);
         unfinish = true;
      }
      if(cell_info.PepID[i][1]<0)
      {
         str.sprintf(tr("No Peptide 2 data selected for Channel %d.\n"), i+1);
         STR.append(str);
         unfinish = true;
      }
      if(cell_info.PepID[i][2]<0)
      {
         str.sprintf(tr("No Peptide 3 data selected for Channel %d.\n"), i+1);
         STR.append(str);
         unfinish = true;
      }
      if(cell_info.DNAID[i][0]<0)
      {
         str.sprintf(tr("No DNA 1 data selected for Channel %d. \n"), i+1);
         STR.append(str);
         unfinish = true;
      }
      if(cell_info.DNAID[i][1]<0)
      {
         str.sprintf(tr("No DNA 2 data selected for Channel %d. \n"), i+1);
         STR.append(str);
         unfinish = true;
      }
      if(cell_info.DNAID[i][2]<0)
      {
         str.sprintf(tr("No DNA 3 data selected for Channel %d. \n"), i+1);
         STR.append(str);
         unfinish = true;
      }


   }
   STR.append("\nSelecting 'Yes' will set missing items to zero.");
   STR.append("\nSelecting 'No' will allow you to make a selection.");

   if(unfinish)
   {
      switch(QMessageBox::information(this, tr("Attention"), STR,
                                      tr("Yes"), tr("No"),   0,1))
      {
      case 0:
         {
            for(int i=0; i<cell_info.Num_Channel; i++)
            {
               if(cell_info.BufferID[i]<0)
               {
                  cell_info.BufferID[i]=0;
                  le_buffer->setText(show_buffer(cell_info.BufferID[i]));
               }
               if(cell_info.PepID[i][0]<0)
               {
                  cell_info.PepID[i][0]=0;
                  le_peptide1->setText(show_peptide(cell_info.PepID[i][0]));
               }
               if(cell_info.PepID[i][1]<0)
               {
                  cell_info.PepID[i][1]=0;
                  le_peptide2->setText(show_peptide(cell_info.PepID[i][1]));
               }
               if(cell_info.PepID[i][2]<0)
               {
                  cell_info.PepID[i][2]=0;
                  le_peptide3->setText(show_peptide(cell_info.PepID[i][2]));
               }

               if(cell_info.DNAID[i][0]<0)
               {
                  cell_info.DNAID[i][0] = 0;
                  le_DNA1->setText(show_DNA(cell_info.DNAID[i][0]));
               }
               if(cell_info.DNAID[i][1]<0)
               {
                  cell_info.DNAID[i][1] = 0;
                  le_DNA2->setText(show_DNA(cell_info.DNAID[i][1]));
               }
               if(cell_info.DNAID[i][2]<0)
               {
                  cell_info.DNAID[i][2] = 0;
                  le_DNA3->setText(show_DNA(cell_info.DNAID[i][2]));
               }


            }
            break;
         }
      case 1:
         {
            return;
         }
      }
   }

   QSqlCursor check( "tblCell");
	check.last();
   QString condition;
   condition.sprintf("CellID = %d", cell_info.CellID);
	cout << condition << endl;
   check.select(condition);
   if(check.next())
   {
      QMessageBox::message(tr("Attention:"),
                           tr("This Cell Table ID already exists \n"
                              "You need to delete the old one first!"));
      return;
   }
   /*
     QSqlCursor cur( "tblCell");
     cur.setMode( QSqlCursor::Insert);
     QSqlRecord *buffer = cur.primeInsert();
     buffer->setValue("CellID", cell_info.CellID);
     buffer->setValue("CenterpieceID", cell_info.CenterpieceID);
     buffer->setValue("Position", cell_info.Position);
     buffer->setValue("InvestigatorID", cell_info.InvID);
     buffer->setValue("Description", cell_info.Description);
     buffer->setValue("Wavelength_1", cell_info.Wavelength[0]);
     buffer->setValue("Wavelength_2", cell_info.Wavelength[1]);
     buffer->setValue("Wavelength_3", cell_info.Wavelength[2]);
     buffer->setValue("Wl_Scans_1", cell_info.Scans[0]);
     buffer->setValue("Wl_Scans_2", cell_info.Scans[1]);
     buffer->setValue("Wl_Scans_3", cell_info.Scans[2]);
     buffer->setValue("DNAID_1", cell_info.DNAID[0]);
     buffer->setValue("DNAID_2", cell_info.DNAID[1]);
     buffer->setValue("DNAID_3", cell_info.DNAID[2]);
     buffer->setValue("DNAID_4", cell_info.DNAID[3]);
     buffer->setValue("BufferID_1", cell_info.BufferID[0]);
     buffer->setValue("BufferID_2", cell_info.BufferID[1]);
     buffer->setValue("BufferID_3", cell_info.BufferID[2]);
     buffer->setValue("BufferID_4", cell_info.BufferID[3]);
     buffer->setValue("PeptideID_1", cell_info.PepID[0]);
     buffer->setValue("PeptideID_2", cell_info.PepID[1]);
     buffer->setValue("PeptideID_3", cell_info.PepID[2]);
     buffer->setValue("PeptideID_4", cell_info.PepID[3]);
     buffer->setValue("Num_Channel", cell_info.Num_Channel);
     buffer->setValue("ExperimentID", cell_info.ExperimentID);

     int cnt = cur.insert();
     cout<<"cnt = "<<cnt<<endl;
   */
   QSqlQuery target;
   STR = "INSERT INTO tblCell(CellID, CenterpieceID, Position, InvestigatorID, Description, Wavelength_1, Wavelength_2, Wavelength_3, Wl_Scans_1, Wl_Scans_2, Wl_Scans_3, ";
   if (cell_info.DNAID[0][0] > 0)
   {
      STR += "DNAID_1, ";
   }
   if (cell_info.DNAID[0][1] > 0)
   {
      STR += "DNA2ID_1, ";
   }
   if (cell_info.DNAID[0][2] > 0)
   {
      STR += "DNA3ID_1, ";
   }
   if (cell_info.DNAID[1][0] > 0)
   {
      STR += "DNAID_2, ";
   }
   if (cell_info.DNAID[1][1] > 0)
   {
      STR += "DNA2ID_2, ";
   }
   if (cell_info.DNAID[1][2] > 0)
   {
      STR += "DNA3ID_2, ";
   }
   if (cell_info.DNAID[2][0] > 0)
   {
      STR += "DNAID_3, ";
   }
   if (cell_info.DNAID[2][1] > 0)
   {
      STR += "DNA2ID_3, ";
   }
   if (cell_info.DNAID[2][2] > 0)
   {
      STR += "DNA3ID_3, ";
   }
   if (cell_info.DNAID[3][0] > 0)
   {
      STR += "DNAID_4, ";
   }
   if (cell_info.DNAID[3][1] > 0)
   {
      STR += "DNA2ID_4, ";
   }
   if (cell_info.DNAID[3][2] > 0)
   {
      STR += "DNA3ID_4, ";
   }
   if (cell_info.BufferID[0] > 0)
   {
      STR += "BufferID_1, ";
   }
   if (cell_info.BufferID[1] > 0)
   {
      STR += "BufferID_2, ";
   }
   if (cell_info.BufferID[2] > 0)
   {
      STR += "BufferID_3, ";
   }
   if (cell_info.BufferID[3] > 0)
   {
      STR += "BufferID_4, ";
   }
   if (cell_info.PepID[0][0] > 0)
   {
      STR += "PeptideID_1, ";
   }
   if (cell_info.PepID[0][1] > 0)
   {
      STR += "Peptide2ID_1, ";
   }
   if (cell_info.PepID[0][2] > 0)
   {
      STR += "Peptide3ID_1, ";
   }
   if (cell_info.PepID[1][0] > 0)
   {
      STR += "PeptideID_2, ";
   }
   if (cell_info.PepID[1][1] > 0)
   {
      STR += "Peptide2ID_2, ";
   }
   if (cell_info.PepID[1][2] > 0)
   {
      STR += "Peptide3ID_2, ";
   }
   if (cell_info.PepID[2][0] > 0)
   {
      STR += "PeptideID_3, ";
   }
   if (cell_info.PepID[2][1] > 0)
   {
      STR += "Peptide2ID_3, ";
   }
   if (cell_info.PepID[2][2] > 0)
   {
      STR += "Peptide3ID_3, ";
   }
   if (cell_info.PepID[3][0] > 0)
   {
      STR += "PeptideID_4, ";
   }
   if (cell_info.PepID[3][1] > 0)
   {
      STR += "Peptide2ID_4, ";
   }
   if (cell_info.PepID[3][2] > 0)
   {
      STR += "Peptide3ID_4, ";
   }
   STR += "Num_Channel, ExperimentID) VALUES (";
   STR += QString::number(cell_info.CellID) + ", ";
   STR += QString::number(cell_info.CenterpieceID) + ", ";
   STR += QString::number(cell_info.Position) + ", ";
   STR += QString::number(cell_info.InvID) + ", '";
   STR += cell_info.Description + "', ";
   STR += QString::number(cell_info.Wavelength[0]) + ", ";
   STR += QString::number(cell_info.Wavelength[1]) + ", ";
   STR += QString::number(cell_info.Wavelength[2]) + ", ";
   STR += QString::number(cell_info.Scans[0]) + ", ";
   STR += QString::number(cell_info.Scans[1]) + ", ";
   STR += QString::number(cell_info.Scans[2]) + ", ";
   if (cell_info.DNAID[0][0] > 0)
   {
      STR += QString::number(cell_info.DNAID[0][0]) + ", ";
   }
   if (cell_info.DNAID[0][1] > 0)
   {
      STR += QString::number(cell_info.DNAID[0][1]) + ", ";
   }
   if (cell_info.DNAID[0][2] > 0)
   {
      STR += QString::number(cell_info.DNAID[0][2]) + ", ";
   }
   if (cell_info.DNAID[1][0] > 0)
   {
      STR += QString::number(cell_info.DNAID[1][0]) + ", ";
   }
   if (cell_info.DNAID[1][1] > 0)
   {
      STR += QString::number(cell_info.DNAID[1][1]) + ", ";
   }
   if (cell_info.DNAID[1][2] > 0)
   {
      STR += QString::number(cell_info.DNAID[1][2]) + ", ";
   }
   if (cell_info.DNAID[2][0] > 0)
   {
      STR += QString::number(cell_info.DNAID[2][0]) + ", ";
   }
   if (cell_info.DNAID[2][1] > 0)
   {
      STR += QString::number(cell_info.DNAID[2][1]) + ", ";
   }
   if (cell_info.DNAID[2][2] > 0)
   {
      STR += QString::number(cell_info.DNAID[2][2]) + ", ";
   }
   if (cell_info.DNAID[3][0] > 0)
   {
      STR += QString::number(cell_info.DNAID[3][0]) + ", ";
   }
   if (cell_info.DNAID[3][1] > 0)
   {
      STR += QString::number(cell_info.DNAID[3][1]) + ", ";
   }
   if (cell_info.DNAID[3][2] > 0)
   {
      STR += QString::number(cell_info.DNAID[3][2]) + ", ";
   }
   if (cell_info.BufferID[0] > 0)
   {
      STR += QString::number(cell_info.BufferID[0]) + ", ";
   }
   if (cell_info.BufferID[1] > 0)
   {
      STR += QString::number(cell_info.BufferID[1]) + ", ";
   }
   if (cell_info.BufferID[2] > 0)
   {
      STR += QString::number(cell_info.BufferID[2]) + ", ";
   }
   if (cell_info.BufferID[3] > 0)
   {
      STR += QString::number(cell_info.BufferID[3]) + ", ";
   }
   if (cell_info.PepID[0][0] > 0)
   {
      STR += QString::number(cell_info.PepID[0][0]) + ", ";
   }
   if (cell_info.PepID[0][1] > 0)
   {
      STR += QString::number(cell_info.PepID[0][1]) + ", ";
   }
   if (cell_info.PepID[0][2] > 0)
   {
      STR += QString::number(cell_info.PepID[0][2]) + ", ";
   }
   if (cell_info.PepID[1][0] > 0)
   {
      STR += QString::number(cell_info.PepID[1][0]) + ", ";
   }
   if (cell_info.PepID[1][1] > 0)
   {
      STR += QString::number(cell_info.PepID[1][1]) + ", ";
   }
   if (cell_info.PepID[1][2] > 0)
   {
      STR += QString::number(cell_info.PepID[1][2]) + ", ";
   }
   if (cell_info.PepID[2][0] > 0)
   {
      STR += QString::number(cell_info.PepID[2][0]) + ", ";
   }
   if (cell_info.PepID[2][1] > 0)
   {
      STR += QString::number(cell_info.PepID[2][1]) + ", ";
   }
   if (cell_info.PepID[2][2] > 0)
   {
      STR += QString::number(cell_info.PepID[2][2]) + ", ";
   }
   if (cell_info.PepID[3][0] > 0)
   {
      STR += QString::number(cell_info.PepID[3][0]) + ", ";
   }
   if (cell_info.PepID[3][1] > 0)
   {
      STR += QString::number(cell_info.PepID[3][1]) + ", ";
   }
   if (cell_info.PepID[3][2] > 0)
   {
      STR += QString::number(cell_info.PepID[3][2]) + ", ";
   }
   STR += QString::number(cell_info.Num_Channel) + ", ";
   STR += QString::number(cell_info.ExperimentID) + ");";
   cout<<STR<<endl;
   bool finished = target.exec(STR);

   if(finished)
   {
      (*cell_table_unfinished)--;       //After commit all Cell data then you can save Expdata to DB
      pb_add->setEnabled(false);
      lb_query->clear();
      str.sprintf("Cell %d has been saved to the database. ",cell_info.CellID);
      lb_query->insertItem(str);
      lb_query->insertItem("Now you can 'Close' this dialogue and save");
      lb_query->insertItem("the experimental data.");
      lb_query->insertItem("************************************************");
      lb_query->insertItem("In order to change the selected item,");
      lb_query->insertItem("You need to delete the current Entry");
      lb_query->insertItem("and repear the 'Add to DB' process");
   }
   else
   {
      QSqlError sqlerr = target.lastError();
      QMessageBox::message(tr("Attention:"),
                           tr("Saving to DB table 'tblCell' failed.\n"
                              "Attempted to execute this command:\n\n"
                              + STR + "\n\n"
                              "Causing the following error:\n\n")
                           + sqlerr.text());

      lb_query->clear();
      lb_query->insertItem("Warning: Data can NOT be saved.");
      lb_query->insertItem("Ask your Database administrator for help.");
   }
}

/*!
  List selected experimental data's all cell info that stored in DB table: <tt>tblCell</tt>.
*/
void US_Cell_DB::query_db()
{
   QSqlCursor cur( "tblCell" );
   QStringList orderFields = QStringList()<<"CellID"<<"Description";
   QSqlIndex order = cur.index( orderFields );
   QSqlIndex filter = cur.index( "ExperimentID" );
   cur.setValue( "ExperimentID", cell_info.ExperimentID );
   cur.select(filter, order);

   int maxID = get_newID("tblCell", "CellID");
   int count = 0;
   item_CellID = new int[maxID];
   item_Description = new QString[maxID];
   while(cur.next() )
   {
      int id = cur.value("CellID").toInt();
      item_CellID[count] = id;
      item_Description[count] = cur.value("Description").toString();
      count++;
   }
   if(count>0)
   {
      lb_query->clear();
      for( int i=0; i<count; i++)
      {
         lb_query->insertItem(item_Description[i]);
      }
      select_flag = true;
   }
   else
   {
      lb_query->clear();
      lb_query->insertItem("No data available");
   }

}

/*!
  Select one cell <var>item</var> listed in listbox,
  this function will show all its info in interface.
*/
void US_Cell_DB::sel_query(int item)
{
   if(select_flag)
   {
      cell_info.CellID = item_CellID[item];
      cell_info.Description = item_Description[item];

      QSqlCursor cur( "tblCell" );
      QStringList orderFields = QStringList()<<"CenterpieceID"<<"Position"<<"InvestigatorID"
                                             <<"Wavelength_1"<<"Wavelength_2"<<"Wavelength_3"
                                             <<"Wl_scans_1"<<"Wl_scans_2"<<"Wl_scans_3"
                                             <<"BufferID_1"<<"BufferID_2"<<"BufferID_3"<<"BufferID_4"
                                             <<"Peptide1ID_1"<<"Peptide2ID_1"<<"Peptide2ID_1"
                                             <<"Peptide1ID_2"<<"Peptide2ID_2"<<"Peptide2ID_2"
                                             <<"Peptide1ID_3"<<"Peptide2ID_3"<<"Peptide2ID_3"
                                             <<"Peptide1ID_4"<<"Peptide2ID_4"<<"Peptide2ID_4"
                                             <<"DNA1ID_1"<<"DNA2ID_1"<<"DNA3ID_1"
                                             <<"DNA1ID_2"<<"DNA2ID_2"<<"DNA3ID_2"
                                             <<"DNA1ID_3"<<"DNA2ID_3"<<"DNA3ID_3"
                                             <<"DNA1ID_4"<<"DNA2ID_4"<<"DNA3ID_4"
                                             <<"Num_Channel"<<"ExperimentID";
      QSqlIndex order = cur.index( orderFields );
      QSqlIndex filter = cur.index( "CellID" );
      cur.setValue( "CellID", cell_info.CellID );
      cur.select(filter, order);
      while(cur.next())
      {
         cell_info.ExperimentID = cur.value("ExperimentID").toInt();
         cell_info.CenterpieceID = cur.value("CenterpieceID").toInt();
         cell_info.Position = cur.value("Position").toInt();
         cell_info.InvID = cur.value("InvestigatorID").toInt();
         cell_info.Num_Channel = cur.value("Num_Channel").toInt();
         QString wl_str, scans_str;
         for(int i=0; i<3; i++)
         {
            wl_str = "";
            wl_str.sprintf("Wavelength_%d",i+1);
            cell_info.Wavelength[i] = cur.value(wl_str).toInt();
            scans_str = "";
            scans_str.sprintf("Wl_Scans_%d",i+1);
            cell_info.Scans[i] = cur.value(scans_str).toInt();
         }
         QString DNA_Str, Buff_Str, Pep_Str;
         for(int i= 0; i<4; i++)
         {
            Buff_Str = "";
            Buff_Str.sprintf("BufferID_%d",i+1);
            cell_info.BufferID[i] = cur.value(Buff_Str).toInt();
            for (int j=0; j<3; j++)
            {
               if(j==0)
               {
                  Pep_Str = "";
                  Pep_Str.sprintf("PeptideID_%d",(i+1));
                  cell_info.PepID[i][j] = cur.value(Pep_Str).toInt();
                  DNA_Str = "";
                  DNA_Str.sprintf("DNAID_%d",(i+1));
                  cell_info.DNAID[i][j] = cur.value(DNA_Str).toInt();

               }
               else
               {
                  Pep_Str = "";
                  Pep_Str.sprintf("Peptide%dID_%d",(j+1),(i+1));
                  cell_info.PepID[i][j] = cur.value(Pep_Str).toInt();
                  DNA_Str = "";
                  DNA_Str.sprintf("DNA%dID_%d",(j+1),(i+1));
                  cell_info.DNAID[i][j] = cur.value(DNA_Str).toInt();
               }
            }
         }
      }
      from_db = true;
      lbl_cellid->setNum(cell_info.CellID);
      lbl_invest->setNum(cell_info.InvID);
      lbl_ctpc->setNum(cell_info.CenterpieceID);
      lbl_position->setNum(cell_info.Position+1);
      lbl_exp->setNum(cell_info.ExperimentID);
      if(cell_info.Wavelength[0] == 0)
      {
         lbl_wl1->setText("No Data available");
      }
      else
      {
         lbl_wl1->setNum(cell_info.Wavelength[0]);
      }
      if(cell_info.Wavelength[1] == 0)
      {
         lbl_wl2->setText("No Data available");
      }
      else
      {
         lbl_wl2->setNum(cell_info.Wavelength[1]);
      }
      if(cell_info.Wavelength[2] == 0)
      {
         lbl_wl3->setText("No Data available");
      }
      else
      {
         lbl_wl3->setNum(cell_info.Wavelength[2]);
      }
      if(cell_info.Scans[0] == 0)
      {
         lbl_scan1->setText("No Scans");
      }
      else
      {
         lbl_scan1->setNum(cell_info.Scans[0]);
      }
      if(cell_info.Scans[1] == 0)
      {
         lbl_scan2->setText("No Scans");
      }
      else
      {
         lbl_scan2->setNum(cell_info.Scans[1]);
      }
      if(cell_info.Scans[2] == 0)
      {
         lbl_scan3->setText("No Scans");
      }
      else
      {
         lbl_scan3->setNum(cell_info.Scans[2]);
      }
      cmbb_channel->clear();
      QString channel_Str;
      for(int i=0; i<cell_info.Num_Channel; i++)
      {
         channel_Str = "";
         channel_Str.sprintf("Channel %d", i+1);
         cmbb_channel->insertItem(channel_Str);
      }
      int j = cmbb_channel->currentItem();

      le_buffer->setText(show_buffer(cell_info.BufferID[j]));
      if(cell_info.BufferID[j] == -1)
      {
         le_buffer->setText("not selected");
      }
      le_peptide1->setText(show_peptide(cell_info.PepID[j][0]));
      if(cell_info.PepID[j][0] == -1)
      {
         le_peptide1->setText("not selected");
      }
      le_peptide2->setText(show_peptide(cell_info.PepID[j][1]));
      if(cell_info.PepID[j][1] == -1)
      {
         le_peptide2->setText("not selected");
      }
      le_peptide3->setText(show_peptide(cell_info.PepID[j][2]));
      if(cell_info.PepID[j][2] == -1)
      {
         le_peptide3->setText("not selected");
      }
      le_DNA1->setText(show_DNA(cell_info.DNAID[j][0]));
      if(cell_info.DNAID[j][0] == -1)
      {
         le_DNA1->setText("not selected");
      }
      le_DNA2->setText(show_DNA(cell_info.DNAID[j][1]));
      if(cell_info.DNAID[j][1] == -1)
      {
         le_DNA2->setText("not selected");
      }
      le_DNA3->setText(show_DNA(cell_info.DNAID[j][2]));
      if(cell_info.DNAID[j][2] == -1)
      {
         le_DNA3->setText("not selected");
      }


      pb_add->setEnabled(false);
      pb_buffer->setEnabled(false);
      pb_peptide1->setEnabled(false);
      pb_peptide2->setEnabled(false);
      pb_peptide3->setEnabled(false);
      pb_DNA1->setEnabled(false);
      pb_DNA2->setEnabled(false);
      pb_DNA3->setEnabled(false);
   }
   else
   {
      QMessageBox::message(tr("Attention:"),
                           tr("This item cannot be selected"));
      return;
   }
}

/*!
  Open US_DB_Admin to check delete permission.
*/
void US_Cell_DB::check_permission()
{
   US_DB_Admin *db_admin;
   db_admin = new US_DB_Admin("");
   db_admin->show();
   connect(db_admin, SIGNAL(issue_permission(bool)), SLOT(delete_db(bool)));
}

/*!
  Delete one selected cell table entry.
*/
void US_Cell_DB::delete_db(bool permission)
{
   if(!permission)
   {
      QMessageBox::message(tr("Attention:"),
                           tr("Permission denied"));
      return;
   }
   int id = cell_info.CellID;
   if(from_db == false)
   {
      QMessageBox::message(tr("Attention:"),
                           tr("Please use 'Query DB Entry' to select\n"
                              "the record to be deleted from the database"));

   }
   else
   {
      QSqlCursor cur( "tblCell");
      cur.setMode( QSqlCursor::Delete);
      QString filter;
      filter.sprintf("CellID = %d", id);
      cur.select(filter);
      if(cur.next())
      {
         cur.primeDelete();

         switch(QMessageBox::information(this, tr("Delete this record?"),
                                         tr("Clicking 'OK' will delete the selected data from the database,\n"
                                            "and your experimental data will lose this cell's information."),
                                         tr("OK"), tr("CANCEL"),   0,1))
         {
         case 0:
            {
               cur.del();
               (*cell_table_unfinished)++;
               reset();
               quit();
               break;
            }
         case 1:
            {
               break;
            }
         }
      }
      else
      {
         QString str;
         str.sprintf(tr("No Cell Table ID = %d in database"), id);
         QMessageBox::message(tr("Attention:"), str);
         return;
      }
   }
}

/*! Reset all variables to start values. */
void US_Cell_DB::reset()
{
   select_flag = false;
   from_db = false;
   for(int i=0; i<4; i++)
   {
      cell_info.BufferID[i] = -1;
      for(int j=0; j<3; j++)
      {
         cell_info.PepID[i][j] = -1;
         cell_info.DNAID[i][j] = -1;
      }
   }
   le_buffer->setText("not selected");
   le_peptide1->setText("not selected");
   le_peptide2->setText("not selected");
   le_peptide3->setText("not selected");
   le_DNA1->setText("not selected");
   le_DNA2->setText("not selected");
   le_DNA3->setText("not selected");
   lb_query->clear();

   pb_add->setEnabled(true);
   pb_buffer->setEnabled(true);
   pb_peptide1->setEnabled(true);
   pb_peptide2->setEnabled(true);
   pb_peptide3->setEnabled(true);
   pb_DNA1->setEnabled(true);
   pb_DNA2->setEnabled(true);
   pb_DNA3->setEnabled(true);

}
/*!
  This event handler, for event <var>e</var>, to receive widget close events.
*/
void US_Cell_DB::closeEvent(QCloseEvent *e)
{
   e->accept();
   global_Xpos -= 30;
   global_Ypos -= 30;
}

/*! Open a netscape browser to load help page.*/
void US_Cell_DB::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/cell_db.html");
}

/*! Close the interface.*/
void US_Cell_DB::quit()
{
   close();
}
