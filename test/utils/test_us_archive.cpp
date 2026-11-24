//! \file test_us_archive.cpp
// US_Archive::gzip() writes one complete gzip member and nothing after it.
// The output is decoded here without libarchive.

#include "qt_test_base.h"
#include "us_archive.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QtEndian>

namespace
{
quint32 crc32( const QByteArray& data )
{
   quint32 crc = 0xFFFFFFFFu;
   for ( const char byte : data )
   {
      crc ^= static_cast<quint8>( byte );
      for ( int bit = 0; bit < 8; ++bit )
      {
         crc = ( crc >> 1 ) ^ ( ( crc & 1u ) ? 0xEDB88320u : 0u );
      }
   }
   return ~crc;
}

quint32 adler32( const QByteArray& data )
{
   quint32 low  = 1;
   quint32 high = 0;
   for ( const char byte : data )
   {
      low  = ( low + static_cast<quint8>( byte ) ) % 65521u;
      high = ( high + low ) % 65521u;
   }
   return ( high << 16 ) | low;
}

QByteArray bigEndian( quint32 value )
{
   QByteArray bytes( 4, '\0' );
   qToBigEndian( value, bytes.data() );
   return bytes;
}

quint32 littleEndianAt( const QByteArray& bytes, int offset )
{
   return qFromLittleEndian<quint32>( bytes.constData() + offset );
}

//! Checks that gz is exactly one gzip member holding expected.
//! The trailer is read from the end of the file, so any bytes after the member fail.
void expectGzipOf( const QByteArray& gz, const QByteArray& expected )
{
   ASSERT_GE( gz.size(), 18 );
   EXPECT_EQ( gz.left( 3 ).toHex().toStdString(), "1f8b08" );
   ASSERT_EQ( gz.at( 3 ), '\0' ) << "header has optional fields";

   const int trailer = static_cast<int>( gz.size() ) - 8;
   EXPECT_EQ( littleEndianAt( gz, trailer ), crc32( expected ) );
   EXPECT_EQ( littleEndianAt( gz, trailer + 4 ), static_cast<quint32>( expected.size() ) );

   // qUncompress reads a length and a zlib stream, so wrap the deflate data as one
   const QByteArray deflated = gz.mid( 10, trailer - 10 );
   const QByteArray zlib     = bigEndian( static_cast<quint32>( expected.size() ) ) + QByteArray::fromHex( "789c" ) + deflated +
                           bigEndian( adler32( expected ) );
   EXPECT_EQ( qUncompress( zlib ), expected );
}

QByteArray svgLike( int lines )
{
   QByteArray data( "<svg xmlns=\"http://www.w3.org/2000/svg\">\n" );
   for ( int ii = 0; ii < lines; ++ii )
   {
      data += "<path d=\"M" + QByteArray::number( ii * 7919 % 10007 ) + " " + QByteArray::number( ii ) + "\"/>\n";
   }
   return data + "</svg>\n";
}
} // namespace

class TestUSArchiveGzip : public QtTestBase
{
   protected:
      void SetUp() override
      {
         ASSERT_TRUE( dir.isValid() );
      }

      QString path( const QString& name ) const
      {
         return dir.filePath( name );
      }

      static void write( const QString& name, const QByteArray& data )
      {
         QFile file( name );
         ASSERT_TRUE( file.open( QIODevice::WriteOnly ) );
         ASSERT_EQ( file.write( data ), data.size() );
      }

      static QByteArray read( const QString& name )
      {
         QFile file( name );
         return file.open( QIODevice::ReadOnly ) ? file.readAll() : QByteArray();
      }

      QTemporaryDir dir;
      US_Archive    archive;
};

TEST_F( TestUSArchiveGzip, CompressesAndKeepsTheSource )
{
   const QByteArray data = svgLike( 50 );
   write( path( "plot.svg" ), data );

   ASSERT_TRUE( archive.gzip( path( "plot.svg" ), path( "plot.svgz" ) ) ) << qPrintable( archive.getError() );

   expectGzipOf( read( path( "plot.svgz" ) ), data );
   EXPECT_EQ( read( path( "plot.svg" ) ), data );
   EXPECT_TRUE( archive.getError().isEmpty() );
}

TEST_F( TestUSArchiveGzip, CompressesDataLargerThanTheReadBuffer )
{
   const QByteArray data = svgLike( 20000 );
   ASSERT_GT( data.size(), 4 * 65536 );
   write( path( "plot.svg" ), data );

   ASSERT_TRUE( archive.gzip( path( "plot.svg" ), path( "plot.svgz" ) ) ) << qPrintable( archive.getError() );

   const QByteArray gz = read( path( "plot.svgz" ) );
   expectGzipOf( gz, data );
   EXPECT_LT( gz.size(), data.size() / 2 );
}

TEST_F( TestUSArchiveGzip, CompressesAnEmptyFile )
{
   write( path( "empty.svg" ), QByteArray() );

   ASSERT_TRUE( archive.gzip( path( "empty.svg" ), path( "empty.svgz" ) ) ) << qPrintable( archive.getError() );

   expectGzipOf( read( path( "empty.svgz" ) ), QByteArray() );
}

TEST_F( TestUSArchiveGzip, ReplacesALongerExistingFile )
{
   const QByteArray data = svgLike( 5 );
   write( path( "plot.svg" ), data );
   write( path( "plot.svgz" ), QByteArray( 100000, 'x' ) );

   ASSERT_TRUE( archive.gzip( path( "plot.svg" ), path( "plot.svgz" ) ) ) << qPrintable( archive.getError() );

   expectGzipOf( read( path( "plot.svgz" ) ), data );
}

TEST_F( TestUSArchiveGzip, HandlesPathsOutsideLatin1 )
{
   const QString    folder = path( QString::fromUtf8( "\xce\xb1\xce\xb2 \xe6\xbc\xa2" ) );
   const QByteArray data   = svgLike( 5 );
   ASSERT_TRUE( QDir().mkpath( folder ) );
   write( folder + "/plot.svg", data );

   ASSERT_TRUE( archive.gzip( folder + "/plot.svg", folder + "/plot.svgz" ) ) << qPrintable( archive.getError() );

   expectGzipOf( read( folder + "/plot.svgz" ), data );
}

TEST_F( TestUSArchiveGzip, MissingSourceLeavesTheTargetAlone )
{
   write( path( "plot.svgz" ), "existing" );

   EXPECT_FALSE( archive.gzip( path( "missing.svg" ), path( "plot.svgz" ) ) );

   EXPECT_TRUE( archive.getError().contains( "missing.svg" ) ) << qPrintable( archive.getError() );
   EXPECT_EQ( read( path( "plot.svgz" ) ), QByteArray( "existing" ) );
}

TEST_F( TestUSArchiveGzip, UnwritableTargetFails )
{
   write( path( "plot.svg" ), svgLike( 5 ) );
   const QString target = path( "no-such-folder/plot.svgz" );

   EXPECT_FALSE( archive.gzip( path( "plot.svg" ), target ) );

   EXPECT_TRUE( archive.getError().contains( "no-such-folder" ) ) << qPrintable( archive.getError() );
   EXPECT_FALSE( QFile::exists( target ) );
}

TEST_F( TestUSArchiveGzip, ErrorIsClearedByTheNextSuccess )
{
   write( path( "plot.svg" ), svgLike( 5 ) );
   ASSERT_FALSE( archive.gzip( path( "missing.svg" ), path( "plot.svgz" ) ) );

   ASSERT_TRUE( archive.gzip( path( "plot.svg" ), path( "plot.svgz" ) ) );

   EXPECT_TRUE( archive.getError().isEmpty() );
}
