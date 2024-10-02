//! \file us_extinctfitter_gui.cpp

#include "us_extinctfitter_gui.h"
#include "us_gui_util.h"
#if QT_VERSION < 0x050000
#define setSamples(a,b,c) setData(a,b,c)
#define setSymbol(a) setSymbol(*a)
#endif

//using namespace std;

/*
US_ExtinctFitter::US_ExtinctFitter(QVector <struct WavelengthScan> *temp_wls_v, double*& temp_guess, unsigned int& temp_order, unsigned int& temp_parameters, QString& temp_projectName, bool *temp_fitting_widget, bool bufferfit) : US_Minimize(temp_fitting_widget, true)
{
   guess = temp_guess;
   parameters = temp_parameters;
   order = temp_order;
   wls_v = temp_wls_v;
   runs = 0;
   runs_percent = 0;

   //pb_pause->hide();                // Test - edit what to show/hide, depending on the General/Buffer/Analyte Global Fit
   //pb_resume->hide();
   
   projectName = temp_projectName;
	connect(pb_overlays, SIGNAL(clicked()), SLOT(plot_overlays()));
   connect(pb_residuals, SIGNAL(clicked()), SLOT(plot_residuals()));
}
*/

US_ExtinctFitter::US_ExtinctFitter(QVector <struct WavelengthScan> *temp_wls_v, double*& temp_guess, unsigned int& temp_order, unsigned int& temp_parameters, QString& temp_projectName,  bool *temp_fitting_widget) : US_Minimize(temp_fitting_widget, true)
{
   guess = temp_guess;
   parameters = temp_parameters;
   order = temp_order;
   wls_v = temp_wls_v;
   runs = 0;
   runs_percent = 0;
   
   us_auto_mode = false;
     
   projectName = temp_projectName;
	connect(pb_overlays, SIGNAL(clicked()), SLOT(plot_overlays()));
   connect(pb_residuals, SIGNAL(clicked()), SLOT(plot_residuals()));
}

US_ExtinctFitter::US_ExtinctFitter(QVector <struct WavelengthScan> *temp_wls_v, double*& temp_guess, unsigned int& temp_order, unsigned int& temp_parameters, QString& temp_projectName,  bool *temp_fitting_widget, bool auto_mode ) : US_Minimize(temp_fitting_widget, true)
{
   guess = temp_guess;
   parameters = temp_parameters;
   order = temp_order;
   wls_v = temp_wls_v;
   runs = 0;
   runs_percent = 0;

   us_auto_mode = true;

     
   projectName = temp_projectName;
	connect(pb_overlays, SIGNAL(clicked()), SLOT(plot_overlays()));
   connect(pb_residuals, SIGNAL(clicked()), SLOT(plot_residuals()));
}

void US_ExtinctFitter::saveFit()
{
	QMessageBox message;
	message.setWindowTitle(tr("Extinction Fitter:"));
	message.setText(tr("Please use the save function from\nthe main extinction calculation\nmodule. It will allow you to save the ]ncoefficients of the fit as well as a\ncopy of the plot window."));
	message.exec();
}
	
void US_ExtinctFitter::startFit()
{
  if (autoconverge)
  {
    iteration = 1000;
    while (iteration > 1) 
    {
      return_value = Fit(); // don't bother checking return values...
      if (nlsMethod == 3)   // switch between Levenberg Marquardt and Quasi Newton
      {
         nlsMethod = 0;
         if (GUI)
         {
            cbb_nlsMethod->setCurrentIndex(0);
         }
       }
       else
       {
         nlsMethod = 3;
         if (GUI)
         {
				cbb_nlsMethod->setCurrentIndex(3);
         }
       }
     }
   }
   else // just do a single fit
   {
     return_value = Fit();
     if (return_value != 0)
     {
		 QString str;
     	 if (GUI)
       {
         QMessageBox message;
			message.setWindowTitle(tr("Extinction Fitter:"));
			//message.setText(tr("The program exited with a return\nvalue of" + str.sprintf("%d.", return_value)));
       }
		}
	}
}

