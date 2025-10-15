//! \file us_solution.h
#ifndef US_SOLUTION_H
#define US_SOLUTION_H

#include "us_extern.h"
#include "us_db2.h"
#include "us_buffer.h"
#include "us_analyte.h"

/*! \class US_Solution
           This class provides the ability to associate the solution
           parameters, buffers, and analytes with the appropriate
           cell / channel / wavelength combination in the experiment.
*/
           
class US_UTIL_EXTERN US_Solution
{
   public:

      //! Class that contains information about the individual analytes
      class US_UTIL_EXTERN AnalyteInfo
      {
         public:
         US_Analyte    analyte;           //!< The analyte information
         double        amount;            //!< The amount of the analyte in the solution

         //! Constructor for the AnalyteInfo class
         AnalyteInfo();

         //! Test for equality
         bool          operator==( const AnalyteInfo& ) const;

      };

      //! \brief   Some status codes to keep track of where solution data has been saved to
      enum solutionStatus
      {
         NOT_SAVED,                           //!< The file has not been saved
         EDITING,                             //!< Data is being edited; certain operations not permitted
         HD_ONLY,                             //!< The file has been saved to the HD
         DB_ONLY,                             //!< The file has been saved to the DB
         BOTH                                 //!< The file has been saved to both HD and DB
      };

      int              solutionID;         //!< The ID of the solution for this triple
      QString          solutionGUID;       //!< The GUID of the solution for this triple
      QString          solutionDesc;       //!< A description of the solution 
      US_Buffer        buffer;             //!< The associated buffer information
      QList< AnalyteInfo > analyteInfo;    //!< A list of the analyte information
      double           commonVbar20;       //!< The weighted average vbar of all analytes present ( 20 degrees C)
      double           storageTemp;        //!< The temperature that the solution was stored
      QString          notes;              //!< Notes on the channel solution
      solutionStatus   saveStatus;         //!< Most recent save status

      bool             replace_spectrum;
      bool             new_or_changed_spectrum;
      
      //! extinction[ wavelength ] <=> value
      QMap< double, double > extinction;

      //! \brief Generic constructor for the US_Solution class.
      US_Solution();

      //! A null destructor. 
      ~US_Solution() {};

      /*! \brief    Function to read an entire solution structure from the disk

          \param    guid The GUID of the solution to look for
          \return   One of the IUS_DB2 error codes
      */
      int readFromDisk( QString& );

      /*! \brief    Function to read an entire solution structure from the DB

          \param    solutionID The database solutionID of the desired solution
          \param    db For database access, an open database connection
          \return   One of the IUS_DB2 error codes
      */
      int readFromDB  ( int, IUS_DB2* = 0 );

      //! \brief    Quick method to zero out the solution attributes
      void clear             ( void );

      //! \brief    Method to save the current solution to disk
      void saveToDisk        ( void );

      /*! \brief    Function to save the solution information to db

          \param    expID The ID of the experiment this solution is associated with
          \param    channelID The ID of the channel
          \param    db For database access, an open database connection
      */
      int saveToDB           ( int = 1, int = 1, IUS_DB2* = 0 );

      //! \brief    Function to delete the current solution from disk, if it exists
      int deleteFromDisk    ( void );

      /*! \brief    Function to delete the current solution from the db, if it exists

          \param    db For database access, an open database connection
      */
      int deleteFromDB      ( IUS_DB2* = 0 );

      /*! \brief    Function to find the filename of a solution on disk, if it exists.
                    Returns true if successful, false otherwise

          \param    guid The GUID of the solution to look for on disk
          \param    filename The function will return the filename here if it is found
      */
      bool diskFilename      ( const QString& , QString& );

      /*! \brief    Function to find out where the solution xml files are stored on disk
                    Returns true if successful, false otherwise

          \param    path The function will return the path here if it is found
      */
      bool diskPath          ( QString& );

      //! \brief Function to count occurrences of solution in protocols
      //! \param    dbP  Pointer to database connection or NULL if local test
      //! \returns       Count of this solution in protocols
      int countInProtocols   ( IUS_DB2* = 0 );

      //! \brief An overloaded assignment operator
      US_Solution&           operator=( const US_Solution& );

      //! \brief Displays the contents of the class variables in qDebug() statements
      void show              ( void );

   private:

      void readSolutionInfo  ( QXmlStreamReader& xml );
      int readBufferDiskGUID ( US_Buffer& , QString& );
      void saveBufferDisk    ( void );
      void saveAnalytesDisk  ( void );
      QString get_filename   ( const QString&, bool& );
      int analyte_type       ( QString );
      QString analyte_typetext( int );
      bool solutionInUse     ( QString& );
      bool solutionInProtocol( const QString, const bool );
};
#endif
