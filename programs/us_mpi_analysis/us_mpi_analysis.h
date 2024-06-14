//! \file us_mpi_analysis.h
#ifndef US_MPI_ANALYSIS_H
#define US_MPI_ANALYSIS_H

#include <QtCore>
#include <QtNetwork>
#include <mpi.h>

#include "us_model.h"
#include "us_dmga_constr.h"
#include "us_pcsa_modelrec.h"
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

//! \class US_MPI_Analysis
//! \brief Class for performing MPI-based analysis.
class US_MPI_Analysis : public QObject
{
    Q_OBJECT

    public:
        //! \brief Constructor for US_MPI_Analysis
        //! \param argc Argument count
        //! \param argv Argument vector
        US_MPI_Analysis(int argc, QStringList& argv);

    public slots:
                //! \brief Start the analysis process
                void start(void);

    private:
        //! \enum attr_type
        //! \brief Enumeration for attribute types
        enum attr_type { ATTR_S, ATTR_K, ATTR_W, ATTR_V, ATTR_D, ATTR_F };

        int proc_count;          //!< Number of processes
        int my_rank;             //!< Rank of the current process
        int my_group;            //!< Group of the current process
        int mgroup_count;        //!< Number of groups
        int group_rank;          //!< Rank within the group
        int my_workers;          //!< Number of workers in the group
        int gcores_count;        //!< Global core count
        int max_walltime;        //!< Maximum wall time
        int iterations;          //!< Number of iterations (Master only - Iterative)
        int max_iterations;      //!< Maximum number of iterations (Master only - Iterative)
        int mc_iterations;       //!< Monte Carlo iterations
        int mc_iteration;        //!< Current Monte Carlo iteration
        int max_experiment_size; //!< Maximum experiment size
        int total_points;        //!< Total points
        int dbg_level;           //!< Debug level
        bool dbg_timing;         //!< Debug timing flag
        bool glob_runid;         //!< Global run ID flag
        bool do_astfem;          //!< ASTFEM flag
        bool is_global_fit;      //!< Global fit flag
        bool is_composite_job;   //!< Composite job flag

        MPI_Comm my_communicator; //!< MPI communicator

        int current_dataset;      //!< Current dataset for global fit or composite
        int datasets_to_process;  //!< Datasets to process for global fit or composite
        int count_calc_residuals; //!< Counter for calculated residuals
        int count_datasets;       //!< Dataset count

        int population;           //!< Population size
        int generations;          //!< Number of generations
        int crossover;            //!< Crossover rate
        int mutation;             //!< Mutation rate
        int plague;               //!< Plague rate
        int migrate_count;        //!< Migration count
        int elitism;              //!< Elitism rate
        int attr_x;               //!< X attribute
        int attr_y;               //!< Y attribute
        int attr_z;               //!< Z attribute

        double mutate_sigma;      //!< Mutation sigma
        double p_mutate_s;        //!< Probability of mutation for s
        double p_mutate_k;        //!< Probability of mutation for k
        double p_mutate_sk;       //!< Probability of mutation for sk
        double alpha;             //!< Alpha value

        long int maxrss;          //!< Maximum RSS
        int min_experiment_size;  //!< Minimum experiment size
        static const int def_experiment_size = 100; //!< Default experiment size

    #if QT_VERSION > 0x050000
        const double min_variance_improvement = 1.0e-100; //!< Minimum variance improvement
    #else
        static const double min_variance_improvement = 1.0e-100; //!< Minimum variance improvement
    #endif

        QVector< int > worker_status;      //!< Worker status
        QVector< int > worker_depth;       //!< Worker depth
        QList< int > ds_startx;            //!< Dataset start indices
        QList< int > ds_points;            //!< Dataset points
        QVector< double > gl_nnls_a;       //!< Global NNLS A
        QVector< double > gl_nnls_b;       //!< Global NNLS B
        QVector< double > meniscus_values; //!< Meniscus values
        QVector< double > bottom_values;   //!< Bottom values

