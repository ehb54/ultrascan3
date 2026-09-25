// Verify tests and utilities use the same NO_DB setting; US_DB2 layout depends on it.
// DB builds persist us_debug in settings; NO_DB builds keep it in memory.

#include "qt_test_base.h"
#include "us_defines.h"
#include "us_settings.h"

#include <QSettings>

namespace
{
// Only DB-enabled utilities persist the debug level in settings.
bool linkedUtilsLibraryPersistsDebugLevel()
{
    const int  restore = US_Settings::us_debug();
    const char key[]   = "us_debug";

    // Probe the library's sandbox store, not the user's native settings.
    US_SettingsStore settings;
    settings.remove( key );
    settings.sync();

    // A non-zero level; set_us_debug() removes the key instead of storing zero.
    US_Settings::set_us_debug( 7 );

    US_SettingsStore probe;
    probe.sync();
    const bool persisted = probe.value( key, 0 ).toInt() == 7;

    US_Settings::set_us_debug( restore );
    return persisted;
}
}

TEST( BuildVariantConsistency, TestExecutableMatchesLinkedUtilsLibrary )
{
    // Both variants must round-trip the debug level.
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
