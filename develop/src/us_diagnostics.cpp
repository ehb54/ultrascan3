#include "../include/us_diagnostics.h"


US_Diagnostics::US_Diagnostics(QWidget *p , const char *name) : QFrame(p, name)
{
   USglobal = new US_Config();
   int xpos = 2, ypos = 2, spacing = 2, buttonh = 26, buttonw0 = 220;
   converted = false;

   setCaption(tr("Scan Diagnostics"));
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   QString str;

   delta_plot = new QwtPlot(this);
   delta_plot->setTitle(tr("Delta-R Plot"));
   delta_plot->enableGridXMin();
   delta_plot->enableGridYMin();
   delta_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
   delta_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   delta_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
   //delta_plot->setPlotBackground(USglobal->global_colors.plot);      //old version
   delta_plot->setCanvasBackground(USglobal->global_colors.plot);      //new version
   delta_plot->setAxisTitle(QwtPlot::xBottom, tr("Datapoint Number"));
   delta_plot->setAxisTitle(QwtPlot::yLeft, tr("Delta-Radius"));
   delta_plot->enableOutline(true);
   delta_plot->setOutlinePen(white);
   delta_plot->setOutlineStyle(Qwt::Cross);
   delta_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   delta_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   delta_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   delta_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   delta_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   delta_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   delta_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));      
   delta_plot->setMargin(USglobal->config_list.margin);

   scan_plot= new QwtPlot(this);
   scan_plot->setTitle(tr("Scan Plot:"));
   scan_plot->enableGridXMin();
   scan_plot->enableGridYMin();
   scan_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
   scan_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   scan_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
   //scan_plot->setPlotBackground(USglobal->global_colors.plot);      //old version
   scan_plot->setCanvasBackground(USglobal->global_colors.plot);      //new version
   scan_plot->setAxisTitle(QwtPlot::xBottom, tr("Radius (cm)"));
   scan_plot->setAxisTitle(QwtPlot::yLeft, tr("Absorbance"));
   scan_plot->enableOutline(true);
   scan_plot->setOutlinePen(white);
   scan_plot->setOutlineStyle(Qwt::Cross);
   scan_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   scan_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   scan_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   scan_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   scan_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   scan_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   scan_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));      
   scan_plot->setMargin(USglobal->config_list.margin);
   
   pb_load = new QPushButton(tr("Load Scan"), this);
   Q_CHECK_PTR(pb_load);
   pb_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_load->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_load->setGeometry(xpos, ypos, buttonw0, buttonh);
   pb_load->setAutoDefault(false);   
   connect(pb_load, SIGNAL(clicked()), SLOT(load()));

   ypos += buttonh + spacing;
   pb_print_delta = new QPushButton(tr("Print Delta Plot"), this);
   Q_CHECK_PTR(pb_print_delta);
   pb_print_delta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_print_delta->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_print_delta->setAutoDefault(false);
   pb_print_delta->setGeometry(xpos, ypos, buttonw0, buttonh);
   connect(pb_print_delta, SIGNAL(clicked()), SLOT(print_delta()));

   pb_print_scan = new QPushButton(tr("Print Scan Plot"), this);
   Q_CHECK_PTR(pb_print_scan);
   pb_print_scan->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_print_scan->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_print_scan->setAutoDefault(false);
   pb_print_scan->setGeometry(xpos, ypos, buttonw0, buttonh);
   connect(pb_print_scan, SIGNAL(clicked()), SLOT(print_scan()));

   pb_convert = new QPushButton(tr("Convert to Pseudo-Abs"), this);
   pb_convert->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_convert->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_convert->setAutoDefault(false);
   connect(pb_convert, SIGNAL(clicked()), SLOT(convert()));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_help->setGeometry(xpos, ypos, buttonw0, buttonh);
   pb_help->setAutoDefault(false);
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   ypos += buttonh + spacing;

   pb_close = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_close);
   pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_close->setGeometry(xpos, ypos, buttonw0, buttonh);
   pb_close->setAutoDefault(false);
   connect(pb_close, SIGNAL(clicked()), SLOT(close()));

   ypos += buttonh + spacing + 2;

   banner1 = new QLabel(tr("Information for this Scan:"),this);
   banner1->setFrameStyle(QFrame::WinPanel|Raised);
   banner1->setAlignment(AlignCenter|AlignVCenter);
   banner1->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
   banner1->setGeometry(xpos, ypos, buttonw0, buttonh);
   banner1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   ypos += buttonh + spacing + 2;
   int buttonw = 138;
   int buttonw2 = 80;
   
   lbl_file1 = new QLabel(tr(" Filename:"),this);
   lbl_file1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_file1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_file1->setGeometry(xpos, ypos, buttonw0, buttonh);
   lbl_file1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));   
   lbl_file1->setAlignment(AlignCenter|AlignVCenter);

   ypos += buttonh + spacing;
   
   lbl_file2 = new QLabel("", this);
   lbl_file2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_file2->setGeometry(xpos, ypos, buttonw0, buttonh);
   lbl_file2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_file2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_file2->setAlignment(AlignCenter|AlignVCenter);
   
   ypos += buttonh + spacing;
   
   lbl_cell1 = new QLabel(tr(" Cell:"),this);
   lbl_cell1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_cell1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_cell1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_cell1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));   
   lbl_cell1->setAlignment(AlignLeft|AlignVCenter);

   xpos += spacing + buttonw;
   
   lbl_cell2 = new QLabel("", this);
   lbl_cell2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_cell2->setGeometry(xpos, ypos, buttonw2, buttonh);
   lbl_cell2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_cell2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_cell2->setAlignment(AlignCenter|AlignVCenter);

   xpos = spacing;
   ypos += buttonh + spacing;
   
   lbl_scanmode1 = new QLabel(tr(" Scanmode:"),this);
   lbl_scanmode1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_scanmode1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_scanmode1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_scanmode1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));   
   lbl_scanmode1->setAlignment(AlignLeft|AlignVCenter);

   xpos += spacing + buttonw;
   
   lbl_scanmode2 = new QLabel("", this);
   lbl_scanmode2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_scanmode2->setGeometry(xpos, ypos, buttonw2, buttonh);
   lbl_scanmode2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_scanmode2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_scanmode2->setAlignment(AlignCenter|AlignVCenter);

   xpos = spacing;
   ypos += buttonh + spacing;
   
   lbl_temperature1 = new QLabel(tr(" Temperature:"),this);
   lbl_temperature1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_temperature1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_temperature1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_temperature1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));   
   lbl_temperature1->setAlignment(AlignLeft|AlignVCenter);

   xpos += spacing + buttonw;
   
   lbl_temperature2 = new QLabel("", this);
   lbl_temperature2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_temperature2->setGeometry(xpos, ypos, buttonw2, buttonh);
   lbl_temperature2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_temperature2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_temperature2->setAlignment(AlignCenter|AlignVCenter);

   xpos = spacing;
   ypos += buttonh + spacing;
   
   lbl_rpm1 = new QLabel(tr(" Rotor Speed:"),this);
   lbl_rpm1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_rpm1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_rpm1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_rpm1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));   
   lbl_rpm1->setAlignment(AlignLeft|AlignVCenter);

   xpos += spacing + buttonw;
   
   lbl_rpm2 = new QLabel("", this);
   lbl_rpm2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_rpm2->setGeometry(xpos, ypos, buttonw2, buttonh);
   lbl_rpm2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_rpm2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_rpm2->setAlignment(AlignCenter|AlignVCenter);

   xpos = spacing;
   ypos += buttonh + spacing;
   
   lbl_time1 = new QLabel(tr(" Scan Time:"),this);
   lbl_time1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_time1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_time1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_time1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));   
   lbl_time1->setAlignment(AlignLeft|AlignVCenter);

   xpos += spacing + buttonw;
   
   lbl_time2 = new QLabel("", this);
   lbl_time2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_time2->setGeometry(xpos, ypos, buttonw2, buttonh);
   lbl_time2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_time2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_time2->setAlignment(AlignCenter|AlignVCenter);

   xpos = spacing;
   ypos += buttonh + spacing;
   
   lbl_omega1 = new QLabel(tr(" Omega-Square-t:"),this);
   lbl_omega1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_omega1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_omega1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_omega1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));   
   lbl_omega1->setAlignment(AlignLeft|AlignVCenter);

   xpos += spacing + buttonw;
   
   lbl_omega2 = new QLabel("", this);
   lbl_omega2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_omega2->setGeometry(xpos, ypos, buttonw2, buttonh);
   lbl_omega2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_omega2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_omega2->setAlignment(AlignCenter|AlignVCenter);

   xpos = spacing;
   ypos += buttonh + spacing;
   
   lbl_lambda1 = new QLabel(tr(" Wavelength:"),this);
   lbl_lambda1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_lambda1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_lambda1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_lambda1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));   
   lbl_lambda1->setAlignment(AlignLeft|AlignVCenter);

   xpos += spacing + buttonw;
   
   lbl_lambda2 = new QLabel("", this);
   lbl_lambda2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_lambda2->setGeometry(xpos, ypos, buttonw2, buttonh);
   lbl_lambda2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_lambda2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_lambda2->setAlignment(AlignCenter|AlignVCenter);

   xpos = spacing;
   ypos += buttonh + spacing;
   
   lbl_repetitions1 = new QLabel(tr(" Repetitions:"),this);
   lbl_repetitions1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_repetitions1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_repetitions1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_repetitions1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));   
   lbl_repetitions1->setAlignment(AlignLeft|AlignVCenter);

   xpos += spacing + buttonw;
   
   lbl_repetitions2 = new QLabel("", this);
   lbl_repetitions2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_repetitions2->setGeometry(xpos, ypos, buttonw2, buttonh);
   lbl_repetitions2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_repetitions2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_repetitions2->setAlignment(AlignCenter|AlignVCenter);

   xpos = spacing;
   ypos += buttonh + spacing;
   
   lbl_datapoints1 = new QLabel(tr(" Datapoints:"),this);
   lbl_datapoints1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_datapoints1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_datapoints1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_datapoints1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));   
   lbl_datapoints1->setAlignment(AlignLeft|AlignVCenter);

   xpos += spacing + buttonw;
   
   lbl_datapoints2 = new QLabel("", this);
   lbl_datapoints2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_datapoints2->setGeometry(xpos, ypos, buttonw2, buttonh);
   lbl_datapoints2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_datapoints2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_datapoints2->setAlignment(AlignCenter|AlignVCenter);

   xpos = spacing;
   ypos += buttonh + spacing;
   
   lbl_filesize1 = new QLabel(tr(" Filesize:"),this);
   lbl_filesize1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_filesize1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_filesize1->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_filesize1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));   
   lbl_filesize1->setAlignment(AlignLeft|AlignVCenter);

   xpos += spacing + buttonw;
   
   lbl_filesize2 = new QLabel("", this);
   lbl_filesize2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_filesize2->setGeometry(xpos, ypos, buttonw2, buttonh);
   lbl_filesize2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_filesize2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_filesize2->setAlignment(AlignCenter|AlignVCenter);

   xpos = spacing;
   ypos += buttonh + spacing;
   
   lbl_pt_density1 = new QLabel(tr(" Datapoint Density:"),this);
   lbl_pt_density1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_pt_density1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_pt_density1->setGeometry(xpos, ypos, buttonw0, buttonh);
   lbl_pt_density1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));   
   lbl_pt_density1->setAlignment(AlignCenter|AlignVCenter);

   ypos += buttonh + spacing;
   
   lbl_pt_density2 = new QLabel("", this);
   lbl_pt_density2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_pt_density2->setGeometry(xpos, ypos, buttonw0, buttonh);
   lbl_pt_density2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_pt_density2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_pt_density2->setAlignment(AlignCenter|AlignVCenter);
   
   ypos += buttonh + spacing;

   global_Xpos += 30;
   global_Ypos += 30;
   
   setMinimumSize(720, ypos);
   setGeometry(global_Xpos, global_Ypos, 720, ypos);
   
   setup_GUI();
}

