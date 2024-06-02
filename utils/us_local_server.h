//! \file us_local_server.h
#ifndef US_LSERVER_H
#define US_LSERVER_H

#include "us_extern.h"
#include <QLocalSocket>
#include <QLocalServer>

// *********** user defines *************

/*!
 * \brief Convert boolean to string representation.
 * \param b Boolean value to convert.
 * \return "true" if b is true, otherwise "false".
 */
static QString is_true( bool b ) {
    return b ? "true" : "false";
}

/*!
 * \class QInstances
 * \brief Manages instances of a local server with a unique name prefix.
 */
class US_UTIL_EXTERN QInstances {
public:
    /*!
     * \brief Constructor to initialize QInstances with a name prefix.
     * \param name_prefix Prefix for instance names.
     */
    QInstances( const QString & name_prefix );

    /*!
     * \brief Destructor for QInstances.
     */
    ~QInstances();

    /*!
     * \brief Create a unique instance.
     * \return True if the instance was created successfully, otherwise false.
     */
    bool create();

    /*!
     * \brief Remove dead instances filesystem remnants.
     * \return Number of instances cleaned (expected to be MAX_INSTANCES - active).
     */
    int cleanup();

    /*!
     * \brief Count active instances.
     * \return Number of active instances.
     */
    int count();

    /*!
     * \brief Close the instance.
     * \return True if the instance was closed successfully, otherwise false.
     */
    bool close();

private:
    QString name_prefix; /*!< Prefix for instance names. */

    /*!
     * \brief Generate instance name based on a number.
     * \param n Instance number.
     * \return Generated instance name.
     */
    QString instance_name( int n );

    /*!
     * \brief Try to create an instance with a specific number.
     * \param n Instance number.
     * \return True if the instance was created successfully, otherwise false.
     */
    bool try_create( int n );

    /*!
     * \brief Check if an instance with a specific number is running.
     * \param n Instance number.
     * \return True if the instance is running, otherwise false.
     */
    bool is_running( int n );

    QLocalServer * server; /*!< Local server instance. */
};

#endif
