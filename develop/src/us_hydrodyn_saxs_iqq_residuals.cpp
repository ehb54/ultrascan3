#include "../include/us_hydrodyn_saxs_iqq_residuals.h"

US_Hydrodyn_Saxs_Iqq_Residuals::US_Hydrodyn_Saxs_Iqq_Residuals(
                                                             bool *saxs_iqq_residuals_widget,
                                                             unsigned int width,
                                                             QString title,
                                                             vector < double > q,
                                                             vector < double > difference,
                                                             vector < double > difference_no_errors,
                                                             vector < double > target,
                                                             vector < double > log_difference,
                                                             vector < double > log_target,
                                                             QColor plot_color,
                                                             bool use_errors,
                                                             bool plot_log,
                                                             bool plot_difference,
                                                             bool plot_as_percent,
                                                             QWidget *p, 
                                                             const char *name
                                                             ) : QFrame(p, name)
{
   this->saxs_iqq_residuals_widget = saxs_iqq_residuals_widget;
   this->title = title;

   this->use_errors = use_errors;
   this->plot_log = plot_log;
   this->plot_difference = plot_difference;
   this->plot_as_percent = plot_as_percent;

   plot_zoomer = (ScrollZoomer *)0;

   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, width, 0);
   detached = false;

   add(
       width, 
       q,
       difference,
       difference_no_errors,
       target,
       log_difference,
       log_target,
       plot_color
       );
}

US_Hydrodyn_Saxs_Iqq_Residuals::~US_Hydrodyn_Saxs_Iqq_Residuals()
{
   if ( !detached && saxs_iqq_residuals_widget )
   {
      *saxs_iqq_residuals_widget = false;
   }
}

void US_Hydrodyn_Saxs_Iqq_Residuals::add(
                                         unsigned int width,
                                         vector < double > q,
                                         vector < double > difference,
                                         vector < double > difference_no_errors,
                                         vector < double > target,
                                         vector < double > log_difference,
                                         vector < double > log_target,
                                         QColor plot_color
                                         )
{
   unsigned int pos = qs.size();

   this->qs                        .push_back(q);
   this->differences               .push_back(difference);
   this->differences_no_errors     .push_back(difference_no_errors);
   this->targets                   .push_back(target);
   this->log_differences           .push_back(log_difference);
   this->log_targets               .push_back(log_target);
   this->plot_colors               .push_back(plot_color);

   difference_pcts                 .push_back(log_difference);
   differences_no_errors_pcts      .push_back(log_difference);
   log_difference_pcts             .push_back(log_difference);

   // make sure things aren't to big

   unsigned int min_len = qs[pos].size();
   if ( differences[pos].size() <  min_len ) 
   {
      min_len = differences[pos].size();
   }
   if ( log_differences[pos].size() <  min_len ) 
   {
      min_len = log_differences[pos].size();
   }
   if ( targets[pos].size() <  min_len ) 
   {
      min_len = targets[pos].size();
   }
   if ( log_targets[pos].size() <  min_len ) 
   {
      min_len = log_targets[pos].size();
   }

   qs                           [pos].resize(min_len);
   differences                  [pos].resize(min_len);
   differences_no_errors        [pos].resize(min_len);
   log_differences              [pos].resize(min_len);
   targets                      [pos].resize(min_len);
   log_targets                  [pos].resize(min_len);
   difference_pcts              [pos].resize(min_len);
   differences_no_errors_pcts   [pos].resize(min_len);
   log_difference_pcts          [pos].resize(min_len);

   double area = 0e0;
   double log_area = 0e0;
   for ( unsigned int i = 0; i < targets[pos].size(); i++ )
   {
      area += fabs(targets[pos][i]);
      log_area += fabs(log_targets[pos][i]);
   }
   if ( targets[pos].size() )
   {
      area /= targets[pos].size();
   }
   if ( log_targets[pos].size() )
   {
      log_area /= log_targets[pos].size();
   }

   for ( unsigned int i = 0; i < targets[pos].size(); i++ )
   {
      difference_pcts[pos][i] = 100.0 * differences[pos][i] / area;
   }

   for ( unsigned int i = 0; i < targets[pos].size(); i++ )
   {
      if ( targets[pos][i] != 0e0 )
      {
         differences_no_errors_pcts[pos][i] = 100.0 * differences_no_errors[pos][i] / targets[pos][i];
      } else {
         differences_no_errors_pcts[pos][i] = 0e0;
      }
   }

   for ( unsigned int i = 0; i < log_targets[pos].size(); i++ )
   {
      log_difference_pcts[pos][i] = 100.0 * log_differences[pos][i] / area;
   }

   QRect qr = geometry();
   qr.setWidth( width );
   setGeometry( qr );
   // setGeometry(0, 0, width, 0);
   update_plot();
}

