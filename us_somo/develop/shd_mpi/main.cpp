#include "shd.h"

// #define SH_TEST
#define SHOW_MPI_TIMING
#define RUN_SINGLE

#if defined(SH_TEST)
#include <sstream>
#endif

#if defined(RUN_SINGLE)
#include <time.h>
#endif

int world_size;
int world_rank;

void our_abort(MPI_Comm comm, int errno) {
#if !defined(RUN_SINGLE)
  MPI_Abort(comm, errno);
#endif
  exit(errno);
}

// #define USE_GSL

int main(int argc, char **argv) {
#if defined(SH_TEST)
  shd_double delta = .5;
  for (int i = 0; i < 50; i++) {
    ostringstream fname;
    fname << "shbes_";
#if defined(USE_GSL)
    fname << "gsl";
#else
    fname << "nr";
#endif
    fname << delta;
    fname << "_" << i << ".dat";
    cout << ">" << fname.str() << endl;
    ofstream ofs(fname.str().c_str(), ios::out);
    ofs << "# us-somo: shbes l from nr::shbes" << endl;
    for (shd_double x = 0; x < 1000; x += delta) {
      shd_double res;
      nr::sphbes(i, x, res);
      ofs << x << "\t" << res << endl;
      // cout << x << "\t" << res << endl;
    }
    ofs.close();
  }
  exit(0);
#endif

  shd_input_data id;

  vector<shd_double> q;
  vector<vector<shd_double> > F;
  vector<shd_double> I;
  vector<shd_point> my_model;

#if defined(RUN_SINGLE)
  world_rank = 0;
  world_size = 1;
#else
  MPI_Init(&argc, &argv);

  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  MPI_Get_processor_name(processor_name, &name_len);
  int is_mic = std::string::npos != std::string(processor_name).find("-mic");
#endif

  //    printf("%s, rank %d"
  //           " of %d a %sa MIC\n",
  //           processor_name, world_rank, world_size, is_mic ? "" : "NOT " );

  vector<shd_point> model;
  if (!world_rank) {
    cout << "sizeof float :" << sizeof(float) << endl;
    cout << "sizeof shd_double:" << sizeof(shd_double) << endl;

    int errorno = -1;
    if (argc < 2) {
      cerr << "Error: usage: shd inputfile\n";
      our_abort(MPI_COMM_WORLD, errorno);
      exit(errorno);
    }
    errorno--;

#if defined(RUN_SINGLE)
    if (world_size != 1) {
      cerr << "Error: np must be exactly 1\n";
      our_abort(MPI_COMM_WORLD, errorno);
      exit(errorno);
    }
#else
    if (world_size < 2) {
      cerr << "Error: np must be at least 2\n";
      our_abort(MPI_COMM_WORLD, errorno);
      exit(errorno);
    }
#endif
    errorno--;

    //       if ( world_size != 2 )
    //       {
    //          cerr << "Error: np must be at exactly 2 for now\n";
    //          our_abort( MPI_COMM_WORLD, errorno );
    //          exit( errorno );
    //       }
    //       errorno--;

    // read in data file

    ifstream ifs(argv[1], ios::in | ios::binary);

    if (!ifs.is_open()) {
      cerr << "Error: shd coult not open inputfile " << argv[1] << endl;
      our_abort(MPI_COMM_WORLD, errorno);
      exit(errorno);
    }
    errorno--;

    if (!ifs.read((char *)&id, sizeof(id))) {
      cerr << "Error: " << argv[1] << " can not read input data" << endl;
      our_abort(MPI_COMM_WORLD, errorno);
      exit(errorno);
    }
    errorno--;
    cout << "max harmonics:" << id.max_harmonics << endl;
    cout << "F size       :" << id.F_size << endl;
    cout << "q size       :" << id.q_size << endl;
    cout << "model size   :" << id.model_size << endl;

    vector<shd_double> tmp_F(id.q_size);

    q.resize(id.q_size);
    model.resize(id.model_size);

    for (int32_t i = 0; i < id.F_size; ++i) {
      if (!ifs.read((char *)(&tmp_F[0]), sizeof(shd_double) * id.q_size)) {
        cerr << "Error: " << argv[1] << " can not read F data" << endl;
        our_abort(MPI_COMM_WORLD, errorno);
        exit(errorno);
      }
      F.push_back(tmp_F);
    }
    errorno--;

    if (!ifs.read((char *)(&q[0]), sizeof(shd_double) * id.q_size)) {
      cerr << "Error: " << argv[1] << " can not read q data" << endl;
      our_abort(MPI_COMM_WORLD, errorno);
      exit(errorno);
    }
    errorno--;

    if (!ifs.read((char *)(&model[0]),
                  sizeof(struct shd_point) * id.model_size)) {
      cerr << "Error: " << argv[1] << " can not read model data" << endl;
      our_abort(MPI_COMM_WORLD, errorno);
      exit(errorno);
    }
    errorno--;

    ifs.close();

#if defined(RUN_SINGLE)
#if defined(SHOW_MPI_TIMING)
    clock_t time_start;
    time_start = clock();
#endif
    vector<complex<float> > Avp;
    SHD tSHD((unsigned int)id.max_harmonics, model, F, q, I, 0);
    // tSHD.printF();
    // tSHD.printmodel();
    if (!tSHD.compute_amplitudes(Avp)) {
      cout << tSHD.error_msg << endl << flush;
      cerr << tSHD.error_msg << endl << flush;
      exit(-1);
    }

    vector<double> I_result(tSHD.q_points);
    complex<float> *A1vp = &(Avp[0]);
    // tSHD.printA( Avp );
    for (unsigned int j = 0; j < tSHD.q_points; ++j) {
      I_result[j] = 0e0;
      for (unsigned int k = 0; k < tSHD.Y_points; ++k) {
        I_result[j] += norm((*A1vp));
        ++A1vp;
      }
      I_result[j] *= M_4PI;
    }
    // write it out
    string fname = string(argv[1]) + "_I.dat";
    ;
    // fname.replace( fname.end() - 4, 4, "_I_out.dat" );
    ofstream ofs(fname.c_str(), ios::out);

    ofs << "# us-somo: shd I from " << argv[1] << endl;
    for (unsigned int j = 0; j < tSHD.q_points; ++j) {
      ofs << q[j] << "\t" << I_result[j] << endl;
    }
    ofs.close();
#if defined(SHOW_MPI_TIMING)
    clock_t time_end = clock();
    printf("%d of %d: compute amplitudes %gms model size %d\n", world_rank,
           world_size, (time_end - time_start) * 1e3 / CLOCKS_PER_SEC,
           (int)my_model.size());
#endif
    exit(0);
#endif

    uint32_t org_model_size = id.model_size;
    id.model_size = (org_model_size % (world_size - 1) ? 1 : 0) +
                    org_model_size / (world_size - 1);
    uint32_t extd_model_size = id.model_size * (world_size - 1);

    printf("org model_size %d\n", org_model_size);
    printf("new model size %d\n", id.model_size);
    printf("extd model size %d\n", extd_model_size);

    // pad for scatter
    model.resize(extd_model_size);
    for (int i = org_model_size; i < extd_model_size; i++) {
      model[i].ff_type = -1;
    }

    // broadcast input data

    if (MPI_SUCCESS !=
        MPI_Bcast(&id, sizeof(id), MPI_CHAR, 0, MPI_COMM_WORLD)) {
      cerr << "Error: MPI_Bcast( id ) sender failed" << endl;
      our_abort(MPI_COMM_WORLD, errorno);
      exit(errorno);
    }
    errorno--;

    cout << world_rank << ": bcast F" << endl << flush;
    // broadcast F
    for (int32_t i = 0; i < id.F_size; ++i) {
      if (MPI_SUCCESS !=
          MPI_Bcast(&(F[i][0]), id.q_size, MPI_SHD_DOUBLE, 0, MPI_COMM_WORLD)) {
        cerr << "Error: MPI_Bcast( id ) sender failed" << endl;
        our_abort(MPI_COMM_WORLD, errorno);
        exit(errorno);
      }
    }
    errorno--;

    cout << world_rank << ": bcast q" << endl << flush;
    // broadcast q
    if (MPI_SUCCESS !=
        MPI_Bcast(&(q[0]), id.q_size, MPI_SHD_DOUBLE, 0, MPI_COMM_WORLD)) {
      cerr << "Error: MPI_Bcast( q ) sender failed" << endl;
      our_abort(MPI_COMM_WORLD, errorno);
      exit(errorno);
    }
    errorno--;

    // Scatter model

    my_model.resize(id.model_size);
    cout << world_rank << ": send model" << endl << flush;

#if defined USE_SCATTER

    if (MPI_SUCCESS != MPI_Scatter((void *)&(model[0]),
                                   id.model_size * sizeof(struct shd_point),
                                   MPI_CHAR, (void *)&(my_model[0]),
                                   id.model_size * sizeof(struct shd_point),
                                   MPI_CHAR, 0, MPI_COMM_WORLD)) {
      cerr << "Error: MPI_Scatter( model ) failed" << endl;
      our_abort(MPI_COMM_WORLD, errorno);
      exit(errorno);
    }
    errorno--;

#else

    MPI_Request mpi_req[world_size - 1];
    for (int i = 1; i < world_size; ++i) {
      // cout << "0 sending to" << i << endl << flush;
      if (MPI_SUCCESS != MPI_Isend((void *)&(model[id.model_size * (i - 1)]),
                                   id.model_size * sizeof(struct shd_point),
                                   MPI_CHAR, i, i, MPI_COMM_WORLD,
                                   &(mpi_req[i - 1]))) {
        cerr << "Error: MPI_send( model ) failed" << endl;
        our_abort(MPI_COMM_WORLD, errorno);
        exit(errorno);
      }
      // cout << "0 sending to " << i << "done" << endl << flush;
    }
    errorno--;

#endif

    // split model & distribute to processes
  } else {
    int errorno = -100;
    // broadcast (receive) data

    if (MPI_SUCCESS !=
        MPI_Bcast(&id, sizeof(id), MPI_CHAR, 0, MPI_COMM_WORLD)) {
      cerr << "Error:" << world_rank << " MPI_Bcast( id ) failed" << endl;
      our_abort(MPI_COMM_WORLD, errorno);
      exit(errorno);
    }
    cout << "max harmonics:" << world_rank << " " << id.max_harmonics << endl;
    cout << "F size       :" << world_rank << " " << id.F_size << endl;
    cout << "q size       :" << world_rank << " " << id.q_size << endl;
    cout << "model size   :" << world_rank << " " << id.model_size << endl;

    F.resize(id.F_size);
    q.resize(id.q_size);
    my_model.resize(id.model_size);

    // broadcast (receive) F

    cout << world_rank << ": receive F" << endl << flush;
    for (int32_t i = 0; i < id.F_size; ++i) {
      F[i].resize(id.q_size);
      if (MPI_SUCCESS !=
          MPI_Bcast(&(F[i][0]), id.q_size, MPI_SHD_DOUBLE, 0, MPI_COMM_WORLD)) {
        cerr << "Error: MPI_Bcast( id ) sender failed" << endl;
        our_abort(MPI_COMM_WORLD, errorno);
        exit(errorno);
      }
    }
    errorno--;

    cout << world_rank << ": receive q" << endl << flush;
    // broadcast (receive) q
    if (MPI_SUCCESS !=
        MPI_Bcast(&(q[0]), id.q_size, MPI_SHD_DOUBLE, 0, MPI_COMM_WORLD)) {
      cerr << "Error: MPI_Bcast( q ) sender failed" << endl;
      our_abort(MPI_COMM_WORLD, errorno);
      exit(errorno);
    }
    errorno--;
    cout << world_rank << ": q received" << endl << flush;

    void *null = (void *)0;

#if defined USE_SCATTER

    if (MPI_SUCCESS !=
        MPI_Scatter(null, id.model_size * sizeof(struct shd_point), MPI_CHAR,
                    &(my_model[0]), id.model_size * sizeof(struct shd_point),
                    MPI_CHAR, 0, MPI_COMM_WORLD)) {
      cerr << "Error: MPI_Scatter( model ) failed" << endl;
      our_abort(MPI_COMM_WORLD, errorno);
      exit(errorno);
    }

    errorno--;
#else
    cout << world_rank << ": receive" << endl << flush;
    MPI_Status mpistat;
    // cout << world_rank << ": waiting to receive\n" << flush;
    if (MPI_SUCCESS != MPI_Recv((void *)&(my_model[0]),
                                id.model_size * sizeof(struct shd_point),
                                MPI_CHAR, 0, world_rank, MPI_COMM_WORLD,
                                &mpistat)) {
      cerr << "Error: MPI_recv( model ) failed" << endl;
      our_abort(MPI_COMM_WORLD, errorno);
      exit(errorno);
    }
    cout << world_rank << ": model received" << endl << flush;
    // cout << world_rank << ": received\n" << flush;
#endif
  }

  vector<complex<float> > Avp;

  cout << world_rank << " initial barrier\n" << endl << flush;
  MPI_Barrier(MPI_COMM_WORLD);
  cout << world_rank << " initial barrier exit\n" << endl << flush;

#if defined(SHOW_MPI_TIMING)
  double time_start;
  if (!world_rank) {
    time_start = MPI_Wtime();
  }
#endif

  SHD tSHD((unsigned int)id.max_harmonics, my_model, F, q, I, 0);
  if (world_rank) {
    if (!tSHD.compute_amplitudes(Avp)) {
      cout << tSHD.error_msg << endl << flush;
      cerr << tSHD.error_msg << endl << flush;
      MPI_Abort(MPI_COMM_WORLD, -2000);
      exit(-1);
    }
  } else {
    Avp = tSHD.A1v0;
  }

  vector<complex<float> > Avpsum = Avp;

  if (MPI_SUCCESS != MPI_Reduce((void *)&(Avp[0]), (void *)&(Avpsum[0]),
                                tSHD.q_Y_points * 2, MPI_FLOAT, MPI_SUM, 0,
                                MPI_COMM_WORLD)) {
    cerr << world_rank << "Error: MPI_reduce() failed" << endl;
    our_abort(MPI_COMM_WORLD, -1000);
    exit(-1000);
  }

  //    if ( world_rank == 1 )
  //    {
  //       tSHD.printmodel();
  //       tSHD.printF();
  //       tSHD.printq();
  //       tSHD.printA( Avpsum );
  //    }

  if (!world_rank) {
    // tSHD.printA( Avpsum );
    vector<double> I_result(tSHD.q_points);
    complex<float> *A1vp = &(Avpsum[0]);
    for (unsigned int j = 0; j < tSHD.q_points; ++j) {
      I_result[j] = 0e0;
      for (unsigned int k = 0; k < tSHD.Y_points; ++k) {
        I_result[j] += norm((*A1vp));
        ++A1vp;
      }
      I_result[j] *= M_4PI;
    }
    // write it out
    string fname = string(argv[1]) + "_I.dat";
    ;
    // fname.replace( fname.end() - 4, 4, "_I_out.dat" );
    ofstream ofs(fname.c_str(), ios::out);

    ofs << "# us-somo: shd I from " << argv[1] << endl;
    for (unsigned int j = 0; j < tSHD.q_points; ++j) {
      ofs << q[j] << "\t" << I_result[j] << endl;
    }
    ofs.close();
  }

  cout << world_rank << " done" << endl << flush;

#if defined(SHOW_MPI_TIMING)
  // cout << world_rank << " final barrier\n" << endl << flush;
  MPI_Barrier(MPI_COMM_WORLD);
  //    cout << world_rank << " final barrier exit\n" << endl << flush;
  if (!world_rank) {
    double time_end = MPI_Wtime();
    printf("%d of %d: compute amplitudes %gms model size %d\n", world_rank,
           world_size, (time_end - time_start) * 1e3, (int)my_model.size());
  }
#endif
  MPI_Finalize();
  exit(0);
}
