//! \file us_http_post.cpp
#include "us_http_post.h"

US_HttpPost::US_HttpPost( const QString& url, const QString& request ) : QObject()
{
  QNetworkAccessManager* manager = new QNetworkAccessManager( this );
  QNetworkRequest        httpPost;
  
  httpPost.setHeader( QNetworkRequest::ContentTypeHeader, 
                      QString( "application/x-www-form-urlencoded" ) );
  httpPost.setUrl( QUrl( url ) );

  QSslConfiguration conf = httpPost.sslConfiguration();
  conf.setPeerVerifyMode(QSslSocket::VerifyNone);
  httpPost.setSslConfiguration(conf);
  
  reply = manager->post( httpPost, request.toLatin1().data() );
    
  connect( reply, SIGNAL( finished    ( void ) ), 
           this,  SLOT  ( postFinished( void ) ) );
  
  connect( reply, SIGNAL( error    ( QNetworkReply::NetworkError ) ),
           this,  SLOT  ( postError( QNetworkReply::NetworkError ) ) );
  
}

void US_HttpPost::postFinished( void )
{
  int error = reply->error();

  if ( error == QNetworkReply::NoError )
    emit US_Http_post_response( reply->readAll() );
}

void US_HttpPost::postError( QNetworkReply::NetworkError error )
{
  static int count = 0;  // Just send the first error

  if ( count == 0 )
  {
    count++;
    QString err = QString( "9-Network Error (%1): " ).arg( error ) + reply->errorString();
    emit US_Http_post_response( err );
  }
}

