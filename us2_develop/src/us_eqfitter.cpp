#include "../include/us_eqfitter.h"
#include <cerrno>
#include "../include/us_math.h"

US_EqFitter::US_EqFitter(   vector <struct EquilScan_FitProfile> *temp_scanfit_vector,
                            vector <struct runinfo> *temp_run_information,
                            struct EquilRun_FitProfile *temp_runfit,
                            class US_EqModelControl **temp_model_control,
                            const unsigned int temp_model,
                            bool temp_GUI,
                            vector <QString> *temp_scan_info,
                            bool *temp_fitting_widget, QString *temp_projectName, float temp_od_limit,
                            QWidget *p,
                            const char *name) : US_Minimize(temp_fitting_widget, temp_GUI, p, name)
{
   GUI = temp_GUI;
   runs = 0;
   od_limit = temp_od_limit;
   runs_percent = 0;
   plotResiduals = false;
   print_plot = false;
   scanfit_vector = temp_scanfit_vector;
   runfit = temp_runfit;
   which_model = temp_model;
   run_information = temp_run_information;
   model_control = temp_model_control;
   scan_info = temp_scan_info;
   projectName = temp_projectName;
   pm = new US_Pixmap();
   initialized_1 = false;
   initialized_2 = false;
   if (GUI)
   {
      createHtmlDir();
      lbl_header->setText(tr("Nonlinear Least Squares\nGlobal Equilibrium Fitting Control"));
      pb_plotfive->setText(tr("Comments"));
      pb_plotfive->setEnabled(true);
   }
   cp_list.clear();
   rotor_list.clear();
   if (!readCenterpieceInfo(&cp_list))
   {
      QString msg_str = "There was a problem opening the\n"
         "centerpiece database file:\n\n"
         + USglobal->config_list.system_dir + "/etc/centerpiece.dat\n\n"
         "Please install the centerpiece database file\n"
         "before proceeding.";
      if (GUI)
      {
         QMessageBox::critical(0, "UltraScan Fatal Error:", msg_str, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
      }
      else
      {
         cout << msg_str.latin1() << endl;
      }
      exit(-1);
   }
   if (!readRotorInfo(&rotor_list))
   {
      QString msg_str = "There was a problem opening the\n"
         "rotor database file:\n\n"
         + USglobal->config_list.system_dir + "/etc/rotor.dat\n\n"
         "Please install the rotor database file\n"
         "before proceeding.";
      if (GUI)
      {
         QMessageBox::critical(0, "UltraScan Fatal Error:", msg_str, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
      }
      else
      {
         cout << msg_str.latin1() << endl;
      }
      exit(-1);
   }
}

US_EqFitter::~US_EqFitter()
{
}

bool US_EqFitter::fit_init()
{

   unsigned int i, j, point_counter = 0, dataset_counter = 0, numpoints;
   //
   // Calculate how many points there are in each dataset, sum them up for "points" and
   // leave out datasets that aren't fitted to keep the vectors/matrices as small as possible.
   //
   points = 0;
   datasets = 0;

   i = 0;
   while (!(*scanfit_vector)[i].FitScanFlag)
   {
      i++;
   }
   first_fitted_scan = i;
   for (i=0; i<(*scanfit_vector).size(); i++)
   {
      if ((*scanfit_vector)[i].FitScanFlag)
      {
         numpoints = (*scanfit_vector)[i].stop_index - (*scanfit_vector)[i].start_index;
         points_per_dataset.push_back(numpoints);
         points += numpoints;
         datasets ++;
      }
   }
   if (datasets == 0)
   {
      return(false);
   }

   // How many parameters need to be fit?

   parameters = 0;

   for (i=0; i<(*runfit).components; i++)
   {
      if ((*runfit).mw_fit[i] == true || (*runfit).vbar20_fit[i] == true)
      {
         parameters++;
      }
      if ((*runfit).virial_fit[i] == true)
      {
         parameters++;
      }
   }
   for (i=0; i<(*scanfit_vector).size(); i++)
   {
      if ((*scanfit_vector)[i].FitScanFlag == true)
      {
         for (j=0; j<(*runfit).components; j++)
         {
            if ((*scanfit_vector)[i].amplitude_fit[j] == true)
            {
               parameters++;
            }
         }
         if ((*scanfit_vector)[i].baseline_fit == true)
         {
            parameters++;
         }
      }
   }
   for (i=0; i<(*runfit).association_constants; i++)
   {
      if ((*runfit).eq_fit[i] == true)
      {
         parameters++;
      }
   }
   if (GUI)
   {
      (*model_control)->fp_mw->setFloatDisabled();
      (*model_control)->fp_amplitude->setFloatDisabled();
      (*model_control)->fp_baseline->setFloatDisabled();
   }
   y_raw                  = new float    [points];      // experimental data (absorbance)
   y_guess               = new double   [points];      // simulated solution
   y_delta               = new double   [points];      // y_raw - y_guess
   B                     = new double   [parameters];
   guess                  = new double   [parameters];
   test_guess            = new double   [parameters];
   jacobian               = new double  *[points];
   information_matrix   = new double  *[parameters];
   LL_transpose         = new double  *[parameters];
   dcr2                  = new double  *[datasets];
   dlncr2               = new double  *[datasets];
   lncr2                  = new double  *[datasets];

   for (i=0; i<points; i++)
   {
      jacobian[i] = new double [parameters];
   }
   if (which_model != 3)
   {
      for (i=0; i<parameters; i++)
      {
         information_matrix[i]   = new double [parameters];
         LL_transpose[i]         = new double [parameters];
      }
   }

   // initialize y_raw:
   point_counter = 0;
   dataset_counter = 0;
   for (i=0; i<(*scanfit_vector).size(); i++)
   {
      if ((*scanfit_vector)[i].FitScanFlag)
      {
         vector<float>::iterator yval_it = (*scanfit_vector)[i].y.begin() + (*scanfit_vector)[i].start_index;
         lncr2[dataset_counter] = new double [points_per_dataset[dataset_counter]];
         dlncr2[dataset_counter] = new double [points_per_dataset[dataset_counter]];
         dcr2[dataset_counter] = new double [points_per_dataset[dataset_counter]];
         for (unsigned int j=0; j<points_per_dataset[dataset_counter]; j++)
         {
            y_raw[point_counter] = *yval_it;
            //cout << "Y["<<point_counter<<"]: " << y_raw[point_counter] << ", i: " << i << endl;
            yval_it++;
            point_counter++;
         }
         dataset_counter++;
      }
   }

   // Initialize parameter guess:
   guess_mapForward(guess);

   return(true);
}

void US_EqFitter::guess_mapForward(double *guess_par)
{
   /*   Parameter order (needs to be maintained so Jacobian columns match):
    *
    *   for each component k:
    *       1. Molecular Weight (k)
    *       2. vbar (k)
    *       3. Virial Coefficient (k)
    *
    *   for each scan:
    *      for each component:
    *         4. Amplitude
    *      5. baseline
    *
    *   for each association constant:
    *      6. association constant
    */
   unsigned int i = 0, j, k;
   for (k=0; k<(*runfit).components; k++)
   {
      if ((*runfit).mw_fit[k])
      {
         guess_par[i] = (*runfit).mw[k];
         (*runfit).mw_index[k] = i;
         i++;
      }
      if ((*runfit).vbar20_fit[k])
      {
         guess_par[i] = (*runfit).vbar20[k];
         (*runfit).vbar20_index[k] = i;
         i++;
      }
      if ((*runfit).virial_fit[k])
      {
         guess_par[i] = (*runfit).virial[k];
         (*runfit).virial_index[k] = i;
         i++;
      }
   }
   for (j=0; j<(*scanfit_vector).size(); j++)
   {
      if ((*scanfit_vector)[j].FitScanFlag)
      {
         for (k=0; k<(*runfit).components; k++)
         {
            if ((*scanfit_vector)[j].amplitude_fit[k])
            {
               guess_par[i] = (*scanfit_vector)[j].amplitude[k];
               (*scanfit_vector)[j].amplitude_index[k] = i;
               i++;
            }
         }
         if ((*scanfit_vector)[j].baseline_fit)
         {
            guess_par[i] = (*scanfit_vector)[j].baseline;
            (*scanfit_vector)[j].baseline_index = i;
            i++;
         }
      }
   }
   for (k=0; k<(*runfit).association_constants; k++)
   {
      if ((*runfit).eq_fit[k])
      {
         guess_par[i] = (*runfit).eq[k];
         (*runfit).eq_index[k] = i;
         i++;
      }
   }
   /*
     cout << "parameter mapping: \n";
     for (i=0; i<(*scanfit_vector).size(); i++)
     {
     if ((*scanfit_vector)[i].FitScanFlag)
     {
     cout    << "Amplitude: " << (*scanfit_vector)[i].amplitude_index[0]
     << ", Molecular weight: " << (*runfit).mw_index[0]
     << ", Equilibrium: " << (*runfit).eq_index[0]
     << ", Baseline: " << (*scanfit_vector)[i].baseline_index << endl;
     }
     }
   */
}

// use this function to attempt hard-adjusting parameters that turned into unreasonable values
bool US_EqFitter::try_adjustment()
{
   unsigned int i = 0, j, k;
   float target;
   for (k=0; k<(*runfit).components; k++)
   {
      if ((*runfit).mw_fit[k])
      {
         //         (*runfit).mw[k] = guess_par[i];
         i++;
      }
      if ((*runfit).vbar20_fit[k])
      {
         //         (*runfit).vbar20[k] = guess_par[i];
         i++;
      }
      if ((*runfit).virial_fit[k])
      {
         //         (*runfit).virial[k] = guess_par[i];
         i++;
      }
   }
   for (j=0; j<(*scanfit_vector).size(); j++)
   {
      if ((*scanfit_vector)[j].FitScanFlag)
      {
         for (k=0; k<(*runfit).components; k++)
         {
            if ((*scanfit_vector)[j].amplitude_fit[k])
            {
               if ((*scanfit_vector)[j].amplitude[k] > 50)
               {
                  //cout << "j: " << j << ", k: " << k << ", amplitude: " << (*scanfit_vector)[j].amplitude[k] << endl;
                  (*scanfit_vector)[j].amplitude[k] = -1.0;
               }
               i++;
            }
         }
         if ((*scanfit_vector)[j].baseline_fit)
         {
            //            (*scanfit_vector)[j].baseline = guess_par[i];
            i++;
         }
      }
   }
   for (k=0; k<(*runfit).association_constants; k++)
   {
      if ((*runfit).eq_fit[k])
      {
         if (k == 0 && (*runfit).eq[k] > 20)
         {
            (*runfit).eq[k] = 8;
         }
         i++;
      }
   }
   target = calc_residuals()/points;
   if (target <= 0)
   {
      return (false);
   }
   else
   {
      return (true);
   }
}

void US_EqFitter::parameter_mapBackward(double *guess_par)
{
   unsigned int i = 0, j, k;
   for (k=0; k<(*runfit).components; k++)
   {
      if ((*runfit).mw_fit[k])
      {
         (*runfit).mw[k] = guess_par[i];
         i++;
      }
      if ((*runfit).vbar20_fit[k])
      {
         (*runfit).vbar20[k] = guess_par[i];
         i++;
      }
      if ((*runfit).virial_fit[k])
      {
         (*runfit).virial[k] = guess_par[i];
         i++;
      }
   }
   for (j=0; j<(*scanfit_vector).size(); j++)
   {
      if ((*scanfit_vector)[j].FitScanFlag)
      {
         for (k=0; k<(*runfit).components; k++)
         {
            if ((*scanfit_vector)[j].amplitude_fit[k])
            {
               (*scanfit_vector)[j].amplitude[k] = guess_par[i];
               i++;
            }
         }
         if ((*scanfit_vector)[j].baseline_fit)
         {
            (*scanfit_vector)[j].baseline = guess_par[i];
            i++;
         }
      }
   }
   for (k=0; k<(*runfit).association_constants; k++)
   {
      if ((*runfit).eq_fit[k])
      {
         (*runfit).eq[k] = guess_par[i];
         i++;
      }
   }
   if (which_model == 3)
   {
      for (j=first_fitted_scan + 1; j<(*scanfit_vector).size(); j++)
      {
         if ((*scanfit_vector)[j].FitScanFlag)
         {
            for (k=0; k<(*runfit).components; k++)
            {
               (*scanfit_vector)[j].amplitude[k] = (*scanfit_vector)[first_fitted_scan].amplitude[k];
            }
         }
      }
   }
   emit newParameters();
}

void US_EqFitter::parameter_addRandomNoise(double *guess_par, float percent)
{
   unsigned int i = 0, j, k;
   for (k=0; k<(*runfit).components; k++)
   {
      if ((*runfit).mw_fit[k])
      {
         (*runfit).mw[k] = guess_par[i] + box_muller(0, percent * fabs(guess_par[i])/100);
         i++;
      }
      if ((*runfit).vbar20_fit[k])
      {
         (*runfit).vbar20[k] = guess_par[i] + box_muller(0, percent * fabs(guess_par[i])/100);
         i++;
      }
      if ((*runfit).virial_fit[k])
      {
         (*runfit).virial[k] = guess_par[i] + box_muller(0, percent * fabs(guess_par[i])/100);
         i++;
      }
   }
   for (j=0; j<(*scanfit_vector).size(); j++)
   {
      if ((*scanfit_vector)[j].FitScanFlag)
      {
         for (k=0; k<(*runfit).components; k++)
         {
            if ((*scanfit_vector)[j].amplitude_fit[k])
            {
               (*scanfit_vector)[j].amplitude[k] = guess_par[i] + box_muller(0, percent * fabs(guess_par[i])/100);
               i++;
            }
         }
         if ((*scanfit_vector)[j].baseline_fit)
         {
            (*scanfit_vector)[j].baseline = guess_par[i] + box_muller(0, percent * fabs(guess_par[i])/100);
            i++;
         }
      }
   }
   for (k=0; k<(*runfit).association_constants; k++)
   {
      if ((*runfit).eq_fit[k])
      {
         (*runfit).eq[k] = guess_par[i] + box_muller(0, percent * fabs(guess_par[i])/100);
         i++;
      }
   }
   if (which_model == 3)
   {
      for (j=first_fitted_scan + 1; j<(*scanfit_vector).size(); j++)
      {
         if ((*scanfit_vector)[j].FitScanFlag)
         {
            for (k=0; k<(*runfit).components; k++)
            {
               (*scanfit_vector)[j].amplitude[k] = (*scanfit_vector)[first_fitted_scan].amplitude[k];
            }
         }
      }
   }
   emit newParameters();
}

void US_EqFitter::startFit()
{
   QString str;
   if((nlsMethod == 4) && (which_model == 3))
   {
      return_value = GLLS_individual();
      return;
   }
   if((nlsMethod == 5) && (which_model == 3))
   {
      return_value = NNLS_individual();
      return;
   }
   else
   {
      if (GUI)
      {
         if ((which_model >= 0) && (which_model <= 3))
         {
            str.sprintf(tr("Working on Equilibrium Fit for ") + modelString[which_model] + "...");
            lbl_status4->setText(str);
         }
         else if ((which_model >= 4) && (which_model <= 14))
         {
            str.sprintf(tr("Stage 1: Converging on MW for ") + modelString[which_model] + "...");
            lbl_status4->setText(str);
         }
      }
      if (autoconverge)
      {
         iteration = 1000;
         while (iteration > 1) //
         {
            return_value = Fit(); // don't bother checking return values...
            if (nlsMethod == 3)   // switch between Levenberg Marquardt and Quasi Newton
            {
               nlsMethod = 0;
               if (GUI)
               {
                  lbl_status4->setText("Switching to Levenberg-Marquardt...");
                  cbb_nlsMethod->setCurrentItem(0);
               }
            }
            else
            {
               nlsMethod = 3;
               if (GUI)
               {
                  lbl_status4->setText("Switching to Quasi-Newton...");
                  cbb_nlsMethod->setCurrentItem(3);
               }
            }
         }
      }
      else // just do a single fit
      {
         return_value = Fit();
         if (return_value != 0)
         {
            QString temp_str;
            temp_str.sprintf(tr("The fit completed with a non-zero return value!\nThe value returned was: %d"), return_value);
            if (GUI)
            {
               QMessageBox::message(tr("Attention:"), temp_str);
            }
            else
            {
               cout << temp_str.latin1() << endl;
            }
         }
      }
   }
}

int US_EqFitter::GLLS_global()
{
   QString str;
   if (converged || completed || aborted)
   {
      cleanup();
   }
   first_plot = true;      // reset first plot each time we do a new fit
   completed = false;
   aborted = false;
   converged = false;
   if (GUI)
   {
      pb_cancel->setText(tr("Abort"));
      pb_fit->setEnabled(false);
      pb_pause->setEnabled(true);
      qApp->processEvents();
   }
   if (!fit_init())   // initialize the fitting process
   {
      if (GUI)
      {
         QMessageBox::message(tr("Please Note:"), tr("No scans have been selected\n"
                                                     "for fitting or all scans have\n"
                                                     "been excluded.\n\n"
                                                     "Please review the Scan Diagnostics\n"
                                                     "and check the scans for fit before\n"
                                                     "proceeding."));
         lbl_status2->setText(tr("Fit aborted..."));
         lbl_status3->setText("");
         pb_cancel->setText(tr("Close"));
         pb_fit->setEnabled(true);
         pb_pause->setEnabled(false);
      }
      aborted = true;
      return(-2);
   }
   if (GUI)
   {
      str.sprintf(" %d", parameters);
      lbl_parameters2->setText(str);
      str.sprintf(" %ld", datasets);
      lbl_datasets2->setText(str);
      str.sprintf(" %ld", points);
      lbl_points2->setText(str);
   }
   if (aborted)
   {
      suspend_flag = false;
      return(-3);
   }
   if (GUI)
   {
      pb_saveFit->setEnabled(true);
      pb_report->setEnabled(true);
      pb_residuals->setEnabled(true);
      pb_overlays->setEnabled(true);
      pb_print->setEnabled(true);
      bt_plotAll->setEnabled(true);
      bt_plotGroup->setEnabled(true);
      bt_plotSingle->setEnabled(true);
      pgb_progress->reset();
      pgb_progress->setTotalSteps(datasets);
   }
   double **M, **A, *B;
   unsigned int i, j, k, order, dataset_counter, point_counter;
   float x, xm_sqr, omega_sqr, *vbar, *buoyancy, constant;
   vbar      = new float[(*runfit).components];
   buoyancy   = new float[(*runfit).components];
   order = (*runfit).components + 1;
   A = new double *[order];
   B = new double [order];
   for (i=0; i<order; i++)
   {
      A[i] = new double [order];
   }
   for (i=0; i<order; i++)
   {
      for (j=0; j<order; j++)
      {
         A[i][j] = 0.0;
      }
   }
   dataset_counter = 0;
   point_counter = 0;
   vector<float>::iterator yval_it;
   M = new double *[points]; //set up a matrix that can be decomposed for general linear least squares
   for (j=0; j<points; j++)
   {
      M[j] = new double [order];
   }
   for (i=0; i<(*scanfit_vector).size(); i++)
   {
      if ((*scanfit_vector)[i].FitScanFlag)
      {
         xm_sqr = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2);
         omega_sqr = pow(M_PI * (*scanfit_vector)[i].rpm / 30, 2);
         constant = omega_sqr/(2 * R * (K0 + (*scanfit_vector)[i].temperature));
         for (k=0; k<(*runfit).components; k++)
         {
            vbar[k] = adjust_vbar20((*runfit).vbar20[k], (*scanfit_vector)[i].temperature);
            buoyancy[k] = (1.0 - vbar[k] * (*scanfit_vector)[i].density);
         }
         yval_it = (*scanfit_vector)[i].y.begin() + (*scanfit_vector)[i].start_index;
         for (j=0; j<points_per_dataset[dataset_counter]; j++)
         {
            if (j == 0)
            {
               x = 0.0;
            }
            else
            {
               x = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j], 2) - xm_sqr;
            }
            M[point_counter][0] = 1.0;
            for (k=0; k<(*runfit).components; k++)
            {
               M[point_counter][k+1] = exp ((*runfit).mw[k] * constant * x * buoyancy[k]);
            }
            y_raw[point_counter] = *yval_it;
            yval_it++;
            point_counter++;
         }
         dataset_counter++;
      }
   }
   // generate matrix for decomposition (only lower triangular is needed for Cholesky)
   A[0][0] = (double) points;
   for (i=1; i<order; i++)
   {
      for (j=0; j<=i; j++)
      {
         A[i][j] = 0;
         for (k=0; k<points; k++)
         {
            A[i][j] += M[k][i] * M[k][j];
         }
      }
   }
   // calculate right hand side:
   for (j=0; j<order; j++)
   {
      B[j] = 0.0;
      for (k=0; k<points; k++)
      {
         B[j] += y_raw[k] * M[k][j];
      }
   }
   Cholesky_Decomposition(A, order);
   Cholesky_SolveSystem(A, B, order);
   // B now contains the solution (the linear combination multipliers for each exponential term:
   (*scanfit_vector)[first_fitted_scan].baseline = B[0]; // the zeroth-order term is the baseline
   for (j=1; j<order; j++)
   {
      (*scanfit_vector)[first_fitted_scan].amplitude[j-1] = B[j]; // and the amplitudes
   }
   dataset_counter = 0;
   point_counter = 0;
   for (i=0; i<(*scanfit_vector).size(); i++)
   {
      if ((*scanfit_vector)[i].FitScanFlag)
      {
         xm_sqr = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2);
         omega_sqr = pow(M_PI * (*scanfit_vector)[i].rpm / 30, 2);
         constant = omega_sqr/(2 * R * (K0 + (*scanfit_vector)[i].temperature));
         for (k=0; k<(*runfit).components; k++)
         {
            vbar[k] = adjust_vbar20((*runfit).vbar20[k], (*scanfit_vector)[i].temperature);
            buoyancy[k] = (1.0 - vbar[k] * (*scanfit_vector)[i].density);
         }
         for (j=0; j<points_per_dataset[dataset_counter]; j++)
         {
            if (j == 0)
            {
               x = 0.0;
            }
            else
            {
               x = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j], 2) - xm_sqr;
            }
            y_guess[point_counter] = B[0];
            for (k=0; k<(*runfit).components; k++)
            {
               y_guess[point_counter] += B[k+1] * exp ((*runfit).mw[k] * constant * x * buoyancy[k]);
            }
            point_counter++;
         }
         dataset_counter++;
         if (GUI)
         {
            pgb_progress->setProgress(dataset_counter);
         }
      }
   }
   for (j=0; j<points; j++)
   {
      delete [] M[j];
   }
   delete [] M;
   if (GUI)
   {
      plot_overlays();
   }
   variance = calc_residuals()/points;
   emit currentStatus("Converged");
   emit hasConverged();
   converged = true;
   if (GUI)
   {
      pb_cancel->setText(tr("Close"));
      pb_fit->setEnabled(true);
      lbl_status2->setText(tr("The fitting process converged..."));
   }
   for (unsigned int i=0; i<order; i++)
   {
      delete [] A[i];
   }
   delete [] vbar;
   delete [] buoyancy;
   delete [] A;
   delete [] B;
   return(0);
}


int US_EqFitter::GLLS_individual()
{
   QString str;
   update_nlsMethod(4);
   first_plot = true;      // reset first plot each time we do a new fit
   completed = false;
   aborted = false;
   converged = false;
   if (GUI)
   {
      pb_cancel->setText(tr("Abort"));
      pb_fit->setEnabled(false);
      pb_pause->setEnabled(true);
      qApp->processEvents();
   }
   if (!fit_init())   // initialize the fitting process
   {
      if(GUI)
      {
         QMessageBox::message(tr("Please Note:"), tr("No scans have been selected\n"
                                                     "for fitting or all scans have\n"
                                                     "been excluded.\n\n"
                                                     "Please review the Scan Diagnostics\n"
                                                     "and check the scans for fit before\n"
                                                     "proceeding."));
         lbl_status2->setText(tr("Fit aborted..."));
         lbl_status3->setText("");
         pb_cancel->setText(tr("Close"));
         pb_fit->setEnabled(true);
         pb_pause->setEnabled(false);
      }
      aborted = true;
      return(-2);
   }
   if (aborted)
   {
      suspend_flag = false;
      return(-3);
   }
   if (GUI)
   {
      str.sprintf(" %d", parameters);
      lbl_parameters2->setText(str);
      str.sprintf(" %ld", datasets);
      lbl_datasets2->setText(str);
      str.sprintf(" %ld", points);
      lbl_points2->setText(str);
      pb_saveFit->setEnabled(true);
      pb_report->setEnabled(true);
      pb_residuals->setEnabled(true);
      pb_overlays->setEnabled(true);
      pb_print->setEnabled(true);
      bt_plotAll->setEnabled(true);
      bt_plotGroup->setEnabled(true);
      bt_plotSingle->setEnabled(true);
      pgb_progress->reset();
      pgb_progress->setTotalSteps(datasets);
   }
   double **M, **A, *B;
   unsigned int i, j, k, l, order, dataset_counter, point_counter, point_counter1, log_counter;
   order = (*runfit).components + 1;
   A = new double *[order];
   B = new double [order];
   float x, xm_sqr, omega_sqr, *vbar, *buoyancy, constant, argument;
   vbar      = new float[(*runfit).components];
   buoyancy   = new float[(*runfit).components];
   for (i=0; i<order; i++)
   {
      A[i] = new double [order];
   }
   for (i=0; i<order; i++)
   {
      for (j=0; j<order; j++)
      {
         A[i][j] = 0.0;
      }
   }
   dataset_counter = 0;
   point_counter = 0;
   point_counter1 = 0;
   vector<float>::iterator yval_it;
   parameters = 0;
   logpoints_per_dataset.clear();
   for (i=0; i<(*scanfit_vector).size(); i++)
   {
      if ((*scanfit_vector)[i].FitScanFlag)
      {
         M = new double *[points_per_dataset[dataset_counter]];
         for (j=0; j<points_per_dataset[dataset_counter]; j++)
         {
            M[j] = new double [order];
         }
         xm_sqr = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2);
         omega_sqr = pow(M_PI * (*scanfit_vector)[i].rpm / 30, 2);
         constant = omega_sqr/(2 * R * (K0 + (*scanfit_vector)[i].temperature));
         for (k=0; k<(*runfit).components; k++)
         {
            vbar[k] = adjust_vbar20((*runfit).vbar20[k], (*scanfit_vector)[i].temperature);
            buoyancy[k] = (1.0 - vbar[k] * (*scanfit_vector)[i].density);
         }
         yval_it = (*scanfit_vector)[i].y.begin() + (*scanfit_vector)[i].start_index;
         for (j=0; j<points_per_dataset[dataset_counter]; j++)
         {
            if (j == 0)
            {
               x = 0.0;
            }
            else
            {
               x = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j], 2) - xm_sqr;
            }
            M[j][0] = 1.0;
            for (k=0; k<(*runfit).components; k++)
            {
               M[j][k+1] = exp ((*runfit).mw[k] * constant * x * buoyancy[k]);
            }
            y_raw[point_counter1] = *yval_it;
            point_counter1++;
            yval_it++;
         }
         A[0][0] = (double) points_per_dataset[dataset_counter];
         for (l=1; l<order; l++)
         {
            for (j=0; j<=l; j++)
            {
               A[l][j] = 0;
               for (k=0; k<points_per_dataset[dataset_counter]; k++)
               {
                  A[l][j] += M[k][l] * M[k][j];
               }
            }
         }
         for (j=0; j<order; j++)
         {
            B[j] = 0.0;
            yval_it = (*scanfit_vector)[i].y.begin() + (*scanfit_vector)[i].start_index;
            parameters++;
            for (k=0; k<points_per_dataset[dataset_counter]; k++)
            {
               B[j] += *yval_it * M[k][j];
               yval_it++;
            }
         }
         Cholesky_Decomposition(A, order);
         Cholesky_SolveSystem(A, B, order);
         (*scanfit_vector)[i].baseline = B[0];
         for (j=1; j<order; j++)
         {
            (*scanfit_vector)[i].amplitude[j-1] = B[j];
         }
         log_counter = 0;
         for (j=0; j<points_per_dataset[dataset_counter]; j++)
         {
            //         cout << j << " " ;
            if (j == 0)
            {
               x = 0.0;
            }
            else
            {
               x = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j], 2) - xm_sqr;
            }
            y_guess[point_counter] = B[0];
            dcr2[dataset_counter][j] = 0.0;
            dlncr2[dataset_counter][j] = 0.0;
            for (k=0; k<(*runfit).components; k++)
            {
               y_guess[point_counter] += B[k+1] * exp ((*runfit).mw[k] * constant * x * buoyancy[k]);
            }
            argument = y_guess[point_counter] - B[0];
            if (argument > 0)
            {
               lncr2[dataset_counter][log_counter] = log(argument);
               log_counter ++;
            }
            point_counter++;
         }
         //cout << "Log counter: " << log_counter << ", dataset: " << dataset_counter <<endl;
         logpoints_per_dataset.push_back(log_counter);
         for (j=0; j<points_per_dataset[dataset_counter]; j++)
         {
            delete [] M[j];
         }
         delete [] M;
         dataset_counter++;
         if (GUI)
         {
            pgb_progress->setProgress(dataset_counter);
         }
      }
   }
   variance = calc_residuals()/point_counter;
   calc_dlncr2();
   if (GUI)
   {
      plot_residuals();
      pb_cancel->setText(tr("Close"));
      pb_fit->setEnabled(true);
      pb_plottwo->setEnabled(true);
      pb_plotthree->setEnabled(true);
      pb_plotfour->setEnabled(true);
      pb_fit->setEnabled(true);
      pb_plottwo->setText(tr("Ln(C) vs R^2"));
      pb_plotthree->setText(tr("MW vs R^2"));
      pb_plotfour->setText(tr("MW vs C"));
      write_report();
      qApp->processEvents();
      update_fitDialog();
      lbl_status2->setText(tr("The fitting process converged..."));
   }
   str.sprintf("%d", parameters);
   lbl_parameters2->setText(str);
   str.sprintf("%3.5e", variance);
   lbl_variance2->setText(str);
   str.sprintf("%3.5e", pow((double) variance, (double) 0.5));
   lbl_stddev2->setText(str);
   lbl_improvement2->setText("0");
   lbl_iteration2->setText("1");
   lbl_decompositions2->setText("1");
   lbl_evaluations2->setText("1");
   lbl_currentLambda2->setText("0");
   emit currentStatus("Converged");
   emit hasConverged();
   //cout << "setting converged to true in GLLS_individual()\n";
   converged = true;

   for (unsigned int i=0; i<order; i++)
   {
      delete [] A[i];
   }
   delete [] vbar;
   delete [] buoyancy;
   delete [] A;
   delete [] B;
   return(0);
}

