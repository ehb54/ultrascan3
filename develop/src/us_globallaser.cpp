#include "../include/us_globallaser.h"


US_GlobalLaser::US_GlobalLaser(QWidget *p, const char *name) : US_GlobalEquil(p, name)
{
   disconnect(pb_details, 0, 0,0);
   lb_scans->disconnect();
   data_plot->disconnect();
   pb_load_data = new QPushButton(tr("Load Data"), this);
   Q_CHECK_PTR(pb_load_data);   
   pb_load_data->setAutoDefault(false);
   pb_load_data->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_load_data->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_load_data, SIGNAL(clicked()), SLOT(load_data()));   

   connect(lb_scans, SIGNAL(highlighted(int)), SLOT(select_ls(int)));
   connect(lb_scans, SIGNAL(selected(int)), SLOT(select_ls(int)));
   connect(pb_details, SIGNAL(clicked()), SLOT(show_detail()));
   
}

US_GlobalLaser::~US_GlobalLaser()
{
}

void US_GlobalLaser::load_data()
{
   us_laser_db = new US_DB_Laser();
   us_laser_db->pb_close->setText("Accept");
   us_laser_db->show();
   connect(us_laser_db, SIGNAL(dataChanged(struct LaserData)), SLOT(update_data(struct LaserData)));

}

void US_GlobalLaser:: update_data(struct LaserData data_list)
{
   QString str;
   if(data_list.sampleID !="")
   {
      if(data_list.LaserID >0)
         str= "DB-> "+data_list.sampleID +", ("+data_list.date+")";
      else
         str= "HD-> "+data_list.sampleID +", ("+data_list.date+")";
      lb_scans->insertItem(str);
   }
   load_plot(data_list);

   ls_info.push_back(data_list);
   pb_unload->setEnabled(true);
   pb_print->setEnabled(true);
   pb_model->setEnabled(true);
}

void US_GlobalLaser::load_plot(struct LaserData data_list)
{
   double *delay_time, *contents;
   int total_channel = (int)(data_list.channels + data_list.ext_channels);
   delay_time = new double[total_channel];
   contents = new double[total_channel];
   
   for(int i=0; i<total_channel; i++)
   {
      delay_time[i]= data_list.dl_time[i];
      contents[i] =data_list.correlation[i];
   }
   QwtSymbol sym;
   sym.setStyle(QwtSymbol::Ellipse);
   sym.setPen(blue);
   sym.setBrush(white);
   sym.setSize(5);
   long data_curve;
   data_curve = data_plot->insertCurve("Experimental Graph");
   data_plot->setCurvePen(data_curve, yellow);
   data_plot->setCurveStyle(data_curve, QwtCurve::Spline);
   data_plot->setCurveSymbol(data_curve, QwtSymbol(sym));
   data_plot->setCurveData(data_curve, delay_time, contents, total_channel);
   data_plot->replot();

}   

