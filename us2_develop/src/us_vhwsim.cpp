#include "../include/us_vhwsim.h"

Extrapolation_Legend_F::Extrapolation_Legend_F(int divisions, QWidget *p, const char* name)
   : QFrame(p, name)
{
   USglobal = new US_Config();

   division_lbl = new QLabel(tr("  Divisions:"), this);
   Q_CHECK_PTR(division_lbl);
   division_lbl->setAlignment(AlignLeft|AlignVCenter);
   division_lbl->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   division_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   division_lbl->setGeometry(7, 7, 210, 26);

   division_counter= new QwtCounter(this);
   Q_CHECK_PTR(division_counter);
   division_counter->setRange(10, 100, 1);
   division_counter->setValue(divisions);
   division_counter->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   division_counter->setGeometry(90,9,120,22);

   progress_lbl = new QLabel(tr("    Analysis Progress:"), this);
   Q_CHECK_PTR(progress_lbl);
   progress_lbl->setAlignment(AlignLeft|AlignVCenter);
   progress_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   progress_lbl->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   progress_lbl->setGeometry(210,7,342,26);

   progress = new QProgressBar(100, this, "Analysis Progress");
   Q_CHECK_PTR(progress);
   progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   progress->setGeometry(350, 10, 195, 20);

   dist = new QPushButton(tr("Distribution Plot"), this);
   Q_CHECK_PTR(dist);
   dist->setAutoDefault(false);
   dist->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   dist->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   dist->setGeometry(7, 40, 140, 26);

   save = new QPushButton(tr("Save"), this);
   Q_CHECK_PTR(save);
   save->setAutoDefault(false);
   save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   save->setGeometry(157, 40, 60, 26);

   close = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(close);
   close->setAutoDefault(false);
   close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   close->setGeometry(227, 40, 80, 26);
   close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
}

Extrapolation_Legend_F::~Extrapolation_Legend_F()
{
   delete division_lbl;
   delete division_counter;
   delete progress_lbl;
   delete progress;
   delete dist;
   delete save;
   delete close;   
}

Distribution_Legend_F::Distribution_Legend_F(QWidget *p, const char* name)
   : QFrame(p, name)
{
   USglobal = new US_Config();
   setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
   
   int buttonh = 26, buttonw = 150, spacing = 2, xpos = 2, ypos = 2;

   expand = new QPushButton(tr("Zoom out"), this);
   Q_CHECK_PTR(expand);
   expand->setAutoDefault(false);
   expand->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   expand->setGeometry(xpos, ypos, buttonw, buttonh);
   expand->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   
   xpos += buttonw + spacing;

   compress = new QPushButton(tr("Zoom in"), this);
   Q_CHECK_PTR(compress);
   compress->setAutoDefault(false);
   compress->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   compress->setGeometry(xpos, ypos, buttonw, buttonh);
   compress->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   
   xpos += buttonw + spacing;

   pb_histogram = new QPushButton(tr("Histogram"), this);
   Q_CHECK_PTR(pb_histogram);
   pb_histogram->setAutoDefault(false);
   pb_histogram->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_histogram->setGeometry(xpos, ypos, buttonw, buttonh);
   pb_histogram->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));

   xpos += buttonw + spacing;

   pb_print = new QPushButton(tr("Print"), this);
   Q_CHECK_PTR(pb_print);
   pb_print->setAutoDefault(false);
   pb_print->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_print->setGeometry(xpos, ypos, buttonw, buttonh);
   pb_print->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));

   xpos = spacing;
   ypos += buttonh + spacing;
   
   pb_histogram_plot = new QPushButton(tr("Hide Histogram"), this);
   Q_CHECK_PTR(pb_histogram_plot);
   pb_histogram_plot->setAutoDefault(false);
   pb_histogram_plot->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_histogram_plot->setGeometry(xpos, ypos, buttonw, buttonh);
   pb_histogram_plot->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   
   xpos += buttonw + spacing;

   lbl_sensitivity_histogram = new QLabel(tr("Sensitivity:"),this);
   lbl_sensitivity_histogram->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_sensitivity_histogram->setPalette(QPalette( USglobal->global_colors.cg_label, 
                                                   USglobal->global_colors.cg_label, 
                                                   USglobal->global_colors.cg_label));
   lbl_sensitivity_histogram->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_sensitivity_histogram->setFont(QFont( USglobal->config_list.fontFamily, 
                                             USglobal->config_list.fontSize - 1, 
                                             QFont::Bold));

   xpos += buttonw + spacing;

   ct_histogram= new QwtCounter(this);
   Q_CHECK_PTR(ct_histogram);
   ct_histogram->setRange(10, 100, 1);
   ct_histogram->setPalette( QPalette( USglobal->global_colors.cg_normal, 
                                       USglobal->global_colors.cg_normal, 
                                       USglobal->global_colors.cg_normal) );

   ct_histogram->setFont(QFont( USglobal->config_list.fontFamily, 
                                USglobal->config_list.fontSize - 1) );

   ct_histogram->setGeometry(xpos, ypos, buttonw, buttonh);

   xpos += buttonw + spacing;

   close = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(close);
   close->setAutoDefault(false);
   close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   close->setGeometry(xpos, ypos, buttonw, buttonh);
   
   xpos = spacing;
   ypos += buttonh + spacing;
   
   pb_envelope_plot = new QPushButton(tr("Hide Envelope"), this);
   Q_CHECK_PTR(pb_envelope_plot);
   pb_envelope_plot->setAutoDefault(false);
   pb_envelope_plot->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_envelope_plot->setGeometry(xpos, ypos, buttonw, buttonh);
   pb_envelope_plot->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));

   xpos += buttonw + spacing;

   lbl_sensitivity_envelope = new QLabel(tr("Smoothing:"),this);
   lbl_sensitivity_envelope->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_sensitivity_envelope->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_sensitivity_envelope->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_sensitivity_envelope->setFont(QFont( USglobal->config_list.fontFamily, 
                                            USglobal->config_list.fontSize - 1, 
                                            QFont::Bold));

   xpos += buttonw + spacing;

   ct_envelope= new QwtCounter(this);
   Q_CHECK_PTR(ct_envelope);
   ct_envelope->setRange(10, 100, 1);
   ct_envelope->setPalette( QPalette( USglobal->global_colors.cg_normal, 
                                      USglobal->global_colors.cg_normal, 
                                      USglobal->global_colors.cg_normal));

   ct_envelope->setFont(QFont( USglobal->config_list.fontFamily, 
                               USglobal->config_list.fontSize - 1 ) );

   ct_envelope->setGeometry(xpos, ypos, buttonw, buttonh);

   xpos += buttonw + spacing;
   
   help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(help);
   help->setAutoDefault(false);
   help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   help->setGeometry(xpos, ypos, buttonw, buttonh);

   setMinimumSize(610, 550);
}

