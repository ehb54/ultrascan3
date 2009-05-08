#include "../include/us_finite_single.h"

US_Finite_single::US_Finite_single(QWidget *p, const char *name) : Data_Control_W(13, p, name)
{
   delete smoothing_lbl;
   delete smoothing_counter;
   smoothing_counter = NULL;
   delete range_lbl;
   delete range_counter;
   range_counter = NULL;
   delete position_lbl;
   delete position_counter;
   position_counter = NULL;
   delete pb_vbar;
   delete vbar_le;
   delete lbl1_excluded;
   delete lbl2_excluded;
   completed = false;
   mem_initialized = false;
   aborted = false;
   converged = false;
   fitting_widget = false;
   model_widget = false;
   monte_carlo_widget = false;
   step = 0;
   model_defined = false;
   suspend_flag = false;
   pb_second_plot->setText(tr("Monte Carlo"));
   pb_second_plot->setEnabled(false);
   constrained_fit = true;
   model = 0;
   xpos = border;
   components = 1;
   ypos = 382;
   pm = new US_Pixmap();
   
   pb_save->setEnabled(false);
   pb_create_model = new QPushButton(tr("Create new Model"), this);
   Q_CHECK_PTR(pb_create_model);
   pb_create_model->setAutoDefault(false);
   pb_create_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_create_model->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_create_model->setGeometry(xpos, ypos, buttonw, buttonh);
   pb_create_model->setEnabled(false);
   connect(pb_create_model, SIGNAL(clicked()), SLOT(create_model()));

   xpos += buttonw + spacing;

   pb_fit_control = new QPushButton(tr("Fitting Control"), this);
   Q_CHECK_PTR(pb_fit_control);
   pb_fit_control->setAutoDefault(false);
   pb_fit_control->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_fit_control->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_fit_control->setGeometry(xpos, ypos, buttonw, buttonh);
   pb_fit_control->setEnabled(false);
   connect(pb_fit_control, SIGNAL(clicked()), SLOT(select_fit_control()));
   
   xpos = border + buttonw + spacing;
   ypos += buttonh + spacing;

   pb_model_control = new QPushButton(tr("Model Control"), this);
   Q_CHECK_PTR(pb_model_control);
   pb_model_control->setAutoDefault(false);
   pb_model_control->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_model_control->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_model_control->setGeometry(xpos, ypos, buttonw, buttonh);
   pb_model_control->setEnabled(false);
   connect(pb_model_control, SIGNAL(clicked()), SLOT(select_model_control()));

   xpos = border;
   ypos += buttonh + spacing;
   
   pb_load_fit = new QPushButton(tr("Load Fit"), this);
   Q_CHECK_PTR(pb_load_fit);
   pb_load_fit->setAutoDefault(false);
   pb_load_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_load_fit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_load_fit->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_load_fit, SIGNAL(clicked()), SLOT(load_fit()));

   xpos += buttonw + spacing;

   pb_save_fit = new QPushButton("", this);
   Q_CHECK_PTR(pb_save_fit);
   pb_save_fit->setAutoDefault(false);
   pb_save_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_save_fit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_save_fit->setGeometry(xpos, ypos, buttonw, buttonh);
   pb_save_fit->setEnabled(false);
   
   xpos = border;
   ypos += buttonh + spacing;
   //   ypos = 466;

   residuals_lbl = new QLabel(tr(" Residuals:"), this);
   Q_CHECK_PTR(residuals_lbl);
   residuals_lbl->setAlignment(AlignLeft|AlignVCenter);
   residuals_lbl->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   residuals_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   residuals_lbl->setGeometry(xpos, ypos, buttonw, buttonh);

   xpos += buttonw + spacing;

   residuals_counter= new QwtCounter(this);
   Q_CHECK_PTR(residuals_counter);
   residuals_counter->setRange(1, 1, 1);
   residuals_counter->setNumButtons(2);
   residuals_counter->setValue(1);
   residuals_counter->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   residuals_counter->setGeometry(xpos, ypos, buttonw, buttonh);
   //   connect(residuals_counter, SIGNAL(buttonReleased(double)), SLOT(update_residuals(double)));
   connect(residuals_counter, SIGNAL(valueChanged(double)), SLOT(update_residuals(double)));

   xpos = border + buttonw + spacing;
   ypos += buttonh + spacing;

   bd_range = 100;
   range_counter= new QwtCounter(this);
   Q_CHECK_PTR(range_counter);
   range_counter->setRange(10, 100, 0.1);
   range_counter->setValue(bd_range);
   range_counter->setNumButtons(3);
   range_counter->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   range_counter->setGeometry(xpos, ypos, buttonw, buttonh);
   //   connect(range_counter, SIGNAL(buttonReleased(double)), SLOT(update_boundary_range(double)));
   connect(range_counter, SIGNAL(valueChanged(double)), SLOT(update_boundary_range(double)));

   xpos = border + buttonw + spacing;
   ypos += buttonh + spacing;

   bd_position = 0;
   position_counter= new QwtCounter(this);
   Q_CHECK_PTR(position_counter);
   position_counter->setRange(0, 90, 0.1);
   position_counter->setNumButtons(3);
   position_counter->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   position_counter->setValue(bd_position);
   position_counter->setGeometry(xpos, ypos, buttonw, buttonh);
   //   connect(position_counter, SIGNAL(buttonReleased(double)), SLOT(update_boundary_position(double)));
   connect(position_counter, SIGNAL(valueChanged(double)), SLOT(update_boundary_position(double)));

   cp_list.clear();
   rotor_list.clear();
   if (!readCenterpieceInfo(&cp_list))
   {
      QString msg_str = "There was a problem opening the\n"
         "centerpiece database file:\n\n"
         + USglobal->config_list.system_dir + "/etc/centerpiece.dat\n\n"
         "Please install the centerpiece database file\n"
         "before proceeding.";
      QMessageBox::critical(0, "UltraScan Fatal Error:", msg_str, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
      exit(-1);
   }
   if (!readRotorInfo(&rotor_list))
   {
      QString msg_str = "There was a problem opening the\n"
         "rotor database file:\n\n"
         + USglobal->config_list.system_dir + "/etc/rotor.dat\n\n"
         "Please install the rotor database file\n"
         "before proceeding.";
      QMessageBox::critical(0, "UltraScan Fatal Error:", msg_str, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
      exit(-1);
   }
}

US_Finite_single::~US_Finite_single()
{
   cleanup();
}

void US_Finite_single::closeEvent(QCloseEvent *e)
{
   e->accept();
   global_Xpos -= 30;
   global_Ypos -= 30;
   if (model_widget)
   {
      model_control->close();
   }
   if (fitting_widget)
   {
      velocFitter->close();
   }
   if (monte_carlo_widget)
   {
      monte_carlo_window->close();
   }
}

void US_Finite_single::updateButtons()
{
   pb_save_fit->setEnabled(false);
   pb_save->setEnabled(false);
   pb_fit_control->setEnabled(false);
   pb_model_control->setEnabled(false);
   pb_create_model->setEnabled(true);
   pb_second_plot->setEnabled(false);   
   pb_view->setEnabled(false);
}

void US_Finite_single::reset()
{
   if (step == 0)
   {
      QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n      Click on \"Load Data\"\n"));
      return;
   }
   reset_flag = true;
   update_boundary_range(100.0);
   bd_range=100;
   range_counter->setValue(bd_range);
}

