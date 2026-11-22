//! \file test_us_eprofile_xml.cpp
#include "qt_test_base.h"
#include "us_eprofile.h"
#include <QXmlStreamReader>
#include <cmath>
#include <limits>

namespace
{
using ExtProfileXml = QtTestBase;

TEST_F( ExtProfileXml, ReadsFixedProfileMetadataAndReplacesPreviousSpectrum )
{
   QString              xml  = "<EProfileData version='1.0'><extinctionProfile componentID='17' componentType='Buffer' "
                               "valueType='absorbance'><extinction wavelength='280' value='1.25'/>"
                               "<extinction wavelength='260' value='-0.5'/></extinctionProfile></EProfileData>";
   int                  id   = 99;
   QString              type = "old", valueType = "old";
   QMap<double, double> spectrum{ { 999, 99 } };
   ASSERT_TRUE( US_ExtProfile::parse_eprofile( xml, &id, &type, &valueType, spectrum ) );
   EXPECT_EQ( 17, id );
   EXPECT_EQ( QString( "Buffer" ), type );
   EXPECT_EQ( QString( "absorbance" ), valueType );
   EXPECT_EQ( ( QMap<double, double>{ { 260, -0.5 }, { 280, 1.25 } } ), spectrum );
}

TEST_F( ExtProfileXml, ReadsAnalyteFrequencyAliasWithoutChangingSuppliedMetadata )
{
   QString              xml  = "<AnalyteData><analyte type='PROTEIN'><extinction frequency='280.5' value='9000'/>"
                               "<extinction frequency='260' value='3000'/></analyte></AnalyteData>";
   int                  id   = 42;
   QString              type = "Analyte", valueType = "molarExtinction";
   QMap<double, double> spectrum;
   ASSERT_TRUE( US_ExtProfile::parse_eprofile( xml, &id, &type, &valueType, spectrum ) );
   EXPECT_EQ( 42, id );
   EXPECT_EQ( QString( "Analyte" ), type );
   EXPECT_EQ( QString( "molarExtinction" ), valueType );
   EXPECT_EQ( ( QMap<double, double>{ { 260, 3000 }, { 280.5, 9000 } } ), spectrum );
}

TEST_F( ExtProfileXml, FrequencyTakesPrecedenceAndMissingWavelengthDefaultsTo280 )
{
   QString              xml = "<extinctionProfile><extinction wavelength='260' frequency='290' value='1'/>"
                              "<extinction wavelength='270' frequency='' value='2'/>"
                              "<extinction value='3'/></extinctionProfile>";
   QMap<double, double> spectrum;
   ASSERT_TRUE( US_ExtProfile::parse_eprofile( xml, nullptr, nullptr, nullptr, spectrum ) );
   EXPECT_EQ( ( QMap<double, double>{ { 270, 2 }, { 280, 3 }, { 290, 1 } } ), spectrum );
}

TEST_F( ExtProfileXml, DuplicateWavelengthUsesLastValueAndSupportsScientificNumbers )
{
   QString              xml = "<extinctionProfile><extinction wavelength='2.8e2' value='1'/>"
                              "<extinction frequency='280' value='-2.5e1'/>"
                              "<extinction wavelength='300' value='+4'/></extinctionProfile>";
   QMap<double, double> spectrum;
   ASSERT_TRUE( US_ExtProfile::parse_eprofile( xml, nullptr, nullptr, nullptr, spectrum ) );
   EXPECT_EQ( ( QMap<double, double>{ { 280, -25 }, { 300, 4 } } ), spectrum );
}

TEST_F( ExtProfileXml, EmptyProfileIsRecognizedAndClearsSpectrum )
{
   QString              xml = "<extinctionProfile componentID='4' componentType='Sample' valueType='massExtinction'/>";
   int                  id  = 0;
   QString              type, valueType;
   QMap<double, double> spectrum{ { 280, 1 } };
   ASSERT_TRUE( US_ExtProfile::parse_eprofile( xml, &id, &type, &valueType, spectrum ) );
   EXPECT_TRUE( spectrum.isEmpty() );
   EXPECT_EQ( 4, id );
   EXPECT_EQ( QString( "Sample" ), type );
   EXPECT_EQ( QString( "massExtinction" ), valueType );
}

TEST_F( ExtProfileXml, OptionalMetadataPointersCanBeOmittedIndependently )
{
   for ( int mask = 0; mask < 8; ++mask )
   {
      SCOPED_TRACE( mask );
      QString              xml  = "<extinctionProfile componentID='8' componentType='Buffer' valueType='absorbance'>"
                                  "<extinction wavelength='250' value='0.75'/></extinctionProfile>";
      int                  id   = -1;
      QString              type = "retained", valueType = "retained";
      QMap<double, double> spectrum;
      ASSERT_TRUE( US_ExtProfile::parse_eprofile(
         xml, mask & 1 ? &id : nullptr, mask & 2 ? &type : nullptr, mask & 4 ? &valueType : nullptr, spectrum ) );
      EXPECT_EQ( mask & 1 ? 8 : -1, id );
      EXPECT_EQ( mask & 2 ? QString( "Buffer" ) : QString( "retained" ), type );
      EXPECT_EQ( mask & 4 ? QString( "absorbance" ) : QString( "retained" ), valueType );
      EXPECT_EQ( ( QMap<double, double>{ { 250, 0.75 } } ), spectrum );
   }
}

TEST_F( ExtProfileXml, AcceptsUnknownWrappersNamespacesAndEscapedMetadata )
{
   QString              xml = "<legacy xmlns:p='urn:legacy'><unknown/><p:extinctionProfile componentID='9' "
                              "componentType='A &amp; B' valueType='&#955;'><extra><p:extinction wavelength='260' value='2'/>"
                              "</extra></p:extinctionProfile></legacy>";
   int                  id  = 0;
   QString              type, valueType;
   QMap<double, double> spectrum;
   ASSERT_TRUE( US_ExtProfile::parse_eprofile( xml, &id, &type, &valueType, spectrum ) );
   EXPECT_EQ( 9, id );
   EXPECT_EQ( QString( "A & B" ), type );
   EXPECT_EQ( QString::fromUtf8( "\xce\xbb" ), valueType );
   EXPECT_EQ( ( QMap<double, double>{ { 260, 2 } } ), spectrum );
}

TEST_F( ExtProfileXml, UnrecognizedWellFormedDocumentsAreNotProfiles )
{
   for ( QString xml : { QString( "<unknown/>" ), QString( "<root><extinction wavelength='280' value='1'/></root>" ) } )
   {
      SCOPED_TRACE( xml.toStdString() );
      QMap<double, double> spectrum;
      EXPECT_FALSE( US_ExtProfile::parse_eprofile( xml, nullptr, nullptr, nullptr, spectrum ) );
   }
}

TEST_F( ExtProfileXml, WriterProducesMetadataAndSortedValuesWithoutMutatingInputs )
{
   QString              type = QString::fromUtf8( "Buffer & \xce\xbb" ), valueType = "mass<Extinction>";
   const QString        originalType = type, originalValueType = valueType;
   QMap<double, double> spectrum{ { 300, -2.5 }, { 260, 0.125 }, { 280, 9000 } };
   const auto           original = spectrum;
   QString              output;
   ASSERT_TRUE( US_ExtProfile::xml_eprofile( 27, type, valueType, spectrum, output ) );
   EXPECT_EQ( original, spectrum );
   EXPECT_EQ( originalType, type );
   EXPECT_EQ( originalValueType, valueType );
   QXmlStreamReader xml( output );
   QStringList      elements;
   QList<double>    wavelengths, values;
   bool             dtd = false;
   while ( !xml.atEnd() )
   {
      xml.readNext();
      if ( xml.isDTD() )
      {
         dtd = true;
         EXPECT_EQ( QString( "US_EProfile" ), xml.dtdName().toString() );
      }
      if ( !xml.isStartElement() )
      {
         continue;
      }
      const QString name = xml.name().toString();
      elements << name;
      const auto attrs = xml.attributes();
      if ( name == "EProfileData" )
      {
         EXPECT_EQ( QString( "1.0" ), attrs.value( "version" ).toString() );
      }
      if ( name == "extinctionProfile" )
      {
         EXPECT_EQ( QString( "27" ), attrs.value( "componentID" ).toString() );
         EXPECT_EQ( originalType, attrs.value( "componentType" ).toString() );
         EXPECT_EQ( originalValueType, attrs.value( "valueType" ).toString() );
      }
      if ( name == "extinction" )
      {
         wavelengths << attrs.value( "wavelength" ).toDouble();
         values << attrs.value( "value" ).toDouble();
         EXPECT_FALSE( attrs.hasAttribute( "frequency" ) );
      }
   }
   EXPECT_FALSE( xml.hasError() );
   EXPECT_TRUE( dtd );
   EXPECT_EQ( ( QStringList{ "EProfileData", "extinctionProfile", "extinction", "extinction", "extinction" } ), elements );
   EXPECT_EQ( ( QList<double>{ 260, 280, 300 } ), wavelengths );
   EXPECT_EQ( ( QList<double>{ 0.125, 9000, -2.5 } ), values );
}

TEST_F( ExtProfileXml, WriterProducesARecognizableEmptyProfileEnvelope )
{
   QString              type = "Analyte", valueType = "molarExtinction", output;
   QMap<double, double> spectrum;
   ASSERT_TRUE( US_ExtProfile::xml_eprofile( 0, type, valueType, spectrum, output ) );
   QXmlStreamReader xml( output );
   QStringList      elements;
   while ( !xml.atEnd() )
   {
      xml.readNext();
      if ( xml.isStartElement() )
      {
         elements << xml.name().toString();
      }
   }
   EXPECT_FALSE( xml.hasError() );
   EXPECT_EQ( ( QStringList{ "EProfileData", "extinctionProfile" } ), elements );
}

TEST_F( ExtProfileXml, MalformedProfilePublishesNothing )
{
   for ( QString xml : { QString( "<extinctionProfile componentID='17'><extinction wavelength='280' value='1'/>" ),
                         QString( "<analyte><extinction value='2'/></wrong>" ),
                         QString( "<extinctionProfile/><broken" ),
                         QString( "<unknown/>" ),
                         QString(),
                         QString( "<root><extinction value='2'/></root>" ),
                         QString( "<root><extinctionProfile/></root>trailing" ) } )
   {
      SCOPED_TRACE( xml.toStdString() );
      int                  id   = 99;
      QString              type = "old", valueType = "old";
      QMap<double, double> spectrum{ { 999, 99 } };
      EXPECT_FALSE( US_ExtProfile::parse_eprofile( xml, &id, &type, &valueType, spectrum ) );
      EXPECT_EQ( 99, id );
      EXPECT_EQ( QString( "old" ), type );
      EXPECT_EQ( QString( "old" ), valueType );
      EXPECT_EQ( ( QMap<double, double>{ { 999, 99 } } ), spectrum );
   }
}

TEST_F( ExtProfileXml, ProfileWriterReplacesReusedOutput )
{
   QString              type = "Buffer", valueType = "molarExtinction", output = "previous";
   QMap<double, double> spectrum{ { 280, 1 }, { 300, 2 } };
   for ( int i = 0; i < 2; ++i )
   {
      ASSERT_TRUE( US_ExtProfile::xml_eprofile( 17, type, valueType, spectrum, output ) );
      QMap<double, double> actual;
      ASSERT_TRUE( US_ExtProfile::parse_eprofile( output, nullptr, nullptr, nullptr, actual ) );
      EXPECT_EQ( spectrum, actual );
      spectrum.clear();
   }
}

TEST_F( ExtProfileXml, ProfileWriterPreservesAdjacentWavelengthsAndValuePrecision )
{
   QString              type = "Buffer", valueType = "molarExtinction", output;
   const double         adjacent = std::nextafter( 280.0, 281.0 );
   QMap<double, double> spectrum{ { 280.0, 1.2345678901234567 },
                                  { adjacent, 9.876543210987654 },
                                  { 280.00001, -1.234567890123456e-100 },
                                  { 280.00002, std::numeric_limits<double>::max() },
                                  { 300, std::numeric_limits<double>::min() } };
   ASSERT_TRUE( US_ExtProfile::xml_eprofile( 17, type, valueType, spectrum, output ) );
   // Inspect the writer independently of parse_eprofile, so reader behavior
   // cannot hide a collapsed wavelength or rounded value.
   QXmlStreamReader     xml( output );
   QMap<double, double> written;
   int                  points = 0;
   while ( !xml.atEnd() )
   {
      xml.readNext();
      if ( xml.isStartElement() && xml.name() == QStringLiteral( "extinction" ) )
      {
         const auto   attrs = xml.attributes();
         bool         keyOk = false, valueOk = false;
         const double key   = attrs.value( "wavelength" ).toDouble( &keyOk );
         const double value = attrs.value( "value" ).toDouble( &valueOk );
         EXPECT_TRUE( keyOk );
         EXPECT_TRUE( valueOk );
         written[key] = value;
         ++points;
      }
   }
   EXPECT_FALSE( xml.hasError() );
   EXPECT_EQ( spectrum.size(), points );
   EXPECT_EQ( spectrum, written );
   QMap<double, double> actual;
   ASSERT_TRUE( US_ExtProfile::parse_eprofile( output, nullptr, nullptr, nullptr, actual ) );
   EXPECT_EQ( spectrum, actual );
}

TEST_F( ExtProfileXml, ProfileWriterUsesShortestLocaleIndependentRoundTripNumbers )
{
   const QLocale previous = QLocale();
   QLocale::setDefault( QLocale( QLocale::German ) );
   QString              type = "Buffer", valueType = "molarExtinction", output;
   QMap<double, double> spectrum{ { 280.1, 0.1 } };
   EXPECT_TRUE( US_ExtProfile::xml_eprofile( 17, type, valueType, spectrum, output ) );
   QLocale::setDefault( previous );
   EXPECT_TRUE( output.contains( "wavelength=\"280.1\"" ) );
   EXPECT_TRUE( output.contains( "value=\"0.1\"" ) );
   QMap<double, double> actual;
   ASSERT_TRUE( US_ExtProfile::parse_eprofile( output, nullptr, nullptr, nullptr, actual ) );
   EXPECT_EQ( spectrum, actual );
}
}
