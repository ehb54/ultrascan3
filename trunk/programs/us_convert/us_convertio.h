//! \file us_convertio.h
#ifndef US_CONVERT_IO_H
#define US_CONVERT_IO_H

#include <QtCore>

#include "us_extern.h"
#include "us_dataIO2.h"
#include "us_help.h"
#include "us_convert.h"
#include "us_expinfo.h"

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

      /*! \brief    Determine if the experiment exists in the DB. Returns
                    -1 if there is a database connection error, 0 if the
                    run ID doesn't match any records in the DB, and a positive
                    value is the experimentID itself

          \param    runID The run ID to check
      */
      static int checkRunID( QString runID );

      /*! \brief    Save new experiment info in the database

          \param    ExpData A structure containing all the experiment information
          \param    triples A reference to a structure provided by the calling
                        function that will contain all the different
                        cell/channel/wavelength information.
          \param    dir     The location of the binary auc files
      */
      static QString newDBExperiment( US_ExpInfo::ExperimentInfo&, 
                                      QList< US_ExpInfo::TripleInfo >& ,
                                      QString );

      /*! \brief    Update experiment info in the database

          \param    ExpData A structure containing all the experiment information
          \param    triples A reference to a structure provided by the calling
                        function that will contain all the different
                        cell/channel/wavelength information.
          \param    dir     The location of the binary auc files
      */
      static QString updateDBExperiment( US_ExpInfo::ExperimentInfo&, 
                                         QList< US_ExpInfo::TripleInfo >& ,
                                         QString );

      /*! \brief    Reads entire experiment and auc files from the database, save to HD

          \param    The run ID to look up in the database
          \param    dir     The location where the binary auc files are to go.
      */
      static QString readDBExperiment( QString,
                                       QString );

      /*! \brief    Reads secondary experiment info from the database. Call 
                    this function when you already have IDs stored and
                    want to fill out with GUIDs serial numbers and the like.
                    For instance, after loading the xml file to fill in the gaps,
                    or to load experiment info after reading
                    auc files.

          \param    expInfo A structure that contains the experiment information
      */
      static QString readExperimentInfoDB( US_ExpInfo::ExperimentInfo& );

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
                 US_ExpInfo::ExperimentInfo& ExpData,
                 QList< US_ExpInfo::TripleInfo >& ,
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
                 US_ExpInfo::ExperimentInfo& ,
                 QList< US_ExpInfo::TripleInfo >& ,
                 QString ,
                 QString ,
                 QString );

   private:
      static void readExperiment( 
                 QXmlStreamReader& , 
                 US_ExpInfo::ExperimentInfo& ,
                 QList< US_ExpInfo::TripleInfo >& ,
                 QString ,
                 QString );

      static void readDataset( 
                 QXmlStreamReader& , 
                 US_ExpInfo::TripleInfo& );

      static int verifyXml( 
                 US_ExpInfo::ExperimentInfo&,
                 QList< US_ExpInfo::TripleInfo >& );

      static QString writeRawDataToDB(
                 US_ExpInfo::ExperimentInfo& , 
                 QList< US_ExpInfo::TripleInfo >& ,
                 QString );

      static QString readRawDataFromDB( 
                 US_ExpInfo::ExperimentInfo& , 
                 QList< US_ExpInfo::TripleInfo >& ,
                 QString& );
      
};
#endif