// This virtual function doubles as the Monte Carlo calling routine:
void US_Finite_single::second_plot()
{
   QString str;
   mc.parameter.clear();
   mc.fitName = fitName;
   mc.experiment = 0; //velocity experiment
   mc.run_id = run_inf.run_id   + str.sprintf("-%d%d", selected_cell + 1, selected_lambda + 1);
   mc.random_seed = 0;
   mc.iterations = 10000;
   mc.filename = USglobal->config_list.result_dir + "/" + mc.run_id + ".mc";
   mc.append = true;
   mc.status = false;
   mc.rule = 0;   // use residual as standard deviation
   mc.percent_bootstrap = 100;
   mc.data_type = 0;
   mc.percent_gaussian = 50;
   mc.parameters = 1;
   mc.parameterNoise = 8.0;   // add noise to the parameter value with a STDD of 10 percent of absolute parameter value
   mc.varianceThreshold = (float) 1.0e-4;
   mc.addNoise = 0; // use parameters from previous fit as initial guess
   str.sprintf(tr("%d: Variance"), mc.parameters);
   mc.parameter.push_back(str);
   if (runData->meniscus_fit)
   {
      mc.parameters++;
      str.sprintf(tr("%d: Meniscus"), mc.parameters);
      mc.parameter.push_back(str);
   }
   if (runData->baseline_fit)
   {
      mc.parameters++;
      str.sprintf(tr("%d: Baseline"), mc.parameters);
      mc.parameter.push_back(str);
   }
   if (runData->slope_fit)
   {
      mc.parameters++;
      str.sprintf(tr("%d: Linear Slope"), mc.parameters);
      mc.parameter.push_back(str);
   }
   if (runData->stray_fit)
   {
      mc.parameters++;
      str.sprintf(tr("%d: Stray Light"), mc.parameters);
      mc.parameter.push_back(str);
   }
   for (unsigned int m=0; m<components; m++)
   {
      if (component_vector[m].conc_fit != 0)
      {
         mc.parameters++;
         str.sprintf(tr("%d: Partial Concentration (%d)"), mc.parameters, m+1);
         mc.parameter.push_back(str);
      }
      if (component_vector[m].sed_fit != 0)
      {
         mc.parameters++;
         str.sprintf(tr("%d: Sedimentation Coefficient (%d)"), mc.parameters, m+1);
         mc.parameter.push_back(str);
      }
      if (component_vector[m].diff_fit != 0)
      {
         mc.parameters++;
         str.sprintf("%d: Diffusion Coefficient (%d)", mc.parameters, m+1);
         mc.parameter.push_back(str);
      }
      if (component_vector[m].sigma_fit != 0)
      {
         mc.parameters++;
         str.sprintf(tr("%d: Sigma (%d)"), mc.parameters, m+1);
         mc.parameter.push_back(str);
      }
      if (component_vector[m].delta_fit != 0)
      {
         mc.parameters++;
         str.sprintf(tr("%d: Delta (%d)"), mc.parameters, m+1);
         mc.parameter.push_back(str);
      }
      if (component_vector[m].mw_fit != 0)
      {
         mc.parameters++;
         str.sprintf(tr("%d: Molecular Weight (%d)"), mc.parameters, m+1);
         mc.parameter.push_back(str);
      }
      if (component_vector[m].vbar_fit != 0)
      {
         mc.parameters++;
         str.sprintf(tr("%d: Partial Spec. Vol. (%d)"), mc.parameters, m+1);
         mc.parameter.push_back(str);
      }
   }
   monte_carlo_window = new US_MonteCarlo(&mc, &monte_carlo_widget);
   connect(monte_carlo_window, SIGNAL(iterate()), SLOT(monte_carlo()));
   monte_carlo_window->show();
}

