#ifndef US_JSON_H
#define US_JSON_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <qstring.h>
#include <qregexp.h>
#include <map>
#include <q3socketdevice.h> 
#include "us_extern.h"

using namespace std;

class US_EXTERN US_Json
{
 public:
   static map < QString, QString > split( QString );
   static QString compose( map < QString, QString > & );
};


class US_EXTERN US_Udp_Msg
{
 public:
   US_Udp_Msg( QString host, Q_UINT16 port );
   ~US_Udp_Msg();

   void set_default_json ( map < QString, QString > & json );
   void send             ( QString & msg );
   void send_json        ( map < QString, QString > json );

 private:
   Q3SocketDevice            * qsd;
   QString                    host;
   Q_UINT16                   port;
   map < QString, QString >   default_json;
};

#endif
