//! \file test_us_dataIO_load.cpp
// Scan metadata after US_DataIO::loadData reads a raw file and its edits.

#include "qt_test_base.h"
#include "us_dataIO.h"
#include "us_util.h"

#include <QTemporaryDir>

namespace
{
#if defined( _MSC_VER )
#define TEST_NOINLINE __declspec( noinline )
#else
#define TEST_NOINLINE __attribute__( ( noinline ) )
#endif

// Leaves a byte pattern on the stack where the next call's locals will be, so a
// scan value loadData forgets to set reads as the pattern rather than 0 by luck.
TEST_NOINLINE void dirtyStack()
{
   volatile unsigned char junk[65536];
   for ( size_t ii = 0; ii < sizeof( junk ); ii++ )
   {
      junk[ii] = 0xAA;
   }
}

const QString      kRawName  = "plateau.RA.1.A.280.auc";
const QString      kEditName = "plateau.2401010000.RA.1.A.280.xml";

US_DataIO::RawData rawWithScans( int scanCount )
{
   US_DataIO::RawData raw;
   memcpy( raw.type, "RA", 2 );
   memcpy( raw.rawGUID, "0123456789abcdef", 16 );
   raw.cell        = 1;
   raw.channel     = 'A';
   raw.description = "plateau fixture";

   for ( int point = 0; point < 11; point++ )
   {
      raw.xvalues << 5.8 + 0.1 * point;
   }

   for ( int scan = 0; scan < scanCount; scan++ )
   {
      US_DataIO::Scan sc{};
      sc.temperature = 20.0;
      sc.rpm         = 50000.0;
      sc.seconds     = 300 + 60 * scan;
      sc.omega2t     = 2.0e10 * ( scan + 1 );
      sc.wavelength  = 280.0;
      sc.delta_r     = 0.1;
      for ( int point = 0; point < raw.xvalues.size(); point++ )
      {
         sc.rvalues << 0.1 * ( scan + 1 ) + 0.01 * point;
      }
      sc.interpolated = QByteArray( 2, '\0' );
      raw.scanData << sc;
   }

   return raw;
}

US_DataIO::EditValues editsFor( const US_DataIO::RawData& raw )
{
   US_DataIO::EditValues ev;
   ev.expType    = "Velocity";
   ev.runID      = "plateau";
   ev.cell       = "1";
   ev.channel    = "A";
   ev.wavelength = "280";
   ev.editGUID   = "11111111-2222-3333-4444-555555555555";
   ev.dataGUID   = US_Util::uuid_unparse( (uchar*) const_cast<char*>( raw.rawGUID ) );
   ev.meniscus   = 5.85;
   ev.bottom     = 7.2;
   ev.rangeLeft  = 5.9;
   ev.rangeRight = 6.6;
   ev.plateau    = 6.3;
   ev.baseline   = 5.9;
   ev.ODlimit    = 1.5;
   ev.excludes << 1;
   return ev;
}
} // namespace

class US_DataIOLoadTest : public QtTestBase
{
};

// Raw scans carry no plateau and the raw file does not store one; loadData gives
// every edited scan the reading at the edit's plateau radius.
TEST_F( US_DataIOLoadTest, EveryEditedScanHasThePlateauReading )
{
   QTemporaryDir dir;
   ASSERT_TRUE( dir.isValid() );
   US_DataIO::RawData raw = rawWithScans( 4 );
   ASSERT_EQ( US_DataIO::writeRawData( dir.filePath( kRawName ), raw ), US_DataIO::OK );
   ASSERT_EQ( US_DataIO::writeEdits( dir.filePath( kEditName ), editsFor( raw ) ), US_DataIO::OK );

   for ( int round = 0; round < 3; round++ )
   {
      dirtyStack();
      QVector<US_DataIO::EditedData> edited;
      QVector<US_DataIO::RawData>    raws;
      ASSERT_EQ( US_DataIO::loadData( dir.path(), kEditName, edited, raws ), US_DataIO::OK );
      ASSERT_EQ( edited.size(), 1 );

      US_DataIO::EditedData& ed = edited[0];
      ASSERT_EQ( ed.scanData.size(), 3 ); // scan 1 is excluded
      const int point = ed.xindex( 6.3 );
      ASSERT_GT( point, 0 );

      for ( const US_DataIO::Scan& scan : ed.scanData )
      {
         EXPECT_EQ( scan.plateau, scan.rvalues[point] );
      }
   }
}
