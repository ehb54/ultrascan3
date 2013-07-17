//! \file us_model_record.h
#ifndef US_MODEL_RECORD_H
#define US_MODEL_RECORD_H

#include <QtCore>

#include "us_extern.h"
#include "us_solute.h"
#include "us_dataIO.h"
#include "us_model.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

//! \brief Model_Record object

/*! \class ModelRecord
 *
    This class sets up a record of a model used in the set
    of models defined by curves in PCSA. A vector of such objects
    can be sorted and evaluated in analysis and display of a PCSA run.
*/
class ModelRecord
{
   public:
      //! Constructor for model record class
      ModelRecord();

      ~ModelRecord();

      int                  taskx;      // Task index (submit order)
      int                  ctype;      // Curve type flag (0-2=>SL,IS,DS)
      double               str_k;      // Start k value
      double               end_k;      // End k value
      double               par1;       // Sigmoid par1 value
      double               par2;       // Sigmoid par2 value
      double               variance;   // Variance value
      double               rmsd;       // RMSD value
      double               smin;       // Minimum s value
      double               smax;       // Minimum s value
      double               kmin;       // Minimum k value
      double               kmax;       // Minimum k value
      QVector< US_Solute > isolutes;   // Input solutes
      QVector< US_Solute > csolutes;   // Computed solutes
      QVector< double >    ti_noise;   // Computed TI noise
      QVector< double >    ri_noise;   // Computed RI noise

      US_Model             model;      // Computed model
      US_DataIO::RawData   sim_data;   // Simulation data from this fit
      US_DataIO::RawData   residuals;  // Residuals data from this fit

      //! A test for ordering model descriptions. Sort by variance.
      bool operator< ( const ModelRecord& mrec ) const
      {
         return ( variance < mrec.variance );
      }

   public slots:
      //! A public slot to clear data vectors (sim_data, residuals, *noise)
      void clear_data( void );

      //! Static public function to compute straight line model records
      static int compute_slines( double&, double&, double&, double&, double&,
            int&, QVector< ModelRecord >& );

      //! Static public function to compute straight line model records
      static int compute_sigmoids( int&, double&, double&, double&, double&,
            int&, int&, QVector< ModelRecord >& );

};
#endif

