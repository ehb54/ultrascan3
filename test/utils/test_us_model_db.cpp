//! \file test_us_model_db.cpp
#include "mock_us_db2.h"
#include "qt_test_base.h"
#include "us_model.h"
#include "us_settings.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QXmlStreamReader>
#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
#include <QTextCodec>
#endif

using ::testing::_;
using ::testing::Eq;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::Matcher;
using ::testing::Return;
using ::testing::StrictMock;

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

TEST_F( ModelDb, FailedWriteReturnsItsStatusAndKeepsTheDiskCopy )
{
   prepareModel();
   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "get_modelID", kGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) ).WillOnce( Return( IUS_DB2::DBERROR ) );
   }
   EXPECT_EQ( IUS_DB2::DBERROR, model.write( true, QString(), &db ) );
#ifdef NO_DB
   // NO_DB builds have no models directory, so whether the copy is saved depends on the host.
   EXPECT_TRUE( model.message.startsWith( "could not be written; " ) ) << model.message.toStdString();
#else
   EXPECT_EQ( QString( "could not be written; a local copy was saved and can be uploaded later" ), model.message );
   // The model stays on disk so it can be uploaded later.
   const QMap<QString, QByteArray> files = modelFiles();
   ASSERT_EQ( 1, files.size() );
   EXPECT_EQ( kGuid, modelAttributes( QString::fromUtf8( files.first() ) ).value( "modelGUID" ).toString() );
#endif
}

TEST_F( ModelDb, FailedLookupWritesOnlyTheDiskCopy )
{
   prepareModel();
   for ( int status : { IUS_DB2::DBERROR, IUS_DB2::NOT_CONNECTED } )
   {
      SCOPED_TRACE( status );
      {
         InSequence sequence;
         EXPECT_CALL( db, query( args( { "get_modelID", kGuid } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( status ) );
      }
      EXPECT_EQ( status, model.write( true, QString(), &db ) );
#ifdef NO_DB
      EXPECT_TRUE( model.message.startsWith( "could not be looked up; " ) ) << model.message.toStdString();
#else
      EXPECT_EQ( QString( "could not be looked up; a local copy was saved and can be uploaded later" ), model.message );
#endif
      ::testing::Mock::VerifyAndClearExpectations( &db );
   }
#ifndef NO_DB
   // The model stays on disk so it can be uploaded later.
   const QMap<QString, QByteArray> files = modelFiles();
   ASSERT_EQ( 1, files.size() );
   EXPECT_EQ( kGuid, modelAttributes( QString::fromUtf8( files.first() ) ).value( "modelGUID" ).toString() );
#endif
}

#ifndef NO_DB
TEST_F( ModelDb, FailedWriteSaysWhenNoLocalCopyCouldBeSaved )
{
   prepareModel();
   // A plain file where the models directory belongs makes the disk copy fail too.
   QString path;
   ASSERT_TRUE( US_Model::model_path( path ) );
   ASSERT_TRUE( QDir( path ).removeRecursively() );
   QFile blocker( path );
   ASSERT_TRUE( blocker.open( QIODevice::WriteOnly ) );
   blocker.close();
   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "get_modelID", kGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) ).WillOnce( Return( IUS_DB2::DBERROR ) );
   }
   EXPECT_EQ( IUS_DB2::DBERROR, model.write( true, QString(), &db ) );
   EXPECT_EQ( QString( "could not be written; no local copy could be saved either" ), model.message );
   EXPECT_TRUE( QFileInfo( path ).isFile() );
}
#endif

TEST_F( ModelDb, WriteInsertsWhenTheLookupFindsNoRow )
{
   prepareModel();
   QStringList written;
   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "get_modelID", kGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) )
         .WillOnce( Invoke(
            [&written]( const QStringList& q )
            {
               written = q;
               return IUS_DB2::OK;
            } ) );
   }
   EXPECT_EQ( IUS_DB2::OK, model.write( true, QString(), &db ) );
   EXPECT_EQ( QString( "new_model" ), written.value( 0 ) );
}

// A failed or empty load leaves the model it was called on unchanged.
class ModelDbLoadFailure : public ModelDb
{
   protected:
      void SetUp() override
      {
         ASSERT_NO_FATAL_FAILURE( ModelDb::SetUp() );
         prepareModel();
      }

      void expectModelUnchanged()
      {
         EXPECT_EQ( QString( "test-CustomGrid.model" ), model.description );
         EXPECT_EQ( kGuid, model.modelGUID );
         ASSERT_EQ( 1, model.components.size() );
         EXPECT_EQ( QString( "solute" ), model.components[0].name );
      }
};