bool US_ExtinctFitter::fit_init()
{

   int i, point_counter;
   //
   // Calculate how many points there are in each dataset, sum them up for "points" and
   // leave out datasets that aren't fitted to keep the vectors/matrices as small as possible.
   //
   points = 0;
   datasets = (*wls_v).size();

   for (i=0; i<(*wls_v).size(); i++)
   {
      points_per_dataset.push_back((*wls_v).at(i).v_readings.size());
      points += (*wls_v).at(i).v_readings.size();
   }
   if ((*wls_v).size() == 0)
   {
      return(false);
   }
   iteration = 0;

   y_raw                  = new float    [points];      // experimental data (absorbance)
   y_guess               = new double   [points];      // simulated solution
   y_delta               = new double   [points];      // y_raw - y_guess
   B                     = new double   [parameters];
   test_guess            = new double   [parameters];
   jacobian               = new double  *[points];
   information_matrix   = new double  *[parameters];
   LL_transpose         = new double  *[parameters];

   for (unsigned int i=0; i<points; i++)
   {
      jacobian[i] = new double [parameters];
   }
   for (uint i=0; i<parameters; i++)
   {
      information_matrix[i]   = new double [parameters];
      LL_transpose[i]         = new double [parameters];
   }

 // initialize y_raw:
   point_counter = 0;
   for (i=0; i<(*wls_v).size(); i++)
   {
      for (unsigned int j=0; j<points_per_dataset[i]; j++)
      {
         y_raw[point_counter] = (*wls_v).at(i).v_readings.at(j).od;
         point_counter++;
      }
   }

   return(true);
}

int US_ExtinctFitter::calc_model(double *guess_par)
{
	QString str;
   unsigned int j, k, point_counter=0;
   float gaussian;
   for (int i=0; i<(*wls_v).size(); i++)
   {
      for (j=0; j<points_per_dataset[i]; j++)
      {
         gaussian = 0.0;
         for (k=0; k<order; k++)
         {
            gaussian += exp(guess_par[(*wls_v).size() + (3 * k)]
                            - (pow(((*wls_v).at(i).v_readings.at(j).lambda - guess_par[(*wls_v).size() + (3 * k) + 1]), 2)
                               / ( 2 * pow(guess_par[(*wls_v).size() + (3 * k) + 2], 2))));
         }
         y_guess[point_counter] = guess_par[i] * gaussian;
         point_counter++;
      }
   }
   function_evaluations++;
   le_evaluations->setText(str.sprintf(" %d", function_evaluations));
   qApp->processEvents();
   if (aborted)
   {
      return(-1);
   }
   return(0);

   /* //Polynomial fit:
      QString str;
      unsigned int i, j, k, point_counter=0;
      float polynomial;
      for (i=0; i<(*wls_v).size(); i++)
      {
      for (j=0; j<points_per_dataset[i]; j++)
      {
      for (k=0; k<order; k++)
      {
      polynomial += guess_par[(*wls_v).size() + k] * pow((*wls_v)[i].lambda[j], k);
      }
      y_guess[point_counter] = guess_par[i] * polynomial;
      //qDebug() << "dataset: " << i << ", point: " << j << ", guessed y-value: " << y_guess[point_counter]  ;
      point_counter++;
      }
      }
      function_evaluations++;
      lbl_evaluations2->setText(str.sprintf(" %d", function_evaluations));
      qApp->processEvents();
      if (aborted)
      {
      return(-1);
      }  
   return(0);
*/
}

