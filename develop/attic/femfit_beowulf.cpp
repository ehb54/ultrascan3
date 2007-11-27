#include "../include/femfit_beowulf.h"

int main (int argc, char **argv)
{
	US_Finite_W_noGUI *Beowulf;
	Beowulf = new US_Finite_W_noGUI();
	return 0;
}

US_Finite_W_noGUI::US_Finite_W_noGUI()
{
	converged = false;
	completed = false;
	aborted = false;
	step = 0;
	constrained_fit = true;
	model = 0;
	components = 1;
}

US_Finite_W_noGUI::~US_Finite_W_noGUI()
{
}

void US_Finite_W_noGUI::dud()
{
	QString str, str1;
	if (converged || completed || aborted)
	{
		cleanup_dud();
	}

	first_plot = true;		// reset first plot each time we do a new fit
	completed = false;
	aborted = false;
	converged = false;
	func_eval = 0;
	float improvement=0, d, d_start=0.5;
	unsigned int i, j, loopcount = 0;
	init_simulation = false;
	dud_init();		// initialize the basis vectors
	F_init();		// initialize the corresponding function vectors
	if (aborted)
	{
		suspend_flag = false;
		return;
	}
	order_variance();			// sort the variances - theta causing largest goes into first element, theta causing best variance goes into parameters+1
	while (!aborted)
	{
		for (i=0; i<parameters; i++)
		{
			calc_deltas(i);
		}
		if (!calc_alpha())
		{
			cout << "Attention: The Cholesky Decomposition of the\nHessian matrix failed due to a\n"
			<<	"singularity in the matrix.\n\nYou can probably achieve convergence\nby re-fitting with the current\n"
			<< "parameter estimate as initial guess.\n";
			aborted = true;
			return;
		}

// update theta. We don't know how good the new guess is, so we temporarily place this
// guess for theta into the extra theta vector (p+2 = [parameters+1]).
// Then we calculate the residuals and reorder afterwards:

		for (i=0; i<parameters; i++)
		{
			theta[parameters+1][i] = 0.0;
			for (j=0; j<parameters; j++)
			{
				theta[parameters+1][i] += delta_theta[j][i] * alpha[j];
			}
			theta[parameters+1][i] += theta[parameters][i];
		}
		d = 0.1;
		for (i=0; i<parameters; i++)
		{
			theta[parameters+1][i] = d * theta[parameters+1][i] + (1.0 - d) * theta[parameters][i];
		}
		iteration ++;
		if (constrained_fit)
		{
			update_simulation_parameters_constrained(parameters+1);
		}
		else
		{
			update_simulation_parameters_unconstrained(parameters+1);
		}
		switch (model)
		{
			case 0:
			{
				completed = false;
				while (!completed)
				{
					non_interacting_model();
					while (suspend_flag)
					{
						if (aborted)
						{
							suspend_flag = false;
							return;
						}
					}
				}
				break;
			}
			case 1:
			{
				completed = false;
				while (!completed)
				{
					monomer_dimer_model();
					while (suspend_flag)
					{
						if (aborted)
						{
							suspend_flag = false;
							return;
						}
					}
				}
				break;
			}
			case 2:
			{
				completed = false;
				while (!completed)
				{
					isomerizing_model();
					while (suspend_flag)
					{
						if (aborted)
						{
							suspend_flag = false;
							return;
						}
					}
				}
				break;
			}
		}
		assign_F(parameters+1);
		calc_residuals(parameters+1);
		improvement =  variance[parameters] - variance[parameters+1];
		loopcount = 0;
		while (improvement <= 0.0)		// we need a step shortening procedure here
		{	//implement step shortening procedure
			loopcount++;
			d = (-1.0) * pow(-d_start, loopcount);
			for (i=0; i<parameters; i++)
			{
				theta[parameters+1][i] = d * theta[parameters+1][i] + (1.0 - d) * theta[parameters][i];
			}
			if (constrained_fit)	// =different GUI's depending on this flag
			{
				update_simulation_parameters_constrained(parameters+1);
			}
			else
			{
				update_simulation_parameters_unconstrained(parameters+1);
			}
			switch (model)
			{
				case 0:
				{
					completed = false;
					while (!completed)
					{
						non_interacting_model();	// calculate
						while (suspend_flag)
						{
							if (aborted)
							{
								suspend_flag = false;
								return;
							}
						}
					}
					break;
				}
				case 1:
				{
					completed = false;
					while (!completed)
					{
						monomer_dimer_model();		// calculate
						while (suspend_flag)
						{
							if (aborted)
							{
								suspend_flag = false;
								return;
							}
						}
					}
					break;
				}
				case 2:
				{
					completed = false;
					while (!completed)
					{
						isomerizing_model();			// calculate
						while (suspend_flag)
						{
							if (aborted)
							{
								suspend_flag = false;
								return;
							}
						}
					}
					break;
				}
			}
			assign_F(parameters+1);
			calc_residuals(parameters+1);
			improvement =  variance[parameters] - variance[parameters+1];
			if (improvement == 0 || loopcount > 5)
			{
				converged = true;
//				update_plot(1);	//show the first set of residuals
				cleanup_finite();
				return; // we can't do any better, quit (this is the exit of this function)
			}
		}
		if (alpha[0] >= 1.0e-5)
		{
			shift_all();
		}
		else
		{
			j = 0;
			while ((alpha[j] < 1.0e-5) && (j < parameters-1))
			{
				j ++; // find first alpha[j] that is larger/equal than 1e-5 and use that index's alpha to replace with theta new
			}
			loopcount++;
			for (i=0; i<parameters; i++) 
			{ // make sure that old values of theta1 aren't retained forever
				theta[0][i] = (theta[0][i] + theta[parameters+1][i]) / 2.0;
				theta[j][i] = theta[parameters+1][i]; // update theta[j] with the new theta.
				variance[j] = variance[parameters+1];
			} // and replace with the average of theta1 and theta_new
			for (i=0; i<allpoints; i++)
			{
				F[j][i] = F[parameters+1][i];	// we already have the F[j] from above, no need to re-calculate
			}
			if (constrained_fit)
			{
				update_simulation_parameters_constrained(0);
			}
			else
			{
				update_simulation_parameters_unconstrained(0);
			}
			switch (model)
			{
				case 0:
				{
					completed = false;
					while (!completed)
					{
						non_interacting_model();
						while (suspend_flag)
						{
							if (aborted)
							{
								suspend_flag = false;
								return;
							}
						}
					}
					break;
				}
				case 1:
				{
					completed = false;
					while (!completed)
					{
						monomer_dimer_model();
						while (suspend_flag)
						{
							if (aborted)
							{
								suspend_flag = false;
								return;
							}
						}
					}
					break;
				}
				case 2:
				{
					completed = false;
					while (!completed)
					{
						isomerizing_model();
						while (suspend_flag)
						{
							if (aborted)
							{
								suspend_flag = false;
								return;
							}
						}
					}
					break;
				}
			}
			assign_F(0);
			calc_residuals(0);
			improvement =  variance[parameters+1] - variance[0]; // the best one we have so far is theta[parameters+1]
			order_variance();
		}
	}
}
void US_Finite_W_noGUI::cleanup_dud()
{
	unsigned int i;
	for (i=0; i<parameters; i++)
	{
		delete [] delta_theta[i];
		delete [] delta_F[i];
	}
	for (i=0; i<parameters+3; i++)
	{
		delete [] theta[i];
	}
	for (i=0; i<parameters+2; i++)
	{
		delete [] F[i];
	}
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		delete [] residuals[i];
		delete [] y[i];
		delete [] line_y[i];
	}
	delete [] theta;
	delete [] delta_theta;
	delete [] F;
	delete [] delta_F;
	delete [] alpha;
	delete [] variance;
	delete [] old_variance;
	delete [] order;
	delete [] residuals;
	delete [] y;
	delete [] line_x;
	delete [] line;
	delete [] line_y;
}

