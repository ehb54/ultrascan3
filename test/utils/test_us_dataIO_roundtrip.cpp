// AUC-T02: valid read and write -> read round-trip through the public filename
// API, with tolerances derived from the format rather than guessed.
//
// AUC-T01 established the error contract and the shape of a valid read.  This
// suite covers what that left unexecuted: all six data types, the standard
// deviation series, multi-byte interpolation bitmaps, and the quantization the
// format applies to every stored quantity.
//
// Tolerances are computed from the encoding, not tuned until green.  Readings
// are stored as a 16-bit fraction of the value range, so the round-trip error
// bound is (max - min) / 65535 / 2 and nothing smaller is achievable.
//
// Everything runs on fixtures written by the production writer into a per-test
// QTemporaryDir.  Two known-unsafe inputs are deliberately not exercised; see
// the comments on kMaxSafeDescription and buildScan().

#include "qt_test_base.h"
#include "us_dataIO.h"
#include "us_settings.h"

#include <QByteArray>
#include <QFile>
#include <QTemporaryDir>

namespace
{
// The writer copies the description into a 240-byte field with strncpy() and
// the reader rebuilds it with QString(char*).  At exactly 240 characters the
// field holds no terminator and the reader runs off the end of its buffer, so
// 239 is the longest length that can be exercised safely.
constexpr int kMaxSafeDescription = 239;

// Readings and standard deviations are stored as 16-bit fractions of their
// respective ranges.
double quantizationBound(double minValue, double maxValue)
{
    return (maxValue - minValue) / 65535.0 / 2.0;
}

// The interpolation bitmap is one bit per reading, rounded up to whole bytes.
// The writer copies exactly that many bytes out of the QByteArray without
// checking its length, so a short buffer would be an out-of-bounds read; every
// fixture here sizes it correctly.
int bitmapBytes(int pointCount)
{
    return (pointCount + 7) / 8;
}

US_DataIO::Scan buildScan(int pointCount, double base, bool withStdDev)
{
    US_DataIO::Scan sc;
    sc.temperature = 20.5;
    sc.rpm         = 45000.0;
    sc.seconds     = 300;
    sc.omega2t     = 2.5e10;
    sc.wavelength  = 280.0;
    sc.delta_r     = 0.01;
    sc.nz_stddev   = withStdDev;

    for (int point = 0; point < pointCount; point++)
    {
        sc.rvalues << base + 0.05 * point;
        sc.stddevs << (withStdDev ? 0.001 * (point + 1) : 0.0);
    }

    sc.interpolated = QByteArray(bitmapBytes(pointCount), '\0');
    return sc;
}

US_DataIO::RawData buildRawData(int scanCount, int pointCount,
                                const char* type = "RA", bool withStdDev = false)
{
    US_DataIO::RawData data;
    memcpy(data.type, type, 2);
    memcpy(data.rawGUID, "fedcba9876543210", 16);
    data.cell        = 2;
    data.channel     = 'B';
    data.description = "AUC-T02 round-trip fixture";

    for (int point = 0; point < pointCount; point++)
        data.xvalues << 5.8 + 0.01 * point;

    for (int scan = 0; scan < scanCount; scan++)
        data.scanData << buildScan(pointCount, 0.2 + scan, withStdDev);

    return data;
}

// Writes a fixture and reads it back, returning the reader's verdict.
class RoundTrip
{
public:
    explicit RoundTrip(US_DataIO::RawData source)
    {
        EXPECT_TRUE(dir_.isValid());
        const QString file = dir_.path() + "/roundtrip.auc";
        result_ = US_DataIO::writeRawData(file, source);
        if (result_ == US_DataIO::OK)
            result_ = US_DataIO::readRawData(file, data_);
    }

    int result() const { return result_; }
    US_DataIO::RawData& data() { return data_; }

private:
    QTemporaryDir      dir_;
    US_DataIO::RawData data_;
    int                result_ = US_DataIO::OK;
};
}

// ---------------------------------------------------------------------------
// Shape and metadata
// ---------------------------------------------------------------------------

TEST(AucRoundTrip, EverySupportedDataTypeSurvivesTheRoundTrip)
{
    for (const char* type : { "RA", "IP", "RI", "FI", "WA", "WI" })
    {
        SCOPED_TRACE(type);
        RoundTrip trip(buildRawData(2, 8, type));
        ASSERT_EQ(trip.result(), US_DataIO::OK);
        EXPECT_EQ(QString::fromLatin1(trip.data().type, 2), QString::fromLatin1(type));
    }
}

