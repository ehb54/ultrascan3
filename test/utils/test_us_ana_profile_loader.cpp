//! \file test_us_ana_profile_loader.cpp
//! The loader gives the profiles recorded from the analysis profile window.
#include "../programs/us_analysis_profile/profile_fixture.h"
#include "../programs/us_analysis_profile/profile_snapshot.h"
#include "../programs/us_analysis_profile/run_report_clicks.h"
#include "mock_us_db2.h"
#include "us3_file_test_base.h"
#include "us_ana_profile_loader.h"

namespace
{
using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Invoke;
using ::testing::Matcher;
using ::testing::NiceMock;
using ::testing::Return;

QString xml_of( US_AnaProfile profile )
{
   QString          xml;
   QXmlStreamWriter writer( &xml );
   writer.setAutoFormatting( true );
   profile.toXml( writer );
   return xml;
}

std::string text( const QJsonValue& value )
{
   return QJsonDocument( QJsonObject{ { "value", value } } ).toJson().toStdString();
}

QStringList texts( const US_AnaProfileLoader::Result& result )
{
   QStringList list;
   for ( const US_AnaProfileLoader::Message& message : result.messages )
   {
      list << message.text;
   }
   return list;
}

class AnaProfileLoaderTest : public Us3FileTestBase
{
   protected:
      //! Answers as the procedures in fixture_database.sql do
      void answer( const QStringList& args )
      {
         queries << args.join( " " );
         rows.clear();
         row    = -1;
         status = IUS_DB2::OK;
         if ( args[0] == failing )
         {
            status = IUS_DB2::DBERROR;
         }
         else if ( args[0] == "get_aprofile_info" )
         {
            if ( xml.isNull() )
            {
               status = IUS_DB2::NOROWS;
            }
            else
            {
               rows << ( QVariantList() << 29 << "Stored profile" << xml << "unused" << "{\"fixture\":\"mask\"}"
                                        << "{\"fixture\":\"plots\"}" );
            }
         }
         else if ( args[0] == "get_report_info_by_id" )
         {
            const int id = args[1].toInt();
            rows << ( QVariantList() << "1A:UV/vis.:Sample" << 1.25 << 0.004 << 15000 << 9000 << ( id == 17 ? 280 : id % 1000 )
                                     << 12 << 15
                                     << "{\"tot_conc\":\"0\",\"rmsd_lim\":\"1\",\"intensity\":\"0\",\"exp_dur\":\"1\","
                                        "\"integration\":\"0\",\"plots\":\"1\"}" );
         }
         else if ( args[0] == "get_report_items_ids_by_report_id" )
         {
            if ( args[1].toInt() > 1000 )
            {
               status = IUS_DB2::NOROWS;
            }
            else
            {
               rows << ( QVariantList() << 41 );
            }
         }
         else if ( args[0] == "get_report_item_info_by_id" )
         {
            rows << ( QVariantList() << "s" << "2DSA-MC" << 2.5 << 4.5 << 0.8 << 5 << 80 << 1 << 0 );
         }
         else
         {
            ADD_FAILURE() << "Unexpected query " << qPrintable( args.join( " " ) );
         }
      }

      void SetUp() override
      {
         ASSERT_NO_FATAL_FAILURE( Us3FileTestBase::SetUp() );
         ASSERT_TRUE( QDir().mkpath( US_Settings::dataDir() + "/projects" ) );
         location_ = US_Settings::default_data_location();
         US_Settings::set_default_data_location( 1 );
         ON_CALL( db, query( Matcher<const QStringList&>( _ ) ) )
            .WillByDefault( Invoke( [this]( const QStringList& args ) { answer( args ); } ) );
         ON_CALL( db, isConnected() ).WillByDefault( Return( true ) );
         ON_CALL( db, lastErrno() ).WillByDefault( Invoke( [this] { return status; } ) );
         ON_CALL( db, lastError() ).WillByDefault( Return( QString( "fixture error" ) ) );
         ON_CALL( db, numRows() ).WillByDefault( Invoke( [this] { return int( rows.size() ); } ) );
         ON_CALL( db, next() ).WillByDefault( Invoke( [this] { return ++row < rows.size(); } ) );
         ON_CALL( db, value( _ ) )
            .WillByDefault( Invoke(
               [this]( unsigned column )
               {
                  const bool ok = row >= 0 && row < rows.size() && int( column ) < rows[row].size();
                  return ok ? rows[row][int( column )] : QVariant();
               } ) );
         ON_CALL( off, isConnected() ).WillByDefault( Return( false ) );
         ON_CALL( off, lastError() )
            .WillByDefault( Return( QString( "US_DB2 error: could not connect\nUS_DB2 error: DB not configured" ) ) );
      }

