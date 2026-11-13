//! \file test_us_analyte_xml.cpp
#include "qt_test_base.h"
#include "us3_test_io.h"
#include "us_analyte.h"
#include <QBuffer>
#include <QXmlStreamReader>
#include <cmath>
#include <cstring>
#include <limits>

namespace
{
class AnalyteXml : public QtTestBase, public ::testing::WithParamInterface<bool>
{
   protected:
      void read( US_Analyte& value, const QByteArray& body, const QString& guid = "fixture", int expected = IUS_DB2::OK )
      {
         if ( GetParam() )
         {
            QBuffer buffer;
            buffer.setData( body );
            ASSERT_TRUE( buffer.open( QIODevice::ReadOnly ) );
            ASSERT_EQ( expected, value.read_xml( buffer, guid ) );
            EXPECT_TRUE( buffer.isOpen() );
            return;
         }
         QString path;
         ASSERT_TRUE( US_Analyte::analyte_path( path ) );
         ASSERT_TRUE( us3WriteTestFile( path + "/A0000001.xml", body ) );
         ASSERT_EQ( expected, value.load( false, guid ) );
      }
      void write( US_Analyte& value, QByteArray& bytes )
      {
         if ( GetParam() )
         {
            QBuffer buffer( &bytes );
            ASSERT_TRUE( buffer.open( QIODevice::WriteOnly ) );
            ASSERT_EQ( IUS_DB2::OK, value.write_xml( buffer ) );
            EXPECT_TRUE( buffer.isOpen() );
            return;
         }
         QString dir;
         ASSERT_TRUE( US_Analyte::analyte_path( dir ) );
         const QString path = dir + "/output.xml";
         ASSERT_EQ( IUS_DB2::OK, value.write( false, path ) );
         QFile file( path );
         ASSERT_TRUE( file.open( QIODevice::ReadOnly ) );
         bytes = file.readAll();
         ASSERT_EQ( QFileDevice::NoError, file.error() );
      }
};
INSTANTIATE_TEST_SUITE_P( FilenameAndDevice, AnalyteXml, ::testing::Bool() );

TEST_P( AnalyteXml, ReadsAllTypesAndIgnoresUnknownFields )
{
   const char* names[] = { "PROTEIN", "DNA", "RNA", "CARBOHYDRATE" };
   for ( int i = 0; i < 4; ++i )
   {
      SCOPED_TRACE( names[i] );
      US_Analyte a;
      ASSERT_NO_FATAL_FAILURE( read( a,
                                     QByteArray( "<legacy><analyte analyteGUID='fixture' type='" ) + names[i] +
                                        "' description='A &amp; B' vbar20='0.72' mw='123' extra='ignored'/><unknown/></legacy>" ) );
      EXPECT_EQ( i, a.type );
      EXPECT_EQ( QString( "A & B" ), a.description );
      EXPECT_DOUBLE_EQ( 0.72, a.vbar20 );
      EXPECT_DOUBLE_EQ( 123, a.mw );
   }
}

TEST_P( AnalyteXml, MissingFieldsUnknownTypeAndExistingState )
{
   US_Analyte a;
   a.type            = US_Analyte::CARBOHYDRATE;
   a.sequence        = "old";
   a.extinction[250] = 12;
   ASSERT_NO_FATAL_FAILURE(
      read( a, "<analyte analyteGUID='fixture' type='future'><extinction frequency='280' value='8'/></analyte>" ) );
   EXPECT_EQ( US_Analyte::CARBOHYDRATE, a.type );
   EXPECT_TRUE( a.description.isEmpty() );
   EXPECT_DOUBLE_EQ( 0, a.vbar20 );
   EXPECT_DOUBLE_EQ( 0, a.mw );
   EXPECT_EQ( QString( "old" ), a.sequence );
   EXPECT_DOUBLE_EQ( 12, a.extinction.value( 250 ) );
   EXPECT_DOUBLE_EQ( 8, a.extinction.value( 280 ) );
}

TEST_P( AnalyteXml, ProteinSequenceAndExplicitExtinctionAreOrderDependent )
{
   for ( bool sequenceFirst : { true, false } )
   {
      US_Analyte       a;
      const QByteArray sequence = "<sequence>AAA</sequence>";
      const QByteArray spectrum = "<extinction frequency='280' value='99'/>";
      ASSERT_NO_FATAL_FAILURE( read( a,
                                     "<analyte analyteGUID='fixture' type='PROTEIN' mw='123'>" +
                                        ( sequenceFirst ? sequence + spectrum : spectrum + sequence ) + "</analyte>" ) );
      EXPECT_DOUBLE_EQ( 123, a.mw );
      EXPECT_DOUBLE_EQ( sequenceFirst ? 99 : 0, a.extinction.value( 280 ) );
   }
}

TEST_P( AnalyteXml, NucleotidesRecomputeMassAndNormalizeCase )
{
   for ( const char* type : { "DNA", "RNA" } )
   {
      US_Analyte a;
      ASSERT_NO_FATAL_FAILURE( read( a,
                                     QByteArray( "<analyte analyteGUID='fixture' type='" ) + type +
                                        "' mw='9999' stranded='0' complement_only='0' ThreePrimeHydroxyl='T' FivePrimeHydroxyl='1' "
                                        "sodium='1'><sequence>A</sequence></analyte>" ) );
      EXPECT_EQ( QString( "a" ), a.sequence );
      EXPECT_FALSE( a.doubleStranded );
      EXPECT_TRUE( a._3prime );
      EXPECT_TRUE( a._5prime );
      EXPECT_NEAR( 313.209 + ( a.type == US_Analyte::RNA ? 15.999 : 0 ) + 22.99 + 17.01 - 77.96, a.mw, 1e-9 );
   }
}

TEST_P( AnalyteXml, MalformedXmlFailsWithoutPublishingPartialState )
{
   US_Analyte a;
   a.description           = "retained";
   a.extinction[250]       = 10;
   const US_Analyte before = a;
   ASSERT_NO_FATAL_FAILURE(
      read( a,
            "<analyte analyteGUID='fixture' description='partial'><extinction frequency='280' value='5'/><broken>",
            "fixture",
            IUS_DB2::DBERROR ) );
   EXPECT_TRUE( a == before );
   EXPECT_EQ( before.analyteID, a.analyteID );
   EXPECT_FALSE( a.message.isEmpty() );
}

TEST_P( AnalyteXml, WriterProducesIndependentStructureValuesAndWrapping )
{
   for ( int type = 0; type < 4; ++type )
   {
      US_Analyte a;
      a.type               = static_cast<US_Analyte::analyte_t>( type );
      a.description        = "A & <B>";
      a.analyteGUID        = "fixture";
      a.vbar20             = 0.72;
      a.mw                 = 123;
      a.sequence           = QString( 81, 'A' );
      a.grad_form          = true;
      a.sodium             = 0.25;
      a.extinction[280.26] = 12.5;
      a.refraction[300]    = 1.5;
      a.fluorescence[400]  = 2.5;
      QByteArray bytes;
      ASSERT_NO_FATAL_FAILURE( write( a, bytes ) );
      QXmlStreamReader xml( bytes );
      QStringList      elements;
      while ( !xml.atEnd() )
      {
         xml.readNext();
         if ( !xml.isStartElement() )
         {
            continue;
         }
         const QString name = xml.name().toString();
         elements << name;
         const auto attrs = xml.attributes();
         if ( name == "AnalyteData" )
         {
            EXPECT_EQ( QString( "1.0" ), attrs.value( "version" ).toString() );
         }
         if ( name == "analyte" )
         {
            const char* types[] = { "PROTEIN", "DNA", "RNA", "CARBOHYDRATE" };
            EXPECT_EQ( QString( types[type] ), attrs.value( "type" ).toString() );
            EXPECT_EQ( a.description, attrs.value( "description" ).toString() );
            EXPECT_EQ( QString( "fixture" ), attrs.value( "analyteGUID" ).toString() );
            EXPECT_EQ( QString( "123" ), attrs.value( "mw" ).toString() );
            EXPECT_EQ( QString( "0.72" ), attrs.value( "vbar20" ).toString() );
            EXPECT_EQ( type == 1 || type == 2, attrs.hasAttribute( "stranded" ) );
            if ( type == 1 || type == 2 )
            {
               EXPECT_EQ( QString( "0.25" ), attrs.value( "sodium" ).toString() );
            }
            EXPECT_EQ( type == 3, attrs.hasAttribute( "gradient_forming" ) );
         }
         if ( name == "sequence" )
         {
            EXPECT_EQ( "\n" + QString( 80, 'A' ) + "\nA\n", xml.readElementText() );
         }
         if ( name == "extinction" )
         {
            EXPECT_DOUBLE_EQ( 280.26, attrs.value( "frequency" ).toDouble() );
            EXPECT_EQ( QString( "12.5" ), attrs.value( "value" ).toString() );
         }
         if ( name == "refraction" )
         {
            EXPECT_EQ( QString( "1.5" ), attrs.value( "value" ).toString() );
         }
         if ( name == "fluorescence" )
         {
            EXPECT_EQ( QString( "2.5" ), attrs.value( "value" ).toString() );
         }
      }
      EXPECT_FALSE( xml.hasError() );
      EXPECT_EQ( ( QStringList{ "AnalyteData", "analyte", "sequence", "extinction", "refraction", "fluorescence" } ), elements );
   }
}

TEST_P( AnalyteXml, WriterOmitsNonpositiveMass )
{
   for ( double mass : { 0.0, -1.0 } )
   {
      US_Analyte a;
      a.mw = mass;
      QByteArray bytes;
      ASSERT_NO_FATAL_FAILURE( write( a, bytes ) );
      QXmlStreamReader xml( bytes );
      bool             found = false;
      while ( !xml.atEnd() )
      {
         xml.readNext();
         if ( xml.isStartElement() && xml.name() == "analyte" )
         {
            found = true;
            EXPECT_FALSE( xml.attributes().hasAttribute( "mw" ) );
         }
      }
      EXPECT_TRUE( found );
      EXPECT_FALSE( xml.hasError() );
   }
}

TEST_P( AnalyteXml, ExternalCorpusEmptySequencesPreserveCurrentMassRules )
{
   struct Example
   {
         const char*           name;
         const char*           guid;
         US_Analyte::analyte_t type;
         double                mass;
         double                vbar;
   };
   const Example examples[] = { { "netrin1.xml", "a23098bf-78be-4f42-af2c-1b31049bf584", US_Analyte::PROTEIN, 49535, 0.7134 },
                                { "dna-208bp.xml", "f96065f9-2c61-404e-8edd-20ca21de6634", US_Analyte::DNA, 128396, 0.55 } };
   for ( const auto& example : examples )
   {
      SCOPED_TRACE( example.name );
      QFile fixture( QString( ":/test/analyte/" ) + example.name );
      ASSERT_TRUE( fixture.open( QIODevice::ReadOnly ) );
      const QByteArray bytes = fixture.readAll();
      ASSERT_EQ( QFileDevice::NoError, fixture.error() );
      US_Analyte a;
      ASSERT_NO_FATAL_FAILURE( read( a, bytes, example.guid ) );
      EXPECT_EQ( example.type, a.type );
      EXPECT_EQ( QString( example.guid ), a.analyteGUID );
      EXPECT_EQ( QString( "\n\n" ), a.sequence );
      EXPECT_DOUBLE_EQ( example.mass, a.mw );
      EXPECT_DOUBLE_EQ( example.vbar, a.vbar20 );
      ASSERT_TRUE( a.extinction.contains( 0 ) );
      EXPECT_DOUBLE_EQ( 0, a.extinction.value( 0 ) );
   }
}

TEST_P( AnalyteXml, SpectraMergeAndDuplicateFrequenciesUseLastValue )
{
   US_Analyte a;
   a.extinction[250]   = 5;
   a.refraction[250]   = 6;
   a.fluorescence[250] = 7;
   ASSERT_NO_FATAL_FAILURE( read( a,
                                  "<analyte analyteGUID='fixture' type='CARBOHYDRATE'>"
                                  "<extinction frequency='280.25' value='1'/><extinction frequency='280.25' value='2'/>"
                                  "<refraction frequency='300' value='3'/><refraction frequency='300' value='4'/>"
                                  "<fluorescence frequency='400' value='8'/><fluorescence frequency='400' value='9'/>"
                                  "<extinction/><refraction frequency='invalid' value='invalid'/><fluorescence/>"
                                  "</analyte>" ) );
   EXPECT_EQ( ( QMap<double, double>{ { 0, 0 }, { 250, 5 }, { 280.25, 2 } } ), a.extinction );
   EXPECT_EQ( ( QMap<double, double>{ { 0, 0 }, { 250, 6 }, { 300, 4 } } ), a.refraction );
   EXPECT_EQ( ( QMap<double, double>{ { 0, 0 }, { 250, 7 }, { 400, 9 } } ), a.fluorescence );
}

TEST_P( AnalyteXml, BooleanSpellingsRemainCaseSensitive )
{
   struct Flag
   {
         const char* text;
         bool        value;
   };
   for ( const auto& flag : { Flag{ "1", true },
                              Flag{ "T", true },
                              Flag{ "true", false },
                              Flag{ "t", false },
                              Flag{ "0", false },
                              Flag{ "", false } } )
   {
      SCOPED_TRACE( flag.text );
      US_Analyte       a;
      const QByteArray value( flag.text );
      ASSERT_NO_FATAL_FAILURE( read( a,
                                     "<analyte analyteGUID='fixture' type='DNA' gradient_forming='" + value + "' stranded='" +
                                        value + "' complement_only='" + value + "' ThreePrimeHydroxyl='" + value +
                                        "' FivePrimeHydroxyl='" + value + "'><sequence/></analyte>" ) );
      EXPECT_EQ( flag.value, a.grad_form );
      EXPECT_EQ( flag.value, a.doubleStranded );
      EXPECT_EQ( flag.value, a.complement );
      EXPECT_EQ( flag.value, a._3prime );
      EXPECT_EQ( flag.value, a._5prime );
   }
}

TEST_P( AnalyteXml, AbsentAndEmptySequencesHaveDifferentStateEffects )
{
   const char* types[] = { "PROTEIN", "DNA", "RNA", "CARBOHYDRATE" };
   for ( int type = 0; type < 4; ++type )
   {
      for ( bool present : { false, true } )
      {
         SCOPED_TRACE( types[type] );
         SCOPED_TRACE( present );
         US_Analyte a;
         a.sequence        = "retained";
         a.doubleStranded  = true;
         a.sodium          = 0.5;
         a.extinction[280] = 99;
         ASSERT_NO_FATAL_FAILURE( read( a,
                                        QByteArray( "<analyte analyteGUID='fixture' type='" ) + types[type] + "' mw='123'>" +
                                           ( present ? "<sequence/>" : "" ) + "</analyte>" ) );
         EXPECT_EQ( present ? QString() : QString( "retained" ), a.sequence );
         const bool nucleotide = type == 1 || type == 2;
         EXPECT_DOUBLE_EQ( 123, a.mw );
         EXPECT_DOUBLE_EQ( present && type == 0 ? 0 : 99, a.extinction.value( 280 ) );
         EXPECT_EQ( !( present && nucleotide ), a.doubleStranded );
         EXPECT_DOUBLE_EQ( present && nucleotide ? 0 : 0.5, a.sodium );
      }
   }
}

TEST_P( AnalyteXml, NamespacedFieldsCdataAndUnicodeRemainReadable )
{
   US_Analyte a;
   ASSERT_NO_FATAL_FAILURE(
      read( a,
            QByteArray( "<legacy xmlns:x='urn:legacy'><x:analyte analyteGUID='fixture' type='CARBOHYDRATE' "
                        "description='caf&#233; &amp; &#945;'><unknown><x:sequence><![CDATA[A<& B\n]]> C</x:sequence>"
                        "<x:refraction frequency='310.25' value='1.4'/></unknown><!-- retained tolerance -->"
                        "</x:analyte></legacy>" ) ) );
   EXPECT_EQ( QString::fromUtf8( "caf\xc3\xa9 & \xce\xb1" ), a.description );
   EXPECT_EQ( QString( "A<& B\n C" ), a.sequence );
   EXPECT_DOUBLE_EQ( 1.4, a.refraction.value( 310.25 ) );
}

TEST_P( AnalyteXml, LaterAnalyteElementsCannotOverwriteSelectedIdentityOrSpectra )
{
   US_Analyte a;
   ASSERT_NO_FATAL_FAILURE( read( a,
                                  "<AnalyteData><analyte analyteGUID='fixture' type='CARBOHYDRATE' description='first'>"
                                  "<extinction frequency='250' value='1'/></analyte>"
                                  "<analyte analyteGUID='second' type='RNA' description='second' mw='456'>"
                                  "<extinction frequency='280' value='2'/></analyte></AnalyteData>" ) );
   // Both entry points publish only the requested analyte.
   EXPECT_EQ( QString( "fixture" ), a.analyteGUID );
   EXPECT_EQ( QString( "first" ), a.description );
   EXPECT_EQ( US_Analyte::CARBOHYDRATE, a.type );
   EXPECT_DOUBLE_EQ( 0, a.mw );
   EXPECT_EQ( ( QMap<double, double>{ { 250, 1 } } ), a.extinction );
}

TEST_P( AnalyteXml, WriterSequenceLineBoundariesIncludeEmptyFinalLine )
{
   struct Example
   {
         int     length;
         QString expected;
   };
   const Example examples[] = { { 0, "\n\n" },
                                { 1, "\nA\n" },
                                { 79, "\n" + QString( 79, 'A' ) + "\n" },
                                { 80, "\n" + QString( 80, 'A' ) + "\n\n" },
                                { 81, "\n" + QString( 80, 'A' ) + "\nA\n" },
                                { 160, "\n" + QString( 80, 'A' ) + "\n" + QString( 80, 'A' ) + "\n\n" } };
   for ( const auto& example : examples )
   {
      SCOPED_TRACE( example.length );
      US_Analyte a;
      a.sequence = QString( example.length, 'A' );
      QByteArray bytes;
      ASSERT_NO_FATAL_FAILURE( write( a, bytes ) );
      QXmlStreamReader xml( bytes );
      int              sequences = 0;
      while ( !xml.atEnd() )
      {
         xml.readNext();
         if ( xml.isStartElement() && xml.name() == "sequence" )
         {
            ++sequences;
            EXPECT_EQ( example.expected, xml.readElementText() );
         }
      }
      EXPECT_FALSE( xml.hasError() );
      EXPECT_EQ( 1, sequences );
   }
}

TEST_P( AnalyteXml, WriterPreservesDistinctSpectrumFrequencies )
{
   US_Analyte   a;
   const double adjacent = std::nextafter( 280.21, std::numeric_limits<double>::infinity() );
   a.extinction          = { { 280.24, 2.5 }, { 280.21, 1.25 }, { adjacent, 3.75 } };
   a.refraction          = a.extinction;
   a.fluorescence        = a.extinction;
   QByteArray bytes;
   ASSERT_NO_FATAL_FAILURE( write( a, bytes ) );
   QXmlStreamReader             xml( bytes );
   QMap<QString, QList<double>> frequencies;
   QMap<QString, QStringList>   values;
   while ( !xml.atEnd() )
   {
      xml.readNext();
      const QString name = xml.name().toString();
      if ( xml.isStartElement() && ( name == "extinction" || name == "refraction" || name == "fluorescence" ) )
      {
         frequencies[name] << xml.attributes().value( "frequency" ).toDouble();
         values[name] << xml.attributes().value( "value" ).toString();
      }
   }
   EXPECT_FALSE( xml.hasError() );
   for ( const QString& name : { QString( "extinction" ), QString( "refraction" ), QString( "fluorescence" ) } )
   {
      EXPECT_EQ( ( QList<double>{ 280.21, adjacent, 280.24 } ), frequencies.value( name ) );
      EXPECT_EQ( ( QStringList{ "1.25", "3.75", "2.5" } ), values.value( name ) );
   }
   // Serialization does not collapse the in-memory keys.
   EXPECT_EQ( 3, a.extinction.size() );
}

TEST_P( AnalyteXml, WriterEscapesUnicodeAndOmitsMemoryOnlyFields )
{
   US_Analyte a;
   a.type        = US_Analyte::CARBOHYDRATE;
   a.description = QString::fromUtf8( "caf\xc3\xa9 & <\xce\xb1> \"quoted\"" );
   a.sequence    = "A<&B";
   a.message     = "PRIVATE-DIAGNOSTIC";
   a.analyteID   = "PRIVATE-DB-ID";
   QByteArray bytes;
   ASSERT_NO_FATAL_FAILURE( write( a, bytes ) );
   EXPECT_FALSE( bytes.contains( "PRIVATE-DIAGNOSTIC" ) );
   EXPECT_FALSE( bytes.contains( "PRIVATE-DB-ID" ) );
   QXmlStreamReader xml( bytes );
   int              analytes = 0, sequences = 0;
   while ( !xml.atEnd() )
   {
      xml.readNext();
      if ( !xml.isStartElement() )
      {
         continue;
      }
      if ( xml.name() == "analyte" )
      {
         ++analytes;
         EXPECT_EQ( a.description, xml.attributes().value( "description" ).toString() );
         EXPECT_FALSE( xml.attributes().hasAttribute( "analyteID" ) );
      }
      if ( xml.name() == "sequence" )
      {
         ++sequences;
         EXPECT_EQ( QString( "\nA<&B\n" ), xml.readElementText() );
      }
   }
   EXPECT_FALSE( xml.hasError() );
   EXPECT_EQ( 1, analytes );
   EXPECT_EQ( 1, sequences );
}

TEST_P( AnalyteXml, RequestedGuidCanFollowOtherAnalytes )
{
   US_Analyte a;
   ASSERT_NO_FATAL_FAILURE( read( a,
                                  "<legacy><analyte analyteGUID='other'><extinction frequency='300' value='9'/></analyte>"
                                  "<analyte analyteGUID='fixture' description='chosen' type='CARBOHYDRATE' mw='456'>"
                                  "<extinction frequency='250' value='2'/></analyte></legacy>" ) );
   EXPECT_EQ( QString( "fixture" ), a.analyteGUID );
   EXPECT_EQ( QString( "chosen" ), a.description );
   EXPECT_DOUBLE_EQ( 456, a.mw );
   EXPECT_EQ( ( QMap<double, double>{ { 250, 2 } } ), a.extinction );
}

TEST_P( AnalyteXml, NestedAndOutsideFieldsCannotContaminateSelectedAnalyte )
{
   US_Analyte a;
   ASSERT_NO_FATAL_FAILURE(
      read( a,
            "<legacy><extinction frequency='100' value='9'/>"
            "<analyte analyteGUID='fixture' description='chosen' type='CARBOHYDRATE'>"
            "<analyte analyteGUID='nested' description='wrong'><extinction frequency='200' value='9'/></analyte>"
            "<extinction frequency='250' value='2'/></analyte>"
            "<extinction frequency='300' value='9'/></legacy>" ) );
   EXPECT_EQ( QString( "fixture" ), a.analyteGUID );
   EXPECT_EQ( QString( "chosen" ), a.description );
   EXPECT_EQ( ( QMap<double, double>{ { 250, 2 } } ), a.extinction );
}

TEST_P( AnalyteXml, MissingGuidDoesNotPublishAnotherAnalyte )
{
   US_Analyte a;
   a.description           = "retained";
   a.analyteID             = "old-id";
   const US_Analyte before = a;
   ASSERT_NO_FATAL_FAILURE( read( a, "<analyte analyteGUID='other' description='wrong'/>", "fixture", IUS_DB2::NO_ANALYTE ) );
   EXPECT_TRUE( a == before );
   EXPECT_EQ( before.analyteID, a.analyteID );
   EXPECT_FALSE( a.message.isEmpty() );
}

TEST_P( AnalyteXml, TrailingMalformedXmlCannotPublishSelectedData )
{
   US_Analyte a;
   a.description           = "retained";
   const US_Analyte before = a;
   ASSERT_NO_FATAL_FAILURE(
      read( a, "<legacy><analyte analyteGUID='fixture' description='new'/><broken>", "fixture", IUS_DB2::DBERROR ) );
   EXPECT_TRUE( a == before );
   EXPECT_FALSE( a.message.isEmpty() );
}

TEST_P( AnalyteXml, EmptyNucleotideSequencePreservesExplicitMassAndDoesNotInventMass )
{
   for ( const char* type : { "DNA", "RNA" } )
   {
      for ( const QByteArray& mass : { QByteArray( " mw='12345'" ), QByteArray() } )
      {
         for ( const QByteArray& sequence : { QByteArray(), QByteArray( " \n\t " ) } )
         {
            US_Analyte a;
            ASSERT_NO_FATAL_FAILURE( read( a,
                                           QByteArray( "<analyte analyteGUID='fixture' type='" ) + type + "' stranded='1'" + mass +
                                              "><sequence>" + sequence + "</sequence></analyte>" ) );
            EXPECT_DOUBLE_EQ( mass.isEmpty() ? 0 : 12345, a.mw );
            EXPECT_EQ( QString::fromUtf8( sequence ), a.sequence );
            EXPECT_TRUE( a.doubleStranded );
         }
      }
   }
}

// A sequential transport that succeeds for a fixed prefix, then reports an
// actual I/O error. No sleeps, filesystem permissions or global hooks.
class FailingDevice : public QIODevice
{
   public:
      QByteArray prefix;
      QByteArray written;
      bool       failed     = false;
      qint64     writeLimit = 64;
      bool       isSequential() const override
      {
         return true;
      }

