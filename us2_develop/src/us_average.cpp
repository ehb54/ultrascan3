#include "../include/us_average.h"

US_Average_Legend_F::US_Average_Legend_F(QWidget *p, const char* name)
   : QFrame(p, name)
{
   USglobal = new US_Config();

   int ButtonHeight=26, buttonWidth=90, spacing=2;
   int xpos=7;
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   expand = new QPushButton(tr("Zoom out"), this);
   Q_CHECK_PTR(expand);
   expand->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   expand->setGeometry(xpos, 7, buttonWidth, ButtonHeight);
   expand->setAutoDefault(false);
   expand->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   
   xpos += buttonWidth + spacing;

   compress = new QPushButton(tr("Zoom in"), this);
   Q_CHECK_PTR(compress);
   compress->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   compress->setGeometry(xpos, 7, buttonWidth, ButtonHeight);
   compress->setAutoDefault(false);
   compress->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(compress, SIGNAL(clicked()), SIGNAL(clicked()));

   xpos += buttonWidth + spacing;

   pb_print = new QPushButton(tr("Print"), this);
   Q_CHECK_PTR(pb_print);
   pb_print->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_print->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_print->setAutoDefault(false);
   pb_print->setGeometry(xpos, 7, buttonWidth, ButtonHeight);

   xpos += buttonWidth + spacing;

   save = new QPushButton(tr("Save"), this);
   Q_CHECK_PTR(save);
   save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   save->setAutoDefault(false);
   save->setGeometry(xpos, 7, buttonWidth, ButtonHeight);

   xpos += buttonWidth + spacing;

   close = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(close);
   close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   close->setAutoDefault(false);
   close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   close->setGeometry(xpos, 7, buttonWidth, ButtonHeight);
}

US_Average_Legend_F::~US_Average_Legend_F()
{
   delete expand;
   delete compress;
   delete close;
}

US_Average::US_Average(int pts, int sc, double ***xval, double ***yval, const QString temp_extension, 
                       const QString temp_htmlDir, const QString title,
                       const int temp_cell, const int temp_lambda, QWidget *p , const char *name) : QWidget(p, name)
{
   USglobal = new US_Config();
   pm = new US_Pixmap();
   points = pts;
   total_points = points;
   scans = sc;
   x = xval;
   y = yval;
   extension = temp_extension;
   htmlDir = temp_htmlDir;
   cell = temp_cell;
   lambda = temp_lambda;

   avg_plot = new QwtPlot(this);
   Q_CHECK_PTR(avg_plot);
   avg_plot->setAxisTitle(QwtPlot::xBottom, tr("Sedimentation Coefficient x 10^(13) sec"));
   avg_plot->setAxisTitle(QwtPlot::yLeft, tr("Frequency"));
   avg_plot->enableGridXMin();
   avg_plot->enableGridYMin();
   avg_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
   avg_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   avg_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
   //avg_plot->setPlotBackground(USglobal->global_colors.plot);     //old version
   avg_plot->setCanvasBackground(USglobal->global_colors.plot);      //new version
   avg_plot->enableOutline(true);
   avg_plot->setOutlinePen(white);
   avg_plot->setTitle(title);
   avg_plot->setOutlineStyle(Qwt::Cross);
   avg_plot->setMargin(USglobal->config_list.margin);
   
   avg_legend = new US_Average_Legend_F(this);
   Q_CHECK_PTR(avg_legend);
   avg_legend->setFrameStyle(QFrame::Panel|QFrame::Raised);
   avg_legend->setLineWidth(2);

   connect(avg_legend->expand, SIGNAL(clicked()), this, SLOT(expand()));
   connect(avg_legend->compress, SIGNAL(clicked()), this, SLOT(compress()));
   connect(avg_legend->pb_print, SIGNAL(clicked()), this, SLOT(print_plot()));
   connect(avg_legend->save, SIGNAL(clicked()), this, SLOT(save_avg()));
   connect(avg_legend->close, SIGNAL(clicked()), this, SLOT(close_dis()));

   calc();
   avg_plot->clear();
   avg_curve = avg_plot->insertCurve("Derivative");
   avg_plot->setCurvePen(avg_curve, yellow);
   avg_plot->setCurveStyle(avg_curve, QwtCurve::Lines);
   avg_plot->setCurveData(avg_curve, sval, sum, points);

   //   m_min = qwtGetMin(sval, points);
   m_max = qwtGetMax(sval, points);
   //   diff = (m_max-m_min)/2;
   //   m_max += diff;
   //   m_min -= diff;
   avg_plot->setAxisScale(QwtPlot::xBottom, 0, m_max);
   avg_plot->replot();
   diff = m_max/10;
   
   setMinimumSize(560,480);
   global_Xpos += 30;
   global_Ypos += 30;
   
   move(global_Xpos, global_Ypos);
}

US_Average::~US_Average()
{
   delete avg_plot;
   delete avg_legend;
   delete sum;
   delete sval;
}

void US_Average::closeEvent(QCloseEvent *e)
{
   e->accept();
   update_pixmap();
   global_Xpos -= 30;
   global_Ypos -= 30;
}

void US_Average::update_pixmap()
{
   qApp->processEvents();
   QPixmap p;
   QString fileName;
   fileName.sprintf(htmlDir + "/" + extension + "_avg_%d%d.", cell + 1, lambda +1);
   p = QPixmap::grabWidget(avg_plot, 2, 2, avg_plot->width() - 4, avg_plot->height() - 4);
   pm->save_file(fileName, p);
}

