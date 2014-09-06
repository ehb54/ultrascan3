#ifndef US_MPI_ANALYSIS_H
#define US_MPI_ANALYSIS_H

#include <QtCore>
#include <QtNetwork>
#include <mpi.h>

#include "us_model.h"
#include "us_dmga_constr.h"
#include "us_dataIO.h"
#include "us_noise.h"
#include "us_simparms.h"
#include "us_solve_sim.h"
#include "us_vector.h"
#include "us_math2.h"

#define SIMULATION       US_SolveSim::Simulation
#define DATASET          US_SolveSim::DataSet
#define DGene            US_Model

#define DbgLv(a) if(dbg_level>=a)qDebug() //!< debug-level-conditioned qDebug()
#define DbTiming if(dbg_timing)qDebug()   //!< debug-timing-conditioned qDebug()

class US_MPI_Analysis : public QObject
{
 Q_OBJECT

  public:
    US_MPI_Analysis( int, QStringList& );

  public slots:
    void start( void );
     
  private:

    enum attr_type { ATTR_S, ATTR_K, ATTR_W, ATTR_V, ATTR_D, ATTR_F };

    int                 proc_count;
    int                 my_rank;
    int                 my_group;
    int                 mgroup_count;
    int                 group_rank;
    int                 my_workers;
    int                 gcores_count;
    int                 max_walltime;
    int                 iterations;           // Master only - Iterative
    int                 max_iterations;       // Master only - Iterative
    int                 mc_iterations;        // Monte Carlo
    int                 mc_iteration;         // Monte Carlo current iteration
    int                 max_experiment_size;
    int                 total_points;
    int                 dbg_level;
    bool                dbg_timing;
    bool                glob_runid;
    bool                do_astfem;
    MPI_Comm            my_communicator;

    int                 current_dataset;      // For global fit
    int                 datasets_to_process;  // For global fit
    int                 count_calc_residuals; // Simple counter

    int                 population;
    int                 generations;
    int                 crossover;
    int                 mutation;
    int                 plague;
    int                 migrate_count;
    int                 elitism;
    int                 attr_x;
    int                 attr_y;
    int                 attr_z;

    double              mutate_sigma;
    double              p_mutate_s;
    double              p_mutate_k;
    double              p_mutate_sk;
    double              beta;

    long int            maxrss;
    static const int    min_experiment_size      = 100;
    static const double min_variance_improvement = 1.0e-100;
                        
    QVector< int >      worker_status;
    QVector< int >      worker_depth;
    QList< int >        ds_startx;
    QList< int >        ds_points;
    QVector< double >   gl_nnls_a;
    QVector< double >   gl_nnls_b;
    int                 max_depth;
    int                 worknext;
    enum                WorkerStatus { INIT, READY, WORKING };
    enum                PMGTag { ADATESIZE=1000, ADATE, STARTITER, STARTLAST,
                                 UDPSIZE, UDPMSG, DONEITER, DONELAST };
                        
    int                 meniscus_points;
    int                 meniscus_run;
    double              meniscus_range;   // Only used by master
    double              meniscus_value;   // Only used by worker
    QVector< double >   meniscus_values;

    QVector< double >   concentrations;
    QVector< double >   maxods;
    QVector< double >   mc_data;
    QVector< double >   sigmas;
    QVector< long >     work_rss;

    US_DataIO::RawData* res_data;        // Populated in calc_residuals
    US_DataIO::RawData* sim_data;        // Populated in calc_residuals
    US_DataIO::RawData  sim_data1;       // Simulation for mc iteration 1
    US_DataIO::RawData  scaled_data;     // Populated after global fit

    QHostAddress        server;
    quint16             port;
    QUdpSocket*         socket;
                       
    QString             requestID;
    QString             directory;
    QString             analysis_type;
    QString             cluster;
    QString             db_name;
    QString             modelGUID;
    QString             requestGUID;
    QString             analysisDate;

    QMap< QString, QString > parameters;
    QMap< QString, QString > task_params;
  
    QDateTime           submitTime;
    QDateTime           startTime;

    int                 set_count;
    QList< DATASET* >   data_sets;

    class MPI_Job
    {
        public:
            static const int MASTER = 0;
            static const int TAG3   = 3;

            enum Command { IDLE, PROCESS, WAIT, SHUTDOWN, NEWDATA };
            enum Status  { TAG0, READY, RESULTS };

