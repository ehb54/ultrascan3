#include "../include/us_meniscus.h"

#ifndef WIN32
#include <unistd.h>
#endif

US_Meniscus::US_Meniscus(QWidget *p, const char *name) : QFrame(p, name)
{
   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));

   order = 3;
   
   lbl_minimum = new QLabel(tr(" Meniscus Minimum: "), this);
   Q_CHECK_PTR(lbl_minimum);
   lbl_minimum->setAlignment(AlignLeft|AlignVCenter);
   lbl_minimum->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_minimum->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_rmsd = new QLabel(tr(" Fit RMSD: "), this);
   Q_CHECK_PTR(lbl_rmsd);
   lbl_rmsd->setAlignment(AlignLeft|AlignVCenter);
   lbl_rmsd->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_rmsd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_order = new QLabel(tr(" Fit order: "), this);
   Q_CHECK_PTR(lbl_order);
   lbl_order->setAlignment(AlignLeft|AlignVCenter);
   lbl_order->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_order->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   ct_order= new QwtCounter(this);
   Q_CHECK_PTR(ct_order);
   ct_order->setRange(2, 10, 1);
   ct_order->setValue(2);
   ct_order->setNumButtons(1);
   ct_order->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(ct_order, SIGNAL(valueChanged(double)), SLOT(update_order(double)));

   editor = new QTextEdit(this, 0);
   editor->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   editor->setTextFormat( Qt::PlainText );
   
   plot_meniscus = new QwtPlot(this);
   Q_CHECK_PTR(plot_meniscus);
   plot_meniscus->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
/*
   plot_meniscus->enableGridXMin();
   plot_meniscus->enableGridYMin();
   plot_meniscus->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot_meniscus->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
   plot_meniscus->enableOutline(true);
   plot_meniscus->setOutlinePen(white);
   plot_meniscus->setOutlineStyle(Qwt::Cross);
*/
   plot_meniscus->setCanvasBackground(USglobal->global_colors.plot);      //new version
   plot_meniscus->setAxisTitle(QwtPlot::xBottom, tr("Radius"));
   plot_meniscus->setAxisTitle(QwtPlot::yLeft, tr("RMSD"));
   plot_meniscus->setTitle(tr("Meniscus Fit"));
   plot_meniscus->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   plot_meniscus->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   plot_meniscus->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_meniscus->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   plot_meniscus->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_meniscus->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   plot_meniscus->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_meniscus->setMargin(USglobal->config_list.margin);
   plot_meniscus->setMinimumSize(550, 350);

   pb_plot = new QPushButton(tr("Plot"), this);
   Q_CHECK_PTR(pb_plot);
   pb_plot->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_plot->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_plot->setAutoDefault(false);
   connect(pb_plot, SIGNAL(clicked()), SLOT(update_plot()));

   pb_reset = new QPushButton(tr("Reset"), this);
   Q_CHECK_PTR(pb_reset);
   pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_reset->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_reset->setAutoDefault(false);
   pb_reset->setEnabled(false);
   connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_help->setAutoDefault(false);
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_close = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_close);
   pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_close->setAutoDefault(false);
   connect(pb_close, SIGNAL(clicked()), SLOT(close()));
   
   le_minimum = new QLineEdit(this);
   le_minimum->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_minimum->setText("");
   le_minimum->setReadOnly(true);
   le_minimum->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   
   le_rmsd = new QLineEdit(this);
   le_rmsd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_rmsd->setText("");
   le_rmsd->setReadOnly(true);
   le_rmsd->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   
   setupGUI();
}

US_Meniscus::~US_Meniscus()
{
}

void US_Meniscus::setupGUI()
{
   int j=0;
   int rows = 6, columns = 3, spacing = 2;
   
   QGridLayout * Grid1 = new QGridLayout(this, rows, columns, spacing);
   for (int i=0; i<rows; i++)
   {
      Grid1->setRowSpacing(i, 26);
   }
   
   Grid1->addMultiCellWidget(editor, j, j, 0, 1);
   Grid1->addMultiCellWidget(plot_meniscus, j, j+5, 2, 2);
   j++;
   Grid1->addWidget(lbl_order, j, 0);
   Grid1->addWidget(ct_order, j, 1);
   j++;
   Grid1->addWidget(lbl_minimum, j, 0);
   Grid1->addWidget(le_minimum, j, 1);
   j++;
   Grid1->addWidget(lbl_rmsd, j, 0);
   Grid1->addWidget(le_rmsd, j, 1);
   j++;
   Grid1->addWidget(pb_plot, j, 0);
   Grid1->addWidget(pb_reset, j, 1);
   j++;
   Grid1->addWidget(pb_help, j, 0);
   Grid1->addWidget(pb_close, j, 1);
   j++;

   qApp->processEvents();
   
   global_Xpos += 30;
   global_Ypos += 30;
   
   this->setGeometry(global_Xpos, global_Ypos, this->width(), this->height());
}

void US_Meniscus::update_order(double val)
{
   order = (unsigned int) val+1;
   update_plot();
}

