#include "../include/us_fematch.h"
#ifndef WIN32
#include <unistd.h>
#endif
#include <math.h>
#include <float.h>

US_FeMatch_W::US_FeMatch_W(QWidget *p, const char *name) : Data_Control_W(1, p, name)
{
   setCaption(tr("Finite Element Match Analysis:"));
   pm = new US_Pixmap();
   window_3d_flag = false;
   components = 0;
   cofs_GUI();
   plot2 = 1;
   connect(this, SIGNAL(dataLoaded()), this, SLOT(enableButtons()));
   connect(this, SIGNAL(vbarChanged(float, float)), this, SLOT(updateParameters()));
   connect(this, SIGNAL(datasetChanged()), this, SLOT(clearDisplay()));
   us_3d_solutes = NULL;
   sa2d_ctrl_vars.fit_posBaseline = false;
   sa2d_ctrl_vars.fit_negBaseline = false;
   sa2d_ctrl_vars.ff0=1.5;
   sa2d_ctrl_vars.min_s = 1.0;
   sa2d_ctrl_vars.max_s = 10.0;
   sa2d_ctrl_vars.resolution_s = 30;
   sa2d_ctrl_vars.min_ff0 = 1.0;
   sa2d_ctrl_vars.max_ff0 = 4.0;
   sa2d_ctrl_vars.resolution_ff0 = 30;
   analysis_type = "";
   current_component = 0;
   extraCounter = -1;
   resplot = NULL;
}

US_FeMatch_W::~US_FeMatch_W()
{
   delete resplot;
}

void US_FeMatch_W::clearDisplay()
{
   if (resplot != NULL)
   {
      resplot->close();
      resplot = NULL;
   }
   if (us_3d_solutes != NULL)
   {
      us_3d_solutes->close();
   }
   analysis_plot->clear();
   analysis_plot->replot();
}

void US_FeMatch_W::cofs_GUI()
{
   if (resplot == NULL)
   {
      resplot = new US_ResidualPlot(0,0);
   }

   int spacing = 2;
   pb_second_plot->setText(tr("s20,W distribution"));
   delete pb_reset;
   delete smoothing_lbl;
   delete smoothing_counter;
   smoothing_counter = NULL;
   delete range_lbl;
   delete range_counter;
   range_counter = NULL;
   delete position_lbl;
   delete position_counter;
   position_counter = NULL;

   current_component = 1;
   baseline = 0.0;
   lbl1_excluded->setText(tr("RMSD:"));

   xpos = border;
   ypos = 409;

   pb_loadModel = new QPushButton(tr("Load Model"), this);
   Q_CHECK_PTR(pb_loadModel);
   pb_loadModel->setAutoDefault(false);
   pb_loadModel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_loadModel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_loadModel->setGeometry(xpos, ypos, buttonw, buttonh);
   pb_loadModel->setEnabled(false);
   connect(pb_loadModel, SIGNAL(clicked()), SLOT(load_model()));

   xpos += buttonw + spacing;

   pb_fit = new QPushButton(tr("Simulate Model"), this);
   Q_CHECK_PTR(pb_fit);
   pb_fit->setAutoDefault(false);
   pb_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_fit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_fit->setGeometry(xpos, ypos, buttonw, buttonh);
   pb_fit->setEnabled(false);
   connect(pb_fit, SIGNAL(clicked()), SLOT(fit()));

   xpos = border;
   ypos += buttonh + spacing;

   lbl_s = new QLabel(tr(" Sedimentation Coeff.:"),this);
   lbl_s->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_s->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos += buttonw + spacing;

   lbl_s2 = new QLabel(this);
   lbl_s2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_s2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_s2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_s2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lbl_s2->setText("");

   xpos = border;
   ypos += buttonh + spacing;

   lbl_D = new QLabel(tr(" Diffusion Coeff.:"),this);
   lbl_D->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_D->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_D->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos += buttonw + spacing;

   lbl_D2 = new QLabel(this);
   lbl_D2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_D2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_D2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_D2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lbl_D2->setText("");

   xpos = border;
   ypos += buttonh + spacing;

   lbl_C = new QLabel(tr(" Partial Concentration:"),this);
   lbl_C->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_C->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_C->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos += buttonw + spacing;

   lbl_C2 = new QLabel(this);
   lbl_C2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_C2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_C2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_C2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lbl_C2->setText("");

   xpos = border;
   ypos += buttonh + spacing;

   lbl_variance = new QLabel(tr(" Variance:"),this);
   lbl_variance->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_variance->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_variance->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos += buttonw + spacing;

   lbl_variance2 = new QLabel(this);
   lbl_variance2->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_variance2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_variance2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_variance2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lbl_variance2->setText("0.0");

   xpos = border;
   ypos += buttonh + spacing;

   lbl_MW_ff01 = new QLabel(tr(" Mol. Weight, f/f0:"),this);
   lbl_MW_ff01->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_MW_ff01->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_MW_ff01->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos += buttonw + spacing;

   lbl_MW_ff02 = new QLabel(tr(""),this);
   lbl_MW_ff02->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_MW_ff02->setAlignment(AlignCenter|AlignVCenter);
   lbl_MW_ff02->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lbl_MW_ff02->setGeometry(xpos, ypos, buttonw, buttonh);
   lbl_MW_ff02->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   xpos = border;
   ypos += buttonh + spacing;

   lbl_component = new QLabel(tr(" Component:"), this);
   Q_CHECK_PTR(lbl_component);
   lbl_component->setAlignment(AlignLeft|AlignVCenter);
   lbl_component->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_component->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_component->setGeometry(xpos, ypos, buttonw, buttonh);

   xpos += buttonw + spacing;

   cnt_component= new QwtCounter(this);
   Q_CHECK_PTR(cnt_component);
   cnt_component->setRange(1.0, 9999.0, 1.0);
   cnt_component->setValue(current_component);
   cnt_component->setNumButtons(3);
   cnt_component->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cnt_component->setEnabled(false);
   cnt_component->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(cnt_component, SIGNAL(valueChanged(double)), SLOT(update_component(double)));

   setup_GUI();
}

void US_FeMatch_W::setup_GUI()
{
   int j=0;
   int rows = 15, columns = 4, spacing = 2;

   QGridLayout * background = new QGridLayout(this,2,2,spacing);
   QGridLayout * subGrid1 = new QGridLayout(rows, columns, spacing);
   for (int i=0; i<rows; i++)
   {
      subGrid1->setRowSpacing(i, 26);
   }
   subGrid1->addMultiCellWidget(pb_load,j,j,0,1);
   subGrid1->addMultiCellWidget(pb_details,j,j,2,3);
   j++;
   subGrid1->addMultiCellWidget(pb_second_plot,j,j,0,1);
   subGrid1->addMultiCellWidget(pb_save,j,j,2,3);
   j++;
   subGrid1->addMultiCellWidget(pb_print,j,j,0,1);
   subGrid1->addMultiCellWidget(pb_view,j,j,2,3);
   j++;
   subGrid1->addMultiCellWidget(pb_help,j,j,0,1);
   subGrid1->addMultiCellWidget(pb_close,j,j,2,3);
   j++;
   subGrid1->addMultiCellWidget(banner1,j,j,0,3);
   j++;
   subGrid1->addMultiCellWidget(lbl_run_id1,j,j,0,1);
   subGrid1->addMultiCellWidget(lbl_run_id2,j,j,2,3);
   j++;
   subGrid1->addMultiCellWidget(lbl_temperature1,j,j,0,1);
   subGrid1->addMultiCellWidget(lbl_temperature2,j,j,2,3);
   j++;
   subGrid1->addMultiCellWidget(lbl_cell_info1,j,j,0,1);
   subGrid1->addMultiCellWidget(lbl_cell_info2,j,j,2,3);
   j++;
   subGrid1->addMultiCellWidget(lbl_cell_descr,j,j,0,3);
   j++;
   subGrid1->addMultiCellWidget(cell_select,j,j+2,0,1);
   subGrid1->addMultiCellWidget(lambda_select,j,j+2,2,3);
   j=j+3;
   subGrid1->addMultiCellWidget(banner2,j,j,0,3);
   j++;
   subGrid1->addWidget(pb_density,j,0);
   subGrid1->addWidget(density_le,j,1);
   subGrid1->addWidget(pb_viscosity,j,2);
   subGrid1->addWidget(viscosity_le,j,3);
   j++;
   subGrid1->addWidget(pb_vbar,j,0);
   subGrid1->addWidget(vbar_le,j,1);
   subGrid1->addWidget(lbl1_excluded,j,2);
   subGrid1->addWidget(lbl2_excluded,j,3);


   rows = 10, columns = 4, spacing = 2, j=0;
   QGridLayout * subGrid2 = new QGridLayout(rows, columns, spacing);
   for (int i=0; i<rows; i++)
   {
      subGrid2->setRowSpacing(i, 26);
   }
   subGrid2->addMultiCellWidget(pb_loadModel,j,j,0,1);
   subGrid2->addMultiCellWidget(pb_fit,j,j,2,3);
   j++;
   subGrid2->addMultiCellWidget(lbl_s,j,j,0,1);
   subGrid2->addMultiCellWidget(lbl_s2,j,j,2,3);
   j++;
   subGrid2->addMultiCellWidget(lbl_D,j,j,0,1);
   subGrid2->addMultiCellWidget(lbl_D2,j,j,2,3);
   j++;
   subGrid2->addMultiCellWidget(lbl_C,j,j,0,1);
   subGrid2->addMultiCellWidget(lbl_C2,j,j,2,3);
   j++;
   subGrid2->addMultiCellWidget(lbl_variance,j,j,0,1);
   subGrid2->addMultiCellWidget(lbl_variance2,j,j,2,3);
   j++;
   subGrid2->addMultiCellWidget(lbl_MW_ff01,j,j,0,1);
   subGrid2->addMultiCellWidget(lbl_MW_ff02,j,j,2,3);
   j++;
   subGrid2->addMultiCellWidget(lbl_component,j,j,0,1);
   subGrid2->addMultiCellWidget(cnt_component,j,j,2,3);
   j++;
   subGrid2->addMultiCellWidget(pb_exsingle,j,j,0,1);
   subGrid2->addMultiCellWidget(cnt_exsingle,j,j,2,3);
   j++;
   subGrid2->addMultiCellWidget(pb_exrange,j,j,0,1);
   subGrid2->addMultiCellWidget(cnt_exrange,j,j,2,3);
   j++;
   subGrid2->addWidget(lbl_status,j,0);
   subGrid2->addMultiCellWidget(progress,j,j,1,3);

   background->addLayout(subGrid1,0,0);
   background->addWidget(analysis_plot,0,1);
   background->addLayout(subGrid2,1,0);
   background->addWidget(edit_plot,1,1);
   background->setColStretch(0,1);
   background->setColStretch(1,4);
   background->setColSpacing(0,350);
   background->setColSpacing(1,550);

   qApp->processEvents();
   QRect r = background->cellGeometry(0, 0);

   global_Xpos += 30;
   global_Ypos += 30;

   this->setGeometry(global_Xpos, global_Ypos, r.width()+550+spacing*3, this->height());
}