void US_Finite_single::monte_carlo()
{
   QString str;
   int now = 0;
   unsigned int count = 0, current_point, i, j, k=0;
   QTime t;
   t = QTime::currentTime();
   now = t.msec() + t.second() * 1000 + t.minute() * 60000 + t.hour() * 3600000;
   if (mc.random_seed == 0)
   {
      mc.random_seed =    now;
   }
#ifndef WIN32
   mc.random_seed -= (int) getpid();
#endif
   str.sprintf(" %d", mc.random_seed);
   monte_carlo_window->le_seed->setText(str);
   residuals_v.clear();
   float **temp_absorbance, **fit_absorbance, sigma = 0, *std_dev;
   temp_absorbance = new float *[run_inf.scans[selected_cell][selected_lambda]];
   fit_absorbance = new float *[run_inf.scans[selected_cell][selected_lambda]];

   for (unsigned int i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
   {
      temp_absorbance[i] = new float [run_inf.points[selected_cell][selected_lambda][0]];
      fit_absorbance[i] = new float [run_inf.points[selected_cell][selected_lambda][0]];
   }
   for (unsigned int i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
   {
      for (unsigned int j=0; j<run_inf.points[selected_cell][selected_lambda][0]; j++)
      {
         fit_absorbance[i][j] = F[i][j];
         temp_absorbance[i][j] = absorbance[i][j];
         residuals_v.push_back(fit_absorbance[i][j] - temp_absorbance[i][j]);
      }
   }
   srand(mc.random_seed);   //randomize the box muller function
   if (!fitting_widget)
   {
      select_fit_control();
   }
   
   unsigned int iteration = 1;
   QFile mc_f(mc.filename);
   if (!mc.append)
   {
      mc_f.remove();
   }
   QTextStream ts(&mc_f);
   ts.width(14);
   ts.flags(0x1000);
   if(mc.rule > 2) // then we need to initialize a temporary array with the smoothed SD's
   {
      j = residuals_v.size();
      std_dev = new float [j];
      switch (mc.rule)
      {
      case 3:
         {
            k = 5;
            break;
         }
      case 4:
         {
            k = 10;
            break;
         }
      case 5:
         {
            k = 15;
            break;
         }
      case 6:
         {
            k = 20;
            break;
         }
      case 7:
         {
            k = 25;
            break;
         }
      case 8:
         {
            k = 30;
            break;
         }
      }
      for (i=0; i<j; i++)
      {
         std_dev[i] = fabs(residuals_v[i]);   // only send in the absolute values of the std. deviations
      }
      gaussian_smoothing(&std_dev, k, j);
      for (i=0; i<j; i++)
      {
         if (residuals_v[i] < 0)
         {
            std_dev[i] *= -1.0;   // correct the sign of the residuals after averaging
         }
      }
   }
   while (mc.status && iteration < mc.iterations)
   {
      /* in the finite element analysis, always use the last parameters for initialization
         this will be changed once the fitting routine "dud" has been replaced.
         // re-initialize the parameters with the best-fit result and overwrite the parameters from the
         // last Monte Carlo iteration:
         switch (mc.addNoise)
         {
         case 0:
         {
         // do nothing, use parameters from last fit
         break;
         }
         case 1:
         {
         eqFitter->parameter_addRandomNoise(temp_parameters, mc.parameterNoise);
         break;
         }
         case 2:
         {
         eqFitter->parameter_mapBackward(temp_parameters);
         break;
         }
         }
      */
      iteration ++;
      str.sprintf(" %d", iteration);
      monte_carlo_window->lbl_current_iteration2->setText(str);

      switch (mc.data_type)
      {
      case 0:   // use normal Gaussian random residuals
         {
            count = 0;
            for (unsigned int j=0; j<run_inf.scans[selected_cell][selected_lambda]; j++)
            {
               for (unsigned int k=0; k<run_inf.points[selected_cell][selected_lambda][0]; k++)
               {
                  switch(mc.rule)
                  {
                  case 0:
                     {
                        sigma = residuals_v[count];
                        break;
                     }
                  case 1:
                     {
                        sigma = max(standard_deviation, residuals_v[count]);
                        break;
                     }
                  case 2:
                     {
                        sigma = standard_deviation;
                        break;
                     }
                  }
                  if (mc.rule > 2 && mc.rule < 9)
                  {
                     sigma = std_dev[count];
                  }
                  absorbance[j][k] = fit_absorbance[j][k] + box_muller(0, sigma);
                  count ++;
               }
            }
            break;
         }
      case 1:   // use original residuals reordered in bootstrapped fashion
         {
            count = 0;
            for (unsigned int j=0; j<run_inf.scans[selected_cell][selected_lambda]; j++)
            {
               for (unsigned int k=0; k<run_inf.points[selected_cell][selected_lambda][0]; k++)
               {

                  // This algorithm randomizes the order of the residuals of the original fit and applies the absolute
                  // magnitude of the residuals to different points than where they were originally.

                  // First, find a random point between the first point of the first scan and the last point of the last scan

                  current_point = (unsigned int) (rand()/(RAND_MAX + 1.0) * residuals_v.size());
                  
                  // use the magnitude of the residual of the original fit and add it to the fitted solution:

                  absorbance[j][k] = fit_absorbance[j][k] + residuals_v[current_point];

                  // if the point doesn't fall within the percentage of the bootstrapped points, use the original residual instead:

                  if (rand()/(RAND_MAX + 1.0) > (mc.percent_bootstrap/100.0))
                  {
                     absorbance[j][k] = temp_absorbance[j][k];
                  }
                  count ++;
               }
            }
            break;
         }
      case 2:   // use a mixture of random Gaussian residuals and bootstrapped residuals:
         {
            count = 0;
            for (unsigned int j=0; j<run_inf.scans[selected_cell][selected_lambda]; j++)
            {
               for (unsigned int k=0; k<run_inf.points[selected_cell][selected_lambda][0]; k++)
               {
                  if (rand()/(RAND_MAX + 1.0) > (mc.percent_gaussian/100.0))
                  {
                     switch(mc.rule)
                     {
                     case 0:
                        {
                           sigma = residuals_v[count];
                           break;
                        }
                     case 1:
                        {
                           sigma = max(standard_deviation, residuals_v[count]);
                           break;
                        }
                     case 2:
                        {
                           sigma = standard_deviation;
                           break;
                        }
                     }
                     if (mc.rule > 2 && mc.rule < 9)
                     {
                        sigma = std_dev[count];
                     }
                     absorbance[j][k] = fit_absorbance[j][k] + box_muller(0, sigma);
                  }
                  else
                  {
                     current_point = (unsigned int) (rand()/(RAND_MAX + 1.0) * residuals_v.size());
                     absorbance[j][k] = fit_absorbance[j][k] + residuals_v[current_point];
                     if (rand()/(RAND_MAX + 1.0) > (mc.percent_bootstrap/100.0))
                     {
                        absorbance[j][k] = temp_absorbance[j][k];
                     }
                  }
                  count ++;
               }
            }
            break;
         }
      }
      velocFitter->nlsMethod = 0;
      velocFitter->startFit();
      if(velocFitter->tape_construct)
      {
         connect(velocFitter, SIGNAL(fitStarted()), SLOT(change_tape_status()));
      }
      mc_f.open(IO_WriteOnly | IO_Append);
      ts << tr("Iteration ") << iteration << " (" << mc.random_seed << "): " << variance << " ";
      if (runData->meniscus_fit)
      {
         ts << runData->meniscus << " ";
      }
      if (runData->baseline_fit)
      {
         ts << runData->baseline << " ";
      }
      if (runData->slope_fit)
      {
         ts << runData->slope << " ";
      }
      if (runData->stray_fit)
      {
         ts << runData->stray << " ";
      }
      for (unsigned int m=0; m<components; m++)
      {
         if (component_vector[m].conc_fit != 0)
         {
            ts << component_vector[m].conc << " ";
         }
         if (component_vector[m].sed_fit != 0)
         {
            ts << component_vector[m].s20w << " ";
         }
         if (component_vector[m].diff_fit != 0)
         {
            ts << component_vector[m].d20w << " ";
         }
         if (component_vector[m].sigma_fit != 0)
         {
            ts << component_vector[m].sigma << " ";
         }
         if (component_vector[m].delta_fit != 0)
         {
            ts << component_vector[m].delta << " ";
         }
         if (component_vector[m].mw_fit != 0)
         {
            ts << component_vector[m].mw << " ";
         }
         if (component_vector[m].vbar_fit != 0)
         {
            ts << component_vector[m].vbar << " ";
         }
      }
      ts << endl;
      mc_f.flush();
      mc_f.close();
      if(monte_carlo_widget)
      {
         monte_carlo_window->update_iteration();
      }
   }
   // if we start again to fit some more, first generate a new tape, then the tape status needs to be changed
   // and the tape_construct needs to be true.
   disconnect(velocFitter, SIGNAL(fitStarted()), this, SLOT(change_tape_status()));
   monte_carlo_window->pb_start->setEnabled(true);
   monte_carlo_window->pb_stop->setEnabled(false);
   mc.random_seed = 0;
   str.sprintf(" %d", mc.random_seed);
   monte_carlo_window->le_seed->setText(str);
   for (unsigned int k=0; k<run_inf.scans[selected_cell][selected_lambda]; k++)
   {
      delete [] temp_absorbance[k];
   }
   delete [] temp_absorbance;
}

void US_Finite_single::select_model_control()
{
   if (model_widget)
   {
      if (model_control->isVisible())
      {
         model_control->raise();
      }
      else
      {
         model_control->show();
      }
      return;
   }
   if ((!model_defined) && (step == 0))
   {
      QMessageBox::message(tr("Attention:\n"),tr("First, you need to load some data\n"
                                                 "and then define a model!\n\n"
                                                 "Click on \"Load Data\"\n"
                                                 "and then on \"Create Model\""));
      return;
   }
   if (!model_defined)
   {
      QMessageBox::message(tr("Attention:\n"),tr("Please create a Model first!\n\n"
                                                 "Click on \"Create Model\""));
      return;
   }
   switch (model)
   {
   case 0:
      {
         model_id.sprintf(tr("Non-Interacting"));
         break;
      }
   case 1:
      {
         model_id.sprintf(tr("Monomer-Dimer"));
         break;
      }
   case 2:
      {
         model_id.sprintf(tr("Isomerization"));
         break;
      }
   }
   vbar_model.clear();
   for (i=0; i<components; i++) // in case there is only one peptide defined, all components should have the same vbar
   {
      vbar_model.push_back(Vbar20[selected_cell][selected_channel][0]);
   }
   for (i=0; i<components; i++)
   {
      if ( i>0 && fabs(Vbar20[selected_cell][selected_channel][i] - 0.72) > 1e-6)
      {
         vbar_model[i] = Vbar20[selected_cell][selected_channel][i];
      }
   }
   model_control = new US_VelocModelControl(&model_widget, components, model, &component_vector, &runData, 
                                            &viscosity_tb, &density_tb, run_inf, selected_cell, selected_lambda, selected_channel, vbar_model);
   model_control->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   model_control->show();
}

// this slot is connected to the fitStarted() signal from velocfitter,
// which is issued right after initializing. For each new Monte Carlo iteration, the 
// fitting process is re-initialize, old memory deleted, and the tape construct set to true.
// However, since we are fitting the same solution, we don't need to regenate the tape,
// so as soon as the memory is deleted and about to be re-initialized, we set the 
// tape_construct variable to false (which has been set to "true" in the memory deletion function

void US_Finite_single::change_tape_status()
{
   velocFitter->tape_construct = false;
}

void US_Finite_single::select_fit_control()
{
   float bottom = calc_bottom(rotor_list, cp_list, run_inf.rotor, run_inf.centerpiece[selected_cell], 0, run_inf.rpm[first_cell][0][0]);
   if (fitting_widget)
   {
      if (velocFitter->isVisible())
      {
         velocFitter->raise();
      }
      else
      {
         velocFitter->show();
      }
      return;
   }
   else
   {
      fit_init();
      velocFitter = new US_FeFitter_Single(&fitting_widget, 
                                           true /* show GUI */,
                                           components,
                                           model,
                                           &component_vector,
                                           runData,
                                           &run_inf,
                                           points,
                                           selected_cell,
                                           selected_lambda,
                                           bd_range,
                                           bd_position,
                                           density,
                                           density_tb,
                                           viscosity,
                                           viscosity_tb,
                                           bottom,
                                           &variance,
                                           absorbance,
                                           F,
                                           residuals,
                                           radius);

      connect(velocFitter, SIGNAL(parametersUpdated()), SLOT(updateModelWindow()));
      connect(velocFitter, SIGNAL(fitLoaded()), SLOT(updateMonteCarlo()));
      connect(velocFitter, SIGNAL(fitSuspended()), SLOT(suspend_fit()));
      connect(velocFitter, SIGNAL(hasConverged()), SLOT(fit_converged()));
      connect(velocFitter, SIGNAL(fitStarted()), SLOT(fit_started()));
      connect(velocFitter, SIGNAL(fitAborted()), SLOT(cancel_fit()));

      connect(velocFitter, SIGNAL(meniscus_red()), model_control, SLOT(meniscus_red()));
      connect(velocFitter, SIGNAL(meniscus_green()), model_control, SLOT(meniscus_green()));
      connect(velocFitter, SIGNAL(meniscus_dk_green()), model_control, SLOT(meniscus_dk_green()));

      connect(velocFitter, SIGNAL(baseline_red()), model_control, SLOT(baseline_red()));
      connect(velocFitter, SIGNAL(baseline_green()), model_control, SLOT(baseline_green()));
      connect(velocFitter, SIGNAL(baseline_dk_green()), model_control, SLOT(baseline_dk_green()));

      connect(velocFitter, SIGNAL(slope_red()), model_control, SLOT(slope_red()));
      connect(velocFitter, SIGNAL(slope_green()), model_control, SLOT(slope_green()));
      connect(velocFitter, SIGNAL(slope_dk_green()), model_control, SLOT(slope_dk_green()));

      connect(velocFitter, SIGNAL(stray_red()), model_control, SLOT(stray_red()));
      connect(velocFitter, SIGNAL(stray_green()), model_control, SLOT(stray_green()));
      connect(velocFitter, SIGNAL(stray_dk_green()), model_control, SLOT(stray_dk_green()));

      connect(velocFitter, SIGNAL(sed_red()), model_control, SLOT(sed_red()));
      connect(velocFitter, SIGNAL(sed_green()), model_control, SLOT(sed_green()));
      connect(velocFitter, SIGNAL(sed_dk_green()), model_control, SLOT(sed_dk_green()));
      
      connect(velocFitter, SIGNAL(diff_red()), model_control, SLOT(diff_red()));
      connect(velocFitter, SIGNAL(diff_green()), model_control, SLOT(diff_green()));
      connect(velocFitter, SIGNAL(diff_dk_green()), model_control, SLOT(diff_dk_green()));

      connect(velocFitter, SIGNAL(conc_red()), model_control, SLOT(conc_red()));
      connect(velocFitter, SIGNAL(conc_green()), model_control, SLOT(conc_green()));
      connect(velocFitter, SIGNAL(conc_dk_green()), model_control, SLOT(conc_dk_green()));

      connect(velocFitter, SIGNAL(sigma_red()), model_control, SLOT(sigma_red()));
      connect(velocFitter, SIGNAL(sigma_green()), model_control, SLOT(sigma_green()));
      connect(velocFitter, SIGNAL(sigma_dk_green()), model_control, SLOT(sigma_dk_green()));

      connect(velocFitter, SIGNAL(delta_red()), model_control, SLOT(delta_red()));
      connect(velocFitter, SIGNAL(delta_green()), model_control, SLOT(delta_green()));
      connect(velocFitter, SIGNAL(delta_dk_green()), model_control, SLOT(delta_dk_green()));
      /*
        connect(eqFitter, SIGNAL(fitResumed()), SLOT(fit_resumed()));
        connect(eqFitter, SIGNAL(fittingWidgetClosed()), SLOT(updateViewer()));
        connect(eqFitter, SIGNAL(dataSaved(const QString &, const int)), SLOT(writeScanList(const QString &, const int)));
      */
      velocFitter->show();
   }
}

void US_Finite_single::fit_started()
{
   pb_save->setEnabled(false);
   pb_second_plot->setEnabled(false);
   pb_load_fit->setEnabled(false);
   pb_reset->setEnabled(false);
   pb_create_model->setEnabled(false);   
}

void US_Finite_single::updateModelWindow()
{
   model_control->update_labels();
}

void US_Finite_single::updateMonteCarlo()
{
   pb_second_plot->setEnabled(true);
}

void US_Finite_single::suspend_fit()
{
   suspend_flag = true;
   model_control->cb_meniscus_float->setEnabled(false);
   model_control->cb_meniscus_fix->setEnabled(false);
   model_control->cb_baseline_float->setEnabled(false);
   model_control->cb_baseline_fix->setEnabled(false);
   model_control->cb_slope_float->setEnabled(false);
   model_control->cb_slope_fix->setEnabled(false);
   model_control->cb_stray_float->setEnabled(false);
   model_control->cb_stray_fix->setEnabled(false);
   model_control->cb_sed_float->setEnabled(false);
   model_control->cb_sed_fix->setEnabled(false);
   model_control->cb_diff_float->setEnabled(false);
   model_control->cb_diff_fix->setEnabled(false);
   model_control->cb_conc_float->setEnabled(false);
   model_control->cb_conc_fix->setEnabled(false);
   model_control->cb_sigma_float->setEnabled(false);
   model_control->cb_sigma_fix->setEnabled(false);
   model_control->cb_delta_float->setEnabled(false);
   model_control->cb_delta_fix->setEnabled(false);
}

void US_Finite_single::resume_fit()
{
   suspend_flag = false;
   model_control->cb_meniscus_float->setEnabled(true);
   model_control->cb_meniscus_fix->setEnabled(true);
   model_control->cb_baseline_float->setEnabled(true);
   model_control->cb_baseline_fix->setEnabled(true);
   model_control->cb_slope_float->setEnabled(true);
   model_control->cb_slope_fix->setEnabled(true);
   model_control->cb_stray_float->setEnabled(true);
   model_control->cb_stray_fix->setEnabled(true);
   model_control->cb_sed_float->setEnabled(true);
   model_control->cb_sed_fix->setEnabled(true);
   model_control->cb_diff_float->setEnabled(true);
   model_control->cb_diff_fix->setEnabled(true);
   model_control->cb_conc_float->setEnabled(true);
   model_control->cb_conc_fix->setEnabled(true);
   model_control->cb_sigma_float->setEnabled(true);
   model_control->cb_sigma_fix->setEnabled(true);
   model_control->cb_delta_float->setEnabled(true);
   model_control->cb_delta_fix->setEnabled(true);
}

void US_Finite_single::fit_converged()
{
   converged = true;
   pb_view->setEnabled(true);
   pb_save->setEnabled(true);
   pb_second_plot->setEnabled(true);
   pb_load_fit->setEnabled(true);
   pb_reset->setEnabled(true);
   pb_create_model->setEnabled(true);
   update_residuals(1);   
}

void US_Finite_single::cancel_fit()
{
   pb_load_fit->setEnabled(true);
   pb_reset->setEnabled(true);
   pb_create_model->setEnabled(true);   
   if (aborted || converged)
   {
      velocFitter->cancel();
   }
   else
   {   
      suspend_flag = true;   
      velocFitter->pgb_progress->reset();
      velocFitter->pb_cancel->setText(tr("Close"));
      velocFitter->lbl_iteration2->setText("0");
      velocFitter->lbl_variance2->setText("0");
      //      velocFitter->lbl_variance3->setText("0");
      velocFitter->lbl_stddev2->setText("0");
      //      velocFitter->lbl_difference2->setText("0");
      velocFitter->lbl_evaluations2->setText("0");
      velocFitter->lbl_status2->setText(tr("Aborted"));
      velocFitter->lbl_status3->setText("");
      velocFitter->lbl_status4->setText("");
      aborted = true;
   }
}

void US_Finite_single::create_model()
{
   if (step == 0)
   {
      QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n      Click on \"Load Data\"\n"));
      return;
   }
   if (model_widget && fitting_widget)
   {
      QMessageBox mb(tr("UltraScan - Please Note:"), 
                     tr("A model and fit is currently active\n"
                        "Do you want to abandon the current fit\n"
                        "and start a new model and fit?"),
                     QMessageBox::Warning,
                     QMessageBox::Yes | QMessageBox::Default,
                     QMessageBox::No | QMessageBox::Escape, 0);
      mb.setButtonText(QMessageBox::Yes, tr("Continue"));
      mb.setButtonText(QMessageBox::No, tr("Cancel"));
      switch(mb.exec())
      {
      case QMessageBox::Yes:
         {
            model_control->close();
            delete model_control;
            velocFitter->close();
            delete velocFitter;
            break;
         }
      default:
         {
            return;
         }
      }
   }
   if (model_widget)
   {
      QMessageBox mb(tr("UltraScan - Please Note:"), 
                     tr("A model is currently active\n"
                        "Do you want to abandon the\n"
                        "current model and start a new model?"),
                     QMessageBox::Warning,
                     QMessageBox::Yes | QMessageBox::Default,
                     QMessageBox::No | QMessageBox::Escape, 0);
      mb.setButtonText(QMessageBox::Yes, tr("Continue"));
      mb.setButtonText(QMessageBox::No, tr("Cancel"));
      switch(mb.exec())
      {
      case QMessageBox::Yes:
         {
            model_control->close();
            delete model_control;
            break;
         }
      default:
         {
            return;
         }
      }
   }
   if (fitting_widget)
   {
      QMessageBox mb(tr("UltraScan - Please Note:"), 
                     tr("A fit is currently active\n"
                        "Do you want to abandon the current fit\n"
                        "and start a new model and fit?"),
                     QMessageBox::Warning,
                     QMessageBox::Yes | QMessageBox::Default,
                     QMessageBox::No | QMessageBox::Escape, 0);
      mb.setButtonText(QMessageBox::Yes, tr("Continue"));
      mb.setButtonText(QMessageBox::No, tr("Cancel"));
      switch(mb.exec())
      {
      case QMessageBox::Yes:
         {
            velocFitter->close();
            delete velocFitter;
            break;
         }
      default:
         {
            return;
         }
      }
   }
   if (run_inf.wavelength[selected_cell][selected_lambda] == 999)
   {
      QMessageBox mb(tr("UltraScan - Attention:"),tr("This is simulated data!\n\n"
                                                     "It was generated with the finite element\n"
                                                     "method in the   in the first place. You really\n"
                                                     "could just check the corresponding model\n"
                                                     "for the run parameters.\n\n"
                                                     "Do you still want to fit this data?"),
                     QMessageBox::Information,
                     QMessageBox::Yes,
                     QMessageBox::No | QMessageBox::Default, 0);
      mb.setButtonText(QMessageBox::Yes, "Yes");
      mb.setButtonText(QMessageBox::No, "Cancel");
      if (mb.exec() == QMessageBox::No)
      {
         return;
      }
   }
   runData = new fefit_run;
   US_SelectModel *SelectModel;
   SelectModel = new US_SelectModel(&model, false);
   if (SelectModel->exec())
   {
      model_id.sprintf(tr(modelString[model]));
      switch (model)
      {
      case 0:
         {
            components = 1;
            start_fit();
            break;
         }
      case 1:
         {
            components = 2;
            start_fit();
            break;
         }
      case 2:
         {
            components = 3;
            
            start_fit();
            break;
         }
      case 3:
         {
            US_SelectModel3 *SelectModel3;
            SelectModel3 = new US_SelectModel3(&components);
            if (!SelectModel3->exec())
            {
               model = 0;
               break;
            }
            start_fit();
            break;
         }
      default:
         {
            QMessageBox::message(tr("UltraScan - Attention:"),
                                 tr("This model is not yet available in this module.\n"
                                    "Please only select models 1-4 from the above choices."));
            model = 0;
            return;
         }
      }
   }
   else // use single component ideal system as default
   {
      model = 0;
   }
   pb_fit_control->setEnabled(true);
   pb_model_control->setEnabled(true);
}