      void TearDown() override
      {
         US_Settings::set_default_data_location( location_ );
         Us3FileTestBase::TearDown();
      }

      //! Loads a case of the records, with its progress and messages
      US_AnaProfileLoader::Result load_case( const QString& name )
      {
         ProfileFixture::Input        input = ProfileFixture::make( name );
         US_AnaProfileLoader::Options options;
         if ( input.fresh )
         {
            input.initial         = US_AnaProfileLoader::startProfile();
            input.initial.aprofID = -101;
            input.initial.protoID = -102;
            input.initial.nchan   = -103;
         }
         options.start    = &input.initial;
         options.abde     = input.abde;
         options.progress = [this]( int done, int total, const QString& detail )
         { progress.append( QJsonArray{ done, total, detail } ); };
         options.connect = [this] { return &off; };

         if ( input.storage == "database" && name != "database-no-row" )
         {
            xml = xml_of( input.saved );
         }
         if ( input.storage == "disk" )
         {
            QFile file( US_Settings::dataDir() + "/projects/A0000001.xml" );
            EXPECT_TRUE( file.open( QIODevice::WriteOnly ) );
            file.write( xml_of( input.saved ).toUtf8() );
         }

         US_Settings::set_default_data_location( input.storage == "database" ? 1 : 2 );
         IUS_DB2*                    used   = input.storage == "database" ? &db : nullptr;
         US_AnaProfileLoader::Result result = US_AnaProfileLoader::load( input.protocol, used, options );
         if ( name.endsWith( "-reload" ) )
         {
            // The window reads the saved profile once
            const QList<US_AnaProfileLoader::Message> messages = result.messages;
            const US_AnaProfile                       profile  = result.profile;
            options.start                                      = &profile;
            options.read_saved                                 = false;
            result                                             = US_AnaProfileLoader::load( input.protocol, used, options );
            result.messages                                    = messages + result.messages;
         }
         return result;
      }