            int     solution;
            int     length;
            Command command;
            int     depth;
            double  meniscus_value;
            int     dataset_offset;
            int     dataset_count;

            MPI_Job()
            {
                solution       = 0;
                length         = 0;
                command        = IDLE;
                depth          = 0;
                meniscus_value = 0.0;
                dataset_offset = 0;
                dataset_count  = 1;
            };
    };

    QList< QVector< US_Solute > >  orig_solutes;
    QVector< US_Solute >           ljob_solutes;

    class _2dsa_Job
    {
       public:
          MPI_Job              mpi_job;
          QVector< US_Solute > solutes;
    };

    QList< _2dsa_Job > job_queue;

    static const double LARGE          = 1.e39;
    static const int    solute_doubles = sizeof( US_Solute ) / sizeof( double );
    QList< QVector< US_Solute > > calculated_solutes;
    QList< QVector< US_Solute > > ds_calc_solutes;
    QVector< US_Solute >          dset_calc_solutes;

    SIMULATION simulation_values;
    SIMULATION wksim_vals;
    SIMULATION previous_values;

    // 2DSA class
    class Result
    {
       public:
          int                   depth;
          int                   worker;
          QVector< US_Solute >  solutes;
    };

    QList< Result >             cached_results;

    // GA class variables and classes

    class Bucket
    {
      public:
         double x_min;
         double x_max;
         double y_min;
         double y_max;
         double ds;
         double dk;
    };

    int                       generation;
    int                       s_grid;
    int                       k_grid;
    int                       p_grid;
    int                       fitness_count;

    typedef QVector< US_Solute > Gene;

    double                    regularization;
    double                    concentration_threshold;
    QList< Bucket >           buckets;
    QList< Gene >             genes;
    QList< Gene >             best_genes;   // Size is number of processors
    QList< SIMULATION >       sim_values;
    QMap < QString, double >  fitness_map;
    int                       fitness_hits;
    QList< DGene >            dgenes;
    QList< DGene >            best_dgenes;  // Size is number of workers
    US_dmGA_Constraints       constraints;
    US_Model                  wmodel;
    DGene                     dgene;
    QVector< double >         dgmarker;
    QVector< US_dmGA_Constraints::Constraint >  cns_flt;
    QVector< int >            lfvari;
    double                    base_sig;
    int                       nfloatc;
    int                       ncompc;
    int                       nassocc;
    int                       nfvari;
    int                       minimize_opt;
    int                       g_redo_inc;
    bool                      in_gsm;

    class Fitness
    {
      public:
       int    index;      // Index into genes and sim_values lists
       double fitness;

       // Set the operators so we can sort a fitness list
       bool operator== ( const Fitness& f )
       {
          return f.fitness == fitness;
       }
       
       bool operator!= ( const Fitness& f )
       {
          return f.fitness != fitness;
       }
       
       // Sort smallest to largest
       bool operator< ( const Fitness& f ) const
       {
          return ( fitness < f.fitness );
       }
    };

    QList< Fitness > fitness;
    QList< Fitness > best_fitness; // Size is number of processors

    class MPI_GA_MSG
    {
      public:
       int    generation;  // From worker: -1 = final; From master: 1 = done
       int    size;        // Number of solutes in the following vector or
                           // or genes requested for emmigration
       double fitness;     // Fitness of best result
    };

    enum { GENERATION, GENE, IMMIGRATE, EMMIGRATE, UPDATE, FINISHED };

    // Methods

    void     parse         ( const QString& );
    void     parse_job     ( QXmlStreamReader& );
    void     parse_dataset ( QXmlStreamReader&, DATASET* );
    void     parse_files   ( QXmlStreamReader&, DATASET* );
    void     parse_solution( QXmlStreamReader&, DATASET* );
    void     send_udp      ( const QString& );
    void     abort         ( const QString&, int=-1 );
    long int max_rss       ( void );
    QString  par_key_value ( const QString, const QString );

    Gene     create_solutes( double, double, double,
                             double, double, double );
    void     init_solutes  ( void );
    void     fill_queue    ( void );
    void     limitBucket   ( Bucket& );