void US_Finite_single::start_fit()
{
   unsigned int i;
   struct fefit_component temp_component;

   temp_component.conc = 0.0;
   temp_component.sed = 0.0;
   temp_component.s20w = 0.0;
   temp_component.diff = 0.0;
   temp_component.d20w = 0.0;
   temp_component.sigma = 0.0;
   temp_component.delta = 0.0;
   temp_component.mw = 0.0;
   temp_component.vbar = 0.0;
   temp_component.conc_range = 0.0;
   temp_component.sed_range = 0.0;
   temp_component.diff_range = 0.0;
   temp_component.sigma_range = 0.0;
   temp_component.delta_range = 0.0;
   temp_component.conc_fit = 0;
   temp_component.sed_fit = 0;
   temp_component.diff_fit = 0;
   temp_component.sigma_fit = 0;
   temp_component.delta_fit = 0;
   temp_component.mw_fit = 0;
   temp_component.vbar_fit = 0;
   vbar_model.clear();
   for (i=0; i<components; i++) // in case there is only one peptide defined, all components should have the same vbar
   {
      vbar_model.push_back(Vbar20[selected_cell][selected_channel][0]);
   }
   for (i=0; i<components; i++)
   {
      if ( i>0 && fabs(Vbar20[selected_cell][selected_channel][i] - 0.72) > 1e-6)
      {
         vbar_model[i] = Vbar20[selected_cell][selected_channel][i];
      }
      component_vector.push_back(temp_component);
   }
   if (run_type == 1) // we already subtracted the baseline from the scans during loading for absorbance velocity runs
   {
      run_inf.baseline[selected_cell][selected_lambda] = 0.0; 
   }
   model_control = new US_VelocModelControl(&model_widget, components, model, &component_vector, &runData, 
                                            &viscosity_tb, &density_tb, run_inf, selected_cell, selected_lambda, selected_channel, vbar_model);
   model_control->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   model_control->show();
   model_defined = true;
}