int US_ExtinctFitter::calc_jacobian()
{

   unsigned int i, j, k, point_counter=0;
   float *term;
   term = new float [order];
   for (i=0; i<points; i++)
   {
      for (j=0; j<parameters; j++)
      {
         jacobian[i][j] = 0.0;
      }
   }
   for (int i=0; i<(*wls_v).size(); i++)
   {
      for (j=0; j<points_per_dataset[i]; j++)
      {
         for (k=0; k<order; k++)
         {
            term[k] = exp(guess[(*wls_v).size() + (3 * k)]
                          - (pow(((*wls_v).at(i).v_readings.at(j).lambda - guess[(*wls_v).size() + (3 * k) + 1]), 2)
                             / ( 2 * pow(guess[(*wls_v).size() + (3 * k) + 2], 2))));
         }
         jacobian[point_counter][i] = 0.0;
         for (k=0; k<order; k++)
         {
            jacobian[point_counter][i] += term[k];

            jacobian[point_counter][(*wls_v).size() + (3 * k)]     = guess[i] * term[k];

            jacobian[point_counter][(*wls_v).size() + (3 * k) + 1] = guess[i] * term[k]
               * (((*wls_v).at(i).v_readings.at(j).lambda - guess[(*wls_v).size() + (3 * k) + 1])
                  / pow(guess[(*wls_v).size() + (3 * k) + 2], 2));

            jacobian[point_counter][(*wls_v).size() + (3 * k) + 2] = guess[i] * term[k]
               * (pow(guess[(*wls_v).size() + (3 * k) + 2], -3)
                  * (pow(guess[(*wls_v).size() + (3 * k) + 1], 2)
                     - 2 * guess[(*wls_v).size() + (3 * k) + 1]
                     * (*wls_v).at(i).v_readings.at(j).lambda + pow((*wls_v).at(i).v_readings.at(j).lambda, 2)));
         }
         point_counter++;
      }
   }
   qApp->processEvents();
   if (aborted)
   {
      return(-1);
   }
   return(0);

   /* Jacobian for polynomial fit:
      unsigned int i, j, k, point_counter=0;
      float polynomial;
      for (i=0; i<points; i++)
      {
      for (j=0; j<parameters; j++)
      {
      jacobian[i][j] = 0.0;
      }
      }
      // y[i][j] = guess[i] * (a[0] + a[1] * x[i][j] + a[2] * x[i][j]^2 + ... + a[n] * x[i][j]^n)
      for (i=0; i<(*wls_v).size(); i++)
      {
      for (j=0; j<points_per_dataset[i]; j++)
      {
      polynomial = 0.0;
      for (k=0; k<order; k++)
      {
      polynomial += guess[(*wls_v).size() + k] * pow((*wls_v)[i].lambda[j], k);
      }
      // dy/dguess[i] = (a[0] + a[1] * x[i][j] + a[2] * x[i][j]^2 + ... + a[n] * x[i][j]^n)
      jacobian[point_counter][i] = polynomial;
      // dy/da[n] = guess[i] * x[i][j]^n
      jacobian[point_counter][(*wls_v).size()] = guess[i];
      for (k=1; k<order; k++)
      {
      jacobian[point_counter][(*wls_v).size() + k] = guess[i] * pow((*wls_v)[i].lambda[j], k);
      }
      point_counter++;
      }
      }
      qApp->processEvents();
      if (aborted)
      return(-1);
      }
   return(0);
*/

   delete [] term;
}

void US_ExtinctFitter::cleanup()
{
   bt_plotGroup->setEnabled(false);

   unsigned int i;
   delete [] y_raw;
   delete [] y_guess;
   delete [] y_delta;
   delete [] B;
   delete [] test_guess;

   for (i=0; i<points; i++)
   {
      delete [] jacobian[i];
   }
   for (i=0; i<parameters; i++)
   {
      delete [] information_matrix[i];
      delete [] LL_transpose[i];
   }
   delete [] jacobian;
   delete [] information_matrix;
	delete [] LL_transpose;
}

void US_ExtinctFitter::view_report()
{
/*
   QString fileName, str;
   fileName = USglobal->config_list.result_dir + "/" + projectName + "-extinction.res";
   //view_file(fileName);
   TextEdit *e;
   e = new TextEdit();
   e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
   e->load(fileName);
   e->show();
*/
}