Distribution_Legend_F::~Distribution_Legend_F()
{
   delete expand;
   delete compress;
   delete pb_print;
   delete pb_histogram;
   delete close;
   delete pb_histogram_plot;
   delete lbl_sensitivity_histogram;
   delete pb_envelope_plot;
   delete lbl_sensitivity_envelope;
}

vHW_Sim::vHW_Sim(int mod, unsigned int *comps, unsigned int *pts, int *run_flag,
                 vector <struct component> *components, double ***concentration, 
                 struct simulation_parameters *simparams, float **scan_t,
                 float temp_conc, float temp_k, QWidget *p , const char *name) : QWidget(p, name)
{
   
   USglobal = new US_Config();

   setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
   flag = 0;
   divisions=50;
   comp = *comps;
   setup_already = false;
   model = mod;
   total_conc = temp_conc;
   equil_k = temp_k;
   points = *pts;
   temp_components = components;
   temp_sp = simparams;
   c = concentration;
   scantimes = scan_t;
   omega = (double) (M_PI * (*temp_sp).speed/30);
   omega = square(omega);
   ext_plot = new QwtPlot(this);
   Q_CHECK_PTR(ext_plot);
   ext_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
   ext_plot->setAxisTitle(QwtPlot::yLeft, tr("Sedimentation Coefficient x 10^(13) sec"));
   ext_plot->setAxisTitle(QwtPlot::xBottom, tr("[ Time (sec) ]^(-0.5)"));
   ext_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   ext_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
   ext_plot->setCanvasBackground(USglobal->global_colors.plot);      //new version
   ext_plot->setCanvasBackground(black);      //new version
   ext_plot->enableOutline(true);
   ext_plot->setOutlinePen(white);
   ext_plot->setOutlineStyle(Qwt::Cross);
   ext_plot->setMargin(USglobal->config_list.margin);
   ext_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   ext_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   ext_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   ext_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   ext_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   ext_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   ext_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   ext_legend = new Extrapolation_Legend_F(divisions, this);
   Q_CHECK_PTR(ext_legend);
   ext_legend->setFrameStyle(QFrame::Panel|QFrame::Raised);
   ext_legend->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
   ext_legend->setLineWidth(2);

   connect(ext_legend->dist, SIGNAL(clicked()), this, SLOT(distrib()));
   //   connect(ext_legend->division_counter, SIGNAL(buttonReleased(double)), this, SLOT(update_div(double)));
   connect(ext_legend->division_counter, SIGNAL(valueChanged(double)), this, SLOT(update_div(double)));
   connect(ext_legend->save, SIGNAL(clicked()), this, SLOT(save_vhw()));
   connect(ext_legend->close, SIGNAL(clicked()), this, SLOT(close_vhw()));
   recalc();
   *run_flag = flag;
   
   global_Xpos += 30;
   global_Ypos += 30;
   
   move(global_Xpos, global_Ypos);
}

