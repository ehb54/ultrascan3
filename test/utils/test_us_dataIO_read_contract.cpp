// AUC reader status codes and destination-state guarantees.

#include "qt_test_base.h"
#include "us_dataIO.h"
#include "us_settings.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>

namespace
{
// Byte offsets of the fixed-size header fields, derived from writeRawData():
// 4 magic + 2 version + 2 type + 1 cell + 1 channel + 16 GUID + 240 description
// = 266, then seven 4-byte floats = 294, then a 2-byte scan count = 296.
constexpr int kMagicOffset   = 0;
constexpr int kVersionOffset = 4;
constexpr int kTypeOffset    = 6;
constexpr int kFirstScanMark = 296;

constexpr double kScanRpm = 45130.0;

US_DataIO::RawData makeRawData()
{
    US_DataIO::RawData data;
    memcpy(data.type, "RA", 2);
    memcpy(data.rawGUID, "0123456789abcdef", 16);
    data.cell        = 3;
    data.channel     = 'A';
    data.description = "AUC reader fixture";

    for (int point = 0; point < 8; point++)
        data.xvalues << 5.8 + 0.01 * point;

    for (int scan = 0; scan < 2; scan++)
    {
        US_DataIO::Scan sc;
        sc.temperature = 20.0 + scan;
        sc.rpm         = kScanRpm;
        sc.seconds     = 100 * (scan + 1);
        sc.omega2t     = 1.0e10 * (scan + 1);
        sc.wavelength  = 280.0;
        sc.delta_r     = 0.01;
        sc.nz_stddev   = false;

        for (int point = 0; point < 8; point++)
        {
            sc.rvalues << 0.1 * (point + 1) + scan;
            sc.stddevs << 0.0;
        }

        sc.interpolated = QByteArray(1, '\0');
        data.scanData << sc;
    }

    return data;
}

// One QTemporaryDir per case, holding a freshly written valid fixture plus any
// mutated copies the case needs.
class AucFixture
{
public:
    AucFixture()
    {
        EXPECT_TRUE(dir_.isValid());
        US_DataIO::RawData source = makeRawData();
        EXPECT_EQ(US_DataIO::writeRawData(validPath(), source), US_DataIO::OK);

        QFile ff(validPath());
        EXPECT_TRUE(ff.open(QIODevice::ReadOnly));
        bytes_ = ff.readAll();
    }

    QString validPath() const { return dir_.path() + "/valid.auc"; }
    QString path(const QString& name) const { return dir_.path() + "/" + name; }
    const QByteArray& bytes() const { return bytes_; }

    // Write a mutated copy and return its path.
    QString mutated(const QString& name, int offset, const QByteArray& patch) const
    {
        QByteArray copy = bytes_;
        EXPECT_LE(offset + patch.size(), copy.size());
        copy.replace(offset, patch.size(), patch);

        const QString target = path(name);
        QFile ff(target);
        EXPECT_TRUE(ff.open(QIODevice::WriteOnly));
        ff.write(copy);
        ff.close();
        return target;
    }

    // Write a copy with extra bytes past the stored checksum.
    QString appended(const QString& name, const QByteArray& extra) const
    {
        const QString target = path(name);
        QFile ff(target);
        EXPECT_TRUE(ff.open(QIODevice::WriteOnly));
        ff.write(bytes_);
        ff.write(extra);
        ff.close();
        return target;
    }

private:
    QTemporaryDir dir_;
    QByteArray    bytes_;
};

// Restores debug_text on destruction; readRawData reads it through US_Settings.
class DebugTextGuard
{
public:
    explicit DebugTextGuard(const QStringList& value) : saved_(US_Settings::debug_text())
    {
        US_Settings::set_debug_text(value);
    }
    ~DebugTextGuard() { US_Settings::set_debug_text(saved_); }

private:
    QStringList saved_;
};
}

// ---------------------------------------------------------------------------
// Valid input
// ---------------------------------------------------------------------------

