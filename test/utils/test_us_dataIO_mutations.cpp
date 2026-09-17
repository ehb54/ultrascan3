// Reject malformed AUC files, including mutations with valid checksums.

#include "qt_test_base.h"
#include "us_crc.h"
#include "us_dataIO.h"

#include <QByteArray>
#include <QElapsedTimer>
#include <QFile>
#include <QTemporaryDir>
#include <QtEndian>

namespace
{
// Fixed header layout, from writeRawData():
//   0   magic (4)          4   version (2)      6   type (2)
//   8   cell (1)           9   channel (1)     10   rawGUID (16)
//  26   description (240)
// 266   min_radius (4)   270   max_radius (4) 274   delta_radius (4)
// 278   min_data1 (4)    282   max_data1 (4)  286   min_data2 (4)
// 290   max_data2 (4)    294   scan count (2)
// 296   first scan
constexpr int kMagic          = 0;
constexpr int kVersion        = 4;
constexpr int kType           = 6;
constexpr int kDescPadding    = 250;   // inside the zero padding of a short description
constexpr int kDeltaRadius    = 274;
constexpr int kScanCount      = 294;
constexpr int kHeaderBytes    = 296;
constexpr int kScanHeaderSize = 30;    // DATA + temp + rpm + secs + omega2t + wvl + delta_r + count
constexpr int kScanPointCount = kHeaderBytes + kScanHeaderSize - 4;

// This assertion measures completed reads; CTest enforces the process timeout.
constexpr qint64 kReadBudgetMs = 5000;

US_DataIO::Scan buildScan(int pointCount, double base)
{
    US_DataIO::Scan sc;
    sc.temperature = 20.0;
    sc.rpm         = 45000.0;
    sc.seconds     = 300;
    sc.omega2t     = 2.5e10;
    sc.wavelength  = 280.0;
    sc.delta_r     = 0.01;
    sc.nz_stddev   = false;

    for (int point = 0; point < pointCount; point++)
    {
        sc.rvalues << base + 0.05 * point;
        sc.stddevs << 0.0;
    }

    sc.interpolated = QByteArray((pointCount + 7) / 8, '\0');
    return sc;
}

US_DataIO::RawData buildRawData(int scanCount = 2, int pointCount = 8)
{
    US_DataIO::RawData data;
    memcpy(data.type, "RI", 2);
    memcpy(data.rawGUID, "0123456789abcdef", 16);
    data.cell        = 1;
    data.channel     = 'A';
    data.description = "AUC mutation fixture";

    for (int point = 0; point < pointCount; point++)
        data.xvalues << 5.8 + 0.01 * point;

    for (int scan = 0; scan < scanCount; scan++)
        data.scanData << buildScan(pointCount, 0.2 + scan);

    return data;
}

// Holds the bytes of a written fixture and produces mutated copies of it.
class MutableAuc
{
public:
    explicit MutableAuc(US_DataIO::RawData source = buildRawData())
    {
        EXPECT_TRUE(dir_.isValid());
        const QString file = dir_.path() + "/base.auc";
        EXPECT_EQ(US_DataIO::writeRawData(file, source), US_DataIO::OK);

        QFile ff(file);
        EXPECT_TRUE(ff.open(QIODevice::ReadOnly));
        bytes_ = ff.readAll();
    }

    const QByteArray& bytes() const { return bytes_; }
    QString dir() const { return dir_.path(); }

    // Mutation builder.  Chain the edits, then write().
    class Edit
    {
    public:
        Edit(const MutableAuc& owner, QByteArray bytes)
            : owner_(owner), bytes_(std::move(bytes)) {}

        Edit& patch(int offset, const QByteArray& value)
        {
            EXPECT_LE(offset + value.size(), bytes_.size());
            bytes_.replace(offset, value.size(), value);
            return *this;
        }

        Edit& flipBit(int offset, unsigned char mask)
        {
            EXPECT_LT(offset, bytes_.size());
            bytes_[offset] = static_cast<char>(bytes_.at(offset) ^ mask);
            return *this;
        }