    // Master
    void     _2dsa_master      ( void );
    void     submit            ( _2dsa_Job&, int );
    void     add_to_queue      ( _2dsa_Job& );
    void     process_results   ( int, const int* );
    void     shutdown_all      ( void );
    void     write_noise       ( US_Noise::NoiseType, const QVector< double>& );
    void     iterate           ( void );
    void     set_meniscus      ( void );
    void     set_monteCarlo    ( void );
    void     write_output      ( void );
    void     write_global      ( void );
    void     set_gaussians     ( void );
    void     global_fit        ( void );
    void     write_model       ( const SIMULATION&, US_Model::AnalysisType,
                                 bool = false );
    void     stats_output      ( int, int, int, QDateTime, QDateTime, QDateTime );
    void     pm_2dsa_master    ( void );
    void     pm_ga_master      ( void );
    void     pm_dmga_master    ( void );
    int      ready_worker      ( void );
    int      low_working_depth ( void );
    void     cache_result      ( Result& );
    void     process_solutes   ( int&, int&, QVector< US_Solute >& );
    void     dset_matrices     ( int, int,
                                 QVector< double >&, QVector< double >& );

    // Worker
    void     _2dsa_worker      ( void );

    void     calc_residuals     ( int, int, SIMULATION& );

    // GA Master
    void ga_master       ( void );
    void ga_master_loop  ( void );
    void ga_global_fit   ( void );
    void set_gaMonteCarlo( void );

    // GA Worker
    void   ga_worker     ( void );
    void   ga_worker_loop( void );
    Gene   new_gene      ( void );
    //void   init_fitness  ( void );
    void   mutate_s      ( US_Solute&, int );
    void   mutate_k      ( US_Solute&, int );
    void   mutate_gene   ( Gene& );
    void   cross_gene    ( Gene&, QList< Gene > );
    int    migrate_genes ( void );
    double random_01     ( void );
    int    u_random      ( int = 100 );
    int    e_random      ( void );
    double minimize      ( Gene&, double );
    double get_fitness   ( const Gene& );
    double get_fitness_v ( const US_Vector& );
    double update_fitness( int, US_Vector& );
    void   lamm_gsm_df   ( const US_Vector&, US_Vector& );
    void   align_gene    ( Gene& );

    void   vector_scaled_sum   ( US_Vector&, US_Vector&, double,
                                 US_Vector&, double = 1.0 );
    void   pmasters_start      ( void );
    void   task_parse          ( const QString& );
    void   pmasters_supervisor ( void );
    void   pmasters_master     ( void );
    void   pmasters_worker     ( void );
    void   time_mc_iterations  ( void );
    void   solutes_from_gene   ( Gene&, int );
    void   set_comp_attrib     ( US_Model::SimulationComponent&,
                                 double, int );
    void   build_component( US_Model::SimulationComponent&,
                            US_Math2::SolutionData&, double, double );

    // DMGA Master
    void    dmga_master        ( void );
    void    dmga_master_loop   ( void );
    void    dmga_global_fit    ( void );
    void    set_dmga_gaussians ( void );
    void    set_dmga_MonteCarlo( void );
    void    marker_from_dgene  ( QVector< double >&, DGene& );
    void    dgene_from_marker  ( QVector< double >&, DGene& );
    void    dgenes_to_marker   ( QVector< double >&, QList< DGene >&,
                                 const int, const int );
    void    marker_to_dgenes   ( QVector< double >&, QList< DGene >&,
                                 const int, const int );
    bool    store_attr_value   ( double&, US_Model&,
                                 US_dmGA_Constraints::AttribType&, int& );
    bool    fetch_attr_value   ( double&, US_Model&,
                                 US_dmGA_Constraints::AttribType&, int& );
    void    model_from_dgene   ( US_Model&, DGene& dgene );

    // DMGA Worker
    void    dmga_worker        ( void );
    void    dmga_worker_loop   ( void );
    DGene   new_dmga_gene      ( void );
    void    mutate_dgene       ( DGene& );
    void    cross_dgene        ( DGene&, QList< DGene > );
    int     migrate_dgenes     ( void );
    double  get_fitness_dmga   ( DGene& );
    double  get_fitness_v_dmga ( US_Vector&, US_Vector& );
    void    lamm_gsm_df_dmga   ( US_Vector&, US_Vector&, US_Vector& );
    double  minimize_dmga      ( DGene&, double );
    QString dgene_key          ( DGene& );
    void    calc_residuals_dmga( int, int, SIMULATION&, DGene& );

    // Debug
    void dump_buckets( void );
    void dump_genes  ( int );
    void dump_fitness( const QList< Fitness >& );
};
#endif

