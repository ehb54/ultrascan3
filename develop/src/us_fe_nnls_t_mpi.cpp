#define NO_US
#include "../include/us_fe_nnls_t.h"
#include "../include/us_ga.h"
#include "../include/us_ga_round.h"
// #define US_DEBUG_MPI
// #define SLIST
// #define SLIST2
#include <mpi.h>
#include <stdio.h>
#define MIN_EXPERIMENT_SIZE 100
// MIN_EXPERIMENT_SIZE is so that unions have room to work with extremely small grids
// #define MAX_ITERATIONS 15
// MAX_ITERATION could be a user controlled parameter, but in case a system is unstable(? have not seen any yet!)
#define USE_ALPHA .95
// USE_ALPHA is for meniscus fitting regularization
#define VARIANCE_IMPROVEMENT_TOLERANCE 1e-100


#if defined(TESTING)
double float_mc_edge_max = 20;
#else
double float_mc_edge_max = 0;
#endif
double float_mc_edge_inc;

extern int npes;
extern int myrank;
MPI_Status mpi_status, mpi_status2;
int monte_carlo_iterations = 1;
int this_monte_carlo = 0;

static vector<struct mfem_data> org_experiment;
static vector<struct mfem_data> last_residuals;
static vector<struct mfem_data> save_gaussians;

// only define one of these _TIMING!
#define GLOBAL_JOB_TIMING
// #define SHOW_TIMING
// #define JOB_TIMING
// #define JOB_TIMING_CR
// #define JOB_TIMING_CRMP
#if defined(JOB_TIMING)
# undef SHOW_TIMING
# undef SLIST
# undef SLIST2
#endif
#if defined(SHOW_TIMING) || defined(JOB_TIMING) || defined(GLOBAL_JOB_TIMING)
# include <sys/time.h>
static struct timeval start_tv, end_tv;
# if defined(JOB_TIMING_CR)
struct timeval start_tv_cr, end_tv_cr;
# endif
#endif

long maxrss;

static void clear_data(mfem_data *d)
{
	unsigned int i;
	for (i=0; i<(*d).scan.size(); i++)
	{
		(*d).scan[i].conc.clear();
	}
	(*d).radius.clear();
	(*d).scan.clear();
}

US_fe_nnls_t::US_fe_nnls_t()
// US_fe_nnls_t::US_fe_nnls_t(QWidget * p, const char *name):QWidget(p, name)
{
}

US_fe_nnls_t::~US_fe_nnls_t()
{}

QString tr(QString s)
{
	return s;
}

Simulation_values US_fe_nnls_t::regularize(Simulation_values org_sv, double use_meniscus)
{
	// take a set of vectors, compute regularized fitness
	// delete minimum concentration, repeat until regularized fitness goes up

	unsigned int i;
	double min_c;
	int min_c_pos;
	double fitness;
	double reg_fitness = 1e99;
	vector<Solute> prev_solutes;
	vector<Solute> solutes;
	vector<Solute_vector> solute_vectors;
	Solute_vector solute_vector;
	Solute solute;
	Simulation_values sv;
	solutes = org_sv.solutes;
	do
	{
		sv.solutes = solutes;
		sv = calc_residuals(experiment, sv.solutes, use_meniscus, 0);
		prev_solutes = sv.solutes;
		fitness = sv.variance;
		solutes.clear();
		min_c = 1e99;
		min_c_pos = -1;
		for (i = 0; i < prev_solutes.size(); i++)
		{
			if (prev_solutes[i].c < min_c)
			{
				min_c = prev_solutes[i].c;
				min_c_pos = i;
			}
		}

		for (i = 0; i < prev_solutes.size(); i++)
		{
			if (prev_solutes[i].c > 0)
			{
				solutes.push_back(prev_solutes[i]);
			}
		}
		reg_fitness = pow(sqrt(fitness) * (1e0 + (1e0 - regularization) * solutes.size()),2e0);
		solute_vector.solutes = solutes;
		solute_vector.fitness = fitness;
		solute_vector.reg_fitness = reg_fitness;
		solute_vectors.push_back(solute_vector);
#if defined(DEBUG)

		printf("solutes %d fitness %g regularized %g\n", solutes.size(), fitness, reg_fitness);
		printf("min_c_pos %d %g\n", min_c_pos, min_c);
#endif

		// now remove the mininum one
		solutes.clear();
		for (i = 0; i < prev_solutes.size(); i++)
		{
			if (prev_solutes[i].c > 0 &&
					i != (unsigned) min_c_pos)
			{
				solutes.push_back(prev_solutes[i]);
			}
		}
	}
	while(solutes.size() > 0);

	double min_reg_fitness = 1e99;
	int min_reg_fitness_pos = 0;
	for (i = 0; i < solute_vectors.size(); i++)
	{
		if (solute_vectors[i].reg_fitness < min_reg_fitness)
		{
			min_reg_fitness = solute_vectors[i].reg_fitness;
			min_reg_fitness_pos = i;
		}
	}
	sv.solutes = solute_vectors[min_reg_fitness_pos].solutes;
	sv = calc_residuals(experiment, sv.solutes, use_meniscus, 0);
	// solutes = solute_vectors[min_reg_fitness_pos].solutes;
	// variance = solute_vectors[min_reg_fitness_pos].fitness;
#if defined(SLIST)

	{
		unsigned int i;
		vector<Solute> solutes = sv.solutes;
		puts("s\tk\tc");
		for (i = 0; i < solutes.size(); i++)
		{
			printf("%d\t%g\t%g\t%g\n",
				   i,
				   solutes[i].s,
				   solutes[i].k,
				   solutes[i].c);
		}
	}
#endif
	return sv;
}


static vector <struct mfem_data> get_gaussian_means(vector <struct mfem_data> res)
{
	// take an experiment & the residuals and return a set of gaussian smoothed means
	unsigned int e, j, k;
	unsigned int ssize = 5; // smoothing size;
	float *std_dev;
	vector <struct mfem_data> ret = res;
	for (e = 0; e < res.size(); e++)
	{
		for (j = 0; j < res[e].scan.size(); j++)
		{
			std_dev = new float[res[e].radius.size()];
			for (k = 0; k < res[e].radius.size(); k++)
			{
				std_dev[k] = fabs(res[e].scan[j].conc[k]);
			}
			gaussian_smoothing(&std_dev, ssize, res[e].radius.size());
			for (k = 0; k < res[e].radius.size(); k++)
			{
				ret[e].scan[j].conc[k] = res[e].scan[j].conc[k] >= 0 ? std_dev[k] : -std_dev[k];
			}
			delete std_dev;
		}
	}
	return ret;
}

static vector <struct mfem_data> get_monte_carlo(vector <struct mfem_data> exp, vector <struct mfem_data> means)
{
	// take an experiment & the means & return a new experiment
	unsigned int e, j, k;
	vector <struct mfem_data> ret = exp;
	for (e = 0; e < exp.size(); e++)
	{
		for (j = 0; j < exp[e].scan.size(); j++)
		{
			for (k = 0; k < exp[e].radius.size(); k++)
			{
				ret[e].scan[j].conc[k] += box_muller(0, means[e].scan[j].conc[k]);
			}
		}
	}
	return ret;
}


void US_fe_nnls_t::WriteResults(vector <struct mfem_data> experiment, vector<Solute> solutes,
								vector <Simulation_values> sve, QString tag, double meniscus, unsigned int iterations)
{
	// this one is for the GA with possible global results
  //	printf("0: writeresults 1 it %u mc %d\n", iterations, monte_carlo_iterations);
  //	fflush(stdout);
	unsigned int e;
	unsigned int ti_noise_offset = 0;
	unsigned int ri_noise_offset = 0;
	if (experiment.size() > 1)
	{
		QString cellwave;
		cellwave = cellwave.sprintf(".%d%d", experiment[0].cell + 1, experiment[0].wavelength + 1);
		QString filenametags;
		if (monte_carlo_iterations <= 1)
		{
			filenametags = experiment[0].id + "_" + analysis_type + "_global" + tag + "_" + startDateTime.toString("yyMMddhhmmss");
		}
		else
		{
			filenametags = experiment[0].id + "_" + analysis_type + "_MonteCarlo_" + QString("%1").arg(monte_carlo_iterations) + "_global" + tag + "_" + startDateTime.toString("yyMMddhhmmss");
		}
		{
			QFile f(filenametags + ".model" + cellwave);
			unsigned int i;
			if (f.open(IO_WriteOnly))
			{
				QTextStream ts(&f);
				if (monte_carlo_iterations > 1)
				{
					ts << analysis_type << "_MC" << endl;
					ts << monte_carlo_iterations << endl;
				}
				else
				{
					ts << analysis_type << endl;
				}
				ts << experiment[0].id << "_global" << endl;
				ts << "0" << endl;
				ts << solutes.size() << endl;
				for (i = 0; i < solutes.size(); i++)
				{
					ts << (solutes[i].c / experiment.size()) << endl;
					//	if (analysis_type == "GA_MW") {
					//		ts <<
					//		pow(pow((solutes[i].s * experiment[0].vbar20)/(AVOGADRO * M_PI), 2e0)/6e0,1e0/3e0) *
					//		(1 - experiment[0].density * experiment[0].vbar20) /
					//		(3 * solutes[i].k * experiment[0].vbar20 * experiment[0].viscosity * .01) << endl;
					//	} else {
					ts << solutes[i].s << endl;
					//	}
					ts << (R * K20/(AVOGADRO * solutes[i].k * 9.0 * VISC_20W * M_PI
									   * pow( (2.0 * solutes[i].s * solutes[i].k *
											   experiment[0].vbar20 * VISC_20W)/(1.0 - experiment[0].vbar20 * DENS_20W), 0.5)))
					<< endl;
					ts << "0\n0\n";
				}
			}
			f.close();
			QFile f2("email_list_" + startDateTime.toString("yyMMddhhmmss"));
			if (f2.open(IO_WriteOnly | IO_Append))
			{
				QTextStream ts(&f2);
				ts << filenametags + ".model" + cellwave << endl;
			}
			f2.close();
		}
	}
	for (e = 0; e < experiment.size(); e++)
	{
		QString cellwave;
		cellwave = cellwave.sprintf(".%d%d", experiment[e].cell + 1, experiment[e].wavelength + 1);
		QString filenametags;
		if (monte_carlo_iterations <= 1)
		{
			filenametags = experiment[e].id + "_" + analysis_type + tag + "_" + startDateTime.toString("yyMMddhhmmss");
		}
		else
		{
			filenametags = experiment[e].id + "_" + analysis_type + "_MonteCarlo_" + QString("%1").arg(monte_carlo_iterations) + tag + "_" + startDateTime.toString("yyMMddhhmmss");
		}
		{
			QFile f(filenametags + ".model" + cellwave);
			unsigned int i;
			if (f.open(IO_WriteOnly))
			{
				QTextStream ts(&f);
				if (monte_carlo_iterations > 1)
				{
					ts << analysis_type << "_MC" << endl;
					ts << monte_carlo_iterations << endl;
				}
				else
				{
					ts << analysis_type << endl;
				}
				ts << experiment[e].id << endl;
				ts << "0" << endl;
				ts << sve[e].solutes.size() << endl;
				for (i = 0; i < sve[e].solutes.size(); i++)
				{
					ts << sve[e].solutes[i].c << endl;
					//	if (analysis_type == "GA_MW") {
					//		ts <<
					//		pow(pow((sve[e].solutes[i].s * experiment[e].vbar20)/(AVOGADRO * M_PI), 2e0)/6e0,1e0/3e0) *
					//		(1 - experiment[e].density * experiment[e].vbar20) /
					//		(3 * sve[e].solutes[i].k * experiment[e].vbar20 * experiment[e].viscosity * .01) << endl;
					//	} else {
					ts << sve[e].solutes[i].s << endl;
					//	}
					ts << (R * K20/(AVOGADRO * sve[e].solutes[i].k * 9.0 * VISC_20W * M_PI
									   * pow( (2.0 * sve[e].solutes[i].s * sve[e].solutes[i].k *
											   experiment[e].vbar20 * VISC_20W)/(1.0 - experiment[e].vbar20 * DENS_20W), 0.5)))
					<< endl;
					ts << "0\n0\n";
				}
			}
			f.close();
			QFile f2("email_list_" + startDateTime.toString("yyMMddhhmmss"));
			if (f2.open(IO_WriteOnly | IO_Append))
			{
				QTextStream ts(&f2);
				ts << filenametags + ".model" + cellwave << endl;
			}
			f2.close();
		}
		if (fit_tinoise)
		{
			QFile f(filenametags + ".ti_noise" + cellwave);
			unsigned int i;
			if (f.open(IO_WriteOnly))
			{
				QTextStream ts(&f);
				//	ts << experiment[e].id << endl;
				//	ts << sv.ti_noise.size() << endl;
				//	for (i = 0; i < sv.ti_noise.size(); i++) {
				for (i = 0; i < experiment[e].radius.size(); i++)
				{
					ts << sve[e].ti_noise[i] << endl;
				}
			}
			f.close();
			QFile f2("email_list_" + startDateTime.toString("yyMMddhhmmss"));
			if (f2.open(IO_WriteOnly | IO_Append))
			{
				QTextStream ts(&f2);
				ts << filenametags + ".ti_noise" + cellwave << endl;
			}
			f2.close();
		}
		if (fit_rinoise)
		{
			QFile f(filenametags + ".ri_noise" + cellwave);
			unsigned int i;
			if (f.open(IO_WriteOnly))
			{
				QTextStream ts(&f);
				//	ts << experiment[e].id << endl;
				//	ts << sv.ri_noise.size() << endl;
				//	for (i = 0; i < sv.ri_noise.size(); i++) {
				for (i = 0; i < experiment[e].scan.size(); i++)
				{
					ts << sve[e].ri_noise[i] << endl;
				}
			}
			f.close();
			QFile f2("email_list_" + startDateTime.toString("yyMMddhhmmss"));
			if (f2.open(IO_WriteOnly | IO_Append))
			{
				QTextStream ts(&f2);
				ts << filenametags + ".ri_noise" + cellwave << endl;
			}
			f2.close();
		}
		QFile f3("email_text_" + startDateTime.toString("yyMMddhhmmss"));
		if (f3.open(IO_WriteOnly | IO_Append))
		{
			QTextStream ts(&f3);
			if (meniscus != 0)
			{
				//	if (!e && experiment.size() > 1) {
				//	ts << QString("Global rmsd %1 meniscus offset %2\n").
				//		arg(sqrt(sve.variance)).
				//		arg(meniscus);
				//	}
				ts << QString("Experiment %1, cell %2, wavelength %3, meniscus %7,"
							  " solutes %4, rmsd %5, iterations %6\n").
				arg(experiment[e].id).arg(experiment[e].cell + 1).arg(experiment[e].wavelength + 1).
				arg(sve[e].solutes.size()).
				//		arg(sqrt(sve[e].variances[e])).
				arg(sqrt(sve[e].variance)).
				arg(iterations).
				arg(meniscus + experiment[e].meniscus);
			}
			else
			{
				//	if (!e && experiment.size() > 1) {
				//	ts << QString("Global rmsd %1\n").arg(sqrt(sv.variance));
				//	}
				ts << QString("Experiment %1, cell %2, wavelength %3,"
							  " solutes %4, rmsd %5, iterations %6\n").
				arg(experiment[e].id).arg(experiment[e].cell + 1).arg(experiment[e].wavelength + 1).
				arg(sve[e].solutes.size()).
				//		arg(sqrt(sve[e].variances[e])).
				arg(sqrt(sve[e].variance)).
				arg(iterations);
			}
		}
		f3.close();
		ri_noise_offset += experiment[e].scan.size();
		ti_noise_offset += experiment[e].radius.size();
	}
}

void US_fe_nnls_t::WriteResults(vector <struct mfem_data> experiment,
								Simulation_values sv, QString tag, double meniscus, unsigned int iterations)
{
  //	printf("0: writeresults 2 it %u mc %d\n", iterations, monte_carlo_iterations);
  //	fflush(stdout);
	unsigned int e;
	unsigned int ti_noise_offset = 0;
	unsigned int ri_noise_offset = 0;
	for (e = 0; e < experiment.size(); e++)
	{
		QString cellwave;
		cellwave = cellwave.sprintf(".%d%d", experiment[e].cell + 1, experiment[e].wavelength + 1);
		QString filenametags;
		if (monte_carlo_iterations <= 1)
		{
			filenametags = experiment[e].id + "_" + analysis_type + tag + "_" + startDateTime.toString("yyMMddhhmmss");
		}
		else
		{
			filenametags = experiment[e].id + "_" + analysis_type + "_MonteCarlo_" + QString("%1").arg(monte_carlo_iterations) + tag + "_" + startDateTime.toString("yyMMddhhmmss");
		}
		{
			QFile f(filenametags + ".model" + cellwave);
			unsigned int i;
			if (f.open(IO_WriteOnly))
			{
				QTextStream ts(&f);
				if (monte_carlo_iterations > 1)
				{
					ts << analysis_type << "_MC" << endl;
					ts << monte_carlo_iterations << endl;
				}
				else
				{
					ts << analysis_type << endl;
				}
				ts << experiment[e].id << endl;
				ts << "0" << endl;
				ts << sv.solutes.size() << endl;
				for (i = 0; i < sv.solutes.size(); i++)
				{
					ts << sv.solutes[i].c << endl;
					//	if (analysis_type == "GA_MW") {
					//		ts <<
					//		pow(pow((sv.solutes[i].s * experiment[e].vbar20)/(AVOGADRO * M_PI), 2e0)/6e0,1e0/3e0) *
					//		(1 - experiment[e].density * experiment[e].vbar20) /
					//		(3 * sv.solutes[i].k * experiment[e].vbar20 * experiment[e].viscosity * .01) << endl;
					//	} else {
					ts << sv.solutes[i].s << endl;
					//	}
					ts << (R * K20/(AVOGADRO * sv.solutes[i].k * 9.0 * VISC_20W * M_PI
									   * pow( (2.0 * sv.solutes[i].s * sv.solutes[i].k *
											   experiment[e].vbar20 * VISC_20W)/(1.0 - experiment[e].vbar20 * DENS_20W), 0.5)))
					<< endl;
					ts << "0\n0\n";
				}
			}
			f.close();
			QFile f2("email_list_" + startDateTime.toString("yyMMddhhmmss"));
			if (f2.open(IO_WriteOnly | IO_Append))
			{
				QTextStream ts(&f2);
				ts << filenametags + ".model" + cellwave << endl;
			}
			f2.close();
		}
		if (fit_tinoise)
		{
			QFile f(filenametags + ".ti_noise" + cellwave);
			unsigned int i;
			if (f.open(IO_WriteOnly))
			{
				QTextStream ts(&f);
				//	ts << experiment[e].id << endl;
				//	ts << sv.ti_noise.size() << endl;
				//	for (i = 0; i < sv.ti_noise.size(); i++) {
				for (i = 0; i < experiment[e].radius.size(); i++)
				{
					ts << sv.ti_noise[i + ti_noise_offset] << endl;
				}
			}
			f.close();
			QFile f2("email_list_" + startDateTime.toString("yyMMddhhmmss"));
			if (f2.open(IO_WriteOnly | IO_Append))
			{
				QTextStream ts(&f2);
				ts << filenametags + ".ti_noise" + cellwave << endl;
			}
			f2.close();
		}
		if (fit_rinoise)
		{
			QFile f(filenametags + ".ri_noise" + cellwave);
			unsigned int i;
			if (f.open(IO_WriteOnly))
			{
				QTextStream ts(&f);
				//	ts << experiment[e].id << endl;
				//	ts << sv.ri_noise.size() << endl;
				//	for (i = 0; i < sv.ri_noise.size(); i++) {
				for (i = 0; i < experiment[e].scan.size(); i++)
				{
					ts << sv.ri_noise[i + ri_noise_offset] << endl;
				}
			}
			f.close();
			QFile f2("email_list_" + startDateTime.toString("yyMMddhhmmss"));
			if (f2.open(IO_WriteOnly | IO_Append))
			{
				QTextStream ts(&f2);
				ts << filenametags + ".ri_noise" + cellwave << endl;
			}
			f2.close();
		}
		QFile f3("email_text_" + startDateTime.toString("yyMMddhhmmss"));
		if (f3.open(IO_WriteOnly | IO_Append))
		{
			QTextStream ts(&f3);
			if (meniscus != 0)
			{
				if (!e && experiment.size() > 1)
				{
					ts << QString("Global rmsd %1 meniscus offset %2\n").
					arg(sqrt(sv.variance)).
					arg(meniscus);
				}
				ts << QString("Experiment %1, cell %2, wavelength %3, meniscus %7,"
							  " solutes %4, rmsd %5, iterations %6\n").
				arg(experiment[e].id).arg(experiment[e].cell + 1).arg(experiment[e].wavelength + 1).
				arg(sv.solutes.size()).arg(sqrt(sv.variances[e])).
				arg(iterations).
				arg(meniscus + experiment[e].meniscus);
			}
			else
			{
				if (!e && experiment.size() > 1)
				{
					ts << QString("Global rmsd %1\n").arg(sqrt(sv.variance));
				}
				ts << QString("Experiment %1, cell %2, wavelength %3,"
							  " solutes %4, rmsd %5, iterations %6\n").
				arg(experiment[e].id).arg(experiment[e].cell + 1).arg(experiment[e].wavelength + 1).
				arg(sv.solutes.size()).arg(sqrt(sv.variances[e])).
				arg(iterations);
			}
		}
		f3.close();
		ri_noise_offset += experiment[e].scan.size();
		ti_noise_offset += experiment[e].radius.size();
	}
}


