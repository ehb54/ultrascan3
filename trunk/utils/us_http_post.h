//! \file us_http_post.h
#ifndef US_HTTP_POST_H
#define US_HTTP_POST_H

#include <QtCore>
#include <QtNetwork>

//! \brief A text only class to provide a check to ensure a valid license

class US_HttpPost : public QObject
{
  Q_OBJECT

  public:
    //!  \brief Make an HTTP POST request 
    //!  \param url    The destination URL
    //!  \param request The data to send with the request
    US_HttpPost( const QString&, const QString& );

  public slots:
    //! \brief Handle HTTP response
    //! \param reply The data and header response to the request
    void postFinished( void );

    //! \brief Handle network error
    void postError( QNetworkReply::NetworkError );

  signals:
    void US_Http_post_response( const QString& );

  private:
    QNetworkReply* reply;
};

#endif
