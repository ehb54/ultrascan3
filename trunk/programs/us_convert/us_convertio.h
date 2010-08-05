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

      /*! \brief    Save new experiment info in the database

          \param    ExpData A structure containing all the experiment information
          \param    tripleMap A reference to a QList of index values indicating
                        which triples are members of the current data set. Triples
                        included in the map will be written, others will not.
          \param    dir     The location of the binary auc files
      */
      static QString newDBExperiment( US_ExpInfo::ExperimentInfo&, 
                                      QList< int >& ,
                                      QString );

      /*! \brief    Update experiment info in the database

          \param    ExpData A structure containing all the experiment information
          \param    tripleMap A reference to a QList of index values indicating
                        which triples are members of the current data set. Triples
                        included in the map will be written, others will not.
          \param    dir     The location of the binary auc files
      */
      static QString updateDBExperiment( US_ExpInfo::ExperimentInfo&, 
                                         QList< int >& ,
                                         QString );

      /*! \brief    Writes an xml file

          \param ExpData A reference to a structure provided by the calling function
                         that already contains the hardware and other database
                         connection information relevant to this experiment.
          \param triples A reference to a structure provided by the calling
                        function that already contains all the different
                        cell/channel/wavelength combinations in the data. 
          \param tripleMap A reference to a QList of index values indicating
                        which triples are members of the current data set. Triples
                        included in the map will be written, others will not.
          \param runType A reference to a variable that already contains the type
                        of data ( "RA", "IP", "RI", "FI", "WA", or "WI").
                        This information will affect how the data is
                        written.
          \param runID  The run ID of the experiment.
          \param dirname The directory in which the files are to be written.
      */
      static int writeXmlFile( 
                 US_ExpInfo::ExperimentInfo& ExpData,
                 QStringList& ,
                 QList< int >& ,
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
                 QStringList& ,
                 QString ,
                 QString ,
                 QString );

   private:
      static void readExperiment( 
                 QXmlStreamReader& , 
                 US_ExpInfo::ExperimentInfo& ,
                 QStringList& ,
                 QString ,
                 QString );

      static void readDataset( 
                 QXmlStreamReader& , 
                 US_ExpInfo::TripleInfo& );

      static int verifyXml( 
                 US_ExpInfo::ExperimentInfo& );

      static QString writeRawDataToDB(
                 US_ExpInfo::ExperimentInfo& , 
                 QList< int >& ,
                 QString );
      
};
#endif
