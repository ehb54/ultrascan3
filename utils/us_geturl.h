/**
 * @file us_geturl.h
 * @brief Defines the US_GetUrl class for performing HTTP GET requests.
 */

#ifndef US_GETURL_H
#define US_GETURL_H

#include <QtNetwork>
#include <QtWidgets/QApplication>
#include "us_extern.h"

/**
 * @class US_GetUrl
 * @brief A class to perform HTTP GET requests.
 *
 * This class uses QNetworkAccessManager to perform HTTP GET requests and
 * retrieve data from a specified URL.
 */
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
    QNetworkAccessManager *manager; ///< Manager for handling network operations.
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
