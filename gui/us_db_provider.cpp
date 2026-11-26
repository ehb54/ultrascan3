//! \file us_db_provider.cpp
#include "us_db_provider.h"

#include "us_passwd.h"
#include "us_session.h"

std::unique_ptr<IUS_DB2> US_DBProvider::open()
{
   US_Session& session = US_Session::instance();

   // An installed provider authenticates on its own; only the saved profile
   // needs the master password. A cancelled or failed unlock leaves the
   // session locked, and the saved-profile connection then fails at once
   // without contacting the server.
   if ( !session.hasProvider() )
   {
      US_Passwd pw;
      pw.getPasswd();
   }
   return session.openDatabase();
}