vHW_Sim::~vHW_Sim()
{
}

void vHW_Sim::closeEvent(QCloseEvent *e)
{
   e->accept();
   global_Xpos -= 30;
   global_Ypos -= 30;
}

void vHW_Sim::resizeEvent(QResizeEvent *e)
{
   QRect r(0,0, e->size().width(), (int) (e->size().height()-77));
   ext_plot->setGeometry(r);
   ext_legend->setGeometry(0, e->size().height()-75, r.width(), 73);
}

void vHW_Sim::recalc()
{
   QwtSymbol sym;
   sym.setStyle(QwtSymbol::Ellipse);
   sym.setPen(blue);
   sym.setBrush(white);
   sym.setSize(8);

   double a, b;
   int step=0;
   if (setup_already)   // if the program has been run before, memory has already been allocated and needs
   {                     // to be deallocated before re-allocating again to avoid memory leaks
      cleanup();
   }
   setups();
   if (flag == 1)
   {
      return;
   }
   for (i=exclude; i<(*temp_sp).scans; i++)
   {
      interval = plateau[i]/(divisions+1);

      testy=0;
      for (j=0; j<divisions; j++)
      {
         testy += interval;  // increment the division tester by one interval
         count=1;
         while ((*c)[i][count] < testy)
         {
            count++;      // climb up the concentration gradient until one 
         }               // point beyond testy is reached
         //
         // Now we need a linear interpolation between the last point before and the 
         // first point beyond testy. The radius corresponding to the interpolated 
         // concentration of testy on that line is the radius needed for the calculation
         // of the apparent sedimentation coefficient. If the test concentration is equal
         // to a point in the concentration matrix, we don't have to interpolate and can
         // use the point count instead to obtain the correct radius:
         //
         if ((*c)[i][count] == testy)
         {
            temp_r = (*temp_sp).meniscus + count * (*temp_sp).resolution;
         }
         else
         {
            //
            // solve: y = a*x + b:
            //
            a = ((*c)[i][count] - (*c)[i][count-1])/((*temp_sp).resolution);
            b = (*c)[i][count] - a * ((*temp_sp).meniscus + count * (*temp_sp).resolution);
            temp_r = (testy - b)/a;
         }
         //cout << "r: " << temp_r << ", meniscus: " << (*temp_sp).meniscus << ", omega: " << omega 
         //<< ", times: " << (*scantimes)[i] << endl;
         sed_app[i-exclude][j] = (log(temp_r/(*temp_sp).meniscus))/(omega * (*scantimes)[i]) * 1.0e13;
         // (first dimension for scans, second for divisions)
      }
      step++;
      ext_legend->progress->setProgress(step);
   }
   //
   // for each division, fit the sed_app's of all scans to a straight line, the intercept
   // represents the distribution, the slopes can be saved for other things (i.e, Diffusion).
   //
   ext_plot->clear();
   ext_plot->enableGridXMin();
   ext_plot->enableGridYMin();
   ext_plot->setGridMajPen(QPen(colorGroup().light(), 0, DotLine));
   ext_plot->setGridMinPen(QPen(colorGroup().mid(), 0 , DotLine));
   for (j=0; j<divisions; j++)
   {
      for (i=exclude; i<(*temp_sp).scans; i++)
      {
         tempx[i - exclude] = 1.0/sqrt((*scantimes)[i]);
         tempy[i - exclude] = sed_app[i-exclude][j];
      }
      linefit(&tempx, &tempy, &slope[j], &intercept[j], &sigma, &correl, ((*temp_sp).scans - exclude));
      sa_points[j] = ext_plot->insertCurve("Apparent Sedimentation Coefficients");
      ext_plot->setCurveStyle(sa_points[j], QwtCurve::NoCurve);
      ext_plot->setCurveSymbol(sa_points[j], QwtSymbol(sym));
      ext_plot->setCurveData(sa_points[j], tempx, tempy, (*temp_sp).scans-exclude);
   }
   for (j=0; j<divisions; j++)
   {
      sa_lines[j] = ext_plot->insertCurve("Sedimentation Extrapolations");
      ext_plot->setCurveStyle(sa_lines[j], QwtCurve::Lines);
      lines_x[0] = 0.0;
      lines_x[1] = 1.0/sqrt((*scantimes)[exclude]) + 1.0/(20.0 * sqrt((*scantimes)[exclude]));
      lines_y[0] = intercept[j];
      lines_y[1] = slope[j] * lines_x[1] + lines_y[0];
      ext_plot->setCurvePen(sa_lines[j], yellow);
      ext_plot->setCurveData(sa_lines[j], lines_x, lines_y, 2);
   }
   ext_plot->replot();
}