void US_Finite_W_noGUI::dud_init()
{
	unsigned int i, j, parameter_count=0;

// How many parameters need to be fit?

	parameters = 0;
	if (run_vector->meniscus_fit)
	{
		parameters++;
	}
	if (run_vector->baseline_fit)
	{
		parameters++;
	}
	if (run_vector->slope_fit)
	{
		parameters++;
	}
	if (run_vector->stray_fit)
	{
		parameters++;
	}
	for (i=0; i<components; i++)
	{
		if (component_vector[i].sed_fit)
		{
			parameters++;
		}
		if (component_vector[i].diff_fit)
		{
			parameters++;
		}
		if (component_vector[i].conc_fit)
		{
			parameters++;
		}
		if (component_vector[i].sigma_fit)
		{
			parameters++;
		}
		if (component_vector[i].delta_fit)
		{
			parameters++;
		}
	}
	iteration = 0;

// About the parameters for MW and Vbar:
// if S and D are floating, either MW or vbar also must be floating
// if the molecular weight is fixed, we can still float vbar, but if
// both are fixed, we have to adjust the ratio of s/D by floating S
// and adjusting D through the ratio constraint. In that case D doesn't float
// even though it is adjusted, but the adjustment is dictated by the ratio,
// and D is not considered a fitted parameter.

// Rows (in dimension 1):

	theta			 = new float*  [parameters + 3];	//columns in dimension 1 (1 extra for theta_new, 1 for bounds checking)
	delta_theta	 = new float*  [parameters];		//columns in dimension 1
	F 				 = new float*  [parameters + 2];	//columns in dimension 1 (1 extra for theta_new)
	delta_F		 = new float*  [parameters];		//columns in dimension 1
	alpha			 = new float   [parameters];
	variance		 = new float   [parameters + 2];
	old_variance = new float   [parameters + 2];
	order 		 = new unsigned int [parameters + 1];
	residuals	 = new double* [run_inf.scans[selected_cell][selected_lambda]];
	y 				 = new double* [run_inf.scans[selected_cell][selected_lambda]];
	line_x		 = new double  [2];
	line			 = new uint		[5];
	line_y		 = new double* [run_inf.scans[selected_cell][selected_lambda]];
	for (i=0; i<parameters; i++)
	{
		delta_theta[i]	= new float [parameters];
		delta_F[i] 		= new float [points * run_inf.scans[selected_cell][selected_lambda]];
	}
	for (i=0; i<parameters+3; i++)
	{
		theta[i] 		= new float [parameters];
	}
	for (i=0; i<parameters+2; i++)
	{
		F[i] 				= new float [points * run_inf.scans[selected_cell][selected_lambda]];
	}
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		residuals[i]	= new double [points];
		y[i] 				= new double [points];
		line_y[i] 		= new double [2];
	}

// Initialize Theta with an improved scheme:

	for (i=0; i<parameters+1; i++)
	{
		parameter_count = 0;

// For each column, check if it is the last column or for any other column, check if the element
// is on the diagonal or off-diagonal

		if (run_vector->meniscus_fit)
		{
			if (i == parameters)				// this is the p+1 column of Theta, which is not offset by the range bias
			{
				theta[i][parameter_count] = run_vector->meniscus;
			}
			else
			{
				if (i == parameter_count)		//then we have a diagonal element
				{
					theta[i][parameter_count] = run_vector->meniscus + run_vector->meniscus_range;
				}
				else 		//then we have an off-diagonal element
				{
					theta[i][parameter_count] = run_vector->meniscus - run_vector->meniscus_range;
				}
			}
			parameter_count++;
		}
		if (run_vector->baseline_fit)
		{
			if (i == parameters)				// this is the p+1 column of Theta, which is not offset by the range bias
			{
				theta[i][parameter_count] = run_vector->baseline;
			}
			else
			{
				if (i == parameter_count)		//then we have a diagonal element
				{
					theta[i][parameter_count] = run_vector->baseline + run_vector->baseline_range;
				}
				else 		//then we have an off-diagonal element
				{
					theta[i][parameter_count] = run_vector->baseline - run_vector->baseline_range;
				}
			}
			parameter_count++;
		}
		if (run_vector->slope_fit)
		{
			if (i == parameters)				// this is the p+1 column of Theta, which is not offset by the range bias
			{
				theta[i][parameter_count] = run_vector->slope;
			}
			else
			{
				if (i == parameter_count)		//then we have a diagonal element
				{
					theta[i][parameter_count] = run_vector->slope + run_vector->slope_range;
				}
				else 		//then we have an off-diagonal element
				{
					theta[i][parameter_count] = run_vector->slope - run_vector->slope_range;
				}
			}
			parameter_count++;
		}
		if (run_vector->stray_fit)
		{
			if (i == parameters)				// this is the p+1 column of Theta, which is not offset by the range bias
			{
				theta[i][parameter_count] = run_vector->stray;
			}
			else
			{
				if (i == parameter_count)		//then we have a diagonal element
				{
					theta[i][parameter_count] = run_vector->stray + run_vector->stray_range;
				}
				else 		//then we have an off-diagonal element
				{
					theta[i][parameter_count] = run_vector->stray - run_vector->stray_range;
				}
			}
			parameter_count++;
		}
		for (j=0; j<components; j++)
		{
			if (component_vector[j].sed_fit)
			{
				if (i == parameters)				// this is the p+1 column of Theta, which is not offset by the range bias
				{
					theta[i][parameter_count] = component_vector[j].sed;
				}
				else
				{
					if (i == parameter_count)		//then we have a diagonal element
					{
						theta[i][parameter_count] = component_vector[j].sed + component_vector[j].sed_range;
					}
					else 		//then we have an off-diagonal element
					{
						theta[i][parameter_count] = component_vector[j].sed - component_vector[j].sed_range;
					}
				}
				parameter_count++;
			}
			if (component_vector[j].diff_fit)
			{
				if (i == parameters)				// this is the p+1 column of Theta, which is not offset by the range bias
				{
					theta[i][parameter_count] = component_vector[j].diff;
				}
				else
				{
					if (i == parameter_count)		//then we have a diagonal element
					{
						theta[i][parameter_count] = component_vector[j].diff + component_vector[j].diff_range;
					}
					else 		//then we have an off-diagonal element
					{
						theta[i][parameter_count] = component_vector[j].diff - component_vector[j].diff_range;
					}
				}
				parameter_count++;
			}
			if (component_vector[j].conc_fit)
			{
				if (i == parameters)				// this is the p+1 column of Theta, which is not offset by the range bias
				{
					theta[i][parameter_count] = component_vector[j].conc;
				}
				else
				{
					if (i == parameter_count)		//then we have a diagonal element
					{
						theta[i][parameter_count] = component_vector[j].conc + component_vector[j].conc_range;
					}
					else 		//then we have an off-diagonal element
					{
						theta[i][parameter_count] = component_vector[j].conc - component_vector[j].conc_range;
					}
				}
				parameter_count++;
			}
			if (component_vector[j].sigma_fit)
			{
				if (i == parameters)				// this is the p+1 column of Theta, which is not offset by the range bias
				{
					theta[i][parameter_count] = component_vector[j].sigma;
				}
				else
				{
					if (i == parameter_count)		//then we have a diagonal element
					{
						theta[i][parameter_count] = component_vector[j].sigma + component_vector[j].sigma_range;
					}
					else 		//then we have an off-diagonal element
					{
						theta[i][parameter_count] = component_vector[j].sigma - component_vector[j].sigma_range;
					}
				}
				parameter_count++;
			}
			if (component_vector[j].delta_fit)
			{
				if (i == parameters)				// this is the p+1 column of Theta, which is not offset by the range bias
				{
					theta[i][parameter_count] = component_vector[j].delta;
				}
				else
				{
					if (i == parameter_count)		//then we have a diagonal element
					{
						theta[i][parameter_count] = component_vector[j].delta + component_vector[j].delta_range;
					}
					else 		//then we have an off-diagonal element
					{
						theta[i][parameter_count] = component_vector[j].delta - component_vector[j].delta_range;
					}
				}
				parameter_count++;
			}
		}
	}
	
