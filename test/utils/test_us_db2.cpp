//! \file test_us_db2.cpp
// US_DB2::composeQuery(): stored-procedure invocation and credential placement,
// and how a refused procedure name fails the query and the blob transfers.  The escaping itself is in
// test_us_db2_escaping.cpp.  US_DB2::writeBlobFile(): how a downloaded blob is written to its file.
// The rest of US_DB2 needs a live server.

#include "qt_test_base.h"
#include "us_db2.h"

#include <QFile>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QTemporaryFile>

#ifdef Q_OS_UNIX
#include <csignal>
#include <sys/resource.h>
#endif

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

TEST( UsDb2Query, AnEmptyArgumentListYieldsAnEmptyQuery )
{
   // The procedure name is arguments[ 0 ]; an empty list has no procedure to
   // call.
   EXPECT_TRUE( US_DB2::composeQuery( "CALL", QStringList(), kGuid, kPw ).isEmpty() );
}

TEST( UsDb2Query, ARefusedProcedureNameFailsTheQueryWithItsOwnError )
{
   // Nothing is sent, so no connection is needed to see the refusal.
   US_DB2        db;
   const QString name = "p; DROP TABLE users";

   EXPECT_EQ( db.statusQuery( QStringList() << name << "1" ), IUS_DB2::DBERROR );
   EXPECT_EQ( db.lastError(), QObject::tr( US_DB2::invalidProcedureMessage ).arg( name ) );

   EXPECT_EQ( db.functionQuery( QStringList() << "od'd" ), IUS_DB2::DBERROR );
   EXPECT_EQ( db.lastError(), QObject::tr( US_DB2::invalidProcedureMessage ).arg( "od'd" ) );

   db.query( QStringList() << "" );
   EXPECT_EQ( db.lastErrno(), IUS_DB2::DBERROR );
   EXPECT_FALSE( db.next() );
}

// NO_DB compiles blob transfers as no-op stubs; this validates the DB adapter.
#ifndef NO_DB
TEST( UsDb2Query, TheBlobTransfersRefuseAnInvalidProcedureName )
{
   // A readable, non-empty file, so only the procedure name can be refused.
   QTemporaryFile file;
   ASSERT_TRUE( file.open() );
   file.write( "blob" );
   file.close();

   US_DB2        db;
   const QString name = "p; DROP TABLE users";

   EXPECT_EQ( db.writeBlobToDB( file.fileName(), name, 1 ), IUS_DB2::DBERROR );
   EXPECT_EQ( db.lastError(), QObject::tr( US_DB2::invalidProcedureMessage ).arg( name ) );

   EXPECT_EQ( db.readBlobFromDB( file.fileName(), "od'd", 1 ), IUS_DB2::DBERROR );
   EXPECT_EQ( db.lastError(), QObject::tr( US_DB2::invalidProcedureMessage ).arg( "od'd" ) );
}

#endif

TEST( UsDb2Query, AnEmptyArgumentIsStillPassedAsAnEmptyLiteral )
{
   EXPECT_EQ( call( QStringList() << "p" << "" << "z" ), QString( "CALL p('%1', '%2', '', 'z')" ).arg( kGuid ).arg( kPw ) );
}

TEST( UsDb2Query, DisconnectedAdapterDoesNotOwnATransaction )
{
   US_DB2 db;
   EXPECT_FALSE( db.transactionActive() );
   QString error;
   EXPECT_FALSE( db.beginTransaction( error ) );
   EXPECT_FALSE( db.transactionActive() );
}

// writeBlobFile():  how readBlobFromDB() writes a download (ultrascan-tickets#1251).
TEST( UsDb2BlobFile, WritesEveryByte )
{
   QTemporaryDir dir;
   ASSERT_TRUE( dir.isValid() );
   const QString    path = dir.filePath( "run.auc" );
   const QByteArray data( 1 << 20, 'x' );
   QString          error;
   EXPECT_EQ( US_DB2::writeBlobFile( path, data, error ), IUS_DB2::OK );
   EXPECT_TRUE( error.isEmpty() );
   QFile file( path );
   ASSERT_TRUE( file.open( QIODevice::ReadOnly ) );
   EXPECT_EQ( file.readAll(), data );
}

TEST( UsDb2BlobFile, AFileThatCannotBeOpenedIsAnError )
{
   QTemporaryDir dir;
   ASSERT_TRUE( dir.isValid() );
   const QString path = dir.filePath( "missing-folder/run.auc" );
   QString       error;
   EXPECT_EQ( US_DB2::writeBlobFile( path, QByteArray( "data" ), error ), IUS_DB2::DBERROR );
   EXPECT_TRUE( error.contains( path ) );
   EXPECT_FALSE( QFile::exists( path ) );
}

#ifdef Q_OS_UNIX
// A short write, as on a full disk:  the process's file size limit stops the
// write partway.  The error is reported and the partial file removed.
TEST( UsDb2BlobFile, AShortWriteIsAnErrorAndLeavesNoPartialFile )
{
   QTemporaryDir dir;
   ASSERT_TRUE( dir.isValid() );
   const QString path = dir.filePath( "run.auc" );
   QString       error;
   int           status = IUS_DB2::OK;

   struct rlimit saved;
   ASSERT_EQ( getrlimit( RLIMIT_FSIZE, &saved ), 0 );
   void ( *savedHandler )( int ) = signal( SIGXFSZ, SIG_IGN ); // EFBIG instead of the signal
   struct rlimit limited         = saved;
   limited.rlim_cur              = 4096;
   ASSERT_EQ( setrlimit( RLIMIT_FSIZE, &limited ), 0 );
   status = US_DB2::writeBlobFile( path, QByteArray( 1 << 16, 'x' ), error );
   setrlimit( RLIMIT_FSIZE, &saved );
   signal( SIGXFSZ, savedHandler );

   EXPECT_EQ( status, IUS_DB2::DBERROR );
   EXPECT_TRUE( error.contains( "65536" ) ) << error.toStdString();
   EXPECT_FALSE( QFile::exists( path ) );
}
#endif
