//! \file test_us_model_db.cpp
#include "mock_us_db2.h"
#include "qt_test_base.h"
#include "us_model.h"
#include "us_settings.h"
#include <QDir>
#include <QFile>
#include <QXmlStreamReader>

using ::testing::_;
using ::testing::Eq;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::Matcher;
using ::testing::Return;
using ::testing::StrictMock;

// Known defects these tests deliberately do not assert:
// - load() and write() dereference a null DB pointer.
// - When a lookup or info query succeeds without a row, load() still returns OK, clearing the
//   components (an empty ID is sent to get_model_info after an empty lookup).
// - load() ignores XML errors, so truncated XML returns OK with partial data.
// - load() converts the stored XML with toLatin1(), so a model with non-ASCII text (for example
//   "λ 280 nm – Å") returns OK without its stored data.
// - A failed write() still saves the model to disk and reports "created".

namespace
{
const QString               kGuid     = "11111111-2222-3333-4444-555555555555";
const QString               kEditGuid = "66666666-7777-8888-9999-000000000000";

Matcher<const QStringList&> args( const QStringList& expected )
{
   return Matcher<const QStringList&>( Eq( expected ) );
}

// Attributes of the <model> element in a model XML document.
QXmlStreamAttributes modelAttributes( const QString& xml )
{
   QXmlStreamReader reader( xml );
   while ( !reader.atEnd() )
   {
      reader.readNext();
      if ( reader.isStartElement() && reader.name() == QLatin1String( "model" ) )
      {
         return reader.attributes();
      }
   }
   ADD_FAILURE() << "no <model> element in: " << xml.toStdString();
   return {};
}

const char* kStoredXml = "<?xml version=\"1.0\"?><!DOCTYPE US_Model><ModelData version=\"1.0\">"
                         "<model description=\"stored &amp; loaded\" modelGUID=\"11111111-2222-3333-4444-555555555555\" "
                         "editGUID=\"66666666-7777-8888-9999-000000000000\" variance=\"0.25\" meniscus=\"5.9\" "
                         "analysisType=\"1\"><analyte name=\"solute\" vbar20=\"0.72\" mw=\"50000\" s=\"4.5e-13\" "
                         "D=\"6e-07\" f_f0=\"1.3\"/></model></ModelData>";

class ModelDb : public QtTestBase
{
   protected:
      StrictMock<US_DB2_Mock> db;
      US_Model                model;

      void                    SetUp() override
      {
         ASSERT_NO_FATAL_FAILURE( QtTestBase::SetUp() );
         US_Settings::set_us_inv_ID( 23 );
      }

      void prepareModel()
      {
         model.modelGUID   = kGuid;
         model.editGUID    = kEditGuid;
         model.description = "test-CustomGrid.model";
         model.analysis    = US_Model::CUSTOMGRID;
         model.subGrids    = 3;
         model.variance    = 0.25;
         model.meniscus    = 5.9;
         US_Model::SimulationComponent sc;
         sc.name   = "solute";
         sc.vbar20 = 0.72;
         sc.mw     = 50000;
         sc.s      = 4.5e-13;
         model.components << sc;
      }

      // Contents of every file in the model directory, keyed by name.
      QMap<QString, QByteArray> modelFiles()
      {
         QString path;
         EXPECT_TRUE( US_Model::model_path( path ) );
         QMap<QString, QByteArray> files;
         for ( const QString& name : QDir( path ).entryList( QDir::Files ) )
         {
            QFile file( path + "/" + name );
            EXPECT_TRUE( file.open( QIODevice::ReadOnly ) ) << name.toStdString();
            files[name] = file.readAll();
         }
         return files;
      }

      void expectInfo( const QString& id, const QString& xml )
      {
         EXPECT_CALL( db, query( args( { "get_model_info", id } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
         EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
         EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( xml ) ) );
         EXPECT_CALL( db, value( 6 ) ).WillOnce( Return( QVariant( "2026-11-14 09:30:00" ) ) );
      }
};

// Load

TEST_F( ModelDb, LoadByGuidLooksUpTheIdAndParsesTheStoredXml )
{
   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "get_modelID", kGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "42" ) ) );
      expectInfo( "42", kStoredXml );
   }

   ASSERT_EQ( IUS_DB2::OK, model.load( true, kGuid, &db ) );
   EXPECT_EQ( QString( "stored & loaded" ), model.description );
   EXPECT_EQ( kGuid, model.modelGUID );
   EXPECT_EQ( kEditGuid, model.editGUID );
   EXPECT_DOUBLE_EQ( 0.25, model.variance );
   EXPECT_DOUBLE_EQ( 5.9, model.meniscus );
   EXPECT_EQ( US_Model::TWODSA, model.analysis );
   EXPECT_EQ( QString( "2026-11-14 09:30:00" ), model.timeCreated );
   ASSERT_EQ( 1, model.components.size() );
   EXPECT_EQ( QString( "solute" ), model.components[0].name );
   EXPECT_DOUBLE_EQ( 0.72, model.components[0].vbar20 );
   EXPECT_DOUBLE_EQ( 50000, model.components[0].mw );
   EXPECT_DOUBLE_EQ( 4.5e-13, model.components[0].s );
   EXPECT_DOUBLE_EQ( 6e-07, model.components[0].D );
   EXPECT_DOUBLE_EQ( 1.3, model.components[0].f_f0 );
}

TEST_F( ModelDb, LoadByIdSkipsTheGuidLookup )
{
   {
      InSequence sequence;
      expectInfo( "42", kStoredXml );
   }
   ASSERT_EQ( IUS_DB2::OK, model.load( QString( "42" ), &db ) );
   EXPECT_EQ( QString( "stored & loaded" ), model.description );
}

