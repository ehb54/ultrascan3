#include "../include/us_laser.h"

US_Laser::US_Laser(QWidget *p, const char *name) : US_DB( p, name)
{
   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   
   buttonw = 180; buttonh = 26; border = 4; spacing = 2;
   load_flag=false;
   from_HD=true;
   
   xpos = border;
   ypos = border;   
   
   pb_load = new QPushButton(tr("Load Data"), this);
   Q_CHECK_PTR(pb_load);
   pb_load->setAutoDefault(false);
   pb_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_load->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_load->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_load, SIGNAL(clicked()), SLOT(load()));
   
   xpos += buttonw;
   
   data_plot = new QwtPlot(this);
   Q_CHECK_PTR(data_plot);
   data_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
   data_plot->setGeometry(xpos, ypos, buttonw*3, buttonh*11);
   data_plot->enableGridXMin();
   data_plot->enableGridYMin();
   data_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   data_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
   //data_plot->setPlotBackground(USglobal->global_colors.plot);      //old version
   data_plot->setCanvasBackground(USglobal->global_colors.plot);      //new version
   data_plot->setMargin(USglobal->config_list.margin);
   data_plot->enableOutline(true);
   data_plot->setAxisTitle(QwtPlot::xBottom, "Time (usec) ");
   data_plot->setAxisTitle(QwtPlot::yLeft, " ");
   data_plot->setTitle(tr("Experimental Graph"));
   data_plot->setOutlinePen(white);
   data_plot->setOutlineStyle(Qwt::Cross);
   data_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   data_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   data_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   data_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   data_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   data_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   data_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));      
   
   xpos = border;
   ypos += buttonh + spacing;
   
   pb_param = new QPushButton(tr("Parameters"), this);
   Q_CHECK_PTR(pb_param);
   pb_param->setAutoDefault(false);
   pb_param->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_param->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_param->setGeometry(xpos, ypos, buttonw, buttonh);
   pb_param->setEnabled(false);
   connect(pb_param, SIGNAL(clicked()), SLOT(get_parameters()));

   xpos = border;
   ypos += buttonh + spacing;
   
   pb_channel = new QPushButton(tr("Channel Settings"), this);
   Q_CHECK_PTR(pb_channel);
   pb_channel->setAutoDefault(false);
   pb_channel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_channel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_channel->setGeometry(xpos, ypos, buttonw, buttonh);
   pb_channel->setEnabled(false);
   connect(pb_channel, SIGNAL(clicked()), SLOT(get_channels()));

   xpos = border;
   ypos += buttonh + spacing;
   
   pb_result = new QPushButton(tr("Results"), this);
   Q_CHECK_PTR(pb_result);
   pb_result->setAutoDefault(false);
   pb_result->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_result->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_result->setGeometry(xpos, ypos, buttonw, buttonh);
   pb_result->setEnabled(false);
   connect(pb_result, SIGNAL(clicked()), SLOT(get_results()));

   xpos = border;
   ypos += buttonh + spacing;   
      
   lbl_sample = new QLabel(tr(" Sample:"), this);
   Q_CHECK_PTR(lbl_sample);
   lbl_sample->setAlignment(AlignLeft|AlignVCenter);
   lbl_sample->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_sample->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_sample->setGeometry(xpos, ypos, buttonw, buttonh);

   xpos = border;
   ypos += buttonh + spacing;
   
   le_sample = new QLineEdit(this, "sample");
   le_sample->setGeometry(xpos, ypos, buttonw, buttonh);
   le_sample->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   le_sample->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_sample->setReadOnly(true);
   
   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_operator = new QLabel(tr(" Operator:"), this);
   Q_CHECK_PTR(lbl_operator);
   lbl_operator->setAlignment(AlignLeft|AlignVCenter);
   lbl_operator->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_operator->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_operator->setGeometry(xpos, ypos, buttonw, buttonh);

   xpos = border;
   ypos += buttonh + spacing;
   
   le_operator =  new QLineEdit(this, "operator");
   le_operator->setGeometry(xpos, ypos, buttonw, buttonh);
   le_operator->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   le_operator->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_operator->setReadOnly(true);
      
   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_date1 = new QLabel(tr(" Date :"), this);
   Q_CHECK_PTR(lbl_date1);
   lbl_date1->setAlignment(AlignLeft|AlignVCenter);
   lbl_date1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_date1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_date1->setGeometry(xpos, ypos, buttonw/2, buttonh);
   
   xpos += buttonw/2;
   
   lbl_date2 = new QLabel("",this);
   lbl_date2->setGeometry(xpos, ypos, buttonw/2, buttonh);
   lbl_date2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_date2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1));
   
   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_time1 = new QLabel(tr(" Time :"), this);
   Q_CHECK_PTR(lbl_time1);
   lbl_time1->setAlignment(AlignLeft|AlignVCenter);
   lbl_time1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_time1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_time1->setGeometry(xpos, ypos, buttonw/2, buttonh);
   
   xpos += buttonw/2;
   
   lbl_time2 = new QLabel("",this);
   lbl_time2->setGeometry(xpos, ypos, buttonw/2, buttonh);
   lbl_time2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_time2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   
   xpos = border;
   ypos += buttonh + spacing;
   
   pb_print = new QPushButton(tr("Print"), this);
   Q_CHECK_PTR(pb_print);
   pb_print->setAutoDefault(false);
   pb_print->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //pb_print->setEnabled(false);
   pb_print->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_print->setGeometry(xpos, ypos, buttonw, buttonh);
   pb_print->setEnabled(false);
   connect(pb_print, SIGNAL(clicked()), SLOT(print()));

   xpos = border;
   ypos += buttonh + spacing;
   
   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setAutoDefault(false);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_help->setGeometry(xpos, ypos, buttonw, buttonh);
   pb_help->setEnabled(true);
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));
   
   xpos = border;
   ypos += buttonh + spacing;
   
   pb_close = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_close);
   pb_close->setAutoDefault(false);
   pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_close->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_close, SIGNAL(clicked()), SLOT(quit()));

}


