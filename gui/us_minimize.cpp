//! \file us_minimize.cpp

#include "us_minimize.h"
//#include <iostream>
#include <cerrno> //for floating point errors
//#include <cfloat>


#define FLT_MIN  1.17549e-38
#define FLT_MAX  3.40282e+38

//using namespace std;

//Constructor

US_Minimize::US_Minimize(bool *temp_fitting_widget, bool temp_GUI) : US_Widgets()
{
   GUI = temp_GUI;
	fitting_widget = temp_fitting_widget;
   suspend_flag = false;
   converged = false;
   aborted = false;
   completed = false;
	plotGroup = 0;
   plotResiduals = false;
   autoconverge = false;
	maxIterations = 50000;
	suspend_flag = false;
	lambdaStart = 20.0;
   lambdaStep = 1.05;
   tolerance =  (float) 1.0e-12;
   constrained = false;
	nlsMethod = 0;
   showGuiFit = false; // show fitting process graphically
	if(GUI)
	{
		showGuiFit = true;
		*fitting_widget = true; //widget is on screen now
		setup_GUI();
	}
}
US_Minimize::~US_Minimize()
{
  //*fitting_widget = false;
}
void US_Minimize::setup_GUI()
{
	QGridLayout* gl1;
	gl1 = new QGridLayout();
	lbl_iteration = us_label(tr("Iteration Number"));
	gl1->addWidget(lbl_iteration, 0, 0);
	le_iteration = us_lineedit("0", 1, false);
	gl1->addWidget(le_iteration, 0, 1);
	pb_fit = us_pushbutton(tr("Fit"));
	connect(pb_fit, SIGNAL(clicked()), SLOT(Fit()));
	gl1->addWidget(pb_fit, 0, 2);
	lbl_variance = us_label(tr("Variance"));
	gl1->addWidget(lbl_variance, 1, 0);
  	le_variance = us_lineedit("0", 1, false);
	gl1->addWidget(le_variance, 1, 1);
	pb_pause = us_pushbutton(tr("Pause"),false);
	gl1->addWidget(pb_pause, 1, 2);
   lbl_stddev = us_label(tr("Std. Deviation:"));
	gl1->addWidget(lbl_stddev, 2, 0);
   le_stddev = us_lineedit("0", 1, false);
	gl1->addWidget(le_stddev, 2, 1);
	pb_resume = us_pushbutton(tr("Resume"),false);
	gl1->addWidget(pb_resume, 2, 2);
	lbl_improvement = us_label(tr("Improvement"));
	gl1->addWidget(lbl_improvement, 3, 0);
   le_improvement = us_lineedit("0", 1, false);
	gl1->addWidget(le_improvement, 3, 1);
	pb_saveFit = us_pushbutton(tr("Save Fit"),false);
	connect(pb_saveFit, SIGNAL(clicked()), SLOT(save_Fit()));
	gl1->addWidget(pb_saveFit, 3, 2);
   lbl_evaluations = us_label(tr("Function Evaluations"));
   gl1->addWidget(lbl_evaluations, 4, 0);
   le_evaluations = us_lineedit("0", 1, false);
	gl1->addWidget(le_evaluations, 4, 1);
	pb_report = us_pushbutton(tr("View Report"),false);
	gl1->addWidget(pb_report, 4, 2);
   lbl_decompositions = us_label(tr("Decompositions"));
	gl1->addWidget(lbl_decompositions, 5, 0);
   le_decompositions = us_lineedit("0", 1, false);
	gl1->addWidget(le_decompositions, 5, 1);
	pb_residuals = us_pushbutton(tr("Residuals"), false);
 	gl1->addWidget(pb_residuals, 5, 2);
   lbl_currentLambda = us_label(tr("Current Lambda:"));
	gl1->addWidget(lbl_currentLambda, 6, 0);
   le_currentLambda = us_lineedit("1.0e+05", 1, false);
	gl1->addWidget(le_currentLambda, 6, 1);
	pb_overlays = us_pushbutton(tr("Overlays"),false);
	gl1->addWidget(pb_overlays, 6, 2);
   lbl_parameters = us_label(tr("# of Parameters:"));
	gl1->addWidget(lbl_parameters, 7, 0);
   le_parameters = us_lineedit("0", 1, false);
	gl1->addWidget(le_parameters, 7, 1);
	pb_print = us_pushbutton(tr("Print"),false);
	gl1->addWidget(pb_print, 7, 2);
   lbl_datasets = us_label(tr("# of Datasets in Fit:"));
	gl1->addWidget(lbl_datasets, 8, 0);
   le_datasets = us_lineedit("0", 1, false);
	gl1->addWidget(le_datasets, 8, 1);
   pb_help = us_pushbutton(tr("Help"));
	gl1->addWidget(pb_help, 8, 2);
	lbl_points = us_label(tr("# of Datapoints in Fit:"));
	gl1->addWidget(lbl_points, 9, 0);
   le_points = us_lineedit("0", 1, false);
	gl1->addWidget(le_points, 9, 1);
	pb_close = us_pushbutton("Close");
	connect(pb_close, SIGNAL(clicked()), SLOT(close()));
	gl1->addWidget(pb_close, 9, 2);

	data_plot = new QwtPlot();
	plotLayout = new US_Plot(data_plot, tr(""), tr(""), tr(""));
	data_plot->setCanvasBackground(Qt::black);	
	data_plot->setMinimumSize(560, 240);

	lbl_controls1 = us_banner(tr("Graph Plotting Controls"));
	QGridLayout* gl2;
	gl2 = new QGridLayout();
	QGridLayout* monitor_layout = us_checkbox(tr("Monitor Fit Graphically"), ck_monitor, false);
	bg1= new QButtonGroup();
	QGridLayout* plot1_layout = us_radiobutton(tr("Plot All Data"), bt_plotAll);
	QGridLayout* plot2_layout = us_radiobutton(tr("Plot Groups of 5"), bt_plotGroup);
	QGridLayout* plot3_layout = us_radiobutton(tr("Plot Single Scans"), bt_plotSingle);
	bg1->addButton(bt_plotAll);
	bg1->addButton(bt_plotGroup);
	bg1->addButton(bt_plotSingle);
	ct_unlabeled = us_counter(3, 1, 50, 1);
	gl2->addLayout(plot1_layout, 0, 0);
	gl2->addLayout(monitor_layout, 0, 1);
	gl2->addLayout(plot2_layout, 1, 0);
	gl2->addWidget(ct_unlabeled, 1, 1);
	gl2->addLayout(plot3_layout, 2, 0);
	lbl_controls2 = us_banner(tr("NLSQ Fit Tuning Controls"));
 	QGridLayout* gl3;
	gl3 = new QGridLayout();
	lbl_constrained = us_label(tr("Linear Constraints:"));
	gl3->addWidget(lbl_constrained, 0, 0);
	QGridLayout* glrb1;
	glrb1 = new QGridLayout();
	QGridLayout* linearN_layout = us_radiobutton(tr("No"), rb_linearN);
	bg2 = new QButtonGroup();
	glrb1->addLayout(linearN_layout, 0, 0);
   QGridLayout* linearY_layout = us_radiobutton(tr("Yes"), rb_linearY);
	glrb1->addLayout(linearY_layout, 0, 1);
	gl3->addLayout(glrb1, 0, 1);
	bg2->addButton(rb_linearY);
	bg2->addButton(rb_linearN);
	lbl_autoconverge = us_label(tr("Autoconverge:"));
	gl3->addWidget(lbl_autoconverge, 1, 0);
	bg3 = new QButtonGroup();
	QGridLayout* glrb2;
	glrb2 = new QGridLayout();
	QGridLayout* autoN_layout = us_radiobutton(tr("No"), rb_autoN);
	glrb2->addLayout(autoN_layout, 0, 0);
	QGridLayout* autoY_layout = us_radiobutton(tr("Yes"), rb_autoY);
	glrb2->addLayout(autoY_layout, 0, 1);
	bg3->addButton(rb_autoN);
	bg3->addButton(rb_autoY);
	gl3->addLayout(glrb2, 1, 1);
	lbl_nlsMethod = us_label(tr("NLS Algorithm:"));
	gl3->addWidget(lbl_nlsMethod, 2, 0);
	cb_nlsalg = new QComboBox();
	cb_nlsalg->addItem(QString("Levenberg Marquardt"));
	cb_nlsalg->addItem(QString("Modified Gauss-Newton"));
	cb_nlsalg->addItem(QString("Hybrid Method"));
	cb_nlsalg->addItem(QString("Quasi-Newton Method"));
	cb_nlsalg->addItem(QString("Generalized Linear LS"));
	cb_nlsalg->addItem(QString("Nonnegative constrained LS"));
	gl3->addWidget(cb_nlsalg, 2, 1);
	lbl_lambdaStart = us_label(tr("Lambda Start:"));
	gl3->addWidget(lbl_lambdaStart, 3, 0);
   le_lambdaStart = us_lineedit("20.0", 1, false);
   connect(le_lambdaStart, SIGNAL(textChanged(const QString&)), SLOT(update_lambdaStart(const QString &)));
	gl3->addWidget(le_lambdaStart, 3, 1);
	pb_plottwo = us_pushbutton(tr(""));
	gl3->addWidget(pb_plottwo, 3, 2);	
	lbl_lambdaStep = us_label(tr("Lambda Step Size:"));
	gl3->addWidget(lbl_lambdaStep, 4, 0);
	//le_lambdaStep = us_lineedit("1.0000e+01", 1, false);
	le_lambdaStep = us_lineedit("1.050", 1, false);
   connect(le_lambdaStep, SIGNAL(textChanged(const QString&)), SLOT(update_lambdaStep(const QString &)));
	gl3->addWidget(le_lambdaStep, 4, 1);
	pb_plotthree = us_pushbutton(tr(""));
	gl3->addWidget(pb_plotthree, 4, 2);
	lbl_maxIterations = us_label(tr("Maximum Iterations:"));
	gl3->addWidget(lbl_maxIterations, 5, 0);
   le_maxIterations = us_lineedit("50000", 1, false);
	connect(le_maxIterations, SIGNAL(textChanged(const QString&)), SLOT(update_maxIterations(const QString &)));
	gl3->addWidget(le_maxIterations, 5, 1);
	pb_plotfour = us_pushbutton(tr(""));
	gl3->addWidget(pb_plotfour, 5, 2);
	lbl_tolerance = us_label(tr("Fit Tolerance:"));
	gl3->addWidget(lbl_tolerance, 6, 0);
   le_tolerance = us_lineedit("1.000e-12", 1, false);
	connect(le_tolerance, SIGNAL(textChanged(const QString&)), SLOT(update_tolerance(const QString &)));
	gl3->addWidget(le_tolerance, 6, 1);
	pb_plotfive = us_pushbutton(tr(""));
	gl3->addWidget(pb_plotfive, 6, 2);
	lbl_progress = us_label(tr("Fitting Progress:"));
	gl3->addWidget(lbl_progress, 7, 0);
	pgb_progress = new QProgressBar(this);
	gl3->addWidget(pgb_progress, 7, 1);

	lbl_global = us_banner(tr("Global Nonlinear Least Squares\n Extinction Profile Fitting Control"));	
 	QGridLayout* subMain;
	subMain = new QGridLayout();
	subMain->addWidget(lbl_global, 0, 0);
	subMain->addLayout(gl1, 1, 0);	
	subMain->addWidget(lbl_controls1, 2, 0);	
	subMain->addLayout(gl2, 3, 0);
	subMain->addWidget(lbl_controls2, 4, 0);
	subMain->addLayout(gl3, 5, 0);

	QGridLayout* mainLayout;
	mainLayout = new QGridLayout(this);
	mainLayout->setSpacing(2);
	mainLayout->setContentsMargins(2, 2, 2, 2);
   mainLayout->addLayout(subMain, 0, 0);
	mainLayout->addLayout(plotLayout, 0, 1);
}
void US_Minimize::update_lambdaStep(const QString &str)
{
	lambdaStep = str.toFloat();
}
void US_Minimize::update_lambdaStart(const QString &str)
{
   lambdaStart = str.toFloat();
}
void US_Minimize::update_maxIterations(const QString &str)
{
	maxIterations = str.toInt();
}
void US_Minimize::update_tolerance(const QString &str)
{
	tolerance = str.toFloat();
}
void US_Minimize::updateQN(float **gamma, float **delta)
{
	unsigned int i, j;
   float *hgamma, lambda, deltagamma, *v, *temp, **vvtranspose, **ddtranspose, **hgammatranspose;
   hgamma = new float [parameters];
   v = new float [parameters];
   temp = new float [parameters];
   vvtranspose = new float *[parameters];
   hgammatranspose = new float *[parameters];
   ddtranspose = new float *[parameters];
   for (i=0; i<parameters; i++)
   {
      vvtranspose[i] = new float [parameters];
      hgammatranspose[i] = new float [parameters];
      ddtranspose[i] = new float [parameters];
   }

   matrix->mmv(&hgamma, gamma, &information_matrix, parameters, parameters);
   lambda = matrix->dotproduct(gamma, &hgamma, parameters);
   deltagamma = matrix->dotproduct(delta, gamma, parameters);
   //qDebug() << "Deltagamma: " << deltagamma  ;
  	for (i=0; i<parameters; i++)
   {
 
      v[i] = (*delta)[i]/deltagamma - hgamma[i]/lambda;
   }
   matrix->vvt(&vvtranspose, &v, &v, parameters);
   matrix->vvt(&ddtranspose, delta, delta, parameters);
   matrix->vvt(&hgammatranspose, &hgamma, &hgamma, parameters);
   for (i=0; i<parameters; i++)
   {
      for (j=0; j<parameters; j++)
      {
         information_matrix[i][j] = information_matrix[i][j] - (double) hgammatranspose[i][j]/lambda
            + (double) ddtranspose[i][j]/deltagamma + (double) vvtranspose[i][j]*lambda;
      }
   }
   matrix->mmv(&temp, gamma, &information_matrix, parameters, parameters);
   /*
 *      for (i=0; i<parameters; i++)
 *           {
 *                qDebug() << "difference: " << temp[i] - (*delta)[i]  ;
 *                     }
 *                        */
   for (i=0; i<parameters; i++)
   {
      delete [] vvtranspose[i];
      delete [] hgammatranspose[i];
      delete [] ddtranspose[i];
   }
   delete [] hgamma;
   delete [] v;
   delete [] temp;
   delete [] vvtranspose;
   delete [] hgammatranspose;
   delete [] ddtranspose;

}