void US_FeMatch_W::second_plot()
{
   plot2++;
   if (plot2 == 9)
   {
      plot2 = 1;
   }
   second_plot(plot2);
   if (plot2 == 1)
   {
      pb_second_plot->setText(tr("s20,W Distribution"));
   }
   if (plot2 == 2)
   {
      pb_second_plot->setText(tr("MW Distribution"));
   }
   if (plot2 == 3)
   {
      pb_second_plot->setText(tr("D20,W Distribution"));
   }
   if (plot2 == 4)
   {
      pb_second_plot->setText(tr("f/f0 vs. s20,W"));
   }
   if (plot2 == 5)
   {
      pb_second_plot->setText(tr("f/f0 vs. MW"));
   }
   if (plot2 == 6)
   {
      pb_second_plot->setText(tr("D20,W vs. s20,W"));
   }
   if (plot2 == 7)
   {
      pb_second_plot->setText(tr("D20,W vs. MW"));
   }
   if (plot2 == 8)
   {
      pb_second_plot->setText(tr("Residuals"));
   }
}

void US_FeMatch_W::second_plot(int val)
{
   if (resplot == NULL)
   {
      resplot = new US_ResidualPlot(0,0);
   }
   resplot->setData(&residuals, global_Xpos+30, global_Ypos+30);
   resplot->show();
   resplot->repaint();
   QwtSymbol symbol;
   unsigned int i, j;
   double max_y = -1.0e6;
   analysis_plot->setAxisAutoScale(QwtPlot::yLeft);
   //cout << "Val: " << val << ", components: " << components << endl;
   if (val == 1)
   {
      QString s1, s2;
      analysis_plot->clear();
      s1.sprintf(tr(": Cell %d (%d nm)\nResiduals"), selected_cell+1, run_inf.wavelength[selected_cell][selected_lambda]);
      s2 = tr("Run ");
      s2.append(run_inf.run_id);
      s2.append(s1);
      analysis_plot->setTitle(s2);
      double *x, line_x[2];
      double *y, line_y[2];
      unsigned int *curve, zeroline;
      x = new double [residuals.scan[0].conc.size()];
      y = new double [residuals.scan[0].conc.size()];
      curve = new unsigned int [residuals.scan.size()]; // one extra for the zero line
      QPen p_raw, p_zero;
      p_raw.setWidth(1);
      p_zero.setWidth(2);
      symbol.setStyle(QwtSymbol::Ellipse);
      p_raw.setColor(Qt::yellow);
      p_zero.setColor(Qt::red);
      symbol.setPen(Qt::yellow);
      symbol.setBrush(Qt::yellow);
      analysis_plot->setAxisTitle(QwtPlot::xBottom, tr("Radius in cm"));
      analysis_plot->setAxisTitle(QwtPlot::yLeft, "OD Difference");
      line_x[0] = residuals.radius[0] - (residuals.radius[residuals.radius.size()-1] - residuals.radius[0])/30.0;
      line_x[1] = residuals.radius[residuals.radius.size()-1] + (residuals.radius[residuals.radius.size()-1] - residuals.radius[0])/30;
      analysis_plot->setAxisScale(QwtPlot::xBottom, line_x[0], line_x[1]);
      line_y[0] = 0.0;
      line_y[1] = 0.0;
      symbol.setSize(1);
      zeroline = analysis_plot->insertCurve("Zero Line");
      analysis_plot->setCurveStyle(zeroline, QwtCurve::Lines);
      analysis_plot->setCurveData(zeroline, line_x, line_y, 2);
      analysis_plot->setCurvePen(zeroline, p_zero);

      for (i=0; i<residuals.radius.size(); i++)
      {
         x[i] = residuals.radius[i];
      }
      for (i=0; i<residuals.scan.size(); i++)
      {
         for (j=0; j<residuals.scan[0].conc.size(); j++)
         {
            y[j] = residuals.scan[i].conc[j];
         }
         curve[i] = analysis_plot->insertCurve("Residual Scan");
         analysis_plot->setCurveSymbol(curve[i], symbol);
         analysis_plot->setCurveStyle(curve[i], QwtCurve::NoCurve);
         analysis_plot->setCurveData(curve[i], x, y, residuals.radius.size());
         analysis_plot->setCurvePen(curve[i], p_raw);
      }
      analysis_plot->replot();
      delete [] curve;
      delete [] x;
      delete [] y;
   }
   if (val == 2)
   {
      QString s1, s2;
      double *x, *y;
      x = new double [components];
      y = new double [components];
      for (i=0; i<components; i++)
      {
         x[i] = s20w[i];
         y[i] = partial_concentration[i];
         max_y = max(max_y, y[i]);
      }
      analysis_plot->clear();
      s1.sprintf(tr(": Cell %d (%d nm)\nS20,W Distribution"), selected_cell+1, run_inf.wavelength[selected_cell][selected_lambda]);
      s2 = tr("Run ");
      s2.append(run_inf.run_id);
      s2.append(s1);
      analysis_plot->setTitle(s2);
      analysis_plot->setAxisTitle(QwtPlot::xBottom, tr("Corrected Sed. Coeff. (1e-13 s)"));
      analysis_plot->setAxisTitle(QwtPlot::yLeft, tr("Relative Concentration"));
      analysis_plot->setAxisAutoScale(QwtPlot::xBottom);
      analysis_plot->setAxisScale(QwtPlot::yLeft, 0, max_y);
      long distro = analysis_plot->insertCurve("C(s) Distribution");
      analysis_plot->setCurveStyle(distro, QwtCurve::Sticks);
      analysis_plot->setCurvePen(distro, QPen(Qt::yellow, 2, SolidLine));
      analysis_plot->setCurveData(distro, x, y, components);
      analysis_plot->replot();
      delete [] x;
      delete [] y;
   }
   if (val == 3)
   {
      QString s1, s2;
      double *x, *y;
      x = new double [components];
      y = new double [components];
      for (i=0; i<components; i++)
      {
         x[i] = mw[i];
         y[i] = partial_concentration[i];
         max_y = max(max_y, y[i]);
      }
      analysis_plot->clear();
      s1.sprintf(tr(": Cell %d (%d nm)\nMolecular Weight Distribution"), selected_cell+1, run_inf.wavelength[selected_cell][selected_lambda]);
      s2 = tr("Run ");
      s2.append(run_inf.run_id);
      s2.append(s1);
      analysis_plot->setTitle(s2);
      analysis_plot->setAxisTitle(QwtPlot::xBottom, tr("Molecular Weight (Dalton)"));
      analysis_plot->setAxisTitle(QwtPlot::yLeft, tr("Relative Concentration"));
      analysis_plot->setAxisAutoScale(QwtPlot::xBottom);
      analysis_plot->setAxisScale(QwtPlot::yLeft, 0, max_y);
      long distro = analysis_plot->insertCurve("Molecular Weight Distribution");
      analysis_plot->setCurveStyle(distro, QwtCurve::Sticks);
      analysis_plot->setCurvePen(distro, QPen(Qt::yellow, 2, SolidLine));
      analysis_plot->setCurveData(distro, x, y, components);
      analysis_plot->replot();
      delete [] x;
      delete [] y;
   }
   if (val == 4)
   {
      QString s1, s2;
      double *x, *y;
      x = new double [components];
      y = new double [components];
      for (i=0; i<components; i++)
      {
         //         x[i] = 1.0/pow(D20w[i], 0.5);
         x[i] = D20w[i];
         y[i] = partial_concentration[i];
         max_y = max(max_y, y[i]);
      }
      analysis_plot->clear();
      s1.sprintf(tr(": Cell %d (%d nm)\nD20,W Distribution"), selected_cell+1, run_inf.wavelength[selected_cell][selected_lambda]);
      s2 = tr("Run ");
      s2.append(run_inf.run_id);
      s2.append(s1);
      analysis_plot->setTitle(s2);
      analysis_plot->setAxisTitle(QwtPlot::xBottom, tr("D20,W (cm^2/sec)"));
      analysis_plot->setAxisTitle(QwtPlot::yLeft, tr("Relative Concentration"));
      //      analysis_plot->setAxisOptions(QwtPlot::xBottom, QwtAutoScale::Logarithmic);
      analysis_plot->setAxisAutoScale(QwtPlot::xBottom);
      analysis_plot->setAxisScale(QwtPlot::yLeft, 0, max_y);
      long distro = analysis_plot->insertCurve("Diffusion Coefficient Distribution");
      analysis_plot->setCurveStyle(distro, QwtCurve::Sticks);
      analysis_plot->setCurvePen(distro, QPen(Qt::yellow, 2, SolidLine));
      analysis_plot->setCurveData(distro, x, y, components);
      analysis_plot->replot();
      delete [] x;
      delete [] y;
   }
   if (val == 5) // f/f0 vs s
   {
      QString s1, s2;
      symbol.setStyle(QwtSymbol::Ellipse);
      symbol.setPen(Qt::red);
      symbol.setBrush(Qt::yellow);
      if (components < 100 && components >= 50)
      {
         symbol.setSize(8);
      }
      else if (components < 50  && components >= 20)
      {
         symbol.setSize(10);
      }
      else if (components < 20)
      {
         symbol.setSize(12);
      }
      else if (components >= 100)
      {
         symbol.setSize(6);
      }
      double *x, *y;
      x = new double [components];
      y = new double [components];
      for (i=0; i<components; i++)
      {
         x[i] = s20w[i];
         y[i] = f_f0[i];
      }
      analysis_plot->clear();
      s1.sprintf(tr(": Cell %d (%d nm)\nf/f0 vs S"), selected_cell+1, run_inf.wavelength[selected_cell][selected_lambda]);
      s2 = tr("Run ");
      s2.append(run_inf.run_id);
      s2.append(s1);
      analysis_plot->setTitle(s2);
      analysis_plot->setAxisTitle(QwtPlot::xBottom, tr("s20,W (cm^2/sec)"));
      analysis_plot->setAxisTitle(QwtPlot::yLeft, tr("Frictional Ratio f/f0"));
      analysis_plot->setAxisAutoScale(QwtPlot::xBottom);
      //      analysis_plot->setAxisScale(QwtPlot::yLeft, 0, max_y);
      long distro = analysis_plot->insertCurve("Solute Distribution");
      analysis_plot->setCurveStyle(distro, QwtCurve::NoCurve);
      analysis_plot->setCurveSymbol(distro, symbol);
      //      analysis_plot->setCurvePen(distro, QPen(Qt::yellow, 2, SolidLine));
      analysis_plot->setCurveData(distro, x, y, components);
      analysis_plot->replot();
      delete [] x;
      delete [] y;
   }
   if (val == 6) // f/f0 vs MW
   {
      QString s1, s2;
      symbol.setStyle(QwtSymbol::Ellipse);
      symbol.setPen(Qt::red);
      symbol.setBrush(Qt::yellow);
      if (components < 100 && components >= 50)
      {
         symbol.setSize(8);
      }
      else if (components < 50  && components >= 20)
      {
         symbol.setSize(10);
      }
      else if (components < 20)
      {
         symbol.setSize(12);
      }
      else if (components >= 100)
      {
         symbol.setSize(6);
      }
      double *x, *y;
      x = new double [components];
      y = new double [components];
      for (i=0; i<components; i++)
      {
         x[i] = mw[i];
         y[i] = f_f0[i];
      }
      analysis_plot->clear();
      s1.sprintf(tr(": Cell %d (%d nm)\nf/f0 vs Mol. Weight"), selected_cell+1, run_inf.wavelength[selected_cell][selected_lambda]);
      s2 = tr("Run ");
      s2.append(run_inf.run_id);
      s2.append(s1);
      analysis_plot->setTitle(s2);
      analysis_plot->setAxisTitle(QwtPlot::xBottom, tr("Molecular Weight"));
      analysis_plot->setAxisTitle(QwtPlot::yLeft, tr("Frictional Ratio f/f0"));
      analysis_plot->setAxisAutoScale(QwtPlot::xBottom);
      //      analysis_plot->setAxisScale(QwtPlot::yLeft, 0, max_y);
      long distro = analysis_plot->insertCurve("Solute Distribution");
      analysis_plot->setCurveStyle(distro, QwtCurve::NoCurve);
      analysis_plot->setCurveSymbol(distro, symbol);
      //      analysis_plot->setCurvePen(distro, QPen(Qt::yellow, 2, SolidLine));
      analysis_plot->setCurveData(distro, x, y, components);
      analysis_plot->replot();
      delete [] x;
      delete [] y;
   }
   if (val == 7) // D vs s
   {
      QString s1, s2;
      symbol.setStyle(QwtSymbol::Ellipse);
      symbol.setPen(Qt::red);
      symbol.setBrush(Qt::yellow);
      if (components < 100 && components >= 50)
      {
         symbol.setSize(8);
      }
      else if (components < 50  && components >= 20)
      {
         symbol.setSize(10);
      }
      else if (components < 20)
      {
         symbol.setSize(12);
      }
      else if (components >= 100)
      {
         symbol.setSize(6);
      }
      double *x, *y;
      x = new double [components];
      y = new double [components];
      for (i=0; i<components; i++)
      {
         x[i] = s20w[i];
         y[i] = D20w[i];
      }
      analysis_plot->clear();
      s1.sprintf(tr(": Cell %d (%d nm)\nDiff. Coeff. vs Sed. Coeff."), selected_cell+1, run_inf.wavelength[selected_cell][selected_lambda]);
      s2 = tr("Run ");
      s2.append(run_inf.run_id);
      s2.append(s1);
      analysis_plot->setTitle(s2);
      analysis_plot->setAxisTitle(QwtPlot::xBottom, tr("Sedimentation Coefficient s20,W"));
      analysis_plot->setAxisTitle(QwtPlot::yLeft, tr("Diffusion Coefficient D20,W"));
      analysis_plot->setAxisAutoScale(QwtPlot::xBottom);
      //      analysis_plot->setAxisScale(QwtPlot::yLeft, 0, max_y);
      long distro = analysis_plot->insertCurve("Solute Distribution");
      analysis_plot->setCurveStyle(distro, QwtCurve::NoCurve);
      analysis_plot->setCurveSymbol(distro, symbol);
      //      analysis_plot->setCurvePen(distro, QPen(Qt::yellow, 2, SolidLine));
      analysis_plot->setCurveData(distro, x, y, components);
      analysis_plot->replot();
      delete [] x;
      delete [] y;
   }
   if (val == 8) // D vs MW
   {
      QString s1, s2;
      symbol.setStyle(QwtSymbol::Ellipse);
      symbol.setPen(Qt::red);
      symbol.setBrush(Qt::yellow);
      if (components < 100 && components >= 50)
      {
         symbol.setSize(8);
      }
      else if (components < 50  && components >= 20)
      {
         symbol.setSize(10);
      }
      else if (components < 20)
      {
         symbol.setSize(12);
      }
      else if (components >= 100)
      {
         symbol.setSize(6);
      }
      double *x, *y;
      x = new double [components];
      y = new double [components];
      for (i=0; i<components; i++)
      {
         x[i] = mw[i];
         y[i] = D20w[i];
      }
      analysis_plot->clear();
      s1.sprintf(tr(": Cell %d (%d nm)\nDiff. Coeff vs. Molecular Weight"), selected_cell+1, run_inf.wavelength[selected_cell][selected_lambda]);
      s2 = tr("Run ");
      s2.append(run_inf.run_id);
      s2.append(s1);
      analysis_plot->setTitle(s2);
      analysis_plot->setAxisTitle(QwtPlot::xBottom, tr("Molecular Weight"));
      analysis_plot->setAxisTitle(QwtPlot::yLeft, tr("Diffusion Coefficient D20,W"));
      analysis_plot->setAxisAutoScale(QwtPlot::xBottom);
      //      analysis_plot->setAxisScale(QwtPlot::yLeft, 0, max_y);
      long distro = analysis_plot->insertCurve("Solute Distribution");
      analysis_plot->setCurveStyle(distro, QwtCurve::NoCurve);
      analysis_plot->setCurveSymbol(distro, symbol);
      //      analysis_plot->setCurvePen(distro, QPen(Qt::yellow, 2, SolidLine));
      analysis_plot->setCurveData(distro, x, y, components);
      analysis_plot->replot();
      delete [] x;
      delete [] y;
   }
}

