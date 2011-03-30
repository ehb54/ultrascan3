//! \file us_convertio.h
#ifndef US_CONVERT_IO_H
#define US_CONVERT_IO_H

#include <QtCore>

#include "us_extern.h"
#include "us_help.h"
#include "us_convert.h"
#include "us_experiment.h"

/*! \class US_ConvertIO
           This class provides the ability to save converted US3
           data to the disk and the database.
           All methods are static.
*/
class US_EXTERN US_ConvertIO 
{
   public:
      // \brief Generic constructor for the US_ConvertIO class.
      US_ConvertIO( void );

      /*! \brief    Reads entire experiment and auc files from the database, save to HD

          \param    The run ID to look up in the database
          \param    dir     The location where the binary auc files are to go.
      */
      static QString readDBExperiment( QString,
                                       QString );

      /*! \brief    Writes an xml file

          \param ExpData A reference to a structure provided by the calling function
                         that already contains the hardware and other database
                         connection information relevant to this experiment.
          \param triples A reference to a structure provided by the calling
                        function that already contains all the different
                        cell/channel/wavelength combinations in the data. 
          \param runType A reference to a variable that already contains the type
                        of data ( "RA", "IP", "RI", "FI", "WA", or "WI").
                        This information will affect how the data is
                        written.
          \param runID  The run ID of the experiment.
          \param dirname The directory in which the files are to be written.
      */
      static int writeXmlFile( 
                 US_Experiment& ExpData,
                 QList< US_Convert::TripleInfo >& ,
                 QString ,
                 QString ,
                 QString );

      /*! \brief    Reads an xml file

          \param ExpData A reference to a structure provided by the calling function
                         that will contain the hardware and other database
                         connection information provide by the xml file.
          \param triples A reference to a structure provided by the calling
                        function that will contain all the different
                        cell/channel/wavelength defined by the xml file.
          \param runType A reference to a variable that will contain the type
                        of data ( "RA", "IP", "RI", "FI", "WA", or "WI").
                        This information will affect how the data is
                        stored.
          \param runID  The run ID of the experiment.
          \param dirname The directory from which the files are read.
      */
      static int readXmlFile( 
                 US_Experiment& ,
                 QList< US_Convert::TripleInfo >& ,
                 QString ,
                 QString ,
                 QString );

      static QString writeRawDataToDB(
                 US_Experiment& , 
                 QList< US_Convert::TripleInfo >& ,
                 QString );

      /*! \brief    Checks some info that was read from disk with values from DB

          \param ExpData A reference to a structure provided by the calling function
                         that contains the hardware and other database
                         connection information provided by the xml file.
          \param triples A reference to a structure provided by the calling
                        function that contains all the different
                        cell/channel/wavelength defined by the xml file.
      */
      static int checkDiskData( 
                 US_Experiment&,
                 QList< US_Convert::TripleInfo >& );

   private:
      static void readExperiment( 
                 QXmlStreamReader& , 
                 US_Experiment& ,
                 QList< US_Convert::TripleInfo >& ,
                 QString ,
                 QString );

      static void readDataset( 
                 QXmlStreamReader& , 
                 US_Convert::TripleInfo& );

      static QString readRawDataFromDB( 
                 US_Experiment& , 
                 QList< US_Convert::TripleInfo >& ,
                 QString& );
      
};
#endif