US_Laser::~US_Laser()
{
}

void US_Laser::resizeEvent(QResizeEvent *e)
{
   int dialogw = buttonw + spacing + 2 * border;
   int plot_width = e->size().width() - dialogw - border;
   int plot_height = e->size().height()-2*border;
   data_plot->setGeometry(   dialogw, border, plot_width, plot_height); 
}

void US_Laser::load()
{
   QString trashcan;
   if(from_HD)
      fn = QFileDialog::getOpenFileName(USglobal->config_list.data_dir, "*.dat", 0);
   if (!fn.isEmpty())
   {
      QFile f(fn);
      f.open(IO_ReadOnly);
      QTextStream ts (&f);
      trashcan = ts.readLine();
      data_list.run_number = trashcan.toDouble();      // 1.run number
      trashcan = ts.readLine();
      data_list.count_A = trashcan.toDouble();         // 2.Total counts - A Input   
      trashcan = ts.readLine();   
      data_list.count_B = trashcan.toDouble();         // 3.Total counts - B Input
      trashcan = ts.readLine();
      data_list.total = trashcan.toDouble();            // 4.Number of samples
      
      trashcan = ts.readLine();                        // 5.not used
      trashcan = ts.readLine();
      data_list.first_delay = trashcan.toDouble();      // 6.First Delay (usec)
      
      trashcan = ts.readLine();
      data_list.channels = trashcan.toDouble();         // 7.Number of data channels
      
      trashcan = ts.readLine();
      data_list.baseline = trashcan.toFloat();         // 8.which baseline to use
      trashcan = ts.readLine();
      data_list.angle = trashcan.toFloat();            // 9.Angle
      trashcan = ts.readLine();
      data_list.lambda = trashcan.toFloat();            // 10.Lambda
      
      trashcan = ts.readLine();
      data_list.temperature = trashcan.toFloat();      // 11.Temperature
      
      trashcan = ts.readLine();
      data_list.viscosity = trashcan.toFloat();         // 12.Viscosity
   
      trashcan = ts.readLine();                        // 13.not used
      trashcan = ts.readLine();
      data_list.liquid = trashcan.toFloat();            // 14.Refractive index of liquid
      
      trashcan = ts.readLine();
      data_list.real = trashcan.toFloat();            // 15.Refractive index of praticle, real
      trashcan = ts.readLine();
      data_list.imaginary = trashcan.toFloat();         // 16.Refractive index of praticle, imaginary
      trashcan = ts.readLine();                        // 17.not used
      trashcan = ts.readLine();
      data_list.first_channel = trashcan.toFloat();   // 18.First channel used to calculations
      trashcan = ts.readLine();
      data_list.mode1 = trashcan.toFloat();            // 19.Time delay mode
      trashcan = ts.readLine();
      data_list.mode2 = trashcan.toFloat();            // 20.Analysis mode
      trashcan = ts.readLine();   
      data_list.ext_channels = trashcan.toDouble();      // 21.Number of extended baseline channels
   
      trashcan = ts.readLine();   
      data_list.calculated = trashcan.toDouble();      // 22.Calculated baseline
      
      trashcan = ts.readLine();   
      data_list.measured = trashcan.toDouble();         // 23.Measured baseline
      trashcan = ts.readLine();
      data_list.last_delay = trashcan.toDouble();      // 24.Last Delay (usec)
      trashcan = ts.readLine();                        
      data_list.S_time = trashcan.toDouble();            // 25.Sampling time used to generate of samples(usec)
                           
      trashcan = ts.readLine();                        // 26 First delay used from High speed section
      data_list.FH_delay = trashcan.toDouble();
      trashcan = ts.readLine();                        
      data_list.H_channel = trashcan.toDouble();         // 27.Number of High speed channels used
      
      trashcan = ts.readLine();
      data_list.M_time = trashcan.toDouble();            // 28. Middle speed sampling time(us)
      
      trashcan = ts.readLine();
      data_list.M_channel = trashcan.toDouble();         // 29.Number of Middle speed channels
      trashcan = ts.readLine();
      data_list.L_time = trashcan.toDouble();            // 30. Low speed sampling time(us)
      trashcan = ts.readLine();
      data_list.L_channel = trashcan.toDouble();         // 31.Number of Low speed channels
      
      trashcan = ts.readLine();                        // 32. not used
      trashcan = ts.readLine();                        // 33. not used
      trashcan = ts.readLine();                        // 34. not used
      trashcan = ts.readLine();
      data_list.FM_channel = trashcan.toDouble();      // 35.First measured baseline channel number
      trashcan = ts.readLine();
      data_list.LM_channel = trashcan.toDouble();      // 36.Last measured baseline channel number
      trashcan = ts.readLine();                        // 37. not used
      
      int total_channel = (int)(data_list.channels + data_list.ext_channels);
   
      for(int i = 0; i<total_channel; i++)
      {
         ts >> trashcan;
         data_list.dl_time.push_back(trashcan.toDouble());
         ts >> trashcan;
         data_list.correlation.push_back(trashcan.toDouble());
         ts.readLine();
      }
   
      data_list.sampleID = ts.readLine();               // Sample ID
      data_list.operatorID = ts.readLine();            // Operator ID
      data_list.date = ts.readLine();                  // Date
      data_list.time = ts.readLine();                  // Time
      f.close();
      
      load_interface(data_list);
         
   }
}

