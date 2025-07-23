//! \file us_project.h
#ifndef US_PROJECT_H
#define US_PROJECT_H

#include "us_extern.h"
#include "us_db2.h"

/*! \class US_Project
           This class provides the ability to enter information about a
           new project, and to select the project.
           */
           
class US_UTIL_EXTERN US_Project
{
   public:

      //! \brief   Some status codes to keep track of where project data has been saved to
      enum projectStatus
      {
         NOT_SAVED,                           //!< The file has not been saved
         EDITING,                             //!< Data is being edited; certain operations not permitted
         HD_ONLY,                             //!< The file has been saved to the HD
         DB_ONLY,                             //!< The file has been saved to the DB
         BOTH                                 //!< The file has been saved to both HD and DB
      };

      int              projectID;          //!< The ID of the project
      QString          projectGUID;        //!< The GUID of the project
      QString          goals;              //!< A detailed description of the goals of the research
      QString          molecules;          //!< Proteins/DNA molecules involved, and mw
      QString          purity;             //!< Approximate purity of the samples, in %
      QString          expense;            //!< Expense of providing appropriate concentration ok
      QString          bufferComponents;   //!< A text listing of buffer components
      QString          saltInformation;    //!< Explanation of an acceptable salt concentration 
      QString          AUC_questions;      //!< Questions research is attempting to answer with the AUC
      QString          expDesign;          //!< Experiment design
      QString          notes;              //!< Special instructions, notes, questions
      QString          projectDesc;        //!< A description of the project 
      QString          status;             //!< Project status

      projectStatus    saveStatus;         //!< Most recent save status
      QDateTime        lastUpdated;        //!< Last date-time updated

      //! \brief Generic constructor for the US_Project class.
      US_Project();

      //! A null destructor. 
      ~US_Project() {};

      /*! \brief    Function to read an entire project structure from the disk

          \param    guid The GUID of the project to look for
          \return   One of the US_DB2 error codes
      */
      int readFromDisk( QString& );

      /*! \brief    Function to read an entire project structure from the DB

          \param    projectID The database projectID of the desired project
          \param    db For database access, an open database connection
          \return   One of the US_DB2 error codes
      */
      int readFromDB  ( int, US_DB2* = 0 );

      //! \brief    Quick method to zero out the project attributes
      void clear             ( void );

      //! \brief    Method to save the current project to disk
      void saveToDisk        ( void );

      /*! \brief    Function to save the project information to db

          \param    db For database access, an open database connection
      */
      int saveToDB           ( US_DB2* = 0 );
      int saveToDB_auto      ( int, US_DB2* = 0 );

      //! \brief    Function to delete the current project from disk, if it exists
      void deleteFromDisk    ( void );

      /*! \brief    Function to delete the current project from the db, if it exists

          \param    db For database access, an open database connection
      */
      void deleteFromDB      ( US_DB2* = 0 );

      /*! \brief    Function to find the filename of a project on disk, if it exists.
                    Returns true if successful, false otherwise

          \param    guid The GUID of the project to look for on disk
          \param    filename The function will return the filename here if it is found
      */
      bool diskFilename      ( const QString& , QString& );

      /*! \brief    Function to find out where the project xml files are stored on disk
                    Returns true if successful, false otherwise

          \param    path The function will return the path here if it is found
      */
      bool diskPath          ( QString& );

      //! \brief Displays the contents of the class variables in qDebug() statements
      void show              ( void );

   private:

      void readProjectInfo  ( QXmlStreamReader& xml );
      QString get_filename   ( const QString&, bool& );
};
#endif
