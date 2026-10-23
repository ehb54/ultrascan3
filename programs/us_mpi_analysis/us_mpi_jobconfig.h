//! \file us_mpi_jobconfig.h
#ifndef US_MPI_JOBCONFIG_H
#define US_MPI_JOBCONFIG_H

#include <QtCore>

#include "us_simparms.h"
#include "us_solution.h"
#include "us_solve_sim.h"

//! \class US_MPI_JobConfig
//! \brief The job description of an hpcrequest XML file (US_JobSubmit).
//!
//! fromXml() only reads the file: it has no MPI, no output and no global
//! state, and it collects errors instead of aborting.  The interpretation
//! of the values (solute types, defaults, validation) is left to the
//! caller.
class US_MPI_JobConfig
{
   public:
      //! \brief A value that may be unset (std::optional needs C++17; the
      //!        Qt5 build uses C++14)
      template <typename T> class Optional
      {
         public:
            Optional& operator=( const T& value )
            {
               val = value;
               set = true;
               return *this;
            }
            explicit operator bool() const
            {
               return set;
            }
            const T& operator*() const
            {
               return val;
            }
            T value_or( const T& other ) const
            {
               return set ? val : other;
            }

         private:
            bool set = false;
            T    val{};
      };

      //! \brief A GA bucket; limits rounded to 7 significant digits
      class Bucket
      {
         public:
            double      x_min = 0.0; //!< Minimum X value
            double      x_max = 0.0; //!< Maximum X value
            double      y_min = 0.0; //!< Minimum Y value
            double      y_max = 0.0; //!< Maximum Y value
            QStringList unreadable;  //!< Limit attributes missing or not a number
      };

      //! \brief The job parameters as us_mpi_analysis uses them.  A value
      //!        not given, empty or not a number reads as 0 unless a
      //!        default is noted; validate() reports values that are not
      //!        numbers for the parameters of the job's method.
      class Parameters
      {
         public:
            //! \brief 2DSA grid and PCSA limits
            class Grid
            {
               public:
                  double           s_min        = 0.0; //!< s minimum (S)
                  double           s_max        = 0.0; //!< s maximum (S)
                  double           ff0_min      = 0.0; //!< f/f0 minimum
                  double           ff0_max      = 0.0; //!< f/f0 maximum
                  double           x_min        = 0.0; //!< x minimum (s_min if given as s)
                  double           x_max        = 0.0; //!< x maximum (s_max if given as s)
                  double           y_min        = 0.0; //!< y minimum (ff0_min if given as f/f0)
                  double           y_max        = 0.0; //!< y maximum (ff0_max if given as f/f0)
                  int              uniform_grid = 0;   //!< Grid repetitions
                  Optional<double> s_grid_points;      //!< s grid points
                  Optional<double> s_resolution;       //!< s points per repetition
                  Optional<double> ff0_grid_points;    //!< f/f0 grid points
                  Optional<double> ff0_resolution;     //!< f/f0 points per repetition

                  //! \brief Grid repetitions, at least 1
                  int              repetitions() const
                  {
                     return qMax( uniform_grid, 1 );
                  }
                  //! \brief s points: s_grid_points, else s_resolution times
                  //!        the repetitions, else 60
                  double s_points() const
                  {
                     return s_grid_points ? *s_grid_points : s_resolution ? *s_resolution * repetitions() : 60.0;
                  }
                  //! \brief f/f0 points: ff0_grid_points, else ff0_resolution
                  //!        times the repetitions, else 60
                  double ff0_points() const
                  {
                     return ff0_grid_points ? *ff0_grid_points : ff0_resolution ? *ff0_resolution * repetitions() : 60.0;
                  }
            };

            //! \brief GA and DMGA parameters
            class Ga
            {
               public:
                  int     population     = 0;
                  int     generations    = 0;
                  int     crossover      = 0; //!< Percent
                  int     mutation       = 0; //!< Percent
                  int     plague         = 0; //!< Percent
                  int     migration      = 0; //!< Percent
                  int     elitism        = 0;
                  double  mutate_sigma   = 2.0; //!< 2.0 if not given
                  int     p_mutate_s     = 20;  //!< 20 if not given
                  int     p_mutate_k     = 20;  //!< 20 if not given
                  int     p_mutate_sk    = 20;  //!< 20 if not given
                  double  regularization = 0.0;
                  double  conc_threshold = 0.0;
                  int     minimize_opt   = 0;
                  int     s_grid         = 100; //!< 100 if not given
                  int     k_grid         = 100; //!< 100 if not given
                  int     p_grid         = 100; //!< 100 if not given (DMGA)
                  double  bucket_fixed   = 0.0; //!< Fixed attribute value
                  QString xtype;                //!< Bucket x attribute
                  QString ytype;                //!< Bucket y attribute
                  QString ztype;                //!< Fixed attribute
            };

            //! \brief PCSA parameters
            class Pcsa
            {
               public:
                  QString curve_type;
                  int     vars_count      = 0;
                  int     curves_points   = 0;
                  double  thr_deltr_ratio = 0.0;
                  int     tikreg_option   = 0;
                  double  tikreg_alpha    = 0.0;
            };