int US_EqFitter::NNLS_individual()
{
   if (converged || completed || aborted)
   {
      cleanup();
   }
   QString str;
   first_plot = true;      // reset first plot each time we do a new fit
   completed = false;
   aborted = false;
   converged = false;
   //   bool signed_baseline = false; // false is negative
   if (GUI)
   {
      pb_cancel->setText(tr("Abort"));
      pb_fit->setEnabled(false);
      pb_pause->setEnabled(true);
      qApp->processEvents();
   }
   if (!fit_init())   // initialize the fitting process
   {
      if(GUI)
      {
         QMessageBox::message(tr("Please Note:"), tr("No scans have been selected\n"
                                                     "for fitting or all scans have\n"
                                                     "been excluded.\n\n"
                                                     "Please review the Scan Diagnostics\n"
                                                     "and check the scans for fit before\n"
                                                     "proceeding."));
         lbl_status2->setText(tr("Fit aborted..."));
         lbl_status3->setText("");
         pb_cancel->setText(tr("Close"));
         pb_fit->setEnabled(true);
         pb_pause->setEnabled(false);
      }
      aborted = true;
      return(-2);
   }
   if (aborted)
   {
      suspend_flag = false;
      return(-3);
   }
   if (GUI)
   {
      str.sprintf(" %d", parameters);
      lbl_parameters2->setText(str);
      str.sprintf(" %ld", datasets);
      lbl_datasets2->setText(str);
      str.sprintf(" %ld", points);
      lbl_points2->setText(str);
      pb_saveFit->setEnabled(true);
      pb_report->setEnabled(true);
      pb_residuals->setEnabled(true);
      pb_overlays->setEnabled(true);
      pb_print->setEnabled(true);
      bt_plotAll->setEnabled(true);
      bt_plotGroup->setEnabled(true);
      bt_plotSingle->setEnabled(true);
      pgb_progress->reset();
      pgb_progress->setTotalSteps(datasets);
   }
   unsigned int i, j, k;
   int order, dataset_counter;
   int counter1; // points for all datasets (used with logs)
   int counter2; // points for all datasets (used with y_raw)
   int counter3; // points for all datasets (used with y_guess)
   int counter4; // points for all datasets times order (used with nnls_a)
   order = (*runfit).components + 1; // plus 1 for pos or neg baseline
   float x, xm_sqr, omega_sqr, *vbar, *buoyancy, constant, argument;
   vbar      = new float[(*runfit).components];
   buoyancy   = new float[(*runfit).components];
   dataset_counter = 0;
   counter1 = 0;
   counter2 = 0;
   counter3 = 0;
   vector<float>::iterator yval_it;
   parameters = 0;
   logpoints_per_dataset.clear();
   if (GUI)
   {
      lbl_status2->setText(tr("Calculating NNLS solutions..."));
      qApp->processEvents();
   }
   for (i=0; i<(*scanfit_vector).size(); i++)
   {
      if ((*scanfit_vector)[i].FitScanFlag)
      {
         double *nnls_a, *nnls_b, *nnls_x, nnls_rnorm, *nnls_wp, *nnls_zzp;
         int *nnls_indexp;
         nnls_a = new double [points_per_dataset[dataset_counter] * order]; // contains the model functions, end-to-end
         nnls_b = new double [points_per_dataset[dataset_counter]]; // contains the experimental data
         nnls_zzp = new double [points_per_dataset[dataset_counter]]; // pre-allocated working space for nnls
         nnls_x = new double [order]; // the solution vector, pre-allocated for nnls
         nnls_wp = new double [order]; // pre-allocated working space for nnls, On exit, wp[] will contain the dual solution vector, wp[i]=0.0 for all i in set p and wp[i]<=0.0 for all i in set z.
         nnls_indexp = new int [order];
         xm_sqr = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2);
         omega_sqr = pow(M_PI * (*scanfit_vector)[i].rpm / 30, 2);
         constant = omega_sqr/(2 * R * (K0 + (*scanfit_vector)[i].temperature));
         for (k=0; k<(*runfit).components; k++)
         {
            vbar[k] = adjust_vbar20((*runfit).vbar20[k], (*scanfit_vector)[i].temperature);
            buoyancy[k] = (1.0 - vbar[k] * (*scanfit_vector)[i].density);
         }
         yval_it = (*scanfit_vector)[i].y.begin() + (*scanfit_vector)[i].start_index;
         counter4 = 0;
         for (j=0; j<points_per_dataset[dataset_counter]; j++)
         {
            y_raw[counter2] = *yval_it;
            nnls_b[j] = *yval_it+120;
            counter2++;
            yval_it++;
            nnls_a[counter4] = 1.0; // pos baseline term (i.e, exp[MW=0]), used in first column
            counter4++;
         }
         for (k=0; k<(*runfit).components; k++)
         {
            for (j=0; j<points_per_dataset[dataset_counter]; j++)
            {
               if (j == 0)
               {
                  x = 0.0;
               }
               else
               {
                  x = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j], 2) - xm_sqr;
               }
               // for each equilibrium scan, calculate a linear combination with non-negative amplitudes:
               nnls_a[counter4] = exp ((*runfit).mw[k] * constant * x * buoyancy[k]);
               counter4++;
            }
         }

         nnls(nnls_a, points_per_dataset[dataset_counter], points_per_dataset[dataset_counter], order,
              nnls_b, nnls_x, &nnls_rnorm, nnls_wp, nnls_zzp, nnls_indexp);
         (*scanfit_vector)[i].baseline = nnls_x[0] - 120; // adjust (+) baseline term
         for (int l=1; l<order; l++)
         {
            //cout << "l: " << l << ", nnls_x: " << nnls_x[l] << ", amplitude: " ;
            if (nnls_x[l] > 0.0)
            {
               (*scanfit_vector)[i].amplitude[l-1] = nnls_x[l];
            }
            else
            {
               (*scanfit_vector)[i].amplitude[l-1] = 0.0;
            }
            //cout << (*scanfit_vector)[i].amplitude[l-1] << endl;
         }

         counter3 = 0;
         for (j=0; j<points_per_dataset[dataset_counter]; j++)
         {
            if (j == 0)
            {
               x = 0.0;
            }
            else
            {
               x = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j], 2) - xm_sqr;
            }
            y_guess[counter1] = (*scanfit_vector)[i].baseline;
            dcr2[dataset_counter][j] = 0.0;
            dlncr2[dataset_counter][j] = 0.0;
            for (k=0; k<(*runfit).components; k++)
            {
               if (nnls_x[k+1] > 0.0) // no need to calculate if amplitude is set to zero
               {
                  y_guess[counter1] += nnls_x[k+1] * exp ((*runfit).mw[k] * constant * x * buoyancy[k]);
               }
            }
            argument = y_guess[counter1] - (*scanfit_vector)[i].baseline;
            if (argument > 1e-10)
            {
               lncr2[dataset_counter][counter3] = log(argument);
               counter3 ++;
            }
            counter1++;
         }
         logpoints_per_dataset.push_back(counter3);
         delete [] nnls_a;
         delete [] nnls_b;
         delete [] nnls_zzp;
         delete [] nnls_x;
         delete [] nnls_wp;
         delete [] nnls_indexp;

         dataset_counter++;
         if (GUI)
         {
            pgb_progress->setProgress(dataset_counter);
            lbl_status4->setText(tr(str.sprintf("Working on data set %d...", dataset_counter)));
            qApp->processEvents();
         }
      }
   }
   if (GUI)
   {
      lbl_status4->setText("");
      lbl_status2->setText(tr("Calculating Residuals..."));
      qApp->processEvents();
   }
   variance = calc_residuals()/counter1;
   calc_dlncr2();
   if (GUI)
   {
      plot_residuals();
      pb_cancel->setText(tr("Close"));
      pb_fit->setEnabled(true);
      pb_plottwo->setEnabled(true);
      pb_plotthree->setEnabled(true);
      pb_plotfour->setEnabled(true);
      pb_fit->setEnabled(true);
      pb_plottwo->setText(tr("Ln(C) vs R^2"));
      pb_plotthree->setText(tr("MW vs R^2"));
      pb_plotfour->setText(tr("MW vs C"));
      write_report();
      qApp->processEvents();
      update_fitDialog();
      lbl_status2->setText(tr("The fitting process converged..."));
      str.sprintf("%d", parameters);
      lbl_parameters2->setText(str);
      str.sprintf("%3.5e", variance);
      lbl_variance2->setText(str);
      str.sprintf("%3.5e", pow((double) variance, (double) 0.5));
      lbl_stddev2->setText(str);
      lbl_improvement2->setText("0");
      lbl_iteration2->setText("1");
      lbl_decompositions2->setText("1");
      lbl_evaluations2->setText("1");
      lbl_currentLambda2->setText("0");
   }
   emit currentStatus("Converged");
   emit hasConverged();
   //cout << "setting converged to true in NNLS_individual()\n";
   converged = true;
   delete [] vbar;
   delete [] buoyancy;
   return(0);
}

int US_EqFitter::calc_model(double *guess_par)
{
   errno = 0;
   bool noerror = true;
   parameter_mapBackward(guess_par);
   QString str;
   unsigned int i, j, k, dataset_counter = 0, point_counter = 0, log_counter;
   float x, xm_sqr, omega_sqr, *vbar, *buoyancy, constant, constant_x, OD_correction1, OD_correction2, argument;
   logpoints_per_dataset.clear();
   if (GUI)
   {
      str.sprintf(tr("Fitting ") + modelString[which_model] + "...");
      lbl_status2->setText(str);
      qApp->processEvents();
   }

   if ((which_model >= 0) && (which_model <= 3))
   {
      vbar      = new float[(*runfit).components];
      buoyancy   = new float[(*runfit).components];
      for (i=0; i<(*scanfit_vector).size(); i++)
      {
         if ((*scanfit_vector)[i].FitScanFlag)
         {
            xm_sqr = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2);
            omega_sqr = pow(M_PI * (*scanfit_vector)[i].rpm / 30, 2);
            constant = omega_sqr/(2 * R * (K0 + (*scanfit_vector)[i].temperature));
            for (k=0; k<(*runfit).components; k++)
            {
               vbar[k] = adjust_vbar20((*runfit).vbar20[k], (*scanfit_vector)[i].temperature);
               buoyancy[k] = (1.0 - vbar[k] * (*scanfit_vector)[i].density);
            }
            log_counter = 0;
            for (j=0; j<points_per_dataset[dataset_counter]; j++)
            {
               if (j == 0)
               {
                  x = 0.0;
               }
               else
               {
                  x = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j], 2) - xm_sqr;
               }
               y_guess[point_counter] = (*scanfit_vector)[i].baseline;
               for (k=0; k<(*runfit).components; k++)
               {
                  y_guess[point_counter] += exp ((*scanfit_vector)[i].amplitude[k]
                                                 + ((*runfit).mw[k] * constant * x * buoyancy[k]));
                  //cout << "yguess[" << point_counter << ", " <<k<< "]: " << y_guess[point_counter] << ", mw: " << (*runfit).mw[k] << ", a: " << (*scanfit_vector)[i].amplitude[k] << endl;
               }
               argument = y_guess[point_counter] - (*scanfit_vector)[i].baseline;
               if (!isnormal(argument))
               {
                  cout << "Argument 1: " << argument << endl;
                  noerror = false;
               }
               if (argument > 0)
               {
                  lncr2[dataset_counter][log_counter] = log(argument);
                  log_counter ++;
               }
               point_counter++;
            }
            logpoints_per_dataset.push_back(log_counter);
            if (GUI)
            {
               qApp->processEvents();
            }
            if(suspend_flag || aborted)
            {
               return(-1);
            }
            dataset_counter++;
         }
      }
      delete [] vbar;
      delete [] buoyancy;
   }
   else if ((which_model >= 4) && (which_model <= 10))
   {
      switch (which_model)
      {
      case 4: // Monomer-Dimer Equilibrium
         {
            (*runfit).stoich1 = 2;
            break;
         }
      case 5: // Monomer-Trimer Equilibrium
         {
            (*runfit).stoich1 = 3;
            break;
         }
      case 6: // Monomer-Tetramer Equilibrium
         {
            (*runfit).stoich1 = 4;
            break;
         }
      case 7: // Monomer-Pentamer Equilibrium
         {
            (*runfit).stoich1 = 5;
            break;
         }
      case 8: // Monomer-Hexamer Equilibrium
         {
            (*runfit).stoich1 = 6;
            break;
         }
      case 9: // Monomer-Heptamer Equilibrium
         {
            (*runfit).stoich1 = 7;
            break;
         }
      }
      vbar      = new float[1];
      buoyancy   = new float[1];
      for (unsigned int i=0; i<(*scanfit_vector).size(); i++)
      {
         if ((*scanfit_vector)[i].FitScanFlag)
         {
            xm_sqr = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2);
            omega_sqr = pow(M_PI * (*scanfit_vector)[i].rpm / 30, 2);
            constant = omega_sqr/(2 * R * (K0 + (*scanfit_vector)[i].temperature));
            vbar[0] = adjust_vbar20((*runfit).vbar20[0], (*scanfit_vector)[i].temperature);
            buoyancy[0] = (1.0 - vbar[0] * (*scanfit_vector)[i].density);
            OD_correction1 = log((*runfit).stoich1 / pow((*scanfit_vector)[i].extinction[0]
                                                         * (*scanfit_vector)[i].pathlength, (*runfit).stoich1-1));
            log_counter = 0;
            for (j=0; j<points_per_dataset[dataset_counter]; j++)
            {
               if (j == 0)
               {
                  x = 0.0;
               }
               else
               {
                  x = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j], 2) - xm_sqr;
               }
               y_guess[point_counter] = (*scanfit_vector)[i].baseline
                  + exp ((*scanfit_vector)[i].amplitude[0] + ((*runfit).mw[0] * constant * x * buoyancy[0]))
                  + exp ((*runfit).stoich1 * (*scanfit_vector)[i].amplitude[0] + (*runfit).eq[0]
                         + OD_correction1 + (*runfit).stoich1 * (*runfit).mw[0] * constant * x * buoyancy[0]);
               argument = y_guess[point_counter] - (*scanfit_vector)[i].baseline;
               if (!isnormal(argument))
               {
                  cout << "Argument 2: " << argument << endl;
                  noerror = false;
               }
               if (argument > 0)
               {
                  lncr2[dataset_counter][log_counter] = log(argument);
                  log_counter ++;
               }
               point_counter++;
            }
            logpoints_per_dataset.push_back(log_counter);
            if (GUI)
            {
               qApp->processEvents();
            }
            if(suspend_flag || aborted)
            {
               return(-1);
            }
            dataset_counter++;
         }
      }
      delete [] vbar;
      delete [] buoyancy;
   }
   else if ((which_model >= 11) && (which_model <= 13))
   {
      switch (which_model)
      {
      case 11: // Monomer-Dimer-Trimer Equilibrium
         {
            (*runfit).stoich1 = 2;
            (*runfit).stoich2 = 3;
            break;
         }
      case 12: // Monomer-Dimer-Tetramer Equilibrium
         {
            (*runfit).stoich1 = 2;
            (*runfit).stoich2 = 4;
            break;
         }
      }
      vbar      = new float[1];
      buoyancy   = new float[1];
      for (unsigned int i=0; i<(*scanfit_vector).size(); i++)
      {
         if ((*scanfit_vector)[i].FitScanFlag)
         {
            xm_sqr = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2);
            omega_sqr = pow(M_PI * (*scanfit_vector)[i].rpm / 30, 2);
            constant = omega_sqr/(2 * R * (K0 + (*scanfit_vector)[i].temperature));
            vbar[0] = adjust_vbar20((*runfit).vbar20[0], (*scanfit_vector)[i].temperature);
            buoyancy[0] = (1.0 - vbar[0] * (*scanfit_vector)[i].density);
            OD_correction1 = log((*runfit).stoich1 / pow((*scanfit_vector)[i].extinction[0]
                                                         * (*scanfit_vector)[i].pathlength, (*runfit).stoich1-1));
            OD_correction2 = log((*runfit).stoich2 / pow((*scanfit_vector)[i].extinction[0]
                                                         * (*scanfit_vector)[i].pathlength, (*runfit).stoich2-1));
            log_counter = 0;
            for (j=0; j<points_per_dataset[dataset_counter]; j++)
            {
               if (j == 0)
               {
                  x = 0.0;
               }
               else
               {
                  x = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j], 2) - xm_sqr;
               }
               y_guess[point_counter] = (*scanfit_vector)[i].baseline
                  + exp ((*scanfit_vector)[i].amplitude[0] + ((*runfit).mw[0] * constant * x * buoyancy[0]))
                  + exp ((*runfit).stoich1 * (*scanfit_vector)[i].amplitude[0] + (*runfit).eq[0]
                         + OD_correction1 + (*runfit).stoich1 * (*runfit).mw[0] * constant * x * buoyancy[0])
                  + exp ((*runfit).stoich2 * (*scanfit_vector)[i].amplitude[0] + (*runfit).eq[1]
                         + OD_correction2 + (*runfit).stoich2 * (*runfit).mw[0] * constant * x * buoyancy[0]);
               /*               if (errno != 0)
                                {
                                errno=0;
                                return(-2);
                                }*/
               argument = y_guess[point_counter] - (*scanfit_vector)[i].baseline;
               if (!isnormal(argument))
               {
                  cout << "Argument 3: " << argument << endl;
                  noerror = false;
               }
               if (argument > 0)
               {
                  lncr2[dataset_counter][log_counter] = log(argument);
                  log_counter ++;
               }
               point_counter++;
            }
            logpoints_per_dataset.push_back(log_counter);
            if (GUI)
            {
               qApp->processEvents();
            }
            if(suspend_flag || aborted)
            {
               return(-1);
            }
            dataset_counter++;
         }
      }
      delete [] vbar;
      delete [] buoyancy;
   }
   else if (which_model == 14)
   {
      float mw_ab, extinction_ab, vbar_ab, buoyancy_ab;
      vbar      = new float[(*runfit).components];
      buoyancy   = new float[(*runfit).components];
      mw_ab = (*runfit).mw[0] + (*runfit).mw[1]; // 3rd MW is constrained
      for (i=0; i<(*scanfit_vector).size(); i++)
      {
         if ((*scanfit_vector)[i].FitScanFlag)
         {
            xm_sqr = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2);
            omega_sqr = pow(M_PI * (*scanfit_vector)[i].rpm / 30, 2);
            constant = omega_sqr/(2 * R * (K0 + (*scanfit_vector)[i].temperature));
            for (k=0; k<2; k++) // 2 components are given, the third is calculated
            {
               vbar[k] = adjust_vbar20((*runfit).vbar20[k], (*scanfit_vector)[i].temperature);
               buoyancy[k] = (1.0 - vbar[k] * (*scanfit_vector)[i].density);
            }
            vbar_ab = ((*runfit).vbar20[0] * (*runfit).mw[0]
                       + (*runfit).vbar20[1] * (*runfit).mw[1])
               / mw_ab; // vbar_ab is a weight-average of vbar[1] and vbar[0]
            buoyancy_ab = (1.0 - vbar_ab * (*scanfit_vector)[i].density);
            log_counter = 0;
            // OD correction: e_AB/(e_A * e_B * l)
            // e_AB = (e_A + e_B)
            extinction_ab = (*scanfit_vector)[i].extinction[0] + (*scanfit_vector)[i].extinction[1];
            OD_correction1 = log(extinction_ab/((*scanfit_vector)[i].extinction[0] * (*scanfit_vector)[i].extinction[1] * (*scanfit_vector)[i].pathlength));
            log_counter = 0;
            for (j=0; j<points_per_dataset[dataset_counter]; j++)
            {
               if (j == 0)
               {
                  x = 0.0;
               }
               else
               {
                  x = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j], 2) - xm_sqr;
               }
               y_guess[point_counter] = (*scanfit_vector)[i].baseline;
               for (k=0; k<2; k++)
               {
                  y_guess[point_counter] += exp ((*scanfit_vector)[i].amplitude[k]
                                                 + ((*runfit).mw[k] * constant * x * buoyancy[k]));
               }
               y_guess[point_counter] += exp ((*scanfit_vector)[i].amplitude[0]
                                              + (*scanfit_vector)[i].amplitude[1]
                                              + (*runfit).eq[0] + OD_correction1
                                              + mw_ab * constant * x * buoyancy_ab);
               argument = y_guess[point_counter] - (*scanfit_vector)[i].baseline;
               if (!isnormal(argument))
               {
                  cout << "Argument 4: " << argument << endl;
                  noerror = false;
               }
               if (argument > 0)
               {
                  lncr2[dataset_counter][log_counter] = log(argument);
                  log_counter ++;
               }
               point_counter++;
            }
            logpoints_per_dataset.push_back(log_counter);
            if (GUI)
            {
               qApp->processEvents();
            }
            if(suspend_flag || aborted)
            {
               return(-1);
            }
            dataset_counter++;
         }
      }
      delete [] vbar;
      delete [] buoyancy;
   }
   else if (which_model == 15) // A + B <=> AB, SUM(1->n)[ A ] <=> An
   {
      float mw_ab, extinction_ab, vbar_ab, buoyancy_ab;
      vbar      = new float[(*runfit).components];
      buoyancy   = new float[(*runfit).components];
      mw_ab = (*runfit).mw[0] + (*runfit).mw[1]; // 3rd MW is constrained
      for (i=0; i<(*scanfit_vector).size(); i++)
      {
         if ((*scanfit_vector)[i].FitScanFlag)
         {
            xm_sqr = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2);
            omega_sqr = pow(M_PI * (*scanfit_vector)[i].rpm / 30, 2);
            constant = omega_sqr/(2 * R * (K0 + (*scanfit_vector)[i].temperature));
            for (k=0; k<2; k++) // 2 components are given, the third is calculated
            {
               vbar[k] = adjust_vbar20((*runfit).vbar20[k], (*scanfit_vector)[i].temperature);
               buoyancy[k] = (1.0 - vbar[k] * (*scanfit_vector)[i].density);
            }
            vbar_ab = ((*runfit).vbar20[0] * (*runfit).mw[0]
                       + (*runfit).vbar20[1] * (*runfit).mw[1])
               / mw_ab; // vbar_ab is a weight-average of vbar[1] and vbar[0]
            buoyancy_ab = (1.0 - vbar_ab * (*scanfit_vector)[i].density);
            log_counter = 0;
            // e_AB = (e_A + e_B)
            // OD correction: e_AB/(e_A * e_B * l)
            extinction_ab = (*scanfit_vector)[i].extinction[0] + (*scanfit_vector)[i].extinction[1];
            OD_correction1 = log(extinction_ab/((*scanfit_vector)[i].extinction[0] * (*scanfit_vector)[i].extinction[1] * (*scanfit_vector)[i].pathlength));
            OD_correction2 = log((*runfit).stoich1 / pow((*scanfit_vector)[i].extinction[0] * (*scanfit_vector)[i].pathlength, (*runfit).stoich1-1));
            log_counter = 0;
            for (j=0; j<points_per_dataset[dataset_counter]; j++)
            {
               if (j == 0)
               {
                  x = 0.0;
               }
               else
               {
                  x = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j], 2) - xm_sqr;
               }
               y_guess[point_counter] = (*scanfit_vector)[i].baseline;
               for (k=0; k<2; k++) //individual components A and B
               {
                  y_guess[point_counter] += exp ((*scanfit_vector)[i].amplitude[k]
                                                 + ((*runfit).mw[k] * constant * x * buoyancy[k]));
               }
               y_guess[point_counter] += exp ((*scanfit_vector)[i].amplitude[0] //heteroassociation
                                              + (*scanfit_vector)[i].amplitude[1]
                                              + (*runfit).eq[0] + OD_correction1
                                              + mw_ab * constant * x * buoyancy_ab)
                  + exp ((*runfit).stoich1 * (*scanfit_vector)[i].amplitude[0] //monomer-dimer for A
                         + (*runfit).eq[1] + OD_correction2
                         + (*runfit).stoich1 * (*runfit).mw[0] * constant * x * buoyancy[0]);
               argument = y_guess[point_counter] - (*scanfit_vector)[i].baseline;
               if (!isnormal(argument))
               {
                  cout << "Argument 5: " << argument << endl;
                  noerror = false;
               }
               if (argument > 0)
               {
                  lncr2[dataset_counter][log_counter] = log(argument);
                  log_counter ++;
               }
               point_counter++;
            }
            logpoints_per_dataset.push_back(log_counter);
            if (GUI)
            {
               qApp->processEvents();
            }
            if(suspend_flag || aborted)
            {
               return(-1);
            }
            dataset_counter++;
         }
      }
      delete [] vbar;
      delete [] buoyancy;
   }
   else if ((which_model == 16) || (which_model == 17))
   {
      vbar      = new float[1];
      buoyancy   = new float[1];
      if (which_model == 16)
      {
         (*runfit).mw[1] = (*runfit).mw[0];
      }
      else if (which_model == 17)
      {
         (*runfit).mw[1] = (*runfit).stoich1 * (*runfit).mw[0];
      }
      for (unsigned int i=0; i<(*scanfit_vector).size(); i++)
      {
         if ((*scanfit_vector)[i].FitScanFlag)
         {
            xm_sqr = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2);
            omega_sqr = pow(M_PI * (*scanfit_vector)[i].rpm / 30, 2);
            constant = omega_sqr/(2 * R * (K0 + (*scanfit_vector)[i].temperature));
            vbar[0] = adjust_vbar20((*runfit).vbar20[0], (*scanfit_vector)[i].temperature);
            buoyancy[0] = (1.0 - vbar[0] * (*scanfit_vector)[i].density);
            OD_correction1 = log((*runfit).stoich1 / pow((*scanfit_vector)[i].extinction[0]
                                                         * (*scanfit_vector)[i].pathlength, (*runfit).stoich1-1));
            log_counter = 0;
            for (j=0; j<points_per_dataset[dataset_counter]; j++)
            {
               if (j == 0)
               {
                  x = 0.0;
               }
               else
               {
                  x = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j], 2) - xm_sqr;
               }
               constant_x = constant * x;
               y_guess[point_counter] = (*scanfit_vector)[i].baseline
                  + exp ((*scanfit_vector)[i].amplitude[0] + ((*runfit).mw[0] * constant_x * buoyancy[0]))
                  + exp ((*scanfit_vector)[i].amplitude[1] + ((*runfit).mw[1] * constant_x * buoyancy[0]))
                  + exp ((*runfit).stoich1 * (*scanfit_vector)[i].amplitude[0] + (*runfit).eq[0]
                         + OD_correction1 + (*runfit).mw[1] * constant_x * buoyancy[0]);
               argument = y_guess[point_counter] - (*scanfit_vector)[i].baseline;
               if (!isnormal(argument))
               {
                  cout << "Argument 6: " << argument << endl;
                  noerror = false;
               }
               if (argument > 0)
               {
                  lncr2[dataset_counter][log_counter] = log(argument);
                  log_counter ++;
               }
               point_counter++;
            }
            logpoints_per_dataset.push_back(log_counter);
            if (GUI)
            {
               qApp->processEvents();
            }
            if(suspend_flag || aborted)
            {
               return(-1);
            }
            dataset_counter++;
         }
      }
      delete [] vbar;
      delete [] buoyancy;
   }
   else if (which_model == 18)
   {
      vbar      = new float[1];
      buoyancy   = new float[1];
      (*runfit).mw[1] = (*runfit).stoich1 * (*runfit).mw[0];
      for (unsigned int i=0; i<(*scanfit_vector).size(); i++)
      {
         if ((*scanfit_vector)[i].FitScanFlag)
         {
            xm_sqr = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2);
            omega_sqr = pow(M_PI * (*scanfit_vector)[i].rpm / 30, 2);
            constant = omega_sqr/(2 * R * (K0 + (*scanfit_vector)[i].temperature));
            vbar[0] = adjust_vbar20((*runfit).vbar20[0], (*scanfit_vector)[i].temperature);
            buoyancy[0] = (1.0 - vbar[0] * (*scanfit_vector)[i].density);
            OD_correction1 = log((*runfit).stoich1 / pow((*scanfit_vector)[i].extinction[0]
                                                         * (*scanfit_vector)[i].pathlength, (*runfit).stoich1-1));
            log_counter = 0;
            for (j=0; j<points_per_dataset[dataset_counter]; j++)
            {
               if (j == 0)
               {
                  x = 0.0;
               }
               else
               {
                  x = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j], 2) - xm_sqr;
               }
               y_guess[point_counter] = (*scanfit_vector)[i].baseline
                  + exp ((*scanfit_vector)[i].amplitude[0] + ((*runfit).mw[0] * constant * x * buoyancy[0]))
                  + exp ((*scanfit_vector)[i].amplitude[1] + ((*runfit).mw[1] * constant * x * buoyancy[0]));
               argument = y_guess[point_counter] - (*scanfit_vector)[i].baseline;
               if (!isnormal(argument))
               {
                  cout << "Argument 7: " << argument << endl;
                  noerror = false;
               }
               if (argument > 0)
               {
                  lncr2[dataset_counter][log_counter] = log(argument);
                  log_counter ++;
               }
               point_counter++;
            }
            logpoints_per_dataset.push_back(log_counter);
            if (GUI)
            {
               qApp->processEvents();
            }
            if(suspend_flag || aborted)
            {
               return(-1);
            }
            dataset_counter++;
         }
      }
      delete [] vbar;
      delete [] buoyancy;
   }
   else if (which_model == 19)
   {
      vbar      = new float[2];
      buoyancy   = new float[2];
      for (unsigned int i=0; i<(*scanfit_vector).size(); i++)
      {
         if ((*scanfit_vector)[i].FitScanFlag)
         {
            xm_sqr = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2);
            omega_sqr = pow(M_PI * (*scanfit_vector)[i].rpm / 30, 2);
            constant = omega_sqr/(2 * R * (K0 + (*scanfit_vector)[i].temperature));
            vbar[0] = adjust_vbar20((*runfit).vbar20[0], (*scanfit_vector)[i].temperature);
            vbar[1] = adjust_vbar20((*runfit).vbar20[1], (*scanfit_vector)[i].temperature);
            buoyancy[0] = (1.0 - vbar[0] * (*scanfit_vector)[i].density);
            buoyancy[1] = (1.0 - vbar[1] * (*scanfit_vector)[i].density);
            OD_correction1 = log((*runfit).stoich1 / pow((*scanfit_vector)[i].extinction[0]
                                                         * (*scanfit_vector)[i].pathlength, (*runfit).stoich1-1));
            log_counter = 0;
            for (j=0; j<points_per_dataset[dataset_counter]; j++)
            {
               if (j == 0)
               {
                  x = 0.0;
               }
               else
               {
                  x = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j], 2) - xm_sqr;
               }
               y_guess[point_counter] = (*scanfit_vector)[i].baseline
                  + exp ((*scanfit_vector)[i].amplitude[0] + ((*runfit).mw[0] * constant * x * buoyancy[0]))
                  + exp ((*scanfit_vector)[i].amplitude[1] + ((*runfit).mw[1] * constant * x * buoyancy[1]))
                  + exp ((*runfit).stoich1 * (*scanfit_vector)[i].amplitude[0] + (*runfit).eq[0]
                         + OD_correction1 + (*runfit).stoich1 * (*runfit).mw[0] * constant * x * buoyancy[0]);
               argument = y_guess[point_counter] - (*scanfit_vector)[i].baseline;
               if (!isnormal(argument))
               {
                  cout << "Argument 8: " << argument << endl;
                  noerror = false;
               }
               if (argument > 0)
               {
                  lncr2[dataset_counter][log_counter] = log(argument);
                  log_counter ++;
               }
               point_counter++;
            }
            logpoints_per_dataset.push_back(log_counter);
            if (GUI)
            {
               qApp->processEvents();
            }
            if(suspend_flag || aborted)
            {
               return(-1);
            }
            dataset_counter++;
         }
      }
      delete [] vbar;
      delete [] buoyancy;
   }
   function_evaluations++;
   if (GUI)
   {
      str.sprintf("%d", function_evaluations);
      lbl_evaluations2->setText(str);
      qApp->processEvents();
   }
   if (noerror)
   {
      cout << "errno is set to: " << errno << endl;
      return(0);
   }
   else
   {
      cout << "errno: " << errno <<  ", noerror: " << noerror << endl;
      return(-1); // a mathematical exception has occured
   }
}

