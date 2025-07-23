/**
 * @file us_extinctutils.h
 * @brief Defines structures for handling extinction-related data.
 */

#ifndef US_EXTINCTUTILS_H
#define US_EXTINCTUTILS_H

/**
 * @struct Reading
 * @brief Represents a reading with wavelength and optical density.
 *
 * This structure holds the data for a single reading.
 *
 * @var Reading::lambda
 * Wavelength of the reading.
 *
 * @var Reading::od
 * Optical density of the reading.
 */
struct Reading
{
    float lambda; ///< Wavelength
    float od;     ///< Optical density
};

/**
 * @struct WavelengthScan
 * @brief Represents a scan over a range of wavelengths.
 *
 * This structure holds the data for a scan that includes multiple readings and related metadata.
 *
 * @var WavelengthScan::v_readings
 * Vector of readings.
 *
 * @var WavelengthScan::fileName
 * Name of the file containing the scan data.
 *
 * @var WavelengthScan::filePath
 * Path to the file containing the scan data.
 *
 * @var WavelengthScan::description
 * Description of the scan.
 *
 * @var WavelengthScan::pos
 * Number of positive runs.
 *
 * @var WavelengthScan::neg
 * Number of negative runs.
 *
 * @var WavelengthScan::runs
 * Total number of runs.
 */
struct WavelengthScan
{
    QVector<Reading> v_readings; ///< Vector of readings
    QString fileName;            ///< Name of the file
    QString filePath;            ///< Path to the file
    QString description;         ///< Description of the scan
    int pos;                     ///< Number of positive runs
    int neg;                     ///< Number of negative runs
    int runs;                    ///< Total number of runs
};

#endif // US_EXTINCTUTILS_H
