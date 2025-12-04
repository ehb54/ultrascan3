//! \file us_convertio.h
#ifndef US_CONVERTIO_H
#define US_CONVERTIO_H

#include <QtCore>

#include "us_extern.h"
#include "us_convert.h"
#include "us_experiment.h"
#include "us_db2.h"

struct cellInfo
{
  QString     cellName;
  QString     channelName;
  int         centerpieceID;
};

/*! \class US_ConvertIO
           This class provides the ability to save converted US3
           data to the disk and the database.
           All methods are static.
*/
class US_ConvertIO 
{
   public:
      // \brief Generic constructor for the US_ConvertIO class.
      US_ConvertIO( void );

      /*! \brief Reads entire experiment and auc files from the database,
                 save to HD
          \param runID      The run ID to look up in the database
          \param dir        The location where the binary auc files are to go.
          \param db         An opened db connection
          \param speedsteps Reference for returned experiment speed steps vector
      */
      static QString readDBExperiment( QString, QString, US_DB2*,
				       QVector< SP_SPEEDPROFILE >&, const QString = QString("") );

      /*! \brief Writes a new DB rawData record for each triple

          \param ExpData A reference to a structure provided by the calling
                         function that contains the hardware and other database
                         connection information provided by the xml file.
          \param triples A reference to a structure provided by the calling
                         function that contains all the different
                         cell/channel/wavelength defined by the xml file.
          \param dir     Local disk directory where auc files can be found
          \param db      An opened db connection
      */
      static QString writeRawDataToDB(
                 US_Experiment& , 
                 QList< US_Convert::TripleInfo >& ,
                 const QString&,
                 US_DB2* = 0 );

      /*! \brief Checks some info that was read from disk with values from DB

          \param ExpData A reference to a structure provided by the calling
                         function that contains the hardware and other database
                         connection information provided by the xml file.
          \param triples A reference to a structure provided by the calling
                         function that contains all the different
                         cell/channel/wavelength defined by the xml file.
          \param db      An opened db connection
      */
      static int checkDiskData( 
                 US_Experiment&,
                 QList< US_Convert::TripleInfo >& ,
                 US_DB2* = 0 );
      static int checkDiskData_auto( 
                 US_Experiment&,
                 QList< US_Convert::TripleInfo >& ,
                 US_DB2* = 0 );

   private:
      static QString readRawDataFromDB( 
                 US_Experiment& , 
                 QList< US_Convert::TripleInfo >& ,
                 QString& ,
                 US_DB2* = 0 );
      
};
#endif
