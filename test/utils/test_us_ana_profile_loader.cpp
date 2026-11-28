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
      QString protocol_xml;

      void    answer( const QStringList& args )
      {
         queries << args.join( " " );
         rows.clear();
         row    = -1;
         status = IUS_DB2::OK;
         if ( args[0] == failing )
         {
            status = IUS_DB2::DBERROR;
         }
         else if ( args[0] == "get_protocol_desc" )
         {
            if ( !protocol_xml.isEmpty() )
            {
               rows << ( QVariantList() << 54 << "test-guid" << "Fixture protocol" << protocol_xml << "unused" << QDateTime() );
            }
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

TEST_F( AnaProfileLoaderTest, FailedProtocolReadsLeaveThePriorProtocolIntact )
{
   US_RunProtocol protocol;
   protocol.protoname = "Previous";
   const QString name = "Fixture protocol";
   EXPECT_FALSE( US_AnaProfileLoader::readProtocol( name, 1, db, protocol ) );
   EXPECT_EQ( protocol.protoname, QString( "Previous" ) );

   protocol_xml = "<ProtocolData><protocol description='Fixture protocol' guid='new-guid'>";
   EXPECT_FALSE( US_AnaProfileLoader::readProtocol( name, 1, db, protocol ) );
   EXPECT_EQ( protocol.protoname, QString( "Previous" ) );

   protocol_xml += "</protocol></ProtocolData>";
   EXPECT_TRUE( US_AnaProfileLoader::readProtocol( name, 1, db, protocol ) );
   EXPECT_EQ( protocol.protoname, name );
   EXPECT_EQ( protocol.protoGUID, QString( "new-guid" ) );
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
   const QList<QPair<ProfileFixture::Input, QString>> cases = { { hidden, "no wavelengths" },
                                                                { none, "no channel to analyze" },
                                                                { shorter, "channels without values" },
                                                                { no_speed, "protocol has no speed" } };
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

//! Channels of a protocol, as in "1A:UI 1B:U": U is UV/vis., I is interference
US_RunProtocol protocol_of( const QString& text, const QString& solution = "Sample" )
{
   US_RunProtocol protocol;
   protocol.protoname         = "Channels";
   protocol.protoGUID         = "13290000-0000-0000-0000-000000000002";
   protocol.rpAprof.aprofname = protocol.protoname;
   protocol.rpSolut.chsols.clear();
   protocol.rpOptic.chopts.clear();
   protocol.rpRange.chrngs.clear();
   for ( const QString& each : text.split( " " ) )
   {
      const QString                               name  = each.left( 1 ) + " / " + each.mid( 1, 1 );
      const QString                               scans = each.section( ":", 1, 1 );
      US_RunProtocol::RunProtoSolutions::ChanSolu solute;
      solute.channel  = name;
      solute.solution = solution;
      protocol.rpSolut.chsols << solute;
      US_RunProtocol::RunProtoOptics::OpticSys optics;
      optics.channel = name;
      optics.scan1   = scans.startsWith( "U" ) ? "UV/visible" : "Rayleigh Interference";
      optics.scan2   = scans == "UI" ? "Rayleigh Interference" : "";
      optics.scan3   = "";
      protocol.rpOptic.chopts << optics;
      US_RunProtocol::RunProtoRanges::Ranges range;
      range.channel = name;
      range.wvlens  = { 260, 270, 280, 290, 300 };
      protocol.rpRange.chrngs << range;
   }
   return protocol;
}

QString text_of( const QList<int>& list )
{
   QStringList parts;
   for ( int value : list )
   {
      parts << QString::number( value );
   }
   return parts.join( "," );
}

//! A profile that was loaded for a protocol and edited, loaded for another one
class ChannelChanges : public AnaProfileLoaderTest
{
   protected:
      void SetUp() override
      {
         AnaProfileLoaderTest::SetUp();
         US_Settings::set_default_data_location( 2 );
      }

      //! Load, and give each shown channel its own values
      void first( const QString& channels )
      {
         const US_AnaProfileLoader::Result result = US_AnaProfileLoader::load( protocol_of( channels ), nullptr );
         ASSERT_TRUE( result.loaded );
         profile         = result.profile;
         const int count = profile.l_volumes.size();
         profile.scan_excl_begin.clear();
         profile.scan_excl_end.clear();
         profile.scan_excl_nth.clear();
         for ( int ii = 0; ii < count; ii++ )
         {
            profile.lc_ratios[ii]  = 1 + ii;
            profile.l_volumes[ii]  = 400 + 10 * ii;
            profile.replicates[ii] = ii % 2;
            // The exclusions dialog has an entry for each shown channel
            profile.scan_excl_begin << 11 + ii;
            profile.scan_excl_end << 21 + ii;
            profile.scan_excl_nth << 1 + ii;
         }
      }

      void set_edit( const QList<int>& wavelengths )
      {
         profile.wvl_edit = wavelengths;
      }

      void set_boxes( const QList<int>& run, const QList<int>& report, const QList<int>& ref, const QList<int>& use )
      {
         profile.analysis_run     = run;
         profile.report_run       = report;
         profile.ref_channels     = ref;
         profile.ref_use_channels = use;
      }

      US_AnaProfileLoader::Result second( const QString& channels, bool abde = false, const QString& solution = "Sample" )
      {
         US_AnaProfileLoader::Options options;
         options.start      = &profile;
         options.abde       = abde;
         options.read_saved = false;
         return US_AnaProfileLoader::load( protocol_of( channels, solution ), nullptr, options );
      }

      //! Reference numbers, and those used
      QString references( const US_AnaProfileLoader::Result& result )
      {
         return "run " + text_of( result.profile.analysis_run ) + " report " + text_of( result.profile.report_run ) + " ref " +
                text_of( result.profile.ref_channels ) + " use " + text_of( result.profile.ref_use_channels );
      }

      US_AnaProfile profile;
};

TEST_F( ChannelChanges, InterferenceRemovedFromOneCell )
{
   first( "1A:UI 1B:UI 2A:U" );
   ASSERT_EQ( 4, int( profile.l_volumes.size() ) );
   set_edit( { 260, 660, 270, 300 } );
   profile.wvl_not_run                      = QStringList() << "270" << "" << "280" << "290:300";
   profile.analysis_run                     = { 1, 1, 0, 1 };
   profile.report_run                       = { 1, 1, 0, 1 };
   const US_AnaProfileLoader::Result result = second( "1A:U 1B:U 2A:U" );
   ASSERT_TRUE( result.loaded );
   EXPECT_TRUE( result.messages.isEmpty() );
   EXPECT_EQ( QList<double>() << 400 << 420 << 430, result.profile.l_volumes );
   EXPECT_EQ( QList<double>() << 1 << 3 << 4, result.profile.lc_ratios );
   EXPECT_EQ( "1,0,1", text_of( result.profile.analysis_run ) );
   EXPECT_EQ( "1,0,1", text_of( result.profile.report_run ) );
   EXPECT_EQ( "0,0,1", text_of( result.profile.replicates ) );
   EXPECT_EQ( "260,270,300", text_of( result.profile.wvl_edit ) );
   EXPECT_EQ( QStringList() << "270" << "280" << "290:300", result.profile.wvl_not_run );
   EXPECT_EQ( "11,13,14", text_of( result.profile.scan_excl_begin ) );
   EXPECT_EQ( "21,23,24", text_of( result.profile.scan_excl_end ) );
   EXPECT_EQ( "1,3,4", text_of( result.profile.scan_excl_nth ) );
}

TEST_F( ChannelChanges, HiddenChannelRemoved )
{
   first( "1A:UI 1B:I 2A:U" );
   const US_AnaProfileLoader::Result result = second( "1A:U 2A:U" );
   ASSERT_TRUE( result.loaded );
   EXPECT_EQ( QList<double>() << 400 << 420, result.profile.l_volumes );
   EXPECT_EQ( "11,13", text_of( result.profile.scan_excl_begin ) );
}

TEST_F( ChannelChanges, MostChannelsRemoved )
{
   first( "1A:UI 1B:I 2A:UI 2B:I 3A:UI 3B:I" );
   const US_AnaProfileLoader::Result result = second( "1A:U" );
   ASSERT_TRUE( result.loaded ) << qPrintable( result.errors.join( "\n" ) );
   EXPECT_TRUE( result.messages.isEmpty() );
   EXPECT_EQ( QList<double>() << 400, result.profile.l_volumes );
   EXPECT_EQ( "11", text_of( result.profile.scan_excl_begin ) );
}

TEST_F( ChannelChanges, OrderChanged )
{
   first( "1A:U 1B:U 2A:U" );
   const US_AnaProfileLoader::Result result = second( "2A:U 1A:U 1B:U" );
   EXPECT_EQ( QList<double>() << 420 << 400 << 410, result.profile.l_volumes );
   EXPECT_EQ( "13,11,12", text_of( result.profile.scan_excl_begin ) );
}

TEST_F( ChannelChanges, SolutionChanged )
{
   first( "1A:U 1B:U 2A:U" );
   EXPECT_EQ( QList<double>() << 400 << 410 << 420, second( "1A:U 1B:U 2A:U", false, "Other" ).profile.l_volumes );
   EXPECT_EQ( QList<double>() << 400 << 420, second( "1A:U 2A:U", false, "Other" ).profile.l_volumes );
}

TEST_F( ChannelChanges, NewChannels )
{
   first( "1A:U 1B:U 2A:U" );
   // In the row of a channel that is gone
   EXPECT_EQ( QList<double>() << 400 << 410 << 420, second( "1A:U 3B:U 2A:U" ).profile.l_volumes );
   // Among the channels, and at the end
   EXPECT_EQ( QList<double>() << 400 << 400 << 410 << 420, second( "1A:U 3A:U 1B:U 2A:U" ).profile.l_volumes );
   EXPECT_EQ( QList<double>() << 400 << 410 << 420 << 420, second( "1A:U 1B:U 2A:U 3A:U" ).profile.l_volumes );
   // At the start there is no previous channel
   EXPECT_EQ( QList<double>() << 400 << 400 << 410 << 420, second( "4A:U 1A:U 1B:U 2A:U" ).profile.l_volumes );
}

TEST_F( ChannelChanges, SameChannelsKeepEveryListAsItIs )
{
   first( "1A:UI 1B:UI 2A:U" );
   const US_AnaProfileLoader::Result result = second( "1A:UI 1B:UI 2A:U" );
   EXPECT_EQ( QList<double>() << 400 << 410 << 420 << 430, result.profile.l_volumes );
   EXPECT_EQ( "11,12,13,14,14", text_of( result.profile.scan_excl_begin ) );
   // Saved profiles leave out the hidden channel
   profile.pchans.removeAt( 3 );
   profile.chndescs.removeAt( 3 );
   profile.chndescs_alt.removeLast();
   const US_AnaProfileLoader::Result saved = second( "1A:UI 1B:UI 2A:U" );
   ASSERT_TRUE( saved.loaded );
   EXPECT_EQ( QList<double>() << 400 << 410 << 420 << 430, saved.profile.l_volumes );
   EXPECT_EQ( "11,12,13,14,14", text_of( saved.profile.scan_excl_begin ) );
}

TEST_F( ChannelChanges, NoValuesStartWithTheDefaults )
{
   first( "1A:U 1B:U" );
   profile.lc_ratios.clear();
   profile.lc_tolers.clear();
   profile.l_volumes.clear();
   profile.lv_tolers.clear();
   profile.data_ends.clear();
   profile.ld_dens_0s.clear();
   profile.gm_vbars.clear();
   profile.gm_mws.clear();
   profile.replicates.clear();
   profile.wvl_edit.clear();
   profile.wvl_not_run.clear();
   set_boxes( {}, {}, {}, {} );
   for ( const QString& channels : QStringList{ "1A:U 1B:U", "1A:U", "2A:U 3A:U" } )
   {
      const US_AnaProfileLoader::Result result = second( channels );
      ASSERT_TRUE( result.loaded ) << qPrintable( channels );
      EXPECT_TRUE( result.messages.isEmpty() );
      EXPECT_EQ( 460, result.profile.l_volumes[0] );
      EXPECT_EQ( 1, result.profile.analysis_run[0] );
   }
   // Only some values are missing
   profile.lc_ratios = { 1, 2 };
   EXPECT_FALSE( second( "2A:U 3A:U" ).loaded );
}

TEST_F( ChannelChanges, ReferenceRemoved )
{
   for ( bool abde : { true, false } )
   {
      // The only one
      first( "1A:U 1B:U 2A:U" );
      set_boxes( { 1, 1, 1 }, { 1, 0, 1 }, { 0, 1, 0 }, { 1, 0, 1 } );
      EXPECT_EQ( "run 1,1 report 1,1 ref 0,0 use 0,0", references( second( "1A:U 2A:U", abde ) ) );

      first( "1A:U 1B:U 2A:U 2B:U" );
      set_boxes( { 1, 1, 1, 1 }, { 1, 0, 1, 0 }, { 0, 1, 0, 2 }, { 1, 0, 2, 0 } );
      // The first of two, and the second
      EXPECT_EQ( "run 1,1,1 report 1,1,0 ref 0,0,1 use 0,1,0", references( second( "1A:U 2A:U 2B:U", abde ) ) );
      EXPECT_EQ( "run 1,1,1 report 1,0,1 ref 0,1,0 use 1,0,0", references( second( "1A:U 1B:U 2A:U", abde ) ) );
      // Not a reference
      EXPECT_EQ( "run 1,1,1 report 0,1,0 ref 1,0,2 use 0,2,0", references( second( "1B:U 2A:U 2B:U", abde ) ) );
      // Its row goes to another channel
      EXPECT_EQ( "run 1,1,1,1 report 1,0,1,0 ref 0,1,0,2 use 1,0,2,0", references( second( "1A:U 3B:U 2A:U 2B:U", abde ) ) );
   }
}

TEST_F( ChannelChanges, RemovedReferenceLeavesTheFirstBChannelInAbde )
{
   first( "1A:U 1B:U 2A:U 2B:U" );
   set_boxes( { 1, 1, 1, 1 }, { 1, 0, 1, 1 }, { 0, 1, 0, 0 }, { 1, 0, 1, 1 } );
   EXPECT_EQ( "run 1,1,0 report 1,1,0 ref 0,0,1 use 1,1,0", references( second( "1A:U 2A:U 2B:U", true ) ) );
   EXPECT_EQ( "run 1,1,1 report 1,1,1 ref 0,0,0 use 0,0,0", references( second( "1A:U 2A:U 2B:U", false ) ) );
}

TEST_F( ChannelChanges, HiddenInterferenceIsNotAReference )
{
   // With every report on, only a shown B channel can be the reference
   for ( const QString& channels : QStringList{ "1A:UI 1B:I", "1A:U 1B:I 2A:U" } )
   {
      US_AnaProfileLoader::Options options;
      options.abde                             = true;
      const US_AnaProfileLoader::Result result = US_AnaProfileLoader::load( protocol_of( channels ), nullptr, options );
      ASSERT_TRUE( result.loaded ) << qPrintable( channels );
      EXPECT_TRUE( result.messages.isEmpty() );
      EXPECT_EQ( "run 1,1 report 1,1 ref 0,0 use 0,0", references( result ) );
   }
   first( "1A:U 1B:U 2A:U" );
   set_boxes( { 1, 1, 1 }, { 1, 0, 1 }, { 0, 1, 0 }, { 1, 0, 1 } );
   const US_AnaProfileLoader::Result result = second( "1A:U 1B:I 2A:U", true );
   ASSERT_TRUE( result.loaded );
   EXPECT_TRUE( result.messages.isEmpty() );
   EXPECT_EQ( "run 1,1 report 1,1 ref 0,0 use 0,0", references( result ) );
}

TEST_F( ChannelChanges, OnlyAHiddenChannelRemoved )
{
   first( "1A:U 1B:I 2A:U" );
   // As after another load
   profile.scan_excl_begin << 12;
   profile.scan_excl_end << 22;
   profile.scan_excl_nth << 2;
   const US_AnaProfileLoader::Result result = second( "1A:U 2A:U" );
   EXPECT_EQ( QList<double>() << 400 << 410, result.profile.l_volumes );
   EXPECT_EQ( "11,12", text_of( result.profile.scan_excl_begin ) );
}

TEST_F( ChannelChanges, CopyOfAReferenceIsNotOne )
{
   for ( bool abde : { true, false } )
   {
      first( "1A:U 1B:U 3A:U" );
      set_boxes( { 1, 1, 1 }, { 1, 0, 1 }, { 0, 1, 0 }, { 1, 0, 1 } );
      EXPECT_EQ( "run 1,1,1,1 report 1,0,1,1 ref 0,1,0,0 use 1,0,0,1", references( second( "1A:U 1B:U 2A:U 3A:U", abde ) ) );
      // With Run off, as ABDE makes references
      set_boxes( { 1, 0, 1 }, { 1, 0, 1 }, { 0, 1, 0 }, { 1, 0, 1 } );
      EXPECT_EQ( "run 1,0,1,1 report 1,0,1,1 ref 0,1,0,0 use 1,0,0,1", references( second( "1A:U 1B:U 2A:U 3A:U", abde ) ) );
      // At the end it is copied as before
      EXPECT_EQ( "run 1,0,1,1,1 report 1,0,1,1,1", references( second( "1A:U 1B:U 3A:U 4A:U 5A:U", abde ) ).left( 30 ) );
      first( "1A:U 1B:U" );
      set_boxes( { 1, 1 }, { 1, 0 }, { 0, 1 }, { 1, 0 } );
      EXPECT_EQ( "run 1,1,1 report 1,0,0 ref 0,1,1 use 1,0,0", references( second( "1A:U 1B:U 2A:U", abde ) ) );
   }
}
}