void US_FeMatch_W::enableButtons()
{
   pb_second_plot->setEnabled(false);
   pb_save->setEnabled(false);
   pb_view->setEnabled(false);
   pb_print->setEnabled(false);
   pb_loadModel->setEnabled(true);
}

void US_FeMatch_W::save()
{
   if (step == 0)
   {
      QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n      Click on \"Load Data\"\n"));
      return;
   }
   QPixmap p;
   QString fileName;
   write_cofs();
   write_res();
   if(!window_3d_flag)
   {
      QMessageBox mb(tr("UltraScan FE Match"),
                     tr("Do you want to save the 3-D distribution plot?"),
                     QMessageBox::Question,
                     QMessageBox::Yes | QMessageBox::Default,
                     QMessageBox::No,
                     QMessageBox::Cancel | QMessageBox::Escape, 0, 0, false, 0);

      switch (mb.exec())
      {
      case QMessageBox::Cancel:
         {
            return;
         }
      case QMessageBox::Yes:
         {
            us_3d_solutes = new US_3d_Solutes(&sa2d_ctrl_vars, &window_3d_flag, run_inf.run_id,
                                              &run_inf.scans[selected_cell][selected_lambda], &points, selected_cell, selected_lambda,
                                              absorbance, radius, &fem_model, &ti_noise, &ri_noise);
            us_3d_solutes->solutes.resize(s20w.size());
            for (unsigned int i=0; i<s20w.size(); i++)
            {
               us_3d_solutes->solutes[i].s = s20w[i];
               us_3d_solutes->solutes[i].D = D20w[i];
               us_3d_solutes->solutes[i].mw = mw[i];
               us_3d_solutes->solutes[i].c = partial_concentration[i];
               us_3d_solutes->solutes[i].f = fv[i];
               us_3d_solutes->solutes[i].ff0 = f_f0[i];
            }
            us_3d_solutes->show();
            us_3d_solutes->plot();
            QMessageBox mb2(tr("UltraScan FE Match"),
                            tr("If you want to edit the 3D plot first,\n"
                               "click on \"Edit First\", then click\n"
                               "on \"Save Data\" again.\n"
                               "Otherwise, click on \"Continue...\""),
                            QMessageBox::Information,
                            QMessageBox::Yes,
                            QMessageBox::No,
                            QMessageBox::Cancel, 0, 0, false, 0);
            mb2.setButtonText(QMessageBox::Yes, tr("Edit first"));
            mb2.setButtonText(QMessageBox::No, tr("Continue..."));
            switch (mb2.exec())
            {
               case QMessageBox::Yes:
               {
                  us_3d_solutes->mainwindow->raise();
                  qApp->processEvents();
                  return;
               }
               case QMessageBox::Cancel:
               {
                  return;
               }
               case QMessageBox::No:
               {
                  us_3d_solutes->mainwindow->raise();
                  us_3d_solutes->show();
                  us_3d_solutes->plot();
                  qApp->processEvents();
                  fileName.sprintf(htmlDir + "/" + analysis_type + "_3ddis_%d%d.png", selected_cell + 1, selected_lambda +1);
                  us_3d_solutes->mainwindow->dumpImage(fileName, "PNG");
                  qApp->processEvents();
               }
            }
         }
      }
   }
   else
   {
      us_3d_solutes->raise();
      qApp->processEvents();
      if (!us_3d_solutes->widget3d_flag)
      {
         us_3d_solutes->plot();
         qApp->processEvents();
      }
      else
      {
         us_3d_solutes->mainwindow->raise();
         qApp->processEvents();
      }
      QMessageBox mb(tr("UltraScan FE Match"),
                     tr("Save the 3-D distribution plot?"),
                     QMessageBox::Question,
                     QMessageBox::Yes | QMessageBox::Default,
                     QMessageBox::No,
                     QMessageBox::Cancel | QMessageBox::Escape);
      switch (mb.exec())
      {
      case QMessageBox::Cancel:
         {
            return;
         }
         case QMessageBox::Yes:
         {
            us_3d_solutes->mainwindow->raise();
            qApp->processEvents();
            QMessageBox mb2(tr("UltraScan FE Match"),
                            tr("If you want to edit the 3D plot first,\n"
                               "click on \"Edit First\", then click\n"
                               "on \"Save Data\" again.\n"
                               "Otherwise, click on \"Continue...\""),
                            QMessageBox::Information,
                            QMessageBox::Yes,
                            QMessageBox::No,
                            QMessageBox::Cancel, 0, 0, false, 0);
            mb2.setButtonText(QMessageBox::Yes, tr("Edit first"));
            mb2.setButtonText(QMessageBox::No, tr("Continue..."));
            switch (mb2.exec())
            {
               case QMessageBox::Yes:
               {
                  return;
               }
               case QMessageBox::Cancel:
               {
                  return;
               }
               case QMessageBox::No:
               {
                  us_3d_solutes->mainwindow->raise();
                  us_3d_solutes->show();
                  us_3d_solutes->plot();
                  qApp->processEvents();
                  fileName.sprintf(htmlDir + "/" + analysis_type + "_3ddis_%d%d.png", selected_cell + 1, selected_lambda +1);
                  us_3d_solutes->mainwindow->dumpImage(fileName, "PNG");
               }
            }
         }
      }
   }
   second_plot(1);
   qApp->processEvents();
   fileName.sprintf(htmlDir + "/" + analysis_type + "_resid_%d%d.", selected_cell + 1, selected_lambda +1);
   p = QPixmap::grabWidget(analysis_plot, 2, 2, analysis_plot->width() - 4, analysis_plot->height() - 4);
   pm->save_file(fileName, p);
   second_plot(2);
   qApp->processEvents();
   fileName.sprintf(htmlDir + "/" + analysis_type + "_dis_%d%d.", selected_cell + 1, selected_lambda +1);
   p = QPixmap::grabWidget(analysis_plot, 2, 2, analysis_plot->width() - 4, analysis_plot->height() - 4);
   pm->save_file(fileName, p);
   second_plot(3);
   qApp->processEvents();
   fileName.sprintf(htmlDir + "/" + analysis_type + "_mw_%d%d.", selected_cell + 1, selected_lambda +1);
   p = QPixmap::grabWidget(analysis_plot, 2, 2, analysis_plot->width() - 4, analysis_plot->height() - 4);
   pm->save_file(fileName, p);
   second_plot(4);
   qApp->processEvents();
   fileName.sprintf(htmlDir + "/" + analysis_type + "_d_%d%d.", selected_cell + 1, selected_lambda +1);
   p = QPixmap::grabWidget(analysis_plot, 2, 2, analysis_plot->width() - 4, analysis_plot->height() - 4);
   pm->save_file(fileName, p);
   second_plot(5);
   qApp->processEvents();
   fileName.sprintf(htmlDir + "/" + analysis_type + "_fs_%d%d.", selected_cell + 1, selected_lambda +1);
   p = QPixmap::grabWidget(analysis_plot, 2, 2, analysis_plot->width() - 4, analysis_plot->height() - 4);
   pm->save_file(fileName, p);
   second_plot(6);
   qApp->processEvents();
   fileName.sprintf(htmlDir + "/" + analysis_type + "_fmw_%d%d.", selected_cell + 1, selected_lambda +1);
   p = QPixmap::grabWidget(analysis_plot, 2, 2, analysis_plot->width() - 4, analysis_plot->height() - 4);
   pm->save_file(fileName, p);
   second_plot(7);
   qApp->processEvents();
   fileName.sprintf(htmlDir + "/" + analysis_type + "_ds_%d%d.", selected_cell + 1, selected_lambda +1);
   p = QPixmap::grabWidget(analysis_plot, 2, 2, analysis_plot->width() - 4, analysis_plot->height() - 4);
   pm->save_file(fileName, p);
   second_plot(8);
   qApp->processEvents();
   fileName.sprintf(htmlDir + "/" + analysis_type + "_dmw_%d%d.", selected_cell + 1, selected_lambda +1);
   p = QPixmap::grabWidget(analysis_plot, 2, 2, analysis_plot->width() - 4, analysis_plot->height() - 4);
   pm->save_file(fileName, p);
   fileName.sprintf(htmlDir + "/" + analysis_type + "_edited_%d%d.", selected_cell + 1, selected_lambda +1);
   p = QPixmap::grabWidget(edit_plot, 2, 2, edit_plot->width() - 4, edit_plot->height() - 4);
   pm->save_file(fileName, p);
   fileName.sprintf(htmlDir + "/" + analysis_type + "_pixmap_%d%d.", selected_cell + 1, selected_lambda +1);
   p = QPixmap::grabWidget(resplot, 1, 1, resplot->width() - 2, resplot->height() - 2);
   pm->save_file(fileName, p);
   QFile f;
   QString temp;
   fileName = USglobal->config_list.result_dir.copy() + "/" + run_inf.run_id + "." + analysis_type + "_fit" +
      temp.sprintf(".%d%d", selected_cell+1, selected_lambda+1);
   f.setName(fileName);
   if (f.open(IO_WriteOnly))
   {
      QDataStream ds(&f);
      ds << (QString) run_inf.run_id;
      ds << (Q_UINT16) selected_cell;
      ds << (Q_UINT16) selected_lambda;
      ds << (Q_UINT16) run_inf.scans[selected_cell][selected_lambda];
      ds << (Q_UINT16) points;
      for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
      {
         ds << (Q_UINT32) run_inf.time[selected_cell][selected_lambda][i];
      }
      ds << (Q_UINT16) mw.size();
      for (i=0; i<mw.size(); i++)
      {// all doubles:
         ds << partial_concentration[i] << mw[i] << s20w[i] << D20w[i] << f_f0[i] << fv[i];
      }
      Q_UINT16 m;
      if (ti_noise.size() == points)
      {
         m = 1;
         ds << m;
         ds << (Q_UINT16) ti_noise.size();
         for (i=0; i<ti_noise.size(); i++)
         {
            ds << (double) ti_noise[i];
         }
      }
      else
      {
         m = 0;
         ds << m;
      }
      if (ri_noise.size() == run_inf.scans[selected_cell][selected_lambda])
      {
         m = 1;
         ds << m;
         ds << (Q_UINT16) ri_noise.size();
         for (i=0; i<ri_noise.size(); i++)
         {
            ds << (double) ri_noise[i];
         }
      }
      else
      {
         m = 0;
         ds << m;
      }
      double dval = 1.0e34;
      for (i=0; i<s20w.size(); i++)
      {
         dval = min(s20w[i]*1e13, dval);
      }
      ds << dval;
      dval = 0.0;
      for (i=0; i<s20w.size(); i++)
      {
         dval = max(s20w[i]*1e13, dval);
      }
      ds <<   dval;
      m = 10;
      ds << (Q_UINT16) m;
      dval = 1.0e34;
      for (i=0; i<f_f0.size(); i++)
      {
         dval = min(f_f0[i], dval);
      }
      ds << dval;
      dval = 0.0;
      for (i=0; i<f_f0.size(); i++)
      {
         dval = max(f_f0[i], dval);
      }
      ds <<   dval;
      m = 10;
      ds << (Q_UINT16) m;
      ds << rmsd; // float
      ds << (Q_UINT16) fem_model.scan.size();
      for (i=0; i<fem_model.scan.size(); i++)
      {
         ds << (Q_UINT16) fem_model.scan[i].conc.size();
         ds << fem_model.scan[i].time; // double
         for (j=0; j<fem_model.scan[i].conc.size(); j++)
         {
            ds << fem_model.scan[i].conc[j]; // double
         }
      }
      f.close();
   }
}