        QVector< double > concentrations;  //!< Concentrations
        QVector< double > maxods;          //!< Maximum OD values
        QVector< double > mc_data;         //!< Monte Carlo data
        QVector< double > sigmas;          //!< Sigma values
        QVector< long > work_rss;          //!< Work RSS

        double meniscus_range;   //!< Meniscus range (Master only)
        double meniscus_value;   //!< Meniscus value (Worker only)
        double bottom_range;     //!< Bottom range (Master only)
        double bottom_value;     //!< Bottom value (Worker only)

        int max_depth;           //!< Maximum depth
        int worknext;            //!< Next work index
        int fit_mb_select;       //!< Fit meniscus/bottom selection (0-3)
        int menibott_ndx;        //!< Meniscus/bottom index
        int meniscus_run;        //!< Meniscus run
        int bottom_run;          //!< Bottom run
        int menibott_count;      //!< Meniscus/bottom count
        int meniscus_points;     //!< Meniscus points
        int bottom_points;       //!< Bottom points
        int set_count;           //!< Set count

        //! \enum WorkerStatus
        //! \brief Enumeration for worker statuses
        enum WorkerStatus { INIT, READY, WORKING };

        //! \enum PMGTag
        //! \brief Enumeration for PMG tags
        enum PMGTag { ADATESIZE=1000, ADATE, STARTITER, STARTLAST, UDPSIZE, UDPMSG, DONEITER, DONELAST };

        bool fit_meni;           //!< Fit meniscus flag
        bool fit_bott;           //!< Fit bottom flag
        bool fit_menbot;         //!< Fit meniscus+bottom flag

        US_DataIO::RawData* res_data;      //!< Residual data (populated in calc_residuals)
        US_DataIO::RawData* sim_data;      //!< Simulation data (populated in calc_residuals)
        US_DataIO::RawData  sim_data1;     //!< Simulation data for Monte Carlo iteration 1
        US_DataIO::RawData  scaled_data;   //!< Scaled data after global fit

        US_ModelRecord mrec;                //!< Work PCSA model record
        QVector< US_ModelRecord > mrecs;    //!< PCSA model records (curves)
        QStringList cm_files;               //!< PCSA Monte Carlo component models files
        double pararry[36];                 //!< PCSA parameter limits array

        QHostAddress server;                //!< Server address
        quint16 port;                       //!< Server port
        QUdpSocket* socket;                 //!< UDP socket

        QString requestID;                  //!< Request ID
        QString directory;                  //!< Directory path
        QString analysis_type;              //!< Analysis type
        QString cluster;                    //!< Cluster name
        QString db_name;                    //!< Database name
        QString modelGUID;                  //!< Model GUID
        QString requestGUID;                //!< Request GUID
        QString analysisDate;               //!< Analysis date

        QMap< QString, QString > parameters; //!< Parameters map
        QMap< QString, QString > task_params; //!< Task parameters map

        QDateTime submitTime;               //!< Submission time
        QDateTime startTime;                //!< Start time

        QList< DATASET* > data_sets;        //!< List of datasets

        //! \class MPI_Job
        //! \brief Class representing an MPI job.
        class MPI_Job
        {
            public:
                static const int MASTER = 0; //!< Master rank
                static const int TAG3 = 3;   //!< Tag 3

                //! \enum Command
                //! \brief Enumeration for MPI job commands
                enum Command { IDLE, PROCESS, WAIT, SHUTDOWN, NEWDATA, PROCESS_MC };

                //! \enum Status
                //! \brief Enumeration for MPI job statuses
                enum Status { TAG0, READY, RESULTS, RESULTS_MC };

                int solution;       //!< Solution index
                int length;         //!< Length of the job
                Command command;    //!< Command for the job
                int depth;          //!< Depth of the job
                double meniscus_value; //!< Meniscus value
                double bottom_value; //!< Bottom value
                int dataset_offset; //!< Dataset offset
                int dataset_count;  //!< Dataset count

                //! \brief Constructor for MPI_Job
                MPI_Job()
                {
                    solution = 0;
                    length = 0;
                    command = IDLE;
                    depth = 0;
                    meniscus_value = 0.0;
                    bottom_value = 0.0;
                    dataset_offset = 0;
                    dataset_count = 1;
                }
        };

