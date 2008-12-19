//! \file us_http_post.cpp
#include "us_http_post.h"

US_HttpPost::US_HttpPost( const QString& url, const QString& data ) : QObject()
{
  QNetworkAccessManager* manager = new QNetworkAccessManager(this);
  QNetworkRequest        request;
  
  request.setUrl( url );


  reply = manager->post( request, data.toAscii().data() );
    
  connect( reply, SIGNAL( finished    ( void ) ), 
           this,  SLOT  ( postFinished( void ) ) );
  
  connect( reply, SIGNAL( error    ( QNetworkReply::NetworkError ) ),
           this,  SLOT  ( postError( QNetworkReply::NetworkError ) ) );
  
}

void US_HttpPost::postFinished( void )
{
  int error = reply->error();
  QString err = QString( "7-%1" ).arg( error );

  if ( error == QNetworkReply::NoError )
    emit US_Http_post_response( reply->readAll() );
}

void US_HttpPost::postError( QNetworkReply::NetworkError error )
{
  static int count = 0;  // Just send the first error

  if ( count == 0 )
  {
    count++;
    QString err = QString( "7-%1" ).arg( error );
    emit US_Http_post_response( err );
  }
}