int US_EqFitter::calc_jacobian()
{
   unsigned int i, j, k, point_counter = 0, dataset_counter = 0;
   float xm_sqr, x, omega_sqr, constant, constant_x, *u_function,
      *buoyancy, *vbar, OD_correction1, OD_correction2;
   QString str, str1;
   for (i=0; i<points; i++)
   {
      for (j=0; j<parameters; j++)
      {
         jacobian[i][j] = 0.0;
      }
   }
   if (GUI)
   {
      str.sprintf(tr("Calculating Jacobian for ") + modelString[which_model] + "...");
      lbl_status2->setText(str);
      qApp->processEvents();
   }
   if ((which_model >= 0) && (which_model <= 3))
   {
      /*
        Function:
        y = SUM [exp[ln(amplitude[i]) + MW[i] * w^2/2RT * (r^2 - r0^2) * (1 - vbar[i] * rho)] ] + baseline
        Substitutions:
        x = r^2  - r0^2
        Constant = omega^2/2RT
        u[i] = ln(amplitude[i]) + MW[i] * Constant * x * (1 - vbar[i] * rho)

        Partials:
        1:  dy/dMW[i] = dy/du[i] * du[i]/dMW[i] = exp[u[i]] * Constant * x * (1 - vbar[i] * rho)
        1a: dy/dvbar[i] = dy/du[i] * du[i]/dvbar[i] = exp[u[i]] * (-1) * MW[i] * Constant *  x * rho
        2: dy/dln(amplitude[i]) = dy/du[i] * du[i]/dln(amplitude[i]) = exp[u[i]]
        3: dy/dbaseline = 1.0
      */
      u_function   = new float [(*runfit).components];
      vbar         = new float [(*runfit).components];
      buoyancy      = new float [(*runfit).components];
      point_counter = 0;
      dataset_counter = 0;
      for (i=0; i<(*scanfit_vector).size(); i++)
      {
         if ((*scanfit_vector)[i].FitScanFlag)
         {
            xm_sqr = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2);
            omega_sqr = pow(M_PI * (*scanfit_vector)[i].rpm / 30, 2);
            constant = omega_sqr / (2.0 * R * (K0 + (*scanfit_vector)[i].temperature));
            for (k=0; k<(*runfit).components; k++)
            {
               vbar[k] = adjust_vbar20((*runfit).vbar20[k], (*scanfit_vector)[i].temperature);
               buoyancy[k] = (1.0 - vbar[k] * (*scanfit_vector)[i].density);
            }
            for (j=0; j<points_per_dataset[dataset_counter]; j++)
            {
               if (j == 0)
               {
                  x = 0.0;
               }
               else
               {
                  x = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j], 2) - xm_sqr;
               }
               for (k=0; k<(*runfit).components; k++)
               {
                  u_function[k] = (*scanfit_vector)[i].amplitude[k] + constant * (*runfit).mw[k] * buoyancy[k] * x;
                  if ((*runfit).mw_fit[k])
                  {
                     jacobian[point_counter][(*runfit).mw_index[k]] = constant * x * buoyancy[k] * exp(u_function[k]);
                  }
                  if ((*runfit).vbar20_fit[k])
                  {
                     jacobian[point_counter][(*runfit).vbar20_index[k]] = (-1.0) * constant
                        * (*runfit).mw[k] * x * exp(u_function[k]) * (*scanfit_vector)[i].density;
                  }
                  if ((*scanfit_vector)[i].amplitude_fit[k])
                  {
                     jacobian[point_counter][(*scanfit_vector)[i].amplitude_index[k]] = exp(u_function[k]);
                  }
               }
               if ((*scanfit_vector)[i].baseline_fit)
               {
                  jacobian[point_counter][(*scanfit_vector)[i].baseline_index] = 1.0;
               }
               point_counter++;
            }
            dataset_counter++;
         }
      }
      delete [] u_function;
      delete [] vbar;
      delete [] buoyancy;
   }
   else if ((which_model >= 4) && (which_model <= 10))
   {
      switch (which_model)
      {
      case 4: // Monomer-Dimer Equilibrium
         {
            (*runfit).stoich1 = 2;
            break;
         }
      case 5: // Monomer-Trimer Equilibrium
         {
            (*runfit).stoich1 = 3;
            break;
         }
      case 6: // Monomer-Tetramer Equilibrium
         {
            (*runfit).stoich1 = 4;
            break;
         }
      case 7: // Monomer-Pentamer Equilibrium
         {
            (*runfit).stoich1 = 5;
            break;
         }
      case 8: // Monomer-Hexamer Equilibrium
         {
            (*runfit).stoich1 = 6;
            break;
         }
      case 9: // Monomer-Heptamer Equilibrium
         {
            (*runfit).stoich1 = 7;
            break;
         }
      }
      /*
        Function:
        y = [exp[ln(amplitude[0]) + MW[0] * w^2/2RT * (r^2 - r0^2) * (1 - vbar[0] * rho)] ]
        + [exp[(*runfit).stoich1 * ln(amplitude[0]) + ln(K1-(*runfit).stoich1) + (*runfit).stoich1 * MW[0] * w^2/2RT * (r^2 - r0^2) * (1 - vbar[0] * rho)] ]
        + baseline

        Substitutions:
        x = r^2  - r0^2
        Constant = omega^2/2RT
        buoyancy = (1 - vbar[0] * rho)
        E = ln(k1-(*runfit).stoich1)
        u[0] = ln(amplitude[0]) + MW[0] * Constant * x * buoyancy
        u[1] = ln((*runfit).stoich1 * amplitude[0]) + E + (*runfit).stoich1 * MW[0] * Constant * x * buoyancy

        Partials:
        1:  dy/dMW = exp[u[0]] * Constant * x * buoyancy + exp[u[1]] * (*runfit).stoich1 * Constant * x * buoyancy

        1a: dy/dvbar = (-1) * exp[u[0]] * MW[0] * Constant *  x * rho - exp[u[1]] * (*runfit).stoich1 * MW[0] * Constant * x * rho

        2: dy/dln(amplitude[0]) = exp[u[0]] + (*runfit).stoich1 * exp[u[1]]

        3: dy/dE = exp[u[1]]

        4: dy/dbaseline = 1.0
      */
      u_function   = new float [2];
      vbar         = new float [1];
      buoyancy      = new float [1];
      point_counter = 0;
      dataset_counter = 0;
      for (i=0; i<(*scanfit_vector).size(); i++)
      {
         if ((*scanfit_vector)[i].FitScanFlag)
         {
            xm_sqr = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2);
            omega_sqr = pow(M_PI * (*scanfit_vector)[i].rpm / 30, 2);
            constant = omega_sqr / (2.0 * R * (K0 + (*scanfit_vector)[i].temperature));
            vbar[0] = adjust_vbar20((*runfit).vbar20[0], (*scanfit_vector)[i].temperature);
            buoyancy[0] = (1.0 - vbar[0] * (*scanfit_vector)[i].density);
            OD_correction1 = log((*runfit).stoich1 / pow((*scanfit_vector)[i].extinction[0]
                                                         * (*scanfit_vector)[i].pathlength, (*runfit).stoich1-1));
            for (j=0; j<points_per_dataset[dataset_counter]; j++)
            {
               if (j == 0)
               {
                  x = 0.0;
               }
               else
               {
                  x = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j], 2) - xm_sqr;
               }
               u_function[0] = (*scanfit_vector)[i].amplitude[0] +
                  constant * (*runfit).mw[0] * buoyancy[0] * x;
               u_function[1] = (*runfit).stoich1 * (*scanfit_vector)[i].amplitude[0] + (*runfit).eq[0] +
                  OD_correction1 + constant * (*runfit).stoich1 * (*runfit).mw[0] * buoyancy[0] * x;
               if ((*runfit).mw_fit[0])
               {
                  //      1:  dy/dMW = exp[u[0]] * Constant * x * buoyancy + exp[u[1]] * (*runfit).stoich1 * Constant * x * buoyancy
                  jacobian[point_counter][(*runfit).mw_index[0]] = constant * x * buoyancy[0] * exp(u_function[0])
                     + exp(u_function[1]) * (*runfit).stoich1 * constant * x * buoyancy[0];
               }
               if ((*runfit).vbar20_fit[0])
               {
                  //      1a: dy/dvbar = (-1) * exp[u[0]] * MW[0] * Constant *  x * rho - exp[u[1]] * (*runfit).stoich1 * MW[0] * Constant * x * rho
                  jacobian[point_counter][(*runfit).vbar20_index[0]] =
                     (-1.0) * exp(u_function[0]) * constant * (*runfit).mw[0] * x * (*scanfit_vector)[i].density
                     - exp(u_function[1]) * (*runfit).stoich1 * constant * (*runfit).mw[0] * x * (*scanfit_vector)[i].density;
               }
               if ((*scanfit_vector)[i].amplitude_fit[0])
               {
                  //      2: dy/dln(amplitude[0]) = exp[u[0]] + (*runfit).stoich1 * exp[u[1]]
                  jacobian[point_counter][(*scanfit_vector)[i].amplitude_index[0]] =
                     exp(u_function[0]) + (*runfit).stoich1 * exp(u_function[1]);
               }
               if ((*runfit).eq_fit[0])
               {
                  //      3: dy/dE = exp[u[1]]
                  jacobian[point_counter][(*runfit).eq_index[0]] = exp(u_function[1]);
               }
               if ((*scanfit_vector)[i].baseline_fit)
               {
                  //      4: dy/dbaseline = 1.0
                  jacobian[point_counter][(*scanfit_vector)[i].baseline_index] = 1.0;
               }
               point_counter++;
            }
            dataset_counter++;
         }
      }
      delete [] u_function;
      delete [] vbar;
      delete [] buoyancy;
   }
   else if ((which_model >= 11) && (which_model <= 13))
   {
      switch (which_model)
      {
      case 11: // Monomer-Dimer-Trimer Equilibrium
         {
            (*runfit).stoich1 = 2;
            (*runfit).stoich2 = 3;
            break;
         }
      case 12: // Monomer-Dimer-Tetramer Equilibrium
         {
            (*runfit).stoich1 = 2;
            (*runfit).stoich2 = 4;
            break;
         }
      }
      /*
        Function:
        y = [exp[ln(amplitude[0]) + MW[0] * w^2/2RT * (r^2 - r0^2) * (1 - vbar[0] * rho)] ]
        + [exp[(*runfit).stoich1 * ln(amplitude[0]) + ln(K1-(*runfit).stoich1) + (*runfit).stoich1 * MW[0] * w^2/2RT * (r^2 - r0^2) * (1 - vbar[0] * rho)] ]
        + [exp[(*runfit).stoich2 * ln(amplitude[0]) + ln(K2-(*runfit).stoich2) + (*runfit).stoich2 * MW[0] * w^2/2RT * (r^2 - r0^2) * (1 - vbar[0] * rho)] ]
        + baseline

        Substitutions:
        x = r^2  - r0^2
        Constant = omega^2/2RT
        buoyancy = (1 - vbar[0] * rho)
        E = ln(k1-(*runfit).stoich1)
        u[0] = ln(amplitude[0]) + MW[0] * Constant * x * buoyancy
        u[1] = ln((*runfit).stoich1 * amplitude[0]) + E1 + (*runfit).stoich1 * MW[0] * Constant * x * buoyancy
        u[2] = ln((*runfit).stoich2 * amplitude[0]) + E2 + (*runfit).stoich2 * MW[0] * Constant * x * buoyancy

        Partials:
        1:  dy/dMW = exp[u[0]] * Constant * x * buoyancy + exp[u[1]] * (*runfit).stoich1 * Constant * x * buoyancy + exp[u[2]] * (*runfit).stoich2 * Constant * x * buoyancy

        1a: dy/dvbar = (-1) * exp[u[0]] * MW[0] * Constant *  x * rho - exp[u[1]] * (*runfit).stoich1 * MW[0] * Constant * x * rho - exp[u[1]] * (*runfit).stoich2 * MW[0] * Constant * x * rho

        2: dy/dln(amplitude[0]) = exp[u[0]] + (*runfit).stoich1 * exp[u[1]] + (*runfit).stoich2 * exp[u[2]]

        3: dy/dE1 = exp[u[1]]

        4: dy/dE2 = exp[u[2]]

        5: dy/dbaseline = 1.0
      */
      u_function   = new float [2];
      vbar         = new float [1];
      buoyancy      = new float [1];
      point_counter = 0;
      dataset_counter = 0;
      for (i=0; i<(*scanfit_vector).size(); i++)
      {
         if ((*scanfit_vector)[i].FitScanFlag)
         {
            xm_sqr = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2);
            omega_sqr = pow(M_PI * (*scanfit_vector)[i].rpm / 30, 2);
            constant = omega_sqr / (2.0 * R * (K0 + (*scanfit_vector)[i].temperature));
            vbar[0] = adjust_vbar20((*runfit).vbar20[0], (*scanfit_vector)[i].temperature);
            buoyancy[0] = (1.0 - vbar[0] * (*scanfit_vector)[i].density);
            OD_correction1 = log((*runfit).stoich1 / pow((*scanfit_vector)[i].extinction[0]
                                                         * (*scanfit_vector)[i].pathlength, (*runfit).stoich1-1));
            OD_correction2 = log((*runfit).stoich2 / pow((*scanfit_vector)[i].extinction[0]
                                                         * (*scanfit_vector)[i].pathlength, (*runfit).stoich2-1));
            for (j=0; j<points_per_dataset[dataset_counter]; j++)
            {
               if (j == 0)
               {
                  x = 0.0;
               }
               else
               {
                  x = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j], 2) - xm_sqr;
               }
               u_function[0] = (*scanfit_vector)[i].amplitude[0] +
                  constant * (*runfit).mw[0] * buoyancy[0] * x;
               u_function[1] = (*runfit).stoich1 * (*scanfit_vector)[i].amplitude[0] + (*runfit).eq[0] +
                  OD_correction1 + constant * (*runfit).stoich1 * (*runfit).mw[0] * buoyancy[0] * x;
               u_function[2] = (*runfit).stoich2 * (*scanfit_vector)[i].amplitude[0] + (*runfit).eq[1] +
                  OD_correction2 + constant * (*runfit).stoich2 * (*runfit).mw[0] * buoyancy[0] * x;
               if ((*runfit).mw_fit[0])
               {
                  //      1:  dy/dMW = exp[u[0]] * Constant * x * buoyancy + exp[u[1]] * (*runfit).stoich1 * Constant * x * buoyancy
                  jacobian[point_counter][(*runfit).mw_index[0]] = constant * x * buoyancy[0] * exp(u_function[0])
                     + exp(u_function[1]) * (*runfit).stoich1 * constant * x * buoyancy[0]
                     + exp(u_function[2]) * (*runfit).stoich2 * constant * x * buoyancy[0];
               }
               if ((*runfit).vbar20_fit[0])
               {
                  //      1a: dy/dvbar = (-1) * exp[u[0]] * MW[0] * Constant *  x * rho - exp[u[1]] * (*runfit).stoich1 * MW[0] * Constant * x * rho
                  jacobian[point_counter][(*runfit).vbar20_index[0]] =
                     (-1.0) * exp(u_function[0]) * constant * (*runfit).mw[0] * x * (*scanfit_vector)[i].density
                     - exp(u_function[1]) * (*runfit).stoich1 * constant * (*runfit).mw[0] * x * (*scanfit_vector)[i].density
                     - exp(u_function[2]) * (*runfit).stoich2 * constant * (*runfit).mw[0] * x * (*scanfit_vector)[i].density;
               }
               if ((*scanfit_vector)[i].amplitude_fit[0])
               {
                  //      2: dy/dln(amplitude[0]) = exp[u[0]] + (*runfit).stoich1 * exp[u[1]]
                  jacobian[point_counter][(*scanfit_vector)[i].amplitude_index[0]] =
                     exp(u_function[0]) + (*runfit).stoich1 * exp(u_function[1]) + (*runfit).stoich2 * exp(u_function[2]);
               }
               if ((*runfit).eq_fit[0])
               {
                  //      3: dy/dE1 = exp[u[1]]
                  jacobian[point_counter][(*runfit).eq_index[0]] = exp(u_function[1]);
               }
               if ((*runfit).eq_fit[1])
               {
                  //      4: dy/dE2 = exp[u[2]]
                  jacobian[point_counter][(*runfit).eq_index[1]] = exp(u_function[2]);
               }
               if ((*scanfit_vector)[i].baseline_fit)
               {
                  //      5: dy/dbaseline = 1.0
                  jacobian[point_counter][(*scanfit_vector)[i].baseline_index] = 1.0;
               }
               point_counter++;
            }
            dataset_counter++;
         }
      }
      delete [] u_function;
      delete [] vbar;
      delete [] buoyancy;
   }
   else if (which_model == 14) // hetero association with self-association A + B <=> AB, nA <=> An
   {
      /*
        Function:
        y = exp[ln(amplitude[0]) + MW[0] * w^2/2RT * (r^2 - r0^2) * (1 - vbar[0] * rho)]
        + exp[ln(amplitude[1]) + MW[1] * w^2/2RT * (r^2 - r0^2) * (1 - vbar[1] * rho)]
        + exp[ln(amplitude[0]) + ln(amplitude[1]) + K_AB + ln((e[0] + e[1])/(e[0]*e[1]*l)) + (M[0] + M[1])*w^2/2RT*(r^2 - r0^2)*(1 - [(vbar[0]*M[0] + vbar[1]*M[1])/(M[0] + M[1])] * rho)]
        + baseline
      */
      float mw_ab;
      u_function   = new float [3];
      vbar         = new float [3];
      buoyancy      = new float [3];
      point_counter = 0;
      dataset_counter = 0;
      mw_ab = (*runfit).mw[0] + (*runfit).mw[1];
      for (i=0; i<(*scanfit_vector).size(); i++)
      {
         if ((*scanfit_vector)[i].FitScanFlag)
         {
            xm_sqr = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2);
            omega_sqr = pow(M_PI * (*scanfit_vector)[i].rpm / 30, 2);
            constant = omega_sqr / (2.0 * R * (K0 + (*scanfit_vector)[i].temperature));
            vbar[0] = adjust_vbar20((*runfit).vbar20[0], (*scanfit_vector)[i].temperature);
            vbar[1] = adjust_vbar20((*runfit).vbar20[0], (*scanfit_vector)[i].temperature);
            vbar[2] = (vbar[0] * (*runfit).mw[0] + vbar[1] * (*runfit).mw[1]) / mw_ab;
            buoyancy[0] = (1.0 - vbar[0] * (*scanfit_vector)[i].density);
            buoyancy[1] = (1.0 - vbar[1] * (*scanfit_vector)[i].density);
            buoyancy[2] = (1.0 - vbar[2] * (*scanfit_vector)[i].density);
            OD_correction1 = log(((*scanfit_vector)[i].extinction[0]
                                  + (*scanfit_vector)[i].extinction[1])
                                 /   ((*scanfit_vector)[i].pathlength
                                      * (*scanfit_vector)[i].extinction[0]
                                      * (*scanfit_vector)[i].extinction[1]));
            for (j=0; j<points_per_dataset[dataset_counter]; j++)
            {
               if (j == 0)
               {
                  x = 0.0;
               }
               else
               {
                  x = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j], 2) - xm_sqr;
               }
               constant_x = constant * x;
               u_function[0] = (*scanfit_vector)[i].amplitude[0] + constant_x * (*runfit).mw[0] * buoyancy[0];
               u_function[1] = (*scanfit_vector)[i].amplitude[1] + constant_x * (*runfit).mw[1] * buoyancy[1];
               u_function[2] = (*scanfit_vector)[i].amplitude[0] + (*scanfit_vector)[i].amplitude[1]
                  + (*runfit).eq[0] + OD_correction1  + constant_x * mw_ab * buoyancy[2];
               if ((*runfit).mw_fit[0])
               {
                  jacobian[point_counter][(*runfit).mw_index[0]] = constant_x * buoyancy[0] * exp(u_function[0])
                     + constant_x * buoyancy[2]
                     + constant_x * (vbar[2] * (*scanfit_vector)[i].density - vbar[0] * (*scanfit_vector)[i].density)
                     + exp(u_function[2]);
               }
               if ((*runfit).mw_fit[1])
               {
                  jacobian[point_counter][(*runfit).mw_index[1]] = constant_x * buoyancy[1] * exp(u_function[1])
                     + constant_x * buoyancy[2]
                     + constant_x * (vbar[2] * (*scanfit_vector)[i].density - vbar[1] * (*scanfit_vector)[i].density)
                     + exp(u_function[2]);
               }
               if ((*runfit).vbar20_fit[0])
               {
                  jacobian[point_counter][(*runfit).vbar20_index[0]] = (-1.0) * constant_x * (*runfit).mw[0] * (*scanfit_vector)[i].density * exp(u_function[0])
                     + (-1.0) * constant_x * (*runfit).mw[0] * (*scanfit_vector)[i].density * exp(u_function[2]);
               }
               if ((*runfit).vbar20_fit[1])
               {
                  jacobian[point_counter][(*runfit).vbar20_index[1]] = (-1.0) * constant_x * (*runfit).mw[1] * (*scanfit_vector)[i].density * exp(u_function[1])
                     + (-1.0) * constant_x * (*runfit).mw[1] * (*scanfit_vector)[i].density * exp(u_function[2]);
               }
               if ((*scanfit_vector)[i].amplitude_fit[0])
               {
                  jacobian[point_counter][(*scanfit_vector)[i].amplitude_index[0]] = exp(u_function[0]) + exp(u_function[2]);
               }
               if ((*scanfit_vector)[i].amplitude_fit[1])
               {
                  jacobian[point_counter][(*scanfit_vector)[i].amplitude_index[1]] = exp(u_function[1]) + exp(u_function[2]);
               }
               if ((*runfit).eq_fit[0])
               {
                  jacobian[point_counter][(*runfit).eq_index[0]] = exp(u_function[2]);
               }
               if ((*scanfit_vector)[i].baseline_fit)
               {
                  jacobian[point_counter][(*scanfit_vector)[i].baseline_index] = 1.0;
               }
               point_counter++;
            }
            dataset_counter++;
         }
      }
      delete [] u_function;
      delete [] vbar;
      delete [] buoyancy;
   }
   else if (which_model == 15) // A + B <=> AB, SUM(1->n)[ A ] <=> An
   {
      /*
        Function:
        y = exp[ln(amplitude[0]) + MW[0] * w^2/2RT * (r^2 - r0^2) * (1 - vbar[0] * rho)]
        + exp[ln(amplitude[1]) + MW[1] * w^2/2RT * (r^2 - r0^2) * (1 - vbar[1] * rho)]
        + exp[ln(amplitude[0]) + ln(amplitude[1]) + K_AB + ln((e[0] + e[1])/(e[0]*e[1]*l)) + (M[0] + M[1])*w^2/2RT*(r^2 - r0^2)*(1 - [(vbar[0]*M[0] + vbar[1]*M[1])/(M[0] + M[1])] * rho)]
        + exp[n*ln(amplitude[0]) + K_12 + ln(n/(e[0]*e*l)^(n-1)) + n*M[0]*w^2/2RT*(r^2 - r0^2)*(1 - vbar[0]*rho)]
        + baseline
      */
      float mw_ab;
      u_function   = new float [4];
      vbar         = new float [3];
      buoyancy      = new float [3];
      point_counter = 0;
      dataset_counter = 0;
      mw_ab = (*runfit).mw[0] + (*runfit).mw[1];
      for (i=0; i<(*scanfit_vector).size(); i++)
      {
         if ((*scanfit_vector)[i].FitScanFlag)
         {
            xm_sqr = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2);
            omega_sqr = pow(M_PI * (*scanfit_vector)[i].rpm / 30, 2);
            constant = omega_sqr / (2.0 * R * (K0 + (*scanfit_vector)[i].temperature));
            vbar[0] = adjust_vbar20((*runfit).vbar20[0], (*scanfit_vector)[i].temperature);
            vbar[1] = adjust_vbar20((*runfit).vbar20[0], (*scanfit_vector)[i].temperature);
            vbar[2] = (vbar[0] * (*runfit).mw[0] + vbar[1] * (*runfit).mw[1]) / mw_ab;
            buoyancy[0] = (1.0 - vbar[0] * (*scanfit_vector)[i].density);
            buoyancy[1] = (1.0 - vbar[1] * (*scanfit_vector)[i].density);
            buoyancy[2] = (1.0 - vbar[2] * (*scanfit_vector)[i].density);
            OD_correction1 = log(((*scanfit_vector)[i].extinction[0] + (*scanfit_vector)[i].extinction[1])
                                 /((*scanfit_vector)[i].extinction[0] * (*scanfit_vector)[i].extinction[1] * (*scanfit_vector)[i].pathlength));
            OD_correction2 = log((*runfit).stoich1 / pow((*scanfit_vector)[i].extinction[0]
                                                         * (*scanfit_vector)[i].pathlength, (*runfit).stoich1-1));
            for (j=0; j<points_per_dataset[dataset_counter]; j++)
            {
               if (j == 0)
               {
                  x = 0.0;
               }
               else
               {
                  x = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j], 2) - xm_sqr;
               }
               constant_x = constant * x;
               u_function[0] = (*scanfit_vector)[i].amplitude[0] + constant_x * (*runfit).mw[0] * buoyancy[0];
               u_function[1] = (*scanfit_vector)[i].amplitude[1] + constant_x * (*runfit).mw[1] * buoyancy[1];
               u_function[2] = (*scanfit_vector)[i].amplitude[0] + (*scanfit_vector)[i].amplitude[1]
                  + (*runfit).eq[0] + OD_correction1  + constant_x * mw_ab * buoyancy[2];
               u_function[3] = (*runfit).stoich1 * (*scanfit_vector)[i].amplitude[0] + (*runfit).eq[1] + OD_correction2 + constant_x * (*runfit).stoich1 * mw_ab * buoyancy[0];
               if ((*runfit).mw_fit[0])
               {
                  jacobian[point_counter][(*runfit).mw_index[0]] = constant_x * buoyancy[0] * exp(u_function[0])
                     + constant_x * buoyancy[2]
                     + constant_x * (vbar[2] * (*scanfit_vector)[i].density - vbar[0] * (*scanfit_vector)[i].density)
                     + (*runfit).stoich1 * constant_x * buoyancy[0] * exp(u_function[3])
                     + exp(u_function[2]);
               }
               if ((*runfit).mw_fit[1])
               {
                  jacobian[point_counter][(*runfit).mw_index[1]] = constant_x * buoyancy[1] * exp(u_function[1])
                     + constant_x * buoyancy[2]
                     + constant_x * (vbar[2] * (*scanfit_vector)[i].density - vbar[1] * (*scanfit_vector)[i].density)
                     + exp(u_function[2]);
               }
               if ((*runfit).vbar20_fit[0])
               {
                  jacobian[point_counter][(*runfit).vbar20_index[0]] = (-1.0) * constant_x * (*runfit).mw[0] * (*scanfit_vector)[i].density * exp(u_function[0])
                     - (*runfit).stoich1 * (*runfit).mw[0] * constant_x * (*scanfit_vector)[i].density * exp(u_function[3])
                     - constant_x * (*runfit).mw[0] * (*scanfit_vector)[i].density * exp(u_function[2]);
               }
               if ((*runfit).vbar20_fit[1])
               {
                  jacobian[point_counter][(*runfit).vbar20_index[1]] = (-1.0) * constant_x * (*runfit).mw[1] * (*scanfit_vector)[i].density * exp(u_function[1])
                     + (-1.0) * constant_x * (*runfit).mw[1] * (*scanfit_vector)[i].density * exp(u_function[2]);
               }
               if ((*scanfit_vector)[i].amplitude_fit[0])
               {
                  jacobian[point_counter][(*scanfit_vector)[i].amplitude_index[0]] = exp(u_function[0]) + (*runfit).stoich1 * exp(u_function[3]) + exp(u_function[2]);
               }
               if ((*scanfit_vector)[i].amplitude_fit[1])
               {
                  jacobian[point_counter][(*scanfit_vector)[i].amplitude_index[1]] = exp(u_function[1]) + exp(u_function[2]);
               }
               if ((*runfit).eq_fit[0])
               {
                  jacobian[point_counter][(*runfit).eq_index[0]] = exp(u_function[2]);
               }
               if ((*runfit).eq_fit[1])
               {
                  jacobian[point_counter][(*runfit).eq_index[0]] = exp(u_function[3]);
               }
               if ((*scanfit_vector)[i].baseline_fit)
               {
                  jacobian[point_counter][(*scanfit_vector)[i].baseline_index] = 1.0;
               }
               point_counter++;
            }
            dataset_counter++;
         }
      }
      delete [] u_function;
      delete [] vbar;
      delete [] buoyancy;
   }
   else if (which_model == 16)
   {
      u_function   = new float [3];
      vbar         = new float [1];
      buoyancy      = new float [1];
      point_counter = 0;
      dataset_counter = 0;
      (*runfit).mw[1] = (*runfit).mw[0];
      for (i=0; i<(*scanfit_vector).size(); i++)
      {
         if ((*scanfit_vector)[i].FitScanFlag)
         {
            xm_sqr = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2);
            omega_sqr = pow(M_PI * (*scanfit_vector)[i].rpm / 30, 2);
            constant = omega_sqr / (2.0 * R * (K0 + (*scanfit_vector)[i].temperature));
            vbar[0] = adjust_vbar20((*runfit).vbar20[0], (*scanfit_vector)[i].temperature);
            buoyancy[0] = (1.0 - vbar[0] * (*scanfit_vector)[i].density);
            OD_correction1 = log((*runfit).stoich1 / pow((*scanfit_vector)[i].extinction[0]
                                                         * (*scanfit_vector)[i].pathlength, (*runfit).stoich1-1));
            for (j=0; j<points_per_dataset[dataset_counter]; j++)
            {
               if (j == 0)
               {
                  x = 0.0;
               }
               else
               {
                  x = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j], 2) - xm_sqr;
               }
               constant_x = constant * x;
               u_function[0] = exp((*scanfit_vector)[i].amplitude[0] + constant_x * (*runfit).mw[0] * buoyancy[0]);
               u_function[1] = exp((*scanfit_vector)[i].amplitude[1] + constant_x * (*runfit).mw[0] * buoyancy[0]);
               u_function[2] = exp((*runfit).stoich1 * (*scanfit_vector)[i].amplitude[0] + (*runfit).eq[0] + OD_correction1 + constant_x * (*runfit).stoich1 * (*runfit).mw[0] * buoyancy[0]);
               //cout << u_function[2] << ", " <<   (*runfit).stoich1  << ", " <<(*scanfit_vector)[i].amplitude[0] << ", " << (*runfit).eq[0]  << ", " <<     OD_correction1<< ", " << constant_x << ", " <<  (*runfit).mw[0] << ", " << buoyancy[0] << endl;
               if ((*runfit).mw_fit[0])
               {
                  jacobian[point_counter][(*runfit).mw_index[0]] = constant_x * buoyancy[0] * u_function[0]
                     + constant_x * buoyancy[0] * u_function[1]
                     + constant_x * buoyancy[0] * u_function[2] * (*runfit).stoich1;
               }
               if ((*runfit).vbar20_fit[0])
               {
                  jacobian[point_counter][(*runfit).vbar20_index[0]] = (-1.0) * constant_x * (*runfit).mw[0] * (*scanfit_vector)[i].density * u_function[0]
                     - constant_x * (*runfit).mw[0] * (*scanfit_vector)[i].density * u_function[1]
                     - constant_x * (*runfit).mw[0] * (*scanfit_vector)[i].density * u_function[2] * (*runfit).stoich1;
               }
               if ((*scanfit_vector)[i].amplitude_fit[0])
               {
                  jacobian[point_counter][(*scanfit_vector)[i].amplitude_index[0]] = u_function[0] + (*runfit).stoich1 * u_function[2];
               }
               if ((*scanfit_vector)[i].amplitude_fit[1])
               {
                  jacobian[point_counter][(*scanfit_vector)[i].amplitude_index[1]] = u_function[1];
               }
               if ((*runfit).eq_fit[0])
               {
                  jacobian[point_counter][(*runfit).eq_index[0]] = u_function[2];
                  //                  cout <<    jacobian[point_counter][(*runfit).eq_index[0]]  << endl;
               }
               if ((*scanfit_vector)[i].baseline_fit)
               {
                  jacobian[point_counter][(*scanfit_vector)[i].baseline_index] = 1.0;
               }
               point_counter++;
            }
            dataset_counter++;
         }
      }
      delete [] u_function;
      delete [] vbar;
      delete [] buoyancy;
   }
   else if (which_model == 17)
   {
      u_function   = new float [3];
      vbar         = new float [1];
      buoyancy      = new float [1];
      point_counter = 0;
      dataset_counter = 0;
      (*runfit).mw[1] = (*runfit).stoich1 * (*runfit).mw[0];
      for (i=0; i<(*scanfit_vector).size(); i++)
      {
         if ((*scanfit_vector)[i].FitScanFlag)
         {
            xm_sqr = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2);
            omega_sqr = pow(M_PI * (*scanfit_vector)[i].rpm / 30, 2);
            constant = omega_sqr / (2.0 * R * (K0 + (*scanfit_vector)[i].temperature));
            vbar[0] = adjust_vbar20((*runfit).vbar20[0], (*scanfit_vector)[i].temperature);
            buoyancy[0] = (1.0 - vbar[0] * (*scanfit_vector)[i].density);
            OD_correction1 = log((*runfit).stoich1 / pow((*scanfit_vector)[i].extinction[0]
                                                         * (*scanfit_vector)[i].pathlength, (*runfit).stoich1-1));
            for (j=0; j<points_per_dataset[dataset_counter]; j++)
            {
               if (j == 0)
               {
                  x = 0.0;
               }
               else
               {
                  x = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j], 2) - xm_sqr;
               }
               constant_x = constant * x;
               u_function[0] = exp((*scanfit_vector)[i].amplitude[0] + constant_x * (*runfit).mw[0] * buoyancy[0]);
               u_function[1] = exp((*scanfit_vector)[i].amplitude[1] + constant_x * (*runfit).mw[1] * buoyancy[0]);
               u_function[2] = exp((*runfit).stoich1 * (*scanfit_vector)[i].amplitude[0] + (*runfit).eq[0] + OD_correction1 + constant_x * (*runfit).stoich1 * (*runfit).mw[0] * buoyancy[0]);
               //cout << u_function[2] << ", " <<   (*runfit).stoich1  << ", " <<(*scanfit_vector)[i].amplitude[0] << ", " << (*runfit).eq[0]  << ", " <<     OD_correction1<< ", " << constant_x << ", " <<  (*runfit).mw[0] << ", " << buoyancy[0] << endl;
               if ((*runfit).mw_fit[0])
               {
                  jacobian[point_counter][(*runfit).mw_index[0]] = constant_x * buoyancy[0] * u_function[0]
                     + constant_x * buoyancy[0] * u_function[1] * (*runfit).stoich1
                     + constant_x * buoyancy[0] * u_function[2] * (*runfit).stoich1;
               }
               if ((*runfit).vbar20_fit[0])
               {
                  jacobian[point_counter][(*runfit).vbar20_index[0]] = (-1.0) * constant_x * (*runfit).mw[0] * (*scanfit_vector)[i].density * u_function[0]
                     - constant_x * (*runfit).mw[0] * (*scanfit_vector)[i].density * u_function[1] * (*runfit).stoich1
                     - constant_x * (*runfit).mw[0] * (*scanfit_vector)[i].density * u_function[2] * (*runfit).stoich1;
               }
               if ((*scanfit_vector)[i].amplitude_fit[0])
               {
                  jacobian[point_counter][(*scanfit_vector)[i].amplitude_index[0]] = u_function[0] + (*runfit).stoich1 * u_function[2];
               }
               if ((*scanfit_vector)[i].amplitude_fit[1])
               {
                  jacobian[point_counter][(*scanfit_vector)[i].amplitude_index[1]] = u_function[1];
               }
               if ((*runfit).eq_fit[0])
               {
                  jacobian[point_counter][(*runfit).eq_index[0]] = u_function[2];
                  //                  cout <<    jacobian[point_counter][(*runfit).eq_index[0]]  << endl;
               }
               if ((*scanfit_vector)[i].baseline_fit)
               {
                  jacobian[point_counter][(*scanfit_vector)[i].baseline_index] = 1.0;
               }
               point_counter++;
            }
            dataset_counter++;
         }
      }
      delete [] u_function;
      delete [] vbar;
      delete [] buoyancy;
   }
   else if (which_model == 18)
   {
      u_function   = new float [3];
      vbar         = new float [1];
      buoyancy      = new float [1];
      point_counter = 0;
      dataset_counter = 0;
      (*runfit).mw[1] = (*runfit).stoich1 * (*runfit).mw[0];
      for (i=0; i<(*scanfit_vector).size(); i++)
      {
         if ((*scanfit_vector)[i].FitScanFlag)
         {
            xm_sqr = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2);
            omega_sqr = pow(M_PI * (*scanfit_vector)[i].rpm / 30, 2);
            constant = omega_sqr / (2.0 * R * (K0 + (*scanfit_vector)[i].temperature));
            vbar[0] = adjust_vbar20((*runfit).vbar20[0], (*scanfit_vector)[i].temperature);
            buoyancy[0] = (1.0 - vbar[0] * (*scanfit_vector)[i].density);
            OD_correction1 = log((*runfit).stoich1 / pow((*scanfit_vector)[i].extinction[0]
                                                         * (*scanfit_vector)[i].pathlength, (*runfit).stoich1-1));
            for (j=0; j<points_per_dataset[dataset_counter]; j++)
            {
               if (j == 0)
               {
                  x = 0.0;
               }
               else
               {
                  x = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j], 2) - xm_sqr;
               }
               constant_x = constant * x;
               u_function[0] = exp((*scanfit_vector)[i].amplitude[0] + constant_x * (*runfit).mw[0] * buoyancy[0]);
               u_function[1] = exp((*scanfit_vector)[i].amplitude[1] + constant_x * (*runfit).mw[1] * buoyancy[0]);
               if ((*runfit).mw_fit[0])
               {
                  jacobian[point_counter][(*runfit).mw_index[0]] = constant_x * buoyancy[0] * u_function[0]
                     + constant_x * buoyancy[0] * u_function[1] * (*runfit).stoich1;
                  //cout << "i: " << i << ", : " << j << ": " << "MW: " << jacobian[point_counter][(*runfit).mw_index[0]] << ", ";
               }
               if ((*runfit).vbar20_fit[0])
               {
                  jacobian[point_counter][(*runfit).vbar20_index[0]] = (-1.0) * constant_x * (*runfit).mw[0] * (*scanfit_vector)[i].density * u_function[0]
                     - constant_x * (*runfit).mw[0] * (*scanfit_vector)[i].density * u_function[1] * (*runfit).stoich1;
               }
               if ((*scanfit_vector)[i].amplitude_fit[0])
               {
                  jacobian[point_counter][(*scanfit_vector)[i].amplitude_index[0]] = u_function[0];
                  //cout << "amplitude[0]: " << jacobian[point_counter][(*scanfit_vector)[i].amplitude_index[0]] << ", ";
               }
               if ((*scanfit_vector)[i].amplitude_fit[1])
               {
                  jacobian[point_counter][(*scanfit_vector)[i].amplitude_index[1]] = u_function[1];
                  //cout << "amplitude[1]: " << jacobian[point_counter][(*scanfit_vector)[i].amplitude_index[1]] << ", ";
               }
               if ((*scanfit_vector)[i].baseline_fit)
               {
                  jacobian[point_counter][(*scanfit_vector)[i].baseline_index] = 1.0;
                  //cout << "baseline: " << jacobian[point_counter][(*scanfit_vector)[i].baseline_index] << endl;
               }
               point_counter++;
            }
            dataset_counter++;
         }
      }

      delete [] u_function;
      delete [] vbar;
      delete [] buoyancy;
   }
   else if (which_model == 19)
   {
      u_function   = new float [3];
      vbar         = new float [2];
      buoyancy      = new float [2];
      point_counter = 0;
      dataset_counter = 0;
      for (i=0; i<(*scanfit_vector).size(); i++)
      {
         if ((*scanfit_vector)[i].FitScanFlag)
         {
            xm_sqr = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2);
            omega_sqr = pow(M_PI * (*scanfit_vector)[i].rpm / 30, 2);
            constant = omega_sqr / (2.0 * R * (K0 + (*scanfit_vector)[i].temperature));
            vbar[0] = adjust_vbar20((*runfit).vbar20[0], (*scanfit_vector)[i].temperature);
            vbar[1] = adjust_vbar20((*runfit).vbar20[1], (*scanfit_vector)[i].temperature);
            buoyancy[0] = (1.0 - vbar[0] * (*scanfit_vector)[i].density);
            buoyancy[1] = (1.0 - vbar[1] * (*scanfit_vector)[i].density);
            OD_correction1 = log((*runfit).stoich1 / pow((*scanfit_vector)[i].extinction[0]
                                                         * (*scanfit_vector)[i].pathlength, (*runfit).stoich1-1));
            for (j=0; j<points_per_dataset[dataset_counter]; j++)
            {
               if (j == 0)
               {
                  x = 0.0;
               }
               else
               {
                  x = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j], 2) - xm_sqr;
               }
               constant_x = constant * x;
               u_function[0] = exp((*scanfit_vector)[i].amplitude[0] + constant_x * (*runfit).mw[0] * buoyancy[0]);
               u_function[1] = exp((*scanfit_vector)[i].amplitude[1] + constant_x * (*runfit).mw[1] * buoyancy[1]);
               u_function[2] = exp((*runfit).stoich1 * (*scanfit_vector)[i].amplitude[0] + (*runfit).eq[0] + OD_correction1 + constant_x * (*runfit).stoich1 * (*runfit).mw[0] * buoyancy[0]);
               if ((*runfit).mw_fit[0])
               {
                  jacobian[point_counter][(*runfit).mw_index[0]] = constant_x * buoyancy[0] * u_function[0]
                     + constant_x * buoyancy[0] * u_function[2] * (*runfit).stoich1;
               }
               if ((*runfit).mw_fit[1])
               {
                  jacobian[point_counter][(*runfit).mw_index[0]] = constant_x * buoyancy[1] * u_function[1];
               }
               if ((*runfit).vbar20_fit[0])
               {
                  jacobian[point_counter][(*runfit).vbar20_index[0]] = (-1.0) * constant_x * (*runfit).mw[0] * (*scanfit_vector)[i].density * u_function[0]
                     - constant_x * (*runfit).mw[0] * (*scanfit_vector)[i].density * u_function[2] * (*runfit).stoich1;
               }
               if ((*runfit).vbar20_fit[1])
               {
                  jacobian[point_counter][(*runfit).vbar20_index[0]] = (-1.0) * constant_x * (*runfit).mw[1] * (*scanfit_vector)[i].density * u_function[1];
               }
               if ((*scanfit_vector)[i].amplitude_fit[0])
               {
                  jacobian[point_counter][(*scanfit_vector)[i].amplitude_index[0]] = u_function[0] + (*runfit).stoich1 * u_function[2];
               }
               if ((*scanfit_vector)[i].amplitude_fit[1])
               {
                  jacobian[point_counter][(*scanfit_vector)[i].amplitude_index[1]] = u_function[1];
               }
               if ((*runfit).eq_fit[0])
               {
                  jacobian[point_counter][(*runfit).eq_index[0]] = u_function[2];
               }
               if ((*scanfit_vector)[i].baseline_fit)
               {
                  jacobian[point_counter][(*scanfit_vector)[i].baseline_index] = 1.0;
               }
               point_counter++;
            }
            dataset_counter++;
         }
      }
      delete [] u_function;
      delete [] vbar;
      delete [] buoyancy;
   }
   /*
     cout << "In Jacobian:" << parameters << " Parameters\n";
     for (i=0; i<(*scanfit_vector).size(); i++)
     {
     if ((*scanfit_vector)[i].FitScanFlag)
     {
     cout    << "Amplitude: " << (*scanfit_vector)[i].amplitude_index[0]
     << ", Molecular weight: " << (*runfit).mw_index[0]
     << ", Equilibrium: " << (*runfit).eq_index[0]
     << ", Baseline: " << (*scanfit_vector)[i].baseline_index << endl;
     }
     }

     for (i=0; i<points; i++)
     {
     str1="";
     for (j=0; j<parameters; j++)
     {
     str.sprintf("%2.3e\t", jacobian[i][j]);
     str1 +=  str;
     }
     cout << str1.latin1() << endl;
     }
   */

   return (0);
}