void US_GlobalLaser::select_ls(int item)
{
   currentData = ls_info[item];
   load_plot(currentData);
   pb_details->setEnabled(true);
}
void US_GlobalLaser::show_detail()
{

   ls_detail = new US_GlobalLaser_Detail(currentData);
   ls_detail->show();
      
}                     
void US_GlobalLaser::resizeEvent(QResizeEvent *e)
{
   buttonw = 156;   //make divisible by 3 and by 2
   buttonh = 26;
   border = 4;
   xpos = border;
   ypos = border;
   pb_load->setGeometry(0,0,0,0);
   pb_diagnostics->setGeometry(0,0,0,0);
   lbl_projectName->setGeometry(0,0,0,0);
   le_projectName->setGeometry(0,0,0,0);
   pb_load_data->setGeometry(xpos, ypos, buttonw, buttonh);
   xpos += buttonw + spacing;
   pb_details->setGeometry(xpos, ypos, buttonw, buttonh);
   xpos = border;
   ypos += buttonh + spacing;
   pb_unload->setGeometry(xpos, ypos, buttonw, buttonh);
   xpos += buttonw + spacing;
   pb_print->setGeometry(xpos, ypos, buttonw, buttonh);
   xpos = border;
   ypos += buttonh + spacing;
   pb_histogram->setGeometry(xpos, ypos, buttonw, buttonh);
   xpos += buttonw + spacing;
   pb_scan_check->setGeometry(xpos, ypos, buttonw, buttonh);   
   xpos = border;
   ypos += buttonh + spacing;
   
   xpos += buttonw + spacing;
   pb_reset_limits->setGeometry(xpos, ypos, buttonw, buttonh);
   xpos = border;
   ypos += buttonh + spacing;
   lbl_banner1->setGeometry(xpos, ypos, buttonw*2+spacing, buttonh);
   ypos += buttonh;
   lb_scans->setGeometry(xpos, ypos, 2 * buttonw + spacing, 9 * buttonh);
   ypos += 9 * buttonh + 2 * spacing;
   lbl_banner2->setGeometry(xpos, ypos, buttonw*2+spacing, buttonh);
   xpos = border;
   ypos += buttonh + 2 * spacing;
   pb_model->setGeometry(xpos, ypos, buttonw, buttonh);
   xpos += buttonw + spacing;
   pb_fitcontrol->setGeometry(xpos, ypos, buttonw, buttonh);
   xpos = border;
   ypos += buttonh + spacing;
   pb_report->setGeometry(xpos, ypos, buttonw, buttonh);
   xpos += buttonw + spacing;
   pb_monte_carlo->setGeometry(xpos, ypos, buttonw, buttonh);
   xpos = border;
   ypos += buttonh + spacing;
   pb_loadfit->setGeometry(xpos, ypos, buttonw, buttonh);
   xpos += buttonw + spacing;
   pb_help->setGeometry(xpos, ypos, buttonw, buttonh);
   xpos = border;
   ypos += buttonh + 2 * spacing;
   lbl_banner3->setGeometry(xpos, ypos, buttonw*2+spacing, buttonh);
   xpos = border;
   ypos += buttonh + 2 * spacing;
   pb_float_all->setGeometry(xpos, ypos, buttonw, buttonh);
   xpos += buttonw + spacing;
   pb_initialize->setGeometry(xpos, ypos, buttonw, buttonh);
   xpos = border;
   ypos += buttonh + spacing;
   pb_model_control->setGeometry(xpos, ypos, buttonw, buttonh);
   xpos += buttonw + spacing;
   pb_close->setGeometry(xpos, ypos, buttonw, buttonh);
   xpos = border;
   ypos += buttonh + spacing;
   lbl_scans->setGeometry(xpos, ypos, buttonw, buttonh);
   xpos += buttonw + spacing;
   cnt_scans->setGeometry(xpos, ypos, buttonw, buttonh);
   
   
   int dialogw = 2 * buttonw + spacing + 2 * border;
   int plot_width = e->size().width() - dialogw - border;
   int plot_height = e->size().height() - 3 * border - 3 * buttonh - 2 * spacing;
   data_plot->setGeometry(   dialogw, border, plot_width, plot_height);
   InfoPanel->setGeometry(   dialogw, plot_height + 2 * border, plot_width, 3 * buttonh + 2 * spacing);
}

