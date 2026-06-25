//! \file us_http_post.h
#ifndef US_HTTP_POST_H
#define US_HTTP_POST_H

#include <QtCore>
#include <QtNetwork>

// #include "us_extern.h"
#define US_UTIL_EXTERN

//! \brief A text only class to manage HTTP requests

class US_UTIL_EXTERN US_HttpPost : public QObject
{
  Q_OBJECT

  public:
    //!  \brief Make an HTTP POST request 
    //!  \param url    The destination URL
    //!  \param request The data to send with the request
    US_HttpPost( const QString&, const QString& );

  public slots:
    //! \brief Handle HTTP response
    void postFinished( void );

    //! \brief Handle network error
    //! \param error The type of network error douemtnted in 
    //!              QNetworkReply::NetworkError
    void postError( QNetworkReply::NetworkError );

  signals:
    //! \brief Forward the response of the post to the interested function
    void US_Http_post_response( const QString& );

  private:
    QNetworkReply* reply;
};

#endif