void US_FeMatch_W::update_distribution()
{
   s_distribution.clear();
   D_distribution.clear();
   calc_correction(run_inf.avg_temperature);
   for (i=0; i<components; i++)
   {
      // this distro is s20,W corrected from vhw and needs to be
      s_distribution.push_back(s20w[i] / correction);     // adjusted for temperature and buffer to match expt. data
      D_distribution.push_back(D20w[i] * (K0 + run_inf.avg_temperature) * (100.0 * VISC_20W)/( K20 * viscosity_tb));
   }
}

void US_FeMatch_W::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/fematch.html");
}

void US_FeMatch_W::update_component(double val)
{
   current_component = (unsigned int) val;
   update_labels();
}

void US_FeMatch_W::view()
{
   if (step == 0)
   {
      QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n      Click on \"Load Data\"\n"));
      return;
   }
   QString filestr, filename, temp;
   filename = USglobal->config_list.result_dir.copy();
   filename.append("/");
   filename.append(run_inf.run_id);
   temp.sprintf(".%d%d", selected_cell+1, selected_lambda+1);
   filestr = filename.copy();
   filestr.append("." + analysis_type + "_res");
   filestr.append(temp);
   write_res();
   TextEdit *e;
   e = new TextEdit();
   e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
   e->load(filestr);
   e->show();

}

