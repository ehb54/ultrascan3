//! \file us_import_run.cpp
//! \brief Load a generated AUC run into a US3 LIMS database.
//! Uses the desktop experiment and raw-data persistence paths.

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTextStream>
#include <QDir>
#include <QRegularExpression>
#include <QSet>

#include "us_db2.h"
#include "us_settings.h"
#include "us_convert.h"
#include "us_convertio.h"
#include "us_dataIO.h"
#include "us_experiment.h"
#include "us_solution.h"
#include "us_util.h"
#include "us_defines.h"

namespace
{

QTextStream& out() { static QTextStream s( stdout ); return s; }
QTextStream& err() { static QTextStream s( stderr ); return s; }

//! solution.description is VARCHAR(80) in the LIMS schema.  Reject an
//! archive before opening a transaction rather than letting new_solution()
//! fail with LAST_INSERT_ID() == 0 and a misleading missing-solution error.
constexpr int MAX_SOLUTION_DESCRIPTION_CHARACTERS = 80;

//! Databases accepted without --force.
const QStringList& allowedDatabases()
{
   static const QStringList names{ "uslims3_dev", "uslims3_test" };
   return names;
}

//! Match an experiment record named <runID>.<runType>.xml.
const QRegularExpression& experimentFilePattern()
{
   static const QRegularExpression re( "^(.+)\\.([A-Z]{2})\\.xml$" );
   return re;
}

//! Match <runID>.<editID>.<type>.<cell>.<channel>.<wavelength>.xml.
const QRegularExpression& editFilePattern()
{
   static const QRegularExpression re(
      "^(.+)\\.([^.]+)\\.([A-Z]{2})\\.(\\d+)\\.([A-Z])\\.([^.]+)\\.xml$" );
   return re;
}

void isolateSettings( const QString& root )
{
   if ( ! qEnvironmentVariableIsSet( "US3_SETTINGS_ROOT" ) )
      qputenv( "US3_SETTINGS_ROOT", root.toUtf8() );
}

//! A chemistry record's filename prefix and staging directory.
struct RecordKind
{
   QChar   letter;
   QString directory;
};

const QList< RecordKind >& recordKinds()
{
   // P records join S/B/A because the experiment XML names a project GUID and
   // US_Project::readFromDisk() resolves it out of the same data tree.
   static const QList< RecordKind > kinds{
      { 'P', "projects" }, { 'S', "solutions" }, { 'B', "buffers" },
      { 'A', "analytes" } };
   return kinds;
}

//! Stage chemistry records under the names US_Solution expects.
bool stageRecords( const QString& archive_dir, QMap< QString, int >& staged,
                   QString& error )
{
   QDir source( archive_dir );

   for ( const RecordKind& kind : recordKinds() )
   {
      const QString pattern     = QString( kind.letter ) + "???????.xml";
      const QString target_path = US_Settings::dataDir() + "/" + kind.directory;

      QDir target;
      if ( ! target.mkpath( target_path ) )
      {
         error = "Could not create the staging directory " + target_path;
         return false;
      }

      // Prevent a stale record from satisfying this archive's GUID lookup.
      QDir existing( target_path );
      for ( const QString& stale :
            existing.entryList( QStringList( pattern ), QDir::Files ) )
         existing.remove( stale );

      int count = 0;
      for ( const QString& name :
            source.entryList( QStringList( pattern ), QDir::Files, QDir::Name ) )
      {
         const QString staged_name = QString( "%1%2.xml" )
            .arg( kind.letter )
            .arg( count + 1, 7, 10, QChar( '0' ) );

         if ( ! QFile::copy( source.filePath( name ),
                             target_path + "/" + staged_name ) )
         {
            error = "Could not stage " + kind.directory + " file " + name;
            return false;
         }
         count++;
      }

      staged[ kind.directory ] = count;
   }

   return true;
}

//! Refuse a directory whose experiment records name a run other than this one.
bool rejectExtraRuns( const QString& run_dir, const QString& run_id,
                      QString& error )
{
   // US_Convert::readUS3Disk() takes the first .auc file's run ID for the whole
   // directory, and readDiskRun() names the mixture when the .auc files
   // disagree.  Neither looks at the experiment records, so a directory holding
   // one run's data beside another run's metadata still reads as one run.  A
   // corpus load takes a directory that is exactly one run or nothing.
   QStringList others;

   for ( const QString& name :
         QDir( run_dir ).entryList( QStringList( "*.xml" ), QDir::Files,
                                    QDir::Name ) )
   {
      const QRegularExpressionMatch m = experimentFilePattern().match( name );

      if ( m.hasMatch()  &&  m.captured( 1 ) != run_id  &&
           ! others.contains( m.captured( 1 ) ) )
         others << m.captured( 1 );
   }

   if ( others.isEmpty() )
      return true;

   error = "Directory " + run_dir + " holds run " + run_id
         + " alongside experiment records for " + others.join( ", " )
         + "\nThis loads one run; split the archive by run ID first";
   return false;
}

//! One of the archive's edit files, paired with the raw dataset it names.
struct EditFile
{
   QString path;      //!< Full path, for the blob upload
   QString name;      //!< Basename, stored as editedData.filename
   QString editGUID;  //!< Identity of the edit itself
   QString dataGUID;  //!< The rawDataGUID it applies to
};

//! Validate that edits belong to, and completely cover, the run's raw data.
bool collectEdits( const US_ConvertIO::DiskRun& run, QList< EditFile >& edits,
                   QString& error )
{
   const QString                        run_id   = run.runID;
   const QVector< US_DataIO::RawData >& raw_data = run.rawData;

   QDir dir( run.directory );
   edits.clear();

   // Every check below reads the triple that sits at a raw dataset's index, so
   // a run whose two lists have drifted apart cannot be validated at all.
   if ( run.triples.size() != raw_data.size() )
   {
      error = "Run " + run_id + " has " + QString::number( raw_data.size() )
            + " data files but " + QString::number( run.triples.size() )
            + " triples";
      return false;
   }

   QMap< QString, int > raw_by_guid;
   for ( int raw_index = 0; raw_index < raw_data.size(); raw_index++ )
   {
      const US_DataIO::RawData& raw = raw_data[ raw_index ];
      const QString guid = US_Util::uuid_unparse(
         reinterpret_cast< const unsigned char* >( raw.rawGUID ) ).toLower();

      if ( raw_by_guid.contains( guid ) )
      {
         error = "Run " + run_id + " contains duplicate raw data GUID " + guid;
         return false;
      }

      raw_by_guid.insert( guid, raw_index );
   }

   QSet< QString > covered;
   QSet< QString > edit_guids;

   for ( const QString& name :
         dir.entryList( QStringList( "*.xml" ), QDir::Files, QDir::Name ) )
   {
      const QRegularExpressionMatch match = editFilePattern().match( name );
      if ( ! match.hasMatch() )
         continue;

      // Ignore edits owned by another run in the archive.
      if ( match.captured( 1 ) != run_id )
         continue;

      EditFile edit;
      edit.path = dir.filePath( name );
      edit.name = name;

      US_DataIO::EditValues values;
      const int read_status = US_DataIO::readEdits( edit.path, values );
      if ( read_status != US_DataIO::OK )
      {
         error = "Could not read the edit file " + name
               + " (status " + QString::number( read_status ) + ")";
         return false;
      }

      edit.editGUID = values.editGUID;
      edit.dataGUID = values.dataGUID;

      if ( edit.dataGUID.isEmpty()  ||  edit.editGUID.isEmpty() )
      {
         error = "Edit file " + name + " names no rawDataGUID or editGUID";
         return false;
      }

      const QString data_guid = edit.dataGUID.toLower();
      const QString edit_guid = edit.editGUID.toLower();

      if ( values.runID != run_id )
      {
         error = "Edit file " + name + " identifies run " + values.runID
               + ", not " + run_id;
         return false;
      }

      if ( match.captured( 4 ) != values.cell
           || match.captured( 5 ) != values.channel
           || match.captured( 6 ) != values.wavelength )
      {
         error = "Edit file " + name
               + " does not match the dataset triple in its XML";
         return false;
      }

      if ( ! raw_by_guid.contains( data_guid ) )
      {
         error = "Edit file " + name + " points to raw data " + edit.dataGUID
               + ", which is not present in run " + run_id;
         return false;
      }

      const int raw_index    = raw_by_guid[ data_guid ];
      const US_DataIO::RawData& raw = raw_data[ raw_index ];
      const QString raw_type = QString::fromLatin1( raw.type, 2 );

      if ( match.captured( 3 ) != raw_type
           || values.cell.toInt() != raw.cell
           || values.channel != QString( raw.channel ) )
      {
         error = "Edit file " + name
               + " does not match the type, cell, or channel of raw data "
               + edit.dataGUID;
         return false;
      }

      // The wavelength is checked against the triple rather than against
      // raw.scanData[ 0 ].wavelength, because the scan value is not what the
      // filenames carry.  US_Convert::readUS3Disk() builds the triple out of
      // the .auc filename field, dividing it by 1000 for a WA run because that
      // field is a radius; us_astfem_sim writes the sentinel 123 into that
      // field when the scan wavelength is below 99; and US_Edit names its edit
      // files from the triple.  Comparing the raw numeric text would reject a
      // valid WA or sub-99 run.
      const QString triple_wavelength =
         run.triples[ raw_index ].tripleDesc.section( " / ", 2, 2 );

      if ( values.wavelength != triple_wavelength )
      {
         error = "Edit file " + name + " is for wavelength "
               + values.wavelength + ", but raw data " + edit.dataGUID
               + " is " + triple_wavelength;
         return false;
      }

      if ( edit_guids.contains( edit_guid ) )
      {
         error = "Run " + run_id + " contains duplicate edit GUID "
               + edit.editGUID;
         return false;
      }

      if ( covered.contains( data_guid ) )
      {
         error = "Run " + run_id + " contains more than one edit for raw data "
               + edit.dataGUID;
         return false;
      }

      edit_guids.insert( edit_guid );
      covered.insert( data_guid );
      edits << edit;
   }

   // Submission requires an edit for every raw dataset.
   QStringList missing;
   for ( auto it = raw_by_guid.cbegin(); it != raw_by_guid.cend(); ++it )
   {
      if ( ! covered.contains( it.key() ) )
         missing << it.key();
   }

   if ( ! missing.isEmpty() )
   {
      error = "No edit file for raw data " + missing.join( ", " )
            + "\nEvery .auc needs its edit XML, or the run cannot be "
              "submitted for analysis";
      return false;
   }

   return true;
}

bool uploadEdits( const QList< EditFile >& edits, const QString& run_id,
                  US_DB2* db, QString& error )
{
   // Create-only: a corpus run is loaded into a scratch database once, so an
   // edit that already has a record means the run is not what it claimed to
   // be.  US_ConvertIO owns the mechanics; the policy of stopping at the first
   // failure, and of never updating, is this program's.
   for ( const EditFile& edit : edits )
   {
      int rawDataID = -1;

      if ( US_ConvertIO::findRawDataId( db, edit.dataGUID, rawDataID, error )
           != US_DB2::OK )
      {
         error = "No raw data record for GUID " + edit.dataGUID
               + ", named by edit file " + edit.name;
         return false;
      }

      US_ConvertIO::EditedDataRecord record;
      record.rawDataID = rawDataID;
      record.editGUID  = edit.editGUID;
      record.label     = run_id;
      record.filename  = edit.name;

      int edit_id = -1;

      if ( US_ConvertIO::createEditedData( db, record, edit_id, error )
           != US_DB2::OK )
         return false;

      if ( US_ConvertIO::uploadEditedDataBlob( db, edit_id, edit.path, error )
           != US_DB2::OK )
         return false;
   }

   return true;
}

//! One run, read and checked but not yet written.
struct PreparedRun
{
   US_ConvertIO::DiskRun  disk;   //!< Raw data, triples, experiment, speed steps
   QList< EditFile >      edits;  //!< Corpus-specific; not part of a disk run
};

//! Read and validate one run without touching the database.
bool prepareRun( const QString& run_dir, const QString& wanted_run,
                 const QString& wanted_type, PreparedRun& run, QString& error )
{
   // Reading is shared with the desktop; the strictness is not.  readDiskRun()
   // reports what it finds and leaves the verdict to its caller, and for a
   // corpus load every one of those reports is fatal: a run that goes into a
   // test database has to be exactly what it claims to be.
   const int status = US_ConvertIO::readDiskRun( run_dir, run.disk, error );

   if ( status != US_Convert::OK )
      return false;

   const QString run_id = run.disk.runID;

   if ( ! wanted_run.isEmpty()  &&  wanted_run != run_id )
   {
      error = "--run-id " + wanted_run + " does not match " + run_dir
            + ", which holds run " + run_id;
      return false;
   }

   if ( ! rejectExtraRuns( run.disk.directory, run_id, error ) )
      return false;

   if ( ! wanted_type.isEmpty()  &&  wanted_type != run.disk.runType )
   {
      error = "--run-type " + wanted_type + " does not match run " + run_id
            + ", whose AUC files are " + run.disk.runType;
      return false;
   }

   // Project hydration, like chemistry hydration below, is corpus policy
   // rather than part of reading a run.  The desktop reports a missing project
   // and clears it; a corpus load refuses, because a run whose project cannot
   // be resolved would have one minted for it at write time and the seed would
   // stop being reproducible.
   US_Project& project = run.disk.experiment.project;

   if ( project.projectGUID.isEmpty() )
   {
      error = "Run " + run_id + " names no project GUID";
      return false;
   }

   if ( project.readFromDisk( project.projectGUID ) != US_DB2::OK )
   {
      error = "No project record for GUID " + project.projectGUID + " (run "
            + run_id + ")\nThe archive must include the P record the "
              "simulation wrote";
      return false;
   }

   // Chemistry hydration is corpus policy, not part of reading a run: the
   // desktop recovers from a missing solution and this must not.
   for ( int ii = 0; ii < run.disk.triples.size(); ii++ )
   {
      US_Solution& solution = run.disk.triples[ ii ].solution;
      QString guid          = solution.solutionGUID;

      if ( guid.isEmpty() )
      {
         error = "Triple " + run.disk.triples[ ii ].tripleDesc + " of run "
               + run_id + " names no solution GUID";
         return false;
      }

      if ( solution.readFromDisk( guid ) != US_DB2::OK )
      {
         error = "No solution record for GUID " + guid + " (triple "
               + run.disk.triples[ ii ].tripleDesc + " of run " + run_id
               + ")\nThe archive must include the solution XML the simulation "
                 "wrote";
         return false;
      }

      const int description_length = solution.solutionDesc.toUcs4().size();
      if ( description_length > MAX_SOLUTION_DESCRIPTION_CHARACTERS )
      {
         error = "Solution " + guid + " in run " + run_id + " (triple "
               + run.disk.triples[ ii ].tripleDesc + ") has a description of "
               + QString::number( description_length ) + " characters; the "
                 "LIMS solution.description limit is "
               + QString::number( MAX_SOLUTION_DESCRIPTION_CHARACTERS );
         return false;
      }

      if ( solution.analyteInfo.isEmpty() )
      {
         error = "Solution " + guid + " lists no analytes";
         return false;
      }
   }

   return collectEdits( run.disk, run.edits, error );
}

//! Roll back an open transaction unless the load reaches its commit.
class TransactionGuard
{
public:
   explicit TransactionGuard( US_DB2& connection ) : db( connection ) {}