      int                   location_ = 0;
      NiceMock<US_DB2_Mock> db;
      NiceMock<US_DB2_Mock> off;
      QString               xml;
      QString               failing;
      QStringList           queries;
      QList<QVariantList>   rows;
      int                   row    = -1;
      int                   status = IUS_DB2::OK;
      QJsonArray            progress;
};

class AnaProfileLoaderRecords : public AnaProfileLoaderTest, public ::testing::WithParamInterface<QString>
{
};

TEST_P( AnaProfileLoaderRecords, GivesTheRecordedProfile )
{
   const US_AnaProfileLoader::Result result = load_case( GetParam() );
   QFile                             file( QString( PROFILE_FIXTURE_DIR ) + "/" + GetParam() + ".json" );
   ASSERT_TRUE( file.open( QIODevice::ReadOnly ) );
   const QJsonObject recorded = QJsonDocument::fromJson( file.readAll() ).object();

   ASSERT_TRUE( result.loaded ) << qPrintable( result.errors.join( "\n" ) );
   EXPECT_EQ( text( recorded["profile"] ), text( ProfileSnapshot::value( result.profile ) ) );
   EXPECT_EQ( text( recorded["progress"] ), text( progress ) );
   EXPECT_EQ( text( recorded["messages"] ), text( QJsonArray::fromStringList( texts( result ) ) ) );
}

INSTANTIATE_TEST_SUITE_P( Window,
                          AnaProfileLoaderRecords,
                          ::testing::ValuesIn( ProfileFixture::cases() ),
                          []( const ::testing::TestParamInfo<QString>& info )
                          { return QString( info.param ).replace( "-", "_" ).toStdString(); } );

//! The clicks recorded from the window give the same values without it
TEST_F( AnaProfileLoaderTest, RunAndReportGiveTheRecordedValues )
{
   for ( const RunReportClicks& clicks : run_report_clicks() )
   {
      US_AnaProfileLoader::Channels values;
      for ( int ii = 0; ii < 3; ii++ )
      {
         values.run << 1;
         values.report << 1;
         values.ref << 0;
         values.use << 0;
         values.use_max << 1;
      }
      if ( clicks.abde )
      {
         values.setRun( 1, false, true );
      }
      for ( const QString& click : clicks.clicks )
      {
         const int channel = click.section( ":", 0, 0 ).toInt();
         if ( click.endsWith( "RunReport" ) )
         {
            values.setReport( channel, values.report[channel] == 0 );
         }
         else
         {
            values.setRun( channel, values.run[channel] == 0, clicks.abde );
         }
      }

      QStringList      parts;
      const QList<int> lists[] = { values.run, values.report, values.ref, values.use };
      const char*      names[] = { "run ", "report ", "ref ", "use " };
      for ( int ii = 0; ii < 4; ii++ )
      {
         QStringList numbers;
         for ( int number : lists[ii] )
         {
            numbers << QString::number( number );
         }
         parts << names[ii] + numbers.join( "," );
      }
      EXPECT_EQ( clicks.expected, parts.join( " " ) ) << qPrintable( clicks.clicks.join( ", " ) ) << " abde " << clicks.abde;
   }
}

TEST_F( AnaProfileLoaderTest, UsesWhatWasReadOfIncompleteXmlAndDoesNotShowTheError )
{
   ProfileFixture::Input input = ProfileFixture::make( "database" );
   xml                         = xml_of( input.saved );
   xml.chop( QString( "</analysis_profile>" ).size() + 2 );
   US_AnaProfileLoader::Options options;
   options.start                            = &input.initial;
   const US_AnaProfileLoader::Result result = US_AnaProfileLoader::load( input.protocol, &db, options );
   ASSERT_TRUE( result.loaded ) << qPrintable( result.errors.join( "\n" ) );
   EXPECT_EQ( QList<double>() << 1.23457, result.profile.lc_ratios );
   EXPECT_FALSE( result.errors.isEmpty() );
   EXPECT_TRUE( result.messages.isEmpty() );
}

//! Loads that read past a list in the window give a message and no profile
TEST_F( AnaProfileLoaderTest, UnusableProfilesGiveAMessage )
{
   US_Settings::set_default_data_location( 2 );
   // An optic that is not installed comes before a usable one
   ProfileFixture::Input hidden            = ProfileFixture::make( "one" );
   hidden.protocol.rpOptic.chopts[0].scan1 = "(not installed)";
   hidden.protocol.rpOptic.chopts[0].scan2 = "UV/visible";
   ProfileFixture::Input none              = ProfileFixture::make( "one" );
   none.protocol.rpOptic.chopts[0].scan1   = "(not installed)";
   ProfileFixture::Input shorter           = ProfileFixture::make( "several" );
   shorter.initial.report_run.clear();
   ProfileFixture::Input no_speed = ProfileFixture::make( "one" );
   no_speed.protocol.rpSpeed.ssteps.clear();
   // ABDE whose only B channel is interference
   ProfileFixture::Input abde = ProfileFixture::make( "interference-pair" );
   abde.abde                  = true;

   const QList<QPair<ProfileFixture::Input, QString>> cases = { { hidden, "no wavelengths" },
                                                                { none, "no channel to analyze" },
                                                                { shorter, "channels without values" },
                                                                { no_speed, "protocol has no speed" },
                                                                { abde, "use as a reference" } };
   for ( const QPair<ProfileFixture::Input, QString>& each : cases )
   {
      US_AnaProfileLoader::Options options;
      options.start                            = &each.first.initial;
      options.abde                             = each.first.abde;
      const US_AnaProfileLoader::Result result = US_AnaProfileLoader::load( each.first.protocol, nullptr, options );
      EXPECT_FALSE( result.loaded );
      ASSERT_EQ( 1, int( result.messages.size() ) ) << qPrintable( each.second );
      EXPECT_EQ( QString( "Analysis Profile" ), result.messages[0].title );
      EXPECT_TRUE( result.messages[0].text.contains( each.second ) ) << qPrintable( result.messages[0].text );
   }
}

TEST_F( AnaProfileLoaderTest, SavedChannelThatTheProtocolLacksGivesAMessage )
{
   ProfileFixture::Input input = ProfileFixture::make( "disk-several" );
   input.protocol.rpSolut.chsols.removeLast();
   input.protocol.rpOptic.chopts.removeLast();
   xml = xml_of( input.saved );
   US_AnaProfileLoader::Options options;
   options.start                            = &input.initial;
   const US_AnaProfileLoader::Result result = US_AnaProfileLoader::load( input.protocol, &db, options );
   EXPECT_FALSE( result.loaded );
   ASSERT_EQ( 1, int( result.messages.size() ) );
   EXPECT_TRUE( result.messages[0].text.contains( "3A:UV/vis.:Sample" ) ) << qPrintable( result.messages[0].text );
}

//! Titles say what each message is about, when there are two
TEST_F( AnaProfileLoaderTest, MessagesHaveTheirOwnTitles )
{
   ProfileFixture::Input input = ProfileFixture::make( "disk-several" );
   input.protocol.rpSolut.chsols.removeLast();
   input.protocol.rpOptic.chopts.removeLast();
   US_Settings::set_default_data_location( 2 );
   QFile file( US_Settings::dataDir() + "/projects/A0000001.xml" );
   ASSERT_TRUE( file.open( QIODevice::WriteOnly ) );
   file.write( xml_of( input.saved ).toUtf8() );
   file.close();
   US_AnaProfileLoader::Options options;
   options.start                            = &input.initial;
   const US_AnaProfileLoader::Result result = US_AnaProfileLoader::load( input.protocol, &off, options );
   ASSERT_EQ( 2, int( result.messages.size() ) );
   EXPECT_EQ( QString( "Connection Problem" ), result.messages[0].title );
   EXPECT_EQ( QString( "Analysis Profile" ), result.messages[1].title );
}

//! Removing interference leaves lists shorter than the channels. The window, without
//! assertions, gave the last value to the channels that lack one.
TEST_F( AnaProfileLoaderTest, ChannelsWithoutValuesHaveThoseOfTheLastOne )
{
   US_Settings::set_default_data_location( 2 );
   ProfileFixture::Input input            = ProfileFixture::make( "interference-pair" );
   input.protocol.rpOptic.chopts[0].scan1 = "UV/visible";
   input.protocol.rpOptic.chopts[1].scan1 = "UV/visible";
   input.protocol.rpOptic.chopts[1].scan2 = "Rayleigh Interference";
   US_AnaProfileLoader::Options options;
   options.start                     = &input.initial;
   US_AnaProfileLoader::Result first = US_AnaProfileLoader::load( input.protocol, nullptr, options );
   ASSERT_TRUE( first.loaded );
   ASSERT_EQ( 3, int( first.profile.pchans.size() ) );
   ASSERT_EQ( 2, int( first.profile.lc_ratios.size() ) );

   input.protocol.rpOptic.chopts[1].scan2.clear();
   options.start                            = &first.profile;
   options.read_saved                       = false;
   const US_AnaProfileLoader::Result second = US_AnaProfileLoader::load( input.protocol, nullptr, options );
   ASSERT_TRUE( second.loaded ) << qPrintable( second.errors.join( "\n" ) );
   EXPECT_EQ( QList<double>() << 1 << 1, second.profile.lc_ratios );
   EXPECT_EQ( QList<int>() << 1 << 1, second.profile.analysis_run );
   EXPECT_EQ( QList<int>() << 280 << 280, second.profile.wvl_edit );
}

TEST_F( AnaProfileLoaderTest, FailedQueriesAreErrorsThatAreNotShown )
{
   const QStringList queries_to_fail = {
      "get_aprofile_info", "get_report_info_by_id", "get_report_items_ids_by_report_id", "get_report_item_info_by_id"
   };
   for ( const QString& name : queries_to_fail )
   {
      ProfileFixture::Input input = ProfileFixture::make( "database" );
      xml                         = xml_of( input.saved );
      failing                     = name;
      US_AnaProfileLoader::Options options;
      options.start                            = &input.initial;
      const US_AnaProfileLoader::Result result = US_AnaProfileLoader::load( input.protocol, &db, options );
      EXPECT_TRUE( result.loaded ) << qPrintable( name + ": " + result.errors.join( "\n" ) );
      EXPECT_TRUE( result.errors.join( " " ).contains( name ) ) << qPrintable( name );
      EXPECT_TRUE( result.messages.isEmpty() ) << qPrintable( name );
   }
}

TEST_F( AnaProfileLoaderTest, UnconnectedDatabaseIsReportedWithTheProfileFromDisk )
{
   ProfileFixture::Input input = ProfileFixture::make( "disk" );
   QFile                 file( US_Settings::dataDir() + "/projects/A0000001.xml" );
   ASSERT_TRUE( file.open( QIODevice::WriteOnly ) );
   file.write( xml_of( input.saved ).toUtf8() );
   file.close();
   ON_CALL( off, lastErrno() ).WillByDefault( Return( int( IUS_DB2::NOT_CONNECTED ) ) );
   US_AnaProfileLoader::Options options;
   options.start                            = &input.initial;
   const US_AnaProfileLoader::Result result = US_AnaProfileLoader::load( input.protocol, &off, options );
   ASSERT_TRUE( result.loaded );
   EXPECT_EQ( -1, result.profile.aprofID );
   ASSERT_EQ( 1, int( result.messages.size() ) );
   EXPECT_TRUE( result.messages[0].text.contains( "DB not configured" ) );
}

//! Connections are opened only when something is read
TEST_F( AnaProfileLoaderTest, ConnectsOnlyWhenNeeded )
{
   int                          connects = 0;
   ProfileFixture::Input        input    = ProfileFixture::make( "database" );
   US_AnaProfileLoader::Options options;
   options.start   = &input.initial;
   options.connect = [&]
   {
      connects++;
      return &db;
   };
   // In the database: for the profile, and the same connection for its reports
   xml = xml_of( input.saved );
   EXPECT_TRUE( US_AnaProfileLoader::load( input.protocol, nullptr, options ).loaded );
   EXPECT_EQ( 1, connects );
   // The protocol has no channels
   const ProfileFixture::Input empty = ProfileFixture::make( "no-channels" );
   EXPECT_TRUE( US_AnaProfileLoader::load( empty.protocol, nullptr, options ).loaded );
   EXPECT_EQ( 1, connects );
   // The profile was read before
   options.read_saved = false;
   EXPECT_TRUE( US_AnaProfileLoader::load( input.protocol, nullptr, options ).loaded );
   EXPECT_EQ( 1, connects );
   // On disk: only for its reports
   options.read_saved = true;
   US_Settings::set_default_data_location( 2 );
   EXPECT_TRUE( US_AnaProfileLoader::load( input.protocol, nullptr, options ).loaded );
   EXPECT_EQ( 1, connects );
   QFile file( US_Settings::dataDir() + "/projects/A0000001.xml" );
   ASSERT_TRUE( file.open( QIODevice::WriteOnly ) );
   file.write( xml.toUtf8() );
   file.close();
   EXPECT_TRUE( US_AnaProfileLoader::load( input.protocol, nullptr, options ).loaded );
   EXPECT_EQ( 2, connects );
}
}