        QList< QVector< US_Solute > > orig_solutes; //!< Original solutes list
        QVector< US_Solute > ljob_solutes;          //!< Local job solutes
        QList< QVector< US_ZSolute > > orig_zsolutes; //!< Original Z solutes list

        //! \class Sa_Job
        //! \brief Class representing a simulated annealing job.
        class Sa_Job
        {
            public:
                MPI_Job mpi_job;                 //!< MPI job
                QVector< US_Solute > solutes;    //!< Solutes list
                QVector< US_ZSolute > zsolutes;  //!< Z solutes list
        };

        QList< Sa_Job > job_queue; //!< Job queue

    #if QT_VERSION > 0x050000
        const double LARGE = 1.e39; //!< Large value
    #else
        static const double LARGE = 1.e39; //!< Large value
    #endif
        static const int solute_doubles = sizeof(US_Solute) / sizeof(double); //!< Solute doubles
        static const int zsolut_doubles = sizeof(US_ZSolute) / sizeof(double); //!< Z solute doubles
        QList< QVector< US_Solute > > calculated_solutes; //!< Calculated solutes list
        QList< QVector< US_Solute > > ds_calc_solutes; //!< Dataset calculated solutes list
        QVector< US_Solute > dset_calc_solutes; //!< Dataset calculated solutes
        QList< QVector< US_ZSolute > > calculated_zsolutes; //!< Calculated Z solutes list

        SIMULATION simulation_values; //!< Simulation values
        SIMULATION wksim_vals;        //!< Worker simulation values
        SIMULATION previous_values;   //!< Previous simulation values

        //! \class Result
        //! \brief Class representing a result for 2DSA.
        class Result
        {
            public:
                int depth;                      //!< Depth of the result
                int worker;                     //!< Worker index
                QVector< US_Solute > solutes;   //!< Solutes list
                QVector< US_ZSolute > zsolutes; //!< Z solutes list
        };

        QList< Result > cached_results; //!< Cached results list

        //! \class Bucket
        //! \brief Class representing a bucket for genetic algorithm.
        class Bucket
        {
            public:
                double x_min; //!< Minimum X value
                double x_max; //!< Maximum X value
                double y_min; //!< Minimum Y value
                double y_max; //!< Maximum Y value
                double ds;    //!< Delta S value
                double dk;    //!< Delta K value
        };

        int generation;       //!< Generation count
        int s_grid;           //!< S grid size
        int k_grid;           //!< K grid size
        int p_grid;           //!< P grid size
        int fitness_count;    //!< Fitness count

        typedef QVector< US_Solute > Gene; //!< Gene type

        double regularization; //!< Regularization value
        double concentration_threshold; //!< Concentration threshold
        QList< Bucket > buckets; //!< Buckets list
        QList< Gene > genes;     //!< Genes list
        QList< Gene > best_genes; //!< Best genes list (size is number of processors)
        QList< SIMULATION > sim_values; //!< Simulation values list
        QMap< QString, double > fitness_map; //!< Fitness map
        int fitness_hits; //!< Fitness hits count
        QList< DGene > dgenes; //!< DGene list
        QList< DGene > best_dgenes; //!< Best DGene list (size is number of workers)
        US_dmGA_Constraints constraints; //!< Genetic algorithm constraints
        US_Model wmodel; //!< Model object
        DGene dgene; //!< DGene object
        QVector< double > dgmarker; //!< DG marker
        QVector< US_dmGA_Constraints::Constraint > cns_flt; //!< Constraints filter
        QVector< int > lfvari; //!< LF variables
        double base_sig; //!< Base sigma
        int nfloatc; //!< Float count
        int ncompc; //!< Component count
        int nassocc; //!< Association count
        int nfvari; //!< NF variables
        int minimize_opt; //!< Minimize option
        int g_redo_inc; //!< Redo increment
        bool in_gsm; //!< In GSM flag