void vHW_Sim::setups()
{
   //
   // Calculate expected plateaus for each scan by summing the plateau contributions 
   // of each component:
   //
   plateau = new double[(*temp_sp).scans];
   for (i=0; i<(*temp_sp).scans; i++)
   {
      plateau[i] = 0.0;
      for (j=0; j<comp; j++)
      {
         //
         // This is the equation for radial dilution:
         //
         plateau[i] += (*temp_components)[j].conc * exp(-2.0 * 
                                                        (*temp_components)[j].sed * omega * (*scantimes)[i]);
      }
      //      cout << "Time: " << i << ": " << (*scantimes)[i] << ", Plateau: " << plateau[i] << endl;
   }
   // 
   // make sure that the first datapoint of each scan has a lower concentration than 
   // what would be needed for the first division:
   //
   exclude=0;
   for (i=0; i<(*temp_sp).scans; i++)
   {
      if ((*c)[i][0] >= plateau[i]/(divisions+1))
      {
         exclude++;
      }
   }
   if (((*temp_sp).scans - exclude) < 3)
   {
      QMessageBox::message( tr("Warning"), tr("Attention:\n\nNot enough scans have cleared the meniscus.\n"
                                              "In order to perform a meaningful van Holde - Weischet Analysis,\n"
                                              "you need at least 3 scans that have cleared the meniscus.\n"
                                              "You can try to increase the length of the experiment or use\n"
                                              "a higher speed for the same sedimentation coefficient.\n"
                                              "Alternatively, you can try to use fewer divisions, or increase\n"
                                              "the sedimentation coefficient and decrease the diffusion\n"
                                              "coefficient for the slowest component(s).\n"));
      flag=1;
      return;
   }
   ext_legend->progress->setTotalSteps((*temp_sp).scans - exclude);
   ext_legend->progress->reset();
   ext_legend->progress->setProgress(0);

   tempx = new double [(*temp_sp).scans - exclude];
   tempy = new double [(*temp_sp).scans - exclude];
   sed_app = new double* [(*temp_sp).scans - exclude]; // first dimension for scans
   for (i=0; i < ((*temp_sp).scans - exclude); i++)
   {
      sed_app[i] = new double [divisions]; // second dimension for divisions
   }
   slope = new double [divisions];
   intercept = new double [divisions];
   boundary_fractions = new double [divisions];
   sa_points = new uint [divisions];
   sa_lines = new uint [divisions];
   lines_y = new double [2];
   lines_x = new double [2];
   setup_already = true;
}

void vHW_Sim::cleanup()
{
   delete [] tempx;
   delete [] tempy;
   for (i=0; i < ((*temp_sp).scans - exclude); i++)
   {
      delete [] sed_app[i]; // second dimension for divisions
   }
   delete [] sed_app; // first dimension for scans
   delete [] slope;
   delete [] intercept;
   delete [] boundary_fractions;
   delete [] sa_points;
   delete [] sa_lines;
   delete [] lines_y;
   delete [] lines_x;
}

void vHW_Sim::distrib()
{

   unsigned int i;
   USglobal = new US_Config();
   QString htmlDir = USglobal->config_list.html_dir + "/simulated";
   QDir d(htmlDir);
   if (!d.exists())
   {
      d.mkdir(htmlDir, true);
   }
   for (i=1; i<=divisions; i++)
   {
      boundary_fractions[i-1] = (double) (i * 100/(divisions+1));
   }
   dis = new Distribution("simulation", &divisions, &boundary_fractions, &intercept, htmlDir, "Simulated Data", 0, 0);
   dis->setCaption(tr("van Holde - Weischet Distribution Plot:"));
   dis->setMinimumSize(560,480);
   dis->setGeometry(0, 0, 560, 480);
   dis->show();
}

void vHW_Sim::update_div(double div)
{
   divisions = (unsigned int) div;
   recalc();
}

void vHW_Sim::save_vhw()
{
   QString fn = QFileDialog::getSaveFileName( 0, "*.vhw", 0);
   int k;
   if ( !fn.isEmpty() ) 
   {
      k = fn.find(".", 0, FALSE);
      if (k != -1) //if an extension was given, strip it
      {
         fn.truncate(k);
      }
      save_vhw( fn );      // the user gave a file name
   }
}