// save the p+1 column in the p+3 column for later bounds checking. The p+3 column will not get changed
// The p+2 column is reserved for new_theta. The P+3 column will be used to calculate the maximal bounds
// for the constrain checking.

	for (i=0; i<parameters; i++)
	{
		theta[parameters+2][i] = theta[parameters][i];
	}

/*
// print out initialized theta matrix:
	cout << "in dud_init:\n";
	for (i=0; i<parameters; i++)
	{
		for (j=0; j<parameters+3; j++)
		{
			cout << theta[j][i] << "\t";
		}
		cout << "\n";
	}
*/
}

void US_Finite_W_noGUI::F_init()
{
	unsigned int i;
	float improvement=0;
	for (i=0; i<parameters+1; i++)
	{ 
		if (constrained_fit)
		{
			update_simulation_parameters_constrained(i);
		}
		else
		{
			update_simulation_parameters_unconstrained(i);
		}

		switch (model)
		{
			case 0:
			{
				completed = false;
				while (!completed)
				{
					non_interacting_model();
					while (suspend_flag)
					{
						if (aborted)
						{
							suspend_flag = false;
							return;
						}
					}
				}
				break;
			}
			case 1:
			{
				completed = false;
				while (!completed)
				{
					monomer_dimer_model();
					while (suspend_flag)
					{
						if (aborted)
						{
							suspend_flag = false;
							return;
						}
					}
				}
				break;
			}
			case 2:
			{
				completed = false;
				while (!completed)
				{
					isomerizing_model();
					while (suspend_flag)
					{
						if (aborted)
						{
							suspend_flag = false;
							return;
						}
					}
				}
				break;
			}
		}
		assign_F(i);
		calc_residuals(i);
		if (i > 0)
		{
			improvement =  variance[i-1] - variance[i];
		}
	}
}

void US_Finite_W_noGUI::assign_F(const unsigned int i)
{

// synchronize the radial stepsize in the simulation with the stepsize in the raw data:

	unsigned int j=0, k, start_index, stop_index, count, radius_count;
	float m, b=0.0;
	
	while (run_inf.range_left[selected_cell][selected_lambda][0] >= sim_radius[j])
	{
		j++;
	}
	start_index = j-1;
	while (run_inf.range_right[selected_cell][selected_lambda][0] >= sim_radius[j])
	{
		j++;
	}
	stop_index = j-1;

	if (run_vector->delta_r == run_inf.delta_r)
	{
		count = 0;
		for (k=0; k<run_inf.scans[selected_cell][selected_lambda]; k++)
		{
			for (j=start_index; j<start_index+points; j++) //second dimension of F is equal to number of points*scans in raw data
			{
				F[i][count] = concentration[k][j];
				count++;
			}
		}
		allpoints = count; //should be the same as points*scans
		if (allpoints != points*run_inf.scans[selected_cell][selected_lambda])
		{
			cout << "Problem with number of points - they don't match!\n";
			cout << "Allpoints: " << allpoints << ", points*scans: " << points*run_inf.scans[selected_cell][selected_lambda] << endl;
		}
	}
	else // if the radial discretizations don't agree, we need to interpolate:
	{
		count = 0;
		for (k=0; k<run_inf.scans[selected_cell][selected_lambda]; k++)
		{
			radius_count = start_index;
			for (j=0; j<points; j++)
			{
				while (sim_radius[radius_count] < radius[j])
				{
					radius_count++;
				}
				m = (concentration[k][radius_count] - concentration[k][radius_count-1])
					/(sim_radius[radius_count] - sim_radius[radius_count-1]);
				b = concentration[k][radius_count] - m * sim_radius[radius_count];
				F[i][count] = m * radius[j] + b;
				count++;
			}
		}
		allpoints = count;
	}
}

void US_Finite_W_noGUI::monomer_dimer_model()
{
	unsigned int i, j, k, time_steps, scan = 0, count = 0;
	float temp1, temp2, scan_timer;

	time_steps = (unsigned int) (0.5 + (run_inf.time[selected_cell][selected_lambda][run_inf.scans[selected_cell][selected_lambda] - 1]
	/ run_vector->delta_t));
	if (!init_simulation)
	{
		init_finite_element();
	}
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		for (j=0; j<sim_points; j++)
		{
			concentration[i][j]=0.0;
		}
	}