void US_FeMatch_W::write_res()
{
   QString filestr, filename, temp, str;
   filename = USglobal->config_list.result_dir.copy();
   filename.append("/");
   filename.append(run_inf.run_id);
   temp.sprintf(".%d%d", selected_cell+1, selected_lambda+1);
   filestr = filename.copy();
   filestr.append("." + analysis_type + "_res");
   filestr.append(temp);
   QFile res_f(filestr);
   if (res_f.open(IO_WriteOnly))
   {
      QTextStream ts(&res_f);
      ts << "***************************************************\n";
      if (analysis_type == "sa2d")
      {
         str =  tr("*        2-dimensional Spectrum Analysis         *\n");
      }
      else if (analysis_type == "cofs")
      {
         str = tr("*                 C(s) Analysis                   *\n");
      }
      else if (analysis_type == "fe")
      {
         str = tr("*             Finite Element Analysis             *\n");
      }
      else if (analysis_type == "ga")
      {
         str = tr("*           Genetic Algorithm Analysis            *\n");
      }
      else
      {
         str =  tr("*        2-dimensional Spectrum Analysis         *\n");
      }

      ts << str;
      ts << "***************************************************\n\n\n";
      ts << tr("Data Report for Run \"") << run_inf.run_id << tr("\", Cell ") << (selected_cell + 1)
         << tr(", Wavelength ") << (selected_lambda + 1) << "\n\n";
      ts << tr("Detailed Run Information:\n\n");
      ts << tr("Cell Description:        ") << run_inf.cell_id[selected_cell] << "\n";
      ts << tr("Raw Data Directory:      ") << run_inf.data_dir << "\n";
      ts << tr("Rotor Speed:             ") << run_inf.rpm[first_cell][0][0]  << " rpm\n";
      ts << tr("Average Temperature:     ") << run_inf.avg_temperature << " ºC\n" ;
      if (run_inf.temperature_check == 1)
      {
         ts << tr("Temperature Variation:   Within Tolerance\n");
      }
      if (run_inf.temperature_check == -1)
      {
         ts << tr("Temperature Variation: (!) OUTSIDE TOLERANCE (!)\n");
      }
      i = (unsigned int) (run_inf.time_correction / 60);
      j = (unsigned int) (0.5 + (run_inf.time_correction - (i * 60)));
      ts << tr("Time Correction:         ") << i << tr(" minute(s) ") << j << tr(" second(s)\n");
      i = (unsigned int) (run_inf.duration / 3600);
      j = (unsigned int) (0.5 + (run_inf.duration - i * 3600) / 60);
      ts << tr("Run Duration:            ") << i << tr(" hour(s) ") << j << tr(" minute(s)\n");
      ts << tr("Wavelength:              ") << run_inf.wavelength[selected_cell][selected_lambda] << " nm\n";
      ts << tr("Baseline Absorbance:     ") << run_inf.baseline[selected_cell][selected_lambda] << " OD\n";
      ts << tr("Meniscus Position:       ") << run_inf.meniscus[selected_cell] << " cm\n";
      ts << tr("Edited Data starts at:   ") << run_inf.range_left[selected_cell][selected_lambda][0] << " cm\n";
      ts << tr("Edited Data stops at:    ") << run_inf.range_right[selected_cell][selected_lambda][0] << " cm\n\n\n";
      ts << tr("Hydrodynamic Settings:\n\n");
      ts << tr("Viscosity correction:    ") << viscosity << "\n" ;
      ts << tr("Viscosity (absolute):    ") << viscosity_tb << "\n" ;
      ts << tr("Density correction:      ") << density << " g/ccm\n" ;
      ts << tr("Density (absolute):      ") << density_tb << " g/ccm\n" ;
      ts << tr("Vbar:                    ") << vbar << " ccm/g\n" ;
      ts << tr("Vbar corrected for 20ºC: ") << vbar20 << " ccm/g\n" ;
      ts << tr("Buoyancy (Water, 20ºC) : ") << buoyancyw << "\n" ;
      ts << tr("Buoyancy (absolute)      ") << buoyancyb << "\n" ;
      ts << tr("Correction Factor:       ") << correction << "\n\n\n";
      ts << tr("Data Analysis Settings:\n\n");
      ts << tr("Number of Components:    ") << components << "\n";
      ts << tr("Residual Mean Square Deviation: ") << rmsd << "\n";
      /*
      // take the first s/MW pair and calculate an f/f0 corrected for 20,W:
      float f0, vol, rad_sphere, frict;
      vol = vbar20 * mw[0] / AVOGADRO;
      rad_sphere = pow((double) (vol * (3.0/4.0))/M_PI, (double) (1.0/3.0));
      f0 = rad_sphere * 6.0 * M_PI * VISC_20W;
      frict = (mw[0] * (1.0 - vbar20 * DENS_20W)) / (s20w[0] * AVOGADRO);
      ts << "f/f0 corrected for Water at 20ºC: " << frict/f0 << "\n\n\n";
      */
      ts << tr("Weight-Average sedimentation coefficient:\n\n");
      float sum_mw=0.0;
      float sum_s=0.0;
      float sum_freq=0.0;
      float sum_D=0.0;
      for (i=0; i<components; i++)
      {
         sum_mw += partial_concentration[i] * mw[i];
         sum_s += partial_concentration[i] * s20w[i];
         sum_D += partial_concentration[i] * D20w[i];
         sum_freq += partial_concentration[i];
      }
      ts << str.sprintf("Weight Average S20,W: %6.4e\n", sum_s/sum_freq);
      ts << str.sprintf("Weight Average D20,W: %6.4e\n", sum_D/sum_freq);
      ts << str.sprintf("Weight Average Molecular Weight: %6.4e\n", sum_mw/sum_freq);
      ts << str.sprintf("Total concentration: %6.4e\n", sum_freq);
      ts << "\n\n";
      ts << tr("Distribution Information:\n\n");
      ts << tr("Molecular Weight:\tS 20,W:\t\tD 20,W:\t\t\tconcentration:\n\n");
      for (i=0; i<components; i++)
      {
         ts << str.sprintf("%6.4e\t\t%6.4e\t\t%6.4e\t\t%6.4e\t(%6.3f",
                           mw[i], s20w[i], D20w[i], partial_concentration[i], 100.0 * partial_concentration[i]/sum_freq);
         ts << " %)\n";
      }
      for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
      {
         if (i+1<10)
         {
            ts << "  " << (i+1) << ":    ";
         }
         else if (i+1 < 100 && i+1 > 9)
         {
            ts << " " << (i+1) << ":    ";
         }
         else if (i+1 > 99)
         {
            ts << (i+1) << ":    ";
         }
         k = (unsigned int) (run_inf.time[selected_cell][selected_lambda][i]/ 60);
         j = (unsigned int) (0.5 + (run_inf.time[selected_cell][selected_lambda][i] - (k * 60)));
         ts.width(5);
         ts << k << tr(" min");
         ts.width(3);
         ts << j << tr(" sec     ");
         ts.width(11);
         ts << run_inf.plateau[selected_cell][selected_lambda][i]
            << " OD (" << run_inf.omega_s_t[selected_cell][selected_lambda][i]
            << ", " << simdata[0].scan[i].omega_s_t << ")\n";
      }

      res_f.close();
   }
}

void US_FeMatch_W::write_cofs()
{
   QString filestr, filename, temp;
   filename = USglobal->config_list.result_dir.copy();
   filename.append("/");
   filename.append(run_inf.run_id);
   temp.sprintf(".%d%d", selected_cell+1, selected_lambda+1);
   filestr = filename.copy();
   filestr.append("."+ analysis_type + "_dis");
   filestr.append(temp);
   //   QFile f(filestr);
   FILE *f;
   f = fopen(filestr, "w");
   unsigned int i;
   if (f != NULL)
   {
      fprintf(f, "S_apparent\tS_20,W    \tD_apparent\tD_20,W    \tMW        \tFrequency\tf/f0(20,W)\n");
      if (analysis_type == "ga_mc" || analysis_type == "sa2d_mc")
      {
         fprintf(f, "%d\t\t# Number of Monte Carlo Iterations\n", monte_carlo_iterations);
      }
      for (i=0; i<components; i++)
      {
         fprintf(f, "%6.4e\t%6.4e\t%6.4e\t%6.4e\t%6.4e\t%6.4e\t%6.4e\n", s_distribution[i], s20w[i], D_distribution[i], D20w[i], mw[i], partial_concentration[i], f_f0[i]);
      }

      fclose(f);
   }
   /*
     if (f.open(IO_WriteOnly))
     {
     QTextStream ts(&f);
     ts << "S_apparent\tS_20,W    \tD_apparent\tD_20,W    \tMW        \tFrequency\tf/f0(20,W)\n";
     if (analysis_type == "ga_mc" || analysis_type == "sa2d_mc")
     {
     ts << monte_carlo_iterations << "\t\t# Number of Monte Carlo Iterations\n";
     }
     QString str2, str3;
     for (i=0; i<components; i++)
     {
     //         ts << temp.sprintf("%6.4e\t%6.4e\t%6.4e\t%6.4e\t%6.4e\t%6.4e\t%6.4e\n", s_distribution[i], s20w[i], D_distribution[i], D20w[i], mw[i], partial_concentration[i], f_f0[i]);
     //cout << temp;
     ts << s_distribution[i] << "\t" << s20w[i] << "\t" <<  D_distribution[i] << "\t" <<  D20w[i] << "\t" <<  mw[i] << "\t" <<  partial_concentration[i] << "\t" <<  f_f0[i] << endl;
     //         printf("%6.4e\t%6.4e\t%6.4e\t%6.4e\t%6.4e\t%6.4e\t%6.4e\n", s_distribution[i], s20w[i], D_distribution[i], D20w[i], mw[i], partial_concentration[i], f_f0[i]);
     }
     f.close();
     }
   */
}

void US_FeMatch_W::fit()
{
   calc_residuals_ra();
   if(!window_3d_flag)
   {
      us_3d_solutes = new US_3d_Solutes(&sa2d_ctrl_vars, &window_3d_flag, run_inf.run_id,
                                        &run_inf.scans[selected_cell][selected_lambda], &points, selected_cell, selected_lambda,
                                        absorbance, radius, &fem_model, &ti_noise, &ri_noise);
      us_3d_solutes->solutes.resize(s20w.size());
      for (unsigned int i=0; i<s20w.size(); i++)
      {
         us_3d_solutes->solutes[i].s = s20w[i];
         us_3d_solutes->solutes[i].D = D20w[i];
         us_3d_solutes->solutes[i].mw = mw[i];
         us_3d_solutes->solutes[i].c = partial_concentration[i];
         us_3d_solutes->solutes[i].f = fv[i];
         us_3d_solutes->solutes[i].ff0 = f_f0[i];
      }
      us_3d_solutes->show();
   }
   else
   {
      us_3d_solutes->raise();
   }
}

void US_FeMatch_W::update_s(const QString &str)
{
   s20w[current_component-1] = str.toDouble();
}

void US_FeMatch_W::update_D(const QString &str)
{
   D20w[current_component-1] = str.toDouble();
}

void US_FeMatch_W::update_C(const QString &str)
{
   partial_concentration[current_component-1] = str.toDouble();
}

void US_FeMatch_W::update_baseline(const QString &str)
{
   //cout << "updating baseline...\n";
   baseline = str.toFloat();
}

