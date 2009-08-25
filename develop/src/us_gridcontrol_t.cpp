#include "../include/us_gridcontrol_t.h"
#include "string.h"
#include <sys/file.h>

// NOTICE : GA_SC can only handle 1 experiment at this time!

// these is the 's' exclusion zone
#define EXCLUDE_S(x) (x >= -0.1e-13 && x <= 0.1e-13)

// this constructor is used for non-gui calls from the command line. It
// reads an input file with all the details assigned from a web interface
US_GridControl_T::US_GridControl_T(const QString &control_file,
                                   const QString &gridopt, const QString &system_name, QObject *p, const char *name) : QObject(p, name)
{
   cout.precision(8);
   cerr.precision(8);
   USglobal = new US_Config();
   gcfile = control_file;
   this->gridopt = gridopt;
   cerr << "gridopt: " << gridopt << endl;
   this->system_name = system_name;
   cerr << "system: " << system_name << endl;
   cerr << "non-gui\n";
   timestamp = QDateTime::currentDateTime();
   timestamp_string = timestamp.toString("yyMMddhhmmss");
   SA2D_Params.ff0_min = 1.0;
   SA2D_Params.ff0_max = 4.0;
   SA2D_Params.ff0_resolution = 10;
   SA2D_Params.s_min = 1.0;
   SA2D_Params.s_max = 10.0;
   SA2D_Params.s_resolution = 10;
   SA2D_Params.regularization = 0.0;
   SA2D_Params.uniform_grid_repetition = 1;
   SA2D_Params.meniscus_range = 0.0;
   SA2D_Params.meniscus_gridpoints = 0;
   SA2D_Params.fit_meniscus = false;
   SA2D_Params.max_iterations = 3;
   SA2D_Params.monte_carlo = 0;
   SA2D_Params.use_iterative = false;
   GA_Params.genes = 500;
   GA_Params.demes = 10;
   GA_Params.generations = 100;
   GA_Params.crossover = 50;
   GA_Params.initial_solutes = 5;
   GA_Params.mutation = 50;
   GA_Params.plague = 0;
   GA_Params.elitism = 2;
   GA_Params.random_seed = 0;
   GA_Params.regularization = 0.95;
   GA_Params.meniscus_range = 0.0;
   GA_Params.fit_meniscus = false;
   GA_Params.solute.clear();
   GA_Params.monte_carlo = 0;
   meniscus_range = 0.0;
   fit_meniscus = false;
   use_iterative = false;
   max_iterations = 3;
   cerr << "l1\n";
   struct bucket temp_bucket;
   QString file_info, str;
   unsigned int count;
   Control_Params.int_params.clear();
   Control_Params.float_params.clear();
   simparams_extra.clear();
   simulation_parameters_vec.clear();
   total_points = 0;
   cp_list.clear();
   rotor_list.clear();
   GA_Params.solute.clear();
   if (!readCenterpieceInfo(&cp_list))
   {
      cerr << "UltraScan Fatal Error: There was a problem opening the\n"
         "centerpiece database file:\n\n"
         + USglobal->config_list.system_dir + "/etc/centerpiece.dat\n\n"
         "Please install the centerpiece database file\n"
         "before proceeding.\n\n";
      exit(-1);
   }
   if (!readRotorInfo(&rotor_list))
   {
      cerr << "UltraScan Fatal Error:", "There was a problem opening the\n"
         "rotor database file:\n\n"
         + USglobal->config_list.system_dir + "/etc/rotor.dat\n\n"
         "Please install the rotor database file\n"
         "before proceeding.\n\n";
      exit(-2);
   }
   cerr << "l2\n";
   cerr << control_file << endl;
   QFile f(control_file);
   cerr << "l2a\n";
   QString fn;
   if(f.open(IO_ReadOnly))
   {
      cerr << "l2b\n";
      QTextStream ts(&f);
      db_login.driver = "QMYSQL3";
      ts >> analysis_type;
      if(analysis_type == "2DSA-MW-Constraint")
      {
         analysis_type = "2DSA_MW";
      }
      cerr << "Analysis Type: " << analysis_type << endl;
      ts >> db_login.user;
      cerr << "Database user: " << db_login.user << endl;
      ts >> db_login.password;
      cerr << "Database password: " << db_login.password << endl;
      ts >> db_login.database;
      cerr << "Database name: " << db_login.database << endl;
      ts >> db_login.host;
      cerr << "Database host: " << db_login.host << endl;
      ts >> count;
      cerr << "Number of experiments: " << count << endl;
      bool baseline_flag = false;
      for (unsigned  int i=0; i<count; i++)
      {
         dataIO = new US_Data_IO(&run_inf, baseline_flag);
         ts >> file_info; // run ID
         int position = file_info.find(".veloc.", 0, false);
         file_info = file_info.left(position);
         cerr << "Run ID for experiment " << i+1 << ": " << file_info << endl;
         //USglobal paths need to get initialized for both "this" object and for US_Data_IO
         ts >> dataIO->USglobal->config_list.result_dir; // place where data was extracted to
         USglobal->config_list.result_dir = dataIO->USglobal->config_list.result_dir;
         cerr << "Path for experiment " << i+1 << ": " << USglobal->config_list.result_dir << endl;
         fn = USglobal->config_list.result_dir + "/" + file_info + ".us.v";
         ts >> selected_cell;
         selected_cell --;
         cerr << "Cell for experiment " << i+1 << ": " << selected_cell << endl;
         ts >> selected_lambda;
         selected_lambda --;
         cerr << "Wavelength for experiment " << i+1 << ": " << selected_lambda << endl;
         ts >> selected_channel;
         selected_channel --;
         cerr << "Channel for experiment " << i+1 << ": " << selected_channel << endl;
         cerr << "About to load file: " << fn << endl;
         int result = dataIO->load_run(fn, 1, has_data, &cp_list);
         cerr << "loading resulted in: " << result << endl;
         if (result < 0)
         {
            str = tr("Error: the selected run could not be loaded:\n\n"
                     "selected Experiment: " + fn + "\n"
                     "Run aborted with exit code -3");
            cerr << str << endl;
            exit(-3);
         }
         if (analysis_type == "GA_SC")
         {
            QString constraint_file;
            dataIO->assign_simparams(&simulation_parameters, selected_cell, selected_lambda, selected_channel);
            ts >> constraint_file;
            constraint_file = USglobal->config_list.result_dir + "/" + constraint_file;

            US_FemGlobal us_femglobal;
            printf("read constraints %d\n", us_femglobal.read_constraints(&model_system, &model_system_constraints, constraint_file));
            simulation_parameters.simpoints = model_system_constraints.simpoints;
            simulation_parameters.mesh = model_system_constraints.mesh;
            simulation_parameters.moving_grid = model_system_constraints.moving_grid;
            // NOTICE: the centerpiece type assigned in US_Data_IO::assign_simparams() sets the flag for band forming centerpieces.
            // if the flag is true, then the band_volume set below should be used. The band_volume needs to be set by the user.
            simulation_parameters.band_volume = model_system_constraints.band_volume;

            constraints_file_name = USglobal->config_list.result_dir + "/" + file_info + ".tmp.constraints";
            simulation_parameters_file_name = USglobal->config_list.result_dir + "/" + file_info + ".tmp.simulation_parameters";

            QFile f1(constraints_file_name);
            QFile f2(simulation_parameters_file_name);
            char *US = getenv("ULTRASCAN");
            char lockfile[strlen(US) + strlen("/tigre.lock") + 2];
            sprintf(lockfile, "%s/tigre.lock", US);
            int lfd = open(lockfile, O_RDONLY);
            flock(lfd, LOCK_EX);
            int increment = 0;
            while (f1.open(IO_ReadOnly) || f2.open(IO_ReadOnly))
            {
               f1.close();
               f2.close();
               increment++;
               constraints_file_name = USglobal->config_list.result_dir + "/" + file_info + QString(".tmp-%1.constraints").arg(increment);
               simulation_parameters_file_name = USglobal->config_list.result_dir + "/" + file_info + QString(".tmp-%1.simulation_parameters").arg(increment);
               f1.setName(constraints_file_name);
               f2.setName(simulation_parameters_file_name);
            }
            f1.close();
            f2.close();
            flock(lfd, LOCK_UN);
            close(lfd);

            us_femglobal.write_constraints(&model_system, &model_system_constraints, constraints_file_name);
            us_femglobal.write_simulationParameters(&simulation_parameters, simulation_parameters_file_name);
         }
         if (analysis_type == "2DSA_RA" ||
             analysis_type == "2DSA_MW_RA" ||
             analysis_type == "GA_RA" ||
             analysis_type == "GA_MW_RA")
         {
            dataIO->assign_simparams(&simulation_parameters, selected_cell, selected_lambda, selected_channel);
            simulation_parameters_file_name = USglobal->config_list.result_dir + "/" + file_info + ".tmp.simulation_parameters";
            cout << "simulation_parameters_file_name: " << simulation_parameters_file_name << endl;
            SimparamsExtra tmp_extra;
            tmp_extra.simulation_parameters_file_name = simulation_parameters_file_name;
            simparams_extra.push_back(tmp_extra);
            cerr << QString("assign loop simparams_extra[%1].simulation_parameters_file_name = %2\n").
               arg(simparams_extra.size() - 1).
               arg(simparams_extra[simparams_extra.size() - 1].simulation_parameters_file_name);
            simulation_parameters_vec.push_back(simulation_parameters);
         }

         points = run_inf.points[selected_cell][selected_lambda][selected_channel];
         cerr << " there are " << points << " in this dataset...\n";
         total_points += points * run_inf.scans[selected_cell][selected_lambda];
         result = dataIO->load_veloc_scan(&channel_data, selected_cell, selected_lambda, selected_channel);
         if(result < 0)
         {
            str.sprintf(tr("Error: the selected scan could not be loaded:\n\n"
                           "selected Experiment: " + fn + "\n"
                           "selected cell: %d\n"
                           "selected lambda: %d\n"
                           "selected channel: %d\n"
                           "Run aborted with exit code -4"),
                        selected_cell + 1, selected_lambda + 1, selected_channel + 1);
            cerr << str << endl;
            exit(-4);
         }
         cerr << "Result: " << result << endl;
         cerr << "successfully loaded " << fn << ", cell: " << selected_cell+1 << ", lambda: " << selected_lambda+1 << ", channel: " << selected_channel << "\n";
         dataIO->load_hydrodynamics(db_login, &hydro_inf);
         corr_inf.density = hydro_inf.Density[selected_cell][selected_lambda];
         cerr << "corr_inf.density: " << corr_inf.density << endl;
         corr_inf.viscosity = hydro_inf.Viscosity[selected_cell][selected_lambda];
         cerr << "corr_inf.viscosity: " << corr_inf.viscosity << endl;
         corr_inf.vbar = hydro_inf.Vbar[selected_cell][selected_lambda][selected_channel];
         cerr << "corr_inf.vbar: " << corr_inf.vbar << endl;
         corr_inf.vbar20 = hydro_inf.Vbar20[selected_cell][selected_lambda][selected_channel];
         cerr << "corr_inf.vbar20: " << corr_inf.vbar20 << endl;
         corr_inf.temperature = run_inf.avg_temperature;
         cerr << "corr_inf.temperature: " << corr_inf.temperature << endl;
         dataIO->calc_correction(&corr_inf);
         add_experiment();
         delete dataIO;
      }
      // ------------- read experiments loop
      if (analysis_type == "GA")
      {
         ts >> GA_Params.demes;
         cerr << "demes:" << GA_Params.demes << endl;
         ts >> GA_Params.generations;
         cerr << "generations:" << GA_Params.generations << endl;
         ts >> GA_Params.crossover;
         cerr << "crossover:" << GA_Params.crossover << endl;
         ts >> GA_Params.mutation;
         cerr << "mutation:" << GA_Params.mutation << endl;
         ts >> GA_Params.plague;
         cerr << "plague:" << GA_Params.plague << endl;
         ts >> GA_Params.elitism;
         cerr << "elitism:" << GA_Params.elitism << endl;
         ts >> GA_Params.migration_rate;
         cerr << "Migration Rate:" << GA_Params.migration_rate << endl;
         ts >> GA_Params.genes;
         cerr << "genes:" << GA_Params.genes << endl;
         ts >> GA_Params.initial_solutes;
         cerr << "solutes:" << GA_Params.initial_solutes << endl;
         printf("solutes:%d\n", GA_Params.initial_solutes);
         ts >> GA_Params.random_seed;
         cerr << "seed:" << GA_Params.random_seed << endl;
         ts >> GA_Params.monte_carlo;
         cerr << "Monte Carlo Iterations:" << GA_Params.monte_carlo << endl;
         printf("monte carlo:%d\n", GA_Params.monte_carlo);
         ts >> GA_Params.regularization;
         cerr << "regularization:" << GA_Params.regularization << endl;
         regularization = GA_Params.regularization;
         printf("regularization:%f\n", GA_Params.regularization);
         for (unsigned int i=0; i<GA_Params.initial_solutes; i++)
         {
            ts >>  temp_bucket.s;
            ts >>  temp_bucket.s_min;
            ts >>  temp_bucket.s_max;
            ts >>  temp_bucket.ff0;
            ts >>  temp_bucket.ff0_min;
            ts >>  temp_bucket.ff0_max;
            GA_Params.solute.push_back(temp_bucket);
         }
         ts >> count;
         cerr << "Fit meniscus?: " << count << endl;
         GA_Params.fit_meniscus = count;
         fit_meniscus = count;
         printf("fit_meniscus: %d\n", fit_meniscus);
         ts >> GA_Params.meniscus_range;
         cerr << "Meniscus range: " << GA_Params.meniscus_range << endl;
         meniscus_range = GA_Params.meniscus_range;
         ts >> count;
         cerr << "Fit ti noise?: " << count << endl;
         fit_tinoise = count;
         printf("fit_ti: %d\n", fit_rinoise);
         ts >> count;
         cerr << "Fit ri noise?: " << count << endl;
         fit_rinoise = count;
         printf("fit_ri: %d\n", fit_rinoise);
         ts >> email;
         cerr << "Email: " << email << endl;
         printf("email: %s\n", email.ascii());
         {
            analysis_defined = true;
            data_loaded = true;
            write_experiment();
         }
      }
      if (analysis_type == "GA_RA")
      {
         ts >> GA_Params.demes;
         cerr << "demes:" << GA_Params.demes << endl;
         ts >> GA_Params.generations;
         cerr << "generations:" << GA_Params.generations << endl;
         ts >> GA_Params.crossover;
         cerr << "crossover:" << GA_Params.crossover << endl;
         ts >> GA_Params.mutation;
         cerr << "mutation:" << GA_Params.mutation << endl;
         ts >> GA_Params.plague;
         cerr << "plague:" << GA_Params.plague << endl;
         ts >> GA_Params.elitism;
         cerr << "elitism:" << GA_Params.elitism << endl;
         ts >> GA_Params.migration_rate;
         cerr << "Migration Rate:" << GA_Params.migration_rate << endl;
         ts >> GA_Params.genes;
         cerr << "genes:" << GA_Params.genes << endl;
         ts >> GA_Params.initial_solutes;
         cerr << "solutes:" << GA_Params.initial_solutes << endl;
         printf("solutes:%d\n", GA_Params.initial_solutes);
         ts >> GA_Params.random_seed;
         cerr << "seed:" << GA_Params.random_seed << endl;
         ts >> GA_Params.monte_carlo;
         cerr << "Monte Carlo Iterations:" << GA_Params.monte_carlo << endl;
         printf("monte carlo:%d\n", GA_Params.monte_carlo);
         ts >> GA_Params.regularization;
         cerr << "regularization:" << GA_Params.regularization << endl;
         regularization = GA_Params.regularization;
         printf("regularization:%f\n", GA_Params.regularization);
         for (unsigned int i=0; i<GA_Params.initial_solutes; i++)
         {
            ts >>  temp_bucket.s;
            ts >>  temp_bucket.s_min;
            ts >>  temp_bucket.s_max;
            ts >>  temp_bucket.ff0;
            ts >>  temp_bucket.ff0_min;
            ts >>  temp_bucket.ff0_max;
            GA_Params.solute.push_back(temp_bucket);
         }
         ts >> count;
         cerr << "Fit meniscus?: " << count << endl;
         GA_Params.fit_meniscus = count;
         fit_meniscus = count;
         printf("fit_meniscus: %d\n", fit_meniscus);
         ts >> GA_Params.meniscus_range;
         cerr << "Meniscus range: " << GA_Params.meniscus_range << endl;
         meniscus_range = GA_Params.meniscus_range;
         ts >> count;
         cerr << "Fit ti noise?: " << count << endl;
         fit_tinoise = count;
         printf("fit_ti: %d\n", fit_rinoise);
         ts >> count;
         cerr << "Fit ri noise?: " << count << endl;
         fit_rinoise = count;
         printf("fit_ri: %d\n", fit_rinoise);
         ts >> email;
         cerr << "Email: " << email << endl;
         printf("email: %s\n", email.ascii());
         for (unsigned int i = 0; i < experiment.size(); i++)
         {
            cerr << QString("Experiment %1 simparameters:\n").arg(i);
            ts >> simparams_extra[i].simpoints;
            cerr << "Simpoints:" << simparams_extra[i].simpoints << endl;
            ts >> simparams_extra[i].band_volume;
            cerr << "Band volume:" << simparams_extra[i].band_volume << endl;
            ts >> simparams_extra[i].radial_grid;
            cerr << "Radial grid:" << simparams_extra[i].radial_grid << endl;
            ts >> simparams_extra[i].moving_grid;
            cerr << "Moving grid:" << simparams_extra[i].moving_grid << endl;
            {
               simulation_parameters_file_name = simparams_extra[i].simulation_parameters_file_name;
               QFile f(simulation_parameters_file_name);
               char *US = getenv("ULTRASCAN");
               char lockfile[strlen(US) + strlen("/tigre.lock") + 2];
               sprintf(lockfile, "%s/tigre.lock", US);
               int lfd = open(lockfile, O_RDONLY);
               flock(lfd, LOCK_EX);
               int increment = 0;
               while (f.open(IO_ReadOnly))
               {
                  f.close();
                  increment++;
                  simulation_parameters_file_name = simparams_extra[i].simulation_parameters_file_name + QString("-%1").arg(increment);
                  f.setName(simulation_parameters_file_name);
               }
               f.close();
               flock(lfd, LOCK_UN);
               close(lfd);
               simparams_extra[i].simulation_parameters_file_name = simulation_parameters_file_name;
               US_FemGlobal us_femglobal;
               simulation_parameters_vec[i].simpoints = simparams_extra[i].simpoints;
               simulation_parameters_vec[i].band_volume = simparams_extra[i].band_volume;
               simulation_parameters_vec[i].mesh = simparams_extra[i].radial_grid;
               simulation_parameters_vec[i].moving_grid = simparams_extra[i].moving_grid;
               fprintf(stderr, "write simulation parameters returned %d <%s>\n",
                       us_femglobal.write_simulationParameters(&simulation_parameters_vec[i], simulation_parameters_file_name),
                       simulation_parameters_file_name.ascii());
            }
            { // read simulation parameters
               QFile fsimulation_parameters(simulation_parameters_file_name);
               if (!fsimulation_parameters.open(IO_ReadOnly))
               {
                  cerr << "Failed to open simulation_parameters file!\n";
                  exit(-4);
               }
               simulation_parameters_full_text.clear();
               QTextStream ts(&fsimulation_parameters);
               QString qs_tmp;
               while (!ts.atEnd())
               {
                  qs_tmp = ts.readLine();
                  printf("simu line: %s\n", qs_tmp.ascii());
                  simulation_parameters_full_text.push_back(qs_tmp);
               }
               fsimulation_parameters.close();
               simparams_extra[i].simulation_parameters_full_text = simulation_parameters_full_text;
            }
         }
         {
            analysis_defined = true;
            data_loaded = true;
            write_experiment();
         }
      }
      if (analysis_type == "GA_MW")
      {
         ts >> GA_Params.demes;
         cerr << "demes:" << GA_Params.demes << endl;
         ts >> GA_Params.generations;
         cerr << "generations:" << GA_Params.generations << endl;
         ts >> GA_Params.crossover;
         cerr << "crossover:" << GA_Params.crossover << endl;
         ts >> GA_Params.mutation;
         cerr << "mutation:" << GA_Params.mutation << endl;
         ts >> GA_Params.plague;
         cerr << "plague:" << GA_Params.plague << endl;
         ts >> GA_Params.elitism;
         cerr << "elitism:" << GA_Params.elitism << endl;
         ts >> GA_Params.migration_rate;
         cerr << "Migration Rate:" << GA_Params.migration_rate << endl;
         ts >> GA_Params.genes;
         cerr << "genes:" << GA_Params.genes << endl;
         ts >> GA_Params.largest_oligomer_string;
         cerr << "largest_oligomer:" << GA_Params.largest_oligomer_string << endl;
         GA_Params.largest_oligomer = strlen(GA_Params.largest_oligomer_string.ascii());
         printf("largest_oligomer_string: %s\n", GA_Params.largest_oligomer_string.ascii());
         printf("largest_oligomer:%d\n", GA_Params.largest_oligomer);
         ts >> GA_Params.random_seed;
         cerr << "seed:" << GA_Params.random_seed << endl;
         ts >> GA_Params.monte_carlo;
         cerr << "Monte Carlo Iterations:" << GA_Params.monte_carlo << endl;
         printf("monte carlo:%d\n", GA_Params.monte_carlo);
         ts >> GA_Params.regularization;
         cerr << "regularization:" << GA_Params.regularization << endl;
         regularization = GA_Params.regularization;
         printf("regularization:%f\n", GA_Params.regularization);

         ts >> GA_Params.mw_min;
         cerr << "mw_min:" << GA_Params.mw_min << endl;
         printf("mw_min:%e\n", GA_Params.mw_min);

         ts >> GA_Params.mw_max;
         cerr << "mw_max:" << GA_Params.mw_max << endl;
         printf("mw_max:%e\n", GA_Params.mw_max);

         ts >> GA_Params.ff0_min;
         cerr << "ff0_min:" << GA_Params.ff0_min << endl;
         printf("ff0_min:%e\n", GA_Params.ff0_min);

         ts >> GA_Params.ff0_max;
         cerr << "ff0_max:" << GA_Params.ff0_max << endl;
         printf("mw_max:%e\n", GA_Params.ff0_max);

         ts >> count;
         cerr << "Fit meniscus?: " << count << endl;
         GA_Params.fit_meniscus = count;
         fit_meniscus = count;
         printf("fit_meniscus: %d\n", fit_meniscus);
         ts >> GA_Params.meniscus_range;
         cerr << "Meniscus range: " << GA_Params.meniscus_range << endl;
         meniscus_range = GA_Params.meniscus_range;
         ts >> count;
         cerr << "Fit ti noise?: " << count << endl;
         fit_tinoise = count;
         printf("fit_ti: %d\n", fit_rinoise);
         ts >> count;
         cerr << "Fit ri noise?: " << count << endl;
         fit_rinoise = count;
         printf("fit_ri: %d\n", fit_rinoise);
         ts >> email;
         cerr << "Email: " << email << endl;
         printf("email: %s\n", email.ascii());
         {
            analysis_defined = true;
            data_loaded = true;
            write_experiment();
         }
      }
      if (analysis_type == "GA_MW_RA")
      {
         ts >> GA_Params.demes;
         cerr << "demes:" << GA_Params.demes << endl;
         ts >> GA_Params.generations;
         cerr << "generations:" << GA_Params.generations << endl;
         ts >> GA_Params.crossover;
         cerr << "crossover:" << GA_Params.crossover << endl;
         ts >> GA_Params.mutation;
         cerr << "mutation:" << GA_Params.mutation << endl;
         ts >> GA_Params.plague;
         cerr << "plague:" << GA_Params.plague << endl;
         ts >> GA_Params.elitism;
         cerr << "elitism:" << GA_Params.elitism << endl;
         ts >> GA_Params.migration_rate;
         cerr << "Migration Rate:" << GA_Params.migration_rate << endl;
         ts >> GA_Params.genes;
         cerr << "genes:" << GA_Params.genes << endl;
         ts >> GA_Params.largest_oligomer_string;
         cerr << "largest_oligomer:" << GA_Params.largest_oligomer_string << endl;
         GA_Params.largest_oligomer = strlen(GA_Params.largest_oligomer_string.ascii());
         printf("largest_oligomer_string: %s\n", GA_Params.largest_oligomer_string.ascii());
         printf("largest_oligomer:%d\n", GA_Params.largest_oligomer);
         ts >> GA_Params.random_seed;
         cerr << "seed:" << GA_Params.random_seed << endl;
         ts >> GA_Params.monte_carlo;
         cerr << "Monte Carlo Iterations:" << GA_Params.monte_carlo << endl;
         printf("monte carlo:%d\n", GA_Params.monte_carlo);
         ts >> GA_Params.regularization;
         cerr << "regularization:" << GA_Params.regularization << endl;
         regularization = GA_Params.regularization;
         printf("regularization:%f\n", GA_Params.regularization);

         ts >> GA_Params.mw_min;
         cerr << "mw_min:" << GA_Params.mw_min << endl;
         printf("mw_min:%e\n", GA_Params.mw_min);

         ts >> GA_Params.mw_max;
         cerr << "mw_max:" << GA_Params.mw_max << endl;
         printf("mw_max:%e\n", GA_Params.mw_max);

         ts >> GA_Params.ff0_min;
         cerr << "ff0_min:" << GA_Params.ff0_min << endl;
         printf("ff0_min:%e\n", GA_Params.ff0_min);

         ts >> GA_Params.ff0_max;
         cerr << "ff0_max:" << GA_Params.ff0_max << endl;
         printf("mw_max:%e\n", GA_Params.ff0_max);

         ts >> count;
         cerr << "Fit meniscus?: " << count << endl;
         GA_Params.fit_meniscus = count;
         fit_meniscus = count;
         printf("fit_meniscus: %d\n", fit_meniscus);
         ts >> GA_Params.meniscus_range;
         cerr << "Meniscus range: " << GA_Params.meniscus_range << endl;
         meniscus_range = GA_Params.meniscus_range;
         ts >> count;
         cerr << "Fit ti noise?: " << count << endl;
         fit_tinoise = count;
         printf("fit_ti: %d\n", fit_rinoise);
         ts >> count;
         cerr << "Fit ri noise?: " << count << endl;
         fit_rinoise = count;
         printf("fit_ri: %d\n", fit_rinoise);
         ts >> email;
         cerr << "Email: " << email << endl;
         printf("email: %s\n", email.ascii());
         for (unsigned int i = 0; i < experiment.size(); i++)
         {
            cerr << QString("Experiment %1 simparameters:\n").arg(i);
            ts >> simparams_extra[i].simpoints;
            cerr << "Simpoints:" << simparams_extra[i].simpoints << endl;
            ts >> simparams_extra[i].band_volume;
            cerr << "Band volume:" << simparams_extra[i].band_volume << endl;
            ts >> simparams_extra[i].radial_grid;
            cerr << "Radial grid:" << simparams_extra[i].radial_grid << endl;
            ts >> simparams_extra[i].moving_grid;
            cerr << "Moving grid:" << simparams_extra[i].moving_grid << endl;
            {
               simulation_parameters_file_name = simparams_extra[i].simulation_parameters_file_name;
               QFile f(simulation_parameters_file_name);
               char *US = getenv("ULTRASCAN");
               char lockfile[strlen(US) + strlen("/tigre.lock") + 2];
               sprintf(lockfile, "%s/tigre.lock", US);
               int lfd = open(lockfile, O_RDONLY);
               flock(lfd, LOCK_EX);
               int increment = 0;
               while (f.open(IO_ReadOnly))
               {
                  f.close();
                  increment++;
                  simulation_parameters_file_name = simparams_extra[i].simulation_parameters_file_name + QString("-%1").arg(increment);
                  f.setName(simulation_parameters_file_name);
               }
               f.close();
               flock(lfd, LOCK_UN);
               close(lfd);
               simparams_extra[i].simulation_parameters_file_name = simulation_parameters_file_name;
               US_FemGlobal us_femglobal;
               simulation_parameters_vec[i].simpoints = simparams_extra[i].simpoints;
               simulation_parameters_vec[i].band_volume = simparams_extra[i].band_volume;
               simulation_parameters_vec[i].mesh = simparams_extra[i].radial_grid;
               simulation_parameters_vec[i].moving_grid = simparams_extra[i].moving_grid;
               fprintf(stderr, "write simulation parameters returned %d <%s>\n",
                       us_femglobal.write_simulationParameters(&simulation_parameters_vec[i], simulation_parameters_file_name),
                       simulation_parameters_file_name.ascii());
            }
            { // read simulation parameters
               QFile fsimulation_parameters(simulation_parameters_file_name);
               if (!fsimulation_parameters.open(IO_ReadOnly))
               {
                  cerr << "Failed to open simulation_parameters file!\n";
                  exit(-4);
               }
               simulation_parameters_full_text.clear();
               QTextStream ts(&fsimulation_parameters);
               QString qs_tmp;
               while (!ts.atEnd())
               {
                  qs_tmp = ts.readLine();
                  printf("simu line: %s\n", qs_tmp.ascii());
                  simulation_parameters_full_text.push_back(qs_tmp);
               }
               fsimulation_parameters.close();
               simparams_extra[i].simulation_parameters_full_text = simulation_parameters_full_text;
            }
         }
         {
            analysis_defined = true;
            data_loaded = true;
            write_experiment();
         }
      }
      if (analysis_type == "GA_SC")
      {
         ts >> GA_Params.demes;
         cerr << "demes:" << GA_Params.demes << endl;
         ts >> GA_Params.generations;
         cerr << "generations:" << GA_Params.generations << endl;
         ts >> GA_Params.crossover;
         cerr << "crossover:" << GA_Params.crossover << endl;
         ts >> GA_Params.mutation;
         cerr << "mutation:" << GA_Params.mutation << endl;
         ts >> GA_Params.plague;
         cerr << "plague:" << GA_Params.plague << endl;
         ts >> GA_Params.elitism;
         cerr << "elitism:" << GA_Params.elitism << endl;
         ts >> GA_Params.migration_rate;
         cerr << "Migration Rate:" << GA_Params.migration_rate << endl;
         ts >> GA_Params.genes;
         cerr << "Poplulation:" << GA_Params.genes << endl;
         ts >> GA_Params.random_seed;
         cerr << "seed:" << GA_Params.random_seed << endl;
         ts >> GA_Params.monte_carlo;
         cerr << "Monte Carlo Iterations:" << GA_Params.monte_carlo << endl;
         printf("monte carlo:%d\n", GA_Params.monte_carlo);
         ts >> GA_Params.regularization;
         cerr << "regularization:" << GA_Params.regularization << endl;
         regularization = GA_Params.regularization;
         printf("regularization:%f\n", GA_Params.regularization);
         ts >> count;
         cerr << "Fit meniscus?: " << count << endl;
         GA_Params.fit_meniscus = count;
         fit_meniscus = count;
         printf("fit_meniscus: %d\n", fit_meniscus);
         ts >> GA_Params.meniscus_range;
         cerr << "Meniscus range: " << GA_Params.meniscus_range << endl;
         meniscus_range = GA_Params.meniscus_range;
         ts >> count;
         cerr << "Fit ti noise?: " << count << endl;
         fit_tinoise = count;
         printf("fit_ti: %d\n", fit_rinoise);
         ts >> count;
         cerr << "Fit ri noise?: " << count << endl;
         fit_rinoise = count;
         printf("fit_ri: %d\n", fit_rinoise);
         ts >> email;
         cerr << "Email: " << email << endl;

         { // read constraints
            QFile fconstraints(constraints_file_name);
            if (!fconstraints.open(IO_ReadOnly))
            {
               cerr << "Failed to open constraints file!\n";
               exit(-3);
            }
            constraints_full_text.clear();
            QTextStream ts(&fconstraints);
            QString qs_tmp;
            while(!ts.atEnd())
            {
               qs_tmp = ts.readLine();
               printf("constr line: %s\n", qs_tmp.ascii());
               constraints_full_text.push_back(qs_tmp);
            }
            fconstraints.close();
         }

         { // read simulation parameters
            QFile fsimulation_parameters(simulation_parameters_file_name);
            if (!fsimulation_parameters.open(IO_ReadOnly))
            {
               cerr << "Failed to open simulation_parameters file!\n";
               exit(-4);
            }
            simulation_parameters_full_text.clear();
            QTextStream ts(&fsimulation_parameters);
            QString qs_tmp;
            while(!ts.atEnd())
            {
               qs_tmp = ts.readLine();
               printf("simu line: %s\n", qs_tmp.ascii());
               simulation_parameters_full_text.push_back(qs_tmp);
            }
            fsimulation_parameters.close();
         }

         printf("email: %s\n", email.ascii());
         {
            analysis_defined = true;
            data_loaded = true;
            write_experiment();
         }
      }
      if (analysis_type == "SA2D" ||
          analysis_type == "2DSA")
      {
         ts >> SA2D_Params.ff0_min;
         cerr << "ff0 min: " << SA2D_Params.ff0_min << endl;
         ts >> SA2D_Params.ff0_max;
         cerr << "ff0 max: " << SA2D_Params.ff0_max << endl;
         ts >> SA2D_Params.ff0_resolution;
         cerr << "ff0 Resolution: " << SA2D_Params.ff0_resolution << endl;
         ts >> SA2D_Params.s_min;
         cerr << "s min: " << SA2D_Params.s_min << endl;
         ts >> SA2D_Params.s_max;
         cerr << "s max: " << SA2D_Params.s_max << endl;
         ts >> SA2D_Params.s_resolution;
         cerr << "s Resolution: " << SA2D_Params.s_resolution<< endl;
         ts >> SA2D_Params.regularization;
         regularization = SA2D_Params.regularization;
         cerr << "Regularization: " << SA2D_Params.regularization << endl;
         ts >> SA2D_Params.uniform_grid_repetition;
         cerr << "Grid Repetitions: " << SA2D_Params.uniform_grid_repetition << endl;
         ts >> count;
         cerr << "Fit meniscus?: " << count << endl;
         SA2D_Params.fit_meniscus = count;
         fit_meniscus = count;
         ts >> SA2D_Params.meniscus_range;
         cerr << "Meniscus range: " << SA2D_Params.meniscus_range << endl;
         meniscus_range = SA2D_Params.meniscus_range;
         ts >> SA2D_Params.meniscus_gridpoints;
         cerr << "Meniscus Gridpoints: " << SA2D_Params.meniscus_gridpoints << endl;
         meniscus_gridpoints = SA2D_Params.meniscus_gridpoints;
         ts >> count;
         cerr << "Fit ti noise?: " << count << endl;
         fit_tinoise = count;
         ts >> count;
         cerr << "Fit ri noise?: " << count << endl;
         fit_rinoise = count;
         ts >> email;
         cerr << "Email: " << email << endl;
         ts >> count;
         cerr << "Use iterative Method?: " << count << endl;
         SA2D_Params.use_iterative = count;
         use_iterative = count;
         ts >> SA2D_Params.max_iterations;
         cerr << "Max iterations: " << SA2D_Params.max_iterations << endl;
         max_iterations = SA2D_Params.max_iterations;
         ts >> SA2D_Params.monte_carlo;
         cerr << "Monte Carlo Iterations:" << SA2D_Params.monte_carlo << endl;

         cerr << "px\n";
         {
            analysis_defined = true;
            data_loaded = true;
            write_experiment();
         }
      }
      if (analysis_type == "2DSA_RA")
      {
         ts >> SA2D_Params.ff0_min;
         cerr << "ff0 min: " << SA2D_Params.ff0_min << endl;
         ts >> SA2D_Params.ff0_max;
         cerr << "ff0 max: " << SA2D_Params.ff0_max << endl;
         ts >> SA2D_Params.ff0_resolution;
         cerr << "ff0 Resolution: " << SA2D_Params.ff0_resolution << endl;
         ts >> SA2D_Params.s_min;
         cerr << "s min: " << SA2D_Params.s_min << endl;
         ts >> SA2D_Params.s_max;
         cerr << "s max: " << SA2D_Params.s_max << endl;
         ts >> SA2D_Params.s_resolution;
         cerr << "s Resolution: " << SA2D_Params.s_resolution<< endl;
         ts >> SA2D_Params.regularization;
         regularization = SA2D_Params.regularization;
         cerr << "Regularization: " << SA2D_Params.regularization << endl;
         ts >> SA2D_Params.uniform_grid_repetition;
         cerr << "Grid Repetitions: " << SA2D_Params.uniform_grid_repetition << endl;
         ts >> count;
         cerr << "Fit meniscus?: " << count << endl;
         SA2D_Params.fit_meniscus = count;
         fit_meniscus = count;
         ts >> SA2D_Params.meniscus_range;
         cerr << "Meniscus range: " << SA2D_Params.meniscus_range << endl;
         meniscus_range = SA2D_Params.meniscus_range;
         ts >> SA2D_Params.meniscus_gridpoints;
         cerr << "Meniscus Gridpoints: " << SA2D_Params.meniscus_gridpoints << endl;
         meniscus_gridpoints = SA2D_Params.meniscus_gridpoints;
         ts >> count;
         cerr << "Fit ti noise?: " << count << endl;
         fit_tinoise = count;
         ts >> count;
         cerr << "Fit ri noise?: " << count << endl;
         fit_rinoise = count;
         ts >> email;
         cerr << "Email: " << email << endl;
         ts >> count;
         cerr << "Use iterative Method?: " << count << endl;
         SA2D_Params.use_iterative = count;
         use_iterative = count;
         ts >> SA2D_Params.max_iterations;
         cerr << "Max iterations: " << SA2D_Params.max_iterations << endl;
         max_iterations = SA2D_Params.max_iterations;
         ts >> SA2D_Params.monte_carlo;
         cerr << "Monte Carlo Iterations:" << SA2D_Params.monte_carlo << endl;
         for (unsigned int i = 0; i < experiment.size(); i++)
         {
            cerr << QString("Experiment %1 simparameters:\n").arg(i);
            ts >> simparams_extra[i].simpoints;
            cerr << "Simpoints:" << simparams_extra[i].simpoints << endl;
            ts >> simparams_extra[i].band_volume;
            cerr << "Band volume:" << simparams_extra[i].band_volume << endl;
            ts >> simparams_extra[i].radial_grid;
            cerr << "Radial grid:" << simparams_extra[i].radial_grid << endl;
            ts >> simparams_extra[i].moving_grid;
            cerr << "Moving grid:" << simparams_extra[i].moving_grid << endl;
            {
               simulation_parameters_file_name = simparams_extra[i].simulation_parameters_file_name;
               QFile f(simulation_parameters_file_name);
               char *US = getenv("ULTRASCAN");
               char lockfile[strlen(US) + strlen("/tigre.lock") + 2];
               sprintf(lockfile, "%s/tigre.lock", US);
               int lfd = open(lockfile, O_RDONLY);
               flock(lfd, LOCK_EX);
               int increment = 0;
               while (f.open(IO_ReadOnly))
               {
                  f.close();
                  increment++;
                  simulation_parameters_file_name = simparams_extra[i].simulation_parameters_file_name + QString("-%1").arg(increment);
                  f.setName(simulation_parameters_file_name);
               }
               f.close();
               flock(lfd, LOCK_UN);
               close(lfd);
               simparams_extra[i].simulation_parameters_file_name = simulation_parameters_file_name;
               US_FemGlobal us_femglobal;
               simulation_parameters_vec[i].simpoints = simparams_extra[i].simpoints;
               simulation_parameters_vec[i].band_volume = simparams_extra[i].band_volume;
               simulation_parameters_vec[i].mesh = simparams_extra[i].radial_grid;
               simulation_parameters_vec[i].moving_grid = simparams_extra[i].moving_grid;
               fprintf(stderr, "write simulation parameters returned %d <%s>\n",
                       us_femglobal.write_simulationParameters(&simulation_parameters_vec[i], simulation_parameters_file_name),
                       simulation_parameters_file_name.ascii());
            }
            { // read simulation parameters
               fprintf(stderr, "opening simulation_parameters file %s:\n",
                       simulation_parameters_file_name.ascii());
               QFile fsimulation_parameters(simulation_parameters_file_name);
               if (!fsimulation_parameters.open(IO_ReadOnly))
               {
                  cerr << "Failed to open simulation_parameters file!\n";
                  exit(-4);
               }
               simulation_parameters_full_text.clear();
               QTextStream ts(&fsimulation_parameters);
               QString qs_tmp;
               while (!ts.atEnd())
               {
                  qs_tmp = ts.readLine();
                  printf("simu line: %s\n", qs_tmp.ascii());
                  simulation_parameters_full_text.push_back(qs_tmp);
               }
               fsimulation_parameters.close();
               simparams_extra[i].simulation_parameters_full_text = simulation_parameters_full_text;
            }
         }
         {
            analysis_defined = true;
            data_loaded = true;
            write_experiment();
         }
      }
      if (analysis_type == "2DSA_MW")
      {
         cerr << "2DSA_MW" << endl;
         ts >> SA2D_Params.ff0_min;
         cerr << "ff0 min: " << SA2D_Params.ff0_min << endl;
         ts >> SA2D_Params.ff0_max;
         cerr << "ff0 max: " << SA2D_Params.ff0_max << endl;
         ts >> SA2D_Params.ff0_resolution;
         cerr << "ff0 Resolution: " << SA2D_Params.ff0_resolution << endl;
         ts >> SA2D_Params.mw_min;
         cerr << "mw min: " << SA2D_Params.mw_min << endl;
         ts >> SA2D_Params.mw_max;
         cerr << "mw max: " << SA2D_Params.mw_max << endl;
         ts >> SA2D_Params.grid_resolution;
         cerr << "grid Resolution: " << SA2D_Params.grid_resolution<< endl;
         ts >> SA2D_Params.regularization;
         regularization = SA2D_Params.regularization;
         cerr << "Regularization: " << SA2D_Params.regularization << endl;
         ts >> SA2D_Params.uniform_grid_repetition;
         cerr << "Grid Repetitions: " << SA2D_Params.uniform_grid_repetition << endl;
         ts >> count;
         cerr << "Fit meniscus?: " << count << endl;
         SA2D_Params.fit_meniscus = count;
         fit_meniscus = count;
         ts >> SA2D_Params.meniscus_range;
         cerr << "Meniscus range: " << SA2D_Params.meniscus_range << endl;
         meniscus_range = SA2D_Params.meniscus_range;
         ts >> SA2D_Params.meniscus_gridpoints;
         cerr << "Meniscus Gridpoints: " << SA2D_Params.meniscus_gridpoints << endl;
         meniscus_gridpoints = SA2D_Params.meniscus_gridpoints;
         ts >> count;
         cerr << "Fit ti noise?: " << count << endl;
         fit_tinoise = count;
         ts >> count;
         cerr << "Fit ri noise?: " << count << endl;
         fit_rinoise = count;
         ts >> email;
         cerr << "Email: " << email << endl;
         ts >> count;
         cerr << "Use iterative Method?: " << count << endl;
         SA2D_Params.use_iterative = count;
         use_iterative = count;
         ts >> SA2D_Params.max_iterations;
         cerr << "Max iterations: " << SA2D_Params.max_iterations << endl;
         max_iterations = SA2D_Params.max_iterations;
         ts >> SA2D_Params.monte_carlo;
         cerr << "Monte Carlo Iterations:" << SA2D_Params.monte_carlo << endl;
         ts >> SA2D_Params.max_mer_string;
         SA2D_Params.max_mer = strlen(SA2D_Params.max_mer_string.ascii());
         cerr << "Max mer string:" << SA2D_Params.max_mer_string << endl;
         cerr << "Max mer:" << SA2D_Params.max_mer << endl;
         cerr << "px\n";
         {
            analysis_defined = true;
            data_loaded = true;
            write_experiment();
         }
      }
      if (analysis_type == "2DSA_MW_RA")
      {
         cerr << "2DSA_MW_RA" << endl;
         ts >> SA2D_Params.ff0_min;
         cerr << "ff0 min: " << SA2D_Params.ff0_min << endl;
         ts >> SA2D_Params.ff0_max;
         cerr << "ff0 max: " << SA2D_Params.ff0_max << endl;
         ts >> SA2D_Params.ff0_resolution;
         cerr << "ff0 Resolution: " << SA2D_Params.ff0_resolution << endl;
         ts >> SA2D_Params.mw_min;
         cerr << "mw min: " << SA2D_Params.mw_min << endl;
         ts >> SA2D_Params.mw_max;
         cerr << "mw max: " << SA2D_Params.mw_max << endl;
         ts >> SA2D_Params.grid_resolution;
         cerr << "grid Resolution: " << SA2D_Params.grid_resolution<< endl;
         ts >> SA2D_Params.regularization;
         regularization = SA2D_Params.regularization;
         cerr << "Regularization: " << SA2D_Params.regularization << endl;
         ts >> SA2D_Params.uniform_grid_repetition;
         cerr << "Grid Repetitions: " << SA2D_Params.uniform_grid_repetition << endl;
         ts >> count;
         cerr << "Fit meniscus?: " << count << endl;
         SA2D_Params.fit_meniscus = count;
         fit_meniscus = count;
         ts >> SA2D_Params.meniscus_range;
         cerr << "Meniscus range: " << SA2D_Params.meniscus_range << endl;
         meniscus_range = SA2D_Params.meniscus_range;
         ts >> SA2D_Params.meniscus_gridpoints;
         cerr << "Meniscus Gridpoints: " << SA2D_Params.meniscus_gridpoints << endl;
         meniscus_gridpoints = SA2D_Params.meniscus_gridpoints;
         ts >> count;
         cerr << "Fit ti noise?: " << count << endl;
         fit_tinoise = count;
         ts >> count;
         cerr << "Fit ri noise?: " << count << endl;
         fit_rinoise = count;
         ts >> email;
         cerr << "Email: " << email << endl;
         ts >> count;
         cerr << "Use iterative Method?: " << count << endl;
         SA2D_Params.use_iterative = count;
         use_iterative = count;
         ts >> SA2D_Params.max_iterations;
         cerr << "Max iterations: " << SA2D_Params.max_iterations << endl;
         max_iterations = SA2D_Params.max_iterations;
         ts >> SA2D_Params.monte_carlo;
         cerr << "Monte Carlo Iterations:" << SA2D_Params.monte_carlo << endl;
         ts >> SA2D_Params.max_mer_string;
         SA2D_Params.max_mer = strlen(SA2D_Params.max_mer_string.ascii());
         cerr << "Max mer string:" << SA2D_Params.max_mer_string << endl;
         cerr << "Max mer:" << SA2D_Params.max_mer << endl;
         for (unsigned int i = 0; i < experiment.size(); i++)
         {
            cerr << QString("Experiment %1 simparameters:\n").arg(i);
            ts >> simparams_extra[i].simpoints;
            cerr << "Simpoints:" << simparams_extra[i].simpoints << endl;
            ts >> simparams_extra[i].band_volume;
            cerr << "Band volume:" << simparams_extra[i].band_volume << endl;
            ts >> simparams_extra[i].radial_grid;
            cerr << "Radial grid:" << simparams_extra[i].radial_grid << endl;
            ts >> simparams_extra[i].moving_grid;
            cerr << "Moving grid:" << simparams_extra[i].moving_grid << endl;
            {
               simulation_parameters_file_name = simparams_extra[i].simulation_parameters_file_name;
               QFile f(simulation_parameters_file_name);
               char *US = getenv("ULTRASCAN");
               char lockfile[strlen(US) + strlen("/tigre.lock") + 2];
               sprintf(lockfile, "%s/tigre.lock", US);
               int lfd = open(lockfile, O_RDONLY);
               flock(lfd, LOCK_EX);
               int increment = 0;
               while (f.open(IO_ReadOnly))
               {
                  f.close();
                  increment++;
                  simulation_parameters_file_name = simparams_extra[i].simulation_parameters_file_name + QString("-%1").arg(increment);
                  f.setName(simulation_parameters_file_name);
               }
               f.close();
               flock(lfd, LOCK_UN);
               close(lfd);
               simparams_extra[i].simulation_parameters_file_name = simulation_parameters_file_name;
               US_FemGlobal us_femglobal;
               simulation_parameters_vec[i].simpoints = simparams_extra[i].simpoints;
               simulation_parameters_vec[i].band_volume = simparams_extra[i].band_volume;
               simulation_parameters_vec[i].mesh = simparams_extra[i].radial_grid;
               simulation_parameters_vec[i].moving_grid = simparams_extra[i].moving_grid;
               fprintf(stderr, "write simulation parameters returned %d <%s>\n",
                       us_femglobal.write_simulationParameters(&simulation_parameters_vec[i], simulation_parameters_file_name),
                       simulation_parameters_file_name.ascii());
            }
            { // read simulation parameters
               QFile fsimulation_parameters(simulation_parameters_file_name);
               if (!fsimulation_parameters.open(IO_ReadOnly))
               {
                  cerr << "Failed to open simulation_parameters file!\n";
                  exit(-4);
               }
               simulation_parameters_full_text.clear();
               QTextStream ts(&fsimulation_parameters);
               QString qs_tmp;
               while (!ts.atEnd())
               {
                  qs_tmp = ts.readLine();
                  printf("simu line: %s\n", qs_tmp.ascii());
                  simulation_parameters_full_text.push_back(qs_tmp);
               }
               fsimulation_parameters.close();
               simparams_extra[i].simulation_parameters_full_text = simulation_parameters_full_text;
            }
         }
         cerr << "px\n";
         {
            analysis_defined = true;
            data_loaded = true;
            write_experiment();
         }
      }
   }
   else
   {
      cerr << "Error: cannot read input file " << control_file << endl;
      cerr << "Run aborted ...\n\n";
   }
   exit(0);
}