TEST(AucRoundTrip, MetadataAndShapeAreCarriedThroughUnchanged)
{
    RoundTrip trip(buildRawData(5, 32));
    ASSERT_EQ(trip.result(), US_DataIO::OK);

    EXPECT_EQ(trip.data().cell, 2);
    EXPECT_EQ(trip.data().channel, 'B');
    EXPECT_EQ(trip.data().description, QString("AUC-T02 round-trip fixture"));
    EXPECT_EQ(QByteArray(trip.data().rawGUID, 16), QByteArray("fedcba9876543210"));
    EXPECT_EQ(trip.data().scanCount(), 5);
    EXPECT_EQ(trip.data().pointCount(), 32);
    EXPECT_EQ(trip.data().scanData[0].rvalues.size(), 32);
}

TEST(AucRoundTrip, TheLongestSafeDescriptionIsPreservedExactly)
{
    US_DataIO::RawData source = buildRawData(1, 8);
    source.description = QString(kMaxSafeDescription, QChar('D'));

    RoundTrip trip(source);
    ASSERT_EQ(trip.result(), US_DataIO::OK);
    EXPECT_EQ(trip.data().description.length(), kMaxSafeDescription);
    EXPECT_EQ(trip.data().description, QString(kMaxSafeDescription, QChar('D')));
}

// ---------------------------------------------------------------------------
// Reading values and quantization
// ---------------------------------------------------------------------------

TEST(AucRoundTrip, ReadingsReturnWithinTheSixteenBitQuantizationBound)
{
    US_DataIO::RawData source = buildRawData(3, 16);

    // The stored range spans every reading in every scan.
    double minValue = source.scanData[0].rvalues[0];
    double maxValue = minValue;
    for (const US_DataIO::Scan& sc : source.scanData)
        for (double value : sc.rvalues)
        {
            minValue = qMin(minValue, value);
            maxValue = qMax(maxValue, value);
        }

    const double bound = quantizationBound(minValue, maxValue);
    ASSERT_GT(bound, 0.0);

    RoundTrip trip(source);
    ASSERT_EQ(trip.result(), US_DataIO::OK);

    double worst = 0.0;
    for (int scan = 0; scan < source.scanCount(); scan++)
        for (int point = 0; point < source.scanData[scan].rvalues.size(); point++)
            worst = qMax(worst, qAbs(trip.data().scanData[scan].rvalues[point]
                                     - source.scanData[scan].rvalues[point]));

    // Half a quantization step is the floor for this encoding; allow a little
    // slack for the float storage of the range endpoints themselves.
    EXPECT_LE(worst, bound * 1.5)
        << "worst error " << worst << " exceeded the format bound " << bound;

    // And the loss is real: if this were lossless the bound would be proving
    // nothing.  Guards against the test passing because it compared a value
    // with itself.
    EXPECT_GT(worst, 0.0) << "expected quantization loss, saw an exact match";
}

TEST(AucRoundTrip, NonZeroStandardDeviationsAreStoredAndReturned)
{
    US_DataIO::RawData source = buildRawData(2, 16, "RA", /*withStdDev=*/true);

    double minSd = source.scanData[0].stddevs[0];
    double maxSd = minSd;
    for (const US_DataIO::Scan& sc : source.scanData)
        for (double value : sc.stddevs)
        {
            minSd = qMin(minSd, value);
            maxSd = qMax(maxSd, value);
        }

    RoundTrip trip(source);
    ASSERT_EQ(trip.result(), US_DataIO::OK);

    ASSERT_EQ(trip.data().scanCount(), 2);
    EXPECT_TRUE(trip.data().scanData[0].nz_stddev);
    ASSERT_EQ(trip.data().scanData[0].stddevs.size(), 16);

    const double bound = quantizationBound(minSd, maxSd);
    for (int point = 0; point < 16; point++)
        EXPECT_NEAR(trip.data().scanData[0].stddevs[point],
                    source.scanData[0].stddevs[point], bound * 1.5);
}