void vHW_Sim::save_vhw(const char* filename)
{
   QString filestr1, filestr2;
   filestr1 = QString(filename);
   filestr2 = QString(filename);
   filestr1.append(".vhw");
   filestr2.append(".dis");
   QFile vhw_f(filestr1);
   unsigned int i, j;   
   if (vhw_f.exists())
   {
      if(!QMessageBox::query( tr("Warning"), tr("Attention:\nThis file exists already!\n\nDo you want to overwrite it?"), tr("Yes"), tr("No")))
      {
         vhw_f.close();
         return;
      }
   }
   if (vhw_f.open(IO_WriteOnly | IO_Translate))
   {
      QTextStream ts(&vhw_f);
      //
      // Arrange the file as follows: First, write to the file all apparent sedimentation
      // values, for each scan time one row which contains all divisions, starting at the 
      // lowest concentration. The first column has the times of each scan, then follow
      // the sedimentation coefficients for each division for that time:
      //
      for (i=exclude; i<(*temp_sp).scans; i++)
      {   
         ts << (1.0 / pow((double) (*scantimes)[i], (double) 0.5)) << "\t";
         for (j=0; j<divisions-1; j++)
         {
            ts << sed_app[i-exclude][j] << "\t";
         }
         ts << sed_app[i-exclude][j] << "\n";
      }
      vhw_f.close();
   }
   //
   // Print the distributions, slopes and intercepts to a file and include all 
   // statistical information obtained from the linefits:
   //
   QFile dis_f(filestr2);
   if (dis_f.open(IO_WriteOnly | IO_Translate))
   {
      QTextStream ts(&dis_f);
      ts << tr("Division\tAverage\tSlope\tIntercept\tSigma\tCorrelation\n");
      for (j=0; j<divisions; j++)
      {
         ts << (float) (j+1)/ (float)(divisions+1) << "\t";
         for (i=exclude; i<(*temp_sp).scans; i++)
         {
            tempx[i - exclude] = 1.0/sqrt((*scantimes)[i]);
            tempy[i - exclude] = sed_app[i-exclude][j];
         }
         ts << linefit(&tempx, &tempy, &slope[j], &intercept[j], &sigma, &correl, ((*temp_sp).scans - exclude)) << "\t";
         ts << slope[j] << "\t";
         ts << intercept[j] << "\t";
         ts << sigma << "\t";
         ts << correl << "\n";
      }
      dis_f.close();
   }
}

void vHW_Sim::close_vhw()
{
   //
   // temp_components[0/1].conc are now set to partial concentrations, 
   // reset to total concentration and equilibrium constant before returning
   //
   if (model == 1 || model == 2)
   {
      (*temp_components)[0].conc = total_conc;
      (*temp_components)[1].conc = equil_k;
   }
   close();
}

Distribution::Distribution(QString temp_run_name, unsigned int *pts, double ** boundary_fractions, double **intercepts,
                           const QString temp_htmlDir, const QString title, const int temp_cell, const int temp_lambda,
                           QWidget *p , const char *name) : QWidget(p, name)
{
   USglobal = new US_Config();
   setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
   
   pm = new US_Pixmap();
   run_name = temp_run_name;
   histogram_is_hidden = false;
   envelope_is_hidden = false;
   close_flag = false;
   divisions = *pts;
   fractions = boundary_fractions;   
   htmlDir = temp_htmlDir;
   cell = temp_cell;
   lambda = temp_lambda;
   sed = intercepts;
   
   smoothval = 30;
   sensitivity = 50;
   dis_plot = new QwtPlot(this);
   Q_CHECK_PTR(dis_plot);
   dis_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
   dis_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   dis_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
   //dis_plot->setPlotBackground(USglobal->global_colors.plot);      //old version
   dis_plot->setCanvasBackground(USglobal->global_colors.plot);      //new version
   dis_plot->setMargin(USglobal->config_list.margin);
   dis_plot->enableOutline(true);
   dis_plot->setAxisTitle(QwtPlot::xBottom, tr("Sedimentation Coefficient x 10^(13) sec"));
   dis_plot->setAxisTitle(QwtPlot::yLeft, tr("% of Total Concentration"));
   dis_plot->setOutlinePen(white);
   dis_plot->setOutlineStyle(Qwt::Cross);
   dis_plot->setTitle(title);
   dis_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   dis_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   dis_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   dis_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   dis_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   dis_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   dis_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   dis_legend = new Distribution_Legend_F(this);
   Q_CHECK_PTR(dis_legend);
   dis_legend->setFrameStyle(QFrame::Panel|QFrame::Raised);
   dis_legend->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
   dis_legend->setLineWidth(2);
   dis_legend->ct_envelope->setValue(smoothval);
   dis_legend->ct_histogram->setValue(sensitivity);
   dis_legend->ct_histogram->setEnabled(false);
   dis_legend->ct_envelope->setEnabled(false);

   connect(dis_legend->expand, SIGNAL(clicked()), this, SLOT(expand()));
   connect(dis_legend->compress, SIGNAL(clicked()), this, SLOT(compress()));
   connect(dis_legend->pb_histogram, SIGNAL(clicked()), this, SLOT(show_histogram()));
   connect(dis_legend->pb_print, SIGNAL(clicked()), this, SLOT(print_plot()));
   connect(dis_legend->close, SIGNAL(clicked()), this, SLOT(close_dis()));
   connect(dis_legend->help, SIGNAL(clicked()), this, SLOT(help()));
   connect(dis_legend->pb_histogram_plot, SIGNAL(clicked()), this, SLOT(hide_histogram()));
   connect(dis_legend->pb_envelope_plot, SIGNAL(clicked()), this, SLOT(hide_envelope()));
   //   connect(dis_legend->ct_histogram, SIGNAL(buttonReleased(double)), this, SLOT(update_histogram(double)));
   connect(dis_legend->ct_histogram, SIGNAL(valueChanged(double)), this, SLOT(update_histogram(double)));
   //   connect(dis_legend->ct_envelope, SIGNAL(buttonReleased(double)), this, SLOT(update_envelope(double)));
   connect(dis_legend->ct_envelope, SIGNAL(valueChanged(double)), this, SLOT(update_envelope(double)));
   setGeometry(30,30, 610, 550);
   show_distribution();
}