   ~TransactionGuard()
   {
      if ( ! armed )
         return;

      QString error;

      if ( ! db.rollbackTransaction( error ) )
         err() << "Rollback also failed: " << error << Qt::endl;
      else if ( ! error.isEmpty() )
         err() << "Rollback warning: " << error << Qt::endl;
   }

   //! Called once the commit has succeeded, warning or not.
   void disarm() { armed = false; }

private:
   US_DB2& db;
   bool    armed = true;
};

} // namespace

int main( int argc, char* argv[] )
{
   QCoreApplication application( argc, argv );
   QCoreApplication::setApplicationName( "us_import_run" );
   QCoreApplication::setApplicationVersion( US_Version );

   QCommandLineParser parser;
   parser.setApplicationDescription(
      "Load a generated AUC run into a US3 LIMS database using the desktop's "
      "own write path." );
   const QCommandLineOption help_option    = parser.addHelpOption();
   const QCommandLineOption version_option = parser.addVersionOption();

   QCommandLineOption dir_option( "dir",
      "Directory holding exactly one run -- its .auc files, edit XML and "
      "experiment XML -- along with the P/S/B/A records they name. The "
      "directory name is the run ID", "path" );
   QCommandLineOption host_option( "host",
      "Database host, optionally host:port (default localhost)",
      "host", "localhost" );
   QCommandLineOption db_option( "database",
      "Database name. Required, and refused unless it is one of "
      + allowedDatabases().join( ", " ) + " or --force is given", "name" );
   QCommandLineOption user_option( "user",
      "Database user", "user", "us3" );
   QCommandLineOption password_option( "password",
      "Database password", "password", "" );
   QCommandLineOption investigator_option( "investigator-id",
      "personID recorded as the owner of everything created. Defaults to the "
      "person the US3 credentials authenticate as", "id" );
   QCommandLineOption investigator_guid_option( "investigator-guid",
      "personGUID of the US3 account to act as. Defaults to the investigator "
      "GUID in the experiment XML", "guid" );
   QCommandLineOption investigator_password_option( "investigator-password",
      "Password of that US3 account. This is the US3 login, not the MySQL "
      "password given by --password", "password", "" );
   QCommandLineOption operator_option( "operator-id",
      "personID recorded as the instrument operator, who must hold a permit "
      "for the instrument. Defaults to the operator in the experiment XML, or "
      "to the investigator when it names none", "id" );
   QCommandLineOption work_dir_option( "work-dir",
      "Scratch directory for staged solution records and isolated settings "
      "(default: a 'us_import_run' directory beside --dir)", "path" );
   QCommandLineOption run_id_option( "run-id",
      "Refuse the load unless --dir holds this run. Selects nothing: the "
      "directory already names exactly one run", "id" );
   QCommandLineOption run_type_option( "run-type",
      "Two-character optical type. Defaults to the one named by the "
      "experiment XML", "type" );
   QCommandLineOption force_option( "force",
      "Accept a database name outside the allowed list. This is the only "
      "thing it does: it does not permit overwriting a run that is already "
      "loaded, which fails either way" );
   QCommandLineOption dry_run_option( "dry-run",
      "Read and validate the archive, then stop before database writes. "
      "Record staging still uses --work-dir" );

   parser.addOptions( { dir_option, host_option, db_option, user_option,
                        password_option, investigator_option,
                        investigator_guid_option, investigator_password_option,
                        operator_option, work_dir_option, run_id_option,
                        run_type_option, force_option, dry_run_option } );

   if ( ! parser.parse( QCoreApplication::arguments() ) )
   {
      err() << parser.errorText() << Qt::endl;
      return 1;
   }
   if ( parser.isSet( version_option ) )
   {
      out() << QCoreApplication::applicationName() << " "
            << QCoreApplication::applicationVersion() << Qt::endl;
      return 0;
   }
   if ( parser.isSet( help_option ) )
   {
      out() << parser.helpText() << Qt::endl;
      return 0;
   }

   if ( ! parser.isSet( dir_option ) )
   {
      err() << "--dir is required" << Qt::endl;
      return 1;
   }
   if ( ! parser.isSet( db_option ) )
   {
      err() << "--database is required; there is no default" << Qt::endl;
      return 1;
   }

   const QString database = parser.value( db_option );
   if ( ! allowedDatabases().contains( database ) && ! parser.isSet( force_option ) )
   {
      err() << "Refusing to write to \"" << database << "\": expected one of "
            << allowedDatabases().join( ", " )
            << "\nPass --force to accept this name anyway." << Qt::endl;
      return 1;
   }

   QDir archive( parser.value( dir_option ) );
   if ( ! archive.exists() )
   {
      err() << "No such directory: " << archive.path() << Qt::endl;
      return 1;
   }
   // writeRawDataToDB and readFromDisk both build paths by plain concatenation,
   // so the separator has to be part of the string handed to them.
   const QString archive_dir = archive.absolutePath() + "/";

   const QString work_dir = parser.isSet( work_dir_option )
      ? QDir( parser.value( work_dir_option ) ).absolutePath()
      : archive.absoluteFilePath( "../us_import_run" );

   isolateSettings( work_dir + "/settings" );
   US_Settings::set_workBaseDir( work_dir );

   QMap< QString, int > staged;
   QString stage_error;
   if ( ! stageRecords( archive_dir, staged, stage_error ) )
   {
      err() << stage_error << Qt::endl;
      return 1;
   }

   out() << "archive      : " << archive_dir << Qt::endl;
   out() << "work dir     : " << work_dir << Qt::endl;
   out() << "staged       : " << staged[ "projects" ] << " projects, "
         << staged[ "solutions" ] << " solutions, "
         << staged[ "buffers" ] << " buffers, "
         << staged[ "analytes" ] << " analytes" << Qt::endl;
   out().flush();

   PreparedRun run;
   QString     prepare_error;

   if ( ! prepareRun( archive_dir, parser.value( run_id_option ),
                      parser.value( run_type_option ).toUpper(), run,
                      prepare_error ) )
   {
      err() << prepare_error << Qt::endl;
      return 2;
   }

   out() << "run          : " << run.disk.runID << " : " << run.disk.runType
         << ", " << run.disk.triples.size() << " triples, "
         << run.edits.size() << " edits" << Qt::endl;

   if ( parser.isSet( dry_run_option ) )
   {
      out() << "dry run, nothing written to the database" << Qt::endl;
      return 0;
   }

   QString investigator_guid = parser.value( investigator_guid_option );
   if ( investigator_guid.isEmpty() )
      investigator_guid = run.disk.experiment.invGUID;

   if ( investigator_guid.isEmpty() )
   {
      err() << "No investigator: the experiment XML names no GUID and "
               "--investigator-guid was not given" << Qt::endl;
      return 3;
   }

   // Connect from the values on the command line.  connectAuthenticated()
   // exists so that this does not have to write a database profile into the
   // user's settings to get validate_user run for it.
   US_DB2 db;
   QString db_error;
   if ( ! db.connectAuthenticated( parser.value( host_option ), database,
                                   parser.value( user_option ),
                                   parser.value( password_option ),
                                   investigator_guid,
                                   parser.value(
                                      investigator_password_option ),
                                   db_error ) )
   {
      err() << "Could not connect to " << database << " as "
            << investigator_guid << ": " << db_error << Qt::endl;
      return 3;
   }

   // validate_user leaves the authenticated person ID in this session value.
   int investigator = 0;
   db.rawQuery( "SELECT @US3_ID" );
   if ( db.next() )
      investigator = db.value( 0 ).toInt();

   if ( parser.isSet( investigator_option ) )
      investigator = parser.value( investigator_option ).toInt();

   if ( investigator <= 0 )
   {
      err() << "Authenticated, but no personID came back for "
            << investigator_guid << Qt::endl;
      return 3;
   }

   out() << "investigator : " << investigator << " (" << investigator_guid
         << ")" << Qt::endl;

   QString transaction_error;
   if ( ! db.beginTransaction( transaction_error ) )
   {
      err() << "Could not begin database transaction: " << transaction_error
            << Qt::endl;
      return 3;
   }

   // Everything from here to the commit is one run's experiment, raw and edit
   // rows.  The guard rolls back on any path out, so a failure part way through
   // cannot leave the run half loaded for the seed dump to pick up.
   TransactionGuard transaction( db );

   // New chemistry records are owned by the active investigator.
   run.disk.experiment.invID = investigator;
   US_Settings::set_us_inv_ID( investigator );

   // Generated experiments use the investigator when no operator is set.
   if ( parser.isSet( operator_option ) )
      run.disk.experiment.operatorID = parser.value( operator_option ).toInt();
   else if ( run.disk.experiment.operatorID <= 0 )
      run.disk.experiment.operatorID = investigator;

   // Create only.  The corpus is built into a fresh database, so a run ID that
   // is already there is a duplicate archive rather than a revision, and
   // saveToDB() answers that with DUPFIELD instead of rewriting the run.  The
   // speed steps are the ones the experiment XML carried: saveToDB() rewrites
   // the rows from this vector, so passing the empty one it used to be given is
   // what left every loaded run with no speed profile in the database.
   const int save_status =
      run.disk.experiment.saveToDB( false, &db, run.disk.speedSteps );

   if ( save_status == US_DB2::DUPFIELD )
   {
      err() << "Run " << run.disk.runID << " is already in " << database
            << Qt::endl;
      return 3;
   }

   if ( save_status != US_DB2::OK )
   {
      err() << "Could not save experiment " << run.disk.runID << " (status "
            << save_status << "): " << db.lastError() << Qt::endl;
      return 3;
   }

   const QString write_error = US_ConvertIO::writeRawDataToDB(
      run.disk.experiment, run.disk.triples, run.disk.directory, &db );

   if ( ! write_error.isEmpty() )
   {
      err() << write_error << Qt::endl;
      return 3;
   }

   QString edit_error;
   if ( ! uploadEdits( run.edits, run.disk.runID, &db, edit_error ) )
   {
      err() << edit_error << Qt::endl;
      return 3;
   }

   if ( ! db.commitTransaction( transaction_error ) )
   {
      err() << "Could not commit database transaction: " << transaction_error
            << Qt::endl;
      return 3;
   }

   transaction.disarm();

   if ( ! transaction_error.isEmpty() )
      err() << "Commit warning: " << transaction_error << Qt::endl;

   out() << "loaded       : " << run.disk.runID << ", experimentID "
         << run.disk.experiment.expID << ", " << run.disk.triples.size()
         << " triples, " << run.edits.size() << " edits into " << database
         << Qt::endl;
   return 0;
}
