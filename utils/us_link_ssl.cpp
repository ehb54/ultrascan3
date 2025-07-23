#include <QSslKey>
#include <QSslCertificate>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include "us_link_ssl.h"
#include "us_settings.h"


Link::Link( QString alias )
//Link::Link()
{
  connect(&server, &QSslSocket::readyRead, this, &Link::rx);
  connect(&server, &QSslSocket::disconnected, this, &Link::serverDisconnect);
  connect(&server, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrors(QList<QSslError>)));
 
  certPath = US_Settings::etcDir() + QString("/optima/");

  disconnected_itself = false;
  connected_itself    = false;
  
  QString client_name = alias;
  client_name.simplified();
  client_name.replace(" ", "");
  client_name = client_name.toLower();
  
  // QString keyFile  = certPath + QString( "client.key" );
  // QString pemFile  = certPath + QString( "client.pem" );

  QString keyFile  = certPath + client_name + QString( ".key" );
  QString pemFile  = certPath + client_name + QString( ".pem" );
  
  //  qDebug() << "keyFile, pemFile -- " << keyFile << pemFile;

  server.setPrivateKey(keyFile, QSsl::Ec );
  server.setLocalCertificate(pemFile);

  qDebug() << "Client's certs directory: " << keyFile << pemFile;
   
  server.setPeerVerifyMode(QSslSocket::VerifyNone);

}

bool Link::connectToServer( const QString& host, const int port )
{
  bool status_ok = false;

  // if ( !QDir( certPath ).exists() )
  //   return status_ok;

  
  server.connectToHostEncrypted(host, port);
  if (server.waitForEncrypted(15000))
    {
      //server.write("***qsslsocket_client_example sent this nothing command***\n");
      //status_ok = true;
      status_ok = server.waitForReadyRead(30000);
    }
  else
    {
      qDebug("Unable to connect to server");
      //exit(0);
    }

  return status_ok;
}


void Link::stopOptima( void )
{
  server.write("9\n");
  // if (server.waitForEncrypted(10000)) {
  //   server.write("9\n");
  // }
  // else {
  //   qDebug("Unable to connect to server");
  //   //exit(0);
  // }
}

void Link::skipOptimaStage( void )
{
  server.write("6\n");
  // if (server.waitForEncrypted(10000)) {
  //   server.write("6\n");
  // }
  // else {
  //   qDebug("Unable to connect to server");
  //   //exit(0);
  // }
}

void Link::disconnectFromServer( void  )
{
  server.disconnectFromHost();
  qDebug() << "Closing Connection";
}

void Link::sslErrors(const QList<QSslError> &errors)
{
  foreach (const QSslError &error, errors)
    {
      qDebug() << error.errorString();
      // if ( QString (error.errorString() ).contains("The certificate is self-signed, and untrusted")  )
      // 	server.ignoreSslErrors(errors);
    }
  //server.ignoreSslErrors(errors);
}

void Link::serverDisconnect(void)
{
  qDebug() << "Server disconnected from inside LINK -- e.g. [DROPPED CONNECTION]";
  disconnected_itself = true;

  //exit(0);
}

void Link::rx(void)
{
   connected_itself = true;

   //qDebug() << "In LINK: ready to read: connected_itself " << connected_itself;

   QByteArray response_data = server.readAll();
   // optima sometimes returns 0xFF for the board
   response_data.replace( '\xff', ' ' );
   QJsonParseError parseerror;
   QJsonDocument json = QJsonDocument::fromJson(response_data, &parseerror );
   if ( parseerror.error ) {
      qDebug() << "Link::rx() json parse error " << parseerror.errorString() << "\n";
      qDebug() << "Link::rx() response_data " << response_data;
   } else {
      QJsonObject rootObj = json.object();

      if (
          rootObj.contains("elapsedTime") &&
          rootObj.contains("omega2T") &&
          rootObj.contains("speed") &&
          rootObj.contains("temperature") &&
          rootObj.contains("vacuum") &&
          rootObj.contains("stage") &&
          rootObj.contains("Running scan") &&
          rootObj.contains("Total scans")
          ) {

         elapsedTime = rootObj["elapsedTime"].toString();
         omega2T     = rootObj["omega2T"].toString();
         rpm         = rootObj["speed"].toString();
         temperature = rootObj["temperature"].toString();
         vacuum      = rootObj["vacuum"].toString();
         current_stage = rootObj["stage"].toString().split("/")[0];
         tot_stages  = rootObj["stage"].toString().split("/")[1];
         running_scans = rootObj["Running scan"].toString();
         tot_scans   = rootObj["Total scans"].toString();
      } else {
         qDebug() << "Link::rx() received incorrect json\n";
         qDebug() << "Link::rx() response_data " << response_data;
      }
   }

   //qDebug() << rootObj["elapsedTime"].toString() << rootObj["omega2T"].toString() << rootObj["speed"].toString() << rootObj["temperature"].toString();

   //QTextStream(stdout) << server.readAll();
}