TEST(AucRoundTrip, AllZeroStandardDeviationsAreDroppedRatherThanStored)
{
    RoundTrip trip(buildRawData(2, 8, "RA", /*withStdDev=*/false));
    ASSERT_EQ(trip.result(), US_DataIO::OK);

    // The writer omits the second series entirely when the range is empty, and
    // the reader reports the absence rather than a vector of zeros.
    EXPECT_FALSE(trip.data().scanData[0].nz_stddev);
    EXPECT_TRUE(trip.data().scanData[0].stddevs.isEmpty());
}

// ---------------------------------------------------------------------------
// Per-scan fields
// ---------------------------------------------------------------------------

TEST(AucRoundTrip, InterpolationBitmapSurvivesAcrossMultipleBytes)
{
    const int points = 20;                       // three bytes of flags
    US_DataIO::RawData source = buildRawData(1, points);

    QByteArray flags(bitmapBytes(points), '\0');
    ASSERT_EQ(flags.size(), 3);
    flags[0] = '\xa5';
    flags[1] = '\x00';
    flags[2] = '\x0f';
    source.scanData[0].interpolated = flags;

    RoundTrip trip(source);
    ASSERT_EQ(trip.result(), US_DataIO::OK);
    EXPECT_EQ(trip.data().scanData[0].interpolated, flags);
}

TEST(AucRoundTrip, WavelengthQuantizesToATenthOfANanometre)
{
    US_DataIO::RawData source = buildRawData(1, 8);
    source.scanData[0].wavelength = 280.37;

    RoundTrip trip(source);
    ASSERT_EQ(trip.result(), US_DataIO::OK);

    // Stored as qRound(wavelength * 10) in 16 bits, so 280.37 -> 2804 -> 280.4.
    EXPECT_NEAR(trip.data().scanData[0].wavelength, 280.4, 1.0e-9);
}

TEST(AucRoundTrip, FractionalSecondsAreTruncatedToWholeSeconds)
{
    US_DataIO::RawData source = buildRawData(1, 8);
    source.scanData[0].seconds = 123.9;

    RoundTrip trip(source);
    ASSERT_EQ(trip.result(), US_DataIO::OK);

    // seconds is cast to quint32 on write, so the fraction is discarded rather
    // than rounded.
    EXPECT_NEAR(trip.data().scanData[0].seconds, 123.0, 1.0e-9);
}

TEST(AucRoundTrip, ScanMetadataReturnsWithinFloatPrecision)
{
    US_DataIO::RawData source = buildRawData(1, 8);
    source.scanData[0].temperature = 20.123;
    source.scanData[0].omega2t     = 2.5e10;
    source.scanData[0].delta_r     = 0.0125;

    RoundTrip trip(source);
    ASSERT_EQ(trip.result(), US_DataIO::OK);

    // These fields are stored as 32-bit floats, so the tolerance is relative.
    EXPECT_NEAR(trip.data().scanData[0].temperature, 20.123, 1.0e-5);
    EXPECT_NEAR(trip.data().scanData[0].omega2t, 2.5e10, 2.5e10 * 1.0e-6);
    EXPECT_NEAR(trip.data().scanData[0].delta_r, 0.0125, 1.0e-8);
}

TEST(AucRoundTrip, RpmIsRoundedToTheSpeedResolutionOnRead)
{
    US_DataIO::RawData source = buildRawData(1, 8);
    source.scanData[0].rpm = 45130.0;

    RoundTrip trip(source);
    ASSERT_EQ(trip.result(), US_DataIO::OK);

    // Not a round trip: the reader rounds RPM to the configured resolution,
    // 100 by default.  AUC-T01 records the settings dependency behind this.
    EXPECT_NEAR(trip.data().scanData[0].rpm, 45100.0, 1.0e-9);
}

// ---------------------------------------------------------------------------
// Radii
// ---------------------------------------------------------------------------

TEST(AucRoundTrip, UniformRadiiAreReconstructedFromTheStoredOriginAndSpacing)
{
    US_DataIO::RawData source = buildRawData(1, 8);

    RoundTrip trip(source);
    ASSERT_EQ(trip.result(), US_DataIO::OK);
    ASSERT_EQ(trip.data().xvalues.size(), 8);

    for (int point = 0; point < 8; point++)
        EXPECT_NEAR(trip.data().xvalues[point], source.xvalues[point], 1.0e-5)
            << "at point " << point;
}

