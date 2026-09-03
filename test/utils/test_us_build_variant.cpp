// Prove that the test executable and the utilities library it links were
// compiled with consistent database-related definitions. US_DB2 changes layout
// with NO_DB, and other utilities also change behavior, so mixing variants is
// unsafe.
//
// The linked library's variant is detected through US_Settings::us_debug(): the
// DB-enabled implementation persists the level through the sandboxed QSettings
// store, while the NO_DB implementation keeps it in process-local storage.

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

    // Has to be the same store the library writes to, which is the sandbox one
    // here.  Reaching for QSettings( US3, "UltraScan" ) directly would probe the
    // developer's real settings instead, find nothing there, and report the
    // library as a NO_DB build when it is nothing of the sort.
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
