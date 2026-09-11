// AUC-T03: which historical .auc format versions readRawData() must accept.
//
// The read-contract suite could only patch the version byte of a version 5
// file, which the checksum then rejects, so historical acceptance was left
// open.  These cases synthesize whole files at the older layouts -- checksum
// recomputed over the bytes actually emitted -- and pin the answer:
//
//   version 5  current writer output.
//   version 4  identical layout, wavelength on the pre-v5 scale; accepted.
//   version 3  no cell/channel byte pair; every later field is displaced.
//   version 2  also stores the radius limits as 2-byte integers.
//
// Versions 2 and 3 were dropped upstream in 2010 when cell and channel were
// added to the header ("Updates format version and invalidates old auc files"),
// and the version-2 read workaround was deleted in the same commit.  Nothing at
// those versions has been loadable since, which is what the cases below prove
// before the reader started naming them as unsupported versions.

#include "qt_test_base.h"
#include "us_crc.h"
#include "us_dataIO.h"

#include <QByteArray>
#include <QFile>
#include <QTemporaryDir>
#include <QtEndian>

namespace
{
// Header layout of a version 5 file, from writeRawData():  4 magic + 2 version
// + 2 type + 1 cell + 1 channel + 16 GUID + 240 description = 266, then seven
// 4-byte floats, then the 2-byte scan count.
constexpr int kVersionOffset = 4;
constexpr int kCellOffset    = 8;
constexpr int kFloatsOffset  = 266;

constexpr double kWavelength = 280.0;

US_DataIO::RawData makeRawData()
{
    US_DataIO::RawData data;
    memcpy(data.type, "RA", 2);
    memcpy(data.rawGUID, "0123456789abcdef", 16);
    data.cell        = 3;
    data.channel     = 'A';
    data.description = "AUC-T03 version fixture";

    for (int point = 0; point < 8; point++)
        data.xvalues << 5.8 + 0.01 * point;

    for (int scan = 0; scan < 2; scan++)
    {
        US_DataIO::Scan sc;
        sc.temperature = 20.0;
        sc.rpm         = 45000.0;
        sc.seconds     = 100 * (scan + 1);
        sc.omega2t     = 1.0e10 * (scan + 1);
        sc.wavelength  = kWavelength;
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

// Replace the trailing checksum with one computed over the preceding bytes, so
// a rewritten file is internally consistent and reaches the layout checks
// rather than short-circuiting on BADCRC.
QByteArray resealed(QByteArray body)
{
    const quint32 crc =
        US_Crc::crc32(0xffffffffUL,
                      reinterpret_cast<const unsigned char*>(body.constData()),
                      static_cast<unsigned int>(body.size()));

    quint32 stored = 0;
    qToLittleEndian(crc, reinterpret_cast<uchar*>(&stored));
    body.append(reinterpret_cast<const char*>(&stored), 4);
    return body;
}

QString writeBytes(const QTemporaryDir& dir, const QString& name,
                   const QByteArray& bytes)
{
    const QString target = dir.path() + "/" + name;
    QFile ff(target);
    EXPECT_TRUE(ff.open(QIODevice::WriteOnly));
    ff.write(bytes);
    ff.close();
    return target;
}

// A freshly written version 5 file, minus its checksum.
QByteArray currentBody(const QTemporaryDir& dir)
{
    US_DataIO::RawData source = makeRawData();
    const QString      path   = dir.path() + "/v5.auc";
    EXPECT_EQ(US_DataIO::writeRawData(path, source), US_DataIO::OK);

    QFile ff(path);
    EXPECT_TRUE(ff.open(QIODevice::ReadOnly));
    QByteArray bytes = ff.readAll();
    return bytes.left(bytes.size() - 4);
}
}

// ---------------------------------------------------------------------------
// Accepted versions
// ---------------------------------------------------------------------------

TEST(AucVersions, TheCurrentWriterEmitsVersionFive)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    const QByteArray body = currentBody(dir);
    ASSERT_GT(body.size(), kFloatsOffset);
    EXPECT_EQ(body.mid(kVersionOffset, 2), QByteArray("05"));
    EXPECT_EQ(US_DataIO::format_version, 5u);
}

TEST(AucVersions, AVersionFourFileLoadsAndDecodesWavelengthByTheOlderRule)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    // Version 4 has the layout of version 5, so only the version bytes change.
    // It differs solely in the wavelength scale: v4 stored (nm - 180) * 100,
    // v5 stores nm * 10, and the reader picks the rule from the version.
    QByteArray body = currentBody(dir);
    body.replace(kVersionOffset, 2, "04");

