#include "../include/us_astfem_rsa.h"

void AstFemParameters::print_af(FILE *outf)
{
   unsigned int i;

	fprintf(outf, "#####################################################\n");
	fprintf(outf, "#  \n");
	fprintf(outf, "#  Model Number: %d \n", model);
	fprintf(outf, "#  Number of species = %d\n", s.size() );
	for (i=0; i<s.size(); i++)
	{
	   fprintf(outf, "#  s[%d]=%20.12e D[%d]=%20.12e \n", i, s[i], i, D[i]);
	}
	fprintf(outf, "#  \n");
	fprintf(outf, "#  parameters for reactions:\n");
	for (i=0; i<keq.size(); i++)
	{
	   fprintf(outf, "#  keq[%d]=%12.5e \t",  i, keq[i]);
	}
	fprintf(outf, "\n");
	for (i=0; i<koff.size(); i++)
	{
	   fprintf(outf, "#  koff[%d]=%12.5e \t", i, koff[i]);
	}
	fprintf(outf, "\n");
	for (i=0; i<n.size(); i++)
	{
	   fprintf(outf, "#  exponent[%d]=%d \n", i, n[i]);
	}
	fprintf(outf, "#  \n");
	fprintf(outf, "#  parameters for simulation:\n");
   fprintf(outf, "#  meniscus =%12.5e \n",  meniscus);
   fprintf(outf, "#  bottom =%12.5e \n",  bottom);
   fprintf(outf, "#  start time =%12.5e \n",  start_time);
   fprintf(outf, "#  mesh opt =%d \n",  mesh);
   if (moving_grid) fprintf(outf, "#  grids = moving \n");
   else fprintf(outf, "#  grids = fixed \n");
   if (acceleration) fprintf(outf, "#  acceleration = True \n");
   else fprintf(outf, "#  acceleration = False \n");
   fprintf(outf, "#  simpoints =%d \n",  simpoints);
   fprintf(outf, "#  dt =%12.5e \n",  dt);
   fprintf(outf, "#  Total Number of Steps =%d \n",  time_steps);

	fprintf(outf, "#  \n");
	fprintf(outf, "#####################################################\n");

   return;
}


US_Astfem_RSA::US_Astfem_RSA(bool *stopFlag, bool guiFlag, bool *movieFlag, QObject *parent, const char *name) : QObject(parent, name)
{
	this->stopFlag = stopFlag;
	this->guiFlag = guiFlag;
	this->movieFlag = movieFlag;
}

US_Astfem_RSA::~US_Astfem_RSA()
{
}

int US_Astfem_RSA::calculate(struct ModelSystem *system, struct SimulationParameters *simparams,
vector <struct mfem_data> *exp_data)
{
	cout << "Meniscus: " << (*simparams).meniscus << endl;
	cout << "Bottom: " << (*simparams).bottom << endl;
	unsigned int i, j;
	float current_time = 0.0;
	double current_speed;
	double s_max_abs;			// largest sedimenting or floating speed (absolute value)
	unsigned int duration, delay;
	mfem_data simdata;
	mfem_initial CT0;			// initial total concentration
	af_params.model = (*system).model;
	if (af_params.model < 4) // non-interacting single or multicomponent systems
	{
		for (i=0; i<(*system).component_vector.size(); i++)
		{
			if (guiFlag)
			{
				emit current_component((int) i+1);
				qApp->processEvents();
			}
			current_time = 0.0; // reset time, which now tracks the beginning of each speed step (duration)
			current_speed = 0.0; // start at rest
			CT0.radius.clear();
			CT0.concentration.clear();
			double dr = ((*simparams).bottom - (*simparams).meniscus)/((*simparams).simpoints - 1);
			for (j=0; j<(*simparams).simpoints; j++)
			{
				CT0.radius.push_back((*simparams).meniscus + j * dr );
				CT0.concentration.push_back(0.0);
			}
			if ((*system).component_vector[i].c0.concentration.size() == 0) // we don't have an existing CT0 concentration vector
			{ // build up the initial concentration vector with constant concentration
				if ((*simparams).band_forming)
				{
					cout << "Band Forming centerpiece selected\n";
//					CT0.concentration.push_back(exp(-pow((CT0.radius[j] - (*simparams).meniscus)/0.05, 2.0)));
					j = 0;
// find the width of the band and fill those concentration points with initial concentration:
					while (CT0.radius[j] < pow((*simparams).meniscus*(*simparams).meniscus
							 + (*simparams).band_volume * 360.0/(2.5 * 1.2 * M_PI), 0.5))
					{
						CT0.concentration[j] += (*system).component_vector[i].concentration;
						j++;
					}
				}
				else
				{
					for (j=0; j<(*simparams).simpoints; j++)
					{
						CT0.concentration[j] += (*system).component_vector[i].concentration;
					}
				}
			}
			else
			{
				// take the existing initial concentration vector and copy it to the temporary CT0 vector:
				CT0.radius.clear();
				CT0.concentration.clear();
				CT0 = (*system).component_vector[i].c0;
			}
			af_params.s.clear();
			af_params.D.clear();
			af_params.s.push_back((double) (*system).component_vector[i].s);
			af_params.D.push_back((double) (*system).component_vector[i].D);
			for (j=0; j<(*simparams).speed_step.size(); j++)
			{
				//fabs( (*simparams).speed_step[j].rotorspeed - current_speed ) > (*simparams).speed_step[j].acceleration)
				if((*simparams).speed_step[j].acceleration_flag) // we need to simulate acceleration
				{// if the speed difference is larger than acceleration rate then we have at least 1 acceleration step
					af_params.time_steps = (unsigned int) fabs((*simparams).speed_step[j].rotorspeed
					- current_speed)/(*simparams).speed_step[j].acceleration;
					af_params.dt = 1.0; // each simulation step is 1 second long in the acceleration phase
					af_params.simpoints = 2 * (*simparams).simpoints; // use a fixed grid with refinement at both ends and with twice the number of points
					af_params.start_time = current_time;
					af_params.meniscus = (*simparams).meniscus;
					af_params.bottom = (*simparams).bottom;
					af_params.mesh = (*simparams).mesh;
					af_params.moving_grid = false;
					af_params.acceleration = true;
					vector <double> rpm;
					rpm.clear();
					for (unsigned int step=0; step<af_params.time_steps; step++)
					{
						rpm.push_back(current_speed + (step + 1) * (*simparams).speed_step[j].acceleration);
					}

					// on exit, contains final concentration in CT0
					calculate_ni(rpm[0], rpm[rpm.size()-1], af_params.s[0], af_params.D[0], &CT0, &simdata);

					// add the acceleration time:
					current_time += af_params.dt * af_params.time_steps;
					if (guiFlag)
					{
						emit new_time(current_time);
						qApp->processEvents();
					}
					if (*stopFlag)
					{
						qApp->processEvents();
						interpolate(&(*exp_data)[j], &simdata); // interpolate the simulated data onto the experimental time- and radius grid
						return(1); // early termination = 1
					}
				}  // end of for acceleration
				duration = ((*simparams).speed_step[j].duration_hours * 3600
				+ (*simparams).speed_step[j].duration_minutes * 60);
				delay = ((*simparams).speed_step[j].delay_hours * 3600
				+ (*simparams).speed_step[j].delay_minutes * 60);
				af_params.omega_s = pow((*simparams).speed_step[j].rotorspeed * M_PI/30.0, 2.0);
// find out the minimum dt between actual scans
				af_params.dt = (duration - delay)/(*simparams).speed_step[j].scans;
// find out the minimum number of simpoints needed to provide the necessary dt:
				af_params.simpoints = 1 + (unsigned int) (log((*exp_data)[j].bottom/(*exp_data)[j].meniscus)
				/(af_params.omega_s * fabs((*system).component_vector[i].s) * af_params.dt));
// if calculated # of simpoints is smaller than user selected number of simpoints, follow user's request:
				if (af_params.simpoints < (*simparams).simpoints)
				{
					af_params.simpoints = (*simparams).simpoints;
					af_params.dt = log((*exp_data)[j].bottom/(*exp_data)[j].meniscus)
					/(af_params.omega_s * fabs((*system).component_vector[i].s) * ((*simparams).simpoints - 1)); // delta_t
				}
				else
				{
					cout << "Number of simpoints adjusted to " << af_params.simpoints
					<< " for component " << i + 1 << " and speed step " << j + 1 << endl;
				}
				af_params.time_steps = 1 + (unsigned int) (duration/af_params.dt);
cout << "speed step:\t" << j << ", component: " << i << endl;
cout << "speed:\t\t" << (*simparams).speed_step[j].rotorspeed << endl;
cout << "hours:\t\t" << (*simparams).speed_step[j].duration_hours << endl;
cout << "minutes:\t" << (*simparams).speed_step[j].duration_minutes << endl;
				af_params.start_time = current_time;
				af_params.meniscus = (*exp_data)[j].meniscus;
				af_params.bottom = (*exp_data)[j].bottom;
				af_params.mesh = (*simparams).mesh;
				af_params.moving_grid = (*simparams).moving_grid;
				af_params.acceleration = false;
				print_af();
				vector <double> rpm;
				rpm.clear();
				rpm.push_back((*simparams).speed_step[j].rotorspeed);
				calculate_ni(rpm[0], rpm[0], af_params.s[0], af_params.D[0], &CT0, &simdata);
cout << "simdata scan size: " << simdata.scan.size() << endl;
cout << "expdata scan size: " << (*exp_data)[j].scan.size() << endl;
cout << endl;
				interpolate(&(*exp_data)[j], &simdata); // interpolate the simulated data onto the experimental time- and radius grid

				// set the current time to the last scan of this speed step
				current_time = (*simparams).speed_step[j].duration_hours * 3600
				+ (*simparams).speed_step[j].duration_minutes * 60;

				// set the current speed to the constant rotor speed of the current speed step
				current_speed = (*simparams).speed_step[j].rotorspeed;
				qApp->processEvents();
				if (*stopFlag)
				{
					return(1); // early termination = 1
				}
			} // speed step loop
			if (guiFlag)
			{
				emit current_component(i+1);
				cout << "Current component: " << i+1 << endl;
				qApp->processEvents();
			}
		} // component loop
	}  // end of non-interacting case
	else if (af_params.model >= 4 && af_params.model <= 10) // A + nA = An
	{
		current_time = 0.0; // reset time, which now tracks the beginning of each speed step (duration)
		current_speed = 0.0; // start at rest
		CT0.radius.clear();
		CT0.concentration.clear();

		if ((*system).component_vector[0].c0.concentration.size() == 0
		&&  (*system).component_vector[1].c0.concentration.size() == 0) // we don't have an existing CT0 concentration vector
		{ // build up the initial concentration vector with constant concentration from the two components
			double dr = ((*simparams).bottom - (*simparams).meniscus)/((*simparams).simpoints - 1);
			for (j=0; j<(*simparams).simpoints; j++)
			{
				CT0.radius.push_back((*simparams).meniscus + j * dr );
				CT0.concentration.push_back((*system).component_vector[0].concentration + (*system).component_vector[1].concentration);
			}
		}
		else
		if ((*system).component_vector[0].c0.concentration.size() > 0
		&&  (*system).component_vector[1].c0.concentration.size() == 0) // we only have an existing CT0 concentration vector for component 1
		{
// take the existing initial concentration vector and copy it to the temporary CT0 vector:
			CT0 = (*system).component_vector[0].c0;
		}
		else
		if ((*system).component_vector[0].c0.concentration.size() == 0
		&&  (*system).component_vector[1].c0.concentration.size() > 0) // we only have an existing CT0 concentration vector for component 1
		{
// take the existing initial concentration vector and copy it to the temporary CT0 vector:
			CT0 = (*system).component_vector[1].c0;
		}

/**********************/

		af_params.s.clear();
		af_params.D.clear();
		af_params.keq.clear();
		af_params.koff.clear();
		af_params.n.clear();
		af_params.keq.push_back((double) (*system).assoc_vector[0].keq);
		af_params.koff.push_back((double) (*system).assoc_vector[0].k_off);
		af_params.n.push_back((*system).assoc_vector[0].stoichiometry1);
		af_params.n.push_back((*system).assoc_vector[0].stoichiometry2);
		for (i=0; i<2; i++)
		{
			af_params.s.push_back((double) (*system).component_vector[i].s);
			af_params.D.push_back((double) (*system).component_vector[i].D);
		}

      // find the largest sedimenting or floating speed
      s_max_abs = fabs(maxval(af_params.s)) > fabs(minval(af_params.s))?
                  fabs(maxval(af_params.s)) : fabs(minval(af_params.s));


      // decompose the initial total concentration CT0 into partial concentration C0
	   mfem_initial *C0;		// inital partial concentration decomposed from initial total concentration CT0
      C0 = new mfem_initial [ (*system).component_vector.size() ];

      for( i=0; i<(*system).component_vector.size(); i++)
      {
         C0[i].radius.clear();
         C0[i].concentration.clear();
      }
      double *vtmp ;
      vtmp = new double [ (*system).component_vector.size() ];
	   for (j=0; j< CT0.radius.size(); j++)
      {
         DecomposeCT( CT0.concentration[j], vtmp);
         for( i=0; i<(*system).component_vector.size(); i++)
         {
            C0[i].radius.push_back( CT0.radius[j] );
            C0[i].concentration.push_back( vtmp[i] );
         }
      }
      delete [] vtmp;


      // start the simulation
		for (j=0; j<(*simparams).speed_step.size(); j++)
		{
			if (guiFlag)
			{
				emit current_component(-1);
				qApp->processEvents();
			}
			if((*simparams).speed_step[j].acceleration_flag) // we need to simulate acceleration
			{// if the speed difference is larger than acceleration rate then we have at least 1 acceleration step
				af_params.time_steps = (unsigned int) fabs((*simparams).speed_step[j].rotorspeed
				- current_speed)/(*simparams).speed_step[j].acceleration;
				af_params.dt = 1.0; // each simulation step is 1 second long in the acceleration phase
				af_params.simpoints = 2 * (*simparams).simpoints; // use a fixed grid with refinement at both ends and with twice the number of points
				af_params.start_time = current_time;
				af_params.meniscus = (*simparams).meniscus;
				af_params.bottom = (*simparams).bottom;
				af_params.mesh = (*simparams).mesh;
				af_params.moving_grid = false;
				af_params.acceleration = true;
				vector <double> rpm;
				rpm.clear();
				for (unsigned int step=0; step<af_params.time_steps; step++)
				{
					rpm.push_back(current_speed + (step + 1) * (*simparams).speed_step[j].acceleration);
				}
// on exit, contains final concentration in C0
				calculate_ra2(rpm[0], rpm[rpm.size()-1], C0, &simdata);
// add the acceleration time:
				current_time += af_params.dt * af_params.time_steps;
				if (guiFlag)
				{
					emit new_time(current_time);
					qApp->processEvents();
				}
				if (*stopFlag)
				{
					qApp->processEvents();
					interpolate(&(*exp_data)[j], &simdata); // interpolate the simulated data onto the experimental time- and radius grid
					return(1); // early termination = 1
				}
			}  // end of for acceleration
			duration = ((*simparams).speed_step[j].duration_hours * 3600
					+ (*simparams).speed_step[j].duration_minutes * 60);
			delay = ((*simparams).speed_step[j].delay_hours * 3600
					+ (*simparams).speed_step[j].delay_minutes * 60);
			af_params.omega_s = pow((*simparams).speed_step[j].rotorspeed * M_PI/30.0, 2.0);
// find out the minimum dt between actual scans
			af_params.dt = (duration - delay)/(*simparams).speed_step[j].scans;
// find out the minimum number of simpoints needed to provide the necessary dt:

			af_params.simpoints = 1 + (unsigned int) (log((*exp_data)[j].bottom/(*exp_data)[j].meniscus)/( af_params.omega_s * s_max_abs * af_params.dt));


// if calculated # of simpoints is smaller than user selected number of simpoints, follow user's request:
			if ( af_params.simpoints < (*simparams).simpoints )
			{
				af_params.simpoints = (*simparams).simpoints;
				af_params.dt = log((*exp_data)[j].bottom/(*exp_data)[j].meniscus)
				    /(af_params.omega_s * s_max_abs * ((*simparams).simpoints - 1)); // delta_t
			}
			else
			{
				cout << "Number of simpoints adjusted to " << af_params.simpoints
						<< " for component " << i + 1 << " and speed step " << j + 1 << endl;
			}
			af_params.time_steps = 1 + (unsigned int) (duration/af_params.dt);
			af_params.start_time = current_time;
			af_params.meniscus = (*exp_data)[j].meniscus;
			af_params.bottom = (*exp_data)[j].bottom;
			af_params.mesh = (*simparams).mesh;
			af_params.moving_grid = (*simparams).moving_grid;
			af_params.acceleration = false;
			print_af();
			vector <double> rpm;
			rpm.clear();
			rpm.push_back((*simparams).speed_step[j].rotorspeed);
			calculate_ra2(rpm[0], rpm[0], C0, &simdata);
			interpolate(&(*exp_data)[j], &simdata); // interpolate the simulated data onto the experimental time- and radius grid

				// set the current time to the last scan of this speed step
			current_time = (*simparams).speed_step[j].duration_hours * 3600
					+ (*simparams).speed_step[j].duration_minutes * 60;

				// set the current speed to the constant rotor speed of the current speed step
			current_speed = (*simparams).speed_step[j].rotorspeed;
			qApp->processEvents();
			if (*stopFlag)
			{
				return(1); // early termination = 1
			}
		} // speed step loop

      delete [] C0;

	}
	else
	{
		cout << "This model is not yet supported..." << endl;
	}
	return(0);
}

void US_Astfem_RSA::interpolate_C0(struct mfem_initial *C0, double *C1)
{
	unsigned int i, j, ja;
	double a, b, xs, tmp;

	ja = 0;
	for(j=0; j<N; j++)
	{
		xs = x[j];
		for(i=ja; i<(*C0).radius.size(); i++)
		{
			if((*C0).radius[i] > xs + 1.e-12)
			{
				break;
			}
		}
		a = (*C0).radius[i-1];
		b = (*C0).radius[i];
		tmp = (xs - a)/(b - a);
		C1[j] = (*C0).concentration[i-1] * (1. - tmp) + (*C0).concentration[i] * tmp;
		ja = i-1;
	}
	C1[N-1] = (*C0).concentration[(*C0).radius.size() - 1];
}

void US_Astfem_RSA::interpolate_Cfinal(struct mfem_initial *C0, double *cfinal)
{
// this routine also considers cases where C0 starts before the meniscus or stops
// after the bottom is reached, however, C0 needs to cover both meniscus and bottom
// In those cases it will fill the C0 vector with the first and/or last value of C0,
// respectively, for the missing points.

	unsigned int i, j, ja;
	double a, b, xs, tmp;

	ja = 0;
	for(j=0; j<(*C0).radius.size(); j++)
	{
		xs = (*C0).radius[j];
		for (i=ja; i<N; i++)
		{
			if( x[i] > xs + 1.e-12)
			{
				break;
			}
		}
		a = x[i-1];
		b = x[i];
		tmp = (xs-a)/(b-a);
		(*C0).concentration[j] = cfinal[i-1] * (1. - tmp) + cfinal[i] * tmp;
		ja = i-1;
	}
}

void US_Astfem_RSA::print_af()
{
	cout << "Model Number:\t" << af_params.model << endl;
	cout << "Simpoints:\t" << af_params.simpoints << endl;
	for (unsigned int i=0; i< af_params.s.size(); i++)
	{
		cout << "s[" << i << "]:\t\t" << af_params.s[i] << endl;
		cout << "D[" << i << "]:\t\t" << af_params.D[i] << endl;
	}
	cout << "dt:\t\t" << af_params.dt << endl;
	cout << "time_steps:\t" << af_params.time_steps << endl;
	cout << "omega_s:\t" << af_params.omega_s << endl;
	cout << "start_time:\t" << af_params.start_time << endl;
	cout << "meniscus:\t" << af_params.meniscus << endl;
	cout << "bottom:\t\t" << af_params.bottom << endl;
	cout << "mesh:\t\t" << af_params.mesh << endl;
	cout << "moving grid\t\t" << af_params.moving_grid << endl;
}