//
// allocate memory for the partial concentrations of monomer ([0]) and dimer ([1]):
//
	for (i=0; i<sim_points; i++)
	{
		right[i] = component_vector[0].conc;
		partial_c[0][i] = (-1 + pow((1 + 4 * right[i] * component_vector[1].conc), 0.5))
								/ 2 * component_vector[1].conc;
		partial_c[1][i] = right[i] - partial_c[0][i];
	}
	func_eval++;
	temp1 = run_vector->delta_t * omega_s;
	scan_timer=0.0;
	for (j=0; j<time_steps; j++)
	{
		count++;
		scan_timer += run_vector->delta_t;
		for (i=0; i<sim_points; i++)
		{
			if ((partial_c[0][i] + partial_c[1][i]) > 1e-16)
			{
				s_bar[i] = (component_vector[0].sed * partial_c[0][i]
							+ component_vector[1].sed * partial_c[1][i])
							/ (partial_c[0][i] + partial_c[1][i]);
			}
			else
			{
				s_bar[i] = (component_vector[0].sed + component_vector[1].sed)/2;
			}
		}
		for (i=0; i< (unsigned int) sim_points; i++)
		{
			if (((partial_c[0][i+1] - partial_c[0][i]) +
				 (partial_c[1][i+1] - partial_c[1][i])) > 1e-16)
			{
				D_bar[i] = ((component_vector[0].diff * (partial_c[0][i+1] - partial_c[0][i]))
							+ (component_vector[1].diff * (partial_c[1][i+1] - partial_c[1][i])))
							/ ((partial_c[0][i+1] - partial_c[0][i]) +
				 				(partial_c[1][i+1] - partial_c[1][i]));
			}
			else
			{
				D_bar[i] = (component_vector[0].diff + component_vector[1].diff)/2;
			}
		}
		D_bar[sim_points-1] = (component_vector[0].diff + component_vector[1].diff)/2;
		for (i=0; i<sim_points; i++)
		{
			for (k=0; k<3; k++)
			{
				left[i][k] = b[i][k] + run_vector->delta_t * D_bar[i] * a1[i][k] -
				temp1 * s_bar[i] * a2[i][k];
			}
		}
		m3vm(&b, &right, sim_points);
		ldu(&left, &right, run_vector->delta_t, sim_points, 3, run_vector->delta_t);
		if (fabs(run_inf.time[selected_cell][selected_lambda][scan] - scan_timer) <= (run_vector->delta_t/2.0))
		{
			for (i=0; i<sim_points; i++)
			{
				concentration[scan][i] = right[i];
			}
			scan++;
			if (scan == run_inf.scans[selected_cell][selected_lambda])
			{
				scan = 0;
			}
		}
		for (i=0; i<sim_points; i++)
		{
			temp2 = 1 + 4 * right[i] * component_vector[1].conc;
			if (temp2 > 0)
			{
				partial_c[0][i] = (-1 + pow(temp2, 0.5))
									/ (2 * component_vector[1].conc);
				partial_c[1][i] = right[i] - partial_c[0][i];
			}
			else
			{
				cout << "Attention:\n\n" <<
				"The solution encountered a discontinuity at the\n" <<
				"bottom of the cell.\n\n" <<
				"This is most often caused by a combination of\n" <<
				"small diffusion coefficients and large radial\n" <<
				"discretization stepsizes. Please try again with\n" <<
				"a smaller radial discretization stepsize.\n";
				return;
			}
		}
		if(suspend_flag || aborted)
		{
			return;
		}
	}

// before exiting this function, add the baseline and the slope to the run:

	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		for (j=0; j<sim_points; j++)
		{
			concentration[i][j] += run_vector->baseline + (sim_radius[j]-run_inf.range_left[selected_cell][selected_lambda][0]) 
									 	* run_vector->slope + (sim_radius[j]-run_inf.range_left[selected_cell][selected_lambda][0])
										* run_vector->stray * concentration[i][j];
		}
	}
	completed = true;
}

void US_Finite_W_noGUI::isomerizing_model()
{
	unsigned int i, j, k, time_steps, scan = 0, count = 0;
	float temp1, temp2, scan_timer;
	time_steps = (unsigned int) (0.5 + (run_inf.time[selected_cell][selected_lambda][run_inf.scans[selected_cell][selected_lambda] - 1]
	/ run_vector->delta_t));
	if (!init_simulation)
	{
		init_finite_element();
	}
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		for (j=0; j<sim_points; j++)
		{
			concentration[i][j]=0.0;
		}
	}
//
// allocate memory for the partial concentrations of monomer ([0]) and dimer ([1]):
//
	for (i=0; i<sim_points; i++)
	{
		right[i] = component_vector[0].conc;
		partial_c[0][i] = right[i] / (component_vector[1].conc + 1.0);
		partial_c[1][i] = right[i] - partial_c[0][i];
	}
	func_eval++;
	temp1 = run_vector->delta_t * omega_s;
	scan_timer=0.0;
	for (j=0; j<time_steps; j++)
	{
		count++;
		scan_timer += run_vector->delta_t;
		for (i=0; i<sim_points; i++)
		{
			if ((partial_c[0][i] + partial_c[1][i]) > 1e-16)
			{
				s_bar[i] = (component_vector[0].sed * partial_c[0][i]
							+ component_vector[1].sed * partial_c[1][i])
							/ (partial_c[0][i] + partial_c[1][i]);
			}
			else
			{
				s_bar[i] = (component_vector[0].sed + component_vector[1].sed)/2;
			}
		}
		for (i=0; i< (unsigned int) sim_points-1; i++)
		{
			if (((partial_c[0][i+1] - partial_c[0][i]) +
				 (partial_c[1][i+1] - partial_c[1][i])) > 1e-16)
			{
				D_bar[i] = ((component_vector[0].diff * (partial_c[0][i+1] - partial_c[0][i]))
							+ (component_vector[1].diff * (partial_c[1][i+1] - partial_c[1][i])))
							/ ((partial_c[0][i+1] - partial_c[0][i]) +
				 				(partial_c[1][i+1] - partial_c[1][i]));
			}
			else
			{
				D_bar[i] = (component_vector[0].diff + component_vector[1].diff)/2;
			}
		}
		D_bar[sim_points-1] = (component_vector[0].diff + component_vector[1].diff)/2;
		for (i=0; i<sim_points; i++)
		{
			for (k=0; k<3; k++)
			{
				left[i][k] = b[i][k] + run_vector->delta_t * D_bar[i] * a1[i][k] -
				temp1 * s_bar[i] * a2[i][k];
			}
		}
		m3vm(&b, &right, sim_points);
		ldu(&left, &right, run_vector->delta_t, sim_points, 3, run_vector->delta_t);
		if (fabs(run_inf.time[selected_cell][selected_lambda][scan] - scan_timer) <= (run_vector->delta_t/2.0))
		{
			for (i=0; i<sim_points; i++)
			{
				concentration[scan][i] = right[i];
			}
			scan++;
			if (scan == run_inf.scans[selected_cell][selected_lambda])
			{
				scan = 0;
			}
		}
		for (i=0; i<sim_points; i++)
		{
			temp2 = 1 + 4 * right[i] * component_vector[1].conc;
			if (temp2 > 0)
			{
				partial_c[0][i] = right[i] / (component_vector[1].conc + 1.0);
				partial_c[1][i] = right[i] - partial_c[0][i];
			}
			else
			{
				cout << "Attention:\n\n" <<
				"The solution encountered a discontinuity at the\n" <<
				"bottom of the cell.\n\n" <<
				"This is most often caused by a combination of\n" <<
				"small diffusion coefficients and large radial\n" <<
				"discretization stepsizes. Please try again with\n" <<
				"a smaller radial discretization stepsize.\n";
				return;
			}
		}
		if(suspend_flag || aborted)
		{
			return;
		}
	}