US_Diagnostics::~US_Diagnostics()
{
}

void US_Diagnostics::setup_GUI()
{
   int j=0;
   int rows = 14, columns = 2, spacing = 2;
   
   QGridLayout * background = new QGridLayout(this,2,2,spacing);   
   QGridLayout * subGrid1 = new QGridLayout(rows, columns, spacing);
   for (int i=0; i<rows; i++)
   {
      subGrid1->setRowSpacing(i, 26);
   }
   subGrid1->addMultiCellWidget(pb_load,j,j,0,1);
   j++;
   subGrid1->addMultiCellWidget(pb_print_delta,j,j,0,1);
   j++;
   subGrid1->addMultiCellWidget(pb_print_scan,j,j,0,1);
   j++;
   subGrid1->addMultiCellWidget(pb_convert,j,j,0,1);
   j++;
   subGrid1->addMultiCellWidget(pb_help,j,j,0,1);
   j++;
   subGrid1->addMultiCellWidget(pb_close,j,j,0,1);
   j++;
   subGrid1->addMultiCellWidget(banner1,j,j,0,1);
   j++;
   subGrid1->addMultiCellWidget(lbl_file1,j,j,0,1);
   j++;
   subGrid1->addMultiCellWidget(lbl_file2,j,j,0,1);
   j++;
   subGrid1->addWidget(lbl_cell1,j,0);
   subGrid1->addWidget(lbl_cell2,j,1);
   j++;
   subGrid1->addWidget(lbl_scanmode1,j,0);
   subGrid1->addWidget(lbl_scanmode2,j,1);
   j++;
   subGrid1->addWidget(lbl_temperature1,j,0);
   subGrid1->addWidget(lbl_temperature2,j,1);
   j++;
   subGrid1->addWidget(lbl_rpm1,j,0);
   subGrid1->addWidget(lbl_rpm2,j,1);
   j++;
   subGrid1->addWidget(lbl_time1,j,0);
   subGrid1->addWidget(lbl_time2,j,1);
   j++;
   subGrid1->addWidget(lbl_omega1,j,0);
   subGrid1->addWidget(lbl_omega2,j,1);
   j++;
   subGrid1->addWidget(lbl_lambda1,j,0);
   subGrid1->addWidget(lbl_lambda2,j,1);
   j++;
   subGrid1->addWidget(lbl_repetitions1,j,0);
   subGrid1->addWidget(lbl_repetitions2,j,1);
   j++;
   subGrid1->addWidget(lbl_datapoints1,j,0);
   subGrid1->addWidget(lbl_datapoints2,j,1);
   j++;
   subGrid1->addWidget(lbl_filesize1,j,0);
   subGrid1->addWidget(lbl_filesize2,j,1);
   j++;
   subGrid1->addMultiCellWidget(lbl_pt_density1,j,j,0,1);
   j++;
   subGrid1->addMultiCellWidget(lbl_pt_density2,j,j,0,1);
   
   background->addMultiCell(subGrid1,0,1,0,0);
   background->addWidget(delta_plot,0,1);
   background->addWidget(scan_plot,1,1);
   background->setColStretch(0,1);
   background->setColStretch(1,4);
   background->setColSpacing(1,530);

   qApp->processEvents();
   QRect r = background->cellGeometry(0, 0);
   
   global_Xpos += 30;
   global_Ypos += 30;
   
   this->setGeometry(global_Xpos, global_Ypos, r.width()+530+spacing*3, r.height());

}