int US_Minimize::Fit()
{
   //qDebug() << "converged: " << converged << ", completed: " << completed << ", aborted: " << aborted  ;
   float lambda = lambdaStart; // re-initialize Lambda
   float *search, alpha, *gamma, *delta;
   int step_counter = 0; // count how many times we try to shorten the step size in the hybrid method
   int lambda_loop = 0; //counts how many times we enlarge lambda
   // if we try more than 3 times, chances are we found the local minimum.

   QString str;
   if (converged || completed || aborted)
   {
      cleanup();
   }
   qDebug() << "starting fit\n";
   first_plot = true;      // reset first plot each time we do a new fit
   completed = false;
   aborted = false;
   converged = false;
   if (GUI)
   {
      suspend_flag = false;
      pb_close->setText(tr("Abort"));
      pb_fit->setEnabled(false);
      pb_pause->setEnabled(true);
      qApp->processEvents();
   }
   function_evaluations = 0;
   decompositions = 0;
   iteration = 0;
   variance = 0;
   if (GUI)
   {
      //      update_fitDialog();
      // lbl_status2->setText(tr("Results from loaded fit are shown"));
   }
   if (!fit_init())   // initialize the fitting process
   {
      if (GUI)
      {
         QMessageBox message;
			message.setWindowTitle(tr("Please Note:"));
			message.setText(tr("No scans have been selected\nfor fitting or all scans have\nbeen excluded.\n\nPlease review the Scan Diagnostics\nand check the scans for fit before\nproceeding."));
			message.exec();
//       lbl_status2->setText(tr("Fit aborted..."));
        // lbl_status3->setText("");
         pb_close->setText(tr("Close"));
         pb_fit->setEnabled(true);
         pb_pause->setEnabled(false);
      }
      aborted = true;
      return(-2);
   }
   if (nlsMethod == 3)
   {
      // set up Hessian matrix, initialized to identity matrix:
      for (unsigned int i=0; i<parameters; i++)
      {
         for(unsigned int j=0; j<parameters; j++)
         {
            if (i == j)
            {
               information_matrix[i][j] = 1.0;
            }
            else
            {
               information_matrix[i][j] = 0.0;
            }
         }
      }
      search = new float [parameters];
      gamma = new float [parameters];
      delta = new float [parameters];
   }
   if (GUI)
   {
      str.sprintf(" %d", parameters);
      le_parameters->setText(str);
      str.sprintf(" %ld", datasets);
      le_datasets->setText(str);
      str.sprintf(" %ld", points);
      le_points->setText(str);
   }
   bool dostep = false;   // only needed for hybrid method
   float step = 1.0;      // only needed for hybrid method
   unsigned int count;
   float target; // keeps track of progress
   float old_residuals = 0, new_residuals = 0;
   if(calc_model(guess) < 0)
   {
      if (GUI)
      {
         QMessageBox message;
			message.setWindowTitle(tr("UltraScan Error:"));
			message.setText(tr("The residuals from the initial guess\n"
                                 "are too large.\n\n"
                                 "Please manually adjust the fitting\n"
                                 "parameters and retry the fit.\n\n"
                                 "Return code: -4"));
			message.exec();
         pgb_progress->setValue(totalSteps);
         pb_close->setText(tr("Close"));
         pb_fit->setEnabled(true);
         //lbl_status2->setText(tr("The fitting process didn't converge..."));
         pb_saveFit->setEnabled(true);
         pb_report->setEnabled(true);
         pb_residuals->setEnabled(true);
         pb_overlays->setEnabled(true);
      }
      qDebug() << "The residuals from the initial guess are too large.\n"
           << "Please manually adjust the fitting parameters and retry the fit.\n"
           << "Return code: -4 (using nls method" << nlsMethod << ").\n\nFailing Parameters:\n";
      for (unsigned int i=0; i<parameters; i++)
      {
         qDebug() << guess[i]  ;
      }
      cleanup();
      return (-4);
   }
   if (GUI)
   {
      bt_plotAll->setEnabled(true);
      bt_plotGroup->setEnabled(true);
      bt_plotSingle->setEnabled(true);
   }
   new_residuals = calc_residuals();
   
   qDebug() << "returning from calc_residuals in CP1 with " << new_residuals  ;
  
   if (new_residuals < 0)
   {
      qDebug() << "exited with -4, residuals: " << new_residuals  ;
      return(-4);
   }
   //qDebug() << "Residuals: " << new_residuals  ;
   target = new_residuals/points;
   totalSteps = 0;
   if(target <= 0)
   {
      if (GUI)
      {
         QMessageBox message;
			message.setWindowTitle(tr("UltraScan Error:"));
         message.setText(tr("The residuals from the initial guess\n"
                                 "are too large.\n\n"
                                 "Please manually adjust the fitting\n"
                                 "parameters and retry the fit."));
			message.exec();
         pgb_progress->setValue(totalSteps);
         pb_close->setText(tr("Close"));
         pb_fit->setEnabled(true);
         //lbl_status2->setText(tr("The fitting process converged..."));
         pb_saveFit->setEnabled(true);
         pb_report->setEnabled(true);
         pb_residuals->setEnabled(true);
         pb_overlays->setEnabled(true);
      }
      cleanup();
      return(-5);
      //      }
   }
   while(target > tolerance)
   {
      target /= 2.0;
      totalSteps++;
   }
   totalSteps -= 3;
   target = new_residuals/(points * 2);
   if (GUI)
   {
      pgb_progress->reset();
      //pgb_progress->setValue(totalSteps);
      pgb_progress->setMaximum(totalSteps);
   }
   count = 0;

   qDebug() << "iter: " << iteration << ", maxiter: " << maxIterations << ", new_resid: " << new_residuals/points << ", tolerance: " << tolerance  ;

   ////////// BEGINNING OF ITERATIONS //////////////////////////////////////////////////////////////////////////////////////////////////////////
   while((iteration < maxIterations) && (new_residuals/points > tolerance))
   {

     US_Timer fit_timers; 
     fit_timers.init_timer ( "jacobian calc" );
     fit_timers.init_timer ( "A_transpose_A calc" );
     fit_timers.init_timer ( "calc_B calc" );
     fit_timers.init_timer( "Cholesky SolveSystem calc" );

     
     qDebug() << "current iteration: " << iteration  ;

      if(GUI && showGuiFit)
      {
	//qDebug() << "GUI: " << GUI << ", showGuiFit: " << showGuiFit  ;
	
         if(plotResiduals)
         {
	   plot_residuals();
         }
         else
         {
	   //qDebug() << "plot_overlays: 1"  ;
	   plot_overlays();                                   // BUG IN GUI QT-5.7.0 /////////////////////////////////////////////////////////////////////////////////////////
	   //qDebug() << "plot_overlays: 2"  ;
         }
         qApp->processEvents();
      }
      variance = new_residuals/points;
      iteration++;
      if (GUI)
      {
         str.sprintf("%3.5e", variance);
         le_variance->setText(str);
         str.sprintf("%3.5e", pow((double) variance, (double) 0.5));
         le_stddev->setText(str);
         str.sprintf("%3.5e", (old_residuals - new_residuals)/points);
         le_improvement->setText(str);
         str.sprintf("%d", iteration);
         le_iteration->setText(str);
      }
      old_residuals = new_residuals;
      if (old_residuals/points < target)
      {
         count++;
         if (GUI)
         {
            pgb_progress->setValue(count);
         }
         target /= 2.0;
      }
      if (GUI)
      {
         //lbl_status2->setText(tr("Calculating the Information Matrix..."));
         qApp->processEvents();
      }
      
      if ((nlsMethod < 3) || (nlsMethod == 3 && iteration == 1))
      {
	 fit_timers.start_timer( "jacobian calc" );
         calc_jacobian();
	 fit_timers.end_timer( "jacobian calc" );
	          
	 //print_matrix(jacobian, points, parameters);
	 
	 // qDebug() << "Setting threads: " << settings->threads();
	 //qDebug() << "Rows, Columns " <<  points << ", " <<  parameters; 
	 
	 //US_Matrix matrix_temp;
	 fit_timers.start_timer( "A_transpose_A calc" );
	 matrix->calc_A_transpose_A(&jacobian, &information_matrix, points, parameters, settings->threads()); // ATTENTION /////////

	 //matrix->calc_A_transpose_A(&jacobian, &information_matrix, points, parameters, 20);

	 //matrix_temp.calc_A_transpose_A(&jacobian, &information_matrix, points, parameters, 15);
	 fit_timers.end_timer( "A_transpose_A calc" );
	 //qDebug().noquote() << fit_timers.list_times();
      }
      //print_matrix(information_matrix, parameters, parameters);
      if (nlsMethod == 3)   // Quasi-Newton
      {
         if (iteration == 1)
         {
            double **inf;
            inf =  new  double * [parameters];
            for (unsigned int i=0; i<parameters; i++)
            {
               inf[i] = new double [parameters];
            }
            calc_B();
            for (unsigned int i=0; i<parameters; i++)
            {
               for (unsigned int j=0; j<parameters; j++)
               {
                  inf[i][j] = information_matrix[i][j];
               }
            }
            matrix->Cholesky_Invert(inf, LL_transpose, parameters);
            for (unsigned int i=0; i<parameters; i++)
            {
               delete [] inf[i];
            }
            delete [] inf;

            for (unsigned int i=0; i<parameters; i++)
            {
               for (unsigned int j=0; j<parameters; j++)
               {
                  information_matrix[i][j] = LL_transpose[i][j];
               }
            }
         }
         float test=0;
         for (unsigned int i=0; i<parameters; i++)
         {
            test += pow((double) B[i], (double) 2);
         }
         test = pow((double) test, (double) 0.5);
         if (test < tolerance)
         {
            variance = new_residuals/points;
            //qDebug() << "Calling endfit 1\n";
            endFit();
            delete [] delta;
            delete [] gamma;
            delete [] search;
            return(0);
         }
         /*
           print_matrix(information_matrix, parameters, parameters);
           for (unsigned int i=0; i<parameters; i++)
           {
           qDebug() << "Search before: " << search[i]  ;
           }
         */
         matrix->mmv(&search, &B, &information_matrix, parameters, parameters);
         /*
           for (unsigned int i=0; i<parameters; i++)
           {
           qDebug() << "Search after: " << search[i]  ;
           }
         */
         for (unsigned int i=0; i<parameters; i++)
         {
            gamma[i] = (float) B[i];   // B = J'y_delta = -gradient; in updateQN we need
         }                              // gamma=gradient(k+1) - gradient(k)
         // in updateQN we will calculate gamma = gamma - B
         // where gamma at this point is the old gradient, and -B is the new       
			// gradient.
         alpha = linesearch(&search, new_residuals);
         if (alpha == 0.0)
         {
            variance = new_residuals/points;
            delete [] delta;
            delete [] gamma;
            delete [] search;
            //qDebug() << "Calling endfit 2, points: " << points << "\n";
            endFit();
            return(0);
         }
         if (alpha < 0)
         {
            if (autoconverge)
            {
               delete [] delta;
               delete [] gamma;
               delete [] search;
               cleanup();
               return(-10);
            }
            else
            {
               QMessageBox message;
					message.setWindowTitle(tr("UltraScan Error:"));
					message.setText(tr("The fit failed to converge!\n\nPlease try different initial guesses."));
					message.exec();
               delete [] delta;
               delete [] gamma; 
			     	delete [] search;
               cleanup();
               return(-10);
            }
         }
         for (unsigned int i=0; i<parameters; i++)
         {
            guess[i] = guess[i] + search[i] * (double) alpha;
         }
         calc_model(guess);// updates solution, needed to calculate y_delta
         calc_jacobian();  // gives us jacobian matrix
         old_residuals = new_residuals;
         new_residuals = calc_residuals(); // gives us new y_delta
         //qDebug() << "returning from calc_residuals in CP2 with " << new_residuals  ;
         calc_B();
         for (unsigned int i=0; i<parameters; i++)
         {
            gamma[i] = gamma[i] - (float) B[i];
            delta[i] = alpha * search[i];
            //qDebug() << "Paramter " << i << ": gamma:" << gamma[i] << ", delta: " << delta[i] << ", alpha: " << alpha << ", search: " << search[i] << ", B: " << B[i]  ;
         }
         updateQN(&gamma, &delta);
      }
      /// END OF QUASI-NEWTON //////////////////////////////////////////////////////////////////////////////////////////

      if (nlsMethod == 0 || nlsMethod == 2)
      {
         // Add lambda to the information matrix to make diagonal large to make columns
         // linearly independent:
         for (unsigned int i=0; i<parameters; i++)
         {
	   information_matrix[i][i] += lambda;
         }
      }

      while (new_residuals >= old_residuals && nlsMethod != 3)
      {
	//qDebug() << "Going to Method: "  ;
	//qDebug() << "FLT_MIN: " << FLT_MIN ;
	//qDebug() << "FLT_MAX: " << FLT_MAX ;

         //   Problem: J * R = y_delta, where R = (a[i] - guess[i]) and we want to find a[i], so solve for R:
         // (1)   J'J * R = J' * y_delta

         // Solution - try to find R:
         // (2)   (J'J)-1 * (J'J) * R = (J'J)-1 * (J' * y_delta)
         // (3)   R = (J'J)-1 * (J`*y_delta)

         // Alternative Solution: This can be solved using first forward and then backward substitution,
         // and Cholesky Decomposition:
         // (1)   J'J * R = J'*y_delta

         // (2)    Substitute: B = J'*y_delta

         if (GUI)
         {
            //lbl_status2->setText(tr("Calculating the Gradient..."));
            qApp->processEvents();
         }
	 
	 fit_timers.start_timer( "calc_B calc" );
	 calc_B();
	 fit_timers.end_timer( "calc_B calc" );
	 //qDebug().noquote() << fit_timers.list_times();
 

        // save information matrix in LL_transpose in case chi-2 is larger, in which case we would
         // need to reset Lambda

         //cout.precision(3);
         //cout.setf(ios::scientific|ios::showpos);

         //qDebug() << "\nLL_transpose (for " << parameters << " parameters): ";
         for (unsigned int i=0; i<parameters; i++)
         {
            //qDebug() << "\n";
            for (unsigned int j=0; j<parameters; j++)
            {
               //               qDebug() << i << ", " << j << ": " << information_matrix[i][j] << " ";
               LL_transpose[i][j] = information_matrix[i][j];
            }
         }
         //qDebug() << "\n";

         // (3)   J'J -> Cholesky Decomposition -> LL'

         decompositions++;
         if (GUI)
         {
            //lbl_status2->setText(tr("Processing Cholesky Decomposition..."));
            str.sprintf("%d", decompositions);
            le_decompositions->setText(str);
            qApp->processEvents();
         }

         if(!matrix->Cholesky_Decomposition(LL_transpose, parameters))
         {
            if (GUI)
            {
               if (nlsMethod == 0)
               {
		 qDebug() << "Cholesky: "  ;
                  if (showGuiFit && !us_auto_mode )
                  {
                     QMessageBox message;
							message.setWindowTitle(tr("Attention:"));
							message.setText( tr("The Cholesky Decomposition of the\nInformation matrix failed due to a\nsingularity in the matrix.\n\nYou may achieve convergence by\nre-fitting the current data with\nnew initial parameter estimates."));

							message.exec();
                  }
               }
               else
               {
                  if (showGuiFit && !us_auto_mode )
                  {
                     QMessageBox message;
							message.setWindowTitle(tr("Attention:"));
							message.setText( tr("The Cholesky Decomposition of the\nInformation matrix failed due to a\nsingularity in the matrix.\n\nYou may achieve convergence by\nre-fitting the current data with\nthe Levenberg-Marquardt method or\nby using different initial parameter\nestimates."));
							message.exec();
                  }
               }
              // lbl_status2->setText(tr("Fit aborted..."));
              // lbl_status3->setText("");
               pb_close->setText(tr("Close"));
               pb_resume->setEnabled(false);
               pb_pause->setEnabled(false);
               pb_fit->setEnabled(true);
            }
            //qDebug() << "Aborting at 2...\n";
            aborted = true;
            cleanup();
            return (-6);
         }
	 
	 //qDebug() << "Continue Method"  ;

         // Now the information matrix actually contains the decomposed information matrix LL'
         // (4)   L(L'*R) = B
         // (5)    Substitute:  Z = L'*R
         // (6)    Solve for Z using forward substitution: L*Z = B
         // (7)    Solve for R using backward substitution: L'*R = Z

         if (GUI)
         {
            //lbl_status2->setText(tr("Solving Cholesky System..."));
            qApp->processEvents();
         }
	 fit_timers.start_timer( "Cholesky SolveSystem calc" );
         matrix->Cholesky_SolveSystem(LL_transpose, B, parameters);
	 fit_timers.end_timer( "Cholesky SolveSystem calc" );
// 	 qDebug().noquote() << fit_timers.list_times();
         // Now B is changed to R (the difference between the current parameter estimate with the "true" parameter)

         switch (nlsMethod)
         {
         case 0:
            {
               for (unsigned int i=0; i<parameters; i++)
               {
                  test_guess[i] = guess[i] + (float) B[i];
               }
               break;
            }
         case 1:
            {
               // Add the new R (parameter correction) to the old guess
					float *t, st;
               t = new float [parameters];
               for (unsigned int i=0; i<parameters; i++)
               {
                  t[i] = (float) B[i];
               }
               //qDebug() << "calling linesearch from lsp2\n";
               st = linesearch(&t, new_residuals);
               delete [] t;
               if (st == 0)
               {
                  variance = new_residuals/points;
                  //qDebug() << "Calling endfit 3\n";
                  endFit();
                  return(0);
               }
               if (st < 0)
               {
                  QMessageBox message;
						message.setWindowTitle(tr("UltraScan Error:"));
						message.setText(tr("The fit failed to converge!\n\nPlease try different initial guesses."));
						message.exec();
                  return(-11);
               }
               //               qDebug() << "Alpha: " << st  ;
               for (unsigned int i=0; i<parameters; i++)
               {
                  test_guess[i] = guess[i] + st * B[i];
                  //                  test_guess[i] = guess[i] + lambda * (float) B[i];
               }
					break;
				}
         case 2:
            {
               float *t, st;
               t = new float [parameters];
               for (unsigned int i=0; i<parameters; i++)
               {
                  t[i] = (float) B[i];
               }
               //qDebug() << "calling linesearch from lsp3\n";
               st = linesearch(&t, new_residuals);
               delete [] t;
               if (st == 0)
               {
                  variance = new_residuals/points;
                  //qDebug() << "Calling endfit 4\n";
                  endFit();
                  return(0);
               }
               if (st < 0)
               {
                  QMessageBox message;
						message.setWindowTitle(tr("UltraScan Error:"));
						message.setText(tr( "The fit failed to converge!\n\nPlease try different initial guesses."));
						message.exec();
                  return(-11);
               }
               for (unsigned int i=0; i<parameters; i++)
               {
                  test_guess[i] = guess[i] + st * B[i];
               }
               break;
            }
         }
	 if(calc_model(test_guess) < 0)
         {
	   cleanup();
            return(-7);
         }

	 // qDebug() << "Continue Method 1"  ;

         new_residuals = calc_residuals();
         if (new_residuals < old_residuals)
         {
	   //qDebug() << "new_residuals <  old_residuals"  ;
            if (GUI)
            {
	      qApp->processEvents();
            }
            switch (nlsMethod)
            {
            case 0:
               {
                  lambda = lambda/lambdaStep;
		  //qDebug() << "lambda = " << lambda << ", lambdaStep = " << lambdaStep  ;
                  break;
               }
            case 1:
               {
                  lambda_loop ++;
                  lambda *= pow((double) lambdaStep, (double) lambda_loop);
                  if (lambda > 1.0e10)
                  {
                     lambda = 1.0e6;
                     variance = new_residuals/points;

                     endFit();
                     return(0);
                  }
                  break;
               }
            case 2:
               {
                  lambda = lambda/lambdaStep;
                  if (lambda < 1.0)
                  {
                     if (!dostep)
                     {
                        step =  (float) 0.01;
                     }
                     lambda = 0.0;
                     step *= 2.0;
                     step_counter = 0; //reset step counter to zero, because we lengthened the step.
                     dostep = true;
                  }
                  break;
               }
            }
            for (unsigned int i=0; i<parameters; i++)
            {
               guess[i] = test_guess[i];
            }
         }
         else if (new_residuals == old_residuals)
         {
            variance = new_residuals/points;
            endFit();
            return(0);
         }
         else
         {
	   //qDebug() << "new_residuals > old_residuals"  ;
            if(GUI)
            {
               str.sprintf("%3.5e", (old_residuals - new_residuals)/points);
               le_improvement->setText(str);
	       qApp->processEvents();
            }
            switch (nlsMethod)
            {
            case 0:
               {
                  for (unsigned int i=0; i<parameters; i++)
                  {
                     information_matrix[i][i] -= lambda;
                  }
                  lambda_loop ++;
                  lambda *= pow((double) lambdaStep, (double) lambda_loop);

		  // qDebug() << "After new_res MORE: "  ;

                  if (lambda > 1.0e10)
                  {
                     lambda = 1.0e6;
                     variance = new_residuals/points;
		     // qDebug() << "Calling endfit 7\n";
                     endFit();
                     return(0);
                  }
                  for (unsigned int i=0; i<parameters; i++)
                  {
                     information_matrix[i][i] += lambda;
                  }
                  break;
               }
            case 1:
               {
                  lambda /= lambdaStep;
                  if (lambda < tolerance)
                  {
                     endFit();
                     return(0);
                  }
                  break;
               }
            case 2:
               {
                  for (unsigned int i=0; i<parameters; i++)
                  {
                     information_matrix[i][i] -= lambda;
                  }
                  lambda *= lambdaStep;
                  if (lambda > 1.0e10)
                  {
                     lambda = 1.0e6;
                     if (GUI)
                     {
                        qApp->processEvents();
                     }
                     variance = new_residuals/points;
                     endFit();
                     return(0);
                  }
                	if (dostep)
                  {
                     step /= 2.0;
                     step_counter++;
                     if (step_counter > 3 && step < tolerance)
                     {
                        endFit();
                        return(0);
                     }
                  }
                  for (unsigned int i=0; i<parameters; i++)
                  {
                     information_matrix[i][i] += lambda;
                  }
                  break;
               }
            }
            if(calc_model(guess) < 0)
            {
	      //qDebug() << "After new_res MORE 1:"  ;
               cleanup();
               return (-8);
            }
	    //qDebug() << "After new_res MORE 11:"  ;
            new_residuals = calc_residuals();
	    //qDebug() << "After new_res MORE 12:"  ;
         }
         if (GUI)
         {
            str.sprintf("%3.5e", lambda);
            le_currentLambda->setText(str);
	    //return(0);
         }
      }
      //qDebug() << "Continue Method 2"  ;
      //qDebug() << "new_residuals = " << new_residuals << ", points = " <<  points << ", tolerance = " << tolerance  ;
      //qDebug() << "GUI: " << GUI  ;
   }
   //qDebug() << "END ITER: "  ;
   return(0);
}