int US_Astfem_RSA::calculate_ni(double rpm_start, double rpm_stop, double s, double D, mfem_initial *C_init, mfem_data *simdata) // non-interacting solute, constant speed
{
	unsigned int i, j;
	double sw2, rpm_current;
	double **CA;				// stiffness matrix on left hand side
									// CA[0...Ms-1][0...N-1][4]
	double **CB;				// stiffness matrix on right hand side
									// CB[0...Ms-1][0...N-1][4]
	double *C0, *C1;			// C[m][j]: current/next concentration of m-th component at x_j
									// C[0...Ms-1][0....N-1]:
	double **CA1, **CA2, **CB1, **CB2;		// for matrices used in acceleration

	CA = NULL;
	CB = NULL;
	C0 = NULL;
	C1 = NULL;
	(*simdata).radius.clear();
	(*simdata).scan.clear();
	mfem_scan simscan;

//
// generate the adaptive mesh
//

	sw2 = s * pow(rpm_stop * M_PI/30.0, 2.0);
	vector <double> nu;
	nu.clear();
	nu.push_back(sw2/D);
	mesh_gen(nu, af_params.mesh);
	if (af_params.acceleration)  		// refine left hand side (when s>0) or
	{										// right hand side (when s<0) for acceleration
		double xc ;
		if (s > 0)
		{ // radial distance from meniscus how far the boundary will move during this acceleration step (without diffusion)
			xc = af_params.meniscus + sw2 * (af_params.time_steps * af_params.dt) /3.;
			for (j=0; j<N-3; j++)
			{
				if (x[j] > xc)
				{
					break;
				}
			}
		}
		else
		{
			xc = af_params.bottom + sw2 * (af_params.time_steps * af_params.dt) /3.;
			for (j=0; j<N-3; j++)
			{
				if (x[N-j-1] < xc)
				{
					break;
				}
			}
		}
		mesh_gen_RefL(j+1, 4*j);
	}

	for (i=0; i<N; i++)
	{
		(*simdata).radius.push_back(x[i]);
		cout << "Mesh radius [" << i+1 << "]: " << x[i] << endl;
	}

//
// initialize the coefficient matrices
//
	initialize_2d(3, N, &CA);
	initialize_2d(3, N, &CB);

	if(!af_params.acceleration) // no acceleration
	{
		sw2 = s * pow(rpm_stop * M_PI/30.0, 2.0);
		if (!af_params.moving_grid)
		{
			ComputeCoefMatrixFixedMesh(D, sw2, CA, CB);
		}
		else
		{
			if (s > 0)
			{
				ComputeCoefMatrixMovingMeshR(D, sw2, CA, CB);
			}
			else
			{
				ComputeCoefMatrixMovingMeshL(D, sw2, CA, CB);
			}
		}
	}
	else				// for acceleration
	{
		initialize_2d(3, N, &CA1);
		initialize_2d(3, N, &CB1);
		initialize_2d(3, N, &CA2);
		initialize_2d(3, N, &CB2);

		if (!af_params.moving_grid)
		{
			sw2 = 0.;
			ComputeCoefMatrixFixedMesh(D, sw2, CA1, CB1);
			sw2 = s * pow(rpm_stop * M_PI/30.0, 2.0);
			ComputeCoefMatrixFixedMesh(D, sw2, CA2, CB2);
		}
		else
		{
			if (s > 0)
			{
				sw2 = 0.;
				ComputeCoefMatrixMovingMeshR(D, sw2, CA1, CB1);
				sw2 = s * pow(rpm_stop * M_PI/30.0, 2.0);
				ComputeCoefMatrixMovingMeshR(D, sw2, CA2, CB2);
			}
			else
			{
				sw2 = 0.;
				ComputeCoefMatrixMovingMeshL(D, sw2, CA1, CB1);
				sw2 = s * pow(rpm_stop * M_PI/30.0, 2.0);
				ComputeCoefMatrixMovingMeshL(D, sw2, CA2, CB2);
			}
		}
	}

//
// Initial condition
//
	C0 = new double [N];
	C1 = new double [N];
	interpolate_C0(C_init, C0); //interpolate the given C_init vector on the new C0 grid

	FILE *outf = fopen("tmp.out", "w");
   af_params.print_af(outf);

//
// time evolution
//
	double *right_hand_side;
	right_hand_side = new double [N];
	print_af();
	for (i=0; i<af_params.time_steps; i++) // calculate all time steps f
	{
		rpm_current = rpm_start + (rpm_stop - rpm_start) * (i+0.5)/af_params.time_steps;
		emit current_speed((unsigned int) rpm_current);


		if(af_params.acceleration) // then we have acceleration
		{
			for(unsigned int j1=0; j1<3; j1++)
			{
				for(unsigned int j2=0; j2<N; j2++)
				{
					CA[j1][j2] = CA1[j1][j2] + pow(rpm_current/rpm_stop, 2.0) * (CA2[j1][j2] - CA1[j1][j2]);
					CB[j1][j2] = CB1[j1][j2] + pow(rpm_current/rpm_stop, 2.0) * (CB2[j1][j2] - CB1[j1][j2]);
				}
			}
		}
		simscan.time = af_params.start_time + i * af_params.dt;
		simscan.omega_s_t = simscan.time * pow(rpm_current * M_PI/30.0, 2.0);

/**** delete to see if it the leak of mem
		if (guiFlag)
		{
			if(*movieFlag)
			{
				emit new_scan(&x, C0);
				emit new_time((float) simscan.time);
				qApp->processEvents();
			}
		}
*****/
		simscan.conc.clear();
		for (j=0; j<N; j++)
		{
			simscan.conc.push_back(C0[j]);
		}
		(*simdata).scan.push_back(simscan);
//***
	   if(i%10 == 0 || i<5) {		// output for n=101
//      if( i%1000 == 0 || i<5) {		// output for n=10001
		  for (j=0; j<N; j++)
		  {
		  		fprintf(outf, "%12.5e %15.8e %15.8e\n", simscan.time, x[j], C0[j]);
		  }
		  fprintf(outf, "\n\n");
      }
//***/
		//
		// sedimentation part:
		// Calculate thr right hand side vector //

		if (!af_params.moving_grid)
		{
			right_hand_side[0] = -CB[1][0] * C0[0] - CB[2][0] * C0[1];
			for(j=1; j<N-1; j++)
			{
				right_hand_side[j] = -CB[0][j] * C0[j-1] - CB[1][j] * C0[j] - CB[2][j] * C0[j+1];
			}
			j = N-1;
			right_hand_side[j] = -CB[0][j] * C0[j-1] - CB[1][j] * C0[j];
		}
		else
		{
			if (af_params.s[0] > 0)
			{
				right_hand_side[0] = -CB[2][0] * C0[0];
				right_hand_side[1] = -CB[1][1] * C0[0] - CB[2][1] * C0[1];
				for(j=2; j<N; j++)
				{
					right_hand_side[j] = -CB[0][j] * C0[j-2] - CB[1][j] * C0[j-1] - CB[2][j] * C0[j];
				}
			}
			else
			{
				for(j=0; j<N-2; j++)
				{
					right_hand_side[j] = -CB[0][j] * C0[j] - CB[1][j] * C0[j+1] - CB[2][j] * C0[j+2];
				}
				j = N-2;
				right_hand_side[j] = -CB[0][j] * C0[j] - CB[1][j] * C0[j+1];
				j = N-1;
				right_hand_side[j] = -CB[0][j] * C0[j];
			}
		}
		tridiag(CA[0], CA[1], CA[2], right_hand_side, C1);
		for (j=0; j<N; j++)
		{
			C0[j] = C1[j];
		}
	} // time loop
	fclose(outf);

	(*C_init).radius.clear();
	(*C_init).concentration.clear();
	for (j=0; j<N; j++)
	{
		(*C_init).radius.push_back( x[j] );
		(*C_init).concentration.push_back( C1[j] );
	}
	delete [] right_hand_side;
	delete [] C0;
	delete [] C1;
	clear_2d(3, CA);
	clear_2d(3, CB);
	if(af_params.acceleration) // then we have acceleration
	{
		clear_2d(3, CA1);
		clear_2d(3, CB1);
		clear_2d(3, CA2);
		clear_2d(3, CB2);
	}
	return (0);
}


// ***
// *** this is the SNI version of operator scheme
// ***

int US_Astfem_RSA::calculate_ra2(double rpm_start, double rpm_stop, mfem_initial *C_init, mfem_data *simdata)
{
	unsigned int Mcomp, i, j, kkk;
	double sw2, rpm_current, dval;
	double alpha, s_max, s_min;
	double ***CA, ***CA1, ***CA2;	// stiffness matrix on left hand side
											// CA[0...Ms-1][4][0...N-1]
	double ***CB, ***CB1, ***CB2;	// stiffness matrix on right hand side
											// CB[0...Ms-1][4][0...N-1]
	double **C0, **C1;				// C[m][j]: current/next concentration of m-th component at x_j
											// C[0...Ms-1][0....N-1]:
	double *CT0, *CT1;				// total concentration at current and next time step
	vector <double> xb;				// grid for moving adaptive FEM for faster sedimentation

	Mcomp = af_params.s.size();
	s_max = maxval( af_params.s );  	// used for mesh and dt
	s_min = minval( af_params.s );  	// used for mesh and dt

	FILE *outf = fopen("tmp.out", "w");
   af_params.print_af(outf);

	(*simdata).radius.clear();
	(*simdata).scan.clear();
	mfem_scan simscan;

// generate the adaptive mesh
	vector <double> nu;
	nu.clear();
	for (i=0; i<Mcomp; i++)
	{
		sw2 = af_params.s[i] * pow( rpm_stop * M_PI/30., 2.0);
		nu.push_back( sw2 / af_params.D[i]);
      printf("s[%d]=%20.12e  D=%20.12e, sw2=%20.12e\n", i, af_params.s[i], af_params.D[i], sw2);
	}
	mesh_gen(nu, af_params.mesh);

	// refine left hand side (when s_max>0) or  right hand side (when s<0) for acceleration
	if (af_params.acceleration)
	{
		double xc ;
		if ( s_min > 0 )  				// all sediment towards bottom
		{
			sw2 = s_max * pow( rpm_stop * M_PI/30., 2.0);
			xc = af_params.meniscus + sw2 * (af_params.time_steps * af_params.dt) /3.;
			for (j=0; j<N-3; j++)
			{
				if (x[j] > xc )
				{
					break;
				}
			}
			mesh_gen_RefL(j+1, 4*j);
		}
		else if ( s_max < 0 )		// all float towards meniscus
		{
			sw2 = s_min * pow( rpm_stop * M_PI/30., 2.0);	// s_min corresponds to fastest component
			xc = af_params.bottom + sw2 * (af_params.time_steps * af_params.dt) /3.;
			for (j=0; j<N-3; j++)
			{
				if (x[N-j-1] < xc )
				{
					break;
				}
			}
			mesh_gen_RefL(j+1, 4*j);
		}
		else
		{
			cout << "multicomponent system with sedimentation and floating mixed, use uniform mesh" << endl;
		}
	}
	for (i=0; i<N; i++)
	{
		(*simdata).radius.push_back(x[i]);
	}

// initialize the coefficient matrices
	initialize_3d(Mcomp, 4, N, &CA);
	initialize_3d(Mcomp, 4, N, &CB);

	if(af_params.acceleration) //  acceleration, so use fixed grid
	{
		initialize_3d(Mcomp, 3, N, &CA1);
		initialize_3d(Mcomp, 3, N, &CA2);
		initialize_3d(Mcomp, 3, N, &CB1);
		initialize_3d(Mcomp, 3, N, &CB2);
		for( i=0; i<Mcomp; i++)
		{
			sw2 = 0.;
			ComputeCoefMatrixFixedMesh(af_params.D[i], sw2, CA1[i], CB1[i]);
			sw2 = af_params.s[i] * pow(rpm_stop * M_PI/30.0, 2.0);
			ComputeCoefMatrixFixedMesh(af_params.D[i], sw2, CA2[i], CB2[i]);
		}
	}
	else		// constant sedimentation speed
	{
		if (!af_params.moving_grid )
		{
			for( i=0; i<Mcomp; i++)
			{
				sw2 = af_params.s[i] * pow(rpm_stop * M_PI/30.0, 2.0);
				ComputeCoefMatrixFixedMesh(af_params.D[i], sw2, CA[i], CB[i]);
               /****
               double tmpa=0, tmpb=0.;
					for (j=0; j<N; j++) {
                 tmpa += CA[i][0][j] + CA[i][1][j] + CA[i][2][j] + CA[i][3][j] ;
                 tmpb += CB[i][0][j] + CB[i][1][j] + CB[i][2][j] + CB[i][3][j] ;
               }
               printf("fix mesh: comp[%d]: Ca=%20.10e Cb=%20.10e \n", i, tmpa, tmpb);
               ****/
			}
		}
		else	// moving grid
		{
			if ( s_min > 0)		// all components sedimenting
			{
				for (i=0; i<Mcomp; i++)
				{
					sw2 = af_params.s[i] * pow(rpm_stop * M_PI/30.0, 2.0);
					xb.clear();
					xb.push_back(af_params.meniscus);
					for (j=0; j<N-1; j++)
					{
						dval = 0.1*exp( sw2/af_params.D[i]*( pow(0.5*(x[j]+x[j+1]), 2.0) - pow(af_params.bottom, 2.0) )/2. );
						alpha = af_params.s[i]/s_max * (1.-dval) + dval;
						// alpha = af_params.s[i]/s_max ;
						xb.push_back( pow(x[j], alpha) * pow(x[j+1], (1.0 - alpha) ) );
					}
					GlobalStiff(&xb, CA[i], CB[i], af_params.D[i], sw2 );
					// GlobalStiff_ellam(&xb, CA[i], CB[i], af_params.D[i], sw2 );
               //****
               double tmpa=0, tmpb=0.;
					for (j=0; j<N; j++) {
                 tmpa += CA[i][0][j] + CA[i][1][j] + CA[i][2][j] + CA[i][3][j] ;
                 tmpb += CB[i][0][j] + CB[i][1][j] + CB[i][2][j] + CB[i][3][j] ;
               }
               printf("mov mesh: comp[%d]: Ca=%20.10e Cb=%20.10e \n", i, tmpa, tmpb);
               //****/
				}
			}
			else if (s_max <0) 		// all components floating
			{
				cout << "all components floating, not implemented yet" << endl;
				return(-1);
			}
			else 		// sedmientation and floating mixed
			{
				cout << "sedimentation and floating mixed, suppose use fixed grid!" << endl;
				return(-1);
			}
		} // moving mesh
	} // acceleration

// Initial condition
	initialize_2d(Mcomp, N, &C0);
	initialize_2d(Mcomp, N, &C1);
	CT0 = new double [N];
	CT1 = new double [N];

// here we need the interpolatie the initial partial concentration onto new grid x[j]
   for( i=0; i<Mcomp; i++)
   {
	  interpolate_C0(&(C_init[i]), C0[i]); //interpolate the given C_init vector on the new C0 grid
   }
   for (j=0; j<N; j++)
   {
       CT0[j] = 0.;
		 for (i=0; i<Mcomp; i++)
		 {
		  	 CT0[j] += C0[i][j];
	  	 }
		 CT1[j] = CT0[j];
	}

// time evolution
	double *right_hand_side;
	right_hand_side = new double [N];
	for (kkk=0; kkk<af_params.time_steps; kkk +=2)		// two steps in together
	{
		rpm_current = rpm_start + (rpm_stop - rpm_start) * (kkk+0.5)/af_params.time_steps;
		emit current_speed((unsigned int) rpm_current);
		simscan.time = af_params.start_time + kkk * af_params.dt;
		simscan.omega_s_t = simscan.time * pow(rpm_current * M_PI/30.0, 2.0);
/**** delete to see if it the leak of mem
		if (guiFlag)
		{
			if(*movieFlag)
			{
				emit new_scan(&x, CT0);
				emit new_time((float) simscan.time);
				qApp->processEvents();
			}
		}
****/
		simscan.conc.clear();
		for (j=0; j<N; j++)
		{
			simscan.conc.push_back(CT0[j]);
		}
		(*simdata).scan.push_back(simscan);

		if(kkk%10 == 0 || kkk<5)
//		if(kkk%1000 == 0 || kkk<5)		// output for n=10001
		{
			for(j=0; j<N; j++)
			{
				fprintf(outf, "%12.5e %15.8e %15.8e ", simscan.time, x[j], CT0[j]);
			   for(i=0; i<Mcomp; i++) fprintf(outf, "%15.8e ", C0[i][j]);
			   fprintf(outf, "\n");
			}
			fprintf(outf, "\n\n");
         printf("t=%12.5e C_ttl=%15.8e \n", simscan.time, IntConcentration(x, CT0));
		}

      //
      // first half step of sedimentation:
      //
		if( af_params.acceleration ) // need to reconstruct CA and CB by linear interpolation
		{
			dval =  pow(rpm_current/rpm_stop, 2.0) ;
			for(i=0; i<Mcomp; i++)
			{
				for(unsigned int j1=0; j1<3; j1++)
				{
					for(unsigned int j2=0; j2<N; j2++)
					{
						CA[i][j1][j2] = CA1[i][j1][j2] + dval * (CA2[i][j1][j2] - CA1[i][j1][j2]) ;
						CB[i][j1][j2] = CB1[i][j1][j2] + dval * (CB2[i][j1][j2] - CB1[i][j1][j2]) ;
					}
				}
			}
		}
		if (!af_params.moving_grid)   // for fixed grid
		{
			for (i=0; i<Mcomp; i++)
			{
				right_hand_side[0] = -CB[i][1][0] * C0[i][0] - CB[i][2][0] * C0[i][1];
				for(j=1; j<N-1; j++)
				{
					right_hand_side[j] = -CB[i][0][j] * C0[i][j-1] - CB[i][1][j] * C0[i][j] - CB[i][2][j] * C0[i][j+1];
				}
				j = N-1;
				right_hand_side[j] = -CB[i][0][j] * C0[i][j-1] - CB[i][1][j] * C0[i][j];
				tridiag(CA[i][0], CA[i][1], CA[i][2], right_hand_side, C1[i]);
			}
		}
		else // moving grid
		{
			for (i=0; i<Mcomp; i++)
			{
            // Calculate the right hand side vector //
			   right_hand_side[0] = -CB[i][2][0] * C0[i][0] - CB[i][3][0] * C0[i][1];
			   right_hand_side[1] = -CB[i][1][1] * C0[i][0] - CB[i][2][1] * C0[i][1] - CB[i][3][1] * C0[i][2];
			   for (j=2; j<N-1; j++)
			   {
				   right_hand_side[j] = - CB[i][0][j] * C0[i][j-2]
				   							- CB[i][1][j] * C0[i][j-1]
				   							- CB[i][2][j] * C0[i][j]
				   							- CB[i][3][j] * C0[i][j+1];
			   }
				j = N-1;
				right_hand_side[j] = - CB[i][0][j] * C0[i][j-2]
											- CB[i][1][j] * C0[i][j-1]
											- CB[i][2][j] * C0[i][j];

				QuadSolver(CA[i][0], CA[i][1], CA[i][2], CA[i][3], right_hand_side, C1[i]);
				// QuadSolver_ellam(CA[i][0], CA[i][1], CA[i][2], CA[i][3], right_hand_side, C1[i]);
			}
		}
      //
      // reaction part: instantanuous reaction at each node
      //
      // instantanuous reaction at each node
      // [C1]=ReactionOneStep_inst(C1);
      //
      // finite reaction rate: linear interpolation of instantaneous reaction
		      ReactionOneStep_Euler_imp(C1, 2*af_params.dt);
      //
      //
      // for next half time-step in SNI operator splitting scheme
      //
		for (j=0; j<N; j++)
		{
         CT1[j] = 0.;
			for (i=0; i<Mcomp; i++)
			{
			   CT1[j] += C1[i][j];
				C0[i][j] = C1[i][j];
			}
			CT0[j] = CT1[j];
		}

      //
      // 2nd half step of sedimentation:
      //
		rpm_current = rpm_start + (rpm_stop - rpm_start) * (kkk+1.5)/af_params.time_steps;
		if( af_params.acceleration ) // need to reconstruct CA and CB by linear interpolation
		{
			dval =  pow(rpm_current/rpm_stop, 2.0) ;
			for(i=0; i<Mcomp; i++)
			{
				for(unsigned int j1=0; j1<3; j1++)
				{
					for(unsigned int j2=0; j2<N; j2++)
					{
						CA[i][j1][j2] = CA1[i][j1][j2] + dval * (CA2[i][j1][j2] - CA1[i][j1][j2]) ;
						CB[i][j1][j2] = CB1[i][j1][j2] + dval * (CB2[i][j1][j2] - CB1[i][j1][j2]) ;
					}
				}
			}
		}
		if (!af_params.moving_grid)   // for fixed grid
		{
			for (i=0; i<Mcomp; i++)
			{
				right_hand_side[0] = -CB[i][1][0] * C0[i][0] - CB[i][2][0] * C0[i][1];
				for(j=1; j<N-1; j++)
				{
					right_hand_side[j] = -CB[i][0][j] * C0[i][j-1] - CB[i][1][j] * C0[i][j] - CB[i][2][j] * C0[i][j+1];
				}
				j = N-1;
				right_hand_side[j] = -CB[i][0][j] * C0[i][j-1] - CB[i][1][j] * C0[i][j];
				tridiag(CA[i][0], CA[i][1], CA[i][2], right_hand_side, C1[i]);
			}
		}
		else // moving grid
		{
			for (i=0; i<Mcomp; i++)
			{
            // Calculate the right hand side vector //
			   right_hand_side[0] = -CB[i][2][0] * C0[i][0] - CB[i][3][0] * C0[i][1];
			   right_hand_side[1] = -CB[i][1][1] * C0[i][0] - CB[i][2][1] * C0[i][1] - CB[i][3][1] * C0[i][2];
			   for (j=2; j<N-1; j++)
			   {
				   right_hand_side[j] = - CB[i][0][j] * C0[i][j-2]
				   - CB[i][1][j] * C0[i][j-1]
				   - CB[i][2][j] * C0[i][j]
				   - CB[i][3][j] * C0[i][j+1];
			   }
				j = N-1;
				right_hand_side[j] = - CB[i][0][j] * C0[i][j-2]
											- CB[i][1][j] * C0[i][j-1]
											- CB[i][2][j] * C0[i][j];
				QuadSolver(CA[i][0], CA[i][1], CA[i][2], CA[i][3], right_hand_side, C1[i]);
				// QuadSolver_ellam(CA[i][0], CA[i][1], CA[i][2], CA[i][3], right_hand_side, C1[i]);
			}
		}
      // end of 2nd half step of sendimentation

      //
      // for next 2 time steps
      //
		for (j=0; j<N; j++)
		{
			CT1[j] = 0.;
			for (i=0; i<Mcomp; i++)
			{
			   CT1[j] += C1[i][j];
				C0[i][j] = C1[i][j];
			}
			CT0[j] = CT1[j];
		}


	} // time loop
	emit new_scan(&x, CT0);
	fclose(outf);
   for(i=0;i<Mcomp;i++)
   {
	  C_init[i].radius.clear();
	  C_init[i].concentration.clear();
	  for (j=0; j<N; j++)
	  {
		  C_init[i].radius.push_back( x[j] );
		  C_init[i].concentration.push_back( C1[i][j] );
	  }
   }
	delete [] CT1;
	delete [] CT0;
	delete [] right_hand_side;
	clear_2d(Mcomp, C0);
	clear_2d(Mcomp, C1);
	clear_3d(Mcomp, 4, CA);
	clear_3d(Mcomp, 4, CB);
	if( af_params.acceleration ) // then we have acceleration
	{
		clear_3d(Mcomp, 3, CA1);
		clear_3d(Mcomp, 3, CB1);
		clear_3d(Mcomp, 3, CA2);
		clear_3d(Mcomp, 3, CB2);
	}
	return(0);
}