void US_FeMatch_W::update_labels()
{
   if (current_component == 0)
   {
      return;
   }
   QString str;
   lbl_s2->setText(str.setNum(s20w[current_component-1]));
   lbl_D2->setText(str.setNum(D20w[current_component-1]));
   lbl_C2->setText(str.setNum(partial_concentration[current_component-1]));
   str.sprintf(" %8.2f kD, %4.2f", mw[current_component-1]/1000, f_f0[current_component-1]);
   lbl_MW_ff02->setText(str);
}

fematch_thr_t::fematch_thr_t(int a_thread) : QThread()
{
   thread = a_thread;
   work_to_do = 0;
   work_done = 1;
   work_to_do_waiters = 0;
   work_done_waiters = 0;
}

void fematch_thr_t::fematch_thr_setup(QProgressBar *a_progress,
                                      struct mfem_data *a_fem_data,
                                      struct mfem_data *a_experiment,
                                      struct mfem_data *a_residuals,
                                      struct mfem_initial *a_initCVector,
                                      US_MovingFEM *a_mfem,
                                      struct runinfo *a_run_inf,
                                      vector <double> *a_s_distribution,
                                      vector <double> *a_D_distribution,
                                      unsigned int a_selected_cell,
                                      unsigned int a_selected_lambda,
                                      vector <double> *a_partial_concentration,
                                      double a_bottom,
                                      unsigned int a_points,
                                      unsigned int a_j_start,
                                      unsigned int a_j_end)
{
   /* this starts up a new work load for the thread */
   progress = a_progress;
   fem_data = a_fem_data;
   experiment = a_experiment;
   residuals = a_residuals;
   initCVector = a_initCVector;
   mfem = a_mfem;
   run_inf = a_run_inf;
   s_distribution = a_s_distribution;
   D_distribution = a_D_distribution;
   selected_cell = a_selected_cell;
   selected_lambda = a_selected_lambda;
   partial_concentration = a_partial_concentration;
   bottom = a_bottom;
   points = a_points;
   j_start = a_j_start;
   j_end = a_j_end;

   work_mutex.lock();
   work_to_do = 1;
   work_done = 0;
   work_mutex.unlock();
   cond_work_to_do.wakeOne();

   //  cerr << "thread " << thread << " has new work to do\n";
}

void fematch_thr_t::fematch_thr_shutdown()
{
   /* this signals the thread to exit the run method */
   work_mutex.lock();
   work_to_do = -1;
   work_mutex.unlock();
   cond_work_to_do.wakeOne();

   //  cerr << "thread " << thread << " shutdown requested\n";
}

void fematch_thr_t::fematch_thr_wait()
{
   /* this is for the master thread to wait until the work is done */
   work_mutex.lock();

   //  cerr << "thread " << thread << " has a waiter\n";

   while(!work_done) {
      cond_work_done.wait(&work_mutex);
   }
   work_done = 0;
   work_mutex.unlock();

   //  cerr << "thread " << thread << " waiter released\n";
}

void fematch_thr_t::run()
{
   while(1)
   {
      work_mutex.lock();
      //    cerr << "thread " << thread << " waiting for work\n";
      work_to_do_waiters++;
      while(!work_to_do)
      {
         cond_work_to_do.wait(&work_mutex);
      }
      if(work_to_do == -1)
      {
         //      cerr << "thread " << thread << " shutting down\n";
         work_mutex.unlock();
         return;
      }

      work_to_do_waiters = 0;
      work_mutex.unlock();
      //    cerr << "thread " << thread << " starting work\n";
      unsigned int i, j, k;

      for (i = j_start; i <= j_end; i++)
      {
         //      printf("thread %u working on comp %u\n", thread, i);
         // for each term in the linear combination we need to reset the
         // simulation vectors, the experimental vector simply keeps getting overwritten:
         //      clear_data(&(fem_data));
         {
            unsigned int i;
            for (i=0; i<(*fem_data).scan.size(); i++)
            {
               (*fem_data).scan[i].conc.clear();
            }
            (*fem_data).radius.clear();
            (*fem_data).scan.clear();
         }
         for (j=0; j<(*experiment).scan.size(); j++)
         {
            for (k=0; k<points; k++)
            {
               // reset concentration to zero:
               (*experiment).scan[j].conc[k] = 0.0;
            }
         }

         mfem->set_params(100, (*s_distribution)[i], (*D_distribution)[i],
                          (double) (*run_inf).rpm[selected_cell][selected_lambda][0],
                          (*experiment).scan[(*experiment).scan.size()-1].time,
                          (double) (*run_inf).meniscus[selected_cell], bottom, (double) (*partial_concentration)[i], initCVector);

         // generate the next term of the linear combination:
         mfem->skipEvents = true;
         mfem->run();

         // interpolate model function to the experimental data so dimension 1 in A matches dimension of B:
         mfem->interpolate(experiment, fem_data);

         for (j=0; j<(*experiment).scan.size(); j++)
         {
            for (k=0; k<points; k++)
            {
               (*residuals).scan[j].conc[k] += (*experiment).scan[j].conc[k];
            }
         }

         if(thread == 0)
         {
            progress->setProgress(i+1);
         }
      }

      //    cerr << "thread " << thread << " finished work\n";
      work_mutex.lock();
      work_done = 1;
      work_to_do = 0;
      work_mutex.unlock();
      cond_work_done.wakeOne();
   }
}

float US_FeMatch_W::calc_residuals_ra()
{
   QString str;
   unsigned int i, j, k;
   struct mfem_scan single_scan;
   struct mfem_data solution; //collect all models from multiple threads in this variable

   progress->setTotalSteps(USglobal->config_list.numThreads); // one extra column for the baseline
   progress->reset();
   clear_data(&residuals);
   simdata.clear(); // simdata is a vector since astfem_rsa requires a vector for all speed steps
   simdata.resize(1); // we only have 1 speed step.
   simdata[0].radius.clear();
   solution.radius.clear();
   simdata[0].scan.clear();
   solution.scan.clear();
   single_scan.conc.clear();
   for (i=0; i<points; i++)
   {
      simdata[0].radius.push_back(radius[i]);
      solution.radius.push_back(radius[i]);
      residuals.radius.push_back(radius[i]);
      single_scan.conc.push_back(0.0); // populate with zeros
   }
   for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
   {
      single_scan.time = (double) run_inf.time[selected_cell][selected_lambda][i];
      single_scan.omega_s_t = (double) run_inf.omega_s_t[selected_cell][selected_lambda][i];
      //cout << "i=" << i << ": " << single_scan.time << ", " << single_scan.omega_s_t << endl;
      simdata[0].scan.push_back(single_scan);
      solution.scan.push_back(single_scan);
      residuals.scan.push_back(single_scan);
   }
   US_Data_IO *data_io;
   data_io = new US_Data_IO(&run_inf, false); // (baseline flag can be false, we don't need it)
   data_io->assign_simparams(&sp, selected_cell, selected_lambda, selected_channel);
   delete data_io;
   if (sp.band_forming)
   {
      OneLiner *ol;
      ol = new OneLiner(tr("Please enter a\nband-loading volume\n(in milliliter):"));
      ol->show();
      ol->parameter1->setText(str.sprintf("%5.3f", sp.band_volume));
      if (ol->exec())
      {
         sp.band_volume = ol->string.toFloat();
      }
      delete ol;
   }
   sp.simpoints = 200;
   US_Astfem_RSA *astfem_rsa;
   astfem_rsa = new US_Astfem_RSA(false);
   //US_FemGlobal fg;
   //fg.write_simulationParameters(&sp, "simparams.out");
   for (j=0; j<USglobal->config_list.numThreads; j++)
   {
      //fg.write_experiment(&msv[j], &sp, str.sprintf("/usr/local/ultrascan/develop/%d-model", j));
      for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
      {
         for (k=0; k<points; k++)
         {
            simdata[0].scan[i].conc[k] = 0.0;
         }
      }
      /*
        for (i=0; i<msv[j].component_vector.size(); i++)
        {
        cout << "s: " << msv[j].component_vector[i].s << ", D: " << msv[j].component_vector[i].D << ", c: " << msv[j].component_vector[i].concentration << " (" << msv[j].component_vector[i].name<<")\n";
        }
      */
      if(!ti_noise_avail && sp.band_forming) {
         cout << "band_firstScanIsConcentration = true\n";
         sp.band_firstScanIsConcentration = true;
      } else {
         cout << "band_firstScanIsConcentration = false\n";
         sp.band_firstScanIsConcentration = false;
      }
      astfem_rsa->calculate(&msv[j], &sp, &simdata); // calculate the model for the current thread
      // combine the current model's solution with the total solution vector:
      for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
      {
         for (k=0; k<points; k++)
         {
            solution.scan[i].conc[k] += simdata[0].scan[i].conc[k];
         }
         //cout << "i=" << i << ": " << simdata[0].scan[i].time << ", " << simdata[0].scan[i].omega_s_t << endl;
      }
      progress->setProgress(j+1);
      qApp->processEvents();
   }
   delete astfem_rsa;
   plot_edit();
   long *resids;
   double **res;
   res = new double *[solution.scan.size()];
   resids = new long [solution.scan.size()];
   for (i=0; i<solution.scan.size(); i++)
   {
      res[i] = new double [points];
   }
   for (j=0; j<solution.scan.size(); j++)
   {
      for (k=0; k<points; k++)
      {
         res[j][k] = solution.scan[j].conc[k] + ti_noise[k] + ri_noise[j];
      }
      if (analysis_type == "sa2d")
      {
         str = "2D-Spectrum Analysis Model";
      }
      else if (analysis_type == "cofs")
      {
         str = "C(s) Model";
      }
      else if (analysis_type == "fe")
      {
         str = "Finite Element Model";
      }
      else if (analysis_type == "ga")
      {
         str = "Genetic Algorithm Model";
      }
      else
      {
         str = "2D-Spectrum Analysis Model";
      }
      resids[j] = edit_plot->insertCurve(str);
      edit_plot->setCurvePen(resids[j], QPen(Qt::red, 1, SolidLine));
      edit_plot->setCurveData(resids[j], radius, res[j], points);
   }
   edit_plot->replot();
   clear_data(&fem_model);
   fem_model = solution;
   rmsd = 0.0;
   for (j=0; j<solution.scan.size(); j++)
   {
      for (k=0; k<points; k++)
      {
         residuals.scan[j].conc[k] = absorbance[j][k] - res[j][k];
         rmsd += pow(residuals.scan[j].conc[k], 2.0);
      }
   }
   rmsd /= (points * solution.scan.size());
   str.sprintf("%6.4e", rmsd);
   lbl_variance2->setText(str);
   rmsd = pow((double)rmsd, 0.5);
   str.sprintf("%6.4e", rmsd);
   lbl2_excluded->setText(str);
   second_plot(plot2);
   for (i=0; i<solution.scan.size(); i++)
   {
      delete [] res[i];
   }
   delete [] res;
   delete [] resids;
   pb_second_plot->setEnabled(true);
   pb_save->setEnabled(true);
   pb_view->setEnabled(true);
   pb_print->setEnabled(true);
   return rmsd;
}