void US_ExtinctFitter::write_report()
{
}

void US_ExtinctFitter::plot_overlays()
{
  //qDebug() << "Starting plot overlay: "  ;
   unsigned int numScans = 0;
	QVector <QwtPlotCurve*> v_curve_raw, v_curve_fit;
   long unsigned int point_counter = 0;
   plotResiduals = false;
   QString s1, s2;
	QVector <QVector<double> > v_all_xplot;
	QVector <QVector<double> > v_all_yplot_fit;
	QVector <QVector<double> > v_all_yplot_raw;
   QVector <double> v_xplot;
   QVector <double> v_yplot_fit;
   QVector <double> v_yplot_raw;
	v_all_xplot.clear();
	v_all_yplot_fit.clear();
	v_all_yplot_raw.clear();
   v_curve_fit.clear();
   v_curve_raw.clear();
  	if (plotGroup)
   {
      if (datasets - firstScan == 0)
      {
         numScans = 1;
         s1.sprintf((tr("Overlays for fitted Scan %ld")).toLatin1().data(), firstScan);
      }
      else if (datasets - firstScan < 5)
      {
         numScans = datasets - firstScan + 1;
         s1.sprintf((tr("Overlays for fitted Scans %ld - %ld")).toLatin1().data(), firstScan, firstScan + numScans - 1);
      }
      else
      {
         numScans = 5;
         s1.sprintf((tr("Overlays for fitted Scans %ld - %ld")).toLatin1().data(),  firstScan, firstScan+4);
      }
   }
   else
   {
      s1 = tr("Overlays");
   }
	
	//	qDebug() << "Starting plot overlay 1: "  ;

   //s2.sprintf((tr("Optical Density")).toLatin1().data());
   s2 = tr("Optical Density");
   point_counter = 0;
   for (int i=0; i<(*wls_v).size(); i++)
   {
		v_xplot.clear();
      v_yplot_fit.clear();
      v_yplot_raw.clear();
      for (unsigned int j=0; j<points_per_dataset[i]; j++)
      {
         v_xplot.push_back((*wls_v).at(i).v_readings.at(j).lambda);
         v_yplot_fit.push_back(y_guess[point_counter]);
         v_yplot_raw.push_back(y_raw[point_counter]);
         point_counter++;
      }
		v_all_xplot.push_back(v_xplot);
		v_all_yplot_fit.push_back(v_yplot_fit);
		v_all_yplot_raw.push_back(v_yplot_raw);
   }
   
   //emit get_yfit( v_all_xplot, v_all_yplot_fit );   // Send fitting data
   
   //qDebug() << "Starting plot overlay 2: "  ; 

   //QwtSymbol* symbol = new QwtSymbol(QwtSymbol::Diamond,QBrush(Qt::red),QPen(Qt::red),QSize(5,5));
   
   QPen p;
   p.setColor(Qt::red);
   p.setWidth(2);
 
   //QwtSymbol* symbol = new QwtSymbol;
   //symbol->setSize(10);
   //symbol->setPen(QPen(Qt::blue));
   //symbol->setBrush(Qt::yellow);
   //symbol->setStyle(QwtSymbol::Ellipse);
   
   //qDebug() << "Starting plot overlay 2a: "  ;
   dataPlotClear( data_plot );
      
   us_grid(data_plot);
   
   //qDebug() << "Starting plot overlay 2b: "  ;

   data_plot->setTitle(s1);

   //qDebug() << "Starting plot overlay 2c: "  ;

   data_plot->setAxisTitle(QwtPlot::xBottom, tr("Wavelength (nm)"));

   //qDebug() << "Starting plot overlay 2d: "  ;

   data_plot->setAxisTitle(QwtPlot::yLeft, s2);

   //qDebug() << "Starting plot overlay 3: "  ;

   //qDebug() << "PlotGroup: " << plotGroup  ;

   if (plotGroup)
   {
      for (int i = 0; i < v_all_xplot.size(); i++)
      {
	QwtSymbol* symbol = new QwtSymbol;
	symbol->setSize(10);
	symbol->setPen(QPen(Qt::blue));
	symbol->setBrush(Qt::yellow);
	symbol->setStyle(QwtSymbol::Ellipse);

        QwtPlotCurve *c_raw;
	c_raw = us_curve(data_plot, "raw data");
	QwtPlotCurve *c_fit;
	c_fit  = us_curve(data_plot, "fitted data");
	double* xx = (double*)v_all_xplot.at(i).data();
	double* yy_raw = (double*)v_all_yplot_raw.at(i).data();
	double* yy = (double*)v_all_yplot_fit.at(i).data();
	int     nn = v_all_xplot.at(i).size();
	c_raw->setStyle(QwtPlotCurve::NoCurve);
	//c_raw->setStyle(QwtPlotCurve::Lines);
	
	c_raw->setSymbol(symbol);    
	c_raw->setSamples( xx, yy_raw, nn );
	c_fit->setStyle(QwtPlotCurve::Lines);
	c_fit->setPen(p);
	c_fit->setSamples( xx, yy, nn );
	//v_curve_raw.push_back(c_raw);
	//v_curve_fit.push_back(c_fit);
      }
   }
   else
   {
      for (int i=0; i < v_all_xplot.size(); i++)
      {
	QwtSymbol* symbol = new QwtSymbol;
	symbol->setSize(10);
	symbol->setPen(QPen(Qt::blue));
	symbol->setBrush(Qt::yellow);
	symbol->setStyle(QwtSymbol::Ellipse);

        QwtPlotCurve *c_raw;
	c_raw = us_curve(data_plot, "raw data");
	QwtPlotCurve *c_fit;
	c_fit  = us_curve(data_plot, "fitted data");
	double* xx = (double*)v_all_xplot.at(i).data();
	double* yy_raw = (double*)v_all_yplot_raw.at(i).data();
	double* yy = (double*)v_all_yplot_fit.at(i).data();
	int     nn = v_all_xplot.at(i).size();
	c_raw->setStyle(QwtPlotCurve::NoCurve);
	//c_raw->setStyle(QwtPlotCurve::Lines);
	
	c_raw->setSymbol(symbol);    
	c_raw->setSamples( xx, yy_raw, nn );
	c_fit->setStyle(QwtPlotCurve::Lines);
	c_fit->setPen(p);
	c_fit->setSamples( xx, yy, nn );
	//v_curve_raw.push_back(c_raw);
	//v_curve_fit.push_back(c_fit);
      }
   }
   //qDebug() << "Ending plot overlay:";
   //   data_plot->setAxisScale(QwtPlot::xBottom, -xmax/30.0, xmax + xmax/30.0, 0);
   data_plot->replot();
  // data_plot->updatePlot();      //no updatePlot() in new version
   pb_print->setEnabled(true);

   qDebug() << "EMITTING get_yfit SIGNAL!!";
   emit get_yfit( v_all_xplot, v_all_yplot_fit );   // Send fitting data
   emit get_variance ( le_variance->text().toDouble()  );
}