double US_Astfem_RSA::minval(vector <double> val)
{
	double minimum=1.0e300;
	for (unsigned int i=0; i<val.size(); i++)
	{
		minimum = min(minimum, val[i]);
	}
	return minimum;
}

double US_Astfem_RSA::minval(vector <SimulationComponent> val)
{
	double minimum=1.0e300;
	for (unsigned int i=0; i<val.size(); i++)
	{
		minimum = min(minimum, (double) val[i].s);
	}
	return minimum;
}

double US_Astfem_RSA::maxval(vector <double> val)
{
	double maximum = -1.0e300;
	for (unsigned int i=0; i<val.size(); i++)
	{
		maximum = max(maximum, val[i]);
	}
	return maximum;
}

double US_Astfem_RSA::maxval(vector <SimulationComponent> val)
{
	double maximum = -1.0e300;
	for (unsigned int i=0; i<val.size(); i++)
	{
		maximum = max(maximum, (double) val[i].s);
	}
	return maximum;
}

//************ new version *************

//////////////////////////////////////////////////////////////%
//
// Generate exponential mesh and refine cell bottom (for s>0)
//
//////////////////////////////////////////////////////////////%
void US_Astfem_RSA::mesh_gen_s_pos(vector <double> nu)
{
	cout << "using adaptive mesh...\n";
	double uth = 1.0/af_params.simpoints;		// threshold of u for steep region
	double tmp_xc, tmp_Hstar, xa;
	unsigned int IndLayer=0;		// number of layers for grids in steep region
	unsigned int i, j, tmp_Nf;
	vector <double> xc, Hstar, y;
	vector <unsigned int> Nf;
	xc.clear();
	Hstar.clear();
	Nf.clear();

	for (i=0; i<af_params.s.size(); i++) 	// markers for steep regions
	{
		tmp_xc = af_params.bottom - (1.0/(nu[i] * af_params.bottom)) * log(nu[i]
				* (pow((double) af_params.bottom, (double) 2.0)
				- pow((double) af_params.meniscus, (double) 2.0))/(2.0 * uth));
		tmp_Nf = (int) (M_PI/2.0 * (af_params.bottom - tmp_xc)
				* nu[i] * af_params.bottom/2.0 + 0.5) + 1; // # of pts for i-th layer
		tmp_Hstar = (af_params.bottom - tmp_xc)/tmp_Nf * M_PI/2.0;			// step required by Pac(i)<1
		if ((tmp_xc > af_params.meniscus) &&
			(af_params.bottom - af_params.meniscus *
			pow((double) af_params.bottom/af_params.meniscus,
			(double)((af_params.simpoints-4.0/2.0)/(af_params.simpoints-1.0))) > tmp_Hstar))
//			(double)((af_params.simpoints-5.0/2.0)/(af_params.simpoints-1.0))) > tmp_Hstar))
		{
			xc.push_back(tmp_xc);
			Nf.push_back(tmp_Nf);
			Hstar.push_back(tmp_Hstar);
			IndLayer ++;
		}
	}
	xc.push_back(af_params.bottom);
	print_vector(&xc);

	if (IndLayer == 0)	// use Schuck's grid only
	{
		x.push_back(af_params.meniscus);
//		for(i=1; i<af_params.simpoints ; i++)	// add one more point to Schuck's grids
		for(i=1; i<af_params.simpoints -1 ; i++)	// add one more point to Schuck's grids
		{
			x.push_back(af_params.meniscus * pow((double) (af_params.bottom/af_params.meniscus),
			(((double) i - 0.0)/((double)(af_params.simpoints - 1)))));	// Schuck's mesh
//			(((double) i - 0.5)/((double)(af_params.simpoints - 1)))));	// Schuck's mesh
		}
		x.push_back(af_params.bottom);
	}
	else				// need a composite grid
	{
		printf("IndLayer=%d \n", IndLayer);
// steep region
		unsigned int indp = 0, Mp=0; 	// index for a grid point
		double HL, HR, Hf, alpha, beta, xi;
		for (i=0; i<IndLayer; i++)  // consider i-th steep region
		{
			if (i < IndLayer-1)	// use linear distribution for step size distrib
			{
				HL = Hstar[i];
				HR = Hstar[i+1];
				Mp = int ((xc[i+1] - xc[i]) * 2.0/(HL + HR));
            if ( Mp>1 ) {
				   // alpha = Mp * HL - ((HR - HL)/2.0) * Mp/(Mp - 1.0);
				   // beta = ((HR-HL)/2.0) * pow(Mp, 2.0)/(Mp - 1.0);
				   beta = ((HR-HL)/2.0) * Mp ;
               alpha = (xc[i+1]-xc[i])-beta;
				   for (j=0; j<=Mp-1; j++)
				   {
				 	   xi = (double) j/ (double) Mp;
				 	   y.push_back(xc[i] + alpha * xi + beta * pow(xi, 2.0));
					   indp++;
				   }
            }
			}
			else		// last layer, use sine distribution for grids
			{
				for (j=0; j<=Nf[i]-1; j++)
				{
					indp++;
					y.push_back(xc[i] + (af_params.bottom - xc[i]) * sin(j/(Nf[i] - 1.0) * M_PI/2.0));
					if (y[indp-1] > xc[i+1])
					{
						break;
					}
				}
			}
		}
		int NfTotal = indp;
		vector <double> ytmp;
		ytmp.clear();
// reverse the order of y
		j = NfTotal;
		do
		{
			j--;
			ytmp.push_back(y[j]);
		} while (j != 0);
		y.clear();
		y = ytmp;
// transition region
// smallest step size in transit region
		Hf = y[NfTotal - 2] - y[NfTotal-1];
		unsigned int Nm = (unsigned int) (floor(log(af_params.bottom/((af_params.simpoints - 1) * Hf)
				* log(af_params.bottom/af_params.meniscus))/log(2.0))+1) ; // number of pts in trans region
		xa = y[NfTotal-1] - Hf * (pow(2.0, (double)Nm) -1.);
		unsigned int Js = (unsigned int) (floor(0.0 + (af_params.simpoints - 1)
				* log(xa/af_params.meniscus)/log(af_params.bottom/af_params.meniscus)));
// xa is  modified so that y[NfTotal-Nm] matches xa exactly
		xa = af_params.meniscus*pow((double)(af_params.bottom/af_params.meniscus),
				(((double) Js - 0.0)/((double)af_params.simpoints - 1.0)));
      tmp_xc = y[NfTotal-1];
      HL = xa * (1.-af_params.meniscus/af_params.bottom);
		HR = y[NfTotal - 2]- y[NfTotal-1];
		Mp = (unsigned int) (((tmp_xc - xa) * 2.0/(HL + HR))+1);
      if ( Mp>1 ) {
			beta = ((HR-HL)/2.0) * Mp ;
         alpha = (tmp_xc - xa) -beta;
			for (j=Mp-1; j>0; j--)
			{
			   xi = (double) j/ (double) Mp;
				y.push_back(xa + alpha * xi + beta * pow(xi, 2.0));
		   }
      }
      Nm = Mp;

		printf("xa=%15.8e Hf=%12.5e Nm=%d, Js=%d, NfTotal=%d\n", xa, HR, Nm, Js, NfTotal);

// regular region
		x.push_back(af_params.meniscus);
		for (j=1; j<=Js; j++)
		{
			x.push_back(af_params.meniscus*
			pow((double) (af_params.bottom/af_params.meniscus),
			(((double) j - 0.0)/((double)af_params.simpoints - 1.0))));
		}

		for (j=0; j<NfTotal+Nm-1; j++)
		{
			x.push_back(y[NfTotal+Nm-j-2]);
		}

/*** smooth out the grids in transition region ***
      for (j=0; j<Nm-1; j++)
      {
	      x[j + Js]=(x[j + Js - 1] + x[j + Js +1])/2.0;
      }

      for (j=0; j<x.size()-1; j++)
      {
	      printf("x[%d]=%15.8e dx=%12.5e\n", j, x[j], x[j+1]-x[j]);
      }
      printf("x[%d]=%15.8e dx=%12.5e\n", j, x[j], 0.);
***/

	}
}

/*********************** old version **********

//////////////////////////////////////////////////////////////%
//
// Generate exponential mesh and refine cell bottom (for s>0)
//
//////////////////////////////////////////////////////////////%
void US_Astfem_RSA::mesh_gen_s_pos(vector <double> nu)
{
	cout << "using adaptive mesh...\n";
	double uth = 1.0/af_params.simpoints;		// threshold of u for steep region
	double tmp_xc, tmp_Hstar, xa;
	unsigned int IndLayer=0;		// number of layers for grids in steep region
	unsigned int i, j, tmp_Nf;
	vector <double> xc, Hstar, y;
	vector <unsigned int> Nf;
	xc.clear();
	Hstar.clear();
	Nf.clear();

	for (i=0; i<af_params.s.size(); i++) 	// markers for steep regions
	{
		tmp_xc = af_params.bottom - (1.0/(nu[i] * af_params.bottom)) * log(nu[i]
				* (pow((double) af_params.bottom, (double) 2.0)
				- pow((double) af_params.meniscus, (double) 2.0))/(2.0 * uth));
		tmp_Nf = (int) (M_PI/2.0 * (af_params.bottom - tmp_xc)
				* nu[i] * af_params.bottom/2.0 + 0.5) + 1; // # of pts for i-th layer
		tmp_Hstar = (af_params.bottom - tmp_xc)/tmp_Nf * M_PI/2.0;			// step required by Pac(i)<1
		if ((tmp_xc > af_params.meniscus) &&
			(af_params.bottom - af_params.meniscus *
			pow((double) af_params.bottom/af_params.meniscus,
			(double)((af_params.simpoints-5.0/2.0)/(af_params.simpoints-1.0))) > tmp_Hstar))
		{
			xc.push_back(tmp_xc);
			Nf.push_back(tmp_Nf);
			Hstar.push_back(tmp_Hstar);
			IndLayer ++;
		}
	}
	xc.push_back(af_params.bottom);
	print_vector(&xc);

	if (IndLayer == 0)	// use Schuck's grid only
	{
		x.push_back(af_params.meniscus);
//		for(i=1; i<af_params.simpoints - 1; i++)	// standard Schuck's grids
		for(i=1; i<af_params.simpoints - 0; i++)	// add one more point to Schuck's grids
		{
			x.push_back(af_params.meniscus * pow((double) (af_params.bottom/af_params.meniscus),
			(((double) i - 0.5)/((double)(af_params.simpoints - 1)))));	// Schuck's mesh
		}
		x.push_back(af_params.bottom);
	}
	else				// need a composite grid
	{
		printf("IndLayer=%d \n", IndLayer);
// steep region
		int indp = 0, Mp=0; 	// index for a grid point
		double HL, HR, Hf, alpha, beta, xi;
		for (i=0; i<IndLayer; i++)  // consider i-th steep region
		{
			if (i < IndLayer-1)	// use linear distribution for step size distrib
			{
				HL = Hstar[i];
				HR = Hstar[i+1];
				Mp = int ((xc[i+1] - xc[i]) * 2.0/(HL + HR));
            if ( Mp>1 ) {
				   // alpha = Mp * HL - ((HR - HL)/2.0) * Mp/(Mp - 1.0);
				   // beta = ((HR-HL)/2.0) * pow(Mp, 2.0)/(Mp - 1.0);
				   beta = ((HR-HL)/2.0) * Mp ;
               alpha = (xc[i+1]-xc[i])-beta;
				   for (j=0; j<=Mp-1; j++)
				   {
				 	   xi = (double) j/ (double) Mp;
				 	   y.push_back(xc[i] + alpha * xi + beta * pow(xi, 2.0));
					   indp++;
				   }
            }
			}
			else		// last layer, use sine distribution for grids
			{
				for (j=0; j<=Nf[i]-1; j++)
				{
					indp++;
					y.push_back(xc[i] + (af_params.bottom - xc[i]) * sin(j/(Nf[i] - 1.0) * M_PI/2.0));
					if (y[indp-1] > xc[i+1])
					{
						break;
					}
				}
			}
		}
		int NfTotal = indp;
		vector <double> ytmp;
		ytmp.clear();
// reverse the order of y
		j = NfTotal;
		do
		{
			j--;
			ytmp.push_back(y[j]);
		} while (j != 0);
		y.clear();
		y = ytmp;
// transition region
// smallest step size in transit region
		Hf = y[NfTotal - 2]- y[NfTotal-1];
		int Nm = floor(log(af_params.bottom/((af_params.simpoints - 1) * Hf)
				* log(af_params.bottom/af_params.meniscus))/log(2.0))+1 ; // number of pts in trans region
		xa = y[NfTotal-1] - Hf * (pow(2.0, (double)Nm) -1.);
		int Js = floor(0.5 + (af_params.simpoints - 1)
				* log(xa/af_params.meniscus)/log(af_params.bottom/af_params.meniscus));

// xa and Hf are modified so that y[NfTotal-Nm] matches xa exactly
		xa = af_params.meniscus*pow((double)(af_params.bottom/af_params.meniscus),
				(((double) Js - 0.5)/((double)af_params.simpoints - 1.0)));
      Nm += 1;		// use one more element to ensure the elem size in transition region is smaller than in regular region
		Hf = ( y[NfTotal-1] - xa )/(pow(2.0, (double)Nm) - 1.0);

		printf("xa=%15.8e Hf=%12.5e Nm=%d, Js=%d, NfTotal=%d\n", xa, Hf, Nm, Js, NfTotal);
		for (j=1; j<Nm; j++)
		{
			y.push_back(y[NfTotal-1] - Hf * (pow(2.0, (double) j) - 1));
		}
// regular region
		x.push_back(af_params.meniscus);
		for (j=1; j<=Js; j++)
		{
			x.push_back(af_params.meniscus*
			pow((double) (af_params.bottom/af_params.meniscus),
			(((double) j - 0.5)/((double)af_params.simpoints - 1.0))));
		}

		for (j=0; j<NfTotal+Nm-1; j++)
		{
			x.push_back(y[NfTotal+Nm-j-2]);
		}

// *** smooth out the grids in transition region ***
//      for (j=0; j<NfTotal+Nm-2; j++)
//      {
//	      x[j + Js - 1]=(x[j + Js - 2] + x[j + Js])/2.0;
//      }
//
//      for (j=0; j<x.size()-1; j++)
//      {
//	      printf("x[%d]=%15.8e dx=%12.5e\n", j, x[j], x[j+1]-x[j]);
//      }
//      printf("x[%d]=%15.8e dx=%12.5e\n", j, x[j], 0.);
// ***

	}
}
******************end of old version ************/

//////////////////////////////////////////////////////////////%
//
// Generate exponential mesh and refine cell meniscus (for  s<0)
//
//////////////////////////////////////////////////////////////%
void US_Astfem_RSA::mesh_gen_s_neg(vector <double> nu)
{
	cout << "using adaptive mesh...\n";
	unsigned int j, Js, Nf, Nm;
	double uth = 1.0/af_params.simpoints;		// threshold of u for steep region
	double xc, xa, Hstar;
	vector <double> yr, ys, yt;

	printf("m=%12.5e b=%12.5e nu=%12.5e Nr=%d \n",
	af_params.meniscus, af_params.bottom, nu[0], af_params.simpoints);

	x.clear();
	yr.clear();
	ys.clear();
	yt.clear();

	double nu0 = nu[0];
	xc = af_params.meniscus + 1./(fabs(nu0) * af_params.meniscus) *
	log((pow(af_params.bottom, 2.0) - pow(af_params.meniscus, 2.0)) * fabs(nu0)/(2.0*uth));
	Nf = (unsigned int) (1 + (floor)( (xc - af_params.meniscus) * fabs(nu0) * af_params.meniscus * M_PI/4.0));
	Hstar = (xc - af_params.meniscus)/Nf * M_PI/2.0;
	Nm = (unsigned int) (1 + (floor) (log(af_params.meniscus/((af_params.simpoints - 1.) * Hstar)
			* log(af_params.bottom/af_params.meniscus))/log(2.0)));
	xa = xc + (pow(2.0, (double) Nm) - 1.0) * Hstar;
	Js = (unsigned int) ((floor)((af_params.simpoints - 1) * log(af_params.bottom/xa)/log(af_params.bottom/af_params.meniscus) + 0.5 ));
	printf("Nf=%d Nm=%d Js=%d \n", Nf, Nm, Js);
	printf("xc=%12.5e xa=%12.5e \n", xc, xa);

// all grdi points at exponentials
	yr.push_back(af_params.bottom);
//	for(j=1; j<(int) af_params.simpoints-1; j++)		// standard Schuck's grids
	for(j=1; j<af_params.simpoints; j++)		// add one more point to Schuck's grids
	{
		yr.push_back(af_params.bottom * pow(af_params.meniscus/af_params.bottom, (j - 0.5)/(af_params.simpoints - 1.0)));
	}
	yr.push_back(af_params.meniscus);
	if(af_params.bottom * (pow(af_params.meniscus/af_params.bottom, (af_params.simpoints - 3.5)/(af_params.simpoints - 1.0))
	- pow(af_params.meniscus/af_params.bottom, (af_params.simpoints - 2.5)/(af_params.simpoints - 1.0))) < Hstar || Nf <= 2 )
	{
// no need for steep region
		for(j=0; j<af_params.simpoints; j++)
		{
			x.push_back(yr[af_params.simpoints - 1 - j]);
		}
		printf("use exponential grid only!\n");
	}
	else
	{// Nf>2
		for(j=1;j<Nf;j++)
		{
			ys.push_back(xc - (xc - af_params.meniscus) * sin((j - 1.0)/(Nf - 1.0) * M_PI/2.0));
		}
		ys.push_back(af_params.meniscus);
		for (j=0; j<Nm; j++)
		{
			yt.push_back(xc + (pow(2.0, (double) j) - 1.0) * Hstar);
		}
// set x:
		for(j=0; j<Nf; j++)
		{
			x.push_back(ys[Nf - 1 - j]);
		}
		for(j=1; j<Nm; j++)
		{
			x.push_back(yt[j]);
		}
		for(j=Js+1; j>0; j--)
		{
			x.push_back(yr[j-1]);
		}
// smooth out
		x[Nf + Nm] = (x[Nf + Nm - 1] + x[Nf + Nm + 1])/2.0;
		x[Nf + Nm + 1] = (x[Nf + Nm] + x[Nf + Nm + 2])/2.0;
	} // if
}