        Edit& patchFloat(int offset, float value)
        {
            union { float f; quint32 u; } uf;
            uf.f = value;
            uchar le[4];
            qToLittleEndian(uf.u, le);
            return patch(offset, QByteArray(reinterpret_cast<char*>(le), 4));
        }

        Edit& patchU16(int offset, quint16 value)
        {
            uchar le[2];
            qToLittleEndian(value, le);
            return patch(offset, QByteArray(reinterpret_cast<char*>(le), 2));
        }

        Edit& patchU32(int offset, quint32 value)
        {
            uchar le[4];
            qToLittleEndian(value, le);
            return patch(offset, QByteArray(reinterpret_cast<char*>(le), 4));
        }

        // Keep the first `keep` bytes of payload, then re-append a checksum
        // slot.  Used to build shorter but still well-formed files.
        Edit& keepThrough(int keep)
        {
            EXPECT_LE(keep + 4, bytes_.size());
            bytes_ = bytes_.left(keep) + QByteArray(4, '\0');
            return *this;
        }

        // Cut the file short with no checksum slot appended, so the reader runs
        // out of bytes mid-structure.
        Edit& truncate(int size)
        {
            EXPECT_LE(size, bytes_.size());
            bytes_ = bytes_.left(size);
            return *this;
        }

        // Update the CRC so structural mutations retain a valid checksum.
        Edit& resealChecksum()
        {
            const int payload = bytes_.size() - 4;
            EXPECT_GT(payload, 0);

            quint32 crc = 0xffffffffUL;
            crc = US_Crc::crc32(
                crc, reinterpret_cast<const unsigned char*>(bytes_.constData()),
                static_cast<unsigned int>(payload));

            uchar le[4];
            qToLittleEndian(crc, le);
            bytes_.replace(payload, 4, QByteArray(reinterpret_cast<char*>(le), 4));
            return *this;
        }

        QString write(const QString& name) const
        {
            const QString target = owner_.dir() + "/" + name;
            QFile ff(target);
            EXPECT_TRUE(ff.open(QIODevice::WriteOnly));
            ff.write(bytes_);
            ff.close();
            return target;
        }

    private:
        const MutableAuc& owner_;
        QByteArray        bytes_;
    };

    Edit edit() const { return Edit(*this, bytes_); }

private:
    QTemporaryDir dir_;
    QByteArray    bytes_;
};

// Check elapsed time after the read returns.
int readWithTimingCheck(const QString& path, US_DataIO::RawData& data)
{
    QElapsedTimer timer;
    timer.start();
    const int result = US_DataIO::readRawData(path, data);
    EXPECT_LT(timer.elapsed(), kReadBudgetMs)
        << "readRawData did not complete within the budget";
    return result;
}
}

// ---------------------------------------------------------------------------
// Control
// ---------------------------------------------------------------------------

TEST(AucMutation, UnchangedFixtureIsTheControl)
{
    MutableAuc base;
    US_DataIO::RawData data;

    ASSERT_EQ(readWithTimingCheck(base.edit().write("m00.auc"), data), US_DataIO::OK);
    EXPECT_EQ(data.scanCount(), 2);
    EXPECT_EQ(data.pointCount(), 8);
    EXPECT_NEAR(data.scanData[0].rvalues[0], 0.2, 1.0e-4);
}

TEST(AucMutation, ResealingWithoutAnyMutationStillReadsAsValid)
{
    MutableAuc base;
    US_DataIO::RawData data;

    const QString path = base.edit().resealChecksum().write("reseal.auc");

    QFile ff(path);
    ASSERT_TRUE(ff.open(QIODevice::ReadOnly));
    EXPECT_EQ(ff.readAll(), base.bytes()) << "reseal changed the bytes it rewrote";

    EXPECT_EQ(readWithTimingCheck(path, data), US_DataIO::OK);
    EXPECT_EQ(data.scanCount(), 2);
}

TEST(AucMutation, SingleMagicByteChangeIsRejected)
{
    MutableAuc base;
    US_DataIO::RawData data;

    EXPECT_EQ(readWithTimingCheck(base.edit().flipBit(kMagic + 2, 0x01).write("m01.auc"), data),
              US_DataIO::NOT_USDATA);
    EXPECT_EQ(data.scanCount(), 0);
}

