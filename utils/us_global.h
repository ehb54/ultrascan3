//! \file us_global.h
#ifndef US_GLOBAL_H
#define US_GLOBAL_H

#include <QSharedMemory>
#include <QPoint>

#include "us_extern.h"

//!  \brief Manage shared memory

/*! \class US_Global 

    This class sets up shared memory and provides a convenient
    interface to read and write data in the shared memory area.
*/

class US_UTIL_EXTERN US_Global
{
  public:

    /*! \brief Create or attach to shared memory

       The constructor will attach to an existing shared memory area.  If
       the area does not exist, it create it and then attach to it. 

       A shared memory area is identified by a unique key.  In this case it
       is defined as:
       
         "UltraScan" + QString( getuid() )

       In this way, each user on a multi-user system will get a uniqe key.

       Note that this has not been tested yet in Windows as Windows doesn't have 
       getuid() equivalent.  On th eother hand, it probably isn't necessary 
       because Windows generally only has one person running on a client system
       at a time.

       Right now there is only one value stored in shared memory: the location of the
       upper left corner of the last program that started.  It is initialized
       to QPoint( 50, 50 ).
    */
    US_Global();

    /*! \brief The destructor detaches from shared memory

        The destructor detaches from shared memory.  If it is the last program
        attached, the shared memory area is destroyed.
    */
    ~US_Global();

    /*! \brief An indicator that shared memory is valid

       A function to return the state of the shared memory segment.  If there was
       an error creating or attaching to the shared memory, it will return false.
    */
    bool   isValid() { return valid; }

    /*! \brief Get the point stored in shared memory

       This function returns the point of the upper left corner that the
       window should use.
    */
    QPoint global_position( void );

    /*! \brief Sets the point in shared memory
        \param p The point to save

       This function sets the point that the next program should use for its upper
       left corner.  The calling program should increnment this point after it
       retrives the current point and decrement it when terminating.
    */
    void   set_global_position( const QPoint& );


    /*! \brief Retrieves the unencrypted master password from shared memory

       This function returns a character string as a QString.
    */
    QString passwd( void );

    /*! \brief Sets the unencrypted master password into shared memory
        \param pw The unencrypted master password
    */
    void setPasswd( const QString& );

    /*! \brief Only called by master program to schedule deleting of
        shared memory when killing child processes
    */
    void scheduleDelete( void ) { deleteFlag = true; }

    /*! \brief last shared memory errorString
    */
    QString errorString();

  private:

    /*!
       Global memory is accessed as raw bytes, so this is a structure of all the
       data defined in the shared memory.  Additional values can be added to this 
       structure as the need for additional shared memory objects is identified.

       Note that variable length construct like QString should not be used, but 
       converted to a fixed length ( e.g. char[32] ).  
       
       If additional shared memory items are added, new functions to get and 
       set those items  should be created.
    */   
    class Global
    {
      public:
      QPoint current_position;
      char   passwd[64];
      // Add other global values as necessary
    };

    Global        global;
    bool          valid;
    bool          deleteFlag;
    QSharedMemory sharedMemory;
    QStringList   errors;

    void read_global ( void );
    void write_global( void );

#if !defined( Q_OS_WIN ) && !defined( Q_OS_MACOS )
    void *shmbuf;
#endif
};

#endif
