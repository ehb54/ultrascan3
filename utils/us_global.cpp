//! \file us_global.cpp
#include "us_global.h"
#include <QtCore>
#include <QTextStream>

#ifndef Q_OS_WIN
#include <unistd.h>
#endif

US_Global::US_Global()
{
  valid      = false;
  deleteFlag = false;
  errors.clear();
  
#ifndef Q_OS_WIN
  // Make the key specific to the uid
  QString key = QString( "UltraScan%1" ).arg( getuid() );
#else
  QString key = QString( "UltraScan" );
#endif

  qDebug() << "us_global constructor key ='" << key << "'";

  sharedMemory.setKey( key );

  if ( sharedMemory.attach() ) { 
     qDebug() << "us_global constructor sharedMemory.attach() ok";
     valid = true;
  } else {
     switch( sharedMemory.error() ) {
     case QSharedMemory::NotFound :
        {
           // this is expected when shared memory doesn't already exist
           if ( sharedMemory.create( sizeof global ) ) {
              qDebug() << "us_global constructor sharedMemory.create() ok";
              valid = true;
              set_global_position( QPoint( 50, 50 ) );
              setPasswd( "" );
              // Add an additional global initialization here
           } else {
              qDebug() << "us_global constructor sharedMemory.create() failed " << sharedMemory.error() << " " << sharedMemory.errorString();
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
}

US_Global::~US_Global()
{
  sharedMemory.detach();
  if ( deleteFlag ) sharedMemory.deleteLater();
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
  sharedMemory.lock();
  char* from = (char*)sharedMemory.data();
  memcpy( (char*)&global, from, qMin( sharedMemory.size(), (int)sizeof global ) );
  sharedMemory.unlock();
}

void  US_Global::write_global( void )
{
  sharedMemory.lock();
  char* to = (char*)sharedMemory.data();
  memcpy( to, (char*)&global, qMin( sharedMemory.size(), (int)sizeof global ) );
  sharedMemory.unlock();
}

QString US_Global::errorString() {
   return errors.join( "\n" );
}