void US_Laser::load_interface(struct LaserData data_list)
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
   
   le_sample->setText(" "+data_list.sampleID);
   le_operator->setText(" "+data_list.operatorID);
   lbl_date2->setText(" "+data_list.date);
   lbl_time2->setText(" "+data_list.time);
   pb_param->setEnabled(true);
   pb_channel->setEnabled(true);
   pb_result->setEnabled(true);
   pb_print->setEnabled(true);
      
   load_flag=true;
}   

void US_Laser::get_parameters()
{
   US_Laser_Param *prmt_dlg;
   prmt_dlg = new US_Laser_Param(data_list);
   prmt_dlg->show();
}

void US_Laser::get_channels()
{
   US_Laser_Channel *channel_dlg;
   channel_dlg = new US_Laser_Channel(data_list);
   channel_dlg->show();
}

void US_Laser::get_results()
{
   US_Laser_Result *rst_dlg;
   rst_dlg = new US_Laser_Result(data_list);
   rst_dlg->show();
}

void US_Laser::help() 
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/us_laser.html");
}

void US_Laser::print()
{
   QPrinter printer;
   if   (printer.setup(0))
   {
      //data_plot->print(printer, QwtFltrInv());         //old version
      //data_plot->print(printer, QwtPlotPrintFilter());   //new version
   }
}

