//! \file us_global.cpp
#include "us_global.h"
#include <QtCore>
#include <QTextStream>

#ifdef Q_WS_X11
#include <unistd.h>
#endif

US_Global::US_Global()
{
  valid      = false;
  deleteFlag = false;

#ifndef WIN32
  // Make the key specific to the uid
  QString key = QString( "UltraScan%1" ).arg( getuid() );
#else
  QString key = QString( "UltraScan" );
#endif

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
  strncpy( global.passwd, pw.toLatin1(), sizeof global.passwd );
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