   protected:
      qint64 readData( char* data, qint64 size ) override
      {
         if ( prefix.isEmpty() )
         {
            failed = true;
            setErrorString( "injected read failure" );
            return -1;
         }
         const qint64 count = qMin( size, qint64( prefix.size() ) );
         memcpy( data, prefix.constData(), size_t( count ) );
         prefix.remove( 0, int( count ) );
         return count;
      }
      qint64 writeData( const char* data, qint64 size ) override
      {
         const qint64 count = qMin( size, qint64( writeLimit - written.size() ) );
         if ( count < size )
         {
            failed = true;
            setErrorString( "injected short write" );
         }
         if ( count == 0 )
         {
            return -1;
         }
         written.append( data, int( count ) );
         return count;
      }
};
using AnalyteXmlDevice = QtTestBase;

TEST_F( AnalyteXmlDevice, ClosedAndWrongModeDevicesFailWithoutChangingData )
{
   US_Analyte a;
   a.description = "retained";
   QBuffer buffer;
   EXPECT_EQ( IUS_DB2::DBERROR, a.read_xml( buffer ) );
   EXPECT_EQ( IUS_DB2::DBERROR, a.write_xml( buffer ) );
   ASSERT_TRUE( buffer.open( QIODevice::WriteOnly ) );
   EXPECT_EQ( IUS_DB2::DBERROR, a.read_xml( buffer ) );
   EXPECT_TRUE( buffer.isOpen() );
   buffer.close();
   ASSERT_TRUE( buffer.open( QIODevice::ReadOnly ) );
   EXPECT_EQ( IUS_DB2::DBERROR, a.write_xml( buffer ) );
   EXPECT_TRUE( buffer.isOpen() );
   EXPECT_EQ( QString( "retained" ), a.description );
   EXPECT_TRUE( buffer.data().isEmpty() );
}

TEST_F( AnalyteXmlDevice, ReadFailurePreservesExistingData )
{
   FailingDevice device;
   device.prefix = "<analyte description='partial' type='CARBOHYDRATE'>";
   ASSERT_TRUE( device.open( QIODevice::ReadOnly ) );
   US_Analyte       a;
   const US_Analyte before = a;
   EXPECT_EQ( IUS_DB2::DBERROR, a.read_xml( device ) );
   EXPECT_TRUE( a == before );
   EXPECT_FALSE( a.message.isEmpty() );
   EXPECT_TRUE( device.failed );
   EXPECT_TRUE( device.isOpen() );
}

TEST_F( AnalyteXmlDevice, WriteFailureReportsErrorWithoutClosingDevice )
{
   FailingDevice device;
   ASSERT_TRUE( device.open( QIODevice::WriteOnly ) );
   US_Analyte a;
   a.sequence = QString( 100, 'A' );
   EXPECT_EQ( IUS_DB2::DBERROR, a.write_xml( device ) );
   EXPECT_FALSE( a.message.isEmpty() );
   EXPECT_TRUE( device.failed );
   EXPECT_TRUE( device.isOpen() );
   EXPECT_FALSE( device.written.isEmpty() );
   EXPECT_FALSE( device.written.contains( "</AnalyteData>" ) );
}

TEST_F( AnalyteXmlDevice, ReadErrorAfterCompleteXmlStillFailsWithoutPublication )
{
   FailingDevice device;
   device.prefix = "<analyte description='new'/>";
   ASSERT_TRUE( device.open( QIODevice::ReadOnly ) );
   US_Analyte       a;
   const US_Analyte before = a;
   EXPECT_EQ( IUS_DB2::DBERROR, a.read_xml( device ) );
   EXPECT_TRUE( device.failed );
   EXPECT_TRUE( a == before );
   EXPECT_FALSE( a.message.isEmpty() );
}

TEST_F( AnalyteXmlDevice, ImmediateWriteErrorIsReported )
{
   FailingDevice device;
   device.writeLimit = 0;
   ASSERT_TRUE( device.open( QIODevice::WriteOnly ) );
   US_Analyte a;
   EXPECT_EQ( IUS_DB2::DBERROR, a.write_xml( device ) );
   EXPECT_TRUE( device.failed );
   EXPECT_TRUE( device.written.isEmpty() );
   EXPECT_TRUE( device.isOpen() );
   EXPECT_FALSE( a.message.isEmpty() );
}

TEST_F( AnalyteXmlDevice, DefaultSelectionUsesFirstAnalyteAndClearsOldError )
{
   QBuffer device;
   device.setData( "<legacy><analyte analyteGUID='first'/><analyte analyteGUID='second'/></legacy>" );
   ASSERT_TRUE( device.open( QIODevice::ReadOnly ) );
   US_Analyte a;
   a.message = "old error";
   ASSERT_EQ( IUS_DB2::OK, a.read_xml( device ) );
   EXPECT_EQ( QString( "first" ), a.analyteGUID );
   EXPECT_TRUE( a.message.isEmpty() );
   QBuffer output;
   ASSERT_TRUE( output.open( QIODevice::WriteOnly ) );
   a.message = "old error";
   ASSERT_EQ( IUS_DB2::OK, a.write_xml( output ) );
   EXPECT_TRUE( a.message.isEmpty() );
}

TEST_F( AnalyteXmlDevice, ReadsAndWritesAtCurrentPosition )
{
   QBuffer input;
   input.setData( "skip<analyte description='position'/>" );
   ASSERT_TRUE( input.open( QIODevice::ReadOnly ) );
   ASSERT_TRUE( input.seek( 4 ) );
   US_Analyte a;
   ASSERT_EQ( IUS_DB2::OK, a.read_xml( input ) );
   EXPECT_EQ( QString( "position" ), a.description );
   QBuffer output;
   output.setData( "keep" );
   ASSERT_TRUE( output.open( QIODevice::ReadWrite ) );
   ASSERT_TRUE( output.seek( 4 ) );
   ASSERT_EQ( IUS_DB2::OK, a.write_xml( output ) );
   EXPECT_TRUE( output.data().startsWith( "keep<?xml" ) );
   EXPECT_TRUE( input.isOpen() );
   EXPECT_TRUE( output.isOpen() );
}

}