void US_Laser::quit()
{
   emit dataChanged(data_list);
   close();
}
/*********************************************************************************************/
US_Laser_Param::US_Laser_Param(struct LaserData data_list, QWidget *p, const char *name) : QFrame( p, name)
{
   buttonw = 160; buttonh = 26; border = 4; spacing = 2;

   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));

   xpos = border;
   ypos = border;
   
   QLabel *lbl_banner;
   lbl_banner = new QLabel(tr(" Parameters:"),this);
   Q_CHECK_PTR(lbl_banner);
   lbl_banner->setFrameStyle(QFrame::WinPanel|Raised);
   lbl_banner->setAlignment(AlignCenter|AlignVCenter);
   lbl_banner->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_banner->setGeometry(xpos, ypos, buttonw*3+spacing, buttonh);
   lbl_banner->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_angle1 = new QLabel(tr(" Angle :"), this);
   Q_CHECK_PTR(lbl_angle1);
   lbl_angle1->setAlignment(AlignLeft|AlignVCenter);
   lbl_angle1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_angle1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_angle1->setGeometry(xpos, ypos, buttonw*2, buttonh);
   
   xpos += buttonw*2;
   
   lbl_angle2 = new QLabel("",this);
   lbl_angle2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_angle2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_angle2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));
   lbl_angle2->setNum(data_list.angle);
   lbl_angle2->setIndent(5);
   
   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_lambda1 = new QLabel(tr(" Lambda (nanometres) :"), this);
   Q_CHECK_PTR(lbl_lambda1);
   lbl_lambda1->setAlignment(AlignLeft|AlignVCenter);
   lbl_lambda1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_lambda1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_lambda1->setGeometry(xpos, ypos, buttonw*2, buttonh);
   
   xpos += buttonw*2;
   
   lbl_lambda2 = new QLabel("",this);
   lbl_lambda2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_lambda2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_lambda2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_lambda2->setNum(data_list.lambda);
   lbl_lambda2->setIndent(5);
   
   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_temperature1 = new QLabel(tr(" Temperature (degrees K) :"), this);
   Q_CHECK_PTR(lbl_temperature1);
   lbl_temperature1->setAlignment(AlignLeft|AlignVCenter);
   lbl_temperature1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_temperature1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_temperature1->setGeometry(xpos, ypos, buttonw*2, buttonh);
   
   xpos += buttonw*2;
   
   lbl_temperature2 = new QLabel("",this);
   lbl_temperature2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_temperature2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_temperature2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_temperature2->setNum(data_list.temperature);
   lbl_temperature2->setIndent(5);
   
   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_viscosity1 = new QLabel(tr(" Viscosity (centipoise) :"), this);
   Q_CHECK_PTR(lbl_viscosity1);
   lbl_viscosity1->setAlignment(AlignLeft|AlignVCenter);
   lbl_viscosity1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_viscosity1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_viscosity1->setGeometry(xpos, ypos, buttonw*2, buttonh);
   
   xpos += buttonw*2;
   
   lbl_viscosity2 = new QLabel("",this);
   lbl_viscosity2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_viscosity2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_viscosity2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_viscosity2->setNum(data_list.viscosity);
   lbl_viscosity2->setIndent(5);

   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_liquid1 = new QLabel(tr(" Refractive index of liquid :"), this);
   Q_CHECK_PTR(lbl_liquid1);
   lbl_liquid1->setAlignment(AlignLeft|AlignVCenter);
   lbl_liquid1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_liquid1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_liquid1->setGeometry(xpos, ypos, buttonw*2, buttonh);
   
   xpos += buttonw*2;
   
   lbl_liquid2 = new QLabel("",this);
   lbl_liquid2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_liquid2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_liquid2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_liquid2->setNum(data_list.liquid);
   lbl_liquid2->setIndent(5);
   
   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_real1 = new QLabel(tr(" Refractive index of particle, real :"), this);
   Q_CHECK_PTR(lbl_real1);
   lbl_real1->setAlignment(AlignLeft|AlignVCenter);
   lbl_real1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_real1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_real1->setGeometry(xpos, ypos, buttonw*2, buttonh);
   
   xpos += buttonw*2;
   
   lbl_real2 = new QLabel("",this);
   lbl_real2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_real2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_real2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_real2->setNum(data_list.real);
   lbl_real2->setIndent(5);
   
   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_imaginary1 = new QLabel(tr(" Refractive index of particle, imaginary :"), this);
   Q_CHECK_PTR(lbl_imaginary1);
   lbl_imaginary1->setAlignment(AlignLeft|AlignVCenter);
   lbl_imaginary1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_imaginary1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_imaginary1->setGeometry(xpos, ypos, buttonw*2, buttonh);
   
   xpos += buttonw*2;
   
   lbl_imaginary2 = new QLabel("",this);
   lbl_imaginary2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_imaginary2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_imaginary2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_imaginary2->setNum(data_list.imaginary);
   lbl_imaginary2->setIndent(5);
   
   ypos+= buttonh+border;
   xpos = buttonw*3+2*border+spacing;

   global_Xpos += 30;
   global_Ypos += 30;
   
   setMinimumSize(xpos, ypos);
   setGeometry(global_Xpos, global_Ypos, xpos, ypos);
}
US_Laser_Param::~US_Laser_Param()
{
}
/*********************************************************************************************/
US_Laser_Channel::US_Laser_Channel(struct LaserData data_list, QWidget *p, const char *name) : QFrame( p, name)
{
   buttonw = 160; buttonh = 26; border = 4; spacing = 2;

   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));

   xpos = border;
   ypos = border;
   
   QLabel *lbl_banner;
   lbl_banner = new QLabel(tr(" Channel Settings:"),this);
   Q_CHECK_PTR(lbl_banner);
   lbl_banner->setFrameStyle(QFrame::WinPanel|Raised);
   lbl_banner->setAlignment(AlignCenter|AlignVCenter);
   lbl_banner->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_banner->setGeometry(xpos, ypos, buttonw*3+spacing, buttonh);
   lbl_banner->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   
   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_baseline1 = new QLabel(tr(" Baseline Mode:"), this);
   Q_CHECK_PTR(lbl_baseline1);
   lbl_baseline1->setAlignment(AlignLeft|AlignVCenter);
   lbl_baseline1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_baseline1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   lbl_baseline1->setGeometry(xpos, ypos, buttonw*3/2, buttonh);
   
   xpos += buttonw*3/2;
   
   lbl_baseline2 = new QLabel("",this);
   lbl_baseline2->setGeometry(xpos, ypos, buttonw*3/2, buttonh);
   lbl_baseline2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_baseline2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   if(data_list.baseline == 0)
   {
      lbl_baseline2->setText(" will prompt");            
   }
   if(data_list.baseline == 1)
   {
      lbl_baseline2->setText(" use calculated baseline");      
   }
   if(data_list.baseline == 2)
   {
      lbl_baseline2->setText(" use measured baseline");   
   }

   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_mode11 = new QLabel(tr(" Time Delay Mode :"), this);
   Q_CHECK_PTR(lbl_mode11);
   lbl_mode11->setAlignment(AlignLeft|AlignVCenter);
   lbl_mode11->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_mode11->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   lbl_mode11->setGeometry(xpos, ypos, buttonw*3/2, buttonh);
   
   xpos += buttonw*3/2;
   
   lbl_mode12 = new QLabel("",this);
   lbl_mode12->setGeometry(xpos, ypos, buttonw*3/2, buttonh);
   lbl_mode12->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_mode12->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   if(data_list.mode1 == -2)
   {
      lbl_mode12->setText(" Constant ratio spacing");      
   }
   if(data_list.mode1 == -1)
   {
      lbl_mode12->setText(" Spacing from delay file");
   }
   if(data_list.mode1 == 1)
   {
      lbl_mode12->setText(" Linear spacing");      
   }
   
   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_mode21 = new QLabel(tr(" Analysis Mode :"), this);
   Q_CHECK_PTR(lbl_mode21);
   lbl_mode21->setAlignment(AlignLeft|AlignVCenter);
   lbl_mode21->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_mode21->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   lbl_mode21->setGeometry(xpos, ypos, buttonw*3/2, buttonh);
   
   xpos += buttonw*3/2;
   
   lbl_mode22 = new QLabel("",this);
   lbl_mode22->setGeometry(xpos, ypos, buttonw*3/2, buttonh);
   lbl_mode22->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_mode22->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   if(data_list.mode2 == 2)
   {
      lbl_mode22->setText(" Auto Correlation");   
   }
   if(data_list.mode2 == 3)
   {
      lbl_mode22->setText(" Cross Correlation");         
   }
   if(data_list.mode2 == 4)
   {
      lbl_mode22->setText(" Test");      
   }

   xpos = border;
   ypos += buttonh + spacing;


   lbl_channels1 = new QLabel(tr(" Number of data channels :"), this);
   Q_CHECK_PTR(lbl_channels1);
   lbl_channels1->setAlignment(AlignLeft|AlignVCenter);
   lbl_channels1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_channels1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_channels1->setGeometry(xpos, ypos, buttonw*2, buttonh);
   
   xpos += buttonw*2;
   
   lbl_channels2 = new QLabel("",this);
   lbl_channels2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_channels2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_channels2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_channels2->setNum(data_list.channels);
   lbl_channels2->setIndent(5);
   
   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_extended1 = new QLabel(tr(" # of extended channels:"), this);
   Q_CHECK_PTR(lbl_extended1);
   lbl_extended1->setAlignment(AlignLeft|AlignVCenter);
   lbl_extended1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_extended1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_extended1->setGeometry(xpos, ypos, buttonw*2, buttonh);
   
   xpos += buttonw*2;
   
   lbl_extended2 = new QLabel("",this);
   lbl_extended2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_extended2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_extended2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_extended2->setNum(data_list.ext_channels);
   lbl_extended2->setIndent(5);
   
   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_Fdelay1 = new QLabel(tr(" First delay (usec) :"), this);
   Q_CHECK_PTR(lbl_Fdelay1);
   lbl_Fdelay1->setAlignment(AlignLeft|AlignVCenter);
   lbl_Fdelay1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_Fdelay1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_Fdelay1->setGeometry(xpos, ypos, buttonw*2, buttonh);
   
   xpos += buttonw*2;
   
   lbl_Fdelay2 = new QLabel("",this);
   lbl_Fdelay2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_Fdelay2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_Fdelay2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_Fdelay2->setNum(data_list.first_delay);
   lbl_Fdelay2->setIndent(5);
   
   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_Ldelay1 = new QLabel(tr(" Last delay (usec) :"), this);
   Q_CHECK_PTR(lbl_Ldelay1);
   lbl_Ldelay1->setAlignment(AlignLeft|AlignVCenter);
   lbl_Ldelay1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_Ldelay1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_Ldelay1->setGeometry(xpos, ypos, buttonw*2, buttonh);
   lbl_Ldelay1->setIndent(5);
   
   xpos += buttonw*2;
   
   lbl_Ldelay2 = new QLabel("",this);
   lbl_Ldelay2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_Ldelay2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_Ldelay2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_Ldelay2->setNum(data_list.last_delay);
   lbl_Ldelay2->setIndent(5);
   
   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_first1 = new QLabel(tr(" First channel used for calculations:"), this);
   Q_CHECK_PTR(lbl_first1);
   lbl_first1->setAlignment(AlignLeft|AlignVCenter);
   lbl_first1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_first1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_first1->setGeometry(xpos, ypos, buttonw*2, buttonh);
   
   xpos += buttonw*2;
   
   lbl_first2 = new QLabel("",this);
   lbl_first2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_first2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_first2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_first2->setNum(data_list.first_channel);
   lbl_first2->setIndent(5);
   
   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_FMchannel1 = new QLabel(tr(" First measured baseline channel number:"), this);
   Q_CHECK_PTR(lbl_FMchannel1);
   lbl_FMchannel1->setAlignment(AlignLeft|AlignVCenter);
   lbl_FMchannel1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_FMchannel1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_FMchannel1->setGeometry(xpos, ypos, buttonw*2, buttonh);
   
   xpos += buttonw*2;
   
   lbl_FMchannel2 = new QLabel("",this);
   lbl_FMchannel2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_FMchannel2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_FMchannel2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_FMchannel2->setNum(data_list.FM_channel);
   lbl_FMchannel2->setIndent(5);
   
   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_LMchannel1 = new QLabel(tr(" Last measured baseline channel number:"), this);
   Q_CHECK_PTR(lbl_LMchannel1);
   lbl_LMchannel1->setAlignment(AlignLeft|AlignVCenter);
   lbl_LMchannel1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_LMchannel1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_LMchannel1->setGeometry(xpos, ypos, buttonw*2, buttonh);
   
   xpos += buttonw*2;
   
   lbl_LMchannel2 = new QLabel("",this);
   lbl_LMchannel2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_LMchannel2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_LMchannel2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_LMchannel2->setNum(data_list.LM_channel);
   lbl_LMchannel2->setIndent(5);
   
   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_Stime1 = new QLabel(tr(" Sampling time (usec) :"), this);
   Q_CHECK_PTR(lbl_Stime1);
   lbl_Stime1->setAlignment(AlignLeft|AlignVCenter);
   lbl_Stime1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_Stime1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_Stime1->setGeometry(xpos, ypos, buttonw*2, buttonh);
   
   xpos += buttonw*2;
      
   lbl_Stime2 = new QLabel("",this);
   lbl_Stime2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_Stime2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_Stime2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_Stime2->setNum(data_list.S_time);
   lbl_Stime2->setIndent(5);

   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_FH_delay1 = new QLabel(tr(" First delay used from High speed section:"), this);
   Q_CHECK_PTR(lbl_FH_delay1);
   lbl_FH_delay1->setAlignment(AlignLeft|AlignVCenter);
   lbl_FH_delay1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_FH_delay1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_FH_delay1->setGeometry(xpos, ypos, buttonw*2, buttonh);
   
   xpos += buttonw*2;
      
   lbl_FH_delay2 = new QLabel("",this);
   lbl_FH_delay2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_FH_delay2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_FH_delay2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_FH_delay2->setNum(data_list.FH_delay);
   lbl_FH_delay2->setIndent(5);
   
   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_Hchannel1 = new QLabel(tr(" # of High Speed Channels :"), this);
   Q_CHECK_PTR(lbl_Hchannel1);
   lbl_Hchannel1->setAlignment(AlignLeft|AlignVCenter);
   lbl_Hchannel1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_Hchannel1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_Hchannel1->setGeometry(xpos, ypos, buttonw*2, buttonh);
      
   xpos += buttonw*2;
   
   lbl_Hchannel2 = new QLabel("",this);
   lbl_Hchannel2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_Hchannel2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_Hchannel2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_Hchannel2->setNum(data_list.H_channel);
   lbl_Hchannel2->setIndent(5);
   
   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_Mtime1 = new QLabel(tr(" Middle speed Sampling time (usec) :"), this);
   Q_CHECK_PTR(lbl_Mtime1);   
   lbl_Mtime1->setAlignment(AlignLeft|AlignVCenter);
   lbl_Mtime1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_Mtime1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_Mtime1->setGeometry(xpos, ypos, buttonw*2, buttonh);
   
   xpos += buttonw*2;
      
   lbl_Mtime2 = new QLabel("",this);
   lbl_Mtime2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_Mtime2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_Mtime2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_Mtime2->setNum(data_list.M_time);
   lbl_Mtime2->setIndent(5);
   
   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_Mchannel1 = new QLabel(tr(" # of Middle Speed Channels :"), this);
   Q_CHECK_PTR(lbl_Mchannel1);
   lbl_Mchannel1->setAlignment(AlignLeft|AlignVCenter);
   lbl_Mchannel1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_Mchannel1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_Mchannel1->setGeometry(xpos, ypos, buttonw*2, buttonh);
      
   xpos += buttonw*2;
   
   lbl_Mchannel2 = new QLabel("",this);
   lbl_Mchannel2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_Mchannel2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_Mchannel2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_Mchannel2->setNum(data_list.M_channel);
   lbl_Mchannel2->setIndent(5);
   
   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_Ltime1 = new QLabel(tr(" Low speed Sampling time (usec) :"), this);
   Q_CHECK_PTR(lbl_Ltime1);   
   lbl_Ltime1->setAlignment(AlignLeft|AlignVCenter);
   lbl_Ltime1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_Ltime1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_Ltime1->setGeometry(xpos, ypos, buttonw*2, buttonh);
   
   xpos += buttonw*2;
      
   lbl_Ltime2 = new QLabel("",this);
   lbl_Ltime2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_Ltime2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_Ltime2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_Ltime2->setNum(data_list.L_time);
   lbl_Ltime2->setIndent(5);
   
   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_Lchannel1 = new QLabel(tr(" # of Low Speed Channels :"), this);
   Q_CHECK_PTR(lbl_Lchannel1);
   lbl_Lchannel1->setAlignment(AlignLeft|AlignVCenter);
   lbl_Lchannel1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_Lchannel1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_Lchannel1->setGeometry(xpos, ypos, buttonw*2, buttonh);
      
   xpos += buttonw*2;
   
   lbl_Lchannel2 = new QLabel("",this);
   lbl_Lchannel2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_Lchannel2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_Lchannel2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_Lchannel2->setNum(data_list.L_channel);
   lbl_Lchannel2->setIndent(5);

   ypos+= buttonh+border;
   xpos = buttonw*3+2*border+spacing;

   global_Xpos += 30;
   global_Ypos += 30;
   
   setMinimumSize(xpos, ypos);
   setGeometry(global_Xpos, global_Ypos, xpos, ypos);
}
US_Laser_Channel::~US_Laser_Channel()
{
}