void US_Hydrodyn_Saxs_Iqq_Residuals::setupGUI()
{

   plot = new QwtPlot(this);
#ifndef QT4
   // plot->enableOutline(true);
   // plot->setOutlinePen(Qt::white);
   // plot->setOutlineStyle(Qwt::VLine);
   plot->enableGridXMin();
   plot->enableGridYMin();
#else
   grid = new QwtPlotGrid;
   grid->enableXMin( true );
   grid->enableYMin( true );
#endif
   plot->setPalette( QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
#ifndef QT4
   plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   grid->setMajPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid->setMinPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid->attach( plot );
#endif
   plot->setAxisTitle(QwtPlot::xBottom, tr("q (1/Angstrom)"));
   plot->setAxisTitle(QwtPlot::yLeft, tr(""));
#ifndef QT4
   plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#ifndef QT4
   plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#ifndef QT4
   plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot->setMargin(USglobal->config_list.margin);
   plot->setTitle(title);
   plot->setCanvasBackground(USglobal->global_colors.plot);

   cb_plot_difference = new QCheckBox(this);
   cb_plot_difference->setText(tr(" Plot difference"));
   cb_plot_difference->setEnabled(true);
   cb_plot_difference->setChecked(plot_difference);
   cb_plot_difference->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_plot_difference->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   connect(cb_plot_difference, SIGNAL(clicked()), SLOT(set_plot_difference()));

   cb_plot_log = new QCheckBox(this);
   cb_plot_log->setText(tr(" Plot log"));
   cb_plot_log->setEnabled(true);
   cb_plot_log->setChecked(plot_log);
   cb_plot_log->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_plot_log->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   connect(cb_plot_log, SIGNAL(clicked()), SLOT(set_plot_log()));

   cb_plot_as_percent = new QCheckBox(this);
   cb_plot_as_percent->setText(tr(" Plot as percent"));
   cb_plot_as_percent->setEnabled(true);
   cb_plot_as_percent->setChecked(plot_as_percent);
   cb_plot_as_percent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_plot_as_percent->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_plot_as_percent, SIGNAL(clicked()), SLOT(set_plot_as_percent()));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   // pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   // pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));


   // build layout
   // left box / possible

   int rows = 0, columns = 2, spacing = 2, j=0, margin=4;
   QGridLayout *background = new QGridLayout(this, rows, columns, margin, spacing);

   // background->addMultiCellWidget(lbl_title, j, j, 0, 1);
   // j++;

   background->addMultiCellWidget(plot, j, j, 0, 1);
   j++;

   QBoxLayout *hbl = new QHBoxLayout(0);
   hbl->addWidget(cb_plot_difference);
   hbl->addWidget(cb_plot_log);
   hbl->addWidget(cb_plot_as_percent);

   background->addMultiCellLayout(hbl, j, j, 0, 1);
   j++;

   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_cancel, j, 1);
   j++;

}

void US_Hydrodyn_Saxs_Iqq_Residuals::cancel()
{
   close();
}

void US_Hydrodyn_Saxs_Iqq_Residuals::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_saxs_iqq_residuals.html");
}