void US_EqFitter::cleanup()
{
   if(GUI)
   {
      bt_plotGroup->setEnabled(false);
      bt_plotSingle->setEnabled(false);
   }

   unsigned int i;
   if (y_raw != NULL)
   {
      delete [] y_raw;
      y_raw = NULL;
      //cout << "deleting y_raw...\n";
   }
   if (y_guess != NULL)
   {
      delete [] y_guess;
      y_guess = NULL;
      //cout << "deleting y_guess...\n";
   }
   if (y_delta != NULL)
   {
      delete [] y_delta;
      y_delta = NULL;
      //cout << "deleting y_delta...\n";
   }
   if (B != NULL)
   {
      delete [] B;
      B = NULL;
      //cout << "deleting B...\n";
   }
   if (guess != NULL)
   {
      delete [] guess;
      guess = NULL;
      //cout << "deleting guess...\n";
   }
   if (test_guess != NULL)
   {
      delete [] test_guess;
      test_guess = NULL;
      //cout << "deleting test_guess...\n";
   }
   if (jacobian != NULL)
   {
      for (i=0; i<points; i++)
      {
         delete [] jacobian[i];
      }
      delete [] jacobian;
      jacobian = NULL;
      //cout << "deleting jacobian...\n";
   }
   if (which_model != 3)
   {
      if (information_matrix != NULL)
      {
         for (i=0; i<parameters; i++)
         {
            delete [] information_matrix[i];
         }
         delete [] information_matrix;
         information_matrix = NULL;
         //cout << "deleting information matrix...\n";
      }

      if (LL_transpose != NULL)
      {
         for (i=0; i<parameters; i++)
         {
            delete [] LL_transpose[i];
         }
         delete [] LL_transpose;
         LL_transpose = NULL;
         //cout << "deleting LL_transpose...\n";
      }
   }
   if (lncr2 != NULL)
   {
      for (i=0; i<datasets; i++)
      {
         delete [] lncr2[i];
      }
      delete [] lncr2;
      lncr2 = NULL;
      //cout << "deleting lncr2...\n";
   }
   if (dlncr2 != NULL)
   {
      for (i=0; i<datasets; i++)
      {
         delete [] dlncr2[i];
      }
      delete [] dlncr2;
      dlncr2 = NULL;
      //cout << "deleting dlncr2...\n";
   }
   if (dcr2 != NULL)
   {
      for (i=0; i<datasets; i++)
      {
         delete [] dcr2[i];
      }
      delete [] dcr2;
      dcr2 = NULL;
      //cout << "deleting dcr2...\n";
   }
   dcr2 = NULL;
}

void US_EqFitter::saveFit()
{
   QString str;
   QString startFile = USglobal->config_list.result_dir + "/" + *projectName + str.sprintf("-%d", which_model) + ".eq_fit";
   QString fn = QFileDialog::getSaveFileName(startFile, "*.eq_fit", 0);
   int k;
   if (!fn.isEmpty())
   {
      k = fn.find(".eq_fit", 0, FALSE);
      if (k != -1) //if an extension was given, strip it
      {
         fn.truncate(k);
      }
      saveFit(fn); // the user gave a file name
   }
   QString ProjectFile = USglobal->config_list.result_dir + "/" + *projectName + ".eq-project";
   QFile p(ProjectFile);
   if (p.open(IO_WriteOnly | IO_Translate))
   {
      QTextStream ts(&p);
      ts << *projectName << "\n";
      ts << (*run_information).size() << "\n";
      for (unsigned int i=0; i<(*run_information).size(); i++)
      {
         ts << "Experiment " << i+1 << ":\n";
         ts << (*run_information)[i].run_id << "\n";
         ts << (*run_information)[i].expdata_id << "\n";
         ts << (*run_information)[i].investigator << "\n";
         ts << (*run_information)[i].date << "\n";
         ts << (*run_information)[i].description << "\n";
         ts << (*run_information)[i].dbname << "\n";
         ts << (*run_information)[i].dbhost << "\n";
         ts << (*run_information)[i].dbdriver << "\n";
      }
      p.close();
   }
   else
   {
      QMessageBox::message(tr("UltraScan Error:"), tr("Trouble writing the report file:\n\n"
                                                      "   ") + ProjectFile + tr("\n\n"
                                                                                "Please make sure the disk is not full or write protected"));
      return;
   }
}