class Expdata
{
public:
	QString tag;
	vector<Solute> solutes;
	vector<Simulation_values> sve;
	Simulation_values sv;
	bool operator==(const Expdata& objIn)
	{
		return (tag == objIn.tag);
	}
};

list <Expdata> expdata_list;

void US_fe_nnls_t::BufferResults(vector <struct mfem_data> experiment, vector<Solute> solutes,
								 vector <Simulation_values> sve, QString tag, double meniscus, unsigned int iterations)
{
	// this one is for the GA with possible global results
  //	printf("0: BufferResults\n");
  //    fflush(stdout);
	if (monte_carlo_iterations <= 1)
	{
	  //		printf("0: no monte carlo\n");
	  //		fflush(stdout);
		WriteResults(experiment, solutes, sve, tag, meniscus, iterations);
		return;
	}
	// accumulate these results
	list <Expdata>::iterator expdata_iter;
	Expdata expdata;
	expdata.tag = tag;
	unsigned int i, e;
	if ((expdata_iter = find(expdata_list.begin(), expdata_list.end(), expdata)) == expdata_list.end())
	{
		// a new one
	  //		printf("0: BufferResults: new tag %s\n", tag.ascii());
	  //		fflush(stdout);
		for (i = 0; i < solutes.size(); i++)
		{
			solutes[i].c /= monte_carlo_iterations * 1e0;
		}
		expdata.solutes = solutes;
		for (e = 0; e < experiment.size(); e++)
		{
			for (i = 0; i < sve[e].solutes.size(); i++)
			{
				sve[e].solutes[i].c /= monte_carlo_iterations * 1e0;
			}
		}
		expdata.sve = sve;
		expdata_list.push_back(expdata);
		// this should never be our last, since we intercept non-monte carlo runs above
		if (this_monte_carlo == monte_carlo_iterations - 1)
		{
			fprintf(stderr, "0: error: unexpected premature end in BufferResults!\n");
		}
		//		printf("0: BufferResults: new tag %s solutes.size %u\n", tag.ascii(), expdata.solutes.size());
		// fflush(stdout);
	}
	else
	{
	  //		printf("0: BufferResults: accum to existing tag %s\n", tag.ascii());
	  //		fflush(stdout);
		for (i = 0; i < solutes.size(); i++)
		{
			solutes[i].c /= monte_carlo_iterations * 1e0;
			(*expdata_iter).solutes.push_back(solutes[i]);
		}
		for (e = 0; e < experiment.size(); e++)
		{
			for (i = 0; i < sve[e].solutes.size(); i++)
			{
				sve[e].solutes[i].c /= monte_carlo_iterations * 1e0;
				(*expdata_iter).sve[e].solutes.push_back(sve[e].solutes[i]);
			}
		}
		// is this our last?
		printf("0: BufferResults: existing tag %s solutes.size %u\n", tag.ascii(), (*expdata_iter).solutes.size());
		fflush(stdout);
		if (this_monte_carlo == monte_carlo_iterations - 1)
		{
			WriteResults(experiment, (*expdata_iter).solutes, (*expdata_iter).sve, tag, meniscus, iterations);
		}
	}
}

void US_fe_nnls_t::BufferResults(vector <struct mfem_data> experiment,
								 Simulation_values sv, QString tag, double meniscus, unsigned int iterations)
{
  //	printf("0: BufferResults\n");
  //	fflush(stdout);
	if (monte_carlo_iterations <= 1)
	{
	  //		printf("0: no monte carlo\n");
	  //		fflush(stdout);
		WriteResults(experiment, sv, tag, meniscus, iterations);
		return;
	}
	// accumulate these results
	list <Expdata>::iterator expdata_iter;
	Expdata expdata;
	expdata.tag = tag;
	if ((expdata_iter = find(expdata_list.begin(), expdata_list.end(), expdata)) == expdata_list.end())
	{
		// a new one
		unsigned int i;
		//		printf("0: BufferResults: new tag %s\n", tag.ascii());
		//		fflush(stdout);
		for (i = 0; i < sv.solutes.size(); i++)
		{
			sv.solutes[i].c /= monte_carlo_iterations * 1e0;
		}
		expdata.sv = sv;
		expdata_list.push_back(expdata);
		// this should never be our last, since we intercept non-monte carlo runs above
		if (this_monte_carlo == monte_carlo_iterations - 1)
		{
			fprintf(stderr, "0: error: unexpected premature end in BufferResults!\n");
		}
		//		printf("0: BufferResults: new tag %s solutes.size %u\n", tag.ascii(), expdata.sv.solutes.size());
		//		fflush(stdout);
	}
	else
	{
	  //		printf("0: BufferResults: accum to existing tag %s\n", tag.ascii());
	  //		fflush(stdout);
		unsigned int i;
		for (i = 0; i < sv.solutes.size(); i++)
		{
			sv.solutes[i].c /= monte_carlo_iterations * 1e0;
			(*expdata_iter).sv.solutes.push_back(sv.solutes[i]);
		}
		// is this our last?
		//		printf("0: BufferResults: existing tag %s solutes.size %u\n", tag.ascii(), (*expdata_iter).sv.solutes.size());
		//		fflush(stdout);
		if (this_monte_carlo == monte_carlo_iterations - 1)
		{
			WriteResults(experiment, (*expdata_iter).sv, tag, meniscus, iterations);
		}
	}
}

int
US_fe_nnls_t::init_run(const QString & data_file,
					   const QString & solute_file,
					   const QString & gridopt)
{
	// return codes:
	// 0: no error
	// -1: couldn't open data file
	// -2: couldn't open solute file
	// -3: some undefined error happened in calc_residuals
	this->gridopt = gridopt;
	this->gridopt = "no";
	maxrss = 0l;

	startDateTime = QDateTime::currentDateTime();
	solutions.clear();
	experiment.clear();
	QFile f(data_file);
	unsigned int count1, count2;
	//	short int short_int_val;
	double short_int_val;
	int int_val;
	float float_val;
	if (!f.open(IO_ReadOnly))
	{
		cout << tr("Could not open data file: ") << data_file <<
		tr(" for input\n");
		cout <<
		tr
		("Please check the path, file name and read permissions...\n\n");
		return (-1);
	}
	else
	{
		struct mfem_data temp_experiment;
		struct mfem_scan temp_scan;
		vector < double >concentration;
		QDataStream ds(&f);
		ds >> email;
		cout << "email: " + email + "\n";
		ds >> analysis_type;
		if (analysis_type == "SA2D")
		{
			analysis_type = "2DSA";
		}
		cout << "at: " + analysis_type + "\n";
		ds >> int_val;
		fit_tinoise = int_val;
		cout << "ti: " << fit_tinoise << "\n";
		ds >> int_val;
		fit_rinoise = int_val;
		//	if (fit_rinoise) {
		//	fit_rinoise = 0;
		//	monte_carlo_iterations = 1000;
		//	}
		cout << "ri: " << fit_rinoise << "\n";
		ds >> int_val;
		union_results = int_val;
		cout << "ur: " << union_results << "\n";
		ds >> meniscus_range;
		cout << "meniscus_range: " << meniscus_range << "\n";
		ds >> int_val;
		fit_meniscus = int_val;
		cout << "fit meniscus: " << fit_meniscus << "\n";
		ds >> meniscus_gridpoints;
		cout << "meniscus gridpoints: " << meniscus_gridpoints << "\n";
		ds >> int_val;
		use_iterative = int_val;
		cout << "Use iterative method: " << use_iterative << "\n";
		ds >> max_iterations;
		cout << "max. iterations: " << max_iterations << "\n";
		ds >> regularization;
		//	if (monte_carlo_iterations > 1 &&
		//	regularization != 0) {
		//	cout << "regularization turned off by monte carlo\n";
		//	regularization = 0;
		//	}
		cout << "regularization: " << regularization << "\n";
		GA_Params.regularization = regularization;
		ds >> count1;
		cout << "count1: " << count1 << "\n";
		//	cout << "sizeof(count1): " << sizeof(count1) << "\n";
		//	cout << "sizeof(unsigned int): " << sizeof(unsigned int) << "\n";
		fflush(stdout);
		if (analysis_type == "2DSA")
		{
			ds >> SA2D_Params.monte_carlo;
			if (SA2D_Params.monte_carlo < 1)
			{
				SA2D_Params.monte_carlo = 1;
			}
			cout << "Monte carlo: " << SA2D_Params.monte_carlo << endl;
			monte_carlo_iterations = SA2D_Params.monte_carlo;
			ds >> SA2D_Params.ff0_min;
			ds >> SA2D_Params.ff0_max;
			ds >> SA2D_Params.ff0_resolution;
			ds >> SA2D_Params.s_min;
			ds >> SA2D_Params.s_max;
			ds >> SA2D_Params.s_resolution;
			ds >> SA2D_Params.uniform_grid_repetition;
			if (!myrank)
			{
				cout << "SA2D_Params.ff0_min: " << SA2D_Params.ff0_min << endl;
				cout << "SA2D_Params.ff0_max: " << SA2D_Params.ff0_max << endl;
				cout << "SA2D_Params.ff0_resolution: " << SA2D_Params.ff0_resolution << endl;
				cout << "SA2D_Params.s_min: " << SA2D_Params.s_min << endl;
				cout << "SA2D_Params.s_max: " << SA2D_Params.s_max << endl;
				cout << "SA2D_Params.s_resolution: " << SA2D_Params.s_resolution << endl;
				cout << "SA2D_Params.uniform_grid_repetition: " << SA2D_Params.uniform_grid_repetition << endl;
				fflush(stdout);
			}
		}
		if (analysis_type == "2DSA_MW")
		{
			ds >> SA2D_Params.monte_carlo;
			if (SA2D_Params.monte_carlo < 1)
			{
				SA2D_Params.monte_carlo = 1;
			}
			cout << "Monte carlo: " << SA2D_Params.monte_carlo << endl;
			monte_carlo_iterations = SA2D_Params.monte_carlo;
			ds >> SA2D_Params.ff0_min;
			ds >> SA2D_Params.ff0_max;
			ds >> SA2D_Params.ff0_resolution;
			ds >> SA2D_Params.mw_min;
			ds >> SA2D_Params.mw_max;
			ds >> SA2D_Params.grid_resolution;
			ds >> SA2D_Params.uniform_grid_repetition;
			ds >> SA2D_Params.max_mer;
			if (!myrank)
			{
				cout << "2DSA_Params.ff0_min: " << SA2D_Params.ff0_min << endl;
				cout << "2DSA_Params.ff0_max: " << SA2D_Params.ff0_max << endl;
				cout << "2DSA_Params.ff0_resolution: " << SA2D_Params.ff0_resolution << endl;
				cout << "2DSA_Params.mw_min: " << SA2D_Params.mw_min << endl;
				cout << "2DSA_Params.mw_max: " << SA2D_Params.mw_max << endl;
				cout << "2DSA_Params.grid_resolution: " << SA2D_Params.grid_resolution << endl;
				cout << "2DSA_Params.uniform_grid_repetition: " << SA2D_Params.uniform_grid_repetition << endl;
				cout << "2DSA_Params.max_mer: " << SA2D_Params.max_mer << endl;
				fflush(stdout);
			}
		}
		if (analysis_type == "GA")
		{
			GA_Params.analysis_type = analysis_type;
			ds >> GA_Params.monte_carlo;
			if (GA_Params.monte_carlo < 1)
			{
				GA_Params.monte_carlo = 1;
			}
			cout << "Monte carlo: " << GA_Params.monte_carlo << endl;
			monte_carlo_iterations = GA_Params.monte_carlo;
			ds >> GA_Params.demes;
			ds >> GA_Params.generations;
			//	GA_Params.generations = 2;
			ds >> GA_Params.crossover;
			ds >> GA_Params.mutation;
			ds >> GA_Params.plague;
			ds >> GA_Params.elitism;
			ds >> GA_Params.migration_rate;
			ds >> GA_Params.genes;
			//	GA_Params.genes = 10;
			ds >> GA_Params.initial_solutes;
			ds >> GA_Params.random_seed;
			if (!myrank)
			{
				cout << "reading GA demes:" << GA_Params.demes << endl;
				cout << "generations:" << GA_Params.generations << endl;
				cout << "crossover:" << GA_Params.crossover << endl;
				cout << "mutation:" << GA_Params.mutation << endl;
				cout << "plague:" << GA_Params.plague << endl;
				cout << "elitism:" << GA_Params.elitism << endl;
				cout << "migration_rate:" << GA_Params.migration_rate << endl;
				cout << "genes:" << GA_Params.genes << endl;
				cout << "solutes:" << GA_Params.initial_solutes << endl;
				cout << "seed:" << GA_Params.random_seed << endl;
				cout << "regularization:" << GA_Params.regularization << endl;
				cout << "sizeof(unsigned long):" << sizeof(unsigned long) << endl;
			}
			struct bucket temp_bucket;
			unsigned int i;
			for (i = 0; i < GA_Params.initial_solutes; i++)
			{
				ds >> temp_bucket.s;
				ds >> temp_bucket.s_min;
				ds >> temp_bucket.s_max;
				ds >> temp_bucket.ff0;
				ds >> temp_bucket.ff0_min;
				ds >> temp_bucket.ff0_max;
				if (temp_bucket.s_min >= .999e-14)
				{
					if (!myrank)
					{
						cout << "reading solute:" << i << endl;
						cout << "solute s:" << temp_bucket.s << endl;
						cout << "solute smin:" << temp_bucket.s_min << endl;
						cout << "solute smax:" << temp_bucket.s_max << endl;
						cout << "solute ff0:" << temp_bucket.ff0 << endl;
						cout << "solute ff0min:" << temp_bucket.ff0_min << endl;
						cout << "solute ff0max:" << temp_bucket.ff0_max << endl;
					}
					GA_Params.solute.push_back(temp_bucket);
				}
				else
				{
					if (!myrank)
					{
						cout << "skipping solute:" << i << endl;
						cout << "solute s:" << temp_bucket.s << endl;
						cout << "solute smin:" << temp_bucket.s_min << endl;
						cout << "solute smax:" << temp_bucket.s_max << endl;
						cout << "solute ff0:" << temp_bucket.ff0 << endl;
						cout << "solute ff0min:" << temp_bucket.ff0_min << endl;
						cout << "solute ff0max:" << temp_bucket.ff0_max << endl;
					}
				}
			}
			GA_Params.initial_solutes = GA_Params.solute.size();
			if (!myrank)
			{
				for (i = 0; i < GA_Params.solute.size(); i++)
				{
					cout << myrank << ":checking solute:" << i << endl;
					cout << myrank << ":solute s:" << GA_Params.solute[i].s << endl;
					cout << myrank << ":solute smin:" << GA_Params.solute[i].s_min << endl;
					cout << myrank << ":solute smax:" << GA_Params.solute[i].s_max << endl;
					cout << myrank << ":solute ff0:" << GA_Params.solute[i].ff0 << endl;
					cout << myrank << ":solute ff0min:" << GA_Params.solute[i].ff0_min << endl;
					cout << myrank << ":solute ff0max:" << GA_Params.solute[i].ff0_max << endl;
				}
				fflush(stdout);
			}
		}
		if (analysis_type == "GA_MW")
		{
			GA_Params.analysis_type = analysis_type;
			ds >> GA_Params.monte_carlo;
			if (GA_Params.monte_carlo < 1)
			{
				GA_Params.monte_carlo = 1;
			}
			cout << "Monte carlo: " << GA_Params.monte_carlo << endl;
			monte_carlo_iterations = GA_Params.monte_carlo;
			ds >> GA_Params.demes;
			ds >> GA_Params.generations;
			//	GA_Params.generations = 2;
			ds >> GA_Params.crossover;
			ds >> GA_Params.mutation;
			ds >> GA_Params.plague;
			ds >> GA_Params.elitism;
			ds >> GA_Params.migration_rate;
			ds >> GA_Params.genes;
			//	GA_Params.genes = 10;
			ds >> GA_Params.largest_oligomer;
			ds >> GA_Params.largest_oligomer_string;
			if (!myrank)
			{
				cout << "GA Params largest oligomer string " << GA_Params.largest_oligomer_string << endl;
				cout << "GA Params largest oligomer " << GA_Params.largest_oligomer << endl;
			}
			fflush(stdout);
			GA_Params.initial_solutes = 0;
			for (unsigned int i = 0; i < GA_Params.largest_oligomer; i++)
			{
				if ((GA_Params.largest_oligomer_string.ascii())[i] == '1')
				{
					GA_Params.initial_solutes++;
				}
			}
			if (!myrank)
			{
				cout << "GA Params initial solutes: " << GA_Params.initial_solutes << endl;
			}
			fflush(stdout);
			ds >> GA_Params.random_seed;
			ds >> GA_Params.mw_min;
			ds >> GA_Params.mw_max;
			ds >> GA_Params.ff0_min;
			ds >> GA_Params.ff0_max;
			if (!myrank)
			{
				cout << "reading GA demes:" << GA_Params.demes << endl;
				cout << "generations:" << GA_Params.generations << endl;
				cout << "crossover:" << GA_Params.crossover << endl;
				cout << "mutation:" << GA_Params.mutation << endl;
				cout << "plague:" << GA_Params.plague << endl;
				cout << "elitism:" << GA_Params.elitism << endl;
				cout << "migration_rate:" << GA_Params.migration_rate << endl;
				cout << "genes:" << GA_Params.genes << endl;
				cout << "largest_oligomer:" << GA_Params.initial_solutes << endl;
				cout << "seed:" << GA_Params.random_seed << endl;
				cout << "regularization:" << GA_Params.regularization << endl;
				cout << "sizeof(unsigned long):" << sizeof(unsigned long) << endl;
				cout << "mw_min:" << GA_Params.mw_min << endl;
				cout << "mw_max:" << GA_Params.mw_max << endl;
				cout << "ff0_min:" << GA_Params.ff0_min << endl;
				cout << "ff0_max:" << GA_Params.ff0_max << endl;
			}
			struct bucket temp_bucket;
			unsigned int i;
			for (i = 0; i < GA_Params.largest_oligomer; i++)
			{
				if ((GA_Params.largest_oligomer_string.ascii())[i] == '1')
				{
					temp_bucket.s = (i + 1e0) * (GA_Params.mw_min + GA_Params.mw_max)/2e0;
					temp_bucket.s_min = (i + 1e0) * GA_Params.mw_min;
					temp_bucket.s_max = (i + 1e0) * GA_Params.mw_max;
					temp_bucket.ff0 = (GA_Params.ff0_min + GA_Params.ff0_max)/2e0;
					temp_bucket.ff0_min = GA_Params.ff0_min;
					temp_bucket.ff0_max = GA_Params.ff0_max;

					if (!myrank)
					{
						cout << "0:computing solute:" << i << endl;
						cout << "0:solute mw:" << temp_bucket.s << endl;
						cout << "0:solute mwmin:" << temp_bucket.s_min << endl;
						cout << "0:solute mwmax:" << temp_bucket.s_max << endl;
						cout << "0:solute ff0:" << temp_bucket.ff0 << endl;
						cout << "0:solute ff0min:" << temp_bucket.ff0_min << endl;
						cout << "0:solute ff0max:" << temp_bucket.ff0_max << endl;
					}
					GA_Params.solute.push_back(temp_bucket);
				}
			}
			GA_Params.initial_solutes = GA_Params.solute.size();
			if (!myrank)
			{
				for (i = 0; i < GA_Params.solute.size(); i++)
				{
					cout << myrank << ":checking solute:" << i << endl;
					cout << myrank << ":solute mw:" << GA_Params.solute[i].s << endl;
					cout << myrank << ":solute mwmin:" << GA_Params.solute[i].s_min << endl;
					cout << myrank << ":solute mwmax:" << GA_Params.solute[i].s_max << endl;
					cout << myrank << ":solute ff0:" << GA_Params.solute[i].ff0 << endl;
					cout << myrank << ":solute ff0min:" << GA_Params.solute[i].ff0_min << endl;
					cout << myrank << ":solute ff0max:" << GA_Params.solute[i].ff0_max << endl;
				}
				fflush(stdout);
			}
		}

		for (unsigned int i = 0; i < count1; i++)
		{
			ds >> temp_experiment.id;
			ds >> temp_experiment.cell;
			ds >> temp_experiment.channel;
			ds >> temp_experiment.wavelength;
			ds >> temp_experiment.meniscus;
			ds >> temp_experiment.bottom;
			ds >> temp_experiment.rpm;
			ds >> temp_experiment.s20w_correction;
			ds >> temp_experiment.D20w_correction;
			ds >> count2;	// number of radius points
			temp_experiment.radius.clear();
			for (unsigned int j = 0; j < count2; j++)
			{
				ds >> short_int_val;
				//		temp_experiment.radius.push_back((double) (short_int_val / 10000.0 + 5.0));
				temp_experiment.radius.push_back(short_int_val);
			}
			ds >> count2;	// number of scans
			temp_experiment.scan.clear();
			for (unsigned int j = 0; j < count2; j++)
			{
				ds >> temp_scan.time;
				temp_scan.conc.clear();
				for (unsigned int k = 0; k < temp_experiment.radius.size();
						k++)
				{
					ds >> short_int_val;
					//			temp_scan.conc.push_back((double) short_int_val / 10000.0);
					temp_scan.conc.push_back(short_int_val);
				}
				temp_experiment.scan.push_back(temp_scan);
			}
			ds >> temp_experiment.viscosity;
			ds >> temp_experiment.density;
			ds >> temp_experiment.vbar;
			ds >> temp_experiment.vbar20;
			ds >> temp_experiment.avg_temperature;
#if defined(DEBUG_HYDRO)
			
			printf("experiment time %g avg_temp %.12g vbar %.12g vbar20 %.12g visc %.12g density %.12g\n",
				   temp_experiment.scan[temp_experiment.scan.size()-1].time,
				   temp_experiment.avg_temperature,
				   temp_experiment.vbar,
				   temp_experiment.vbar20,
				   temp_experiment.viscosity,
				   temp_experiment.density);

#endif
			experiment.push_back(temp_experiment);
		}
	}
	f.close();
	f.setName(solute_file);
	if (!f.open(IO_ReadOnly))
	{
		cout << tr("Could not open solute file: ") << solute_file <<
		tr("for input\n");
		cout <<
		tr
		("Please check the path, file name and read permissions...\n\n");
		return (-2);
	}
	else
	{
		if (myrank)
		{
			f.close();
		}
		else
		{
			Solute temp_solute;
			gene temp_gene;
			QDataStream ds(&f);
			solutions.clear();
			ds >> count1;
			for (unsigned int i = 0; i < count1; i++)
			{
				ds >> count2;
				//		printf("solute %d size %d\n", i, count2);
				temp_gene.component.clear();
				for (unsigned int j = 0; j < count2; j++)
				{
					ds >> temp_solute.s;
					ds >> temp_solute.k;
					ds >> temp_solute.c;
					temp_gene.component.push_back(temp_solute);
				}
				solutions.push_back(temp_gene);
			}

			ds >> count2;
			Control_Params.float_params.clear();
			for (unsigned int i = 0; i < count2; i++)
			{
				ds >> float_val;
				Control_Params.float_params.push_back(float_val);
			}
			ds >> count2;
			Control_Params.int_params.clear();
			for (unsigned int i = 0; i < count2; i++)
			{
				ds >> int_val;
				Control_Params.int_params.push_back(int_val);
			}
		}
	}
	return (0);
}

