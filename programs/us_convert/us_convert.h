//! \file us_process_convert.h
#ifndef US_PROCESS_CONVERT_H
#define US_PROCESS_CONVERT_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_dataIO2.h"
#include "us_help.h"
#include "us_experiment.h"
#include "us_solution.h"

/*! \class US_Convert
           This class provides the ability to convert raw data in the
           Beckman format to the file format used by US3. 
*/
class US_EXTERN US_Convert // : public US_WidgetsDialog 
{
//  Q_OBJECT

   public:
      //! \brief   Some status codes returned by the us_convert program
      enum ioError
      { 
         OK,                                  //!< Ok, no error
         CANTOPEN,                            //!< The file cannot be opened
         DUP_RUNID,                           //!< The given run ID already exists
         NODATA,                              //!< There is no data to read or write
         NODB,                                //!< Connection to database cannot be made
         NOAUC,                               //!< AUC File cannot be opened
         NOXML,                               //!< XML data has not been entered
         BADXML,                              //!< XML not formed correctly
         BADGUID,                             //!< GUID read in the XML was not found in the database
         PARTIAL_XML,                         //!< XML data has not been entered for all c/c/w combinations
         NOT_WRITTEN                          //!< data was not written
      };

      //! \brief Class to contain a list of scans to exclude from a data set
      //!        for a single c/c/w combination
      class Excludes
      {
         public:
         QList< int >  excludes;              //!< list of scan indexes to exclude 
         bool contains ( int x )              //!< function to determine if x is contained in the list
           { return excludes.contains( x ); }
         bool empty    ()                     //!< function to determine if the list is empty
           { return excludes.empty(); }
         void push_back( int x )              //!< function to add x to the end of the list
           { excludes.push_back( x ); }
         Excludes& operator<<( const int x )  //!< function to insert x at the end of the list
            { this->push_back( x ); return *this; }

      };

      //! \brief  Class that contains information about relevant 
      //!         cell/channel/wavelength combinations
      class TripleInfo
      {
         public:
         int              tripleID;           //!< The ID of this c/c/w combination
         QString          tripleDesc;         //!< The description of this triple ( e.g., "2 / A / 260" )
         QString          description;        //!< A text description of this triple
         char             tripleGUID[16];     //!< The GUID of this triple
         QString          tripleFilename;     //!< The filename of this auc file
         bool             excluded;           //!< Whether this triple has been dropped or not
         int              centerpiece;        //!< The ID of the centerpiece that was used
         US_Solution      solution;           //!< The solution information for this triple
         TripleInfo();                        //!< A generic constructor
         void             clear( void );
         void             show( void );       // temporary
      };

      /*! \brief Generic constructor for the US_Convert class. This
                 constructor establishes the dialog and its relationship to
                 the parent dialog.

          \param parent Establishes the US_Convert dialog as a child
                        of the specified parent.
      */
      US_Convert( void );

      /*! \brief Reads the legacy raw data from disk into the program. 

          \param dir    The directory in which the program will look for the
                        raw data files.
          \param rawLegacyData A reference to a structure provided by the calling
                        function that will be used to store the imported raw data.
          \param runType A reference to a variable that will contain the type
                        of data that is being read ( "RA", "IP", "RI", "FI", "WA", or "WI").
                        This determination already affects how some data is
                        handled when read.
      */
      static void   readLegacyData( 
                    QString ,
                    QList< US_DataIO2::BeckmanRawScan >& ,
                    QString& );

      /*! \brief Converts legacy raw data into US3 data. 
                 This function will convert existing datapoints and cell/
                 channel/wavelength combinations in the data.

          \param rawLegacyData A reference to a structure provided by the calling
                        function that already contains the imported raw data.
          \param rawConvertedData A reference to a structure provided by the calling
                        function that will be used to store the US3 raw converted data.
          \param triples A reference to a structure provided by the calling
                        function that will be used to store all the different
                        cell/channel/wavelength combinations found in the data. 
          \param runType A reference to a variable that already contains the type
                        of data ( "RA", "IP", "RI", "FI", "WA", or "WI").
                        This information will affect how the data is
                        converted.
          \param tolerance How far apart the wavelength readings can be and still
                        be considered part of the same cell/channel/wavelength.
          \param ss_limits In the case of RA data only, sometimes the data needs
                        to be split into multiple datasets. In this case, ss_limits
                        should already contain a list of radius values that define
                        where one dataset ends and the next one begins.
      */
      static void   convertLegacyData(
                    QList  < US_DataIO2::BeckmanRawScan >& ,
                    QVector< US_DataIO2::RawData        >& ,
                    QList< TripleInfo >& ,
                    QString ,
                    double );

