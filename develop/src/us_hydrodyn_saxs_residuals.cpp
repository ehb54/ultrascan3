#include "../include/us_hydrodyn_saxs_residuals.h"

US_Hydrodyn_Saxs_Residuals::US_Hydrodyn_Saxs_Residuals(
                                                       bool *saxs_residuals_widget,
                                                       unsigned int width,
                                                       QString title,
                                                       vector < double > r,
                                                       vector < double > difference,
                                                       vector < double > residuals,
                                                       vector < double > target,
                                                       bool plot_residuals,
                                                       bool plot_difference,
                                                       bool plot_as_percent,
                                                       QWidget *p, 
                                                       const char *name
                                                       ) : QFrame(p, name)
{
   this->saxs_residuals_widget = saxs_residuals_widget;
   this->title = title;
   this->r = r;
   this->difference = difference;
   this->residuals = residuals;
   this->target = target;
   this->plot_residuals = plot_residuals;
   this->plot_difference = plot_difference;
   this->plot_as_percent = plot_as_percent;

   // make sure things aren't to big

   unsigned int min_len = r.size();
   if ( difference.size() <  min_len ) 
   {
      min_len = difference.size();
   }
   if ( residuals.size() <  min_len ) 
   {
      min_len = residuals.size();
   }
   if ( target.size() <  min_len ) 
   {
      min_len = target.size();
   }
   r.resize(min_len);
   difference.resize(min_len);
   residuals.resize(min_len);
   target.resize(min_len);
   difference_pct.resize(min_len);
   residuals_pct.resize(min_len);
   double area = 0e0;
   for ( unsigned int i = 0; i < target.size(); i++ )
   {
      area += target[i];
   }
   if ( target.size() )
   {
      area /= target.size();
   }
   for ( unsigned int i = 0; i < target.size(); i++ )
   {
      difference_pct[i] = 100.0 * difference[i] / area;
      residuals_pct[i] = 100.0 * residuals[i] / area;
   }

   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, width, 0);
   update_plot();
}

US_Hydrodyn_Saxs_Residuals::~US_Hydrodyn_Saxs_Residuals()
{
   if ( saxs_residuals_widget )
   {
      *saxs_residuals_widget = false;
   }
}