void Distribution::show_distribution()
{
   dis_legend->pb_histogram_plot->setEnabled(false);
   dis_legend->pb_envelope_plot->setEnabled(false);
   dis_legend->ct_histogram->setEnabled(false);
   dis_legend->ct_envelope->setEnabled(false);
   histogram = false;
   QwtSymbol sym;
   sym.setStyle(QwtSymbol::Ellipse);
   sym.setPen(blue);
   sym.setBrush(white);
   sym.setSize(8);

   dis_legend->pb_histogram->setText("Histogram");
   disconnect(dis_legend->pb_histogram, SIGNAL(clicked()), this, SLOT(show_distribution()));
   connect(dis_legend->pb_histogram, SIGNAL(clicked()), this, SLOT(show_histogram()));
   dis_plot->clear();
   dis_plot->enableGridXMin();
   dis_plot->enableGridYMin();
   dis_plot->setGridMajPen(QPen(colorGroup().light(), 0, DotLine));
   dis_plot->setGridMinPen(QPen(colorGroup().mid(), 0 , DotLine));

   dis_curve = dis_plot->insertCurve("vhw distribution");
   dis_plot->setCurvePen(dis_curve, yellow);
   dis_plot->setCurveStyle(dis_curve, QwtCurve::Lines);
   dis_plot->setCurveSymbol(dis_curve, QwtSymbol(sym));
   dis_plot->setCurveData(dis_curve, *sed, *fractions, divisions);
   dis_plot->setAxisTitle(QwtPlot::yLeft, "Boundary Fraction");
   dis_plot->setTitle(tr("Sedimentation Coefficient Distribution"));

   m_min = qwtGetMin(*sed, divisions);
   m_max = qwtGetMax(*sed, divisions);
   diff = (m_max-m_min)/2;
   m_max += diff;
   m_min -= diff;
   dis_plot->setAxisScale(QwtPlot::xBottom, m_min, m_max);
   dis_plot->replot();
   diff = diff/2;
   update_pixmap();
}

Distribution::~Distribution()
{
}

void Distribution::closeEvent(QCloseEvent *e)
{
   close_flag = true;
   show_histogram_plot();
   e->accept();
}

void Distribution::resizeEvent(QResizeEvent *e)
{
   setMinimumSize(614, 550);
   dis_plot->setGeometry(2, 2, e->size().width()-4, e->size().height() - 92);
   dis_legend->setGeometry(2, e->size().height()-88, e->size().width(), 86);
}

void Distribution::print_plot()
{
   QPrinter printer;
   bool print_bw =  false;
   bool print_inv =  false;
   US_SelectPlot *sp;
   sp = new US_SelectPlot(&print_bw, &print_inv);
   sp->exec();
   if   (printer.setup(0))
   {
      if (print_bw)
      {
         PrintFilter pf;
         pf.setOptions(QwtPlotPrintFilter::PrintTitle
                       |QwtPlotPrintFilter::PrintMargin
                       |QwtPlotPrintFilter::PrintLegend
                       |QwtPlotPrintFilter::PrintGrid);
         dis_plot->print(printer, pf);
      }
      else if (print_inv)
      {
         PrintFilterDark pf;
         pf.setOptions(QwtPlotPrintFilter::PrintTitle
                       |QwtPlotPrintFilter::PrintMargin
                       |QwtPlotPrintFilter::PrintLegend
                       |QwtPlotPrintFilter::PrintGrid);
         dis_plot->print(printer, pf);
      }
      else
      {
         dis_plot->print(printer);
      }
   }
}

