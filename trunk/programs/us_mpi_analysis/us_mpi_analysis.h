#ifndef US_MPI_ANALYSIS_H
#define US_MPI_ANALYSIS_H

#include <QtCore>
#include <QtNetwork>
#include <mpi.h>

#include "us_model.h"
#include "us_dataIO2.h"

#define MPI_COMM_WORLD2  (((MPI_Comm)(void*)&(ompi_mpi_comm_world)))
#define MPI_BYTE2        (((MPI_Datatype)(void*)&(ompi_mpi_byte)))
#define MPI_DOUBLE2      (((MPI_Datatype)(void *)&(ompi_mpi_double)))
#define MPI_INT2         (((MPI_Datatype)(void *)&(ompi_mpi_int)))

class US_MPI_Analysis : public QObject
{
 Q_OBJECT

  public:
    US_MPI_Analysis( const QString& );

    static void debug( const QString& );

  public slots:
    void start( void );
     
  private:
    int          node_count;
    int          my_rank;
    int          iterations;
    long int     maxrss;

    QHostAddress server;
    quint16      port;
    QUdpSocket*  socket;
    
    QString      requestID;
    QString      directory;
    QString      analysis_type;
    QString      cluster;
    QString      db_name;

    QMap< QString, QString > parameters;
    
    class DataSet
    {
        public:
            QString                requestID;
            QString                auc_file;
            QString                edit_file;
            QString                model_file;
            QList< QString >       noise_files;
            US_DataIO2::EditedData run_data;
            US_Model               model;    
            int                    simpoints;
            double                 band_volume;
            int                    radial_grid;
            int                    time_grid;
            double                 vbar20;
            double                 viscosity;
            double                 density;
            double                 rotor_stretch[ 5 ];
            double                 centerpiece_bottom;
    };

    int               set_count;
    QList< DataSet* > data_sets;

    class MPI_Job
    {
        public:
            static const int MASTER = 0;
            static const int TAG3   = 3;

            enum Command { IDLE, PROCESS, WAIT, SHUTDOWN, WAKEUP_SENT };
            enum Status  { TAG0, READY, RESULTS };

            int     solution;
            int     length;
            Command command;
            int     depth;
            double  meniscus_offset;

            MPI_Job()
            {
                solution        = 0;
                length          = 0;
                command         = IDLE;
                depth           = 0;
                meniscus_offset = 0.0;
            };
    };

    class Solute
    {
       public:
          double s;
          double k;
          double c;
          
          Solute( double s0 = 0.0, double k0 = 0.0, double c0 = 0.0 )
          {
            s = s0;
            k = k0;
            c = c0;
          }

          bool operator== ( const Solute& solute )
          {
             return s == solute.s && k == solute.k;
          }
         
          bool operator!= ( const Solute& solute )
          {
             return s != solute.s || k != solute.k;
          }
         
          bool operator< ( const Solute& solute ) const
          {
             if ( s < solute.s )
                return true;
             
             else if (  s == solute.s && k < solute.k )
                return true;
             
             else
                return false;
          }
    };

    static const int solute_doubles = sizeof( Solute ) / sizeof( double );
    QVector< Solute > calculated_solutes;

    class Simulation
    {
      public:
         double variance;
         QVector< double > variances;
         QVector< double > ti_noise;
         QVector< double > ri_noise;
         QVector< Solute > solutes;
    };

    void     parse        ( const QString& );
    void     parse_job    ( QXmlStreamReader& );
    void     parse_dataset( QXmlStreamReader&, DataSet* );
    void     parse_files  ( QXmlStreamReader&, DataSet* );
    void     send_udp     ( const QString& );
    void     abort        ( const QString&, int=-1 );
    long int max_rss      ( void );

    QVector< Solute > create_solutes( double, double, double,
                                      double, double, double );

    // Master
    void     _2dsa_master      ( void );
    void     get_results       ( const MPI_Status&, const int* );
    void     write_2dsa        ( void );

    // Worker
    void     _2dsa_worker      ( void );
    void     _2dsa_worker_loop ( int, int );

    void     calc_residuals    ( int, int, Simulation& );
    double   calc_bottom       ( int, double );
};
#endif