void US_Average::calc()
{
   int i, j, k, total_scans;
   int *s_stop;   // stop index for each s-value array
   end_flag = new bool [scans];
   s_stop = new int [scans];
   sum = new double [points];
   sval = new double [points];
   double max_s=0, min_s=9.9e10, a, b;

   //
   // initialize s_stop to be the second point:
   //
   for (i=0; i<scans; i++)
   {
      s_stop[i] = 1;
      end_flag[i]=FALSE;
   }
   //
   // find the maximum X value (s_value), and initialize the stops:
   //
   for (i=0; i<points; i++)
   {
      max_s = max(max_s, (*y)[0][i]);
      min_s = min(min_s, (*y)[0][i]);
   }
   //
   // use `a` as a temporary variable to hold the range of possible y-values. If the
   // value of the y-value is larger than 1% of the total range, then we want to
   // pick the x-value (s-value) at that point and consider it the maximum s-value.
   // This procedure will only be necessary for the first scan, since the 
   //
   a = (max_s - min_s)/100;       // cutoff criterion is 1 % of total

   i = points-1;
   while (a > (*y)[0][i])
   {
      i--;
   }
   max_s = (*x)[0][i];
   
   //
   // Assign new equally spaced s-values for the X-Axis:
   //
   for (i=0; i<points; i++)
   {
      sval[i] = i * max_s/(points - 1);
   }
   //
   // For each new s value, find the corresponding frequency through linear 
   // interpolation and add up, then average by the number of total scans included:
   //
   for (j=0; j<points; j++)
   {
      total_scans = scans;
      sum[j] = 0.0;
      for (i=0; i<scans; i++)
      {
         //
         // starting at the last s_stop, move up the x-scale to find the next s value 
         // that matches the current sval, reassign s_stop and continue:
         //
         k = s_stop[i];
         while ((!end_flag[i]) && ((*x)[i][k] < sval[j]))
         {
            k++;
            if (k == (total_points - 1))
            {
               end_flag[i]=TRUE;
               total_scans--;
               s_stop[i] = k-1;
            }
         }
         s_stop[i] = k;
         // 
         // using this s value and the adjacent value below it, do a linear interpolation 
         // for the matching s-value to find the appropriate frequency. Add this to the 
         // current frequency and continue to the next scan:
         //
         if (!end_flag[i])
         {
            a = ((*y)[i][k] - (*y)[i][k-1])/((*x)[i][k] - (*x)[i][k-1]);
            b = (*y)[i][k] - a * (*x)[i][k];
            sum[j] += a * sval[j] + b; 
         }
      }

      if (total_scans > 0)
      {
         sum[j]=sum[j]/total_scans;
      }
      else
      {
         sum[j]=0.0;
      }
      emit status(j);
   }
}

void US_Average::resizeEvent(QResizeEvent *e)
{
   QRect r(0,0, e->size().width(), (int) (e->size().height()-44));
   avg_plot->setGeometry(r);
   avg_legend->setGeometry(0, e->size().height()-42, r.width(), 40);
}

void US_Average::expand()
{
   //   m_min -= diff;
   m_max += diff;
   diff = m_max/10;
   avg_plot->setAxisScale(QwtPlot::xBottom, 0, m_max);
   avg_plot->replot();
}

void US_Average::compress()
{
   //   m_min += diff;
   m_max -= diff;
   diff = m_max/10;
   avg_plot->setAxisScale(QwtPlot::xBottom, 0, m_max);
   avg_plot->replot();
}

void US_Average::save_avg()
{
   int k;
   QString temp = extension.copy();
   temp.prepend("*.");
   QString fn = QFileDialog::getSaveFileName( 0, temp, 0);
   if ( !fn.isEmpty() ) 
   {
      k = fn.find(".", 0, FALSE);
      if (k != -1) //if an extension was given, strip it
      {
         fn.truncate(k);
      }
      save_avg( fn );      // the user gave a file name
   }
}

void US_Average::print_plot()
{
   QPrinter printer;
   
   if   (printer.setup(0))
   {
      //avg_plot->print(printer, QwtFltrInv()); //colors inverted
      avg_plot->print(printer, QwtPlotPrintFilter());      //new version
   }
}

void US_Average::save_avg(const QString &filename)
{
   QString filestr = QString(filename);
   filestr.append(".");
   filestr.append(extension);
   QFile f(filestr);
   int i, j;   
   if (f.exists())
   {
      if(!QMessageBox::query( tr("Warning"),tr( "Attention:\nThis file exists already!\n\nDo you want to overwrite it?"), tr("Yes"), tr("No")))
      {
         f.close();
         return;
      }
   }
   if (f.open(IO_WriteOnly | IO_Translate))
   {
      QTextStream ts(&f);
      for (i=0; i<points; i++)
      {
         ts << sval[i] << "\t" << sum[i] << "\t";
         for (j=0; j<scans-1; j++)
         {
            ts << (*x)[j][i] << "\t" << (*y)[j][i] << "\t";
         }
         ts << (*x)[scans-1][i] << "\t" << (*y)[scans-1][i]  << "\n";
      }
      f.close();
   }
}

void US_Average::close_dis()
{
   close();
}