float US_Minimize::linesearch(float **search, float f0)
{
   bool check_flag=true;
   double old_f0=0.0, old_f1=0.0, old_f2=0.0;
   //qDebug() << "before: " << errno  ;
   errno = 0; //clear old error state
   // look for the minimum residual. Residual values are f0, f1, f2, evaluated at x0, x1, x2.
   // x0, x1, x2 are multipliers for incremental change of the parameter vector 'search'
   // `search` contains the vector of parameters we need to optimize
   // calculate bracket: Assume the minimum is between x0=0 and some stepsize x2 away from x0.
   // then find an x1 in the middle between x0 and x2 and calculate f1(x1) and f2(x2), where
   // shift to the right
   // f is the residual of the function.
   float x0 = 0.0, x1 =  (float) 0.5, x2 = 1.0, h =  (float) 0.01, xmin, fmin;
   unsigned int iter = 0;
   //qDebug() << "calling calc_testParameter from ctp1...\n";
   float f1 = calc_testParameter(search, x1);
   if (f1 < 0) return(0.0);
   //qDebug() << "-1: " << errno  ;

   //qDebug() << "calling calc_testParameter from ctp2...\n";
   float f2 = calc_testParameter(search, x2);
   if (f2 < 0) return(0.0);
   //qDebug() << "-2: " << errno << ", f0: " << f0 << ", f1: " << f1 << ", f2: " << f2  ;

   while (errno != 0 || (f0 >= 10000 || f0 < 0 || f1 >= 10000 || f1 < 0 || f2 >= 10000 || f2 < 0)) //make the initial step size smaller if we have infinite residuals
   {
      x1 /= 10;
      x2 /= 10;
      //qDebug() << "Calling from 1. loop\n";
      //qDebug() << "calling calc_testParameter from Ctp3...\n";
      f1 = calc_testParameter(search, x1);
      if (f1 < 0) return(0.0);
      //qDebug() << "calling calc_testParameter from ctp4...\n";
      f2 = calc_testParameter(search, x2);
      if (f2 < 0) return(0.0);
      if (x1 < FLT_MIN)
      {
         //qDebug() << "abandoned with: " << "x0: " << x0 << ", x1: " << x1 << ", x2: " << x2 << ", f0: " << f0 << ", f1: " << f1 << ", f2: " << f2  ;
         //qDebug() << FLT_MIN << ", " << FLT_MAX << ", errno: " << errno  ;
         errno = 0;
         return (-1); //couldn't do anything for this search direction - fit didn't converge
      }
   }
   check_flag=true;
   while(check_flag)
   {
      //qDebug() << "mathlib: "  << errno << ", x0: " << x0 << ", x1: " << x1 << ", x2: " << x2 << ", f0: " << f0 << ", f1: " << f1 << ", f2: " << f2 << ", d1: " << f1-f0 << ", d2: " << f2-f1 << ", d3: " << f2-f0  ;
      if((qIsNaN(f0) && qIsNaN(f1))
         || (qIsNaN(f1) && qIsNaN(f2))
         || (qIsNaN(f0) || qIsNaN(f2))) // at least two values are screwed up, exit.
      {
         //qDebug() << "error1 " << "x0: " << x0 << ", x1: " << x1 << ", x2: " << x2 << ", f0: " << f0 << ", f1: " << f1 << ", f2: " << f2  ;
         errno = 0;
         return(-1);
      }
      // Check to see if the solution is really converging:

      if ((fabs(f2 - old_f2) < FLT_MIN) && (fabs(f1 - old_f1) < FLT_MIN) && (fabs(f0 - old_f0) < FLT_MIN))   // is the solution horizontal?
      {
         return(0);   // we found the minimum, return alpha=0
      }
      old_f0 = f0;
      old_f1 = f1;
      old_f2 = f2;

      if ( (
             (fabs(f2 - f0) < FLT_MIN) && (fabs(f1 - f0) < FLT_MIN)
           ) || 
           (
             (f0 > f1) && (fabs(f2 - f1) < FLT_MIN)
           )
         )   // is the solution horizontal?
      {
         //qDebug() << "error2 "  << "x0: " << x0 << ", x1: " << x1 << ", x2: " << x2 << ", f0: " << f0 << ", f1: " << f1 << ", f2: " << f2  ;
         return(0);   // we found the minimum, return alpha=0
      }
      if ((fabs(x0) < FLT_MIN) && (fabs(x1) < FLT_MIN) && (fabs(x2) < FLT_MIN))   // is the solution horizontal?
      {
         //qDebug() << "error3 "  << "x0: " << x0 << ", x1: " << x1 << ", x2: " << x2 << ", f0: " << f0 << ", f1: " << f1 << ", f2: " << f2  ;
         return(0);   // we found the minimum, return alpha=0
      }
      //      if ((f0 < f1) && (f1 == f2) || (f0 < f1) && (f1 > f2) || (fabs(f1 - f0) < FLT_MIN) && f2 > f1)   // some weird cases can happen near the minimum
      if (((fabs(f0 - f1) < FLT_MIN) && (fabs(f1 - f2) < FLT_MIN))
          || ((fabs(f0 - f1) < FLT_MIN) && (f2 > f1)))   // some weird cases can happen near the minimum
      {
         //qDebug() << "error4 "  << "x0: " << x0 << ", x1: " << x1 << ", x2: " << x2 << ", f0: " << f0 << ", f1: " << f1 << ", f2: " << f2  ;
         return(0);   // we found the minimum, return alpha=0
      }
      if (f0 > f1 && f2 > f1) // we have a bracket
      {
         //qDebug() << "bracket " << "x0: " << x0 << ", x1: " << x1 << ", x2: " << x2 << ", f0: " << f0 << ", f1: " << f1 << ", f2: " << f2  ;
         check_flag = false;
         break;
      }
      else if ((f2 > f1 && f1 > f0) || (f1 > f0 && f1 > f2) || (f1 == f2 && f1 > f0)) // shift to the left
      {
         //qDebug() << "leftshift " << "x0: " << x0 << ", x1: " << x1 << ", x2: " << x2 << ", f0: " << f0 << ", f1: " << f1 << ", f2: " << f2  ;
         x2 = x1;
         f2 = f1;
         x1 = (x2 + x0)/2.0;
         //qDebug() << "calling calc_testParameter from ctp5...\n";
         f1 = calc_testParameter(search, x1);
         if (f1 < 0) return(0.0);
      }
      else if (f0 > f1 && f1 > f2) // shift to the right
      {
         //qDebug() << "rightshift " << "x0: " << x0 << ", x1: " << x1 << ", x2: " << x2 << ", f0: " << f0 << ", f1: " << f1 << ", f2: " << f2  ;
         x0 = x1;
         f0 = f1;
         x1 = x2;
         f1 = f2;
         x2 = x2 + (pow((double) 2, (double) (iter+2))) * h;
         //qDebug() << "calling calc_testParameter from ctp6...\n";
         f2 = calc_testParameter(search, x2);
         if (f2 < 0) return(0.0);
         //qDebug() << "rightshift #2" << "x0: " << x0 << ", x1: " << x1 << ", x2: " << x2 << ", f0: " << f0 << ", f1: " << f1 << ", f2: " << f2  ;
      }
      iter++;
      //qDebug() << iter  ;
   } // get a bracket
   // search inside the bracket for the minimum and do a 2nd order polynomial fit
   x1 = (x0 + x2)/2.0;
   h = x1 - x0;
   //qDebug() << "Calling from 2. loop\n";
   //qDebug() << "calling calc_testParameter from ctp7...\n";
   f1 = calc_testParameter(search, x1);
   if (f1 < 0) return(0.0);
   while(true)
   {
      if (f0 < f1) // shift left
      {
         x2 = x1;
         f2 = f1;
         x1 = x0;
         f1 = f0;
         x0 = x1 - h;
         //qDebug() << "Calling from 3. loop\n";
         //qDebug() << "calling calc_testParameter from ctp8...\n";
         f0 = calc_testParameter(search, x0);
         if (f0 < 0) return(0.0);
      }
      if (f2 < f1) // shift right
      {
         x0 = x1;
         f0 = f1;
         x1 = x2;
         f1 = f2;
         x2 = x1 + h;
         //qDebug() << "Calling from 4. loop\n";
         //qDebug() << "calling calc_testParameter from ctp9...\n";
         f2 = calc_testParameter(search, x2);
         if (f2 < 0) return(0.0);
      }
      errno = 0;
      if (fabs(f0 - 2 * f1 + f2) <  FLT_MIN)
      {
         return(0); //division by zero above
      }
      xmin = x1 + (h * (f0 - f2))/(2 * (f0 - 2 * f1 + f2));
      //qDebug() << "Calling from 5. loop\n";
      //qDebug() << "xmin: " << xmin << ", diff: " << (2 * (f0 - 2 * f1 + f2)) << ", h: " << h << ", f0: " << f0 << ", f1: " << f1 << ", f2: " << f2  ;
      //qDebug() << "calling calc_testParameter from ctp10...\n";
      fmin = calc_testParameter(search, xmin);
      if (fmin < 0) return(0.0);

      if (fmin < f1)
      {
         x1 = xmin;
         f1 = fmin;
      }
      h = h / 2.0;
      if (h < tolerance)
      {
         return(x1);
      }
      x0 = x1 - h;
      x2 = x1 + h;
      //qDebug() << "Calling from 6. loop\n";
      //qDebug() << "calling calc_testParameter from ctp11...\n";
      f0 = calc_testParameter(search, x0);
      if (f0 < 0) return(0.0);
      //qDebug() << "calling calc_testParameter from ctp12...\n";
      f2 = calc_testParameter(search, x2);
      if (f2 < 0) return(0.0);
   }
}