TEST_F( ModelDbLoadFailure, NullDatabaseIsNotConnectedAndWriteKeepsTheDiskCopy )
{
   EXPECT_EQ( IUS_DB2::NOT_CONNECTED, model.load( true, kGuid, nullptr ) );
   EXPECT_EQ( IUS_DB2::NOT_CONNECTED, model.load( QString( "42" ), nullptr ) );
   EXPECT_EQ( IUS_DB2::NOT_CONNECTED, model.write( true, QString(), nullptr ) );
   expectModelUnchanged();
#ifndef NO_DB
   // The model stays on disk so it can be uploaded later.
   const QMap<QString, QByteArray> files = modelFiles();
   ASSERT_EQ( 1, files.size() );
   EXPECT_EQ( kGuid, modelAttributes( QString::fromUtf8( files.first() ) ).value( "modelGUID" ).toString() );
#endif
}

TEST_F( ModelDbLoadFailure, LookupWithoutARowReturnsNoRows )
{
   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "get_modelID", kGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
   }
   EXPECT_EQ( IUS_DB2::NOROWS, model.load( true, kGuid, &db ) );
   expectModelUnchanged();
}

TEST_F( ModelDbLoadFailure, InfoWithoutARowReturnsNoRows )
{
   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "get_model_info", "42" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
   }
   EXPECT_EQ( IUS_DB2::NOROWS, model.load( QString( "42" ), &db ) );
   expectModelUnchanged();
}

TEST_F( ModelDbLoadFailure, TruncatedXmlFailsAndKeepsTheModel )
{
   const QString stored    = QString::fromUtf8( kStoredXml );
   const QString truncated = stored.left( stored.indexOf( "<analyte" ) + 20 );
   {
      InSequence sequence;
      expectInfo( "42", truncated );
   }
   EXPECT_EQ( IUS_DB2::DBERROR, model.load( QString( "42" ), &db ) );
   expectModelUnchanged();
}

TEST_F( ModelDb, MonteCarloStreamLoadsAsAnAveragedCompositeModel )
{
   // One model per Monte Carlo iteration, one XML element per line.
   const QString iteration = "<model description=\"run.2DSA-MC_01.model\" modelGUID=\"%1\" monteCarlo=\"1\" "
                             "variance=\"0.1\" meniscus=\"5.9\" analysisType=\"1\">\n"
                             "<analyte name=\"solute\" vbar20=\"0.72\" mw=\"50000\" s=\"%2\" D=\"6e-07\" "
                             "f_f0=\"1.3\" signal=\"%3\"/>\n"
                             "</model>\n";
   const QString xml       = QString( "<?xml version=\"1.0\"?>\n<!DOCTYPE US_Model>\n<ModelData version=\"1.0\">\n" ) +
                       iteration.arg( kGuid, "4e-13", "0.6" ) + iteration.arg( kGuid, "5e-13", "0.4" ) + "</ModelData>\n";
   {
      InSequence sequence;
      expectInfo( "42", xml );
      EXPECT_CALL( db, value( 3 ) ).WillOnce( Return( QVariant( "0.0125" ) ) );
   }

   ASSERT_EQ( IUS_DB2::OK, model.load( QString( "42" ), &db ) );
   EXPECT_TRUE( model.monteCarlo );
   EXPECT_EQ( 2, model.nmcixs );
   EXPECT_DOUBLE_EQ( 0.0125, model.variance );
   EXPECT_EQ( QString( "2026-11-14 09:30:00" ), model.timeCreated );
   EXPECT_EQ( QString( "run.2DSA-MC_mcN002.model" ), model.description );
   // Each iteration's solute becomes a component, its signal scaled by 1/iterations.
   ASSERT_EQ( 2, model.components.size() );
   EXPECT_DOUBLE_EQ( 4e-13, model.components[0].s );
   EXPECT_DOUBLE_EQ( 0.3, model.components[0].signal_concentration );
   EXPECT_DOUBLE_EQ( 5e-13, model.components[1].s );
   EXPECT_DOUBLE_EQ( 0.2, model.components[1].signal_concentration );
}

TEST_F( ModelDbLoadFailure, TruncatedMonteCarloStreamFailsAndKeepsTheModel )
{
   const QString iteration = "<model description=\"run.2DSA-MC_01.model\" modelGUID=\"%1\" monteCarlo=\"1\" "
                             "variance=\"0.1\" meniscus=\"5.9\" analysisType=\"1\">\n"
                             "<analyte name=\"solute\" vbar20=\"0.72\" mw=\"50000\" s=\"%2\" D=\"6e-07\" "
                             "f_f0=\"1.3\" signal=\"0.5\"/>\n"
                             "</model>\n";
   // A complete first model followed by a second one cut off inside its analyte.
   const QString complete  = QString( "<?xml version=\"1.0\"?>\n<!DOCTYPE US_Model>\n<ModelData version=\"1.0\">\n" ) +
                            iteration.arg( kGuid, "4e-13" ) + iteration.arg( kGuid, "5e-13" );
   const QString truncated = complete.left( complete.lastIndexOf( "s=\"5e-13\"" ) );
   {
      InSequence sequence;
      expectInfo( "42", truncated );
   }
   EXPECT_EQ( IUS_DB2::DBERROR, model.load( QString( "42" ), &db ) );
   expectModelUnchanged();
}