void US_Diagnostics::closeEvent(QCloseEvent *e)
{
   e->accept();
   global_Xpos -= 30;
   global_Ypos -= 30;
}
/*
  void US_Diagnostics::resizeEvent(QResizeEvent *e)
  {
  int controlPanelWidth=224;
  delta_plot->setGeometry(controlPanelWidth, 
  2, 
  e->size().width()-2-controlPanelWidth, 
  (int) (4 * (e->size().height()/7) - 3));
  scan_plot->setGeometry (controlPanelWidth, 
  (int) ((4 * e->size().height()/7) + 1), 
  e->size().width()-2-controlPanelWidth, 
  (int) ((3 * e->size().height()/7) - 3));
  }
*/
void US_Diagnostics::load()
{
   vector <float> sigma;
   QString fn = QFileDialog::getOpenFileName(USglobal->config_list.data_dir, "*", 0);
   if ( !fn.isEmpty() ) 
   {
      load(fn);
   }
}

void US_Diagnostics::load(const QString &fn)
{
   rad.clear();
   abs.clear();
   vector <float> sigma;
   QFile f;
   converted = false;
   unsigned int i;
   QString rpm, omega, lambda, repetitions, str;
   f.setName(fn);
   lbl_filesize2->setText(str.sprintf("%ld bytes", f.size()));
   lbl_file2->setText(fn);
   if (f.open(IO_ReadOnly))
   {
      QTextStream ts(&f);
      if (!ts.eof())
      {
         cell_info = ts.readLine();
         ts >> scanmode;
         if (scanmode == "P")
         {
            scanmode = "Interference";
            scan_plot->setAxisTitle(QwtPlot::yLeft, tr("Fringes"));
         }
         else if (scanmode == "R")
         {
            scanmode = "Radial";
            scan_plot->setAxisTitle(QwtPlot::yLeft, tr("Absorbance"));
            scan_plot->setAxisTitle(QwtPlot::xBottom, tr("Radius (cm)"));
            delta_plot->setAxisTitle(QwtPlot::yLeft, tr("Delta-Radius"));
         }
         else if (scanmode == "W")
         {
            scanmode = "Wavelength";
            scan_plot->setAxisTitle(QwtPlot::yLeft, tr("Absorbance"));
            scan_plot->setAxisTitle(QwtPlot::xBottom, tr("Wavelength (nm)"));
            delta_plot->setAxisTitle(QwtPlot::yLeft, tr("Delta-lambda"));
         }
         else if (scanmode == "I")
         {
            scanmode = "Intensity";
            scan_plot->setAxisTitle(QwtPlot::yLeft, tr(scanmode));
         }
         else if (scanmode == "V")
         {
            scanmode = "Wavelength Intensity";
            scan_plot->setAxisTitle(QwtPlot::yLeft, tr("Wavelength Intensity"));
         }
         else if (scanmode == "F")
         {
            scanmode = "Fluorescence Intensity";
            scan_plot->setAxisTitle(QwtPlot::yLeft, tr("Fluorescence Intensity"));
         }
         else
         {
            scan_plot->clear();
            scan_plot->setTitle("Scan Plot");
            scan_plot->replot();
            delta_plot->clear();
            delta_plot->replot();
            lbl_scanmode2->setText("");
            lbl_cell2->setText("");
            lbl_temperature2->setText("");
            lbl_rpm2->setText("");
            lbl_time2->setText("");
            lbl_omega2->setText("");
            lbl_lambda2->setText("");
            lbl_repetitions2->setText("");
            lbl_pt_density2->setText("");
            lbl_datapoints2->setText("");
            QMessageBox::message(tr("Attention:"), tr("You loaded an invalid file.\n\nPlease try again..."));
            return;
         }
         lbl_scanmode2->setText(scanmode);
         ts >> cell;
         lbl_cell2->setText(cell);
         ts >> temperature;
         lbl_temperature2->setText(temperature + " ºC");
         ts >> rpm;
         lbl_rpm2->setText(rpm + " rpm");
         ts >> time;
         i = time.toUInt();
         str.sprintf("%d sec", i);
         lbl_time2->setText(str);
         ts >> omega;
         lbl_omega2->setText(omega);
         ts >> lambda;
         if (scanmode == "Wavelength" || scanmode == "Wavelength Intensity")  // then this variable is not really lambda, it is radial stepsize
         {
            lbl_lambda1->setText("Radial Position:");
            lbl_lambda2->setText(lambda + " cm");
         }
         else
         {
            lbl_lambda1->setText("Wavelength:");
            lbl_lambda2->setText(lambda + " nm");
            scan_plot->setAxisTitle(QwtPlot::xBottom, tr("Radius (cm)"));
         }
         ts >> repetitions;
         lbl_repetitions2->setText(repetitions);
      }
      QString filestr, valstr;
      filestr = ts.readLine();
      while (!ts.eof())
      {
         filestr = ts.readLine();
         //cout << "Line: " << filestr << ", 1st token: ";
         valstr = getToken(&filestr, " ");
         //cout << valstr << ", 2nd token: ";
         rad.push_back(valstr.toFloat());
         valstr = getToken(&filestr, " ");
         //cout << valstr << ", Sigma: ";
         abs.push_back(valstr.toFloat());
         if (filestr.stripWhiteSpace() != "")
         {
            valstr = getToken(&filestr, " ");
            sigma.push_back(valstr.toFloat());
            //cout << valstr;
         }
         //cout << endl;
      }
      f.close();
   }
   plot_graph();
}

