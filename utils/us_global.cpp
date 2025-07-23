//! \file us_global.cpp
#include "us_global.h"
#include <QtCore>
#include <QTextStream>

#ifndef Q_OS_WIN
#include <unistd.h>
#endif

#if !defined( Q_OS_WIN ) && !defined( Q_OS_MACOS )
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

US_Global::US_Global()
{
  valid      = false;
  deleteFlag = false;
  errors.clear();
  
#if !defined( Q_OS_WIN ) && !defined( Q_OS_MACOS )
  // for Linux use direct sysV shm

  // the SHM_KEY_BASE can be adjusted if the current value clobbers some other program's usage
# define SHM_KEY_BASE 0xe1000000 

  key_t key = SHM_KEY_BASE + getuid();

  // shm_open
  int shmid = shmget( key
                      ,sizeof(Global)
                      ,IPC_CREAT | 0600
                      );
  if ( shmid == -1 ) {
     valid = false;
     errors << "Unable to get shared memory";
     return;
  }

  QTextStream( stdout ) << "shmget (create) succeeded\n";
         
  struct shmid_ds shmds;
         
  if ( -1 == shmctl( shmid, IPC_STAT, &shmds ) ) {
     valid = false;
     errors << "Unable to get status of shared memory";
     return;
  }
  QTextStream( stdout ) << "shmctl IPC_STAT succedded\n";
  QTextStream( stdout ) << "shmctl nattch is " << shmds.shm_nattch << "\n";

  if ( (void *)-1 == (shmbuf = shmat( shmid, 0, 0 ) ) ) {
     valid = false;
     errors << "Unable to attach to of shared memory";
     return;
  }
  QTextStream( stdout ) << "shmat succeeded\n";

  if ( shmds.shm_nattch == 0 ) {
     QTextStream( stdout ) << "zeroing data\n";
     set_global_position( QPoint( 50, 50 ) );
     setPasswd( "" );
     write_global();
  } else {
     read_global();
  }
  valid = true;
  return;

#else
# ifndef Q_OS_WIN
  // Make the key specific to the uid
  QString key = QString( "UltraScan%1" ).arg( getuid() );
# else
  QString key = QString( "UltraScan" );
# endif

  sharedMemory.setKey( key );

  if ( sharedMemory.attach() ) { 
     // qDebug() << "us_global constructor sharedMemory.attach() ok";
     valid = true;
  } else {
     switch( sharedMemory.error() ) {
     case QSharedMemory::NotFound :
        // this is the expected path when shared memory doesn't already exist
        {
           if ( sharedMemory.create( sizeof global ) ) {
              // qDebug() << "us_global constructor sharedMemory.create() ok";
              valid = true;
              set_global_position( QPoint( 50, 50 ) );
              setPasswd( "" );
              // Add an additional global initialization here
           } else {
              // create errors
              switch( sharedMemory.error() ) {
              case QSharedMemory::NoError :          // should never happen
              case QSharedMemory::LockError :        // should never happen 
              case QSharedMemory::KeyError :         // strange if key is invalid
              case QSharedMemory::AlreadyExists :    // bad ... possible on race condition ... could try attach again
              case QSharedMemory::PermissionDenied : // bad ... likely needs shmutil fix (or system reboot if no one is available to fix with shmutil)
              case QSharedMemory::OutOfResources :   // bad ... system out of resources, need admin help
              case QSharedMemory::InvalidSize :      // bad
              case QSharedMemory::UnknownError :     // bad
              default :
                 {
                    qDebug() << "US_Global() constructor : unexpected sharedMemory create error : " << sharedMemory.errorString();
                    errors << "on create() : " + sharedMemory.errorString();
                    break;
                 }
              }
           }
           break;
        }

     case QSharedMemory::NoError :          // should never happen
     case QSharedMemory::InvalidSize :      // should never happen
     case QSharedMemory::AlreadyExists :    // should never happen
     case QSharedMemory::LockError :        // should never happen 
     case QSharedMemory::OutOfResources :   // should never happen
     case QSharedMemory::PermissionDenied : // bad ... likely requires admin intervention, shmutil to clear
     case QSharedMemory::KeyError :         // strange if key is invalid
     case QSharedMemory::UnknownError :     // bad
     default:
        {
           qDebug() << "US_Global() constructor : unexpected sharedMemory attach error : " << sharedMemory.error() << " " << sharedMemory.errorString();
           errors << "on attach() : " + sharedMemory.errorString();
           break;
        }        

     }           
  }
#endif
}

US_Global::~US_Global()
{
#if !defined( Q_OS_WIN ) && !defined( Q_OS_MACOS )
  sharedMemory.detach();
  if ( deleteFlag ) sharedMemory.deleteLater();
#endif
}

void US_Global::set_global_position( const QPoint& p )
{
  read_global();
  global.current_position = p;
  write_global();
}

QPoint US_Global::global_position( void )
{
  read_global();
  return global.current_position;
}

void US_Global::setPasswd( const QString& pw )
{
  read_global();
  strncpy( global.passwd, pw.toLatin1().constData(), sizeof global.passwd );
  write_global();
}

QString US_Global::passwd( void )
{
  read_global();
  char pw[64];
  strncpy( pw, global.passwd, sizeof global.passwd );
  return QString( pw );
}

void  US_Global::read_global( void )
{
#if !defined( Q_OS_WIN ) && !defined( Q_OS_MACOS )
  // we are not locking, hopefully not an issue, if strange issues are apparent, could add a separate locking semaphore
  memcpy( (void*)&global, shmbuf, sizeof(Global) );
#else
  sharedMemory.lock();
  char* from = (char*)sharedMemory.data();
  memcpy( (char*)&global, from, qMin( sharedMemory.size(), (int)sizeof global ) );
  sharedMemory.unlock();
#endif
}

void  US_Global::write_global( void )
{
#if !defined( Q_OS_WIN ) && !defined( Q_OS_MACOS )
  // we are not locking, hopefully not an issue, if strange issues are apparent, could add a separate locking semaphore
  memcpy( shmbuf, (void*)&global, sizeof(Global) );
#else
  sharedMemory.lock();
  char* to = (char*)sharedMemory.data();
  memcpy( to, (char*)&global, qMin( sharedMemory.size(), (int)sizeof global ) );
  sharedMemory.unlock();
#endif
}

QString US_Global::errorString() {
   return errors.join( "\n" );
}
