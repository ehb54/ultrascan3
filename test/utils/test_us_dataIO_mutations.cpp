// AUC-T03/T04/T05: mutation recipes against the public filename API.
//
// Each case implements a named recipe from AUC_MUTATION_TEST_ANALYSIS.md and
// carries its AUC-Mxx identifier.  Mutations are generated at runtime from a
// fixture written by the production writer -- there is no collection of
// hand-edited binaries.
//
// Two capabilities make this suite different from AUC-T01's recording pass:
//
//   * resealChecksum() recomputes the trailing CRC-32 the way the writer does,
//     so a fixture can be made structurally invalid *and* checksum-valid.
//     Without that, a structural mutation is indistinguishable from payload
//     corruption -- the earlier check simply wins and proves nothing.
//   * every mutation read is bounded by an explicit time budget, so a recipe
//     that starts looping on a corrupt count fails as a test rather than
//     hanging the suite.
//
// The truncation family (AUC-M06..M09) and the hostile counts (AUC-M12) were
// deferred from AUC-T03 because running them before the reader checked its read
// lengths meant exercising undefined behavior.  AUC-T04 added that checking, so
// they are implemented below.
//
// See the AUC-T03 report for the recipes that cannot be built by patching.

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

// A mutation must never take longer than a well-formed read.  Generous enough
// not to be flaky under a loaded CI machine, tight enough that an unbounded
// loop on a corrupt count is a failure rather than a hang.
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
    data.description = "AUC-T03 mutation base";

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

        // Recompute the trailing CRC-32 over everything before it, exactly as
        // writeRawData() accumulates it.  This is what lets a mutation be
        // structurally wrong yet checksum-valid.
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

// Reads a mutation, asserting the call completes inside the budget.
int readBounded(const QString& path, US_DataIO::RawData& data)
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

TEST(AucMutation, M00_UnchangedFixtureIsTheControl)
{
    MutableAuc base;
    US_DataIO::RawData data;

    ASSERT_EQ(readBounded(base.edit().write("m00.auc"), data), US_DataIO::OK);
    EXPECT_EQ(data.scanCount(), 2);
    EXPECT_EQ(data.pointCount(), 8);
    EXPECT_NEAR(data.scanData[0].rvalues[0], 0.2, 1.0e-4);
}

TEST(AucMutation, ResealingWithoutAnyMutationStillReadsAsValid)
{
    MutableAuc base;
    US_DataIO::RawData data;

    // Validates the mutation tool, not the parser.  Several recipes below are
    // meaningful only if resealChecksum() really produces a checksum the reader
    // accepts; if it were wrong, those cases would return BADCRC and their
    // structural findings would be accidents.  A reseal with no edit must be
    // indistinguishable from the original.
    const QString path = base.edit().resealChecksum().write("reseal.auc");

    QFile ff(path);
    ASSERT_TRUE(ff.open(QIODevice::ReadOnly));
    EXPECT_EQ(ff.readAll(), base.bytes()) << "reseal changed the bytes it rewrote";

    EXPECT_EQ(readBounded(path, data), US_DataIO::OK);
    EXPECT_EQ(data.scanCount(), 2);
}

// ---------------------------------------------------------------------------
// Header rejections, checked before the payload is trusted
// ---------------------------------------------------------------------------

TEST(AucMutation, M01_SingleMagicByteChangeIsRejected)
{
    MutableAuc base;
    US_DataIO::RawData data;

    // One byte, not the whole field: the magic check must not depend on how
    // much of it was damaged.  No reseal -- magic is checked long before the CRC.
    EXPECT_EQ(readBounded(base.edit().flipBit(kMagic + 2, 0x01).write("m01.auc"), data),
              US_DataIO::NOT_USDATA);
    EXPECT_EQ(data.scanCount(), 0);
}

TEST(AucMutation, M02_UnsupportedVersionIsRejected)
{
    MutableAuc base;
    US_DataIO::RawData data;

    EXPECT_EQ(readBounded(base.edit().patch(kVersion, "06").write("m02.auc"), data),
              US_DataIO::BAD_VERSION);
    EXPECT_EQ(data.scanCount(), 0);
}