TEST(AucReadContract, ValidFixtureRoundTripsThroughTheProductionWriterAndReader)
{
    AucFixture fixture;

    US_DataIO::RawData data;
    ASSERT_EQ(US_DataIO::readRawData(fixture.validPath(), data), US_DataIO::OK);

    EXPECT_EQ(QString::fromLatin1(data.type, 2), QString("RA"));
    EXPECT_EQ(data.cell, 3);
    EXPECT_EQ(data.channel, 'A');
    EXPECT_EQ(data.description, QString("AUC reader fixture"));
    EXPECT_EQ(QByteArray(data.rawGUID, 16), QByteArray("0123456789abcdef"));

    ASSERT_EQ(data.scanCount(), 2);
    ASSERT_EQ(data.pointCount(), 8);
    EXPECT_EQ(data.xvalues.size(), 8);
    EXPECT_NEAR(data.xvalues.first(), 5.8, 1.0e-4);

    // The format stores readings as 16-bit fractions of the value range, so a
    // round trip is quantized rather than exact.
    EXPECT_NEAR(data.scanData[0].rvalues[0], 0.1, 1.0e-4);
    EXPECT_NEAR(data.scanData[1].rvalues[7], 1.8, 1.0e-4);
    EXPECT_NEAR(data.scanData[0].temperature, 20.0, 1.0e-3);
    EXPECT_NEAR(data.scanData[0].wavelength, 280.0, 1.0e-6);

    // All standard deviations were zero, so the writer omits the second series
    // and the reader reports the scan as having none.
    EXPECT_FALSE(data.scanData[0].nz_stddev);
    EXPECT_TRUE(data.scanData[0].stddevs.isEmpty());

    // One interpolation bit per reading, rounded up to whole bytes.
    EXPECT_EQ(data.scanData[0].interpolated.size(), 1);
}

// ---------------------------------------------------------------------------
// Rejection paths
// ---------------------------------------------------------------------------

TEST(AucReadContract, MissingFileReportsCantopenAndLeavesTheDestinationUntouched)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    US_DataIO::RawData data = makeRawData();
    const int scansBefore   = data.scanCount();

    EXPECT_EQ(US_DataIO::readRawData(dir.path() + "/absent.auc", data),
              US_DataIO::CANTOPEN);
    EXPECT_EQ(data.scanCount(), scansBefore);
}

TEST(AucReadContract, EmptyFileIsRejectedAsNotUsData)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    const QString empty = dir.path() + "/empty.auc";
    QFile ff(empty);
    ASSERT_TRUE(ff.open(QIODevice::WriteOnly));
    ff.close();

    US_DataIO::RawData data;
    EXPECT_EQ(US_DataIO::readRawData(empty, data), US_DataIO::NOT_USDATA);
    EXPECT_EQ(data.scanCount(), 0);
}

TEST(AucReadContract, BadMagicIsRejectedAsNotUsData)
{
    AucFixture fixture;
    US_DataIO::RawData data;
    EXPECT_EQ(US_DataIO::readRawData(
                  fixture.mutated("bad_magic.auc", kMagicOffset, "XCDA"), data),
              US_DataIO::NOT_USDATA);
    EXPECT_EQ(data.scanCount(), 0);
}

TEST(AucReadContract, VersionAboveTheSupportedFormatIsRejected)
{
    AucFixture fixture;
    US_DataIO::RawData data;
    // The writer emits the version as ASCII "05"; the reader keeps only the low
    // nibble of each byte, so "06" decodes to 6, one past format_version.
    EXPECT_EQ(US_DataIO::readRawData(
                  fixture.mutated("bad_version.auc", kVersionOffset, "06"), data),
              US_DataIO::BAD_VERSION);
    EXPECT_EQ(data.scanCount(), 0);
}

TEST(AucReadContract, UnknownDataTypeIsRejected)
{
    AucFixture fixture;
    US_DataIO::RawData data;
    EXPECT_EQ(US_DataIO::readRawData(
                  fixture.mutated("bad_type.auc", kTypeOffset, "ZZ"), data),
              US_DataIO::BADTYPE);
    EXPECT_EQ(data.scanCount(), 0);
}

TEST(AucReadContract, CorruptScanMarkerIsReportedAsNotUsDataRatherThanItsOwnCode)
{
    AucFixture fixture;
    ASSERT_GT(fixture.bytes().size(), kFirstScanMark + 4);
    ASSERT_EQ(fixture.bytes().mid(kFirstScanMark, 4), QByteArray("DATA"));

    US_DataIO::RawData data;
    EXPECT_EQ(US_DataIO::readRawData(
                  fixture.mutated("bad_scan.auc", kFirstScanMark, "DAT?"), data),
              US_DataIO::NOT_USDATA);
}

TEST(AucReadContract, CorruptPayloadIsCaughtByTheTrailingChecksum)
{
    AucFixture fixture;

    // Flip a reading byte well past the header; every parsed byte feeds the CRC.
    const int payloadOffset = kFirstScanMark + 32;
    ASSERT_LT(payloadOffset, fixture.bytes().size() - 4);

    QByteArray flipped(1, fixture.bytes().at(payloadOffset) ^ 0xff);

    US_DataIO::RawData data;
    EXPECT_EQ(US_DataIO::readRawData(
                  fixture.mutated("bad_crc.auc", payloadOffset, flipped), data),
              US_DataIO::BADCRC);
}