void US_Hydrodyn_Saxs_Residuals::setupGUI()
{

   // int minHeight1 = 30;

   // lbl_title = new QLabel(title, this);
   // lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   // lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   // lbl_title->setMinimumHeight(minHeight1);
   // lbl_title->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   // lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   plot = new QwtPlot(this);
#ifndef QT4
   plot->enableOutline(true);
   plot->setOutlinePen(Qt::white);
   plot->setOutlineStyle(Qwt::VLine);
   plot->enableGridXMin();
   plot->enableGridYMin();
#endif
   plot->setPalette( QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
#ifndef QT4
   plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#endif
   plot->setAxisTitle(QwtPlot::xBottom, tr("Distance (Angstrom)"));
   plot->setAxisTitle(QwtPlot::yLeft, tr("Frequency"));
#ifndef QT4
   plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#endif
   plot->setMargin(USglobal->config_list.margin);
   plot->setTitle(title);
   plot->setCanvasBackground(USglobal->global_colors.plot);

   cb_plot_residuals = new QCheckBox(this);
   cb_plot_residuals->setText(tr(" Plot residuals"));
   cb_plot_residuals->setEnabled(true);
   cb_plot_residuals->setChecked(plot_residuals);
   cb_plot_residuals->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   QColorGroup qcg_plot_residuals = QColorGroup(
                                                QBrush(Qt::green), // USglobal->global_colors.cg_normal.foreground(),
                                                USglobal->global_colors.cg_normal.button(), 
                                                USglobal->global_colors.cg_normal.light(), 
                                                USglobal->global_colors.cg_normal.dark(), 
                                                USglobal->global_colors.cg_normal.mid(), 
                                                USglobal->global_colors.cg_normal.text(),
                                                USglobal->global_colors.cg_normal.brightText(), 
                                                USglobal->global_colors.cg_normal.base(), 
                                                QBrush(Qt::black) // USglobal->global_colors.cg_normal.background()
                                                );
   cb_plot_residuals->setPalette( QPalette(qcg_plot_residuals, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   connect(cb_plot_residuals, SIGNAL(clicked()), SLOT(set_plot_residuals()));

   cb_plot_difference = new QCheckBox(this);
   cb_plot_difference->setText(tr(" Plot difference"));
   cb_plot_difference->setEnabled(true);
   cb_plot_difference->setChecked(plot_difference);
   cb_plot_difference->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   QColorGroup qcg_plot_difference = QColorGroup(
                                                QBrush(Qt::yellow), //USglobal->global_colors.cg_normal.foreground(),
                                                USglobal->global_colors.cg_normal.button(), 
                                                USglobal->global_colors.cg_normal.light(), 
                                                USglobal->global_colors.cg_normal.dark(), 
                                                USglobal->global_colors.cg_normal.mid(), 
                                                USglobal->global_colors.cg_normal.text(),
                                                USglobal->global_colors.cg_normal.brightText(), 
                                                USglobal->global_colors.cg_normal.base(), 
                                                QBrush(Qt::black) // USglobal->global_colors.cg_normal.background()
                                                );
   cb_plot_difference->setPalette( QPalette(qcg_plot_difference, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_plot_difference, SIGNAL(clicked()), SLOT(set_plot_difference()));

   cb_plot_as_percent = new QCheckBox(this);
   cb_plot_as_percent->setText(tr(" Plot as percent"));
   cb_plot_as_percent->setEnabled(true);
   cb_plot_as_percent->setChecked(plot_as_percent);
   cb_plot_as_percent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   QColorGroup qcg_plot_as_percent = QColorGroup(
                                                 QBrush(Qt::white), //USglobal->global_colors.cg_normal.foreground(),
                                                 USglobal->global_colors.cg_normal.button(), 
                                                 USglobal->global_colors.cg_normal.light(), 
                                                 USglobal->global_colors.cg_normal.dark(), 
                                                 USglobal->global_colors.cg_normal.mid(), 
                                                 USglobal->global_colors.cg_normal.text(),
                                                 USglobal->global_colors.cg_normal.brightText(), 
                                                 USglobal->global_colors.cg_normal.base(), 
                                                 QBrush(Qt::black) // USglobal->global_colors.cg_normal.background()
                                                );
   cb_plot_as_percent->setPalette( QPalette(qcg_plot_as_percent, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
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
   hbl->addWidget(cb_plot_residuals);
   hbl->addWidget(cb_plot_difference);
   hbl->addWidget(cb_plot_as_percent);

   background->addMultiCellLayout(hbl, j, j, 0, 1);
   // background->addWidget(cb_plot_residuals, j, 0);
   // background->addWidget(cb_plot_difference, j, 1);
   j++;

   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_cancel, j, 1);
   j++;

}

void US_Hydrodyn_Saxs_Residuals::cancel()
{
   close();
}

void US_Hydrodyn_Saxs_Residuals::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_saxs_residuals.html");
}

void US_Hydrodyn_Saxs_Residuals::closeEvent( QCloseEvent *e )
{
   *saxs_residuals_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Saxs_Residuals::set_plot_residuals()
{
   plot_residuals = cb_plot_residuals->isChecked();
   if ( !plot_residuals && !plot_difference )
   {
      cb_plot_difference->setChecked(true);
      set_plot_difference();
   } else {
      update_plot();
   }
}

void US_Hydrodyn_Saxs_Residuals::set_plot_difference()
{
   plot_difference = cb_plot_difference->isChecked();
   if ( !plot_residuals && !plot_difference )
   {
      cb_plot_residuals->setChecked(true);
      set_plot_residuals();
   } else {
      update_plot();
   }
}

void US_Hydrodyn_Saxs_Residuals::set_plot_as_percent()
{
   plot_as_percent = cb_plot_as_percent->isChecked();
   update_plot();
}

void US_Hydrodyn_Saxs_Residuals::update_plot()
{
   plot->clear();
   plot->setAxisTitle(QwtPlot::yLeft, plot_as_percent ? tr("Percent") : tr("Frequency"));
   if ( plot_residuals ) 
   {
#ifndef QT4
      long prr = plot->insertCurve("P(r) vs r");
      plot->setCurveStyle(prr, QwtCurve::Lines);
      plot->setCurveData(prr, 
                         (double *)&(r[0]), 
                         plot_as_percent ? (double *)&(residuals_pct[0]) : (double *)&(residuals[0]), 
                         (int)r.size());
      plot->setCurvePen(prr, QPen(Qt::green, 2, SolidLine));
#endif
   }
   if ( plot_difference ) 
   {
#ifndef QT4
      long prr = plot->insertCurve("P(r) vs r");
      plot->setCurveStyle(prr, QwtCurve::Lines);
      plot->setCurveData(prr, 
                         (double *)&(r[0]), 
                         plot_as_percent ? (double *)&(difference_pct[0]) : (double *)&(difference[0]),
                         (int)r.size());
      plot->setCurvePen(prr, QPen(Qt::yellow, 2, SolidLine));
#endif
   }
   plot->replot();
}