TEST(AucMutation, M03_UnsupportedTypeIsRejected)
{
    MutableAuc base;
    US_DataIO::RawData data;

    EXPECT_EQ(readBounded(base.edit().patch(kType, "ZZ").write("m03.auc"), data),
              US_DataIO::BADTYPE);
    EXPECT_EQ(data.scanCount(), 0);
}

// ---------------------------------------------------------------------------
// Checksum
// ---------------------------------------------------------------------------

TEST(AucMutation, M04_BitFlipInDescriptionPaddingIsCaughtByTheChecksum)
{
    MutableAuc base;
    ASSERT_EQ(base.bytes().at(kDescPadding), '\0')
        << "expected zero padding after a short description";

    US_DataIO::RawData data;
    // The layout is untouched -- no count, marker or field changes size.  This
    // is the recipe that proves the checksum, and only the checksum, rejects it.
    EXPECT_EQ(readBounded(base.edit().flipBit(kDescPadding, 0x01).write("m04.auc"), data),
              US_DataIO::BADCRC);
}

TEST(AucMutation, M18_BitFlipInTheStoredChecksumFieldIsRejected)
{
    MutableAuc base;
    US_DataIO::RawData data;

    EXPECT_EQ(readBounded(
                  base.edit().flipBit(base.bytes().size() - 4, 0x80).write("m18.auc"), data),
              US_DataIO::BADCRC);
}

// ---------------------------------------------------------------------------
// Structural rejection that survives a valid checksum
// ---------------------------------------------------------------------------

TEST(AucMutation, M05_CorruptScanMarkerIsRejectedEvenWithAValidChecksum)
{
    MutableAuc base;
    ASSERT_EQ(base.bytes().mid(kHeaderBytes, 4), QByteArray("DATA"));

    US_DataIO::RawData data;
    // The reseal makes this fixture genuinely checksum-valid, so the rejection
    // is provably structural rather than incidental corruption.  The marker
    // check happens to run before the checksum is verified, so the verdict is
    // the same either way -- but only the resealed fixture proves the reader
    // rejects well-formed-but-structurally-wrong input.
    EXPECT_EQ(readBounded(base.edit()
                              .patch(kHeaderBytes, "DAT?")
                              .resealChecksum()
                              .write("m05.auc"),
                          data),
              US_DataIO::NOT_USDATA);

    // Rejected at the first scan, so nothing was appended.
    EXPECT_EQ(data.scanCount(), 0);
}

// ---------------------------------------------------------------------------
// Structurally degenerate input that is currently ACCEPTED
//
// These recipes are structurally invalid in ways the checksum cannot catch, so
// each is resealed and each depends on the validation AUC-T04 added.  Before
// that issue every one of them loaded successfully.
// ---------------------------------------------------------------------------

TEST(AucMutation, M10_ZeroScanFileIsRejectedAsNoData)
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

    EXPECT_EQ(readBounded(path, data), US_DataIO::NODATA);
    EXPECT_EQ(data.scanCount(), 0);
}

TEST(AucMutation, M11_ZeroPointScanIsRejectedAsNoData)
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

    EXPECT_EQ(readBounded(path, data), US_DataIO::NODATA);
    EXPECT_EQ(data.scanCount(), 0);
}

TEST(AucMutation, M13_ScansWithDifferentPointCountsAreRejected)
{
    // Built rather than patched: the writer emits each scan's own reading count,
    // so a RawData whose scans disagree produces a valid file directly.
    US_DataIO::RawData source = buildRawData(2, 8);
    source.scanData[1] = buildScan(16, 1.2);

    MutableAuc base(source);
    US_DataIO::RawData data;

    // Downstream code shares one xvalues vector across every scan, so mismatched
    // dimensions were a latent out-of-bounds read in every consumer: pointCount()
    // reported the last scan's 16 while scan 0 held only 8 readings.
    EXPECT_EQ(readBounded(base.edit().write("m13.auc"), data), US_DataIO::NOT_USDATA);
    EXPECT_EQ(data.scanCount(), 0);
}