float US_Minimize::calc_testParameter(float **search, float step)
{
   float res;
   for (unsigned int i=0; i<parameters; i++)
   {
      test_guess[i] = guess[i] + (double) step * (*search)[i];
      //qDebug() << "Step: " << step << ", Test-guess(" << i << "): " << test_guess[i] << ", guess: " << guess[i] << ", search: " << (*search)[i]  ;
   }
   /*
     QString s="", format;
     for (unsigned int i=0; i<parameters; i++)
     {
     s += format.sprintf("Parameter[%d]: %e, Search[%d]: %e\n", i, test_guess[i], i, (*search)[i]);
     }
     QMessageBox message;
		message.setWindowTitle(tr("Debug:"));
		message.setText(tr("The following values are passed to calc_model:\n" + s + ""));
		message.exec();
   */
   if(calc_model(test_guess) < 0)
   {
      qDebug() << "Attention: there was an error in the model calculation, resetting to original parameters...\n";
      for (unsigned int i=0; i<parameters; i++)
      {
         test_guess[i] = guess[i];
         //qDebug() << "Step: " << step << ", Test-guess(" << i << "): " << test_guess[i] << ", guess: " << guess[i] << ", search: " << (*search)[i]  ;
         calc_model(guess); // reset model to
      }
   }
   if (errno > 0) return(-1.0);
   //qDebug() << "calling calc_residuals from calc_testParameter...\n";
   res = calc_residuals();
   //qDebug() << "res: " << res  ;
   return (res);
}
void US_Minimize::plot_overlays()
{
  plotResiduals = false;
}
void US_Minimize::plot_residuals()
{
	plotResiduals = true;
}
float US_Minimize::calc_residuals()
{
	float residual = 0.0;
  	errno = 0;
   for (unsigned int i=0; i<points; i++)
   {
      y_delta[i] = y_raw[i] - y_guess[i];
      //qDebug() << "y_raw: " << y_raw[i] << ", y_guess: " << y_guess[i]  ;
      residual += pow((double) y_delta[i], (double) 2.0);
	  	if (errno != 0)
      {
         qDebug() << "Floating point exception in the residuals calculation!\n";
         if (GUI)
         {
            QMessageBox message;
				message.setWindowTitle(tr("UltraScan Error:"));
            message.setText(tr("The residuals from the initial guess\n"
                                    "are too large.\n\n"
                                    "Please manually adjust the fitting\n"
                                    "parameters and retry the fit.\n\n"
                                    "Return code: -4"));
				message.exec();
            pgb_progress->setValue(totalSteps);
            pb_close->setText(tr("Close"));
            pb_fit->setEnabled(true);
            //lbl_status2->setText(tr("The fitting process converged..."));
            pb_saveFit->setEnabled(true);
            pb_report->setEnabled(true);
            pb_residuals->setEnabled(true);
            pb_overlays->setEnabled(true);
         }
         cleanup();
         qDebug() << "about to return with -1\n";
         return (-1);
      }
   }
   if(residual > FLT_MAX)
   {
      residual = -1;
   }
   return residual;
}
void US_Minimize::closeEvent(QCloseEvent *e)
{
	emit fittingWidgetClosed();
	e->accept();
}
bool US_Minimize::fit_init()
{
	emit fitStarted();
	return(false);
}
//Garbage collection
void US_Minimize::cleanup()
{
}
//calculate the simulation functions
int US_Minimize::calc_model(double *)
{
	return(-1);
}
void US_Minimize::save_Fit()
{
	QMessageBox mBox;
	QPushButton* bOk = mBox.addButton(tr("Ok"), QMessageBox::RejectRole);
	mBox.setDefaultButton(bOk);
	mBox.setWindowTitle(tr("Extinction Fitter:"));
	mBox.setTextFormat(Qt::RichText);
	mBox.setText(tr("Please use the save function from the main extinction calculation module. It will allow you to save the coefficients of the fit as well as a copy of the plot window"));
	mBox.setIcon(QMessageBox::Information);
	mBox.exec();
}
// if a fit was loaded, update all the dialogs and plot windows:
void US_Minimize::update_fitDialog()
{
    if (!GUI)
   {
      return;
   }
   QString str;
   str.sprintf("%1.4e", variance);
   le_variance->setText(str);
   str.sprintf("%d", function_evaluations);
   le_evaluations->setText(str);
   str.sprintf("%1.4e", pow((double) variance, (double) 0.5));
   le_stddev->setText(str);
   str.sprintf("%1.4e", 0.0);
   le_improvement->setText(str);
   //lbl_status2->setText(tr("Results from loaded fit are shown"));
   str.sprintf(" %d", parameters);
   le_parameters->setText(str);
   str.sprintf(" %ld", datasets);
   le_datasets->setText(str);
   str.sprintf(" %ld", points);
   le_points->setText(str);
   str.sprintf((tr("%ld Runs (%2.2f")).toLatin1().data(), runs, runs_percent);
   str += " %)";
   //lbl_status4->setText(str);
}