void US_Finite_single::update_residuals(double val)
{
   if (step == 0)
   {
      QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n      Click on \"Load Data\"\n"));
      return;
   }
   if (!converged)
   {
      QMessageBox::message(tr("Attention:\n"),tr("There are no residuals to display...\n\n"
                                                 "Please fit your data first until it is\n"
                                                 "converged before trying to display results.\n"));
      return;
   }
   unsigned int first_resid = (unsigned int) val;
   unsigned int i, j, num_scans, linecount;
   QString s1, s2, s3;
   analysis_plot->clear();
   if ((unsigned int) run_inf.scans[selected_cell][selected_lambda] - first_resid == 0)
   {
      num_scans = 1;
      s1.sprintf(tr(": Cell %d - Residuals for scan %d"), selected_cell+1, first_resid);
   }
   else if (run_inf.scans[selected_cell][selected_lambda] - first_resid < 5)
   {
      num_scans = run_inf.scans[selected_cell][selected_lambda] - first_resid + 1;
      s1.sprintf(tr(": Cell %d - Residuals from scans %d - %d"), selected_cell+1, first_resid, run_inf.scans[selected_cell][selected_lambda]);
   }
   else
   {
      num_scans = 5;
      s1.sprintf(tr(": Cell %d - Residuals from scans %d - %d"), selected_cell+1, first_resid, first_resid+4);
   }
   s2 = tr("Run ") + run_inf.run_id + s1; 
   s3.sprintf(tr("Delta OD (%d nm)"), run_inf.wavelength[selected_cell][selected_lambda]);
   analysis_plot->setTitle(s2);
   analysis_plot->setAxisTitle(QwtPlot::xBottom, tr("Radius in cm"));
   analysis_plot->setAxisTitle(QwtPlot::yLeft, s3);

   line_x[0] = radius[0]-0.02;
   line_x[1] = radius[points-1]+0.02;
   linecount = 0;
   for (i=(unsigned int) first_resid-1; i<(unsigned int) first_resid+num_scans-1; i++)
   {
      line[linecount]  = analysis_plot->insertCurve("Baselines");
      line_y[linecount][0] = linecount*0.03;
      line_y[linecount][1] = linecount*0.03;
      curve[i] = analysis_plot->insertCurve("Residuals");

      // we need to recalculate the residuals to add the proper offsets and when loaded from
      // fit the residuals are unknown

      for (j=0; j<points; j++)
      {
         residuals[i][j] = absorbance[i][j] - F[i][j] + line_y[linecount][0];
      }
      analysis_plot->setCurvePen(curve[i], green);
      analysis_plot->setCurveData(curve[i], radius, residuals[i], points);
      analysis_plot->setCurvePen(line[linecount], white);
      analysis_plot->setCurveData(line[linecount], line_x, line_y[linecount], 2);
      linecount++;
   }
   analysis_plot->replot();
   //analysis_plot->updatePlot();      // no updatePlot() in new version
   update_plot(first_resid);   //need to paint the proper scans in red.
}