///////////////////////////////////////////////////////////
//
//	mesh_gen_RefL:	refine mesh near meniscus (for s>0) or near bottom (for s<0)
//						to be used for the acceleration stage
//  parameters: N0 = number of elements near meniscus (or bottom) to be refined
//				M0 = number of elements to be used for the refined region
//
///////////////////////////////////////////////////////////
void US_Astfem_RSA::mesh_gen_RefL(int N0, int M0)
{
	int j;
	vector <double> zz;	// temperary array for adaptive grids
	zz.clear();
	if( minval(af_params.s) > 0 )				// all species with s>0
	{
// refine around the meniscus for acceleration
		for(j=0; j<M0; j++)
		{
			double tmp = (double)(j)/(double)(M0);
			tmp = 1. - cos(tmp*M_PI/2.);
			zz.push_back( x[0]*(1.-tmp) + x[N0]*tmp );
		}
		for(j=N0; j<(int) x.size(); j++)
		{
			zz.push_back( x[j] );
		}
		x.clear();
		for(j=0;j<(int) zz.size(); j++)
		{
			x.push_back( zz[j] );
		}
	}
	else if ( maxval(af_params.s)<0 )    //  all species with s<0
	{
		for(j=0; j<(int) x.size()-N0; j++)
		{
			zz.push_back( x[j] );
		}
		// refine around the bottom for acceleration
		for(j=1; j<=M0; j++)
		{
			double tmp = (double)(j)/(double)(M0);
			tmp = sin(tmp*M_PI/2.);
			zz.push_back( x[ x.size() -N0-1 ]*(1.-tmp) + x[ x.size() -1 ]*tmp );
		}
		x.clear();
		for (j=0; j<(int) zz.size();j++)
		{
			x.push_back(zz[j]);
		}
	}
	else								// sedimentation and floating mixed up
	{
		cout << "no refinement at ends since sedimentation and floating mixed ...\n" ;
	}
	N = x.size();
}


void US_Astfem_RSA::mesh_gen(vector <double> nu, unsigned int MeshOpt)
{
//////////////////////////////////////////////////////////////%
//
// Generate adaptive grids for multi-component Lamm equations
//
//
// Here: N: Number of points in the ASTFEM
//		m, b: meniscus, bottom
//		nuMax, nuMin = max and min of nu=sw^2/D
//		MeshType: = 0 ASTFEM grid based on all nu (composite in sharp region)
//					= 1 Claverie (uniform), etc,
//					= 2 Exponential mesh (Schuck's formular, no refinement at bottom)
//					= 3 input from data file: "mesh_data.dat"
//					= 10, af_params.acceleration mesh (left and right refinement)
//////////////////////////////////////////////////////////////%

////////////////////%
// generate the mesh
////////////////////%


	x.clear();
	sort(nu.begin(), nu.end());	// put nu in ascending order
	switch ( MeshOpt )
	{
		//////////////////////%
		// Mesh Type 0 (default): adaptive mesh based on all nu
		//////////////////////%
		case 0: //Astfem without left hand refinement
		{
			if( nu[0]>0 )
			{
				cout << "exponential mesh plus refinement at bottom, for s>0 ...\n";
				mesh_gen_s_pos(nu);
			}
			else if ( nu[ nu.size()-1 ] < 0 )
			{
				cout << "exponential mesh plus refinement at meniscus, for s<0 ...\n";
				mesh_gen_s_neg(nu);
			}
			else							// some species with s<0 and some with s>0
			{
				cout << "multicomponent system with sedimentation and floating mixed, use uniform mesh...\n";
				for ( unsigned int i=0; i<af_params.simpoints; i++)
				{
					x.push_back(af_params.meniscus + (af_params.bottom -
					af_params.meniscus) * i/(af_params.simpoints-1));
				}
			}
			break;
		}
		case 1: //Claverie mesh without left hand refinement
		{
			cout << "using uniform mesh ...\n";
			for ( unsigned int i=0; i<af_params.simpoints; i++)
			{
				x.push_back(af_params.meniscus + (af_params.bottom -
				af_params.meniscus) * i/(af_params.simpoints-1));
			}
			break;
		}
		case 2: //Moving Hat (Peter Schuck's Mesh) without left hand side refinement
		{
			cout << "using moving hat mesh...\n";
			x.push_back(af_params.meniscus);
			for(unsigned int i=1; i<af_params.simpoints-1; i++)  // standard Schuck's grids
			{
				x.push_back(af_params.meniscus * pow((double) (af_params.bottom/af_params.meniscus),
				(((double) i - 0.5)/((double)(af_params.simpoints-1)))));	// Schuck's mesh
			}
			x.push_back(af_params.bottom);
			break;
		}
		case 3: // user defined mesh generated from data file
		{
			cout << "using mesh from file $ULTRASCAN/mesh.dat...\n";
			QString str = getenv("ULTRASCAN");
			QFile f(str + "/mesh.dat");
			if (f.open(IO_ReadOnly))
			{
				QTextStream ts(&f);
				while(!ts.atEnd())
				{
					str = ts.readLine();
					x.push_back(str.toDouble());
				}
				f.close();
				if (fabs(x[0] - af_params.meniscus) > 1.0e7)
				{
					cout << "The meniscus from the mesh file does not match the set meniscus - using Claverie Mesh instead\n";
				}
				if (fabs(x[x.size()-1] - af_params.bottom) > 1.0e7)
				{
					cout << "The cell bottom from the mesh file does not match the set cell bottom - using Claverie Mesh instead\n";
				}
			}
			else
			{
				cerr << tr("Could not read the mesh file - using Claverie Mesh instead\n");
				for (unsigned int i=0; i<af_params.simpoints; i++)
				{
					x.push_back(af_params.meniscus + (af_params.bottom -
					af_params.meniscus) * i/(af_params.simpoints-1));
				}
			}
			break;
		}
		default:
		{
			cerr << "undefined mesh option\n";
		}
	}

	N = x.size();
	cout << "total number of points = " << N << "\n";
}

//
//	Compute the coefficient matrices based on fixed mesh
//
void US_Astfem_RSA::ComputeCoefMatrixFixedMesh(double D, double sw2, double **CA, double **CB)
{
	unsigned int k;
	// compute local stiffness matrices
	StiffBase stfb0 ;
	double ***Stif; 	// Local stiffness matrix at each element
	initialize_3d(N-1, (unsigned int) 4, (unsigned int) 4, &Stif);
	double xd[4][2];		// coord for verices of quad elem
	for(k=0; k<N-1; k++)
	{ 	// loop for all elem
		xd[0][0] = x[k  ];
		xd[0][1] = 0.;
		xd[1][0] = x[k+1];
		xd[1][1] = 0.;
		xd[2][0] = x[k+1];
		xd[2][1] = af_params.dt;
		xd[3][0] = x[k  ];
		xd[3][1] = af_params.dt;
		stfb0.CompLocalStif(4, xd, D, sw2, Stif[k]);
	}
	// assembly coefficient matrices
	// elem[0]; i=0
	k = 0;
	CA[1][k] = Stif[k][3][0] + Stif[k][3][3]; 	// j=3;
	CA[2][k] = Stif[k][2][0] + Stif[k][2][3];	// j=2;
	CB[1][k] = Stif[k][0][0] + Stif[k][0][3]; 	// j=0;
	CB[2][k] = Stif[k][1][0] + Stif[k][1][3];	// j=1;

	for(k=1; k<N-1;k++)
	{ 	// loop for all elem
		// elem k-1: i=1,2
		CA[0][k]  = Stif[k-1][3][1] + Stif[k-1][3][2];	// j=3;
		CA[1][k]  = Stif[k-1][2][1] + Stif[k-1][2][2];	// j=2;
		CB[0][k]  = Stif[k-1][0][1] + Stif[k-1][0][2];	// j=0;
		CB[1][k]  = Stif[k-1][1][1] + Stif[k-1][1][2];	// j=1;

		// elem k: i=0,3
		CA[1][k] += Stif[k  ][3][0] + Stif[k  ][3][3];	// j=3;
		CA[2][k]  = Stif[k  ][2][0] + Stif[k  ][2][3];	// j=2;
		CB[1][k] += Stif[k  ][0][0] + Stif[k  ][0][3];	// j=0;
		CB[2][k]  = Stif[k  ][1][0] + Stif[k  ][1][3];	// j=1;
	}

	// elem[N-2]; i=1,2
	k = N-1;
	CA[0][k]  = Stif[k-1][3][1] + Stif[k-1][3][2];	// j=3;
	CA[1][k]  = Stif[k-1][2][1] + Stif[k-1][2][2];	// j=2;
	CB[0][k]  = Stif[k-1][0][1] + Stif[k-1][0][2];	// j=0;
	CB[1][k]  = Stif[k-1][1][1] + Stif[k-1][1][2];	// j=1;
	clear_3d(N-1, (unsigned int) 4, Stif);
}

void US_Astfem_RSA::ComputeCoefMatrixMovingMeshR(double D, double sw2, double **CA, double **CB)
{
	unsigned int k;
	// compute local stiffness matrices
	StiffBase stfb0 ;
	double ***Stif; 	// Local stiffness matrix at each element
	initialize_3d(N, (unsigned int) 4, (unsigned int) 4, &Stif);
	double xd[4][2];		// coord for verices of quad elem

	// elem[0]: triangle
	xd[0][0] = x[0];	xd[0][1] = 0.;
	xd[1][0] = x[1];	xd[1][1] = af_params.dt;
	xd[2][0] = x[0];	xd[2][1] = af_params.dt;
	stfb0.CompLocalStif(3, xd, D, sw2, Stif[0]);

	// elem[k]: k=1..(N-2), quadrilateral
	for(k=1; k<N-1; k++)
	{ 	// loop for all elem
		xd[0][0] = x[k-1];	xd[0][1] = 0.;
		xd[1][0] = x[k  ];	xd[1][1] = 0.;
		xd[2][0] = x[k+1];	xd[2][1] = af_params.dt;
		xd[3][0] = x[k  ];	xd[3][1] = af_params.dt;
		stfb0.CompLocalStif(4, xd, D, sw2, Stif[k]);
	}

	// elem[N-1]: triangle
	xd[0][0] = x[N-2];	xd[0][1] = 0.;
	xd[1][0] = x[N-1];	xd[1][1] = 0.;
	xd[2][0] = x[N-1];	xd[2][1] = af_params.dt;
	stfb0.CompLocalStif(3, xd, D, sw2, Stif[N-1]);

	// assembly coefficient matrices

	k = 0;
	CA[1][k] = Stif[k][2][2] ;
	CA[2][k] = Stif[k][1][2] ;
	CB[2][k] = Stif[k][0][2] ;

	k = 1;
	CA[0][k] = Stif[k-1][2][0] + Stif[k-1][2][1];
	CA[1][k] = Stif[k-1][1][0] + Stif[k-1][1][1];
	CA[1][k]+= Stif[k  ][3][0] + Stif[k  ][3][3];
	CA[2][k] = Stif[k  ][2][0] + Stif[k  ][2][3] ;

	CB[1][k] = Stif[k-1][0][0] + Stif[k-1][0][1]; 	// j=0;
	CB[1][k]+= Stif[k  ][0][0] + Stif[k  ][0][3]; 	// j=0;
	CB[2][k] = Stif[k  ][1][0] + Stif[k  ][1][3];	// j=1;

	for(k=2; k<N-1;k++)
	{ 	// loop for all elem
		// elem k-1: i=1,2
		CA[0][k]  = Stif[k-1][3][1] + Stif[k-1][3][2];	// j=3;
		CA[1][k]  = Stif[k-1][2][1] + Stif[k-1][2][2];	// j=2;
		CB[0][k]  = Stif[k-1][0][1] + Stif[k-1][0][2];	// j=0;
		CB[1][k]  = Stif[k-1][1][1] + Stif[k-1][1][2];	// j=1;

		// elem k: i=0,3
		CA[1][k] += Stif[k  ][3][0] + Stif[k  ][3][3];	// j=3;
		CA[2][k]  = Stif[k  ][2][0] + Stif[k  ][2][3];	// j=2;
		CB[1][k] += Stif[k  ][0][0] + Stif[k  ][0][3];	// j=0;
		CB[2][k]  = Stif[k  ][1][0] + Stif[k  ][1][3];	// j=1;
	}

	k = N-1;
	// elem[k-1]: quadrilateral
	CA[0][k]  = Stif[k-1][3][1] + Stif[k-1][3][2];	// j=3;
	CA[1][k]  = Stif[k-1][2][1] + Stif[k-1][2][2];	// j=2;
	CB[0][k]  = Stif[k-1][0][1] + Stif[k-1][0][2];	// j=0;
	CB[1][k]  = Stif[k-1][1][1] + Stif[k-1][1][2];	// j=1;

	// elem[k]: triangle
	CA[1][k] += Stif[k][2][0] + Stif[k][2][1] + Stif[k][2][2];
	CB[1][k] += Stif[k][0][0] + Stif[k][0][1] + Stif[k][0][2];
	CB[2][k]  = Stif[k][1][0] + Stif[k][1][1] + Stif[k][1][2];
	clear_3d(N, (unsigned int) 4, Stif);
}

void US_Astfem_RSA::ComputeCoefMatrixMovingMeshL(double D, double sw2, double **CA, double **CB)
{
	unsigned int k;
	// compute local stiffness matrices
	StiffBase stfb0 ;
	double ***Stif; 	// Local stiffness matrix at each element
	initialize_3d(N, (unsigned int) 4, (unsigned int) 4, &Stif);
	double xd[4][2];		// coord for verices of quad elem
	// elem[0]: triangle
	xd[0][0] = x[0];
	xd[0][1] = 0.;
	xd[1][0] = x[1];	xd[1][1] = 0.;
	xd[2][0] = x[0];	xd[2][1] = af_params.dt;
	stfb0.CompLocalStif(3, xd, D, sw2, Stif[0]);

	// elem[k]: k=1..(N-2), quadrilateral
	for(k=1; k<N-1;k++)
	{ 	// loop for all elem
		xd[0][0] = x[k  ];	xd[0][1] = 0.;
		xd[1][0] = x[k+1];	xd[1][1] = 0.;
		xd[2][0] = x[k  ];	xd[2][1] = af_params.dt;
		xd[3][0] = x[k-1];	xd[3][1] = af_params.dt;
		stfb0.CompLocalStif(4, xd, D, sw2, Stif[k]);
	}

	// elem[N-1]: triangle
	xd[0][0] = x[N-1];	xd[0][1] = 0.;
	xd[1][0] = x[N-1];	xd[1][1] = af_params.dt;
	xd[2][0] = x[N-2];	xd[2][1] = af_params.dt;
	stfb0.CompLocalStif(3, xd, D, sw2, Stif[N-1]);


	// assembly coefficient matrices

	k = 0;
	CA[1][0] = Stif[0][2][0] + Stif[0][2][1] + Stif[0][2][2];
	CB[0][0] = Stif[0][0][0] + Stif[0][0][1] + Stif[0][0][2] ;
	CB[1][0] = Stif[0][1][0] + Stif[0][1][1] + Stif[0][1][2] ;

	CA[1][0]+= Stif[1][3][0] + Stif[1][3][3] ;
	CA[2][0] = Stif[1][2][0] + Stif[1][2][3] ;
	CB[1][0]+= Stif[1][0][0] + Stif[1][0][3] ;
	CB[2][0] = Stif[1][1][0] + Stif[1][1][3] ;

	for(k=1; k<N-2; k++)
	{ 	// loop for all elem
		// elem k:
		CA[0][k]  = Stif[k  ][3][1] + Stif[k  ][3][2];	// j=3;
		CA[1][k]  = Stif[k  ][2][1] + Stif[k  ][2][2];	// j=2;
		CB[0][k]  = Stif[k  ][0][1] + Stif[k  ][0][2];	// j=0;
		CB[1][k]  = Stif[k  ][1][1] + Stif[k  ][1][2];	// j=1;

		// elem k+1:
		CA[1][k] += Stif[k+1][3][0] + Stif[k+1][3][3];	// j=3;
		CA[2][k]  = Stif[k+1][2][0] + Stif[k+1][2][3];	// j=2;
		CB[1][k] += Stif[k+1][0][0] + Stif[k+1][0][3];	// j=0;
		CB[2][k]  = Stif[k+1][1][0] + Stif[k+1][1][3];	// j=1;
	}

	k = N-2;
	// elem k:
	CA[0][k]  = Stif[k  ][3][1] + Stif[k  ][3][2];	// j=3;
	CA[1][k]  = Stif[k  ][2][1] + Stif[k  ][2][2];	// j=2;
	CB[0][k]  = Stif[k  ][0][1] + Stif[k  ][0][2];	// j=0;
	CB[1][k]  = Stif[k  ][1][1] + Stif[k  ][1][2];	// j=1;

	// elem k+1: (triangle)
	CA[1][k] += Stif[k+1][2][0] + Stif[k+1][2][2];	// j=3;
	CA[2][k]  = Stif[k+1][1][0] + Stif[k+1][1][2];	// j=2;
	CB[1][k] += Stif[k+1][0][0] + Stif[k+1][0][2];	// j=0;


	k = N-1;
	// elem[k]: triangle
	CA[0][k]  = Stif[k  ][2][1] ;
	CA[1][k]  = Stif[k  ][1][1] ;
	CB[0][k]  = Stif[k  ][0][1] ;
	clear_3d(N, (unsigned int) 4, Stif);
}

void US_Astfem_RSA::tridiag(double *a, double *b, double *c, double *r, double *u)
{
	int j;
	double bet, *gam;
	gam = new double [N];
	if (b[0] == 0.0) printf("Error 1 in tridag");

	u[0] = r[0]/(bet = b[0]);
	for (j=1; j<(int) N; j++)
	{
		gam[j] = c[j-1]/bet;
		bet = b[j] - a[j] * gam[j];
		if (bet == 0.0)
		{
			printf("Error 2 in tridag");
		}
		u[j] = (r[j] - a[j] * u[j-1])/bet;
	}
	for (j=(N-2); j>=0; j--)
	{
		u[j] -= gam[j+1] * u[j+1];
	}
	delete [] gam;
}

void US_Astfem_RSA::initialize_3d(unsigned int val1, unsigned int val2, unsigned int val3, double ****matrix)
{
	unsigned int i, j, k;
	*matrix = new double **[val1];
	for (i=0; i<val1; i++)
	{
		(*matrix)[i] = new double *[val2];
		for (j=0; j<val2; j++)
		{
			(*matrix)[i][j] = new double [val3];
			for (k=0; k<val3; k++)
			{
				(*matrix)[i][j][k] = 0.0;
			}
		}
	}
}

void US_Astfem_RSA::initialize_2d(unsigned int val1, unsigned int val2, double ***matrix)
{
	unsigned int i, j;
	*matrix = new double *[val1];
	for (i=0; i<val1; i++)
	{
		(*matrix)[i] = new double [val2];
		for (j=0; j<val2; j++)
		{
			(*matrix)[i][j] = 0.0;
		}
	}
}

void US_Astfem_RSA::clear_3d(unsigned int val1, unsigned int val2, double ***matrix)
{
	unsigned int i, j;
	for (i=0; i<val1; i++)
	{
		for (j=0; j<val2; j++)
		{
			delete [] matrix[i][j];
		}
		delete [] matrix[i];
	}
	delete [] matrix;
}

void US_Astfem_RSA::clear_2d(unsigned int val1, double **matrix)
{
	unsigned int i;
	for (i=0; i<val1; i++)
	{
		delete [] matrix[i];
	}
	delete [] matrix;
}

