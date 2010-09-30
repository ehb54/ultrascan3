#ifndef US_FE_NNLS_T_H
#define US_FE_NNLS_T_H

#if defined(NO_US)
#include "us_mfem_test.h"
#else
#include "us_mfem.h"
#endif
#include "us_extern.h"
#include "us_gridcontrol.h"
#include "us_sa2dbase.h"
#include <qfile.h>
#include <qstring.h>
#include <qwidget.h>
#include <qdatastream.h>
#include <vector>
#include <iostream>
#include <qdatetime.h>
#include <qhostaddress.h>
#include <qsocketdevice.h>

using namespace std;

class Expdata
{
public:
   QString tag;
   vector < Solute > solutes;
   vector < Simulation_values > sve;
   Simulation_values sv;
   bool operator==(const Expdata& objIn)
   {
      return (tag == objIn.tag);
   }
};

class US_EXTERN US_fe_nnls_t 
// : public QWidget
{
   //   Q_OBJECT
   
 public:
   
   //      US_fe_nnls_t(QWidget *p = 0, const char *name = 0);
   US_fe_nnls_t();
   ~US_fe_nnls_t();
   vector <struct mfem_data> experiment;
   vector <struct gene> solutions;
   bool fit_tinoise, fit_rinoise, union_results;
   struct control_parameters Control_Params;
   QString email;
   QString analysis_type;
   float meniscus_range;
   bool fit_meniscus, use_iterative;
   unsigned int meniscus_gridpoints, max_iterations;
   float regularization;
   QDateTime startDateTime, endDateTime;
   void WriteResults(struct mfem_data, Simulation_values, QString, double, unsigned int);
   void WriteResults(vector<struct mfem_data>, Simulation_values, QString, double, unsigned int);
   void WriteResults(vector<struct mfem_data>, vector<Solute>, vector<Simulation_values>, QString, double, unsigned int);
   void WriteResultsSC(vector<struct mfem_data>, vector<Solute>, vector<Simulation_values>, QString, double, unsigned int, int);
   void BufferResults(vector<struct mfem_data>, Simulation_values, QString, double, unsigned int);
   void BufferResults(vector<struct mfem_data>, vector<Solute>, vector<Simulation_values>, QString, double, unsigned int);
   Simulation_values buffer_sv;
   vector<Simulation_values> buffer_sve;
   struct ga_data GA_Params;
   struct sa2d_data SA2D_Params;
   QString gridopt;
   vector <QString> constraints_full_text;
   vector <QString> simulation_parameters_full_text;
   vector <SimparamsExtra> simparams_extra;
   QHostAddress host_address_udp;
   int host_port;
   QSocketDevice *socket_device_udp;
   QString job_id;
   QString job_udp_msg_key;
   QString job_udp_msg_status;
   QString job_udp_msg_mc;
   QString job_udp_msg_gen;
   QString job_udp_msg_gen_best;
   QString job_udp_msg_meniscus;
   QString job_udp_msg_iterative;
   void send_udp_msg();
   vector < rotorInfo > rotor_list;
   QString checkpoint_file;
   int mc_cutoff;

   Simulation_values calc_residuals(vector <struct mfem_data>, vector <Solute>, double, int, unsigned int);
   Simulation_values calc_residuals(struct mfem_data, vector <Solute>);
   Simulation_values regularize(Simulation_values, double);
   int init_run(const QString &, const QString &, const QString &, const QString &, const QString &, int);
   int run();
   int run(int);

   void write_checkpoint(
                         int *monte_carlo_iteration,
                         vector < mfem_data > *org_experiment,
                         vector < mfem_data > *save_gaussians,
                         list < Expdata > *expdata_list
                         );
   void read_checkpoint(
                        int *monte_carlo_iteration,
                        vector < mfem_data > *org_experiment,
                        vector < mfem_data > *save_gaussians,
                        list < Expdata > *expdata_list
                        );

   bool gridrmsd;
   bool gridrmsd2;

   vector < mfem_data > build_model(double s, double k);

   void match_rmsd(
                   double *s_new, 
                   double *k_new,
                   double s,
                   double k,
                   bool float_s,
                   double rmsd
                   );
};

typedef struct _MPI_GA_Work_Msg {
   int gen; // from worker -1 = final results, from master 0 = keep going, 1 were done!
   unsigned int size;        // size of migrate file or # of solutes in following solute vector
   double fitness;           // fitness of the best result
} MPI_GA_Work_Msg;

extern long getrss(int pid);
extern long maxrss;
extern SimulationParameters simulation_parameters;
extern ModelSystem model_system;
extern ModelSystemConstraints model_system_constraints;
extern vector<struct mfem_data> last_residuals;

#endif