void US_ExtinctFitter::plot_residuals()
{
   double **xplot = 0, **yplot_res = 0, line_x[2], line_y[2];
   unsigned int numScans = 0, l;
   unsigned long point_counter = 0;
   QString s1, s2, s3;
   float offset = 0;
   double xmax = -1e6, xmin = 1e6;
   int last_sign = 0, current_sign = 0;
   plotResiduals = true;
   xplot = new double* [datasets];
   yplot_res = new double* [datasets];
   QVector <QwtPlotCurve*> v_curve_res, v_zeroLine;
	v_curve_res.clear();
	v_zeroLine.clear(); 
   if (plotGroup)
   {
      if (datasets - firstScan == 0)
      {
         numScans = 1;
         s1.sprintf((tr("Residuals from fitted Scan %ld")).toLatin1().data(), firstScan);
     }
      else if (datasets - firstScan < 5)
      {
         numScans = datasets - firstScan + 1;
         s1.sprintf((tr("Residuals from fitted Scans %ld - %ld")).toLatin1().data(), firstScan, firstScan + numScans - 1);
      }
      else
      {
         numScans = 5;
         s1.sprintf((tr("Residuals from fitted Scans %ld - %ld")).toLatin1().data(),  firstScan, firstScan+4);
      }
   }
   else
   {
      s1 = tr("Residuals");
   }
   //s2.sprintf((tr("Optical Density Difference\n")).toLatin1().data());
   s2 = tr("Optical Density Difference\n");
   point_counter = 0;
   for (int i=0; i<(*wls_v).size(); i++)
   {
      (*wls_v)[i].pos = 0;
      (*wls_v)[i].neg = 0;
      (*wls_v)[i].runs = 0;
      xplot[i] = new double [points_per_dataset[i]];
      yplot_res[i] = new double [points_per_dataset[i]];
      for (unsigned int j=0; j<points_per_dataset[i]; j++)
      {
         xplot[i][j] = (*wls_v).at(i).v_readings.at(j).lambda;
         yplot_res[i][j] = y_guess[point_counter] - y_raw[point_counter];
         if (yplot_res[i][j] > 0)
         {
            (*wls_v)[i].pos++;
            current_sign = 1;
         }
         else
         {
            (*wls_v)[i].neg++;
            current_sign = -1;
         }
         if (j == 0)
         {
            if (yplot_res[i][j] > 0)
            {
               last_sign = 1;
            }
            else
            {
               last_sign = -1;
            }
         }
         else
         {
            if (last_sign != current_sign)
            {
               (*wls_v)[i].runs++;
               last_sign = current_sign;
            }
         }
         point_counter++;
      }
      xmax = max(xplot[i][points_per_dataset[i] - 1], xmax);
      xmin = min(xplot[i][0], xmin);
   }
   //QwtSymbol* symbol = new QwtSymbol;
   QPen p_raw, p_zero;
   p_raw.setColor(Qt::green);
   p_raw.setWidth(1);
   p_zero.setColor(Qt::red);
   p_zero.setWidth(2);
   //symbol->setPen(QPen(Qt::blue));
   //symbol->setBrush(QBrush(Qt::yellow));
   //symbol->setStyle(QwtSymbol::Ellipse);

   qDebug() << "Plot_residuals: Detaching Items..: "  ;
   dataPlotClear( data_plot );
   qDebug() << "Plot_residuals: DETACHED.. "  ;
   
   us_grid(data_plot);
   
   data_plot->setTitle(s1);
   data_plot->setAxisTitle(QwtPlot::xBottom, tr("Wavelength (nm)"));
   data_plot->setAxisTitle(QwtPlot::yLeft, s2);
   line_y[0] = 0;
   line_y[1] = 0;
   l = 0;
   if (plotGroup)
   {
    
      line_x[0] = xmin - 2;
      line_x[1] = xmax + 2;
      for (unsigned int i = firstScan - 1; i< numScans + firstScan - 1; i++)
      {
	
	QwtSymbol* symbol = new QwtSymbol;
	symbol->setPen(QPen(Qt::blue));
	symbol->setBrush(QBrush(Qt::yellow));
	symbol->setStyle(QwtSymbol::Ellipse);
	symbol->setSize(8);

	QwtPlotCurve* c;
	c = us_curve(data_plot, "residuals");
         c->setStyle(QwtPlotCurve::Lines);
         if (i != firstScan - 1)
         {
            for (unsigned int j=0; j<points_per_dataset[i]; j++)
            {
               yplot_res[i][j] += offset;
            }
            line_y[0] = offset;
            line_y[1] = offset;
         }
         c->setSamples(xplot[i], yplot_res[i], points_per_dataset[i]);
         c->setSymbol(symbol);
         c->setPen(p_raw);
			v_curve_res.push_back(c);
			QwtPlotCurve* c_two;
			c_two = us_curve(data_plot, "Zero Line");
         c_two->setSamples(line_x, line_y, 2);
         c_two->setStyle(QwtPlotCurve::Lines);
         c_two->setPen(p_zero);
			v_zeroLine.push_back(c_two);
         offset += (float) 0.03;
         l++;
      }
      data_plot->setAxisScale(QwtPlot::xBottom, xmin - 10, xmax + 15, 0);
   }
   else
   {

     
     line_x[0] = xmin - 2;
      line_x[1] = xmax + 2;
      for (unsigned int i=0; i<datasets; i++)
      {
	QwtSymbol* symbol = new QwtSymbol;
	symbol->setPen(QPen(Qt::blue));
	symbol->setBrush(QBrush(Qt::yellow));
	symbol->setStyle(QwtSymbol::Ellipse);
	symbol->setSize(5);
	
	QwtPlotCurve* c;
         c = us_curve(data_plot, "residuals");
         c->setStyle(QwtPlotCurve::Lines);
         c->setSamples(xplot[i], yplot_res[i], points_per_dataset[i]);
         c->setSymbol(symbol);
         c->setPen(p_raw);
      }
		QwtPlotCurve* c_two;
      c_two = us_curve(data_plot, "Zero Line");
      c_two->setSamples(line_x, line_y, 2);
      c_two->setStyle(QwtPlotCurve::Lines);
      c_two->setPen(p_zero);
      data_plot->setAxisScale(QwtPlot::xBottom, xmin - 10, xmax + 10, 0);
   }
   data_plot->replot();
   //data_plot->updatePlot();         //no updatePlot() in new version
   int i, total_pos = 0, total_neg = 0;
   for (i=0; i<(*wls_v).size(); i++)
   {
      total_pos += (*wls_v)[i].pos;
      total_neg += (*wls_v)[i].neg;
      runs += (*wls_v)[i].runs;
   }
   runs_percent = (float) (total_pos + total_neg);
   runs_percent = (float) runs / runs_percent;
   runs_percent *= 100.0;
   update_fitDialog();
   for (unsigned int i=0; i<datasets; i++)
   {
      delete [] xplot[i];
      delete [] yplot_res[i];
   }
   delete [] xplot;
   delete [] yplot_res;
   write_report();
}