void US_EqFitter::saveFit(const QString &fileName)
{
   if(!GUI)
   {
      return;
   }
   unsigned int point_counter, dataset_counter;
   QString filestr;
   filestr = fileName;
   filestr.append(".eq_fit");
   QFile eqf_file(filestr);
   if (eqf_file.exists())
   {
      if(!QMessageBox::query( tr("Warning"), tr("Attention:\nThis file exists already!\n\nDo you want to overwrite it?"), tr("Yes"), tr("No")))
      {
         eqf_file.close();
         return;
      }
   }
   if (eqf_file.open(IO_WriteOnly))
   {
      QDataStream ds(&eqf_file);
      ds << (Q_UINT16) (*run_information).size();
      for (unsigned int i=0; i<(*run_information).size(); i++)
      {
         ds << (QString) (*run_information)[i].run_id;
      }
      ds << (Q_UINT16) which_model;
      switch (which_model)
      {
      case 10:
         {
            ds << (Q_UINT16)(*runfit).stoich1;
            break;
         }
      case 13:
         {
            ds << (Q_UINT16)(*runfit).stoich1;
            ds << (Q_UINT16)(*runfit).stoich2;
            break;
         }
      case 16:
         {
            ds << (Q_UINT16)(*runfit).stoich1;
            break;
         }
      case 17:
         {
            ds << (Q_UINT16)(*runfit).stoich1;
            break;
         }
      case 18:
         {
            ds << (Q_UINT16)(*runfit).stoich1;
            break;
         }
      case 19:
         {
            ds << (Q_UINT16)(*runfit).stoich1;
            break;
         }
      }
      ds << *projectName;
      ds << (float) pow((double) variance, (double) 0.5);

      // provide the various dimensions, they can be used for load-checking later on to make sure
      // that the datasets still have the same number of scans when loading the fit

      ds << (Q_UINT16)(*scanfit_vector).size();

      // for  model-independent loading

      ds << (Q_UINT16)(*runfit).components;
      ds << (Q_UINT16)(*runfit).association_constants;

      // first process the scanfit info:

      for (unsigned int i=0; i<(*scanfit_vector).size(); i++)
      {
         ds << (Q_UINT16)(*scanfit_vector)[i].FitScanFlag;
         ds << (Q_UINT16)(*scanfit_vector)[i].autoExcluded;
         ds << (Q_UINT32)(*scanfit_vector)[i].start_index;
         //cout << "Save fit:\nScan: " << i << ": " << (*scanfit_vector)[i].start_index << " - " << (*scanfit_vector)[i].stop_index << endl;
         ds << (Q_UINT32)(*scanfit_vector)[i].stop_index;
         ds << (Q_UINT16)(*scanfit_vector)[i].limits_modified;
         ds << (float)(*scanfit_vector)[i].density;
         ds << (float)(*scanfit_vector)[i].pathlength;
         ds << (Q_UINT16)(*scanfit_vector)[i].baseline_fit;
         ds << (float)(*scanfit_vector)[i].baseline;
         ds << (float)(*scanfit_vector)[i].baseline_range;
         ds << (Q_UINT16)(*scanfit_vector)[i].baseline_bound;
         for (unsigned int j=0; j<(*runfit).components; j++)
         {
            ds << (Q_UINT16) (*scanfit_vector)[i].amplitude_fit[j];
            ds << (float) (*scanfit_vector)[i].amplitude[j];
            ds << (float) (*scanfit_vector)[i].amplitude_range[j];
            ds << (Q_UINT16) (*scanfit_vector)[i].amplitude_bound[j];
            ds << (float) (*scanfit_vector)[i].extinction[j];
         }
      }

      // now save the runfit info:
      for (unsigned int i=0; i<(*runfit).components; i++)
      {
         ds << (Q_UINT16)(*runfit).mw_fit[i];
         ds << (float)(*runfit).mw[i];
         ds << (float)(*runfit).mw_range[i];
         ds << (Q_UINT16)(*runfit).mw_bound[i];
         ds << (Q_UINT16)(*runfit).vbar20_fit[i];
         ds << (float)(*runfit).vbar20[i];
         ds << (float)(*runfit).vbar20_range[i];
         ds << (Q_UINT16)(*runfit).vbar20_bound[i];
         ds << (Q_UINT16)(*runfit).virial_fit[i];
         ds << (float)(*runfit).virial[i];
         ds << (float)(*runfit).virial_range[i];
         ds << (Q_UINT16)(*runfit).virial_bound[i];
      }
      for (unsigned int i=0; i<(*runfit).association_constants; i++)
      {
         ds << (Q_UINT16)(*runfit).eq_fit[i];
         ds << (float)(*runfit).eq[i];
         ds << (float)(*runfit).eq_range[i];
         ds << (Q_UINT16)(*runfit).eq_bound[i];
      }
      // writing the best fit for Monte Carlo
      point_counter = 0;
      dataset_counter = 0;
      ds << (Q_UINT16) points;
      for (unsigned int i=0; i<points; i++)
      {
         ds << (float) y_guess[i];
      }
      ds << od_limit;
   }
   eqf_file.close();
   QPixmap p;
   QString f, str, str1;
   plotGroup = 0;
   plot_residuals();
   qApp->processEvents();
   f = htmlDir + "/" + *projectName + "-" + str.sprintf("%d", which_model) + ".scans.html";
   emit dataSaved(f, which_model); //usglobalequil will process a the scan data to html file
   f = htmlDir + "/" + *projectName + "-" + str.sprintf("%d", which_model) + ".residuals-combined.";
   p = QPixmap::grabWidget(data_plot, 2, 2, data_plot->width() - 4, data_plot->height() - 4);
   pm->save_file(f, p);
   plot_overlays();
   qApp->processEvents();
   f = htmlDir + "/" + *projectName + "-" + str.sprintf("%d", which_model) + ".overlays-combined.";
   p = QPixmap::grabWidget(data_plot, 2, 2, data_plot->width() - 4, data_plot->height() - 4);
   pm->save_file(f, p);
   plot_two();
   if(which_model >= 0 && which_model < 4)
   {
      qApp->processEvents();
      f = htmlDir + "/" + *projectName + "-" + str.sprintf("%d", which_model) + ".lnc-vs-r2.";
      p = QPixmap::grabWidget(data_plot, 2, 2, data_plot->width() - 4, data_plot->height() - 4);
      pm->save_file(f, p);
   }
   plot_three();
   qApp->processEvents();
   f = htmlDir + "/" + *projectName + "-" + str.sprintf("%d", which_model) + ".mw-vs-r2.";
   p = QPixmap::grabWidget(data_plot, 2, 2, data_plot->width() - 4, data_plot->height() - 4);
   pm->save_file(f, p);
   plot_four();
   qApp->processEvents();
   f = htmlDir + "/" + *projectName + "-" + str.sprintf("%d", which_model) + ".mw-vs-c.";
   p = QPixmap::grabWidget(data_plot, 2, 2, data_plot->width() - 4, data_plot->height() - 4);
   pm->save_file(f, p);
   plotGroup = 5;
   firstScan = 1;
   unsigned int scan_count = 0, scan_set = 1;
   plot_residuals();
   qApp->processEvents();
   f = htmlDir + "/" + *projectName + "-" + str.sprintf("%d", which_model) + ".residuals-set1.";
   p = QPixmap::grabWidget(data_plot, 2, 2, data_plot->width() - 4, data_plot->height() - 4);
   pm->save_file(f, p);
   plot_overlays();
   qApp->processEvents();
   f = htmlDir + "/" + *projectName + "-" + str.sprintf("%d", which_model) + ".overlays-set1.";
   p = QPixmap::grabWidget(data_plot, 2, 2, data_plot->width() - 4, data_plot->height() - 4);
   pm->save_file(f, p);
   write_data();
   plotGroup = 5;
   for (unsigned int i=0; i<(*scanfit_vector).size(); i++)
   {
      if ((*scanfit_vector)[i].FitScanFlag)
      {
         scan_count++;

         if (scan_count == plotGroup)
         {
            scan_count = 0;
            firstScan += plotGroup;
            scan_set++;
            if (firstScan > datasets)
            {
               plotGroup = 0;
               updateRange(1.0); // reset scan range to the first scanset
               plot_residuals();
               return;
            }
            plot_residuals();
            qApp->processEvents();
            f = htmlDir + "/" + *projectName + "-" + str.sprintf("%d", which_model) + ".residuals-set" + str1.sprintf("%u", scan_set) + ".";
            p = QPixmap::grabWidget(data_plot, 2, 2, data_plot->width() - 4, data_plot->height() - 4);
            pm->save_file(f, p);
            plot_overlays();
            qApp->processEvents();
            f = htmlDir + "/" + *projectName + "-" + str.sprintf("%d", which_model) + ".overlays-set" + str1.sprintf("%u", scan_set) + ".";
            p = QPixmap::grabWidget(data_plot, 2, 2, data_plot->width() - 4, data_plot->height() - 4);
            pm->save_file(f, p);
         }
      }
   }
}

void US_EqFitter::print()
{
   if (!GUI)
   {
      return;
   }
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
         data_plot->print(printer, pf);
      }
      else if (print_inv)
      {
         PrintFilterDark pf;
         pf.setOptions(QwtPlotPrintFilter::PrintTitle
                       |QwtPlotPrintFilter::PrintMargin
                       |QwtPlotPrintFilter::PrintLegend
                       |QwtPlotPrintFilter::PrintGrid);
         data_plot->print(printer, pf);
      }
      else
      {
         data_plot->print(printer);
      }
   }
}

void US_EqFitter::view_report()
{
   if(!GUI)
   {
      return;
   }
   QString fileName, str;
   fileName = USglobal->config_list.result_dir + "/" + *projectName + "-" + str.sprintf("%d", which_model) + ".res";
   //view_file(fileName);
   TextEdit *e;
   e = new TextEdit();
   e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
   e->load(fileName);
   e->show();

}

void US_EqFitter::write_report()
{
   QString fileName, str, str1, str2;
   unsigned int i, j, k;
   float total_neg = 0.0, total_pos = 0.0;
   float percentage = 0, runs_variance, runs_expected, scale;
   fileName = USglobal->config_list.result_dir + "/" + *projectName + "-" + str.sprintf("%d", which_model) + ".res";
   QFile f1(fileName);
   (*runfit).runs = 0;
   calc_integral();
   if (f1.open(IO_WriteOnly | IO_Translate))
   {
      QTextStream ts(&f1);
      ts << "******************************************************************************************\n";
      ts << tr("*                              Global Equilibrium Analysis                               *\n");
      ts << "******************************************************************************************\n\n";
      ts << tr("Data Report for Project \"") << *projectName << "\"\n";
      ts << tr("Fitted Model: ") << modelString[which_model] << "\n\n";
      ts << tr("Parameters for this model:\n\n");
      for (i=0; i<(*runfit).components; i++)
      {
         str1.sprintf(tr("%2.3e dalton "), (*runfit).mw[i]);
         if ((*runfit).mw_fit[i])
         {
            str1 += tr("(fitted)\n");
         }
         else
         {
            str1 += tr("(fixed)\n");
         }
         str2.sprintf(tr("Molecular Weight for component %d: "), i + 1);
         ts << str2 << str1;
         str1.sprintf("%2.3e ccm/g ", (*runfit).vbar20[i]);
         if ((*runfit).vbar20_fit[i])
         {
            str1 += tr("(fitted)\n");
         }
         else
         {
            str1 += tr("(fixed)\n");
         }
         str2.sprintf(tr("Partial Specific Volume for component %d (at 20ºC): "), i + 1);
         ts << str2 << str1;
      }
      for (j=0; j<(*runfit).association_constants; j++)
      {
         str1.sprintf(tr("Association (Dissociation) Constant %d: %2.3e (%2.3e)\n"),
                      j + 1, exp((*runfit).eq[j]), 1.0/exp((*runfit).eq[j]));
         if ((*runfit).eq_fit[j])
         {
            str1 += tr("(fitted)\n");
         }
         else
         {
            str1 += tr("(fixed)\n");
         }
         ts << str1;
      }
      ts << tr("\nGlobal Fitting Statistics:\n\n");
      str1.sprintf("%1.4e", variance);
      ts << tr("Variance:                        ") << str1 << "\n";
      str1.sprintf("%1.4e", pow((double) variance, (double) 0.5));
      ts << tr("Standard Deviation:              ") << str1 << "\n";
      str1.sprintf("%d", parameters);
      ts << tr("Number of floated Parameters:    ") << str1 << "\n";
      str1.sprintf("%ld", datasets);
      ts << tr("Number of Datasets:              ") << str1 << "\n";
      str1.sprintf("%ld", points);
      ts << tr("Number of Datapoints:            ") << str1 << "\n";
      if (concentration_average != 0.0)
      {
         str1.sprintf("%le M", concentration_average);
      }
      else
      {
         str1 = "Not determined";
      }
      ts << tr("Average Datapoint concentration: ") << str1 << "\n";
      str1.sprintf("%ld", points - parameters);
      ts << tr("Number of Degrees of Freedom:    ") << str1 << "\n";

      for (i=0; i<(*scanfit_vector).size(); i++)
      {
         if ((*scanfit_vector)[i].FitScanFlag)
         {
            k=0;
            while ((*run_information)[k].run_id != (*scanfit_vector)[i].run_id)
            {
               k++;   // find the correct runinfo to match up with for the point_density
            }
            scale = (*run_information)[k].delta_r
               / (*run_information)[k].point_density[(*scanfit_vector)[i].cell][(*scanfit_vector)[i].lambda][(*scanfit_vector)[i].channel];
            total_neg += (*scanfit_vector)[i].neg * scale;
            total_pos += (*scanfit_vector)[i].pos * scale;
            (*runfit).runs += (long) (*scanfit_vector)[i].runs;
         }
      }
      (*runfit).runs_percent = (float) (total_pos + total_neg);
      (*runfit).runs_percent = (float) (*runfit).runs / (*runfit).runs_percent;
      (*runfit).runs_percent *= 100.0;
      str1.sprintf(tr("Number of Runs:               %ld (%2.3f"), (*runfit).runs, (*runfit).runs_percent);
      ts << str1 << tr(" %, corrected)\n");
      //
      // Formula for expected number of runs (P = nummber of positives, N = number of Negatives):
      // E(r) = 1 + 2PN/(P + N)
      // Formula for its variance:
      // V(R) = [2PN (2PN - P - N)] / [(P + N)^2 (P + N - 1)]
      //
      (*runfit).runs_expected = 1.0 + (2.0 * total_pos * total_neg) / (total_pos + total_neg);
      (*runfit).runs_variance = ((2.0 * total_pos * total_neg) *   (2.0 * total_pos * total_neg - total_pos - total_neg))
         / ((pow((total_pos + total_neg), 2)) * (total_pos + total_neg - 1.0));
      str1.sprintf(tr("Expected Number of Runs:      %ld (corrected)\nRun Variance:                 %2.3e (corrected)\n\n"),
                   (long) (*runfit).runs_expected, (*runfit).runs_variance);      ts << str1;
      if ((*runfit).runs_percent < 26.0)
      {
         ts << tr("According to these statistical tests, this model is either inappropriate for the\n");
         ts << tr("experimental data, or the fitting process has not yet converged. Please try to reduce\n");
         ts << tr("the variance by additional nonlinear least-squares minimization of the data.\n");
         ts << tr("This fit cannot not be used for a Monte Carlo Analysis.\n");
      }
      if ((*runfit).runs_percent > 26.0 && (*runfit).runs_percent < 30.0)
      {
         ts << tr("According to these statistical tests, this model is either a poor candidate for the\n");
         ts << tr("experimental data, or the fitting process has not yet converged. Please try to reduce\n");
         ts << tr("the variance by additional nonlinear least-squares minimization of the data.\n");
         ts << tr("This fit cannot be used for a Monte Carlo Analysis.\n");
      }
      if ((*runfit).runs_percent > 30.0 && (*runfit).runs_percent < 35.0)
      {
         ts << tr("According to these statistical tests, this model is an acceptable candidate for the experimental data.\n");
         ts << tr("This fit can be used for a Monte Carlo Analysis with reservations.\n");
      }
      if ((*runfit).runs_percent > 35.0)
      {
         ts << tr("According to these statistical tests, this model is a good candidate for the experimental data.\n");
         ts << tr("This fit is recommended for Monte Carlo Analysis.\n");
      }
      runs = (*runfit).runs;
      runs_percent = (*runfit).runs_percent;
      ts << tr("\nDetailed Information for fitted Scans:\n");
      for (i=0; i<(*scanfit_vector).size(); i++)
      {
         int split = (*scan_info)[i].find("rpm") + 3;
         str1 = (*scan_info)[i];
         str1.truncate(split);
         str2 = (*scan_info)[i];
         str2.remove(0, split+2);
         ts << "                                                                                                                                                                             \n";
         ts << "******************************************************************************************\n";
         ts << tr("Information for Scan ") << str1 << "\n";
         ts << "(" << str2 << ")\n";
         ts << "\n";
         if ((*scanfit_vector)[i].FitScanFlag)
         {
            str1.sprintf("%2.3e OD ", (*scanfit_vector)[i].baseline);
            if ((*scanfit_vector)[i].baseline_fit)
            {
               str1 += tr("(fitted)\n");
            }
            else
            {
               str1 += tr("(fixed)\n");
            }
            ts << tr("Baseline: ") << str1;
            for (j=0; j<(*runfit).components; j++)
            {
               str1.sprintf(tr("\nInformation for component %d:\n"), j + 1);
               ts << str1;
               str1.sprintf(tr("Amplitude of component %d: %2.3e OD "),
                            j + 1, (*scanfit_vector)[i].amplitude[j]);
               if ((*scanfit_vector)[i].amplitude_fit[j])
               {
                  str1 += tr("(fitted)\n");
               }
               else
               {
                  str1 += tr("(fixed)\n");
               }
               ts << str1;
               if(calc_bottom(rotor_list, cp_list, (*scanfit_vector)[i].rotor, (*scanfit_vector)[i].centerpiece,
                              (*scanfit_vector)[i].channel, (*scanfit_vector)[i].rpm) > 0)
               {
                  if ((which_model >= 0) && (which_model <= 3))
                  {
                     str1.sprintf("Integral of component %d from Meniscus (%2.5f cm) to bottom (%2.5f cm): %2.3e OD\n",
                                  j + 1, (*scanfit_vector)[i].meniscus, calc_bottom(rotor_list, cp_list, (*scanfit_vector)[i].rotor,
                                                                                    (*scanfit_vector)[i].centerpiece,
                                                                                    (*scanfit_vector)[i].channel,
                                                                                    (*scanfit_vector)[i].rpm), (*scanfit_vector)[i].integral[j]);
                     ts << str1;
                  }
                  else if ((which_model >= 4) && (which_model <= 10))
                  {
                     str1.sprintf("Integral of monomer from Meniscus (%2.5f cm) to bottom (%2.5f cm): %2.3e OD\n",
                                  (*scanfit_vector)[i].meniscus, calc_bottom(rotor_list, cp_list, (*scanfit_vector)[i].rotor,
                                                                             (*scanfit_vector)[i].centerpiece,
                                                                             (*scanfit_vector)[i].channel,
                                                                             (*scanfit_vector)[i].rpm), (*scanfit_vector)[i].integral[0]);
                     ts << str1;
                     str1.sprintf("Integral of multimer %d from Meniscus (%2.5f cm) to bottom (%2.5f cm): %2.3e OD\n",
                                  (int)(*runfit).stoich1, (*scanfit_vector)[i].meniscus, calc_bottom(rotor_list, cp_list, (*scanfit_vector)[i].rotor,
                                                                                                     (*scanfit_vector)[i].centerpiece,
                                                                                                     (*scanfit_vector)[i].channel,
                                                                                                     (*scanfit_vector)[i].rpm),
                                  (*scanfit_vector)[i].integral[1]);
                     ts << str1;
                  }
                  else if ((which_model >= 11) && (which_model <= 13))
                  {
                     str1.sprintf("Integral of monomer from Meniscus (%2.5f cm) to bottom (%2.5f cm): %2.3e OD\n",
                                  (*scanfit_vector)[i].meniscus, calc_bottom(rotor_list, cp_list, (*scanfit_vector)[i].rotor,
                                                                             (*scanfit_vector)[i].centerpiece,
                                                                             (*scanfit_vector)[i].channel,
                                                                             (*scanfit_vector)[i].rpm), (*scanfit_vector)[i].integral[0]);
                     ts << str1;
                     str1.sprintf("Integral of multimer %d from Meniscus (%2.5f cm) to bottom (%2.5f cm): %2.3e OD\n",
                                  (int)(*runfit).stoich1, (*scanfit_vector)[i].meniscus, calc_bottom(rotor_list, cp_list, (*scanfit_vector)[i].rotor,
                                                                                                     (*scanfit_vector)[i].centerpiece,
                                                                                                     (*scanfit_vector)[i].channel,
                                                                                                     (*scanfit_vector)[i].rpm),
                                  (*scanfit_vector)[i].integral[1]);
                     ts << str1;
                     str1.sprintf("Integral of multimer %d from Meniscus (%2.5f cm) to bottom (%2.5f cm): %2.3e OD\n",
                                  (int)(*runfit).stoich2, (*scanfit_vector)[i].meniscus, calc_bottom(rotor_list, cp_list, (*scanfit_vector)[i].rotor,
                                                                                                     (*scanfit_vector)[i].centerpiece,
                                                                                                     (*scanfit_vector)[i].channel,
                                                                                                     (*scanfit_vector)[i].rpm),
                                  (*scanfit_vector)[i].integral[2]);
                     ts << str1;
                  }
                  else if (which_model == 14)
                  {
                     str1.sprintf("Integral of component A from Meniscus (%2.5f cm) to bottom (%2.5f cm): %2.3e OD\n",
                                  (*scanfit_vector)[i].meniscus, calc_bottom(rotor_list, cp_list,
                                                                             (*scanfit_vector)[i].rotor,
                                                                             (*scanfit_vector)[i].centerpiece,
                                                                             (*scanfit_vector)[i].channel,
                                                                             (*scanfit_vector)[i].rpm),
                                  (*scanfit_vector)[i].integral[0]);
                     ts << str1;
                     str1.sprintf("Integral of Component B from Meniscus (%2.5f cm) to bottom (%2.5f cm): %2.3e OD\n",
                                  (*scanfit_vector)[i].meniscus, calc_bottom(rotor_list, cp_list,
                                                                             (*scanfit_vector)[i].rotor,
                                                                             (*scanfit_vector)[i].centerpiece,
                                                                             (*scanfit_vector)[i].channel,
                                                                             (*scanfit_vector)[i].rpm),
                                  (*scanfit_vector)[i].integral[1]);
                     ts << str1;
                     str1.sprintf("Integral of AB Association from Meniscus (%2.5f cm) to bottom (%2.5f cm): %2.3e OD\n",
                                  (*scanfit_vector)[i].meniscus, calc_bottom(rotor_list, cp_list,
                                                                             (*scanfit_vector)[i].rotor,
                                                                             (*scanfit_vector)[i].centerpiece,
                                                                             (*scanfit_vector)[i].channel,
                                                                             (*scanfit_vector)[i].rpm),
                                  (*scanfit_vector)[i].integral[2]);
                     ts << str1;
                  }
               }
               str1.sprintf(tr("Extinction Coefficient for component %d: %2.3e\n"), j + 1, (*scanfit_vector)[i].extinction[j]);
               ts << str1;
               float vbar = adjust_vbar20((*runfit).vbar20[j], (*scanfit_vector)[i].temperature);
               str1.sprintf(tr("Partial Specific Volume for component %d (at %2.1fºC): %1.4f\n"), j + 1, (*scanfit_vector)[i].temperature, vbar);
               ts << str1;
               float buoyancy_tb = (1.0 - (vbar * (*model_control)->calc_density_tb((*scanfit_vector)[i].density, (*scanfit_vector)[i].temperature)));
               float buoyancy = (1.0 - ((*runfit).vbar20[j] * (*scanfit_vector)[i].density));
               str1.sprintf(tr("Buoyancy (20ºC, H2O):    %2.3e\n"), buoyancy);
               ts << str1;
               str1.sprintf(tr("Buoyancy, absolute:      %2.3e\n"), buoyancy_tb);
               ts << str1;
            }
            str1.sprintf(tr("Density Setting:         %2.3e g/ccm\n"), (*scanfit_vector)[i].density);
            ts << str1;
            str1.sprintf(tr("Density, absolute:       %2.3e g/ccm\n"), (*model_control)->calc_density_tb((*scanfit_vector)[i].density, (*scanfit_vector)[i].temperature));
            ts << str1;
            str1.sprintf(tr("Pathlength:              %2.3e cm\n"), (*scanfit_vector)[i].pathlength);
            ts << str1;
            ts << tr("\nFitting Statistics for this Scan:\n");
            k=0;
            while ((*run_information)[k].run_id != (*scanfit_vector)[i].run_id)
            {
               k++;   // find the correct runinfo to match up with for the point_density
            }
            str1.sprintf(tr("Raw Point Density:       %2.3e\n"), (*run_information)[k].point_density[(*scanfit_vector)[i].cell][(*scanfit_vector)[i].lambda][(*scanfit_vector)[i].channel]);
            ts << str1;
            scale = ((*run_information)[k].delta_r / (*run_information)[k].point_density[(*scanfit_vector)[i].cell][(*scanfit_vector)[i].lambda][(*scanfit_vector)[i].channel]);
            total_neg = (*scanfit_vector)[i].neg * scale;
            total_pos = (*scanfit_vector)[i].pos * scale;
            percentage = (float) (total_pos + total_neg);
            percentage = (float) (*scanfit_vector)[i].runs / percentage;
            percentage *= 100.0;
            str1.sprintf(tr("Number of Runs:          %d (%2.3f"), (*scanfit_vector)[i].runs, percentage);
            ts << str1 << tr(" %, corrected)\n");
            //
            // Formula for expected number of runs (P = nummber of positives, N = number of Negatives):
            // E(r) = 1 + 2PN/(P + N)
            // Formula for its variance:
            // V(R) = [2PN (2PN - P - N)] / [(P + N)^2 (P + N - 1)]
            //
            runs_expected = 1.0 + (2.0 * total_pos * total_neg) / (total_pos + total_neg);
            runs_variance = ((2.0 * total_pos * total_neg) *
                             (2.0 * total_pos * total_neg - total_pos - total_neg))
               / ((pow((total_pos + total_neg), 2)) * (total_pos + total_neg - 1.0));
            str1.sprintf(tr("Expected Number of Runs: %d (corrected)\n"
                            "Run Variance:            %2.3e (corrected)\n\n"),
                         (unsigned int) runs_expected, runs_variance);
            ts << str1;
         }
         else
         {
            ts << tr("This scan has been excluded from the Fit.\n");
         }
      }
   }
}