void US_Meniscus::update_plot()
{
   QString contents = editor->text(), str;
   QStringList lines = QStringList::split("\n", contents, false);
   QStringList parsed;
   unsigned int i, j, count=0;
   double minimum, fitymin;
   
   double* radius_values = new double[ lines.size() ];
   double* rmsd_values   = new double[ lines.size() ];
   
   double  minx = 1e20;
   double  maxx = 0.0;

   double  miny = 1e20;
   double  maxy = 0.0;

   // Remove any non-data lines and put values in arrays
   for ( i = 0; i < lines.size(); i++ )
   {
      QStringList values = QStringList::split(" ", lines[i], false);
      if ( values.size() > 1 )
      {
         radius_values[ count ] = values[ 0 ].toDouble();
         rmsd_values  [ count ] = values[ 1 ].toDouble();

         // Find min and max
         minx = min( minx, radius_values[ count ] );
         maxx = max( maxx, radius_values[ count ] );

         miny = min( miny, rmsd_values[ count ] );
         maxy = max( maxy, rmsd_values[ count ] );

         count++;
      }
   }
   if ( count < 3 ) return;

   //editor->setTextFormat( parsed.join( "\n" ) );

   double overscan = ( maxx - minx ) * 0.10;  // 10% overscan

   plot_meniscus->setAxisScale( QwtPlot::xBottom,
         minx - overscan, maxx + overscan );
    
   // Adjust y axis to scale all the data
   double dy = fabs( maxy - miny ) / 10.0;

   plot_meniscus->setAxisScale( QwtPlot::yLeft, miny - dy, maxy + dy );

   plot_meniscus->clear();
   unsigned int raw_curve = plot_meniscus->insertCurve(tr("Raw Data"));
   plot_meniscus->setCurveStyle(raw_curve, QwtCurve::Lines);
   plot_meniscus->setCurvePen(raw_curve, yellow);
   plot_meniscus->setCurveData(raw_curve, radius_values, rmsd_values, count);
   plot_meniscus->replot();

   double *c;
   double *fity, *fitx;
   fit_points = 1000;
   US_lsfit *polyfit;
   c = new double [order]; // coefficients
   fitx = new double [fit_points];
   fity = new double [fit_points];
   double rmsd = 0.0;

   polyfit = new US_lsfit(c, radius_values, rmsd_values, order, count, true);

   for (i=0; i<fit_points; i++)
   {
      fitx[i] = radius_values[0]-0.001 + i * ((radius_values[count-1] - radius_values[0] + 0.002)/99);
      fity[i] = 0.0;
      for (j=0; j<order; j++)
      {
         fity[i] +=  c[j] * pow((double) fitx[i], (double) j);
      }
   }

   unsigned int fit_curve = plot_meniscus->insertCurve(tr("Raw Data"));
   plot_meniscus->setCurveStyle(fit_curve, QwtCurve::Lines);
   plot_meniscus->setCurvePen(fit_curve, cyan);
   plot_meniscus->setCurveData(fit_curve, fitx, fity, fit_points);
   plot_meniscus->replot();
   unsigned int index, last_index=0;
   for (i=0; i<count; i++)
   {
      index = find_index(radius_values[i], last_index, fit_points, fitx);
      last_index = index;
      rmsd += pow(rmsd_values[i] - fity[index], 2.0);
   }
   rmsd /= count;
   str.sprintf("%8.6e", rmsd);
   le_rmsd->setText(str);

   // Find the minimum
   
   i = 0;
   while (i<fit_points-1 && (fity[i+1] - fity[i]) < 0.0)
   {
      i++;
   }
   if (i == fit_points-1)
   {
      le_minimum->setText(tr("No minimum found!"));
      return;
   }
   minimum = fitx[i];
   fitymin = fity[i];
   str.sprintf("%8.6f cm", minimum);
   le_minimum->setText(str);
   
   // Plot the minimum

   double radius_min[ 2 ];
   double rmsd_min  [ 2 ];

   radius_min[ 0 ] = minimum;
   radius_min[ 1 ] = minimum;

   rmsd_min  [ 0 ] = fitymin - 1.0 * dy;
   rmsd_min  [ 0 ] = fitymin + 2.0 * dy;

   unsigned int minimum_curve = plot_meniscus->insertCurve(tr("Minimum Pointer"));
   plot_meniscus->setCurveStyle(minimum_curve, QwtCurve::Lines);
   plot_meniscus->setCurvePen(minimum_curve, QPen(Qt::cyan, 3, SolidLine));
   plot_meniscus->setCurveData(minimum_curve, radius_min, rmsd_min, 2);

   str.sprintf(tr("Meniscus at:\n%8.6f cm"), minimum);
   unsigned int meniscus_label = plot_meniscus->insertMarker();
   plot_meniscus->setMarkerLabel(meniscus_label, str);
   plot_meniscus->setMarkerPos(meniscus_label, minimum, fitymin + 4 * dy);
   plot_meniscus->setMarkerFont(meniscus_label, QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   plot_meniscus->setMarkerPen(meniscus_label, QPen(cyan, 0, DashDotLine));
   
   plot_meniscus->replot();

   pb_reset->setEnabled(true);
   plot_meniscus->replot();
}

void US_Meniscus::reset()
{
   plot_meniscus->clear();
   plot_meniscus->replot();
   editor->clear();
   ct_order->setValue( 2 );
   le_minimum->setText( "" );
   le_rmsd ->setText( "" );
   pb_reset->setEnabled(false);
}

int US_Meniscus::find_index(double testx, int start, int maxcount, double *xarray)
{
   int i=start;
   maxcount --;
   while ((xarray[i] < testx) && (i<=maxcount))
   {
      i++;
      if (i > maxcount)
      {
         return(-1);
      }
//      cout << "array: " << xarray[i] << ", test: " << testx << ", index: " << i << endl;
   }
   if (fabs(xarray[i]-testx) > fabs(xarray[i-1]-testx))
   {
      return (i-1);
   }
   else
   {
      return (i);
   }
}

void US_Meniscus::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/fit_meniscus.html");
}