TEST_F( ModelDb, NonAsciiTextLoadsIntact )
{
   const QString description = QString( QChar( 0x03bb ) ) + " 280 nm " + QChar( 0x2013 ) + " " + QChar( 0x00c5 );
   QString       xml         = QString::fromUtf8( kStoredXml );
   xml.replace( "stored &amp; loaded", description );
   {
      InSequence sequence;
      expectInfo( "42", xml );
   }
   ASSERT_EQ( IUS_DB2::OK, model.load( QString( "42" ), &db ) );
   EXPECT_EQ( description, model.description );
   ASSERT_EQ( 1, model.components.size() );
}

// Local loads, which share the stream readers with the DB loads.

// The XML of a two-iteration Monte Carlo model of the given run.
QString monteCarloXml( const QString& run )
{
   const QString iteration = "<model description=\"%1.2DSA-MC_01.model\" modelGUID=\"%2\" monteCarlo=\"1\" "
                             "variance=\"0.1\" meniscus=\"5.9\" analysisType=\"1\">\n"
                             "<analyte name=\"solute\" vbar20=\"0.72\" mw=\"50000\" s=\"%3\" D=\"6e-07\" "
                             "f_f0=\"1.3\" signal=\"0.5\"/>\n"
                             "</model>\n";
   return QString( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!DOCTYPE US_Model>\n<ModelData version=\"1.0\">\n" ) +
          iteration.arg( run, kGuid, "4e-13" ) + iteration.arg( run, kGuid, "5e-13" ) + "</ModelData>\n";
}

// Write text to a file as UTF-8 and return the file's path.
QString writeUtf8( const QTemporaryDir& dir, const QString& name, const QString& text )
{
   const QString path = dir.filePath( name );
   QFile         file( path );
   EXPECT_TRUE( file.open( QIODevice::WriteOnly ) );
   file.write( text.toUtf8() );
   return path;
}

TEST_F( ModelDbLoadFailure, TruncatedLocalFileFailsAndKeepsTheModel )
{
   QTemporaryDir dir;
   ASSERT_TRUE( dir.isValid() );
   const QString stored = QString::fromUtf8( kStoredXml );
   EXPECT_EQ( IUS_DB2::DBERROR, model.load( writeUtf8( dir, "single.xml", stored.left( stored.indexOf( "<analyte" ) + 20 ) ) ) );
   expectModelUnchanged();

   const QString mc = monteCarloXml( "run" );
   EXPECT_EQ( IUS_DB2::DBERROR, model.load( writeUtf8( dir, "mc.xml", mc.left( mc.lastIndexOf( "s=\"5e-13\"" ) ) ) ) );
   expectModelUnchanged();
}

TEST_F( ModelDbLoadFailure, MalformedStringFailsAndKeepsTheModel )
{
   const QString stored = QString::fromUtf8( kStoredXml );
   EXPECT_EQ( IUS_DB2::DBERROR, model.load_string( stored.left( stored.indexOf( "<analyte" ) + 20 ) ) );
   expectModelUnchanged();
}

TEST_F( ModelDb, NonAsciiMonteCarloTextLoadsIntactFromTheDbAndALocalFile )
{
#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
   // A Latin-1 locale codec makes a stream that is not set to UTF-8 decode the text wrongly.
   struct LocaleCodec
   {
         QTextCodec* saved = QTextCodec::codecForLocale();
         LocaleCodec()
         {
            QTextCodec::setCodecForLocale( QTextCodec::codecForName( "ISO-8859-1" ) );
         }
         ~LocaleCodec()
         {
            QTextCodec::setCodecForLocale( saved );
         }
   } latin1;
#endif
   const QString run = QString( "run " ) + QChar( 0x00c5 ) + QChar( 0x00b5 );
   {
      InSequence sequence;
      expectInfo( "42", monteCarloXml( run ) );
      EXPECT_CALL( db, value( 3 ) ).WillOnce( Return( QVariant( "0.0125" ) ) );
   }
   ASSERT_EQ( IUS_DB2::OK, model.load( QString( "42" ), &db ) );
   EXPECT_EQ( run + ".2DSA-MC_mcN002.model", model.description );

   QTemporaryDir dir;
   ASSERT_TRUE( dir.isValid() );
   US_Model local;
   ASSERT_EQ( IUS_DB2::OK, local.load( writeUtf8( dir, "mc.xml", monteCarloXml( run ) ) ) );
   EXPECT_EQ( run + ".2DSA-MC_mcN002.model", local.description );
}
} // namespace