void US_EqFitter::write_data()
{
   unsigned int dataset_counter = 0, i, point_counter = 0;
   float x, xm_sqr, resid;
   QFile f1;
   QString baseName, fileName, temp_str, rm_file, str;
   baseName = USglobal->config_list.result_dir + "/" + *projectName + "-" + str.sprintf("%d", which_model);
   i = 1;
   temp_str.sprintf(".%d.dat", i);
   rm_file = baseName + temp_str;
   while(QFile::remove(rm_file))
   {
      i++;
      temp_str.sprintf(".%d.dat", i);
      rm_file = baseName + temp_str;
   }
   for (i=0; i<(*scanfit_vector).size(); i++)
   {
      if ((*scanfit_vector)[i].FitScanFlag)
      {
         temp_str.sprintf(".%d.dat", dataset_counter + 1);
         fileName = baseName + temp_str;
         f1.setName(fileName);
         if (f1.open(IO_WriteOnly | IO_Translate))
         {
            QTextStream ts(&f1);
            ts.reset();
            ts.width(14);
            ts.setf(QTextStream::scientific);
            ts.precision(10);
//            ts.width(20);
            //cout.precision(10);
            ts << "\"x^2-x_r^2\"" << "\t" << tr("\"Fitted Data\"") <<"\t" << tr("\"Raw Data\"" )<< "\t" << tr("\"Residuals\"") << endl;
            xm_sqr = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2);
            vector<float>::iterator yval_it = (*scanfit_vector)[i].y.begin() + (*scanfit_vector)[i].start_index;
            for (unsigned int j=0; j<points_per_dataset[dataset_counter]; j++)
            {
               if (j == 0)
               {
                  x = 0.0;
               }
               else
               {
                  x = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j], 2) - xm_sqr;
               }
               resid = y_guess[point_counter] - *yval_it;
//               cout << ts.precision() << y_guess[point_counter] <<"\t" << *yval_it << "\t" << resid << endl;
               ts << x << "\t" << y_guess[point_counter] <<"\t" << *yval_it << "\t" << resid << endl;
               yval_it ++;
               point_counter++;
            }
            f1.flush();
            f1.close();
         }
         else
         {
            if (GUI)
            {
               QMessageBox::message(tr("Error:"), tr("There has been a problem accessing\n"
                                                     "the disk - please make sure you have\n"
                                                     "proper permissions and make sure the\n"
                                                     "the disk is not full"));
            }
            else
            {
               cout << tr("Error: There has been a problem accessing the disk\n")
                    << tr("please make sure you have proper permissions and make sure the\n")
                    << tr("the disk is not full\n");

            }
         }
         dataset_counter++;
      }
   }
}

void US_EqFitter::plot_overlays()
{
   if (!GUI)
   {
      return;
   }
   double **xplot = 0, **yplot_fit = 0, **yplot_raw = 0;
   unsigned int *curve_raw, *curve_fit, dataset_counter = 0, numScans = 0;
   long unsigned int point_counter = 0;
   concentration_average = 0;
   plotResiduals = false;
   QString s1, s2;
   float xm_sqr;
   double xmax = -1e6;
   xplot = new double* [datasets];
   yplot_fit = new double* [datasets];
   yplot_raw = new double* [datasets];
   curve_fit = new unsigned int [datasets];
   curve_raw = new unsigned int [datasets];
   if (plotGroup == 0)
   {
      s1 = tr("Overlays");
   }
   else if (plotGroup > 0)
   {
      if (datasets - firstScan == 0)
      {
         numScans = 1;
         s1.sprintf(tr("Overlays for fitted Scan %ld"), firstScan);
      }
      else if (datasets - firstScan < plotGroup)
      {
         numScans = datasets - firstScan + 1;
         s1.sprintf(tr("Overlays for fitted Scans %ld - %ld"), firstScan, firstScan + numScans - 1);
      }
      else
      {
         numScans = plotGroup;
         s1.sprintf(tr("Overlays for fitted Scans %ld - %ld"),  firstScan, firstScan+plotGroup-1);
      }
   }
   s2.sprintf(tr("Optical Density"));
   point_counter = 0;
   dataset_counter = 0;
   for (unsigned int i=0; i<(*scanfit_vector).size(); i++)
   {
      if ((*scanfit_vector)[i].FitScanFlag)
      {
         xplot[dataset_counter] = new double [points_per_dataset[dataset_counter]];
         yplot_fit[dataset_counter] = new double [points_per_dataset[dataset_counter]];
         yplot_raw[dataset_counter] = new double [points_per_dataset[dataset_counter]];
         xm_sqr = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2);
         vector<float>::iterator yval_it = (*scanfit_vector)[i].y.begin() + (*scanfit_vector)[i].start_index;
         for (unsigned int j=0; j<points_per_dataset[dataset_counter]; j++)
         {
            if (j == 0)
            {
               xplot[dataset_counter][j] = 0.0;
            }
            else
            {
               xplot[dataset_counter][j] = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j], 2) - xm_sqr;
            }
            yplot_fit[dataset_counter][j] = y_guess[point_counter];
            yplot_raw[dataset_counter][j] = *yval_it;
            if ((*scanfit_vector)[i].extinction[0] != 1)
            {
               concentration_average += (*yval_it)/((*scanfit_vector)[i].pathlength * (*scanfit_vector)[i].extinction[0]);
            }
            yval_it++;
            point_counter++;
         }
         xmax = max(xplot[dataset_counter][points_per_dataset[dataset_counter] - 1], xmax);
         dataset_counter++;
      }
   }
   concentration_average /= point_counter;
   QwtSymbol symbol;
   QPen p;
   p.setWidth(1);
   symbol.setSize(4);
   symbol.setStyle(QwtSymbol::Ellipse);
   if (print_plot)
   {
      p.setColor(Qt::red);
      symbol.setPen(Qt::blue);
      symbol.setBrush(Qt::yellow);
      /*
        p.setColor(Qt::white);
        symbol.setPen(Qt::black);
        symbol.setBrush(Qt::white);
      */
   }
   else
   {
      p.setColor(Qt::red);
      symbol.setPen(Qt::blue);
      symbol.setBrush(Qt::yellow);
   }
   data_plot->clear();
   data_plot->setTitle(s1);
   data_plot->setAxisTitle(QwtPlot::xBottom, tr("Radius^2 - Radius(ref)^2 in cm"));
   data_plot->setAxisTitle(QwtPlot::yLeft, s2);
   if (plotGroup > 0)
   {
      for (unsigned int i = firstScan - 1; i< numScans + firstScan - 1; i++)
      {
         curve_raw[i] = data_plot->insertCurve("raw data");
         curve_fit[i] = data_plot->insertCurve("fitted data");
         data_plot->setCurveStyle(curve_raw[i], QwtCurve::NoCurve);
         data_plot->setCurveData(curve_raw[i], xplot[i], yplot_raw[i], points_per_dataset[i]);
         data_plot->setCurveSymbol(curve_raw[i], symbol);
         data_plot->setCurveData(curve_fit[i], xplot[i], yplot_fit[i], points_per_dataset[i]);
         data_plot->setCurveStyle(curve_fit[i], QwtCurve::Lines);
         data_plot->setCurvePen(curve_fit[i], p);
      }
   }
   else
   {
      for (unsigned int i=0; i<datasets; i++)
      {
         curve_raw[i] = data_plot->insertCurve("raw data");
         curve_fit[i] = data_plot->insertCurve("fitted data");
         data_plot->setCurveStyle(curve_raw[i], QwtCurve::NoCurve);
         data_plot->setCurveData(curve_raw[i], xplot[i], yplot_raw[i], points_per_dataset[i]);
         data_plot->setCurveSymbol(curve_raw[i], symbol);
         data_plot->setCurveData(curve_fit[i], xplot[i], yplot_fit[i], points_per_dataset[i]);
         data_plot->setCurveStyle(curve_fit[i], QwtCurve::Lines);
         data_plot->setCurvePen(curve_fit[i], p);
      }
   }
   data_plot->setAxisScale(QwtPlot::xBottom, -xmax/30.0, xmax + xmax/30.0, 0);
   data_plot->replot();
   //data_plot->updatePlot();      //no updatePlot() in new version
   for (unsigned int i=0; i<datasets; i++)
   {
      delete [] xplot[i];
      delete [] yplot_raw[i];
      delete [] yplot_fit[i];
   }
   delete [] xplot;
   delete [] yplot_raw;
   delete [] yplot_fit;
   delete [] curve_fit;
   delete [] curve_raw;
   pb_print->setEnabled(true);
}

void US_EqFitter::plot_residuals()
{
   if (!GUI)
   {
      return;
   }
   double **xplot = 0, **yplot_res = 0, line_x[2], line_y[2];
   unsigned int *curve_res, dataset_counter = 0, zeroLine[5], numScans = 0;
   unsigned int l, *dataset_index = 0;
   unsigned long point_counter = 0;
   QString s1, s2, s3;
   float xm_sqr, offset = 0;
   double xmax = -1e6;
   int last_sign = 0, current_sign = 0;
   long scanMarker[5];
   if (converged)
   {
      profile_205.absorbance.clear();
      profile_205.residual.clear();
      profile_208.absorbance.clear();
      profile_208.residual.clear();
      profile_210.absorbance.clear();
      profile_210.residual.clear();
      profile_215.absorbance.clear();
      profile_215.residual.clear();
      profile_220.absorbance.clear();
      profile_220.residual.clear();
      profile_230.absorbance.clear();
      profile_230.residual.clear();
      profile_280.absorbance.clear();
      profile_280.residual.clear();
      profile_260.absorbance.clear();
      profile_260.residual.clear();
      profile_262.absorbance.clear();
      profile_262.residual.clear();
   }
   (*runfit).runs = 0;
   plotResiduals = true;
   xplot = new double* [datasets];
   yplot_res = new double* [datasets];
   curve_res = new unsigned int [datasets];
   dataset_index = new unsigned int [datasets];
   if (plotGroup > 0)
   {
      if (datasets - firstScan == 0)
      {
         numScans = 1;
         s1.sprintf(tr("Residuals from fitted Scan %ld"), firstScan);
      }
      else if (datasets - firstScan < plotGroup)
      {
         numScans = datasets - firstScan + 1;
         s1.sprintf(tr("Residuals from fitted Scans %ld - %ld"), firstScan, firstScan + numScans - 1);
      }
      else
      {
         numScans = plotGroup;
         s1.sprintf(tr("Residuals from fitted Scans %ld - %ld"),  firstScan, firstScan+plotGroup-1);
      }
   }
   else
   {
      s1 = tr("Residuals");
   }
   s2.sprintf(tr("Optical Density Difference\n"));
   point_counter = 0;
   for (unsigned int i=0; i<(*scanfit_vector).size(); i++)
   {
      if ((*scanfit_vector)[i].FitScanFlag)
      {
         (*scanfit_vector)[i].pos = 0;
         (*scanfit_vector)[i].neg = 0;
         (*scanfit_vector)[i].runs = 0;
         dataset_index[dataset_counter] = i + 1;
         xplot[dataset_counter] = new double [points_per_dataset[dataset_counter]];
         yplot_res[dataset_counter] = new double [points_per_dataset[dataset_counter]];
         xm_sqr = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2);
         vector<float>::iterator yval_it = (*scanfit_vector)[i].y.begin() + (*scanfit_vector)[i].start_index;
         for (unsigned int j=0; j<points_per_dataset[dataset_counter]; j++)
         {
            if (j == 0)
            {
               xplot[dataset_counter][j] = 0.0;
            }
            else
            {
               xplot[dataset_counter][j] = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j], 2) - xm_sqr;
            }
            yplot_res[dataset_counter][j] = y_guess[point_counter] - *yval_it;
            //cout << "point_counter: " << point_counter << ", y_guess: " << y_guess[point_counter] << ", yval: " << *yval_it << endl;
            if (converged)
            {
               switch ((*scanfit_vector)[i].wavelength)
               {
               case 205:
                  {
                     profile_205.absorbance.push_back(y_guess[point_counter]);
                     profile_205.residual.push_back(yplot_res[dataset_counter][j]);
                     break;
                  }
               case 208:
                  {
                     profile_208.absorbance.push_back(y_guess[point_counter]);
                     profile_208.residual.push_back(yplot_res[dataset_counter][j]);
                     break;
                  }
               case 210:
                  {
                     profile_210.absorbance.push_back(y_guess[point_counter]);
                     profile_210.residual.push_back(yplot_res[dataset_counter][j]);
                     break;
                  }
               case 215:
                  {
                     profile_215.absorbance.push_back(y_guess[point_counter]);
                     profile_215.residual.push_back(yplot_res[dataset_counter][j]);
                     break;
                  }
               case 220:
                  {
                     profile_220.absorbance.push_back(y_guess[point_counter]);
                     profile_220.residual.push_back(yplot_res[dataset_counter][j]);
                     break;
                  }
               case 230:
                  {
                     profile_230.absorbance.push_back(y_guess[point_counter]);
                     profile_230.residual.push_back(yplot_res[dataset_counter][j]);
                     break;
                  }
               case 260:
                  {
                     profile_260.absorbance.push_back(y_guess[point_counter]);
                     profile_260.residual.push_back(yplot_res[dataset_counter][j]);
                     break;
                  }
               case 262:
                  {
                     profile_262.absorbance.push_back(y_guess[point_counter]);
                     profile_262.residual.push_back(yplot_res[dataset_counter][j]);
                     break;
                  }
               case 280:
                  {
                     profile_280.absorbance.push_back(y_guess[point_counter]);
                     profile_280.residual.push_back(yplot_res[dataset_counter][j]);
                     break;
                  }
               }
            }
            if (yplot_res[dataset_counter][j] > 0)
            {
               (*scanfit_vector)[i].pos++;
               current_sign = 1;
            }
            else
            {
               (*scanfit_vector)[i].neg++;
               current_sign = -1;
            }
            if (j == 0)
            {
               xplot[dataset_counter][j] = 0.0;
            }
            if (j == 0)
            {
               if (yplot_res[dataset_counter][j] > 0)
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
                  (*scanfit_vector)[i].runs++;
                  last_sign = current_sign;
               }
            }
            yval_it++;
            point_counter++;
         }
         xmax = max(xplot[dataset_counter][points_per_dataset[dataset_counter] - 1], xmax);
         (*runfit).runs += (*scanfit_vector)[i].runs;
         dataset_counter++;
      }
   }
   QwtSymbol symbol;
   QPen p_raw, p_zero;
   p_raw.setWidth(1);
   p_zero.setWidth(2);
   symbol.setStyle(QwtSymbol::Ellipse);
   if (print_plot)
   {
      p_raw.setColor(Qt::blue);
      p_zero.setColor(Qt::red);
      symbol.setPen(Qt::blue);
      symbol.setBrush(Qt::yellow);
      /*
        p_raw.setColor(Qt::white);
        p_zero.setColor(Qt::white);
        symbol.setPen(Qt::black);
        symbol.setBrush(Qt::black);
      */
   }
   else
   {
      p_raw.setColor(Qt::green);
      p_zero.setColor(Qt::red);
      symbol.setPen(Qt::blue);
      symbol.setBrush(Qt::yellow);
   }
   data_plot->clear();
   data_plot->setTitle(s1);
   data_plot->setAxisTitle(QwtPlot::xBottom, tr("Radius^2 - Radius(ref)^2 in cm"));
   data_plot->setAxisTitle(QwtPlot::yLeft, s2);
   line_y[0] = 0.0;
   line_y[1] = 0.0;
   l = 0;
   if (plotGroup > 0)
   {
      symbol.setSize(8);
      line_x[0] = -xmax/30.0;
      line_x[1] = xmax/30.0 + xmax;
      for (unsigned int i=firstScan - 1; i<numScans + firstScan - 1; i++)
      {
         curve_res[i] = data_plot->insertCurve("residuals");
         data_plot->setCurveStyle(curve_res[i], QwtCurve::Lines);
         if (i != firstScan - 1)
         {
            for (unsigned int j=0; j<points_per_dataset[i]; j++)
            {
               yplot_res[i][j] += offset;
            }
            line_y[0] = offset;
            line_y[1] = offset;
         }
         data_plot->setCurveData(curve_res[i], xplot[i], yplot_res[i], points_per_dataset[i]);
         data_plot->setCurveSymbol(curve_res[i], symbol);
         data_plot->setCurvePen(curve_res[i], p_raw);
         zeroLine[l] = data_plot->insertCurve("Zero Line");
         data_plot->setCurveData(zeroLine[l], line_x, line_y, 2);
         data_plot->setCurveStyle(zeroLine[l], QwtCurve::Lines);
         data_plot->setCurvePen(zeroLine[l], p_zero);
         scanMarker[l] = data_plot->insertMarker();
         data_plot->setMarkerLabel(scanMarker[l], s3.sprintf("%d", dataset_index[i]));
         data_plot->setMarkerPos(scanMarker[l], line_x[1] + xmax/40, offset);
         data_plot->setMarkerPen(scanMarker[l], p_zero);
         data_plot->setMarkerFont(scanMarker[l], QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
         offset += (float) 0.03;
         l++;
      }
      data_plot->setAxisScale(QwtPlot::xBottom, line_x[0], xmax + xmax/10, 0);
   }
   else
   {
      symbol.setSize(5);
      line_x[0] = -xmax/30.0;
      line_x[1] = xmax/30.0 + xmax;
      for (unsigned int i=0; i<datasets; i++)
      {
         curve_res[i] = data_plot->insertCurve("residuals");
         data_plot->setCurveStyle(curve_res[i], QwtCurve::Lines);
         data_plot->setCurveData(curve_res[i], xplot[i], yplot_res[i], points_per_dataset[i]);
         data_plot->setCurveSymbol(curve_res[i], symbol);
         data_plot->setCurvePen(curve_res[i], p_raw);
      }
      zeroLine[0] = data_plot->insertCurve("Zero Line");
      data_plot->setCurveData(zeroLine[0], line_x, line_y, 2);
      data_plot->setCurveStyle(zeroLine[0], QwtCurve::Lines);
      data_plot->setCurvePen(zeroLine[0], p_zero);
      data_plot->setAxisScale(QwtPlot::xBottom, line_x[0], line_x[1], 0);
   }
   data_plot->replot();
   //data_plot->updatePlot();      //no updatePlot() in new version
   for (unsigned int i=0; i<datasets; i++)
   {
      delete [] xplot[i];
      delete [] yplot_res[i];
   }
   delete [] xplot;
   delete [] yplot_res;
   delete [] curve_res;
   delete [] dataset_index;
   if (converged) // write out machine profile
   {
      //cout << "now writing profile data...\n";
      QFile f;
      QString FileName, str;
      if(profile_205.absorbance.size() > 0)
      {
         FileName = USglobal->config_list.result_dir + "/" + *projectName + "-" + str.sprintf("%d", which_model) + ".optical_profile-205.res";
         f.setName(FileName);
         if (f.open(IO_WriteOnly))
         {
            QTextStream ts(&f);
            for (unsigned int i=0; i<profile_205.absorbance.size(); i++)
            {
               ts << profile_205.absorbance[i] << "\t" << profile_205.residual[i] << endl;
            }
            f.close();
         }
      }
      if(profile_208.absorbance.size() > 0)
      {
         FileName = USglobal->config_list.result_dir + "/" + *projectName + "-" + str.sprintf("%d", which_model) + ".optical_profile-208.res";
         f.setName(FileName);
         if (f.open(IO_WriteOnly))
         {
            QTextStream ts(&f);
            for (unsigned int i=0; i<profile_208.absorbance.size(); i++)
            {
               ts << profile_208.absorbance[i] << "\t" << profile_208.residual[i] << endl;
            }
            f.close();
         }
      }
      if(profile_210.absorbance.size() > 0)
      {
         FileName = USglobal->config_list.result_dir + "/" + *projectName + "-" + str.sprintf("%d", which_model) + ".optical_profile-210.res";
         f.setName(FileName);
         if (f.open(IO_WriteOnly))
         {
            QTextStream ts(&f);
            for (unsigned int i=0; i<profile_210.absorbance.size(); i++)
            {
               ts << profile_210.absorbance[i] << "\t" << profile_210.residual[i] << endl;
            }
            f.close();
         }
      }
      if(profile_215.absorbance.size() > 0)
      {
         FileName = USglobal->config_list.result_dir + "/" + *projectName + "-" + str.sprintf("%d", which_model) + ".optical_profile-215.res";
         f.setName(FileName);
         if (f.open(IO_WriteOnly))
         {
            QTextStream ts(&f);
            for (unsigned int i=0; i<profile_215.absorbance.size(); i++)
            {
               ts << profile_215.absorbance[i] << "\t" << profile_215.residual[i] << endl;
            }
            f.close();
         }
      }
      if(profile_220.absorbance.size() > 0)
      {
         FileName = USglobal->config_list.result_dir + "/" + *projectName + "-" + str.sprintf("%d", which_model) + ".optical_profile-220.res";
         f.setName(FileName);
         if (f.open(IO_WriteOnly))
         {
            QTextStream ts(&f);
            for (unsigned int i=0; i<profile_220.absorbance.size(); i++)
            {
               ts << profile_220.absorbance[i] << "\t" << profile_220.residual[i] << endl;
            }
            f.close();
         }
      }
      if(profile_230.absorbance.size() > 0)
      {
         FileName = USglobal->config_list.result_dir + "/" + *projectName + "-" + str.sprintf("%d", which_model) + ".optical_profile-230.res";
         f.setName(FileName);
         if (f.open(IO_WriteOnly))
         {
            QTextStream ts(&f);
            for (unsigned int i=0; i<profile_230.absorbance.size(); i++)
            {
               ts << profile_230.absorbance[i] << "\t" << profile_230.residual[i] << endl;
            }
            f.close();
         }
      }
      if(profile_260.absorbance.size() > 0)
      {
         FileName = USglobal->config_list.result_dir + "/" + *projectName + "-" + str.sprintf("%d", which_model) + ".optical_profile-260.res";
         f.setName(FileName);
         if (f.open(IO_WriteOnly))
         {
            QTextStream ts(&f);
            for (unsigned int i=0; i<profile_260.absorbance.size(); i++)
            {
               ts << profile_260.absorbance[i] << "\t" << profile_260.residual[i] << endl;
            }
            f.close();
         }
      }
      if(profile_262.absorbance.size() > 0)
      {
         FileName = USglobal->config_list.result_dir + "/" + *projectName + "-" + str.sprintf("%d", which_model) + ".optical_profile-262.res";
         f.setName(FileName);
         if (f.open(IO_WriteOnly))
         {
            QTextStream ts(&f);
            for (unsigned int i=0; i<profile_262.absorbance.size(); i++)
            {
               ts << profile_262.absorbance[i] << "\t" << profile_262.residual[i] << endl;
            }
            f.close();
         }
      }
      if(profile_280.absorbance.size() > 0)
      {
         FileName = USglobal->config_list.result_dir + "/" + *projectName + "-" + str.sprintf("%d", which_model) + ".optical_profile-280.res";
         f.setName(FileName);
         if (f.open(IO_WriteOnly))
         {
            QTextStream ts(&f);
            for (unsigned int i=0; i<profile_280.absorbance.size(); i++)
            {
               ts << profile_280.absorbance[i] << "\t" << profile_280.residual[i] << endl;
            }
            f.close();
         }
      }
   }
}

void US_EqFitter::setRange(double scan)
{
   if (!GUI)
   {
      return;
   }
   firstScan = (int) scan;
   if (firstScan > datasets)
   {
      firstScan = 1;
   }
   if (plotGroup == 0) // this is for plotting all residuals
   {
      cnt_scan->setRange(1, 1, 1);
   }
   else if (plotGroup == 1) // this is for plotting single scans
   {
      cnt_scan->setRange(1, datasets, 1);
   }
   else if (plotGroup == 5) // this is for plotting 5 scans at a time
   {
      cnt_scan->setRange(1, datasets-(datasets % plotGroup), plotGroup);
   }
}

void US_EqFitter::updateRange(double scan)
{
   setRange(scan);
   //cnt_scan->setValue(firstScan);
   if (plotResiduals)
   {
      plot_residuals();
   }
   else
   {
      plot_overlays();
   }
}

// plot the second plot - Ln(C) vs. R^2
void US_EqFitter::plot_two()
{
   QPixmap p;
   QString str;
   if (!GUI)
   {
      return;
   }
   if(which_model >= 0 && which_model < 4)
   {
      double **xplot = 0;
      unsigned int *curve_lnc, dataset_counter = 0, numScans = 0;
      long unsigned int point_counter = 0;
      curve_lnc = new unsigned int [datasets];
      QString s1, s2;
      float xm_sqr;
      double xmax = -1e6;
      xplot = new double* [datasets];
      if (plotGroup > 0)
      {
         if (datasets - firstScan == 0)
         {
            numScans = 1;
            s1.sprintf(tr("Ln(C) vs R^2 for Scan %ld"), firstScan);
         }
         else if (datasets - firstScan < plotGroup)
         {
            numScans = datasets - firstScan + 1;
            s1.sprintf(tr("Ln(C) vs R^2 for Scans %ld - %ld"), firstScan, firstScan + numScans - 1);
         }
         else
         {
            numScans = plotGroup;
            s1.sprintf(tr("Ln(C) vs R^2 for Scan %ld - %ld"),  firstScan, firstScan+plotGroup-1);
         }
      }
      else
      {
         s1 = tr("Ln(C) vs R^2 Scans");
      }
      s2.sprintf(tr("Optical Density"));
      point_counter = 0;
      dataset_counter = 0;
      for (unsigned int i=0; i<(*scanfit_vector).size(); i++)
      {
         if ((*scanfit_vector)[i].FitScanFlag)
         {
            xplot[dataset_counter] = new double [logpoints_per_dataset[dataset_counter]];
            xm_sqr = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2);
            for (unsigned int j=0; j<logpoints_per_dataset[dataset_counter]; j++)
            {
               if (j == 0 && logpoints_per_dataset[dataset_counter] == points_per_dataset[dataset_counter])
               {
                  xplot[dataset_counter][j] = 0.0;
               }
               else   // we need to shift data by the amount of excluded points (in lncr2 calculation) to right
               {
                  xplot[dataset_counter][j] = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j
                                                                         + points_per_dataset[dataset_counter] - logpoints_per_dataset[dataset_counter]], 2) - xm_sqr;
               }
            }
            if (logpoints_per_dataset[dataset_counter] > 10)
            {
               xmax = max(xplot[dataset_counter][logpoints_per_dataset[dataset_counter] - 1], xmax);
            }
            dataset_counter++;
         }
      }
      QString lncFile = USglobal->config_list.result_dir + "/" + *projectName
         + str.sprintf("-%d", which_model) + ".lncr2";
      QFile output;
      output.setName(lncFile);
      // find largest dataset:
      unsigned int most_points = 0, largest_set = 0;
      dataset_counter = 0;
      for (unsigned int i=0; i<(*scanfit_vector).size(); i++)
      {
         if ((*scanfit_vector)[i].FitScanFlag)
         {
            if (most_points < logpoints_per_dataset[dataset_counter])
            {
               most_points = logpoints_per_dataset[dataset_counter];
               largest_set = dataset_counter;
            }
            dataset_counter++;
         }
      }
      // most_points now holds the number of datapoints of the largest dataset, and largest_set is the index of the set.
      if (output.open(IO_WriteOnly | IO_Translate))
      {
         QTextStream ts(&output);
         for (unsigned int j=0; j<logpoints_per_dataset[largest_set] - 1; j++)
         {
            dataset_counter = 0;
            for (unsigned int i=0; i<(*scanfit_vector).size(); i++)
            {
               if ((*scanfit_vector)[i].FitScanFlag)
               {
                  if(j < logpoints_per_dataset[dataset_counter] - 1)
                  {
                     ts << xplot[dataset_counter][j] << ", " << lncr2[dataset_counter][j] << ", ";
                  }
                  else
                  {
                     ts << ", , ";
                  }
                  dataset_counter++;
               }
            }
            ts << "\n";
         }
         output.close();
      }
      else
      {
         QMessageBox::message(tr("Error:"), tr("There has been a problem trying to write\n"
                                               "the file:\n\n") + lncFile + tr("\n\n"
                                                                               "Please make sure you have the proper permissions\n"
                                                                               "permissions and make sure the disk is not full"));
      }
      QwtSymbol symbol;
      QPen p;
      p.setColor(Qt::red);
      p.setWidth(2);
      symbol.setSize(10);
      symbol.setPen(Qt::blue);
      symbol.setBrush(Qt::yellow);
      symbol.setStyle(QwtSymbol::Ellipse);
      data_plot->clear();
      data_plot->setTitle(s1);
      data_plot->setAxisTitle(QwtPlot::xBottom, tr("Radius^2 - Radius(ref)^2 in cm"));
      data_plot->setAxisTitle(QwtPlot::yLeft, s2);
      /*
        for (unsigned int i=0; i<datasets; i++)
        {
        cout << xplot[i][0] << ", " << xplot[i][points_per_dataset[i]-1] << ", " << lncr2[i][0] << ", " << lncr2[i][points_per_dataset[i]-1] << endl;
        }
      */
      if (plotGroup > 0)
      {
         for (unsigned int i = firstScan - 1; i< numScans + firstScan - 1; i++)
         {
            if (logpoints_per_dataset[i] > 10)
            {
               curve_lnc[i] = data_plot->insertCurve("lnc vs. R^2 data");
               data_plot->setCurveData(curve_lnc[i], xplot[i], lncr2[i], logpoints_per_dataset[i]);
               data_plot->setCurveStyle(curve_lnc[i], QwtCurve::Lines);
               data_plot->setCurvePen(curve_lnc[i], p);
            }
         }
      }
      else
      {
         for (unsigned int i=0; i<datasets; i++)
         {
            if (logpoints_per_dataset[i] > 10)
            {
               curve_lnc[i] = data_plot->insertCurve("lnc vs. R^2 data");
               data_plot->setCurveData(curve_lnc[i], xplot[i], lncr2[i], logpoints_per_dataset[i]);
               data_plot->setCurveStyle(curve_lnc[i], QwtCurve::Lines);
               data_plot->setCurvePen(curve_lnc[i], p);
            }
         }
      }
      data_plot->setAxisScale(QwtPlot::xBottom, -xmax/30.0, xmax + xmax/30.0, 0);
      data_plot->replot();
      //data_plot->updatePlot();      //no updatePlot() in new version
      for (unsigned int i=0; i<datasets; i++)
      {
         delete [] xplot[i];
      }

      delete [] xplot;
      delete [] curve_lnc;
   }
   else if (which_model >= 4 && which_model <= 10)
   {
      sas_w = new US_Sassoc((*runfit).eq[0], -1.0e10, (*runfit).stoich1, 1.0, *projectName, which_model, false);
      sas_w->show();
      sas_w->write_data();
      QString f = htmlDir + "/" + *projectName + "-" + str.sprintf("%d", which_model) + ".assoc.";
      p = QPixmap::grabWidget(sas_w->data_plot, 2, 2, sas_w->data_plot->width() - 4, sas_w->data_plot->height() - 4);
      pm->save_file(f, p);
   }
   else if (which_model >= 11 && which_model <= 13)
   {
      sas_w = new US_Sassoc((*runfit).eq[0], (*runfit).eq[1], (*runfit).stoich1, (*runfit).stoich2, *projectName, which_model, false);
      sas_w->show();
      sas_w->write_data();
      QString f = htmlDir + "/" + *projectName + "-" + str.sprintf("%d", which_model) + ".assoc.";
      p = QPixmap::grabWidget(sas_w->data_plot, 2, 2, sas_w->data_plot->width() - 4, sas_w->data_plot->height() - 4);
      pm->save_file(f, p);
   }
}