      /*! \brief Writes the converted US3 data to disk. 

          \param status  A reference to an integer that will contain the status of the
                         operation when it is completed.
          \param rawConvertedData A reference to a structure provided by the calling
                        function that already contains the US3 raw converted data.
          \param ExpData A reference to a structure provided by the calling function
                         that already contains the hardware and other database
                         connection information relevant to this experiment.
          \param triples A reference to a structure provided by the calling
                        function that already contains all the different
                        cell/channel/wavelength combinations in the data. 
          \param allExcludes A reference to a QVector of excluded scans for
                        each c/c/w combination
          \param runType A reference to a variable that already contains the type
                        of data ( "RA", "IP", "RI", "FI", "WA", or "WI").
                        This information will affect how the data is
                        written.
          \param runID  The run ID of the experiment.
          \param dirname The directory in which the files are to be written.
          \param saveGUIDs Boolean value that indicates whether data has been saved
                        to disk before and doesn't require new GUIDs
      */
      static void   writeConvertedData(
                    int& status,
                    QVector< US_DataIO2::RawData >& ,
                    US_Experiment& ,
                    QList< TripleInfo >& ,
                    QVector< Excludes >& ,
                    QString ,
                    QString ,
                    QString,
                    bool );

      /*! \brief Reloads converted US3 data back into the program to sync
                 up with the database. 

          \param dir    The directory that contains the auc files
          \param rawConvertedData A reference to a structure provided by the calling
                        function that will be used to store the US3 raw converted data.
          \param ExpData A reference to a structure provided by the calling function
                         that will contain the hardware and other database
                         connection information relevant to this experiment.
          \param triples A reference to a structure provided by the calling
                        function that will be used to store all the different
                        cell/channel/wavelength combinations found in the data. 
          \param runType A reference to a variable that already contains the type
                        of data ( "RA", "IP", "RI", "FI", "WA", or "WI").
                        This information will affect how the data is
                        converted.
          \returns      An ioError status code
      */
      static int    reloadUS3Data(
                    QString ,
                    QVector< US_DataIO2::RawData        >& ,
                    US_Experiment&,
                    QList< TripleInfo >& ,
                    QString&,
                    QString );

      /*! \brief Splits existing raw converted data into multiple datasets. 
                 Also rearranges triples in the data accordingly.

          \param rawConvertedData A reference to a structure that contains
                        the US3 raw converted data.
          \param triples A reference to a structure that contains the different
                        cell/channel/wavelength combinations in the data. 
          \param currentTriple The triple that will be split.
          \param subsets A list of radius limits that define where a dataset 
                         begins and ends.
      */
      static void splitRAData ( QVector< US_DataIO2::RawData >& ,
                                QList< TripleInfo >& ,
                                int ,
                                QList< double >& );

   private:
      US_Help            showHelp;

      static void convert( QList< US_DataIO2::BeckmanRawScan >& rawLegacyData,
                           US_DataIO2::RawData&          newRawData,
                           QString                       triple, 
                           QString                       runType,
                           double                        tolerance );

      static void setTriples ( QList< US_DataIO2::BeckmanRawScan >& rawLegacyData,
                           QList< TripleInfo >& triples,
                           QString                              runType,
                           double                               tolerance );
      
      static void setCcwTriples ( QList< US_DataIO2::BeckmanRawScan >& rawLegacyData,
                           QList< TripleInfo >& triples,
                           double                               tolerance );
      
      static void setCcrTriples ( QList< US_DataIO2::BeckmanRawScan >& rawLegacyData,
                           QList< TripleInfo >& triples,
                           double                               tolerance );
      
      static void setInterpolated ( unsigned char*, int );
      
      static US_DataIO2::Scan newScanSubset(
                           US_DataIO2::Scan& oldScan,
                           QVector< US_DataIO2::XValue >& radii, 
                           double r_start,
                           double r_end );

   private slots:
};
#endif