// before exiting this function, add the baseline and the slope to the run:

	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		for (j=0; j<sim_points; j++)
		{
			concentration[i][j] += run_vector->baseline + (sim_radius[j]-run_inf.range_left[selected_cell][selected_lambda][0]) 
									 	* run_vector->slope + (sim_radius[j]-run_inf.range_left[selected_cell][selected_lambda][0])
										* run_vector->stray * concentration[i][j];
		}
	}
	completed = true;
}

void US_Finite_W_noGUI::non_interacting_model()
{
	unsigned int i, j, k, time_steps, scan = 0, count = 0;
	bool conc_dep_flag = false;
	float temp1, temp2, scan_timer;

	time_steps = (unsigned int) (0.5 + (run_inf.time[selected_cell][selected_lambda][run_inf.scans[selected_cell][selected_lambda] - 1]/run_vector->delta_t));
	completed = false;
	if (!init_simulation)
	{
		init_finite_element();
	}
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		for (j=0; j<sim_points; j++)
		{
			concentration[i][j]=0.0;
		}
	}
	for (k=0; k<components; k++)
	{
		if (component_vector[k].sigma != 0 || component_vector[k].delta != 0)
		{
			conc_dep_flag = true;
		}
	}
	if (conc_dep_flag)
	{
		c_current = new float [sim_points];
	}
	func_eval++;
	for (k=0; k<components; k++)
	{
		scan_timer=0.0;
		temp1 = run_vector->delta_t * component_vector[k].sed * omega_s;
		temp2 = run_vector->delta_t * component_vector[k].diff;
		for (i=0; i<sim_points; i++)
		{
			right[i] = component_vector[k].conc;
			for (j=0; j<3; j++)
			{
				left[i][j] = b[i][j] + temp2 * a1[i][j] - temp1 * a2[i][j];
			}
		}
		for (j=0; j<time_steps; j++)
		{
			count++;
			scan_timer += run_vector->delta_t;
			if (component_vector[k].sigma != 0 || component_vector[k].delta != 0)
			{
				for (i=0; i<sim_points; i++)
				{
					c_current[i] = right[i];
				}
			}
			m3vm(&b, &right, sim_points);

			if (component_vector[k].sigma != 0)
			{
				conc_dep_s(component_vector[k].sigma, &right, &c_current, &a2, temp1, sim_points);
			}
			if (component_vector[k].delta != 0)
			{
				conc_dep_d(component_vector[k].delta, &right, &c_current, &u, &v, &w, temp2, sim_points,
				&cu, &cv, &cw);
			}
			ldu(&left, &right, scan_timer, sim_points, 3, run_vector->delta_t);
			if (fabs(run_inf.time[selected_cell][selected_lambda][scan] - scan_timer) <= (run_vector->delta_t/2.0))
			{
				for (i=0; i<sim_points; i++)
				{
					concentration[scan][i] += right[i];
				}
				scan++;
				if (scan == run_inf.scans[selected_cell][selected_lambda])
				{
					scan = 0;
				}
			}
			if(suspend_flag || aborted)
			{
				return;
			}
		}
	}

// before exiting this function, add the baseline and the slope to the run:

	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		for (j=0; j<sim_points; j++)
		{
			concentration[i][j] += run_vector->baseline + (sim_radius[j]-run_inf.range_left[selected_cell][selected_lambda][0]) 
									 	* run_vector->slope + (sim_radius[j]-run_inf.range_left[selected_cell][selected_lambda][0])
										* run_vector->stray * concentration[i][j];
		}
	}
	if (conc_dep_flag)
	{
		delete [] c_current;
	}
	completed = true;
}