TEST(AucMutation, M14_NonFiniteRadiusDeltaIsRejected)
{
    MutableAuc base;
    US_DataIO::RawData data;

    const QString path = base.edit()
                             .patchFloat(kDeltaRadius, std::numeric_limits<float>::quiet_NaN())
                             .resealChecksum()
                             .write("m14.auc");

    // A non-finite coordinate axis is structurally unusable regardless of any
    // physical range policy.  It previously produced a finite origin followed by
    // an entire vector of NaN.
    EXPECT_EQ(readBounded(path, data), US_DataIO::NOT_USDATA);
    EXPECT_TRUE(data.xvalues.isEmpty());
}

TEST(AucMutation, M15_ZeroRadiusDeltaIsRejected)
{
    MutableAuc base;
    US_DataIO::RawData data;

    const QString path = base.edit()
                             .patchFloat(kDeltaRadius, 0.0f)
                             .resealChecksum()
                             .write("m15.auc");

    // Every reading would otherwise collapse onto a single radius, which is not
    // a usable coordinate axis.
    EXPECT_EQ(readBounded(path, data), US_DataIO::NOT_USDATA);
    EXPECT_TRUE(data.xvalues.isEmpty());
}

TEST(AucMutation, M15b_NegativeRadiusDeltaIsRejected)
{
    MutableAuc base;
    US_DataIO::RawData data;

    const QString path = base.edit()
                             .patchFloat(kDeltaRadius, -0.01f)
                             .resealChecksum()
                             .write("m15b.auc");

    // A descending axis breaks the ordering that xindex() and every consumer of
    // it assume.
    EXPECT_EQ(readBounded(path, data), US_DataIO::NOT_USDATA);
    EXPECT_TRUE(data.xvalues.isEmpty());
}

// ---------------------------------------------------------------------------
// Truncation and hostile counts
//
// These were deferred from AUC-T03: before AUC-T04 the reader ignored the
// return of QDataStream::readRawData(), so a truncated file was parsed out of
// whatever the stack buffers already held and the checksum was computed over
// that -- undefined, not merely wrong.  M12 additionally drove a loop and a
// heap allocation from an untrusted 32-bit count.
// ---------------------------------------------------------------------------

TEST(AucMutation, M06_TruncationInsideTheFixedHeaderIsRejected)
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

        const int result = readBounded(path, data);
        EXPECT_NE(result, US_DataIO::OK) << "at truncation size " << size;
        EXPECT_EQ(data.scanCount(), 0)   << "at truncation size " << size;
    }
}

TEST(AucMutation, M07_TruncationInsideAScanHeaderIsRejected)
{
    MutableAuc base;

    for (int size = kHeaderBytes; size < kHeaderBytes + kScanHeaderSize; size++)
    {
        US_DataIO::RawData data;
        const QString path = base.edit()
                                 .truncate(size)
                                 .write(QString("m07_%1.auc").arg(size));

        EXPECT_NE(readBounded(path, data), US_DataIO::OK) << "at truncation size " << size;
        EXPECT_EQ(data.scanCount(), 0)                    << "at truncation size " << size;
    }
}

TEST(AucMutation, M08_TruncationInsideTheReadingsIsRejected)
{
    MutableAuc base;
    const int firstReading = kHeaderBytes + kScanHeaderSize;

    for (int size = firstReading; size < firstReading + 16; size++)
    {
        US_DataIO::RawData data;
        const QString path = base.edit()
                                 .truncate(size)
                                 .write(QString("m08_%1.auc").arg(size));

        EXPECT_NE(readBounded(path, data), US_DataIO::OK) << "at truncation size " << size;
        EXPECT_EQ(data.scanCount(), 0)                    << "at truncation size " << size;
    }
}

