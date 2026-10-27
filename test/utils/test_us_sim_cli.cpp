//! \file test_us_sim_cli.cpp
#include "qt_test_base.h"
#include "us_sim_cli.h"

#include <climits>

class US_SimCliTest : public QtTestBase
{
   protected:
      // Parse the options after a program name; return the exit status.
      int parse( US_SimCli::Tool tool, const QStringList& options )
      {
         return US_SimCli::parse( tool, QStringList{ "sim" } + options, args, output );
      }

      US_SimCliArgs args;
      QString       output;
};

TEST_F( US_SimCliTest, NoOptionsLeavesEverythingUnset )
{
   EXPECT_EQ( parse( US_SimCli::Astfem, {} ), -1 );

   EXPECT_FALSE( args.given );
   EXPECT_FALSE( args.start );
   EXPECT_TRUE( args.model.isEmpty() );
   EXPECT_EQ( args.centerpiece, -1 );
   EXPECT_EQ( args.centerpiece_row, -1 );
   EXPECT_EQ( args.cell, 0 );
   EXPECT_EQ( args.channel, 0 );
   EXPECT_EQ( args.noise_seed, 0u );
}

TEST_F( US_SimCliTest, NoDbAloneDoesNotCountAsGiven )
{
   // us_mwl_species_sim opens its GUI when nothing else was given.
   EXPECT_EQ( parse( US_SimCli::MwlSpecies, { "--no-db" } ), -1 );
   EXPECT_TRUE( args.no_db );
   EXPECT_FALSE( args.given );
}

TEST_F( US_SimCliTest, AstfemOptionsFillTheirFields )
{
   ASSERT_EQ( parse( US_SimCli::Astfem,
                     { "--model",
                       "m.xml",
                       "--buffer",
                       "b.xml",
                       "--simparams",
                       "sp.xml",
                       "--rotor",
                       "2",
                       "--centerpiece",
                       "3",
                       "--centerpiece-channel",
                       "C",
                       "--movie",
                       "--timecorr",
                       "--close",
                       "--errors-cl",
                       "--runtype",
                       "ip",
                       "--description",
                       "Run",
                       "--cell",
                       "2",
                       "--channel",
                       "c",
                       "--noise-seed",
                       "42",
                       "--guid-seed",
                       "seed",
                       "--edit-timestamp",
                       "2401010000" } ),
              -1 )
      << output.toStdString();

   EXPECT_TRUE( args.given );
   EXPECT_EQ( args.model, "m.xml" );
   EXPECT_EQ( args.buffer, "b.xml" );
   EXPECT_EQ( args.simparams, "sp.xml" );
   EXPECT_EQ( args.rotor, "2" );
   EXPECT_EQ( args.centerpiece, 3 );
   EXPECT_EQ( args.centerpiece_row, 1 ); // C and D share row 1
   EXPECT_TRUE( args.movie );
   EXPECT_TRUE( args.timecorr );
   EXPECT_TRUE( args.close );
   EXPECT_TRUE( args.errors_cl );
   EXPECT_EQ( args.run_type, "IP" );
   EXPECT_EQ( args.description, "Run" );
   EXPECT_EQ( args.cell, 2 );
   EXPECT_EQ( args.channel, 'C' );
   EXPECT_EQ( args.noise_seed, 42u );
   EXPECT_EQ( args.guid_seed, "seed" );
   EXPECT_EQ( args.edit_stamp, "2401010000" );
   EXPECT_FALSE( args.start );
}

TEST_F( US_SimCliTest, ModelsSplitIntoOnePathPerWavelength )
{
   ASSERT_EQ( parse( US_SimCli::MwlSpecies, { "--models", "a.xml,,b.xml" } ), -1 );
   EXPECT_EQ( args.models, QStringList( { "a.xml", "b.xml" } ) );
}

TEST_F( US_SimCliTest, SaveImpliesStartAndUsesForwardSlashes )
{
   ASSERT_EQ( parse( US_SimCli::Astfem, { "--save", "C:\\runs\\r1" } ), -1 );
   EXPECT_EQ( args.save, "C:/runs/r1" );
   EXPECT_TRUE( args.start );
}

TEST_F( US_SimCliTest, EmptyPathsAreLeftUnset )
{
   ASSERT_EQ( parse( US_SimCli::Astfem, { "--save=", "--model=" } ), -1 );
   EXPECT_TRUE( args.save.isEmpty() );
   EXPECT_TRUE( args.model.isEmpty() );
   EXPECT_FALSE( args.start );
}

TEST_F( US_SimCliTest, OptionsBelongToTheirSimulator )
{
   EXPECT_EQ( parse( US_SimCli::MwlSpecies, { "--cell", "1" } ), 1 );
   EXPECT_TRUE( output.contains( "cell" ) );
   EXPECT_EQ( parse( US_SimCli::Astfem, { "--models", "a.xml" } ), 1 );
   EXPECT_TRUE( output.contains( "models" ) );
}

TEST_F( US_SimCliTest, InvalidValuesAreRejected )
{
   const QList<QStringList> cases = {
      { "noise-seed", "0" },
      { "noise-seed", "abc" },
      { "noise-seed", QString::number( quint64( UINT_MAX ) + 1 ) },
      { "cell", "0" },
      { "cell", "9" },
      { "channel", "X" },
      { "channel", "AB" },
      { "runtype", "RI" },
      { "runtype", "WI" },
      { "edit-timestamp", "abc" },
      { "edit-timestamp", "2413320000" },
      { "description", "" },
      { "description", QString( 240, 'x' ) },
      { "description", QString::fromUtf8( "\xe2\x82\xac" ) },
      { "centerpiece", "abc" },
      { "centerpiece", "-1" },
      { "centerpiece-channel", "Z" },
      { "centerpiece-channel", "-2" },
   };

   for ( const QStringList& c : cases )
   {
      SCOPED_TRACE( ( c[0] + "=" + c[1] ).toStdString() );
      EXPECT_EQ( parse( US_SimCli::Astfem, { "--" + c[0] + "=" + c[1] } ), 1 );
      EXPECT_EQ( output, US_SimCli::invalidValueMessage( c[0], c[1] ) );
   }
}

TEST_F( US_SimCliTest, InvalidValueMessageSaysWhatIsExpected )
{
   EXPECT_EQ( US_SimCli::invalidValueMessage( "runtype", "RI" ),
              "Invalid --runtype RI; expected one of " + US_SimCli::runTypes().join( ", " ) );
   EXPECT_EQ( US_SimCli::invalidValueMessage( "cell", "9" ), "Invalid --cell 9; expected 1 to 8" );
}

TEST_F( US_SimCliTest, HelpListsOnlyThisSimulatorsOptions )
{
   EXPECT_EQ( parse( US_SimCli::MwlSpecies, { "--help" } ), 0 );
   EXPECT_TRUE( output.contains( "--models" ) );
   EXPECT_TRUE( output.contains( "--noise-seed" ) );
   EXPECT_FALSE( output.contains( "--movie" ) );

   EXPECT_EQ( parse( US_SimCli::Astfem, { "--help" } ), 0 );
   EXPECT_TRUE( output.contains( "--movie" ) );
   EXPECT_FALSE( output.contains( "--models" ) );
}

TEST_F( US_SimCliTest, VersionAndUnknownOptionsExitImmediately )
{
   EXPECT_EQ( parse( US_SimCli::Astfem, { "--version" } ), 0 );
   EXPECT_TRUE( output.contains( "Version" ) );

   EXPECT_EQ( parse( US_SimCli::Astfem, { "--bogus" } ), 1 );
   EXPECT_TRUE( output.contains( "bogus" ) );
}