void US_Astfem_RSA::GlobalStiff(vector <double> *xb, double **ca, double **cb,
double D, double sw2)
{

//
//  4: global stifness matrix
//

//function [CA, CB]=4(x, xb, dt, D, sw2)

	unsigned int i;
	double ***Stif=NULL;
	vector <double> vx;
	initialize_3d(N, 6, 2, &Stif);

// 1st elem
	vx.clear();
	vx.push_back(x[0]);
	vx.push_back(x[1]);
	vx.push_back(x[0]);
	vx.push_back(x[1]);
	vx.push_back(x[2]);
	vx.push_back((*xb)[1]);
	IntQT1(vx, D, sw2, Stif[0]);

	// elems in middle
	for (i=1; i<(N-2); i++)
	{
		vx.clear();
		vx.push_back(x[i-1]);
		vx.push_back(x[i]);
		vx.push_back(x[i+1]);
		vx.push_back(x[i]);
		vx.push_back(x[i+1]);
		vx.push_back(x[i+2]);
		vx.push_back((*xb)[i]);
		vx.push_back((*xb)[i+1]);
		IntQTm(vx, D, sw2, Stif[i]);
	}

// 2nd last elems
	vx.clear();
	vx.push_back(x[N-3]);
	vx.push_back(x[N-2]);
	vx.push_back(x[N-1]);
	vx.push_back(x[N-2]);
	vx.push_back(x[N-1]);
	vx.push_back((*xb)[N-2]);
	vx.push_back((*xb)[N-1]);

	IntQTn2(vx, D, sw2, Stif[N-2]);

	// last elems
	vx.clear();
	vx.push_back(x[N-2]);
	vx.push_back(x[N-1]);
	vx.push_back(x[N-1]);
	vx.push_back((*xb)[N-1]);
	IntQTn1(vx, D, sw2, Stif[N-1]);

	//
	// assembly into global stiffness matrix
	//

	ca[0][0] = 0.0;
	ca[1][0] = Stif[0][2][0];
	ca[2][0] = Stif[0][3][0];
	ca[3][0] = Stif[0][4][0];

	cb[0][0] = 0.0;
	cb[1][0] = 0.0;
	cb[2][0] = Stif[0][0][0];
	cb[3][0] = Stif[0][1][0];

	// i=2
	ca[0][1] = Stif[0][2][1];
	ca[1][1] = Stif[0][3][1] + Stif[1][3][0];
	ca[2][1] = Stif[0][4][1] + Stif[1][4][0];
	ca[3][1] =                 Stif[1][5][0];

	cb[0][1] = 0.0;
	cb[1][1] = Stif[0][0][1] + Stif[1][0][0];
	cb[2][1] = Stif[0][1][1] + Stif[1][1][0];
	cb[3][1] =                 Stif[1][2][0];

	// i: middle
	for (i=2; i<N-2; i++)
	{
		ca[0][i] = Stif[i-1][3][1];
		ca[1][i] = Stif[i-1][4][1] + Stif[i][3][0];
		ca[2][i] = Stif[i-1][5][1] + Stif[i][4][0];
		ca[3][i] =                   Stif[i][5][0];

		cb[0][i] = Stif[i-1][0][1];
		cb[1][i] = Stif[i-1][1][1] + Stif[i][0][0];
		cb[2][i] = Stif[i-1][2][1] + Stif[i][1][0];
		cb[3][i] =                   Stif[i][2][0];
	}

	// i=n
	i = N-2;
	ca[0][i] = Stif[i-1][3][1];
	ca[1][i] = Stif[i-1][4][1] + Stif[i][3][0];
	ca[2][i] = Stif[i-1][5][1] + Stif[i][4][0];
	ca[3][i] = 0.0;

	cb[0][i] = Stif[i-1][0][1];
	cb[1][i] = Stif[i-1][1][1] + Stif[i][0][0];
	cb[2][i] = Stif[i-1][2][1] + Stif[i][1][0];
	cb[3][i] =                   Stif[i][2][0];

	// i=n+1
	i = N-1;
	ca[0][i] = Stif[i-1][3][1];
	ca[1][i] = Stif[i-1][4][1] + Stif[i][2][0];
	ca[2][i] = 0.0;
	ca[3][i] = 0.0;

	cb[0][i] = Stif[i-1][0][1];
	cb[1][i] = Stif[i-1][1][1] + Stif[i][0][0];
	cb[2][i] = Stif[i-1][2][1] + Stif[i][1][0];
	cb[3][i] = 0.0;

/*****************

  FILE *outf=fopen("tmp.out1","w");
  fprintf(outf, "dt=%20.12e  diff=%20.12e  sw2=%20.12e \n", af_params.dt, D, sw2);
  for (unsigned int ii=0; ii<N; ii++) {
	 fprintf(outf, "x[%d]=%20.12e  xb=%20.12e \n", ii, x[ii], (*xb)[ii]);
  }
  fprintf(outf, "\n");

	for (unsigned int ii=0; ii<N; ii++)
	{
		fprintf(outf, "i=[%d]\n", ii);
		for (unsigned int kk=0; kk<2; kk++)
		{
			for (unsigned int jj=0; jj<6; jj++)
			{
		      fprintf(outf, "%12.5e ", Stif[ii][jj][kk] );
			}
		   fprintf(outf, "\n");
		}
		fprintf(outf, "\n");
	}
   fclose(outf);
   exit(1);
********************/


	clear_3d(N, 6, Stif);
}



//
// source: http://www.math.ntnu.no/num/nnm/Program/Numlibc/
//
void US_Astfem_RSA::DefineGaussian(unsigned int nGauss, double **Gs2)
{
	unsigned int i, j, k;
	double *Gs1, *w;
	Gs1 = new double [nGauss];
	w = new double [nGauss];

	switch (nGauss)
	{
		case 3:
		{
			Gs1[0] = -0.774596669241483; 	w[0] = 5.0/9.0;
			Gs1[1] = 0.0; 						w[1] = 8.0/9.0;
			Gs1[2] = 0.774596669241483; 	w[2] = 5.0/9.0;
			break;
		}
		case 5:
		{
         Gs1[0] = 0.906179845938664 ; 	w[0] = 0.236926885056189;
         Gs1[1] = 0.538469310105683 ; 	w[1] = 0.478628670499366;
         Gs1[2] = 0.000000000000000 ; 	w[2] = 0.568888888888889;
			Gs1[3] = -Gs1[1]; 				w[3] = w[1];
			Gs1[4] = -Gs1[0]; 				w[4] = w[0];
			break;
		}
		case 7:
		{
         Gs1[0] = 0.949107912342759 ;	w[0] = 0.129484966168870;
         Gs1[1] = 0.741531185599394 ;	w[1] = 0.279705391489277;
         Gs1[2] = 0.405845151377397 ;	w[2] = 0.381830050505119;
         Gs1[3] = 0.000000000000000 ;	w[3] = 0.417959183673469;
			Gs1[4] = -Gs1[2]; 				w[4] = w[2];
			Gs1[5] = -Gs1[1]; 				w[5] = w[1];
			Gs1[6] = -Gs1[0]; 				w[6] = w[0];
			break;
		}
		case 10:
		{
   		Gs1[0] = 0.973906528517172 ;	w[0] = 0.066671344308688;
      	Gs1[1] = 0.865063366688985 ;	w[1] = 0.149451349150581;
      	Gs1[2] = 0.679409568299024 ;	w[2] = 0.219086362515982;
      	Gs1[3] = 0.433395394129247 ;	w[3] = 0.269266719309996;
      	Gs1[4] = 0.148874338981631 ;	w[4] = 0.295524224714753;
			Gs1[5] = -Gs1[4]; 				w[5] = w[4];
			Gs1[6] = -Gs1[3]; 				w[6] = w[3];
			Gs1[7] = -Gs1[2]; 				w[7] = w[2];
			Gs1[8] = -Gs1[1]; 				w[8] = w[1];
			Gs1[9] = -Gs1[0]; 				w[9] = w[0];
			break;
      }
		default:
		{
			return;
		}
	}

	for (i=0; i<nGauss; i++)
	{
		for (j=0; j<nGauss; j++) 	// map to [0,1] x [0,1]
		{
			k = j + (i) * nGauss;
			Gs2[k][0] = (Gs1[i] + 1.0)/2.0;
			Gs2[k][1] = (Gs1[j] + 1.0)/2.0;
			Gs2[k][2] = w[i] * w[j]/4.0;
		}
	}
	delete [] w;
	delete [] Gs1;
}


//
// integrand for Lamm equation
//
double US_Astfem_RSA::Integrand(double x_gauss, double D, double sw2,
double u, double ux, double ut, double v, double vx)
{
	return (x_gauss * ut * v + D * x_gauss * ux * vx -
	sw2 * pow(x_gauss, 2.0) * u * vx);
}


//
// old version: perform integration on supp(test function) separately on left Q and right T
//
void US_Astfem_RSA::IntQT1(vector <double> vx, double D, double sw2, double **Stif)
{
	// element to define basis functions
	//
	unsigned int npts, i, k;
	double x_gauss, y_gauss, dval;
   double hh, slope, xn1, phiC, phiCx;
	vector <double> Lx, Ly, Rx, Ry, Qx, Qy, Tx, Ty;
	double *phiL, *phiLx, *phiLy, *phiR, *phiRx, *phiRy;
	double **StifL=NULL, **StifR=NULL, **Lam=NULL, DJac;
	Lx.clear();
	Ly.clear();
	Rx.clear();
	Ry.clear();
	Qx.clear();
	Qy.clear();
	Tx.clear();
	Ty.clear();
   phiL  = new double [3];
   phiLx = new double [3];
   phiLy = new double [3];
   phiR  = new double [4];
   phiRx = new double [4];
   phiRy = new double [4];

	// elements for define the trial function phi
	Lx.push_back(vx[0]); 	// vertices of left Triangle
	Lx.push_back(vx[3]);
	Lx.push_back(vx[2]);

	Ly.push_back(0.0);
	Ly.push_back(af_params.dt);
	Ly.push_back(af_params.dt);

	Rx.push_back(vx[0]);	// vertices of Q on right quadrilateral
	Rx.push_back(vx[1]);
	Rx.push_back(vx[4]);
	Rx.push_back(vx[3]);

	Ry.push_back(0.0);
	Ry.push_back(0.0);
	Ry.push_back(af_params.dt);
	Ry.push_back(af_params.dt);

	initialize_2d(3, 2, &StifL);
	initialize_2d(4, 2, &StifR);
   hh = vx[3] - vx[2];
   slope = (vx[3] - vx[5])/af_params.dt;
	npts = 28;
	initialize_2d(npts, 4, &Lam);
	DefineFkp(npts, Lam);

   //
   // integration over element Q (a triangle):
   //
	Qx.push_back(vx[0]);	// vertices of Q on left
	Qx.push_back(vx[3]);
	Qx.push_back(vx[2]);

	Qy.push_back(0.0);
	Qy.push_back(af_params.dt);
	Qy.push_back(af_params.dt);	// vertices of left T

	for (k=0; k<npts; k++)
	{
		x_gauss = Lam[k][0] * Qx[0] + Lam[k][1] * Qx[1] + Lam[k][2] * Qx[2];
		y_gauss = Lam[k][0] * Qy[0] + Lam[k][1] * Qy[1] + Lam[k][2] * Qy[2];
		DJac = 2.0 * AreaT(&Qx, &Qy);

      xn1 = x_gauss + slope * ( af_params.dt - y_gauss );	// trace-forward point at t_n+1 from (x_g, y_g)

		//
		// find phi, phi_x, phi_y on L and C at (x,y)
		//

		BasisTR(Lx, Ly, x_gauss, y_gauss, phiL, phiLx, phiLy);
      phiC  = ( xn1 - vx[2] )/hh;		// hat function on t_n+1, =1 at vx[3]; =0 at vx[2]
      phiCx = 1./hh;

		for (i=0; i<3; i++)
		{
			dval = Integrand(x_gauss, D, sw2, phiL[i], phiLx[i], phiLy[i], 1.-phiC, -phiCx );
			StifL[i][0] += Lam[k][3] * DJac * dval;

			dval = Integrand(x_gauss, D, sw2, phiL[i], phiLx[i], phiLy[i], phiC, phiCx );
			StifL[i][1] += Lam[k][3] * DJac * dval;
		}
	}

	//
	// integration over T:
	//
	Tx.push_back(vx[0]);	// vertices of T on right
	Tx.push_back(vx[5]);
	Tx.push_back(vx[3]);

	Ty.push_back(0.0);
	Ty.push_back(0.0);
	Ty.push_back(af_params.dt);

	for (k=0; k<npts; k++)
	{
		x_gauss = Lam[k][0] * Tx[0] + Lam[k][1] * Tx[1] + Lam[k][2] * Tx[2];
		y_gauss = Lam[k][0] * Ty[0] + Lam[k][1] * Ty[1] + Lam[k][2] * Ty[2];
		DJac = 2.0 * AreaT(&Tx, &Ty);

      if(DJac<1.e-22) break;

      xn1 = x_gauss + slope * ( af_params.dt - y_gauss );	// trace-forward point at t_n+1 from (x_g, y_g)

		//
		// find phi, phi_x, phi_y on R and C at (x,y)
		//

		BasisQR(Rx, x_gauss, y_gauss, phiR, phiRx, phiRy);
      phiC  = ( xn1 - vx[2] )/hh;		// hat function on t_n+1, =1 at vx[3]; =0 at vx[2]
      phiCx = 1./hh;

		for (i=0; i<4; i++)
		{
			dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i], 1.-phiC, -phiCx);
			StifR[i][0] += Lam[k][3] * DJac * dval;

			dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i], phiC, phiCx);
			StifR[i][1] += Lam[k][3] * DJac * dval;
		}
	}

	clear_2d(npts, Lam);

	for (i=0; i<2; i++)
	{
		Stif[0][i] = StifL[0][i] + StifR[0][i];
		Stif[1][i] =               StifR[1][i];
		Stif[2][i] = StifL[2][i];
		Stif[3][i] = StifL[1][i] + StifR[3][i];
		Stif[4][i] =               StifR[2][i];
	}

	delete [] phiR;
	delete [] phiRx;
	delete [] phiRy;
	delete [] phiL;
	delete [] phiLx;
	delete [] phiLy;

	clear_2d(3, StifL);
	clear_2d(4, StifR);


}

void US_Astfem_RSA::IntQTm(vector <double> vx, double D, double sw2, double **Stif)
{
	// element to define basis functions
	//
	unsigned int npts, i, k;
	double x_gauss, y_gauss, dval;
	vector <double> Lx, Ly, Cx, Cy, Rx, Ry, Qx, Qy, Tx, Ty;
	double *phiR, *phiRx, *phiRy;
	double **StifL=NULL, **StifR=NULL, **Lam=NULL, DJac;
	double *phiL, *phiLx, *phiLy, *phiCx, *phiCy, *phiC;
	double **Gs=NULL;
	phiL = new double [4];
	phiLx = new double [4];
	phiLy = new double [4];
	phiCx = new double [4];
	phiCy = new double [4];
	phiC = new double [4];
	phiR = new double [4];
	phiRx = new double [4];
	phiRy = new double [4];
	Lx.clear();
	Ly.clear();
	Cx.clear();
	Cy.clear();
	Rx.clear();
	Ry.clear();
	Qx.clear();
	Qy.clear();
	Tx.clear();
	Ty.clear();

	Lx.push_back(vx[0]);
	Lx.push_back(vx[1]);
	Lx.push_back(vx[4]);
	Lx.push_back(vx[3]);

	Ly.push_back(0.0);
	Ly.push_back(0.0);
	Ly.push_back(af_params.dt);
	Ly.push_back(af_params.dt); 			// vertices of left T

	Cx.push_back(vx[6]);
	Cx.push_back(vx[7]);
	Cx.push_back(vx[4]);
	Cx.push_back(vx[3]);

	Cy.push_back(0.0);
	Cy.push_back(0.0);
	Cy.push_back(af_params.dt);
	Cy.push_back(af_params.dt);

	Rx.push_back(vx[1]);	// vertices of Q on right
	Rx.push_back(vx[2]);
	Rx.push_back(vx[5]);
	Rx.push_back(vx[4]);

	Ry.push_back(0.0);
	Ry.push_back(0.0);
	Ry.push_back(af_params.dt);
	Ry.push_back(af_params.dt);

	initialize_2d(4, 2, &StifL);
	initialize_2d(4, 2, &StifR);

   //
   // integration over element Q :
   //
	Qx.push_back(vx[6]);	// vertices of Q on right
	Qx.push_back(vx[1]);
	Qx.push_back(vx[4]);
	Qx.push_back(vx[3]);

	Qy.push_back(0.0);
	Qy.push_back(0.0);
	Qy.push_back(af_params.dt);
	Qy.push_back(af_params.dt);	// vertices of left T

	npts = 5 * 5;
	initialize_2d(npts, 3, &Gs);
	DefineGaussian(5, Gs);

	double psi[4], psi1[4], psi2[4], jac[4];
	for (k=0; k<npts; k++)
	{
		BasisQS(Gs[k][0], Gs[k][1], psi, psi1, psi2);

		x_gauss = 0.0;
		y_gauss = 0.0;
		for (i=0; i<4; i++)
		{
			jac[i] = 0.0;
		}
		for (i=0; i<4; i++)
		{
			x_gauss += psi[i] * Qx[i];
			y_gauss += psi[i] * Qy[i];
			jac[0] += Qx[i] * psi1[i];
			jac[1] += Qx[i] * psi2[i];
			jac[2] += Qy[i] * psi1[i];
			jac[3] += Qy[i] * psi2[i];
		}

		DJac = jac[0] * jac[3] - jac[1] * jac[2];

		//
		// find phi, phi_x, phi_y on L and C at (x,y)
		//

		BasisQR(Lx, x_gauss, y_gauss, phiL, phiLx, phiLy);
		BasisQR(Cx, x_gauss, y_gauss, phiC, phiCx, phiCy);
		for (i=0; i<4; i++)
		{
			dval = Integrand(x_gauss, D, sw2, phiL[i], phiLx[i], phiLy[i],
 											phiC[0] + phiC[3], phiCx[0] + phiCx[3]);
			StifL[i][0] += Gs[k][2] * DJac * dval;

			dval = Integrand(x_gauss, D, sw2, phiL[i], phiLx[i], phiLy[i],
											phiC[1] + phiC[2], phiCx[1] + phiCx[2]);
			StifL[i][1] += Gs[k][2] * DJac * dval;
		}
	}
	clear_2d(npts, Gs);

	//
	// integration over T:
	//
	Tx.push_back(vx[1]);	// vertices of T on right
	Tx.push_back(vx[7]);
	Tx.push_back(vx[4]);

	Ty.push_back(0.0);
	Ty.push_back(0.0);
	Ty.push_back(af_params.dt);

	npts = 28;
	initialize_2d(npts, 4, &Lam);
	DefineFkp(npts, Lam);

	for (k=0; k<npts; k++)
	{
		x_gauss = Lam[k][0] * Tx[0] + Lam[k][1] * Tx[1] + Lam[k][2] * Tx[2];
		y_gauss = Lam[k][0] * Ty[0] + Lam[k][1] * Ty[1] + Lam[k][2] * Ty[2];
		DJac = 2.0 * AreaT(&Tx, &Ty);

      if(DJac<1.e-22) break;

		//
		// find phi, phi_x, phi_y on R and C at (x,y)
		//

		BasisQR(Rx, x_gauss, y_gauss, phiR, phiRx, phiRy);
		BasisQR(Cx, x_gauss, y_gauss, phiC, phiCx, phiCy);

		for (i=0; i<4; i++)
		{
			dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i],
			       						phiC[0] + phiC[3], phiCx[0] + phiCx[3]);
			StifR[i][0] += Lam[k][3] * DJac * dval;

			dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i],
					 						phiC[1] + phiC[2], phiCx[1] + phiCx[2]);
			StifR[i][1] += Lam[k][3] * DJac * dval;
		}
	}
	clear_2d(npts, Lam);


	for (i=0; i<2; i++)
	{
		Stif[0][i] = StifL[0][i];
		Stif[1][i] = StifL[1][i] + StifR[0][i];
		Stif[2][i] = 					StifR[1][i];
		Stif[3][i] = StifL[3][i];
		Stif[4][i] = StifL[2][i] + StifR[3][i];
		Stif[5][i] = 					StifR[2][i];
	}
	delete [] phiR;
	delete [] phiRx;
	delete [] phiRy;
	delete [] phiL;
	delete [] phiLx;
	delete [] phiLy;
	delete [] phiCx;
	delete [] phiCy;
	delete [] phiC;

	clear_2d(3, StifL);
	clear_2d(4, StifR);
}