TEST(AucMutation, UnsupportedVersionIsRejected)
{
    MutableAuc base;
    US_DataIO::RawData data;

    EXPECT_EQ(readWithTimingCheck(base.edit().patch(kVersion, "06").write("m02.auc"), data),
              US_DataIO::BAD_VERSION);
    EXPECT_EQ(data.scanCount(), 0);
}

TEST(AucMutation, UnsupportedTypeIsRejected)
{
    MutableAuc base;
    US_DataIO::RawData data;

    EXPECT_EQ(readWithTimingCheck(base.edit().patch(kType, "ZZ").write("m03.auc"), data),
              US_DataIO::BADTYPE);
    EXPECT_EQ(data.scanCount(), 0);
}

// ---------------------------------------------------------------------------
// Checksum
// ---------------------------------------------------------------------------

TEST(AucMutation, BitFlipInDescriptionPaddingIsCaughtByTheChecksum)
{
    MutableAuc base;
    ASSERT_EQ(base.bytes().at(kDescPadding), '\0')
        << "expected zero padding after a short description";

    US_DataIO::RawData data;
    // Change a payload byte without updating the checksum.
    EXPECT_EQ(readWithTimingCheck(base.edit().flipBit(kDescPadding, 0x01).write("m04.auc"), data),
              US_DataIO::BADCRC);
}

TEST(AucMutation, BitFlipInTheStoredChecksumFieldIsRejected)
{
    MutableAuc base;
    US_DataIO::RawData data;

    EXPECT_EQ(readWithTimingCheck(
                  base.edit().flipBit(base.bytes().size() - 4, 0x80).write("m18.auc"), data),
              US_DataIO::BADCRC);
}

// ---------------------------------------------------------------------------
// Structural rejection that survives a valid checksum
// ---------------------------------------------------------------------------

TEST(AucMutation, CorruptScanMarkerIsRejectedEvenWithAValidChecksum)
{
    MutableAuc base;
    ASSERT_EQ(base.bytes().mid(kHeaderBytes, 4), QByteArray("DATA"));

    US_DataIO::RawData data;
    EXPECT_EQ(readWithTimingCheck(base.edit()
                              .patch(kHeaderBytes, "DAT?")
                              .resealChecksum()
                              .write("m05.auc"),
                          data),
              US_DataIO::NOT_USDATA);

    // Rejected at the first scan, so nothing was appended.
    EXPECT_EQ(data.scanCount(), 0);
}

TEST(AucMutation, ZeroScanFileIsRejectedAsNoData)
{
    MutableAuc base;
    US_DataIO::RawData data;

    // Header only: drop every scan and reseal, so the file is entirely
    // well-formed and simply contains nothing.
    const QString path = base.edit()
                             .keepThrough(kHeaderBytes)
                             .patchU16(kScanCount, 0)
                             .resealChecksum()
                             .write("m10.auc");

    EXPECT_EQ(readWithTimingCheck(path, data), US_DataIO::NODATA);
    EXPECT_EQ(data.scanCount(), 0);
}

TEST(AucMutation, ZeroPointScanIsRejectedAsNoData)
{
    MutableAuc base;
    US_DataIO::RawData data;

    // One scan whose header declares no readings, with the payload removed so
    // the file stays consistent with that declaration.
    const QString path = base.edit()
                             .keepThrough(kHeaderBytes + kScanHeaderSize)
                             .patchU16(kScanCount, 1)
                             .patchU32(kScanPointCount, 0)
                             .resealChecksum()
                             .write("m11.auc");

    EXPECT_EQ(readWithTimingCheck(path, data), US_DataIO::NODATA);
    EXPECT_EQ(data.scanCount(), 0);
}

TEST(AucMutation, ScansWithDifferentPointCountsAreRejected)
{
    // The writer stores each scan's reading count separately.
    US_DataIO::RawData source = buildRawData(2, 8);
    source.scanData[1] = buildScan(16, 1.2);

    MutableAuc base(source);
    US_DataIO::RawData data;

    EXPECT_EQ(readWithTimingCheck(base.edit().write("m13.auc"), data), US_DataIO::NOT_USDATA);
    EXPECT_EQ(data.scanCount(), 0);
}

