// UT-005: tests named for US_DB2 must execute production US_DB2 logic.
//
// This file previously held twenty cases that programmed a US_DB2_Mock with
// EXPECT_CALL(...).WillOnce(Return(x)) and then asserted the mock returned x.
// They exercised googlemock, not the database layer -- UT-002 measured
// us_db2.cpp at 0.0% line coverage despite this file existing.  See the UT-005
// report for the per-case disposition.
//
// What remains testable offline is the deterministic text assembly in
// composeQuery(): stored-procedure invocation, credential placement and quote
// escaping.  Everything else in US_DB2 needs a live server and belongs to the
// integration suite defined in UT-014.

#include "qt_test_base.h"
#include "mock_us_db2.h"
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
}

// ---------------------------------------------------------------------------
// Query composition -- production US_DB2 logic, no connection involved
// ---------------------------------------------------------------------------

TEST( UsDb2Query, CredentialsArePassedAsTheFirstTwoProcedureArguments )
{
    EXPECT_EQ( call( QStringList() << "get_user_info" ),
               QString( "CALL get_user_info('%1', '%2')" ).arg( kGuid ).arg( kPw ) );
}

TEST( UsDb2Query, CallerArgumentsFollowTheCredentialsInOrder )
{
    EXPECT_EQ( call( QStringList() << "get_experiment" << "42" << "RI" ),
               QString( "CALL get_experiment('%1', '%2', '42', 'RI')" )
                   .arg( kGuid ).arg( kPw ) );
}

TEST( UsDb2Query, TheKeywordSelectsBetweenAProcedureCallAndAFunctionSelect )
{
    const QStringList arguments = QStringList() << "count_rows" << "7";

    EXPECT_TRUE( US_DB2::composeQuery( "CALL", arguments, kGuid, kPw )
                     .startsWith( "CALL count_rows(" ) );
    EXPECT_TRUE( US_DB2::composeQuery( "SELECT", arguments, kGuid, kPw )
                     .startsWith( "SELECT count_rows(" ) );
}

TEST( UsDb2Query, ArgumentsDifferingOnlyByKeywordAreOtherwiseIdentical )
{
    const QStringList arguments = QStringList() << "count_rows" << "7";
    const QString     asCall    = US_DB2::composeQuery( "CALL",   arguments, kGuid, kPw );
    const QString     asSelect  = US_DB2::composeQuery( "SELECT", arguments, kGuid, kPw );

    EXPECT_EQ( asCall.mid( QString( "CALL" ).size() ),
               asSelect.mid( QString( "SELECT" ).size() ) );
}

TEST( UsDb2Query, SingleQuotesInAnArgumentAreEscaped )
{
    // The argument delimiter is a single quote, so an unescaped quote in user
    // data would terminate the literal early.
    EXPECT_EQ( call( QStringList() << "find_buffer" << "O'Brien" ),
               QString( "CALL find_buffer('%1', '%2', 'O\\'Brien')" )
                   .arg( kGuid ).arg( kPw ) );
}

TEST( UsDb2Query, EveryQuoteInAnArgumentIsEscapedNotJustTheFirst )
{
    const QString query = call( QStringList() << "find_buffer" << "a'b'c" );

    EXPECT_TRUE( query.contains( "'a\\'b\\'c'" ) ) << qPrintable( query );
}

TEST( UsDb2Query, EachArgumentIsEscapedIndependently )
{
    const QString query = call( QStringList() << "p" << "x'1" << "y'2" );

    EXPECT_TRUE( query.contains( "'x\\'1'" ) ) << qPrintable( query );
    EXPECT_TRUE( query.contains( "'y\\'2'" ) ) << qPrintable( query );
}

TEST( UsDb2Query, TheProcedureNameIsNotEscaped )
{
    // OBSERVED: only arguments 1..n are escaped.  The procedure name is
    // interpolated verbatim, so it must never come from untrusted input.
    EXPECT_TRUE( call( QStringList() << "od'd" ).contains( "CALL od'd(" ) );
}

TEST( UsDb2Query, BackslashesInAnArgumentAreNotEscaped )
{
    // OBSERVED-DEFECT: only the quote character is escaped.  A trailing
    // backslash escapes the closing delimiter under MySQL's default
    // NO_BACKSLASH_ESCAPES=off, so this composes a query the server misreads.
    EXPECT_TRUE( call( QStringList() << "p" << "path\\" ).contains( "'path\\'" ) );
}

TEST( UsDb2Query, AnEmptyArgumentListYieldsAnEmptyQuery )
{
    // The procedure name is arguments[ 0 ]; an empty list has no procedure to
    // call.  Before UT-005 this indexed past the end of the list.
    EXPECT_TRUE( US_DB2::composeQuery( "CALL", QStringList(), kGuid, kPw ).isEmpty() );
}

TEST( UsDb2Query, AnEmptyArgumentIsStillPassedAsAnEmptyLiteral )
{
    EXPECT_EQ( call( QStringList() << "p" << "" << "z" ),
               QString( "CALL p('%1', '%2', '', 'z')" ).arg( kGuid ).arg( kPw ) );
}

// ---------------------------------------------------------------------------
// Mock contract
//
// All that is worth asserting about US_DB2_Mock is that it still satisfies the
// interface its consumers inject it through.  Behavioral claims about the
// database belong to tests that call US_DB2.
// ---------------------------------------------------------------------------

TEST( UsDb2MockContract, TheMockIsUsableWhereverTheInterfaceIsRequired )
{
    US_DB2_Mock mock;
    IUS_DB2*    injected = &mock;

    ASSERT_NE( injected, nullptr );
}
