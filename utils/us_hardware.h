//! \file us_hardware.h
#ifndef US_HARDWARE_H
#define US_HARDWARE_H

#include <QtCore>
#include "us_db2.h"
#include "us_extern.h"
#include "us_util.h"

//! \brief A set of static functions to handle hardware data
class US_UTIL_EXTERN US_Hardware {
   public:
      //! \brief Read rotor information from a local XML file to a QMap
      //! \param rotor_map Reference of QMap into which to place data
      static bool readRotorMap(QMap<QString, QString> &);

      //! \brief Read rotor information from the database to a QMap
      //! \param db        Pointer to opened database connection
      //! \param rotor_map Reference of QMap into which to place data
      static bool readRotorMap(US_DB2 *, QMap<QString, QString> &);

      //! \brief Get coefficient values for a specified rotor
      //! \param rCalID    Rotor calibration ID for which to get values
      //! \param rotor_map QMap of serial,value mappings
      //! \param rotcoeffs Array of 2 doubles to fill with rotor coefficients
      static bool rotorValues(QString, QMap<QString, QString>, double *);
};

//! \brief Centerpiece data
class US_UTIL_EXTERN US_AbstractCenterpiece {
   public:
      US_AbstractCenterpiece();

      int serial_number; //!< internal identifier
      QString guid; //!< global identifier
      QString name; //!< textual description
      QString material; //!< epon, aluminum, titanium
      int channels; //!< number of columns of channels
      QString shape; //!< shape of the channel

      //! Angle of sector, if sector shaped, default: 2.5 degrees
      double angle;

      //! Width of channel if rectangular, 0 otherwise.
      double width;

      //! Maximum speed in RPM
      double maxRPM;

      //! Bottom position of each row
      QList<double> path_length; //!< path lengths of channels in a column
      QList<double> bottom_position; //!< bottom of each row of channels

      //!  Read centerpieces from database (or local disk if db==NULL)
      //!  \param db           Pointer to database connection (NULL for local)
      //!  \param centerpieces A list of centerpiece data
      //!  \return A boolean indicating success
      static bool read_centerpieces(US_DB2 *, QList<US_AbstractCenterpiece> &);

      //!  Read centerpieces from local disk
      //!  \param centerpieces A list of centerpiece data
      //!  \return A boolean indicating success
      static bool read_centerpieces(QList<US_AbstractCenterpiece> &);
};
#endif