            int               mc_iterations    = 0;
            int               max_iterations   = 0;
            int               gfit_iterations  = 0;
            int               montecarlo_value = 0;
            int               fit_mb_select    = 0;
            double            meniscus_range   = 0.0;
            int               meniscus_points  = 0;
            int               tinoise_option   = 0;
            int               rinoise_option   = 0;
            bool              global_fit       = false; //!< "1" or "T"
            uint              seed             = 0;
            QString           req_mgroupcount; //!< Requested master groups, as given
            double            z_value = 0.0;   //!< "0.0" if a job element is given
            QString           solute_type;     //!< PCSA/2DSA solute type ("013.skv")
            Optional<QString> cg_model;        //!< Custom grid model file, if given
            Optional<QString> dc_model;        //!< DMGA constraints model file, if given
            Grid              grid;
            Ga                ga;
            Pcsa              pcsa;
      };

      //! \brief The input of one dataset.  Values the file does not give are
      //!        unset; where an element occurs more than once the last wins.
      class DatasetSpec
      {
         public:
            QString                                        auc_file;    //!< Raw AUC file name
            QString                                        edit_file;   //!< Edit file name, as given
            Optional<QString>                              tmst_file;   //!< TimeState file name, as given
            QStringList                                    noise_files; //!< Noise file names

            Optional<double>                               density;              //!< Buffer density
            Optional<double>                               viscosity;            //!< Buffer viscosity
            Optional<double>                               compress;             //!< Buffer compressibility
            Optional<bool>                                 manual;               //!< Manual density/viscosity
            QList<US_Solution::AnalyteInfo>                analytes;             //!< Solution analytes
            bool                                           has_solution = false; //!< A solution element was read

            Optional<int>                                  simpoints;          //!< Simulation points
            Optional<double>                               band_volume;        //!< Band loading volume
            Optional<int>                                  mesh_type;          //!< Radial grid type
            Optional<int>                                  grid_type;          //!< Time grid type
            Optional<double>                               rotor_stretch[2];   //!< Rotor stretch coefficients
            Optional<double>                               centerpiece_bottom; //!< Centerpiece bottom
            Optional<int>                                  cp_sector;          //!< Centerpiece shape index
            Optional<double>                               cp_angle;           //!< Centerpiece angle
            Optional<double>                               cp_pathlen;         //!< Centerpiece path length
            Optional<double>                               cp_width;           //!< Centerpiece width

            QVector<US_SimulationParameters::SpeedProfile> speed_steps;          //!< Speed steps
            QVector<double>                                total_concentrations; //!< total_concentration values

            //! \brief The edit file name to load; for MWL data the wavelength
            //!        range is combined with the wavelength of the AUC file
            QString                                        edit_file_name() const;
      };

      //! \brief Read a job description
      //! \param device  Open device with the XML text
      //! \param source  Name of the source for error messages
      static US_MPI_JobConfig fromXml( QIODevice& device, const QString& source );

      //! \brief Read a job description from a file
      static US_MPI_JobConfig fromFile( const QString& path );

      //! \brief True if the description was read without errors
      bool                    ok() const
      {
         return errors.isEmpty();
      }

      //! \brief The debug level (debug_level parameter, 0 if not given)
      int                          debug_level() const;

      //! \brief The debug timings flag (debug_timings parameter)
      bool                         debug_timings() const;

      //! \brief The comma-separated entries of the debug_text parameter
      QStringList                  debug_text() const;

      //! \brief Look up an entry of debug_text, "key=value" or "key value"
      //! \param key    The entry key (matched exactly)
      //! \param found  Set to true if the key is present
      //! \returns      The value; empty for an entry without value
      QString                      debug_value( const QString& key, bool* found = nullptr ) const;

      //! \brief Check the job: method, parameter values and their ranges,
      //!        the parameters the method requires, buckets and datasets.
      //!        Problems are appended to errors; unknown parameters to
      //!        warnings.  An empty value counts as not given.
      void                         validate();

      //! \brief The analysis method families us_mpi_analysis runs; a method
      //!        name starts with one of them (e.g. 2DSA_CG)
      static QStringList           method_families();

      //! \brief Datasets as us_mpi_analysis uses them: a timestate file not
      //!        given is taken from the previous dataset, the edit file name
      //!        is completed for MWL data, vbar20 is computed from the
      //!        solution.  The caller owns the returned objects.
      QList<US_SolveSim::DataSet*> make_datasets() const;

      QString                      method;        //!< Analysis method (US_JobSubmit method)
      QString                      cluster;       //!< Cluster short name
      QString                      db_name;       //!< Database name
      QString                      udp_server;    //!< UDP status server address
      quint16                      udp_port = 0;  //!< UDP status server port
      QString                      request_id;    //!< Request ID, six digits
      QString                      request_guid;  //!< Request GUID
      Optional<int>                dataset_count; //!< datasetCount, if given

      //! Job parameters as strings by name.  s_min/s_max and ff0_min/ff0_max
      //!  are also stored as x_min/x_max and y_min/y_max; bucket_fixed sets
      //!  xtype, ytype and ztype; the first bucket sets ytype if not yet set;
      //!  a job element sets z_value to "0.0" if not given.
      QMap<QString, QString>       parameters;
      QList<Bucket>                buckets;  //!< GA buckets
      QList<DatasetSpec>           datasets; //!< Datasets in file order
      Parameters                   params;   //!< Typed job parameters
      QStringList                  errors;   //!< Errors found while reading
      QStringList                  warnings; //!< Problems that do not stop a run

   private:
      void read_parameters( void );
      void read_job( QXmlStreamReader& );
      void read_job_parameters( QXmlStreamReader& );
      void read_bucket( const QXmlStreamAttributes&, QString& );
      void read_dataset( QXmlStreamReader&, DatasetSpec& );
      void read_files( QXmlStreamReader&, DatasetSpec& );
      void read_solution( QXmlStreamReader&, DatasetSpec& );
};

#endif
