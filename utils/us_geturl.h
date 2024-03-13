#ifndef US_GETURL_H
#define US_GETURL_H

#include <QtNetwork>
#include <QtWidgets/QApplication>
#include "us_extern.h"

// derived from last answer https://stackoverflow.com/questions/46943134/how-do-i-write-a-qt-http-get-request

class US_UTIL_EXTERN US_GetUrl : public QObject {
   Q_OBJECT
 public:
   explicit US_GetUrl();
   void get( const QString & url );
   QNetworkAccessManager *manager;
   QByteArray downloadedData() const;

 signals:
   void downloaded();

 private:
   QByteArray m_DownloadedData;
   
 private slots:
   void ReplyFinished(QNetworkReply *reply);
};

#endif // US_GETURL_H