float US_FeMatch_W::calc_residuals()
{
   QString str;
   struct mfem_scan single_scan;
   struct mfem_data experiment;
   struct mfem_initial initCVector;
   unsigned int i, j, k, count;
   unsigned threads = USglobal->config_list.numThreads;
   if(threads < 1) {
      threads = 1;
   }
   if (threads > components)
   {
      threads = components;
   }
   vector<struct mfem_data> fem_data(threads);
   vector<US_MovingFEM*> mfem(threads);
   for(i = 0; i < threads; i++)
   {
      mfem[i] = new US_MovingFEM(&(fem_data[i]), false);
   }

   // initialize experimental data array sizes and radius positions:

   clear_data(&experiment);
   clear_data(&residuals);
   progress->setTotalSteps(components); // one extra column for the baseline
   progress->reset();
   initCVector.concentration.clear();
   initCVector.radius.clear();
   double bottom = calc_bottom(rotor_list, cp_list, run_inf.rotor,
                               run_inf.centerpiece[selected_cell], 0, run_inf.rpm[selected_cell][selected_lambda][0]);
   for (i=0; i<points; i++)
   {
      experiment.radius.push_back(radius[i]);
      residuals.radius.push_back(radius[i]);
      single_scan.conc.push_back(0.0); // this is the baseline vector to be added first
   }
   for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
   {
      single_scan.time = (double) run_inf.time[selected_cell][selected_lambda][i];
      experiment.scan.push_back(single_scan);
      residuals.scan.push_back(single_scan);
   }
   count = 0;
   // parallelize this emre
   if(threads > 1)
   {
      // create threads
      //struct mfem_data z_experiment[USglobal->config_list.numThreads];
      vector<struct mfem_data> z_experiment(USglobal->config_list.numThreads);

      //struct mfem_data z_residuals[USglobal->config_list.numThreads];
      vector<struct mfem_data> z_residuals(USglobal->config_list.numThreads);


      for(j = 0; j < threads; j++)
      {
         z_experiment[j] = experiment;
         z_experiment[j].scan = experiment.scan;
         z_residuals[j] = residuals;
         z_residuals[j].scan = residuals.scan;
         //       printf("z_exp.scan %d %lx %lx\n", j, &(z_experiment[j].scan), &(experiment.scan));
      }

      //fematch_thr_t *fematch_thr_threads[USglobal->config_list.numThreads];
      vector<fematch_thr_t*> fematch_thr_threads(USglobal->config_list.numThreads);

      for(j = 0; j < threads; j++)
      {
         fematch_thr_threads[j] = new fematch_thr_t(j);
         fematch_thr_threads[j]->start();
      }

      unsigned int j_inc = components / threads;
      unsigned int j_end;
      unsigned int j_start;
      unsigned int maxprog = 0;
      for(j = 0; j < threads; j++)
      {
         j_start = j_inc * j;
         j_end = (j_inc * (j + 1)) - 1;
         if(j + 1 == threads)
         {
            j_end = components - 1;
         }
         if(!j)
         {
            maxprog = j_end + 3;
            progress->setTotalSteps(maxprog); // add for sum up
            progress->reset();
         }
         //       cout << "thread " << j << " j range " << j_start << " - " << j_end << endl;
         fematch_thr_threads[j]->fematch_thr_setup(progress,
                                                   &fem_data[j],
                                                   &z_experiment[j],
                                                   &z_residuals[j],
                                                   &initCVector,
                                                   mfem[j],
                                                   &run_inf,
                                                   &s_distribution,
                                                   &D_distribution,
                                                   selected_cell,
                                                   selected_lambda,
                                                   &partial_concentration,
                                                   bottom,
                                                   points,
                                                   j_start,
                                                   j_end
                                                   );
      }

      for(j = 0; j < threads; j++)
      {
         qApp->processEvents();
         fematch_thr_threads[j]->fematch_thr_wait();
      }

      // destroy
      for(j = 0; j < threads; j++)
      {
         qApp->processEvents();
         fematch_thr_threads[j]->fematch_thr_shutdown();
      }

      for(j = 0; j < threads; j++)
      {
         qApp->processEvents();
         fematch_thr_threads[j]->wait();
      }

      progress->setProgress(maxprog - 1);
      qApp->processEvents();

      for(i = 0; i < threads; i++)
      {
         for (j=0; j<experiment.scan.size(); j++)
         {
            for (k=0; k<points; k++)
            {
               residuals.scan[j].conc[k] += z_residuals[i].scan[j].conc[k];
            }
         }
      }

      progress->setProgress(maxprog);
      qApp->processEvents();

      for(j = 0; j < USglobal->config_list.numThreads; j++)
      {
         delete fematch_thr_threads[j];
      }

   }
   else
   {
      for (i=0; i<components; i++)
      {
         qApp->processEvents();
         // for each term in the linear combination we need to reset the
         // simulation vectors, the experimental vector simply keeps getting overwritten:
         clear_data(&(fem_data[0]));
         for (j=0; j<experiment.scan.size(); j++)
         {
            for (k=0; k<points; k++)
            {
               // reset concentration to zero:
               experiment.scan[j].conc[k] = 0.0;
            }
         }

         mfem[0]->set_params(100, s_distribution[i], D_distribution[i],
                             (double) run_inf.rpm[selected_cell][selected_lambda][0],
                             experiment.scan[experiment.scan.size()-1].time,
                             (double) run_inf.meniscus[selected_cell], bottom, (double) partial_concentration[i], &initCVector);

         // generate the next term of the linear combination:
         mfem[0]->run();

         // interpolate model function to the experimental data so dimension 1 in A matches dimension of B:
         mfem[0]->interpolate(&experiment, &(fem_data[0]));
         for (j=0; j<experiment.scan.size(); j++)
         {
            for (k=0; k<points; k++)
            {
               residuals.scan[j].conc[k] += experiment.scan[j].conc[k];
            }
         }
         progress->setProgress(i+1);
      }
   }
   qApp->processEvents();
   // subtract ti/ri noise later graphically in additional plots
   /*
     for (j=0; j<experiment.scan.size(); j++)
     {
     for (k=0; k<points; k++)
     {
     residuals.scan[j].conc[k] += ri_noise[j] + ti_noise[k];
     }
     }
   */
   plot_edit();
   long *resids;
   double **res;
   res = new double *[experiment.scan.size()];
   resids = new long [experiment.scan.size()];
   for (i=0; i<experiment.scan.size(); i++)
   {
      res[i] = new double [points];
   }
   for (j=0; j<experiment.scan.size(); j++)
   {
      for (k=0; k<points; k++)
      {
         res[j][k] = residuals.scan[j].conc[k] + ti_noise[k] + ri_noise[j];
      }
      if (analysis_type == "sa2d")
      {
         str = "2D-Spectrum Analysis Model";
      }
      else if (analysis_type == "cofs")
      {
         str = "C(s) Model";
      }
      else if (analysis_type == "fe")
      {
         str = "Finite Element Model";
      }
      else if (analysis_type == "ga")
      {
         str = "Genetic Algorithm Model";
      }
      else
      {
         str = "2D-Spectrum Analysis Model";
      }
      resids[j] = edit_plot->insertCurve(str);
      edit_plot->setCurvePen(resids[j], QPen(Qt::red, 1, SolidLine));
      edit_plot->setCurveData(resids[j], radius, res[j], points);
      //      qApp->processEvents();
   }
   edit_plot->replot();
   clear_data(&fem_model);
   fem_model = residuals;
   rmsd = 0.0;
   for (j=0; j<experiment.scan.size(); j++)
   {
      for (k=0; k<points; k++)
      {
         //         residuals.scan[j].conc[k] = absorbance[j][k] - residuals.scan[j].conc[k];
         residuals.scan[j].conc[k] = absorbance[j][k] - res[j][k];
         //         cout << "C[" << j << "][" << k << "]: " << residuals.scan[j].conc[k] << endl;
         rmsd += residuals.scan[j].conc[k] * residuals.scan[j].conc[k];
      }
   }
   //cout << "RMSD: " << rmsd << ", points: " << points << ", scans: " << experiment.scan.size() << endl;
   rmsd /= (points * experiment.scan.size());
   str.sprintf("%6.4e", rmsd);
   lbl_variance2->setText(str);
   rmsd = pow((double)rmsd, 0.5);
   str.sprintf("%6.4e", rmsd);
   lbl2_excluded->setText(str);
   //   calc_distros();
   second_plot(plot2);
   for (i=0; i<experiment.scan.size(); i++)
   {
      delete [] res[i];
   }
   delete [] res;
   delete [] resids;
   pb_second_plot->setEnabled(true);
   pb_save->setEnabled(true);
   pb_view->setEnabled(true);
   pb_print->setEnabled(true);
   return rmsd;
}

void US_FeMatch_W::calc_distros()
{
   mw.clear();
   s20w.clear();
   D20w.clear();
   for (unsigned int i=0; i<components; i++)
   {
      s20w.push_back(s_distribution[i] * correction);
      D20w.push_back(D_distribution[i] * K20 * viscosity/((100.0 * VISC_20W) * (run_inf.avg_temperature + K0)));
      mw.push_back((fabs(s20w[i])/D20w[i])*(R * K20)/(1.0 - vbar20 * DENS_20W));
   }
}

void US_FeMatch_W::clear_data(mfem_data *d)
{
   unsigned int i;
   for (i=0; i<(*d).scan.size(); i++)
   {
      (*d).scan[i].conc.clear();
   }
   (*d).radius.clear();
   (*d).scan.clear();
}

void US_FeMatch_W::load_model()
{
   QString fn = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*.model.??", 0);
   if ( !fn.isEmpty() )
   {
      load_model(fn);      // the user gave a file name
   }
}