TEST(AucReadContract, BytesAfterTheChecksumAreIgnored)
{
    AucFixture fixture;

    US_DataIO::RawData data;
    EXPECT_EQ(US_DataIO::readRawData(
                  fixture.appended("trailing_junk.auc", QByteArray(8, '\x5a')), data),
              US_DataIO::OK);
    EXPECT_EQ(data.scanCount(), 2);
}

TEST(AucReadContract, AnOlderVersionByteCannotBeExercisedByPatchingAlone)
{
    AucFixture fixture;

    US_DataIO::RawData data;
    EXPECT_EQ(US_DataIO::readRawData(
                  fixture.mutated("version_04.auc", kVersionOffset, "04"), data),
              US_DataIO::BADCRC);

    EXPECT_EQ(data.scanCount(), 0);
}

// ---------------------------------------------------------------------------
// Destination state on failure
// ---------------------------------------------------------------------------

TEST(AucReadContract, ChecksumFailureLeavesTheDestinationUntouched)
{
    AucFixture fixture;

    // Damage the stored checksum itself, leaving every parsed field intact.
    const int  crcByte = fixture.bytes().size() - 1;
    QByteArray flipped(1, fixture.bytes().at(crcByte) ^ 0xff);
    const QString path = fixture.mutated("trailing_crc.auc", crcByte, flipped);

    US_DataIO::RawData data;
    EXPECT_EQ(US_DataIO::readRawData(path, data), US_DataIO::BADCRC);

    EXPECT_EQ(data.scanCount(), 0);
    EXPECT_TRUE(data.xvalues.isEmpty());
}

TEST(AucReadContract, AnAlreadyPopulatedDestinationIsReplacedRatherThanExtended)
{
    AucFixture fixture;

    US_DataIO::RawData data = makeRawData();
    ASSERT_EQ(data.scanCount(), 2);

    ASSERT_EQ(US_DataIO::readRawData(fixture.validPath(), data), US_DataIO::OK);

    EXPECT_EQ(data.scanCount(), 2);
    EXPECT_EQ(data.xvalues.size(), 8);
}

// ---------------------------------------------------------------------------
// Global settings dependency
// ---------------------------------------------------------------------------

TEST(AucReadContract, ParsedRpmDependsOnTheSetSpeedResoDebugSetting)
{
    AucFixture fixture;

    double defaultRpm = 0.0;
    {
        DebugTextGuard guard(QStringList());
        US_DataIO::RawData data;
        ASSERT_EQ(US_DataIO::readRawData(fixture.validPath(), data), US_DataIO::OK);
        defaultRpm = data.scanData[0].rpm;
    }

    double configuredRpm = 0.0;
    {
        DebugTextGuard guard(QStringList() << "SetSpeedReso=50");
        US_DataIO::RawData data;
        ASSERT_EQ(US_DataIO::readRawData(fixture.validPath(), data), US_DataIO::OK);
        configuredRpm = data.scanData[0].rpm;
    }

    EXPECT_NEAR(defaultRpm, 45100.0, 1.0e-6);
    EXPECT_NEAR(configuredRpm, 45150.0, 1.0e-6);
    EXPECT_NE(defaultRpm, configuredRpm);
}

TEST(AucReadContract, AnUnusableSpeedResolutionSettingFallsBackToTheDefault)
{
    AucFixture fixture;

    const QStringList unusable = QStringList()
        << "SetSpeedResolution="
        << "SetSpeedResolution=abc"
        << "SetSpeedResolution=0"
        << "SetSpeedResolution=-100"
        << "SetSpeedResolution=inf"
        << "SetSpeedResolution=nan";

    for (const QString& setting : unusable)
    {
        DebugTextGuard guard(QStringList() << setting);

        US_DataIO::RawData data;
        ASSERT_EQ(US_DataIO::readRawData(fixture.validPath(), data), US_DataIO::OK)
            << setting.toStdString();
        EXPECT_NEAR(data.scanData[0].rpm, 45100.0, 1.0e-6) << setting.toStdString();
    }
}

TEST(AucReadContract, TheDocumentedSpellingOfTheSpeedResolutionSettingIsAccepted)
{
    AucFixture fixture;

    // The manual spells it SetSpeedResolution; the code matches on the prefix
    // SetSpeedReso, so both forms have to reach the reader.
    DebugTextGuard guard(QStringList() << "SetSpeedResolution=50");

    US_DataIO::RawData data;
    ASSERT_EQ(US_DataIO::readRawData(fixture.validPath(), data), US_DataIO::OK);
    EXPECT_NEAR(data.scanData[0].rpm, 45150.0, 1.0e-6);
}
