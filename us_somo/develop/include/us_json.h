#ifndef US_JSON_H
#define US_JSON_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <qstring.h>
#include <map>
#include <qudpsocket.h> 
#include "us_extern.h"

using namespace std;

#ifdef WIN32
# if QT_VERSION < 0x040000
  #pragma warning ( disable: 4251 )
# endif
#endif

class US_EXTERN US_Json
{
 public:
   static map < QString, QString > split( QString );
   static QString compose( map < QString, QString > & );
   static bool decode_array_to_vector_double( const QString & str, vector < double > & resultf, QString & /* errormsg */ );
   static QString encode_vector_double( const vector < double > & data );
};

class US_EXTERN US_Udp_Msg
{
 public:
   US_Udp_Msg( QString host, quint16 port );
   ~US_Udp_Msg();

   void set_default_json ( map < QString, QString > & json );
   void send             ( QString & msg );
   void send_json        ( map < QString, QString > json );

 private:
   QUdpSocket            * qsd;
   QString                    host;
   quint16                   port;
   map < QString, QString >   default_json;
};

#ifdef WIN32
# if QT_VERSION < 0x040000
  #pragma warning ( default: 4251 )
# endif
#endif

#endif