void US_Finite_single::save()
{
   if (step == 0)
   {
      QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n      Click on \"Load Data\"\n"));
      return;
   }
   if (!converged)
   {
      QMessageBox::message(tr("Attention:\n"),tr("     There is nothing to save...\n\n"
                                                 "Please fit your data first until it is\n"
                                                 "converged before trying to save the results.\n"));
      return;
   }
   write_fef();
   QPixmap p;
   QString fileName;
   for (unsigned int i=0; i<run_inf.scans[selected_cell][selected_lambda]; i+=5)
   {
      update_residuals((double) i+1);
      qApp->processEvents();
      fileName.sprintf(htmlDir + "/fef_%d%d_%d.", selected_cell + 1, selected_lambda + 1, i+1);
      p = QPixmap::grabWidget(analysis_plot, 2, 2, analysis_plot->width() - 4, analysis_plot->height() - 4);
      pm->save_file(fileName, p);
      fileName.sprintf(htmlDir + "/fef_edited_%d%d_%d.", selected_cell + 1, selected_lambda +1, i+1);
      p = QPixmap::grabWidget(edit_plot, 2, 2, edit_plot->width() - 4, edit_plot->height() - 4);
      pm->save_file(fileName, p);
   }
}

void US_Finite_single::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/finite-ad.html");
}

int US_Finite_single::plot_analysis()
{
   return(0);
}