// MW vs. R^2
void US_EqFitter::plot_three()
{
   if (!GUI)
   {
      return;
   }
   double **xplot = 0;
   unsigned int *curve_lnc, dataset_counter = 0, numScans = 0;
   long unsigned int point_counter = 0;
   curve_lnc = new unsigned int [datasets];
   QString s1, s2, str;
   float xm_sqr;
   double xmax = -1e6;
   xplot = new double* [datasets];
   if (plotGroup > 0)
   {
      if (datasets - firstScan == 0)
      {
         numScans = 1;
         s1.sprintf(tr("MW vs R^2 for Scan %ld"), firstScan);
      }
      else if (datasets - firstScan < plotGroup)
      {
         numScans = datasets - firstScan + 1;
         s1.sprintf(tr("MW vs R^2 for Scans %ld - %ld"), firstScan, firstScan + numScans - 1);
      }
      else
      {
         numScans = plotGroup;
         s1.sprintf(tr("MW vs R^2 for Scan %ld - %ld"),  firstScan, firstScan+plotGroup-1);
      }
   }
   else
   {
      s1 = tr("MW vs R^2 Scans");
   }
   s2.sprintf(tr("Molecular Weight"));
   point_counter = 0;
   dataset_counter = 0;
   for (unsigned int i=0; i<(*scanfit_vector).size(); i++)
   {
      if ((*scanfit_vector)[i].FitScanFlag)
      {
         xplot[dataset_counter] = new double [logpoints_per_dataset[dataset_counter]];
         xm_sqr = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2);
         for (unsigned int j=0; j<logpoints_per_dataset[dataset_counter]; j++)
         {
            if (j == 0 && logpoints_per_dataset[dataset_counter] == points_per_dataset[dataset_counter])
            {
               xplot[dataset_counter][j] = 0.0;
            }
            else   // we need to shift data by the amount of excluded points (in lncr2 calculation) to right
            {
               xplot[dataset_counter][j] = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j
                                                                      + points_per_dataset[dataset_counter] - logpoints_per_dataset[dataset_counter]], 2) - xm_sqr;
            }
         }
         xmax = max(xplot[dataset_counter][logpoints_per_dataset[dataset_counter] - 1], xmax);
         dataset_counter++;
      }
   }
   QString lncFile = USglobal->config_list.result_dir + "/" + *projectName
      + str.sprintf("-%d", which_model) + ".mwr2";
   QFile output;
   output.setName(lncFile);
   // find largest dataset:
   unsigned int most_points = 0, largest_set = 0;
   dataset_counter = 0;
   for (unsigned int i=0; i<(*scanfit_vector).size(); i++)
   {
      if ((*scanfit_vector)[i].FitScanFlag)
      {
         if (most_points < logpoints_per_dataset[dataset_counter])
         {
            most_points = logpoints_per_dataset[dataset_counter];
            largest_set = dataset_counter;
         }
         dataset_counter++;
      }
   }
   // most_points now holds the number of datapoints of the largest dataset, and largest_set is the index of the set.
   if (output.open(IO_WriteOnly | IO_Translate))
   {
      QTextStream ts(&output);
      for (unsigned int j=0; j<logpoints_per_dataset[largest_set] - 1; j++)
      {
         dataset_counter = 0;
         for (unsigned int i=0; i<(*scanfit_vector).size(); i++)
         {
            if ((*scanfit_vector)[i].FitScanFlag)
            {
               if(j < logpoints_per_dataset[dataset_counter] - 1)
               {
                  ts << xplot[dataset_counter][j] << ", " << dlncr2[dataset_counter][j] << ", ";
               }
               else
               {
                  ts << ", , ";
               }
               dataset_counter++;
            }
         }
         ts << "\n";
      }
      output.close();
   }
   else
   {
      QMessageBox::message(tr("Error:"), tr("There has been a problem trying to write\n"
                                            "the file:\n\n") + lncFile + tr("\n\n"
                                                                            "Please make sure you have the proper permissions\n"
                                                                            "permissions and make sure the disk is not full"));
   }
   QwtSymbol symbol;
   QPen p;
   p.setColor(Qt::red);
   p.setWidth(2);
   symbol.setSize(10);
   symbol.setPen(Qt::blue);
   symbol.setBrush(Qt::yellow);
   symbol.setStyle(QwtSymbol::Ellipse);
   data_plot->clear();
   data_plot->setTitle(s1);
   data_plot->setAxisTitle(QwtPlot::xBottom, tr("Radius^2 - Radius(ref)^2 in cm"));
   data_plot->setAxisTitle(QwtPlot::yLeft, s2);
   if (plotGroup > 0)
   {
      for (unsigned int i = firstScan - 1; i< numScans + firstScan - 1; i++)
      {
         if (logpoints_per_dataset[i] > 10)
         {
            curve_lnc[i] = data_plot->insertCurve("MW vs. R^2 data");
            data_plot->setCurveData(curve_lnc[i], xplot[i], dlncr2[i], logpoints_per_dataset[i] - 1);
            data_plot->setCurveStyle(curve_lnc[i], QwtCurve::Lines);
            data_plot->setCurvePen(curve_lnc[i], p);
         }
      }
   }
   else
   {
      for (unsigned int i=0; i<datasets; i++)
      {
         if (logpoints_per_dataset[i] > 10)
         {
            curve_lnc[i] = data_plot->insertCurve("MW vs. R^2 data");
            data_plot->setCurveData(curve_lnc[i], xplot[i], dlncr2[i], logpoints_per_dataset[i] - 1);
            data_plot->setCurveStyle(curve_lnc[i], QwtCurve::Lines);
            data_plot->setCurvePen(curve_lnc[i], p);
         }
      }
   }

   data_plot->setAxisScale(QwtPlot::xBottom, -xmax/30.0, xmax + xmax/30.0, 0);
   data_plot->replot();
   //data_plot->updatePlot();      //no updatePlot() in new version
   for (unsigned int i=0; i<datasets; i++)
   {
      delete [] xplot[i];
   }
   delete [] xplot;
   delete [] curve_lnc;
}

// MW vs. Concentration
void US_EqFitter::plot_four()
{
   if (!GUI)
   {
      return;
   }
   double **xplot = 0;
   unsigned int *curve_lnc, dataset_counter = 0, numScans = 0;
   long unsigned int point_counter = 0;
   curve_lnc = new unsigned int [datasets];
   QString s1, s2;
   float xm_sqr;
   double xmax = -1e6;
   xplot = new double* [datasets];
   if (plotGroup > 0)
   {
      if (datasets - firstScan == 0)
      {
         numScans = 1;
         s1.sprintf(tr("MW vs C for Scan %ld"), firstScan);
      }
      else if (datasets - firstScan < plotGroup)
      {
         numScans = datasets - firstScan + 1;
         s1.sprintf(tr("MW vs C for Scans %ld - %ld"), firstScan, firstScan + numScans - 1);
      }
      else
      {
         numScans = plotGroup;
         s1.sprintf(tr("MW vs C for Scan %ld - %ld"),  firstScan, firstScan+plotGroup-1);
      }
   }
   else
   {
      s1 = tr("MW vs C Scans");
   }
   s2.sprintf(tr("Molecular Weight"));
   point_counter = 0;
   dataset_counter = 0;
   for (unsigned int i=0; i<(*scanfit_vector).size(); i++)
   {
      if ((*scanfit_vector)[i].FitScanFlag)
      {
         xplot[dataset_counter] = new double [logpoints_per_dataset[dataset_counter]];
         xm_sqr = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2);
         for (unsigned int j=0; j<logpoints_per_dataset[dataset_counter]; j++)
         {
            xplot[dataset_counter][j] = y_guess[point_counter];
            point_counter++;
         }
         if (logpoints_per_dataset[dataset_counter] > 10)
         {
            xmax = max(xplot[dataset_counter][logpoints_per_dataset[dataset_counter] - 1], xmax);
         }
         dataset_counter++;
      }
   }
   QString str;
   QString lncFile = USglobal->config_list.result_dir + "/" + *projectName
      + str.sprintf("-%d", which_model) + ".mwc";
   QFile output;
   output.setName(lncFile);
   // find largest dataset:
   unsigned int most_points = 0, largest_set = 0;
   dataset_counter = 0;
   for (unsigned int i=0; i<(*scanfit_vector).size(); i++)
   {
      if ((*scanfit_vector)[i].FitScanFlag)
      {
         if (most_points < logpoints_per_dataset[dataset_counter])
         {
            most_points = logpoints_per_dataset[dataset_counter];
            largest_set = dataset_counter;
         }
         dataset_counter++;
      }
   }
   // most_points now holds the number of datapoints of the largest dataset, and largest_set is the index of the set.
   if (output.open(IO_WriteOnly | IO_Translate))
   {
      QTextStream ts(&output);
      for (unsigned int j=0; j<logpoints_per_dataset[largest_set] - 1; j++)
      {
         dataset_counter = 0;
         for (unsigned int i=0; i<(*scanfit_vector).size(); i++)
         {
            if ((*scanfit_vector)[i].FitScanFlag)
            {
               if(j < logpoints_per_dataset[dataset_counter] - 1)
               {
                  ts << xplot[dataset_counter][j] << ", " << dlncr2[dataset_counter][j] << ", ";
               }
               else
               {
                  ts << ", , ";
               }
               dataset_counter++;
            }
         }
         ts << "\n";
      }
      output.close();
   }
   else
   {
      QMessageBox::message(tr("Error:"), tr("There has been a problem trying to write\n"
                                            "the file:\n\n") + lncFile + tr("\n\n"
                                                                            "Please make sure you have the proper permissions\n"
                                                                            "permissions and make sure the disk is not full"));
   }
   QwtSymbol symbol;
   QPen p;
   p.setWidth(2);
   symbol.setStyle(QwtSymbol::Ellipse);
   symbol.setSize(10);
   if (print_plot)
   {
      p.setColor(Qt::white);
      symbol.setPen(Qt::white);
      symbol.setBrush(Qt::black);
   }
   else
   {
      p.setColor(Qt::red);
      symbol.setPen(Qt::blue);
      symbol.setBrush(Qt::yellow);
   }
   data_plot->clear();
   data_plot->setTitle(s1);
   data_plot->setAxisTitle(QwtPlot::xBottom, tr("Optical Density"));
   data_plot->setAxisTitle(QwtPlot::yLeft, s2);
   if (plotGroup > 0)
   {
      for (unsigned int i = firstScan - 1; i< numScans + firstScan - 1; i++)
      {
         if (logpoints_per_dataset[i] > 10)
         {
            curve_lnc[i] = data_plot->insertCurve("MW vs. Concentration data");
            data_plot->setCurveData(curve_lnc[i], xplot[i], dlncr2[i], logpoints_per_dataset[i] - 1);
            data_plot->setCurveStyle(curve_lnc[i], QwtCurve::Lines);
            data_plot->setCurvePen(curve_lnc[i], p);
         }
      }
   }
   else
   {
      for (unsigned int i=0; i<datasets; i++)
      {
         if (logpoints_per_dataset[i] > 10)
         {
            curve_lnc[i] = data_plot->insertCurve("MW vs. Concentration data");
            data_plot->setCurveData(curve_lnc[i], xplot[i], dlncr2[i], logpoints_per_dataset[i] - 1);
            data_plot->setCurveStyle(curve_lnc[i], QwtCurve::Lines);
            data_plot->setCurvePen(curve_lnc[i], p);
         }
      }
   }
   data_plot->setAxisScale(QwtPlot::xBottom, -xmax/30.0, xmax + xmax/30.0, 0);
   data_plot->replot();
   //data_plot->updatePlot();      //no updatePlot() in new version
   for (unsigned int i=0; i<datasets; i++)
   {
      delete [] xplot[i];
   }
   delete [] xplot;
   delete [] curve_lnc;
   if (nlsMethod == 5)
   {
      plot_six();
   }
}