TEST(AucRoundTrip, AnAxisAccumulatedByRepeatedAdditionIsStillUniformEnough)
{
    // Producers build the axis by adding the step over and over, so successive
    // differences drift by rounding.  That must not read as non-uniform.
    US_DataIO::RawData source = buildRawData(1, 500);
    source.xvalues.clear();

    double radius = 5.8;
    for (int point = 0; point < 500; point++)
    {
        source.xvalues << radius;
        radius += 0.001;
    }

    RoundTrip trip(source);
    ASSERT_EQ(trip.result(), US_DataIO::OK);
    EXPECT_EQ(trip.data().pointCount(), 500);
}

TEST(AucRoundTrip, AnEndpointSnappedOntoTheCellBottomIsStillAccepted)
{
    // us_mwl_species_sim builds its grid by accumulation but then overwrites
    // the last radius with the cell bottom, which the point count only
    // approximates.  That displaces one point by up to half a step, so the
    // uniformity check has to leave room for it -- the file it writes today
    // must keep being written.  us_astfem_sim has the same line commented out.
    const double meniscus = 5.8;
    const double bottom   = 7.2005;
    const double radinc   = 0.001;
    const int    points   = qRound((bottom - meniscus) / radinc) + 1;

    US_DataIO::RawData source = buildRawData(1, points);
    source.xvalues.clear();

    double radius = meniscus;
    for (int point = 0; point < points - 1; point++)
    {
        source.xvalues << radius;
        radius += radinc;
    }
    source.xvalues << bottom;

    // The snapped point sits half a step off the grid it would be rebuilt on.
    const double displaced = qAbs(source.xvalues.last()
                                  - (meniscus + radinc * (points - 1)));
    ASSERT_GT(displaced, radinc * 0.4);
    ASSERT_LT(displaced, radinc);

    RoundTrip trip(source);
    ASSERT_EQ(trip.result(), US_DataIO::OK);
    EXPECT_EQ(trip.data().pointCount(), points);
}

TEST(AucRoundTrip, NonUniformRadiiAreRejectedRatherThanSilentlyResampled)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    US_DataIO::RawData source = buildRawData(1, 5);

    // Spacing that widens along the sweep.  Only the first gap can be stored.
    source.xvalues.clear();
    source.xvalues << 5.80 << 5.81 << 5.83 << 5.86 << 5.90;

    // The format keeps a first radius and one spacing, taken as xvalues[1] -
    // xvalues[0], and the reader regenerates the vector arithmetically.  Until
    // this was rejected the write returned OK and the file read back with a
    // last radius of 5.84 instead of the 5.90 that was handed in -- readings
    // relabelled with radii they were not measured at, no error reported.
    //
    // No acquisition path produces such an axis: the legacy Beckman, Optima,
    // CFA and MWL importers all accumulate origin + n * step, the one path that
    // reshapes an existing axis (splitting a triple into radial subsets) copies
    // a contiguous run of an already uniform one, and the chromatic aberration
    // correction in us_xpn_viewer -- the one place a user is warned and can
    // continue -- subtracts the same per-wavelength constant from every point.
    const QString path = dir.path() + "/non-uniform.auc";
    EXPECT_EQ(US_DataIO::writeRawData(path, source), US_DataIO::NOT_USDATA);
    EXPECT_FALSE(QFile::exists(path));

    // A flat or descending axis is rejected for the same reason, and could not
    // be read back in any case -- the reader requires an ascending spacing.
    US_DataIO::RawData flat = buildRawData(1, 8);
    flat.xvalues.fill(5.8);

    const QString flatPath = dir.path() + "/flat-radii.auc";
    EXPECT_EQ(US_DataIO::writeRawData(flatPath, flat), US_DataIO::NOT_USDATA);
    EXPECT_FALSE(QFile::exists(flatPath));
}

// ---------------------------------------------------------------------------
// Interpolation bitmap consistency
//
// writeScan() copies ( pointCount + 7 ) / 8 bytes out of Scan::interpolated.
// Nothing sized that array for it, so a producer that grew its readings without
// growing the bitmap -- which us_astfem_sim did on its save path -- read past
// the end of a QByteArray.
// ---------------------------------------------------------------------------