void US_Finite_single::update_plot(const unsigned int first_residual)
{
   unsigned int i, last_red;
   unsigned int *curve_abs;
   curve_abs = new unsigned int [run_inf.scans[selected_cell][selected_lambda]];
   edit_plot->clear();
   for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
   {
      curve_abs[i] = edit_plot->insertCurve("Optical Density");
      edit_plot->setCurvePen(curve_abs[i], yellow);
      edit_plot->setCurveData(curve_abs[i], radius, absorbance[i], points);
   }

   if ((run_inf.scans[selected_cell][selected_lambda] - first_residual) < 5)
   {
      last_red = run_inf.scans[selected_cell][selected_lambda];
   }
   else
   {
      last_red = first_residual+4;
   }
   for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
   {
      curve[i] = edit_plot->insertCurve("Optical Density");
      if ((i >= first_residual - 1) && (i < last_red))
      {
         edit_plot->setCurvePen(curve[i], red);
      }
      else
      {
         edit_plot->setCurvePen(curve[i], cyan);
      }
      edit_plot->setCurveData(curve[i], radius, F[i], points);
   }
   edit_plot->replot();
   //edit_plot->updatePlot();      //no updatePlot() in new version
   unsigned int modulus = run_inf.scans[selected_cell][selected_lambda] % 5;
   switch (modulus)
   {
   case 0:
      {
         residuals_counter->setRange(1, run_inf.scans[selected_cell][selected_lambda]-4, 5);
         break;
      }
   case 1:
      {
         residuals_counter->setRange(1, run_inf.scans[selected_cell][selected_lambda], 5);
         break;
      }
   case 2:
      {
         residuals_counter->setRange(1, run_inf.scans[selected_cell][selected_lambda]-1, 5);
         break;
      }
   case 3:
      {
         residuals_counter->setRange(1, run_inf.scans[selected_cell][selected_lambda]-2, 5);
         break;
      }
   case 4:
      {
         residuals_counter->setRange(1, run_inf.scans[selected_cell][selected_lambda]-3, 5);
         break;
      }
   }
   residuals_counter->setValue(first_residual);
   delete [] curve_abs;
}

void US_Finite_single::fit_init()
{
   if (!mem_initialized)
   {
      F = new double* [run_inf.scans[selected_cell][selected_lambda]];
      residuals = new double *[run_inf.scans[selected_cell][selected_lambda]];
      for (unsigned int i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
      {
         residuals[i] = new double [points];
         F[i]   = new double [points];
      }
   }
   mem_initialized = true;
}

void US_Finite_single::cleanup()
{
   if (mem_initialized)
   {
      unsigned int i;
      for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
      {
         delete [] residuals[i];
         delete [] F[i];
      }
      delete [] F;
      delete [] residuals;
   }
   mem_initialized = false;
}

void US_Finite_single::view()
{
   if (step == 0)
   {
      QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n      Click on \"Load Data\"\n"));
      return;
   }
   if (!converged)
   {
      QMessageBox::message(tr("Attention:\n"),tr("     There is nothing to report...\n\n"
                                                 "Please fit your data first until it is\n"
                                                 "converged before trying to save the results.\n"));
      return;
   }
   QString filestr, filename, temp;
   filename = USglobal->config_list.result_dir.copy();
   filename.append("/");
   filename.append(run_inf.run_id);
   temp.sprintf(".%d%d", selected_cell+1, selected_lambda+1);
   filestr = filename.copy();
   filestr.append(".fef_res");
   filestr.append(temp);
   //view_file(filestr);
   TextEdit *e;
   e = new TextEdit();
   e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
   e->load(filestr);
   e->show();
}

void US_Finite_single::load_fit()
{
   if (model_widget && fitting_widget)
   {
      QMessageBox mb(tr("UltraScan - Please Note:"), 
                     tr("A model and fit is currently active\n"
                        "Do you want to abandon the current fit\n"
                        "and start a new model and fit?"),
                     QMessageBox::Warning,
                     QMessageBox::Yes | QMessageBox::Default,
                     QMessageBox::No | QMessageBox::Escape, 0);
      mb.setButtonText(QMessageBox::Yes, tr("Continue"));
      mb.setButtonText(QMessageBox::No, tr("Cancel"));
      switch(mb.exec())
      {
      case QMessageBox::Yes:
         {
            model_control->close();
            delete model_control;
            velocFitter->close();
            delete velocFitter;
            break;
         }
      default:
         {
            return;
         }
      }
   }
   if (model_widget)
   {
      QMessageBox mb(tr("UltraScan - Please Note:"), 
                     tr("A model is currently active\n"
                        "Do you want to abandon the\n"
                        "current model and start a new model?"),
                     QMessageBox::Warning,
                     QMessageBox::Yes | QMessageBox::Default,
                     QMessageBox::No | QMessageBox::Escape, 0);
      mb.setButtonText(QMessageBox::Yes, tr("Continue"));
      mb.setButtonText(QMessageBox::No, tr("Cancel"));
      switch(mb.exec())
      {
      case QMessageBox::Yes:
         {
            model_control->close();
            delete model_control;
            break;
         }
      default:
         {
            return;
         }
      }
   }
   if (fitting_widget)
   {
      QMessageBox mb(tr("UltraScan - Please Note:"), 
                     tr("A fit is currently active\n"
                        "Do you want to abandon the current fit\n"
                        "and start a new model and fit?"),
                     QMessageBox::Warning,
                     QMessageBox::Yes | QMessageBox::Default,
                     QMessageBox::No | QMessageBox::Escape, 0);
      mb.setButtonText(QMessageBox::Yes, tr("Continue"));
      mb.setButtonText(QMessageBox::No, tr("Cancel"));
      switch(mb.exec())
      {
      case QMessageBox::Yes:
         {
            velocFitter->close();
            delete velocFitter;
            break;
         }
      default:
         {
            return;
         }
      }
   }
   QString fn = QFileDialog::getOpenFileName(USglobal->config_list.root_dir, "*.fef_fit", 0);
   if ( !fn.isEmpty() ) 
   {
      load_fit(fn);
   }
}

