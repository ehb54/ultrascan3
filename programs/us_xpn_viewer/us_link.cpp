#include <QSslKey>
#include <QSslCertificate>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include "us_link.h"
#include "us_settings.h"

Link::Link()
{
  connect(&server, &QSslSocket::readyRead, this, &Link::rx);
  connect(&server, &QSslSocket::disconnected, this, &Link::serverDisconnect);
  connect(&server, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrors(QList<QSslError>)));

  QString certPath = US_Settings::etcDir() + QString("/optima/"); //US_Settings::appBaseDir() + QString( "/etc/sys_server/" );
  QString keyFile  = certPath + QString( "client.key" );
  QString pemFile  = certPath + QString( "client.pem" );
  server.setPrivateKey(keyFile, QSsl::Ec );
  server.setLocalCertificate(pemFile);

  qDebug() << "Client's certs directory: " << keyFile << pemFile;
  // server.setPrivateKey("client.key", QSsl::Ec );
  // server.setLocalCertificate("client.pem");
  server.setPeerVerifyMode(QSslSocket::VerifyNone);
}

bool Link::connectToServer( const QString& host, const int port )
{
  bool status_ok = false;
  
  server.connectToHostEncrypted(host, port);
  if (server.waitForEncrypted(15000))
    {
      server.write("***qsslsocket_client_example sent this nothing command***\n");
      status_ok = true;
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
    qDebug() << error.errorString();
}

void Link::serverDisconnect(void)
{
  qDebug("Server disconnected");
  //exit(0);
}

void Link::rx(void)
{
  QByteArray response_data = server.readAll();
  QJsonDocument json = QJsonDocument::fromJson(response_data);
  
  QJsonObject rootObj = json.object();

  elapsedTime = rootObj["elapsedTime"].toString();
  omega2T     = rootObj["omega2T"].toString();
  rpm         = rootObj["speed"].toString();
  temperature = rootObj["temperature"].toString();
  vacuum      = rootObj["vacuum"].toString();
  current_stage = rootObj["stage"].toString().split("/")[0];
  tot_stages  = rootObj["stage"].toString().split("/")[1];
  running_scans   = rootObj["Running scan"].toString();
  tot_scans   = rootObj["Total scans"].toString();
  
  
  
  //qDebug() << rootObj["elapsedTime"].toString() << rootObj["omega2T"].toString() << rootObj["speed"].toString() << rootObj["temperature"].toString();
	
  //QTextStream(stdout) << server.readAll();
}