TEST(AucMutation, NonFiniteRadiusDeltaIsRejected)
{
    MutableAuc base;
    US_DataIO::RawData data;

    const QString path = base.edit()
                             .patchFloat(kDeltaRadius, std::numeric_limits<float>::quiet_NaN())
                             .resealChecksum()
                             .write("m14.auc");

    EXPECT_EQ(readWithTimingCheck(path, data), US_DataIO::NOT_USDATA);
    EXPECT_TRUE(data.xvalues.isEmpty());
}

TEST(AucMutation, ZeroRadiusDeltaIsRejected)
{
    MutableAuc base;
    US_DataIO::RawData data;

    const QString path = base.edit()
                             .patchFloat(kDeltaRadius, 0.0f)
                             .resealChecksum()
                             .write("m15.auc");

    // Every reading would otherwise collapse onto a single radius, which is not
    // a usable coordinate axis.
    EXPECT_EQ(readWithTimingCheck(path, data), US_DataIO::NOT_USDATA);
    EXPECT_TRUE(data.xvalues.isEmpty());
}

TEST(AucMutation, NegativeRadiusDeltaIsRejected)
{
    MutableAuc base;
    US_DataIO::RawData data;

    const QString path = base.edit()
                             .patchFloat(kDeltaRadius, -0.01f)
                             .resealChecksum()
                             .write("m15b.auc");

    // A descending axis breaks the ordering that xindex() and every consumer of
    // it assume.
    EXPECT_EQ(readWithTimingCheck(path, data), US_DataIO::NOT_USDATA);
    EXPECT_TRUE(data.xvalues.isEmpty());
}

TEST(AucMutation, TruncationInsideTheFixedHeaderIsRejected)
{
    MutableAuc base;

    // Every offset from the magic number through the scan count.  Each must be
    // a clean rejection rather than a read of uninitialized memory.
    for (int size = 0; size < kHeaderBytes; size++)
    {
        US_DataIO::RawData data;
        const QString path = base.edit()
                                 .truncate(size)
                                 .write(QString("m06_%1.auc").arg(size));

        const int result = readWithTimingCheck(path, data);
        EXPECT_NE(result, US_DataIO::OK) << "at truncation size " << size;
        EXPECT_EQ(data.scanCount(), 0)   << "at truncation size " << size;
    }
}

TEST(AucMutation, TruncationInsideAScanHeaderIsRejected)
{
    MutableAuc base;

    for (int size = kHeaderBytes; size < kHeaderBytes + kScanHeaderSize; size++)
    {
        US_DataIO::RawData data;
        const QString path = base.edit()
                                 .truncate(size)
                                 .write(QString("m07_%1.auc").arg(size));

        EXPECT_NE(readWithTimingCheck(path, data), US_DataIO::OK) << "at truncation size " << size;
        EXPECT_EQ(data.scanCount(), 0)                    << "at truncation size " << size;
    }
}

TEST(AucMutation, TruncationInsideTheReadingsIsRejected)
{
    MutableAuc base;
    const int firstReading = kHeaderBytes + kScanHeaderSize;

    for (int size = firstReading; size < firstReading + 16; size++)
    {
        US_DataIO::RawData data;
        const QString path = base.edit()
                                 .truncate(size)
                                 .write(QString("m08_%1.auc").arg(size));

        EXPECT_NE(readWithTimingCheck(path, data), US_DataIO::OK) << "at truncation size " << size;
        EXPECT_EQ(data.scanCount(), 0)                    << "at truncation size " << size;
    }
}

TEST(AucMutation, TruncationOfTheTrailingChecksumIsRejected)
{
    MutableAuc base;
    const int full = base.bytes().size();

    for (int missing = 1; missing <= 4; missing++)
    {
        US_DataIO::RawData data;
        const QString path = base.edit()
                                 .truncate(full - missing)
                                 .write(QString("m09_%1.auc").arg(missing));

        EXPECT_EQ(readWithTimingCheck(path, data), US_DataIO::NOT_USDATA) << "missing " << missing;
        EXPECT_EQ(data.scanCount(), 0)                            << "missing " << missing;
    }
}

