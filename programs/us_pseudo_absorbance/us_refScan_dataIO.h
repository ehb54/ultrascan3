//! \file us_refscan_dataio.h
#ifndef US_REFSCAN_DATAIO_H
#define US_REFSCAN_DATAIO_H

#include <QtCore>
#include <us_crc.h>

/**
 * @class US_RefScanDataIO
 * @brief The US_RefScanDataIO class provides functions to read and write reference scan data.
 */
class US_RefScanDataIO
{
    public:
        /**
         * @enum ioError
         * @brief Error codes for IO operations.
         */
        enum ioError { OK, CANTOPEN, BADCRC, NOT_RSDATA, BADTYPE };

        /**
         * @class RefData
         * @brief Class to store reference scan data.
         */
        class RefData
        {
            public:
                char    type[2];         //!< Data type: "RA"|"IP"|"RI"|"FI"|"WA"|"WI"
                int     nWavelength;     //!< Number of wavelengths
                int     nPoints;         //!< Number of data points
                bool    CAState;         //!< Chromatic aberration correction state
                QVector<double> wavelength; //!< Vector of wavelengths
                QVector<double> CAValues;   //!< Vector of chromatic aberration values
                QVector<double> xValues;    //!< Vector of x values
                QVector<QVector<double>> rValues; //!< 2D vector of reference values
                QVector<QVector<double>> std; //!< 2D vector of standard deviations

                /**
                 * @brief Clears the reference data.
                 */
                void clear();

                /**
                 * @brief Gets chromatic aberration corrected values.
                 * @param corrected Reference to boolean indicating if values are corrected
                 * @return 2D vector of corrected values
                 */
                QVector<QVector<double>> get_CA_corrected(bool& corrected);
            };

            /**
             * @brief Writes reference data to a file.
             * @param filename The name of the file
             * @param refData The reference data to write
             * @return Error code
             */
            static int writeRefData(const QString& filename, RefData& refData);

            /**
             * @brief Reads reference data from a file.
             * @param filename The name of the file
             * @param refData The reference data to read
             * @return Error code
             */
            static int readRefData(const QString& filename, RefData& refData);

            /**
             * @brief Gets a string representation of an error code.
             * @param errorCode The error code
             * @return Error string
             */
            static QString errorString(int errorCode);

        private:
            /**
             * @union union16
             * @brief Union to handle 16-bit data.
             */
            union union16
            {
                quint16 ui;  //!< Unsigned 16-bit integer
                char c[2];   //!< Character array
            };

            /**
             * @union union32
             * @brief Union to handle 32-bit data.
             */
            union union32
            {
                float n;     //!< Floating point number
                quint32 ui;  //!< Unsigned 32-bit integer
                char c[4];   //!< Character array
            };

            /**
             * @union union64
             * @brief Union to handle 64-bit data.
             */
            union union64
            {
                double n;    //!< Double precision number
                quint64 ui;  //!< Unsigned 64-bit integer
                char c[8];   //!< Character array
            };

            /**
             * @brief Writes data to a stream.
             * @param stream The data stream
             * @param data The data to write
             * @param size The size of the data
             * @param crc The CRC value to update
             */
            static void write(QDataStream& stream, const char* data, int size, quint32& crc);

            /**
             * @brief Reads data from a stream.
             * @param stream The data stream
             * @param data The buffer to read into
             * @param size The size of the data
             * @param crc The CRC value to update
             */
            static void read(QDataStream& stream, char* data, int size, quint32& crc);
};

#endif // US_REFSCAN_DATAIO_H