void US_Finite_single::load_fit(const QString &filename)
{
   QFile f(filename);
   fitName = filename;
   QString str;
   Q_UINT16 int16;
   Q_UINT16 temp_scans;
   Q_UINT32 int32;
   float val;
   if (f.open(IO_ReadOnly))
   {
      QDataStream ds(&f);
      ds >> str;   // run ID
      ds >> int16;
      selected_cell = (unsigned int) int16;
      ds >> int16;
      selected_lambda = (unsigned int) int16;
      ds >> int16;
      parameters = (unsigned int) int16;
      ds >> temp_scans;
      fn = USglobal->config_list.result_dir + "/" + str + ".us.v";
      reset_flag = true;
      if (load_data(fn) != 0)
      {
         QMessageBox::message(tr("Attention:"), tr("There was a problem loading your fit.\n\n"
                                                   "Please try again."));
         return;
      }
      details();
      ds >> val;
      bd_range = val;
      ds >> val;
      bd_position = val;
      ds >> density;
      ds >> viscosity;
      ds >> standard_deviation;
      variance = pow(standard_deviation, 2);
      ds >> int32;
      sim_points = (unsigned int) int32;
      for (unsigned int i=0; i<(unsigned int) temp_scans; i++)
      {
         ds >> int32;
         while (run_inf.time[selected_cell][selected_lambda][i] != int32)
         {
            exclude_single = i+1;
            ex_single();
         }   // exclude scans until the times match.
      }
      // if there were scans excluded at the end of the run, we don't catch them with the exclusion algorithm
      // above, so we need to catch them by comparing the true number of scans with the temp_scan variable

      while (run_inf.scans[selected_cell][selected_lambda] > (unsigned int) temp_scans)
      {
         exclude_single = run_inf.scans[selected_cell][selected_lambda];   // set to the last scan of the original dataset
         ex_single();
      }   // exclude scans at the end until the numbers of scans match
      
      plot_edit();
      ds >> int16;
      model = (unsigned int) int16;
      ds >> int16;
      components = (unsigned int) int16;
      step = 1;
      create_model();
      ds >> runData->meniscus;
      ds >> runData->meniscus_range;
      ds >> int16;
      runData->meniscus_fit = (unsigned int) int16;
      ds >> runData->baseline;
      ds >> runData->baseline_range;
      ds >> int16;
      runData->baseline_fit = (unsigned int) int16;
      ds >> runData->slope;
      ds >> runData->slope_range;
      ds >> int16;
      runData->slope_fit = (unsigned int) int16;
      ds >> runData->stray;
      ds >> runData->stray_range;
      ds >> int16;
      runData->stray_fit = (unsigned int) int16;
      ds >> runData->delta_t;
      ds >> runData->delta_r;
      for (unsigned int i=0; i<components; i++)
      {
         ds >> component_vector[i].sed;
         ds >> component_vector[i].sed_range;
         ds >> int16;
         component_vector[i].sed_fit = (unsigned int) int16;
         ds >> component_vector[i].diff;
         ds >> component_vector[i].diff_range;
         ds >> int16;
         component_vector[i].diff_fit = (unsigned int) int16;
         ds >> component_vector[i].conc;
         ds >> component_vector[i].conc_range;
         ds >> int16;
         component_vector[i].conc_fit = (unsigned int) int16;
         ds >> component_vector[i].sigma;
         ds >> component_vector[i].sigma_range;
         ds >> int16;
         component_vector[i].sigma_fit = (unsigned int) int16;
         ds >> component_vector[i].delta;
         ds >> component_vector[i].delta_range;
         ds >> int16;
         component_vector[i].delta_fit = (unsigned int) int16;
         ds >> int16;
         component_vector[i].mw_fit = (unsigned int) int16;
         ds >> component_vector[i].vbar;
         ds >> int16;
         component_vector[i].vbar_fit = (unsigned int) int16;
      }
      update_buffer_lbl(density, viscosity);
      update_buffer_signal(density, viscosity);
      model_control->update_labels();
      fit_init();
      for (unsigned int i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
      {
         for (unsigned int j=0; j<run_inf.points[selected_cell][selected_lambda][0]; j++)
         {
            ds >> val;
            F[i][j] = (double) val;
         }
      }
      f.close();
      converged = true;
      first_plot = true;
      update_plot(1);
      update_residuals(1);
      pb_second_plot->setEnabled(true);
      pb_save_fit->setEnabled(true);
      pb_save->setEnabled(true);
      pb_view->setEnabled(true);
   }
}

void US_Finite_single::write_fef()
{
   QString filestr1, filestr2, filestr3, filename, temp, message;
   filename = USglobal->config_list.result_dir.copy();
   filename.append("/");
   filename.append(run_inf.run_id);
   temp.sprintf(".%d%d", selected_cell+1, selected_lambda+1);
   filestr1 = filename.copy();
   filestr1.append(".fef_dat");
   filestr1.append(temp);
   QFile fef_f1(filestr1);
   unsigned int i, j;
   int f1_error, f2_error, f3_error;
   if (fef_f1.open(IO_WriteOnly | IO_Translate))
   {
      QTextStream ts(&fef_f1);
      ts << tr("Radius:\t");
      for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
      {   
         ts << tr("Scan-") << (i+1) << "\t";
      }
      ts << "\n";
      for (j=0; j<points; j++)
      {
         ts << radius[j] << "\t";
         for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
         {   
            ts << F[i][j] << "\t";
         }
         ts << "\n";
      }
      fef_f1.close();
      f1_error=0;
   }
   else
   {
      f1_error=1;
   }
   filename = "";
   filename = USglobal->config_list.result_dir.copy();
   filename.append("/");
   filename.append(run_inf.run_id);
   temp.sprintf(".%d%d", selected_cell+1, selected_lambda+1);
   filestr2 = filename.copy();
   filestr2.append(".fef_residuals");
   filestr2.append(temp);
   QFile fef_f2(filestr2);
   if (fef_f2.open(IO_WriteOnly | IO_Translate))
   {
      QTextStream ts(&fef_f2);
      ts << tr("Radius:\t");
      for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
      {   
         ts << tr("Scan-") << (i+1) << "\t";
      }
      ts << "\n";
      for (j=0; j<points; j++)
      {
         ts << radius[j] << "\t";
         for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
         {   
            ts << residuals[i][j] << "\t";
         }
         ts << "\n";
      }
      fef_f2.close();
      f2_error=0;
   }
   else
   {
      f2_error=1;
   }
   filename = "";
   filename = USglobal->config_list.result_dir.copy();
   filename.append("/");
   filename.append(run_inf.run_id);
   temp.sprintf(".%d%d", selected_cell+1, selected_lambda+1);
   filestr3 = filename.copy();
   filestr3.append(".fef_raw");
   filestr3.append(temp);
   QFile fef_f3(filestr3);
   if (fef_f3.open(IO_WriteOnly | IO_Translate))
   {
      QTextStream ts(&fef_f3);
      ts << tr("Radius:\t");
      for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
      {   
         ts << tr("Scan-") << (i+1) << "\t";
      }
      ts << "\n";
      for (j=0; j<points; j++)
      {
         ts << radius[j] << "\t";
         for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
         {   
            ts << absorbance[i][j] << "\t";
         }
         ts << "\n";
      }
      fef_f3.close();
      f3_error=0;
   }
   else
   {
      f3_error=1;
   }
   message = tr("The following files where saved to disk:\n\n");
   if (!f1_error)
   {
      message.append(tr("The Finite Element Simulation:\n"));
      message.append(filestr1);
      message.append("\n\n");
   }
   if (!f2_error)
   {
      message.append(tr("The Residuals of the Fit:\n"));
      message.append(filestr2);
      message.append("\n\n");
   }
   if (!f3_error)
   {
      message.append(tr("The Experimental Data:\n"));
      message.append(filestr3);
      message.append("\n");
   }
   message.append("\n");
   if (f1_error || f2_error || f3_error)
   {
      message.append(tr("There was a problem saving the following files:\n\n"));
   }
   if (f1_error)
   {
      message.append(tr("The Finite Element Simulation:\n"));
      message.append(filestr1);
      message.append("\n\n");
   }
   if (f2_error)
   {
      message.append(tr("The Residuals of the Fit:\n"));
      message.append(filestr2);
      message.append("\n\n");
   }
   if (f3_error)
   {
      message.append(tr("The Experimental Data:\n"));
      message.append(filestr3);
      message.append("\n");
   }
   QMessageBox::message(tr("Please note:\n"), message);
}
