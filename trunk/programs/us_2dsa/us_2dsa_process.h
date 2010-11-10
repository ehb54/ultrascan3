//! \file us_2dsa_process.h
#ifndef US_2DSA_PROCESS_H
#define US_2DSA_PROCESS_H

#include <QtCore>

#include "us_extern.h"
#include "us_dataIO2.h"
#include "us_model.h"
#include "us_noise.h"
#include "us_db2.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

//! \brief Solute triples input/output for 2DSA analysis

/*! \class Solute
 *
    This class represents the s,k,c values of a solute that is
    input to or output from the 2DSA analysis.
*/
class Solute : public QObject
{
   Q_OBJECT

   public:
      double s;     // sedimentation coefficent of solute
      double k;     // frictional ratio (f/f0) of solute
      double c;     // concentration of solute

      // Solute constructor:  set s,k,c
      Solute( double s0 = 0.0, double k0 = 0.0, double c0 = 0.0 )
      {
         s = s0;
         k = k0;
         c = c0;
      }

      bool operator== ( const Solute& solute )
      {
         return ( s == solute.s  &&  k == solute.k );
      }

      bool operator!= ( const Solute& solute )
      {
         return ( s != solute.s  ||  k != solute.k );
      }

      bool operator<  ( const Solute& solute )
      {
         return ( s < solute.s  ||  ( s == solute.s && k < solute.k ) );
      }

};

//! \brief Worker thread job-defining arguments
typedef struct work_define_s
{
   double  ll_s;       // grid lower-limit s
   double  ul_s;       // grid upper-limit s
   double  delta_s;    // grid delta in s
   double  ll_k;       // grid lower-limit k
   double  ul_k;       // grid upper-limit k
   double  delta_k;    // grid delta in k
   int     thrx;       // thread index (1,...)
   int     taskx;      // task index (1,...)
   US_DataIO2::EditedData*  edata;    // pointer to experiment data
} WorkDefine;

//! \brief Worker thread results-output arguments
typedef struct work_results_s
{
   int    thrx;
   int    taskx;
   double ll_s;
   double ll_k;

   QVector< Solute* >  csolutes;  // computed solutes

   US_DataIO2::RawData sdata;     // simulation data
   US_DataIO2::RawData rdata;     // residuals data
   US_Model            model;     // composite model data
} WorkResult;

//! \brief Worker thread to do actual work of 2DSA analysis

/*! \class WorkerThread
 *
    This class is for each of the individual worker threads that do the
    actual work of 2DSA analysis.
*/
class WorkerThread : public QThread
{
   Q_OBJECT

   public:
      WorkerThread( QObject* parent = 0 );
      ~WorkerThread();

      void define_work( WorkDefine& );
      void get_result(  WorkResult& );
      void run();

   signals:
      void work_progress( int );
      void work_complete( WorkerThread* );

   private:
      QMutex mutex;
      QWaitCondition condition;

      double  llim_s;
      double  ulim_s;
      double  delta_s;
      double  llim_k;
      double  ulim_k;
      double  delta_k;

      int     nscan;
      int     nconc;
      int     ngls;
      int     nglk;
      int     ngltot;
      int     thrx;
      int     taskx;
      int     dbg_level;

      US_DataIO2::EditedData* edata;
      US_DataIO2::RawData     sdata;
      US_DataIO2::RawData     rdata;
      US_Model                model;
      US_Model                omodel;
      US_Noise                ri_noise;
      US_Noise                ti_noise;
      US_Noise                ra_noise;
      QVector< Solute* >      solute_i;
      QVector< Solute* >      solute_c;
};

//! \brief 2DSA Simulation object

/*! \class US_2dsaProcess
 *
    This class sets up a set of 2DSA simulations for a
    grid across an s and k range. It divides the refinements
    in the grid across a specified number of worker threads.
*/
class US_EXTERN US_2dsaProcess : public QObject
{
   Q_OBJECT

   public:
      //! \brief Create a 2DSA processor object
      //! \param da_exper  Pointer to input experiment data
      //! \param parent    Pointer to parent object
      US_2dsaProcess( US_DataIO2::EditedData*, QObject* = 0 );

      //! \brief Start the fit calculations
      //! \param sll     s lower limit
      //! \param sul     s upper limit
      //! \param nss     number of s steps
      //! \param kll     k lower limit
      //! \param kul     k upper limit
      //! \param nks     number of k steps
      //! \param ngf     number of grid refinements
      //! \param nthr    number of threads
      //! \param noif    noise flag: 0-3 for none|ti|ri|both
      void start_fit( double, double, int, double, double, int,
                      int, int, int );

      //! \brief Get results upon completion of all refinements
      //! \param da_sim  Calculated simulation data
      //! \param da_res  Residuals data (exper - simul)
      //! \param da_mdl  Composite model
      //! \param da_tin  Time-invariant noise (or null)
      //! \param da_rin  Radially-invariant noise (or null)
      //! \returns       Success flag:  true if successful
      bool get_results( US_DataIO2::RawData*, US_DataIO2::RawData*,
                        US_Model*, US_Noise*, US_Noise* );

      //! \brief Get message for last error
      //! \returns       Message about last error
      QString lastError( void ) { return errMsg; }

      static const int solute_doubles = sizeof( Solute ) / sizeof( double );
      QVector< Solute > c_solutes;  // calculated solutes

      class Simulation
      {
         public:
            double variance;
            QVector< double > variances;
            QVector< double > ti_noise;
            QVector< double > ri_noise;
            QVector< Solute > solutes;
      };

      public slots:
      void thread_finished( WorkerThread* );
      void step_progress( int );

      signals:
      void progress_update(   int     );
      void refine_complete(   int     );
      void subgrids_complete( void    );
      void process_complete(  void    );
      void message_update(    QString );

      private:

      QList< WorkerThread* > wthreads;    // worker threads
      QList< WorkDefine >    workdefs;
      QList< WorkResult >    workouts;

      US_DataIO2::EditedData* edata;      // experimental data

      US_DataIO2::RawData     sdata;      // simulation data

      US_DataIO2::RawData     rdata;      // residuals data

      US_Model                model;      // constructed model

      US_Noise                ti_noise;   // time-invariant noise
      US_Noise                ri_noise;   // radially-invariant noise

      QObject*   parentw;      // parent object

      QString    errMsg;       // message from last error

      int        dbg_level;    // debug level
      int        nthreads;     // number of worker threads
      int        nssteps;      // number of s steps
      int        nksteps;      // number of k steps
      int        ngrefine;     // number of grid refinements
      int        ntpsteps;     // number of total progress steps
      int        kcpsteps;     // count of completed progress steps
      int        noisflag;     // noise out flag: 0(none), 1(ti), 2(ri), 3(both)
      int        nscans;       // number of experiment scans
      int        npoints;      // number of reading points per experiment scan
      int        nsubgrid;     // number of subgrids (tasks)
      int        kctask;       // count of completed subgrid tasks
      int        kstask;       // count of started subgrid tasks;

      double     slolim;       // s lower limit
      double     suplim;       // s upper limit
      double     klolim;       // k lower limit
      double     kuplim;       // k upper limit
      double     gdelta_s;     // grid delta in s
      double     gdelta_k;     // grid delta in k
      double     sdelta_s;     // subgrid delta in s
      double     sdelta_k;     // subgrid delta in k

      QString get_model_filename( QString );
      QString get_noise_filename( QString );

};
#endif

