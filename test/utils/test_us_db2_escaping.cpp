// Escaping happens exactly once, in composeQuery(), and nowhere else.
//
// Four call sites -- model XML, noise XML, time-state definitions and the
// experiment RI profile -- used to run mysqlEscapeString() over their payload
// before handing it to a query builder that escapes again.  What reached the
// database was double-escaped, so a value came back carrying literal escape
// bytes it never had.  These pin the single-escape contract those call sites
// now depend on, using the characters that actually broke.

#include "qt_test_base.h"
#include "us_db2.h"

#include <QString>
#include <QStringList>

namespace
{
const QString kGuid = "11111111-2222-3333-4444-555555555555";
const QString kPw   = "s3cret";

QString call( const QStringList& arguments )
{
    return US_DB2::composeQuery( "CALL", arguments, kGuid, kPw );
}

//! The argument as it appears inside its quotes in the composed query.
QString storedArgument( const QString& query, int index )
{
    // CALL name('a', 'b', ...) -- split on "', '" after the opening quote.
    const int open = query.indexOf( "('" );
    if ( open < 0 ) return QString();

    QString body = query.mid( open + 2 );
    body.chop( 2 );                       // trailing "')"

    return body.split( "', '" ).value( index );
}
}

TEST( UsDb2Escaping, ASingleQuoteIsEscapedOnce )
{
    const QString xml = "<model desc='O'Brien'/>";

    EXPECT_EQ( storedArgument( call( QStringList() << "new_model" << xml ), 2 )
                   .toStdString(),
               "<model desc=\\'O\\'Brien\\'/>" );
}

TEST( UsDb2Escaping, ABackslashIsEscapedOnce )
{
    const QString xml = "<model path='C:\\runs\\demo'/>";

    EXPECT_EQ( storedArgument( call( QStringList() << "new_model" << xml ), 2 )
                   .toStdString(),
               "<model path=\\'C:\\\\runs\\\\demo\\'/>" );
}

TEST( UsDb2Escaping, BackslashesAreDoubledBeforeQuotesAreEscaped )
{
    // Order matters: escaping the quote first would leave a backslash that the
    // backslash pass then doubles, turning \' into \\' -- a literal backslash
    // followed by an unescaped quote, which ends the string early.
    const QString value = "a\\'b";

    EXPECT_EQ( storedArgument( call( QStringList() << "new_model" << value ), 2 )
                   .toStdString(),
               "a\\\\\\'b" );
}

TEST( UsDb2Escaping, APayloadThatWasPreEscapedWouldBeVisiblyDoubled )
{
    // What the four call sites used to send.  This is not the contract; it is
    // the shape of the bug, kept so the difference is legible.
    const QString raw        = "<v desc='x'/>";
    const QString preEscaped = "<v desc=\\'x\\'/>";

    const QString once  = storedArgument(
        call( QStringList() << "new_model" << raw ), 2 );
    const QString twice = storedArgument(
        call( QStringList() << "new_model" << preEscaped ), 2 );

    EXPECT_NE( once, twice );
    EXPECT_EQ( twice.toStdString(), "<v desc=\\\\\\'x\\\\\\'/>" );
}

TEST( UsDb2Escaping, EveryArgumentIsEscapedIncludingTheCredentials )
{
    // The credentials sit in the same kind of quoted literal as everything
    // else.  They were interpolated as written until this was fixed, which was
    // survivable while both came from a stored profile and is not now that
    // us_import_run passes a password straight from its command line.
    const QString query = US_DB2::composeQuery(
        "CALL", QStringList() << "validate_user" << "pw'; DROP--",
        "guid'--", QString() );

    EXPECT_THAT( query.toStdString(),
                 ::testing::HasSubstr( "'guid\\'--'" ) );
    EXPECT_THAT( query.toStdString(),
                 ::testing::HasSubstr( "'pw\\'; DROP--'" ) );

    // Nothing closed the literal early, so the call is still one statement.
    EXPECT_EQ( query.count( "DROP" ), 1 );
    EXPECT_TRUE( query.endsWith( ")" ) );
}

TEST( UsDb2Escaping, AnXmlPayloadSurvivesComposeUnchangedApartFromEscaping )
{
    // Round-tripping the escape by hand is what the server does on the way in;
    // if these agree, the value stored is the value given.
    const QString xml =
        "<US_Noise><values count='2'><v r='5.8' n=\"0.1\"/>"
        "<v r='5.9' n='-0.2'/></values><path>a\\b</path></US_Noise>";

    QString stored = storedArgument(
        call( QStringList() << "new_noise" << xml ), 2 );

    // Undo exactly one level, in the reverse of the order it was applied.
    stored.replace( "\\'", "'" );
    stored.replace( "\\\\", "\\" );

    EXPECT_EQ( stored.toStdString(), xml.toStdString() );
}