void Distribution::expand()
{
   m_min -= diff;
   m_max += diff;
   dis_plot->setAxisScale(QwtPlot::xBottom, m_min, m_max);
   dis_plot->replot();
}

void Distribution::compress()
{
   m_min += diff;
   m_max -= diff;
   dis_plot->setAxisScale(QwtPlot::xBottom, m_min, m_max);
   dis_plot->replot();
}

void Distribution::close_dis()
{
   close();
}

void Distribution::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/distribution.html");
}

void Distribution::update_pixmap()
{
   qApp->processEvents();
   QPixmap p;
   QString fileName;
   if (histogram)
   {
      fileName.sprintf(htmlDir + "/vhw_his_%d%d.", cell + 1, lambda + 1);
   }
   else
   {
      fileName.sprintf(htmlDir + "/vhw_dis_%d%d.", cell + 1, lambda + 1);
   }
   p = QPixmap::grabWidget(dis_plot, 2, 2, dis_plot->width() - 4, dis_plot->height() - 4);
   pm->save_file(fileName, p);
}

void Distribution::update_histogram(double val)
{
   sensitivity = val;
   show_histogram_plot();
}

void Distribution::update_envelope(double val)
{
   smoothval = val;
   show_histogram_plot();
}

void Distribution::hide_histogram()
{
   if (histogram_is_hidden)
   {
      histogram_is_hidden = false;
      dis_legend->pb_histogram_plot->setText("Hide Histogram");
      dis_legend->ct_histogram->setEnabled(true);
   }
   else
   {
      histogram_is_hidden = true;
      dis_legend->pb_histogram_plot->setText("Show Histogram");
      dis_legend->ct_histogram->setEnabled(false);
   }
   show_histogram_plot();
}

void Distribution::hide_envelope()
{
   if (envelope_is_hidden)
   {
      envelope_is_hidden = false;
      dis_legend->pb_envelope_plot->setText("Hide Envelope");
      dis_legend->ct_envelope->setEnabled(true);
   }
   else
   {
      envelope_is_hidden = true;
      dis_legend->pb_envelope_plot->setText("Show Envelope");
      dis_legend->ct_envelope->setEnabled(false);
   }
   show_histogram_plot();
}

void Distribution::show_histogram()
{
   dis_legend->ct_histogram->setEnabled(true);
   dis_legend->ct_envelope->setEnabled(true);
   show_histogram_plot();
}

