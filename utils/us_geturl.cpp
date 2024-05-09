#include "us_geturl.h"

US_GetUrl::US_GetUrl() {
   manager = new QNetworkAccessManager();
}

void US_GetUrl::get( const QString & url) {
   m_DownloadedData.clear();

   connect(manager, &QNetworkAccessManager::finished,
           this, &US_GetUrl::ReplyFinished, Qt::UniqueConnection);

   manager->get(QNetworkRequest(QUrl(url)));
}

void US_GetUrl::ReplyFinished(QNetworkReply *reply) {
   m_DownloadedData = reply->readAll();
   emit downloaded();
}

QByteArray US_GetUrl::downloadedData() const {
   return m_DownloadedData;
}
