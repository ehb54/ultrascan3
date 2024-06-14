/**
 * @file us_link_ssl.h
 * @brief Defines the Link class for managing SSL connections to a server.
 */

#ifndef LINK_H_
#define LINK_H_

#include "us_extern.h"
#include <QSslSocket>

/**
 * @class Link
 * @brief A class to manage SSL connections to a server.
 *
 * This class handles connecting to, disconnecting from, and communicating with a server
 * using SSL.
 */
class US_UTIL_EXTERN Link : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a Link object.
     *
     * @param certPath The path to the SSL certificate.
     */
    Link(QString certPath);

    /**
     * @brief Connects to a server.
     *
     * @param serverAddress The address of the server to connect to.
     * @param port The port to connect to.
     * @return True if the connection was successful, false otherwise.
     */
    bool connectToServer(const QString& serverAddress, const int port);

    /**
     * @brief Disconnects from the server.
     */
    void disconnectFromServer(void);

    /**
     * @brief Stops the Optima process.
     */
    void stopOptima(void);

    /**
     * @brief Skips the current stage of the Optima process.
     */
    void skipOptimaStage(void);

    QString certPath; ///< Path to the SSL certificate.
    bool disconnected_itself; ///< Indicates if the disconnection was initiated by the client.
    bool connected_itself; ///< Indicates if the connection was initiated by the client.

    QString elapsedTime; ///< Elapsed time.
    QString temperature; ///< Current temperature.
    QString rpm; ///< Current RPM.
    QString omega2T; ///< Omega squared T.
    QString vacuum; ///< Current vacuum level.
    QString current_stage; ///< Current stage.
    QString tot_stages; ///< Total number of stages.
    QString tot_scans; ///< Total number of scans.
    QString running_scans; ///< Number of running scans.

private:
    QSslSocket server; ///< SSL socket for server communication.

    Q_SIGNALS:
            /**
             * @brief Signal emitted when disconnected from the server.
             */
            void disconnected(void);

private slots:
            /**
             * @brief Slot called when SSL errors occur.
             *
             * @param errors List of SSL errors.
             */
            void sslErrors(const QList<QSslError> &errors);

    /**
     * @brief Slot called to receive data from the server.
     */
    void rx(void);

    /**
     * @brief Slot called when disconnected from the server.
     */
    void serverDisconnect(void);
};

#endif /* LINK_H_ */