/*********************************************************************************************/
US_GlobalLaser_Detail::US_GlobalLaser_Detail(struct LaserData temp_list, QWidget *p, const char *name) : US_DB_Laser(p, name)
{
   ls_list = temp_list;
   lbl_invest = new QLabel(tr(" Investigator:"),this);
   lbl_invest->setAlignment(AlignLeft|AlignVCenter);
   lbl_invest->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_invest->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   lbl_buff = new QLabel(tr(" Buffer:"),this);
   lbl_buff->setAlignment(AlignLeft|AlignVCenter);
   lbl_buff->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_buff->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   lbl_pep = new QLabel(tr(" Peptide:"),this);
   lbl_pep->setAlignment(AlignLeft|AlignVCenter);
   lbl_pep->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_pep->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   
   lbl_dna = new QLabel(tr(" DNA:"),this);
   lbl_dna->setAlignment(AlignLeft|AlignVCenter);
   lbl_dna->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_dna->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   

   load_interface(ls_list);
   le_buffer->setText(show_buffer(ls_list.BuffID));
   le_peptide->setText(show_peptide(ls_list.PepID));
   le_DNA->setText(show_DNA(ls_list.DNAID));   
   le_investigator->setText(show_investigator(ls_list.InvID));
   pb_param->disconnect();
   pb_channel->disconnect();
   pb_result->disconnect();
   connect(pb_param, SIGNAL(clicked()), SLOT(parameter_detail()));
   connect(pb_channel, SIGNAL(clicked()), SLOT(channel_detail()));
   connect(pb_result, SIGNAL(clicked()), SLOT(result_detail()));

}
US_GlobalLaser_Detail::~US_GlobalLaser_Detail()
{
}

void US_GlobalLaser_Detail::parameter_detail()
{
   US_Laser_Param *prmt_dlg;
   prmt_dlg = new US_Laser_Param(ls_list);
   prmt_dlg->show();
}

void US_GlobalLaser_Detail::channel_detail()
{
   US_Laser_Channel *channel_dlg;
   channel_dlg = new US_Laser_Channel(ls_list);
   channel_dlg->show();
}

void US_GlobalLaser_Detail::result_detail()
{
   US_Laser_Result *rst_dlg;
   rst_dlg = new US_Laser_Result(ls_list);
   rst_dlg->show();
}


void US_GlobalLaser_Detail::resizeEvent(QResizeEvent *e)
{

   xpos = border;
   ypos = border;
   pb_load_db->setGeometry(0,0,0,0);
   pb_reset->setGeometry(0,0,0,0);
   pb_save_db->setGeometry(0,0,0,0);
   pb_del_db->setGeometry(0,0,0,0);
   pb_investigator->setGeometry(0,0,0,0);
   pb_buffer->setGeometry(0,0,0,0);
   pb_peptide->setGeometry(0,0,0,0);
   pb_DNA->setGeometry(0,0,0,0);
   lb_data->setGeometry(0,0,0,0);
   instr_lbl->setGeometry(0,0,0,0);
   pb_print->setGeometry(0,0,0,0);
   pb_param->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos+=buttonw+spacing;
   pb_channel->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos = border;
   ypos += buttonh + spacing;
   pb_result->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos = border;
   ypos += buttonh + spacing;
   lbl_invest->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos+=buttonw+spacing;
   le_investigator->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos = border;
   ypos += buttonh + spacing;
   lbl_buff->setGeometry(xpos, ypos, buttonw, buttonh);
      
   xpos+=buttonw+spacing;
   le_buffer->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos = border;
   ypos += buttonh + spacing;
   lbl_pep->setGeometry(xpos, ypos, buttonw, buttonh);
      
   xpos+=buttonw+spacing;
   le_peptide->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos = border;
   ypos += buttonh + spacing;
   lbl_dna->setGeometry(xpos, ypos, buttonw, buttonh);
      
   xpos+=buttonw+spacing;
   le_DNA->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos = border;
   ypos += buttonh + spacing;
   lbl_sample->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos+=buttonw+spacing;
   le_sample->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos = border;
   ypos += buttonh + spacing;
   lbl_operator->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos+=buttonw+spacing;
   le_operator->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos = border;
   ypos += buttonh + spacing;
   lbl_date1->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos += buttonw+ spacing;;
   lbl_date2->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos = border;
   ypos += buttonh + spacing;
   lbl_time1->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos += buttonw + spacing;
   lbl_time2->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos = border;
   ypos += buttonh + spacing;
   pb_help->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos+=buttonw+spacing;
   pb_close->setGeometry(xpos, ypos, buttonw, buttonh);
   
   int dialogw = buttonw*2 + spacing + 2 * border;
   int plot_width = e->size().width() - dialogw - border;
   int plot_height = e->size().height()-2*border;
   data_plot->setGeometry(   dialogw, border, plot_width, plot_height); 
}