US_GridControl_T::~US_GridControl_T()
{}

void US_GridControl_T::update_email(const QString &str)
{
   email = str;
}

void US_GridControl_T::write_experiment()
{
   cerr << "write_experiment\n";
   if (!analysis_defined || !data_loaded)
   {
      return;
   }
   if (email.isEmpty())
   {
      cerr << tr("You have to provide an e-mail address!") << endl;
      return;
   }


   /*

   NOTE POTENTIAL BUG:

   There is a potential bug here for the global case because USglobal gets initialized
   for each experiment - pointing potentially to a different result directory for
   different experiments. Although this should really never happen, we should probably
   change the code to parse the result directory outside of the experiment loop.


   */


   printf("write experiment<%s>\n", USglobal->config_list.result_dir.ascii());
   unsigned int count, i, j, k;
   bool union_results;
   QString str;
   QString user = "ultrascan";


   QFile f(USglobal->config_list.result_dir + QString("/experiments%1.dat").arg(timestamp_string));
   cerr << "try to write experiment data for " << analysis_type << "\n";

   {
      char *US = getenv("ULTRASCAN");
      char lockfile[strlen(US) + strlen("/tigre.lock") + 2];
      sprintf(lockfile, "%s/tigre.lock", US);
      int lfd = open(lockfile, O_RDONLY);
      flock(lfd, LOCK_EX);
      int increment = 0;
      while (f.open(IO_ReadOnly))
      {
         f.close();
         increment++;
         f.setName(USglobal->config_list.result_dir + QString("/experiments%1%2.dat").arg(timestamp_string).arg(increment));
      }
      f.close();
      f.open(IO_WriteOnly);
      f.close();
      flock(lfd, LOCK_UN);
      close(lfd);
      if (increment) 
      {
         timestamp_string = QString("%1%2").arg(timestamp_string).arg(increment);
      }
   }
       
   if(f.open(IO_WriteOnly))
   {
      cerr << "write experiment data for " << analysis_type << "\n";
      QDataStream ds(&f);
      ds << email;
      ds << analysis_type;
      ds << fit_tinoise;
      ds << fit_rinoise;
      union_results = false;
      if((analysis_type == "SA2D" ||
          analysis_type == "2DSA" ||
          analysis_type == "2DSA_RA" ||
          analysis_type == "2DSA_MW" ||
          analysis_type == "2DSA_MW_RA")
         && SA2D_Params.uniform_grid_repetition > 1)
      {
         union_results = true;
      }
      ds << union_results;
      ds << meniscus_range;
      ds << fit_meniscus;
      ds << meniscus_gridpoints;
      ds << use_iterative;
      ds << max_iterations;
      ds << regularization;
      ds << (unsigned int)experiment.size();
      cerr << "experiment size is " << experiment.size() << endl;
      cerr << "sizeof(experiment size) is " << sizeof(experiment.size()) << endl;
      cerr << "sizeof(unsigned int) is " << sizeof(unsigned int) << endl;
      if (analysis_type == "SA2D" ||
          analysis_type == "2DSA" ||
          analysis_type == "2DSA_RA")
      {
         ds << SA2D_Params.monte_carlo;
         ds << SA2D_Params.ff0_min;
         ds << SA2D_Params.ff0_max;
         ds << SA2D_Params.ff0_resolution;
         ds << SA2D_Params.s_min;
         ds << SA2D_Params.s_max;
         ds << SA2D_Params.s_resolution;
         ds << SA2D_Params.uniform_grid_repetition;
      }
      if(analysis_type == "2DSA_MW" ||
         analysis_type == "2DSA_MW_RA")
      {
         ds << SA2D_Params.monte_carlo;
         ds << SA2D_Params.ff0_min;
         ds << SA2D_Params.ff0_max;
         ds << SA2D_Params.ff0_resolution;
         ds << SA2D_Params.mw_min;
         ds << SA2D_Params.mw_max;
         ds << SA2D_Params.grid_resolution;
         ds << SA2D_Params.uniform_grid_repetition;
         ds << SA2D_Params.max_mer;
      }
      if (analysis_type == "2DSA_RA" ||
          analysis_type == "2DSA_MW_RA")
      {
         for (unsigned int i = 0; i < experiment.size(); i++)
         {
            ds << simparams_extra[i].simpoints;
            ds << simparams_extra[i].band_volume;
            ds << simparams_extra[i].radial_grid;
            ds << simparams_extra[i].moving_grid;
            ds << (unsigned int)simparams_extra[i].simulation_parameters_full_text.size();
            for (unsigned int j = 0; j < simparams_extra[i].simulation_parameters_full_text.size(); j++) 
            {
               ds << simparams_extra[i].simulation_parameters_full_text[j];
               cout << simparams_extra[i].simulation_parameters_full_text[j] << "\n";
            }
         }
      }
      if(analysis_type == "GA" || 
         analysis_type == "GA_RA")
      {
         ds << GA_Params.monte_carlo;
         ds << GA_Params.demes;
         ds << GA_Params.generations;
         ds << GA_Params.crossover;
         ds << GA_Params.mutation;
         ds << GA_Params.plague;
         ds << GA_Params.elitism;
         ds << GA_Params.migration_rate;
         ds << GA_Params.genes;
         ds << GA_Params.initial_solutes;
         ds << GA_Params.random_seed;
         cerr << "writing demes:" << GA_Params.demes << endl;
         cerr << "generations:" << GA_Params.generations << endl;
         cerr << "crossover:" << GA_Params.crossover << endl;
         cerr << "mutation:" << GA_Params.mutation << endl;
         cerr << "plague:" << GA_Params.plague << endl;
         cerr << "elitism:" << GA_Params.elitism << endl;
         cerr << "migration:" << GA_Params.migration_rate << endl;
         cerr << "genes:" << GA_Params.genes << endl;
         cerr << "solutes:" << GA_Params.initial_solutes << endl;
         cerr << "seed:" << GA_Params.random_seed << endl;
         cerr << "sizeof(unsigned long):" << sizeof(unsigned long) << endl;
         for(i = 0; i < GA_Params.solute.size(); i++)
         {
            ds <<  GA_Params.solute[i].s;
            ds <<  GA_Params.solute[i].s_min;
            ds <<  GA_Params.solute[i].s_max;
            ds <<  GA_Params.solute[i].ff0;
            ds <<  GA_Params.solute[i].ff0_min;
            ds <<  GA_Params.solute[i].ff0_max;
            cerr << "writing solute:" << i << endl;
            cerr << "solute s:" << GA_Params.solute[i].s << endl;
            cerr << "solute smin:" << GA_Params.solute[i].s_min << endl;
            cerr << "solute ff0:" << GA_Params.solute[i].ff0 << endl;
         }
      }
      if(analysis_type == "GA_MW" ||
         analysis_type == "GA_MW_RA")
      {
         ds << GA_Params.monte_carlo;
         ds << GA_Params.demes;
         ds << GA_Params.generations;
         ds << GA_Params.crossover;
         ds << GA_Params.mutation;
         ds << GA_Params.plague;
         ds << GA_Params.elitism;
         ds << GA_Params.migration_rate;
         ds << GA_Params.genes;
         ds << GA_Params.largest_oligomer;
         ds << GA_Params.largest_oligomer_string;
         ds << GA_Params.random_seed;
         ds << GA_Params.mw_min;
         ds << GA_Params.mw_max;
         ds << GA_Params.ff0_min;
         ds << GA_Params.ff0_max;
         cerr << "writing demes:" << GA_Params.demes << endl;
         cerr << "generations:" << GA_Params.generations << endl;
         cerr << "crossover:" << GA_Params.crossover << endl;
         cerr << "mutation:" << GA_Params.mutation << endl;
         cerr << "plague:" << GA_Params.plague << endl;
         cerr << "elitism:" << GA_Params.elitism << endl;
         cerr << "migration:" << GA_Params.migration_rate << endl;
         cerr << "genes:" << GA_Params.genes << endl;
         cerr << "largest_oligomer:" << GA_Params.largest_oligomer << endl;
         cerr << "seed:" << GA_Params.random_seed << endl;
         cerr << "mw_min:" << GA_Params.mw_min << endl;
         cerr << "mw_max:" << GA_Params.mw_max << endl;
         cerr << "ff0_min:" << GA_Params.ff0_min << endl;
         cerr << "ff0_max:" << GA_Params.ff0_max << endl;
      }
      if (analysis_type == "GA_RA" ||
          analysis_type == "GA_MW_RA")
      {
         for (unsigned int i = 0; i < experiment.size(); i++)
         {
            ds << simparams_extra[i].simpoints;
            ds << simparams_extra[i].band_volume;
            ds << simparams_extra[i].radial_grid;
            ds << simparams_extra[i].moving_grid;
            ds << (unsigned int)simparams_extra[i].simulation_parameters_full_text.size();
            for (unsigned int j = 0; j < simparams_extra[i].simulation_parameters_full_text.size(); j++) 
            {
               ds << simparams_extra[i].simulation_parameters_full_text[j];
               cout << simparams_extra[i].simulation_parameters_full_text[j] << "\n";
            }
         }
      }
      if(analysis_type == "GA_SC")
      {
         ds << GA_Params.monte_carlo;
         ds << GA_Params.demes;
         ds << GA_Params.generations;
         ds << GA_Params.crossover;
         ds << GA_Params.mutation;
         ds << GA_Params.plague;
         ds << GA_Params.elitism;
         ds << GA_Params.migration_rate;
         ds << GA_Params.genes;
         ds << GA_Params.random_seed;
         cerr << "writing demes:" << GA_Params.demes << endl;
         cerr << "generations:" << GA_Params.generations << endl;
         cerr << "crossover:" << GA_Params.crossover << endl;
         cerr << "mutation:" << GA_Params.mutation << endl;
         cerr << "plague:" << GA_Params.plague << endl;
         cerr << "elitism:" << GA_Params.elitism << endl;
         cerr << "migration:" << GA_Params.migration_rate << endl;
         cerr << "genes:" << GA_Params.genes << endl;
         cerr << "seed:" << GA_Params.random_seed << endl;
         ds << (unsigned int)constraints_full_text.size();
         for (i = 0; i < constraints_full_text.size(); i++) 
         {
            ds << constraints_full_text[i];
            cout << constraints_full_text[i] << "\n";
         }
         ds << (unsigned int)simulation_parameters_full_text.size();
         for (i = 0; i < simulation_parameters_full_text.size(); i++) 
         {
            ds << simulation_parameters_full_text[i];
            cout << simulation_parameters_full_text[i] << "\n";
         }
      }
      for (i=0; i<experiment.size(); i++)
      {
         ds << experiment[i].id;
         ds << experiment[i].cell;
         ds << experiment[i].channel;
         ds << experiment[i].wavelength;
         ds << experiment[i].meniscus;
         ds << experiment[i].bottom;
         ds << experiment[i].rpm;
         ds << experiment[i].s20w_correction;
         ds << experiment[i].D20w_correction;
         ds << (unsigned int) experiment[i].radius.size();
         for (j=0; j<experiment[i].radius.size(); j++)
         {
            ds << experiment[i].radius[j];
            // ds << (short int)( 10000 * (experiment[i].radius[j] - 5.0));
         }
         ds << (unsigned int)experiment[i].scan.size();
         printf("scan[0][0] %.12g\n", experiment[i].scan[0].conc[0]);

         for (j=0; j<experiment[i].scan.size(); j++)
         {
            ds << experiment[i].scan[j].time;
            ds << experiment[i].scan[j].omega_s_t;
            for (k=0; k<experiment[i].radius.size(); k++)
            {
               ds << experiment[i].scan[j].conc[k];
               // ds << (short int) (10000 * experiment[i].scan[j].conc[k]);
            }
         }

         fprintf(stderr,"experiment last time %g avg_temp %g vbar %g visc %g density %g vbar %g vbar20 %g rpm %u bottom %g meniscus %g scorr %g Dcorr %g\n",
                 experiment[i].scan[experiment[i].scan.size()-1].time,
                 experiment[i].avg_temperature,
                 experiment[i].vbar,
                 experiment[i].viscosity,
                 experiment[i].density,
                 experiment[i].vbar,
                 experiment[i].vbar20,
                 experiment[i].rpm,
                 experiment[i].bottom,
                 experiment[i].meniscus,
                 experiment[i].s20w_correction,
                 experiment[i].D20w_correction);
         ds << experiment[i].viscosity;
         ds << experiment[i].density;
         ds << experiment[i].vbar;
         ds << experiment[i].vbar20;
         ds << experiment[i].avg_temperature;
      }
      f.close();

      /*
        how the process is parallelized:

        At this point we want to split the job into however many clusters exist.
        the total job should be divided into a loadbalanced share for each cluster,
        where the number of jobs depends on the available nodes in a particular cluster and
        their performance.

        Initially, we just want to run on the bcf cluster. So there is only one node
        to send this to, master.hydrodyn.beowulf.

        A job sent to a particular cluster will be sent to the MPI master, which
        allocates portions from an array of jobs to individual nodes available
        over MPI in the cluster. So the job sent to the MPI master will actually
        consist of an array of jobs, or an array of genes, where each element
        in the gene array is a gene, which contains one or more components, or a
        list of solutes that will be simulated and then interpolated to the
        experimental data, then NNLS is performed on the interpolated solution
        and the experimental data. Each job returns the following information:
        the amplitudes found by NNLS for each solute in the simulated solution,
        the time- and radially invariant noise vectors, and finally the variance.

        Each cluster also gets its own job control file, which lists only the
        gene arrays to be compared to the experimental solution, and a list of
        float and integer arrays for any desired use. So each cluster
        gets 2 files: 1. experimental_data.dat which contains the experimental data,
        and 2. genes-<i>.dat, which contains the solute arrays or genes to be computed.
        Each MPI master is expected to be able to figure out how best to re-distribute
        all genes it receives to its own cluster slaves.

        The name of the experimental_data.dat file and the name of the
        solutes-<i>.dat file   are passed as a commandline argument to the MPI
        master, which is called   using a system call.
      */
      QString f_name;

      /*
        find out how many headnodes can be used by looking at the load of each cluster node
      */
      cerr << "b1\n";
      //      cluster->update_load(); // check out the current load and sort according to load
      //      list <cluster_node>::iterator grid_iter;
      cerr << "b2\n";
      count = 0; // count how many clusters are available so we can make some estimate on how to distribute the load
      //   if ((*grid_iter).load_average >= 0 && (*grid_iter).load_average < 2)
      //   {
      // count ++;
      //   }
      i = 0;
      cerr << "b3\n";
      //      for (grid_iter = cluster->grid.begin(); grid_iter != cluster->grid.end(); grid_iter++)
      //      {
      cerr << "b4\n";
      //         i++;
      //         if ((*grid_iter).load_average >= 0 && (*grid_iter).load_average < 2)
      //         {
      /*
        Here, the job vector needs to be populated:

        vector <struct gene> solutions

        where each gene is a vector of solutes. The solutions are genes, i.e.,
        one or more components (solutes) constituting a solution. Basically,
        we have "count" number of clusters available. For now, we can just divide
        the total workload by "count" to scale the jobs.

        The MPI master module (us_fe_nnls_t) will get more than one solution to distribute among its slaves.

        The job vector should be allocated according to the load that can be handled
        by each cluster.

      */
      cerr << "p2\n";
      if (analysis_type == "GA" ||
          analysis_type == "GA_MW" ||
          analysis_type == "GA_RA" ||
          analysis_type == "GA_MW_RA")
      {
         /*
           For the GA method, we may want to perform each deme on a different cluster,
           and split the job into the calculation of individual genes. f_name contains
           the solute-<i>.dat file to be passed to the MPI master module.
         */
         solutions.clear();  // empty solute file for GA
      }
      if (analysis_type == "SA2D" ||
          analysis_type == "2DSA" ||
          analysis_type == "2DSA_RA")
      {
         cerr << "sa2d_2\n";
         /*
           For the SA2D method, we consider the array of solutes inside one grid
           simply as the "gene" to be simulated. So each low-resolution frame is
           one "gene". All low-resolution frames together make up the genes to be
           calculated. f_name contains the solute-<i>.dat file to be passed to the
           MPI master module.
         */
         gene solutes;
         Solute solute;
         unsigned int s_partition = SA2D_Params.s_resolution;
         unsigned int f_partition = SA2D_Params.ff0_resolution;
         unsigned int i, step_s, step_k, m, n, steps_k;
         unsigned int steps = SA2D_Params.uniform_grid_repetition;
         if(f_partition <= 1)
         {
            steps_k = 1;
         }
         else
         {
            steps_k = steps;
         }
         double inc_s, inc_k;
         double s, k, use_s, use_k;
         double s_diff = 1.0e-13 * (SA2D_Params.s_max - SA2D_Params.s_min);
         double ff0_diff = SA2D_Params.ff0_max - SA2D_Params.ff0_min;
         printf("s %g %g %g  ff0 %g %g %g\n",
                SA2D_Params.s_max*1.0e-13 , SA2D_Params.s_min*1.0e-13, s_diff,
                SA2D_Params.ff0_max, SA2D_Params.ff0_min,  ff0_diff);
         for(step_s = 0; step_s < steps; step_s++)
         {
            for(step_k = 0; step_k < steps_k; step_k++)
            {
               solutes.component.clear();
               inc_s = step_s * (s_diff) / ((s_partition - 1) * steps);
               if(f_partition > 1)
               {
                  inc_k = step_k * (ff0_diff) / ((f_partition - 1) * steps);
               }
               else
               {
                  inc_k = 0;
               }
#define DEBUG
#if defined(DEBUG)
               printf("step_s %d step_k %d inc_s %g inc_k %g\n", step_s, step_k, inc_s, inc_k);
#endif

               i = 0;
               for (m = 0; m < s_partition; m++)
               {
                  s = SA2D_Params.s_min*1.0e-13 + m * (s_diff) / (s_partition - 1);
                  for (n = 0; n < f_partition; n++)
                  {
                     if(f_partition > 1)
                     {
                        k = SA2D_Params.ff0_min + n * (ff0_diff) / (f_partition - 1);
                     }
                     else
                     {
                        k = SA2D_Params.ff0_min;
                     }
                     use_s = s + inc_s;
                     use_k = k + inc_k;
                     if(use_s > SA2D_Params.s_max*1.0e-13)
                     {
                        use_s -= s_diff;
                     }
                     if(use_k > SA2D_Params.ff0_max)
                     {
                        use_k -= ff0_diff;
                     }
                     solute.s = use_s;
                     solute.k = use_k;
                     // clipping
                     if(use_s <= (SA2D_Params.s_max*1.0e-13) && use_s >= (SA2D_Params.s_min*1.0e-13) &&
                        use_k <= SA2D_Params.ff0_max && use_k >= SA2D_Params.ff0_min)
                     {
                        if (find(solutes.component.begin(),
                                 solutes.component.end(),
                                 solute) == solutes.component.end())
                        {
                           if ( EXCLUDE_S(use_s) )
                           {
                              printf("skipped, exclusion zone %g %g\n", use_s, use_k);
                           }
                           else
                           {
                              solutes.component.push_back(solute);
                           }
                        }
                        else
                        {
                           printf("skipped, dup on file %g %g\n", use_s, use_k);
                        }
                     }
                  } // n
               } // m
               printf("solute size on push back %ud\n", (unsigned int) solutes.component.size());
               solutions.push_back(solutes);
            } // step_k
         }
      }
      if (analysis_type == "2DSA_MW" ||
          analysis_type == "2DSA_MW_RA")
      {
         cerr << "2dsa_mw\n";
         QString cellwave;
         QDateTime startDateTime = QDateTime::currentDateTime();
         cellwave = cellwave.sprintf(".%d%d", experiment[0].cell + 1, experiment[0].wavelength + 1);
         QFile f("/lustre/tmp/" + experiment[0].id + "_" + analysis_type + "_" + startDateTime.toString("yyMMddhhmmss") + "_grid.model" + cellwave);
         unsigned int f_open = 0;
         QTextStream *ts = (QTextStream *)NULL;
         unsigned int total_solutes = 0;
         if(f.open(IO_WriteOnly))
         {
            f_open = 1;
            QTextStream uts(&f);
            ts = &uts;
            *ts << analysis_type << endl;
            *ts << experiment[0].id << endl;
            *ts << "0" << endl;
         }

         /*
           For the SA2D method, we consider the array of solutes inside one grid
           simply as the "gene" to be simulated. So each low-resolution frame is
           one "gene". All low-resolution frames together make up the genes to be
           calculated. f_name contains the solute-<i>.dat file to be passed to the
           MPI master module.
         */
         gene solutes;
         Solute solute;
         unsigned int mw_partition = SA2D_Params.grid_resolution;
         unsigned int f_partition = SA2D_Params.ff0_resolution;
         unsigned int step_mw, step_k, m, n, steps_k, l;
         unsigned int steps = SA2D_Params.uniform_grid_repetition;
         if(f_partition <= 1)
         {
            steps_k = 1;
         }
         else
         {
            steps_k = steps;
         }
         double inc_mw, inc_k;
         double mw, k, use_mw, use_k, use_s, mw_diff;
         double ff0_diff = SA2D_Params.ff0_max - SA2D_Params.ff0_min;
         for(step_mw = 0; step_mw < steps; step_mw++)
         {
            for(step_k = 0; step_k < steps_k; step_k++)
            {
               if(f_partition > 1)
               {
                  inc_k = step_k * (ff0_diff) / ((f_partition - 1) * steps);
               }
               else
               {
                  inc_k = 0;
               }
               for (m = 0; m < mw_partition; m++)
               {
                  solutes.component.clear();
                  for(l = 1; l <= SA2D_Params.max_mer; l++)
                  {
                     if((SA2D_Params.max_mer_string.ascii())[l-1] != '1')
                     {
                        printf("pos mer %u skipped\n", l);
                     }
                     else
                     {
                        printf("pos mer %u\n", l);
                        mw_diff = (SA2D_Params.mw_max - SA2D_Params.mw_min) * l;
                        inc_mw = step_mw * (mw_diff) / ((mw_partition - 1) * steps);
                        printf("mw grid %u mw %g %g diff %g inc_mw %g\n", l,
                               SA2D_Params.mw_min * l,
                               SA2D_Params.mw_max * l,
                               mw_diff,
                               inc_mw);
                        mw = SA2D_Params.mw_min * l + m * (mw_diff) / (mw_partition - 1);
#define DEBUG
#if defined(DEBUG)

                        printf("m %u step_mw %u step_k %u inc_mw %g mw_diff %g inc_k %g\n", m, step_mw, step_k, inc_mw, mw_diff, inc_k);
#endif

                        for (n = 0; n < f_partition; n++)
                        {
                           if(f_partition > 1)
                           {
                              k = SA2D_Params.ff0_min + n * (ff0_diff) / (f_partition - 1);
                           }
                           else
                           {
                              k = SA2D_Params.ff0_min;
                           }
                           use_mw = mw + inc_mw;
                           use_k = k + inc_k;
                           if(use_mw > SA2D_Params.mw_max * l)
                           {
                              use_mw -= mw_diff;
                           }
                           if(use_k > SA2D_Params.ff0_max)
                           {
                              use_k -= ff0_diff;
                           }
                           use_s = pow(pow((use_mw * experiment[0].vbar20)/(AVOGADRO * M_PI), 2e0)/6e0,1e0/3e0)
                              *   (1.0 - DENS_20W * experiment[0].vbar20)
                              / (3.0 * use_k * experiment[0].vbar20 * VISC_20W);

                           solute.s = use_s;
                           solute.k = use_k;
#if defined(DEBUG)

                           printf("push (%g) %g,%g\n", use_mw, use_s, use_k);
#endif
                           // clipping
                           if(use_mw <= (SA2D_Params.mw_max*l) && use_mw >= (SA2D_Params.mw_min*l) &&
                              use_k <= SA2D_Params.ff0_max && use_k >= SA2D_Params.ff0_min)
                           {
                              if(find(solutes.component.begin(),
                                      solutes.component.end(),
                                      solute) == solutes.component.end())
                              {
                                 if ( EXCLUDE_S(use_s) )
                                 {
                                    printf("skipped, exclusion zone %g %g\n", use_s, use_k);
                                 }
                                 else
                                 {
                                    solutes.component.push_back(solute);
                                 }
                              }
                              else
                              {
                                 printf("skipped, dup on file (%g) %g %g\n", use_mw, use_s, use_k);
                              }
                           }
                        } // n
                     } // else
                  } // l
                  printf("solute size on push back %u\n", (unsigned int) solutes.component.size());
                  solutions.push_back(solutes);
                  total_solutes += solutes.component.size();
               } // m
            } // step_k
         } // step_mw
         fflush(stdout);
         if(f_open)
         {
            *ts << total_solutes << endl;
            unsigned int i,j;
            for(i = 0; i < solutions.size(); i++)
            {
               for(j = 0; j < solutions[i].component.size(); j++)
               {
                  *ts << "1" << endl;
                  *ts << solutions[i].component[j].s << endl;
                  *ts << (R * K20/(AVOGADRO * solutions[i].component[j].k * 9.0 * VISC_20W * M_PI
                                   * pow( (2.0 * solutions[i].component[j].s * solutions[i].component[j].k *
                                           experiment[0].vbar20 * VISC_20W)/(1.0 - experiment[0].vbar20 * DENS_20W), 0.5))) << endl;
                  *ts << "0\n0\n";
               }
            }
            f.close();
         }
      }

      f_name.sprintf(USglobal->config_list.result_dir + "/solutes%s-%d.dat", timestamp_string.ascii(), i);
      write_solutes(f_name); // write the solutions vector to a file
      QString mpifilebase = "> /lustre/tmp/gc_mpi_" + timestamp_string + ".%1";
      if(gridopt == "TIGRE")
      {
         mpifilebase = "> /lustre/tmp/gc_tigre_" + timestamp_string + ".%1";
      }
      QString npstring;
      QString machines;
      QString gp_analysis_type = analysis_type;
      if (analysis_type == "SA2D" ||
          analysis_type == "2DSA" ||
          analysis_type == "2DSA_MW")
      {
         npstring = "`head -1 $ULTRASCAN/etc/mpi_sa2d_proccount`";
         machines = " -machinefile $ULTRASCAN/etc/mpi_sa2d_machines ";
         if(SA2D_Params.monte_carlo > 1)
         {
            gp_analysis_type = gp_analysis_type + QString("-MC-%1").arg(SA2D_Params.monte_carlo);
         }
      }

      if(analysis_type == "GA" ||
         analysis_type == "GA_MW")
      {
         npstring = QString("%1").arg(GA_Params.demes + 1);
         machines = " -machinefile $ULTRASCAN/etc/mpi_ga_machines ";
         if(GA_Params.monte_carlo > 1)
         {
            gp_analysis_type = gp_analysis_type + QString("-MC-%1").arg(GA_Params.monte_carlo);
         }
      }

#ifdef BIN64
      QString syscall = "echo 'mpi_job_run [" + experiment[0].id + "-" + QString("%1").arg(experiment[0].cell + 1) + "]["
         + email + "][" + gp_analysis_type + "][" + gcfile +
         + "] nohup rsh -n -l apache bcf.uthscsa.edu \". /etc/profile; cd " + USglobal->config_list.result_dir + "; mpirun -np "
         + npstring + machines + " $ULTRASCAN/bin64/us_fe_nnls_t_mpi "
         + USglobal->config_list.result_dir + QString("/experiments%1.dat ").arg(timestamp_string)
         + f_name + " "
         + mpifilebase.arg(1) + " 2" + mpifilebase.arg(2) + "\" 2>&1 " + mpifilebase.arg(0)
         + " ' > $ULTRASCAN/etc/us_gridpipe";
#else

      QString syscall = "echo 'mpi_job_run [" + experiment[0].id + "-" + QString("%1").arg(experiment[0].cell + 1) + "]["
         + email + "][" + gp_analysis_type + "][" + gcfile +
         + "] nohup rsh -n -l apache bcf.uthscsa.edu \". /etc/profile; cd " + USglobal->config_list.result_dir + "; mpirun -np "
         + npstring + machines
         + " $ULTRASCAN/bin/us_fe_nnls_t_mpi "
         + USglobal->config_list.result_dir + QString("/experiments%1.dat ").arg(timestamp_string)
         + f_name + " "
         + mpifilebase.arg(1) + " 2" + mpifilebase.arg(2) + "\" 2>&1 " + mpifilebase.arg(0)
         + " ' > $ULTRASCAN/etc/us_gridpipe";
#endif

      if(gridopt == "TIGRE")
      {
         syscall = "echo 'tigre_job_run [" + experiment[0].id + QString("%1").arg(experiment[0].cell + 1) + "]["
            + email + "][" + gp_analysis_type + "][" + gcfile +
            + "] nohup perl -I$ULTRASCAN/etc $ULTRASCAN/etc/us_tigre_job.pl "
            + gcfile + " " + QString("%1").arg(total_points)
            + " " + email + " " + timestamp_string + " " + USglobal->config_list.result_dir
            + " " + USglobal->config_list.result_dir + QString("/experiments%1.dat ").arg(timestamp_string)
            + f_name + " " + system_name + " " + mpifilebase.arg(1) + " 2" + mpifilebase.arg(2)
            + " ' > $ULTRASCAN/etc/us_gridpipe";
      }

#if defined(KOZA)
      syscall = "nohup rsh -n -l root koza \". /etc/profile; mpirun -np 2 /root/ultrascan/bin/us_fe_nnls_t_mpi "
         + USglobal->config_list.result_dir + QString("/experiments%1.dat ").arg(timestamp_string)
         + f_name + " > mpilog.1 2> mpilog.2\" 2>&1 > /mpilog.0";
#endif

      cerr << syscall << endl;
      printf("$ %s\n", syscall.ascii());
      // issue the call to the remote MPI master which will
      // be sent to each individual cluster.
      if(gridopt != "no")
      {
         system(syscall.ascii());
      }
      else
      {
         cerr << "syscall disabled!" << endl;
      }
   }
   else
   {
      cerr << tr("UltraScan Fatal Error: There was a problem opening the output file:\n\n")
           << USglobal->config_list.result_dir + "/us_gridcontrol.dat\n\n"
           << tr("Please make sure you have write permission and then try again...");
      exit(-5);
   }
}