void US_EqFitter::plot_six()
{
   double *xplot, *yplot, *zplot, *xplot2;
   unsigned int i, j, k;
   QPixmap pix;
   QString filename, str;
   float buoyancy, range, c_0, constant;
   float bottom;
   float c_max = -1000.0;
   xplot = new double [(*runfit).components];
   yplot = new double [(*runfit).components];
   zplot = new double [(*runfit).components - 2];
   xplot2 = new double [(*runfit).components - 2];
   range = ((*runfit).mw[1] - (*runfit).mw[0])/2;
   for (i=0; i<(*runfit).components; i++)
   {
      xplot[i] = (*runfit).mw[i];
      yplot[i] = 0.0;
   }
   for (i=1; i<(*runfit).components-1; i++)
   {
      xplot2[i-1] = (*runfit).mw[i];
      zplot[i-1] = 0.0;
      for (j=0; j<mwpoints.size(); j++)
      {
         if(mwpoints[j] > (*runfit).mw[i] - range && mwpoints[j] < (*runfit).mw[i] + range)
         {
            zplot[i-1] ++;
         }
      }
      //      cout << "range: " << range << ", i: " << i << ": " << zplot[i] << endl;
   }
   float val;
   for (i=0; i<(*scanfit_vector).size(); i++)
   {
      if ((*scanfit_vector)[i].FitScanFlag)
      {
         bottom = calc_bottom(rotor_list, cp_list, (*scanfit_vector)[i].rotor, (*scanfit_vector)[i].centerpiece,
                              (*scanfit_vector)[i].channel, (*scanfit_vector)[i].rpm);
         for (k=0; k<(*runfit).components; k++)
         {
            //cout << "Scan " << i << ", component " << k << ": amplitude: " << (*scanfit_vector)[i].amplitude[k] << endl;
            if ((*scanfit_vector)[i].amplitude[k] != 0.0)
            {
               //cout << "!Scan " << i << ", component " << k << ": amplitude: " << (*scanfit_vector)[i].amplitude[k] << endl;
               buoyancy = (1.0 - adjust_vbar20((*runfit).vbar20[k], (*scanfit_vector)[i].temperature)
                           * (*scanfit_vector)[i].density);
               constant = (pow(M_PI * (*scanfit_vector)[i].rpm / 30, 2) * buoyancy)
                  / (R * (K0 + (*scanfit_vector)[i].temperature));
               /* Integral of equilibrium function:

               C = (c_i R T)/(M_i omega^2 (1 - vbar rho)) exp(((r^2_bottom - r^2_meniscus) M_i omega^2 (1 - vbar rho)/(2 R T)
               */
               //cout << "Scan " << i << ", component " << k << ": c0: ";
               c_0 = (*scanfit_vector)[i].amplitude[k]
                  / ((*scanfit_vector)[i].extinction[k] * (*scanfit_vector)[i].pathlength);
               //cout << c_0 << ", val: ";
               val = (c_0/(constant * (*runfit).mw[k]))
                  * exp((pow((double) (*scanfit_vector)[i].x[(*scanfit_vector)[i].stop_index], 2.0)
                         - pow((double) (*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2.0)) * (*runfit).mw[k] * constant/2);
               //cout << val << ", yplot: ";
               /*
                 if (k == 0)
                 {
                 //cout << "scan: " << i << ", c_0: " << c_0 << ", MW: " << (*runfit).mw[k] << ", val: " << val << endl;
                 //cout << "buoyancy: " << buoyancy << ", constant: " << constant << ", vbar: " << (*runfit).vbar20[k] << ", rho: " << (*scanfit_vector)[i].density << endl;
                 //cout << "rpm: " <<  (*scanfit_vector)[i].rpm << ", temp: " << (*scanfit_vector)[i].temperature << "Meniscus " << (*scanfit_vector)[i].meniscus << ", bottom: " << bottom << endl;
                 //cout << "Amplitude " << k << ": " << (*scanfit_vector)[i].amplitude[k] << endl << endl << endl;
                 }
               */
               yplot[k] += val;
               //cout << yplot[k] << endl;
               /*
                 range = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].stop_index], 2) - xm_sqr;
                 dr = range/5000;
                 sigma = constant * (*runfit).mw[k] * buoyancy;
                 sum = 0;
                 c_0 = (*scanfit_vector)[i].amplitude[k]
                 / ((*scanfit_vector)[i].extinction[0] * (*scanfit_vector)[i].pathlength);
                 temp1 = dr;
                 for (j=1; j<5000; j++) // use trapezoidal rule to estimate integral
                 {
                 xval =  j * dr;
                 temp2 = exp(sigma * xval) * dr;
                 sum += temp1 + (temp2 - temp1)/2.0;
                 temp1 = temp2;
                 }
                 yplot[k] += sum * c_0;
               */
            }
         }
      }
   }
   for (i=0; i<(*runfit).components; i++)
   {
      c_max = max((double)c_max, yplot[i]);
      //cout << i << ": " << yplot[i] << endl;
   }
   QWidget *p=0;
   histogram_plot = new QwtPlot(p);
   histogram_plot->setGeometry(global_Xpos + 60, global_Ypos + 60, 600, 500);
   histogram_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
   histogram_plot->enableGridXMin();
   histogram_plot->enableGridYMin();
   histogram_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   histogram_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
   histogram_plot->setCanvasBackground(USglobal->global_colors.plot);      //new version
   histogram_plot->setMargin(USglobal->config_list.margin);
   histogram_plot->enableOutline(true);
   histogram_plot->setAxisTitle(QwtPlot::xBottom, tr("Molecular Weight"));
   histogram_plot->setAxisTitle(QwtPlot::yLeft, tr("Relative Concentration"));
   histogram_plot->setTitle(tr("Molecular Weight Histogram"));
   histogram_plot->setOutlinePen(white);
   histogram_plot->setOutlineStyle(Qwt::Cross);
   histogram_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   histogram_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   histogram_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   histogram_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   histogram_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   histogram_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   histogram_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   histogram_plot->clear();
   long curve = histogram_plot->insertCurve("Histogram");
   long curve2 = histogram_plot->insertCurve("MW Average");
   histogram_plot->setCurveYAxis(curve, QwtPlot::yLeft);
   histogram_plot->setCurveYAxis(curve2, QwtPlot::yRight);
   histogram_plot->setCurveStyle(curve, QwtCurve::Sticks);
   histogram_plot->setCurvePen(curve, QPen(Qt::red, 7));
   histogram_plot->setCurveStyle(curve2, QwtCurve::Lines);
   histogram_plot->setCurvePen(curve2, QPen(Qt::green, 2));
   histogram_plot->setCurveData(curve2, xplot2, zplot, (*runfit).components-2);
   histogram_plot->setCurveData(curve, xplot, yplot, (*runfit).components);
   //   histogram_plot->setAxisScale(QwtPlot::yLeft, 0.0 - c_max/20.0, c_max + c_max/20.0);
   histogram_plot->setAxisAutoScale(QwtPlot::yRight);
   histogram_plot->setAxisAutoScale(QwtPlot::yLeft);
   QwtSymbol symbol;
   symbol.setSize(12);
   symbol.setStyle(QwtSymbol::Ellipse);
   symbol.setPen(Qt::blue);
   symbol.setBrush(Qt::yellow);
   histogram_plot->setCurveSymbol(curve, symbol);
   histogram_plot->show();
   histogram_plot->replot();
   filename = htmlDir + "/" + *projectName + str.sprintf("-%d", which_model) + ".mw_histogram.";
   pix = QPixmap::grabWidget(histogram_plot, 2, 2, histogram_plot->width() - 4, histogram_plot->height() - 4);
   pm->save_file(filename, pix);
   filename = htmlDir + "/" + *projectName + str.sprintf("-%d", which_model) + ".mw_histogram.dat";
   QPushButton *pb_print;
   pb_print = new QPushButton(histogram_plot, "Print");
   pb_print->setText("Print");
   pb_print->setAutoDefault(false);
   pb_print->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_print->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_print->setGeometry(0, 0, 100, 26);
   pb_print->show();
   connect(pb_print, SIGNAL(clicked()), SLOT(print_histogram()));
   QFile f(filename);
   if (f.open(IO_WriteOnly | IO_Translate))
   {
      QTextStream ts(&f);
      ts << tr("Molar Concentration\tFrequency\n");
      for (i=0; i<(*runfit).components; i++)
      {
         ts << xplot[i] << "\t" << yplot[i] << endl;
      }
   }
   f.close();
   delete [] xplot;
   delete [] xplot2;
   delete [] yplot;
   delete [] zplot;
}

// calculate the integral from the meniscus to the bottom of the cell for each exponential term
void US_EqFitter::print_histogram()
{
   QPrinter printer;
   if   (printer.setup(0))
   {
      histogram_plot->print(printer);
   }
}

void US_EqFitter::calc_integral()
{
   if (GUI)
   {
      lbl_status2->setText(tr("Calculating Integrals..."));
      qApp->processEvents();
   }
   QString str, filename;
   QFile f;
   filename = htmlDir + "/" + *projectName + "-" + str.sprintf("%d", which_model) + ".integral.txt";
   f.setName(filename);
   unsigned int i, k=0;
   float xm_sqr, xval, xval_squared, omega_sqr, *vbar, *buoyancy, constant;
   float OD_correction1, OD_correction2, bottom=0.0, sum, dr, temp1, temp2;
   if ((which_model >= 0) && (which_model <= 2))
   {
      vbar      = new float[(*runfit).components];
      buoyancy   = new float[(*runfit).components];
      for (i=0; i<(*scanfit_vector).size(); i++)
      {
         if ((*scanfit_vector)[i].FitScanFlag)
         {
            bottom = calc_bottom(rotor_list, cp_list, (*scanfit_vector)[i].rotor, (*scanfit_vector)[i].centerpiece,
                                 (*scanfit_vector)[i].channel, (*scanfit_vector)[i].rpm);
            (*scanfit_vector)[i].integral.clear();
            xm_sqr = pow((*scanfit_vector)[i].meniscus, 2);
            omega_sqr = pow(M_PI * (*scanfit_vector)[i].rpm / 30, 2);
            constant = omega_sqr/(2 * R * (K0 + (*scanfit_vector)[i].temperature));
            dr = (bottom - (*scanfit_vector)[i].meniscus)/5000.0;
            for (k=0; k<(*runfit).components; k++)
            {
               vbar[k] = adjust_vbar20((*runfit).vbar20[k], (*scanfit_vector)[i].temperature);
               buoyancy[k] = (1.0 - vbar[k] * (*scanfit_vector)[i].density);
            }
            for (k=0; k<(*runfit).components; k++)
            {
               sum = 0;
               xval = (*scanfit_vector)[i].meniscus;
               temp1 = exp ((*scanfit_vector)[i].amplitude[k]);
               for (int m=0; m<4999; m++)
               {
                  xval += dr;
                  xval_squared = pow((double) xval, 2.0) - xm_sqr;
                  temp2 = exp ((*scanfit_vector)[i].amplitude[k]
                               + ((*runfit).mw[k] * constant * buoyancy[k] * xval_squared));
                  sum += dr * (temp1 + (temp2-temp1)/2.0);
                  temp1 = temp2;
               }
               (*scanfit_vector)[i].integral.push_back(sum);
            }
         }
      }
      delete [] vbar;
      delete [] buoyancy;
   }
   else if (which_model == 3)   // this model doesn't fit the log of the amplitude, therefore we need
   {                        // another method to calculate the integral
      vbar      = new float[1];
      buoyancy   = new float[1];
      for (i=0; i<(*scanfit_vector).size(); i++)
      {
         if ((*scanfit_vector)[i].FitScanFlag)
         {
            bottom = calc_bottom(rotor_list, cp_list, (*scanfit_vector)[i].rotor, (*scanfit_vector)[i].centerpiece,
                                 (*scanfit_vector)[i].channel, (*scanfit_vector)[i].rpm);
            (*scanfit_vector)[i].integral.clear();
            for (k=0; k<(*runfit).components; k++)
            {
               buoyancy[0] = (1.0 - adjust_vbar20((*runfit).vbar20[k], (*scanfit_vector)[i].temperature)
                              * (*scanfit_vector)[i].density);
               constant = (pow(M_PI * (*scanfit_vector)[i].rpm / 30, 2) * buoyancy[0])
                  / (R * (K0 + (*scanfit_vector)[i].temperature));
               /* Integral of equilibrium function:

               C = (c_i R T)/(M_i omega^2 (1 - vbar rho)) exp(((r^2_bottom - r^2_meniscus) M_i omega^2 (1 - vbar rho)/(2 R T)
               */
               temp1 = (*scanfit_vector)[i].amplitude[k]
                  / ((*scanfit_vector)[i].extinction[k] * (*scanfit_vector)[i].pathlength);
               (*scanfit_vector)[i].integral.push_back((temp1/(constant * (*runfit).mw[k]))
                                                       * exp((pow((double) (*scanfit_vector)[i].x[(*scanfit_vector)[i].stop_index], 2.0)
                                                              - pow((double) (*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2.0)) * (*runfit).mw[k] * constant/2));
            }
         }
      }
      delete [] vbar;
      delete [] buoyancy;
   }
   else if ((which_model >= 4) && (which_model <= 10))
   {
      switch (which_model)
      {
      case 4: // Monomer-Dimer Equilibrium
         {
            (*runfit).stoich1 = 2;
            break;
         }
      case 5: // Monomer-Trimer Equilibrium
         {
            (*runfit).stoich1 = 3;
            break;
         }
      case 6: // Monomer-Tetramer Equilibrium
         {
            (*runfit).stoich1 = 4;
            break;
         }
      case 7: // Monomer-Pentamer Equilibrium
         {
            (*runfit).stoich1 = 5;
            break;
         }
      case 8: // Monomer-Hexamer Equilibrium
         {
            (*runfit).stoich1 = 6;
            break;
         }
      case 9: // Monomer-Heptamer Equilibrium
         {
            (*runfit).stoich1 = 7;
            break;
         }
      }
      vbar      = new float[1];
      buoyancy   = new float[1];
      for (i=0; i<(*scanfit_vector).size(); i++)
      {
         if ((*scanfit_vector)[i].FitScanFlag)
         {
            bottom = calc_bottom(rotor_list, cp_list, (*scanfit_vector)[i].rotor, (*scanfit_vector)[i].centerpiece,
                                 (*scanfit_vector)[i].channel, (*scanfit_vector)[i].rpm);
            (*scanfit_vector)[i].integral.clear();
            xm_sqr = pow((*scanfit_vector)[i].meniscus, 2);
            omega_sqr = pow(M_PI * (*scanfit_vector)[i].rpm / 30, 2);
            constant = omega_sqr/(2 * R * (K0 + (*scanfit_vector)[i].temperature));
            vbar[0] = adjust_vbar20((*runfit).vbar20[0], (*scanfit_vector)[i].temperature);
            buoyancy[0] = (1.0 - vbar[0] * (*scanfit_vector)[i].density);
            OD_correction1 = log((*runfit).stoich1 / pow((*scanfit_vector)[i].extinction[0]
                                                         * (*scanfit_vector)[i].pathlength, (*runfit).stoich1));
            dr = (bottom - (*scanfit_vector)[i].meniscus)/5000.0;

            sum = 0;
            xval = (*scanfit_vector)[i].meniscus;
            temp1 = exp ((*scanfit_vector)[i].amplitude[0]);
            for (int m=0; m<4999; m++)
            {
               xval += dr;
               xval_squared = pow((double) xval, 2.0) - xm_sqr;
               temp2 = exp ((*scanfit_vector)[i].amplitude[0]
                            + ((*runfit).mw[0] * constant * buoyancy[0] * xval_squared));
               sum += dr * (temp1 + (temp2-temp1)/2.0);
               temp1 = temp2;
            }
            (*scanfit_vector)[i].integral.push_back(sum);

            sum = 0;
            xval = (*scanfit_vector)[i].meniscus;
            temp1 = exp ((*runfit).stoich1 * (*scanfit_vector)[i].amplitude[0] + (*runfit).eq[0] + OD_correction1);
            for (int m=0; m<4999; m++)
            {
               xval += dr;
               xval_squared = pow((double) xval, 2.0) - xm_sqr;
               temp2 = exp ((*runfit).stoich1 * (*scanfit_vector)[i].amplitude[0]
                            + (*runfit).eq[0]   + OD_correction1
                            + ((*runfit).stoich1 * (*runfit).mw[0] * constant * buoyancy[0] * xval_squared));
               sum += dr * (temp1 + (temp2-temp1)/2.0);
               temp1 = temp2;
            }
            (*scanfit_vector)[i].integral.push_back(sum);
         }
      }
      delete [] vbar;
      delete [] buoyancy;
   }
   else if ((which_model >= 11) && (which_model <= 13))
   {
      switch (which_model)
      {
      case 11: // Monomer-Dimer-Trimer Equilibrium
         {
            (*runfit).stoich1 = 2;
            (*runfit).stoich2 = 3;
            break;
         }
      case 12: // Monomer-Dimer-Tetramer Equilibrium
         {
            (*runfit).stoich1 = 2;
            (*runfit).stoich2 = 4;
            break;
         }
      }
      vbar      = new float[1];
      buoyancy   = new float[1];
      for (unsigned int i=0; i<(*scanfit_vector).size(); i++)
      {
         if ((*scanfit_vector)[i].FitScanFlag)
         {
            bottom = calc_bottom(rotor_list, cp_list, (*scanfit_vector)[i].rotor, (*scanfit_vector)[i].centerpiece,
                                 (*scanfit_vector)[i].channel, (*scanfit_vector)[i].rpm);
            (*scanfit_vector)[i].integral.clear();
            xm_sqr = pow((*scanfit_vector)[i].meniscus, 2);
            omega_sqr = pow(M_PI * (*scanfit_vector)[i].rpm / 30, 2);
            constant = omega_sqr/(2 * R * (K0 + (*scanfit_vector)[i].temperature));
            vbar[0] = adjust_vbar20((*runfit).vbar20[0], (*scanfit_vector)[i].temperature);
            buoyancy[0] = (1.0 - vbar[0] * (*scanfit_vector)[i].density);
            OD_correction1 = log((*runfit).stoich1 / pow((*scanfit_vector)[i].extinction[0]
                                                         * (*scanfit_vector)[i].pathlength, (*runfit).stoich1));
            OD_correction2 = log((*runfit).stoich2 / pow((*scanfit_vector)[i].extinction[0]
                                                         * (*scanfit_vector)[i].pathlength, (*runfit).stoich2));
            dr = (bottom - (*scanfit_vector)[i].meniscus)/5000.0;

            sum = 0;
            xval = (*scanfit_vector)[i].meniscus;
            temp1 = exp ((*scanfit_vector)[i].amplitude[0]);
            for (int m=0; m<4999; m++)
            {
               xval += dr;
               xval_squared = pow((double) xval, 2.0) - xm_sqr;
               temp2 = exp ((*scanfit_vector)[i].amplitude[0]
                            + ((*runfit).mw[0] * constant * buoyancy[0] * xval_squared));
               sum += dr * (temp1 + (temp2-temp1)/2.0);
               temp1 = temp2;
            }
            (*scanfit_vector)[i].integral.push_back(sum);

            sum = 0;
            xval = (*scanfit_vector)[i].meniscus;
            temp1 = exp ((*runfit).stoich1 * (*scanfit_vector)[i].amplitude[0] + (*runfit).eq[0] + OD_correction1);
            for (int m=0; m<4999; m++)
            {
               xval += dr;
               xval_squared = pow((double) xval, 2.0) - xm_sqr;
               temp2 = exp ((*runfit).stoich1 * (*scanfit_vector)[i].amplitude[0]
                            + (*runfit).eq[0]   + OD_correction1
                            + ((*runfit).stoich1 * (*runfit).mw[0] * constant * buoyancy[0] * xval_squared));
               sum += dr * (temp1 + (temp2-temp1)/2.0);
               temp1 = temp2;
            }
            (*scanfit_vector)[i].integral.push_back(sum);

            sum = 0;
            xval = (*scanfit_vector)[i].meniscus;
            temp1 = exp ((*runfit).stoich2 * (*scanfit_vector)[i].amplitude[0] + (*runfit).eq[1] + OD_correction2);
            for (int m=0; m<4999; m++)
            {
               xval += dr;
               xval_squared = pow((double) xval, 2.0) - xm_sqr;
               temp2 = exp ((*runfit).stoich2 * (*scanfit_vector)[i].amplitude[0]
                            + (*runfit).eq[1]   + OD_correction2
                            + ((*runfit).stoich2 * (*runfit).mw[0] * constant * buoyancy[0] * xval_squared));
               sum += dr * (temp1 + (temp2-temp1)/2.0);
               temp1 = temp2;
            }
            (*scanfit_vector)[i].integral.push_back(sum);
         }
      }
      delete [] vbar;
      delete [] buoyancy;
   }
   else if (which_model == 14)
   {
      float extinction_ab;
      vbar      = new float[3];
      buoyancy   = new float[3];
      for (unsigned int i=0; i<(*scanfit_vector).size(); i++)
      {
         if ((*scanfit_vector)[i].FitScanFlag)
         {
            bottom = calc_bottom(rotor_list, cp_list, (*scanfit_vector)[i].rotor, (*scanfit_vector)[i].centerpiece,
                                 (*scanfit_vector)[i].channel, (*scanfit_vector)[i].rpm);
            (*scanfit_vector)[i].integral.clear();
            xm_sqr = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2);
            omega_sqr = pow(M_PI * (*scanfit_vector)[i].rpm / 30, 2);
            constant = omega_sqr/(2 * R * (K0 + (*scanfit_vector)[i].temperature));
            for (k=0; k<2; k++) // 2 components are given, the third is calculated
            {
               vbar[k] = adjust_vbar20((*runfit).vbar20[k], (*scanfit_vector)[i].temperature);
               buoyancy[k] = (1.0 - vbar[k] * (*scanfit_vector)[i].density);
            }
            vbar[2] = ((*runfit).vbar20[0] * (*runfit).mw[0]
                       + (*runfit).vbar20[1] * (*runfit).mw[1])
               / ((*runfit).mw[0] + (*runfit).mw[0]); // vbar_ab is a weight-average of vbar[1] and vbar[0]
            buoyancy[2] = (1.0 - vbar[2] * (*scanfit_vector)[i].density);
            // OD correction: e_AB/(e_A * e_B * l)
            // e_AB = (e_A * MW_A + e_B * MW_B)/(MW_A + MW_B)
            extinction_ab = (*scanfit_vector)[i].extinction[0] + (*scanfit_vector)[i].extinction[1];
            dr = (bottom - (*scanfit_vector)[i].meniscus)/5000.0;
            OD_correction1 = log(extinction_ab/
                                 ( (*scanfit_vector)[i].extinction[0]
                                   * (*scanfit_vector)[i].extinction[1]
                                   * (*scanfit_vector)[i].pathlength) );

            sum = 0;
            xval = (*scanfit_vector)[i].meniscus;
            temp1 = exp ((*scanfit_vector)[i].amplitude[0]);
            for (int m=0; m<4999; m++)
            {
               xval += dr;
               xval_squared = pow((double) xval, 2.0) - xm_sqr;
               temp2 = exp ((*scanfit_vector)[i].amplitude[0]
                            + ((*runfit).mw[0] * constant * buoyancy[0] * xval_squared));
               sum += dr * (temp1 + (temp2-temp1)/2.0);
               temp1 = temp2;
            }
            (*scanfit_vector)[i].integral.push_back(sum);

            sum = 0;
            xval = (*scanfit_vector)[i].meniscus;
            temp1 = exp ((*scanfit_vector)[i].amplitude[1]);
            for (int m=0; m<4999; m++)
            {
               xval += dr;
               xval_squared = pow((double) xval, 2.0) - xm_sqr;
               temp2 = exp ((*scanfit_vector)[i].amplitude[1]
                            + ((*runfit).mw[1] * constant * buoyancy[1] * xval_squared));
               sum += dr * (temp1 + (temp2-temp1)/2.0);
               temp1 = temp2;
            }
            (*scanfit_vector)[i].integral.push_back(sum);

            sum = 0;
            xval = (*scanfit_vector)[i].meniscus;
            temp1 = exp((*scanfit_vector)[i].amplitude[0]
                        + (*scanfit_vector)[i].amplitude[1]
                        + (*runfit).eq[0] + OD_correction1);
            for (int m=0; m<4999; m++)
            {
               xval += dr;
               xval_squared = pow((double) xval, 2.0) - xm_sqr;
               temp2 = exp((*scanfit_vector)[i].amplitude[0]
                           + (*scanfit_vector)[i].amplitude[0]
                           + (*runfit).eq[0]
                           + OD_correction1
                           + (((*runfit).mw[0] + (*runfit).mw[2]) * constant * buoyancy[2] * xval_squared));
               sum += dr * (temp1 + (temp2-temp1)/2.0);
               temp1 = temp2;
            }
            (*scanfit_vector)[i].integral.push_back(sum);
         }
      }
      delete [] vbar;
      delete [] buoyancy;
   }
}

// plot the 5th plot
void US_EqFitter::plot_five()
{
   if (!GUI)
   {
      return;
   }
   QString str, filename;
   QFile f;
   filename = htmlDir + "/" + *projectName + "-" + str.sprintf("%d", which_model) + ".comments.html";
   f.setName(filename);
   vector <QString> str_vector;
   if (f.exists())
   {
      f.open(IO_ReadOnly);
      QTextStream ts(&f);
      str = ts.readLine();
      while (str.left(9) != "<img src=")
      {
         str = ts.readLine();
      }
      while (!ts.eof())
      {
         str = ts.readLine();
         if (str == "<hr><font face=helvetica size=-1>")
         {
            break;
         }
         if (str.stripWhiteSpace() == "<p>" || str.stripWhiteSpace() == "<P>") // convert <P> tags to empty lines
         {
            str = "";
         }
         str_vector.push_back(str);
      }
      f.close();
   }
   US_htmlEdit *he;
   he = new US_htmlEdit(filename, which_model, 0, "Editor");
   for (unsigned int i=0; i<str_vector.size(); i++)
   {
      he->mle_text->insertParagraph(str_vector[i], -1);
   }
   he->show();
}

void US_EqFitter::endFit()
{
   QString str;
   // if we are fitting a multi-stage fitting profile we need to capture the relevant events (2 equilibrium constants):
   if ((which_model >= 11) && (which_model <= 13) && (*runfit).eq_fit[0]  && !(*runfit).eq_fit[1] && !initialized_2)
   {
      // scale equilibrium constant initialization with extinction coefficient, stoichiometry and pathlength
      initialized_2 = true;
      (*runfit).eq[1] = -1.0 *(*runfit).stoich2 - log((*runfit).stoich2
                                                      / pow((*scanfit_vector)[first_fitted_scan].extinction[0]
                                                            * (*scanfit_vector)[first_fitted_scan].pathlength, (*runfit).stoich2));
      (*runfit).eq_fit[1] = true;
      if (GUI)
      {
         (*model_control)->eqconst2_float(true);
         (*model_control)->fp_eqconst2->updateValue((*runfit).eq[1]);
         lbl_status4->setText(tr("Stage 2: Now converging on Equilibrium Constant 2..."));
         switch (QMessageBox::information(this, "UltraScan - " + modelString[which_model],
                                          tr("The program is about to set the second equilibrium constant to be floated for\n"
                                             "the third stage of the fitting process.\n\n"
                                             "Although a reasonable estimate will be automatically calculated, you can\n"
                                             "now change this estimate in the Model Control Window in case the fit does\n"
                                             "not converge by itself due to a poor initial estimate of the equilibrium\n"
                                             "constant.\n\n"
                                             "If you want to accept the automatic estimate of ")
                                          + str.sprintf("%2.4e", (*runfit).eq[1]) +   tr(" simply click\n"
                                                                                         "on \"Continue\" to continue fitting, otherwise, click on \"Adjust\" and adjust the\n"
                                                                                         "equilibrium constant first and then click on \"Fit\" in the fitting control\n\n"),
                                          "Continue",
                                          "Adjust", 0, 0, 1))
         {
         case 0: // The user clicked the continue button or pressed Enter
            {
               Fit();
               break;
            }
         case 1: // The user clicked the Quit or pressed Escape
            {
               cancelFit();
               break;
            }
         }
      }
   }
   if ((which_model >= 14) && (which_model <= 15) && (!(*runfit).eq_fit[0]) && !initialized_1)
   {
      // start with a small equilibrium constant initialization
      initialized_1 = true;
      (*runfit).eq[0] = -0.5;
      if (GUI)
      {
         (*model_control)->eqconst1_float(true);
         (*model_control)->fp_eqconst1->updateValue((*runfit).eq[0]);
         lbl_status4->setText(tr("Stage 2: Now converging on Equilibrium Constant 1..."));
         switch (QMessageBox::information(this, "UltraScan - " + modelString[which_model],
                                          tr("The program is about to set the equilibrium constant to be floated for\n"
                                             "the third stage of the fitting process.\n\n"
                                             "Although a reasonable estimate will be automatically calculated, you can\n"
                                             "now change this estimate in the Model Control Window in case the fit does\n"
                                             "not converge by itself due to a poor initial estimate of the equilibrium\n"
                                             "constant.\n\n"
                                             "If you want to accept the automatic estimate of ")
                                          + str.sprintf("%2.4e", (*runfit).eq[0]) +   tr(" simply click\n"
                                                                                         "on \"Continue\" to continue fitting, otherwise, click on \"Adjust\" and adjust the\n"
                                                                                         "equilibrium constant first and then click on \"Fit\" in the fitting control\n\n"),
                                          "Continue",
                                          "Adjust", 0, 0, 1))
         {
         case 0: // The user clicked the continue button or pressed Enter
            {
               Fit();
               break;
            }
         case 1: // The user clicked the Quit or pressed Escape
            {
               cancelFit();
               break;
            }
         }
      }
   }
   if ((which_model == 15) && (*runfit).eq_fit[0]  && !(*runfit).eq_fit[1] && !initialized_2)
   {
      initialized_2 = true;

      // scale equilibrium constant initialization with extinction coefficient, stoichiometry and pathlength
      (*runfit).eq[1] = -1.0 *(*runfit).stoich1 - log((*runfit).stoich1
                                                      / pow((*scanfit_vector)[first_fitted_scan].extinction[0]
                                                            * (*scanfit_vector)[first_fitted_scan].pathlength, (*runfit).stoich1));
      (*runfit).eq_fit[1] = true;
      if (GUI)
      {
         (*model_control)->eqconst2_float(true);
         (*model_control)->fp_eqconst2->updateValue((*runfit).eq[1]);
         lbl_status4->setText(tr("Stage 2: Now converging on Monomer-Dimer Equilibrium Constant..."));
         switch (QMessageBox::information(this, "UltraScan - " + modelString[which_model],
                                          tr("The program is about to set the second equilibrium constant to be floated for\n"
                                             "the third stage of the fitting process.\n\n"
                                             "Although a reasonable estimate will be automatically calculated, you can\n"
                                             "now change this estimate in the Model Control Window in case the fit does\n"
                                             "not converge by itself due to a poor initial estimate of the equilibrium\n"
                                             "constant.\n\n"
                                             "If you want to accept the automatic estimate of ")
                                          + str.sprintf("%2.4e", (*runfit).eq[1]) +   tr(" simply click\n"
                                                                                         "on \"Continue\" to continue fitting, otherwise, click on \"Adjust\" and adjust the\n"
                                                                                         "equilibrium constant first and then click on \"Fit\" in the fitting control\n\n"),
                                          "Continue",
                                          "Adjust", 0, 0, 1))
         {
         case 0: // The user clicked the continue button or pressed Enter
            {
               Fit();
               break;
            }
         case 1: // The user clicked the Quit or pressed Escape
            {
               cancelFit();
               break;
            }
         }
      }
   }
   if ((which_model == 16) && !initialized_1)
   {
      initialized_1 = true;

      // scale incompetent monomer amplitudes and equil constant 3:1 association monomer:incompetent monomer and set to float
      for (unsigned int i=0; i<(*scanfit_vector).size(); i++)
      {
         if ((*scanfit_vector)[i].FitScanFlag)
         {
            (*scanfit_vector)[i].amplitude[0] = log (3.0 * exp((*scanfit_vector)[i].amplitude[0])/4.0);
            (*scanfit_vector)[i].amplitude[1] = log (exp((*scanfit_vector)[i].amplitude[0])/4.0);
            (*scanfit_vector)[i].amplitude_fit[1] = true;
         }
      }
      (*runfit).eq[0] = log(3.0 * exp((*runfit).eq[0])/4.0);
      (*runfit).eq_fit[0] = true;
      if (GUI)
      {
         (*model_control)->eqconst1_float(true);
         (*model_control)->fp_eqconst1->updateValue((*runfit).eq[0]);
         (*model_control)->update_include();
         lbl_status4->setText(tr("Stage 2: Now converging on incompetent monomer concentration..."));
         switch (QMessageBox::information(this, "UltraScan - " + modelString[which_model],
                                          tr("The program is about to set the amplitudes for the incompetent monomer \n"
                                             "to be floated for the second stage of the fitting process.\n\n"
                                             "Although a reasonable estimate will be automatically calculated, you can\n"
                                             "now change this estimate in the Model Control Window in case the fit does\n"
                                             "not converge by itself due to a poor initial estimate of the amplitudes.\n\n"
                                             "If you want to accept the automatic estimates for the amplitudes simply click\n"
                                             "on \"Continue\" to continue fitting, otherwise, click on \"Adjust\" and adjust the\n"
                                             "amplitudes first and then click on \"Fit\" in the fitting control\n\n"),
                                          "Continue",
                                          "Adjust", 0, 0, 1))
         {
         case 0: // The user clicked the continue button or pressed Enter
            {
               Fit();
               break;
            }
         case 1: // The user clicked the Quit or pressed Escape
            {
               cancelFit();
               break;
            }
         }
      }
   }
   if ((which_model == 17) && !initialized_1)
   {
      initialized_1 = true;

      // scale incompetent monomer amplitudes and equil constant 3:1 association monomer:incompetent n-mer and set to float
      for (unsigned int i=0; i<(*scanfit_vector).size(); i++)
      {
         if ((*scanfit_vector)[i].FitScanFlag)
         {
            (*scanfit_vector)[i].amplitude[0] = log (19.0 * exp((*scanfit_vector)[i].amplitude[0])/20.0);
            (*scanfit_vector)[i].amplitude[1] = log (exp((*scanfit_vector)[i].amplitude[0])/(20.0*(*runfit).stoich1));
            (*scanfit_vector)[i].amplitude_fit[1] = true;
         }
      }
      (*runfit).eq[0] = log(3.0 * exp((*runfit).eq[0])/4.0);
      (*model_control)->eqconst1_float(true);
      (*model_control)->fp_eqconst1->updateValue((*runfit).eq[0]);
      if (GUI)
      {
         (*model_control)->update_include();
         lbl_status4->setText(tr("Stage 2: Now converging on incompetent n-mer concentration..."));
         switch (QMessageBox::information(this, "UltraScan - " + modelString[which_model],
                                          tr("The program is about to set the amplitudes for the incompetent n-mer \n"
                                             "to be floated for the second stage of the fitting process.\n\n"
                                             "Although a reasonable estimate will be automatically calculated, you can\n"
                                             "now change this estimate in the Model Control Window in case the fit does\n"
                                             "not converge by itself due to a poor initial estimate of the amplitudes.\n\n"
                                             "If you want to accept the automatic estimates for the amplitudes simply click\n"
                                             "on \"Continue\" to continue fitting, otherwise, click on \"Adjust\" and adjust the\n"
                                             "amplitudes first and then click on \"Fit\" in the fitting control\n\n"),
                                          "Continue",
                                          "Adjust", 0, 0, 1))
         {
         case 0: // The user clicked the continue button or pressed Enter
            {
               Fit();
               break;
            }
         case 1: // The user clicked the Quit or pressed Escape
            {
               cancelFit();
               break;
            }
         }
      }
   }
   if ((which_model == 19) && !initialized_1)
   {
      initialized_1 = true;

      // scale incompetent monomer amplitudes and equil constant 3:1 association monomer:incompetent monomer and set to float
      for (unsigned int i=0; i<(*scanfit_vector).size(); i++)
      {
         if ((*scanfit_vector)[i].FitScanFlag)
         {
            (*scanfit_vector)[i].amplitude[0] = log (3.0 * exp((*scanfit_vector)[i].amplitude[0])/4.0);
            (*scanfit_vector)[i].amplitude[1] = log (exp((*scanfit_vector)[i].amplitude[0])/4.0);
            (*scanfit_vector)[i].amplitude_fit[1] = true;
         }
      }
      (*runfit).eq[0] = log(3.0 * exp((*runfit).eq[0])/4.0);
      if (GUI)
      {
         (*model_control)->update_include();
         lbl_status4->setText(tr("Stage 2: Now converging on incompetent monomer concentration..."));
         switch (QMessageBox::information(this, "UltraScan - " + modelString[which_model],
                                          tr("The program is about to set the amplitudes for the incompetent monomer \n"
                                             "to be floated for the second stage of the fitting process.\n\n"
                                             "Although a reasonable estimate will be automatically calculated, you can\n"
                                             "now change this estimate in the Model Control Window in case the fit does\n"
                                             "not converge by itself due to a poor initial estimate of the amplitudes.\n\n"
                                             "If you want to accept the automatic estimates for the amplitudes simply click\n"
                                             "on \"Continue\" to continue fitting, otherwise, click on \"Adjust\" and adjust the\n"
                                             "amplitudes first and then click on \"Fit\" in the fitting control\n\n"),
                                          "Continue",
                                          "Adjust", 0, 0, 1))
         {
         case 0: // The user clicked the continue button or pressed Enter
            {
               Fit();
               break;
            }
         case 1: // The user clicked the Quit or pressed Escape
            {
               cancelFit();
               break;
            }
         }
      }
   }
   else
   {
      calc_dlncr2();
      converged = true;
      if (GUI)
      {
         plot_residuals();
         write_report();
         qApp->processEvents();
         update_fitDialog();
         pb_cancel->setText(tr("Close"));
         pb_fit->setEnabled(true);
         pb_plottwo->setEnabled(true);
         pb_plotthree->setEnabled(true);
         pb_plotfour->setEnabled(true);
         pb_report->setEnabled(true);
         pb_saveFit->setEnabled(true);
         pb_residuals->setEnabled(true);
         pb_overlays->setEnabled(true);
         pb_fit->setEnabled(true);
         lbl_status2->setText(tr("The fitting process converged..."));
         pb_plottwo->setText(tr("Ln(C) vs R^2"));
         pb_plotthree->setText(tr("MW vs R^2"));
         pb_plotfour->setText(tr("MW vs C"));
         pgb_progress->setProgress(totalSteps);
      }
      if(which_model >= 4 && which_model <= 13 && GUI)
      {
         pb_plottwo->setEnabled(true);
         pb_plottwo->setText(tr("Distribution"));
      }
      emit currentStatus(tr("Converged"));
      write_report();
      emit hasConverged();
      //cout << "setting converged to true in endfit()\n";
   }
}

bool US_EqFitter::createHtmlDir()
{
   htmlDir = USglobal->config_list.html_dir + "/" + *projectName;
   QDir d(htmlDir);
   if (d.exists())
   {
      return true;
   }
   else
   {
      if (d.mkdir(htmlDir, true))
      {
         return true;
      }
      else
      {
         return false;
      }
   }
}

void US_EqFitter::calc_dlncr2()
{
   unsigned int i, j, dataset_counter = 0, point_counter = 0;
   float x1, x2, xm_sqr, omega_sqr, *vbar, *buoyancy, constant;
   vbar      = new float[(*runfit).components];
   buoyancy   = new float[(*runfit).components];
   mwpoints.clear();
   for (i=0; i<(*scanfit_vector).size(); i++)
   {
      if ((*scanfit_vector)[i].FitScanFlag)
      {
         xm_sqr = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index], 2);
         omega_sqr = pow(M_PI * (*scanfit_vector)[i].rpm / 30, 2);
         constant = omega_sqr/(2 * R * (K0 + (*scanfit_vector)[i].temperature));
         vbar[0] = adjust_vbar20((*runfit).vbar20[0], (*scanfit_vector)[i].temperature);
         buoyancy[0] = (1.0 - vbar[0] * (*scanfit_vector)[i].density);
         if (logpoints_per_dataset[dataset_counter] > 10 )
         {
            for (j=0; j<logpoints_per_dataset[dataset_counter]-1; j++)
            {
               if (j == 0 && logpoints_per_dataset[dataset_counter] == points_per_dataset[dataset_counter])
               {
                  x1 = 0.0;
               }
               else   // we need to shift data by the amount of excluded points (in lncr2 calculation) to right
               {
                  x1 = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j
                                                  + points_per_dataset[dataset_counter] - logpoints_per_dataset[dataset_counter]], 2) - xm_sqr;
               }
               x2 = pow((*scanfit_vector)[i].x[(*scanfit_vector)[i].start_index + j
                                               + points_per_dataset[dataset_counter] - logpoints_per_dataset[dataset_counter] + 1], 2) - xm_sqr;
               dlncr2[dataset_counter][j] = (lncr2[dataset_counter][j+1] - lncr2[dataset_counter][j])/(x2 - x1);
               dlncr2[dataset_counter][j] /= constant * buoyancy[0];
               mwpoints.push_back(dlncr2[dataset_counter][j]);
               point_counter++;
            }
         }
         dataset_counter++;
      }
   }
}

