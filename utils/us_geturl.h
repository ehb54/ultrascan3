/**
 * @file us_geturl.h
 * @brief Defines the US_GetUrl class for performing HTTP GET requests.
 */

#ifndef US_GETURL_H
#define US_GETURL_H

#include <QtNetwork>
#include <QtWidgets/QApplication>
#include "us_extern.h"

// derived from last answer https://stackoverflow.com/questions/46943134/how-do-i-write-a-qt-http-get-request

class US_UTIL_EXTERN US_GetUrl : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Constructs a US_GetUrl object.
     */
    explicit US_GetUrl();

    /**
     * @brief Initiates an HTTP GET request for the given URL.
     *
     * @param url The URL to send the GET request to.
     */
    void get(const QString & url);

    QNetworkAccessManager *manager; ///< Manager for handling network operations.

    /**
     * @brief Returns the downloaded data.
     *
     * @return The downloaded data as a QByteArray.
     */
    QByteArray downloadedData() const;

    signals:
            /**
             * @brief Signal emitted when the download is complete.
             */
            void downloaded();

private:
    QByteArray m_DownloadedData;    ///< Byte array to store downloaded data.

private slots:
            /**
             * @brief Slot called when the network reply is finished.
             *
             * @param reply The QNetworkReply containing the response.
             */
            void ReplyFinished(QNetworkReply *reply);
};

#endif // US_GETURL_H