TEST(AucMutation, M09_TruncationOfTheTrailingChecksumIsRejected)
{
    MutableAuc base;
    const int full = base.bytes().size();

    // One, two and three checksum bytes present instead of four.  This is the
    // path that previously compared the crc against uninitialized stack.
    for (int missing = 1; missing <= 4; missing++)
    {
        US_DataIO::RawData data;
        const QString path = base.edit()
                                 .truncate(full - missing)
                                 .write(QString("m09_%1.auc").arg(missing));

        EXPECT_EQ(readBounded(path, data), US_DataIO::NOT_USDATA) << "missing " << missing;
        EXPECT_EQ(data.scanCount(), 0)                            << "missing " << missing;
    }
}

TEST(AucMutation, M12_PointCountBeyondTheAvailableBytesIsRejected)
{
    MutableAuc base;
    US_DataIO::RawData data;

    // A count far larger than the file, resealed so nothing earlier rejects it.
    // Before the bounds check this drove both the reading loop and a
    // new char[] allocation from this value.
    const QString path = base.edit()
                             .patchU32(kScanPointCount, 0x3fffffffu)
                             .resealChecksum()
                             .write("m12.auc");

    EXPECT_EQ(readBounded(path, data), US_DataIO::NOT_USDATA);
    EXPECT_EQ(data.scanCount(), 0);
}

TEST(AucMutation, M12b_NegativePointCountIsRejected)
{
    MutableAuc base;
    US_DataIO::RawData data;

    // The count is read into a signed 32-bit value, so the high bit makes it
    // negative rather than merely large.
    const QString path = base.edit()
                             .patchU32(kScanPointCount, 0xffffffffu)
                             .resealChecksum()
                             .write("m12b.auc");

    EXPECT_EQ(readBounded(path, data), US_DataIO::NODATA);
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

    EXPECT_EQ(readBounded(path, data), US_DataIO::NODATA);
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

    EXPECT_EQ(readBounded(path, data), US_DataIO::NOT_USDATA);
    EXPECT_EQ(data.scanCount(), 0);
}

// ---------------------------------------------------------------------------
// AUC-T05: failure output is atomic
//
// The documented guarantee is "unchanged": a failed read leaves the caller's
// RawData exactly as it was.  Previously scans were appended as they parsed and
// the checksum was verified only at the end, so a rejected file left plausible
// looking data behind and a reused destination accumulated scans.
// ---------------------------------------------------------------------------

TEST(AucMutation, ARejectedReadLeavesAnEmptyDestinationUntouched)
{
    MutableAuc base;
    US_DataIO::RawData data;

    // Corrupt a payload byte so the file parses completely and fails only at
    // the very end, on the checksum.  This is the case that used to leave two
    // fully populated scans in the destination.
    const QString path = base.edit()
                             .flipBit(kHeaderBytes + kScanHeaderSize, 0x01)
                             .write("t05_empty.auc");

    ASSERT_EQ(readBounded(path, data), US_DataIO::BADCRC);
    EXPECT_EQ(data.scanCount(), 0);
    EXPECT_TRUE(data.xvalues.isEmpty());
    EXPECT_TRUE(data.description.isEmpty());
}

TEST(AucMutation, ARejectedReadLeavesAPopulatedDestinationUntouched)
{
    MutableAuc base;
    US_DataIO::RawData data;

    // Load a good file first, then fail a read into the same object.
    ASSERT_EQ(readBounded(base.edit().write("t05_good.auc"), data), US_DataIO::OK);
    ASSERT_EQ(data.scanCount(), 2);
    const QString    description = data.description;
    const QVector<double> xvalues = data.xvalues;

    const QString bad = base.edit()
                            .flipBit(kHeaderBytes + kScanHeaderSize, 0x01)
                            .write("t05_bad.auc");

    ASSERT_EQ(readBounded(bad, data), US_DataIO::BADCRC);

    // Not 4:  the previous contents survive intact rather than accumulating.
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

    ASSERT_EQ(readBounded(path, data), US_DataIO::OK);
    ASSERT_EQ(data.scanCount(), 2);

    // Reading the same file into the same object must not double the scans.
    ASSERT_EQ(readBounded(path, data), US_DataIO::OK);
    EXPECT_EQ(data.scanCount(), 2);
    EXPECT_EQ(data.xvalues.size(), 8);
}