void US_FeMatch_W::load_model(const QString &fileName)
{
   clearDisplay();
   int pos1, pos2;
   pos1 = fileName.findRev("/");
   pos2 = fileName.findRev(".model");
   QString temp_name = fileName.mid(pos1 + 1, pos2 - pos1 - 1);
   float f0, vol, rad_sphere, fval;
   int response;
   QString str;
   QFile f;
   ti_noise.clear();
   ri_noise.clear();
   f.setName(USglobal->config_list.result_dir + "/" + temp_name + str.sprintf(".ri_noise.%d%d", selected_cell+1, selected_lambda+1));
   if (f.exists())
   {
      response = QMessageBox::question(this, "Radially Invariant Noise",
                                       "A radially invariant noise file is available, load it?",
                                       "Yes", "No");
      if (response == 0 && f.open(IO_ReadOnly))
      {
         QTextStream ts(&f);
         for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
         {
            if (!ts.eof())
            {
               ts >> fval;
               ri_noise.push_back((double)fval);
            }
         }
         f.close();
      }
      else
      {
         for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
         {
            ri_noise.push_back(0.0);
         }
      }
   }
   else
   {
      for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
      {
         ri_noise.push_back(0.0);
      }
   }
   ti_noise_avail = false;
   f.setName(USglobal->config_list.result_dir + "/" + temp_name + str.sprintf(".ti_noise.%d%d", selected_cell+1, selected_lambda+1));
   if (f.exists())
   {
      response = QMessageBox::question(this, "Time Invariant Noise",
                                       "A time invariant noise file is available, load it?",
                                       "Yes", "No");
      if (response == 0 && f.open(IO_ReadOnly))
      {
         ti_noise_avail = true;
         QTextStream ts(&f);
         for (i=0; i<points; i++)
         {
            if (!ts.eof())
            {
               ts >> fval;
               ti_noise.push_back((double)fval);
            }
         }
         f.close();
      }
      else
      {
         for (i=0; i<points; i++)
         {
            ti_noise.push_back(0.0);
         }
      }
   }
   else
   {
      for (i=0; i<points; i++)
      {
         ti_noise.push_back(0.0);
      }
   }
   f.setName(fileName);
   unsigned int i;
   s20w.clear();
   D20w.clear();
   fv.clear();
   f_f0.clear();
   mw.clear();
   sigma.clear();
   delta.clear();
   partial_concentration.clear();
   current_component=1;
   if (f.open(IO_ReadOnly))
   {
      QTextStream ts(&f);
      str = ts.readLine();
      str.stripWhiteSpace();
      if (str == "SA2D" || str == "2DSA" || str == "2DSA_RA")
      {
         analysis_type = "sa2d";
         str = ts.readLine();
      }
      else if (str == "SA2D_MW" || str == "2DSA_MW" || str == "2DSA_MW_RA")
      {
         analysis_type = "sa2d_mw";
         str = ts.readLine();
      }
      else if (str == "GA" || str == "GA_RA" )
      {
         analysis_type = "ga";
         str = ts.readLine();
      }
      else if (str == "GA_MW" || str == "GA_MW_RA")
      {
         analysis_type = "ga_mw";
         str = ts.readLine();
      }
      else if (str == "FE")
      {
         analysis_type = "fe";
         str = ts.readLine();
      }
      else if (str == "COFS")
      {
         analysis_type = "cofs";
         str = ts.readLine();
      }
      else if (str == "GA_MC" || str == "GA_RA_MC")
      {
         analysis_type = "ga_mc";
         str = ts.readLine(); // number of Monte Carlo iterations
         monte_carlo_iterations = str.toUInt();
         str = ts.readLine(); // run name
      }
      else if (str == "SA2D_MC" || str == "2DSA_MC" || str == "2DSA_RA_MC")
      {
         analysis_type = "sa2d_mc";
         str = ts.readLine(); // number of Monte Carlo iterations
         monte_carlo_iterations = str.toUInt();
         str = ts.readLine(); // run name
      }
      else if (str == "2DSA_MW_MC" || str == "2DSA_MW_RA_MC")
      {
         analysis_type = "sa2d_mw_mc";
         str = ts.readLine(); // number of Monte Carlo iterations
         monte_carlo_iterations = str.toUInt();
         str = ts.readLine(); // run name
      }
      else if (str == "GA_MW_MC" || str == "GA_MW_RA_MC")
      {
         analysis_type = "ga_mw_mc";
         str = ts.readLine(); // number of Monte Carlo iterations
         monte_carlo_iterations = str.toUInt();
         str = ts.readLine(); // run name
      }
      else
      {
         analysis_type = "sa2d";
      }
      QMessageBox::information(this, tr("UltraScan"), "Loading Distribution for " + str + " Model");
      str = ts.readLine();
      model = str.toInt();
      str = ts.readLine();
      components  = str.toUInt();
      if (components == 0)
      {
         QMessageBox::information(this, tr("UltraScan Error"), "Attention: This model does not contain any "
               "solutes...\nplease check the model file or try another model.");
         return;
      }
      //cout << "Components: " << components << endl;
      for (i=0; i<components; i++)
      {
         str = ts.readLine();
         if (i==0) lbl_C2->setText(str);
         partial_concentration.push_back(str.toDouble());
         str = ts.readLine();
         if (i==0) lbl_s2->setText(str);
         s20w.push_back(str.toDouble());
         str = ts.readLine();
         if (i==0) lbl_D2->setText(str);
         D20w.push_back(str.toDouble());
         str = ts.readLine();
         sigma.push_back(str.toDouble());
         str = ts.readLine();
         delta.push_back(str.toDouble());
         mw.push_back((fabs(s20w[i])/D20w[i])*(R * K20)/(1.0 - vbar20 * DENS_20W));
         vol = vbar20 * mw[i] / AVOGADRO;
         rad_sphere = pow((double) (vol * (3.0/4.0))/M_PI, (double) (1.0/3.0));
         f0 = rad_sphere * 6.0 * M_PI * VISC_20W;
         fv.push_back((mw[i] * (1.0 - vbar20 * DENS_20W)) / (fabs(s20w[i]) * AVOGADRO));
         f_f0.push_back(fv[i]/f0);
         //cout << "MW: " << mw[i] << ", s: " << s20w[i] << ", D: " << D20w[i] << ", k: " << f_f0[i] << endl;
      }

      f.close();
      cnt_component->setRange(1.0, (double) components, 1.0);
      cnt_component->setEnabled(true);
      str.sprintf(" %10.6e kD, %4.2f", mw[0]/1000, f_f0[0]);
      lbl_MW_ff02->setText(str);
      pb_fit->setEnabled(true);
      update_distribution();
      create_modelsystems();
   }
}

void US_FeMatch_W::create_modelsystems()
{
   struct ModelSystem ms;
   msv.clear();
   QString str;
   unsigned int i, j, k, l, threads=USglobal->config_list.numThreads;
   //unsigned int components_per_job[threads];
   unsigned int* components_per_job = (unsigned int*) malloc( threads * sizeof(unsigned int) );

   for (i=0; i<threads; i++)
   {
      components_per_job[i] = (int)(components/threads); // initialize
   }
   j = components_per_job[0] * threads;
   //cout << "j: " << j << ", components/job[0]: " << components_per_job[0] << ", components/job[1]: " << components_per_job[1] << ", components: " << components << endl;
   while (j<components)
   {
      for (i=0; i<threads; i++)
      {
         components_per_job[i]++; // if remainder != zero, pile the remainder on at the end
         //cout << "components/job[" << i << "]: " << components_per_job[i] << ", j: " << j << endl;
         j++;
         if (j == components) break;
      }
   }

   // distribute all components among as many models as there are threads:
   j=0;
   //cout << "threads: " << threads << endl;
   for (i=0; i<threads; i++)
   {
      l=0;
      ms.component_vector.resize(components_per_job[i]);
      //cout << "Components for job " << i << ": " << components_per_job[i] << endl;
      for (k=j; k<j+components_per_job[i]; k++)
      {
         ms.description = str.sprintf("Components for thread %d", i+1);
         ms.model = 3; // fixed molecular weight distribution model with noninteracting species
         ms.component_vector[l].vbar20 = vbar20; // not used
         ms.component_vector[l].mw = mw[k]; // not used
         ms.component_vector[l].s = s_distribution[k]; // populate with s and D values that have
         ms.component_vector[l].D = D_distribution[k]; // been adjusted for experimental space
         ms.component_vector[l].sigma = 0.0; // not used
         ms.component_vector[l].delta = 0.0; // not used
         ms.component_vector[l].extinction = 0.0; // not used
         ms.component_vector[l].concentration = partial_concentration[k];
         ms.component_vector[l].f_f0 = 0.0; // not used
         ms.component_vector[l].show_conc =  false; // not used
         ms.component_vector[l].show_keq = false; // not used
         ms.component_vector[l].show_koff = false; // not used
         ms.component_vector[l].show_stoich = 0; // not used
         ms.component_vector[l].show_component.resize(0); // not used
         ms.component_vector[l].shape = "undefined"; // not used
         ms.component_vector[l].name = str.sprintf("Component %d (of %d)", k+1, components); // not used
         ms.component_vector[l].c0.radius.clear(); // not used
         ms.component_vector[l].c0.concentration.clear(); // not used
         ms.assoc_vector.clear();
         //cout << "s: " << ms.component_vector[l].s << ", D: " << ms.component_vector[l].D << ", c: " << ms.component_vector[l].concentration << " (" << ms.component_vector[l].name<<")\n";
         l++;
      }
      j += components_per_job[i];
      msv.push_back(ms);
   }
   delete [] components_per_job;
}

//void US_FeMatch_W::updateParameters(float val1, float val2)
void US_FeMatch_W::updateParameters()
{
   float vol, rad_sphere, f0;
   for (i=0; i<components; i++)
   {
      mw[i] = (fabs(s20w[i])/D20w[i])*(R * K20)/(1.0 - vbar20 * DENS_20W);
      vol = vbar20 * mw[i] / AVOGADRO;
      rad_sphere = pow((double) (vol * (3.0/4.0))/M_PI, (double) (1.0/3.0));
      f0 = rad_sphere * 6.0 * M_PI * VISC_20W;
      fv[i] = (mw[i] * (1.0 - vbar20 * DENS_20W)) / (fabs(s20w[i]) * AVOGADRO);
      f_f0[i] = fv[i]/f0;
   }
   update_distribution();
   update_labels();
}
