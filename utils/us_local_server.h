//! \file us_local_server.h
#ifndef US_LSERVER_H
#define US_LSERVER_H

#include <QLocalSocket>
#include <QLocalServer>

// *********** user defines *************

static QString is_true( bool b ) {
  return b ? "true" : "false";
}

class QInstances {
public:
   QInstances( const QString & name_prefix );
   ~QInstances();

   bool           create();    // create a unique instance
   int            cleanup();   // removes dead instances filesystem remnants, returns number cleaned (expected to be MAX_INSTANCES - active), not essential
   int            count();     // counts active instances
   bool           close();     // close the instance

private:
   QString        name_prefix;
   QString        instance_name( int n );
   bool           try_create( int n );
   bool           is_running( int n );
   QLocalServer * server;
};

#endif
