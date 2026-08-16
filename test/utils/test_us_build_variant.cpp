// Prove that the test executable and the utilities library it links were
// compiled with the same database-related definitions.
//
// us_db2.h declares three private MYSQL members only when NO_DB is undefined, so
// a test translation unit compiled with NO_DB disagrees with a DB-enabled
// library about the layout of US_DB2 -- an ODR violation that silently corrupts
// every US_DB2 the tests touch.
//
// This translation unit's variant comes from the preprocessor; the library's is
// probed through US_Settings::us_debug(), which persists the level through
// QSettings only in the DB-enabled build.  Test settings are already sandboxed,
// so the probe writes nothing outside it.

#include "qt_test_base.h"
#include "us_defines.h"
#include "us_settings.h"

#include <QSettings>

namespace
{
// True when the linked utilities library routes the debug level through
// QSettings, i.e. when it was compiled with database support.
bool linkedUtilsLibraryPersistsDebugLevel()
{
    const int  restore = US_Settings::us_debug();
    const char key[]   = "us_debug";

    QSettings settings( US3, "UltraScan" );
    settings.remove( key );
    settings.sync();

    // A non-zero level; set_us_debug() removes the key instead of storing zero.
    US_Settings::set_us_debug( 7 );

    QSettings probe( US3, "UltraScan" );
    probe.sync();
    const bool persisted = probe.value( key, 0 ).toInt() == 7;

    US_Settings::set_us_debug( restore );
    return persisted;
}
}

TEST( BuildVariantConsistency, TestExecutableMatchesLinkedUtilsLibrary )
{
    // Sanity: the probe round-trips through the library either way.
    const bool libraryHasDatabaseSupport = linkedUtilsLibraryPersistsDebugLevel();
    US_Settings::set_us_debug( 3 );
    ASSERT_EQ( US_Settings::us_debug(), 3 );
    US_Settings::set_us_debug( 0 );

#ifdef NO_DB
    EXPECT_FALSE( libraryHasDatabaseSupport )
        << "This test executable is compiled with NO_DB but the utilities "
           "library it links was built with database support.  Configure the "
           "whole tree with -DUS3_NO_DB=ON instead of defining NO_DB on the "
           "test target.";
#else
    EXPECT_TRUE( libraryHasDatabaseSupport )
        << "This test executable is compiled with database support but the "
           "utilities library it links was built with NO_DB.  Configure the "
           "whole tree consistently (US3_NO_DB controls both).";
#endif
}
