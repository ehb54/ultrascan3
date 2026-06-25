#include "us_db2.h"

int main() {
   QTextStream(stdout) << "testdb runs!\n";
   QString error;
   US_DB2  db;

   if ( db.test_secure_connection(
                                  "host.ip.address"
                                  ,"database name"
                                  ,"user name"
                                  ,"password"
                                  ,"investigator email"
                                  ,"investigator password"
                                  ,error
                                  ) ) {
      QTextStream( stdout ) << "connected!\n";
   } else {
      QTextStream( stdout ) << "connection failed: " << error << "\n";
   }
}