/*********************************************************************************************/
US_Laser_Result::US_Laser_Result(struct LaserData data_list, QWidget *p, const char *name) : QFrame( p, name)
{
   buttonw = 140; buttonh = 26; border = 4; spacing = 2;

   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));

   xpos = border;
   ypos = border;
   
   QLabel *lbl_banner;
   lbl_banner = new QLabel(tr(" Results:"),this);
   Q_CHECK_PTR(lbl_banner);
   lbl_banner->setFrameStyle(QFrame::WinPanel|Raised);
   lbl_banner->setAlignment(AlignCenter|AlignVCenter);
   lbl_banner->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_banner->setGeometry(xpos, ypos, buttonw*3+spacing, buttonh);
   lbl_banner->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   
   xpos = border;
   ypos += buttonh + spacing;

   lbl_countA1 = new QLabel(tr(" Total counts - A :"), this);
   Q_CHECK_PTR(lbl_countA1);
   lbl_countA1->setAlignment(AlignLeft|AlignVCenter);
   lbl_countA1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_countA1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_countA1->setGeometry(xpos, ypos, buttonw*2, buttonh);
   
   xpos += buttonw*2;
   
   lbl_countA2 = new QLabel("",this);
   lbl_countA2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_countA2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_countA2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_countA2->setNum(data_list.count_A);
   lbl_countA2->setIndent(5);
   
   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_countB1 = new QLabel(tr(" Total counts - B :"), this);
   Q_CHECK_PTR(lbl_countB1);
   lbl_countB1->setAlignment(AlignLeft|AlignVCenter);
   lbl_countB1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_countB1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_countB1->setGeometry(xpos, ypos, buttonw*2, buttonh);
   
   xpos += buttonw*2;
   
   lbl_countB2 = new QLabel("",this);
   lbl_countB2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_countB2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_countB2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_countB2->setNum(data_list.count_B);
   lbl_countB2->setIndent(5);
   
   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_total1 = new QLabel(tr(" Number of samples :"), this);
   Q_CHECK_PTR(lbl_total1);
   lbl_total1->setAlignment(AlignLeft|AlignVCenter);
   lbl_total1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_total1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_total1->setGeometry(xpos, ypos, buttonw*2, buttonh);
   
   xpos += buttonw*2;
   
   lbl_total2 = new QLabel("",this);
   lbl_total2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_total2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_total2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_total2->setNum(data_list.total);
   lbl_total2->setIndent(5);
   
   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_calculated1 = new QLabel(tr(" Calculated baseline :"), this);
   Q_CHECK_PTR(lbl_calculated1);
   lbl_calculated1->setAlignment(AlignLeft|AlignVCenter);
   lbl_calculated1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_calculated1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_calculated1->setGeometry(xpos, ypos, buttonw*2, buttonh);
   
   xpos += buttonw*2;
   
   lbl_calculated2 = new QLabel("",this);
   lbl_calculated2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_calculated2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_calculated2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_calculated2->setNum(data_list.calculated);
   lbl_calculated2->setIndent(5);
   
   xpos = border;
   ypos += buttonh + spacing;
   
   lbl_measured1 = new QLabel(tr(" Measured baseline :"), this);
   Q_CHECK_PTR(lbl_measured1);
   lbl_measured1->setAlignment(AlignLeft|AlignVCenter);
   lbl_measured1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_measured1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_measured1->setGeometry(xpos, ypos, buttonw*2, buttonh);
   
   xpos += buttonw*2;
   
   lbl_measured2 = new QLabel("",this);
   lbl_measured2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_measured2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_measured2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_measured2->setNum(data_list.measured);
   lbl_measured2->setIndent(5);

   ypos+= buttonh+border;
   xpos = buttonw*3+2*border+spacing;

   global_Xpos += 30;
   global_Ypos += 30;
   
   setMinimumSize(xpos, ypos);
   setGeometry(global_Xpos, global_Ypos, xpos, ypos);
}
US_Laser_Result::~US_Laser_Result()
{
}

