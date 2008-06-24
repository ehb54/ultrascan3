#include "../include/us_astfem_rsa.h"
#include "../include/us_util.h"
#include <algorithm>

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
	//US_FemGlobal fg;
	//fg.write_experiment(system, simparams, "/root/astfem_rsa-output");
	this->simparams = simparams;
	this->system = system;
	unsigned int i, j;
	float current_time = 0.0;
	float current_speed;
	double s_max_abs;			// largest sedimenting or floating speed (absolute value)
	unsigned int duration, delay, initial_npts=5000;
	mfem_data simdata;
	mfem_initial CT0;			// initial total concentration
	af_params.first_speed = (*simparams).speed_step[0].rotorspeed;
	if ((*system).model < 4) // non-interacting single or multicomponent systems
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
			// before going into acceleration phase set w2t and last_time to zero, increment in function caculate_ni()
			last_time = 0.0;
			w2t_integral = 0.0;
			CT0.radius.clear();
			CT0.concentration.clear();
			adjust_limits((*simparams).speed_step[0].rotorspeed);
			(*exp_data)[0].meniscus = af_params.current_meniscus;
			(*exp_data)[0].bottom = af_params.current_bottom;
			double dr = (af_params.current_bottom - af_params.current_meniscus)/(initial_npts-1);
			for (j=0; j<initial_npts; j++)
			{
				CT0.radius.push_back(af_params.current_meniscus + j * dr );
				CT0.concentration.push_back(0.0);
			}
			if ((*system).component_vector[i].c0.concentration.size() == 0) // we don't have an existing CT0 concentration vector
			{ // build up the initial concentration vector with constant concentration
				if ((*simparams).band_forming)
				{
					// calculate the width of the lamella
					double lamella_width = pow(af_params.current_meniscus * af_params.current_meniscus
								+ (*simparams).band_volume * 360.0/(2.5 * 1.2 * M_PI), 0.5) - af_params.current_meniscus;
					// calculate the spread of the lamella:
					for (j=0; j<initial_npts; j++)
					{
						CT0.concentration[j] = (*system).component_vector[i].concentration * exp(-pow( (CT0.radius[j] - af_params.current_meniscus) / lamella_width, 4.0 ) );
						// if (CT0.radius[j] < af_params.current_meniscus + lamella_width)
						// {
							// CT0.concentration[j] = (*system).component_vector[i].concentration;
						// }
					}
				}
				else
				{
					for (j=0; j<initial_npts; j++)
					{
						CT0.concentration[j] = (*system).component_vector[i].concentration;
					}
				}
			}
			else
			{
				// take the existing initial concentration vector and copy it to the temporary CT0 vector:
				// needs rubber band to make sure meniscus and bottom equal current_meniscus and current_bottom
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
				adjust_limits((*simparams).speed_step[j].rotorspeed);
				(*exp_data)[j].meniscus = af_params.current_meniscus;
				(*exp_data)[j].bottom = af_params.current_bottom;
				if((*simparams).speed_step[j].acceleration_flag) // we need to simulate acceleration
				{// if the speed difference is larger than acceleration rate then we have at least 1 acceleration step
					af_params.time_steps = (unsigned int) fabs((*simparams).speed_step[j].rotorspeed
					- current_speed)/(*simparams).speed_step[j].acceleration;
					af_params.dt = 1.0; // each simulation step is 1 second long in the acceleration phase
					af_params.simpoints = 2 * (*simparams).simpoints; // use a fixed grid with refinement at both ends and with twice the number of points
					af_params.start_time = current_time;
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
					//cout << "Current time: " << current_time << endl;
					if (guiFlag)
					{
						emit new_time(current_time);
						qApp->processEvents();
					}
					if (*stopFlag)
					{
					        if (guiFlag)
						{
						    qApp->processEvents();
						}
						interpolate(&(*exp_data)[j], &simdata, af_params.omega_s, af_params.acceleration); // interpolate the simulated data onto the experimental time- and radius grid
						return(1); // early termination = 1
					}
				}  // end of for acceleration
				duration = ((*simparams).speed_step[j].duration_hours * 3600
  				          + (*simparams).speed_step[j].duration_minutes * 60);
				delay = (unsigned int) ((*simparams).speed_step[j].delay_hours * 3600
				                      + (*simparams).speed_step[j].delay_minutes * 60);
				af_params.omega_s = pow((*simparams).speed_step[j].rotorspeed * M_PI/30.0, 2.0);
				// find out the minimum dt between actual scans
            if(duration<=delay) {printf("duration is less than delay! \n"); return(1); }
				af_params.dt = log((*exp_data)[j].bottom/(*exp_data)[j].meniscus)
								/((af_params.omega_s * fabs((*system).component_vector[i].s)) *((*simparams).simpoints - 1));
				if (af_params.dt > (duration - delay))
				{
					af_params.dt = duration - delay;
					af_params.simpoints = 1 + (unsigned int) (log((*exp_data)[j].bottom/(*exp_data)[j].meniscus)
            	/(af_params.omega_s * fabs((*system).component_vector[i].s) * af_params.dt));
				}
				if (af_params.simpoints > 10000)
				{
					af_params.simpoints = 10000;
				}
            printf("std: duration=%d delay=%d, scans=%d\n", duration, delay, (*simparams).speed_step[j].scans);
				// find out the minimum number of simpoints needed to provide the necessary dt:
				af_params.simpoints = (*simparams).simpoints;
				af_params.time_steps = (unsigned int) (1+duration/af_params.dt);
//cout << "speed step:\t" << j << ", component: " << i << endl;
//cout << "speed:\t\t" << (*simparams).speed_step[j].rotorspeed << endl;
//cout << "hours:\t\t" << (*simparams).speed_step[j].duration_hours << endl;
//cout << "minutes:\t" << (*simparams).speed_step[j].duration_minutes << endl;
				af_params.start_time = current_time;
				(*simparams).mesh = (*simparams).mesh;
				af_params.moving_grid = (*simparams).moving_grid;
				af_params.acceleration = false;
				vector <double> rpm;
				rpm.clear();
				rpm.push_back((*simparams).speed_step[j].rotorspeed);

//WMC
            printf("std: time_steps=%d duration=%d dt=%12.5e \n", af_params.time_steps, duration, af_params.dt);

				calculate_ni(rpm[0], rpm[0], af_params.s[0], af_params.D[0], &CT0, &simdata);

				// set the current time to the last scan of this speed step
				current_time = (*simparams).speed_step[j].duration_hours * 3600
				+ (*simparams).speed_step[j].duration_minutes * 60;
				//cout << "Current time: " << current_time << ", duration: " << duration << endl;
				af_params.acceleration = (*simparams).speed_step[j].acceleration_flag; // reset acceleration flag for correct time correction interpolation
				interpolate(&(*exp_data)[j], &simdata, af_params.omega_s, af_params.acceleration); // interpolate the simulated data onto the experimental time- and radius grid

				// set the current speed to the constant rotor speed of the current speed step
				current_speed = (*simparams).speed_step[j].rotorspeed;
				if (guiFlag)
				{
				    qApp->processEvents();
				}
				if (*stopFlag)
				{
					return(1); // early termination = 1
				}
			} // speed step loop
			if (guiFlag)
			{
				emit current_component(i+1);
				//cout << "Current component: " << i+1 << endl;
				qApp->processEvents();
			}
		} // component loop
	}  // end of non-interacting case
	else if ((*system).model >= 4 && (*system).model <= 10) // A + nA = An
	{
		current_time = 0.0; // reset time, which now tracks the beginning of each speed step (duration)
		current_speed = 0.0; // start at rest
		// before going into acceleration phase set w2t and last_time to zero, increment in function caculate_ni()
		last_time = 0.0; // initialize to zero
		w2t_integral = 0.0; // initialize to zero, will be incremented in calculate_ra2
		CT0.radius.clear();
		CT0.concentration.clear();
		adjust_limits((*simparams).speed_step[0].rotorspeed);
		double dr = (af_params.current_bottom - af_params.current_meniscus)/(initial_npts-1);
		//cerr.precision(10);
		//cerr << "Initial points: " << initial_npts << endl;
		for (j=0; j<initial_npts; j++)
		{
			CT0.radius.push_back(af_params.current_meniscus + j * dr );
			//cerr << af_params.current_meniscus + j * dr << endl;
			CT0.concentration.push_back(0.0);
		}
		if ((*system).component_vector[0].c0.concentration.size() == 0) // we don't have an existing CT0 concentration vector
		{ // build up the initial concentration vector with constant concentration
			if ((*simparams).band_forming)
			{
					// calculate the width of the lamella
				double lamella_width = pow(af_params.current_meniscus * af_params.current_meniscus
							+ (*simparams).band_volume * 360.0/(2.5 * 1.2 * M_PI), 0.5) - af_params.current_meniscus;
					// calculate the spread of the lamella:
				for (j=0; j<initial_npts; j++)
				{
					CT0.concentration[j] = (*system).component_vector[0].concentration * exp(-pow( (CT0.radius[j] - af_params.current_meniscus) / lamella_width, 4.0));
				}
			}
			else
			{
				for (j=0; j<initial_npts; j++)
				{
					CT0.concentration[j] = (*system).component_vector[0].concentration;
				}
			}
		}
		else
		{
				// take the existing initial concentration vector and copy it to the temporary CT0 vector:
				// needs rubber band to make sure meniscus and bottom equal current_meniscus and current_bottom
			CT0.radius.clear();
			CT0.concentration.clear();
			CT0 = (*system).component_vector[0].c0;
		}
		af_params.s.clear();
		af_params.D.clear();
		af_params.n.clear();
		af_params.keq.clear();
		af_params.koff.clear();
		af_params.kext.clear();
		af_params.pathlength = 1.2;
		af_params.keq.push_back((double) (*system).assoc_vector[0].keq);
		af_params.koff.push_back((double) (*system).assoc_vector[0].k_off);
		af_params.n.push_back((*system).assoc_vector[0].stoichiometry1);
		af_params.n.push_back((*system).assoc_vector[0].stoichiometry2);
		for (i=0; i<(*system).component_vector.size(); i++)
		{
			//cout << "s[" << i<< "]: " << (*system).component_vector[i].s <<
			//", D[" << i << "]: " << (*system).component_vector[i].D << endl;
			af_params.s.push_back((*system).component_vector[i].s);
			af_params.D.push_back((*system).component_vector[i].D);
			af_params.kext.push_back((double) (*system).component_vector[i].extinction);
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
			adjust_limits((*simparams).speed_step[j].rotorspeed);
			(*exp_data)[j].meniscus = af_params.current_meniscus;
			(*exp_data)[j].bottom = af_params.current_bottom;
			if((*simparams).speed_step[j].acceleration_flag &&
			   (*simparams).speed_step[j].acceleration) // we need to simulate acceleration
			{// if the speed difference is larger than acceleration rate then we have at least 1 acceleration step
				af_params.time_steps = (unsigned int) fabs((*simparams).speed_step[j].rotorspeed
				- current_speed)/(*simparams).speed_step[j].acceleration;
				af_params.dt = 1.0; // each simulation step is 1 second long in the acceleration phase
				af_params.simpoints = 2 * (*simparams).simpoints; // use a fixed grid with refinement at both ends and with twice the number of points
				af_params.start_time = current_time;
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
//				print_af();
// add the acceleration time:
				current_time += af_params.dt * af_params.time_steps;
				if (guiFlag)
				{
					emit new_time(current_time);
					qApp->processEvents();
				}
				if (*stopFlag)
				{
				        if (guiFlag)
					{
					    qApp->processEvents();
					}
					interpolate(&(*exp_data)[j], &simdata, af_params.omega_s, af_params.acceleration); // interpolate the simulated data onto the experimental time- and radius grid
					return(1); // early termination = 1
				}
			}  // end of for acceleration
			duration = ((*simparams).speed_step[j].duration_hours * 3600
					+ (*simparams).speed_step[j].duration_minutes * 60);
			delay = (unsigned int) ((*simparams).speed_step[j].delay_hours * 3600
					+ (*simparams).speed_step[j].delay_minutes * 60);
			af_params.omega_s = pow((*simparams).speed_step[j].rotorspeed * M_PI/30.0, 2.0);
// find out the minimum dt between actual scans
			af_params.dt = (duration - delay)/(*simparams).speed_step[j].scans;
// find out the minimum number of simpoints needed to provide the necessary dt:

			af_params.simpoints = 1 + (unsigned int) (log(af_params.current_bottom/af_params.current_meniscus)/( af_params.omega_s * s_max_abs * af_params.dt));


// if calculated # of simpoints is smaller than user selected number of simpoints, follow user's request:
			if ( af_params.simpoints < (*simparams).simpoints )
			{
				af_params.simpoints = (*simparams).simpoints;
				af_params.dt = log(af_params.current_bottom/af_params.current_meniscus)
				    /(af_params.omega_s * s_max_abs * ((*simparams).simpoints - 1)); // delta_t
			}
			else
			{
				if (guiFlag)
				{
				    cerr << "Number of simpoints adjusted to " << af_params.simpoints
						<< " for component " << i + 1 << " and speed step " << j + 1 << endl;
				}
			}
			af_params.time_steps = (unsigned int) (1+duration/af_params.dt);
			af_params.start_time = current_time;
			(*simparams).mesh = (*simparams).mesh;
			af_params.moving_grid = (*simparams).moving_grid;
			af_params.acceleration = false;
			vector <double> rpm;
			rpm.clear();
			rpm.push_back((*simparams).speed_step[j].rotorspeed);
			calculate_ra2(rpm[0], rpm[0], C0, &simdata);
//			print_af();
			af_params.acceleration = (*simparams).speed_step[j].acceleration_flag; // reset acceleration flag for correct time correction interpolation
			interpolate(&(*exp_data)[j], &simdata, af_params.omega_s, af_params.acceleration); // interpolate the simulated data onto the experimental time- and radius grid

				// set the current time to the last scan of this speed step
			current_time = (*simparams).speed_step[j].duration_hours * 3600
					+ (*simparams).speed_step[j].duration_minutes * 60;

				// set the current speed to the constant rotor speed of the current speed step
			current_speed = (*simparams).speed_step[j].rotorspeed;
			if (guiFlag)
			{
			    qApp->processEvents();
			}
			if (*stopFlag)
			{
				return(1); // early termination = 1
			}
		} // speed step loop
      delete [] C0;
	}
	else
	{
		cerr << "This model is not yet supported..." << endl;
	}
	return(0);
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

	FILE *outf = fopen("tmp.out", "a");

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
	mesh_gen(nu, (*simparams).mesh);
	if (af_params.acceleration)  		// refine left hand side (when s>0) or
	{										// right hand side (when s<0) for acceleration
		double xc ;
		if (s > 0)
		{ // radial distance from meniscus how far the boundary will move during this acceleration step (without diffusion)
			xc = af_params.current_meniscus + sw2 * (af_params.time_steps * af_params.dt) /3.;
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
			xc = af_params.current_bottom + sw2 * (af_params.time_steps * af_params.dt) /3.;
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

//
// time evolution
//

	double *right_hand_side;
	right_hand_side = new double [N];
	for (i=0; i<af_params.time_steps+1; i++) // calculate all time steps f
	{
		rpm_current = rpm_start + (rpm_stop - rpm_start) * (i+0.5)/af_params.time_steps;
		emit current_speed((unsigned int) rpm_current);

      printf("rpm=%12.5e time_steps i=%d C_ttl=%20.10e \n", rpm_current, i,
             IntConcentration(x, C0));

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
		simscan.rpm = (unsigned int) rpm_current;
		simscan.time = af_params.start_time + i * af_params.dt;
		w2t_integral += (simscan.time - last_time) * pow(rpm_current * M_PI/30.0, 2.0);
		last_time = simscan.time;
		simscan.omega_s_t = w2t_integral;
//		cout << "rpm: " << simscan.rpm << ", t: " << simscan.time << ", w2t: " << simscan.omega_s_t << ", dt: " << af_params.dt << endl;
		if (guiFlag)
		{
			if(*movieFlag)
			{
				emit new_scan(&x, C0);
				emit new_time((float) simscan.time);
				if (guiFlag)
				{
				    qApp->processEvents();
				}
			}
		}
		simscan.conc.clear();
		for (j=0; j<N; j++)
		{
			simscan.conc.push_back(C0[j]);
		}
		(*simdata).scan.push_back(simscan);

		if(i%1 == 0 || i<5)
		{
			for (j=0; j<N; j++)
			{
				fprintf(outf, "%12.5e %15.8e %15.8e\n", simscan.time, x[j], C0[j]);
			}
			fprintf(outf, "\n\n");
		}

		//
		// sedimentation part:
		// Calculate thr right hand side vector //
		//
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
		tridiag(CA[0], CA[1], CA[2], right_hand_side, C1, N);
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
//	FILE *outf = fopen("tmp.out", "w");

	Mcomp = af_params.s.size();
	s_max = maxval( af_params.s );  	// used for mesh and dt
	s_min = minval( af_params.s );  	// used for mesh and dt

//	print_af(outf);

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
      //printf("s[%d]=%20.12e  D=%20.12e, sw2=%20.12e\n", i, af_params.s[i], af_params.D[i], sw2);
	}
	mesh_gen(nu, (*simparams).mesh);

	// refine left hand side (when s_max>0) or  right hand side (when s<0) for acceleration
	if (af_params.acceleration)
	{
		double xc ;
		if ( s_min > 0 )  				// all sediment towards bottom
		{
			sw2 = s_max * pow( rpm_stop * M_PI/30., 2.0);
			xc = af_params.current_meniscus + sw2 * (af_params.time_steps * af_params.dt) /3.;
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
			xc = af_params.current_bottom + sw2 * (af_params.time_steps * af_params.dt) /3.;
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
			cerr << "multicomponent system with sedimentation and floating mixed, use uniform mesh" << endl;
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
					xb.push_back(af_params.current_meniscus);
					for (j=0; j<N-1; j++)
					{
						dval = 0.1*exp( sw2/af_params.D[i]*( pow(0.5*(x[j]+x[j+1]), 2.0) - pow(af_params.current_bottom, 2.0) )/2. );
						alpha = af_params.s[i]/s_max * (1.-dval) + dval;
						// alpha = af_params.s[i]/s_max ;
						xb.push_back( pow(x[j], alpha) * pow(x[j+1], (1.0 - alpha) ) );
					}
					GlobalStiff(&xb, CA[i], CB[i], af_params.D[i], sw2 );
					// GlobalStiff_ellam(&xb, CA[i], CB[i], af_params.D[i], sw2 );
				}
			}
			else if (s_max <0) 		// all components floating
			{
				cerr << "all components floating, not implemented yet" << endl;
				return(-1);
			}
			else 		// sedmientation and floating mixed
			{
				cerr << "sedimentation and floating mixed, suppose use fixed grid!" << endl;
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
	for (kkk=0; kkk<af_params.time_steps+2; kkk +=2)		// two steps in together
	{
		rpm_current = rpm_start + (rpm_stop - rpm_start) * (kkk+0.5)/af_params.time_steps;
		emit current_speed((unsigned int) rpm_current);
		simscan.time = af_params.start_time + kkk * af_params.dt;
		simscan.rpm = (unsigned int) rpm_current;
		w2t_integral += (simscan.time - last_time) * pow(rpm_current * M_PI/30.0, 2.0);
		last_time = simscan.time;
		simscan.omega_s_t = w2t_integral;
//		cout << "rpm: " << simscan.rpm << ", t: " << simscan.time << ", w2t: " << simscan.omega_s_t << ", dt: " << af_params.dt << endl;
		if (guiFlag)
		{
			if(*movieFlag)
			{
				emit new_scan(&x, CT0);
				emit new_time((float) simscan.time);
				if (guiFlag)
				{
				    qApp->processEvents();
				}
			}
		}
		simscan.conc.clear();
		for (j=0; j<N; j++)
		{
			simscan.conc.push_back(CT0[j]);
		}
		(*simdata).scan.push_back(simscan);

/**
		if(kkk%1 == 0 || kkk<5)
		{
			for(j=0; j<N; j++)
			{
				fprintf(outf, "%12.5e %15.8e %15.8e ", simscan.time, x[j], CT0[j]);
			   for(i=0; i<Mcomp; i++) fprintf(outf, "%15.8e ", C0[i][j]);
			   fprintf(outf, "\n");
			}
			fprintf(outf, "\n\n");
         // printf("t=%12.5e C_ttl=%15.8e \n", simscan.time, IntConcentration(x, CT0));
		}
**/

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
				tridiag(CA[i][0], CA[i][1], CA[i][2], right_hand_side, C1[i], N);
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

				QuadSolver(CA[i][0], CA[i][1], CA[i][2], CA[i][3], right_hand_side, C1[i], N);
				// QuadSolver_ellam(CA[i][0], CA[i][1], CA[i][2], CA[i][3], right_hand_side, C1[i], N);
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
				tridiag(CA[i][0], CA[i][1], CA[i][2], right_hand_side, C1[i], N);
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
				QuadSolver(CA[i][0], CA[i][1], CA[i][2], CA[i][3], right_hand_side, C1[i], N);
				// QuadSolver_ellam(CA[i][0], CA[i][1], CA[i][2], CA[i][3], right_hand_side, C1[i], N);
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
//	fclose(outf);
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


//************ new version *************

//////////////////////////////////////////////////////////////%
//
// Generate exponential mesh and refine cell bottom (for s>0)
//
//////////////////////////////////////////////////////////////%
void US_Astfem_RSA::mesh_gen_s_pos(vector <double> nu)
{
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
		tmp_xc = af_params.current_bottom - (1.0/(nu[i] * af_params.current_bottom)) * log(nu[i]
				* (pow((double) af_params.current_bottom, (double) 2.0)
				- pow((double) af_params.current_meniscus, (double) 2.0))/(2.0 * uth));
		tmp_Nf = (int) (M_PI/2.0 * (af_params.current_bottom - tmp_xc)
				* nu[i] * af_params.current_bottom/2.0 + 0.5) + 1; // # of pts for i-th layer
		tmp_Hstar = (af_params.current_bottom - tmp_xc)/tmp_Nf * M_PI/2.0;			// step required by Pac(i)<1
		if ((tmp_xc > af_params.current_meniscus) &&
			(af_params.current_bottom - af_params.current_meniscus *
			pow((double) af_params.current_bottom/af_params.current_meniscus,
			(double)((af_params.simpoints-4.0/2.0)/(af_params.simpoints-1.0))) > tmp_Hstar))
//			(double)((af_params.simpoints-5.0/2.0)/(af_params.simpoints-1.0))) > tmp_Hstar))
		{
			xc.push_back(tmp_xc);
			Nf.push_back(tmp_Nf);
			Hstar.push_back(tmp_Hstar);
			IndLayer ++;
		}
	}
	xc.push_back(af_params.current_bottom);
	// print_vector(&xc);

	if (IndLayer == 0)	// use Schuck's grid only
	{
		x.push_back(af_params.current_meniscus);
//		for(i=1; i<af_params.simpoints ; i++)	// add one more point to Schuck's grids
		for(i=1; i<af_params.simpoints -1 ; i++)	// add one more point to Schuck's grids
		{
			x.push_back(af_params.current_meniscus * pow((double) (af_params.current_bottom/af_params.current_meniscus),
			(((double) i - 0.0)/((double)(af_params.simpoints - 1)))));	// Schuck's mesh
//			(((double) i - 0.5)/((double)(af_params.simpoints - 1)))));	// Schuck's mesh
		}
		x.push_back(af_params.current_bottom);
	}
	else				// need a composite grid
	{
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
					y.push_back(xc[i] + (af_params.current_bottom - xc[i]) * sin(j/(Nf[i] - 1.0) * M_PI/2.0));
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
		unsigned int Nm = (unsigned int) (floor(log(af_params.current_bottom/((af_params.simpoints - 1) * Hf)
				* log(af_params.current_bottom/af_params.current_meniscus))/log(2.0))+1) ; // number of pts in trans region
		xa = y[NfTotal-1] - Hf * (pow(2.0, (double)Nm) -1.);
		unsigned int Js = (unsigned int) (floor(0.0 + (af_params.simpoints - 1)
				* log(xa/af_params.current_meniscus)/log(af_params.current_bottom/af_params.current_meniscus)));
// xa is  modified so that y[NfTotal-Nm] matches xa exactly
		xa = af_params.current_meniscus*pow((double)(af_params.current_bottom/af_params.current_meniscus),
				(((double) Js - 0.0)/((double)af_params.simpoints - 1.0)));
      tmp_xc = y[NfTotal-1];
      HL = xa * (1.-af_params.current_meniscus/af_params.current_bottom);
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

		// printf("xa=%15.8e Hf=%12.5e Nm=%d, Js=%d, NfTotal=%d\n", xa, HR, Nm, Js, NfTotal);

// regular region
		x.push_back(af_params.current_meniscus);
		for (j=1; j<=Js; j++)
		{
			x.push_back(af_params.current_meniscus*
			pow((double) (af_params.current_bottom/af_params.current_meniscus),
			(((double) j - 0.0)/((double)af_params.simpoints - 1.0))));
		}

		for (j=0; j<NfTotal+Nm-1; j++)
		{
			x.push_back(y[NfTotal+Nm-j-2]);
		}

	}
}

//////////////////////////////////////////////////////////////%
//
// Generate exponential mesh and refine cell meniscus (for  s<0)
//
//////////////////////////////////////////////////////////////%
void US_Astfem_RSA::mesh_gen_s_neg(vector <double> nu)
{
	unsigned int j, Js, Nf, Nm;
	double uth = 1.0/af_params.simpoints;		// threshold of u for steep region
	double xc, xa, Hstar;
	vector <double> yr, ys, yt;

	x.clear();
	yr.clear();
	ys.clear();
	yt.clear();

	double nu0 = nu[0];
	xc = af_params.current_meniscus + 1./(fabs(nu0) * af_params.current_meniscus) *
	log((pow(af_params.current_bottom, 2.0) - pow(af_params.current_meniscus, 2.0)) * fabs(nu0)/(2.0*uth));
	Nf = (unsigned int) (1 + (floor)( (xc - af_params.current_meniscus) * fabs(nu0) * af_params.current_meniscus * M_PI/4.0));
	Hstar = (xc - af_params.current_meniscus)/Nf * M_PI/2.0;
	Nm = (unsigned int) (1 + (floor) (log(af_params.current_meniscus/((af_params.simpoints - 1.) * Hstar)
			* log(af_params.current_bottom/af_params.current_meniscus))/log(2.0)));
	xa = xc + (pow(2.0, (double) Nm) - 1.0) * Hstar;
	Js = (unsigned int) ((floor)((af_params.simpoints - 1) * log(af_params.current_bottom/xa)/log(af_params.current_bottom/af_params.current_meniscus) + 0.5 ));
	// printf("Nf=%d Nm=%d Js=%d \n", Nf, Nm, Js);
	// printf("xc=%12.5e xa=%12.5e \n", xc, xa);

// all grid points at exponentials
	yr.push_back(af_params.current_bottom);
//	for(j=1; j<(int) af_params.simpoints-1; j++)		// standard Schuck's grids
	for(j=1; j<af_params.simpoints; j++)		// add one more point to Schuck's grids
	{
		yr.push_back(af_params.current_bottom * pow((*simparams).meniscus/af_params.current_bottom, (j - 0.5)/(af_params.simpoints - 1.0)));
	}
	yr.push_back(af_params.current_meniscus);
	if(af_params.current_bottom * (pow(af_params.current_meniscus/af_params.current_bottom, (af_params.simpoints - 3.5)/(af_params.simpoints - 1.0))
	- pow(af_params.current_meniscus/af_params.current_bottom, (af_params.simpoints - 2.5)/(af_params.simpoints - 1.0))) < Hstar || Nf <= 2 )
	{
// no need for steep region
		for(j=0; j<af_params.simpoints; j++)
		{
			x.push_back(yr[af_params.simpoints - 1 - j]);
		}
		if (guiFlag)
		{
		    cerr << "use exponential grid only!\n" << endl;
		}
	}
	else
	{// Nf>2
		for(j=1;j<Nf;j++)
		{
			ys.push_back(xc - (xc - af_params.current_meniscus) * sin((j - 1.0)/(Nf - 1.0) * M_PI/2.0));
		}
		ys.push_back(af_params.current_meniscus);
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
		cerr << "no refinement at ends since sedimentation and floating mixed ...\n" ;
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
//				cout << "exponential mesh plus refinement at bottom, for s>0 ...\n";
				mesh_gen_s_pos(nu);
			}
			else if ( nu[ nu.size()-1 ] < 0 )
			{
//				cout << "exponential mesh plus refinement at meniscus, for s<0 ...\n";
				mesh_gen_s_neg(nu);
			}
			else							// some species with s<0 and some with s>0
			{
//				cout << "multicomponent system with sedimentation and floating mixed, use uniform mesh...\n";
				for ( unsigned int i=0; i<af_params.simpoints; i++)
				{
					x.push_back(af_params.current_meniscus + (af_params.current_bottom -
					af_params.current_meniscus) * i/(af_params.simpoints-1));
				}
			}
			break;
		}
		case 1: //Claverie mesh without left hand refinement
		{
//			cout << "using uniform mesh ...\n";
			for ( unsigned int i=0; i<af_params.simpoints; i++)
			{
				x.push_back(af_params.current_meniscus + (af_params.current_bottom -
				af_params.current_meniscus) * i/(af_params.simpoints-1));
			}
			break;
		}
		case 2: //Moving Hat (Peter Schuck's Mesh) without left hand side refinement
		{
			//cout << "using moving hat mesh...\n";
			x.push_back(af_params.current_meniscus);
			for(unsigned int i=1; i<af_params.simpoints-1; i++)  // standard Schuck's grids
			{
				x.push_back(af_params.current_meniscus * pow((double) (af_params.current_bottom/af_params.current_meniscus),
				(((double) i - 0.5)/((double)(af_params.simpoints-1)))));	// Schuck's mesh
			}
			x.push_back(af_params.current_bottom);
			break;
		}
		case 3: // user defined mesh generated from data file
		{
			//cout << "using mesh from file $ULTRASCAN/mesh.dat...\n";
			// QString str = getenv("ULTRASCAN");

#if defined(USE_MPI)
			cout << "using mesh from file $ULTRASCAN/mesh.dat...\n";
			QString str = getenv("ULTRASCAN");
#else
			US_Config* USglobal = new US_Config();
			QString    str      = USglobal->config_list.system_dir;
#endif

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
				if (fabs(x[0] - af_params.current_meniscus) > 1.0e7)
				{
					cerr << "The meniscus from the mesh file does not match the set meniscus - using Claverie Mesh instead\n";
				}
				if (fabs(x[x.size()-1] - af_params.current_bottom) > 1.0e7)
				{
					cerr << "The cell bottom from the mesh file does not match the set cell bottom - using Claverie Mesh instead\n";
				}
			}
			else
			{
				cerr << tr("Could not read the mesh file - using Claverie Mesh instead\n");
				for (unsigned int i=0; i<af_params.simpoints; i++)
				{
					x.push_back(af_params.current_meniscus + (af_params.current_bottom -
					af_params.current_meniscus) * i/(af_params.simpoints-1));
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
	//cout << "total number of points = " << N << "\n";
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
	IntQT1(vx, D, sw2, Stif[0], af_params.dt);

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
		IntQTm(vx, D, sw2, Stif[i], af_params.dt);
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

	IntQTn2(vx, D, sw2, Stif[N-2], af_params.dt);

	// last elems
	vx.clear();
	vx.push_back(x[N-2]);
	vx.push_back(x[N-1]);
	vx.push_back(x[N-1]);
	vx.push_back((*xb)[N-1]);
	IntQTn1(vx, D, sw2, Stif[N-1], af_params.dt);

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

	clear_3d(N, 6, Stif);
}


//
// ************* ELLAM ***********
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
	IntQT1_ellam(vx, D, sw2, Stif[0], af_params.dt);

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
		IntQTm_ellam(vx, D, sw2, Stif[i], af_params.dt);
	}


	// last elems
	vx.clear();
	vx.push_back(x[N-2]);
	vx.push_back(x[N-1]);
	vx.push_back(x[N-2]);
	vx.push_back(x[N-1]);
	vx.push_back((*xb)[N-1]);
	IntQTn1_ellam(vx, D, sw2, Stif[N-1], af_params.dt);

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
	unsigned int i, j;

	for (j=0; j<N; j++)
	{
		ct = 0.;
		for (i=0; i<af_params.s.size(); i++)
		{
			ct += C1[i][j];
		}
		if((*system).model >= 4 && (*system).model <= 10)  // monomer - (n)mer
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
				switch((*system).model)
				{
					case 4:	// mono <--> dimer
                  if (dvb*dvb+4*dva*dvc<=0)
                  {
						   uhat = C1[0][j];
                  }
                  else
                  {
						   uhat = 2*dvc / ( dvb+sqrt(dvb*dvb+4.*dva*dvc) );
                  }
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
		else if((*system).model == 11 )  	//  A + B <--> AB
		{
         double exta = 1.0, extb=1.0;			// extinction rate for A and B, should be specified outside
         double K1, K_1;			// reation rates
			double dva, dvb, dvc, dC[3];

         K1  = af_params.koff[0] * af_params.keq[0];
         K_1 = af_params.koff[0] ;

			dva = TimeStep * K1 * extb;
			dvb = 2 + TimeStep * ( K1*(extb*C1[0][j] + exta*C1[1][j]) + K_1*(exta+extb) );
			dvc = TimeStep * exta * ( K1*C1[0][j]*C1[1][j] - K_1*C1[2][j] );

         dC[0] = -2*dvc / ( dvb + sqrt( dvb*dvb - 4*dva*dvc ) );
         dC[1] = extb / exta * dC[0];
         dC[2] = - ( dC[0] + dC[1] );

			C1[0][j] += 2*dC[0];
			C1[1][j] += 2*dC[1];
			C1[2][j] += 2*dC[2];
		}
		else if((*system).model == 12 )  	//  general reaction
      {
         unsigned int Nspec = af_params.s.size();	// number of species
         unsigned int iter_max = 20; 		// maximum number of Newton iteration allowed
         unsigned int i, k, iter;
         double **A, *y0, *delta_n, *b, diff;

         y0 = new double [Nspec];
         delta_n = new double [Nspec];
         b = new double [Nspec];
         A = new double* [Nspec];
         for(i=0;i<Nspec;i++) A[i] = new double [Nspec];

         for(i=0;i<Nspec;i++)
         {
            y0[i]=C1[i][j];
            delta_n[i]=0.;
         }

         for(iter=0; iter<iter_max;iter++) 		// Newton iteration
         {
            for(i=0;i<Nspec;i++)
              y0[i]=C1[i][j]+delta_n[i];

            Reaction_dydt(y0, b);						// b=dy/dt

            Reaction_dfdy(y0, A);						// A=df/dy

            for(i=0;i<Nspec;i++)
            {
              for(k=0;k<Nspec;k++) A[i][k] = -TimeStep * A[i][k];
              A[i][i] += 2.;
              b[i] = 2.*delta_n[i] - TimeStep * b[i];
            }

            if( GaussElim(Nspec, A, b) ==-1 )
            {
              printf("Matrix singular in Reaction_Euler_imp: model 12\n");
              break;
            }
            else
            {
              diff = 0.;
              for(i=0;i<Nspec;i++)
              {
                delta_n[i] += b[i];
                diff += fabs(delta_n[i]);
              }
            }

            if(diff <1.e-7*ct) break;

         } // end of Newton iteration;
         for(i=0;i<Nspec;i++) C1[i][j] += delta_n[i];

         for(i=0;i<Nspec;i++)
			{
				delete [] A[i];
			}
         delete [] A;
         delete [] b;
         delete [] delta_n;
         delete [] y0;
      }
		else
		{
			cerr << "warning: The reaction for model " << (*system).model << " has not yet been implemented" << endl;
		}
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

		if ((*system).model == 4) // monomer-dimer
		{
			C1 = (sqrt(1.0 + 4.0 * af_params.keq[0] * dval) - 1.0)/(2.0 * af_params.keq[0]) ;
			C[0] = C1;
			C[1] = af_params.keq[0] * pow(C1, (double) af_params.n[1]);
		}
		else if ((*system).model == 5) // monomer-trimer
		{
			C1 = cube_root(-dval/af_params.keq[0], 1.0/af_params.keq[0], 0.0);
			C[0] = C1;
			C[1] = af_params.keq[0] * pow(C1, (double) af_params.n[1]);
		}
		else if ((*system).model > 5 && (*system).model < 11)
		{
			C1 = find_C1_mono_Nmer( af_params.n[1], af_params.keq[0], dval );
			C[0] = C1;
			C[1] = af_params.keq[0] * pow(C1, (double) af_params.n[1]);
		}
		else if ((*system).model == 11)// monomer-dimer-trimer system
		{
			C1 = cube_root(-dval/af_params.keq[1], 1.0/af_params.keq[1], af_params.keq[0]/af_params.keq[1]);
			C[0] = C1;
			C[1] = af_params.keq[0] * pow(C1, (double) af_params.n[1]);
			C[2] = af_params.keq[1] * pow(C1, (double) af_params.n[2]);
		}
		else
		{
			cerr << "warning: model #" << (*system).model << " is not yet supported in find_C1()" << endl;
			return (-1);
		}
	   return (0);
}


void US_Astfem_RSA::Reaction_dydt(double *y, double *yt)
{
	switch ( (*system).model )
   {
      case 12:						// n A <--> An,   m An <--> Anm
      {
         int n, m;			// should be supplied by the model
			double extA;		// extinction rate of A, should be supplied by the model
			double K1, K_1, K2, K_2, Q1, Q2;

         K1  = af_params.koff[0] * af_params.keq[0];
         K2  = af_params.koff[1] * af_params.keq[1];
         K_1 = af_params.koff[0] ;
         K_2 = af_params.koff[1] ;

         Q1 = K1 * pow(y[0], n) - K_1 * y[1];
         Q2 = K2 * pow(y[1], m) - K_2 * y[2];

         yt[0] = -double(n) * extA * Q1;
         yt[1] =  double(n) * extA * ( Q1 - double(m) * Q2);
         yt[2] =  double(m*n) * extA * Q2;

         break;
      }
		default:
		{
			cerr << "undefined reaction model \n";
      }
   }
}

void US_Astfem_RSA::Reaction_dfdy(double *y, double **dfdy)
{
	switch ( (*system).model )
   {
      case 12:						// n A <--> An,   m An <--> Anm
      {
         int n, m;			// should be supplied by the model
			double extA;		// extinction rate of A, should be supplied by the model
			double K1, K_1, K2, K_2;

         K1  = af_params.koff[0] * af_params.keq[0];
         K2  = af_params.koff[1] * af_params.keq[1];
         K_1 = af_params.koff[0] ;
         K_2 = af_params.koff[1] ;

         dfdy[0][0] = -double(n) * extA * ( K1 * double(n) * pow(y[0], n-1) );
         dfdy[0][1] = -double(n) * extA * ( -K_1 );
         dfdy[0][2] =  0.;

         dfdy[2][0] =  0.;
         dfdy[2][1] =  double(m*n) * extA * ( K2 * double(m) * pow(y[1], m-1) );
         dfdy[2][2] =  double(m*n) * extA * ( -K_2 );

         dfdy[1][0] =  dfdy[0][0] - dfdy[2][0];
         dfdy[1][1] =  dfdy[0][1] - dfdy[2][1];
         dfdy[1][2] =  dfdy[0][2] - dfdy[2][2];

         break;
      }
		default:
		{
			cerr << "undefined reaction model \n";
		}
   }
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

      if ( i == 0 ) 				// x[j] < C0.radius[0]
      {
         C1[j] = (*C0).concentration[0];		// use the first value
      }
      else if ( i == (*C0).radius.size() ) 	// x[j] > last point in (*C0).radius
      {
         C1[j] = (*C0).concentration[ i-1 ];
      }
      else
      {
		   a = (*C0).radius[i-1];
		   b = (*C0).radius[i];
		   tmp = (xs - a)/(b - a);
		   C1[j] = (*C0).concentration[i-1] * (1. - tmp) + (*C0).concentration[i] * tmp;
		   ja = i-1;
      }
	}
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

      if ( i == 0 ) 				// xs < x[0]
      {
         (*C0).concentration[j] = cfinal[0];		// use the first value
      }
      else if ( i == N ) 	   // xs > x[N]
      {
         (*C0).concentration[j] = cfinal[ i-1 ];
      }
      else 							// x[i-1] < xs <x[i]
      {
		   a = x[i-1];
		   b = x[i];
		   tmp = (xs-a)/(b-a);
		   (*C0).concentration[j] = cfinal[i-1] * (1. - tmp) + cfinal[i] * tmp;
		   ja = i-1;
      }
	}
}

void US_Astfem_RSA::adjust_limits(unsigned int speed)
{
	// first correct meniscus to theoretical position at rest:
	double stretch_val = stretch((*simparams).rotor, af_params.first_speed);
//	cout << "rotor: " << (*simparams).rotor << ", stretch: " << stretch_val << endl;
	// this is the meniscus at rest
	af_params.current_meniscus = (*simparams).meniscus - stretch_val;
//	cout << "1st speed meniscus: " << (*simparams).meniscus << ", rest meniscus: " << af_params.current_meniscus << endl;
	// calculate rotor stretch at current speed
	stretch_val = stretch((*simparams).rotor, speed);
	// add current stretch to meniscus at rest
	af_params.current_meniscus +=  stretch_val;
	// add current stretch to bottom at rest
	af_params.current_bottom = (*simparams).bottom + stretch_val;
//	cout << "corrected meniscus: " << af_params.current_meniscus << ", current_bottom: " << af_params.current_bottom << endl;
}

void US_Astfem_RSA::adjust_grid(unsigned int old_speed, unsigned int new_speed, vector <double> *radius)
{
	double stretch_val1 = stretch((*simparams).rotor, old_speed);
	double stretch_val2 = stretch((*simparams).rotor, new_speed);
	for (unsigned int i=0; i<(*radius).size(); i++)
	{
		(*radius)[i] += stretch_val2 - stretch_val1;
	}
}

void US_Astfem_RSA::print_vector(double *dval, unsigned int ival)
{
	unsigned int i;
	for (i=0; i<ival; i++)
	{
		printf("x[%d]=%20.10e \n", i, dval[i]);
	}
}

void US_Astfem_RSA::print_vector(vector <double> *dval)
{
	unsigned int i;
	for (i=0; i<(*dval).size(); i++)
	{
		printf("x[%d]=%20.10e \n", i, (*dval)[i]);
	}
}

void US_Astfem_RSA::print_simparams()
{
}

void US_Astfem_RSA::print_af()
{
	unsigned int i;
	QString str;
	cout.precision(10);
	cout << "Model Number:\t" << (*system).model << endl;
	cout << "Rotor:\t" << (*simparams).rotor << endl;
	cout << "First speed:\t" << af_params.first_speed << endl;
	cout << "Simpoints:\t" << af_params.simpoints << endl;
	cout << "\nHydrodynamic Parameters:\n";
	for (i=0; i< af_params.s.size(); i++)
	{
		cout << "s[" << i << "]:\t\t" << af_params.s[i] << endl;
		cout << "D[" << i << "]:\t\t" << af_params.D[i] << endl;
	}
	cout << "\nEquilibrium constants:\n";
	for (i=0; i<af_params.keq.size(); i++)
	{
		cout <<  str.sprintf("#  keq[%d]=%12.5e \t",  i, af_params.keq[i]) << endl;
	}
	cout << "\nK_off rates:\n";
			for (i=0; i<af_params.koff.size(); i++)
	{
		cout << str.sprintf("#  koff[%d]=%12.5e \t", i, af_params.koff[i]) << endl;
	}
	cout << "\nExponents:\n";
	for (i=0; i<af_params.n.size(); i++)
	{
		cout << str.sprintf("#  exponent[%d]=%d \n", i, af_params.n[i]) << endl;
	}
	cout << "\ndt:\t\t" << af_params.dt << endl;
	cout << "time_steps:\t" << af_params.time_steps << endl;
	cout << "omega_s:\t" << af_params.omega_s << endl;
	cout << "start_time:\t" << af_params.start_time << endl;
	cout << "current meniscus:\t" << af_params.current_meniscus << endl;
	cout << "current bottom:\t\t" << af_params.current_bottom << endl;
	cout << "mesh:\t\t" << (*simparams).mesh << endl;
	cout << "moving grid\t\t" << af_params.moving_grid << endl;
	if (af_params.acceleration)
	{
		cout << "Acceleration is *on*\n";
	}
	else
	{
		cout << "Acceleration is *off*\n";
	}
}

void US_Astfem_RSA::print_af(FILE *outf)
{
	unsigned int i;

	fprintf(outf, "#####################################################\n");
	fprintf(outf, "#  \n");
	fprintf(outf, "#  Model Number: %d \n", (*system).model);
	fprintf(outf, "#  Number of species = %d\n", af_params.s.size() );
	for (i=0; i<af_params.s.size(); i++)
	{
		fprintf(outf, "#  s[%d]=%20.12e D[%d]=%20.12e \n", i, af_params.s[i], i, af_params.D[i]);
	}
	fprintf(outf, "#  \n");
	fprintf(outf, "#  parameters for reactions:\n");
	for (i=0; i<af_params.keq.size(); i++)
	{
		fprintf(outf, "#  keq[%d]=%12.5e \t",  i, af_params.keq[i]);
	}
	fprintf(outf, "\n");
	for (i=0; i<af_params.koff.size(); i++)
	{
		fprintf(outf, "#  koff[%d]=%12.5e \t", i, af_params.koff[i]);
	}
	fprintf(outf, "\n");
	for (i=0; i<af_params.n.size(); i++)
	{
		fprintf(outf, "#  exponent[%d]=%d \n", i, af_params.n[i]);
	}
	fprintf(outf, "#  \n");
	fprintf(outf, "#  parameters for simulation:\n");
	fprintf(outf, "#  current meniscus =%12.5e \n",  af_params.current_meniscus);
	fprintf(outf, "#  current bottom =%12.5e \n",  af_params.current_bottom);
	fprintf(outf, "#  start time =%12.5e \n",  af_params.start_time);
	fprintf(outf, "#  mesh opt =%d \n",  (*simparams).mesh);
	if (af_params.moving_grid) fprintf(outf, "#  grids = moving \n");
	else fprintf(outf, "#  grids = fixed \n");
	if (af_params.acceleration) fprintf(outf, "#  acceleration = True \n");
	else fprintf(outf, "#  acceleration = False \n");
	fprintf(outf, "#  simpoints =%d \n",  af_params.simpoints);
	fprintf(outf, "#  dt =%12.5e \n",  af_params.dt);
	fprintf(outf, "#  Total Number of Steps =%d \n",  af_params.time_steps);

	fprintf(outf, "#  \n");
	fprintf(outf, "#####################################################\n");

	return;
}