void US_Finite_W_noGUI::init_finite_element()
{
	unsigned int i;
	bool conc_dep_diff_flag = false;
	init_simulation = true;
	sim_points = (unsigned int) (1.5 + (bottom[selected_cell] - run_vector->meniscus)/run_vector->delta_r);
	sim_radius = new float [sim_points];
	sim_radius[0] = run_vector->meniscus;
	for (i=1; i<sim_points; i++)
	{
		sim_radius[i] = sim_radius[i-1] + run_vector->delta_r;
	}
	for (i=0; i<components; i++)
	{
		if (component_vector[i].delta != 0)
			conc_dep_diff_flag = true;
	}


	if (model == 1 || model == 2)
	{
		s_bar = new float [sim_points];
		D_bar = new float [sim_points];
		partial_c = new float* [2];
		partial_c[0] = new float [sim_points];
		partial_c[1] = new float [sim_points];
	}

	if (conc_dep_diff_flag) //we got concentration dependency, allocate memory and initialize:
	{ 
		cu = new float [sim_points]; 
		cv = new float [sim_points];
		cw = new float [sim_points];
		u = new float* [sim_points];
		v = new float* [sim_points];
		w = new float* [sim_points];
		for (i=0; i<sim_points; i++)
		{
			u[i] = new float [2];
			v[i] = new float [3];
			w[i] = new float [2];
		}
		v[0][0] = 0.0;
		for (i=1; i<sim_points; i++)
		{
			v[i][0] = -(sim_radius)[i-1]/(2.0*run_vector->delta_r)-1.0/6.0;
		}
		v[0][1] = sim_radius[0]/(2.0*run_vector->delta_r)+1.0/6.0;
		for (i=1; i<(unsigned int) (sim_points-1); i++)
		{
			v[i][1] = sim_radius[i]/run_vector->delta_r;
		}
		v[sim_points-1][1] = sim_radius[sim_points-1]/(2.0*run_vector->delta_r)-1.0/6.0;
		for (i=0; i< (unsigned int) sim_points-1; i++)
		{
			v[i][2] = -(sim_radius)[i]/(2.0*run_vector->delta_r)-1.0/3.0;
		}
		v[sim_points-1][2] = 0.0;
		u[0][0] = 0.0;
		for (i=1; i<sim_points; i++)
		{
			u[i][0] = -(v)[i][0];
		}
		for (i=0; i< (unsigned int) sim_points-1; i++)
		{
			u[i][1] = v[i+1][0];
		}
		u[sim_points-1][1] = 0.0;
		w[0][0] = 0.0;
		for (i=1; i<sim_points; i++)
		{
			w[i][0] = v[i-1][2];
		}
		for (i=0; i< (unsigned int) sim_points-1; i++)
		{
			w[i][1] = -v[i][2];
		}
		w[sim_points-1][1]=0.0;
	}
	a1 = new float* [sim_points];
	a2 = new float* [sim_points];
	b = new float* [sim_points];

// concentration has the current concentration for all scans

	concentration = new float* [run_inf.scans[selected_cell][selected_lambda]];
	left = new float* [sim_points];
	right = new float [sim_points];
	for (i=0; i<sim_points; i++)
	{
		a1[i] = new float [3];
		a2[i] = new float [3];
		b[i] = new float [3];
		left[i] = new float [3];
	}
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		concentration[i] = new float [sim_points];
	}
	a1[0][1] = (sim_radius[0]/run_vector->delta_r) + 0.5;
	a1[0][0] = 0.0;
	for (i=1; i<sim_points; i++)
	{
		a1[i][0] = (-(sim_radius)[i]/run_vector->delta_r) + 0.5;
		a1[i][1] = 2.0 * sim_radius[i]/run_vector->delta_r;
		a1[i-1][2] = a1[i][0];
	}
	a1[sim_points-1][1] = (sim_radius[sim_points-1]/run_vector->delta_r) - 0.5;
	a1[sim_points-1][2] = 0.0;
	float delta2 = square(run_vector->delta_r);
	a2[0][1] = -1 * (sim_radius[0] * sim_radius[0])/2.0 - sim_radius[0] * (run_vector->delta_r/3.0) - delta2/12.0;
	a2[0][0] = 0.0;
	for (i=1; i<sim_points; i++)
	{
		a2[i][0] = (sim_radius[i] * sim_radius[i])/2.0 - 2.0 * sim_radius[i] * (run_vector->delta_r/3.0) + delta2/4.0;
		a2[i][1] = -2 * sim_radius[i] * run_vector->delta_r/3.0;
		a2[i-1][2]= -1 * (sim_radius[i-1] * sim_radius[i-1])/2.0 - 2.0 * sim_radius[i-1] * (run_vector->delta_r/3.0) - delta2/4.0;
	}
	a2[sim_points-1][1] = (sim_radius[sim_points-1] * sim_radius[sim_points-1])/2.0 - sim_radius[sim_points-1] * (run_vector->delta_r/3.0) + delta2/12.0;
	a2[sim_points-1][2] = 0.0;
	b[0][0] = 0.0;
	b[0][1] = sim_radius[0] * (run_vector->delta_r/3.0) + delta2/12.0;
	for (i=1; i<sim_points; i++)
	{
		b[i][0] = sim_radius[i] * (run_vector->delta_r/6.0) - delta2/12.0;
		b[i][1] = 2.0 * sim_radius[i]*(run_vector->delta_r/3.0);
		b[i-1][2] = b[i][0];
	}
	b[sim_points-1][1] = sim_radius[sim_points-1] * (run_vector->delta_r/3.0) - delta2/12.0;
	b[sim_points-1][2] = 0.0;
}

void US_Finite_W_noGUI::cleanup_finite()
{
	unsigned int i;
	bool conc_dep_diff_flag = false;
	init_simulation = false;
	for (unsigned int i=0; i<components; i++)
	{
		if (component_vector[i].delta != 0)
			conc_dep_diff_flag = true;
	}

	delete [] sim_radius;
	if (model == 1 || model == 2)
	{
		delete [] s_bar;
		delete [] D_bar;
		delete [] partial_c[0];
		delete [] partial_c[1];
		delete [] partial_c;
	}

	if (conc_dep_diff_flag) //we got concentration dependency, de-allocate memory
	{ 
		delete [] cu; 
		delete [] cv;
		delete [] cw;
		for (i=0; i<sim_points; i++)
		{
			delete [] u[i];
			delete [] v[i];
			delete [] w[i];
		}
		delete [] u;
		delete [] v;
		delete [] w;
	}
	for (i=0; i<sim_points; i++)
	{
		delete [] a1[i];
		delete [] a2[i];
		delete [] b[i];
		delete [] left[i];
	}
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		delete [] concentration[i];
	}
	delete [] a1;
	delete [] a2;
	delete [] b;
	delete [] concentration;
	delete [] left;
	delete [] right;
}

void US_Finite_W_noGUI::order_variance()
{
	unsigned int i, j;
	for (i=0; i<parameters+1; i++)
	{
		old_variance[i] = variance[i];
	}
	for (j=0; j<parameters+1; j++)
	{
		float maxval=0.0;
		for (i=0; i<parameters+1; i++)
		{
			if (variance[i] > maxval)
			{
				maxval = variance[i];
				order[j] = i;
			}
		}
		if (j != parameters)
		{
			variance[order[j]] = 0.0;
// do not lose the best variance, since we need it for comparison for the first iteration!
		}
	}
	swap();
}

void US_Finite_W_noGUI::swap()
{// reorders theta, F and variance, so they are in the proper order as determined by order_variance()
	float **matrix;
	unsigned int i, j;
	
	matrix = new float* [parameters+1];
	for (i=0; i<parameters+1; i++)
	{
		matrix[i] = new float [parameters];
	}
	for (i=0; i<parameters+1; i++)
	{
		for (j=0; j<parameters; j++)
		{
			matrix[i][j] = theta[order[i]][j];
		}
	}
	for (i=0; i<parameters+1; i++)
	{
		for (j=0; j<parameters; j++)
		{
			theta[i][j] = matrix[i][j];
		}
	}
	for (i=0; i<parameters+1; i++)
	{
		delete [] matrix[i];
	}
	delete [] matrix;
	matrix = new float* [parameters+1];
	for (i=0; i<parameters+1; i++)
	{
		matrix[i] = new float [allpoints];
	}
	for (i=0; i<parameters+1; i++)
	{
		for (j=0; j<allpoints; j++)
		{
			matrix[i][j] = F[order[i]][j];
		}
	}
	for (i=0; i<parameters+1; i++)
	{
		for (j=0; j<allpoints; j++)
		{
			F[i][j] = matrix[i][j];
		}
	}
	for (i=0; i<parameters+1; i++)
	{
		variance[i] = old_variance[order[i]];
	}
	for (i=0; i<parameters+1; i++)
	{	//update old_variance so we have the correct values in the proper indices in the next iteration when we compare
		old_variance[i] = variance[i];
	}
	for (i=0; i<parameters+1; i++)
	{
		delete [] matrix[i];
	}
	delete [] matrix;
}

void US_Finite_W_noGUI::shift_all()
{
	unsigned int i,j;
	for (i=1; i<parameters+2; i++)
	{
		for (j=0; j<parameters; j++)
		{
			theta[i-1][j] = theta[i][j];
		}
	}
	for (i=1; i<parameters+2; i++)
	{
		for (j=0; j<allpoints; j++)
		{
			F[i-1][j] = F[i][j];
		}
	}
	for (i=1; i<parameters+2; i++)
	{
		variance[i-1] = variance[i];
	}
}