void US_Minimize::update_nlsMethod(int item)
{
   nlsMethod = item;
   switch (item)
   {
   case 0:
      {
         lambdaStart = 20.0;
         lambdaStep = 1.05;
         if (GUI)
         {
            //lbl_status4->setText(tr("Levenberg-Marquardt Method selected..."));
            le_lambdaStart->setText(" 20.0");
            le_lambdaStep->setText(" 1.05");
         }
         break;
      }
   case 1:
      {
         lambdaStart =  (float) 20.0;
         lambdaStep = 1.05;
         if (GUI)
         {
            //lbl_status4->setText(tr("Modified Gauss Newton Method selected..."));           
            le_lambdaStart->setText(" 20.0");
            le_lambdaStep->setText(" 1.05");

         }
         break;
      }
   case 2:
      {
         lambdaStart = 20;
         lambdaStep = 1.05;
         if (GUI)
         {
            //lbl_status4->setText(tr("Hybrid Method selected..."));
            le_lambdaStart->setText(" 20.0");
            le_lambdaStep->setText(" 1.05");
         }
         break;
      }
   case 3:
      {
         if (GUI)
         {
            //lbl_status4->setText(tr("Quasi-Newton Method selected..."));
         }
         break;
      }
   case 4:
      {
         lambdaStart = 0.0;
         lambdaStep = 0.0;
         if (GUI)
         {  
            //lbl_status4->setText(tr("Generalized Linear Least Squares Method selected..."));
            le_lambdaStart->setText(" 0.0");
            le_lambdaStep->setText(" 0.0");
         }
         break;
      }
   }
}

// calculate the B matrix: 
int US_Minimize::calc_B()
{
   for (unsigned int i=0; i<parameters; i++)
   {
      B[i] = 0.0;
      for (unsigned int j=0; j<points; j++)
      {
         B[i] += jacobian[j][i] * y_delta[j];
         //         qDebug() << "jac: " << jacobian[j][i] << ", y_delta: " << y_delta[j]  ;
      }
      //      qDebug() << "B["<<i<<"]: " << B[i]<<endl;
   }
   return (-1);
}
int US_Minimize::calc_jacobian()
{
	return(-1);
}
// called when fit has converged, can be overridden by derived class

void US_Minimize::endFit()
{
   emit currentStatus(tr("Converged"));
   emit hasConverged();
   converged = true; 
   if (GUI)
   {
     pgb_progress->setValue(totalSteps);
     //pgb_progress->setValue(100); 
     pb_close->setText(tr("Close"));
      pb_fit->setEnabled(true);
      //lbl_status2->setText(tr("The fitting process converged..."));
      pb_saveFit->setEnabled(true);
      pb_report->setEnabled(true);
      pb_residuals->setEnabled(true);
      pb_overlays->setEnabled(true);
      qDebug() << "EndFit in US_minimize "  ;
   }
}