void US_GridControl_T::write_solutes(const QString &filename)
{
   // write solute arrays (genes) to file:
   QFile f(filename);
   if (f.open(IO_WriteOnly))
   {
      QDataStream ds(&f);
      ds << (unsigned int) solutions.size(); // how many genes
      for (unsigned int i=0; i<solutions.size(); i++)
      {
         printf("solute %d size %u\n", i, (unsigned int) solutions[i].component.size());
         ds << (unsigned int) solutions[i].component.size(); // the size of each gene
         for (unsigned int j=0; j<solutions[i].component.size(); j++)
         {
            ds << solutions[i].component[j].s;
            ds << solutions[i].component[j].k;
            ds << solutions[i].component[j].c;
         }
      }
      ds << (unsigned int) Control_Params.float_params.size();
      for (unsigned int i=0; i<Control_Params.float_params.size(); i++)
      {
         ds << (float) Control_Params.float_params[i];
      }
      ds << (unsigned int) Control_Params.int_params.size();
      for (unsigned int i=0; i<Control_Params.int_params.size(); i++)
      {
         ds << (int) Control_Params.int_params[i];
      }
      f.close();
   }
}

// this function is probably not needed...
void US_GridControl_T::write_ga_experiment()
{
   QFile f(USglobal->config_list.result_dir + "/ga_control.dat");
   if(f.open(IO_WriteOnly))
   {
      QDataStream ds(&f);
      ds << GA_Params.demes;
      ds << GA_Params.generations;
      ds << GA_Params.crossover;
      ds << GA_Params.mutation;
      ds << GA_Params.plague;
      ds << GA_Params.elitism;
      ds << GA_Params.genes;
      ds << GA_Params.initial_solutes;
      ds << GA_Params.random_seed;
      ds << GA_Params.regularization;
      ds << GA_Params.solute.size();
      for (unsigned int i=0; i<GA_Params.solute.size(); i++)
      {
         ds << (float) GA_Params.solute[i].s;
         ds << (float) GA_Params.solute[i].s_min;
         ds << (float) GA_Params.solute[i].s_max;
         ds << (float) GA_Params.solute[i].ff0;
         ds << (float) GA_Params.solute[i].ff0_min;
         ds << (float) GA_Params.solute[i].ff0_max;
      }
      f.close();
   }
}