bool US_Finite_W_noGUI::calc_alpha()
{
	double **temp1_matrix, **temp2_matrix, *temp1_vector, *temp2_vector;
	unsigned int i, j, k, count;
	
	temp1_matrix = new double* [parameters];
	temp2_matrix = new double* [parameters];
	temp1_vector = new double  [allpoints];
	temp2_vector = new double  [parameters];
	for (i=0; i<parameters; i++)
	{
		temp1_matrix[i] = new double [parameters];
		temp2_matrix[i] = new double [parameters];
	}
	
	count = 0;
	for (j=0; j<run_inf.scans[selected_cell][selected_lambda]; j++)
	{
		for (k=0; k<points; k++)
		{
			temp1_vector[count] = absorbance[j][k] - F[parameters][count];	// y - f(p+1)
			count++;
		}
	}
	for (i=0; i<parameters; i++)
	{
		temp2_vector[i] = 0.0;
		count = 0;
		for (j=0; j<run_inf.scans[selected_cell][selected_lambda]; j++)
		{
			for (k=0; k<points; k++)
			{
				temp2_vector[i] += delta_F[i][count] * temp1_vector[count];
				count++;
			}
		}
		
// calculate delta_F(transpose) * delta_F, hold temporary result in temp1_matrix:

		for (j=0; j<parameters; j++)
		{
			temp1_matrix[i][j] = 0.0;
			for (k=0; k<allpoints; k++)
			{
				temp1_matrix[i][j] += delta_F[i][k] * delta_F[j][k];
			}
		}
	}

// invert delta_F(transpose) * delta_F using Cholesky Decomposition, hold inverse in temp2_matrix:
// if the cholesky decomposition fails, we need to start over with the last best estimate for theta

	if (!Cholesky_Invert(temp1_matrix, temp2_matrix, parameters))
	{
		return(false);
	}

// multiply the inverse matrix by the temp_vector to obtain alpha:

	for (i=0; i<parameters; i++)
	{
		alpha[i] = 0.0;
		for (j=0; j<parameters; j++)
		{
			alpha[i]	+= temp2_matrix[i][j] * temp2_vector[j];
		}
	}
	for (i=0; i<parameters; i++)
	{
		delete [] temp1_matrix[i];
		delete [] temp2_matrix[i];
	}
	delete [] temp1_matrix;
	delete [] temp2_matrix;
	delete [] temp1_vector;
	delete [] temp2_vector;
	return(true);
}

void US_Finite_W_noGUI::calc_deltas(const unsigned int i)
{
	unsigned int j;
	for (j=0; j<parameters; j++)
	{
		delta_theta[i][j] = theta[i][j] - theta[parameters][j];
	}

	for (j=0; j<allpoints; j++)
	{
		delta_F[i][j] = F[i][j] - F[parameters][j];
//cout << delta_F[i][j] << "\n";
	}
}

void US_Finite_W_noGUI::calc_residuals(const unsigned int i)
{

// if only a portion of the boundary is used to calculate residuals then the 

	unsigned int j, k, count=0, subcount=0;
	float start_y, stop_y;
	variance[i] = 0.0;
	if (bd_range == 100)
	{
		for (j=0; j<run_inf.scans[selected_cell][selected_lambda]; j++)
		{
			for (k=0; k<points; k++)
			{
				variance[i] += pow((F[i][count] - absorbance[j][k]), 2);
				count++;
			}
		}
		variance[i] = variance[i]/(count - parameters);
	}
	else
	{
		for (j=0; j<run_inf.scans[selected_cell][selected_lambda]; j++)
		{
			start_y = run_inf.plateau[selected_cell][selected_lambda][j] * bd_position / 100;
			stop_y  = start_y + run_inf.plateau[selected_cell][selected_lambda][j] * bd_range/100;
			for (k=0; k<points; k++)
			{
				if((absorbance[j][k] >= start_y) && (absorbance[j][k] <= stop_y))
				{
					variance[i] += pow((F[i][count] - absorbance[j][k]), 2);
					subcount++;
				}
				count++;
			}
		}
		variance[i] = variance[i]/(subcount - parameters);
	}
}