void US_Astfem_RSA::IntQTn2(vector <double> vx, double D, double sw2, double **Stif)
{
	// element to define basis functions
	//
	unsigned int npts, i, k;
	double x_gauss, y_gauss, dval;
	vector <double> Lx, Ly, Cx, Cy, Rx, Ry, Qx, Qy, Tx, Ty;
	double *phiR, *phiRx, *phiRy;
	double **StifL=NULL, **StifR=NULL, **Lam=NULL, DJac;
	double *phiL, *phiLx, *phiLy, *phiCx, *phiCy, *phiC;
	double **Gs=NULL;
	phiL = new double [4];
	phiLx = new double [4];
	phiLy = new double [4];
	phiCx = new double [4];
	phiCy = new double [4];
	phiC = new double [4];
	phiR = new double [3];
	phiRx = new double [3];
	phiRy = new double [3];
	Lx.clear();
	Ly.clear();
	Cx.clear();
	Cy.clear();
	Rx.clear();
	Ry.clear();
	Qx.clear();
	Qy.clear();
	Tx.clear();
	Ty.clear();

	Lx.push_back(vx[0]);
	Lx.push_back(vx[1]);
	Lx.push_back(vx[4]);
	Lx.push_back(vx[3]);

	Ly.push_back(0.0);
	Ly.push_back(0.0);
	Ly.push_back(af_params.dt);
	Ly.push_back(af_params.dt); 			// vertices of left T

	Cx.push_back(vx[5]);
	Cx.push_back(vx[6]);
	Cx.push_back(vx[4]);
	Cx.push_back(vx[3]);

	Cy.push_back(0.0);
	Cy.push_back(0.0);
	Cy.push_back(af_params.dt);
	Cy.push_back(af_params.dt);

	Rx.push_back(vx[1]);	// vertices of Q on right
	Rx.push_back(vx[2]);
	Rx.push_back(vx[4]);

	Ry.push_back(0.0);
	Ry.push_back(0.0);
	Ry.push_back(af_params.dt);

	initialize_2d(4, 2, &StifL);
	initialize_2d(4, 2, &StifR);

   //
   // integration over element Q
   //
	Qx.push_back(vx[5]);	// vertices of Q on right
	Qx.push_back(vx[1]);
	Qx.push_back(vx[4]);
	Qx.push_back(vx[3]);

	Qy.push_back(0.0);
	Qy.push_back(0.0);
	Qy.push_back(af_params.dt);
	Qy.push_back(af_params.dt);

	npts = 5 * 5;
	initialize_2d(npts, 3, &Gs);
	DefineGaussian(5, Gs);

	double psi[4], psi1[4], psi2[4], jac[4];
	for (k=0; k<npts; k++)
	{
		BasisQS(Gs[k][0], Gs[k][1], psi, psi1, psi2);

		x_gauss = 0.0;
		y_gauss = 0.0;
		for (i=0; i<4; i++)
		{
			jac[i] = 0.0;
		}
		for (i=0; i<4; i++)
		{
			x_gauss += psi[i] * Qx[i];
			y_gauss += psi[i] * Qy[i];
			jac[0] += Qx[i] * psi1[i];
			jac[1] += Qx[i] * psi2[i];
			jac[2] += Qy[i] * psi1[i];
			jac[3] += Qy[i] * psi2[i];
		}

		DJac = jac[0] * jac[3] - jac[1] * jac[2];

		//
		// find phi, phi_x, phi_y on L and C at (x,y)
		//

		BasisQR(Lx, x_gauss, y_gauss, phiL, phiLx, phiLy);
		BasisQR(Cx, x_gauss, y_gauss, phiC, phiCx, phiCy);
		for (i=0; i<4; i++)
		{
			dval = Integrand(x_gauss, D, sw2, phiL[i], phiLx[i], phiLy[i],
											phiC[0] + phiC[3], phiCx[0] + phiCx[3]);
			StifL[i][0] += Gs[k][2] * DJac * dval;

			dval = Integrand(x_gauss, D, sw2, phiL[i], phiLx[i], phiLy[i],
											phiC[1] + phiC[2], phiCx[1] + phiCx[2]);
			StifL[i][1] += Gs[k][2] * DJac * dval;
		}
	}
	clear_2d(npts, Gs);

	//
	// integration over T:
	//
	Tx.push_back(vx[1]);	// vertices of T on right
	Tx.push_back(vx[6]);
	Tx.push_back(vx[4]);

	Ty.push_back(0.0);
	Ty.push_back(0.0);
	Ty.push_back(af_params.dt);

	npts = 28;
	initialize_2d(npts, 4, &Lam);
	DefineFkp(npts, Lam);

	for (k=0; k<npts; k++)
	{
		x_gauss = Lam[k][0] * Tx[0] + Lam[k][1] * Tx[1] + Lam[k][2] * Tx[2];
		y_gauss = Lam[k][0] * Ty[0] + Lam[k][1] * Ty[1] + Lam[k][2] * Ty[2];
		DJac = 2.0 * AreaT(&Tx, &Ty);

      if(DJac<1.e-22) break;

		//
		// find phi, phi_x, phi_y on R and C at (x,y)
		//

		BasisQR(Cx, x_gauss, y_gauss, phiC, phiCx, phiCy);
		BasisTR(Rx, Ry, x_gauss, y_gauss, phiR, phiRx, phiRy);

		for (i=0; i<3; i++)
		{
			dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i],
											phiC[0] + phiC[3], phiCx[0] + phiCx[3]);
			StifR[i][0] += Lam[k][3] * DJac * dval;

			dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i],
											phiC[1] + phiC[2], phiCx[1] + phiCx[2]);
			StifR[i][1] += Lam[k][3] * DJac * dval;
		}
	}
	clear_2d(npts, Lam);

	for (i=0; i<2; i++)
	{
		Stif[0][i] = StifL[0][i];
		Stif[1][i] = StifL[1][i] + StifR[0][i] ;
		Stif[2][i] = 					StifR[1][i];
		Stif[3][i] = StifL[3][i];
		Stif[4][i] = StifL[2][i] + StifR[2][i];
	}
	delete [] phiR;
	delete [] phiRx;
	delete [] phiRy;
	delete [] phiL;
	delete [] phiLx;
	delete [] phiLy;
	delete [] phiCx;
	delete [] phiCy;
	delete [] phiC;

	clear_2d(3, StifL);
	clear_2d(4, StifR);
}

void US_Astfem_RSA::IntQTn1(vector <double> vx, double D, double sw2, double **Stif)
{
	// element to define basis functions
	//
	unsigned int npts, i, k;
	double x_gauss, y_gauss, dval;
	vector <double> Lx, Ly, Tx, Ty;
	double **StifR=NULL, **Lam=NULL, DJac;
	double *phiL, *phiLx, *phiLy;
	phiL = new double [4];
	phiLx = new double [4];
	phiLy = new double [4];
	Lx.clear();
	Ly.clear();
	Tx.clear();
	Ty.clear();

	Lx.push_back(vx[0]);
	Lx.push_back(vx[1]);
	Lx.push_back(vx[2]);

	Ly.push_back(0.0);
	Ly.push_back(0.0);
	Ly.push_back(af_params.dt);

	initialize_2d(4, 2, &StifR);

	//
	// integration over T:
	//
	Tx.push_back(vx[3]);	// vertices of T on right
	Tx.push_back(vx[1]);
	Tx.push_back(vx[2]);

	Ty.push_back(0.0);
	Ty.push_back(0.0);
	Ty.push_back(af_params.dt);

	npts = 28;
	initialize_2d(npts, 4, &Lam);
	DefineFkp(npts, Lam);

	for (k=0; k<npts; k++)
	{
		x_gauss = Lam[k][0] * Tx[0] + Lam[k][1] * Tx[1] + Lam[k][2] * Tx[2];
		y_gauss = Lam[k][0] * Ty[0] + Lam[k][1] * Ty[1] + Lam[k][2] * Ty[2];
		DJac = 2.0 * AreaT(&Tx, &Ty);

      if(DJac<1.e-22) break;
		//
		// find phi, phi_x, phi_y on R and C at (x,y)
		//

		BasisTR(Lx, Ly, x_gauss, y_gauss, phiL, phiLx, phiLy);

		for (i=0; i<3; i++)
		{
			dval = Integrand(x_gauss, D, sw2, phiL[i], phiLx[i], phiLy[i], 1.0, 0.0);
			StifR[i][0] += Lam[k][3] * DJac * dval;
		}
	}
	clear_2d(npts, Lam);

	for (i=0; i<2; i++)
	{
		Stif[0][i] = StifR[0][i];
		Stif[1][i] = StifR[1][i];
		Stif[2][i] = StifR[2][i];
	}
	delete [] phiL;
	delete [] phiLx;
	delete [] phiLy;

	clear_2d(4, StifR);
}


void US_Astfem_RSA::QuadSolver(double *ai, double *bi, double *ci, double *di, double *cr, double *solu)
{
//
// solve Quad-diagonal system [a_i, *b_i*, c_i, d_i]*[x]=[r_i]
// b_i ar e on the main diagonal line
//
// test
//	n=100; a=-1+rand(100,1); b=2+rand(200,1); c=-0.7*rand(100,1); d=-0.3*rand(100,1);
//	xs=rand(100,1);
//	r(1)=b(1)*xs(1)+c(1)*xs(2)+d(1)*xs(3);
//	for i=2:n-2,
//	r(i)=a(i)*xs(i-1)+b(i)*xs(i)+c(i)*xs(i+1)+d(i)*xs(i+2);
//	end;
//	i=n-1; r(i)=a(i)*xs(i-1)+b(i)*xs(i)+c(i)*xs(i+1);
//	i=n;	r(i)=a(i)*xs(i-1)+b(i)*xs(i);
//

	unsigned int i;
	double tmp;
	vector<double> ca, cb, cc, cd;
	ca.clear();
	cb.clear();
	cc.clear();
	cd.clear();
	for (i=0; i<N; i++)
	{
		ca.push_back( ai[i] );
		cb.push_back( bi[i] );
		cc.push_back( ci[i] );
		cd.push_back( di[i] );
	}

	for (i=1; i<=N-2; i++)
	{
		tmp = ca[i]/cb[i-1];
		cb[i] = cb[i]-cc[i-1]*tmp;
		cc[i] = cc[i]-cd[i-1]*tmp;
		cr[i] = cr[i]-cr[i-1]*tmp;
	}
   i=N-1;
		tmp = ca[i]/cb[i-1];
		cb[i] = cb[i]-cc[i-1]*tmp;
		cr[i] = cr[i]-cr[i-1]*tmp;


	solu[N-1] = cr[N-1] / cb[N-1];
	solu[N-2] = (cr[N-2] - cc[N-2] * solu[N-1]) / cb[N-2];
	i = N - 2;
	do
	{
		i--;
		solu[i] = (cr[i] - cc[i] * solu[i+1] - cd[i] * solu[i+2]) / cb[i];
	} while (i != 0);
}


//
// ************* ELLAM ***********
//
//

void US_Astfem_RSA::GlobalStiff_ellam(vector <double> *xb, double **ca, double **cb,
											     double D, double sw2)
{
//
//  4: global stifness matrix
//

//function [CA, CB]=4(x, xb, dt, D, sw2)

	unsigned int i;
	double ***Stif=NULL;
	vector <double> vx;
	initialize_3d(N, 6, 2, &Stif);

// 1st elem
	vx.clear();
	vx.push_back(x[0]);
	vx.push_back(x[1]);
	vx.push_back(x[0]);
	vx.push_back(x[1]);
	vx.push_back((*xb)[1]);
	IntQT1_ellam(vx, D, sw2, Stif[0]);

	// elems in middle
	for (i=1; i<=(N-2); i++)
	{
		vx.clear();
		vx.push_back(x[i-1]);
		vx.push_back(x[i]);
		vx.push_back(x[i+1]);
		vx.push_back(x[i-1]);
		vx.push_back(x[i]);
		vx.push_back(x[i+1]);
		vx.push_back((*xb)[i]);
		vx.push_back((*xb)[i+1]);
		IntQTm_ellam(vx, D, sw2, Stif[i]);
	}


	// last elems
	vx.clear();
	vx.push_back(x[N-2]);
	vx.push_back(x[N-1]);
	vx.push_back(x[N-2]);
	vx.push_back(x[N-1]);
	vx.push_back((*xb)[N-1]);
	IntQTn1_ellam(vx, D, sw2, Stif[N-1]);

	//
	// assembly into global stiffness matrix
	//
	ca[0][0] = 0.0;
	ca[1][0] = 0.0;
	ca[2][0] = Stif[0][2][0];
	ca[3][0] = Stif[0][3][0];

	cb[0][0] = 0.0;
	cb[1][0] = 0.0;
	cb[2][0] = Stif[0][0][0];
	cb[3][0] = Stif[0][1][0];

	// i=1
	ca[0][1] = 0.0;
	ca[1][1] = Stif[0][2][1] + Stif[1][3][0];
	ca[2][1] = Stif[0][3][1] + Stif[1][4][0];
	ca[3][1] =                 Stif[1][5][0];

	cb[0][1] = 0.0;
	cb[1][1] = Stif[0][0][1] + Stif[1][0][0];
	cb[2][1] = Stif[0][1][1] + Stif[1][1][0];
	cb[3][1] =                 Stif[1][2][0];

	// i: middle
	for (i=2; i<=N-2; i++)
	{
		ca[0][i] = Stif[i-1][3][1];
		ca[1][i] = Stif[i-1][4][1] + Stif[i][3][0];
		ca[2][i] = Stif[i-1][5][1] + Stif[i][4][0];
		ca[3][i] =                   Stif[i][5][0];

		cb[0][i] = Stif[i-1][0][1];
		cb[1][i] = Stif[i-1][1][1] + Stif[i][0][0];
		cb[2][i] = Stif[i-1][2][1] + Stif[i][1][0];
		cb[3][i] =                   Stif[i][2][0];
	}

	// i=n-1
	i = N-1;
	ca[0][i] = Stif[i-1][3][1];
	ca[1][i] = Stif[i-1][4][1] + Stif[i][2][0];
	ca[2][i] = Stif[i-1][5][1] + Stif[i][3][0];
	ca[3][i] = 0.0;

	cb[0][i] = Stif[i-1][0][1];
	cb[1][i] = Stif[i-1][1][1] + Stif[i][0][0];
	cb[2][i] = Stif[i-1][2][1] + Stif[i][1][0];
	cb[3][i] = 0.0;

	clear_3d(N, 6, Stif);
}


void US_Astfem_RSA::IntQT1_ellam(vector <double> vx, double D, double sw2, double **Stif)
{
	// element to define basis functions
	//
	unsigned int npts, i, k;
	double x_gauss, y_gauss, dval;
	vector <double>  Rx, Ry, Qx, Qy;
	double **StifR=NULL, DJac;
	double *phiR, *phiRx, *phiRy;
   double hh, slope, xn1, phiC, phiCx;
	phiR = new double [4];
	phiRx = new double [4];
	phiRy = new double [4];

	Rx.clear();
	Ry.clear();
	Rx.push_back(vx[0]);	// vertices of Q on right
	Rx.push_back(vx[1]);
	Rx.push_back(vx[3]);
	Rx.push_back(vx[2]);

	Ry.push_back(0.0);
	Ry.push_back(0.0);
	Ry.push_back(af_params.dt);
	Ry.push_back(af_params.dt);

	initialize_2d(4, 2, &StifR);
   hh = vx[3] - vx[2];
   slope = (vx[3] - vx[4])/af_params.dt;

   //
   // integration over quadrilateral element Q :
   //
   if( (vx[1]-vx[4])/(vx[1]-vx[0]) <1.e-3 ) 		// Q_{0,4,3,2} is almost degenerated into a triangle
   {
	    // elements for integration
	    //
	    Qx.clear();
	    Qy.clear();
	    Qx.push_back(vx[0]);	// vertices of Q on right
	    Qx.push_back(vx[3]);
	    Qx.push_back(vx[2]);
	    Qy.push_back(0.0);
	    Qy.push_back(af_params.dt);
	    Qy.push_back(af_params.dt);

       double **Lam;
	    npts = 28;
	    initialize_2d(npts, 4, &Lam);
	    DefineFkp(npts, Lam);

	    for (k=0; k<npts; k++)
	    {
		    x_gauss = Lam[k][0] * Qx[0] + Lam[k][1] * Qx[1] + Lam[k][2] * Qx[2];
		    y_gauss = Lam[k][0] * Qy[0] + Lam[k][1] * Qy[1] + Lam[k][2] * Qy[2];
		    DJac = 2.0 * AreaT(&Qx, &Qy);

		    //
		    // find phi, phi_x, phi_y on R and C at (x,y)
		    //

	     	 BasisQR(Rx, x_gauss, y_gauss, phiR, phiRx, phiRy);

          xn1 = x_gauss + slope * ( af_params.dt - y_gauss );	// trace-forward point at t_n+1 from (x_g, y_g)
          phiC  = ( xn1 - vx[2] )/hh;		// hat function on t_n+1, =1 at vx[3]; =0 at vx[2]
          phiCx = 1./hh;

		    for (i=0; i<4; i++)
		    {
			    dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i], 1.-phiC, -phiCx);
			    StifR[i][0] += Lam[k][3] * DJac * dval;

			    dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i],    phiC,  phiCx);
			    StifR[i][1] += Lam[k][3] * DJac * dval;
		    }
      }
	   clear_2d(npts, Lam);

   }
	else
	{	// Q_{0,4,3,2} is non-degenerate
		// elements for integration
		//
		Qx.clear();
		Qy.clear();
		Qx.push_back(vx[0]);	// vertices of Q on right
		Qx.push_back(vx[4]);
		Qx.push_back(vx[3]);
		Qx.push_back(vx[2]);

		Qy.push_back(0.0);
		Qy.push_back(0.0);
		Qy.push_back(af_params.dt);
		Qy.push_back(af_params.dt);

		double **Gs=NULL;
		npts = 5 * 5;
		initialize_2d(npts, 3, &Gs);
		DefineGaussian(5, Gs);

		double psi[4], psi1[4], psi2[4], jac[4];
		for (k=0; k<npts; k++)
		{
			BasisQS(Gs[k][0], Gs[k][1], psi, psi1, psi2);
			x_gauss = 0.0;
			y_gauss = 0.0;
			for (i=0; i<4; i++)
			{
				jac[i] = 0.0;
			}
			for (i=0; i<4; i++)
			{
				x_gauss += psi[i] * Qx[i];
				y_gauss += psi[i] * Qy[i];
				jac[0] += Qx[i] * psi1[i];
				jac[1] += Qx[i] * psi2[i];
				jac[2] += Qy[i] * psi1[i];
				jac[3] += Qy[i] * psi2[i];
			}
			DJac = jac[0] * jac[3] - jac[1] * jac[2];
			//
			// find phi, phi_x, phi_y on L and C at (x,y)
			//
			BasisQR(Rx, x_gauss, y_gauss, phiR, phiRx, phiRy);
			xn1 = x_gauss + slope * ( af_params.dt - y_gauss );	// trace-forward point at t_n+1 from (x_g, y_g)
			phiC  = ( xn1 - vx[2] )/hh;		// hat function on t_n+1, =1 at vx[3]; =0 at vx[2]
			phiCx = 1./hh;
			for (i=0; i<4; i++)
			{
				dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i], 1.-phiC, -phiCx);
				StifR[i][0] += Gs[k][2] * DJac * dval;
				dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i],    phiC,  phiCx);
				StifR[i][1] += Gs[k][2] * DJac * dval;
			}
		}
		clear_2d(npts, Gs);
	}
	for (i=0; i<2; i++)
	{
		Stif[0][i] = StifR[0][i];
		Stif[1][i] = StifR[1][i];
		Stif[2][i] = StifR[3][i];
		Stif[3][i] = StifR[2][i];
	}
	delete [] phiR;
	delete [] phiRx;
	delete [] phiRy;
	clear_2d(4, StifR);
}

void US_Astfem_RSA::IntQTm_ellam(vector <double> vx, double D, double sw2, double **Stif)
{
	// element to define basis functions
	//
	unsigned int npts, i, k;
	double x_gauss, y_gauss, dval;
	vector <double> Lx, Ly, Cx, Cy, Rx, Ry, Qx, Qy, Tx, Ty;
	double *phiR, *phiRx, *phiRy;
	double **StifL=NULL, **StifR=NULL, **Lam=NULL, DJac;
	double *phiL, *phiLx, *phiLy, *phiCx, *phiCy, *phiC;
	double **Gs=NULL;
	phiL = new double [4];
	phiLx = new double [4];
	phiLy = new double [4];
	phiCx = new double [4];
	phiCy = new double [4];
	phiC = new double [4];
	phiR = new double [4];
	phiRx = new double [4];
	phiRy = new double [4];
	Lx.clear();
	Ly.clear();
	Cx.clear();
	Cy.clear();
	Rx.clear();
	Ry.clear();
	Qx.clear();
	Qy.clear();
	Tx.clear();
	Ty.clear();

	Lx.push_back(vx[0]);
	Lx.push_back(vx[1]);
	Lx.push_back(vx[4]);
	Lx.push_back(vx[3]);

	Ly.push_back(0.0);
	Ly.push_back(0.0);
	Ly.push_back(af_params.dt);
	Ly.push_back(af_params.dt); 			// vertices of left T

	Cx.push_back(vx[6]);
	Cx.push_back(vx[7]);
	Cx.push_back(vx[5]);
	Cx.push_back(vx[4]);

	Cy.push_back(0.0);
	Cy.push_back(0.0);
	Cy.push_back(af_params.dt);
	Cy.push_back(af_params.dt);

	Rx.push_back(vx[1]);	// vertices of Q on right
	Rx.push_back(vx[2]);
	Rx.push_back(vx[5]);
	Rx.push_back(vx[4]);

	Ry.push_back(0.0);
	Ry.push_back(0.0);
	Ry.push_back(af_params.dt);
	Ry.push_back(af_params.dt);


	initialize_2d(4, 2, &StifL);
	initialize_2d(4, 2, &StifR);

	//
	// integration over triangle T:
	//
	Tx.push_back(vx[6]);	// vertices of T on left
	Tx.push_back(vx[1]);
	Tx.push_back(vx[4]);

	Ty.push_back(0.0);
	Ty.push_back(0.0);
	Ty.push_back(af_params.dt);

	npts = 28;
	initialize_2d(npts, 4, &Lam);
	DefineFkp(npts, Lam);

	for (k=0; k<npts; k++)
	{
		x_gauss = Lam[k][0] * Tx[0] + Lam[k][1] * Tx[1] + Lam[k][2] * Tx[2];
		y_gauss = Lam[k][0] * Ty[0] + Lam[k][1] * Ty[1] + Lam[k][2] * Ty[2];
		DJac = 2.0 * AreaT(&Tx, &Ty);

      if(DJac<1.e-22) break;

		//
		// find phi, phi_x, phi_y on R and C at (x,y)
		//

		BasisQR(Lx, x_gauss, y_gauss, phiL, phiLx, phiLy);
		BasisQR(Cx, x_gauss, y_gauss, phiC, phiCx, phiCy);

		for (i=0; i<4; i++)
		{
			dval = Integrand(x_gauss, D, sw2, phiL[i], phiLx[i], phiLy[i],
			       						phiC[0] + phiC[3], phiCx[0] + phiCx[3]);
			StifL[i][0] += Lam[k][3] * DJac * dval;

			dval = Integrand(x_gauss, D, sw2, phiL[i], phiLx[i], phiLy[i],
					 						phiC[1] + phiC[2], phiCx[1] + phiCx[2]);
			StifL[i][1] += Lam[k][3] * DJac * dval;
		}
	}
	clear_2d(npts, Lam);

   //
   // integration over quadrilateral element Q :
   //
   if( (vx[7]-vx[1])/(vx[2]-vx[1]) <1.e-3 ) 		// Q_{1,7,5,4} is almost degenerated into a triangle
   {
	    Qx.push_back(vx[1]);	// vertices of Q on right
	    Qx.push_back(vx[5]);
	    Qx.push_back(vx[4]);
	    Qy.push_back(0.0);
	    Qy.push_back(af_params.dt);
	    Qy.push_back(af_params.dt);

	    npts = 28;
	    initialize_2d(npts, 4, &Lam);
	    DefineFkp(npts, Lam);

	    for (k=0; k<npts; k++)
	    {
		    x_gauss = Lam[k][0] * Qx[0] + Lam[k][1] * Qx[1] + Lam[k][2] * Qx[2];
		    y_gauss = Lam[k][0] * Qy[0] + Lam[k][1] * Qy[1] + Lam[k][2] * Qy[2];
		    DJac = 2.0 * AreaT(&Qx, &Qy);

		    //
		    // find phi, phi_x, phi_y on R and C at (x,y)
		    //

	     	 BasisQR(Rx, x_gauss, y_gauss, phiR, phiRx, phiRy);
		    BasisQR(Cx, x_gauss, y_gauss, phiC, phiCx, phiCy);
		    for (i=0; i<4; i++)
		    {
			    dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i],
											    phiC[0] + phiC[3], phiCx[0] + phiCx[3]);
			    StifR[i][0] += Lam[k][3] * DJac * dval;

			    dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i],
											    phiC[1] + phiC[2], phiCx[1] + phiCx[2]);
			    StifR[i][1] += Lam[k][3] * DJac * dval;
		    }
       }
	    clear_2d(npts, Lam);
   }
   else 				// Q is a non-degenerate quadrilateral
   {
	    Qx.push_back(vx[1]);	// vertices of Q on right
	    Qx.push_back(vx[7]);
	    Qx.push_back(vx[5]);
	    Qx.push_back(vx[4]);

	    Qy.push_back(0.0);
	    Qy.push_back(0.0);
	    Qy.push_back(af_params.dt);
	    Qy.push_back(af_params.dt);

	    npts = 5 * 5;
	    initialize_2d(npts, 3, &Gs);
	    DefineGaussian(5, Gs);

	    double psi[4], psi1[4], psi2[4], jac[4];
	    for (k=0; k<npts; k++)
	    {
		    BasisQS(Gs[k][0], Gs[k][1], psi, psi1, psi2);

		    x_gauss = 0.0;
		    y_gauss = 0.0;
		    for (i=0; i<4; i++)
		    {
			    jac[i] = 0.0;
		    }
		    for (i=0; i<4; i++)
		    {
			    x_gauss += psi[i] * Qx[i];
			    y_gauss += psi[i] * Qy[i];
			    jac[0] += Qx[i] * psi1[i];
			    jac[1] += Qx[i] * psi2[i];
			    jac[2] += Qy[i] * psi1[i];
			    jac[3] += Qy[i] * psi2[i];
		    }

		    DJac = jac[0] * jac[3] - jac[1] * jac[2];

		    //
		    // find phi, phi_x, phi_y on L and C at (x,y)
		    //

		    BasisQR(Rx, x_gauss, y_gauss, phiR, phiRx, phiRy);
		    BasisQR(Cx, x_gauss, y_gauss, phiC, phiCx, phiCy);
		    for (i=0; i<4; i++)
		    {
			    dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i],
											    phiC[0] + phiC[3], phiCx[0] + phiCx[3]);
			    StifR[i][0] += Gs[k][2] * DJac * dval;

			    dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i],
											    phiC[1] + phiC[2], phiCx[1] + phiCx[2]);
			    StifR[i][1] += Gs[k][2] * DJac * dval;
		    }
	    }
	    clear_2d(npts, Gs);
   }

	for (i=0; i<2; i++)
	{
		Stif[0][i] = StifL[0][i];
		Stif[1][i] = StifL[1][i] + StifR[0][i];
		Stif[2][i] = 					StifR[1][i];
		Stif[3][i] = StifL[3][i];
		Stif[4][i] = StifL[2][i] + StifR[3][i];
		Stif[5][i] = 					StifR[2][i];
	}
	delete [] phiR;
	delete [] phiRx;
	delete [] phiRy;
	delete [] phiL;
	delete [] phiLx;
	delete [] phiLy;
	delete [] phiCx;
	delete [] phiCy;
	delete [] phiC;

	clear_2d(3, StifL);
	clear_2d(4, StifR);
}