        //! \class Fitness
        //! \brief Class representing a fitness value for genetic algorithm.
        class Fitness
        {
            public:
                int index;      //!< Index into genes and sim_values lists
                double fitness; //!< Fitness value

                //! \brief Equality operator
                //! \param f Another Fitness object
                //! \return True if fitness values are equal
                bool operator== (const Fitness& f)
                {
                    return f.fitness == fitness;
                }

                //! \brief Inequality operator
                //! \param f Another Fitness object
                //! \return True if fitness values are not equal
                bool operator!= (const Fitness& f)
                {
                    return f.fitness != fitness;
                }

                //! \brief Less-than operator for sorting fitness values
                //! \param f Another Fitness object
                //! \return True if the fitness value is less than the other
                bool operator< (const Fitness& f) const
                {
                    return (fitness < f.fitness);
                }
        };

        QList< Fitness > fitness; //!< Fitness list
        QList< Fitness > best_fitness; //!< Best fitness list (size is number of processors)

        //! \class MPI_GA_MSG
        //! \brief Class representing a message for genetic algorithm MPI.
        class MPI_GA_MSG
        {
            public:
                int generation; //!< Generation count (from worker: -1 = final; from master: 1 = done)
                int size;       //!< Number of solutes in the following vector or genes requested for emigration
                double fitness; //!< Fitness value of best result
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
        void     submit            ( Sa_Job&, int );
        void     submit_pcsa       ( Sa_Job&, int );
        void     add_to_queue      ( Sa_Job& );
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
        void     write_superg      ( const SIMULATION&, US_Model::AnalysisType );
        void     stats_output      ( int, int, int,
                                     QDateTime, QDateTime, QDateTime );
        void     pm_2dsa_master    ( void );
        void     pm_ga_master      ( void );
        void     pm_dmga_master    ( void );
        int      ready_worker      ( void );
        int      low_working_depth ( void );
        void     cache_result      ( Result& );
        void     process_solutes   ( int&, int&, QVector< US_Solute >& );
        void     dset_matrices     ( int, int*,
                                     QVector< double >&, QVector< double >&,
                                     QVector< int >& );
        void     update_outputs    ( bool = false );
        US_Model::AnalysisType model_type( const QString );

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

        // PCSA Master
        void    pcsa_master         ( void );
        void    init_pcsa_solutes   ( void );
        void    fill_pcsa_queue     ( void );
        void    process_pcsa_results( const int, const int* );
        void    process_pcsa_solutes( Result& );
        void    write_mrecs         ( void );
        void    iterate_pcsa        ( void );
        void    tikreg_pcsa         ( void );
        void    montecarlo_pcsa     ( void );
        void    pcsa_best_model     ( void );
        void    write_pcsa_aux_model( const int );
        void    filter_mrecs        ( const int, QVector< US_ModelRecord >&,
                                                 QVector< US_ModelRecord >& );
        void    clean_mrecs         ( QVector< US_ModelRecord >& );
        double  alpha_scan          ( void );
        void    apply_alpha( const double, QVector< double >*, QVector< double >*,
                             const int, const int, const int, double&, double& );
        QString shorter_filename    ( const QString );


        // PCSA Worker
        void    pcsa_worker         ( void );

        // Parallel Masters
        void    pmasters_start     ( void );
        void    task_parse         ( const QString& );
        void    pmasters_supervisor( void );
        void    pmasters_master    ( void );
        void    pmasters_worker    ( void );
        void    time_mc_iterations ( void );
        void    pm_cjobs_start     ( void );
        void    pm_cjobs_supervisor( void );
        void    pm_cjobs_master    ( void );
        void    pm_cjobs_worker    ( void );
        void    time_datasets_left ( void );
        void    pm_2dsa_cjmast     ( void );
        void    pm_ga_cjmast       ( void );
        void    pm_dmga_cjmast     ( void );
        void    pm_pcsa_cjmast     ( void );

        // Debug
        void    dump_buckets( void );
        void    dump_genes  ( int );
        void    dump_fitness( const QList< Fitness >& );
};

#endif
