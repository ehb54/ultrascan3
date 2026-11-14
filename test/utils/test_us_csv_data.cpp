//! \file test_us_csv_data.cpp
#include "us3_file_test_base.h"
#include "us3_test_io.h"
#include "us_csv_data.h"

namespace
{
using CsvData = Us3FileTestBase;

TEST_F( CsvData, RectangularColumnsPreserveHeadersValuesAndPath )
{
   US_CSV_Data                    csv;
   const QString                  path = caseWorkRoot() + "/data.csv";
   const QStringList              header{ QString::fromUtf8( "wavelength \xce\xbb" ), "signal" };
   const QVector<QVector<double>> columns{ { 260, 280 }, { -1.25, 2.5 } };
   ASSERT_TRUE( csv.setData( path, header, columns ) );
   EXPECT_EQ( path, csv.filePath() );
   EXPECT_EQ( header, csv.header() );
   EXPECT_EQ( 2, csv.columnCount() );
   EXPECT_EQ( 2, csv.rowCount() );
   EXPECT_EQ( columns[0], csv.columnAt( 0 ) );
   EXPECT_EQ( columns[1], csv.columnAt( 1 ) );
   EXPECT_TRUE( csv.columnAt( -1 ).isEmpty() );
   EXPECT_TRUE( csv.columnAt( 2 ).isEmpty() );
   EXPECT_TRUE( csv.error().isEmpty() );
}

TEST_F( CsvData, RejectsMismatchedRaggedAndEmptyColumns )
{
   const QVector<QVector<QVector<double>>> invalid{ { { 1, 2 } }, { { 1, 2 }, { 3 } }, { {}, {} } };
   for ( const auto& columns : invalid )
   {
      US_CSV_Data csv;
      ASSERT_TRUE( csv.setData( { "old" }, { { 9 } } ) );
      EXPECT_FALSE( csv.setData( { "x", "y" }, columns ) );
      EXPECT_EQ( 0, csv.columnCount() );
      EXPECT_EQ( 0, csv.rowCount() );
      EXPECT_FALSE( csv.error().isEmpty() );
   }
}

TEST_F( CsvData, ReplacementAndClearDiscardPreviousDataAndErrors )
{
   US_CSV_Data csv;
   ASSERT_FALSE( csv.setData( "old.csv", { "x", "y" }, { { 1 } } ) );
   ASSERT_TRUE( csv.setData( { "new" }, { { 7, 8, 9 } } ) );
   EXPECT_TRUE( csv.error().isEmpty() );
   EXPECT_EQ( QString( "old.csv" ), csv.filePath() );
   EXPECT_EQ( ( QStringList{ "new" } ), csv.header() );
   EXPECT_EQ( ( QVector<double>{ 7, 8, 9 } ), csv.columnAt( 0 ) );
   csv.clear();
   EXPECT_EQ( 0, csv.rowCount() );
   EXPECT_EQ( 0, csv.columnCount() );
   EXPECT_TRUE( csv.filePath().isEmpty() );
   EXPECT_TRUE( csv.error().isEmpty() );
}

TEST_F( CsvData, ReadsExplicitDelimitersWithoutTransposingColumns )
{
   for ( const QByteArray& separator : { QByteArray( "," ), QByteArray( ";" ), QByteArray( "\t" ), QByteArray( " " ) } )
   {
      SCOPED_TRACE( separator.toStdString() );
      const QString path = caseWorkRoot() + "/input.csv";
      ASSERT_TRUE( us3WriteTestFile( path, "x" + separator + "y\n1" + separator + "-2.5\n3e2" + separator + "+4\n" ) );
      US_CSV_Data csv;
      ASSERT_TRUE( csv.readFile( path, QString::fromLatin1( separator ) ) );
      EXPECT_EQ( ( QStringList{ "x", "y" } ), csv.header() );
      EXPECT_EQ( ( QVector<double>{ 1, 300 } ), csv.columnAt( 0 ) );
      EXPECT_EQ( ( QVector<double>{ -2.5, 4 } ), csv.columnAt( 1 ) );
      EXPECT_EQ( path, csv.filePath() );
      EXPECT_EQ( 2, csv.rowCount() );
      EXPECT_TRUE( csv.error().isEmpty() );
   }
}

TEST_F( CsvData, NumericFirstLineBecomesDataWithGeneratedHeaders )
{
   const QString path = caseWorkRoot() + "/no-header.csv";
   ASSERT_TRUE( us3WriteTestFile( path, "260,1.25\n280,2.5\n" ) );
   US_CSV_Data csv;
   ASSERT_TRUE( csv.readFile( path, "," ) );
   EXPECT_EQ( ( QStringList{ "A", "B" } ), csv.header() );
   EXPECT_EQ( ( QVector<double>{ 260, 280 } ), csv.columnAt( 0 ) );
   EXPECT_EQ( ( QVector<double>{ 1.25, 2.5 } ), csv.columnAt( 1 ) );
}

TEST_F( CsvData, ReadsCrLfBlankLinesWhitespaceAndTrailingSeparator )
{
   const QString path = caseWorkRoot() + "/legacy.csv";
   ASSERT_TRUE( us3WriteTestFile( path, "\r\n x , y ,\r\n 1 , 2 ,\r\n \r\n 3 , 4 ,\r\n" ) );
   US_CSV_Data csv;
   ASSERT_TRUE( csv.readFile( path, "," ) );
   EXPECT_EQ( ( QStringList{ "x", "y" } ), csv.header() );
   EXPECT_EQ( ( QVector<double>{ 1, 3 } ), csv.columnAt( 0 ) );
   EXPECT_EQ( ( QVector<double>{ 2, 4 } ), csv.columnAt( 1 ) );
}

TEST_F( CsvData, AutoDetectsTabDelimitedInput )
{
   const QString path = caseWorkRoot() + "/tab.txt";
   ASSERT_TRUE( us3WriteTestFile( path, "radius\tsignal\n6.1\t0.25\n6.2\t0.5\n" ) );
   US_CSV_Data csv;
   ASSERT_TRUE( csv.readFile( path, QString() ) );
   EXPECT_EQ( ( QStringList{ "radius", "signal" } ), csv.header() );
   EXPECT_EQ( ( QVector<double>{ 6.1, 6.2 } ), csv.columnAt( 0 ) );
   EXPECT_EQ( ( QVector<double>{ 0.25, 0.5 } ), csv.columnAt( 1 ) );
}

TEST_F( CsvData, AutoDetectsCommaSemicolonAndSpaceDelimitedInput )
{
   const QList<QByteArray> inputs{ "x,y\n1,2\n3,4\n", "x;y\n1;2\n3;4\n", "x y\n1 2\n3 4\n" };
   for ( const QByteArray& bytes : inputs )
   {
      SCOPED_TRACE( bytes.toStdString() );
      const QString path = caseWorkRoot() + "/auto.txt";
      ASSERT_TRUE( us3WriteTestFile( path, bytes ) );
      US_CSV_Data csv;
      ASSERT_TRUE( csv.readFile( path, QString() ) ) << csv.error().toStdString();
      EXPECT_EQ( ( QStringList{ "x", "y" } ), csv.header() );
      EXPECT_EQ( ( QVector<double>{ 1, 3 } ), csv.columnAt( 0 ) );
      EXPECT_EQ( ( QVector<double>{ 2, 4 } ), csv.columnAt( 1 ) );
      EXPECT_TRUE( csv.error().isEmpty() );
   }
}

TEST_F( CsvData, AutoDetectionPrefersEarlierValidDelimiter )
{
   // Both tab and space give a numeric table; tab is tried first.
   const QString path = caseWorkRoot() + "/tab-space.txt";
   ASSERT_TRUE( us3WriteTestFile( path, "a b\tc\n1\t2\n3\t4\n" ) );
   US_CSV_Data csv;
   ASSERT_TRUE( csv.readFile( path, QString() ) );
   EXPECT_EQ( ( QStringList{ "a b", "c" } ), csv.header() );
   EXPECT_EQ( ( QVector<double>{ 1, 3 } ), csv.columnAt( 0 ) );
}

TEST_F( CsvData, FailedReadsReportConversionAndSplitErrors )
{
   const QString path = caseWorkRoot() + "/nonnumeric.txt";
   ASSERT_TRUE( us3WriteTestFile( path, "x,y\n1,nope\n" ) );
   US_CSV_Data csv;
   EXPECT_FALSE( csv.readFile( path, QString() ) );
   EXPECT_EQ( QString( "Cannot convert the line to the floating numbers. Error at the line: 2" ), csv.error() );
   EXPECT_EQ( 0, csv.columnCount() );

   ASSERT_TRUE( us3WriteTestFile( path, "x\ty\n1\t2\t3\n" ) );
   EXPECT_FALSE( csv.readFile( path, "\t" ) );
   EXPECT_EQ( QString( "Cannot split the lines with the given separator. Error at the line: 2" ), csv.error() );
}

TEST_F( CsvData, RejectsEmptySingleLineRaggedAndNonnumericFiles )
{
   const QList<QByteArray> inputs{ "", " \n\r\n", "x,y\n", "1,2\n", "x,y\n1,2\n3\n", "x,y\n1,nope\n", "x,y\n1,,\n" };
   for ( const QByteArray& bytes : inputs )
   {
      SCOPED_TRACE( bytes.toStdString() );
      const QString path = caseWorkRoot() + "/invalid.csv";
      ASSERT_TRUE( us3WriteTestFile( path, bytes ) );
      US_CSV_Data csv;
      ASSERT_TRUE( csv.setData( "old.csv", { "old" }, { { 9 } } ) );
      EXPECT_FALSE( csv.readFile( path, "," ) );
      EXPECT_FALSE( csv.error().isEmpty() );
      EXPECT_EQ( 0, csv.rowCount() );
      EXPECT_EQ( 0, csv.columnCount() );
      EXPECT_TRUE( csv.filePath().isEmpty() );
   }
}

TEST_F( CsvData, MissingFileClearsDataAndSuccessfulReadClearsError )
{
   US_CSV_Data csv;
   ASSERT_TRUE( csv.setData( "old.csv", { "old" }, { { 9 } } ) );
   EXPECT_FALSE( csv.readFile( caseWorkRoot() + "/missing.csv", "," ) );
   EXPECT_FALSE( csv.error().isEmpty() );
   EXPECT_EQ( 0, csv.rowCount() );
   const QString path = caseWorkRoot() + "/valid.csv";
   ASSERT_TRUE( us3WriteTestFile( path, "x\n3\n" ) );
   ASSERT_TRUE( csv.readFile( path, "," ) );
   EXPECT_TRUE( csv.error().isEmpty() );
   EXPECT_EQ( ( QVector<double>{ 3 } ), csv.columnAt( 0 ) );
   EXPECT_EQ( path, csv.filePath() );
}

TEST_F( CsvData, WriterEmitsExpectedRowsAndReplacesLongerFile )
{
   const QString path = caseWorkRoot() + "/output.csv";
   ASSERT_TRUE( us3WriteTestFile( path, QByteArray( 200, 'X' ) ) );
   US_CSV_Data csv;
   ASSERT_TRUE( csv.setData( path, { " x ", "y" }, { { 1, 3 }, { 2.5, -4 } } ) );
   ASSERT_TRUE( csv.writeFile( ";" ) );
   QFile output( path );
   ASSERT_TRUE( output.open( QIODevice::ReadOnly | QIODevice::Text ) );
   EXPECT_EQ( QByteArray( "x;y\n1;2.5\n3;-4\n" ), output.readAll() );
   EXPECT_EQ( QFileDevice::NoError, output.error() );
   EXPECT_EQ( ( QVector<double>{ 1, 3 } ), csv.columnAt( 0 ) );
}

TEST_F( CsvData, WriterRejectsMissingArgumentsDataAndUnopenablePath )
{
   US_CSV_Data csv;
   ASSERT_TRUE( csv.setData( { "x" }, { { 1 } } ) );
   EXPECT_FALSE( csv.writeFile( "," ) );
   EXPECT_FALSE( csv.error().isEmpty() );
   const QString path = caseWorkRoot() + "/sentinel.csv";
   ASSERT_TRUE( us3WriteTestFile( path, "unchanged" ) );
   csv.setFilePath( path );
   EXPECT_FALSE( csv.writeFile( QString() ) );
   QFile sentinel( path );
   ASSERT_TRUE( sentinel.open( QIODevice::ReadOnly ) );
   EXPECT_EQ( QByteArray( "unchanged" ), sentinel.readAll() );
   sentinel.close();
   csv.clear();
   csv.setFilePath( path );
   EXPECT_FALSE( csv.writeFile( "," ) );
   ASSERT_TRUE( sentinel.open( QIODevice::ReadOnly ) );
   EXPECT_EQ( QByteArray( "unchanged" ), sentinel.readAll() );
   ASSERT_TRUE( csv.setData( caseWorkRoot() + "/absent/output.csv", { "x" }, { { 1 } } ) );
   EXPECT_FALSE( csv.writeFile( "," ) );
   EXPECT_FALSE( csv.error().isEmpty() );
}
}
