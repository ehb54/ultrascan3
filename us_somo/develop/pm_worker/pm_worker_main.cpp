#include "pm_worker.h"

int npes;
int myrank;

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &npes);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

  MPI_Status mpi_status;

  PM_WORKER *pm = (PM_WORKER *)0;

  pm_msg msg;

  msg.type = PM_REGISTER;

  vector<double> F;
  vector<double> q;
  vector<double> I;
  vector<double> e;

  vector<double> I_result;
  vector<double> params;
  set<pm_point> model;

  int errorno = -10000;

  /* no registry for now, assume all workers available
     if ( MPI_SUCCESS != MPI_Send( &msg,
     sizeof( pm_msg ),
     MPI_CHAR,
     0,
     0,
     MPI_COMM_WORLD ) )
     {
     cout << myrank << ":MPI send failed in nsa_ga_worker() initial registry\n"
     << flush; MPI_Abort( MPI_COMM_WORLD, errorno - myrank ); exit( errorno -
     myrank );
     }
  */

  do {
    // cout << myrank << ":worker listening\n"  << flush;
    if (MPI_SUCCESS != MPI_Recv(&msg, sizeof(pm_msg), MPI_CHAR, 0, PM_MSG,
                                MPI_COMM_WORLD, &mpi_status)) {
      cout << myrank << ":MPI Initial Receive failed in pm_mpi_worker()\n"
           << flush;
      MPI_Abort(MPI_COMM_WORLD, errorno - myrank);
      exit(errorno - myrank);
    }

    // cout << msg;

    switch (msg.type) {
      case PM_SHUTDOWN: {
        cout << myrank << ":worker PM_SHUTDOWN\n" << flush;
        MPI_Finalize();
        exit(0);
      } break;

      case PM_NEW_PM: {
        // cout << myrank << ":worker PM_NEW_PM\n"  << flush;
        if (pm) {
          delete pm;
          pm = (PM_WORKER *)0;
        }
        bool use_errors = msg.flags & PM_USE_ERRORS;
        unsigned int tot_vsize = msg.vsize * (use_errors ? 4 : 3);
        vector<double> d(tot_vsize);
        F.resize(msg.vsize);
        q.resize(msg.vsize);
        I.resize(msg.vsize);
        e.resize(use_errors ? msg.vsize : 0);

        if (MPI_SUCCESS != MPI_Recv(&(d[0]), tot_vsize * sizeof(double),
                                    MPI_CHAR, 0, PM_NEW_PM, MPI_COMM_WORLD,
                                    &mpi_status)) {
          cout << myrank << ":MPI PM_NEW_PM Receive failed in pm_mpi_worker()\n"
               << flush;
          MPI_Abort(MPI_COMM_WORLD, errorno - myrank);
          exit(errorno - myrank);
        }

        if (use_errors) {
          for (int i = 0; i < msg.vsize; i++) {
            F[i] = d[i];
            q[i] = d[msg.vsize + i];
            I[i] = d[2 * msg.vsize + i];
            e[i] = d[3 * msg.vsize + i];
          }
        } else {
          for (int i = 0; i < msg.vsize; i++) {
            F[i] = d[i];
            q[i] = d[msg.vsize + i];
            I[i] = d[2 * msg.vsize + i];
          }
        }

        pm = new PM_WORKER(msg.grid_conversion_factor, msg.max_dimension,
                           msg.max_harmonics, F, q, I, e, msg.max_mem_in_MB, 0,
                           true);
      } break;

      case PM_NEW_GRID_SIZE: {
        // cout << myrank << ":worker PM_NEW_GRID_SIZE\n"  << flush;
        if (!pm) {
          cout << myrank
               << ":MPI PM_NEW_GRID_SIZE Receive called before PM_NEW_PM  "
                  "pm_mpi_worker()\n"
               << flush;
          MPI_Abort(MPI_COMM_WORLD, errorno - myrank);
          exit(errorno - myrank);
        }
        pm->set_grid_size(msg.grid_conversion_factor, true);
      } break;

      case PM_CALC_FITNESS: {
        // cout << myrank << ":worker PM_CALC_FITNESS\n"  << flush;
        if (!pm) {
          cout << myrank
               << ":MPI PM_CALC_FITNESS Receive called before PM_NEW_PM  "
                  "pm_mpi_worker()\n"
               << flush;
          MPI_Abort(MPI_COMM_WORLD, errorno - myrank);
          exit(errorno - myrank);
        }

        params.resize(msg.vsize);
        if (MPI_SUCCESS != MPI_Recv(&(params[0]), msg.vsize * sizeof(double),
                                    MPI_CHAR, 0, PM_CALC_FITNESS,
                                    MPI_COMM_WORLD, &mpi_status)) {
          cout << myrank
               << ":MPI PM_CALC_FITNESS Receive failed in pm_mpi_worker()\n"
               << flush;
          MPI_Abort(MPI_COMM_WORLD, errorno - myrank);
          exit(errorno - myrank);
        }

        if (!pm->create_model(params, model) ||
            !pm->compute_I(model, I_result)) {
          cout << myrank << ":MPI PM_CALC_FITNESS error in pm_mpi_worker()\n"
               << pm->error_msg << endl
               << flush;
          MPI_Abort(MPI_COMM_WORLD, errorno - myrank);
          exit(errorno - myrank);
        }

        msg.type = PM_FITNESS_RESULT;
        msg.vsize = model.size();
        msg.model_fitness = pm->fitness2(I_result);

        vector<int16_t> vmodel;
        for (set<pm_point>::iterator it = model.begin(); it != model.end();
             it++) {
          vmodel.push_back(it->x[0]);
          vmodel.push_back(it->x[1]);
          vmodel.push_back(it->x[2]);
        }

        if (MPI_SUCCESS != MPI_Send(&msg, sizeof(msg), MPI_CHAR, 0,
                                    PM_FITNESS_RESULT, MPI_COMM_WORLD)) {
          cout << myrank
               << ":MPI PM_CALC_FITNESS Send 1 failed in pm_mpi_worker()\n"
               << flush;
          MPI_Abort(MPI_COMM_WORLD, errorno - myrank);
          exit(errorno - myrank);
        }

        if (MPI_SUCCESS !=
            MPI_Send(&(vmodel[0]), 3 * msg.vsize * sizeof(int16_t), MPI_CHAR, 0,
                     PM_FITNESS_RESULT_MODEL, MPI_COMM_WORLD)) {
          cout << myrank
               << ":MPI PM_CALC_FITNESS Send 2 failed in pm_mpi_worker()\n"
               << flush;
          MPI_Abort(MPI_COMM_WORLD, errorno - myrank);
          exit(errorno - myrank);
        }
      } break;

      default: {
        cout << myrank
             << ":MPI Receive unknown message type in pm_mpi_worker() "
             << msg.type << endl
             << flush;
        MPI_Abort(MPI_COMM_WORLD, errorno - myrank);
        exit(errorno - myrank);
      }
    }
  } while (true);
  MPI_Abort(MPI_COMM_WORLD, -5);
  exit(-5);
}