// this function is probably not needed...

void US_GridControl_T::write_sa2d_experiment()
{
   QFile f(USglobal->config_list.result_dir + "/sa2d_control.dat");
   if(f.open(IO_WriteOnly))
   {
      QDataStream ds(&f);
      ds << SA2D_Params.ff0_min;
      ds << SA2D_Params.ff0_max;
      ds << SA2D_Params.ff0_resolution;
      ds << SA2D_Params.s_min;
      ds << SA2D_Params.s_max;
      ds << SA2D_Params.s_resolution;
      f.close();
   }
}

void US_GridControl_T::add_experiment()
{
   if (run_inf.scans[selected_cell][selected_lambda] == 0)
   {
      cerr << "There are no scans in this experiment, aborting with exit code -4..." << endl;
      exit(-4);
   }
   mfem_data temp_experiment;
   mfem_scan temp_scan;
   QString str;
   temp_experiment.radius.clear();
   temp_experiment.scan.clear();
   for (unsigned int i=0; i<experiment.size(); i++) // check to make sure this data has not already been selected
   {
      if (experiment[i].id         == run_inf.run_id
          &&  experiment[i].cell       == selected_cell
          &&  experiment[i].wavelength == selected_lambda
          &&  experiment[i].channel    == selected_channel)
      {
         cerr << experiment[i].id << tr(" has already been included, skipped...\n");
      }
   }
   cerr.precision(10);
   cout.precision(10);
   cerr << "buoyancyb: " << corr_inf.buoyancyb << ", buoyancyw: " << corr_inf.buoyancyw << endl;
   cerr << "viscosity_tb: " << corr_inf.viscosity_tb << ", density_tb: " << corr_inf.density_tb << endl;
   temp_experiment.s20w_correction = (corr_inf.buoyancyw * corr_inf.viscosity_tb)
      /(corr_inf.buoyancyb * (100.0 * VISC_20W));
   temp_experiment.D20w_correction = (K20 * corr_inf.viscosity_tb)
      /((100.0 * VISC_20W) * (run_inf.avg_temperature + K0));
   // (D_tb * D20w_correction = D_20w, D_20w/D20w_correction = D_tb)
   cerr << "s_correction: " << temp_experiment.s20w_correction << ", D_correction: " << temp_experiment.D20w_correction << endl;
   cerr << "Density: " << corr_inf.density << ", Viscosity: " << corr_inf.viscosity << ", vbar: " << corr_inf.vbar << ", vbar20: " << corr_inf.vbar20 << endl;
   cerr << "Temperature:  " << run_inf.avg_temperature << endl;
   temp_experiment.id = run_inf.run_id;
   temp_experiment.cell = selected_cell;
   temp_experiment.channel = selected_channel;
   temp_experiment.wavelength = selected_lambda;
   temp_experiment.meniscus = run_inf.meniscus[selected_cell];
   if (analysis_type == "GA_SC") 
   {
      temp_experiment.bottom = calc_bottom(rotor_list,
                                           cp_list, run_inf.rotor, run_inf.centerpiece[selected_cell],
                                           selected_channel, 0);
   } else {
      temp_experiment.bottom = calc_bottom(rotor_list,
                                           cp_list, run_inf.rotor, run_inf.centerpiece[selected_cell],
                                           selected_channel, run_inf.rpm[selected_cell][selected_lambda][0]);
   }
   temp_experiment.rpm = run_inf.rpm[selected_cell][selected_lambda][0];
   printf("absorbance[0][0] %.12g baseline %12g\n", channel_data.absorbance[0][0], run_inf.baseline[selected_cell][selected_lambda]);

   for (unsigned int i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
   {
      temp_scan.conc.clear();
      temp_scan.time = run_inf.time[selected_cell][selected_lambda][i];
      temp_scan.omega_s_t = run_inf.omega_s_t[selected_cell][selected_lambda][i];
      for (unsigned int j=0; j<points; j++)
      {
         temp_scan.conc.push_back(channel_data.absorbance[i][j]);
      }
      temp_experiment.scan.push_back(temp_scan);
   }
   for (unsigned int i=0; i<points; i++)
   {
      temp_experiment.radius.push_back(channel_data.radius[i]);
   }
   printf("den %g visc %g\n", corr_inf.density, corr_inf.viscosity);
   temp_experiment.viscosity = corr_inf.viscosity;
   temp_experiment.density = corr_inf.density;
   temp_experiment.vbar = corr_inf.vbar;
   temp_experiment.vbar20 = corr_inf.vbar20;
   temp_experiment.avg_temperature = run_inf.avg_temperature;
   experiment.push_back(temp_experiment);
   data_loaded = true;
   printf("range_left = %f\n", run_inf.range_left[selected_cell][selected_lambda][0]);
}
