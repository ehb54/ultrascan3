//! \file us_solution.h
#ifndef US_SOLUTION_H
#define US_SOLUTION_H

#include "us_extern.h"

/*! \class US_Solution
           This class provides the ability to associate the solution
           parameters, buffers, and analytes with the appropriate
           cell / channel / wavelength combination in the experiment.
*/
           
class US_EXTERN US_Solution
{
   public:

      //! \brief  Class that contains information about the
      //!         individual analytes
      class AnalyteInfo
      {
         public:
         int           analyteID;          //!< The ID of the analyte that was associated
         QString       analyteGUID;        //!< The GUID of the analyte
         QString       analyteDesc;        //!< The corresponding analyte description
         float         amount;             //!< The amount of this component in the solution

         //! \brief    Generic operator== to test for equality
         bool          operator== ( const AnalyteInfo & ) const;
      };

      int              invID;              //!< The ID of the investigator
      int              solutionID;         //!< The ID of the solution for this triple
      QString          solutionGUID;       //!< The GUID of the solution for this triple
      QString          solutionDesc;       //!< A description of the solution 
      int              bufferID;           //!< The ID of the buffer that was associated
      QString          bufferGUID;         //!< The GUID of the buffer
      QString          bufferDesc;         //!< The corresponding buffer description
      QList< AnalyteInfo > analytes;       //!< A list of the analyte information
      float            storageTemp;        //!< The temperature that the solution was stored
      QString          notes;              //!< Notes on the channel solution

      //! \brief Generic constructor for the US_Solution class.
      US_Solution();

      //! A null destructor. 
      ~US_Solution() {};

      //! \brief   Some status codes to keep track of where solution data has been saved to
      enum solutionStatus
      {
         NOT_SAVED,                           //!< The file has not been saved
         EDITING,                             //!< Data is being edited; certain operations not permitted
         HD_ONLY,                             //!< The file has been saved to the HD
         DB_ONLY,                             //!< The file has been saved to the DB
         BOTH                                 //!< The file has been saved to both HD and DB
      };

      /*! \brief    Function to read an entire solution structure from the disk

          \param    guid The GUID of the solution to look for
      */
      void readFromDisk( QString& );

      /*! \brief    Function to read an entire solution structure from the DB

          \param    solutionID The database solutionID of the desired solution
          \param    db For database access, an open database connection
      */
      void readFromDB  ( int, US_DB2* = 0 );

      //! \brief    Quick method to zero out the solution attributes
      void clear             ( void );

      //! \brief    Method to save the current solution to disk
      void saveToDisk        ( void );

      /*! \brief    Function to save the solution information to db

          \param    expID The ID of the experiment this solution is associated with
          \param    channelID The ID of the channel
          \param    db For database access, an open database connection
      */
      void saveToDB          ( int = 1, int = 1, US_DB2* = 0 );

      //! \brief    Function to delete the current solution from disk, if it exists
      void deleteFromDisk    ( void );

      /*! \brief    Function to delete the current solution from the db, if it exists

          \param    db For database access, an open database connection
      */
      void deleteFromDB      ( US_DB2* = 0 );

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

      void show              ( void );       // temporary

   private:

      void readSolutionInfo  ( QXmlStreamReader& xml );
      QString get_filename   ( const QString&, bool& );
};
#endif