void US_Astfem_RSA::IntQTn1_ellam(vector <double> vx, double D, double sw2, double **Stif)
{
	// element to define basis functions
	//
	unsigned int npts, i, k;
	double x_gauss, y_gauss, dval;
	vector <double> Lx, Ly, Tx, Ty;
	double **StifL=NULL, **Lam=NULL, DJac;
	double *phiL, *phiLx, *phiLy;
	phiL = new double [4];
	phiLx = new double [4];
	phiLy = new double [4];
	Lx.clear();
	Ly.clear();
	Tx.clear();
	Ty.clear();

	Lx.push_back(vx[0]);
	Lx.push_back(vx[1]);
	Lx.push_back(vx[3]);
	Lx.push_back(vx[2]);

	Ly.push_back(0.0);
	Ly.push_back(0.0);
	Ly.push_back(af_params.dt);
	Ly.push_back(af_params.dt);

	initialize_2d(4, 2, &StifL);

	//
	// integration over T:
	//

	Tx.push_back(vx[4]);	// triangle for integration
	Tx.push_back(vx[1]);
	Tx.push_back(vx[3]);

	Ty.push_back(0.0);
	Ty.push_back(0.0);
	Ty.push_back(af_params.dt);

	npts = 28;
	initialize_2d(npts, 4, &Lam);
	DefineFkp(npts, Lam);

	for (k=0; k<npts; k++)
	{
		x_gauss = Lam[k][0] * Tx[0] + Lam[k][1] * Tx[1] + Lam[k][2] * Tx[2];
		y_gauss = Lam[k][0] * Ty[0] + Lam[k][1] * Ty[1] + Lam[k][2] * Ty[2];
		DJac = 2.0 * AreaT(&Tx, &Ty);

      if(DJac<1.e-22) break;
		//
		// find phi, phi_x, phi_y on R and C at (x,y)
		//

		BasisQR(Lx, x_gauss, y_gauss, phiL, phiLx, phiLy);

		for (i=0; i<4; i++)
		{
			dval = Integrand(x_gauss, D, sw2, phiL[i], phiLx[i], phiLy[i], 1.0, 0.0);
			StifL[i][0] += Lam[k][3] * DJac * dval;
		}
	}
	clear_2d(npts, Lam);


	for (i=0; i<2; i++)
	{
		Stif[0][i] = StifL[0][i];
		Stif[1][i] = StifL[1][i];
		Stif[2][i] = StifL[3][i];
		Stif[3][i] = StifL[2][i];
	}
	delete [] phiL;
	delete [] phiLx;
	delete [] phiLy;

	clear_2d(4, StifL);
}


void US_Astfem_RSA::QuadSolver_ellam(double *ai, double *bi, double *ci, double *di, double *cr, double *solu)
{
//
// solve Quad-diagonal system [a_i, b_i, *c_i*, d_i]*[x]=[r_i]
// c_i are on the main diagonal line
//

	unsigned int i;
	double tmp;
	vector<double> ca, cb, cc, cd;
	ca.clear();
	cb.clear();
	cc.clear();
	cd.clear();
	for (i=0; i<N; i++)
	{
		ca.push_back( ai[i] );
		cb.push_back( bi[i] );
		cc.push_back( ci[i] );
		cd.push_back( di[i] );
	}

	for (i=N-2; i>=1; i--)
	{
		tmp = cd[i]/cc[i+1];
		cc[i] = cc[i]-cb[i+1]*tmp;
		cb[i] = cb[i]-ca[i+1]*tmp;
		cr[i] = cr[i]-cr[i+1]*tmp;
	}
   i=0;
	tmp = cd[i]/cc[i+1];
	cc[i] = cc[i]-cb[i+1]*tmp;
	cr[i] = cr[i]-cr[i+1]*tmp;
	solu[0] = cr[0] / cc[0];
	solu[1] = (cr[1] - cb[1] * solu[0]) / cc[1];
	i = 1;
	do
	{
		i++;
		solu[i] = (cr[i] - ca[i] * solu[i-2] - cb[i] * solu[i-1]) / cc[i];
	} while (i != N-1);
}

// ******* end of ELLAM *********************



//
// computer basis on standard element
//
void US_Astfem_RSA::BasisTS(double xi, double et, double *phi, double *phi1, double *phi2)
{
//function [phi, phi1, phi2] = BasisTS(xi,et)

	phi[0] = 1.0 - xi - et;
	phi1[0] = -1.0;
	phi2[0]= -1.0;

	phi[1] = xi;
	phi1[1] = 1.0;
	phi2[1] = 0.0;

	phi[2] = et;
	phi1[2] = 0.0;
	phi2[2] = 1.0;
}

//
// computer basis on standard element
//
void US_Astfem_RSA::BasisQS(double xi, double et, double *phi, double *phi1, double *phi2)
{

	phi[0] = (1.0 - xi) * (1.0 - et);
	phi1[0] = -1.0 * (1.0 - et);
	phi2[0]= -1.0 * (1.0 - xi);

	phi[1] = xi * (1.0 - et);
	phi1[1] = 1.0 - et;
	phi2[1] = -xi;

	phi[2] = xi * et;
	phi1[2] = et;
	phi2[2] = xi;

	phi[3] = (1.0 - xi) * et;
	phi1[3] = -et;
	phi2[3] = 1.0 - xi;
}

void US_Astfem_RSA::DefineFkp(unsigned int npts, double **Lam)
{
   //
   // source: http://people.scs.fsu.edu/~burkardt/datasets/quadrature_rules_tri/quadrature_rules_tri.html
   //
	switch (npts)
	{
		case 12:
      //  STRANG9, order 12, degree of precision 6.
		{
  				Lam[ 0][0] = 0.873821971016996;	Lam[ 0][1] = 0.063089014491502;
  				Lam[ 1][0] = 0.063089014491502;	Lam[ 1][1] = 0.873821971016996;
  				Lam[ 2][0] = 0.063089014491502;	Lam[ 2][1] = 0.063089014491502;
  				Lam[ 3][0] = 0.501426509658179;	Lam[ 3][1] = 0.249286745170910;
  				Lam[ 4][0] = 0.249286745170910;	Lam[ 4][1] = 0.501426509658179;
  				Lam[ 5][0] = 0.249286745170910;	Lam[ 5][1] = 0.249286745170910;
  				Lam[ 6][0] = 0.636502499121399;	Lam[ 6][1] = 0.310352451033785;
  				Lam[ 7][0] = 0.636502499121399;	Lam[ 7][1] = 0.053145049844816;
  				Lam[ 8][0] = 0.310352451033785;	Lam[ 8][1] = 0.636502499121399;
  				Lam[ 9][0] = 0.310352451033785;	Lam[ 9][1] = 0.053145049844816;
  				Lam[10][0] = 0.053145049844816;	Lam[10][1] = 0.636502499121399;
  				Lam[11][0] = 0.053145049844816;	Lam[11][1] = 0.310352451033785;

  				Lam[ 0][3] = 0.050844906370207;
  				Lam[ 1][3] = 0.050844906370207;
  				Lam[ 2][3] = 0.050844906370207;
  				Lam[ 3][3] = 0.116786275726379;
  				Lam[ 4][3] = 0.116786275726379;
  				Lam[ 5][3] = 0.116786275726379;
  				Lam[ 6][3] = 0.082851075618374;
  				Lam[ 7][3] = 0.082851075618374;
  				Lam[ 8][3] = 0.082851075618374;
  				Lam[ 9][3] = 0.082851075618374;
  				Lam[10][3] = 0.082851075618374;
  				Lam[11][3] = 0.082851075618374;
			break;
		}
		case 28:
      // TOMS612_28, order 28, degree of precision 11, a rule from ACM TOMS algorithm #612
		{
  			Lam[ 0][0] = 0.33333333333333333  ;	Lam[ 0][1] = 0.333333333333333333 ;
  			Lam[ 1][0] = 0.9480217181434233   ;	Lam[ 1][1] = 0.02598914092828833 ;
  			Lam[ 2][0] = 0.02598914092828833  ;	Lam[ 2][1] = 0.9480217181434233 ;
  			Lam[ 3][0] = 0.02598914092828833  ;	Lam[ 3][1] = 0.02598914092828833 ;
  			Lam[ 4][0] = 0.8114249947041546   ;	Lam[ 4][1] = 0.09428750264792270 ;
  			Lam[ 5][0] = 0.09428750264792270  ;	Lam[ 5][1] = 0.8114249947041546 ;
  			Lam[ 6][0] = 0.09428750264792270  ;	Lam[ 6][1] = 0.09428750264792270 ;
  			Lam[ 7][0] = 0.01072644996557060  ;	Lam[ 7][1] = 0.4946367750172147 ;
  			Lam[ 8][0] = 0.4946367750172147   ;	Lam[ 8][1] = 0.01072644996557060 ;
  			Lam[ 9][0] = 0.4946367750172147   ;	Lam[ 9][1] = 0.4946367750172147 ;
  			Lam[10][0] = 0.5853132347709715   ;	Lam[10][1] = 0.2073433826145142 ;
  			Lam[11][0] = 0.2073433826145142   ;	Lam[11][1] = 0.5853132347709715 ;
  			Lam[12][0] = 0.2073433826145142   ;	Lam[12][1] = 0.2073433826145142 ;
  			Lam[13][0] = 0.1221843885990187   ;	Lam[13][1] = 0.4389078057004907 ;
  			Lam[14][0] = 0.4389078057004907   ;	Lam[14][1] = 0.1221843885990187 ;
  			Lam[15][0] = 0.4389078057004907   ;	Lam[15][1] = 0.4389078057004907 ;
  			Lam[16][0] = 0.6779376548825902   ;	Lam[16][1] = 0.04484167758913055 ;
  			Lam[17][0] = 0.6779376548825902   ;	Lam[17][1] = 0.27722066752827925 ;
  			Lam[18][0] = 0.04484167758913055  ;	Lam[18][1] = 0.6779376548825902 ;
  			Lam[19][0] = 0.04484167758913055  ;	Lam[19][1] = 0.27722066752827925 ;
  			Lam[20][0] = 0.27722066752827925  ;	Lam[20][1] = 0.6779376548825902 ;
  			Lam[21][0] = 0.27722066752827925  ;	Lam[21][1] = 0.04484167758913055 ;
  			Lam[22][0] = 0.8588702812826364   ;	Lam[22][1] = 0.00000000000000000 ;
  			Lam[23][0] = 0.8588702812826364   ;	Lam[23][1] = 0.1411297187173636 ;
  			Lam[24][0] = 0.0000000000000000   ;	Lam[24][1] = 0.8588702812826364 ;
  			Lam[25][0] = 0.0000000000000000   ;	Lam[25][1] = 0.1411297187173636 ;
  			Lam[26][0] = 0.1411297187173636   ; Lam[26][1] = 0.8588702812826364 ;
  			Lam[27][0] = 0.1411297187173636   ;	Lam[27][1] = 0.0000000000000000 ;

  			Lam[ 0][3] = 0.08797730116222190 ;
  			Lam[ 1][3] = 0.008744311553736190 ;
  			Lam[ 2][3] = 0.008744311553736190 ;
  			Lam[ 3][3] = 0.008744311553736190 ;
  			Lam[ 4][3] = 0.03808157199393533 ;
  			Lam[ 5][3] = 0.03808157199393533 ;
  			Lam[ 6][3] = 0.03808157199393533 ;
  			Lam[ 7][3] = 0.01885544805613125 ;
  			Lam[ 8][3] = 0.01885544805613125 ;
  			Lam[ 9][3] = 0.01885544805613125 ;
  			Lam[10][3] = 0.07215969754474100 ;
  			Lam[11][3] = 0.07215969754474100 ;
  			Lam[12][3] = 0.07215969754474100 ;
  			Lam[13][3] = 0.06932913870553720 ;
  			Lam[14][3] = 0.06932913870553720 ;
  			Lam[15][3] = 0.06932913870553720 ;
  			Lam[16][3] = 0.04105631542928860 ;
  			Lam[17][3] = 0.04105631542928860 ;
  			Lam[18][3] = 0.04105631542928860 ;
  			Lam[19][3] = 0.04105631542928860 ;
  			Lam[20][3] = 0.04105631542928860 ;
  			Lam[21][3] = 0.04105631542928860 ;
  			Lam[22][3] = 0.007362383783300573 ;
  			Lam[23][3] = 0.007362383783300573 ;
  			Lam[24][3] = 0.007362383783300573 ;
  			Lam[25][3] = 0.007362383783300573 ;
  			Lam[26][3] = 0.007362383783300573 ;
  			Lam[27][3] = 0.007362383783300573 ;
			break;
		}
		case 37:
      //   TOMS706_37, order 37, degree of precision 13, a rule from ACM TOMS algorithm #706.
		{
  			Lam[ 0][0] = 0.333333333333333333333333333333; 	Lam[ 0][1] = 0.333333333333333333333333333333;
  			Lam[ 1][0] = 0.950275662924105565450352089520;  Lam[ 1][1] = 0.024862168537947217274823955239;
  			Lam[ 2][0] = 0.024862168537947217274823955239;  Lam[ 2][1] = 0.950275662924105565450352089520;
  			Lam[ 3][0] = 0.024862168537947217274823955239;  Lam[ 3][1] = 0.024862168537947217274823955239;
  			Lam[ 4][0] = 0.171614914923835347556304795551;  Lam[ 4][1] = 0.414192542538082326221847602214;
  			Lam[ 5][0] = 0.414192542538082326221847602214;  Lam[ 5][1] = 0.171614914923835347556304795551;
  			Lam[ 6][0] = 0.414192542538082326221847602214;  Lam[ 6][1] = 0.414192542538082326221847602214;
  			Lam[ 7][0] = 0.539412243677190440263092985511;  Lam[ 7][1] = 0.230293878161404779868453507244;
  			Lam[ 8][0] = 0.230293878161404779868453507244;  Lam[ 8][1] = 0.539412243677190440263092985511;
  			Lam[ 9][0] = 0.230293878161404779868453507244;  Lam[ 9][1] = 0.230293878161404779868453507244;
  			Lam[10][0] = 0.772160036676532561750285570113;  Lam[10][1] = 0.113919981661733719124857214943;
         Lam[11][0] = 0.113919981661733719124857214943;  Lam[11][1] = 0.772160036676532561750285570113;
         Lam[12][0] = 0.113919981661733719124857214943;  Lam[12][1] = 0.113919981661733719124857214943;
         Lam[13][0] = 0.009085399949835353883572964740;  Lam[13][1] = 0.495457300025082323058213517632;
         Lam[14][0] = 0.495457300025082323058213517632;  Lam[14][1] = 0.009085399949835353883572964740;
         Lam[15][0] = 0.495457300025082323058213517632;  Lam[15][1] = 0.495457300025082323058213517632;
         Lam[16][0] = 0.062277290305886993497083640527;  Lam[16][1] = 0.468861354847056503251458179727;
         Lam[17][0] = 0.468861354847056503251458179727;  Lam[17][1] = 0.062277290305886993497083640527;
         Lam[18][0] = 0.468861354847056503251458179727;  Lam[18][1] = 0.468861354847056503251458179727;
         Lam[19][0] = 0.022076289653624405142446876931;  Lam[19][1] = 0.851306504174348550389457672223;
         Lam[20][0] = 0.022076289653624405142446876931;  Lam[20][1] = 0.126617206172027096933163647918;
         Lam[21][0] = 0.851306504174348550389457672223;  Lam[21][1] = 0.022076289653624405142446876931;
         Lam[22][0] = 0.851306504174348550389457672223;  Lam[22][1] = 0.126617206172027096933163647918;
         Lam[23][0] = 0.126617206172027096933163647918;  Lam[23][1] = 0.022076289653624405142446876931;
         Lam[24][0] = 0.126617206172027096933163647918;  Lam[24][1] = 0.851306504174348550389457672223;
         Lam[25][0] = 0.018620522802520968955913511549;  Lam[25][1] = 0.689441970728591295496647976487;
         Lam[26][0] = 0.018620522802520968955913511549;  Lam[26][1] = 0.291937506468887771754472382212;
         Lam[27][0] = 0.689441970728591295496647976487;  Lam[27][1] = 0.018620522802520968955913511549;
         Lam[28][0] = 0.689441970728591295496647976487;  Lam[28][1] = 0.291937506468887771754472382212;
         Lam[29][0] = 0.291937506468887771754472382212;  Lam[29][1] = 0.018620522802520968955913511549;
         Lam[30][0] = 0.291937506468887771754472382212;  Lam[30][1] = 0.689441970728591295496647976487;
         Lam[31][0] = 0.096506481292159228736516560903;  Lam[31][1] = 0.635867859433872768286976979827;
         Lam[32][0] = 0.096506481292159228736516560903;  Lam[32][1] = 0.267625659273967961282458816185;
         Lam[33][0] = 0.635867859433872768286976979827;  Lam[33][1] = 0.096506481292159228736516560903;
         Lam[34][0] = 0.635867859433872768286976979827;  Lam[34][1] = 0.267625659273967961282458816185;
         Lam[35][0] = 0.267625659273967961282458816185;  Lam[35][1] = 0.096506481292159228736516560903;
         Lam[36][0] = 0.267625659273967961282458816185;  Lam[36][1] = 0.635867859433872768286976979827;


  			Lam[ 0][3] = 0.051739766065744133555179145422;
  			Lam[ 1][3] = 0.008007799555564801597804123460;
  			Lam[ 2][3] = 0.008007799555564801597804123460;
  			Lam[ 3][3] = 0.008007799555564801597804123460;
  			Lam[ 4][3] = 0.046868898981821644823226732071;
  			Lam[ 5][3] = 0.046868898981821644823226732071;
  			Lam[ 6][3] = 0.046868898981821644823226732071;
  			Lam[ 7][3] = 0.046590940183976487960361770070;
  			Lam[ 8][3] = 0.046590940183976487960361770070;
  			Lam[ 9][3] = 0.046590940183976487960361770070;
  			Lam[10][3] = 0.031016943313796381407646220131;
  			Lam[11][3] = 0.031016943313796381407646220131;
  			Lam[12][3] = 0.031016943313796381407646220131;
  			Lam[13][3] = 0.010791612736631273623178240136;
  			Lam[14][3] = 0.010791612736631273623178240136;
  			Lam[15][3] = 0.010791612736631273623178240136;
  			Lam[16][3] = 0.032195534242431618819414482205;
  			Lam[17][3] = 0.032195534242431618819414482205;
  			Lam[18][3] = 0.032195534242431618819414482205;
  			Lam[19][3] = 0.015445834210701583817692900053;
  			Lam[20][3] = 0.015445834210701583817692900053;
  			Lam[21][3] = 0.015445834210701583817692900053;
         Lam[22][3] = 0.015445834210701583817692900053;
         Lam[23][3] = 0.015445834210701583817692900053;
         Lam[24][3] = 0.015445834210701583817692900053;
         Lam[25][3] = 0.017822989923178661888748319485;
         Lam[26][3] = 0.017822989923178661888748319485;
         Lam[27][3] = 0.017822989923178661888748319485;
         Lam[28][3] = 0.017822989923178661888748319485;
         Lam[29][3] = 0.017822989923178661888748319485;
         Lam[30][3] = 0.017822989923178661888748319485;
         Lam[31][3] = 0.037038683681384627918546472190;
         Lam[32][3] = 0.037038683681384627918546472190;
         Lam[33][3] = 0.037038683681384627918546472190;
         Lam[34][3] = 0.037038683681384627918546472190;
         Lam[35][3] = 0.037038683681384627918546472190;
         Lam[36][3] = 0.037038683681384627918546472190;

			break;
      }
		default:
		{
			return;
		}
	}
   for(unsigned int i=0; i<npts; i++)
   {
      Lam[i][2] = 1. - Lam[i][0] - Lam[i][1];
      Lam[i][3] /= 2.; 			// to make the sum( wt ) = 0.5 = area of standard elem
   }
}