    US_DataIO::RawData data;
    ASSERT_EQ(US_DataIO::readRawData(
                  writeBytes(dir, "v4.auc", resealed(body)), data),
              US_DataIO::OK);

    ASSERT_EQ(data.scanCount(), 2);
    EXPECT_EQ(data.cell, 3);
    EXPECT_EQ(data.channel, 'A');
    EXPECT_EQ(data.pointCount(), 8);

    // The bytes hold 280.0 nm written by the v5 rule -- 2800 -- and reading
    // them as v4 yields 2800 / 100 + 180.  A v4 file recorded at 280 nm holds
    // 10000 instead; either way the decode follows the version, not the writer.
    EXPECT_NEAR(data.scanData[0].wavelength, 208.0, 1.0e-6);
}

// ---------------------------------------------------------------------------
// Rejected versions
// ---------------------------------------------------------------------------

TEST(AucVersions, AVersionThreeFileIsRejectedAsAnUnsupportedVersion)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    // Version 3 wrote no cell or channel byte, so drop them and reseal.  With
    // the version floor removed this file returns NOT_USDATA instead: the two
    // bytes the reader takes for cell and channel displace everything after
    // them, and the first scan marker is no longer where it is looked for.
    // Nothing is lost by naming the version as the reason.
    QByteArray body = currentBody(dir);
    body.replace(kVersionOffset, 2, "03");
    body.remove(kCellOffset, 2);

    US_DataIO::RawData data;
    EXPECT_EQ(US_DataIO::readRawData(
                  writeBytes(dir, "v3.auc", resealed(body)), data),
              US_DataIO::BAD_VERSION);
    EXPECT_EQ(data.scanCount(), 0);
}

TEST(AucVersions, AVersionTwoFileIsRejectedAsAnUnsupportedVersion)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    // Version 2 also stored the first two radius values as 2-byte integers of
    // millimeters rather than 4-byte floats.  With the version floor removed
    // this file returns NODATA -- the displaced bytes decode to a reading count
    // that is not positive.
    QByteArray body = currentBody(dir);
    body.replace(kVersionOffset, 2, "02");
    body.remove(kCellOffset, 2);

    QByteArray radii(4, '\0');
    qToLittleEndian(static_cast<quint16>(5800),
                    reinterpret_cast<uchar*>(radii.data()));
    qToLittleEndian(static_cast<quint16>(5870),
                    reinterpret_cast<uchar*>(radii.data() + 2));
    body.replace(kFloatsOffset - 2, 8, radii);

    US_DataIO::RawData data;
    EXPECT_EQ(US_DataIO::readRawData(
                  writeBytes(dir, "v2.auc", resealed(body)), data),
              US_DataIO::BAD_VERSION);
    EXPECT_EQ(data.scanCount(), 0);
}

TEST(AucVersions, AFileWithNoVersionFieldAtAllIsRejected)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    // Before October 2009 the magic number was followed directly by the data
    // type, so the two type bytes land where the version is read.  "RA" decodes
    // to 513, far above the current format version.
    QByteArray body = currentBody(dir);
    body.remove(kVersionOffset, 2);

    US_DataIO::RawData data;
    EXPECT_EQ(US_DataIO::readRawData(
                  writeBytes(dir, "v0.auc", resealed(body)), data),
              US_DataIO::BAD_VERSION);
    EXPECT_EQ(data.scanCount(), 0);
}