void Distribution::show_histogram_plot()
{
   histogram = true;
   dis_legend->pb_histogram_plot->setEnabled(true);
   dis_legend->pb_envelope_plot->setEnabled(true);
   float max_intercept = -1e6, min_intercept = 1e6, sed_bin, lower, upper;
   float max_step, sigma, histogram_sum, envelope_sum, normalization;
   unsigned int j, k, steps, array=300, envelope;
   double *bin_count, *sval, *xval, *yval;
   
   for (j=0; j<divisions; j++)
   {
      max_intercept = max((float) max_intercept, (float) (*sed)[j]);
      min_intercept = min((float) min_intercept, (float) (*sed)[j]);
   }
   sed_bin = (max_intercept - min_intercept)/(sensitivity*divisions/100);
   /*
     for (j=0; j<divisions; j++)
     {
     sval[j] = min_intercept + (1 + j) * sed_bin - sed_bin/2;
     for (k=0; k<divisions; k++)
     {
     //         cout << "sed[" << j << "]: " << (*sed)[j] << ", k: " <<  k << endl;
     lower = min_intercept + k * sed_bin;
     upper = min_intercept + (1 + k) * sed_bin;
     if (((*sed)[j] >= lower) && ((*sed)[j] < upper))
     {
     bin_count[k] ++;
     }
     }
     }
   */
   max_step = max_intercept + max_intercept/3;
   steps = (unsigned int) (max_step/sed_bin);
   if (array <= steps)
   {
      array = steps + 1; // always make sure that array is at least one larger than steps
   }
   bin_count = new double [steps];
   sval = new double [steps];
   xval = new double [array];
   yval = new double [array];
   //   sigma = 0.6826 * sed_bin;
   sigma = 0.02 * smoothval * sed_bin;
   for (j=0; j<array; j++)
   {
      xval[j] = j * (max_step/array);
      yval[j] = 0.0;
   }
   for (j=0; j<steps; j++)
   { // assign the xvalue to be the center of the current bin (between zero and max intercept + 20%)
      sval[j] = (j + 1) * sed_bin - sed_bin/2; 
      bin_count[j] = 0; // assign the yvalue to zero
      lower = j * sed_bin; // lower limit of the current bin
      upper = (1 + j) * sed_bin;  // upper limit of the current bin
      for (k=0; k<divisions; k++) // check each sedimentation value for a match
      {
         if ((*sed)[k] >= lower && (*sed)[k] < upper)
         { //if the current svalue is between the limits, increment the current yplot by one count
            bin_count[j] += 1.0;
            //            cout << "sed: " << (*sed)[k] << ", upper: " << upper << ", lower: " << lower << ", bincount: " << bin_count[j] << endl;
         }
      }
      if(bin_count[j] > 0.0)   // add up basis functions of gaussians
      {
         for (k=0; k<array; k++)
         {
            yval[k] += ((bin_count[j]/(sigma * pow(2 * M_PI, 0.5))) * exp(- pow((xval[k] - sval[j])/sigma, 2.0)/2));
         }
      }
   }
   histogram_sum = 0;
   envelope_sum = 0;   
   for (j=0; j<steps; j++)
   {
      histogram_sum += bin_count[j] * sed_bin;
   }
   
   for (j=0; j<array; j++)
   {
      envelope_sum += yval[j] * xval[1];
   }
   
   normalization = envelope_sum/histogram_sum;
   
   for (j=0; j<array; j++)
   {
      yval[j] /= normalization;
   }

   dis_legend->pb_histogram->setText("Distribution");
   disconnect(dis_legend->pb_histogram, SIGNAL(clicked()), this, SLOT(show_histogram()));
   connect(dis_legend->pb_histogram, SIGNAL(clicked()), this, SLOT(show_distribution()));

   QwtSymbol sym;
   QPen p_histo, p_envelope;
   sym.setStyle(QwtSymbol::Ellipse);
   sym.setPen(blue);
   sym.setBrush(white);
   sym.setSize(11);
   p_histo.setWidth(5);
   p_histo.setColor(Qt::red);
   p_envelope.setWidth(3);
   p_envelope.setColor(Qt::cyan);
   sym.setStyle(QwtSymbol::Ellipse);
   sym.setPen(Qt::blue);
   sym.setBrush(Qt::yellow);
   dis_plot->clear();
   dis_plot->setAxisTitle(QwtPlot::xBottom, "Sedimentation Coefficient");
   dis_plot->setAxisTitle(QwtPlot::yLeft, "Relative Concentration");
   dis_plot->setTitle(tr("Sedimentation Coefficient Histogram"));
   dis_plot->clear();
   dis_plot->enableGridXMin();
   dis_plot->enableGridYMin();
   dis_plot->setGridMajPen(QPen(colorGroup().light(), 0, DotLine));
   dis_plot->setGridMinPen(QPen(colorGroup().mid(), 0 , DotLine));
   if (!histogram_is_hidden)
   {
      dis_curve = dis_plot->insertCurve("vhw histogram");
      dis_plot->setCurvePen(dis_curve, p_histo);
      dis_plot->setCurveStyle(dis_curve, QwtCurve::Sticks);
      dis_plot->setCurveData(dis_curve, sval, bin_count, steps);
   }
   if (!envelope_is_hidden)
   {
      envelope = dis_plot->insertCurve("histogram envelope");
      dis_plot->setCurveStyle(envelope, QwtCurve::Lines);
      dis_plot->setCurvePen(envelope, p_envelope);
      dis_plot->setCurveData(envelope, xval, yval, array);
   }
   m_min = qwtGetMin(sval, steps);
   m_max = qwtGetMax(sval, steps);
   diff = (m_max-m_min)/20;
   m_min = 0;
   m_max += m_max/5;
   dis_plot->setAxisScale(QwtPlot::xBottom, m_min, m_max);
   dis_plot->replot();
   if (close_flag)
   {
      update_pixmap();
      QString fileName;
      if (histogram)
      {
         fileName.sprintf(USglobal->config_list.result_dir + "/" + run_name + ".vhw_his.%d%d", cell + 1, lambda + 1);
         QFile f(fileName);
         if (f.open(IO_WriteOnly | IO_Translate))
         {
            QTextStream ts(&f);
            ts << tr("\"S-value (Envelope)\"\t\"Frequency\"\t\"S-value (Histogram)\":\t\"Frequency:\"\n");
            ts.setf(QTextStream::scientific);
            for (j=0; j<steps; j++)
            {
               ts << xval[j] << "\t" << yval[j] << "\t" << sval[j] << "\t" << bin_count[j] << endl;
            }
            for (j=steps; j<array; j++)
            {
               ts << xval[j] << "\t" << yval[j] << endl;
            }
            f.close();
         }
      }
   }
}