void US_Finite_W_noGUI::update_simulation_parameters_constrained(const unsigned int i)
{
//
// if any parameters are floated, the resepective component_vector member will be reassigned to 
// reflect the value stored in theta. If it isn't floated, the current value is kept unchanged.
// the parameter_count is incremented each time a floating parameter is encountered
//
	unsigned int k, parameter_count = 0;
	if (run_vector->meniscus_fit)
	{
		run_vector->meniscus = theta[i][parameter_count];
		if (run_vector->meniscus < (theta[parameters+2][parameter_count] - run_vector->meniscus_range))
		{
			run_vector->meniscus = theta[parameters+2][parameter_count] - run_vector->meniscus_range;
			theta[i][parameter_count] = run_vector->meniscus;
		}
		if (run_vector->meniscus > (theta[parameters+2][parameter_count] + run_vector->meniscus_range))
		{
			run_vector->meniscus = theta[parameters+2][parameter_count] + run_vector->meniscus_range;
			theta[i][parameter_count] = run_vector->meniscus;
		}
		parameter_count++;
	}
	if (run_vector->baseline_fit)
	{
		run_vector->baseline = theta[i][parameter_count];
		if (run_vector->baseline < (theta[parameters+2][parameter_count] - run_vector->baseline_range))
		{
			run_vector->baseline = theta[parameters+2][parameter_count] - run_vector->baseline_range;
			theta[i][parameter_count] = run_vector->baseline;
		}
		if (run_vector->baseline > (theta[parameters+2][parameter_count] + run_vector->baseline_range))
		{
			run_vector->baseline = theta[parameters+2][parameter_count] + run_vector->baseline_range;
			theta[i][parameter_count] = run_vector->baseline;
		}
		parameter_count++;
	}
	if (run_vector->slope_fit)
	{
		run_vector->slope = theta[i][parameter_count];
		if (run_vector->slope < (theta[parameters+2][parameter_count] - run_vector->slope_range))
		{
			run_vector->slope = theta[parameters+2][parameter_count] - run_vector->slope_range;
			theta[i][parameter_count] = run_vector->slope;
		}
		if (run_vector->slope > (theta[parameters+2][parameter_count] + run_vector->slope_range))
		{
			run_vector->slope = theta[parameters+2][parameter_count] + run_vector->slope_range;
			theta[i][parameter_count] = run_vector->slope;
		}
		parameter_count++;
	}
	if (run_vector->stray_fit)
	{
		run_vector->stray = theta[i][parameter_count];
		if (run_vector->stray < (theta[parameters+2][parameter_count] - run_vector->stray_range))
		{
			run_vector->stray = theta[parameters+2][parameter_count] - run_vector->stray_range;
			theta[i][parameter_count] = run_vector->stray;
		}
		if (run_vector->stray > (theta[parameters+2][parameter_count] + run_vector->stray_range))
		{
			run_vector->stray = theta[parameters+2][parameter_count] + run_vector->stray_range;
			theta[i][parameter_count] = run_vector->stray;
		}
		parameter_count++;
	}
	for (k=0; k<components; k++)
	{
		if (component_vector[k].sed_fit)
		{
			component_vector[k].sed = theta[i][parameter_count];
			if (component_vector[k].sed < (theta[parameters+2][parameter_count] - component_vector[k].sed_range))
			{
				component_vector[k].sed = theta[parameters+2][parameter_count] - component_vector[k].sed_range;
				theta[i][parameter_count] = component_vector[k].sed;
			}
			if (component_vector[k].sed > (theta[parameters+2][parameter_count] + component_vector[k].sed_range))
			{
				component_vector[k].sed = theta[parameters+2][parameter_count] + component_vector[k].sed_range;
				theta[i][parameter_count] = component_vector[k].sed;
			}
			parameter_count++;
		}
		if (component_vector[k].diff_fit)
		{
			component_vector[k].diff = theta[i][parameter_count];
			if (component_vector[k].diff < (theta[parameters+2][parameter_count] - component_vector[k].diff_range))
			{
				component_vector[k].diff = theta[parameters+2][parameter_count] - component_vector[k].diff_range;
				theta[i][parameter_count] = component_vector[k].diff;
			}
			if (component_vector[k].diff > (theta[parameters+2][parameter_count] + component_vector[k].diff_range))
			{
				component_vector[k].diff = theta[parameters+2][parameter_count] + component_vector[k].diff_range;
				theta[i][parameter_count] = component_vector[k].diff;
			}
			parameter_count++;
		}
		if (component_vector[k].conc_fit)
		{
			component_vector[k].conc = theta[i][parameter_count];
			if (component_vector[k].conc < (theta[parameters+2][parameter_count] - component_vector[k].conc_range))
			{
				component_vector[k].conc = theta[parameters+2][parameter_count] - component_vector[k].conc_range;
				theta[i][parameter_count] = component_vector[k].conc;
			}
			if (component_vector[k].conc > (theta[parameters+2][parameter_count] + component_vector[k].conc_range))
			{
				component_vector[k].conc = theta[parameters+2][parameter_count] + component_vector[k].conc_range;
				theta[i][parameter_count] = component_vector[k].conc;
			}
			parameter_count++;
		}
		if (component_vector[k].sigma_fit)
		{
			component_vector[k].sigma = theta[i][parameter_count];
			if (component_vector[k].sigma < (theta[parameters+2][parameter_count] - component_vector[k].sigma_range))
			{
				component_vector[k].sigma = theta[parameters+2][parameter_count] - component_vector[k].sigma_range;
				theta[i][parameter_count] = component_vector[k].sigma;
			}
			if (component_vector[k].sigma > (theta[parameters+2][parameter_count] + component_vector[k].sigma_range))
			{
				component_vector[k].sigma = theta[parameters+2][parameter_count] + component_vector[k].sigma_range;
				theta[i][parameter_count] = component_vector[k].sigma;
			}
			parameter_count++;
		}
		if (component_vector[k].delta_fit)
		{
			component_vector[k].delta = theta[i][parameter_count];
			if (component_vector[k].delta < (theta[parameters+2][parameter_count] - component_vector[k].delta_range))
			{
				component_vector[k].delta = theta[parameters+2][parameter_count] - component_vector[k].delta_range;
				theta[i][parameter_count] = component_vector[k].delta;
			}
			if (component_vector[k].delta > (theta[parameters+2][parameter_count] + component_vector[k].delta_range))
			{
				component_vector[k].delta = theta[parameters+2][parameter_count] + component_vector[k].delta_range;
				theta[i][parameter_count] = component_vector[k].delta;
			}
			parameter_count++;
		}
	}
}

void US_Finite_W_noGUI::update_simulation_parameters_unconstrained(const unsigned int i)
{
//
// if any parameters are floated, the respective component_vector member will be reassigned to 
// reflect the value stored in theta. If it isn't floated, the current value is kept unchanged.
// the parameter_count is incremented each time a floating parameter is encountered
//
	unsigned int k, parameter_count = 0;
	if (run_vector->meniscus_fit)
	{
		run_vector->meniscus = theta[i][parameter_count];
		if (run_vector->meniscus - run_inf.meniscus[selected_cell] < -0.3)
		{
			run_vector->meniscus = run_inf.meniscus[selected_cell] - 0.3;
			theta[i][parameter_count]  = run_vector->meniscus;
		}
		if (run_vector->meniscus - run_inf.meniscus[selected_cell] > 0.3)
		{
			run_vector->meniscus = run_inf.meniscus[selected_cell] + 0.3;
			theta[i][parameter_count]  = run_vector->meniscus;
		}
		parameter_count++;
	}
	if (run_vector->baseline_fit)
	{
		run_vector->baseline = theta[i][parameter_count];
		parameter_count++;
	}
	if (run_vector->slope_fit)
	{
		run_vector->slope = theta[i][parameter_count];
		parameter_count++;
	}
	if (run_vector->stray_fit)
	{
		run_vector->stray = theta[i][parameter_count];
		parameter_count++;
	}
	for (k=0; k<components; k++)
	{
		if (component_vector[k].sed_fit)
		{
			component_vector[k].sed = theta[i][parameter_count];
			if (component_vector[k].sed < 1.0e-15)
			{
				component_vector[k].sed = 1.0e-15;
				theta[i][parameter_count] = 1.0e-15;
			}
			parameter_count++;
		}
		if (component_vector[k].diff_fit)
		{
			component_vector[k].diff = theta[i][parameter_count];
			if (component_vector[k].diff < 5.0e-10)
			{
				component_vector[k].diff = 5.0e-10;
				theta[i][parameter_count] = 5.0e-10;
			}
			parameter_count++;
		}
		if (component_vector[k].conc_fit)
		{
			component_vector[k].conc = theta[i][parameter_count];
			if (component_vector[k].conc < 5.0e-3)
			{
				component_vector[k].conc = 5.0e-3;
				theta[i][parameter_count] = 5.0e-3;
			}
			parameter_count++;
		}
		if (component_vector[k].sigma_fit)
		{
			component_vector[k].sigma = theta[i][parameter_count];
			if (component_vector[k].sigma < 0.0)
			{
				component_vector[k].sigma = 0.0;
				theta[i][parameter_count] = 0.0;
			}
			parameter_count++;
		}
		if (component_vector[k].delta_fit)
		{
			component_vector[k].delta = theta[i][parameter_count];
			if (component_vector[k].delta < 0.0)
			{
				component_vector[k].delta = 0.0;
				theta[i][parameter_count] = 0.0;
			}
			parameter_count++;
		}
	}
}
