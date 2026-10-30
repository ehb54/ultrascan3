//! \file test_us_db2.cpp
// US_DB2::composeQuery(): stored-procedure invocation, credential placement and
// argument escaping.  The rest of US_DB2 needs a live server.

#include "qt_test_base.h"
#include "us_db2.h"

#include <QString>
#include <QStringList>

namespace
{
const QString kGuid = "11111111-2222-3333-4444-555555555555";
const QString kPw   = "s3cret";

QString       call( const QStringList& arguments )
{
   return US_DB2::composeQuery( "CALL", arguments, kGuid, kPw );
}
}

TEST( UsDb2Query, CredentialsArePassedAsTheFirstTwoProcedureArguments )
{
   EXPECT_EQ( call( QStringList() << "get_user_info" ), QString( "CALL get_user_info('%1', '%2')" ).arg( kGuid ).arg( kPw ) );
}

TEST( UsDb2Query, CallerArgumentsFollowTheCredentialsInOrder )
{
   EXPECT_EQ( call( QStringList() << "get_experiment" << "42" << "RI" ),
              QString( "CALL get_experiment('%1', '%2', '42', 'RI')" ).arg( kGuid ).arg( kPw ) );
}

TEST( UsDb2Query, TheKeywordSelectsBetweenAProcedureCallAndAFunctionSelect )
{
   const QStringList arguments = QStringList() << "count_rows" << "7";

   EXPECT_TRUE( US_DB2::composeQuery( "CALL", arguments, kGuid, kPw ).startsWith( "CALL count_rows(" ) );
   EXPECT_TRUE( US_DB2::composeQuery( "SELECT", arguments, kGuid, kPw ).startsWith( "SELECT count_rows(" ) );
}

TEST( UsDb2Query, ArgumentsDifferingOnlyByKeywordAreOtherwiseIdentical )
{
   const QStringList arguments = QStringList() << "count_rows" << "7";
   const QString     asCall    = US_DB2::composeQuery( "CALL", arguments, kGuid, kPw );
   const QString     asSelect  = US_DB2::composeQuery( "SELECT", arguments, kGuid, kPw );

   EXPECT_EQ( asCall.mid( QString( "CALL" ).size() ), asSelect.mid( QString( "SELECT" ).size() ) );
}

TEST( UsDb2Query, SingleQuotesInAnArgumentAreEscaped )
{
   // The argument delimiter is a single quote, so an unescaped quote in user
   // data would terminate the literal early.
   EXPECT_EQ( call( QStringList() << "find_buffer" << "O'Brien" ),
              QString( "CALL find_buffer('%1', '%2', 'O\\'Brien')" ).arg( kGuid ).arg( kPw ) );
}

TEST( UsDb2Query, EveryQuoteInAnArgumentIsEscapedNotJustTheFirst )
{
   EXPECT_EQ( call( QStringList() << "find_buffer" << "a'b'c" ),
              QString( "CALL find_buffer('%1', '%2', 'a\\'b\\'c')" ).arg( kGuid ).arg( kPw ) );
}

TEST( UsDb2Query, EachArgumentIsEscapedIndependently )
{
   EXPECT_EQ( call( QStringList() << "p" << "x'1" << "y'2" ),
              QString( "CALL p('%1', '%2', 'x\\'1', 'y\\'2')" ).arg( kGuid ).arg( kPw ) );
}

TEST( UsDb2Query, AProcedureNameThatIsNotAPlainIdentifierIsRefused )
{
   // The name is interpolated as a SQL identifier rather than a quoted
   // literal, so escaping cannot make it safe and it is rejected instead.
   EXPECT_TRUE( call( QStringList() << "od'd" ).isEmpty() );
   EXPECT_TRUE( call( QStringList() << "p; DROP TABLE users" ).isEmpty() );
   EXPECT_TRUE( call( QStringList() << "" ).isEmpty() );
   EXPECT_TRUE( call( QStringList() << "9starts_with_a_digit" ).isEmpty() );
}

TEST( UsDb2Query, OrdinaryProcedureNamesAreStillAccepted )
{
   // The guard must not reject the names the codebase actually uses.
   EXPECT_FALSE( call( QStringList() << "get_analyte_info" ).isEmpty() );
   EXPECT_FALSE( call( QStringList() << "_leading_underscore" ).isEmpty() );
   EXPECT_FALSE( call( QStringList() << "Mixed_Case9" ).isEmpty() );
}

TEST( UsDb2Query, BackslashesInAnArgumentAreEscaped )
{
   // A trailing backslash would otherwise escape the closing delimiter under
   // MySQL's default NO_BACKSLASH_ESCAPES=off and run the argument into the
   // next one.
   EXPECT_EQ( call( QStringList() << "p" << "path\\" ), QString( "CALL p('%1', '%2', 'path\\\\')" ).arg( kGuid ).arg( kPw ) );
}

TEST( UsDb2Query, ABackslashBeforeAQuoteIsEscapedExactlyOnce )
{
   // Order matters:  escaping the quote first would double the backslash the
   // quote step inserts, producing a literal backslash and an open quote.
   EXPECT_EQ( call( QStringList() << "p" << "a\\'b" ), QString( "CALL p('%1', '%2', 'a\\\\\\'b')" ).arg( kGuid ).arg( kPw ) );
}

TEST( UsDb2Query, AnEmptyArgumentListYieldsAnEmptyQuery )
{
   // The procedure name is arguments[ 0 ]; an empty list has no procedure to
   // call.
   EXPECT_TRUE( US_DB2::composeQuery( "CALL", QStringList(), kGuid, kPw ).isEmpty() );
}

TEST( UsDb2Query, AnEmptyArgumentIsStillPassedAsAnEmptyLiteral )
{
   EXPECT_EQ( call( QStringList() << "p" << "" << "z" ), QString( "CALL p('%1', '%2', '', 'z')" ).arg( kGuid ).arg( kPw ) );
}