void US_Hydrodyn_Saxs_Iqq_Residuals::closeEvent( QCloseEvent *e )
{
   if ( !detached )
   {
      *saxs_iqq_residuals_widget = false;
   }
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Saxs_Iqq_Residuals::set_plot_log()
{
   plot_log = cb_plot_log->isChecked();
   cb_plot_difference->setChecked( !plot_log );
   plot_difference = !plot_log;
   update_plot();
}

void US_Hydrodyn_Saxs_Iqq_Residuals::set_plot_difference()
{
   plot_difference = cb_plot_difference->isChecked();
   cb_plot_log->setChecked( !plot_difference );
   plot_log = !plot_difference;
   update_plot();
}

void US_Hydrodyn_Saxs_Iqq_Residuals::set_plot_as_percent()
{
   plot_as_percent = cb_plot_as_percent->isChecked();
   update_plot();
}

void US_Hydrodyn_Saxs_Iqq_Residuals::update_plot()
{
   plot->clear();
   plot->setAxisTitle(QwtPlot::yLeft, plot_as_percent ? tr("Percent") : tr("I(q)"));
   double minx;
   double maxx;
   double miny;
   double maxy;
   bool any_set = false;

   for ( unsigned int pos = 0; pos < qs.size(); pos++ )
   {
      if ( pos )
      {
         if ( minx > qs[ pos ][ 0 ] )
         {
            minx = qs[ pos ][ 0 ];
         }
         if ( maxx < qs[ pos ][ qs[ pos ].size() - 1 ] )
         {
            maxx = qs[ pos ][ qs[ pos ].size() - 1 ];
         }
      } else {
         minx = qs[ pos ][ 0 ];
         maxx = qs[ pos ][ qs[ pos ].size() - 1 ];
      }
         
      if ( plot_log ) 
      {
#ifndef QT4
         long iqq = plot->insertCurve("Log10 I(q) vs q"); 
         plot->setCurveStyle(iqq, QwtCurve::Lines);
         plot->setCurveData(iqq, 
                            (double *)&(qs[pos][0]), 
                            plot_as_percent ? (double *)&(log_difference_pcts[pos][0]) : (double *)&(log_differences[pos][0]), 
                            (int)qs[pos].size());
         plot->setCurvePen(iqq, QPen(plot_colors[pos], 2, SolidLine));
#else
         QwtPlotCurve *curve = new QwtPlotCurve( "Log10 I(q) vs q" );
         curve->setStyle( QwtPlotCurve::Lines );
         curve->setData(
                        (double *)&(qs[pos][0]), 
                        plot_as_percent ? (double *)&(log_difference_pcts[pos][0]) : (double *)&(log_differences[pos][0]), 
                        (int)qs[pos].size()
                        );
         curve->setPen( QPen(plot_colors[pos], 2, SolidLine) );
         curve->attach( plot );
#endif
         double this_miny = plot_as_percent ? log_difference_pcts[ pos ][ 0 ] : log_differences[ pos ][ 0 ];
         double this_maxy = this_miny;
         for ( unsigned int i = 1; i < qs[ pos ].size(); i++ )
         {
            double val = plot_as_percent ? log_difference_pcts[ pos ][ i ] : log_differences[ pos ][ i ];
            if ( this_miny > val )
            {
               this_miny = val;
            }
            if ( this_maxy < val )
            {
               this_maxy = val;
            }
         }
         if ( any_set )
         {
            if ( miny > this_miny )
            {
               miny = this_miny;
            } 
            if ( maxy < this_maxy )
            {
               maxy = this_maxy;
            }
         } else {
            miny = this_miny;
            maxy = this_maxy;
            any_set = true;
         }
      }
      if ( plot_difference ) 
      {
#ifndef QT4
         long iqq = plot->insertCurve("Log10 I(q) vs q"); 
         plot->setCurveStyle(iqq, QwtCurve::Lines);
         plot->setCurveData(iqq, 
                            (double *)&(qs[pos][0]), 
                            plot_as_percent ? (double *)&(differences_no_errors_pcts[pos][0]) : (double *)&(differences[pos][0]),
                            (int)qs[pos].size());
         plot->setCurvePen(iqq, QPen(plot_colors[pos], 2, SolidLine));
#else
         QwtPlotCurve *curve = new QwtPlotCurve( "Log10 I(q) vs q" );
         curve->setStyle( QwtPlotCurve::Lines );
         curve->setData(
                        (double *)&(qs[pos][0]), 
                        plot_as_percent ? (double *)&(differences_no_errors_pcts[pos][0]) : (double *)&(differences[pos][0]),
                        (int)qs[pos].size()
                        );
         curve->setPen( QPen(plot_colors[pos], 2, SolidLine) );
         curve->attach( plot );
#endif
         double this_miny = plot_as_percent ? differences_no_errors_pcts[ pos ][ 0 ] : differences[ pos ][ 0 ];
         double this_maxy = this_miny;
         for ( unsigned int i = 1; i < qs[ pos ].size(); i++ )
         {
            double val = plot_as_percent ? differences_no_errors_pcts[ pos ][ i ] : differences[ pos ][ i ];
            if ( this_miny > val )
            {
               this_miny = val;
            }
            if ( this_maxy < val )
            {
               this_maxy = val;
            }
         }
         if ( any_set )
         {
            if ( miny > this_miny )
            {
               miny = this_miny;
            } 
            if ( maxy < this_maxy )
            {
               maxy = this_maxy;
            }
         } else {
            miny = this_miny;
            maxy = this_maxy;
            any_set = true;
         }
      }
   }

   // display 2sd bars
   if ( qs.size() && plot_difference && use_errors && !plot_as_percent )
   {
      double x[2];
      double y[2];
      x[0] = qs[0][0];
      x[1] = qs[0][qs[0].size() - 1];
      y[0] = 2;
      y[1] = 2;
#ifndef QT4
      long iqq = plot->insertCurve("+2 sd"); 
      plot->setCurveStyle(iqq, QwtCurve::Lines);
      plot->setCurveData(iqq, 
                         (double *)&(x[0]), 
                         (double *)&(y[0]), 
                         2);
      plot->setCurvePen(iqq, QPen(Qt::white, 2, SolidLine));
#else
      QwtPlotCurve *curve = new QwtPlotCurve( "+2 sd" );
      curve->setStyle( QwtPlotCurve::Lines );
      curve->setData(
                     (double *)&(x[0]), 
                     (double *)&(y[0]), 
                     2
                     );
      curve->setPen( QPen(Qt::white, 2, SolidLine) );
      curve->attach( plot );
#endif
      y[0] = -2;
      y[1] = -2;
#ifndef QT4
      iqq = plot->insertCurve("-2 sd"); 
      plot->setCurveStyle(iqq, QwtCurve::Lines);
      plot->setCurveData(iqq, 
                         (double *)&(x[0]), 
                         (double *)&(y[0]), 
                         2);
      plot->setCurvePen(iqq, QPen(Qt::white, 2, SolidLine));
#else
      QwtPlotCurve *curve = new QwtPlotCurve( "-2 sd" );
      curve->setStyle( QwtPlotCurve::Lines );
      curve->setData(
                     (double *)&(x[0]), 
                     (double *)&(y[0]), 
                     2
                     );
      curve->setPen( QPen(Qt::white, 2, SolidLine) );
      curve->attach( plot );
#endif
      if ( miny > -2.2 )
      {
         miny = -2.2;
      }
      if ( maxy < 2.2 )
      {
         maxy = 2.2;
      }
      if ( miny > - maxy )
      {
         miny = - maxy;
      }
      if ( maxy < - miny )
      {
         maxy = - miny;
      }
   }

   // enable zooming
   if ( plot_zoomer )
   {
      delete plot_zoomer;
   }
   
   plot->setAxisScale( QwtPlot::xBottom, minx, maxx );
   plot->setAxisScale( QwtPlot::yLeft,   miny, maxy );
   
   plot_zoomer = new ScrollZoomer(plot->canvas());
   plot_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
   plot_zoomer->setCursorLabelPen(QPen(Qt::yellow));

   plot->replot();
}
