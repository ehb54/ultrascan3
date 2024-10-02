//! \file us3i_global.cpp
#include "us3i_global.h"
#include <QtCore>
#include <QTextStream>

#ifndef Q_OS_WIN
#include <unistd.h>
#endif

US3i_Global::US3i_Global()
{
  valid      = false;
  deleteFlag = false;

#ifndef Q_OS_WIN
  // Make the key specific to the uid
  QString key = QString( "UltraScan%1" ).arg( getuid() );
#else
  QString key = QString( "UltraScan" );
#endif

#if !defined( NO_SHARED_MEMORY )
  sharedMemory.setKey( key );

  if ( sharedMemory.attach() )
  { 
    valid = true;
  }

  else if ( sharedMemory.error() == QSharedMemory::OutOfResources )
  {
    qDebug() << "Shared memory out of resources";
  }

  else if ( sharedMemory.create( sizeof global ) )
  {
    valid = true;
    set_global_position( QPoint( 50, 50 ) );
    setPasswd( "" );
    // Add an additional global initialization here
  }

  else
  {
    qDebug( "Failure to create shared memory" );
    qDebug() << sharedMemory.errorString();
  }
#endif
}

US3i_Global::~US3i_Global()
{
#if !defined( NO_SHARED_MEMORY )
  sharedMemory.detach();
  if ( deleteFlag ) sharedMemory.deleteLater();
#endif
}

void US3i_Global::set_global_position( const QPoint& p )
{
  read_global();
  global.current_position = p;
  write_global();
}

QPoint US3i_Global::global_position( void )
{
  read_global();
  return global.current_position;
}

void US3i_Global::setPasswd( const QString& pw )
{
  read_global();
  strncpy( global.passwd, pw.toLatin1().constData(), sizeof global.passwd - 1 );
  write_global();
}

QString US3i_Global::passwd( void )
{
  read_global();
  char pw[64];
  strncpy( pw, global.passwd, sizeof global.passwd );
  return QString( pw );
}

void  US3i_Global::read_global( void )
{
#if !defined( NO_SHARED_MEMORY )
  sharedMemory.lock();
  char* from = (char*)sharedMemory.data();
  memcpy( (char*)&global, from, qMin( sharedMemory.size(), (int)sizeof global ) );
  sharedMemory.unlock();
#endif
}

void  US3i_Global::write_global( void )
{
#if !defined( NO_SHARED_MEMORY )
  sharedMemory.lock();
  char* to = (char*)sharedMemory.data();
  memcpy( to, (char*)&global, qMin( sharedMemory.size(), (int)sizeof global ) );
  sharedMemory.unlock();
#endif
}