void US_ExtinctFitter::updateRange(double scan)
{

   firstScan = (int) scan;

   if (firstScan > datasets)
   {
      firstScan = 1;
   }
   /*unsigned int modulus = datasets % 5;
   switch (modulus)
   {
   case 0:
      {
         cnt_scan->setRange(1, datasets-4, 5);
         break;
      }
   case 1:
      {
         cnt_scan->setRange(1, datasets, 5);
         break;
      }
   case 2:
      {
         cnt_scan->setRange(1, datasets-1, 5);
         break;
      }
   case 3:
      {
         cnt_scan->setRange(1, datasets-2, 5);
         break;
      }
   case 4:
      {
         cnt_scan->setRange(1, datasets-3, 5);
         break;
      }
   }
   cnt_scan->setValue(firstScan);*/
   if (plotResiduals)
   {
      plot_residuals();
   }
   else
   {
      plot_overlays();
	}
}

void US_ExtinctFitter::endFit()
{
   emit currentStatus("Converged");
   emit hasConverged();
   converged = true;
   pb_close->setText(tr("Close"));
   pb_fit->setEnabled(true);
   //lbl_status2->setText(tr("The fitting process converged..."));
   plot_residuals();
   pgb_progress->setValue(totalSteps);
   //pgb_progress->setValue(100);
   pb_residuals->setEnabled(true);
   pb_overlays->setEnabled(true);
   pb_saveFit->setEnabled(true);
   qDebug() << "EndFit in US_extinctfitter "  ;
}