TEST(AucMutation, PointCountBeyondTheAvailableBytesIsRejected)
{
    MutableAuc base;
    US_DataIO::RawData data;

    const QString path = base.edit()
                             .patchU32(kScanPointCount, 0x3fffffffu)
                             .resealChecksum()
                             .write("m12.auc");

    EXPECT_EQ(readWithTimingCheck(path, data), US_DataIO::NOT_USDATA);
    EXPECT_EQ(data.scanCount(), 0);
}

TEST(AucMutation, NegativePointCountIsRejected)
{
    MutableAuc base;
    US_DataIO::RawData data;

    // The count is read into a signed 32-bit value, so the high bit makes it
    // negative rather than merely large.
    const QString path = base.edit()
                             .patchU32(kScanPointCount, 0xffffffffu)
                             .resealChecksum()
                             .write("m12b.auc");

    EXPECT_EQ(readWithTimingCheck(path, data), US_DataIO::NODATA);
    EXPECT_EQ(data.scanCount(), 0);
}

TEST(AucMutation, NegativeScanCountIsRejected)
{
    MutableAuc base;
    US_DataIO::RawData data;

    // scan_count is a qint16, so 0x8000 is the most negative value it can hold.
    const QString path = base.edit()
                             .patchU16(kScanCount, 0x8000u)
                             .resealChecksum()
                             .write("negscans.auc");

    EXPECT_EQ(readWithTimingCheck(path, data), US_DataIO::NODATA);
    EXPECT_EQ(data.scanCount(), 0);
}

TEST(AucMutation, ScanCountBeyondTheAvailableBytesIsRejected)
{
    MutableAuc base;
    US_DataIO::RawData data;

    const QString path = base.edit()
                             .patchU16(kScanCount, 0x7fffu)
                             .resealChecksum()
                             .write("bigscans.auc");

    EXPECT_EQ(readWithTimingCheck(path, data), US_DataIO::NOT_USDATA);
    EXPECT_EQ(data.scanCount(), 0);
}

TEST(AucMutation, ARejectedReadLeavesAnEmptyDestinationUntouched)
{
    MutableAuc base;
    US_DataIO::RawData data;

    const QString path = base.edit()
                             .flipBit(kHeaderBytes + kScanHeaderSize, 0x01)
                             .write("t05_empty.auc");

    ASSERT_EQ(readWithTimingCheck(path, data), US_DataIO::BADCRC);
    EXPECT_EQ(data.scanCount(), 0);
    EXPECT_TRUE(data.xvalues.isEmpty());
    EXPECT_TRUE(data.description.isEmpty());
}

TEST(AucMutation, ARejectedReadLeavesAPopulatedDestinationUntouched)
{
    MutableAuc base;
    US_DataIO::RawData data;

    // Load a good file first, then fail a read into the same object.
    ASSERT_EQ(readWithTimingCheck(base.edit().write("t05_good.auc"), data), US_DataIO::OK);
    ASSERT_EQ(data.scanCount(), 2);
    const QString    description = data.description;
    const QVector<double> xvalues = data.xvalues;

    const QString bad = base.edit()
                            .flipBit(kHeaderBytes + kScanHeaderSize, 0x01)
                            .write("t05_bad.auc");

    ASSERT_EQ(readWithTimingCheck(bad, data), US_DataIO::BADCRC);

    EXPECT_EQ(data.scanCount(), 2);
    EXPECT_EQ(data.pointCount(), 8);
    EXPECT_EQ(data.description, description);
    EXPECT_EQ(data.xvalues, xvalues);
}

TEST(AucMutation, ASuccessfulReadReplacesRatherThanAppends)
{
    MutableAuc base;
    US_DataIO::RawData data;

    const QString path = base.edit().write("t05_reuse.auc");

    ASSERT_EQ(readWithTimingCheck(path, data), US_DataIO::OK);
    ASSERT_EQ(data.scanCount(), 2);

    // Reading the same file into the same object must not double the scans.
    ASSERT_EQ(readWithTimingCheck(path, data), US_DataIO::OK);
    EXPECT_EQ(data.scanCount(), 2);
    EXPECT_EQ(data.xvalues.size(), 8);
}