void US_Diagnostics::plot_graph()
{
   QString str;
   double *radius, *absorbance, *delta, *points;
   unsigned int num = rad.size() - 1, delta_curve, scan_curve;
   radius = new double [num];
   absorbance = new double [num];
   unsigned int i;
   delta = new double [num - 1];
   points = new double [num - 1];
   for (i=0; i<num; i++)
   {
      radius[i] = (double) rad[i];
      absorbance[i] = (double) abs[i];
   }
   float density=0.0;
   for (i=1; i<num; i++)
   {
      delta[i-1] = (double) (rad[i] - rad[i-1]);
      density += delta[i-1];
      points[i-1] = (double) (i-1);
   }
   density /= (num - 1);
   str.sprintf("%3.4e cm/point", density);
   lbl_pt_density2->setText(str);
   str.sprintf("%d", num);
   lbl_datapoints2->setText(str);
   
   delta_plot->clear();
   delta_curve = delta_plot->insertCurve("Delta Plot");
   delta_plot->setCurveStyle(delta_curve, QwtCurve::Lines);
   delta_plot->setCurvePen(delta_curve, yellow);
   delta_plot->setCurveData(delta_curve, points, delta, num-1);
   delta_plot->replot();
   
   scan_plot->clear();
   scan_curve = scan_plot->insertCurve("Scan Plot");
   scan_plot->setCurveStyle(scan_curve, QwtCurve::Lines);
   scan_plot->setCurvePen(scan_curve, yellow);
   scan_plot->setCurveData(scan_curve, radius, absorbance, num);
   scan_plot->setTitle(cell_info);
   scan_plot->replot();

   delete [] radius;
   delete [] absorbance;
   delete [] delta;
   delete [] points;
}

void US_Diagnostics::quit()
{
   close();
}

void US_Diagnostics::convert()
{
   if (scanmode == "Intensity" && !converted)
   {
      float maxval = -1000.0;
      unsigned int i;
      for (i=0; i<abs.size(); i++)
      {
         maxval = max(abs[i], maxval);
      }
      for (i=0; i<abs.size(); i++)
      {
         abs[i] = log10(maxval/abs[i]);
      }
      scan_plot->setAxisTitle(QwtPlot::yLeft, tr("Pseudo-Absorbance"));
      plot_graph();
      converted = true;
   }
}

void US_Diagnostics::print_delta()
{
   QPrinter printer;
   if   (printer.setup(0))
   {
      //delta_plot->print(printer, QwtFltrInv());            //old version
      delta_plot->print(printer, QwtPlotPrintFilter());      //new version
   }
}

void US_Diagnostics::print_scan()
{
   QPrinter printer;
   if   (printer.setup(0))
   {
      //scan_plot->print(printer, QwtFltrInv());            //old version
      scan_plot->print(printer, QwtPlotPrintFilter());      //new version

   }
}

void US_Diagnostics::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/diagnostics.html");
}
