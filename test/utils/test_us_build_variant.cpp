// Prove the test executable and the utils library were compiled with the
// same NO_DB setting: us_db2.h declares MYSQL members only when NO_DB is
// undefined, so a mismatch is an ODR violation on US_DB2.  The library is
// probed through us_debug(), which uses QSettings only in the DB build.

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