TEST(AucRoundTrip, AnAbsentInterpolationBitmapMeansNoPointIsInterpolated)
{
    // Simulated data never marks points as interpolated, so an empty bitmap is
    // a legitimate state rather than a defect, and must round-trip as zeroes.
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    US_DataIO::RawData source = buildRawData(2, 20);
    for (int scan = 0; scan < source.scanData.size(); scan++)
        source.scanData[scan].interpolated.clear();

    const QString path = dir.path() + "/no-bitmap.auc";
    ASSERT_EQ(US_DataIO::writeRawData(path, source), US_DataIO::OK);

    US_DataIO::RawData read;
    ASSERT_EQ(US_DataIO::readRawData(path, read), US_DataIO::OK);

    ASSERT_EQ(read.scanCount(), 2);
    for (int scan = 0; scan < read.scanCount(); scan++)
    {
        EXPECT_EQ(read.scanData[scan].interpolated,
                  QByteArray(bitmapBytes(20), '\0')) << "scan " << scan;
    }
}

TEST(AucRoundTrip, AnInterpolationBitmapShorterThanItsReadingsIsRejected)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    US_DataIO::RawData source = buildRawData(1, 20);
    // Present but one byte short: the producer tracked interpolation for some
    // points and not others, which has no coherent meaning.
    source.scanData[0].interpolated = QByteArray(bitmapBytes(20) - 1, '\0');

    const QString path = dir.path() + "/short-bitmap.auc";
    EXPECT_EQ(US_DataIO::writeRawData(path, source), US_DataIO::NOT_USDATA);

    // Rejected before the file was opened, so nothing was left behind.
    EXPECT_FALSE(QFile::exists(path));
}

TEST(AucRoundTrip, AnOversizedInterpolationBitmapIsTruncatedToTheReadings)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    US_DataIO::RawData source = buildRawData(1, 20);
    source.scanData[0].interpolated = QByteArray(bitmapBytes(20) + 4, '\0');

    const QString path = dir.path() + "/long-bitmap.auc";
    ASSERT_EQ(US_DataIO::writeRawData(path, source), US_DataIO::OK);

    US_DataIO::RawData read;
    ASSERT_EQ(US_DataIO::readRawData(path, read), US_DataIO::OK);

    EXPECT_EQ(read.scanData[0].interpolated.size(), bitmapBytes(20));
}

// ---------------------------------------------------------------------------
// Writer input validation
//
// Both cases below were recorded but deliberately not exercised before the
// writer was hardened: each was an out-of-bounds read reached before any
// validation ran.
// ---------------------------------------------------------------------------

TEST(AucRoundTrip, ADatasetWithFewerThanTwoRadiiIsRejected)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    // The radius spacing is derived from xvalues[ 0 ] and xvalues[ 1 ], so a
    // single-point dataset used to read past the end of the vector.
    US_DataIO::RawData single = buildRawData(1, 1);
    const QString singlePath = dir.path() + "/one-radius.auc";
    EXPECT_EQ(US_DataIO::writeRawData(singlePath, single), US_DataIO::NODATA);
    EXPECT_FALSE(QFile::exists(singlePath));

    US_DataIO::RawData none = buildRawData(1, 8);
    none.xvalues.clear();
    const QString nonePath = dir.path() + "/no-radii.auc";
    EXPECT_EQ(US_DataIO::writeRawData(nonePath, none), US_DataIO::NODATA);
    EXPECT_FALSE(QFile::exists(nonePath));
}

TEST(AucRoundTrip, ADescriptionAtTheFieldWidthIsTruncatedRatherThanLeftUnterminated)
{
    // The stored field is 240 bytes.  Filling all 240 left no terminator, and
    // the reader then built a QString from a buffer with none.  239 characters
    // is the most that survives.
    US_DataIO::RawData source = buildRawData(1, 8);
    source.description = QString(240, QChar('D'));

    RoundTrip trip(source);
    ASSERT_EQ(trip.result(), US_DataIO::OK);

    EXPECT_EQ(trip.data().description.size(), kMaxSafeDescription);
    EXPECT_EQ(trip.data().description, QString(kMaxSafeDescription, QChar('D')));
}

TEST(AucRoundTrip, ADescriptionFarLongerThanTheFieldIsTruncatedToTheSameLength)
{
    US_DataIO::RawData source = buildRawData(1, 8);
    source.description = QString(1000, QChar('E'));

    RoundTrip trip(source);
    ASSERT_EQ(trip.result(), US_DataIO::OK);

    EXPECT_EQ(trip.data().description, QString(kMaxSafeDescription, QChar('E')));
}