//
// AreaT: area of a triangle (v1, v2, v3)
//
double US_Astfem_RSA::AreaT(vector <double> *xv, vector <double> *yv)
{
	return (0.5 * (((*xv)[1] - (*xv)[0]) * ((*yv)[2] - (*yv)[0])
					- ((*xv)[2] - (*xv)[0]) * ((*yv)[1] - (*yv)[0])));
}

//
// function BasisTR: compute basis on real element T:
//	phi, phi_x, phi_t at a given (xs,ts) point
//	the triangular is assumed to be (x1,y1), (x2, y2), (x3, y3)
//
void US_Astfem_RSA::BasisTR(vector <double> vx, vector <double> vy, double xs,
double ys, double *phi, double *phix, double *phiy)
{
	// find (xi,et) corresponding to (xs, ts)
	//
	unsigned int i;
	vector <double> tempv1, tempv2;
	tempv1.clear();
	tempv2.clear();
	tempv1.push_back(xs);
	tempv1.push_back(vx[2]);
	tempv1.push_back(vx[0]);
	tempv2.push_back(ys);
	tempv2.push_back(vy[2]);
	tempv2.push_back(vy[0]);
	double AreaK = AreaT(&vx, &vy);
	double xi = AreaT(&tempv1, &tempv2)/AreaK;
	tempv1.clear();
	tempv2.clear();
	tempv1.push_back(xs);
	tempv1.push_back(vx[0]);
	tempv1.push_back(vx[1]);
	tempv2.push_back(ys);
	tempv2.push_back(vy[0]);
	tempv2.push_back(vy[1]);

	double et = AreaT(&tempv1, &tempv2)/AreaK;

	double *phi1, *phi2;

	phi1 = new double [3];
	phi2 = new double [3];

	BasisTS(xi, et, phi, phi1, phi2);

	double Jac[4], JacN[4], det;

	Jac[0] = vx[0] * phi1[0] + vx[1] * phi1[1] + vx[2] * phi1[2];
	Jac[1] = vx[0] * phi2[0] + vx[1] * phi2[1] + vx[2] * phi2[2];
	Jac[2] = vy[0] * phi1[0] + vy[1] * phi1[1] + vy[2] * phi1[2];
	Jac[3] = vy[0] * phi2[0] + vy[1] * phi2[1] + vy[2] * phi2[2];

	det = Jac[0] * Jac[3] - Jac[1] * Jac [2];

	JacN[0] = Jac[3]/det;
	JacN[1] = -Jac[1]/det;
	JacN[2] = -Jac[2]/det;
	JacN[3] = Jac[0]/det;

	for (i=0; i<3; i++)
	{
		phix[i] = JacN[0] * phi1[i] + JacN[2] * phi2[i];
		phiy[i] = JacN[1] * phi1[i] + JacN[3] * phi2[i];
	}
	delete [] phi1;
	delete [] phi2;
}

void US_Astfem_RSA::BasisQR(vector <double> vx, double xs,
double ts, double *phi, double *phix, double *phiy)
{
	unsigned int i;

	// find (xi,et) corresponding to (xs, ts)
	double et = ts/af_params.dt;
	double A = vx[0] * (1.0 - et) + vx[3] * et;
	double B = vx[1] * (1.0 - et) + vx[2] * et;
	double xi = (xs - A)/(B - A);

	double *phi1, *phi2;

	phi1 = new double [4];
	phi2 = new double [4];

	BasisQS(xi, et, phi, phi1, phi2);

	double Jac[4], JacN[4], det;

	Jac[0] = vx[0] * phi1[0] + vx[1] * phi1[1] + vx[2] * phi1[2] + vx[3] * phi1[3];
	Jac[1] = vx[0] * phi2[0] + vx[1] * phi2[1] + vx[2] * phi2[2] + vx[3] * phi2[3];
	Jac[2] = af_params.dt * phi1[2] + af_params.dt * phi1[3];
	Jac[3] = af_params.dt * phi2[2] + af_params.dt * phi2[3];

	det = Jac[0] * Jac[3] - Jac[1] * Jac [2];

	JacN[0] = Jac[3]/det;
	JacN[1] = -Jac[1]/det;
	JacN[2] = -Jac[2]/det;
	JacN[3] = Jac[0]/det;

	for (i=0; i<4; i++)
	{
		phix[i] = JacN[0] * phi1[i] + JacN[2] * phi2[i];
		phiy[i] = JacN[1] * phi1[i] + JacN[3] * phi2[i];
	}
	delete [] phi1;
	delete [] phi2;
}


////////////////////////////////////////////////////////////////////////%
// calculate total mass
// (r,u) concentration defined at r(1), ...., r(M)
//  M: r(1).... r(M): the interval for total mass, (M-1) subintervals
////////////////////////////////////////////////////////////////////////%
double US_Astfem_RSA::IntConcentration(vector<double> r, double *u)
{
//function T=IntConcentration(r,M,u)
	double T = 0.0;
	for (unsigned int j=0; j<r.size()-1; j++)
	{
		T += (r[j+1] - r[j]) * ((r[j+1] - r[j]) * (u[j+1]/3.0 + u[j]/6.0)
			+ r[j] * (u[j] + u[j+1])/2.0);
	}
	return T;
}



void US_Astfem_RSA::ReactionOneStep_Euler_imp(double **C1, double TimeStep)
{
//////////////////////////////%
//
// ReactionOneStep_Euler_imp:  implicit Mid-point Euler
//
//////////////////////////////%

	// the following parameters should be supplied outside

	// instantanuous association
	//
	double ct;
//   int NegComp=0;
	unsigned int i, j;

	for (j=0; j<N; j++)
	{
		ct = 0.;
		for (i=0; i<af_params.s.size(); i++)
		{
			ct += C1[i][j];
//         if(C1[i][j]<0) NegComp = 1;
		}

//      if(NegComp==0)
//      {

	   if(af_params.model >= 4 && af_params.model <= 10)  // monomer - (n)mer
      {
	      double dva, dvb, dvc, uhat;
         dva = TimeStep * af_params.koff[0] * af_params.keq[0];
         dvb = TimeStep * af_params.koff[0] + 2.;
         dvc = TimeStep * af_params.koff[0] * ct + 2. * C1[0][j];
         if(dva<1.e-12)
         {
            uhat = dvc / dvb;
         }
         else
         {
            switch(af_params.model)
            {
               case 4:	// mono <--> dimer
                  uhat = 2*dvc / ( dvb+sqrt(dvb*dvb+4.*dva*dvc) );
                  break;
               case 5:	// mono <--> trimer
                  uhat = cube_root(-dvc/dva, dvb/dva, 0.0);
                  break;
               default:	// mono <--> (n)mer
			         uhat = find_C1_mono_Nmer( af_params.n[1], dva/dvb, dvc/dvb);
            }
         }
         C1[0][j] = 2.*uhat - C1[0][j];
         C1[1][j] = ct - C1[0][j];
       }
	    else
	    {
	   	cout<<"warning: finite reation not implemeted"<<endl;
	    }
//    }	// end if
    }
}


////////////////////////////////////////
//
// DecomposeCT according to
//		K_1*C1^1+K2_C1^2+...+K_mC_1^m=CT
//
////////////////////////////////////////
int US_Astfem_RSA::DecomposeCT(double CT, double *C)
{
	double dval, C1;

		dval = CT;

		if (af_params.model == 4) // monomer-dimer
		{
			C1 = (sqrt(1.0 + 4.0 * af_params.keq[0] * dval) - 1.0)/(2.0 * af_params.keq[0]) ;
			C[0] = C1;
			C[1] = af_params.keq[0] * pow(C1, (double) af_params.n[1]);
		}
		else if (af_params.model == 5) // monomer-trimer
		{
			C1 = cube_root(-dval/af_params.keq[0], 1.0/af_params.keq[0], 0.0);
			C[0] = C1;
			C[1] = af_params.keq[0] * pow(C1, (double) af_params.n[1]);
		}
		else if (af_params.model > 5 && af_params.model < 11)
		{
			C1 = find_C1_mono_Nmer( af_params.n[1], af_params.keq[0], dval );
			C[0] = C1;
			C[1] = af_params.keq[0] * pow(C1, (double) af_params.n[1]);
		}
		else if (af_params.model == 11)// monomer-dimer-trimer system
		{
			C1 = cube_root(-dval/af_params.keq[1], 1.0/af_params.keq[1], af_params.keq[0]/af_params.keq[1]);
			C[0] = C1;
			C[1] = af_params.keq[0] * pow(C1, (double) af_params.n[1]);
			C[2] = af_params.keq[1] * pow(C1, (double) af_params.n[2]);
		}
		else
		{
			cout << "warning: model #" << af_params.model << " is not yet supported in find_C1()" << endl;
			return (-1);
		}
	   return (0);
}

////////////////////////////////////////
//
// find_C1_mono_Nmer:	find C1 from    C1 + K * C1^n = CT
//
////////////////////////////////////////
double US_Astfem_RSA::find_C1_mono_Nmer( int n, double K, double CT )
{
   //
   // use Newton's method for f(x) = x + K*x^n - CT
   //    x_{i+1} = x_i - f(x_i)/f'(x_i)
   //
   double x0=0., x1;
	unsigned int i, MaxNumIt = 1000;

   if ( CT<=0. ) return( 0. );
   if ( CT<=1.e-12 ) return( CT );

	for ( i=1; i < MaxNumIt; i++)
   {
		x1 = ( K * (n-1.0) * pow(x0,(double)n) + CT ) / ( 1. + K * n * pow(x0, n-1.) );
	   if ( fabs(x1 - x0)/(1.+fabs(x1)) < 1.e-12 ) break;
      x0 = x1;
	}

   if( i == MaxNumIt )
   {
      cout<<"warning: Newton's method did not coonverges in find_C1_mono_Nmer"<<endl;
      return( -1.0 );
   }
   return( 0.5*(x0+x1) );
}



//////////////////////////////////////////////////////////////////
//
// cub_root: find the positive cubic-root of a cubic polynomial
//			p(x)=a0+a1*x+a2*x^2+x^3
//
// with: a0<=0,  and  a1, a2>=0;
//
//////////////////////////////////////////////////////////////////
double US_Astfem_RSA::cube_root(double a0, double a1, double a2)
{
	double B, Q, S, D, Dc, Dc2, theta, x;
	Q = (3.0 * a1 - pow(a2, 2.0)) / 9.0;
	S = (9.0 * a1 * a2 - 27.0 * a0 - 2.0 * pow(a2, 3.0)) / 54.0;
	D = pow(Q, 3.0) + pow(S, 2.0);
	if (D < 0)
	{
		theta = acos(S / sqrt(pow(-Q, 3.0)));
		x = 2.0 * sqrt(-Q) * cos(theta / 3.0);
	}
	else
	{
		Dc = sqrt(D);
		if (S + Dc < 0)
		{
			B = - pow(-(S + Dc), 1.0/3.0) - pow(Dc - S, 1.0/3.0);
		}
		else if (S - Dc < 0)
		{
			B = pow(S+Dc, 1.0/3.0) - pow(Dc - S, 1.0/3.0);
		}
		else
		{
			B = pow(S + Dc, 1.0/3.0) + pow(S - Dc, 1.0/3.0);
		}
		Dc2 = -3.0 * (pow(B, 2.0) + 4 * Q);
		if (Dc2 > 0)
		{
			x = max(B, 0.5 * (-B + sqrt(Dc2)));
		}
		else
		{
			x = B;
		}
	}
	x = x - a2 / 3.0;
	return(x);
}

void US_Astfem_RSA::DefInitCond(double **C0)
{
	unsigned int j;
	for(j=0; j<N; j++)
	{
		C0[0][j] = 0.3;
		C0[1][j] = 0.7;
	}
}


void US_Astfem_RSA::print_vector(double *dval, unsigned int ival)
{
	unsigned int i;
	for (i=0; i<ival; i++)
	{
//WMC		cout << i << ": " << dval[i] << endl;
		printf("x[%d]=%20.10e \n", i, dval[i]);
	}
}

void US_Astfem_RSA::print_vector(vector <double> *dval)
{
	unsigned int i;
	for (i=0; i<(*dval).size(); i++)
	{
//WMC		cout << i << ": " << (*dval)[i] << endl;
		printf("x[%d]=%20.10e \n", i, (*dval)[i]);
	}
}

int US_Astfem_RSA::interpolate(struct mfem_data *simdata, unsigned int scans, unsigned int points,
float *scantimes, double *radius, double **c)
{

/********************************************************************************************
 * interpolation:																									*
 *																														*
 * First, we need to interpolate the time. Create a new array with the same time dimensions *
 * as the raw data and the same radius dimensions as the simulated data. Then find the time *
 * steps from the simulated data that bracket the experimental data from the left and right.*
 * Then make a linear interpolation for the concentration values at each radius point from  *
 * the simulated data. Then interpolate the radius points by linear interpolation.			*
 *																														*
 ********************************************************************************************/

	unsigned int i, j;
	double slope, intercept;
	double **ip_array;
	ip_array	= 	new double* [scans];
	for (i=0; i<scans; i++)
	{
		ip_array[i] = new double [(*simdata).radius.size()];
	}
	unsigned int count = 0; // counting the number of time steps of the raw data
	for (i=0; i<scans; i++)
	{
		while (count < (*simdata).scan.size()-1
				&& scantimes[i] >= (*simdata).scan[count].time)
		{
			count++;
		}
		if (scantimes[i] == (*simdata).scan[count].time)
		{
			for (j=0; j<(*simdata).radius.size(); j++)
			{
				ip_array[i][j] = (*simdata).scan[count].conc[j];
			}
		}
		else  // else, perform a linear time interpolation:
		{
			for (j=0; j<(*simdata).radius.size(); j++)
			{
				slope = ((*simdata).scan[count].conc[j] - (*simdata).scan[count-1].conc[j])
							/((*simdata).scan[count].time - (*simdata).scan[count-1].time);
				intercept = (*simdata).scan[count].conc[j] - slope * (*simdata).scan[count].time;
				ip_array[i][j] = slope * scantimes[i] + intercept;
			}
		}
	}
	//interpolate radius then add to concentration vector
	for (i=0; i<scans; i++)
	{
		c[i][0] += ip_array[i][0]; // meniscus position is identical for all scans
	}
	// all other points may need interpolation:
	for (i=0; i<scans; i++)
	{
		count = 1;
		for (j=1; j<(unsigned int) points; j++)
		{
			while (radius[j] > (*simdata).radius[count] && count < (*simdata).radius.size()-1)
			{
				count++;
			}
			if (radius[j] == (*simdata).radius[count])
			{
				c[i][j] += ip_array[i][count];
			}
			else
			{
				slope = (ip_array[i][count] - ip_array[i][count-1])/((*simdata).radius[count] - (*simdata).radius[count-1]);
				intercept = ip_array[i][count] - (*simdata).radius[count] * slope;
				c[i][j] += slope * radius[j] + intercept;
			}
		}
	}
	for (i=0; i<scans; i++)
	{
		delete [] ip_array[i];
	}
	delete [] ip_array;
	return 0;
}

// new version: Weiming 05/27/06
int US_Astfem_RSA::interpolate(struct mfem_data *expdata, struct mfem_data *simdata)
{
// NOTE: *expdata has to be initialized to have the proper size (filled with zeros)
// before using this routine! The radius also has to be assigned!

	if ((*expdata).scan.size() == 0 || (*expdata).scan[0].conc.size() == 0)
	{
		return -1;
	}

	unsigned int i, j, kkk, ja;
   double a, b, tmp, xs, **tmpC;

   initialize_2d((*simdata).scan.size(), (*expdata).radius.size(), &tmpC);

   // interpolate all simdata scan onto the grid of expdata.radius
	for (kkk=0; kkk<(*simdata).scan.size(); kkk++)
   {
      ja=0;
      for(j=0; j<(*expdata).radius.size();j++)
      {
		   xs = (*expdata).radius[j];
		   for(i=ja; i<(*simdata).radius.size(); i++)
		   {
			   if((*simdata).radius[i] > xs + 1.e-12) break;
		   }
		   a = (i>0)? (*simdata).radius[i-1] : (*simdata).radius[0];
		   b = (*simdata).radius[i];
		   tmp = (xs - a)/(b - a);
		   tmpC[kkk][j] = (*simdata).scan[kkk].conc[i-1] * (1. - tmp) + (*simdata).scan[kkk].conc[i] * tmp;
		   ja = (i>0)? i-1 : 0;
      }
	}

   // interpolation between 2 time scans across the expdata.scan[].time

   ja=0;
	for (kkk=0; kkk<(*expdata).scan.size(); kkk++)
   {
      xs = (*expdata).scan[kkk].time;
      for( i=ja; i<(*simdata).scan.size(); i++)
      {
         if( (*simdata).scan[i].time>xs+1.e-12 ) break;
      }

      if ( i<=0 )		// time before the first scan of simdata
      {
         for( j=0; j<(*expdata).radius.size(); j++)
         {
            (*expdata).scan[kkk].conc[j] += tmpC[0][j];
         }
         ja = 0;
         (*expdata).scan[kkk].omega_s_t = (*simdata).scan[0].omega_s_t;
      }
      else if ( i >= (*simdata).scan.size()-1 )		// time after the last scan of simdata
      {
         for( j=0; j<(*expdata).radius.size(); j++)
         {
            (*expdata).scan[kkk].conc[j] += tmpC[(*simdata).scan.size()-1][j];
         }
         ja = (*simdata).scan.size()-2;
         (*expdata).scan[kkk].omega_s_t = (*simdata).scan[(*simdata).scan.size()-1].omega_s_t;
      }
      else 		// time between scan i-1 and i
      {
		   a = (*simdata).scan[i-1].time;
		   b = (*simdata).scan[i  ].time;
		   tmp = (xs - a)/(b - a);
         for( j=0; j<(*expdata).radius.size(); j++)
         {
            (*expdata).scan[kkk].conc[j] += tmpC[i-1][j]*(1.-tmp) + tmpC[i][j]*tmp;
         }
         ja = i-1;
         (*expdata).scan[kkk].omega_s_t = (*simdata).scan[i-1].omega_s_t*(1.-tmp) + (*simdata).scan[i].omega_s_t*tmp;
      }

   }
   clear_2d((*simdata).scan.size(), tmpC);
	return(0);
}