/*
 *
 * this function will run all MPI jobs. After running, it will return the
 * variance and the solute vector with updated c values (s, f/f0 and c for
 * each vector element). If requested, it will also return the ti and ri
 * vectors, the residuals vector,
 *
 */

typedef struct _MPI_Work_Msg
{
	unsigned int this_solutions;
	unsigned int this_length;
	int command; // 1 process solutes, 2 wait for wakeup, 3 shutdown, 0 idle (after data received), 4 wakeup sent
	unsigned int depth; // nonzero if union
	double meniscus_offset;
}
MPI_Work_Msg;

typedef struct _Jobqueue
{
	MPI_Work_Msg msg;
	vector<Solute> solutes;
}
Jobqueue;

int US_fe_nnls_t::run(int status)
{
#if defined(GLOBAL_JOB_TIMING)
	gettimeofday(&start_tv, NULL);
#endif

	int iterative = use_iterative;
	vector <struct gene> sav_solutions;
	Simulation_values sv;
	// unsigned int i = 0, j = 0;
	// for each experiment
	unsigned int use_size = experiment.size();
	if (!use_size)
	{
		use_size = 1;
	}

	if (status)
	{
		MPI_Finalize();
		exit(0);
	}

	if (analysis_type == "GA" ||
			analysis_type == "GA_MW")
	{
		// temporary overrides
		//	GA_Params.generations = 2;
		//	GA_Params.genes = 10;
		for (this_monte_carlo = 0; this_monte_carlo < monte_carlo_iterations; this_monte_carlo++)
		{
			if (this_monte_carlo)
			{
				// broadcast monte carlo data to the workers
				printf("%d: monte carlo iteration %u\n", myrank, this_monte_carlo);
				if (!myrank)
				{
					printf("0: get_monte_carlo (generate monte carlo data)\n");
					fflush(stdout);
					experiment = get_monte_carlo(org_experiment, save_gaussians);
				}
				// broadcast experiment data
				if (this_monte_carlo)
				{
#if defined(US_DEBUG_MPI)
					printf("%d: MPI pre monte carlo barrier enter\n", myrank);
					fflush(stdout);
#endif

					MPI_Barrier(MPI_COMM_WORLD);
#if defined(US_DEBUG_MPI)

					printf("%d: MPI pre monte carlo barrier exit\n", myrank);
					fflush(stdout);
#endif

				}
				//				printf("%d: broadcast monte carlo data\n", myrank);
				//				fflush(stdout);
				{
					unsigned int e, k;
					for (e = 0; e < experiment.size(); e++)
					{
						for (k = 0; k < experiment[e].scan.size(); k++)
						{
							MPI_Bcast(&experiment[e].scan[k].conc[0], experiment[e].radius.size(), MPI_DOUBLE, 0, MPI_COMM_WORLD);
							//		printf("%d: broadcast exp %u scan %u\n", myrank, e, k); fflush(stdout);
						}
					}
				}
				//				printf("%d: broadcast monte carlo data complete\n", myrank);
				//				fflush(stdout);
			}

			if (this_monte_carlo)
			{
#if defined(US_DEBUG_MPI)
				printf("%d: MPI post monte carlo barrier enter\n", myrank);
				fflush(stdout);
#endif

				MPI_Barrier(MPI_COMM_WORLD);
#if defined(US_DEBUG_MPI)

				printf("%d: MPI post monte carlo barrier exit\n", myrank);
				fflush(stdout);
#endif

			}

			if (myrank)
			{
			  //				printf("%d: ga_setup called\n", myrank);
			  //				fflush(stdout);
				ga_setup(GA_Params, myrank, this);
			}
			// the master will keep a list of all demes and their current generations,
			// which will report once a generation along with the migration list
			// the master will keep a migration list for each deme, as well as make
			// results available to each deme.
			// when the last deme has reached the requested generations, the master
			// will report that it is time to finish up
			// when each deme is finished, it will message the last best & terminate
			if (!myrank)
			{ // master
				int i, from1, from2;
				int generation[npes];
				char *migration_data[npes];
				int migration_size[npes];
				int infrom[npes][2]; // these give the 2 indices of processes to migrate in from (using ring)
				for (i = 1; i < npes; i++)
				{
					generation[i] = 0;
					migration_data[i] = (char *)0;
					migration_size[i] = 0;
					from1 = i - 1;
					if (from1 < 1)
					{
						from1 = npes - 1;
					}
					from2 = i + 1;
					if (from2 >= npes)
					{
						from2 = 1;
					}
					infrom[i][0] = from1;
					infrom[i][1] = from2; // simple ring topo
				}
				int procsleft = npes - 1;
				int last_achieved_gen = 0;

				MPI_GA_Work_Msg mpi_ga_msg_in, mpi_ga_msg_out;
				vector<Solute> demes_results[npes];
				double demes_fitness[npes];
				Solute tmp_solutes[GA_Params.initial_solutes];
				double best_fitness = 9e99;
				int fitness_same_count = 0;
				int max_gen = 0;
				int early_termination = 0;

				int return_stat = 0;
				while(procsleft)
				{
				  //					printf("0: master waiting for comm processes left %d\n", procsleft);
					fflush(stdout);
					for (i = 1; i < npes; i++)
					{
					  printf("0: master has %d at generation %d of %d\n", i, generation[i], GA_Params.generations);
					  fflush(stdout);
					}

					// master receives generation completion message from a worker

#if defined(US_DEBUG_MPI)
					printf("0: MPI_Recv generation comp msg %d MPI_CHAR from any\n", sizeof(mpi_ga_msg_in));
					fflush(stdout);
					//	printf("0: sleeping 5\n"); fflush(stdout);
					//	sleep(5);
					//	printf("0: done sleeping 5\n"); fflush(stdout);
#endif

					return_stat = MPI_Recv(&mpi_ga_msg_in,
										   sizeof(mpi_ga_msg_in),
										   MPI_CHAR,
										   MPI_ANY_SOURCE,
										   0,
										   MPI_COMM_WORLD,
										   &mpi_status);
#if defined(US_DEBUG_MPI)

					printf("0: MPI returned %d from %d\n", return_stat, mpi_status.MPI_SOURCE);
					fflush(stdout);
					printf("0: received %d from %d generation %d length %d\n", mpi_status.MPI_TAG, mpi_status.MPI_SOURCE,
					       mpi_ga_msg_in.gen, mpi_ga_msg_in.size);
					fflush(stdout);
#endif

					// update generation table

					generation[mpi_status.MPI_SOURCE] = mpi_ga_msg_in.gen;

					if (max_gen < mpi_ga_msg_in.gen)
					{
						max_gen = mpi_ga_msg_in.gen;
					}

					if (!early_termination)
					{
						if (best_fitness > roundn(mpi_ga_msg_in.fitness, 1, 9))
						{
							best_fitness = roundn(mpi_ga_msg_in.fitness, 1, 9);
							fitness_same_count = 0;
						}
						else
						{
							fitness_same_count++;
						}
						if (max_gen > 10 && fitness_same_count > 5 * (npes - 1) )
						{
						  // printf("0: fitness hasn't improved in the last %d deme results. early termination\n", fitness_same_count);
						  // fflush(stdout);
							early_termination = 1;
						}
						// printf("0: early term test max_gen %d same fitness count %d best %.16g last %.16g\n", max_gen, fitness_same_count, best_fitness, mpi_ga_msg_in.fitness);
						// fflush(stdout);
					}

					if (mpi_ga_msg_in.gen == -1)
					{
						// final results to be received
#if defined(US_DEBUG_MPI)
						printf("0: MPI_Recv results %d %d MPI_CHAR from %d\n", 1000 + mpi_status.MPI_SOURCE, mpi_ga_msg_in.size * sizeof(Solute), mpi_status.MPI_SOURCE);
						fflush(stdout);
						fflush(stdout);
#endif
						//	printf("0: master received out final solutes preamble size %u\n", mpi_ga_msg_in.size); fflush(stdout);
						demes_fitness[mpi_status.MPI_SOURCE] = mpi_ga_msg_in.fitness;
						return_stat = MPI_Recv(tmp_solutes,
											   mpi_ga_msg_in.size * sizeof(Solute),
											   MPI_CHAR,
											   mpi_status.MPI_SOURCE,
											   1000 + mpi_status.MPI_SOURCE,
											   MPI_COMM_WORLD,
											   &mpi_status2);
#if defined(US_DEBUG_MPI)

						printf("0: MPI returned %d from %d\n", return_stat, mpi_status.MPI_SOURCE);
						fflush(stdout);
						printf("0: master received final solutes size %u from %d\n", mpi_ga_msg_in.size * sizeof(Solute), mpi_status.MPI_SOURCE);
						fflush(stdout);
#endif

						demes_results[mpi_status.MPI_SOURCE].clear();
						for (unsigned int i = 0; i < mpi_ga_msg_in.size; i++)
						{
							demes_results[mpi_status.MPI_SOURCE].push_back(tmp_solutes[i]);
						}
						//	printf("0: listing best deme just received\n"); fflush(stdout);
						//	for (unsigned int i = 0; i < demes_results[mpi_status.MPI_SOURCE].size(); i++) {
						//		printf("0: deme %d sol %d %.4g %.4g\n",
						//		mpi_status.MPI_SOURCE, i, demes_results[mpi_status.MPI_SOURCE][i].s, demes_results[mpi_status.MPI_SOURCE][i].k);
						//		fflush(stdout);
						//	}
						procsleft--;
						// skip for now
						printf("0: process %d finishing\n", mpi_status.MPI_SOURCE);
						continue;
					}

					// all achieve last generation ?

					if (!last_achieved_gen)
					{
						last_achieved_gen = 1;
						for (i = 1; i < npes; i++)
						{
							if (generation[i] < (int)GA_Params.generations &&
									generation[i] != -1)
							{
								last_achieved_gen = 0;
								break;
							}
						}
						if (last_achieved_gen)
						{
							printf("0: last generation achieved by all demes\n");
							fflush(stdout);
						}
					}

					// take in migration data (always)

					if (migration_data[mpi_status.MPI_SOURCE])
					{
						free(migration_data[mpi_status.MPI_SOURCE]);
						migration_data[mpi_status.MPI_SOURCE] = 0;
					}

					migration_size[mpi_status.MPI_SOURCE] = mpi_ga_msg_in.size;
					if (mpi_ga_msg_in.size)
					{

						// only receive migration data if not zero size

						if ((migration_data[mpi_status.MPI_SOURCE] = (char *)malloc(mpi_ga_msg_in.size)) == NULL)
						{
							fputs("0: GA migration data malloc failure\r\n", stderr);
							MPI_Finalize();
							exit(-1);
						}
						//	printf("0: alloced migration data\n"); fflush(stdout);
#if defined(US_DEBUG_MPI)
						printf("0: MPI_Recv emigrants %d %d MPI_CHAR from %d\n", 1001 + mpi_status.MPI_SOURCE, mpi_ga_msg_in.size, mpi_status.MPI_SOURCE);
						fflush(stdout);
#endif

						return_stat = MPI_Recv(migration_data[mpi_status.MPI_SOURCE],
											   mpi_ga_msg_in.size,
											   MPI_CHAR,
											   mpi_status.MPI_SOURCE,
											   1001 + mpi_status.MPI_SOURCE,
											   MPI_COMM_WORLD,
											   &mpi_status2);
#if defined(US_DEBUG_MPI)

						printf("0: MPI returned %d from %d\n", return_stat, mpi_status.MPI_SOURCE);
						fflush(stdout);
						printf("0: MPI returned\n");
						fflush(stdout);
#endif
						//
						// printf("0: migration data received from %d size %d\n", mpi_status.MPI_SOURCE, mpi_ga_msg_in.size);
						// fflush(stdout);
					}
					else
					{
					  // printf("0: skipping migration in from %d, no data\n", mpi_status.MPI_SOURCE);
					  // fflush(stdout);
					}

					// let worker know about last achieved generation

					mpi_ga_msg_out.gen = last_achieved_gen;
					// printf("0: last acheived gen %d\n", mpi_ga_msg_out.gen);
					if (generation[mpi_status.MPI_SOURCE] > 1.3 * GA_Params.generations)
					{
					  // printf("0: early termination requested for %d - 30%% over max generations\n",
					  //	   mpi_status.MPI_SOURCE);
					  // fflush(stdout);
						mpi_ga_msg_out.gen = 1;
					}
					if (early_termination)
					{
#if defined(US_DEBUG_MPI)
						printf("0: early termination on\n");
#endif

						last_achieved_gen = mpi_ga_msg_out.gen = 1;
					}
					mpi_ga_msg_out.size = migration_size[infrom[mpi_status.MPI_SOURCE][0]];
					// printf("0: sending migrate 1 last_achieved %d=%d size %d\n", last_achieved_gen, mpi_ga_msg_out.gen, mpi_ga_msg_out.size);
#if defined(US_DEBUG_MPI)

					printf("0: MPI_Send immigrants msg 1 %d MPI_CHAR to %d\n", sizeof(mpi_ga_msg_out), mpi_status.MPI_SOURCE);
					fflush(stdout);
					//	printf("0: sleeping 5\n"); fflush(stdout);
					//	sleep(5);
					//	printf("0: done sleeping 5\n"); fflush(stdout);
#endif

					return_stat = MPI_Send(&mpi_ga_msg_out,
										   sizeof(mpi_ga_msg_out),
										   MPI_CHAR,
										   mpi_status.MPI_SOURCE,
										   1,
										   MPI_COMM_WORLD);
#if defined(US_DEBUG_MPI)

					printf("0: MPI returned %d from send to %d\n", return_stat, mpi_status.MPI_SOURCE);
					fflush(stdout);
					printf("0: MPI returned\n");
					fflush(stdout);
#endif

					if (last_achieved_gen || mpi_ga_msg_out.gen)
					{
						// no migrate out to worker if last generation
#if defined(US_DEBUG_MPI)
						printf("0: skipping migration sends, last gen\n");
						fflush(stdout);
#endif
						// worker should start building simulation values for final results
					}
					else
					{
						if (mpi_ga_msg_out.size > 0)
						{
#if defined(US_DEBUG_MPI)
							printf("0: sending infrom[%d] to %d\n", infrom[mpi_status.MPI_SOURCE][0], mpi_status.MPI_SOURCE);
							fflush(stdout);
							printf("0: MPI_Send immigrants buf 2 %d MPI_CHAR to %d\n", mpi_ga_msg_out.size, mpi_status.MPI_SOURCE);
							fflush(stdout);
#endif

							return_stat = MPI_Send(migration_data[infrom[mpi_status.MPI_SOURCE][0]],
												   mpi_ga_msg_out.size,
												   MPI_CHAR,
												   mpi_status.MPI_SOURCE,
												   2,
												   MPI_COMM_WORLD);
						}
						mpi_ga_msg_out.size = migration_size[infrom[mpi_status.MPI_SOURCE][1]];
#if defined(US_DEBUG_MPI)

						printf("0: MPI returned %d from send to %d\n", return_stat, mpi_status.MPI_SOURCE);
						fflush(stdout);
						printf("0: MPI_Send immigrants msg 3 %d MPI_CHAR to %d\n", sizeof(mpi_ga_msg_out), mpi_status.MPI_SOURCE);
						fflush(stdout);
						//	printf("0: sleeping 5\n"); fflush(stdout);
						//	sleep(5);
						//	printf("0: done sleeping 5\n"); fflush(stdout);
#endif

						// printf("0: sending migrate 2 last_achieved %d=%d size %d\n", last_achieved_gen, mpi_ga_msg_out.gen, sizeof(mpi_ga_msg_out));
						return_stat = MPI_Send(&mpi_ga_msg_out,
											   sizeof(mpi_ga_msg_out),
											   MPI_CHAR,
											   mpi_status.MPI_SOURCE,
											   3,
											   MPI_COMM_WORLD);
#if defined(US_DEBUG_MPI)

						printf("0: MPI returned %d\n", return_stat);
						fflush(stdout);
#endif

						if (mpi_ga_msg_out.size > 0)
						{
#if defined(US_DEBUG_MPI)
							printf("0: sending infrom[%d] to %d\n", infrom[mpi_status.MPI_SOURCE][1], mpi_status.MPI_SOURCE);
							fflush(stdout);
							printf("0: MPI_Send immigrants buf 4 %d MPI_CHAR to %d\n", mpi_ga_msg_out.size, mpi_status.MPI_SOURCE);
							fflush(stdout);
#endif

							return_stat = MPI_Send(migration_data[infrom[mpi_status.MPI_SOURCE][1]],
												   mpi_ga_msg_out.size,
												   MPI_CHAR,
												   mpi_status.MPI_SOURCE,
												   4,
												   MPI_COMM_WORLD);
#if defined(US_DEBUG_MPI)

							printf("0: MPI returned %d from send to %d\n", return_stat, mpi_status.MPI_SOURCE);
							fflush(stdout);
#endif

						}
					}
				}
				// write out results
				// printf("0: find best deme\n");
				// fflush(stdout);

				int best_deme = 0;
				double best_deme_fitness = 1e99;
				for (int i = 1; i < npes; i++)
				{
					if (demes_fitness[i] < best_deme_fitness)
					{
						best_deme = i;
						best_deme_fitness = demes_fitness[i];
					}
				}
				endDateTime = QDateTime::currentDateTime();

				for (unsigned int i = 0; i < demes_results[best_deme].size(); i++)
				{
				  // printf("0: deme %d sol %d %.4g %.4g\n",
				  //	   best_deme, i, demes_results[best_deme][i].s, demes_results[best_deme][i].k);
				  // fflush(stdout);
				}
				QFile f("email_text_" + startDateTime.toString("yyMMddhhmmss"));
				if (this_monte_carlo == 0)
				{
					if (f.open(IO_WriteOnly))
					{
						QTextStream ts(&f);
						ts << email + "\n";
						ts << "Your " + analysis_type + " grid job has finished\n\n";
						//	if (regularization > 0e0) {
						//	ts << "Regularization factor " << regularization << "\n\n";
						//	}

						ts << "GA control parameters were defined as follows:\n\n";

						if (monte_carlo_iterations > 1)
						{
							ts << "Monte Carlo iterations:   " << monte_carlo_iterations << endl;
						}

						if (analysis_type == "GA_MW")
						{
							ts << "MW minimum:                  " << GA_Params.mw_min << endl;
							ts << "MW maximum:                  " << GA_Params.mw_max << endl;
							ts << "f/f0 minimum:                " << GA_Params.ff0_min << endl;
							ts << "f/f0 maximum:                " << GA_Params.ff0_max << endl;
							ts << "Largest oligomer             " << GA_Params.largest_oligomer << endl;
							ts << "Oligomer selection bitmap    " << GA_Params.largest_oligomer_string << endl;

						}
						ts << "Population (Genes per deme): " << GA_Params.genes << endl;
						ts << "Demes:                       " << npes - 1 << endl;
						ts << "Generations:                 " << GA_Params.generations << endl;
						ts << "Crossover %:                 " << GA_Params.crossover << endl;
						ts << "Mutation %:                  " << GA_Params.mutation << endl;
						if (GA_Params.plague > 0)
						{
							ts << "Plague %:                    " << GA_Params.plague << endl;
						}
						if (GA_Params.elitism > 0)
						{
							ts << "Elitism:                     " << GA_Params.elitism << endl;
						}
						if (GA_Params.migration_rate> 0)
						{
							ts << "Migration %:                 " << GA_Params.migration_rate << endl;
						}
						if (analysis_type == "GA")
						{
							ts << "Initial # of solutes:        " << GA_Params.initial_solutes << endl;
						}
						ts << "Random seed:                 " << GA_Params.random_seed << endl;
						if (GA_Params.regularization > 0)
						{
							ts << "Regularization factor:       " << GA_Params.regularization << endl;
						}
						if (fit_tinoise)
						{
							ts << "Fit TI noise:                on\n";
						}
						if (fit_rinoise)
						{
							ts << "Fit RI noise:                on\n";
						}
						if (fit_meniscus)
						{
							ts << "Fit meniscus:                on\n";
							ts << "Meniscus range:              " << meniscus_range << "\n";
							ts << "Meniscus gridpoints:         " << meniscus_gridpoints << "\n";
						}
						ts << endl;
						ts << "The results of your " + analysis_type + " analysis involved datasets:\n\n";
					}
					f.close();
				}
				unsigned int e;
				Simulation_values sv;
				vector<Simulation_values> sve;
				vector <Solute> final_use_solutes = demes_results[best_deme];
				if (analysis_type == "GA_MW")
				{
					for (unsigned int i = 0; i < final_use_solutes.size(); i++)
					{
						final_use_solutes[i].s =
							pow(pow((final_use_solutes[i].s * experiment[0].vbar20)/(AVOGADRO * M_PI), 2e0)/6e0,1e0/3e0) *
							(1.0 - DENS_20W * experiment[0].vbar20) /
							(3.0 * final_use_solutes[i].k * experiment[0].vbar20 * VISC_20W);
					}
				}
				for (e = 0; e < experiment.size(); e++)
				{
					sve.push_back(sv);
					vector <struct mfem_data> use_experiment;
					use_experiment.push_back(experiment[e]);
					for (unsigned int l=0; l < final_use_solutes.size(); l++)
					{
					  // printf("0: final_use_solute 1 e%u %u s %g ff0 %g\n", e, l, final_use_solutes[l].s, final_use_solutes[l].k);
					  //	fflush(stdout);
					}
					sve[e] = calc_residuals(use_experiment, final_use_solutes, 0e0, 1);
				} // for e

				//	Simulation_values sv = calc_residuals(experiment, demes_results[best_deme], 0e0, 0);
				printf("0: best deme is %d size %u fitness %.4g\n", best_deme,
					   demes_results[best_deme].size(), best_deme_fitness);
				fflush(stdout);
				{
					unsigned int i;
					for (i = 0; i < demes_results[best_deme].size(); i++)
					{
						demes_results[best_deme][i].c = 0;
					}

					double this_sum;
					for (e = 0; e < experiment.size(); e++)
					{
						this_sum = 0e0;
						for (i = 0; i < sve[e].solutes.size(); i++)
						{
							this_sum += sve[e].solutes[i].c;
						}
						if (this_sum == 0e0)
						{
							this_sum = 1e0;
						}
						for (i = 0; i < sve[e].solutes.size(); i++)
						{
							demes_results[best_deme][i].c += sve[e].solutes[i].c / this_sum;
						}
					}
					this_sum = 0e0;
					for (i = 0; i < demes_results[best_deme].size(); i++)
					{
						this_sum += demes_results[best_deme][i].c;
					}
					if (this_sum == 0e0)
					{
						this_sum = 1e0;
					}
					for (i = 0; i < demes_results[best_deme].size(); i++)
					{
						demes_results[best_deme][i].c /= this_sum;
					}
				}
				final_use_solutes = demes_results[best_deme];
				if (analysis_type == "GA_MW")
				{
					for (unsigned int i = 0; i < final_use_solutes.size(); i++)
					{
						final_use_solutes[i].s =
							pow(pow((final_use_solutes[i].s * experiment[0].vbar20)/(AVOGADRO * M_PI), 2e0)/6e0,1e0/3e0) *
							(1 - DENS_20W * experiment[0].vbar20) /
							(3 * final_use_solutes[i].k * experiment[0].vbar20 * (100.0 * VISC_20W) * .01);
					}
				}
				sve.clear();
				{
					vector <struct mfem_data> global_last_residuals;
					for (e = 0; e < experiment.size(); e++)
					{
						sve.push_back(sv);
						vector <struct mfem_data> use_experiment;
						use_experiment.push_back(experiment[e]);
						for (unsigned int l=0; l < final_use_solutes.size(); l++)
						{
						  // printf("0: final_use_solute 2 e%u %u s %g ff0 %g\n", e, l, final_use_solutes[l].s, final_use_solutes[l].k);
						  // fflush(stdout);
						}
						sve[e] = calc_residuals(use_experiment, final_use_solutes, 0e0, 1);
						// printf("0: sve exp %u sol %u var %g\n", e, sve[e].solutes.size(), sve[e].variance);
						// fflush(stdout);
						if (monte_carlo_iterations > 1 && !this_monte_carlo)
						{
							global_last_residuals.push_back(last_residuals[0]);
						}
					} // for e
					if (monte_carlo_iterations > 1 && !this_monte_carlo)
					{
						last_residuals = global_last_residuals;
					}
				}

				BufferResults(experiment, final_use_solutes, sve, "", 0, GA_Params.generations);

				if (this_monte_carlo == monte_carlo_iterations - 1)
				{
					if (f.open(IO_WriteOnly | IO_Append))
					{
						QTextStream ts(&f);
						ts << "\nsubmitted at " + startDateTime.toString("hh:mm:ss") + " on "
						<< startDateTime.toString("MM/dd/yyyy") + " has completed.\n\n"
						<< "The results are attached.\n\n";
					}
					f.close();
				}

				// email results
				if (this_monte_carlo == monte_carlo_iterations - 1)
				{
#ifdef BIN64
					QString email_cmd =
						"perl $ULTRASCAN/bin64/us_email.pl email_list_" + startDateTime.toString("yyMMddhhmmss") +
						" email_text_" + startDateTime.toString("yyMMddhhmmss");
#else

					QString email_cmd =
						"perl $ULTRASCAN/bin/us_email.pl email_list_" + startDateTime.toString("yyMMddhhmmss") +
						" email_text_" + startDateTime.toString("yyMMddhhmmss");
#endif

					cout << email_cmd << endl;
#if !defined(JOB_TIMING)

					printf("0: emailing...\n");
					fflush(stdout);
					if (gridopt != "no")
					{
						system(email_cmd.ascii());
					}
					else
					{
						printf("0: emailing disabled\n");
						fflush(stdout);
					}
					printf("0: echo job complete...\n");
					fflush(stdout);
					// # if !defined(NO_US)
					if (gridopt != "no")
					{
						system("echo mpi_job_complete > $ULTRASCAN/etc/us_gridpipe");
					}
					else
					{
						printf("0: job complete msg not sent to gridpipe\n");
						fflush(stdout);
					}

					// # endif
#endif
#if defined(GLOBAL_JOB_TIMING)
					gettimeofday(&end_tv, NULL);
					printf("0: job time %lu\n",
						   1000000l * (end_tv.tv_sec - start_tv.tv_sec) + end_tv.tv_usec -
						   start_tv.tv_usec);
					fflush(stdout);
#endif

				}
				printf("0: max rss %ld pages\n", maxrss);

				if (monte_carlo_iterations > 1 && !this_monte_carlo)
				{
					printf("0: get_gaussian_means for monte carlo\n");
					fflush(stdout);
					save_gaussians = get_gaussian_means(last_residuals);
					org_experiment = experiment;
				}
				for (i = 1; i < npes; i++)
				{
					if (migration_data[i])
					{
						free(migration_data[i]);
						migration_data[i] = (char *)0;
					}
				}
			}
		} // end for monte carlo
		printf("%d: finalizing\n", myrank);
		MPI_Finalize();
		exit(0);
	}

	if (!myrank)
	{ // master
		cout << "email " << email << " exp " << experiment[0].id << endl;
		fflush(stdout);
		vector <struct gene> org_solutions = solutions;
		float_mc_edge_inc = 0;
		printf("0: float_mc_edge_max %f\n", float_mc_edge_max);
		fflush(stdout);
		printf("0: float_mc_edge_ff0_max %f\n", SA2D_Params.ff0_max);
		fflush(stdout);
		if (float_mc_edge_max && monte_carlo_iterations > 1)
		{
			float_mc_edge_inc = (float_mc_edge_max - SA2D_Params.ff0_max) / (monte_carlo_iterations - 1.0);
			printf("0: float_mc_edge_inc %f\n", float_mc_edge_inc);
			fflush(stdout);
		}
		for (this_monte_carlo = 0; this_monte_carlo < monte_carlo_iterations; this_monte_carlo++)
		{
			if (float_mc_edge_max && (monte_carlo_iterations > 1))
			{
				for (unsigned int i = 0; i < org_solutions.size(); i++)
				{
					double this_max = 0e0;
					for (unsigned int j = 0; j < org_solutions[i].component.size(); j++)
					{
						// first find max
						if (org_solutions[i].component[j].k > this_max)
						{
							this_max = org_solutions[i].component[j].k;
						}
					}
					printf("0: this_max [%d] %f\n", i, this_max);
					fflush(stdout);
					for (unsigned int j = 0; j < org_solutions[i].component.size(); j++)
					{
						if (org_solutions[i].component[j].k == this_max)
						{
							solutions[i].component[j].k = org_solutions[i].component[j].k +
														  this_monte_carlo * float_mc_edge_inc;
							printf("0: float [%d][%d] %f %f\n", i, j,
								   org_solutions[i].component[j].k,
								   solutions[i].component[j].k);
							fflush(stdout);
						}
					}
				}
			}

			Simulation_values last_results;
			if (iterative && !union_results)
			{
				iterative = 0;
			}
			unsigned int procsleft = npes - 1;
			unsigned int x;
			unsigned int i, j;
			MPI_Work_Msg mpi_work_msgs[npes];
			list<Jobqueue> jobqueue;
			Jobqueue thisjob;
			int sleeping[npes];
			memset(sleeping, 0, sizeof(int) * npes);
			Simulation_values results[solutions.size()];
			vector<Simulation_values> unions;
			unsigned int max_experiment_size;
			unsigned int max_job_depth;
			unsigned int last_split_intermediate;
			unsigned int iterations;
			unsigned int ti_noise_size = 0;
			unsigned int ri_noise_size = 0;
			for (i = 0; i < experiment.size(); i++)
			{
				ti_noise_size += experiment[i].radius.size();
				ri_noise_size += experiment[i].scan.size();
			}

			int jobs_outstanding = 0;
			double meniscus_offset = 0;
			double meniscus_end = 0;
			double meniscus_increment = 0;
			vector<Simulation_values> meniscus_results;
			vector<double> meniscus_meniscus;
			vector<unsigned int> meniscus_iterations;

			if (fit_meniscus)
			{
				meniscus_offset = - meniscus_range / 2.0;
				meniscus_end = meniscus_range / 2.0;
				meniscus_increment = meniscus_range / meniscus_gridpoints;
			}

			printf("meniscus offs %.12g end %.12g incr %.12g\n"
				   "experiment[0] meniscus %.12g\n",
				   meniscus_offset,
				   meniscus_end,
				   meniscus_increment,
				   experiment[0].meniscus
				  );
			fflush(stdout);

			// add all experiments to work queue
			max_job_depth = 0;
			last_split_intermediate = 0;
			max_experiment_size = 0;
			iterations = 0;
			for (j = 0; j < solutions.size(); j++)
			{
				thisjob.msg.this_solutions = j;
				thisjob.msg.this_length = solutions[j].component.size();
				if (thisjob.msg.this_length > max_experiment_size)
				{
					max_experiment_size = thisjob.msg.this_length;
				}
				//				printf("sol %d len %d max_len %d\n", j, solutions[j].component.size(), max_experiment_size);
				//				fflush(stdout);
				thisjob.msg.command = 1;
				thisjob.msg.depth = 0;
				thisjob.solutes = solutions[j].component;
				thisjob.msg.meniscus_offset = meniscus_offset;
				jobqueue.push_back(thisjob);
			}

			max_experiment_size += 5;
			if (max_experiment_size < MIN_EXPERIMENT_SIZE)
			{
				max_experiment_size = MIN_EXPERIMENT_SIZE;
			}

#if defined(JOB_TIMING)
			max_experiment_size = 4096;
			unsigned int depth_0 = 0;
			unsigned int depth_0_count = 0;
#endif

			printf("jobs added %d experiments %d solutions %d max_experiment_size %d union %d\n",
				   jobqueue.size(),
				   experiment.size(),
				   solutions.size(),
				   max_experiment_size,
				   union_results);
			fflush(stdout);

#if defined(JOB_TIMING)

			MPI_Barrier(MPI_COMM_WORLD);
			gettimeofday(&start_tv, NULL);
#endif

			if (this_monte_carlo)
			{
				// send the monte carlo data to the workers
				printf("0: monte carlo iteration %u\n", this_monte_carlo);
				printf("0: get_monte_carlo (generate monte carlo data)\n");
				fflush(stdout);
				experiment = get_monte_carlo(org_experiment, save_gaussians);
				if (float_mc_edge_max)
				{
					experiment = org_experiment;
				}
				// send new experiment data
				printf("0: broadcast monte carlo data\n");
				fflush(stdout);
				{
					unsigned int e, k;
					for (e = 0; e < experiment.size(); e++)
					{
						for (k = 0; k < experiment[e].scan.size(); k++)
						{
							MPI_Bcast(&experiment[e].scan[k].conc[0], experiment[e].radius.size(), MPI_DOUBLE, 0, MPI_COMM_WORLD);
							printf("%d: sent exp %u scan %u\n", myrank, e, k);
							fflush(stdout);
						}
					}
				}
			}
			if (this_monte_carlo)
			{
				printf("%d: post monte carlo barrier enter\n", myrank);
				fflush(stdout);
				MPI_Barrier(MPI_COMM_WORLD);
				printf("%d: post monte carlo barrier exit\n", myrank);
				fflush(stdout);
			}

			while(procsleft)
			{
			  //				printf("master waiting for comm\n");
			  // fflush(stdout);
				MPI_Recv(&x, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);
				//				printf("master received %d from %d value %d\n", mpi_status.MPI_TAG, mpi_status.MPI_SOURCE, x);
				//				fflush(stdout);
				switch(mpi_status.MPI_TAG)
				{
				case 1 : // ready for work
				  //					printf("jobs outstanding %d queuesize %d\n", jobs_outstanding, jobqueue.size());
				  //					fflush(stdout);
					if (jobqueue.size())
					{
						thisjob = jobqueue.front();
						printf("master to send job solu %d depth %d to %d\n",
							   thisjob.msg.this_solutions,
							   thisjob.msg.depth,
							   mpi_status.MPI_SOURCE);
						fflush(stdout);
						// serialize experiment & solution
						mpi_work_msgs[mpi_status.MPI_SOURCE].this_solutions = thisjob.msg.this_solutions;
						mpi_work_msgs[mpi_status.MPI_SOURCE].this_length = thisjob.msg.this_length;
						mpi_work_msgs[mpi_status.MPI_SOURCE].command = 1;
						mpi_work_msgs[mpi_status.MPI_SOURCE].depth = thisjob.msg.depth;
						mpi_work_msgs[mpi_status.MPI_SOURCE].meniscus_offset = thisjob.msg.meniscus_offset;
						MPI_Send(&mpi_work_msgs[mpi_status.MPI_SOURCE], sizeof(MPI_Work_Msg), MPI_CHAR, mpi_status.MPI_SOURCE, 0, MPI_COMM_WORLD);
#if defined(SLIST2)

						{
							Simulation_values sv;
							sv.solutes = thisjob.solutes;
							unsigned int i;
							printf("p\t\ts\tk\tc\t send solutes\n");
							for (i = 0; i < sv.solutes.size(); i++)
							{
							    printf("%d\t%d\t%g\t%g\n",
								   myrank,
								   i,
								   sv.solutes[i].s,
								   sv.solutes[i].k
								  );
							}
						}
						fflush(stdout);
#endif

						MPI_Send(&thisjob.solutes[0], thisjob.solutes.size() * sizeof(Solute), MPI_CHAR, mpi_status.MPI_SOURCE, 0, MPI_COMM_WORLD);
						jobs_outstanding++;
						jobqueue.pop_front();
					}
					else
					{
						if (jobs_outstanding)
						{
							// send the sleep request
							mpi_work_msgs[mpi_status.MPI_SOURCE].this_solutions = 0;
							mpi_work_msgs[mpi_status.MPI_SOURCE].this_length = 0;
							mpi_work_msgs[mpi_status.MPI_SOURCE].command = 2; // sleep
							mpi_work_msgs[mpi_status.MPI_SOURCE].depth = 0;
							mpi_work_msgs[mpi_status.MPI_SOURCE].meniscus_offset = 0;
							sleeping[mpi_status.MPI_SOURCE] = 1;
							// printf("master wants to send (sleep) to %d\n", mpi_status.MPI_SOURCE);
							// fflush(stdout);
							MPI_Send(&mpi_work_msgs[mpi_status.MPI_SOURCE], sizeof(MPI_Work_Msg), MPI_CHAR, mpi_status.MPI_SOURCE, 0, MPI_COMM_WORLD);
						}
						else
						{
							// shut them down
							for (i = 0; i < (unsigned int)npes; i++)
							{
								if (sleeping[i])
								{ // send wakeup
								  // printf("master wants to send (wakeup) to %d\n", i);
								  // fflush(stdout);
									mpi_work_msgs[i].this_solutions = 0;
									mpi_work_msgs[i].this_length = 0;
									mpi_work_msgs[i].command = 4; // wakeup sent
									mpi_work_msgs[i].depth = 0;
									mpi_work_msgs[i].meniscus_offset = 0;
									MPI_Send(&mpi_work_msgs[i], sizeof(MPI_Work_Msg), MPI_CHAR, i, 0, MPI_COMM_WORLD);
									sleeping[i] = 0;
								}
							}
							procsleft--;
							mpi_work_msgs[mpi_status.MPI_SOURCE].this_solutions = 0;
							mpi_work_msgs[mpi_status.MPI_SOURCE].this_length = 0;
							mpi_work_msgs[mpi_status.MPI_SOURCE].command = 3; // terminate
							mpi_work_msgs[mpi_status.MPI_SOURCE].depth = 0;
							mpi_work_msgs[mpi_status.MPI_SOURCE].meniscus_offset = 0;
							// printf("master wants to send (terminate) to %d\n", mpi_status.MPI_SOURCE);
							// fflush(stdout);
							MPI_Send(&mpi_work_msgs[mpi_status.MPI_SOURCE], sizeof(MPI_Work_Msg), MPI_CHAR, mpi_status.MPI_SOURCE, 0, MPI_COMM_WORLD);
						}
					}
					break;
				case 3 : // return solute data
					{
						jobs_outstanding--;
						// solutes
						Solute temp_solutes[x];
						unsigned int i;
						unsigned int use_solutions = mpi_work_msgs[mpi_status.MPI_SOURCE].this_solutions;
						//	unsigned int use_length = mpi_work_msgs[mpi_status.MPI_SOURCE].this_length;
						unsigned int use_depth = mpi_work_msgs[mpi_status.MPI_SOURCE].depth;
						MPI_Recv(temp_solutes, x * sizeof(Solute), MPI_CHAR, mpi_status.MPI_SOURCE, 0, MPI_COMM_WORLD, &mpi_status2);
						// printf("master received length %d from %d depth %d\n", x, mpi_status.MPI_SOURCE, use_depth);
						// fflush(stdout);
#if defined(JOB_TIMING)

						if (use_depth == 0)
						{
							depth_0 += x;
							depth_0_count++;
						}
#endif
						Simulation_values sv;
						for (i = 0; i < x; i++)
						{
							sv.solutes.push_back(temp_solutes[i]);
						}
						// variance
						MPI_Recv(
							&sv.variance,
							1, MPI_DOUBLE, mpi_status.MPI_SOURCE, 0, MPI_COMM_WORLD, &mpi_status2);
						sv.variances.clear();
						unsigned int e;
						for (e = 0; e < experiment.size(); e++)
						{
							sv.variances.push_back(0);
						}
						MPI_Recv(
							&sv.variances[0],
							sv.variances.size(), MPI_DOUBLE, mpi_status.MPI_SOURCE, 0, MPI_COMM_WORLD, &mpi_status2);
#if defined(SLIST)

						{
							unsigned int i;
							printf("p\t\ts\tk\tc\tvariance %g\n", sv.variance);
							for (i = 0; i < sv.solutes.size(); i++)
							{
								printf("%d\t%d\t%g\t%g\t%g\n",
									   myrank,
									   i,
									   sv.solutes[i].s,
									   sv.solutes[i].k,
									   sv.solutes[i].c);
							}
						}
						fflush(stdout);
#endif
						// ti_noise
						{
							unsigned int use_radius = ti_noise_size;
							double temp_ti_noise[use_radius];
							MPI_Recv(temp_ti_noise, use_radius,
									 MPI_DOUBLE, mpi_status.MPI_SOURCE, 0, MPI_COMM_WORLD, &mpi_status2);
							for (i = 0; i < use_radius; i++)
							{
								sv.ti_noise.push_back(temp_ti_noise[i]);
							}
						}
						// ri_noise
						{
							unsigned int use_scan = ri_noise_size;
							double temp_ri_noise[use_scan];
							MPI_Recv(temp_ri_noise, use_scan,
									 MPI_DOUBLE, mpi_status.MPI_SOURCE, 0, MPI_COMM_WORLD, &mpi_status2);
							for (i = 0; i < use_scan; i++)
							{
								sv.ri_noise.push_back(temp_ri_noise[i]);
							}
						}
						//	printf("master cp0 use_depth %d use_sol %d use_length %d\n", use_depth, use_solutions, use_length); fflush(stdout);
						if (!use_depth)
						{ // regular return
							results[use_solutions] = sv;
						}
						if (union_results)
						{
						  // printf("solutes union_results\n");
						  // fflush(stdout);
							while(unions.size() <= use_depth)
							{ // vector<Simulation_values> sizes
								Simulation_values bsv;
								bsv.variance = 0;
								unions.push_back(bsv);
							}
							if (sv.solutes.size() + unions[use_depth].solutes.size()
									> max_experiment_size)
							{ // create a new job on the queue
								Jobqueue thisjob;
								thisjob.msg.command = 1;
								thisjob.msg.depth = use_depth + 1;
								if (thisjob.msg.depth < max_job_depth)
								{
									max_job_depth = thisjob.msg.depth;
								}
								if (thisjob.msg.depth < last_split_intermediate)
								{
									last_split_intermediate = thisjob.msg.depth;
								}
								thisjob.msg.this_solutions = 0;
								thisjob.msg.this_length = unions[use_depth].solutes.size();
								thisjob.solutes = unions[use_depth].solutes;
								thisjob.msg.meniscus_offset = meniscus_offset;
								jobqueue.push_back(thisjob);
								unions[use_depth] = sv;
							}
							else
							{
								if (unions[use_depth].solutes.size())
								{
									unions[use_depth].variance = -1;
									for (i = 0; i < sv.solutes.size(); i++)
									{
										if (find(unions[use_depth].solutes.begin(),
												 unions[use_depth].solutes.end(),
												 sv.solutes[i]) ==
												unions[use_depth].solutes.end())
										{
											unions[use_depth].solutes.push_back(sv.solutes[i]);
										}
									}
								}
								else
								{
									unions[use_depth] = sv;
								}
							}

							// clear mpi message for this source
							mpi_work_msgs[mpi_status.MPI_SOURCE].this_solutions = 0;
							mpi_work_msgs[mpi_status.MPI_SOURCE].this_length = 0;
							mpi_work_msgs[mpi_status.MPI_SOURCE].command = 0; // idle
							mpi_work_msgs[mpi_status.MPI_SOURCE].depth = 0;
							mpi_work_msgs[mpi_status.MPI_SOURCE].meniscus_offset = 0;

							// check msgs & queue to see if any outstanding at use_depth or earlier
							list<Jobqueue>::iterator jq;
							int any_left = 0;
							for (jq = jobqueue.begin(); !any_left && jq != jobqueue.end(); jq++)
							{
								if (jq->msg.depth <= use_depth)
								{
									any_left = 1;
									// printf("found in queue\n");
									// fflush(stdout);
								}
							}
							for (i = 0; !any_left && i < (unsigned int)npes; i++)
							{
								if (mpi_work_msgs[i].command == 1 &&
										mpi_work_msgs[i].depth <= use_depth)
								{
									any_left = 1;
									// printf("found in messages_left\n");
									// fflush(stdout);
								}
							}
							if (!any_left)
							{
								// this union as a job
								// printf("!any_left\n");
								// fflush(stdout);
								// printf("lsi %d use_depth %d\n", last_split_intermediate,
								//	   use_depth);
							        // fflush(stdout);
								if (last_split_intermediate + 1 >= use_depth)
								{
									Jobqueue thisjob;
									thisjob.msg.command = 1;
									thisjob.msg.depth = use_depth + 1;
									if (thisjob.msg.depth < max_job_depth)
									{
										max_job_depth = thisjob.msg.depth;
									}
									thisjob.msg.this_solutions = 0;
									thisjob.msg.this_length = unions[use_depth].solutes.size();
									thisjob.solutes = unions[use_depth].solutes;
									thisjob.msg.meniscus_offset = meniscus_offset;
									jobqueue.push_back(thisjob);
									unions[use_depth] = sv;
								}
								else
								{
									// this job is done
									// printf("this job done\n");
									// fflush(stdout);
									if (iterative)
									{
									  // printf("iterative union\n");
									  //	fflush(stdout);
										{
											Simulation_values sv = calc_residuals(experiment, unions[unions.size()-1].solutes, 0e0, 0);
											sv = unions[unions.size() - 1];
											if (sv.solutes.size() != unions[unions.size()-1].solutes.size())
											{
												printf("!!iterative final mismatch! fixing correct size %d\n", sv.solutes.size());
											}
											unions[unions.size() - 1] = sv;
										}
#if defined(SLIST2)

										{
											Simulation_values sv;
											sv = last_results;
											unsigned int i;
											printf("p\t\ts\tk\tc\t last solutes\n");
											for (i = 0; i < sv.solutes.size(); i++)
											{
												printf("%d\t%d\t%g\t%g\n",
													   myrank,
													   i,
													   sv.solutes[i].s,
													   sv.solutes[i].k
													  );
											}
											sv = unions[unions.size() - 1];
											printf("p\t\ts\tk\tc\t current solutes\n");
											for (i = 0; i < sv.solutes.size(); i++)
											{
												printf("%d\t%d\t%g\t%g\n",
													   myrank,
													   i,
													   sv.solutes[i].s,
													   sv.solutes[i].k
													  );
											}
										}
										fflush(stdout);
#endif

										if (iterations > 0)
										{
											printf("tolerance %.12g %.12g %.12g\n",
												   last_results.variance,
												   unions[unions.size() - 1].variance,
												   fabs(last_results.variance -
														unions[unions.size() - 1].variance)
												  );
											fflush(stdout);
										}
										int any_change = 1;
										if (last_results.solutes.size() !=
												unions[unions.size() - 1].solutes.size())
										{
											printf("iterative size difference last %d new %d\n",
												   last_results.solutes.size(),
												   unions[unions.size() - 1].solutes.size());
											fflush(stdout);
										}

										if (last_results.solutes.size() ==
												unions[unions.size() - 1].solutes.size())
										{
											any_change = 0;
											for (i = 0; i < last_results.solutes.size(); i++)
											{
												if (find(last_results.solutes.begin(),
														 last_results.solutes.end(),
														 unions[unions.size() - 1].solutes[i]) ==
														last_results.solutes.end())
												{
													any_change = 1;
													break;
												}
											}
										}
										if (any_change &&
												iterations >= max_iterations ||
												(iterations > 1 &&
												 fabs(last_results.variance -
													  unions[unions.size() - 1].variance)
												 <= VARIANCE_IMPROVEMENT_TOLERANCE))
										{
											printf("max iteration or tolerance limit hit %d %.12g %.12g %.12g\n",
												   max_iterations,
												   last_results.variance,
												   unions[unions.size() - 1].variance,
												   fabs(last_results.variance -
														unions[unions.size() - 1].variance)
												  );
											fflush(stdout);
											any_change = 0;
										}
										if (any_change)
										{
											iterations++;
											printf("iterative union forces change\n");
											fflush(stdout);
											last_results = unions[unions.size() - 1];
											max_job_depth = 0;
											last_split_intermediate = 0;
											max_experiment_size = 0;
											unions.clear();
											for (j = 0; j < solutions.size(); j++)
											{
												thisjob.msg.this_solutions = j;
												thisjob.solutes = solutions[j].component;
												results[j].solutes.clear();
												for (i = 0; i < last_results.solutes.size(); i++)
												{
													if (find(thisjob.solutes.begin(),
															 thisjob.solutes.end(),
															 last_results.solutes[i]) == thisjob.solutes.end())
													{
														thisjob.solutes.push_back(last_results.solutes[i]);
													}
												}
												thisjob.msg.this_length = thisjob.solutes.size();
												if (thisjob.msg.this_length > max_experiment_size)
												{
													max_experiment_size = thisjob.msg.this_length;
												}
												thisjob.msg.command = 1;
												thisjob.msg.depth = 0;
												thisjob.msg.meniscus_offset = meniscus_offset;
												jobqueue.push_back(thisjob);
											}
											// wakeup any sleepers
											for (i = 0; i < (unsigned int)npes; i++)
											{
												if (sleeping[i])
												{ // send wakeup
													printf("master wants to send (wakeup) to %d\n", i);
													fflush(stdout);
													mpi_work_msgs[i].this_solutions = 0;
													mpi_work_msgs[i].this_length = 0;
													mpi_work_msgs[i].command = 4; // wakeup sent
													mpi_work_msgs[i].depth = 0;
													mpi_work_msgs[i].meniscus_offset = 0;
													MPI_Send(&mpi_work_msgs[i], sizeof(MPI_Work_Msg), MPI_CHAR, i, 0, MPI_COMM_WORLD);
													sleeping[i] = 0;
												}
											}
										}
										else
										{
										  // printf("iterative union no change\n");
										  // fflush(stdout);
											if (fit_meniscus)
											{
												printf("experiment meniscus_offset %.12g variance %.12g solutes %d\n",
													   meniscus_offset,
													   unions[unions.size() - 1].variance,
													   unions[unions.size() - 1].solutes.size()
													  );
												meniscus_results.push_back(unions[unions.size() - 1]);
												meniscus_meniscus.push_back(meniscus_offset);
												meniscus_iterations.push_back(iterations);

												meniscus_offset += meniscus_increment;
												if (meniscus_offset <= meniscus_end)
												{
													iterations = 0;
													printf("meniscus increment\n");
													fflush(stdout);
													last_results.solutes.clear();
													last_results.variance = 0;
													max_job_depth = 0;
													last_split_intermediate = 0;
													max_experiment_size = 0;
													unions.clear();
													for (j = 0; j < solutions.size(); j++)
													{
														thisjob.msg.this_solutions = j;
														thisjob.solutes = solutions[j].component;
														results[j].solutes.clear();
														for (i = 0; i < last_results.solutes.size(); i++)
														{
															if (find(thisjob.solutes.begin(),
																	 thisjob.solutes.end(),
																	 last_results.solutes[i]) == thisjob.solutes.end())
															{
																thisjob.solutes.push_back(last_results.solutes[i]);
															}
														}
														thisjob.msg.this_length = thisjob.solutes.size();
														if (thisjob.msg.this_length > max_experiment_size)
														{
															max_experiment_size = thisjob.msg.this_length;
														}
														thisjob.msg.command = 1;
														thisjob.msg.depth = 0;
														thisjob.msg.meniscus_offset = meniscus_offset;
														jobqueue.push_back(thisjob);
													}
													// wakeup any sleepers
													for (i = 0; i < (unsigned int)npes; i++)
													{
														if (sleeping[i])
														{ // send wakeup
															printf("master wants to send (wakeup) to %d\n", i);
															fflush(stdout);
															mpi_work_msgs[i].this_solutions = 0;
															mpi_work_msgs[i].this_length = 0;
															mpi_work_msgs[i].command = 4; // wakeup sent
															mpi_work_msgs[i].depth = 0;
															mpi_work_msgs[i].meniscus_offset = 0;
															MPI_Send(&mpi_work_msgs[i], sizeof(MPI_Work_Msg), MPI_CHAR, i, 0, MPI_COMM_WORLD);
															sleeping[i] = 0;
														}
													}
												}
											}
										}
									}
									else
									{ // from if (iterative)
										// -----
										if (fit_meniscus)
										{
											printf("experiment meniscus %.12g variance %.12g solutes %d\n",
												   meniscus_offset,
												   unions[unions.size() - 1].variance,
												   unions[unions.size() - 1].solutes.size()
												  );
#if defined(SLIST2)

											{
												Simulation_values sv;
												sv = unions[unions.size() - 1];
												unsigned int i;
												printf("p\t\ts\tk\tc\t meniscus %.12gs\n",
													   meniscus_offset
													  );
												fflush(stdout);
												for (i = 0; i < sv.solutes.size(); i++)
												{
													printf("%d\t%d\t%g\t%g\n",
														   myrank,
														   i,
														   sv.solutes[i].s,
														   sv.solutes[i].k
														  );
												}
											}
											fflush(stdout);
#endif

											meniscus_results.push_back(unions[unions.size() - 1]);
											meniscus_meniscus.push_back(meniscus_offset);
											meniscus_iterations.push_back(iterations);

											meniscus_offset += meniscus_increment;
											if (meniscus_offset <= meniscus_end)
											{
												iterations = 0;
												printf("meniscus increment\n");
												fflush(stdout);
												last_results.solutes.clear();
												last_results.variance = 0;
												max_job_depth = 0;
												last_split_intermediate = 0;
												max_experiment_size = 0;
												unions.clear();
												for (j = 0; j < solutions.size(); j++)
												{
													thisjob.msg.this_solutions = j;
													thisjob.solutes = solutions[j].component;
													results[j].solutes.clear();
													for (i = 0; i < last_results.solutes.size(); i++)
													{
														if (find(thisjob.solutes.begin(),
																 thisjob.solutes.end(),
																 last_results.solutes[i]) == thisjob.solutes.end())
														{
															thisjob.solutes.push_back(last_results.solutes[i]);
														}
													}
													thisjob.msg.this_length = thisjob.solutes.size();
													if (thisjob.msg.this_length > max_experiment_size)
													{
														max_experiment_size = thisjob.msg.this_length;
													}
													thisjob.msg.command = 1;
													thisjob.msg.depth = 0;
													thisjob.msg.meniscus_offset = meniscus_offset;
													jobqueue.push_back(thisjob);
												}
												// wakeup any sleepers
												for (i = 0; i < (unsigned int)npes; i++)
												{
													if (sleeping[i])
													{ // send wakeup
														printf("master wants to send (wakeup) to %d\n", i);
														fflush(stdout);
														mpi_work_msgs[i].this_solutions = 0;
														mpi_work_msgs[i].this_length = 0;
														mpi_work_msgs[i].command = 4; // wakeup sent
														mpi_work_msgs[i].depth = 0;
														mpi_work_msgs[i].meniscus_offset = 0;
														MPI_Send(&mpi_work_msgs[i], sizeof(MPI_Work_Msg), MPI_CHAR, i, 0, MPI_COMM_WORLD);
														sleeping[i] = 0;
													}
												}
											}
										}
										// -----

									}
								}
							}
						}
						else
						{
							if (fit_meniscus)
							{
								// !union_results, fit meniscus stuff
								// clear mpi message for this source
								mpi_work_msgs[mpi_status.MPI_SOURCE].this_solutions = 0;
								mpi_work_msgs[mpi_status.MPI_SOURCE].this_length = 0;
								mpi_work_msgs[mpi_status.MPI_SOURCE].command = 0; // idle
								mpi_work_msgs[mpi_status.MPI_SOURCE].depth = 0;
								mpi_work_msgs[mpi_status.MPI_SOURCE].meniscus_offset = 0;

								// check msgs & queue to see if any outstanding at use_depth or earlier
								list<Jobqueue>::iterator jq;
								int any_left = 0;
								for (jq = jobqueue.begin(); !any_left && jq != jobqueue.end(); jq++)
								{
									if (jq->msg.depth <= use_depth)
									{
										any_left = 1;
										printf("found in queue\n");
										fflush(stdout);
									}
								}
								for (i = 0; !any_left && i < (unsigned int)npes; i++)
								{
									if (mpi_work_msgs[i].command == 1 &&
											mpi_work_msgs[i].depth <= use_depth)
									{
										any_left = 1;
										printf("found in messages_left\n");
										fflush(stdout);
									}
								}
								if (!any_left)
								{
									// we can increment the meniscus
									printf("experiment solution %d meniscus %.12g variance %.12g\n",
										   use_solutions,
										   meniscus_offset,
										   results[use_solutions].variance);
#if defined(SLIST2)

									{
										Simulation_values sv;
										sv = results[use_solutions];
										unsigned int i;
										printf("p\t\ts\tk\tc\t meniscus %.12gs\n",
											   meniscus_offset
											  );
										fflush(stdout);
										for (i = 0; i < sv.solutes.size(); i++)
										{
											printf("%d\t%d\t%g\t%g\n",
												   myrank,
												   i,
												   sv.solutes[i].s,
												   sv.solutes[i].k
												  );
										}
									}
									fflush(stdout);
#endif

									meniscus_results.push_back(results[use_solutions]);
									meniscus_meniscus.push_back(meniscus_offset);
									meniscus_iterations.push_back(iterations);

									meniscus_offset += meniscus_increment;
									if (meniscus_offset <= meniscus_end)
									{
										iterations = 0;
										printf("meniscus increment\n");
										fflush(stdout);
										last_results.solutes.clear();
										last_results.variance = 0;
										max_job_depth = 0;
										last_split_intermediate = 0;
										max_experiment_size = 0;
										unions.clear();
										for (j = 0; j < solutions.size(); j++)
										{
											thisjob.msg.this_solutions = j;
											thisjob.solutes = solutions[j].component;
											results[j].solutes.clear();
											for (i = 0; i < last_results.solutes.size(); i++)
											{
												if (find(thisjob.solutes.begin(),
														 thisjob.solutes.end(),
														 last_results.solutes[i]) == thisjob.solutes.end())
												{
													thisjob.solutes.push_back(last_results.solutes[i]);
												}
											}
											thisjob.msg.this_length = thisjob.solutes.size();
											if (thisjob.msg.this_length > max_experiment_size)
											{
												max_experiment_size = thisjob.msg.this_length;
											}
											thisjob.msg.command = 1;
											thisjob.msg.depth = 0;
											thisjob.msg.meniscus_offset = meniscus_offset;
											jobqueue.push_back(thisjob);
										}
										// wakeup any sleepers
										for (i = 0; i < (unsigned int)npes; i++)
										{
											if (sleeping[i])
											{ // send wakeup
												printf("master wants to send (wakeup) to %d\n", i);
												fflush(stdout);
												mpi_work_msgs[i].this_solutions = 0;
												mpi_work_msgs[i].this_length = 0;
												mpi_work_msgs[i].command = 4; // wakeup sent
												mpi_work_msgs[i].depth = 0;
												mpi_work_msgs[i].meniscus_offset = 0;
												MPI_Send(&mpi_work_msgs[i], sizeof(MPI_Work_Msg), MPI_CHAR, i, 0, MPI_COMM_WORLD);
												sleeping[i] = 0;
											}
										}
									}
								}
							}
						}
					}
					break;
				default :
					printf("!!!master received unknown %d from %d\n", x, mpi_status.MPI_SOURCE);
					fflush(stdout);
					break;
				}
			}
#if defined(JOB_TIMING)
			gettimeofday(&end_tv, NULL);
			double pct = (depth_0 * 1e0);
			pct /= (depth_0_count * 1e0);
			printf("depth_0 %u depth_0_count %u %g\n",
				   depth_0, depth_0_count, pct);
			printf("|%u|%g|%ld|\n",
				   solutions[0].component.size(),
				   pct,
				   1000000l * (end_tv.tv_sec - start_tv.tv_sec) + end_tv.tv_usec -
				   start_tv.tv_usec);
#endif

			printf("master finishing up\n");
			endDateTime = QDateTime::currentDateTime();
			QFile f("email_text_" + startDateTime.toString("yyMMddhhmmss"));
			if (this_monte_carlo == 0)
			{
				if (f.open(IO_WriteOnly))
				{
					QTextStream ts(&f);
					ts << email + "\n"
					<< "Your " + analysis_type + " grid job has finished\n\n";

					ts << "2DSA control parameters were defined as follows:\n\n";

					if (monte_carlo_iterations > 1)
					{
						ts << "Monte Carlo iterations:     " << monte_carlo_iterations << endl;
					}

					if (analysis_type == "2DSA")
					{
						ts << "s minimum:                  " << SA2D_Params.s_min << endl;
						ts << "s maximum:                  " << SA2D_Params.s_max << endl;
						ts << "s resolution:               " << SA2D_Params.s_resolution << endl;
					}
					if (analysis_type == "2DSA_MW")
					{
						ts << "MW minimum:                 " << SA2D_Params.mw_min << endl;
						ts << "MW maximum:                 " << SA2D_Params.mw_max << endl;
						ts << "Largest oligomer            " << SA2D_Params.max_mer << endl;
						ts << "grid resolution:            " << SA2D_Params.grid_resolution << endl;
					}

					ts << "f/f0 minimum:               " << SA2D_Params.ff0_min << endl;
					ts << "f/f0 maximum:               " << SA2D_Params.ff0_max << endl;
					ts << "f/f0 resolution:            " << SA2D_Params.ff0_resolution << endl;
					ts << "Grid repetitions:           " << SA2D_Params.uniform_grid_repetition << endl;
					if (use_iterative)
					{
						ts << "Iterative method:           on\n";
						ts << "Maximum iterations:         " << max_iterations << endl;
					}
					if (regularization > 0)
					{
						ts << "Regularization factor:      " << regularization << endl;
					}
					if (fit_tinoise)
					{
						ts << "Fit TI noise:               on\n";
					}
					if (fit_rinoise)
					{
						ts << "Fit RI noise:               on\n";
					}
					if (fit_meniscus)
					{
						ts << "Fit meniscus:               on\n";
						ts << "Meniscus range:             " << meniscus_range << "\n";
						ts << "Meniscus gridpoints:        " << meniscus_gridpoints << "\n";
					}
					if (float_mc_edge_max)
					{
						ts << "Float Mc f/f0 edge maximum: " << float_mc_edge_max << endl;
					}
					ts << endl;
					ts << "The results of your " + analysis_type + " analysis involved datasets:\n\n";
				}
				f.close();
			}

			if (union_results)
			{ // write the unions
				if (fit_meniscus)
				{
					for (j = 0; j < meniscus_results.size(); j++)
					{
						printf("writing %u size %u\n", j, meniscus_results[j].solutes.size());
						fflush(stdout);
						Simulation_values sv = calc_residuals(experiment, meniscus_results[j].solutes, meniscus_meniscus[j], 0);
						printf("writing return from calc_resid %u size %u\n", j, meniscus_results[j].solutes.size());
						fflush(stdout);
						if (sv.solutes.size() != meniscus_results[j].solutes.size())
						{
							printf("!!<fi>final mismatch! writing size %u\n", sv.solutes.size());
							fflush(stdout);
						}
						if (regularization > 0e0)
						{
							sv = regularize(sv, meniscus_meniscus[j]);
						}
						printf("buffer results %u\n", j);
						fflush(stdout);
						BufferResults(experiment, sv, QString("_m%1").arg(j), meniscus_meniscus[j], meniscus_iterations[j]);
						printf("results buffered %u\n", j);
						fflush(stdout);
					}
				}
				else
				{
					printf("writing depth %d size %d\n", unions.size(), unions[unions.size() - 1].solutes.size());
					fflush(stdout);
					Simulation_values sv = calc_residuals(experiment, unions[unions.size()-1].solutes, 0e0, 0);
					if (sv.solutes.size() != unions[unions.size()-1].solutes.size())
					{
						printf("!!final mismatch! writing size %u\n", sv.solutes.size());
						fflush(stdout);
					}
					if (regularization > 0e0)
					{
						sv = regularize(sv, 0e0);
					}
					BufferResults(experiment, sv, "", 0, iterations);
					printf("results buffered\n");
					fflush(stdout);
				}
			}
			else
			{
				unsigned int j;
				if (fit_meniscus)
				{
					for (j = 0; j < meniscus_results.size(); j++)
					{
						Simulation_values sv = meniscus_results[j];
						if (regularization > 0e0)
						{
							sv = regularize(sv, meniscus_meniscus[j]);
						}
						BufferResults(experiment, sv, QString("_m%1").arg(j), meniscus_meniscus[j], meniscus_iterations[j]);
					}
				}
				else
				{
					for (j = 0; j < solutions.size(); j++)
					{
						Simulation_values sv = results[j];
						if (regularization > 0e0)
						{
							sv = regularize(sv, 0);
						}
						BufferResults(experiment, sv, QString("%1").arg(j + 1), 0, 1);
					}
				}
			}
			if (this_monte_carlo == monte_carlo_iterations - 1)
			{
				if (f.open(IO_WriteOnly | IO_Append))
				{
					QTextStream ts(&f);
					ts << "\nsubmitted at " + startDateTime.toString("hh:mm:ss") + " on "
					<< startDateTime.toString("MM/dd/yyyy") + " has completed.\n\n"
					<< "The results are attached.\n\n";
				}
				f.close();
			}

			if (monte_carlo_iterations > 1 && !this_monte_carlo)
			{
				printf("0: get_gaussian_means for monte carlo\n");
				fflush(stdout);
				save_gaussians = get_gaussian_means(last_residuals);
				org_experiment = experiment;
			}

			// email results
			if (this_monte_carlo == monte_carlo_iterations - 1)
			{
#ifdef BIN64
				QString email_cmd =
					"perl $ULTRASCAN/bin64/us_email.pl email_list_" + startDateTime.toString("yyMMddhhmmss") +
					" email_text_" + startDateTime.toString("yyMMddhhmmss");
#else

				QString email_cmd =
					"perl $ULTRASCAN/bin/us_email.pl email_list_" + startDateTime.toString("yyMMddhhmmss") +
					" email_text_" + startDateTime.toString("yyMMddhhmmss");
#endif

				cout << email_cmd << endl;
#if !defined(JOB_TIMING)

				printf("0: emailing...\n");
				fflush(stdout);
				if (gridopt != "no")
				{
					system(email_cmd.ascii());
				}
				else
				{
					printf("0: emailing disabled\n");
					fflush(stdout);
				}
				printf("0: echo job complete...\n");
				fflush(stdout);
				// # if !defined(NO_US)
				if (gridopt != "no")
				{
					system("echo mpi_job_complete > $ULTRASCAN/etc/us_gridpipe");
				}
				else
				{
					printf("0: job complete msg not sent to gridpipe\n");
					fflush(stdout);
				}
				// # endif
#endif
#if defined(GLOBAL_JOB_TIMING)
				gettimeofday(&end_tv, NULL);
				printf("0: job time %lu\n",
					   1000000l * (end_tv.tv_sec - start_tv.tv_sec) + end_tv.tv_usec -
					   start_tv.tv_usec);
				fflush(stdout);
#endif

			}
		}
		printf("0: max rss %ld pages\n", maxrss);
		printf("master exiting up\n");
	}
	else
	{ // worker
		for (this_monte_carlo = 0; this_monte_carlo < monte_carlo_iterations; this_monte_carlo++)
		{
			if (this_monte_carlo)
			{
				// receive the monte carlo data
				printf("%d: monte carlo iteration %u\n", myrank, this_monte_carlo);
				// send new experiment data
				printf("%d: receive broadcast of monte carlo data\n", myrank);
				fflush(stdout);
				{
					unsigned int e, k;
					for (e = 0; e < experiment.size(); e++)
					{
						for (k = 0; k < experiment[e].scan.size(); k++)
						{
							MPI_Bcast(&experiment[e].scan[k].conc[0], experiment[e].radius.size(), MPI_DOUBLE, 0, MPI_COMM_WORLD);
							printf("%d: received exp %u scan %u\n", myrank, e, k);
							fflush(stdout);
						}
					}
				}
			}
			if (this_monte_carlo)
			{
				printf("%d: post monte carlo barrier enter\n", myrank);
				fflush(stdout);
				MPI_Barrier(MPI_COMM_WORLD);
				printf("%d: post monte carlo barrier exit\n", myrank);
				fflush(stdout);
			}
			MPI_Work_Msg mpi_work_msg;
			unsigned int x;
			int cont = 1;
			printf("worker %d starting\n", myrank);
			fflush(stdout);
#ifdef JOB_TIMING

			MPI_Barrier(MPI_COMM_WORLD);
#endif

			while(cont)
			{
				x = 0;
				// printf("worker %d polling master\n", myrank);
				// fflush(stdout);
				MPI_Send(&x, 1, MPI_INT, 0, 1, MPI_COMM_WORLD); // let master know we are ready
				MPI_Recv(&mpi_work_msg, sizeof(MPI_Work_Msg), MPI_CHAR, 0, 0, MPI_COMM_WORLD, &mpi_status); // get masters' response
				//printf("node %d received value %d %d %d %.12g, from master\n", myrank,
				//	   mpi_work_msg.this_solutions,
				//	   mpi_work_msg.this_length,
				//	   mpi_work_msg.command,
				//	   mpi_work_msg.meniscus_offset);
				// fflush(stdout);
				switch(mpi_work_msg.command)
				{
				case 1 :
					// receive solutes
					{
						vector<Solute> use_solutes;
						{
							Solute tmp_solutes[mpi_work_msg.this_length];
							unsigned int i;
							MPI_Recv(tmp_solutes, mpi_work_msg.this_length * sizeof(Solute), MPI_CHAR, 0, 0, MPI_COMM_WORLD, &mpi_status2);
							for (i = 0; i < mpi_work_msg.this_length; i++)
							{
								use_solutes.push_back(tmp_solutes[i]);
							}
						}
#if defined(SLIST2)

						{
							Simulation_values sv;
							sv.solutes = use_solutes;
							unsigned int i;
							printf("p\t\ts\tk\tc\t received solutes\n");
							for (i = 0; i < sv.solutes.size(); i++)
							{
								printf("%d\t%d\t%g\t%g\n",
									   myrank,
									   i,
									   sv.solutes[i].s,
									   sv.solutes[i].k
									  );
							}
						}
						fflush(stdout);
#endif

						// printf("p %d using meniscus %.12g\n", myrank, mpi_work_msg.meniscus_offset);
						sv = calc_residuals(experiment, use_solutes, mpi_work_msg.meniscus_offset, 0);
						x = sv.solutes.size();
						MPI_Send(&x, 1, MPI_INT, 0, 3, MPI_COMM_WORLD); // let master know we are ready
						MPI_Send(&sv.solutes[0], x * sizeof(Solute), MPI_CHAR, 0, 0, MPI_COMM_WORLD);
						MPI_Send(&sv.variance, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
						MPI_Send(&sv.variances[0], sv.variances.size(), MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
						MPI_Send(&sv.ti_noise[0], sv.ti_noise.size(), MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
						MPI_Send(&sv.ri_noise[0], sv.ri_noise.size(), MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
					}
					break;
				case 2 :
					// go to sleep
					printf("node %d go to sleep\n", myrank);
					fflush(stdout);
					MPI_Recv(&mpi_work_msg, sizeof(MPI_Work_Msg), MPI_CHAR, 0, 0, MPI_COMM_WORLD, &mpi_status); // get masters' response
					printf("node %d wakeup\n", myrank);
					fflush(stdout);
					break;
				case 3 :
					// we're done
					cont = 0;
					printf("node %d received quit from master\n", myrank);
					printf("%d: max rss %ld pages\n", myrank, maxrss);
					fflush(stdout);
					fflush(stdout);
					break;
				default :
					// we're done
					cont = 0;
					printf("!!!node %d received unknown command %d from master\n", myrank, mpi_work_msg.command);
					fflush(stdout);
					break;
				}
			}
		}
	}
	printf("%d: finalizing\n", myrank);
	fflush(stdout);
	MPI_Finalize();
	exit(0);
}

extern void clear_data(mfem_data *);

Simulation_values US_fe_nnls_t::calc_residuals(vector <struct mfem_data> experiment,
		vector < Solute > solutes,
		double meniscus_offset,
		int return_all_solutes)
{
	//	printf("%d: calc_residuals\n", myrank); fflush(stdout);
	Simulation_values sv;

#if defined(JOB_TIMING_CR)

	printf("job_timing_cr size %u\n", solutes.size());
	fflush(stdout);
# if defined(JOB_TIMING_CRMP)

	printf("job_timing_crmp size %u\n", solutes.size());
	fflush(stdout);
	unsigned int use_size, tmp_uint;
	vector<Solute> crmp_save_solutes = solutes;
	vector<struct mfem_data> save_experiment = experiment;
	for (use_size = 1; use_size <= crmp_save_solutes.size(); use_size *= 2)
	{
		printf("job_timing_crmp %u of %u\n", use_size, crmp_save_solutes.size());
		fflush(stdout);
		solutes.clear();
		experiment = save_experiment;
		for (tmp_uint = 0; tmp_uint < use_size; tmp_uint++)
		{
			solutes.push_back(crmp_save_solutes[tmp_uint]);
		}
# endif
		gettimeofday(&start_tv_cr, NULL);
#endif

		vector<double> ti_noise;
		vector<double> ri_noise;
		// #if defined(DO_RESIDUALS)
		//	float residual = 0;
		vector<Solute> save_solutes = solutes;
		vector<struct mfem_data> residuals = experiment;
		vector<struct mfem_data> save_experiment = experiment;
		QString str;
		double *nnls_a,
		*nnls_b,
		*nnls_x,
		nnls_rnorm, *nnls_wp, *nnls_zzp, initial_concentration = 1.0;
		int *nnls_indexp, result;
		vector<struct mfem_data> fem_data = experiment;
		struct mfem_initial initCVector[experiment.size()];
		unsigned int i, j, k = 0, count;
		i = 0;
		unsigned int ti_noise_size = 0;
		unsigned int ri_noise_size = 0;
		for (j = 0; j < experiment.size(); j++)
		{
			i += experiment[j].radius.size() * experiment[j].scan.size();
			ti_noise_size += experiment[j].radius.size();
			ri_noise_size += experiment[j].scan.size();
		}
		unsigned int total_points_size = i;

		j = solutes.size();
		//	printf("i %d j %d\n", i, j);
		nnls_a = new double[i * j];	// contains the model functions,
		// end-to-end
		//	unsigned int a_size = i * j;
		nnls_b = new double[i];	// contains the experimental data
		unsigned int b_size = i;
		nnls_zzp = new double[i];	// pre-allocated working space for nnls
		nnls_x = new double[j];	// the solution vector, pre-allocated for
		// nnls
		nnls_wp = new double[j];	// pre-allocated working space for nnls,
		// On exit, wp[] will contain the dual
		// solution vector, wp[i]=0.0 for all i in
		// set p and wp[i]<=0.0 for all i in set
		// z. */
		nnls_indexp = new int[j];
		//	printf("%d: calc_residuals 1\n", myrank); fflush(stdout);
		US_MovingFEM *mfem[experiment.size()];
		for (i = 0; i < experiment.size(); i++)
		{
			mfem[i] = new US_MovingFEM(&fem_data[i], false);
		}
		//	mfem->fprintparams(stdout);

		//	cerr << "p1\n";
		// initialize experimental data array sizes and radius positions:
		//	clear_data(&residuals);
		//	printf("%d: calc_residuals 2\n", myrank); fflush(stdout);
		for (i = 0; i < experiment.size(); i++)
		{
			initCVector[i].concentration.clear();
			initCVector[i].radius.clear();
		}
		//	cerr << "p2\n";
		//	printf("%g %g %g\n",
		//	experiment.scan[0].conc[0],
		//	experiment.scan[0].conc[5],
		//	experiment.scan[5].conc[0]);

		count = 0;
		unsigned int e;
		//		printf("%d: calc_residuals 3\n", myrank); fflush(stdout);
		for (e = 0; e < experiment.size(); e++)
		{
			experiment[e].meniscus += meniscus_offset;
			for (i = 0; i < experiment[e].scan.size(); i++)
			{
				for (j = 0; j < experiment[e].radius.size(); j++)
				{
					// populate the A matrix for the NNLS routine with the model
					// function:
					nnls_b[count] = experiment[e].scan[i].conc[j];
					count++;
				}
			}
		}
		/*	printf("s20w_correction %.12g D20w_correction %.12g scan.size %d\n",
		experiment.s20w_correction,
		experiment.D20w_correction,
		experiment.scan.size());
		fflush(stdout);
		*/
		// check this out later??
		//	for (i=0; i < solutes.size(); i++) {
		//	solutes[i].s /= experiment.s20w_correction;
		//	}
		//	cerr << "p3\n";
		//	puts("calc_residuals 4"); fflush(stdout);

#if defined(SHOW_TIMING)
		gettimeofday(&start_tv, NULL);
#endif

		count = 0;
		for (i = 0; i < solutes.size(); i++)
		{
			for (e = 0; e < experiment.size(); e++)
			{
				// for each term in the linear combination we need to reset the
				// simulation vectors, the experimental vector simply keeps
				// getting overwritten:
				str.sprintf(tr
							("Calculating Lamm Equation %d of %d\n"),
							i + 1, solutes.size());
				//	cout << str;
				//	cerr << "p3b\n";
				clear_data(&fem_data[e]);
				for (j = 0; j < experiment[e].scan.size(); j++)
				{
					for (k = 0; k < experiment[e].radius.size(); k++)
					{
						// reset concentration to zero:
						experiment[e].scan[j].conc[k] = 0.0;
					}
				}
				//	cerr << myrank << " p4\n";
				double D_20w = (R * K20) /
							(AVOGADRO * 18 * M_PI * pow(solutes[i].k * VISC_20W, 3.0/2.0) *
							pow((solutes[i].s * experiment[e].vbar20)/(2.0 * (1.0 - experiment[e].vbar20 * DENS_20W)), 0.5));
				double D_tb = D_20w/experiment[e].D20w_correction;

#if defined(DEBUG_HYDRO)

				printf("experiment %d s_correction: %.8g D_correction: %.8g\n",
					   e, experiment[e].s20w_correction, experiment[e].D20w_correction);
				printf("s_20w: %.8g s_tb: %.8g k: %.8g\n",
					   solutes[i].s, solutes[i].s / experiment[e].s20w_correction, solutes[i].k);
				printf("D_20w: %.8g D_tb: %.8g\n", D_20w, D_tb);
				printf("AVOGADRO: %.8g VISC_20W: %.8g DENS_20W: %8g vbar20: %.8g R: %.8g\n\n", AVOGADRO, VISC_20W, DENS_20W, experiment[e].vbar20, R);
				fflush(stdout);
#endif
				mfem[e]->set_params(100, solutes[i].s / experiment[e].s20w_correction, D_tb,
									(double) experiment[e].rpm,
									experiment[e].scan[experiment[e].scan.size() - 1].time,
									experiment[e].meniscus,
									experiment[e].bottom,
									initial_concentration,
									&initCVector[e]);
				// generate the next term of the linear combination:
				//		printf("%d: calc_residuals 4\n", myrank); fflush(stdout);
				//	puts("p5-1");fflush(stdout);
				//	mfem[e]->fprintinitparams(stdout, myrank); fflush(stdout);
				mfem[e]->skipEvents = true;

				mfem[e]->run();

				//	puts("p5-2");fflush(stdout);

				// printf("%d: calc_residuals back from mfem run\n", myrank); fflush(stdout);
				// interpolate model function to the experimental data so
				// dimension 1 in A matches dimension of B:
				//	printf("meniscus A %g first radius %g\n"
				//		"meniscus B %g first radius %g\n",
				//		experiment[e].meniscus, experiment[e].radius[0],
				//		fem_data.meniscus, fem_data.radius[0]);
				//	fflush(stdout);

				//	printf("%d: calc_residuals 5 interpolate\n", myrank); fflush(stdout);
				mfem[e]->interpolate(&experiment[e], &fem_data[e]);

				// puts("p5-3");fflush(stdout);
				//	cerr << "p6\n";
				//	printf("%d: calc_residuals 6\n", myrank); fflush(stdout);
				for (j = 0; j < experiment[e].scan.size(); j++)
				{
					for (k = 0; k < experiment[e].radius.size(); k++)
					{
						// populate the A matrix for the NNLS routine with the
						// model function:
						nnls_a[count] = experiment[e].scan[j].conc[k];
						count++;
					}
				}
			} // for e
		} // for i
		//	cerr << "p7\n";
		//	printf("fit_tinoise %d fit_rinoise %d\n", fit_tinoise, fit_rinoise);

#if defined(SHOW_TIMING)
		gettimeofday(&end_tv, NULL);
		printf("mfem timing = %ld\n",
			   1000000l * (end_tv.tv_sec - start_tv.tv_sec) + end_tv.tv_usec -
			   start_tv.tv_usec);
#endif
		//	cout << tr("Calculating NNLS solution...");
		experiment = save_experiment;
		//	printf("%g %g %g\n",
		//	experiment.scan[0].conc[0],
		//	experiment.scan[0].conc[5],
		//	experiment.scan[5].conc[0]);

		if (fit_tinoise)
		{
			//		printf("solutes size %d\n", solutes.size());
			//		printf("points %d ti_noise_size %d ri_noise_size %d b_size %d a_size %d\n",
			//		total_points_size,
			//		ti_noise_size,
			//			ri_noise_size, b_size, a_size);
			//	for (e = 0; e < experiment.size(); e++) {
			//	printf("experiment %d scan size %d radius size %d\n",
			//		e, experiment[e].scan.size(), experiment[e].radius.size());
			//	}
			//	fflush(stdout);

			double *L = new double[b_size];	// this is Sum(concentration * Lamm) for the models after NNLS
			double *L_bars = new double[ti_noise_size * solutes.size()];	// an average for each distribution
			double *L_tildes = new double[ri_noise_size * solutes.size()];	// an average for each distribution

			double small_a[solutes.size() * solutes.size()];
			double small_b[solutes.size()];
			double small_x[solutes.size()];

			//	double (*nnls_a_pa)[solutes.size()][total_points_size] = (double (*)[solutes.size()][total_points_size])nnls_a;
			//	double (*L_tildes_pa)[solutes.size()][ri_noise_size] = (double (*)[solutes.size()][ri_noise_size])L_tildes;
			//	double (*L_bars_pa)[solutes.size()][ti_noise_size] = (double (*)[solutes.size()][ti_noise_size])L_bars;

			//	printf("nnls_a[0][0] %g nnls_a[1][0] %g\n", nnls_a[0], nnls_a[total_points_size]); fflush(stdout);
			//	printf("nnls_a_pa[0][0] %g nnls_a_pa[1][0] %g\n", (*nnls_a_pa)[0][0], (*nnls_a_pa)[1][0]); fflush(stdout);

			double *L_bar = new double[ti_noise_size];	// a concentration weighted average
			double *L_tilde = new double[ri_noise_size];
			unsigned int l;
			unsigned int countNZ;
			unsigned int countL;
			// unsigned int iterations = 0;
			double *new_ti_noise = new double[ti_noise_size];
			double *new_ri_noise = new double[ri_noise_size];

			double *a_bar = new double[ti_noise_size];
			double *a_tilde = new double[ri_noise_size];

			// start with no noise
			memset(new_ti_noise, 0, ti_noise_size * sizeof(double));
			memset(new_ri_noise, 0, ri_noise_size * sizeof(double));

			// compute a_bar, the average experiment signal at each radius
			// compute a_tilde, the average experiment signal at each time
			unsigned int i_offset, j_offset, exp_offset;

			memset(a_tilde, 0, ri_noise_size * sizeof(double));
			if (fit_rinoise)
			{
				i_offset = 0;
				for (e = 0; e < experiment.size(); e++)
				{
					for (i = 0; i < experiment[e].scan.size(); i++)
					{
						for (j = 0; j < experiment[e].radius.size(); j++)
						{
							a_tilde[i + i_offset] += experiment[e].scan[i].conc[j];
						}
						a_tilde[i + i_offset] /= experiment[e].radius.size();
					}
					i_offset += experiment[e].scan.size();
				}
			}

			memset(a_bar, 0, ti_noise_size * sizeof(double));
			i_offset = j_offset = 0;
			for (e = 0; e < experiment.size(); e++)
			{
				for (j = 0; j < experiment[e].radius.size(); j++)
				{
					for (i = 0; i < experiment[e].scan.size(); i++)
					{
						// a_bar[j] += experiment[e].scan[i].conc[j];
						a_bar[j + j_offset] += experiment[e].scan[i].conc[j] - a_tilde[i + i_offset];
					}
					a_bar[j + j_offset] /= experiment[e].scan.size();
				}
				j_offset += experiment[e].radius.size();
				i_offset += experiment[e].scan.size();
			}
			//	for (j = 0; j < 10; j++) {
			//	printf(" %g", a_bar[j]);
			//	}
			//	puts("");

			// compute the new nnls_b vector including noise
			count = 0;

			// compute L_tildes, the average model signal at each radius
			memset(L_tildes, 0, ri_noise_size * sizeof(double) * solutes.size());

			if (fit_rinoise)
			{
				exp_offset = i_offset = j_offset = 0;
				for (e = 0; e < experiment.size(); e++)
				{
					for (l = 0; l < solutes.size(); l++)
					{
						for (i = 0; i < experiment[e].scan.size(); i++)
						{
							for (j = 0; j < experiment[e].radius.size(); j++)
							{
								L_tildes[l * ri_noise_size + i + i_offset] +=
									nnls_a[l * total_points_size + exp_offset + i * experiment[e].radius.size() + j];
							}
							L_tildes[l * ri_noise_size + i + i_offset] /= experiment[e].radius.size();
						}
					}
					i_offset += experiment[e].scan.size();
					exp_offset += experiment[e].scan.size() * experiment[e].radius.size();
				}
			}

			//	printf("L_tildes y0 %g\n", L_tildes[0]); fflush(stdout);
			//	printf("L_tildes y1 %g\n", L_tildes[1 * ri_noise_size]); fflush(stdout);
			//	printf("L_tildes x0 %g\n", (*L_tildes_pa)[0][0]); fflush(stdout);
			//	printf("L_tildes x1 %g\n", (*L_tildes_pa)[1][0]); fflush(stdout);

			i_offset = 0;
			for (e = 0; e < experiment.size(); e++)
			{
				for (l = 0; l < 3; l++)
				{
					//		printf("L_tildes for eq %d solute %d i_ofs %d :", e, l, i_offset);
					//		fflush(stdout);
					for (i = 0; i < 10; i++)
					{
						//		printf(" %g", L_tildes[l * ri_noise_size + i + i_offset]);
						//		fflush(stdout);
					}
					//		puts("");
					//		fflush(stdout);
				}
				i_offset += experiment[e].scan.size();
			}

			// compute L_bars, the average for each equation
			//	puts("ti nnls step 1 create L_bar");

			memset(L_bars, 0, ti_noise_size * solutes.size() * sizeof(double));
			exp_offset = i_offset = j_offset = 0;
			for (e = 0; e < experiment.size(); e++)
			{
				//	printf("exp %u %u %u %u\n", e, exp_offset, j_offset, i_offset); fflush(stdout);
				for (l = 0; l < solutes.size(); l++)
				{
					//		printf("l %d\n", l); fflush(stdout);
					for (j = 0; j < experiment[e].radius.size(); j++)
					{
						//		printf("j %d\n", j); fflush(stdout);
						for (i = 0; i < experiment[e].scan.size(); i++)
						{
							//		printf("1 %g\n",(*L_bars_pa)[l][j + j_offset]); fflush(stdout);
							//		printf("2 %g\n", (*nnls_a_pa)[l][exp_offset + i * experiment[e].radius.size() + j]); fflush(stdout);
							//		printf("3 %g\n", (*L_tildes_pa)[l][i + i_offset]); fflush(stdout);
							L_bars[l * ti_noise_size + j + j_offset] +=
								nnls_a[l * total_points_size + exp_offset + i * experiment[e].radius.size() + j] -
								L_tildes[l * ri_noise_size + i + i_offset];
							//		if (l == 0 && j == 0) {
							//			printf("lji %d %d %d %g\n", l, j, i, L_bars[l * ti_noise_size + j + j_offset]); fflush(stdout);
							//}
						}
						L_bars[l * ti_noise_size + j + j_offset] /= experiment[e].scan.size();
					}
				}
				exp_offset += experiment[e].scan.size() * experiment[e].radius.size();
				j_offset += experiment[e].radius.size();
				i_offset += experiment[e].scan.size();
			}

			//	for (j = 0; j < 10; j++) {
			//	printf(" %g", L_bars[j]); fflush(stdout);
			//	}
			//	puts("");
			//	fflush(stdout);

			//	puts("ti nnls step 2 create the nnls matrix & rhs");
			// unncessary?
			{
				unsigned int k;
				double residual;
				// setup small_a, small_b for the alternate nnls
				memset(small_a, 0,
					   solutes.size() * solutes.size() * sizeof(double));
				memset(small_b, 0, solutes.size() * sizeof(double));
				memset(small_x, 0, solutes.size() * sizeof(double));


#if defined(SHOW_TIMING)

				gettimeofday(&start_tv, NULL);
#endif

				exp_offset = i_offset = j_offset = 0;
				for (e = 0; e < experiment.size(); e++)
				{
					for (l = 0; l < solutes.size(); l++)
					{
						// printf("l = %d of %d\r", l, solutes.size());
						// fflush(stdout);
						for (j = 0; j < experiment[e].radius.size(); j++)
						{
							for (i = 0; i < experiment[e].scan.size(); i++)
							{
								small_b[l] += (experiment[e].scan[i].conc[j] - a_bar[j + j_offset]) *
											  (nnls_a[l * total_points_size + exp_offset + i * experiment[e].radius.size() + j] -
											   L_bars[l * ti_noise_size + j + j_offset]);
								for (k = 0; k < solutes.size(); k++)
								{
									small_a[k * solutes.size() + l] +=
										(nnls_a[k * total_points_size + exp_offset + i * experiment[e].radius.size() + j] -
										 L_bars[k * ti_noise_size + j + j_offset]) *
										(nnls_a[l * total_points_size + exp_offset + i * experiment[e].radius.size() + j] -
										 L_bars[l * ti_noise_size + j + j_offset]);
								}
							}
						}
						//		str.sprintf(tr
						//				("Working on Term %d of %d\r"),
						//				l + 1, solutes.size());
						//		cout << str;
						//		fflush(stdout);
					}
					exp_offset += experiment[e].scan.size() * experiment[e].radius.size();
					//??
					j_offset += experiment[e].radius.size();
					i_offset += experiment[e].scan.size();
				}

				//		for (j = 0; j < 10; j++) {
				//		printf(" %g ", small_a[j]);
				//		}
				//		puts("");
				//		for (j = 0; j < 10; j++) {
				//		printf(" %g ", small_b[j]);
				//		}
				//		puts("");


#if defined(SHOW_TIMING)
				gettimeofday(&end_tv, NULL);
				//		printf("nnls prep time = %ld\n",
				//		1000000l * (end_tv.tv_sec - start_tv.tv_sec) +
				//		end_tv.tv_usec - start_tv.tv_usec);
#endif
				// unncessary?
				{
					double cks1 = 0e0, cks2 = 0e0;
					for (i = 0; i < solutes.size(); i++)
					{
						cks1 += small_b[i];
					}
					for (i = 0; i < solutes.size() * solutes.size(); i++)
					{
						cks2 += small_a[i];
					}
					//		printf("cks %g %g\n", cks1, cks2);
				}
				//		puts("ti nnls step 3 run nnls");
				{
					long myrss = getrss(0);
					if (myrss > maxrss)
					{
						maxrss = myrss;
					}
				}
				result =
					nnls(small_a, solutes.size(), solutes.size(),
						 solutes.size(), small_b, small_x, &residual, NULL,
						 NULL, NULL);
				memcpy(nnls_x, small_x, solutes.size() * sizeof(double));
			}
			//	puts("ti nnls step 4 compute ti & ri noise");

			// print out original nnls concentrations & the new concentrations

			// compute L the sum of the equations
			memset(L, 0, b_size * sizeof(double));
			// we ignore the fit_baseline stuff
			count = 0;
			countNZ = 0;
			for (l = 0; l < solutes.size(); l++)
			{
				if (nnls_x[l] > 0)
				{
					countNZ++;
					countL = 0;
					// printf("l %d count %d l * b_size %d\n", l, count, l *
					// b_size);
					exp_offset = i_offset = j_offset = 0;
					for (e = 0; e < experiment.size(); e++)
					{
						for (i = 0; i < experiment[e].scan.size(); i++)
						{
							for (j = 0; j < experiment[e].radius.size(); j++)
							{
								L[countL] +=
									nnls_x[l] * nnls_a[l * total_points_size + exp_offset + i * experiment[e].radius.size() +
													   j];
								countL++;
								count++;
							}
						}
						exp_offset += experiment[e].scan.size() * experiment[e].radius.size();
					}
				}
				else
				{
					count += b_size;
				}
			}

			if (countNZ == 0)
			{
				fprintf(stderr, "countNZ is zero!\n");
				//		exit(-1);
			}
			// now L contains the best fit sum of L equations

			// compute L_tilde, the average model signal at each radius
			memset(L_tilde, 0, ri_noise_size * sizeof(double));
			if (fit_rinoise)
			{
				count = 0;
				exp_offset = i_offset = j_offset = 0;
				for (e = 0; e < experiment.size(); e++)
				{
					for (i = 0; i < experiment[e].scan.size(); i++)
					{
						for (j = 0; j < experiment[e].radius.size(); j++)
						{
							L_tilde[i + i_offset] += L[exp_offset + i * experiment[e].radius.size() + j];
							count++;
						}
						L_tilde[i + i_offset] /= experiment[e].radius.size();
					}
					i_offset += experiment[e].scan.size();
					exp_offset += experiment[e].scan.size() * experiment[e].radius.size();
				}
			}

			// compute L_bar, the average model signal at each radius
			memset(L_bar, 0, ti_noise_size * sizeof(double));
			exp_offset = i_offset = j_offset = 0;
			for (e = 0; e < experiment.size(); e++)
			{
				for (j = 0; j < experiment[e].radius.size(); j++)
				{
					for (i = 0; i < experiment[e].scan.size(); i++)
					{
						L_bar[j + j_offset] += L[exp_offset + i * experiment[e].radius.size() + j] - L_tilde[i + i_offset];
					}
					L_bar[j + j_offset] /= experiment[e].scan.size();
				}
				i_offset += experiment[e].scan.size();
				j_offset += experiment[e].radius.size();
				exp_offset += experiment[e].scan.size() * experiment[e].radius.size();
			}

			// compute ti_noise
			//	printf("new ti noise");
			for (i = 0; i < ti_noise_size; i++)
			{
				new_ti_noise[i] = a_bar[i] - L_bar[i];
				//		if (i < 20) {
				//		printf("a_bar %g - L_bar %g = %g\n", a_bar[i], L_bar[i], new_ti_noise[i]);
				//		}
			}

			//	puts("");
			// unncessary?
			{
				double cks = 0e0;
				for (i = 0; i < ti_noise_size; i++)
				{
					cks += new_ti_noise[i];
				}
				//		printf("sum of b(i) = %g\n", cks);
			}

			// compute ri_noise, this is not correct!
			//	printf("new ri noise");
			for (i = 0; i < ri_noise_size; i++)
			{
				new_ri_noise[i] = a_tilde[i] - L_tilde[i];
				//		if (i < 10) {
				//		printf(" %g", new_ri_noise[i]);
				//		}
			}
			//	puts("");
			// unncessary?
			{
				double cks = 0e0;
				for (i = 0; i < ri_noise_size; i++)
				{
					cks += new_ri_noise[i];
				}
				//		printf("sum of beta(i) = %g\n", cks);
			}

			delete[]L_bars;
			delete[]a_bar;
			delete[]a_tilde;
			delete[]L;
			delete[]L_bar;
			delete[]L_tilde;
			delete[]L_tildes;
			// copy new_ti_noise to ti_noise
			ti_noise.clear();
			for (i = 0; i < ti_noise_size; i++)
			{
				ti_noise.push_back(new_ti_noise[i]);
			}
			//	printf("ti noise");
			//	for (i = 0; i < 10; i++) {
			//		printf(" %g", ti_noise[i]);
			//	}
			//	puts("");
			// copy new_ri_noise to ri_noise
			ri_noise.clear();
			for (i = 0; i < ri_noise_size; i++)
			{
				ri_noise.push_back(new_ri_noise[i]);
			}
			//	puts("ti nnls end");
			// unncessary?
			for (e = 0; e < experiment.size(); e++)
			{
				unsigned int k;
				for (j = 0; j < experiment[e].scan.size(); j++)
				{
					for (k = 0; k < experiment[e].radius.size(); k++)
					{
						residuals[e].scan[j].conc[k] = 0;
					}
				}
			}
			delete[] new_ti_noise;
			delete[] new_ri_noise;
		}
		else
		{
			if (fit_rinoise)
			{
				//		printf("solutes size %d\n", solutes.size());
				//		printf("points %d ti_noise_size %d ri_noise_size %d b_size %d a_size %d\n",
				//		total_points_size,
				//		ti_noise_size,
				//			ri_noise_size, b_size, a_size);
				//	for (e = 0; e < experiment.size(); e++) {
				//	printf("experiment %d scan size %d radius size %d\n",
				//		e, experiment[e].scan.size(), experiment[e].radius.size());
				//	}
				//	fflush(stdout);

				double *L = new double[b_size];	// this is Sum(concentration * Lamm) for the models after NNLS
				double *L_tildes = new double[ri_noise_size * solutes.size()];	// an average for each distribution

				double small_a[solutes.size() * solutes.size()];
				double small_b[solutes.size()];
				double small_x[solutes.size()];

				double *L_tilde = new double[ri_noise_size];
				unsigned int l;
				unsigned int countNZ;
				unsigned int countL;
				// unsigned int iterations = 0;
				double *new_ri_noise = new double[ri_noise_size];

				double *a_tilde = new double[ri_noise_size];

				// start with no noise
				memset(new_ri_noise, 0, ri_noise_size * sizeof(double));

				// compute a_tilde, the average experiment signal at each time
				unsigned int i_offset, j_offset, exp_offset;

				memset(a_tilde, 0, ri_noise_size * sizeof(double));
				i_offset = 0;
				for (e = 0; e < experiment.size(); e++)
				{
					for (i = 0; i < experiment[e].scan.size(); i++)
					{
						for (j = 0; j < experiment[e].radius.size(); j++)
						{
							a_tilde[i + i_offset] += experiment[e].scan[i].conc[j];
						}
						a_tilde[i + i_offset] /= experiment[e].radius.size();
					}
					i_offset += experiment[e].scan.size();
				}

				// compute the new nnls_b vector including noise
				count = 0;

				// compute L_tildes, the average model signal at each radius
				memset(L_tildes, 0, ri_noise_size * sizeof(double) * solutes.size());

				exp_offset = i_offset = j_offset = 0;
				for (e = 0; e < experiment.size(); e++)
				{
					for (l = 0; l < solutes.size(); l++)
					{
						for (i = 0; i < experiment[e].scan.size(); i++)
						{
							for (j = 0; j < experiment[e].radius.size(); j++)
							{
								L_tildes[l * ri_noise_size + i + i_offset] +=
									nnls_a[l * total_points_size + exp_offset + i * experiment[e].radius.size() + j];
							}
							L_tildes[l * ri_noise_size + i + i_offset] /= experiment[e].radius.size();
						}
					}
					i_offset += experiment[e].scan.size();
					exp_offset += experiment[e].scan.size() * experiment[e].radius.size();
				}

				//	printf("L_tildes y0 %g\n", L_tildes[0]); fflush(stdout);
				//	printf("L_tildes y1 %g\n", L_tildes[1 * ri_noise_size]); fflush(stdout);
				//	printf("L_tildes x0 %g\n", (*L_tildes_pa)[0][0]); fflush(stdout);
				//	printf("L_tildes x1 %g\n", (*L_tildes_pa)[1][0]); fflush(stdout);

				i_offset = 0;
				for (e = 0; e < experiment.size(); e++)
				{
					for (l = 0; l < 3; l++)
					{
						//		printf("L_tildes for eq %d solute %d i_ofs %d :", e, l, i_offset);
						//		fflush(stdout);
						for (i = 0; i < 10; i++)
						{
							//		printf(" %g", L_tildes[l * ri_noise_size + i + i_offset]);
							//		fflush(stdout);
						}
						//		puts("");
						//		fflush(stdout);
					}
					i_offset += experiment[e].scan.size();
				}

				//	puts("ti nnls step 2 create the nnls matrix & rhs");
				// unncessary?
				{
					unsigned int k;
					double residual;
					// setup small_a, small_b for the alternate nnls
					memset(small_a, 0,
						   solutes.size() * solutes.size() * sizeof(double));
					memset(small_b, 0, solutes.size() * sizeof(double));
					memset(small_x, 0, solutes.size() * sizeof(double));


#if defined(SHOW_TIMING)

					gettimeofday(&start_tv, NULL);
#endif

					exp_offset = i_offset = j_offset = 0;
					for (e = 0; e < experiment.size(); e++)
					{
						for (l = 0; l < solutes.size(); l++)
						{
							// printf("l = %d of %d\r", l, solutes.size());
							// fflush(stdout);
							for (j = 0; j < experiment[e].radius.size(); j++)
							{
								for (i = 0; i < experiment[e].scan.size(); i++)
								{
									small_b[l] += (experiment[e].scan[i].conc[j] - a_tilde[i + i_offset]) *
												  (nnls_a[l * total_points_size + exp_offset + i * experiment[e].radius.size() + j] -
												   L_tildes[l * ri_noise_size + i + i_offset]);
									for (k = 0; k < solutes.size(); k++)
									{
										small_a[k * solutes.size() + l] +=
											(nnls_a[k * total_points_size + exp_offset + i * experiment[e].radius.size() + j] -
											 L_tildes[k * ri_noise_size + i + i_offset]) *
											(nnls_a[l * total_points_size + exp_offset + i * experiment[e].radius.size() + j] -
											 L_tildes[l * ri_noise_size + i + i_offset]);
									}
								}
							}
							//		str.sprintf(tr
							//				("Working on Term %d of %d\r"),
							//				l + 1, solutes.size());
							//		cout << str;
							//		fflush(stdout);
						}
						exp_offset += experiment[e].scan.size() * experiment[e].radius.size();
						//??
						j_offset += experiment[e].radius.size();
						i_offset += experiment[e].scan.size();
					}

					//		for (j = 0; j < 10; j++) {
					//		printf(" %g ", small_a[j]);
					//		}
					//		puts("");
					//		for (j = 0; j < 10; j++) {
					//		printf(" %g ", small_b[j]);
					//		}
					//		puts("");


#if defined(SHOW_TIMING)
					gettimeofday(&end_tv, NULL);
					//		printf("nnls prep time = %ld\n",
					//		1000000l * (end_tv.tv_sec - start_tv.tv_sec) +
					//		end_tv.tv_usec - start_tv.tv_usec);
#endif
					// unncessary?
					{
						double cks1 = 0e0, cks2 = 0e0;
						for (i = 0; i < solutes.size(); i++)
						{
							cks1 += small_b[i];
						}
						for (i = 0; i < solutes.size() * solutes.size(); i++)
						{
							cks2 += small_a[i];
						}
						//		printf("cks %g %g\n", cks1, cks2);
					}
					//		puts("ti nnls step 3 run nnls");
					{
						long myrss = getrss(0);
						if (myrss > maxrss)
						{
							maxrss = myrss;
						}
					}
					result =
						nnls(small_a, solutes.size(), solutes.size(),
							 solutes.size(), small_b, small_x, &residual, NULL,
							 NULL, NULL);
					memcpy(nnls_x, small_x, solutes.size() * sizeof(double));
				}
				//	puts("ti nnls step 4 compute ti & ri noise");

				// print out original nnls concentrations & the new concentrations

				// compute L the sum of the equations
				memset(L, 0, b_size * sizeof(double));
				// we ignore the fit_baseline stuff
				count = 0;
				countNZ = 0;
				for (l = 0; l < solutes.size(); l++)
				{
					if (nnls_x[l] > 0)
					{
						countNZ++;
						countL = 0;
						// printf("l %d count %d l * b_size %d\n", l, count, l *
						// b_size);
						exp_offset = i_offset = j_offset = 0;
						for (e = 0; e < experiment.size(); e++)
						{
							for (i = 0; i < experiment[e].scan.size(); i++)
							{
								for (j = 0; j < experiment[e].radius.size(); j++)
								{
									L[countL] +=
										nnls_x[l] * nnls_a[l * total_points_size + exp_offset + i * experiment[e].radius.size() +
														   j];
									countL++;
									count++;
								}
							}
							exp_offset += experiment[e].scan.size() * experiment[e].radius.size();
						}
					}
					else
					{
						count += b_size;
					}
				}

				if (countNZ == 0)
				{
					fprintf(stderr, "countNZ is zero!\n");
					//		exit(-1);
				}
				// now L contains the best fit sum of L equations

				// compute L_tilde, the average model signal at each radius
				memset(L_tilde, 0, ri_noise_size * sizeof(double));
				count = 0;
				exp_offset = i_offset = j_offset = 0;
				for (e = 0; e < experiment.size(); e++)
				{
					for (i = 0; i < experiment[e].scan.size(); i++)
					{
						for (j = 0; j < experiment[e].radius.size(); j++)
						{
							L_tilde[i + i_offset] += L[exp_offset + i * experiment[e].radius.size() + j];
							count++;
						}
						L_tilde[i + i_offset] /= experiment[e].radius.size();
					}
					i_offset += experiment[e].scan.size();
					exp_offset += experiment[e].scan.size() * experiment[e].radius.size();
				}

				// compute ri_noise, (this is not correct???)
				//	printf("new ri noise");
				for (i = 0; i < ri_noise_size; i++)
				{
					new_ri_noise[i] = a_tilde[i] - L_tilde[i];
					//		if (i < 10) {
					//		printf(" %g", new_ri_noise[i]);
					//		}
				}
				//	puts("");
				// unncessary?
				{
					double cks = 0e0;
					for (i = 0; i < ri_noise_size; i++)
					{
						cks += new_ri_noise[i];
					}
					//		printf("sum of beta(i) = %g\n", cks);
				}

				delete[]a_tilde;
				delete[]L;
				delete[]L_tilde;
				delete[]L_tildes;
				// copy new_ri_noise to ri_noise
				ri_noise.clear();
				for (i = 0; i < ri_noise_size; i++)
				{
					ri_noise.push_back(new_ri_noise[i]);
				}
				//	puts("ti nnls end");
				// unncessary?
				for (e = 0; e < experiment.size(); e++)
				{
					unsigned int k;
					for (j = 0; j < experiment[e].scan.size(); j++)
					{
						for (k = 0; k < experiment[e].radius.size(); k++)
						{
							residuals[e].scan[j].conc[k] = 0;
						}
					}
				}
				delete[] new_ri_noise;
				// zero ti_noise
				ti_noise.clear();
				for (i = 0; i < ti_noise_size; i++)
				{
					ti_noise.push_back(0);
				}
			}
			else
			{
				// no ti or ri noise

				//	cout << "old nnls stuff\n";
				//	printf("nnls_b %.12g %.12g %.12g %.12g\n", nnls_b[0], nnls_b[1], nnls_b[300], nnls_b[1000]);
				//	fflush(stdout);
				{
					long myrss = getrss(0);
					if (myrss > maxrss)
					{
						maxrss = myrss;
					}
				}
				result = nnls(nnls_a,
							  total_points_size,
							  total_points_size,
							  solutes.size(),
							  nnls_b, nnls_x, &nnls_rnorm, nnls_wp, nnls_zzp,
							  nnls_indexp);
				// zero ti_noise
				ti_noise.clear();
				for (i = 0; i < ti_noise_size; i++)
				{
					ti_noise.push_back(0);
				}
				// no ri_noise for now
				ri_noise.clear();
				for (i = 0; i < ri_noise_size; i++)
				{
					ri_noise.push_back(0);
				}
				//	printf("nnls norm %g\n", nnls_rnorm);
			}
		}
		//	cout << tr("Calculating Residuals...\n");
		//	printf("residuals %d %d\n", residuals.radius.size(), residuals.scan.size());
		for (e = 0; e < experiment.size(); e++)
		{
			unsigned int k;
			for (j = 0; j < experiment[e].scan.size(); j++)
			{
				for (k = 0; k < experiment[e].radius.size(); k++)
				{
					residuals[e].scan[j].conc[k] = 0;
				}
			}
		}

		for (i = 0; i < solutes.size(); i++)
		{
			//	frequency.push_back(nnls_x[i]);
			if (nnls_x[i] != 0.0)
			{
				for (e = 0; e < experiment.size(); e++)
				{
					//	printf("exp %d solute %d %g %g %g %g\n", e, i, solutes[i].s / experiment[e].s20w_correction, solutes[i].k,
					//		(R * (experiment[e].avg_temperature + K0)) /
					//		(AVOGADRO * solutes[i].k
					//		* 6.0 * experiment[e].viscosity * 0.01 * M_PI *
					//		pow((9.0 * (solutes[i].s / experiment[e].s20w_correction) * solutes[i].k *
					//			experiment[e].vbar * experiment[e].viscosity * 0.01) /
					//			(2.0 * (1.0 - experiment[e].vbar * experiment[e].density)), 0.5)),
					//
					//		nnls_x[i]);
					//	cerr << "pe1a0\n";
					//	clear_data(&fem_data[e]);
					//		clear_data(&fem_data);
					//	cerr << "pe1a1\n";
					for (j = 0; j < experiment[e].scan.size(); j++)
					{
						for (k = 0; k < experiment[e].radius.size(); k++)
						{
							// reset concentration to zero:
							experiment[e].scan[j].conc[k] = 0.0;
						}
					}
					//	cerr << "pe1a2\n";
					double D_20w = (R * K20) /
								(AVOGADRO * 18 * M_PI * pow(solutes[i].k * VISC_20W, 3.0/2.0) *
								pow((solutes[i].s * experiment[e].vbar20)/(2.0 * (1.0 - experiment[e].vbar20 * DENS_20W)), 0.5));
					double D_tb = D_20w/experiment[e].D20w_correction;

					mfem[e]->set_params(100, solutes[i].s / experiment[e].s20w_correction, D_tb,
										(double) experiment[e].rpm,
										experiment[e].scan[experiment[e].scan.size() - 1].time,
										experiment[e].meniscus,
										experiment[e].bottom, initial_concentration,
										&initCVector[e]);
					mfem[e]->skipEvents = true;
					mfem[e]->run();
					//		mfem[e]->fprintparams(stdout);

					// interpolate model function to the experimental data so
					// dimension 1 in A matches dimension of B:
					mfem[e]->interpolate(&experiment[e], &fem_data[e]);
					double cks4 = 0e0;
					for (j = 0; j < experiment[e].scan.size(); j++)
					{
						for (k = 0; k < experiment[e].radius.size(); k++)
						{
							residuals[e].scan[j].conc[k] +=
								nnls_x[i] * experiment[e].scan[j].conc[k];
							//	printf("resid %d %d %g\n", j, k, residuals[e].scan[j].conc[k]); fflush(stdout);
							cks4 += residuals[e].scan[j].conc[k];
						}
					}
					//		printf("e %d nnls used %d cks %g\n", e, i, cks4);
				} // for e
				//	cerr << "pe1a\n";
			} // if (nnls_x[i] != 0)
		} // for i
		double rmsd = 0.0;
		double rmsds[experiment.size()];
		// unncessary?
		unsigned int ti_noise_offset = 0, ri_noise_offset = 0;
		for (e = 0; e < experiment.size(); e++)
		{
			//	cerr << "pe1\n";
			double cks1 = 0e0, cks2 = 0e0, cks3 = 0e0;
			rmsds[e] = 0;
			for (j = 0; j < experiment[e].scan.size(); j++)
			{
				for (k = 0; k < experiment[e].radius.size(); k++)
				{
					/*		printf("save_e %g -ti_noise %g -ri_noise %g -resid %g\n",
					save_experiment[e].scan[j].conc[k],
					ti_noise[k + ti_noise_offset],
					ri_noise[j+ri_noise_offset],
					residuals[e].scan[j].conc[k]); fflush(stdout);
					*/
					residuals[e].scan[j].conc[k] =
						save_experiment[e].scan[j].conc[k] -
						residuals[e].scan[j].conc[k] - ti_noise[k + ti_noise_offset] - ri_noise[j+ri_noise_offset];
					cks1 += experiment[e].scan[j].conc[k];
					cks2 += residuals[e].scan[j].conc[k];
					cks3 += ti_noise[k + ti_noise_offset];
					// cout << "C[" << j << "][" << k << "]: " <<
					//		residuals[e].scan[j].conc[k] << endl; fflush(stdout);
					rmsd +=
						residuals[e].scan[j].conc[k] * residuals[e].scan[j].conc[k];
					rmsds[e] +=
						residuals[e].scan[j].conc[k] * residuals[e].scan[j].conc[k];
				}
			}
			ti_noise_offset += experiment[e].radius.size();
			ri_noise_offset += experiment[e].scan.size();

			//	cerr << "pe2\n";
			// printf("cks %g %g %g\n", cks1, cks2, cks3); fflush(stdout);
		}
		last_residuals = residuals;
		//	cerr << "pe3\n";
		rmsd /= total_points_size;
		float variance = rmsd;
		vector<double> variances;
		rmsd = pow((double) rmsd, 0.5);
		for (e = 0; e < experiment.size(); e++)
		{
			variances.push_back(rmsds[e] / (experiment[e].scan.size() * experiment[e].radius.size()));
			rmsds[e] = pow((double) rmsds[e], 0.5);
			//	printf("experiment %d variance %g\n", e, variances[e]);
		}


		//	cerr << "pe4\n";
		//	cout << str.
		//	sprintf(tr
		//		("Solution converged...\nRMSD: %8.6e,\nVariance: %8.6e\n"),
		//		rmsd, rmsd * rmsd);
		//	calc_20W_distros();
		//	cerr << "pe5\n";

		for (i = 0; i < save_solutes.size(); i++)
		{
			if (nnls_x[i] > 0 || return_all_solutes)
			{
				save_solutes[i].c = nnls_x[i];
				sv.solutes.push_back(save_solutes[i]);
			}
		}
		sv.variance = variance;
		sv.variances = variances;
		sv.ti_noise = ti_noise;
		sv.ri_noise = ri_noise;

		delete[]nnls_a;
		delete[]nnls_b;
		delete[]nnls_zzp;
		delete[]nnls_x;
		delete[]nnls_wp;
		delete[]nnls_indexp;
		for (i = 0; i < experiment.size(); i++)
		{
			delete mfem[i];
		}

#if defined(JOB_TIMING_CR)
		gettimeofday(&end_tv_cr, NULL);
# if defined(JOB_TIMING_CRMP)

		printf("crmp time %u %lu\n",
			   use_size,
			   1000000l * (end_tv_cr.tv_sec - start_tv_cr.tv_sec) + end_tv_cr.tv_usec -
			   start_tv_cr.tv_usec);
	}
# else
		printf("cr time %lu\n",
			   1000000l * (end_tv_cr.tv_sec - start_tv_cr.tv_sec) + end_tv_cr.tv_usec -
			   start_tv_cr.tv_usec);
# endif

	fflush(stdout);
#endif

	return sv;

	// return(residual);
}


#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#define DEV_ENCODE(M,m) ( \
 ( (M&0xfff) << 8) | ( (m&0xfff00) << 12) | (m&0xff) \
)

#include <asm/param.h> /* HZ */
#include <asm/page.h> /* PAGE_SIZE */
#define NO_TTY_VALUE DEV_ENCODE(0,0)
#ifndef HZ
#warning HZ not defined, assuming it is 100
#define HZ 100
#endif

#ifndef PAGE_SIZE
#warning PAGE_SIZE not defined, assuming it is 4096
#define PAGE_SIZE 4096
#endif

long getrss(int pid)
{
	if (!pid)
	{
		pid = getpid();
	}
	char P_tty_text[16];
	char P_cmd[16];
	char P_state;
	int P_euid;
	int P_pid;
	int P_ppid, P_pgrp, P_session, P_tty_num, P_tpgid;
	unsigned long P_flags, P_min_flt, P_cmin_flt, P_maj_flt, P_cmaj_flt, P_utime, P_stime;
	long P_cutime, P_cstime, P_priority, P_nice, P_timeout, P_alarm;
	unsigned long P_start_time, P_vsize;
	long P_rss;
	unsigned long P_rss_rlim, P_start_code, P_end_code, P_start_stack, P_kstk_esp, P_kstk_eip;
	unsigned P_signal, P_blocked, P_sigignore, P_sigcatch;
	unsigned long P_wchan, P_nswap, P_cnswap;
	char buf[800]; /* about 40 fields, 64-bit decimal is about 20 chars */
	int num;
	int fd;
	char* tmp;
	struct stat sb; /* stat() used to get EUID */
	snprintf(buf, 32, "/proc/%d/stat", pid);
	if ( (fd = open(buf, O_RDONLY, 0) ) == -1 )
		return 0;
	num = read(fd, buf, sizeof buf - 1);
	fstat(fd, &sb);
	P_euid = sb.st_uid;
	close(fd);
	if (num<80)
		return 0;
	buf[num] = '\0';
	tmp = strrchr(buf, ')');	/* split into "PID (cmd" and "<rest>" */
	*tmp = '\0';				/* replace trailing ')' with NUL */
	/* parse these two strings separately, skipping the leading "(". */
	memset(P_cmd, 0, sizeof P_cmd);		/* clear */
	sscanf(buf, "%d (%15c", &P_pid, P_cmd); /* comm[16] in kernel */
	num = sscanf(tmp + 2,					/* skip space after ')' too */
				 "%c "
				 "%d %d %d %d %d "
				 "%lu %lu %lu %lu %lu %lu %lu "
				 "%ld %ld %ld %ld %ld %ld "
				 "%lu %lu "
				 "%ld "
				 "%lu %lu %lu %lu %lu %lu "
				 "%u %u %u %u " /* no use for RT signals */
				 "%lu %lu %lu",
				 &P_state,
				 &P_ppid, &P_pgrp, &P_session, &P_tty_num, &P_tpgid,
				 &P_flags, &P_min_flt, &P_cmin_flt, &P_maj_flt, &P_cmaj_flt, &P_utime, &P_stime,
				 &P_cutime, &P_cstime, &P_priority, &P_nice, &P_timeout, &P_alarm,
				 &P_start_time, &P_vsize,
				 &P_rss,
				 &P_rss_rlim, &P_start_code, &P_end_code, &P_start_stack, &P_kstk_esp, &P_kstk_eip,
				 &P_signal, &P_blocked, &P_sigignore, &P_sigcatch,
				 &P_wchan, &P_nswap, &P_cnswap
				);
	/*	fprintf(stderr, "stat2proc converted %d fields.\n",num); */
	P_vsize /= 1024;
	P_rss *= (PAGE_SIZE/1024);

	memcpy(P_tty_text, " ? ", 8);
	if (P_tty_num != NO_TTY_VALUE)
	{
		int tty_maj = (P_tty_num>>8)&0xfff;
		int tty_min = (P_tty_num&0xff) | ((P_tty_num>>12)&0xfff00);
		snprintf(P_tty_text, sizeof P_tty_text, "%3d,%-3d", tty_maj, tty_min);
	}

	if (num < 30)
		return 0;
	if (P_pid != pid)
		return 0;
	return P_rss;
}

QString getToken(QString *str, const QString &separator)
{
	int pos;
	QString token;
	pos = str->find(separator, 0, false);
	if (pos < 0)
	{
		if (str->length() > 0)
		{
			token = (*str);
			(*str) = "";
			return (token);
		}
		else
		{
			return((QString) "");
		}
	}
	while (pos == 0)
	{
		(*str) = str->mid(pos + 1, str->length());	
		pos = str->find(separator, 0, false);
		if (pos < 0)
		{
			if (str->length() > 0)
			{
				token = (*str);
				(*str) = "";
				return (token);
			}
			else
			{
				return((QString) "");
			}
		}
	}
	token = str->left(pos);
	(*str) = str->mid(pos + 1, str->length());
	return(token);
}

bool readRotorInfo(vector <struct rotorInfo> *rotor_info_vector)
{
	struct rotorInfo temp_rotor_info;
	QString str1, str2;
	bool ok;
	char *US = getenv("ULTRASCAN");
	QFile rotor_file(QString("%1").arg(US) + "/etc/rotor.dat");
	if( rotor_file.open(IO_ReadOnly))
	{
		QTextStream ts(&rotor_file);
		while (!ts.atEnd())
		{
			str1 = ts.readLine();	// process line by line
			str2 = getToken(&str1, " "); // get the first token of the line
			temp_rotor_info.serial_number = str2.toInt(&ok, 10);
			if (ok)  // if the first token is not an integer, we got a comment and we'll skip this line
			{
				temp_rotor_info.type = getToken(&str1, " ");
				for (int i=0; i<5; i++)
				{
					str2 = getToken(&str1, " ");
					temp_rotor_info.coefficient[i] = str2.toFloat(&ok);
				}
				(*rotor_info_vector).push_back(temp_rotor_info);
			}
		}
		rotor_file.close();
		return (true);
	}
	else
	{
		return(false);
	}
}

bool readCenterpieceInfo(vector <struct centerpieceInfo> *cp_info_vector)
{
	struct centerpieceInfo temp_cp_info;
	QString str1, str2;
	bool ok;
	char *US = getenv("ULTRASCAN");
	QFile cp_file(QString("%1").arg(US) + "/etc/centerpiece.dat");
	if(cp_file.open(IO_ReadOnly))
	{
		QTextStream ts(&cp_file);
		while(!ts.atEnd())
		{
			str1 = ts.readLine();	// process line by line
			str2 = getToken(&str1, " "); // get the first token of the line
			temp_cp_info.serial_number = str2.toInt(&ok, 10);
			if (ok)  // if the first token is not an integer, we got a comment and we'll skip this line
			{
				temp_cp_info.material = getToken(&str1, " ");
				str2 = getToken(&str1, " ");
				temp_cp_info.channels = str2.toInt(&ok, 10);
				for (unsigned int i=0; i<temp_cp_info.channels; i++)
				{
					str2 = getToken(&str1, " ");
					temp_cp_info.bottom_position[i] = str2.toFloat(&ok);
				}
				str2 = getToken(&str1, " ");
				temp_cp_info.sector = str2.toInt(&ok, 10);
				str2 = getToken(&str1, " ");
				temp_cp_info.pathlength = str2.toFloat(&ok);
				str2 = getToken(&str1, " ");
				temp_cp_info.angle = str2.toFloat(&ok);
				str2 = getToken(&str1, " ");
				temp_cp_info.width = str2.toFloat(&ok);
				(*cp_info_vector).push_back(temp_cp_info);
			}
		}
		cp_file.close();
		return(true);
	}
	else
	{
		return(false);
	}
}