TEST_F( ModelDb, FailedGuidLookupReturnsTheErrorWithoutReadingTheModel )
{
   for ( int error : { IUS_DB2::NOROWS, IUS_DB2::DBERROR } )
   {
      SCOPED_TRACE( error );
      prepareModel();
      const US_Model previous = model;
      {
         InSequence sequence;
         EXPECT_CALL( db, query( args( { "get_modelID", kGuid } ) ) );
         EXPECT_CALL( db, lastErrno() ).Times( ::testing::AtLeast( 1 ) ).WillRepeatedly( Return( error ) );
      }
      EXPECT_EQ( error, model.load( true, kGuid, &db ) );
      EXPECT_TRUE( model == previous );
      ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
   }
}

TEST_F( ModelDb, FailedInfoQueryReturnsTheErrorWithoutReadingColumns )
{
   prepareModel();
   const US_Model previous = model;
   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "get_model_info", "42" } ) ) );
      EXPECT_CALL( db, lastErrno() ).Times( ::testing::AtLeast( 1 ) ).WillRepeatedly( Return( IUS_DB2::DBERROR ) );
   }
   EXPECT_EQ( IUS_DB2::DBERROR, model.load( QString( "42" ), &db ) );
   EXPECT_TRUE( model == previous );
}

// Write

TEST_F( ModelDb, WriteCreatesANewModelWithRawXmlAndTheInvestigator )
{
   prepareModel();
   QStringList written;
   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "get_modelID", kGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) )
         .WillOnce( Invoke(
            [&written]( const QStringList& q )
            {
               written = q;
               return IUS_DB2::OK;
            } ) );
   }

   EXPECT_EQ( IUS_DB2::OK, model.write( true, QString(), &db ) );
   ASSERT_EQ( 8, written.size() );
   EXPECT_EQ( ( QStringList{ "new_model", kGuid, "test-CustomGrid.model" } ), written.mid( 0, 3 ) );
   EXPECT_EQ( ( QStringList{ "0.25", "5.9", kEditGuid, "23" } ), written.mid( 4 ) );
   EXPECT_EQ( QString( "created" ), model.message );

   // The XML reaches the DB layer unescaped; the query layer escapes every argument.
   const QString xml = written[3];
   EXPECT_TRUE( xml.contains( "<?xml version=\"1.0\"" ) );
   EXPECT_FALSE( xml.contains( "\\\"" ) );
   EXPECT_FALSE( xml.contains( "\\n" ) );
   const QXmlStreamAttributes attrs = modelAttributes( xml );
   EXPECT_EQ( kGuid, attrs.value( "modelGUID" ).toString() );
   EXPECT_EQ( QString( "test-CustomGrid.model" ), attrs.value( "description" ).toString() );
   EXPECT_EQ( QString( "3" ), attrs.value( "subGrids" ).toString() );

#ifndef NO_DB
   // NO_DB builds have no model directory: model_path() returns an empty path.
   const QMap<QString, QByteArray> files = modelFiles();
   ASSERT_EQ( 1, files.size() );
   EXPECT_EQ( kGuid, modelAttributes( QString::fromUtf8( files.first() ) ).value( "modelGUID" ).toString() );
#endif
}

TEST_F( ModelDb, WriteUpdatesAnExistingModelWithoutTheInvestigator )
{
   prepareModel();
   QStringList written;
   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "get_modelID", kGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "42" ) ) );
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) )
         .WillOnce( Invoke(
            [&written]( const QStringList& q )
            {
               written = q;
               return IUS_DB2::OK;
            } ) );
   }

   EXPECT_EQ( IUS_DB2::OK, model.write( true, QString(), &db ) );
   ASSERT_EQ( 7, written.size() );
   EXPECT_EQ( ( QStringList{ "update_model", "42", "test-CustomGrid.model" } ), written.mid( 0, 3 ) );
   EXPECT_EQ( ( QStringList{ "0.25", "5.9", kEditGuid } ), written.mid( 4 ) );
   EXPECT_EQ( kGuid, modelAttributes( written[3] ).value( "modelGUID" ).toString() );
   EXPECT_EQ( QString( "updated" ), model.message );
}

TEST_F( ModelDb, WriteAssignsAGuidWhenTheModelHasNone )
{
   prepareModel();
   model.modelGUID.clear();
   QString lookedUp, written;
   {
      InSequence sequence;
      EXPECT_CALL( db, query( Matcher<const QStringList&>( _ ) ) )
         .WillOnce( Invoke(
            [&lookedUp]( const QStringList& q )
            {
               EXPECT_EQ( QString( "get_modelID" ), q.value( 0 ) );
               lookedUp = q.value( 1 );
            } ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) )
         .WillOnce( Invoke(
            [&written]( const QStringList& q )
            {
               written = q.value( 1 );
               return IUS_DB2::OK;
            } ) );
   }

   EXPECT_EQ( IUS_DB2::OK, model.write( true, QString(), &db ) );
   EXPECT_EQ( 36, model.modelGUID.size() );
   EXPECT_EQ( model.modelGUID, lookedUp );
   EXPECT_EQ( model.modelGUID, written );
}

TEST_F( ModelDb, WriteReturnsTheStatusOfAFailedWrite )
{
   prepareModel();
   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "get_modelID", kGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) ).WillOnce( Return( IUS_DB2::DBERROR ) );
   }
   EXPECT_EQ( IUS_DB2::DBERROR, model.write( true, QString(), &db ) );
}
} // namespace
