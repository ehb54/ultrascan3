//! \file test_us_buffer.cpp
#include "qt_test_base.h"
#include "mock_us_db2.h"
#include "us_buffer.h"
#include "us_constants.h"
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QXmlStreamWriter>
#include <cstring>

using ::testing::_;
using ::testing::DoAll;
using ::testing::DoubleEq;
using ::testing::DoubleNear;
using ::testing::Eq;
using ::testing::InSequence;
using ::testing::Return;
using ::testing::SetArgReferee;
using ::testing::StrictMock;

class US_BufferComponentTest : public QtTestBase
{
   protected:
      void SetUp() override
      {
         ASSERT_NO_FATAL_FAILURE( QtTestBase::SetUp() );
         setupValidComponent();
      }

      void setupValidComponent()
      {
         component.componentID = "1";
         component.name        = "NaCl";
         component.unit        = "M";
         component.range       = "0.0 - 1.0";
         component.grad_form   = false;

         // Initialize coefficients
         for ( int i = 0; i < 6; i++ )
         {
            component.dens_coeff[i] = 1.0 + i * 0.1;
            component.visc_coeff[i] = 2.0 + i * 0.1;
         }
      }

      US_BufferComponent component;
};

class US_BufferTest : public QtTestBase
{
   protected:
      void SetUp() override
      {
         ASSERT_NO_FATAL_FAILURE( QtTestBase::SetUp() );
         setupValidBuffer();
      }

      void setupValidBuffer()
      {
         buffer.person                  = "Test User";
         buffer.bufferID                = "1";
         buffer.GUID                    = "test-guid-123";
         buffer.description             = "Test Buffer";
         buffer.compressibility         = 4.6e-10;
         buffer.pH                      = 7.4;
         buffer.density                 = DENS_20W;
         buffer.viscosity               = VISC_20W;
         buffer.manual                  = false;
         buffer.replace_spectrum        = false;
         buffer.new_or_changed_spectrum = false;

         // Add a test component
         US_BufferComponent comp;
         comp.componentID = "1";
         comp.name        = "NaCl";
         comp.unit        = "M";
         for ( int i = 0; i < 6; i++ )
         {
            comp.dens_coeff[i] = 1.0 + i * 0.1;
            comp.visc_coeff[i] = 2.0 + i * 0.1;
         }

         buffer.component.append( comp );
         buffer.componentIDs.append( "1" );
         buffer.concentration.append( 0.15 );

         // Add test spectrum data
         buffer.extinction[280.0]   = 1.5;
         buffer.extinction[250.0]   = 2.0;
         buffer.refraction[280.0]   = 0.1;
         buffer.fluorescence[350.0] = 0.5;
      }

      US_Buffer buffer;
};

// US_BufferComponent Tests

TEST_F(US_BufferComponentTest, DefaultConstruction) {
US_BufferComponent comp;

EXPECT_TRUE(comp.componentID.isEmpty());
EXPECT_TRUE(comp.name.isEmpty());
EXPECT_TRUE(comp.unit.isEmpty());
EXPECT_TRUE( comp.range.isEmpty() );
EXPECT_FALSE(comp.grad_form);

for ( int i = 0; i < 6; i++ )
{
   EXPECT_THAT( comp.dens_coeff[i], DoubleEq( 0.0 ) ) << "dens_coeff[" << i << "]";
   EXPECT_THAT( comp.visc_coeff[i], DoubleEq( 0.0 ) ) << "visc_coeff[" << i << "]";
}
}

TEST_F( US_BufferTest, SpectrumConsumesFiniteRowsWithExactQueryAndColumnContract )
{
   StrictMock<US_DB2_Mock> db;
   const QStringList       query{ "get_spectrum", "1", "Buffer", "Extinction" };
   {
      InSequence sequence;
      EXPECT_CALL( db, query( ::testing::Matcher<const QStringList&>( Eq( query ) ) ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( 260.0 ) ) );
      EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( 0.25 ) ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( 280.0 ) ) );
      EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( 0.75 ) ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
   }
   buffer.extinction.clear();
   buffer.getSpectrum( &db, "Extinction" );
   ASSERT_EQ( buffer.extinction.size(), 2 );
   EXPECT_DOUBLE_EQ( buffer.extinction.value( 260.0 ), 0.25 );
   EXPECT_DOUBLE_EQ( buffer.extinction.value( 280.0 ), 0.75 );
}

TEST_F( US_BufferTest, EmptySpectrumDoesNotReadNonexistentColumns )
{
   StrictMock<US_DB2_Mock> db;
   const QStringList       query{ "get_spectrum", "1", "Buffer", "Extinction" };
   InSequence              sequence;
   EXPECT_CALL( db, query( ::testing::Matcher<const QStringList&>( Eq( query ) ) ) );
   EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
   EXPECT_CALL( db, value( _ ) ).Times( 0 );
   buffer.extinction.clear();
   buffer.getSpectrum( &db, "Extinction" );
   EXPECT_TRUE( buffer.extinction.isEmpty() );
}

TEST_F( US_BufferTest, DiskLoadReplacesHeldSpectra )
{
   QTemporaryDir dir;
   ASSERT_TRUE( dir.isValid() );
   const QString file = dir.filePath( "B0000001.xml" );

   US_Buffer     source;
   source.GUID         = "disk-guid";
   source.description  = "From disk";
   source.extinction   = { { 260.0, 0.3 } };
   source.fluorescence = { { 400.0, 0.2 } };
   ASSERT_TRUE( source.writeToDisk( file ) );

   ASSERT_TRUE( buffer.readFromDisk( file ) );
   EXPECT_EQ( QString( "From disk" ), buffer.description );
   EXPECT_EQ( source.extinction, buffer.extinction );
   EXPECT_TRUE( buffer.refraction.isEmpty() );
   EXPECT_EQ( source.fluorescence, buffer.fluorescence );
}

TEST_F( US_BufferTest, MalformedOrIncompleteDiskFileFailsAndLeavesBufferUnchanged )
{
   QTemporaryDir dir;
   ASSERT_TRUE( dir.isValid() );
   const US_Buffer  before = buffer;

   const QByteArray bodies[] = {
      // Truncated inside the spectrum
      "<?xml version='1.0'?><BufferData version='1.0'>"
      "<buffer id='0' guid='partial-guid' description='partial' ph='5.0'>"
      "<component id='9' concentration='0.5'/><spectrum><extinction wavelength='300.0' value='1.0e+00'/>",
      // Well-formed, but no buffer element
      "<?xml version='1.0'?><BufferData version='1.0'/>",
   };

   for ( const QByteArray& body : bodies )
   {
      SCOPED_TRACE( body.constData() );
      const QString file = dir.filePath( "B0000001.xml" );
      QFile         out( file );
      ASSERT_TRUE( out.open( QIODevice::WriteOnly | QIODevice::Truncate ) );
      ASSERT_EQ( body.size(), out.write( body ) );
      out.close();

      EXPECT_FALSE( buffer.readFromDisk( file ) );
      EXPECT_EQ( before.bufferID, buffer.bufferID );
      EXPECT_EQ( before.GUID, buffer.GUID );
      EXPECT_EQ( before.description, buffer.description );
      EXPECT_DOUBLE_EQ( before.pH, buffer.pH );
      EXPECT_EQ( before.componentIDs, buffer.componentIDs );
      EXPECT_EQ( before.concentration, buffer.concentration );
      EXPECT_EQ( before.extinction, buffer.extinction );
      EXPECT_EQ( before.refraction, buffer.refraction );
      EXPECT_EQ( before.fluorescence, buffer.fluorescence );
   }
}
